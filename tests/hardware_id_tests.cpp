#include <cstdio>
#include <string>

#include "HardwareId.h"
#include "WebStore.h"

namespace {

int failures = 0;

void check(bool ok, const std::string& what) {
    std::printf("%s %s\n", ok ? "ok  " : "FAIL", what.c_str());
    if (!ok) ++failures;
}

}  // namespace

int main() {
    using namespace rblx::hwid;
    using rblx::web::Store;

    // The digest is stable, the right shape and different for a different machine.
    const std::string desk = hashInputs("vol=1234;host=desk");
    check(desk.size() == kHardwareIdLength, "a hash is the length one packet field holds");
    check(valid(desk), "a hash is lowercase hex");
    check(hashInputs("vol=1234;host=desk") == desk, "the same machine hashes the same way");
    check(hashInputs("vol=1235;host=desk") != desk, "a different volume is a different machine");
    check(hashInputs("vol=1234;host=laptop") != desk, "a different host is a different machine");

    // Normalizing is what lets a hash that arrived in capitals still match.
    check(normalize("0011AABBCCDDEEFF0011AABBCCDDEEFF") == "0011aabbccddeeff0011aabbccddeeff",
          "normalizing lowercases a hash");
    check(normalize(" 0011aabbccdd ") == "0011aabbccdd", "normalizing drops stray characters");
    check(normalize("zz0011aabb") == "0011aabb", "normalizing keeps hex only");

    // Anything that is not a full id is refused rather than stored, so a client
    // that sends nothing or a placeholder cannot sit in the ban list.
    check(!valid(""), "an empty id is not valid");
    check(!valid("0011aabb"), "a short id is not valid");
    check(!valid("0011AABBCCDDEEFF0011AABBCCDDEEFF"), "an id in capitals is not valid until normalized");
    check(!valid("zz11aabbccdd0011aabbccdd0011aabb"), "a non-hex id is not valid");
    check(valid(normalize("0011AABBCCDDEEFF0011AABBCCDDEEFF")),
          "a normalized id is accepted");

    // The machine the tests run on.
    const std::string here = machineId();
    check(valid(here), "this machine has an id");
    check(machineId() == here, "this machine's id does not change between calls");

    // The ban record lifecycle, through the same store the website saves.
    const std::string path = "hardware_id_tests_store.json";
    std::remove(path.c_str());

    std::string error;
    Store store(path);
    check(store.load(&error), "a missing data file loads as empty");
    check(store.activeHardwareBan(desk) == nullptr, "a machine starts unbanned");

    check(store.banHardware(desk, 7, "cheater", "flying", "2026-01-01T00:00:00Z"),
          "banning a machine records it");
    const rblx::web::HardwareBanRecord* ban = store.activeHardwareBan(desk);
    check(ban != nullptr, "the banned machine is found");
    check(ban != nullptr && ban->userId == 7, "the ban names the account it came from");
    check(ban != nullptr && ban->reason == "flying", "the ban keeps its reason for the record");

    check(!store.banHardware(desk, 9, "alt", "flying", "2026-01-02T00:00:00Z"),
          "banning the same machine twice does not add a second row");
    check(store.hardwareBans().size() == 1, "one machine is one row");

    check(store.save(&error), "the ban is written to the data file");
    Store reloaded(path);
    check(reloaded.load(&error), "the data file loads again: " + error);
    check(reloaded.activeHardwareBan(desk) != nullptr, "a ban survives a restart");
    check(reloaded.hardwareBans().size() == 1, "the reloaded list holds one machine");

    // A pardon has to reach the machine as well: an admin who fixes a false
    // positive must not leave the player locked out of the computer it happened on.
    check(reloaded.liftHardwareBansForUser(7, "2026-01-03T00:00:00Z") == 1,
          "a pardon lifts the machine ban the account created");
    check(reloaded.activeHardwareBan(desk) == nullptr, "a lifted ban no longer refuses the machine");
    check(reloaded.hardwareBans().size() == 1, "a lifted ban stays in the record as history");
    check(reloaded.hardwareBans().front().active == false, "the record says it was lifted");
    check(!reloaded.liftHardwareBan(desk, "2026-01-04T00:00:00Z"),
          "lifting a machine that is not banned is refused");

    // The account keeps the machine it last played from, which is what a ban is
    // copied onto.
    check(!reloaded.setLastHardwareId(999, desk, "2026-01-05T00:00:00Z"),
          "an unknown account keeps no machine");
    check(!reloaded.banHardware("not-a-hash", 7, "cheater", "flying", "2026-01-05T00:00:00Z"),
          "a malformed machine id is not banned");

    std::remove(path.c_str());
    std::printf("\n%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
