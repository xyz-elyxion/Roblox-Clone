#include "DocsLayout.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <utility>

namespace linear {
namespace docs {
namespace {

// Widening these only moves text inside the column: every value is a gap the
// window has nothing to know about, because the layout has already added it.
constexpr float kBulletIndent = 26.0f;
constexpr float kQuoteIndent = 24.0f;
constexpr float kQuoteBar = 3.0f;
constexpr float kCodeIndent = 16.0f;
constexpr float kCodePadding = 8.0f;
constexpr float kCellPadding = 12.0f;
constexpr float kMinColumn = 76.0f;
constexpr float kTableGap = 16.0f;

std::string trimmed(const std::string& text) {
    std::size_t begin = 0;
    std::size_t end = text.size();
    while (begin < end && (text[begin] == ' ' || text[begin] == '\t' || text[begin] == '\r')) {
        ++begin;
    }
    while (end > begin &&
           (text[end - 1] == ' ' || text[end - 1] == '\t' || text[end - 1] == '\r')) {
        --end;
    }
    return text.substr(begin, end - begin);
}

bool startsWith(const std::string& text, const char* prefix) {
    const std::string head(prefix);
    return text.size() >= head.size() && text.compare(0, head.size(), head) == 0;
}

bool isBlank(char c) { return c == ' ' || c == '\t' || c == '\r'; }

bool isWordChar(char c) { return !isBlank(c); }

// ---------------------------------------------------------------------------
// Inline markup

// Appends `text` as words in one style. Words are the unit of wrapping, so a
// run of spaces collapses at a line break instead of stretching it.
void appendWords(const std::string& text, RunStyle style, std::vector<Run>* words,
                 const std::string& target = std::string()) {
    std::size_t index = 0;
    while (index < text.size()) {
        while (index < text.size() && isBlank(text[index])) ++index;
        const std::size_t start = index;
        while (index < text.size() && isWordChar(text[index])) ++index;
        if (index == start) break;
        Run run;
        run.text = text.substr(start, index - start);
        run.style = style;
        run.target = target;
        words->push_back(run);
    }
}

// Inline markup as these files write it: `code`, **bold**, *emphasis*,
// [label](target) and ![alt](target), plus a bare <address>. Every branch looks
// for its closing marker first, so a stray * or [ stays the character it was.
std::vector<Run> inlineWords(const std::string& text) {
    std::vector<Run> words;
    std::string plain;
    auto flushPlain = [&]() {
        appendWords(plain, RunStyle::Text, &words);
        plain.clear();
    };

    std::size_t index = 0;
    while (index < text.size()) {
        const char c = text[index];
        if (c == '`') {
            const std::size_t close = text.find('`', index + 1);
            if (close != std::string::npos) {
                flushPlain();
                appendWords(text.substr(index + 1, close - index - 1), RunStyle::Code, &words);
                index = close + 1;
                continue;
            }
        } else if (c == '*' && index + 1 < text.size() && text[index + 1] == '*') {
            const std::size_t close = text.find("**", index + 2);
            if (close != std::string::npos) {
                flushPlain();
                appendWords(text.substr(index + 2, close - index - 2), RunStyle::Strong, &words);
                index = close + 2;
                continue;
            }
        } else if (c == '*') {
            // Emphasised text is drawn as ordinary text: the sentence matters and
            // the slant does not. The star has to sit on a word boundary on both
            // sides, or a name like face*.png would lose it.
            const std::size_t close = text.find('*', index + 1);
            const bool opening =
                index == 0 || !std::isalnum(static_cast<unsigned char>(text[index - 1]));
            const bool closing = close != std::string::npos && close > index + 1 &&
                                 (close + 1 >= text.size() ||
                                  !std::isalnum(static_cast<unsigned char>(text[close + 1])));
            if (opening && closing && text[index + 1] != ' ') {
                flushPlain();
                appendWords(text.substr(index + 1, close - index - 1), RunStyle::Text, &words);
                index = close + 1;
                continue;
            }
        } else if (c == '[' || (c == '!' && index + 1 < text.size() && text[index + 1] == '[')) {
            const std::size_t open = c == '!' ? index + 1 : index;
            const std::size_t label = text.find(']', open + 1);
            if (label != std::string::npos && label + 1 < text.size() && text[label + 1] == '(') {
                const std::size_t close = text.find(')', label + 2);
                if (close != std::string::npos) {
                    flushPlain();
                    // A link reads as its label. The target is an anchor in the
                    // same file or a path beside it, and printing it would make
                    // the sentence harder to read rather than easier. It travels
                    // with the run so a window can open it.
                    appendWords(text.substr(open + 1, label - open - 1), RunStyle::Link, &words,
                                text.substr(label + 2, close - label - 2));
                    index = close + 1;
                    continue;
                }
            }
        } else if (c == '<') {
            const std::size_t close = text.find('>', index + 1);
            if (close != std::string::npos) {
                flushPlain();
                // Kept whole, brackets and all: this is an address to type.
                appendWords(text.substr(index, close - index + 1), RunStyle::Link, &words);
                index = close + 1;
                continue;
            }
        }
        plain.push_back(c);
        ++index;
    }
    flushPlain();
    return words;
}

std::string plainOf(const std::vector<Run>& words) {
    std::string text;
    for (const Run& word : words) {
        if (!text.empty()) text += ' ';
        text += word.text;
    }
    return text;
}

// ---------------------------------------------------------------------------
// Block structure

bool isRule(const std::string& text) {
    if (text.size() < 3) return false;
    const char first = text[0];
    if (first != '-' && first != '*' && first != '_') return false;
    for (char c : text) {
        if (c != first && c != ' ') return false;
    }
    return true;
}

bool isTableLine(const std::string& text) { return !text.empty() && text[0] == '|'; }

// The |---|---| line under a table's header. Its cells say only how to align
// them, so it becomes no row of its own.
bool isTableDivider(const std::string& text) {
    if (!isTableLine(text)) return false;
    bool sawDash = false;
    for (char c : text) {
        if (c == '-') {
            sawDash = true;
        } else if (c != '|' && c != ':' && c != ' ') {
            return false;
        }
    }
    return sawDash;
}

std::vector<std::string> splitRow(const std::string& line) {
    std::string row = trimmed(line);
    if (!row.empty() && row.front() == '|') row.erase(row.begin());
    if (!row.empty() && row.back() == '|') row.pop_back();

    std::vector<std::string> cells;
    std::size_t start = 0;
    for (std::size_t index = 0; index <= row.size(); ++index) {
        if (index == row.size() || row[index] == '|') {
            cells.push_back(trimmed(row.substr(start, index - start)));
            start = index + 1;
        }
    }
    return cells;
}

int headingLevel(const std::string& text) {
    std::size_t hashes = 0;
    while (hashes < text.size() && text[hashes] == '#') ++hashes;
    if (hashes == 0 || hashes > 6 || hashes >= text.size() || text[hashes] != ' ') return 0;
    return static_cast<int>(hashes);
}

int numberedStart(const std::string& text) {
    std::size_t digits = 0;
    while (digits < text.size() && std::isdigit(static_cast<unsigned char>(text[digits]))) {
        ++digits;
    }
    if (digits == 0 || digits + 1 >= text.size()) return -1;
    if (text[digits] != '.' || text[digits + 1] != ' ') return -1;
    return static_cast<int>(std::atoi(text.substr(0, digits).c_str()));
}

bool isBullet(const std::string& text) {
    return text.size() > 2 && (text[0] == '-' || text[0] == '*' || text[0] == '+') &&
           text[1] == ' ';
}

// Whether a line starts a new block rather than continuing the paragraph above
// it. A paragraph runs until a blank line or one of these.
bool startsConstruct(const std::string& text) {
    if (text.empty()) return true;
    if (startsWith(text, "```")) return true;
    if (headingLevel(text) > 0) return true;
    if (text[0] == '>') return true;
    if (isBullet(text)) return true;
    if (numberedStart(text) >= 0) return true;
    if (isTableLine(text)) return true;
    return isRule(text);
}

// A list item's continuation lines are indented, so the item is gathered from
// them here instead of ending at its first line and leaving the rest to become a
// paragraph of its own.
void absorbContinuation(const std::vector<std::string>& lines, std::size_t* index,
                        std::string* content) {
    while (*index < lines.size()) {
        const std::string& line = lines[*index];
        if (line.empty() || line[0] != ' ') break;
        const std::string intro = trimmed(line);
        if (intro.empty() || startsConstruct(intro)) break;
        if (!content->empty()) *content += ' ';
        *content += intro;
        ++*index;
    }
}

}  // namespace

std::vector<Block> parse(const std::string& source) {
    std::vector<std::string> lines;
    std::size_t start = 0;
    while (true) {
        const std::size_t end = source.find('\n', start);
        if (end == std::string::npos) {
            lines.push_back(source.substr(start));
            break;
        }
        lines.push_back(source.substr(start, end - start));
        start = end + 1;
    }

    std::vector<Block> blocks;
    std::size_t index = 0;
    while (index < lines.size()) {
        const std::string text = trimmed(lines[index]);
        if (text.empty()) {
            ++index;
            continue;
        }

        if (startsWith(text, "```")) {
            Block block;
            block.kind = Block::Kind::Code;
            ++index;
            while (index < lines.size()) {
                const std::string codeLine = trimmed(lines[index]);
                if (startsWith(codeLine, "```")) {
                    ++index;
                    break;
                }
                block.code.push_back(lines[index]);
                ++index;
            }
            blocks.push_back(std::move(block));
            continue;
        }

        const int level = headingLevel(text);
        if (level > 0) {
            Block block;
            block.kind = Block::Kind::Heading;
            block.level = level;
            // A closing run of # is syntax, not part of the title.
            std::string title = trimmed(text.substr(static_cast<std::size_t>(level)));
            while (!title.empty() && title.back() == '#') title.pop_back();
            block.words = inlineWords(trimmed(title));
            blocks.push_back(std::move(block));
            ++index;
            continue;
        }

        if (isRule(text)) {
            Block block;
            block.kind = Block::Kind::Rule;
            blocks.push_back(std::move(block));
            ++index;
            continue;
        }

        if (isTableLine(text) && index + 1 < lines.size() &&
            isTableDivider(trimmed(lines[index + 1]))) {
            Block block;
            block.kind = Block::Kind::Table;
            block.rows.push_back(splitRow(text));
            index += 2;
            while (index < lines.size()) {
                const std::string row = trimmed(lines[index]);
                if (!isTableLine(row)) break;
                block.rows.push_back(splitRow(row));
                ++index;
            }
            for (std::vector<std::string>& row : block.rows) {
                for (std::string& cell : row) cell = plainOf(inlineWords(cell));
            }
            blocks.push_back(std::move(block));
            continue;
        }

        if (text[0] == '>') {
            Block block;
            block.kind = Block::Kind::Quote;
            std::string content;
            while (index < lines.size()) {
                const std::string quote = trimmed(lines[index]);
                if (quote.empty() || quote[0] != '>') break;
                const std::string inner = trimmed(quote.substr(1));
                if (!content.empty() && !inner.empty()) content += ' ';
                content += inner;
                ++index;
            }
            block.words = inlineWords(content);
            blocks.push_back(std::move(block));
            continue;
        }

        if (isBullet(text)) {
            Block block;
            block.kind = Block::Kind::Bullet;
            std::string content = trimmed(text.substr(2));
            ++index;
            absorbContinuation(lines, &index, &content);
            block.words = inlineWords(content);
            blocks.push_back(std::move(block));
            continue;
        }

        const int numbered = numberedStart(text);
        if (numbered >= 0) {
            Block block;
            block.kind = Block::Kind::Numbered;
            block.number = numbered;
            std::size_t digits = 0;
            while (digits < text.size() && std::isdigit(static_cast<unsigned char>(text[digits]))) {
                ++digits;
            }
            std::string content = trimmed(text.substr(digits + 2));
            ++index;
            absorbContinuation(lines, &index, &content);
            block.words = inlineWords(content);
            blocks.push_back(std::move(block));
            continue;
        }

        Block block;
        block.kind = Block::Kind::Paragraph;
        std::string paragraph;
        while (index < lines.size()) {
            const std::string intro = trimmed(lines[index]);
            if (intro.empty()) break;
            if (!paragraph.empty() && startsConstruct(intro)) break;
            if (!paragraph.empty()) paragraph += ' ';
            paragraph += intro;
            ++index;
        }
        block.words = inlineWords(paragraph);
        blocks.push_back(std::move(block));
    }
    return blocks;
}

TextStyle textStyle(LineStyle style) {
    TextStyle set;
    switch (style) {
        case LineStyle::Heading1:
            set.size = 30.0f;
            set.bold = true;
            break;
        case LineStyle::Heading2:
            set.size = 22.0f;
            set.bold = true;
            break;
        case LineStyle::Heading3:
            set.size = 18.0f;
            set.bold = true;
            break;
        case LineStyle::Heading4:
            set.size = 15.5f;
            set.bold = true;
            break;
        case LineStyle::Body:
        case LineStyle::Bullet:
        case LineStyle::Quote:
            set.size = 15.0f;
            break;
        case LineStyle::Code:
            set.size = 13.0f;
            set.mono = true;
            break;
        case LineStyle::TableHeader:
            set.size = 13.0f;
            set.bold = true;
            break;
        case LineStyle::TableCell:
            set.size = 13.0f;
            break;
    }
    return set;
}

Spacing spacingOf(LineStyle style) {
    Spacing space;
    switch (style) {
        case LineStyle::Heading1:
            space.above = 10.0f;
            space.below = 14.0f;
            break;
        case LineStyle::Heading2:
            space.above = 30.0f;
            space.below = 10.0f;
            break;
        case LineStyle::Heading3:
            space.above = 22.0f;
            space.below = 8.0f;
            break;
        case LineStyle::Heading4:
            space.above = 18.0f;
            space.below = 6.0f;
            break;
        case LineStyle::Body:
            space.below = 14.0f;
            break;
        case LineStyle::Bullet:
            space.below = 8.0f;
            break;
        case LineStyle::Quote:
            space.above = 2.0f;
            space.below = 12.0f;
            break;
        case LineStyle::Code:
        case LineStyle::TableHeader:
        case LineStyle::TableCell:
            break;
    }
    return space;
}

namespace {

// Lays blocks out by remembering one number, the y of the next line, and
// appending to the page as it goes. Everything the window draws is a position in
// this page: it never works out a gap for itself.
class Layout {
public:
    Layout(float width, const Metrics& metrics) : width_(width), metrics_(metrics) {}

    Page run(const std::vector<Block>& blocks) {
        for (std::size_t index = 0; index < blocks.size(); ++index) {
            block_ = static_cast<int>(index);
            const Block& block = blocks[index];
            switch (block.kind) {
                case Block::Kind::Heading:
                    addHeading(block);
                    break;
                case Block::Kind::Paragraph:
                    addText(LineStyle::Body, 0.0f, std::string(), block.words);
                    break;
                case Block::Kind::Bullet:
                    addText(LineStyle::Bullet, kBulletIndent, std::string("\xE2\x80\xA2"),
                            block.words);
                    break;
                case Block::Kind::Numbered:
                    addText(LineStyle::Bullet, kBulletIndent,
                            std::to_string(block.number) + ".", block.words);
                    break;
                case Block::Kind::Quote:
                    addQuote(block);
                    break;
                case Block::Kind::Code:
                    addCode(block);
                    break;
                case Block::Kind::Table:
                    addTable(block);
                    break;
                case Block::Kind::Rule:
                    addRule();
                    break;
            }
        }
        page_.height = y_;
        return std::move(page_);
    }

private:
    // Wraps `words` into lines that start at `x`, breaking before a word would
    // pass `right`. The first line carries `marker`; the rest start at the same
    // x with nothing beside them, which is what keeps an item's text in one
    // column.
    std::vector<Line> wrapWords(LineStyle style, float x, const std::string& marker,
                                const std::vector<Run>& words, float right) {
        const TextStyle set = textStyle(style);
        std::vector<Line> lines;
        Line line;
        line.style = style;
        line.x = x;
        line.marker = marker;
        float used = 0.0f;

        auto flush = [&]() {
            if (line.runs.empty()) return;
            line.y = 0.0f;
            line.height = metrics_.step(set.size);
            lines.push_back(line);
            line = Line();
            line.style = style;
            line.x = x;
            used = 0.0f;
        };

        for (const Run& word : words) {
            const bool mono = set.mono || word.style == RunStyle::Code;
            const bool bold = set.bold || word.style == RunStyle::Strong;
            const float wordWidth = metrics_.width(word.text, set.size, bold, mono);
            const float gap = line.runs.empty() ? 0.0f : metrics_.width(" ", set.size, bold, mono);
            if (!line.runs.empty() && x + used + gap + wordWidth > right) flush();
            LineRun out;
            out.text = word.text;
            out.style = word.style;
            out.mono = mono;
            out.spaceBefore = line.runs.empty() ? 0.0f : gap;
            out.target = word.target;
            line.runs.push_back(out);
            used += out.spaceBefore + wordWidth;
        }
        flush();
        if (lines.empty()) {
            // An empty block still takes a line, so the gaps around it do not
            // collapse into each other.
            Line empty;
            empty.style = style;
            empty.x = x;
            empty.height = metrics_.step(set.size);
            lines.push_back(empty);
        }
        return lines;
    }

    // Appends lines at `top` and returns the height they took.
    float emit(const std::vector<Line>& lines, float top) {
        float offset = 0.0f;
        for (const Line& source : lines) {
            Line line = source;
            line.y = top + offset;
            line.block = block_;
            offset += line.height;
            page_.lines.push_back(std::move(line));
        }
        return offset;
    }

    void addText(LineStyle style, float indent, const std::string& marker,
                 const std::vector<Run>& words) {
        const Spacing space = spacingOf(style);
        y_ += space.above;
        const std::vector<Line> lines =
            wrapWords(style, indent, marker, words, width_ - kCodeIndent);
        y_ += emit(lines, y_) + space.below;
    }

    void addHeading(const Block& block) {
        const int level = std::min(std::max(block.level, 1), 4);
        const LineStyle style = level == 1   ? LineStyle::Heading1
                                : level == 2 ? LineStyle::Heading2
                                : level == 3 ? LineStyle::Heading3
                                             : LineStyle::Heading4;
        const Spacing space = spacingOf(style);
        y_ += space.above;
        // The section list jumps to this y, so it is recorded before the lines
        // are emitted rather than measured back out of them.
        if (level <= 3) {
            Section section;
            section.level = level;
            section.title = plainOf(block.words);
            section.y = y_;
            section.block = block_;
            page_.sections.push_back(std::move(section));
        }
        const std::vector<Line> lines = wrapWords(style, 0.0f, std::string(), block.words, width_);
        y_ += emit(lines, y_) + space.below;
    }

    void addQuote(const Block& block) {
        const Spacing space = spacingOf(LineStyle::Quote);
        y_ += space.above;
        const float top = y_;
        const std::vector<Line> lines = wrapWords(LineStyle::Quote, kQuoteIndent, std::string(),
                                                 block.words, width_ - kQuoteIndent);
        const float height = emit(lines, top);
        Box box;
        box.kind = Box::Kind::Quote;
        box.x0 = 0.0f;
        box.x1 = kQuoteBar;
        box.y0 = top;
        box.y1 = top + height;
        box.block = block_;
        page_.boxes.push_back(box);
        y_ += height + space.below;
    }

    void addCode(const Block& block) {
        const Spacing space = spacingOf(LineStyle::Code);
        y_ += 6.0f;
        const float top = y_;
        const TextStyle set = textStyle(LineStyle::Code);
        for (const std::string& text : block.code) {
            Line line;
            line.y = y_;
            line.x = kCodeIndent;
            line.style = LineStyle::Code;
            line.height = metrics_.step(set.size);
            line.block = block_;
            if (!text.empty()) {
                LineRun run;
                run.text = text;
                run.mono = true;
                line.runs.push_back(std::move(run));
            }
            page_.lines.push_back(std::move(line));
            y_ += line.height;
        }
        if (block.code.empty()) y_ += metrics_.step(set.size);

        Box box;
        box.kind = Box::Kind::Code;
        box.x0 = 0.0f;
        box.x1 = width_;
        box.y0 = top - kCodePadding;
        box.y1 = y_ + kCodePadding;
        box.block = block_;
        page_.boxes.push_back(box);
        y_ += space.below + 12.0f;
    }

    void addRule() {
        y_ += 12.0f;
        Box box;
        box.kind = Box::Kind::Rule;
        box.x0 = 0.0f;
        box.x1 = width_;
        box.y0 = y_;
        box.y1 = y_;
        box.block = block_;
        page_.boxes.push_back(box);
        y_ += 16.0f;
    }

    // Columns take the width their longest cell wants, then shrink in proportion
    // when the table is wider than the page: a table in these files is a
    // reference, not a picture, so it is better squeezed than cut off. A column
    // handed less than its minimum keeps the minimum and the rest share what is
    // left, which is the only way a long cell cannot push a short one to nothing.
    std::vector<float> columnWidths(const Block& block, std::size_t columns) const {
        const TextStyle set = textStyle(LineStyle::TableCell);
        std::vector<float> natural(columns, kMinColumn);
        for (const std::vector<std::string>& row : block.rows) {
            for (std::size_t cell = 0; cell < row.size() && cell < columns; ++cell) {
                const float want =
                    metrics_.width(row[cell], set.size, false, false) + kCellPadding * 2.0f;
                natural[cell] = std::max(natural[cell], want);
            }
        }

        std::vector<float> widths = natural;
        float total = 0.0f;
        for (float width : natural) total += width;
        if (total <= width_) return widths;

        std::vector<bool> pinned(columns, false);
        for (std::size_t pass = 0; pass <= columns; ++pass) {
            float pinnedWidth = 0.0f;
            float freeNatural = 0.0f;
            for (std::size_t cell = 0; cell < columns; ++cell) {
                if (pinned[cell]) {
                    pinnedWidth += natural[cell];
                } else {
                    freeNatural += natural[cell];
                }
            }
            const float budget = width_ - pinnedWidth;
            bool repinned = false;
            for (std::size_t cell = 0; cell < columns; ++cell) {
                if (pinned[cell]) continue;
                const float share = freeNatural > 0.0f ? budget * (natural[cell] / freeNatural)
                                                       : kMinColumn;
                if (share < kMinColumn) {
                    pinned[cell] = true;
                    repinned = true;
                }
                widths[cell] = std::max(share, kMinColumn);
            }
            // Every column at its minimum is wider than the page, but that is the
            // least bad answer: the table runs to the edge rather than setting text
            // in a column no wider than a letter.
            if (!repinned) break;
        }
        return widths;
    }

    void addTable(const Block& block) {
        std::size_t columns = 0;
        for (const std::vector<std::string>& row : block.rows) {
            columns = std::max(columns, row.size());
        }
        if (columns == 0) return;

        const std::vector<float> widths = columnWidths(block, columns);
        for (std::size_t row = 0; row < block.rows.size(); ++row) {
            const bool header = row == 0;
            const LineStyle style = header ? LineStyle::TableHeader : LineStyle::TableCell;
            const float step = metrics_.step(textStyle(style).size);

            std::vector<std::vector<Line>> cells;
            float x = 0.0f;
            float textHeight = step;
            for (std::size_t cell = 0; cell < columns; ++cell) {
                const std::string& text =
                    cell < block.rows[row].size() ? block.rows[row][cell] : std::string();
                std::vector<Run> words;
                appendWords(text, RunStyle::Text, &words);
                cells.push_back(wrapWords(style, x + kCellPadding, std::string(), words,
                                          x + widths[cell] - kCellPadding));
                textHeight = std::max(textHeight, static_cast<float>(cells.back().size()) * step);
                x += widths[cell];
            }

            const float height = textHeight + kCellPadding;
            Box box;
            box.kind = header ? Box::Kind::TableHeader : Box::Kind::TableRow;
            box.x0 = 0.0f;
            box.x1 = x;
            box.y0 = y_;
            box.y1 = y_ + height;
            box.block = block_;
            page_.boxes.push_back(box);

            // A cell that wraps finishes below where the next column starts, so the
            // row's lines are gathered and sorted: everything that walks the page
            // in order then sees it top to bottom.
            std::vector<Line> rowLines;
            for (const std::vector<Line>& lines : cells) {
                float offset = 0.0f;
                for (const Line& source : lines) {
                    Line line = source;
                    line.y = y_ + kCellPadding * 0.5f + offset;
                    line.block = block_;
                    offset += line.height;
                    rowLines.push_back(std::move(line));
                }
            }
            std::stable_sort(rowLines.begin(), rowLines.end(),
                             [](const Line& left, const Line& right) { return left.y < right.y; });
            for (Line& line : rowLines) page_.lines.push_back(std::move(line));
            y_ += height;
        }
        y_ += kTableGap;
    }

    float width_;
    const Metrics& metrics_;
    Page page_;
    float y_ = 0.0f;
    int block_ = 0;
};

}  // namespace

Page layOut(const std::vector<Block>& blocks, float width, const Metrics& metrics) {
    Layout layout(std::max(width, 120.0f), metrics);
    return layout.run(blocks);
}

}  // namespace docs
}  // namespace linear
