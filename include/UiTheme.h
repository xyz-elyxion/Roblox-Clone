#pragma once

#include <cctype>
#include <cstdlib>
#include <fstream>
#include "Environment.h"

#include <string>

#include "rblxui.h"

// One look for every window this project puts on screen. The installer and the
// updater draw it first; Studio, RBLXCode, the server console and the game
// client read the same values from here instead of each keeping a copy, so the
// palette cannot drift apart window by window.
//
// There are two palettes, a dark one and a light one that keeps the mint accent.
// Which one is used is a setting, read once per process from a file every window
// shares:
//
//   RBLX_THEME        "light" or "dark"; wins over the file, handy for a one-off
//   RBLX_UI_SETTINGS  path to the settings file, if it is not the usual one
//
// The settings file is
//
//   Windows  %APPDATA%\RBLXClone\ui.settings
//   else     $XDG_CONFIG_HOME/rblxclone/ui.settings
//            or ~/.config/rblxclone/ui.settings
//
// and holds lines of "key = value", with # or ; starting a comment. The only key
// is "theme":
//
//   # every RBLX Clone window follows this file
//   theme = light
namespace rblx {
namespace ui {
namespace theme {

enum class Palette { Dark, Light };

// Every color the windows use, named for what it is for rather than for where it
// is drawn, so a light value can be picked without hunting through call sites.
struct Colors {
    RblxVec4 text;
    RblxVec4 muted;
    RblxVec4 accent;
    // The accent as a mark on a pale surface: the same mint, deep enough to read
    // against white. Equal to accent on the dark palette.
    RblxVec4 accentStrong;
    RblxVec4 ok;
    RblxVec4 error;
    RblxVec4 warning;

    RblxVec4 background;
    RblxVec4 panel;
    RblxVec4 popup;
    RblxVec4 border;

    RblxVec4 primaryButton;
    RblxVec4 secondaryButton;
    RblxVec4 dangerButton;

    RblxVec4 frame;
    RblxVec4 frameHovered;
    RblxVec4 frameActive;

    RblxVec4 titleBar;
    RblxVec4 titleBarActive;
    RblxVec4 menuBar;
    RblxVec4 scrollBackground;
    RblxVec4 scrollGrab;
    RblxVec4 scrollGrabHovered;
    RblxVec4 buttonHovered;
    RblxVec4 buttonActive;
    RblxVec4 header;
    RblxVec4 headerHovered;
    RblxVec4 headerActive;
    RblxVec4 separatorHovered;
    RblxVec4 resizeGrip;
    RblxVec4 resizeGripHovered;
    RblxVec4 tab;
    RblxVec4 tabHovered;
    RblxVec4 tabSelected;
    RblxVec4 tabDimmed;
    RblxVec4 tabDimmedSelected;
    RblxVec4 tableHeader;
    RblxVec4 tableBorderLight;
    RblxVec4 tableRowAlternate;
    RblxVec4 textSelected;
    RblxVec4 modalDim;
    // Panels that float over the 3D view: the client sets the alpha itself,
    // because how much of the world shows through is its own business.
    RblxVec4 overlayBase;
};

namespace detail {

inline std::string trim(const std::string& text) {
    std::size_t begin = 0;
    std::size_t end = text.size();
    while (begin < end && (text[begin] == ' ' || text[begin] == '\t' || text[begin] == '\r')) {
        ++begin;
    }
    while (end > begin && (text[end - 1] == ' ' || text[end - 1] == '\t' || text[end - 1] == '\r')) {
        --end;
    }
    return text.substr(begin, end - begin);
}

inline std::string lower(std::string text) {
    for (char& c : text) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return text;
}

// Where the shared settings file lives. A path that does not exist is not an
// error: every window then draws the dark palette, which is what it did before
// anyone wrote the file.
inline std::string settingsPath() {
    const std::string explicitPath = rblx::environment::get("RBLX_UI_SETTINGS");
    if (!explicitPath.empty()) return explicitPath;

    const std::string appData = rblx::environment::get("APPDATA");
    if (!appData.empty()) return appData + "/RBLXClone/ui.settings";
    const std::string home = rblx::environment::get("USERPROFILE");
    if (!home.empty()) return home + "/.config/rblxclone/ui.settings";
    return std::string();
}

// Applies one "key = value" line; anything unknown is ignored, so a settings
// file written for a later version still loads here.
inline void applyLine(const std::string& line, const std::string& path, Palette* palette,
                      std::string* origin) {
    const std::string trimmed = trim(line);
    if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') return;

    const std::size_t equals = trimmed.find('=');
    if (equals == std::string::npos) return;

    const std::string key = lower(trim(trimmed.substr(0, equals)));
    const std::string value = lower(trim(trimmed.substr(equals + 1)));
    if (key != "theme" && key != "palette") return;

    if (value == "light") {
        *palette = Palette::Light;
        *origin = path;
    } else if (value == "dark") {
        *palette = Palette::Dark;
        *origin = path;
    }
}

struct Settings {
    bool loaded = false;
    Palette palette = Palette::Dark;
    std::string path;    // the file that was consulted, empty when there is none
    std::string origin;  // RBLX_THEME, the settings path, or "default"
};

inline Settings& settings() {
    static Settings state;
    return state;
}

inline void read() {
    Settings& state = settings();
    state.loaded = true;
    state.palette = Palette::Dark;
    state.origin = "default";
    state.path = settingsPath();

    if (!state.path.empty()) {
        std::ifstream file(state.path);
        std::string line;
        while (std::getline(file, line)) {
            applyLine(line, state.path, &state.palette, &state.origin);
        }
    }

    // The environment comes last so a single launch can be shown in the other
    // palette without editing the file.
    const std::string env = rblx::environment::get("RBLX_THEME");
    if (!env.empty()) {
        const std::string value = lower(trim(env));
        if (value == "light") {
            state.palette = Palette::Light;
            state.origin = "RBLX_THEME";
        } else if (value == "dark") {
            state.palette = Palette::Dark;
            state.origin = "RBLX_THEME";
        }
    }
}

}  // namespace detail

// Which palette this process draws. Reading the file happens once, the first
// time anything asks for a color.
inline bool isLight() {
    if (!detail::settings().loaded) detail::read();
    return detail::settings().palette == Palette::Light;
}

inline const char* activeName() { return isLight() ? "light" : "dark"; }

// The file that was consulted, and what chose the palette ("default",
// "RBLX_THEME" or that path). For a status line or a --version style report.
inline std::string settingsFile() {
    isLight();
    return detail::settings().path;
}

inline std::string settingsOrigin() {
    isLight();
    return detail::settings().origin;
}

// Forgets the cached choice and reads the file again. A window that wants to
// follow an edited file instead of restarting can call this; nothing has to.
inline void reload() {
    detail::settings().loaded = false;
    isLight();
}

inline Colors darkColors() {
    Colors c;
    c.text = RblxVec4(0.82f, 0.84f, 0.88f, 1.00f);
    c.muted = RblxVec4(0.55f, 0.58f, 0.64f, 1.00f);
    c.accent = RblxVec4(0.42f, 0.86f, 0.72f, 1.00f);
    c.accentStrong = c.accent;
    c.ok = RblxVec4(0.42f, 0.86f, 0.60f, 1.00f);
    c.error = RblxVec4(0.96f, 0.45f, 0.45f, 1.00f);
    c.warning = RblxVec4(0.95f, 0.72f, 0.28f, 1.00f);

    c.background = RblxVec4(0.07f, 0.08f, 0.10f, 1.00f);
    c.panel = RblxVec4(0.05f, 0.06f, 0.07f, 1.00f);
    c.popup = RblxVec4(0.09f, 0.10f, 0.12f, 1.00f);
    c.border = RblxVec4(0.18f, 0.20f, 0.24f, 1.00f);

    c.primaryButton = RblxVec4(0.20f, 0.50f, 0.44f, 1.00f);
    c.secondaryButton = RblxVec4(0.16f, 0.19f, 0.23f, 1.00f);
    c.dangerButton = RblxVec4(0.55f, 0.22f, 0.24f, 1.00f);

    c.frame = RblxVec4(0.13f, 0.15f, 0.18f, 1.00f);
    c.frameHovered = RblxVec4(0.18f, 0.21f, 0.25f, 1.00f);
    c.frameActive = RblxVec4(0.20f, 0.24f, 0.28f, 1.00f);

    c.titleBar = RblxVec4(0.06f, 0.07f, 0.09f, 1.00f);
    c.titleBarActive = RblxVec4(0.09f, 0.11f, 0.13f, 1.00f);
    c.menuBar = RblxVec4(0.06f, 0.07f, 0.09f, 1.00f);
    c.scrollBackground = RblxVec4(0.05f, 0.06f, 0.07f, 1.00f);
    c.scrollGrab = RblxVec4(0.24f, 0.28f, 0.32f, 1.00f);
    c.scrollGrabHovered = RblxVec4(0.30f, 0.36f, 0.40f, 1.00f);
    c.buttonHovered = RblxVec4(0.24f, 0.32f, 0.38f, 1.00f);
    c.buttonActive = RblxVec4(0.20f, 0.42f, 0.46f, 1.00f);
    c.header = RblxVec4(0.18f, 0.28f, 0.32f, 1.00f);
    c.headerHovered = RblxVec4(0.22f, 0.34f, 0.38f, 1.00f);
    c.headerActive = RblxVec4(0.24f, 0.40f, 0.44f, 1.00f);
    c.separatorHovered = RblxVec4(0.24f, 0.34f, 0.36f, 1.00f);
    c.resizeGrip = RblxVec4(0.18f, 0.28f, 0.32f, 0.60f);
    c.resizeGripHovered = RblxVec4(0.24f, 0.40f, 0.44f, 0.80f);
    c.tab = RblxVec4(0.10f, 0.12f, 0.14f, 1.00f);
    c.tabHovered = RblxVec4(0.22f, 0.34f, 0.38f, 1.00f);
    c.tabSelected = RblxVec4(0.18f, 0.28f, 0.32f, 1.00f);
    c.tabDimmed = RblxVec4(0.09f, 0.10f, 0.12f, 1.00f);
    c.tabDimmedSelected = RblxVec4(0.14f, 0.20f, 0.22f, 1.00f);
    c.tableHeader = RblxVec4(0.10f, 0.12f, 0.14f, 1.00f);
    c.tableBorderLight = RblxVec4(0.14f, 0.16f, 0.19f, 1.00f);
    c.tableRowAlternate = RblxVec4(1.0f, 1.0f, 1.0f, 0.03f);
    c.textSelected = RblxVec4(0.24f, 0.44f, 0.42f, 0.60f);
    c.modalDim = RblxVec4(0.02f, 0.03f, 0.04f, 0.60f);
    c.overlayBase = RblxVec4(0.02f, 0.03f, 0.05f, 1.00f);
    return c;
}

// The same mint on a pale surface. Widgets take the deeper shade where the
// accent is a thin mark on white; buttons take a mint tint with dark labels.
inline Colors lightColors() {
    Colors c;
    c.text = RblxVec4(0.12f, 0.14f, 0.17f, 1.00f);
    c.muted = RblxVec4(0.42f, 0.46f, 0.51f, 1.00f);
    c.accent = RblxVec4(0.42f, 0.86f, 0.72f, 1.00f);
    c.accentStrong = RblxVec4(0.09f, 0.50f, 0.38f, 1.00f);
    c.ok = RblxVec4(0.16f, 0.56f, 0.32f, 1.00f);
    c.error = RblxVec4(0.80f, 0.24f, 0.24f, 1.00f);
    c.warning = RblxVec4(0.66f, 0.44f, 0.04f, 1.00f);

    c.background = RblxVec4(0.96f, 0.97f, 0.98f, 1.00f);
    c.panel = RblxVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c.popup = RblxVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c.border = RblxVec4(0.78f, 0.81f, 0.85f, 1.00f);

    c.primaryButton = RblxVec4(0.62f, 0.90f, 0.80f, 1.00f);
    c.secondaryButton = RblxVec4(0.90f, 0.92f, 0.94f, 1.00f);
    c.dangerButton = RblxVec4(0.95f, 0.73f, 0.73f, 1.00f);

    c.frame = RblxVec4(0.89f, 0.91f, 0.93f, 1.00f);
    c.frameHovered = RblxVec4(0.85f, 0.88f, 0.91f, 1.00f);
    c.frameActive = RblxVec4(0.80f, 0.85f, 0.88f, 1.00f);

    c.titleBar = RblxVec4(0.93f, 0.95f, 0.96f, 1.00f);
    c.titleBarActive = RblxVec4(0.88f, 0.94f, 0.92f, 1.00f);
    c.menuBar = RblxVec4(0.94f, 0.95f, 0.97f, 1.00f);
    c.scrollBackground = RblxVec4(0.94f, 0.95f, 0.96f, 1.00f);
    c.scrollGrab = RblxVec4(0.74f, 0.78f, 0.82f, 1.00f);
    c.scrollGrabHovered = RblxVec4(0.64f, 0.70f, 0.74f, 1.00f);
    c.buttonHovered = RblxVec4(0.84f, 0.90f, 0.88f, 1.00f);
    c.buttonActive = RblxVec4(0.72f, 0.88f, 0.83f, 1.00f);
    c.header = RblxVec4(0.86f, 0.93f, 0.90f, 1.00f);
    c.headerHovered = RblxVec4(0.79f, 0.92f, 0.87f, 1.00f);
    c.headerActive = RblxVec4(0.70f, 0.89f, 0.83f, 1.00f);
    c.separatorHovered = RblxVec4(0.55f, 0.72f, 0.68f, 1.00f);
    c.resizeGrip = RblxVec4(0.62f, 0.76f, 0.72f, 0.60f);
    c.resizeGripHovered = RblxVec4(0.48f, 0.70f, 0.64f, 0.80f);
    c.tab = RblxVec4(0.91f, 0.93f, 0.95f, 1.00f);
    c.tabHovered = RblxVec4(0.80f, 0.92f, 0.88f, 1.00f);
    c.tabSelected = RblxVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c.tabDimmed = RblxVec4(0.87f, 0.89f, 0.92f, 1.00f);
    c.tabDimmedSelected = RblxVec4(0.93f, 0.97f, 0.95f, 1.00f);
    c.tableHeader = RblxVec4(0.92f, 0.94f, 0.96f, 1.00f);
    c.tableBorderLight = RblxVec4(0.88f, 0.90f, 0.92f, 1.00f);
    c.tableRowAlternate = RblxVec4(0.0f, 0.0f, 0.0f, 0.03f);
    c.textSelected = RblxVec4(0.55f, 0.85f, 0.75f, 0.60f);
    c.modalDim = RblxVec4(0.10f, 0.12f, 0.14f, 0.45f);
    c.overlayBase = RblxVec4(0.97f, 0.98f, 0.99f, 1.00f);
    return c;
}

inline const Colors& colors() {
    static const Colors dark = darkColors();
    static const Colors light = lightColors();
    return isLight() ? light : dark;
}

// The individual colors the windows reach for by hand.
inline RblxVec4 accent() { return colors().accent; }
inline RblxVec4 accentStrong() { return colors().accentStrong; }
inline RblxVec4 text() { return colors().text; }
inline RblxVec4 muted() { return colors().muted; }
inline RblxVec4 ok() { return colors().ok; }
inline RblxVec4 error() { return colors().error; }
inline RblxVec4 warning() { return colors().warning; }

inline RblxVec4 background() { return colors().background; }
inline RblxVec4 panel() { return colors().panel; }
inline RblxVec4 border() { return colors().border; }

// Buttons: a primary action in the accent, a quiet secondary one, and red for
// the action that leaves the game or removes something.
inline RblxVec4 primaryButton() { return colors().primaryButton; }
inline RblxVec4 secondaryButton() { return colors().secondaryButton; }
inline RblxVec4 dangerButton() { return colors().dangerButton; }

// A panel that floats over the 3D view; the caller picks the alpha.
inline RblxVec4 overlay(float alpha) {
    RblxVec4 color = colors().overlayBase;
    color.w = alpha;
    return color;
}

// Colors only, for a window that has its own idea of spacing: the game client
// keeps its rounder HUD but speaks the same palette as the tools.
inline void applyColors(RblxUIStyle& style) {
    const Colors& c = colors();
    RblxVec4* col = style.Colors;
    col[RblxUICol_Text] = c.text;
    col[RblxUICol_TextDisabled] = c.muted;
    col[RblxUICol_WindowBg] = c.background;
    col[RblxUICol_ChildBg] = c.panel;
    col[RblxUICol_PopupBg] = c.popup;
    col[RblxUICol_Border] = c.border;
    col[RblxUICol_BorderShadow] = RblxVec4(0.0f, 0.0f, 0.0f, 0.0f);
    col[RblxUICol_FrameBg] = c.frame;
    col[RblxUICol_FrameBgHovered] = c.frameHovered;
    col[RblxUICol_FrameBgActive] = c.frameActive;
    col[RblxUICol_TitleBg] = c.titleBar;
    col[RblxUICol_TitleBgActive] = c.titleBarActive;
    col[RblxUICol_TitleBgCollapsed] = c.titleBar;
    col[RblxUICol_MenuBarBg] = c.menuBar;
    col[RblxUICol_ScrollbarBg] = c.scrollBackground;
    col[RblxUICol_ScrollbarGrab] = c.scrollGrab;
    col[RblxUICol_ScrollbarGrabHovered] = c.scrollGrabHovered;
    col[RblxUICol_ScrollbarGrabActive] = c.accentStrong;
    col[RblxUICol_CheckMark] = c.accentStrong;
    col[RblxUICol_SliderGrab] = c.accentStrong;
    col[RblxUICol_SliderGrabActive] = c.accentStrong;
    col[RblxUICol_Button] = c.secondaryButton;
    col[RblxUICol_ButtonHovered] = c.buttonHovered;
    col[RblxUICol_ButtonActive] = c.buttonActive;
    col[RblxUICol_Header] = c.header;
    col[RblxUICol_HeaderHovered] = c.headerHovered;
    col[RblxUICol_HeaderActive] = c.headerActive;
    col[RblxUICol_Separator] = c.border;
    col[RblxUICol_SeparatorHovered] = c.separatorHovered;
    col[RblxUICol_SeparatorActive] = c.accentStrong;
    col[RblxUICol_ResizeGrip] = c.resizeGrip;
    col[RblxUICol_ResizeGripHovered] = c.resizeGripHovered;
    col[RblxUICol_ResizeGripActive] = c.accentStrong;
    col[RblxUICol_Tab] = c.tab;
    col[RblxUICol_TabHovered] = c.tabHovered;
    col[RblxUICol_TabSelected] = c.tabSelected;
    col[RblxUICol_TabSelectedOverline] = c.accentStrong;
    col[RblxUICol_TabDimmed] = c.tabDimmed;
    col[RblxUICol_TabDimmedSelected] = c.tabDimmedSelected;
    col[RblxUICol_DockingPreview] = RblxVec4(c.accent.x, c.accent.y, c.accent.z, 0.55f);
    col[RblxUICol_DockingEmptyBg] = c.background;
    col[RblxUICol_PlotLines] = c.accentStrong;
    col[RblxUICol_PlotLinesHovered] = c.ok;
    col[RblxUICol_PlotHistogram] = c.accentStrong;
    col[RblxUICol_PlotHistogramHovered] = c.ok;
    col[RblxUICol_TableHeaderBg] = c.tableHeader;
    col[RblxUICol_TableBorderStrong] = c.border;
    col[RblxUICol_TableBorderLight] = c.tableBorderLight;
    col[RblxUICol_TableRowBg] = RblxVec4(0.0f, 0.0f, 0.0f, 0.0f);
    col[RblxUICol_TableRowBgAlt] = c.tableRowAlternate;
    col[RblxUICol_TextSelectedBg] = c.textSelected;
    col[RblxUICol_ModalWindowDimBg] = c.modalDim;
}

// The palette plus the geometry the tool windows share: square frames, softly
// rounded fields and buttons, and generous padding.
inline void apply() {
    if (isLight()) {
        RblxUI::StyleColorsLight();
    } else {
        RblxUI::StyleColorsDark();
    }

    RblxUIStyle& style = RblxUI::GetStyle();
    style.WindowRounding = 0.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.FramePadding = RblxVec2(10.0f, 6.0f);
    style.ItemSpacing = RblxVec2(10.0f, 8.0f);
    applyColors(style);
}

}  // namespace theme
}  // namespace ui
}  // namespace rblx
