#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "WebJson.h"

namespace rblx {
namespace web {

// Avatar slots in the order the six RGB triplets are stored and rendered.
enum class AvatarSlot {
    Head = 0,
    Torso = 1,
    LeftArm = 2,
    RightArm = 3,
    LeftLeg = 4,
    RightLeg = 5
};

const int kAvatarSlotCount = 6;

// How much playtime earns one coin. The game server reports playtime in
// one-minute chunks, so a minute played is one coin earned.
const long long kSecondsPerCoin = 60;

struct Avatar {
    double colors[kAvatarSlotCount][3] = {
        {0.8, 0.6, 0.4},
        {0.2, 0.4, 0.8},
        {0.8, 0.6, 0.4},
        {0.8, 0.6, 0.4},
        {0.2, 0.6, 0.2},
        {0.2, 0.6, 0.2}
    };
    std::string faceId = "classic";
    // Equipped hat id; empty when the character wears none.
    std::string hatId;
};

struct UserRecord {
    int id = 0;
    std::string username;
    std::string passwordHash;
    // Hash of the one-time recovery code handed to the account when it was
    // created or when the code was rotated. Empty when no code was issued yet.
    std::string recoveryHash;
    std::string createdAt;
    // Wall-clock time of the account's last authenticated request, in ms since
    // the epoch. Used for the online dot and never sent to another account.
    long long lastSeenAtMs = 0;
    // Raised whenever every existing session for the account must stop being
    // accepted: a password change, a recovery reset, a suspension or a ban.
    // Tokens carry the value they were issued with, so a token minted before the
    // bump is refused instead of staying valid until it expires.
    int tokenVersion = 0;
    // Confirmed anti-cheat detections. The ladder in Moderation.h decides what
    // each one costs, so the account stores the count and the outcome of the
    // last one rather than the rule itself.
    int cheatStrikes = 0;
    // Wall-clock time the suspension ends, in ms since the epoch. Zero when the
    // account is not suspended, and an expired time is not a restriction.
    long long suspendedUntilMs = 0;
    bool banned = false;
    // What the last strike was for, and when it happened, so the dashboard shows
    // an admin why an account is where it is.
    std::string moderationReason;
    std::string moderatedAt;
    // Anti-cheat detections the client reported from inside its own process.
    // They are evidence an admin weighs, not strikes: the client that reports
    // them is the one being cheated, so they never move the ladder on their own.
    int clientDetections = 0;
    std::string lastClientDetectionAt;
};

struct AvatarRecord {
    int userId = 0;
    Avatar avatar;
    // Hats this account bought. Ownership lives with the avatar record so one
    // entry per account carries both appearance and inventory.
    std::vector<std::string> ownedHatIds;
};

struct FriendshipRecord {
    int id = 0;
    int requesterId = 0;
    int addresseeId = 0;
    std::string status;
    std::string createdAt;
    std::string updatedAt;
};

struct StatsRecord {
    int userId = 0;
    long long playtimeSeconds = 0;
    long long coins = 0;
    std::string lastPlayedAt;
};

// A report filed from the site against a game or another account. The names are
// copied in so a report still reads correctly after a rename or a deletion.
struct ReportRecord {
    int id = 0;
    int reporterId = 0;
    std::string reporterUsername;
    std::string targetType;  // "game" or "user"
    int targetId = 0;
    std::string targetLabel;
    std::string reason;
    std::string createdAt;
};

// One direction of a block. Visibility is cut both ways, so a pair is checked
// with blockedEitherWay instead of a row per side.
struct BlockRecord {
    int id = 0;
    int blockerId = 0;
    int blockedId = 0;
    std::string createdAt;
};

struct GameRecord {
    int id = 0;
    int ownerId = 0;
    std::string title;
    std::string slug;
    std::string description;
    bool isPublic = true;
    std::string worldPath;
    std::string scriptPath;
    std::string iconPath;
    std::string bannerPath;
    int partsCount = 0;
    int dynamicPartsCount = 0;
    int spawnCount = 0;
    int launchCount = 0;
    std::string createdAt;
    std::string updatedAt;
    std::string publishedAt;
    std::string ownerUsername;
};

// In-memory records backed by a single JSON document on disk. Not thread safe:
// the web app serialises access with its own mutex.
class Store {
public:
    explicit Store(std::string path);

    const std::string& path() const { return path_; }
    void setPath(std::string path) { path_ = std::move(path); }

    bool load(std::string* error);
    bool save(std::string* error) const;
    void clear();

    // users
    const UserRecord* findUserByName(const std::string& username) const;
    const UserRecord* findUserById(int id) const;
    int createUser(const std::string& username, const std::string& passwordHash,
                   const std::string& nowIso);
    // Keeps the account's id, password hash and ownership; refuses a name that
    // another account already holds.
    bool renameUser(int id, const std::string& username);
    // Replaces the stored hash; the caller builds it with hashPassword so the
    // iterations and salt stay with the credential rather than the store.
    bool setPasswordHash(int id, const std::string& passwordHash);
    // Raises the account's token version and returns the new value, or -1 when
    // there is no account with that id. Every token issued earlier stops being
    // accepted once this returns.
    int bumpTokenVersion(int id);
    // Writes the moderation state of an account; false when the id is unknown.
    bool setModeration(int id, int strikeCount, long long suspendedUntilMs, bool banned,
                       const std::string& reason, const std::string& nowIso);
    // Counts one detection reported by the account's own client.
    bool recordClientDetection(int id, const std::string& nowIso);
    bool setRecoveryHash(int id, const std::string& recoveryHash);
    // Records activity for the online dot; true when the stored time was old
    // enough to be replaced, so the caller only rewrites the file when needed.
    bool touchUser(int id, long long nowMs);
    // Removes the account with its avatar, stats, friendships, blocks and the
    // reports it filed. The caller removes its games, which own files on disk.
    bool removeUser(int id);
    const UserRecord* firstUser() const;
    const std::vector<UserRecord>& users() const { return users_; }
    std::vector<const UserRecord*> searchUsers(const std::string& query, std::size_t limit) const;

    // avatars
    Avatar avatarFor(int userId) const;
    void setAvatar(int userId, const Avatar& avatar);
    std::vector<std::string> ownedHatsFor(int userId) const;
    bool ownsHat(int userId, const std::string& hatId) const;
    // Adds a hat to the account's inventory; false when it already owns it.
    bool grantHat(int userId, const std::string& hatId);
    // Deducts coins; false when the account cannot afford the price.
    bool spendCoins(int userId, long long amount);

    // stats
    long long playtimeFor(int userId) const;
    long long coinsFor(int userId) const;
    std::string lastPlayedAtFor(int userId) const;
    void addPlaytime(int userId, long long seconds, const std::string& nowIso);

    // friendships
    const FriendshipRecord* findFriendship(int firstUserId, int secondUserId) const;
    int countAcceptedFriends(int userId) const;
    std::vector<const FriendshipRecord*> acceptedFriendships(int userId, std::size_t limit) const;
    std::vector<const FriendshipRecord*> pendingFriendships(int userId, bool incoming) const;
    int addFriendship(int requesterId, int addresseeId, const std::string& nowIso);
    void setFriendshipStatus(int friendshipId, const std::string& status,
                             const std::string& nowIso);
    void removeFriendship(int friendshipId);
    void removeFriendshipsBetween(int firstUserId, int secondUserId);

    // reports
    int addReport(const ReportRecord& record);
    std::vector<const ReportRecord*> recentReports(std::size_t limit) const;
    std::size_t reportCount() const { return reports_.size(); }

    // blocks
    bool addBlock(int blockerId, int blockedId, const std::string& nowIso);
    bool removeBlock(int blockerId, int blockedId);
    bool isBlocked(int blockerId, int blockedId) const;
    // True when either account blocked the other: a block hides both sides from
    // each other, whoever filed it.
    bool blockedEitherWay(int firstUserId, int secondUserId) const;
    std::vector<const BlockRecord*> blocksBy(int userId) const;

    // games
    const GameRecord* findGame(int id) const;
    std::vector<const GameRecord*> allGames() const;
    std::vector<const GameRecord*> publicGames(std::size_t limit) const;
    std::vector<const GameRecord*> gamesOwnedBy(int ownerId) const;
    int addGame(const GameRecord& record);
    void updateGame(const GameRecord& record);
    void incrementLaunchCount(int gameId);
    bool removeGame(int id);
    std::size_t gameCount() const { return games_.size(); }

private:
    void resolveOwnerNames();
    StatsRecord* findOrCreateStats(int userId);
    AvatarRecord* findOrCreateAvatar(int userId);

    std::string path_;
    std::vector<UserRecord> users_;
    std::vector<AvatarRecord> avatars_;
    std::vector<FriendshipRecord> friendships_;
    std::vector<StatsRecord> stats_;
    std::vector<GameRecord> games_;
    std::vector<ReportRecord> reports_;
    std::vector<BlockRecord> blocks_;
    int nextUserId_ = 1;
    int nextFriendshipId_ = 1;
    int nextGameId_ = 1;
    int nextReportId_ = 1;
    int nextBlockId_ = 1;
};

}  // namespace web
}  // namespace rblx
