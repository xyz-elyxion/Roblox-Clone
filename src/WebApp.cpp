#include "WebApp.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <sstream>
#include <utility>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <direct.h>
#include <windows.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "WebCrypto.h"

namespace rblx {
namespace web {
namespace {

const char* kDefaultGameIcon = "/assets/games/default-game-icon.png";
const char* kDefaultGameBanner = "/assets/games/default-game-banner.png";
const long long kMaxWorldBytes = 2 * 1024 * 1024;

const char* kAvatarSlots[kAvatarSlotCount] = {
    "headColor", "torsoColor", "leftArmColor",
    "rightArmColor", "leftLegColor", "rightLegColor"
};

std::string lowerCopy(const std::string& value) {
    std::string out = value;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

std::string trimCopy(const std::string& value) {
    std::size_t begin = 0;
    std::size_t end = value.size();
    while (begin < end && (value[begin] == ' ' || value[begin] == '\t' ||
                           value[begin] == '\r' || value[begin] == '\n')) {
        ++begin;
    }
    while (end > begin && (value[end - 1] == ' ' || value[end - 1] == '\t' ||
                           value[end - 1] == '\r' || value[end - 1] == '\n')) {
        --end;
    }
    return value.substr(begin, end - begin);
}

std::string twoDigit(int value) {
    const std::string text = std::to_string(value);
    return text.size() < 2 ? "0" + text : text;
}

std::string isoFromTime(std::time_t value) {
    std::tm parts{};
#ifdef _WIN32
    gmtime_s(&parts, &value);
#else
    gmtime_r(&value, &parts);
#endif
    char year[16];
    std::snprintf(year, sizeof(year), "%04d", parts.tm_year + 1900);
    return std::string(year) + "-" + twoDigit(parts.tm_mon + 1) + "-" +
           twoDigit(parts.tm_mday) + "T" + twoDigit(parts.tm_hour) + ":" +
           twoDigit(parts.tm_min) + ":" + twoDigit(parts.tm_sec) + "Z";
}

std::string nowIso() {
    return isoFromTime(std::time(nullptr));
}

long long nowMilliseconds() {
    return static_cast<long long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
}

bool isAbsolutePath(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    if (path[0] == '/' || path[0] == '\\') {
        return true;
    }
    return path.size() > 1 && std::isalpha(static_cast<unsigned char>(path[0])) != 0 &&
           path[1] == ':';
}

std::string directoryOf(const std::string& path) {
    const std::size_t slash = path.find_last_of("/\\");
    if (slash == std::string::npos) {
        return std::string();
    }
    if (slash == 0) {
        return path.substr(0, 1);
    }
    return path.substr(0, slash);
}

std::string joinPath(const std::string& base, const std::string& child) {
    if (base.empty()) {
        return child;
    }
    if (child.empty()) {
        return base;
    }
    if (base.back() == '/' || base.back() == '\\') {
        return base + child;
    }
    return base + "/" + child;
}

bool fileExists(const std::string& path) {
    std::ifstream stream(path.c_str(), std::ios::binary);
    return stream.good();
}

bool makeDirectories(const std::string& path) {
    if (path.empty()) {
        return true;
    }
    std::string current;
    for (std::size_t i = 0; i < path.size(); ++i) {
        const char c = path[i];
        current.push_back(c);
        const bool separator = c == '/' || c == '\\';
        const bool last = i + 1 == path.size();
        if (!separator && !last) {
            continue;
        }
        if (current == "/" || (current.size() == 2 && current[1] == ':')) {
            continue;
        }
#ifdef _WIN32
        _mkdir(current.c_str());
#else
        ::mkdir(current.c_str(), 0777);
#endif
    }
    return true;
}

std::string currentDirectory() {
    char buffer[4096];
#ifdef _WIN32
    if (_getcwd(buffer, sizeof(buffer)) == nullptr) {
        return std::string(".");
    }
#else
    if (::getcwd(buffer, sizeof(buffer)) == nullptr) {
        return std::string(".");
    }
#endif
    return std::string(buffer);
}

std::string executableDirectory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    const DWORD length = ::GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length == 0) {
        return currentDirectory();
    }
    return directoryOf(std::string(buffer, length));
#else
    char buffer[4096];
    const ssize_t length = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length <= 0) {
        return currentDirectory();
    }
    buffer[length] = '\0';
    return directoryOf(std::string(buffer, static_cast<std::size_t>(length)));
#endif
}

std::string resolvePath(const std::string& base, const std::string& path) {
    if (path.empty()) {
        return path;
    }
    return isAbsolutePath(path) ? path : joinPath(base, path);
}

bool readWholeFile(const std::string& path, std::string* out) {
    std::ifstream stream(path.c_str(), std::ios::binary);
    if (!stream) {
        return false;
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    *out = buffer.str();
    return true;
}

// Client.exe opens shaders/, ServerWorld.world and face*.png relative to its
// working directory (see scripts/package-build.ps1), so it has to run from the
// folder that ships them - the one holding the executable in a packaged build.
std::string preferredClientWorkingDirectory(const std::string& clientPath,
                                            const std::string& fallback) {
    const std::string clientDir = directoryOf(clientPath);
    if (!clientDir.empty() &&
        fileExists(joinPath(joinPath(clientDir, "shaders"), "vertex.glsl"))) {
        return clientDir;
    }
    return fallback;
}

// Client.cpp redirects its own stdout and stderr into these files before main()
// runs, so a spawned copy prints nothing on the handles we give it and its log is
// the only record of why it stopped. Report the end of it rather than an exit code
// on its own.
std::string clientLogTail(const std::string& directory) {
    const char* const names[] = {"client_log.txt", "client_error_log.txt"};
    for (const char* name : names) {
        std::string text;
        if (!readWholeFile(joinPath(directory, name), &text)) {
            continue;
        }
        const std::size_t limit = 240;
        std::string tail = text.size() > limit ? text.substr(text.size() - limit) : text;
        for (char& c : tail) {
            if (c == '\r' || c == '\n' || c == '\t') {
                c = ' ';
            }
        }
        const std::size_t first = tail.find_first_not_of(' ');
        if (first == std::string::npos) {
            continue;
        }
        return " " + std::string(name) + " says: " + tail.substr(first);
    }
    return std::string();
}

// A game server that dies during startup usually says why on stderr; keep the
// end of that output so the failure message can include it.
std::string readLogTail(const std::string& path, std::size_t maxBytes) {
    if (path.empty()) {
        return std::string();
    }
    std::ifstream stream(path.c_str(), std::ios::binary);
    if (!stream) {
        return std::string();
    }
    stream.seekg(0, std::ios::end);
    const std::streamoff size = stream.tellg();
    if (size <= 0) {
        return std::string();
    }
    const std::streamoff start = size > static_cast<std::streamoff>(maxBytes)
                                     ? size - static_cast<std::streamoff>(maxBytes)
                                     : 0;
    stream.seekg(start, std::ios::beg);
    std::string text(static_cast<std::size_t>(size - start), '\0');
    stream.read(&text[0], static_cast<std::streamsize>(text.size()));
    text.resize(static_cast<std::size_t>(stream.gcount()));
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
        text.pop_back();
    }
    return text;
}

bool writeWholeFile(const std::string& path, const std::string& contents) {
    makeDirectories(directoryOf(path));
    std::ofstream stream(path.c_str(), std::ios::binary | std::ios::trunc);
    if (!stream) {
        return false;
    }
    stream.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    return stream.good();
}

std::string percentEncode(const std::string& value) {
    static const char kDigits[] = "0123456789ABCDEF";
    std::string out;
    out.reserve(value.size() * 3);
    for (const char rawChar : value) {
        const unsigned char c = static_cast<unsigned char>(rawChar);
        const bool unreserved = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                                (c >= '0' && c <= '9') || c == '-' || c == '_' ||
                                c == '.' || c == '~';
        if (unreserved) {
            out.push_back(static_cast<char>(c));
        } else {
            out.push_back('%');
            out.push_back(kDigits[(c >> 4) & 0x0F]);
            out.push_back(kDigits[c & 0x0F]);
        }
    }
    return out;
}

// parseInt semantics: leading integer, trailing characters ignored.
bool parseIntegerPrefix(const std::string& token, int* out) {
    if (token.empty()) {
        return false;
    }
    char* end = nullptr;
    const long value = std::strtol(token.c_str(), &end, 10);
    if (end == token.c_str() || value < -100000000L || value > 100000000L) {
        return false;
    }
    *out = static_cast<int>(value);
    return true;
}

// Number() semantics: the entire token must be numeric.
bool parseNumberToken(const std::string& token, double* out) {
    if (trimCopy(token).empty()) {
        return false;
    }
    char* end = nullptr;
    const double value = std::strtod(token.c_str(), &end);
    if (end == token.c_str() || *end != '\0' || !std::isfinite(value)) {
        return false;
    }
    *out = value;
    return true;
}

std::vector<std::string> tokenizeWorldLine(const std::string& line) {
    std::vector<std::string> tokens;
    std::size_t i = 0;
    while (i < line.size()) {
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
            ++i;
        }
        if (i >= line.size()) {
            break;
        }
        if (line[i] == '"') {
            ++i;
            std::string token;
            while (i < line.size()) {
                if (line[i] == '\\' && i + 1 < line.size() && line[i + 1] == '"') {
                    token.push_back('"');
                    i += 2;
                    continue;
                }
                if (line[i] == '"') {
                    ++i;
                    break;
                }
                token.push_back(line[i]);
                ++i;
            }
            tokens.push_back(token);
            continue;
        }
        const std::size_t start = i;
        while (i < line.size() && line[i] != ' ' && line[i] != '\t') {
            ++i;
        }
        tokens.push_back(line.substr(start, i - start));
    }
    return tokens;
}

bool worldBool(const std::string& token) {
    return token == "1" || token == "true";
}

bool boolValue(const Json& value, bool fallback) {
    if (value.isBool()) {
        return value.asBool();
    }
    if (value.isNumber()) {
        return value.asNumber() != 0.0;
    }
    if (value.isString()) {
        const std::string normalized = lowerCopy(trimCopy(value.asString()));
        if (normalized == "true" || normalized == "1" || normalized == "yes" ||
            normalized == "public") {
            return true;
        }
        if (normalized == "false" || normalized == "0" || normalized == "no" ||
            normalized == "private") {
            return false;
        }
    }
    return fallback;
}

int parseUserId(const std::string& value) {
    int parsed = 0;
    if (!parseIntegerPrefix(value, &parsed)) {
        return 0;
    }
    return parsed > 0 ? parsed : 0;
}

int parseUserIdValue(const Json& value) {
    if (value.isNumber()) {
        const long long parsed = value.asInteger(0);
        return parsed > 0 && parsed < 1000000 ? static_cast<int>(parsed) : 0;
    }
    if (value.isString()) {
        return parseUserId(value.asString());
    }
    return 0;
}

long long clampPlaytimeSeconds(const Json& value) {
    long long seconds = 0;
    if (value.isNumber()) {
        seconds = value.asInteger(0);
    } else if (value.isString()) {
        int parsed = 0;
        if (!parseIntegerPrefix(value.asString(), &parsed)) {
            return 0;
        }
        seconds = parsed;
    }
    if (seconds <= 0) {
        return 0;
    }
    return std::min<long long>(seconds, 24 * 60 * 60);
}

int parsePlayerCount(const Json& value) {
    long long count = -1;
    if (value.isNumber()) {
        count = value.asInteger(-1);
    } else if (value.isString()) {
        int parsed = 0;
        if (!parseIntegerPrefix(value.asString(), &parsed)) {
            return -1;
        }
        count = parsed;
    }
    if (count < 0 || count > 200) {
        return -1;
    }
    return static_cast<int>(count);
}

std::string normalizeWhitespace(const std::string& value) {
    std::string out;
    bool previousWasSpace = false;
    for (const char rawChar : trimCopy(value)) {
        const bool isSpace = rawChar == ' ' || rawChar == '\t' || rawChar == '\r' ||
                             rawChar == '\n';
        if (isSpace) {
            if (!previousWasSpace) {
                out.push_back(' ');
            }
            previousWasSpace = true;
            continue;
        }
        out.push_back(rawChar);
        previousWasSpace = false;
    }
    return out;
}

std::string normalizeGameTitle(const std::string& value) {
    const std::string title = normalizeWhitespace(value);
    if (title.size() < 3 || title.size() > 60) {
        return std::string();
    }
    return title;
}

std::string normalizeGameDescription(const std::string& value) {
    const std::string description = normalizeWhitespace(value);
    return description.size() > 240 ? description.substr(0, 240) : description;
}

std::string slugify(const std::string& value) {
    std::string slug;
    bool previousWasDash = false;
    for (const char rawChar : lowerCopy(value)) {
        const unsigned char c = static_cast<unsigned char>(rawChar);
        if (std::isalnum(c) != 0) {
            slug.push_back(static_cast<char>(c));
            previousWasDash = false;
        } else if (!previousWasDash) {
            slug.push_back('-');
            previousWasDash = true;
        }
    }
    while (!slug.empty() && slug.front() == '-') {
        slug.erase(slug.begin());
    }
    while (!slug.empty() && slug.back() == '-') {
        slug.pop_back();
    }
    if (slug.size() > 48) {
        slug.resize(48);
        while (!slug.empty() && slug.back() == '-') {
            slug.pop_back();
        }
    }
    return slug.empty() ? std::string("game") : slug;
}

std::string validateCredentials(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return "Username and password are required.";
    }
    if (username.size() < 3 || username.size() > 20) {
        return "Username must be 3-20 characters and use only letters, numbers, or underscores.";
    }
    for (const char rawChar : username) {
        const unsigned char c = static_cast<unsigned char>(rawChar);
        if (std::isalnum(c) == 0 && c != '_') {
            return "Username must be 3-20 characters and use only letters, numbers, or underscores.";
        }
    }
    if (password.size() < 8 || password.size() > 128) {
        return "Password must be between 8 and 128 characters.";
    }
    return std::string();
}

std::string validateLoginInput(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return "Username and password are required.";
    }
    if (username.size() > 20 || password.size() > 128) {
        return "Invalid username or password.";
    }
    return std::string();
}

bool sanitizeAvatar(const Json& value, Avatar* avatar) {
    if (!value.isObject()) {
        return false;
    }
    Avatar sanitized;
    for (int slot = 0; slot < kAvatarSlotCount; ++slot) {
        const Json& colors = value.member(kAvatarSlots[slot]);
        if (!colors.isArray() || colors.size() != 3) {
            return false;
        }
        for (std::size_t component = 0; component < 3; ++component) {
            const Json& entry = colors.elements()[component];
            if (!entry.isNumber()) {
                return false;
            }
            const double number = entry.asNumber();
            if (!std::isfinite(number) || number < 0.0 || number > 1.0) {
                return false;
            }
            sanitized.colors[slot][component] = number;
        }
    }
    if (value.has("faceId")) {
        const std::string faceId = value.member("faceId").asString();
        if (faceId != "classic" && faceId != "happy" && faceId != "surprised" &&
            faceId != "smirk" && faceId != "wink") {
            return false;
        }
        sanitized.faceId = faceId;
    }
    *avatar = sanitized;
    return true;
}

std::string bearerToken(const std::string& authorization) {
    const std::string lowered = lowerCopy(authorization);
    const std::string prefix = "bearer ";
    if (lowered.compare(0, prefix.size(), prefix) != 0) {
        return std::string();
    }
    return trimCopy(authorization.substr(prefix.size()));
}

long long claimUserId(const Json& claims) {
    if (claims.member("userId").isNumber()) {
        return claims.member("userId").asInteger(0);
    }
    const std::string subject = claims.member("sub").asString();
    int parsed = 0;
    return parseIntegerPrefix(subject, &parsed) ? parsed : 0;
}

std::string issueToken(const Config& config, int userId, const std::string& username) {
    const long long issuedAt = static_cast<long long>(std::time(nullptr));
    Json payload = Json::makeObject();
    payload.set("userId", userId);
    payload.set("username", username);
    payload.set("sub", std::to_string(userId));
    payload.set("iat", issuedAt);
    payload.set("exp", issuedAt + config.jwtExpiresSeconds);
    return signJwt(payload, config.jwtSecret);
}

bool isUnsafeDevSecret(const std::string& secret) {
    static const char* kUnsafe[] = {
        "", "your-secret-key-change-this-in-production",
        "replace-this-with-a-long-random-secret-before-production", "change-me", "dev-secret",
        "local-dev-secret", "local-dev-secret-change-me"
    };
    for (const char* entry : kUnsafe) {
        if (secret == entry) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> splitCsv(const std::string& value) {
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (start <= value.size()) {
        const std::size_t comma = value.find(',', start);
        const std::string part = trimCopy(
            value.substr(start, comma == std::string::npos ? std::string::npos : comma - start));
        if (!part.empty()) {
            parts.push_back(part);
        }
        if (comma == std::string::npos) {
            break;
        }
        start = comma + 1;
    }
    return parts;
}

std::string envString(const char* name) {
    const char* value = std::getenv(name);
    return value == nullptr ? std::string() : std::string(value);
}

int parsePositiveInt(const std::string& text, int fallback) {
    int parsed = 0;
    if (!parseIntegerPrefix(text, &parsed) || parsed <= 0) {
        return fallback;
    }
    return parsed;
}

long long parseDurationSeconds(const std::string& text, long long fallback) {
    const std::string value = trimCopy(text);
    if (value.empty()) {
        return fallback;
    }
    int amount = 0;
    if (!parseIntegerPrefix(value, &amount) || amount <= 0) {
        return fallback;
    }
    const char unit = value.back();
    switch (unit) {
        case 's': return amount;
        case 'm': return static_cast<long long>(amount) * 60;
        case 'h': return static_cast<long long>(amount) * 3600;
        case 'd': return static_cast<long long>(amount) * 86400;
        default: return amount;
    }
}

std::string firstExistingPath(const std::vector<std::string>& candidates) {
    for (const std::string& candidate : candidates) {
        if (!candidate.empty() && fileExists(candidate)) {
            return candidate;
        }
    }
    return std::string();
}

void errorJson(Response& response, int status, const std::string& message) {
    Json body = Json::makeObject();
    body.set("error", message);
    response.json(status, body.dump());
}

void sendJson(Response& response, int status, const Json& body) {
    response.json(status, body.dump());
}

Json colorsJson(const double colors[3]) {
    Json array = Json::makeArray();
    for (int i = 0; i < 3; ++i) {
        array.push(Json(colors[i]));
    }
    return array;
}

std::string pageFileForPath(const std::string& path) {
    if (path == "/" || path == "/index") {
        return "index.html";
    }
    if (path == "/login") {
        return "login.html";
    }
    if (path == "/signup") {
        return "signup.html";
    }
    if (path == "/dashboard") {
        return "dashboard.html";
    }
    if (path == "/avatar") {
        return "avatar.html";
    }
    if (path == "/search") {
        return "search.html";
    }
    if (path == "/games") {
        return "games.html";
    }
    if (path == "/create") {
        return "create.html";
    }
    if (path == "/profile") {
        return "profile.html";
    }
    if (path == "/admin") {
        return "admin.html";
    }
    if (path.compare(0, 7, "/games/") == 0 || path.compare(0, 9, "/profile/") == 0) {
        return path.compare(0, 7, "/games/") == 0 ? "game.html" : "profile.html";
    }
    return std::string();
}

#ifdef _WIN32
std::string quoteWindowsArgument(const std::string& argument) {
    if (!argument.empty() && argument.find_first_of(" \t\"") == std::string::npos) {
        return argument;
    }
    std::string result = "\"";
    std::size_t backslashes = 0;
    for (const char c : argument) {
        if (c == '\\') {
            ++backslashes;
            continue;
        }
        if (c == '"') {
            result.append(backslashes * 2 + 1, '\\');
            result.push_back('"');
            backslashes = 0;
            continue;
        }
        result.append(backslashes, '\\');
        backslashes = 0;
        result.push_back(c);
    }
    result.append(backslashes * 2, '\\');
    result.push_back('"');
    return result;
}
#endif

}  // namespace

bool analyzeWorldSource(const std::string& worldText, WorldAnalysis* analysis,
                        std::string* error) {
    if (analysis == nullptr) {
        return false;
    }
    if (trimCopy(worldText).empty()) {
        if (error) {
            *error = "World data is required.";
        }
        return false;
    }
    if (worldText.size() > static_cast<std::size_t>(kMaxWorldBytes)) {
        if (error) {
            *error = "World data is too large for this local server.";
        }
        return false;
    }

    std::string normalized;
    normalized.reserve(worldText.size());
    for (std::size_t i = 0; i < worldText.size(); ++i) {
        if (worldText[i] == '\r') {
            if (i + 1 < worldText.size() && worldText[i + 1] == '\n') {
                continue;
            }
            normalized.push_back('\n');
            continue;
        }
        normalized.push_back(worldText[i]);
    }

    std::vector<std::string> lines;
    std::size_t start = 0;
    while (start <= normalized.size()) {
        const std::size_t newline = normalized.find('\n', start);
        const std::string line = normalized.substr(
            start, newline == std::string::npos ? std::string::npos : newline - start);
        if (!trimCopy(line).empty()) {
            lines.push_back(line);
        }
        if (newline == std::string::npos) {
            break;
        }
        start = newline + 1;
    }

    if (lines.empty()) {
        if (error) {
            *error = "World data is required.";
        }
        return false;
    }

    int partsCount = 0;
    if (!parseIntegerPrefix(lines[0], &partsCount) || partsCount < 0 || partsCount > 4096) {
        if (error) {
            *error = "World file header must contain a valid part count.";
        }
        return false;
    }
    if (static_cast<int>(lines.size()) - 1 < partsCount) {
        if (error) {
            *error = "World file ended before all parts were present.";
        }
        return false;
    }

    WorldAnalysis result;
    int dynamicPartsCount = 0;
    int spawnCount = 0;
    int shapeCounts[4] = {0, 0, 0, 0};

    for (int index = 0; index < partsCount; ++index) {
        const std::vector<std::string> tokens =
            tokenizeWorldLine(lines[static_cast<std::size_t>(index) + 1]);
        const std::string partLabel = "World part " + std::to_string(index + 1);
        if (tokens.size() < 23) {
            if (error) {
                *error = partLabel + " is missing fields.";
            }
            return false;
        }

        int shape = 0;
        if (!parseIntegerPrefix(tokens[0], &shape) || shape < 0 || shape > 3) {
            if (error) {
                *error = partLabel + " has an unsupported shape.";
            }
            return false;
        }

        const int numberFields[] = {2, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22};
        for (const int fieldIndex : numberFields) {
            double value = 0.0;
            if (!parseNumberToken(tokens[static_cast<std::size_t>(fieldIndex)], &value)) {
                if (error) {
                    *error = partLabel + " contains invalid numeric data.";
                }
                return false;
            }
        }

        if (worldBool(tokens[5])) {
            ++spawnCount;
        }
        if (!worldBool(tokens[20]) && !worldBool(tokens[3])) {
            ++dynamicPartsCount;
        }
        ++shapeCounts[shape];
    }

    result.source = normalized;
    if (result.source.empty() || result.source.back() != '\n') {
        result.source.push_back('\n');
    }
    result.partsCount = partsCount;
    result.dynamicPartsCount = dynamicPartsCount;
    result.spawnCount = spawnCount;
    result.cubeCount = shapeCounts[0];
    result.sphereCount = shapeCounts[1];
    result.wedgeCount = shapeCounts[2];
    result.cylinderCount = shapeCounts[3];
    *analysis = result;
    return true;
}

bool loadConfigFromEnvironment(Config* config, std::string* error) {
    if (config == nullptr) {
        return false;
    }
    Config loaded;

    loaded.nodeEnv = envString("NODE_ENV");
    if (loaded.nodeEnv.empty()) {
        loaded.nodeEnv = "development";
    }
    loaded.isProduction = loaded.nodeEnv == "production";

    loaded.port = parsePositiveInt(envString("PORT"), 3000);
    loaded.jwtSecret = envString("JWT_SECRET");
    if (loaded.jwtSecret.empty() && !loaded.isProduction) {
        loaded.jwtSecret = "local-dev-secret-change-me";
    }
    if (loaded.isProduction &&
        (isUnsafeDevSecret(loaded.jwtSecret) || loaded.jwtSecret.size() < 32)) {
        if (error) {
            *error = "JWT_SECRET must be set to a strong non-default value in production.";
        }
        return false;
    }
    loaded.jwtExpiresSeconds = parseDurationSeconds(envString("JWT_EXPIRES_IN"), 7 * 86400);
    loaded.passwordIterations = parsePositiveInt(envString("PASSWORD_ITERATIONS"), 120000);

    const std::string requestedWebRoot = envString("WEB_DIR");
    std::vector<std::string> roots;
    if (!requestedWebRoot.empty()) {
        roots.push_back(resolvePath(currentDirectory(), requestedWebRoot));
    } else {
        const std::string bases[] = {executableDirectory(), currentDirectory()};
        for (const std::string& base : bases) {
            std::string candidate = base;
            for (int depth = 0; depth < 4; ++depth) {
                roots.push_back(joinPath(candidate, "web"));
                roots.push_back(candidate);
                const std::string parent = directoryOf(candidate);
                if (parent.empty() || parent == candidate) {
                    break;
                }
                candidate = parent;
            }
        }
    }

    std::string webRoot;
    for (const std::string& candidate : roots) {
        if (fileExists(joinPath(joinPath(candidate, "public"), "index.html"))) {
            webRoot = candidate;
            break;
        }
    }
    if (webRoot.empty()) {
        for (const std::string& candidate : roots) {
            if (fileExists(joinPath(candidate, "index.html"))) {
                webRoot = directoryOf(candidate);
                break;
            }
        }
    }
    if (webRoot.empty()) {
        if (error) {
            *error =
                "Could not find the web assets. Set WEB_DIR to the directory that contains "
                "public/index.html.";
        }
        return false;
    }

    loaded.webRoot = webRoot;
    loaded.repoRoot = directoryOf(webRoot);
    loaded.dataPath = resolvePath(webRoot, envString("DATABASE_PATH"));
    if (loaded.dataPath.empty()) {
        loaded.dataPath = joinPath(webRoot, "users.json");
    }
    loaded.gameWorldsDir = resolvePath(webRoot, envString("GAME_WORLDS_DIR"));
    if (loaded.gameWorldsDir.empty()) {
        loaded.gameWorldsDir = joinPath(webRoot, "game-worlds");
    }

    const std::string cors = envString("CORS_ORIGIN");
    if (!cors.empty()) {
        loaded.corsOrigins = splitCsv(cors);
    } else if (!loaded.isProduction) {
        loaded.corsOrigins.push_back("http://localhost:" + std::to_string(loaded.port));
        loaded.corsOrigins.push_back("http://127.0.0.1:" + std::to_string(loaded.port));
    }

    loaded.authRateLimitWindowMs =
        static_cast<long long>(parsePositiveInt(envString("AUTH_RATE_LIMIT_WINDOW_MS"), 900000));
    loaded.authRateLimitMax = parsePositiveInt(envString("AUTH_RATE_LIMIT_MAX"), 20);
    loaded.jsonBodyLimitBytes = static_cast<std::size_t>(
        parsePositiveInt(envString("JSON_BODY_LIMIT_BYTES"), 2 * 1024 * 1024));

    loaded.gameServerHost = envString("GAME_SERVER_HOST");
    if (loaded.gameServerHost.empty()) {
        loaded.gameServerHost = "127.0.0.1";
    }
    loaded.gameServerBasePort = parsePositiveInt(envString("GAME_SERVER_BASE_PORT"), 7777);
    loaded.gameInstanceEmptyGraceMs =
        static_cast<long long>(parsePositiveInt(envString("GAME_INSTANCE_EMPTY_GRACE_MS"), 30000));
    loaded.gameInstanceLaunchGraceMs =
        static_cast<long long>(parsePositiveInt(envString("GAME_INSTANCE_LAUNCH_GRACE_MS"), 120000));

    loaded.adminUsernames.clear();
    const std::string adminNames = envString("ADMIN_USERNAMES");
    if (adminNames.empty()) {
        // The seeded creator account owns the starter games.
        loaded.adminUsernames.push_back("RBLXCreator");
    } else {
        for (const std::string& name : splitCsv(adminNames)) {
            const std::string trimmed = trimCopy(name);
            if (!trimmed.empty()) {
                loaded.adminUsernames.push_back(trimmed);
            }
        }
    }

    loaded.publicBaseUrl = envString("PUBLIC_BASE_URL");
    if (loaded.publicBaseUrl.empty()) {
        loaded.publicBaseUrl = "http://localhost:" + std::to_string(loaded.port);
    }

    // The packaged build keeps WebApp.exe, Server.exe and Client.exe side by side,
    // so look next to this executable before the development build folders. Without
    // this, starting WebApp.exe straight from the package finds no game server and
    // Play launches a client that has nothing to connect to.
    const std::string exeDir = executableDirectory();

    const std::string serverOverride = envString("SERVER_EXECUTABLE_PATH");
    if (!serverOverride.empty()) {
        loaded.serverExecutablePath = resolvePath(currentDirectory(), serverOverride);
    } else {
        loaded.serverExecutablePath = firstExistingPath({
            joinPath(exeDir, "Server.exe"),
            joinPath(currentDirectory(), "Server.exe"),
            joinPath(loaded.repoRoot, "Server.exe"),
            joinPath(loaded.repoRoot, "build-portable4/Server.exe"),
            joinPath(loaded.repoRoot, "build-portable3/Server.exe"),
            joinPath(loaded.repoRoot, "build/windows-debug/Debug/Server.exe"),
            joinPath(loaded.repoRoot, "GameRelease/Server/Server.exe")
        });
    }

    const std::string clientOverride = envString("CLIENT_EXECUTABLE_PATH");
    if (!clientOverride.empty()) {
        loaded.clientExecutablePath = resolvePath(currentDirectory(), clientOverride);
    } else {
        loaded.clientExecutablePath = firstExistingPath({
            joinPath(exeDir, "Client.exe"),
            joinPath(currentDirectory(), "Client.exe"),
            joinPath(loaded.repoRoot, "Client.exe"),
            joinPath(loaded.repoRoot, "build-portable4/Client.exe"),
            joinPath(loaded.repoRoot, "build-portable3/Client.exe"),
            joinPath(loaded.repoRoot, "build/windows-debug/Debug/Client.exe"),
            joinPath(loaded.repoRoot, "GameRelease/Client/Client.exe")
        });
    }

    *config = loaded;
    return true;
}

WebsiteApp::WebsiteApp(Config config) : config_(std::move(config)), store_(config_.dataPath) {}

WebsiteApp::~WebsiteApp() {
    stop();
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& entry : instances_) {
        releaseProcess(entry.second);
    }
    instances_.clear();
}

bool WebsiteApp::initialize(std::string* error) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return true;
    }

    if (!config_.dataPath.empty()) {
        makeDirectories(directoryOf(config_.dataPath));
    }
    store_.setPath(config_.dataPath);
    if (!store_.load(error)) {
        return false;
    }
    if (!seedStarterGames(error)) {
        return false;
    }

    server_.setBodyLimitBytes(config_.jsonBodyLimitBytes);
    initialized_ = true;

    if (config_.gameInstanceEmptyGraceMs > 0) {
        tickerRunning_ = true;
        ticker_ = std::thread([this]() {
            while (true) {
                {
                    std::lock_guard<std::mutex> tickLock(mutex_);
                    if (!tickerRunning_) {
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                tickInstances();
            }
        });
    }
    return true;
}

bool WebsiteApp::seedStarterGames(std::string* error) {
    struct Seed {
        std::string title;
        std::string description;
        std::string worldFile;  // relative to the repo or package root
        std::string scriptFile;
    };
    const Seed seeds[] = {
        {"Starter Baseplate",
         "The default shared world, now published as the first public game.",
         "ServerWorld.world", std::string()},
        {"Separate Server Test Arena",
         "A second published world for testing that each game starts its own local server "
         "instance.",
         "web/seed-worlds/separate-server-test.world", std::string()},
        {"Sky Platformer",
         "A scripted platformer with moving platforms, lava and a sky goal, driven by a "
         ".rbx script.",
         "games/sky-platformer.world", "games/sky-platformer.rbx"}
    };

    // Publish every starter game this data file does not have yet, so an install
    // created before a starter game existed picks it up on the next start instead
    // of only a brand new database getting it. Games that are already present are
    // never duplicated or overwritten.
    std::vector<const Seed*> missing;
    for (const Seed& seed : seeds) {
        const std::string slug = slugify(seed.title);
        bool present = false;
        for (const GameRecord* game : store_.allGames()) {
            if (game != nullptr && (game->title == seed.title || game->slug == slug)) {
                present = true;
                break;
            }
        }
        if (!present) {
            missing.push_back(&seed);
        }
    }
    if (missing.empty()) {
        return true;
    }

    int ownerId = 0;
    if (const UserRecord* owner = store_.firstUser()) {
        ownerId = owner->id;
    } else {
        const std::string passwordHash =
            hashPassword("local-starter-account", config_.passwordIterations);
        ownerId = store_.createUser("RBLXCreator", passwordHash, nowIso());
        if (ownerId <= 0) {
            if (error) {
                *error = "Could not create the starter account.";
            }
            return false;
        }
    }

    // Starter assets live next to the sources during development and next to
    // WebApp.exe in a packaged build. The working directory is deliberately not
    // searched: seeding must not depend on where the process was started.
    auto locate = [this](const std::string& relative) {
        return firstExistingPath({
            joinPath(config_.repoRoot, relative),
            joinPath(executableDirectory(), relative)
        });
    };

    AuthUser owner;
    owner.id = ownerId;
    if (const UserRecord* record = store_.findUserById(ownerId)) {
        owner.username = record->username;
    }

    for (const Seed* seed : missing) {
        const std::string worldPath = locate(seed->worldFile);
        if (worldPath.empty()) {
            continue;  // not shipped with this build
        }
        std::string worldText;
        if (!readWholeFile(worldPath, &worldText)) {
            continue;
        }
        Json body = Json::makeObject();
        body.set("title", seed->title);
        body.set("description", seed->description);
        body.set("isPublic", true);
        body.set("worldText", worldText);
        Response ignored;
        GameRecord created;
        if (!createGame(owner, body, &created, ignored)) {
            if (error) {
                *error = "Could not publish the starter world " + worldPath + ".";
            }
            return false;
        }

        if (!seed->scriptFile.empty()) {
            const std::string scriptPath = locate(seed->scriptFile);
            std::string scriptText;
            if (!scriptPath.empty() && readWholeFile(scriptPath, &scriptText)) {
                makeDirectories(config_.gameWorldsDir);
                const std::string scriptFile = joinPath(
                    config_.gameWorldsDir, "game-" + std::to_string(created.id) + ".rbx");
                if (writeWholeFile(scriptFile, scriptText)) {
                    if (const GameRecord* stored = store_.findGame(created.id)) {
                        GameRecord updated = *stored;
                        updated.scriptPath = scriptFile;
                        store_.updateGame(updated);
                        std::string saveError;
                        store_.save(&saveError);
                    }
                }
            }
        }
    }
    return true;
}

void WebsiteApp::collectGameInstances(std::vector<GameInstanceInfo>* out) {
    if (out == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    refreshInstances();
    out->clear();
    for (const auto& entry : instances_) {
        const GameInstance& instance = entry.second;
        GameInstanceInfo info;
        info.id = instance.id;
        info.gameId = instance.gameId;
        if (const GameRecord* game = store_.findGame(instance.gameId)) {
            info.title = game->title;
        }
        if (info.title.empty()) {
            info.title = "Game " + std::to_string(instance.gameId);
        }
        info.status = instance.status;
        info.logPath = instance.logPath;
        info.startedAtMs = instance.startedAtMs;
        info.port = instance.port;
        info.players = instance.playerCount;
        info.pid = instance.processId;
        out->push_back(std::move(info));
    }
    std::sort(out->begin(), out->end(),
              [](const GameInstanceInfo& left, const GameInstanceInfo& right) {
                  return left.port < right.port;
              });
}

bool WebsiteApp::stopGameInstanceById(const std::string& instanceId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (instances_.find(instanceId) == instances_.end()) {
        return false;
    }
    stopInstance(instanceId, "console");
    return true;
}

bool WebsiteApp::start(std::string* error) {
    return server_.listen("0.0.0.0", config_.port, error);
}

void WebsiteApp::run() {
    server_.serveForever();
}

void WebsiteApp::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tickerRunning_ = false;
    }
    if (ticker_.joinable()) {
        ticker_.join();
    }
    server_.stop();
}

bool WebsiteApp::isOriginAllowed(const std::string& origin) const {
    return std::find(config_.corsOrigins.begin(), config_.corsOrigins.end(), origin) !=
           config_.corsOrigins.end();
}

bool WebsiteApp::isRateLimited(const std::string& remoteAddress) {
    const long long now = nowMilliseconds();
    std::vector<long long>& attempts = authAttempts_[remoteAddress];
    while (!attempts.empty() && now - attempts.front() > config_.authRateLimitWindowMs) {
        attempts.erase(attempts.begin());
    }
    if (static_cast<int>(attempts.size()) >= config_.authRateLimitMax) {
        return true;
    }
    attempts.push_back(now);
    return false;
}

HttpServer::Handler WebsiteApp::guard(HttpServer::Handler handler) {
    return [this, handler](Request& request, Response& response) {
        const std::string origin = request.header("origin");
        if (!origin.empty()) {
            const std::string host = request.header("host");
            const bool sameOrigin =
                !host.empty() &&
                (origin == "http://" + host || origin == "https://" + host);
            if (!sameOrigin && !isOriginAllowed(origin)) {
                errorJson(response, 403, "Origin is not allowed.");
                return true;
            }
            response.headers["Access-Control-Allow-Origin"] = origin;
            response.headers["Vary"] = "Origin";
        }

        if (request.method == "OPTIONS") {
            response.status = 204;
            response.contentType = "text/plain; charset=utf-8";
            response.body.clear();
            response.headers["Access-Control-Allow-Methods"] =
                "GET,POST,PUT,PATCH,DELETE,OPTIONS";
            response.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
            return true;
        }

        const bool mutating = request.method == "POST" || request.method == "PUT" ||
                              request.method == "PATCH";
        if (mutating && request.path.compare(0, 5, "/api/") == 0 &&
            !request.contentTypeStartsWith("application/json")) {
            errorJson(response, 415, "Content-Type must be application/json.");
            return true;
        }

        try {
            return handler(request, response);
        } catch (const std::exception& exception) {
            std::fprintf(stderr, "Unhandled server error: %s\n", exception.what());
            errorJson(response, 500, "Server error.");
            return true;
        } catch (...) {
            std::fprintf(stderr, "Unhandled server error.\n");
            errorJson(response, 500, "Server error.");
            return true;
        }
    };
}

bool WebsiteApp::parseJsonBody(const Request& request, Json* body, Response& response) const {
    if (request.body.empty()) {
        *body = Json::makeObject();
        return true;
    }
    std::string parseError;
    const Json parsed = Json::parse(request.body, &parseError);
    if (!parsed.isObject()) {
        errorJson(response, 400, "Malformed JSON body.");
        return false;
    }
    *body = parsed;
    return true;
}

bool WebsiteApp::isAdminUser(const AuthUser& user) const {
    if (user.id <= 0) {
        return false;
    }
    return std::find(config_.adminUsernames.begin(), config_.adminUsernames.end(),
                     user.username) != config_.adminUsernames.end();
}

bool WebsiteApp::requireAuth(const Request& request, Response& response, AuthUser* user) {
    const std::string token = bearerToken(request.header("authorization"));
    if (token.empty()) {
        errorJson(response, 401, "Authorization bearer token is required.");
        return false;
    }
    Json claims;
    if (!verifyJwt(token, config_.jwtSecret, &claims)) {
        errorJson(response, 401, "Invalid or expired token.");
        return false;
    }
    const long long userId = claimUserId(claims);
    if (userId <= 0 || userId > 1000000) {
        errorJson(response, 401, "Invalid or expired token.");
        return false;
    }
    const UserRecord* record = store_.findUserById(static_cast<int>(userId));
    if (record == nullptr) {
        errorJson(response, 401, "Invalid or expired token.");
        return false;
    }
    user->id = record->id;
    user->username = record->username;
    return true;
}

bool WebsiteApp::optionalAuth(const Request& request, AuthUser* user, bool* authenticated) {
    *authenticated = false;
    if (user == nullptr) {
        return true;
    }
    const std::string token = bearerToken(request.header("authorization"));
    if (token.empty()) {
        return true;
    }
    Json claims;
    if (!verifyJwt(token, config_.jwtSecret, &claims)) {
        return true;
    }
    const long long userId = claimUserId(claims);
    const UserRecord* record =
        userId > 0 ? store_.findUserById(static_cast<int>(userId)) : nullptr;
    if (record == nullptr) {
        return true;
    }
    user->id = record->id;
    user->username = record->username;
    *authenticated = true;
    return true;
}

Json WebsiteApp::avatarJson(const Avatar& avatar) const {
    Json result = Json::makeObject();
    for (int slot = 0; slot < kAvatarSlotCount; ++slot) {
        result.set(kAvatarSlots[slot], colorsJson(avatar.colors[slot]));
    }
    result.set("faceId", avatar.faceId);
    return result;
}

Json WebsiteApp::profileJson(int userId, int viewerId) {
    Json profile = Json::makeObject();
    const UserRecord* user = store_.findUserById(userId);
    if (user == nullptr) {
        return Json();
    }
    profile.set("id", user->id);
    profile.set("username", user->username);
    profile.set("createdAt", user->createdAt);
    profile.set("avatar", avatarJson(store_.avatarFor(userId)));

    Json stats = Json::makeObject();
    stats.set("playtimeSeconds", store_.playtimeFor(userId));
    stats.set("coins", store_.coinsFor(userId));
    const std::string lastPlayed = store_.lastPlayedAtFor(userId);
    if (lastPlayed.empty()) {
        stats.setNull("lastPlayedAt");
    } else {
        stats.set("lastPlayedAt", lastPlayed);
    }
    profile.set("stats", stats);

    profile.set("friendCount", store_.countAcceptedFriends(userId));

    std::string relationship = "none";
    if (viewerId == userId) {
        relationship = "self";
    } else if (viewerId > 0) {
        if (const FriendshipRecord* friendship = store_.findFriendship(viewerId, userId)) {
            if (friendship->status == "accepted") {
                relationship = "friends";
            } else {
                relationship = friendship->requesterId == viewerId ? "outgoing" : "incoming";
            }
        }
    }
    profile.set("relationship", relationship);
    return profile;
}

Json WebsiteApp::gameJson(const GameRecord& game, bool canEdit) const {
    Json result = Json::makeObject();
    result.set("id", game.id);
    result.set("ownerId", game.ownerId);
    result.set("ownerUsername", game.ownerUsername);
    result.set("title", game.title);
    result.set("slug", game.slug);
    result.set("description", game.description);
    result.set("isPublic", game.isPublic);
    result.set("iconUrl", game.iconPath.empty() ? std::string(kDefaultGameIcon) : game.iconPath);
    result.set("bannerUrl",
               game.bannerPath.empty() ? std::string(kDefaultGameBanner) : game.bannerPath);

    Json stats = Json::makeObject();
    stats.set("partsCount", game.partsCount);
    stats.set("dynamicPartsCount", game.dynamicPartsCount);
    stats.set("spawnCount", game.spawnCount);
    stats.set("launchCount", game.launchCount);
    result.set("stats", stats);

    result.set("createdAt", game.createdAt);
    result.set("updatedAt", game.updatedAt);
    result.set("publishedAt", game.publishedAt);
    result.set("canEdit", canEdit);
    return result;
}

Json WebsiteApp::instanceJson(const GameInstance& instance) const {
    Json result = Json::makeObject();
    result.set("id", instance.id);
    result.set("host", instance.host);
    result.set("port", instance.port);
    result.set("status", instance.status);
    if (instance.processId > 0) {
        result.set("processId", static_cast<long long>(instance.processId));
    } else {
        result.setNull("processId");
    }
    result.set("startedAt", instance.startedAt);
    result.set("playerCount", instance.playerCount);
    if (instance.lastHeartbeatAt.empty()) {
        result.setNull("lastHeartbeatAt");
    } else {
        result.set("lastHeartbeatAt", instance.lastHeartbeatAt);
    }
    if (instance.emptySince.empty()) {
        result.setNull("emptySince");
    } else {
        result.set("emptySince", instance.emptySince);
    }
    if (!instance.hasShutdownAt) {
        result.setNull("emptyShutdownAt");
    } else {
        result.set("emptyShutdownAt",
                   isoFromTime(static_cast<std::time_t>(instance.emptyShutdownAtMs / 1000)));
    }
    return result;
}

void WebsiteApp::installRoutes() {
    server_.route("POST", "/api/signup", guard([this](Request& request, Response& response) {
                      return handleSignup(request, response);
                  }));
    server_.route("POST", "/api/login", guard([this](Request& request, Response& response) {
                      return handleLogin(request, response);
                  }));
    server_.route("POST", "/api/verify", guard([this](Request& request, Response& response) {
                      return handleVerify(request, response);
                  }));
    server_.route("GET", "/api/me/social", guard([this](Request& request, Response& response) {
                      return handleSocial(request, response);
                  }));
    server_.route("POST", "/api/me/playtime", guard([this](Request& request, Response& response) {
                      return handlePlaytime(request, response);
                  }));
    server_.route("POST", "/api/game-instances/:id/heartbeat",
                  guard([this](Request& request, Response& response) {
                      return handleHeartbeat(request, response);
                  }));
    server_.route("GET", "/api/users/search", guard([this](Request& request, Response& response) {
                      return handleUserSearch(request, response);
                  }));
    server_.route("GET", "/api/users/:id", guard([this](Request& request, Response& response) {
                      return handleUserProfile(request, response);
                  }));
    server_.route("POST", "/api/friends/request",
                  guard([this](Request& request, Response& response) {
                      return handleFriendRequest(request, response);
                  }));
    server_.route("POST", "/api/friends/respond",
                  guard([this](Request& request, Response& response) {
                      return handleFriendRespond(request, response);
                  }));
    server_.route("POST", "/api/friends/remove",
                  guard([this](Request& request, Response& response) {
                      return handleFriendRemove(request, response);
                  }));
    server_.route("GET", "/api/games", guard([this](Request& request, Response& response) {
                      return handleListGames(request, response);
                  }));
    server_.route("GET", "/api/games/mine", guard([this](Request& request, Response& response) {
                      return handleMyGames(request, response);
                  }));
    server_.route("POST", "/api/games", guard([this](Request& request, Response& response) {
                      return handleCreateGame(request, response);
                  }));
    server_.route("POST", "/api/games/publish",
                  guard([this](Request& request, Response& response) {
                      return handlePublishGame(request, response);
                  }));
    server_.route("GET", "/api/games/:id/servers",
                  guard([this](Request& request, Response& response) {
                      return handleGameServers(request, response);
                  }));
    server_.route("GET", "/api/games/:id", guard([this](Request& request, Response& response) {
                      return handleGetGame(request, response);
                  }));
    server_.route("PUT", "/api/games/:id", guard([this](Request& request, Response& response) {
                      return handleUpdateGame(request, response);
                  }));
    server_.route("POST", "/api/games/:id/play",
                  guard([this](Request& request, Response& response) {
                      return handlePlayGame(request, response);
                  }));
    server_.route("POST", "/api/client-reports",
                  guard([this](Request& request, Response& response) {
                      return handleClientReport(request, response);
                  }));
    server_.route("GET", "/api/admin/overview",
                  guard([this](Request& request, Response& response) {
                      return handleAdminOverview(request, response);
                  }));
    server_.route("DELETE", "/api/admin/games/:id",
                  guard([this](Request& request, Response& response) {
                      return handleAdminDeleteGame(request, response);
                  }));
    server_.route("GET", "/api/avatar", guard([this](Request& request, Response& response) {
                      return handleGetAvatar(request, response);
                  }));
    server_.route("POST", "/api/avatar", guard([this](Request& request, Response& response) {
                      return handleSetAvatar(request, response);
                  }));

    server_.setFallback([this](Request& request, Response& response) {
        return handleStatic(request, response);
    });
}

bool WebsiteApp::handleSignup(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (isRateLimited(request.remoteAddress)) {
        errorJson(response, 429, "Too many authentication attempts. Try again later.");
        return true;
    }
    Json body;
    if (!parseJsonBody(request, &body, response)) {
        return true;
    }

    const std::string username = body.member("username").asString();
    const std::string password = body.member("password").asString();
    const std::string validation = validateCredentials(username, password);
    if (!validation.empty()) {
        errorJson(response, 400, validation);
        return true;
    }
    if (store_.findUserByName(username) != nullptr) {
        errorJson(response, 409, "Username already exists.");
        return true;
    }

    const std::string passwordHash = hashPassword(password, config_.passwordIterations);
    const int userId = store_.createUser(username, passwordHash, nowIso());
    if (userId <= 0) {
        errorJson(response, 409, "Username already exists.");
        return true;
    }
    std::string saveError;
    if (!store_.save(&saveError)) {
        std::fprintf(stderr, "Could not save the data file: %s\n", saveError.c_str());
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("token", issueToken(config_, userId, username));
    result.set("userId", userId);
    result.set("username", username);
    sendJson(response, 201, result);
    return true;
}

bool WebsiteApp::handleLogin(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (isRateLimited(request.remoteAddress)) {
        errorJson(response, 429, "Too many authentication attempts. Try again later.");
        return true;
    }
    Json body;
    if (!parseJsonBody(request, &body, response)) {
        return true;
    }

    const std::string username = body.member("username").asString();
    const std::string password = body.member("password").asString();
    const std::string validation = validateLoginInput(username, password);
    if (!validation.empty()) {
        errorJson(response, 400, validation);
        return true;
    }

    const UserRecord* user = store_.findUserByName(username);
    if (user == nullptr || !verifyPassword(password, user->passwordHash)) {
        errorJson(response, 401, "Invalid username or password.");
        return true;
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("token", issueToken(config_, user->id, user->username));
    result.set("userId", user->id);
    result.set("username", user->username);
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleVerify(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    Json body = Json::makeObject();
    if (!request.body.empty()) {
        std::string parseError;
        body = Json::parse(request.body, &parseError);
        if (!body.isObject()) {
            errorJson(response, 400, "Malformed JSON body.");
            return true;
        }
    }

    std::string token = bearerToken(request.header("authorization"));
    if (token.empty() && body.member("token").isString()) {
        response.headers["Deprecation"] = "true";
        token = body.member("token").asString();
    }
    if (token.empty()) {
        errorJson(response, 401, "Authorization bearer token is required.");
        return true;
    }

    Json claims;
    if (!verifyJwt(token, config_.jwtSecret, &claims)) {
        errorJson(response, 401, "Invalid or expired token.");
        return true;
    }
    const long long userId = claimUserId(claims);
    const UserRecord* user =
        userId > 0 ? store_.findUserById(static_cast<int>(userId)) : nullptr;
    if (user == nullptr) {
        errorJson(response, 401, "Invalid or expired token.");
        return true;
    }

    AuthUser authUser;
    authUser.id = user->id;
    authUser.username = user->username;

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("userId", user->id);
    result.set("username", user->username);
    result.set("coins", store_.coinsFor(user->id));
    result.set("isAdmin", isAdminUser(authUser));
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleSocial(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("isAdmin", isAdminUser(user));
    result.set("profile", profileJson(user.id, user.id));

    Json friends = Json::makeArray();
    for (const FriendshipRecord* record : store_.acceptedFriendships(user.id, 48)) {
        const int friendId =
            record->requesterId == user.id ? record->addresseeId : record->requesterId;
        friends.push(profileJson(friendId, user.id));
    }
    result.set("friends", friends);

    Json incoming = Json::makeArray();
    for (const FriendshipRecord* record : store_.pendingFriendships(user.id, true)) {
        Json profile = profileJson(record->requesterId, user.id);
        profile.set("relationship", "incoming");
        incoming.push(profile);
    }
    result.set("incomingRequests", incoming);

    Json outgoing = Json::makeArray();
    for (const FriendshipRecord* record : store_.pendingFriendships(user.id, false)) {
        Json profile = profileJson(record->addresseeId, user.id);
        profile.set("relationship", "outgoing");
        outgoing.push(profile);
    }
    result.set("outgoingRequests", outgoing);

    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handlePlaytime(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }
    Json body;
    if (!parseJsonBody(request, &body, response)) {
        return true;
    }

    const long long seconds = clampPlaytimeSeconds(body.member("seconds"));
    if (seconds <= 0) {
        errorJson(response, 400, "Playtime seconds must be a positive number.");
        return true;
    }

    store_.addPlaytime(user.id, seconds, nowIso());
    std::string saveError;
    store_.save(&saveError);

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("profile", profileJson(user.id, user.id));
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleHeartbeat(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    const std::string instanceId = request.param("id");
    auto found = instances_.find(instanceId);
    if (found == instances_.end()) {
        errorJson(response, 404, "Game instance not found.");
        return true;
    }

    GameInstance& instance = found->second;
    const std::string managerToken = bearerToken(request.header("authorization"));
    if (managerToken.empty() || managerToken != instance.managerToken) {
        errorJson(response, 403, "Game instance token is invalid.");
        return true;
    }

    Json body;
    if (!parseJsonBody(request, &body, response)) {
        return true;
    }
    const int playerCount = parsePlayerCount(body.member("playerCount"));
    if (playerCount < 0) {
        errorJson(response, 400, "Player count must be a number from 0 to 200.");
        return true;
    }

    const long long now = nowMilliseconds();
    instance.playerCount = playerCount;
    instance.lastHeartbeatAt = nowIso();
    instance.lastHeartbeatAtMs = now;
    instance.staleHeartbeatWarned = false;
    if (playerCount > 0) {
        if (!instance.manual) {
            instance.status = "running";
        }
        instance.emptySince.clear();
        instance.hasShutdownAt = false;
        instance.hadPlayer = true;
    } else {
        if (!instance.manual && instance.status == "starting" && processRunning(instance)) {
            instance.status = "running";
        }
        if (instance.emptySince.empty()) {
            instance.emptySince = instance.lastHeartbeatAt;
            instance.emptySinceMs = now;
        }
        // Before the first player joins, an empty count only means the client has
        // not finished connecting, so the launch grace is used instead.
        const long long grace = instance.hadPlayer ? config_.gameInstanceEmptyGraceMs
                                                   : config_.gameInstanceLaunchGraceMs;
        const long long elapsed = now - instance.emptySinceMs;
        const long long delay = std::max<long long>(0, grace - elapsed);
        instance.hasShutdownAt = true;
        instance.emptyShutdownAtMs = now + delay;
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("server", instanceJson(instance));
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleUserSearch(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }

    std::string query = trimCopy(request.queryParam("q"));
    if (query.size() > 30) {
        query.resize(30);
    }

    Json users = Json::makeArray();
    for (const UserRecord* record : store_.searchUsers(query, 12)) {
        if (record->id == user.id) {
            continue;
        }
        users.push(profileJson(record->id, user.id));
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("query", query);
    result.set("users", users);
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleUserProfile(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }

    const int userId = parseUserId(request.param("id"));
    if (userId <= 0) {
        errorJson(response, 400, "User ID must be a positive number.");
        return true;
    }
    if (store_.findUserById(userId) == nullptr) {
        errorJson(response, 404, "User not found.");
        return true;
    }

    Json friends = Json::makeArray();
    for (const FriendshipRecord* record : store_.acceptedFriendships(userId, 24)) {
        const int friendId =
            record->requesterId == userId ? record->addresseeId : record->requesterId;
        friends.push(profileJson(friendId, user.id));
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("profile", profileJson(userId, user.id));
    result.set("friends", friends);
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleFriendRequest(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }
    Json body;
    if (!parseJsonBody(request, &body, response)) {
        return true;
    }

    const int targetUserId = parseUserIdValue(body.member("userId"));
    if (targetUserId <= 0 || targetUserId == user.id) {
        errorJson(response, 400, "Choose another user to add as a friend.");
        return true;
    }
    if (store_.findUserById(targetUserId) == nullptr) {
        errorJson(response, 404, "User not found.");
        return true;
    }

    if (const FriendshipRecord* existing = store_.findFriendship(user.id, targetUserId)) {
        const int existingId = existing->id;
        const bool iAmRequester = existing->requesterId == user.id;
        if (existing->status == "accepted") {
            errorJson(response, 409, "You are already friends.");
            return true;
        }
        if (iAmRequester) {
            Json result = Json::makeObject();
            result.set("success", true);
            result.set("relationship", "outgoing");
            result.set("profile", profileJson(targetUserId, user.id));
            sendJson(response, 200, result);
            return true;
        }
        store_.setFriendshipStatus(existingId, "accepted", nowIso());
        std::string saveError;
        store_.save(&saveError);
        Json result = Json::makeObject();
        result.set("success", true);
        result.set("relationship", "friends");
        result.set("profile", profileJson(targetUserId, user.id));
        sendJson(response, 200, result);
        return true;
    }

    store_.addFriendship(user.id, targetUserId, nowIso());
    std::string saveError;
    store_.save(&saveError);

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("relationship", "outgoing");
    result.set("profile", profileJson(targetUserId, user.id));
    sendJson(response, 201, result);
    return true;
}

bool WebsiteApp::handleFriendRespond(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }
    Json body;
    if (!parseJsonBody(request, &body, response)) {
        return true;
    }

    const int requesterId = parseUserIdValue(body.member("userId"));
    const std::string action = lowerCopy(trimCopy(body.member("action").asString()));
    if (requesterId <= 0 || (action != "accept" && action != "decline")) {
        errorJson(response, 400, "Friend response requires a requester and action.");
        return true;
    }

    const FriendshipRecord* pending = nullptr;
    for (const FriendshipRecord* record : store_.pendingFriendships(user.id, true)) {
        if (record->requesterId == requesterId) {
            pending = record;
            break;
        }
    }
    if (pending == nullptr) {
        errorJson(response, 404, "Friend request not found.");
        return true;
    }

    const int friendshipId = pending->id;
    if (action == "accept") {
        store_.setFriendshipStatus(friendshipId, "accepted", nowIso());
    } else {
        store_.removeFriendship(friendshipId);
    }
    std::string saveError;
    store_.save(&saveError);

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("relationship", action == "accept" ? "friends" : "none");
    result.set("profile", profileJson(requesterId, user.id));
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleFriendRemove(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }
    Json body;
    if (!parseJsonBody(request, &body, response)) {
        return true;
    }

    const int targetUserId = parseUserIdValue(body.member("userId"));
    if (targetUserId <= 0 || targetUserId == user.id) {
        errorJson(response, 400, "Choose another user.");
        return true;
    }
    if (store_.findUserById(targetUserId) == nullptr) {
        errorJson(response, 404, "User not found.");
        return true;
    }

    store_.removeFriendshipsBetween(user.id, targetUserId);
    std::string saveError;
    store_.save(&saveError);

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("relationship", "none");
    result.set("profile", profileJson(targetUserId, user.id));
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleListGames(Request& request, Response& response) {
    (void)request;
    std::lock_guard<std::mutex> lock(mutex_);
    Json games = Json::makeArray();
    for (const GameRecord* record : store_.publicGames(60)) {
        games.push(gameJson(*record, false));
    }
    Json result = Json::makeObject();
    result.set("success", true);
    result.set("games", games);
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleMyGames(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }

    Json games = Json::makeArray();
    for (const GameRecord* record : store_.gamesOwnedBy(user.id)) {
        games.push(gameJson(*record, true));
    }
    Json result = Json::makeObject();
    result.set("success", true);
    result.set("games", games);
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleCreateGame(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }
    Json body;
    if (!parseJsonBody(request, &body, response)) {
        return true;
    }

    GameRecord created;
    if (!createGame(user, body, &created, response)) {
        return true;
    }
    Json result = Json::makeObject();
    result.set("success", true);
    result.set("game", gameJson(created, true));
    sendJson(response, 201, result);
    return true;
}

bool WebsiteApp::handlePublishGame(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }
    Json body;
    if (!parseJsonBody(request, &body, response)) {
        return true;
    }

    const int gameId = parseUserIdValue(body.member("gameId"));
    GameRecord saved;
    if (gameId > 0) {
        if (!updateGameRecord(user, gameId, body, &saved, response)) {
            return true;
        }
    } else {
        if (!createGame(user, body, &saved, response)) {
            return true;
        }
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("game", gameJson(saved, true));
    sendJson(response, gameId > 0 ? 200 : 201, result);
    return true;
}

bool WebsiteApp::handleGetGame(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser viewer;
    bool authenticated = false;
    optionalAuth(request, &viewer, &authenticated);

    const int gameId = parseUserId(request.param("id"));
    if (gameId <= 0) {
        errorJson(response, 400, "Game ID must be a positive number.");
        return true;
    }
    const GameRecord* game = store_.findGame(gameId);
    if (game == nullptr || (!game->isPublic && (!authenticated || viewer.id != game->ownerId))) {
        errorJson(response, 404, "Game not found.");
        return true;
    }

    Json servers = Json::makeArray();
    for (const std::string& instanceId : activeInstanceIds(gameId)) {
        const auto found = instances_.find(instanceId);
        if (found != instances_.end()) {
            servers.push(instanceJson(found->second));
        }
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("game", gameJson(*game, authenticated && viewer.id == game->ownerId));
    result.set("servers", servers);
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleUpdateGame(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }
    Json body;
    if (!parseJsonBody(request, &body, response)) {
        return true;
    }

    const int gameId = parseUserId(request.param("id"));
    if (gameId <= 0) {
        errorJson(response, 400, "Game ID must be a positive number.");
        return true;
    }

    GameRecord updated;
    if (!updateGameRecord(user, gameId, body, &updated, response)) {
        return true;
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("game", gameJson(updated, true));
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleGameServers(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser viewer;
    bool authenticated = false;
    optionalAuth(request, &viewer, &authenticated);

    const int gameId = parseUserId(request.param("id"));
    const GameRecord* game = gameId > 0 ? store_.findGame(gameId) : nullptr;
    if (game == nullptr || (!game->isPublic && (!authenticated || viewer.id != game->ownerId))) {
        errorJson(response, 404, "Game not found.");
        return true;
    }

    Json servers = Json::makeArray();
    for (const std::string& instanceId : activeInstanceIds(gameId)) {
        const auto found = instances_.find(instanceId);
        if (found != instances_.end()) {
            servers.push(instanceJson(found->second));
        }
    }
    Json result = Json::makeObject();
    result.set("success", true);
    result.set("servers", servers);
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handlePlayGame(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }

    const int gameId = parseUserId(request.param("id"));
    const GameRecord* game = gameId > 0 ? store_.findGame(gameId) : nullptr;
    if (game == nullptr || (!game->isPublic && game->ownerId != user.id)) {
        errorJson(response, 404, "Game not found.");
        return true;
    }

    Json body = Json::makeObject();
    if (!request.body.empty()) {
        std::string parseError;
        const Json parsed = Json::parse(request.body, &parseError);
        if (!parsed.isObject()) {
            errorJson(response, 400, "Malformed JSON body.");
            return true;
        }
        body = parsed;
    }

    std::string instanceError;
    GameInstance* instance =
        ensureInstance(*game, boolValue(body.member("newServer"), false), &instanceError);
    if (instance == nullptr) {
        errorJson(response, 500, instanceError.empty() ? "Server error." : instanceError);
        return true;
    }

    // The server process was only just spawned; give it a moment to bind its port
    // so the client started below has something to connect to.
    const std::string instanceId = instance->id;
    const int instancePort = instance->port;
    std::string failureDetail;
    // The port only becomes busy once the server has loaded its world and its
    // scripts, so this wait now covers the whole start-up rather than just the
    // socket creation. A long world needs more than the old ten seconds.
    if (!waitForGameServer(*instance, 30000, &failureDetail)) {
        if (failureDetail.empty()) {
            failureDetail = "The game server did not start listening on port " +
                            std::to_string(instancePort) + ".";
        }
        std::fprintf(stderr, "Game server for game %d failed: %s\n", gameId,
                     failureDetail.c_str());
        stopInstance(instanceId, "unreachable");
        errorJson(response, 503, failureDetail);
        return true;
    }

    reserveInstanceForLaunch(*instance);
    store_.incrementLaunchCount(gameId);
    std::string saveError;
    store_.save(&saveError);

    const GameRecord* refreshed = store_.findGame(gameId);
    const GameInstance snapshot = *instance;

    std::string webServerUrl = "http://" + request.header("host");
    if (request.header("host").empty()) {
        webServerUrl = "http://localhost:" + std::to_string(server_.port());
    }
    const std::string clientPath =
        config_.clientExecutablePath.empty() ? std::string("Client.exe")
                                             : config_.clientExecutablePath;
    const std::vector<std::string> args = {
        "--server", snapshot.host,
        "--port", std::to_string(snapshot.port),
        "--web", webServerUrl,
        "--token", bearerToken(request.header("authorization")),
        "--game", std::to_string(gameId),
        "--connect"
    };

    Json launch = Json::makeObject();
    launch.set("host", snapshot.host);
    launch.set("port", snapshot.port);
    launch.set("webServerUrl", webServerUrl);
    launch.set(
        "protocolUrl",
        "rblxclone://play?gameId=" + percentEncode(std::to_string(gameId)) + "&host=" +
            percentEncode(snapshot.host) + "&port=" + percentEncode(std::to_string(snapshot.port)) +
            "&web=" + percentEncode(webServerUrl) + "&token=" +
            percentEncode(bearerToken(request.header("authorization"))));
    std::string command = "\"" + clientPath + "\"";
    for (const std::string& arg : args) {
        command += " \"" + arg + "\"";
    }
    launch.set("command", command);
    Json launchArgs = Json::makeArray();
    for (const std::string& arg : args) {
        launchArgs.push(Json(arg));
    }
    launch.set("args", launchArgs);

    Json player = Json::makeObject();
    const bool launchClientValue = body.has("launchClient")
                                       ? boolValue(body.member("launchClient"), true)
                                       : true;
    const bool clientSkipped =
        body.has("launchClient") && body.member("launchClient").isBool() && !launchClientValue;
    if (clientSkipped) {
        player.set("launched", false);
        player.set("reason", "Client launch was skipped.");
    } else if (clientPath.empty() || !fileExists(clientPath)) {
        player.set("launched", false);
        player.set("reason", "Client executable was not found.");
    } else {
        SpawnedProcess spawned;
        std::string spawnError;
        const std::string clientDirectory =
            preferredClientWorkingDirectory(clientPath, config_.repoRoot);
        if (spawnProcess(clientPath, args, clientDirectory, &spawned, &spawnError)) {
            // CreateProcess/fork only reports that the image was started, and the
            // client does work before main() (the anti-cheat TLS callback). A client
            // that dies there used to be reported as "player opening", so the page
            // promised a window that never appeared. Give it a moment and check.
            bool exitedEarly = false;
            int exitCode = 0;
#ifdef _WIN32
            if (spawned.handle != nullptr) {
                const HANDLE handle = static_cast<HANDLE>(spawned.handle);
                if (::WaitForSingleObject(handle, 1200) != WAIT_TIMEOUT) {
                    DWORD code = 0;
                    if (::GetExitCodeProcess(handle, &code) != 0) {
                        exitCode = static_cast<int>(code);
                    }
                    exitedEarly = true;
                }
            }
#else
            if (spawned.pid > 0) {
                ::usleep(1200 * 1000);
                int status = 0;
                const pid_t result = ::waitpid(static_cast<pid_t>(spawned.pid), &status, WNOHANG);
                if (result > 0) {
                    exitedEarly = true;
                    if (WIFEXITED(status)) {
                        exitCode = WEXITSTATUS(status);
                    } else if (WIFSIGNALED(status)) {
                        exitCode = 128 + WTERMSIG(status);
                    }
                }
            }
#endif
            if (exitedEarly) {
                player.set("launched", false);
                player.set("exitedEarly", true);
                player.set("exitCode", exitCode);
                player.set("reason", "The player started and then exited immediately (exit code " +
                                        std::to_string(exitCode) + ")." +
                                        clientLogTail(clientDirectory));
            } else {
                player.set("launched", true);
                player.set("processId", static_cast<long long>(spawned.pid));
            }
        } else {
            player.set("launched", false);
            player.set("reason", spawnError);
        }
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("game", gameJson(refreshed != nullptr ? *refreshed : *game, false));
    result.set("server", instanceJson(snapshot));
    result.set("launch", launch);
    result.set("player", player);
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleClientReport(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    Json body;
    if (!parseJsonBody(request, &body, response)) {
        return true;
    }

    std::string reason = body.member("reason").asString();
    if (reason.empty()) {
        errorJson(response, 400, "A reason is required.");
        return true;
    }
    if (reason.size() > 600) {
        reason.resize(600);
    }

    ClientReport report;
    report.createdAt = nowIso();
    report.stage = trimCopy(body.member("stage").asString());
    if (report.stage.empty()) {
        report.stage = "unknown";
    }
    report.reason = reason;
    report.serverAddress = trimCopy(body.member("serverAddress").asString());
    const long long gameId = body.member("gameId").asInteger();
    report.gameId = gameId > 0 && gameId <= 1000000 ? static_cast<int>(gameId) : 0;

    // A client may be signed in or a guest, so the token only decides which
    // account the report is filed under; it never blocks the report itself.
    AuthUser user;
    bool authenticated = false;
    optionalAuth(request, &user, &authenticated);
    report.userId = authenticated ? user.id : -1;
    report.username = authenticated ? user.username : std::string("guest");

    constexpr std::size_t kMaxClientReports = 50;
    if (clientReports_.size() >= kMaxClientReports) {
        clientReports_.pop_front();
    }
    clientReports_.push_back(report);

    std::fprintf(stdout, "  client problem [%s] %s: %s\n", report.username.c_str(),
                 report.stage.c_str(), report.reason.c_str());

    Json result = Json::makeObject();
    result.set("success", true);
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleAdminOverview(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }
    if (!isAdminUser(user)) {
        errorJson(response, 403, "Admin access is required.");
        return true;
    }

    refreshInstances();
    const std::vector<const GameRecord*> games = store_.allGames();
    int publicGames = 0;
    long long launchCount = 0;
    for (const GameRecord* game : games) {
        if (game->isPublic) {
            ++publicGames;
        }
        launchCount += game->launchCount;
    }

    long long playtimeSeconds = 0;
    Json users = Json::makeArray();
    for (const UserRecord& record : store_.users()) {
        const long long seconds = store_.playtimeFor(record.id);
        playtimeSeconds += seconds;

        AuthUser listed;
        listed.id = record.id;
        listed.username = record.username;

        Json entry = Json::makeObject();
        entry.set("id", record.id);
        entry.set("username", record.username);
        entry.set("createdAt", record.createdAt);
        entry.set("playtimeSeconds", seconds);
        entry.set("coins", store_.coinsFor(record.id));
        entry.set("friendCount", store_.countAcceptedFriends(record.id));
        entry.set("gameCount", static_cast<long long>(store_.gamesOwnedBy(record.id).size()));
        entry.set("isAdmin", isAdminUser(listed));
        users.push(entry);
    }

    Json stats = Json::makeObject();
    stats.set("userCount", static_cast<long long>(store_.users().size()));
    stats.set("gameCount", static_cast<long long>(games.size()));
    stats.set("publicGameCount", publicGames);
    stats.set("instanceCount", static_cast<long long>(instances_.size()));
    stats.set("playtimeSeconds", playtimeSeconds);
    stats.set("launchCount", launchCount);

    Json gameList = Json::makeArray();
    for (const GameRecord* game : games) {
        gameList.push(gameJson(*game, true));
    }

    Json instanceList = Json::makeArray();
    for (const auto& entry : instances_) {
        instanceList.push(instanceJson(entry.second));
    }

    Json reportList = Json::makeArray();
    for (auto entry = clientReports_.rbegin(); entry != clientReports_.rend(); ++entry) {
        Json report = Json::makeObject();
        report.set("createdAt", entry->createdAt);
        report.set("stage", entry->stage);
        report.set("reason", entry->reason);
        report.set("serverAddress", entry->serverAddress);
        report.set("gameId", entry->gameId);
        report.set("userId", entry->userId);
        report.set("username", entry->username);
        reportList.push(report);
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("stats", stats);
    result.set("users", users);
    result.set("games", gameList);
    result.set("instances", instanceList);
    result.set("clientReports", reportList);
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleAdminDeleteGame(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }
    if (!isAdminUser(user)) {
        errorJson(response, 403, "Admin access is required.");
        return true;
    }

    const int gameId = parseUserId(request.param("id"));
    const GameRecord* game = gameId > 0 ? store_.findGame(gameId) : nullptr;
    if (game == nullptr) {
        errorJson(response, 404, "Game not found.");
        return true;
    }

    const std::string worldPath = game->worldPath;
    std::vector<std::string> doomedInstances;
    for (const auto& entry : instances_) {
        if (entry.second.gameId == gameId) {
            doomedInstances.push_back(entry.first);
        }
    }
    for (const std::string& instanceId : doomedInstances) {
        stopInstance(instanceId, "admin");
    }

    if (!store_.removeGame(gameId)) {
        errorJson(response, 500, "Could not delete the game.");
        return true;
    }
    std::string saveError;
    if (!store_.save(&saveError)) {
        errorJson(response, 500, saveError.empty() ? "Could not save the change." : saveError);
        return true;
    }
    if (!worldPath.empty()) {
        std::remove(worldPath.c_str());
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("deletedGameId", gameId);
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleGetAvatar(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("avatar", avatarJson(store_.avatarFor(user.id)));
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleSetAvatar(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }
    Json body;
    if (!parseJsonBody(request, &body, response)) {
        return true;
    }

    Avatar avatar;
    if (!sanitizeAvatar(body.member("avatar"), &avatar)) {
        errorJson(response, 400,
                  "Avatar data must contain six RGB arrays with values from 0 to 1.");
        return true;
    }

    store_.setAvatar(user.id, avatar);
    std::string saveError;
    store_.save(&saveError);

    Json result = Json::makeObject();
    result.set("success", true);
    sendJson(response, 200, result);
    return true;
}

bool WebsiteApp::handleStatic(Request& request, Response& response) {
    if (request.method != "GET") {
        return false;
    }
    std::string relative = request.path;
    if (relative.empty()) {
        relative = "/";
    }
    if (relative.find("..") != std::string::npos || relative.find('\\') != std::string::npos) {
        return false;
    }

    if (relative == "/") {
        relative = "/index.html";
    }

    if (relative.compare(0, 14, "/vendor/three/") == 0) {
        const std::string rest = relative.substr(14);
        const std::string vendored =
            joinPath(joinPath(joinPath(joinPath(config_.webRoot, "public"), "vendor"), "three"),
                     rest);
        if (fileExists(vendored)) {
            response.file(200, vendored, HttpServer::mimeTypeForPath(vendored));
            return true;
        }
        return false;
    }

    const std::string publicFile = joinPath(joinPath(config_.webRoot, "public"), relative);
    if (fileExists(publicFile)) {
        response.file(200, publicFile, HttpServer::mimeTypeForPath(publicFile));
        return true;
    }

    const std::string page = pageFileForPath(request.path);
    if (!page.empty()) {
        const std::string pagePath = joinPath(joinPath(config_.webRoot, "public"), page);
        if (fileExists(pagePath)) {
            response.file(200, pagePath, "text/html; charset=utf-8");
            return true;
        }
    }
    return false;
}

bool WebsiteApp::createGame(const AuthUser& user, const Json& body, GameRecord* created,
                            Response& response) {
    const std::string title = normalizeGameTitle(body.member("title").asString());
    if (title.empty()) {
        errorJson(response, 400, "Game title must be 3-60 characters.");
        return false;
    }

    std::string worldText = body.member("worldText").asString();
    if (worldText.empty()) {
        worldText = body.member("world").asString();
    }
    WorldAnalysis analysis;
    std::string worldError;
    if (!analyzeWorldSource(worldText, &analysis, &worldError)) {
        errorJson(response, 400, worldError);
        return false;
    }

    GameRecord record;
    record.ownerId = user.id;
    record.ownerUsername = user.username;
    record.title = title;
    record.slug = slugify(title);
    record.description = normalizeGameDescription(body.member("description").asString());
    record.isPublic = boolValue(body.member("isPublic"), true);
    record.iconPath = body.member("iconUrl").asString(kDefaultGameIcon);
    if (record.iconPath.empty()) {
        record.iconPath = kDefaultGameIcon;
    }
    record.bannerPath = body.member("bannerUrl").asString(kDefaultGameBanner);
    if (record.bannerPath.empty()) {
        record.bannerPath = kDefaultGameBanner;
    }
    record.partsCount = analysis.partsCount;
    record.dynamicPartsCount = analysis.dynamicPartsCount;
    record.spawnCount = analysis.spawnCount;
    record.createdAt = nowIso();
    record.updatedAt = record.createdAt;
    record.publishedAt = record.createdAt;

    const int gameId = store_.addGame(record);
    if (gameId <= 0) {
        errorJson(response, 500, "Server error.");
        return false;
    }
    const std::string worldPath = saveWorldFile(gameId, analysis.source);
    const GameRecord* stored = store_.findGame(gameId);
    if (stored == nullptr) {
        errorJson(response, 500, "Server error.");
        return false;
    }
    GameRecord updated = *stored;
    updated.worldPath = worldPath;
    store_.updateGame(updated);
    std::string saveError;
    store_.save(&saveError);

    if (const GameRecord* refreshed = store_.findGame(gameId)) {
        *created = *refreshed;
    }
    return true;
}

bool WebsiteApp::updateGameRecord(const AuthUser& user, int gameId, const Json& body,
                                  GameRecord* updated, Response& response) {
    const GameRecord* existing = store_.findGame(gameId);
    if (existing == nullptr || existing->ownerId != user.id) {
        errorJson(response, 404, "Game not found.");
        return false;
    }

    GameRecord record = *existing;
    if (body.has("title")) {
        const std::string title = normalizeGameTitle(body.member("title").asString());
        if (title.empty()) {
            errorJson(response, 400, "Game title must be 3-60 characters.");
            return false;
        }
        record.title = title;
        record.slug = slugify(title);
    }

    if (body.has("description")) {
        record.description = normalizeGameDescription(body.member("description").asString());
    }
    if (body.has("isPublic")) {
        record.isPublic = boolValue(body.member("isPublic"), record.isPublic);
    }
    if (body.has("iconUrl")) {
        const std::string icon = body.member("iconUrl").asString();
        record.iconPath = icon.empty() ? std::string(kDefaultGameIcon) : icon;
    }
    if (body.has("bannerUrl")) {
        const std::string banner = body.member("bannerUrl").asString();
        record.bannerPath = banner.empty() ? std::string(kDefaultGameBanner) : banner;
    }

    if (body.has("worldText") || body.has("world")) {
        std::string worldText = body.member("worldText").asString();
        if (worldText.empty()) {
            worldText = body.member("world").asString();
        }
        WorldAnalysis analysis;
        std::string worldError;
        if (!analyzeWorldSource(worldText, &analysis, &worldError)) {
            errorJson(response, 400, worldError);
            return false;
        }
        record.worldPath = saveWorldFile(gameId, analysis.source);
        record.partsCount = analysis.partsCount;
        record.dynamicPartsCount = analysis.dynamicPartsCount;
        record.spawnCount = analysis.spawnCount;
    }

    record.updatedAt = nowIso();
    record.publishedAt = record.updatedAt;
    store_.updateGame(record);
    std::string saveError;
    store_.save(&saveError);

    if (const GameRecord* refreshed = store_.findGame(gameId)) {
        *updated = *refreshed;
    }
    return true;
}

std::string WebsiteApp::saveWorldFile(int gameId, const std::string& contents) {
    makeDirectories(config_.gameWorldsDir);
    const std::string filename =
        "game-" + std::to_string(gameId) + "-" + std::to_string(nowMilliseconds()) + ".world";
    const std::string path = joinPath(config_.gameWorldsDir, filename);
    if (!writeWholeFile(path, contents)) {
        return std::string();
    }
    return path;
}

// Server.cpp binds the wildcard address with a datagram socket, so probe the
// same thing. Binding 127.0.0.1 instead answers a different question: on Windows
// a specific-address bind can succeed next to an existing wildcard listener,
// which reports a port held by a stale Server.exe as free and then the new
// server fails to bind. The protocol has to match too: a TCP bind succeeds next
// to a running UDP game server, which would report every busy port as free.
bool WebsiteApp::isPortFreeForGameServer(int port) {
#ifdef _WIN32
    const SOCKET handle = ::socket(AF_INET, SOCK_DGRAM, 0);
    const bool invalid = handle == INVALID_SOCKET;
#else
    const int handle = ::socket(AF_INET, SOCK_DGRAM, 0);
    const bool invalid = handle < 0;
#endif
    if (invalid) {
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<unsigned short>(port));
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    const int result = ::bind(handle, reinterpret_cast<sockaddr*>(&address), sizeof(address));
#ifdef _WIN32
    closesocket(handle);
#else
    ::close(handle);
#endif
    return result == 0;
}

int WebsiteApp::findAvailablePort(int startPort) {
    for (int offset = 0; offset < 200; ++offset) {
        const int port = startPort + offset;
        bool used = false;
        for (const auto& entry : instances_) {
            if (entry.second.port == port) {
                used = true;
                break;
            }
        }
        if (used) {
            continue;
        }
        if (isPortFreeForGameServer(port)) {
            return port;
        }
    }
    return 0;
}

// Spawning the game server only queues the process; it still has to start up,
// initialise physics and bind its port. Launching the client in that window is
// why a fresh Play reports "Could not connect to the game server", so wait until
// the port stops being bindable, which is when the server owns it.
//
// A server that dies immediately (a busy port, a missing world file) is reported
// straight away together with its own output, so the caller can show the reason.
bool WebsiteApp::waitForGameServer(GameInstance& instance, int timeoutMs,
                                   std::string* failureDetail) {
    // A manual instance has no process of ours behind it; it points at a server
    // somebody else started, so there is nothing to wait for.
    if (instance.manual) {
        return true;
    }
    // Readiness is the server's own announcement, not a busy port. Polling the
    // port let any leftover process that still held it pass as a running server,
    // so Play launched the client against a server that was not reading its
    // socket: the client sat on its sky-blue clear colour until the join deadline
    // and then dropped back to the menu.
    const std::string readyMarker = "listening on port " + std::to_string(instance.port);
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    for (;;) {
        const std::string output = readLogTail(instance.logPath, 8000);
        if (output.find(readyMarker) != std::string::npos) {
            return true;
        }
        if (output.find("Bind failed") != std::string::npos) {
            if (failureDetail) {
                *failureDetail = "The game server could not bind port " +
                                 std::to_string(instance.port) +
                                 ", so another program already holds it." +
                                 describeGameServerLog(instance);
            }
            return false;
        }
        if (!processRunning(instance)) {
            std::string message =
                "The game server stopped before it began listening on port " +
                std::to_string(instance.port);
            if (instance.exitCodeKnown) {
                message += " (exit code " + std::to_string(instance.exitCode) + ")";
            }
            message += ".";
            if (failureDetail) {
                *failureDetail = message + describeGameServerLog(instance);
            }
            return false;
        }
        if (std::chrono::steady_clock::now() >= deadline) {
            if (failureDetail) {
                *failureDetail = "The game server did not report that it was listening on port " +
                                 std::to_string(instance.port) + " within " +
                                 std::to_string(timeoutMs / 1000) +
                                 " seconds. If its output stops before that line, the installed "
                                 "Server.exe is an older build." +
                                 describeGameServerLog(instance);
            }
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
}

std::string WebsiteApp::describeGameServerLog(const GameInstance& instance) const {
    std::string detail;
    const std::string tail = readLogTail(instance.logPath, 2000);
    if (!tail.empty()) {
        detail += "\n\nGame server output:\n" + tail;
    }
    if (!instance.logPath.empty()) {
        detail += "\n\nLog file: " + instance.logPath;
    }
    return detail;
}

bool WebsiteApp::spawnProcess(const std::string& executable, const std::vector<std::string>& args,
                              const std::string& workingDirectory, SpawnedProcess* spawned,
                              std::string* error, const std::string& stdoutLogPath,
                              bool showConsole) {
    if (executable.empty()) {
        if (error) {
            *error = "Executable path is empty.";
        }
        return false;
    }

#ifdef _WIN32
    std::string commandLine = quoteWindowsArgument(executable);
    for (const std::string& arg : args) {
        commandLine += " ";
        commandLine += quoteWindowsArgument(arg);
    }
    std::vector<char> mutableCommand(commandLine.begin(), commandLine.end());
    mutableCommand.push_back('\0');

    STARTUPINFOA startup{};
    startup.cb = sizeof(startup);

    HANDLE logHandle = INVALID_HANDLE_VALUE;
    HANDLE nullInput = INVALID_HANDLE_VALUE;
    // A process with its own console cannot inherit a redirected log handle, so a
    // visible server is handed --log and writes that file itself. Its directory is
    // created either way: the file is wanted whether we redirect or the child writes.
    if (!stdoutLogPath.empty()) {
        makeDirectories(directoryOf(stdoutLogPath));
    }
    if (!stdoutLogPath.empty() && !showConsole) {
        logHandle = ::CreateFileA(stdoutLogPath.c_str(), FILE_APPEND_DATA,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL, nullptr);
        if (logHandle == INVALID_HANDLE_VALUE) {
            if (error) {
                *error = "Could not open " + stdoutLogPath + " (error " +
                         std::to_string(::GetLastError()) + ").";
            }
            return false;
        }
        // With STARTF_USESTDHANDLES every standard handle must be valid, and the
        // child's stdin is not redirected, so point it at NUL.
        nullInput = ::CreateFileA("NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  nullptr, OPEN_EXISTING, 0, nullptr);
        startup.dwFlags |= STARTF_USESTDHANDLES;
        startup.hStdInput = nullInput;
        startup.hStdOutput = logHandle;
        startup.hStdError = logHandle;
    }

    PROCESS_INFORMATION information{};
    const DWORD creationFlags = showConsole ? CREATE_NEW_CONSOLE : CREATE_NO_WINDOW;
    const BOOL created = ::CreateProcessA(
        executable.c_str(), mutableCommand.data(), nullptr, nullptr,
        logHandle == INVALID_HANDLE_VALUE ? FALSE : TRUE, creationFlags, nullptr,
        workingDirectory.empty() ? nullptr : workingDirectory.c_str(), &startup, &information);
    if (logHandle != INVALID_HANDLE_VALUE) {
        ::CloseHandle(logHandle);
    }
    if (nullInput != INVALID_HANDLE_VALUE) {
        ::CloseHandle(nullInput);
    }
    if (created == 0) {
        if (error) {
            *error = "Could not start " + executable + " (error " +
                     std::to_string(::GetLastError()) + ").";
        }
        return false;
    }
    ::CloseHandle(information.hThread);
    spawned->pid = static_cast<long>(information.dwProcessId);
    spawned->handle = static_cast<void*>(information.hProcess);
    return true;
#else
    if (!stdoutLogPath.empty()) {
        makeDirectories(directoryOf(stdoutLogPath));
    }
    const pid_t pid = ::fork();
    if (pid < 0) {
        if (error) {
            *error = "Could not fork a child process.";
        }
        return false;
    }
    if (pid == 0) {
        ::setsid();
        if (!stdoutLogPath.empty() && !showConsole) {
            const int logFd = ::open(stdoutLogPath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
            if (logFd >= 0) {
                ::dup2(logFd, STDOUT_FILENO);
                ::dup2(logFd, STDERR_FILENO);
                if (logFd > STDERR_FILENO) {
                    ::close(logFd);
                }
            }
        }
        if (!workingDirectory.empty() && ::chdir(workingDirectory.c_str()) != 0) {
            _exit(127);
        }
        std::vector<std::string> storage;
        storage.push_back(executable);
        for (const std::string& arg : args) {
            storage.push_back(arg);
        }
        std::vector<char*> argv;
        argv.reserve(storage.size() + 1);
        for (std::string& item : storage) {
            argv.push_back(const_cast<char*>(item.c_str()));
        }
        argv.push_back(nullptr);
        ::execv(executable.c_str(), argv.data());
        _exit(127);
    }
    spawned->pid = static_cast<long>(pid);
    spawned->handle = nullptr;
    return true;
#endif
}

bool WebsiteApp::processRunning(GameInstance& instance) {
#ifdef _WIN32
    if (instance.processHandle == nullptr) {
        return false;
    }
    const HANDLE handle = static_cast<HANDLE>(instance.processHandle);
    if (::WaitForSingleObject(handle, 0) == WAIT_TIMEOUT) {
        return true;
    }
    DWORD code = 0;
    if (::GetExitCodeProcess(handle, &code) != 0) {
        instance.exitCode = static_cast<int>(code);
        instance.exitCodeKnown = true;
    }
    return false;
#else
    if (instance.processId <= 0) {
        return false;
    }
    int status = 0;
    const pid_t result = ::waitpid(static_cast<pid_t>(instance.processId), &status, WNOHANG);
    if (result == 0) {
        return true;
    }
    if (result > 0) {
        instance.exitCodeKnown = true;
        if (WIFEXITED(status)) {
            instance.exitCode = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            instance.exitCode = 128 + WTERMSIG(status);
        }
    }
    return false;
#endif
}

void WebsiteApp::releaseProcess(GameInstance& instance) {
#ifdef _WIN32
    if (instance.processHandle != nullptr) {
        ::CloseHandle(static_cast<HANDLE>(instance.processHandle));
        instance.processHandle = nullptr;
    }
#else
    (void)instance;
#endif
    instance.processId = 0;
}

void WebsiteApp::refreshInstances() {
    for (auto it = instances_.begin(); it != instances_.end();) {
        GameInstance& instance = it->second;
        if (!instance.manual && !processRunning(instance)) {
            // A game server that ends on its own used to disappear from the table
            // without a word, so Play reported a timeout (or the next Play started a
            // fresh server) and nothing said whether the old one had crashed. Report
            // the exit code and where its output stopped instead.
            std::string message = "Game server for game " + std::to_string(instance.gameId) +
                                  " on port " + std::to_string(instance.port) + " exited";
            if (instance.exitCodeKnown) {
                message += " (exit code " + std::to_string(instance.exitCode) + ")";
            }
            message += ".";
            std::fprintf(stderr, "%s\n", (message + describeGameServerLog(instance)).c_str());
            releaseProcess(instance);
            it = instances_.erase(it);
            continue;
        }
        if (!instance.manual && instance.status == "starting" && processRunning(instance)) {
            instance.status = "running";
        }
        ++it;
    }
}

std::vector<std::string> WebsiteApp::activeInstanceIds(int gameId) {
    refreshInstances();
    std::vector<std::string> ids;
    for (const auto& entry : instances_) {
        if (entry.second.gameId == gameId) {
            ids.push_back(entry.first);
        }
    }
    return ids;
}

WebsiteApp::GameInstance* WebsiteApp::ensureInstance(const GameRecord& game, bool forceNew,
                                                     std::string* error) {
    refreshInstances();
    if (!forceNew) {
        for (auto& entry : instances_) {
            if (entry.second.gameId == game.id) {
                return &entry.second;
            }
        }
    }

    const int port = findAvailablePort(config_.gameServerBasePort);
    if (port <= 0) {
        if (error) {
            *error = "No local game server ports are available.";
        }
        return nullptr;
    }

    GameInstance instance;
    instance.id = randomUuid();
    instance.gameId = game.id;
    instance.host = config_.gameServerHost;
    instance.port = port;
    instance.status = "manual";
    instance.manual = true;
    instance.startedAt = nowIso();
    instance.startedAtMs = nowMilliseconds();
    instance.managerToken = randomHex(24);

    if (!config_.serverExecutablePath.empty() && fileExists(config_.serverExecutablePath) &&
        !game.worldPath.empty()) {
        instance.manual = false;
        instance.logPath = joinPath(directoryOf(config_.gameWorldsDir),
                                    "game-server-logs/game-server-" + std::to_string(port) +
                                        ".log");
        std::vector<std::string> args = {
            "--port", std::to_string(port),
            "--world", game.worldPath,
            "--web", config_.publicBaseUrl,
            "--instance", instance.id,
            "--instance-token", instance.managerToken
        };
        // The game server runs with its own console window so its output is visible
        // while it runs, so it writes the log itself instead of having it redirected.
        args.push_back("--log");
        args.push_back(instance.logPath);
        if (!game.scriptPath.empty()) {
            args.push_back("--script");
            args.push_back(game.scriptPath);
        }
        SpawnedProcess spawned;
        std::string spawnError;
        if (spawnProcess(config_.serverExecutablePath, args, config_.repoRoot, &spawned,
                         &spawnError, instance.logPath, true)) {
            instance.processId = spawned.pid;
#ifdef _WIN32
            instance.processHandle = spawned.handle;
#endif
            instance.status = "starting";
            std::fprintf(stdout, "Started game server for game %d on port %d (pid %ld)\n",
                         game.id, port, spawned.pid);
            std::fprintf(stdout, "  server log: %s\n", instance.logPath.c_str());
        } else {
            instance.status = "error";
            std::fprintf(stderr, "Could not start a game server: %s\n", spawnError.c_str());
        }
        std::string command = "\"" + config_.serverExecutablePath + "\"";
        for (const std::string& arg : args) {
            command += " \"" + arg + "\"";
        }
        instance.command = command;
    } else {
        instance.command = "Server.exe --port " + std::to_string(port) + " --world \"" +
                           game.worldPath + "\" --web \"" + config_.publicBaseUrl + "\"";
    }

    auto inserted = instances_.emplace(instance.id, instance);
    return &inserted.first->second;
}

void WebsiteApp::reserveInstanceForLaunch(GameInstance& instance) {
    if (instance.manual || instance.playerCount > 0) {
        return;
    }
    instance.emptySince = nowIso();
    instance.emptySinceMs = nowMilliseconds();
    instance.lastHeartbeatAtMs = instance.emptySinceMs;
    instance.hasShutdownAt = true;
    const long long grace = instance.hadPlayer ? config_.gameInstanceEmptyGraceMs
                                               : config_.gameInstanceLaunchGraceMs;
    instance.emptyShutdownAtMs = instance.emptySinceMs + grace;
}

void WebsiteApp::stopInstance(const std::string& instanceId, const std::string& reason) {
    const auto found = instances_.find(instanceId);
    if (found == instances_.end()) {
        return;
    }
    GameInstance& instance = found->second;
    instance.status = "stopping";
    std::fprintf(stdout, "Stopped game server for game %d on port %d (%s)\n",
                 instance.gameId, instance.port, reason.empty() ? "requested" : reason.c_str());

    if (processRunning(instance)) {
#ifdef _WIN32
        ::TerminateProcess(static_cast<HANDLE>(instance.processHandle), 0);
#else
        ::kill(static_cast<pid_t>(instance.processId), SIGTERM);
#endif
    }
    releaseProcess(instance);
    instances_.erase(found);
}

void WebsiteApp::tickInstances() {
    std::lock_guard<std::mutex> lock(mutex_);
    refreshInstances();
    const long long now = nowMilliseconds();
    // The reaper only acts on evidence: if the server has stopped reporting, the
    // player count it saw last is stale and may belong to a server somebody is
    // playing on, so it is left alone rather than killed on a guess.
    const long long heartbeatStaleMs = 30 * 1000;
    std::vector<std::string> expired;
    for (auto& entry : instances_) {
        GameInstance& instance = entry.second;
        if (!instance.hasShutdownAt || instance.playerCount > 0) {
            continue;
        }
        if (now < instance.emptyShutdownAtMs) {
            continue;
        }
        if (instance.lastHeartbeatAtMs > 0 && now - instance.lastHeartbeatAtMs > heartbeatStaleMs) {
            if (!instance.staleHeartbeatWarned) {
                instance.staleHeartbeatWarned = true;
                std::fprintf(stderr,
                             "Game server for game %d on port %d stopped reporting players, so it "
                             "is being left running instead of shut down on a guess.\n",
                             instance.gameId, instance.port);
            }
            continue;
        }
        expired.push_back(instance.id);
    }
    for (const std::string& instanceId : expired) {
        stopInstance(instanceId, "empty");
    }
}

}  // namespace web
}  // namespace rblx
