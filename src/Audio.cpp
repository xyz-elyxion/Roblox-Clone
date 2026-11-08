#include "Audio.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <mutex>
#include <thread>
#include <vector>

#include <windows.h>
#include <mmsystem.h>

// waveOut is the OS mixer every Windows copy has; the build lists link winmm.
#pragma comment(lib, "winmm.lib")

namespace rblx {
namespace audio {
namespace {

// Three short blocks, so the next one is filled while the last plays. 512 frames
// is 23 ms: short enough that a door starts on the frame it was asked for, long
// enough that the mixer wakes ~43 times a second rather than thousands.
constexpr int kBlockFrames = 512;
constexpr int kBlockCount = 3;

// Enough for a room: the door, the wardrobe, the entity and footsteps at once,
// with a couple to spare. Every voice costs one multiply per sample, so this is
// a limit on taste, not on the machine.
constexpr int kVoiceCount = 8;

struct Voice {
    const Clip* clip = nullptr;
    std::size_t cursor = 0;
    float volume = 1.0f;
    bool loop = false;
    bool active = false;
};

HWAVEOUT device = nullptr;
HANDLE wakeEvent = nullptr;
WAVEFORMATEX format{};
WAVEHDR headers[kBlockCount] = {};
std::vector<int16_t> blocks[kBlockCount];
std::thread mixer;
std::mutex voiceMutex;
Voice voices[kVoiceCount];
std::atomic<bool> running{false};
std::atomic<bool> stopping{false};
std::atomic<float> gain{0.7f};

// Mixes one block from the active voices into `blocks[index]`. Calls that
// happened while this block was being played are heard from this block on,
// which is why a sound started between two blocks is never cut in half.
void fillBlock(int index) {
    std::int16_t* out = blocks[index].data();
    std::fill(out, out + kBlockFrames, static_cast<std::int16_t>(0));

    std::lock_guard<std::mutex> lock(voiceMutex);
    const float master = gain.load();
    for (Voice& voice : voices) {
        if (!voice.active || voice.clip == nullptr) {
            continue;
        }
        const std::vector<std::int16_t>& samples = voice.clip->samples;
        if (samples.empty()) {
            voice.active = false;
            voice.clip = nullptr;
            continue;
        }

        const float level = voice.volume * master;
        for (int frame = 0; frame < kBlockFrames; ++frame) {
            if (voice.cursor >= samples.size()) {
                if (!voice.loop) {
                    voice.active = false;
                    voice.clip = nullptr;
                    break;
                }
                voice.cursor = 0;
            }
            // Clamped per sample rather than wrapped: a loud mix that clips is a
            // click, and a wrapped sample is a bang.
            const int mixed = out[frame] +
                              static_cast<int>(samples[voice.cursor] * level);
            out[frame] = static_cast<std::int16_t>(std::clamp(mixed, -32768, 32767));
            ++voice.cursor;
        }
    }
}

void mixerLoop() {
    for (int index = 0; index < kBlockCount; ++index) {
        fillBlock(index);
        headers[index].lpData = reinterpret_cast<LPSTR>(blocks[index].data());
        headers[index].dwBufferLength = kBlockFrames * sizeof(std::int16_t);
        headers[index].dwFlags = 0;
        waveOutPrepareHeader(device, &headers[index], sizeof(WAVEHDR));
        waveOutWrite(device, &headers[index], sizeof(WAVEHDR));
    }

    // The callback is an event rather than a function: the OS signals this from
    // its own thread, and mixing inside that callback would put the voice mutex
    // in the audio thread's path. The timeout only exists so shutdown is not
    // left waiting for a sound that already finished.
    while (!stopping.load()) {
        WaitForSingleObject(wakeEvent, 50);
        for (int index = 0; index < kBlockCount; ++index) {
            if (stopping.load()) {
                break;
            }
            if ((headers[index].dwFlags & WHDR_DONE) == 0) {
                continue;
            }
            fillBlock(index);
            waveOutWrite(device, &headers[index], sizeof(WAVEHDR));
        }
    }
}

}  // namespace

bool init(std::string* error) {
    if (device != nullptr) {
        return true;
    }

    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 1;
    format.wBitsPerSample = 16;
    format.nSamplesPerSec = kSampleRate;
    format.nBlockAlign = static_cast<WORD>(format.nChannels * format.wBitsPerSample / 8);
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    format.cbSize = 0;

    wakeEvent = CreateEventA(nullptr, FALSE, FALSE, nullptr);
    if (wakeEvent == nullptr) {
        if (error != nullptr) {
            *error = "could not create the audio wake event";
        }
        return false;
    }

    const MMRESULT opened = waveOutOpen(&device, WAVE_MAPPER, &format,
                                        reinterpret_cast<DWORD_PTR>(wakeEvent), 0,
                                        CALLBACK_EVENT);
    if (opened != MMSYSERR_NOERROR) {
        // The number is worth printing: waveOut's codes say which part of the
        // stack refused, and "no sound" alone sends people looking at the game.
        if (error != nullptr) {
            *error = "no audio output device (waveOutOpen error " +
                     std::to_string(static_cast<int>(opened)) + ")";
        }
        CloseHandle(wakeEvent);
        wakeEvent = nullptr;
        return false;
    }

    for (std::vector<int16_t>& block : blocks) {
        block.assign(kBlockFrames, 0);
    }
    stopping = false;
    running = true;
    mixer = std::thread(mixerLoop);
    return true;
}

bool available() { return running.load(); }

void shutdown() {
    if (device == nullptr) {
        return;
    }

    stopping = true;
    if (mixer.joinable()) {
        mixer.join();
    }
    waveOutReset(device);
    for (WAVEHDR& header : headers) {
        if ((header.dwFlags & WHDR_PREPARED) != 0) {
            waveOutUnprepareHeader(device, &header, sizeof(WAVEHDR));
        }
    }
    waveOutClose(device);
    device = nullptr;

    if (wakeEvent != nullptr) {
        CloseHandle(wakeEvent);
        wakeEvent = nullptr;
    }
    running = false;
    stopAll();
}

bool play(const Clip& clip, float volume, bool loop) {
    if (!running.load() || clip.samples.empty()) {
        return false;
    }
    const float level = std::clamp(volume, 0.0f, 1.0f);
    if (level <= 0.0f) {
        return false;
    }

    std::lock_guard<std::mutex> lock(voiceMutex);
    Voice* target = nullptr;
    for (Voice& voice : voices) {
        if (!voice.active) {
            target = &voice;
            break;
        }
    }
    if (target == nullptr) {
        // A full pool steals from the quietest sound: in a busy room that is a
        // distant whisper, not the door the player just touched.
        for (Voice& voice : voices) {
            if (target == nullptr || voice.volume < target->volume) {
                target = &voice;
            }
        }
    }
    if (target == nullptr) {
        return false;
    }

    target->clip = &clip;
    target->cursor = 0;
    target->volume = level;
    target->loop = loop;
    target->active = true;
    return true;
}

void stopAll() {
    std::lock_guard<std::mutex> lock(voiceMutex);
    for (Voice& voice : voices) {
        voice.active = false;
        voice.clip = nullptr;
        voice.cursor = 0;
    }
}

void setMasterVolume(float volume) {
    gain = std::clamp(volume, 0.0f, 1.0f);
}

float masterVolume() { return gain.load(); }

}  // namespace audio
}  // namespace rblx
