#include "ConsoleGui.h"

#include "Environment.h"
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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#include <fcntl.h>
#include <io.h>

#include <lineargl/lineargl.h>
#include <LINEARWIN/linearwin.h>

#include "linearui.h"
#include "linearui_impl_linearwin.h"
#include "linearui_impl_opengl3.h"
#include "UiTheme.h"

namespace linear {
namespace console {
namespace {

constexpr std::size_t kMaxEntries = 2000;
constexpr std::size_t kMaxTailBytes = 96 * 1024;

int g_echoOut = -1;
int g_echoErr = -1;
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
    if (g_echoOut < 0) {
        return;
    }
    const std::string text = line + "\r\n";
    (void)!::_write(g_echoOut, text.data(), static_cast<unsigned>(text.size()));
}

void captureLoop(int readFd) {
    char buffer[4096];
    std::string pending;
    for (;;) {
        const int received = ::_read(readFd, buffer, static_cast<unsigned>(sizeof(buffer)));
        if (received <= 0) {
            break;
        }
        const std::size_t count = static_cast<std::size_t>(received);
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
    localtime_s(&parts, &seconds);
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

// Shared with every other window: UiTheme.h owns the palette and the spacing.
void applyTheme() { linear::ui::theme::apply(); }

void loadConsoleFont() {
    LinearUIIO& io = LinearUI::GetIO();
    LinearFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 1;
    const char* candidates[] = {
        "C:\\Windows\\Fonts\\consola.ttf",
        "C:\\Windows\\Fonts\\cour.ttf",
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

    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::setvbuf(stderr, nullptr, _IONBF, 0);
    g_captureActive.store(true);

    std::thread reader([readFd]() { captureLoop(readFd); });
    reader.detach();
    return true;
}

bool captureActive() { return g_captureActive.load(); }

bool guiAllowed(int argc, char** argv) {
    const std::string disabled = linear::environment::get("LINEAR_NO_GUI");
    if (!disabled.empty() && disabled != "0") {
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
    ::ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

int runConsoleWindow(Host host) {
    linearwinSetErrorCallback([](int, const char* description) {
        std::fprintf(stderr, "Console window: %s\n", description == nullptr ? "unknown" : description);
    });
    if (linearwinInit() == 0) {
        return 1;
    }

    linearwinWindowHint(LINEARWIN_CONTEXT_VERSION_MAJOR, 3);
    linearwinWindowHint(LINEARWIN_CONTEXT_VERSION_MINOR, 3);
    linearwinWindowHint(LINEARWIN_OPENGL_PROFILE, LINEARWIN_OPENGL_CORE_PROFILE);

    LINEARWINwindow* window = linearwinCreateWindow(1040, 680, host.title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        linearwinTerminate();
        return 1;
    }
    linearwinSetWindowSizeLimits(window, 860, 520, LINEARWIN_DONT_CARE, LINEARWIN_DONT_CARE);
    linearwinMakeContextCurrent(window);
    linearwinSwapInterval(1);

    if (linearglLoadGLLoader((LINEARGLloadproc)linearwinGetProcAddress) == 0) {
        linearwinDestroyWindow(window);
        linearwinTerminate();
        return 1;
    }

    LINEARUI_CHECKVERSION();
    LinearUI::CreateContext();
    LinearUIIO& io = LinearUI::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= LinearUIConfigFlags_NavEnableKeyboard;
    applyTheme();
    loadConsoleFont();

    LinearUI_ImplLinearWin_InitForOpenGL(window, true);
    LinearUI_ImplOpenGL3_Init("#version 330");

    // The whole point is replacing the black terminal window, so drop the
    // console once the GUI owns the output. Detaching only affects this
    // process: a shell the server was launched from keeps its own window.
    if (::GetConsoleWindow() != nullptr) {
        ::FreeConsole();
    }

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

    const LinearVec4 infoColor(0.80f, 0.82f, 0.86f, 1.00f);
    const LinearVec4 warnColor(0.96f, 0.78f, 0.36f, 1.00f);
    const LinearVec4 errorColor(0.96f, 0.45f, 0.45f, 1.00f);
    const LinearVec4 okColor(0.42f, 0.86f, 0.60f, 1.00f);
    const LinearVec4 mutedColor(0.55f, 0.58f, 0.64f, 1.00f);

    while (linearwinWindowShouldClose(window) == 0) {
        linearwinPollEvents();

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

        LinearUI_ImplOpenGL3_NewFrame();
        LinearUI_ImplLinearWin_NewFrame();
        LinearUI::NewFrame();

        LinearUI::SetNextWindowPos(LinearVec2(0.0f, 0.0f));
        LinearUI::SetNextWindowSize(io.DisplaySize);
        LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(14.0f, 12.0f));
        LinearUI::Begin("##linear-console", nullptr,
                     LinearUIWindowFlags_NoTitleBar | LinearUIWindowFlags_NoResize |
                         LinearUIWindowFlags_NoMove | LinearUIWindowFlags_NoCollapse |
                         LinearUIWindowFlags_NoSavedSettings |
                         LinearUIWindowFlags_NoBringToFrontOnFocus);
        LinearUI::PopStyleVar();

        // Header: server state, address and the actions that were previously
        // only available by opening a terminal.
        LinearUI::TextColored(servers.empty() ? warnColor : okColor, "%s",
                           servers.empty() ? "ONLINE (IDLE)" : "ONLINE");
        LinearUI::SameLine();
        LinearUI::TextUnformatted(host.siteUrl.c_str());
        LinearUI::SameLine();
        LinearUI::TextColored(mutedColor, "|  %d game server%s running", static_cast<int>(servers.size()),
                           servers.size() == 1 ? "" : "s");

        const float actionsWidth = 380.0f;
        if (LinearUI::GetContentRegionAvail().x > actionsWidth) {
            LinearUI::SameLine(LinearUI::GetWindowWidth() - actionsWidth);
        } else {
            LinearUI::SameLine();
        }
        if (LinearUI::Button("Open site")) {
            openInBrowser(host.siteUrl);
            statusMessage = "Opened " + host.siteUrl + " in the default browser.";
            statusUntil = frameNow + 6000;
        }
        LinearUI::SameLine();
        if (LinearUI::Button("Copy log")) {
            std::string clipboard;
            for (const Entry& entry : entries) {
                clipboard += "[" + formatClock(entry.timeMs) + "] " + entry.text + "\n";
            }
            LinearUI::SetClipboardText(clipboard.c_str());
            statusMessage = "Copied " + std::to_string(entries.size()) + " log lines to the clipboard.";
            statusUntil = frameNow + 6000;
        }
        LinearUI::SameLine();
        if (LinearUI::Button("Clear")) {
            log.clear();
            entries.clear();
            cachedRevision = 0;
            statusMessage = "Cleared the console.";
            statusUntil = frameNow + 6000;
        }
        LinearUI::SameLine();
        if (LinearUI::Button("Quit")) {
            linearwinSetWindowShouldClose(window, LINEARWIN_TRUE);
        }

        LinearUI::Spacing();
        LinearUI::Separator();
        LinearUI::Spacing();

        // Filters
        LinearUI::SetNextItemWidth(240.0f);
        LinearUI::InputTextWithHint("##filter", "Filter lines", filter, sizeof(filter));
        LinearUI::SameLine();
        LinearUI::Checkbox("Info", &showInfo);
        LinearUI::SameLine();
        LinearUI::Checkbox("Warnings", &showWarn);
        LinearUI::SameLine();
        LinearUI::Checkbox("Errors", &showError);
        LinearUI::SameLine();
        LinearUI::Checkbox("Auto-scroll", &autoScroll);
        LinearUI::SameLine();
        LinearUI::TextColored(mutedColor, "%d lines", static_cast<int>(entries.size()));

        LinearUI::Spacing();

        const LinearVec2 available = LinearUI::GetContentRegionAvail();
        const float footerHeight = LinearUI::GetTextLineHeightWithSpacing() + 8.0f;
        const float bodyHeight = std::max(120.0f, available.y - footerHeight);
        const float logWidth = std::max(320.0f, available.x * 0.62f);
        const std::string needle = lowerCopy(filter);

        LinearUI::BeginChild("##log", LinearVec2(logWidth, bodyHeight), 0,
                          LinearUIWindowFlags_HorizontalScrollbar);
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
            const LinearVec4 color = entry.level == Level::Error   ? errorColor
                                 : entry.level == Level::Warn ? warnColor
                                                              : infoColor;
            LinearUI::TextColored(mutedColor, "[%s]", formatClock(entry.timeMs).c_str());
            LinearUI::SameLine();
            LinearUI::TextColored(color, "%s", entry.text.c_str());
        }
        if (visible == 0) {
            LinearUI::TextColored(mutedColor, "Nothing to show yet.");
        }
        if (autoScroll) {
            LinearUI::SetScrollHereY(1.0f);
        }
        LinearUI::EndChild();

        LinearUI::SameLine();
        LinearUI::BeginChild("##servers", LinearVec2(0.0f, bodyHeight));
        LinearUI::TextColored(mutedColor, "GAME SERVERS");
        LinearUI::Separator();

        if (servers.empty()) {
            LinearUI::TextColored(mutedColor,
                               "No game servers are running.\nOpen the site and press Play to start one.");
        }

        for (const ServerRow& row : servers) {
            LinearUI::PushID(row.id.c_str());
            const bool selected = row.id == selectedId;
            if (LinearUI::Selectable(row.title.c_str(), selected, LinearUISelectableFlags_None,
                                  LinearVec2(std::max(80.0f, LinearUI::GetContentRegionAvail().x - 62.0f),
                                         0.0f))) {
                selectedId = row.id;
                tailInstanceId.clear();
                tailReadAt = 0;
                tailText = "(Loading output...)";
            }
            LinearUI::SameLine();
            if (LinearUI::SmallButton("Stop")) {
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
            LinearUI::TextColored(mutedColor, "  :%d  |  pid %ld  |  %d playing  |  up %s  |  %s",
                               row.port, row.pid, row.players,
                               formatUptime(row.startedAtMs, frameNow).c_str(), row.status.c_str());
            LinearUI::PopID();
            LinearUI::Spacing();
        }

        LinearUI::Spacing();
        LinearUI::Separator();
        LinearUI::TextColored(mutedColor, "GAME SERVER OUTPUT");
        const ServerRow* selectedRow = selectedId.empty() ? nullptr : findServer(servers, selectedId);
        if (selectedRow == nullptr) {
            LinearUI::TextColored(mutedColor, "Select a game server above to follow its output.");
        } else {
            LinearUI::TextColored(mutedColor, "game-server-%d.log", selectedRow->port);
            LinearUI::BeginChild("##tail", LinearVec2(0.0f, 0.0f), 0,
                              LinearUIWindowFlags_HorizontalScrollbar);
            LinearUI::TextUnformatted(tailText.c_str());
            LinearUI::EndChild();
        }
        LinearUI::EndChild();

        if (statusUntil > frameNow && !statusMessage.empty()) {
            LinearUI::TextColored(okColor, "%s", statusMessage.c_str());
        } else {
            LinearUI::TextColored(mutedColor,
                               "Closing this window stops the website and every game server it started.");
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

    LinearUI_ImplOpenGL3_Shutdown();
    LinearUI_ImplLinearWin_Shutdown();
    LinearUI::DestroyContext();
    linearwinDestroyWindow(window);
    linearwinTerminate();
    return 0;
}

}  // namespace console
}  // namespace linear
