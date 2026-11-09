#pragma once

#include <cstddef>
#include <deque>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "ChatRoom.h"
#include "Moderation.h"
#include "WebHttp.h"
#include "WebJson.h"
#include "WebStore.h"

namespace rblx {
namespace web {

struct Config {
    std::string nodeEnv = "development";
    bool isProduction = false;
    int port = 3000;
    std::string jwtSecret = "local-dev-secret-change-me";
    long long jwtExpiresSeconds = 7 * 24 * 60 * 60;
    int passwordIterations = 120000;
    // What the data file is sealed with, as whoever runs the install wrote it: 64
    // hex characters, or any text that is hashed into a key. Empty means the
    // install has not chosen one and the key is derived from JWT_SECRET instead.
    std::string dataKeyText;
    std::string dataPath;
    std::vector<std::string> corsOrigins;
    long long authRateLimitWindowMs = 15 * 60 * 1000;
    int authRateLimitMax = 20;
    // Every /api/ request counts against this window. The budget is loose on
    // purpose: one page load makes a handful of calls, and the game servers
    // running beside the website use the same endpoints over loopback.
    long long rateLimitWindowMs = 60 * 1000;
    int rateLimitMax = 600;
    // How often one account may post in a game's chat. Chat is the one endpoint a
    // person legitimately sends a burst through, so it gets a budget of its own
    // instead of eating the site-wide one.
    long long chatRateWindowMs = 10 * 1000;
    int chatRateMax = 5;
    // Detections are acted on in waves. An account climbs one step once its
    // oldest held detection is this old, so everyone caught in the same window is
    // answered together and a burst on one account is one step, not a climb.
    long long banWaveWindowMs = 5 * 60 * 1000;
    std::size_t jsonBodyLimitBytes = 2 * 1024 * 1024;
    std::string repoRoot;
    std::string webRoot;
    std::string gameWorldsDir;
    std::string gameServerHost = "127.0.0.1";
    int gameServerBasePort = 7777;
    long long gameInstanceEmptyGraceMs = 30 * 1000;
    // A freshly started server has not had a chance to be joined yet, and a join
    // includes an account check, the avatar fetch and the world download. That
    // window is not the same thing as an empty server, so it gets its own, longer
    // grace before the empty server reaper is allowed to touch it.
    long long gameInstanceLaunchGraceMs = 120 * 1000;
    std::string publicBaseUrl;
    std::string serverExecutablePath;
    std::string clientExecutablePath;
    // Extra usernames that may open the admin dashboard, matched case-sensitively.
    // The account that owns the install is always allowed on top of these, so
    // naming another account here cannot lock the owner out of /admin.
    std::vector<std::string> adminUsernames = {"defaultuser0"};
    // What a confirmed anti-cheat detection costs. The ladder is in Moderation.h;
    // this is the one place an install can tune it.
    rblx::moderation::Policy moderation;
};

struct WorldAnalysis {
    std::string source;
    int partsCount = 0;
    int dynamicPartsCount = 0;
    int spawnCount = 0;
    int cubeCount = 0;
    int sphereCount = 0;
    int wedgeCount = 0;
    int cylinderCount = 0;
};

// Snapshot of one running game server, used by the desktop console window.
struct GameInstanceInfo {
    std::string id;
    std::string title;
    std::string status;
    std::string logPath;
    long long startedAtMs = 0;
    int gameId = 0;
    int port = 0;
    int players = 0;
    long pid = 0;
};

// Validates and normalises a `.world` document exactly like the Node backend.
bool analyzeWorldSource(const std::string& worldText, WorldAnalysis* analysis,
                        std::string* error);

bool loadConfigFromEnvironment(Config* config, std::string* error);

class WebsiteApp {
public:
    explicit WebsiteApp(Config config);
    ~WebsiteApp();

    WebsiteApp(const WebsiteApp&) = delete;
    WebsiteApp& operator=(const WebsiteApp&) = delete;

    bool initialize(std::string* error);
    void installRoutes();

    HttpServer& server() { return server_; }
    const Config& config() const { return config_; }
    Store& store() { return store_; }

    // Read-only view of the running game servers for the console window. The
    // list is sorted by port so the window's order stays stable.
    void collectGameInstances(std::vector<GameInstanceInfo>* out);
    // Stops one game server by instance id; false when it already stopped.
    bool stopGameInstanceById(const std::string& instanceId);

    bool start(std::string* error);
    void run();
    void stop();

private:
    struct GameInstance {
        std::string id;
        int gameId = 0;
        std::string host;
        int port = 0;
        std::string status = "manual";
        bool manual = true;
        std::string startedAt;
        long long startedAtMs = 0;
        int playerCount = 0;
        std::string lastHeartbeatAt;
        std::string emptySince;
        long long emptySinceMs = 0;
        long long emptyShutdownAtMs = 0;
        bool hasShutdownAt = false;
        // Set once the server has reported a player. Until then "no players" only
        // means "nobody has finished joining", so the launch grace applies.
        bool hadPlayer = false;
        long long lastHeartbeatAtMs = 0;
        bool staleHeartbeatWarned = false;
        std::string managerToken;
        long processId = 0;
        std::string command;
        // Game server output is captured here so a startup failure (a busy port,
        // a missing world) can be reported instead of disappearing.
        std::string logPath;
        int exitCode = -1;
        bool exitCodeKnown = false;
        void* processHandle = nullptr;
    };

    struct AuthUser {
        int id = 0;
        std::string username;
    };

    // A captcha that has been handed out but not spent yet. A human challenge
    // carries the digits the image shows; a machine challenge carries the work
    // the client had to do to answer it.
    struct CaptchaChallenge {
        std::string kind;
        std::string answer;
        int difficulty = 0;
        long long expiresAtMs = 0;
    };

    // Why a client stopped working, sent by the client itself so the reason is
    // readable on the website instead of only in its local log files.
    struct ClientReport {
        std::string createdAt;
        std::string stage;
        std::string reason;
        std::string serverAddress;
        int gameId = 0;
        int userId = -1;
        std::string username;
    };

    HttpServer::Handler guard(HttpServer::Handler handler);
    bool isOriginAllowed(const std::string& origin) const;
    bool isRateLimited(const std::string& remoteAddress);
    // Counts one request from `remoteAddress` against the site-wide budget.
    // Returns true when it is spent, handing back how long to wait.
    bool isRequestRateLimited(const std::string& remoteAddress, int* retryAfterSeconds);

    bool parseJsonBody(const Request& request, Json* body, Response& response) const;
    bool requireAuth(const Request& request, Response& response, AuthUser* user);
    bool optionalAuth(const Request& request, AuthUser* user, bool* authenticated);
    bool isAdminUser(const AuthUser& user) const;
    // 403 body for a signed-in account that is not allowed into the dashboard.
    std::string adminDeniedMessage(const AuthUser& user) const;

    Json profileJson(int userId, int viewerId);
    // `viewerId` is what makes "you liked this" possible; a signed-out caller
    // passes zero and only the count is filled in.
    Json gameJson(const GameRecord& game, bool canEdit, int viewerId = 0) const;
    Json instanceJson(const GameInstance& instance) const;
    Json avatarJson(const Avatar& avatar) const;

    // Hands out a single-use challenge: an image for people, a proof-of-work
    // target for scripts.
    bool handleCaptcha(Request& request, Response& response);
    // Spends the captcha named in `body`; false with a reason when it does not
    // check out. The challenge is gone either way.
    bool consumeCaptcha(const Json& body, std::string* error);

    bool handleSignup(Request& request, Response& response);
    bool handleLogin(Request& request, Response& response);
    bool handleVerify(Request& request, Response& response);
    // Whether a machine may join at all, for a guest whose join has no account
    // for /api/verify to refuse.
    bool handleHardwareCheck(Request& request, Response& response);
    bool handleSocial(Request& request, Response& response);
    bool handlePlaytime(Request& request, Response& response);
    bool handleHeartbeat(Request& request, Response& response);
    bool handleUserSearch(Request& request, Response& response);
    bool handleUserProfile(Request& request, Response& response);
    bool handleFriendRequest(Request& request, Response& response);
    bool handleFriendRespond(Request& request, Response& response);
    bool handleFriendRemove(Request& request, Response& response);
    bool handleListGames(Request& request, Response& response);
    bool handleMyGames(Request& request, Response& response);
    bool handleCreateGame(Request& request, Response& response);
    bool handlePublishGame(Request& request, Response& response);
    bool handleGetGame(Request& request, Response& response);
    // Toggles the caller's like on one game; the count comes back with it so the
    // page never has to guess what the row now says.
    bool handleToggleGameLike(Request& request, Response& response);
    // The developer portal: long-lived tokens a program signs in with, and the
    // per-game switch that lets the AI trainer play a map.
    bool handleListDevTokens(Request& request, Response& response);
    bool handleCreateDevToken(Request& request, Response& response);
    bool handleRevokeDevToken(Request& request, Response& response);
    bool handleGameAiSettings(Request& request, Response& response);
    // AiTrainer signs in here with a developer token rather than an account
    // session. The answer names the map to train on, and only for a game its owner
    // has marked AI compatible.
    bool handleAiLogin(Request& request, Response& response);
    Json devTokenJson(const DevTokenRecord& token) const;
    bool handleUpdateGame(Request& request, Response& response);
    // The .rbx script the game server loads beside the world, as the website's
    // code editor reads and saves it.
    bool handleGetGameScript(Request& request, Response& response);
    bool handlePutGameScript(Request& request, Response& response);
    bool handleGameServers(Request& request, Response& response);
    bool handlePlayGame(Request& request, Response& response);
    bool handleClientReport(Request& request, Response& response);
    // The client's own anti-cheat, reporting a detection from inside the process.
    // The report is filed against the signed-in account, which is what makes it
    // attributable: a token-bearing detection climbs the moderation ladder.
    bool handleClientDetection(Request& request, Response& response);
    bool handleAdminOverview(Request& request, Response& response);
    bool handleAdminDeleteGame(Request& request, Response& response);
    // The running game server reports a confirmed detection here. The instance's
    // own launch token proves the report came from us and not from a client
    // claiming another player cheated.
    bool handleInstanceModeration(Request& request, Response& response);
    // Admin override: warn, suspend, ban or pardon an account by hand.
    bool handleAdminModerateUser(Request& request, Response& response);
    // The machines barred from the game, and the switch that lets one back in.
    bool handleAdminHardwareBans(Request& request, Response& response);
    bool handleAdminLiftHardwareBan(Request& request, Response& response);
    // Admin override: hand an account coins, from the dashboard.
    bool handleAdminGrantCoins(Request& request, Response& response);
    // The account's own view of its record: where it stands, every entry that
    // put it there, and the appeals it has filed. Settings shows this.
    bool handleAccountStanding(Request& request, Response& response);
    // Files an appeal against the restriction the account is under.
    bool handleAccountAppeal(Request& request, Response& response);
    // An admin accepts or denies one appeal.
    bool handleAdminAppeals(Request& request, Response& response);
    // The recording behind one action, so an admin judging an appeal can watch
    // what the server saw rather than take the label on trust.
    bool handleAdminInfractionReplay(Request& request, Response& response);
    bool handleGetAvatar(Request& request, Response& response);
    bool handleSetAvatar(Request& request, Response& response);
    bool handleChangePassword(Request& request, Response& response);
    // Issues a fresh recovery code for the signed-in account.
    bool handleRotateRecovery(Request& request, Response& response);
    // Public: resets a forgotten password with a recovery code.
    bool handleRecovery(Request& request, Response& response);
    bool handleChangeUsername(Request& request, Response& response);
    bool handleDeleteAccount(Request& request, Response& response);
    bool handleExportData(Request& request, Response& response);
    bool handleBlockUser(Request& request, Response& response);
    bool handleReport(Request& request, Response& response);
    // Chat: the rooms a signed-out visitor may read, one room's messages and
    // members, and posting into a channel.
    bool handleChatServers(Request& request, Response& response);
    bool handleChatRoom(Request& request, Response& response);
    bool handleChatPost(Request& request, Response& response);
    Json chatChannelJson(const chat::Channel& channel) const;
    Json chatMessageJson(const ChatMessageRecord& message, int viewerId) const;
    // The people in a room: whoever polled it recently plus whoever wrote the
    // page of messages being read, so a quiet room still names its regulars.
    Json chatMembersJson(const GameRecord& game, long long nowMs, int viewerId,
                         const std::vector<const ChatMessageRecord*>& page);
    int chatOnlineCount(int gameId, long long nowMs) const;
    void markChatPresence(int gameId, int userId, long long nowMs);
    bool isChatRateLimited(int userId, int* retryAfterSeconds);
    bool handleBuyItem(Request& request, Response& response);
    bool handleEquipItem(Request& request, Response& response);
    // Catalog plus what this account owns and wears, in one list.
    Json itemsJson(int userId) const;
    bool handleStatic(Request& request, Response& response);

    bool createGame(const AuthUser& user, const Json& body, GameRecord* created,
                    Response& response);
    bool updateGameRecord(const AuthUser& user, int gameId, const Json& body,
                          GameRecord* updated, Response& response);
    std::string saveWorldFile(int gameId, const std::string& contents);

    GameInstance* ensureInstance(const GameRecord& game, bool forceNew, std::string* error);
    void reserveInstanceForLaunch(GameInstance& instance);
    void stopInstance(const std::string& instanceId, const std::string& reason);
    void tickInstances();
    void refreshInstances();
    // One wave: gives every account whose oldest held detection has waited out
    // `banWaveWindowMs` exactly one more step, then clears the evidence it used.
    // The caller holds `mutex_`. Returns how many accounts were acted on.
    int applyBanWaves();
    std::vector<std::string> activeInstanceIds(int gameId);
    struct SpawnedProcess {
        long pid = 0;
        void* handle = nullptr;
    };

    bool spawnProcess(const std::string& executable, const std::vector<std::string>& args,
                      const std::string& workingDirectory, SpawnedProcess* spawned,
                      std::string* error,
                      const std::string& stdoutLogPath = std::string(),
                      bool showConsole = false);
    bool processRunning(GameInstance& instance);
    void releaseProcess(GameInstance& instance);
    static bool isPortAvailable(const std::string& host, int port);
    // Mirrors the bind the game server performs (INADDR_ANY) rather than binding
    // the loopback address, which Windows lets coexist with a wildcard listener.
    static bool isPortFreeForGameServer(int port);
    int findAvailablePort(int startPort);
    bool waitForGameServer(GameInstance& instance, int timeoutMs, std::string* failureDetail);
    std::string describeGameServerLog(const GameInstance& instance) const;

    bool seedStarterGames(std::string* error);

    Config config_;
    Store store_;
    HttpServer server_;
    std::mutex mutex_;
    std::map<std::string, GameInstance> instances_;
    std::deque<ClientReport> clientReports_;
    std::map<std::string, std::vector<long long>> authAttempts_;
    std::map<std::string, std::deque<long long>> requestHits_;
    std::map<std::string, CaptchaChallenge> captchas_;
    // Who is in which room right now, and when each account last posted. Both
    // are presence rather than history: the data file keeps messages, not who
    // happened to be looking at them when the server restarted.
    std::map<int, std::map<int, long long>> chatPresence_;
    std::map<int, std::deque<long long>> chatPostTimes_;
    std::thread ticker_;
    bool tickerRunning_ = false;
    bool initialized_ = false;
};

// Serves the site, showing the desktop console window unless --nogui /
// --headless is passed or RBLX_NO_GUI is set.
int runWebsiteServer(int argc, char** argv);

}  // namespace web
}  // namespace rblx
