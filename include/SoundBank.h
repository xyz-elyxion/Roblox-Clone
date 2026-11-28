#pragma once

#include "Audio.h"

#include <cstddef>
#include <string>
#include <vector>

// The sounds this build can make.
//
// A sound is a file: <name>.wav in the sounds folder. The name is the stable
// part - a script writes "door", the server forwards that name, and the client
// resolves it - so a game replaces a sound by replacing the file, with no
// rebuild, and a client without that file plays silence instead of a wrong
// noise. A typo fails in the script that made it rather than becoming a mystery.
//
// The ten names below are the ones the client and the bundled games ask for, and
// this repository carries a file for each of them. Nothing here synthesises
// audio: the bank opens files and converts them into the mixer's one format.
namespace linear {
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

// The longest name the bank takes. A name travels in PacketSound's 24-byte field
// (see Network.h), so a longer one would be cut in silence on the way out.
constexpr std::size_t kMaxSoundNameLength = 23;

// Whether `name` can be a sound at all: letters, digits, '-' and '_' only, with
// no extension and no directory. The strict rule is what stops a name in a
// published script from reading a file outside the sounds folder.
bool isSoundName(const std::string& name);

// Where the files are looked for. "sounds" - beside the program, the way shaders/
// and textures/ are - unless a program names another folder before it plays
// anything. A build folder is two levels under the repository, so the folder two
// levels up is tried too, exactly as a part texture is: a dev build run from
// build/<preset>/ finds the sounds the checkout carries.
void setSoundDirectory(const std::string& directory);
std::string soundDirectory();

// Reads a WAV file into the bank's one format: 16-bit PCM, mono or stereo, at any
// sample rate, mixed down and resampled to mono 16-bit at kSampleRate. Returns
// false and says what the file was instead when it is something else - 8- or
// 24-bit samples, a compressed format, more than two channels, or more than a
// minute of audio, which is a song rather than the noise a script asked for.
bool loadWav(const std::string& path, Clip* out, std::string* error);

// The name a script and a packet use, and the name of the file in the sounds
// folder. Empty for SoundId::Count.
const char* soundName(SoundId id);

// The clip for an id, read on first use and kept for the life of the process: a
// voice only holds a pointer, so a clip must never be moved or freed while it
// could be playing. Returns nullptr for an id out of range or a file that is not
// in the sounds folder.
const Clip* sound(SoundId id);

// The clip for a name a script or a packet carried: <name>.wav in the sounds
// folder. nullptr when there is no such file.
const Clip* soundByName(const std::string& name);

// The ten names this build ships a file for, in enum order, for the message a
// script gets for a typo.
std::vector<std::string> soundNames();

}  // namespace audio
}  // namespace linear
