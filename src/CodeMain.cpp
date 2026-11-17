// The Open folder popup hands its dialog this window as the owner, and the HWND
// accessor only exists when linearwin is asked for it before its headers are read.
#define LINEARWIN_EXPOSE_NATIVE_WIN32

#include "CodeBuild.h"
#include "CodeBuildRun.h"
#include "CodeGit.h"
#include "CodeProject.h"
#include "GitRepo.h"
#include "LinearTextEditor.h"
#include "Window.h"

#include <lineargl/lineargl.h>
#include <LINEARWIN/linearwin.h>
#include <LINEARWIN/linearwinnative.h>
// SHBrowseForFolder, the dialog Explorer uses to pick a folder.
#include <shlobj.h>
#include "linearui.h"
#include "linearui_impl_linearwin.h"
#include "linearui_impl_opengl3.h"
#include "UiTheme.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// LinearCode is the source editor for this repository: the window, the project
// tree, the tabs and the text widget are here, and the parts that can be tested
// without a display live in CodeProject. It opens the folder it is launched from
// (or the one named on the command line) and edits the files in it in place.
namespace {

// Where the folder picker starts when the box already holds a path.
int CALLBACK browseStartAt(HWND window, UINT message, LPARAM, LPARAM data) {
    if (message == BFFM_INITIALIZED && data != 0) {
        ::SendMessageA(window, BFFM_SETSELECTIONA, TRUE, data);
    }
    return 0;
}

// The folder the person picked, or empty when they closed the dialog. Typing a
// path is how this started, but a folder is a thing you point at: this is the
// dialog Explorer opens, so nothing here has to list a drive one directory at a
// time. It is COM, which the editor does not otherwise use, so it is initialised
// for the length of the call only.
std::string chooseFolder(HWND owner, const std::string& start) {
    const HRESULT initialised =
        ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    BROWSEINFOA info{};
    info.hwndOwner = owner;
    info.lpszTitle = "Choose the folder to open";
    info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
    info.lpfn = browseStartAt;
    info.lParam = reinterpret_cast<LPARAM>(start.c_str());

    std::string chosen;
    if (LPITEMIDLIST selected = ::SHBrowseForFolderA(&info)) {
        char path[MAX_PATH] = {};
        if (::SHGetPathFromIDListA(selected, path) == TRUE) {
            chosen = path;
        }
        ::CoTaskMemFree(selected);
    }
    if (SUCCEEDED(initialised)) {
        ::CoUninitialize();
    }
    return chosen;
}

using linear::code::FileEntry;
using linear::code::Language;
using linear::code::OutlineSymbol;
using linear::code::SearchHit;
using linear::code::SourceDocument;
using linear::code::TextMatch;
using linear::code::CompletionItem;
using linear::code::TreeNode;
using linear::code::TreeNode;

constexpr int kWindowWidth = 1440;
constexpr int kWindowHeight = 880;
constexpr float kSidebarWidth = 320.0f;
constexpr float kOutlineWidth = 264.0f;
constexpr std::size_t kQuickOpenLimit = 40;
constexpr std::size_t kNoFile = static_cast<std::size_t>(-1);
// How often the open tabs are compared against the disk. A second and a half is
// often enough to notice a git checkout or a generated file without stat-ing
// every tab on every frame.
constexpr double kDiskCheckSeconds = 1.5;

// The widget ships definitions for a handful of languages. Anything it has no
// definition for opens as plain text with its real name in the status bar, rather
// than being coloured as C++ and marking the wrong words.
const LinearTextEditor::LanguageDefinition& definitionFor(Language language) {
    static const LinearTextEditor::LanguageDefinition plain;
    switch (language) {
        case Language::Cpp: return LinearTextEditor::LanguageDefinition::CPlusPlus();
        case Language::Glsl: return LinearTextEditor::LanguageDefinition::GLSL();
        case Language::Lua: return LinearTextEditor::LanguageDefinition::Lua();
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
    // The file's size and modification time when it was read or written, so an
    // edit made outside the editor is noticed instead of being overwritten.
    std::string stamp;
    // Set when the file changed on disk while this buffer had unsaved edits; the
    // editor asks before throwing them away.
    bool stale = false;
    std::unique_ptr<LinearTextEditor> editor;
};

// Where the caret was, for the back and forward keys. Only a jump records one, so
// typing does not fill the list.
struct NavPoint {
    std::string relative;
    int line = 1;
    int column = 0;
};

struct App {
    std::string root;
    std::vector<FileEntry> entries;
    TreeNode tree;
    std::vector<OpenFile> files;
    std::vector<SearchHit> hits;
    std::string lastQuery;
    std::string status = "Open a folder to start.";
    std::string branch;
    int active = -1;
    std::size_t pendingClose = kNoFile;

    // Building from the editor and committing from it. Both tools are part of this
    // checkout - the vendored build tool and the repository code LinearGit drives -
    // so the editor can run them itself instead of sending somebody to a terminal.
    // A release is what the Windows workflow makes: configure, build, the tests,
    // then the packager. The stages run one process at a time, because each one's
    // output is only readable when it is the only thing writing.
    // The dialog is owned by this window, so it stays in front of the editor.
    LINEARWINwindow* owner = nullptr;

    linear::code::BuildRun build;
    std::vector<linear::code::Stage> buildStages;
    std::size_t buildStage = 0;
    std::string buildStageLabel;
    std::vector<std::string> buildLog;   // everything the finished stages printed
    std::vector<std::string> buildLive;  // what the running stage has printed so far
    std::size_t buildLiveCount = 0;
    std::vector<linear::code::Diagnostic> buildDiagnostics;
    std::map<std::string, std::size_t> buildDiagnosticByText;
    std::string buildProgram;
    // The checkout the build belongs to, which is not always the open folder: the
    // editor is normally started from the folder the executables land in.
    std::string buildRoot;
    std::string buildCommand;
    std::string buildPreset = linear::code::kDefaultPreset;
    std::string buildState = "not built yet";
    bool buildFailed = false;
    bool showBuild = true;
    bool buildPending = false;
    // The bar needs a total that outlives the queue: the stages are cleared when
    // the release finishes, and a bar that drops back to zero then would read as a
    // failure. The tool help is kept for the case the checkout has never been
    // bootstrapped, which a downloaded ZIP always is.
    std::size_t buildStageTotal = 0;
    bool buildComplete = false;
    bool buildNeedsBootstrap = false;
    std::string buildBootstrapCommands;

    linear::git::Repo repo;
    bool repoOpen = false;
    std::vector<linear::code::ChangeRow> changes;
    std::string repoNote = "No folder open";
    char commitMessage[256] = {};
    bool showChanges = false;

    char query[128] = {};
    char quickFilter[128] = {};
    char folderPath[512] = {};
    char goToLine[16] = {};

    bool quickOpenRequested = false;
    bool goToLineRequested = false;
    bool symbolPaletteRequested = false;
    bool openFolderRequested = false;
    bool showShortcuts = false;
    bool selectOnOpen = false;
    bool focusQuickFilter = false;
    bool focusSymbolFilter = false;
    bool focusQuery = false;
    bool focusFolder = false;
    bool focusGoToLine = false;

    // Find and replace, for the file in front.
    char findText[256] = {};
    char replaceText[256] = {};
    linear::code::FindOptions findOptions;
    std::vector<TextMatch> findMatches;
    std::string findNote;
    int findIndex = -1;
    bool showFind = false;
    bool showReplaceField = false;
    bool focusFind = false;
    bool focusReplace = false;
    bool findDirty = true;

    // The outline is rebuilt when the buffer changes rather than every frame: a
    // scan of a large file on each frame would show up as stutter.
    std::vector<OutlineSymbol> outlineCache;
    int outlineFile = -1;
    bool outlineDirty = true;
    bool showOutline = true;
    char symbolFilter[128] = {};

    // Completions and documentation for the buffer in front. Both are rebuilt
    // from the text rather than stored, so they never describe an older buffer.
    linear::code::CompletionList completions;
    int completionIndex = 0;
    bool completionOpen = false;
    bool completionRequested = false;
    bool completionJustAccepted = false;
    bool swallowKeysThisFrame = false;
    linear::code::DocEntry hoverPage;
    linear::code::DocEntry docPage;
    bool showDoc = false;
    char docFilter[96] = {};
    std::vector<linear::code::DocEntry> docIndex;
    int docIndexFile = -1;
    bool docIndexDirty = true;
    // A file page for a tooltip is read once per hovered path: a tooltip is drawn
    // every frame it is up, and re-reading the file each time would be wasteful.
    linear::code::DocEntry filePageCache;
    std::string filePagePath;

    // Navigation history and the view settings every editor shares.
    std::vector<NavPoint> history;
    std::size_t historyIndex = 0;
    bool navigating = false;
    int tabSize = 4;
    bool showWhitespace = false;
    // The editor's own palette starts out matching the window's, which comes
    // from the shared settings file; the View menu can still differ from it.
    bool lightTheme = linear::ui::theme::isLight();
    double nextDiskCheck = 0.0;

    void setRoot(const std::string& path) {
        root = path;
        std::snprintf(folderPath, sizeof(folderPath), "%s", root.c_str());
        // Another folder is another repository, and possibly none at all.
        repo = linear::git::Repo();
        repoOpen = false;
        refresh();
    }

    void refresh() {
        entries = linear::code::scanProject(root);
        tree = linear::code::buildTree(entries);
        hits.clear();
        lastQuery.clear();
        branch = linear::code::gitBranchOf(root);
        refreshGit();
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

    // Applies the View menu's settings to every open buffer, so changing the tab
    // width or the theme does not only affect files opened afterwards.
    void applyPreferences() {
        for (OpenFile& file : files) {
            file.editor->SetTabSize(tabSize);
            file.editor->SetShowWhitespaces(showWhitespace);
            file.editor->SetPalette(lightTheme ? LinearTextEditor::GetLightPalette()
                                               : LinearTextEditor::GetDarkPalette());
        }
    }

    void openRelative(const std::string& relative) {
        for (std::size_t i = 0; i < files.size(); ++i) {
            if (files[i].relative == relative) {
                if (static_cast<int>(i) != active) {
                    active = static_cast<int>(i);
                    findDirty = true;
                    outlineDirty = true;
                }
                selectOnOpen = true;
                return;
            }
        }

        const std::string absolute = linear::code::joinProjectPath(root, relative);
        std::string bytes;
        std::string error;
        if (!linear::code::readSourceFile(absolute, &bytes, &error)) {
            status = error;
            return;
        }

        OpenFile file;
        file.relative = relative;
        file.absolute = absolute;
        file.name = linear::code::fileNameOf(relative);
        file.document = linear::code::decodeSource(bytes);
        file.savedText = file.document.text;
        file.stamp = linear::code::fileStamp(absolute);

        file.editor = std::make_unique<LinearTextEditor>();
        file.editor->SetTabSize(tabSize);
        file.editor->SetShowWhitespaces(showWhitespace);
        file.editor->SetPalette(lightTheme ? LinearTextEditor::GetLightPalette()
                                           : LinearTextEditor::GetDarkPalette());
        file.editor->SetLanguageDefinition(definitionFor(linear::code::languageForPath(relative)));
        file.editor->SetText(file.document.text);
        file.editor->SetCursorPosition(LinearTextEditor::Coordinates(0, 0));

        files.push_back(std::move(file));
        active = static_cast<int>(files.size()) - 1;
        selectOnOpen = true;
        findDirty = true;
        outlineDirty = true;
        status = "Opened " + relative;
    }

    // The widget counts a tab as the columns it draws, so a byte offset in the
    // text has to be turned into a place the caret can be put.
    LinearTextEditor::Coordinates coordinatesFor(const std::string& text, std::size_t offset) const {
        int line = 1;
        int column = 1;
        linear::code::lineAndColumn(text, offset, &line, &column);
        const std::size_t start = linear::code::offsetOfLine(text, line);
        std::size_t end = text.find('\n', start);
        if (end == std::string::npos) {
            end = text.size();
        }
        const std::string lineText = text.substr(start, end - start);
        return LinearTextEditor::Coordinates(
            line - 1, linear::code::displayColumn(lineText, static_cast<std::size_t>(column - 1), tabSize));
    }

    void openAt(const std::string& relative, int line, int column = 0) {
        rememberPosition();
        openRelative(relative);
        if (active < 0) {
            return;
        }
        LinearTextEditor& editor = *files[static_cast<std::size_t>(active)].editor;
        const int clamped = std::max(1, std::min(line, editor.GetTotalLines()));
        const std::string text = editor.GetText();
        const std::size_t start = linear::code::offsetOfLine(text, clamped);
        std::size_t end = text.find('\n', start);
        if (end == std::string::npos) {
            end = text.size();
        }
        const std::string lineText = text.substr(start, end - start);
        editor.SetCursorPosition(LinearTextEditor::Coordinates(
            clamped - 1,
            static_cast<int>(linear::code::byteOffsetOfColumn(lineText, column, tabSize))));
    }

    void jumpToLine(int line) {
        if (active < 0) {
            return;
        }
        rememberPosition();
        LinearTextEditor& editor = *files[static_cast<std::size_t>(active)].editor;
        const int clamped = std::max(1, std::min(line, editor.GetTotalLines()));
        editor.SetCursorPosition(LinearTextEditor::Coordinates(clamped - 1, 0));
    }

    // One entry per place a jump left, so Alt+Left returns to what was being read
    // before the jump rather than to wherever the caret happened to be.
    void rememberPosition() {
        if (navigating || active < 0) {
            return;
        }
        const OpenFile& file = files[static_cast<std::size_t>(active)];
        const LinearTextEditor::Coordinates cursor = file.editor->GetCursorPosition();
        NavPoint point;
        point.relative = file.relative;
        point.line = cursor.mLine + 1;
        point.column = cursor.mColumn;
        if (historyIndex < history.size() && history[historyIndex].relative == point.relative &&
            history[historyIndex].line == point.line) {
            return;
        }
        if (historyIndex + 1 < history.size()) {
            history.resize(historyIndex + 1);
        }
        history.push_back(point);
        if (history.size() > 64) {
            history.erase(history.begin());
        }
        historyIndex = history.size() - 1;
    }

    void navigate(int step) {
        if (history.empty()) {
            status = "No navigation history yet";
            return;
        }
        const int wanted = static_cast<int>(historyIndex) + step;
        if (wanted < 0 || wanted >= static_cast<int>(history.size())) {
            status = step < 0 ? "This is the first place you were" : "This is the newest place you were";
            return;
        }
        historyIndex = static_cast<std::size_t>(wanted);
        const NavPoint point = history[historyIndex];
        navigating = true;
        openRelative(point.relative);
        if (active >= 0) {
            LinearTextEditor& editor = *files[static_cast<std::size_t>(active)].editor;
            const int line = std::max(1, std::min(point.line, editor.GetTotalLines()));
            const std::string text = editor.GetText();
            const std::size_t start = linear::code::offsetOfLine(text, line);
            std::size_t end = text.find('\n', start);
            if (end == std::string::npos) {
                end = text.size();
            }
            const std::string lineText = text.substr(start, end - start);
            editor.SetCursorPosition(LinearTextEditor::Coordinates(
                line - 1,
                static_cast<int>(
                    linear::code::byteOffsetOfColumn(lineText, point.column, tabSize))));
        }
        navigating = false;
        status = "Went to " + point.relative + ":" + std::to_string(point.line);
    }

    void cycleTab(int step) {
        if (files.size() < 2) {
            return;
        }
        const int count = static_cast<int>(files.size());
        int next = (std::max(0, active) + step) % count;
        if (next < 0) {
            next += count;
        }
        active = next;
        selectOnOpen = true;
        findDirty = true;
        outlineDirty = true;
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
        if (!linear::code::writeSourceFile(file.absolute, linear::code::encodeSource(document), &error)) {
            status = error;
            return;
        }
        file.savedText = document.text;
        file.stamp = linear::code::fileStamp(file.absolute);
        file.stale = false;
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
        if (!linear::code::readSourceFile(file.absolute, &bytes, &error)) {
            status = error;
            return;
        }
        const LinearTextEditor::Coordinates caret = file.editor->GetCursorPosition();
        file.document = linear::code::decodeSource(bytes);
        file.savedText = file.document.text;
        file.stamp = linear::code::fileStamp(file.absolute);
        file.stale = false;
        file.editor->SetText(file.document.text);
        file.editor->SetCursorPosition(caret);
        findDirty = true;
        outlineDirty = true;
        status = "Reloaded " + file.relative;
    }

    // Loads a clean buffer again when something outside the editor rewrote it. A
    // buffer with edits is only marked, because dropping them silently would be
    // the worst thing an editor could do here.
    void checkDiskChanges() {
        const double now = LinearUI::GetTime();
        if (now < nextDiskCheck) {
            return;
        }
        nextDiskCheck = now + kDiskCheckSeconds;
        for (OpenFile& file : files) {
            const std::string stamp = linear::code::fileStamp(file.absolute);
            if (stamp.empty() || stamp == file.stamp) {
                continue;
            }
            if (isDirty(file)) {
                file.stale = true;
                continue;
            }
            std::string bytes;
            std::string error;
            if (!linear::code::readSourceFile(file.absolute, &bytes, &error)) {
                file.stale = true;
                status = error;
                continue;
            }
            const LinearTextEditor::Coordinates caret = file.editor->GetCursorPosition();
            file.document = linear::code::decodeSource(bytes);
            file.savedText = file.document.text;
            file.stamp = stamp;
            file.stale = false;
            file.editor->SetText(file.document.text);
            file.editor->SetCursorPosition(caret);
            findDirty = true;
            outlineDirty = true;
            status = "Reloaded " + file.relative + " after it changed on disk";
        }
    }

    // ------------------------------------------------------------------ find

    void refreshFind() {
        findMatches.clear();
        findIndex = -1;
        if (active < 0 || findText[0] == '\0') {
            return;
        }
        const OpenFile& file = files[static_cast<std::size_t>(active)];
        findMatches = linear::code::findInText(file.editor->GetText(), findText, findOptions);
        if (findMatches.empty()) {
            return;
        }
        // Start from where the caret is, so the shown count is the match under it
        // rather than the first in the file.
        const int line = file.editor->GetCursorPosition().mLine + 1;
        findIndex = 0;
        for (std::size_t i = 0; i < findMatches.size(); ++i) {
            if (findMatches[i].line >= line) {
                findIndex = static_cast<int>(i);
                break;
            }
        }
    }

    void ensureFindMatches() {
        if (!findDirty) {
            return;
        }
        findDirty = false;
        refreshFind();
    }

    void selectMatch(const TextMatch& match) {
        if (active < 0) {
            return;
        }
        LinearTextEditor& editor = *files[static_cast<std::size_t>(active)].editor;
        const std::string text = editor.GetText();
        editor.SetSelection(coordinatesFor(text, match.offset),
                            coordinatesFor(text, match.offset + match.length));
    }

    void openFindBar(bool replace) {
        showFind = true;
        if (replace) {
            showReplaceField = true;
            focusReplace = true;
            focusFind = false;
        } else if (!showFind || findMatches.empty()) {
            focusFind = true;
        }
        findDirty = true;
        ensureFindMatches();
        if (!showReplaceField) {
            findNote.clear();
        }
        if (findIndex >= 0 && findIndex < static_cast<int>(findMatches.size())) {
            selectMatch(findMatches[static_cast<std::size_t>(findIndex)]);
        }
    }

    void findStep(bool forward) {
        if (!showFind) {
            showFind = true;
            focusFind = !showReplaceField;
            findDirty = true;
        }
        ensureFindMatches();
        if (findMatches.empty()) {
            findNote = findText[0] == '\0' ? "Type what to look for" : "No match";
            return;
        }
        const int count = static_cast<int>(findMatches.size());
        if (findIndex < 0) {
            findIndex = forward ? 0 : count - 1;
        } else {
            findIndex = forward ? (findIndex + 1) % count : (findIndex <= 0 ? count - 1 : findIndex - 1);
        }
        selectMatch(findMatches[static_cast<std::size_t>(findIndex)]);
        findNote.clear();
    }

    void replaceCurrent() {
        if (active < 0) {
            return;
        }
        ensureFindMatches();
        if (findMatches.empty() || findIndex < 0 || findIndex >= static_cast<int>(findMatches.size())) {
            findStep(true);
            return;
        }
        OpenFile& file = files[static_cast<std::size_t>(active)];
        LinearTextEditor& editor = *file.editor;
        const std::string text = editor.GetText();
        const TextMatch match = findMatches[static_cast<std::size_t>(findIndex)];
        // Through the widget's own selection and insert, so Ctrl+Z still takes the
        // replacement back.
        editor.SetSelection(coordinatesFor(text, match.offset),
                            coordinatesFor(text, match.offset + match.length));
        editor.Delete();
        editor.InsertText(replaceText);
        findDirty = true;
        ensureFindMatches();
        findNote = "Replaced one";
        findStep(true);
    }

    void replaceAllMatches() {
        if (active < 0 || findText[0] == '\0') {
            return;
        }
        OpenFile& file = files[static_cast<std::size_t>(active)];
        std::string text = file.editor->GetText();
        const std::size_t changed =
            linear::code::replaceAllInText(&text, findText, replaceText, findOptions);
        if (changed == 0) {
            findNote = "No match";
            return;
        }
        file.editor->SetText(text);
        file.editor->SetCursorPosition(LinearTextEditor::Coordinates(0, 0));
        findDirty = true;
        outlineDirty = true;
        ensureFindMatches();
        // One buffer rewrite, so the widget keeps a single undo step for it.
        findNote = "Replaced " + std::to_string(changed);
        status = findNote + " in " + file.relative;
    }

    // ------------------------------------------------------------- structure

    const std::vector<OutlineSymbol>& activeOutline() {
        if (active < 0) {
            outlineCache.clear();
            outlineFile = -1;
            return outlineCache;
        }
        if (outlineDirty || outlineFile != active) {
            const OpenFile& file = files[static_cast<std::size_t>(active)];
            outlineCache = linear::code::outlineSymbols(
                file.editor->GetText(), linear::code::languageForPath(file.relative));
            outlineFile = active;
            outlineDirty = false;
        }
        return outlineCache;
    }

    void toggleComment() {
        if (active < 0) {
            return;
        }
        OpenFile& file = files[static_cast<std::size_t>(active)];
        LinearTextEditor& editor = *file.editor;
        const std::string prefix =
            linear::code::lineCommentPrefix(linear::code::languageForPath(file.relative));
        if (prefix.empty()) {
            status = "This file has no line comment to toggle.";
            return;
        }

        const LinearTextEditor::Coordinates cursor = editor.GetCursorPosition();
        const std::string lineText = editor.GetCurrentLineText();
        std::size_t marker = 0;
        if (linear::code::lineHasComment(lineText, prefix, &marker)) {
            const int begin = linear::code::displayColumn(lineText, marker, tabSize);
            const int end = linear::code::displayColumn(lineText, marker + prefix.size(), tabSize);
            editor.SetSelection(LinearTextEditor::Coordinates(cursor.mLine, begin),
                                LinearTextEditor::Coordinates(cursor.mLine, end));
            editor.Delete();
            editor.SetCursorPosition(LinearTextEditor::Coordinates(cursor.mLine, begin));
        } else {
            std::size_t indent = 0;
            while (indent < lineText.size() && (lineText[indent] == ' ' || lineText[indent] == '\t')) {
                ++indent;
            }
            const int column = linear::code::displayColumn(lineText, indent, tabSize);
            editor.SetCursorPosition(LinearTextEditor::Coordinates(cursor.mLine, column));
            editor.InsertText(prefix + " ");
        }
        findDirty = true;
        outlineDirty = true;
    }

    void jumpToMatchingBracket() {
        if (active < 0) {
            return;
        }
        OpenFile& file = files[static_cast<std::size_t>(active)];
        LinearTextEditor& editor = *file.editor;
        const std::string text = editor.GetText();
        const LinearTextEditor::Coordinates cursor = editor.GetCursorPosition();
        const std::string lineText = editor.GetCurrentLineText();
        const std::size_t lineStart = linear::code::offsetOfLine(text, cursor.mLine + 1);
        const std::size_t offset =
            lineStart + linear::code::byteOffsetOfColumn(lineText, cursor.mColumn, tabSize);
        const std::size_t other = linear::code::matchingBracket(text, offset);
        if (other == std::string::npos) {
            status = "No matching bracket here.";
            return;
        }
        editor.SetCursorPosition(coordinatesFor(text, other));
        status = "Jumped to the matching bracket";
    }


    // -------------------------------------------------------- documentation

    Language activeLanguage() const {
        if (active < 0) {
            return Language::Plain;
        }
        return linear::code::languageForPath(files[static_cast<std::size_t>(active)].relative);
    }

    std::string activeText() const {
        if (active < 0) {
            return std::string();
        }
        return files[static_cast<std::size_t>(active)].editor->GetText();
    }

    // Where the caret is in bytes. The widget counts columns as it draws them, so
    // a tab has to be turned back into one byte before the text can be asked
    // what is under it.
    std::size_t caretOffset() const {
        if (active < 0) {
            return 0;
        }
        const OpenFile& file = files[static_cast<std::size_t>(active)];
        const std::string text = file.editor->GetText();
        const LinearTextEditor::Coordinates cursor = file.editor->GetCursorPosition();
        const std::size_t lineStart =
            linear::code::offsetOfLine(text, std::max(1, cursor.mLine + 1));
        std::size_t lineEnd = text.find('\n', lineStart);
        if (lineEnd == std::string::npos) {
            lineEnd = text.size();
        }
        const std::string lineText = text.substr(lineStart, lineEnd - lineStart);
        const std::size_t column =
            linear::code::byteOffsetOfColumn(lineText, cursor.mColumn, tabSize);
        return std::min(lineStart + column, text.size());
    }

    void openDoc(const linear::code::DocEntry& page) {
        if (page.name.empty()) {
            status = "Nothing here has a documentation page.";
            return;
        }
        docPage = page;
        showDoc = true;
        status = "Documentation: " + page.name;
    }

    void openDocForFile(const std::string& relative) {
        for (const OpenFile& file : files) {
            if (file.relative == relative) {
                openDoc(linear::code::documentationForFile(relative, file.editor->GetText()));
                return;
            }
        }
        std::string text;
        std::string error;
        if (!linear::code::readSourceFile(linear::code::joinProjectPath(root, relative), &text,
                                        &error)) {
            status = error;
            return;
        }
        openDoc(linear::code::documentationForFile(relative, text));
    }

    // What F1 does: the page for the name under the caret, and the page for the
    // file when the caret is not on a name at all.
    void openDocAtCaret() {
        if (active < 0) {
            status = "Open a file to read its documentation.";
            return;
        }
        const linear::code::DocEntry page =
            linear::code::documentationAt(activeText(), activeLanguage(), caretOffset());
        if (!page.name.empty()) {
            openDoc(page);
            return;
        }
        openDocForFile(files[static_cast<std::size_t>(active)].relative);
    }

    void rebuildDocIndex() {
        docIndex.clear();
        for (const linear::code::DocEntry& page : linear::code::editorPages()) {
            docIndex.push_back(page);
        }
        for (const linear::code::DocEntry& page : linear::code::referencePages(activeLanguage())) {
            docIndex.push_back(page);
        }
        if (active >= 0) {
            const OpenFile& file = files[static_cast<std::size_t>(active)];
            const std::string text = file.editor->GetText();
            const Language language = linear::code::languageForPath(file.relative);
            docIndex.push_back(linear::code::documentationForFile(file.relative, text));
            for (const OutlineSymbol& symbol : linear::code::outlineSymbols(text, language)) {
                linear::code::DocEntry entry;
                entry.name = symbol.name;
                entry.kind = symbol.kind.empty() ? "symbol" : symbol.kind;
                entry.signature = symbol.detail;
                const std::string comment = linear::code::docCommentAbove(text, symbol.line);
                entry.summary = comment.empty()
                                    ? "Declared in this file at line " +
                                          std::to_string(symbol.line) + "."
                                    : comment;
                entry.body = comment.empty() ? entry.summary : comment;
                docIndex.push_back(entry);
            }
        }
        docIndexFile = active;
        docIndexDirty = false;
    }

    // The page for a file, for a tooltip over the tree or a tab. The last one is
    // kept because a tooltip asks on every frame it is up.
    const linear::code::DocEntry& pageForFile(const std::string& relative) {
        if (relative == filePagePath) {
            return filePageCache;
        }
        filePagePath = relative;
        for (const OpenFile& file : files) {
            if (file.relative == relative) {
                filePageCache =
                    linear::code::documentationForFile(relative, file.editor->GetText());
                return filePageCache;
            }
        }
        std::string text;
        std::string error;
        if (!linear::code::readSourceFile(linear::code::joinProjectPath(root, relative), &text,
                                        &error)) {
            text.clear();
        }
        filePageCache = linear::code::documentationForFile(relative, text);
        return filePageCache;
    }

    void drawFileTooltip(const std::string& relative) {
        const linear::code::DocEntry& page = pageForFile(relative);
        LinearUI::BeginTooltip();
        LinearUI::TextUnformatted(relative.c_str());
        LinearUI::Separator();
        LinearUI::TextDisabled("%s", page.signature.c_str());
        LinearUI::PushTextWrapPos(LinearUI::GetFontSize() * 30.0f);
        LinearUI::TextUnformatted(page.summary.c_str());
        LinearUI::PopTextWrapPos();
        if (LinearUI::SmallButton("Open documentation")) {
            openDoc(page);
        }
        LinearUI::EndTooltip();
    }

    void drawHoverTooltip() {
        LinearUI::BeginTooltip();
        LinearUI::TextUnformatted(hoverPage.name.c_str());
        LinearUI::SameLine();
        LinearUI::TextDisabled("%s", hoverPage.kind.c_str());
        LinearUI::Separator();
        if (!hoverPage.signature.empty()) {
            LinearUI::PushStyleColor(LinearUICol_Text, linear::ui::theme::accentStrong());
            LinearUI::TextUnformatted(hoverPage.signature.c_str());
            LinearUI::PopStyleColor();
        }
        LinearUI::PushTextWrapPos(LinearUI::GetFontSize() * 34.0f);
        LinearUI::TextUnformatted(hoverPage.summary.c_str());
        LinearUI::PopTextWrapPos();
        if (LinearUI::SmallButton("Open documentation  (F1)")) {
            openDoc(hoverPage);
        }
        LinearUI::EndTooltip();
    }

    // ---------------------------------------------------------- completions

    void refreshCompletions(bool force) {
        if (active < 0) {
            completionOpen = false;
            return;
        }
        const OpenFile& file = files[static_cast<std::size_t>(active)];
        const linear::code::CompletionList list = linear::code::completionsAt(
            file.editor->GetText(), linear::code::languageForPath(file.relative), caretOffset());
        // A list is not worth a window until a name is being typed, or a dot has
        // just been typed and its members are the point.
        if (list.empty() || (!force && !list.memberAccess && list.prefix.empty())) {
            completionOpen = false;
            completions = list;
            return;
        }
        const bool sameList = completionOpen && completions.tokenStart == list.tokenStart &&
                              completions.prefix == list.prefix;
        completions = list;
        if (!sameList) {
            completionIndex = 0;
        }
        if (completionIndex >= static_cast<int>(completions.items.size())) {
            completionIndex = 0;
        }
        completionOpen = true;
    }

    void acceptCompletion() {
        if (!completionOpen || completionIndex < 0 ||
            completionIndex >= static_cast<int>(completions.items.size()) || active < 0) {
            completionOpen = false;
            return;
        }
        OpenFile& file = files[static_cast<std::size_t>(active)];
        LinearTextEditor& editor = *file.editor;
        const std::string text = editor.GetText();
        const std::size_t start = std::min(completions.tokenStart, text.size());
        const std::size_t end = std::min(start + completions.prefix.size(), text.size());
        // Through the widget's selection and insert, so Ctrl+Z takes it back in one
        // step rather than leaving half a name behind.
        editor.SetSelection(coordinatesFor(text, start), coordinatesFor(text, end));
        editor.Delete();
        editor.InsertText(completions.items[static_cast<std::size_t>(completionIndex)].insertText);
        status = "Completed " + completions.items[static_cast<std::size_t>(completionIndex)].label;
        completionOpen = false;
        completionJustAccepted = true;
        findDirty = true;
        outlineDirty = true;
        docIndexDirty = true;
        filePagePath.clear();
    }

    void handleCompletionKeys() {
        if (!completionOpen) {
            return;
        }
        const int count = static_cast<int>(completions.items.size());
        if (LinearUI::IsKeyPressed(LinearUIKey_Escape)) {
            completionOpen = false;
            swallowKeysThisFrame = true;
            return;
        }
        if (LinearUI::IsKeyPressed(LinearUIKey_DownArrow)) {
            completionIndex = count == 0 ? 0 : (completionIndex + 1) % count;
            swallowKeysThisFrame = true;
        }
        if (LinearUI::IsKeyPressed(LinearUIKey_UpArrow)) {
            completionIndex = count == 0 ? 0 : (completionIndex + count - 1) % count;
            swallowKeysThisFrame = true;
        }
        if (LinearUI::IsKeyPressed(LinearUIKey_Enter) || LinearUI::IsKeyPressed(LinearUIKey_KeypadEnter) ||
            LinearUI::IsKeyPressed(LinearUIKey_Tab)) {
            acceptCompletion();
            swallowKeysThisFrame = true;
        }
    }

    void drawCompletionPopup() {
        if (!completionOpen || completions.items.empty()) {
            return;
        }
        const LinearUIIO& io = LinearUI::GetIO();
        const float width = 430.0f;
        const float height = 236.0f;
        const float x = std::max(8.0f, std::min(io.MousePos.x + 16.0f, io.DisplaySize.x - width - 8.0f));
        const float y = std::max(8.0f, std::min(io.MousePos.y + 20.0f, io.DisplaySize.y - height - 8.0f));
        LinearUI::SetNextWindowPos(LinearVec2(x, y), LinearUICond_Always);
        LinearUI::SetNextWindowSize(LinearVec2(width, height), LinearUICond_Always);
        const LinearUIWindowFlags flags =
            LinearUIWindowFlags_NoTitleBar | LinearUIWindowFlags_NoResize | LinearUIWindowFlags_NoMove |
            LinearUIWindowFlags_NoSavedSettings | LinearUIWindowFlags_NoFocusOnAppearing |
            LinearUIWindowFlags_NoNavInputs;
        LinearUI::Begin("##completions", nullptr, flags);
        LinearUI::TextDisabled("%s  (%zu)", completions.prefix.empty() ? "all names" : completions.prefix.c_str(),
                            completions.items.size());
        LinearUI::Separator();
        LinearUI::BeginChild("##completionlist", LinearVec2(0.0f, -42.0f));
        for (std::size_t i = 0; i < completions.items.size(); ++i) {
            const CompletionItem& item = completions.items[i];
            LinearUI::PushID(static_cast<int>(i));
            const std::string label = item.label + "    " + item.kind;
            if (LinearUI::Selectable(label.c_str(), static_cast<int>(i) == completionIndex)) {
                completionIndex = static_cast<int>(i);
                acceptCompletion();
                LinearUI::PopID();
                break;
            }
            LinearUI::PopID();
        }
        LinearUI::EndChild();
        LinearUI::Separator();
        if (completionIndex >= 0 && completionIndex < static_cast<int>(completions.items.size())) {
            const CompletionItem& item = completions.items[static_cast<std::size_t>(completionIndex)];
            LinearUI::TextDisabled("%s", item.detail.c_str());
            if (!item.documentation.empty()) {
                LinearUI::PushTextWrapPos(0.0f);
                LinearUI::TextUnformatted(item.documentation.c_str());
                LinearUI::PopTextWrapPos();
            }
        }
        LinearUI::End();
    }

    void drawDocumentationWindow() {
        if (!showDoc) {
            return;
        }
        if (docIndexDirty || docIndexFile != active) {
            rebuildDocIndex();
        }
        LinearUI::SetNextWindowSize(LinearVec2(860.0f, 580.0f), LinearUICond_FirstUseEver);
        if (!LinearUI::Begin("Documentation", &showDoc)) {
            LinearUI::End();
            return;
        }

        LinearUI::BeginChild("##doclist", LinearVec2(280.0f, 0.0f), true);
        LinearUI::SetNextItemWidth(-1.0f);
        LinearUI::InputTextWithHint("##docfilter", "Filter pages", docFilter, sizeof(docFilter));
        LinearUI::Separator();
        const std::string filter = docFilter;
        for (std::size_t i = 0; i < docIndex.size(); ++i) {
            const linear::code::DocEntry& page = docIndex[i];
            if (!filter.empty() && linear::code::fuzzyScore(page.name, filter) < 0) {
                continue;
            }
            LinearUI::PushID(static_cast<int>(i));
            const std::string label = page.kind + "   " + page.name;
            if (LinearUI::Selectable(label.c_str(), docPage.name == page.name)) {
                openDoc(page);
            }
            LinearUI::PopID();
        }
        LinearUI::EndChild();

        LinearUI::SameLine();
        LinearUI::BeginChild("##docpage", LinearVec2(0.0f, 0.0f), true);
        if (docPage.name.empty()) {
            LinearUI::TextDisabled(
                "Choose a page, or put the caret on a name and press F1.");
        } else {
            LinearUI::TextUnformatted(docPage.name.c_str());
            LinearUI::SameLine();
            LinearUI::TextDisabled("%s", docPage.kind.c_str());
            LinearUI::Separator();
            if (!docPage.signature.empty()) {
                LinearUI::PushStyleColor(LinearUICol_Text, linear::ui::theme::accentStrong());
                LinearUI::TextUnformatted(docPage.signature.c_str());
                LinearUI::PopStyleColor();
            }
            LinearUI::Separator();
            LinearUI::PushTextWrapPos(0.0f);
            LinearUI::TextUnformatted(docPage.body.c_str());
            LinearUI::PopTextWrapPos();
            if (!docPage.seeAlso.empty()) {
                LinearUI::Separator();
                LinearUI::TextDisabled("See also");
                const std::vector<std::string> links = docPage.seeAlso;
                for (const std::string& name : links) {
                    if (LinearUI::SmallButton(name.c_str())) {
                        const linear::code::DocEntry* page =
                            linear::code::findReferencePage(name, activeLanguage());
                        if (page != nullptr) {
                            openDoc(*page);
                        } else {
                            status = "No page for " + name;
                        }
                    }
                }
            }
        }
        LinearUI::EndChild();
        LinearUI::End();
    }

    // --------------------------------------------------------------- drawing

    void drawMenuBar(Window& window) {
        if (!LinearUI::BeginMainMenuBar()) {
            return;
        }
        const bool hasFile = active >= 0;
        LinearTextEditor* editor = hasFile ? files[static_cast<std::size_t>(active)].editor.get() : nullptr;

        if (LinearUI::BeginMenu("File")) {
            if (LinearUI::MenuItem("Save", "Ctrl+S", false, hasFile)) {
                saveFile(active);
            }
            if (LinearUI::MenuItem("Save all", "Ctrl+Shift+S", false, !files.empty())) {
                saveAll();
            }
            if (LinearUI::MenuItem("Reload from disk", "Ctrl+R", false, hasFile)) {
                reloadActive();
            }
            if (LinearUI::MenuItem("Close tab", "Ctrl+W", false, hasFile)) {
                requestClose(static_cast<std::size_t>(active));
            }
            LinearUI::Separator();
            if (LinearUI::MenuItem("Open folder...")) {
                openFolderRequested = true;
                focusFolder = true;
            }
            if (LinearUI::MenuItem("Refresh tree", "F5")) {
                refresh();
            }
            LinearUI::Separator();
            if (LinearUI::MenuItem("Exit")) {
                linearwinSetWindowShouldClose(window.getNativeWindow(), true);
            }
            LinearUI::EndMenu();
        }
        if (LinearUI::BeginMenu("Edit")) {
            if (LinearUI::MenuItem("Undo", "Ctrl+Z", false, editor != nullptr && editor->CanUndo())) {
                editor->Undo();
            }
            if (LinearUI::MenuItem("Redo", "Ctrl+Y", false, editor != nullptr && editor->CanRedo())) {
                editor->Redo();
            }
            LinearUI::Separator();
            if (LinearUI::MenuItem("Cut", "Ctrl+X", false, editor != nullptr && editor->HasSelection())) {
                editor->Cut();
            }
            if (LinearUI::MenuItem("Copy", "Ctrl+C", false, editor != nullptr && editor->HasSelection())) {
                editor->Copy();
            }
            if (LinearUI::MenuItem("Paste", "Ctrl+V", false, editor != nullptr)) {
                editor->Paste();
            }
            if (LinearUI::MenuItem("Select all", "Ctrl+A", false, editor != nullptr)) {
                editor->SelectAll();
            }
            LinearUI::Separator();
            if (LinearUI::MenuItem("Find in file", "Ctrl+F", false, hasFile)) {
                openFindBar(false);
            }
            if (LinearUI::MenuItem("Replace in file", "Ctrl+H", false, hasFile)) {
                openFindBar(true);
            }
            if (LinearUI::MenuItem("Find next", "F3", false, hasFile)) {
                findStep(true);
            }
            if (LinearUI::MenuItem("Find previous", "Shift+F3", false, hasFile)) {
                findStep(false);
            }
            LinearUI::Separator();
            if (LinearUI::MenuItem("Toggle line comment", "Ctrl+/", false, hasFile)) {
                toggleComment();
            }
            if (LinearUI::MenuItem("Code suggestions", "Ctrl+Space", false, hasFile)) {
                completionRequested = true;
            }
            if (LinearUI::MenuItem("Go to matching bracket", "Ctrl+]", false, hasFile)) {
                jumpToMatchingBracket();
            }
            LinearUI::EndMenu();
        }
        if (LinearUI::BeginMenu("Go")) {
            if (LinearUI::MenuItem("Quick open...", "Ctrl+P")) {
                quickOpenRequested = true;
                focusQuickFilter = true;
            }
            if (LinearUI::MenuItem("Go to line...", "Ctrl+G", false, hasFile)) {
                goToLineRequested = true;
                focusGoToLine = true;
            }
            if (LinearUI::MenuItem("Go to symbol...", "Ctrl+Shift+O", false, hasFile)) {
                symbolPaletteRequested = true;
                focusSymbolFilter = true;
            }
            if (LinearUI::MenuItem("Find in project...", "Ctrl+Shift+F")) {
                focusQuery = true;
            }
            LinearUI::Separator();
            if (LinearUI::MenuItem("Back", "Alt+Left", false, !history.empty())) {
                navigate(-1);
            }
            if (LinearUI::MenuItem("Forward", "Alt+Right", false, !history.empty())) {
                navigate(1);
            }
            LinearUI::Separator();
            if (LinearUI::MenuItem("Next tab", "Ctrl+Tab", false, files.size() > 1)) {
                cycleTab(1);
            }
            if (LinearUI::MenuItem("Previous tab", "Ctrl+Shift+Tab", false, files.size() > 1)) {
                cycleTab(-1);
            }
            LinearUI::EndMenu();
        }
        if (LinearUI::BeginMenu("Build")) {
            if (LinearUI::MenuItem("Build release", "Ctrl+B", false, !root.empty())) {
                startBuild();
            }
            if (LinearUI::MenuItem("Cancel build", "", false, build.running())) {
                stopBuild();
            }
            LinearUI::Separator();
            LinearUI::MenuItem("Build panel", "", &showBuild);
            if (LinearUI::MenuItem("Source control panel", "", showChanges)) {
                showChanges = !showChanges;
                if (showChanges) {
                    refreshGit();
                }
            }
            LinearUI::EndMenu();
        }
        if (LinearUI::BeginMenu("View")) {
            LinearUI::MenuItem("Outline panel", "", &showOutline);
            LinearUI::MenuItem("Show whitespace", "", &showWhitespace);
            LinearUI::MenuItem("Light theme", "", &lightTheme);
            if (LinearUI::BeginMenu("Tab size")) {
                if (LinearUI::MenuItem("2", "", tabSize == 2)) {
                    tabSize = 2;
                }
                if (LinearUI::MenuItem("4", "", tabSize == 4)) {
                    tabSize = 4;
                }
                if (LinearUI::MenuItem("8", "", tabSize == 8)) {
                    tabSize = 8;
                }
                LinearUI::EndMenu();
            }
            // A widget keeps its own copy of these, so the open buffers are told
            // about the change rather than only the next one to be opened.
            applyPreferences();
            LinearUI::EndMenu();
        }
        if (LinearUI::BeginMenu("Help")) {
            if (LinearUI::MenuItem("Documentation", "F1", false, hasFile)) {
                openDocAtCaret();
            }
            if (LinearUI::MenuItem("Documentation window")) {
                showDoc = true;
            }
            if (LinearUI::MenuItem("Keyboard shortcuts")) {
                showShortcuts = true;
            }
            LinearUI::EndMenu();
        }
        LinearUI::SameLine(LinearUI::GetWindowWidth() - 520.0f);
        LinearUI::TextDisabled("%s", root.c_str());
        LinearUI::EndMainMenuBar();
    }

    // ---------------------------------------------------------------------
    // Source control

    // The branch, and every changed path. Reopened rather than cached across a
    // folder change, because the checkout is a property of the folder.
    void refreshGit() {
        changes.clear();
        if (root.empty()) {
            repoOpen = false;
            repoNote = "No folder open";
            return;
        }
        std::string error;
        if (!repo.isOpen() && !repo.open(root, &error)) {
            repoOpen = false;
            repoNote = error;
            return;
        }
        repoOpen = true;
        if (!linear::code::loadChanges(repo, &changes, &error)) {
            repoNote = error;
            return;
        }
        repoNote = linear::code::changesSummary(changes);
        const std::string name = repo.branch();
        if (!name.empty()) {
            branch = name;
        }
    }

    void commitChanges() {
        std::string id;
        std::string error;
        std::size_t committed = 0;
        if (!linear::code::commitEverything(repo, commitMessage, &id, &committed, &error)) {
            status = error;
            return;
        }
        status = "Committed " + std::to_string(committed) +
                 (committed == 1 ? " path as " : " paths as ") +
                 id.substr(0, id.size() < 7 ? id.size() : 7);
        commitMessage[0] = '\0';
        refreshGit();
    }

    void drawSourceControl() {
        const std::string heading =
            !repoOpen ? "Not a repository"
                      : (branch.empty() ? "No commits yet" : branch);
        LinearUI::TextDisabled("%s", heading.c_str());
        LinearUI::TextWrapped("%s", repoNote.c_str());
        if (LinearUI::SmallButton("Refresh status")) {
            refreshGit();
        }
        LinearUI::Separator();

        LinearUI::SetNextItemWidth(-1.0f);
        LinearUI::InputTextWithHint("##commit-message", "Commit message", commitMessage,
                                  sizeof(commitMessage));
        if (!repoOpen) {
            LinearUI::TextDisabled("This folder is not a repository");
        } else if (changes.empty()) {
            LinearUI::TextDisabled("Nothing to commit");
        } else if (LinearUI::Button("Stage all and commit")) {
            commitChanges();
        }
        LinearUI::Separator();

        LinearUI::BeginChild("##changes");
        for (const linear::code::ChangeRow& row : changes) {
            LinearUI::PushID(row.path.c_str());
            const std::string label = linear::code::changeLabel(row) + "  " + row.path;
            if (LinearUI::Selectable(label.c_str(), false)) {
                openRelative(row.path);
            }
            if (LinearUI::IsItemHovered()) {
                LinearUI::SetTooltip("%s", row.path.c_str());
            }
            LinearUI::PopID();
        }
        LinearUI::EndChild();
    }

    // ---------------------------------------------------------------------
    // Build

    void startBuild() {
        showBuild = true;
        if (build.running()) {
            status = "A release is already being built";
            return;
        }
        std::string error;
        buildRoot = linear::code::findBuildRoot(root);
        if (!linear::code::findBuildTool(buildRoot, &buildProgram, &error)) {
            buildStages.clear();
            buildStageTotal = 0;
            buildComplete = false;
            buildNeedsBootstrap = true;
            buildBootstrapCommands =
                "mkdir -p build/linearbuild-tool\n"
                "cd build/linearbuild-tool\n"
                "../../linearbuild/bootstrap --parallel 4\n"
                "cd ../..";
            buildState = error;
            status = error;
            buildFailed = true;
            return;
        }
        buildNeedsBootstrap = false;
        buildBootstrapCommands.clear();
        // The stages the workflow runs, in its order: configure a preset whose
        // folder has never been written, build it, run the tests the build
        // produced, then package - which stamps a new version.
        buildStages = linear::code::releaseStages(buildRoot, buildProgram, buildPreset);
        buildStage = 0;
        buildStageTotal = buildStages.size();
        buildComplete = false;
        buildLog.clear();
        buildLive.clear();
        buildLiveCount = 0;
        buildDiagnostics.clear();
        buildDiagnosticByText.clear();
        buildState = "building a release";
        startStage();
    }

    void startStage() {
        if (buildStage >= buildStages.size()) {
            return;
        }
        const linear::code::Stage& stage = buildStages[buildStage];
        buildStageLabel = "step " + std::to_string(buildStage + 1) + " of " +
                          std::to_string(buildStages.size()) + ": ";
        if (stage.name == "test") {
            buildStageLabel += "test " + std::filesystem::path(stage.program).stem().string();
        } else {
            buildStageLabel += stage.name;
        }
        buildCommand = linear::code::stageCommandLine(stage);
        std::string error;
        if (!build.start(buildRoot, stage.program, buildCommand, &error)) {
            buildState = error;
            status = error;
            buildFailed = true;
            buildStages.clear();
            return;
        }
        buildPending = true;
        buildFailed = false;
        buildLiveCount = 0;
        buildLive.clear();
        status = buildStageLabel + " (" + stage.program + ")";
    }

    void stopBuild() {
        if (!build.running()) {
            return;
        }
        build.stop();
        // The stages stop with the process: the next one is only started when a
        // stage finished, so emptying the list and forgetting the pending stage is
        // what "cancelled" means. The reader still has to finish what was written.
        buildStages.clear();
        buildPending = false;
        buildComplete = false;
        buildStageLabel.clear();
        buildState = "cancelled";
        status = "Build cancelled";
        buildFailed = false;
    }

    // Called once a frame. The log is copied when it grows rather than every frame,
    // and the diagnostics are read once, when the tool exits.
    void pollBuild() {
        if (build.running()) {
            const std::size_t count = build.lineCount();
            if (count != buildLiveCount) {
                buildLive = build.lines();
                buildLiveCount = buildLive.size();
            }
            return;
        }
        if (!buildPending || !build.finished()) {
            return;
        }
        buildPending = false;

        // The finished stage's output joins the log, so the compiler's lines stay
        // readable while the tests and the packager run after it.
        const std::vector<std::string> printed = build.lines();
        buildLog.insert(buildLog.end(), printed.begin(), printed.end());
        buildLive.clear();
        buildLiveCount = 0;
        buildDiagnostics = linear::code::parseDiagnostics(buildLog);
        buildDiagnosticByText.clear();
        std::size_t errors = 0;
        std::size_t warnings = 0;
        for (std::size_t index = 0; index < buildDiagnostics.size(); ++index) {
            const linear::code::Diagnostic& diagnostic = buildDiagnostics[index];
            if (diagnostic.severity == "error") {
                ++errors;
            } else if (diagnostic.severity == "warning") {
                ++warnings;
            }
            buildDiagnosticByText.emplace(diagnostic.text, index);
        }

        const int code = build.exitCode();
        // The link step of LinearCode.exe itself cannot replace a running program.
        // The workflow never trips over it, and a release built from the panel is
        // not a failed one because of it.
        const bool excused = code != 0 && errors == 0 && linear::code::selfLinkBlocked(printed);
        if (code < 0) {
            buildFailed = true;
            buildState = buildStageLabel + ": the program did not run";
            status = buildState;
            buildStages.clear();
            return;
        }
        if (code != 0 && !excused) {
            buildFailed = true;
            buildState = buildStageLabel + " failed (exit " + std::to_string(code) + ")";
            if (errors > 0 || warnings > 0) {
                buildState += " - " + std::to_string(errors) + " error(s), " +
                              std::to_string(warnings) + " warning(s)";
            }
            status = buildState;
            buildStages.clear();
            return;
        }
        if (buildStage + 1 < buildStages.size()) {
            ++buildStage;
            startStage();
            return;
        }
        buildFailed = false;
        buildStage = buildStageTotal;
        buildComplete = true;
        buildStages.clear();
        buildState = "release packaged into dist/linear-windows-release - a new version, with "
                     "update.json and manifest.json";
        if (linear::code::selfLinkBlocked(printed)) {
            buildState += " (the editor is still the copy that was running)";
        }
        status = buildState;
    }

    void openDiagnostic(const linear::code::Diagnostic& diagnostic) {
        const std::string& base = buildRoot.empty() ? root : buildRoot;
        const std::string absolute = linear::code::resolveDiagnosticPath(base, diagnostic.path);
        std::string relative = linear::code::relativePathOf(root, absolute);
        if (relative.empty()) {
            relative = diagnostic.path;
        }
        openAt(relative, diagnostic.line > 0 ? diagnostic.line : 1, diagnostic.column);
    }

    void drawDiagnosticRow(const linear::code::Diagnostic& diagnostic) {
        LinearUI::PushStyleColor(LinearUICol_Text, diagnostic.severity == "error"
                                                   ? linear::ui::theme::error()
                                                   : linear::ui::theme::warning());
        if (LinearUI::Selectable(diagnostic.text.c_str())) {
            openDiagnostic(diagnostic);
        }
        LinearUI::PopStyleColor();
    }

    void drawBuildPanel() {
        if (LinearUI::SmallButton(build.running() ? "Release (running)" : "Build release")) {
            startBuild();
        }
        if (build.running()) {
            LinearUI::SameLine();
            if (LinearUI::SmallButton("Cancel")) {
                stopBuild();
            }
        }
        LinearUI::SameLine();
        if (LinearUI::SmallButton("Close")) {
            showBuild = false;
        }
        LinearUI::SameLine();
        if (buildFailed) {
            LinearUI::PushStyleColor(LinearUICol_Text, linear::ui::theme::error());
            LinearUI::Text("%s", buildState.c_str());
            LinearUI::PopStyleColor();
        } else {
            LinearUI::TextDisabled("%s", buildState.c_str());
        }
        if (build.running() && !buildStageLabel.empty()) {
            LinearUI::SameLine();
            LinearUI::TextDisabled("(%s)", buildStageLabel.c_str());
        }
        // The exact command and the folder it runs in, so a stage that does not
        // work can be repeated in a terminal without guessing either.
        if (!buildCommand.empty()) {
            LinearUI::TextDisabled("%s", buildCommand.c_str());
            if (buildRoot != root) {
                LinearUI::TextDisabled("in %s", buildRoot.c_str());
            }
        }
        // A bar for the whole release, not just the step: the build is the long part
        // and seeing "step 2 of 5" move is what says it is working.
        if (buildStageTotal > 0) {
            const float progress = static_cast<float>(buildStage) /
                                   static_cast<float>(buildStageTotal);
            const char* overlay = build.running()
                                      ? buildStageLabel.c_str()
                                      : (buildComplete ? "release packaged" : "");
            LinearUI::ProgressBar(progress, LinearVec2(-1.0f, LinearUI::GetFrameHeight()), overlay);
        }
        if (buildNeedsBootstrap) {
            const std::filesystem::path tool = std::filesystem::path(buildRoot) / "build" /
                                               "linearbuild-tool" / "bin" / "linearbuild.exe";
            LinearUI::TextDisabled("Looked for %s", tool.string().c_str());
            if (!std::filesystem::exists(std::filesystem::path(buildRoot) / "build")) {
                LinearUI::TextWrapped("This folder has never been built here, which is what a ZIP "
                                    "of the repository looks like: the sources are there, the "
                                    "build tool is not. Bootstrap it once, then Build release.");
            } else {
                LinearUI::TextWrapped("The build tool the bootstrap produces is missing:");
            }
            LinearUI::TextDisabled("%s", buildBootstrapCommands.c_str());
            if (LinearUI::SmallButton("Copy the commands")) {
                LinearUI::SetClipboardText(buildBootstrapCommands.c_str());
                status = "The bootstrap commands are on the clipboard";
            }
        }
        LinearUI::Separator();

        LinearUI::BeginChild("##buildlog", LinearVec2(0.0f, 0.0f), false,
                           LinearUIWindowFlags_HorizontalScrollbar);
        if (buildLog.empty() && buildLive.empty() && !buildNeedsBootstrap) {
            LinearUI::TextDisabled("Ctrl+B makes a release the way the Windows workflow does: "
                                 "configure, build, run the tests, then package a new version "
                                 "into dist/linear-windows-release (preset %s)",
                                 buildPreset.c_str());
        }
        for (const std::string& line : buildLog) {
            const auto found = buildDiagnosticByText.find(line);
            if (found == buildDiagnosticByText.end()) {
                LinearUI::TextUnformatted(line.c_str());
                continue;
            }
            drawDiagnosticRow(buildDiagnostics[found->second]);
        }
        // The running stage's lines are not in the log yet: they are shown as they
        // arrive, and become rows that can be clicked when the stage finishes.
        for (const std::string& line : buildLive) {
            LinearUI::TextUnformatted(line.c_str());
        }
        if (build.running()) {
            LinearUI::SetScrollHereY(1.0f);
        }
        LinearUI::EndChild();
    }

    void drawSidebar() {
        if (LinearUI::SmallButton(showChanges ? "Files" : "[Files]")) {
            showChanges = false;
        }
        LinearUI::SameLine();
        if (LinearUI::SmallButton(showChanges ? "[Source control]" : "Source control")) {
            showChanges = true;
            refreshGit();
        }
        LinearUI::Separator();
        if (showChanges) {
            drawSourceControl();
            return;
        }
        LinearUI::TextDisabled("%s", root.empty() ? "No folder open" : root.c_str());
        LinearUI::Separator();

        if (focusQuery) {
            LinearUI::SetKeyboardFocusHere();
            focusQuery = false;
        }
        LinearUI::SetNextItemWidth(-1.0f);
        LinearUI::InputTextWithHint("##query", "Search in project (Ctrl+Shift+F)", query, sizeof(query));

        const std::string wanted = query;
        if (wanted.empty()) {
            hits.clear();
            lastQuery.clear();
        } else if (wanted != lastQuery) {
            // Only when the text changes: scanning every file each frame would
            // stutter while somebody types in a large repository.
            hits = linear::code::searchProject(root, entries, wanted);
            lastQuery = wanted;
        }

        if (!wanted.empty()) {
            LinearUI::TextDisabled("%zu result%s", hits.size(), hits.size() == 1 ? "" : "s");
            const float resultsHeight = std::min(220.0f, LinearUI::GetTextLineHeightWithSpacing() * 9.0f);
            LinearUI::BeginChild("##results", LinearVec2(0.0f, resultsHeight), true);
            for (const SearchHit& hit : hits) {
                LinearUI::PushID(hit.relativePath.c_str());
                LinearUI::PushID(hit.line);
                const std::string label = hit.relativePath + ":" + std::to_string(hit.line) + "  " +
                                          hit.text;
                if (LinearUI::Selectable(label.c_str())) {
                    openAt(hit.relativePath, hit.line);
                }
                LinearUI::PopID();
                LinearUI::PopID();
            }
            if (hits.empty()) {
                LinearUI::TextDisabled("No match");
            }
            LinearUI::EndChild();
            LinearUI::Separator();
        }

        LinearUI::BeginChild("##tree");
        if (!root.empty()) {
            drawTree(tree, true);
        }
        LinearUI::EndChild();
    }

    void drawTree(const TreeNode& node, bool firstLevel) {
        for (const TreeNode& child : node.children) {
            if (child.entry.directory) {
                LinearUITreeNodeFlags flags =
                    LinearUITreeNodeFlags_OpenOnArrow | LinearUITreeNodeFlags_SpanAvailWidth;
                if (firstLevel) {
                    flags |= LinearUITreeNodeFlags_DefaultOpen;
                }
                const bool open = LinearUI::TreeNodeEx(child.entry.relativePath.c_str(), flags,
                                                    "%s", child.entry.name.c_str());
                if (open) {
                    drawTree(child, false);
                    LinearUI::TreePop();
                }
                continue;
            }
            LinearUI::PushID(child.entry.relativePath.c_str());
            const bool selected =
                active >= 0 && files[static_cast<std::size_t>(active)].relative ==
                                    child.entry.relativePath;
            if (LinearUI::Selectable(child.entry.name.c_str(), selected)) {
                openRelative(child.entry.relativePath);
            }
            if (LinearUI::IsItemHovered(LinearUIHoveredFlags_DelayNormal)) {
                drawFileTooltip(child.entry.relativePath);
            }
            LinearUI::PopID();
        }
    }

    void drawTabs() {
        std::vector<std::size_t> closing;
        if (LinearUI::BeginTabBar("##tabs", LinearUITabBarFlags_AutoSelectNewTabs |
                                             LinearUITabBarFlags_Reorderable |
                                             LinearUITabBarFlags_FittingPolicyScroll)) {
            for (std::size_t i = 0; i < files.size(); ++i) {
                OpenFile& file = files[i];
                const bool dirty = isDirty(file);
                const std::string label =
                    (dirty ? "* " : "") + file.name + "###" + file.relative;
                bool open = true;
                LinearUITabItemFlags flags = 0;
                if (selectOnOpen && static_cast<int>(i) == active) {
                    flags |= LinearUITabItemFlags_SetSelected;
                }
                if (LinearUI::BeginTabItem(label.c_str(), &open, flags)) {
                    if (active != static_cast<int>(i)) {
                        active = static_cast<int>(i);
                        findDirty = true;
                        outlineDirty = true;
                    }
                    LinearUI::EndTabItem();
                }
                if (LinearUI::IsItemHovered(LinearUIHoveredFlags_DelayNormal)) {
                    drawFileTooltip(file.relative);
                }
                if (!open) {
                    closing.push_back(i);
                }
            }
            LinearUI::EndTabBar();
        }
        selectOnOpen = false;
        // Closed from the end so the indices of the tabs still to close stay valid.
        for (auto it = closing.rbegin(); it != closing.rend(); ++it) {
            requestClose(*it);
        }
    }

    float findBarHeight() const {
        float height = LinearUI::GetFrameHeightWithSpacing();
        if (showReplaceField) {
            height += LinearUI::GetFrameHeightWithSpacing();
        }
        return height + LinearUI::GetStyle().ItemSpacing.y;
    }

    void drawFindBar() {
        LinearUI::BeginChild("##findbar", LinearVec2(0.0f, findBarHeight()), true);

        if (focusFind) {
            LinearUI::SetKeyboardFocusHere();
            focusFind = false;
        }
        LinearUI::SetNextItemWidth(260.0f);
        const bool submitted = LinearUI::InputTextWithHint(
            "##find", "Find in file (Ctrl+F)", findText, sizeof(findText),
            LinearUIInputTextFlags_EnterReturnsTrue);
        if (submitted) {
            findStep(!LinearUI::GetIO().KeyShift);
        }
        if (LinearUI::IsItemEdited()) {
            findDirty = true;
            findNote.clear();
        }

        LinearUI::SameLine();
        if (LinearUI::SmallButton("Prev")) {
            findStep(false);
        }
        LinearUI::SameLine();
        if (LinearUI::SmallButton("Next")) {
            findStep(true);
        }
        LinearUI::SameLine();
        if (LinearUI::Checkbox("Match case", &findOptions.caseSensitive)) {
            findDirty = true;
        }
        LinearUI::SameLine();
        if (LinearUI::Checkbox("Whole word", &findOptions.wholeWord)) {
            findDirty = true;
        }
        ensureFindMatches();
        LinearUI::SameLine();
        if (!findMatches.empty()) {
            LinearUI::TextDisabled("%d of %zu", findIndex + 1, findMatches.size());
        } else if (findText[0] != '\0') {
            LinearUI::TextDisabled("No match");
        }

        LinearUI::SameLine();
        if (LinearUI::SmallButton(showReplaceField ? "Hide replace" : "Replace")) {
            showReplaceField = !showReplaceField;
            if (showReplaceField) {
                focusReplace = true;
            }
        }
        LinearUI::SameLine();
        if (LinearUI::SmallButton("Close")) {
            showFind = false;
            showReplaceField = false;
        }

        if (showReplaceField) {
            if (focusReplace) {
                LinearUI::SetKeyboardFocusHere();
                focusReplace = false;
            }
            LinearUI::SetNextItemWidth(260.0f);
            LinearUI::InputTextWithHint("##replace", "Replace with", replaceText,
                                     sizeof(replaceText));
            LinearUI::SameLine();
            if (LinearUI::SmallButton("Replace")) {
                replaceCurrent();
            }
            LinearUI::SameLine();
            if (LinearUI::SmallButton("Replace all")) {
                replaceAllMatches();
            }
        }
        if (!findNote.empty()) {
            LinearUI::SameLine();
            LinearUI::TextDisabled("%s", findNote.c_str());
        }
        LinearUI::EndChild();
    }

    void drawOutlinePanel() {
        LinearUI::TextDisabled("Outline");
        LinearUI::SameLine();
        if (LinearUI::SmallButton("Symbols...")) {
            symbolPaletteRequested = true;
            focusSymbolFilter = true;
        }
        LinearUI::Separator();
        if (active < 0) {
            LinearUI::TextDisabled("No file open");
            return;
        }

        const std::vector<OutlineSymbol>& symbols = activeOutline();
        LinearUI::TextDisabled("%zu symbol%s", symbols.size(), symbols.size() == 1 ? "" : "s");
        LinearUI::BeginChild("##outlinelist");
        for (std::size_t i = 0; i < symbols.size(); ++i) {
            const OutlineSymbol& symbol = symbols[i];
            LinearUI::PushID(static_cast<int>(i));
            std::string label;
            for (int depth = 0; depth < symbol.depth && depth < 6; ++depth) {
                label += "    ";
            }
            label += symbol.name;
            if (LinearUI::Selectable(label.c_str())) {
                jumpToLine(symbol.line);
            }
            if (LinearUI::IsItemHovered()) {
                LinearUI::SetTooltip("%s  (line %d)", symbol.detail.c_str(), symbol.line);
            }
            LinearUI::PopID();
        }
        if (symbols.empty()) {
            LinearUI::TextDisabled("No symbols in this file");
        }
        LinearUI::EndChild();
    }

    void drawEditorArea() {
        drawTabs();
        if (active < 0 || active >= static_cast<int>(files.size())) {
            LinearUI::TextDisabled("Choose a file in the tree, or press Ctrl+P to open one by name.");
            return;
        }

        OpenFile& file = files[static_cast<std::size_t>(active)];
        float footer = LinearUI::GetTextLineHeightWithSpacing() + LinearUI::GetStyle().ItemSpacing.y * 2.0f;
        if (showFind) {
            footer += findBarHeight();
        }
        if (file.stale) {
            footer += LinearUI::GetFrameHeightWithSpacing();
        }
        // A key the completion list has just used belongs to the list, not to the
        // text: the widget is told not to look at the keyboard for this frame.
        if (swallowKeysThisFrame) {
            file.editor->SetHandleKeyboardInputs(false);
        }
        file.editor->Render("##code", LinearVec2(0.0f, -footer), true);
        if (swallowKeysThisFrame) {
            file.editor->SetHandleKeyboardInputs(true);
            swallowKeysThisFrame = false;
        }
        const LinearVec2 editorOrigin = LinearUI::GetItemRectMin();
        const bool editorHovered = LinearUI::IsItemHovered();
        const bool textChanged = file.editor->IsTextChanged();
        const bool caretMoved = file.editor->IsCursorPositionChanged();
        if (textChanged) {
            findDirty = true;
            outlineDirty = true;
            docIndexDirty = true;
            filePagePath.clear();
        }

        // What the mouse is over, documented. The widget answers where a screen
        // position falls in the text, so a word is read the same way a click
        // would put the caret in it.
        hoverPage = linear::code::DocEntry();
        const Language languageUnderMouse = linear::code::languageForPath(file.relative);
        if (editorHovered && !completionOpen) {
            const LinearTextEditor::Coordinates under =
                file.editor->ScreenPosToCoordinates(LinearUI::GetMousePos(), editorOrigin);
            const std::string hoverText = file.editor->GetText();
            if (under.mLine >= 0 && under.mLine < file.editor->GetTotalLines()) {
                const std::size_t lineStart = linear::code::offsetOfLine(hoverText, under.mLine + 1);
                std::size_t lineEnd = hoverText.find('\n', lineStart);
                if (lineEnd == std::string::npos) {
                    lineEnd = hoverText.size();
                }
                const std::string lineText = hoverText.substr(lineStart, lineEnd - lineStart);
                const std::size_t offset =
                    lineStart + linear::code::byteOffsetOfColumn(lineText, under.mColumn, tabSize);
                hoverPage =
                    linear::code::documentationAt(hoverText, languageUnderMouse, offset);
            }
            if (!hoverPage.name.empty()) {
                drawHoverTooltip();
            }
        }

        if (completionRequested) {
            refreshCompletions(true);
            completionRequested = false;
        } else if (completionJustAccepted) {
            completionJustAccepted = false;
        } else if (textChanged || caretMoved) {
            refreshCompletions(false);
        }

        if (file.stale) {
            LinearUI::PushStyleColor(LinearUICol_Text, linear::ui::theme::warning());
            LinearUI::TextUnformatted("This file changed on disk while you had edits in it.");
            LinearUI::PopStyleColor();
            LinearUI::SameLine();
            if (LinearUI::SmallButton("Reload and lose my edits")) {
                reloadActive();
            }
            LinearUI::SameLine();
            if (LinearUI::SmallButton("Keep editing")) {
                file.stale = false;
                status = "Kept the buffer for " + file.relative;
            }
        }

        if (showFind) {
            drawFindBar();
        }

        const LinearTextEditor::Coordinates cursor = file.editor->GetCursorPosition();
        const Language language = linear::code::languageForPath(file.relative);
        std::string line = file.relative + "  |  Ln " + std::to_string(cursor.mLine + 1) +
                           ", Col " + std::to_string(cursor.mColumn + 1) + "  |  " +
                           linear::code::languageName(language) + "  |  " +
                           std::to_string(file.editor->GetTotalLines()) + " lines  |  " +
                           (isDirty(file) ? "modified" : "saved");
        if (!branch.empty()) {
            line += "  |  " + branch;
        }
        if (file.editor->HasSelection()) {
            line += "  |  " + std::to_string(file.editor->GetSelectedText().size()) + " selected";
        }
        LinearUI::TextDisabled("%s", line.c_str());
        LinearUI::SameLine();
        LinearUI::TextDisabled("%s", status.c_str());
    }

    void handleKeys() {
        LinearUIIO& io = LinearUI::GetIO();
        const bool ctrl = io.KeyCtrl;
        const bool shift = io.KeyShift;

        handleCompletionKeys();
        if (LinearUI::IsKeyPressed(LinearUIKey_F1)) {
            openDocAtCaret();
        }
        if (ctrl && LinearUI::IsKeyPressed(LinearUIKey_Space)) {
            completionRequested = true;
        }

        if (ctrl && LinearUI::IsKeyPressed(LinearUIKey_S)) {
            if (shift) {
                saveAll();
            } else {
                saveFile(active);
            }
        }
        if (ctrl && LinearUI::IsKeyPressed(LinearUIKey_R)) {
            reloadActive();
        }
        if (ctrl && LinearUI::IsKeyPressed(LinearUIKey_W) && active >= 0) {
            requestClose(static_cast<std::size_t>(active));
        }
        if (ctrl && LinearUI::IsKeyPressed(LinearUIKey_P)) {
            quickOpenRequested = true;
            focusQuickFilter = true;
        }
        if (ctrl && LinearUI::IsKeyPressed(LinearUIKey_G)) {
            goToLineRequested = true;
            focusGoToLine = true;
        }
        if (ctrl && shift && LinearUI::IsKeyPressed(LinearUIKey_O)) {
            symbolPaletteRequested = true;
            focusSymbolFilter = true;
        }
        if (ctrl && LinearUI::IsKeyPressed(LinearUIKey_F)) {
            if (shift) {
                focusQuery = true;
            } else {
                openFindBar(false);
            }
        }
        if (ctrl && LinearUI::IsKeyPressed(LinearUIKey_H)) {
            openFindBar(true);
        }
        if (LinearUI::IsKeyPressed(LinearUIKey_F3)) {
            findStep(!shift);
        }
        if (showFind && LinearUI::IsKeyPressed(LinearUIKey_Escape) &&
            !LinearUI::IsPopupOpen("", LinearUIPopupFlags_AnyPopup)) {
            showFind = false;
            showReplaceField = false;
        }
        if (ctrl && LinearUI::IsKeyPressed(LinearUIKey_Slash)) {
            toggleComment();
        }
        if (ctrl && LinearUI::IsKeyPressed(LinearUIKey_RightBracket)) {
            jumpToMatchingBracket();
        }
        if (io.KeyAlt && LinearUI::IsKeyPressed(LinearUIKey_LeftArrow)) {
            navigate(-1);
        }
        if (io.KeyAlt && LinearUI::IsKeyPressed(LinearUIKey_RightArrow)) {
            navigate(1);
        }
        if (ctrl && LinearUI::IsKeyPressed(LinearUIKey_Tab)) {
            cycleTab(shift ? -1 : 1);
        }
        if (ctrl && LinearUI::IsKeyPressed(LinearUIKey_B)) {
            startBuild();
        }
        if (LinearUI::IsKeyPressed(LinearUIKey_F5)) {
            refresh();
        }
    }

    // A ranked list over the scanned files: the query only has to be an
    // in-order abbreviation of the path, so "cmain" finds src/CodeMain.cpp.
    void drawQuickOpen() {
        if (!LinearUI::BeginPopupModal("Quick open", nullptr, LinearUIWindowFlags_NoSavedSettings)) {
            return;
        }
        LinearUI::SetNextWindowSize(LinearVec2(620.0f, 440.0f), LinearUICond_Appearing);
        if (focusQuickFilter) {
            LinearUI::SetKeyboardFocusHere();
            focusQuickFilter = false;
        }
        LinearUI::SetNextItemWidth(-1.0f);
        const bool submitted = LinearUI::InputText("##quick", quickFilter, sizeof(quickFilter),
                                                LinearUIInputTextFlags_EnterReturnsTrue);

        std::vector<std::pair<int, const FileEntry*>> ranked;
        for (const FileEntry& entry : entries) {
            if (entry.directory) {
                continue;
            }
            const int score = linear::code::fuzzyScore(entry.relativePath, quickFilter);
            if (score < 0) {
                continue;
            }
            ranked.emplace_back(score, &entry);
        }
        std::stable_sort(ranked.begin(), ranked.end(),
                         [](const std::pair<int, const FileEntry*>& left,
                            const std::pair<int, const FileEntry*>& right) {
                             if (left.first != right.first) {
                                 return left.first > right.first;
                             }
                             return left.second->relativePath < right.second->relativePath;
                         });
        if (ranked.size() > kQuickOpenLimit) {
            ranked.resize(kQuickOpenLimit);
        }

        LinearUI::BeginChild("##matches", LinearVec2(0.0f, 330.0f), true);
        for (const std::pair<int, const FileEntry*>& match : ranked) {
            if (LinearUI::Selectable(match.second->relativePath.c_str())) {
                openRelative(match.second->relativePath);
                LinearUI::CloseCurrentPopup();
            }
        }
        if (ranked.empty()) {
            LinearUI::TextDisabled("No file matches");
        }
        LinearUI::EndChild();
        if (submitted && !ranked.empty()) {
            openRelative(ranked.front().second->relativePath);
            LinearUI::CloseCurrentPopup();
        }
        if (LinearUI::Button("Cancel")) {
            LinearUI::CloseCurrentPopup();
        }
        LinearUI::EndPopup();
    }

    void drawSymbolPalette() {
        if (!LinearUI::BeginPopupModal("Go to symbol", nullptr, LinearUIWindowFlags_NoSavedSettings)) {
            return;
        }
        LinearUI::SetNextWindowSize(LinearVec2(560.0f, 420.0f), LinearUICond_Appearing);
        if (focusSymbolFilter) {
            LinearUI::SetKeyboardFocusHere();
            focusSymbolFilter = false;
        }
        LinearUI::SetNextItemWidth(-1.0f);
        const bool submitted = LinearUI::InputTextWithHint("##symbol", "Symbol name",
                                                        symbolFilter, sizeof(symbolFilter),
                                                        LinearUIInputTextFlags_EnterReturnsTrue);

        const std::vector<OutlineSymbol> symbols = activeOutline();
        std::vector<const OutlineSymbol*> shown;
        for (const OutlineSymbol& symbol : symbols) {
            if (linear::code::fuzzyScore(symbol.name, symbolFilter) < 0) {
                continue;
            }
            shown.push_back(&symbol);
            if (shown.size() >= 200) {
                break;
            }
        }

        LinearUI::BeginChild("##symbols", LinearVec2(0.0f, 310.0f), true);
        for (const OutlineSymbol* symbol : shown) {
            const std::string label = symbol->kind + "  " + symbol->name + "    (line " +
                                      std::to_string(symbol->line) + ")";
            if (LinearUI::Selectable(label.c_str())) {
                jumpToLine(symbol->line);
                LinearUI::CloseCurrentPopup();
            }
        }
        if (shown.empty()) {
            LinearUI::TextDisabled("No symbol matches");
        }
        LinearUI::EndChild();
        if (submitted && !shown.empty()) {
            jumpToLine(shown.front()->line);
            LinearUI::CloseCurrentPopup();
        }
        if (LinearUI::Button("Cancel")) {
            LinearUI::CloseCurrentPopup();
        }
        LinearUI::EndPopup();
    }

    void drawGoToLine() {
        if (!LinearUI::BeginPopupModal("Go to line", nullptr, LinearUIWindowFlags_AlwaysAutoResize)) {
            return;
        }
        if (focusGoToLine) {
            LinearUI::SetKeyboardFocusHere();
            focusGoToLine = false;
        }
        LinearUI::SetNextItemWidth(140.0f);
        const bool submitted = LinearUI::InputText("Line", goToLine, sizeof(goToLine),
                                                LinearUIInputTextFlags_EnterReturnsTrue);
        if (submitted && active >= 0) {
            const int line = std::atoi(goToLine);
            if (line > 0) {
                openAt(files[static_cast<std::size_t>(active)].relative, line);
            }
            LinearUI::CloseCurrentPopup();
        }
        if (LinearUI::Button("Cancel")) {
            LinearUI::CloseCurrentPopup();
        }
        LinearUI::EndPopup();
    }

    void drawOpenFolder() {
        if (!LinearUI::BeginPopupModal("Open folder", nullptr, LinearUIWindowFlags_AlwaysAutoResize)) {
            return;
        }
        if (focusFolder) {
            LinearUI::SetKeyboardFocusHere();
            focusFolder = false;
        }
        LinearUI::SetNextItemWidth(520.0f);
        const bool submitted = LinearUI::InputText("Folder", folderPath, sizeof(folderPath),
                                                LinearUIInputTextFlags_EnterReturnsTrue);
        // The box is still there for a path that is already known, and this is for
        // every other time: Explorer's own folder picker fills it in.
        if (LinearUI::Button("Browse...")) {
            const std::string chosen = chooseFolder(linearwinGetWin32Window(owner), folderPath);
            if (!chosen.empty()) {
                std::snprintf(folderPath, sizeof(folderPath), "%s", chosen.c_str());
            }
        }
        LinearUI::SameLine();
        if (submitted || LinearUI::Button("Open")) {
            std::error_code error;
            if (std::filesystem::is_directory(folderPath, error)) {
                const std::string opened = folderPath;
                openFolderRequested = false;
                setRoot(opened);
                status = "Opened " + root;
            } else {
                status = std::string("No folder at ") + folderPath;
            }
            LinearUI::CloseCurrentPopup();
        }
        LinearUI::SameLine();
        if (LinearUI::Button("Cancel")) {
            LinearUI::CloseCurrentPopup();
        }
        LinearUI::EndPopup();
    }

    void drawUnsavedPopup() {
        if (!LinearUI::BeginPopupModal("Unsaved changes", nullptr, LinearUIWindowFlags_AlwaysAutoResize)) {
            return;
        }
        if (pendingClose >= files.size()) {
            LinearUI::CloseCurrentPopup();
            LinearUI::EndPopup();
            return;
        }
        OpenFile& file = files[pendingClose];
        const bool changedOnDisk = !file.stamp.empty() &&
                                   linear::code::fileStamp(file.absolute) != file.stamp;
        LinearUI::Text("Save %s before closing it?", file.name.c_str());
        if (changedOnDisk) {
            LinearUI::TextDisabled("It also changed on disk since it was opened.");
        }
        if (LinearUI::Button("Save and close")) {
            saveFile(static_cast<int>(pendingClose));
            const std::size_t index = pendingClose;
            pendingClose = kNoFile;
            closeTab(index);
            LinearUI::CloseCurrentPopup();
        }
        LinearUI::SameLine();
        if (LinearUI::Button("Close without saving")) {
            const std::size_t index = pendingClose;
            pendingClose = kNoFile;
            closeTab(index);
            LinearUI::CloseCurrentPopup();
        }
        LinearUI::SameLine();
        if (LinearUI::Button("Cancel")) {
            pendingClose = kNoFile;
            LinearUI::CloseCurrentPopup();
        }
        LinearUI::EndPopup();
    }

    void drawShortcuts() {
        if (!showShortcuts) {
            return;
        }
        LinearUI::SetNextWindowSize(LinearVec2(560.0f, 430.0f), LinearUICond_FirstUseEver);
        if (LinearUI::Begin("Keyboard shortcuts", &showShortcuts)) {
            LinearUI::TextUnformatted("Ctrl+Space   suggest what fits here");
            LinearUI::TextUnformatted("F1   documentation for the name under the caret");
            LinearUI::TextUnformatted("Ctrl+P   open a file by name");
            LinearUI::TextUnformatted("Ctrl+Shift+O   go to a symbol in this file");
            LinearUI::TextUnformatted("Ctrl+G   go to a line");
            LinearUI::TextUnformatted("Ctrl+Shift+F   search the whole project");
            LinearUI::Separator();
            LinearUI::TextUnformatted("Ctrl+F   find in this file");
            LinearUI::TextUnformatted("Ctrl+H   find and replace in this file");
            LinearUI::TextUnformatted("F3 / Shift+F3   next / previous match");
            LinearUI::TextUnformatted("Ctrl+/   comment or uncomment the line");
            LinearUI::Separator();
            LinearUI::TextUnformatted("Ctrl+S   save the file in front");
            LinearUI::TextUnformatted("Ctrl+Shift+S   save every open file");
            LinearUI::TextUnformatted("Ctrl+R   reload the file from disk");
            LinearUI::TextUnformatted("Ctrl+W   close the file in front");
            LinearUI::TextUnformatted("Ctrl+Tab   next tab, Ctrl+Shift+Tab   previous tab");
            LinearUI::Separator();
            LinearUI::TextUnformatted(
            "Ctrl+B   build a release: configure, build, tests, package");
            LinearUI::TextUnformatted(
                "Source control (left panel)   the changed files, stage everything, commit");
            LinearUI::Separator();
            LinearUI::TextUnformatted("Alt+Left / Alt+Right   back and forward between jumps");
            LinearUI::TextUnformatted("Ctrl+]   jump to the matching bracket");
            LinearUI::Separator();
            LinearUI::TextWrapped(
                "The text widget keeps the editing keys: Ctrl+Z and Ctrl+Y undo and redo, Ctrl+C, "
                "Ctrl+X and Ctrl+V copy, cut and paste, and Ctrl+A selects everything.");
        }
        LinearUI::End();
    }

    void drawPopups() {
        if (quickOpenRequested) {
            LinearUI::OpenPopup("Quick open");
            quickOpenRequested = false;
        }
        if (goToLineRequested) {
            LinearUI::OpenPopup("Go to line");
            goToLineRequested = false;
        }
        if (symbolPaletteRequested) {
            LinearUI::OpenPopup("Go to symbol");
            symbolPaletteRequested = false;
        }
        if (openFolderRequested) {
            LinearUI::OpenPopup("Open folder");
            openFolderRequested = false;
        }
        if (pendingClose != kNoFile && pendingClose < files.size() &&
            isDirty(files[pendingClose])) {
            LinearUI::OpenPopup("Unsaved changes");
        } else if (pendingClose != kNoFile) {
            const std::size_t index = pendingClose;
            pendingClose = kNoFile;
            closeTab(index);
        }

        drawQuickOpen();
        drawGoToLine();
        drawSymbolPalette();
        drawOpenFolder();
        drawUnsavedPopup();
        drawShortcuts();
    }

    void closeTab(std::size_t index) {
        if (index >= files.size()) {
            return;
        }
        files.erase(files.begin() + static_cast<std::ptrdiff_t>(index));
        outlineFile = -1;
        findDirty = true;
        outlineDirty = true;
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

    void draw(Window& window) {
        owner = window.getNativeWindow();
        checkDiskChanges();
        pollBuild();
        handleKeys();
        drawMenuBar(window);

        const LinearUIViewport* viewport = LinearUI::GetMainViewport();
        LinearUI::SetNextWindowPos(viewport->WorkPos);
        LinearUI::SetNextWindowSize(viewport->WorkSize);
        const LinearUIWindowFlags workspaceFlags =
            LinearUIWindowFlags_NoTitleBar | LinearUIWindowFlags_NoResize | LinearUIWindowFlags_NoMove |
            LinearUIWindowFlags_NoCollapse | LinearUIWindowFlags_NoBringToFrontOnFocus |
            LinearUIWindowFlags_NoScrollbar | LinearUIWindowFlags_NoScrollWithMouse |
            LinearUIWindowFlags_NoSavedSettings;
        LinearUI::Begin("##workspace", nullptr, workspaceFlags);

        LinearUI::BeginChild("##sidebar", LinearVec2(kSidebarWidth, 0.0f), true);
        drawSidebar();
        LinearUI::EndChild();

        LinearUI::SameLine();
        if (showOutline) {
            LinearUI::BeginChild("##outline", LinearVec2(kOutlineWidth, 0.0f), true);
            drawOutlinePanel();
            LinearUI::EndChild();
            LinearUI::SameLine();
        }

        LinearUI::BeginChild("##editor", LinearVec2(0.0f, showBuild ? -190.0f : 0.0f), false,
                          LinearUIWindowFlags_NoScrollbar | LinearUIWindowFlags_NoScrollWithMouse);
        drawEditorArea();
        LinearUI::EndChild();

        if (showBuild) {
            LinearUI::BeginChild("##build", LinearVec2(0.0f, 178.0f), true);
            drawBuildPanel();
            LinearUI::EndChild();
        }

        drawCompletionPopup();
        drawDocumentationWindow();
        // The workspace is the one window this frame opened, so it is closed once:
        // a second End() lands with no window open and linearui asserts on it.
        LinearUI::End();
        drawPopups();
    }
};

void beginLinearUI(Window& window) {
    LINEARUI_CHECKVERSION();
    LinearUI::CreateContext();
    LinearUIIO& io = LinearUI::GetIO();
    io.ConfigFlags |= LinearUIConfigFlags_NavEnableKeyboard;
    // The layout is fixed in code, so no linearui.ini is written beside the
    // executable and every checkout opens the same way.
    io.IniFilename = nullptr;
    // One palette for every window in the project, from UiTheme.h.
    linear::ui::theme::apply();
    LinearUI_ImplLinearWin_InitForOpenGL(window.getNativeWindow(), true);
    LinearUI_ImplOpenGL3_Init("#version 330");
}

}  // namespace

int main(int argc, char** argv) {
    try {
        Window window(kWindowWidth, kWindowHeight, "Linear Code");
        beginLinearUI(window);

        App app;
        std::string start = argc > 1 ? argv[1] : std::filesystem::current_path().string();
        app.setRoot(start);
        app.status = "Editing " + app.root +
                     " - Ctrl+P opens a file, Ctrl+F finds in it, F1 documents what is under "
                     "the caret";

        while (!window.shouldClose()) {
            window.pollEvents();

            LinearUI_ImplOpenGL3_NewFrame();
            LinearUI_ImplLinearWin_NewFrame();
            LinearUI::NewFrame();

            app.draw(window);

            LinearUI::Render();
            int displayWidth = 0;
            int displayHeight = 0;
            linearwinGetFramebufferSize(window.getNativeWindow(), &displayWidth, &displayHeight);
            glViewport(0, 0, displayWidth, displayHeight);
            glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            LinearUI_ImplOpenGL3_RenderDrawData(LinearUI::GetDrawData());
            window.swapBuffers();
        }

        LinearUI_ImplOpenGL3_Shutdown();
        LinearUI_ImplLinearWin_Shutdown();
        LinearUI::DestroyContext();
    } catch (const std::exception& error) {
        std::fprintf(stderr, "LinearCode could not start: %s\n", error.what());
        return 1;
    }
    return 0;
}
