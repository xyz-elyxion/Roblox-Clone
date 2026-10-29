#include "ConsoleGui.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cfloat>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#include <fcntl.h>
#include <io.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace rblx {
namespace console {
namespace {

constexpr std::size_t kMaxEntries = 2000;
constexpr std::size_t kMaxTailBytes = 96 * 1024;

#if defined(_WIN32)
int g_echoOut = -1;
int g_echoErr = -1;
#else
int g_echoOut = -1;
int g_echoErr = -1;
#endif
std::atomic<bool> g_captureActive{false};

long long nowMilliseconds() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

std::string lowerCopy(const std::string& text) {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

Level inferLevel(const std::string& line) {
    const std::string lower = lowerCopy(line);
    if (lower.find("error") != std::string::npos || lower.find("failed") != std::string::npos ||
        lower.find("could not") != std::string::npos ||
        lower.find("cannot") != std::string::npos) {
        return Level::Error;
    }
    if (lower.find("warn") != std::string::npos || lower.find("timeout") != std::string::npos ||
        lower.find("deprecat") != std::string::npos) {
        return Level::Warn;
    }
    return Level::Info;
}

void echoLine(const std::string& line) {
#ifdef _WIN32
    if (g_echoOut < 0) {
        return;
    }
    const std::string text = line + "\r\n";
    (void)!::_write(g_echoOut, text.data(), static_cast<unsigned>(text.size()));
#else
    if (g_echoOut < 0) {
        return;
    }
    const std::string text = line + "\n";
    (void)!::write(g_echoOut, text.data(), text.size());
#endif
}

void captureLoop(int readFd) {
    char buffer[4096];
    std::string pending;
    for (;;) {
#ifdef _WIN32
        const int received = ::_read(readFd, buffer, static_cast<unsigned>(sizeof(buffer)));
        if (received <= 0) {
            break;
        }
        const std::size_t count = static_cast<std::size_t>(received);
#else
        const ssize_t received = ::read(readFd, buffer, sizeof(buffer));
        if (received <= 0) {
            break;
        }
        const std::size_t count = static_cast<std::size_t>(received);
#endif
        pending.append(buffer, count);
        std::size_t newline = 0;
        while ((newline = pending.find('\n')) != std::string::npos) {
            std::string line = pending.substr(0, newline);
            pending.erase(0, newline + 1);
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.empty()) {
                continue;
            }
            echoLine(line);
            LogBuffer::instance().add(inferLevel(line), line);
        }
        // A message that never ends with a newline must still become visible.
        if (pending.size() > 8192) {
            echoLine(pending);
            LogBuffer::instance().add(inferLevel(pending), pending);
            pending.clear();
        }
    }
    if (!pending.empty()) {
        echoLine(pending);
        LogBuffer::instance().add(inferLevel(pending), pending);
    }
}

std::string formatClock(long long timeMs) {
    const std::time_t seconds = static_cast<std::time_t>(timeMs / 1000);
    std::tm parts{};
#ifdef _WIN32
    localtime_s(&parts, &seconds);
#else
    localtime_r(&seconds, &parts);
#endif
    char text[16];
    std::snprintf(text, sizeof(text), "%02d:%02d:%02d", parts.tm_hour, parts.tm_min,
                  parts.tm_sec);
    return std::string(text);
}

std::string formatUptime(long long startedAtMs, long long now) {
    if (startedAtMs <= 0) {
        return "--:--:--";
    }
    const long long seconds = std::max<long long>(0, (now - startedAtMs) / 1000);
    char text[32];
    std::snprintf(text, sizeof(text), "%02lld:%02lld:%02lld", seconds / 3600,
                  (seconds / 60) % 60, seconds % 60);
    return std::string(text);
}

std::string readTail(const std::string& path, std::size_t maxBytes) {
    if (path.empty()) {
        return "(No captured output: this server was started outside the website.)";
    }
    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        return "(No output yet.)";
    }
    input.seekg(0, std::ios::end);
    const std::streamoff size = input.tellg();
    if (size <= 0) {
        return "(No output yet.)";
    }
    const std::streamoff start =
        size > static_cast<std::streamoff>(maxBytes) ? size - static_cast<std::streamoff>(maxBytes) : 0;
    input.seekg(start, std::ios::beg);
    std::string text(static_cast<std::size_t>(size - start), '\0');
    input.read(&text[0], static_cast<std::streamsize>(text.size()));
    text.resize(static_cast<std::size_t>(input.gcount()));
    if (start > 0) {
        text.insert(0, "...\n");
    }
    if (text.empty()) {
        return "(No output yet.)";
    }
    return text;
}

const ServerRow* findServer(const std::vector<ServerRow>& servers, const std::string& id) {
    for (const ServerRow& row : servers) {
        if (row.id == id) {
            return &row;
        }
    }
    return nullptr;
}

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
    style.FramePadding = ImVec2(8.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.05f, 0.06f, 0.07f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.21f, 0.25f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.16f, 0.19f, 0.23f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.32f, 0.38f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.42f, 0.46f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.28f, 0.32f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.34f, 0.38f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.40f, 0.44f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.42f, 0.86f, 0.72f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.24f, 0.44f, 0.42f, 0.60f);
}

void loadConsoleFont() {
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 1;
    const char* candidates[] = {
#ifdef _WIN32
        "C:\\Windows\\Fonts\\consola.ttf",
        "C:\\Windows\\Fonts\\cour.ttf",
#else
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
#endif
    };
    for (const char* path : candidates) {
        std::ifstream probe(path, std::ios::binary);
        if (!probe.good()) {
            continue;
        }
        probe.close();
        if (io.Fonts->AddFontFromFileTTF(path, 16.0f, &config) != nullptr) {
            return;
        }
    }
    io.Fonts->AddFontDefault();
}

}  // namespace

LogBuffer& LogBuffer::instance() {
    // Intentionally never destroyed: the capture thread may still be writing
    // while the process shuts down.
    static LogBuffer* buffer = new LogBuffer();
    return *buffer;
}

void LogBuffer::add(Level level, const std::string& text) {
    std::lock_guard<std::mutex> lock(mutex_);
    Entry entry;
    entry.timeMs = nowMilliseconds();
    entry.level = level;
    entry.text = text;
    entries_.push_back(std::move(entry));
    if (entries_.size() > kMaxEntries) {
        entries_.erase(entries_.begin(),
                       entries_.begin() + static_cast<std::ptrdiff_t>(entries_.size() - kMaxEntries));
    }
    ++revision_;
}

void LogBuffer::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    entries_.clear();
    ++revision_;
}

std::vector<Entry> LogBuffer::snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_;
}

unsigned long long LogBuffer::revision() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return revision_;
}

std::size_t LogBuffer::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_.size();
}

bool beginCapture() {
    if (g_captureActive.load()) {
        return true;
    }

    std::fflush(stdout);
    std::fflush(stderr);

    int readFd = -1;
#ifdef _WIN32
    int fds[2] = {-1, -1};
    if (::_pipe(fds, 64 * 1024, _O_BINARY) != 0) {
        return false;
    }
    g_echoOut = ::_dup(::_fileno(stdout));
    g_echoErr = ::_dup(::_fileno(stderr));
    if (::_dup2(fds[1], ::_fileno(stdout)) != 0 || ::_dup2(fds[1], ::_fileno(stderr)) != 0) {
        if (g_echoOut >= 0) {
            ::_dup2(g_echoOut, ::_fileno(stdout));
        }
        if (g_echoErr >= 0) {
            ::_dup2(g_echoErr, ::_fileno(stderr));
        }
        ::_close(fds[0]);
        ::_close(fds[1]);
        return false;
    }
    ::_close(fds[1]);
    readFd = fds[0];
#else
    int fds[2] = {-1, -1};
    if (::pipe(fds) != 0) {
        return false;
    }
    g_echoOut = ::dup(STDOUT_FILENO);
    g_echoErr = ::dup(STDERR_FILENO);
    if (::dup2(fds[1], STDOUT_FILENO) < 0 || ::dup2(fds[1], STDERR_FILENO) < 0) {
        if (g_echoOut >= 0) {
            ::dup2(g_echoOut, STDOUT_FILENO);
        }
        if (g_echoErr >= 0) {
            ::dup2(g_echoErr, STDERR_FILENO);
        }
        ::close(fds[0]);
        ::close(fds[1]);
        return false;
    }
    ::close(fds[1]);
    readFd = fds[0];
#endif

    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::setvbuf(stderr, nullptr, _IONBF, 0);
    g_captureActive.store(true);

    std::thread reader([readFd]() { captureLoop(readFd); });
    reader.detach();
    return true;
}

bool captureActive() { return g_captureActive.load(); }

bool guiAllowed(int argc, char** argv) {
    const char* disabled = std::getenv("RBLX_NO_GUI");
    if (disabled != nullptr && disabled[0] != '\0' && std::strcmp(disabled, "0") != 0) {
        return false;
    }
    for (int index = 1; index < argc; ++index) {
        const char* arg = argv[index];
        if (arg == nullptr) {
            continue;
        }
        if (std::strcmp(arg, "--nogui") == 0 || std::strcmp(arg, "--headless") == 0) {
            return false;
        }
    }
    return true;
}

void openInBrowser(const std::string& url) {
    if (url.empty()) {
        return;
    }
#ifdef _WIN32
    ::ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#else
    const std::string command = "xdg-open \"" + url + "\" >/dev/null 2>&1 &";
    (void)!std::system(command.c_str());
#endif
}

int runConsoleWindow(Host host) {
    glfwSetErrorCallback([](int, const char* description) {
        std::fprintf(stderr, "Console window: %s\n", description == nullptr ? "unknown" : description);
    });
    if (glfwInit() == 0) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1040, 680, host.title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return 1;
    }
    glfwSetWindowSizeLimits(window, 860, 520, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    applyTheme();
    loadConsoleFont();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // The whole point is replacing the black terminal window, so drop the
    // console once the GUI owns the output. Detaching only affects this
    // process: a shell the server was launched from keeps its own window.
#ifdef _WIN32
    if (::GetConsoleWindow() != nullptr) {
        ::FreeConsole();
    }
#endif

    LogBuffer& log = LogBuffer::instance();
    std::vector<Entry> entries;
    unsigned long long cachedRevision = 0;
    std::vector<ServerRow> servers;
    long long serversRefreshedAt = 0;
    std::string selectedId;
    std::string tailText = "(Select a running game server to follow its output.)";
    std::string tailInstanceId;
    long long tailReadAt = 0;
    std::string statusMessage;
    long long statusUntil = 0;

    char filter[128] = {};
    bool showInfo = true;
    bool showWarn = true;
    bool showError = true;
    bool autoScroll = true;

    const ImVec4 infoColor(0.80f, 0.82f, 0.86f, 1.00f);
    const ImVec4 warnColor(0.96f, 0.78f, 0.36f, 1.00f);
    const ImVec4 errorColor(0.96f, 0.45f, 0.45f, 1.00f);
    const ImVec4 okColor(0.42f, 0.86f, 0.60f, 1.00f);
    const ImVec4 mutedColor(0.55f, 0.58f, 0.64f, 1.00f);

    while (glfwWindowShouldClose(window) == 0) {
        glfwPollEvents();

        const long long frameNow = nowMilliseconds();

        const unsigned long long revision = log.revision();
        if (revision != cachedRevision) {
            entries = log.snapshot();
            cachedRevision = revision;
        }

        if (host.collectServers && frameNow - serversRefreshedAt >= 500) {
            serversRefreshedAt = frameNow;
            servers.clear();
            host.collectServers(&servers);
            if (!selectedId.empty() && findServer(servers, selectedId) == nullptr) {
                selectedId.clear();
                tailText = "(Select a running game server to follow its output.)";
                tailInstanceId.clear();
            }
        }

        if (!selectedId.empty() && (tailInstanceId != selectedId || frameNow - tailReadAt >= 400)) {
            const ServerRow* row = findServer(servers, selectedId);
            if (row != nullptr) {
                tailInstanceId = selectedId;
                tailReadAt = frameNow;
                tailText = readTail(row->logPath, kMaxTailBytes);
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 12.0f));
        ImGui::Begin("##rblx-console", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::PopStyleVar();

        // Header: server state, address and the actions that were previously
        // only available by opening a terminal.
        ImGui::TextColored(servers.empty() ? warnColor : okColor, "%s",
                           servers.empty() ? "ONLINE (IDLE)" : "ONLINE");
        ImGui::SameLine();
        ImGui::TextUnformatted(host.siteUrl.c_str());
        ImGui::SameLine();
        ImGui::TextColored(mutedColor, "|  %d game server%s running", static_cast<int>(servers.size()),
                           servers.size() == 1 ? "" : "s");

        const float actionsWidth = 380.0f;
        if (ImGui::GetContentRegionAvail().x > actionsWidth) {
            ImGui::SameLine(ImGui::GetWindowWidth() - actionsWidth);
        } else {
            ImGui::SameLine();
        }
        if (ImGui::Button("Open site")) {
            openInBrowser(host.siteUrl);
            statusMessage = "Opened " + host.siteUrl + " in the default browser.";
            statusUntil = frameNow + 6000;
        }
        ImGui::SameLine();
        if (ImGui::Button("Copy log")) {
            std::string clipboard;
            for (const Entry& entry : entries) {
                clipboard += "[" + formatClock(entry.timeMs) + "] " + entry.text + "\n";
            }
            ImGui::SetClipboardText(clipboard.c_str());
            statusMessage = "Copied " + std::to_string(entries.size()) + " log lines to the clipboard.";
            statusUntil = frameNow + 6000;
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            log.clear();
            entries.clear();
            cachedRevision = 0;
            statusMessage = "Cleared the console.";
            statusUntil = frameNow + 6000;
        }
        ImGui::SameLine();
        if (ImGui::Button("Quit")) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Filters
        ImGui::SetNextItemWidth(240.0f);
        ImGui::InputTextWithHint("##filter", "Filter lines", filter, sizeof(filter));
        ImGui::SameLine();
        ImGui::Checkbox("Info", &showInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Warnings", &showWarn);
        ImGui::SameLine();
        ImGui::Checkbox("Errors", &showError);
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &autoScroll);
        ImGui::SameLine();
        ImGui::TextColored(mutedColor, "%d lines", static_cast<int>(entries.size()));

        ImGui::Spacing();

        const ImVec2 available = ImGui::GetContentRegionAvail();
        const float footerHeight = ImGui::GetTextLineHeightWithSpacing() + 8.0f;
        const float bodyHeight = std::max(120.0f, available.y - footerHeight);
        const float logWidth = std::max(320.0f, available.x * 0.62f);
        const std::string needle = lowerCopy(filter);

        ImGui::BeginChild("##log", ImVec2(logWidth, bodyHeight), 0,
                          ImGuiWindowFlags_HorizontalScrollbar);
        int visible = 0;
        for (const Entry& entry : entries) {
            if (entry.level == Level::Info && !showInfo) {
                continue;
            }
            if (entry.level == Level::Warn && !showWarn) {
                continue;
            }
            if (entry.level == Level::Error && !showError) {
                continue;
            }
            if (!needle.empty() && lowerCopy(entry.text).find(needle) == std::string::npos) {
                continue;
            }
            ++visible;
            const ImVec4 color = entry.level == Level::Error   ? errorColor
                                 : entry.level == Level::Warn ? warnColor
                                                              : infoColor;
            ImGui::TextColored(mutedColor, "[%s]", formatClock(entry.timeMs).c_str());
            ImGui::SameLine();
            ImGui::TextColored(color, "%s", entry.text.c_str());
        }
        if (visible == 0) {
            ImGui::TextColored(mutedColor, "Nothing to show yet.");
        }
        if (autoScroll) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("##servers", ImVec2(0.0f, bodyHeight));
        ImGui::TextColored(mutedColor, "GAME SERVERS");
        ImGui::Separator();

        if (servers.empty()) {
            ImGui::TextColored(mutedColor,
                               "No game servers are running.\nOpen the site and press Play to start one.");
        }

        for (const ServerRow& row : servers) {
            ImGui::PushID(row.id.c_str());
            const bool selected = row.id == selectedId;
            if (ImGui::Selectable(row.title.c_str(), selected, ImGuiSelectableFlags_None,
                                  ImVec2(std::max(80.0f, ImGui::GetContentRegionAvail().x - 62.0f),
                                         0.0f))) {
                selectedId = row.id;
                tailInstanceId.clear();
                tailReadAt = 0;
                tailText = "(Loading output...)";
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Stop")) {
                if (host.stopServer && host.stopServer(row.id)) {
                    statusMessage = "Stopped " + row.title + " on port " + std::to_string(row.port) + ".";
                    statusUntil = frameNow + 6000;
                    selectedId.clear();
                    serversRefreshedAt = 0;
                } else {
                    statusMessage = row.title + " was already gone.";
                    statusUntil = frameNow + 6000;
                }
            }
            ImGui::TextColored(mutedColor, "  :%d  |  pid %ld  |  %d playing  |  up %s  |  %s",
                               row.port, row.pid, row.players,
                               formatUptime(row.startedAtMs, frameNow).c_str(), row.status.c_str());
            ImGui::PopID();
            ImGui::Spacing();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(mutedColor, "GAME SERVER OUTPUT");
        const ServerRow* selectedRow = selectedId.empty() ? nullptr : findServer(servers, selectedId);
        if (selectedRow == nullptr) {
            ImGui::TextColored(mutedColor, "Select a game server above to follow its output.");
        } else {
            ImGui::TextColored(mutedColor, "game-server-%d.log", selectedRow->port);
            ImGui::BeginChild("##tail", ImVec2(0.0f, 0.0f), 0,
                              ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::TextUnformatted(tailText.c_str());
            ImGui::EndChild();
        }
        ImGui::EndChild();

        if (statusUntil > frameNow && !statusMessage.empty()) {
            ImGui::TextColored(okColor, "%s", statusMessage.c_str());
        } else {
            ImGui::TextColored(mutedColor,
                               "Closing this window stops the website and every game server it started.");
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

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

}  // namespace console
}  // namespace rblx
