#include "ClientShared.h"

#include <LINEARWIN/linearwin.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <string>

#include "linearui.h"
#include "Auth.h"
#include "ClientMenu.h"
#include "LogoMark.h"
#include "UiScale.h"
#include "UiTheme.h"
#include "WebJson.h"
#include "Vehicle.h"

void applyModernPlayerStyle() {
    LinearUIStyle& style = LinearUI::GetStyle();
    style.WindowRounding = 18.0f;
    style.ChildRounding = 14.0f;
    style.FrameRounding = 10.0f;
    style.PopupRounding = 12.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 10.0f;
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowPadding = LinearVec2(18.0f, 18.0f);
    style.FramePadding = LinearVec2(12.0f, 10.0f);
    style.ItemSpacing = LinearVec2(12.0f, 12.0f);

    // The palette is the project's own, from UiTheme.h. Only the translucency
    // stays local: these panels float over the world and must let it show
    // through, which the tool windows have no reason to do.
    linear::ui::theme::applyColors(style);
    style.Colors[LinearUICol_WindowBg].w = 0.92f;
    style.Colors[LinearUICol_ChildBg].w = 0.88f;
    style.Colors[LinearUICol_PopupBg].w = 0.96f;
    style.Colors[LinearUICol_Border].w = 0.30f;
}

LinearUIWindowFlags overlayFlags() {
    return LinearUIWindowFlags_NoDecoration |
           LinearUIWindowFlags_NoSavedSettings |
           LinearUIWindowFlags_NoFocusOnAppearing |
           LinearUIWindowFlags_NoNav |
           LinearUIWindowFlags_NoMove;
}

void drawLabel(const char* text) {
    LinearUI::PushStyleColor(LinearUICol_Text, linear::ui::theme::muted());
    LinearUI::TextUnformatted(text);
    LinearUI::PopStyleColor();
}

// A dark button lights up when the pointer arrives and a pale one has to go the
// other way, or the hover reads as disabled on the light palette.
LinearVec4 shiftColor(const LinearVec4& color, float step) {
    return LinearVec4(std::min(std::max(color.x + step, 0.0f), 1.0f),
                  std::min(std::max(color.y + step, 0.0f), 1.0f),
                  std::min(std::max(color.z + step, 0.0f), 1.0f), color.w);
}

bool modernButton(const char* label, const LinearVec2& size, const LinearVec4& color) {
    const bool light = linear::ui::theme::isLight();
    LinearUI::PushStyleColor(LinearUICol_Button, color);
    LinearUI::PushStyleColor(LinearUICol_ButtonHovered, shiftColor(color, light ? -0.07f : 0.08f));
    LinearUI::PushStyleColor(LinearUICol_ButtonActive, shiftColor(color, light ? -0.14f : -0.06f));
    bool pressed = LinearUI::Button(label, size);
    LinearUI::PopStyleColor(3);
    return pressed;
}

namespace {

LinearU32 packedColor(const LinearVec4& color) {
    const auto channel = [](float value) {
        return static_cast<LinearU32>(std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f);
    };
    return channel(color.x) | (channel(color.y) << 8) | (channel(color.z) << 16) |
           (channel(color.w) << 24);
}

LinearVec4 fadedColor(LinearVec4 color, float alpha) {
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
    MENU_ICON_BLOCK,
    MENU_ICON_MENU
};

void drawMenuIcon(LinearDrawList* draw, MenuIcon icon, const LinearVec2& center, float size,
                  LinearU32 color) {
    const float half = size * 0.5f;
    const float stroke = std::max(1.0f, size * 0.09f);
    switch (icon) {
        case MENU_ICON_PEOPLE:
            // One figure in front of another: two heads over two shoulders is all
            // that reads at this size.
            draw->AddCircleFilled(LinearVec2(center.x - half * 0.36f, center.y - half * 0.34f),
                                  size * 0.16f, color, 16);
            draw->AddRectFilled(LinearVec2(center.x - half * 0.74f, center.y + half * 0.02f),
                                LinearVec2(center.x + half * 0.02f, center.y + half * 0.54f), color,
                                size * 0.16f);
            draw->AddCircleFilled(LinearVec2(center.x + half * 0.42f, center.y - half * 0.24f),
                                  size * 0.13f, color, 16);
            draw->AddRectFilled(LinearVec2(center.x + half * 0.12f, center.y + half * 0.14f),
                                LinearVec2(center.x + half * 0.74f, center.y + half * 0.54f), color,
                                size * 0.13f);
            break;
        case MENU_ICON_SETTINGS:
            // A ring with eight teeth: a gear at this size is a ring and a comb.
            draw->AddCircle(center, size * 0.26f, color, 20, stroke);
            for (int tooth = 0; tooth < 8; ++tooth) {
                const float angle = static_cast<float>(tooth) * 3.14159265f / 4.0f;
                draw->AddLine(LinearVec2(center.x + std::cos(angle) * size * 0.30f,
                                       center.y + std::sin(angle) * size * 0.30f),
                              LinearVec2(center.x + std::cos(angle) * size * 0.46f,
                                       center.y + std::sin(angle) * size * 0.46f),
                              color, stroke);
            }
            break;
        case MENU_ICON_REPORT:
            draw->AddLine(LinearVec2(center.x - half * 0.36f, center.y - half * 0.64f),
                          LinearVec2(center.x - half * 0.10f, center.y + half * 0.64f), color,
                          stroke);
            draw->AddTriangleFilled(
                LinearVec2(center.x - half * 0.30f, center.y - half * 0.58f),
                LinearVec2(center.x + half * 0.68f, center.y - half * 0.26f),
                LinearVec2(center.x - half * 0.16f, center.y + half * 0.08f), color);
            break;
        case MENU_ICON_HELP: {
            draw->AddCircle(center, size * 0.40f, color, 22, stroke);
            const LinearVec2 mark = LinearUI::CalcTextSize("?");
            draw->AddText(LinearVec2(center.x - mark.x * 0.5f, center.y - mark.y * 0.5f), color,
                          "?");
            break;
        }
        case MENU_ICON_SEARCH:
            draw->AddCircle(LinearVec2(center.x - size * 0.07f, center.y - size * 0.07f),
                            size * 0.25f, color, 20, stroke);
            draw->AddLine(LinearVec2(center.x + size * 0.12f, center.y + size * 0.12f),
                          LinearVec2(center.x + size * 0.34f, center.y + size * 0.34f), color,
                          std::max(1.0f, stroke * 1.2f));
            break;
        case MENU_ICON_FLAG:
            draw->AddLine(LinearVec2(center.x - size * 0.24f, center.y - size * 0.36f),
                          LinearVec2(center.x - size * 0.24f, center.y + size * 0.36f), color,
                          stroke);
            draw->AddRectFilled(LinearVec2(center.x - size * 0.19f, center.y - size * 0.31f),
                                LinearVec2(center.x + size * 0.31f, center.y + size * 0.01f), color,
                                size * 0.04f);
            break;
        case MENU_ICON_BLOCK:
            draw->AddCircle(center, size * 0.30f, color, 22, std::max(1.0f, stroke * 1.2f));
            draw->AddLine(LinearVec2(center.x - size * 0.20f, center.y + size * 0.20f),
                          LinearVec2(center.x + size * 0.20f, center.y - size * 0.20f), color,
                          std::max(1.0f, stroke * 1.2f));
            break;
        case MENU_ICON_MENU:
            // Three bars, the mark a player already reads as "the menu".
            for (int bar = -1; bar <= 1; ++bar) {
                const float y = center.y + half * 0.44f * static_cast<float>(bar);
                draw->AddLine(LinearVec2(center.x - half * 0.72f, y),
                              LinearVec2(center.x + half * 0.72f, y), color, stroke);
            }
            break;
    }
}

// A tab is an item with no look of its own. The mark under the open one is what
// says which page is showing, so a hovered tab brightens without ever reading as
// selected.
bool escapeTabButton(const char* label, EscapeMenuTab tab, MenuIcon icon, float width,
                     float height, float uiScale) {
    const linear::ui::theme::Colors& palette = linear::ui::theme::colors();
    const LinearVec2 origin = LinearUI::GetCursorScreenPos();
    LinearUI::PushID(static_cast<int>(tab));
    const bool pressed = LinearUI::InvisibleButton("##tab", LinearVec2(width, height));
    const bool hovered = LinearUI::IsItemHovered();
    LinearUI::PopID();

    const bool active = escapeMenuTab == tab;
    const LinearVec4 tint = (active || hovered) ? palette.text : palette.muted;
    const float iconSize = UiScale::Px(20.0f, uiScale);
    const float gap = UiScale::Px(10.0f, uiScale);
    const LinearVec2 textSize = LinearUI::CalcTextSize(label);
    const float total = iconSize + gap + textSize.x;
    const float left = origin.x + std::max(0.0f, (width - total) * 0.5f);
    const float middle = origin.y + height * 0.5f;

    LinearDrawList* draw = LinearUI::GetWindowDrawList();
    drawMenuIcon(draw, icon, LinearVec2(left + iconSize * 0.5f, middle), iconSize,
                 packedColor(tint));
    draw->AddText(LinearVec2(left + iconSize + gap, middle - textSize.y * 0.5f),
                  packedColor(tint), label);
    if (active) {
        draw->AddRectFilled(LinearVec2(origin.x, origin.y + height - UiScale::Px(3.0f, uiScale)),
                            LinearVec2(origin.x + width, origin.y + height),
                            packedColor(palette.accentStrong));
    }
    return pressed;
}

// A small square mark beside a player: open their profile, report them, block
// them. The tooltip is the label, and the item's id, so three of them can sit on
// one row without colliding.
bool menuIconButton(MenuIcon icon, const char* tooltip, float size, float uiScale) {
    const linear::ui::theme::Colors& palette = linear::ui::theme::colors();
    const LinearVec2 origin = LinearUI::GetCursorScreenPos();
    const float rounding = UiScale::Px(9.0f, uiScale);
    LinearUI::PushID(tooltip);
    const bool pressed = LinearUI::InvisibleButton("##mark", LinearVec2(size, size));
    const bool hovered = LinearUI::IsItemHovered();
    LinearUI::SetItemTooltip("%s", tooltip);
    LinearUI::PopID();

    LinearDrawList* draw = LinearUI::GetWindowDrawList();
    const LinearVec2 maxCorner(origin.x + size, origin.y + size);
    draw->AddRectFilled(origin, maxCorner,
                        packedColor(hovered ? palette.frameHovered : palette.frame), rounding);
    draw->AddRect(origin, maxCorner, packedColor(hovered ? palette.muted : palette.border),
                  rounding, 0, 1.0f);
    drawMenuIcon(draw, icon, LinearVec2(origin.x + size * 0.5f, origin.y + size * 0.5f),
                 size * 0.62f, packedColor(hovered ? palette.text : palette.muted));
    return pressed;
}

// The outlined button a player row ends with. A filled button next to three
// quiet marks would be the only thing the eye lands on, and adding a friend is
// one action of several.
bool outlineButton(const char* label, const LinearVec2& size, float uiScale) {
    const linear::ui::theme::Colors& palette = linear::ui::theme::colors();
    const LinearVec2 origin = LinearUI::GetCursorScreenPos();
    const float rounding = UiScale::Px(9.0f, uiScale);
    LinearUI::PushID(label);
    const bool pressed = LinearUI::InvisibleButton("##outline", size);
    const bool hovered = LinearUI::IsItemHovered();
    LinearUI::PopID();

    LinearDrawList* draw = LinearUI::GetWindowDrawList();
    const LinearVec2 maxCorner(origin.x + size.x, origin.y + size.y);
    if (hovered) {
        draw->AddRectFilled(origin, maxCorner, packedColor(fadedColor(palette.text, 0.07f)),
                            rounding);
    }
    draw->AddRect(origin, maxCorner, packedColor(hovered ? palette.text : palette.border), rounding,
                  0, 1.0f);
    const LinearVec2 textSize = LinearUI::CalcTextSize(label);
    draw->AddText(LinearVec2(origin.x + (size.x - textSize.x) * 0.5f,
                           origin.y + (size.y - textSize.y) * 0.5f),
                  packedColor(palette.text), label);
    return pressed;
}

// A list row cannot be a 3D portrait, so the tile is the figure a player would
// recognise: their head colour over their shirt colour.
void drawMenuAvatar(LinearDrawList* draw, const LinearVec2& origin, float size,
                    const linearmath::vec3& head, const linearmath::vec3& torso, float uiScale) {
    const linear::ui::theme::Colors& palette = linear::ui::theme::colors();
    const float rounding = UiScale::Px(9.0f, uiScale);
    const LinearVec2 maxCorner(origin.x + size, origin.y + size);
    draw->AddRectFilled(origin, maxCorner, packedColor(palette.panel), rounding);
    draw->AddRectFilled(LinearVec2(origin.x + size * 0.20f, origin.y + size * 0.54f),
                        LinearVec2(origin.x + size * 0.80f, origin.y + size * 0.99f),
                        packedColor(LinearVec4(torso.x, torso.y, torso.z, 1.0f)), rounding * 0.7f);
    draw->AddCircleFilled(LinearVec2(origin.x + size * 0.5f, origin.y + size * 0.38f),
                          size * 0.21f, packedColor(LinearVec4(head.x, head.y, head.z, 1.0f)),
                          18);
    draw->AddRect(origin, maxCorner, packedColor(palette.border), rounding, 0, 1.0f);
}

// A name resolved to an account id costs one request, and a session asks about
// the same players again, so each answer is kept until the client exits.
std::map<std::string, int> resolvedUserIds;
std::string menuStatus;
bool menuStatusIsError = false;
int reportTargetKind = 0;  // 1 when the open report is about a player
std::string reportTargetName;
char reportReason[240] = "";
// The name typed on the Report page, kept beside the resolved target because a
// text field needs a buffer and a report needs a name.
char reportTargetInput[32] = "";

void setMenuStatus(const std::string& text, bool isError = false) {
    menuStatus = text;
    menuStatusIsError = isError;
}

// One place that points a report at a target: a row's flag and the Report page's
// own field have to agree on who is being reported, whichever was used last.
void setReportTarget(const std::string& username) {
    reportTargetKind = username.empty() ? 0 : 1;
    reportTargetName = username;
    copyToBuffer(reportTargetInput, sizeof(reportTargetInput), username);
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
    const int userId = linear::client_menu::findUserId(response, username);
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
    const linear::http::Response response = linear::http::request(
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
    const linear::http::Response response = linear::http::request(
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
    const linear::http::Response response = linear::http::request(
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

// A row's action icons are laid out from its right edge inwards, so a name long
// enough to reach them is painted first and covered by them: the row then reads as
// a nameless entry whose buttons belong to nobody. Cut the name to the room the
// icons leave and mark the cut.
std::string fitTextToWidth(const std::string& text, float width) {
    if (width <= 0.0f) {
        return std::string();
    }
    if (LinearUI::CalcTextSize(text.c_str()).x <= width) {
        return text;
    }
    const std::string tail = "...";
    std::string cut = text;
    while (!cut.empty() && LinearUI::CalcTextSize((cut + tail).c_str()).x > width) {
        cut.pop_back();
        // Remove a whole UTF-8 sequence rather than half of one.
        while (!cut.empty() && (static_cast<unsigned char>(cut.back()) & 0xC0) == 0x80) {
            cut.pop_back();
        }
    }
    return cut + tail;
}

// The people in this session, the account itself first: that is the list a
// player opens the menu to look at.
void drawPeopleRows(float uiScale) {
    const linear::ui::theme::Colors& palette = linear::ui::theme::colors();

    struct Person {
        std::string name;
        linearmath::vec3 head;
        linearmath::vec3 torso;
        bool isMe = false;
    };
    std::vector<Person> people;
    people.push_back(Person{std::string(myUsername), currentAvatar.headColor,
                            currentAvatar.torsoColor, true});
    for (const PlayerInfo& player : playerList) {
        const std::string name =
            PacketProtocol::fixedString(player.username, sizeof(player.username));
        // The local player is the first row whatever the server calls them. The id
        // is what says so: the client joins before its token has been verified, so
        // its own name arrives from the server under the account name while the
        // menu was still calling it "Guest", and comparing names put the same
        // player in this list twice - once as you, once as a stranger with Add
        // Friend, Block and Report buttons.
        if (name.empty() || player.playerId == myPlayerId) {
            continue;
        }
        people.push_back(
            Person{name,
                   linearmath::vec3(player.headColor[0], player.headColor[1], player.headColor[2]),
                   linearmath::vec3(player.torsoColor[0], player.torsoColor[1],
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
        LinearUI::PushID(static_cast<int>(index));
        const LinearVec2 origin = LinearUI::GetCursorScreenPos();
        const float width =
            std::max(UiScale::Px(300.0f, uiScale), LinearUI::GetContentRegionAvail().x);
        const LinearVec2 maxCorner(origin.x + width, origin.y + rowHeight);

        // The row is a target for the eye before it is a target for the mouse, so
        // the whole strip lights up together.
        const LinearVec2 mouse = LinearUI::GetIO().MousePos;
        const bool hovered = mouse.x >= origin.x && mouse.x <= maxCorner.x && mouse.y >= origin.y &&
                             mouse.y <= maxCorner.y;
        LinearDrawList* draw = LinearUI::GetWindowDrawList();
        draw->AddRectFilled(origin, maxCorner, packedColor(hovered ? palette.frame : palette.panel),
                            UiScale::Px(10.0f, uiScale));
        drawMenuAvatar(draw, LinearVec2(origin.x + edge, origin.y + (rowHeight - tile) * 0.5f), tile,
                       person.head, person.torso, uiScale);

        const float textX = origin.x + edge + tile + UiScale::Px(12.0f, uiScale);
        // What the icons below will take, so the name is cut to what is left
        // rather than painted underneath them.
        float reserved = mark;
        if (!person.isMe) {
            reserved += (mark + gap) * 2 + friendWidth + gap;
        }
        const float nameWidth = std::max(0.0f, (maxCorner.x - edge - reserved - gap) - textX);
        const std::string shownName = fitTextToWidth(person.name, nameWidth);
        const LinearVec2 nameSize = LinearUI::CalcTextSize(shownName.c_str());
        const std::string handle = linear::client_menu::handle(person.name);
        const std::string shownHandle = fitTextToWidth(handle, nameWidth);
        const float lineGap = UiScale::Px(3.0f, uiScale);
        const float textHeight = handle.empty() ? nameSize.y : nameSize.y * 2.0f + lineGap;
        const float textY = origin.y + (rowHeight - textHeight) * 0.5f;
        draw->AddText(LinearVec2(textX, textY), packedColor(palette.text), shownName.c_str());
        if (!shownHandle.empty()) {
            draw->AddText(LinearVec2(textX, textY + nameSize.y + lineGap),
                          packedColor(palette.muted), shownHandle.c_str());
        }

        // Laid out from the right edge inwards, so a row that has no block or
        // report mark never shifts the things that come before it.
        float right = maxCorner.x - edge;
        if (!person.isMe) {
            right -= friendWidth;
            LinearUI::SetCursorScreenPos(
                LinearVec2(right, origin.y + (rowHeight - friendHeight) * 0.5f));
            if (outlineButton("Add Friend", LinearVec2(friendWidth, friendHeight), uiScale)) {
                requestFriend(person.name);
            }
            right -= gap;
        }

        const float markY = origin.y + (rowHeight - mark) * 0.5f;
        if (!person.isMe) {
            right -= mark;
            LinearUI::SetCursorScreenPos(LinearVec2(right, markY));
            if (menuIconButton(MENU_ICON_BLOCK, "Block on the site", mark, uiScale)) {
                blockPlayer(person.name);
            }
            right -= gap;

            right -= mark;
            LinearUI::SetCursorScreenPos(LinearVec2(right, markY));
            if (menuIconButton(MENU_ICON_FLAG, "Report this player", mark, uiScale)) {
                setReportTarget(person.name);
                setMenuStatus(std::string());
                escapeMenuTab = ESCAPE_TAB_REPORT;
            }
            right -= gap;
        }

        right -= mark;
        LinearUI::SetCursorScreenPos(LinearVec2(right, markY));
        if (menuIconButton(MENU_ICON_SEARCH, "Open profile on the site", mark, uiScale)) {
            // The account's own row has no id to look up: the search this would
            // ask leaves the caller out, and /profile is the signed-in account.
            const std::string url =
                person.isMe ? automaticWebServerUrl() + "/profile"
                            : linear::client_menu::profileUrl(automaticWebServerUrl(),
                                                            resolveUserId(person.name));
            if (!url.empty()) {
                Auth::openBrowser(url);
            }
        }

        LinearUI::SetCursorScreenPos(LinearVec2(origin.x, maxCorner.y + UiScale::Px(6.0f, uiScale)));
        // Moving the cursor below the row has to be followed by an item: a window
        // whose last call is a cursor move that grew it fails linearui's assert, and
        // the empty item is what tells the list how tall the row was.
        LinearUI::Dummy(LinearVec2(0.0f, 0.0f));
        LinearUI::PopID();
    }
}

}  // namespace

void renderAuthMenu(float uiScale) {
    const LinearVec2 display = LinearUI::GetIO().DisplaySize;
    const float panelWidth = std::min(display.x - UiScale::Px(40.0f, uiScale), UiScale::Px(520.0f, uiScale));
    const float panelHeight = UiScale::Px(360.0f, uiScale);
    LinearUI::SetNextWindowPos(LinearVec2((display.x - panelWidth) * 0.5f, (display.y - panelHeight) * 0.5f), LinearUICond_Always);
    LinearUI::SetNextWindowSize(LinearVec2(panelWidth, panelHeight), LinearUICond_Always);

    if (LinearUI::Begin("PlayerAuth", nullptr, overlayFlags())) {
        // The mark sits in the panel's corner rather than in the flow: the panel
        // has a fixed height, and a longer sign-in message must not push the
        // buttons out of it.
        const float markSize = UiScale::Px(28.0f, uiScale);
        const LinearVec2 panel = LinearUI::GetWindowPos();
        linear::ui::logo::drawMark(
            LinearUI::GetWindowDrawList(),
            LinearVec2(panel.x + panelWidth - markSize - UiScale::Px(18.0f, uiScale),
                       panel.y + UiScale::Px(16.0f, uiScale)),
            markSize);

        drawLabel("LINEAR PLAYER");
        LinearUI::TextUnformatted("Sign in through the website");
        LinearUI::PushTextWrapPos(LinearUI::GetCursorPosX() + panelWidth - UiScale::Px(48.0f, uiScale));
        LinearUI::TextWrapped("Sign in to continue with your account.");
        LinearUI::PopTextWrapPos();
        LinearUI::Spacing();

        if (std::strlen(authToken) > 0) {
            LinearUI::Text("Signed in as %s", myUsername);
            if (modernButton("Continue", LinearVec2(-1, UiScale::Px(46.0f, uiScale)), linear::ui::theme::primaryButton())) {
                currentState = MENU_PLAY_CHOICE;
            }
        }

        if (modernButton(waitingForWebLogin ? "Waiting for login" : "Login with website", LinearVec2(-1, UiScale::Px(50.0f, uiScale)), linear::ui::theme::primaryButton())) {
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

        if (modernButton("Create account", LinearVec2(-1, UiScale::Px(42.0f, uiScale)), linear::ui::theme::secondaryButton())) {
            Auth::openBrowser(automaticWebServerUrl() + "/signup");
        }

        if (modernButton("Play as guest", LinearVec2(-1, UiScale::Px(42.0f, uiScale)), linear::ui::theme::secondaryButton())) {
            authToken[0] = '\0';
            accountCoins = 0;
            copyToBuffer(myUsername, sizeof(myUsername), "Guest");
            currentState = MENU_PLAY_CHOICE;
            showLoginError = false;
        }

        if (showLoginError) {
            LinearUI::PushStyleColor(LinearUICol_Text, linear::ui::theme::error());
            LinearUI::TextWrapped("%s", loginErrorMsg);
            LinearUI::PopStyleColor();
        }
    }
    LinearUI::End();
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
    const linear::web::Json root = linear::web::Json::parse(response, &parseError);
    if (!root.isObject()) {
        discoverLoaded = false;
        discoverError = parseError.empty() ? "The website sent a game list the player could not read."
                                           : parseError;
        return;
    }

    discoverGames.clear();
    const linear::web::Json& games = root.member("games");
    if (games.isArray()) {
        for (const linear::web::Json& item : games.elements()) {
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
    const linear::web::Json root = linear::web::Json::parse(response, &parseError);
    if (!root.isObject() || !root.member("success").asBool(false)) {
        const std::string message = root.member("error").asString();
        error = !message.empty() ? message : "The website could not start that game.";
        return false;
    }

    const linear::web::Json& launch = root.member("launch");
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
    const LinearVec2 display = LinearUI::GetIO().DisplaySize;
    const float panelWidth = std::min(display.x - UiScale::Px(40.0f, uiScale), UiScale::Px(620.0f, uiScale));
    const float panelHeight = UiScale::Px(330.0f, uiScale);
    LinearUI::SetNextWindowPos(LinearVec2((display.x - panelWidth) * 0.5f, (display.y - panelHeight) * 0.5f), LinearUICond_Always);
    LinearUI::SetNextWindowSize(LinearVec2(panelWidth, panelHeight), LinearUICond_Always);

    if (LinearUI::Begin("PlayMenu", nullptr, overlayFlags())) {
        drawLabel(std::strlen(authToken) > 0 ? "SIGNED IN" : "GUEST SESSION");
        LinearUI::Text("Welcome, %s", myUsername);
        if (std::strlen(authToken) > 0) {
            LinearUI::Text("%d coins", accountCoins);
        }
        LinearUI::TextWrapped("Browse public games and join without leaving the player.");
        LinearUI::Spacing();

        if (modernButton("Browse games", LinearVec2(-1, UiScale::Px(52.0f, uiScale)), linear::ui::theme::primaryButton())) {
            currentState = MENU_GAMES;
            if (!discoverLoaded) {
                loadDiscoverGames();
            }
        }

        if (modernButton("Open website", LinearVec2(-1, UiScale::Px(42.0f, uiScale)), linear::ui::theme::secondaryButton())) {
            Auth::openBrowser(automaticWebServerUrl() + "/games");
            showLoginError = false;
        }

        if (modernButton("Switch account", LinearVec2(-1, UiScale::Px(42.0f, uiScale)), linear::ui::theme::secondaryButton())) {
            currentState = MENU_AUTH_CHOICE;
            showLoginError = false;
        }

        if (showLoginError) {
            LinearUI::PushStyleColor(LinearUICol_Text, linear::ui::theme::error());
            LinearUI::TextWrapped("%s", loginErrorMsg);
            LinearUI::PopStyleColor();
        }
    }
    LinearUI::End();
}

void renderGamesMenu(float uiScale) {
    const LinearVec2 display = LinearUI::GetIO().DisplaySize;
    const float panelWidth = std::min(display.x - UiScale::Px(40.0f, uiScale), UiScale::Px(760.0f, uiScale));
    const float panelHeight = std::min(display.y - UiScale::Px(40.0f, uiScale), UiScale::Px(620.0f, uiScale));
    LinearUI::SetNextWindowPos(LinearVec2((display.x - panelWidth) * 0.5f, (display.y - panelHeight) * 0.5f), LinearUICond_Always);
    LinearUI::SetNextWindowSize(LinearVec2(panelWidth, panelHeight), LinearUICond_Always);

    if (LinearUI::Begin("GamesMenu", nullptr, overlayFlags())) {
        drawLabel("DISCOVER");
        LinearUI::Text("Games");
        LinearUI::PushStyleColor(LinearUICol_Text, linear::ui::theme::muted());
        LinearUI::TextWrapped("%s", discoverLoaded
                                    ? "Public games from the website. Press Play to join one."
                                    : "Loading public games from the website...");
        LinearUI::PopStyleColor();
        LinearUI::Spacing();

        if (modernButton("Refresh", LinearVec2(UiScale::Px(110.0f, uiScale), UiScale::Px(34.0f, uiScale)), linear::ui::theme::secondaryButton())) {
            loadDiscoverGames();
        }
        LinearUI::SameLine();
        if (modernButton("Back", LinearVec2(UiScale::Px(90.0f, uiScale), UiScale::Px(34.0f, uiScale)), linear::ui::theme::secondaryButton())) {
            currentState = MENU_PLAY_CHOICE;
        }
        LinearUI::Separator();

        const bool signedIn = std::strlen(authToken) > 0;
        if (!signedIn) {
            LinearUI::PushStyleColor(LinearUICol_Text, linear::ui::theme::warning());
            LinearUI::TextWrapped("Browsing as a guest. Sign in to join a game.");
            LinearUI::PopStyleColor();
            if (modernButton("Sign in", LinearVec2(-1, UiScale::Px(40.0f, uiScale)), linear::ui::theme::primaryButton())) {
                currentState = MENU_AUTH_CHOICE;
            }
            LinearUI::Separator();
        }

        if (!discoverError.empty()) {
            LinearUI::PushStyleColor(LinearUICol_Text, linear::ui::theme::error());
            LinearUI::TextWrapped("%s", discoverError.c_str());
            LinearUI::PopStyleColor();
        }

        LinearUI::BeginChild("GameList", LinearVec2(0, 0), false);
        if (discoverGames.empty()) {
            LinearUI::TextWrapped("%s", discoverLoaded ? "No public games have been published yet."
                                                    : "No games loaded yet. Press Refresh to try again.");
        }

        for (const DiscoverGame& game : discoverGames) {
            LinearUI::PushID(game.id);
            const float playWidth = UiScale::Px(84.0f, uiScale);

            LinearUI::Text("%s", game.title.c_str());
            LinearUI::SameLine();
            const float remaining = LinearUI::GetContentRegionAvail().x;
            if (remaining > playWidth + UiScale::Px(12.0f, uiScale)) {
                LinearUI::SetCursorPosX(LinearUI::GetCursorPosX() + remaining - playWidth);
            }
            if (!signedIn) LinearUI::BeginDisabled();
            if (modernButton("Play", LinearVec2(playWidth, UiScale::Px(30.0f, uiScale)), linear::ui::theme::primaryButton())) {
                std::string joinError;
                if (!joinDiscoveredGame(game, joinError)) {
                    discoverError = joinError;
                }
            }
            if (!signedIn) LinearUI::EndDisabled();

            LinearUI::PushStyleColor(LinearUICol_Text, linear::ui::theme::muted());
            LinearUI::Text("by %s - %d parts - %d launches",
                        game.owner.empty() ? "unknown" : game.owner.c_str(),
                        game.partsCount,
                        game.launchCount);
            LinearUI::PopStyleColor();
            if (!game.description.empty()) {
                LinearUI::TextWrapped("%s", game.description.c_str());
            }
            LinearUI::Separator();
            LinearUI::PopID();
        }
        LinearUI::EndChild();
    }
    LinearUI::End();
}

void renderTopBar(std::deque<Part>& parts, PhysicsWorld& physicsWorld, float uiScale) {
    const LinearVec2 display = LinearUI::GetIO().DisplaySize;
    const float width = std::min(UiScale::Px(420.0f, uiScale), display.x - UiScale::Px(32.0f, uiScale));
    LinearUI::SetNextWindowPos(LinearVec2((display.x - width) * 0.5f, UiScale::Px(14.0f, uiScale)), LinearUICond_Always);
    LinearUI::SetNextWindowSize(LinearVec2(width, UiScale::Px(58.0f, uiScale)), LinearUICond_Always);
    if (LinearUI::Begin("TopBarHud", nullptr, overlayFlags())) {
        LinearUI::Text("%s", currentState == ONLINE ? "Online" : "Offline");
        LinearUI::SameLine();
        if (modernButton("Leave game", LinearVec2(UiScale::Px(130.0f, uiScale), UiScale::Px(34.0f, uiScale)), linear::ui::theme::dangerButton())) {
            leaveCurrentGame(parts, physicsWorld);
        }
        LinearUI::SameLine();
        if (modernButton(showDebugGui ? "Hide debug" : "Debug", LinearVec2(UiScale::Px(100.0f, uiScale), UiScale::Px(34.0f, uiScale)), linear::ui::theme::secondaryButton())) {
            showDebugGui = !showDebugGui;
        }
    }
    LinearUI::End();
}

void renderPlayerList(float uiScale) {
    const LinearVec2 display = LinearUI::GetIO().DisplaySize;
    const float margin = UiScale::Px(18.0f, uiScale);
    const float width = std::clamp(display.x * 0.24f, UiScale::Px(230.0f, uiScale), UiScale::Px(340.0f, uiScale));
    const float height = std::clamp(UiScale::Px(92.0f + playerList.size() * 30.0f, uiScale), UiScale::Px(128.0f, uiScale), display.y * 0.46f);

    LinearUI::SetNextWindowPos(LinearVec2(display.x - width - margin, margin), LinearUICond_Always);
    LinearUI::SetNextWindowSize(LinearVec2(width, height), LinearUICond_Always);
    if (LinearUI::Begin("PlayerListHud", nullptr, overlayFlags())) {
        drawLabel("PLAYERS");
        int count = 1;
        for (const auto& player : playerList) {
            if (player.playerId != myPlayerId) ++count;
        }
        LinearUI::Text("%d online", count);
        LinearUI::Separator();
        LinearUI::Text("%s  (You)", myUsername);
        for (const auto& player : playerList) {
            if (player.playerId == myPlayerId) continue;
            const std::string username = PacketProtocol::fixedString(player.username, sizeof(player.username));
            LinearUI::Text("%s", username.empty() ? "Player" : username.c_str());
        }
    }
    LinearUI::End();
}

void renderChat(float uiScale) {
    const LinearVec2 display = LinearUI::GetIO().DisplaySize;
    const float margin = UiScale::Px(18.0f, uiScale);
    const float width = std::clamp(display.x * 0.34f, UiScale::Px(300.0f, uiScale), UiScale::Px(460.0f, uiScale));
    const float height = std::clamp(display.y * 0.30f, UiScale::Px(210.0f, uiScale), UiScale::Px(320.0f, uiScale));

    // Below the corner menu button, which owns the corner itself.
    LinearUI::SetNextWindowPos(LinearVec2(margin, margin + UiScale::Px(50.0f, uiScale)),
                             LinearUICond_Always);
    LinearUI::SetNextWindowSize(LinearVec2(width, height), LinearUICond_Always);
    if (LinearUI::Begin("ChatHud", nullptr, overlayFlags())) {
        drawLabel("CHAT");
        LinearUI::BeginChild("ChatMessages", LinearVec2(0, -UiScale::Px(44.0f, uiScale)), false);
        for (const auto& line : chatLines) {
            if (line.system) {
                LinearUI::PushStyleColor(LinearUICol_Text, linear::ui::theme::text());
                LinearUI::TextWrapped("%s", line.message.c_str());
                LinearUI::PopStyleColor();
            } else {
                LinearUI::TextWrapped("%s: %s", line.username.c_str(), line.message.c_str());
            }
        }
        if (LinearUI::GetScrollY() >= LinearUI::GetScrollMaxY() - 4.0f) {
            LinearUI::SetScrollHereY(1.0f);
        }
        LinearUI::EndChild();

        LinearUI::SetNextItemWidth(-UiScale::Px(72.0f, uiScale));
        bool enterPressed = LinearUI::InputText("##chatInput", chatInput, sizeof(chatInput), LinearUIInputTextFlags_EnterReturnsTrue);
        LinearUI::SameLine();
        if (modernButton("Send", LinearVec2(UiScale::Px(62.0f, uiScale), 0), linear::ui::theme::primaryButton()) || enterPressed) {
            sendChatMessage();
        }
    }
    LinearUI::End();
}

void renderBottomHud(float uiScale) {
    const LinearVec2 display = LinearUI::GetIO().DisplaySize;
    const float width = std::min(display.x - UiScale::Px(32.0f, uiScale), UiScale::Px(560.0f, uiScale));
    const float height = UiScale::Px(138.0f, uiScale);
    LinearUI::SetNextWindowPos(LinearVec2((display.x - width) * 0.5f, display.y - height - UiScale::Px(18.0f, uiScale)), LinearUICond_Always);
    LinearUI::SetNextWindowSize(LinearVec2(width, height), LinearUICond_Always);
    if (LinearUI::Begin("BottomHud", nullptr, overlayFlags())) {
        const float slotSize = std::min(UiScale::Px(62.0f, uiScale), (width - UiScale::Px(72.0f, uiScale)) / 5.0f);
        const float startX = (width - (slotSize * 5.0f + UiScale::Px(8.0f, uiScale) * 4.0f)) * 0.5f;
        LinearUI::SetCursorPosX(startX);
        for (int i = 0; i < 5; ++i) {
            LinearUI::PushID(i);
            modernButton(std::to_string(i + 1).c_str(), LinearVec2(slotSize, slotSize), linear::ui::theme::secondaryButton());
            LinearUI::PopID();
            if (i < 4) LinearUI::SameLine();
        }

        LinearUI::Spacing();
        float health = myCharacter ? myCharacter->health : 100.0f;
        float maxHealth = myCharacter ? myCharacter->maxHealth : 100.0f;
        const float fraction = maxHealth > 0.0f ? std::clamp(health / maxHealth, 0.0f, 1.0f) : 0.0f;
        LinearUI::PushStyleColor(LinearUICol_PlotHistogram, fraction > 0.35f ? linear::ui::theme::accent() : linear::ui::theme::error());
        char label[64];
        std::snprintf(label, sizeof(label), "Health %.0f / %.0f", health, maxHealth);
        LinearUI::ProgressBar(fraction, LinearVec2(-1, UiScale::Px(24.0f, uiScale)), label);
        LinearUI::PopStyleColor();

        if (vehicleDriverId == myPlayerId) {
            LinearUI::Text("%d km/h", linear::vehicle::speedKmh(vehicleSpeed));
        }
    }
    LinearUI::End();
}

void renderDeathOverlay(float uiScale) {
    if (!playerIsDead) return;

    const LinearVec2 display = LinearUI::GetIO().DisplaySize;
    LinearUI::SetNextWindowPos(LinearVec2(0.0f, 0.0f), LinearUICond_Always);
    LinearUI::SetNextWindowSize(display, LinearUICond_Always);
    LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0.0f, 0.0f));
    LinearUI::PushStyleColor(LinearUICol_WindowBg, linear::ui::theme::overlay(0.55f));
    LinearUI::Begin("DeathBackdrop", nullptr,
                 LinearUIWindowFlags_NoDecoration | LinearUIWindowFlags_NoSavedSettings |
                 LinearUIWindowFlags_NoMove | LinearUIWindowFlags_NoNav |
                 LinearUIWindowFlags_NoFocusOnAppearing | LinearUIWindowFlags_NoBringToFrontOnFocus);
    LinearUI::End();
    LinearUI::PopStyleColor();
    LinearUI::PopStyleVar();

    const float width = std::min(UiScale::Px(380.0f, uiScale), display.x - UiScale::Px(40.0f, uiScale));
    LinearUI::SetNextWindowPos(LinearVec2((display.x - width) * 0.5f,
                                  display.y * 0.5f - UiScale::Px(48.0f, uiScale)),
                            LinearUICond_Always);
    LinearUI::SetNextWindowSize(LinearVec2(width, UiScale::Px(104.0f, uiScale)), LinearUICond_Always);
    if (LinearUI::Begin("DeathNotice", nullptr, overlayFlags())) {
        LinearUI::PushStyleColor(LinearUICol_Text, linear::ui::theme::error());
        LinearUI::TextUnformatted("You died");
        LinearUI::PopStyleColor();
        const int seconds = std::max(1, static_cast<int>(respawnCountdown + 0.999f));
        LinearUI::Text("Respawning at the spawn point in %d...", seconds);
    }
    LinearUI::End();
}

// The Esc key and the button in the corner open the same menu, so both come
// through here rather than each doing half of it.
void openEscapeMenu() {
    escapeMenuOpen = true;
    escapeMenuTab = ESCAPE_TAB_PEOPLE;
    menuStatus.clear();
    applyCursorCapture();
}

// Where a game puts its menu: a square in the top-left corner, so a player who
// never learned the Esc key still finds it. Chat sits below it rather than under.
void renderMenuButton(float uiScale) {
    const float margin = UiScale::Px(18.0f, uiScale);
    const float size = UiScale::Px(40.0f, uiScale);
    LinearUI::SetNextWindowPos(LinearVec2(margin, margin), LinearUICond_Always);
    LinearUI::SetNextWindowSize(LinearVec2(size, size), LinearUICond_Always);
    LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0.0f, 0.0f));
    if (LinearUI::Begin("MenuButtonHud", nullptr, overlayFlags())) {
        if (menuIconButton(MENU_ICON_MENU, "Menu (Esc)", size, uiScale)) {
            openEscapeMenu();
        }
    }
    LinearUI::End();
    LinearUI::PopStyleVar();
}

// The first screen: the window is up and the saved sign-in is being checked, so
// the client says what it is doing instead of showing an empty frame.
void renderLoadingScreen(float uiScale) {
    const linear::ui::theme::Colors& palette = linear::ui::theme::colors();
    const LinearVec2 display = LinearUI::GetIO().DisplaySize;
    const float panelWidth =
        std::min(display.x - UiScale::Px(40.0f, uiScale), UiScale::Px(430.0f, uiScale));
    const float panelHeight = UiScale::Px(170.0f, uiScale);
    LinearUI::SetNextWindowPos(
        LinearVec2((display.x - panelWidth) * 0.5f, (display.y - panelHeight) * 0.5f),
        LinearUICond_Always);
    LinearUI::SetNextWindowSize(LinearVec2(panelWidth, panelHeight), LinearUICond_Always);

    if (LinearUI::Begin("LoadingScreen", nullptr, overlayFlags())) {
        drawLabel("LINEAR");
        LinearUI::TextUnformatted("Loading");
        LinearUI::PushStyleColor(LinearUICol_Text, palette.muted);
        LinearUI::TextWrapped("Checking the saved sign-in with the website, then opening the "
                            "menu. A webserver that is not running only means signing in "
                            "again.");
        LinearUI::PopStyleColor();
    }
    LinearUI::End();
}

void renderGameHud(std::deque<Part>& parts, PhysicsWorld& physicsWorld, float uiScale) {
    if (escapeMenuOpen) return;
    if ((currentState != ONLINE && currentState != OFFLINE) || !myCharacter) return;
    renderMenuButton(uiScale);
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
        linear::vehicle::canEnter(parts[vehiclePartIndex].position, myCharacter->getPosition())) {
        const LinearVec2 display = LinearUI::GetIO().DisplaySize;
        const float width = UiScale::Px(240.0f, uiScale);
        LinearUI::SetNextWindowPos(LinearVec2((display.x - width) * 0.5f,
                                      display.y - UiScale::Px(188.0f, uiScale)),
                                LinearUICond_Always);
        LinearUI::SetNextWindowSize(LinearVec2(width, UiScale::Px(50.0f, uiScale)), LinearUICond_Always);
        if (LinearUI::Begin("VehiclePrompt", nullptr, overlayFlags())) {
            LinearUI::TextUnformatted("Press E to drive");
        }
        LinearUI::End();
    }
    // Drawn last: while dead the server holds the character still, so the player
    // needs to know why the controls stopped responding.
    renderDeathOverlay(uiScale);
}

// One place decides whether the mouse is captured, and it decides from the state the
// player is in rather than from whichever event changed it: shift lock, a held right
// button, the Esc menu, and death. The other paths only change that state. Before this,
// every one of them set the platform mode itself, so a teleport correction arriving while
// shift lock was on handed the cursor back to the desktop: it slid across the screen until
// the next mouse move grabbed it again, which hid it and snapped it to the middle, over
// and over for as long as the player moved the mouse.
void applyCursorCapture() {
    LINEARWINwindow* window = linearwinGetCurrentContext();
    if (!window) return;

    const bool inGame = currentState == ONLINE || currentState == OFFLINE;
    // Right click only looks when the view is the thing being clicked: a right click on a
    // panel is the panel's, which is what WantCaptureMouse reports.
    const bool lookingWithRightButton =
        linearwinGetMouseButton(window, LINEARWIN_MOUSE_BUTTON_RIGHT) == LINEARWIN_PRESS &&
        !LinearUI::GetIO().WantCaptureMouse;
    const bool capture = inGame && !escapeMenuOpen && !playerIsDead &&
                         (shiftLockEnabled || lookingWithRightButton);
    if (capture == isCursorLocked) return;

    isCursorLocked = capture;
    linearwinSetInputMode(window, LINEARWIN_CURSOR,
                        capture ? LINEARWIN_CURSOR_DISABLED : LINEARWIN_CURSOR_NORMAL);
    if (capture) {
        // A fresh capture starts from wherever the cursor is, so the first movement
        // after it is not read as a jump across the screen.
        firstMouse = true;
    }
}

void setShiftLockEnabled(bool enabled) {
    shiftLockEnabled = enabled;
    applyCursorCapture();
}

void closeEscapeMenu() {
    escapeMenuOpen = false;
    escapeMenuTab = ESCAPE_TAB_PEOPLE;
    menuStatus.clear();
    applyCursorCapture();
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

    const linear::ui::theme::Colors& palette = linear::ui::theme::colors();
    const LinearVec2 display = LinearUI::GetIO().DisplaySize;

    LinearUI::SetNextWindowPos(LinearVec2(0.0f, 0.0f), LinearUICond_Always);
    LinearUI::SetNextWindowSize(display, LinearUICond_Always);
    LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0.0f, 0.0f));
    LinearUI::PushStyleColor(LinearUICol_WindowBg, linear::ui::theme::overlay(0.72f));
    LinearUI::Begin("EscapeMenuBackdrop", nullptr,
                 LinearUIWindowFlags_NoDecoration | LinearUIWindowFlags_NoSavedSettings |
                 LinearUIWindowFlags_NoMove | LinearUIWindowFlags_NoNav |
                 LinearUIWindowFlags_NoFocusOnAppearing | LinearUIWindowFlags_NoBringToFrontOnFocus);
    LinearUI::End();
    LinearUI::PopStyleColor();
    LinearUI::PopStyleVar();

    const float panelWidth =
        std::min(display.x - UiScale::Px(40.0f, uiScale), UiScale::Px(660.0f, uiScale));
    const float panelHeight =
        std::min(display.y - UiScale::Px(40.0f, uiScale), UiScale::Px(548.0f, uiScale));
    LinearUI::SetNextWindowPos(
        LinearVec2((display.x - panelWidth) * 0.5f, (display.y - panelHeight) * 0.5f),
        LinearUICond_Always);
    LinearUI::SetNextWindowSize(LinearVec2(panelWidth, panelHeight), LinearUICond_Always);

    if (!LinearUI::Begin("EscapeMenu", nullptr, overlayFlags())) {
        LinearUI::End();
        return;
    }

    // The strip is the menu's navigation. It is drawn rather than built from tab
    // items because each page has a mark beside its name, and the open page is
    // marked with a bar across its whole tab, which a tab item cannot give.
    const float stripHeight = UiScale::Px(48.0f, uiScale);
    const float stripWidth = LinearUI::GetContentRegionAvail().x;
    const LinearVec2 stripOrigin = LinearUI::GetCursorScreenPos();

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
        if (index > 0) LinearUI::SameLine(0.0f, 0.0f);
        if (escapeTabButton(tabs[index].label, tabs[index].tab, tabs[index].icon, tabWidth,
                            stripHeight, uiScale)) {
            if (escapeMenuTab != tabs[index].tab) menuStatus.clear();
            escapeMenuTab = tabs[index].tab;
        }
    }

    // The hairline under the strip belongs to the window, not to the page, so it
    // runs edge to edge behind the mark that says which page is open.
    const LinearVec2 windowPos = LinearUI::GetWindowPos();
    const LinearVec2 windowSize = LinearUI::GetWindowSize();
    const float stripBottom = stripOrigin.y + stripHeight;
    LinearUI::GetWindowDrawList()->AddLine(LinearVec2(windowPos.x, stripBottom),
                                         LinearVec2(windowPos.x + windowSize.x, stripBottom),
                                         packedColor(palette.border), 1.0f);
    LinearUI::SetCursorScreenPos(
        LinearVec2(stripOrigin.x, stripBottom + UiScale::Px(16.0f, uiScale)));

    const float footerHeight = UiScale::Px(56.0f, uiScale);
    if (escapeMenuTab == ESCAPE_TAB_PEOPLE) {
        const float inviteHeight = UiScale::Px(48.0f, uiScale);
        const float inviteWidth = LinearUI::GetContentRegionAvail().x;
        const LinearVec2 inviteOrigin = LinearUI::GetCursorScreenPos();
        LinearUI::PushID("invite");
        const bool pressed =
            LinearUI::InvisibleButton("##invite", LinearVec2(inviteWidth, inviteHeight));
        const bool hovered = LinearUI::IsItemHovered();
        LinearUI::PopID();

        LinearDrawList* draw = LinearUI::GetWindowDrawList();
        const float rounding = UiScale::Px(10.0f, uiScale);
        draw->AddRectFilled(
            inviteOrigin,
            LinearVec2(inviteOrigin.x + inviteWidth, inviteOrigin.y + inviteHeight),
            packedColor(hovered ? palette.frameHovered : palette.frame), rounding);
        const float iconSize = UiScale::Px(20.0f, uiScale);
        drawMenuIcon(draw, MENU_ICON_PEOPLE,
                     LinearVec2(inviteOrigin.x + UiScale::Px(16.0f, uiScale) + iconSize * 0.5f,
                              inviteOrigin.y + inviteHeight * 0.5f),
                     iconSize, packedColor(palette.text));
        const char* inviteLabel = "Invite friends to join";
        const LinearVec2 inviteTextSize = LinearUI::CalcTextSize(inviteLabel);
        draw->AddText(LinearVec2(inviteOrigin.x + UiScale::Px(16.0f, uiScale) + iconSize +
                                   UiScale::Px(12.0f, uiScale),
                               inviteOrigin.y + (inviteHeight - inviteTextSize.y) * 0.5f),
                      packedColor(palette.text), inviteLabel);
        if (pressed) {
            // The game's page is the invitation: opening it starts a server and
            // hands the friend a client, so there is nothing else to send.
            const std::string link =
                linear::client_menu::inviteUrl(automaticWebServerUrl(), launchGameId);
            if (link.empty()) {
                setMenuStatus("This session has no game page to invite anyone to.", true);
            } else {
                LinearUI::SetClipboardText(link.c_str());
                setMenuStatus("Invite link copied: " + link);
            }
        }

        if (!menuStatus.empty()) {
            LinearUI::PushStyleColor(LinearUICol_Text,
                                   menuStatusIsError ? palette.error : palette.muted);
            LinearUI::TextWrapped("%s", menuStatus.c_str());
            LinearUI::PopStyleColor();
        }

        LinearUI::BeginChild("EscapePeople", LinearVec2(0, -footerHeight), false);
        drawPeopleRows(uiScale);
        LinearUI::EndChild();
    } else if (escapeMenuTab == ESCAPE_TAB_SETTINGS) {
        LinearUI::BeginChild("EscapeSettings", LinearVec2(0, -footerHeight), false);

        drawLabel("CONTROLS");
        bool shiftLockValue = shiftLockEnabled;
        if (LinearUI::Checkbox("Shift lock", &shiftLockValue)) {
            setShiftLockEnabled(shiftLockValue);
        }
        LinearUI::TextWrapped(
            "Locks the mouse to the centre and keeps the character facing the camera.");

        drawLabel("MOUSE SENSITIVITY");
        LinearUI::SetNextItemWidth(-1.0f);
        LinearUI::SliderFloat("##mouseSensitivity", &camera.MouseSensitivity, 0.02f, 0.50f, "%.2f");
        LinearUI::Checkbox("Invert mouse Y", &invertMouseY);

        drawLabel("FIELD OF VIEW");
        LinearUI::SetNextItemWidth(-1.0f);
        LinearUI::SliderFloat("##fieldOfView", &camera.Zoom, 30.0f, 90.0f, "%.0f");

        LinearUI::Separator();
        drawLabel("INTERFACE");
        LinearUI::Checkbox("Show chat", &showChatHud);
        LinearUI::Checkbox("Show player list", &showPlayerListHud);
        LinearUI::Checkbox("Debug overlay", &showDebugGui);

        LinearUI::Separator();
        if (modernButton("Reset to defaults", LinearVec2(-1, UiScale::Px(38.0f, uiScale)),
                         linear::ui::theme::secondaryButton())) {
            resetClientSettings();
        }
        LinearUI::EndChild();
    } else if (escapeMenuTab == ESCAPE_TAB_REPORT) {
        LinearUI::BeginChild("EscapeReport", LinearVec2(0, -footerHeight), false);

        drawLabel("WHAT TO REPORT");
        if (LinearUI::RadioButton("This game", reportTargetKind == 0)) {
            setReportTarget(std::string());
        }
        LinearUI::SameLine();
        if (LinearUI::RadioButton("A player", reportTargetKind == 1)) {
            reportTargetKind = 1;
            reportTargetName = reportTargetInput;
        }
        if (reportTargetKind == 1) {
            LinearUI::SetNextItemWidth(-1.0f);
            if (LinearUI::InputText("##reportTarget", reportTargetInput,
                                  sizeof(reportTargetInput))) {
                reportTargetName = reportTargetInput;
            }
            LinearUI::PushStyleColor(LinearUICol_Text, palette.muted);
            LinearUI::TextWrapped("The account name, spelled the way the site has it. Reporting "
                                "a player from the People list fills this in for you.");
            LinearUI::PopStyleColor();
        } else if (launchGameId > 0) {
            LinearUI::Text("Game %d", launchGameId);
        } else {
            LinearUI::TextUnformatted("This session");
        }

        LinearUI::Spacing();
        drawLabel("WHY");
        LinearUI::InputTextMultiline("##reportReason", reportReason, sizeof(reportReason),
                                   LinearVec2(-1, UiScale::Px(108.0f, uiScale)));
        LinearUI::PushStyleColor(LinearUICol_Text, palette.muted);
        LinearUI::TextWrapped(
            "A moderator reads every report. Say what happened and where, in a sentence or two.");
        LinearUI::PopStyleColor();

        const bool signedIn = std::strlen(authToken) > 0;
        const bool ready = signedIn && std::strlen(reportReason) >= 3 &&
                           (reportTargetKind != 1 || std::strlen(reportTargetInput) > 0);
        if (!ready) LinearUI::BeginDisabled();
        if (modernButton("Send report", LinearVec2(-1, UiScale::Px(40.0f, uiScale)),
                         linear::ui::theme::primaryButton())) {
            sendReport();
        }
        if (!ready) LinearUI::EndDisabled();
        if (!signedIn) {
            LinearUI::PushStyleColor(LinearUICol_Text, palette.warning);
            LinearUI::TextWrapped("Sign in to send a report: the site has to know who reported it.");
            LinearUI::PopStyleColor();
        }
        if (!menuStatus.empty()) {
            LinearUI::PushStyleColor(LinearUICol_Text,
                                   menuStatusIsError ? palette.error : palette.muted);
            LinearUI::TextWrapped("%s", menuStatus.c_str());
            LinearUI::PopStyleColor();
        }
        LinearUI::EndChild();
    } else {
        LinearUI::BeginChild("EscapeHelp", LinearVec2(0, -footerHeight), false);

        drawLabel("KEYS");
        LinearUI::TextUnformatted("W A S D   move");
        LinearUI::TextUnformatted("Space   jump");
        LinearUI::TextUnformatted("Shift   sprint, or lock the camera with shift lock on");
        LinearUI::TextUnformatted("E   get into or out of a car");
        LinearUI::TextUnformatted("F3   debug overlay");
        LinearUI::TextUnformatted("Esc   this menu, and back to the game");

        LinearUI::Separator();
        drawLabel("THE SITE");
        LinearUI::PushStyleColor(LinearUICol_Text, palette.muted);
        LinearUI::TextWrapped("Your profile, your friends and every published game live on the "
                            "website, and the buttons in this menu ask that site to do the work.");
        LinearUI::PopStyleColor();
        LinearUI::Spacing();
        if (modernButton("Open the website", LinearVec2(-1, UiScale::Px(34.0f, uiScale)),
                         linear::ui::theme::secondaryButton())) {
            Auth::openBrowser(automaticWebServerUrl() + "/games");
        }
        if (modernButton("Open my profile", LinearVec2(-1, UiScale::Px(34.0f, uiScale)),
                         linear::ui::theme::secondaryButton())) {
            // The bare page is the signed-in account's own profile; the site
            // rewrites the address to the one with the id once it has loaded.
            Auth::openBrowser(automaticWebServerUrl() + "/profile");
        }
        LinearUI::EndChild();
    }

    LinearUI::Separator();
    std::string sessionLine =
        launchGameId > 0
            ? "Game " + std::to_string(launchGameId) +
                  (currentState == ONLINE ? " - online session" : " - offline session")
            : std::string(currentState == ONLINE ? "Online session" : "Offline session");
    sessionLine += " - Esc resumes";
    LinearUI::PushStyleColor(LinearUICol_Text, palette.muted);
    LinearUI::Text("%s", sessionLine.c_str());
    LinearUI::PopStyleColor();
    LinearUI::SameLine();

    // The two actions that must stay one click away sit at the right edge, so the
    // eye finds them in the same place on every page.
    const float buttonHeight = UiScale::Px(32.0f, uiScale);
    const float resumeWidth = UiScale::Px(104.0f, uiScale);
    const float leaveWidth = UiScale::Px(116.0f, uiScale);
    const float buttonGap = UiScale::Px(10.0f, uiScale);
    const float available = LinearUI::GetContentRegionAvail().x;
    const float needed = resumeWidth + buttonGap + leaveWidth;
    if (available > needed) {
        LinearUI::SetCursorPosX(LinearUI::GetCursorPosX() + available - needed);
    }
    if (modernButton("Resume", LinearVec2(resumeWidth, buttonHeight),
                     linear::ui::theme::primaryButton())) {
        closeEscapeMenu();
    }
    LinearUI::SameLine();
    if (modernButton("Leave game", LinearVec2(leaveWidth, buttonHeight),
                     linear::ui::theme::dangerButton())) {
        LinearUI::End();
        leaveCurrentGame(parts, physicsWorld);
        return;
    }

    LinearUI::End();
}
