#pragma once

// State and functions shared between the client's translation units:
//
//   Client.cpp     menus, HUD, rendering and the main loop
//   ClientNet.cpp  the game socket, the reliable session and joining/leaving
//
// Every global the networking layer touches is declared here and defined once in
// Client.cpp, so the split stays mechanical and there is no exported API to keep
// in sync beyond this header.

#include <chrono>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Avatar.h"
#include "Camera.h"
#include "Character.h"
#include "Network.h"
#include "Part.h"
#include "PhysicsWorld.h"
#include "UdpTransport.h"

// Which screen the player is on. Stored in `currentState` below.
enum GameState {
    MENU,
    MENU_AUTH_CHOICE,  // First menu: Login/Register/Guest
    MENU_PLAY_CHOICE,  // Second menu: Online/Offline
    MENU_GAMES,        // In-client games browser
    OFFLINE,
    ONLINE
};

// Where a remote player was and where it is heading, for position smoothing.
struct NetworkTransform {
    glm::vec3 position;
    glm::vec3 rotation;  // Euler
    float timestamp;
};

// One line in the chat log.
struct ChatLine {
    std::string username;
    std::string message;
    bool system = false;
};

// One entry in the in-client game browser.
struct DiscoverGame {
    int id = 0;
    std::string title;
    std::string owner;
    std::string description;
    int partsCount = 0;
    int launchCount = 0;
};

// How long the client waits for any sign of life before calling the join failed.
// A join is not answered until the game server has checked the login with the
// website and fetched the avatar, and each of those two HTTP calls may use the
// whole five-second timeout, so ten seconds of patience could expire while the
// server was still working on a join it would have completed.
constexpr uint64_t kGameConnectionTimeoutMs = 20000;
// Datagrams drained per frame: enough for a busy server, bounded so one frame
// cannot spend the whole budget on the socket.
constexpr int kDatagramsPerFrame = 128;

extern GameState currentState;

// Connection state owned by ClientNet.cpp.
extern SOCKET clientSocket;
extern std::unique_ptr<rblx::net::UdpSession> clientSession;
extern bool gameConnectionLost;
extern bool clientProblemReported;
extern bool awaitingWelcome;
extern std::chrono::steady_clock::time_point welcomeDeadline;
extern int myPlayerId;
extern bool isConnected;
extern char myUsername[32];
extern char serverIP[32];
extern int serverPort;
extern char authToken[256];
extern char chatInput[256];
extern bool showConnectionUI;

// World and UI state the networking layer also reads or resets.
extern bool escapeMenuOpen;
extern bool escapeMenuShowSettings;
extern std::map<int, Character*> remotePlayers;
extern std::map<int, std::string> playerNames;
extern std::vector<PlayerInfo> playerList;
extern std::vector<char> clientReceiveBuffer;
extern Character* myCharacter;
extern std::map<int, NetworkTransform> targetTransforms;
extern std::map<int, NetworkTransform> startTransforms;

// UI state shared with ClientUi.cpp.
extern bool showDebugGui;
extern bool shiftLockEnabled;
extern bool invertMouseY;
extern bool showChatHud;
extern bool showPlayerListHud;
extern bool waitingForWebLogin;
extern bool isCursorLocked;
extern bool firstMouse;
extern float deltaTime;
extern Camera camera;
extern AvatarConfig currentAvatar;
extern char loginUsername[32];
extern char loginPassword[64];
extern bool showLoginError;
extern char loginErrorMsg[256];
extern int accountCoins;

// Health and the respawn timer come from the server's PLAYER_HEALTH packets.
// The client shows them and freezes input while dead; it never decides them.
extern bool playerIsDead;
extern float respawnCountdown;
extern int launchGameId;
extern std::vector<ChatLine> chatLines;
extern std::vector<DiscoverGame> discoverGames;
extern bool discoverLoaded;
extern std::string discoverError;

// Defined in ClientUi.cpp, called from the main loop.
void applyModernPlayerStyle();
void loadDiscoverGames();
bool joinDiscoveredGame(const DiscoverGame& game, std::string& error);
void renderAuthMenu(float uiScale);
void renderPlayMenu(float uiScale);
void renderGamesMenu(float uiScale);
void renderGameHud(std::deque<Part>& parts, PhysicsWorld& physicsWorld, float uiScale);
void renderEscapeMenu(std::deque<Part>& parts, PhysicsWorld& physicsWorld, float uiScale);
void setShiftLockEnabled(bool enabled);
void closeEscapeMenu();
void resetClientSettings();

// Defined in ClientNet.cpp.
uint64_t steadyMilliseconds();
bool sendClientDatagram(const char* data, size_t size);
bool sendClientPacket(const std::vector<char>& frame, bool reliable);
bool sendPlayerStateToServer(const PacketPlayerState& state);
int receiveGameDatagrams();
void disconnectFromServerOnly();
void leaveCurrentGame(std::deque<Part>& parts, PhysicsWorld& physicsWorld);
bool connectToServer(std::string& error);
void sendChatMessage();
void requestRespawn();

// Defined in Client.cpp, called from ClientNet.cpp and ClientUi.cpp.
struct GLFWwindow;
std::string automaticWebServerUrl();
bool startAuthCallback(std::string& loginUrl, std::string& error);
std::string openFileDialog(GLFWwindow* window);
void addChatLine(const std::string& username, const std::string& message, bool system = false);
void reportClientProblem(const std::string& stage, const std::string& reason);
const char* automaticGameServerIp();
void copyToBuffer(char* destination, size_t destinationSize, const std::string& value);
