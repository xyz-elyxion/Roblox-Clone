// Tests for the server-side movement guard. The guard only depends on glm, so
// these run without a socket, a physics world or a running server.

#include "MovementGuard.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>

namespace {
int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << std::endl;
        ++failures;
    }
}

bool near(float a, float b, float epsilon = 0.001f) {
    return std::fabs(a - b) <= epsilon;
}

// Walks at the client's 20 units/s while sending a packet every 16ms.
void testWalkingAllowed() {
    rblx::movement::Guard guard;
    glm::vec3 position(0.0f, 5.0f, 0.0f);
    guard.reset(position, 0);
    bool corrected = false;
    uint64_t now = 0;
    for (int step = 0; step < 120; ++step) {
        position.x += 20.0f / 60.0f;
        now += 16;
        const auto result = guard.validate(position, now);
        corrected = corrected || result.corrected;
        position = result.position;
    }
    check(!corrected, "a 20 unit/s walk is accepted");
    check(near(position.x, 40.0f, 0.5f), "the accepted walk keeps its position");
}

void testSpeedHackClamped() {
    rblx::movement::Guard guard;
    guard.reset(glm::vec3(0.0f), 0);
    const auto result = guard.validate(glm::vec3(1000.0f, 0.0f, 0.0f), 16);
    check(result.corrected, "a 1000 unit step in one frame is corrected");
    check(result.position.x < 2.0f, "the corrected step is clamped to the allowed envelope");
    check(!result.flight, "a flat speed hack is not reported as flight");
}

// A jump: 12 units/s upward, decaying, never climbing more than ~5 units.
void testJumpArcAllowed() {
    rblx::movement::Guard guard;
    guard.reset(glm::vec3(0.0f), 0);
    bool corrected = false;
    float y = 0.0f;
    float velocity = 12.0f;
    uint64_t now = 0;
    for (int step = 0; step < 60; ++step) {
        const float dt = 0.05f;
        velocity -= 20.0f * dt;
        y = std::max(0.0f, y + velocity * dt);
        now += 50;
        const auto result = guard.validate(glm::vec3(0.0f, y, 0.0f), now);
        corrected = corrected || result.corrected;
        y = result.position.y;
    }
    check(!corrected, "a normal jump arc is accepted");
}

void testFlightClamped() {
    rblx::movement::Guard guard;
    guard.reset(glm::vec3(0.0f), 0);
    bool corrected = false;
    bool flight = false;
    float y = 0.0f;
    uint64_t now = 0;
    for (int step = 0; step < 240; ++step) {
        now += 16;
        y += 8.0f * 0.016f; // quietly floating upward at 8 units/s
        const auto result = guard.validate(glm::vec3(0.0f, y, 0.0f), now);
        corrected = corrected || result.corrected;
        flight = flight || result.flight;
        y = result.position.y;
    }
    check(corrected, "sustained climbing is eventually corrected");
    check(flight, "sustained climbing is reported as flight");
}

void testTeleportBecomesBaseline() {
    rblx::movement::Guard guard;
    guard.reset(glm::vec3(0.0f), 0);
    guard.teleport(glm::vec3(500.0f, 40.0f, 0.0f), 16);
    const auto result = guard.validate(glm::vec3(500.0f, 40.0f, 0.0f), 32);
    check(!result.corrected, "a server teleport becomes the new baseline");
}

void testLongStallDoesNotGrantFlight() {
    rblx::movement::Guard guard;
    guard.reset(glm::vec3(0.0f), 0);
    // Five seconds between packets must not buy five seconds of free climb.
    const auto result = guard.validate(glm::vec3(0.0f, 40.0f, 0.0f), 5000);
    check(result.corrected, "a stalled client cannot claim a 40 unit climb");
    check(result.position.y < 10.0f, "the stall correction is capped by the frame limit");
}

void testFallingAllowed() {
    rblx::movement::Guard guard;
    guard.reset(glm::vec3(0.0f, 100.0f, 0.0f), 0);
    const auto result = guard.validate(glm::vec3(0.0f, 0.0f, 0.0f), 16);
    check(!result.corrected, "falling fast is not a correction");
}

void testRecoveryAfterCorrection() {
    rblx::movement::Guard guard;
    guard.reset(glm::vec3(0.0f), 0);
    const auto bad = guard.validate(glm::vec3(200.0f, 0.0f, 0.0f), 16);
    check(bad.corrected, "the speed hack is corrected");
    // The next honest packet continues from the corrected position, not from 200.
    const auto good = guard.validate(bad.position + glm::vec3(0.3f, 0.0f, 0.0f), 32);
    check(!good.corrected, "normal movement after a correction is accepted again");
}

} // namespace

int main() {
    testWalkingAllowed();
    testSpeedHackClamped();
    testJumpArcAllowed();
    testFlightClamped();
    testTeleportBecomesBaseline();
    testLongStallDoesNotGrantFlight();
    testFallingAllowed();
    testRecoveryAfterCorrection();

    if (failures == 0) {
        std::cout << "MovementGuardTests: all checks passed" << std::endl;
        return 0;
    }

    std::cerr << "MovementGuardTests: " << failures << " check(s) failed" << std::endl;
    return 1;
}
