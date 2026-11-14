// CodeBuildTests: finding the build tool and reading what it prints.
//
// The process cannot run here - it is a Windows program driving a Windows build -
// but the two things that decide whether the panel is useful can be checked
// anywhere: which file is the tool, and which of its lines is an error you can
// click.

#include "CodeBuild.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace code = rblx::code;
namespace fs = std::filesystem;

namespace {

int g_failures = 0;

void check(bool ok, const std::string& what) {
    if (!ok) {
        std::printf("FAIL %s\n", what.c_str());
        ++g_failures;
    }
}

void writeFile(const fs::path& path, const std::string& text) {
    std::ofstream stream(path.string().c_str(), std::ios::binary | std::ios::trunc);
    stream << text;
}

}  // namespace

int main() {
    const fs::path root = fs::temp_directory_path() / "rblx-code-build-tests";
    std::error_code error;
    fs::remove_all(root, error);
    fs::create_directories(root, error);

    // No tool yet: the error has to say what to do, because a fresh clone is where
    // this is read for the first time.
    {
        std::string program;
        std::string message;
        check(!code::findBuildTool(root.string(), &program, &message),
              "a clone with no build tool is reported");
        check(message.find("rblxbuild/bootstrap") != std::string::npos,
              "the error names the bootstrap: " + message);
    }

    // The vendored tool, where the bootstrap puts it.
    {
        const fs::path bin = root / "build" / "rblxbuild-tool" / "bin";
        fs::create_directories(bin, error);
        writeFile(bin / "rblxbuild.exe", "not a program");
        std::string program;
        std::string message;
        check(code::findBuildTool(root.string(), &program, &message),
              "the vendored tool is found: " + message);
        check(program == (bin / "rblxbuild.exe").string(), "the found path is the tool, got " + program);
        check(code::buildCommandLine(program, code::kDefaultPreset) ==
                  "\"" + program + "\" --build --preset windows-release",
              "the command line is what the README and CI run");
        check(code::buildCommandLine(program, std::string()) ==
                  "\"" + program + "\" --build --preset windows-release",
              "an empty preset falls back to the default");
    }

    // The two shapes a compiler prints, and the lines that are not diagnostics.
    {
        const std::vector<std::string> lines = {
            "  ServerSetupMain.cpp",
            "D:\\a\\Roblox-Clone\\src\\ServerSetupMain.cpp(237,10): warning C4996: 'sscanf': This "
            "function or variable may be unsafe.",
            "D:\\a\\Roblox-Clone\\src\\main.cpp(2,9): warning C4005: 'NOMINMAX': macro redefinition",
            "src/WebApp.cpp:5682:5: error: expected ';' before 'return'",
            "src/HubSite.cpp(231): error C2065: 'Json': undeclared identifier",
            "ServerSetup.obj : error LNK2019: unresolved external symbol \"Json::parse\"",
            "##[error]Process completed with exit code 1.",
            "",
            "note: see declaration of 'Json'",
        };
        const std::vector<code::Diagnostic> found = code::parseDiagnostics(lines);

        // The linker line and the two non-diagnostic lines are not clickable, and
        // the last one names no file to click either.
        check(found.size() == 4, "four diagnostics are found, got " + std::to_string(found.size()));

        if (found.size() == 4) {
            check(found[0].path == "D:\\a\\Roblox-Clone\\src\\ServerSetupMain.cpp",
                  "the MSVC path keeps its spaces and backslashes: " + found[0].path);
            check(found[0].line == 237 && found[0].column == 10, "the MSVC line and column");
            check(found[0].severity == "warning", "the severity is read: " + found[0].severity);
            check(found[0].code == "C4996", "the code is read: " + found[0].code);
            check(found[0].message.rfind("'sscanf'", 0) == 0,
                  "the message starts after the code: " + found[0].message);
            check(found[0].text.find("warning C4996") != std::string::npos,
                  "the whole line is kept for the panel");

            check(found[1].line == 2 && found[1].column == 9, "a warning without a message indent");
            check(found[1].code == "C4005", "the second MSVC code");

            check(found[2].path == "src/WebApp.cpp", "the GCC path: " + found[2].path);
            check(found[2].line == 5682 && found[2].column == 5, "the GCC line and column");
            check(found[2].severity == "error", "the GCC severity");
            check(found[2].code.empty(), "a GCC message has no code");
            check(found[2].message == "expected ';' before 'return'",
                  "the GCC message: " + found[2].message);

            check(found[3].path == "src/HubSite.cpp", "MSVC without a column: " + found[3].path);
            check(found[3].line == 231 && found[3].column == 0,
                  "a missing column is zero, not the line");
            check(found[3].code == "C2065", "the code is read without a column");
        }
    }

    // The one failure the panel is guaranteed to cause: the build cannot replace
    // the editor it is running inside, and only that failure is worth excusing.
    {
        check(code::selfLinkBlocked(
                  {"LINK : fatal error LNK1104: cannot open file "
                   "'D:\\repo\\build\\windows-release\\Release\\RBLXCode.exe'"}),
              "the editor's own link failure is recognised");
        check(code::selfLinkBlocked(
                  {"g++: fatal error: cannot open output file RBLXCode.exe: Permission denied"}),
              "MinGW's version of the same failure is recognised");
        check(!code::selfLinkBlocked({"Client.obj : error LNK2019: unresolved external symbol"}),
              "an ordinary link error is not excused");
        check(!code::selfLinkBlocked({"LINK : fatal error LNK1104: cannot open file 'Client.exe'"}),
              "another program's link failure is not excused");
    }

    // The folder opened is usually not the checkout: the editor is started from
    // build\windows-release\Release, which is where the executables land.
    {
        const fs::path repo = root / "checkout";
        const fs::path deep = repo / "build" / "windows-release" / "Release";
        fs::create_directories(deep, error);
        writeFile(repo / "RBLXBuildPresets.json", "[]");
        check(code::findBuildRoot(deep.string()) == repo.string(),
              "a folder inside the checkout resolves to its root, got " +
                  code::findBuildRoot(deep.string()));
        check(code::findBuildRoot(repo.string()) == fs::absolute(repo).string(),
              "a checkout root resolves to itself");
        const fs::path outside = root / "elsewhere";
        fs::create_directories(outside, error);
        check(code::findBuildRoot(outside.string()) == outside.string(),
              "a folder with no checkout above it is used as it is");
        check(code::findBuildRoot(std::string()) == std::string(),
              "no folder stays no folder");
    }

    // A message that names no file at all is not a diagnostic.
    {
        const std::vector<code::Diagnostic> found = code::parseDiagnostics(
            {"Microsoft (R) Build Engine version 17.9", "1 Warning(s)", "0 Error(s)"});
        check(found.empty(), "build chatter is not turned into rows");
    }

    // Printed paths become files the editor can open. The separators are the
    // platform's, so the checks compare paths rather than spellings.
    {
        const fs::path joined = code::resolveDiagnosticPath("C:/repo", "src/main.cpp");
        check(joined == fs::path("C:/repo") / "src/main.cpp",
              "a relative path is taken from the project root, got " + joined.string());
        check(code::resolveDiagnosticPath("C:/repo", "C:/a/src/main.cpp") ==
                  fs::path("C:/a/src/main.cpp"),
              "an absolute path is kept");
        check(code::resolveDiagnosticPath("C:/repo", "").empty(), "no path stays empty");
        check(code::resolveDiagnosticPath("", "src/main.cpp") == fs::path("src/main.cpp"),
              "with no root the path is used as printed");
    }

    fs::remove_all(root, error);

    if (g_failures == 0) {
        std::printf("CodeBuildTests: all checks passed\n");
        return 0;
    }
    std::printf("CodeBuildTests: %d check(s) failed\n", g_failures);
    return 1;
}
