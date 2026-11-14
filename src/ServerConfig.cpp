#include "ServerConfig.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace rblx {
namespace server {
namespace {

namespace fs = std::filesystem;

std::string trim(const std::string& text) {
    std::size_t start = 0;
    while (start < text.size() && (text[start] == ' ' || text[start] == '\t' || text[start] == '\r')) {
        ++start;
    }
    std::size_t end = text.size();
    while (end > start && (text[end - 1] == ' ' || text[end - 1] == '\t' || text[end - 1] == '\r')) {
        --end;
    }
    return text.substr(start, end - start);
}

std::string upperCopy(const std::string& text) {
    std::string out = text;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return out;
}

// A key is applied to the matching field only; anything else is left alone, which
// is how a file written for a later version still loads here.
bool assignKey(const std::string& key, const std::string& value, Settings* settings) {
    const std::string name = upperCopy(key);
    if (name == "NODE_ENV") {
        settings->nodeEnv = value;
    } else if (name == "PORT") {
        settings->port = value;
    } else if (name == "PUBLIC_BASE_URL") {
        settings->publicBaseUrl = value;
    } else if (name == "ADMIN_USERNAMES") {
        settings->adminUsernames = value;
    } else if (name == "STARTER_PASSWORD") {
        settings->starterPassword = value;
    } else if (name == "JWT_SECRET") {
        settings->jwtSecret = value;
    } else if (name == "DATA_KEY") {
        settings->dataKey = value;
    } else {
        return false;
    }
    return true;
}

bool digitsOnly(const std::string& text) {
    if (text.empty()) return false;
    for (char c : text) {
        if (c < '0' || c > '9') return false;
    }
    return true;
}

// Only a plain http(s) host:port, because the client sends its session token to
// whatever origin it reads here. A path, a query or a user name in the URL would
// either be dropped silently or be a way to point the client somewhere else, so
// each of them is refused out loud instead.
bool isPlainBaseUrl(const std::string& value, std::string* error) {
    std::string rest;
    const std::string lowered = [&] {
        std::string out = value;
        std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return out;
    }();
    if (lowered.compare(0, 7, "http://") == 0) {
        rest = value.substr(7);
    } else if (lowered.compare(0, 8, "https://") == 0) {
        rest = value.substr(8);
    } else {
        if (error) *error = "PUBLIC_BASE_URL must start with http:// or https://";
        return false;
    }
    if (rest.empty()) {
        if (error) *error = "PUBLIC_BASE_URL has no host in it";
        return false;
    }
    for (char c : rest) {
        const bool allowed = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
                             (c >= 'A' && c <= 'Z') || c == '.' || c == '-' || c == ':' ||
                             c == '[' || c == ']';
        if (!allowed) {
            if (error) {
                *error = "PUBLIC_BASE_URL must be a plain host:port; \"" + value +
                         "\" has a path, a query or a character that does not belong in one";
            }
            return false;
        }
    }
    return true;
}

bool isBlank(const std::string& text) { return trim(text).empty(); }

}  // namespace

bool loadSettings(const std::string& path, Settings* settings, std::string* error) {
    if (settings == nullptr) {
        if (error) *error = "no settings to fill in";
        return false;
    }
    std::ifstream stream(path.c_str(), std::ios::binary);
    if (!stream.good()) {
        if (error) *error = "there is no settings file at " + path;
        return false;
    }

    Settings loaded;
    std::string line;
    int lineNumber = 0;
    while (std::getline(stream, line)) {
        ++lineNumber;
        const std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') continue;

        const std::size_t equals = trimmed.find('=');
        if (equals == std::string::npos) {
            if (error) {
                *error = path + ":" + std::to_string(lineNumber) +
                         ": expected key = value, found no '='";
            }
            return false;
        }
        const std::string key = trim(trimmed.substr(0, equals));
        if (key.empty()) {
            if (error) {
                *error = path + ":" + std::to_string(lineNumber) + ": the key before '=' is empty";
            }
            return false;
        }
        const std::string value = trim(trimmed.substr(equals + 1));
        assignKey(key, value, &loaded);
    }

    *settings = loaded;
    return true;
}

bool saveSettings(const std::string& path, const Settings& settings, std::string* error) {
    std::ostringstream out;
    out << "# RBLX Clone server settings, written by ServerSetup.exe.\n"
           "#\n"
           "# Every key here is an environment variable the website reads, so this file is a\n"
           "# convenience rather than a second configuration system: a value set in the\n"
           "# environment wins, and deleting a line falls back to the site's own default.\n"
           "#\n"
           "# This file holds the owner password and the secrets the data file is sealed with.\n"
           "# Keep it off shared folders and out of backups other people can read: whoever\n"
           "# reads it can sign in as the owner account.\n"
           "\n";
    for (const std::pair<std::string, std::string>& entry : settingsAsEnvironment(settings)) {
        out << entry.first << " = " << entry.second << "\n";
    }

    const fs::path target(path);
    const fs::path temporary(path + ".tmp");
    std::error_code code;
    if (!target.parent_path().empty()) {
        fs::create_directories(target.parent_path(), code);
        if (code && !fs::exists(target.parent_path())) {
            if (error) *error = "could not create " + target.parent_path().string() + ": " +
                                code.message();
            return false;
        }
    }

    {
        std::ofstream stream(temporary.string().c_str(), std::ios::binary | std::ios::trunc);
        if (!stream.good()) {
            if (error) *error = "could not write " + temporary.string();
            return false;
        }
        stream << out.str();
        stream.flush();
        if (!stream.good()) {
            if (error) *error = "could not write " + temporary.string();
            return false;
        }
    }

    // Renamed over the old file rather than written into it, so an interrupted run
    // leaves the settings that were working before. Losing this file loses the key
    // the account data is sealed with.
    fs::rename(temporary, target, code);
    if (code) {
        fs::remove(temporary, code);
        if (error) *error = "could not replace " + target.string() + ": " + code.message();
        return false;
    }
    return true;
}

bool validateSettings(const Settings& settings, std::string* error) {
    if (settings.nodeEnv != "production" && settings.nodeEnv != "development") {
        if (error) {
            *error = "NODE_ENV must be production or development; the file says \"" +
                     settings.nodeEnv + "\"";
        }
        return false;
    }

    if (!digitsOnly(settings.port)) {
        if (error) *error = "PORT must be a number between 1 and 65535";
        return false;
    }
    const long port = std::strtol(settings.port.c_str(), nullptr, 10);
    if (port < 1 || port > 65535) {
        if (error) *error = "PORT must be between 1 and 65535";
        return false;
    }

    if (isBlank(settings.jwtSecret) || settings.jwtSecret.size() < 32) {
        if (error) {
            *error = "JWT_SECRET must be at least 32 characters: it signs every session the "
                     "site hands out";
        }
        return false;
    }

    // Twelve characters is what WebApp itself insists on in production, because
    // this password opens the seeded owner account.
    if (isBlank(settings.starterPassword) || settings.starterPassword.size() < 12) {
        if (error) {
            *error = "STARTER_PASSWORD must be at least 12 characters: it is the password of "
                     "the owner account";
        }
        return false;
    }

    if (!settings.publicBaseUrl.empty() && !isPlainBaseUrl(settings.publicBaseUrl, error)) {
        return false;
    }
    return true;
}

bool checkServerFolder(const std::string& folder, std::string* error) {
    if (folder.empty()) {
        if (error) *error = "no folder to set up";
        return false;
    }
    const fs::path root(folder);
    std::error_code code;
    const fs::path page = root / "web" / "public" / "index.html";
    if (!fs::is_regular_file(page, code)) {
        if (error) {
            *error = "there is no website in " + folder +
                     ": it needs web/public/index.html, so point this at the folder WebApp.exe is "
                     "in";
        }
        return false;
    }
    const fs::path program = root / "WebApp.exe";
    if (!fs::is_regular_file(program, code)) {
        if (error) {
            *error = "there is no WebApp.exe in " + folder +
                     " to start; the settings could be written, but nothing would read them";
        }
        return false;
    }
    return true;
}

std::vector<std::pair<std::string, std::string>> settingsAsEnvironment(const Settings& settings) {
    std::vector<std::pair<std::string, std::string>> entries;
    const std::pair<const char*, const std::string*> pairs[] = {
        {"NODE_ENV", &settings.nodeEnv},
        {"PORT", &settings.port},
        {"PUBLIC_BASE_URL", &settings.publicBaseUrl},
        {"ADMIN_USERNAMES", &settings.adminUsernames},
        {"STARTER_PASSWORD", &settings.starterPassword},
        {"JWT_SECRET", &settings.jwtSecret},
        {"DATA_KEY", &settings.dataKey},
    };
    for (const auto& entry : pairs) {
        if (entry.second->empty()) continue;
        entries.emplace_back(entry.first, *entry.second);
    }
    return entries;
}

std::string settingsPathFor(const std::string& directory) {
    if (directory.empty()) return kSettingsFileName;
    const char last = directory.back();
    if (last == '\\' || last == '/') return directory + kSettingsFileName;
    return directory + "/" + kSettingsFileName;
}

}  // namespace server
}  // namespace rblx
