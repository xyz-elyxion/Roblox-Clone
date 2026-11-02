#pragma once

// Server-side movement validation.
//
// The client owns its own character physics, and the anti-cheat library only
// runs inside the client process, so before this the server accepted whatever
// position a client claimed as long as it was inside the world bounds. This
// guard keeps the last accepted position and rejects a step that a player
// cannot physically have made: a horizontal hop faster than the walk speed, or
// a climb the client never fell back from.
//
// It is header-only and depends on nothing but glm so tests can run it without
// a socket, a physics world or a running server.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>

#include <glm/glm.hpp>

namespace rblx::movement {

struct Limits {
    // The client walks at 20 units/s; anything much faster is a speed hack.
    float maxHorizontalSpeed = 30.0f;
    // The jump impulse is 12 units/s. Rising faster than this is not a jump.
    float maxRiseSpeed = 14.0f;
    // Slack per packet for step-ups, interpolation and timer jitter.
    float horizontalSlack = 0.4f;
    float riseSlack = 0.75f;
    // Total climbing allowed before the player descends again. A jump peak is
    // roughly five units, so this leaves room for a jump, stairs and a ledge.
    float riseBudget = 22.0f;
    // How quickly the climb budget comes back while the player is not rising.
    float riseBudgetRefillPerSecond = 6.0f;
    // A packet gap longer than this is treated as this long: a client that
    // stalls the network for a second does not earn a second of free flight.
    float maxFrameSeconds = 0.25f;
};

// What one frame was measured against, and what the client claimed. A guard that
// only says "too fast" leaves a moderator with nothing to check, so the site
// records this beside the rule it broke and the dashboard shows both.
struct Evidence {
    float frameSeconds = 0.0f;
    // The last position the server accepted.
    glm::vec3 from{0.0f};
    // Where the client said it was.
    glm::vec3 claimed{0.0f};
    // Claimed horizontal travel this frame and the most the speed limit allows.
    float horizontal = 0.0f;
    float allowedHorizontal = 0.0f;
    // Claimed vertical travel and the most the rise limit allows.
    float rise = 0.0f;
    float allowedRise = 0.0f;
    // Climb left before the player has to descend again.
    float riseBudget = 0.0f;
    Limits limits{};
};

struct Correction {
    bool corrected = false;
    // True when the step was rejected because the client climbed without ever
    // coming down, which is a fly hack rather than a fast one.
    bool flight = false;
    glm::vec3 position{0.0f};
    // Filled for every step, corrected or not, so a caller records what was
    // accepted as easily as what was rejected.
    Evidence evidence;
};

// Two decimals: these are units and milliseconds, and a moderator reads them to
// decide whether a correction was fair.
inline std::string units(float value) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.2f", value);
    return std::string(buffer);
}

// The proof: what the client claimed, what the server had accepted before it, and
// how much of that was horizontal travel or climb.
inline std::string describeProof(const Evidence& evidence) {
    return "claimed " + units(evidence.claimed.x) + ", " + units(evidence.claimed.y) + ", " +
           units(evidence.claimed.z) + " from " + units(evidence.from.x) + ", " +
           units(evidence.from.y) + ", " + units(evidence.from.z) + " after " +
           units(evidence.frameSeconds * 1000.0f) + " ms: " + units(evidence.horizontal) +
           " units horizontal, " + units(evidence.rise) + " units of climb";
}

// The expected gameplay: the rule the frame was held against, in the same units,
// so the two strings can be read against each other.
inline std::string describeExpectation(const Evidence& evidence) {
    return "walking is " + units(evidence.limits.maxHorizontalSpeed) + " units/s with " +
           units(evidence.limits.horizontalSlack) + " units of slack, so at most " +
           units(evidence.allowedHorizontal) + " units this frame; a jump rises " +
           units(evidence.limits.maxRiseSpeed) + " units/s with " +
           units(evidence.limits.riseSlack) + " units of slack, so at most " +
           units(evidence.allowedRise) + " units, against " + units(evidence.riseBudget) +
           " units of climb budget";
}

class Guard {
public:
    // Start from a trusted position, such as the spawn or the state a client
    // sent with its first packet.
    void reset(const glm::vec3& position, uint64_t nowMs) {
        position_ = position;
        atMs_ = nowMs;
        riseBudget_ = limitsBudget_;
        primed_ = true;
    }

    // A position the server itself placed the player at (a script teleport, a
    // respawn). Later packets are measured from there.
    void teleport(const glm::vec3& position, uint64_t nowMs) { reset(position, nowMs); }

    bool primed() const { return primed_; }
    int corrections() const { return corrections_; }

    Correction validate(const glm::vec3& proposed, uint64_t nowMs) {
        Correction result;
        result.position = proposed;
        if (!primed_) {
            reset(proposed, nowMs);
            return result;
        }

        const float dt = frameSeconds(nowMs);
        const glm::vec3 delta = proposed - position_;
        const float maxHorizontal = limits_.maxHorizontalSpeed * dt + limits_.horizontalSlack;
        const float maxRise = limits_.maxRiseSpeed * dt + limits_.riseSlack;

        // Recorded whether the step is corrected or not: the proof a moderator
        // reads is this frame, not a summary of it.
        Evidence evidence;
        evidence.frameSeconds = dt;
        evidence.from = position_;
        evidence.claimed = proposed;
        evidence.horizontal = glm::length(glm::vec2(delta.x, delta.z));
        evidence.allowedHorizontal = maxHorizontal;
        evidence.rise = delta.y;
        evidence.allowedRise = maxRise;
        evidence.riseBudget = riseBudget_;
        evidence.limits = limits_;
        result.evidence = evidence;

        const float horizontal = evidence.horizontal;
        if (horizontal > maxHorizontal) {
            // Clamp along the direction of travel rather than snapping back to
            // the last position, so an honest player with a laggy packet lands
            // close to where they were going instead of being yanked backwards.
            const float scale = maxHorizontal / horizontal;
            result.position.x = position_.x + delta.x * scale;
            result.position.z = position_.z + delta.z * scale;
            result.corrected = true;
        }

        const float rise = result.position.y - position_.y;
        if (rise > 0.01f) {
            if (rise > maxRise || rise > riseBudget_) {
                const float allowed = std::max(0.0f, std::min(maxRise, riseBudget_));
                result.position.y = position_.y + allowed;
                result.corrected = true;
                // A single oversized hop is a speed hack; a step that only
                // fails because the climb budget ran out is sustained flight.
                result.flight = rise > riseBudget_;
            }
            // Spend what was actually allowed, not what was claimed: a client
            // that keeps asking for more never gets the budget back.
            riseBudget_ = std::max(0.0f, riseBudget_ - std::min(rise, std::max(0.0f, result.position.y - position_.y)));
        } else {
            riseBudget_ = std::min(limits_.riseBudget,
                                   riseBudget_ + limits_.riseBudgetRefillPerSecond * dt);
        }

        if (result.corrected) {
            ++corrections_;
        }

        position_ = result.position;
        atMs_ = nowMs;
        return result;
    }

    float riseBudget() const { return riseBudget_; }

    void setLimits(const Limits& limits) {
        limits_ = limits;
        limitsBudget_ = limits.riseBudget;
        riseBudget_ = std::min(riseBudget_, limitsBudget_);
    }

    const Limits& limits() const { return limits_; }

private:
    float frameSeconds(uint64_t nowMs) const {
        float dt = 0.0f;
        if (nowMs > atMs_) {
            dt = static_cast<float>(nowMs - atMs_) / 1000.0f;
        }
        return std::min(std::max(dt, 1.0f / 1000.0f), limits_.maxFrameSeconds);
    }

    bool primed_ = false;
    glm::vec3 position_{0.0f};
    uint64_t atMs_ = 0;
    float riseBudget_ = Limits{}.riseBudget;
    float limitsBudget_ = Limits{}.riseBudget;
    int corrections_ = 0;
    Limits limits_{};
};

} // namespace rblx::movement
