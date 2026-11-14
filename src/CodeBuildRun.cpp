// The build process itself, which only exists on Windows.
//
// It is deliberately thin: start the tool with its output on a pipe, collect the
// lines on a thread, and let the window poll. Everything that could be checked
// without a compiler lives in CodeBuild instead.

#include "CodeBuildRun.h"

#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace rblx {
namespace code {
namespace {

constexpr std::size_t kMaxLines = 20000;

}  // namespace

// ---------------------------------------------------------------------------
// The process

BuildRun::BuildRun() = default;

BuildRun::~BuildRun() {
    stop();
    if (thread_.joinable()) thread_.join();
    if (process_ != nullptr) {
        ::CloseHandle(static_cast<HANDLE>(process_));
        process_ = nullptr;
    }
}

bool BuildRun::start(const std::string& projectRoot, const std::string& program,
                     const std::string& commandLine, std::string* error) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (running_) {
            if (error != nullptr) *error = "a build is already running";
            return false;
        }
    }
    if (thread_.joinable()) thread_.join();
    if (process_ != nullptr) {
        ::CloseHandle(static_cast<HANDLE>(process_));
        process_ = nullptr;
    }

    SECURITY_ATTRIBUTES attributes{};
    attributes.nLength = sizeof(attributes);
    attributes.bInheritHandle = TRUE;
    HANDLE readPipe = nullptr;
    HANDLE writePipe = nullptr;
    if (::CreatePipe(&readPipe, &writePipe, &attributes, 0) == 0) {
        if (error != nullptr) {
            *error = "could not create the pipe for the build output (error " +
                     std::to_string(::GetLastError()) + ")";
        }
        return false;
    }
    // Only the child writes, and only through the handle it is given.
    ::SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = writePipe;
    startup.hStdError = writePipe;
    // Not the process's own stdin: it is not guaranteed to be inheritable, and
    // STARTF_USESTDHANDLES refuses a handle the child cannot inherit. The build
    // reads nothing, so it gets no input at all.
    startup.hStdInput = nullptr;

    PROCESS_INFORMATION process{};
    std::vector<char> mutableCommand(commandLine.begin(), commandLine.end());
    mutableCommand.push_back('\0');

    // Hidden: the output belongs in the panel, and the tool has no window of its
    // own. Inheriting the pipe is what lets the editor show it while it runs.
    const BOOL created = ::CreateProcessA(program.c_str(), mutableCommand.data(), nullptr, nullptr,
                                          TRUE, CREATE_NO_WINDOW, nullptr, projectRoot.c_str(),
                                          &startup, &process);
    ::CloseHandle(writePipe);
    if (created == 0) {
        ::CloseHandle(readPipe);
        if (error != nullptr) {
            *error = "could not start " + program + " (error " +
                     std::to_string(::GetLastError()) + ")";
        }
        return false;
    }

    ::CloseHandle(process.hThread);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        lines_.clear();
        running_ = true;
        finished_ = false;
        stopRequested_ = false;
        exitCode_ = -1;
    }
    process_ = process.hProcess;
    thread_ = std::thread(&BuildRun::readerLoop, this, process.hProcess, readPipe);
    return true;
}

void BuildRun::readerLoop(void* process, void* readPipe) {
    HANDLE pipe = static_cast<HANDLE>(readPipe);
    std::string pending;
    char buffer[4096];
    DWORD read = 0;
    while (::ReadFile(pipe, buffer, sizeof(buffer), &read, nullptr) != 0 && read > 0) {
        pending.append(buffer, read);
        std::size_t end = pending.find('\n');
        while (end != std::string::npos) {
            std::string line = pending.substr(0, end);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            std::lock_guard<std::mutex> lock(mutex_);
            lines_.push_back(line);
            if (lines_.size() > kMaxLines) {
                lines_.erase(lines_.begin(), lines_.begin() + 1000);
            }
            pending.erase(0, end + 1);
            end = pending.find('\n');
        }
    }
    if (!pending.empty()) {
        std::lock_guard<std::mutex> lock(mutex_);
        lines_.push_back(pending);
    }
    ::CloseHandle(pipe);

    DWORD exitCode = 0;
    ::WaitForSingleObject(static_cast<HANDLE>(process), INFINITE);
    ::GetExitCodeProcess(static_cast<HANDLE>(process), &exitCode);

    std::lock_guard<std::mutex> lock(mutex_);
    exitCode_ = static_cast<int>(exitCode);
    running_ = false;
    finished_ = true;
}

void BuildRun::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_ || stopRequested_) return;
        stopRequested_ = true;
    }
    if (process_ != nullptr) {
        ::TerminateProcess(static_cast<HANDLE>(process_), 1);
    }
}

bool BuildRun::running() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return running_;
}

bool BuildRun::finished() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return finished_;
}

int BuildRun::exitCode() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return exitCode_;
}

std::vector<std::string> BuildRun::lines() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lines_;
}

std::size_t BuildRun::lineCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lines_.size();
}


}  // namespace code
}  // namespace rblx
