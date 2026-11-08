#include "SoundBank.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <mutex>

namespace rblx {
namespace audio {
namespace {

constexpr float kPi = 3.14159265358979323846f;

// Synthesis is noise and sines, so a build has to be repeatable or the test that
// compares two builds compares two different noises. A plain LCG rather than
// <random> for the same reason: the same seed is the same bytes everywhere.
class Noise {
public:
    explicit Noise(std::uint32_t seed) : state_(seed) {}

    float next() {
        state_ = state_ * 1664525u + 1013904223u;
        return static_cast<float>(state_ >> 8) * (2.0f / 16777216.0f) - 1.0f;
    }

private:
    std::uint32_t state_;
};

using Samples = std::vector<float>;

int framesFor(float seconds) {
    return static_cast<int>(seconds * static_cast<float>(kSampleRate));
}

// Converts the float mix once, at the end, with 32000 rather than 32767 as full
// scale so no sample ever sits on the clamp.
Clip toClip(const Samples& samples, float gain) {
    Clip clip;
    clip.samples.resize(samples.size());
    for (std::size_t i = 0; i < samples.size(); ++i) {
        const float value = std::clamp(samples[i] * gain, -1.0f, 1.0f);
        clip.samples[i] = static_cast<std::int16_t>(std::lround(value * 32000.0f));
    }
    return clip;
}

Clip buildFootstep() {
    Samples out(static_cast<std::size_t>(framesFor(0.07f)), 0.0f);
    Noise noise(0x51ed270bu);
    float low = 0.0f;
    for (std::size_t i = 0; i < out.size(); ++i) {
        const float t = static_cast<float>(i) / kSampleRate;
        // A step is dull, not a hiss, so the noise is smoothed before it is
        // heard, and the thump is what carries through a wall.
        low += (noise.next() - low) * 0.25f;
        out[i] = low * std::exp(-t * 55.0f) +
                 std::sin(2.0f * kPi * 90.0f * t) * std::exp(-t * 45.0f) * 0.5f;
    }
    return toClip(out, 0.5f);
}

Clip buildJump() {
    Samples out(static_cast<std::size_t>(framesFor(0.18f)), 0.0f);
    float phase = 0.0f;
    for (std::size_t i = 0; i < out.size(); ++i) {
        const float t = static_cast<float>(i) / kSampleRate;
        // Rising, because the character is: a falling pitch would read as a
        // stumble.
        const float frequency = 240.0f + 620.0f * (t / 0.18f);
        phase += 2.0f * kPi * frequency / kSampleRate;
        out[i] = std::sin(phase) * std::exp(-t * 12.0f);
    }
    return toClip(out, 0.45f);
}

Clip buildDoor() {
    const float seconds = 0.8f;
    Samples out(static_cast<std::size_t>(framesFor(seconds)), 0.0f);
    Noise noise(0x2f9a1c37u);
    float low = 0.0f;
    for (std::size_t i = 0; i < out.size(); ++i) {
        const float t = static_cast<float>(i) / kSampleRate;
        low += (noise.next() - low) * 0.08f;
        // A shove, then a rumble that swells under the slide and stops: the
        // hotel's door lifts past the player's head, it does not bang.
        const float slide = std::min(1.0f, t / 0.12f) * std::exp(-t * 4.0f);
        out[i] = low * slide * 0.9f + std::sin(2.0f * kPi * 70.0f * t) * std::exp(-t * 7.0f) * 0.4f;
    }
    return toClip(out, 0.5f);
}

Clip buildWardrobe() {
    Samples out(static_cast<std::size_t>(framesFor(0.42f)), 0.0f);
    Noise noise(0x77c1a5e3u);
    // Two swishes with a breath between them: the cloth, then the door of it
    // being pulled to.
    for (int swish = 0; swish < 2; ++swish) {
        const int start = framesFor(static_cast<float>(swish) * 0.16f);
        const int length = framesFor(0.14f);
        float low = 0.0f;
        for (int j = 0; j < length && start + j < static_cast<int>(out.size()); ++j) {
            const float u = static_cast<float>(j) / static_cast<float>(length);
            low += (noise.next() - low) * 0.35f;
            out[static_cast<std::size_t>(start + j)] += low * std::sin(kPi * u) * 0.9f;
        }
    }
    return toClip(out, 0.4f);
}

Clip buildEntity() {
    const float seconds = 1.6f;
    Samples out(static_cast<std::size_t>(framesFor(seconds)), 0.0f);
    Noise noise(0x1a2b3c4du);
    float phase = 0.0f;
    for (std::size_t i = 0; i < out.size(); ++i) {
        const float t = static_cast<float>(i) / kSampleRate;
        const float u = t / seconds;
        // A growl from below that leaves: the pitch sags, the level swells and
        // trembles at about seven cycles a second, and the noise is what makes
        // it a throat instead of an organ.
        const float frequency = 62.0f - 18.0f * u + 4.0f * std::sin(2.0f * kPi * 3.0f * t);
        phase += 2.0f * kPi * frequency / kSampleRate;
        const float cycle = phase / (2.0f * kPi);
        const float saw = 2.0f * (cycle - std::floor(cycle + 0.5f));
        const float swell = std::sin(kPi * u) * (0.75f + 0.25f * std::sin(2.0f * kPi * 7.0f * t));
        out[i] = (saw * 0.6f + noise.next() * 0.4f) * swell;
    }
    return toClip(out, 0.55f);
}

Clip buildDeath() {
    Samples out(static_cast<std::size_t>(framesFor(0.9f)), 0.0f);
    Noise noise(0x9e3779b9u);
    float phase = 0.0f;
    for (std::size_t i = 0; i < out.size(); ++i) {
        const float t = static_cast<float>(i) / kSampleRate;
        const float frequency = 320.0f * std::exp(-t * 2.2f) + 70.0f;
        phase += 2.0f * kPi * frequency / kSampleRate;
        out[i] = (std::sin(phase) * 0.7f + noise.next() * 0.3f) * std::exp(-t * 3.2f);
    }
    return toClip(out, 0.5f);
}

Clip buildLap() {
    Samples out(static_cast<std::size_t>(framesFor(0.5f)), 0.0f);
    const float notes[2] = {880.0f, 1320.0f};
    const float starts[2] = {0.0f, 0.16f};
    for (int note = 0; note < 2; ++note) {
        const int start = framesFor(starts[note]);
        for (int j = start; j < static_cast<int>(out.size()); ++j) {
            const float t = static_cast<float>(j - start) / kSampleRate;
            // An octave apart with a fifth above it: the same interval a lap
            // bell uses, and short enough to not cover the next lap.
            out[static_cast<std::size_t>(j)] +=
                (std::sin(2.0f * kPi * notes[note] * t) +
                 0.35f * std::sin(4.0f * kPi * notes[note] * t)) *
                std::exp(-t * 12.0f);
        }
    }
    return toClip(out, 0.35f);
}

Clip buildLava() {
    Samples out(static_cast<std::size_t>(framesFor(0.85f)), 0.0f);
    Noise noise(0x3c6ef372u);
    float low = 0.0f;
    for (std::size_t i = 0; i < out.size(); ++i) {
        const float t = static_cast<float>(i) / kSampleRate;
        // A moving filter is what sounds like bubbling: the noise stays, the
        // frequency it is let through at rises and falls.
        const float wobble = 0.12f + 0.10f * std::sin(2.0f * kPi * 5.0f * t);
        low += (noise.next() - low) * wobble;
        out[i] = low * 1.6f * std::exp(-t * 2.0f);
    }
    return toClip(out, 0.45f);
}

Clip buildChime() {
    Samples out(static_cast<std::size_t>(framesFor(0.8f)), 0.0f);
    // Three partials, the higher ones dying first: that is what a struck bell
    // does, and it is why this reads as a goal and not as a beep.
    const float partials[3] = {1046.5f, 1568.0f, 2093.0f};
    const float decays[3] = {4.0f, 6.0f, 9.0f};
    for (std::size_t i = 0; i < out.size(); ++i) {
        const float t = static_cast<float>(i) / kSampleRate;
        float value = 0.0f;
        for (int partial = 0; partial < 3; ++partial) {
            value += std::sin(2.0f * kPi * partials[partial] * t) * std::exp(-t * decays[partial]);
        }
        out[i] = value * 0.4f;
    }
    return toClip(out, 0.3f);
}

Clip buildElevator() {
    Samples out(static_cast<std::size_t>(framesFor(0.7f)), 0.0f);
    // The two notes of a lift arriving: low, then high, each one dying away
    // before the next, so it cannot be mistaken for the lap bell.
    const float notes[2] = {784.0f, 1046.5f};
    const float starts[2] = {0.0f, 0.24f};
    for (int note = 0; note < 2; ++note) {
        const int start = framesFor(starts[note]);
        for (int j = start; j < static_cast<int>(out.size()); ++j) {
            const float t = static_cast<float>(j - start) / kSampleRate;
            out[static_cast<std::size_t>(j)] +=
                std::sin(2.0f * kPi * notes[note] * t) * std::exp(-t * 10.0f);
        }
    }
    return toClip(out, 0.35f);
}

}  // namespace

const char* soundName(SoundId id) {
    switch (id) {
        case SoundId::Footstep: return "footstep";
        case SoundId::Jump:     return "jump";
        case SoundId::Door:     return "door";
        case SoundId::Wardrobe: return "wardrobe";
        case SoundId::Entity:   return "entity";
        case SoundId::Death:    return "death";
        case SoundId::Lap:      return "lap";
        case SoundId::Lava:     return "lava";
        case SoundId::Chime:    return "chime";
        case SoundId::Elevator: return "elevator";
        case SoundId::Count:    break;
    }
    return "";
}

Clip synthesise(SoundId id) {
    switch (id) {
        case SoundId::Footstep: return buildFootstep();
        case SoundId::Jump:     return buildJump();
        case SoundId::Door:     return buildDoor();
        case SoundId::Wardrobe: return buildWardrobe();
        case SoundId::Entity:   return buildEntity();
        case SoundId::Death:    return buildDeath();
        case SoundId::Lap:      return buildLap();
        case SoundId::Lava:     return buildLava();
        case SoundId::Chime:    return buildChime();
        case SoundId::Elevator: return buildElevator();
        case SoundId::Count:    break;
    }
    return Clip{};
}

const Clip* sound(SoundId id) {
    const int index = static_cast<int>(id);
    if (index < 0 || index >= kSoundCount) {
        return nullptr;
    }

    // A voice holds a pointer to this clip, so it lives as long as the process
    // does and is built once: a door should not allocate its own noise.
    static std::mutex mutex;
    static std::unique_ptr<Clip> cache[kSoundCount];

    std::lock_guard<std::mutex> lock(mutex);
    std::unique_ptr<Clip>& slot = cache[index];
    if (!slot) {
        slot = std::make_unique<Clip>(synthesise(id));
    }
    return slot.get();
}

const Clip* soundByName(const std::string& name) {
    if (name.empty()) {
        return nullptr;
    }
    for (int index = 0; index < kSoundCount; ++index) {
        if (name == soundName(static_cast<SoundId>(index))) {
            return sound(static_cast<SoundId>(index));
        }
    }
    return nullptr;
}

std::vector<std::string> soundNames() {
    std::vector<std::string> names;
    names.reserve(static_cast<std::size_t>(kSoundCount));
    for (int index = 0; index < kSoundCount; ++index) {
        names.emplace_back(soundName(static_cast<SoundId>(index)));
    }
    return names;
}

}  // namespace audio
}  // namespace rblx
