#include "HealthState.h"

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

}  // namespace

int main() {
    using rblx::health::State;

    {
        State state;
        check(state.health == rblx::health::kMaxHealth, "a fresh player starts at full health");
        check(state.maxHealth == rblx::health::kMaxHealth, "max health defaults to the constant");
        check(!state.isDead(), "a fresh player is alive");
        check(state.respawnSecondsRemaining(0) == 0, "no respawn countdown while alive");
    }

    {
        State state;
        check(!state.damage(30.0f, 1000), "non-lethal damage does not kill");
        check(state.health == 70.0f, "damage is subtracted from health");
        check(!state.isDead(), "the player survives 30 damage");
        check(state.deaths == 0, "surviving damage is not a death");
    }

    {
        State state;
        check(state.damage(100.0f, 5000), "lethal damage kills");
        check(state.health == 0.0f, "a dead player has no health");
        check(state.isDead(), "the player is dead");
        check(state.deaths == 1, "the death is counted");
        check(state.respawnSecondsRemaining(5000) == 3, "the countdown starts at three seconds");
        check(state.respawnSecondsRemaining(6000) == 2, "the countdown counts down");
        check(state.respawnSecondsRemaining(7999) == 1, "the last second is rounded up");
        check(!state.update(7000), "the player stays dead before the timer ends");
        check(state.isDead(), "still dead inside the respawn delay");
        check(state.update(8000), "the player respawns when the timer ends");
        check(state.health == rblx::health::kMaxHealth, "respawn restores full health");
        check(!state.isDead(), "the player is alive again");
        check(state.respawnSecondsRemaining(8000) == 0, "the countdown clears on respawn");
        check(state.deaths == 1, "respawning does not count as a death");
    }

    {
        State state;
        state.damage(60.0f, 0);
        check(!state.damage(0.0f, 10), "zero damage is ignored");
        check(!state.damage(-50.0f, 10), "negative damage cannot heal a player");
        check(state.health == 40.0f, "ignored damage leaves health alone");
    }

    {
        State state;
        state.damage(100.0f, 0);
        check(!state.damage(100.0f, 10), "damage while dead is ignored");
        check(state.deaths == 1, "a dead player only counts one death");
        check(state.respawnAtMs == rblx::health::kRespawnDelayMs,
              "the death sets one respawn deadline and later damage does not push it back");
    }

    {
        State state;
        check(!state.damage(std::nanf(""), 0), "NaN damage is ignored");
        check(!state.damage(std::numeric_limits<float>::infinity(), 0),
              "infinite damage is ignored");
        check(state.health == rblx::health::kMaxHealth, "invalid damage leaves health untouched");
        check(!state.isDead(), "invalid damage cannot kill a player");
    }

    {
        State state;
        state.health = 5.0f;
        state.damage(50.0f, 0);
        check(state.health == 0.0f, "overkill clamps health to zero instead of going negative");
    }

    {
        State state;
        state.damage(100.0f, 0, 1000);
        check(!state.update(999), "a custom respawn delay is respected");
        check(state.update(1000), "the custom delay ends when it should");
    }

    {
        State state;
        state.health = 12.0f;
        state.dead = true;
        state.reset();
        check(state.health == rblx::health::kMaxHealth, "reset restores full health");
        check(!state.isDead(), "reset clears the death flag");
    }

    {
        check(rblx::health::kFallDeathHeight < 0.0f, "the fall death plane is below the world");
    }

    if (failures != 0) {
        std::cerr << failures << " health state checks failed\n";
        return 1;
    }
    std::cout << "health_state_tests passed\n";
    return 0;
}
