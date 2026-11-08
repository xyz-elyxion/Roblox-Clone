#pragma once

#include <cstdint>
#include <string>
#include <vector>

// The client's mixer: one mono output, a handful of short clips mixed into it.
//
// There are no audio files in the repository, and nothing here loads one. The
// clips are synthesised (see SoundBank.h), which keeps the package a folder of
// executables rather than an asset bundle, and keeps a sound from ever shipping
// out of step with the code that names it.
namespace rblx {
namespace audio {

// Every clip is mono 16-bit at this rate. One fixed format means mixing is a
// multiply and an add, and no clip has to be resampled when it starts.
constexpr int kSampleRate = 22050;

// Sounds are cut off past this distance: a door a hotel away is worth nothing
// and still costs a voice.
constexpr float kMaxAudibleDistance = 90.0f;

struct Clip {
    std::vector<int16_t> samples;  // mono, 16-bit, kSampleRate
};

// How loud something `distance` studs away is: 1 at the ear, half at 12, a
// quarter at 36, nothing past kMaxAudibleDistance. A non-positive or NaN
// distance is the listener's own position and stays at full volume, because a
// sound the script never placed has no distance to fall off.
inline float attenuation(float distance) {
    if (!(distance > 0.0f)) {
        return 1.0f;
    }
    if (!(distance < kMaxAudibleDistance)) {
        return 0.0f;
    }
    return 12.0f / (12.0f + distance);
}

// Opens the default output device and starts the thread that fills it. A machine
// without a sound card is a machine that runs the game silently, so a failure
// here is reported and the caller carries on: audio is never a reason to refuse
// to start.
bool init(std::string* error);
bool available();
void shutdown();

// Starts a clip. `clip` has to outlive the sound, which is why the bank keeps
// every clip it makes for the life of the process. Returns false when there is
// no device or no free voice.
bool play(const Clip& clip, float volume, bool loop = false);

void stopAll();
void setMasterVolume(float volume);
float masterVolume();

}  // namespace audio
}  // namespace rblx
