#include "ClientShared.h"

#include <cmath>
#include "ClientMenu.h"

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

namespace {

RblxU32 packedColor(const RblxVec4& color) {
    const auto channel = [](float value) {
        return static_cast<RblxU32>(std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f);
    };
    return channel(color.x) | (channel(color.y) << 8) | (channel(color.z) << 16) |
           (channel(color.w) << 24);
}

RblxVec4 fadedColor(RblxVec4 color, float alpha) {
    color.w = alpha;
    return color;
}

// The marks in the tab strip and on a row's buttons are drawn from circles,
// lines and rectangles: a glyph font would be one more file the packaged build
// carries, and this is all that has to read at eighteen pixels.
enum MenuIcon {
    MENU_ICON_PEOPLE,
    MENU_ICON_SETTINGS,
    MENU_ICON_REPORT,
    MENU_ICON_HELP,
    MENU_ICON_SEARCH,
    MENU_ICON_FLAG,
    MENU_ICON_BLOCK
};

void drawMenuIcon(RblxDrawList* draw, MenuIcon icon, const RblxVec2& center, float size,
                  RblxU32 color) {
    const float half = size * 0.5f;
    const float stroke = std::max(1.0f, size * 0.09f);
    switch (icon) {
        case MENU_ICON_PEOPLE:
            // One figure in front of another: two heads over two shoulders is all
            // that reads at this size.
            draw->AddCircleFilled(RblxVec2(center.x - half * 0.36f, center.y - half * 0.34f),
                                  size * 0.16f, color, 16);
            draw->AddRectFilled(RblxVec2(center.x - half * 0.74f, center.y + half * 0.02f),
                                RblxVec2(center.x + half * 0.02f, center.y + half * 0.54f), color,
                                size * 0.16f);
            draw->AddCircleFilled(RblxVec2(center.x + half * 0.42f, center.y - half * 0.24f),
                                  size * 0.13f, color, 16);
            draw->AddRectFilled(RblxVec2(center.x + half * 0.12f, center.y + half * 0.14f),
                                RblxVec2(center.x + half * 0.74f, center.y + half * 0.54f), color,
                                size * 0.13f);
            break;
        case MENU_ICON_SETTINGS:
            // A ring with eight teeth: a gear at this size is a ring and a comb.
            draw->AddCircle(center, size * 0.26f, color, 20, stroke);
            for (int tooth = 0; tooth < 8; ++tooth) {
                const float angle = static_cast<float>(tooth) * 3.14159265f / 4.0f;
                draw->AddLine(RblxVec2(center.x + std::cos(angle) * size * 0.30f,
                                       center.y + std::sin(angle) * size * 0.30f),
                              RblxVec2(center.x + std::cos(angle) * size * 0.46f,
                                       center.y + std::sin(angle) * size * 0.46f),
                              color, stroke);
            }
            break;
        case MENU_ICON_REPORT:
            draw->AddLine(RblxVec2(center.x - half * 0.36f, center.y - half * 0.64f),
                          RblxVec2(center.x - half * 0.10f, center.y + half * 0.64f), color,
                          stroke);
            draw->AddTriangleFilled(
                RblxVec2(center.x - half * 0.30f, center.y - half * 0.58f),
                RblxVec2(center.x + half * 0.68f, center.y - half * 0.26f),
                RblxVec2(center.x - half * 0.16f, center.y + half * 0.08f), color);
            break;
        case MENU_ICON_HELP: {
            draw->AddCircle(center, size * 0.40f, color, 22, stroke);
            const RblxVec2 mark = RblxUI::CalcTextSize("?");
            draw->AddText(RblxVec2(center.x - mark.x * 0.5f, center.y - mark.y * 0.5f), color,
                          "?");
            break;
        }
        case MENU_ICON_SEARCH:
            draw->AddCircle(RblxVec2(center.x - size * 0.07f, center.y - size * 0.07f),
                            size * 0.25f, color, 20, stroke);
            draw->AddLine(RblxVec2(center.x + size * 0.12f, center.y + size * 0.12f),
                          RblxVec2(center.x + size * 0.34f, center.y + size * 0.34f), color,
                          std::max(1.0f, stroke * 1.2f));
            break;
        case MENU_ICON_FLAG:
            draw->AddLine(RblxVec2(center.x - size * 0.24f, center.y - size * 0.36f),
                          RblxVec2(center.x - size * 0.24f, center.y + size * 0.36f), color,
                          stroke);
            draw->AddRectFilled(RblxVec2(center.x - size * 0.19f, center.y - size * 0.31f),
                                RblxVec2(center.x + size * 0.31f, center.y + size * 0.01f), color,
                                size * 0.04f);
            break;
        case MENU_ICON_BLOCK:
            draw->AddCircle(center, size * 0.30f, color, 22, std::max(1.0f, stroke * 1.2f));
            draw->AddLine(RblxVec2(center.x - size * 0.20f, center.y + size * 0.20f),
                          RblxVec2(center.x + size * 0.20f, center.y - size * 0.20f), color,
                          std::max(1.0f, stroke * 1.2f));
            break;
    }
}

// A tab is an item with no look of its own. The mark under the open one is what
// says which page is showing, so a hovered tab brightens without ever reading as
// selected.
bool escapeTabButton(const char* label, EscapeMenuTab tab, MenuIcon icon, float width,
                     float height, float uiScale) {
    const rblx::ui::theme::Colors& palette = rblx::ui::theme::colors();
    const RblxVec2 origin = RblxUI::GetCursorScreenPos();
    RblxUI::PushID(static_cast<int>(tab));
    const bool pressed = RblxUI::InvisibleButton("##tab", RblxVec2(width, height));
    const bool hovered = RblxUI::IsItemHovered();
    RblxUI::PopID();

    const bool active = escapeMenuTab == tab;
    const RblxVec4 tint = (active || hovered) ? palette.text : palette.muted;
    const float iconSize = UiScale::Px(20.0f, uiScale);
    const float gap = UiScale::Px(10.0f, uiScale);
    const RblxVec2 textSize = RblxUI::CalcTextSize(label);
    const float total = iconSize + gap + textSize.x;
    const float left = origin.x + std::max(0.0f, (width - total) * 0.5f);
    const float middle = origin.y + height * 0.5f;

    RblxDrawList* draw = RblxUI::GetWindowDrawList();
    drawMenuIcon(draw, icon, RblxVec2(left + iconSize * 0.5f, middle), iconSize,
                 packedColor(tint));
    draw->AddText(RblxVec2(left + iconSize + gap, middle - textSize.y * 0.5f),
                  packedColor(tint), label);
    if (active) {
        draw->AddRectFilled(RblxVec2(origin.x, origin.y + height - UiScale::Px(3.0f, uiScale)),
                            RblxVec2(origin.x + width, origin.y + height),
                            packedColor(palette.accentStrong));
    }
    return pressed;
}

// A small square mark beside a player: open their profile, report them, block
// them. The tooltip is the label, and the item's id, so three of them can sit on
// one row without colliding.
bool menuIconButton(MenuIcon icon, const char* tooltip, float size, float uiScale) {
    const rblx::ui::theme::Colors& palette = rblx::ui::theme::colors();
    const RblxVec2 origin = RblxUI::GetCursorScreenPos();
    const float rounding = UiScale::Px(9.0f, uiScale);
    RblxUI::PushID(tooltip);
    const bool pressed = RblxUI::InvisibleButton("##mark", RblxVec2(size, size));
    const bool hovered = RblxUI::IsItemHovered();
    RblxUI::SetItemTooltip("%s", tooltip);
    RblxUI::PopID();

    RblxDrawList* draw = RblxUI::GetWindowDrawList();
    const RblxVec2 far(origin.x + size, origin.y + size);
    draw->AddRectFilled(origin, far,
                        packedColor(hovered ? palette.frameHovered : palette.frame), rounding);
    draw->AddRect(origin, far, packedColor(hovered ? palette.muted : palette.border), rounding, 0,
                  1.0f);
    drawMenuIcon(draw, icon, RblxVec2(origin.x + size * 0.5f, origin.y + size * 0.5f),
                 size * 0.62f, packedColor(hovered ? palette.text : palette.muted));
    return pressed;
}

// The outlined button a player row ends with. A filled button next to three
// quiet marks would be the only thing the eye lands on, and adding a friend is
// one action of several.
bool outlineButton(const char* label, const RblxVec2& size, float uiScale) {
    const rblx::ui::theme::Colors& palette = rblx::ui::theme::colors();
    const RblxVec2 origin = RblxUI::GetCursorScreenPos();
    const float rounding = UiScale::Px(9.0f, uiScale);
    RblxUI::PushID(label);
    const bool pressed = RblxUI::InvisibleButton("##outline", size);
    const bool hovered = RblxUI::IsItemHovered();
    RblxUI::PopID();

    RblxDrawList* draw = RblxUI::GetWindowDrawList();
    const RblxVec2 far(origin.x + size.x, origin.y + size.y);
    if (hovered) {
        draw->AddRectFilled(origin, far, packedColor(fadedColor(palette.text, 0.07f)), rounding);
    }
    draw->AddRect(origin, far, packedColor(hovered ? palette.text : palette.border), rounding, 0,
                  1.0f);
    const RblxVec2 textSize = RblxUI::CalcTextSize(label);
    draw->AddText(RblxVec2(origin.x + (size.x - textSize.x) * 0.5f,
                           origin.y + (size.y - textSize.y) * 0.5f),
                  packedColor(palette.text), label);
    return pressed;
}

// A list row cannot be a 3D portrait, so the tile is the figure a player would
// recognise: their head colour over their shirt colour.
void drawMenuAvatar(RblxDrawList* draw, const RblxVec2& origin, float size,
                    const rblxmath::vec3& head, const rblxmath::vec3& torso, float uiScale) {
    const rblx::ui::theme::Colors& palette = rblx::ui::theme::colors();
    const float rounding = UiScale::Px(9.0f, uiScale);
    const RblxVec2 far(origin.x + size, origin.y + size);
    draw->AddRectFilled(origin, far, packedColor(palette.panel), rounding);
    draw->AddRectFilled(RblxVec2(origin.x + size * 0.20f, origin.y + size * 0.54f),
                        RblxVec2(origin.x + size * 0.80f, origin.y + size * 0.99f),
                        packedColor(RblxVec4(torso.x, torso.y, torso.z, 1.0f)), rounding * 0.7f);
    draw->AddCircleFilled(RblxVec2(origin.x + size * 0.5f, origin.y + size * 0.38f),
                          size * 0.21f, packedColor(RblxVec4(head.x, head.y, head.z, 1.0f)),
                          18);
    draw->AddRect(origin, far, packedColor(palette.border), rounding, 0, 1.0f);
}

// A name resolved to an account id costs one request, and a session asks about
// the same players again, so each answer is kept until the client exits.
std::map<std::string, int> resolvedUserIds;
std::string menuStatus;
bool menuStatusIsError = false;
int reportTargetKind = 0;  // 1 when the open report is about a player
std::string reportTargetName;
char reportReason[240] = "";

void setMenuStatus(const std::string& text, bool isError = false) {
    menuStatus = text;
    menuStatusIsError = isError;
}

// The site names every account by id, and the menu only knows the names in this
// session. A refused lookup is reported once, in the menu's own status line,
// rather than silently doing nothing.
int resolveUserId(const std::string& username) {
    const auto known = resolvedUserIds.find(username);
    if (known != resolvedUserIds.end()) {
        return known->second;
    }
    if (std::strlen(authToken) == 0) {
        setMenuStatus("Sign in to look players up on the site.", true);
        return 0;
    }
    const std::string response =
        Auth::httpGet(automaticWebServerUrl() + "/api/users/search?q=" + username, authToken);
    const int userId = rblx::client_menu::findUserId(response, username);
    if (userId <= 0) {
        setMenuStatus("The site did not find " + username + ".", true);
        return 0;
    }
    resolvedUserIds[username] = userId;
    return userId;
}

void requestFriend(const std::string& username) {
    const int userId = resolveUserId(username);
    if (userId <= 0) {
        return;
    }
    const std::string body = "{\"userId\":" + std::to_string(userId) + "}";
    const rblx::http::Response response = rblx::http::request(
        "POST", automaticWebServerUrl() + "/api/friends/request", body, authToken);
    if (!response.ok()) {
        std::string message = Auth::jsonStringField(response.body, "error");
        if (message.empty()) {
            message = "The site did not accept the friend request.";
        }
        setMenuStatus(message, true);
        return;
    }
    setMenuStatus("Friend request sent to " + username + ".");
}

void blockPlayer(const std::string& username) {
    const int userId = resolveUserId(username);
    if (userId <= 0) {
        return;
    }
    const std::string body =
        "{\"userId\":" + std::to_string(userId) + ",\"blocked\":true}";
    const rblx::http::Response response = rblx::http::request(
        "POST", automaticWebServerUrl() + "/api/blocks", body, authToken);
    if (!response.ok()) {
        std::string message = Auth::jsonStringField(response.body, "error");
        if (message.empty()) {
            message = "The site did not take the block.";
        }
        setMenuStatus(message, true);
        return;
    }
    setMenuStatus("Blocked " + username + " on the site.");
}

void sendReport() {
    const bool aboutPlayer = reportTargetKind == 1 && !reportTargetName.empty();
    int targetId = launchGameId;
    if (aboutPlayer) {
        targetId = resolveUserId(reportTargetName);
        if (targetId <= 0) {
            return;
        }
    } else if (targetId <= 0) {
        setMenuStatus("Open a game before reporting one.", true);
        return;
    }

    const std::string body = std::string("{\"targetType\":\"") +
                             (aboutPlayer ? "user" : "game") +
                             "\",\"targetId\":" + std::to_string(targetId) +
                             ",\"reason\":\"" + Auth::jsonEscape(reportReason) + "\"}";
    const rblx::http::Response response = rblx::http::request(
        "POST", automaticWebServerUrl() + "/api/reports", body, authToken);
    if (!response.ok()) {
        std::string message = Auth::jsonStringField(response.body, "error");
        if (message.empty()) {
            message = "The site did not take the report.";
        }
        setMenuStatus(message, true);
        return;
    }
    reportReason[0] = '\0';
    setMenuStatus(aboutPlayer ? "Report sent about " + reportTargetName + "."
                              : "Report sent about this game.");
}

// The people in this session, the account itself first: that is the list a
// player opens the menu to look at.
void drawPeopleRows(float uiScale) {
    const rblx::ui::theme::Colors& palette = rblx::ui::theme::colors();

    struct Person {
        std::string name;
        rblxmath::vec3 head;
        rblxmath::vec3 torso;
        bool isMe = false;
    };
    std::vector<Person> people;
    people.push_back(Person{std::string(myUsername), currentAvatar.headColor,
                            currentAvatar.torsoColor, true});
    for (const PlayerInfo& player : playerList) {
        const std::string name =
            PacketProtocol::fixedString(player.username, sizeof(player.username));
        if (name.empty() || name == myUsername) {
            continue;
        }
        people.push_back(
            Person{name,
                   rblxmath::vec3(player.headColor[0], player.headColor[1], player.headColor[2]),
                   rblxmath::vec3(player.torsoColor[0], player.torsoColor[1],
                                  player.torsoColor[2]),
                   false});
    }

    const float rowHeight = UiScale::Px(62.0f, uiScale);
    const float tile = UiScale::Px(42.0f, uiScale);
    const float mark = UiScale::Px(34.0f, uiScale);
    const float friendWidth = UiScale::Px(120.0f, uiScale);
    const float friendHeight = UiScale::Px(34.0f, uiScale);
    const float gap = UiScale::Px(8.0f, uiScale);
    const float edge = UiScale::Px(10.0f, uiScale);

    for (std::size_t index = 0; index < people.size(); ++index) {
        const Person& person = people[index];
        RblxUI::PushID(static_cast<int>(index));
        const RblxVec2 origin = RblxUI::GetCursorScreenPos();
        const float width =
            std::max(UiScale::Px(300.0f, uiScale), RblxUI::GetContentRegionAvail().x);
        const RblxVec2 far(origin.x + width, origin.y + rowHeight);

        // The row is a target for the eye before it is a target for the mouse, so
        // the whole strip lights up together.
        const RblxVec2 mouse = RblxUI::GetIO().MousePos;
        const bool hovered = mouse.x >= origin.x && mouse.x <= far.x && mouse.y >= origin.y &&
                             mouse.y <= far.y;
        RblxDrawList* draw = RblxUI::GetWindowDrawList();
        draw->AddRectFilled(origin, far, packedColor(hovered ? palette.frame : palette.panel),
                            UiScale::Px(10.0f, uiScale));
        drawMenuAvatar(draw, RblxVec2(origin.x + edge, origin.y + (rowHeight - tile) * 0.5f), tile,
                       person.head, person.torso, uiScale);

        const float textX = origin.x + edge + tile + UiScale::Px(12.0f, uiScale);
        const RblxVec2 nameSize = RblxUI::CalcTextSize(person.name.c_str());
        const std::string handle = rblx::client_menu::handle(person.name);
        const float lineGap = UiScale::Px(3.0f, uiScale);
        const float textHeight = handle.empty() ? nameSize.y : nameSize.y * 2.0f + lineGap;
        const float textY = origin.y + (rowHeight - textHeight) * 0.5f;
        draw->AddText(RblxVec2(textX, textY), packedColor(palette.text), person.name.c_str());
        if (!handle.empty()) {
            draw->AddText(RblxVec2(textX, textY + nameSize.y + lineGap),
                          packedColor(palette.muted), handle.c_str());
        }

        // Laid out from the right edge inwards, so a row that has no block or
        // report mark never shifts the things that come before it.
        float right = far.x - edge;
        if (!person.isMe) {
            right -= friendWidth;
            RblxUI::SetCursorScreenPos(
                RblxVec2(right, origin.y + (rowHeight - friendHeight) * 0.5f));
            if (outlineButton("Add Friend", RblxVec2(friendWidth, friendHeight), uiScale)) {
                requestFriend(person.name);
            }
            right -= gap;
        }

        const float markY = origin.y + (rowHeight - mark) * 0.5f;
        if (!person.isMe) {
            right -= mark;
            RblxUI::SetCursorScreenPos(RblxVec2(right, markY));
            if (menuIconButton(MENU_ICON_BLOCK, "Block on the site", mark, uiScale)) {
                blockPlayer(person.name);
            }
            right -= gap;

            right -= mark;
            RblxUI::SetCursorScreenPos(RblxVec2(right, markY));
            if (menuIconButton(MENU_ICON_FLAG, "Report this player", mark, uiScale)) {
                reportTargetKind = 1;
                reportTargetName = person.name;
                setMenuStatus(std::string());
                escapeMenuTab = ESCAPE_TAB_REPORT;
            }
            right -= gap;
        }

        right -= mark;
        RblxUI::SetCursorScreenPos(RblxVec2(right, markY));
        if (menuIconButton(MENU_ICON_SEARCH, "Open profile on the site", mark, uiScale)) {
            const std::string url = rblx::client_menu::profileUrl(automaticWebServerUrl(),
                                                                 resolveUserId(person.name));
            if (!url.empty()) {
                Auth::openBrowser(url);
            }
        }

        RblxUI::SetCursorScreenPos(RblxVec2(origin.x, far.y + UiScale::Px(6.0f, uiScale)));
        RblxUI::PopID();
    }
}

}  // namespace

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
    escapeMenuTab = ESCAPE_TAB_PEOPLE;
    menuStatus.clear();

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

    const rblx::ui::theme::Colors& palette = rblx::ui::theme::colors();
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

    const float panelWidth =
        std::min(display.x - UiScale::Px(40.0f, uiScale), UiScale::Px(660.0f, uiScale));
    const float panelHeight =
        std::min(display.y - UiScale::Px(40.0f, uiScale), UiScale::Px(548.0f, uiScale));
    RblxUI::SetNextWindowPos(
        RblxVec2((display.x - panelWidth) * 0.5f, (display.y - panelHeight) * 0.5f),
        RblxUICond_Always);
    RblxUI::SetNextWindowSize(RblxVec2(panelWidth, panelHeight), RblxUICond_Always);

    if (!RblxUI::Begin("EscapeMenu", nullptr, overlayFlags())) {
        RblxUI::End();
        return;
    }

    // The strip is the menu's navigation. It is drawn rather than built from tab
    // items because each page has a mark beside its name, and the open page is
    // marked with a bar across its whole tab, which a tab item cannot give.
    const float stripHeight = UiScale::Px(48.0f, uiScale);
    const float stripWidth = RblxUI::GetContentRegionAvail().x;
    const RblxVec2 stripOrigin = RblxUI::GetCursorScreenPos();

    struct TabSpec {
        const char* label;
        EscapeMenuTab tab;
        MenuIcon icon;
    };
    const TabSpec tabs[] = {
        {"People", ESCAPE_TAB_PEOPLE, MENU_ICON_PEOPLE},
        {"Settings", ESCAPE_TAB_SETTINGS, MENU_ICON_SETTINGS},
        {"Report", ESCAPE_TAB_REPORT, MENU_ICON_REPORT},
        {"Help", ESCAPE_TAB_HELP, MENU_ICON_HELP},
    };
    const int tabCount = static_cast<int>(sizeof(tabs) / sizeof(tabs[0]));
    const float tabWidth = stripWidth / static_cast<float>(tabCount);
    for (int index = 0; index < tabCount; ++index) {
        if (index > 0) RblxUI::SameLine(0.0f, 0.0f);
        if (escapeTabButton(tabs[index].label, tabs[index].tab, tabs[index].icon, tabWidth,
                            stripHeight, uiScale)) {
            if (escapeMenuTab != tabs[index].tab) menuStatus.clear();
            escapeMenuTab = tabs[index].tab;
        }
    }

    // The hairline under the strip belongs to the window, not to the page, so it
    // runs edge to edge behind the mark that says which page is open.
    const RblxVec2 windowPos = RblxUI::GetWindowPos();
    const RblxVec2 windowSize = RblxUI::GetWindowSize();
    const float stripBottom = stripOrigin.y + stripHeight;
    RblxUI::GetWindowDrawList()->AddLine(RblxVec2(windowPos.x, stripBottom),
                                         RblxVec2(windowPos.x + windowSize.x, stripBottom),
                                         packedColor(palette.border), 1.0f);
    RblxUI::SetCursorScreenPos(
        RblxVec2(stripOrigin.x, stripBottom + UiScale::Px(16.0f, uiScale)));

    const float footerHeight = UiScale::Px(56.0f, uiScale);
    if (escapeMenuTab == ESCAPE_TAB_PEOPLE) {
        const float inviteHeight = UiScale::Px(48.0f, uiScale);
        const float inviteWidth = RblxUI::GetContentRegionAvail().x;
        const RblxVec2 inviteOrigin = RblxUI::GetCursorScreenPos();
        RblxUI::PushID("invite");
        const bool pressed =
            RblxUI::InvisibleButton("##invite", RblxVec2(inviteWidth, inviteHeight));
        const bool hovered = RblxUI::IsItemHovered();
        RblxUI::PopID();

        RblxDrawList* draw = RblxUI::GetWindowDrawList();
        const float rounding = UiScale::Px(10.0f, uiScale);
        draw->AddRectFilled(
            inviteOrigin,
            RblxVec2(inviteOrigin.x + inviteWidth, inviteOrigin.y + inviteHeight),
            packedColor(hovered ? palette.frameHovered : palette.frame), rounding);
        const float iconSize = UiScale::Px(20.0f, uiScale);
        drawMenuIcon(draw, MENU_ICON_PEOPLE,
                     RblxVec2(inviteOrigin.x + UiScale::Px(16.0f, uiScale) + iconSize * 0.5f,
                              inviteOrigin.y + inviteHeight * 0.5f),
                     iconSize, packedColor(palette.text));
        const char* inviteLabel = "Invite friends to join";
        const RblxVec2 inviteTextSize = RblxUI::CalcTextSize(inviteLabel);
        draw->AddText(RblxVec2(inviteOrigin.x + UiScale::Px(16.0f, uiScale) + iconSize +
                                   UiScale::Px(12.0f, uiScale),
                               inviteOrigin.y + (inviteHeight - inviteTextSize.y) * 0.5f),
                      packedColor(palette.text), inviteLabel);
        if (pressed) {
            // The game's page is the invitation: opening it starts a server and
            // hands the friend a client, so there is nothing else to send.
            const std::string link =
                rblx::client_menu::inviteUrl(automaticWebServerUrl(), launchGameId);
            if (link.empty()) {
                setMenuStatus("This session has no game page to invite anyone to.", true);
            } else {
                RblxUI::SetClipboardText(link.c_str());
                setMenuStatus("Invite link copied: " + link);
            }
        }

        if (!menuStatus.empty()) {
            RblxUI::PushStyleColor(RblxUICol_Text,
                                   menuStatusIsError ? palette.error : palette.muted);
            RblxUI::TextWrapped("%s", menuStatus.c_str());
            RblxUI::PopStyleColor();
        }

        RblxUI::BeginChild("EscapePeople", RblxVec2(0, -footerHeight), false);
        drawPeopleRows(uiScale);
        RblxUI::EndChild();
    } else if (escapeMenuTab == ESCAPE_TAB_SETTINGS) {
        RblxUI::BeginChild("EscapeSettings", RblxVec2(0, -footerHeight), false);

        drawLabel("CONTROLS");
        bool shiftLockValue = shiftLockEnabled;
        if (RblxUI::Checkbox("Shift lock", &shiftLockValue)) {
            setShiftLockEnabled(shiftLockValue);
        }
        RblxUI::TextWrapped(
            "Locks the mouse to the centre and keeps the character facing the camera.");

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
        if (modernButton("Reset to defaults", RblxVec2(-1, UiScale::Px(38.0f, uiScale)),
                         rblx::ui::theme::secondaryButton())) {
            resetClientSettings();
        }
        RblxUI::EndChild();
    } else if (escapeMenuTab == ESCAPE_TAB_REPORT) {
        RblxUI::BeginChild("EscapeReport", RblxVec2(0, -footerHeight), false);

        drawLabel("WHAT TO REPORT");
        if (reportTargetKind == 1 && !reportTargetName.empty()) {
            RblxUI::Text("Player %s", reportTargetName.c_str());
        } else if (launchGameId > 0) {
            RblxUI::Text("Game %d", launchGameId);
        } else {
            RblxUI::TextUnformatted("This session");
        }
        if (reportTargetKind == 1) {
            if (modernButton("Report the game instead", RblxVec2(-1, UiScale::Px(32.0f, uiScale)),
                             rblx::ui::theme::secondaryButton())) {
                reportTargetKind = 0;
                reportTargetName.clear();
            }
        }

        RblxUI::Spacing();
        drawLabel("WHY");
        RblxUI::InputTextMultiline("##reportReason", reportReason, sizeof(reportReason),
                                   RblxVec2(-1, UiScale::Px(108.0f, uiScale)));
        RblxUI::PushStyleColor(RblxUICol_Text, palette.muted);
        RblxUI::TextWrapped(
            "A moderator reads every report. Say what happened and where, in a sentence or two.");
        RblxUI::PopStyleColor();

        const bool signedIn = std::strlen(authToken) > 0;
        const bool ready = signedIn && std::strlen(reportReason) >= 3;
        if (!ready) RblxUI::BeginDisabled();
        if (modernButton("Send report", RblxVec2(-1, UiScale::Px(40.0f, uiScale)),
                         rblx::ui::theme::primaryButton())) {
            sendReport();
        }
        if (!ready) RblxUI::EndDisabled();
        if (!signedIn) {
            RblxUI::PushStyleColor(RblxUICol_Text, palette.warning);
            RblxUI::TextWrapped("Sign in to send a report: the site has to know who reported it.");
            RblxUI::PopStyleColor();
        }
        if (!menuStatus.empty()) {
            RblxUI::PushStyleColor(RblxUICol_Text,
                                   menuStatusIsError ? palette.error : palette.muted);
            RblxUI::TextWrapped("%s", menuStatus.c_str());
            RblxUI::PopStyleColor();
        }
        RblxUI::EndChild();
    } else {
        RblxUI::BeginChild("EscapeHelp", RblxVec2(0, -footerHeight), false);

        drawLabel("KEYS");
        RblxUI::TextUnformatted("W A S D   move");
        RblxUI::TextUnformatted("Space   jump");
        RblxUI::TextUnformatted("Shift   sprint, or lock the camera with shift lock on");
        RblxUI::TextUnformatted("E   get into or out of a car");
        RblxUI::TextUnformatted("F3   debug overlay");
        RblxUI::TextUnformatted("Esc   this menu, and back to the game");

        RblxUI::Separator();
        drawLabel("THE SITE");
        RblxUI::PushStyleColor(RblxUICol_Text, palette.muted);
        RblxUI::TextWrapped("Your profile, your friends and every published game live on the "
                            "website, and the buttons in this menu ask that site to do the work.");
        RblxUI::PopStyleColor();
        RblxUI::Spacing();
        if (modernButton("Open the website", RblxVec2(-1, UiScale::Px(34.0f, uiScale)),
                         rblx::ui::theme::secondaryButton())) {
            Auth::openBrowser(automaticWebServerUrl() + "/games");
        }
        if (modernButton("Open my profile", RblxVec2(-1, UiScale::Px(34.0f, uiScale)),
                         rblx::ui::theme::secondaryButton())) {
            // The bare page is the signed-in account's own profile; the site
            // rewrites the address to the one with the id once it has loaded.
            Auth::openBrowser(automaticWebServerUrl() + "/profile");
        }
        RblxUI::EndChild();
    }

    RblxUI::Separator();
    std::string sessionLine =
        launchGameId > 0
            ? "Game " + std::to_string(launchGameId) +
                  (currentState == ONLINE ? " - online session" : " - offline session")
            : std::string(currentState == ONLINE ? "Online session" : "Offline session");
    sessionLine += " - Esc resumes";
    RblxUI::PushStyleColor(RblxUICol_Text, palette.muted);
    RblxUI::Text("%s", sessionLine.c_str());
    RblxUI::PopStyleColor();
    RblxUI::SameLine();

    // The two actions that must stay one click away sit at the right edge, so the
    // eye finds them in the same place on every page.
    const float buttonHeight = UiScale::Px(32.0f, uiScale);
    const float resumeWidth = UiScale::Px(104.0f, uiScale);
    const float leaveWidth = UiScale::Px(116.0f, uiScale);
    const float buttonGap = UiScale::Px(10.0f, uiScale);
    const float available = RblxUI::GetContentRegionAvail().x;
    const float needed = resumeWidth + buttonGap + leaveWidth;
    if (available > needed) {
        RblxUI::SetCursorPosX(RblxUI::GetCursorPosX() + available - needed);
    }
    if (modernButton("Resume", RblxVec2(resumeWidth, buttonHeight),
                     rblx::ui::theme::primaryButton())) {
        closeEscapeMenu();
    }
    RblxUI::SameLine();
    if (modernButton("Leave game", RblxVec2(leaveWidth, buttonHeight),
                     rblx::ui::theme::dangerButton())) {
        RblxUI::End();
        leaveCurrentGame(parts, physicsWorld);
        return;
    }

    RblxUI::End();
}
