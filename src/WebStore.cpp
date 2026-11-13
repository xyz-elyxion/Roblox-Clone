#include "WebStore.h"

#include "Roles.h"

#include "ChatRoom.h"
#include "HardwareId.h"
#include "WebCrypto.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
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

// The moderation record and detection queue travel together with the credential
// fields inside one sealed blob, so these four helpers serve the sealed payload,
// a file written before the format was sealed, and a file written without a key.
Json infractionsToJson(const std::vector<InfractionRecord>& entries) {
    Json array = Json::makeArray();
    for (const InfractionRecord& infraction : entries) {
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
        array.push(saved);
    }
    return array;
}

Json pendingToJson(const std::vector<PendingDetection>& entries) {
    Json array = Json::makeArray();
    for (const PendingDetection& detection : entries) {
        Json saved = Json::makeObject();
        saved.set("atMs", detection.atMs);
        saved.set("reason", detection.reason);
        saved.set("proof", detection.proof);
        saved.set("expected", detection.expected);
        saved.set("replay", detection.replay);
        array.push(saved);
    }
    return array;
}

void readInfractions(const Json& array, std::vector<InfractionRecord>* out, int* nextId) {
    for (const Json& saved : array.elements()) {
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
        out->push_back(infraction);
        if (nextId != nullptr) {
            *nextId = std::max(*nextId, infraction.id + 1);
        }
    }
}

void readPending(const Json& array, std::vector<PendingDetection>* out) {
    for (const Json& saved : array.elements()) {
        PendingDetection pending;
        pending.atMs = saved.member("atMs").asInteger(0);
        pending.reason = saved.member("reason").asString();
        pending.proof = saved.member("proof").asString();
        pending.expected = saved.member("expected").asString();
        pending.replay = saved.member("replay").asString();
        if (pending.reason.empty()) {
            continue;
        }
        out->push_back(pending);
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
    chat_.clear();
    chatReads_.clear();
    likes_.clear();
    nextUserId_ = 1;
    nextFriendshipId_ = 1;
    nextGameId_ = 1;
    nextReportId_ = 1;
    nextBlockId_ = 1;
    nextInfractionId_ = 1;
    nextAppealId_ = 1;
    nextChatMessageId_ = 1;
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

    // A file written by this version names its layout and seals each record; an
    // older one has the same fields in the clear. A sealed file without the key
    // that made it is refused outright: half-reading it would leave every account
    // with no password hash, which looks exactly like a file nobody can sign into.
    const bool sealedFile = document.member("format").asString() == "rblx-store/2";
    if (sealedFile && !hasDataKey()) {
        if (error) {
            *error = "Data file " + path_ +
                     " is sealed with AES-256-GCM; set DATA_KEY to the key it was written with.";
        }
        return false;
    }

    for (const Json& entry : document.member("users").elements()) {
        UserRecord user;
        user.id = static_cast<int>(entry.member("id").asInteger(0));
        user.username = entry.member("username").asString();
        user.createdAt = entry.member("createdAt").asString();
        // An id this build does not know is read back as the default role: a
        // stored string no page can interpret is worse than no role at all.
        user.role = rblx::roles::find(entry.member("role").asString()).id;
        user.lastSeenAtMs = entry.member("lastSeenAtMs").asInteger(0);
        user.tokenVersion = static_cast<int>(entry.member("tokenVersion").asInteger(0));
        user.cheatStrikes = static_cast<int>(entry.member("cheatStrikes").asInteger(0));
        user.suspendedUntilMs = entry.member("suspendedUntilMs").asInteger(0);
        user.banned = entry.member("banned").asBool(false);
        user.moderationReason = entry.member("moderationReason").asString();
        user.moderatedAt = entry.member("moderatedAt").asString();
        user.clientDetections = static_cast<int>(entry.member("clientDetections").asInteger(0));
        user.lastClientDetectionAt = entry.member("lastClientDetectionAt").asString();

        // The secret half: a record written by this version carries it sealed and
        // unread, so loading a file touches no cipher at all. A file from before
        // the format was sealed, or one written without a key, has the same fields
        // in the clear and is sealed the next time it is saved with a key.
        user.sealedSecrets = entry.member("sealed").asString();
        user.secretsOpen = user.sealedSecrets.empty();
        if (user.secretsOpen) {
            user.passwordHash = entry.member("passwordHash").asString();
            user.recoveryHash = entry.member("recoveryHash").asString();
            user.lastHardwareId = rblx::hwid::normalize(entry.member("lastHardwareId").asString());
            user.hardwareIdSeenAt = entry.member("hardwareIdSeenAt").asString();
            readPending(entry.member("pendingDetections"), &user.pendingDetections);
            readInfractions(entry.member("infractions"), &user.infractions, &nextInfractionId_);
            user.pendingDetectionCount = static_cast<int>(user.pendingDetections.size());
            user.oldestPendingAtMs =
                user.pendingDetections.empty() ? 0 : user.pendingDetections.front().atMs;
        } else {
            // What the ban sweep needs to find this record without opening it.
            user.pendingDetectionCount =
                static_cast<int>(entry.member("pendingDetectionCount").asInteger(0));
            user.oldestPendingAtMs = entry.member("oldestPendingAtMs").asInteger(0);
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
        record.aiCompatible = entry.member("aiCompatible").asBool(false);
        record.aiCourse = entry.member("aiCourse").asString("map");
        record.aiGoal = entry.member("aiGoal").asString();
        if (record.id <= 0 || record.ownerId <= 0) {
            continue;
        }
        games_.push_back(record);
        nextGameId_ = std::max(nextGameId_, record.id + 1);
    }

    for (const Json& entry : document.member("devTokens").elements()) {
        DevTokenRecord record;
        record.id = static_cast<int>(entry.member("id").asInteger(0));
        record.userId = static_cast<int>(entry.member("userId").asInteger(0));
        record.username = entry.member("username").asString();
        record.label = entry.member("label").asString();
        record.scope = entry.member("scope").asString("ai");
        record.tokenHash = entry.member("tokenHash").asString();
        record.createdAt = entry.member("createdAt").asString();
        record.lastUsedAt = entry.member("lastUsedAt").asString();
        record.revoked = entry.member("revoked").asBool(false);
        record.revokedAt = entry.member("revokedAt").asString();
        // A row without a hash cannot authenticate anything, so it is not a token
        // and is dropped instead of kept as an entry that never matches.
        if (record.id <= 0 || record.userId <= 0 || record.tokenHash.empty()) {
            continue;
        }
        devTokens_.push_back(record);
        nextDevTokenId_ = std::max(nextDevTokenId_, record.id + 1);
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

    for (const Json& entry : document.member("chat").elements()) {
        ChatMessageRecord record;
        record.id = static_cast<int>(entry.member("id").asInteger(0));
        record.gameId = static_cast<int>(entry.member("gameId").asInteger(0));
        record.channel = entry.member("channel").asString();
        record.userId = static_cast<int>(entry.member("userId").asInteger(0));
        record.username = entry.member("username").asString();
        record.body = entry.member("body").asString();
        record.createdAt = entry.member("createdAt").asString();
        record.atMs = entry.member("atMs").asInteger(0);
        // A row without a room, an author or text cannot be shown anywhere, so
        // it is dropped rather than carried around.
        if (record.id <= 0 || record.gameId <= 0 || record.userId <= 0 ||
            record.channel.empty() || record.body.empty()) {
            continue;
        }
        chat_.push_back(record);
        nextChatMessageId_ = std::max(nextChatMessageId_, record.id + 1);
    }

    for (const Json& entry : document.member("chatReads").elements()) {
        ChatReadRecord record;
        record.userId = static_cast<int>(entry.member("userId").asInteger(0));
        record.gameId = static_cast<int>(entry.member("gameId").asInteger(0));
        record.channel = entry.member("channel").asString();
        record.lastMessageId = static_cast<int>(entry.member("lastMessageId").asInteger(0));
        if (record.userId <= 0 || record.gameId <= 0 || record.lastMessageId <= 0 ||
            chat::findChannel(record.channel) == nullptr) {
            continue;
        }
        chatReads_.push_back(record);
    }

    for (const Json& entry : document.member("gameLikes").elements()) {
        GameLikeRecord record;
        record.userId = static_cast<int>(entry.member("userId").asInteger(0));
        record.gameId = static_cast<int>(entry.member("gameId").asInteger(0));
        record.createdAt = entry.member("createdAt").asString();
        // A like without its account or its game counts for nothing, and a second
        // row for the same pair would count twice.
        if (record.userId <= 0 || record.gameId <= 0) {
            continue;
        }
        if (findUserById(record.userId) == nullptr || findGame(record.gameId) == nullptr) {
            continue;
        }
        if (hasLikedGame(record.userId, record.gameId)) {
            continue;
        }
        likes_.push_back(record);
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
    // Names the layout and the cipher: a sealed file must not be mistaken for one
    // written before records were sealed, and a second cipher can appear beside
    // this one instead of replacing it.
    document.set("format", hasDataKey() ? "rblx-store/2" : "rblx-store/1");
    if (hasDataKey()) {
        document.set("cipher", "aes-256-gcm");
    }

    Json users = Json::makeArray();
    for (const UserRecord& user : users_) {
        Json entry = Json::makeObject();
        entry.set("id", user.id);
        entry.set("username", user.username);
        entry.set("createdAt", user.createdAt);
        entry.set("role", user.role);
        entry.set("lastSeenAtMs", user.lastSeenAtMs);
        entry.set("tokenVersion", user.tokenVersion);
        entry.set("cheatStrikes", user.cheatStrikes);
        entry.set("suspendedUntilMs", user.suspendedUntilMs);
        entry.set("banned", user.banned);
        entry.set("moderationReason", user.moderationReason);
        entry.set("moderatedAt", user.moderatedAt);
        entry.set("clientDetections", user.clientDetections);
        entry.set("lastClientDetectionAt", user.lastClientDetectionAt);
        entry.set("pendingDetectionCount", user.pendingDetectionCount);
        entry.set("oldestPendingAtMs", user.oldestPendingAtMs);
        if (hasDataKey()) {
            // One blob per account, sealed with a fresh nonce every save. A record
            // nothing has asked for is written back exactly as it was read, so a
            // save never decrypts a record it does not have to.
            entry.set("sealed", sealSecrets(user));
        } else {
            // No key was ever set, so the install keeps the layout it had before
            // the file could be sealed rather than losing its data.
            entry.set("passwordHash", user.passwordHash);
            entry.set("recoveryHash", user.recoveryHash);
            entry.set("lastHardwareId", user.lastHardwareId);
            entry.set("hardwareIdSeenAt", user.hardwareIdSeenAt);
            entry.set("infractions", infractionsToJson(user.infractions));
            entry.set("pendingDetections", pendingToJson(user.pendingDetections));
        }
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
        entry.set("aiCompatible", record.aiCompatible);
        entry.set("aiCourse", record.aiCourse);
        entry.set("aiGoal", record.aiGoal);
        games.push(entry);
    }
    document.set("games", games);

    Json devTokens = Json::makeArray();
    for (const DevTokenRecord& record : devTokens_) {
        Json entry = Json::makeObject();
        entry.set("id", record.id);
        entry.set("userId", record.userId);
        entry.set("username", record.username);
        entry.set("label", record.label);
        entry.set("scope", record.scope);
        entry.set("tokenHash", record.tokenHash);
        entry.set("createdAt", record.createdAt);
        entry.set("lastUsedAt", record.lastUsedAt);
        entry.set("revoked", record.revoked);
        entry.set("revokedAt", record.revokedAt);
        devTokens.push(entry);
    }
    document.set("devTokens", devTokens);

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

    Json chat = Json::makeArray();
    for (const ChatMessageRecord& record : chat_) {
        Json entry = Json::makeObject();
        entry.set("id", record.id);
        entry.set("gameId", record.gameId);
        entry.set("channel", record.channel);
        entry.set("userId", record.userId);
        entry.set("username", record.username);
        entry.set("body", record.body);
        entry.set("createdAt", record.createdAt);
        entry.set("atMs", record.atMs);
        chat.push(entry);
    }
    document.set("chat", chat);

    Json chatReads = Json::makeArray();
    for (const ChatReadRecord& record : chatReads_) {
        Json entry = Json::makeObject();
        entry.set("userId", record.userId);
        entry.set("gameId", record.gameId);
        entry.set("channel", record.channel);
        entry.set("lastMessageId", record.lastMessageId);
        chatReads.push(entry);
    }
    document.set("chatReads", chatReads);

    Json gameLikes = Json::makeArray();
    for (const GameLikeRecord& record : likes_) {
        Json entry = Json::makeObject();
        entry.set("userId", record.userId);
        entry.set("gameId", record.gameId);
        entry.set("createdAt", record.createdAt);
        gameLikes.push(entry);
    }
    document.set("gameLikes", gameLikes);

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
            // This is the account the request is about, so it is the one record
            // that gets opened. Every other account stays sealed.
            openSecrets(const_cast<UserRecord&>(user));
            return &user;
        }
    }
    return nullptr;
}

const UserRecord* Store::findUserById(int id) const {
    for (const UserRecord& user : users_) {
        if (user.id == id) {
            openSecrets(const_cast<UserRecord&>(user));
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
            openSecrets(user);
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

bool Store::setRole(int id, const std::string& roleId) {
    if (!rblx::roles::isKnown(roleId)) {
        return false;
    }
    const std::string normalized = rblx::roles::find(roleId).id;
    for (UserRecord& user : users_) {
        if (user.id != id) {
            continue;
        }
        user.role = normalized;
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
            openSecrets(user);
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
    likes_.erase(std::remove_if(likes_.begin(), likes_.end(),
                                [&](const GameLikeRecord& record) {
                                    return isUser(record.userId);
                                }),
                 likes_.end());
    removeChatMessagesForUser(id);
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
        openSecrets(user);
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
            openSecrets(const_cast<UserRecord&>(user));
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
        openSecrets(const_cast<UserRecord&>(user));
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
        openSecrets(user);
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
        // The header copies the ban sweep reads have to move with the queue, or a
        // detection that arrives while the process runs would never be acted on.
        user.pendingDetectionCount = static_cast<int>(user.pendingDetections.size());
        user.oldestPendingAtMs = user.pendingDetections.front().atMs;
        return true;
    }
    return false;
}

const std::vector<PendingDetection>& Store::pendingDetectionsFor(int userId) const {
    static const std::vector<PendingDetection> empty;
    for (const UserRecord& user : users_) {
        if (user.id == userId) {
            openSecrets(const_cast<UserRecord&>(user));
            return user.pendingDetections;
        }
    }
    return empty;
}

void Store::clearPendingDetections(int userId) {
    for (UserRecord& user : users_) {
        if (user.id == userId) {
            openSecrets(user);
            user.pendingDetections.clear();
            user.pendingDetectionCount = 0;
            user.oldestPendingAtMs = 0;
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
            openSecrets(user);
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

int Store::addChatMessage(const ChatMessageRecord& record) {
    if (record.gameId <= 0 || record.userId <= 0 || record.channel.empty() ||
        record.body.empty() || chat::findChannel(record.channel) == nullptr) {
        return 0;
    }
    ChatMessageRecord stored = record;
    stored.id = nextChatMessageId_++;
    chat_.push_back(stored);
    trimChat();
    return stored.id;
}

std::vector<const ChatMessageRecord*> Store::chatMessages(int gameId, const std::string& channel,
                                                          int afterId,
                                                          std::size_t limit) const {
    std::vector<const ChatMessageRecord*> out;
    // Walk from the newest end so the page is the last `limit` messages, then
    // put it back in reading order.
    for (auto it = chat_.rbegin(); it != chat_.rend() && out.size() < limit; ++it) {
        if (it->gameId != gameId || it->channel != channel) {
            continue;
        }
        // Ids rise with time, so the first message at or below the mark means
        // everything older is old news for this reader.
        if (afterId > 0 && it->id <= afterId) {
            break;
        }
        out.push_back(&*it);
    }
    std::reverse(out.begin(), out.end());
    return out;
}

void Store::removeChatMessagesForGame(int gameId) {
    chat_.erase(std::remove_if(chat_.begin(), chat_.end(),
                               [gameId](const ChatMessageRecord& record) {
                                   return record.gameId == gameId;
                               }),
                chat_.end());
    // A read mark without its room would keep a badge alive for a game that is
    // gone, so the marks go with the messages.
    chatReads_.erase(std::remove_if(chatReads_.begin(), chatReads_.end(),
                                    [gameId](const ChatReadRecord& record) {
                                        return record.gameId == gameId;
                                    }),
                     chatReads_.end());
}

void Store::removeChatMessagesForUser(int userId) {
    chat_.erase(std::remove_if(chat_.begin(), chat_.end(),
                               [userId](const ChatMessageRecord& record) {
                                   return record.userId == userId;
                               }),
                chat_.end());
    chatReads_.erase(std::remove_if(chatReads_.begin(), chatReads_.end(),
                                    [userId](const ChatReadRecord& record) {
                                        return record.userId == userId;
                                    }),
                     chatReads_.end());
}

int Store::chatReadMark(int userId, int gameId, const std::string& channel) const {
    for (const ChatReadRecord& record : chatReads_) {
        if (record.userId == userId && record.gameId == gameId &&
            record.channel == channel) {
            return record.lastMessageId;
        }
    }
    return 0;
}

bool Store::setChatReadMark(int userId, int gameId, const std::string& channel,
                           int lastMessageId) {
    if (userId <= 0 || gameId <= 0 || lastMessageId <= 0 ||
        chat::findChannel(channel) == nullptr) {
        return false;
    }
    for (ChatReadRecord& record : chatReads_) {
        if (record.userId != userId || record.gameId != gameId ||
            record.channel != channel) {
            continue;
        }
        if (lastMessageId <= record.lastMessageId) {
            return false;
        }
        record.lastMessageId = lastMessageId;
        return true;
    }

    ChatReadRecord record;
    record.userId = userId;
    record.gameId = gameId;
    record.channel = channel;
    record.lastMessageId = lastMessageId;
    chatReads_.push_back(record);
    return true;
}

int Store::unreadChatMessages(int userId, int gameId, const std::string& channel) const {
    const int mark = chatReadMark(userId, gameId, channel);
    int unread = 0;
    for (const ChatMessageRecord& record : chat_) {
        if (record.gameId == gameId && record.channel == channel && record.id > mark) {
            ++unread;
        }
    }
    return unread;
}

int Store::unreadChatMessagesInGame(int userId, int gameId) const {
    int unread = 0;
    for (const chat::Channel& channel : chat::channels()) {
        unread += unreadChatMessages(userId, gameId, channel.id);
    }
    return unread;
}

void Store::trimChat() {
    if (chat_.size() <= chat::kMaxMessagesPerChannel &&
        chat_.size() <= chat::kMaxMessagesTotal) {
        return;
    }

    // Newest first, keeping a channel's allowance and the file's; everything the
    // walk never reached is what gets dropped. Per-channel order is preserved by
    // reversing the kept rows back afterwards.
    std::vector<ChatMessageRecord> kept;
    kept.reserve(chat_.size());
    std::map<std::pair<int, std::string>, std::size_t> perChannel;
    for (auto it = chat_.rbegin(); it != chat_.rend(); ++it) {
        if (kept.size() >= chat::kMaxMessagesTotal) {
            break;
        }
        std::size_t& count = perChannel[std::make_pair(it->gameId, it->channel)];
        if (count >= chat::kMaxMessagesPerChannel) {
            continue;
        }
        ++count;
        kept.push_back(*it);
    }
    std::reverse(kept.begin(), kept.end());
    chat_.swap(kept);
}

int Store::addDevToken(const DevTokenRecord& record) {
    if (record.userId <= 0 || record.tokenHash.empty()) {
        return 0;
    }
    std::size_t live = 0;
    for (const DevTokenRecord& token : devTokens_) {
        if (token.userId == record.userId && !token.revoked) {
            ++live;
        }
    }
    if (live >= kMaxDevTokensPerUser || devTokens_.size() >= kMaxDevTokens) {
        return 0;
    }
    DevTokenRecord stored = record;
    stored.id = nextDevTokenId_++;
    if (stored.scope.empty()) {
        stored.scope = "ai";
    }
    devTokens_.push_back(stored);
    return stored.id;
}

const DevTokenRecord* Store::findDevToken(int id) const {
    for (const DevTokenRecord& token : devTokens_) {
        if (token.id == id) {
            return &token;
        }
    }
    return nullptr;
}

const DevTokenRecord* Store::findDevTokenByHash(const std::string& tokenHash) const {
    if (tokenHash.empty()) {
        return nullptr;
    }
    for (const DevTokenRecord& token : devTokens_) {
        if (token.tokenHash == tokenHash) {
            return &token;
        }
    }
    return nullptr;
}

std::vector<const DevTokenRecord*> Store::devTokensFor(int userId) const {
    std::vector<const DevTokenRecord*> tokens;
    for (const DevTokenRecord& token : devTokens_) {
        if (token.userId == userId) {
            tokens.push_back(&token);
        }
    }
    return tokens;
}

bool Store::revokeDevToken(int userId, int tokenId, const std::string& nowIso) {
    for (DevTokenRecord& token : devTokens_) {
        if (token.id != tokenId || token.userId != userId) {
            continue;
        }
        if (token.revoked) {
            return false;
        }
        token.revoked = true;
        token.revokedAt = nowIso;
        return true;
    }
    return false;
}

bool Store::touchDevToken(int id, const std::string& nowIso) {
    for (DevTokenRecord& token : devTokens_) {
        if (token.id == id) {
            token.lastUsedAt = nowIso;
            return true;
        }
    }
    return false;
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
            // Deleting a game deletes its rooms: a message addressed to a game
            // that no longer exists is unreachable text in the data file.
            removeChatMessagesForGame(id);
            // A like on a game that is gone is a row nobody can ever see.
            likes_.erase(std::remove_if(likes_.begin(), likes_.end(),
                                        [id](const GameLikeRecord& record) {
                                            return record.gameId == id;
                                        }),
                         likes_.end());
            return true;
        }
    }
    return false;
}

int Store::gameLikeCount(int gameId) const {
    if (gameId <= 0) {
        return 0;
    }

    int count = 0;
    for (const GameLikeRecord& record : likes_) {
        if (record.gameId == gameId) {
            ++count;
        }
    }
    return count;
}

bool Store::hasLikedGame(int userId, int gameId) const {
    for (const GameLikeRecord& record : likes_) {
        if (record.userId == userId && record.gameId == gameId) {
            return true;
        }
    }
    return false;
}

bool Store::toggleGameLike(int userId, int gameId, const std::string& nowIso, bool* liked) {
    if (userId <= 0 || gameId <= 0) {
        return false;
    }
    if (findGame(gameId) == nullptr || findUserById(userId) == nullptr) {
        return false;
    }

    for (auto it = likes_.begin(); it != likes_.end(); ++it) {
        if (it->userId != userId || it->gameId != gameId) {
            continue;
        }
        likes_.erase(it);
        if (liked != nullptr) {
            *liked = false;
        }
        return true;
    }

    GameLikeRecord record;
    record.userId = userId;
    record.gameId = gameId;
    record.createdAt = nowIso;
    likes_.push_back(record);
    if (liked != nullptr) {
        *liked = true;
    }
    return true;
}

void Store::resolveOwnerNames() {
    for (GameRecord& game : games_) {
        game.ownerUsername.clear();
        if (const UserRecord* owner = findUserById(game.ownerId)) {
            game.ownerUsername = owner->username;
        }
    }
}

std::string Store::userAad(int userId) {
    return "rblx-store-user:" + std::to_string(userId);
}

bool Store::openSecrets(UserRecord& user) const {
    if (user.secretsOpen) {
        return true;
    }
    if (!hasDataKey()) {
        return false;
    }

    std::string plaintext;
    if (!aes256GcmOpen(dataKey_, user.sealedSecrets, userAad(user.id), &plaintext)) {
        // A wrong key or a damaged blob leaves the record closed rather than
        // half-filled. An empty password hash refuses every sign-in, so the
        // failure is a lockout with a reason in the log, not a way in.
        std::fprintf(stderr,
                     "rblx-web: could not open the sealed record for account %d "
                     "(wrong DATA_KEY or a damaged data file)\n",
                     user.id);
        return false;
    }

    std::string parseError;
    const Json secret = Json::parse(plaintext, &parseError);
    if (!secret.isObject()) {
        std::fprintf(stderr, "rblx-web: the sealed record for account %d is not readable\n",
                     user.id);
        return false;
    }

    user.passwordHash = secret.member("passwordHash").asString();
    user.recoveryHash = secret.member("recoveryHash").asString();
    user.lastHardwareId = rblx::hwid::normalize(secret.member("lastHardwareId").asString());
    user.hardwareIdSeenAt = secret.member("hardwareIdSeenAt").asString();

    user.infractions.clear();
    readInfractions(secret.member("infractions"), &user.infractions, &nextInfractionId_);
    user.pendingDetections.clear();
    readPending(secret.member("pendingDetections"), &user.pendingDetections);
    user.pendingDetectionCount = static_cast<int>(user.pendingDetections.size());
    user.oldestPendingAtMs =
        user.pendingDetections.empty() ? 0 : user.pendingDetections.front().atMs;

    user.secretsOpen = true;
    return true;
}

std::string Store::sealSecrets(const UserRecord& user) const {
    if (!user.secretsOpen) {
        // Nothing asked to read this account, so its blob is written back exactly
        // as it arrived and the cipher is never run for it.
        return user.sealedSecrets;
    }
    if (!hasDataKey()) {
        return std::string();
    }

    Json secret = Json::makeObject();
    secret.set("passwordHash", user.passwordHash);
    secret.set("recoveryHash", user.recoveryHash);
    secret.set("lastHardwareId", user.lastHardwareId);
    secret.set("hardwareIdSeenAt", user.hardwareIdSeenAt);
    secret.set("infractions", infractionsToJson(user.infractions));
    secret.set("pendingDetections", pendingToJson(user.pendingDetections));

    const std::string sealed = aes256GcmSeal(dataKey_, secret.dump(), userAad(user.id));
    if (sealed.empty()) {
        // The random nonce could not be drawn. Keeping the previous blob is the
        // only answer that does not throw the account's record away.
        return user.sealedSecrets;
    }
    user.sealedSecrets = sealed;
    return sealed;
}

std::vector<std::pair<int, long long>> Store::queuedDetections() const {
    std::vector<std::pair<int, long long>> queued;
    for (const UserRecord& user : users_) {
        if (user.pendingDetectionCount > 0 && user.oldestPendingAtMs > 0) {
            queued.emplace_back(user.id, user.oldestPendingAtMs);
        }
    }
    return queued;
}

std::string dataKeyFromText(const std::string& text) {
    if (text.size() == 64) {
        bool hexadecimal = true;
        for (std::size_t i = 0; i < text.size(); ++i) {
            if (!std::isxdigit(static_cast<unsigned char>(text[i]))) {
                hexadecimal = false;
                break;
            }
        }
        if (hexadecimal) {
            const auto nibble = [](char c) -> int {
                if (c >= '0' && c <= '9') {
                    return c - '0';
                }
                if (c >= 'a' && c <= 'f') {
                    return c - 'a' + 10;
                }
                return c - 'A' + 10;
            };
            std::string key(32, '\0');
            for (std::size_t i = 0; i < 32; ++i) {
                key[i] = static_cast<char>((nibble(text[i * 2]) << 4) | nibble(text[i * 2 + 1]));
            }
            return key;
        }
    }

    // Anything else is key material, not a key: hashing it keeps a passphrase from
    // being stretched into 32 bytes by padding, which would make it far weaker
    // than it looks.
    const Digest digest = sha256("rblx-store-key:v1:" + text);
    return std::string(reinterpret_cast<const char*>(digest.data()), digest.size());
}

}  // namespace web
}  // namespace rblx
