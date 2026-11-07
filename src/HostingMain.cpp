// RBLXHosting: the hosting panel for a Coolify instance.
//
// Windows only, on purpose. The servers this drives are Linux machines reached
// over SSH, and the person pressing Start sits at a Windows desktop; that is the
// arrangement the platform is built around, so the panel says so and refuses to
// start anywhere else instead of half working. It is still compiled on every
// platform CI builds, which keeps a Linux or macOS machine checking that the
// code in here at least still compiles.
//
// It is not another Coolify: the dashboard, the deploy engine and the templates
// stay in hosting/coolify. This is the slice an operator needs from a desktop -
// which applications exist, whether they are up, and buttons for the four calls
// that matter - against the API those endpoints already expose.
//
// The token is held in memory only. There is no settings file, because a token
// is a credential and this tool has no business writing one to disk.

#include "CoolifyClient.h"
#include "UiTheme.h"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <shellapi.h>
#endif

namespace {

constexpr int kWindowWidth = 920;
constexpr int kWindowHeight = 640;
constexpr int kLogLines = 200;

// What the worker thread and the window exchange. One request is in flight at a
// time, so a single set of fields is enough and the window never queues work.
struct Shared {
    std::mutex mutex;
    std::atomic<bool> busy{false};
    std::string error;
    std::string notice;
    std::vector<rblx::coolify::Application> applications;
    std::string logs;
};

struct Panel {
    Shared shared;
    std::thread worker;
    // Kept by uuid, not by row: a refresh reorders the list and a row index
    // would then point at whatever moved into that place.
    std::string selectedUuid;
    char url[256] = "http://localhost:8000";
    char token[512] = {};
};

std::string trimmed(const std::string& value) {
    const char* whitespace = " \t\r\n";
    const std::size_t first = value.find_first_not_of(whitespace);
    if (first == std::string::npos) return std::string();
    const std::size_t last = value.find_last_not_of(whitespace);
    return value.substr(first, last - first + 1);
}

// What was typed, as a config, or the reason it cannot be one yet.
bool configFrom(const Panel& panel, rblx::coolify::Config* config, std::string* error) {
    std::string base;
    if (!rblx::coolify::normalizeBaseUrl(panel.url, &base, error)) return false;
    const std::string token = trimmed(panel.token);
    if (token.empty()) {
        *error = "enter an API token: Coolify's Keys & Tokens -> API tokens";
        return false;
    }
    config->baseUrl = base;
    config->token = token;
    return true;
}

// Runs one request off the draw thread, so a slow or unreachable server does not
// freeze the window. The previous worker is always finished before a new one
// starts: begin() is only reached while `busy` is false.
void begin(Panel& panel, std::function<void(Shared&)> job) {
    if (panel.worker.joinable()) panel.worker.join();

    Shared* shared = &panel.shared;
    {
        std::lock_guard<std::mutex> lock(shared->mutex);
        shared->error.clear();
        shared->notice.clear();
    }
    shared->busy = true;
    panel.worker = std::thread([shared, job]() {
        job(*shared);
        shared->busy = false;
    });
}

void noteError(Shared& shared, const std::string& text) {
    std::lock_guard<std::mutex> lock(shared.mutex);
    shared.error = text;
}

void noteSuccess(Shared& shared, const std::string& text) {
    std::lock_guard<std::mutex> lock(shared.mutex);
    shared.notice = text;
}

// Only the Windows font search uses this, so a build that compiles the panel
// without running it would otherwise warn about an unused helper.
[[maybe_unused]] bool fileExists(const char* path) {
    std::FILE* file = std::fopen(path, "rb");
    if (file == nullptr) return false;
    std::fclose(file);
    return true;
}

// The system's own proportional font, like the installer uses: the built-in
// bitmap font is hard to read at this size. A build without it still runs.
void loadFonts() {
    ImGuiIO& io = ImGui::GetIO();
#ifdef _WIN32
    const char* candidates[] = {
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\tahoma.ttf",
    };
    for (const char* path : candidates) {
        if (!fileExists(path)) continue;
        ImFontConfig config;
        config.OversampleH = 2;
        config.OversampleV = 1;
        if (io.Fonts->AddFontFromFileTTF(path, 17.0f, &config) != nullptr) return;
    }
#endif
    io.Fonts->AddFontDefault();
}

void openInBrowser(const std::string& url) {
#ifdef _WIN32
    ::ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#else
    // Unreachable: runPanel() is only called on Windows. Keeping the branch
    // makes the file compile everywhere, which is what CI checks.
    (void)url;
#endif
}

void drawPanel(Panel& panel) {
    // A copy taken under the lock, so the worker can finish mid-frame without
    // the vectors changing underneath ImGui.
    std::vector<rblx::coolify::Application> applications;
    std::string error;
    std::string notice;
    std::string logs;
    {
        std::lock_guard<std::mutex> lock(panel.shared.mutex);
        applications = panel.shared.applications;
        error = panel.shared.error;
        notice = panel.shared.notice;
        logs = panel.shared.logs;
    }
    const bool busy = panel.shared.busy.load();

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("RBLX Hosting", nullptr, flags);

    // The address and the token are the whole login story, so they sit at the
    // top and stay visible while the list below changes.
    ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::text());
    ImGui::TextUnformatted("Hosting");
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::muted());
    ImGui::TextWrapped("The applications a Coolify instance runs. Windows only; the dashboard itself "
                       "lives in hosting/coolify on the Linux machine.");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    ImGui::PushItemWidth(360.0f);
    ImGui::InputTextWithHint("##server", "http://host:8000", panel.url, sizeof(panel.url));
    ImGui::SameLine();
    ImGui::PushItemWidth(260.0f);
    ImGui::InputTextWithHint("##token", "API token", panel.token, sizeof(panel.token),
                             ImGuiInputTextFlags_Password);
    ImGui::PopItemWidth();
    ImGui::PopItemWidth();

    rblx::coolify::Config config;
    std::string configError;
    const bool configured = configFrom(panel, &config, &configError);
    if (!busy && !configured && panel.url[0] != '\0') {
        // Only after something has been typed: an empty window should not open
        // shouting that the address is missing.
        ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::muted());
        ImGui::TextUnformatted(configError.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::BeginDisabled(busy || !configured);
    if (ImGui::Button("Connect", ImVec2(110.0f, 0.0f))) {
        begin(panel, [config](Shared& shared) {
            std::vector<rblx::coolify::Application> listed;
            std::string failure;
            if (rblx::coolify::listApplications(config, &listed, &failure)) {
                {
                    std::lock_guard<std::mutex> lock(shared.mutex);
                    shared.applications = std::move(listed);
                }
                noteSuccess(shared, "connected");
            } else {
                noteError(shared, failure);
            }
        });
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh", ImVec2(110.0f, 0.0f))) {
        begin(panel, [config](Shared& shared) {
            std::vector<rblx::coolify::Application> listed;
            std::string failure;
            if (rblx::coolify::listApplications(config, &listed, &failure)) {
                {
                    std::lock_guard<std::mutex> lock(shared.mutex);
                    shared.applications = std::move(listed);
                }
                noteSuccess(shared, "list refreshed");
            } else {
                noteError(shared, failure);
            }
        });
    }
    ImGui::SameLine();
    if (ImGui::Button("Open dashboard", ImVec2(150.0f, 0.0f))) {
        openInBrowser(config.baseUrl);
    }
    ImGui::EndDisabled();

    if (busy) {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::muted());
        ImGui::TextUnformatted("working...");
        ImGui::PopStyleColor();
    } else if (!error.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::error());
        ImGui::TextWrapped("%s", error.c_str());
        ImGui::PopStyleColor();
    } else if (!notice.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::ok());
        ImGui::TextWrapped("%s", notice.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::Separator();

    const rblx::coolify::Application* selected = nullptr;
    for (const rblx::coolify::Application& application : applications) {
        if (application.uuid == panel.selectedUuid) {
            selected = &application;
            break;
        }
    }

    // The list gets the room the buttons and the log pane below do not need.
    const float tableHeight = ImGui::GetContentRegionAvail().y - 190.0f;
    const ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                       ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
    if (ImGui::BeginTable("applications", 3, tableFlags, ImVec2(0.0f, tableHeight))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Application", ImGuiTableColumnFlags_WidthStretch, 0.42f);
        ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthStretch, 0.23f);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthStretch, 0.35f);
        ImGui::TableHeadersRow();

        for (const rblx::coolify::Application& application : applications) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (ImGui::Selectable(application.name.c_str(), &application == selected,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                panel.selectedUuid = application.uuid;
            }
            ImGui::TableSetColumnIndex(1);
            ImGui::PushStyleColor(ImGuiCol_Text, application.running() ? rblx::ui::theme::ok()
                                                                      : rblx::ui::theme::muted());
            ImGui::TextUnformatted(application.status.empty() ? "unknown"
                                                             : application.status.c_str());
            ImGui::PopStyleColor();
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(application.uuid.c_str());
        }
        ImGui::EndTable();
    }
    if (applications.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::muted());
        ImGui::TextUnformatted("Nothing listed yet. Connect, or create an application in the dashboard.");
        ImGui::PopStyleColor();
    }

    // Start/Stop/Restart/Logs act on the selection, so they are dead until there
    // is one. Start on a running application is legal in Coolify (it redeploys),
    // which is why it is not disabled by state.
    const bool haveSelection = selected != nullptr;
    const std::string selectedUuid = haveSelection ? selected->uuid : std::string();
    ImGui::BeginDisabled(busy || !configured || !haveSelection);

    const rblx::coolify::Action actions[] = {rblx::coolify::Action::Start,
                                             rblx::coolify::Action::Stop,
                                             rblx::coolify::Action::Restart};
    for (const rblx::coolify::Action action : actions) {
        if (ImGui::Button(rblx::coolify::actionLabel(action), ImVec2(100.0f, 0.0f))) {
            begin(panel, [config, selectedUuid, action](Shared& shared) {
                std::string message;
                std::string failure;
                if (rblx::coolify::performAction(config, selectedUuid, action, &message, &failure)) {
                    noteSuccess(shared, message);
                } else {
                    noteError(shared, failure);
                }
            });
        }
        ImGui::SameLine();
    }
    if (ImGui::Button("Logs", ImVec2(100.0f, 0.0f))) {
        begin(panel, [config, selectedUuid](Shared& shared) {
            std::string text;
            std::string failure;
            if (rblx::coolify::fetchLogs(config, selectedUuid, kLogLines, &text, &failure)) {
                std::lock_guard<std::mutex> lock(shared.mutex);
                shared.logs = std::move(text);
                shared.notice = "logs loaded";
            } else {
                noteError(shared, failure);
            }
        });
    }
    ImGui::EndDisabled();

    ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::muted());
    ImGui::TextUnformatted(selected != nullptr ? selected->name.c_str()
                                               : "select an application to act on it");
    ImGui::PopStyleColor();

    ImGui::BeginChild("logs", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_HorizontalScrollbar);
    if (logs.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::muted());
        ImGui::TextUnformatted("Press Logs to read the container output of the selection.");
        ImGui::PopStyleColor();
    } else {
        ImGui::TextUnformatted(logs.c_str());
    }
    ImGui::EndChild();

    ImGui::End();
}

}  // namespace

// Defined on every platform so the CI compilers check it; main() below is what
// makes the tool Windows only.
int runHostingPanel() {
    glfwSetErrorCallback([](int, const char* description) {
        std::fprintf(stderr, "RBLXHosting: %s\n", description == nullptr ? "window error" : description);
    });
    if (glfwInit() == 0) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow* window =
        glfwCreateWindow(kWindowWidth, kWindowHeight, "RBLX Hosting", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return 1;
    }
    glfwSetWindowSizeLimits(window, 760, 520, GLFW_DONT_CARE, GLFW_DONT_CARE);
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
    io.IniFilename = nullptr;  // nothing of this window is worth remembering
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    rblx::ui::theme::apply();
    loadFonts();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Panel panel;
    const std::string title = "RBLX Hosting";
    glfwSetWindowTitle(window, title.c_str());

    while (glfwWindowShouldClose(window) == 0) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawPanel(panel);

        ImGui::Render();
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        const ImVec4 background = rblx::ui::theme::background();
        glClearColor(background.x, background.y, background.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // A request in flight outlives the window only until the closing click is
    // processed, and the worker only touches Shared, not ImGui, so joining here
    // is all the cleanup it needs.
    if (panel.worker.joinable()) panel.worker.join();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

int main() {
#if !defined(_WIN32)
    std::fprintf(stderr,
                 "RBLXHosting: the hosting panel is a Windows program.\n"
                 "Coolify itself runs on a Linux server (see hosting/README.md);\n"
                 "from this machine, use the dashboard in a browser instead.\n");
    return 2;
#else
    return runHostingPanel();
#endif
}
