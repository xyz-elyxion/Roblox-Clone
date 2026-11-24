#pragma once

// Where the service-level anti-cheat keeps its state, and what it is doing.
//
// The guard runs as a Windows service and the client runs as the player, so the
// two only share a folder and the service manager. This unit is that shared part:
// the client reads it for the snapshot's "guard" line, and the service writes it.
// Both sides are in one file so the path cannot drift between them.

#include <string>

namespace linear::guard {

// %ProgramData%\Linear, or empty when ProgramData is not readable.
std::string folder();

// The detections the service wrote, in the shape ClientCheatReport.h reads, and
// the heartbeat it rewrites every cycle.
std::string logPath();
std::string statePath();

// Creates the folder. The service runs as LocalSystem and is usually the first
// thing to need it; the client only ever reads.
bool ensureFolder(std::string* error);

// One line for the snapshot: installed, running, and how long ago the service
// last said it was alive. Never empty, and it says so when the service manager
// cannot be read, because "the guard is not running" and "the client cannot see
// the guard" are different things to a review.
std::string serviceFacts();

}  // namespace linear::guard
