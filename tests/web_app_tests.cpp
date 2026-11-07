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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

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
    std::ofstream editor((directory / "public" / "editor.html").string().c_str(),
                         std::ios::binary | std::ios::trunc);
    editor << "<!doctype html><title>editor</title><div data-editor-page></div>";
    editor.close();
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

    // The proof a server detection carries and the gameplay it was held against
    // travel with it: held until the wave, kept on the action the wave files, and
    // still there after a reload.
    const int watcher = store.createUser("Watcher", "hash-four", "2026-01-05T00:00:00Z");
    const std::string proof =
        "claimed 0.00, 40.00, 0.00 from 0.00, 0.00, 0.00 after 250.00 ms: 0.00 units "
        "horizontal, 40.00 units of climb";
    const std::string expected =
        "walking is 30.00 units/s with 0.40 units of slack, so at most 7.90 units this "
        "frame; a jump rises 14.00 units/s with 0.75 units of slack, so at most 4.25 "
        "units, against 22.00 units of climb budget";
    const std::string clip =
        "1000:0.0,40.0,0.0:0.0,5.0,0.0|1016:1010.0,40.0,0.0:0.9,5.0,0.0";
    store.addPendingDetection(watcher, "flying", proof, expected, clip, 1000);
    const auto& held = store.pendingDetectionsFor(watcher);
    check(held.size() == 1 && held[0].proof == proof,
          "store keeps the proof with a held detection");
    check(store.pendingDetectionsFor(watcher)[0].replay == clip,
          "store keeps the clip with a held detection");
    store.addInfraction(watcher, "warning", "flying", true, 1, std::string(),
                        "2026-01-05T00:00:00Z", 1, proof, expected, clip);
    check(store.infractionsFor(watcher).size() == 1 &&
              store.infractionsFor(watcher)[0].expected == expected,
          "store keeps the expectation on the action a detection filed");

    check(store.save(&error), "store writes the record with its proof");
    rblx::web::Store proofReloaded(path);
    check(proofReloaded.load(&error), "the record with proof reloads");
    check(proofReloaded.pendingDetectionsFor(watcher).size() == 1 &&
              proofReloaded.pendingDetectionsFor(watcher)[0].expected == expected,
          "the expectation survives a reload with the held detection");
    check(proofReloaded.infractionsFor(watcher).size() == 1 &&
              proofReloaded.infractionsFor(watcher)[0].proof == proof,
          "the proof survives a reload on the action");
    check(proofReloaded.infractionsFor(watcher)[0].replay == clip,
          "the clip survives a reload on the action");
}

// ---------------------------------------------------------------------------
// http end to end
// ---------------------------------------------------------------------------

using TestSocket = SOCKET;
const TestSocket kInvalidTestSocket = INVALID_SOCKET;
void closeTestSocket(TestSocket handle) { closesocket(handle); }

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
        const int written = ::send(handle, raw.data() + sent,
                                   static_cast<int>(raw.size() - sent), 0);
        if (written <= 0) {
            closeTestSocket(handle);
            return std::string();
        }
        sent += static_cast<std::size_t>(written);
    }

    std::string response;
    char buffer[4096];
    while (true) {
        const int received = ::recv(handle, buffer, static_cast<int>(sizeof(buffer)), 0);
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

// The website seals its data file with a key derived from its configuration, so a
// test that reopens that file has to open it the same way.
void openSealed(rblx::web::Store& store, const rblx::web::Config& config) {
    store.setDataKey(rblx::web::dataKeyFromText("jwt:" + config.jwtSecret));
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
    // The suite deliberately hammers the endpoints over loopback.
    config.rateLimitMax = 100000;
    // Zero: the wave lands on the next request, which keeps these checks immediate
    // while still going through the queue a real install uses.
    config.banWaveWindowMs = 0;
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

    // The website's code editor loads and saves the .rbx script the game server
    // launches with. It is the owner's file, so another account is told the game
    // does not exist, and what is saved is what the next run plays.
    const std::string scriptRoute = "/api/games/" + std::to_string(gameId) + "/script";
    checkStatus(app, "GET", scriptRoute, std::string(), std::string(), 401,
                "reading a script needs a token");
    checkStatus(app, "GET", scriptRoute, std::string(), outsiderToken, 404,
                "another account cannot read the script");
    checkStatus(app, "PUT", scriptRoute, "{\"text\":\"-- nope\"}", outsiderToken, 404,
                "another account cannot save the script");
    checkStatus(app, "GET", scriptRoute, std::string(), token, 200, "the owner reads the script",
                &body);
    checkContains(body, "\"text\":\"\"", "a game without a script reads as empty");
    checkContains(body, "\"limit\":262144", "the editor is told the size limit");

    const std::string editorScript = "-- saved from the website\nprint(\"hello\")\n";
    checkStatus(app, "PUT", scriptRoute,
                "{\"text\":\"-- saved from the website\\nprint(\\\"hello\\\")\\n\"}", token, 200,
                "the owner saves a script", &body);
    checkContains(body, "\"bytes\":" + std::to_string(editorScript.size()),
                  "the saved size is reported");
    checkStatus(app, "GET", scriptRoute, std::string(), token, 200, "the script reads back",
                &body);
    checkContains(body, "saved from the website", "the saved text is what the editor reads");
    checkContains(body, "hello", "the whole script is returned");

    {
        rblx::web::Store reloaded(config.dataPath);
        openSealed(reloaded, config);
        std::string loadError;
        check(reloaded.load(&loadError), "the script path reaches the data file");
        const rblx::web::GameRecord* saved = reloaded.findGame(gameId);
        check(saved != nullptr && !saved->scriptPath.empty(),
              "the game remembers where its script is");
        check(saved != nullptr && std::filesystem::file_size(saved->scriptPath) ==
                                     editorScript.size(),
              "the script file holds what the editor saved");
    }

    const std::string oversize(300 * 1024, 'a');
    checkStatus(app, "PUT", scriptRoute, "{\"text\":\"" + oversize + "\"}", token, 413,
                "an oversized script is refused");

    checkStatus(app, "GET", "/editor", std::string(), std::string(), 200,
                "the editor page is served", &body);
    checkContains(body, "data-editor-page", "the editor page markup is returned");

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

    // Moderation. The ladder itself is covered by ModerationTests; these checks
    // are about an account restricted here being unable to sign in or keep a
    // session, which is the part a player actually feels.
    const rblx::web::UserRecord* outsider = app.store().findUserByName("Outsider");
    check(outsider != nullptr, "the second account exists");
    const std::string outsiderId = std::to_string(outsider != nullptr ? outsider->id : 0);

    // Coins are earned by playtime, which is why the dashboard hands them out
    // rather than editing the balance. The grant writes the same balance a hat is
    // bought from, so it is checked here: refused for a normal account, a missing
    // account and an amount that is not a positive whole number, then read back by
    // the account and from the data file.
    checkStatus(app, "POST", "/api/admin/users/" + outsiderId + "/coins",
                "{\"amount\":250}", outsiderToken, 403,
                "a normal account cannot give coins");
    checkStatus(app, "POST", "/api/admin/users/999999/coins", "{\"amount\":10}", token, 404,
                "coins cannot be given to a missing account");
    checkStatus(app, "POST", "/api/admin/users/" + outsiderId + "/coins",
                "{\"amount\":0}", token, 400, "a grant of zero coins is refused");
    checkStatus(app, "POST", "/api/admin/users/" + outsiderId + "/coins",
                "{\"amount\":-5}", token, 400, "a grant only ever adds coins");
    checkStatus(app, "POST", "/api/admin/users/" + outsiderId + "/coins",
                "{\"amount\":250}", token, 200, "an admin gives coins", &body);
    checkContains(body, "\"coins\":250", "the new balance is reported back");
    checkStatus(app, "GET", "/api/me/social", std::string(), outsiderToken, 200,
                "the account reads its own profile", &body);
    checkContains(body, "\"coins\":250", "the account sees the coins it was given");
    {
        rblx::web::Store reloaded(config.dataPath);
        openSealed(reloaded, config);
        std::string loadError;
        check(reloaded.load(&loadError), "the grant reaches the data file");
        check(reloaded.coinsFor(outsider != nullptr ? outsider->id : 0) == 250,
              "the granted coins survive a restart");
    }

    checkStatus(app, "POST", "/api/admin/users/" + outsiderId + "/moderation",
                "{\"action\":\"ban\"}", outsiderToken, 403,
                "a normal account cannot moderate anybody");

    checkStatus(app, "POST", "/api/admin/users/" + outsiderId + "/moderation",
                "{\"action\":\"ban\"}", token, 200, "an admin bans an account", &body);
    checkContains(body, "\"banned\":true", "the ban is reported back");

    checkStatus(app, "GET", "/api/admin/overview", std::string(), token, 200,
                "the overview still loads after a ban", &body);
    checkContains(body, "\"banned\":true", "the dashboard shows the ban");

    // The ban is shown, not used to close the site: the account signs in, keeps
    // the session it already held, and can read what happened and appeal it. The
    // game is the door that stays shut, and the game server reads `restricted`
    // from this same answer to shut it.
    checkStatus(app, "POST", "/api/login",
                "{\"username\":\"Outsider\",\"password\":\"long-enough-password\"," +
                    machineCaptcha(app) + "}",
                std::string(), 200, "a banned account can still sign in", &body);
    checkStatus(app, "POST", "/api/verify", "{}", outsiderToken, 200,
                "a banned account keeps the session it already held", &body);
    checkContains(body, "\"restricted\":true", "the session says the account is restricted");
    checkContains(body, "\"standing\":\"banned\"", "the session names the standing");
    checkContains(body, "\"banned\":true", "the session carries the ban");

    checkStatus(app, "POST", "/api/admin/users/" + outsiderId + "/moderation",
                "{\"action\":\"pardon\"}", token, 200, "an admin pardons the account", &body);
    checkContains(body, "\"banned\":false", "the pardon clears the ban");
    checkStatus(app, "POST", "/api/login",
                "{\"username\":\"Outsider\",\"password\":\"long-enough-password\"," +
                    machineCaptcha(app) + "}",
                std::string(), 200, "a pardoned account can sign in again");

    const rblx::web::UserRecord* owner = app.store().firstUser();
    checkStatus(app, "POST",
                "/api/admin/users/" + std::to_string(owner != nullptr ? owner->id : 0) +
                    "/moderation",
                "{\"action\":\"ban\"}", token, 403,
                "the account that owns the install cannot be restricted");

    checkStatus(app, "POST", "/api/admin/users/" + outsiderId + "/moderation",
                "{\"action\":\"suspend\",\"hours\":2}", token, 200,
                "an admin suspends an account", &body);
    checkContains(body, "suspended", "the suspension is reported back");

    // A restriction that is not written down is not enforcement, so the account
    // is reloaded from the data file it was saved to.
    {
        rblx::web::Store reloaded(config.dataPath);
        openSealed(reloaded, config);
        std::string loadError;
        check(reloaded.load(&loadError), "the moderation state reaches the data file");
        const rblx::web::UserRecord* saved = reloaded.findUserByName("Outsider");
        check(saved != nullptr && saved->suspendedUntilMs > 0,
              "the suspension survives a restart");
    }

    // Standing: the account reads its own record, so the punishment is a reason
    // and a date it can argue with instead of a door that will not open.
    checkStatus(app, "GET", "/api/account/standing", std::string(), outsiderToken, 200,
                "a suspended account can read its standing", &body);
    checkContains(body, "\"standing\":\"suspended\"", "the standing names the suspension");
    checkContains(body, "\"infractions\":[", "the record lists what led here");
    checkContains(body, "\"kind\":\"ban\"", "the earlier ban is in the record");
    checkContains(body, "\"canAppeal\":true", "a suspended account may appeal");

    // An appeal is about one named action, so the record's own ids are what the
    // account sends back.
    int suspensionAction = 0;
    const rblx::web::Json standingAfterBan = rblx::web::Json::parse(body, &error);
    for (const rblx::web::Json& entry : standingAfterBan.member("infractions").elements()) {
        if (entry.member("kind").asString() == "suspension") {
            suspensionAction = static_cast<int>(entry.member("id").asInteger(0));
            break;
        }
    }
    check(suspensionAction > 0, "the record names the action that suspended the account");

    // An appeal names one action, has to say something an admin can judge, and
    // that action can be appealed once.
    const std::string appealTo = "{\"infractionId\":" + std::to_string(suspensionAction) + ",";
    checkStatus(app, "POST", "/api/account/appeal",
                appealTo + "\"message\":\"This account has done nothing wrong at all.\"}",
                token, 400, "an account cannot appeal an action that is not its own");
    checkStatus(app, "POST", "/api/account/appeal",
                appealTo + "\"message\":\"too short\"}", outsiderToken, 400,
                "a one-line appeal is refused");
    checkStatus(app, "POST", "/api/account/appeal",
                appealTo + "\"message\":\"The detection was a false positive from a stutter.\"}",
                outsiderToken, 201, "a suspended account appeals the action it is under", &body);
    checkContains(body, "\"status\":\"open\"", "the appeal starts open");
    int appealId = static_cast<int>(
        rblx::web::Json::parse(body, &error).member("appeal").member("id").asInteger(0));
    check(appealId > 0, "the filed appeal has an id");
    checkStatus(app, "POST", "/api/account/appeal",
                appealTo + "\"message\":\"A second appeal about the same action.\"}",
                outsiderToken, 409, "the same action cannot be appealed twice");
    checkStatus(app, "GET", "/api/account/standing", std::string(), outsiderToken, 200,
                "the standing shows the waiting appeal", &body);
    checkContains(body, "\"hasOpenAppeal\":true", "the appeal is flagged as waiting");
    checkContains(body, "\"appealStatus\":\"open\"", "the action it appeals is marked");

    // An admin reviews it from the dashboard. Accepting is a pardon, and the
    // record keeps who lifted it and why.
    checkStatus(app, "GET", "/api/admin/overview", std::string(), token, 200,
                "the dashboard loads with an appeal waiting", &body);
    checkContains(body, "\"appeals\":[", "the dashboard lists appeals");
    checkContains(body, "\"infractionReason\":", "the dashboard reads the reason behind the appeal");
    checkContains(body, "false positive from a stutter", "the appeal text reaches the admin");
    checkStatus(app, "POST", "/api/admin/appeals/" + std::to_string(appealId),
                "{\"action\":\"accept\"}", outsiderToken, 403,
                "a normal account cannot answer an appeal");
    checkStatus(app, "POST", "/api/admin/appeals/" + std::to_string(appealId),
                "{\"action\":\"accept\"}", token, 200, "an admin accepts the appeal", &body);
    checkContains(body, "\"status\":\"accepted\"", "the decision is reported back");
    checkContains(body, "\"banned\":false", "accepting lifts the restriction");
    checkStatus(app, "POST", "/api/admin/appeals/" + std::to_string(appealId),
                "{\"action\":\"deny\"}", token, 409,
                "a reviewed appeal cannot be answered twice");

    {
        rblx::web::Store reloaded(config.dataPath);
        openSealed(reloaded, config);
        std::string loadError;
        check(reloaded.load(&loadError), "the appeal reaches the data file");
        const rblx::web::AppealRecord* saved = reloaded.findAppeal(appealId);
        check(saved != nullptr && saved->status == "accepted",
              "the accepted appeal survives a restart");
        const rblx::web::UserRecord* account = reloaded.findUserByName("Outsider");
        check(account != nullptr && !account->banned && account->suspendedUntilMs == 0,
              "the pardon survives a restart");
        check(account != nullptr && account->infractions.size() >= 3,
              "the account's whole record survives a restart");
    }

    checkStatus(app, "GET", "/api/account/standing", std::string(), outsiderToken, 200,
                "the standing reflects the accepted appeal", &body);
    checkContains(body, "\"standing\":\"clear\"", "the account is clear after the appeal");
    checkContains(body, "\"kind\":\"pardon\"", "the record shows the restriction was lifted");
    checkContains(body, "\"appealStatus\":\"accepted\"",
                  "the action shows its appeal was accepted");

    // The client's own anti-cheat reports what it finds inside the process. That
    // report can only be filed by the account it runs under, and it is counted
    // as evidence: no strike is added, because the client reporting it is the
    // one being cheated.
    checkStatus(app, "POST", "/api/client-detections",
                "{\"flag\":\"anticheat\",\"detail\":\"Module was modified\"}", std::string(),
                401, "a client detection needs a signed-in account");
    checkStatus(app, "POST", "/api/client-detections",
                "{\"flag\":\"anticheat\",\"detail\":\"Module was modified\"}", token, 200,
                "a signed-in client files a detection", &body);
    checkContains(body, "\"clientDetections\":1", "the detection is counted on the account");
    checkContains(body, "\"queued\":true", "a token-bearing detection is held for the wave");
    checkContains(body, "\"restricted\":false", "a warning does not end the session");
    checkStatus(app, "POST", "/api/client-detections", "{\"detail\":\"  \"}", token, 400,
                "a detection without a detail is refused");

    checkStatus(app, "GET", "/api/admin/overview", std::string(), token, 200,
                "the overview loads after a detection", &body);
    checkContains(body, "\"clientDetections\":1", "the dashboard counts client detections");
    checkContains(body, "\"cheatStrikes\":1", "a client detection adds a strike");
    checkContains(body, "client anti-cheat: Module was modified",
                  "the dashboard reads what the detection was");
    checkContains(body, "\"stage\":\"anticheat\"", "the detection is listed for the dashboard");

    {
        rblx::web::Store reloaded(config.dataPath);
        openSealed(reloaded, config);
        std::string loadError;
        check(reloaded.load(&loadError), "the detection count reaches the data file");
        const rblx::web::UserRecord* saved = reloaded.findUserByName("TestPilot");
        check(saved != nullptr && saved->clientDetections == 1,
              "the detection count survives a restart");
        check(saved != nullptr && saved->cheatStrikes == 1,
              "the strike from a detection survives a restart");
    }

    // The punishment is the point of binding a detection to a token, so a fresh
    // account is walked up the ladder with nothing but its own client's reports.
    checkStatus(app, "POST", "/api/signup",
                "{\"username\":\"Cheater\",\"password\":\"long-enough-password\"," +
                    machineCaptcha(app) + "}",
                std::string(), 201, "an account for the detection ladder signs up", &body);
    const std::string cheaterToken =
        rblx::web::Json::parse(body, &error).member("token").asString();

    checkStatus(app, "POST", "/api/client-detections",
                "{\"flag\":\"anticheat\",\"detail\":\"Debugger attached\"}", cheaterToken, 200,
                "the first detection on a fresh account is held", &body);
    checkContains(body, "\"restricted\":false", "one detection leaves the account playing");
    checkStatus(app, "POST", "/api/client-detections",
                "{\"flag\":\"anticheat\",\"detail\":\"Thread was hijacked\"}", cheaterToken,
                200, "the second detection is held too", &body);
    checkStatus(app, "POST", "/api/client-detections",
                "{\"flag\":\"anticheat\",\"detail\":\"Module was modified\"}", cheaterToken,
                200, "the third detection is held", &body);
    // The next request is the wave: it spends the one step still held and lands the
    // third strike, so the ladder is climbed by waves rather than by reports.
    checkStatus(app, "GET", "/api/account/standing", std::string(), cheaterToken, 200,
                "the wave answers the ladder account", &body);
    checkContains(body, "\"strikes\":3", "three detections climbed three steps");
    checkContains(body, "\"standing\":\"suspended\"", "the third strike suspends the account");
    checkContains(body, "\"pendingDetections\":0", "the wave clears what it acted on");
    checkContains(body, "\"restricted\":true", "the account is restricted after the wave");
    check(body.find("\"reason\":") == std::string::npos,
          "the account's own record does not say what was detected");
    checkContains(body, "suspended for", "the standing says how long is left");

    checkStatus(app, "POST", "/api/login",
                "{\"username\":\"Cheater\",\"password\":\"long-enough-password\"," +
                    machineCaptcha(app) + "}",
                std::string(), 200, "a suspended account can still sign in", &body);
    checkStatus(app, "POST", "/api/verify", "{}", cheaterToken, 200,
                "a suspended account keeps its session", &body);
    checkContains(body, "\"restricted\":true", "the session names the restriction");
    checkContains(body, "suspended for cheating", "the session carries the reason");

    checkStatus(app, "GET", "/does-not-exist", std::string(), std::string(), 404,
                "unknown paths return 404");

    checkStatus(app, "POST", "/api/verify", "{bad json", token, 400,
                "POST /api/verify rejects malformed JSON");

    // The stored document must survive a restart.
    {
        rblx::web::Store reloaded(config.dataPath);
        openSealed(reloaded, config);
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


// The site-wide budget: every /api/ call counts per address, a refusal is not
// counted as another call, and a page request is not part of the budget.
// Detections are held and acted on in waves: a report on its own changes nothing,
// two reports inside one window are one step, and the dashboard is the only place
// the reason and the evidence count are readable.
void testBanWaves() {
    const std::string webRoot = testWebRoot();

    rblx::web::Config config;
    config.nodeEnv = "test";
    config.passwordIterations = 800;
    config.authRateLimitMax = 1000;
    config.rateLimitMax = 100000;
    config.dataPath = webRoot + "/banwave.json";
    config.webRoot = webRoot;
    config.repoRoot = webRoot;
    config.gameWorldsDir = webRoot + "/game-worlds";
    config.port = 0;
    config.gameInstanceEmptyGraceMs = 0;
    config.publicBaseUrl = "http://127.0.0.1:3000";
    // Long enough that two reports in a row are certainly inside one window.
    config.banWaveWindowMs = 2000;
    config.adminUsernames = {"WavePilot"};

    rblx::web::WebsiteApp app(config);
    std::string error;
    check(app.initialize(&error), "ban wave: app initialises: " + error);
    app.installRoutes();
    check(app.start(&error), "ban wave: app listens: " + error);
    if (app.server().port() <= 0) {
        return;
    }

    std::string body;
    checkStatus(app, "POST", "/api/signup",
                "{\"username\":\"WavePilot\",\"password\":\"long-enough-password\"," +
                    machineCaptcha(app) + "}",
                std::string(), 201, "ban wave: the account signs up", &body);
    const std::string token = rblx::web::Json::parse(body, &error).member("token").asString();

    checkStatus(app, "POST", "/api/client-detections",
                "{\"flag\":\"anticheat\",\"detail\":\"Debugger attached\"}", token, 200,
                "ban wave: the first detection is held", &body);
    checkContains(body, "\"pending\":1", "ban wave: the detection is counted as held");
    checkStatus(app, "POST", "/api/client-detections",
                "{\"flag\":\"anticheat\",\"detail\":\"Thread was hijacked\"}", token, 200,
                "ban wave: a second detection in the window is held too", &body);

    checkStatus(app, "GET", "/api/account/standing", std::string(), token, 200,
                "ban wave: the account reads its standing", &body);
    checkContains(body, "\"strikes\":0", "ban wave: nothing is acted on before the window ends");
    checkContains(body, "\"pendingDetections\":2", "ban wave: both detections are still held");

    std::this_thread::sleep_for(std::chrono::milliseconds(2100));

    checkStatus(app, "GET", "/api/account/standing", std::string(), token, 200,
                "ban wave: the wave acts once the window has run out", &body);
    checkContains(body, "\"strikes\":1", "ban wave: two detections in one window cost one step");
    checkContains(body, "\"standing\":\"warned\"", "ban wave: the account lands on a warning");
    checkContains(body, "\"pendingDetections\":0", "ban wave: the wave clears its evidence");
    check(body.find("\"reason\":") == std::string::npos,
          "ban wave: the account is not shown what was detected");

    checkStatus(app, "GET", "/api/admin/overview", std::string(), token, 200,
                "ban wave: the dashboard loads", &body);
    checkContains(body, "client anti-cheat: Thread was hijacked",
                  "ban wave: the dashboard reads the reason");
    checkContains(body, "\"detections\":2", "ban wave: the dashboard sees the evidence count");

    app.stop();
}

// The recording behind an action is pulled by an admin when an appeal is judged,
// so the clip a detection carried has to reach the dashboard intact.
void testAdminReplay() {
    const std::string webRoot = testWebRoot();

    rblx::web::Config config;
    config.nodeEnv = "test";
    config.passwordIterations = 800;
    config.authRateLimitMax = 1000;
    config.rateLimitMax = 100000;
    config.dataPath = webRoot + "/replay.json";
    config.webRoot = webRoot;
    config.repoRoot = webRoot;
    config.gameWorldsDir = webRoot + "/game-worlds";
    config.port = 0;
    config.gameInstanceEmptyGraceMs = 0;
    config.publicBaseUrl = "http://127.0.0.1:3000";
    config.banWaveWindowMs = 0;
    config.adminUsernames = {"ReplayPilot"};

    rblx::web::WebsiteApp app(config);
    std::string error;
    check(app.initialize(&error), "replay: app initialises: " + error);
    app.installRoutes();
    check(app.start(&error), "replay: app listens: " + error);
    if (app.server().port() <= 0) {
        return;
    }

    std::string body;
    checkStatus(app, "POST", "/api/signup",
                "{\"username\":\"ReplayPilot\",\"password\":\"long-enough-password\"," +
                    machineCaptcha(app) + "}",
                std::string(), 201, "replay: the admin signs up", &body);
    app.stop();

    // The clip is filed by the game server, which has no account here, so the
    // action is written the way a wave writes one: into the data file.
    const std::string clip =
        "1000:10.0,5.0,-2.0:10.0,5.0,-2.0|1016:60.0,5.0,-2.0:10.6,5.0,-2.0";
    int actionId = 0;
    {
        rblx::web::Store store(config.dataPath);
        openSealed(store, config);
        check(store.load(&error), "replay: the data file loads: " + error);
        const rblx::web::UserRecord* pilot = store.findUserByName("ReplayPilot");
        check(pilot != nullptr, "replay: the admin is in the data file");
        if (pilot != nullptr) {
            store.addInfraction(pilot->id, "warning", "flying", true, 1, std::string(),
                                "2026-01-07T00:00:00Z", 1,
                                "claimed 60.00, 5.00, -2.00 from 10.00, 5.00, -2.00 after 16.00 ms",
                                "a jump rises 14.00 units/s with 0.75 units of slack", clip);
            actionId = store.infractionsFor(pilot->id).front().id;
            check(actionId > 0, "replay: the action has an id");
            check(store.save(&error), "replay: the action with its clip is written: " + error);
        }
    }

    rblx::web::WebsiteApp restarted(config);
    check(restarted.initialize(&error), "replay: the app restarts: " + error);
    restarted.installRoutes();
    check(restarted.start(&error), "replay: the restarted app listens: " + error);
    if (restarted.server().port() <= 0) {
        return;
    }

    checkStatus(restarted, "POST", "/api/login",
                "{\"username\":\"ReplayPilot\",\"password\":\"long-enough-password\"," +
                    machineCaptcha(restarted) + "}",
                std::string(), 200, "replay: the admin signs in", &body);
    const std::string token = rblx::web::Json::parse(body, &error).member("token").asString();
    check(!token.empty(), "replay: the login returns a token");

    const std::string route = "/api/admin/infractions/" + std::to_string(actionId) + "/replay";
    checkStatus(restarted, "GET", route, std::string(), token, 200,
                "replay: the admin reads the clip", &body);
    checkContains(body, "\"frames\":[{", "replay: the clip is a list of frames");
    checkContains(body, "\"scale\":100", "replay: the frames say what to divide by");
    checkContains(body, "\"proof\":\"claimed 60.00", "replay: the proof travels with the clip");
    checkContains(body, "\"ax\":1060", "replay: a frame carries the position the server accepted");
    checkContains(body, "\"x\":6000", "replay: a frame carries the position the client claimed");
    checkContains(body, "\"username\":\"ReplayPilot\"", "replay: the clip names the account");

    checkStatus(restarted, "GET", "/api/admin/infractions/999999/replay", std::string(), token, 404,
                "replay: an unknown action has no clip");
    checkStatus(restarted, "GET", route, std::string(), std::string(), 401,
                "replay: a signed-out request is refused");

    restarted.stop();
}

void testRateLimit() {
    const std::string webRoot = testWebRoot();

    rblx::web::Config config;
    config.nodeEnv = "test";
    config.passwordIterations = 800;
    config.authRateLimitMax = 1000;
    config.rateLimitMax = 3;
    config.rateLimitWindowMs = 60 * 1000;
    config.dataPath = webRoot + "/ratelimit.json";
    config.webRoot = webRoot;
    config.repoRoot = webRoot;
    config.gameWorldsDir = webRoot + "/game-worlds";
    config.port = 0;
    config.gameInstanceEmptyGraceMs = 0;
    config.publicBaseUrl = "http://127.0.0.1:3000";

    rblx::web::WebsiteApp app(config);
    std::string error;
    check(app.initialize(&error), "rate limit: app initialises: " + error);
    app.installRoutes();
    check(app.start(&error), "rate limit: app listens: " + error);
    if (app.server().port() <= 0) {
        return;
    }

    checkStatus(app, "GET", "/api/games", std::string(), std::string(), 200,
                "rate limit: the first call is allowed");
    checkStatus(app, "GET", "/api/games", std::string(), std::string(), 200,
                "rate limit: the second call is allowed");
    checkStatus(app, "GET", "/api/games", std::string(), std::string(), 200,
                "rate limit: the budget is spent exactly");

    std::thread serverThread([&app]() { app.server().serveOneConnection(); });
    const std::string response = sendRaw(
        app.server().port(),
        buildRequest("GET", "/api/games", std::string(), std::string(), "application/json"));
    serverThread.join();
    checkEqual(std::to_string(statusOf(response)), "429",
               "rate limit: the next call in the window is refused");
    checkContains(bodyOf(response), "Too many requests", "rate limit: the refusal says why");
    checkContains(response, "Retry-After:", "rate limit: the refusal says how long to wait");

    checkStatus(app, "GET", "/api/games", std::string(), std::string(), 429,
                "rate limit: a refusal does not extend the window");

    checkStatus(app, "GET", "/", std::string(), std::string(), 200,
                "rate limit: pages are not counted");
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
    testRateLimit();
    testBanWaves();
    testAdminReplay();

    if (failures == 0) {
        std::printf("\nAll native web app checks passed.\n");
        return 0;
    }
    std::printf("\n%d native web app check(s) failed.\n", failures);
    return 1;
}
