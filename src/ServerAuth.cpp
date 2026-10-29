#include "ServerAuth.h"

#include <cstdlib>
#include <sstream>
#include <string>

#include "HttpClient.h"

// The game server reports to the website: it verifies player tokens, fetches
// avatars, records playtime and posts instance heartbeats. All of it goes
// through HttpClient so the same code runs on Windows and POSIX; there is no
// WinHTTP dependency left in the server.
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

    std::string postJson(const std::string& url, const std::string& body, const std::string& bearerToken) {
        const rblx::http::Response response = rblx::http::request("POST", url, body, bearerToken);
        if (!response.ok()) {
            return "";
        }
        return response.body;
    }
}

VerifiedUser ServerAuth::verifyToken(const std::string& token, const std::string& serverUrl) {
    VerifiedUser user;
    if (token.empty()) return user;

    const std::string response = postJson(serverUrl + "/api/verify", "{}", token);
    if (response.empty() || !jsonBoolTrue(response, "success")) {
        return user;
    }

    user.userId = jsonInt(response, "userId");
    user.username = jsonString(response, "username");
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
