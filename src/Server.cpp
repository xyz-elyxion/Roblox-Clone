#include <exception>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <cstring> // for memset
#include <cmath>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <limits>
#include "Network.h"
#include "ReplayRecorder.h"
#include "MovementGuard.h"
#include "Part.h"
#include "PhysicsWorld.h"
#include "WorldLoader.h"
#include "ServerAuth.h"
#include "WorldScript.h"
#include "UdpTransport.h"
#include "HealthState.h"
#include "AvatarItems.h"
#include "Vehicle.h"

#include <condition_variable>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

// RBLXMATH Includes
#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/rblxmath.hpp>
#include <rblxmath/gtc/quaternion.hpp>
#include <rblxmath/gtx/quaternion.hpp>
#include <rblxmath/gtx/euler_angles.hpp>

const char* DEFAULT_FACE_ID = "classic";

struct ConnectedPlayer {
    rblx::net::UdpSession session;
    sockaddr_in address{};
    uint64_t lastReceiveAtMs = 0;
    int id;
    char username[32];
    PacketPlayerState state;
    bool active;
    int userId = -1;
    std::string authToken;
    // The hashed machine this player joined from (see HardwareId.h). Kept so the
    // website can be told which machine a detection happened on.
    std::string hardwareId;
    std::chrono::steady_clock::time_point lastPlaytimeReportedAt;
    rblxphysics::RigidBody* shadowBody; // Server-side physics body for player
    rblxmath::vec3 headColor = rblxmath::vec3(0.8f, 0.6f, 0.4f);
    rblxmath::vec3 torsoColor = rblxmath::vec3(0.2f, 0.4f, 0.8f);
    rblxmath::vec3 leftArmColor = rblxmath::vec3(0.8f, 0.6f, 0.4f);
    rblxmath::vec3 rightArmColor = rblxmath::vec3(0.8f, 0.6f, 0.4f);
    rblxmath::vec3 leftLegColor = rblxmath::vec3(0.2f, 0.6f, 0.2f);
    rblxmath::vec3 rightLegColor = rblxmath::vec3(0.2f, 0.6f, 0.2f);
    std::string faceId = DEFAULT_FACE_ID;
    // Equipped hat id from the account, or empty. The catalog lives in
    // AvatarItems.h, so an id this build does not know is dropped rather than
    // sent on to clients that could not draw it.
    std::string hatId;
    // The server's own view of where this player can legitimately be.
    rblx::movement::Guard movement;
    // Corrections already reported to the website, so one long episode earns
    // one strike rather than one per packet.
    int reportedCorrections = 0;
    // The last couple of seconds of packets, so a report carries the run-up to
    // what it accuses the player of rather than a single frame.
    rblx::replay::Recorder replay;
    // Health, death and the respawn timer are the server's, not the client's.
    rblx::health::State health;
    // Where a death puts this player back: the world's SpawnLocation.
    rblxmath::vec3 spawnPosition = rblxmath::vec3(0.0f, 10.0f, 0.0f);
};

std::map<int, ConnectedPlayer> players;
int nextPlayerId = 1;

// One UDP socket serves every player: a peer is identified by where its
// datagrams come from and the connection id inside them, not by a socket.
SOCKET gameSocket = INVALID_SOCKET;
std::map<uint64_t, int> peerAddresses;
const int kDatagramsPerTick = 256;
const uint64_t kPeerTimeoutMs = 10000;

uint64_t steadyMilliseconds() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
}

uint64_t addressKey(const sockaddr_in& address) {
    return (static_cast<uint64_t>(ntohl(address.sin_addr.s_addr)) << 16) |
           static_cast<uint64_t>(ntohs(address.sin_port));
}

ConnectedPlayer* findPlayerByAddress(const sockaddr_in& address) {
    const auto entry = peerAddresses.find(addressKey(address));
    if (entry == peerAddresses.end()) {
        return nullptr;
    }
    const auto player = players.find(entry->second);
    if (player == players.end()) {
        peerAddresses.erase(entry);
        return nullptr;
    }
    return &player->second;
}

bool sendToPlayer(ConnectedPlayer& player, const std::vector<char>& frame, bool reliable) {
    if (frame.empty()) {
        return false;
    }
    return player.session.sendMessage(frame.data(), frame.size(), reliable);
}

rblx::net::UdpSession::SendFunction playerSendFunction(const ConnectedPlayer& player) {
    const sockaddr_in address = player.address;
    return [address](const char* data, size_t size) {
        if (gameSocket == INVALID_SOCKET) {
            return false;
        }
        const int sent = sendto(gameSocket, data, static_cast<int>(size), 0,
                                reinterpret_cast<const sockaddr*>(&address), sizeof(address));
        return sent == static_cast<int>(size);
    };
}
std::string webServerUrl = "http://localhost:3000"; // Web server URL for token verification
std::string serverLogPath; // --log: a copy of this output for the website that launched us
std::string gameInstanceId;
std::string gameInstanceToken;
std::string scriptPath;
std::unique_ptr<rblx::script::Interpreter> gameScriptInterpreter;
std::unique_ptr<rblx::worldscript::WorldRuntime> gameScriptRuntime;

// Sounds the script asked for during the tick being built. They are collected
// and sent once instead of one packet per call: a door, a wardrobe and an entity
// in one tick are one burst out, not three.
std::vector<PacketSound> pendingSounds;

// A script that plays a sound from every touched part would otherwise turn into
// a flood, so a tick carries a few and drops the rest: the player already heard
// the first ones, and the mixer has eight voices anyway.
constexpr std::size_t kMaxSoundsPerTick = 8;
std::map<int, rblxmath::vec3> lastBroadcastPositions;

// What the clients already know about the world: every part that was in the join
// snapshot, then each part a script built, then each one it destroyed. A script
// that builds its level as it goes needs this, because the snapshot only ever
// describes the world as it was when a player arrived.
std::size_t announcedPartCount = 0;
std::vector<uint8_t> announcedPartRemovals;

// How each announced part looked when the clients were last told about it. A
// script can repaint a part it never built - the Doors hotel turns its lobby
// lights red before the entity comes - and a colour is not a position, so
// WORLD_UPDATE would never carry it. A part whose appearance changed is
// announced again in full, which a client applies to the part it already has.
std::vector<PartData> announcedPartLooks;

// The world's air. It belongs to the world, but the client never sees a .world
// file, so it is sent to every player as part of joining.
WorldEnvironment serverEnvironment;

// The driving game's car. A part named "Car" in the world is the vehicle: the
// server integrates it and every client follows the packets, so the car cannot
// be teleported by a modified client any more than a character can.
struct VehicleRuntime {
    int partIndex = -1;
    int driverId = -1;
    rblx::vehicle::State state;
    rblx::vehicle::Input input;
    float seatHeight = 1.8f;
};

VehicleRuntime vehicle;
const float MAX_PLAYER_COORDINATE = 10000.0f;
const float MAX_PLAYER_ROTATION_DEGREES = 100000.0f;
const int PLAYTIME_REPORT_INTERVAL_SECONDS = 60;
const int INSTANCE_HEARTBEAT_INTERVAL_SECONDS = 5;
std::chrono::steady_clock::time_point lastInstanceHeartbeatAt = std::chrono::steady_clock::time_point::min();
int lastReportedActivePlayers = -1;

// Heartbeats and playtime reports are HTTP calls to the website. Running them
// inline used to stall the whole game loop (and with it every player's packets)
// for as long as the request took, so they are handed to a background worker.
std::mutex telemetryMutex;
std::condition_variable telemetryWake;
std::deque<std::function<void()>> telemetryJobs;
const std::size_t MAX_PENDING_TELEMETRY_JOBS = 64;

void telemetryWorkerLoop() {
    for (;;) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(telemetryMutex);
            telemetryWake.wait(lock, [] { return !telemetryJobs.empty(); });
            job = std::move(telemetryJobs.front());
            telemetryJobs.pop_front();
        }

        try {
            job();
        } catch (...) {
            std::cerr << "Telemetry report failed" << std::endl;
        }
    }
}

void startTelemetryWorker() {
    std::thread(telemetryWorkerLoop).detach();
}

void enqueueTelemetry(std::function<void()> job) {
    if (!job) return;

    {
        std::lock_guard<std::mutex> lock(telemetryMutex);
        if (telemetryJobs.size() >= MAX_PENDING_TELEMETRY_JOBS) {
            // The website is not answering; drop reports instead of growing forever.
            static bool warned = false;
            if (!warned) {
                warned = true;
                std::cerr << "Telemetry queue is full; dropping reports until the website responds."
                          << std::endl;
            }
            return;
        }
        telemetryJobs.push_back(std::move(job));
    }

    telemetryWake.notify_one();
}

// Server World State
std::deque<Part> serverParts;
PhysicsWorld serverPhysicsWorld;

namespace {

// Everything the server prints goes to two places at once: the console window the
// website opens for it, and the log file the website reads to tell when the server
// is ready. A visible console cannot also inherit a redirected log handle, so the
// server has to keep the file itself.
class TeeStreamBuffer : public std::streambuf {
public:
    TeeStreamBuffer(std::streambuf* console, std::streambuf* file)
        : console_(console), file_(file) {}

protected:
    int_type overflow(int_type ch) override {
        if (ch == traits_type::eof()) {
            return traits_type::not_eof(ch);
        }
        const char c = traits_type::to_char_type(ch);
        const int_type toConsole = console_->sputc(c);
        const int_type toFile = file_->sputc(c);
        if (toConsole == traits_type::eof() || toFile == traits_type::eof()) {
            return traits_type::eof();
        }
        return ch;
    }

    std::streamsize xsputn(const char* text, std::streamsize count) override {
        const std::streamsize toConsole = console_->sputn(text, count);
        const std::streamsize toFile = file_->sputn(text, count);
        return toConsole < toFile ? toConsole : toFile;
    }

    int sync() override {
        const int consoleResult = console_->pubsync();
        const int fileResult = file_->pubsync();
        return (consoleResult == 0 && fileResult == 0) ? 0 : -1;
    }

private:
    std::streambuf* console_;
    std::streambuf* file_;
};

} // namespace

// Mirrors stdout and stderr into the log file the website tails. Installed once,
// after the arguments are parsed and before the first line of output.
void mirrorOutputToLog(const std::string& path) {
    static bool installed = false;
    if (installed) {
        return;
    }
    static std::ofstream logFile;
    logFile.open(path, std::ios::out | std::ios::trunc);
    if (!logFile.is_open()) {
        std::cerr << "Could not open log file " << path << std::endl;
        return;
    }
    installed = true;
    static TeeStreamBuffer consoleOut(std::cout.rdbuf(), logFile.rdbuf());
    static TeeStreamBuffer consoleErr(std::cerr.rdbuf(), logFile.rdbuf());
    std::cout.rdbuf(&consoleOut);
    std::cerr.rdbuf(&consoleErr);
}

// Puts std::cout and std::cerr back on the console when the server winds down. The
// iostreams flush themselves while the process exits, after function-local statics
// are already gone, so leaving them pointed at the log tee would be a use-after-free.
class OutputTeeRestorer {
public:
    ~OutputTeeRestorer() {
        std::cout.flush();
        std::cerr.flush();
        std::cout.rdbuf(consoleOut_);
        std::cerr.rdbuf(consoleErr_);
    }

private:
    // Read when this is constructed, which is before the tee is installed.
    std::streambuf* consoleOut_ = std::cout.rdbuf();
    std::streambuf* consoleErr_ = std::cerr.rdbuf();
};

void parseServerArguments(int argc, char** argv, int& serverPort, std::string& worldPath) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i] ? argv[i] : "";
        if (arg == "--port" && i + 1 < argc) {
            int parsed = std::atoi(argv[++i]);
            if (parsed > 0 && parsed <= 65535) {
                serverPort = parsed;
            }
        } else if (arg == "--world" && i + 1 < argc) {
            worldPath = argv[++i];
        } else if (arg == "--web" && i + 1 < argc) {
            webServerUrl = argv[++i];
        } else if (arg == "--instance" && i + 1 < argc) {
            gameInstanceId = argv[++i];
        } else if (arg == "--instance-token" && i + 1 < argc) {
            gameInstanceToken = argv[++i];
        } else if (arg == "--script" && i + 1 < argc) {
            scriptPath = argv[++i];
        } else if (arg == "--log" && i + 1 < argc) {
            serverLogPath = argv[++i];
        }
    }
}

bool isFiniteVec3(const rblxmath::vec3& value) {
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

bool isSafePlayerState(const PacketPlayerState& state) {
    if (!isFiniteVec3(state.position) || !std::isfinite(state.rotationY) || !std::isfinite(state.health)) {
        return false;
    }

    return std::abs(state.position.x) <= MAX_PLAYER_COORDINATE &&
           std::abs(state.position.y) <= MAX_PLAYER_COORDINATE &&
           std::abs(state.position.z) <= MAX_PLAYER_COORDINATE &&
           std::abs(state.rotationY) <= MAX_PLAYER_ROTATION_DEGREES;
}

float normalizeRotationDegrees(float value) {
    if (!std::isfinite(value)) {
        return 0.0f;
    }

    value = std::fmod(value, 360.0f);
    if (value > 180.0f) value -= 360.0f;
    if (value < -180.0f) value += 360.0f;
    return value;
}

float clamp01(float value) {
    if (!std::isfinite(value)) return 0.0f;
    return std::max(0.0f, std::min(1.0f, value));
}

bool parseAvatarColor(const std::string& avatarJson, const std::string& name, rblxmath::vec3& color) {
    size_t pos = avatarJson.find("\"" + name + "\":[");
    if (pos == std::string::npos) return false;

    pos = avatarJson.find("[", pos);
    size_t end = avatarJson.find("]", pos);
    if (pos == std::string::npos || end == std::string::npos || end <= pos) return false;

    std::string colorStr = avatarJson.substr(pos + 1, end - pos - 1);
    size_t comma1 = colorStr.find(",");
    size_t comma2 = colorStr.find(",", comma1 + 1);
    if (comma1 == std::string::npos || comma2 == std::string::npos) return false;

    try {
        color.r = clamp01(std::stof(colorStr.substr(0, comma1)));
        color.g = clamp01(std::stof(colorStr.substr(comma1 + 1, comma2 - comma1 - 1)));
        color.b = clamp01(std::stof(colorStr.substr(comma2 + 1)));
        return true;
    } catch (...) {
        return false;
    }
}

bool isKnownFaceId(const std::string& faceId) {
    return faceId == "classic" ||
           faceId == "happy" ||
           faceId == "surprised" ||
           faceId == "smirk" ||
           faceId == "wink";
}

std::string normalizeFaceId(const std::string& faceId) {
    return isKnownFaceId(faceId) ? faceId : DEFAULT_FACE_ID;
}

std::string parseAvatarFaceId(const std::string& avatarJson) {
    size_t pos = avatarJson.find("\"faceId\"");
    if (pos == std::string::npos) return DEFAULT_FACE_ID;

    pos = avatarJson.find(":", pos);
    if (pos == std::string::npos) return DEFAULT_FACE_ID;

    pos = avatarJson.find("\"", pos);
    if (pos == std::string::npos) return DEFAULT_FACE_ID;

    size_t end = avatarJson.find("\"", pos + 1);
    if (end == std::string::npos || end <= pos + 1) return DEFAULT_FACE_ID;

    return normalizeFaceId(avatarJson.substr(pos + 1, end - pos - 1));
}

// The equipped hat, or empty when the account wears none. An id the catalog does
// not have is dropped here rather than sent on, so a client is never asked to
// draw a hat it has no shape for.
std::string parseAvatarHatId(const std::string& avatarJson) {
    size_t pos = avatarJson.find("\"hatId\"");
    if (pos == std::string::npos) return std::string();

    pos = avatarJson.find(":", pos);
    if (pos == std::string::npos) return std::string();

    pos = avatarJson.find("\"", pos);
    if (pos == std::string::npos) return std::string();

    size_t end = avatarJson.find("\"", pos + 1);
    if (end == std::string::npos) return std::string();

    const std::string id = avatarJson.substr(pos + 1, end - pos - 1);
    return rblx::items::findAvatarItem(id) == nullptr ? std::string() : id;
}

void copyFixedString(char* destination, size_t destinationSize, const std::string& value) {
    if (!destination || destinationSize == 0) return;
    std::memset(destination, 0, destinationSize);
    const size_t bytesToCopy = std::min(destinationSize - 1, value.size());
    std::memcpy(destination, value.data(), bytesToCopy);
}

// Outcomes of the anti-cheat reports the game server sends the website. The
// report runs on the telemetry thread so a slow website never stalls a tick, so
// the answer comes back through here instead of as a return value.
struct ModerationOutcome {
    int playerId = -1;
    std::string message;
    bool restricted = false;
};

std::mutex moderationOutcomeMutex;
std::vector<ModerationOutcome> moderationOutcomes;

// A confirmed detection is not one stray correction: a laggy packet can cost
// one. Sixty is a second of sustained cheating at a client tick rate, and a fly
// hack reaches it in the same second, so a player has to keep doing it to earn a
// strike. Flying is unambiguous and needs less evidence than a fast step.
const int kCorrectionsPerReport = 60;
const int kFlightCorrectionsPerReport = 30;

// The recorder keeps plain numbers, so the server's vectors are handed over here
// rather than inside a header the website also reads.
rblx::replay::Point replayPoint(const rblxmath::vec3& value) {
    return rblx::replay::Point{value.x, value.y, value.z};
}

void reportCheatingToWebsite(int playerId, const std::string& username, const std::string& reason,
                             const std::string& proof, const std::string& expected,
                             const std::string& replay) {
    if (gameInstanceId.empty() || gameInstanceToken.empty() || username.empty()) {
        // A server started by hand has no launch token, so the website has no way
        // to know the report is ours. Skipping it beats sending an unsigned one
        // that could have come from any client.
        return;
    }

    const std::string instanceId = gameInstanceId;
    const std::string instanceToken = gameInstanceToken;
    const std::string webUrl = webServerUrl;
    enqueueTelemetry([playerId, username, reason, proof, expected, replay, instanceId, instanceToken,
                      webUrl]() {
        const CheatReportResult result = ServerAuth::reportCheating(
            instanceId, instanceToken, username, reason, proof, expected, replay, webUrl);
        if (!result.reported) {
            std::cerr << "Could not report cheating for player " << playerId << std::endl;
            return;
        }
        std::lock_guard<std::mutex> lock(moderationOutcomeMutex);
        moderationOutcomes.push_back({playerId, result.message, result.restricted});
    });
}

void reportPlayerPlaytime(ConnectedPlayer& player, std::chrono::steady_clock::time_point now, int playerId, bool force) {
    if (!player.active || player.userId <= 0 || player.authToken.empty()) {
        return;
    }

    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - player.lastPlaytimeReportedAt
    ).count();

    if (!force && elapsed < PLAYTIME_REPORT_INTERVAL_SECONDS) {
        return;
    }

    const int playtimeSeconds = force ? std::max(1, static_cast<int>(elapsed)) : static_cast<int>(elapsed);
    if (playtimeSeconds <= 0) {
        return;
    }

    const std::string token = player.authToken;
    const std::string webUrl = webServerUrl;
    enqueueTelemetry([token, playtimeSeconds, playerId, webUrl]() {
        if (!ServerAuth::recordPlaytime(token, playtimeSeconds, webUrl)) {
            std::cerr << "Could not record playtime for player " << playerId << std::endl;
        }
    });
    player.lastPlaytimeReportedAt = now;
}

int activePlayerCount() {
    int count = 0;
    for (const auto& [id, player] : players) {
        if (player.active) {
            count++;
        }
    }
    return count;
}

void reportInstanceHeartbeat(bool force) {
    if (gameInstanceId.empty() || gameInstanceToken.empty()) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    const int playerCount = activePlayerCount();
    const bool countChanged = playerCount != lastReportedActivePlayers;
    const bool intervalElapsed = lastInstanceHeartbeatAt == std::chrono::steady_clock::time_point::min() ||
        now - lastInstanceHeartbeatAt >= std::chrono::seconds(INSTANCE_HEARTBEAT_INTERVAL_SECONDS);

    if (!force && !countChanged && !intervalElapsed) {
        return;
    }

    const std::string instanceId = gameInstanceId;
    const std::string instanceToken = gameInstanceToken;
    const std::string webUrl = webServerUrl;
    enqueueTelemetry([instanceId, instanceToken, playerCount, webUrl]() {
        if (!ServerAuth::reportInstanceHeartbeat(instanceId, instanceToken, playerCount, webUrl)) {
            std::cerr << "Could not report heartbeat for game instance " << instanceId << std::endl;
        }
    });

    lastReportedActivePlayers = playerCount;
    lastInstanceHeartbeatAt = now;
}

void broadcastPacket(const std::vector<char>& packet, int excludeId = -1, bool reliable = false) {
    if (packet.empty()) return;

    for (auto& [id, player] : players) {
        if (id == excludeId) continue;
        if (!player.active) continue;
        if (!sendToPlayer(player, packet, reliable)) {
            std::cerr << "Could not queue packet for player " << id << std::endl;
        }
    }
}

std::vector<char> buildPlayerListPacket() {
    std::vector<PlayerInfo> playerInfoList;
    for (auto& [id, player] : players) {
        if (player.active) {
            PlayerInfo info;
            std::memset(&info, 0, sizeof(info));
            info.playerId = id;
            copyFixedString(info.username, sizeof(info.username), PacketProtocol::fixedString(player.username, sizeof(player.username)));
            info.headColor[0] = player.headColor.r;
            info.headColor[1] = player.headColor.g;
            info.headColor[2] = player.headColor.b;
            info.torsoColor[0] = player.torsoColor.r;
            info.torsoColor[1] = player.torsoColor.g;
            info.torsoColor[2] = player.torsoColor.b;
            info.leftArmColor[0] = player.leftArmColor.r;
            info.leftArmColor[1] = player.leftArmColor.g;
            info.leftArmColor[2] = player.leftArmColor.b;
            info.rightArmColor[0] = player.rightArmColor.r;
            info.rightArmColor[1] = player.rightArmColor.g;
            info.rightArmColor[2] = player.rightArmColor.b;
            info.leftLegColor[0] = player.leftLegColor.r;
            info.leftLegColor[1] = player.leftLegColor.g;
            info.leftLegColor[2] = player.leftLegColor.b;
            info.rightLegColor[0] = player.rightLegColor.r;
            info.rightLegColor[1] = player.rightLegColor.g;
            info.rightLegColor[2] = player.rightLegColor.b;
            copyFixedString(info.faceId, sizeof(info.faceId), normalizeFaceId(player.faceId));
            copyFixedString(info.hatId, sizeof(info.hatId), player.hatId);
            playerInfoList.push_back(info);
        }
    }

    uint32_t playerCount = (uint32_t)playerInfoList.size();
    uint32_t payloadSize = sizeof(uint32_t) + static_cast<uint32_t>(playerInfoList.size() * sizeof(PlayerInfo));

    std::vector<char> payload(payloadSize);
    std::memcpy(payload.data(), &playerCount, sizeof(uint32_t));

    if (!playerInfoList.empty()) {
        std::memcpy(payload.data() + sizeof(uint32_t),
                    playerInfoList.data(),
                    playerInfoList.size() * sizeof(PlayerInfo));
    }

    return PacketProtocol::buildPacket(PacketType::PLAYER_LIST, payload.data(), payloadSize);
}

void sendPlayerList(ConnectedPlayer& player) {
    sendToPlayer(player, buildPlayerListPacket(), true);
}

void broadcastPlayerList(int excludeId = -1) {
    broadcastPacket(buildPlayerListPacket(), excludeId, true);
}

void updatePlayerShadowBody(ConnectedPlayer& player) {
    if (!player.active) return;

    if (!player.shadowBody) {
        // Create shadow body if not exists
        rblxphysics::Vector3 pos(player.state.position.x, player.state.position.y, player.state.position.z);
        rblxphysics::Quaternion rot = rblxphysics::Quaternion::identity();
        rblxphysics::Transform transform(pos, rot);

        player.shadowBody = serverPhysicsWorld.world->createRigidBody(transform);
        player.shadowBody->setType(rblxphysics::BodyType::KINEMATIC); // Moved by code (player input)

        // Add a capsule or box shape for collision
        rblxphysics::Vector3 halfExtents(1.0f, 2.5f, 1.0f); // Roughly character size
        rblxphysics::BoxShape* shape = serverPhysicsWorld.physicsCommon.createBoxShape(halfExtents);
        player.shadowBody->addCollider(shape, rblxphysics::Transform::identity());

        // Set friction to 0 like character
        if (player.shadowBody->getNbColliders() > 0) {
            player.shadowBody->getCollider(0)->getMaterial().setFrictionCoefficient(0.0f);
        }
    }

    // Move shadow body smoothly using velocity (KINEMATIC bodies can push DYNAMIC parts when moving)
    rblxphysics::Transform currentTransform = player.shadowBody->getTransform();
    rblxphysics::Vector3 currentPos = currentTransform.getPosition();
    rblxphysics::Vector3 targetPos(player.state.position.x, player.state.position.y, player.state.position.z);

    rblxphysics::Vector3 diff = targetPos - currentPos;
    float distance = diff.length();

    // Set velocity to move towards target (KINEMATIC bodies with velocity can push parts)
    if (distance > 0.01f) {
        // Calculate velocity needed to reach target in one physics step (1/60s)
        float speed = distance * 60.0f; // Move distance in one frame
        rblxphysics::Vector3 direction = diff.getUnit();
        rblxphysics::Vector3 velocity = direction * speed;
        player.shadowBody->setLinearVelocity(velocity);
    } else {
        // Close enough, just set position directly
        player.shadowBody->setLinearVelocity(rblxphysics::Vector3(0, 0, 0));
        rblxphysics::Transform newTransform(targetPos, currentTransform.getOrientation());
        player.shadowBody->setTransform(newTransform);
    }

    // Update rotation smoothly
    rblxphysics::Quaternion currentRot = currentTransform.getOrientation();
    rblxphysics::Quaternion targetRot = rblxphysics::Quaternion::fromEulerAngles(0, rblxmath::radians(player.state.rotationY), 0);
    rblxphysics::Quaternion newRot = rblxphysics::Quaternion::slerp(currentRot, targetRot, 0.3f);
    rblxphysics::Transform newTransform(currentPos, newRot);
    player.shadowBody->setTransform(newTransform);

    // Zero angular velocity (we control rotation directly)
    player.shadowBody->setAngularVelocity(rblxphysics::Vector3(0, 0, 0));
}

void teleportPlayer(const std::string& username, const rblxmath::vec3& position) {
    for (auto& entry : players) {
        ConnectedPlayer& player = entry.second;
        if (!player.active) continue;
        const std::string name =
            PacketProtocol::fixedString(player.username, sizeof(player.username));
        if (name != username) continue;

        player.state.position = position;
        if (player.shadowBody) {
            rblxphysics::Transform transform = player.shadowBody->getTransform();
            transform.setPosition(rblxphysics::Vector3(position.x, position.y, position.z));
            player.shadowBody->setTransform(transform);
            player.shadowBody->setLinearVelocity(rblxphysics::Vector3(0.0f, 0.0f, 0.0f));
            player.shadowBody->setAngularVelocity(rblxphysics::Vector3(0.0f, 0.0f, 0.0f));
        }
        // A script moved this player: that position is the server's own, so it
        // becomes the baseline the next packet is measured against.
        player.movement.teleport(position, steadyMilliseconds());
        player.replay.reset();
        sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::TELEPORT, player.state), true);
        return;
    }
}

// One part as it goes over the wire. The join snapshot fills the same structure
// in place; a part a script builds later is described here.
PartData makePartData(const Part& part) {
    PartData data;
    std::memset(&data, 0, sizeof(data));
    data.shape = static_cast<int>(part.shape);
    copyFixedString(data.name, sizeof(data.name), part.name);
    data.parentIndex = part.parentIndex;

    data.flags = 0;
    if (part.isFolder) data.flags |= 1;
    if (part.isCamera) data.flags |= 2;
    if (part.isSpawn) data.flags |= 4;
    if (part.deleted) data.flags |= 8;

    if (part.physicsBody) {
        rblxphysics::RigidBody* body = static_cast<rblxphysics::RigidBody*>(part.physicsBody);
        const rblxphysics::Transform t = body->getTransform();
        const rblxphysics::Vector3 pos = t.getPosition();
        const rblxphysics::Quaternion rot = t.getOrientation();
        data.position = rblxmath::vec3(pos.x, pos.y, pos.z);

        const rblxmath::quat q(rot.w, rot.x, rot.y, rot.z);
        data.rotation = rblxmath::degrees(rblxmath::eulerAngles(q));
    } else {
        data.position = part.position;
        data.rotation = part.rotation;
    }

    data.size = part.size;
    data.color = part.color;
    data.transparency = part.transparency;
    data.reflectance = part.reflectance;

    data.physicsFlags = 0;
    if (part.anchored) data.physicsFlags |= 1;
    if (part.canCollide) data.physicsFlags |= 2;

    data.mass = part.mass;
    data.material = static_cast<uint8_t>(part.material);
    copyFixedString(data.texture, sizeof(data.texture), part.texture);
    return data;
}

// The join snapshot is the world as it was when a player arrived. A script can
// build the world after that and delete it again as the player moves, and none
// of that is in the snapshot: the rooms ahead have to be announced or a client
// walks into parts it never received, and the rooms behind have to be announced
// gone or a client keeps colliding with a hotel that no longer exists.
// Position and rotation travel in WORLD_UPDATE, one packet for every part that
// moved, so comparing them here would re-announce the moving car every tick.
// Everything else in the part is what a renderer draws, and that is what a
// script changes when it repaints something.
bool samePartLook(const PartData& a, const PartData& b) {
    PartData left = a;
    PartData right = b;
    left.position = rblxmath::vec3(0.0f);
    right.position = rblxmath::vec3(0.0f);
    left.rotation = rblxmath::vec3(0.0f);
    right.rotation = rblxmath::vec3(0.0f);
    return std::memcmp(&left, &right, sizeof(PartData)) == 0;
}

void broadcastPartChanges() {
    while (announcedPartRemovals.size() < serverParts.size()) {
        announcedPartRemovals.push_back(0);
    }
    while (announcedPartLooks.size() < announcedPartCount) {
        announcedPartLooks.emplace_back();
    }

    for (std::size_t i = 0; i < serverParts.size(); ++i) {
        if (!serverParts[i].deleted || announcedPartRemovals[i]) continue;
        announcedPartRemovals[i] = 1;
        PacketPartRemove remove = { static_cast<int>(i) };
        broadcastPacket(PacketProtocol::buildStructPacket(PacketType::PART_REMOVE, remove), -1, true);
    }

    for (std::size_t i = announcedPartCount; i < serverParts.size(); ++i) {
        PacketPartState state;
        std::memset(&state, 0, sizeof(state));
        state.partIndex = static_cast<int>(i);
        state.data = makePartData(serverParts[i]);
        broadcastPacket(PacketProtocol::buildStructPacket(PacketType::PART_STATE, state), -1, true);
        announcedPartLooks.push_back(state.data);
        lastBroadcastPositions[static_cast<int>(i)] = serverParts[i].position;
    }

    // Parts the clients already have can still change how they look. The Doors
    // hotel turns its lobby lights red a few seconds before the entity comes,
    // and that warning has to be visible from inside the hotel, so a part whose
    // appearance no longer matches what was announced is announced again.
    for (std::size_t i = 0; i < announcedPartCount && i < serverParts.size(); ++i) {
        if (serverParts[i].deleted) continue;
        const PartData look = makePartData(serverParts[i]);
        if (samePartLook(look, announcedPartLooks[i])) continue;

        PacketPartState state;
        std::memset(&state, 0, sizeof(state));
        state.partIndex = static_cast<int>(i);
        state.data = look;
        broadcastPacket(PacketProtocol::buildStructPacket(PacketType::PART_STATE, state), -1, true);
        announcedPartLooks[i] = look;
    }

    announcedPartCount = serverParts.size();
}

void broadcastWorldUpdate() {
    // Dynamic parts are sent every tick. Anchored parts are sent only when a
    // script moved them, which keeps scripted platforms in sync without
    // re-sending the whole world.
    std::vector<PartUpdate> updates;
    for (int i = 0; i < static_cast<int>(serverParts.size()); ++i) {
        Part& p = serverParts[i];
        if (p.deleted || p.isFolder || !p.physicsBody) continue;

        rblxphysics::RigidBody* body = static_cast<rblxphysics::RigidBody*>(p.physicsBody);
        const bool dynamic = body->getType() == rblxphysics::BodyType::DYNAMIC;

        rblxphysics::Transform t = body->getTransform();
        const rblxphysics::Vector3 pos = t.getPosition();
        const rblxphysics::Quaternion rot = t.getOrientation();
        const rblxmath::vec3 newPosition(pos.x, pos.y, pos.z);

        const auto previous = lastBroadcastPositions.find(static_cast<int>(i));
        const bool moved = previous == lastBroadcastPositions.end() ||
                           rblxmath::length(previous->second - newPosition) > 0.001f;
        if (!dynamic && !moved) continue;

        p.position = newPosition;

        PartUpdate up;
        up.partIndex = i;
        up.position = newPosition;

        // Convert quaternion to euler for network transmission
        rblxmath::quat q(rot.w, rot.x, rot.y, rot.z);
        up.rotation = rblxmath::degrees(rblxmath::eulerAngles(q));

        updates.push_back(up);
        lastBroadcastPositions[static_cast<int>(i)] = newPosition;
    }

    if (updates.empty()) return;

    if (updates.size() > PacketProtocol::MAX_WORLD_UPDATES) {
        updates.resize(PacketProtocol::MAX_WORLD_UPDATES);
    }

    uint32_t payloadSize = static_cast<uint32_t>(updates.size() * sizeof(PartUpdate));
    broadcastPacket(PacketProtocol::buildPacket(PacketType::WORLD_UPDATE, updates.data(), payloadSize));
}

// The world's first SpawnLocation, lifted by the same offset the client uses when
// it creates a character. A world without one keeps the old default so an empty
// test world still starts.
rblxmath::vec3 findSpawnPosition() {
    for (const Part& part : serverParts) {
        if (part.deleted || !part.isSpawn) continue;
        return part.position + rblxmath::vec3(0.0f, 5.0f, 0.0f);
    }
    return rblxmath::vec3(0.0f, 10.0f, 0.0f);
}

// The name scripts and chat use for a player, matching what setPlayerSource
// reports so a script can address the player it saw in Touched.
std::string scriptPlayerName(const ConnectedPlayer& player, int id) {
    std::string name = PacketProtocol::fixedString(player.username, sizeof(player.username));
    if (name.empty()) {
        name = "Player" + std::to_string(id);
    }
    return name;
}

void broadcastPlayerHealth(int id, const rblx::health::State& health, uint64_t nowMs) {
    PacketPlayerHealth packet;
    packet.playerId = id;
    packet.health = health.health;
    packet.maxHealth = health.maxHealth;
    packet.respawnSeconds = static_cast<uint16_t>(
        std::max(0, std::min(255, health.respawnSecondsRemaining(nowMs))));
    broadcastPacket(PacketProtocol::buildStructPacket(PacketType::PLAYER_HEALTH, packet), -1, true);
}

// The one place damage is applied. A fall, a script and a reset request all end
// up here, so they share the same death and respawn handling.
void damagePlayer(ConnectedPlayer& player, int id, float amount, uint64_t nowMs,
                  const std::string& reason) {
    if (!player.active || player.health.isDead()) {
        return;
    }

    const bool killed = player.health.damage(amount, nowMs);
    player.state.health = player.health.health;
    broadcastPlayerHealth(id, player.health, nowMs);

    if (!killed) {
        return;
    }

    const std::string name = scriptPlayerName(player, id);
    std::cout << "Player " << id << " (" << name << ") died: " << reason << std::endl;

    // Say why the character dropped instead of letting it simply disappear for
    // everyone else in the game.
    PacketChat notice;
    std::memset(&notice, 0, sizeof(notice));
    copyFixedString(notice.username, sizeof(notice.username), "Server");
    copyFixedString(notice.message, sizeof(notice.message), name + " died.");
    broadcastPacket(PacketProtocol::buildStructPacket(PacketType::CHAT, notice), -1, true);
}

// Puts a player back at the spawn point the server holds, and tells every client
// so the character is in the same place for its owner and for the players
// watching it.
void respawnPlayer(ConnectedPlayer& player, int id, uint64_t nowMs) {
    player.health.reset();
    player.state.position = player.spawnPosition;
    player.state.health = player.health.health;

    if (player.shadowBody) {
        rblxphysics::Transform transform = player.shadowBody->getTransform();
        transform.setPosition(rblxphysics::Vector3(player.spawnPosition.x, player.spawnPosition.y,
                                            player.spawnPosition.z));
        player.shadowBody->setTransform(transform);
        player.shadowBody->setLinearVelocity(rblxphysics::Vector3(0.0f, 0.0f, 0.0f));
        player.shadowBody->setAngularVelocity(rblxphysics::Vector3(0.0f, 0.0f, 0.0f));
    }

    player.movement.teleport(player.spawnPosition, nowMs);

    player.replay.reset();
    broadcastPacket(PacketProtocol::buildStructPacket(PacketType::TELEPORT, player.state), -1, true);
    broadcastPlayerHealth(id, player.health, nowMs);
    std::cout << "Player " << id << " respawned at " << player.spawnPosition.x << ", "
              << player.spawnPosition.y << ", " << player.spawnPosition.z << std::endl;
}

Part* vehiclePart() {
    if (vehicle.partIndex < 0 || vehicle.partIndex >= static_cast<int>(serverParts.size())) {
        return nullptr;
    }
    Part& part = serverParts[vehicle.partIndex];
    return part.deleted ? nullptr : &part;
}

void broadcastVehicleState() {
    if (vehicle.partIndex < 0) return;

    PacketVehicleState packet;
    packet.partIndex = vehicle.partIndex;
    packet.driverId = vehicle.driverId;
    packet.position = vehicle.state.position;
    packet.yaw = vehicle.state.yawDegrees;
    packet.speed = vehicle.state.speed;
    broadcastPacket(PacketProtocol::buildStructPacket(PacketType::VEHICLE_STATE, packet));
}

// A part named "Car" is the vehicle, which leaves the world format alone and
// matches how scripts already find SpawnPoint, Lava and the rest by name.
void prepareVehicle() {
    vehicle.partIndex = -1;
    vehicle.driverId = -1;

    for (int i = 0; i < static_cast<int>(serverParts.size()); ++i) {
        Part& part = serverParts[i];
        if (part.deleted || part.isFolder || part.name != "Car") continue;

        vehicle.partIndex = i;
        vehicle.state.position = part.position;
        vehicle.state.yawDegrees = part.rotation.y;
        vehicle.state.speed = 0.0f;
        vehicle.input.set(0.0f, 0.0f);

        // The server moves the body itself along the driving model, so it must
        // not be a dynamic body falling or being shoved while someone drives.
        if (part.physicsBody) {
            static_cast<rblxphysics::RigidBody*>(part.physicsBody)->setType(rblxphysics::BodyType::KINEMATIC);
        }

        std::cout << "Driving: car is part " << i << " at " << part.position.x << ", "
                  << part.position.y << ", " << part.position.z << std::endl;
        return;
    }
}

void seatPlayer(ConnectedPlayer& player, int id, uint64_t nowMs) {
    vehicle.driverId = id;
    vehicle.input.set(0.0f, 0.0f);
    vehicle.state.speed = 0.0f;

    player.state.position = vehicle.state.seatPosition(vehicle.seatHeight);
    player.state.rotationY = vehicle.state.yawDegrees;
    player.movement.teleport(player.state.position, nowMs);
    player.replay.reset();
    sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::TELEPORT, player.state), true);
    broadcastVehicleState();

    std::cout << "Player " << id << " (" << scriptPlayerName(player, id) << ") got into the car"
              << std::endl;
}

// Puts the player on the ground beside the car. The movement guard is reset to
// that spot, so the first walking packet is not "corrected" back into the car.
void unseatPlayer(ConnectedPlayer& player, int id, uint64_t nowMs, bool placeBeside) {
    vehicle.driverId = -1;
    vehicle.state.speed = 0.0f;
    vehicle.input.set(0.0f, 0.0f);

    if (placeBeside) {
        const float yaw = rblxmath::radians(vehicle.state.yawDegrees);
        const rblxmath::vec3 right(-std::sin(yaw), 0.0f, std::cos(yaw));
        player.state.position = vehicle.state.position + right * 3.0f + rblxmath::vec3(0.0f, 2.0f, 0.0f);
        player.movement.teleport(player.state.position, nowMs);
        player.replay.reset();
        sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::TELEPORT, player.state), true);
    }

    broadcastVehicleState();
    std::cout << "Player " << id << " left the car" << std::endl;
}

// The boxes the car has to stop at. Read every tick rather than cached, because
// a script can move, unanchor or delete a part while somebody is driving.
std::vector<rblx::vehicle::Obstacle> vehicleObstacles() {
    std::vector<rblx::vehicle::Obstacle> obstacles;
    for (int i = 0; i < static_cast<int>(serverParts.size()); ++i) {
        const Part& part = serverParts[i];
        if (i == vehicle.partIndex || part.deleted || part.isFolder || !part.canCollide) {
            continue;
        }
        // A yaw-rotated box covers more ground than its own footprint, so the
        // obstacle is the axis-aligned box around it: generous at the corners of
        // a diagonal wall, and never small enough to let the car clip through.
        const float yaw = rblxmath::radians(part.rotation.y);
        const float cosine = std::abs(std::cos(yaw));
        const float sine = std::abs(std::sin(yaw));
        const rblxmath::vec3 half(0.5f * (cosine * part.size.x + sine * part.size.z),
                             0.5f * part.size.y,
                             0.5f * (sine * part.size.x + cosine * part.size.z));
        rblx::vehicle::Obstacle obstacle;
        obstacle.min = part.position - half;
        obstacle.max = part.position + half;
        obstacles.push_back(obstacle);
    }
    return obstacles;
}

// Half the car part's size, so the box that collides is the car the player sees.
rblxmath::vec3 vehicleHalfExtents() {
    const Part* part = vehiclePart();
    if (part == nullptr) {
        return rblxmath::vec3(5.0f, 1.0f, 3.0f);
    }
    return rblxmath::vec3(std::max(0.5f, part->size.x) * 0.5f,
                     std::max(0.5f, part->size.y) * 0.5f,
                     std::max(0.5f, part->size.z) * 0.5f);
}

// One tick of the car. The driver's character rides with it, so every client
// sees the same seat position and the movement guard stays in step.
void updateVehicle(float dt, uint64_t nowMs) {
    if (vehicle.partIndex < 0) return;

    if (vehicle.driverId != -1) {
        const auto driver = players.find(vehicle.driverId);
        if (driver == players.end() || !driver->second.active || driver->second.health.isDead()) {
            vehicle.driverId = -1;
            vehicle.state.speed = 0.0f;
            vehicle.input.set(0.0f, 0.0f);
            broadcastVehicleState();
        }
    }

    if (vehicle.driverId == -1 || dt <= 0.0f) return;

    rblx::vehicle::stepColliding(vehicle.state, vehicle.input, dt, vehicleObstacles(),
                                 vehicleHalfExtents());

    Part* part = vehiclePart();
    if (part == nullptr) {
        vehicle.driverId = -1;
        broadcastVehicleState();
        return;
    }

    part->position = vehicle.state.position;
    part->rotation.y = vehicle.state.yawDegrees;
    if (part->physicsBody) {
        auto* body = static_cast<rblxphysics::RigidBody*>(part->physicsBody);
        body->setTransform(rblxphysics::Transform(
            rblxphysics::Vector3(vehicle.state.position.x, vehicle.state.position.y, vehicle.state.position.z),
            rblxphysics::Quaternion::fromEulerAngles(0.0f, rblxmath::radians(vehicle.state.yawDegrees), 0.0f)));
        body->setLinearVelocity(rblxphysics::Vector3(0.0f, 0.0f, 0.0f));
        body->setAngularVelocity(rblxphysics::Vector3(0.0f, 0.0f, 0.0f));
    }

    ConnectedPlayer& driver = players[vehicle.driverId];
    driver.state.position = vehicle.state.seatPosition(vehicle.seatHeight);
    driver.state.rotationY = vehicle.state.yawDegrees;
    driver.state.isWalking = false;
    driver.state.isJumping = false;
    driver.state.health = driver.health.health;
    driver.movement.teleport(driver.state.position, nowMs);
    driver.replay.reset();

    broadcastPacket(PacketProtocol::buildStructPacket(PacketType::PLAYER_STATE, driver.state),
                    vehicle.driverId);
    broadcastVehicleState();
}

static void sendWorldEnvironment(ConnectedPlayer& player) {
    PacketWorldEnvironment environment;
    std::memset(&environment, 0, sizeof(environment));
    environment.fogColor = serverEnvironment.fogColor;
    environment.fogDensity = serverEnvironment.fogDensity;
    sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::WORLD_ENVIRONMENT, environment), true);
}

static int runServer(int argc, char** argv) {
    // The website redirects this output into game-server-<port>.log. Without this
    // the C runtime buffers it, so the file stays empty until the process exits
    // and a join that failed has nothing to explain itself with.
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // Constructed before any log tee exists, so it remembers the real console.
    OutputTeeRestorer restoreOutput;

    int serverPort = 7777;
    std::string worldPath = "ServerWorld.world";
    parseServerArguments(argc, argv, serverPort, worldPath);
    if (!serverLogPath.empty()) {
        mirrorOutputToLog(serverLogPath);
    }

    // Print each start-up step as it happens, so a crash still leaves the log
    // pointing at the step that failed instead of an empty file.
    std::cout << "RBLX Clone Server starting on port " << serverPort
              << " using " << worldPath
              << " with web auth " << webServerUrl << std::endl;

    // Initialize random seed for guest usernames
    srand((unsigned int)time(NULL));

    // Initialize Physics World
    std::cout << "Initialising physics..." << std::endl;
    serverPhysicsWorld.init();
    std::cout << "Reading world " << worldPath << std::endl;

    if (!Network::init()) {
        std::cerr << "Failed to init winsock" << std::endl;
        return 1;
    }

    gameSocket = Network::createUdpSocket();
    if (gameSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // A world snapshot arrives as one burst of fragments; without room to hold
    // it the kernel drops the tail and the join never finishes.
    Network::setIoBufferSize(gameSocket, 4 << 20);

    // The socket is bound further down, once the world is loaded. Binding here
    // would make the port look ready to the website while this process is still
    // building physics bodies and cannot answer anything.

    // Load Server World
    WorldLoader::loadWorld(worldPath, serverParts, serverPhysicsWorld, &serverEnvironment);
    std::cout << "Loaded ServerWorld with " << serverParts.size() << " parts";
    if (serverEnvironment.fogDensity > 0.0f) {
        std::cout << " and fog at density " << serverEnvironment.fogDensity;
    }
    std::cout << "." << std::endl;

    if (!scriptPath.empty()) {
        std::ifstream scriptFile(scriptPath, std::ios::binary);
        if (!scriptFile.is_open()) {
            std::cerr << "Could not open script " << scriptPath << std::endl;
        } else {
            std::stringstream scriptBuffer;
            scriptBuffer << scriptFile.rdbuf();

            gameScriptInterpreter = std::make_unique<rblx::script::Interpreter>();
            gameScriptInterpreter->setPrintHandler([](const std::string& message, bool isError) {
                std::cout << (isError ? "[script error] " : "[script] ") << message << std::endl;
            });
            gameScriptRuntime = std::make_unique<rblx::worldscript::WorldRuntime>(
                serverParts, &serverPhysicsWorld, *gameScriptInterpreter);
            gameScriptRuntime->setPlayerSource([]() {
                std::vector<rblx::worldscript::ScriptPlayer> snapshot;
                for (const auto& entry : players) {
                    const ConnectedPlayer& player = entry.second;
                    if (!player.active) continue;
                    rblx::worldscript::ScriptPlayer scriptPlayer;
                    scriptPlayer.name = scriptPlayerName(player, entry.first);
                    scriptPlayer.position = player.state.position;
                    snapshot.push_back(scriptPlayer);
                }
                return snapshot;
            });
            gameScriptRuntime->setTeleportHandler([](const std::string& name,
                                                     const rblxmath::vec3& position) {
                teleportPlayer(name, position);
            });
            gameScriptRuntime->setDamageHandler([](const std::string& name, float amount) {
                for (auto& entry : players) {
                    ConnectedPlayer& player = entry.second;
                    if (!player.active) continue;
                    if (scriptPlayerName(player, entry.first) != name) continue;
                    damagePlayer(player, entry.first, amount, steadyMilliseconds(),
                                 "script damage");
                    return;
                }
            });
            gameScriptRuntime->setSoundHandler([](const std::string& sound,
                                                  const rblxmath::vec3& position, float volume,
                                                  bool spatial) {
                if (pendingSounds.size() >= kMaxSoundsPerTick) {
                    return;
                }
                PacketSound packet;
                std::memset(&packet, 0, sizeof(packet));
                copyFixedString(packet.name, sizeof(packet.name), sound);
                packet.position = position;
                packet.volume = std::clamp(volume, 0.0f, 1.0f);
                packet.spatial = spatial ? 1 : 0;
                pendingSounds.push_back(packet);
            });

            std::string scriptError;
            if (!gameScriptRuntime->runScript(scriptBuffer.str(), scriptPath, scriptError)) {
                std::cerr << "Script error: " << scriptError << std::endl;
            } else {
                std::cout << "Loaded script " << scriptPath << std::endl;
            }
        }
    }

    // Everything the world file and the script's opening build put in place is
    // in the join snapshot a client receives, so only what comes later has to be
    // announced part by part.
    announcedPartCount = serverParts.size();
    announcedPartLooks.clear();
    for (const Part& part : serverParts) {
        announcedPartLooks.push_back(makePartData(part));
    }

    prepareVehicle();

    // Bind only now that the world and its scripts are in place. The website
    // starts a game server, waits for this port to become busy and then launches
    // the client, so a port that was bound before the first receive loop made a
    // fresh Play join a server that could not answer yet: the client showed the
    // sky-blue clear colour until its join deadline and then dropped to the menu.
    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<u_short>(serverPort));

    if (bind(gameSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }

    if (!Network::setNonBlocking(gameSocket)) {
        std::cerr << "Could not put the socket in non-blocking mode" << std::endl;
        return 1;
    }

    std::cout << "RBLX Clone Server listening on port " << serverPort << " (UDP)"
              << " using " << worldPath
              << " with web auth " << webServerUrl << std::endl;

    startTelemetryWorker();
    reportInstanceHeartbeat(true);

    auto lastTime = std::chrono::high_resolution_clock::now();
    float physicsAccumulator = 0.0f;
    float networkAccumulator = 0.0f;

    while (true) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Never let one long frame queue up hundreds of physics steps: the
        // catch-up burst would freeze the server for seconds right afterwards.
        if (deltaTime > 0.25f) {
            deltaTime = 0.25f;
        }

        physicsAccumulator += deltaTime;
        networkAccumulator += deltaTime;

        // Physics Step (60Hz) - with safety checks
        while (physicsAccumulator >= 1.0f/60.0f) {
            try {
                serverPhysicsWorld.update(1.0f/60.0f);
            } catch (...) {
                std::cerr << "ERROR: Physics update crashed, continuing..." << std::endl;
            }
            physicsAccumulator -= 1.0f/60.0f;
        }

        // Update all player shadow bodies every frame (so they can push parts)
        for (auto& [id, player] : players) {
            if (player.active) {
                updatePlayerShadowBody(player);
                reportPlayerPlaytime(player, std::chrono::steady_clock::now(), id, false);
            }
        }
        // Deaths and respawns are decided here, not in the client that owns the
        // character: the fall below the world used to be handled locally, which a
        // modified client could simply not run.
        for (auto& [id, player] : players) {
            if (!player.active) continue;
            const uint64_t healthNowMs = steadyMilliseconds();

            if (player.health.isDead()) {
                if (player.health.update(healthNowMs)) {
                    respawnPlayer(player, id, healthNowMs);
                }
                continue;
            }

            if (player.state.position.y < rblx::health::kFallDeathHeight) {
                damagePlayer(player, id, rblx::health::kMaxHealth, healthNowMs,
                             "falling out of the world");
            }
        }

        reportInstanceHeartbeat(false);

        // The car moves before the 60Hz block so its new position goes out in
        // the same world update as the rest of the moving parts.
        updateVehicle(deltaTime, steadyMilliseconds());

        // Scripts tick with the network rate so gameplay logic runs at 60Hz
        // instead of once per loop iteration.
        if (networkAccumulator >= 1.0f/60.0f) {
            if (gameScriptRuntime) {
                gameScriptRuntime->heartbeat(networkAccumulator);
            }
            // What the script built or destroyed during that tick goes out before
            // the positions of the parts that already existed.
            broadcastPartChanges();
            broadcastWorldUpdate();
            // The tick's sounds. They are cosmetic, so they travel unreliably and
            // a lost one is simply not heard.
            for (const PacketSound& sound : pendingSounds) {
                broadcastPacket(PacketProtocol::buildStructPacket(PacketType::SOUND, sound), -1,
                                false);
            }
            pendingSounds.clear();
            networkAccumulator = 0.0f;
        }

        // 1. Read every waiting datagram and hand it to the peer it came from.
        // There are no connections to accept: an unknown source starts a peer
        // and silence is what ends one.
        std::map<int, std::vector<std::vector<char>>> tickMessages;
        const uint64_t nowMs = steadyMilliseconds();
        std::vector<int> disconnectedIds;

        char datagram[2048];
        for (int datagramCount = 0; datagramCount < kDatagramsPerTick; ++datagramCount) {
            sockaddr_in fromAddr;
            std::memset(&fromAddr, 0, sizeof(fromAddr));
            SocketLength fromLength = sizeof(fromAddr);
            const int received = recvfrom(gameSocket, datagram, sizeof(datagram), 0,
                                          reinterpret_cast<sockaddr*>(&fromAddr), &fromLength);
            if (received == SOCKET_ERROR) {
                const int error = WSAGetLastError();
                if (error == WSAEWOULDBLOCK) break;
                // Windows reports an ICMP port-unreachable from a peer that is
                // already gone as WSAECONNRESET; that is not a socket failure.
                if (error == WSAECONNRESET) continue;
                std::cerr << "recvfrom failed: " << error << std::endl;
                break;
            }
            if (received <= 0) break;
            if (static_cast<size_t>(received) < sizeof(rblx::net::UdpHeader)) continue;

            rblx::net::UdpHeader header;
            std::memcpy(&header, datagram, sizeof(header));
            if (header.magic0 != rblx::net::kMagic0 || header.magic1 != rblx::net::kMagic1 ||
                header.version != rblx::net::kVersion) {
                continue;
            }

            ConnectedPlayer* player = findPlayerByAddress(fromAddr);
            if (!player) {
                if (players.size() >= PacketProtocol::MAX_PLAYERS) continue;

                ConnectedPlayer fresh;
                fresh.session.reset(header.connectionId);
                fresh.address = fromAddr;
                fresh.lastReceiveAtMs = nowMs;
                fresh.id = nextPlayerId++;
                fresh.active = false;
                fresh.userId = -1;
                fresh.shadowBody = nullptr;
                std::memset(fresh.username, 0, sizeof(fresh.username));

                const int newId = fresh.id;
                players[newId] = std::move(fresh);
                peerAddresses[addressKey(fromAddr)] = newId;
                player = &players[newId];
                std::cout << "Client connected, assigned ID " << newId << std::endl;
            }

            // Only the session that owns a connection id may drive it, so a
            // forged source address cannot steer somebody else's player. A
            // different id on a port that has gone quiet means the old client
            // left and a new one reused the port: retire the old player and let
            // the fresh connection id open a new one.
            if (header.connectionId != player->session.connectionId()) {
                if (nowMs - player->lastReceiveAtMs > 1000) {
                    disconnectedIds.push_back(player->id);
                }
                continue;
            }
            player->lastReceiveAtMs = nowMs;

            player->session.receiveDatagram(datagram, static_cast<size_t>(received), nowMs,
                                            tickMessages[player->id]);
        }

        // 2. Process Clients

        // Answers to the anti-cheat reports come back from the telemetry thread,
        // so they are collected once per tick and handed to the player they
        // belong to: a warning becomes a chat line, a suspension a dropped peer.
        std::vector<ModerationOutcome> moderationResults;
        {
            std::lock_guard<std::mutex> lock(moderationOutcomeMutex);
            moderationResults.swap(moderationOutcomes);
        }

        for (auto& [id, player] : players) {
            bool shouldDisconnect = false;
            const auto tickEntry = tickMessages.find(id);

            for (const ModerationOutcome& outcome : moderationResults) {
                if (outcome.playerId != id) {
                    continue;
                }
                if (!outcome.message.empty()) {
                    PacketChat notice;
                    std::memset(&notice, 0, sizeof(notice));
                    copyFixedString(notice.username, sizeof(notice.username), "Server");
                    copyFixedString(notice.message, sizeof(notice.message), outcome.message);
                    sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::CHAT, notice), true);
                }
                if (outcome.restricted) {
                    std::cout << "Player " << id << " was removed by moderation." << std::endl;
                    shouldDisconnect = true;
                }
            }

            if (tickEntry != tickMessages.end() && !tickEntry->second.empty()) {
                const std::vector<std::vector<char>>& messages = tickEntry->second;

                for (size_t messageIndex = 0; messageIndex < messages.size() && !shouldDisconnect;
                     ++messageIndex) {
                    PacketProtocol::ParsedPacket frame;
                    PacketProtocol::ParseStatus status = PacketProtocol::tryReadFrame(messages[messageIndex], frame);
                    if (status == PacketProtocol::ParseStatus::NeedMoreData) {
                        break;
                    }
                    if (status == PacketProtocol::ParseStatus::Invalid) {
                        // Every packet type is size-checked, so a client built from
                        // different source fails here before anything is parsed. Name
                        // that instead of implying the packet was corrupt.
                        std::cerr << "Disconnecting player " << id
                                  << " after a packet this build cannot accept; if the client "
                                     "was built separately, rebuild both from the same source"
                                  << std::endl;
                        {
                            // The player only ever saw a blue window and a silent trip
                            // back to the menu, so the reason goes back over the wire in
                            // the one packet shape both builds agree on.
                            PacketChat notice;
                            std::memset(&notice, 0, sizeof(notice));
                            copyFixedString(notice.username, sizeof(notice.username), "Server");
                            copyFixedString(notice.message, sizeof(notice.message),
                                            "This server could not read your join packet, so Client.exe and Server.exe were built from different source. Rebuild both together.");
                            sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::CHAT, notice), true);
                        }
                        shouldDisconnect = true;
                        break;
                    }

                    if (frame.type == PacketType::CONNECT) {
                        // A repeated CONNECT means the WELCOME or its ack was lost.
                        // The peer already exists, so only re-send the welcome.
                        if (player.active) {
                            PacketWelcome repeatedWelcome = {
                                id, PacketProtocol::kProtocolRevision,
                                PacketProtocol::packetLayoutSignature() };
                            sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::WELCOME, repeatedWelcome), true);
                            sendWorldEnvironment(player);
                            continue;
                        }

                        PacketConnect pkt;
                        std::memcpy(&pkt, frame.payload, sizeof(pkt));

                        // Refuse a peer built from a different protocol revision:
                        // its packets would be misread from here on.
                        if (pkt.protocolRevision != PacketProtocol::kProtocolRevision ||
                            pkt.packetLayout != PacketProtocol::packetLayoutSignature()) {
                            std::cerr << "Player " << id << " speaks protocol "
                                      << pkt.protocolRevision << "/" << pkt.packetLayout
                                      << " but this server speaks "
                                      << PacketProtocol::kProtocolRevision << "/"
                                      << PacketProtocol::packetLayoutSignature()
                                      << "; rebuild Client.exe and Server.exe from the same source."
                                      << std::endl;
                            {
                                PacketChat notice;
                                std::memset(&notice, 0, sizeof(notice));
                                copyFixedString(notice.username, sizeof(notice.username), "Server");
                                copyFixedString(notice.message, sizeof(notice.message),
                                                "This server speaks a different protocol version than your client. Rebuild Client.exe and Server.exe from the same source.");
                                sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::CHAT, notice), true);
                            }
                            shouldDisconnect = true;
                            break;
                        }

                        const std::string token = PacketProtocol::fixedString(pkt.authToken, sizeof(pkt.authToken));
                        const std::string hardwareId =
                            PacketProtocol::fixedString(pkt.hardwareId, sizeof(pkt.hardwareId));
                        const bool isGuest = token.empty();

                        player.headColor = rblxmath::vec3(0.8f, 0.6f, 0.4f);
                        player.torsoColor = rblxmath::vec3(0.2f, 0.4f, 0.8f);
                        player.leftArmColor = rblxmath::vec3(0.8f, 0.6f, 0.4f);
                        player.rightArmColor = rblxmath::vec3(0.8f, 0.6f, 0.4f);
                        player.leftLegColor = rblxmath::vec3(0.2f, 0.6f, 0.2f);
                        player.rightLegColor = rblxmath::vec3(0.2f, 0.6f, 0.2f);
                        player.faceId = DEFAULT_FACE_ID;
                        player.hatId.clear();

                        if (!isGuest) {
                            // This is the only place a join waits on the website, and
                            // the client stops waiting after ten seconds. A slow login
                            // therefore looked exactly like a server that died: the log
                            // ended at "Client connected" with no hint about where the
                            // time went. Say when the check starts and how long it took.
                            std::cout << "Player " << id << ": checking the login with "
                                      << webServerUrl << std::endl;
                            const auto authStartedAt = std::chrono::steady_clock::now();
                            // The machine goes with the login: the website records
                            // it and refuses a join from a barred one.
                            VerifiedUser verified = ServerAuth::verifyToken(token, webServerUrl, hardwareId);
                            const long long authMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                         std::chrono::steady_clock::now() - authStartedAt)
                                                         .count();
                            if (!verified.success) {
                                std::cout << "Player " << id << " failed authentication after "
                                          << authMs << " ms" << std::endl;
                                {
                                    // A refused login used to be indistinguishable from a
                                    // dead server on the client side.
                                    PacketChat notice;
                                    std::memset(&notice, 0, sizeof(notice));
                                    copyFixedString(notice.username, sizeof(notice.username), "Server");
                                    // A suspension or a ban refuses the login with a reason
                                    // the player can act on, so it is passed on instead of
                                    // the generic "sign in again" text.
                                    copyFixedString(notice.message, sizeof(notice.message),
                                                    verified.reason.empty()
                                                        ? "The website did not accept this login, so the game server refused the join. Sign in again and retry."
                                                        : verified.reason);
                                    sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::CHAT, notice), true);
                                }
                                shouldDisconnect = true;
                                break;
                            }

                            player.userId = verified.userId;
                            player.authToken = token;
                            player.hardwareId = hardwareId;
                            player.lastPlaytimeReportedAt = std::chrono::steady_clock::now();
                            copyFixedString(player.username, sizeof(player.username), verified.username);

                            std::string avatarJson = ServerAuth::getAvatar(token, webServerUrl);
                            if (!avatarJson.empty() && avatarJson.find("\"success\":true") != std::string::npos) {
                                parseAvatarColor(avatarJson, "headColor", player.headColor);
                                parseAvatarColor(avatarJson, "torsoColor", player.torsoColor);
                                parseAvatarColor(avatarJson, "leftArmColor", player.leftArmColor);
                                parseAvatarColor(avatarJson, "rightArmColor", player.rightArmColor);
                                parseAvatarColor(avatarJson, "leftLegColor", player.leftLegColor);
                                parseAvatarColor(avatarJson, "rightLegColor", player.rightLegColor);
                                player.faceId = parseAvatarFaceId(avatarJson);
                                player.hatId = parseAvatarHatId(avatarJson);
                            }

                            std::cout << "Player " << id << " joined as " << player.username
                                      << " (authenticated, login checked in " << authMs << " ms)"
                                      << std::endl;
                        } else {
                            // A machine ban has to reach guests too: joining without
                            // an account costs nothing, so a check that only ran
                            // behind the login would make the ban worth nothing.
                            std::string hardwareRefusal;
                            if (ServerAuth::isHardwareBanned(hardwareId, &hardwareRefusal, webServerUrl)) {
                                std::cout << "Player " << id << " was refused: its machine is banned"
                                          << std::endl;
                                PacketChat notice;
                                std::memset(&notice, 0, sizeof(notice));
                                copyFixedString(notice.username, sizeof(notice.username), "Server");
                                copyFixedString(notice.message, sizeof(notice.message),
                                                hardwareRefusal.empty()
                                                    ? "This computer is banned from joining games."
                                                    : hardwareRefusal);
                                sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::CHAT, notice), true);
                                shouldDisconnect = true;
                                break;
                            }
                            player.userId = -1;
                            player.authToken.clear();
                            player.hardwareId = hardwareId;
                            int guestNumber = (rand() % 9999) + 1;
                            snprintf(player.username, sizeof(player.username), "guest_%d", guestNumber);
                            std::cout << "Player " << id << " joined as " << player.username << " (guest)" << std::endl;
                        }

                        player.active = true;
                        reportInstanceHeartbeat(true);

                        // Spawn and health are the server's to decide. The world's
                        // SpawnLocation is where a death returns to, and the
                        // movement guard starts from that point instead of from
                        // whatever the client claims in its first packet.
                        player.spawnPosition = findSpawnPosition();
                        player.health.reset();
                        player.state.position = player.spawnPosition;
                        player.state.health = player.health.health;
                        player.movement.teleport(player.spawnPosition, nowMs);
                        player.replay.reset();

                        PacketWelcome welcome = {
                            id, PacketProtocol::kProtocolRevision,
                            PacketProtocol::packetLayoutSignature() };
                        if (!sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::WELCOME, welcome), true)) {
                            shouldDisconnect = true;
                            break;
                        }

                        // Before the snapshot, so the first frame drawn is
                        // already looking through the world's air.
                        sendWorldEnvironment(player);

                        std::vector<PartData> worldData;
                        worldData.reserve(std::min<size_t>(serverParts.size(), PacketProtocol::MAX_WORLD_PARTS));
                        for (size_t i = 0; i < serverParts.size() && worldData.size() < PacketProtocol::MAX_WORLD_PARTS; ++i) {
                            Part& p = serverParts[i];

                            PartData data;
                            std::memset(&data, 0, sizeof(data));
                            data.shape = (int)p.shape;
                            copyFixedString(data.name, sizeof(data.name), p.name);
                            data.parentIndex = p.parentIndex;

                            data.flags = 0;
                            if (p.isFolder) data.flags |= 1;
                            if (p.isCamera) data.flags |= 2;
                            if (p.isSpawn) data.flags |= 4;
                            if (p.deleted) data.flags |= 8;

                            if (p.physicsBody) {
                                rblxphysics::RigidBody* body = (rblxphysics::RigidBody*)p.physicsBody;
                                rblxphysics::Transform t = body->getTransform();
                                rblxphysics::Vector3 pos = t.getPosition();
                                rblxphysics::Quaternion rot = t.getOrientation();

                                data.position = rblxmath::vec3(pos.x, pos.y, pos.z);

                                rblxmath::quat q(rot.w, rot.x, rot.y, rot.z);
                                data.rotation = rblxmath::degrees(rblxmath::eulerAngles(q));
                            } else {
                                data.position = p.position;
                                data.rotation = p.rotation;
                            }

                            data.size = p.size;
                            data.color = p.color;
                            data.transparency = p.transparency;
                            data.reflectance = p.reflectance;

                            data.physicsFlags = 0;
                            if (p.anchored) data.physicsFlags |= 1;
                            if (p.canCollide) data.physicsFlags |= 2;

                            data.mass = p.mass;
                            data.material = static_cast<uint8_t>(p.material);
                            copyFixedString(data.texture, sizeof(data.texture), p.texture);
                            worldData.push_back(data);
                        }

                        uint32_t partCount = static_cast<uint32_t>(worldData.size());
                        uint32_t payloadSize = sizeof(uint32_t) + static_cast<uint32_t>(worldData.size() * sizeof(PartData));
                        std::vector<char> payload(payloadSize);
                        std::memcpy(payload.data(), &partCount, sizeof(uint32_t));
                        if (!worldData.empty()) {
                            std::memcpy(payload.data() + sizeof(uint32_t), worldData.data(), worldData.size() * sizeof(PartData));
                        }

                        if (!sendToPlayer(player, PacketProtocol::buildPacket(PacketType::WORLD_STATE, payload.data(), payloadSize), true)) {
                            shouldDisconnect = true;
                            break;
                        }
                        std::cout << "Sent complete world state (" << partCount << " parts) to player " << id << std::endl;

                        sendPlayerList(player);
                        broadcastPlayerList(id);
                        broadcastPlayerHealth(id, player.health, nowMs);

                        PacketPlayerJoin joinPkt;
                        std::memset(&joinPkt, 0, sizeof(joinPkt));
                        joinPkt.playerId = id;
                        copyFixedString(joinPkt.username, sizeof(joinPkt.username), PacketProtocol::fixedString(player.username, sizeof(player.username)));
                        broadcastPacket(PacketProtocol::buildStructPacket(PacketType::PLAYER_JOIN, joinPkt), id, true);

                        for (auto& [otherId, otherPlayer] : players) {
                            if (otherId != id && otherPlayer.active) {
                                PacketPlayerJoin otherJoinPkt;
                                std::memset(&otherJoinPkt, 0, sizeof(otherJoinPkt));
                                otherJoinPkt.playerId = otherId;
                                copyFixedString(otherJoinPkt.username, sizeof(otherJoinPkt.username), PacketProtocol::fixedString(otherPlayer.username, sizeof(otherPlayer.username)));
                                if (!sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::PLAYER_JOIN, otherJoinPkt), true)) {
                                    shouldDisconnect = true;
                                    break;
                                }
                            }
                        }
                    } else if (frame.type == PacketType::PLAYER_STATE) {
                        if (!player.active) {
                            continue;
                        }

                        PacketPlayerState pkt;
                        std::memcpy(&pkt, frame.payload, sizeof(pkt));
                        if (!isSafePlayerState(pkt)) {
                            std::cerr << "Ignoring invalid player state from player " << id << std::endl;
                            continue;
                        }

                        if (player.health.isDead()) {
                            // Waiting to respawn: ignore the dead character's
                            // movement instead of letting the client steer it
                            // around while the countdown runs.
                            continue;
                        }

                        if (vehicle.driverId == id) {
                            // The car carries the character, so a walking packet
                            // from the driver would fight the seat position.
                            continue;
                        }

                        pkt.rotationY = normalizeRotationDegrees(pkt.rotationY);
                        // Health is the server's now. This used to clamp whatever
                        // the client reported, so a modified client could send
                        // 100 forever and nothing could ever kill it.
                        pkt.health = player.health.health;
                        player.state = pkt;
                        player.state.playerId = id;

                        // The client simulates its own character, so this
                        // position is a claim. Hold it against the last one the
                        // server accepted and clamp anything a player could not
                        // have walked, jumped or fallen into.
                        // Where the client said it was, before the guard touches
                        // it: a replay is only worth watching if it shows the claim
                        // rather than the correction.
                        const rblxmath::vec3 claimedState = player.state.position;
                        // Measured against the client's own clock: a packet that
                        // spent 200 ms in flight still describes movement that
                        // happened over the time the client says it did.
                        const rblx::movement::Correction movement =
                            player.movement.validate(player.state.position, nowMs,
                                                     pkt.clientTimeMs);
                        player.replay.add(nowMs, replayPoint(claimedState),
                                          replayPoint(movement.position));
                        if (movement.corrected) {
                            player.state.position = movement.position;
                            if (movement.flight || player.movement.corrections() <= 3 ||
                                player.movement.corrections() % 100 == 0) {
                                std::cerr << "Corrected movement for player " << id << " ("
                                          << (movement.flight ? "climbed without falling"
                                                              : "moved faster than allowed")
                                          << ", " << player.movement.corrections()
                                          << " corrections so far)" << std::endl;
                            }
                            // Report the episode, not the packet: a fly hack is
                            // corrected once per packet, and a strike each would
                            // ban the account before the warning reached the screen.
                            const int corrections = player.movement.corrections();
                            const int needed = movement.flight ? kFlightCorrectionsPerReport
                                                               : kCorrectionsPerReport;
                            if (corrections - player.reportedCorrections >= needed) {
                                player.reportedCorrections = corrections;
                                // The frame that earned the strike travels with it,
                                // beside the rule it broke. An admin judging an appeal
                                // reads what the server measured against what the game
                                // allows, instead of a label nobody can check.
                                std::string proof =
                                    rblx::movement::describeProof(movement.evidence);
                                proof += ", correction " + std::to_string(corrections) +
                                         " this session";
                                reportCheatingToWebsite(
                                    id, PacketProtocol::fixedString(player.username, sizeof(player.username)),
                                    movement.flight ? "flying" : "moving faster than allowed",
                                    proof, rblx::movement::describeExpectation(movement.evidence),
                                    player.replay.encode());
                            }

                            // Snap the client back, otherwise it keeps
                            // predicting from the position just rejected.
                            sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::TELEPORT, player.state), true);
                        }

                        updatePlayerShadowBody(player);
                        broadcastPacket(PacketProtocol::buildStructPacket(PacketType::PLAYER_STATE, player.state), id);
                    } else if (frame.type == PacketType::VEHICLE_TOGGLE) {
                        if (!player.active || player.health.isDead() || vehicle.partIndex < 0) {
                            continue;
                        }
                        if (vehicle.driverId == id) {
                            unseatPlayer(player, id, nowMs, true);
                            continue;
                        }
                        if (vehicle.driverId != -1) {
                            continue;
                        }
                        // Only a player standing next to the car can get in.
                        if (!rblx::vehicle::canEnter(vehicle.state.position, player.state.position)) {
                            continue;
                        }
                        seatPlayer(player, id, nowMs);
                    } else if (frame.type == PacketType::VEHICLE_INPUT) {
                        if (!player.active || vehicle.driverId != id) {
                            continue;
                        }

                        PacketVehicleInput input;
                        std::memcpy(&input, frame.payload, sizeof(input));
                        // Input::set clamps and rejects non-finite values, so the
                        // car only ever sees -1..1.
                        vehicle.input.set(input.throttle, input.steer);
                    } else if (frame.type == PacketType::RESPAWN_REQUEST) {
                        if (!player.active) {
                            continue;
                        }
                        // "Reset Character": the client asks, the server kills the
                        // character, so a reset and a death share one timer.
                        damagePlayer(player, id, rblx::health::kMaxHealth, nowMs,
                                     "resetting the character");
                    } else if (frame.type == PacketType::CHAT) {
                        if (!player.active) {
                            continue;
                        }

                        PacketChat chat;
                        std::memcpy(&chat, frame.payload, sizeof(chat));
                        chat.playerId = id;
                        copyFixedString(chat.username, sizeof(chat.username), PacketProtocol::fixedString(player.username, sizeof(player.username)));
                        const std::string message = PacketProtocol::fixedString(chat.message, sizeof(chat.message));
                        if (!message.empty()) {
                            copyFixedString(chat.message, sizeof(chat.message), message);
                            broadcastPacket(PacketProtocol::buildStructPacket(PacketType::CHAT, chat), -1, true);
                        }
                    }

                }

            } else if (player.lastReceiveAtMs != 0 && nowMs - player.lastReceiveAtMs > kPeerTimeoutMs) {
                std::cout << "Player " << id << " went silent." << std::endl;
                shouldDisconnect = true;
            }

            if (shouldDisconnect && std::find(disconnectedIds.begin(), disconnectedIds.end(), id) == disconnectedIds.end()) {
                disconnectedIds.push_back(id);
            }
        }

        // Send whatever this tick produced: welcomes, world data, acks.
        for (auto& [id, player] : players) {
            player.session.pump(nowMs, playerSendFunction(player));
        }

        // 3. Handle Disconnects
        for (int id : disconnectedIds) {
            if (players.find(id) == players.end()) {
                continue;
            }
            std::cout << "Player " << id << " disconnected." << std::endl;
            peerAddresses.erase(addressKey(players[id].address));

            if (vehicle.driverId == id) {
                vehicle.driverId = -1;
                vehicle.state.speed = 0.0f;
                vehicle.input.set(0.0f, 0.0f);
                broadcastVehicleState();
            }

            // Cleanup Shadow Body
            if (players[id].shadowBody) {
                serverPhysicsWorld.world->destroyRigidBody(players[id].shadowBody);
            }

            if (players[id].active) {
                if (players[id].userId > 0 && !players[id].authToken.empty()) {
                    reportPlayerPlaytime(players[id], std::chrono::steady_clock::now(), id, true);
                }

                players[id].active = false;
                PacketPlayerLeave leavePkt = { id };
                broadcastPacket(PacketProtocol::buildStructPacket(PacketType::PLAYER_LEAVE, leavePkt), id, true);
                broadcastPlayerList();
                reportInstanceHeartbeat(true);
            }

            players.erase(id);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Network::cleanup();
    return 0;
}

// A server that dies during start-up used to leave an empty log and nothing but a
// status code, so turn an escaped C++ exception into something readable.
int main(int argc, char** argv) {
    try {
        return runServer(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Fatal error: unknown exception" << std::endl;
        return 1;
    }
}
