#define RBLXMATH_ENABLE_EXPERIMENTAL

#include "CodeBuild.h"
#include "CodeBuildRun.h"
#include "CodeGit.h"
#include "CodeProject.h"
#include "GitRepo.h"
#include "RblxTextEditor.h"
#include "Window.h"

#include <rblxgl/rblxgl.h>
#include <RBLXWIN/rblxwin.h>
#include "rblxui.h"
#include "rblxui_impl_rblxwin.h"
#include "rblxui_impl_opengl3.h"
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

// RBLXCode is the source editor for this repository: the window, the project
// tree, the tabs and the text widget are here, and the parts that can be tested
// without a display live in CodeProject. It opens the folder it is launched from
// (or the one named on the command line) and edits the files in it in place.
namespace {

using rblx::code::FileEntry;
using rblx::code::Language;
using rblx::code::OutlineSymbol;
using rblx::code::SearchHit;
using rblx::code::SourceDocument;
using rblx::code::TextMatch;
using rblx::code::CompletionItem;
using rblx::code::TreeNode;
using rblx::code::TreeNode;

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
const RblxTextEditor::LanguageDefinition& definitionFor(Language language) {
    static const RblxTextEditor::LanguageDefinition plain;
    switch (language) {
        case Language::Cpp: return RblxTextEditor::LanguageDefinition::CPlusPlus();
        case Language::Glsl: return RblxTextEditor::LanguageDefinition::GLSL();
        case Language::Lua: return RblxTextEditor::LanguageDefinition::Lua();
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
    std::unique_ptr<RblxTextEditor> editor;
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
    // checkout - the vendored build tool and the repository code RBLXGit drives -
    // so the editor can run them itself instead of sending somebody to a terminal.
    rblx::code::BuildRun build;
    std::vector<std::string> buildLines;
    std::size_t buildLineCount = 0;
    std::vector<rblx::code::Diagnostic> buildDiagnostics;
    std::map<std::string, std::size_t> buildDiagnosticByText;
    std::string buildProgram;
    // The checkout the build belongs to, which is not always the open folder: the
    // editor is normally started from build\windows-release\Release.
    std::string buildRoot;
    std::string buildCommand;
    std::string buildPreset = rblx::code::kDefaultPreset;
    std::string buildState = "not built yet";
    bool buildFailed = false;
    bool showBuild = true;
    bool buildPending = false;

    rblx::git::Repo repo;
    bool repoOpen = false;
    std::vector<rblx::code::ChangeRow> changes;
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
    rblx::code::FindOptions findOptions;
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
    rblx::code::CompletionList completions;
    int completionIndex = 0;
    bool completionOpen = false;
    bool completionRequested = false;
    bool completionJustAccepted = false;
    bool swallowKeysThisFrame = false;
    rblx::code::DocEntry hoverPage;
    rblx::code::DocEntry docPage;
    bool showDoc = false;
    char docFilter[96] = {};
    std::vector<rblx::code::DocEntry> docIndex;
    int docIndexFile = -1;
    bool docIndexDirty = true;
    // A file page for a tooltip is read once per hovered path: a tooltip is drawn
    // every frame it is up, and re-reading the file each time would be wasteful.
    rblx::code::DocEntry filePageCache;
    std::string filePagePath;

    // Navigation history and the view settings every editor shares.
    std::vector<NavPoint> history;
    std::size_t historyIndex = 0;
    bool navigating = false;
    int tabSize = 4;
    bool showWhitespace = false;
    // The editor's own palette starts out matching the window's, which comes
    // from the shared settings file; the View menu can still differ from it.
    bool lightTheme = rblx::ui::theme::isLight();
    double nextDiskCheck = 0.0;

    void setRoot(const std::string& path) {
        root = path;
        std::snprintf(folderPath, sizeof(folderPath), "%s", root.c_str());
        // Another folder is another repository, and possibly none at all.
        repo = rblx::git::Repo();
        repoOpen = false;
        refresh();
    }

    void refresh() {
        entries = rblx::code::scanProject(root);
        tree = rblx::code::buildTree(entries);
        hits.clear();
        lastQuery.clear();
        branch = rblx::code::gitBranchOf(root);
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
            file.editor->SetPalette(lightTheme ? RblxTextEditor::GetLightPalette()
                                               : RblxTextEditor::GetDarkPalette());
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
        file.stamp = rblx::code::fileStamp(absolute);

        file.editor = std::make_unique<RblxTextEditor>();
        file.editor->SetTabSize(tabSize);
        file.editor->SetShowWhitespaces(showWhitespace);
        file.editor->SetPalette(lightTheme ? RblxTextEditor::GetLightPalette()
                                           : RblxTextEditor::GetDarkPalette());
        file.editor->SetLanguageDefinition(definitionFor(rblx::code::languageForPath(relative)));
        file.editor->SetText(file.document.text);
        file.editor->SetCursorPosition(RblxTextEditor::Coordinates(0, 0));

        files.push_back(std::move(file));
        active = static_cast<int>(files.size()) - 1;
        selectOnOpen = true;
        findDirty = true;
        outlineDirty = true;
        status = "Opened " + relative;
    }

    // The widget counts a tab as the columns it draws, so a byte offset in the
    // text has to be turned into a place the caret can be put.
    RblxTextEditor::Coordinates coordinatesFor(const std::string& text, std::size_t offset) const {
        int line = 1;
        int column = 1;
        rblx::code::lineAndColumn(text, offset, &line, &column);
        const std::size_t start = rblx::code::offsetOfLine(text, line);
        std::size_t end = text.find('\n', start);
        if (end == std::string::npos) {
            end = text.size();
        }
        const std::string lineText = text.substr(start, end - start);
        return RblxTextEditor::Coordinates(
            line - 1, rblx::code::displayColumn(lineText, static_cast<std::size_t>(column - 1), tabSize));
    }

    void openAt(const std::string& relative, int line, int column = 0) {
        rememberPosition();
        openRelative(relative);
        if (active < 0) {
            return;
        }
        RblxTextEditor& editor = *files[static_cast<std::size_t>(active)].editor;
        const int clamped = std::max(1, std::min(line, editor.GetTotalLines()));
        const std::string text = editor.GetText();
        const std::size_t start = rblx::code::offsetOfLine(text, clamped);
        std::size_t end = text.find('\n', start);
        if (end == std::string::npos) {
            end = text.size();
        }
        const std::string lineText = text.substr(start, end - start);
        editor.SetCursorPosition(RblxTextEditor::Coordinates(
            clamped - 1, rblx::code::byteOffsetOfColumn(lineText, column, tabSize)));
    }

    void jumpToLine(int line) {
        if (active < 0) {
            return;
        }
        rememberPosition();
        RblxTextEditor& editor = *files[static_cast<std::size_t>(active)].editor;
        const int clamped = std::max(1, std::min(line, editor.GetTotalLines()));
        editor.SetCursorPosition(RblxTextEditor::Coordinates(clamped - 1, 0));
    }

    // One entry per place a jump left, so Alt+Left returns to what was being read
    // before the jump rather than to wherever the caret happened to be.
    void rememberPosition() {
        if (navigating || active < 0) {
            return;
        }
        const OpenFile& file = files[static_cast<std::size_t>(active)];
        const RblxTextEditor::Coordinates cursor = file.editor->GetCursorPosition();
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
            RblxTextEditor& editor = *files[static_cast<std::size_t>(active)].editor;
            const int line = std::max(1, std::min(point.line, editor.GetTotalLines()));
            const std::string text = editor.GetText();
            const std::size_t start = rblx::code::offsetOfLine(text, line);
            std::size_t end = text.find('\n', start);
            if (end == std::string::npos) {
                end = text.size();
            }
            const std::string lineText = text.substr(start, end - start);
            editor.SetCursorPosition(RblxTextEditor::Coordinates(
                line - 1, rblx::code::byteOffsetOfColumn(lineText, point.column, tabSize)));
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
        if (!rblx::code::writeSourceFile(file.absolute, rblx::code::encodeSource(document), &error)) {
            status = error;
            return;
        }
        file.savedText = document.text;
        file.stamp = rblx::code::fileStamp(file.absolute);
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
        if (!rblx::code::readSourceFile(file.absolute, &bytes, &error)) {
            status = error;
            return;
        }
        const RblxTextEditor::Coordinates caret = file.editor->GetCursorPosition();
        file.document = rblx::code::decodeSource(bytes);
        file.savedText = file.document.text;
        file.stamp = rblx::code::fileStamp(file.absolute);
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
        const double now = RblxUI::GetTime();
        if (now < nextDiskCheck) {
            return;
        }
        nextDiskCheck = now + kDiskCheckSeconds;
        for (OpenFile& file : files) {
            const std::string stamp = rblx::code::fileStamp(file.absolute);
            if (stamp.empty() || stamp == file.stamp) {
                continue;
            }
            if (isDirty(file)) {
                file.stale = true;
                continue;
            }
            std::string bytes;
            std::string error;
            if (!rblx::code::readSourceFile(file.absolute, &bytes, &error)) {
                file.stale = true;
                status = error;
                continue;
            }
            const RblxTextEditor::Coordinates caret = file.editor->GetCursorPosition();
            file.document = rblx::code::decodeSource(bytes);
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
        findMatches = rblx::code::findInText(file.editor->GetText(), findText, findOptions);
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
        RblxTextEditor& editor = *files[static_cast<std::size_t>(active)].editor;
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
        RblxTextEditor& editor = *file.editor;
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
            rblx::code::replaceAllInText(&text, findText, replaceText, findOptions);
        if (changed == 0) {
            findNote = "No match";
            return;
        }
        file.editor->SetText(text);
        file.editor->SetCursorPosition(RblxTextEditor::Coordinates(0, 0));
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
            outlineCache = rblx::code::outlineSymbols(
                file.editor->GetText(), rblx::code::languageForPath(file.relative));
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
        RblxTextEditor& editor = *file.editor;
        const std::string prefix =
            rblx::code::lineCommentPrefix(rblx::code::languageForPath(file.relative));
        if (prefix.empty()) {
            status = "This file has no line comment to toggle.";
            return;
        }

        const RblxTextEditor::Coordinates cursor = editor.GetCursorPosition();
        const std::string lineText = editor.GetCurrentLineText();
        std::size_t marker = 0;
        if (rblx::code::lineHasComment(lineText, prefix, &marker)) {
            const int begin = rblx::code::displayColumn(lineText, marker, tabSize);
            const int end = rblx::code::displayColumn(lineText, marker + prefix.size(), tabSize);
            editor.SetSelection(RblxTextEditor::Coordinates(cursor.mLine, begin),
                                RblxTextEditor::Coordinates(cursor.mLine, end));
            editor.Delete();
            editor.SetCursorPosition(RblxTextEditor::Coordinates(cursor.mLine, begin));
        } else {
            std::size_t indent = 0;
            while (indent < lineText.size() && (lineText[indent] == ' ' || lineText[indent] == '\t')) {
                ++indent;
            }
            const int column = rblx::code::displayColumn(lineText, indent, tabSize);
            editor.SetCursorPosition(RblxTextEditor::Coordinates(cursor.mLine, column));
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
        RblxTextEditor& editor = *file.editor;
        const std::string text = editor.GetText();
        const RblxTextEditor::Coordinates cursor = editor.GetCursorPosition();
        const std::string lineText = editor.GetCurrentLineText();
        const std::size_t lineStart = rblx::code::offsetOfLine(text, cursor.mLine + 1);
        const std::size_t offset =
            lineStart + rblx::code::byteOffsetOfColumn(lineText, cursor.mColumn, tabSize);
        const std::size_t other = rblx::code::matchingBracket(text, offset);
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
        return rblx::code::languageForPath(files[static_cast<std::size_t>(active)].relative);
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
        const RblxTextEditor::Coordinates cursor = file.editor->GetCursorPosition();
        const std::size_t lineStart =
            rblx::code::offsetOfLine(text, std::max(1, cursor.mLine + 1));
        std::size_t lineEnd = text.find('\n', lineStart);
        if (lineEnd == std::string::npos) {
            lineEnd = text.size();
        }
        const std::string lineText = text.substr(lineStart, lineEnd - lineStart);
        const std::size_t column =
            rblx::code::byteOffsetOfColumn(lineText, cursor.mColumn, tabSize);
        return std::min(lineStart + column, text.size());
    }

    void openDoc(const rblx::code::DocEntry& page) {
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
                openDoc(rblx::code::documentationForFile(relative, file.editor->GetText()));
                return;
            }
        }
        std::string text;
        std::string error;
        if (!rblx::code::readSourceFile(rblx::code::joinProjectPath(root, relative), &text,
                                        &error)) {
            status = error;
            return;
        }
        openDoc(rblx::code::documentationForFile(relative, text));
    }

    // What F1 does: the page for the name under the caret, and the page for the
    // file when the caret is not on a name at all.
    void openDocAtCaret() {
        if (active < 0) {
            status = "Open a file to read its documentation.";
            return;
        }
        const rblx::code::DocEntry page =
            rblx::code::documentationAt(activeText(), activeLanguage(), caretOffset());
        if (!page.name.empty()) {
            openDoc(page);
            return;
        }
        openDocForFile(files[static_cast<std::size_t>(active)].relative);
    }

    void rebuildDocIndex() {
        docIndex.clear();
        for (const rblx::code::DocEntry& page : rblx::code::editorPages()) {
            docIndex.push_back(page);
        }
        for (const rblx::code::DocEntry& page : rblx::code::referencePages(activeLanguage())) {
            docIndex.push_back(page);
        }
        if (active >= 0) {
            const OpenFile& file = files[static_cast<std::size_t>(active)];
            const std::string text = file.editor->GetText();
            const Language language = rblx::code::languageForPath(file.relative);
            docIndex.push_back(rblx::code::documentationForFile(file.relative, text));
            for (const OutlineSymbol& symbol : rblx::code::outlineSymbols(text, language)) {
                rblx::code::DocEntry entry;
                entry.name = symbol.name;
                entry.kind = symbol.kind.empty() ? "symbol" : symbol.kind;
                entry.signature = symbol.detail;
                const std::string comment = rblx::code::docCommentAbove(text, symbol.line);
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
    const rblx::code::DocEntry& pageForFile(const std::string& relative) {
        if (relative == filePagePath) {
            return filePageCache;
        }
        filePagePath = relative;
        for (const OpenFile& file : files) {
            if (file.relative == relative) {
                filePageCache =
                    rblx::code::documentationForFile(relative, file.editor->GetText());
                return filePageCache;
            }
        }
        std::string text;
        std::string error;
        if (!rblx::code::readSourceFile(rblx::code::joinProjectPath(root, relative), &text,
                                        &error)) {
            text.clear();
        }
        filePageCache = rblx::code::documentationForFile(relative, text);
        return filePageCache;
    }

    void drawFileTooltip(const std::string& relative) {
        const rblx::code::DocEntry& page = pageForFile(relative);
        RblxUI::BeginTooltip();
        RblxUI::TextUnformatted(relative.c_str());
        RblxUI::Separator();
        RblxUI::TextDisabled("%s", page.signature.c_str());
        RblxUI::PushTextWrapPos(RblxUI::GetFontSize() * 30.0f);
        RblxUI::TextUnformatted(page.summary.c_str());
        RblxUI::PopTextWrapPos();
        if (RblxUI::SmallButton("Open documentation")) {
            openDoc(page);
        }
        RblxUI::EndTooltip();
    }

    void drawHoverTooltip() {
        RblxUI::BeginTooltip();
        RblxUI::TextUnformatted(hoverPage.name.c_str());
        RblxUI::SameLine();
        RblxUI::TextDisabled("%s", hoverPage.kind.c_str());
        RblxUI::Separator();
        if (!hoverPage.signature.empty()) {
            RblxUI::PushStyleColor(RblxUICol_Text, rblx::ui::theme::accentStrong());
            RblxUI::TextUnformatted(hoverPage.signature.c_str());
            RblxUI::PopStyleColor();
        }
        RblxUI::PushTextWrapPos(RblxUI::GetFontSize() * 34.0f);
        RblxUI::TextUnformatted(hoverPage.summary.c_str());
        RblxUI::PopTextWrapPos();
        if (RblxUI::SmallButton("Open documentation  (F1)")) {
            openDoc(hoverPage);
        }
        RblxUI::EndTooltip();
    }

    // ---------------------------------------------------------- completions

    void refreshCompletions(bool force) {
        if (active < 0) {
            completionOpen = false;
            return;
        }
        const OpenFile& file = files[static_cast<std::size_t>(active)];
        const rblx::code::CompletionList list = rblx::code::completionsAt(
            file.editor->GetText(), rblx::code::languageForPath(file.relative), caretOffset());
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
        RblxTextEditor& editor = *file.editor;
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
        if (RblxUI::IsKeyPressed(RblxUIKey_Escape)) {
            completionOpen = false;
            swallowKeysThisFrame = true;
            return;
        }
        if (RblxUI::IsKeyPressed(RblxUIKey_DownArrow)) {
            completionIndex = count == 0 ? 0 : (completionIndex + 1) % count;
            swallowKeysThisFrame = true;
        }
        if (RblxUI::IsKeyPressed(RblxUIKey_UpArrow)) {
            completionIndex = count == 0 ? 0 : (completionIndex + count - 1) % count;
            swallowKeysThisFrame = true;
        }
        if (RblxUI::IsKeyPressed(RblxUIKey_Enter) || RblxUI::IsKeyPressed(RblxUIKey_KeypadEnter) ||
            RblxUI::IsKeyPressed(RblxUIKey_Tab)) {
            acceptCompletion();
            swallowKeysThisFrame = true;
        }
    }

    void drawCompletionPopup() {
        if (!completionOpen || completions.items.empty()) {
            return;
        }
        const RblxUIIO& io = RblxUI::GetIO();
        const float width = 430.0f;
        const float height = 236.0f;
        const float x = std::max(8.0f, std::min(io.MousePos.x + 16.0f, io.DisplaySize.x - width - 8.0f));
        const float y = std::max(8.0f, std::min(io.MousePos.y + 20.0f, io.DisplaySize.y - height - 8.0f));
        RblxUI::SetNextWindowPos(RblxVec2(x, y), RblxUICond_Always);
        RblxUI::SetNextWindowSize(RblxVec2(width, height), RblxUICond_Always);
        const RblxUIWindowFlags flags =
            RblxUIWindowFlags_NoTitleBar | RblxUIWindowFlags_NoResize | RblxUIWindowFlags_NoMove |
            RblxUIWindowFlags_NoSavedSettings | RblxUIWindowFlags_NoFocusOnAppearing |
            RblxUIWindowFlags_NoNavInputs;
        RblxUI::Begin("##completions", nullptr, flags);
        RblxUI::TextDisabled("%s  (%zu)", completions.prefix.empty() ? "all names" : completions.prefix.c_str(),
                            completions.items.size());
        RblxUI::Separator();
        RblxUI::BeginChild("##completionlist", RblxVec2(0.0f, -42.0f));
        for (std::size_t i = 0; i < completions.items.size(); ++i) {
            const CompletionItem& item = completions.items[i];
            RblxUI::PushID(static_cast<int>(i));
            const std::string label = item.label + "    " + item.kind;
            if (RblxUI::Selectable(label.c_str(), static_cast<int>(i) == completionIndex)) {
                completionIndex = static_cast<int>(i);
                acceptCompletion();
                RblxUI::PopID();
                break;
            }
            RblxUI::PopID();
        }
        RblxUI::EndChild();
        RblxUI::Separator();
        if (completionIndex >= 0 && completionIndex < static_cast<int>(completions.items.size())) {
            const CompletionItem& item = completions.items[static_cast<std::size_t>(completionIndex)];
            RblxUI::TextDisabled("%s", item.detail.c_str());
            if (!item.documentation.empty()) {
                RblxUI::PushTextWrapPos(0.0f);
                RblxUI::TextUnformatted(item.documentation.c_str());
                RblxUI::PopTextWrapPos();
            }
        }
        RblxUI::End();
    }

    void drawDocumentationWindow() {
        if (!showDoc) {
            return;
        }
        if (docIndexDirty || docIndexFile != active) {
            rebuildDocIndex();
        }
        RblxUI::SetNextWindowSize(RblxVec2(860.0f, 580.0f), RblxUICond_FirstUseEver);
        if (!RblxUI::Begin("Documentation", &showDoc)) {
            RblxUI::End();
            return;
        }

        RblxUI::BeginChild("##doclist", RblxVec2(280.0f, 0.0f), true);
        RblxUI::SetNextItemWidth(-1.0f);
        RblxUI::InputTextWithHint("##docfilter", "Filter pages", docFilter, sizeof(docFilter));
        RblxUI::Separator();
        const std::string filter = docFilter;
        for (std::size_t i = 0; i < docIndex.size(); ++i) {
            const rblx::code::DocEntry& page = docIndex[i];
            if (!filter.empty() && rblx::code::fuzzyScore(page.name, filter) < 0) {
                continue;
            }
            RblxUI::PushID(static_cast<int>(i));
            const std::string label = page.kind + "   " + page.name;
            if (RblxUI::Selectable(label.c_str(), docPage.name == page.name)) {
                openDoc(page);
            }
            RblxUI::PopID();
        }
        RblxUI::EndChild();

        RblxUI::SameLine();
        RblxUI::BeginChild("##docpage", RblxVec2(0.0f, 0.0f), true);
        if (docPage.name.empty()) {
            RblxUI::TextDisabled(
                "Choose a page, or put the caret on a name and press F1.");
        } else {
            RblxUI::TextUnformatted(docPage.name.c_str());
            RblxUI::SameLine();
            RblxUI::TextDisabled("%s", docPage.kind.c_str());
            RblxUI::Separator();
            if (!docPage.signature.empty()) {
                RblxUI::PushStyleColor(RblxUICol_Text, rblx::ui::theme::accentStrong());
                RblxUI::TextUnformatted(docPage.signature.c_str());
                RblxUI::PopStyleColor();
            }
            RblxUI::Separator();
            RblxUI::PushTextWrapPos(0.0f);
            RblxUI::TextUnformatted(docPage.body.c_str());
            RblxUI::PopTextWrapPos();
            if (!docPage.seeAlso.empty()) {
                RblxUI::Separator();
                RblxUI::TextDisabled("See also");
                const std::vector<std::string> links = docPage.seeAlso;
                for (const std::string& name : links) {
                    if (RblxUI::SmallButton(name.c_str())) {
                        const rblx::code::DocEntry* page =
                            rblx::code::findReferencePage(name, activeLanguage());
                        if (page != nullptr) {
                            openDoc(*page);
                        } else {
                            status = "No page for " + name;
                        }
                    }
                }
            }
        }
        RblxUI::EndChild();
        RblxUI::End();
    }

    // --------------------------------------------------------------- drawing

    void drawMenuBar(Window& window) {
        if (!RblxUI::BeginMainMenuBar()) {
            return;
        }
        const bool hasFile = active >= 0;
        RblxTextEditor* editor = hasFile ? files[static_cast<std::size_t>(active)].editor.get() : nullptr;

        if (RblxUI::BeginMenu("File")) {
            if (RblxUI::MenuItem("Save", "Ctrl+S", false, hasFile)) {
                saveFile(active);
            }
            if (RblxUI::MenuItem("Save all", "Ctrl+Shift+S", false, !files.empty())) {
                saveAll();
            }
            if (RblxUI::MenuItem("Reload from disk", "Ctrl+R", false, hasFile)) {
                reloadActive();
            }
            if (RblxUI::MenuItem("Close tab", "Ctrl+W", false, hasFile)) {
                requestClose(static_cast<std::size_t>(active));
            }
            RblxUI::Separator();
            if (RblxUI::MenuItem("Open folder...")) {
                openFolderRequested = true;
                focusFolder = true;
            }
            if (RblxUI::MenuItem("Refresh tree", "F5")) {
                refresh();
            }
            RblxUI::Separator();
            if (RblxUI::MenuItem("Exit")) {
                rblxwinSetWindowShouldClose(window.getNativeWindow(), true);
            }
            RblxUI::EndMenu();
        }
        if (RblxUI::BeginMenu("Edit")) {
            if (RblxUI::MenuItem("Undo", "Ctrl+Z", false, editor != nullptr && editor->CanUndo())) {
                editor->Undo();
            }
            if (RblxUI::MenuItem("Redo", "Ctrl+Y", false, editor != nullptr && editor->CanRedo())) {
                editor->Redo();
            }
            RblxUI::Separator();
            if (RblxUI::MenuItem("Cut", "Ctrl+X", false, editor != nullptr && editor->HasSelection())) {
                editor->Cut();
            }
            if (RblxUI::MenuItem("Copy", "Ctrl+C", false, editor != nullptr && editor->HasSelection())) {
                editor->Copy();
            }
            if (RblxUI::MenuItem("Paste", "Ctrl+V", false, editor != nullptr)) {
                editor->Paste();
            }
            if (RblxUI::MenuItem("Select all", "Ctrl+A", false, editor != nullptr)) {
                editor->SelectAll();
            }
            RblxUI::Separator();
            if (RblxUI::MenuItem("Find in file", "Ctrl+F", false, hasFile)) {
                openFindBar(false);
            }
            if (RblxUI::MenuItem("Replace in file", "Ctrl+H", false, hasFile)) {
                openFindBar(true);
            }
            if (RblxUI::MenuItem("Find next", "F3", false, hasFile)) {
                findStep(true);
            }
            if (RblxUI::MenuItem("Find previous", "Shift+F3", false, hasFile)) {
                findStep(false);
            }
            RblxUI::Separator();
            if (RblxUI::MenuItem("Toggle line comment", "Ctrl+/", false, hasFile)) {
                toggleComment();
            }
            if (RblxUI::MenuItem("Code suggestions", "Ctrl+Space", false, hasFile)) {
                completionRequested = true;
            }
            if (RblxUI::MenuItem("Go to matching bracket", "Ctrl+]", false, hasFile)) {
                jumpToMatchingBracket();
            }
            RblxUI::EndMenu();
        }
        if (RblxUI::BeginMenu("Go")) {
            if (RblxUI::MenuItem("Quick open...", "Ctrl+P")) {
                quickOpenRequested = true;
                focusQuickFilter = true;
            }
            if (RblxUI::MenuItem("Go to line...", "Ctrl+G", false, hasFile)) {
                goToLineRequested = true;
                focusGoToLine = true;
            }
            if (RblxUI::MenuItem("Go to symbol...", "Ctrl+Shift+O", false, hasFile)) {
                symbolPaletteRequested = true;
                focusSymbolFilter = true;
            }
            if (RblxUI::MenuItem("Find in project...", "Ctrl+Shift+F")) {
                focusQuery = true;
            }
            RblxUI::Separator();
            if (RblxUI::MenuItem("Back", "Alt+Left", false, !history.empty())) {
                navigate(-1);
            }
            if (RblxUI::MenuItem("Forward", "Alt+Right", false, !history.empty())) {
                navigate(1);
            }
            RblxUI::Separator();
            if (RblxUI::MenuItem("Next tab", "Ctrl+Tab", false, files.size() > 1)) {
                cycleTab(1);
            }
            if (RblxUI::MenuItem("Previous tab", "Ctrl+Shift+Tab", false, files.size() > 1)) {
                cycleTab(-1);
            }
            RblxUI::EndMenu();
        }
        if (RblxUI::BeginMenu("Build")) {
            if (RblxUI::MenuItem("Build", "Ctrl+B", false, !root.empty())) {
                startBuild();
            }
            if (RblxUI::MenuItem("Cancel build", "", false, build.running())) {
                stopBuild();
            }
            RblxUI::Separator();
            RblxUI::MenuItem("Build panel", "", &showBuild);
            if (RblxUI::MenuItem("Source control panel", "", showChanges)) {
                showChanges = !showChanges;
                if (showChanges) {
                    refreshGit();
                }
            }
            RblxUI::EndMenu();
        }
        if (RblxUI::BeginMenu("View")) {
            RblxUI::MenuItem("Outline panel", "", &showOutline);
            RblxUI::MenuItem("Show whitespace", "", &showWhitespace);
            RblxUI::MenuItem("Light theme", "", &lightTheme);
            if (RblxUI::BeginMenu("Tab size")) {
                if (RblxUI::MenuItem("2", "", tabSize == 2)) {
                    tabSize = 2;
                }
                if (RblxUI::MenuItem("4", "", tabSize == 4)) {
                    tabSize = 4;
                }
                if (RblxUI::MenuItem("8", "", tabSize == 8)) {
                    tabSize = 8;
                }
                RblxUI::EndMenu();
            }
            // A widget keeps its own copy of these, so the open buffers are told
            // about the change rather than only the next one to be opened.
            applyPreferences();
            RblxUI::EndMenu();
        }
        if (RblxUI::BeginMenu("Help")) {
            if (RblxUI::MenuItem("Documentation", "F1", false, hasFile)) {
                openDocAtCaret();
            }
            if (RblxUI::MenuItem("Documentation window")) {
                showDoc = true;
            }
            if (RblxUI::MenuItem("Keyboard shortcuts")) {
                showShortcuts = true;
            }
            RblxUI::EndMenu();
        }
        RblxUI::SameLine(RblxUI::GetWindowWidth() - 520.0f);
        RblxUI::TextDisabled("%s", root.c_str());
        RblxUI::EndMainMenuBar();
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
        if (!rblx::code::loadChanges(repo, &changes, &error)) {
            repoNote = error;
            return;
        }
        repoNote = rblx::code::changesSummary(changes);
        const std::string name = repo.branch();
        if (!name.empty()) {
            branch = name;
        }
    }

    void commitChanges() {
        std::string id;
        std::string error;
        std::size_t committed = 0;
        if (!rblx::code::commitEverything(repo, commitMessage, &id, &committed, &error)) {
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
        RblxUI::TextDisabled("%s", heading.c_str());
        RblxUI::TextWrapped("%s", repoNote.c_str());
        if (RblxUI::SmallButton("Refresh status")) {
            refreshGit();
        }
        RblxUI::Separator();

        RblxUI::SetNextItemWidth(-1.0f);
        RblxUI::InputTextWithHint("##commit-message", "Commit message", commitMessage,
                                  sizeof(commitMessage));
        if (!repoOpen) {
            RblxUI::TextDisabled("This folder is not a repository");
        } else if (changes.empty()) {
            RblxUI::TextDisabled("Nothing to commit");
        } else if (RblxUI::Button("Stage all and commit")) {
            commitChanges();
        }
        RblxUI::Separator();

        RblxUI::BeginChild("##changes");
        for (const rblx::code::ChangeRow& row : changes) {
            RblxUI::PushID(row.path.c_str());
            const std::string label = rblx::code::changeLabel(row) + "  " + row.path;
            if (RblxUI::Selectable(label.c_str(), false)) {
                openRelative(row.path);
            }
            if (RblxUI::IsItemHovered()) {
                RblxUI::SetTooltip("%s", row.path.c_str());
            }
            RblxUI::PopID();
        }
        RblxUI::EndChild();
    }

    // ---------------------------------------------------------------------
    // Build

    void startBuild() {
        showBuild = true;
        if (build.running()) {
            status = "A build is already running";
            return;
        }
        std::string error;
        buildRoot = rblx::code::findBuildRoot(root);
        if (!rblx::code::findBuildTool(buildRoot, &buildProgram, &error)) {
            buildState = error;
            status = error;
            buildFailed = true;
            return;
        }
        buildCommand = rblx::code::buildCommandLine(buildProgram, buildPreset);
        if (!build.start(buildRoot, buildProgram, buildCommand, &error)) {
            buildState = error;
            status = error;
            buildFailed = true;
            return;
        }
        buildFailed = false;
        buildLines.clear();
        buildLineCount = 0;
        buildDiagnostics.clear();
        buildDiagnosticByText.clear();
        buildPending = true;
        buildState = "building with " + buildPreset;
        status = "Building with " + buildProgram;
    }

    void stopBuild() {
        if (!build.running()) {
            return;
        }
        build.stop();
        buildState = "cancelled";
        status = "Build cancelled";
        buildFailed = false;
    }

    // Called once a frame. The log is copied when it grows rather than every frame,
    // and the diagnostics are read once, when the tool exits.
    void pollBuild() {
        if (build.running()) {
            const std::size_t count = build.lineCount();
            if (count != buildLineCount) {
                buildLines = build.lines();
                buildLineCount = buildLines.size();
            }
            return;
        }
        if (!buildPending || !build.finished()) {
            return;
        }
        buildPending = false;
        buildLines = build.lines();
        buildLineCount = buildLines.size();
        buildDiagnostics = rblx::code::parseDiagnostics(buildLines);
        buildDiagnosticByText.clear();
        std::size_t errors = 0;
        std::size_t warnings = 0;
        for (std::size_t index = 0; index < buildDiagnostics.size(); ++index) {
            const rblx::code::Diagnostic& diagnostic = buildDiagnostics[index];
            if (diagnostic.severity == "error") {
                ++errors;
            } else if (diagnostic.severity == "warning") {
                ++warnings;
            }
            buildDiagnosticByText.emplace(diagnostic.text, index);
        }
        const int code = build.exitCode();
        buildFailed = code != 0;
        if (code == 0) {
            buildState = "build succeeded";
        } else if (code < 0) {
            buildState = "the build tool did not run";
        } else {
            buildState = "build failed (exit " + std::to_string(code) + ")";
        }
        if (!buildDiagnostics.empty()) {
            buildState += " - " + std::to_string(errors) + " error(s), " +
                          std::to_string(warnings) + " warning(s)";
        }
        // The editor is running, so the link step of RBLXCode.exe itself cannot
        // finish. When that is the only failure the checkout is otherwise up to
        // date, and calling it a failed build hides the work that did happen.
        if (buildFailed && errors == 0 && rblx::code::selfLinkBlocked(buildLines)) {
            buildFailed = false;
            buildState = std::string("everything but ") + rblx::code::kEditorProgram +
                         " was built - Windows will not replace the editor while it is "
                         "running, so close it and build again to update the editor itself";
        }
        status = buildState;
    }

    void openDiagnostic(const rblx::code::Diagnostic& diagnostic) {
        const std::string& base = buildRoot.empty() ? root : buildRoot;
        const std::string absolute = rblx::code::resolveDiagnosticPath(base, diagnostic.path);
        std::string relative = rblx::code::relativePathOf(root, absolute);
        if (relative.empty()) {
            relative = diagnostic.path;
        }
        openAt(relative, diagnostic.line > 0 ? diagnostic.line : 1, diagnostic.column);
    }

    void drawBuildPanel() {
        if (RblxUI::SmallButton(build.running() ? "Build (running)" : "Build")) {
            startBuild();
        }
        if (build.running()) {
            RblxUI::SameLine();
            if (RblxUI::SmallButton("Cancel")) {
                stopBuild();
            }
        }
        RblxUI::SameLine();
        if (RblxUI::SmallButton("Close")) {
            showBuild = false;
        }
        RblxUI::SameLine();
        if (buildFailed) {
            RblxUI::PushStyleColor(RblxUICol_Text, rblx::ui::theme::error());
            RblxUI::Text("%s", buildState.c_str());
            RblxUI::PopStyleColor();
        } else {
            RblxUI::TextDisabled("%s", buildState.c_str());
        }
        // The exact command and the folder it runs in, so a build that does not
        // work can be repeated in a terminal without guessing either.
        if (!buildCommand.empty()) {
            RblxUI::TextDisabled("%s", buildCommand.c_str());
            if (buildRoot != root) {
                RblxUI::TextDisabled("in %s", buildRoot.c_str());
            }
        }
        RblxUI::Separator();

        RblxUI::BeginChild("##buildlog", RblxVec2(0.0f, 0.0f), false,
                           RblxUIWindowFlags_HorizontalScrollbar);
        if (buildLines.empty()) {
            RblxUI::TextDisabled("Ctrl+B builds this checkout with the tool in build/, "
                                 "preset %s",
                                 buildPreset.c_str());
        }
        for (const std::string& line : buildLines) {
            const auto found = buildDiagnosticByText.find(line);
            if (found == buildDiagnosticByText.end()) {
                RblxUI::TextUnformatted(line.c_str());
                continue;
            }
            const rblx::code::Diagnostic& diagnostic = buildDiagnostics[found->second];
            RblxUI::PushStyleColor(RblxUICol_Text, diagnostic.severity == "error"
                                                       ? rblx::ui::theme::error()
                                                       : rblx::ui::theme::warning());
            if (RblxUI::Selectable(line.c_str())) {
                openDiagnostic(diagnostic);
            }
            RblxUI::PopStyleColor();
        }
        if (build.running()) {
            RblxUI::SetScrollHereY(1.0f);
        }
        RblxUI::EndChild();
    }

    void drawSidebar() {
        if (RblxUI::SmallButton(showChanges ? "Files" : "[Files]")) {
            showChanges = false;
        }
        RblxUI::SameLine();
        if (RblxUI::SmallButton(showChanges ? "[Source control]" : "Source control")) {
            showChanges = true;
            refreshGit();
        }
        RblxUI::Separator();
        if (showChanges) {
            drawSourceControl();
            return;
        }
        RblxUI::TextDisabled("%s", root.empty() ? "No folder open" : root.c_str());
        RblxUI::Separator();

        if (focusQuery) {
            RblxUI::SetKeyboardFocusHere();
            focusQuery = false;
        }
        RblxUI::SetNextItemWidth(-1.0f);
        RblxUI::InputTextWithHint("##query", "Search in project (Ctrl+Shift+F)", query, sizeof(query));

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
            RblxUI::TextDisabled("%zu result%s", hits.size(), hits.size() == 1 ? "" : "s");
            const float resultsHeight = std::min(220.0f, RblxUI::GetTextLineHeightWithSpacing() * 9.0f);
            RblxUI::BeginChild("##results", RblxVec2(0.0f, resultsHeight), true);
            for (const SearchHit& hit : hits) {
                RblxUI::PushID(hit.relativePath.c_str());
                RblxUI::PushID(hit.line);
                const std::string label = hit.relativePath + ":" + std::to_string(hit.line) + "  " +
                                          hit.text;
                if (RblxUI::Selectable(label.c_str())) {
                    openAt(hit.relativePath, hit.line);
                }
                RblxUI::PopID();
                RblxUI::PopID();
            }
            if (hits.empty()) {
                RblxUI::TextDisabled("No match");
            }
            RblxUI::EndChild();
            RblxUI::Separator();
        }

        RblxUI::BeginChild("##tree");
        if (!root.empty()) {
            drawTree(tree, true);
        }
        RblxUI::EndChild();
    }

    void drawTree(const TreeNode& node, bool firstLevel) {
        for (const TreeNode& child : node.children) {
            if (child.entry.directory) {
                RblxUITreeNodeFlags flags =
                    RblxUITreeNodeFlags_OpenOnArrow | RblxUITreeNodeFlags_SpanAvailWidth;
                if (firstLevel) {
                    flags |= RblxUITreeNodeFlags_DefaultOpen;
                }
                const bool open = RblxUI::TreeNodeEx(child.entry.relativePath.c_str(), flags,
                                                    "%s", child.entry.name.c_str());
                if (open) {
                    drawTree(child, false);
                    RblxUI::TreePop();
                }
                continue;
            }
            RblxUI::PushID(child.entry.relativePath.c_str());
            const bool selected =
                active >= 0 && files[static_cast<std::size_t>(active)].relative ==
                                    child.entry.relativePath;
            if (RblxUI::Selectable(child.entry.name.c_str(), selected)) {
                openRelative(child.entry.relativePath);
            }
            if (RblxUI::IsItemHovered(RblxUIHoveredFlags_DelayNormal)) {
                drawFileTooltip(child.entry.relativePath);
            }
            RblxUI::PopID();
        }
    }

    void drawTabs() {
        std::vector<std::size_t> closing;
        if (RblxUI::BeginTabBar("##tabs", RblxUITabBarFlags_AutoSelectNewTabs |
                                             RblxUITabBarFlags_Reorderable |
                                             RblxUITabBarFlags_FittingPolicyScroll)) {
            for (std::size_t i = 0; i < files.size(); ++i) {
                OpenFile& file = files[i];
                const bool dirty = isDirty(file);
                const std::string label =
                    (dirty ? "* " : "") + file.name + "###" + file.relative;
                bool open = true;
                RblxUITabItemFlags flags = 0;
                if (selectOnOpen && static_cast<int>(i) == active) {
                    flags |= RblxUITabItemFlags_SetSelected;
                }
                if (RblxUI::BeginTabItem(label.c_str(), &open, flags)) {
                    if (active != static_cast<int>(i)) {
                        active = static_cast<int>(i);
                        findDirty = true;
                        outlineDirty = true;
                    }
                    RblxUI::EndTabItem();
                }
                if (RblxUI::IsItemHovered(RblxUIHoveredFlags_DelayNormal)) {
                    drawFileTooltip(file.relative);
                }
                if (!open) {
                    closing.push_back(i);
                }
            }
            RblxUI::EndTabBar();
        }
        selectOnOpen = false;
        // Closed from the end so the indices of the tabs still to close stay valid.
        for (auto it = closing.rbegin(); it != closing.rend(); ++it) {
            requestClose(*it);
        }
    }

    float findBarHeight() const {
        float height = RblxUI::GetFrameHeightWithSpacing();
        if (showReplaceField) {
            height += RblxUI::GetFrameHeightWithSpacing();
        }
        return height + RblxUI::GetStyle().ItemSpacing.y;
    }

    void drawFindBar() {
        RblxUI::BeginChild("##findbar", RblxVec2(0.0f, findBarHeight()), true);

        if (focusFind) {
            RblxUI::SetKeyboardFocusHere();
            focusFind = false;
        }
        RblxUI::SetNextItemWidth(260.0f);
        const bool submitted = RblxUI::InputTextWithHint(
            "##find", "Find in file (Ctrl+F)", findText, sizeof(findText),
            RblxUIInputTextFlags_EnterReturnsTrue);
        if (submitted) {
            findStep(!RblxUI::GetIO().KeyShift);
        }
        if (RblxUI::IsItemEdited()) {
            findDirty = true;
            findNote.clear();
        }

        RblxUI::SameLine();
        if (RblxUI::SmallButton("Prev")) {
            findStep(false);
        }
        RblxUI::SameLine();
        if (RblxUI::SmallButton("Next")) {
            findStep(true);
        }
        RblxUI::SameLine();
        if (RblxUI::Checkbox("Match case", &findOptions.caseSensitive)) {
            findDirty = true;
        }
        RblxUI::SameLine();
        if (RblxUI::Checkbox("Whole word", &findOptions.wholeWord)) {
            findDirty = true;
        }
        ensureFindMatches();
        RblxUI::SameLine();
        if (!findMatches.empty()) {
            RblxUI::TextDisabled("%d of %zu", findIndex + 1, findMatches.size());
        } else if (findText[0] != '\0') {
            RblxUI::TextDisabled("No match");
        }

        RblxUI::SameLine();
        if (RblxUI::SmallButton(showReplaceField ? "Hide replace" : "Replace")) {
            showReplaceField = !showReplaceField;
            if (showReplaceField) {
                focusReplace = true;
            }
        }
        RblxUI::SameLine();
        if (RblxUI::SmallButton("Close")) {
            showFind = false;
            showReplaceField = false;
        }

        if (showReplaceField) {
            if (focusReplace) {
                RblxUI::SetKeyboardFocusHere();
                focusReplace = false;
            }
            RblxUI::SetNextItemWidth(260.0f);
            RblxUI::InputTextWithHint("##replace", "Replace with", replaceText,
                                     sizeof(replaceText));
            RblxUI::SameLine();
            if (RblxUI::SmallButton("Replace")) {
                replaceCurrent();
            }
            RblxUI::SameLine();
            if (RblxUI::SmallButton("Replace all")) {
                replaceAllMatches();
            }
        }
        if (!findNote.empty()) {
            RblxUI::SameLine();
            RblxUI::TextDisabled("%s", findNote.c_str());
        }
        RblxUI::EndChild();
    }

    void drawOutlinePanel() {
        RblxUI::TextDisabled("Outline");
        RblxUI::SameLine();
        if (RblxUI::SmallButton("Symbols...")) {
            symbolPaletteRequested = true;
            focusSymbolFilter = true;
        }
        RblxUI::Separator();
        if (active < 0) {
            RblxUI::TextDisabled("No file open");
            return;
        }

        const std::vector<OutlineSymbol>& symbols = activeOutline();
        RblxUI::TextDisabled("%zu symbol%s", symbols.size(), symbols.size() == 1 ? "" : "s");
        RblxUI::BeginChild("##outlinelist");
        for (std::size_t i = 0; i < symbols.size(); ++i) {
            const OutlineSymbol& symbol = symbols[i];
            RblxUI::PushID(static_cast<int>(i));
            std::string label;
            for (int depth = 0; depth < symbol.depth && depth < 6; ++depth) {
                label += "    ";
            }
            label += symbol.name;
            if (RblxUI::Selectable(label.c_str())) {
                jumpToLine(symbol.line);
            }
            if (RblxUI::IsItemHovered()) {
                RblxUI::SetTooltip("%s  (line %d)", symbol.detail.c_str(), symbol.line);
            }
            RblxUI::PopID();
        }
        if (symbols.empty()) {
            RblxUI::TextDisabled("No symbols in this file");
        }
        RblxUI::EndChild();
    }

    void drawEditorArea() {
        drawTabs();
        if (active < 0 || active >= static_cast<int>(files.size())) {
            RblxUI::TextDisabled("Choose a file in the tree, or press Ctrl+P to open one by name.");
            return;
        }

        OpenFile& file = files[static_cast<std::size_t>(active)];
        float footer = RblxUI::GetTextLineHeightWithSpacing() + RblxUI::GetStyle().ItemSpacing.y * 2.0f;
        if (showFind) {
            footer += findBarHeight();
        }
        if (file.stale) {
            footer += RblxUI::GetFrameHeightWithSpacing();
        }
        // A key the completion list has just used belongs to the list, not to the
        // text: the widget is told not to look at the keyboard for this frame.
        if (swallowKeysThisFrame) {
            file.editor->SetHandleKeyboardInputs(false);
        }
        file.editor->Render("##code", RblxVec2(0.0f, -footer), true);
        if (swallowKeysThisFrame) {
            file.editor->SetHandleKeyboardInputs(true);
            swallowKeysThisFrame = false;
        }
        const RblxVec2 editorOrigin = RblxUI::GetItemRectMin();
        const bool editorHovered = RblxUI::IsItemHovered();
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
        hoverPage = rblx::code::DocEntry();
        const Language languageUnderMouse = rblx::code::languageForPath(file.relative);
        if (editorHovered && !completionOpen) {
            const RblxTextEditor::Coordinates under =
                file.editor->ScreenPosToCoordinates(RblxUI::GetMousePos(), editorOrigin);
            const std::string hoverText = file.editor->GetText();
            if (under.mLine >= 0 && under.mLine < file.editor->GetTotalLines()) {
                const std::size_t lineStart = rblx::code::offsetOfLine(hoverText, under.mLine + 1);
                std::size_t lineEnd = hoverText.find('\n', lineStart);
                if (lineEnd == std::string::npos) {
                    lineEnd = hoverText.size();
                }
                const std::string lineText = hoverText.substr(lineStart, lineEnd - lineStart);
                const std::size_t offset =
                    lineStart + rblx::code::byteOffsetOfColumn(lineText, under.mColumn, tabSize);
                hoverPage =
                    rblx::code::documentationAt(hoverText, languageUnderMouse, offset);
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
            RblxUI::PushStyleColor(RblxUICol_Text, rblx::ui::theme::warning());
            RblxUI::TextUnformatted("This file changed on disk while you had edits in it.");
            RblxUI::PopStyleColor();
            RblxUI::SameLine();
            if (RblxUI::SmallButton("Reload and lose my edits")) {
                reloadActive();
            }
            RblxUI::SameLine();
            if (RblxUI::SmallButton("Keep editing")) {
                file.stale = false;
                status = "Kept the buffer for " + file.relative;
            }
        }

        if (showFind) {
            drawFindBar();
        }

        const RblxTextEditor::Coordinates cursor = file.editor->GetCursorPosition();
        const Language language = rblx::code::languageForPath(file.relative);
        std::string line = file.relative + "  |  Ln " + std::to_string(cursor.mLine + 1) +
                           ", Col " + std::to_string(cursor.mColumn + 1) + "  |  " +
                           rblx::code::languageName(language) + "  |  " +
                           std::to_string(file.editor->GetTotalLines()) + " lines  |  " +
                           (isDirty(file) ? "modified" : "saved");
        if (!branch.empty()) {
            line += "  |  " + branch;
        }
        if (file.editor->HasSelection()) {
            line += "  |  " + std::to_string(file.editor->GetSelectedText().size()) + " selected";
        }
        RblxUI::TextDisabled("%s", line.c_str());
        RblxUI::SameLine();
        RblxUI::TextDisabled("%s", status.c_str());
    }

    void handleKeys() {
        RblxUIIO& io = RblxUI::GetIO();
        const bool ctrl = io.KeyCtrl;
        const bool shift = io.KeyShift;

        handleCompletionKeys();
        if (RblxUI::IsKeyPressed(RblxUIKey_F1)) {
            openDocAtCaret();
        }
        if (ctrl && RblxUI::IsKeyPressed(RblxUIKey_Space)) {
            completionRequested = true;
        }

        if (ctrl && RblxUI::IsKeyPressed(RblxUIKey_S)) {
            if (shift) {
                saveAll();
            } else {
                saveFile(active);
            }
        }
        if (ctrl && RblxUI::IsKeyPressed(RblxUIKey_R)) {
            reloadActive();
        }
        if (ctrl && RblxUI::IsKeyPressed(RblxUIKey_W) && active >= 0) {
            requestClose(static_cast<std::size_t>(active));
        }
        if (ctrl && RblxUI::IsKeyPressed(RblxUIKey_P)) {
            quickOpenRequested = true;
            focusQuickFilter = true;
        }
        if (ctrl && RblxUI::IsKeyPressed(RblxUIKey_G)) {
            goToLineRequested = true;
            focusGoToLine = true;
        }
        if (ctrl && shift && RblxUI::IsKeyPressed(RblxUIKey_O)) {
            symbolPaletteRequested = true;
            focusSymbolFilter = true;
        }
        if (ctrl && RblxUI::IsKeyPressed(RblxUIKey_F)) {
            if (shift) {
                focusQuery = true;
            } else {
                openFindBar(false);
            }
        }
        if (ctrl && RblxUI::IsKeyPressed(RblxUIKey_H)) {
            openFindBar(true);
        }
        if (RblxUI::IsKeyPressed(RblxUIKey_F3)) {
            findStep(!shift);
        }
        if (showFind && RblxUI::IsKeyPressed(RblxUIKey_Escape) &&
            !RblxUI::IsPopupOpen("", RblxUIPopupFlags_AnyPopup)) {
            showFind = false;
            showReplaceField = false;
        }
        if (ctrl && RblxUI::IsKeyPressed(RblxUIKey_Slash)) {
            toggleComment();
        }
        if (ctrl && RblxUI::IsKeyPressed(RblxUIKey_RightBracket)) {
            jumpToMatchingBracket();
        }
        if (io.KeyAlt && RblxUI::IsKeyPressed(RblxUIKey_LeftArrow)) {
            navigate(-1);
        }
        if (io.KeyAlt && RblxUI::IsKeyPressed(RblxUIKey_RightArrow)) {
            navigate(1);
        }
        if (ctrl && RblxUI::IsKeyPressed(RblxUIKey_Tab)) {
            cycleTab(shift ? -1 : 1);
        }
        if (ctrl && RblxUI::IsKeyPressed(RblxUIKey_B)) {
            startBuild();
        }
        if (RblxUI::IsKeyPressed(RblxUIKey_F5)) {
            refresh();
        }
    }

    // A ranked list over the scanned files: the query only has to be an
    // in-order abbreviation of the path, so "cmain" finds src/CodeMain.cpp.
    void drawQuickOpen() {
        if (!RblxUI::BeginPopupModal("Quick open", nullptr, RblxUIWindowFlags_NoSavedSettings)) {
            return;
        }
        RblxUI::SetNextWindowSize(RblxVec2(620.0f, 440.0f), RblxUICond_Appearing);
        if (focusQuickFilter) {
            RblxUI::SetKeyboardFocusHere();
            focusQuickFilter = false;
        }
        RblxUI::SetNextItemWidth(-1.0f);
        const bool submitted = RblxUI::InputText("##quick", quickFilter, sizeof(quickFilter),
                                                RblxUIInputTextFlags_EnterReturnsTrue);

        std::vector<std::pair<int, const FileEntry*>> ranked;
        for (const FileEntry& entry : entries) {
            if (entry.directory) {
                continue;
            }
            const int score = rblx::code::fuzzyScore(entry.relativePath, quickFilter);
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

        RblxUI::BeginChild("##matches", RblxVec2(0.0f, 330.0f), true);
        for (const std::pair<int, const FileEntry*>& match : ranked) {
            if (RblxUI::Selectable(match.second->relativePath.c_str())) {
                openRelative(match.second->relativePath);
                RblxUI::CloseCurrentPopup();
            }
        }
        if (ranked.empty()) {
            RblxUI::TextDisabled("No file matches");
        }
        RblxUI::EndChild();
        if (submitted && !ranked.empty()) {
            openRelative(ranked.front().second->relativePath);
            RblxUI::CloseCurrentPopup();
        }
        if (RblxUI::Button("Cancel")) {
            RblxUI::CloseCurrentPopup();
        }
        RblxUI::EndPopup();
    }

    void drawSymbolPalette() {
        if (!RblxUI::BeginPopupModal("Go to symbol", nullptr, RblxUIWindowFlags_NoSavedSettings)) {
            return;
        }
        RblxUI::SetNextWindowSize(RblxVec2(560.0f, 420.0f), RblxUICond_Appearing);
        if (focusSymbolFilter) {
            RblxUI::SetKeyboardFocusHere();
            focusSymbolFilter = false;
        }
        RblxUI::SetNextItemWidth(-1.0f);
        const bool submitted = RblxUI::InputTextWithHint("##symbol", "Symbol name",
                                                        symbolFilter, sizeof(symbolFilter),
                                                        RblxUIInputTextFlags_EnterReturnsTrue);

        const std::vector<OutlineSymbol> symbols = activeOutline();
        std::vector<const OutlineSymbol*> shown;
        for (const OutlineSymbol& symbol : symbols) {
            if (rblx::code::fuzzyScore(symbol.name, symbolFilter) < 0) {
                continue;
            }
            shown.push_back(&symbol);
            if (shown.size() >= 200) {
                break;
            }
        }

        RblxUI::BeginChild("##symbols", RblxVec2(0.0f, 310.0f), true);
        for (const OutlineSymbol* symbol : shown) {
            const std::string label = symbol->kind + "  " + symbol->name + "    (line " +
                                      std::to_string(symbol->line) + ")";
            if (RblxUI::Selectable(label.c_str())) {
                jumpToLine(symbol->line);
                RblxUI::CloseCurrentPopup();
            }
        }
        if (shown.empty()) {
            RblxUI::TextDisabled("No symbol matches");
        }
        RblxUI::EndChild();
        if (submitted && !shown.empty()) {
            jumpToLine(shown.front()->line);
            RblxUI::CloseCurrentPopup();
        }
        if (RblxUI::Button("Cancel")) {
            RblxUI::CloseCurrentPopup();
        }
        RblxUI::EndPopup();
    }

    void drawGoToLine() {
        if (!RblxUI::BeginPopupModal("Go to line", nullptr, RblxUIWindowFlags_AlwaysAutoResize)) {
            return;
        }
        if (focusGoToLine) {
            RblxUI::SetKeyboardFocusHere();
            focusGoToLine = false;
        }
        RblxUI::SetNextItemWidth(140.0f);
        const bool submitted = RblxUI::InputText("Line", goToLine, sizeof(goToLine),
                                                RblxUIInputTextFlags_EnterReturnsTrue);
        if (submitted && active >= 0) {
            const int line = std::atoi(goToLine);
            if (line > 0) {
                openAt(files[static_cast<std::size_t>(active)].relative, line);
            }
            RblxUI::CloseCurrentPopup();
        }
        if (RblxUI::Button("Cancel")) {
            RblxUI::CloseCurrentPopup();
        }
        RblxUI::EndPopup();
    }

    void drawOpenFolder() {
        if (!RblxUI::BeginPopupModal("Open folder", nullptr, RblxUIWindowFlags_AlwaysAutoResize)) {
            return;
        }
        if (focusFolder) {
            RblxUI::SetKeyboardFocusHere();
            focusFolder = false;
        }
        RblxUI::SetNextItemWidth(520.0f);
        const bool submitted = RblxUI::InputText("Folder", folderPath, sizeof(folderPath),
                                                RblxUIInputTextFlags_EnterReturnsTrue);
        if (submitted || RblxUI::Button("Open")) {
            std::error_code error;
            if (std::filesystem::is_directory(folderPath, error)) {
                const std::string opened = folderPath;
                openFolderRequested = false;
                setRoot(opened);
                status = "Opened " + root;
            } else {
                status = std::string("No folder at ") + folderPath;
            }
            RblxUI::CloseCurrentPopup();
        }
        RblxUI::SameLine();
        if (RblxUI::Button("Cancel")) {
            RblxUI::CloseCurrentPopup();
        }
        RblxUI::EndPopup();
    }

    void drawUnsavedPopup() {
        if (!RblxUI::BeginPopupModal("Unsaved changes", nullptr, RblxUIWindowFlags_AlwaysAutoResize)) {
            return;
        }
        if (pendingClose >= files.size()) {
            RblxUI::CloseCurrentPopup();
            RblxUI::EndPopup();
            return;
        }
        OpenFile& file = files[pendingClose];
        const bool changedOnDisk = !file.stamp.empty() &&
                                   rblx::code::fileStamp(file.absolute) != file.stamp;
        RblxUI::Text("Save %s before closing it?", file.name.c_str());
        if (changedOnDisk) {
            RblxUI::TextDisabled("It also changed on disk since it was opened.");
        }
        if (RblxUI::Button("Save and close")) {
            saveFile(static_cast<int>(pendingClose));
            const std::size_t index = pendingClose;
            pendingClose = kNoFile;
            closeTab(index);
            RblxUI::CloseCurrentPopup();
        }
        RblxUI::SameLine();
        if (RblxUI::Button("Close without saving")) {
            const std::size_t index = pendingClose;
            pendingClose = kNoFile;
            closeTab(index);
            RblxUI::CloseCurrentPopup();
        }
        RblxUI::SameLine();
        if (RblxUI::Button("Cancel")) {
            pendingClose = kNoFile;
            RblxUI::CloseCurrentPopup();
        }
        RblxUI::EndPopup();
    }

    void drawShortcuts() {
        if (!showShortcuts) {
            return;
        }
        RblxUI::SetNextWindowSize(RblxVec2(560.0f, 430.0f), RblxUICond_FirstUseEver);
        if (RblxUI::Begin("Keyboard shortcuts", &showShortcuts)) {
            RblxUI::TextUnformatted("Ctrl+Space   suggest what fits here");
            RblxUI::TextUnformatted("F1   documentation for the name under the caret");
            RblxUI::TextUnformatted("Ctrl+P   open a file by name");
            RblxUI::TextUnformatted("Ctrl+Shift+O   go to a symbol in this file");
            RblxUI::TextUnformatted("Ctrl+G   go to a line");
            RblxUI::TextUnformatted("Ctrl+Shift+F   search the whole project");
            RblxUI::Separator();
            RblxUI::TextUnformatted("Ctrl+F   find in this file");
            RblxUI::TextUnformatted("Ctrl+H   find and replace in this file");
            RblxUI::TextUnformatted("F3 / Shift+F3   next / previous match");
            RblxUI::TextUnformatted("Ctrl+/   comment or uncomment the line");
            RblxUI::Separator();
            RblxUI::TextUnformatted("Ctrl+S   save the file in front");
            RblxUI::TextUnformatted("Ctrl+Shift+S   save every open file");
            RblxUI::TextUnformatted("Ctrl+R   reload the file from disk");
            RblxUI::TextUnformatted("Ctrl+W   close the file in front");
            RblxUI::TextUnformatted("Ctrl+Tab   next tab, Ctrl+Shift+Tab   previous tab");
            RblxUI::Separator();
            RblxUI::TextUnformatted("Ctrl+B   build the checkout with the tool in build/");
            RblxUI::TextUnformatted(
                "Source control (left panel)   the changed files, stage everything, commit");
            RblxUI::Separator();
            RblxUI::TextUnformatted("Alt+Left / Alt+Right   back and forward between jumps");
            RblxUI::TextUnformatted("Ctrl+]   jump to the matching bracket");
            RblxUI::Separator();
            RblxUI::TextWrapped(
                "The text widget keeps the editing keys: Ctrl+Z and Ctrl+Y undo and redo, Ctrl+C, "
                "Ctrl+X and Ctrl+V copy, cut and paste, and Ctrl+A selects everything.");
        }
        RblxUI::End();
    }

    void drawPopups() {
        if (quickOpenRequested) {
            RblxUI::OpenPopup("Quick open");
            quickOpenRequested = false;
        }
        if (goToLineRequested) {
            RblxUI::OpenPopup("Go to line");
            goToLineRequested = false;
        }
        if (symbolPaletteRequested) {
            RblxUI::OpenPopup("Go to symbol");
            symbolPaletteRequested = false;
        }
        if (openFolderRequested) {
            RblxUI::OpenPopup("Open folder");
            openFolderRequested = false;
        }
        if (pendingClose != kNoFile && pendingClose < files.size() &&
            isDirty(files[pendingClose])) {
            RblxUI::OpenPopup("Unsaved changes");
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
        checkDiskChanges();
        pollBuild();
        handleKeys();
        drawMenuBar(window);

        const RblxUIViewport* viewport = RblxUI::GetMainViewport();
        RblxUI::SetNextWindowPos(viewport->WorkPos);
        RblxUI::SetNextWindowSize(viewport->WorkSize);
        const RblxUIWindowFlags workspaceFlags =
            RblxUIWindowFlags_NoTitleBar | RblxUIWindowFlags_NoResize | RblxUIWindowFlags_NoMove |
            RblxUIWindowFlags_NoCollapse | RblxUIWindowFlags_NoBringToFrontOnFocus |
            RblxUIWindowFlags_NoScrollbar | RblxUIWindowFlags_NoScrollWithMouse |
            RblxUIWindowFlags_NoSavedSettings;
        RblxUI::Begin("##workspace", nullptr, workspaceFlags);

        RblxUI::BeginChild("##sidebar", RblxVec2(kSidebarWidth, 0.0f), true);
        drawSidebar();
        RblxUI::EndChild();

        RblxUI::SameLine();
        if (showOutline) {
            RblxUI::BeginChild("##outline", RblxVec2(kOutlineWidth, 0.0f), true);
            drawOutlinePanel();
            RblxUI::EndChild();
            RblxUI::SameLine();
        }

        RblxUI::BeginChild("##editor", RblxVec2(0.0f, showBuild ? -190.0f : 0.0f), false,
                          RblxUIWindowFlags_NoScrollbar | RblxUIWindowFlags_NoScrollWithMouse);
        drawEditorArea();
        RblxUI::EndChild();

        if (showBuild) {
            RblxUI::BeginChild("##build", RblxVec2(0.0f, 178.0f), true);
            drawBuildPanel();
            RblxUI::EndChild();
        }

        drawCompletionPopup();
        drawDocumentationWindow();
        // The workspace is the one window this frame opened, so it is closed once:
        // a second End() lands with no window open and rblxui asserts on it.
        RblxUI::End();
        drawPopups();
    }
};

void beginRblxUI(Window& window) {
    RBLXUI_CHECKVERSION();
    RblxUI::CreateContext();
    RblxUIIO& io = RblxUI::GetIO();
    io.ConfigFlags |= RblxUIConfigFlags_NavEnableKeyboard;
    // The layout is fixed in code, so no rblxui.ini is written beside the
    // executable and every checkout opens the same way.
    io.IniFilename = nullptr;
    // One palette for every window in the project, from UiTheme.h.
    rblx::ui::theme::apply();
    RblxUI_ImplRblxWin_InitForOpenGL(window.getNativeWindow(), true);
    RblxUI_ImplOpenGL3_Init("#version 330");
}

}  // namespace

int main(int argc, char** argv) {
    try {
        Window window(kWindowWidth, kWindowHeight, "RBLX Clone Code");
        beginRblxUI(window);

        App app;
        std::string start = argc > 1 ? argv[1] : std::filesystem::current_path().string();
        app.setRoot(start);
        app.status = "Editing " + app.root +
                     " - Ctrl+P opens a file, Ctrl+F finds in it, F1 documents what is under "
                     "the caret";

        while (!window.shouldClose()) {
            window.pollEvents();

            RblxUI_ImplOpenGL3_NewFrame();
            RblxUI_ImplRblxWin_NewFrame();
            RblxUI::NewFrame();

            app.draw(window);

            RblxUI::Render();
            int displayWidth = 0;
            int displayHeight = 0;
            rblxwinGetFramebufferSize(window.getNativeWindow(), &displayWidth, &displayHeight);
            glViewport(0, 0, displayWidth, displayHeight);
            glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            RblxUI_ImplOpenGL3_RenderDrawData(RblxUI::GetDrawData());
            window.swapBuffers();
        }

        RblxUI_ImplOpenGL3_Shutdown();
        RblxUI_ImplRblxWin_Shutdown();
        RblxUI::DestroyContext();
    } catch (const std::exception& error) {
        std::fprintf(stderr, "RBLXCode could not start: %s\n", error.what());
        return 1;
    }
    return 0;
}
