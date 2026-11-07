#include "ClientShared.h"

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "Auth.h"
#include "UiScale.h"
#include "UiTheme.h"
#include "WebJson.h"
#include "Vehicle.h"

void applyModernPlayerStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 18.0f;
    style.ChildRounding = 14.0f;
    style.FrameRounding = 10.0f;
    style.PopupRounding = 12.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 10.0f;
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowPadding = ImVec2(18.0f, 18.0f);
    style.FramePadding = ImVec2(12.0f, 10.0f);
    style.ItemSpacing = ImVec2(12.0f, 12.0f);

    // The palette is the project's own, from UiTheme.h. Only the translucency
    // stays local: these panels float over the world and must let it show
    // through, which the tool windows have no reason to do.
    rblx::ui::theme::applyColors(style);
    style.Colors[ImGuiCol_WindowBg].w = 0.92f;
    style.Colors[ImGuiCol_ChildBg].w = 0.88f;
    style.Colors[ImGuiCol_PopupBg].w = 0.96f;
    style.Colors[ImGuiCol_Border].w = 0.30f;
}

ImGuiWindowFlags overlayFlags() {
    return ImGuiWindowFlags_NoDecoration |
           ImGuiWindowFlags_NoSavedSettings |
           ImGuiWindowFlags_NoFocusOnAppearing |
           ImGuiWindowFlags_NoNav |
           ImGuiWindowFlags_NoMove;
}

void drawLabel(const char* text) {
    ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::muted());
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
}

bool modernButton(const char* label, const ImVec2& size, const ImVec4& color) {
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(std::min(color.x + 0.08f, 1.0f), std::min(color.y + 0.08f, 1.0f), std::min(color.z + 0.08f, 1.0f), color.w));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(std::max(color.x - 0.06f, 0.0f), std::max(color.y - 0.06f, 0.0f), std::max(color.z - 0.06f, 0.0f), color.w));
    bool pressed = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);
    return pressed;
}

void renderAuthMenu(float uiScale) {
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float panelWidth = std::min(display.x - UiScale::Px(40.0f, uiScale), UiScale::Px(520.0f, uiScale));
    const float panelHeight = UiScale::Px(360.0f, uiScale);
    ImGui::SetNextWindowPos(ImVec2((display.x - panelWidth) * 0.5f, (display.y - panelHeight) * 0.5f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    if (ImGui::Begin("PlayerAuth", nullptr, overlayFlags())) {
        drawLabel("RBLX CLONE PLAYER");
        ImGui::TextUnformatted("Sign in through the website");
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + panelWidth - UiScale::Px(48.0f, uiScale));
        ImGui::TextWrapped("Sign in to continue with your account.");
        ImGui::PopTextWrapPos();
        ImGui::Spacing();

        if (std::strlen(authToken) > 0) {
            ImGui::Text("Signed in as %s", myUsername);
            if (modernButton("Continue", ImVec2(-1, UiScale::Px(46.0f, uiScale)), rblx::ui::theme::primaryButton())) {
                currentState = MENU_PLAY_CHOICE;
            }
        }

        if (modernButton(waitingForWebLogin ? "Waiting for login" : "Login with website", ImVec2(-1, UiScale::Px(50.0f, uiScale)), rblx::ui::theme::primaryButton())) {
            std::string loginUrl;
            std::string error;
            if (startAuthCallback(loginUrl, error)) {
                Auth::openBrowser(loginUrl);
                showLoginError = false;
            } else {
                showLoginError = true;
                copyToBuffer(loginErrorMsg, sizeof(loginErrorMsg), error);
            }
        }

        if (modernButton("Create account", ImVec2(-1, UiScale::Px(42.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            Auth::openBrowser(automaticWebServerUrl() + "/signup");
        }

        if (modernButton("Play as guest", ImVec2(-1, UiScale::Px(42.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            authToken[0] = '\0';
            accountCoins = 0;
            copyToBuffer(myUsername, sizeof(myUsername), "Guest");
            currentState = MENU_PLAY_CHOICE;
            showLoginError = false;
        }

        if (showLoginError) {
            ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::error());
            ImGui::TextWrapped("%s", loginErrorMsg);
            ImGui::PopStyleColor();
        }
    }
    ImGui::End();
}

void loadDiscoverGames() {
    discoverError.clear();
    const std::string response = Auth::httpGet(automaticWebServerUrl() + "/api/games");
    if (response.empty()) {
        discoverLoaded = false;
        discoverError = "Could not reach the website. Start it, then press Refresh.";
        return;
    }

    std::string parseError;
    const rblx::web::Json root = rblx::web::Json::parse(response, &parseError);
    if (!root.isObject()) {
        discoverLoaded = false;
        discoverError = parseError.empty() ? "The website sent a game list the player could not read."
                                           : parseError;
        return;
    }

    discoverGames.clear();
    const rblx::web::Json& games = root.member("games");
    if (games.isArray()) {
        for (const rblx::web::Json& item : games.elements()) {
            DiscoverGame game;
            game.id = static_cast<int>(item.member("id").asInteger());
            game.title = item.member("title").asString();
            game.owner = item.member("ownerUsername").asString();
            game.description = item.member("description").asString();
            game.partsCount = static_cast<int>(item.member("stats").member("partsCount").asInteger());
            game.launchCount = static_cast<int>(item.member("stats").member("launchCount").asInteger());
            if (game.id > 0 && !game.title.empty()) {
                discoverGames.push_back(game);
            }
        }
    }
    discoverLoaded = true;
}

bool joinDiscoveredGame(const DiscoverGame& game, std::string& error) {
    if (std::strlen(authToken) == 0) {
        error = "Sign in to join a game.";
        return false;
    }

    const std::string response = Auth::httpPost(
        automaticWebServerUrl() + "/api/games/" + std::to_string(game.id) + "/play",
        "{\"launchClient\":false}",
        authToken);
    if (response.empty()) {
        error = "Could not reach the website to start the game.";
        return false;
    }

    std::string parseError;
    const rblx::web::Json root = rblx::web::Json::parse(response, &parseError);
    if (!root.isObject() || !root.member("success").asBool(false)) {
        const std::string message = root.member("error").asString();
        error = !message.empty() ? message : "The website could not start that game.";
        return false;
    }

    const rblx::web::Json& launch = root.member("launch");
    const std::string host = launch.member("host").asString();
    const int port = static_cast<int>(launch.member("port").asInteger());
    if (port <= 0) {
        error = "The website did not report a server port.";
        return false;
    }

    if (!host.empty()) {
        copyToBuffer(serverIP, sizeof(serverIP), host);
    }
    serverPort = port;
    launchGameId = game.id;
    return connectToServer(error);
}

void renderPlayMenu(float uiScale) {
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float panelWidth = std::min(display.x - UiScale::Px(40.0f, uiScale), UiScale::Px(620.0f, uiScale));
    const float panelHeight = UiScale::Px(330.0f, uiScale);
    ImGui::SetNextWindowPos(ImVec2((display.x - panelWidth) * 0.5f, (display.y - panelHeight) * 0.5f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    if (ImGui::Begin("PlayMenu", nullptr, overlayFlags())) {
        drawLabel(std::strlen(authToken) > 0 ? "SIGNED IN" : "GUEST SESSION");
        ImGui::Text("Welcome, %s", myUsername);
        if (std::strlen(authToken) > 0) {
            ImGui::Text("%d coins", accountCoins);
        }
        ImGui::TextWrapped("Browse public games and join without leaving the player.");
        ImGui::Spacing();

        if (modernButton("Browse games", ImVec2(-1, UiScale::Px(52.0f, uiScale)), rblx::ui::theme::primaryButton())) {
            currentState = MENU_GAMES;
            if (!discoverLoaded) {
                loadDiscoverGames();
            }
        }

        if (modernButton("Open website", ImVec2(-1, UiScale::Px(42.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            Auth::openBrowser(automaticWebServerUrl() + "/games");
            showLoginError = false;
        }

        if (modernButton("Switch account", ImVec2(-1, UiScale::Px(42.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            currentState = MENU_AUTH_CHOICE;
            showLoginError = false;
        }

        if (showLoginError) {
            ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::error());
            ImGui::TextWrapped("%s", loginErrorMsg);
            ImGui::PopStyleColor();
        }
    }
    ImGui::End();
}

void renderGamesMenu(float uiScale) {
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float panelWidth = std::min(display.x - UiScale::Px(40.0f, uiScale), UiScale::Px(760.0f, uiScale));
    const float panelHeight = std::min(display.y - UiScale::Px(40.0f, uiScale), UiScale::Px(620.0f, uiScale));
    ImGui::SetNextWindowPos(ImVec2((display.x - panelWidth) * 0.5f, (display.y - panelHeight) * 0.5f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    if (ImGui::Begin("GamesMenu", nullptr, overlayFlags())) {
        drawLabel("DISCOVER");
        ImGui::Text("Games");
        ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::muted());
        ImGui::TextWrapped("%s", discoverLoaded
                                    ? "Public games from the website. Press Play to join one."
                                    : "Loading public games from the website...");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        if (modernButton("Refresh", ImVec2(UiScale::Px(110.0f, uiScale), UiScale::Px(34.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            loadDiscoverGames();
        }
        ImGui::SameLine();
        if (modernButton("Back", ImVec2(UiScale::Px(90.0f, uiScale), UiScale::Px(34.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            currentState = MENU_PLAY_CHOICE;
        }
        ImGui::Separator();

        const bool signedIn = std::strlen(authToken) > 0;
        if (!signedIn) {
            ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::warning());
            ImGui::TextWrapped("Browsing as a guest. Sign in to join a game.");
            ImGui::PopStyleColor();
            if (modernButton("Sign in", ImVec2(-1, UiScale::Px(40.0f, uiScale)), rblx::ui::theme::primaryButton())) {
                currentState = MENU_AUTH_CHOICE;
            }
            ImGui::Separator();
        }

        if (!discoverError.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::error());
            ImGui::TextWrapped("%s", discoverError.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::BeginChild("GameList", ImVec2(0, 0), false);
        if (discoverGames.empty()) {
            ImGui::TextWrapped("%s", discoverLoaded ? "No public games have been published yet."
                                                    : "No games loaded yet. Press Refresh to try again.");
        }

        for (const DiscoverGame& game : discoverGames) {
            ImGui::PushID(game.id);
            const float playWidth = UiScale::Px(84.0f, uiScale);

            ImGui::Text("%s", game.title.c_str());
            ImGui::SameLine();
            const float remaining = ImGui::GetContentRegionAvail().x;
            if (remaining > playWidth + UiScale::Px(12.0f, uiScale)) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + remaining - playWidth);
            }
            if (!signedIn) ImGui::BeginDisabled();
            if (modernButton("Play", ImVec2(playWidth, UiScale::Px(30.0f, uiScale)), rblx::ui::theme::primaryButton())) {
                std::string joinError;
                if (!joinDiscoveredGame(game, joinError)) {
                    discoverError = joinError;
                }
            }
            if (!signedIn) ImGui::EndDisabled();

            ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::muted());
            ImGui::Text("by %s - %d parts - %d launches",
                        game.owner.empty() ? "unknown" : game.owner.c_str(),
                        game.partsCount,
                        game.launchCount);
            ImGui::PopStyleColor();
            if (!game.description.empty()) {
                ImGui::TextWrapped("%s", game.description.c_str());
            }
            ImGui::Separator();
            ImGui::PopID();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void renderTopBar(std::deque<Part>& parts, PhysicsWorld& physicsWorld, float uiScale) {
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float width = std::min(UiScale::Px(420.0f, uiScale), display.x - UiScale::Px(32.0f, uiScale));
    ImGui::SetNextWindowPos(ImVec2((display.x - width) * 0.5f, UiScale::Px(14.0f, uiScale)), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, UiScale::Px(58.0f, uiScale)), ImGuiCond_Always);
    if (ImGui::Begin("TopBarHud", nullptr, overlayFlags())) {
        ImGui::Text("%s", currentState == ONLINE ? "Online" : "Offline");
        ImGui::SameLine();
        if (modernButton("Leave game", ImVec2(UiScale::Px(130.0f, uiScale), UiScale::Px(34.0f, uiScale)), rblx::ui::theme::dangerButton())) {
            leaveCurrentGame(parts, physicsWorld);
        }
        ImGui::SameLine();
        if (modernButton(showDebugGui ? "Hide debug" : "Debug", ImVec2(UiScale::Px(100.0f, uiScale), UiScale::Px(34.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            showDebugGui = !showDebugGui;
        }
    }
    ImGui::End();
}

void renderPlayerList(float uiScale) {
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float margin = UiScale::Px(18.0f, uiScale);
    const float width = std::clamp(display.x * 0.24f, UiScale::Px(230.0f, uiScale), UiScale::Px(340.0f, uiScale));
    const float height = std::clamp(UiScale::Px(92.0f + playerList.size() * 30.0f, uiScale), UiScale::Px(128.0f, uiScale), display.y * 0.46f);

    ImGui::SetNextWindowPos(ImVec2(display.x - width - margin, margin), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
    if (ImGui::Begin("PlayerListHud", nullptr, overlayFlags())) {
        drawLabel("PLAYERS");
        int count = 1;
        for (const auto& player : playerList) {
            if (player.playerId != myPlayerId) ++count;
        }
        ImGui::Text("%d online", count);
        ImGui::Separator();
        ImGui::Text("%s  (You)", myUsername);
        for (const auto& player : playerList) {
            if (player.playerId == myPlayerId) continue;
            const std::string username = PacketProtocol::fixedString(player.username, sizeof(player.username));
            ImGui::Text("%s", username.empty() ? "Player" : username.c_str());
        }
    }
    ImGui::End();
}

void renderChat(float uiScale) {
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float margin = UiScale::Px(18.0f, uiScale);
    const float width = std::clamp(display.x * 0.34f, UiScale::Px(300.0f, uiScale), UiScale::Px(460.0f, uiScale));
    const float height = std::clamp(display.y * 0.30f, UiScale::Px(210.0f, uiScale), UiScale::Px(320.0f, uiScale));

    ImGui::SetNextWindowPos(ImVec2(margin, margin), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
    if (ImGui::Begin("ChatHud", nullptr, overlayFlags())) {
        drawLabel("CHAT");
        ImGui::BeginChild("ChatMessages", ImVec2(0, -UiScale::Px(44.0f, uiScale)), false);
        for (const auto& line : chatLines) {
            if (line.system) {
                ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::text());
                ImGui::TextWrapped("%s", line.message.c_str());
                ImGui::PopStyleColor();
            } else {
                ImGui::TextWrapped("%s: %s", line.username.c_str(), line.message.c_str());
            }
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 4.0f) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();

        ImGui::SetNextItemWidth(-UiScale::Px(72.0f, uiScale));
        bool enterPressed = ImGui::InputText("##chatInput", chatInput, sizeof(chatInput), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        if (modernButton("Send", ImVec2(UiScale::Px(62.0f, uiScale), 0), rblx::ui::theme::primaryButton()) || enterPressed) {
            sendChatMessage();
        }
    }
    ImGui::End();
}

void renderBottomHud(float uiScale) {
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float width = std::min(display.x - UiScale::Px(32.0f, uiScale), UiScale::Px(560.0f, uiScale));
    const float height = UiScale::Px(138.0f, uiScale);
    ImGui::SetNextWindowPos(ImVec2((display.x - width) * 0.5f, display.y - height - UiScale::Px(18.0f, uiScale)), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
    if (ImGui::Begin("BottomHud", nullptr, overlayFlags())) {
        const float slotSize = std::min(UiScale::Px(62.0f, uiScale), (width - UiScale::Px(72.0f, uiScale)) / 5.0f);
        const float startX = (width - (slotSize * 5.0f + UiScale::Px(8.0f, uiScale) * 4.0f)) * 0.5f;
        ImGui::SetCursorPosX(startX);
        for (int i = 0; i < 5; ++i) {
            ImGui::PushID(i);
            modernButton(std::to_string(i + 1).c_str(), ImVec2(slotSize, slotSize), rblx::ui::theme::secondaryButton());
            ImGui::PopID();
            if (i < 4) ImGui::SameLine();
        }

        ImGui::Spacing();
        float health = myCharacter ? myCharacter->health : 100.0f;
        float maxHealth = myCharacter ? myCharacter->maxHealth : 100.0f;
        const float fraction = maxHealth > 0.0f ? std::clamp(health / maxHealth, 0.0f, 1.0f) : 0.0f;
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, fraction > 0.35f ? rblx::ui::theme::accent() : rblx::ui::theme::error());
        char label[64];
        std::snprintf(label, sizeof(label), "Health %.0f / %.0f", health, maxHealth);
        ImGui::ProgressBar(fraction, ImVec2(-1, UiScale::Px(24.0f, uiScale)), label);
        ImGui::PopStyleColor();

        if (vehicleDriverId == myPlayerId) {
            ImGui::Text("%d km/h", rblx::vehicle::speedKmh(vehicleSpeed));
        }
    }
    ImGui::End();
}

void renderDeathOverlay(float uiScale) {
    if (!playerIsDead) return;

    const ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(display, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.03f, 0.05f, 0.55f));
    ImGui::Begin("DeathBackdrop", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav |
                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    const float width = std::min(UiScale::Px(380.0f, uiScale), display.x - UiScale::Px(40.0f, uiScale));
    ImGui::SetNextWindowPos(ImVec2((display.x - width) * 0.5f,
                                  display.y * 0.5f - UiScale::Px(48.0f, uiScale)),
                            ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, UiScale::Px(104.0f, uiScale)), ImGuiCond_Always);
    if (ImGui::Begin("DeathNotice", nullptr, overlayFlags())) {
        ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::error());
        ImGui::TextUnformatted("You died");
        ImGui::PopStyleColor();
        const int seconds = std::max(1, static_cast<int>(respawnCountdown + 0.999f));
        ImGui::Text("Respawning at the spawn point in %d...", seconds);
    }
    ImGui::End();
}

void renderGameHud(std::deque<Part>& parts, PhysicsWorld& physicsWorld, float uiScale) {
    if (escapeMenuOpen) return;
    if ((currentState != ONLINE && currentState != OFFLINE) || !myCharacter) return;
    if (showChatHud) {
        renderChat(uiScale);
    }
    if (currentState == ONLINE && isConnected && showPlayerListHud) {
        renderPlayerList(uiScale);
    }
    renderTopBar(parts, physicsWorld, uiScale);
    renderBottomHud(uiScale);

    if (currentState == ONLINE && isConnected && myCharacter && vehicleDriverId != myPlayerId &&
        vehiclePartIndex >= 0 && vehiclePartIndex < static_cast<int>(parts.size()) &&
        rblx::vehicle::canEnter(parts[vehiclePartIndex].position, myCharacter->getPosition())) {
        const ImVec2 display = ImGui::GetIO().DisplaySize;
        const float width = UiScale::Px(240.0f, uiScale);
        ImGui::SetNextWindowPos(ImVec2((display.x - width) * 0.5f,
                                      display.y - UiScale::Px(188.0f, uiScale)),
                                ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(width, UiScale::Px(50.0f, uiScale)), ImGuiCond_Always);
        if (ImGui::Begin("VehiclePrompt", nullptr, overlayFlags())) {
            ImGui::TextUnformatted("Press E to drive");
        }
        ImGui::End();
    }
    // Drawn last: while dead the server holds the character still, so the player
    // needs to know why the controls stopped responding.
    renderDeathOverlay(uiScale);
}

void setShiftLockEnabled(bool enabled) {
    shiftLockEnabled = enabled;

    GLFWwindow* window = glfwGetCurrentContext();
    if (!window) return;

    if (enabled) {
        const bool inGame = currentState == ONLINE || currentState == OFFLINE;
        if (inGame && !escapeMenuOpen) {
            isCursorLocked = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
        return;
    }

    if (isCursorLocked) {
        isCursorLocked = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void closeEscapeMenu() {
    escapeMenuOpen = false;
    escapeMenuShowSettings = false;

    if (!shiftLockEnabled) return;
    if (currentState != ONLINE && currentState != OFFLINE) return;

    GLFWwindow* window = glfwGetCurrentContext();
    if (!window) return;
    isCursorLocked = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    firstMouse = true;
}

void resetClientSettings() {
    camera.MouseSensitivity = SENSITIVITY;
    camera.Zoom = ZOOM;
    invertMouseY = false;
    showChatHud = true;
    showPlayerListHud = true;
    showDebugGui = false;
    setShiftLockEnabled(false);
}

void renderEscapeMenu(std::deque<Part>& parts, PhysicsWorld& physicsWorld, float uiScale) {
    if (!escapeMenuOpen) return;

    const ImVec2 display = ImGui::GetIO().DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(display, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.03f, 0.05f, 0.72f));
    ImGui::Begin("EscapeMenuBackdrop", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav |
                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    const float panelWidth = std::min(display.x - UiScale::Px(40.0f, uiScale), UiScale::Px(430.0f, uiScale));
    const float panelHeight = std::min(display.y - UiScale::Px(40.0f, uiScale),
                                       UiScale::Px(escapeMenuShowSettings ? 540.0f : 424.0f, uiScale));
    ImGui::SetNextWindowPos(ImVec2((display.x - panelWidth) * 0.5f, (display.y - panelHeight) * 0.5f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    if (!ImGui::Begin("EscapeMenu", nullptr, overlayFlags())) {
        ImGui::End();
        return;
    }

    drawLabel("NOW PLAYING");
    ImGui::Text("RBLX Clone");
    ImGui::PushStyleColor(ImGuiCol_Text, rblx::ui::theme::muted());
    if (launchGameId > 0) {
        ImGui::Text("Game %d - %s", launchGameId, currentState == ONLINE ? "online session" : "offline session");
    } else {
        ImGui::Text("%s", currentState == ONLINE ? "online session" : "offline session");
    }
    ImGui::PopStyleColor();
    ImGui::Separator();

    if (!escapeMenuShowSettings) {
        if (modernButton("Resume", ImVec2(-1, UiScale::Px(52.0f, uiScale)), rblx::ui::theme::primaryButton())) {
            closeEscapeMenu();
        }
        if (modernButton("Reset Character", ImVec2(-1, UiScale::Px(42.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            // Online, a reset is a death the server performs, so it obeys the
            // same respawn delay as falling out of the world.
            if (currentState == ONLINE) {
                requestRespawn();
            } else if (myCharacter) {
                myCharacter->respawn();
            }
            closeEscapeMenu();
        }
        if (modernButton("Settings", ImVec2(-1, UiScale::Px(42.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            escapeMenuShowSettings = true;
        }
        if (modernButton("Leave Game", ImVec2(-1, UiScale::Px(46.0f, uiScale)), rblx::ui::theme::dangerButton())) {
            leaveCurrentGame(parts, physicsWorld);
        }
    } else {
        drawLabel("CONTROLS");
        ImGui::BeginChild("SettingsList", ImVec2(0, -UiScale::Px(52.0f, uiScale)), false);

        bool shiftLockValue = shiftLockEnabled;
        if (ImGui::Checkbox("Shift lock", &shiftLockValue)) {
            setShiftLockEnabled(shiftLockValue);
        }
        ImGui::TextWrapped("Locks the mouse to the centre and keeps the character facing the camera.");

        drawLabel("MOUSE SENSITIVITY");
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::SliderFloat("##mouseSensitivity", &camera.MouseSensitivity, 0.02f, 0.50f, "%.2f");
        ImGui::Checkbox("Invert mouse Y", &invertMouseY);

        drawLabel("FIELD OF VIEW");
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::SliderFloat("##fieldOfView", &camera.Zoom, 30.0f, 90.0f, "%.0f");

        ImGui::Separator();
        drawLabel("INTERFACE");
        ImGui::Checkbox("Show chat", &showChatHud);
        ImGui::Checkbox("Show player list", &showPlayerListHud);
        ImGui::Checkbox("Debug overlay", &showDebugGui);

        ImGui::Separator();
        if (modernButton("Reset to defaults", ImVec2(-1, UiScale::Px(38.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            resetClientSettings();
        }
        ImGui::EndChild();

        if (modernButton("Back", ImVec2(-1, UiScale::Px(42.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            escapeMenuShowSettings = false;
        }
    }

    ImGui::Separator();
    drawLabel("Press Esc to resume");
    ImGui::End();
}
