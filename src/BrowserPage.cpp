#include "BrowserPage.h"

#include <cctype>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

namespace linear {
namespace browser {
namespace {

bool isSpace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f';
}

std::string trim(const std::string& text) {
    std::size_t begin = 0;
    std::size_t end = text.size();
    while (begin < end && isSpace(text[begin])) ++begin;
    while (end > begin && isSpace(text[end - 1])) --end;
    return text.substr(begin, end - begin);
}

std::string lower(std::string text) {
    for (char& c : text) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return text;
}

bool startsWith(const std::string& text, const char* prefix) {
    const std::string head(prefix);
    return text.size() >= head.size() && text.compare(0, head.size(), head) == 0;
}

// Collapses every run of whitespace into one space, which is what a paragraph,
// a title or a table cell wants: the layout does its own wrapping.
std::string collapseSpaces(const std::string& text) {
    std::string out;
    bool pending = false;
    for (char c : text) {
        if (isSpace(c)) {
            pending = !out.empty();
            continue;
        }
        if (pending) {
            out.push_back(' ');
            pending = false;
        }
        out.push_back(c);
    }
    return out;
}

void appendUtf8(unsigned int code, std::string* out) {
    if (code == 0 || code > 0x10FFFF) return;
    if (code < 0x80) {
        out->push_back(static_cast<char>(code));
    } else if (code < 0x800) {
        out->push_back(static_cast<char>(0xC0 | (code >> 6)));
        out->push_back(static_cast<char>(0x80 | (code & 0x3F)));
    } else if (code < 0x10000) {
        out->push_back(static_cast<char>(0xE0 | (code >> 12)));
        out->push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
        out->push_back(static_cast<char>(0x80 | (code & 0x3F)));
    } else {
        out->push_back(static_cast<char>(0xF0 | (code >> 18)));
        out->push_back(static_cast<char>(0x80 | ((code >> 12) & 0x3F)));
        out->push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
        out->push_back(static_cast<char>(0x80 | (code & 0x3F)));
    }
}

// The named entities a page's prose actually uses, plus numeric ones. Anything
// unknown keeps its ampersand rather than disappearing, so an unknown entity
// cannot swallow the words around it.
std::string decodeEntities(const std::string& text) {
    std::string out;
    std::size_t index = 0;
    while (index < text.size()) {
        if (text[index] != '&') {
            out.push_back(text[index]);
            ++index;
            continue;
        }
        const std::size_t semicolon = text.find(';', index + 1);
        if (semicolon == std::string::npos || semicolon - index > 12) {
            out.push_back('&');
            ++index;
            continue;
        }
        const std::string name = text.substr(index + 1, semicolon - index - 1);
        std::string value;
        if (name == "amp") {
            value = "&";
        } else if (name == "lt") {
            value = "<";
        } else if (name == "gt") {
            value = ">";
        } else if (name == "quot") {
            value = "\"";
        } else if (name == "apos" || name == "nbsp") {
            value = name == "nbsp" ? " " : "'";
        } else if (name == "mdash") {
            value = "\xE2\x80\x94";
        } else if (name == "ndash") {
            value = "\xE2\x80\x93";
        } else if (name == "hellip") {
            value = "\xE2\x80\xA6";
        } else if (name == "copy") {
            value = "\xC2\xA9";
        } else if (name == "rarr") {
            value = "\xE2\x86\x92";
        } else if (name.size() > 1 && name[0] == '#') {
            const bool hex = name[1] == 'x' || name[1] == 'X';
            const char* digits = name.c_str() + (hex ? 2 : 1);
            const unsigned int code =
                static_cast<unsigned int>(std::strtoul(digits, nullptr, hex ? 16 : 10));
            appendUtf8(code, &value);
        } else {
            out.push_back('&');
            ++index;
            continue;
        }
        out += value;
        index = semicolon + 1;
    }
    return out;
}

std::string escapeHtml(const std::string& text) {
    std::string out;
    for (char c : text) {
        switch (c) {
            case '&':
                out += "&amp;";
                break;
            case '<':
                out += "&lt;";
                break;
            case '>':
                out += "&gt;";
                break;
            case '"':
                out += "&quot;";
                break;
            default:
                out.push_back(c);
                break;
        }
    }
    return out;
}

// The value of one attribute in the text between a tag's name and its closing
// bracket. Quoted and unquoted values are both read; a name with no value gives
// an empty string.
std::string attributeValue(const std::string& attributes, const std::string& wanted) {
    const std::string lowerWanted = lower(wanted);
    std::size_t index = 0;
    while (index < attributes.size()) {
        while (index < attributes.size() && isSpace(attributes[index])) ++index;
        const std::size_t nameStart = index;
        while (index < attributes.size() && !isSpace(attributes[index]) && attributes[index] != '=') {
            ++index;
        }
        const std::string name = lower(attributes.substr(nameStart, index - nameStart));
        while (index < attributes.size() && isSpace(attributes[index])) ++index;

        std::string value;
        if (index < attributes.size() && attributes[index] == '=') {
            ++index;
            while (index < attributes.size() && isSpace(attributes[index])) ++index;
            if (index < attributes.size() && (attributes[index] == '"' || attributes[index] == '\'')) {
                const char quote = attributes[index++];
                const std::size_t close = attributes.find(quote, index);
                value = attributes.substr(index, close == std::string::npos ? std::string::npos
                                                                            : close - index);
                index = close == std::string::npos ? attributes.size() : close + 1;
            } else {
                const std::size_t valueStart = index;
                while (index < attributes.size() && !isSpace(attributes[index]) &&
                       attributes[index] != '/') {
                    ++index;
                }
                value = attributes.substr(valueStart, index - valueStart);
            }
        }

        if (name == lowerWanted) return value;
        if (index == nameStart) ++index;  // never stand still on malformed markup
    }
    return std::string();
}

std::string withoutFragment(const std::string& url) {
    const std::size_t hash = url.find('#');
    return hash == std::string::npos ? url : url.substr(0, hash);
}

std::string withoutQueryAndFragment(const std::string& url) {
    std::string out = withoutFragment(url);
    const std::size_t query = out.find('?');
    return query == std::string::npos ? out : out.substr(0, query);
}

// Resolves `.` and `..` in a path without touching the query or fragment that
// follows it. A path that ended in a slash keeps it: "docs/" is a folder and
// "docs" is a file.
std::string normalisePath(const std::string& path) {
    const std::size_t suffixStart = path.find_first_of("?#");
    const std::string body = path.substr(0, suffixStart);
    const std::string suffix =
        suffixStart == std::string::npos ? std::string() : path.substr(suffixStart);
    const bool absolute = !body.empty() && body[0] == '/';

    std::vector<std::string> segments;
    std::size_t index = 0;
    while (index <= body.size()) {
        const std::size_t slash = body.find('/', index);
        const std::size_t stop = slash == std::string::npos ? body.size() : slash;
        const std::string segment = body.substr(index, stop - index);
        if (segment == "..") {
            if (!segments.empty()) segments.pop_back();
        } else if (!segment.empty() && segment != ".") {
            segments.push_back(segment);
        }
        if (slash == std::string::npos) break;
        index = slash + 1;
    }

    std::string out = absolute ? "/" : std::string();
    for (std::size_t segment = 0; segment < segments.size(); ++segment) {
        if (segment != 0) out += '/';
        out += segments[segment];
    }
    if (!body.empty() && body.back() == '/' && (out.empty() || out.back() != '/')) out += '/';
    return out + suffix;
}

int headingLevel(const std::string& name) {
    if (name.size() != 2 || name[0] != 'h') return 0;
    if (name[1] < '1' || name[1] > '6') return 0;
    return name[1] - '0';
}

// Tags that end the paragraph before them. They carry no meaning this reader
// needs beyond "a new block starts here", so the list is by name.
bool startsBlock(const std::string& name) {
    return name == "p" || name == "div" || name == "section" || name == "article" ||
           name == "header" || name == "footer" || name == "nav" || name == "main" ||
           name == "form" || name == "figure" || name == "figcaption" || name == "aside" ||
           name == "address" || name == "dl" || name == "dt" || name == "dd" ||
           name == "fieldset" || name == "legend" || name == "details" ||
           name == "summary" || name == "center" || name == "video" || name == "audio" ||
           name == "canvas";
}

class Reader {
public:
    Reader(const std::string& html, const std::string& pageUrl)
        : html_(html), lowered_(lower(html)), pageUrl_(pageUrl) {}

    Page run() {
        std::size_t index = 0;
        while (index < html_.size()) {
            const std::size_t open = html_.find('<', index);
            if (open == std::string::npos) {
                text(html_.substr(index));
                break;
            }
            if (open > index) text(html_.substr(index, open - index));

            if (html_.compare(open, 4, "<!--") == 0) {
                const std::size_t end = html_.find("-->", open + 4);
                index = end == std::string::npos ? html_.size() : end + 3;
                continue;
            }
            if (html_.compare(open, 2, "<!") == 0 || html_.compare(open, 2, "<?") == 0) {
                const std::size_t end = html_.find('>', open + 2);
                index = end == std::string::npos ? html_.size() : end + 1;
                continue;
            }

            const std::size_t close = html_.find('>', open + 1);
            if (close == std::string::npos) {
                // A '<' with no '>' is text: dropping the rest of the page would
                // lose words over a stray character.
                text(html_.substr(open));
                break;
            }

            std::string tag = html_.substr(open + 1, close - open - 1);
            index = close + 1;

            const bool closing = !tag.empty() && tag[0] == '/';
            if (closing) tag.erase(0, 1);
            if (!tag.empty() && tag.back() == '/') tag.pop_back();

            std::size_t nameEnd = 0;
            while (nameEnd < tag.size() && !isSpace(tag[nameEnd]) && tag[nameEnd] != '=') ++nameEnd;
            const std::string name = lower(tag.substr(0, nameEnd));
            if (name.empty()) continue;

            if (!closing && (name == "script" || name == "style" || name == "template")) {
                index = skipToClose(name, index);
                continue;
            }

            if (closing) {
                endTag(name);
            } else {
                startTag(name, tag.substr(nameEnd));
            }
        }

        flush();
        Page page;
        page.url = pageUrl_;
        page.title = title_.empty() ? pageUrl_ : title_;
        page.blocks = std::move(blocks_);
        return page;
    }

private:
    enum class Kind { Paragraph, Heading, Bullet, Numbered, Quote };

    struct List {
        bool ordered = false;
        int next = 1;
    };

    // The contents of a script or style element are code, not prose, and the
    // page is more readable without it. The search is case-insensitive and has
    // to stop at the tag's real end, or "</scriptx>" would end the wrong thing.
    std::size_t skipToClose(const std::string& name, std::size_t index) {
        const std::string closeTag = "</" + name;
        std::size_t end = lowered_.find(closeTag, index);
        while (end != std::string::npos) {
            const std::size_t after = end + closeTag.size();
            if (after >= lowered_.size() || lowered_[after] == '>' || isSpace(lowered_[after])) {
                break;
            }
            end = lowered_.find(closeTag, end + 1);
        }
        if (end == std::string::npos) return html_.size();
        const std::size_t bracket = html_.find('>', end);
        return bracket == std::string::npos ? html_.size() : bracket + 1;
    }

    void text(const std::string& raw) {
        const std::string decoded = decodeEntities(raw);
        if (inTitle_) {
            titleText_ += decoded;
            return;
        }
        if (pre_) {
            preText(decoded);
            return;
        }
        if (table_) {
            if (inCell_) cell_ += decoded;
            return;
        }
        addWords(decoded);
    }

    void addWords(const std::string& text) {
        std::size_t index = 0;
        while (index < text.size()) {
            while (index < text.size() && isSpace(text[index])) ++index;
            const std::size_t start = index;
            while (index < text.size() && !isSpace(text[index])) ++index;
            if (index == start) break;
            addWord(text.substr(start, index - start));
        }
    }

    void addWord(const std::string& word) {
        docs::Run run;
        run.text = word;
        if (code_ > 0) {
            run.style = docs::RunStyle::Code;
        } else if (bold_ > 0) {
            run.style = docs::RunStyle::Strong;
        } else if (link_ > 0) {
            run.style = docs::RunStyle::Link;
            run.target = href_;
        }
        words_.push_back(std::move(run));
    }

    void preText(const std::string& text) {
        for (char c : text) {
            if (c == '\r') continue;
            if (c == '\n') {
                preLines_.push_back(preLine_);
                preLine_.clear();
                continue;
            }
            preLine_.push_back(c);
        }
    }

    void flush() {
        if (!words_.empty()) {
            docs::Block block;
            switch (kind_) {
                case Kind::Heading:
                    block.kind = docs::Block::Kind::Heading;
                    block.level = heading_;
                    break;
                case Kind::Bullet:
                    block.kind = docs::Block::Kind::Bullet;
                    break;
                case Kind::Numbered:
                    block.kind = docs::Block::Kind::Numbered;
                    block.number = number_;
                    break;
                case Kind::Quote:
                    block.kind = docs::Block::Kind::Quote;
                    break;
                case Kind::Paragraph:
                    block.kind = docs::Block::Kind::Paragraph;
                    break;
            }
            block.words = std::move(words_);
            words_.clear();
            blocks_.push_back(std::move(block));
        }
        kind_ = Kind::Paragraph;
        heading_ = 0;
        number_ = 1;
    }

    void startTag(const std::string& name, const std::string& attributes) {
        if (name == "title") {
            inTitle_ = true;
            titleText_.clear();
            return;
        }
        if (name == "base") {
            const std::string href = attributeValue(attributes, "href");
            if (!href.empty()) baseUrl_ = resolveUrl(pageUrl_, href);
            return;
        }
        if (name == "br") {
            if (pre_) {
                preText("\n");
            } else if (table_) {
                if (inCell_) cell_ += ' ';
            } else {
                flush();
            }
            return;
        }
        if (name == "img") {
            // An image cannot be drawn. Its alt text is the page's own descrip-
            // tion of what is missing, so that is what is kept.
            const std::string alt = attributeValue(attributes, "alt");
            if (!alt.empty()) addWords(decodeEntities(alt));
            return;
        }
        if (name == "hr") {
            flush();
            docs::Block rule;
            rule.kind = docs::Block::Kind::Rule;
            blocks_.push_back(std::move(rule));
            return;
        }
        if (name == "a") {
            if (link_ == 0) {
                const std::string href = attributeValue(attributes, "href");
                if (!href.empty()) {
                    href_ = resolveUrl(baseUrl_.empty() ? pageUrl_ : baseUrl_, href);
                }
            }
            ++link_;
            return;
        }
        if (name == "b" || name == "strong") {
            ++bold_;
            return;
        }
        if ((name == "code" || name == "kbd" || name == "samp") && !pre_) {
            ++code_;
            return;
        }
        if (name == "pre") {
            flush();
            pre_ = true;
            preLine_.clear();
            preLines_.clear();
            return;
        }
        if (name == "table") {
            flush();
            table_ = true;
            inCell_ = false;
            rows_.clear();
            row_.clear();
            cell_.clear();
            return;
        }
        if (name == "tr") {
            row_.clear();
            return;
        }
        if (name == "td" || name == "th") {
            cell_.clear();
            inCell_ = true;
            return;
        }
        if (name == "ul") {
            lists_.push_back(List{false, 1});
            return;
        }
        if (name == "ol") {
            lists_.push_back(List{true, 1});
            return;
        }
        if (name == "li") {
            flush();
            if (!lists_.empty() && lists_.back().ordered) {
                kind_ = Kind::Numbered;
                number_ = lists_.back().next++;
            } else {
                kind_ = Kind::Bullet;
            }
            return;
        }
        const int heading = headingLevel(name);
        if (heading > 0) {
            flush();
            kind_ = Kind::Heading;
            heading_ = heading;
            return;
        }
        if (name == "blockquote") {
            flush();
            kind_ = Kind::Quote;
            return;
        }
        if (startsBlock(name)) flush();
    }

    void endTag(const std::string& name) {
        if (name == "title") {
            inTitle_ = false;
            title_ = collapseSpaces(titleText_);
            return;
        }
        if (name == "a") {
            if (link_ > 0) --link_;
            if (link_ == 0) href_.clear();
            return;
        }
        if (name == "b" || name == "strong") {
            if (bold_ > 0) --bold_;
            return;
        }
        if (name == "code" || name == "kbd" || name == "samp") {
            if (code_ > 0) --code_;
            return;
        }
        if (name == "pre") {
            endPre();
            return;
        }
        if (name == "td" || name == "th") {
            if (inCell_) {
                row_.push_back(collapseSpaces(cell_));
                cell_.clear();
                inCell_ = false;
            }
            return;
        }
        if (name == "tr") {
            if (!row_.empty()) {
                rows_.push_back(row_);
                row_.clear();
            }
            return;
        }
        if (name == "table") {
            endTable();
            return;
        }
        if (name == "ul" || name == "ol") {
            flush();
            if (!lists_.empty()) lists_.pop_back();
            return;
        }
        if (name == "li" || name == "blockquote" || headingLevel(name) > 0 || startsBlock(name)) {
            flush();
        }
    }

    void endPre() {
        if (!preLine_.empty()) preLines_.push_back(preLine_);
        preLine_.clear();
        pre_ = false;
        docs::Block block;
        block.kind = docs::Block::Kind::Code;
        block.code = std::move(preLines_);
        preLines_.clear();
        if (!block.code.empty()) blocks_.push_back(std::move(block));
    }

    void endTable() {
        if (!row_.empty()) {
            rows_.push_back(row_);
            row_.clear();
        }
        table_ = false;
        inCell_ = false;
        if (!rows_.empty()) {
            docs::Block block;
            block.kind = docs::Block::Kind::Table;
            block.rows = std::move(rows_);
            rows_.clear();
            blocks_.push_back(std::move(block));
        }
        // A table is a block of its own, so the text after it is a new paragraph
        // whatever block the table interrupted.
        kind_ = Kind::Paragraph;
        heading_ = 0;
        number_ = 1;
    }

    const std::string& html_;
    std::string lowered_;
    std::string pageUrl_;

    std::vector<docs::Block> blocks_;
    std::vector<docs::Run> words_;
    Kind kind_ = Kind::Paragraph;
    int heading_ = 0;
    int number_ = 1;
    int bold_ = 0;
    int code_ = 0;
    int link_ = 0;
    std::string href_;
    std::vector<List> lists_;

    bool pre_ = false;
    std::string preLine_;
    std::vector<std::string> preLines_;

    bool table_ = false;
    bool inCell_ = false;
    std::vector<std::vector<std::string>> rows_;
    std::vector<std::string> row_;
    std::string cell_;

    bool inTitle_ = false;
    std::string titleText_;
    std::string title_;
    std::string baseUrl_;
};

}  // namespace

std::string resolveUrl(const std::string& base, const std::string& href) {
    const std::string link = trim(href);
    if (link.empty()) return base;

    const std::string lowered = lower(link);
    if (startsWith(lowered, "http://") || startsWith(lowered, "https://")) return link;
    if (startsWith(lowered, "about:") || startsWith(lowered, "mailto:") ||
        startsWith(lowered, "javascript:") || startsWith(lowered, "data:")) {
        return link;
    }

    const std::size_t schemeEnd = base.find("://");
    if (schemeEnd == std::string::npos) return link;
    const std::string scheme = base.substr(0, schemeEnd);
    const std::size_t authorityStart = schemeEnd + 3;
    std::size_t authorityEnd = base.find_first_of("/?#", authorityStart);
    if (authorityEnd == std::string::npos) authorityEnd = base.size();
    const std::string authority = base.substr(authorityStart, authorityEnd - authorityStart);

    if (link.size() >= 2 && link[0] == '/' && link[1] == '/') return scheme + ":" + link;
    if (link[0] == '#') return withoutFragment(base) + link;
    if (link[0] == '?') return withoutQueryAndFragment(base) + link;
    if (link[0] == '/') return scheme + "://" + authority + normalisePath(link);

    std::string basePath = withoutQueryAndFragment(base.substr(authorityEnd));
    const std::size_t lastSlash = basePath.rfind('/');
    const std::string directory =
        lastSlash == std::string::npos ? std::string("/") : basePath.substr(0, lastSlash + 1);
    return scheme + "://" + authority + normalisePath(directory + link);
}

namespace {

// One header's value, found without regard to case. `headers` is the block
// HttpClient hands over: CRLF-separated lines, no trailing blank line, and the
// status line first.
std::string headerValue(const std::string& headers, const std::string& name) {
    std::size_t cursor = 0;
    while (cursor < headers.size()) {
        std::size_t end = headers.find("\r\n", cursor);
        if (end == std::string::npos) end = headers.size();
        const std::string line = headers.substr(cursor, end - cursor);
        const std::size_t colon = line.find(':');
        if (colon != std::string::npos && lower(trim(line.substr(0, colon))) == name) {
            return trim(line.substr(colon + 1));
        }
        cursor = end + 2;
    }
    return std::string();
}

}  // namespace

bool redirectLocation(int status, const std::string& headers, std::string* location) {
    // 300, 304 and 305 are left out on purpose: 304 is a cache answer and the
    // others leave the choice to the reader, which this window does not make.
    if (status != 301 && status != 302 && status != 303 && status != 307 && status != 308) {
        return false;
    }
    const std::string value = headerValue(headers, "location");
    if (value.empty()) return false;
    if (location != nullptr) *location = value;
    return true;
}

std::string normaliseAddress(const std::string& typed) {
    const std::string text = trim(typed);
    if (text.empty()) return "about:start";

    const std::string lowered = lower(text);
    // A scheme is case-insensitive in every URL, and the window tests it as
    // written, so it is folded here rather than at the call site.
    if (startsWith(lowered, "http://")) return "http://" + text.substr(7);
    if (startsWith(lowered, "https://")) return "https://" + text.substr(8);
    if (startsWith(lowered, "about:") || startsWith(lowered, "mailto:")) return text;
    if (startsWith(lowered, "//")) return "http:" + text;
    return "http://" + text;
}

Page parse(const std::string& html, const std::string& pageUrl) {
    Reader reader(html, pageUrl);
    return reader.run();
}

std::string startPage() {
    return
        "<!doctype html><html><head><title>Linear Browser</title>"
        "<style>body{font-family:Segoe UI}</style></head><body>"
        "<h1>Linear Browser</h1>"
        "<p>This is the project's own browser. It reads a page over plain HTTP and draws it as "
        "text: headings, paragraphs, lists, quotes, code, tables and links.</p>"
        "<h2>Where to go</h2>"
        "<ul>"
        "<li><a href=\"http://localhost:3000\">http://localhost:3000</a> &mdash; the website, "
        "when WebApp.exe is running</li>"
        "<li><a href=\"http://localhost:3100\">http://localhost:3100</a> &mdash; LinearHub, the "
        "repository browser</li>"
        "<li><a href=\"http://127.0.0.1:7900\">http://127.0.0.1:7900</a> &mdash; the LinearHosts "
        "control panel, with the link it prints</li>"
        "<li><a href=\"about:start\">about:start</a> &mdash; this page</li>"
        "</ul>"
        "<h2>What it does not do</h2>"
        "<p>There is no TLS stack in this project, so an https:// address cannot be opened. A "
        "page is read, not run: no JavaScript, no style sheets, no images, no cookies and no "
        "forms. A link is the only thing that does anything, which is also why a page cannot ask "
        "this window to send anything anywhere.</p>"
        "<p>Type an address in the bar above and press Enter. Back, Forward, Reload and Home are "
        "beside it; the wheel scrolls the page; Esc closes the window.</p>"
        "</body></html>";
}

std::string errorPage(const std::string& address, const std::string& detail) {
    std::string html =
        "<!doctype html><html><head><title>Cannot open this page</title></head><body>";
    html += "<h1>Cannot open this page</h1>";
    html += "<p>" + escapeHtml(detail) + "</p>";
    if (!address.empty()) html += "<p><code>" + escapeHtml(address) + "</code></p>";
    html += "<p><a href=\"about:start\">about:start</a> &mdash; the start page</p>";
    html += "</body></html>";
    return html;
}

void History::clear() {
    entries_.clear();
    index_ = 0;
}

void History::push(const std::string& url) {
    if (!entries_.empty() && entries_[index_] == url) return;
    if (entries_.empty()) {
        entries_.push_back(url);
        index_ = 0;
        return;
    }
    // Everything ahead of the current entry is what Forward would have shown;
    // a new address replaces it.
    entries_.resize(index_ + 1);
    entries_.push_back(url);
    index_ = entries_.size() - 1;
}

bool History::back(std::string* url) {
    if (index_ == 0 || entries_.empty()) return false;
    --index_;
    if (url != nullptr) *url = entries_[index_];
    return true;
}

bool History::forward(std::string* url) {
    if (entries_.empty() || index_ + 1 >= entries_.size()) return false;
    ++index_;
    if (url != nullptr) *url = entries_[index_];
    return true;
}

bool History::canGoBack() const { return !entries_.empty() && index_ > 0; }

bool History::canGoForward() const {
    return !entries_.empty() && index_ + 1 < entries_.size();
}

const std::string& History::current() const {
    static const std::string empty;
    return entries_.empty() ? empty : entries_[index_];
}

}  // namespace browser
}  // namespace linear
