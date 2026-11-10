#include "ClientShared.h"

#include <RBLXWIN/rblxwin.h>
#include "rblxui.h"
#include "Auth.h"
#include "UiScale.h"
#include "UiTheme.h"
#include "WebJson.h"
#include "Vehicle.h"

void applyModernPlayerStyle() {
    RblxUIStyle& style = RblxUI::GetStyle();
    style.WindowRounding = 18.0f;
    style.ChildRounding = 14.0f;
    style.FrameRounding = 10.0f;
    style.PopupRounding = 12.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 10.0f;
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowPadding = RblxVec2(18.0f, 18.0f);
    style.FramePadding = RblxVec2(12.0f, 10.0f);
    style.ItemSpacing = RblxVec2(12.0f, 12.0f);

    // The palette is the project's own, from UiTheme.h. Only the translucency
    // stays local: these panels float over the world and must let it show
    // through, which the tool windows have no reason to do.
    rblx::ui::theme::applyColors(style);
    style.Colors[RblxUICol_WindowBg].w = 0.92f;
    style.Colors[RblxUICol_ChildBg].w = 0.88f;
    style.Colors[RblxUICol_PopupBg].w = 0.96f;
    style.Colors[RblxUICol_Border].w = 0.30f;
}

RblxUIWindowFlags overlayFlags() {
    return RblxUIWindowFlags_NoDecoration |
           RblxUIWindowFlags_NoSavedSettings |
           RblxUIWindowFlags_NoFocusOnAppearing |
           RblxUIWindowFlags_NoNav |
           RblxUIWindowFlags_NoMove;
}

void drawLabel(const char* text) {
    RblxUI::PushStyleColor(RblxUICol_Text, rblx::ui::theme::muted());
    RblxUI::TextUnformatted(text);
    RblxUI::PopStyleColor();
}

// A dark button lights up when the pointer arrives and a pale one has to go the
// other way, or the hover reads as disabled on the light palette.
RblxVec4 shiftColor(const RblxVec4& color, float step) {
    return RblxVec4(std::min(std::max(color.x + step, 0.0f), 1.0f),
                  std::min(std::max(color.y + step, 0.0f), 1.0f),
                  std::min(std::max(color.z + step, 0.0f), 1.0f), color.w);
}

bool modernButton(const char* label, const RblxVec2& size, const RblxVec4& color) {
    const bool light = rblx::ui::theme::isLight();
    RblxUI::PushStyleColor(RblxUICol_Button, color);
    RblxUI::PushStyleColor(RblxUICol_ButtonHovered, shiftColor(color, light ? -0.07f : 0.08f));
    RblxUI::PushStyleColor(RblxUICol_ButtonActive, shiftColor(color, light ? -0.14f : -0.06f));
    bool pressed = RblxUI::Button(label, size);
    RblxUI::PopStyleColor(3);
    return pressed;
}

void renderAuthMenu(float uiScale) {
    const RblxVec2 display = RblxUI::GetIO().DisplaySize;
    const float panelWidth = std::min(display.x - UiScale::Px(40.0f, uiScale), UiScale::Px(520.0f, uiScale));
    const float panelHeight = UiScale::Px(360.0f, uiScale);
    RblxUI::SetNextWindowPos(RblxVec2((display.x - panelWidth) * 0.5f, (display.y - panelHeight) * 0.5f), RblxUICond_Always);
    RblxUI::SetNextWindowSize(RblxVec2(panelWidth, panelHeight), RblxUICond_Always);

    if (RblxUI::Begin("PlayerAuth", nullptr, overlayFlags())) {
        drawLabel("RBLX CLONE PLAYER");
        RblxUI::TextUnformatted("Sign in through the website");
        RblxUI::PushTextWrapPos(RblxUI::GetCursorPosX() + panelWidth - UiScale::Px(48.0f, uiScale));
        RblxUI::TextWrapped("Sign in to continue with your account.");
        RblxUI::PopTextWrapPos();
        RblxUI::Spacing();

        if (std::strlen(authToken) > 0) {
            RblxUI::Text("Signed in as %s", myUsername);
            if (modernButton("Continue", RblxVec2(-1, UiScale::Px(46.0f, uiScale)), rblx::ui::theme::primaryButton())) {
                currentState = MENU_PLAY_CHOICE;
            }
        }

        if (modernButton(waitingForWebLogin ? "Waiting for login" : "Login with website", RblxVec2(-1, UiScale::Px(50.0f, uiScale)), rblx::ui::theme::primaryButton())) {
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

        if (modernButton("Create account", RblxVec2(-1, UiScale::Px(42.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            Auth::openBrowser(automaticWebServerUrl() + "/signup");
        }

        if (modernButton("Play as guest", RblxVec2(-1, UiScale::Px(42.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            authToken[0] = '\0';
            accountCoins = 0;
            copyToBuffer(myUsername, sizeof(myUsername), "Guest");
            currentState = MENU_PLAY_CHOICE;
            showLoginError = false;
        }

        if (showLoginError) {
            RblxUI::PushStyleColor(RblxUICol_Text, rblx::ui::theme::error());
            RblxUI::TextWrapped("%s", loginErrorMsg);
            RblxUI::PopStyleColor();
        }
    }
    RblxUI::End();
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
    const RblxVec2 display = RblxUI::GetIO().DisplaySize;
    const float panelWidth = std::min(display.x - UiScale::Px(40.0f, uiScale), UiScale::Px(620.0f, uiScale));
    const float panelHeight = UiScale::Px(330.0f, uiScale);
    RblxUI::SetNextWindowPos(RblxVec2((display.x - panelWidth) * 0.5f, (display.y - panelHeight) * 0.5f), RblxUICond_Always);
    RblxUI::SetNextWindowSize(RblxVec2(panelWidth, panelHeight), RblxUICond_Always);

    if (RblxUI::Begin("PlayMenu", nullptr, overlayFlags())) {
        drawLabel(std::strlen(authToken) > 0 ? "SIGNED IN" : "GUEST SESSION");
        RblxUI::Text("Welcome, %s", myUsername);
        if (std::strlen(authToken) > 0) {
            RblxUI::Text("%d coins", accountCoins);
        }
        RblxUI::TextWrapped("Browse public games and join without leaving the player.");
        RblxUI::Spacing();

        if (modernButton("Browse games", RblxVec2(-1, UiScale::Px(52.0f, uiScale)), rblx::ui::theme::primaryButton())) {
            currentState = MENU_GAMES;
            if (!discoverLoaded) {
                loadDiscoverGames();
            }
        }

        if (modernButton("Open website", RblxVec2(-1, UiScale::Px(42.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            Auth::openBrowser(automaticWebServerUrl() + "/games");
            showLoginError = false;
        }

        if (modernButton("Switch account", RblxVec2(-1, UiScale::Px(42.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            currentState = MENU_AUTH_CHOICE;
            showLoginError = false;
        }

        if (showLoginError) {
            RblxUI::PushStyleColor(RblxUICol_Text, rblx::ui::theme::error());
            RblxUI::TextWrapped("%s", loginErrorMsg);
            RblxUI::PopStyleColor();
        }
    }
    RblxUI::End();
}

void renderGamesMenu(float uiScale) {
    const RblxVec2 display = RblxUI::GetIO().DisplaySize;
    const float panelWidth = std::min(display.x - UiScale::Px(40.0f, uiScale), UiScale::Px(760.0f, uiScale));
    const float panelHeight = std::min(display.y - UiScale::Px(40.0f, uiScale), UiScale::Px(620.0f, uiScale));
    RblxUI::SetNextWindowPos(RblxVec2((display.x - panelWidth) * 0.5f, (display.y - panelHeight) * 0.5f), RblxUICond_Always);
    RblxUI::SetNextWindowSize(RblxVec2(panelWidth, panelHeight), RblxUICond_Always);

    if (RblxUI::Begin("GamesMenu", nullptr, overlayFlags())) {
        drawLabel("DISCOVER");
        RblxUI::Text("Games");
        RblxUI::PushStyleColor(RblxUICol_Text, rblx::ui::theme::muted());
        RblxUI::TextWrapped("%s", discoverLoaded
                                    ? "Public games from the website. Press Play to join one."
                                    : "Loading public games from the website...");
        RblxUI::PopStyleColor();
        RblxUI::Spacing();

        if (modernButton("Refresh", RblxVec2(UiScale::Px(110.0f, uiScale), UiScale::Px(34.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            loadDiscoverGames();
        }
        RblxUI::SameLine();
        if (modernButton("Back", RblxVec2(UiScale::Px(90.0f, uiScale), UiScale::Px(34.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            currentState = MENU_PLAY_CHOICE;
        }
        RblxUI::Separator();

        const bool signedIn = std::strlen(authToken) > 0;
        if (!signedIn) {
            RblxUI::PushStyleColor(RblxUICol_Text, rblx::ui::theme::warning());
            RblxUI::TextWrapped("Browsing as a guest. Sign in to join a game.");
            RblxUI::PopStyleColor();
            if (modernButton("Sign in", RblxVec2(-1, UiScale::Px(40.0f, uiScale)), rblx::ui::theme::primaryButton())) {
                currentState = MENU_AUTH_CHOICE;
            }
            RblxUI::Separator();
        }

        if (!discoverError.empty()) {
            RblxUI::PushStyleColor(RblxUICol_Text, rblx::ui::theme::error());
            RblxUI::TextWrapped("%s", discoverError.c_str());
            RblxUI::PopStyleColor();
        }

        RblxUI::BeginChild("GameList", RblxVec2(0, 0), false);
        if (discoverGames.empty()) {
            RblxUI::TextWrapped("%s", discoverLoaded ? "No public games have been published yet."
                                                    : "No games loaded yet. Press Refresh to try again.");
        }

        for (const DiscoverGame& game : discoverGames) {
            RblxUI::PushID(game.id);
            const float playWidth = UiScale::Px(84.0f, uiScale);

            RblxUI::Text("%s", game.title.c_str());
            RblxUI::SameLine();
            const float remaining = RblxUI::GetContentRegionAvail().x;
            if (remaining > playWidth + UiScale::Px(12.0f, uiScale)) {
                RblxUI::SetCursorPosX(RblxUI::GetCursorPosX() + remaining - playWidth);
            }
            if (!signedIn) RblxUI::BeginDisabled();
            if (modernButton("Play", RblxVec2(playWidth, UiScale::Px(30.0f, uiScale)), rblx::ui::theme::primaryButton())) {
                std::string joinError;
                if (!joinDiscoveredGame(game, joinError)) {
                    discoverError = joinError;
                }
            }
            if (!signedIn) RblxUI::EndDisabled();

            RblxUI::PushStyleColor(RblxUICol_Text, rblx::ui::theme::muted());
            RblxUI::Text("by %s - %d parts - %d launches",
                        game.owner.empty() ? "unknown" : game.owner.c_str(),
                        game.partsCount,
                        game.launchCount);
            RblxUI::PopStyleColor();
            if (!game.description.empty()) {
                RblxUI::TextWrapped("%s", game.description.c_str());
            }
            RblxUI::Separator();
            RblxUI::PopID();
        }
        RblxUI::EndChild();
    }
    RblxUI::End();
}

void renderTopBar(std::deque<Part>& parts, PhysicsWorld& physicsWorld, float uiScale) {
    const RblxVec2 display = RblxUI::GetIO().DisplaySize;
    const float width = std::min(UiScale::Px(420.0f, uiScale), display.x - UiScale::Px(32.0f, uiScale));
    RblxUI::SetNextWindowPos(RblxVec2((display.x - width) * 0.5f, UiScale::Px(14.0f, uiScale)), RblxUICond_Always);
    RblxUI::SetNextWindowSize(RblxVec2(width, UiScale::Px(58.0f, uiScale)), RblxUICond_Always);
    if (RblxUI::Begin("TopBarHud", nullptr, overlayFlags())) {
        RblxUI::Text("%s", currentState == ONLINE ? "Online" : "Offline");
        RblxUI::SameLine();
        if (modernButton("Leave game", RblxVec2(UiScale::Px(130.0f, uiScale), UiScale::Px(34.0f, uiScale)), rblx::ui::theme::dangerButton())) {
            leaveCurrentGame(parts, physicsWorld);
        }
        RblxUI::SameLine();
        if (modernButton(showDebugGui ? "Hide debug" : "Debug", RblxVec2(UiScale::Px(100.0f, uiScale), UiScale::Px(34.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            showDebugGui = !showDebugGui;
        }
    }
    RblxUI::End();
}

void renderPlayerList(float uiScale) {
    const RblxVec2 display = RblxUI::GetIO().DisplaySize;
    const float margin = UiScale::Px(18.0f, uiScale);
    const float width = std::clamp(display.x * 0.24f, UiScale::Px(230.0f, uiScale), UiScale::Px(340.0f, uiScale));
    const float height = std::clamp(UiScale::Px(92.0f + playerList.size() * 30.0f, uiScale), UiScale::Px(128.0f, uiScale), display.y * 0.46f);

    RblxUI::SetNextWindowPos(RblxVec2(display.x - width - margin, margin), RblxUICond_Always);
    RblxUI::SetNextWindowSize(RblxVec2(width, height), RblxUICond_Always);
    if (RblxUI::Begin("PlayerListHud", nullptr, overlayFlags())) {
        drawLabel("PLAYERS");
        int count = 1;
        for (const auto& player : playerList) {
            if (player.playerId != myPlayerId) ++count;
        }
        RblxUI::Text("%d online", count);
        RblxUI::Separator();
        RblxUI::Text("%s  (You)", myUsername);
        for (const auto& player : playerList) {
            if (player.playerId == myPlayerId) continue;
            const std::string username = PacketProtocol::fixedString(player.username, sizeof(player.username));
            RblxUI::Text("%s", username.empty() ? "Player" : username.c_str());
        }
    }
    RblxUI::End();
}

void renderChat(float uiScale) {
    const RblxVec2 display = RblxUI::GetIO().DisplaySize;
    const float margin = UiScale::Px(18.0f, uiScale);
    const float width = std::clamp(display.x * 0.34f, UiScale::Px(300.0f, uiScale), UiScale::Px(460.0f, uiScale));
    const float height = std::clamp(display.y * 0.30f, UiScale::Px(210.0f, uiScale), UiScale::Px(320.0f, uiScale));

    RblxUI::SetNextWindowPos(RblxVec2(margin, margin), RblxUICond_Always);
    RblxUI::SetNextWindowSize(RblxVec2(width, height), RblxUICond_Always);
    if (RblxUI::Begin("ChatHud", nullptr, overlayFlags())) {
        drawLabel("CHAT");
        RblxUI::BeginChild("ChatMessages", RblxVec2(0, -UiScale::Px(44.0f, uiScale)), false);
        for (const auto& line : chatLines) {
            if (line.system) {
                RblxUI::PushStyleColor(RblxUICol_Text, rblx::ui::theme::text());
                RblxUI::TextWrapped("%s", line.message.c_str());
                RblxUI::PopStyleColor();
            } else {
                RblxUI::TextWrapped("%s: %s", line.username.c_str(), line.message.c_str());
            }
        }
        if (RblxUI::GetScrollY() >= RblxUI::GetScrollMaxY() - 4.0f) {
            RblxUI::SetScrollHereY(1.0f);
        }
        RblxUI::EndChild();

        RblxUI::SetNextItemWidth(-UiScale::Px(72.0f, uiScale));
        bool enterPressed = RblxUI::InputText("##chatInput", chatInput, sizeof(chatInput), RblxUIInputTextFlags_EnterReturnsTrue);
        RblxUI::SameLine();
        if (modernButton("Send", RblxVec2(UiScale::Px(62.0f, uiScale), 0), rblx::ui::theme::primaryButton()) || enterPressed) {
            sendChatMessage();
        }
    }
    RblxUI::End();
}

void renderBottomHud(float uiScale) {
    const RblxVec2 display = RblxUI::GetIO().DisplaySize;
    const float width = std::min(display.x - UiScale::Px(32.0f, uiScale), UiScale::Px(560.0f, uiScale));
    const float height = UiScale::Px(138.0f, uiScale);
    RblxUI::SetNextWindowPos(RblxVec2((display.x - width) * 0.5f, display.y - height - UiScale::Px(18.0f, uiScale)), RblxUICond_Always);
    RblxUI::SetNextWindowSize(RblxVec2(width, height), RblxUICond_Always);
    if (RblxUI::Begin("BottomHud", nullptr, overlayFlags())) {
        const float slotSize = std::min(UiScale::Px(62.0f, uiScale), (width - UiScale::Px(72.0f, uiScale)) / 5.0f);
        const float startX = (width - (slotSize * 5.0f + UiScale::Px(8.0f, uiScale) * 4.0f)) * 0.5f;
        RblxUI::SetCursorPosX(startX);
        for (int i = 0; i < 5; ++i) {
            RblxUI::PushID(i);
            modernButton(std::to_string(i + 1).c_str(), RblxVec2(slotSize, slotSize), rblx::ui::theme::secondaryButton());
            RblxUI::PopID();
            if (i < 4) RblxUI::SameLine();
        }

        RblxUI::Spacing();
        float health = myCharacter ? myCharacter->health : 100.0f;
        float maxHealth = myCharacter ? myCharacter->maxHealth : 100.0f;
        const float fraction = maxHealth > 0.0f ? std::clamp(health / maxHealth, 0.0f, 1.0f) : 0.0f;
        RblxUI::PushStyleColor(RblxUICol_PlotHistogram, fraction > 0.35f ? rblx::ui::theme::accent() : rblx::ui::theme::error());
        char label[64];
        std::snprintf(label, sizeof(label), "Health %.0f / %.0f", health, maxHealth);
        RblxUI::ProgressBar(fraction, RblxVec2(-1, UiScale::Px(24.0f, uiScale)), label);
        RblxUI::PopStyleColor();

        if (vehicleDriverId == myPlayerId) {
            RblxUI::Text("%d km/h", rblx::vehicle::speedKmh(vehicleSpeed));
        }
    }
    RblxUI::End();
}

void renderDeathOverlay(float uiScale) {
    if (!playerIsDead) return;

    const RblxVec2 display = RblxUI::GetIO().DisplaySize;
    RblxUI::SetNextWindowPos(RblxVec2(0.0f, 0.0f), RblxUICond_Always);
    RblxUI::SetNextWindowSize(display, RblxUICond_Always);
    RblxUI::PushStyleVar(RblxUIStyleVar_WindowPadding, RblxVec2(0.0f, 0.0f));
    RblxUI::PushStyleColor(RblxUICol_WindowBg, rblx::ui::theme::overlay(0.55f));
    RblxUI::Begin("DeathBackdrop", nullptr,
                 RblxUIWindowFlags_NoDecoration | RblxUIWindowFlags_NoSavedSettings |
                 RblxUIWindowFlags_NoMove | RblxUIWindowFlags_NoNav |
                 RblxUIWindowFlags_NoFocusOnAppearing | RblxUIWindowFlags_NoBringToFrontOnFocus);
    RblxUI::End();
    RblxUI::PopStyleColor();
    RblxUI::PopStyleVar();

    const float width = std::min(UiScale::Px(380.0f, uiScale), display.x - UiScale::Px(40.0f, uiScale));
    RblxUI::SetNextWindowPos(RblxVec2((display.x - width) * 0.5f,
                                  display.y * 0.5f - UiScale::Px(48.0f, uiScale)),
                            RblxUICond_Always);
    RblxUI::SetNextWindowSize(RblxVec2(width, UiScale::Px(104.0f, uiScale)), RblxUICond_Always);
    if (RblxUI::Begin("DeathNotice", nullptr, overlayFlags())) {
        RblxUI::PushStyleColor(RblxUICol_Text, rblx::ui::theme::error());
        RblxUI::TextUnformatted("You died");
        RblxUI::PopStyleColor();
        const int seconds = std::max(1, static_cast<int>(respawnCountdown + 0.999f));
        RblxUI::Text("Respawning at the spawn point in %d...", seconds);
    }
    RblxUI::End();
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
        const RblxVec2 display = RblxUI::GetIO().DisplaySize;
        const float width = UiScale::Px(240.0f, uiScale);
        RblxUI::SetNextWindowPos(RblxVec2((display.x - width) * 0.5f,
                                      display.y - UiScale::Px(188.0f, uiScale)),
                                RblxUICond_Always);
        RblxUI::SetNextWindowSize(RblxVec2(width, UiScale::Px(50.0f, uiScale)), RblxUICond_Always);
        if (RblxUI::Begin("VehiclePrompt", nullptr, overlayFlags())) {
            RblxUI::TextUnformatted("Press E to drive");
        }
        RblxUI::End();
    }
    // Drawn last: while dead the server holds the character still, so the player
    // needs to know why the controls stopped responding.
    renderDeathOverlay(uiScale);
}

void setShiftLockEnabled(bool enabled) {
    shiftLockEnabled = enabled;

    RBLXWINwindow* window = rblxwinGetCurrentContext();
    if (!window) return;

    if (enabled) {
        const bool inGame = currentState == ONLINE || currentState == OFFLINE;
        if (inGame && !escapeMenuOpen) {
            isCursorLocked = true;
            rblxwinSetInputMode(window, RBLXWIN_CURSOR, RBLXWIN_CURSOR_DISABLED);
            firstMouse = true;
        }
        return;
    }

    if (isCursorLocked) {
        isCursorLocked = false;
        rblxwinSetInputMode(window, RBLXWIN_CURSOR, RBLXWIN_CURSOR_NORMAL);
    }
}

void closeEscapeMenu() {
    escapeMenuOpen = false;
    escapeMenuShowSettings = false;

    if (!shiftLockEnabled) return;
    if (currentState != ONLINE && currentState != OFFLINE) return;

    RBLXWINwindow* window = rblxwinGetCurrentContext();
    if (!window) return;
    isCursorLocked = true;
    rblxwinSetInputMode(window, RBLXWIN_CURSOR, RBLXWIN_CURSOR_DISABLED);
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

    const RblxVec2 display = RblxUI::GetIO().DisplaySize;

    RblxUI::SetNextWindowPos(RblxVec2(0.0f, 0.0f), RblxUICond_Always);
    RblxUI::SetNextWindowSize(display, RblxUICond_Always);
    RblxUI::PushStyleVar(RblxUIStyleVar_WindowPadding, RblxVec2(0.0f, 0.0f));
    RblxUI::PushStyleColor(RblxUICol_WindowBg, rblx::ui::theme::overlay(0.72f));
    RblxUI::Begin("EscapeMenuBackdrop", nullptr,
                 RblxUIWindowFlags_NoDecoration | RblxUIWindowFlags_NoSavedSettings |
                 RblxUIWindowFlags_NoMove | RblxUIWindowFlags_NoNav |
                 RblxUIWindowFlags_NoFocusOnAppearing | RblxUIWindowFlags_NoBringToFrontOnFocus);
    RblxUI::End();
    RblxUI::PopStyleColor();
    RblxUI::PopStyleVar();

    const float panelWidth = std::min(display.x - UiScale::Px(40.0f, uiScale), UiScale::Px(430.0f, uiScale));
    const float panelHeight = std::min(display.y - UiScale::Px(40.0f, uiScale),
                                       UiScale::Px(escapeMenuShowSettings ? 540.0f : 424.0f, uiScale));
    RblxUI::SetNextWindowPos(RblxVec2((display.x - panelWidth) * 0.5f, (display.y - panelHeight) * 0.5f), RblxUICond_Always);
    RblxUI::SetNextWindowSize(RblxVec2(panelWidth, panelHeight), RblxUICond_Always);

    if (!RblxUI::Begin("EscapeMenu", nullptr, overlayFlags())) {
        RblxUI::End();
        return;
    }

    drawLabel("NOW PLAYING");
    RblxUI::Text("RBLX Clone");
    RblxUI::PushStyleColor(RblxUICol_Text, rblx::ui::theme::muted());
    if (launchGameId > 0) {
        RblxUI::Text("Game %d - %s", launchGameId, currentState == ONLINE ? "online session" : "offline session");
    } else {
        RblxUI::Text("%s", currentState == ONLINE ? "online session" : "offline session");
    }
    RblxUI::PopStyleColor();
    RblxUI::Separator();

    if (!escapeMenuShowSettings) {
        if (modernButton("Resume", RblxVec2(-1, UiScale::Px(52.0f, uiScale)), rblx::ui::theme::primaryButton())) {
            closeEscapeMenu();
        }
        if (modernButton("Reset Character", RblxVec2(-1, UiScale::Px(42.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            // Online, a reset is a death the server performs, so it obeys the
            // same respawn delay as falling out of the world.
            if (currentState == ONLINE) {
                requestRespawn();
            } else if (myCharacter) {
                myCharacter->respawn();
            }
            closeEscapeMenu();
        }
        if (modernButton("Settings", RblxVec2(-1, UiScale::Px(42.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            escapeMenuShowSettings = true;
        }
        if (modernButton("Leave Game", RblxVec2(-1, UiScale::Px(46.0f, uiScale)), rblx::ui::theme::dangerButton())) {
            leaveCurrentGame(parts, physicsWorld);
        }
    } else {
        drawLabel("CONTROLS");
        RblxUI::BeginChild("SettingsList", RblxVec2(0, -UiScale::Px(52.0f, uiScale)), false);

        bool shiftLockValue = shiftLockEnabled;
        if (RblxUI::Checkbox("Shift lock", &shiftLockValue)) {
            setShiftLockEnabled(shiftLockValue);
        }
        RblxUI::TextWrapped("Locks the mouse to the centre and keeps the character facing the camera.");

        drawLabel("MOUSE SENSITIVITY");
        RblxUI::SetNextItemWidth(-1.0f);
        RblxUI::SliderFloat("##mouseSensitivity", &camera.MouseSensitivity, 0.02f, 0.50f, "%.2f");
        RblxUI::Checkbox("Invert mouse Y", &invertMouseY);

        drawLabel("FIELD OF VIEW");
        RblxUI::SetNextItemWidth(-1.0f);
        RblxUI::SliderFloat("##fieldOfView", &camera.Zoom, 30.0f, 90.0f, "%.0f");

        RblxUI::Separator();
        drawLabel("INTERFACE");
        RblxUI::Checkbox("Show chat", &showChatHud);
        RblxUI::Checkbox("Show player list", &showPlayerListHud);
        RblxUI::Checkbox("Debug overlay", &showDebugGui);

        RblxUI::Separator();
        if (modernButton("Reset to defaults", RblxVec2(-1, UiScale::Px(38.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            resetClientSettings();
        }
        RblxUI::EndChild();

        if (modernButton("Back", RblxVec2(-1, UiScale::Px(42.0f, uiScale)), rblx::ui::theme::secondaryButton())) {
            escapeMenuShowSettings = false;
        }
    }

    RblxUI::Separator();
    drawLabel("Press Esc to resume");
    RblxUI::End();
}
