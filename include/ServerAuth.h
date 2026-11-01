#pragma once

#include <string>

struct VerifiedUser {
    bool success = false;
    int userId = -1;
    std::string username;
    // Set when the website refused the token for a reason the player should read,
    // such as a suspension or a ban. Empty for an ordinary bad token.
    std::string reason;
};

// What the website did with one reported detection. The game server shows the
// message to the player and drops them when the account is restricted.
struct CheatReportResult {
    bool reported = false;
    std::string action; // "warned", "suspended", "banned" or "none"
    std::string message;
    bool restricted = false;
};

class ServerAuth {
public:
    static VerifiedUser verifyToken(const std::string& token, const std::string& serverUrl = "http://localhost:3000");
    static std::string getAvatar(const std::string& token, const std::string& serverUrl = "http://localhost:3000");
    static bool recordPlaytime(const std::string& token, int seconds, const std::string& serverUrl = "http://localhost:3000");
    static bool reportInstanceHeartbeat(const std::string& instanceId, const std::string& managerToken, int playerCount, const std::string& serverUrl = "http://localhost:3000");
    // Reports a confirmed movement detection for `username`. The instance's own
    // launch token signs it, because the player cannot be trusted to report
    // themselves and another client cannot be trusted to report them either.
    static CheatReportResult reportCheating(const std::string& instanceId,
                                            const std::string& managerToken,
                                            const std::string& username,
                                            const std::string& reason,
                                            const std::string& serverUrl = "http://localhost:3000");
};
