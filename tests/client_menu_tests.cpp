#include "ClientMenu.h"

#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(bool condition, const char* what) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << what << '\n';
    }
}

void checkEqual(const std::string& actual, const std::string& expected, const char* what) {
    if (actual != expected) {
        ++failures;
        std::cerr << "FAIL: " << what << ": got \"" << actual << "\", wanted \"" << expected << "\"\n";
    }
}

}  // namespace

int main() {
    using rblx::client_menu::findUserId;
    using rblx::client_menu::handle;
    using rblx::client_menu::inviteUrl;
    using rblx::client_menu::profileUrl;

    {
        checkEqual(profileUrl("http://localhost:3000", 42), "http://localhost:3000/profile/42",
                   "a profile URL is the site and the account id the page is named by");
        checkEqual(profileUrl("http://localhost:3000", 0), "",
                   "an account id that was never resolved opens nothing");
    }

    {
        checkEqual(inviteUrl("http://localhost:3000", 12), "http://localhost:3000/games/12",
                   "an invite opens the game's own page");
        checkEqual(inviteUrl("http://localhost:3000", 0), "",
                   "a session without a game has nothing to invite to");
    }

    {
        checkEqual(handle("Alchemist_Thor"), "@alchemist_thor",
                   "a handle is the account name folded to lower case");
        checkEqual(handle("a b/c\"d"), "@abcd",
                   "a name cannot put a space, a slash or a quote into a handle");
        checkEqual(handle(""), "", "a name with nothing in it has no handle");
        checkEqual(handle("!!!!"), "", "punctuation alone is not a handle");
        checkEqual(handle(std::string(40, 'x')), "@" + std::string(20, 'x'),
                   "a handle stops at the length a row can show");
    }

    {
        // The search answers prefix matches, so the exact account has to win over
        // the first name that only starts with the same letters.
        const std::string answer =
            "{\"users\":[{\"id\":7,\"username\":\"Bill\"},{\"id\":9,\"username\":\"Billy\"}]}";
        check(findUserId(answer, "Billy") == 9, "the exact account is chosen, not a prefix match");
        check(findUserId(answer, "Bill") == 7, "an exact match on a shorter name still lands");
        check(findUserId(answer, "Bilbo") == 0, "a name the answer does not hold has no id");
        check(findUserId(answer, "") == 0, "an empty name is never looked up");
    }

    {
        check(findUserId("not json", "Billy") == 0, "an unreadable answer names no account");
        check(findUserId("{\"users\":{}}", "Billy") == 0, "an answer without a user list names none");
        check(findUserId("{}", "Billy") == 0, "an empty answer names no account");
        check(findUserId("{\"users\":[{\"id\":0,\"username\":\"Billy\"}]}", "Billy") == 0,
              "an account without a positive id is not used");
    }

    if (failures != 0) {
        std::cerr << failures << " client menu checks failed\n";
        return 1;
    }
    std::cout << "client_menu_tests passed\n";
    return 0;
}
