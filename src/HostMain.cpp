// RBLXHosts: keeps this machine's game servers running.
//
// This is the project's own answer to a hosting platform, at the size the game
// needs: N servers, each on its own port and log, restarted when they die and
// given up on when they keep dying. The supervising decisions live in
// ServerHost.cpp and are covered by ServerHostTests; this file is the console
// around them.
//
//   RBLXHosts up --config hosting.conf     keep the servers running until Ctrl+C
//   RBLXHosts init --config hosting.conf   write a commented default file
//   RBLXHosts --help

#include "HostDashboard.h"
#include "ServerHost.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <thread>

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>

namespace {

std::atomic<bool> g_stopRequested{false};

BOOL WINAPI consoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT || signal == CTRL_CLOSE_EVENT) {
        g_stopRequested = true;
        return TRUE;
    }
    return FALSE;
}

long long nowMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

void printUsage(const char* program) {
    std::printf(
        "RBLXHosts - keeps the game servers on this machine running\n"
        "\n"
        "  %s up [options]      start the pool and supervise it until Ctrl+C\n"
        "  %s init [--config]   write a commented default config file\n"
        "\n"
        "Options for up:\n"
        "  --config <file>      read settings from this file (key = value)\n"
        "  --instance(s) <n>    how many servers to keep running\n"
        "  --port <n>           the first port; the others count up from it\n"
        "  --dir <folder>       working directory for the servers\n"
        "  --world <file>       world each server loads\n"
        "  --web <url>          website the servers report to\n"
        "  --server <path>      the server executable\n"
        "  --dashboard-port <n> the control panel's port, 0 to turn it off\n"
        "  --dashboard-host <h> the address the panel listens on, loopback by default\n"
        "\n"
        "The control panel prints a URL with a token at startup: open it to see every\n"
        "slot, read a server's log, and start, stop or restart one.\n"
        "\n"
        "A server that exits is started again after a growing delay; one that\n"
        "keeps dying inside the restart window is left failed and shown as such.\n",
        program, program);
}

void applyCommand(rblx::hosting::Supervisor& supervisor,
                  const rblx::hosting::Command& command) {
    using rblx::hosting::Command;

    std::string error;
    bool applied = true;
    switch (command.kind) {
        case Command::Kind::Start:
            applied = supervisor.startSlot(command.index, &error);
            break;
        case Command::Kind::Stop:
            applied = supervisor.stopSlot(command.index, &error);
            break;
        case Command::Kind::Restart:
            applied = supervisor.restartSlot(command.index, &error);
            break;
        case Command::Kind::StartAll:
            supervisor.startAll();
            break;
        case Command::Kind::StopAll:
            supervisor.stopAll();
            break;
    }
    if (!applied && !error.empty()) {
        std::fprintf(stderr, "RBLXHosts: %s\n", error.c_str());
    }
}

bool writeDefaultConfig(const std::string& path, std::string* error) {
    std::ifstream existing(path);
    if (existing) {
        *error = path + " already exists";
        return false;
    }
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        *error = "cannot write " + path;
        return false;
    }
    file << rblx::hosting::defaultConfigText();
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    std::string command;
    std::string configPath;
    rblx::hosting::Config overrides;
    // Which overrides were actually given on the command line: the file is read
    // first, then only these fields replace it.
    bool haveInstances = false;
    bool havePort = false;
    bool haveDirectory = false;
    bool haveWorld = false;
    bool haveWeb = false;
    bool haveServer = false;
    bool haveDashboardPort = false;
    bool haveDashboardHost = false;

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i] != nullptr ? argv[i] : "";
        const bool hasValue = i + 1 < argc;
        if (argument == "--help" || argument == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (argument == "up" || argument == "init") {
            command = argument;
        } else if (argument == "--config" && hasValue) {
            configPath = argv[++i];
        } else if ((argument == "--instances" || argument == "--instance") && hasValue) {
            overrides.instances = std::atoi(argv[++i]);
            haveInstances = true;
        } else if (argument == "--port" && hasValue) {
            overrides.firstPort = std::atoi(argv[++i]);
            havePort = true;
        } else if (argument == "--dir" && hasValue) {
            overrides.directory = argv[++i];
            haveDirectory = true;
        } else if (argument == "--world" && hasValue) {
            overrides.world = argv[++i];
            haveWorld = true;
        } else if (argument == "--web" && hasValue) {
            overrides.webUrl = argv[++i];
            haveWeb = true;
        } else if (argument == "--server" && hasValue) {
            overrides.server = argv[++i];
            haveServer = true;
        } else if (argument == "--dashboard-port" && hasValue) {
            overrides.dashboardPort = std::atoi(argv[++i]);
            haveDashboardPort = true;
        } else if (argument == "--dashboard-host" && hasValue) {
            overrides.dashboardHost = argv[++i];
            haveDashboardHost = true;
        } else {
            std::fprintf(stderr, "RBLXHosts: unknown argument \"%s\"\n\n", argument.c_str());
            printUsage(argv[0]);
            return 2;
        }
    }

    if (command.empty()) {
        printUsage(argv[0]);
        return 2;
    }

    if (command == "init") {
        const std::string path = configPath.empty() ? "hosting.conf" : configPath;
        std::string error;
        if (!writeDefaultConfig(path, &error)) {
            std::fprintf(stderr, "RBLXHosts: %s\n", error.c_str());
            return 1;
        }
        std::printf("wrote %s\n", path.c_str());
        return 0;
    }

    rblx::hosting::Config config;
    if (!configPath.empty()) {
        std::string error;
        if (!rblx::hosting::loadConfig(configPath, &config, &error)) {
            std::fprintf(stderr, "RBLXHosts: %s\n", error.c_str());
            return 1;
        }
    }
    if (haveInstances) config.instances = overrides.instances;
    if (havePort) config.firstPort = overrides.firstPort;
    if (haveDirectory) config.directory = overrides.directory;
    if (haveWorld) config.world = overrides.world;
    if (haveWeb) config.webUrl = overrides.webUrl;
    if (haveServer) config.server = overrides.server;
    if (haveDashboardPort) config.dashboardPort = overrides.dashboardPort;
    if (haveDashboardHost) config.dashboardHost = overrides.dashboardHost;

    if (config.instances < 1 || config.instances > 64) {
        std::fprintf(stderr, "RBLXHosts: instances must be between 1 and 64\n");
        return 1;
    }
    if (config.firstPort < 1 || config.firstPort + config.instances - 1 > 65535) {
        std::fprintf(stderr, "RBLXHosts: the ports from %d would leave the valid range\n",
                     config.firstPort);
        return 1;
    }
    if (config.dashboardPort < 0 || config.dashboardPort > 65535) {
        std::fprintf(stderr, "RBLXHosts: dashboard-port must be between 0 and 65535\n");
        return 1;
    }

    ::SetConsoleCtrlHandler(consoleHandler, TRUE);

    const std::unique_ptr<rblx::hosting::Launcher> launcher = rblx::hosting::makeSystemLauncher();
    rblx::hosting::Supervisor supervisor(config, launcher.get());

    std::printf("RBLXHosts: %d server(s) from %d, %s\n", config.instances, config.firstPort,
                config.server.c_str());

    rblx::hosting::HostDashboard dashboard;
    if (config.dashboardPort > 0) {
        rblx::hosting::DashboardSettings settings;
        settings.host = config.dashboardHost;
        settings.port = config.dashboardPort;
        std::string dashboardError;
        if (!dashboard.start(settings, &dashboardError)) {
            // The pool is the point and the panel is how it is watched, so a port
            // that is already taken is said out loud but is not a reason to stop.
            std::fprintf(stderr, "RBLXHosts: control panel not started: %s\n",
                         dashboardError.c_str());
        } else {
            std::printf("control panel: %s\n", dashboard.url().c_str());
        }
    }

    std::printf("Ctrl+C stops them all.\n\n");

    long long nextReport = 0;
    bool reportedFailure = false;
    while (!g_stopRequested) {
        const long long now = nowMs();

        // The panel only ever queues a command; applying it here keeps every
        // decision about the pool on this thread.
        for (const rblx::hosting::Command& command : dashboard.takeCommands()) {
            applyCommand(supervisor, command);
        }

        supervisor.tick(now);
        dashboard.publish(config, supervisor, now);

        if (now >= nextReport) {
            std::printf("%s\n", supervisor.table(now).c_str());
            nextReport = now + 5000;
        }

        // Every slot having given up is the one state worth leaving on its own:
        // the table already explains why, and staying alive would only spin.
        if (supervisor.allFailed() && !reportedFailure) {
            std::fprintf(stderr, "RBLXHosts: every server slot has failed; see the table above\n");
            reportedFailure = true;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    std::printf("\nstopping...\n");
    dashboard.stop();
    supervisor.stopAll();
    std::printf("%s", supervisor.table(nowMs()).c_str());
    return reportedFailure ? 1 : 0;
}
