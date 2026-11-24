#pragma once

// What the service-level anti-cheat decides, kept apart from the Windows work.
//
// LinearGuard.exe runs as a service, outside the game, so the checks it makes are
// the ones that do not need to be inside the process: which programs are running
// on the machine, and where the modules loaded into the client came from. Those
// judgements are here, with no Windows call among them, so GuardChecksTests can
// answer for every name and path without a machine to look at.

#include <cstddef>
#include <string>
#include <vector>

namespace linear::guard {

// The programs the guard refuses to see running beside the game. File names only:
// a cheat tool can live anywhere, and the process list carries no folder.
// LinearAntiCheat keeps its own list for the scan inside the client
// (Core/Detections.cpp); the service is a separate executable and needs its own.
const std::vector<std::string>& cheatToolNames();

// True when the last component of `nameOrPath` is one of those programs, ignoring
// case and either separator.
bool isCheatToolName(const std::string& nameOrPath);

// Folders a program can write to without an administrator. A module the client
// loaded from one of them was put there by whoever was logged in, not installed
// by this build or by Windows.
bool isUserWritablePath(const std::string& path);

// Modules the client loads for its own reasons and every player has: the game
// library beside the executable, and the overlays that hook whichever game is
// open. Naming them keeps a clean machine out of the report.
bool isExpectedModule(const std::string& fileName);

// One line in the shape the client's log tail reads: "[stamp] [DETECTION]
// message". A message is a single line by construction - a newline in one would
// forge a second detection - so control characters become spaces.
std::string detectionLine(const std::string& stamp, const std::string& message);

// The service's log is bounded. Past this it starts again instead of growing for
// as long as the machine is on, and the client's tail notices the shorter file
// and reads it from the top.
const std::size_t kMaxLogBytes = 256 * 1024;

bool fitsInLog(std::size_t currentBytes, std::size_t lineBytes,
               std::size_t limit = kMaxLogBytes);

}  // namespace linear::guard
