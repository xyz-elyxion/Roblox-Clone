#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

// The client's mixer: one stereo output, a handful of short clips mixed into it.
//
// A clip is mono 16-bit at kSampleRate whichever way it arrived: the bank reads
// WAV files and converts them into that shape (see SoundBank.h). Nothing here
// opens a file - the mixer only ever mixes.
namespace linear {
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

// How one voice sits between the two speakers: -1 is hard left, 0 the middle and
// +1 hard right.
//
// The gains are equal power rather than linear, so a sound sweeping from one ear
// to the other keeps the same loudness instead of dipping through the centre. A
// voice with no side - a sound the script never placed - is the middle.
struct StereoGain {
    float left = 0.70710678f;
    float right = 0.70710678f;
};

inline StereoGain panGains(float pan) {
    StereoGain gain;
    if (!std::isfinite(pan)) {
        return gain;
    }
    constexpr float kQuarterTurn = 0.25f * 3.14159265358979323846f;
    const float angle = (std::clamp(pan, -1.0f, 1.0f) + 1.0f) * kQuarterTurn;
    gain.left = std::cos(angle);
    gain.right = std::sin(angle);
    return gain;
}

// Opens the default output device and starts the thread that fills it. A machine
// without a sound card is a machine that runs the game silently, so a failure
// here is reported and the caller carries on: audio is never a reason to refuse
// to start.
bool init(std::string* error);
bool available();
void shutdown();

// Starts a clip. `clip` has to outlive the sound, which is why the bank keeps
// every clip it makes for the life of the process. `pan` is where it sits
// between the speakers, -1 left to +1 right, 0 for a sound with no place in the
// world. Returns false when there is no device or no free voice.
bool play(const Clip& clip, float volume, bool loop = false, float pan = 0.0f);

void stopAll();
void setMasterVolume(float volume);
float masterVolume();

}  // namespace audio
}  // namespace linear
