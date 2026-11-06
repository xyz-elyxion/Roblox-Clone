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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
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
    // The file's size and modification time when it was read or written, so an
    // edit made outside the editor is noticed instead of being overwritten.
    std::string stamp;
    // Set when the file changed on disk while this buffer had unsaved edits; the
    // editor asks before throwing them away.
    bool stale = false;
    std::unique_ptr<TextEditor> editor;
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
    bool lightTheme = false;
    double nextDiskCheck = 0.0;

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
        branch = rblx::code::gitBranchOf(root);
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
            file.editor->SetPalette(lightTheme ? TextEditor::GetLightPalette()
                                               : TextEditor::GetDarkPalette());
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

        file.editor = std::make_unique<TextEditor>();
        file.editor->SetTabSize(tabSize);
        file.editor->SetShowWhitespaces(showWhitespace);
        file.editor->SetPalette(lightTheme ? TextEditor::GetLightPalette()
                                           : TextEditor::GetDarkPalette());
        file.editor->SetLanguageDefinition(definitionFor(rblx::code::languageForPath(relative)));
        file.editor->SetText(file.document.text);
        file.editor->SetCursorPosition(TextEditor::Coordinates(0, 0));

        files.push_back(std::move(file));
        active = static_cast<int>(files.size()) - 1;
        selectOnOpen = true;
        findDirty = true;
        outlineDirty = true;
        status = "Opened " + relative;
    }

    // The widget counts a tab as the columns it draws, so a byte offset in the
    // text has to be turned into a place the caret can be put.
    TextEditor::Coordinates coordinatesFor(const std::string& text, std::size_t offset) const {
        int line = 1;
        int column = 1;
        rblx::code::lineAndColumn(text, offset, &line, &column);
        const std::size_t start = rblx::code::offsetOfLine(text, line);
        std::size_t end = text.find('\n', start);
        if (end == std::string::npos) {
            end = text.size();
        }
        const std::string lineText = text.substr(start, end - start);
        return TextEditor::Coordinates(
            line - 1, rblx::code::displayColumn(lineText, static_cast<std::size_t>(column - 1), tabSize));
    }

    void openAt(const std::string& relative, int line, int column = 0) {
        rememberPosition();
        openRelative(relative);
        if (active < 0) {
            return;
        }
        TextEditor& editor = *files[static_cast<std::size_t>(active)].editor;
        const int clamped = std::max(1, std::min(line, editor.GetTotalLines()));
        const std::string text = editor.GetText();
        const std::size_t start = rblx::code::offsetOfLine(text, clamped);
        std::size_t end = text.find('\n', start);
        if (end == std::string::npos) {
            end = text.size();
        }
        const std::string lineText = text.substr(start, end - start);
        editor.SetCursorPosition(TextEditor::Coordinates(
            clamped - 1, rblx::code::byteOffsetOfColumn(lineText, column, tabSize)));
    }

    void jumpToLine(int line) {
        if (active < 0) {
            return;
        }
        rememberPosition();
        TextEditor& editor = *files[static_cast<std::size_t>(active)].editor;
        const int clamped = std::max(1, std::min(line, editor.GetTotalLines()));
        editor.SetCursorPosition(TextEditor::Coordinates(clamped - 1, 0));
    }

    // One entry per place a jump left, so Alt+Left returns to what was being read
    // before the jump rather than to wherever the caret happened to be.
    void rememberPosition() {
        if (navigating || active < 0) {
            return;
        }
        const OpenFile& file = files[static_cast<std::size_t>(active)];
        const TextEditor::Coordinates cursor = file.editor->GetCursorPosition();
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
            TextEditor& editor = *files[static_cast<std::size_t>(active)].editor;
            const int line = std::max(1, std::min(point.line, editor.GetTotalLines()));
            const std::string text = editor.GetText();
            const std::size_t start = rblx::code::offsetOfLine(text, line);
            std::size_t end = text.find('\n', start);
            if (end == std::string::npos) {
                end = text.size();
            }
            const std::string lineText = text.substr(start, end - start);
            editor.SetCursorPosition(TextEditor::Coordinates(
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
        const TextEditor::Coordinates caret = file.editor->GetCursorPosition();
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
        const double now = ImGui::GetTime();
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
            const TextEditor::Coordinates caret = file.editor->GetCursorPosition();
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
        TextEditor& editor = *files[static_cast<std::size_t>(active)].editor;
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
        TextEditor& editor = *file.editor;
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
        file.editor->SetCursorPosition(TextEditor::Coordinates(0, 0));
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
        TextEditor& editor = *file.editor;
        const std::string prefix =
            rblx::code::lineCommentPrefix(rblx::code::languageForPath(file.relative));
        if (prefix.empty()) {
            status = "This file has no line comment to toggle.";
            return;
        }

        const TextEditor::Coordinates cursor = editor.GetCursorPosition();
        const std::string lineText = editor.GetCurrentLineText();
        std::size_t marker = 0;
        if (rblx::code::lineHasComment(lineText, prefix, &marker)) {
            const int begin = rblx::code::displayColumn(lineText, marker, tabSize);
            const int end = rblx::code::displayColumn(lineText, marker + prefix.size(), tabSize);
            editor.SetSelection(TextEditor::Coordinates(cursor.mLine, begin),
                                TextEditor::Coordinates(cursor.mLine, end));
            editor.Delete();
            editor.SetCursorPosition(TextEditor::Coordinates(cursor.mLine, begin));
        } else {
            std::size_t indent = 0;
            while (indent < lineText.size() && (lineText[indent] == ' ' || lineText[indent] == '\t')) {
                ++indent;
            }
            const int column = rblx::code::displayColumn(lineText, indent, tabSize);
            editor.SetCursorPosition(TextEditor::Coordinates(cursor.mLine, column));
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
        TextEditor& editor = *file.editor;
        const std::string text = editor.GetText();
        const TextEditor::Coordinates cursor = editor.GetCursorPosition();
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
        const TextEditor::Coordinates cursor = file.editor->GetCursorPosition();
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
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(relative.c_str());
        ImGui::Separator();
        ImGui::TextDisabled("%s", page.signature.c_str());
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
        ImGui::TextUnformatted(page.summary.c_str());
        ImGui::PopTextWrapPos();
        if (ImGui::SmallButton("Open documentation")) {
            openDoc(page);
        }
        ImGui::EndTooltip();
    }

    void drawHoverTooltip() {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(hoverPage.name.c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("%s", hoverPage.kind.c_str());
        ImGui::Separator();
        if (!hoverPage.signature.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.72f, 0.80f, 0.94f, 1.0f));
            ImGui::TextUnformatted(hoverPage.signature.c_str());
            ImGui::PopStyleColor();
        }
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 34.0f);
        ImGui::TextUnformatted(hoverPage.summary.c_str());
        ImGui::PopTextWrapPos();
        if (ImGui::SmallButton("Open documentation  (F1)")) {
            openDoc(hoverPage);
        }
        ImGui::EndTooltip();
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
        TextEditor& editor = *file.editor;
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
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            completionOpen = false;
            swallowKeysThisFrame = true;
            return;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            completionIndex = count == 0 ? 0 : (completionIndex + 1) % count;
            swallowKeysThisFrame = true;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            completionIndex = count == 0 ? 0 : (completionIndex + count - 1) % count;
            swallowKeysThisFrame = true;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter) ||
            ImGui::IsKeyPressed(ImGuiKey_Tab)) {
            acceptCompletion();
            swallowKeysThisFrame = true;
        }
    }

    void drawCompletionPopup() {
        if (!completionOpen || completions.items.empty()) {
            return;
        }
        const ImGuiIO& io = ImGui::GetIO();
        const float width = 430.0f;
        const float height = 236.0f;
        const float x = std::max(8.0f, std::min(io.MousePos.x + 16.0f, io.DisplaySize.x - width - 8.0f));
        const float y = std::max(8.0f, std::min(io.MousePos.y + 20.0f, io.DisplaySize.y - height - 8.0f));
        ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
        const ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNavInputs;
        ImGui::Begin("##completions", nullptr, flags);
        ImGui::TextDisabled("%s  (%zu)", completions.prefix.empty() ? "all names" : completions.prefix.c_str(),
                            completions.items.size());
        ImGui::Separator();
        ImGui::BeginChild("##completionlist", ImVec2(0.0f, -42.0f));
        for (std::size_t i = 0; i < completions.items.size(); ++i) {
            const CompletionItem& item = completions.items[i];
            ImGui::PushID(static_cast<int>(i));
            const std::string label = item.label + "    " + item.kind;
            if (ImGui::Selectable(label.c_str(), static_cast<int>(i) == completionIndex)) {
                completionIndex = static_cast<int>(i);
                acceptCompletion();
                ImGui::PopID();
                break;
            }
            ImGui::PopID();
        }
        ImGui::EndChild();
        ImGui::Separator();
        if (completionIndex >= 0 && completionIndex < static_cast<int>(completions.items.size())) {
            const CompletionItem& item = completions.items[static_cast<std::size_t>(completionIndex)];
            ImGui::TextDisabled("%s", item.detail.c_str());
            if (!item.documentation.empty()) {
                ImGui::PushTextWrapPos(0.0f);
                ImGui::TextUnformatted(item.documentation.c_str());
                ImGui::PopTextWrapPos();
            }
        }
        ImGui::End();
    }

    void drawDocumentationWindow() {
        if (!showDoc) {
            return;
        }
        if (docIndexDirty || docIndexFile != active) {
            rebuildDocIndex();
        }
        ImGui::SetNextWindowSize(ImVec2(860.0f, 580.0f), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Documentation", &showDoc)) {
            ImGui::End();
            return;
        }

        ImGui::BeginChild("##doclist", ImVec2(280.0f, 0.0f), true);
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint("##docfilter", "Filter pages", docFilter, sizeof(docFilter));
        ImGui::Separator();
        const std::string filter = docFilter;
        for (std::size_t i = 0; i < docIndex.size(); ++i) {
            const rblx::code::DocEntry& page = docIndex[i];
            if (!filter.empty() && rblx::code::fuzzyScore(page.name, filter) < 0) {
                continue;
            }
            ImGui::PushID(static_cast<int>(i));
            const std::string label = page.kind + "   " + page.name;
            if (ImGui::Selectable(label.c_str(), docPage.name == page.name)) {
                openDoc(page);
            }
            ImGui::PopID();
        }
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("##docpage", ImVec2(0.0f, 0.0f), true);
        if (docPage.name.empty()) {
            ImGui::TextDisabled(
                "Choose a page, or put the caret on a name and press F1.");
        } else {
            ImGui::TextUnformatted(docPage.name.c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("%s", docPage.kind.c_str());
            ImGui::Separator();
            if (!docPage.signature.empty()) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.72f, 0.80f, 0.94f, 1.0f));
                ImGui::TextUnformatted(docPage.signature.c_str());
                ImGui::PopStyleColor();
            }
            ImGui::Separator();
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextUnformatted(docPage.body.c_str());
            ImGui::PopTextWrapPos();
            if (!docPage.seeAlso.empty()) {
                ImGui::Separator();
                ImGui::TextDisabled("See also");
                const std::vector<std::string> links = docPage.seeAlso;
                for (const std::string& name : links) {
                    if (ImGui::SmallButton(name.c_str())) {
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
        ImGui::EndChild();
        ImGui::End();
    }

    // --------------------------------------------------------------- drawing

    void drawMenuBar(Window& window) {
        if (!ImGui::BeginMainMenuBar()) {
            return;
        }
        const bool hasFile = active >= 0;
        TextEditor* editor = hasFile ? files[static_cast<std::size_t>(active)].editor.get() : nullptr;

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save", "Ctrl+S", false, hasFile)) {
                saveFile(active);
            }
            if (ImGui::MenuItem("Save all", "Ctrl+Shift+S", false, !files.empty())) {
                saveAll();
            }
            if (ImGui::MenuItem("Reload from disk", "Ctrl+R", false, hasFile)) {
                reloadActive();
            }
            if (ImGui::MenuItem("Close tab", "Ctrl+W", false, hasFile)) {
                requestClose(static_cast<std::size_t>(active));
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Open folder...")) {
                openFolderRequested = true;
                focusFolder = true;
            }
            if (ImGui::MenuItem("Refresh tree", "F5")) {
                refresh();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(window.getNativeWindow(), true);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, editor != nullptr && editor->CanUndo())) {
                editor->Undo();
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, editor != nullptr && editor->CanRedo())) {
                editor->Redo();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X", false, editor != nullptr && editor->HasSelection())) {
                editor->Cut();
            }
            if (ImGui::MenuItem("Copy", "Ctrl+C", false, editor != nullptr && editor->HasSelection())) {
                editor->Copy();
            }
            if (ImGui::MenuItem("Paste", "Ctrl+V", false, editor != nullptr)) {
                editor->Paste();
            }
            if (ImGui::MenuItem("Select all", "Ctrl+A", false, editor != nullptr)) {
                editor->SelectAll();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Find in file", "Ctrl+F", false, hasFile)) {
                openFindBar(false);
            }
            if (ImGui::MenuItem("Replace in file", "Ctrl+H", false, hasFile)) {
                openFindBar(true);
            }
            if (ImGui::MenuItem("Find next", "F3", false, hasFile)) {
                findStep(true);
            }
            if (ImGui::MenuItem("Find previous", "Shift+F3", false, hasFile)) {
                findStep(false);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Toggle line comment", "Ctrl+/", false, hasFile)) {
                toggleComment();
            }
            if (ImGui::MenuItem("Code suggestions", "Ctrl+Space", false, hasFile)) {
                completionRequested = true;
            }
            if (ImGui::MenuItem("Go to matching bracket", "Ctrl+]", false, hasFile)) {
                jumpToMatchingBracket();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Go")) {
            if (ImGui::MenuItem("Quick open...", "Ctrl+P")) {
                quickOpenRequested = true;
                focusQuickFilter = true;
            }
            if (ImGui::MenuItem("Go to line...", "Ctrl+G", false, hasFile)) {
                goToLineRequested = true;
                focusGoToLine = true;
            }
            if (ImGui::MenuItem("Go to symbol...", "Ctrl+Shift+O", false, hasFile)) {
                symbolPaletteRequested = true;
                focusSymbolFilter = true;
            }
            if (ImGui::MenuItem("Find in project...", "Ctrl+Shift+F")) {
                focusQuery = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Back", "Alt+Left", false, !history.empty())) {
                navigate(-1);
            }
            if (ImGui::MenuItem("Forward", "Alt+Right", false, !history.empty())) {
                navigate(1);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Next tab", "Ctrl+Tab", false, files.size() > 1)) {
                cycleTab(1);
            }
            if (ImGui::MenuItem("Previous tab", "Ctrl+Shift+Tab", false, files.size() > 1)) {
                cycleTab(-1);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Outline panel", "", &showOutline);
            ImGui::MenuItem("Show whitespace", "", &showWhitespace);
            ImGui::MenuItem("Light theme", "", &lightTheme);
            if (ImGui::BeginMenu("Tab size")) {
                if (ImGui::MenuItem("2", "", tabSize == 2)) {
                    tabSize = 2;
                }
                if (ImGui::MenuItem("4", "", tabSize == 4)) {
                    tabSize = 4;
                }
                if (ImGui::MenuItem("8", "", tabSize == 8)) {
                    tabSize = 8;
                }
                ImGui::EndMenu();
            }
            // A widget keeps its own copy of these, so the open buffers are told
            // about the change rather than only the next one to be opened.
            applyPreferences();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Documentation", "F1", false, hasFile)) {
                openDocAtCaret();
            }
            if (ImGui::MenuItem("Documentation window")) {
                showDoc = true;
            }
            if (ImGui::MenuItem("Keyboard shortcuts")) {
                showShortcuts = true;
            }
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
        ImGui::InputTextWithHint("##query", "Search in project (Ctrl+Shift+F)", query, sizeof(query));

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
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                drawFileTooltip(child.entry.relativePath);
            }
            ImGui::PopID();
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
                    if (active != static_cast<int>(i)) {
                        active = static_cast<int>(i);
                        findDirty = true;
                        outlineDirty = true;
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                    drawFileTooltip(file.relative);
                }
                if (!open) {
                    closing.push_back(i);
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

    float findBarHeight() const {
        float height = ImGui::GetFrameHeightWithSpacing();
        if (showReplaceField) {
            height += ImGui::GetFrameHeightWithSpacing();
        }
        return height + ImGui::GetStyle().ItemSpacing.y;
    }

    void drawFindBar() {
        ImGui::BeginChild("##findbar", ImVec2(0.0f, findBarHeight()), true);

        if (focusFind) {
            ImGui::SetKeyboardFocusHere();
            focusFind = false;
        }
        ImGui::SetNextItemWidth(260.0f);
        const bool submitted = ImGui::InputTextWithHint(
            "##find", "Find in file (Ctrl+F)", findText, sizeof(findText),
            ImGuiInputTextFlags_EnterReturnsTrue);
        if (submitted) {
            findStep(!ImGui::GetIO().KeyShift);
        }
        if (ImGui::IsItemEdited()) {
            findDirty = true;
            findNote.clear();
        }

        ImGui::SameLine();
        if (ImGui::SmallButton("Prev")) {
            findStep(false);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Next")) {
            findStep(true);
        }
        ImGui::SameLine();
        if (ImGui::Checkbox("Match case", &findOptions.caseSensitive)) {
            findDirty = true;
        }
        ImGui::SameLine();
        if (ImGui::Checkbox("Whole word", &findOptions.wholeWord)) {
            findDirty = true;
        }
        ensureFindMatches();
        ImGui::SameLine();
        if (!findMatches.empty()) {
            ImGui::TextDisabled("%d of %zu", findIndex + 1, findMatches.size());
        } else if (findText[0] != '\0') {
            ImGui::TextDisabled("No match");
        }

        ImGui::SameLine();
        if (ImGui::SmallButton(showReplaceField ? "Hide replace" : "Replace")) {
            showReplaceField = !showReplaceField;
            if (showReplaceField) {
                focusReplace = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Close")) {
            showFind = false;
            showReplaceField = false;
        }

        if (showReplaceField) {
            if (focusReplace) {
                ImGui::SetKeyboardFocusHere();
                focusReplace = false;
            }
            ImGui::SetNextItemWidth(260.0f);
            ImGui::InputTextWithHint("##replace", "Replace with", replaceText,
                                     sizeof(replaceText));
            ImGui::SameLine();
            if (ImGui::SmallButton("Replace")) {
                replaceCurrent();
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Replace all")) {
                replaceAllMatches();
            }
        }
        if (!findNote.empty()) {
            ImGui::SameLine();
            ImGui::TextDisabled("%s", findNote.c_str());
        }
        ImGui::EndChild();
    }

    void drawOutlinePanel() {
        ImGui::TextDisabled("Outline");
        ImGui::SameLine();
        if (ImGui::SmallButton("Symbols...")) {
            symbolPaletteRequested = true;
            focusSymbolFilter = true;
        }
        ImGui::Separator();
        if (active < 0) {
            ImGui::TextDisabled("No file open");
            return;
        }

        const std::vector<OutlineSymbol>& symbols = activeOutline();
        ImGui::TextDisabled("%zu symbol%s", symbols.size(), symbols.size() == 1 ? "" : "s");
        ImGui::BeginChild("##outlinelist");
        for (std::size_t i = 0; i < symbols.size(); ++i) {
            const OutlineSymbol& symbol = symbols[i];
            ImGui::PushID(static_cast<int>(i));
            std::string label;
            for (int depth = 0; depth < symbol.depth && depth < 6; ++depth) {
                label += "    ";
            }
            label += symbol.name;
            if (ImGui::Selectable(label.c_str())) {
                jumpToLine(symbol.line);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s  (line %d)", symbol.detail.c_str(), symbol.line);
            }
            ImGui::PopID();
        }
        if (symbols.empty()) {
            ImGui::TextDisabled("No symbols in this file");
        }
        ImGui::EndChild();
    }

    void drawEditorArea() {
        drawTabs();
        if (active < 0 || active >= static_cast<int>(files.size())) {
            ImGui::TextDisabled("Choose a file in the tree, or press Ctrl+P to open one by name.");
            return;
        }

        OpenFile& file = files[static_cast<std::size_t>(active)];
        float footer = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y * 2.0f;
        if (showFind) {
            footer += findBarHeight();
        }
        if (file.stale) {
            footer += ImGui::GetFrameHeightWithSpacing();
        }
        // A key the completion list has just used belongs to the list, not to the
        // text: the widget is told not to look at the keyboard for this frame.
        if (swallowKeysThisFrame) {
            file.editor->SetHandleKeyboardInputs(false);
        }
        file.editor->Render("##code", ImVec2(0.0f, -footer), true);
        if (swallowKeysThisFrame) {
            file.editor->SetHandleKeyboardInputs(true);
            swallowKeysThisFrame = false;
        }
        const ImVec2 editorOrigin = ImGui::GetItemRectMin();
        const bool editorHovered = ImGui::IsItemHovered();
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
            const TextEditor::Coordinates under =
                file.editor->ScreenPosToCoordinates(ImGui::GetMousePos(), editorOrigin);
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.72f, 0.28f, 1.0f));
            ImGui::TextUnformatted("This file changed on disk while you had edits in it.");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            if (ImGui::SmallButton("Reload and lose my edits")) {
                reloadActive();
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Keep editing")) {
                file.stale = false;
                status = "Kept the buffer for " + file.relative;
            }
        }

        if (showFind) {
            drawFindBar();
        }

        const TextEditor::Coordinates cursor = file.editor->GetCursorPosition();
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
        ImGui::TextDisabled("%s", line.c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status.c_str());
    }

    void handleKeys() {
        ImGuiIO& io = ImGui::GetIO();
        const bool ctrl = io.KeyCtrl;
        const bool shift = io.KeyShift;

        handleCompletionKeys();
        if (ImGui::IsKeyPressed(ImGuiKey_F1)) {
            openDocAtCaret();
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Space)) {
            completionRequested = true;
        }

        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
            if (shift) {
                saveAll();
            } else {
                saveFile(active);
            }
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_R)) {
            reloadActive();
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_W) && active >= 0) {
            requestClose(static_cast<std::size_t>(active));
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_P)) {
            quickOpenRequested = true;
            focusQuickFilter = true;
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_G)) {
            goToLineRequested = true;
            focusGoToLine = true;
        }
        if (ctrl && shift && ImGui::IsKeyPressed(ImGuiKey_O)) {
            symbolPaletteRequested = true;
            focusSymbolFilter = true;
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_F)) {
            if (shift) {
                focusQuery = true;
            } else {
                openFindBar(false);
            }
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_H)) {
            openFindBar(true);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_F3)) {
            findStep(!shift);
        }
        if (showFind && ImGui::IsKeyPressed(ImGuiKey_Escape) &&
            !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopup)) {
            showFind = false;
            showReplaceField = false;
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Slash)) {
            toggleComment();
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_RightBracket)) {
            jumpToMatchingBracket();
        }
        if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
            navigate(-1);
        }
        if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
            navigate(1);
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Tab)) {
            cycleTab(shift ? -1 : 1);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
            refresh();
        }
    }

    // A ranked list over the scanned files: the query only has to be an
    // in-order abbreviation of the path, so "cmain" finds src/CodeMain.cpp.
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

        ImGui::BeginChild("##matches", ImVec2(0.0f, 330.0f), true);
        for (const std::pair<int, const FileEntry*>& match : ranked) {
            if (ImGui::Selectable(match.second->relativePath.c_str())) {
                openRelative(match.second->relativePath);
                ImGui::CloseCurrentPopup();
            }
        }
        if (ranked.empty()) {
            ImGui::TextDisabled("No file matches");
        }
        ImGui::EndChild();
        if (submitted && !ranked.empty()) {
            openRelative(ranked.front().second->relativePath);
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    void drawSymbolPalette() {
        if (!ImGui::BeginPopupModal("Go to symbol", nullptr, ImGuiWindowFlags_NoSavedSettings)) {
            return;
        }
        ImGui::SetNextWindowSize(ImVec2(560.0f, 420.0f), ImGuiCond_Appearing);
        if (focusSymbolFilter) {
            ImGui::SetKeyboardFocusHere();
            focusSymbolFilter = false;
        }
        ImGui::SetNextItemWidth(-1.0f);
        const bool submitted = ImGui::InputTextWithHint("##symbol", "Symbol name",
                                                        symbolFilter, sizeof(symbolFilter),
                                                        ImGuiInputTextFlags_EnterReturnsTrue);

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

        ImGui::BeginChild("##symbols", ImVec2(0.0f, 310.0f), true);
        for (const OutlineSymbol* symbol : shown) {
            const std::string label = symbol->kind + "  " + symbol->name + "    (line " +
                                      std::to_string(symbol->line) + ")";
            if (ImGui::Selectable(label.c_str())) {
                jumpToLine(symbol->line);
                ImGui::CloseCurrentPopup();
            }
        }
        if (shown.empty()) {
            ImGui::TextDisabled("No symbol matches");
        }
        ImGui::EndChild();
        if (submitted && !shown.empty()) {
            jumpToLine(shown.front()->line);
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
                const std::string opened = folderPath;
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
        const bool changedOnDisk = !file.stamp.empty() &&
                                   rblx::code::fileStamp(file.absolute) != file.stamp;
        ImGui::Text("Save %s before closing it?", file.name.c_str());
        if (changedOnDisk) {
            ImGui::TextDisabled("It also changed on disk since it was opened.");
        }
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
        ImGui::SetNextWindowSize(ImVec2(560.0f, 430.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Keyboard shortcuts", &showShortcuts)) {
            ImGui::TextUnformatted("Ctrl+Space   suggest what fits here");
            ImGui::TextUnformatted("F1   documentation for the name under the caret");
            ImGui::TextUnformatted("Ctrl+P   open a file by name");
            ImGui::TextUnformatted("Ctrl+Shift+O   go to a symbol in this file");
            ImGui::TextUnformatted("Ctrl+G   go to a line");
            ImGui::TextUnformatted("Ctrl+Shift+F   search the whole project");
            ImGui::Separator();
            ImGui::TextUnformatted("Ctrl+F   find in this file");
            ImGui::TextUnformatted("Ctrl+H   find and replace in this file");
            ImGui::TextUnformatted("F3 / Shift+F3   next / previous match");
            ImGui::TextUnformatted("Ctrl+/   comment or uncomment the line");
            ImGui::Separator();
            ImGui::TextUnformatted("Ctrl+S   save the file in front");
            ImGui::TextUnformatted("Ctrl+Shift+S   save every open file");
            ImGui::TextUnformatted("Ctrl+R   reload the file from disk");
            ImGui::TextUnformatted("Ctrl+W   close the file in front");
            ImGui::TextUnformatted("Ctrl+Tab   next tab, Ctrl+Shift+Tab   previous tab");
            ImGui::Separator();
            ImGui::TextUnformatted("Alt+Left / Alt+Right   back and forward between jumps");
            ImGui::TextUnformatted("Ctrl+]   jump to the matching bracket");
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
        if (symbolPaletteRequested) {
            ImGui::OpenPopup("Go to symbol");
            symbolPaletteRequested = false;
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
        if (showOutline) {
            ImGui::BeginChild("##outline", ImVec2(kOutlineWidth, 0.0f), true);
            drawOutlinePanel();
            ImGui::EndChild();
            ImGui::SameLine();
        }

        ImGui::BeginChild("##editor", ImVec2(0.0f, 0.0f), false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        drawEditorArea();
        ImGui::EndChild();

        drawCompletionPopup();
        drawDocumentationWindow();
        ImGui::End();
        drawPopups();
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
        app.status = "Editing " + app.root +
                     " - Ctrl+P opens a file, Ctrl+F finds in it, F1 documents what is under "
                     "the caret";

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
