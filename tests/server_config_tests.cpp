// ServerConfigTests: the server.conf document, without a server.
//
// The file is what ServerSetup.exe writes and WebApp.exe reads, so the checks are
// about the things that would be silent failures in the field: a secret that does
// not survive the round trip, a comment read as a key, a value cut at a space, or
// a settings document that validates and then will not start.

#include "ServerConfig.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

namespace server = rblx::server;
namespace fs = std::filesystem;

namespace {

int g_failures = 0;

void check(bool ok, const std::string& what) {
    if (!ok) {
        std::printf("FAIL %s\n", what.c_str());
        ++g_failures;
    }
}

server::Settings sample() {
    server::Settings settings;
    settings.nodeEnv = "production";
    settings.port = "8080";
    settings.publicBaseUrl = "http://example.com:8080";
    settings.adminUsernames = "owner,helper";
    settings.starterPassword = "correct horse battery";
    settings.jwtSecret = "0123456789abcdef0123456789abcdef";
    settings.dataKey = "00112233445566778899aabbccddeeff";
    return settings;
}

std::string writeFile(const fs::path& path, const std::string& text) {
    std::ofstream stream(path.string().c_str(), std::ios::binary | std::ios::trunc);
    stream << text;
    stream.close();
    return path.string();
}

std::string readFile(const fs::path& path) {
    std::ifstream stream(path.string().c_str(), std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
}

}  // namespace

int main() {
    const fs::path root = fs::temp_directory_path() / "rblx-server-config-tests";
    std::error_code code;
    fs::remove_all(root, code);
    fs::create_directories(root, code);

    // A saved file reads back as the same settings, secrets and all.
    {
        const fs::path path = root / "round-trip.conf";
        const server::Settings written = sample();
        std::string error;
        check(server::saveSettings(path.string(), written, &error),
              "save writes the file: " + error);

        server::Settings read;
        check(server::loadSettings(path.string(), &read, &error), "load reads it back: " + error);
        check(read.nodeEnv == written.nodeEnv, "NODE_ENV round trips");
        check(read.port == written.port, "PORT round trips");
        check(read.publicBaseUrl == written.publicBaseUrl, "PUBLIC_BASE_URL round trips");
        check(read.adminUsernames == written.adminUsernames, "ADMIN_USERNAMES round trips");
        check(read.starterPassword == written.starterPassword, "STARTER_PASSWORD round trips");
        check(read.jwtSecret == written.jwtSecret, "JWT_SECRET round trips");
        check(read.dataKey == written.dataKey, "DATA_KEY round trips");

        const std::string text = readFile(path);
        check(text.find("This file holds the owner password") != std::string::npos,
              "the file explains what it holds");
        check(text.find("JWT_SECRET = ") != std::string::npos, "keys are written by name");
        check(text.find("server.conf") == std::string::npos,
              "no temporary name is left in the content");

        // Saving over a file replaces it rather than appending a second copy.
        server::Settings changed = written;
        changed.port = "9090";
        check(server::saveSettings(path.string(), changed, &error), "second save: " + error);
        check(server::loadSettings(path.string(), &read, &error), "second load: " + error);
        check(read.port == "9090", "the replacement file wins");
        const std::string replaced = readFile(path);
        check(replaced.find("PORT = 8080") == std::string::npos, "the old port is gone");
        check(!fs::exists(path.string() + ".tmp"), "the temporary file is renamed away");
    }

    // Comments, blank lines, stray spaces and a value that contains '='.
    {
        const fs::path path = root / "hand-written.conf";
        writeFile(path,
                  "# written by hand\n"
                  "; another comment\n"
                  "\n"
                  "  node_env = development  \n"
                  "\tport\t=\t3001\n"
                  "PUBLIC_BASE_URL = https://example.com\n"
                  "STARTER_PASSWORD = a=b=c=d=e=f\n"
                  "SOMETHING_FROM_THE_FUTURE = whatever\n");
        std::string error;
        server::Settings read;
        check(server::loadSettings(path.string(), &read, &error), "hand-written file loads: " + error);
        check(read.nodeEnv == "development", "a lower-case key and padded value are accepted");
        check(read.port == "3001", "tabs around '=' are trimmed");
        check(read.publicBaseUrl == "https://example.com", "the URL keeps its value");
        check(read.starterPassword == "a=b=c=d=e=f", "only the first '=' splits the line");
        check(read.jwtSecret.empty(), "a key the file does not mention stays empty");
    }

    // A line that is not a pair is an error that names the line.
    {
        const fs::path path = root / "broken.conf";
        writeFile(path, "PORT = 3000\nthis line has no equals sign\n");
        std::string error;
        server::Settings read;
        check(!server::loadSettings(path.string(), &read, &error), "a broken line is refused");
        check(error.find(":2:") != std::string::npos, "the error names the line: " + error);
    }

    // A missing file is reported, not treated as an empty document.
    {
        std::string error;
        server::Settings read;
        check(!server::loadSettings((root / "nothing-here.conf").string(), &read, &error),
              "a missing file is an error");
        check(error.find("nothing-here.conf") != std::string::npos,
              "the error names the file: " + error);
    }

    // Validation: the settings a production install cannot start without, and the
    // ones it may leave out.
    {
        std::string error;
        check(server::validateSettings(sample(), &error), "the sample is valid: " + error);

        server::Settings broken = sample();
        broken.port = "eighty";
        check(!server::validateSettings(broken, &error), "a word is not a port");
        broken.port = "0";
        check(!server::validateSettings(broken, &error), "port 0 is refused");
        broken.port = "70000";
        check(!server::validateSettings(broken, &error), "port 70000 is refused");

        broken = sample();
        broken.nodeEnv = "staging";
        check(!server::validateSettings(broken, &error), "an unknown NODE_ENV is refused");

        broken = sample();
        broken.jwtSecret = "short";
        check(!server::validateSettings(broken, &error), "a short signing secret is refused");

        broken = sample();
        broken.starterPassword = "short";
        check(!server::validateSettings(broken, &error), "a short owner password is refused");

        broken = sample();
        broken.publicBaseUrl = "http://example.com/path";
        check(!server::validateSettings(broken, &error), "a URL with a path is refused");
        broken.publicBaseUrl = "ftp://example.com";
        check(!server::validateSettings(broken, &error), "a scheme other than http(s) is refused");
        broken.publicBaseUrl = "http://";
        check(!server::validateSettings(broken, &error), "a URL with no host is refused");
        broken.publicBaseUrl = "https://play.example.com:8443";
        check(server::validateSettings(broken, &error), "a plain https host:port is accepted");

        server::Settings minimal = sample();
        minimal.publicBaseUrl.clear();
        minimal.adminUsernames.clear();
        minimal.dataKey.clear();
        check(server::validateSettings(minimal, &error),
              "the URL, the admin list and the data key are optional: " + error);
    }

    // The environment assignments the host applies, in a fixed order, without the
    // empty ones.
    {
        const auto entries = server::settingsAsEnvironment(sample());
        check(entries.size() == 7, "all seven settings are handed over");
        check(entries.size() == 7 && entries[0].first == "NODE_ENV" && entries[0].second == "production",
              "NODE_ENV comes first");
        check(entries.size() == 7 && entries[6].first == "DATA_KEY", "DATA_KEY comes last");

        server::Settings minimal = sample();
        minimal.publicBaseUrl.clear();
        minimal.dataKey.clear();
        const auto fewer = server::settingsAsEnvironment(minimal);
        check(fewer.size() == 5, "empty settings are left out rather than set to nothing");
        for (const auto& entry : fewer) {
            check(!entry.second.empty(), entry.first + " is not handed over empty");
        }
    }

    // The folder a setup is allowed to write into: it has to hold a server.
    {
        std::string error;
        const fs::path empty = root / "not-a-server";
        fs::create_directories(empty, code);
        check(!server::checkServerFolder(empty.string(), &error), "a folder with nothing in it");
        check(error.find("web/public/index.html") != std::string::npos,
              "the error names what is missing: " + error);

        const fs::path siteOnly = root / "site-only";
        fs::create_directories(siteOnly / "web" / "public", code);
        writeFile(siteOnly / "web" / "public" / "index.html", "<!doctype html>");
        check(!server::checkServerFolder(siteOnly.string(), &error), "a site with no WebApp.exe");
        check(error.find("WebApp.exe") != std::string::npos,
              "the error names the program: " + error);

        const fs::path complete = root / "complete";
        fs::create_directories(complete / "web" / "public", code);
        writeFile(complete / "web" / "public" / "index.html", "<!doctype html>");
        writeFile(complete / "WebApp.exe", "not really a program");
        check(server::checkServerFolder(complete.string(), &error),
              "a folder with the site and the program is accepted: " + error);
        check(!server::checkServerFolder(std::string(), &error), "no folder at all is refused");
    }

    // Where the file goes: beside the programs, whatever form the folder takes.
    {
        check(server::settingsPathFor("C:\\RBLXClone") == "C:\\RBLXClone/server.conf",
              "the file sits in the folder it is given");
        check(server::settingsPathFor("C:\\RBLXClone\\") == "C:\\RBLXClone\\server.conf",
              "a trailing separator does not double up");
        check(server::settingsPathFor("") == "server.conf",
              "no folder means the working directory");
    }

    fs::remove_all(root, code);

    if (g_failures == 0) {
        std::printf("ServerConfigTests: all checks passed\n");
        return 0;
    }
    std::printf("ServerConfigTests: %d check(s) failed\n", g_failures);
    return 1;
}
