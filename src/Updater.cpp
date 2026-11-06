#include "Updater.h"

#include "WebCrypto.h"
#include "WebJson.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <set>
#include <system_error>

namespace rblx {
namespace update {
namespace {

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool isHex64(const std::string& text) {
    if (text.size() != 64) return false;
    for (char c : text) {
        const bool digit = c >= '0' && c <= '9';
        const bool lower = c >= 'a' && c <= 'f';
        if (!digit && !lower) return false;
    }
    return true;
}

// "12-beta" reads as 12, "x" and "" as 0: the numeric part is the version, the
// rest is a label.
long long componentValue(const std::string& text) {
    long long value = 0;
    bool seen = false;
    for (char c : text) {
        if (c >= '0' && c <= '9') {
            value = value * 10 + (c - '0');
            seen = true;
            if (value > 1000000000LL) value = 1000000000LL;
        } else if (seen) {
            break;
        } else if (!std::isspace(static_cast<unsigned char>(c))) {
            break;
        }
    }
    return value;
}

std::vector<long long> versionComponents(const std::string& version) {
    std::string text = version;
    while (!text.empty() && (text.front() == 'v' || text.front() == 'V')) text.erase(text.begin());
    std::vector<long long> parts;
    std::string current;
    for (char c : text) {
        if (c == '.') {
            parts.push_back(componentValue(current));
            current.clear();
        } else {
            current += c;
        }
    }
    parts.push_back(componentValue(current));
    while (!parts.empty() && parts.back() == 0) parts.pop_back();
    return parts;
}

}  // namespace

int compareVersions(const std::string& left, const std::string& right) {
    const std::vector<long long> a = versionComponents(left);
    const std::vector<long long> b = versionComponents(right);
    const std::size_t count = std::max(a.size(), b.size());
    for (std::size_t i = 0; i < count; ++i) {
        const long long value = i < a.size() ? a[i] : 0;
        const long long other = i < b.size() ? b[i] : 0;
        if (value < other) return -1;
        if (value > other) return 1;
    }
    return 0;
}

bool isSafeRelativePath(const std::string& path, std::string* reason) {
    const auto fail = [reason](const char* why) {
        if (reason) *reason = why;
        return false;
    };

    if (path.empty()) return fail("the path is empty");
    if (path.size() > 512) return fail("the path is longer than 512 characters");
    if (path.front() == '/') return fail("an absolute path cannot be installed");
    if (path.find('\\') != std::string::npos) return fail("a manifest path uses forward slashes");
    if (path.find(':') != std::string::npos) return fail("a drive or stream name is not a path");
    for (char c : path) {
        if (static_cast<unsigned char>(c) < 32) return fail("the path contains a control character");
    }

    std::string segment;
    for (char c : path) {
        if (c != '/') {
            segment += c;
            continue;
        }
        if (segment.empty()) return fail("the path has an empty segment");
        if (segment == "." || segment == "..") return fail("a path may not leave the install folder");
        segment.clear();
    }
    if (segment.empty()) return fail("the path ends with a separator");
    if (segment == "." || segment == "..") return fail("a path may not leave the install folder");
    return true;
}

bool resolveUnder(const std::string& root, const std::string& relative, std::string* resolved) {
    if (!isSafeRelativePath(relative, nullptr)) return false;

    std::error_code error;
    std::filesystem::path rootPath = std::filesystem::weakly_canonical(root, error);
    if (error) return false;
    const std::filesystem::path candidate =
        std::filesystem::weakly_canonical(rootPath / relative, error);
    if (error) return false;

    // A symlink inside the folder can point anywhere, so compare the resolved
    // paths rather than the strings the manifest gave.
    std::string prefix = rootPath.generic_string();
    if (!prefix.empty() && prefix.back() != '/') prefix += '/';
    const std::string inside = candidate.generic_string();
    if (inside.size() < prefix.size() || inside.compare(0, prefix.size(), prefix) != 0) {
        return false;
    }
    if (resolved) *resolved = inside;
    return true;
}

std::string sha256Text(const std::string& text) {
    return web::hexEncode(web::sha256(text));
}

std::string sha256File(const std::string& path, std::string* error) {
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        if (error) *error = "could not open " + path;
        return std::string();
    }
    std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (file.bad()) {
        if (error) *error = "could not read " + path;
        return std::string();
    }
    return sha256Text(data);
}

std::string joinUrl(const std::string& baseUrl, const std::string& relative) {
    if (baseUrl.empty() || relative.empty()) return std::string();
    const std::string probe = lowercase(relative);
    if (probe.rfind("http://", 0) == 0 || probe.rfind("https://", 0) == 0) return std::string();

    std::string base = baseUrl;
    while (!base.empty() && base.back() == '/') base.pop_back();

    std::string encoded;
    encoded.reserve(relative.size());
    for (char c : relative) {
        const unsigned char value = static_cast<unsigned char>(c);
        if (c == '/') {
            encoded += '/';
        } else if (std::isalnum(value) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else {
            char escape[4] = {'%', 0, 0, 0};
            std::snprintf(escape, sizeof(escape), "%%%02X", value);
            encoded += escape;
        }
    }
    return base + "/" + encoded;
}

bool isPackagedFile(const std::string& relativePath) {
    std::string path = relativePath;
    std::replace(path.begin(), path.end(), '\\', '/');
    while (!path.empty() && path.front() == '/') path.erase(path.begin());
    if (path.empty()) return false;
    if (path.rfind(".update-staging/", 0) == 0) return false;

    std::string base = path;
    const std::size_t slash = base.find_last_of('/');
    if (slash != std::string::npos) base = base.substr(slash + 1);
    const std::string name = lowercase(base);

    if (name == "manifest.json" || name == "update.json") return false;
    // The updater cannot replace the copy that is running it.
    if (name == "rblxupdater" || name == "rblxupdater.exe") return false;
    if (name.size() > 4 && name.compare(name.size() - 4, 4, ".old") == 0) return false;
    if (name.size() > 4 && name.compare(name.size() - 4, 4, ".tmp") == 0) return false;
    return true;
}

bool collectPayloadFiles(const std::string& directory, std::vector<std::string>* out,
                         std::string* error) {
    std::error_code code;
    if (!std::filesystem::is_directory(directory, code)) {
        if (error) *error = "there is no folder at " + directory;
        return false;
    }

    std::vector<std::string> found;
    for (std::filesystem::recursive_directory_iterator
             it(directory, std::filesystem::directory_options::skip_permission_denied, code),
         end;
         it != end; it.increment(code)) {
        if (code) {
            if (error) *error = "could not list " + directory + ": " + code.message();
            return false;
        }
        if (!it->is_regular_file(code)) continue;

        const std::string relative =
            std::filesystem::relative(it->path(), directory, code).generic_string();
        if (code) {
            if (error) *error = "could not describe " + it->path().string();
            return false;
        }
        if (!isPackagedFile(relative)) continue;
        found.push_back(relative);
    }

    // Sorted, so two runs over the same folder produce the same document and a
    // release can be compared line by line.
    std::sort(found.begin(), found.end());
    if (out) *out = std::move(found);
    return true;
}

bool buildManifestFromDirectory(const std::string& directory, const std::string& version,
                                const std::string& channel, Manifest* out, std::string* error) {
    Manifest manifest;
    manifest.version = version.empty() ? std::string("0.0.0") : version;
    manifest.channel = channel.empty() ? std::string("stable") : channel;

    std::vector<std::string> paths;
    if (!collectPayloadFiles(directory, &paths, error)) return false;

    for (const std::string& relative : paths) {
        const std::string path = (std::filesystem::path(directory) / relative).string();
        FileEntry entry;
        entry.path = relative;
        std::string digestError;
        entry.sha256 = sha256File(path, &digestError);
        if (entry.sha256.empty()) {
            if (error) *error = digestError;
            return false;
        }

        std::error_code code;
        const auto size = std::filesystem::file_size(path, code);
        if (code) {
            if (error) *error = "could not measure " + path;
            return false;
        }
        entry.size = static_cast<long long>(size);
        manifest.files.push_back(entry);
    }

    if (out) *out = manifest;
    return true;
}

bool parseManifest(const std::string& text, Manifest* out, std::string* error) {
    if (!out) return false;

    std::string parseError;
    const web::Json root = web::Json::parse(text, &parseError);
    if (!parseError.empty()) {
        if (error) *error = "the manifest is not valid JSON: " + parseError;
        return false;
    }
    if (!root.isObject()) {
        if (error) *error = "the manifest must be a JSON object";
        return false;
    }

    const web::Json& version = root.member("version");
    if (!version.isString() || version.asString().empty()) {
        if (error) *error = "the manifest has no version";
        return false;
    }

    Manifest manifest;
    manifest.version = version.asString();
    const web::Json& channel = root.member("channel");
    if (channel.isString() && !channel.asString().empty()) manifest.channel = channel.asString();

    const web::Json& files = root.member("files");
    if (!files.isArray()) {
        if (error) *error = "the manifest has no file list";
        return false;
    }

    std::set<std::string> seen;
    for (const web::Json& item : files.elements()) {
        if (!item.isObject()) {
            if (error) *error = "a file entry is not an object";
            return false;
        }

        FileEntry entry;
        entry.path = item.member("path").asString();
        std::string reason;
        if (!isSafeRelativePath(entry.path, &reason)) {
            if (error) *error = "the manifest names \"" + entry.path + "\": " + reason;
            return false;
        }
        if (!isPackagedFile(entry.path)) continue;

        entry.sha256 = lowercase(item.member("sha256").asString());
        if (!isHex64(entry.sha256)) {
            if (error) *error = "the manifest has no SHA-256 for \"" + entry.path + "\"";
            return false;
        }
        entry.size = static_cast<long long>(item.member("size").asInteger(0));
        if (entry.size < 0) entry.size = 0;

        if (!seen.insert(entry.path).second) {
            if (error) *error = "the manifest lists \"" + entry.path + "\" twice";
            return false;
        }
        manifest.files.push_back(entry);
    }

    *out = manifest;
    return true;
}

std::string buildManifestJson(const Manifest& manifest) {
    web::Json root = web::Json::makeObject();
    root.set("version", manifest.version);
    root.set("channel", manifest.channel);

    web::Json files = web::Json::makeArray();
    for (const FileEntry& file : manifest.files) {
        web::Json entry = web::Json::makeObject();
        entry.set("path", file.path);
        entry.set("sha256", file.sha256);
        entry.set("size", file.size);
        files.push(entry);
    }
    root.set("files", files);
    return root.dump();
}

Plan planUpdate(const Manifest& target, const Manifest& installed, const std::string& directory) {
    Plan plan;
    std::set<std::string> targetPaths;

    for (const FileEntry& file : target.files) {
        if (!isPackagedFile(file.path)) continue;
        targetPaths.insert(file.path);

        std::string resolved;
        if (!resolveUnder(directory, file.path, &resolved)) {
            plan.download.push_back(file);
            plan.downloadBytes += file.size;
            continue;
        }

        std::error_code error;
        const auto size = std::filesystem::file_size(resolved, error);
        if (!error && static_cast<long long>(size) == file.size) {
            std::string digestError;
            if (sha256File(resolved, &digestError) == file.sha256) {
                plan.current.push_back(file.path);
                continue;
            }
        }

        plan.download.push_back(file);
        plan.downloadBytes += file.size;
    }

    for (const FileEntry& file : installed.files) {
        if (targetPaths.count(file.path)) continue;
        if (!isPackagedFile(file.path)) continue;
        if (!isSafeRelativePath(file.path, nullptr)) continue;
        plan.remove.push_back(file.path);
    }

    return plan;
}

bool readTextFile(const std::string& path, std::string* out) {
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open()) return false;
    std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (file.bad()) return false;
    if (out) *out = data;
    return true;
}

bool writeFileAtomically(const std::string& path, const std::string& data, std::string* error) {
    std::error_code code;
    const std::filesystem::path target(path);
    if (target.has_parent_path()) {
        std::filesystem::create_directories(target.parent_path(), code);
    }

    const std::string temporary = path + ".tmp";
    {
        std::ofstream file(temporary.c_str(), std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            if (error) *error = "could not create " + temporary;
            return false;
        }
        file.write(data.data(), static_cast<std::streamsize>(data.size()));
        file.flush();
        if (!file.good()) {
            if (error) *error = "could not write " + temporary;
            return false;
        }
    }

    std::filesystem::remove(path, code);
    std::filesystem::rename(temporary, path, code);
    if (code) {
        std::filesystem::remove(temporary, code);
        if (error) *error = "could not write " + path + ": " + code.message();
        return false;
    }
    return true;
}

bool applyStagedFile(const std::string& staged, const std::string& target, std::string* error) {
    std::error_code code;
    const std::filesystem::path targetPath(target);
    if (targetPath.has_parent_path()) {
        std::filesystem::create_directories(targetPath.parent_path(), code);
    }

    const std::string backup = target + ".old";
    std::filesystem::remove(backup, code);
    const bool existed = std::filesystem::exists(target, code);
    if (existed) {
        std::filesystem::rename(target, backup, code);
        if (code) {
            if (error) {
                *error = "could not move the old " + target + " aside (" + code.message() +
                         "); close the game and run the updater again";
            }
            return false;
        }
    }

    std::error_code renameError;
    std::filesystem::rename(staged, target, renameError);
    if (renameError) {
        if (existed) {
            std::error_code ignored;
            std::filesystem::rename(backup, target, ignored);
        }
        if (error) {
            *error = "could not replace " + target + " (" + renameError.message() +
                     "); close the game and run the updater again";
        }
        return false;
    }

    std::filesystem::remove(backup, code);
    return true;
}

bool readManifestFile(const std::string& path, Manifest* out, std::string* error) {
    std::string text;
    if (!readTextFile(path, &text)) {
        if (error) *error = "could not read " + path;
        return false;
    }
    return parseManifest(text, out, error);
}

bool writeManifestFile(const std::string& path, const Manifest& manifest, std::string* error) {
    return writeFileAtomically(path, buildManifestJson(manifest) + "\n", error);
}

bool Config::read(const std::string& text, std::string* error) {
    std::string parseError;
    const web::Json root = web::Json::parse(text, &parseError);
    if (!parseError.empty()) {
        if (error) *error = "update.json is not valid JSON: " + parseError;
        return false;
    }
    if (!root.isObject()) {
        if (error) *error = "update.json must be a JSON object";
        return false;
    }

    const std::string url = root.member("manifestUrl").asString();
    if (url.empty()) {
        if (error) *error = "update.json has no manifestUrl";
        return false;
    }
    manifestUrl = url;

    const std::string program = root.member("launch").asString();
    if (!program.empty()) launch = program;

    const std::string lower = lowercase(url);
    if (lower.rfind("https://", 0) == 0) {
        if (error) {
            *error = "update.json uses https, which this build cannot speak: the updater "
                     "has no TLS stack, so point it at an http:// address";
        }
        return false;
    }
    return true;
}

std::string Config::toJson() const {
    web::Json root = web::Json::makeObject();
    root.set("manifestUrl", manifestUrl);
    root.set("launch", launch);
    return root.dump() + "\n";
}

bool readConfigFile(const std::string& path, Config* out, std::string* error) {
    std::string text;
    if (!readTextFile(path, &text)) {
        if (error) *error = "no update.json in " + path;
        return false;
    }
    Config config;
    if (!config.read(text, error)) return false;
    if (out) *out = config;
    return true;
}

bool writeConfigFile(const std::string& path, const Config& config, std::string* error) {
    return writeFileAtomically(path, config.toJson(), error);
}

}  // namespace update
}  // namespace rblx
