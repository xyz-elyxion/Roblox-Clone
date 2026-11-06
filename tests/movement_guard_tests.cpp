// Tests for the server-side movement guard. The guard only depends on glm, so
// these run without a socket, a physics world or a running server.

#include "MovementGuard.h"
#include "ReplayRecorder.h"

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
    // Under the 14 units/s rise limit, so only the climb budget can catch it:
    // at 12 units/s the budget drains by 6 units/s and runs out after ~4 s.
    for (int step = 0; step < 400; ++step) {
        now += 16;
        y += 12.0f * 0.016f; // quietly floating upward at 12 units/s
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

// The client sends its state every frame over UDP, so three packets can land in
// the same server tick. They are 16 ms apart on the client's clock, and used to
// be measured as 1 ms apart, which turned an honest walk into a speed hack.
void testBunchedPacketsAccepted() {
    rblx::movement::Guard guard;
    glm::vec3 position(0.0f, 5.0f, 0.0f);
    guard.reset(position, 1000);
    uint32_t clientMs = 5000;
    bool corrected = false;
    for (int step = 0; step < 3; ++step) {
        clientMs += 16;
        position.x += 20.0f / 60.0f; // a frame of walking at 20 units/s
        // Every packet of the burst carries the same arrival time.
        const auto result = guard.validate(position, 1016, clientMs);
        corrected = corrected || result.corrected;
        position = result.position;
    }
    check(!corrected, "a burst of packets in one tick is measured by the client's clock");
}

// The stamp cannot be used to buy speed: a client that claims a quarter of a
// second while its datagrams land a millisecond apart is held to the wire.
void testStampThatOutrunsTheWireIsNotBelieved() {
    rblx::movement::Guard guard;
    guard.reset(glm::vec3(0.0f), 0);
    check(!guard.validate(glm::vec3(0.0f), 16, 1000).corrected, "stamp: the baseline is accepted");
    const auto result = guard.validate(glm::vec3(8.0f, 0.0f, 0.0f), 17, 1250);
    check(result.corrected, "a stamp that outruns the wire is not believed");
    check(result.evidence.frameSeconds < 0.002f, "the uncorroborated step is measured in milliseconds");
}

// The other side of the same problem: a real one second stall. Both clocks agree
// the player had a second, so a second of walking is not a teleport.
void testCorroboratedStallAccepted() {
    rblx::movement::Guard guard;
    guard.reset(glm::vec3(0.0f), 0);
    check(!guard.validate(glm::vec3(0.0f), 0, 7000).corrected, "stall: the baseline is accepted");
    const auto result = guard.validate(glm::vec3(20.0f, 0.0f, 0.0f), 1000, 8000);
    check(!result.corrected, "a stall both clocks agree on is walked, not teleported");
    check(near(result.evidence.frameSeconds, 1.0f, 0.0001f),
          "the acknowledged second is what the step is measured against");
}

// Jumping on the spot for a minute must not run the climb budget dry and then be
// reported as flying. Landing pays back what the jump spent.
void testRepeatedJumpingIsNotFlight() {
    rblx::movement::Guard guard;
    guard.reset(glm::vec3(0.0f), 0);
    bool corrected = false;
    bool flight = false;
    uint64_t now = 0;
    for (int jump = 0; jump < 30; ++jump) {
        float velocity = 12.0f;
        float y = 0.0f;
        for (int step = 0; step < 60; ++step) {
            const float dt = 0.016f;
            velocity -= 20.0f * dt;
            y = std::max(0.0f, y + velocity * dt);
            now += 16;
            const auto result = guard.validate(glm::vec3(0.0f, y, 0.0f), now);
            corrected = corrected || result.corrected;
            flight = flight || result.flight;
            y = result.position.y;
        }
    }
    check(!corrected, "thirty jumps in a row are not corrected");
    check(!flight, "jumping is never reported as flying");
}

// A correction is only worth recording if it says what was measured and what the
// game allows, in the same units, so an admin can judge it without the client.
void testEvidenceRecordsProof() {
    rblx::movement::Guard guard;
    guard.reset(glm::vec3(10.0f, 5.0f, -2.0f), 0);
    const auto result = guard.validate(glm::vec3(1010.0f, 5.0f, -2.0f), 16);
    check(result.corrected, "evidence: the speed hack is corrected");
    check(near(result.evidence.horizontal, 1000.0f, 0.01f),
          "evidence: the distance claimed is measured");
    check(near(result.evidence.allowedHorizontal, 30.0f * 0.016f + 0.4f, 0.001f),
          "evidence: the distance allowed is the frame's speed limit");
    check(near(result.evidence.from.x, 10.0f, 0.001f),
          "evidence: the baseline is the position the server accepted");
    check(near(result.evidence.claimed.x, 1010.0f, 0.001f),
          "evidence: the claim is the position the client asked for");
    check(near(result.evidence.frameSeconds, 0.016f, 0.0001f),
          "evidence: the gap between the packets is recorded");

    const std::string proof = rblx::movement::describeProof(result.evidence);
    check(proof.find("claimed 1010.00, 5.00, -2.00") != std::string::npos,
          "evidence: the proof names the claimed position");
    check(proof.find("1000.00 units horizontal") != std::string::npos,
          "evidence: the proof names the distance claimed");

    const std::string expected = rblx::movement::describeExpectation(result.evidence);
    check(expected.find("walking is 30.00 units/s") != std::string::npos,
          "evidence: the expectation names the walk speed");
    check(expected.find("at most 0.88 units this frame") != std::string::npos,
          "evidence: the expectation names the travel allowed this frame");
}

// The climb side reads the frame limit and the budget, which is what tells a fly
// hack apart from a laggy jump.
void testEvidenceRecordsExpectation() {
    rblx::movement::Guard guard;
    guard.reset(glm::vec3(0.0f), 0);
    // A five second stall is read as 250 ms, so the rise limit is 14*0.25+0.75.
    const auto result = guard.validate(glm::vec3(0.0f, 40.0f, 0.0f), 5000);
    check(result.corrected, "evidence: the stalled climb is corrected");
    check(near(result.evidence.rise, 40.0f, 0.01f), "evidence: the climb claimed is recorded");
    check(near(result.evidence.allowedRise, 14.0f * 0.25f + 0.75f, 0.001f),
          "evidence: the climb allowed is the frame's rise limit");
    check(near(result.evidence.riseBudget, 22.0f, 0.001f),
          "evidence: the climb budget is recorded");

    const std::string expected = rblx::movement::describeExpectation(result.evidence);
    check(expected.find("a jump rises 14.00 units/s") != std::string::npos,
          "evidence: the expectation names the rise speed");
    check(expected.find("against 22.00 units of climb budget") != std::string::npos,
          "evidence: the expectation names the climb budget left");
}

// An accepted step carries the same evidence, so a record can say what a clean
// frame looked like beside a rejected one.
void testEvidenceOnAcceptedStep() {
    rblx::movement::Guard guard;
    guard.reset(glm::vec3(0.0f), 0);
    const auto result = guard.validate(glm::vec3(0.3f, 0.0f, 0.0f), 16);
    check(!result.corrected, "evidence: an honest step is accepted");
    check(near(result.evidence.horizontal, 0.3f, 0.001f),
          "evidence: an accepted step still records its distance");
}

// The clip a detection carries: bounded, ordered, and readable back into frames.
void testReplayRecorder() {
    rblx::replay::Recorder recorder;
    const int sent = 120;
    for (int i = 0; i < sent; ++i) {
        const float x = static_cast<float>(i);
        recorder.add(static_cast<uint64_t>(i) * 16, rblx::replay::Point{x, 5.0f, -2.0f},
                     rblx::replay::Point{x, 5.0f, -2.0f});
    }
    check(recorder.frames().size() == rblx::replay::kMaxFrames,
          "replay: the recording is bounded");
    check(recorder.frames().back().atMs == static_cast<uint64_t>(sent - 1) * 16,
          "replay: the newest frame is kept");
    check(recorder.frames().front().atMs ==
              static_cast<uint64_t>(sent - rblx::replay::kMaxFrames) * 16,
          "replay: the run-up to the detection is what stays");

    const std::string encoded = recorder.encode();
    const std::vector<rblx::replay::Frame> decoded = rblx::replay::decode(encoded);
    check(decoded.size() == recorder.frames().size(), "replay: a clip survives encoding");
    check(decoded.back().claimed.x == recorder.frames().back().claimed.x,
          "replay: the claimed position survives encoding");
    check(decoded.back().accepted.z == recorder.frames().back().accepted.z,
          "replay: the accepted position survives encoding");
    check(decoded.front().atMs == recorder.frames().front().atMs,
          "replay: the timing survives encoding");

    const std::vector<rblx::replay::Frame> cut =
        rblx::replay::decode(encoded.substr(0, encoded.size() - 3));
    check(cut.size() + 1 == decoded.size(),
          "replay: a clip cut short loses only its last frame");
    check(rblx::replay::decode(std::string()).empty(), "replay: an empty clip decodes to nothing");

    recorder.reset();
    check(recorder.empty() && recorder.encode().empty(),
          "replay: a server repositioning starts the clip again");
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
    testBunchedPacketsAccepted();
    testStampThatOutrunsTheWireIsNotBelieved();
    testCorroboratedStallAccepted();
    testRepeatedJumpingIsNotFlight();
    testEvidenceRecordsProof();
    testEvidenceRecordsExpectation();
    testEvidenceOnAcceptedStep();
    testReplayRecorder();

    if (failures == 0) {
        std::cout << "MovementGuardTests: all checks passed" << std::endl;
        return 0;
    }

    std::cerr << "MovementGuardTests: " << failures << " check(s) failed" << std::endl;
    return 1;
}
