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
//
// The window is laid out like a product page rather than a dialog: the brand,
// the steps the job is taking and the log in a column on the left, a drawn art
// panel on the right, and one bar across the bottom that says what is happening,
// how much has moved and how far through it is.

#include <cstddef>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace linear {
namespace ui {

// What a running job tells the window.
//
// line() and problem() append to the log; problem() also marks the line as a
// failure so it is drawn in red. progress() moves the bar, and a negative
// fraction means "working, no idea how long". progressBytes() is for a download
// or a copy that knows its size, so the bar shows a real percentage and a
// transfer rate instead of the animation. cancelled() is checked by long
// download and copy loops - always between files, never inside one, because a
// half-written file is worse than a slow one.
class Report {
public:
    void line(const std::string& text);
    void problem(const std::string& text);
    void progress(float fraction);
    // `done` bytes of `total`, with a negative total when the size is unknown.
    void progressBytes(long long done, long long total);
    // Which of Options::steps is being worked on, counted from zero. Everything
    // before it is ticked off. A job that never calls it keeps the first step
    // running for as long as it works.
    void stage(int index);
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
        int stage = 0;
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
    int stage_ = 0;
    bool done_ = false;
    bool ok_ = false;
    bool cancelled_ = false;
};

struct Options {
    std::string title = "Linear";
    std::string heading;
    std::string detail;
    std::string startLabel = "Install";
    std::string cancelLabel = "Cancel";
    // An editable install folder. Empty hides the field and the shortcuts
    // checkbox, which is what an uninstall wants: it only asks to confirm.
    std::string folder;
    std::string folderLabel = "Install folder";
    bool showFolder = true;
    bool shortcuts = true;
    // The installer offers the shortcuts; a server setup has none to offer.
    bool showShortcuts = true;
    // A second checkbox under the shortcuts one, for one optional part of the job
    // the tool can do but the user may not want. The installer offers the guard
    // service there, which needs an administrator: the prompt belongs to that step
    // and the user gets to say no before it appears.
    bool showExtra = false;
    bool extra = false;
    std::string extraLabel;
    // Run the job as soon as the window opens, for a tool whose whole purpose is
    // the job rather than a choice.
    bool autoStart = false;

    // The brand block at the top of the left column: a product name, an optional
    // second word drawn in the accent colour, and a line about the product.
    std::string product = "Linear";
    std::string productAccent;
    std::string tagline = "Play. Build. Create.";

    // What the bottom bar calls the job while it runs. Empty uses `heading`,
    // which reads as a command; a job wants a sentence ("Installing Linear...").
    std::string busyLabel;

    // The steps the left column ticks off, in order. Empty leaves the column to
    // the log, which is what a tool with one indivisible job wants.
    struct Step {
        std::string label;
        std::string detail;
    };
    std::vector<Step> steps;

    // One-line inputs drawn above the buttons, in order, before the job starts.
    // The installer asks for nothing but a folder; a server setup asks for a port,
    // a URL and the secrets the install will run with. `secret` masks what is
    // typed, because a password left readable in a window is a password in the
    // next screenshot.
    struct Field {
        std::string label;
        std::string value;
        bool secret = false;
        std::size_t capacity = 256;
    };
    std::vector<Field> fields;
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
    bool extra = false;
    // What was typed into Options::fields, in the same order.
    std::vector<std::string> fields;
};

// Shows the window, runs `job` on a worker thread once it is started, and
// returns when the user closes it. `actions` are the buttons offered after the
// job finishes; the first one is drawn as the primary action.
Result run(Options& options, const std::vector<std::string>& actions,
           const std::function<bool(Report&)>& job);

}  // namespace ui
}  // namespace linear
