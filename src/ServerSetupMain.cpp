// ServerSetup: what a machine needs before it can host the site.
//
// A published install is configured through environment variables, which is right
// for a service and wrong for somebody who unzipped a folder and double-clicked a
// program: seven variables, two of them secrets they have to invent. This asks for
// the few that matter, writes them to server.conf beside the programs - where
// WebApp reads them, with the environment still able to override any of them -
// and starts the site. Nothing here is a launcher: the settings outlive this
// window, so WebApp.exe started on its own uses them too.
//
// Usage:
//   ServerSetup                       ask in a window, then start the site
//   ServerSetup --port 8080 --url http://play.example.com:8080
//   ServerSetup --no-start            write the settings and stop
//   ServerSetup --folder <dir> --admin helper --password <12+ chars>
//   ServerSetup --help
//
// The secrets are generated when the file does not already have one: regenerating
// them on every run would sign everyone out and, because the data file is sealed
// with one of them, make the accounts unreadable.

#include "InstallUi.h"
#include "ServerConfig.h"
#include "WebCrypto.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace {

namespace fs = std::filesystem;
namespace server = rblx::server;

struct Options {
    std::string folder;
    std::string port;
    std::string publicBaseUrl;
    std::string adminUsernames;
    std::string starterPassword;
    std::string jwtSecret;
    std::string dataKey;
    bool start = true;
    bool quiet = false;
    bool help = false;
    bool ui = false;
    bool noUi = false;
};

// Where a windowed run sends its lines; empty means the console, which is what a
// script and the checks use.
std::function<void(const std::string&)> g_progress;

void say(const Options& options, const std::string& text) {
    if (g_progress) {
        g_progress(text);
        return;
    }
    if (!options.quiet) std::printf("ServerSetup: %s\n", text.c_str());
}

void usage() {
    std::printf(
        "ServerSetup - prepares this folder to host the RBLX Clone website\n\n"
        "  ServerSetup [--port <n>] [--url <http(s)://host:port>] [--admin <names>]\n"
        "              [--password <12+ chars>] [--folder <dir>] [--no-start]\n\n"
        "Writes server.conf beside WebApp.exe and starts the site. The file holds the\n"
        "settings the site otherwise reads from the environment, and the environment\n"
        "still wins over it, so a service or a script can override one value.\n\n"
        "  --no-start     write the file and stop\n"
        "  --ui           use the window even when arguments were given\n"
        "  --no-ui        use the console even when run with no arguments\n"
        "  --quiet        print nothing on success\n\n"
        "The signing secret and the data key are generated when server.conf does not\n"
        "have one. They are not regenerated: everyone would be signed out, and the\n"
        "account data is sealed with the data key.\n");
}

std::string currentDirectory() {
    char buffer[MAX_PATH] = {};
    const DWORD length = ::GetCurrentDirectoryA(MAX_PATH, buffer);
    if (length == 0 || length >= MAX_PATH) return std::string();
    return buffer;
}

std::string executableDirectory() {
    char buffer[MAX_PATH] = {};
    const DWORD length = ::GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) return currentDirectory();
    return fs::path(buffer).parent_path().string();
}

std::string joinPath(const std::string& base, const std::string& name) {
    return (fs::path(base) / name).string();
}

int portNumber(const std::string& text) {
    return static_cast<int>(std::strtol(text.c_str(), nullptr, 10));
}

std::string trim(const std::string& text) {
    std::size_t start = 0;
    while (start < text.size() && (text[start] == ' ' || text[start] == '\t')) ++start;
    std::size_t end = text.size();
    while (end > start && (text[end - 1] == ' ' || text[end - 1] == '\t')) --end;
    return text.substr(start, end - start);
}

// A port can be taken by another service, and asking Windows is cheaper than
// starting the site and reading its error. The bind is wildcard, like WebApp's,
// because a port held on one address only would still refuse the site's own bind.
bool portIsFree(int port, std::string* error) {
    WSADATA data;
    if (::WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        if (error) *error = "could not start WinSock";
        return false;
    }
    SOCKET handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (handle == INVALID_SOCKET) {
        ::WSACleanup();
        if (error) *error = "could not create a socket to test the port";
        return false;
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(static_cast<unsigned short>(port));
    const bool free = ::bind(handle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0;
    ::closesocket(handle);
    ::WSACleanup();
    if (!free) {
        if (error) {
            *error = "port " + std::to_string(port) +
                     " is already in use; stop what is listening on it, or pick another port";
        }
        return false;
    }
    return true;
}

bool answers(int port) {
    WSADATA data;
    if (::WSAStartup(MAKEWORD(2, 2), &data) != 0) return false;
    SOCKET handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bool open = false;
    if (handle != INVALID_SOCKET) {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_port = htons(static_cast<unsigned short>(port));
        ::inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
        open = ::connect(handle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0;
        ::closesocket(handle);
    }
    ::WSACleanup();
    return open;
}

// Starts WebApp.exe from the folder it lives in without waiting for it: the site
// keeps running after this window closes, which is the point of starting it here.
bool startWebApp(const std::string& folder, std::string* error) {
    const std::string program = joinPath(folder, "WebApp.exe");
    std::string command = "\"" + program + "\"";

    STARTUPINFOA startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    std::vector<char> mutableCommand(command.begin(), command.end());
    mutableCommand.push_back('\0');

    const BOOL started = ::CreateProcessA(program.c_str(), mutableCommand.data(), nullptr, nullptr,
                                          FALSE, 0, nullptr, folder.c_str(), &startup, &process);
    if (started == 0) {
        if (error) *error = "could not start " + program + " (error " + std::to_string(::GetLastError()) + ")";
        return false;
    }
    ::CloseHandle(process.hThread);
    ::CloseHandle(process.hProcess);
    return true;
}

std::string browserUrl(const server::Settings& settings) {
    return "http://localhost:" + settings.port + "/";
}

// The window's fields, in the order they are drawn.
enum FieldIndex { kPort, kPublicUrl, kAdmin, kPassword, kSigningSecret, kDataKey, kFieldCount };

std::string secretOrNew(const std::string& existing) {
    if (!trim(existing).empty()) return existing;
    return rblx::web::randomHex(32);
}

// What the folder already runs with, so a second run keeps it.
server::Settings existingSettings(const std::string& folder);

bool prepare(Options& options, server::Settings* settings, std::string* error) {
    if (options.folder.empty()) options.folder = executableDirectory();
    std::error_code code;
    options.folder = fs::weakly_canonical(fs::absolute(options.folder, code), code).string();
    if (code || options.folder.empty()) {
        if (error) *error = "could not work out which folder to set up";
        return false;
    }

    if (!server::checkServerFolder(options.folder, error)) return false;

    // A value that was not given is taken from the file this install already has,
    // not from a blank: the secrets must survive a second run, and so should the
    // port. Only an explicit argument replaces what is there.
    const server::Settings existing = existingSettings(options.folder);
    const auto pick = [](const std::string& given, const std::string& kept) {
        return trim(given).empty() ? kept : trim(given);
    };

    server::Settings next;
    next.nodeEnv = "production";
    next.port = pick(options.port, existing.port.empty() ? "3000" : existing.port);
    next.publicBaseUrl = pick(options.publicBaseUrl, existing.publicBaseUrl);
    next.adminUsernames = pick(options.adminUsernames, existing.adminUsernames);
    next.starterPassword = pick(options.starterPassword, existing.starterPassword);
    next.jwtSecret = secretOrNew(pick(options.jwtSecret, existing.jwtSecret));
    next.dataKey = secretOrNew(pick(options.dataKey, existing.dataKey));

    if (!server::validateSettings(next, error)) return false;

    if (!portIsFree(portNumber(next.port), error)) return false;

    *settings = next;
    return true;
}

bool save(const Options& options, const server::Settings& settings, std::string* error) {
    const std::string path = server::settingsPathFor(options.folder);
    say(options, "writing " + path);
    return server::saveSettings(path, settings, error);
}

// What the window's "Set up" button does, and what the console path does.
bool setUp(Options& options, server::Settings* settings, std::string* error) {
    if (!prepare(options, settings, error)) return false;
    if (!save(options, *settings, error)) return false;
    say(options, "the site will run in production on port " + settings->port);

    if (!options.start) {
        say(options, std::string("done; start it with ") + joinPath(options.folder, "WebApp.exe"));
        return true;
    }

    say(options, "starting WebApp.exe");
    if (!startWebApp(options.folder, error)) return false;

    // The site is ready when its port answers, not when the process exists: the
    // window says "running" only once there is something to open.
    for (int attempt = 0; attempt < 80; ++attempt) {
        if (answers(portNumber(settings->port))) {
            say(options, "the website is running at " + browserUrl(*settings));
            return true;
        }
        ::Sleep(250);
    }

    if (error) {
        *error = "WebApp.exe started but nothing is listening on port " + settings->port +
                 " yet; its console window has the reason";
    }
    return false;
}

bool openBrowser(const server::Settings& settings) {
    const HINSTANCE result = ::ShellExecuteA(nullptr, "open", browserUrl(settings).c_str(), nullptr,
                                            nullptr, SW_SHOWNORMAL);
    return reinterpret_cast<std::intptr_t>(result) > 32;
}

// Starts from what the folder already has, so running the setup twice keeps the
// secrets and the port rather than replacing what the install is using.
server::Settings existingSettings(const std::string& folder) {
    server::Settings settings;
    std::string error;
    if (server::loadSettings(server::settingsPathFor(folder), &settings, &error)) {
        return settings;
    }
    server::Settings fresh;
    fresh.nodeEnv = "production";
    fresh.port = "3000";
    return fresh;
}

void collectFields(const std::vector<std::string>& fields, Options* options) {
    if (fields.size() != kFieldCount) return;
    options->port = fields[kPort];
    options->publicBaseUrl = fields[kPublicUrl];
    options->adminUsernames = fields[kAdmin];
    options->starterPassword = fields[kPassword];
    options->jwtSecret = fields[kSigningSecret];
    options->dataKey = fields[kDataKey];
}

// Runs in a window and returns -1 when there is no window to run in, so the caller
// does the same work on the console.
int windowed(Options& options) {
    if (options.folder.empty()) options.folder = executableDirectory();
    const server::Settings existing = existingSettings(options.folder);

    rblx::ui::Options window;
    window.title = "RBLX Clone server setup";
    window.heading = "Set up this server";
    window.detail =
        "Writes server.conf beside WebApp.exe and starts the website in production. The "
        "environment still overrides the file, and a value already in it is kept unless you "
        "change it here.";
    window.startLabel = "Set up";
    window.cancelLabel = "Cancel";
    window.folder = options.folder;
    window.folderLabel = "Server folder";
    window.showFolder = true;
    window.showShortcuts = false;

    const std::string port = options.port.empty() ? existing.port : options.port;
    const std::string secret = options.jwtSecret.empty() ? existing.jwtSecret : options.jwtSecret;
    window.fields = {
        {"Port", port.empty() ? "3000" : port, false, 16},
        {"Public URL (empty when this machine is the site)", options.publicBaseUrl.empty()
                                                                 ? existing.publicBaseUrl
                                                                 : options.publicBaseUrl,
         false, 200},
        {"Extra /admin accounts, comma separated",
         options.adminUsernames.empty() ? existing.adminUsernames : options.adminUsernames, false,
         200},
        {"Owner password (at least 12 characters)",
         options.starterPassword.empty() ? existing.starterPassword : options.starterPassword, true,
         200},
        {"Signing secret (JWT_SECRET)", secret.empty() ? secretOrNew(std::string()) : secret, true,
         200},
        {"Data key (DATA_KEY)", options.dataKey.empty() ? existing.dataKey : options.dataKey, true,
         200},
    };

    const std::vector<std::string> actions = {"Open the website", "Close"};
    server::Settings settings;

    const rblx::ui::Result result = rblx::ui::run(window, actions, [&](rblx::ui::Report& report) {
        Options run = options;
        if (!window.folder.empty()) run.folder = window.folder;
        std::vector<std::string> values;
        for (const rblx::ui::Options::Field& field : window.fields) {
            values.push_back(field.value);
        }
        collectFields(values, &run);

        g_progress = [&report](const std::string& text) { report.line(text); };
        std::string error;
        bool ok = false;
        if (!prepare(run, &settings, &error)) {
            report.problem(error);
        } else if (!save(run, settings, &error)) {
            report.problem(error);
        } else if (!run.start) {
            report.line("wrote the settings; starting the site was turned off");
            ok = true;
        } else {
            report.line("starting WebApp.exe");
            ok = startWebApp(run.folder, &error);
            if (!ok) {
                report.problem(error);
            } else {
                for (int attempt = 0; attempt < 80 && !answers(portNumber(settings.port));
                     ++attempt) {
                    report.progress(static_cast<float>(attempt) / 80.0f);
                    ::Sleep(250);
                }
                if (!answers(portNumber(settings.port))) {
                    report.problem("WebApp.exe started but nothing is listening on port " +
                                   settings.port + " yet; its console window has the reason");
                } else {
                    report.line("the website is running at " + browserUrl(settings));
                }
            }
        }
        g_progress = nullptr;
        return ok;
    });

    if (!result.shown) return -1;
    if (result.cancelled) return 0;
    if (result.action == "Open the website") {
        // A settings document that failed to prepare still has a usable port from
        // the file, and opening the site is what the button promises.
        if (settings.port.empty()) settings = existing;
        openBrowser(settings);
    }
    return result.ok ? 0 : 1;
}

bool parseOptions(int argc, char** argv, Options* options) {
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        const bool needsValue = argument == "--folder" || argument == "--port" ||
                                argument == "--url" || argument == "--admin" ||
                                argument == "--password" || argument == "--secret" ||
                                argument == "--data-key";
        if (needsValue && i + 1 >= argc) {
            std::fprintf(stderr, "ServerSetup: %s needs a value\n", argument.c_str());
            return false;
        }
        if (argument == "--folder") {
            options->folder = argv[++i];
        } else if (argument == "--port") {
            options->port = argv[++i];
        } else if (argument == "--url") {
            options->publicBaseUrl = argv[++i];
        } else if (argument == "--admin") {
            options->adminUsernames = argv[++i];
        } else if (argument == "--password") {
            options->starterPassword = argv[++i];
        } else if (argument == "--secret") {
            options->jwtSecret = argv[++i];
        } else if (argument == "--data-key") {
            options->dataKey = argv[++i];
        } else if (argument == "--no-start") {
            options->start = false;
        } else if (argument == "--ui") {
            options->ui = true;
        } else if (argument == "--no-ui") {
            options->noUi = true;
        } else if (argument == "--quiet") {
            options->quiet = true;
        } else if (argument == "--help" || argument == "-h") {
            options->help = true;
        } else {
            std::fprintf(stderr, "ServerSetup: unknown option %s\n", argument.c_str());
            usage();
            return false;
        }
    }
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    Options options;
    if (!parseOptions(argc, argv, &options)) return 2;
    if (options.help) {
        usage();
        return 0;
    }

    // No arguments means somebody double-clicked it, which deserves a window. Any
    // argument means a script, which wants the console; --ui and --no-ui override
    // that, and a machine that cannot open a window falls back to the console.
    int status = -1;
    if (!options.noUi && (options.ui || argc == 1)) {
        status = windowed(options);
    }
    if (status < 0) {
        server::Settings settings;
        std::string error;
        // Outside a window there is still output to read, so the lines go to the
        // console as they happen rather than only at the end.
        if (!options.quiet) {
            g_progress = [](const std::string& text) {
                std::printf("ServerSetup: %s\n", text.c_str());
            };
        }
        const bool ok = setUp(options, &settings, &error);
        g_progress = nullptr;
        if (!ok) {
            std::fprintf(stderr, "ServerSetup: %s\n", error.c_str());
            return 1;
        }
        status = 0;
    }
    return status;
}
