#include "CodeProject.h"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace rblx::code;

namespace {

int failures = 0;

void check(bool condition, const std::string& name) {
    if (condition) {
        std::printf("ok   %s\n", name.c_str());
    } else {
        ++failures;
        std::printf("FAIL %s\n", name.c_str());
    }
}

bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

void writeFile(const fs::path& path, const std::string& text) {
    fs::create_directories(path.parent_path());
    std::ofstream out(path.string().c_str(), std::ios::binary | std::ios::trunc);
    out << text;
}

const FileEntry* findEntry(const std::vector<FileEntry>& entries, const std::string& relative) {
    for (const FileEntry& entry : entries) {
        if (entry.relativePath == relative) {
            return &entry;
        }
    }
    return nullptr;
}

const TreeNode* findNode(const TreeNode& node, const std::string& relative) {
    for (const TreeNode& child : node.children) {
        if (child.entry.relativePath == relative) {
            return &child;
        }
        if (const TreeNode* deeper = findNode(child, relative)) {
            return deeper;
        }
    }
    return nullptr;
}

fs::path makeProject() {
    const fs::path root = fs::temp_directory_path() /
                          ("rblx-code-editor-" + std::to_string(
                              std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    writeFile(root / "src" / "main.cpp", "int main() { return 0; }\n");
    writeFile(root / "src" / "ui" / "ClientUi.cpp", "// ui\nint draw() { return 1; }\n");
    writeFile(root / "include" / "Part.h", "#pragma once\n");
    writeFile(root / "web" / "public" / "app.js", "const ready = true;\n");
    writeFile(root / "shaders" / "vertex.glsl", "void main() {}\n");
    writeFile(root / "games" / "doors-hotel.rbx", "-- hotel\nprint(1)\n");
    writeFile(root / "README.md", "# Project\n");
    writeFile(root / "CMakeLists.txt", "project(Test)\n");
    writeFile(root / "build" / "generated.cpp", "// never shown\n");
    writeFile(root / ".git" / "config", "[core]\n");
    writeFile(root / "node_modules" / "pkg" / "index.js", "module.exports = 1;\n");
    writeFile(root / "textures" / "face.png", "\x89PNG");
    fs::create_directories(root / "empty");
    return root;
}

void testLanguages() {
    check(languageForPath("src/main.cpp") == Language::Cpp, "a .cpp file is C++");
    check(languageForPath("include/Part.h") == Language::Cpp, "a header is C++");
    check(languageForPath("shaders/fragment.glsl") == Language::Glsl, "a shader is GLSL");
    check(languageForPath("shaders/shadow.vert") == Language::Glsl, "a vertex shader is GLSL");
    check(languageForPath("games/doors-hotel.rbx") == Language::Lua,
          "an .rbx script is coloured as RBLX Script, which is Lua-shaped");
    check(languageForPath("web/public/app.js") == Language::JavaScript, "a script is JavaScript");
    check(languageForPath("web/public/index.html") == Language::Html, "a page is HTML");
    check(languageForPath("web/public/style.css") == Language::Css, "a stylesheet is CSS");
    check(languageForPath("README.md") == Language::Markdown, "a readme is Markdown");
    check(languageForPath("web/data.json") == Language::Json, "a data file is JSON");
    check(languageForPath("textures/face.png") == Language::Plain, "an image is plain text");
    check(std::string(languageName(Language::Cpp)) == "C++", "C++ names itself");
    check(std::string(languageName(Language::Lua)) == "RBLX script", "RBLX Script names itself");
}

void testFilters() {
    check(isIgnoredDirectory("build"), "build is skipped");
    check(isIgnoredDirectory("GameRelease"), "a packaged build is skipped whatever its case");
    check(isIgnoredDirectory(".git"), "the git directory is skipped");
    check(isIgnoredDirectory("node_modules"), "node_modules is skipped");
    check(isIgnoredDirectory("vendor"), "a vendored dependency is skipped");
    check(!isIgnoredDirectory("src"), "the source directory is not skipped");
    check(!isIgnoredDirectory("include"), "an include directory is not skipped");

    check(isEditableFile("main.cpp"), "a source file is editable");
    check(isEditableFile("ClientShared.h"), "a header is editable");
    check(isEditableFile("CMakeLists.txt"), "CMakeLists.txt is editable without an extension");
    check(isEditableFile("doors-hotel.rbx"), "a script is editable");
    check(isEditableFile("ServerWorld.world"), "a world file is editable");
    check(!isEditableFile("face.png"), "an image is not offered");
    check(!isEditableFile("model.glb"), "a model is not offered");
}

void testScan(const fs::path& root) {
    const std::string path = root.string();
    const std::vector<FileEntry> entries = scanProject(path, 500);
    check(!entries.empty(), "the project scans");

    check(findEntry(entries, "src") != nullptr, "a directory is listed");
    check(findEntry(entries, "src") != nullptr && findEntry(entries, "src")->directory,
          "the directory is marked as one");
    check(findEntry(entries, "src/main.cpp") != nullptr, "a nested file is listed");
    check(findEntry(entries, "src/ui/ClientUi.cpp") != nullptr, "a deeper file is listed");
    check(findEntry(entries, "README.md") != nullptr, "a top-level file is listed");
    check(findEntry(entries, "CMakeLists.txt") != nullptr, "an extensionless project file is listed");
    check(findEntry(entries, "build") == nullptr, "the build directory is not walked");
    check(findEntry(entries, "build/generated.cpp") == nullptr, "a generated file is not listed");
    check(findEntry(entries, ".git") == nullptr, "the git directory is not walked");
    check(findEntry(entries, "node_modules/pkg/index.js") == nullptr,
          "a dependency is not walked");
    check(findEntry(entries, "textures/face.png") == nullptr, "an image is filtered out");

    const FileEntry* nested = findEntry(entries, "src/ui/ClientUi.cpp");
    check(nested != nullptr && nested->depth == 2, "the depth counts the folders above a file");
    check(nested != nullptr && nested->name == "ClientUi.cpp", "the entry keeps the file name");
    check(findEntry(entries, "src") != nullptr && findEntry(entries, "src")->depth == 0,
          "a top-level folder is at depth zero");

    // Directories come before files in the same folder, and each group is sorted,
    // which is what keeps the tree from reshuffling between frames.
    std::size_t firstFile = entries.size();
    for (std::size_t i = 0; i < entries.size(); ++i) {
        if (!entries[i].directory && entries[i].depth == 0) {
            firstFile = i;
            break;
        }
    }
    bool directoriesFirst = true;
    for (std::size_t i = 0; i < entries.size(); ++i) {
        if (entries[i].depth != 0) {
            continue;
        }
        if (i > firstFile && entries[i].directory) {
            directoriesFirst = false;
        }
    }
    check(directoriesFirst, "top-level folders are listed before files");

    const std::vector<FileEntry> capped = scanProject(path, 3);
    check(capped.size() == 3, "the entry cap stops the walk");

    check(scanProject((root / "does-not-exist").string()).empty(),
          "a missing folder scans as empty");
}

void testTree(const std::vector<FileEntry>& entries) {
    const TreeNode root = buildTree(entries);
    const TreeNode* src = findNode(root, "src");
    check(src != nullptr && src->entry.directory, "the tree keeps the folder");
    check(src != nullptr && findNode(*src, "src/main.cpp") != nullptr,
          "a file is a child of its folder");
    const TreeNode* ui = src != nullptr ? findNode(*src, "src/ui") : nullptr;
    check(ui != nullptr && findNode(*ui, "src/ui/ClientUi.cpp") != nullptr,
          "a nested folder keeps its own files");
    check(findNode(root, "src/ui/ClientUi.cpp") != nullptr,
          "a file two folders down is reachable from the root");
    check(findNode(root, "README.md") != nullptr, "a top-level file is in the tree");
}

void testLineEndings() {
    const std::string crlf = "one\r\ntwo\r\n";
    const SourceDocument document = decodeSource(crlf);
    check(document.crlf, "a CRLF file is remembered as CRLF");
    check(document.text == "one\ntwo\n", "a CRLF file is shown with plain newlines");
    check(encodeSource(document) == crlf, "saving puts the CRLF endings back");

    const std::string lf = "one\ntwo\n";
    const SourceDocument plain = decodeSource(lf);
    check(!plain.crlf, "an LF file stays LF");
    check(encodeSource(plain) == lf, "saving an LF file writes LF");

    const std::string bom = "\xEF\xBB\xBFone\ntwo\n";
    check(decodeSource(bom).text == "one\ntwo\n", "a byte-order mark is dropped");

    check(decodeSource("a\nb").text == "a\nb", "a file without a final newline is kept as it is");
}

void testFileIo(const fs::path& root) {
    const fs::path sourcePath = root / "include" / "Part.h";
    std::string text;
    std::string error;
    check(readSourceFile(sourcePath.string(), &text, &error), "a text file reads");
    check(text == "#pragma once\n", "the text is what the file holds");

    check(!readSourceFile((root / "missing.cpp").string(), &text, &error), "a missing file is refused");
    check(!error.empty(), "the refusal says why");

    const fs::path binaryPath = root / "blob.bin";
    writeFile(binaryPath, std::string("abc\0def", 7));
    check(!readSourceFile(binaryPath.string(), &text, &error), "a binary file is refused");
    check(contains(error, "binary"), "the binary refusal names the reason");

    const fs::path bigPath = root / "big.txt";
    writeFile(bigPath, std::string(4096, 'x'));
    check(!readSourceFile(bigPath.string(), &text, &error, 1000),
          "a file past the size limit is refused");
    check(contains(error, "past the"), "the size refusal names the limit");

    const fs::path savedPath = root / "saved.cpp";
    check(writeSourceFile(savedPath.string(), "int saved = 1;\n", &error), "a file writes");
    check(readSourceFile(savedPath.string(), &text, &error) && text == "int saved = 1;\n",
          "what was written is what reads back");
    check(fs::exists(savedPath), "the written file exists on disk");
    check(!writeSourceFile((root / "no-such-folder" / "x.cpp").string(), "x", &error),
          "writing into a missing folder reports a failure");
}

void testSearch(const fs::path& root, const std::vector<FileEntry>& entries) {
    const std::vector<SearchHit> hits = searchProject(root.string(), entries, "MAIN");
    check(!hits.empty(), "search is case-insensitive");
    bool foundMain = false;
    for (const SearchHit& hit : hits) {
        if (hit.relativePath == "src/main.cpp") {
            foundMain = true;
            check(hit.line == 1, "a hit reports the line it is on");
            check(contains(hit.text, "int main"), "a hit carries the line");
        }
    }
    check(foundMain, "search reaches a nested source file");

    const std::vector<SearchHit> uiHits = searchProject(root.string(), entries, "return 1");
    check(uiHits.size() == 1, "search reports one hit per matching line");
    check(!uiHits.empty() && uiHits.front().relativePath == "src/ui/ClientUi.cpp",
          "search names the file a hit is in");
    check(!uiHits.empty() && uiHits.front().line == 2, "the line number counts from one");

    check(searchProject(root.string(), entries, "").empty(), "an empty query finds nothing");
    check(searchProject(root.string(), entries, "zzz-not-here").empty(),
          "a missing word finds nothing");
    check(searchProject(root.string(), entries, "e", 2).size() == 2, "the hit cap is honoured");
}

void testOffsets() {
    const std::string text = "aa\nbbbb\ncc";
    int line = 0;
    int column = 0;
    lineAndColumn(text, 0, &line, &column);
    check(line == 1 && column == 1, "the first byte is line one, column one");
    lineAndColumn(text, 3, &line, &column);
    check(line == 2 && column == 1, "a line starts after the newline before it");
    lineAndColumn(text, 6, &line, &column);
    check(line == 2 && column == 4, "a column counts the bytes on the line");
    lineAndColumn(text, 8, &line, &column);
    check(line == 3 && column == 1, "the last line is counted");
    lineAndColumn(text, 100, &line, &column);
    check(line == 3 && column == 3, "an offset past the end stops at the end");

    check(offsetOfLine(text, 1) == 0, "line one starts at the beginning");
    check(offsetOfLine(text, 2) == 3, "line two starts after the first newline");
    check(offsetOfLine(text, 3) == 8, "line three starts after the second");
    check(offsetOfLine(text, 99) == text.size(), "a line past the end lands at the end");
    check(offsetOfLine(text, 0) == 0, "a line before the first lands at the start");

    check(lineCount("") == 1, "an empty file has one line");
    check(lineCount("a") == 1, "a file without a newline has one line");
    check(lineCount("a\n") == 2, "a trailing newline opens an empty last line");
    check(lineCount(text) == 3, "the line count matches the line numbers");
}

void testPaths() {
    check(fileNameOf("src/main.cpp") == "main.cpp", "a slash separates the file name");
    check(fileNameOf("src\\main.cpp") == "main.cpp", "a backslash separates it too");
    check(fileNameOf("main.cpp") == "main.cpp", "a bare name is its own file name");

    // A fixed root rather than the temporary directory: Windows hands back a path
    // that already ends in a backslash, so a temp-directory root would make these
    // assertions about the temporary directory's shape rather than about joining.
    const std::string root = "checkout";
    check(joinProjectPath(root, "src/main.cpp") == "checkout/src/main.cpp",
          "a project path is joined onto the root");
    check(joinProjectPath(root + "/", "src/main.cpp") == "checkout/src/main.cpp",
          "a root that already ends in a slash is not doubled");
    check(joinProjectPath(root, "src\\main.cpp") == "checkout/src/main.cpp",
          "a Windows path opens on a POSIX build");
    check(joinProjectPath("C:\\checkout\\", "src/main.cpp") == "C:\\checkout\\src/main.cpp",
          "a Windows root that ends in a backslash is not doubled either");
    check(joinProjectPath("", "src/main.cpp") == "src/main.cpp",
          "an empty root leaves the path alone");

    check(formatBytes(900) == "900 B", "small sizes are bytes");
    check(formatBytes(1536) == "1.5 KB", "kilobytes keep one decimal");
    check(formatBytes(2048) == "2 KB", "a whole number of kilobytes has no decimal");
    check(formatBytes(3 * 1024 * 1024) == "3 MB", "megabytes are shown for large files");
}

const OutlineSymbol* findSymbol(const std::vector<OutlineSymbol>& symbols, const std::string& name) {
    for (const OutlineSymbol& symbol : symbols) {
        if (symbol.name == name) {
            return &symbol;
        }
    }
    return nullptr;
}

bool hasMatchAt(const std::vector<TextMatch>& matches, int line, int column) {
    for (const TextMatch& match : matches) {
        if (match.line == line && match.column == column) {
            return true;
        }
    }
    return false;
}

void testFindReplace() {
    const std::string text = "int alpha = 1;\nint Beta = 2;\n// alpha beta\n";

    const std::vector<TextMatch> loose = findInText(text, "alpha");
    check(loose.size() == 2, "find ignores case by default");
    check(hasMatchAt(loose, 1, 5), "a match reports the line and column it starts at");
    check(hasMatchAt(loose, 3, 4), "a match on a later line reports that line");

    FindOptions exact;
    exact.caseSensitive = true;
    check(findInText(text, "alpha", exact).size() == 2,
          "a case-sensitive find keeps both spellings apart");
    check(findInText(text, "ALPHA", exact).empty(), "a case-sensitive find misses the other case");

    FindOptions whole;
    whole.wholeWord = true;
    check(findInText(text, "alpha", whole).size() == 2, "a whole word is found on its own");
    check(findInText(text, "alph", whole).empty(), "a prefix is not a whole word");
    check(findInText("alpha1 alpha", "alpha", whole).size() == 1,
          "a word glued to a digit is not a whole word");
    check(findInText("aaaa", "aa").size() == 2, "matches never overlap");
    check(findInText(text, "").empty(), "an empty needle matches nothing");
    check(findInText(text, "zzz").empty(), "a missing needle matches nothing");
    check(findInText(text, "a", FindOptions(), 3).size() == 3, "the match cap is honoured");

    TextMatch match;
    check(findNextInText(text, "alpha", exact, 0, true, &match) && match.line == 1,
          "find next starts at the caret");
    check(findNextInText(text, "alpha", exact, 6, true, &match) && match.line == 3,
          "find next skips the match under the caret");
    check(findNextInText(text, "alpha", exact, text.size(), true, &match) && match.line == 1,
          "find next wraps to the first match");
    check(findNextInText(text, "alpha", exact, 6, false, &match) && match.line == 1,
          "find previous walks back");
    check(findNextInText(text, "alpha", exact, 0, false, &match) && match.line == 3,
          "find previous wraps to the last match");
    check(!findNextInText(text, "zzz", exact, 0, true, &match), "find next reports a miss");

    std::string replaced = text;
    check(replaceAllInText(&replaced, "alpha", "omega") == 2,
          "replace all counts the matches it changed");
    check(contains(replaced, "int omega = 1;"), "replace all rewrites every match");
    check(!contains(replaced, "alpha"), "replace all leaves none behind");

    std::string cased = text;
    check(replaceAllInText(&cased, "Beta", "gamma", exact) == 1,
          "replace all follows the case rule");
    check(contains(cased, "int gamma = 2;"), "replace all keeps the rest of the line");
    check(replaceAllInText(&cased, "", "x") == 0, "an empty needle replaces nothing");
}

void testOutline() {
    const std::string cpp =
        "namespace rblx {\n"
        "class Part {\n"
        "public:\n"
        "    void draw() const;\n"
        "    bool save() {\n"
        "        return true;\n"
        "    }\n"
        "};\n"
        "struct Loader {\n"
        "};\n"
        "enum class Kind { A, B };\n"
        "// void hidden() {\n"
        "}\n"
        "}  // namespace\n";
    const std::vector<OutlineSymbol> symbols = outlineSymbols(cpp, Language::Cpp);

    check(!symbols.empty(), "a C++ file has an outline");
    const OutlineSymbol* name = findSymbol(symbols, "rblx");
    check(name != nullptr && name->kind == "namespace", "a namespace is in the outline");
    check(name != nullptr && name->line == 1, "a symbol knows its line");
    const OutlineSymbol* part = findSymbol(symbols, "Part");
    check(part != nullptr && part->kind == "class", "a class is in the outline");
    check(part != nullptr && part->depth == 1, "a class inside a namespace is indented");
    const OutlineSymbol* save = findSymbol(symbols, "save");
    check(save != nullptr && save->kind == "function", "a function body is in the outline");
    check(save != nullptr && save->line == 5, "the function's line is the line it starts on");
    check(findSymbol(symbols, "Loader") != nullptr, "a struct is in the outline");
    check(findSymbol(symbols, "Kind") != nullptr, "an enum is in the outline");
    check(findSymbol(symbols, "draw") == nullptr, "a declaration without a body is left out");
    check(findSymbol(symbols, "hidden") == nullptr, "a commented-out function is left out");

    const std::string lua =
        "-- script\n"
        "local function tick(dt)\n"
        "end\n"
        "function M:start()\n"
        "end\n"
        "M.onTouch = function(other)\n"
        "end\n";
    const std::vector<OutlineSymbol> script = outlineSymbols(lua, Language::Lua);
    check(findSymbol(script, "tick") != nullptr, "a local function is in the outline");
    check(findSymbol(script, "M:start") != nullptr, "a method keeps its table name");
    const OutlineSymbol* handler = findSymbol(script, "M.onTouch");
    check(handler != nullptr && handler->kind == "handler", "an assigned handler is in the outline");

    const std::string markdown =
        "# Title\n## Section\ntext\n### Deep\n```\n# not a heading\n```\n";
    const std::vector<OutlineSymbol> headings = outlineSymbols(markdown, Language::Markdown);
    check(headings.size() == 3, "headings are the markdown outline");
    const OutlineSymbol* deep = findSymbol(headings, "Deep");
    check(deep != nullptr && deep->depth == 2, "a deeper heading is indented further");
    check(findSymbol(headings, "not a heading") == nullptr,
          "a hash inside a code fence is not a heading");

    const std::string json = "{\n  \"name\": \"city\",\n  \"parts\": {\n    \"head\": 1\n  }\n}\n";
    const std::vector<OutlineSymbol> keys = outlineSymbols(json, Language::Json);
    check(findSymbol(keys, "name") != nullptr, "a JSON key is in the outline");
    const OutlineSymbol* head = findSymbol(keys, "head");
    check(head != nullptr && head->depth == 2, "a nested key is indented");

    const std::string js = "function boot() {\n}\nconst update = (dt) => {\n};\n";
    const std::vector<OutlineSymbol> jsSymbols = outlineSymbols(js, Language::JavaScript);
    check(findSymbol(jsSymbols, "boot") != nullptr, "a JavaScript function is in the outline");
    check(findSymbol(jsSymbols, "update") != nullptr, "an arrow function is in the outline");

    const std::string css = ".panel {\n}\n@media (max-width: 600px) {\n}\n";
    const std::vector<OutlineSymbol> selectors = outlineSymbols(css, Language::Css);
    check(findSymbol(selectors, ".panel") != nullptr, "a CSS rule is in the outline");
    const OutlineSymbol* media = findSymbol(selectors, "@media (max-width: 600px)");
    check(media != nullptr && media->kind == "at-rule", "an at-rule is named as one");

    check(outlineSymbols("just text\n", Language::Plain).empty(), "plain text has no outline");
}

void testEditingHelpers() {
    const std::string text = "void f() {\n    if (x) {\n        g(x);\n    }\n}\n";
    check(matchingBracket(text, text.find('{')) == text.rfind('}'), "a brace finds its partner");
    check(matchingBracket(text, text.find('(')) == text.find(')'),
          "a parenthesis finds its partner");
    check(matchingBracket(text, text.find(')')) == text.find('('),
          "a closing bracket finds the opener before it");
    check(matchingBracket(text, 0) == std::string::npos, "a letter has no bracket to pair with");

    const std::string commented = "{\n  // )\n}\n";
    check(matchingBracket(commented, 0) == 9, "a bracket inside a comment is ignored");
    const std::string quoted = "if (call(\")\")) {\n}\n";
    check(matchingBracket(quoted, 3) == 13, "a bracket inside a string is ignored");

    const std::string tabbed = "\tint x;";
    check(displayColumn(tabbed, 1, 4) == 4, "a tab counts as the columns it draws");
    check(byteOffsetOfColumn(tabbed, 4, 4) == 1, "a column maps back to the byte");
    check(byteOffsetOfColumn(tabbed, 0, 4) == 0, "column zero is the start of the line");
    check(displayColumn("abc", 3, 4) == 3, "plain characters count one column each");
    check(byteOffsetOfColumn("abc", 99, 4) == 3, "a column past the end stops at the end");

    std::size_t marker = 0;
    check(lineHasComment("    // note", "//", &marker) && marker == 4,
          "a commented line is recognised");
    check(!lineHasComment("int x = 1;", "//", &marker), "a plain line is not commented");
    check(!lineHasComment("// note", "", &marker), "a language with no line comment never matches");
    check(std::string(lineCommentPrefix(Language::Cpp)) == "//", "C++ comments with two slashes");
    check(std::string(lineCommentPrefix(Language::Lua)) == "--",
          "RBLX Script comments with two dashes");
    check(std::string(lineCommentPrefix(Language::Css)).empty(),
          "CSS has no line comment to toggle");

    check(fuzzyScore("src/CodeMain.cpp", "cmain") >= 0, "an abbreviation matches a path");
    check(fuzzyScore("src/CodeMain.cpp", "zzz") < 0, "a letter that is not there does not match");
    check(fuzzyScore("src/CodeMain.cpp", "") == 0, "an empty filter matches everything");
    check(fuzzyScore("main.cpp", "main") > fuzzyScore("domain.cpp", "main"),
          "a match at the start of a name ranks higher");
    check(fuzzyScore("ab", "abc") < 0, "a filter longer than the name does not match");
}

void testWorkspaceHelpers(const fs::path& root) {
    writeFile(root / ".git" / "HEAD", "ref: refs/heads/feature-editor\n");
    check(gitBranchOf(root.string()) == "feature-editor", "the branch is read from HEAD");

    writeFile(root / ".git" / "HEAD", "0123456789abcdef0123456789abcdef01234567\n");
    check(gitBranchOf(root.string()) == "0123456", "a detached HEAD shows the short hash");
    check(gitBranchOf((root / "no-checkout").string()).empty(),
          "a folder without a checkout has no branch");

    const fs::path stamped = root / "stamp.txt";
    writeFile(stamped, "one\n");
    const std::string before = fileStamp(stamped.string());
    check(!before.empty(), "a file has a stamp");
    writeFile(stamped, "one\ntwo\n");
    check(fileStamp(stamped.string()) != before, "the stamp changes when the file changes");
    check(fileStamp((root / "missing.txt").string()).empty(), "a missing file has no stamp");
}


void testDocumentation() {
    check(!referencePages(Language::Lua).empty(), "the script reference has pages");
    const DocEntry* printed = findReferencePage("print", Language::Lua);
    check(printed != nullptr && printed->kind == "function", "print has a page");
    check(printed != nullptr && !printed->body.empty(), "a page carries its text");
    const DocEntry* method = findReferencePage("workspace:FindFirstChild", Language::Lua);
    check(method != nullptr && method->name == "workspace.FindFirstChild",
          "a colon-qualified name finds the page filed under a dot");
    check(findReferencePage("no-such-name", Language::Lua) == nullptr,
          "a name the reference does not have has no page");
    check(!referencePages(Language::Cpp).empty(), "the C++ reference has pages");
    check(findReferencePage("constexpr", Language::Cpp) != nullptr, "a keyword has a page");
    check(findReferencePage("constexpr", Language::Lua) == nullptr,
          "a C++ keyword is not offered to a script");
    check(!editorPages().empty(), "the editor documents its own parts");

    const std::string text = "local plate = workspace:FindFirstChild(\"Spawn\")\n";
    const std::size_t at = text.find("FindFirstChild");
    const WordUnderCursor under = wordUnderCursor(text, at + 3);
    check(under.word == "FindFirstChild", "the word under an offset is found");
    check(under.qualifier == "workspace", "what qualifies it is found");
    check(under.chain == "workspace:FindFirstChild", "the whole chain is found");
    check(under.end - under.start == std::string("FindFirstChild").size(),
          "the word's extent is its own length");
    check(wordUnderCursor("x = 1", 2).empty(), "an operator has no word");

    const DocEntry hovered = documentationAt(text, Language::Lua, at + 3);
    check(hovered.name == "workspace.FindFirstChild", "hovering a method name documents it");
    check(!hovered.summary.empty(), "the page has a summary to show");
    check(documentationAt("x = 1\n", Language::Lua, 0).name.empty(),
          "a bare variable in the reference has no page");

    const std::string annotated =
        "// Moves the platform up and down.\n"
        "// Called once per frame by the loop.\n"
        "function bob(dt)\n"
        "    return dt\n"
        "end\n";
    const DocEntry local = documentationAt(annotated, Language::Lua, annotated.find("bob") + 1);
    check(local.name == "bob", "a function declared in the file has a page");
    check(contains(local.body, "Moves the platform up and down"),
          "the comment above the declaration is its body");
    check(contains(local.summary, "Moves the platform up and down."),
          "the summary is the first sentence of that comment");

    const std::string block =
        "/*\n"
        " * Counts the steps of the walk.\n"
        " */\n"
        "void step();\n";
    check(contains(docCommentAbove(block, 4), "Counts the steps"),
          "a block comment documents the line under it");
    check(docCommentAbove("int a;\nint b;\n", 2).empty(),
          "code above a line is not documentation");
    check(docCommentAbove("// note\n\n// second\nvoid f();\n", 4).find("second") !=
              std::string::npos,
          "only the block next to a declaration counts");

    const std::string file =
        "// The world loader reads .world files.\n"
        "// Nothing here needs a window.\n"
        "\n"
        "#include <string>\n";
    const std::string lead = leadingComment(file);
    check(contains(lead, "The world loader reads"), "a file's opening comment is read");
    check(!contains(lead, "#include"), "the comment stops at the first line of code");
    check(leadingComment("int a;\n").empty(), "a file without a comment has none");

    const DocEntry filePage = documentationForFile("src/WorldLoader.cpp", file);
    check(filePage.kind == "file", "a file has a page of its own");
    check(contains(filePage.summary, "The world loader reads .world files."),
          "the file's summary is its first line");
    check(contains(filePage.signature, "C++"), "the file page names the language it is read as");
    check(contains(filePage.signature, "lines"), "the file page says how long it is");
}

void testCompletions() {
    const std::string script = "local speed = 12\nprint(speed)\nworkspace:Fi";
    const CompletionList members = completionsAt(script, Language::Lua, script.size());
    check(members.memberAccess, "a caret after a colon is a member access");
    check(members.qualifier == "workspace", "the qualifier is the name before the colon");
    check(members.prefix == "Fi", "the prefix is what has been typed");
    check(members.tokenStart == script.size() - 2, "the prefix starts where the name starts");
    bool foundMember = false;
    bool leakedGlobal = false;
    for (const CompletionItem& item : members.items) {
        if (item.label == "FindFirstChild") {
            foundMember = true;
            check(!item.insertText.empty(), "a member carries what taking it inserts");
            check(!item.documentation.empty(), "a member carries its documentation");
        }
        if (item.label == "print") {
            leakedGlobal = true;
        }
    }
    check(foundMember, "the members of workspace are offered");
    check(!leakedGlobal, "a global is not offered as a member");

    const std::string typed = "pri";
    const CompletionList globals = completionsAt(typed, Language::Lua, typed.size());
    check(!globals.empty(), "a prefix has suggestions");
    check(globals.items.front().label == "print", "the name the prefix starts ranks first");
    check(globals.items.front().insertText == "print", "taking it inserts the whole name");

    const std::string file =
        "// Draws the panel.\n"
        "function drawPanel()\n"
        "    local width = 640\n"
        "end\n";
    const CompletionList everything = completionsAt(file, Language::Lua, 0);
    bool foundLocal = false;
    bool foundSymbol = false;
    for (const CompletionItem& item : everything.items) {
        if (item.label == "width") {
            foundLocal = true;
        }
        if (item.label == "drawPanel" && item.kind == "function") {
            foundSymbol = true;
            check(item.documentation.find("line 2") != std::string::npos,
                  "a symbol says where it is declared");
        }
    }
    check(foundLocal, "a name the file already uses is offered");
    check(foundSymbol, "a function the file declares is offered");

    const CompletionList keywords = completionsAt("con", Language::Cpp, 3);
    bool foundKeyword = false;
    for (const CompletionItem& item : keywords.items) {
        if (item.label == "const" || item.label == "constexpr") {
            foundKeyword = true;
        }
    }
    check(foundKeyword, "a C++ keyword is offered in a C++ file");

    const CompletionList nothing = completionsAt("zzzq", Language::Lua, 4);
    check(nothing.empty(), "letters that match nothing offer nothing");
    const CompletionList capped = completionsAt("a", Language::Cpp, 1, 5);
    check(capped.items.size() <= 5, "the list stops at its cap");
    check(completionsAt("", Language::Plain, 0).empty(), "plain text has nothing to suggest");
}

}  // namespace

int main() {
    const fs::path root = makeProject();

    testLanguages();
    testFilters();

    const std::vector<FileEntry> entries = scanProject(root.string(), 500);
    testScan(root);
    testTree(entries);
    testLineEndings();
    testFileIo(root);
    testSearch(root, entries);
    testOffsets();
    testPaths();
    testFindReplace();
    testOutline();
    testEditingHelpers();
    testWorkspaceHelpers(root);
    testDocumentation();
    testCompletions();

    std::error_code cleanupError;
    fs::remove_all(root, cleanupError);

    if (failures != 0) {
        std::printf("%d code editor check(s) failed.\n", failures);
        return 1;
    }
    std::printf("All code editor checks passed.\n");
    return 0;
}
