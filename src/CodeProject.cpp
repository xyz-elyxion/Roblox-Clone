#include "CodeProject.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>

namespace rblx {
namespace code {
namespace {

namespace fs = std::filesystem;

std::string lowered(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string extensionOf(const std::string& path) {
    const std::string name = fileNameOf(path);
    const std::size_t dot = name.find_last_of('.');
    if (dot == std::string::npos || dot == 0) {
        return std::string();
    }
    return lowered(name.substr(dot));
}

void setError(std::string* error, const std::string& message) {
    if (error != nullptr) {
        *error = message;
    }
}

}  // namespace

Language languageForPath(const std::string& path) {
    const std::string extension = extensionOf(path);
    if (extension == ".cpp" || extension == ".cc" || extension == ".cxx" || extension == ".c" ||
        extension == ".h" || extension == ".hpp" || extension == ".hh" || extension == ".hxx" ||
        extension == ".inl" || extension == ".ipp") {
        return Language::Cpp;
    }
    if (extension == ".glsl" || extension == ".vert" || extension == ".frag" ||
        extension == ".geom" || extension == ".comp" || extension == ".tesc" ||
        extension == ".tese") {
        return Language::Glsl;
    }
    // The engine's own script language, RBLX Script, is Lua-shaped, so the Lua
    // definition is the one that colours it correctly.
    if (extension == ".rbx" || extension == ".lua") {
        return Language::Lua;
    }
    if (extension == ".js" || extension == ".mjs" || extension == ".cjs") {
        return Language::JavaScript;
    }
    if (extension == ".html" || extension == ".htm") {
        return Language::Html;
    }
    if (extension == ".css") {
        return Language::Css;
    }
    if (extension == ".md" || extension == ".markdown") {
        return Language::Markdown;
    }
    if (extension == ".json") {
        return Language::Json;
    }
    return Language::Plain;
}

const char* languageName(Language language) {
    switch (language) {
        case Language::Cpp: return "C++";
        case Language::Glsl: return "GLSL";
        case Language::Lua: return "RBLX script";
        case Language::JavaScript: return "JavaScript";
        case Language::Html: return "HTML";
        case Language::Css: return "CSS";
        case Language::Markdown: return "Markdown";
        case Language::Json: return "JSON";
        case Language::Plain: break;
    }
    return "Text";
}

const char* lineCommentPrefix(Language language) {
    switch (language) {
        case Language::Cpp:
        case Language::Glsl:
        case Language::JavaScript:
            return "//";
        case Language::Lua:
            return "--";
        case Language::Html:
        case Language::Css:
        case Language::Markdown:
        case Language::Json:
        case Language::Plain:
            break;
    }
    // HTML, CSS, Markdown and JSON only have block comments here, and inserting
    // half a block would break the file, so those report that they have none.
    return "";
}

bool isIgnoredDirectory(const std::string& name) {
    const std::string key = lowered(name);
    // Generated trees and the vendored libraries. `vendor` is the site's pinned
    // three.js, `rblxwin` and the seven trees beside it are the C++ libraries this
    // project vendored, and `GameRelease` is a packaged build, so none of them is
    // source anybody edits here.
    static const char* const ignored[] = {
        ".git",      ".vs",        "build",       "node_modules", "vendor",
        "artifacts", "__pycache__", ".cache",     "dist",         "out",
        "cmake-build-debug",        "cmake-build-release",          ".idea",
        "rblxbuild-build-debug",    "rblxbuild-build-release",
        "gamerelease",              "data",                         "rblxwin",
        "rblxmath",                 "rblxui",                       "rblxgizmo",
        "rblxtextedit",             "rblxphysics",                  "rblxstb",
        "rblxgl"
    };
    for (const char* entry : ignored) {
        if (key == entry) {
            return true;
        }
    }
    return false;
}

bool isEditableFile(const std::string& name) {
    const std::string key = lowered(name);
    // Files the tree should show even though their name carries no extension.
    static const char* const named[] = {
        "cmakelists.txt", "rblxbuildlists.txt", "makefile", "dockerfile",
        ".gitignore", ".gitattributes",
        "license",        "readme",   ".clang-format"
    };
    for (const char* entry : named) {
        if (key == entry) {
            return true;
        }
    }

    static const char* const extensions[] = {
        ".cpp",  ".cc",   ".cxx",  ".c",    ".h",    ".hpp",  ".hh",   ".hxx",
        ".inl",  ".ipp",  ".glsl", ".vert", ".frag", ".geom", ".comp", ".tesc",
        ".tese", ".rbx",  ".lua",  ".js",   ".mjs",  ".cjs",  ".json", ".html",
        ".htm",  ".css",  ".md",   ".markdown", ".txt", ".ps1", ".bat", ".sh",
        ".cmake", ".rblxbuild", ".yml", ".yaml", ".toml", ".ini", ".world"
    };
    const std::string extension = extensionOf(name);
    for (const char* entry : extensions) {
        if (extension == entry) {
            return true;
        }
    }
    return false;
}

std::vector<FileEntry> scanProject(const std::string& root, std::size_t maxEntries) {
    std::vector<FileEntry> entries;
    if (root.empty() || maxEntries == 0) {
        return entries;
    }
    std::error_code rootError;
    if (!fs::is_directory(root, rootError)) {
        return entries;
    }

    // Depth first with each directory's children sorted, which is what makes the
    // tree stable and lets buildTree assume a parent always precedes its children.
    const auto walk = [&](const fs::path& directory, int depth, auto&& self) -> void {
        std::vector<fs::directory_entry> children;
        std::error_code iterateError;
        fs::directory_iterator it(directory, fs::directory_options::skip_permission_denied,
                                  iterateError);
        const fs::directory_iterator end;
        for (; it != end; it.increment(iterateError)) {
            if (iterateError) {
                break;
            }
            children.push_back(*it);
        }

        std::sort(children.begin(), children.end(),
                  [](const fs::directory_entry& left, const fs::directory_entry& right) {
                      std::error_code leftError;
                      std::error_code rightError;
                      const bool leftDirectory = left.is_directory(leftError);
                      const bool rightDirectory = right.is_directory(rightError);
                      if (leftDirectory != rightDirectory) {
                          return leftDirectory;
                      }
                      return lowered(left.path().filename().string()) <
                             lowered(right.path().filename().string());
                  });

        for (const fs::directory_entry& child : children) {
            if (entries.size() >= maxEntries) {
                return;
            }
            std::error_code typeError;
            const bool isDirectory = child.is_directory(typeError);
            const std::string name = child.path().filename().string();
            if (isDirectory && isIgnoredDirectory(name)) {
                continue;
            }
            if (!isDirectory && !isEditableFile(name)) {
                continue;
            }

            FileEntry entry;
            entry.relativePath = relativePathOf(root, child.path().string());
            entry.name = name;
            entry.directory = isDirectory;
            entry.depth = depth;
            entries.push_back(entry);
            if (isDirectory) {
                self(child.path(), depth + 1, self);
            }
        }
    };

    walk(fs::path(root), 0, walk);
    return entries;
}

TreeNode buildTree(const std::vector<FileEntry>& entries) {
    TreeNode root;
    root.entry.directory = true;
    // The index of the last entry added at each depth, so an entry at depth N is
    // appended to the children of the directory that owns that depth.
    std::vector<std::size_t> cursor;
    for (const FileEntry& entry : entries) {
        if (entry.depth < 0) {
            continue;
        }
        std::vector<TreeNode>* children = &root.children;
        for (int level = 0; level < entry.depth && level < static_cast<int>(cursor.size()); ++level) {
            if (cursor[level] >= children->size()) {
                break;
            }
            children = &(*children)[cursor[level]].children;
        }
        cursor.resize(static_cast<std::size_t>(entry.depth) + 1, 0);
        TreeNode node;
        node.entry = entry;
        children->push_back(std::move(node));
        cursor[static_cast<std::size_t>(entry.depth)] = children->size() - 1;
    }
    return root;
}

SourceDocument decodeSource(const std::string& bytes) {
    SourceDocument document;
    std::string text = bytes;

    // A UTF-8 byte-order mark is not part of the source; showing it would put an
    // invisible character before the first line and look like a stray glyph.
    if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xEF &&
        static_cast<unsigned char>(text[1]) == 0xBB &&
        static_cast<unsigned char>(text[2]) == 0xBF) {
        text.erase(0, 3);
    }

    if (text.find("\r\n") != std::string::npos) {
        document.crlf = true;
        std::string normalized;
        normalized.reserve(text.size());
        for (std::size_t i = 0; i < text.size(); ++i) {
            if (text[i] == '\r' && i + 1 < text.size() && text[i + 1] == '\n') {
                continue;
            }
            normalized.push_back(text[i]);
        }
        text = std::move(normalized);
    }

    document.text = std::move(text);
    return document;
}

std::string encodeSource(const SourceDocument& document) {
    if (!document.crlf) {
        return document.text;
    }
    std::string bytes;
    bytes.reserve(document.text.size() + document.text.size() / 16);
    for (const char c : document.text) {
        if (c == '\n') {
            bytes.push_back('\r');
        }
        bytes.push_back(c);
    }
    return bytes;
}

bool readSourceFile(const std::string& path, std::string* text, std::string* error,
                    std::size_t maxBytes) {
    if (text == nullptr) {
        setError(error, "There is nowhere to put the file.");
        return false;
    }
    std::error_code sizeError;
    const std::uintmax_t size = fs::file_size(path, sizeError);
    if (sizeError) {
        setError(error, "Could not read " + path + ".");
        return false;
    }
    if (size > maxBytes) {
        std::ostringstream message;
        message << path << " is " << formatBytes(static_cast<std::size_t>(size))
                << ", past the " << formatBytes(maxBytes) << " the editor opens.";
        setError(error, message.str());
        return false;
    }

    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        setError(error, "Could not read " + path + ".");
        return false;
    }
    std::string bytes((std::istreambuf_iterator<char>(stream)),
                      std::istreambuf_iterator<char>());
    if (bytes.find('\0') != std::string::npos) {
        setError(error, path + " looks binary, so the editor will not open it.");
        return false;
    }
    *text = std::move(bytes);
    return true;
}

bool writeSourceFile(const std::string& path, const std::string& text, std::string* error) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    if (!stream) {
        setError(error, "Could not write " + path + ".");
        return false;
    }
    stream.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!stream.good()) {
        setError(error, "Could not finish writing " + path + ".");
        return false;
    }
    return true;
}

std::vector<SearchHit> searchProject(const std::string& root,
                                     const std::vector<FileEntry>& entries,
                                     const std::string& query, std::size_t maxHits) {
    std::vector<SearchHit> hits;
    const std::string needle = lowered(query);
    if (needle.empty() || maxHits == 0) {
        return hits;
    }

    for (const FileEntry& entry : entries) {
        if (entry.directory) {
            continue;
        }
        if (hits.size() >= maxHits) {
            break;
        }
        std::string bytes;
        std::string error;
        if (!readSourceFile(joinProjectPath(root, entry.relativePath), &bytes, &error)) {
            continue;
        }
        const SourceDocument document = decodeSource(bytes);
        std::size_t start = 0;
        int line = 1;
        while (start <= document.text.size()) {
            const std::size_t end = document.text.find('\n', start);
            const std::string lineText = document.text.substr(
                start, end == std::string::npos ? std::string::npos : end - start);
            if (lowered(lineText).find(needle) != std::string::npos) {
                SearchHit hit;
                hit.relativePath = entry.relativePath;
                hit.line = line;
                hit.text = lineText;
                while (!hit.text.empty() &&
                       (hit.text.back() == ' ' || hit.text.back() == '\t' ||
                        hit.text.back() == '\r')) {
                    hit.text.pop_back();
                }
                hits.push_back(std::move(hit));
                if (hits.size() >= maxHits) {
                    break;
                }
            }
            if (end == std::string::npos) {
                break;
            }
            start = end + 1;
            ++line;
        }
    }
    return hits;
}

void lineAndColumn(const std::string& text, std::size_t offset, int* line, int* column) {
    offset = std::min(offset, text.size());
    int currentLine = 1;
    int currentColumn = 1;
    for (std::size_t i = 0; i < offset; ++i) {
        if (text[i] == '\n') {
            ++currentLine;
            currentColumn = 1;
        } else {
            ++currentColumn;
        }
    }
    if (line != nullptr) {
        *line = currentLine;
    }
    if (column != nullptr) {
        *column = currentColumn;
    }
}

std::size_t offsetOfLine(const std::string& text, int line) {
    if (line <= 1) {
        return 0;
    }
    int remaining = line - 1;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') {
            if (--remaining == 0) {
                return i + 1;
            }
        }
    }
    // Past the end the last line is the closest thing to the line that was asked
    // for, which is where a Go to Line should leave the cursor.
    return text.size();
}

int lineCount(const std::string& text) {
    if (text.empty()) {
        return 1;
    }
    int lines = 1;
    for (const char c : text) {
        if (c == '\n') {
            ++lines;
        }
    }
    return lines;
}

std::string fileNameOf(const std::string& path) {
    const std::size_t slash = path.find_last_of("/\\");
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

std::string relativePathOf(const std::string& root, const std::string& path) {
    std::error_code error;
    const fs::path relative = fs::relative(fs::path(path), fs::path(root), error);
    if (error || relative.empty()) {
        return path;
    }
    std::string text = relative.generic_string();
    // A file the walk found beside the root has nothing to strip; a path that
    // walked upwards is not something the tree shows, so keep it as it came.
    if (text.compare(0, 3, "../") == 0) {
        return path;
    }
    return text;
}

std::string formatBytes(std::size_t bytes) {
    const char* unit = "B";
    double value = static_cast<double>(bytes);
    int decimals = 0;
    if (bytes >= 1024 * 1024) {
        value = static_cast<double>(bytes) / (1024.0 * 1024.0);
        unit = "MB";
        decimals = 1;
    } else if (bytes >= 1024) {
        value = static_cast<double>(bytes) / 1024.0;
        unit = "KB";
        decimals = 1;
    }
    std::ostringstream out;
    out.setf(std::ios::fixed);
    out.precision(decimals);
    out << value;
    std::string number = out.str();
    // One decimal is enough beside a filename; a whole number of kilobytes reads
    // better as "2 KB" than as "2.0 KB".
    if (number.size() > 2 && number.compare(number.size() - 2, 2, ".0") == 0) {
        number.resize(number.size() - 2);
    }
    return number + " " + unit;
}

std::string joinProjectPath(const std::string& root, const std::string& relativePath) {
    if (root.empty()) {
        return relativePath;
    }
    std::string relative = relativePath;
    std::replace(relative.begin(), relative.end(), '\\', '/');
    if (relative.empty()) {
        return root;
    }
    const bool needsSeparator = root.back() != '/' && root.back() != '\\';
    return root + (needsSeparator ? "/" : "") + relative;
}

// ---------------------------------------------------------------------------
// Find and replace
// ---------------------------------------------------------------------------

std::vector<TextMatch> findInText(const std::string& text, const std::string& needle,
                                  const FindOptions& options, std::size_t maxMatches) {
    std::vector<TextMatch> matches;
    if (needle.empty() || maxMatches == 0 || needle.size() > text.size()) {
        return matches;
    }

    const auto isWord = [](unsigned char c) { return std::isalnum(c) != 0 || c == '_'; };
    const auto sameAt = [&](std::size_t at) {
        for (std::size_t i = 0; i < needle.size(); ++i) {
            unsigned char left = static_cast<unsigned char>(text[at + i]);
            unsigned char right = static_cast<unsigned char>(needle[i]);
            if (!options.caseSensitive) {
                left = static_cast<unsigned char>(std::tolower(left));
                right = static_cast<unsigned char>(std::tolower(right));
            }
            if (left != right) {
                return false;
            }
        }
        return true;
    };

    int line = 1;
    int column = 1;
    std::size_t at = 0;
    while (at + needle.size() <= text.size()) {
        bool hit = sameAt(at);
        if (hit && options.wholeWord) {
            if (at > 0 && isWord(static_cast<unsigned char>(text[at - 1]))) {
                hit = false;
            }
            if (hit && at + needle.size() < text.size() &&
                isWord(static_cast<unsigned char>(text[at + needle.size()]))) {
                hit = false;
            }
        }
        if (hit) {
            TextMatch match;
            match.offset = at;
            match.length = needle.size();
            match.line = line;
            match.column = column;
            matches.push_back(match);
            if (matches.size() >= maxMatches) {
                break;
            }
            // Walking past the whole match is what keeps hits from overlapping,
            // so "aa" in "aaaa" is two matches and a replace-all terminates.
            for (std::size_t i = 0; i < needle.size(); ++i) {
                if (text[at + i] == '\n') {
                    ++line;
                    column = 1;
                } else {
                    ++column;
                }
            }
            at += needle.size();
            continue;
        }
        if (text[at] == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
        ++at;
    }
    return matches;
}

bool findNextInText(const std::string& text, const std::string& needle,
                    const FindOptions& options, std::size_t fromOffset, bool forward,
                    TextMatch* match) {
    const std::vector<TextMatch> matches = findInText(text, needle, options);
    if (matches.empty()) {
        return false;
    }

    const TextMatch* chosen = nullptr;
    if (forward) {
        for (const TextMatch& candidate : matches) {
            if (candidate.offset >= fromOffset) {
                chosen = &candidate;
                break;
            }
        }
        if (chosen == nullptr) {
            chosen = &matches.front();
        }
    } else {
        for (auto it = matches.rbegin(); it != matches.rend(); ++it) {
            if (it->offset < fromOffset) {
                chosen = &(*it);
                break;
            }
        }
        if (chosen == nullptr) {
            chosen = &matches.back();
        }
    }
    if (match != nullptr) {
        *match = *chosen;
    }
    return true;
}

std::size_t replaceAllInText(std::string* text, const std::string& needle,
                             const std::string& replacement, const FindOptions& options) {
    if (text == nullptr) {
        return 0;
    }
    const std::vector<TextMatch> matches = findInText(*text, needle, options);
    if (matches.empty()) {
        return 0;
    }

    std::string out;
    out.reserve(text->size());
    std::size_t copied = 0;
    for (const TextMatch& match : matches) {
        out.append(*text, copied, match.offset - copied);
        out += replacement;
        copied = match.offset + match.length;
    }
    out.append(*text, copied, std::string::npos);
    *text = std::move(out);
    return matches.size();
}

// ---------------------------------------------------------------------------
// Outline
// ---------------------------------------------------------------------------

namespace {

std::string trimCopy(const std::string& value) {
    std::size_t begin = 0;
    std::size_t end = value.size();
    while (begin < end && (value[begin] == ' ' || value[begin] == '\t' || value[begin] == '\r')) {
        ++begin;
    }
    while (end > begin &&
           (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r')) {
        --end;
    }
    return value.substr(begin, end - begin);
}

std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::size_t start = 0;
    while (true) {
        const std::size_t end = text.find('\n', start);
        if (end == std::string::npos) {
            lines.push_back(text.substr(start));
            break;
        }
        lines.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    return lines;
}

bool isIdentifierChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

bool startsWithWordAt(const std::string& line, std::size_t at, const char* word) {
    const std::size_t length = std::strlen(word);
    if (at >= line.size() || line.compare(at, length, word) != 0) {
        return false;
    }
    const std::size_t after = at + length;
    return after >= line.size() || !isIdentifierChar(line[after]);
}

std::string firstWordAfter(const std::string& line, std::size_t at) {
    while (at < line.size() && (line[at] == ' ' || line[at] == '\t')) {
        ++at;
    }
    const std::size_t begin = at;
    while (at < line.size() && isIdentifierChar(line[at])) {
        ++at;
    }
    return line.substr(begin, at - begin);
}

// RBLX Script and JavaScript name their functions the way they are called:
// `M.shop:open`, not just `open`.
std::string qualifiedNameAfter(const std::string& line, std::size_t at) {
    while (at < line.size() && (line[at] == ' ' || line[at] == '\t')) {
        ++at;
    }
    const std::size_t begin = at;
    while (at < line.size() &&
           (isIdentifierChar(line[at]) || line[at] == '.' || line[at] == ':')) {
        ++at;
    }
    return line.substr(begin, at - begin);
}

// The code on a line with comments and string literals blanked out, so a symbol
// scan is not fooled by `// void gone() {` or by a brace inside a string. The
// block-comment state is carried between lines by the caller.
std::string codeOnly(const std::string& line, bool* inBlock) {
    std::string out;
    out.reserve(line.size());
    for (std::size_t i = 0; i < line.size(); ++i) {
        if (*inBlock) {
            if (line[i] == '*' && i + 1 < line.size() && line[i + 1] == '/') {
                *inBlock = false;
                ++i;
            }
            continue;
        }
        if (line[i] == '/' && i + 1 < line.size() && line[i + 1] == '/') {
            break;
        }
        if (line[i] == '/' && i + 1 < line.size() && line[i + 1] == '*') {
            *inBlock = true;
            ++i;
            continue;
        }
        if (line[i] == '"' || line[i] == '\'') {
            const char quote = line[i];
            out.push_back(' ');
            for (++i; i < line.size(); ++i) {
                if (line[i] == '\\') {
                    ++i;
                    continue;
                }
                if (line[i] == quote) {
                    break;
                }
            }
            continue;
        }
        out.push_back(line[i]);
    }
    return out;
}

std::string nameBeforeParen(const std::string& line, std::size_t paren) {
    std::size_t end = paren;
    while (end > 0 && (line[end - 1] == ' ' || line[end - 1] == '\t')) {
        --end;
    }
    if (end == 0) {
        return std::string();
    }
    if (isIdentifierChar(line[end - 1])) {
        std::size_t begin = end;
        while (begin > 0 && isIdentifierChar(line[begin - 1])) {
            --begin;
        }
        return line.substr(begin, end - begin);
    }
    // `operator==(` and the like: the name is the punctuation before the paren.
    std::size_t begin = end;
    while (begin > 0 && line[begin - 1] != ' ' && line[begin - 1] != '\t') {
        --begin;
    }
    return line.substr(begin, end - begin);
}

bool isControlWord(const std::string& word) {
    static const char* const words[] = {
        "if",     "else",     "for",       "while",     "switch", "return",
        "catch",  "do",       "sizeof",    "new",       "delete", "using",
        "typedef", "template", "static_assert", "namespace", "case",  "default",
        "decltype", "assert",  "throw",     "goto",      "try",    "defined"
    };
    for (const char* entry : words) {
        if (word == entry) {
            return true;
        }
    }
    return false;
}

std::string shorten(const std::string& line) {
    const std::string trimmed = trimCopy(line);
    if (trimmed.size() <= 96) {
        return trimmed;
    }
    return trimmed.substr(0, 93) + "...";
}

void addSymbol(std::vector<OutlineSymbol>* out, const std::string& kind, const std::string& name,
               const std::string& detail, int line, int depth) {
    if (name.empty()) {
        return;
    }
    OutlineSymbol symbol;
    symbol.kind = kind;
    symbol.name = name;
    symbol.detail = detail;
    symbol.line = line;
    symbol.depth = std::max(0, depth);
    out->push_back(std::move(symbol));
}

void outlineBraces(const std::string& text, std::vector<OutlineSymbol>* out) {
    const std::vector<std::string> lines = splitLines(text);
    bool inBlock = false;
    int depth = 0;
    for (std::size_t index = 0; index < lines.size(); ++index) {
        const std::string code = codeOnly(lines[index], &inBlock);
        const std::string trimmed = trimCopy(code);
        if (!trimmed.empty() && trimmed[0] != '#' && trimmed[0] != '{' && trimmed[0] != '}' &&
            trimmed[0] != ')' && trimmed[0] != ';') {
            std::string kind;
            std::string name;
            if (startsWithWordAt(trimmed, 0, "namespace")) {
                kind = "namespace";
                name = firstWordAfter(trimmed, 9);
            } else if (startsWithWordAt(trimmed, 0, "class")) {
                kind = "class";
                name = firstWordAfter(trimmed, 5);
            } else if (startsWithWordAt(trimmed, 0, "struct")) {
                kind = "struct";
                name = firstWordAfter(trimmed, 6);
            } else if (startsWithWordAt(trimmed, 0, "union")) {
                kind = "union";
                name = firstWordAfter(trimmed, 5);
            } else if (startsWithWordAt(trimmed, 0, "enum")) {
                kind = "enum";
                name = firstWordAfter(trimmed, 4);
                if (name == "class" || name == "struct") {
                    name = firstWordAfter(trimmed, trimmed.find(name) + name.size());
                }
            } else {
                const std::size_t paren = trimmed.find('(');
                const bool hasParens =
                    paren != std::string::npos && trimmed.find(')', paren) != std::string::npos;
                const char last = trimmed.back();
                // A line that ends in a semicolon or a comma is a declaration or a
                // parameter list, not a body to jump into.
                if (hasParens && last != ';' && last != ',' && last != '=' &&
                    !isControlWord(firstWordAfter(trimmed, 0))) {
                    kind = "function";
                    name = nameBeforeParen(trimmed, paren);
                }
            }
            addSymbol(out, kind, name, shorten(trimmed), static_cast<int>(index) + 1, depth);
        }
        for (const char c : code) {
            if (c == '{') {
                ++depth;
            } else if (c == '}') {
                depth = std::max(0, depth - 1);
            }
        }
    }
}

void outlineScript(const std::string& text, std::vector<OutlineSymbol>* out) {
    const std::vector<std::string> lines = splitLines(text);
    bool inBlock = false;
    int depth = 0;
    for (std::size_t index = 0; index < lines.size(); ++index) {
        const std::string code = codeOnly(lines[index], &inBlock);
        const std::string trimmed = trimCopy(code);
        if (!trimmed.empty()) {
            std::string kind;
            std::string name;
            if (startsWithWordAt(trimmed, 0, "function")) {
                kind = "function";
                name = qualifiedNameAfter(trimmed, std::strlen("function"));
            } else if (startsWithWordAt(trimmed, 0, "class")) {
                kind = "class";
                name = firstWordAfter(trimmed, 5);
            } else if (startsWithWordAt(trimmed, 0, "const") || startsWithWordAt(trimmed, 0, "let") ||
                       startsWithWordAt(trimmed, 0, "var")) {
                const std::size_t space = trimmed.find(' ');
                if (space != std::string::npos &&
                    (trimmed.find("=>", space) != std::string::npos ||
                     trimmed.find("= function", space) != std::string::npos)) {
                    kind = "function";
                    name = firstWordAfter(trimmed, space);
                }
            }
            addSymbol(out, kind, name, shorten(trimmed), static_cast<int>(index) + 1, depth);
        }
        for (const char c : code) {
            if (c == '{') {
                ++depth;
            } else if (c == '}') {
                depth = std::max(0, depth - 1);
            }
        }
    }
}

void outlineStyle(const std::string& text, std::vector<OutlineSymbol>* out) {
    const std::vector<std::string> lines = splitLines(text);
    bool inBlock = false;
    int depth = 0;
    for (std::size_t index = 0; index < lines.size(); ++index) {
        const std::string code = codeOnly(lines[index], &inBlock);
        const std::string trimmed = trimCopy(code);
        if (!trimmed.empty() && trimmed.back() == '{' && trimmed[0] != '}') {
            const std::string selector = trimCopy(trimmed.substr(0, trimmed.size() - 1));
            addSymbol(out, selector.empty() || selector[0] != '@' ? "rule" : "at-rule", selector,
                      shorten(trimmed), static_cast<int>(index) + 1, depth);
        }
        for (const char c : code) {
            if (c == '{') {
                ++depth;
            } else if (c == '}') {
                depth = std::max(0, depth - 1);
            }
        }
    }
}

void outlineLua(const std::string& text, std::vector<OutlineSymbol>* out) {
    const std::vector<std::string> lines = splitLines(text);
    for (std::size_t index = 0; index < lines.size(); ++index) {
        std::string line = lines[index];
        const std::size_t comment = line.find("--");
        if (comment != std::string::npos) {
            line.erase(comment);
        }
        const std::string trimmed = trimCopy(line);
        if (trimmed.empty()) {
            continue;
        }
        std::string kind;
        std::string name;
        std::size_t localFunction = std::string::npos;
        if (startsWithWordAt(trimmed, 0, "local")) {
            const std::size_t space = trimmed.find(' ');
            if (space != std::string::npos) {
                std::size_t at = space;
                while (at < trimmed.size() && trimmed[at] == ' ') {
                    ++at;
                }
                if (startsWithWordAt(trimmed, at, "function")) {
                    localFunction = at + std::strlen("function");
                }
            }
        }
        if (localFunction != std::string::npos) {
            kind = "function";
            name = qualifiedNameAfter(trimmed, localFunction);
        } else if (startsWithWordAt(trimmed, 0, "function")) {
            kind = "function";
            name = qualifiedNameAfter(trimmed, std::strlen("function"));
        } else {
            // `M.onTouch = function(other)` and `local handler = function()`
            const std::size_t assign = trimmed.find("= function");
            if (assign != std::string::npos) {
                std::string left = trimCopy(trimmed.substr(0, assign));
                if (left.compare(0, 6, "local ") == 0) {
                    left = trimCopy(left.substr(6));
                }
                kind = "handler";
                name = left;
            }
        }
        addSymbol(out, kind, name, shorten(trimmed), static_cast<int>(index) + 1, 0);
    }
}

void outlineMarkdown(const std::string& text, std::vector<OutlineSymbol>* out) {
    const std::vector<std::string> lines = splitLines(text);
    bool inFence = false;
    for (std::size_t index = 0; index < lines.size(); ++index) {
        const std::string trimmed = trimCopy(lines[index]);
        if (trimmed.compare(0, 3, "```") == 0) {
            inFence = !inFence;
            continue;
        }
        if (inFence || trimmed.empty() || trimmed[0] != '#') {
            continue;
        }
        std::size_t hashes = 0;
        while (hashes < trimmed.size() && trimmed[hashes] == '#') {
            ++hashes;
        }
        if (hashes > 6 || (hashes < trimmed.size() && trimmed[hashes] != ' ')) {
            continue;
        }
        addSymbol(out, "heading", trimCopy(trimmed.substr(hashes)), shorten(trimmed),
                  static_cast<int>(index) + 1, static_cast<int>(hashes) - 1);
    }
}

void outlineJson(const std::string& text, std::vector<OutlineSymbol>* out) {
    const std::vector<std::string> lines = splitLines(text);
    int depth = 0;
    for (std::size_t index = 0; index < lines.size(); ++index) {
        const std::string trimmed = trimCopy(lines[index]);
        if (trimmed.size() >= 3 && trimmed[0] == '"') {
            const std::size_t close = trimmed.find('"', 1);
            if (close != std::string::npos) {
                std::size_t after = close + 1;
                while (after < trimmed.size() && trimmed[after] == ' ') {
                    ++after;
                }
                if (after < trimmed.size() && trimmed[after] == ':') {
                    addSymbol(out, "key", trimmed.substr(1, close - 1), shorten(trimmed),
                              static_cast<int>(index) + 1, depth);
                }
            }
        }
        for (const char c : trimmed) {
            if (c == '{') {
                ++depth;
            } else if (c == '}') {
                depth = std::max(0, depth - 1);
            }
        }
    }
}

}  // namespace

std::vector<OutlineSymbol> outlineSymbols(const std::string& text, Language language) {
    std::vector<OutlineSymbol> symbols;
    switch (language) {
        case Language::Cpp:
        case Language::Glsl:
            outlineBraces(text, &symbols);
            break;
        case Language::JavaScript:
            outlineScript(text, &symbols);
            break;
        case Language::Css:
            outlineStyle(text, &symbols);
            break;
        case Language::Lua:
            outlineLua(text, &symbols);
            break;
        case Language::Markdown:
            outlineMarkdown(text, &symbols);
            break;
        case Language::Json:
            outlineJson(text, &symbols);
            break;
        case Language::Html:
        case Language::Plain:
            break;
    }
    return symbols;
}

// ---------------------------------------------------------------------------
// Editing helpers
// ---------------------------------------------------------------------------

std::size_t matchingBracket(const std::string& text, std::size_t offset) {
    static const char* const opens = "([{";
    static const char* const closes = ")]}";
    if (offset >= text.size()) {
        return std::string::npos;
    }

    const std::string openSet(opens);
    const std::string closeSet(closes);
    const std::size_t openAt = openSet.find(text[offset]);
    const std::size_t closeAt = closeSet.find(text[offset]);
    if (openAt == std::string::npos && closeAt == std::string::npos) {
        return std::string::npos;
    }

    const bool forward = openAt != std::string::npos;
    const char open = forward ? openSet[openAt] : openSet[closeAt];
    const char close = forward ? closeSet[openAt] : closeSet[closeAt];

    if (forward) {
        int depth = 0;
        char quote = '\0';
        for (std::size_t i = offset; i < text.size(); ++i) {
            const char c = text[i];
            if (quote != '\0') {
                if (c == '\\') {
                    ++i;
                    continue;
                }
                if (c == quote) {
                    quote = '\0';
                }
                continue;
            }
            if (c == '"' || c == '\'') {
                quote = c;
                continue;
            }
            if (c == '/' && i + 1 < text.size() && text[i + 1] == '/') {
                const std::size_t end = text.find('\n', i);
                if (end == std::string::npos) {
                    return std::string::npos;
                }
                i = end;
                continue;
            }
            if (c == '/' && i + 1 < text.size() && text[i + 1] == '*') {
                const std::size_t end = text.find("*/", i + 2);
                if (end == std::string::npos) {
                    return std::string::npos;
                }
                i = end + 1;
                continue;
            }
            if (c == open) {
                ++depth;
            } else if (c == close) {
                --depth;
                if (depth == 0) {
                    return i;
                }
            }
        }
        return std::string::npos;
    }

    // A closing bracket: walk forward with a stack so strings and comments are
    // skipped exactly as they are in the other direction.
    std::vector<std::size_t> stack;
    char quote = '\0';
    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (quote != '\0') {
            if (c == '\\') {
                ++i;
                continue;
            }
            if (c == quote) {
                quote = '\0';
            }
            continue;
        }
        if (c == '"' || c == '\'') {
            quote = c;
            continue;
        }
        if (c == '/' && i + 1 < text.size() && text[i + 1] == '/') {
            const std::size_t end = text.find('\n', i);
            if (end == std::string::npos) {
                return std::string::npos;
            }
            i = end;
            continue;
        }
        if (c == '/' && i + 1 < text.size() && text[i + 1] == '*') {
            const std::size_t end = text.find("*/", i + 2);
            if (end == std::string::npos) {
                return std::string::npos;
            }
            i = end + 1;
            continue;
        }
        if (i == offset) {
            if (stack.empty() || openSet.find(text[stack.back()]) != closeAt) {
                return std::string::npos;
            }
            return stack.back();
        }
        if (openSet.find(c) != std::string::npos) {
            stack.push_back(i);
        } else if (closeSet.find(c) != std::string::npos) {
            if (!stack.empty()) {
                stack.pop_back();
            }
        }
    }
    return std::string::npos;
}

int displayColumn(const std::string& line, std::size_t byteOffset, int tabSize) {
    if (tabSize < 1) {
        tabSize = 1;
    }
    int column = 0;
    for (std::size_t i = 0; i < byteOffset && i < line.size(); ++i) {
        if (line[i] == '\t') {
            column += tabSize - (column % tabSize);
        } else {
            ++column;
        }
    }
    return column;
}

std::size_t byteOffsetOfColumn(const std::string& line, int column, int tabSize) {
    if (column <= 0) {
        return 0;
    }
    if (tabSize < 1) {
        tabSize = 1;
    }
    int current = 0;
    std::size_t i = 0;
    while (i < line.size() && current < column) {
        if (line[i] == '\t') {
            current += tabSize - (current % tabSize);
        } else {
            ++current;
        }
        ++i;
    }
    return i;
}

bool lineHasComment(const std::string& line, const std::string& prefix,
                    std::size_t* markerOffset) {
    if (prefix.empty()) {
        return false;
    }
    std::size_t at = 0;
    while (at < line.size() && (line[at] == ' ' || line[at] == '\t')) {
        ++at;
    }
    if (line.compare(at, prefix.size(), prefix) != 0) {
        return false;
    }
    if (markerOffset != nullptr) {
        *markerOffset = at;
    }
    return true;
}

int fuzzyScore(const std::string& candidate, const std::string& query) {
    if (query.empty()) {
        return 0;
    }

    int score = 0;
    std::size_t at = 0;
    int lastMatch = -1;
    for (const char raw : query) {
        const unsigned char wanted = static_cast<unsigned char>(std::tolower(
            static_cast<unsigned char>(raw)));
        bool found = false;
        while (at < candidate.size()) {
            const unsigned char current = static_cast<unsigned char>(
                std::tolower(static_cast<unsigned char>(candidate[at])));
            if (current == wanted) {
                found = true;
                break;
            }
            ++at;
        }
        if (!found) {
            return -1;
        }

        // A letter that starts a word or a path segment is the one somebody
        // meant when they typed an abbreviation, so it is worth more than one
        // found in the middle of a word.
        const char previous = at == 0 ? '\0' : candidate[at - 1];
        const bool boundary = at == 0 || previous == '/' || previous == '\\' || previous == '_' ||
                              previous == '-' || previous == '.' || previous == ' ';
        const bool camel = std::islower(static_cast<unsigned char>(previous)) != 0 &&
                           std::isupper(static_cast<unsigned char>(candidate[at])) != 0;
        score += boundary ? 8 : (camel ? 6 : 2);
        if (lastMatch >= 0 && static_cast<int>(at) == lastMatch + 1) {
            score += 4;
        }
        lastMatch = static_cast<int>(at);
        ++at;
    }

    // Shorter names read as closer matches, and a shorter path is easier to aim at.
    score -= static_cast<int>(candidate.size() / 8);
    return score;
}

std::string gitBranchOf(const std::string& root) {
    if (root.empty()) {
        return std::string();
    }
    std::ifstream stream(fs::path(root) / ".git" / "HEAD", std::ios::binary);
    if (!stream) {
        return std::string();
    }
    std::string line((std::istreambuf_iterator<char>(stream)),
                     std::istreambuf_iterator<char>());
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r' || line.back() == ' ')) {
        line.pop_back();
    }

    const std::string heads = "ref: refs/heads/";
    if (line.compare(0, heads.size(), heads) == 0) {
        return line.substr(heads.size());
    }
    if (line.compare(0, 5, "ref: ") == 0) {
        const std::size_t slash = line.find_last_of('/');
        return slash == std::string::npos ? line.substr(5) : line.substr(slash + 1);
    }
    // A detached HEAD holds a commit hash; the short form is what a person reads.
    if (line.size() >= 7) {
        return line.substr(0, 7);
    }
    return std::string();
}

std::string fileStamp(const std::string& path) {
    std::error_code sizeError;
    const std::uintmax_t size = fs::file_size(path, sizeError);
    std::error_code timeError;
    const fs::file_time_type time = fs::last_write_time(path, timeError);
    if (sizeError || timeError) {
        return std::string();
    }
    // libc++ counts file times in an extended integer type that has no stream
    // inserter (and libstdc++ counts them in a plain 64-bit one), so the count
    // is narrowed to a fixed type before it is written out. Nanoseconds since
    // the epoch do not come close to the end of a 64-bit integer.
    const std::int64_t ticks = static_cast<std::int64_t>(time.time_since_epoch().count());
    std::ostringstream out;
    out << size << ':' << ticks;
    return out.str();
}

}  // namespace code
}  // namespace rblx
// ---------------------------------------------------------------------------
// Documentation
// ---------------------------------------------------------------------------

namespace rblx {
namespace code {
namespace {

DocEntry makePage(const std::string& name, const std::string& kind, const std::string& signature,
                  const std::string& summary, const std::string& body,
                  std::vector<std::string> seeAlso = std::vector<std::string>()) {
    DocEntry entry;
    entry.name = name;
    entry.kind = kind;
    entry.signature = signature;
    entry.summary = summary;
    entry.body = body.empty() ? summary : body;
    entry.seeAlso = std::move(seeAlso);
    return entry;
}

// A page whose one line is all there is to say about it: a keyword, a math
// helper, an HTML element. Writing those as rows keeps the tables readable.
struct ShortPage {
    const char* name;
    const char* signature;
    const char* summary;
};

template <std::size_t N>
std::vector<DocEntry> shortPages(const ShortPage (&pages)[N], const std::string& kind) {
    std::vector<DocEntry> out;
    out.reserve(N);
    for (std::size_t i = 0; i < N; ++i) {
        out.push_back(makePage(pages[i].name, kind, pages[i].signature, pages[i].summary, ""));
    }
    return out;
}

// RBLX Script: the globals and methods this build's interpreter and world
// runtime actually install, not the ones a Roblox script would expect to find.
std::vector<DocEntry> scriptPages() {
    std::vector<DocEntry> out;
    out.push_back(makePage(
        "print", "function", "print(...)",
        "Writes its arguments to the script output on one line.",
        "print joins the arguments with a space and sends the line to whatever is running the "
        "script -- the server log in this build. Numbers and strings print as written, a table "
        "prints as table, and print returns nothing.\n\nIt is the first thing to reach for when a "
        "script is not doing what it should: put a print above the line that is misbehaving.",
        {"warn"}));
    out.push_back(makePage(
        "warn", "function", "warn(...)",
        "Writes its arguments to the script output and marks the line as a warning.",
        "warn goes through the same handler as print but tells it the line is a warning, so a "
        "warning can be told apart from ordinary output without reading the text.\n\nUse it for "
        "the things that are wrong but survivable: a missing part, a value that fell back to a "
        "default.", {"print"}));
    out.push_back(makePage(
        "type", "function", "type(value)",
        "Names the kind of a value.",
        "Returns one of number, string, boolean, table, function, object or nil. This "
        "interpreter has no typeof operator, so type is how a script checks a value before using "
        "it -- most often before calling something on it.", {"tostring"}));
    out.push_back(makePage(
        "tostring", "function", "tostring(value)",
        "Formats any value as a string.",
        "Numbers keep their decimal part when they have one, booleans become true and false, nil "
        "becomes nil, and a table prints as table -- the same text print shows.\n\nIt is what a "
        "script uses to build a message out of a value whose type it does not know.",
        {"tonumber", "type"}));
    out.push_back(makePage(
        "tonumber", "function", "tonumber(value)",
        "Reads a number out of a value.",
        "A string that holds a number becomes that number and a number passes through unchanged. "
        "Anything else becomes nil, so check the result before doing arithmetic with it.",
        {"tostring"}));
    out.push_back(makePage(
        "error", "function", "error(message)",
        "Stops the script with a message.",
        "Raises an error at the call. The script stops there and the message reaches the server "
        "log, which is how a script says it cannot go on rather than carrying on with a value "
        "that is not there.", {"warn"}));
    out.push_back(makePage(
        "math", "type", "math",
        "The number helpers the interpreter provides.",
        "A table of functions and one constant, math.pi. floor, ceil, sqrt, sin, cos, tan and abs "
        "take one number; min and max take any number of them; random takes none for a value "
        "between 0 and 1, one for 0 up to it, or two for a range.",
        {"math.floor", "math.random"}));

    static const ShortPage mathHelpers[] = {
        {"math.pi", "math.pi", "Pi, the constant the trigonometric helpers work in."},
        {"math.abs", "math.abs(value)", "The value without its sign."},
        {"math.ceil", "math.ceil(value)", "The smallest whole number at or above the value."},
        {"math.floor", "math.floor(value)", "The largest whole number at or below the value."},
        {"math.sqrt", "math.sqrt(value)", "The square root of the value."},
        {"math.sin", "math.sin(radians)", "The sine of an angle in radians."},
        {"math.cos", "math.cos(radians)", "The cosine of an angle in radians."},
        {"math.tan", "math.tan(radians)", "The tangent of an angle in radians."},
        {"math.min", "math.min(a, b, ...)", "The smallest of its arguments."},
        {"math.max", "math.max(a, b, ...)", "The largest of its arguments."},
        {"math.random", "math.random() / (n) / (low, high)",
         "A random number: 0 to 1 with no arguments, 0 up to n with one, low to high with two."},
    };
    for (const DocEntry& helper : shortPages(mathHelpers, "function")) {
        out.push_back(helper);
    }

    out.push_back(makePage(
        "workspace", "type", "workspace",
        "The parts of the world the script runs in.",
        "workspace is the object a script uses to look at and change the world. It carries "
        "FindFirstChild, GetChildren and CreatePart, and every part it hands back carries Name, "
        "Position and the Teleport and Destroy methods.",
        {"workspace.FindFirstChild", "workspace.CreatePart"}));
    out.push_back(makePage(
        "workspace.FindFirstChild", "method", "workspace:FindFirstChild(name)",
        "The part with that name, or nil.",
        "Searches the parts of the world for one whose name matches exactly and returns it. There "
        "is no path syntax and no recursion: name a part directly.\n\n"
        "local plate = workspace:FindFirstChild(\"Spawn\")\n"
        "if plate then plate:Teleport(Vector3.new(0, 12, 0)) end\n\n"
        "Always check the result: a script that assumes a part exists fails on the world that "
        "does not have it.", {"workspace.GetChildren", "Teleport"}));
    out.push_back(makePage(
        "workspace.GetChildren", "method", "workspace:GetChildren()",
        "Every part in the world, as a table.",
        "Returns a table holding every part, in the order the world loader put them there. Walk "
        "it with a numeric for loop to touch all of them at once -- the usual way to find the "
        "parts that match a rule rather than a name.", {"workspace.FindFirstChild"}));
    out.push_back(makePage(
        "workspace.CreatePart", "method", "workspace:CreatePart(...)",
        "Adds a part to the world and returns it.",
        "Creates a part and hands it back so the script can name it, move it or destroy it "
        "afterwards. The fields it reads are the ones the workspace object in "
        "src/WorldScript.cpp implements.", {"workspace.GetChildren", "Destroy"}));
    out.push_back(makePage(
        "Vector3", "type", "Vector3",
        "The table that builds positions.",
        "Vector3 holds one function, new. What it returns is the position type the part fields "
        "and Teleport expect.", {"Vector3.new"}));
    out.push_back(makePage(
        "Vector3.new", "function", "Vector3.new(x, y, z)",
        "A position from three numbers.",
        "Missing arguments are zero, so Vector3.new() is the origin. The result is a vector value "
        "rather than a table: it prints as a vector and compares equal to another one made from "
        "the same numbers.\n\npart:Teleport(Vector3.new(0, 20, 0))",
        {"Vector3", "Teleport"}));
    out.push_back(makePage(
        "Players", "type", "Players",
        "The people connected right now.",
        "Players carries GetPlayers, which returns the connected players as a table. Each player "
        "carries Name and the TakeDamage and Teleport methods.", {"Players.GetPlayers"}));
    out.push_back(makePage(
        "Players.GetPlayers", "method", "Players:GetPlayers()",
        "The connected players, as a table.",
        "Returns one value per player who is connected at the moment of the call. The table is a "
        "snapshot: a player who leaves afterwards is still in it, so check before acting on one.",
        {"Players", "TakeDamage"}));
    out.push_back(makePage(
        "game", "type", "game",
        "The table that describes the running game.",
        "game carries Workspace, the same object as the workspace global, and Name, the name the "
        "world was loaded with.", {"game.Workspace", "game.Name"}));
    out.push_back(makePage(
        "game.Workspace", "field", "game.Workspace",
        "The world, reached the way a Roblox script reaches it.",
        "The same object as workspace. A script written against the Roblox shape can use either; "
        "this build has one world, so both names point at it.", {"workspace"}));
    out.push_back(makePage(
        "game.Name", "field", "game.Name",
        "The name the world was loaded with.",
        "A string, set when the runtime was built. It is handy in a print line when a script is "
        "shared between worlds.", {"game"}));
    out.push_back(makePage(
        "Teleport", "method", "value:Teleport(position)",
        "Moves a part or a player to a position.",
        "Takes a Vector3 and puts the thing there. A part is moved and its physics body put to "
        "rest; a player is moved through the teleport handler the server installs, so the client "
        "sees the move rather than walking to it.", {"Vector3.new"}));
    out.push_back(makePage(
        "Destroy", "method", "value:Destroy()",
        "Removes a part from the world.",
        "The part stops existing for scripts and for physics after the call, and any value that "
        "pointed at it stops being usable.", {"workspace.CreatePart"}));
    out.push_back(makePage(
        "TakeDamage", "method", "player:TakeDamage(amount)",
        "Takes health off a player.",
        "The amount is a number of health points. When it reaches zero the player is respawned by "
        "the server rather than by the script, so a script does not have to handle death itself.",
        {"Players.GetPlayers"}));
    return out;
}

std::vector<DocEntry> cppPages() {
    static const ShortPage keywords[] = {
        {"class", "class Name { };", "A type whose members are private by default."},
        {"struct", "struct Name { };", "A type whose members are public by default."},
        {"namespace", "namespace name { }", "A named scope, which is how this repository keeps parts of the program apart."},
        {"const", "const Type name = value;", "A value that cannot be changed after it is set."},
        {"constexpr", "constexpr Type name = value;", "A value worked out at compile time."},
        {"static", "static Type name;", "Storage for the whole program in a class, and internal linkage at file scope."},
        {"inline", "inline Type name() { }", "A definition that may appear in more than one translation unit."},
        {"explicit", "explicit Name(args);", "A constructor that will not be used for an implicit conversion."},
        {"virtual", "virtual void f();", "A member a derived class may replace."},
        {"override", "void f() override;", "Says this member replaces a virtual one, and fails to compile when it does not."},
        {"final", "class Name final { };", "Nothing may derive from this class, or replace this member."},
        {"template", "template <typename T>", "A declaration written once for many types."},
        {"typename", "typename T::type", "Names a type, including one that depends on a template parameter."},
        {"auto", "auto value = expr;", "The type is taken from what is assigned."},
        {"nullptr", "nullptr", "The null pointer value, with a type of its own rather than an integer."},
        {"new", "new Type(args)", "Allocates and constructs; the result belongs to whoever deletes it."},
        {"delete", "delete pointer;", "Destroys and releases what new made."},
        {"using", "using Name = Type;", "An alias for a type, or a name brought into this scope."},
        {"enum class", "enum class Name { A, B };", "A scoped enumeration whose names do not leak into the enclosing scope."},
        {"mutable", "mutable Type name;", "A member a const member function may still change."},
        {"noexcept", "void f() noexcept;", "Promises not to let an exception out."},
        {"static_cast", "static_cast<Type>(value)", "A conversion the compiler checks, and the one to reach for first."},
        {"const_cast", "const_cast<Type>(value)", "Adds or removes const; needed almost never."},
        {"reinterpret_cast", "reinterpret_cast<Type>(value)", "Reinterprets the bits; the last resort."},
        {"friend", "friend class Other;", "Gives another type access to this one's private members."},
        {"operator", "Type operator+(const Type& other);", "Gives an operator a meaning for a type."},
        {"sizeof", "sizeof(Type)", "The size of a type or value in bytes."},
        {"union", "union Name { };", "Members that share one piece of storage."},
        {"thread_local", "thread_local Type name;", "One value per thread rather than one for the program."},
        {"volatile", "volatile Type* pointer;", "Tells the compiler the value may change behind its back; almost never what you want."},
        {"try", "try { } catch (const std::exception& error) { }", "Runs a block, and catches what it throws."},
    };
    return shortPages(keywords, "keyword");
}
std::vector<DocEntry> glslPages() {
    static const ShortPage words[] = {
        {"uniform", "uniform mat4 model;", "A value the program sets from the CPU before a draw."},
        {"in", "in vec3 position;", "A value arriving from the previous stage, or a vertex attribute."},
        {"out", "out vec3 colour;", "A value sent on to the next stage."},
        {"layout", "layout(location = 0) in vec3 position;", "Fixes where a variable sits in the pipeline."},
        {"vec2", "vec2(x, y)", "Two floats."},
        {"vec3", "vec3(x, y, z)", "Three floats, the shape of a position or a direction here."},
        {"vec4", "vec4(x, y, z, w)", "Four floats, the shape of a colour or a clip-space position."},
        {"mat4", "mat4(1.0)", "A four by four matrix, which is how a transform is passed in."},
        {"texture", "texture(sampler, uv)", "Samples a texture at a coordinate."},
        {"gl_Position", "gl_Position = ...", "Where a vertex lands in clip space; the vertex shader must write it."},
        {"gl_FragColor", "gl_FragColor = ...", "The colour a fragment shader writes, in the GLSL this project targets."},
        {"discard", "discard;", "Throws the fragment away instead of drawing it."},
        {"precision", "precision mediump float;", "The default precision of floating-point values in this shader."},
    };
    return shortPages(words, "keyword");
}

std::vector<DocEntry> javascriptPages() {
    static const ShortPage words[] = {
        {"function", "function name(args) { }", "A named function; calling it runs its body."},
        {"const", "const name = value;", "A binding that cannot be reassigned."},
        {"let", "let name = value;", "A binding scoped to its block."},
        {"var", "var name = value;", "A binding scoped to its function; the older form."},
        {"class", "class Name { }", "A type with a constructor and methods."},
        {"async", "async function f() { await g(); }", "A function that returns a promise and may await inside."},
        {"await", "await promise", "Waits for a promise, and only inside async."},
        {"import", "import { name } from './module.js';", "Brings names in from another module."},
        {"export", "export function f() { }", "Makes a name visible to the modules that import this one."},
        {"document", "document.querySelector(selector)", "The page this script runs in."},
        {"fetch", "await fetch(url)", "Asks the server for a resource and hands back a promise."},
        {"addEventListener", "element.addEventListener('click', handler)", "Runs a handler when an event happens on the element."},
    };
    return shortPages(words, "keyword");
}

std::vector<DocEntry> cssPages() {
    static const ShortPage words[] = {
        {"display", "display: flex;", "How the element takes part in layout: block, inline, flex, grid, none."},
        {"position", "position: absolute;", "How the element is placed: static, relative, absolute, fixed, sticky."},
        {"flex", "display: flex;", "Lays the children out in a row that grows and wraps."},
        {"grid", "display: grid;", "Lays the children out in rows and columns at once."},
        {"color", "color: #f5f7fb;", "The colour of the text."},
        {"margin", "margin: 8px 0;", "Space outside the element's border."},
        {"padding", "padding: 12px;", "Space inside the border, around the content."},
        {"border", "border: 1px solid #2a2f3a;", "The line drawn around the box."},
        {"font-family", "font-family: system-ui, sans-serif;", "The typeface the text is set in."},
        {"@media", "@media (max-width: 700px) { }", "Rules that only apply when the query matches, which is how a page changes on a phone."},
    };
    return shortPages(words, "keyword");
}

std::vector<DocEntry> htmlPages() {
    static const ShortPage words[] = {
        {"div", "<div class=\"panel\"></div>", "A block with no meaning of its own, used to group and style."},
        {"span", "<span>text</span>", "An inline run of text to style on its own."},
        {"a", "<a href=\"/play\">Play</a>", "A link to another page."},
        {"img", "<img src=\"face.png\" alt=\"face\">", "An image, with alt text for when it cannot load."},
        {"script", "<script src=\"app.js\"></script>", "Loads or holds the page's JavaScript."},
        {"link", "<link rel=\"stylesheet\" href=\"style.css\">", "Attaches a stylesheet or another resource."},
        {"canvas", "<canvas id=\"view\"></canvas>", "A drawing surface a script paints into."},
        {"form", "<form action=\"/login\" method=\"post\">", "Collects inputs and submits them to the server."},
    };
    return shortPages(words, "keyword");
}

std::vector<DocEntry> jsonPages() {
    static const ShortPage words[] = {
        {"object", "{ \"key\": value }", "A set of named values, in braces."},
        {"array", "[ value, value ]", "An ordered list of values, in brackets."},
        {"string", "\"text\"", "Text in double quotes, the only quoting JSON has."},
        {"number", "42, -1.5, 2e10", "A number; there is no separate integer type."},
    };
    return shortPages(words, "keyword");
}

std::vector<DocEntry> markdownPages() {
    static const ShortPage words[] = {
        {"heading", "# Title, ## Section", "A heading, one level per hash."},
        {"list", "- item, 1. item", "A bulleted or numbered list."},
        {"code", "a fenced block", "A block set in a monospaced font and coloured by its language."},
        {"link", "[text](path)", "A link, with the text in brackets and the target in parentheses."},
        {"table", "| a | b |", "A pipe-delimited table, with a separator row under the header."},
        {"quote", "> text", "A quotation, marked with a caret."},
    };
    return shortPages(words, "keyword");
}

// The editor explains itself the way it explains the code: a page per part,
// reachable from the documentation window and from F1 like any other.
std::vector<DocEntry> editorReference() {
    std::vector<DocEntry> out;
    out.push_back(makePage(
        "RBLXCode", "editor", "RBLXCode",
        "The source editor for this repository.",
        "RBLXCode opens the folder it is launched from and edits the files in it in place. The "
        "left panel is the project tree and the project search, the middle panel lists the "
        "symbols of the file in front, and the rest is the text, split into tabs.\n\n"
        "It reads and writes the bytes that are already there: a CRLF file stays CRLF, so a "
        "one-line change stays a one-line diff. An edit made outside the editor is noticed and "
        "reloaded, unless the buffer has unsaved work, in which case the editor asks first.",
        {"Completions", "Documentation"}));
    out.push_back(makePage(
        "Completions", "editor", "Ctrl+Space",
        "Suggestions while typing, ranked by what has been typed.",
        "Suggestions open as a name is typed, and after a '.' or ':' they narrow to the members of "
        "what stands in front of the dot. Ctrl+Space asks for them without typing first.\n\n"
        "Up and Down walk the list, Enter or Tab takes the highlighted one, Escape closes it and "
        "leaves what was typed. Taking a suggestion replaces the prefix with the whole name as a "
        "single edit, so one undo takes it back. The list is drawn from three places: the "
        "reference that ships with the editor, the symbols this file declares, and the names the "
        "file already uses.",
        {"Documentation", "RBLXCode"}));
    out.push_back(makePage(
        "Documentation", "editor", "F1",
        "What is under the mouse, and a window that holds every page.",
        "Hovering a name shows how it is written and one sentence about it. Hovering a file in "
        "the tree shows what the file says it does, taken from the comment it opens with.\n\n"
        "F1 opens the documentation for whatever the caret is on, in a window with a filter over "
        "every page: the language reference, these editor pages, the symbols in this file, and "
        "the page for the file itself. The related pages at the bottom of each one are links.",
        {"Completions", "RBLXCode"}));
    out.push_back(makePage(
        "Project tree", "editor", "the left panel",
        "Every file worth opening, and a search over all of them.",
        "The tree lists source, shaders, scripts, pages and project files, skipping build output, "
        "dependencies and the git directory. The box above it searches the text of every listed "
        "file and jumps to the line of a result.\n\nF5 walks the folder again, for when a file "
        "was added outside the editor.", {"RBLXCode"}));
    out.push_back(makePage(
        "Find and replace", "editor", "Ctrl+F, Ctrl+H",
        "Walking the matches of one file, with or without case.",
        "Ctrl+F opens the bar and Ctrl+H opens it with the replacement field. Match case and "
        "whole word narrow the search, F3 and Shift+F3 walk the matches in either direction, and "
        "Replace all rewrites the buffer in one step so a single undo takes it back.",
        {"RBLXCode"}));
    out.push_back(makePage(
        "Outline", "editor", "Ctrl+Shift+O",
        "The symbols of the file in front.",
        "The middle panel lists what the file declares: functions, classes and types from braces "
        "and keywords in C++ and GLSL, functions in RBLX Script and JavaScript, headings in "
        "Markdown and keys in JSON. A click jumps to the line, and Ctrl+Shift+O filters the list "
        "by name.", {"RBLXCode"}));
    return out;
}

bool sameName(const std::string& left, const std::string& right) {
    return lowered(left) == lowered(right);
}

bool isChainChar(char c) {
    return isIdentifierChar(c) || c == '.' || c == ':';
}

// The first sentence of a comment block, for the line under a hover.
std::string firstSentence(const std::string& text) {
    const std::string trimmed = trimCopy(text);
    const std::size_t stop = trimmed.find(". ");
    std::string summary = stop == std::string::npos ? trimmed : trimmed.substr(0, stop + 1);
    const std::size_t newline = summary.find('\n');
    if (newline != std::string::npos) {
        summary = summary.substr(0, newline);
    }
    return trimCopy(summary);
}

// One line of a comment with its marker taken off. false when the line is not a
// comment at all, which is where a doc block ends.
bool stripCommentLine(const std::string& line, std::string* body) {
    const std::string trimmed = trimCopy(line);
    body->clear();
    if (trimmed.empty()) {
        return true;
    }
    if (trimmed.rfind("//", 0) == 0) {
        *body = trimCopy(trimmed.substr(2));
        return true;
    }
    if (trimmed.rfind("--", 0) == 0) {
        *body = trimCopy(trimmed.substr(2));
        return true;
    }
    if (trimmed.rfind("/*", 0) == 0) {
        std::string rest = trimmed.substr(2);
        if (!rest.empty() && rest[0] == '*') {
            rest = rest.substr(1);
        }
        if (!rest.empty() && rest[rest.size() - 1] == '/') {
            rest = rest.substr(0, rest.size() - 1);
        }
        *body = trimCopy(rest);
        return true;
    }
    if (trimmed[0] == '*') {
        *body = trimCopy(trimmed.substr(1));
        return true;
    }
    return false;
}

std::string joinCommentLines(std::vector<std::string> lines) {
    while (!lines.empty() && trimCopy(lines.front()).empty()) {
        lines.erase(lines.begin());
    }
    while (!lines.empty() && trimCopy(lines.back()).empty()) {
        lines.pop_back();
    }
    std::string out;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (i != 0) {
            out.push_back('\n');
        }
        out.append(lines[i]);
    }
    return out;
}

}  // namespace
const std::vector<DocEntry>& referencePages(Language language) {
    static const std::vector<DocEntry> script = scriptPages();
    static const std::vector<DocEntry> cpp = cppPages();
    static const std::vector<DocEntry> glsl = glslPages();
    static const std::vector<DocEntry> javascript = javascriptPages();
    static const std::vector<DocEntry> css = cssPages();
    static const std::vector<DocEntry> html = htmlPages();
    static const std::vector<DocEntry> json = jsonPages();
    static const std::vector<DocEntry> markdown = markdownPages();
    static const std::vector<DocEntry> empty;
    switch (language) {
        case Language::Lua: return script;
        case Language::Cpp: return cpp;
        case Language::Glsl: return glsl;
        case Language::JavaScript: return javascript;
        case Language::Css: return css;
        case Language::Html: return html;
        case Language::Json: return json;
        case Language::Markdown: return markdown;
        case Language::Plain: break;
    }
    return empty;
}

const std::vector<DocEntry>& editorPages() {
    static const std::vector<DocEntry> pages = editorReference();
    return pages;
}

const DocEntry* findReferencePage(const std::string& name, Language language) {
    if (name.empty()) {
        return nullptr;
    }
    const std::vector<DocEntry>& pages = referencePages(language);
    for (const DocEntry& page : pages) {
        if (page.name == name) {
            return &page;
        }
    }
    for (const DocEntry& page : pages) {
        if (sameName(page.name, name)) {
            return &page;
        }
    }
    // `workspace:FindFirstChild` asks for the page the reference files under
    // `workspace.FindFirstChild`, and a bare member name finds it too.
    std::string dotted;
    for (char c : name) {
        dotted.push_back(c == ':' ? '.' : c);
    }
    for (const DocEntry& page : pages) {
        if (sameName(page.name, dotted)) {
            return &page;
        }
    }
    const std::size_t dot = dotted.rfind('.');
    if (dot != std::string::npos) {
        for (const DocEntry& page : pages) {
            if (sameName(page.name, dotted.substr(dot + 1))) {
                return &page;
            }
        }
    }
    return nullptr;
}

WordUnderCursor wordUnderCursor(const std::string& text, std::size_t offset) {
    WordUnderCursor result;
    if (text.empty()) {
        return result;
    }
    if (offset > text.size()) {
        offset = text.size();
    }
    // A caret sits between two characters, so a position just past a name counts
    // as being in it -- that is the position a hover and a completion both use.
    std::size_t at = offset;
    if (at == text.size() || !isChainChar(text[at])) {
        if (at == 0 || !isChainChar(text[at - 1])) {
            return result;
        }
        --at;
    }
    std::size_t begin = at;
    while (begin > 0 && isChainChar(text[begin - 1])) {
        --begin;
    }
    std::size_t end = at;
    while (end < text.size() && isChainChar(text[end])) {
        ++end;
    }
    while (begin < end && (text[begin] == '.' || text[begin] == ':')) {
        ++begin;
    }
    while (end > begin && (text[end - 1] == '.' || text[end - 1] == ':')) {
        --end;
    }
    if (begin >= end) {
        return result;
    }
    result.chain = text.substr(begin, end - begin);
    std::size_t lastStart = begin;
    for (std::size_t i = begin; i < end; ++i) {
        if ((text[i] == '.' || text[i] == ':') && i + 1 < end) {
            lastStart = i + 1;
        }
    }
    result.word = text.substr(lastStart, end - lastStart);
    if (lastStart > begin) {
        result.qualifier = text.substr(begin, lastStart - begin - 1);
    }
    result.start = lastStart;
    result.end = end;
    return result;
}

std::string docCommentAbove(const std::string& text, int line) {
    const std::vector<std::string> lines = splitLines(text);
    std::vector<std::string> collected;
    int index = line - 2;  // the line above a 1-based line, counted from zero
    while (index >= 0 && index < static_cast<int>(lines.size())) {
        const std::string& lineText = lines[static_cast<std::size_t>(index)];
        std::string body;
        if (!stripCommentLine(lineText, &body)) {
            break;
        }
        if (trimCopy(lineText).empty() && collected.empty()) {
            --index;
            continue;
        }
        collected.push_back(body);
        --index;
    }
    const std::vector<std::string> ordered(collected.rbegin(), collected.rend());
    return joinCommentLines(ordered);
}

std::string leadingComment(const std::string& text) {
    const std::vector<std::string> lines = splitLines(text);
    std::vector<std::string> collected;
    bool started = false;
    for (const std::string& line : lines) {
        const std::string trimmed = trimCopy(line);
        if (trimmed.empty() && !started) {
            continue;
        }
        std::string body;
        if (!stripCommentLine(line, &body)) {
            break;
        }
        started = true;
        collected.push_back(body);
    }
    return joinCommentLines(collected);
}

DocEntry documentationAt(const std::string& text, Language language, std::size_t offset) {
    const WordUnderCursor under = wordUnderCursor(text, offset);
    if (under.empty()) {
        return DocEntry();
    }
    if (const DocEntry* page = findReferencePage(under.chain, language)) {
        return *page;
    }
    if (const DocEntry* page = findReferencePage(under.word, language)) {
        return *page;
    }

    // Nothing in the reference: this file may declare the name itself, and if it
    // does, the comment above the declaration is the documentation for it.
    for (const OutlineSymbol& symbol : outlineSymbols(text, language)) {
        if (!sameName(symbol.name, under.word) && !sameName(symbol.name, under.chain)) {
            continue;
        }
        const std::string comment = docCommentAbove(text, symbol.line);
        DocEntry entry;
        entry.name = symbol.name;
        entry.kind = symbol.kind.empty() ? "symbol" : symbol.kind;
        entry.signature = symbol.detail.empty() ? symbol.name : symbol.detail;
        entry.summary = comment.empty()
                            ? "Declared in this file at line " + std::to_string(symbol.line) + "."
                            : firstSentence(comment);
        entry.body = comment.empty() ? entry.summary : comment;
        return entry;
    }
    return DocEntry();
}

DocEntry documentationForFile(const std::string& relativePath, const std::string& text) {
    const Language language = languageForPath(relativePath);
    const std::string comment = leadingComment(text);
    DocEntry entry;
    entry.name = relativePath;
    entry.kind = "file";
    entry.signature = std::string(languageName(language)) + "  |  " +
                      std::to_string(lineCount(text)) + " lines  |  " +
                      formatBytes(text.size());
    entry.summary = comment.empty()
                        ? "No comment at the top of this file; its name is all it says about itself."
                        : firstSentence(comment);
    entry.body = comment.empty() ? entry.summary : comment;
    return entry;
}

// ---------------------------------------------------------------------------
// Completions
// ---------------------------------------------------------------------------

namespace {

// How well a name answers what has been typed: a prefix first, then a match
// anywhere, then an abbreviation, and nothing at all when the letters are not
// there in order.
int rankMatch(const std::string& label, const std::string& prefix) {
    if (prefix.empty()) {
        return 0;
    }
    const std::string name = lowered(label);
    const std::string typed = lowered(prefix);
    if (name.rfind(typed, 0) == 0) {
        return 0;
    }
    if (name.find(typed) != std::string::npos) {
        return 1;
    }
    return fuzzyScore(label, prefix) >= 0 ? 2 : -1;
}

std::string lastSegment(const std::string& name) {
    std::size_t start = 0;
    for (std::size_t i = 0; i < name.size(); ++i) {
        if ((name[i] == '.' || name[i] == ':') && i + 1 < name.size()) {
            start = i + 1;
        }
    }
    return name.substr(start);
}

// The names a file already uses, most used first: a suggestion list that knows
// nothing about types still feels local if it offers the names in the buffer.
std::vector<std::string> identifiersIn(const std::string& text) {
    std::vector<std::pair<int, std::string>> counted;
    std::size_t at = 0;
    while (at < text.size()) {
        if (!std::isalpha(static_cast<unsigned char>(text[at])) && text[at] != '_') {
            ++at;
            continue;
        }
        const std::size_t begin = at;
        while (at < text.size() && isIdentifierChar(text[at])) {
            ++at;
        }
        const std::string word = text.substr(begin, at - begin);
        if (word.size() < 3) {
            continue;
        }
        bool seen = false;
        for (std::pair<int, std::string>& entry : counted) {
            if (entry.second == word) {
                ++entry.first;
                seen = true;
                break;
            }
        }
        if (!seen) {
            counted.emplace_back(1, word);
        }
        if (counted.size() > 600) {
            break;
        }
    }
    std::stable_sort(counted.begin(), counted.end(),
                     [](const std::pair<int, std::string>& left,
                        const std::pair<int, std::string>& right) {
                         if (left.first != right.first) {
                             return left.first > right.first;
                         }
                         return left.second < right.second;
                     });
    std::vector<std::string> words;
    words.reserve(counted.size());
    for (const std::pair<int, std::string>& entry : counted) {
        words.push_back(entry.second);
    }
    return words;
}

struct RankedItem {
    int rank = 0;
    CompletionItem item;
};

}  // namespace

CompletionList completionsAt(const std::string& text, Language language, std::size_t offset,
                             std::size_t maxItems) {
    CompletionList list;
    if (offset > text.size()) {
        offset = text.size();
    }
    std::size_t start = offset;
    while (start > 0 && isIdentifierChar(text[start - 1])) {
        --start;
    }
    list.tokenStart = start;
    list.prefix = text.substr(start, offset - start);
    if (start > 0 && (text[start - 1] == '.' || text[start - 1] == ':')) {
        list.memberAccess = true;
        std::size_t qualifierStart = start - 1;
        while (qualifierStart > 0 && isChainChar(text[qualifierStart - 1])) {
            --qualifierStart;
        }
        list.qualifier = text.substr(qualifierStart, start - 1 - qualifierStart);
    }

    std::vector<RankedItem> ranked;

    // The reference, either the members of what stands before the dot or the
    // global names of the language.
    const std::vector<DocEntry>& pages = referencePages(language);
    if (list.memberAccess && !list.qualifier.empty()) {
        const std::string dotted = lowered(list.qualifier);
        for (const DocEntry& page : pages) {
            const std::string name = lowered(page.name);
            if (name.size() <= dotted.size() || name.compare(0, dotted.size(), dotted) != 0 ||
                name[dotted.size()] != '.') {
                continue;
            }
            CompletionItem item;
            item.label = page.name.substr(dotted.size() + 1);
            item.kind = page.kind;
            item.detail = page.signature;
            item.insertText = item.label;
            item.documentation = page.summary;
            const int rank = rankMatch(item.label, list.prefix);
            if (rank < 0) {
                continue;
            }
            ranked.push_back(RankedItem{rank, item});
        }
    }

    if (ranked.empty()) {
        for (const DocEntry& page : pages) {
            if (page.name.find('.') != std::string::npos || page.name.find(':') != std::string::npos) {
                continue;
            }
            CompletionItem item;
            item.label = page.name;
            item.kind = page.kind;
            item.detail = page.signature;
            item.insertText = page.name;
            item.documentation = page.summary;
            const int rank = rankMatch(item.label, list.prefix);
            if (rank < 0) {
                continue;
            }
            ranked.push_back(RankedItem{rank, item});
        }
        for (const OutlineSymbol& symbol : outlineSymbols(text, language)) {
            CompletionItem item;
            item.label = lastSegment(symbol.name);
            if (item.label.empty()) {
                continue;
            }
            item.kind = symbol.kind.empty() ? "symbol" : symbol.kind;
            item.detail = symbol.detail;
            item.insertText = item.label;
            item.documentation = "Declared at line " + std::to_string(symbol.line) + " of this file.";
            const int rank = rankMatch(item.label, list.prefix);
            if (rank < 0) {
                continue;
            }
            ranked.push_back(RankedItem{rank, item});
        }
        for (const std::string& word : identifiersIn(text)) {
            // The name being typed is already on the line: offering it back would
            // put the half-typed word at the top of its own list.
            if (!list.prefix.empty() && sameName(word, list.prefix)) {
                continue;
            }
            CompletionItem item;
            item.label = word;
            item.kind = "word";
            item.detail = "used in this file";
            item.insertText = word;
            const int rank = rankMatch(item.label, list.prefix);
            if (rank < 0) {
                continue;
            }
            ranked.push_back(RankedItem{rank, item});
        }
    }

    std::stable_sort(ranked.begin(), ranked.end(),
                     [](const RankedItem& left, const RankedItem& right) {
                         if (left.rank != right.rank) {
                             return left.rank < right.rank;
                         }
                         if (left.item.label.size() != right.item.label.size()) {
                             return left.item.label.size() < right.item.label.size();
                         }
                         return left.item.label < right.item.label;
                     });

    for (const RankedItem& entry : ranked) {
        bool duplicate = false;
        for (const CompletionItem& kept : list.items) {
            if (sameName(kept.label, entry.item.label)) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) {
            continue;
        }
        list.items.push_back(entry.item);
        if (list.items.size() >= maxItems) {
            break;
        }
    }
    return list;
}

}  // namespace code
}  // namespace rblx
