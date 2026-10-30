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
#include "Part.h"
#include "PhysicsWorld.h"
#include "WorldLoader.h"
#include "ServerAuth.h"
#include "WorldScript.h"
#include "UdpTransport.h"

#include <condition_variable>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

// GLM Includes
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

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
    std::chrono::steady_clock::time_point lastPlaytimeReportedAt;
    rp3d::RigidBody* shadowBody; // Server-side physics body for player
    glm::vec3 headColor = glm::vec3(0.8f, 0.6f, 0.4f);
    glm::vec3 torsoColor = glm::vec3(0.2f, 0.4f, 0.8f);
    glm::vec3 leftArmColor = glm::vec3(0.8f, 0.6f, 0.4f);
    glm::vec3 rightArmColor = glm::vec3(0.8f, 0.6f, 0.4f);
    glm::vec3 leftLegColor = glm::vec3(0.2f, 0.6f, 0.2f);
    glm::vec3 rightLegColor = glm::vec3(0.2f, 0.6f, 0.2f);
    std::string faceId = DEFAULT_FACE_ID;
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
std::map<int, glm::vec3> lastBroadcastPositions;
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

bool isFiniteVec3(const glm::vec3& value) {
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

bool parseAvatarColor(const std::string& avatarJson, const std::string& name, glm::vec3& color) {
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

void copyFixedString(char* destination, size_t destinationSize, const std::string& value) {
    if (!destination || destinationSize == 0) return;
    std::memset(destination, 0, destinationSize);
    const size_t bytesToCopy = std::min(destinationSize - 1, value.size());
    std::memcpy(destination, value.data(), bytesToCopy);
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
        rp3d::Vector3 pos(player.state.position.x, player.state.position.y, player.state.position.z);
        rp3d::Quaternion rot = rp3d::Quaternion::identity();
        rp3d::Transform transform(pos, rot);

        player.shadowBody = serverPhysicsWorld.world->createRigidBody(transform);
        player.shadowBody->setType(rp3d::BodyType::KINEMATIC); // Moved by code (player input)

        // Add a capsule or box shape for collision
        rp3d::Vector3 halfExtents(1.0f, 2.5f, 1.0f); // Roughly character size
        rp3d::BoxShape* shape = serverPhysicsWorld.physicsCommon.createBoxShape(halfExtents);
        player.shadowBody->addCollider(shape, rp3d::Transform::identity());

        // Set friction to 0 like character
        if (player.shadowBody->getNbColliders() > 0) {
            player.shadowBody->getCollider(0)->getMaterial().setFrictionCoefficient(0.0f);
        }
    }

    // Move shadow body smoothly using velocity (KINEMATIC bodies can push DYNAMIC parts when moving)
    rp3d::Transform currentTransform = player.shadowBody->getTransform();
    rp3d::Vector3 currentPos = currentTransform.getPosition();
    rp3d::Vector3 targetPos(player.state.position.x, player.state.position.y, player.state.position.z);

    rp3d::Vector3 diff = targetPos - currentPos;
    float distance = diff.length();

    // Set velocity to move towards target (KINEMATIC bodies with velocity can push parts)
    if (distance > 0.01f) {
        // Calculate velocity needed to reach target in one physics step (1/60s)
        float speed = distance * 60.0f; // Move distance in one frame
        rp3d::Vector3 direction = diff.getUnit();
        rp3d::Vector3 velocity = direction * speed;
        player.shadowBody->setLinearVelocity(velocity);
    } else {
        // Close enough, just set position directly
        player.shadowBody->setLinearVelocity(rp3d::Vector3(0, 0, 0));
        rp3d::Transform newTransform(targetPos, currentTransform.getOrientation());
        player.shadowBody->setTransform(newTransform);
    }

    // Update rotation smoothly
    rp3d::Quaternion currentRot = currentTransform.getOrientation();
    rp3d::Quaternion targetRot = rp3d::Quaternion::fromEulerAngles(0, glm::radians(player.state.rotationY), 0);
    rp3d::Quaternion newRot = rp3d::Quaternion::slerp(currentRot, targetRot, 0.3f);
    rp3d::Transform newTransform(currentPos, newRot);
    player.shadowBody->setTransform(newTransform);

    // Zero angular velocity (we control rotation directly)
    player.shadowBody->setAngularVelocity(rp3d::Vector3(0, 0, 0));
}

void teleportPlayer(const std::string& username, const glm::vec3& position) {
    for (auto& entry : players) {
        ConnectedPlayer& player = entry.second;
        if (!player.active) continue;
        const std::string name =
            PacketProtocol::fixedString(player.username, sizeof(player.username));
        if (name != username) continue;

        player.state.position = position;
        if (player.shadowBody) {
            rp3d::Transform transform = player.shadowBody->getTransform();
            transform.setPosition(rp3d::Vector3(position.x, position.y, position.z));
            player.shadowBody->setTransform(transform);
            player.shadowBody->setLinearVelocity(rp3d::Vector3(0.0f, 0.0f, 0.0f));
            player.shadowBody->setAngularVelocity(rp3d::Vector3(0.0f, 0.0f, 0.0f));
        }
        sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::TELEPORT, player.state), true);
        return;
    }
}

void broadcastWorldUpdate() {
    // Dynamic parts are sent every tick. Anchored parts are sent only when a
    // script moved them, which keeps scripted platforms in sync without
    // re-sending the whole world.
    std::vector<PartUpdate> updates;
    for (int i = 0; i < static_cast<int>(serverParts.size()); ++i) {
        Part& p = serverParts[i];
        if (p.deleted || p.isFolder || !p.physicsBody) continue;

        rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(p.physicsBody);
        const bool dynamic = body->getType() == rp3d::BodyType::DYNAMIC;

        rp3d::Transform t = body->getTransform();
        const rp3d::Vector3 pos = t.getPosition();
        const rp3d::Quaternion rot = t.getOrientation();
        const glm::vec3 newPosition(pos.x, pos.y, pos.z);

        const auto previous = lastBroadcastPositions.find(i);
        const bool moved = previous == lastBroadcastPositions.end() ||
                           glm::length(previous->second - newPosition) > 0.001f;
        if (!dynamic && !moved) continue;

        p.position = newPosition;

        PartUpdate up;
        up.partIndex = i;
        up.position = newPosition;

        // Convert quaternion to euler for network transmission
        glm::quat q(rot.w, rot.x, rot.y, rot.z);
        up.rotation = glm::degrees(glm::eulerAngles(q));

        updates.push_back(up);
        lastBroadcastPositions[i] = newPosition;
    }

    if (updates.empty()) return;

    if (updates.size() > PacketProtocol::MAX_WORLD_UPDATES) {
        updates.resize(PacketProtocol::MAX_WORLD_UPDATES);
    }

    uint32_t payloadSize = static_cast<uint32_t>(updates.size() * sizeof(PartUpdate));
    broadcastPacket(PacketProtocol::buildPacket(PacketType::WORLD_UPDATE, updates.data(), payloadSize));
}

static int runServer(int argc, char** argv) {
    // The website redirects this output into game-server-<port>.log. Without this
    // the C runtime buffers it, so the file stays empty until the process exits
    // and a join that failed has nothing to explain itself with.
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

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
    WorldLoader::loadWorld(worldPath, serverParts, serverPhysicsWorld);
    std::cout << "Loaded ServerWorld with " << serverParts.size() << " parts." << std::endl;

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
                    scriptPlayer.name = PacketProtocol::fixedString(
                        player.username, sizeof(player.username));
                    if (scriptPlayer.name.empty()) {
                        scriptPlayer.name = "Player" + std::to_string(entry.first);
                    }
                    scriptPlayer.position = player.state.position;
                    snapshot.push_back(scriptPlayer);
                }
                return snapshot;
            });
            gameScriptRuntime->setTeleportHandler([](const std::string& name,
                                                     const glm::vec3& position) {
                teleportPlayer(name, position);
            });

            std::string scriptError;
            if (!gameScriptRuntime->runScript(scriptBuffer.str(), scriptPath, scriptError)) {
                std::cerr << "Script error: " << scriptError << std::endl;
            } else {
                std::cout << "Loaded script " << scriptPath << std::endl;
            }
        }
    }

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
        reportInstanceHeartbeat(false);

        // Scripts tick with the network rate so gameplay logic runs at 60Hz
        // instead of once per loop iteration.
        if (networkAccumulator >= 1.0f/60.0f) {
            if (gameScriptRuntime) {
                gameScriptRuntime->heartbeat(networkAccumulator);
            }
            broadcastWorldUpdate();
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

        for (auto& [id, player] : players) {
            bool shouldDisconnect = false;
            const auto tickEntry = tickMessages.find(id);

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
                        const bool isGuest = token.empty();

                        player.headColor = glm::vec3(0.8f, 0.6f, 0.4f);
                        player.torsoColor = glm::vec3(0.2f, 0.4f, 0.8f);
                        player.leftArmColor = glm::vec3(0.8f, 0.6f, 0.4f);
                        player.rightArmColor = glm::vec3(0.8f, 0.6f, 0.4f);
                        player.leftLegColor = glm::vec3(0.2f, 0.6f, 0.2f);
                        player.rightLegColor = glm::vec3(0.2f, 0.6f, 0.2f);
                        player.faceId = DEFAULT_FACE_ID;

                        if (!isGuest) {
                            VerifiedUser verified = ServerAuth::verifyToken(token, webServerUrl);
                            if (!verified.success) {
                                std::cout << "Player " << id << " failed authentication" << std::endl;
                                {
                                    // A refused login used to be indistinguishable from a
                                    // dead server on the client side.
                                    PacketChat notice;
                                    std::memset(&notice, 0, sizeof(notice));
                                    copyFixedString(notice.username, sizeof(notice.username), "Server");
                                    copyFixedString(notice.message, sizeof(notice.message),
                                                    "The website did not accept this login, so the game server refused the join. Sign in again and retry.");
                                    sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::CHAT, notice), true);
                                }
                                shouldDisconnect = true;
                                break;
                            }

                            player.userId = verified.userId;
                            player.authToken = token;
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
                            }

                            std::cout << "Player " << id << " joined as " << player.username << " (authenticated)" << std::endl;
                        } else {
                            player.userId = -1;
                            player.authToken.clear();
                            int guestNumber = (rand() % 9999) + 1;
                            snprintf(player.username, sizeof(player.username), "guest_%d", guestNumber);
                            std::cout << "Player " << id << " joined as " << player.username << " (guest)" << std::endl;
                        }

                        player.active = true;
                        reportInstanceHeartbeat(true);

                        PacketWelcome welcome = {
                            id, PacketProtocol::kProtocolRevision,
                            PacketProtocol::packetLayoutSignature() };
                        if (!sendToPlayer(player, PacketProtocol::buildStructPacket(PacketType::WELCOME, welcome), true)) {
                            shouldDisconnect = true;
                            break;
                        }

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

                            if (p.physicsBody) {
                                rp3d::RigidBody* body = (rp3d::RigidBody*)p.physicsBody;
                                rp3d::Transform t = body->getTransform();
                                rp3d::Vector3 pos = t.getPosition();
                                rp3d::Quaternion rot = t.getOrientation();

                                data.position = glm::vec3(pos.x, pos.y, pos.z);

                                glm::quat q(rot.w, rot.x, rot.y, rot.z);
                                data.rotation = glm::degrees(glm::eulerAngles(q));
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

                        pkt.rotationY = normalizeRotationDegrees(pkt.rotationY);
                        player.state = pkt;
                        player.state.playerId = id;
                        player.state.health = std::max(0.0f, std::min(100.0f, player.state.health));

                        updatePlayerShadowBody(player);
                        broadcastPacket(PacketProtocol::buildStructPacket(PacketType::PLAYER_STATE, player.state), id);
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
