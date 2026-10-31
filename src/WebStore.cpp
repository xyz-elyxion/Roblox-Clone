#include "WebStore.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <utility>

namespace rblx {
namespace web {
namespace {

std::string lowerCopy(const std::string& value) {
    std::string out = value;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

bool readFile(const std::string& path, std::string* out) {
    std::ifstream stream(path.c_str(), std::ios::binary);
    if (!stream) {
        return false;
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    *out = buffer.str();
    return true;
}

bool writeFileAtomically(const std::string& path, const std::string& contents, std::string* error) {
    const std::string temporary = path + ".tmp";
    {
        std::ofstream stream(temporary.c_str(), std::ios::binary | std::ios::trunc);
        if (!stream) {
            if (error) {
                *error = "Could not open " + temporary + " for writing.";
            }
            return false;
        }
        stream.write(contents.data(), static_cast<std::streamsize>(contents.size()));
        stream.flush();
        if (!stream) {
            if (error) {
                *error = "Could not write " + temporary + ".";
            }
            return false;
        }
    }

    std::remove(path.c_str());
    if (std::rename(temporary.c_str(), path.c_str()) != 0) {
        if (error) {
            *error = "Could not replace " + path + ".";
        }
        return false;
    }
    return true;
}

Json tripleToJson(const double values[3]) {
    Json array = Json::makeArray();
    for (int i = 0; i < 3; ++i) {
        array.push(Json(values[i]));
    }
    return array;
}

void readTriple(const Json& array, double out[3]) {
    if (!array.isArray()) {
        return;
    }
    for (std::size_t i = 0; i < array.elements().size() && i < 3; ++i) {
        out[i] = array.elements()[i].asNumber(out[i]);
    }
}

}  // namespace

Store::Store(std::string path) : path_(std::move(path)) {}

void Store::clear() {
    users_.clear();
    avatars_.clear();
    friendships_.clear();
    stats_.clear();
    games_.clear();
    nextUserId_ = 1;
    nextFriendshipId_ = 1;
    nextGameId_ = 1;
}

bool Store::load(std::string* error) {
    clear();
    std::string contents;
    if (!readFile(path_, &contents)) {
        // A missing data file simply means "no accounts yet".
        return true;
    }
    if (contents.empty()) {
        return true;
    }

    std::string parseError;
    const Json document = Json::parse(contents, &parseError);
    if (!document.isObject()) {
        if (error) {
            *error = "Data file " + path_ + " is not valid JSON: " + parseError;
        }
        return false;
    }

    for (const Json& entry : document.member("users").elements()) {
        UserRecord user;
        user.id = static_cast<int>(entry.member("id").asInteger(0));
        user.username = entry.member("username").asString();
        user.passwordHash = entry.member("passwordHash").asString();
        user.createdAt = entry.member("createdAt").asString();
        if (user.id > 0 && !user.username.empty()) {
            users_.push_back(user);
            nextUserId_ = std::max(nextUserId_, user.id + 1);
        }
    }

    for (const Json& entry : document.member("avatars").elements()) {
        AvatarRecord record;
        record.userId = static_cast<int>(entry.member("userId").asInteger(0));
        if (record.userId <= 0) {
            continue;
        }
        const char* slots[kAvatarSlotCount] = {
            "headColor", "torsoColor", "leftArmColor",
            "rightArmColor", "leftLegColor", "rightLegColor"
        };
        for (int slot = 0; slot < kAvatarSlotCount; ++slot) {
            readTriple(entry.member(slots[slot]), record.avatar.colors[slot]);
        }
        const std::string faceId = entry.member("faceId").asString("classic");
        record.avatar.faceId = faceId.empty() ? "classic" : faceId;
        avatars_.push_back(record);
    }

    for (const Json& entry : document.member("friendships").elements()) {
        FriendshipRecord record;
        record.id = static_cast<int>(entry.member("id").asInteger(0));
        record.requesterId = static_cast<int>(entry.member("requesterId").asInteger(0));
        record.addresseeId = static_cast<int>(entry.member("addresseeId").asInteger(0));
        record.status = entry.member("status").asString("pending");
        record.createdAt = entry.member("createdAt").asString();
        record.updatedAt = entry.member("updatedAt").asString();
        if (record.id <= 0 || record.requesterId <= 0 || record.addresseeId <= 0 ||
            record.requesterId == record.addresseeId) {
            continue;
        }
        friendships_.push_back(record);
        nextFriendshipId_ = std::max(nextFriendshipId_, record.id + 1);
    }

    for (const Json& entry : document.member("stats").elements()) {
        StatsRecord record;
        record.userId = static_cast<int>(entry.member("userId").asInteger(0));
        record.playtimeSeconds = entry.member("playtimeSeconds").asInteger(0);
        // Saves written before coins existed have none stored, so they start with
        // the one coin per minute of playtime they would have earned.
        record.coins = entry.member("coins").asInteger(record.playtimeSeconds / kSecondsPerCoin);
        record.lastPlayedAt = entry.member("lastPlayedAt").asString();
        if (record.userId > 0) {
            stats_.push_back(record);
        }
    }

    for (const Json& entry : document.member("games").elements()) {
        GameRecord record;
        record.id = static_cast<int>(entry.member("id").asInteger(0));
        record.ownerId = static_cast<int>(entry.member("ownerId").asInteger(0));
        record.title = entry.member("title").asString();
        record.slug = entry.member("slug").asString();
        record.description = entry.member("description").asString();
        record.isPublic = entry.member("isPublic").asBool(true);
        record.worldPath = entry.member("worldPath").asString();
        record.scriptPath = entry.member("scriptPath").asString();
        record.iconPath = entry.member("iconPath").asString();
        record.bannerPath = entry.member("bannerPath").asString();
        record.partsCount = static_cast<int>(entry.member("partsCount").asInteger(0));
        record.dynamicPartsCount =
            static_cast<int>(entry.member("dynamicPartsCount").asInteger(0));
        record.spawnCount = static_cast<int>(entry.member("spawnCount").asInteger(0));
        record.launchCount = static_cast<int>(entry.member("launchCount").asInteger(0));
        record.createdAt = entry.member("createdAt").asString();
        record.updatedAt = entry.member("updatedAt").asString();
        record.publishedAt = entry.member("publishedAt").asString();
        if (record.id <= 0 || record.ownerId <= 0) {
            continue;
        }
        games_.push_back(record);
        nextGameId_ = std::max(nextGameId_, record.id + 1);
    }

    resolveOwnerNames();
    return true;
}

bool Store::save(std::string* error) const {
    Json document = Json::makeObject();

    Json users = Json::makeArray();
    for (const UserRecord& user : users_) {
        Json entry = Json::makeObject();
        entry.set("id", user.id);
        entry.set("username", user.username);
        entry.set("passwordHash", user.passwordHash);
        entry.set("createdAt", user.createdAt);
        users.push(entry);
    }
    document.set("users", users);

    Json avatars = Json::makeArray();
    for (const AvatarRecord& record : avatars_) {
        Json entry = Json::makeObject();
        entry.set("userId", record.userId);
        const char* slots[kAvatarSlotCount] = {
            "headColor", "torsoColor", "leftArmColor",
            "rightArmColor", "leftLegColor", "rightLegColor"
        };
        for (int slot = 0; slot < kAvatarSlotCount; ++slot) {
            entry.set(slots[slot], tripleToJson(record.avatar.colors[slot]));
        }
        entry.set("faceId", record.avatar.faceId);
        avatars.push(entry);
    }
    document.set("avatars", avatars);

    Json friendships = Json::makeArray();
    for (const FriendshipRecord& record : friendships_) {
        Json entry = Json::makeObject();
        entry.set("id", record.id);
        entry.set("requesterId", record.requesterId);
        entry.set("addresseeId", record.addresseeId);
        entry.set("status", record.status);
        entry.set("createdAt", record.createdAt);
        entry.set("updatedAt", record.updatedAt);
        friendships.push(entry);
    }
    document.set("friendships", friendships);

    Json stats = Json::makeArray();
    for (const StatsRecord& record : stats_) {
        Json entry = Json::makeObject();
        entry.set("userId", record.userId);
        entry.set("playtimeSeconds", record.playtimeSeconds);
        entry.set("coins", record.coins);
        entry.set("lastPlayedAt", record.lastPlayedAt);
        stats.push(entry);
    }
    document.set("stats", stats);

    Json games = Json::makeArray();
    for (const GameRecord& record : games_) {
        Json entry = Json::makeObject();
        entry.set("id", record.id);
        entry.set("ownerId", record.ownerId);
        entry.set("title", record.title);
        entry.set("slug", record.slug);
        entry.set("description", record.description);
        entry.set("isPublic", record.isPublic);
        entry.set("worldPath", record.worldPath);
        entry.set("scriptPath", record.scriptPath);
        entry.set("iconPath", record.iconPath);
        entry.set("bannerPath", record.bannerPath);
        entry.set("partsCount", record.partsCount);
        entry.set("dynamicPartsCount", record.dynamicPartsCount);
        entry.set("spawnCount", record.spawnCount);
        entry.set("launchCount", record.launchCount);
        entry.set("createdAt", record.createdAt);
        entry.set("updatedAt", record.updatedAt);
        entry.set("publishedAt", record.publishedAt);
        games.push(entry);
    }
    document.set("games", games);

    return writeFileAtomically(path_, document.dump(), error);
}

const UserRecord* Store::findUserByName(const std::string& username) const {
    // Case sensitive on purpose: the original SQLite schema used a plain
    // UNIQUE TEXT column, so "Player" and "player" were distinct accounts.
    for (const UserRecord& user : users_) {
        if (user.username == username) {
            return &user;
        }
    }
    return nullptr;
}

const UserRecord* Store::findUserById(int id) const {
    for (const UserRecord& user : users_) {
        if (user.id == id) {
            return &user;
        }
    }
    return nullptr;
}

int Store::createUser(const std::string& username, const std::string& passwordHash,
                      const std::string& nowIso) {
    if (findUserByName(username) != nullptr) {
        return -1;
    }
    UserRecord user;
    user.id = nextUserId_++;
    user.username = username;
    user.passwordHash = passwordHash;
    user.createdAt = nowIso;
    users_.push_back(user);
    return user.id;
}

bool Store::renameUser(int id, const std::string& username) {
    if (findUserByName(username) != nullptr) {
        return false;
    }
    for (UserRecord& user : users_) {
        if (user.id == id) {
            user.username = username;
            // Game records cache the owner's name, so refresh it with the rename.
            resolveOwnerNames();
            return true;
        }
    }
    return false;
}

const UserRecord* Store::firstUser() const {
    for (const UserRecord& user : users_) {
        if (user.id == 1) {
            return &user;
        }
    }
    return users_.empty() ? nullptr : &users_.front();
}

std::vector<const UserRecord*> Store::searchUsers(const std::string& query,
                                                  std::size_t limit) const {
    std::vector<const UserRecord*> matches;
    const std::string needle = lowerCopy(query);
    int numericId = 0;
    if (!query.empty()) {
        char* end = nullptr;
        const long parsed = std::strtol(query.c_str(), &end, 10);
        if (end != nullptr && *end == '\0' && parsed > 0 && parsed < 1000000) {
            numericId = static_cast<int>(parsed);
        }
    }

    if (query.empty()) {
        std::vector<const UserRecord*> ordered;
        for (const UserRecord& user : users_) {
            ordered.push_back(&user);
        }
        std::sort(ordered.begin(), ordered.end(),
                  [](const UserRecord* a, const UserRecord* b) { return a->id > b->id; });
        if (ordered.size() > limit) {
            ordered.resize(limit);
        }
        return ordered;
    }

    for (const UserRecord& user : users_) {
        if (user.id == numericId || lowerCopy(user.username).find(needle) != std::string::npos) {
            matches.push_back(&user);
        }
    }
    std::sort(matches.begin(), matches.end(), [&needle](const UserRecord* a,
                                                        const UserRecord* b) {
        const bool aExact = lowerCopy(a->username) == needle;
        const bool bExact = lowerCopy(b->username) == needle;
        if (aExact != bExact) {
            return aExact;
        }
        return lowerCopy(a->username) < lowerCopy(b->username);
    });
    if (matches.size() > limit) {
        matches.resize(limit);
    }
    return matches;
}

Avatar Store::avatarFor(int userId) const {
    Avatar avatar;
    for (const AvatarRecord& record : avatars_) {
        if (record.userId == userId) {
            return record.avatar;
        }
    }
    return avatar;
}

void Store::setAvatar(int userId, const Avatar& avatar) {
    for (AvatarRecord& record : avatars_) {
        if (record.userId == userId) {
            record.avatar = avatar;
            return;
        }
    }
    AvatarRecord record;
    record.userId = userId;
    record.avatar = avatar;
    avatars_.push_back(record);
}

long long Store::playtimeFor(int userId) const {
    for (const StatsRecord& record : stats_) {
        if (record.userId == userId) {
            return record.playtimeSeconds;
        }
    }
    return 0;
}

long long Store::coinsFor(int userId) const {
    for (const StatsRecord& record : stats_) {
        if (record.userId == userId) {
            return record.coins;
        }
    }
    return 0;
}

std::string Store::lastPlayedAtFor(int userId) const {
    for (const StatsRecord& record : stats_) {
        if (record.userId == userId) {
            return record.lastPlayedAt;
        }
    }
    return std::string();
}

StatsRecord* Store::findOrCreateStats(int userId) {
    for (StatsRecord& record : stats_) {
        if (record.userId == userId) {
            return &record;
        }
    }
    StatsRecord record;
    record.userId = userId;
    stats_.push_back(record);
    return &stats_.back();
}

void Store::addPlaytime(int userId, long long seconds, const std::string& nowIso) {
    StatsRecord* record = findOrCreateStats(userId);
    record->playtimeSeconds += seconds;
    record->coins += seconds / kSecondsPerCoin;
    record->lastPlayedAt = nowIso;
}

const FriendshipRecord* Store::findFriendship(int firstUserId, int secondUserId) const {
    for (const FriendshipRecord& record : friendships_) {
        const bool forward = record.requesterId == firstUserId &&
                             record.addresseeId == secondUserId;
        const bool backward = record.requesterId == secondUserId &&
                              record.addresseeId == firstUserId;
        if (forward || backward) {
            return &record;
        }
    }
    return nullptr;
}

int Store::countAcceptedFriends(int userId) const {
    int count = 0;
    for (const FriendshipRecord& record : friendships_) {
        if (record.status == "accepted" &&
            (record.requesterId == userId || record.addresseeId == userId)) {
            ++count;
        }
    }
    return count;
}

std::vector<const FriendshipRecord*> Store::acceptedFriendships(int userId,
                                                                std::size_t limit) const {
    std::vector<const FriendshipRecord*> records;
    for (const FriendshipRecord& record : friendships_) {
        if (record.status == "accepted" &&
            (record.requesterId == userId || record.addresseeId == userId)) {
            records.push_back(&record);
        }
    }
    std::sort(records.begin(), records.end(),
              [this, userId](const FriendshipRecord* a, const FriendshipRecord* b) {
                  const int aId = a->requesterId == userId ? a->addresseeId : a->requesterId;
                  const int bId = b->requesterId == userId ? b->addresseeId : b->requesterId;
                  const UserRecord* aUser = findUserById(aId);
                  const UserRecord* bUser = findUserById(bId);
                  const std::string aName = aUser ? lowerCopy(aUser->username) : std::string();
                  const std::string bName = bUser ? lowerCopy(bUser->username) : std::string();
                  return aName < bName;
              });
    if (records.size() > limit) {
        records.resize(limit);
    }
    return records;
}

std::vector<const FriendshipRecord*> Store::pendingFriendships(int userId, bool incoming) const {
    std::vector<const FriendshipRecord*> records;
    for (const FriendshipRecord& record : friendships_) {
        if (record.status != "pending") {
            continue;
        }
        if (incoming ? record.addresseeId == userId : record.requesterId == userId) {
            records.push_back(&record);
        }
    }
    std::sort(records.begin(), records.end(),
              [](const FriendshipRecord* a, const FriendshipRecord* b) {
                  return a->createdAt > b->createdAt;
              });
    return records;
}

int Store::addFriendship(int requesterId, int addresseeId, const std::string& nowIso) {
    FriendshipRecord record;
    record.id = nextFriendshipId_++;
    record.requesterId = requesterId;
    record.addresseeId = addresseeId;
    record.status = "pending";
    record.createdAt = nowIso;
    record.updatedAt = nowIso;
    friendships_.push_back(record);
    return record.id;
}

void Store::setFriendshipStatus(int friendshipId, const std::string& status,
                                const std::string& nowIso) {
    for (FriendshipRecord& record : friendships_) {
        if (record.id == friendshipId) {
            record.status = status;
            record.updatedAt = nowIso;
            return;
        }
    }
}

void Store::removeFriendship(int friendshipId) {
    friendships_.erase(
        std::remove_if(friendships_.begin(), friendships_.end(),
                       [friendshipId](const FriendshipRecord& record) {
                           return record.id == friendshipId;
                       }),
        friendships_.end());
}

void Store::removeFriendshipsBetween(int firstUserId, int secondUserId) {
    friendships_.erase(
        std::remove_if(friendships_.begin(), friendships_.end(),
                       [firstUserId, secondUserId](const FriendshipRecord& record) {
                           const bool forward = record.requesterId == firstUserId &&
                                                record.addresseeId == secondUserId;
                           const bool backward = record.requesterId == secondUserId &&
                                                 record.addresseeId == firstUserId;
                           return forward || backward;
                       }),
        friendships_.end());
}

const GameRecord* Store::findGame(int id) const {
    for (const GameRecord& record : games_) {
        if (record.id == id) {
            return &record;
        }
    }
    return nullptr;
}

std::vector<const GameRecord*> Store::allGames() const {
    std::vector<const GameRecord*> records;
    records.reserve(games_.size());
    for (const GameRecord& record : games_) {
        records.push_back(&record);
    }
    std::sort(records.begin(), records.end(), [](const GameRecord* a, const GameRecord* b) {
        if (a->publishedAt != b->publishedAt) {
            return a->publishedAt > b->publishedAt;
        }
        return a->id > b->id;
    });
    return records;
}

std::vector<const GameRecord*> Store::publicGames(std::size_t limit) const {
    std::vector<const GameRecord*> records;
    for (const GameRecord& record : games_) {
        if (record.isPublic) {
            records.push_back(&record);
        }
    }
    std::sort(records.begin(), records.end(), [](const GameRecord* a, const GameRecord* b) {
        if (a->publishedAt != b->publishedAt) {
            return a->publishedAt > b->publishedAt;
        }
        return a->id > b->id;
    });
    if (records.size() > limit) {
        records.resize(limit);
    }
    return records;
}

std::vector<const GameRecord*> Store::gamesOwnedBy(int ownerId) const {
    std::vector<const GameRecord*> records;
    for (const GameRecord& record : games_) {
        if (record.ownerId == ownerId) {
            records.push_back(&record);
        }
    }
    std::sort(records.begin(), records.end(), [](const GameRecord* a, const GameRecord* b) {
        if (a->updatedAt != b->updatedAt) {
            return a->updatedAt > b->updatedAt;
        }
        return a->id > b->id;
    });
    return records;
}

int Store::addGame(const GameRecord& record) {
    GameRecord stored = record;
    stored.id = nextGameId_++;
    if (const UserRecord* owner = findUserById(stored.ownerId)) {
        stored.ownerUsername = owner->username;
    }
    games_.push_back(stored);
    return stored.id;
}

void Store::updateGame(const GameRecord& record) {
    for (GameRecord& stored : games_) {
        if (stored.id == record.id) {
            stored = record;
            if (const UserRecord* owner = findUserById(stored.ownerId)) {
                stored.ownerUsername = owner->username;
            }
            return;
        }
    }
}

void Store::incrementLaunchCount(int gameId) {
    for (GameRecord& record : games_) {
        if (record.id == gameId) {
            record.launchCount += 1;
            return;
        }
    }
}

bool Store::removeGame(int id) {
    for (auto it = games_.begin(); it != games_.end(); ++it) {
        if (it->id == id) {
            games_.erase(it);
            return true;
        }
    }
    return false;
}

void Store::resolveOwnerNames() {
    for (GameRecord& game : games_) {
        game.ownerUsername.clear();
        if (const UserRecord* owner = findUserById(game.ownerId)) {
            game.ownerUsername = owner->username;
        }
    }
}

}  // namespace web
}  // namespace rblx
