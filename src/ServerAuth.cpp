#include "ServerAuth.h"

#include <cstdlib>
#include <sstream>
#include <string>

#include "HardwareId.h"
#include "HttpClient.h"

// The game server reports to the website: it verifies player tokens, fetches
// avatars, records playtime, posts instance heartbeats and reports confirmed
// cheating. All of it goes through HttpClient so the client and the server share
// one transport; there is no WinHTTP dependency left in the server.
namespace {
    bool jsonBoolTrue(const std::string& json, const std::string& field) {
        const std::string needle = "\"" + field + "\":true";
        return json.find(needle) != std::string::npos;
    }

    int jsonInt(const std::string& json, const std::string& field) {
        const std::string needle = "\"" + field + "\":";
        size_t pos = json.find(needle);
        if (pos == std::string::npos) return -1;
        pos += needle.size();
        return std::atoi(json.c_str() + pos);
    }

    std::string jsonString(const std::string& json, const std::string& field) {
        const std::string needle = "\"" + field + "\":\"";
        size_t pos = json.find(needle);
        if (pos == std::string::npos) return "";
        pos += needle.size();
        size_t end = json.find('"', pos);
        if (end == std::string::npos) return "";
        return json.substr(pos, end - pos);
    }

    // A JSON string field built by hand, so a quote or a newline in a reason
    // cannot break the request. The server only ever sends its own literals and
    // account names, but the escaping is here rather than at every call site.
    std::string jsonSafe(const std::string& value, size_t limit) {
        std::string out;
        for (char c : value) {
            if (out.size() >= limit) break;
            if (c == '"' || c == '\\' || c == '\n' || c == '\r' || c == '\t') continue;
            out.push_back(c);
        }
        return out;
    }

    std::string postJson(const std::string& url, const std::string& body, const std::string& bearerToken) {
        const rblx::http::Response response = rblx::http::request("POST", url, body, bearerToken);
        if (!response.ok()) {
            return "";
        }
        return response.body;
    }
}

VerifiedUser ServerAuth::verifyToken(const std::string& token, const std::string& serverUrl,
                                     const std::string& hardwareId) {
    VerifiedUser user;
    if (token.empty()) return user;

    // The body is read whether the answer was 200 or not: a suspended or banned
    // account is refused with a 403 and the reason the player should see, which
    // the game server passes on instead of a generic "login refused".
    // The machine goes with the login: the website records it on the account and
    // refuses the join when the machine itself is banned.
    std::string body = "{}";
    const std::string machine = rblx::hwid::normalize(hardwareId);
    if (rblx::hwid::valid(machine)) {
        body = "{\"hardwareId\":\"" + machine + "\"}";
    }
    const rblx::http::Response reply =
        rblx::http::request("POST", serverUrl + "/api/verify", body, token);
    if (reply.body.empty() || !jsonBoolTrue(reply.body, "success")) {
        user.reason = jsonString(reply.body, "error");
        return user;
    }

    // A suspension or a ban no longer refuses the website login: the site keeps
    // the player signed in so they can read their record and appeal it. Joining a
    // game is a different door, and it is still shut - with the reason to read.
    if (jsonBoolTrue(reply.body, "restricted")) {
        user.reason = jsonString(reply.body, "message");
        if (user.reason.empty()) {
            user.reason = "This account is suspended or banned, so it cannot join a game. "
                          "Its standing is on the website under Settings.";
        }
        return user;
    }

    user.userId = jsonInt(reply.body, "userId");
    user.username = jsonString(reply.body, "username");
    user.success = user.userId > 0 && !user.username.empty() && user.username.size() < 32;
    return user;
}

std::string ServerAuth::getAvatar(const std::string& token, const std::string& serverUrl) {
    if (token.empty()) return "";

    const rblx::http::Response response = rblx::http::request("GET", serverUrl + "/api/avatar", "", token);
    if (!response.ok()) {
        return "";
    }
    return response.body;
}

bool ServerAuth::recordPlaytime(const std::string& token, int seconds, const std::string& serverUrl) {
    if (token.empty() || seconds <= 0) return false;

    std::ostringstream body;
    body << "{\"seconds\":" << seconds << "}";

    const std::string response = postJson(serverUrl + "/api/me/playtime", body.str(), token);
    return !response.empty() && jsonBoolTrue(response, "success");
}

bool ServerAuth::reportInstanceHeartbeat(const std::string& instanceId, const std::string& managerToken, int playerCount, const std::string& serverUrl) {
    if (instanceId.empty() || managerToken.empty()) return false;

    std::ostringstream body;
    body << "{\"playerCount\":" << (playerCount < 0 ? 0 : playerCount) << "}";

    const std::string response = postJson(
        serverUrl + "/api/game-instances/" + instanceId + "/heartbeat",
        body.str(),
        managerToken);
    return !response.empty() && jsonBoolTrue(response, "success");
}

CheatReportResult ServerAuth::reportCheating(const std::string& instanceId,
                                            const std::string& managerToken,
                                            const std::string& username,
                                            const std::string& reason,
                                            const std::string& proof,
                                            const std::string& expected,
                                            const std::string& replay,
                                            const std::string& serverUrl) {
    CheatReportResult result;
    if (instanceId.empty() || managerToken.empty() || username.empty()) {
        return result;
    }

    std::ostringstream body;
    body << "{\"username\":\"" << jsonSafe(username, 32) << "\",\"reason\":\""
         << jsonSafe(reason, 80) << "\"";
    // The evidence and the rule it broke go with the report, so they are on record
    // where an admin reads them rather than only in the server's console.
    if (!proof.empty()) {
        body << ",\"proof\":\"" << jsonSafe(proof, 400) << "\"";
    }
    if (!expected.empty()) {
        body << ",\"expected\":\"" << jsonSafe(expected, 400) << "\"";
    }
    // A full clip is about ninety frames, so it gets the room the numbers do not
    // need; the engine's own record is bounded well below this.
    if (!replay.empty()) {
        body << ",\"replay\":\"" << jsonSafe(replay, 3600) << "\"";
    }
    body << "}";

    const std::string response = postJson(
        serverUrl + "/api/game-instances/" + instanceId + "/moderation",
        body.str(),
        managerToken);
    if (response.empty() || !jsonBoolTrue(response, "success")) {
        return result;
    }

    result.reported = true;
    result.action = jsonString(response, "action");
    result.message = jsonString(response, "message");
    result.restricted = jsonBoolTrue(response, "restricted");
    return result;
}

bool ServerAuth::isHardwareBanned(const std::string& hardwareId, std::string* reason,
                                  const std::string& serverUrl) {
    if (reason != nullptr) {
        reason->clear();
    }
    const std::string machine = rblx::hwid::normalize(hardwareId);
    if (!rblx::hwid::valid(machine)) {
        // Nothing to ask about, and refusing every join whose client could not
        // name its machine would lock out the players this is meant to protect.
        return false;
    }

    std::ostringstream body;
    body << "{\"hardwareId\":\"" << machine << "\"}";
    // Unsigned: the answer says only whether this machine may play, which the
    // caller already knows by holding its hash.
    const std::string response =
        postJson(serverUrl + "/api/hardware-check", body.str(), std::string());
    if (response.empty() || !jsonBoolTrue(response, "banned")) {
        return false;
    }
    if (reason != nullptr) {
        *reason = jsonString(response, "message");
        if (reason->empty()) {
            *reason = "This computer is banned from joining games.";
        }
    }
    return true;
}
