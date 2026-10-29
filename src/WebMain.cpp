#include "ConsoleGui.h"
#include "WebApp.h"

#include <cstdio>
#include <string>
#include <thread>
#include <vector>

namespace rblx {
namespace web {

int runWebsiteServer(int argc, char** argv) {
    Config config;
    std::string error;
    if (!loadConfigFromEnvironment(&config, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    // Capture before initialization so seeding and world validation messages
    // reach the window too. The capture echoes to the console as well, so a
    // fatal startup error is still visible if no window can be shown.
    const bool wantGui = console::guiAllowed(argc, argv);
    if (wantGui) {
        console::beginCapture();
    }

    WebsiteApp app(config);
    if (!app.initialize(&error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    app.installRoutes();
    if (!app.start(&error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    const std::string siteUrl = "http://localhost:" + std::to_string(app.server().port());
    std::printf("Website server running on %s\n", siteUrl.c_str());
    std::printf("Serving %s\n", config.webRoot.c_str());

    if (!wantGui || !console::captureActive()) {
        std::printf("Console window disabled; press Ctrl+C to stop.\n");
        app.run();
        app.stop();
        return 0;
    }

    std::thread serverThread([&app]() { app.run(); });

    console::Host host;
    host.title = "RBLX Clone Server";
    host.siteUrl = siteUrl;
    host.collectServers = [&app](std::vector<console::ServerRow>* rows) {
        std::vector<GameInstanceInfo> instances;
        app.collectGameInstances(&instances);
        rows->clear();
        rows->reserve(instances.size());
        for (const GameInstanceInfo& instance : instances) {
            console::ServerRow row;
            row.id = instance.id;
            row.title = instance.title;
            row.status = instance.status;
            row.logPath = instance.logPath;
            row.startedAtMs = instance.startedAtMs;
            row.gameId = instance.gameId;
            row.port = instance.port;
            row.players = instance.players;
            row.pid = instance.pid;
            rows->push_back(std::move(row));
        }
    };
    host.stopServer = [&app](const std::string& instanceId) {
        return app.stopGameInstanceById(instanceId);
    };

    const int status = console::runConsoleWindow(host);
    if (status != 0) {
        // No display or no usable OpenGL: keep serving with plain console output
        // instead of exiting on a machine that only needs the HTTP server.
        std::printf("Could not open the console window; continuing in the terminal.\n");
        app.run();
        app.stop();
        return 0;
    }

    app.stop();
    serverThread.join();
    return 0;
}

}  // namespace web
}  // namespace rblx
