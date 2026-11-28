// The reader behind LinearBrowser.
//
// A page is text in, blocks and addresses out, so nothing here needs a socket or
// a window: the HTML subset, the link resolution, what the address bar does with
// what was typed, and the history are all checked on a build server.

#include "BrowserPage.h"

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

const linear::docs::Block* blockOf(const std::vector<linear::docs::Block>& blocks,
                                   linear::docs::Block::Kind kind) {
    for (const linear::docs::Block& block : blocks) {
        if (block.kind == kind) return &block;
    }
    return nullptr;
}

int countOf(const std::vector<linear::docs::Block>& blocks, linear::docs::Block::Kind kind) {
    int count = 0;
    for (const linear::docs::Block& block : blocks) {
        if (block.kind == kind) ++count;
    }
    return count;
}

std::string joined(const std::vector<linear::docs::Block>& blocks, std::size_t index) {
    if (index >= blocks.size()) return std::string();
    std::string text;
    for (const linear::docs::Run& word : blocks[index].words) {
        if (!text.empty()) text += ' ';
        text += word.text;
    }
    return text;
}

void readsTitleAndText() {
    const linear::browser::Page page = linear::browser::parse(
        "<!doctype html><html><head><title> Linear &amp; Browser </title></head>"
        "<body><h1>Hello</h1><p>one   two\nthree</p></body></html>",
        "http://localhost:3000/");
    expectEqual(page.title, "Linear & Browser", "the title is read and its entities decoded");
    expect(page.blocks.size() == 2, "a heading and a paragraph");
    if (page.blocks.size() != 2) return;
    expect(page.blocks[0].kind == linear::docs::Block::Kind::Heading, "h1 is a heading");
    expect(page.blocks[0].level == 1, "h1 is level one");
    expectEqual(joined(page.blocks, 1), "one two three", "whitespace collapses in a paragraph");
}

void readsBlockStructure() {
    const linear::browser::Page page = linear::browser::parse(
        "<h2>Lists</h2><ul><li>first</li><li>second</li></ul>"
        "<ol><li>one</li><li>two</li></ol>"
        "<blockquote>quoted words</blockquote>"
        "<pre>line one\nline two\n</pre>"
        "<table><tr><th>Name</th><th>Port</th></tr><tr><td>WebApp</td><td>3000</td></tr></table>"
        "<hr>"
        "<p>after</p>",
        "http://localhost:3000/");
    const std::vector<linear::docs::Block>& blocks = page.blocks;

    expect(countOf(blocks, linear::docs::Block::Kind::Bullet) == 2, "two bullets");
    expect(countOf(blocks, linear::docs::Block::Kind::Numbered) == 2, "two numbered items");
    expect(countOf(blocks, linear::docs::Block::Kind::Quote) == 1, "one quote");
    expect(countOf(blocks, linear::docs::Block::Kind::Rule) == 1, "one rule");

    const linear::docs::Block* numbered = blockOf(blocks, linear::docs::Block::Kind::Numbered);
    expect(numbered != nullptr && numbered->number == 1, "the first numbered item is 1");
    const linear::docs::Block* second = nullptr;
    bool seenFirst = false;
    for (const linear::docs::Block& block : blocks) {
        if (block.kind != linear::docs::Block::Kind::Numbered) continue;
        if (seenFirst) {
            second = &block;
            break;
        }
        seenFirst = true;
    }
    expect(second != nullptr && second->number == 2, "and the second is 2");

    const linear::docs::Block* code = blockOf(blocks, linear::docs::Block::Kind::Code);
    expect(code != nullptr, "pre is a code block");
    if (code != nullptr) {
        expect(code->code.size() == 2, "the code block keeps its two lines");
        if (code->code.size() == 2) {
            expectEqual(code->code[0], "line one", "the first line is kept as written");
            expectEqual(code->code[1], "line two", "the trailing newline is not a blank line");
        }
    }

    const linear::docs::Block* table = blockOf(blocks, linear::docs::Block::Kind::Table);
    expect(table != nullptr, "a table is a table");
    if (table != nullptr) {
        expect(table->rows.size() == 2, "two rows");
        if (table->rows.size() == 2) {
            expect(table->rows[0].size() == 2, "two header cells");
            expectEqual(table->rows[1][0], "WebApp", "a cell keeps its text");
            expectEqual(table->rows[1][1], "3000", "and so does the next one");
        }
    }
}

void readsLinksWithTargets() {
    const linear::browser::Page page = linear::browser::parse(
        "<p><a href=\"/api/games\">games</a> and <a href='https://example.com/x'>out</a></p>",
        "http://localhost:3000/docs/page.html");
    expect(page.blocks.size() == 1, "one paragraph");
    if (page.blocks.size() != 1) return;

    std::string gamesTarget;
    std::string outTarget;
    for (const linear::docs::Run& word : page.blocks[0].words) {
        if (word.text == "games") gamesTarget = word.target;
        if (word.text == "out") outTarget = word.target;
    }
    expectEqual(gamesTarget, "http://localhost:3000/api/games", "a root link resolves to the host");
    expectEqual(outTarget, "https://example.com/x", "an absolute link is unchanged");

    bool linkStyle = false;
    for (const linear::docs::Run& word : page.blocks[0].words) {
        if (word.target.empty()) continue;
        linkStyle = word.style == linear::docs::RunStyle::Link;
    }
    expect(linkStyle, "a link's words are set as links");
}

void resolvesAddresses() {
    const std::string base = "http://localhost:3000/docs/page.html?x=1#top";
    expectEqual(linear::browser::resolveUrl(base, "other.html"),
                "http://localhost:3000/docs/other.html", "a sibling file");
    expectEqual(linear::browser::resolveUrl(base, "../up.html"),
                "http://localhost:3000/up.html", ".. walks out of the folder");
    expectEqual(linear::browser::resolveUrl(base, "./a/./b"),
                "http://localhost:3000/docs/a/b", "'.' segments are dropped");
    expectEqual(linear::browser::resolveUrl(base, "/root"), "http://localhost:3000/root",
                "a root link keeps the host");
    expectEqual(linear::browser::resolveUrl(base, "//cdn.example.com/a.js"),
                "http://cdn.example.com/a.js", "a protocol-relative link borrows the scheme");
    expectEqual(linear::browser::resolveUrl(base, "#section"),
                "http://localhost:3000/docs/page.html?x=1#section",
                "a fragment keeps the query");
    expectEqual(linear::browser::resolveUrl(base, "?q=2"),
                "http://localhost:3000/docs/page.html?q=2", "a query replaces the old one");
    expectEqual(linear::browser::resolveUrl(base, "mailto:someone@example.com"),
                "mailto:someone@example.com", "an opaque address is left alone");
    expectEqual(linear::browser::resolveUrl(base, ""), base, "an empty link is the page itself");
    expectEqual(linear::browser::resolveUrl("http://localhost:3000", "games"),
                "http://localhost:3000/games", "a bare host has an implicit / directory");

    // A page read over TLS keeps https for what it links to, and a
    // protocol-relative link borrows that scheme as well.
    const std::string secureBase = "https://example.com/docs/page.html";
    expectEqual(linear::browser::resolveUrl(secureBase, "other.html"),
                "https://example.com/docs/other.html", "an https page keeps its scheme");
    expectEqual(linear::browser::resolveUrl(secureBase, "//cdn.example.com/x"),
                "https://cdn.example.com/x", "a protocol-relative link borrows https");
}

void skipsCodeAndKeepsAltText() {
    const linear::browser::Page page = linear::browser::parse(
        "<p>keep</p><script>document.write('<p>drop</p>');</script>"
        "<style>p { color: red }</style>"
        "<p><img src=\"mark.png\" alt=\"the Linear mark\"> after</p>",
        "http://localhost:3000/");
    std::string all;
    for (const linear::docs::Block& block : page.blocks) {
        for (const linear::docs::Run& word : block.words) {
            if (!all.empty()) all += ' ';
            all += word.text;
        }
    }
    expect(all.find("drop") == std::string::npos, "script contents are not page text");
    expect(all.find("color") == std::string::npos, "style contents are not page text");
    expect(all.find("the Linear mark") != std::string::npos, "an image's alt text is kept");
}

void readsBaseAndEntities() {
    const linear::browser::Page page = linear::browser::parse(
        "<head><base href=\"http://127.0.0.1:3100/hub/\"></head>"
        "<body><p><a href=\"files\">files</a> &#65;&#x42;&nbsp;&mdash;</p></body>",
        "http://localhost:3000/");
    std::string target;
    for (const linear::docs::Block& block : page.blocks) {
        for (const linear::docs::Run& word : block.words) {
            if (word.text == "files") target = word.target;
        }
    }
    expectEqual(target, "http://127.0.0.1:3100/hub/files", "<base href> is what links resolve to");
    const std::string text = joined(page.blocks, 0);
    expect(text.find("AB") != std::string::npos, "numeric entities become characters");
    expect(text.find('\xE2') != std::string::npos, "named entities become UTF-8");
}

void followsAResponseThatMoves() {
    // What a plain http:// site sends when it wants https instead. Reporting
    // "HTTP 301" as a failure, which this window used to do, told the reader
    // nothing about where the address had gone.
    const std::string moved =
        "HTTP/1.1 301 Moved Permanently\r\nContent-Length: 0\r\n"
        "Location: https://www.youtube.com/\r\nConnection: close";
    std::string location;
    expect(linear::browser::redirectLocation(301, moved, &location),
           "a 301 names where to go");
    expectEqual(location, "https://www.youtube.com/", "the Location value is read");

    expect(linear::browser::redirectLocation(302, "location: /home\r\n", &location),
           "a header name is not case sensitive");
    expectEqual(location, "/home", "a relative Location is handed back as it was sent");
    expectEqual(linear::browser::resolveUrl("http://localhost:3000/a/b", location),
                "http://localhost:3000/home", "and resolves against the page that redirected");

    expect(!linear::browser::redirectLocation(200, "Location: /ignored", &location),
           "a 200 is a page, not a move");
    expectEqual(location, "/home", "a status that is not a redirect leaves the location alone");
    expect(!linear::browser::redirectLocation(304, "Location: /ignored", &location),
           "304 is a cache answer, not a move");
    expect(!linear::browser::redirectLocation(303, "X-Other: 1", &location),
           "a redirect without a Location cannot be followed");
    expect(linear::browser::redirectLocation(307, "Location: http://localhost:3000/x", &location),
           "307 is followed");
    expectEqual(location, "http://localhost:3000/x", "307's location is read");
    expect(linear::browser::redirectLocation(308, "Location: http://localhost:3000/y", &location),
           "308 is followed");
    expectEqual(location, "http://localhost:3000/y", "308's location is read");
}

void normalisesWhatIsTyped() {
    expectEqual(linear::browser::normaliseAddress(""), "about:start", "empty is the start page");
    expectEqual(linear::browser::normaliseAddress("  localhost:3000  "),
                "http://localhost:3000", "a bare host gets http://");
    expectEqual(linear::browser::normaliseAddress("example.com/a"),
                "http://example.com/a", "so does a host with a path");
    expectEqual(linear::browser::normaliseAddress("  HTTPS://Example.com/a  "),
                "https://Example.com/a", "an https address is kept, with its scheme folded");
    expectEqual(linear::browser::normaliseAddress("HTTP://LOCALHOST:3000"),
                "http://LOCALHOST:3000", "the scheme is folded to lower case");
    expectEqual(linear::browser::normaliseAddress("about:start"), "about:start",
                "about: is not rewritten");
    expectEqual(linear::browser::normaliseAddress("//example.com"),
                "http://example.com", "a protocol-relative address takes http");
}

void startPageOffersLinks() {
    const linear::browser::Page page =
        linear::browser::parse(linear::browser::startPage(), "about:start");
    expectEqual(page.title, "Linear Browser", "the start page has a title");
    expect(!page.blocks.empty(), "the start page has content");

    std::vector<std::string> targets;
    for (const linear::docs::Block& block : page.blocks) {
        for (const linear::docs::Run& word : block.words) {
            if (!word.target.empty()) targets.push_back(word.target);
        }
    }
    bool site = false;
    bool hub = false;
    bool self = false;
    for (const std::string& target : targets) {
        if (target == "http://localhost:3000") site = true;
        if (target == "http://localhost:3100") hub = true;
        if (target == "about:start") self = true;
    }
    expect(site, "the start page links the website");
    expect(hub, "the start page links the repository browser");
    expect(self, "the start page can link itself");
}

void errorPageEscapesDetail() {
    const std::string html = linear::browser::errorPage(
        "http://localhost:3000/<bad>", "The server said <script>alert(1)</script>.");
    expect(html.find("<script>") == std::string::npos, "a detail cannot inject a script tag");
    expect(html.find("&lt;script&gt;") != std::string::npos, "it is escaped instead");
    expect(html.find("&lt;bad&gt;") != std::string::npos, "the address is escaped too");

    const linear::browser::Page page = linear::browser::parse(html, "http://localhost:3000/");
    expectEqual(page.title, "Cannot open this page", "the error page says what it is");
}

void historyMovesBackAndForward() {
    linear::browser::History history;
    expect(!history.canGoBack(), "an empty history cannot go back");
    expect(!history.canGoForward(), "or forward");

    history.push("about:start");
    history.push("http://localhost:3000/");
    expect(history.canGoBack(), "two entries can go back");
    expect(!history.canGoForward(), "the newest entry cannot go forward");
    expectEqual(history.current(), "http://localhost:3000/", "the newest entry is current");

    std::string url;
    expect(history.back(&url), "back succeeds");
    expectEqual(url, "about:start", "back goes to the entry before it");
    expect(!history.canGoBack(), "and there is nothing before that");
    expect(history.canGoForward(), "forward is now possible");

    expect(history.forward(&url), "forward succeeds");
    expectEqual(url, "http://localhost:3000/", "forward returns to the newest entry");

    expect(history.back(&url), "back again");
    history.push("http://localhost:3100/");
    expect(!history.canGoForward(), "a new address drops what was ahead");
    expect(history.back(&url), "back from the new address");
    expectEqual(url, "about:start", "and it is where the reader actually was");

    history.push("about:start");
    expect(history.canGoForward(), "opening the current page keeps what was ahead");
    expect(history.forward(&url), "forward still works after that");
    expectEqual(url, "http://localhost:3100/", "and it goes to the entry that was ahead");
}

// A page can say which part of it is the page. Everything around <main> is the
// site's own furniture: menus, news, a footer, and on a real tool directory a
// hidden menu holding seven hundred links.
void readsTheMainContentWhenAPageMarksIt() {
    const linear::browser::Page page = linear::browser::parse(
        "<!doctype html><html><head><title>Tools</title></head><body>"
        "<header><nav><a href=\"/one\">One</a> <a href=\"/two\">Two</a></nav></header>"
        "<main><h1>The tool</h1><p>It works.</p></main>"
        "<footer>Copyright nobody</footer></body></html>",
        "http://localhost:3000/tool");

    expectEqual(page.title, "Tools", "the title still comes from the whole document");
    std::string text;
    for (const linear::docs::Block& block : page.blocks) {
        for (const linear::docs::Run& word : block.words) {
            if (!text.empty()) text += ' ';
            text += word.text;
        }
    }
    expect(text.find("The tool") != std::string::npos, "the main content is read");
    expect(text.find("It works") != std::string::npos, "and so is the text under it");
    expect(text.find("One") == std::string::npos, "the menu outside main is not");
    expect(text.find("Copyright") == std::string::npos, "nor is the footer");

    // A page whose <main> holds nothing is a page to read whole, not a blank one.
    const linear::browser::Page empty = linear::browser::parse(
        "<html><body><header>Menus</header><main></main><footer>After</footer></body></html>",
        "http://localhost:3000/");
    std::string fallback;
    for (const linear::docs::Block& block : empty.blocks) {
        for (const linear::docs::Run& word : block.words) fallback += word.text + ' ';
    }
    expect(fallback.find("After") != std::string::npos,
           "an empty main falls back to the whole page");
}

}  // namespace

int main() {
    readsTitleAndText();
    readsBlockStructure();
    readsLinksWithTargets();
    resolvesAddresses();
    followsAResponseThatMoves();
    skipsCodeAndKeepsAltText();
    readsBaseAndEntities();
    normalisesWhatIsTyped();
    startPageOffersLinks();
    errorPageEscapesDetail();
    readsTheMainContentWhenAPageMarksIt();
    historyMovesBackAndForward();

    if (g_failures == 0) {
        std::printf("BrowserPageTests: all checks passed.\n");
        return 0;
    }
    std::printf("BrowserPageTests: %d check(s) failed.\n", g_failures);
    return 1;
}
