#include "Vehicle.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const char* what) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << what << '\n';
    }
}

bool near(float value, float expected, float tolerance = 0.001f) {
    return std::abs(value - expected) <= tolerance;
}

}  // namespace

int main() {
    using rblx::vehicle::Input;
    using rblx::vehicle::State;

    {
        State car;
        const glm::vec3 forward = car.forward();
        check(near(forward.x, 1.0f) && near(forward.z, 0.0f),
              "a car at yaw 0 faces +X, matching the engine's camera convention");
    }

    {
        State car;
        Input input;
        input.throttle = 1.0f;
        rblx::vehicle::step(car, input, 0.5f);
        check(car.speed > 0.0f, "throttle accelerates the car");
        check(car.position.x > 0.0f, "the car moves along its heading");
        check(near(car.position.z, 0.0f), "a straight car does not drift sideways");
    }

    {
        State car;
        Input input;
        input.throttle = 1.0f;
        for (int i = 0; i < 600; ++i) {
            rblx::vehicle::step(car, input, 1.0f / 60.0f);
        }
        check(near(car.speed, rblx::vehicle::Limits{}.maxSpeed),
              "speed stops at the car's top speed");
    }

    {
        State car;
        car.speed = 20.0f;
        Input input;  // no throttle
        rblx::vehicle::step(car, input, 0.5f);
        check(car.speed < 20.0f, "a car with no throttle coasts down");
        check(car.speed > 0.0f, "coasting does not stop the car instantly");

        for (int i = 0; i < 200; ++i) {
            rblx::vehicle::step(car, input, 1.0f / 60.0f);
        }
        check(near(car.speed, 0.0f), "coasting eventually stops the car");
    }

    {
        State rolling;
        rolling.speed = 20.0f;
        State braking;
        braking.speed = 20.0f;

        Input coast;
        Input stop;
        stop.throttle = -1.0f;
        rblx::vehicle::step(rolling, coast, 0.2f);
        rblx::vehicle::step(braking, stop, 0.2f);
        check(braking.speed < rolling.speed, "braking sheds speed faster than coasting");
    }

    {
        State car;
        Input reverse;
        reverse.throttle = -1.0f;
        for (int i = 0; i < 600; ++i) {
            rblx::vehicle::step(car, reverse, 1.0f / 60.0f);
        }
        check(near(car.speed, -rblx::vehicle::Limits{}.maxReverseSpeed),
              "reverse speed is capped lower than forward speed");
        check(car.position.x < 0.0f, "reversing moves the car backwards");
    }

    {
        State car;
        Input steer;
        steer.steer = 1.0f;  // right
        rblx::vehicle::step(car, steer, 0.5f);
        check(near(car.yawDegrees, 0.0f), "a stationary car cannot pivot on the spot");
        check(near(car.position.x, 0.0f), "a car with no throttle stays put");
    }

    {
        State car;
        car.speed = 20.0f;
        Input steer;
        steer.steer = 1.0f;
        rblx::vehicle::step(car, steer, 0.2f);
        check(car.yawDegrees > 0.0f, "steering right increases yaw");
        check(car.forward().z > 0.0f, "steering right swings the nose towards +Z");
    }

    {
        State car;
        car.speed = -10.0f;
        Input steer;
        steer.steer = 1.0f;
        rblx::vehicle::step(car, steer, 0.2f);
        check(car.yawDegrees < 0.0f, "reversing flips the steering direction");
    }

    {
        State car;
        Input input;
        input.set(std::numeric_limits<float>::quiet_NaN(), 1.0f);
        rblx::vehicle::step(car, input, 0.2f);
        check(near(car.speed, 0.0f), "NaN throttle is treated as no throttle");

        input.set(1.0e9f, -1.0e9f);
        check(near(input.throttle, 1.0f) && near(input.steer, -1.0f),
              "input is clamped to -1..1");
    }

    {
        // A stalled server tick must not teleport the car across the map.
        State car;
        Input input;
        input.throttle = 1.0f;
        car.speed = rblx::vehicle::Limits{}.maxSpeed;
        rblx::vehicle::step(car, input, 5.0f);
        const float travelled = std::abs(car.position.x);
        check(travelled <= rblx::vehicle::Limits{}.maxSpeed * 0.1f + 0.001f,
              "a long frame is clamped instead of teleporting the car");
    }

    {
        State car;
        check(rblx::vehicle::canEnter(car.position, car.position + glm::vec3(3.0f, 0.0f, 0.0f)),
              "a player standing beside the car can get in");
        check(!rblx::vehicle::canEnter(car.position,
                                       car.position + glm::vec3(0.0f, 0.0f, 40.0f)),
              "a player across the map cannot get in");
        check(!rblx::vehicle::canEnter(car.position,
                                       glm::vec3(std::nanf(""), 0.0f, 0.0f)),
              "a non-finite player position is never in range");
    }

    {
        const State car;
        const glm::vec3 seat = car.seatPosition(2.0f);
        check(near(seat.y, 2.0f), "the seat sits above the car's origin");
        check(rblx::vehicle::speedKmh(10.0f) == 36, "speed converts to km/h for the HUD");
        check(rblx::vehicle::speedKmh(-10.0f) == 36, "reverse speed is shown as a magnitude");
        check(rblx::vehicle::speedKmh(std::nanf("")) == 0, "an invalid speed shows as zero");
    }

    {
        // Driving straight at a wall used to mean driving through it.
        State car;
        car.position = glm::vec3(0.0f, 1.5f, 0.0f);
        car.speed = 20.0f;
        Input input;
        input.throttle = 1.0f;
        const std::vector<rblx::vehicle::Obstacle> wall = {
            {{10.0f, 0.0f, -10.0f}, {12.0f, 8.0f, 10.0f}}};
        for (int i = 0; i < 240; ++i) {
            rblx::vehicle::stepColliding(car, input, 1.0f / 60.0f, wall,
                                         glm::vec3(5.0f, 1.0f, 3.0f));
        }
        check(car.position.x <= 5.001f, "a wall stops the car before it drives through it");
        check(car.position.x > 1.0f, "the car still covered the ground before the wall");
        check(near(car.speed, 0.0f), "hitting a wall drops the car's speed");
    }

    {
        State colliding;
        colliding.position = glm::vec3(0.0f, 1.5f, 0.0f);
        colliding.speed = 15.0f;
        State plain = colliding;
        Input input;
        input.throttle = 1.0f;
        rblx::vehicle::stepColliding(colliding, input, 0.5f, {}, glm::vec3(5.0f, 1.0f, 3.0f));
        rblx::vehicle::step(plain, input, 0.5f);
        check(near(colliding.position.x, plain.position.x) &&
                  near(colliding.position.z, plain.position.z),
              "with no obstacles the car drives exactly as it does without collision");
    }

    {
        // A part that reaches only to y 0.7 against a body starting at 0.5 is a
        // kerb, not a wall: the car drives over it.
        State car;
        car.position = glm::vec3(0.0f, 1.5f, 0.0f);
        Input input;
        input.throttle = 1.0f;
        const std::vector<rblx::vehicle::Obstacle> kerb = {
            {{8.0f, -1.0f, -10.0f}, {20.0f, 0.7f, 10.0f}}};
        for (int i = 0; i < 120; ++i) {
            rblx::vehicle::stepColliding(car, input, 1.0f / 60.0f, kerb,
                                         glm::vec3(5.0f, 1.0f, 3.0f));
        }
        check(car.position.x > 25.0f, "a low part under the car is not a wall");
    }

    {
        // 42 units per second over a 0.1 s tick is 4.2 units of travel: more than
        // a thin wall is wide, so the move has to be sliced to notice it.
        State car;
        car.position = glm::vec3(0.0f, 1.5f, 0.0f);
        car.speed = rblx::vehicle::Limits{}.maxSpeed;
        Input input;
        input.throttle = 1.0f;
        const std::vector<rblx::vehicle::Obstacle> thin = {
            {{20.0f, 0.0f, -10.0f}, {20.5f, 8.0f, 10.0f}}};
        for (int i = 0; i < 300; ++i) {
            rblx::vehicle::stepColliding(car, input, 0.1f, thin, glm::vec3(5.0f, 1.0f, 3.0f));
        }
        check(car.position.x < 20.0f, "a thin wall is not tunneled through at top speed");
    }

    {
        // A world moved onto the car must not trap it for good.
        State car;
        car.position = glm::vec3(10.0f, 1.5f, 0.0f);
        Input input;
        input.throttle = 1.0f;
        const std::vector<rblx::vehicle::Obstacle> wall = {
            {{8.0f, 0.0f, -10.0f}, {12.0f, 8.0f, 10.0f}}};
        rblx::vehicle::stepColliding(car, input, 0.2f, wall, glm::vec3(5.0f, 1.0f, 3.0f));
        check(car.position.x > 10.0f, "a car that starts inside a part can drive out of it");
    }

    {
        // Hitting a wall at an angle should slide along it, not stop dead.
        State car;
        car.position = glm::vec3(0.0f, 1.5f, 0.0f);
        car.yawDegrees = 45.0f;  // heading along +X and +Z
        car.speed = 20.0f;
        Input input;
        input.throttle = 1.0f;
        const std::vector<rblx::vehicle::Obstacle> wall = {
            {{10.0f, 0.0f, -100.0f}, {12.0f, 8.0f, 100.0f}}};
        for (int i = 0; i < 180; ++i) {
            rblx::vehicle::stepColliding(car, input, 1.0f / 60.0f, wall,
                                         glm::vec3(5.0f, 1.0f, 3.0f));
        }
        check(car.position.x <= 5.001f, "a wall holds the car on the axis it blocks");
        check(car.position.z > 2.0f, "a car that clips a wall slides along it");
    }

    if (failures != 0) {
        std::cerr << failures << " vehicle checks failed\n";
        return 1;
    }
    std::cout << "vehicle_tests passed\n";
    return 0;
}
