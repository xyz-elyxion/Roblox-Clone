#include "DocsLibrary.h"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <utility>

namespace linear {
namespace docs {
namespace {

std::string toUtf8(const std::wstring& wide) {
    if (wide.empty()) return std::string();
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()),
                                          nullptr, 0, nullptr, nullptr);
    std::string text(static_cast<std::size_t>(bytes), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), text.data(), bytes,
                        nullptr, nullptr);
    return text;
}

std::wstring toWide(const std::string& text) {
    if (text.empty()) return std::wstring();
    const int wide = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()),
                                         nullptr, 0);
    std::wstring result(static_cast<std::size_t>(wide), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), result.data(),
                        wide);
    return result;
}

std::string parentOf(const std::string& path) {
    const std::size_t slash = path.find_last_of("\\/");
    if (slash == std::string::npos) return std::string();
    // "C:\a" would otherwise walk to "C:" and then keep finding nothing useful.
    if (slash == 2 && path.size() > 2 && path[1] == ':') return std::string();
    return path.substr(0, slash);
}

std::vector<std::string> markdownFiles(const std::string& folder) {
    std::vector<std::string> names;
    WIN32_FIND_DATAW data = {};
    HANDLE handle = FindFirstFileW(toWide(folder + "\\*.md").c_str(), &data);
    if (handle == INVALID_HANDLE_VALUE) return names;
    do {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) continue;
        names.push_back(toUtf8(data.cFileName));
    } while (FindNextFileW(handle, &data) != 0);
    FindClose(handle);
    std::sort(names.begin(), names.end());
    return names;
}

// The title the list shows: the first level-one heading, because every one of
// these files opens with its own name. Falls back to the file name, so a file
// written without a heading still appears.
std::string titleOf(const std::string& text, const std::string& fallback) {
    std::size_t start = 0;
    while (start < text.size()) {
        std::size_t end = text.find('\n', start);
        if (end == std::string::npos) end = text.size();
        std::string line = text.substr(start, end - start);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.size() > 2 && line[0] == '#' && line[1] == ' ') {
            std::string title = line.substr(2);
            while (!title.empty() && (title.back() == '#' || title.back() == ' ')) title.pop_back();
            if (!title.empty()) return title;
        }
        start = end + 1;
    }
    return fallback;
}

}  // namespace

std::string executablePath() {
    wchar_t wide[MAX_PATH * 4] = {};
    const DWORD length =
        GetModuleFileNameW(nullptr, wide, static_cast<DWORD>(sizeof(wide) / sizeof(wide[0])));
    if (length == 0) return std::string();
    return toUtf8(std::wstring(wide, length));
}

std::string folderFor(const std::string& executablePath) {
    // The walk also covers a build tree, where the executable sits several
    // folders below the repository and the documents are at the top of it.
    std::string folder = parentOf(executablePath);
    for (int level = 0; level < 4 && !folder.empty(); ++level) {
        if (!markdownFiles(folder + "\\docs").empty()) return folder + "\\docs";
        if (!markdownFiles(folder).empty()) return folder;
        folder = parentOf(folder);
    }
    return std::string();
}

bool loadAll(const std::string& folder, std::vector<Source>* sources, std::string* error) {
    sources->clear();
    std::vector<std::string> names = markdownFiles(folder);
    if (names.empty()) {
        if (error != nullptr) *error = "No .md files in " + folder + ".";
        return false;
    }

    // Reading order rather than file system order: the README is the manual, and
    // the rest are reference pages somebody opens on purpose.
    const char* const preferred[] = {"README.md",       "SECURITY.md", "HOSTING.md",
                                     "LINEARBUILD.md",  "CONTRIBUTING.md",
                                     "CODE_OF_CONDUCT.md", "AGENTS.md"};
    std::vector<std::string> ordered;
    for (const char* name : preferred) {
        const auto found = std::find(names.begin(), names.end(), std::string(name));
        if (found == names.end()) continue;
        ordered.push_back(*found);
        names.erase(found);
    }
    ordered.insert(ordered.end(), names.begin(), names.end());

    for (const std::string& name : ordered) {
        Source source;
        source.name = name;
        source.path = folder + "\\" + name;

        std::ifstream file(source.path, std::ios::binary);
        if (!file) {
            if (error != nullptr) *error = "Could not open " + source.path + ".";
            continue;
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        source.text = buffer.str();

        source.lines = 1;
        for (char c : source.text) {
            if (c == '\n') ++source.lines;
        }
        if (!source.text.empty() && source.text.back() == '\n') --source.lines;
        source.title = titleOf(source.text, name.substr(0, name.size() - 3));
        sources->push_back(std::move(source));
    }

    if (sources->empty()) {
        if (error != nullptr) *error = "Nothing in " + folder + " could be read.";
        return false;
    }
    return true;
}

}  // namespace docs
}  // namespace linear
