// LinearBrowser: the project's own browser.
//
// It is a browser in the sense this project can honestly build one. A page is
// fetched over plain HTTP - there is no TLS stack in the tree, and HttpClient
// says so instead of pretending - read by BrowserPage.cpp into the same blocks
// LinearDocs lays out, and drawn as a document with an address bar, Back,
// Forward, Reload, Home and links you can click. It runs no script, applies no
// style sheet, draws no image and submits no form: the page is text, which is
// what makes it a reader beside the documentation window instead of a second
// rendering engine.
//
// The window is resizable; the page is laid out again when its width changes and
// only then, so a long page is not re-wrapped every frame.

#include "BrowserPage.h"
#include "DocsLayout.h"
#include "HttpClient.h"
#include "UiTheme.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <string>
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

namespace {

using linear::docs::Box;
using linear::docs::Line;
using linear::docs::LineRun;
using linear::docs::LineStyle;
using linear::docs::RunStyle;
using linear::docs::TextStyle;

constexpr int kWindowWidth = 1180;
constexpr int kWindowHeight = 820;
constexpr float kToolbarHeight = 58.0f;
constexpr float kFooterHeight = 30.0f;
constexpr float kPadding = 20.0f;
constexpr float kPageSide = 30.0f;
// Kept clear on the right of the page so the last word of a line does not run
// under the scrollbar.
constexpr float kScrollbarRoom = 22.0f;
constexpr float kGoWidth = 54.0f;

// The palette lives in UiTheme.h; a colour written here would be a colour the
// other windows of this project do not have.
const LinearVec4 kAccent = linear::ui::theme::accentStrong();
const LinearVec4 kText = linear::ui::theme::text();
const LinearVec4 kMuted = linear::ui::theme::muted();
const LinearVec4 kBorder = linear::ui::theme::border();
const LinearVec4 kError = linear::ui::theme::error();

LinearU32 colorOf(const LinearVec4& color) { return LinearUI::GetColorU32(color); }

struct Fonts {
    LinearFont* regular = nullptr;
    LinearFont* bold = nullptr;
    LinearFont* mono = nullptr;
};

bool fontExists(const char* path) {
    std::ifstream probe(path, std::ios::binary);
    return probe.good();
}

LinearFont* firstOf(const char* const* paths, std::size_t count) {
    LinearUIIO& io = LinearUI::GetIO();
    LinearFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 2;
    for (std::size_t index = 0; index < count; ++index) {
        if (!fontExists(paths[index])) continue;
        LinearFont* font = io.Fonts->AddFontFromFileTTF(paths[index], 17.0f, &config);
        if (font != nullptr) return font;
    }
    return nullptr;
}

void loadFonts(Fonts* fonts) {
    const char* const regularFiles[] = {"C:\\Windows\\Fonts\\segoeui.ttf",
                                        "C:\\Windows\\Fonts\\arial.ttf"};
    const char* const boldFiles[] = {"C:\\Windows\\Fonts\\segoeuib.ttf",
                                     "C:\\Windows\\Fonts\\arialbd.ttf",
                                     "C:\\Windows\\Fonts\\segoeui.ttf"};
    const char* const monoFiles[] = {"C:\\Windows\\Fonts\\consola.ttf",
                                     "C:\\Windows\\Fonts\\cour.ttf"};

    fonts->regular = firstOf(regularFiles, sizeof(regularFiles) / sizeof(regularFiles[0]));
    if (fonts->regular == nullptr) fonts->regular = LinearUI::GetIO().Fonts->AddFontDefault();
    fonts->bold = firstOf(boldFiles, sizeof(boldFiles) / sizeof(boldFiles[0]));
    if (fonts->bold == nullptr) fonts->bold = fonts->regular;
    fonts->mono = firstOf(monoFiles, sizeof(monoFiles) / sizeof(monoFiles[0]));
    if (fonts->mono == nullptr) fonts->mono = fonts->regular;
}

LinearFont* fontFor(const Fonts& fonts, bool bold, bool mono) {
    if (mono) return fonts.mono;
    return bold ? fonts.bold : fonts.regular;
}

linear::docs::Metrics makeMetrics(const Fonts& fonts) {
    linear::docs::Metrics metrics;
    metrics.width = [&fonts](const std::string& text, float size, bool bold, bool mono) {
        LinearUI::PushFont(fontFor(fonts, bold, mono), size);
        const float width = LinearUI::CalcTextSize(text.c_str()).x;
        LinearUI::PopFont();
        return width;
    };
    metrics.step = [](float size) { return std::floor(size * 1.65f) + 1.0f; };
    return metrics;
}

float writeText(LinearFont* font, float size, const LinearVec4& color, const LinearVec2& pos,
                const std::string& value) {
    LinearUI::PushFont(font, size);
    LinearUI::SetCursorScreenPos(pos);
    LinearUI::TextColored(color, "%s", value.c_str());
    const float width = LinearUI::CalcTextSize(value.c_str()).x;
    LinearUI::PopFont();
    return width;
}

// Cuts a label that would run past the space it has, so a long title cannot
// paint over the hint at the other end of the footer.
std::string fit(LinearFont* font, float size, const std::string& text, float width) {
    LinearUI::PushFont(font, size);
    if (LinearUI::CalcTextSize(text.c_str()).x <= width) {
        LinearUI::PopFont();
        return text;
    }
    std::string cut = text;
    while (!cut.empty() && LinearUI::CalcTextSize((cut + "...").c_str()).x > width) {
        cut.pop_back();
    }
    LinearUI::PopFont();
    return cut + "...";
}

struct State {
    linear::browser::Page document;
    linear::browser::History history;
    char address[4096] = {};
    std::string status = "Type an address, or pick one on the start page.";
    bool statusError = false;
    float scroll = 0.0f;
    bool jumpToTop = true;
    // The laid-out page, and the width it was laid out for: wrapping is the
    // expensive part, so it happens when the page or the column changes.
    linear::docs::Page laidOut;
    float pageWidth = -1.0f;
    bool pageDirty = true;
};

struct Frame {
    State& state;
    const Fonts& fonts;
    const linear::docs::Metrics& metrics;
    LinearDrawList* draw = nullptr;
    LinearVec2 origin;
    float displayWidth = 0.0f;
    float displayHeight = 0.0f;
    // A link the reader pressed this frame. It is opened after the page has
    // finished drawing, because opening a page replaces the runs being walked.
    std::string clicked;
};

// Reads `typed` into the window. `remember` is false for Back and Forward, which
// have already moved the history; every other navigation records where it went.
bool openPage(State& state, const std::string& typed, bool remember) {
    const std::string url = linear::browser::normaliseAddress(typed);
    if (remember) state.history.push(url);
    std::snprintf(state.address, sizeof(state.address), "%s", url.c_str());
    state.jumpToTop = true;
    state.pageDirty = true;

    if (url == "about:start") {
        state.document = linear::browser::parse(linear::browser::startPage(), url);
        state.status = "Start page";
        state.statusError = false;
        return true;
    }

    if (url.compare(0, 7, "http://") != 0) {
        const std::string detail =
            url.compare(0, 8, "https://") == 0
                ? "This project has no TLS stack, so https:// cannot be opened. "
                  "Type an http:// address instead."
                : "LinearBrowser opens http:// addresses and about:start. "
                  "A mailto: or javascript: link is not a page.";
        state.document = linear::browser::parse(linear::browser::errorPage(url, detail), url);
        state.status = detail;
        state.statusError = true;
        return false;
    }

    // Blocking on purpose: the window is a reader, and the only other work in
    // this process is drawing the page that is already on screen. The timeout is
    // what keeps a hung server from freezing it forever.
    const linear::http::Response response =
        linear::http::request("GET", url, std::string(), std::string(), 12000);

    if (response.networkError()) {
        const std::string detail =
            response.error.empty() ? "The request did not complete." : response.error;
        state.document = linear::browser::parse(linear::browser::errorPage(url, detail), url);
        state.status = detail;
        state.statusError = true;
        return false;
    }

    if (!response.ok()) {
        char detail[160] = {};
        std::snprintf(detail, sizeof(detail), "The server answered HTTP %d.", response.status);
        state.document = linear::browser::parse(linear::browser::errorPage(url, detail), url);
        state.status = detail;
        state.statusError = true;
        return false;
    }

    state.document = linear::browser::parse(response.body, url);
    char summary[160] = {};
    std::snprintf(summary, sizeof(summary), "HTTP %d  -  %zu bytes", response.status,
                  response.body.size());
    state.status = summary;
    state.statusError = false;
    return true;
}

void ensurePage(Frame& frame) {
    const float width = std::max(200.0f, frame.displayWidth - kPageSide * 2.0f - kScrollbarRoom);
    if (!frame.state.pageDirty && std::fabs(frame.state.pageWidth - width) <= 0.5f) return;
    frame.state.laidOut = linear::docs::layOut(frame.state.document.blocks, width, frame.metrics);
    frame.state.pageWidth = width;
    frame.state.pageDirty = false;
}

void drawToolbar(Frame& frame) {
    State& state = frame.state;
    const linear::ui::theme::Colors& palette = linear::ui::theme::colors();

    frame.draw->AddRectFilled(frame.origin,
                              LinearVec2(frame.origin.x + frame.displayWidth,
                                         frame.origin.y + kToolbarHeight),
                              colorOf(palette.panel));
    frame.draw->AddLine(LinearVec2(frame.origin.x, frame.origin.y + kToolbarHeight),
                        LinearVec2(frame.origin.x + frame.displayWidth,
                                   frame.origin.y + kToolbarHeight),
                        colorOf(kBorder), 1.0f);

    LinearUI::SetCursorScreenPos(LinearVec2(frame.origin.x + kPadding, frame.origin.y + 13.0f));
    if (LinearUI::Button("Back")) {
        std::string url;
        if (state.history.back(&url)) openPage(state, url, false);
    }
    LinearUI::SameLine();
    if (LinearUI::Button("Forward")) {
        std::string url;
        if (state.history.forward(&url)) openPage(state, url, false);
    }
    LinearUI::SameLine();
    if (LinearUI::Button("Reload")) {
        openPage(state, state.document.url.empty() ? state.address : state.document.url, false);
    }
    LinearUI::SameLine();
    if (LinearUI::Button("Home")) openPage(state, "about:start", true);

    // The address field takes whatever the buttons left, less the Go button and
    // the gap before it.
    LinearUI::SameLine();
    const float fieldLeft = LinearUI::GetCursorScreenPos().x;
    const float fieldWidth = std::max(
        160.0f, frame.origin.x + frame.displayWidth - kPadding - fieldLeft - kGoWidth - 10.0f);
    LinearUI::SetNextItemWidth(fieldWidth);
    const bool submitted = LinearUI::InputTextWithHint(
        "##address", "http://localhost:3000", state.address, sizeof(state.address),
        LinearUIInputTextFlags_EnterReturnsTrue);
    LinearUI::SameLine();
    if (LinearUI::Button("Go") || submitted) openPage(state, state.address, true);
}

void drawBoxes(Frame& frame, float contentLeft, float pageTop, float scroll, float visibleTop,
               float visibleBottom) {
    const linear::ui::theme::Colors& palette = linear::ui::theme::colors();
    for (const Box& box : frame.state.laidOut.boxes) {
        if (box.y1 < visibleTop || box.y0 > visibleBottom) continue;
        const LinearVec2 min(contentLeft + box.x0, pageTop + box.y0 - scroll);
        const LinearVec2 max(contentLeft + box.x1, pageTop + box.y1 - scroll);
        switch (box.kind) {
            case Box::Kind::Code:
                frame.draw->AddRectFilled(min, max, colorOf(palette.panel), 6.0f);
                frame.draw->AddRect(min, max, colorOf(palette.border), 6.0f);
                break;
            case Box::Kind::TableHeader:
                frame.draw->AddRectFilled(min, max, colorOf(palette.tableHeader));
                frame.draw->AddLine(LinearVec2(min.x, max.y), max,
                                    colorOf(palette.tableBorderLight), 1.0f);
                break;
            case Box::Kind::TableRow:
                frame.draw->AddLine(LinearVec2(min.x, max.y), max,
                                    colorOf(palette.tableBorderLight), 1.0f);
                break;
            case Box::Kind::Quote:
                frame.draw->AddRectFilled(min, max, colorOf(kAccent), 1.5f);
                break;
            case Box::Kind::Rule:
                frame.draw->AddLine(min, LinearVec2(max.x, min.y), colorOf(kBorder), 1.0f);
                break;
        }
    }
}

void drawLines(Frame& frame, float contentLeft, float pageTop, float scroll, float visibleTop,
               float visibleBottom) {
    const linear::docs::Page& page = frame.state.laidOut;

    for (std::size_t lineIndex = 0; lineIndex < page.lines.size(); ++lineIndex) {
        const Line& line = page.lines[lineIndex];
        if (line.y + line.height < visibleTop || line.y > visibleBottom) continue;

        const TextStyle set = linear::docs::textStyle(line.style);
        const float y = pageTop + line.y - scroll;

        if (!line.marker.empty()) {
            writeText(frame.fonts.bold, set.size, kAccent,
                      LinearVec2(contentLeft + line.x - 24.0f, y), line.marker);
        }

        // Runs are placed by the widths the layout measured, not by letting the
        // text widget advance the cursor: that keeps a bold phrase from shifting
        // the rest of its line, and gives each link a rectangle to hit.
        float x = line.x;
        for (std::size_t runIndex = 0; runIndex < line.runs.size(); ++runIndex) {
            const LineRun& run = line.runs[runIndex];
            x += run.spaceBefore;
            const bool bold = set.bold || run.style == RunStyle::Strong;
            const float runWidth = frame.metrics.width(run.text, set.size, bold, run.mono);

            if (!run.target.empty()) {
                LinearUI::PushID(static_cast<int>(lineIndex * 128 + runIndex));
                LinearUI::SetCursorScreenPos(LinearVec2(contentLeft + x, y));
                const bool pressed =
                    LinearUI::InvisibleButton("##link", LinearVec2(std::max(runWidth, 6.0f),
                                                                   line.height));
                if (pressed) frame.clicked = run.target;
                if (LinearUI::IsItemHovered()) LinearUI::SetMouseCursor(LinearUIMouseCursor_Hand);
                LinearUI::PopID();

                // An underline says the words are a link before the pointer
                // changes shape over them.
                frame.draw->AddLine(LinearVec2(contentLeft + x, y + line.height - 3.0f),
                                    LinearVec2(contentLeft + x + runWidth,
                                               y + line.height - 3.0f),
                                    colorOf(kAccent), 1.0f);
            }

            const bool marked = run.style == RunStyle::Code || run.style == RunStyle::Link;
            writeText(fontFor(frame.fonts, bold, run.mono), set.size, marked ? kAccent : kText,
                      LinearVec2(contentLeft + x, y), run.text);
            x += runWidth;
        }
    }
}

void drawPage(Frame& frame) {
    State& state = frame.state;
    const float left = frame.origin.x + kPageSide;
    const float top = frame.origin.y + kToolbarHeight + 10.0f;
    const float height =
        frame.origin.y + frame.displayHeight - kFooterHeight - top - 8.0f;
    const float width = frame.displayWidth - kPageSide * 2.0f;

    ensurePage(frame);

    LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0.0f, 0.0f));
    LinearUI::SetCursorScreenPos(LinearVec2(left, top));
    if (state.jumpToTop) {
        LinearUI::SetNextWindowScroll(LinearVec2(0.0f, 0.0f));
        state.jumpToTop = false;
    }
    LinearUI::BeginChild("##page", LinearVec2(width + 8.0f, height), 0,
                         LinearUIWindowFlags_NoBackground);
    // The reader can drag the scrollbar, so where it is has to be read back out
    // of the child rather than remembered from the wheel alone.
    state.scroll = LinearUI::GetScrollY();

    // One item as tall as the page gives the child something to scroll. It sits
    // at the content origin, not at the page's screen top, because the height
    // the child measures is taken from that origin.
    LinearUI::SetCursorPos(LinearVec2(0.0f, 0.0f));
    LinearUI::Dummy(LinearVec2(1.0f, state.laidOut.height + 16.0f));

    const float visibleTop = state.scroll - 4.0f;
    const float visibleBottom = state.scroll + height + 4.0f;
    frame.draw->PushClipRect(LinearVec2(left, top), LinearVec2(left + width, top + height), true);
    drawBoxes(frame, left, top, state.scroll, visibleTop, visibleBottom);
    drawLines(frame, left, top, state.scroll, visibleTop, visibleBottom);
    frame.draw->PopClipRect();

    LinearUI::EndChild();
    LinearUI::PopStyleVar();

    // Only now, with the page's runs no longer being walked, is it safe to
    // replace them with another page's.
    if (!frame.clicked.empty()) openPage(state, frame.clicked, true);
}

void drawFooter(Frame& frame) {
    State& state = frame.state;
    const float top = frame.origin.y + frame.displayHeight - kFooterHeight;
    frame.draw->AddLine(LinearVec2(frame.origin.x, top),
                        LinearVec2(frame.origin.x + frame.displayWidth, top), colorOf(kBorder),
                        1.0f);

    const std::string hint = "Enter opens  -  click a link  -  Esc closes";
    LinearUI::PushFont(frame.fonts.regular, 11.5f);
    const float hintWidth = LinearUI::CalcTextSize(hint.c_str()).x;
    LinearUI::PopFont();

    char summary[1600] = {};
    std::snprintf(summary, sizeof(summary), "%s  -  %s", state.document.title.c_str(),
                  state.status.c_str());
    const float summaryWidth =
        frame.displayWidth - kPadding * 2.0f - hintWidth - 24.0f;
    writeText(frame.fonts.regular, 11.5f, state.statusError ? kError : kMuted,
              LinearVec2(frame.origin.x + kPadding, top + 8.0f),
              fit(frame.fonts.regular, 11.5f, summary, std::max(80.0f, summaryWidth)));

    writeText(frame.fonts.regular, 11.5f, kMuted,
              LinearVec2(frame.origin.x + frame.displayWidth - kPadding - hintWidth, top + 8.0f),
              hint);
}

}  // namespace

int main(int argc, char** argv) {
    State state;
    const std::string initial = argc > 1 && argv[1] != nullptr ? argv[1] : "about:start";
    openPage(state, initial, true);

    linearwinSetErrorCallback([](int, const char* description) {
        std::fprintf(stderr, "Linear window: %s\n",
                     description == nullptr ? "unknown" : description);
    });
    if (linearwinInit() == 0) {
        std::fprintf(stderr, "LinearBrowser needs a window, and none could be created.\n");
        return 1;
    }

    linearwinWindowHint(LINEARWIN_CONTEXT_VERSION_MAJOR, 3);
    linearwinWindowHint(LINEARWIN_CONTEXT_VERSION_MINOR, 3);
    linearwinWindowHint(LINEARWIN_OPENGL_PROFILE, LINEARWIN_OPENGL_CORE_PROFILE);
    LINEARWINwindow* window =
        linearwinCreateWindow(kWindowWidth, kWindowHeight, "Linear browser", nullptr, nullptr);
    if (window == nullptr) {
        linearwinTerminate();
        std::fprintf(stderr, "LinearBrowser needs a window, and none could be created.\n");
        return 1;
    }
    // The page re-wraps at whatever width the window is given, so it can be
    // resized like a browser; only a size too small to hold the toolbar is
    // refused.
    linearwinSetWindowSizeLimits(window, 720, 520, LINEARWIN_DONT_CARE, LINEARWIN_DONT_CARE);
    linearwinMakeContextCurrent(window);
    linearwinSwapInterval(1);

    if (linearglLoadGLLoader((LINEARGLloadproc)linearwinGetProcAddress) == 0) {
        linearwinDestroyWindow(window);
        linearwinTerminate();
        std::fprintf(stderr, "LinearBrowser could not load OpenGL.\n");
        return 1;
    }

    LINEARUI_CHECKVERSION();
    LinearUI::CreateContext();
    LinearUIIO& io = LinearUI::GetIO();
    io.IniFilename = nullptr;
    linear::ui::theme::apply();
    Fonts fonts;
    loadFonts(&fonts);

    LinearUI_ImplLinearWin_InitForOpenGL(window, true);
    LinearUI_ImplOpenGL3_Init("#version 330");

    // The window owns the output from here, so the console it was started from
    // has nothing left to say.
    if (::GetConsoleWindow() != nullptr) ::FreeConsole();

    const linear::docs::Metrics metrics = makeMetrics(fonts);

    while (linearwinWindowShouldClose(window) == 0) {
        linearwinPollEvents();

        LinearUI_ImplOpenGL3_NewFrame();
        LinearUI_ImplLinearWin_NewFrame();
        LinearUI::NewFrame();

        if (LinearUI::IsKeyPressed(LinearUIKey_Escape)) break;

        LinearUI::SetNextWindowPos(LinearVec2(0.0f, 0.0f));
        LinearUI::SetNextWindowSize(io.DisplaySize);
        LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0.0f, 0.0f));
        LinearUI::Begin("##linear-browser", nullptr,
                        LinearUIWindowFlags_NoTitleBar | LinearUIWindowFlags_NoResize |
                            LinearUIWindowFlags_NoMove | LinearUIWindowFlags_NoCollapse |
                            LinearUIWindowFlags_NoScrollbar |
                            LinearUIWindowFlags_NoScrollWithMouse |
                            LinearUIWindowFlags_NoSavedSettings |
                            LinearUIWindowFlags_NoBringToFrontOnFocus);
        LinearUI::PopStyleVar();

        Frame frame{state,           fonts,   metrics,
                    LinearUI::GetWindowDrawList(), LinearUI::GetWindowPos(),
                    io.DisplaySize.x, io.DisplaySize.y, std::string()};
        drawToolbar(frame);
        drawPage(frame);
        drawFooter(frame);

        LinearUI::End();

        LinearUI::Render();
        int displayWidth = 0;
        int displayHeight = 0;
        linearwinGetFramebufferSize(window, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        const LinearVec4 background = linear::ui::theme::background();
        glClearColor(background.x, background.y, background.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        LinearUI_ImplOpenGL3_RenderDrawData(LinearUI::GetDrawData());
        linearwinSwapBuffers(window);
    }

    LinearUI_ImplOpenGL3_Shutdown();
    LinearUI_ImplLinearWin_Shutdown();
    LinearUI::DestroyContext();
    linearwinDestroyWindow(window);
    linearwinTerminate();
    return 0;
}
