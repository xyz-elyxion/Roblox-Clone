// The sound bank and the distance falloff, checked without a sound card: the
// device is only opened by audio::init(), which these tests never call.
//
// What matters here is what a client depends on: every sound has a stable name
// that fits a packet, every sound is audible and none of them clips, a build is
// repeatable, and a name the bank does not know is refused instead of playing
// something else.

#include "SoundBank.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
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

void testNames() {
    std::set<std::string> seen;
    for (int index = 0; index < rblx::audio::kSoundCount; ++index) {
        const auto id = static_cast<rblx::audio::SoundId>(index);
        const std::string name = rblx::audio::soundName(id);
        check(!name.empty(), "sound " + std::to_string(index) + " has a name");
        // PacketSound.name is 24 bytes including the terminator, and a name that
        // does not fit would be cut in silence on the way out.
        check(name.size() + 1 <= 24, name + " fits a sound packet's name field");
        check(seen.insert(name).second, name + " is not a duplicate");
    }

    check(std::strlen(rblx::audio::soundName(rblx::audio::SoundId::Count)) == 0,
          "Count has no name of its own");
    check(rblx::audio::soundNames().size() ==
              static_cast<std::size_t>(rblx::audio::kSoundCount),
          "soundNames lists one name per sound");
}

void testClips() {
    for (int index = 0; index < rblx::audio::kSoundCount; ++index) {
        const auto id = static_cast<rblx::audio::SoundId>(index);
        const std::string name = rblx::audio::soundName(id);
        const rblx::audio::Clip clip = rblx::audio::synthesise(id);

        check(!clip.samples.empty(), name + " has samples");

        const float seconds =
            static_cast<float>(clip.samples.size()) / rblx::audio::kSampleRate;
        check(seconds >= 0.05f, name + " is at least 50 ms");
        check(seconds <= 2.5f, name + " is at most 2.5 s so it cannot outlast a voice");

        int peak = 0;
        for (const std::int16_t sample : clip.samples) {
            peak = std::max(peak, std::abs(static_cast<int>(sample)));
        }
        // Quiet enough to be a background noise would be a bug in the synthesis;
        // on the clamp would be a click waiting to happen.
        check(peak >= 500, name + " is audible (peak " + std::to_string(peak) + ")");
        check(peak <= 32000, name + " does not reach the clamp (peak " +
                                 std::to_string(peak) + ")");

        const rblx::audio::Clip again = rblx::audio::synthesise(id);
        check(again.samples == clip.samples, name + " builds the same samples twice");
    }
}

void testLookup() {
    for (int index = 0; index < rblx::audio::kSoundCount; ++index) {
        const auto id = static_cast<rblx::audio::SoundId>(index);
        const std::string name = rblx::audio::soundName(id);
        const rblx::audio::Clip* byName = rblx::audio::soundByName(name);
        const rblx::audio::Clip* byId = rblx::audio::sound(id);
        check(byName != nullptr, name + " resolves by name");
        check(byName == byId, name + " resolves to the same clip both ways");
        // The bank keeps the clip for the life of the process because a playing
        // voice only holds the pointer.
        check(rblx::audio::sound(id) == byId, name + " is handed out as the same clip");
    }

    check(rblx::audio::soundByName("") == nullptr, "an empty name resolves to nothing");
    check(rblx::audio::soundByName("explosion") == nullptr,
          "a name this build does not have resolves to nothing");
    check(rblx::audio::soundByName("Door") == nullptr, "names are case sensitive");
    check(rblx::audio::sound(static_cast<rblx::audio::SoundId>(-1)) == nullptr,
          "an id below the range resolves to nothing");
    check(rblx::audio::sound(rblx::audio::SoundId::Count) == nullptr,
          "Count resolves to nothing");
}

void testAttenuation() {
    check(std::fabs(rblx::audio::attenuation(0.0f) - 1.0f) < 1e-5f,
          "a sound at the ear is full volume");
    check(std::fabs(rblx::audio::attenuation(12.0f) - 0.5f) < 1e-5f,
          "a sound 12 studs away is half volume");
    check(rblx::audio::attenuation(rblx::audio::kMaxAudibleDistance) == 0.0f,
          "a sound at the audible limit is silent");
    check(rblx::audio::attenuation(rblx::audio::kMaxAudibleDistance * 10.0f) == 0.0f,
          "a sound past the limit stays silent");
    check(rblx::audio::attenuation(-5.0f) == 1.0f,
          "a negative distance is the listener's own position");
    check(rblx::audio::attenuation(std::nanf("")) == 1.0f,
          "a distance that is not a number is the listener's own position");

    float previous = rblx::audio::attenuation(0.0f);
    for (float distance = 1.0f; distance <= rblx::audio::kMaxAudibleDistance;
         distance += 1.0f) {
        const float value = rblx::audio::attenuation(distance);
        check(value <= previous, "falloff never rises with distance");
        previous = value;
    }
}

}  // namespace

int main() {
    testNames();
    testClips();
    testLookup();
    testAttenuation();

    if (failures == 0) {
        std::printf("audio_tests: all checks passed\n");
        return 0;
    }
    std::printf("audio_tests: %d check(s) failed\n", failures);
    return 1;
}
