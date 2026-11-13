#include "HubViews.h"

#include <algorithm>
#include <cstdio>

namespace rblx {
namespace hub {
namespace {

std::string shortId(const std::string& id) {
    return id.size() > 7 ? id.substr(0, 7) : id;
}

std::string plural(std::size_t count, const char* one, const char* many) {
    return std::to_string(count) + " " + (count == 1 ? one : many);
}

std::string sidebar(const RepoSummary& repo) {
    std::string out = "<aside class=\"side\">\n<h2>About</h2>\n";
    out += "<p class=\"lede\">" +
           escapeHtml(repo.description.empty() ? "No description yet." : repo.description) +
           "</p>\n<ul class=\"facts\">\n";
    out += "<li><strong>" + plural(repo.commits, "commit", "commits") + "</strong></li>\n";
    out += "<li><strong>" + plural(repo.files, "file", "files") + "</strong> at HEAD</li>\n";
    out += "<li><strong>" + plural(repo.issues, "issue", "issues") + "</strong></li>\n";
    out += "</ul>\n";
    if (!repo.branches.empty()) {
        out += "<h2>Branches</h2>\n<ul class=\"branches\">\n";
        for (const std::string& branch : repo.branches) {
            out += "<li>" + escapeHtml(branch);
            if (branch == repo.branch) {
                out += " <span class=\"pill\">HEAD</span>";
            }
            out += "</li>\n";
        }
        out += "</ul>\n";
    }
    out += "</aside>\n";
    return out;
}

std::string fileRows(const RepoSummary& repo, const std::string& at, const std::vector<TreeRow>& rows) {
    std::string out = "<table class=\"files\">\n<thead><tr><th scope=\"col\">Name</th><th scope=\"col\">Kind</th>"
                      "<th scope=\"col\">Object</th></tr></thead>\n<tbody>\n";
    if (!at.empty()) {
        std::string parent;
        const std::size_t slash = at.rfind('/');
        parent = slash == std::string::npos ? std::string() : at.substr(0, slash);
        out += "<tr><td><a class=\"dir\" href=\"/tree/" + urlPath(parent) + "\">..</a></td>"
               "<td>parent</td><td></td></tr>\n";
    }
    for (const TreeRow& row : rows) {
        const std::string href = row.directory ? "/tree/" + urlPath(row.path) : "/blob/" + urlPath(row.path);
        out += "<tr><td><a class=\"" + std::string(row.directory ? "dir" : "file") + "\" href=\"" + href +
               "\">" + escapeHtml(row.name) + "</a></td>";
        out += "<td>" + std::string(row.directory ? "folder" : "file") + "</td>";
        out += "<td class=\"mono\">" + escapeHtml(shortId(row.id)) + "</td></tr>\n";
    }
    if (rows.empty()) {
        out += "<tr><td colspan=\"3\" class=\"empty\">Nothing here yet.</td></tr>\n";
    }
    out += "</tbody>\n</table>\n";
    (void)repo;
    return out;
}

std::string commitTable(const std::vector<CommitRow>& commits, bool link) {
    std::string out = "<table class=\"commits\">\n<thead><tr><th scope=\"col\">Commit</th><th scope=\"col\">Summary</th>"
                      "<th scope=\"col\">Author</th><th scope=\"col\">When</th></tr></thead>\n<tbody>\n";
    for (const CommitRow& commit : commits) {
        const std::string id = link ? "<a class=\"mono\" href=\"/commit/" + urlPath(commit.id) + "\">" +
                                          escapeHtml(shortId(commit.id)) + "</a>"
                                    : "<span class=\"mono\">" + escapeHtml(shortId(commit.id)) + "</span>";
        out += "<tr><td>" + id + "</td><td>" + escapeHtml(commit.summary) + "</td><td>" +
               escapeHtml(commit.author) + "</td><td class=\"when\">" + escapeHtml(commit.when) +
               "</td></tr>\n";
    }
    if (commits.empty()) {
        out += "<tr><td colspan=\"4\" class=\"empty\">No commits yet.</td></tr>\n";
    }
    out += "</tbody>\n</table>\n";
    return out;
}

std::string codeTable(const std::string& content) {
    std::string out = "<table class=\"code\">\n<tbody>\n";
    int number = 1;
    std::size_t start = 0;
    while (start < content.size()) {
        const std::size_t end = content.find('\n', start);
        const std::string line =
            content.substr(start, end == std::string::npos ? std::string::npos : end - start);
        char label[24];
        std::snprintf(label, sizeof(label), "%d", number++);
        out += "<tr><td class=\"ln\">";
        out += label;
        out += "</td><td class=\"line\"><pre>";
        out += escapeHtml(line);
        out += "</pre></td></tr>\n";
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    if (content.empty()) {
        out += "<tr><td class=\"ln\">1</td><td class=\"line\"><pre></pre></td></tr>\n";
    }
    out += "</tbody>\n</table>\n";
    return out;
}

}  // namespace

std::string escapeHtml(const std::string& text) {
    std::string out;
    out.reserve(text.size() + 16);
    for (const char character : text) {
        switch (character) {
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
            case '\'':
                out += "&#39;";
                break;
            default:
                out.push_back(character);
        }
    }
    return out;
}

std::string urlPath(const std::string& path) {
    static const char* digits = "0123456789ABCDEF";
    std::string out;
    out.reserve(path.size() + 8);
    for (const char character : path) {
        const unsigned char byte = static_cast<unsigned char>(character);
        const bool plain = (byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z') ||
                           (byte >= '0' && byte <= '9') || byte == '-' || byte == '_' ||
                           byte == '.' || byte == '~' || byte == '/';
        if (plain) {
            out.push_back(character);
        } else {
            out.push_back('%');
            out.push_back(digits[byte >> 4]);
            out.push_back(digits[byte & 0x0f]);
        }
    }
    return out;
}

// RBLXHub serves these pages at "/" and WebApp under "/hub", so an absolute link
// has the mount path inserted once, where the page is wrapped. Everything that
// matches is markup written by this file: repository text is escaped into body
// content and never reaches an href or an action.
std::string mountPath;

void prefixAbsoluteUrls(std::string* html) {
    if (mountPath.empty()) {
        return;
    }
    for (const std::string& attribute : { std::string("href=\"/"), std::string("action=\"/") }) {
        std::size_t at = 0;
        while ((at = html->find(attribute, at)) != std::string::npos) {
            html->insert(at + attribute.size() - 1, mountPath);
            at += attribute.size() + mountPath.size();
        }
    }
}

std::string page(const std::string& title, const std::string& body) {
    std::string out;
    out += "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n";
    out += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
    out += "<title>" + escapeHtml(title) + " - RBLXHub</title>\n";
    out += "<link rel=\"stylesheet\" href=\"/hub.css\">\n</head>\n<body>\n";
    out += "<a class=\"skip-link\" href=\"#main\">Skip to content</a>\n";
    out += "<header class=\"top\"><a class=\"brand\" href=\"/\">RBLX<span>Hub</span></a>";
    out += "<span class=\"tagline\">git, served from this machine</span></header>\n";
    out += "<main id=\"main\" tabindex=\"-1\">\n";
    out += body;
    out += "</main>\n<footer class=\"foot\">Served by RBLXHub. Every page is rendered from the "
           "repository; nothing is stored anywhere else.</footer>\n";
    out += "</body>\n</html>\n";
    prefixAbsoluteUrls(&out);
    return out;
}

void setMountPath(const std::string& path) {
    if (path.empty() || path == "/") {
        mountPath.clear();
        return;
    }
    mountPath = path.front() == '/' ? path : "/" + path;
    while (!mountPath.empty() && mountPath.back() == '/') {
        mountPath.pop_back();
    }
}

std::string breadcrumbs(const std::string& path) {
    std::string out = "<a href=\"/\">tree</a>";
    std::string accumulated;
    std::size_t start = 0;
    while (start <= path.size() && !path.empty()) {
        const std::size_t slash = path.find('/', start);
        const std::string part =
            path.substr(start, slash == std::string::npos ? std::string::npos : slash - start);
        if (!accumulated.empty()) {
            accumulated += '/';
        }
        accumulated += part;
        const bool last = slash == std::string::npos;
        if (last) {
            out += " / <strong>" + escapeHtml(part) + "</strong>";
            break;
        }
        out += " / <a href=\"/tree/" + urlPath(accumulated) + "\">" + escapeHtml(part) + "</a>";
        start = slash + 1;
    }
    return out;
}

std::string overview(const RepoSummary& repo, const std::vector<TreeRow>& rows,
                     const std::vector<CommitRow>& commits) {
    std::string body = "<div class=\"repo-head\">\n<h1>" + escapeHtml(repo.name) + "</h1>\n";
    body += "<span class=\"pill\">" + escapeHtml(repo.branch) + "</span>\n";
    body += "<a class=\"button\" href=\"/commits\">History</a>\n";
    body += "<a class=\"button\" href=\"/issues\">Issues <span class=\"count\">" +
            std::to_string(repo.issues) + "</span></a>\n";
    body += "</div>\n";
    body += "<div class=\"cols\">\n<div class=\"main-col\">\n";
    body += "<section class=\"card\">\n<h2>Files</h2>\n" + fileRows(repo, std::string(), rows) + "</section>\n";
    if (!repo.readme.empty()) {
        body += "<section class=\"card readme\">\n<h2>README</h2>\n<pre>" + escapeHtml(repo.readme) +
                "</pre>\n</section>\n";
    }
    body += "<section class=\"card\">\n<h2>Recent commits</h2>\n" + commitTable(commits, true) + "</section>\n";
    body += "</div>\n" + sidebar(repo) + "</div>\n";
    return page(repo.name, body);
}

std::string treePage(const RepoSummary& repo, const std::string& path, const std::vector<TreeRow>& rows) {
    std::string body = "<div class=\"repo-head\">\n<h1>" + escapeHtml(repo.name) + "</h1>\n";
    body += "<span class=\"pill\">" + escapeHtml(repo.branch) + "</span>\n";
    body += "<a class=\"button\" href=\"/commits\">History</a>\n";
    body += "</div>\n<div class=\"cols\">\n<div class=\"main-col\">\n";
    body += "<section class=\"card\"><h2 class=\"crumbs\">" + breadcrumbs(path) + "</h2>\n";
    body += fileRows(repo, path, rows);
    body += "</section>\n</div>\n" + sidebar(repo) + "</div>\n";
    return page(path.empty() ? repo.name : path, body);
}

std::string blobPage(const RepoSummary& repo, const std::string& path, const std::string& content,
                     bool truncated) {
    std::string body = "<div class=\"repo-head\">\n<h1>" + escapeHtml(repo.name) + "</h1>\n";
    body += "<span class=\"pill\">" + escapeHtml(repo.branch) + "</span>\n";
    body += "<a class=\"button\" href=\"/commits\">History</a>\n";
    body += "</div>\n<section class=\"card\">\n<h2 class=\"crumbs\">" + breadcrumbs(path) + "</h2>\n";
    if (truncated) {
        body += "<p class=\"notice\">Only the beginning of this file is shown.</p>\n";
    }
    body += codeTable(content);
    body += "</section>\n";
    return page(path, body);
}

std::string commitsPage(const RepoSummary& repo, const std::vector<CommitRow>& commits) {
    std::string body = "<div class=\"repo-head\">\n<h1>" + escapeHtml(repo.name) + "</h1>\n";
    body += "<span class=\"pill\">" + escapeHtml(repo.branch) + "</span>\n";
    body += "<a class=\"button\" href=\"/\">Files</a>\n";
    body += "</div>\n<section class=\"card\">\n<h2>History</h2>\n" + commitTable(commits, true) +
            "</section>\n";
    return page("History", body);
}

std::string commitPage(const RepoSummary& repo, const CommitRow& commit,
                       const std::vector<ChangedRow>& changes) {
    std::string body = "<div class=\"repo-head\">\n<h1>" + escapeHtml(repo.name) + "</h1>\n";
    body += "<a class=\"button\" href=\"/commits\">History</a>\n";
    body += "<a class=\"button\" href=\"/\">Files</a>\n</div>\n";
    body += "<section class=\"card commit-detail\">\n";
    body += "<h2>" + escapeHtml(commit.summary) + "</h2>\n";
    body += "<p class=\"byline\">" + escapeHtml(commit.author) + " committed " + escapeHtml(commit.when) +
            "</p>\n";
    body += "<p class=\"mono id\">" + escapeHtml(commit.id) + "</p>\n";
    body += "<h3>Changed files</h3>\n<ul class=\"changes\">\n";
    for (const ChangedRow& change : changes) {
        const char* state = change.state == 'A' ? "added" : (change.state == 'D' ? "deleted" : "modified");
        body += "<li><span class=\"badge " + std::string(state) + "\">" + state + "</span> <a href=\"/blob/" +
                urlPath(change.path) + "\">" + escapeHtml(change.path) + "</a></li>\n";
    }
    if (changes.empty()) {
        body += "<li class=\"empty\">No files differ from the parent commit.</li>\n";
    }
    body += "</ul>\n</section>\n";
    return page(commit.summary, body);
}

std::string issuesPage(const RepoSummary& repo, const std::vector<IssueRow>& issues,
                       const std::string& notice) {
    std::string body = "<div class=\"repo-head\">\n<h1>" + escapeHtml(repo.name) + "</h1>\n";
    body += "<a class=\"button primary\" href=\"/issues/new\">New issue</a>\n";
    body += "<a class=\"button\" href=\"/\">Files</a>\n</div>\n";
    if (!notice.empty()) {
        body += "<p class=\"notice\">" + escapeHtml(notice) + "</p>\n";
    }
    body += "<section class=\"card\">\n<h2>Issues</h2>\n<ul class=\"issues\">\n";
    for (const IssueRow& issue : issues) {
        body += "<li><span class=\"badge open\">open</span> <strong>#" + std::to_string(issue.number) +
                "</strong> " + escapeHtml(issue.title) + "<span class=\"byline\"> opened by " +
                escapeHtml(issue.author) + " on " + escapeHtml(issue.when) + "</span>";
        if (!issue.body.empty()) {
            body += "<div class=\"issue-body\">" + escapeHtml(issue.body) + "</div>";
        }
        body += "</li>\n";
    }
    if (issues.empty()) {
        body += "<li class=\"empty\">No issues yet.</li>\n";
    }
    body += "</ul>\n</section>\n";
    return page("Issues", body);
}

std::string newIssuePage(const RepoSummary& repo, const std::string& error) {
    std::string body = "<div class=\"repo-head\">\n<h1>" + escapeHtml(repo.name) + "</h1>\n";
    body += "<a class=\"button\" href=\"/issues\">All issues</a>\n</div>\n";
    body += "<section class=\"card\">\n<h2>New issue</h2>\n";
    if (!error.empty()) {
        body += "<p class=\"notice error\">" + escapeHtml(error) + "</p>\n";
    }
    body += "<form method=\"post\" action=\"/issues\" class=\"form\">\n";
    body += "<label>Title<input name=\"title\" maxlength=\"200\" required></label>\n";
    body += "<label>Details<textarea name=\"body\" rows=\"8\" maxlength=\"4000\"></textarea></label>\n";
    body += "<label>Your name<input name=\"author\" maxlength=\"60\" value=\"guest\"></label>\n";
    body += "<button class=\"button primary\" type=\"submit\">Open issue</button>\n";
    body += "</form>\n</section>\n";
    return page("New issue", body);
}

std::string missingPage(const RepoSummary& repo, const std::string& what) {
    std::string body = "<div class=\"repo-head\">\n<h1>" + escapeHtml(repo.name) + "</h1>\n";
    body += "<a class=\"button\" href=\"/\">Files</a>\n</div>\n";
    body += "<section class=\"card\">\n<h2>Not found</h2>\n<p>" + escapeHtml(what) + "</p>\n";
    body += "<p>Go back to the <a href=\"/\">repository</a>.</p>\n</section>\n";
    return page("Not found", body);
}

}  // namespace hub
}  // namespace rblx
