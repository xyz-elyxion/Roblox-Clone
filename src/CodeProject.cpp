#include "CodeProject.h"

#include <algorithm>
#include <cctype>
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

}  // namespace code
}  // namespace rblx
