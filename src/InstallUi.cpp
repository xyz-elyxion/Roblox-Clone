#include "InstallUi.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>

#include <lineargl/lineargl.h>
#include <LINEARWIN/linearwin.h>

#include "linearui.h"
#include "linearui_impl_linearwin.h"
#include "linearui_impl_opengl3.h"
#include "UiTheme.h"

namespace linear {
namespace ui {
namespace {

constexpr int kWindowWidth = 660;
constexpr int kWindowHeight = 470;
constexpr std::size_t kMaxLines = 400;

// The palette itself lives in UiTheme.h, because every window of this project
// reads it from there and a change in one place should reach all of them.
const LinearVec4 kAccent = theme::accent();
const LinearVec4 kText = theme::text();
const LinearVec4 kMuted = theme::muted();
const LinearVec4 kError = theme::error();
const LinearVec4 kOk = theme::ok();

void applyTheme() { theme::apply(); }

// The system's own proportional font, because the built-in bitmap font looks
// wrong in a window this size. Falls back to it when there is no font file,
// which is a cosmetic problem and not a reason to refuse to install.
void loadFonts(LinearFont** body, LinearFont** heading) {
    LinearUIIO& io = LinearUI::GetIO();
    LinearFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 2;

    const char* candidates[] = {
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
    };

    for (const char* path : candidates) {
        std::ifstream probe(path, std::ios::binary);
        if (!probe.good()) continue;
        probe.close();

        *body = io.Fonts->AddFontFromFileTTF(path, 19.0f, &config);
        if (*body == nullptr) continue;
        *heading = io.Fonts->AddFontFromFileTTF(path, 30.0f, &config);
        if (*heading == nullptr) *heading = *body;
        return;
    }

    *body = io.Fonts->AddFontDefault();
    *heading = *body;
}

// "512 KB", "12.4 MB": sizes are read at a glance, and a raw byte count is not.
std::string formatBytes(long long bytes) {
    const char* const units[] = {"B", "KB", "MB", "GB"};
    double value = static_cast<double>(bytes < 0 ? 0 : bytes);
    int unit = 0;
    while (value >= 1024.0 && unit < 3) {
        value /= 1024.0;
        ++unit;
    }
    char text[32] = {};
    if (unit == 0) {
        std::snprintf(text, sizeof(text), "%.0f %s", value, units[unit]);
    } else {
        std::snprintf(text, sizeof(text), "%.1f %s", value, units[unit]);
    }
    return text;
}

std::string waitingDots() {
    const int count = static_cast<int>(LinearUI::GetTime() * 2.0) % 4;
    return std::string(static_cast<std::size_t>(count), '.');
}

void drawButton(const char* label, const LinearVec4& color) {
    // Toward the light on the dark palette, toward the dark on the light one:
    // a hover that washes a pale button out reads as disabled.
    const float step = theme::isLight() ? -0.07f : 0.08f;
    LinearUI::PushStyleColor(LinearUICol_Button, color);
    LinearUI::PushStyleColor(LinearUICol_ButtonHovered,
                          LinearVec4(std::min(std::max(color.x + step, 0.0f), 1.0f),
                                 std::min(std::max(color.y + step, 0.0f), 1.0f),
                                 std::min(std::max(color.z + step, 0.0f), 1.0f), 1.00f));
    LinearUI::PushStyleColor(LinearUICol_ButtonActive, color);
    LinearUI::Button(label, LinearVec2(132.0f, 0.0f));
    LinearUI::PopStyleColor(3);
}

}  // namespace

// ---------------------------------------------------------------------------
// Report

void Report::line(const std::string& text) {
    std::lock_guard<std::mutex> lock(mutex_);
    lines_.push_back(text);
    problems_.push_back(false);
    if (lines_.size() > kMaxLines) {
        lines_.erase(lines_.begin(), lines_.begin() + 64);
        problems_.erase(problems_.begin(), problems_.begin() + 64);
    }
}

void Report::problem(const std::string& text) {
    std::lock_guard<std::mutex> lock(mutex_);
    lines_.push_back(text);
    problems_.push_back(true);
    if (lines_.size() > kMaxLines) {
        lines_.erase(lines_.begin(), lines_.begin() + 64);
        problems_.erase(problems_.begin(), problems_.begin() + 64);
    }
}

void Report::progress(float fraction) {
    std::lock_guard<std::mutex> lock(mutex_);
    fraction_ = fraction;
}

void Report::progressBytes(long long done, long long total) {
    std::lock_guard<std::mutex> lock(mutex_);
    doneBytes_ = done < 0 ? 0 : done;
    totalBytes_ = total;
    if (total > 0) {
        const double ratio = static_cast<double>(doneBytes_) / static_cast<double>(total);
        fraction_ = static_cast<float>(ratio < 0.0 ? 0.0 : (ratio > 1.0 ? 1.0 : ratio));
    } else {
        fraction_ = -1.0f;
    }
}

void Report::cancel() {
    std::lock_guard<std::mutex> lock(mutex_);
    cancelled_ = true;
}

void Report::finish(bool ok) {
    std::lock_guard<std::mutex> lock(mutex_);
    done_ = true;
    ok_ = ok;
}

bool Report::cancelled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cancelled_;
}

float Report::fraction() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return fraction_;
}

bool Report::done() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return done_;
}

bool Report::ok() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return ok_;
}

Report::Snapshot Report::snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    Snapshot snapshot;
    snapshot.lines = lines_;
    snapshot.problems = problems_;
    snapshot.fraction = fraction_;
    snapshot.doneBytes = doneBytes_;
    snapshot.totalBytes = totalBytes_;
    snapshot.done = done_;
    snapshot.ok = ok_;
    snapshot.cancelled = cancelled_;
    return snapshot;
}

// ---------------------------------------------------------------------------
// The window

Result run(Options& options, const std::vector<std::string>& actions,
           const std::function<bool(Report&)>& job) {
    Result result;
    Report report;

    linearwinSetErrorCallback([](int, const char* description) {
        std::fprintf(stderr, "Linear window: %s\n", description == nullptr ? "unknown" : description);
    });
    if (linearwinInit() == 0) return result;

    linearwinWindowHint(LINEARWIN_CONTEXT_VERSION_MAJOR, 3);
    linearwinWindowHint(LINEARWIN_CONTEXT_VERSION_MINOR, 3);
    linearwinWindowHint(LINEARWIN_OPENGL_PROFILE, LINEARWIN_OPENGL_CORE_PROFILE);

    // A window with inputs in it needs room for them instead of clipping the
    // buttons off the bottom, so it grows with the field count up to a sane limit.
    const int height = std::min(kWindowHeight + static_cast<int>(options.fields.size()) * 54, 900);
    LINEARWINwindow* window =
        linearwinCreateWindow(kWindowWidth, height, options.title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        linearwinTerminate();
        return result;
    }
    linearwinSetWindowSizeLimits(window, 560, 380, LINEARWIN_DONT_CARE, LINEARWIN_DONT_CARE);
    linearwinMakeContextCurrent(window);
    linearwinSwapInterval(1);

    if (linearglLoadGLLoader((LINEARGLloadproc)linearwinGetProcAddress) == 0) {
        linearwinDestroyWindow(window);
        linearwinTerminate();
        return result;
    }

    LINEARUI_CHECKVERSION();
    LinearUI::CreateContext();
    LinearUIIO& io = LinearUI::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= LinearUIConfigFlags_NavEnableKeyboard;
    applyTheme();
    LinearFont* body = nullptr;
    LinearFont* heading = nullptr;
    loadFonts(&body, &heading);

    LinearUI_ImplLinearWin_InitForOpenGL(window, true);
    LinearUI_ImplOpenGL3_Init("#version 330");

    // The window owns the output from here, so the console it was started from
    // has nothing left to say. A shell that launched this keeps its own window.
    if (::GetConsoleWindow() != nullptr) ::FreeConsole();

    char folderBuffer[1024] = {};
    std::snprintf(folderBuffer, sizeof(folderBuffer), "%s", options.folder.c_str());
    bool shortcuts = options.shortcuts;

    // One buffer per field, because the widget edits a fixed-size char array. The
    // value goes back into the options when the job is started, so the tool reads
    // exactly what was typed.
    std::vector<std::vector<char>> fieldBuffers;
    fieldBuffers.reserve(options.fields.size());
    for (const Options::Field& field : options.fields) {
        const std::size_t capacity = field.capacity < 16 ? 16 : field.capacity;
        std::vector<char> buffer(capacity + 1, '\0');
        std::snprintf(buffer.data(), buffer.size(), "%s", field.value.c_str());
        fieldBuffers.push_back(std::move(buffer));
    }

    bool started = false;
    bool closeRequested = false;
    std::thread worker;
    // A transfer rate is a difference between samples, so keep the last one.
    double lastBytes = 0.0;
    double lastSampleTime = 0.0;
    double speed = 0.0;

    auto startJob = [&]() {
        if (started) return;
        // What the user typed is what the tool must use.
        if (options.showFolder) options.folder = folderBuffer;
        options.shortcuts = shortcuts;
        for (std::size_t index = 0; index < options.fields.size(); ++index) {
            options.fields[index].value = fieldBuffers[index].data();
        }
        started = true;
        worker = std::thread([&report, &job]() { report.finish(job(report)); });
    };

    if (options.autoStart) startJob();

    while (!closeRequested) {
        linearwinPollEvents();

        const Report::Snapshot state = report.snapshot();
        if (linearwinWindowShouldClose(window) != 0) {
            // A job in flight must finish: a half-written install with no window
            // left to explain it is the worst outcome here.
            if (state.done || !started) {
                closeRequested = true;
            } else {
                linearwinSetWindowShouldClose(window, LINEARWIN_FALSE);
            }
        }
        if (closeRequested) break;

        LinearUI_ImplOpenGL3_NewFrame();
        LinearUI_ImplLinearWin_NewFrame();
        LinearUI::NewFrame();

        LinearUI::SetNextWindowPos(LinearVec2(0.0f, 0.0f));
        LinearUI::SetNextWindowSize(io.DisplaySize);
        LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(24.0f, 20.0f));
        LinearUI::Begin("##linear-install", nullptr,
                     LinearUIWindowFlags_NoTitleBar | LinearUIWindowFlags_NoResize |
                         LinearUIWindowFlags_NoMove | LinearUIWindowFlags_NoCollapse |
                         LinearUIWindowFlags_NoSavedSettings |
                         LinearUIWindowFlags_NoBringToFrontOnFocus);
        LinearUI::PopStyleVar();

        LinearUI::PushFont(heading);
        LinearUI::TextUnformatted(options.heading.c_str());
        LinearUI::PopFont();

        if (!options.detail.empty()) {
            LinearUI::PushTextWrapPos(LinearUI::GetContentRegionAvail().x);
            LinearUI::TextColored(kMuted, "%s", options.detail.c_str());
            LinearUI::PopTextWrapPos();
        }
        LinearUI::Spacing();
        LinearUI::Separator();
        LinearUI::Spacing();

        if (!started) {
            if (options.showFolder) {
                LinearUI::TextColored(kMuted, "%s", options.folderLabel.c_str());
                LinearUI::SetNextItemWidth(-1.0f);
                LinearUI::InputText("##folder", folderBuffer, sizeof(folderBuffer));
                LinearUI::Spacing();
                if (options.showShortcuts) {
                    LinearUI::Checkbox("Add Play to the Start menu and the desktop", &shortcuts);
                    LinearUI::Spacing();
                }
                LinearUI::Spacing();
            }

            for (std::size_t index = 0; index < options.fields.size(); ++index) {
                const Options::Field& field = options.fields[index];
                LinearUI::TextColored(kMuted, "%s", field.label.c_str());
                LinearUI::SetNextItemWidth(-1.0f);
                char id[32] = {};
                std::snprintf(id, sizeof(id), "##field%zu", index);
                LinearUI::InputText(id, fieldBuffers[index].data(), fieldBuffers[index].size(),
                                  field.secret ? LinearUIInputTextFlags_Password : 0);
                LinearUI::Spacing();
            }
            if (!options.fields.empty()) LinearUI::Spacing();

            const std::string startLabel = options.startLabel;
            if (LinearUI::Button(startLabel.c_str(), LinearVec2(132.0f, 0.0f))) {
                startJob();
            }
            LinearUI::SameLine();
            if (LinearUI::Button(options.cancelLabel.c_str())) {
                result.cancelled = true;
                closeRequested = true;
            }
            LinearUI::Spacing();
            LinearUI::TextColored(kMuted, "This window closes when the work is done.");
        } else {
            const double now = LinearUI::GetTime();
            const float fraction =
                state.fraction < 0.0f ? -1.0f * static_cast<float>(now) : state.fraction;
            char overlay[32] = {};
            if (state.totalBytes > 0) {
                std::snprintf(overlay, sizeof(overlay), "%.0f%%",
                              static_cast<double>(fraction) * 100.0);
            }
            LinearUI::ProgressBar(fraction, LinearVec2(-1.0f, 0.0f), overlay);
            LinearUI::Spacing();

            // A download that knows its size says how far in it is and how fast,
            // smoothed so the number does not flicker with every frame.
            if (now - lastSampleTime > 0.3) {
                if (lastSampleTime > 0.0 && state.doneBytes >= lastBytes) {
                    const double rate = (static_cast<double>(state.doneBytes) - lastBytes) /
                                        (now - lastSampleTime);
                    speed = speed > 0.0 ? speed * 0.6 + rate * 0.4 : rate;
                }
                lastBytes = static_cast<double>(state.doneBytes);
                lastSampleTime = now;
            }

            std::string transfer;
            if (state.doneBytes > 0) {
                transfer = formatBytes(state.doneBytes);
                if (state.totalBytes > 0) transfer += " of " + formatBytes(state.totalBytes);
                if (!state.done && speed > 1.0) {
                    transfer += "  -  " + formatBytes(static_cast<long long>(speed)) + "/s";
                }
            }

            std::string status;
            LinearVec4 statusColor = kMuted;
            if (!state.done) {
                // The click is answered at once even when the file in flight has
                // to finish, so the button never looks dead.
                status = state.cancelled ? "Stopping at the next file boundary..."
                                         : (transfer.empty() ? "Working" + waitingDots() : transfer);
            } else if (state.ok) {
                status = "Finished.";
                statusColor = kOk;
            } else if (state.cancelled) {
                status = "Stopped at the last file boundary.";
            } else {
                status = "That did not finish.";
                statusColor = kError;
            }
            LinearUI::TextColored(statusColor, "%s", status.c_str());
            if (!state.done && !state.cancelled) {
                LinearUI::SameLine();
                if (LinearUI::SmallButton("Cancel")) report.cancel();
            }

            LinearUI::Spacing();
            const float buttonsHeight =
                state.done ? LinearUI::GetFrameHeightWithSpacing() + 6.0f : 0.0f;
            LinearUI::BeginChild("##log", LinearVec2(0.0f, -buttonsHeight), 0,
                              LinearUIWindowFlags_HorizontalScrollbar);
            if (state.lines.empty()) {
                LinearUI::TextColored(kMuted, "Nothing yet.");
            }
            for (std::size_t index = 0; index < state.lines.size(); ++index) {
                const bool problem = index < state.problems.size() && state.problems[index];
                LinearUI::TextColored(problem ? kError : kText, "%s", state.lines[index].c_str());
            }
            LinearUI::SetScrollHereY(1.0f);
            LinearUI::EndChild();

            if (state.done) {
                for (std::size_t index = 0; index < actions.size(); ++index) {
                    if (index > 0) LinearUI::SameLine();
                    if (index == 0) {
                        drawButton(actions[index].c_str(), theme::primaryButton());
                    } else {
                        LinearUI::Button(actions[index].c_str(), LinearVec2(132.0f, 0.0f));
                    }
                    if (LinearUI::IsItemClicked()) {
                        result.action = actions[index];
                        closeRequested = true;
                    }
                }
            }
        }

        LinearUI::End();

        LinearUI::Render();
        int displayWidth = 0;
        int displayHeight = 0;
        linearwinGetFramebufferSize(window, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        LinearUI_ImplOpenGL3_RenderDrawData(LinearUI::GetDrawData());
        linearwinSwapBuffers(window);
    }

    if (worker.joinable()) worker.join();

    LinearUI_ImplOpenGL3_Shutdown();
    LinearUI_ImplLinearWin_Shutdown();
    LinearUI::DestroyContext();
    linearwinDestroyWindow(window);
    linearwinTerminate();

    result.shown = true;
    result.ok = report.ok();
    result.shortcuts = shortcuts;
    if (options.showFolder) result.folder = folderBuffer;
    result.fields.reserve(options.fields.size());
    for (std::size_t index = 0; index < options.fields.size(); ++index) {
        result.fields.push_back(fieldBuffers[index].data());
    }
    // Closing the window before starting is the same answer as pressing Cancel.
    if (!started) result.cancelled = true;
    return result;
}

}  // namespace ui
}  // namespace linear
