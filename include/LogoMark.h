#pragma once

#include "UiTheme.h"

// The Linear mark, drawn as five shapes: a tile of sky over water, the sun above
// the horizon, and an L standing on it. It lives in a header rather than in one
// window's source because the client's sign-in panel, the installer, the updater,
// the server setup and the certificate tool all show it, and each of those is a
// separate executable.
//
// Nothing is read from disk: a window can put the mark on screen with no artwork
// beside it, which is the same choice the installer's art panel makes. The two
// copies of the same geometry are web/public/assets/linear-mark.svg for the
// browser and the icon files scripts/make-logo.py draws from it - all three use
// the coordinates below.
namespace linear {
namespace ui {
namespace logo {

// `pos` is the top left corner of the tile and `size` its side, so the mark can
// be drawn at 28 pixels in a panel corner or at 64 beside the installer's name.
inline void drawMark(LinearDrawList* draw, const LinearVec2& pos, float size) {
    const auto color = [](const LinearVec4& value) { return LinearUI::GetColorU32(value); };
    const LinearVec2 corner(pos.x + size, pos.y + size);
    const float radius = size * 0.22f;

    // Two bands meeting in a hard edge, which is the horizon: the same two-quad
    // sky the installer's art panel draws, and the reason the mark reads as a
    // world rather than as a letter in a box.
    const float horizon = pos.y + size * 0.55f;
    draw->AddRectFilled(pos, LinearVec2(corner.x, horizon), color(theme::logoSkyTop()), radius,
                        LinearDrawFlags_RoundCornersTop);
    draw->AddRectFilled(LinearVec2(pos.x, horizon), corner, color(theme::logoSkyBottom()), radius,
                        LinearDrawFlags_RoundCornersBottom);

    // The letter: an upright and its arm, both rounded, because an icon has to
    // survive being scaled down to sixteen pixels in a title bar.
    const float softness = size * 0.032f;
    draw->AddRectFilled(LinearVec2(pos.x + size * 0.27f, pos.y + size * 0.23f),
                        LinearVec2(pos.x + size * 0.41f, pos.y + size * 0.73f),
                        color(theme::logoInk()), softness);
    draw->AddRectFilled(LinearVec2(pos.x + size * 0.27f, pos.y + size * 0.59f),
                        LinearVec2(pos.x + size * 0.71f, pos.y + size * 0.73f),
                        color(theme::logoInk()), softness);

    // The sun, sitting above the end of the arm so the two shapes belong to each
    // other instead of being a letter and a dot.
    draw->AddCircleFilled(LinearVec2(pos.x + size * 0.68f, pos.y + size * 0.33f), size * 0.115f,
                          color(theme::logoSun()), 48);
}

}  // namespace logo
}  // namespace ui
}  // namespace linear
