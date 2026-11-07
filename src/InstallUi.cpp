#include "InstallUi.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace rblx {
namespace ui {
namespace {

constexpr int kWindowWidth = 660;
constexpr int kWindowHeight = 470;
constexpr std::size_t kMaxLines = 400;

const ImVec4 kAccent(0.42f, 0.86f, 0.72f, 1.00f);
const ImVec4 kText(0.82f, 0.84f, 0.88f, 1.00f);
const ImVec4 kMuted(0.55f, 0.58f, 0.64f, 1.00f);
const ImVec4 kError(0.96f, 0.45f, 0.45f, 1.00f);
const ImVec4 kOk(0.42f, 0.86f, 0.60f, 1.00f);

// The same dark palette the server console and the editors use, so a person who
// has seen one window of this project recognises the next.
void applyTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.FramePadding = ImVec2(10.0f, 6.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.05f, 0.06f, 0.07f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.21f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.24f, 0.28f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.16f, 0.19f, 0.23f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.32f, 0.38f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.42f, 0.46f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.28f, 0.32f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.34f, 0.38f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.40f, 0.44f, 1.00f);
    colors[ImGuiCol_CheckMark] = kAccent;
    colors[ImGuiCol_SliderGrab] = kAccent;
    colors[ImGuiCol_PlotHistogram] = kAccent;
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.24f, 0.44f, 0.42f, 0.60f);
}

// The system's own proportional font, because the built-in bitmap font looks
// wrong in a window this size. Falls back to it when there is no font file,
// which is a cosmetic problem and not a reason to refuse to install.
void loadFonts(ImFont** body, ImFont** heading) {
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 2;

    const char* candidates[] = {
#ifdef _WIN32
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
#elif defined(__APPLE__)
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
#else
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
#endif
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
    const int count = static_cast<int>(ImGui::GetTime() * 2.0) % 4;
    return std::string(static_cast<std::size_t>(count), '.');
}

void drawButton(const char* label, const ImVec4& color) {
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImVec4(color.x + 0.08f, color.y + 0.06f, color.z + 0.06f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
    ImGui::Button(label, ImVec2(132.0f, 0.0f));
    ImGui::PopStyleColor(3);
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

    glfwSetErrorCallback([](int, const char* description) {
        std::fprintf(stderr, "RBLX window: %s\n", description == nullptr ? "unknown" : description);
    });
    if (glfwInit() == 0) return result;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow* window =
        glfwCreateWindow(kWindowWidth, kWindowHeight, options.title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return result;
    }
    glfwSetWindowSizeLimits(window, 560, 380, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return result;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    applyTheme();
    ImFont* body = nullptr;
    ImFont* heading = nullptr;
    loadFonts(&body, &heading);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // The window owns the output from here, so the console it was started from
    // has nothing left to say. A shell that launched this keeps its own window.
#ifdef _WIN32
    if (::GetConsoleWindow() != nullptr) ::FreeConsole();
#endif

    char folderBuffer[1024] = {};
    std::snprintf(folderBuffer, sizeof(folderBuffer), "%s", options.folder.c_str());
    bool shortcuts = options.shortcuts;

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
        started = true;
        worker = std::thread([&report, &job]() { report.finish(job(report)); });
    };

    if (options.autoStart) startJob();

    while (!closeRequested) {
        glfwPollEvents();

        const Report::Snapshot state = report.snapshot();
        if (glfwWindowShouldClose(window) != 0) {
            // A job in flight must finish: a half-written install with no window
            // left to explain it is the worst outcome here.
            if (state.done || !started) {
                closeRequested = true;
            } else {
                glfwSetWindowShouldClose(window, GLFW_FALSE);
            }
        }
        if (closeRequested) break;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f, 20.0f));
        ImGui::Begin("##rblx-install", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::PopStyleVar();

        ImGui::PushFont(heading);
        ImGui::TextUnformatted(options.heading.c_str());
        ImGui::PopFont();

        if (!options.detail.empty()) {
            ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
            ImGui::TextColored(kMuted, "%s", options.detail.c_str());
            ImGui::PopTextWrapPos();
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (!started) {
            if (options.showFolder) {
                ImGui::TextColored(kMuted, "Install folder");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::InputText("##folder", folderBuffer, sizeof(folderBuffer));
                ImGui::Spacing();
                ImGui::Checkbox("Add Play to the Start menu and the desktop", &shortcuts);
                ImGui::Spacing();
                ImGui::Spacing();
            }

            const std::string startLabel = options.startLabel;
            if (ImGui::Button(startLabel.c_str(), ImVec2(132.0f, 0.0f))) {
                startJob();
            }
            ImGui::SameLine();
            if (ImGui::Button(options.cancelLabel.c_str())) {
                result.cancelled = true;
                closeRequested = true;
            }
            ImGui::Spacing();
            ImGui::TextColored(kMuted, "This window closes when the work is done.");
        } else {
            const double now = ImGui::GetTime();
            const float fraction =
                state.fraction < 0.0f ? -1.0f * static_cast<float>(now) : state.fraction;
            char overlay[32] = {};
            if (state.totalBytes > 0) {
                std::snprintf(overlay, sizeof(overlay), "%.0f%%",
                              static_cast<double>(fraction) * 100.0);
            }
            ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f), overlay);
            ImGui::Spacing();

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
            ImVec4 statusColor = kMuted;
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
            ImGui::TextColored(statusColor, "%s", status.c_str());
            if (!state.done && !state.cancelled) {
                ImGui::SameLine();
                if (ImGui::SmallButton("Cancel")) report.cancel();
            }

            ImGui::Spacing();
            const float buttonsHeight =
                state.done ? ImGui::GetFrameHeightWithSpacing() + 6.0f : 0.0f;
            ImGui::BeginChild("##log", ImVec2(0.0f, -buttonsHeight), 0,
                              ImGuiWindowFlags_HorizontalScrollbar);
            if (state.lines.empty()) {
                ImGui::TextColored(kMuted, "Nothing yet.");
            }
            for (std::size_t index = 0; index < state.lines.size(); ++index) {
                const bool problem = index < state.problems.size() && state.problems[index];
                ImGui::TextColored(problem ? kError : kText, "%s", state.lines[index].c_str());
            }
            ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();

            if (state.done) {
                for (std::size_t index = 0; index < actions.size(); ++index) {
                    if (index > 0) ImGui::SameLine();
                    if (index == 0) {
                        drawButton(actions[index].c_str(), ImVec4(0.16f, 0.44f, 0.38f, 1.00f));
                    } else {
                        ImGui::Button(actions[index].c_str(), ImVec2(132.0f, 0.0f));
                    }
                    if (ImGui::IsItemClicked()) {
                        result.action = actions[index];
                        closeRequested = true;
                    }
                }
            }
        }

        ImGui::End();

        ImGui::Render();
        int displayWidth = 0;
        int displayHeight = 0;
        glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    if (worker.joinable()) worker.join();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    result.shown = true;
    result.ok = report.ok();
    result.shortcuts = shortcuts;
    if (options.showFolder) result.folder = folderBuffer;
    // Closing the window before starting is the same answer as pressing Cancel.
    if (!started) result.cancelled = true;
    return result;
}

}  // namespace ui
}  // namespace rblx
