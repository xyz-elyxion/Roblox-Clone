#pragma once

// What the client sends staff when its anti-cheat reports a suspicion.
//
// A detection line names the check and nothing else: enough to file a strike, not
// enough to tell an intrusion from a false positive. The snapshot is the machine
// around the detection - the operating system, this process, every module the
// client has loaded and every process running beside it. A review reads that and
// sees the injector or the trainer the check tripped over, or sees that a
// signature false positive came from a clean machine.
//
// The check that fired usually named what it found - a module path, a process, a
// window title - and that name is folded away before the detection reaches the
// site, because one scan that names six modules is one finding on the ladder. A
// SnapshotSubject carries the name here, so the snapshot can put it first and say
// where it is now, instead of leaving staff to look for it in a list the
// per-machine cap may already have cut.
//
// It is plain text on purpose. Staff read it in the admin console, it is stored
// with the detection, and text survives being pasted into a report unchanged. It
// is bounded, because a machine with four hundred processes must not turn one
// detection into a megabyte of data file, and it carries no file contents, no
// credential and no environment variable - only names, ids and sizes.
//
// Windows-only, like everything else here; the builder is in
// src/SystemSnapshot.cpp and the caller gets a std::string it can put in a
// request body.

#include <cstddef>
#include <string>

namespace linear {
namespace anticheat {

struct SnapshotLimits {
    // Processes listed, in the order Windows hands them over. A cheat's process
    // runs while the client does, so a bounded window is enough for a review.
    std::size_t maxProcesses = 200;
    // Modules the client has loaded.
    std::size_t maxModules = 60;
    // Ceiling on the whole snapshot. The text is JSON-escaped before it travels
    // and every path doubles its backslashes on the way, so this stays well under
    // the website's kMaxSnapshotCharacters.
    std::size_t maxBytes = 8000;
};

// The website's limit for one snapshot, in characters of decoded text. Kept here
// so the client and the site agree without the site having to tell the client.
const std::size_t kMaxSnapshotCharacters = 12000;

// What the detection line named, when it named anything. Every field is optional
// and every field comes from the log the reviewed machine writes, so none of it
// is trusted: the snapshot treats it as a claim to look up, not as a fact.
struct SnapshotSubject {
    // A module the check named - a full path from the module scan, or a bare
    // module name from the checksum and IAT checks.
    std::string modulePath;
    // A process the check named, by name or by full path.
    std::string processName;
    // The pid behind an open handle, when the line carried one.
    unsigned long processId = 0;
    // Window text a check looked for.
    std::string windowTitle;

    bool empty() const {
        return modulePath.empty() && processName.empty() && processId == 0 &&
               windowTitle.empty();
    }
};

// A description of this machine and this process: one "key: value" line per
// fact, the subject the check named if there is one, then the module and process
// lists. Never empty - a fact that cannot be read leaves a line saying so, so
// staff can tell "clean" from "the client could not look".
std::string systemSnapshot(const SnapshotLimits& limits = SnapshotLimits(),
                           const SnapshotSubject& subject = SnapshotSubject());

}  // namespace anticheat
}  // namespace linear
