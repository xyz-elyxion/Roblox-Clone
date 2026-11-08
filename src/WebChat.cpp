// The chat endpoints: the room list, one room, and posting into a channel.
//
// They live in their own unit so the rules a message is checked against
// (ChatRoom.h) and the HTTP that carries it sit together instead of adding a few
// hundred more lines about one page to WebApp.cpp, which is the whole site.

#include "WebApp.h"

#include "ChatRoom.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <utility>

namespace rblx {
namespace web {
namespace {

// The small helpers WebApp.cpp keeps in its own anonymous namespace cannot be
// shared with another unit, so the few lines this file needs live here rather
// than being published in a header that only these handlers would include.

long long chatMillis() {
    return static_cast<long long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
}

std::string chatIsoNow() {
    const std::time_t now = std::time(nullptr);
    std::tm parts{};
    gmtime_s(&parts, &now);
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02dZ",
                  parts.tm_year + 1900, parts.tm_mon + 1, parts.tm_mday, parts.tm_hour,
                  parts.tm_min, parts.tm_sec);
    return std::string(buffer);
}

void chatError(Response& response, int status, const std::string& message) {
    Json body = Json::makeObject();
    body.set("error", message);
    response.json(status, body.dump());
}

// A positive count out of a query string or a route, with the caller's fallback
// for anything that is not one.
int chatInt(const std::string& text, int fallback) {
    if (text.empty()) {
        return fallback;
    }
    char* end = nullptr;
    const long value = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || value < 0 || value > 100000000) {
        return fallback;
    }
    return static_cast<int>(value);
}

bool chatRestricted(const UserRecord& account, long long nowMs) {
    return account.banned || account.suspendedUntilMs > nowMs;
}

}  // namespace

Json WebsiteApp::chatChannelJson(const chat::Channel& channel) const {
    Json entry = Json::makeObject();
    entry.set("id", channel.id);
    entry.set("title", channel.title);
    entry.set("topic", channel.topic);
    return entry;
}

Json WebsiteApp::chatMessageJson(const ChatMessageRecord& message, int viewerId) const {
    Json entry = Json::makeObject();
    entry.set("id", message.id);
    entry.set("userId", message.userId);
    entry.set("username", message.username);
    entry.set("body", message.body);
    entry.set("at", message.createdAt);
    entry.set("mine", viewerId > 0 && message.userId == viewerId);
    return entry;
}

int WebsiteApp::chatOnlineCount(int gameId, long long nowMs) const {
    const auto room = chatPresence_.find(gameId);
    if (room == chatPresence_.end()) {
        return 0;
    }
    int online = 0;
    for (const auto& entry : room->second) {
        if (nowMs - entry.second <= chat::kOnlineWindowMs) {
            ++online;
        }
    }
    return online;
}

void WebsiteApp::markChatPresence(int gameId, int userId, long long nowMs) {
    std::map<int, long long>& room = chatPresence_[gameId];
    // Presence is a window, not a record: whoever would not be listed any more is
    // dropped here rather than kept for the life of the process.
    for (auto entry = room.begin(); entry != room.end();) {
        if (nowMs - entry->second > chat::kMemberWindowMs) {
            entry = room.erase(entry);
        } else {
            ++entry;
        }
    }
    room[userId] = nowMs;
}

bool WebsiteApp::isChatRateLimited(int userId, int* retryAfterSeconds) {
    const long long now = chatMillis();
    std::deque<long long>& times = chatPostTimes_[userId];
    while (!times.empty() && now - times.front() >= config_.chatRateWindowMs) {
        times.pop_front();
    }
    if (static_cast<int>(times.size()) >= config_.chatRateMax) {
        const long long waitMs = config_.chatRateWindowMs - (now - times.front());
        if (retryAfterSeconds != nullptr) {
            *retryAfterSeconds = static_cast<int>(std::max<long long>(1, (waitMs + 999) / 1000));
        }
        return true;
    }
    times.push_back(now);
    return false;
}

Json WebsiteApp::chatMembersJson(const GameRecord& game, long long nowMs, int viewerId,
                                 const std::vector<const ChatMessageRecord*>& page) {
    std::map<int, long long> seen;
    const auto room = chatPresence_.find(game.id);
    if (room != chatPresence_.end()) {
        for (const auto& entry : room->second) {
            if (nowMs - entry.second <= chat::kMemberWindowMs) {
                seen[entry.first] = entry.second;
            }
        }
    }
    // A reader who just opened a quiet room still sees who wrote what is on the
    // screen, so the people in the page of messages are members too.
    for (const ChatMessageRecord* message : page) {
        const auto found = seen.find(message->userId);
        if (found == seen.end()) {
            seen[message->userId] = message->atMs;
        } else {
            found->second = std::max(found->second, message->atMs);
        }
    }

    struct Member {
        int id = 0;
        std::string name;
        bool online = false;
        bool owner = false;
    };
    std::vector<Member> members;
    for (const auto& entry : seen) {
        const UserRecord* account = store_.findUserById(entry.first);
        if (account == nullptr) {
            continue;
        }
        // A block hides both accounts from each other everywhere on the site, and
        // the member list is part of everywhere.
        if (viewerId > 0 && entry.first != viewerId &&
            store_.blockedEitherWay(viewerId, entry.first)) {
            continue;
        }
        Member member;
        member.id = entry.first;
        member.name = account->username;
        member.online = nowMs - entry.second <= chat::kOnlineWindowMs;
        member.owner = entry.first == game.ownerId;
        members.push_back(member);
    }

    std::sort(members.begin(), members.end(), [](const Member& left, const Member& right) {
        if (left.online != right.online) {
            return left.online;
        }
        if (left.owner != right.owner) {
            return left.owner;
        }
        return left.name < right.name;
    });

    Json list = Json::makeArray();
    for (const Member& member : members) {
        if (list.size() >= chat::kMaxMembers) {
            break;
        }
        Json entry = Json::makeObject();
        entry.set("id", member.id);
        entry.set("username", member.name);
        entry.set("online", member.online);
        entry.set("role", member.owner ? "Owner" : "Player");
        list.push(entry);
    }
    return list;
}

bool WebsiteApp::handleChatServers(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    bool signedIn = false;
    optionalAuth(request, &user, &signedIn);
    const long long now = chatMillis();

    Json servers = Json::makeArray();
    for (const GameRecord* game : store_.publicGames(chat::kMaxServers)) {
        Json entry = Json::makeObject();
        entry.set("id", game->id);
        entry.set("title", game->title);
        entry.set("owner", game->ownerUsername);
        entry.set("iconPath", game->iconPath);
        entry.set("online", chatOnlineCount(game->id, now));
        servers.push(entry);
    }

    Json channels = Json::makeArray();
    for (const chat::Channel& channel : chat::channels()) {
        channels.push(chatChannelJson(channel));
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("servers", servers);
    result.set("channels", channels);
    result.set("maxMessageLength", static_cast<long long>(chat::kMaxMessageLength));
    result.set("signedIn", signedIn);
    if (signedIn) {
        result.set("userId", user.id);
        result.set("username", user.username);
    }
    response.json(200, result.dump());
    return true;
}

bool WebsiteApp::handleChatRoom(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    bool signedIn = false;
    optionalAuth(request, &user, &signedIn);

    const int gameId = chatInt(request.param("id"), 0);
    const GameRecord* game = gameId > 0 ? store_.findGame(gameId) : nullptr;
    // A private game's rooms are its owner's business; everyone else gets the
    // same answer a game that does not exist gets.
    if (game == nullptr || (!game->isPublic && game->ownerId != user.id)) {
        chatError(response, 404, "Game not found.");
        return true;
    }

    std::string channelId = request.queryParam("channel");
    if (channelId.empty()) {
        channelId = chat::channels().front().id;
    }
    const chat::Channel* channel = chat::findChannel(channelId);
    if (channel == nullptr) {
        chatError(response, 400, "Unknown channel.");
        return true;
    }

    const int afterId = chatInt(request.queryParam("after"), 0);
    const long long now = chatMillis();
    if (signedIn) {
        // Reading a room is being in it, so the member list follows the polls
        // instead of asking the page to announce itself separately.
        markChatPresence(game->id, user.id, now);
    }

    bool canPost = false;
    if (signedIn) {
        const UserRecord* account = store_.findUserById(user.id);
        // The site stays readable to a restricted account - it has to, to read
        // its record and appeal - but a restricted account cannot post.
        canPost = account != nullptr && !chatRestricted(*account, now);
    }

    const std::vector<const ChatMessageRecord*> messages =
        store_.chatMessages(game->id, channel->id, afterId, chat::kMessagePageSize);

    Json list = Json::makeArray();
    int hidden = 0;
    for (const ChatMessageRecord* message : messages) {
        if (signedIn && store_.blockedEitherWay(user.id, message->userId)) {
            ++hidden;
            continue;
        }
        list.push(chatMessageJson(*message, user.id));
    }

    Json channels = Json::makeArray();
    for (const chat::Channel& entry : chat::channels()) {
        channels.push(chatChannelJson(entry));
    }

    Json server = Json::makeObject();
    server.set("id", game->id);
    server.set("title", game->title);
    server.set("owner", game->ownerUsername);
    server.set("description", game->description);
    server.set("isOwner", signedIn && game->ownerId == user.id);
    server.set("online", chatOnlineCount(game->id, now));

    Json you = Json::makeObject();
    you.set("signedIn", signedIn);
    you.set("canPost", canPost);
    if (signedIn) {
        you.set("id", user.id);
        you.set("username", user.username);
    }

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("game", server);
    result.set("channel", channel->id);
    result.set("channels", channels);
    result.set("messages", list);
    result.set("hidden", hidden);
    result.set("members", chatMembersJson(*game, now, user.id, messages));
    result.set("you", you);
    result.set("maxMessageLength", static_cast<long long>(chat::kMaxMessageLength));
    response.json(200, result.dump());
    return true;
}

bool WebsiteApp::handleChatPost(Request& request, Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    AuthUser user;
    if (!requireAuth(request, response, &user)) {
        return true;
    }

    const long long now = chatMillis();
    const UserRecord* account = store_.findUserById(user.id);
    if (account != nullptr && chatRestricted(*account, now)) {
        chatError(response, 403,
                  "This account is restricted and cannot post. Settings has the record and the "
                  "appeal.");
        return true;
    }

    const int gameId = chatInt(request.param("id"), 0);
    const GameRecord* game = gameId > 0 ? store_.findGame(gameId) : nullptr;
    if (game == nullptr || (!game->isPublic && game->ownerId != user.id)) {
        chatError(response, 404, "Game not found.");
        return true;
    }

    Json body;
    if (!parseJsonBody(request, &body, response)) {
        return true;
    }

    std::string channelId = body.member("channel").asString();
    if (channelId.empty()) {
        channelId = chat::channels().front().id;
    }
    const chat::Channel* channel = chat::findChannel(channelId);
    if (channel == nullptr) {
        chatError(response, 400, "Unknown channel.");
        return true;
    }

    std::string text;
    std::string textError;
    if (!chat::normalizeBody(body.member("body").asString(), &text, &textError)) {
        chatError(response, 400, textError);
        return true;
    }

    int retryAfterSeconds = 0;
    if (isChatRateLimited(user.id, &retryAfterSeconds)) {
        response.headers["Retry-After"] = std::to_string(retryAfterSeconds);
        chatError(response, 429, "You are sending messages too quickly. Try again in " +
                                     std::to_string(retryAfterSeconds) + " seconds.");
        return true;
    }

    ChatMessageRecord record;
    record.gameId = game->id;
    record.channel = channel->id;
    record.userId = user.id;
    record.username = user.username;
    record.body = text;
    record.createdAt = chatIsoNow();
    record.atMs = now;
    record.id = store_.addChatMessage(record);
    if (record.id <= 0) {
        chatError(response, 400, "That message could not be stored.");
        return true;
    }

    std::string saveError;
    if (!store_.save(&saveError)) {
        chatError(response, 500, "Could not update the data file.");
        return true;
    }

    markChatPresence(game->id, user.id, now);

    Json result = Json::makeObject();
    result.set("success", true);
    result.set("message", chatMessageJson(record, user.id));
    response.json(201, result.dump());
    return true;
}

}  // namespace web
}  // namespace rblx
