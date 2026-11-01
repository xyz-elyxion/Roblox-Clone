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
    using rblx::anticheat::detectionDetail;
    using rblx::anticheat::detectionDetails;
    using rblx::anticheat::isDetectionLine;
    using rblx::anticheat::LogTail;

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

    using rblx::anticheat::canonicalDetection;
    using rblx::anticheat::namesOurOwnProcesses;

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
    check(canonicalDetection("Cheat Engine is running") == "Cheat Engine is running",
          "a detection outside the handle family keeps its own text");

    std::string longLine = "[t] [DETECTION] ";
    longLine.append(400, 'x');
    check(detectionDetail(longLine).size() == 300, "a long detail is clipped to the limit");

    // Tail reading: only new, whole lines are reported, once.
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "rblx-client-cheat-report-test.log";
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
        (std::filesystem::temp_directory_path() / "rblx-client-cheat-report-missing.log").string();
    lines.clear();
    check(!tail.poll(missing, &lines), "a log that does not exist is reported as unreadable");
    check(lines.empty(), "an unreadable log produces no lines");

    std::filesystem::remove(path);

    std::printf("\n%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
