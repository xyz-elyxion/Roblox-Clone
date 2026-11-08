#pragma once

#include <cstddef>
#include <string>
#include <utility>
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

// How many machine bans are kept. Active ones are never dropped; the oldest
// lifted entries go first, because the record is history once a pardon landed.
const std::size_t kMaxHardwareBans = 500;

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
    // What the game server measured and the gameplay it was held against. Read by
    // the dashboard; never sent to the account, because the numbers say which
    // check caught it and by how much.
    std::string proof;
    std::string expected;
    // The packets around the detection, as the game server recorded them. Read by
    // the dashboard, which draws them; never sent to the account.
    std::string replay;
};

// One detection waiting for the next wave.
struct PendingDetection {
    long long atMs = 0;
    std::string reason;
    // The evidence the game server sent with the report, empty for a client
    // detection whose reason already names what it saw.
    std::string proof;
    std::string expected;
    std::string replay;
};

// A machine barred from joining games. `hardwareId` is the hashed machine
// identity the client sends with every join (see HardwareId.h); the account it
// came from is kept beside it so an admin reading the list knows which player put
// the machine there. Lifting sets `active` false instead of deleting the row, so a
// pardon is visible in the record rather than looking like a ban that never was.
struct HardwareBanRecord {
    std::string hardwareId;
    int userId = 0;
    std::string username;
    std::string reason;
    std::string createdAt;
    bool active = true;
    std::string liftedAt;
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
    // The machine this account last joined a game from, and when. Kept on the
    // account so a ban that lands later can reach the computer without the game
    // server having to resend it at exactly the wrong moment.
    std::string lastHardwareId;
    std::string hardwareIdSeenAt;
    // Moderation's record of this account, oldest first. The newest
    // kMaxInfractionsPerUser entries are kept.
    std::vector<InfractionRecord> infractions;
    // Detections that have arrived since the last wave. Nothing acts on one of
    // these on its own: the wave sweep turns everything held into one step.
    std::vector<PendingDetection> pendingDetections;

    // The half of this record that never sits on disk in the clear -- the
    // password and recovery hashes, the machine id, the moderation record and
    // the detection queue -- sealed with AES-256-GCM, together with whether it
    // has been opened since the file was read. A record loaded from disk starts
    // sealed and empty, so a store with a thousand accounts opens exactly the one
    // a request asked for. `Store::save` reseals every open record with a fresh
    // nonce, so no value on disk is ever written twice.
    mutable std::string sealedSecrets;
    mutable bool secretsOpen = true;

    // Header copies of the queue: how many detections are waiting and when the
    // oldest arrived. They are what lets the ban sweep find the accounts that
    // have something queued without opening every record in the file.
    mutable int pendingDetectionCount = 0;
    mutable long long oldestPendingAtMs = 0;
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

// One line in one channel of one game's chat. The username is copied in the way
// a report's is, so a message still reads correctly after a rename or a deletion
// rather than turning into an id nobody can look up.
struct ChatMessageRecord {
    int id = 0;
    int gameId = 0;
    // One of the channel ids in ChatRoom.h; the store never invents one.
    std::string channel;
    int userId = 0;
    std::string username;
    std::string body;
    std::string createdAt;
    // When the same moment as createdAt, in milliseconds, so a message's age can
    // be compared with presence without parsing a timestamp.
    long long atMs = 0;
};

// How far one account has read in one channel. Kept on disk so an unread badge
// survives a restart: a badge that resets every time the website restarts would
// teach people to ignore it.
struct ChatReadRecord {
    int userId = 0;
    int gameId = 0;
    std::string channel;
    int lastMessageId = 0;
};

// How many AI training tokens one account may hold at once. A developer token is
// a long-lived credential for a program rather than a session, so a handful of
// them is plenty, and the cap keeps one leaked account from filling the file with
// live secrets.
const std::size_t kMaxDevTokensPerUser = 8;

// How many tokens the whole install keeps, revoked ones included.
const std::size_t kMaxDevTokens = 500;

// A developer token minted in the developer portal that lets a program sign in as
// the account that owns it. Only the SHA-256 of the secret is stored: the value is
// shown once, when it is issued, and cannot be recovered afterwards. `scope` names
// what the token is for, so a token handed to a trainer cannot be used as a
// session. `rootHash` is the hash of a separately shown root secret; the secret the
// program holds is never on disk in the clear.
struct DevTokenRecord {
    int id = 0;
    int userId = 0;
    std::string username;
    std::string label;
    // "ai" today. A route checks the scope it needs instead of accepting any token.
    std::string scope = "ai";
    std::string tokenHash;
    std::string createdAt;
    std::string lastUsedAt;
    bool revoked = false;
    std::string revokedAt;
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
    // Whether the AI trainer may play this game. A map is off until its owner
    // says a headless run of it makes sense; the trainer refuses every other one.
    bool aiCompatible = false;
    // "map" trains a policy to walk to a part in the world file; "doors" builds
    // the 100-door hotel the Doors script generates and trains the run to door 100.
    std::string aiCourse = "map";
    // The part a "map" course walks to. Empty means the part furthest from the
    // spawn, which is how the trainer has always picked its goal.
    std::string aiGoal;
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

    // The AES-256 key the data file is sealed with: exactly 32 bytes. Without one
    // the file is written in the clear, which is what an older install without a
    // key keeps doing; with one every record is sealed, and a file that already
    // holds sealed records cannot be opened without the key that made them.
    void setDataKey(std::string key) { dataKey_ = std::move(key); }
    bool hasDataKey() const { return dataKey_.size() == 32; }

    // Accounts holding detections the next wave can act on, each with the time of
    // its oldest queued detection. Only records with something queued are
    // opened, so a sweep of the whole install does not decrypt the whole file.
    std::vector<std::pair<int, long long>> queuedDetections() const;

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
                       const std::string& nowIso, int detections = 0,
                       const std::string& proof = std::string(),
                       const std::string& expected = std::string(),
                       const std::string& replay = std::string());
    // Empty for an account that does not exist.
    const std::vector<InfractionRecord>& infractionsFor(int userId) const;
    // The action with this id, or nullptr. `ownerId` receives the account it
    // belongs to, so the caller can name it without a second search.
    const InfractionRecord* findInfraction(int id, int* ownerId = nullptr) const;
    // Holds one detection for the next wave; false when the id is unknown.
    bool addPendingDetection(int id, const std::string& reason, const std::string& proof,
                             const std::string& expected, const std::string& replay, long long atMs);
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
    // Adds coins by hand, which is what an admin hands out from the dashboard.
    // False for an unknown account or an amount that is not positive.
    bool grantCoins(int userId, long long amount);

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

    // machines
    // Records the machine an account joined from; false for an unknown account.
    bool setLastHardwareId(int userId, const std::string& hardwareId, const std::string& nowIso);
    // Bars a machine. A machine that is already barred keeps its original row, so
    // the list does not grow one entry per detection: false is returned and
    // nothing changes.
    bool banHardware(const std::string& hardwareId, int userId, const std::string& username,
                     const std::string& reason, const std::string& nowIso);
    // The active ban for a machine, or nullptr when it is clear.
    const HardwareBanRecord* activeHardwareBan(const std::string& hardwareId) const;
    // Lifts one machine's ban; false when it was not barred.
    bool liftHardwareBan(const std::string& hardwareId, const std::string& nowIso);
    // Lifts every active ban an account put on a machine and returns how many.
    // A pardon has to reach the machine too, or a false positive leaves the player
    // locked out of every account on that computer.
    int liftHardwareBansForUser(int userId, const std::string& nowIso);
    const std::vector<HardwareBanRecord>& hardwareBans() const { return hardwareBans_; }

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

    // chat
    // Appends one message and returns its id, or 0 for a record that is not a
    // message (no game, no account, no channel or no text). The oldest messages
    // of a channel, and of the whole file, are dropped to keep it bounded.
    int addChatMessage(const ChatMessageRecord& record);
    // The newest messages of one channel, oldest first. An `afterId` above zero
    // returns only what a polling page has not seen yet.
    std::vector<const ChatMessageRecord*> chatMessages(int gameId, const std::string& channel,
                                                      int afterId, std::size_t limit) const;
    std::size_t chatMessageCount() const { return chat_.size(); }
    // A game's chat goes with the game, and an account's messages go with the
    // account: deleting either leaves nothing behind that a reader could find.
    void removeChatMessagesForGame(int gameId);
    void removeChatMessagesForUser(int userId);

    // unread
    // The newest message id the account has seen in one channel; 0 when it has
    // never opened that channel.
    int chatReadMark(int userId, int gameId, const std::string& channel) const;
    // Advances the mark; false when it would stand still or move backwards, so a
    // caller rewrites the data file only when something was actually read.
    bool setChatReadMark(int userId, int gameId, const std::string& channel,
                         int lastMessageId);
    // Messages in one channel newer than the account's mark.
    int unreadChatMessages(int userId, int gameId, const std::string& channel) const;
    // Every channel of one game added up, for the badge on the server rail.
    int unreadChatMessagesInGame(int userId, int gameId) const;

    // developer tokens
    // Stores one token; 0 when the account already holds its share or the install
    // holds its own. The caller mints and hashes the secret.
    int addDevToken(const DevTokenRecord& record);
    const DevTokenRecord* findDevToken(int id) const;
    // The token this secret hashes to, revoked or not: the caller decides what a
    // revoked token means, because the answer differs between "refuse" and "say it
    // was revoked".
    const DevTokenRecord* findDevTokenByHash(const std::string& tokenHash) const;
    std::vector<const DevTokenRecord*> devTokensFor(int userId) const;
    // Revokes one token the account owns; false when it is not there or is
    // already revoked.
    bool revokeDevToken(int userId, int tokenId, const std::string& nowIso);
    // Records a use of a token that was just accepted. False for an unknown id.
    bool touchDevToken(int id, const std::string& nowIso);
    std::size_t devTokenCount() const { return devTokens_.size(); }

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
    // Drops the oldest messages once a channel or the file is over its bound.
    void trimChat();
    StatsRecord* findOrCreateStats(int userId);
    AvatarRecord* findOrCreateAvatar(int userId);

    // Decrypts one record's sealed half into its plaintext fields, once. False
    // when there is no key or the ciphertext does not authenticate.
    bool openSecrets(UserRecord& user) const;
    // The value to write for one record: the stored blob when the record was
    // never opened, a freshly sealed one when it was.
    std::string sealSecrets(const UserRecord& user) const;
    // A sealed record is bound to the row it came from, so a blob copied into
    // another account fails to open instead of decrypting in the wrong place.
    static std::string userAad(int userId);

    std::string path_;
    std::string dataKey_;
    std::vector<UserRecord> users_;
    std::vector<AvatarRecord> avatars_;
    std::vector<FriendshipRecord> friendships_;
    std::vector<StatsRecord> stats_;
    std::vector<GameRecord> games_;
    std::vector<ReportRecord> reports_;
    std::vector<BlockRecord> blocks_;
    std::vector<AppealRecord> appeals_;
    std::vector<HardwareBanRecord> hardwareBans_;
    std::vector<DevTokenRecord> devTokens_;
    std::vector<ChatMessageRecord> chat_;
    std::vector<ChatReadRecord> chatReads_;
    int nextUserId_ = 1;
    int nextFriendshipId_ = 1;
    int nextGameId_ = 1;
    int nextReportId_ = 1;
    int nextBlockId_ = 1;
    // Mutable because a record that is still sealed holds infractions whose ids
    // have not been seen yet: opening it has to raise this, or the next action
    // would reuse an id an earlier one already carries.
    mutable int nextInfractionId_ = 1;
    int nextAppealId_ = 1;
    int nextDevTokenId_ = 1;
    int nextChatMessageId_ = 1;
};

// Turns key material written by a person into the 32 bytes AES-256 takes: 64 hex
// characters are read as raw bytes, and anything else is hashed with SHA-256 so a
// passphrase still yields a usable key instead of being rejected.
std::string dataKeyFromText(const std::string& text);

}  // namespace web
}  // namespace rblx
