#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "ClientCheatReport.h"

namespace {

int failures = 0;

void check(bool ok, const std::string& what) {
    std::printf("%s %s\n", ok ? "ok  " : "FAIL", what.c_str());
    if (!ok) ++failures;
}

void appendTo(const std::string& path, const std::string& text) {
    std::ofstream file(path.c_str(), std::ios::binary | std::ios::app);
    file << text;
}

}  // namespace

int main() {
    using linear::anticheat::detectionDetail;
    using linear::anticheat::detectionDetails;
    using linear::anticheat::isDetectionLine;
    using linear::anticheat::LogTail;

    // The shape the library writes: "[stamp] [DETECTION] message".
    const std::string detection = "[2026-11-01 10:00:00] [DETECTION] Module 'ntdll.dll' was modified";
    check(isDetectionLine(detection), "a detection line is recognised");
    check(!isDetectionLine("[2026-11-01 10:00:00] [INFO] Starting networking component..."),
          "an info line is not a detection");
    check(!isDetectionLine("[2026-11-01 10:00:00] [WARNING] Parent process is unsigned"),
          "a warning is not a detection");
    check(!isDetectionLine("[2026-11-01 10:00:00] [ERROR] NetClient was NULL @ Detections::Flag"),
          "an error from the disabled transport is not a detection");
    check(detectionDetail(detection) == "Module 'ntdll.dll' was modified",
          "the detail drops the timestamp and the tag");
    check(detectionDetail("no tag here").empty(), "a line without the tag has no detail");
    check(detectionDetail("[t] [DETECTION]    spaced   ") == "spaced", "the detail is trimmed");

    const std::string text = "[t] [INFO] started\n"
                             "[t] [DETECTION] first\n"
                             "[t] [ERROR] NetClient was NULL @ Detections::Flag\n"
                             "[t] [DETECTION] second\r\n";
    const std::vector<std::string> details = detectionDetails(text);
    check(details.size() == 2, "only detection lines are pulled out of a log");
    check(details.size() == 2 && details[0] == "first" && details[1] == "second",
          "the details keep their order and lose the line ending");

    using linear::anticheat::canonicalDetection;
    using linear::anticheat::namesOurOwnProcesses;

    // One open-handle scan writes three lines about the same handle. The site
    // launcher is one of the processes it names, and launching the client is not
    // a detection at all.
    const std::string perHandle =
        "[t] [DETECTION] Handle 12 from process 29916 is referencing our process.";
    const std::string owningProcess =
        "[t] [DETECTION] Process WebApp.exe has open process handle to our process.";
    const std::string scanSummary =
        "[t] [DETECTION] Found open process handles to our process from other processes";
    check(canonicalDetection(detectionDetail(perHandle)) ==
              canonicalDetection(detectionDetail(scanSummary)),
          "the handle and the scan summary are filed as one detection");
    check(canonicalDetection(detectionDetail(owningProcess)) ==
              canonicalDetection(detectionDetail(scanSummary)),
          "the owning process is filed as the same detection");
    check(namesOurOwnProcesses(detectionDetail(owningProcess)),
          "a handle from our own launcher is not the player's doing");
    check(namesOurOwnProcesses("[t] [DETECTION] Process Server.exe has open process handle "
                               "to our process."),
          "the game server this project starts is ours too");
    check(!namesOurOwnProcesses("[t] [DETECTION] Process cheatengine-x86_64.exe has open "
                                "process handle to our process."),
          "a handle from anybody else is still reported");
    check(!namesOurOwnProcesses("[t] [DETECTION] Module 'Client.exe' was modified"),
          "tampering with our own module is still reported");
    using linear::anticheat::namesOverlayModules;
    const std::string discordHook =
        "[t] [DETECTION] Module 'C:\\Users\\player\\AppData\\Local\\Discord\\app-1.0.9266"
        "\\modules\\discord_hook-1\\discord_hook\\ff98341dfafc82\\DiscordHook64.dll' was modified";
    check(namesOverlayModules(detectionDetail(discordHook)),
          "a detection naming Discord's hook is dismissed");
    check(namesOverlayModules("IAT was hooked! DiscordHook64.dll"),
          "a hooked IAT that points into Discord's hook is dismissed");
    check(!namesOverlayModules("Module 'ntdll.dll' was modified"),
          "a detection naming a system module is kept");
    check(!namesOverlayModules("Cheat Engine is running"),
          "a detection that names no module is kept");

    check(canonicalDetection("Cheat Engine is running") == "Cheat Engine is running",
          "a detection outside the handle and module families keeps its own text");

    // The module scan writes one line per module it distrusts, all at start-up.
    const std::string firstModule =
        "[t] [DETECTION] Module C:\\WINDOWS\\system32\\wbem\\wbemprox.dll is not signed "
        "by a trusted publisher";
    const std::string secondModule =
        "[t] [DETECTION] Module C:\\WINDOWS\\SYSTEM32\\GLU32.dll is not signed by a "
        "trusted publisher";
    check(canonicalDetection(detectionDetail(firstModule)) ==
              canonicalDetection(detectionDetail(secondModule)),
          "two modules from one scan are filed as one detection");
    check(canonicalDetection(detectionDetail(firstModule)) != detectionDetail(firstModule),
          "the module family does not keep the path of the module it names");
    check(canonicalDetection("Module 'ntdll.dll' was modified") ==
              "Module 'ntdll.dll' was modified",
          "a modified module is still its own detection");

    // What the check named travels in the snapshot, because the filed text does
    // not: the module scan's label is the same for every module it distrusts.
    using linear::anticheat::subjectFromDetection;
    const linear::anticheat::SnapshotSubject moduleSubject = subjectFromDetection(
        "Module C:\\WINDOWS\\system32\\wbem\\wbemprox.dll is not signed by a trusted "
        "publisher");
    check(moduleSubject.modulePath == "C:\\WINDOWS\\system32\\wbem\\wbemprox.dll",
          "the module scan's subject is the path it named");
    check(moduleSubject.processName.empty() && moduleSubject.processId == 0 &&
              moduleSubject.windowTitle.empty(),
          "a module subject names nothing else");
    check(subjectFromDetection("Checksum for module LinearGame.dll, section .text is "
                               "different, tampering detected")
                  .modulePath == "LinearGame.dll",
          "a checksum detection names its module");
    check(subjectFromDetection("Hooked IAT detected: kernel32.dll at 7FFB2C10").modulePath ==
              "kernel32.dll",
          "a hooked IAT names the module it points into");
    check(subjectFromDetection("Process cheatengine-x86_64.exe has open process handle to "
                               "our process.")
                  .processName == "cheatengine-x86_64.exe",
          "a handle detection names the process behind it");
    check(subjectFromDetection("Handle 12 from process 29916 is referencing our process.")
                  .processId == 29916,
          "a handle detection carries the pid behind the handle");
    check(subjectFromDetection("Handle ABC from process 12 is referencing our process.")
                  .processId == 12,
          "a non-numeric handle value does not stop the pid being read");
    check(subjectFromDetection("Blacklisted process was spawned: C:\\cheats\\aim.exe")
                  .processName == "C:\\cheats\\aim.exe",
          "a spawned blacklisted process is named as it was logged");
    check(subjectFromDetection("Blacklisted process was found through byte signature: "
                               "C:\\cheats\\aim.exe")
                  .processName == "C:\\cheats\\aim.exe",
          "a byte-signature match names the process it scanned");
    check(subjectFromDetection("Detected a window with text 'Cheat Engine 7.5' (includes "
                               "open folder names)")
                  .windowTitle == "Cheat Engine 7.5",
          "a window detection names the text it looked for");
    check(subjectFromDetection("Hypervisor was present with vendor: KVM").empty(),
          "a detection that names nothing carries no subject");
    check(subjectFromDetection("Module 'ntdll.dll' was modified").empty(),
          "the modified-module line keeps its name in the filed text instead");

    // The guard service writes outside the process and its lines are read from the
    // same tail; they have to anchor the snapshot the same way.
    const linear::anticheat::SnapshotSubject runningSubject =
        subjectFromDetection("Process aim.exe (pid 4321) is running on this machine");
    check(runningSubject.processName == "aim.exe",
          "the guard's running-program line names the program");
    check(runningSubject.processId == 4321, "and carries the pid it named");
    const linear::anticheat::SnapshotSubject guardedModule = subjectFromDetection(
        "Module C:\\Users\\player\\AppData\\Local\\Temp\\a.dll was loaded into the game "
        "from a folder any program can write to");
    check(guardedModule.modulePath == "C:\\Users\\player\\AppData\\Local\\Temp\\a.dll",
          "the guard's module line names the path it was loaded from");

    std::string longLine = "[t] [DETECTION] ";
    longLine.append(400, 'x');
    check(detectionDetail(longLine).size() == 300, "a long detail is clipped to the limit");

    // Tail reading: only new, whole lines are reported, once.
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "linear-client-cheat-report-test.log";
    {
        std::ofstream file(path.string().c_str(), std::ios::binary | std::ios::trunc);
        file << "[t] [DETECTION] one\n";
    }

    LogTail tail;
    std::vector<std::string> lines;
    check(tail.poll(path.string(), &lines), "the log is readable");
    check(lines.size() == 1 && detectionDetail(lines[0]) == "one",
          "the first poll reads the line that was already there");
    check(tail.offset() > 0, "the tail remembers how far it read");

    lines.clear();
    check(tail.poll(path.string(), &lines) && lines.empty(),
          "a second poll with nothing appended reads nothing");

    appendTo(path.string(), "[t] [DETECTION] two");
    lines.clear();
    check(tail.poll(path.string(), &lines) && lines.empty(),
          "a line that is still being written waits for its newline");

    appendTo(path.string(), " done\n");
    lines.clear();
    check(tail.poll(path.string(), &lines) && lines.size() == 1,
          "the completed line is reported once");
    check(!lines.empty() && detectionDetail(lines[0]) == "two done",
          "the completed line is reported whole");

    {
        std::ofstream file(path.string().c_str(), std::ios::binary | std::ios::trunc);
        file << "[t] [DETECTION] rotated\n";
    }
    lines.clear();
    check(tail.poll(path.string(), &lines) && lines.size() == 1,
          "a log that shrank is read again from the start");

    const std::string missing =
        (std::filesystem::temp_directory_path() / "linear-client-cheat-report-missing.log").string();
    lines.clear();
    check(!tail.poll(missing, &lines), "a log that does not exist is reported as unreadable");
    check(lines.empty(), "an unreadable log produces no lines");

    std::filesystem::remove(path);

    std::printf("\n%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
