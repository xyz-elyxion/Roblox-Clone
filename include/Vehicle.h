#pragma once

// Arcade car for the driving game.
//
// The car is simulated by the game server: a client sends throttle and steer,
// never a position, so the same reasoning as MovementGuard applies. The
// integration is deliberately rigid-body-free. A kinematic heading with
// acceleration, drag and a speed-dependent turn rate is predictable, cheap and
// cannot be destabilised by a physics step, which matters for a car that has to
// stay on a track.
//
// Header-only and free of sockets and physics, so the driving model can be
// tested without running a server.

#include <algorithm>
#include <cmath>
#include <cstdint>

#include <glm/glm.hpp>

namespace rblx::vehicle {

struct Limits {
    float maxSpeed = 42.0f;         // units per second, forward
    float maxReverseSpeed = 14.0f;  // units per second, backward
    float acceleration = 16.0f;     // units per second squared
    float braking = 34.0f;
    float coastDeceleration = 7.0f;
    // Yaw rate in degrees per second at full steering, reduced as the car nears
    // its top speed so a keyboard can hold a line.
    float turnRateDegrees = 105.0f;
    float minTurnSpeed = 3.0f;      // a stationary car cannot pivot
    float enterRange = 9.0f;        // how close a player must stand to get in
};

// What the driver is asking for this tick.
struct Input {
    float throttle = 0.0f;  // +1 forward, -1 brake and reverse
    float steer = 0.0f;     // +1 right, -1 left

    // Clamps to -1..1 and maps a non-finite value to "no input", so a modified
    // client cannot send 1e9 and launch the car across the map.
    void set(float throttleValue, float steerValue);
};

struct State {
    glm::vec3 position{0.0f};
    float yawDegrees = 0.0f;
    float speed = 0.0f;  // signed, along the heading

    // Matches the engine's camera yaw convention (x = cos, z = sin), so yaw 0
    // points along +X and a positive yaw turns right.
    glm::vec3 forward() const {
        const float yaw = glm::radians(yawDegrees);
        return glm::vec3(std::cos(yaw), 0.0f, std::sin(yaw));
    }

    // Where the driver sits in world space, so the server can keep the
    // character in the car and every client sees it in the same place.
    glm::vec3 seatPosition(float seatHeight) const {
        return position + glm::vec3(0.0f, seatHeight, 0.0f);
    }
};

inline void Input::set(float throttleValue, float steerValue) {
    throttle = std::isfinite(throttleValue) ? std::clamp(throttleValue, -1.0f, 1.0f) : 0.0f;
    steer = std::isfinite(steerValue) ? std::clamp(steerValue, -1.0f, 1.0f) : 0.0f;
}

// Advances the car by one tick. `dt` is clamped so a stalled server tick cannot
// teleport the car across the map when it wakes up.
inline void step(State& state, const Input& input, float dt, const Limits& limits = Limits{}) {
    if (!std::isfinite(dt) || dt <= 0.0f) {
        return;
    }
    dt = std::min(dt, 0.1f);

    const float throttle = std::clamp(input.throttle, -1.0f, 1.0f);
    const float steer = std::clamp(input.steer, -1.0f, 1.0f);

    if (throttle > 0.0f) {
        state.speed += limits.acceleration * throttle * dt;
    } else if (throttle < 0.0f) {
        // S brakes while rolling forward and reverses once the car has stopped.
        const float rate = state.speed > 0.0f ? limits.braking : limits.acceleration;
        state.speed += rate * throttle * dt;
    } else {
        const float coast = limits.coastDeceleration * dt;
        if (std::abs(state.speed) <= coast) {
            state.speed = 0.0f;
        } else {
            state.speed -= state.speed > 0.0f ? coast : -coast;
        }
    }

    state.speed = std::clamp(state.speed, -limits.maxReverseSpeed, limits.maxSpeed);

    if (std::abs(state.speed) > limits.minTurnSpeed && steer != 0.0f) {
        const float speedFraction = std::min(1.0f, std::abs(state.speed) / limits.maxSpeed);
        const float sensitivity = 1.0f - 0.45f * speedFraction;
        // Reversing steers the other way, like a real car backing up.
        const float direction = state.speed >= 0.0f ? 1.0f : -1.0f;
        state.yawDegrees += steer * limits.turnRateDegrees * sensitivity * direction * dt;
    }

    if (state.yawDegrees > 180.0f) state.yawDegrees -= 360.0f;
    if (state.yawDegrees < -180.0f) state.yawDegrees += 360.0f;

    state.position += state.forward() * (state.speed * dt);
}

// Whether a player is close enough to get into a car at `carPosition`.
// Non-finite positions are never in range.
inline bool canEnter(const glm::vec3& carPosition, const glm::vec3& playerPosition,
                     float range = Limits{}.enterRange) {
    if (!std::isfinite(carPosition.x) || !std::isfinite(carPosition.y) ||
        !std::isfinite(carPosition.z) || !std::isfinite(playerPosition.x) ||
        !std::isfinite(playerPosition.y) || !std::isfinite(playerPosition.z)) {
        return false;
    }
    const glm::vec3 offset = carPosition - playerPosition;
    return glm::dot(offset, offset) <= range * range;
}

// Whole km/h for the HUD. One world unit is treated as one metre.
inline int speedKmh(float unitsPerSecond) {
    if (!std::isfinite(unitsPerSecond)) {
        return 0;
    }
    return static_cast<int>(std::lround(std::abs(unitsPerSecond) * 3.6f));
}

} // namespace rblx::vehicle
