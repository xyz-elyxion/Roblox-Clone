#pragma once

#include <cctype>
#include <cstdlib>
#include <fstream>
#include "Environment.h"

#include <string>

#include "linearui.h"

// One look for every window this project puts on screen. The installer and the
// updater draw it first; Studio, LinearCode, the server console and the game
// client read the same values from here instead of each keeping a copy, so the
// palette cannot drift apart window by window.
//
// There are two palettes, a dark one and a light one that keeps the mint accent.
// Which one is used is a setting, read once per process from a file every window
// shares:
//
//   LINEAR_THEME        "light" or "dark"; wins over the file, handy for a one-off
//   LINEAR_UI_SETTINGS  path to the settings file, if it is not the usual one
//
// The settings file is
//
//   Windows  %APPDATA%\Linear\ui.settings
//   else     $XDG_CONFIG_HOME/linear/ui.settings
//            or ~/.config/linear/ui.settings
//
// and holds lines of "key = value", with # or ; starting a comment. The only key
// is "theme":
//
//   # every Linear window follows this file
//   theme = light
namespace linear {
namespace ui {
namespace theme {

enum class Palette { Dark, Light };

// Every color the windows use, named for what it is for rather than for where it
// is drawn, so a light value can be picked without hunting through call sites.
struct Colors {
    LinearVec4 text;
    LinearVec4 muted;
    LinearVec4 accent;
    // The accent as a mark on a pale surface: the same mint, deep enough to read
    // against white. Equal to accent on the dark palette.
    LinearVec4 accentStrong;
    LinearVec4 ok;
    LinearVec4 error;
    LinearVec4 warning;

    LinearVec4 background;
    LinearVec4 panel;
    LinearVec4 popup;
    LinearVec4 border;

    LinearVec4 primaryButton;
    LinearVec4 secondaryButton;
    LinearVec4 dangerButton;

    LinearVec4 frame;
    LinearVec4 frameHovered;
    LinearVec4 frameActive;

    LinearVec4 titleBar;
    LinearVec4 titleBarActive;
    LinearVec4 menuBar;
    LinearVec4 scrollBackground;
    LinearVec4 scrollGrab;
    LinearVec4 scrollGrabHovered;
    LinearVec4 buttonHovered;
    LinearVec4 buttonActive;
    LinearVec4 header;
    LinearVec4 headerHovered;
    LinearVec4 headerActive;
    LinearVec4 separatorHovered;
    LinearVec4 resizeGrip;
    LinearVec4 resizeGripHovered;
    LinearVec4 tab;
    LinearVec4 tabHovered;
    LinearVec4 tabSelected;
    LinearVec4 tabDimmed;
    LinearVec4 tabDimmedSelected;
    LinearVec4 tableHeader;
    LinearVec4 tableBorderLight;
    LinearVec4 tableRowAlternate;
    LinearVec4 textSelected;
    LinearVec4 modalDim;
    // Panels that float over the 3D view: the client sets the alpha itself,
    // because how much of the world shows through is its own business.
    LinearVec4 overlayBase;

    // The art panel of the setup window. It is drawn rather than loaded so the
    // package ships no artwork beside the executable: four scenes take turns there,
    // so beside the day sky and its islands there is a night sky and the warm one
    // over the dunes, with the sand and what is drawn on it.
    LinearVec4 heroSkyTop;
    LinearVec4 heroSkyMid;
    LinearVec4 heroSkyBottom;
    LinearVec4 heroLight;
    LinearVec4 heroIslandFar;
    LinearVec4 heroIslandNear;
    LinearVec4 heroGrassFar;
    LinearVec4 heroGrassNear;
    // The night sky, its stars and the moon.
    LinearVec4 heroNightTop;
    LinearVec4 heroNightBottom;
    LinearVec4 heroStar;
    LinearVec4 heroMoon;
    // The evening sky over the dunes, and the sand itself, far and near.
    LinearVec4 heroWarmSkyTop;
    LinearVec4 heroWarmSkyBottom;
    LinearVec4 heroSandFar;
    LinearVec4 heroSandNear;

    // The mark: the sky band above its horizon, the water under it, the sun over
    // it and the ink of the letter. The same four values in both palettes - the
    // brand does not change with the theme - but named here so the windows that
    // draw it, the icon the executables carry and the site's copy cannot drift
    // apart, and so no window keeps a colour of its own.
    LinearVec4 logoSkyTop;
    LinearVec4 logoSkyBottom;
    LinearVec4 logoSun;
    LinearVec4 logoInk;

    // The trough of a progress bar, which is neither a frame nor a button.
    LinearVec4 progressTrack;
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
    const std::string explicitPath = linear::environment::get("LINEAR_UI_SETTINGS");
    if (!explicitPath.empty()) return explicitPath;

    const std::string appData = linear::environment::get("APPDATA");
    if (!appData.empty()) return appData + "/Linear/ui.settings";
    const std::string home = linear::environment::get("USERPROFILE");
    if (!home.empty()) return home + "/.config/linear/ui.settings";
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
    std::string origin;  // LINEAR_THEME, the settings path, or "default"
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
    const std::string env = linear::environment::get("LINEAR_THEME");
    if (!env.empty()) {
        const std::string value = lower(trim(env));
        if (value == "light") {
            state.palette = Palette::Light;
            state.origin = "LINEAR_THEME";
        } else if (value == "dark") {
            state.palette = Palette::Dark;
            state.origin = "LINEAR_THEME";
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
// "LINEAR_THEME" or that path). For a status line or a --version style report.
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
    c.text = LinearVec4(0.82f, 0.84f, 0.88f, 1.00f);
    c.muted = LinearVec4(0.55f, 0.58f, 0.64f, 1.00f);
    c.accent = LinearVec4(0.42f, 0.86f, 0.72f, 1.00f);
    c.accentStrong = c.accent;
    c.ok = LinearVec4(0.42f, 0.86f, 0.60f, 1.00f);
    c.error = LinearVec4(0.96f, 0.45f, 0.45f, 1.00f);
    c.warning = LinearVec4(0.95f, 0.72f, 0.28f, 1.00f);

    c.background = LinearVec4(0.07f, 0.08f, 0.10f, 1.00f);
    c.panel = LinearVec4(0.05f, 0.06f, 0.07f, 1.00f);
    c.popup = LinearVec4(0.09f, 0.10f, 0.12f, 1.00f);
    c.border = LinearVec4(0.18f, 0.20f, 0.24f, 1.00f);

    c.primaryButton = LinearVec4(0.20f, 0.50f, 0.44f, 1.00f);
    c.secondaryButton = LinearVec4(0.16f, 0.19f, 0.23f, 1.00f);
    c.dangerButton = LinearVec4(0.55f, 0.22f, 0.24f, 1.00f);

    c.frame = LinearVec4(0.13f, 0.15f, 0.18f, 1.00f);
    c.frameHovered = LinearVec4(0.18f, 0.21f, 0.25f, 1.00f);
    c.frameActive = LinearVec4(0.20f, 0.24f, 0.28f, 1.00f);

    c.titleBar = LinearVec4(0.06f, 0.07f, 0.09f, 1.00f);
    c.titleBarActive = LinearVec4(0.09f, 0.11f, 0.13f, 1.00f);
    c.menuBar = LinearVec4(0.06f, 0.07f, 0.09f, 1.00f);
    c.scrollBackground = LinearVec4(0.05f, 0.06f, 0.07f, 1.00f);
    c.scrollGrab = LinearVec4(0.24f, 0.28f, 0.32f, 1.00f);
    c.scrollGrabHovered = LinearVec4(0.30f, 0.36f, 0.40f, 1.00f);
    c.buttonHovered = LinearVec4(0.24f, 0.32f, 0.38f, 1.00f);
    c.buttonActive = LinearVec4(0.20f, 0.42f, 0.46f, 1.00f);
    c.header = LinearVec4(0.18f, 0.28f, 0.32f, 1.00f);
    c.headerHovered = LinearVec4(0.22f, 0.34f, 0.38f, 1.00f);
    c.headerActive = LinearVec4(0.24f, 0.40f, 0.44f, 1.00f);
    c.separatorHovered = LinearVec4(0.24f, 0.34f, 0.36f, 1.00f);
    c.resizeGrip = LinearVec4(0.18f, 0.28f, 0.32f, 0.60f);
    c.resizeGripHovered = LinearVec4(0.24f, 0.40f, 0.44f, 0.80f);
    c.tab = LinearVec4(0.10f, 0.12f, 0.14f, 1.00f);
    c.tabHovered = LinearVec4(0.22f, 0.34f, 0.38f, 1.00f);
    c.tabSelected = LinearVec4(0.18f, 0.28f, 0.32f, 1.00f);
    c.tabDimmed = LinearVec4(0.09f, 0.10f, 0.12f, 1.00f);
    c.tabDimmedSelected = LinearVec4(0.14f, 0.20f, 0.22f, 1.00f);
    c.tableHeader = LinearVec4(0.10f, 0.12f, 0.14f, 1.00f);
    c.tableBorderLight = LinearVec4(0.14f, 0.16f, 0.19f, 1.00f);
    c.tableRowAlternate = LinearVec4(1.0f, 1.0f, 1.0f, 0.03f);
    c.textSelected = LinearVec4(0.24f, 0.44f, 0.42f, 0.60f);
    c.modalDim = LinearVec4(0.02f, 0.03f, 0.04f, 0.60f);
    c.overlayBase = LinearVec4(0.02f, 0.03f, 0.05f, 1.00f);

    c.heroSkyTop = LinearVec4(0.13f, 0.32f, 0.76f, 1.00f);
    c.heroSkyMid = LinearVec4(0.40f, 0.63f, 0.92f, 1.00f);
    c.heroSkyBottom = LinearVec4(0.87f, 0.94f, 1.00f, 1.00f);
    c.heroLight = LinearVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c.heroIslandFar = LinearVec4(0.53f, 0.63f, 0.79f, 1.00f);
    c.heroIslandNear = LinearVec4(0.24f, 0.34f, 0.49f, 1.00f);
    c.heroGrassFar = LinearVec4(0.45f, 0.63f, 0.57f, 1.00f);
    c.heroGrassNear = LinearVec4(0.20f, 0.39f, 0.33f, 1.00f);

    c.heroNightTop = LinearVec4(0.03f, 0.05f, 0.14f, 1.00f);
    c.heroNightBottom = LinearVec4(0.20f, 0.25f, 0.44f, 1.00f);
    c.heroStar = LinearVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c.heroMoon = LinearVec4(0.93f, 0.95f, 1.00f, 1.00f);
    c.heroWarmSkyTop = LinearVec4(0.95f, 0.58f, 0.33f, 1.00f);
    c.heroWarmSkyBottom = LinearVec4(1.00f, 0.87f, 0.68f, 1.00f);
    c.heroSandFar = LinearVec4(0.89f, 0.74f, 0.53f, 1.00f);
    c.heroSandNear = LinearVec4(0.78f, 0.58f, 0.35f, 1.00f);

    c.logoSkyTop = LinearVec4(0.04f, 0.12f, 0.22f, 1.00f);
    c.logoSkyBottom = LinearVec4(0.18f, 0.48f, 0.84f, 1.00f);
    c.logoSun = LinearVec4(1.00f, 0.82f, 0.40f, 1.00f);
    c.logoInk = LinearVec4(1.00f, 1.00f, 1.00f, 1.00f);

    c.progressTrack = LinearVec4(0.15f, 0.17f, 0.21f, 1.00f);
    return c;
}

// The same mint on a pale surface. Widgets take the deeper shade where the
// accent is a thin mark on white; buttons take a mint tint with dark labels.
inline Colors lightColors() {
    Colors c;
    c.text = LinearVec4(0.12f, 0.14f, 0.17f, 1.00f);
    c.muted = LinearVec4(0.42f, 0.46f, 0.51f, 1.00f);
    c.accent = LinearVec4(0.42f, 0.86f, 0.72f, 1.00f);
    c.accentStrong = LinearVec4(0.09f, 0.50f, 0.38f, 1.00f);
    c.ok = LinearVec4(0.16f, 0.56f, 0.32f, 1.00f);
    c.error = LinearVec4(0.80f, 0.24f, 0.24f, 1.00f);
    c.warning = LinearVec4(0.66f, 0.44f, 0.04f, 1.00f);

    c.background = LinearVec4(0.96f, 0.97f, 0.98f, 1.00f);
    c.panel = LinearVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c.popup = LinearVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c.border = LinearVec4(0.78f, 0.81f, 0.85f, 1.00f);

    c.primaryButton = LinearVec4(0.62f, 0.90f, 0.80f, 1.00f);
    c.secondaryButton = LinearVec4(0.90f, 0.92f, 0.94f, 1.00f);
    c.dangerButton = LinearVec4(0.95f, 0.73f, 0.73f, 1.00f);

    c.frame = LinearVec4(0.89f, 0.91f, 0.93f, 1.00f);
    c.frameHovered = LinearVec4(0.85f, 0.88f, 0.91f, 1.00f);
    c.frameActive = LinearVec4(0.80f, 0.85f, 0.88f, 1.00f);

    c.titleBar = LinearVec4(0.93f, 0.95f, 0.96f, 1.00f);
    c.titleBarActive = LinearVec4(0.88f, 0.94f, 0.92f, 1.00f);
    c.menuBar = LinearVec4(0.94f, 0.95f, 0.97f, 1.00f);
    c.scrollBackground = LinearVec4(0.94f, 0.95f, 0.96f, 1.00f);
    c.scrollGrab = LinearVec4(0.74f, 0.78f, 0.82f, 1.00f);
    c.scrollGrabHovered = LinearVec4(0.64f, 0.70f, 0.74f, 1.00f);
    c.buttonHovered = LinearVec4(0.84f, 0.90f, 0.88f, 1.00f);
    c.buttonActive = LinearVec4(0.72f, 0.88f, 0.83f, 1.00f);
    c.header = LinearVec4(0.86f, 0.93f, 0.90f, 1.00f);
    c.headerHovered = LinearVec4(0.79f, 0.92f, 0.87f, 1.00f);
    c.headerActive = LinearVec4(0.70f, 0.89f, 0.83f, 1.00f);
    c.separatorHovered = LinearVec4(0.55f, 0.72f, 0.68f, 1.00f);
    c.resizeGrip = LinearVec4(0.62f, 0.76f, 0.72f, 0.60f);
    c.resizeGripHovered = LinearVec4(0.48f, 0.70f, 0.64f, 0.80f);
    c.tab = LinearVec4(0.91f, 0.93f, 0.95f, 1.00f);
    c.tabHovered = LinearVec4(0.80f, 0.92f, 0.88f, 1.00f);
    c.tabSelected = LinearVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c.tabDimmed = LinearVec4(0.87f, 0.89f, 0.92f, 1.00f);
    c.tabDimmedSelected = LinearVec4(0.93f, 0.97f, 0.95f, 1.00f);
    c.tableHeader = LinearVec4(0.92f, 0.94f, 0.96f, 1.00f);
    c.tableBorderLight = LinearVec4(0.88f, 0.90f, 0.92f, 1.00f);
    c.tableRowAlternate = LinearVec4(0.0f, 0.0f, 0.0f, 0.03f);
    c.textSelected = LinearVec4(0.55f, 0.85f, 0.75f, 0.60f);
    c.modalDim = LinearVec4(0.10f, 0.12f, 0.14f, 0.45f);
    c.overlayBase = LinearVec4(0.97f, 0.98f, 0.99f, 1.00f);

    c.heroSkyTop = LinearVec4(0.20f, 0.40f, 0.82f, 1.00f);
    c.heroSkyMid = LinearVec4(0.48f, 0.69f, 0.94f, 1.00f);
    c.heroSkyBottom = LinearVec4(0.91f, 0.96f, 1.00f, 1.00f);
    c.heroLight = LinearVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c.heroIslandFar = LinearVec4(0.59f, 0.69f, 0.83f, 1.00f);
    c.heroIslandNear = LinearVec4(0.31f, 0.41f, 0.56f, 1.00f);
    c.heroGrassFar = LinearVec4(0.50f, 0.68f, 0.61f, 1.00f);
    c.heroGrassNear = LinearVec4(0.25f, 0.45f, 0.37f, 1.00f);

    c.heroNightTop = LinearVec4(0.05f, 0.08f, 0.19f, 1.00f);
    c.heroNightBottom = LinearVec4(0.26f, 0.31f, 0.50f, 1.00f);
    c.heroStar = LinearVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c.heroMoon = LinearVec4(0.97f, 0.98f, 1.00f, 1.00f);
    c.heroWarmSkyTop = LinearVec4(0.97f, 0.64f, 0.39f, 1.00f);
    c.heroWarmSkyBottom = LinearVec4(1.00f, 0.91f, 0.76f, 1.00f);
    c.heroSandFar = LinearVec4(0.92f, 0.79f, 0.58f, 1.00f);
    c.heroSandNear = LinearVec4(0.83f, 0.64f, 0.41f, 1.00f);

    c.logoSkyTop = LinearVec4(0.04f, 0.12f, 0.22f, 1.00f);
    c.logoSkyBottom = LinearVec4(0.18f, 0.48f, 0.84f, 1.00f);
    c.logoSun = LinearVec4(1.00f, 0.82f, 0.40f, 1.00f);
    c.logoInk = LinearVec4(1.00f, 1.00f, 1.00f, 1.00f);

    c.progressTrack = LinearVec4(0.85f, 0.87f, 0.90f, 1.00f);
    return c;
}

inline const Colors& colors() {
    static const Colors dark = darkColors();
    static const Colors light = lightColors();
    return isLight() ? light : dark;
}

// The individual colors the windows reach for by hand.
inline LinearVec4 accent() { return colors().accent; }
inline LinearVec4 accentStrong() { return colors().accentStrong; }
inline LinearVec4 text() { return colors().text; }
inline LinearVec4 muted() { return colors().muted; }
inline LinearVec4 ok() { return colors().ok; }
inline LinearVec4 error() { return colors().error; }
inline LinearVec4 warning() { return colors().warning; }

// The mark's own colours, which are the same on both palettes.
inline LinearVec4 logoSkyTop() { return colors().logoSkyTop; }
inline LinearVec4 logoSkyBottom() { return colors().logoSkyBottom; }
inline LinearVec4 logoSun() { return colors().logoSun; }
inline LinearVec4 logoInk() { return colors().logoInk; }

inline LinearVec4 background() { return colors().background; }
inline LinearVec4 panel() { return colors().panel; }
inline LinearVec4 border() { return colors().border; }

// Buttons: a primary action in the accent, a quiet secondary one, and red for
// the action that leaves the game or removes something.
inline LinearVec4 primaryButton() { return colors().primaryButton; }
inline LinearVec4 secondaryButton() { return colors().secondaryButton; }
inline LinearVec4 dangerButton() { return colors().dangerButton; }

// A panel that floats over the 3D view; the caller picks the alpha.
inline LinearVec4 overlay(float alpha) {
    LinearVec4 color = colors().overlayBase;
    color.w = alpha;
    return color;
}

// Colors only, for a window that has its own idea of spacing: the game client
// keeps its rounder HUD but speaks the same palette as the tools.
inline void applyColors(LinearUIStyle& style) {
    const Colors& c = colors();
    LinearVec4* col = style.Colors;
    col[LinearUICol_Text] = c.text;
    col[LinearUICol_TextDisabled] = c.muted;
    col[LinearUICol_WindowBg] = c.background;
    col[LinearUICol_ChildBg] = c.panel;
    col[LinearUICol_PopupBg] = c.popup;
    col[LinearUICol_Border] = c.border;
    col[LinearUICol_BorderShadow] = LinearVec4(0.0f, 0.0f, 0.0f, 0.0f);
    col[LinearUICol_FrameBg] = c.frame;
    col[LinearUICol_FrameBgHovered] = c.frameHovered;
    col[LinearUICol_FrameBgActive] = c.frameActive;
    col[LinearUICol_TitleBg] = c.titleBar;
    col[LinearUICol_TitleBgActive] = c.titleBarActive;
    col[LinearUICol_TitleBgCollapsed] = c.titleBar;
    col[LinearUICol_MenuBarBg] = c.menuBar;
    col[LinearUICol_ScrollbarBg] = c.scrollBackground;
    col[LinearUICol_ScrollbarGrab] = c.scrollGrab;
    col[LinearUICol_ScrollbarGrabHovered] = c.scrollGrabHovered;
    col[LinearUICol_ScrollbarGrabActive] = c.accentStrong;
    col[LinearUICol_CheckMark] = c.accentStrong;
    col[LinearUICol_SliderGrab] = c.accentStrong;
    col[LinearUICol_SliderGrabActive] = c.accentStrong;
    col[LinearUICol_Button] = c.secondaryButton;
    col[LinearUICol_ButtonHovered] = c.buttonHovered;
    col[LinearUICol_ButtonActive] = c.buttonActive;
    col[LinearUICol_Header] = c.header;
    col[LinearUICol_HeaderHovered] = c.headerHovered;
    col[LinearUICol_HeaderActive] = c.headerActive;
    col[LinearUICol_Separator] = c.border;
    col[LinearUICol_SeparatorHovered] = c.separatorHovered;
    col[LinearUICol_SeparatorActive] = c.accentStrong;
    col[LinearUICol_ResizeGrip] = c.resizeGrip;
    col[LinearUICol_ResizeGripHovered] = c.resizeGripHovered;
    col[LinearUICol_ResizeGripActive] = c.accentStrong;
    col[LinearUICol_Tab] = c.tab;
    col[LinearUICol_TabHovered] = c.tabHovered;
    col[LinearUICol_TabSelected] = c.tabSelected;
    col[LinearUICol_TabSelectedOverline] = c.accentStrong;
    col[LinearUICol_TabDimmed] = c.tabDimmed;
    col[LinearUICol_TabDimmedSelected] = c.tabDimmedSelected;
    col[LinearUICol_DockingPreview] = LinearVec4(c.accent.x, c.accent.y, c.accent.z, 0.55f);
    col[LinearUICol_DockingEmptyBg] = c.background;
    col[LinearUICol_PlotLines] = c.accentStrong;
    col[LinearUICol_PlotLinesHovered] = c.ok;
    col[LinearUICol_PlotHistogram] = c.accentStrong;
    col[LinearUICol_PlotHistogramHovered] = c.ok;
    col[LinearUICol_TableHeaderBg] = c.tableHeader;
    col[LinearUICol_TableBorderStrong] = c.border;
    col[LinearUICol_TableBorderLight] = c.tableBorderLight;
    col[LinearUICol_TableRowBg] = LinearVec4(0.0f, 0.0f, 0.0f, 0.0f);
    col[LinearUICol_TableRowBgAlt] = c.tableRowAlternate;
    col[LinearUICol_TextSelectedBg] = c.textSelected;
    col[LinearUICol_ModalWindowDimBg] = c.modalDim;
}

// The palette plus the geometry the tool windows share: square frames, softly
// rounded fields and buttons, and generous padding.
inline void apply() {
    if (isLight()) {
        LinearUI::StyleColorsLight();
    } else {
        LinearUI::StyleColorsDark();
    }

    LinearUIStyle& style = LinearUI::GetStyle();
    style.WindowRounding = 0.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.FramePadding = LinearVec2(10.0f, 6.0f);
    style.ItemSpacing = LinearVec2(10.0f, 8.0f);
    applyColors(style);
}

}  // namespace theme
}  // namespace ui
}  // namespace linear
