#pragma once

// A short recording of where a player's client said it was, and where the server
// put it, around a detection.
//
// The movement guard reports one frame, and one frame shows a hop without showing
// the approach to it: sixty corrections in a second look the same whether the
// player was standing still or already running. Keeping the last second and a
// half of packets lets a report carry the run-up, and lets the dashboard draw the
// path an admin needs to judge an appeal.
//
// Deliberately free of rblxmath: the game server has vectors to spare, but the website
// reads these clips too and has no business linking a renderer to do it.

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace rblx::replay {

// The three numbers of a position, kept apart from any vector type so both sides
// can hand one over without agreeing on a maths library.
struct Point {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Frame {
    uint64_t atMs = 0;
    // What the client sent.
    Point claimed;
    // Where the server put the player.
    Point accepted;
};

// Ninety packets is about a second and a half at a client tick rate: long enough
// to see the run-up to a hop, short enough that a record stays small.
inline const std::size_t kMaxFrames = 90;

class Recorder {
public:
    void reset() { frames_.clear(); }

    void add(uint64_t atMs, const Point& claimed, const Point& accepted) {
        Frame frame;
        frame.atMs = atMs;
        frame.claimed = claimed;
        frame.accepted = accepted;
        frames_.push_back(frame);
        // The run-up to a detection is the recent past, so the oldest frame is the
        // one worth dropping.
        if (frames_.size() > kMaxFrames) {
            frames_.erase(frames_.begin(),
                          frames_.begin() + static_cast<std::ptrdiff_t>(frames_.size() - kMaxFrames));
        }
    }

    const std::vector<Frame>& frames() const { return frames_; }
    bool empty() const { return frames_.empty(); }

    // "1234:1.0,2.0,3.0:1.0,2.0,3.0|..." - the format decode reads. A tenth of a
    // unit is finer than any correction worth judging, and it keeps a full record
    // small enough to store with the action.
    std::string encode() const {
        std::string out;
        for (const Frame& frame : frames_) {
            out += std::to_string(frame.atMs);
            out += ':';
            out += point(frame.claimed);
            out += ':';
            out += point(frame.accepted);
            out += '|';
        }
        if (!out.empty()) {
            out.pop_back();
        }
        return out;
    }

private:
    static std::string point(const Point& value) {
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%.1f,%.1f,%.1f", value.x, value.y, value.z);
        return std::string(buffer);
    }

    std::vector<Frame> frames_;
};

// Reads what encode wrote. A record cut short by a size limit loses its last
// frame and nothing else, so a malformed entry ends the read rather than dropping
// the frames already read.
inline std::vector<Frame> decode(const std::string& text) {
    std::vector<Frame> frames;
    std::size_t start = 0;
    while (start < text.size() && frames.size() < kMaxFrames) {
        const std::size_t end = text.find('|', start);
        const std::string entry =
            text.substr(start, end == std::string::npos ? std::string::npos : end - start);

        unsigned long long atMs = 0;
        char claimed[64] = {0};
        char accepted[64] = {0};
        Frame frame;
        if (std::sscanf(entry.c_str(), "%llu:%63[^:]:%63[^:]", &atMs, claimed, accepted) != 3 ||
            std::sscanf(claimed, "%f,%f,%f", &frame.claimed.x, &frame.claimed.y,
                        &frame.claimed.z) != 3 ||
            std::sscanf(accepted, "%f,%f,%f", &frame.accepted.x, &frame.accepted.y,
                        &frame.accepted.z) != 3) {
            break;
        }
        frame.atMs = static_cast<uint64_t>(atMs);
        frames.push_back(frame);
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return frames;
}

} // namespace rblx::replay
