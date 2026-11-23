#include "InstallUi.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>

#include <lineargl/lineargl.h>
#include <LINEARWIN/linearwin.h>

#include "linearui.h"
#include "linearui_impl_linearwin.h"
#include "linearui_impl_opengl3.h"
#include "LogoMark.h"
#include "UiTheme.h"

namespace linear {
namespace ui {
namespace {

// The window is laid out like a product page rather than a dialog: the brand,
// the steps the job is taking and its log in a column on the left, a drawn art
// panel on the right, and one bar across the bottom that says what is happening
// and how far in it is. The sizes are in pixels because the window has no title
// bar of its own and cannot be resized, so nothing has to reflow.
constexpr int kWindowWidth = 1160;
constexpr int kWindowHeight = 760;
constexpr std::size_t kMaxLines = 400;

constexpr float kPadding = 38.0f;
constexpr float kSidebarShare = 0.365f;
constexpr float kBottomHeight = 172.0f;
constexpr float kStepRow = 68.0f;

// The palette itself lives in UiTheme.h, because every window of this project
// reads it from there and a change in one place should reach all of them.
const LinearVec4 kAccent = theme::accent();
const LinearVec4 kText = theme::text();
const LinearVec4 kMuted = theme::muted();
const LinearVec4 kError = theme::error();
const LinearVec4 kOk = theme::ok();
const LinearVec4 kBorder = theme::border();

void applyTheme() { theme::apply(); }

// One regular face and one bold face are enough for every size the window uses:
// a font is rasterised at whatever size it is pushed at, so the wordmark, the
// step labels and the log are the same two files. Falls back to the built-in
// bitmap font when there is no font file, which is cosmetic and not a reason to
// refuse to install.
struct Fonts {
    LinearFont* regular = nullptr;
    LinearFont* bold = nullptr;
};

bool fontExists(const char* path) {
    std::ifstream probe(path, std::ios::binary);
    return probe.good();
}

void loadFonts(Fonts* fonts) {
    LinearUIIO& io = LinearUI::GetIO();
    LinearFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 2;

    const char* const regularFiles[] = {"C:\\Windows\\Fonts\\segoeui.ttf",
                                        "C:\\Windows\\Fonts\\arial.ttf"};
    const char* const boldFiles[] = {"C:\\Windows\\Fonts\\segoeuib.ttf",
                                     "C:\\Windows\\Fonts\\arialbd.ttf",
                                     "C:\\Windows\\Fonts\\segoeui.ttf",
                                     "C:\\Windows\\Fonts\\arial.ttf"};

    // The regular face is added first so it is also the default font: the
    // buttons, the folder field and the checkboxes draw in it.
    for (const char* path : regularFiles) {
        if (!fontExists(path)) continue;
        fonts->regular = io.Fonts->AddFontFromFileTTF(path, 17.0f, &config);
        if (fonts->regular != nullptr) break;
    }
    if (fonts->regular == nullptr) fonts->regular = io.Fonts->AddFontDefault();

    for (const char* path : boldFiles) {
        if (!fontExists(path)) continue;
        fonts->bold = io.Fonts->AddFontFromFileTTF(path, 19.0f, &config);
        if (fonts->bold != nullptr) break;
    }
    if (fonts->bold == nullptr) fonts->bold = fonts->regular;
}

// "512 KB", "12.4 MB": sizes are read at a glance, and a raw byte count is not.
std::string formatBytes(long long bytes) {
    const char* const units[] = {"B", "KB", "MB", "GB"};
    double value = static_cast<double>(bytes < 0 ? 0 : bytes);
    int unit = 0;
    while (value >= 1024.0 && unit < 3) {
        value /= 1024.0;
        ++unit;
    }
    char text[32] = {};
    if (unit == 0) {
        std::snprintf(text, sizeof(text), "%.0f %s", value, units[unit]);
    } else {
        std::snprintf(text, sizeof(text), "%.1f %s", value, units[unit]);
    }
    return text;
}

std::string waitingDots() {
    const int count = static_cast<int>(LinearUI::GetTime() * 2.0) % 4;
    return std::string(static_cast<std::size_t>(count), '.');
}

float clamp01(float value) { return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value); }

LinearVec4 mix(const LinearVec4& from, const LinearVec4& to, float amount) {
    return LinearVec4(from.x + (to.x - from.x) * amount, from.y + (to.y - from.y) * amount,
                      from.z + (to.z - from.z) * amount, from.w + (to.w - from.w) * amount);
}

// A colour toward white, for the faces of the islands the sun reaches.
LinearVec4 lighten(const LinearVec4& color, float amount) {
    return LinearVec4(color.x + (1.0f - color.x) * amount, color.y + (1.0f - color.y) * amount,
                      color.z + (1.0f - color.z) * amount, color.w);
}

LinearVec4 darken(const LinearVec4& color, float amount) {
    return LinearVec4(color.x * (1.0f - amount), color.y * (1.0f - amount),
                      color.z * (1.0f - amount), color.w);
}

LinearU32 colorOf(const LinearVec4& color) { return LinearUI::GetColorU32(color); }

LinearU32 tint(const LinearVec4& color, float alpha) {
    LinearVec4 faded = color;
    faded.w *= clamp01(alpha);
    return LinearUI::GetColorU32(faded);
}

// A rectangle in screen pixels, in the order a draw list wants it: two corners
// rather than a size, because that is what every primitive takes.
struct ArtRect {
    LinearVec2 min;
    LinearVec2 max;
};

// A point given as a fraction of a rectangle, which is how the art is placed so
// it keeps its composition at any window size.
LinearVec2 pointIn(const ArtRect& rect, float x, float y) {
    return LinearVec2(rect.min.x + (rect.max.x - rect.min.x) * x,
                      rect.min.y + (rect.max.y - rect.min.y) * y);
}

// Draws one line at an absolute position and returns its width, so a caller can
// put a second word beside it.
float writeText(LinearFont* font, float size, const LinearVec4& color, const LinearVec2& pos,
                const std::string& value) {
    LinearUI::PushFont(font, size);
    LinearUI::SetCursorScreenPos(pos);
    LinearUI::TextColored(color, "%s", value.c_str());
    const float width = LinearUI::CalcTextSize(value.c_str()).x;
    LinearUI::PopFont();
    return width;
}

// A paragraph that wraps at the right edge of the child it is drawn in, which is
// what keeps a long explanation inside the left column.
void writeWrapped(LinearFont* font, float size, const LinearVec4& color, const LinearVec2& pos,
                  const std::string& value) {
    LinearUI::PushFont(font, size);
    LinearUI::SetCursorScreenPos(pos);
    LinearUI::PushTextWrapPos(0.0f);
    LinearUI::TextColored(color, "%s", value.c_str());
    LinearUI::PopTextWrapPos();
    LinearUI::PopFont();
}

float measureWrapped(LinearFont* font, float size, const std::string& value, float width) {
    LinearUI::PushFont(font, size);
    const LinearVec2 measured = LinearUI::CalcTextSize(value.c_str(), nullptr, false, width);
    LinearUI::PopFont();
    return measured.y;
}

// A button in a colour of its own, hovered toward the light on the dark palette
// and toward the dark on the light one: a hover that washes a pale button out
// reads as disabled.
bool drawButton(const std::string& label, const LinearVec4& color, const LinearVec2& size) {
    const float step = theme::isLight() ? -0.07f : 0.08f;
    LinearUI::PushStyleColor(LinearUICol_Button, color);
    LinearUI::PushStyleColor(LinearUICol_ButtonHovered,
                             LinearVec4(std::min(std::max(color.x + step, 0.0f), 1.0f),
                                        std::min(std::max(color.y + step, 0.0f), 1.0f),
                                        std::min(std::max(color.z + step, 0.0f), 1.0f), 1.00f));
    LinearUI::PushStyleColor(LinearUICol_ButtonActive, color);
    const bool pressed = LinearUI::Button(label.c_str(), size);
    LinearUI::PopStyleColor(3);
    return pressed;
}

// The brand block: the mark, the product name with its optional accent word, the
// tagline and the rule under them. Returns the y a caller starts the steps or the
// form at, so the column is measured in one place.
float drawBrand(LinearDrawList* draw, const Fonts& fonts, const Options& options, float left,
                const LinearVec2& origin, float contentWidth) {
    float y = origin.y + 34.0f;
    // The mark from include/LogoMark.h: every window of this project shows the
    // same one, drawn rather than loaded so each stays a single executable.
    logo::drawMark(draw, LinearVec2(left, y), 64.0f);
    y += 64.0f + 26.0f;

    const float productWidth =
        writeText(fonts.bold, 30.0f, kText, LinearVec2(left, y), options.product);
    if (!options.productAccent.empty()) {
        writeText(fonts.bold, 30.0f, kAccent, LinearVec2(left + productWidth + 9.0f, y),
                  options.productAccent);
    }
    y += 40.0f;

    if (!options.tagline.empty()) {
        writeText(fonts.regular, 15.0f, kMuted, LinearVec2(left, y), options.tagline);
        y += 22.0f;
    }

    y += 14.0f;
    draw->AddLine(LinearVec2(left, y), LinearVec2(left + contentWidth, y), colorOf(kBorder), 1.0f);
    return y + 26.0f;
}

// A cloud drifting in the sky. `phase` and `time` give it its own slow slide, and
// `speed` is how quickly it does it: the ones drawn near have to move further in the
// same second, or the whole panel reads as one flat sheet. The slide is a sine rather
// than a crossing, so nothing ever jumps back to the other side on screen.
void drawCloud(LinearDrawList* draw, const ArtRect& rect, float x, float y, float scale,
               float phase, float speed, float time, const LinearVec4& light, float alpha) {
    const float width = (rect.max.x - rect.min.x) * scale;
    const float drift = std::sin(time * speed + phase) * (0.012f + alpha * 0.075f);
    const float hover = std::sin(time * speed * 1.7f + phase * 2.3f) * 0.005f;
    const LinearVec2 center = pointIn(rect, x + drift, y + hover);
    for (int puff = 0; puff < 6; ++puff) {
        const float offset = (static_cast<float>(puff) - 2.5f) * width * 0.42f;
        const float radius = width * (0.26f + 0.05f * static_cast<float>((puff * 7) % 4));
        const float lift = (puff % 2 == 0) ? -radius * 0.18f : radius * 0.07f;
        draw->AddCircleFilled(LinearVec2(center.x + offset, center.y + lift), radius,
                              tint(light, alpha), 32);
    }
}

// One floating island: a rock that narrows as it goes down, a grass cap, three
// trees and the water falling off the far side. `depth` fades it into the sky so
// the ones drawn first read as distant, and also sets how far it drifts: the near
// islands move more than the far ones.
void drawIsland(LinearDrawList* draw, const ArtRect& rect, float x, float y, float scale,
                float depth, float phase, float time, const theme::Colors& palette) {
    const float full = rect.max.x - rect.min.x;
    const float width = full * 0.26f * scale;
    const float height = width * 0.62f;
    // A slow bob and an even slower sway, out of step per island: one motion shared
    // by all of them would read as the window shaking, not as islands in the sky.
    const float bob = std::sin(time * 0.5f + phase) * 0.011f * (0.6f + depth * 0.8f);
    const float sway = std::sin(time * 0.19f + phase * 1.7f) * 0.009f;
    const LinearVec2 top = pointIn(rect, x + sway, y + bob);

    const LinearVec4 rock = mix(palette.heroIslandFar, palette.heroIslandNear, depth);
    const LinearVec4 grass = mix(palette.heroGrassFar, palette.heroGrassNear, depth);

    draw->AddQuadFilled(LinearVec2(top.x - width * 0.50f, top.y),
                        LinearVec2(top.x + width * 0.50f, top.y),
                        LinearVec2(top.x + width * 0.10f, top.y + height),
                        LinearVec2(top.x - width * 0.06f, top.y + height), colorOf(rock));
    draw->AddQuadFilled(LinearVec2(top.x - width * 0.50f, top.y),
                        LinearVec2(top.x - width * 0.02f, top.y),
                        LinearVec2(top.x + width * 0.02f, top.y + height * 0.90f),
                        LinearVec2(top.x - width * 0.06f, top.y + height * 0.90f),
                        colorOf(lighten(rock, 0.22f)));

    draw->AddRectFilled(LinearVec2(top.x - width * 0.54f, top.y - height * 0.11f),
                        LinearVec2(top.x + width * 0.54f, top.y + height * 0.06f),
                        colorOf(grass), height * 0.06f);

    const LinearVec4 leaf = darken(grass, 0.28f);
    for (int tree = -1; tree <= 1; ++tree) {
        const float lean = static_cast<float>(tree);
        const LinearVec2 base(top.x + lean * width * 0.24f, top.y - height * 0.05f);
        const float tall = height * (0.34f - 0.05f * (lean < 0.0f ? -lean : lean));
        // The trees bend one after another, never quite together.
        const float bend = std::sin(time * 0.9f + phase + lean * 0.7f) * width * 0.02f;
        draw->AddTriangleFilled(LinearVec2(base.x - width * 0.075f, base.y),
                                LinearVec2(base.x + width * 0.075f, base.y),
                                LinearVec2(base.x + bend, base.y - tall), colorOf(leaf));
    }

    // The fall shimmers, and reaches a little further down on each swell.
    const float shimmer = 0.70f + 0.14f * std::sin(time * 1.6f + phase * 2.3f);
    const float reach = 1.20f + 0.05f * std::sin(time * 1.1f + phase);
    const LinearVec2 fallStart(top.x + width * 0.24f, top.y + height * 0.02f);
    const LinearVec2 fallEnd(fallStart.x + width * 0.13f, fallStart.y + height * reach);
    draw->AddRectFilledMultiColor(fallStart, fallEnd, tint(palette.heroLight, shimmer),
                                  tint(palette.heroLight, shimmer), tint(palette.heroLight, 0.0f),
                                  tint(palette.heroLight, 0.0f));
}

// ---------------------------------------------------------------------------
// The scenes in the art panel
//
// Four drawn pictures take turns there: the islands, a city skyline, dunes and a
// night sky. They share the helpers below so each one is a short list of shapes,
// and all of them move on `time` - seconds since the window opened - so the panel
// is alive while the job runs without any of it asking for attention.

struct SkylineBlock {
    float x;      // left edge, as a fraction of the panel's width
    float width;  // how wide it is, the same way
    float top;    // where the roof sits, as a fraction of the panel's height
};

// Two bands rather than one quad, so the colour holds near the top and opens up
// lower down instead of washing out through the middle.
void drawSky(LinearDrawList* draw, const ArtRect& rect, const LinearVec4& top,
             const LinearVec4& mid, const LinearVec4& bottom) {
    const float split = rect.min.y + (rect.max.y - rect.min.y) * 0.58f;
    draw->AddRectFilledMultiColor(rect.min, LinearVec2(rect.max.x, split), colorOf(top),
                                  colorOf(top), colorOf(mid), colorOf(mid));
    draw->AddRectFilledMultiColor(LinearVec2(rect.min.x, split), rect.max, colorOf(mid),
                                  colorOf(mid), colorOf(bottom), colorOf(bottom));
}

// A light that breathes: a stack of discs that get brighter as they shrink, the
// whole stack swelling a little so the sky never sits perfectly still.
void drawLight(LinearDrawList* draw, const ArtRect& rect, const LinearVec4& light, float time,
               float x, float y, float radius, float rate) {
    const float width = rect.max.x - rect.min.x;
    const float breath = 0.5f + 0.5f * std::sin(time * rate);
    const LinearVec2 center = pointIn(rect, x, y);
    for (int ring = 8; ring >= 1; --ring) {
        const float fraction = static_cast<float>(ring) / 8.0f;
        draw->AddCircleFilled(center, width * radius * (0.92f + 0.08f * breath) * fraction,
                              tint(light, 0.035f + 0.10f * (1.0f - fraction) + 0.02f * breath), 64);
    }
}

// The haze along the bottom, so a scene ends in light instead of at a hard line.
void drawHaze(LinearDrawList* draw, const ArtRect& rect, const LinearVec4& light, float strength) {
    const float height = rect.max.y - rect.min.y;
    draw->AddRectFilledMultiColor(LinearVec2(rect.min.x, rect.max.y - height * 0.28f), rect.max,
                                  tint(light, 0.0f), tint(light, 0.0f), tint(light, strength),
                                  tint(light, strength));
}

// A dune, a hill or a bank of sand: a circle big enough that only its crown shows
// above the bottom edge, which is the only curve a draw list can fill.
void drawCrown(LinearDrawList* draw, const ArtRect& rect, const LinearVec4& color, float centerX,
               float radiusShare, float riseShare) {
    const float width = rect.max.x - rect.min.x;
    const float height = rect.max.y - rect.min.y;
    const float radius = width * radiusShare;
    const LinearVec2 center(rect.min.x + width * centerX, rect.max.y + radius - height * riseShare);
    draw->AddCircleFilled(center, radius, colorOf(color), 96);
}

// One row of buildings standing on the bottom edge. With `windows`, every window
// has its own moment, so the city flickers instead of glowing evenly.
void drawSkyline(LinearDrawList* draw, const ArtRect& rect, const LinearVec4& color,
                 const SkylineBlock* blocks, int count, float time, bool windows,
                 const LinearVec4& windowColor) {
    const float width = rect.max.x - rect.min.x;
    const float height = rect.max.y - rect.min.y;

    for (int index = 0; index < count; ++index) {
        const SkylineBlock& block = blocks[index];
        const float left = rect.min.x + width * block.x;
        const float right = left + width * block.width;
        const float roof = rect.min.y + height * block.top;

        draw->AddRectFilled(LinearVec2(left, roof), LinearVec2(right, rect.max.y), colorOf(color));
        // A lit edge down the left side, so a rectangle reads as a block with a face.
        draw->AddRectFilled(LinearVec2(left, roof), LinearVec2(left + width * 0.006f, rect.max.y),
                            colorOf(lighten(color, 0.20f)));

        if (!windows) continue;

        const float margin = width * 0.010f;
        const float size = width * 0.007f;
        int column = 0;
        for (float windowX = left + margin; windowX + size < right - margin;
             windowX += size * 2.4f, ++column) {
            int row = 0;
            for (float windowY = roof + margin; windowY + size < rect.max.y - margin;
                 windowY += size * 2.9f, ++row) {
                const float mood = std::sin(
                    time * 0.8f + static_cast<float>(index * 7 + column * 3 + row * 11));
                draw->AddRectFilled(LinearVec2(windowX, windowY),
                                    LinearVec2(windowX + size, windowY + size),
                                    tint(windowColor, mood > 0.15f ? 0.72f : 0.18f));
            }
        }
    }
}

// A cactus: a trunk and two arms, each arm a piece across and a piece up.
void drawCactus(LinearDrawList* draw, const ArtRect& rect, float x, float base, float scale,
                const LinearVec4& color) {
    const float width = rect.max.x - rect.min.x;
    const float height = rect.max.y - rect.min.y;
    const float trunk = width * 0.014f * scale;
    const float tall = height * 0.15f * scale;
    const LinearVec2 foot = pointIn(rect, x, base);

    draw->AddRectFilled(LinearVec2(foot.x - trunk * 0.5f, foot.y - tall),
                        LinearVec2(foot.x + trunk * 0.5f, foot.y), colorOf(color), trunk * 0.5f);

    for (int side = -1; side <= 1; side += 2) {
        const LinearVec2 elbow(foot.x + static_cast<float>(side) * trunk * 1.7f,
                               foot.y - tall * 0.62f);
        draw->AddRectFilled(LinearVec2(std::min(elbow.x, foot.x), elbow.y - trunk * 0.5f),
                            LinearVec2(std::max(elbow.x, foot.x), elbow.y + trunk * 0.5f),
                            colorOf(color), trunk * 0.5f);
        draw->AddRectFilled(LinearVec2(elbow.x - trunk * 0.5f, elbow.y - tall * 0.26f),
                            LinearVec2(elbow.x + trunk * 0.5f, elbow.y + trunk * 0.5f),
                            colorOf(color), trunk * 0.5f);
    }
}

// Scene one: the islands. A day sky, a low sun, clouds, and four islands whose
// grass, trees and waterfalls all move.
void drawIslandScene(LinearDrawList* draw, const ArtRect& rect, float time) {
    const theme::Colors& palette = theme::colors();
    drawSky(draw, rect, palette.heroSkyTop, palette.heroSkyMid, palette.heroSkyBottom);
    drawLight(draw, rect, palette.heroLight, time, 0.74f, 0.15f, 0.26f, 0.7f);

    // Far clouds, then the islands, then near clouds: depth here is nothing but the
    // order things are drawn in, which is what lets a near cloud slide across a far
    // island.
    drawCloud(draw, rect, 0.16f, 0.36f, 0.20f, 0.0f, 0.13f, time, palette.heroLight, 0.26f);
    drawCloud(draw, rect, 0.52f, 0.30f, 0.16f, 1.9f, 0.16f, time, palette.heroLight, 0.22f);
    drawCloud(draw, rect, 0.88f, 0.42f, 0.22f, 3.7f, 0.11f, time, palette.heroLight, 0.28f);

    drawIsland(draw, rect, 0.34f, 0.46f, 0.70f, 0.30f, 0.0f, time, palette);
    drawIsland(draw, rect, 0.66f, 0.33f, 0.50f, 0.15f, 2.1f, time, palette);
    drawIsland(draw, rect, 0.74f, 0.62f, 0.95f, 0.80f, 4.2f, time, palette);
    drawIsland(draw, rect, 0.18f, 0.74f, 1.05f, 1.00f, 5.6f, time, palette);

    drawCloud(draw, rect, 0.30f, 0.80f, 0.30f, 2.6f, 0.17f, time, palette.heroLight, 0.42f);
    drawCloud(draw, rect, 0.82f, 0.84f, 0.34f, 5.1f, 0.19f, time, palette.heroLight, 0.45f);

    drawHaze(draw, rect, palette.heroLight, 0.62f);
}

// Scene two: a city on the same day sky. Two rows of blocks, the far one pale and
// still, the near one dark with windows that come and go, and a beacon on the
// tallest roof.
void drawCityScene(LinearDrawList* draw, const ArtRect& rect, float time) {
    const theme::Colors& palette = theme::colors();

    static const SkylineBlock kFar[] = {
        {0.00f, 0.10f, 0.30f}, {0.09f, 0.07f, 0.24f}, {0.15f, 0.12f, 0.36f},
        {0.26f, 0.08f, 0.27f}, {0.33f, 0.11f, 0.42f}, {0.43f, 0.07f, 0.29f},
        {0.49f, 0.13f, 0.35f}, {0.61f, 0.09f, 0.25f}, {0.69f, 0.12f, 0.39f},
        {0.80f, 0.08f, 0.28f}, {0.87f, 0.13f, 0.34f},
    };
    static const SkylineBlock kNear[] = {
        {0.03f, 0.15f, 0.52f}, {0.20f, 0.12f, 0.63f}, {0.34f, 0.17f, 0.47f},
        {0.54f, 0.14f, 0.58f}, {0.70f, 0.19f, 0.44f},
    };
    const int farCount = static_cast<int>(sizeof(kFar) / sizeof(kFar[0]));
    const int nearCount = static_cast<int>(sizeof(kNear) / sizeof(kNear[0]));

    drawSky(draw, rect, palette.heroSkyTop, palette.heroSkyMid, palette.heroSkyBottom);
    drawLight(draw, rect, palette.heroLight, time, 0.18f, 0.14f, 0.22f, 0.55f);

    drawCloud(draw, rect, 0.30f, 0.20f, 0.22f, 1.2f, 0.14f, time, palette.heroLight, 0.22f);
    drawCloud(draw, rect, 0.72f, 0.12f, 0.18f, 4.4f, 0.12f, time, palette.heroLight, 0.20f);

    drawSkyline(draw, rect, palette.heroIslandFar, kFar, farCount, time, false, palette.heroLight);
    drawSkyline(draw, rect, palette.heroIslandNear, kNear, nearCount, time, true, palette.heroLight);

    // The beacon: a mast on one roof with a light that pulses, so the skyline has
    // one thing clearly alive in it.
    const float height = rect.max.y - rect.min.y;
    const LinearVec2 mast = pointIn(rect, kNear[2].x + kNear[2].width * 0.5f, kNear[2].top);
    const LinearVec2 tip(mast.x, mast.y - height * 0.11f);
    draw->AddLine(mast, tip, colorOf(palette.heroIslandNear), 2.0f);
    draw->AddCircleFilled(tip, 4.0f,
                          tint(kAccent, 0.35f + 0.65f * (0.5f + 0.5f * std::sin(time * 2.6f))), 16);

    drawHaze(draw, rect, palette.heroLight, 0.30f);
}

// Scene three: dunes under a warm sky, with cacti on the near bank and dust moving
// low across the sand.
void drawDesertScene(LinearDrawList* draw, const ArtRect& rect, float time) {
    const theme::Colors& palette = theme::colors();
    drawSky(draw, rect, palette.heroWarmSkyTop,
            mix(palette.heroWarmSkyTop, palette.heroWarmSkyBottom, 0.5f),
            palette.heroWarmSkyBottom);
    drawLight(draw, rect, palette.heroLight, time, 0.28f, 0.20f, 0.20f, 0.5f);

    drawCloud(draw, rect, 0.58f, 0.34f, 0.22f, 0.9f, 0.10f, time, palette.heroSandFar, 0.30f);

    drawCrown(draw, rect, palette.heroSandFar, 0.30f, 0.55f, 0.30f);
    drawCrown(draw, rect, palette.heroSandFar, 0.82f, 0.42f, 0.22f);
    drawCrown(draw, rect, palette.heroSandNear, 0.55f, 0.62f, 0.16f);
    drawCrown(draw, rect, palette.heroSandNear, 0.10f, 0.45f, 0.10f);

    drawCactus(draw, rect, 0.36f, 0.88f, 1.00f, palette.heroGrassNear);
    drawCactus(draw, rect, 0.72f, 0.82f, 0.68f, palette.heroGrassNear);

    drawHaze(draw, rect, palette.heroLight, 0.22f);
}

// Scene four: a night sky. Stars twinkle out of step, the moon has a halo, a
// shooting star crosses every few seconds, and two hills keep it all from floating.
void drawNightScene(LinearDrawList* draw, const ArtRect& rect, float time) {
    const theme::Colors& palette = theme::colors();
    const float width = rect.max.x - rect.min.x;

    drawSky(draw, rect, palette.heroNightTop,
            mix(palette.heroNightTop, palette.heroNightBottom, 0.5f), palette.heroNightBottom);

    for (int index = 0; index < 54; ++index) {
        const float starX = std::fmod(static_cast<float>(index) * 0.6180339887f, 1.0f);
        const float starY = std::fmod(static_cast<float>(index) * 0.4142135624f + 0.07f, 0.74f);
        const float twinkle =
            0.35f + 0.55f * (0.5f + 0.5f * std::sin(time * 1.3f + static_cast<float>(index) * 1.7f));
        draw->AddCircleFilled(pointIn(rect, starX, starY),
                              width * (0.0014f + 0.0011f * static_cast<float>(index % 3)),
                              tint(palette.heroStar, twinkle), 12);
    }

    drawLight(draw, rect, palette.heroMoon, time, 0.74f, 0.20f, 0.14f, 0.4f);
    draw->AddCircleFilled(pointIn(rect, 0.74f, 0.20f), width * 0.042f, colorOf(palette.heroMoon), 48);

    // A shooting star, once every seven seconds and gone in under one.
    const float shot = std::fmod(time, 7.0f);
    if (shot < 0.75f) {
        const float progress = shot / 0.75f;
        const LinearVec2 from = pointIn(rect, 0.10f, 0.10f);
        const LinearVec2 to = pointIn(rect, 0.52f, 0.32f);
        const LinearVec2 head(from.x + (to.x - from.x) * progress,
                              from.y + (to.y - from.y) * progress);
        const float fade = 1.0f - progress;
        const LinearVec2 tail(head.x - (to.x - from.x) * 0.16f, head.y - (to.y - from.y) * 0.16f);
        draw->AddLine(tail, head, tint(palette.heroStar, 0.55f * fade), 2.0f);
        draw->AddCircleFilled(head, 2.4f, tint(palette.heroStar, 0.9f * fade), 10);
    }

    drawCrown(draw, rect, lighten(palette.heroNightTop, 0.16f), 0.26f, 0.60f, 0.16f);
    drawCrown(draw, rect, lighten(palette.heroNightTop, 0.10f), 0.74f, 0.52f, 0.11f);

    drawHaze(draw, rect, palette.heroMoon, 0.10f);
}

// How long one scene holds, and how long the dip between two of them takes.
constexpr float kSceneSeconds = 15.0f;
constexpr float kSceneDip = 0.55f;

// `time` is seconds since the window opened. Four scenes take turns in the panel,
// and the change is hidden under a dip to the window's own colour, so nothing cuts.
void drawHero(LinearDrawList* draw, const ArtRect& rect, float time) {
    const theme::Colors& palette = theme::colors();
    draw->PushClipRect(rect.min, rect.max, true);

    const float cycle = std::fmod(time, kSceneSeconds * 4.0f);
    if (cycle < kSceneSeconds) {
        drawIslandScene(draw, rect, time);
    } else if (cycle < kSceneSeconds * 2.0f) {
        drawCityScene(draw, rect, time);
    } else if (cycle < kSceneSeconds * 3.0f) {
        drawDesertScene(draw, rect, time);
    } else {
        drawNightScene(draw, rect, time);
    }

    // The soft edge where the column ends, so the art reads as part of the window
    // rather than as an image pasted beside it.
    const float width = rect.max.x - rect.min.x;
    draw->AddRectFilledMultiColor(rect.min, LinearVec2(rect.min.x + width * 0.12f, rect.max.y),
                                  tint(palette.background, 0.55f),
                                  tint(palette.background, 0.0f), tint(palette.background, 0.0f),
                                  tint(palette.background, 0.55f));

    // The dip: opaque at the moment the scene changes, gone half a second either
    // side of it. It also fades the panel in when the window opens.
    const float local = std::fmod(time, kSceneSeconds);
    float cover = 0.0f;
    if (local < kSceneDip) {
        cover = 1.0f - local / kSceneDip;
    } else if (local > kSceneSeconds - kSceneDip) {
        cover = 1.0f - (kSceneSeconds - local) / kSceneDip;
    }
    if (cover > 0.001f) {
        draw->AddRectFilled(rect.min, rect.max, tint(palette.background, cover));
    }

    draw->PopClipRect();
}

// The circle beside a step: ticked when the work is done, a pulse while it is
// running, a cross when it failed, an empty ring when it has not started.
void drawStepMarker(LinearDrawList* draw, const LinearVec2& center, bool done, bool active,
                    bool failed) {
    const float radius = 15.0f;
    const LinearU32 mark = colorOf(theme::background());
    if (done || failed) {
        draw->AddCircleFilled(center, radius, colorOf(done ? kAccent : kError), 40);
        if (done) {
            draw->AddLine(LinearVec2(center.x - 6.0f, center.y),
                          LinearVec2(center.x - 1.5f, center.y + 5.0f), mark, 2.2f);
            draw->AddLine(LinearVec2(center.x - 1.5f, center.y + 5.0f),
                          LinearVec2(center.x + 6.5f, center.y - 5.0f), mark, 2.2f);
        } else {
            draw->AddLine(LinearVec2(center.x - 5.0f, center.y - 5.0f),
                          LinearVec2(center.x + 5.0f, center.y + 5.0f), mark, 2.2f);
            draw->AddLine(LinearVec2(center.x - 5.0f, center.y + 5.0f),
                          LinearVec2(center.x + 5.0f, center.y - 5.0f), mark, 2.2f);
        }
        return;
    }
    draw->AddCircle(center, radius, active ? colorOf(kAccent) : colorOf(kBorder), 40, 1.6f);
    if (active) draw->AddCircleFilled(center, radius - 6.0f, tint(kAccent, 0.35f), 40);
}

}  // namespace

// ---------------------------------------------------------------------------
// Report

void Report::line(const std::string& text) {
    std::lock_guard<std::mutex> lock(mutex_);
    lines_.push_back(text);
    problems_.push_back(false);
    if (lines_.size() > kMaxLines) {
        lines_.erase(lines_.begin(), lines_.begin() + 64);
        problems_.erase(problems_.begin(), problems_.begin() + 64);
    }
}

void Report::problem(const std::string& text) {
    std::lock_guard<std::mutex> lock(mutex_);
    lines_.push_back(text);
    problems_.push_back(true);
    if (lines_.size() > kMaxLines) {
        lines_.erase(lines_.begin(), lines_.begin() + 64);
        problems_.erase(problems_.begin(), problems_.begin() + 64);
    }
}

void Report::progress(float fraction) {
    std::lock_guard<std::mutex> lock(mutex_);
    fraction_ = fraction;
}

void Report::progressBytes(long long done, long long total) {
    std::lock_guard<std::mutex> lock(mutex_);
    doneBytes_ = done < 0 ? 0 : done;
    totalBytes_ = total;
    if (total > 0) {
        const double ratio = static_cast<double>(doneBytes_) / static_cast<double>(total);
        fraction_ = static_cast<float>(ratio < 0.0 ? 0.0 : (ratio > 1.0 ? 1.0 : ratio));
    } else {
        fraction_ = -1.0f;
    }
}

void Report::stage(int index) {
    std::lock_guard<std::mutex> lock(mutex_);
    stage_ = index < 0 ? 0 : index;
}

void Report::cancel() {
    std::lock_guard<std::mutex> lock(mutex_);
    cancelled_ = true;
}

void Report::finish(bool ok) {
    std::lock_guard<std::mutex> lock(mutex_);
    done_ = true;
    ok_ = ok;
}

bool Report::cancelled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cancelled_;
}

float Report::fraction() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return fraction_;
}

bool Report::done() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return done_;
}

bool Report::ok() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return ok_;
}

Report::Snapshot Report::snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    Snapshot snapshot;
    snapshot.lines = lines_;
    snapshot.problems = problems_;
    snapshot.fraction = fraction_;
    snapshot.doneBytes = doneBytes_;
    snapshot.totalBytes = totalBytes_;
    snapshot.stage = stage_;
    snapshot.done = done_;
    snapshot.ok = ok_;
    snapshot.cancelled = cancelled_;
    return snapshot;
}

// ---------------------------------------------------------------------------
// The window

Result run(Options& options, const std::vector<std::string>& actions,
           const std::function<bool(Report&)>& job) {
    Result result;
    Report report;

    linearwinSetErrorCallback([](int, const char* description) {
        std::fprintf(stderr, "Linear window: %s\n", description == nullptr ? "unknown" : description);
    });
    if (linearwinInit() == 0) return result;

    linearwinWindowHint(LINEARWIN_CONTEXT_VERSION_MAJOR, 3);
    linearwinWindowHint(LINEARWIN_CONTEXT_VERSION_MINOR, 3);
    linearwinWindowHint(LINEARWIN_OPENGL_PROFILE, LINEARWIN_OPENGL_CORE_PROFILE);

    // A tool that asks for more inputs - the server setup asks for six - gets a
    // taller window so they are visible without scrolling, up to what fits a
    // 1080p screen beside its own title bar.
    const int height =
        std::min(kWindowHeight + static_cast<int>(options.fields.size()) * 40, 900);
    LINEARWINwindow* window =
        linearwinCreateWindow(kWindowWidth, height, options.title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        linearwinTerminate();
        return result;
    }
    linearwinSetWindowSizeLimits(window, kWindowWidth, height, LINEARWIN_DONT_CARE,
                                 LINEARWIN_DONT_CARE);
    linearwinMakeContextCurrent(window);
    linearwinSwapInterval(1);

    if (linearglLoadGLLoader((LINEARGLloadproc)linearwinGetProcAddress) == 0) {
        linearwinDestroyWindow(window);
        linearwinTerminate();
        return result;
    }

    LINEARUI_CHECKVERSION();
    LinearUI::CreateContext();
    LinearUIIO& io = LinearUI::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= LinearUIConfigFlags_NavEnableKeyboard;
    applyTheme();
    Fonts fonts;
    loadFonts(&fonts);

    LinearUI_ImplLinearWin_InitForOpenGL(window, true);
    LinearUI_ImplOpenGL3_Init("#version 330");

    // The window owns the output from here, so the console it was started from
    // has nothing left to say. A shell that launched this keeps its own window.
    if (::GetConsoleWindow() != nullptr) ::FreeConsole();

    char folderBuffer[1024] = {};
    std::snprintf(folderBuffer, sizeof(folderBuffer), "%s", options.folder.c_str());
    bool shortcuts = options.shortcuts;

    // One buffer per field, because the widget edits a fixed-size char array. The
    // value goes back into the options when the job is started, so the tool reads
    // exactly what was typed.
    std::vector<std::vector<char>> fieldBuffers;
    fieldBuffers.reserve(options.fields.size());
    for (const Options::Field& field : options.fields) {
        const std::size_t capacity = field.capacity < 16 ? 16 : field.capacity;
        std::vector<char> buffer(capacity + 1, '\0');
        std::snprintf(buffer.data(), buffer.size(), "%s", field.value.c_str());
        fieldBuffers.push_back(std::move(buffer));
    }

    bool started = false;
    bool closeRequested = false;
    std::thread worker;
    // A transfer rate is a difference between samples, so keep the last one.
    double lastBytes = 0.0;
    double lastSampleTime = 0.0;
    double speed = 0.0;

    auto startJob = [&]() {
        if (started) return;
        // What the user typed is what the tool must use.
        if (options.showFolder) options.folder = folderBuffer;
        options.shortcuts = shortcuts;
        for (std::size_t index = 0; index < options.fields.size(); ++index) {
            options.fields[index].value = fieldBuffers[index].data();
        }
        started = true;
        worker = std::thread([&report, &job]() { report.finish(job(report)); });
    };

    if (options.autoStart) startJob();

    while (!closeRequested) {
        linearwinPollEvents();

        const Report::Snapshot state = report.snapshot();
        if (linearwinWindowShouldClose(window) != 0) {
            // A job in flight must finish: a half-written install with no window
            // left to explain it is the worst outcome here.
            if (state.done || !started) {
                closeRequested = true;
            } else {
                linearwinSetWindowShouldClose(window, LINEARWIN_FALSE);
            }
        }
        if (closeRequested) break;

        LinearUI_ImplOpenGL3_NewFrame();
        LinearUI_ImplLinearWin_NewFrame();
        LinearUI::NewFrame();

        LinearUI::SetNextWindowPos(LinearVec2(0.0f, 0.0f));
        LinearUI::SetNextWindowSize(io.DisplaySize);
        LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0.0f, 0.0f));
        LinearUI::Begin("##linear-install", nullptr,
                        LinearUIWindowFlags_NoTitleBar | LinearUIWindowFlags_NoResize |
                            LinearUIWindowFlags_NoMove | LinearUIWindowFlags_NoCollapse |
                            LinearUIWindowFlags_NoSavedSettings |
                            LinearUIWindowFlags_NoBringToFrontOnFocus);
        LinearUI::PopStyleVar();

        LinearDrawList* draw = LinearUI::GetWindowDrawList();
        const LinearVec2 origin = LinearUI::GetWindowPos();
        const LinearVec2 display = io.DisplaySize;
        const float sidebarWidth =
            std::min(std::max(display.x * kSidebarShare, 380.0f), 520.0f);
        const float contentWidth = sidebarWidth - kPadding * 2.0f;
        const float bottomTop = display.y - kBottomHeight;
        const float left = origin.x + kPadding;

        // The art panel fills everything beside the column, above the bar.
        ArtRect art;
        art.min = LinearVec2(origin.x + sidebarWidth, origin.y);
        art.max = LinearVec2(origin.x + display.x, origin.y + bottomTop);
        drawHero(draw, art, static_cast<float>(LinearUI::GetTime()));

        // The brand block, top left.
        const float stateTop = drawBrand(draw, fonts, options, left, origin, contentWidth);

        if (!started) {
            // What the job will do, then the few things it asks about. It is a
            // child so a server setup with six fields scrolls instead of spilling
            // out of the column.
            LinearUI::SetCursorScreenPos(LinearVec2(left, stateTop));
            LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0.0f, 0.0f));
            LinearUI::BeginChild("##form",
                                 LinearVec2(contentWidth, origin.y + bottomTop - stateTop - 20.0f),
                                 0, LinearUIWindowFlags_NoBackground);
            float y = stateTop;
            if (!options.detail.empty()) {
                writeWrapped(fonts.regular, 14.0f, kMuted, LinearVec2(left, y), options.detail);
                y += measureWrapped(fonts.regular, 14.0f, options.detail, contentWidth) + 22.0f;
            }

            const float frameHeight = LinearUI::GetFrameHeight();
            if (options.showFolder) {
                writeText(fonts.regular, 13.0f, kMuted, LinearVec2(left, y), options.folderLabel);
                y += 20.0f;
                LinearUI::SetCursorScreenPos(LinearVec2(left, y));
                LinearUI::SetNextItemWidth(contentWidth);
                LinearUI::InputText("##folder", folderBuffer, sizeof(folderBuffer));
                y += frameHeight + 14.0f;
                if (options.showShortcuts) {
                    LinearUI::SetCursorScreenPos(LinearVec2(left, y));
                    LinearUI::Checkbox("Add Play to the Start menu and the desktop", &shortcuts);
                    y += frameHeight + 16.0f;
                }
            }

            for (std::size_t index = 0; index < options.fields.size(); ++index) {
                const Options::Field& field = options.fields[index];
                writeText(fonts.regular, 13.0f, kMuted, LinearVec2(left, y), field.label);
                y += 20.0f;
                LinearUI::SetCursorScreenPos(LinearVec2(left, y));
                LinearUI::SetNextItemWidth(contentWidth);
                char id[32] = {};
                std::snprintf(id, sizeof(id), "##field%zu", index);
                LinearUI::InputText(id, fieldBuffers[index].data(), fieldBuffers[index].size(),
                                    field.secret ? LinearUIInputTextFlags_Password : 0);
                y += frameHeight + 14.0f;
            }
            LinearUI::EndChild();
            LinearUI::PopStyleVar();
        } else {
            // The steps tick off left to right, then the log fills what is left
            // of the column: the lines are the detail behind the bar.
            float y = stateTop;
            const float circleX = left + 15.0f;
            for (std::size_t index = 0; index < options.steps.size(); ++index) {
                const float rowTop = y + static_cast<float>(index) * kStepRow;
                const int position = static_cast<int>(index);
                const bool done =
                    state.done ? (state.ok || position < state.stage) : position < state.stage;
                const bool active = !state.done && position == state.stage;
                const bool failed = state.done && !state.ok && position == state.stage;

                if (index + 1 < options.steps.size()) {
                    draw->AddLine(LinearVec2(circleX, rowTop + 34.0f),
                                  LinearVec2(circleX, rowTop + kStepRow - 8.0f),
                                  colorOf(done ? kAccent : kBorder), 2.0f);
                }
                drawStepMarker(draw, LinearVec2(circleX, rowTop + 15.0f), done, active, failed);
                writeText(fonts.bold, 18.0f, (done || active || failed) ? kText : kMuted,
                          LinearVec2(left + 50.0f, rowTop + 1.0f), options.steps[index].label);
                writeText(fonts.regular, 14.0f, kMuted, LinearVec2(left + 50.0f, rowTop + 27.0f),
                          options.steps[index].detail);
            }
            y += static_cast<float>(options.steps.size()) * kStepRow + 4.0f;

            const float logHeight = origin.y + bottomTop - y - 18.0f;
            if (logHeight > 28.0f) {
                LinearUI::SetCursorScreenPos(LinearVec2(left, y));
                LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0.0f, 0.0f));
                LinearUI::BeginChild("##log", LinearVec2(contentWidth, logHeight), 0,
                                     LinearUIWindowFlags_NoBackground);
                LinearUI::PushFont(fonts.regular, 13.0f);
                LinearUI::PushTextWrapPos(0.0f);
                if (state.lines.empty()) {
                    LinearUI::TextColored(kMuted, "%s", "Nothing yet.");
                }
                for (std::size_t index = 0; index < state.lines.size(); ++index) {
                    const bool problem = index < state.problems.size() && state.problems[index];
                    LinearUI::TextColored(problem ? kError : kMuted, "%s",
                                          state.lines[index].c_str());
                }
                LinearUI::PopTextWrapPos();
                LinearUI::PopFont();
                // The newest line has to be in sight while the work is running;
                // once it is done the reader is left where they scrolled to.
                if (!state.done) LinearUI::SetScrollHereY(1.0f);
                LinearUI::EndChild();
                LinearUI::PopStyleVar();
            }
        }

        // The bottom bar: what is happening on the left, the percentage on the
        // right, and the bar itself across the window.
        const double now = LinearUI::GetTime();
        if (now - lastSampleTime > 0.3) {
            if (lastSampleTime > 0.0 && state.doneBytes >= lastBytes) {
                const double rate =
                    (static_cast<double>(state.doneBytes) - lastBytes) / (now - lastSampleTime);
                speed = speed > 0.0 ? speed * 0.6 + rate * 0.4 : rate;
            }
            lastBytes = static_cast<double>(state.doneBytes);
            lastSampleTime = now;
        }

        const float titleTop = origin.y + bottomTop + 28.0f;
        const float statsTop = origin.y + bottomTop + 64.0f;
        const float buttonTop = origin.y + bottomTop + 56.0f;
        const float barTop = origin.y + bottomTop + 110.0f;
        const float barLeft = origin.x + kPadding;
        const float barRight = origin.x + display.x - kPadding;

        draw->AddLine(LinearVec2(origin.x, origin.y + bottomTop),
                      LinearVec2(origin.x + display.x, origin.y + bottomTop), colorOf(kBorder),
                      1.0f);

        // While the job runs the bar says what it is doing; before and after it
        // names the job itself.
        const std::string title = started && !options.busyLabel.empty() ? options.busyLabel
                                                                        : options.heading;
        if (!title.empty()) writeText(fonts.bold, 18.0f, kText, LinearVec2(left, titleTop), title);

        std::string stats;
        LinearVec4 statsColor = kMuted;
        if (!started) {
            stats = "Waiting to start.";
        } else if (!state.done) {
            // The click is answered at once even when the file in flight has to
            // finish, so the button never looks dead.
            if (state.cancelled) {
                stats = "Stopping at the next file boundary...";
            } else if (state.doneBytes > 0) {
                stats = formatBytes(state.doneBytes);
                if (state.totalBytes > 0) stats += " of " + formatBytes(state.totalBytes);
                if (speed > 1.0) {
                    stats += "   -   " + formatBytes(static_cast<long long>(speed)) + "/s";
                }
            } else {
                stats = "Working" + waitingDots();
            }
        } else if (state.ok) {
            stats = "Finished.";
            statsColor = kOk;
        } else if (state.cancelled) {
            stats = "Stopped at the last file boundary.";
        } else {
            stats = "That did not finish. The reason is in the list beside the bar.";
            statsColor = kError;
        }
        writeText(fonts.regular, 14.0f, statsColor, LinearVec2(left, statsTop), stats);

        // A finished job that never reported bytes still reads as complete.
        const float shownFraction =
            state.done && state.ok ? 1.0f : (state.fraction < 0.0f ? -1.0f : state.fraction);
        if (shownFraction >= 0.0f) {
            char percentText[16] = {};
            std::snprintf(percentText, sizeof(percentText), "%.0f%%",
                          static_cast<double>(shownFraction) * 100.0);
            const float percentWidth =
                [&]() {
                    LinearUI::PushFont(fonts.bold, 32.0f);
                    const float width = LinearUI::CalcTextSize(percentText).x;
                    LinearUI::PopFont();
                    return width;
                }();
            writeText(fonts.bold, 32.0f, kText,
                      LinearVec2(origin.x + display.x - kPadding - percentWidth,
                                 origin.y + bottomTop + 16.0f),
                      percentText);
        }

        draw->AddRectFilled(LinearVec2(barLeft, barTop), LinearVec2(barRight, barTop + 14.0f),
                            colorOf(theme::colors().progressTrack), 7.0f);
        if (started && !state.done && shownFraction < 0.0f) {
            // No size to divide by: a segment sweeps the trough instead of a bar
            // that would sit at nothing while the work is clearly happening.
            const float sweepWidth = (barRight - barLeft) * 0.26f;
            const float phase = static_cast<float>(std::fmod(now * 0.55, 1.0));
            const float sweepLeft = barLeft + ((barRight - barLeft) - sweepWidth) * phase;
            draw->AddRectFilled(LinearVec2(sweepLeft, barTop),
                                LinearVec2(sweepLeft + sweepWidth, barTop + 14.0f),
                                colorOf(kAccent), 7.0f);
        } else if (shownFraction > 0.0f) {
            const float filled = barLeft + (barRight - barLeft) * clamp01(shownFraction);
            if (filled > barLeft + 4.0f) {
                draw->AddRectFilled(LinearVec2(barLeft, barTop), LinearVec2(filled, barTop + 14.0f),
                                    colorOf(kAccent), 7.0f);
            }
        }

        const float rightEdge = origin.x + display.x - kPadding;
        if (!started) {
            const float startWidth = 148.0f;
            const float cancelWidth = 108.0f;
            float buttonX = rightEdge - startWidth - cancelWidth - 10.0f;
            LinearUI::SetCursorScreenPos(LinearVec2(buttonX, buttonTop));
            if (drawButton(options.startLabel, theme::primaryButton(),
                           LinearVec2(startWidth, 34.0f))) {
                startJob();
            }
            buttonX += startWidth + 10.0f;
            LinearUI::SetCursorScreenPos(LinearVec2(buttonX, buttonTop));
            if (drawButton(options.cancelLabel, theme::secondaryButton(),
                           LinearVec2(cancelWidth, 34.0f))) {
                result.cancelled = true;
                closeRequested = true;
            }
        } else if (!state.done) {
            LinearUI::SetCursorScreenPos(LinearVec2(rightEdge - 92.0f, buttonTop + 5.0f));
            if (drawButton("Cancel", theme::secondaryButton(), LinearVec2(92.0f, 28.0f))) {
                report.cancel();
            }
        } else if (!actions.empty()) {
            float total = 10.0f * static_cast<float>(actions.size() - 1);
            for (std::size_t index = 0; index < actions.size(); ++index) {
                total += index == 0 ? 148.0f : 108.0f;
            }
            float buttonX = rightEdge - total;
            for (std::size_t index = 0; index < actions.size(); ++index) {
                const float width = index == 0 ? 148.0f : 108.0f;
                LinearUI::SetCursorScreenPos(LinearVec2(buttonX, buttonTop));
                const bool pressed =
                    index == 0
                        ? drawButton(actions[index], theme::primaryButton(),
                                     LinearVec2(width, 34.0f))
                        : drawButton(actions[index], theme::secondaryButton(),
                                     LinearVec2(width, 34.0f));
                if (pressed) {
                    result.action = actions[index];
                    closeRequested = true;
                }
                buttonX += width + 10.0f;
            }
        }

        LinearUI::End();

        LinearUI::Render();
        int displayWidth = 0;
        int displayHeight = 0;
        linearwinGetFramebufferSize(window, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        const LinearVec4 background = theme::background();
        glClearColor(background.x, background.y, background.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        LinearUI_ImplOpenGL3_RenderDrawData(LinearUI::GetDrawData());
        linearwinSwapBuffers(window);
    }

    if (worker.joinable()) worker.join();

    LinearUI_ImplOpenGL3_Shutdown();
    LinearUI_ImplLinearWin_Shutdown();
    LinearUI::DestroyContext();
    linearwinDestroyWindow(window);
    linearwinTerminate();

    result.shown = true;
    result.ok = report.ok();
    result.shortcuts = shortcuts;
    if (options.showFolder) result.folder = folderBuffer;
    result.fields.reserve(options.fields.size());
    for (std::size_t index = 0; index < options.fields.size(); ++index) {
        result.fields.push_back(fieldBuffers[index].data());
    }
    // Closing the window before starting is the same answer as pressing Cancel.
    if (!started) result.cancelled = true;
    return result;
}

}  // namespace ui
}  // namespace linear
