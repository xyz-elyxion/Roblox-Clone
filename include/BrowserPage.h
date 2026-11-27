#pragma once

// The document model behind LinearBrowser, the project's own browser window.
//
// It reads the HTML an http:// server sends and turns it into the block model
// LinearDocs already lays out - headings, paragraphs, lists, quotes, code, tables
// and rules - with the one thing a reader needs that documentation does not: a
// link keeps the address it points at, so the window can turn it into a button.
// Wrapping and drawing are shared with LinearDocs instead of copied.
//
// This is a reader, not a rendering engine: it runs no script, applies no style
// sheet, draws no image and submits no form. It fetches through
// include/HttpClient.h, which speaks plain http:// because this project has no
// TLS stack, so an https:// address is answered with a page that says so.

#include <cstddef>
#include <string>
#include <vector>

#include "DocsLayout.h"

namespace linear {
namespace browser {

// One fetched page: what to call it, and the blocks to lay out.
struct Page {
    std::string title;
    std::string url;  // the address it was read from
    std::vector<docs::Block> blocks;
};

// Turns a response body into blocks. `pageUrl` is what relative links are
// resolved against; a <base href> in the document wins over it.
Page parse(const std::string& html, const std::string& pageUrl);

// Resolves `href` against `base`: absolute, root-relative, query-only and
// fragment-only links, `.` and `..` segments, and `//host` (which borrows the
// base's scheme). An address that is already absolute is returned unchanged.
std::string resolveUrl(const std::string& base, const std::string& href);

// What the address bar does with what was typed: it adds http:// when no scheme
// is given, leaves about: and mailto: alone, and trims spaces. Empty becomes
// "about:start".
std::string normaliseAddress(const std::string& typed);

// The page shown at startup and by Home. It is HTML in the subset this reader
// understands, so it goes through the same path as a fetched one.
std::string startPage();

// A page saying why an address could not be opened. `detail` is escaped, so a
// server's error text cannot inject markup.
std::string errorPage(const std::string& address, const std::string& detail);

// Back and forward over the addresses this window has opened. A new address
// drops the entries ahead of the current one, as a browser's history does, and
// opening the page that is already current adds nothing.
class History {
public:
    void clear();
    void push(const std::string& url);
    bool back(std::string* url);
    bool forward(std::string* url);
    bool canGoBack() const;
    bool canGoForward() const;
    bool empty() const { return entries_.empty(); }
    const std::string& current() const;

private:
    std::vector<std::string> entries_;
    std::size_t index_ = 0;
};

}  // namespace browser
}  // namespace linear
