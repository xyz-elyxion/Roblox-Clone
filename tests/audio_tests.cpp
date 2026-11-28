// The sound bank and the distance falloff, checked without a sound card: the
// device is only opened by audio::init(), which these tests never call.
//
// What matters here is what a client depends on: every shipped sound has a name
// that fits a packet and a file that is audible and does not clip, a file is read
// once, and a name the bank cannot find is refused instead of played as something
// else.

#include "SoundBank.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const std::string& what) {
    if (!condition) {
        std::printf("FAIL: %s\n", what.c_str());
        ++failures;
    }
}

// Panning is a stereo mixer's whole point: a sound with a place in the world is
// louder in the ear it is closer to, and a sound with no place stays in the
// middle.
void testPan() {
    const linear::audio::StereoGain centre = linear::audio::panGains(0.0f);
    check(std::fabs(centre.left - centre.right) < 1e-5f, "the middle is level");
    // Not both ears at full: an equal-power centre is each side at 1/sqrt(2), so
    // a sound sweeping across the middle does not get louder as it arrives.
    check(centre.left > 0.6f && centre.left < 0.8f, "the middle is not both ears at once");

    const linear::audio::StereoGain hardLeft = linear::audio::panGains(-1.0f);
    check(hardLeft.left > 0.999f && hardLeft.right < 0.001f, "hard left is the left ear alone");
    const linear::audio::StereoGain hardRight = linear::audio::panGains(1.0f);
    check(hardRight.right > 0.999f && hardRight.left < 0.001f, "hard right is the right ear alone");

    // Past the ends it is clamped to that side, and a broken value is the middle
    // rather than a silent voice or a NaN in the mix.
    const linear::audio::StereoGain beyond = linear::audio::panGains(4.0f);
    check(std::fabs(beyond.right - 1.0f) < 1e-5f, "past the right end stays on the right");
    const linear::audio::StereoGain broken = linear::audio::panGains(std::nanf(""));
    check(std::fabs(broken.left - broken.right) < 1e-5f, "a broken pan is the middle");

    // Equal power all the way across: the squares add up to one at every step,
    // which is what keeps a car passing the player from dipping in volume.
    for (int step = -10; step <= 10; ++step) {
        const float pan = static_cast<float>(step) / 10.0f;
        const linear::audio::StereoGain gain = linear::audio::panGains(pan);
        check(std::fabs(gain.left * gain.left + gain.right * gain.right - 1.0f) < 1e-4f,
              "equal power at pan " + std::to_string(pan));
    }
}

void testNames() {
    std::set<std::string> seen;
    for (int index = 0; index < linear::audio::kSoundCount; ++index) {
        const auto id = static_cast<linear::audio::SoundId>(index);
        const std::string name = linear::audio::soundName(id);
        check(!name.empty(), "sound " + std::to_string(index) + " has a name");
        // PacketSound.name is 24 bytes including the terminator, and a name that
        // does not fit would be cut in silence on the way out.
        check(name.size() + 1 <= 24, name + " fits a sound packet's name field");
        check(seen.insert(name).second, name + " is not a duplicate");
    }

    check(std::strlen(linear::audio::soundName(linear::audio::SoundId::Count)) == 0,
          "Count has no name of its own");
    check(linear::audio::soundNames().size() ==
              static_cast<std::size_t>(linear::audio::kSoundCount),
          "soundNames lists one name per sound");
}

// The ten names are files, so these tests need the ones the checkout carries: a
// build folder sits a few levels under it, so the folder is found by walking up
// from the tests' own directory and from their working directory, the way the
// bank itself looks beside the program and two folders up.
std::filesystem::path findSoundsFolder(const char* argv0) {
    std::error_code ignored;
    std::vector<std::filesystem::path> starts;
    starts.push_back(std::filesystem::current_path(ignored));
    starts.push_back(std::filesystem::absolute(argv0, ignored).parent_path());

    for (const std::filesystem::path& start : starts) {
        std::filesystem::path folder = start;
        for (int hop = 0; hop < 4; ++hop) {
            const std::filesystem::path candidate = folder / "sounds";
            if (std::filesystem::exists(candidate / "footstep.wav")) {
                return candidate;
            }
            folder = folder.parent_path();
            if (folder.empty()) {
                break;
            }
        }
    }
    return std::filesystem::path();
}

void testShippedSounds(const std::filesystem::path& folder) {
    check(!folder.empty(), "the sounds folder is beside the tests or a few levels above them");
    if (folder.empty()) {
        return;
    }
    linear::audio::setSoundDirectory(folder.string());

    for (int index = 0; index < linear::audio::kSoundCount; ++index) {
        const auto id = static_cast<linear::audio::SoundId>(index);
        const std::string name = linear::audio::soundName(id);

        check(std::filesystem::exists(folder / (name + ".wav")),
              name + " ships as a file in the sounds folder");

        const linear::audio::Clip* clip = linear::audio::soundByName(name);
        check(clip != nullptr, name + " resolves to a clip");
        if (clip == nullptr) {
            continue;
        }
        check(!clip->samples.empty(), name + " has samples");

        const float seconds =
            static_cast<float>(clip->samples.size()) / linear::audio::kSampleRate;
        check(seconds >= 0.05f, name + " is at least 50 ms");
        check(seconds <= 2.5f, name + " is at most 2.5 s so it cannot outlast a voice");

        int peak = 0;
        for (const std::int16_t sample : clip->samples) {
            peak = std::max(peak, std::abs(static_cast<int>(sample)));
        }
        // A file that is nearly silence would waste a voice; one on the clamp is a
        // click waiting to happen.
        check(peak >= 500, name + " is audible (peak " + std::to_string(peak) + ")");
        check(peak <= 32000,
              name + " does not reach the clamp (peak " + std::to_string(peak) + ")");
    }
}

void testLookup() {
    for (int index = 0; index < linear::audio::kSoundCount; ++index) {
        const auto id = static_cast<linear::audio::SoundId>(index);
        const std::string name = linear::audio::soundName(id);
        const linear::audio::Clip* byName = linear::audio::soundByName(name);
        const linear::audio::Clip* byId = linear::audio::sound(id);
        check(byName != nullptr, name + " resolves by name");
        check(byName == byId, name + " resolves to the same clip both ways");
        // The bank keeps the clip for the life of the process because a playing
        // voice only holds the pointer.
        check(linear::audio::sound(id) == byId, name + " is handed out as the same clip");
    }

    check(linear::audio::soundByName("") == nullptr, "an empty name resolves to nothing");
    check(linear::audio::soundByName("explosion") == nullptr,
          "a name this build does not have resolves to nothing");
    check(linear::audio::soundByName("Door") == nullptr, "names are case sensitive");
    check(linear::audio::sound(static_cast<linear::audio::SoundId>(-1)) == nullptr,
          "an id below the range resolves to nothing");
    check(linear::audio::sound(linear::audio::SoundId::Count) == nullptr,
          "Count resolves to nothing");
}

std::filesystem::path tempRoot() {
    return std::filesystem::temp_directory_path() / "linear-audio-tests";
}

void put16(std::vector<unsigned char>* out, unsigned int value) {
    out->push_back(static_cast<unsigned char>(value & 0xFFu));
    out->push_back(static_cast<unsigned char>((value >> 8) & 0xFFu));
}

void put32(std::vector<unsigned char>* out, unsigned int value) {
    for (int byte = 0; byte < 4; ++byte) {
        out->push_back(static_cast<unsigned char>((value >> (byte * 8)) & 0xFFu));
    }
}

void putTag(std::vector<unsigned char>* out, const char* tag) {
    for (int i = 0; i < 4; ++i) {
        out->push_back(static_cast<unsigned char>(tag[i]));
    }
}

// Writes the files the reader is meant to take and the ones it is meant to
// refuse: the fields it judges are the arguments, and a chunk of metadata on
// request is how a file out of a real editor arrives.
bool writeWav(const std::filesystem::path& path, int rate, int channels, int bits,
              const std::vector<std::int16_t>& samples, bool extraChunk = false) {
    std::vector<unsigned char> file;
    putTag(&file, "RIFF");
    put32(&file, 0);  // patched once the whole file is written
    putTag(&file, "WAVE");
    if (extraChunk) {
        putTag(&file, "LIST");
        put32(&file, 4);
        putTag(&file, "INFO");
    }
    putTag(&file, "fmt ");
    put32(&file, 16);
    put16(&file, 1);  // PCM
    put16(&file, static_cast<unsigned int>(channels));
    put32(&file, static_cast<unsigned int>(rate));
    put32(&file, static_cast<unsigned int>(rate * channels * bits / 8));
    put16(&file, static_cast<unsigned int>(channels * bits / 8));
    put16(&file, static_cast<unsigned int>(bits));
    putTag(&file, "data");
    put32(&file, static_cast<unsigned int>(samples.size() * static_cast<std::size_t>(bits) / 8));
    for (const std::int16_t sample : samples) {
        put16(&file, static_cast<unsigned int>(static_cast<std::uint16_t>(sample)));
    }

    const unsigned int riffSize = static_cast<unsigned int>(file.size() - 8);
    for (int byte = 0; byte < 4; ++byte) {
        file[4 + static_cast<std::size_t>(byte)] =
            static_cast<unsigned char>((riffSize >> (byte * 8)) & 0xFFu);
    }

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return false;
    }
    out.write(reinterpret_cast<const char*>(file.data()),
              static_cast<std::streamsize>(file.size()));
    return out.good();
}

void testSoundNameRules() {
    for (int index = 0; index < linear::audio::kSoundCount; ++index) {
        const auto id = static_cast<linear::audio::SoundId>(index);
        check(linear::audio::isSoundName(linear::audio::soundName(id)),
              std::string(linear::audio::soundName(id)) + " is a sound name");
    }
    check(linear::audio::isSoundName("example"), "a plain word is a sound name");
    check(linear::audio::isSoundName("door_2-open"), "digits, '_' and '-' are allowed");
    check(linear::audio::isSoundName(std::string(linear::audio::kMaxSoundNameLength, 'a')),
          "a name as long as a packet allows is a sound name");
    check(!linear::audio::isSoundName(std::string(linear::audio::kMaxSoundNameLength + 1, 'a')),
          "a name longer than the packet's field is not");
    check(!linear::audio::isSoundName(""), "an empty name is not a sound name");
    check(!linear::audio::isSoundName("file.wav"), "a name never carries its extension");
    check(!linear::audio::isSoundName("two words"), "a space is not part of a sound name");
    check(!linear::audio::isSoundName("../escape"), "a name cannot climb out of the folder");
    check(!linear::audio::isSoundName("C:absolute"), "a name cannot name a drive");
}

void testWavLoading() {
    std::filesystem::remove_all(tempRoot());
    std::filesystem::create_directories(tempRoot());

    std::vector<std::int16_t> ramp(64);
    for (int i = 0; i < 64; ++i) {
        ramp[static_cast<std::size_t>(i)] = static_cast<std::int16_t>((i - 32) * 900);
    }
    std::string error;
    linear::audio::Clip clip;

    // A file already in the bank's format comes back as it was written: no
    // resampling, no rounding, the same bytes.
    const std::filesystem::path mono = tempRoot() / "mono.wav";
    check(writeWav(mono, linear::audio::kSampleRate, 1, 16, ramp), "a mono wav is written");
    check(linear::audio::loadWav(mono.string(), &clip, &error),
          "a mono 16-bit wav loads: " + error);
    check(clip.samples == ramp, "a file at the bank's rate keeps its samples");

    // Twice the rate and two channels: halved in length, and the channels are
    // averaged rather than one of them being picked.
    std::vector<std::int16_t> stereo;
    for (int i = 0; i < 128; ++i) {
        stereo.push_back(1200);
        stereo.push_back(-1200);
    }
    const std::filesystem::path wide = tempRoot() / "wide.wav";
    check(writeWav(wide, 2 * linear::audio::kSampleRate, 2, 16, stereo, true),
          "a stereo wav with a LIST chunk is written");
    check(linear::audio::loadWav(wide.string(), &clip, &error),
          "a stereo 44.1 kHz wav loads: " + error);
    check(clip.samples.size() == 64, "twice the bank's rate is resampled to half the frames");
    bool silent = true;
    for (const std::int16_t sample : clip.samples) {
        silent = silent && sample == 0;
    }
    check(silent, "equal left and right channels cancel when mixed down");

    const std::filesystem::path eight = tempRoot() / "eight.wav";
    check(writeWav(eight, 8000, 1, 8, ramp), "an 8-bit wav is written");
    check(!linear::audio::loadWav(eight.string(), &clip, &error), "8-bit samples are refused");
    check(error.find("8-bit") != std::string::npos, "the refusal names the sample size: " + error);

    const std::filesystem::path deep = tempRoot() / "deep.wav";
    check(writeWav(deep, 8000, 1, 24, ramp), "a 24-bit wav is written");
    check(!linear::audio::loadWav(deep.string(), &clip, &error), "24-bit samples are refused");
    check(error.find("24-bit") != std::string::npos, "the refusal names the sample size: " + error);

    const std::filesystem::path empty = tempRoot() / "empty.wav";
    check(writeWav(empty, 8000, 1, 16, {}), "a wav with no samples is written");
    check(!linear::audio::loadWav(empty.string(), &clip, &error), "a file with no samples is refused");
    check(error.find("no samples") != std::string::npos, "the refusal says so: " + error);

    const std::filesystem::path junk = tempRoot() / "junk.wav";
    {
        std::ofstream out(junk, std::ios::binary);
        out << "this is not a wave file at all";
    }
    check(!linear::audio::loadWav(junk.string(), &clip, &error), "a file that is not WAVE is refused");
    check(error.find("RIFF") != std::string::npos, "the refusal says what it looked for: " + error);

    check(!linear::audio::loadWav((tempRoot() / "absent.wav").string(), &clip, &error),
          "a missing file is refused");
    check(error.find("cannot open") != std::string::npos, "the refusal says it could not open it: " + error);
}

void testFileSounds() {
    const std::filesystem::path directory = tempRoot() / "sounds";
    std::filesystem::create_directories(directory);
    std::vector<std::int16_t> tone(120, 800);
    check(writeWav(directory / "example.wav", linear::audio::kSampleRate, 1, 16, tone),
          "a game's own wav is written");

    linear::audio::setSoundDirectory(directory.string());
    check(linear::audio::soundDirectory() == directory.string(),
          "the bank reports the folder it was given");

    const linear::audio::Clip* file = linear::audio::soundByName("example");
    check(file != nullptr, "a wav in the sounds folder resolves by name");
    check(file != nullptr && file->samples == tone, "the file's samples are the clip's");
    check(linear::audio::soundByName("example") == file,
          "a file-backed clip is read once and handed out again");
    check(linear::audio::soundByName("absent") == nullptr,
          "a name with neither a built-in nor a file stays silent");
    check(linear::audio::soundByName("absent") == nullptr,
          "a missing file is not looked for twice");
    check(linear::audio::soundByName("../escape") == nullptr,
          "a name cannot reach a file outside the folder");
    // No name is privileged: the door in the folder the bank was given is the door
    // that plays, which is what makes a game's audio the game's.
    std::vector<std::int16_t> door(90, 1500);
    check(writeWav(directory / "door.wav", linear::audio::kSampleRate, 1, 16, door),
          "a game's own door.wav is written");
    const linear::audio::Clip* shipped = linear::audio::soundByName("door");
    check(shipped != nullptr && shipped->samples == door,
          "a shipped name is read from the folder the bank was given");

    linear::audio::setSoundDirectory("sounds");
    check(linear::audio::soundDirectory() == "sounds", "the folder goes back to its default");
    std::filesystem::remove_all(tempRoot());
}

void testAttenuation() {
    check(std::fabs(linear::audio::attenuation(0.0f) - 1.0f) < 1e-5f,
          "a sound at the ear is full volume");
    check(std::fabs(linear::audio::attenuation(12.0f) - 0.5f) < 1e-5f,
          "a sound 12 studs away is half volume");
    check(linear::audio::attenuation(linear::audio::kMaxAudibleDistance) == 0.0f,
          "a sound at the audible limit is silent");
    check(linear::audio::attenuation(linear::audio::kMaxAudibleDistance * 10.0f) == 0.0f,
          "a sound past the limit stays silent");
    check(linear::audio::attenuation(-5.0f) == 1.0f,
          "a negative distance is the listener's own position");
    check(linear::audio::attenuation(std::nanf("")) == 1.0f,
          "a distance that is not a number is the listener's own position");

    float previous = linear::audio::attenuation(0.0f);
    for (float distance = 1.0f; distance <= linear::audio::kMaxAudibleDistance;
         distance += 1.0f) {
        const float value = linear::audio::attenuation(distance);
        check(value <= previous, "falloff never rises with distance");
        previous = value;
    }
}

}  // namespace

int main(int argc, char** argv) {
    const std::filesystem::path sounds =
        findSoundsFolder(argc > 0 ? argv[0] : "");
    testNames();
    testPan();
    testShippedSounds(sounds);
    testLookup();
    testSoundNameRules();
    testWavLoading();
    testFileSounds();
    testAttenuation();

    if (failures == 0) {
        std::printf("audio_tests: all checks passed\n");
        return 0;
    }
    std::printf("audio_tests: %d check(s) failed\n", failures);
    return 1;
}
