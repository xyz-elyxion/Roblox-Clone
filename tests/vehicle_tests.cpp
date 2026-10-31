#include "Vehicle.h"

#include <cmath>
#include <iostream>
#include <limits>

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

    if (failures != 0) {
        std::cerr << failures << " vehicle checks failed\n";
        return 1;
    }
    std::cout << "vehicle_tests passed\n";
    return 0;
}
