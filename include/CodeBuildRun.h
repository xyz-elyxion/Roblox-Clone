#pragma once

// One build at a time, run from the editor.
//
// The window owns this: it starts a build, polls it once a frame and stops it when
// somebody asks. Nothing here touches the UI or knows what a diagnostic is - the
// lines are collected whole, in order, and CodeBuild reads them.

#include <cstddef>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace rblx {
namespace code {

class BuildRun {
public:
    BuildRun();
    ~BuildRun();

    BuildRun(const BuildRun&) = delete;
    BuildRun& operator=(const BuildRun&) = delete;

    // Starts `program` with `commandLine` in `projectRoot`, with its output on a
    // pipe. A build that is already running is refused rather than replaced: its
    // output would be lost and two builds would write the same files.
    bool start(const std::string& projectRoot, const std::string& program,
               const std::string& commandLine, std::string* error);

    // Ends the child process. The reader still has to finish what was already
    // written, so `finished()` turns true shortly after.
    void stop();

    bool running() const;
    bool finished() const;
    int exitCode() const;
    // The lines so far, in order. A line is only ever appended whole, so the panel
    // never draws half a path.
    std::vector<std::string> lines() const;
    // How many lines there are. The window polls this every frame and only copies
    // the log when the answer changes, because a build prints thousands of lines.
    std::size_t lineCount() const;

private:
    void readerLoop(void* process, void* readPipe);

    mutable std::mutex mutex_;
    std::vector<std::string> lines_;
    bool running_ = false;
    bool finished_ = false;
    bool stopRequested_ = false;
    int exitCode_ = -1;
    void* process_ = nullptr;
    std::thread thread_;
};

}  // namespace code
}  // namespace rblx
