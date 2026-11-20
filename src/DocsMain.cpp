// LinearDocs: the project's documentation in a window.
//
// The manual for this project is a folder of Markdown: README.md for the whole
// thing and a shorter file for each topic it links to. That is fine for somebody
// reading on GitHub and awkward for somebody who unzipped a release and wants to
// know what a flag does, so this program reads the same files and draws them as a
// document instead - a list of what is in docs/ on the left, the page itself on
// the right, and the headings of the open page under the list, each one a jump.
//
// The files are read from disk rather than compiled in: docs/ ships in the package
// (scripts/package-build.ps1 copies it) and a run from a source checkout finds the
// repository's own files by walking up from the executable. Nothing has to be
// regenerated when a sentence changes, which is the point of documentation.

#include "DocsLayout.h"
#include "DocsLibrary.h"
#include "UiTheme.h"

#include <cmath>
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

using linear::docs::Block;
using linear::docs::Box;
using linear::docs::Line;
using linear::docs::LineRun;
using linear::docs::LineStyle;
using linear::docs::Page;
using linear::docs::RunStyle;
using linear::docs::Section;
using linear::docs::Source;
using linear::docs::TextStyle;

// The window has no title bar of its own and cannot be resized, so these are
// pixels rather than shares: nothing reflows, which is what lets the page be laid
// out once instead of every frame.
constexpr int kWindowWidth = 1180;
constexpr int kWindowHeight = 800;

constexpr float kSidebarWidth = 322.0f;
constexpr float kPadding = 26.0f;
constexpr float kContentLeft = 36.0f;
constexpr float kHeaderHeight = 104.0f;
constexpr float kFooterHeight = 40.0f;
constexpr float kDocRow = 50.0f;
constexpr float kSectionRow = 26.0f;
// How far left of its text a bullet or a number sits, and the width kept clear on
// the right so the last column of a table does not run under the scrollbar.
constexpr float kMarkerOffset = 24.0f;
constexpr float kScrollbarRoom = 20.0f;

// The palette lives in UiTheme.h; a colour written here would be a colour the
// other windows of this project do not have.
const LinearVec4 kAccent = linear::ui::theme::accent();
const LinearVec4 kAccentStrong = linear::ui::theme::accentStrong();
const LinearVec4 kText = linear::ui::theme::text();
const LinearVec4 kMuted = linear::ui::theme::muted();
const LinearVec4 kBorder = linear::ui::theme::border();

LinearU32 colorOf(const LinearVec4& color) { return LinearUI::GetColorU32(color); }

// Three faces: the body, the bold one every heading is set in, and a fixed-width
// one for code, which is much of what these files show. Without a font file the
// embedded bitmap font is used, which still reads.
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

    // The regular face is added first, so it is also the default font.
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

// The layout measures with these and the window draws with them, so a line breaks
// where the pixels actually end.
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

// Draws one string at an absolute position and returns its width, so a caller can
// put the next word beside it.
float writeText(LinearFont* font, float size, const LinearVec4& color, const LinearVec2& pos,
                const std::string& value) {
    LinearUI::PushFont(font, size);
    LinearUI::SetCursorScreenPos(pos);
    LinearUI::TextColored(color, "%s", value.c_str());
    const float width = LinearUI::CalcTextSize(value.c_str()).x;
    LinearUI::PopFont();
    return width;
}

// Cuts a label that would run past its column. A list row is a signpost, not the
// document: the whole title is on the page it opens.
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
    std::vector<Source> sources;
    std::vector<std::vector<Block>> blocks;
    int selected = 0;
    Page page;
    // Which document, at which width, the page was laid out for: the layout is
    // the expensive part, so it happens when one of those changes and not once a
    // frame.
    int pageDocument = -1;
    float pageWidth = 0.0f;
    // A section the reader asked for, applied as the page begins.
    float jump = -1.0f;
    float scroll = 0.0f;
    std::string error;
};

// Everything one frame of this window needs, so the parts below are functions
// instead of one long block.
struct Frame {
    State& state;
    const Fonts& fonts;
    const linear::docs::Metrics& metrics;
    LinearDrawList* draw = nullptr;
    LinearVec2 origin;
    float displayWidth = 0.0f;
    float displayHeight = 0.0f;
};

// The same mark the installer draws: the accent tile with a play glyph cut out of
// it, made of primitives so no artwork has to ship beside the program.
void drawMark(LinearDrawList* draw, const LinearVec2& pos, float size) {
    draw->AddRectFilled(pos, LinearVec2(pos.x + size, pos.y + size), colorOf(kAccentStrong),
                        size * 0.28f);
    const LinearVec2 middle(pos.x + size * 0.545f, pos.y + size * 0.5f);
    const float glyph = size * 0.28f;
    draw->AddTriangleFilled(LinearVec2(middle.x - glyph * 0.55f, middle.y - glyph * 0.80f),
                            LinearVec2(middle.x - glyph * 0.55f, middle.y + glyph * 0.80f),
                            LinearVec2(middle.x + glyph * 0.85f, middle.y),
                            colorOf(linear::ui::theme::background()));
}

void drawBrand(Frame& frame) {
    const float left = frame.origin.x + kPadding;
    const float top = frame.origin.y + 22.0f;
    drawMark(frame.draw, LinearVec2(left, top), 40.0f);

    const float textLeft = left + 54.0f;
    const float nameWidth =
        writeText(frame.fonts.bold, 21.0f, kText, LinearVec2(textLeft, top + 1.0f), "Linear");
    writeText(frame.fonts.bold, 21.0f, kAccent, LinearVec2(textLeft + nameWidth + 6.0f, top + 1.0f),
              "Docs");
    writeText(frame.fonts.regular, 11.5f, kMuted, LinearVec2(textLeft, top + 27.0f),
              "This project's own documentation");
}

void drawDocumentList(Frame& frame, float top, float height) {
    const float left = frame.origin.x + kPadding;
    const float innerWidth = kSidebarWidth - kPadding * 2.0f;
    const linear::ui::theme::Colors& palette = linear::ui::theme::colors();

    LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0.0f, 0.0f));
    LinearUI::SetCursorScreenPos(LinearVec2(left, top));
    LinearUI::BeginChild("##documents", LinearVec2(innerWidth, height), 0,
                         LinearUIWindowFlags_NoBackground);

    float y = top;
    for (std::size_t index = 0; index < frame.state.sources.size(); ++index) {
        const Source& source = frame.state.sources[index];
        const int position = static_cast<int>(index);
        const bool current = position == frame.state.selected;

        LinearUI::PushID(position);
        LinearUI::SetCursorScreenPos(LinearVec2(left, y));
        const bool pressed = LinearUI::InvisibleButton("##row", LinearVec2(innerWidth, kDocRow));
        const bool hovered = LinearUI::IsItemHovered();
        LinearUI::PopID();
        if (pressed && !current) {
            frame.state.selected = position;
            frame.state.jump = 0.0f;
            frame.state.pageDocument = -1;
        }

        if (current || hovered) {
            frame.draw->AddRectFilled(
                LinearVec2(left, y), LinearVec2(left + innerWidth, y + kDocRow - 4.0f),
                colorOf(current ? palette.header : palette.tab), 6.0f);
        }
        if (current) {
            frame.draw->AddRectFilled(LinearVec2(left + 1.0f, y + 8.0f),
                                      LinearVec2(left + 4.0f, y + kDocRow - 12.0f),
                                      colorOf(kAccent), 1.5f);
        }

        writeText(frame.fonts.bold, 14.0f, current ? kText : kMuted,
                  LinearVec2(left + 16.0f, y + 6.0f),
                  fit(frame.fonts.bold, 14.0f, source.title, innerWidth - 32.0f));
        char meta[192] = {};
        std::snprintf(meta, sizeof(meta), "%s  -  %d lines", source.name.c_str(), source.lines);
        writeText(frame.fonts.regular, 11.0f, kMuted, LinearVec2(left + 16.0f, y + 26.0f), meta);
        y += kDocRow;
    }

    LinearUI::EndChild();
    LinearUI::PopStyleVar();
}

void drawSectionList(Frame& frame, float top, float height) {
    const float left = frame.origin.x + kPadding;
    const float innerWidth = kSidebarWidth - kPadding * 2.0f;
    const std::vector<Section>& sections = frame.state.page.sections;
    const linear::ui::theme::Colors& palette = linear::ui::theme::colors();

    // The heading the reader is inside, so the list says where they are as well as
    // where they can go. It uses the scroll position read back last frame, which
    // is the only one there is before the page has drawn.
    int current = -1;
    for (std::size_t index = 0; index < sections.size(); ++index) {
        if (sections[index].y <= frame.state.scroll + 6.0f) current = static_cast<int>(index);
    }

    LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0.0f, 0.0f));
    LinearUI::SetCursorScreenPos(LinearVec2(left, top));
    LinearUI::BeginChild("##sections", LinearVec2(innerWidth, height), 0,
                         LinearUIWindowFlags_NoBackground);

    float y = top;
    for (std::size_t index = 0; index < sections.size(); ++index) {
        const Section& section = sections[index];
        const int position = static_cast<int>(index);
        const bool active = position == current;
        const float indent = section.level >= 3 ? 26.0f : 12.0f;

        LinearUI::PushID(1000 + position);
        LinearUI::SetCursorScreenPos(LinearVec2(left, y));
        const bool pressed =
            LinearUI::InvisibleButton("##section", LinearVec2(innerWidth, kSectionRow));
        const bool hovered = LinearUI::IsItemHovered();
        LinearUI::PopID();
        if (pressed) frame.state.jump = section.y;

        if (active || hovered) {
            frame.draw->AddRectFilled(LinearVec2(left, y),
                                      LinearVec2(left + innerWidth, y + kSectionRow),
                                      colorOf(palette.tab), 4.0f);
        }
        if (active) {
            frame.draw->AddRectFilled(LinearVec2(left, y + 6.0f),
                                      LinearVec2(left + 2.0f, y + kSectionRow - 6.0f),
                                      colorOf(kAccent), 1.0f);
        }

        writeText(frame.fonts.regular, 12.0f, active ? kText : kMuted,
                  LinearVec2(left + indent, y + 4.0f),
                  fit(frame.fonts.regular, 12.0f, section.title, innerWidth - indent - 12.0f));
        y += kSectionRow;
    }

    LinearUI::EndChild();
    LinearUI::PopStyleVar();
}

void drawSidebar(Frame& frame) {
    const linear::ui::theme::Colors& palette = linear::ui::theme::colors();
    frame.draw->AddRectFilled(frame.origin,
                              LinearVec2(frame.origin.x + kSidebarWidth,
                                         frame.origin.y + frame.displayHeight),
                              colorOf(palette.panel));
    frame.draw->AddLine(LinearVec2(frame.origin.x + kSidebarWidth, frame.origin.y),
                        LinearVec2(frame.origin.x + kSidebarWidth,
                                   frame.origin.y + frame.displayHeight),
                        colorOf(kBorder), 1.0f);

    drawBrand(frame);

    const float left = frame.origin.x + kPadding;
    float y = frame.origin.y + 92.0f;
    writeText(frame.fonts.bold, 10.5f, kMuted, LinearVec2(left, y), "DOCUMENTS");
    y += 20.0f;

    const float listHeight =
        std::min(static_cast<float>(frame.state.sources.size()), 7.0f) * kDocRow;
    if (listHeight > 0.0f) {
        drawDocumentList(frame, y, listHeight);
        y += listHeight;
    }

    y += 20.0f;
    writeText(frame.fonts.bold, 10.5f, kMuted, LinearVec2(left, y), "IN THIS DOCUMENT");
    y += 20.0f;

    const float sectionsHeight =
        frame.origin.y + frame.displayHeight - kFooterHeight - y - 14.0f;
    if (sectionsHeight > kSectionRow) drawSectionList(frame, y, sectionsHeight);
}

float contentWidthOf(const Frame& frame) {
    const float contentLeft = frame.origin.x + kSidebarWidth + kContentLeft;
    return frame.origin.x + frame.displayWidth - kContentLeft - contentLeft;
}

// A document is parsed once at startup and laid out when it is opened or the
// column changes width, not on every frame. It runs before anything is drawn, so
// the section list beside the page always belongs to the page it sits next to.
void ensurePage(Frame& frame) {
    if (frame.state.sources.empty()) return;
    const float width = contentWidthOf(frame) - kScrollbarRoom;
    if (frame.state.pageDocument == frame.state.selected &&
        std::fabs(frame.state.pageWidth - width) <= 0.5f) {
        return;
    }
    frame.state.page = linear::docs::layOut(
        frame.state.blocks[static_cast<std::size_t>(frame.state.selected)], width, frame.metrics);
    frame.state.pageDocument = frame.state.selected;
    frame.state.pageWidth = width;
}

LinearVec4 lineColor(LineStyle style) {
    return style == LineStyle::Quote ? kMuted : kText;
}

LinearVec4 runColor(RunStyle style, LineStyle line) {
    if (style == RunStyle::Code || style == RunStyle::Link) return kAccentStrong;
    return lineColor(line);
}

void drawBoxes(Frame& frame, float contentLeft, float pageTop, float scroll, float visibleTop,
               float visibleBottom) {
    const linear::ui::theme::Colors& palette = linear::ui::theme::colors();
    for (const Box& box : frame.state.page.boxes) {
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
                frame.draw->AddRectFilled(min, max, colorOf(kAccentStrong), 1.5f);
                break;
            case Box::Kind::Rule:
                frame.draw->AddLine(min, LinearVec2(max.x, min.y), colorOf(kBorder), 1.0f);
                break;
        }
    }
}

void drawLines(Frame& frame, float contentLeft, float pageTop, float scroll, float visibleTop,
               float visibleBottom) {
    for (const Line& line : frame.state.page.lines) {
        if (line.y + line.height < visibleTop || line.y > visibleBottom) continue;
        const TextStyle set = linear::docs::textStyle(line.style);
        const float y = pageTop + line.y - scroll;

        if (!line.marker.empty()) {
            writeText(frame.fonts.bold, set.size, kAccentStrong,
                      LinearVec2(contentLeft + line.x - kMarkerOffset, y), line.marker);
        }

        // Runs are placed by the widths the layout measured, not by letting the
        // text widget advance the cursor: that is what keeps a bold phrase or a
        // code span from shifting the rest of the line.
        float x = line.x;
        for (const LineRun& run : line.runs) {
            x += run.spaceBefore;
            const bool bold = set.bold || run.style == RunStyle::Strong;
            writeText(fontFor(frame.fonts, bold, run.mono), set.size,
                      runColor(run.style, line.style), LinearVec2(contentLeft + x, y), run.text);
            x += frame.metrics.width(run.text, set.size, bold, run.mono);
        }
    }
}

void drawContent(Frame& frame) {
    const float contentLeft = frame.origin.x + kSidebarWidth + kContentLeft;
    const float contentWidth = contentWidthOf(frame);
    const float headerTop = frame.origin.y + 28.0f;

    frame.draw->AddLine(
        LinearVec2(frame.origin.x + kSidebarWidth + 1.0f, frame.origin.y + kHeaderHeight),
        LinearVec2(frame.origin.x + frame.displayWidth, frame.origin.y + kHeaderHeight),
        colorOf(kBorder), 1.0f);

    if (frame.state.sources.empty()) {
        writeText(frame.fonts.bold, 26.0f, kText, LinearVec2(contentLeft, headerTop),
                  "Documentation");
        const std::string message = frame.state.error.empty()
                                        ? "No documentation was found beside this program."
                                        : frame.state.error;
        writeText(frame.fonts.regular, 15.0f, kMuted, LinearVec2(contentLeft, headerTop + 46.0f),
                  message);
        writeText(frame.fonts.regular, 15.0f, kMuted, LinearVec2(contentLeft, headerTop + 72.0f),
                  "LinearDocs shows the .md files in the docs folder beside it.");
        return;
    }

    const Source& source = frame.state.sources[static_cast<std::size_t>(frame.state.selected)];
    writeText(frame.fonts.bold, 26.0f, kText, LinearVec2(contentLeft, headerTop),
              fit(frame.fonts.bold, 26.0f, source.title, contentWidth));
    char meta[400] = {};
    std::snprintf(meta, sizeof(meta), "%s  -  %d lines  -  %s", source.name.c_str(), source.lines,
                  source.path.c_str());
    writeText(frame.fonts.regular, 11.5f, kMuted, LinearVec2(contentLeft, headerTop + 38.0f),
              fit(frame.fonts.regular, 11.5f, meta, contentWidth));

    const float pageTop = frame.origin.y + kHeaderHeight + 12.0f;
    const float pageHeight =
        frame.origin.y + frame.displayHeight - kFooterHeight - pageTop - 10.0f;
    const Page& page = frame.state.page;

    LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0.0f, 0.0f));
    LinearUI::SetCursorScreenPos(LinearVec2(contentLeft, pageTop));
    if (frame.state.jump >= 0.0f) {
        LinearUI::SetNextWindowScroll(LinearVec2(0.0f, frame.state.jump));
        frame.state.jump = -1.0f;
    }
    LinearUI::BeginChild("##page", LinearVec2(contentWidth + 8.0f, pageHeight), 0,
                         LinearUIWindowFlags_NoBackground);
    // The reader can drag the scrollbar, so where it is has to be read back out of
    // the child: the section list and any jump both work from it.
    frame.state.scroll = LinearUI::GetScrollY();

    // One item as tall as the page gives the child something to scroll.
    LinearUI::SetCursorScreenPos(LinearVec2(contentLeft, pageTop));
    LinearUI::Dummy(LinearVec2(1.0f, page.height + 12.0f));

    const float visibleTop = frame.state.scroll - 40.0f;
    const float visibleBottom = frame.state.scroll + pageHeight + 40.0f;
    drawBoxes(frame, contentLeft, pageTop, frame.state.scroll, visibleTop, visibleBottom);
    drawLines(frame, contentLeft, pageTop, frame.state.scroll, visibleTop, visibleBottom);

    LinearUI::EndChild();
    LinearUI::PopStyleVar();
}

void drawFooter(Frame& frame) {
    const float top = frame.origin.y + frame.displayHeight - kFooterHeight;
    frame.draw->AddLine(LinearVec2(frame.origin.x, top),
                        LinearVec2(frame.origin.x + frame.displayWidth, top), colorOf(kBorder),
                        1.0f);

    char summary[192] = {};
    std::snprintf(summary, sizeof(summary), "LinearDocs  -  %d documents",
                  static_cast<int>(frame.state.sources.size()));
    writeText(frame.fonts.regular, 11.5f, kMuted,
              LinearVec2(frame.origin.x + kPadding, top + 14.0f), summary);

    const std::string hint = "Wheel scrolls  -  click a section to jump  -  Esc closes";
    LinearUI::PushFont(frame.fonts.regular, 11.5f);
    const float hintWidth = LinearUI::CalcTextSize(hint.c_str()).x;
    LinearUI::PopFont();
    writeText(frame.fonts.regular, 11.5f, kMuted,
              LinearVec2(frame.origin.x + frame.displayWidth - kPadding - hintWidth, top + 14.0f),
              hint);
}

}  // namespace

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    State state;
    const std::string folder = linear::docs::folderFor(linear::docs::executablePath());
    if (folder.empty()) {
        state.error = "No folder with .md files was found beside this program or above it.";
    } else if (!linear::docs::loadAll(folder, &state.sources, &state.error)) {
        state.sources.clear();
    }
    for (const Source& source : state.sources) {
        state.blocks.push_back(linear::docs::parse(source.text));
    }

    linearwinSetErrorCallback([](int, const char* description) {
        std::fprintf(stderr, "Linear window: %s\n",
                     description == nullptr ? "unknown" : description);
    });
    if (linearwinInit() == 0) {
        std::fprintf(stderr, "LinearDocs needs a window, and none could be created.\n");
        return 1;
    }

    linearwinWindowHint(LINEARWIN_CONTEXT_VERSION_MAJOR, 3);
    linearwinWindowHint(LINEARWIN_CONTEXT_VERSION_MINOR, 3);
    linearwinWindowHint(LINEARWIN_OPENGL_PROFILE, LINEARWIN_OPENGL_CORE_PROFILE);
    LINEARWINwindow* window = linearwinCreateWindow(kWindowWidth, kWindowHeight,
                                                    "Linear documentation", nullptr, nullptr);
    if (window == nullptr) {
        linearwinTerminate();
        std::fprintf(stderr, "LinearDocs needs a window, and none could be created.\n");
        return 1;
    }
    linearwinSetWindowSizeLimits(window, kWindowWidth, kWindowHeight, LINEARWIN_DONT_CARE,
                                 LINEARWIN_DONT_CARE);
    linearwinMakeContextCurrent(window);
    linearwinSwapInterval(1);

    if (linearglLoadGLLoader((LINEARGLloadproc)linearwinGetProcAddress) == 0) {
        linearwinDestroyWindow(window);
        linearwinTerminate();
        std::fprintf(stderr, "LinearDocs could not load OpenGL.\n");
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

    // The window owns the output from here, so the console it was started from has
    // nothing left to say. A shell that launched this keeps its own window.
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
        LinearUI::Begin("##linear-docs", nullptr,
                        LinearUIWindowFlags_NoTitleBar | LinearUIWindowFlags_NoResize |
                            LinearUIWindowFlags_NoMove | LinearUIWindowFlags_NoCollapse |
                            LinearUIWindowFlags_NoScrollbar |
                            LinearUIWindowFlags_NoScrollWithMouse |
                            LinearUIWindowFlags_NoSavedSettings |
                            LinearUIWindowFlags_NoBringToFrontOnFocus);
        LinearUI::PopStyleVar();

        Frame frame{state, fonts, metrics, LinearUI::GetWindowDrawList(), LinearUI::GetWindowPos(),
                    io.DisplaySize.x, io.DisplaySize.y};
        ensurePage(frame);
        drawSidebar(frame);
        drawContent(frame);
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
