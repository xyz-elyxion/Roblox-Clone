#include "SoundBank.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace linear {
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

// --- Files ---------------------------------------------------------------

std::uint16_t read16(const unsigned char* bytes) {
    return static_cast<std::uint16_t>(bytes[0] | (bytes[1] << 8));
}

std::uint32_t read32(const unsigned char* bytes) {
    return static_cast<std::uint32_t>(bytes[0]) | (static_cast<std::uint32_t>(bytes[1]) << 8) |
           (static_cast<std::uint32_t>(bytes[2]) << 16) |
           (static_cast<std::uint32_t>(bytes[3]) << 24);
}

// A sample is two's complement, and a plain cast of the high half is not
// guaranteed to wrap, so the sign is put back by hand.
float sampleAt(const unsigned char* bytes) {
    const int raw = static_cast<int>(read16(bytes));
    return static_cast<float>(raw >= 0x8000 ? raw - 0x10000 : raw);
}

bool refuse(std::string* error, const std::string& message) {
    if (error != nullptr) {
        *error = message;
    }
    return false;
}

constexpr std::uint16_t kFormatPcm = 1;
constexpr std::uint16_t kFormatExtensible = 0xFFFE;

// Past a minute it is music, and a script asked for an effect: the whole file is
// held in memory while it plays, so the cap is what keeps a stray download from
// becoming one.
constexpr double kMaxFileSeconds = 60.0;

// The bank's state. A voice holds a bare pointer to a clip, so every clip the
// bank hands out lives until the process ends; a change of folder retires the
// ones it can no longer hand out instead of freeing them under a playing voice.
std::mutex gMutex;
std::string gDirectory = "sounds";
std::unique_ptr<Clip> gBuiltin[kSoundCount];
std::map<std::string, std::unique_ptr<Clip>> gFiles;
std::set<std::string> gMissing;
std::vector<std::unique_ptr<Clip>> gRetired;

std::string soundPath(const std::string& directory, const std::string& name) {
    if (directory.empty()) {
        return name + ".wav";
    }
    return directory + "/" + name + ".wav";
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

bool isSoundName(const std::string& name) {
    if (name.empty() || name.size() > kMaxSoundNameLength) {
        return false;
    }
    for (const char character : name) {
        const bool allowed = (character >= 'a' && character <= 'z') ||
                             (character >= 'A' && character <= 'Z') ||
                             (character >= '0' && character <= '9') || character == '-' ||
                             character == '_';
        if (!allowed) {
            return false;
        }
    }
    return true;
}

void setSoundDirectory(const std::string& directory) {
    std::lock_guard<std::mutex> lock(gMutex);
    if (gDirectory == directory) {
        return;
    }
    for (auto& entry : gFiles) {
        gRetired.push_back(std::move(entry.second));
    }
    gFiles.clear();
    gMissing.clear();
    gDirectory = directory;
}

std::string soundDirectory() {
    std::lock_guard<std::mutex> lock(gMutex);
    return gDirectory;
}

bool loadWav(const std::string& path, Clip* out, std::string* error) {
    if (out == nullptr) {
        return refuse(error, "there is no clip to fill");
    }
    out->samples.clear();

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return refuse(error, "cannot open " + path);
    }
    const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)),
                                           std::istreambuf_iterator<char>());
    if (bytes.size() < 12 || std::memcmp(bytes.data(), "RIFF", 4) != 0 ||
        std::memcmp(bytes.data() + 8, "WAVE", 4) != 0) {
        return refuse(error, path + " is not a RIFF/WAVE file");
    }

    // A WAV is a sequence of chunks, and the two that matter come in either order
    // with anything else between them (LIST, fact, cue), so the file is walked
    // rather than read as one fixed header.
    std::uint16_t format = 0;
    std::uint16_t channels = 0;
    std::uint16_t bits = 0;
    std::uint32_t rate = 0;
    const unsigned char* data = nullptr;
    std::size_t dataSize = 0;

    std::size_t offset = 12;
    while (offset + 8 <= bytes.size()) {
        const unsigned char* chunk = bytes.data() + offset;
        std::size_t size = read32(chunk + 4);
        const std::size_t body = offset + 8;
        if (size > bytes.size() - body) {
            // A file cut short mid-chunk: what arrived is still worth playing, so
            // the last chunk keeps the bytes it actually has.
            size = bytes.size() - body;
        }
        if (std::memcmp(chunk, "fmt ", 4) == 0) {
            if (size < 16) {
                return refuse(error, path + " has a truncated format chunk");
            }
            format = read16(bytes.data() + body);
            channels = read16(bytes.data() + body + 2);
            rate = read32(bytes.data() + body + 4);
            bits = read16(bytes.data() + body + 14);
            if (format == kFormatExtensible && size >= 26) {
                // The extensible form puts the real format in the first two bytes
                // of a sub-format GUID. Tools write it for plain 16-bit files too,
                // so it is read rather than refused.
                format = read16(bytes.data() + body + 24);
            }
        } else if (std::memcmp(chunk, "data", 4) == 0 && data == nullptr) {
            data = bytes.data() + body;
            dataSize = size;
        }
        offset = body + size + (size & 1u);
    }

    if (format == 0) {
        return refuse(error, path + " has no format chunk");
    }
    if (data == nullptr) {
        return refuse(error, path + " has no data chunk");
    }
    if (format != kFormatPcm) {
        return refuse(error, path + " is compressed (format " + std::to_string(format) +
                                 "); only 16-bit PCM is read");
    }
    if (bits != 16) {
        return refuse(error, path + " has " + std::to_string(bits) +
                                 "-bit samples; only 16-bit is read");
    }
    if (channels == 0 || channels > 2) {
        return refuse(error, path + " has " + std::to_string(channels) +
                                 " channels; one or two is read");
    }
    if (rate == 0) {
        return refuse(error, path + " has no sample rate");
    }

    const std::size_t bytesPerFrame = static_cast<std::size_t>(channels) * 2u;
    const std::size_t frames = dataSize / bytesPerFrame;
    if (frames == 0) {
        return refuse(error, path + " has no samples");
    }
    if (static_cast<double>(frames) / static_cast<double>(rate) > kMaxFileSeconds) {
        return refuse(error, path + " is longer than " +
                                 std::to_string(static_cast<int>(kMaxFileSeconds)) +
                                 " seconds");
    }

    const auto frame = [data, bytesPerFrame, channels](std::size_t index) {
        const unsigned char* bytesAt = data + index * bytesPerFrame;
        float value = sampleAt(bytesAt);
        if (channels == 2) {
            value = 0.5f * (value + sampleAt(bytesAt + 2));
        }
        return value;
    };

    // One format in the mixer means every file is converted here: the samples are
    // read at their own rate and written at the bank's, which is the same bytes
    // when the rates agree and a straight interpolation when they do not.
    const double ratio = static_cast<double>(rate) / static_cast<double>(kSampleRate);
    const std::size_t count = std::max<std::size_t>(
        1, static_cast<std::size_t>(std::llround(static_cast<double>(frames) / ratio)));
    out->samples.resize(count);
    for (std::size_t i = 0; i < count; ++i) {
        const double source = static_cast<double>(i) * ratio;
        const std::size_t index = std::min(static_cast<std::size_t>(source), frames - 1);
        const std::size_t next = std::min(index + 1, frames - 1);
        const float mix = static_cast<float>(source - static_cast<double>(index));
        const float value = frame(index) * (1.0f - mix) + frame(next) * mix;
        out->samples[i] =
            static_cast<std::int16_t>(std::lround(std::clamp(value, -32768.0f, 32767.0f)));
    }
    return true;
}

const Clip* sound(SoundId id) {
    const int index = static_cast<int>(id);
    if (index < 0 || index >= kSoundCount) {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(gMutex);
    std::unique_ptr<Clip>& slot = gBuiltin[index];
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
    if (!isSoundName(name)) {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(gMutex);
    const auto cached = gFiles.find(name);
    if (cached != gFiles.end()) {
        return cached->second.get();
    }
    if (gMissing.count(name) != 0) {
        return nullptr;
    }

    // The folder, then the one two levels up: a build folder is build/<preset>/, so
    // a dev build running from it finds the sounds the checkout carries, and a
    // packaged build finds the ones beside itself. Part textures are looked for
    // the same two ways.
    const std::string candidates[] = {soundPath(gDirectory, name),
                                      soundPath("../../" + gDirectory, name)};
    std::string error;
    for (const std::string& path : candidates) {
        std::unique_ptr<Clip> clip = std::make_unique<Clip>();
        if (!loadWav(path, clip.get(), &error)) {
            continue;
        }
        const Clip* stored = clip.get();
        gFiles.emplace(name, std::move(clip));
        return stored;
    }

    // Remembered, because a script asks on every heartbeat and a missing file is
    // not going to appear in the next one; setSoundDirectory forgets it.
    gMissing.insert(name);
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
}  // namespace linear
