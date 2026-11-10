// The hub's pages are strings, so the escaping, the links and the line numbering
// are checked here instead of in a browser. The escaping checks are the important
// ones: a repository can contain a file called `<script>`, and a page that pasted
// that name in unescaped would run it.

#include "HubViews.h"

#include <cstdio>
#include <string>
#include <vector>

namespace hub = rblx::hub;

namespace {

int failures = 0;

void check(bool ok, const std::string& what) {
    if (!ok) {
        std::printf("  failed: %s\n", what.c_str());
        ++failures;
    }
}

bool has(const std::string& text, const std::string& needle) {
    return text.find(needle) != std::string::npos;
}

std::size_t countOf(const std::string& text, const std::string& needle) {
    std::size_t total = 0;
    std::size_t at = text.find(needle);
    while (at != std::string::npos) {
        ++total;
        at = text.find(needle, at + needle.size());
    }
    return total;
}

hub::RepoSummary sampleRepo() {
    hub::RepoSummary repo;
    repo.name = "demo";
    repo.description = "A demo <repo>";
    repo.branch = "main";
    repo.commits = 2;
    repo.files = 3;
    repo.issues = 1;
    repo.readme = "# Demo\n\nA <b>markup</b> free readme.\n";
    repo.branches = {"main"};
    return repo;
}

void testEscaping() {
    check(hub::escapeHtml("<a href=\"x\">&'</a>") == "&lt;a href=&quot;x&quot;&gt;&amp;&#39;&lt;/a&gt;",
          "the five characters are escaped");
    check(hub::escapeHtml("plain") == "plain", "plain text is untouched");
    check(hub::urlPath("src/a b&c.txt") == "src/a%20b%26c.txt",
          "urlPath keeps slashes and encodes the rest");
    check(hub::urlPath("\"><script>alert(1)</script>") ==
              "%22%3E%3Cscript%3Ealert%281%29%3C/script%3E",
          "a path cannot leave its attribute");
}

void testOverview() {
    const hub::RepoSummary repo = sampleRepo();
    std::vector<hub::TreeRow> rows = {{"readme.md", "readme.md", false, "abc1234deadbeef"},
                                      {"src", "src", true, "def5678"}};
    std::vector<hub::CommitRow> commits = {
        {std::string(40, 'a'), "Tester <t@example.com>", "today", "first commit"}};

    const std::string page = hub::overview(repo, rows, commits);
    check(has(page, "<!DOCTYPE html>"), "a whole document");
    check(has(page, "A demo &lt;repo&gt;"), "the description is escaped");
    check(has(page, "A &lt;b&gt;markup&lt;/b&gt; free readme."), "the readme is escaped");
    check(has(page, "href=\"/tree/src\""), "a folder links to the tree page");
    check(has(page, "href=\"/blob/readme.md\""), "a file links to the blob page");
    check(has(page, "href=\"/commits\""), "the history link");
    check(has(page, "href=\"/issues\""), "the issues link");
    check(has(page, "<span class=\"pill\">main</span>"), "the branch is shown");
    check(!has(page, "<script"), "no script tag is ever written");
    check(!has(page, "onclick"), "no inline handler is ever written");
}

void testTreeAndBlob() {
    const hub::RepoSummary repo = sampleRepo();
    std::vector<hub::TreeRow> rows = {{"main.cpp", "src/main.cpp", false, "abc1234"}};
    const std::string tree = hub::treePage(repo, "src", rows);
    check(has(tree, "href=\"/tree/\">..</a>"), "the parent link goes up");
    check(has(tree, "href=\"/blob/src/main.cpp\""), "the file path is the full path");
    check(has(tree, "<strong>src</strong>"), "the breadcrumb marks the folder being shown");
    check(has(hub::breadcrumbs("src/app/main.cpp"), "/tree/src/app\">app</a>"),
          "a deeper breadcrumb links its folders");

    const std::string blob = hub::blobPage(repo, "src/main.cpp", "int main() {}\nreturn 0;\n", false);
    check(countOf(blob, "<tr>") == 2, "one row per line");
    check(has(blob, ">1<") && has(blob, ">2<"), "both line numbers are written");
    check(has(blob, "int main() {}"), "the line is shown");
    check(has(hub::blobPage(repo, "x", "<b>", false), "&lt;b&gt;"), "content is escaped");
    check(has(hub::blobPage(repo, "x", "abc", true), "Only the beginning"),
          "a truncated file says so");
    check(has(hub::blobPage(repo, "x", std::string(), false), "<pre></pre>"),
          "an empty file still renders a line");
}

void testCommits() {
    const hub::RepoSummary repo = sampleRepo();
    std::vector<hub::CommitRow> commits = {
        {std::string(40, 'a'), "Tester <t@example.com>", "today", "first <commit>"},
        {std::string(40, 'b'), "Tester <t@example.com>", "yesterday", "second"}};

    const std::string list = hub::commitsPage(repo, commits);
    check(countOf(list, "href=\"/commit/") == 2, "one link per commit");
    check(has(list, "first &lt;commit&gt;"), "the summary is escaped");

    const std::string detail =
        hub::commitPage(repo, commits[0], {{'M', "a b.txt"}, {'A', "new.txt"}, {'D', "gone.txt"}});
    check(has(detail, "a%20b.txt"), "a changed path is encoded");
    check(has(detail, "badge added"), "an added file is marked");
    check(has(detail, "badge deleted"), "a deleted file is marked");
    check(has(detail, "badge modified"), "a modified file is marked");
    check(has(hub::commitPage(repo, commits[0], {}), "No files differ"),
          "a commit with no parent says so");
}

void testIssues() {
    const hub::RepoSummary repo = sampleRepo();
    std::vector<hub::IssueRow> issues = {{1, "<b>title</b>", "body & more", "guest", "today", true}};

    const std::string list = hub::issuesPage(repo, issues, "");
    check(has(list, "&lt;b&gt;title&lt;/b&gt;"), "the issue title is escaped");
    check(has(list, "body &amp; more"), "the issue body is escaped");
    check(has(list, "#1"), "the issue number is shown");
    check(has(list, "badge open"), "the issue is marked open");
    check(has(hub::issuesPage(repo, {}, ""), "No issues yet"), "an empty list says so");
    check(has(hub::issuesPage(repo, {}, "Saved."), "Saved."), "a notice is shown");

    const std::string form = hub::newIssuePage(repo, "");
    check(has(form, "method=\"post\""), "the form posts");
    check(has(form, "action=\"/issues\""), "to the issues route");
    check(has(form, "name=\"title\""), "with a title field");
    check(has(form, "name=\"body\""), "with a body field");
    check(has(hub::newIssuePage(repo, "An issue needs a title."), "class=\"notice error\""),
          "the error is shown");

    check(has(hub::missingPage(repo, "no-such-page"), "Not found"), "the missing page explains itself");
}

}  // namespace

int main() {
    testEscaping();
    testOverview();
    testTreeAndBlob();
    testCommits();
    testIssues();

    if (failures == 0) {
        std::printf("HubViewsTests: all checks passed\n");
        return 0;
    }
    std::printf("HubViewsTests: %d checks failed\n", failures);
    return 1;
}
