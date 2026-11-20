// The Markdown parser and the layout behind LinearDocs.
//
// The window cannot be opened on a build server, but nothing that decides what
// the window shows needs one: parsing and laying out take text and a width and
// return positions. The font is faked here - every glyph is half the point size
// wide - so the wrapped lines are predictable enough to check.

#include "DocsLayout.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {

int g_failures = 0;

void expect(bool condition, const char* what) {
    if (condition) return;
    std::printf("FAIL: %s\n", what);
    ++g_failures;
}

void expectEqual(const std::string& actual, const std::string& expected, const char* what) {
    if (actual == expected) return;
    std::printf("FAIL: %s (got \"%s\", wanted \"%s\")\n", what, actual.c_str(),
                expected.c_str());
    ++g_failures;
}

linear::docs::Metrics fakeMetrics() {
    linear::docs::Metrics metrics;
    metrics.width = [](const std::string& text, float size, bool, bool) {
        return static_cast<float>(text.size()) * size * 0.5f;
    };
    metrics.step = [](float size) { return std::floor(size * 1.65f) + 1.0f; };
    return metrics;
}

const linear::docs::Block* firstOf(const std::vector<linear::docs::Block>& blocks,
                                   linear::docs::Block::Kind kind) {
    for (const linear::docs::Block& block : blocks) {
        if (block.kind == kind) return &block;
    }
    return nullptr;
}

void parseHeadings() {
    const std::vector<linear::docs::Block> blocks =
        linear::docs::parse("# Linear\n\n## Where things live ##\n\n#### Deep\n");
    expect(blocks.size() == 3, "three headings make three blocks");
    if (blocks.size() != 3) return;
    expect(blocks[0].kind == linear::docs::Block::Kind::Heading, "a heading is a heading");
    expect(blocks[0].level == 1, "one # is level one");
    expectEqual(blocks[1].words[0].text, "Where", "a heading keeps its first word");
    expectEqual(blocks[1].words.back().text, "live", "a closing run of # is syntax");
    expect(blocks[2].level == 4, "four # are level four");
}

void parseParagraphsJoinLines() {
    const std::vector<linear::docs::Block> blocks =
        linear::docs::parse("one line\nand its continuation\n\nafter the gap\n");
    expect(blocks.size() == 2, "a blank line ends a paragraph");
    if (blocks.size() != 2) return;
    const std::vector<linear::docs::Run>& words = blocks[0].words;
    expect(words.size() == 5, "the second line is joined onto the first");
    if (words.size() == 5) {
        expectEqual(words[2].text, "and", "the joined line continues the sentence");
    }
}

void parseLists() {
    const std::vector<linear::docs::Block> blocks = linear::docs::parse(
        "- first item, which\n  carries on here\n- second\n\n1. numbered\n2. again\n");
    expect(blocks.size() == 4, "two bullets and two numbered items");
    if (blocks.size() != 4) return;
    expect(blocks[0].kind == linear::docs::Block::Kind::Bullet, "a dash is a bullet");
    expect(blocks[0].words.size() == 6, "an indented line continues the bullet");
    expect(blocks[2].kind == linear::docs::Block::Kind::Numbered, "1. is a numbered item");
    expect(blocks[2].number == 1, "the number written in the file is kept");
    expect(blocks[3].number == 2, "and so is the next one");
}

void parseCodeKeepsLines() {
    const std::vector<linear::docs::Block> blocks =
        linear::docs::parse("```powershell\ndir *.md | sort\n\ndone\n```\n");
    expect(blocks.size() == 1, "a fence is one block");
    if (blocks.size() != 1) return;
    expect(blocks[0].kind == linear::docs::Block::Kind::Code, "the fence is a code block");
    expect(blocks[0].code.size() == 3, "including the blank line, every line is kept");
    if (blocks[0].code.size() == 3) {
        expectEqual(blocks[0].code[1], "", "a blank line inside the fence stays blank");
        expectEqual(blocks[0].code[2], "done", "the last line is kept as written");
    }
}

void parseTables() {
    const std::vector<linear::docs::Block> blocks = linear::docs::parse(
        "| Area | Files |\n| --- | --- |\n| Build | `src/X.cpp` |\n"
        "| Docs | README.md |\n\nAfter.\n");
    const linear::docs::Block* table = firstOf(blocks, linear::docs::Block::Kind::Table);
    expect(table != nullptr, "a pipe table is a table");
    if (table == nullptr) return;
    expect(table->rows.size() == 3, "the divider row is not a row of the table");
    if (table->rows.size() != 3) return;
    expect(table->rows[0].size() == 2, "two columns");
    expectEqual(table->rows[1][1], "src/X.cpp", "a cell keeps its text and drops the backticks");
}

void parseQuoteAndRule() {
    const std::vector<linear::docs::Block> blocks =
        linear::docs::parse("> a note\n> over two lines\n\n---\n");
    expect(blocks.size() == 2, "a quote and a rule");
    if (blocks.size() != 2) return;
    expect(blocks[0].kind == linear::docs::Block::Kind::Quote, "> is a quote");
    expect(blocks[0].words.size() == 5, "the quote's lines are joined");
    expect(blocks[1].kind == linear::docs::Block::Kind::Rule, "--- is a rule");
}

void parseInline() {
    const std::vector<linear::docs::Block> blocks = linear::docs::parse(
        "call `CalcTextSize()` and **bold** and [SECURITY](SECURITY.md) but not face*.png\n");
    expect(blocks.size() == 1, "one paragraph");
    if (blocks.size() != 1) return;

    bool sawCode = false;
    bool sawStrong = false;
    bool sawLink = false;
    bool sawLiteral = false;
    for (const linear::docs::Run& word : blocks[0].words) {
        if (word.text == "CalcTextSize()" && word.style == linear::docs::RunStyle::Code) {
            sawCode = true;
        }
        if (word.text == "bold" && word.style == linear::docs::RunStyle::Strong) sawStrong = true;
        if (word.text == "SECURITY" && word.style == linear::docs::RunStyle::Link) sawLink = true;
        if (word.text == "face*.png") sawLiteral = true;
    }
    expect(sawCode, "a code span is one word, brackets and all");
    expect(sawStrong, "double stars mark a bold phrase");
    expect(sawLink, "a link reads as its label");
    expect(sawLiteral, "an asterisk inside a file name stays a character");
}

linear::docs::Page layOutText(const std::string& text, float width,
                              linear::docs::Metrics* metrics) {
    const std::vector<linear::docs::Block> blocks = linear::docs::parse(text);
    return linear::docs::layOut(blocks, width, *metrics);
}

void layoutFitsTheColumn() {
    linear::docs::Metrics metrics = fakeMetrics();
    const linear::docs::Page page = layOutText(
        "# Title\n\nA paragraph that is long enough to need more than one line at this width, "
        "which is the point of wrapping.\n\n- a bullet with `code` in it, and enough words to "
        "wrap onto a second line\n",
        400.0f, &metrics);

    expect(!page.lines.empty(), "the page has lines");
    float previous = -1.0f;
    for (const linear::docs::Line& line : page.lines) {
        expect(line.y >= previous, "lines are laid out downwards");
        previous = line.y;

        float x = line.x;
        const linear::docs::TextStyle set = linear::docs::textStyle(line.style);
        for (const linear::docs::LineRun& run : line.runs) {
            x += run.spaceBefore;
            const bool bold = set.bold || run.style == linear::docs::RunStyle::Strong;
            x += metrics.width(run.text, set.size, bold, run.mono);
        }
        expect(x <= 400.0f + 0.5f, "a wrapped line stays inside the column");
    }
    expect(page.height >= previous, "the page is at least as tall as its last line");
}

void layoutWrapsInsteadOfClipping() {
    linear::docs::Metrics metrics = fakeMetrics();
    const linear::docs::Page page = layOutText(
        "one two three four five six seven eight nine ten eleven twelve thirteen fourteen\n",
        200.0f, &metrics);
    expect(page.lines.size() > 1, "a long paragraph makes more than one line");
    // Every word has to survive wrapping, in order.
    std::string joined;
    for (const linear::docs::Line& line : page.lines) {
        for (const linear::docs::LineRun& run : line.runs) {
            if (!joined.empty()) joined += ' ';
            joined += run.text;
        }
    }
    expectEqual(joined,
                "one two three four five six seven eight nine ten eleven twelve thirteen fourteen",
                "wrapping moves words, not drops them");
}

void layoutMarksBulletsOnce() {
    linear::docs::Metrics metrics = fakeMetrics();
    const linear::docs::Page page = layOutText(
        "- a bullet long enough to reach a second line at this width, so the marker has to stay "
        "on the first one\n",
        240.0f, &metrics);
    expect(page.lines.size() > 1, "the bullet wraps");
    if (page.lines.size() < 2) return;
    expect(!page.lines[0].marker.empty(), "the first line carries the bullet");
    expect(page.lines[1].marker.empty(), "the second line carries none");
    expect(page.lines[1].x == page.lines[0].x, "and it starts in the same column");
}

void layoutSectionsAndBoxes() {
    linear::docs::Metrics metrics = fakeMetrics();
    const linear::docs::Page page = layOutText(
        "# One\n\ntext\n\n## Two\n\n```\ncode\ncode\n```\n\n| a | b |\n| --- | --- |\n| 1 | 2 |\n",
        500.0f, &metrics);

    expect(page.sections.size() == 2, "both headings are listed as sections");
    if (page.sections.size() == 2) {
        expect(page.sections[0].y < page.sections[1].y, "sections are ordered down the page");
        expectEqual(page.sections[1].title, "Two", "a section is named by its heading");
    }

    int codeBoxes = 0;
    int tableBoxes = 0;
    for (const linear::docs::Box& box : page.boxes) {
        if (box.kind == linear::docs::Box::Kind::Code) {
            ++codeBoxes;
            // The panel has to cover every line of the block it belongs to.
            int inside = 0;
            for (const linear::docs::Line& line : page.lines) {
                if (line.style != linear::docs::LineStyle::Code) continue;
                expect(line.y >= box.y0 && line.y + line.height <= box.y1,
                       "a code line sits inside its panel");
                ++inside;
            }
            expect(inside == 2, "the panel belongs to the block's two lines");
        }
        if (box.kind == linear::docs::Box::Kind::TableHeader) ++tableBoxes;
    }
    expect(codeBoxes == 1, "one code block makes one panel");
    expect(tableBoxes == 1, "a table header gets its own band");
}

void layoutTableFitsItsColumns() {
    linear::docs::Metrics metrics = fakeMetrics();
    const linear::docs::Page page = layOutText(
        "| Route | What it does |\n"
        "| --- | --- |\n"
        "| GET /api/games | lists the games, with the page size and the search term as query "
        "parameters |\n",
        360.0f, &metrics);

    std::vector<const linear::docs::Line*> cells;
    for (const linear::docs::Line& line : page.lines) {
        if (line.style == linear::docs::LineStyle::TableCell) cells.push_back(&line);
    }
    expect(cells.size() >= 3, "the long cell wraps inside its column");
    for (const linear::docs::Line* cell : cells) {
        expect(cell->x >= 0.0f && cell->x < 360.0f, "a cell starts inside the table");
    }
    // The table must not push the page wider than the column it was given.
    for (const linear::docs::Box& box : page.boxes) {
        if (box.kind == linear::docs::Box::Kind::TableHeader ||
            box.kind == linear::docs::Box::Kind::TableRow) {
            expect(box.x1 <= 360.0f + 0.5f, "a row fits the column");
        }
    }
}

void layoutRuleIsALine() {
    linear::docs::Metrics metrics = fakeMetrics();
    const linear::docs::Page page = layOutText("above\n\n---\n\nbelow\n", 400.0f, &metrics);
    int rules = 0;
    for (const linear::docs::Box& box : page.boxes) {
        if (box.kind != linear::docs::Box::Kind::Rule) continue;
        ++rules;
        expect(box.y0 == box.y1, "a rule has no height of its own");
    }
    expect(rules == 1, "one rule makes one line");
}

}  // namespace

int main() {
    parseHeadings();
    parseParagraphsJoinLines();
    parseLists();
    parseCodeKeepsLines();
    parseTables();
    parseQuoteAndRule();
    parseInline();

    layoutFitsTheColumn();
    layoutWrapsInsteadOfClipping();
    layoutMarksBulletsOnce();
    layoutSectionsAndBoxes();
    layoutTableFitsItsColumns();
    layoutRuleIsALine();

    if (g_failures == 0) {
        std::printf("DocsLayoutTests: all checks passed.\n");
        return 0;
    }
    std::printf("DocsLayoutTests: %d check(s) failed.\n", g_failures);
    return 1;
}
