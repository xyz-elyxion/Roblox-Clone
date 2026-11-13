#pragma once

#include <string>
#include <vector>

namespace rblx {
namespace roles {

// What a role lets its holder do on the site. A role carries these as bits, so a
// check is one AND against the table instead of a walk through the account's
// history, and adding a capability is one enum value plus the roles that get it.
enum Permission : unsigned {
    kNone = 0,
    kViewDashboard = 1u << 0,
    kModerateUsers = 1u << 1,
    kDeleteContent = 1u << 2,
    kGrantCoins = 1u << 3,
    kHardwareBans = 1u << 4,
    kReviewAppeals = 1u << 5,
    kAssignRoles = 1u << 6,
};

constexpr unsigned operator|(Permission first, Permission second) {
    return static_cast<unsigned>(first) | static_cast<unsigned>(second);
}

constexpr unsigned kAllPermissions = kViewDashboard | kModerateUsers | kDeleteContent |
                                     kGrantCoins | kHardwareBans | kReviewAppeals |
                                     kAssignRoles;

struct Role {
    // Stored on the account and never renamed: `name` is the label pages show.
    std::string id;
    std::string name;
    // Where the role sits in the ladder. A role may only be handed out by an
    // account above it, which is what stops a moderator from making themselves
    // an owner, or demoting the person who appointed them.
    int rank = 0;
    unsigned permissions = kNone;
    // Staff roles carry the staff badge in chat and on the profile. The community
    // roles sit below them and are badges only.
    bool staff = false;
};

// Every role there is, lowest rank first. A role that is not in this table cannot
// be stored on an account: the store reads an unknown id back as `player`.
inline const std::vector<Role>& all() {
    static const std::vector<Role> table = {
        {"player", "Player", 0, kNone, false},
        {"premium", "Premium", 10, kNone, false},
        {"contributor", "Contributor", 20, kNone, false},
        {"translator", "Translator", 30, kNone, false},
        // Verified sits above the programme roles because the identity check is
        // the one badge an account cannot be given by a game or a purchase.
        {"verified", "Verified", 40, kNone, false},
        {"creator", "Star Creator", 50, kNone, false},
        // The staff ladder. Support reads and answers appeals; a moderator also
        // acts on accounts; an administrator runs the site; the owner is the
        // account that owns the install and can never be demoted by anyone.
        {"support", "Support", 100, kViewDashboard | kReviewAppeals, true},
        {"moderator", "Moderator", 200, kViewDashboard | kReviewAppeals | kModerateUsers, true},
        {"administrator", "Administrator", 300,
         kViewDashboard | kReviewAppeals | kModerateUsers | kDeleteContent | kGrantCoins |
             kHardwareBans | kAssignRoles,
         true},
        {"owner", "Owner", 400, kAllPermissions, true},
    };
    return table;
}

inline std::string lowerAscii(std::string value) {
    for (char& c : value) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return value;
}

// The role an account falls back to when it holds nothing, or something this
// build does not know.
inline const Role& player() { return all().front(); }

inline bool isKnown(const std::string& id) {
    const std::string lowered = lowerAscii(id);
    for (const Role& role : all()) {
        if (role.id == lowered) {
            return true;
        }
    }
    return false;
}

inline const Role& find(const std::string& id) {
    const std::string lowered = lowerAscii(id);
    for (const Role& role : all()) {
        if (role.id == lowered) {
            return role;
        }
    }
    return player();
}

// Takes the permission as a plain mask as well as a single Permission, so a check
// against kAllPermissions reads the same way as a check against one capability.
inline bool has(unsigned granted, unsigned permission) {
    return (granted & permission) != 0;
}

inline bool isStaffRole(const std::string& id) { return find(id).staff; }

// Whether `actor` may put an account that currently holds `subject` onto
// `wanted`. An account may only move roles strictly below its own, and may not
// touch an account that already sits at or above it, so a support member cannot
// quietly become an administrator and a moderator cannot demote one.
inline bool mayAssign(const Role& actor, const Role& subject, const Role& wanted) {
    if (!has(actor.permissions, kAssignRoles)) {
        return false;
    }
    return wanted.rank < actor.rank && subject.rank < actor.rank;
}

// The capabilities as words, for the dashboard's role table and for server logs.
inline std::vector<std::string> permissionNames(unsigned granted) {
    std::vector<std::string> names;
    if (has(granted, kViewDashboard)) {
        names.push_back("dashboard");
    }
    if (has(granted, kReviewAppeals)) {
        names.push_back("appeals");
    }
    if (has(granted, kModerateUsers)) {
        names.push_back("moderation");
    }
    if (has(granted, kDeleteContent)) {
        names.push_back("content");
    }
    if (has(granted, kGrantCoins)) {
        names.push_back("coins");
    }
    if (has(granted, kHardwareBans)) {
        names.push_back("hardware bans");
    }
    if (has(granted, kAssignRoles)) {
        names.push_back("roles");
    }
    return names;
}

} // namespace roles
} // namespace rblx
