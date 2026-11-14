#include "CodeBuild.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <system_error>

namespace rblx {
namespace code {
namespace {

namespace fs = std::filesystem;

std::string trim(const std::string& text) {
    std::size_t start = 0;
    while (start < text.size() && (text[start] == ' ' || text[start] == '\t')) ++start;
    std::size_t end = text.size();
    while (end > start && (text[end - 1] == ' ' || text[end - 1] == '\t' || text[end - 1] == '\r')) {
        --end;
    }
    return text.substr(start, end - start);
}

// A path the editor could open: it has to look like a source file, because a
// linker message names `ServerSetup.obj` and a row that cannot be opened is worse
// than no row at all.
bool looksLikeSource(const std::string& path) {
    const std::size_t dot = path.rfind('.');
    if (dot == std::string::npos) return false;
    std::string extension = path.substr(dot);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    for (const char* known : {".cpp", ".cc", ".cxx", ".c", ".h", ".hpp", ".hh", ".inl", ".glsl",
                             ".frag", ".vert", ".js", ".rbx", ".asm", ".cs"}) {
        if (extension == known) return true;
    }
    return false;
}

bool startsWithDigits(const std::string& text, int* value, std::size_t* consumed) {
    std::size_t index = 0;
    int parsed = 0;
    while (index < text.size() && text[index] >= '0' && text[index] <= '9' && index < 9) {
        parsed = parsed * 10 + (text[index] - '0');
        ++index;
    }
    if (index == 0) return false;
    *value = parsed;
    if (consumed != nullptr) *consumed = index;
    return true;
}

// The severity and code of the sentence after the location: "error C2065:",
// "warning C4005:", "error:", "note:".
bool splitSeverity(const std::string& text, std::string* severity, std::string* code,
                   std::string* message) {
    std::size_t index = 0;
    while (index < text.size() && text[index] == ' ') ++index;
    const std::size_t start = index;
    while (index < text.size() && std::isalpha(static_cast<unsigned char>(text[index]))) ++index;
    const std::string word = text.substr(start, index - start);
    if (word != "error" && word != "warning" && word != "note" && word != "fatal") {
        return false;
    }
    *severity = word == "fatal" ? "error" : word;
    while (index < text.size() && text[index] == ' ') ++index;

    // The code is one word of upper-case letters and digits.
    std::size_t codeStart = index;
    while (index < text.size() &&
           (std::isupper(static_cast<unsigned char>(text[index])) ||
            std::isdigit(static_cast<unsigned char>(text[index])))) {
        ++index;
    }
    if (index > codeStart) {
        *code = text.substr(codeStart, index - codeStart);
        while (index < text.size() && text[index] == ' ') ++index;
    }
    if (index < text.size() && text[index] == ':') {
        ++index;
        while (index < text.size() && text[index] == ' ') ++index;
    }
    *message = trim(text.substr(index));
    return true;
}

}  // namespace

bool findBuildTool(const std::string& projectRoot, std::string* program, std::string* error) {
    std::vector<fs::path> candidates;
    const char* overridePath = std::getenv("RBLXBUILD");
    if (overridePath != nullptr && *overridePath != '\0') {
        const fs::path given(overridePath);
        candidates.push_back(given.is_absolute() ? given : fs::path(projectRoot) / given);
    }
    candidates.push_back(fs::path(projectRoot) / "build" / "rblxbuild-tool" / "bin" /
                         "rblxbuild.exe");
    candidates.push_back(fs::path(projectRoot) / "build" / "rblxbuild-tool" / "bin" / "rblxbuild");

    std::error_code code;
    for (const fs::path& candidate : candidates) {
        if (fs::is_regular_file(candidate, code)) {
            *program = candidate.string();
            return true;
        }
    }

    if (error != nullptr) {
        *error = "No build tool in " + projectRoot +
                 ". Build it once from this folder: mkdir -p build/rblxbuild-tool, then "
                 "../../rblxbuild/bootstrap --parallel 4 inside it (RBLXBUILD points at one "
                 "somewhere else).";
    }
    return false;
}

std::string buildCommandLine(const std::string& program, const std::string& preset) {
    const std::string tool = preset.empty() ? kDefaultPreset : preset;
    return "\"" + program + "\" --build --preset " + tool;
}

std::string resolveDiagnosticPath(const std::string& projectRoot, const std::string& printed) {
    if (printed.empty()) return std::string();
    const fs::path path(printed);
    if (path.is_absolute() || (printed.size() > 1 && printed[1] == ':')) {
        return path.lexically_normal().string();
    }
    if (projectRoot.empty()) return path.string();
    return (fs::path(projectRoot) / path).lexically_normal().string();
}

std::vector<Diagnostic> parseDiagnostics(const std::vector<std::string>& lines) {
    std::vector<Diagnostic> found;
    for (const std::string& raw : lines) {
        const std::string line = trim(raw);
        if (line.empty()) continue;

        // MSVC: `path(line,col): severity CODE: message`, or `path(line): ...`.
        // The path is everything before the parenthesis, which may contain spaces
        // (a folder called "Roblox Clone"), so the match is anchored at the end.
        Diagnostic diagnostic;
        bool matched = false;

        const std::size_t close = line.find("): ");
        if (close != std::string::npos) {
            const std::size_t open = line.rfind('(', close);
            if (open != std::string::npos && open > 0) {
                const std::string numbers = line.substr(open + 1, close - open - 1);
                const std::size_t comma = numbers.find(',');
                int number = 0;
                std::size_t consumed = 0;
                if (startsWithDigits(numbers, &number, &consumed)) {
                    diagnostic.path = line.substr(0, open);
                    diagnostic.line = number;
                    std::size_t rest = consumed;
                    if (comma != std::string::npos) {
                        if (startsWithDigits(numbers.substr(comma + 1), &number, &consumed)) {
                            diagnostic.column = number;
                            rest = comma + 1 + consumed;
                        }
                    }
                    if (rest >= numbers.size()) {
                        matched = looksLikeSource(diagnostic.path) &&
                                  splitSeverity(line.substr(close + 2), &diagnostic.severity,
                                                &diagnostic.code, &diagnostic.message);
                    }
                }
            }
        }

        // GCC/Clang: `path:line:col: severity: message`.
        if (!matched) {
            const std::size_t first = line.find(':');
            if (first != std::string::npos && first > 0) {
                const std::string path = line.substr(0, first);
                int number = 0;
                std::size_t consumed = 0;
                int column = 0;
                std::size_t after = 0;
                if (looksLikeSource(path) &&
                    startsWithDigits(line.substr(first + 1), &number, &consumed)) {
                    after = first + 1 + consumed;
                    if (after < line.size() && line[after] == ':') {
                        column = 0;
                        std::size_t columnConsumed = 0;
                        if (startsWithDigits(line.substr(after + 1), &column, &columnConsumed)) {
                            after = after + 1 + columnConsumed;
                        }
                        if (after < line.size() && line[after] == ':') {
                            matched = splitSeverity(line.substr(after + 1), &diagnostic.severity,
                                                    &diagnostic.code, &diagnostic.message);
                            if (matched) {
                                diagnostic.path = path;
                                diagnostic.line = number;
                                diagnostic.column = column;
                            }
                        }
                    }
                }
            }
        }

        if (!matched) continue;
        diagnostic.text = line;
        found.push_back(diagnostic);
    }
    return found;
}

}  // namespace code
}  // namespace rblx
