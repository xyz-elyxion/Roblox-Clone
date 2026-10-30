#include "ClientShared.h"

uint64_t steadyMilliseconds() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
}

bool sendClientDatagram(const char* data, size_t size) {
    if (clientSocket == INVALID_SOCKET) return false;
    const int sent = send(clientSocket, data, static_cast<int>(size), 0);
    return sent == static_cast<int>(size);
}

bool sendClientPacket(const std::vector<char>& frame, bool reliable) {
    if (!clientSession) return false;
    return clientSession->sendMessage(frame.data(), frame.size(), reliable);
}

bool sendPlayerStateToServer(const PacketPlayerState& state) {
    return sendClientPacket(PacketProtocol::buildStructPacket(PacketType::PLAYER_STATE, state), false);
}

int receiveGameDatagrams() {
    gameConnectionLost = false;
    if (!clientSession || clientSocket == INVALID_SOCKET) {
        gameConnectionLost = true;
        return 0;
    }

    const uint64_t nowMs = steadyMilliseconds();
    std::vector<std::vector<char>> messages;

    char datagram[2048];
    for (int i = 0; i < kDatagramsPerFrame; ++i) {
        const int received = recv(clientSocket, datagram, sizeof(datagram), 0);
        if (received == SOCKET_ERROR) {
            const int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK) break;
            // An ICMP port-unreachable arrives as WSAECONNRESET on a connected
            // datagram socket; the server may simply not be listening yet.
            if (error == WSAECONNRESET) continue;
            std::cerr << "Game socket failed: " << error << std::endl;
            gameConnectionLost = true;
            break;
        }
        if (received <= 0) break;
        clientSession->receiveDatagram(datagram, static_cast<size_t>(received), nowMs, messages);
    }

    clientSession->pump(nowMs, sendClientDatagram);

    int appended = 0;
    for (std::vector<char>& message : messages) {
        appended += static_cast<int>(message.size());
        clientReceiveBuffer.insert(clientReceiveBuffer.end(), message.begin(), message.end());
    }

    bool joinFailed = false;
    if (clientSession->timedOut(nowMs, kGameConnectionTimeoutMs)) {
        std::cout << "The game server stopped responding." << std::endl;
        gameConnectionLost = true;
        joinFailed = true;
    }
    if (awaitingWelcome && std::chrono::steady_clock::now() > welcomeDeadline) {
        std::cout << "The game server did not answer the join request." << std::endl;
        gameConnectionLost = true;
        joinFailed = true;
    }
    if (joinFailed && !clientProblemReported) {
        clientProblemReported = true;
        reportClientProblem("join",
                            std::string("The game server at ") + automaticGameServerIp() + ":" +
                                std::to_string(serverPort) +
                                " never answered the join request, so the player could not get "
                                "into the world. The usual cause is a Server.exe built from "
                                "different source than this client: the handshake sizes then "
                                "differ and the server drops the connect packet. Rebuild both "
                                "from the same source before checking the port.");
    }

    return appended;
}

void disconnectFromServerOnly() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
    clientSession.reset();
    awaitingWelcome = false;
    gameConnectionLost = false;
    clientProblemReported = false;
    isConnected = false;
    myPlayerId = -1;
    showConnectionUI = false;
    clientReceiveBuffer.clear();
}

void leaveCurrentGame(std::deque<Part>& parts, PhysicsWorld& physicsWorld) {
    disconnectFromServerOnly();
    if (myCharacter) {
        delete myCharacter;
        myCharacter = nullptr;
    }
    for (auto& [id, remoteChar] : remotePlayers) {
        if (remoteChar) delete remoteChar;
    }
    remotePlayers.clear();
    playerNames.clear();
    playerList.clear();
    targetTransforms.clear();
    startTransforms.clear();
    physicsWorld.reset();
    parts.clear();
    currentState = MENU_PLAY_CHOICE;
    escapeMenuOpen = false;
    escapeMenuShowSettings = false;
    addChatLine("System", "Left the game.", true);
}

bool connectToServer(std::string& error) {
    disconnectFromServerOnly();

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, automaticGameServerIp(), &serverAddr.sin_addr) != 1) {
        error = "Could not find the game server.";
        return false;
    }
    serverAddr.sin_port = htons(static_cast<u_short>(serverPort));

    // The website starts the game server and this client together, so the server
    // may still be booting when we get here. A datagram socket cannot fail on a
    // refused connection, so joining is the CONNECT packet: the session keeps
    // sending it until the server answers with a WELCOME.
    std::cout << "Connecting to game server " << automaticGameServerIp() << ':' << serverPort
              << std::endl;

    clientSocket = Network::createUdpSocket();
    if (clientSocket == INVALID_SOCKET) {
        error = "Could not create network socket.";
        return false;
    }
    Network::setIoBufferSize(clientSocket, 4 << 20);
    Network::setNonBlocking(clientSocket);

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) ==
        SOCKET_ERROR) {
        error = "Could not reach the game server.";
        disconnectFromServerOnly();
        return false;
    }

    clientSession = std::make_unique<rblx::net::UdpSession>();

    PacketConnect pkt;
    std::memset(&pkt, 0, sizeof(pkt));
    if (std::strlen(authToken) > 0) {
        copyToBuffer(pkt.username, sizeof(pkt.username), myUsername);
        copyToBuffer(pkt.authToken, sizeof(pkt.authToken), authToken);
    } else {
        copyToBuffer(pkt.username, sizeof(pkt.username), "Guest");
    }

    pkt.protocolRevision = PacketProtocol::kProtocolRevision;
    pkt.packetLayout = PacketProtocol::packetLayoutSignature();

    if (!sendClientPacket(PacketProtocol::buildStructPacket(PacketType::CONNECT, pkt), true)) {
        error = "Could not send the join packet.";
        disconnectFromServerOnly();
        currentState = MENU_PLAY_CHOICE;
        return false;
    }

    isConnected = true;
    currentState = ONLINE;
    awaitingWelcome = true;
    welcomeDeadline = std::chrono::steady_clock::now() +
                      std::chrono::milliseconds(static_cast<long long>(kGameConnectionTimeoutMs));
    showConnectionUI = false;
    addChatLine("System", "Joining online game...", true);
    return true;
}

void sendChatMessage() {
    std::string message = chatInput;
    message.erase(message.begin(), std::find_if(message.begin(), message.end(), [](unsigned char c) { return !std::isspace(c); }));
    message.erase(std::find_if(message.rbegin(), message.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), message.end());
    if (message.empty()) return;

    if (currentState == ONLINE && isConnected && clientSocket != INVALID_SOCKET) {
        PacketChat chat;
        std::memset(&chat, 0, sizeof(chat));
        chat.playerId = myPlayerId;
        copyToBuffer(chat.username, sizeof(chat.username), myUsername);
        copyToBuffer(chat.message, sizeof(chat.message), message);
        if (!sendClientPacket(PacketProtocol::buildStructPacket(PacketType::CHAT, chat), true)) {
            addChatLine("System", "Could not send chat message.", true);
        }
    } else {
        addChatLine("System", "Chat is available after joining an online game.", true);
    }
    chatInput[0] = '\0';
}
