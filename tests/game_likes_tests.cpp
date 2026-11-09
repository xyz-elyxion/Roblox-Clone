// Likes are a row per account and game, so the same account cannot raise a
// game's score twice, and the score survives a restart.

#include "WebStore.h"

#include <cstdio>
#include <string>

namespace {
int failures = 0;

void check(bool condition, const std::string& name) {
    if (condition) {
        return;
    }
    ++failures;
    std::printf("FAIL %s\n", name.c_str());
}

int addAccount(rblx::web::Store& store, const std::string& name) {
    return store.createUser(name, "hash", "2026-01-01T00:00:00Z");
}

int addGame(rblx::web::Store& store, int ownerId, const std::string& title) {
    rblx::web::GameRecord game;
    game.ownerId = ownerId;
    game.title = title;
    game.isPublic = true;
    return store.addGame(game);
}

void testTheSameAccountCannotLikeTwice() {
    rblx::web::Store store("game-likes-test.json");
    const int owner = addAccount(store, "likes-owner");
    const int fan = addAccount(store, "likes-fan");
    const int game = addGame(store, owner, "Liked Game");
    check(game > 0, "the game is created");

    check(store.gameLikeCount(game) == 0, "a new game starts with no likes");
    check(!store.hasLikedGame(fan, game), "nobody has liked it yet");

    bool liked = false;
    check(store.toggleGameLike(fan, game, "2026-01-02T00:00:00Z", &liked), "a like is accepted");
    check(liked, "the toggle reports the like");
    check(store.gameLikeCount(game) == 1, "the count follows the row");
    check(store.hasLikedGame(fan, game), "the account that placed it is recorded");

    // The second press is the same account taking its like back, not a second
    // like: there is one row and it is either there or not.
    check(store.toggleGameLike(fan, game, "2026-01-02T00:01:00Z", &liked),
          "the same account may take its like back");
    check(!liked, "the second toggle reports the like gone");
    check(store.gameLikeCount(game) == 0, "the count is back to zero");
    check(!store.hasLikedGame(fan, game), "the row is gone");

    check(!store.toggleGameLike(fan, game + 500, "2026-01-02T00:02:00Z", nullptr),
          "a like on a game that does not exist is refused");
    check(!store.toggleGameLike(5000, game, "2026-01-02T00:02:00Z", nullptr),
          "a like from an account that does not exist is refused");
}

void testTwoAccountsCountTwice() {
    rblx::web::Store store("game-likes-two.json");
    const int owner = addAccount(store, "likes-owner-two");
    const int first = addAccount(store, "likes-first");
    const int second = addAccount(store, "likes-second");
    const int game = addGame(store, owner, "Popular Game");

    store.toggleGameLike(first, game, "2026-01-02T00:00:00Z", nullptr);
    store.toggleGameLike(second, game, "2026-01-02T00:00:00Z", nullptr);
    check(store.gameLikeCount(game) == 2, "two accounts are two likes");

    store.toggleGameLike(first, game, "2026-01-02T00:05:00Z", nullptr);
    check(store.gameLikeCount(game) == 1, "taking one back leaves the other");
    check(store.hasLikedGame(second, game), "the other account still holds its like");
}

void testLikesSurviveARestart() {
    const std::string path = "game-likes-restart.json";
    std::remove(path.c_str());
    int game = 0;
    int fan = 0;
    {
        rblx::web::Store store(path);
        const int owner = addAccount(store, "likes-saved-owner");
        fan = addAccount(store, "likes-saved-fan");
        game = addGame(store, owner, "Saved Game");
        store.toggleGameLike(fan, game, "2026-01-02T00:00:00Z", nullptr);

        std::string error;
        check(store.save(&error), "the store writes the like to the data file");
    }

    rblx::web::Store reopened(path);
    std::string error;
    check(reopened.load(&error), "the data file loads again");
    check(reopened.gameLikeCount(game) == 1, "the like came back");
    check(reopened.hasLikedGame(fan, game), "the account that placed it came back too");
    std::remove(path.c_str());
}

void testDeletingTakesTheLikesWithIt() {
    rblx::web::Store store("game-likes-delete.json");
    const int owner = addAccount(store, "likes-delete-owner");
    const int fan = addAccount(store, "likes-delete-fan");
    const int game = addGame(store, owner, "Doomed Game");
    store.toggleGameLike(fan, game, "2026-01-02T00:00:00Z", nullptr);

    check(store.removeGame(game), "the game is deleted");
    check(store.gameLikeCount(game) == 0, "a like cannot outlive its game");

    const int kept = addGame(store, owner, "Kept Game");
    store.toggleGameLike(fan, kept, "2026-01-02T00:10:00Z", nullptr);
    check(store.removeUser(fan), "the account is deleted");
    check(store.gameLikeCount(kept) == 0, "a like cannot outlive its account");
}
}  // namespace

int main() {
    testTheSameAccountCannotLikeTwice();
    testTwoAccountsCountTwice();
    testLikesSurviveARestart();
    testDeletingTakesTheLikesWithIt();

    if (failures != 0) {
        std::printf("game_likes_tests: %d check(s) failed\n", failures);
        return 1;
    }
    std::printf("game_likes_tests: all checks passed\n");
    return 0;
}
