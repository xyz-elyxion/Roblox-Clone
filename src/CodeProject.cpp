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
    // Generated or third-party trees. `vendor` is the site's pinned three.js and
    // `GameRelease` is a packaged build, so neither is source anybody edits here.
    static const char* const ignored[] = {
        ".git",      ".vs",        "build",       "node_modules", "vendor",
        "artifacts", "__pycache__", ".cache",     "dist",         "out",
        "cmake-build-debug",        "cmake-build-release",          ".idea",
        "gamerelease",              "data"
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
        "cmakelists.txt", "makefile", "dockerfile", ".gitignore", ".gitattributes",
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
        ".cmake", ".yml", ".yaml", ".toml", ".ini", ".world"
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
