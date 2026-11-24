#include <cstdio>
#include <string>
#include <vector>

#include "ClientCheatReport.h"
#include "GuardChecks.h"

namespace {

int failures = 0;

void check(bool ok, const std::string& what) {
    std::printf("%s %s\n", ok ? "ok  " : "FAIL", what.c_str());
    if (!ok) ++failures;
}

}  // namespace

int main() {
    using linear::guard::cheatToolNames;
    using linear::guard::detectionLine;
    using linear::guard::fitsInLog;
    using linear::guard::isCheatToolName;
    using linear::guard::isExpectedModule;
    using linear::guard::isUserWritablePath;
    using linear::guard::kMaxLogBytes;

    // The table itself: names only, lowercase, no path, and never empty.
    check(!cheatToolNames().empty(), "the guard has a list of programs to look for");
    bool allPlainNames = true;
    for (const std::string& name : cheatToolNames()) {
        if (name.find('\\') != std::string::npos || name.find('/') != std::string::npos) {
            allPlainNames = false;
        }
        for (char c : name) {
            if (c >= 'A' && c <= 'Z') {
                allPlainNames = false;
            }
        }
    }
    check(allPlainNames, "every entry is a lowercase file name");

    check(isCheatToolName("cheatengine-x86_64.exe"), "a cheat tool is recognised by name");
    check(isCheatToolName("CheatEngine-x86_64.EXE"), "the comparison ignores case");
    check(isCheatToolName("C:\\Tools\\x64dbg\\x64dbg.exe"),
          "a program is recognised from a full path");
    check(isCheatToolName("D:/cheats/WinDbg.exe"), "either separator is accepted");
    check(!isCheatToolName("Client.exe"), "the game is not a cheat tool");
    check(!isCheatToolName("x64dbg.exe.bak"), "a name that merely starts the same is not one");
    check(!isCheatToolName(""), "an empty name is nobody");

    // Folders the logged-in user can write to without an administrator.
    check(isUserWritablePath("C:\\Users\\player\\AppData\\Local\\Temp\\a.dll"),
          "a module from a temp folder is reported");
    check(isUserWritablePath("C:\\Users\\player\\Downloads\\cheat.dll"),
          "a module from the downloads folder is reported");
    check(!isUserWritablePath("C:\\Windows\\System32\\ntdll.dll"),
          "a system module is not");
    check(!isUserWritablePath("C:\\Program Files\\Linear\\LinearGame.dll"),
          "an installed module is not");
    check(!isUserWritablePath(""), "an empty path is not reported");

    // Modules every player has, whatever machine they run.
    check(isExpectedModule("LinearGame.dll"), "the game's own library is expected");
    check(isExpectedModule("DiscordHook64.dll"), "Discord's overlay is expected");
    check(!isExpectedModule("aim.dll"), "another module is not");

    // The line shape ClientCheatReport.h reads.
    const std::string line = detectionLine("2026-11-24T07:00:00Z", "A tool was running");
    check(line == "[2026-11-24T07:00:00Z] [DETECTION] A tool was running",
          "a detection line is the shape the client already parses");
    check(linear::anticheat::isDetectionLine(line), "the client's own reader accepts it");
    check(linear::anticheat::detectionDetail(line) == "A tool was running",
          "the client reads back the message unchanged");
    const std::string multiline = detectionLine("t", "one\ntwo\rthree");
    check(multiline.find('\n') == std::string::npos && multiline.find('\r') == std::string::npos,
          "a newline in a message cannot forge a second detection");
    check(linear::anticheat::detectionDetail(multiline) == "one two three",
          "the rest of the message survives the flattening");
    check(detectionLine("t", "   ") == "[t] [DETECTION]",
          "trailing whitespace is dropped");

    // The bounded log.
    check(fitsInLog(0, 100), "the first line fits in an empty log");
    check(fitsInLog(kMaxLogBytes - 10, 9), "the last line that fits is accepted");
    check(!fitsInLog(kMaxLogBytes - 10, 10), "a line that would pass the bound is refused");
    check(!fitsInLog(0, kMaxLogBytes), "a line longer than the whole log is refused");

    std::printf("\n%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
