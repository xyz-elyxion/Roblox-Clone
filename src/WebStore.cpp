#include "WebStore.h"

#include "HardwareId.h"

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
    reports_.clear();
    blocks_.clear();
    appeals_.clear();
    hardwareBans_.clear();
    nextUserId_ = 1;
    nextFriendshipId_ = 1;
    nextGameId_ = 1;
    nextReportId_ = 1;
    nextBlockId_ = 1;
    nextInfractionId_ = 1;
    nextAppealId_ = 1;
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
        user.recoveryHash = entry.member("recoveryHash").asString();
        user.createdAt = entry.member("createdAt").asString();
        user.lastSeenAtMs = entry.member("lastSeenAtMs").asInteger(0);
        user.tokenVersion = static_cast<int>(entry.member("tokenVersion").asInteger(0));
        user.cheatStrikes = static_cast<int>(entry.member("cheatStrikes").asInteger(0));
        user.suspendedUntilMs = entry.member("suspendedUntilMs").asInteger(0);
        user.banned = entry.member("banned").asBool(false);
        user.moderationReason = entry.member("moderationReason").asString();
        user.moderatedAt = entry.member("moderatedAt").asString();
        user.clientDetections = static_cast<int>(entry.member("clientDetections").asInteger(0));
        user.lastClientDetectionAt = entry.member("lastClientDetectionAt").asString();
        user.lastHardwareId = rblx::hwid::normalize(entry.member("lastHardwareId").asString());
        user.hardwareIdSeenAt = entry.member("hardwareIdSeenAt").asString();
        for (const Json& saved : entry.member("pendingDetections").elements()) {
            PendingDetection pending;
            pending.atMs = saved.member("atMs").asInteger(0);
            pending.reason = saved.member("reason").asString();
            pending.proof = saved.member("proof").asString();
            pending.expected = saved.member("expected").asString();
            pending.replay = saved.member("replay").asString();
            if (pending.reason.empty()) {
                continue;
            }
            user.pendingDetections.push_back(pending);
        }
        for (const Json& saved : entry.member("infractions").elements()) {
            InfractionRecord infraction;
            infraction.id = static_cast<int>(saved.member("id").asInteger(0));
            infraction.kind = saved.member("kind").asString();
            infraction.reason = saved.member("reason").asString();
            infraction.createdAt = saved.member("createdAt").asString();
            infraction.expiresAt = saved.member("expiresAt").asString();
            infraction.strikes = static_cast<int>(saved.member("strikes").asInteger(0));
            infraction.automatic = saved.member("automatic").asBool(false);
            infraction.detections = static_cast<int>(saved.member("detections").asInteger(0));
            infraction.proof = saved.member("proof").asString();
            infraction.expected = saved.member("expected").asString();
            infraction.replay = saved.member("replay").asString();
            if (infraction.id <= 0 || infraction.kind.empty()) {
                continue;
            }
            user.infractions.push_back(infraction);
            nextInfractionId_ = std::max(nextInfractionId_, infraction.id + 1);
        }
        if (user.id > 0 && !user.username.empty()) {
            users_.push_back(user);
            nextUserId_ = std::max(nextUserId_, user.id + 1);
        }
    }

    for (const Json& entry : document.member("hardwareBans").elements()) {
        HardwareBanRecord record;
        record.hardwareId = rblx::hwid::normalize(entry.member("hardwareId").asString());
        record.userId = static_cast<int>(entry.member("userId").asInteger(0));
        record.username = entry.member("username").asString();
        record.reason = entry.member("reason").asString();
        record.createdAt = entry.member("createdAt").asString();
        record.active = entry.member("active").asBool(true);
        record.liftedAt = entry.member("liftedAt").asString();
        if (!rblx::hwid::valid(record.hardwareId)) {
            continue;
        }
        hardwareBans_.push_back(record);
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
        record.avatar.hatId = entry.member("hatId").asString();
        for (const Json& owned : entry.member("ownedHats").elements()) {
            const std::string hatId = owned.asString();
            if (!hatId.empty()) {
                record.ownedHatIds.push_back(hatId);
            }
        }
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

    for (const Json& entry : document.member("reports").elements()) {
        ReportRecord record;
        record.id = static_cast<int>(entry.member("id").asInteger(0));
        record.reporterId = static_cast<int>(entry.member("reporterId").asInteger(0));
        record.reporterUsername = entry.member("reporterUsername").asString();
        record.targetType = entry.member("targetType").asString();
        record.targetId = static_cast<int>(entry.member("targetId").asInteger(0));
        record.targetLabel = entry.member("targetLabel").asString();
        record.reason = entry.member("reason").asString();
        record.createdAt = entry.member("createdAt").asString();
        if (record.id <= 0 || record.targetId <= 0) {
            continue;
        }
        reports_.push_back(record);
        nextReportId_ = std::max(nextReportId_, record.id + 1);
    }

    for (const Json& entry : document.member("blocks").elements()) {
        BlockRecord record;
        record.id = static_cast<int>(entry.member("id").asInteger(0));
        record.blockerId = static_cast<int>(entry.member("blockerId").asInteger(0));
        record.blockedId = static_cast<int>(entry.member("blockedId").asInteger(0));
        record.createdAt = entry.member("createdAt").asString();
        if (record.id <= 0 || record.blockerId <= 0 || record.blockedId <= 0 ||
            record.blockerId == record.blockedId) {
            continue;
        }
        blocks_.push_back(record);
        nextBlockId_ = std::max(nextBlockId_, record.id + 1);
    }

    for (const Json& entry : document.member("appeals").elements()) {
        AppealRecord record;
        record.id = static_cast<int>(entry.member("id").asInteger(0));
        record.userId = static_cast<int>(entry.member("userId").asInteger(0));
        record.username = entry.member("username").asString();
        record.message = entry.member("message").asString();
        record.status = entry.member("status").asString("open");
        record.createdAt = entry.member("createdAt").asString();
        record.reviewedAt = entry.member("reviewedAt").asString();
        record.reviewer = entry.member("reviewer").asString();
        record.reviewNote = entry.member("reviewNote").asString();
        record.infractionId = static_cast<int>(entry.member("infractionId").asInteger(0));
        if (record.id <= 0 || record.userId <= 0 || record.status.empty()) {
            continue;
        }
        appeals_.push_back(record);
        nextAppealId_ = std::max(nextAppealId_, record.id + 1);
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
        entry.set("recoveryHash", user.recoveryHash);
        entry.set("createdAt", user.createdAt);
        entry.set("lastSeenAtMs", user.lastSeenAtMs);
        entry.set("tokenVersion", user.tokenVersion);
        entry.set("cheatStrikes", user.cheatStrikes);
        entry.set("suspendedUntilMs", user.suspendedUntilMs);
        entry.set("banned", user.banned);
        entry.set("moderationReason", user.moderationReason);
        entry.set("moderatedAt", user.moderatedAt);
        entry.set("clientDetections", user.clientDetections);
        entry.set("lastClientDetectionAt", user.lastClientDetectionAt);
        entry.set("lastHardwareId", user.lastHardwareId);
        entry.set("hardwareIdSeenAt", user.hardwareIdSeenAt);
        Json infractions = Json::makeArray();
        for (const InfractionRecord& infraction : user.infractions) {
            Json saved = Json::makeObject();
            saved.set("id", infraction.id);
            saved.set("kind", infraction.kind);
            saved.set("reason", infraction.reason);
            saved.set("createdAt", infraction.createdAt);
            saved.set("expiresAt", infraction.expiresAt);
            saved.set("strikes", infraction.strikes);
            saved.set("automatic", infraction.automatic);
            saved.set("detections", infraction.detections);
            saved.set("proof", infraction.proof);
            saved.set("expected", infraction.expected);
            saved.set("replay", infraction.replay);
            infractions.push(saved);
        }
        entry.set("infractions", infractions);

        Json pending = Json::makeArray();
        for (const PendingDetection& detection : user.pendingDetections) {
            Json saved = Json::makeObject();
            saved.set("atMs", detection.atMs);
            saved.set("reason", detection.reason);
            saved.set("proof", detection.proof);
            saved.set("expected", detection.expected);
            saved.set("replay", detection.replay);
            pending.push(saved);
        }
        entry.set("pendingDetections", pending);
        users.push(entry);
    }
    document.set("users", users);

    Json hardwareBans = Json::makeArray();
    for (const HardwareBanRecord& ban : hardwareBans_) {
        Json entry = Json::makeObject();
        entry.set("hardwareId", ban.hardwareId);
        entry.set("userId", ban.userId);
        entry.set("username", ban.username);
        entry.set("reason", ban.reason);
        entry.set("createdAt", ban.createdAt);
        entry.set("active", ban.active);
        entry.set("liftedAt", ban.liftedAt);
        hardwareBans.push(entry);
    }
    document.set("hardwareBans", hardwareBans);

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
        entry.set("hatId", record.avatar.hatId);
        Json ownedHats = Json::makeArray();
        for (const std::string& hatId : record.ownedHatIds) {
            ownedHats.push(Json(hatId));
        }
        entry.set("ownedHats", ownedHats);
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

    Json reports = Json::makeArray();
    for (const ReportRecord& record : reports_) {
        Json entry = Json::makeObject();
        entry.set("id", record.id);
        entry.set("reporterId", record.reporterId);
        entry.set("reporterUsername", record.reporterUsername);
        entry.set("targetType", record.targetType);
        entry.set("targetId", record.targetId);
        entry.set("targetLabel", record.targetLabel);
        entry.set("reason", record.reason);
        entry.set("createdAt", record.createdAt);
        reports.push(entry);
    }
    document.set("reports", reports);

    Json blocks = Json::makeArray();
    for (const BlockRecord& record : blocks_) {
        Json entry = Json::makeObject();
        entry.set("id", record.id);
        entry.set("blockerId", record.blockerId);
        entry.set("blockedId", record.blockedId);
        entry.set("createdAt", record.createdAt);
        blocks.push(entry);
    }
    document.set("blocks", blocks);

    Json appeals = Json::makeArray();
    for (const AppealRecord& record : appeals_) {
        Json entry = Json::makeObject();
        entry.set("id", record.id);
        entry.set("userId", record.userId);
        entry.set("username", record.username);
        entry.set("message", record.message);
        entry.set("status", record.status);
        entry.set("createdAt", record.createdAt);
        entry.set("reviewedAt", record.reviewedAt);
        entry.set("reviewer", record.reviewer);
        entry.set("reviewNote", record.reviewNote);
        entry.set("infractionId", record.infractionId);
        appeals.push(entry);
    }
    document.set("appeals", appeals);

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

bool Store::setPasswordHash(int id, const std::string& passwordHash) {
    for (UserRecord& user : users_) {
        if (user.id == id) {
            user.passwordHash = passwordHash;
            return true;
        }
    }
    return false;
}

bool Store::setModeration(int id, int strikeCount, long long suspendedUntilMs, bool banned,
                          const std::string& reason, const std::string& nowIso) {
    for (UserRecord& user : users_) {
        if (user.id != id) {
            continue;
        }
        user.cheatStrikes = strikeCount > 0 ? strikeCount : 0;
        user.suspendedUntilMs = suspendedUntilMs > 0 ? suspendedUntilMs : 0;
        user.banned = banned;
        user.moderationReason = reason;
        user.moderatedAt = nowIso;
        return true;
    }
    return false;
}

bool Store::recordClientDetection(int id, const std::string& nowIso) {
    for (UserRecord& user : users_) {
        if (user.id != id) {
            continue;
        }
        ++user.clientDetections;
        user.lastClientDetectionAt = nowIso;
        return true;
    }
    return false;
}

int Store::bumpTokenVersion(int id) {
    for (UserRecord& user : users_) {
        if (user.id == id) {
            ++user.tokenVersion;
            return user.tokenVersion;
        }
    }
    return -1;
}

bool Store::setRecoveryHash(int id, const std::string& recoveryHash) {
    for (UserRecord& user : users_) {
        if (user.id == id) {
            user.recoveryHash = recoveryHash;
            return true;
        }
    }
    return false;
}

bool Store::touchUser(int id, long long nowMs) {
    for (UserRecord& user : users_) {
        if (user.id != id) {
            continue;
        }
        // A single page load is several authenticated calls, so an almost current
        // timestamp is left alone instead of rewriting the file every time.
        if (nowMs - user.lastSeenAtMs < 30 * 1000) {
            return false;
        }
        user.lastSeenAtMs = nowMs;
        return true;
    }
    return false;
}

bool Store::removeUser(int id) {
    const auto user = std::find_if(users_.begin(), users_.end(),
                                   [id](const UserRecord& record) { return record.id == id; });
    if (user == users_.end()) {
        return false;
    }
    users_.erase(user);

    const auto isUser = [id](int userId) { return userId == id; };
    avatars_.erase(std::remove_if(avatars_.begin(), avatars_.end(),
                                  [&](const AvatarRecord& record) {
                                      return isUser(record.userId);
                                  }),
                   avatars_.end());
    stats_.erase(std::remove_if(stats_.begin(), stats_.end(),
                                [&](const StatsRecord& record) { return isUser(record.userId); }),
                 stats_.end());
    friendships_.erase(
        std::remove_if(friendships_.begin(), friendships_.end(),
                       [&](const FriendshipRecord& record) {
                           return isUser(record.requesterId) || isUser(record.addresseeId);
                       }),
        friendships_.end());
    blocks_.erase(std::remove_if(blocks_.begin(), blocks_.end(),
                                 [&](const BlockRecord& record) {
                                     return isUser(record.blockerId) || isUser(record.blockedId);
                                 }),
                  blocks_.end());
    reports_.erase(std::remove_if(reports_.begin(), reports_.end(),
                                  [&](const ReportRecord& record) {
                                      return isUser(record.reporterId);
                                  }),
                   reports_.end());
    appeals_.erase(std::remove_if(appeals_.begin(), appeals_.end(),
                                  [&](const AppealRecord& record) {
                                      return isUser(record.userId);
                                  }),
                   appeals_.end());
    return true;
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

AvatarRecord* Store::findOrCreateAvatar(int userId) {
    for (AvatarRecord& record : avatars_) {
        if (record.userId == userId) {
            return &record;
        }
    }
    AvatarRecord record;
    record.userId = userId;
    avatars_.push_back(record);
    return &avatars_.back();
}

std::vector<std::string> Store::ownedHatsFor(int userId) const {
    for (const AvatarRecord& record : avatars_) {
        if (record.userId == userId) {
            return record.ownedHatIds;
        }
    }
    return std::vector<std::string>();
}

bool Store::ownsHat(int userId, const std::string& hatId) const {
    const std::vector<std::string> owned = ownedHatsFor(userId);
    return std::find(owned.begin(), owned.end(), hatId) != owned.end();
}

bool Store::grantHat(int userId, const std::string& hatId) {
    if (userId <= 0 || hatId.empty() || ownsHat(userId, hatId)) {
        return false;
    }
    findOrCreateAvatar(userId)->ownedHatIds.push_back(hatId);
    return true;
}

bool Store::spendCoins(int userId, long long amount) {
    if (userId <= 0 || amount <= 0) {
        return false;
    }
    StatsRecord* record = findOrCreateStats(userId);
    if (record == nullptr || record->coins < amount) {
        return false;
    }
    record->coins -= amount;
    return true;
}

bool Store::grantCoins(int userId, long long amount) {
    if (userId <= 0 || amount <= 0) {
        return false;
    }
    StatsRecord* record = findOrCreateStats(userId);
    if (record == nullptr) {
        return false;
    }
    record->coins += amount;
    return true;
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

int Store::addReport(const ReportRecord& record) {
    ReportRecord stored = record;
    stored.id = nextReportId_++;
    reports_.push_back(stored);
    return stored.id;
}

std::vector<const ReportRecord*> Store::recentReports(std::size_t limit) const {
    std::vector<const ReportRecord*> out;
    for (auto entry = reports_.rbegin(); entry != reports_.rend() && out.size() < limit; ++entry) {
        out.push_back(&*entry);
    }
    return out;
}

bool Store::addInfraction(int id, const std::string& kind, const std::string& reason,
                          bool automatic, int strikes, const std::string& expiresAt,
                          const std::string& nowIso, int detections,
                          const std::string& proof, const std::string& expected,
                          const std::string& replay) {
    for (UserRecord& user : users_) {
        if (user.id != id) {
            continue;
        }
        InfractionRecord entry;
        entry.id = nextInfractionId_++;
        entry.kind = kind;
        entry.reason = reason;
        entry.createdAt = nowIso;
        entry.expiresAt = expiresAt;
        entry.strikes = strikes > 0 ? strikes : 0;
        entry.automatic = automatic;
        entry.detections = detections > 0 ? detections : 0;
        entry.proof = proof;
        entry.expected = expected;
        entry.replay = replay;
        user.infractions.push_back(entry);
        // The record is a history, not an archive: the oldest entries fall off so
        // one account cannot grow the data file without end.
        if (user.infractions.size() > kMaxInfractionsPerUser) {
            user.infractions.erase(
                user.infractions.begin(),
                user.infractions.begin() + static_cast<std::ptrdiff_t>(
                                             user.infractions.size() - kMaxInfractionsPerUser));
        }
        return true;
    }
    return false;
}

const std::vector<InfractionRecord>& Store::infractionsFor(int userId) const {
    static const std::vector<InfractionRecord> empty;
    for (const UserRecord& user : users_) {
        if (user.id == userId) {
            return user.infractions;
        }
    }
    return empty;
}

const InfractionRecord* Store::findInfraction(int id, int* ownerId) const {
    if (id <= 0) {
        return nullptr;
    }
    for (const UserRecord& user : users_) {
        for (const InfractionRecord& entry : user.infractions) {
            if (entry.id != id) {
                continue;
            }
            if (ownerId != nullptr) {
                *ownerId = user.id;
            }
            return &entry;
        }
    }
    return nullptr;
}

bool Store::addPendingDetection(int id, const std::string& reason, const std::string& proof,
                                const std::string& expected, const std::string& replay,
                                long long atMs) {
    for (UserRecord& user : users_) {
        if (user.id != id) {
            continue;
        }
        PendingDetection pending;
        pending.atMs = atMs;
        pending.reason = reason;
        pending.proof = proof;
        pending.expected = expected;
        pending.replay = replay;
        user.pendingDetections.push_back(pending);
        // The newest are the ones a wave acts on, so the oldest fall off first.
        if (user.pendingDetections.size() > kMaxPendingDetections) {
            user.pendingDetections.erase(
                user.pendingDetections.begin(),
                user.pendingDetections.begin() +
                    static_cast<std::ptrdiff_t>(user.pendingDetections.size() -
                                                kMaxPendingDetections));
        }
        return true;
    }
    return false;
}

const std::vector<PendingDetection>& Store::pendingDetectionsFor(int userId) const {
    static const std::vector<PendingDetection> empty;
    for (const UserRecord& user : users_) {
        if (user.id == userId) {
            return user.pendingDetections;
        }
    }
    return empty;
}

void Store::clearPendingDetections(int userId) {
    for (UserRecord& user : users_) {
        if (user.id == userId) {
            user.pendingDetections.clear();
            return;
        }
    }
}

bool Store::setLastHardwareId(int userId, const std::string& hardwareId,
                              const std::string& nowIso) {
    const std::string normalized = rblx::hwid::normalize(hardwareId);
    if (!rblx::hwid::valid(normalized)) {
        return false;
    }
    for (UserRecord& user : users_) {
        if (user.id == userId) {
            if (user.lastHardwareId == normalized) {
                // The same machine as last time: only the recent-seen time moves.
                user.hardwareIdSeenAt = nowIso;
                return false;
            }
            user.lastHardwareId = normalized;
            user.hardwareIdSeenAt = nowIso;
            return true;
        }
    }
    return false;
}

bool Store::banHardware(const std::string& hardwareId, int userId, const std::string& username,
                        const std::string& reason, const std::string& nowIso) {
    const std::string normalized = rblx::hwid::normalize(hardwareId);
    if (!rblx::hwid::valid(normalized)) {
        return false;
    }
    for (HardwareBanRecord& ban : hardwareBans_) {
        if (ban.hardwareId == normalized && ban.active) {
            return false;
        }
    }

    HardwareBanRecord ban;
    ban.hardwareId = normalized;
    ban.userId = userId;
    ban.username = username;
    ban.reason = reason;
    ban.createdAt = nowIso;
    ban.active = true;
    hardwareBans_.push_back(ban);

    // Active bans are never trimmed: dropping one would silently unban a machine.
    // Lifted rows are only history, so the oldest go first.
    while (hardwareBans_.size() > kMaxHardwareBans) {
        std::size_t index = 0;
        while (index < hardwareBans_.size() && hardwareBans_[index].active) {
            ++index;
        }
        if (index == hardwareBans_.size()) {
            break;
        }
        hardwareBans_.erase(hardwareBans_.begin() + static_cast<std::ptrdiff_t>(index));
    }
    return true;
}

const HardwareBanRecord* Store::activeHardwareBan(const std::string& hardwareId) const {
    const std::string normalized = rblx::hwid::normalize(hardwareId);
    if (!rblx::hwid::valid(normalized)) {
        return nullptr;
    }
    for (const HardwareBanRecord& ban : hardwareBans_) {
        if (ban.active && ban.hardwareId == normalized) {
            return &ban;
        }
    }
    return nullptr;
}

bool Store::liftHardwareBan(const std::string& hardwareId, const std::string& nowIso) {
    const std::string normalized = rblx::hwid::normalize(hardwareId);
    for (HardwareBanRecord& ban : hardwareBans_) {
        if (ban.active && ban.hardwareId == normalized) {
            ban.active = false;
            ban.liftedAt = nowIso;
            return true;
        }
    }
    return false;
}

int Store::liftHardwareBansForUser(int userId, const std::string& nowIso) {
    int lifted = 0;
    for (HardwareBanRecord& ban : hardwareBans_) {
        if (ban.active && ban.userId == userId) {
            ban.active = false;
            ban.liftedAt = nowIso;
            ++lifted;
        }
    }
    return lifted;
}

int Store::addAppeal(const AppealRecord& record) {
    AppealRecord stored = record;
    stored.id = nextAppealId_++;
    appeals_.push_back(stored);
    return stored.id;
}

const AppealRecord* Store::findAppeal(int id) const {
    for (const AppealRecord& record : appeals_) {
        if (record.id == id) {
            return &record;
        }
    }
    return nullptr;
}

const AppealRecord* Store::appealForInfraction(int infractionId) const {
    if (infractionId <= 0) {
        return nullptr;
    }
    for (const AppealRecord& record : appeals_) {
        if (record.infractionId == infractionId) {
            return &record;
        }
    }
    return nullptr;
}

const AppealRecord* Store::openAppealFor(int userId) const {
    for (const AppealRecord& record : appeals_) {
        if (record.userId == userId && record.status == "open") {
            return &record;
        }
    }
    return nullptr;
}

std::vector<const AppealRecord*> Store::appealsFor(int userId) const {
    std::vector<const AppealRecord*> out;
    for (auto entry = appeals_.rbegin(); entry != appeals_.rend(); ++entry) {
        if (entry->userId == userId) {
            out.push_back(&*entry);
        }
    }
    return out;
}

std::vector<const AppealRecord*> Store::recentAppeals(std::size_t limit) const {
    std::vector<const AppealRecord*> out;
    for (auto entry = appeals_.rbegin(); entry != appeals_.rend() && out.size() < limit; ++entry) {
        out.push_back(&*entry);
    }
    return out;
}

std::size_t Store::openAppealCount() const {
    std::size_t count = 0;
    for (const AppealRecord& record : appeals_) {
        if (record.status == "open") {
            ++count;
        }
    }
    return count;
}

bool Store::reviewAppeal(int id, const std::string& status, const std::string& reviewer,
                         const std::string& note, const std::string& nowIso) {
    for (AppealRecord& record : appeals_) {
        if (record.id != id) {
            continue;
        }
        record.status = status;
        record.reviewer = reviewer;
        record.reviewNote = note;
        record.reviewedAt = nowIso;
        return true;
    }
    return false;
}

bool Store::addBlock(int blockerId, int blockedId, const std::string& nowIso) {
    if (blockerId <= 0 || blockedId <= 0 || blockerId == blockedId) {
        return false;
    }
    if (isBlocked(blockerId, blockedId)) {
        return true;
    }
    BlockRecord record;
    record.id = nextBlockId_++;
    record.blockerId = blockerId;
    record.blockedId = blockedId;
    record.createdAt = nowIso;
    blocks_.push_back(record);
    return true;
}

bool Store::removeBlock(int blockerId, int blockedId) {
    const auto entry = std::find_if(blocks_.begin(), blocks_.end(),
                                    [&](const BlockRecord& record) {
                                        return record.blockerId == blockerId &&
                                               record.blockedId == blockedId;
                                    });
    if (entry == blocks_.end()) {
        return false;
    }
    blocks_.erase(entry);
    return true;
}

bool Store::isBlocked(int blockerId, int blockedId) const {
    for (const BlockRecord& record : blocks_) {
        if (record.blockerId == blockerId && record.blockedId == blockedId) {
            return true;
        }
    }
    return false;
}

bool Store::blockedEitherWay(int firstUserId, int secondUserId) const {
    return isBlocked(firstUserId, secondUserId) || isBlocked(secondUserId, firstUserId);
}

std::vector<const BlockRecord*> Store::blocksBy(int userId) const {
    std::vector<const BlockRecord*> out;
    for (const BlockRecord& record : blocks_) {
        if (record.blockerId == userId) {
            out.push_back(&record);
        }
    }
    return out;
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
