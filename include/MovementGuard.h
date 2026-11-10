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
// It is header-only and depends on nothing but rblxmath so tests can run it without
// a socket, a physics world or a running server.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>

#include <rblxmath/rblxmath.hpp>

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
    // A gap the client's own clock agrees with is believed in full, up to this:
    // a player whose game really did stall for a second walked for a second.
    float maxCorroboratedFrameSeconds = 1.0f;
    // How far ahead of the server's arrival gap the client's stamp may run
    // before it stops being believed.
    float clientStampToleranceMs = 120.0f;
};

// What one frame was measured against, and what the client claimed. A guard that
// only says "too fast" leaves a moderator with nothing to check, so the site
// records this beside the rule it broke and the dashboard shows both.
struct Evidence {
    float frameSeconds = 0.0f;
    // The last position the server accepted.
    rblxmath::vec3 from{0.0f};
    // Where the client said it was.
    rblxmath::vec3 claimed{0.0f};
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
    rblxmath::vec3 position{0.0f};
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
    void reset(const rblxmath::vec3& position, uint64_t nowMs) {
        position_ = position;
        atMs_ = nowMs;
        clientMs_ = 0;
        haveClient_ = false;
        riseBudget_ = limitsBudget_;
        primed_ = true;
    }

    // A position the server itself placed the player at (a script teleport, a
    // respawn). Later packets are measured from there.
    void teleport(const rblxmath::vec3& position, uint64_t nowMs) { reset(position, nowMs); }

    bool primed() const { return primed_; }
    int corrections() const { return corrections_; }

    // Without a client stamp, the gap is the server's own arrival time.
    Correction validate(const rblxmath::vec3& proposed, uint64_t nowMs) {
        return validate(proposed, nowMs, 0);
    }

    // With the client's clock (monotonic milliseconds). The stamp is what makes
    // bursty UDP harmless: several packets that land in one server tick are
    // still measured over the time the player actually moved.
    Correction validate(const rblxmath::vec3& proposed, uint64_t arrivalMs, uint32_t clientMs) {
        Correction result;
        result.position = proposed;
        if (!primed_) {
            reset(proposed, arrivalMs);
            clientMs_ = clientMs;
            haveClient_ = clientMs != 0;
            return result;
        }

        const float dt = frameSeconds(arrivalMs, clientMs);
        const rblxmath::vec3 delta = proposed - position_;
        const float maxHorizontal = limits_.maxHorizontalSpeed * dt + limits_.horizontalSlack;
        const float maxRise = limits_.maxRiseSpeed * dt + limits_.riseSlack;

        // Recorded whether the step is corrected or not: the proof a moderator
        // reads is this frame, not a summary of it.
        Evidence evidence;
        evidence.frameSeconds = dt;
        evidence.from = position_;
        evidence.claimed = proposed;
        evidence.horizontal = rblxmath::length(rblxmath::vec2(delta.x, delta.z));
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
        if (rise > 0.01f && (rise > maxRise || rise > riseBudget_)) {
            const float allowed = std::max(0.0f, std::min(maxRise, riseBudget_));
            result.position.y = position_.y + allowed;
            result.corrected = true;
            // A single oversized hop is a speed hack; a step that only fails
            // because the climb budget ran out is sustained flight.
            result.flight = rise > riseBudget_;
        }

        // Spend what was actually allowed, not what was claimed, so a client
        // that keeps asking for more never gets the budget back. Coming down
        // pays the climb back, which is what stops a player who keeps jumping
        // from running out and being reported as flying, and time refills it
        // too so walking up a staircase never runs a hole in it either.
        const float climbed = std::max(0.0f, result.position.y - position_.y);
        const float descended = std::max(0.0f, position_.y - result.position.y);
        riseBudget_ = std::min(limits_.riseBudget,
                               riseBudget_ - climbed + limits_.riseBudgetRefillPerSecond * dt +
                                   descended);
        riseBudget_ = std::max(0.0f, riseBudget_);

        if (result.corrected) {
            ++corrections_;
        }

        position_ = result.position;
        atMs_ = arrivalMs;
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
    // How much time this step is credited with. The arrival gap is the floor and
    // the client's stamp is used when the two agree, which is what tells a
    // player who moved for 16 ms and had three packets land in one tick apart
    // from a client claiming 16 ms of travel in a single datagram.
    float frameSeconds(uint64_t arrivalMs, uint32_t clientMs) {
        const float arrived = secondsBetween(atMs_, arrivalMs);
        float measured = arrived;
        bool corroborated = false;
        if (haveClient_ && clientMs != 0 && clientMs > clientMs_) {
            const float claimed = static_cast<float>(clientMs - clientMs_) / 1000.0f;
            if (claimed <= arrived + limits_.clientStampToleranceMs / 1000.0f) {
                measured = claimed;
                corroborated = true;
            }
        }

        if (clientMs != 0) {
            clientMs_ = clientMs;
            haveClient_ = true;
        }

        // Long gaps are believed only when both clocks agree on them: a lone
        // datagram claiming a quarter of a second after a millisecond on the
        // wire is held to the millisecond.
        const float cap = corroborated ? limits_.maxCorroboratedFrameSeconds
                                       : limits_.maxFrameSeconds;
        return std::min(std::max(measured, 1.0f / 1000.0f), cap);
    }

    static float secondsBetween(uint64_t fromMs, uint64_t toMs) {
        if (toMs <= fromMs) return 0.0f;
        return static_cast<float>(toMs - fromMs) / 1000.0f;
    }

    bool primed_ = false;
    rblxmath::vec3 position_{0.0f};
    uint64_t atMs_ = 0;
    uint32_t clientMs_ = 0;
    bool haveClient_ = false;
    float riseBudget_ = Limits{}.riseBudget;
    float limitsBudget_ = Limits{}.riseBudget;
    int corrections_ = 0;
    Limits limits_{};
};

} // namespace rblx::movement
