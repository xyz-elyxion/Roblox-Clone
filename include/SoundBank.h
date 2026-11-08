#pragma once

#include "Audio.h"

#include <string>
#include <vector>

// The sounds this build can make, synthesised rather than shipped as files.
//
// A name is the stable part: a script writes "door", the server forwards that
// name, and the client resolves it. A client built from other source therefore
// plays silence for a sound it does not know instead of a wrong noise, and a
// typo fails in the script that made it rather than becoming a mystery.
namespace rblx {
namespace audio {

enum class SoundId {
    Footstep,
    Jump,
    Door,
    Wardrobe,
    Entity,
    Death,
    Lap,
    Lava,
    Chime,
    Elevator,
    Count,
};

constexpr int kSoundCount = static_cast<int>(SoundId::Count);

// The name a script and a packet use. Empty for SoundId::Count.
const char* soundName(SoundId id);

// Builds a fresh clip. Deterministic: the same id is the same samples, which is
// what lets the test compare two builds.
Clip synthesise(SoundId id);

// The clip for an id, made on first use and kept for the life of the process: a
// voice only holds a pointer, so a clip must never be moved or freed while it
// could be playing. Returns nullptr for an id out of range.
const Clip* sound(SoundId id);

// The clip for a name a script or a packet carried, or nullptr when this build
// has no such sound.
const Clip* soundByName(const std::string& name);

// Every name, in enum order, for the message a script gets for a typo.
std::vector<std::string> soundNames();

}  // namespace audio
}  // namespace rblx
