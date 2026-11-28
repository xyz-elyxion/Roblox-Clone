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
    return soundByName(soundName(id));
}

const Clip* soundByName(const std::string& name) {
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

    // Beside the program, then the folders this tree builds into: a dev build run
    // from build/<preset>/ finds the checkout's sounds two levels up, and one run
    // from build/<preset>/Release/ finds them three. A packaged build finds the
    // folder beside itself. Part textures are looked for the same way.
    const std::string folders[] = {gDirectory, "../../" + gDirectory,
                                   "../../../" + gDirectory};
    std::string error;
    for (const std::string& folder : folders) {
        std::unique_ptr<Clip> clip = std::make_unique<Clip>();
        if (!loadWav(soundPath(folder, name), clip.get(), &error)) {
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
