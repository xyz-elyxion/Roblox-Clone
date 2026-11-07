#pragma once

#include "ServerHost.h"

#include <memory>
#include <string>
#include <vector>

// RBLXHosts' control panel.
//
// A pool of processes is only useful if you can see it and act on it, so the
// supervisor serves a small dashboard on loopback: the state of every slot, its
// log, and the four buttons an operator actually presses (start, stop, restart,
// stop the pool). It is the dashboard half of what a hosting platform gives you,
// at the size of one machine.
//
// The dashboard never touches the Supervisor from its own thread. It serves a
// snapshot the main loop publishes and queues commands for the main loop to
// apply, so every decision about the pool is still made on one thread, in
// Supervisor, where the tests can reach it.
namespace rblx {
namespace hosting {

struct DashboardSettings {
    // Loopback by default: this endpoint starts and stops processes.
    std::string host = "127.0.0.1";
    int port = 7900;
    // Empty means one is made up for this run and printed with the URL.
    std::string token;
    // web/host. Empty means: look for it next to the executables.
    std::string pageDirectory;
};

struct Command {
    enum class Kind { Start, Stop, Restart, StartAll, StopAll };

    Kind kind = Kind::Start;
    int index = 0;
};

class HostDashboard {
public:
    // Internal state, defined in HostDashboard.cpp. It is named here so the
    // request handlers in that file can take it; nothing else constructs one.
    struct Impl;

    HostDashboard();
    ~HostDashboard();

    HostDashboard(const HostDashboard&) = delete;
    HostDashboard& operator=(const HostDashboard&) = delete;

    // Binds the port and starts answering on its own thread. A failure here is
    // reported and the pool keeps running: the console still supervises.
    bool start(const DashboardSettings& settings, std::string* error);
    void stop();
    bool running() const;

    // Called by the main loop, so the page always reads the supervisor's own
    // view rather than a copy taken at some other moment.
    void publish(const Config& config, const Supervisor& supervisor, long long nowMs);

    // What the page asked for since the last call. Applying them is the main
    // loop's job.
    std::vector<Command> takeCommands();

    // The URL to paste into a browser, token included. The page exchanges that
    // token for an HttpOnly cookie and redirects, so it is not kept in the URL.
    std::string url() const;
    std::string token() const;

private:
    std::unique_ptr<Impl> impl_;
    std::string host_;
    std::string token_;
    int port_ = 0;
};

}  // namespace hosting
}  // namespace rblx
