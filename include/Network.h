#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

#include <glm/glm.hpp>

// Packet Types
enum class PacketType : uint8_t {
    CONNECT = 0,
    WELCOME = 1,
    PLAYER_STATE = 2,
    PLAYER_JOIN = 3,
    PLAYER_LEAVE = 4,
    WORLD_UPDATE = 5,
    CHAT = 6,
    WORLD_STATE = 7,
    PLAYER_LIST = 8,
    TELEPORT = 9,
    PLAYER_HEALTH = 10,
    RESPAWN_REQUEST = 11,
    VEHICLE_STATE = 12,
    VEHICLE_INPUT = 13,
    VEHICLE_TOGGLE = 14,
    PART_STATE = 15,
    PART_REMOVE = 16
};

#pragma pack(push, 1)

struct PacketHeader {
    PacketType type;
    uint32_t size; // Payload size in bytes. Current protocol is little-endian.
};

// 1. Connect (Client -> Server)
struct PacketConnect {
    char username[32];
    char authToken[256]; // JWT token for authentication
    // Handshake fingerprint: a build compiled from a different revision would
    // parse every later packet as garbage, so both sides compare these first.
    uint16_t protocolRevision;
    uint16_t packetLayout;
};

// 2. Welcome (Server -> Client)
struct PacketWelcome {
    int playerId;
    uint16_t protocolRevision;
    uint16_t packetLayout;
};

// 3. Player State (Client <-> Server)
struct PacketPlayerState {
    int playerId;
    glm::vec3 position;
    float rotationY;
    bool isWalking;
    bool isJumping;
    float health;
};

// 10. Player Health (Server -> Client)
//
// Health and the respawn timer belong to the server. The client only displays
// them, and can ask to be respawned with RESPAWN_REQUEST, which the server
// treats as a death so the two never disagree about who is alive.
struct PacketPlayerHealth {
    int playerId;
    float health;
    float maxHealth;
    uint16_t respawnSeconds; // 0 while alive
};

// 12. Vehicle State (Server -> Client)
//
// The car is simulated by the server; every client follows these packets. A
// client never reports the car's position.
struct PacketVehicleState {
    int partIndex;  // index in the world's part list, -1 when the world has no car
    int driverId;   // -1 when nobody is driving
    glm::vec3 position;
    float yaw;      // degrees
    float speed;    // units per second along the heading
};

// 13. Vehicle Input (Client -> Server)
struct PacketVehicleInput {
    int playerId;
    float throttle;  // -1 brake/reverse, +1 forward
    float steer;     // -1 left, +1 right
};

// 4. Player Join (Server -> Client)
struct PacketPlayerJoin {
    int playerId;
    char username[32];
};

// 5. Player Leave (Server -> Client)
struct PacketPlayerLeave {
    int playerId;
};

// 6. Chat (Client <-> Server)
struct PacketChat {
    int playerId;
    char username[32];
    char message[256];
};

// 8. Player List (Server -> Client)
struct PlayerInfo {
    int playerId;
    char username[32];
    float headColor[3];
    float torsoColor[3];
    float leftArmColor[3];
    float rightArmColor[3];
    float leftLegColor[3];
    float rightLegColor[3];
    char faceId[16];
    char hatId[16];
};

// 6. World Update (Server -> Client)
struct PartUpdate {
    int partIndex;
    glm::vec3 position;
    glm::vec3 rotation;
};

// 7. World State (Server -> Client)
struct PartData {
    int shape;
    char name[64];
    int parentIndex;
    uint8_t flags; // isFolder (bit 0), isCamera (bit 1), isSpawn (bit 2), deleted (bit 3)
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;
    glm::vec3 rotation;
    float transparency;
    float reflectance;
    uint8_t physicsFlags; // anchored (bit 0), canCollide (bit 1)
    float mass;
    uint8_t material; // Material enum value
    char texture[64]; // Optional image path, relative to textures/
};

// 9. Part State (Server -> Client): a part the script built after this client
// joined, described exactly like a world snapshot entry. The index is the
// server's, which is the index the snapshot gave the parts before it.
struct PacketPartState {
    int partIndex;
    PartData data;
};

// 10. Part Remove (Server -> Client): a part the script destroyed. The index
// stays taken so the parts after it keep theirs.
struct PacketPartRemove {
    int partIndex;
};

#pragma pack(pop)

namespace PacketProtocol {
    // Bumped whenever a packet layout or its meaning changes. Sent in the
    // handshake next to `kPacketLayoutSignature` so two differently built
    // binaries fail with an explanation instead of silently misparsing.
    constexpr uint16_t kProtocolRevision = 8;

    constexpr uint16_t packetLayoutSignature() {
        return static_cast<uint16_t>(sizeof(PacketConnect) * 31u + sizeof(PacketWelcome) * 17u +
                                     sizeof(PacketPlayerState) * 13u +
                                     sizeof(PacketPlayerJoin) * 7u + sizeof(PacketChat) * 3u +
                                     sizeof(PacketPlayerHealth) * 11u +
                                     sizeof(PacketVehicleState) * 13u +
                                     sizeof(PacketVehicleInput) * 7u +
                                     sizeof(PlayerInfo) * 5u);
    }

    // A snapshot of the world is one frame, so this is the size of the largest
    // world a server can hand to a client, not the size of a datagram. The
    // transport fragments a frame into datagrams and reassembles it (see
    // rblx::net::kMaxMessageSize in UdpTransport.h), and the sockets are given
    // multi-megabyte buffers for the burst, so the only real limit is this one.
    // It used to be 64 KB - one datagram - which was enough for a painted world
    // of a few hundred parts and silently too small for a script that builds a
    // hundred rooms: the client rejected the frame as malformed, dropped the
    // connection and went back to the menu with nothing drawn.
    constexpr uint32_t MAX_PAYLOAD_SIZE = 1024 * 1024;
    constexpr uint32_t MAX_PLAYERS = 128;
    constexpr uint32_t MAX_WORLD_PARTS = 4096;
    constexpr uint32_t MAX_WORLD_UPDATES = 4096;

    static_assert(sizeof(uint32_t) + MAX_WORLD_PARTS * sizeof(PartData) <= MAX_PAYLOAD_SIZE,
                  "a full world snapshot has to fit in one frame");
    static_assert(MAX_WORLD_UPDATES * sizeof(PartUpdate) <= MAX_PAYLOAD_SIZE,
                  "a full world update has to fit in one frame");

    enum class ParseStatus {
        Complete,
        NeedMoreData,
        Invalid
    };

    struct ParsedPacket {
        PacketType type = PacketType::CONNECT;
        const char* payload = nullptr;
        uint32_t payloadSize = 0;
        size_t frameSize = 0;
    };

    inline bool isValidType(PacketType type) {
        switch (type) {
            case PacketType::CONNECT:
            case PacketType::WELCOME:
            case PacketType::PLAYER_STATE:
            case PacketType::PLAYER_JOIN:
            case PacketType::PLAYER_LEAVE:
            case PacketType::WORLD_UPDATE:
            case PacketType::CHAT:
            case PacketType::WORLD_STATE:
            case PacketType::PLAYER_LIST:
            case PacketType::TELEPORT:
            case PacketType::PLAYER_HEALTH:
            case PacketType::RESPAWN_REQUEST:
            case PacketType::VEHICLE_STATE:
            case PacketType::VEHICLE_INPUT:
            case PacketType::VEHICLE_TOGGLE:
            case PacketType::PART_STATE:
            case PacketType::PART_REMOVE:
                return true;
            default:
                return false;
        }
    }

    inline bool validatePayload(PacketType type, const char* payload, uint32_t payloadSize) {
        if (payloadSize > MAX_PAYLOAD_SIZE) {
            return false;
        }

        switch (type) {
            case PacketType::CONNECT:
                return payloadSize == sizeof(PacketConnect);
            case PacketType::WELCOME:
                return payloadSize == sizeof(PacketWelcome);
            case PacketType::PLAYER_STATE:
            case PacketType::TELEPORT:
                return payloadSize == sizeof(PacketPlayerState);
            case PacketType::PLAYER_HEALTH:
                return payloadSize == sizeof(PacketPlayerHealth);
            case PacketType::RESPAWN_REQUEST:
                return payloadSize == 0;
            case PacketType::VEHICLE_STATE:
                return payloadSize == sizeof(PacketVehicleState);
            case PacketType::VEHICLE_INPUT:
                return payloadSize == sizeof(PacketVehicleInput);
            case PacketType::VEHICLE_TOGGLE:
                return payloadSize == 0;
            case PacketType::PART_STATE:
                return payloadSize == sizeof(PacketPartState);
            case PacketType::PART_REMOVE:
                return payloadSize == sizeof(PacketPartRemove);
            case PacketType::PLAYER_JOIN:
                return payloadSize == sizeof(PacketPlayerJoin);
            case PacketType::PLAYER_LEAVE:
                return payloadSize == sizeof(PacketPlayerLeave);
            case PacketType::WORLD_UPDATE:
                return payloadSize % sizeof(PartUpdate) == 0 &&
                       payloadSize / sizeof(PartUpdate) <= MAX_WORLD_UPDATES;
            case PacketType::CHAT:
                return payloadSize == sizeof(PacketChat);
            case PacketType::WORLD_STATE: {
                if (payloadSize < sizeof(uint32_t) || !payload) return false;
                uint32_t count = 0;
                std::memcpy(&count, payload, sizeof(count));
                return count <= MAX_WORLD_PARTS &&
                       payloadSize == sizeof(uint32_t) + count * sizeof(PartData);
            }
            case PacketType::PLAYER_LIST: {
                if (payloadSize < sizeof(uint32_t) || !payload) return false;
                uint32_t count = 0;
                std::memcpy(&count, payload, sizeof(count));
                return count <= MAX_PLAYERS &&
                       payloadSize == sizeof(uint32_t) + count * sizeof(PlayerInfo);
            }
            default:
                return false;
        }
    }

    inline ParseStatus tryReadFrame(const std::vector<char>& buffer, ParsedPacket& packet) {
        if (buffer.size() < sizeof(PacketHeader)) {
            return ParseStatus::NeedMoreData;
        }

        PacketHeader header;
        std::memcpy(&header, buffer.data(), sizeof(header));

        if (!isValidType(header.type) || header.size > MAX_PAYLOAD_SIZE) {
            return ParseStatus::Invalid;
        }

        const size_t frameSize = sizeof(PacketHeader) + static_cast<size_t>(header.size);
        if (buffer.size() < frameSize) {
            return ParseStatus::NeedMoreData;
        }

        const char* payload = buffer.data() + sizeof(PacketHeader);
        if (!validatePayload(header.type, payload, header.size)) {
            return ParseStatus::Invalid;
        }

        packet.type = header.type;
        packet.payload = payload;
        packet.payloadSize = header.size;
        packet.frameSize = frameSize;
        return ParseStatus::Complete;
    }

    inline void consumeFrame(std::vector<char>& buffer, size_t frameSize) {
        buffer.erase(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(frameSize));
    }

    inline std::vector<char> buildPacket(PacketType type, const void* payload, uint32_t payloadSize) {
        std::vector<char> packet(sizeof(PacketHeader) + payloadSize);
        PacketHeader header{ type, payloadSize };
        std::memcpy(packet.data(), &header, sizeof(header));
        if (payload && payloadSize > 0) {
            std::memcpy(packet.data() + sizeof(PacketHeader), payload, payloadSize);
        }
        return packet;
    }

    template <typename T>
    inline std::vector<char> buildStructPacket(PacketType type, const T& payload) {
        return buildPacket(type, &payload, static_cast<uint32_t>(sizeof(T)));
    }

    inline std::string fixedString(const char* value, size_t capacity) {
        if (!value || capacity == 0) {
            return {};
        }
        size_t length = 0;
        while (length < capacity && value[length] != '\0') {
            ++length;
        }
        return std::string(value, length);
    }
}

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")

// The length argument of accept()/recvfrom()/getsockname(): an int on WinSock,
// a socklen_t on POSIX. Call sites use this so they compile on both.
using SocketLength = int;

class Network {
public:
    // How long sendAll keeps retrying a full socket before giving up.
    static constexpr int SEND_TIMEOUT_MS = 500;

    static bool init() {
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
    }

    static void cleanup() {
        WSACleanup();
    }

    static bool setNonBlocking(SOCKET s) {
        u_long mode = 1;
        return ioctlsocket(s, FIONBIO, &mode) == 0;
    }

    // Gameplay packets are tiny and latency sensitive, so stop the stack from
    // holding them back while it waits for more data (Nagle) or for a delayed
    // ACK. Without this a steady 60Hz stream stalls for tens to hundreds of
    // milliseconds at a time and then catches up in a burst.
    static bool setNoDelay(SOCKET s) {
        const int enabled = 1;
        return setsockopt(s, IPPROTO_TCP, TCP_NODELAY,
                          reinterpret_cast<const char*>(&enabled), sizeof(enabled)) == 0;
    }

    static SOCKET createUdpSocket() {
        return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }

    // A world snapshot arrives as a burst of fragments, so the socket needs room
    // for the whole burst instead of dropping the tail of it.
    static bool setIoBufferSize(SOCKET s, int bytes) {
        setsockopt(s, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&bytes), sizeof(bytes));
        setsockopt(s, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&bytes), sizeof(bytes));
        return true;
    }

    static bool sendAll(SOCKET socket, const void* data, size_t size) {
        const char* bytes = static_cast<const char*>(data);
        size_t sentTotal = 0;

        // The game loop sends to every player from one thread, so a client that
        // stops draining its receive window must not be able to wedge it here.
        const auto deadline = std::chrono::steady_clock::now() +
                              std::chrono::milliseconds(SEND_TIMEOUT_MS);

        while (sentTotal < size) {
            const int chunkSize = static_cast<int>(std::min<size_t>(size - sentTotal, 16384));
            const int sent = send(socket, bytes + sentTotal, chunkSize, 0);
            if (sent == SOCKET_ERROR) {
                const int error = WSAGetLastError();
                if (error == WSAEWOULDBLOCK) {
                    if (std::chrono::steady_clock::now() >= deadline) {
                        return false;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }
                return false;
            }
            if (sent == 0) {
                return false;
            }
            sentTotal += static_cast<size_t>(sent);
        }

        return true;
    }

    static bool sendPacket(SOCKET socket, PacketType type, const void* payload, uint32_t payloadSize) {
        if (!PacketProtocol::validatePayload(type, static_cast<const char*>(payload), payloadSize)) {
            return false;
        }
        const std::vector<char> packet = PacketProtocol::buildPacket(type, payload, payloadSize);
        return sendAll(socket, packet.data(), packet.size());
    }

    template <typename T>
    static bool sendStructPacket(SOCKET socket, PacketType type, const T& payload) {
        return sendPacket(socket, type, &payload, static_cast<uint32_t>(sizeof(T)));
    }
};
#else  // POSIX sockets

#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using SOCKET = int;
constexpr SOCKET INVALID_SOCKET = -1;
constexpr int SOCKET_ERROR = -1;

// The length argument of accept()/recvfrom()/getsockname(): an int on WinSock,
// a socklen_t on POSIX. Call sites use this so they compile on both.
using SocketLength = socklen_t;

// The client and server were written against WinSock, and their socket code is
// identical apart from the names. Aliasing the handful of WinSock spellings
// here keeps one implementation of the game instead of an #ifdef around every
// send, close and error check in Client.cpp and Server.cpp.
inline int closesocket(SOCKET s) {
    return ::close(s);
}

inline int WSAGetLastError() {
    return errno;
}

#ifndef WSAEWOULDBLOCK
    #define WSAEWOULDBLOCK EWOULDBLOCK
#endif
#ifndef WSAECONNRESET
    #define WSAECONNRESET ECONNRESET
#endif
#ifndef WSAEINPROGRESS
    #define WSAEINPROGRESS EINPROGRESS
#endif
#ifndef WSAEISCONN
    #define WSAEISCONN EISCONN
#endif

// A peer that disappears mid-send would otherwise raise SIGPIPE and kill the
// process; MSG_NOSIGNAL is the per-send form of the Windows behaviour.
#ifndef MSG_NOSIGNAL
    #define MSG_NOSIGNAL 0
#endif

class Network {
public:
    // How long sendAll keeps retrying a full socket before giving up.
    static constexpr int SEND_TIMEOUT_MS = 500;

    static bool init() {
#ifdef SIGPIPE
        std::signal(SIGPIPE, SIG_IGN);
#endif
        return true;
    }

    static void cleanup() {}

    static bool setNonBlocking(SOCKET s) {
        const int flags = fcntl(s, F_GETFL, 0);
        if (flags < 0) {
            return false;
        }
        return fcntl(s, F_SETFL, flags | O_NONBLOCK) == 0;
    }

    // Gameplay packets are tiny and latency sensitive, so stop the stack from
    // holding them back while it waits for more data (Nagle) or for a delayed
    // ACK. Without this a steady 60Hz stream stalls for tens to hundreds of
    // milliseconds at a time and then catches up in a burst.
    static bool setNoDelay(SOCKET s) {
        const int enabled = 1;
        return setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &enabled, sizeof(enabled)) == 0;
    }

    static SOCKET createUdpSocket() {
        return ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }

    // A world snapshot arrives as a burst of fragments, so the socket needs room
    // for the whole burst instead of dropping the tail of it.
    static bool setIoBufferSize(SOCKET s, int bytes) {
        setsockopt(s, SOL_SOCKET, SO_RCVBUF, &bytes, sizeof(bytes));
        setsockopt(s, SOL_SOCKET, SO_SNDBUF, &bytes, sizeof(bytes));
        return true;
    }

    static bool sendAll(SOCKET socket, const void* data, size_t size) {
        const char* bytes = static_cast<const char*>(data);
        size_t sentTotal = 0;

        // The game loop sends to every player from one thread, so a client that
        // stops draining its receive window must not be able to wedge it here.
        const auto deadline = std::chrono::steady_clock::now() +
                              std::chrono::milliseconds(SEND_TIMEOUT_MS);

        while (sentTotal < size) {
            const size_t chunkSize = std::min<size_t>(size - sentTotal, 16384);
            const ssize_t sent = ::send(socket, bytes + sentTotal, chunkSize, MSG_NOSIGNAL);
            if (sent < 0) {
                const int error = errno;
                if (error == EWOULDBLOCK || error == EINTR) {
                    if (std::chrono::steady_clock::now() >= deadline) {
                        return false;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }
                return false;
            }
            if (sent == 0) {
                return false;
            }
            sentTotal += static_cast<size_t>(sent);
        }

        return true;
    }

    static bool sendPacket(SOCKET socket, PacketType type, const void* payload, uint32_t payloadSize) {
        if (!PacketProtocol::validatePayload(type, static_cast<const char*>(payload), payloadSize)) {
            return false;
        }
        const std::vector<char> packet = PacketProtocol::buildPacket(type, payload, payloadSize);
        return sendAll(socket, packet.data(), packet.size());
    }

    template <typename T>
    static bool sendStructPacket(SOCKET socket, PacketType type, const T& payload) {
        return sendPacket(socket, type, &payload, static_cast<uint32_t>(sizeof(T)));
    }
};
#endif
