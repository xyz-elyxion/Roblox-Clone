#include <cstdio>
#include <string>

#include "Moderation.h"

namespace {

int failures = 0;

void check(bool ok, const std::string& what) {
    std::printf("%s %s\n", ok ? "ok  " : "FAIL", what.c_str());
    if (!ok) ++failures;
}

bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

}  // namespace

int main() {
    using namespace rblx::moderation;

    const Policy policy;
    const long long now = 1700000000000LL;

    State state;
    const Decision first = applyStrike(&state, policy, now, "moving faster than allowed");
    check(first.action == Action::Warned, "the first strike is a warning");
    check(first.strikes == 1 && state.strikes == 1, "the strike is recorded");
    check(contains(first.message, "Warning 1 of 2"), "the warning numbers itself");
    check(!contains(first.message, "moving faster than allowed"),
          "the warning does not say what was detected");
    check(contains(first.detection, "moving faster than allowed"),
          "the detection is kept for the record instead");
    check(!isRestricted(state, now), "a warning leaves the account playing");
    check(state.suspendedUntilMs == 0, "a warning sets no suspension");

    const Decision second = applyStrike(&state, policy, now + 1000, "flying");
    check(second.action == Action::Warned, "the second strike is still a warning");
    check(contains(second.message, "next confirmed detection suspends"),
          "the last warning says what comes next");
    check(!isRestricted(state, now + 1000), "two warnings leave the account playing");

    const Decision third = applyStrike(&state, policy, now + 2000, "flying");
    check(third.action == Action::Suspended, "the third strike suspends the account");
    check(third.suspendedUntilMs == now + 2000 + 24 * 3600000LL, "the first suspension is a day");
    check(isRestricted(state, now + 2000), "a suspension restricts the account");
    check(contains(third.message, "24 hours"), "the suspension says how long");
    check(isRestricted(state, now + 2000 + 23 * 3600000LL), "the suspension lasts the day");
    check(!isRestricted(state, now + 2000 + 24 * 3600000LL), "the suspension expires on its own");
    check(restrictionMessage(state, now + 2000 + 24 * 3600000LL).empty(),
          "an expired suspension has no refusal text");
    check(contains(restrictionMessage(state, now + 2000), "24 hours"),
          "the refusal says how long is left");

    const Decision fourth = applyStrike(&state, policy, now + 3000, "flying");
    check(fourth.action == Action::Suspended, "the fourth strike suspends again");
    check(fourth.suspendedUntilMs == now + 3000 + 72 * 3600000LL,
          "the second suspension is three days");

    const Decision fifth = applyStrike(&state, policy, now + 4000, "flying");
    check(fifth.action == Action::Suspended, "the fifth strike suspends again");
    check(contains(fifth.message, "7 days"), "the third suspension is a week");

    const Decision sixth = applyStrike(&state, policy, now + 5000, "flying");
    check(sixth.action == Action::Banned, "the strike after the last suspension bans the account");
    check(state.banned, "the ban is stored on the account");
    check(isRestricted(state, now + 1000000000LL), "a ban never expires");
    check(contains(restrictionMessage(state, now), "banned"), "the refusal says the account is banned");
    check(contains(sixth.message, "banned"), "the player is told about the ban");

    const Decision again = applyStrike(&state, policy, now + 6000, "flying");
    check(again.action == Action::None, "a banned account gathers no further strikes");
    check(again.strikes == 6, "the strike count stops at the ban");

    const Decision pardoned = liftRestriction(&state);
    check(state.strikes == 0 && !state.banned && state.suspendedUntilMs == 0,
          "a pardon clears strikes, the suspension and the ban");
    check(!isRestricted(state, now), "a pardoned account can play again");
    check(contains(pardoned.message, "lifted"), "the pardon is announced");

    // One detection in an otherwise clean account must not escalate on its own.
    State honest;
    applyStrike(&honest, policy, now, "flying");
    check(!isRestricted(honest, now), "one detection in isolation is only a warning");

    // Rounding: a player is never told nothing is left while the site still
    // refuses the account.
    State almostOver;
    almostOver.suspendedUntilMs = now + 60000;
    check(remainingHours(almostOver, now) == 1, "a minute left reads as an hour");
    check(describeHours(1) == "1 hour", "a single hour is singular");
    check(describeHours(24) == "24 hours", "a day reads in hours");
    check(describeHours(168) == "7 days", "a week reads in days");

    // The ladder is policy, not code: an install can drop the warnings, or the
    // suspensions, without touching the rule.
    Policy strict;
    strict.warnings = 0;
    strict.suspensionHours = {1};
    State tuned;
    check(applyStrike(&tuned, strict, now, "flying").action == Action::Suspended,
          "a policy with no warnings suspends on the first strike");

    Policy noSuspensions;
    noSuspensions.warnings = 1;
    noSuspensions.suspensionHours.clear();
    State direct;
    applyStrike(&direct, noSuspensions, now, "flying");
    check(applyStrike(&direct, noSuspensions, now, "flying").action == Action::Banned,
          "a policy without suspensions bans on the strike after the warnings");

    check(applyStrike(nullptr, policy, now, "flying").action == Action::None,
          "a missing state is not a reason to crash");

    std::printf("\n%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
