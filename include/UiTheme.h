#pragma once

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <string>

#include "imgui.h"

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
    ImVec4 text;
    ImVec4 muted;
    ImVec4 accent;
    // The accent as a mark on a pale surface: the same mint, deep enough to read
    // against white. Equal to accent on the dark palette.
    ImVec4 accentStrong;
    ImVec4 ok;
    ImVec4 error;
    ImVec4 warning;

    ImVec4 background;
    ImVec4 panel;
    ImVec4 popup;
    ImVec4 border;

    ImVec4 primaryButton;
    ImVec4 secondaryButton;
    ImVec4 dangerButton;

    ImVec4 frame;
    ImVec4 frameHovered;
    ImVec4 frameActive;

    ImVec4 titleBar;
    ImVec4 titleBarActive;
    ImVec4 menuBar;
    ImVec4 scrollBackground;
    ImVec4 scrollGrab;
    ImVec4 scrollGrabHovered;
    ImVec4 buttonHovered;
    ImVec4 buttonActive;
    ImVec4 header;
    ImVec4 headerHovered;
    ImVec4 headerActive;
    ImVec4 separatorHovered;
    ImVec4 resizeGrip;
    ImVec4 resizeGripHovered;
    ImVec4 tab;
    ImVec4 tabHovered;
    ImVec4 tabSelected;
    ImVec4 tabDimmed;
    ImVec4 tabDimmedSelected;
    ImVec4 tableHeader;
    ImVec4 tableBorderLight;
    ImVec4 tableRowAlternate;
    ImVec4 textSelected;
    ImVec4 modalDim;
    // Panels that float over the 3D view: the client sets the alpha itself,
    // because how much of the world shows through is its own business.
    ImVec4 overlayBase;
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
    const char* explicitPath = std::getenv("RBLX_UI_SETTINGS");
    if (explicitPath && *explicitPath) return explicitPath;

    const char* appData = std::getenv("APPDATA");
    if (appData && *appData) return std::string(appData) + "/RBLXClone/ui.settings";
    const char* home = std::getenv("USERPROFILE");
    if (home && *home) return std::string(home) + "/.config/rblxclone/ui.settings";
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
    const char* env = std::getenv("RBLX_THEME");
    if (env && *env) {
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
    c.text = ImVec4(0.82f, 0.84f, 0.88f, 1.00f);
    c.muted = ImVec4(0.55f, 0.58f, 0.64f, 1.00f);
    c.accent = ImVec4(0.42f, 0.86f, 0.72f, 1.00f);
    c.accentStrong = c.accent;
    c.ok = ImVec4(0.42f, 0.86f, 0.60f, 1.00f);
    c.error = ImVec4(0.96f, 0.45f, 0.45f, 1.00f);
    c.warning = ImVec4(0.95f, 0.72f, 0.28f, 1.00f);

    c.background = ImVec4(0.07f, 0.08f, 0.10f, 1.00f);
    c.panel = ImVec4(0.05f, 0.06f, 0.07f, 1.00f);
    c.popup = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
    c.border = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);

    c.primaryButton = ImVec4(0.20f, 0.50f, 0.44f, 1.00f);
    c.secondaryButton = ImVec4(0.16f, 0.19f, 0.23f, 1.00f);
    c.dangerButton = ImVec4(0.55f, 0.22f, 0.24f, 1.00f);

    c.frame = ImVec4(0.13f, 0.15f, 0.18f, 1.00f);
    c.frameHovered = ImVec4(0.18f, 0.21f, 0.25f, 1.00f);
    c.frameActive = ImVec4(0.20f, 0.24f, 0.28f, 1.00f);

    c.titleBar = ImVec4(0.06f, 0.07f, 0.09f, 1.00f);
    c.titleBarActive = ImVec4(0.09f, 0.11f, 0.13f, 1.00f);
    c.menuBar = ImVec4(0.06f, 0.07f, 0.09f, 1.00f);
    c.scrollBackground = ImVec4(0.05f, 0.06f, 0.07f, 1.00f);
    c.scrollGrab = ImVec4(0.24f, 0.28f, 0.32f, 1.00f);
    c.scrollGrabHovered = ImVec4(0.30f, 0.36f, 0.40f, 1.00f);
    c.buttonHovered = ImVec4(0.24f, 0.32f, 0.38f, 1.00f);
    c.buttonActive = ImVec4(0.20f, 0.42f, 0.46f, 1.00f);
    c.header = ImVec4(0.18f, 0.28f, 0.32f, 1.00f);
    c.headerHovered = ImVec4(0.22f, 0.34f, 0.38f, 1.00f);
    c.headerActive = ImVec4(0.24f, 0.40f, 0.44f, 1.00f);
    c.separatorHovered = ImVec4(0.24f, 0.34f, 0.36f, 1.00f);
    c.resizeGrip = ImVec4(0.18f, 0.28f, 0.32f, 0.60f);
    c.resizeGripHovered = ImVec4(0.24f, 0.40f, 0.44f, 0.80f);
    c.tab = ImVec4(0.10f, 0.12f, 0.14f, 1.00f);
    c.tabHovered = ImVec4(0.22f, 0.34f, 0.38f, 1.00f);
    c.tabSelected = ImVec4(0.18f, 0.28f, 0.32f, 1.00f);
    c.tabDimmed = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
    c.tabDimmedSelected = ImVec4(0.14f, 0.20f, 0.22f, 1.00f);
    c.tableHeader = ImVec4(0.10f, 0.12f, 0.14f, 1.00f);
    c.tableBorderLight = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
    c.tableRowAlternate = ImVec4(1.0f, 1.0f, 1.0f, 0.03f);
    c.textSelected = ImVec4(0.24f, 0.44f, 0.42f, 0.60f);
    c.modalDim = ImVec4(0.02f, 0.03f, 0.04f, 0.60f);
    c.overlayBase = ImVec4(0.02f, 0.03f, 0.05f, 1.00f);
    return c;
}

// The same mint on a pale surface. Widgets take the deeper shade where the
// accent is a thin mark on white; buttons take a mint tint with dark labels.
inline Colors lightColors() {
    Colors c;
    c.text = ImVec4(0.12f, 0.14f, 0.17f, 1.00f);
    c.muted = ImVec4(0.42f, 0.46f, 0.51f, 1.00f);
    c.accent = ImVec4(0.42f, 0.86f, 0.72f, 1.00f);
    c.accentStrong = ImVec4(0.09f, 0.50f, 0.38f, 1.00f);
    c.ok = ImVec4(0.16f, 0.56f, 0.32f, 1.00f);
    c.error = ImVec4(0.80f, 0.24f, 0.24f, 1.00f);
    c.warning = ImVec4(0.66f, 0.44f, 0.04f, 1.00f);

    c.background = ImVec4(0.96f, 0.97f, 0.98f, 1.00f);
    c.panel = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c.popup = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c.border = ImVec4(0.78f, 0.81f, 0.85f, 1.00f);

    c.primaryButton = ImVec4(0.62f, 0.90f, 0.80f, 1.00f);
    c.secondaryButton = ImVec4(0.90f, 0.92f, 0.94f, 1.00f);
    c.dangerButton = ImVec4(0.95f, 0.73f, 0.73f, 1.00f);

    c.frame = ImVec4(0.89f, 0.91f, 0.93f, 1.00f);
    c.frameHovered = ImVec4(0.85f, 0.88f, 0.91f, 1.00f);
    c.frameActive = ImVec4(0.80f, 0.85f, 0.88f, 1.00f);

    c.titleBar = ImVec4(0.93f, 0.95f, 0.96f, 1.00f);
    c.titleBarActive = ImVec4(0.88f, 0.94f, 0.92f, 1.00f);
    c.menuBar = ImVec4(0.94f, 0.95f, 0.97f, 1.00f);
    c.scrollBackground = ImVec4(0.94f, 0.95f, 0.96f, 1.00f);
    c.scrollGrab = ImVec4(0.74f, 0.78f, 0.82f, 1.00f);
    c.scrollGrabHovered = ImVec4(0.64f, 0.70f, 0.74f, 1.00f);
    c.buttonHovered = ImVec4(0.84f, 0.90f, 0.88f, 1.00f);
    c.buttonActive = ImVec4(0.72f, 0.88f, 0.83f, 1.00f);
    c.header = ImVec4(0.86f, 0.93f, 0.90f, 1.00f);
    c.headerHovered = ImVec4(0.79f, 0.92f, 0.87f, 1.00f);
    c.headerActive = ImVec4(0.70f, 0.89f, 0.83f, 1.00f);
    c.separatorHovered = ImVec4(0.55f, 0.72f, 0.68f, 1.00f);
    c.resizeGrip = ImVec4(0.62f, 0.76f, 0.72f, 0.60f);
    c.resizeGripHovered = ImVec4(0.48f, 0.70f, 0.64f, 0.80f);
    c.tab = ImVec4(0.91f, 0.93f, 0.95f, 1.00f);
    c.tabHovered = ImVec4(0.80f, 0.92f, 0.88f, 1.00f);
    c.tabSelected = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c.tabDimmed = ImVec4(0.87f, 0.89f, 0.92f, 1.00f);
    c.tabDimmedSelected = ImVec4(0.93f, 0.97f, 0.95f, 1.00f);
    c.tableHeader = ImVec4(0.92f, 0.94f, 0.96f, 1.00f);
    c.tableBorderLight = ImVec4(0.88f, 0.90f, 0.92f, 1.00f);
    c.tableRowAlternate = ImVec4(0.0f, 0.0f, 0.0f, 0.03f);
    c.textSelected = ImVec4(0.55f, 0.85f, 0.75f, 0.60f);
    c.modalDim = ImVec4(0.10f, 0.12f, 0.14f, 0.45f);
    c.overlayBase = ImVec4(0.97f, 0.98f, 0.99f, 1.00f);
    return c;
}

inline const Colors& colors() {
    static const Colors dark = darkColors();
    static const Colors light = lightColors();
    return isLight() ? light : dark;
}

// The individual colors the windows reach for by hand.
inline ImVec4 accent() { return colors().accent; }
inline ImVec4 accentStrong() { return colors().accentStrong; }
inline ImVec4 text() { return colors().text; }
inline ImVec4 muted() { return colors().muted; }
inline ImVec4 ok() { return colors().ok; }
inline ImVec4 error() { return colors().error; }
inline ImVec4 warning() { return colors().warning; }

inline ImVec4 background() { return colors().background; }
inline ImVec4 panel() { return colors().panel; }
inline ImVec4 border() { return colors().border; }

// Buttons: a primary action in the accent, a quiet secondary one, and red for
// the action that leaves the game or removes something.
inline ImVec4 primaryButton() { return colors().primaryButton; }
inline ImVec4 secondaryButton() { return colors().secondaryButton; }
inline ImVec4 dangerButton() { return colors().dangerButton; }

// A panel that floats over the 3D view; the caller picks the alpha.
inline ImVec4 overlay(float alpha) {
    ImVec4 color = colors().overlayBase;
    color.w = alpha;
    return color;
}

// Colors only, for a window that has its own idea of spacing: the game client
// keeps its rounder HUD but speaks the same palette as the tools.
inline void applyColors(ImGuiStyle& style) {
    const Colors& c = colors();
    ImVec4* col = style.Colors;
    col[ImGuiCol_Text] = c.text;
    col[ImGuiCol_TextDisabled] = c.muted;
    col[ImGuiCol_WindowBg] = c.background;
    col[ImGuiCol_ChildBg] = c.panel;
    col[ImGuiCol_PopupBg] = c.popup;
    col[ImGuiCol_Border] = c.border;
    col[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    col[ImGuiCol_FrameBg] = c.frame;
    col[ImGuiCol_FrameBgHovered] = c.frameHovered;
    col[ImGuiCol_FrameBgActive] = c.frameActive;
    col[ImGuiCol_TitleBg] = c.titleBar;
    col[ImGuiCol_TitleBgActive] = c.titleBarActive;
    col[ImGuiCol_TitleBgCollapsed] = c.titleBar;
    col[ImGuiCol_MenuBarBg] = c.menuBar;
    col[ImGuiCol_ScrollbarBg] = c.scrollBackground;
    col[ImGuiCol_ScrollbarGrab] = c.scrollGrab;
    col[ImGuiCol_ScrollbarGrabHovered] = c.scrollGrabHovered;
    col[ImGuiCol_ScrollbarGrabActive] = c.accentStrong;
    col[ImGuiCol_CheckMark] = c.accentStrong;
    col[ImGuiCol_SliderGrab] = c.accentStrong;
    col[ImGuiCol_SliderGrabActive] = c.accentStrong;
    col[ImGuiCol_Button] = c.secondaryButton;
    col[ImGuiCol_ButtonHovered] = c.buttonHovered;
    col[ImGuiCol_ButtonActive] = c.buttonActive;
    col[ImGuiCol_Header] = c.header;
    col[ImGuiCol_HeaderHovered] = c.headerHovered;
    col[ImGuiCol_HeaderActive] = c.headerActive;
    col[ImGuiCol_Separator] = c.border;
    col[ImGuiCol_SeparatorHovered] = c.separatorHovered;
    col[ImGuiCol_SeparatorActive] = c.accentStrong;
    col[ImGuiCol_ResizeGrip] = c.resizeGrip;
    col[ImGuiCol_ResizeGripHovered] = c.resizeGripHovered;
    col[ImGuiCol_ResizeGripActive] = c.accentStrong;
    col[ImGuiCol_Tab] = c.tab;
    col[ImGuiCol_TabHovered] = c.tabHovered;
    col[ImGuiCol_TabSelected] = c.tabSelected;
    col[ImGuiCol_TabSelectedOverline] = c.accentStrong;
    col[ImGuiCol_TabDimmed] = c.tabDimmed;
    col[ImGuiCol_TabDimmedSelected] = c.tabDimmedSelected;
    col[ImGuiCol_DockingPreview] = ImVec4(c.accent.x, c.accent.y, c.accent.z, 0.55f);
    col[ImGuiCol_DockingEmptyBg] = c.background;
    col[ImGuiCol_PlotLines] = c.accentStrong;
    col[ImGuiCol_PlotLinesHovered] = c.ok;
    col[ImGuiCol_PlotHistogram] = c.accentStrong;
    col[ImGuiCol_PlotHistogramHovered] = c.ok;
    col[ImGuiCol_TableHeaderBg] = c.tableHeader;
    col[ImGuiCol_TableBorderStrong] = c.border;
    col[ImGuiCol_TableBorderLight] = c.tableBorderLight;
    col[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    col[ImGuiCol_TableRowBgAlt] = c.tableRowAlternate;
    col[ImGuiCol_TextSelectedBg] = c.textSelected;
    col[ImGuiCol_ModalWindowDimBg] = c.modalDim;
}

// The palette plus the geometry the tool windows share: square frames, softly
// rounded fields and buttons, and generous padding.
inline void apply() {
    if (isLight()) {
        ImGui::StyleColorsLight();
    } else {
        ImGui::StyleColorsDark();
    }

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.FramePadding = ImVec2(10.0f, 6.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);
    applyColors(style);
}

}  // namespace theme
}  // namespace ui
}  // namespace rblx
