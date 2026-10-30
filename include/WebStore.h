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
};

struct UserRecord {
    int id = 0;
    std::string username;
    std::string passwordHash;
    std::string createdAt;
};

struct AvatarRecord {
    int userId = 0;
    Avatar avatar;
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
    const UserRecord* firstUser() const;
    const std::vector<UserRecord>& users() const { return users_; }
    std::vector<const UserRecord*> searchUsers(const std::string& query, std::size_t limit) const;

    // avatars
    Avatar avatarFor(int userId) const;
    void setAvatar(int userId, const Avatar& avatar);

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

    std::string path_;
    std::vector<UserRecord> users_;
    std::vector<AvatarRecord> avatars_;
    std::vector<FriendshipRecord> friendships_;
    std::vector<StatsRecord> stats_;
    std::vector<GameRecord> games_;
    int nextUserId_ = 1;
    int nextFriendshipId_ = 1;
    int nextGameId_ = 1;
};

}  // namespace web
}  // namespace rblx
