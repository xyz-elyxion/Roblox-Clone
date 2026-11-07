#pragma once

// A window for the installer and the updater.
//
// Both tools are command-line programs that a player only ever starts by
// double-clicking, so both can show their work in a window instead of a console.
// The work itself stays in those tools: this module owns a window, keeps it
// drawing while the job runs on another thread, and reports back which button
// was pressed. When no window can be created - a headless machine, a build
// server, a driver that cannot give us a context - run() says so and the caller
// keeps its console behaviour.

#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace rblx {
namespace ui {

// What a running job tells the window.
//
// line() and problem() append to the log; problem() also marks the line as a
// failure so it is drawn in red. progress() moves the bar, and a negative
// fraction means "working, no idea how long". progressBytes() is for a download
// that knows its size, so the bar shows a real percentage and a transfer rate
// instead of the animation. cancelled() is checked by long download and copy
// loops - always between files, never inside one, because a half-written file is
// worse than a slow one.
class Report {
public:
    void line(const std::string& text);
    void problem(const std::string& text);
    void progress(float fraction);
    // `done` bytes of `total`, with a negative total when the size is unknown.
    void progressBytes(long long done, long long total);
    void cancel();
    void finish(bool ok);

    bool cancelled() const;
    float fraction() const;
    bool done() const;
    bool ok() const;

    struct Snapshot {
        std::vector<std::string> lines;
        // Parallel to `lines`: true when the line reports a failure.
        std::vector<bool> problems;
        float fraction = -1.0f;
        long long doneBytes = 0;
        long long totalBytes = -1;
        bool done = false;
        bool ok = false;
        bool cancelled = false;
    };
    Snapshot snapshot() const;

private:
    mutable std::mutex mutex_;
    std::vector<std::string> lines_;
    std::vector<bool> problems_;
    float fraction_ = -1.0f;
    long long doneBytes_ = 0;
    long long totalBytes_ = -1;
    bool done_ = false;
    bool ok_ = false;
    bool cancelled_ = false;
};

struct Options {
    std::string title = "RBLX Clone";
    std::string heading;
    std::string detail;
    std::string startLabel = "Install";
    std::string cancelLabel = "Cancel";
    // An editable install folder. Empty hides the field and the shortcuts
    // checkbox, which is what an uninstall wants: it only asks to confirm.
    std::string folder;
    bool showFolder = true;
    bool shortcuts = true;
    // Run the job as soon as the window opens, for a tool whose whole purpose is
    // the job rather than a choice.
    bool autoStart = false;
};

struct Result {
    // False when no window could be created; the caller must use its console
    // path instead.
    bool shown = false;
    bool ok = false;
    // True when the user dismissed the window before starting the job.
    bool cancelled = false;
    // The label of the button pressed after the job finished, or empty when the
    // window was closed.
    std::string action;
    std::string folder;
    bool shortcuts = true;
};

// Shows the window, runs `job` on a worker thread once it is started, and
// returns when the user closes it. `actions` are the buttons offered after the
// job finishes; the first one is drawn as the primary action.
Result run(Options& options, const std::vector<std::string>& actions,
           const std::function<bool(Report&)>& job);

}  // namespace ui
}  // namespace rblx
