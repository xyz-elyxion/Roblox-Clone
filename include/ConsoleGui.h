#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace rblx {
namespace console {

enum class Level { Info = 0, Warn = 1, Error = 2 };

struct Entry {
    long long timeMs = 0;
    Level level = Level::Info;
    std::string text;
};

// Process-wide log store. Existing printf / std::cout calls feed it through the
// capture below, so no call site needs to know the GUI exists.
class LogBuffer {
public:
    static LogBuffer& instance();

    void add(Level level, const std::string& text);
    void clear();
    std::vector<Entry> snapshot() const;
    // Bumped whenever the contents change, so the window can skip re-copying.
    unsigned long long revision() const;
    std::size_t size() const;

private:
    LogBuffer() = default;

    mutable std::mutex mutex_;
    std::vector<Entry> entries_;
    unsigned long long revision_ = 1;
};

// Sends this process's stdout/stderr through a pipe that feeds LogBuffer while
// still echoing whole lines to the original handles. Call once, before the
// output worth showing is produced.
bool beginCapture();
bool captureActive();

struct ServerRow {
    std::string id;
    std::string title;
    std::string status;
    std::string logPath;
    long long startedAtMs = 0;
    int gameId = 0;
    int port = 0;
    int players = 0;
    long pid = 0;
};

struct Host {
    std::string title = "RBLX Clone Server";
    std::string siteUrl;
    // Fills the game server list for the current frame.
    std::function<void(std::vector<ServerRow>*)> collectServers;
    // Stops one game server; returns false when it already went away.
    std::function<bool(const std::string&)> stopServer;
};

// False when --nogui / --headless is passed or RBLX_NO_GUI is set.
bool guiAllowed(int argc, char** argv);

// Runs the console window until it is closed. Returns 0 on a normal close and
// non-zero when no window could be created, in which case the caller should
// fall back to the plain console loop.
int runConsoleWindow(Host host);

void openInBrowser(const std::string& url);

}  // namespace console
}  // namespace rblx
