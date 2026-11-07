#pragma once

#include <cstddef>
#include <string>
#include <vector>

// The parts of the source editor that do not need a window: what the project
// tree holds, which file is which language, how a file is read and written, how
// a search finds the line it names, and the small pieces of editor behaviour
// that are worth testing on their own. They live here so a headless test can
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
// The marker that comments out one line of this language, or "" when the
// language has no single-line comment the editor can toggle.
const char* lineCommentPrefix(Language language);

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

// Find and replace inside one file. The widget has no search of its own, so the
// match list is built here and the window only walks it.
struct FindOptions {
    bool caseSensitive = false;
    bool wholeWord = false;
};

struct TextMatch {
    std::size_t offset = 0;  // byte offset in the text
    std::size_t length = 0;  // bytes, so a needle may span lines
    int line = 1;            // 1-based, for the status label
    int column = 1;          // 1-based byte column
};

// Every match in reading order. Matches never overlap, so "aa" in "aaaa" is two
// hits and replacing them both cannot loop.
std::vector<TextMatch> findInText(const std::string& text, const std::string& needle,
                                  const FindOptions& options = FindOptions(),
                                  std::size_t maxMatches = 5000);
// The next match at or after `fromOffset`, or the previous one before it. Both
// directions wrap, which is what makes one key walk the whole file.
bool findNextInText(const std::string& text, const std::string& needle,
                    const FindOptions& options, std::size_t fromOffset, bool forward,
                    TextMatch* match);
// Replaces every match and returns how many were replaced.
std::size_t replaceAllInText(std::string* text, const std::string& needle,
                             const std::string& replacement,
                             const FindOptions& options = FindOptions());

// A symbol panel or a Go to Symbol list, taken from the shape of the language:
// braces and declaration keywords in C++ and GLSL, `function` in RBLX Script and
// JavaScript, headings in Markdown and keys in JSON.
struct OutlineSymbol {
    std::string kind;    // "function", "class", "heading", "key"...
    std::string name;
    std::string detail;  // the declaration line, shortened
    int line = 1;        // 1-based
    int depth = 0;       // nesting the symbol sits at, for indenting the list
};
std::vector<OutlineSymbol> outlineSymbols(const std::string& text, Language language);

// The bracket that pairs with the one at `offset`, or npos when that byte is not
// a bracket or has no partner. Brackets inside strings and comments are ignored,
// so a character in a literal does not send the cursor across the file.
std::size_t matchingBracket(const std::string& text, std::size_t offset);

// The widget counts a tab as the spaces it draws, so a column in its coordinates
// has to be converted to and from a byte offset in the line.
int displayColumn(const std::string& line, std::size_t byteOffset, int tabSize);
std::size_t byteOffsetOfColumn(const std::string& line, int column, int tabSize);

// Whether a line already carries a comment marker, and where the marker starts.
bool lineHasComment(const std::string& line, const std::string& prefix,
                    std::size_t* markerOffset);

// How well `query` reads as an abbreviation of `candidate`, for ranking the
// quick open list. -1 when some character of the query is not there at all, so a
// file that cannot match never appears; higher is a better match.
int fuzzyScore(const std::string& candidate, const std::string& query);

// The branch a checkout is on, from `.git/HEAD`: "main", a short hash when HEAD
// is detached, or "" when the folder is not a checkout.
std::string gitBranchOf(const std::string& root);
// Which version of a file is on disk, as a value that changes when it does. The
// window keeps one per open tab so it can notice an edit made outside the editor
// instead of overwriting it on the next save. "" when the file cannot be read.
std::string fileStamp(const std::string& path);

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
// relative part so a path written with either separator still joins.
std::string joinProjectPath(const std::string& root, const std::string& relativePath);

// ---------------------------------------------------------------------------
// Documentation
// ---------------------------------------------------------------------------

// One page of documentation: what a name does. Pages come from three places --
// the reference that ships with the editor (the RBLX script API and the keywords
// of each other language), the comment above a declaration in the file being
// read, and the comment a file opens with. They all have the same shape, so the
// hover tooltip, the completion popup and the documentation window draw any of
// them without knowing which it is.
struct DocEntry {
    std::string name;       // "workspace.FindFirstChild", "Vector3", "src/WebStore.cpp"
    std::string kind;       // "function", "method", "field", "type", "keyword", "file", "symbol"
    std::string signature;  // one line: how it is written
    std::string summary;    // one sentence, for a tooltip
    std::string body;       // the full page, paragraphs separated by blank lines
    std::vector<std::string> seeAlso;  // names of related pages
};

// The pages that ship with the editor for a language family: the script API for
// RBLX Script and JavaScript, the keywords and built-ins of the rest.
const std::vector<DocEntry>& referencePages(Language language);
// A page by name: the exact name first, then a case-insensitive match, and a
// member's page by its bare name when the qualified one is not there. nullptr
// when the reference has nothing under that name.
const DocEntry* findReferencePage(const std::string& name, Language language);
// The pages that describe the editor itself rather than the language it edits.
const std::vector<DocEntry>& editorPages();

// What sits under a byte offset: the whole identifier chain around it
// (`workspace.FindFirstChild`), the part the offset is in, and what qualifies it.
struct WordUnderCursor {
    std::string chain;      // "workspace.FindFirstChild", written with '.' or ':'
    std::string word;       // "FindFirstChild"
    std::string qualifier;  // "workspace"
    std::size_t start = 0;  // byte offset where `word` starts
    std::size_t end = 0;    // byte offset just past it
    bool empty() const { return word.empty(); }
};
WordUnderCursor wordUnderCursor(const std::string& text, std::size_t offset);

// The documentation for whatever is at that offset: a reference page, else the
// declaration this file makes of that name, with the comment above it as the
// body. An entry whose name is empty means nothing was found there.
DocEntry documentationAt(const std::string& text, Language language, std::size_t offset);
// The comment block written directly above a 1-based line, markers stripped. ""
// when the lines above it are code rather than commentary.
std::string docCommentAbove(const std::string& text, int line);
// The comment a file opens with, which is how this repository introduces a file.
std::string leadingComment(const std::string& text);
// A page for a file: its leading comment, and the size and language it is read as.
DocEntry documentationForFile(const std::string& relativePath, const std::string& text);

// ---------------------------------------------------------------------------
// Completions
// ---------------------------------------------------------------------------

struct CompletionItem {
    std::string label;          // what is listed and matched
    std::string kind;           // "function", "field", "type", "keyword", "symbol", "word"
    std::string detail;         // one line shown beside the label
    std::string insertText;     // replaces the typed prefix
    std::string documentation;  // the summary shown under the list
};

// The suggestions for one caret position, already filtered by what was typed.
struct CompletionList {
    std::vector<CompletionItem> items;
    std::size_t tokenStart = 0;  // byte offset the typed prefix starts at
    std::string prefix;          // the prefix typed so far, possibly empty
    std::string qualifier;       // "workspace" in "workspace.Fi|"
    bool memberAccess = false;   // the caret follows a '.' or a ':'
    bool empty() const { return items.empty(); }
};
// What is worth offering at that offset: the members of the qualifier after a
// dot, otherwise the names this file uses, the reference and its keywords. A
// prefix match ranks above a match in the middle, which ranks above an
// abbreviation, and the list is capped so a large file cannot stall a frame.
CompletionList completionsAt(const std::string& text, Language language, std::size_t offset,
                             std::size_t maxItems = 200);

}  // namespace code
}  // namespace rblx
