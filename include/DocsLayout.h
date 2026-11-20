#pragma once

// The document model behind LinearDocs, the window that shows this project's own
// documentation.
//
// The files the app reads are Markdown, so this is the half of it that knows
// Markdown and nothing about windows: parse a file into blocks, then lay those
// blocks out as positioned lines at a given width. The font is the caller's
// business through Metrics, which is what lets a test check the layout with no
// window, no font and no GL context.

#include <functional>
#include <string>
#include <vector>

namespace linear {
namespace docs {

// How a piece of text inside a block is set: the body face, a bold phrase, an
// inline `code` span, or the label of a [link](target).
enum class RunStyle { Text, Strong, Code, Link };

// One word. Wrapping is per word, which is all these files need: no line of them
// depends on two words staying together.
struct Run {
    std::string text;
    RunStyle style = RunStyle::Text;
};

// The block kinds the project's documents actually use. Anything the parser does
// not recognise becomes a paragraph, which is the one answer that never loses
// text.
struct Block {
    enum class Kind { Heading, Paragraph, Bullet, Numbered, Code, Quote, Table, Rule };

    Kind kind = Kind::Paragraph;
    int level = 0;   // Heading: 1 to 6
    int number = 1;  // Numbered: the number written in the file
    std::vector<Run> words;                    // Heading, Paragraph, Bullet, Numbered, Quote
    std::vector<std::string> code;             // Code: the lines, as written
    std::vector<std::vector<std::string>> rows;  // Table: rows of cells, markup stripped
};

// Parses the Markdown these files are written in: ATX headings, paragraphs,
// bullets and numbered lists, fenced code, block quotes, pipe tables and rules.
std::vector<Block> parse(const std::string& source);

// How a laid-out line is set. textStyle() is the single table the layout and the
// window both read, so a heading cannot be measured at one size and drawn at
// another.
enum class LineStyle {
    Heading1,
    Heading2,
    Heading3,
    Heading4,
    Body,
    Bullet,
    Quote,
    Code,
    TableHeader,
    TableCell,
};

struct TextStyle {
    float size = 15.0f;
    bool bold = false;
    bool mono = false;
};

TextStyle textStyle(LineStyle style);

// The gaps before and after a block of that style.
struct Spacing {
    float above = 0.0f;
    float below = 0.0f;
};

Spacing spacingOf(LineStyle style);

// A run as it is placed on a line: the gap to it was measured at layout time
// with the same metrics the window draws with, so runs cannot drift apart.
struct LineRun {
    std::string text;
    RunStyle style = RunStyle::Text;
    bool mono = false;
    float spaceBefore = 0.0f;
};

struct Line {
    float y = 0.0f;
    float x = 0.0f;
    float height = 0.0f;  // the step to the next line, not the glyph height
    LineStyle style = LineStyle::Body;
    std::string marker;  // the bullet or the "3." drawn to the left of the text
    std::vector<LineRun> runs;
    int block = 0;
};

// An area drawn behind some lines: the panel of a code block, one table row, the
// bar beside a quote, or a horizontal rule.
struct Box {
    enum class Kind { Code, TableHeader, TableRow, Quote, Rule };

    Kind kind = Kind::Code;
    float x0 = 0.0f;
    float y0 = 0.0f;
    float x1 = 0.0f;
    float y1 = 0.0f;
    int block = 0;
};

// A heading, for the section list and for jumping to it.
struct Section {
    int level = 0;
    std::string title;
    float y = 0.0f;
    int block = 0;
};

struct Page {
    std::vector<Line> lines;
    std::vector<Box> boxes;
    std::vector<Section> sections;
    float height = 0.0f;
};

// What the layout needs to know about the fonts it is set in.
struct Metrics {
    // Width of `text` on one line at `size` points: in the bold face when `bold`,
    // the fixed-width one when `mono`.
    std::function<float(const std::string& text, float size, bool bold, bool mono)> width;
    // How far the next line starts below this one, at `size` points.
    std::function<float(float size)> step;
};

Page layOut(const std::vector<Block>& blocks, float width, const Metrics& metrics);

}  // namespace docs
}  // namespace linear
