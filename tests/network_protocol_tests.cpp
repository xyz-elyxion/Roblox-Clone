#include "Network.h"
#include "UdpTransport.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>

int main() {
    PacketWelcome welcome{42};
    std::vector<char> packet = PacketProtocol::buildStructPacket(PacketType::WELCOME, welcome);

    std::vector<char> buffer(packet.begin(), packet.begin() + 3);
    PacketProtocol::ParsedPacket parsed;
    assert(PacketProtocol::tryReadFrame(buffer, parsed) == PacketProtocol::ParseStatus::NeedMoreData);

    buffer.insert(buffer.end(), packet.begin() + 3, packet.end());
    assert(PacketProtocol::tryReadFrame(buffer, parsed) == PacketProtocol::ParseStatus::Complete);
    assert(parsed.type == PacketType::WELCOME);
    assert(parsed.payloadSize == sizeof(PacketWelcome));

    PacketWelcome decoded{};
    std::memcpy(&decoded, parsed.payload, sizeof(decoded));
    assert(decoded.playerId == 42);

    PacketProtocol::consumeFrame(buffer, parsed.frameSize);
    assert(buffer.empty());

    std::vector<char> twoPackets = packet;
    twoPackets.insert(twoPackets.end(), packet.begin(), packet.end());
    assert(PacketProtocol::tryReadFrame(twoPackets, parsed) == PacketProtocol::ParseStatus::Complete);
    PacketProtocol::consumeFrame(twoPackets, parsed.frameSize);
    assert(PacketProtocol::tryReadFrame(twoPackets, parsed) == PacketProtocol::ParseStatus::Complete);

    PacketChat chat{};
    chat.playerId = 7;
    std::snprintf(chat.username, sizeof(chat.username), "%s", "ChatUser");
    std::snprintf(chat.message, sizeof(chat.message), "%s", "Hello from chat");
    std::vector<char> chatPacket = PacketProtocol::buildStructPacket(PacketType::CHAT, chat);
    assert(PacketProtocol::tryReadFrame(chatPacket, parsed) == PacketProtocol::ParseStatus::Complete);
    assert(parsed.type == PacketType::CHAT);
    assert(parsed.payloadSize == sizeof(PacketChat));

    PacketChat decodedChat{};
    std::memcpy(&decodedChat, parsed.payload, sizeof(decodedChat));
    assert(decodedChat.playerId == 7);
    assert(PacketProtocol::fixedString(decodedChat.username, sizeof(decodedChat.username)) == "ChatUser");
    assert(PacketProtocol::fixedString(decodedChat.message, sizeof(decodedChat.message)) == "Hello from chat");

    PacketHeader badChatHeader{PacketType::CHAT, static_cast<uint32_t>(sizeof(PacketChat) - 1)};
    std::vector<char> badChat(sizeof(PacketHeader) + sizeof(PacketChat));
    std::memcpy(badChat.data(), &badChatHeader, sizeof(badChatHeader));
    assert(PacketProtocol::tryReadFrame(badChat, parsed) == PacketProtocol::ParseStatus::Invalid);

    PacketPlayerHealth health{};
    health.playerId = 9;
    health.health = 0.0f;
    health.maxHealth = 100.0f;
    health.respawnSeconds = 3;
    std::vector<char> healthPacket =
        PacketProtocol::buildStructPacket(PacketType::PLAYER_HEALTH, health);
    assert(PacketProtocol::tryReadFrame(healthPacket, parsed) == PacketProtocol::ParseStatus::Complete);
    assert(parsed.type == PacketType::PLAYER_HEALTH);
    assert(parsed.payloadSize == sizeof(PacketPlayerHealth));

    PacketPlayerHealth decodedHealth{};
    std::memcpy(&decodedHealth, parsed.payload, sizeof(decodedHealth));
    assert(decodedHealth.playerId == 9);
    assert(decodedHealth.respawnSeconds == 3);

    std::vector<char> respawnRequest =
        PacketProtocol::buildPacket(PacketType::RESPAWN_REQUEST, nullptr, 0);
    assert(PacketProtocol::tryReadFrame(respawnRequest, parsed) == PacketProtocol::ParseStatus::Complete);
    assert(parsed.type == PacketType::RESPAWN_REQUEST);
    assert(parsed.payloadSize == 0);
    assert(!PacketProtocol::validatePayload(PacketType::RESPAWN_REQUEST, nullptr, 1));

    PacketVehicleState car{};
    car.partIndex = 3;
    car.driverId = 7;
    car.position = rblxmath::vec3(1.0f, 2.0f, 3.0f);
    car.yaw = 45.0f;
    car.speed = 12.5f;
    std::vector<char> carPacket = PacketProtocol::buildStructPacket(PacketType::VEHICLE_STATE, car);
    assert(PacketProtocol::tryReadFrame(carPacket, parsed) == PacketProtocol::ParseStatus::Complete);
    assert(parsed.type == PacketType::VEHICLE_STATE);
    assert(parsed.payloadSize == sizeof(PacketVehicleState));

    PacketVehicleState decodedCar{};
    std::memcpy(&decodedCar, parsed.payload, sizeof(decodedCar));
    assert(decodedCar.partIndex == 3);
    assert(decodedCar.driverId == 7);
    assert(decodedCar.speed == 12.5f);

    PacketVehicleInput carInput{};
    carInput.playerId = 7;
    carInput.throttle = 1.0f;
    carInput.steer = -1.0f;
    std::vector<char> inputPacket =
        PacketProtocol::buildStructPacket(PacketType::VEHICLE_INPUT, carInput);
    assert(PacketProtocol::tryReadFrame(inputPacket, parsed) == PacketProtocol::ParseStatus::Complete);
    assert(parsed.type == PacketType::VEHICLE_INPUT);
    assert(parsed.payloadSize == sizeof(PacketVehicleInput));

    std::vector<char> togglePacket =
        PacketProtocol::buildPacket(PacketType::VEHICLE_TOGGLE, nullptr, 0);
    assert(PacketProtocol::tryReadFrame(togglePacket, parsed) == PacketProtocol::ParseStatus::Complete);
    assert(parsed.type == PacketType::VEHICLE_TOGGLE);
    assert(parsed.payloadSize == 0);
    assert(!PacketProtocol::validatePayload(PacketType::VEHICLE_INPUT, nullptr, 0));

    // A part the script builds after a client joined, and one it destroys there.
    PacketPartState partState{};
    partState.partIndex = 1512;
    partState.data.flags = 8;
    partState.data.size = rblxmath::vec3(1.0f, 2.0f, 3.0f);
    std::vector<char> partStatePacket =
        PacketProtocol::buildStructPacket(PacketType::PART_STATE, partState);
    assert(PacketProtocol::tryReadFrame(partStatePacket, parsed) == PacketProtocol::ParseStatus::Complete);
    assert(parsed.type == PacketType::PART_STATE);
    assert(parsed.payloadSize == sizeof(PacketPartState));
    assert(!PacketProtocol::validatePayload(PacketType::PART_STATE, nullptr, 0));

    PacketPartRemove partRemove{};
    partRemove.partIndex = 7;
    std::vector<char> partRemovePacket =
        PacketProtocol::buildStructPacket(PacketType::PART_REMOVE, partRemove);
    assert(PacketProtocol::tryReadFrame(partRemovePacket, parsed) == PacketProtocol::ParseStatus::Complete);
    assert(parsed.type == PacketType::PART_REMOVE);
    assert(parsed.payloadSize == sizeof(PacketPartRemove));
    assert(!PacketProtocol::validatePayload(PacketType::PART_REMOVE, nullptr, 0));

    // The world's air, which the client cannot read from a .world file because
    // it is only ever given the parts, so the server sends it on join.
    PacketWorldEnvironment environment{};
    environment.fogColor = rblxmath::vec3(0.04f, 0.04f, 0.05f);
    environment.fogDensity = 0.032f;
    std::vector<char> environmentPacket =
        PacketProtocol::buildStructPacket(PacketType::WORLD_ENVIRONMENT, environment);
    assert(PacketProtocol::tryReadFrame(environmentPacket, parsed) == PacketProtocol::ParseStatus::Complete);
    assert(parsed.type == PacketType::WORLD_ENVIRONMENT);
    assert(parsed.payloadSize == sizeof(PacketWorldEnvironment));
    assert(!PacketProtocol::validatePayload(PacketType::WORLD_ENVIRONMENT, nullptr, 0));
    assert(!PacketProtocol::validatePayload(PacketType::WORLD_ENVIRONMENT, environmentPacket.data(),
                                            static_cast<uint32_t>(sizeof(PacketWorldEnvironment) - 1)));

    // A world snapshot is one frame, and the largest legal world has to fit in
    // it. A script that builds a hundred rooms sends a few hundred parts' worth
    // of data in one go, so a frame the size of a single datagram is not enough.
    // Nothing carries a frame larger than the transport's message limit either.
    static_assert(PacketProtocol::MAX_PAYLOAD_SIZE <= rblx::net::kMaxMessageSize,
                  "a frame has to fit in one transport message");

    std::vector<char> snapshot(sizeof(uint32_t) + PacketProtocol::MAX_WORLD_PARTS * sizeof(PartData), 0);
    const uint32_t snapshotParts = PacketProtocol::MAX_WORLD_PARTS;
    std::memcpy(snapshot.data(), &snapshotParts, sizeof(snapshotParts));
    assert(PacketProtocol::validatePayload(PacketType::WORLD_STATE, snapshot.data(),
                                           static_cast<uint32_t>(snapshot.size())));

    std::vector<char> snapshotFrame = PacketProtocol::buildPacket(
        PacketType::WORLD_STATE, snapshot.data(), static_cast<uint32_t>(snapshot.size()));
    assert(PacketProtocol::tryReadFrame(snapshotFrame, parsed) == PacketProtocol::ParseStatus::Complete);
    assert(parsed.type == PacketType::WORLD_STATE);
    assert(parsed.payloadSize == snapshot.size());

    // The same one-frame rule for a script that moves every part at once.
    std::vector<char> updatePayload(PacketProtocol::MAX_WORLD_UPDATES * sizeof(PartUpdate), 0);
    assert(PacketProtocol::validatePayload(PacketType::WORLD_UPDATE, updatePayload.data(),
                                           static_cast<uint32_t>(updatePayload.size())));

    PacketHeader badHeader{PacketType::WORLD_STATE, PacketProtocol::MAX_PAYLOAD_SIZE + 1};
    std::vector<char> bad(sizeof(PacketHeader));
    std::memcpy(bad.data(), &badHeader, sizeof(badHeader));
    assert(PacketProtocol::tryReadFrame(bad, parsed) == PacketProtocol::ParseStatus::Invalid);

    std::cout << "network_protocol_tests passed\n";
    return 0;
}
