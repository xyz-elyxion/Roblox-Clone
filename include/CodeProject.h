#pragma once

#include <cstddef>
#include <string>
#include <vector>

// The parts of the source editor that do not need a window: what the project
// tree holds, which file is which language, how a file is read and written, and
// how a search finds the line it names. They live here so a headless test can
// exercise them; `src/CodeMain.cpp` is only the drawing.
namespace rblx {
namespace code {

// What the editor colours a file as. The pinned TextEditor widget carries a
// definition per language family, so this is the choice of which one to hand it.
enum class Language {
    Plain,
    Cpp,
    Glsl,
    Lua,
    JavaScript,
    Html,
    Css,
    Markdown,
    Json
};

Language languageForPath(const std::string& path);
// Short name for the status bar: "C++", "GLSL", "RBLX script".
const char* languageName(Language language);

// Directories that are never part of the source a person edits. Matched by name
// at any depth, because a dependency checkout can carry its own build folder and
// a nested `vendor` is third-party code either way.
bool isIgnoredDirectory(const std::string& name);
// Files worth opening as text: source, shaders, scripts, pages and project files.
// Images, models and packaged binaries are filtered by extension, so the tree
// never offers something the editor would then refuse.
bool isEditableFile(const std::string& name);

struct FileEntry {
    std::string relativePath;  // '/' separators, relative to the project root
    std::string name;          // last segment
    bool directory = false;
    int depth = 0;             // 0 for an entry directly under the root
};

// One walk of the project in a stable order: directories before files, both
// case-insensitively alphabetical, so the tree does not reshuffle between frames.
// `maxEntries` stops a walk of something enormous instead of freezing the window.
std::vector<FileEntry> scanProject(const std::string& root, std::size_t maxEntries = 20000);

// The same entries as a tree, ready for a recursive draw.
struct TreeNode {
    FileEntry entry;
    std::vector<TreeNode> children;
};
TreeNode buildTree(const std::vector<FileEntry>& entries);

// A file is shown with '\n' line endings whatever it uses on disk, so the widget
// does not draw a stray '\r' at the end of every line. The save puts back what the
// file had, byte for byte: this repository is not uniform (the anti-cheat sources
// are CRLF and most of the rest is LF), and an editor that rewrote every line of a
// file it touched would bury a one-line fix in a whole-file diff.
struct SourceDocument {
    std::string text;
    bool crlf = false;
};
SourceDocument decodeSource(const std::string& bytes);
std::string encodeSource(const SourceDocument& document);

// Reads a file the editor can show. A missing file, one larger than `maxBytes`
// and one holding a NUL byte all report a reason instead of a buffer: the NUL is
// how a binary announces itself, and a huge file would freeze the frame.
bool readSourceFile(const std::string& path, std::string* text, std::string* error,
                    std::size_t maxBytes = 4 * 1024 * 1024);
bool writeSourceFile(const std::string& path, const std::string& text, std::string* error);

struct SearchHit {
    std::string relativePath;
    int line = 0;     // 1-based
    std::string text;  // the matching line, trailing whitespace trimmed
};

// Case-insensitive substring search over the editable files of a scanned tree.
// It stops at `maxHits`, so typing in the box cannot stall on a large repository.
std::vector<SearchHit> searchProject(const std::string& root,
                                     const std::vector<FileEntry>& entries,
                                     const std::string& query, std::size_t maxHits = 200);

// Byte offset <-> line arithmetic, both counted from one where a reader counts
// from one. The status bar and a search hit share it, so a jump lands on the line
// the results list named.
void lineAndColumn(const std::string& text, std::size_t offset, int* line, int* column);
std::size_t offsetOfLine(const std::string& text, int line);
int lineCount(const std::string& text);

std::string fileNameOf(const std::string& path);
std::string relativePathOf(const std::string& root, const std::string& path);
std::string formatBytes(std::size_t bytes);
// Joins a project-relative path onto a root, accepting either separator in the
// relative part so an entry built on Windows still opens on a POSIX build.
std::string joinProjectPath(const std::string& root, const std::string& relativePath);

}  // namespace code
}  // namespace rblx
