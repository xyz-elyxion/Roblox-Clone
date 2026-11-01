#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "WebApp.h"
#include "WebCaptcha.h"
#include "WebCrypto.h"
#include "WebJson.h"
#include "WebStore.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace {

int failures = 0;

void check(bool condition, const std::string& name) {
    if (condition) {
        std::printf("ok   %s\n", name.c_str());
        return;
    }
    ++failures;
    std::printf("FAIL %s\n", name.c_str());
}

void checkEqual(const std::string& actual, const std::string& expected,
                const std::string& name) {
    if (actual == expected) {
        std::printf("ok   %s\n", name.c_str());
        return;
    }
    ++failures;
    std::printf("FAIL %s\n     expected: %s\n     actual:   %s\n", name.c_str(),
                expected.c_str(), actual.c_str());
}

void checkContains(const std::string& haystack, const std::string& needle,
                   const std::string& name) {
    if (haystack.find(needle) != std::string::npos) {
        std::printf("ok   %s\n", name.c_str());
        return;
    }
    ++failures;
    std::printf("FAIL %s\n     missing: %s\n     in:      %s\n", name.c_str(), needle.c_str(),
                haystack.c_str());
}

std::string testWebRoot() {
    const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const auto directory =
        std::filesystem::temp_directory_path() / ("rblx-native-web-" + std::to_string(stamp));
    std::filesystem::create_directories(directory / "public");
    std::ofstream index((directory / "public" / "index.html").string().c_str(),
                        std::ios::binary | std::ios::trunc);
    index << "<!doctype html><title>native</title><h1>native web app test page</h1>";
    index.close();
    std::ofstream admin((directory / "public" / "admin.html").string().c_str(),
                        std::ios::binary | std::ios::trunc);
    admin << "<!doctype html><title>admin</title><div data-admin-page></div>";
    admin.close();
    return directory.string();
}

std::string validWorldLine() {
    return "0 Part 1 0 0 1 1 0 0 0 0 0 0 0 1 1 1 1 0 0 0 0 0";
}

// ---------------------------------------------------------------------------
// crypto
// ---------------------------------------------------------------------------

void testCrypto() {
    checkEqual(rblx::web::hexEncode(rblx::web::sha256("abc")),
               "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
               "sha256(\"abc\") matches the FIPS vector");
    checkEqual(rblx::web::hexEncode(rblx::web::sha256("")),
               "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
               "sha256(\"\") matches the FIPS vector");
    checkEqual(rblx::web::hexEncode(rblx::web::hmacSha256("Jefe", "what do ya want for nothing?")),
               "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843",
               "hmac-sha256 matches RFC 4231 case 2");

    const std::vector<std::uint8_t> oneRound =
        rblx::web::pbkdf2Sha256("password", "salt", 1, 32);
    checkEqual(rblx::web::hexEncode(oneRound.data(), oneRound.size()),
               "120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b",
               "pbkdf2-sha256 matches the reference vector (1 round)");
    const std::vector<std::uint8_t> twoRounds =
        rblx::web::pbkdf2Sha256("password", "salt", 2, 32);
    checkEqual(rblx::web::hexEncode(twoRounds.data(), twoRounds.size()),
               "ae4d0c95af6b46d32d0adff928f06dd02a303f8ef3c251dfd6e2d85a95474c43",
               "pbkdf2-sha256 matches the reference vector (2 rounds)");

    const std::vector<std::uint8_t> bytes = {0xFB, 0xFF};
    checkEqual(rblx::web::base64UrlEncode(bytes), "-_8", "base64url encodes without padding");
    std::vector<std::uint8_t> decoded;
    check(rblx::web::base64UrlDecode("-_8", &decoded) && decoded == bytes,
          "base64url decode round-trips");

    const std::string hash = rblx::web::hashPassword("correct horse battery", 800);
    check(rblx::web::verifyPassword("correct horse battery", hash),
          "password hash verifies the original password");
    check(!rblx::web::verifyPassword("wrong password", hash),
          "password hash rejects a wrong password");
    checkContains(hash, "pbkdf2-sha256$800$", "password hash records its iteration count");
}

void testJwt() {
    rblx::web::Json payload = rblx::web::Json::makeObject();
    const long long now = static_cast<long long>(std::time(nullptr));
    payload.set("userId", 7);
    payload.set("username", "Pilot");
    payload.set("sub", std::string("7"));
    payload.set("iat", now);
    payload.set("exp", now + 60);

    const std::string token = rblx::web::signJwt(payload, "secret");
    rblx::web::Json claims;
    check(rblx::web::verifyJwt(token, "secret", &claims), "JWT verifies with the right secret");
    check(claims.member("userId").asInteger(0) == 7, "JWT keeps the userId claim");
    check(!rblx::web::verifyJwt(token, "other-secret", &claims),
          "JWT rejects a different secret");

    std::string tampered = token;
    tampered[tampered.size() - 2] = tampered[tampered.size() - 2] == 'A' ? 'B' : 'A';
    check(!rblx::web::verifyJwt(tampered, "secret", &claims), "JWT rejects a tampered signature");

    payload.set("exp", now - 60);
    const std::string expired = rblx::web::signJwt(payload, "secret");
    check(!rblx::web::verifyJwt(expired, "secret", &claims), "JWT rejects an expired token");
}

// ---------------------------------------------------------------------------
// json
// ---------------------------------------------------------------------------

void testJson() {
    std::string error;
    const rblx::web::Json parsed = rblx::web::Json::parse(
        "{\"name\":\"Pilot\",\"tags\":[1,2.5,\"x\"],\"flags\":{\"on\":true},\"none\":null}",
        &error);
    check(parsed.isObject() && error.empty(), "json parses an object");
    checkEqual(parsed.member("name").asString(), "Pilot", "json keeps string values");
    check(parsed.member("tags").isArray() && parsed.member("tags").size() == 3,
          "json keeps array values");
    check(parsed.member("flags").member("on").asBool(), "json keeps nested booleans");
    check(parsed.member("none").isNull(), "json keeps nulls");

    const std::string serialized = parsed.dump();
    const rblx::web::Json roundTripped = rblx::web::Json::parse(serialized, &error);
    check(roundTripped.isObject(), "json output parses again");
    checkEqual(roundTripped.member("tags").elements()[1].dump(), "2.5",
               "json numbers round-trip");

    const rblx::web::Json unicode = rblx::web::Json::parse("\"caf\\u00e9\"", &error);
    checkEqual(unicode.asString(), "caf\xC3\xA9", "json decodes \\u escapes to UTF-8");

    const rblx::web::Json escaped = rblx::web::Json("line\n\"quoted\"");
    checkEqual(escaped.dump(), "\"line\\n\\\"quoted\\\"\"", "json escapes control characters");

    error.clear();
    rblx::web::Json::parse("{\"broken\":", &error);
    check(!error.empty(), "json reports malformed input");

    checkEqual(rblx::web::formatJsonNumber(0.2), "0.2", "json prints doubles without noise");
}

// ---------------------------------------------------------------------------
// world validation
// ---------------------------------------------------------------------------

void testWorld() {
    rblx::web::WorldAnalysis analysis;
    std::string error;
    const std::string world = "2\n" + validWorldLine() + "\n" + validWorldLine() + "\n";
    check(rblx::web::analyzeWorldSource(world, &analysis, &error),
          "world analysis accepts a valid document");
    check(analysis.partsCount == 2, "world analysis counts parts");
    check(analysis.spawnCount == 2, "world analysis counts spawn markers");
    check(analysis.dynamicPartsCount == 2, "world analysis counts dynamic parts");
    check(analysis.cubeCount == 2, "world analysis counts shapes");
    check(!analysis.source.empty() && analysis.source.back() == '\n',
          "world analysis normalises the trailing newline");

    // Parts may carry two optional trailing fields, a material id and a quoted
    // texture path. Publishing has to keep accepting worlds that use them.
    const std::string materialWorld = "1\n" + validWorldLine() + " 3 \"brick.png\"\n";
    error.clear();
    check(rblx::web::analyzeWorldSource(materialWorld, &analysis, &error),
          "world analysis accepts material and texture fields");
    check(analysis.partsCount == 1, "a material world still counts its parts");

    error.clear();
    check(!rblx::web::analyzeWorldSource("", &analysis, &error), "world analysis rejects empty data");
    checkEqual(error, "World data is required.", "empty world reports the expected message");

    error.clear();
    check(!rblx::web::analyzeWorldSource("abc\n" + validWorldLine(), &analysis, &error),
          "world analysis rejects a bad header");
    checkEqual(error, "World file header must contain a valid part count.",
               "bad header reports the expected message");

    error.clear();
    check(!rblx::web::analyzeWorldSource("3\n" + validWorldLine(), &analysis, &error),
          "world analysis rejects a truncated document");
    checkEqual(error, "World file ended before all parts were present.",
               "truncated world reports the expected message");

    error.clear();
    check(!rblx::web::analyzeWorldSource("1\n0 1 2\n", &analysis, &error),
          "world analysis rejects short lines");
    checkEqual(error, "World part 1 is missing fields.", "short line reports the expected message");

    error.clear();
    std::string badShape = validWorldLine();
    badShape[0] = '9';
    check(!rblx::web::analyzeWorldSource("1\n" + badShape + "\n", &analysis, &error),
          "world analysis rejects unknown shapes");
    checkEqual(error, "World part 1 has an unsupported shape.",
               "unknown shape reports the expected message");

    error.clear();
    check(!rblx::web::analyzeWorldSource("1\n" + validWorldLine() + "\r\n", &analysis, &error) ==
              false,
          "world analysis accepts CRLF input");

    error.clear();
    const std::string huge = "1\n" + validWorldLine() + std::string(3 * 1024 * 1024, ' ');
    check(!rblx::web::analyzeWorldSource(huge, &analysis, &error),
          "world analysis rejects oversized data");
    checkContains(error, "too large", "oversized world reports the expected message");
}

// ---------------------------------------------------------------------------
// store
// ---------------------------------------------------------------------------

void testStore() {
    const std::string directory = testWebRoot();
    const std::string path = directory + "/data.json";

    rblx::web::Store store(path);
    std::string error;
    check(store.load(&error), "store loads a missing data file as empty");
    const int first = store.createUser("Pilot", "hash-one", "2026-01-01T00:00:00Z");
    const int second = store.createUser("Builder", "hash-two", "2026-01-01T00:00:00Z");
    check(first > 0 && second > first, "store assigns increasing user ids");
    check(store.createUser("Pilot", "hash-three", "2026-01-01T00:00:00Z") < 0,
          "store rejects duplicate usernames");

    rblx::web::Avatar avatar;
    avatar.colors[0][0] = 0.25;
    avatar.faceId = "wink";
    store.setAvatar(first, avatar);
    checkEqual(store.avatarFor(first).faceId, "wink", "store saves the avatar face");
    check(store.avatarFor(first).colors[0][0] == 0.25, "store saves avatar colours");

    store.addPlaytime(first, 120, "2026-01-02T00:00:00Z");
    store.addPlaytime(first, 30, "2026-01-02T00:00:00Z");
    check(store.playtimeFor(first) == 150, "store accumulates playtime");

    const int friendship = store.addFriendship(first, second, "2026-01-03T00:00:00Z");
    check(friendship > 0 && store.countAcceptedFriends(first) == 0,
          "new friend requests stay pending");
    store.setFriendshipStatus(friendship, "accepted", "2026-01-03T00:01:00Z");
    check(store.countAcceptedFriends(first) == 1, "accepting a request counts as a friend");
    const auto friends = store.acceptedFriendships(first, 10);
    check(friends.size() == 1 && friends[0]->status == "accepted",
          "accepted friendships are listed");

    rblx::web::GameRecord game;
    game.ownerId = first;
    game.title = "Test World";
    game.slug = "test-world";
    game.worldPath = directory + "/game.world";
    game.createdAt = "2026-01-04T00:00:00Z";
    game.updatedAt = game.createdAt;
    game.publishedAt = game.createdAt;
    const int gameId = store.addGame(game);
    check(gameId > 0 && store.findGame(gameId) != nullptr, "store inserts a game");
    checkEqual(store.findGame(gameId)->ownerUsername, "Pilot", "store joins the owner name");
    store.incrementLaunchCount(gameId);
    check(store.findGame(gameId)->launchCount == 1, "store increments launch counts");

    check(store.save(&error), "store writes the data file");
    rblx::web::Store reloaded(path);
    check(reloaded.load(&error), "store reloads the data file");
    check(reloaded.findUserByName("Pilot") != nullptr, "reloaded store keeps users");
    check(reloaded.playtimeFor(first) == 150, "reloaded store keeps playtime");
    check(reloaded.findGame(gameId) != nullptr, "reloaded store keeps games");
    check(reloaded.avatarFor(first).faceId == "wink", "reloaded store keeps avatars");
    check(reloaded.countAcceptedFriends(first) == 1, "reloaded store keeps friendships");

    const auto search = reloaded.searchUsers("uild", 5);
    check(search.size() == 1 && search[0]->username == "Builder",
          "store searches usernames case-insensitively");
}

// ---------------------------------------------------------------------------
// http end to end
// ---------------------------------------------------------------------------

#ifdef _WIN32
using TestSocket = SOCKET;
const TestSocket kInvalidTestSocket = INVALID_SOCKET;
void closeTestSocket(TestSocket handle) { closesocket(handle); }
#else
using TestSocket = int;
const TestSocket kInvalidTestSocket = -1;
void closeTestSocket(TestSocket handle) { ::close(handle); }
#endif

std::string sendRaw(int port, const std::string& raw) {
    const TestSocket handle = ::socket(AF_INET, SOCK_STREAM, 0);
    if (handle == kInvalidTestSocket) {
        return std::string();
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<unsigned short>(port));
    if (::inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) != 1) {
        closeTestSocket(handle);
        return std::string();
    }
    if (::connect(handle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        closeTestSocket(handle);
        return std::string();
    }

    std::size_t sent = 0;
    while (sent < raw.size()) {
#ifdef _WIN32
        const int written = ::send(handle, raw.data() + sent,
                                   static_cast<int>(raw.size() - sent), 0);
#else
        const ssize_t written = ::send(handle, raw.data() + sent, raw.size() - sent, 0);
#endif
        if (written <= 0) {
            closeTestSocket(handle);
            return std::string();
        }
        sent += static_cast<std::size_t>(written);
    }

    std::string response;
    char buffer[4096];
    while (true) {
#ifdef _WIN32
        const int received = ::recv(handle, buffer, static_cast<int>(sizeof(buffer)), 0);
#else
        const ssize_t received = ::recv(handle, buffer, sizeof(buffer), 0);
#endif
        if (received <= 0) {
            break;
        }
        response.append(buffer, static_cast<std::size_t>(received));
    }
    closeTestSocket(handle);
    return response;
}

int statusOf(const std::string& response) {
    if (response.compare(0, 9, "HTTP/1.1 ") != 0) {
        return 0;
    }
    return std::atoi(response.c_str() + 9);
}

std::string bodyOf(const std::string& response) {
    const std::size_t separator = response.find("\r\n\r\n");
    return separator == std::string::npos ? std::string() : response.substr(separator + 4);
}

std::string buildRequest(const std::string& method, const std::string& path,
                         const std::string& body, const std::string& token,
                         const std::string& contentType) {
    std::string raw = method + " " + path + " HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n";
    if (!contentType.empty()) {
        raw += "Content-Type: " + contentType + "\r\n";
    }
    if (!token.empty()) {
        raw += "Authorization: Bearer " + token + "\r\n";
    }
    if (!body.empty() || method == "POST" || method == "PUT") {
        raw += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    }
    raw += "\r\n";
    raw += body;
    return raw;
}

// The account endpoints only answer a solved captcha. The tests take the machine
// route, which is the same path a script or the desktop client uses.
std::string machineCaptcha(rblx::web::WebsiteApp& app) {
    std::thread serverThread([&app]() { app.server().serveOneConnection(); });
    const std::string response = sendRaw(
        app.server().port(),
        buildRequest("GET", "/api/captcha?kind=machine", std::string(), std::string(),
                     "application/json"));
    serverThread.join();

    std::string error;
    const rblx::web::Json challenge = rblx::web::Json::parse(bodyOf(response), &error);
    const std::string id = challenge.member("id").asString();
    const int difficulty = static_cast<int>(challenge.member("difficulty").asInteger(0));
    std::string nonce;
    if (id.empty() || !rblx::web::solveCaptchaPow(id, difficulty, &nonce)) {
        ++failures;
        std::printf("FAIL could not solve a machine captcha: %s\n", bodyOf(response).c_str());
        return "\"captchaId\":\"\",\"captchaNonce\":\"\"";
    }
    return "\"captchaId\":\"" + id + "\",\"captchaNonce\":\"" + nonce + "\"";
}

void checkStatus(rblx::web::WebsiteApp& app, const std::string& method,
                 const std::string& path, const std::string& body, const std::string& token,
                 int expected, const std::string& name, std::string* bodyOut = nullptr,
                 const std::string& contentType = "application/json") {
    std::thread serverThread([&app]() { app.server().serveOneConnection(); });
    const std::string response =
        sendRaw(app.server().port(), buildRequest(method, path, body, token, contentType));
    serverThread.join();

    const int status = statusOf(response);
    if (status == expected) {
        std::printf("ok   %s\n", name.c_str());
    } else {
        ++failures;
        std::printf("FAIL %s\n     expected HTTP %d, got %d\n     body: %s\n", name.c_str(),
                    expected, status, bodyOf(response).c_str());
    }
    if (bodyOut != nullptr) {
        *bodyOut = bodyOf(response);
    }
}

void testHttp() {
    const std::string webRoot = testWebRoot();

    rblx::web::Config config;
    config.nodeEnv = "test";
    config.isProduction = false;
    config.jwtSecret = "test-secret-that-is-long-enough-for-hs256";
    config.passwordIterations = 800;
    config.authRateLimitMax = 1000;
    config.dataPath = webRoot + "/data.json";
    config.webRoot = webRoot;
    config.repoRoot = webRoot;
    config.gameWorldsDir = webRoot + "/game-worlds";
    config.port = 0;
    config.gameInstanceEmptyGraceMs = 0;
    config.publicBaseUrl = "http://127.0.0.1:3000";
    config.adminUsernames.push_back("TestPilot");

    rblx::web::WebsiteApp app(config);
    std::string error;
    check(app.initialize(&error), "app initialises: " + error);
    app.installRoutes();
    check(app.start(&error), "app listens: " + error);
    check(app.server().port() > 0, "app reports its listening port");

    std::string body;
    checkStatus(app, "GET", "/", std::string(), std::string(), 200, "GET / serves the index page",
                &body);
    checkContains(body, "native web app test page", "the index page body is served");

    checkStatus(app, "GET", "/api/games", std::string(), std::string(), 200,
                "GET /api/games is public", &body);
    checkContains(body, "\"games\":[]", "a fresh install has no games");

    checkStatus(app, "POST", "/api/games", "{\"title\":\"No Token\"}", std::string(), 401,
                "POST /api/games requires a token");

    checkStatus(app, "POST", "/api/games", "{\"title\":\"No Token\"}", std::string(), 415,
                "POST /api/games requires a JSON content type", nullptr, "text/plain");

    checkStatus(app, "POST", "/api/signup", "{\"username\":\"ab\",\"password\":\"short\"}",
                std::string(), 400, "signup validates the credentials");

    checkStatus(app, "POST", "/api/signup",
                "{\"username\":\"NoCaptcha\",\"password\":\"long-enough-password\"}",
                std::string(), 400, "signup refuses a request without a captcha");

    // Kept so the reuse check below can spend the same challenge twice.
    const std::string signupCaptcha = machineCaptcha(app);
    const std::string signupBody =
        "{\"username\":\"TestPilot\",\"password\":\"long-enough-password\"," + signupCaptcha +
        "}";
    checkStatus(app, "POST", "/api/signup", signupBody, std::string(), 201, "signup creates a user",
                &body);
    rblx::web::Json signup = rblx::web::Json::parse(body, &error);
    const std::string token = signup.member("token").asString();
    check(!token.empty(), "signup returns a token");
    check(signup.member("userId").asInteger(0) > 0, "signup returns the user id");

    checkStatus(app, "POST", "/api/signup",
                "{\"username\":\"TestPilot\",\"password\":\"long-enough-password\"," +
                    machineCaptcha(app) + "}",
                std::string(), 409, "signup rejects a duplicate username");

    checkStatus(app, "POST", "/api/login",
                "{\"username\":\"TestPilot\",\"password\":\"wrong-password\"," +
                    machineCaptcha(app) + "}",
                std::string(), 401, "login rejects a wrong password");

    checkStatus(app, "POST", "/api/login",
                "{\"username\":\"TestPilot\",\"password\":\"long-enough-password\"," +
                    machineCaptcha(app) + "}",
                std::string(), 200, "login accepts the right password", &body);

    checkStatus(app, "POST", "/api/login",
                "{\"username\":\"TestPilot\",\"password\":\"long-enough-password\"," +
                    signupCaptcha + "}",
                std::string(), 400, "a captcha cannot be answered twice");
    checkContains(body, "\"token\"", "login returns a token");

    checkStatus(app, "POST", "/api/verify", "{}", token, 200, "verify accepts a bearer token");

    checkStatus(app, "POST", "/api/games", "{\"title\":\"X\"}", token, 400,
                "creating a game rejects a short title");

    checkStatus(app, "POST", "/api/games", "{\"title\":\"X\",\"worldText\":\"nope\"}", token, 400,
                "creating a game validates the world");

    const std::string createGameBody = "{\"title\":\"Test World\",\"description\":\"A world\","
                                       "\"isPublic\":true,\"worldText\":\"1\\n" +
                                       validWorldLine() + "\\n\"}";
    checkStatus(app, "POST", "/api/games", createGameBody, token, 201, "creating a game succeeds",
                &body);
    rblx::web::Json created = rblx::web::Json::parse(body, &error);
    const int gameId = static_cast<int>(created.member("game").member("id").asInteger(0));
    check(gameId > 0, "the created game has an id");
    check(created.member("game").member("canEdit").asBool(), "the owner sees canEdit");

    checkStatus(app, "GET", "/api/games", std::string(), std::string(), 200,
                "GET /api/games lists the game", &body);
    checkContains(body, "Test World", "the published game appears in the list");

    checkStatus(app, "GET", "/api/games/" + std::to_string(gameId), std::string(), std::string(),
                200, "GET /api/games/:id works for the public", &body);
    checkContains(body, "\"servers\":[]", "a game starts with no servers");

    checkStatus(app, "POST", "/api/games/" + std::to_string(gameId) + "/play",
                "{\"launchClient\":false}", token, 200, "POST play returns a launch", &body);
    rblx::web::Json play = rblx::web::Json::parse(body, &error);
    checkEqual(play.member("player").member("reason").asString(), "Client launch was skipped.",
               "play honours launchClient:false");
    check(play.member("server").member("port").asInteger(0) >= 7777,
          "play allocates a game server port");
    check(play.member("game").member("stats").member("launchCount").asInteger(0) == 1,
          "play increments the launch count");

    checkStatus(app, "GET", "/api/games/" + std::to_string(gameId) + "/servers", std::string(),
                std::string(), 200, "GET servers lists the instance", &body);
    checkContains(body, "\"status\":\"manual\"",
                  "a manual instance is reported when Server.exe is missing");

    const std::string avatarBody =
        "{\"avatar\":{\"headColor\":[0.1,0.2,0.3],\"torsoColor\":[0.1,0.2,0.3],"
        "\"leftArmColor\":[0.1,0.2,0.3],\"rightArmColor\":[0.1,0.2,0.3],"
        "\"leftLegColor\":[0.1,0.2,0.3],\"rightLegColor\":[0.1,0.2,0.3],\"faceId\":\"wink\"}}";
    checkStatus(app, "POST", "/api/avatar", avatarBody, token, 200, "POST /api/avatar saves");
    checkStatus(app, "GET", "/api/avatar", std::string(), token, 200, "GET /api/avatar reads",
                &body);
    checkEqual(rblx::web::Json::parse(body, &error).member("avatar").member("faceId").asString(),
               "wink", "the saved avatar face is returned");

    checkStatus(app, "POST", "/api/avatar", "{\"avatar\":{\"headColor\":[9,0,0]}}", token, 400,
                "POST /api/avatar rejects bad colours");

    checkStatus(app, "POST", "/api/me/playtime", "{\"seconds\":120}", token, 200,
                "POST /api/me/playtime records time", &body);
    check(rblx::web::Json::parse(body, &error)
              .member("profile")
              .member("stats")
              .member("playtimeSeconds")
              .asInteger(0) == 120,
          "playtime is added to the profile");

    checkStatus(app, "POST", "/api/me/playtime", "{\"seconds\":0}", token, 400,
                "POST /api/me/playtime rejects zero");

    checkStatus(app, "GET", "/api/me/social", std::string(), token, 200,
                "GET /api/me/social returns the profile", &body);
    checkContains(body, "\"relationship\":\"self\"", "the social payload marks the viewer");

    checkStatus(app, "GET", "/api/users/search?q=Test", std::string(), token, 200,
                "GET /api/users/search works", &body);
    checkContains(body, "\"users\":[]", "search hides the viewer");

    checkStatus(app, "POST", "/api/friends/request",
                "{\"userId\":" + std::to_string(signup.member("userId").asInteger(0)) + "}", token,
                400, "adding yourself as a friend is rejected");

    checkStatus(app, "GET", "/api/games/mine", std::string(), token, 200,
                "GET /api/games/mine lists owned games", &body);
    checkContains(body, "Test World", "the owned game appears");

    checkStatus(app, "PUT", "/api/games/" + std::to_string(gameId),
                "{\"description\":\"Updated description\"}", token, 200,
                "PUT /api/games/:id updates", &body);
    checkContains(body, "Updated description", "the update is applied");

    checkStatus(app, "GET", "/api/admin/overview", std::string(), std::string(), 401,
                "the admin overview requires a token");

    checkStatus(app, "POST", "/api/signup",
                "{\"username\":\"Outsider\",\"password\":\"long-enough-password\"," +
                    machineCaptcha(app) + "}",
                std::string(), 201, "a second account signs up", &body);
    const std::string outsiderToken =
        rblx::web::Json::parse(body, &error).member("token").asString();
    checkStatus(app, "GET", "/api/admin/overview", std::string(), outsiderToken, 403,
                "the admin overview rejects a normal account");

    checkStatus(app, "POST", "/api/verify", "{}", token, 200, "verify reports the admin flag",
                &body);
    checkContains(body, "\"isAdmin\":true", "verify marks the admin account");
    checkStatus(app, "POST", "/api/verify", "{}", outsiderToken, 200,
                "verify answers for a normal account", &body);
    checkContains(body, "\"isAdmin\":false", "verify marks a normal account as not admin");

    checkStatus(app, "GET", "/api/admin/overview", std::string(), token, 200,
                "the admin overview lists site data", &body);
    // The store seeds a creator account, so the count covers it plus the two signups.
    checkContains(body, "\"userCount\":3", "the overview counts users");
    checkContains(body, "\"username\":\"Outsider\"", "the overview lists every account");
    checkContains(body, "\"gameCount\":1", "the overview counts games");
    checkContains(body, "Test World", "the overview lists games");
    checkContains(body, "\"isAdmin\":true", "the overview flags the admin account");

    checkStatus(app, "GET", "/admin", std::string(), std::string(), 200,
                "the admin page is served", &body);
    checkContains(body, "data-admin-page", "the admin page markup is returned");

    checkStatus(app, "DELETE", "/api/admin/games/" + std::to_string(gameId), std::string(),
                outsiderToken, 403, "deleting a game needs an admin");
    checkStatus(app, "DELETE", "/api/admin/games/" + std::to_string(gameId), std::string(), token,
                200, "an admin deletes a game", &body);
    checkContains(body, "\"deletedGameId\":" + std::to_string(gameId),
                  "the deleted game id is returned");
    checkStatus(app, "GET", "/api/games/" + std::to_string(gameId), std::string(), std::string(),
                404, "the deleted game is gone");

    checkStatus(app, "GET", "/does-not-exist", std::string(), std::string(), 404,
                "unknown paths return 404");

    checkStatus(app, "POST", "/api/verify", "{bad json", token, 400,
                "POST /api/verify rejects malformed JSON");

    // The stored document must survive a restart.
    {
        rblx::web::Store reloaded(config.dataPath);
        std::string loadError;
        check(reloaded.load(&loadError), "the app persisted its data file");
        check(reloaded.findUserByName("TestPilot") != nullptr, "the account survived the write");
    }

    app.stop();
}

// Starter games must show up in an install that already has a database, not only in a
// brand new one: adding a starter game used to leave existing installs without it.
void testStarterSeeding() {
    const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const auto root = std::filesystem::temp_directory_path() /
                      ("rblx-starter-seed-" + std::to_string(stamp));
    std::filesystem::create_directories(root / "games");

    const std::string worldText = "1\n" + validWorldLine() + "\n";
    const std::string scriptText = "-- starter script\nprint(\"hello from the platformer\")\n";
    {
        std::ofstream base((root / "ServerWorld.world").string().c_str(),
                           std::ios::binary | std::ios::trunc);
        base << worldText;
    }

    auto platformerWorld = [&root]() { return (root / "games" / "sky-platformer.world").string(); };
    auto platformerScript = [&root]() { return (root / "games" / "sky-platformer.rbx").string(); };

    rblx::web::Config config;
    config.nodeEnv = "test";
    config.passwordIterations = 800;
    config.gameInstanceEmptyGraceMs = 0;
    config.port = 0;
    config.dataPath = (root / "data" / "data.json").string();
    config.webRoot = (root / "web").string();
    config.repoRoot = root.string();
    config.gameWorldsDir = (root / "data" / "game-worlds").string();

    auto findByTitle = [](rblx::web::Store& store, const std::string& title) {
        for (const rblx::web::GameRecord* game : store.allGames()) {
            if (game->title == title) {
                return game;
            }
        }
        return static_cast<const rblx::web::GameRecord*>(nullptr);
    };

    // An install that predates the platformer: only the baseplate was shipped.
    {
        rblx::web::WebsiteApp app(config);
        std::string error;
        check(app.initialize(&error), "seeding: an old install initialises: " + error);
        check(findByTitle(app.store(), "Starter Baseplate") != nullptr,
              "seeding: the baseplate is published");
        check(findByTitle(app.store(), "Sky Platformer") == nullptr,
              "seeding: a starter game whose assets are absent is skipped");
    }

    // The same install after the platformer ships with the build.
    {
        std::ofstream world(platformerWorld().c_str(), std::ios::binary | std::ios::trunc);
        world << worldText;
        std::ofstream script(platformerScript().c_str(), std::ios::binary | std::ios::trunc);
        script << scriptText;
    }

    {
        rblx::web::WebsiteApp app(config);
        std::string error;
        check(app.initialize(&error), "seeding: the existing install restarts: " + error);
        const rblx::web::GameRecord* platformer = findByTitle(app.store(), "Sky Platformer");
        check(platformer != nullptr, "seeding: an existing install gains the new starter game");
        check(platformer != nullptr && !platformer->scriptPath.empty(),
              "seeding: the starter script is copied next to the world");
        check(platformer != nullptr && std::filesystem::exists(platformer->scriptPath),
              "seeding: the copied starter script exists on disk");

        std::ifstream copied(platformer != nullptr ? platformer->scriptPath.c_str() : "",
                             std::ios::binary);
        std::ostringstream buffer;
        buffer << copied.rdbuf();
        checkEqual(buffer.str(), scriptText, "seeding: the copied starter script keeps its contents");
    }

    // Restarting again must not duplicate what is already published.
    {
        rblx::web::WebsiteApp app(config);
        std::string error;
        check(app.initialize(&error), "seeding: a third start succeeds: " + error);
        int platformers = 0;
        for (const rblx::web::GameRecord* game : app.store().allGames()) {
            if (game->title == "Sky Platformer") {
                ++platformers;
            }
        }
        checkEqual(std::to_string(platformers), "1", "seeding: starter games are not duplicated");
    }
}

}  // namespace

int main() {
    testCrypto();
    testJwt();
    testJson();
    testWorld();
    testStore();
    testStarterSeeding();
    testHttp();

    if (failures == 0) {
        std::printf("\nAll native web app checks passed.\n");
        return 0;
    }
    std::printf("\n%d native web app check(s) failed.\n", failures);
    return 1;
}
