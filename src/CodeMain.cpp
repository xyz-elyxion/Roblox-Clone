#define GLM_ENABLE_EXPERIMENTAL

#include "CodeProject.h"
#include "TextEditor.h"
#include "Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

// RBLXCode is the source editor for this repository: the window, the project
// tree, the tabs and the text widget are here, and the parts that can be tested
// without a display live in CodeProject. It opens the folder it is launched from
// (or the one named on the command line) and edits the files in it in place.
namespace {

using rblx::code::FileEntry;
using rblx::code::Language;
using rblx::code::SearchHit;
using rblx::code::SourceDocument;
using rblx::code::TreeNode;

constexpr int kWindowWidth = 1440;
constexpr int kWindowHeight = 880;
constexpr float kSidebarWidth = 340.0f;
constexpr std::size_t kQuickOpenLimit = 40;
constexpr std::size_t kNoFile = static_cast<std::size_t>(-1);

std::string loweredCopy(const std::string& value) {
    std::string out = value;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

// The widget ships definitions for a handful of languages. Anything it has no
// definition for opens as plain text with its real name in the status bar, rather
// than being coloured as C++ and marking the wrong words.
const TextEditor::LanguageDefinition& definitionFor(Language language) {
    static const TextEditor::LanguageDefinition plain;
    switch (language) {
        case Language::Cpp: return TextEditor::LanguageDefinition::CPlusPlus();
        case Language::Glsl: return TextEditor::LanguageDefinition::GLSL();
        case Language::Lua: return TextEditor::LanguageDefinition::Lua();
        case Language::JavaScript:
        case Language::Html:
        case Language::Css:
        case Language::Markdown:
        case Language::Json:
        case Language::Plain:
            break;
    }
    return plain;
}

struct OpenFile {
    std::string relative;
    std::string absolute;
    std::string name;
    SourceDocument document;
    // What the file held when it was loaded or last saved. The tab's star and the
    // close confirmation compare against it, so an edit that is undone by hand
    // stops counting as unsaved.
    std::string savedText;
    std::unique_ptr<TextEditor> editor;
};

struct App {
    std::string root;
    std::vector<FileEntry> entries;
    TreeNode tree;
    std::vector<OpenFile> files;
    std::vector<SearchHit> hits;
    std::string lastQuery;
    std::string status = "Open a folder to start.";
    int active = -1;
    std::size_t pendingClose = kNoFile;

    char query[128] = {};
    char quickFilter[128] = {};
    char folderPath[512] = {};
    char goToLine[16] = {};

    bool quickOpenRequested = false;
    bool goToLineRequested = false;
    bool openFolderRequested = false;
    bool showShortcuts = false;
    bool selectOnOpen = false;
    bool focusQuickFilter = false;
    bool focusQuery = false;
    bool focusFolder = false;
    bool focusGoToLine = false;

    void setRoot(const std::string& path) {
        root = path;
        std::snprintf(folderPath, sizeof(folderPath), "%s", root.c_str());
        refresh();
    }

    void refresh() {
        entries = rblx::code::scanProject(root);
        tree = rblx::code::buildTree(entries);
        hits.clear();
        lastQuery.clear();
        std::size_t fileCount = 0;
        for (const FileEntry& entry : entries) {
            if (!entry.directory) {
                ++fileCount;
            }
        }
        status = "Loaded " + std::to_string(fileCount) + " files";
    }

    bool isDirty(const OpenFile& file) const {
        return file.editor->GetText() != file.savedText;
    }

    void openRelative(const std::string& relative) {
        for (std::size_t i = 0; i < files.size(); ++i) {
            if (files[i].relative == relative) {
                active = static_cast<int>(i);
                selectOnOpen = true;
                return;
            }
        }

        const std::string absolute = rblx::code::joinProjectPath(root, relative);
        std::string bytes;
        std::string error;
        if (!rblx::code::readSourceFile(absolute, &bytes, &error)) {
            status = error;
            return;
        }

        OpenFile file;
        file.relative = relative;
        file.absolute = absolute;
        file.name = rblx::code::fileNameOf(relative);
        file.document = rblx::code::decodeSource(bytes);
        file.savedText = file.document.text;

        file.editor = std::make_unique<TextEditor>();
        file.editor->SetPalette(TextEditor::GetDarkPalette());
        file.editor->SetTabSize(4);
        file.editor->SetShowWhitespaces(false);
        file.editor->SetLanguageDefinition(definitionFor(rblx::code::languageForPath(relative)));
        file.editor->SetText(file.document.text);
        file.editor->SetCursorPosition(TextEditor::Coordinates(0, 0));

        files.push_back(std::move(file));
        active = static_cast<int>(files.size()) - 1;
        selectOnOpen = true;
        status = "Opened " + relative;
    }

    void openAt(const std::string& relative, int line) {
        openRelative(relative);
        if (active < 0) {
            return;
        }
        TextEditor& editor = *files[static_cast<std::size_t>(active)].editor;
        const int clamped = std::max(1, std::min(line, editor.GetTotalLines()));
        editor.SetCursorPosition(TextEditor::Coordinates(clamped - 1, 0));
    }

    void saveFile(int index) {
        if (index < 0 || index >= static_cast<int>(files.size())) {
            return;
        }
        OpenFile& file = files[static_cast<std::size_t>(index)];
        SourceDocument document;
        document.text = file.editor->GetText();
        document.crlf = file.document.crlf;
        std::string error;
        if (!rblx::code::writeSourceFile(file.absolute, rblx::code::encodeSource(document), &error)) {
            status = error;
            return;
        }
        file.savedText = document.text;
        status = "Saved " + file.relative;
    }

    void saveAll() {
        int saved = 0;
        for (std::size_t i = 0; i < files.size(); ++i) {
            if (!isDirty(files[i])) {
                continue;
            }
            saveFile(static_cast<int>(i));
            ++saved;
        }
        status = saved == 0 ? "Nothing to save" : "Saved " + std::to_string(saved) + " files";
    }

    void reloadActive() {
        if (active < 0) {
            return;
        }
        OpenFile& file = files[static_cast<std::size_t>(active)];
        std::string bytes;
        std::string error;
        if (!rblx::code::readSourceFile(file.absolute, &bytes, &error)) {
            status = error;
            return;
        }
        file.document = rblx::code::decodeSource(bytes);
        file.savedText = file.document.text;
        file.editor->SetText(file.document.text);
        file.editor->SetCursorPosition(TextEditor::Coordinates(0, 0));
        status = "Reloaded " + file.relative;
    }

    void closeTab(std::size_t index) {
        if (index >= files.size()) {
            return;
        }
        files.erase(files.begin() + static_cast<std::ptrdiff_t>(index));
        if (files.empty()) {
            active = -1;
        } else if (active > static_cast<int>(index) || active >= static_cast<int>(files.size())) {
            active = std::max(0, static_cast<int>(files.size()) - 1);
        }
    }

    void requestClose(std::size_t index) {
        if (index >= files.size()) {
            return;
        }
        pendingClose = index;
    }

    void drawMenuBar(Window& window) {
        if (!ImGui::BeginMainMenuBar()) {
            return;
        }
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save", "Ctrl+S", false, active >= 0)) {
                saveFile(active);
            }
            if (ImGui::MenuItem("Save all", "Ctrl+Shift+S", false, !files.empty())) {
                saveAll();
            }
            if (ImGui::MenuItem("Reload from disk", "Ctrl+R", false, active >= 0)) {
                reloadActive();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Open folder...")) {
                openFolderRequested = true;
                focusFolder = true;
            }
            if (ImGui::MenuItem("Refresh tree")) {
                refresh();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(window.getNativeWindow(), true);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Go")) {
            if (ImGui::MenuItem("Quick open...", "Ctrl+P")) {
                quickOpenRequested = true;
                focusQuickFilter = true;
            }
            if (ImGui::MenuItem("Go to line...", "Ctrl+G", false, active >= 0)) {
                goToLineRequested = true;
                focusGoToLine = true;
            }
            if (ImGui::MenuItem("Find in project...", "Ctrl+F")) {
                focusQuery = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Keyboard shortcuts")) {
                showShortcuts = true;
            }
            ImGui::EndMenu();
        }
        ImGui::SameLine(ImGui::GetWindowWidth() - 520.0f);
        ImGui::TextDisabled("%s", root.c_str());
        ImGui::EndMainMenuBar();
    }

    void drawSidebar() {
        ImGui::TextDisabled("%s", root.empty() ? "No folder open" : root.c_str());
        ImGui::Separator();

        if (focusQuery) {
            ImGui::SetKeyboardFocusHere();
            focusQuery = false;
        }
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint("##query", "Search in project (Ctrl+F)", query, sizeof(query));

        const std::string wanted = query;
        if (wanted.empty()) {
            hits.clear();
            lastQuery.clear();
        } else if (wanted != lastQuery) {
            // Only when the text changes: scanning every file each frame would
            // stutter while somebody types in a large repository.
            hits = rblx::code::searchProject(root, entries, wanted);
            lastQuery = wanted;
        }

        if (!wanted.empty()) {
            ImGui::TextDisabled("%zu result%s", hits.size(), hits.size() == 1 ? "" : "s");
            const float resultsHeight = std::min(220.0f, ImGui::GetTextLineHeightWithSpacing() * 9.0f);
            ImGui::BeginChild("##results", ImVec2(0.0f, resultsHeight), true);
            for (const SearchHit& hit : hits) {
                ImGui::PushID(hit.relativePath.c_str());
                ImGui::PushID(hit.line);
                const std::string label = hit.relativePath + ":" + std::to_string(hit.line) + "  " +
                                          hit.text;
                if (ImGui::Selectable(label.c_str())) {
                    openAt(hit.relativePath, hit.line);
                }
                ImGui::PopID();
                ImGui::PopID();
            }
            if (hits.empty()) {
                ImGui::TextDisabled("No match");
            }
            ImGui::EndChild();
            ImGui::Separator();
        }

        ImGui::BeginChild("##tree");
        if (!root.empty()) {
            drawTree(tree, true);
        }
        ImGui::EndChild();
    }

    void drawTree(const TreeNode& node, bool firstLevel) {
        for (const TreeNode& child : node.children) {
            if (child.entry.directory) {
                ImGuiTreeNodeFlags flags =
                    ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
                if (firstLevel) {
                    flags |= ImGuiTreeNodeFlags_DefaultOpen;
                }
                const bool open = ImGui::TreeNodeEx(child.entry.relativePath.c_str(), flags,
                                                    "%s", child.entry.name.c_str());
                if (open) {
                    drawTree(child, false);
                    ImGui::TreePop();
                }
                continue;
            }
            ImGui::PushID(child.entry.relativePath.c_str());
            const bool selected =
                active >= 0 && files[static_cast<std::size_t>(active)].relative ==
                                    child.entry.relativePath;
            if (ImGui::Selectable(child.entry.name.c_str(), selected)) {
                openRelative(child.entry.relativePath);
            }
            ImGui::PopID();
        }
    }

    void drawTabs() {
        std::vector<std::size_t> closing;
        if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_AutoSelectNewTabs |
                                             ImGuiTabBarFlags_Reorderable |
                                             ImGuiTabBarFlags_FittingPolicyScroll)) {
            for (std::size_t i = 0; i < files.size(); ++i) {
                OpenFile& file = files[i];
                const bool dirty = isDirty(file);
                const std::string label =
                    (dirty ? "* " : "") + file.name + "###" + file.relative;
                bool open = true;
                ImGuiTabItemFlags flags = 0;
                if (selectOnOpen && static_cast<int>(i) == active) {
                    flags |= ImGuiTabItemFlags_SetSelected;
                }
                if (ImGui::BeginTabItem(label.c_str(), &open, flags)) {
                    active = static_cast<int>(i);
                    ImGui::EndTabItem();
                }
                if (!open) {
                    closing.push_back(i);
                }
            }
            ImGui::EndTabBar();
        }
        selectOnOpen = false;
        // Closed from the end so the indices of the tabs still to close stay valid.
        for (auto it = closing.rbegin(); it != closing.rend(); ++it) {
            requestClose(*it);
        }
    }

    void drawEditorArea() {
        drawTabs();
        if (active < 0 || active >= static_cast<int>(files.size())) {
            ImGui::TextDisabled("Choose a file in the tree, or press Ctrl+P to open one by name.");
            return;
        }

        OpenFile& file = files[static_cast<std::size_t>(active)];
        const float footerHeight = ImGui::GetFrameHeightWithSpacing() +
                                   ImGui::GetStyle().ItemSpacing.y * 2.0f;
        file.editor->Render("##code", ImVec2(0.0f, -footerHeight), true);

        const TextEditor::Coordinates cursor = file.editor->GetCursorPosition();
        const Language language = rblx::code::languageForPath(file.relative);
        ImGui::TextDisabled("%s  |  Ln %d, Col %d  |  %s  |  %d lines  |  %s%s", file.relative.c_str(),
                            cursor.mLine + 1, cursor.mColumn + 1,
                            rblx::code::languageName(language), file.editor->GetTotalLines(),
                            isDirty(file) ? "modified" : "saved",
                            file.document.crlf ? "  |  CRLF" : "");
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status.c_str());
    }

    void handleKeys() {
        ImGuiIO& io = ImGui::GetIO();
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
            if (io.KeyShift) {
                saveAll();
            } else {
                saveFile(active);
            }
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R)) {
            reloadActive();
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_W) && active >= 0) {
            requestClose(static_cast<std::size_t>(active));
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_P)) {
            quickOpenRequested = true;
            focusQuickFilter = true;
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_G)) {
            goToLineRequested = true;
            focusGoToLine = true;
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F)) {
            focusQuery = true;
        }
    }

    void drawQuickOpen() {
        if (!ImGui::BeginPopupModal("Quick open", nullptr, ImGuiWindowFlags_NoSavedSettings)) {
            return;
        }
        ImGui::SetNextWindowSize(ImVec2(620.0f, 440.0f), ImGuiCond_Appearing);
        if (focusQuickFilter) {
            ImGui::SetKeyboardFocusHere();
            focusQuickFilter = false;
        }
        ImGui::SetNextItemWidth(-1.0f);
        const bool submitted = ImGui::InputText("##quick", quickFilter, sizeof(quickFilter),
                                                ImGuiInputTextFlags_EnterReturnsTrue);

        const std::string filter = loweredCopy(quickFilter);
        std::vector<const FileEntry*> matches;
        for (const FileEntry& entry : entries) {
            if (entry.directory) {
                continue;
            }
            if (!filter.empty() && loweredCopy(entry.relativePath).find(filter) == std::string::npos) {
                continue;
            }
            matches.push_back(&entry);
            if (matches.size() >= kQuickOpenLimit) {
                break;
            }
        }

        ImGui::BeginChild("##matches", ImVec2(0.0f, 330.0f), true);
        for (const FileEntry* entry : matches) {
            if (ImGui::Selectable(entry->relativePath.c_str())) {
                openRelative(entry->relativePath);
                ImGui::CloseCurrentPopup();
            }
        }
        if (matches.empty()) {
            ImGui::TextDisabled("No file matches");
        }
        ImGui::EndChild();
        if (submitted && !matches.empty()) {
            openRelative(matches.front()->relativePath);
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    void drawGoToLine() {
        if (!ImGui::BeginPopupModal("Go to line", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            return;
        }
        if (focusGoToLine) {
            ImGui::SetKeyboardFocusHere();
            focusGoToLine = false;
        }
        ImGui::SetNextItemWidth(140.0f);
        const bool submitted = ImGui::InputText("Line", goToLine, sizeof(goToLine),
                                                ImGuiInputTextFlags_EnterReturnsTrue);
        if (submitted && active >= 0) {
            const int line = std::atoi(goToLine);
            if (line > 0) {
                openAt(files[static_cast<std::size_t>(active)].relative, line);
            }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    void drawOpenFolder() {
        if (!ImGui::BeginPopupModal("Open folder", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            return;
        }
        if (focusFolder) {
            ImGui::SetKeyboardFocusHere();
            focusFolder = false;
        }
        ImGui::SetNextItemWidth(520.0f);
        const bool submitted = ImGui::InputText("Folder", folderPath, sizeof(folderPath),
                                                ImGuiInputTextFlags_EnterReturnsTrue);
        if (submitted || ImGui::Button("Open")) {
            std::error_code error;
            if (std::filesystem::is_directory(folderPath, error)) {
                std::string opened = folderPath;
                openFolderRequested = false;
                setRoot(opened);
                status = "Opened " + root;
            } else {
                status = std::string("No folder at ") + folderPath;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    void drawUnsavedPopup() {
        if (!ImGui::BeginPopupModal("Unsaved changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            return;
        }
        if (pendingClose >= files.size()) {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return;
        }
        OpenFile& file = files[pendingClose];
        ImGui::Text("Save %s before closing it?", file.name.c_str());
        if (ImGui::Button("Save and close")) {
            saveFile(static_cast<int>(pendingClose));
            const std::size_t index = pendingClose;
            pendingClose = kNoFile;
            closeTab(index);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Close without saving")) {
            const std::size_t index = pendingClose;
            pendingClose = kNoFile;
            closeTab(index);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            pendingClose = kNoFile;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    void drawShortcuts() {
        if (!showShortcuts) {
            return;
        }
        ImGui::SetNextWindowSize(ImVec2(460.0f, 300.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Keyboard shortcuts", &showShortcuts)) {
            ImGui::TextUnformatted("Ctrl+P   open a file by name");
            ImGui::TextUnformatted("Ctrl+F   search the project");
            ImGui::TextUnformatted("Ctrl+G   go to a line");
            ImGui::TextUnformatted("Ctrl+S   save the file in front");
            ImGui::TextUnformatted("Ctrl+Shift+S   save every open file");
            ImGui::TextUnformatted("Ctrl+R   reload the file from disk");
            ImGui::TextUnformatted("Ctrl+W   close the file in front");
            ImGui::Separator();
            ImGui::TextWrapped(
                "The text widget keeps the editing keys: Ctrl+Z and Ctrl+Y undo and redo, Ctrl+C, "
                "Ctrl+X and Ctrl+V copy, cut and paste, and Ctrl+A selects everything.");
        }
        ImGui::End();
    }

    void drawPopups() {
        if (quickOpenRequested) {
            ImGui::OpenPopup("Quick open");
            quickOpenRequested = false;
        }
        if (goToLineRequested) {
            ImGui::OpenPopup("Go to line");
            goToLineRequested = false;
        }
        if (openFolderRequested) {
            ImGui::OpenPopup("Open folder");
            openFolderRequested = false;
        }
        if (pendingClose != kNoFile && pendingClose < files.size() &&
            isDirty(files[pendingClose])) {
            ImGui::OpenPopup("Unsaved changes");
        } else if (pendingClose != kNoFile) {
            const std::size_t index = pendingClose;
            pendingClose = kNoFile;
            closeTab(index);
        }

        drawQuickOpen();
        drawGoToLine();
        drawOpenFolder();
        drawUnsavedPopup();
        drawShortcuts();
    }

    void draw(Window& window) {
        handleKeys();
        drawMenuBar(window);

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        const ImGuiWindowFlags workspaceFlags =
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoSavedSettings;
        ImGui::Begin("##workspace", nullptr, workspaceFlags);

        ImGui::BeginChild("##sidebar", ImVec2(kSidebarWidth, 0.0f), true);
        drawSidebar();
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("##editor", ImVec2(0.0f, 0.0f), false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        drawEditorArea();
        ImGui::EndChild();

        ImGui::End();
        drawPopups();
    }
};

void beginImGui(Window& window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // The layout is fixed in code, so no imgui.ini is written beside the
    // executable and every checkout opens the same way.
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::GetStyle().ChildRounding = 0.0f;
    ImGui::GetStyle().TabRounding = 3.0f;
    ImGui_ImplGlfw_InitForOpenGL(window.getNativeWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

}  // namespace

int main(int argc, char** argv) {
    try {
        Window window(kWindowWidth, kWindowHeight, "RBLX Clone Code");
        beginImGui(window);

        App app;
        std::string start = argc > 1 ? argv[1] : std::filesystem::current_path().string();
        app.setRoot(start);
        app.status = "Editing " + app.root + " - Ctrl+P opens a file by name";

        while (!window.shouldClose()) {
            window.pollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            app.draw(window);

            ImGui::Render();
            int displayWidth = 0;
            int displayHeight = 0;
            glfwGetFramebufferSize(window.getNativeWindow(), &displayWidth, &displayHeight);
            glViewport(0, 0, displayWidth, displayHeight);
            glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            window.swapBuffers();
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    } catch (const std::exception& error) {
        std::fprintf(stderr, "RBLXCode could not start: %s\n", error.what());
        return 1;
    }
    return 0;
}
