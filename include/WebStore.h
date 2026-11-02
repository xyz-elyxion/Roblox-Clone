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

// How much of an account's moderation record is kept. Long enough to cover a
// whole climb of the ladder with room to spare, short enough that a data file
// stays readable.
const std::size_t kMaxInfractionsPerUser = 50;

// How many detections are held for one account between waves. A wave turns
// whatever it holds into one action, so a longer queue adds nothing but a bigger
// data file.
const std::size_t kMaxPendingDetections = 20;

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

// One thing moderation did to an account. The account reads these in Settings so
// a strike is a row with a reason and a date rather than a number, and so an
// appeal has something specific to point at.
struct InfractionRecord {
    int id = 0;
    // "warning", "suspension", "ban", "pardon" or "detection".
    std::string kind;
    std::string reason;
    std::string createdAt;
    // End of a suspension; empty for every other kind.
    std::string expiresAt;
    // Strikes the account held after this entry, so the record reads as a ladder.
    int strikes = 0;
    // True when a detection filed it rather than an admin.
    bool automatic = false;
    // How many detections the wave folded into this one action. Read by the
    // dashboard; never sent to the account.
    int detections = 0;
};

// One detection waiting for the next wave.
struct PendingDetection {
    long long atMs = 0;
    std::string reason;
};

// A player asking for a restriction to be looked at again, and what an admin made
// of it. `status` is "open", "accepted" or "denied".
struct AppealRecord {
    int id = 0;
    int userId = 0;
    // The action this appeal is about. One action can be appealed once, ever.
    int infractionId = 0;
    std::string username;
    std::string message;
    std::string status = "open";
    std::string createdAt;
    std::string reviewedAt;
    std::string reviewer;
    std::string reviewNote;
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
    // Moderation's record of this account, oldest first. The newest
    // kMaxInfractionsPerUser entries are kept.
    std::vector<InfractionRecord> infractions;
    // Detections that have arrived since the last wave. Nothing acts on one of
    // these on its own: the wave sweep turns everything held into one step.
    std::vector<PendingDetection> pendingDetections;
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
    // Appends one entry to the account's record and keeps the newest
    // kMaxInfractionsPerUser of them; false when the id is unknown.
    // `detections` is how many held detections the action covers; it is what the
    // dashboard reads and is never part of the account's own view.
    bool addInfraction(int id, const std::string& kind, const std::string& reason,
                       bool automatic, int strikes, const std::string& expiresAt,
                       const std::string& nowIso, int detections = 0);
    // Empty for an account that does not exist.
    const std::vector<InfractionRecord>& infractionsFor(int userId) const;
    // Holds one detection for the next wave; false when the id is unknown.
    bool addPendingDetection(int id, const std::string& reason, long long atMs);
    // Empty for an account that does not exist.
    const std::vector<PendingDetection>& pendingDetectionsFor(int userId) const;
    // Drops what the last wave acted on.
    void clearPendingDetections(int userId);
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

    // appeals
    int addAppeal(const AppealRecord& record);
    const AppealRecord* findAppeal(int id) const;
    // The appeal still waiting for an admin, if any. Used for the badge; an
    // account may hold one open appeal per action it appeals.
    const AppealRecord* openAppealFor(int userId) const;
    // The appeal filed against one action, in any state. This is what limits an
    // account to one appeal per action.
    const AppealRecord* appealForInfraction(int infractionId) const;
    std::vector<const AppealRecord*> appealsFor(int userId) const;
    std::vector<const AppealRecord*> recentAppeals(std::size_t limit) const;
    std::size_t openAppealCount() const;
    bool reviewAppeal(int id, const std::string& status, const std::string& reviewer,
                      const std::string& note, const std::string& nowIso);

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
    std::vector<AppealRecord> appeals_;
    int nextUserId_ = 1;
    int nextFriendshipId_ = 1;
    int nextGameId_ = 1;
    int nextReportId_ = 1;
    int nextBlockId_ = 1;
    int nextInfractionId_ = 1;
    int nextAppealId_ = 1;
};

}  // namespace web
}  // namespace rblx
