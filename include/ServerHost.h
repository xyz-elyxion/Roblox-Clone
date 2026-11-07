#pragma once

#include <memory>
#include <string>
#include <vector>

// RBLXHosts: this project's own server supervisor.
//
// Coolify's job, cut down to what this game needs. It keeps a fixed number of
// game servers alive: one per slot, each on its own port, with its own log; a
// server that exits is started again after a growing delay, and one that keeps
// dying is left failed instead of being restarted forever.
//
// The decisions live here, behind a launcher interface, so they can be checked
// with a fake clock and a fake launcher (ServerHostTests) instead of by waiting
// on real processes. src/HostMain.cpp is the thin console program on top.
namespace rblx {
namespace hosting {

struct Config {
    // Server.exe on Windows, Server elsewhere; a path is allowed.
    std::string server = "Server.exe";
    // Working directory for the children, so a world path can stay relative.
    std::string directory = ".";
    // Empty means the server keeps its own default world.
    std::string world;
    // --web; empty means the server keeps its own default.
    std::string webUrl;
    std::string logDirectory = "logs";
    int firstPort = 7777;
    int instances = 2;
    // A slot that has failed this many times inside the window is given up on.
    int maxRestarts = 5;
    int restartWindowMs = 60000;
    // Wait before the first restart; each further failure doubles it.
    int restartDelayMs = 1000;
};

// `key = value` lines, '#' or ';' comments, unknown keys ignored. Numbers are
// checked, so a typo is an error with the key's name rather than a silent zero.
bool parseConfig(const std::string& text, Config* out, std::string* error);
bool loadConfig(const std::string& path, Config* out, std::string* error);

// A commented file `RBLXHosts init` writes, so the keys are discoverable.
std::string defaultConfigText();

struct Instance {
    enum class State { Pending, Running, Backoff, Failed, Stopped };

    int index = 0;
    int port = 0;
    long pid = 0;
    State state = State::Pending;
    // How many times this slot has been (re)started after a death or a failure.
    int restarts = 0;
    // -1 until the slot has been started: 0 is a legal time for a fake clock.
    long long startedAtMs = -1;
    std::string logPath;
    std::string lastError;
};

const char* stateName(Instance::State state);

// Starting and stopping a process is the one thing that cannot be checked
// without a real machine, so it is the one thing kept behind an interface.
class Launcher {
public:
    virtual ~Launcher() = default;

    virtual bool launch(const Config& config, int port, const std::string& logPath, long* pid,
                        std::string* error) = 0;
    virtual bool alive(long pid) = 0;
    virtual void stop(long pid) = 0;
};

std::unique_ptr<Launcher> makeSystemLauncher();

class Supervisor {
public:
    // `launcher` must outlive the supervisor.
    Supervisor(Config config, Launcher* launcher);

    // Starts what is missing and restarts what died. Call it a few times a
    // second; it never blocks.
    void tick(long long nowMs);

    // Terminates every running slot and leaves the supervisor idle: later ticks
    // start nothing.
    void stopAll();

    const std::vector<Instance>& instances() const { return instances_; }

    // One line per slot plus a summary, for the console.
    std::string table(long long nowMs) const;

    int runningCount() const;
    bool allFailed() const;

private:
    int backoffFor(int failuresInWindow) const;
    void noteFailure(Instance& instance, long long nowMs, const std::string& error);

    Config config_;
    Launcher* launcher_ = nullptr;
    std::vector<Instance> instances_;
    // Per slot: when each restart happened, for the window, and when the next
    // attempt is due while backing off.
    std::vector<std::vector<long long>> restartTimes_;
    std::vector<long long> nextAttempt_;
    bool stopped_ = false;
};

}  // namespace hosting
}  // namespace rblx
