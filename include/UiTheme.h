#pragma once

#include "imgui.h"

// One look for every window this project puts on screen. The installer and the
// updater draw it first; Studio, RBLXCode, the server console and the game
// client read the same values from here instead of each keeping a copy, so the
// palette cannot drift apart window by window.
namespace rblx {
namespace ui {
namespace theme {

inline ImVec4 accent() { return ImVec4(0.42f, 0.86f, 0.72f, 1.00f); }
inline ImVec4 text() { return ImVec4(0.82f, 0.84f, 0.88f, 1.00f); }
inline ImVec4 muted() { return ImVec4(0.55f, 0.58f, 0.64f, 1.00f); }
inline ImVec4 ok() { return ImVec4(0.42f, 0.86f, 0.60f, 1.00f); }
inline ImVec4 error() { return ImVec4(0.96f, 0.45f, 0.45f, 1.00f); }
inline ImVec4 warning() { return ImVec4(0.95f, 0.72f, 0.28f, 1.00f); }

inline ImVec4 background() { return ImVec4(0.07f, 0.08f, 0.10f, 1.00f); }
inline ImVec4 panel() { return ImVec4(0.05f, 0.06f, 0.07f, 1.00f); }
inline ImVec4 border() { return ImVec4(0.18f, 0.20f, 0.24f, 1.00f); }

// Buttons: a mint primary action, a quiet secondary one, and red for the action
// that leaves the game or removes something.
inline ImVec4 primaryButton() { return ImVec4(0.20f, 0.50f, 0.44f, 1.00f); }
inline ImVec4 secondaryButton() { return ImVec4(0.16f, 0.19f, 0.23f, 1.00f); }
inline ImVec4 dangerButton() { return ImVec4(0.55f, 0.22f, 0.24f, 1.00f); }

// Colors only, for a window that has its own idea of spacing: the game client
// keeps its rounder HUD but speaks the same palette as the tools.
inline void applyColors(ImGuiStyle& style) {
    ImVec4* c = style.Colors;
    c[ImGuiCol_Text] = text();
    c[ImGuiCol_TextDisabled] = muted();
    c[ImGuiCol_WindowBg] = background();
    c[ImGuiCol_ChildBg] = panel();
    c[ImGuiCol_PopupBg] = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
    c[ImGuiCol_Border] = border();
    c[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.15f, 0.18f, 1.00f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.21f, 0.25f, 1.00f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.24f, 0.28f, 1.00f);
    c[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.07f, 0.09f, 1.00f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.09f, 0.11f, 0.13f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.06f, 0.07f, 0.09f, 1.00f);
    c[ImGuiCol_MenuBarBg] = ImVec4(0.06f, 0.07f, 0.09f, 1.00f);
    c[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.06f, 0.07f, 1.00f);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.24f, 0.28f, 0.32f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.36f, 0.40f, 1.00f);
    c[ImGuiCol_ScrollbarGrabActive] = accent();
    c[ImGuiCol_CheckMark] = accent();
    c[ImGuiCol_SliderGrab] = accent();
    c[ImGuiCol_SliderGrabActive] = accent();
    c[ImGuiCol_Button] = secondaryButton();
    c[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.32f, 0.38f, 1.00f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.42f, 0.46f, 1.00f);
    c[ImGuiCol_Header] = ImVec4(0.18f, 0.28f, 0.32f, 1.00f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.34f, 0.38f, 1.00f);
    c[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.40f, 0.44f, 1.00f);
    c[ImGuiCol_Separator] = border();
    c[ImGuiCol_SeparatorHovered] = ImVec4(0.24f, 0.34f, 0.36f, 1.00f);
    c[ImGuiCol_SeparatorActive] = accent();
    c[ImGuiCol_ResizeGrip] = ImVec4(0.18f, 0.28f, 0.32f, 0.60f);
    c[ImGuiCol_ResizeGripHovered] = ImVec4(0.24f, 0.40f, 0.44f, 0.80f);
    c[ImGuiCol_ResizeGripActive] = accent();
    c[ImGuiCol_Tab] = ImVec4(0.10f, 0.12f, 0.14f, 1.00f);
    c[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.34f, 0.38f, 1.00f);
    c[ImGuiCol_TabSelected] = ImVec4(0.18f, 0.28f, 0.32f, 1.00f);
    c[ImGuiCol_TabSelectedOverline] = accent();
    c[ImGuiCol_TabDimmed] = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
    c[ImGuiCol_TabDimmedSelected] = ImVec4(0.14f, 0.20f, 0.22f, 1.00f);
    c[ImGuiCol_DockingPreview] = ImVec4(0.42f, 0.86f, 0.72f, 0.55f);
    c[ImGuiCol_DockingEmptyBg] = background();
    c[ImGuiCol_PlotLines] = accent();
    c[ImGuiCol_PlotLinesHovered] = ok();
    c[ImGuiCol_PlotHistogram] = accent();
    c[ImGuiCol_PlotHistogramHovered] = ok();
    c[ImGuiCol_TableHeaderBg] = ImVec4(0.10f, 0.12f, 0.14f, 1.00f);
    c[ImGuiCol_TableBorderStrong] = border();
    c[ImGuiCol_TableBorderLight] = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
    c[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.03f);
    c[ImGuiCol_TextSelectedBg] = ImVec4(0.24f, 0.44f, 0.42f, 0.60f);
    c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.02f, 0.03f, 0.04f, 0.60f);
}

// The palette plus the geometry the tool windows share: square frames, softly
// rounded fields and buttons, and generous padding.
inline void apply() {
    ImGui::StyleColorsDark();
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
