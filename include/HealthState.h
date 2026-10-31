#pragma once

// Server-side player health.
//
// Before this the client sent its own health in every state packet and the
// server only clamped the value to 0..100, so a modified client could report
// 100 forever and nothing in the game could kill anybody. Health, death and the
// respawn timer are now decided by the game server and broadcast to clients.
//
// Header-only and free of sockets and physics, like MovementGuard, so the rules
// can be tested without running a server.

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace rblx::health {

constexpr float kMaxHealth = 100.0f;

// How long a dead player waits before respawning. Long enough to see what
// happened, short enough that a fall is not a punishment.
constexpr uint64_t kRespawnDelayMs = 3000;

// The height the client's characters fall below out of the world. Character.cpp
// uses the same number for its own local death plane; the server needs it too
// now that the server decides when a fall is fatal.
inline constexpr float kFallDeathHeight = -185.0f;

struct State {
    float health = kMaxHealth;
    float maxHealth = kMaxHealth;
    bool dead = false;
    uint64_t respawnAtMs = 0;
    int deaths = 0;

    bool isDead() const { return dead; }

    // Applies damage and returns true when this hit killed the player. Damage
    // while already dead is ignored, and so are negative and non-finite amounts:
    // a NaN would otherwise leave health NaN, which compares false against
    // everything and would never kill anyone.
    bool damage(float amount, uint64_t nowMs, uint64_t respawnDelayMs = kRespawnDelayMs) {
        if (dead || !std::isfinite(amount) || amount <= 0.0f) {
            return false;
        }

        health = std::max(0.0f, health - amount);
        if (health > 0.0f) {
            return false;
        }

        dead = true;
        respawnAtMs = nowMs + respawnDelayMs;
        ++deaths;
        return true;
    }

    // Runs the respawn timer. Returns true on the tick the player comes back.
    bool update(uint64_t nowMs) {
        if (!dead || nowMs < respawnAtMs) {
            return false;
        }
        health = maxHealth;
        dead = false;
        respawnAtMs = 0;
        return true;
    }

    // Whole seconds left, rounded up so the overlay counts 3, 2, 1.
    int respawnSecondsRemaining(uint64_t nowMs) const {
        if (!dead || nowMs >= respawnAtMs) {
            return 0;
        }
        return static_cast<int>((respawnAtMs - nowMs + 999u) / 1000u);
    }

    // Puts the player back to full health without touching the death count.
    void reset() {
        health = maxHealth;
        dead = false;
        respawnAtMs = 0;
    }
};

} // namespace rblx::health
