#include <cstdio>
#include <string>

#include "Roles.h"

namespace {

int failures = 0;

void check(bool ok, const std::string& what) {
    std::printf("%s %s\n", ok ? "ok  " : "FAIL", what.c_str());
    if (!ok) {
        ++failures;
    }
}

} // namespace

int main() {
    using namespace rblx::roles;

    // The table is the whole language of roles: anything not in it cannot be
    // stored, so the ids have to be unique and the ranks have to order the ladder.
    check(!all().empty(), "the ladder has roles");
    bool idsUnique = true;
    bool idsLowercase = true;
    int previousRank = -1;
    bool ranksRise = true;
    for (const Role& role : all()) {
        if (role.id.empty() || lowerAscii(role.id) != role.id) {
            idsLowercase = false;
        }
        if (role.rank <= previousRank) {
            ranksRise = false;
        }
        previousRank = role.rank;
        for (const Role& other : all()) {
            if (&other != &role && other.id == role.id) {
                idsUnique = false;
            }
        }
    }
    check(idsUnique, "role ids are unique");
    check(idsLowercase, "role ids are stored lowercase");
    check(ranksRise, "ranks rise with the table");

    check(player().id == "player", "the fallback role is Player");
    check(find("nonsense").id == "player", "an unknown role reads as Player");
    check(find("").id == "player", "an empty role reads as Player");
    check(find("MODERATOR").id == "moderator", "lookup ignores case");
    check(isKnown("moderator") && !isKnown("superuser"), "only table roles are known");
    for (const Role& role : all()) {
        check(find(role.id).id == role.id, "find() returns " + role.id + " itself");
    }

    check(!has(player().permissions, kViewDashboard), "a player may not open the dashboard");
    check(permissionNames(player().permissions).empty(), "a player is granted nothing");
    check(!player().staff, "a player is not staff");

    // Community roles are badges: they must not quietly gain a capability.
    for (const Role& role : all()) {
        if (!role.staff) {
            check(role.permissions == 0, role.name + " is a badge only");
        }
    }

    bool staffCanSeeDashboard = true;
    for (const Role& role : all()) {
        if (role.staff && !has(role.permissions, kViewDashboard)) {
            staffCanSeeDashboard = false;
        }
    }
    check(staffCanSeeDashboard, "every staff role may open the dashboard");
    check(isStaffRole("moderator") && !isStaffRole("premium"), "staff roles are marked");
    check(has(find("support").permissions, kReviewAppeals), "support answers appeals");
    check(!has(find("support").permissions, kModerateUsers), "support does not act on accounts");
    check(has(find("moderator").permissions, kModerateUsers), "moderators act on accounts");
    check(!has(find("moderator").permissions, kAssignRoles), "moderators do not hand out roles");
    check(has(find("administrator").permissions, kAssignRoles), "administrators hand out roles");
    check(has(find("owner").permissions, kAllPermissions), "the owner holds every permission");
    check(permissionNames(kAllPermissions).size() == 7, "every permission has a name");

    // Who may put whom on what. The rule is the same one the dashboard shows:
    // strictly above the role being handed out, strictly above the target.
    const Role owner = find("owner");
    const Role admin = find("administrator");
    const Role moderator = find("moderator");
    const Role support = find("support");
    const Role playerRole = player();

    check(mayAssign(owner, playerRole, admin), "the owner can make an administrator");
    check(mayAssign(admin, playerRole, moderator), "an administrator can make a moderator");
    check(mayAssign(admin, playerRole, support), "an administrator can make support");
    check(mayAssign(admin, playerRole, find("premium")), "an administrator can hand out a badge");
    check(!mayAssign(admin, playerRole, admin), "an administrator cannot make a peer");
    check(!mayAssign(admin, playerRole, owner), "an administrator cannot make an owner");
    check(!mayAssign(owner, playerRole, owner), "the table has one owner, and no click adds one");
    check(!mayAssign(moderator, playerRole, moderator), "a moderator cannot hand out roles");
    check(!mayAssign(support, playerRole, find("premium")), "support cannot hand out badges");
    check(!mayAssign(playerRole, playerRole, find("premium")), "a player cannot promote anyone");
    check(mayAssign(owner, admin, support), "the owner can move an administrator down");
    check(!mayAssign(owner, owner, admin), "nobody can change the owner's role");
    check(!mayAssign(admin, admin, support), "an administrator cannot demote a peer");

    if (failures != 0) {
        std::printf("%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("all role checks passed\n");
    return 0;
}
