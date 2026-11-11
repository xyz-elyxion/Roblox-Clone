#pragma once

#include <cctype>
#include <string>

#include "WebJson.h"

// The small, pure rules the client's escape menu needs: which URL a row opens,
// how a username is shown under a display name, and which account id a search
// answer names. They live in a header rather than in ClientUi.cpp so the tests
// can reach them without a window, a socket or a signed-in session.
namespace rblx {
namespace client_menu {

// The site names a profile by account id, so the id has to come out of the
// search answer before a row's button can open the right page.
inline std::string profileUrl(const std::string& siteUrl, int userId) {
    if (siteUrl.empty() || userId <= 0) return std::string();
    return siteUrl + "/profile/" + std::to_string(userId);
}

// The game a session runs has a page on the site, and that page is what an
// invite needs: opening it starts a server and hands the friend a client.
inline std::string inviteUrl(const std::string& siteUrl, int gameId) {
    if (siteUrl.empty() || gameId <= 0) return std::string();
    return siteUrl + "/games/" + std::to_string(gameId);
}

// A row shows the display name and, under it, the handle that name is found by.
// The handle keeps only what an account name may hold, folded to lower case, so
// a name with a space, a slash or a control byte cannot become a second line or
// a path in a URL.
inline std::string handle(const std::string& username) {
    std::string out;
    for (const char rawChar : username) {
        const unsigned char c = static_cast<unsigned char>(rawChar);
        if (std::isalnum(c) != 0 || c == '_' || c == '.' || c == '-') {
            out.push_back(static_cast<char>(std::tolower(c)));
        }
        if (out.size() >= 20) break;
    }
    if (out.empty()) return std::string();
    return "@" + out;
}

// The site's search answers every account whose name starts with the query, and
// a friend request or a report needs the one whose name is exactly this. Zero
// means the account is not there, which is a normal answer for a player who has
// since deleted it.
inline int findUserId(const std::string& usersJson, const std::string& username) {
    if (usersJson.empty() || username.empty()) return 0;
    std::string error;
    const rblx::web::Json root = rblx::web::Json::parse(usersJson, &error);
    if (!root.isObject()) return 0;
    const rblx::web::Json& users = root.member("users");
    if (!users.isArray()) return 0;
    for (const rblx::web::Json& user : users.elements()) {
        if (user.member("username").asString() != username) continue;
        const int id = static_cast<int>(user.member("id").asInteger(0));
        if (id > 0) return id;
    }
    return 0;
}

}  // namespace client_menu
}  // namespace rblx
