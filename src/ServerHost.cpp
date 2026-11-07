#include "ServerHost.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>

namespace rblx {
namespace hosting {
namespace {

std::string trim(const std::string& value) {
    const char* whitespace = " \t\r\n";
    const std::size_t first = value.find_first_not_of(whitespace);
    if (first == std::string::npos) return std::string();
    const std::size_t last = value.find_last_not_of(whitespace);
    return value.substr(first, last - first + 1);
}

std::string lowered(std::string value) {
    for (char& c : value) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return value;
}

// strtol that insists on the whole value being the number, so "12abc" and ""
// are errors rather than 12 and 0.
bool parseNumber(const std::string& text, const std::string& key, long* out, std::string* error) {
    const std::string value = trim(text);
    if (value.empty()) {
        *error = key + " has no value";
        return false;
    }
    char* end = nullptr;
    errno = 0;
    const long parsed = std::strtol(value.c_str(), &end, 10);
    if (end == nullptr || *end != '\0' || errno == ERANGE) {
        *error = key + " must be a whole number, not \"" + value + "\"";
        return false;
    }
    *out = parsed;
    return true;
}

std::string join(const std::string& directory, const std::string& name) {
    if (directory.empty()) return name;
    if (directory.back() == '/' || directory.back() == '\\') return directory + name;
    return directory + "/" + name;
}

}  // namespace

const char* stateName(Instance::State state) {
    switch (state) {
        case Instance::State::Pending: return "pending";
        case Instance::State::Running: return "running";
        case Instance::State::Backoff: return "backoff";
        case Instance::State::Failed: return "failed";
        case Instance::State::Stopped: return "stopped";
    }
    return "unknown";
}

std::string defaultConfigText() {
    return "# RBLXHosts: the game servers this machine keeps running.\n"
           "# Every key has the default shown next to it; delete a line to keep it.\n"
           "\n"
           "# The server executable, relative to the directory below or absolute.\n"
           "server = Server.exe\n"
           "\n"
           "# Working directory for the servers, so world paths can stay relative.\n"
           "directory = .\n"
           "\n"
           "# World file, and the website the servers report to.\n"
           "world = ServerWorld.world\n"
           "web = http://localhost:3000\n"
           "\n"
           "# Where each server's output is written: host-<port>.log.\n"
           "log-directory = logs\n"
           "\n"
           "# How many servers, and the port the first one listens on; the rest\n"
           "# count up from there (7777, 7778, ...).\n"
           "instances = 2\n"
           "port = 7777\n"
           "\n"
           "# A server that keeps dying is left failed instead of being restarted\n"
           "# forever: max-restarts failures inside restart-window milliseconds.\n"
           "max-restarts = 5\n"
           "restart-window = 60000\n"
           "\n"
           "# Wait before the first restart; each further failure doubles it.\n"
           "restart-delay = 1000\n";
}

bool parseConfig(const std::string& text, Config* out, std::string* error) {
    if (out == nullptr) return false;
    Config config;  // defaults, so a partial file is still a whole config

    std::istringstream stream(text);
    std::string line;
    int lineNumber = 0;
    while (std::getline(stream, line)) {
        ++lineNumber;
        const std::string stripped = trim(line);
        if (stripped.empty() || stripped[0] == '#' || stripped[0] == ';') continue;

        const std::size_t equals = stripped.find('=');
        if (equals == std::string::npos) {
            *error = "line " + std::to_string(lineNumber) + " is not a key = value pair";
            return false;
        }
        const std::string key = lowered(trim(stripped.substr(0, equals)));
        const std::string value = trim(stripped.substr(equals + 1));
        long number = 0;

        if (key == "server") {
            config.server = value;
        } else if (key == "directory" || key == "dir") {
            config.directory = value;
        } else if (key == "world") {
            config.world = value;
        } else if (key == "web") {
            config.webUrl = value;
        } else if (key == "log-directory" || key == "logs") {
            config.logDirectory = value;
        } else if (key == "port" || key == "first-port") {
            if (!parseNumber(value, key, &number, error)) return false;
            if (number < 1 || number > 65535) {
                *error = "port must be between 1 and 65535";
                return false;
            }
            config.firstPort = static_cast<int>(number);
        } else if (key == "instances") {
            if (!parseNumber(value, key, &number, error)) return false;
            if (number < 1 || number > 64) {
                *error = "instances must be between 1 and 64";
                return false;
            }
            config.instances = static_cast<int>(number);
        } else if (key == "max-restarts") {
            if (!parseNumber(value, key, &number, error)) return false;
            if (number < 0) {
                *error = "max-restarts cannot be negative";
                return false;
            }
            config.maxRestarts = static_cast<int>(number);
        } else if (key == "restart-window") {
            if (!parseNumber(value, key, &number, error)) return false;
            if (number < 0) {
                *error = "restart-window cannot be negative";
                return false;
            }
            config.restartWindowMs = static_cast<int>(number);
        } else if (key == "restart-delay") {
            if (!parseNumber(value, key, &number, error)) return false;
            if (number < 0) {
                *error = "restart-delay cannot be negative";
                return false;
            }
            config.restartDelayMs = static_cast<int>(number);
        }
        // Anything else is ignored on purpose: a file written for a later
        // version still runs here.
    }

    if (config.server.empty()) {
        *error = "server cannot be empty";
        return false;
    }
    *out = config;
    return true;
}

bool loadConfig(const std::string& path, Config* out, std::string* error) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        *error = "cannot read " + path;
        return false;
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return parseConfig(contents.str(), out, error);
}

// ---------------------------------------------------------------------------
// The supervisor

Supervisor::Supervisor(Config config, Launcher* launcher)
    : config_(std::move(config)), launcher_(launcher) {
    const int count = config_.instances < 1 ? 1 : config_.instances;
    instances_.reserve(static_cast<std::size_t>(count));
    restartTimes_.resize(static_cast<std::size_t>(count));
    nextAttempt_.assign(static_cast<std::size_t>(count), 0);

    for (int index = 0; index < count; ++index) {
        Instance instance;
        instance.index = index;
        instance.port = config_.firstPort + index;
        instance.logPath =
            join(config_.logDirectory, "host-" + std::to_string(instance.port) + ".log");
        instances_.push_back(instance);
    }
}

int Supervisor::backoffFor(int failuresInWindow) const {
    // 1x, 2x, 4x ... up to 32x the base delay: a server that dies instantly
    // should not be restarted in a tight loop, and one that ran for an hour
    // should come back quickly.
    int delay = config_.restartDelayMs;
    for (int step = 1; step < failuresInWindow && step < 5; ++step) delay *= 2;
    return delay;
}

void Supervisor::noteFailure(Instance& instance, long long nowMs, const std::string& error) {
    instance.lastError = error;
    ++instance.restarts;
    std::vector<long long>& times = restartTimes_[static_cast<std::size_t>(instance.index)];
    times.push_back(nowMs);
    const long long horizon = nowMs - config_.restartWindowMs;
    times.erase(std::remove_if(times.begin(), times.end(),
                               [horizon](long long at) { return at < horizon; }),
                times.end());

    if (static_cast<int>(times.size()) > config_.maxRestarts) {
        // Given up on: the slot stays down and visible rather than drumming a
        // broken server forever.
        instance.state = Instance::State::Failed;
        nextAttempt_[static_cast<std::size_t>(instance.index)] = 0;
        return;
    }
    instance.state = Instance::State::Backoff;
    nextAttempt_[static_cast<std::size_t>(instance.index)] =
        nowMs + backoffFor(static_cast<int>(times.size()));
}

void Supervisor::tick(long long nowMs) {
    if (stopped_ || launcher_ == nullptr) return;

    for (Instance& instance : instances_) {
        if (instance.state == Instance::State::Failed ||
            instance.state == Instance::State::Stopped) {
            continue;
        }
        const std::size_t slot = static_cast<std::size_t>(instance.index);

        if (instance.pid != 0) {
            if (launcher_->alive(instance.pid)) {
                instance.state = Instance::State::Running;
                continue;
            }
            // It was up and is now gone: that is a restart, not a first start.
            instance.pid = 0;
            noteFailure(instance, nowMs, "the process exited");
            continue;
        }

        if (instance.state == Instance::State::Backoff && nowMs < nextAttempt_[slot]) continue;

        long pid = 0;
        std::string error;
        if (launcher_->launch(config_, instance.port, instance.logPath, &pid, &error)) {
            instance.pid = pid;
            instance.state = Instance::State::Running;
            instance.startedAtMs = nowMs;
            instance.lastError.clear();
        } else {
            noteFailure(instance, nowMs, error.empty() ? std::string("could not start") : error);
        }
    }
}

void Supervisor::stopAll() {
    if (launcher_ != nullptr) {
        for (Instance& instance : instances_) {
            if (instance.pid != 0) launcher_->stop(instance.pid);
            instance.pid = 0;
            instance.state = Instance::State::Stopped;
        }
    }
    stopped_ = true;
}

int Supervisor::runningCount() const {
    int running = 0;
    for (const Instance& instance : instances_) {
        if (instance.state == Instance::State::Running) ++running;
    }
    return running;
}

bool Supervisor::allFailed() const {
    if (instances_.empty()) return false;
    for (const Instance& instance : instances_) {
        if (instance.state != Instance::State::Failed) return false;
    }
    return true;
}

std::string Supervisor::table(long long nowMs) const {
    std::string out;
    char line[192];
    std::snprintf(line, sizeof(line), "%-8s %-6s %-8s %-8s %-9s %-8s %s\n", "instance", "port",
                  "pid", "state", "restarts", "uptime", "log");
    out += line;
    for (const Instance& instance : instances_) {
        // Uptime is only meaningful while the process is the one we started.
        long long seconds = 0;
        if (instance.state == Instance::State::Running && instance.startedAtMs >= 0 &&
            nowMs >= instance.startedAtMs) {
            seconds = (nowMs - instance.startedAtMs) / 1000;
        }
        // Wide enough for an uptime measured in days, not just 99 hours.
        char uptime[32];
        std::snprintf(uptime, sizeof(uptime), "%02lld:%02lld:%02lld", seconds / 3600,
                      (seconds / 60) % 60, seconds % 60);
        std::snprintf(line, sizeof(line), "%-8d %-6d %-8ld %-8s %-9d %-8s %s\n", instance.index,
                      instance.port, instance.pid, stateName(instance.state), instance.restarts,
                      uptime, instance.logPath.c_str());
        out += line;
        if (!instance.lastError.empty()) {
            out += "         ! " + instance.lastError + "\n";
        }
    }
    std::snprintf(line, sizeof(line), "%d of %d running\n", runningCount(),
                  static_cast<int>(instances_.size()));
    out += line;
    return out;
}

// ---------------------------------------------------------------------------
// The system launcher

namespace {

// The log directory is the launcher's business, not the supervisor's: a slot with
// no directory to write into would look like a silent server, and a server whose
// output went nowhere is the one thing an operator cannot debug.
bool ensureDirectory(const std::string& path) {
    if (path.empty() || path == ".") return true;
    std::string current;
    std::size_t index = 0;
    for (;;) {
        const std::size_t slash = path.find_first_of("/\\", index);
        current = path.substr(0, slash == std::string::npos ? path.size() : slash);
        if (!current.empty()) {
            if (::CreateDirectoryA(current.c_str(), nullptr) == FALSE &&
                ::GetLastError() != ERROR_ALREADY_EXISTS) {
                return false;
            }
        }
        if (slash == std::string::npos) break;
        index = slash + 1;
    }
    return true;
}

std::string parentOf(const std::string& path) {
    const std::size_t slash = path.find_last_of("/\\");
    if (slash == std::string::npos) return std::string();
    return path.substr(0, slash);
}


std::string quoteArgument(const std::string& value) {
    std::string quoted = "\"";
    for (const char c : value) {
        if (c == '"') quoted += "\\\"";
        else quoted += c;
    }
    quoted += '"';
    return quoted;
}

class WindowsLauncher : public Launcher {
public:
    ~WindowsLauncher() override {
        for (auto& entry : handles_) ::CloseHandle(entry.second);
    }

    bool launch(const Config& config, int port, const std::string& logPath, long* pid,
                std::string* error) override {
        ensureDirectory(parentOf(logPath));
        std::string command = quoteArgument(config.server) + " --port " + std::to_string(port);
        if (!config.world.empty()) command += " --world " + quoteArgument(config.world);
        if (!config.webUrl.empty()) command += " --web " + quoteArgument(config.webUrl);
        command += " --log " + quoteArgument(logPath);

        SECURITY_ATTRIBUTES attributes{};
        attributes.nLength = sizeof(attributes);
        attributes.bInheritHandle = TRUE;
        HANDLE log = ::CreateFileA(logPath.c_str(), FILE_APPEND_DATA | FILE_WRITE_DATA,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE, &attributes, OPEN_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL, nullptr);
        if (log == INVALID_HANDLE_VALUE) log = nullptr;

        STARTUPINFOA startup{};
        startup.cb = sizeof(startup);
        startup.dwFlags = STARTF_USESTDHANDLES;
        startup.hStdOutput = log;
        startup.hStdError = log;
        startup.hStdInput = nullptr;

        PROCESS_INFORMATION information{};
        std::vector<char> mutableCommand(command.begin(), command.end());
        mutableCommand.push_back('\0');

        // CREATE_NO_WINDOW: the server writes to its log, not to this console.
        const BOOL started = ::CreateProcessA(
            nullptr, mutableCommand.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr,
            config.directory.empty() ? nullptr : config.directory.c_str(), &startup, &information);
        const DWORD startError = started == FALSE ? ::GetLastError() : 0;
        if (log != nullptr) ::CloseHandle(log);
        if (started == FALSE) {
            *error = "CreateProcess failed (error " + std::to_string(startError) + ")";
            return false;
        }

        ::CloseHandle(information.hThread);
        *pid = static_cast<long>(information.dwProcessId);
        handles_[*pid] = information.hProcess;
        return true;
    }

    bool alive(long pid) override {
        const auto entry = handles_.find(pid);
        if (entry == handles_.end()) return false;
        if (::WaitForSingleObject(entry->second, 0) == WAIT_TIMEOUT) return true;
        ::CloseHandle(entry->second);
        handles_.erase(entry);
        return false;
    }

    void stop(long pid) override {
        const auto entry = handles_.find(pid);
        if (entry == handles_.end()) return;
        ::TerminateProcess(entry->second, 0);
        ::WaitForSingleObject(entry->second, 5000);
        ::CloseHandle(entry->second);
        handles_.erase(entry);
    }

private:
    std::map<long, HANDLE> handles_;
};


}  // namespace

std::unique_ptr<Launcher> makeSystemLauncher() {
    return std::unique_ptr<Launcher>(new WindowsLauncher());
}

}  // namespace hosting
}  // namespace rblx
