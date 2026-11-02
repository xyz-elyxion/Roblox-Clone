#pragma once

// Getting the client's anti-cheat detections somewhere that survives the client.
//
// RBLXAntiCheat is linked into the client on purpose, but its own transport is
// switched off (nothing here speaks its protocol) and a detection is written
// only to its log file on the player's machine. The client tails that file and
// files what it finds against the signed-in account, so tampering costs the
// account instead of sitting on the cheater's own disk. A report only counts
// when it carries that account's token: with no account behind it there is
// nobody to punish, and a client that could name somebody else's account would
// be a griefing tool.
//
// The parsing and the tail offset live here rather than in Client.cpp so they
// can be tested without a Windows build or a running client.

#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

namespace rblx::anticheat {

// The library logs "[timestamp] [DETECTION] message". Everything else in the
// file is info, warning or error noise - including the errors its own disabled
// network transport produces, which are not detections and must not be reported
// as if they were.
inline bool isDetectionLine(const std::string& line) {
    return line.find("[DETECTION]") != std::string::npos;
}

// The message after the tag, with the timestamp and the tag removed and clipped
// so one bad line cannot become a megabyte of request body. An empty result
// means the line carried no message.
inline std::string detectionDetail(const std::string& line, std::size_t limit = 300) {
    const std::string tag = "[DETECTION]";
    const std::size_t at = line.find(tag);
    if (at == std::string::npos) {
        return std::string();
    }
    std::size_t start = at + tag.size();
    while (start < line.size() && (line[start] == ' ' || line[start] == '\t')) {
        ++start;
    }

    std::string detail = line.substr(start, limit);
    // Trailing whitespace is not part of the message, and dropping it keeps two
    // readings of the same detection comparable.
    while (!detail.empty() && (detail.back() == '\r' || detail.back() == '\n' ||
                               detail.back() == ' ' || detail.back() == '\t')) {
        detail.pop_back();
    }
    return detail;
}

// A detection about another process holding a handle to the client. The library
// writes this family in three shapes, all from the one scan in
// Detections::CheckOpenHandles, and only one of the three is a summary.
inline bool isHandleDetection(const std::string& detail) {
    return detail.find("referencing our process") != std::string::npos ||
           detail.find("open process handle to our process") != std::string::npos;
}

// True for a handle detection that names one of this project's own executables.
// WebApp.exe launches the client with CreateProcess and keeps the process handle
// it waits on, so a perfectly normal launch by the site leaves an open handle to
// the client and the scan reads it as an intrusion. Server.exe and Studio.exe are
// started the same way. The check is limited to handle detections on purpose:
// a modified module is still tampering even when the module is one of ours.
inline bool namesOurOwnProcesses(const std::string& detail) {
    if (!isHandleDetection(detail)) {
        return false;
    }
    static const char* const own[] = {"WebApp.exe", "Server.exe", "Client.exe", "Studio.exe"};
    for (const char* name : own) {
        if (detail.find(name) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// True for a detection that names a module an overlay injected into the client.
// Discord's hook is loaded into every game process that runs while Discord is
// open and hooks the frame the client presents, so both the module scan and the
// IAT scan have something to say about it on a machine that has never cheated.
// The publisher is allowed by the module signature check; this keeps the rest of
// the family - a checksum that reads different, an IAT entry that points into it
// - off the ladder too. It matches the module Discord actually injects by name,
// not a pattern a file could be renamed to fit.
inline bool namesOverlayModules(const std::string& detail) {
    static const char* const modules[] = {"DiscordHook64.dll", "DiscordHook32.dll", "discord_hook"};
    for (const char* name : modules) {
        if (detail.find(name) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// What a detection is filed as. One open-handle scan logs the handle, the process
// that owns it and the scan's own summary - three lines about one event. Filing
// them separately would let a single scan climb the whole warning-and-suspension
// ladder, so the whole family is reported under the summary.
inline std::string canonicalDetection(const std::string& detail) {
    if (isHandleDetection(detail)) {
        return "Found open process handles to our process from other processes";
    }
    return detail;
}

// Every detection in a chunk of log text, in the order the library wrote them.
inline std::vector<std::string> detectionDetails(const std::string& text,
                                                 std::size_t limit = 300) {
    std::vector<std::string> details;
    std::size_t start = 0;
    while (start <= text.size()) {
        const std::size_t end = text.find('\n', start);
        const std::size_t length = end == std::string::npos ? std::string::npos : end - start;
        const std::string line = text.substr(start, length);
        if (isDetectionLine(line)) {
            const std::string detail = detectionDetail(line, limit);
            if (!detail.empty()) {
                details.push_back(detail);
            }
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return details;
}

// Reads only what was appended since the last poll. The file is opened and
// closed per read because the library appends to it with its own handle, and a
// reader that holds the file open can block a rotation on Windows.
class LogTail {
public:
    static const std::size_t kMaxBytesPerRead = 256 * 1024;

    // Returns false when the file cannot be read at all. A log that does not
    // exist yet is the normal state before the first line is written, so the
    // caller treats that as "nothing to do" rather than as an error to show.
    bool poll(const std::string& path, std::vector<std::string>* lines,
              std::string* error = nullptr) {
        if (lines == nullptr) {
            return false;
        }

        std::ifstream file(path.c_str(), std::ios::binary);
        if (!file) {
            if (error != nullptr) {
                *error = "the anti-cheat log is not readable";
            }
            return false;
        }

        file.seekg(0, std::ios::end);
        const std::streamoff end = file.tellg();
        if (end < 0) {
            if (error != nullptr) {
                *error = "the anti-cheat log has no readable length";
            }
            return false;
        }

        // A file shorter than what has already been read was rotated or
        // truncated, so the old offset would skip the new contents entirely.
        const std::size_t size = static_cast<std::size_t>(end);
        if (size < offset_) {
            offset_ = 0;
        }
        if (size == offset_) {
            return true;
        }

        file.seekg(static_cast<std::streamoff>(offset_), std::ios::beg);
        std::string chunk;
        chunk.resize(kMaxBytesPerRead);
        file.read(&chunk[0], static_cast<std::streamsize>(kMaxBytesPerRead));
        const std::streamsize read = file.gcount();
        if (read <= 0) {
            return true;
        }
        chunk.resize(static_cast<std::size_t>(read));

        // Only whole lines are handed out. A half-written line stays for the
        // next poll, so a detection is never reported cut in two.
        const std::size_t lastNewline = chunk.rfind('\n');
        if (lastNewline == std::string::npos) {
            return true;
        }
        const std::string complete = chunk.substr(0, lastNewline + 1);
        offset_ += complete.size();

        std::size_t start = 0;
        while (start < complete.size()) {
            const std::size_t newline = complete.find('\n', start);
            std::string line = complete.substr(start, newline == std::string::npos ? std::string::npos
                                                                                  : newline - start);
            while (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (!line.empty()) {
                lines->push_back(line);
            }
            if (newline == std::string::npos) {
                break;
            }
            start = newline + 1;
        }
        return true;
    }

    std::size_t offset() const { return offset_; }
    void reset() { offset_ = 0; }

private:
    std::size_t offset_ = 0;
};

}  // namespace rblx::anticheat
