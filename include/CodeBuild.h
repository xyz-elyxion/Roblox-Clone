#pragma once

// Building the checkout from the editor, and reading what the compiler said.
//
// RBLXCode is the editor for this repository, and it used to be the one place where
// changing a source file and finding out whether it still compiles meant leaving:
// the build tool is vendored under build/, so there is nothing to install and no
// reason the editor cannot run it.
//
// Everything here is text and the filesystem, so it is tested without a compiler:
// where the build tool is, what a diagnostic line means, and how a printed path is
// turned into a file the editor can open. The process that prints those lines is
// CodeBuildRun, which is Windows-only like the rest of this project.

#include <string>
#include <vector>

namespace rblx {
namespace code {

// One line the toolchain printed that names a place in a file. Everything else it
// prints is only worth reading in the panel, so this is the subset that becomes a
// row somebody can click.
struct Diagnostic {
    std::string path;      // as printed, which may be absolute or relative to the build
    int line = 0;          // 1-based, 0 when the message names no line
    int column = 0;        // 1-based, 0 when the message names no column
    std::string severity;  // "error", "warning" or "note"
    std::string code;      // "C2065", "LNK2019", or empty
    std::string message;   // the sentence after the code
    std::string text;      // the whole line, for the panel
};

// The default preset is the one RBLXBuildPresets.json ships.
constexpr const char* kDefaultPreset = "windows-release";

// The build tool of this checkout: the vendored one under build/rblxbuild-tool,
// because that is what the bootstrap builds and what CI and the packaging script
// run. `RBLXBUILD` overrides it for a tool somewhere else. The error carries the
// bootstrap line, which is what a fresh clone needs to read.
bool findBuildTool(const std::string& projectRoot, std::string* program, std::string* error);

// `"C:\...\rblxbuild.exe" --build --preset windows-release`
std::string buildCommandLine(const std::string& program, const std::string& preset);

// Every diagnostic in these lines, in the order they were printed. Both the MSVC
// shape (`file(line,col): error C2065: ...`) and the GCC/Clang one
// (`file:line:col: error: ...`) are understood, because a warning from a vendored
// library arrives through whichever compiler is running.
std::vector<Diagnostic> parseDiagnostics(const std::vector<std::string>& lines);

// The file a diagnostic names, as an absolute path. A relative one is taken from
// the project root, which is where the build tool runs.
std::string resolveDiagnosticPath(const std::string& projectRoot, const std::string& printed);

}  // namespace code
}  // namespace rblx
