#pragma once

// The ladder a cheating account climbs.
//
// The client's anti-cheat library runs inside the client process, so a modified
// client can simply not run it. What a client cannot fake is where its character
// went: MovementGuard on the game server catches a speed hack and a fly hack by
// comparing every claimed position with the one the server last accepted. This
// header takes the confirmed detections and turns them into what the player and
// the site act on: a warning, then suspensions of increasing length, then a ban.
//
// It is header-only and free of I/O so the whole ladder can be tested without a
// socket, a data file or a running server.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace rblx::moderation {

struct Policy {
    // Strikes answered with a warning alone before the first suspension. Two
    // means a player who trips the guard twice is told, and the third strike
    // costs them a day rather than their account.
    int warnings = 2;
    // Suspension lengths in hours, used in order. Once a player runs out of
    // entries the next strike bans the account.
    std::vector<long long> suspensionHours = {24, 72, 168};
};

enum class Action {
    // Nothing changed. Used when a strike arrives for an account that is already
    // banned, where another warning or suspension would be noise.
    None = 0,
    Warned,
    Suspended,
    Banned
};

// The part of an account the ladder reads and writes. The site stores these
// alongside the account and the game server never sees them directly.
struct State {
    int strikes = 0;
    long long suspendedUntilMs = 0;
    bool banned = false;
};

struct Decision {
    Action action = Action::None;
    int strikes = 0;
    long long suspendedUntilMs = 0;
    bool banned = false;
    // What the account is told, or empty for Action::None. It never names the
    // detection: a message that says which check fired tells a cheat which check
    // to get around next time.
    std::string message;
    // What was detected, for the record a moderator reads. Never sent to the
    // account it was taken from.
    std::string detection;
};

inline const char* actionName(Action action) {
    switch (action) {
        case Action::Warned: return "warned";
        case Action::Suspended: return "suspended";
        case Action::Banned: return "banned";
        case Action::None:
        default: return "none";
    }
}

// "1 hour", "24 hours", "7 days". Long suspensions read better in days, and the
// longest one the policy hands out is a week.
inline std::string describeHours(long long hours) {
    if (hours <= 1) {
        return "1 hour";
    }
    if (hours % 24 == 0 && hours >= 48) {
        return std::to_string(hours / 24) + " days";
    }
    return std::to_string(hours) + " hours";
}

// Hours left on a suspension, rounded up so a player is never told "0 hours"
// while the account is still refused.
inline long long remainingHours(const State& state, long long nowMs) {
    if (state.suspendedUntilMs <= nowMs) {
        return 0;
    }
    return (state.suspendedUntilMs - nowMs + 3599999) / 3600000;
}

// True when the account may not join a game or hold a session right now. A
// suspension that has run out stops restricting the account on its own, so an
// expired one never needs an admin to clear it.
inline bool isRestricted(const State& state, long long nowMs) {
    return state.banned || state.suspendedUntilMs > nowMs;
}

// Where the account stands, as the player is shown it. The site keeps the
// account signed in and says this on every page instead of closing the door: a
// player who cannot read their own record cannot appeal what is in it.
enum class Standing {
    Clear = 0,
    Warned,
    Suspended,
    Banned
};

inline Standing standingOf(const State& state, long long nowMs) {
    if (state.banned) {
        return Standing::Banned;
    }
    if (state.suspendedUntilMs > nowMs) {
        return Standing::Suspended;
    }
    if (state.strikes > 0) {
        return Standing::Warned;
    }
    return Standing::Clear;
}

inline const char* standingName(Standing standing) {
    switch (standing) {
        case Standing::Warned: return "warned";
        case Standing::Suspended: return "suspended";
        case Standing::Banned: return "banned";
        case Standing::Clear:
        default: return "clear";
    }
}

// What a refused player is told, on the site and over the chat channel.
inline std::string restrictionMessage(const State& state, long long nowMs) {
    if (state.banned) {
        return "This account is banned for cheating.";
    }
    if (state.suspendedUntilMs > nowMs) {
        return "This account is suspended for cheating for another " +
               describeHours(remainingHours(state, nowMs)) + ".";
    }
    return std::string();
}

// Records one confirmed detection and returns what it costs. `reason` is the
// short phrase the player sees, such as "flying" or "moving faster than
// allowed"; it is not parsed anywhere.
inline Decision applyStrike(State* state, const Policy& policy, long long nowMs,
                            const std::string& reason) {
    Decision decision;
    if (state == nullptr) {
        return decision;
    }

    // A banned account stays banned: the ladder has nothing left to take, and
    // counting strikes past it would only make a later pardon look wrong.
    if (state->banned) {
        decision.banned = true;
        decision.strikes = state->strikes;
        decision.suspendedUntilMs = state->suspendedUntilMs;
        decision.message = restrictionMessage(*state, nowMs);
        decision.detection = reason;
        return decision;
    }

    ++state->strikes;
    const std::string detail = reason.empty() ? std::string("cheating") : reason;
    // The detail goes to the record, not to the account: it names the check that
    // fired, which is exactly what the next attempt would avoid.
    decision.detection = detail;

    decision.strikes = state->strikes;
    if (state->strikes <= policy.warnings) {
        decision.action = Action::Warned;
        decision.message = "A cheating detection was confirmed on this account. Warning " +
                           std::to_string(state->strikes) + " of " +
                           std::to_string(policy.warnings) + ".";
        if (state->strikes == policy.warnings) {
            decision.message += " The next confirmed detection suspends this account.";
        }
    } else {
        const std::size_t index = static_cast<std::size_t>(state->strikes - policy.warnings - 1);
        if (index < policy.suspensionHours.size()) {
            const long long hours = policy.suspensionHours[index];
            state->suspendedUntilMs = nowMs + hours * 3600000;
            decision.action = Action::Suspended;
            decision.suspendedUntilMs = state->suspendedUntilMs;
            decision.message = "A cheating detection was confirmed on this account. "
                               "It is suspended for " + describeHours(hours) + ".";
        } else {
            state->banned = true;
            decision.action = Action::Banned;
            decision.banned = true;
            decision.message = "A cheating detection was confirmed on this account. "
                               "It is now banned.";
        }
    }

    decision.suspendedUntilMs = state->suspendedUntilMs;
    return decision;
}

// Clears everything a strike put on the account. The admin dashboard uses it for
// a pardon, which is also how a false positive is undone.
inline Decision liftRestriction(State* state) {
    Decision decision;
    if (state == nullptr) {
        return decision;
    }
    state->strikes = 0;
    state->suspendedUntilMs = 0;
    state->banned = false;
    decision.action = Action::None;
    decision.message = "The restriction on this account was lifted.";
    return decision;
}

}  // namespace rblx::moderation
