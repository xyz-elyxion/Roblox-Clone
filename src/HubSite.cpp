// The repository browser both hosts run: RBLXHub serves these pages at the root of
// its own loopback server, and WebApp serves the same ones under /hub. Nothing in
// this file touches a socket, so the routing and the page contents can be checked
// without starting a server or a browser.

#include "HubSite.h"

#include "WebJson.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;
namespace git = rblx::git;
namespace web = rblx::web;

namespace rblx {
namespace hub {
namespace {

const std::uint32_t kDirectoryMode = 0040000;
const std::size_t kMaxBlobBytes = 512 * 1024;

std::string readText(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return std::string();
    }
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

bool writeText(const std::string& path, const std::string& text, std::string* error) {
    std::error_code ec;
    fs::create_directories(fs::path(path).parent_path(), ec);
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        *error = "cannot write " + path;
        return false;
    }
    out.write(text.data(), static_cast<std::streamsize>(text.size()));
    return static_cast<bool>(out);
}

std::string urlDecode(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '+') {
            out.push_back(' ');
        } else if (text[i] == '%' && i + 2 < text.size()) {
            const std::string hex = text.substr(i + 1, 2);
            const std::size_t used = hex.find_first_not_of("0123456789abcdefABCDEF");
            if (used == std::string::npos) {
                out.push_back(static_cast<char>(std::stoul(hex, nullptr, 16)));
                i += 2;
            } else {
                out.push_back(text[i]);
            }
        } else {
            out.push_back(text[i]);
        }
    }
    return out;
}

std::map<std::string, std::string> parseForm(const std::string& body) {
    std::map<std::string, std::string> fields;
    std::size_t start = 0;
    while (start < body.size()) {
        const std::size_t end = body.find('&', start);
        const std::string pair =
            body.substr(start, end == std::string::npos ? std::string::npos : end - start);
        const std::size_t equals = pair.find('=');
        if (equals != std::string::npos) {
            fields[urlDecode(pair.substr(0, equals))] = urlDecode(pair.substr(equals + 1));
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return fields;
}

// The line under the title on the overview: the README's first heading, or its
// first line of prose when it has none.
std::string firstHeading(const std::string& readme) {
    std::size_t start = 0;
    while (start < readme.size()) {
        const std::size_t end = readme.find('\n', start);
        std::string line =
            readme.substr(start, end == std::string::npos ? std::string::npos : end - start);
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) {
            line.pop_back();
        }
        if (line.rfind("# ", 0) == 0) {
            return line.substr(2);
        }
        if (!line.empty() && line[0] != '#' && line.find_first_not_of("=-") != std::string::npos) {
            return line.size() > 160 ? line.substr(0, 160) : line;
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return std::string();
}

bool treeAt(const git::Repo& repo, const std::string& treeId, const std::string& path,
            std::map<std::string, git::TreeEntry>* entries, std::string* error) {
    std::string cursor = treeId;
    std::string rest = path;
    while (!rest.empty()) {
        const std::size_t slash = rest.find('/');
        const std::string part = rest.substr(0, slash);
        std::map<std::string, git::TreeEntry> level;
        if (!repo.treeEntries(cursor, &level, error)) {
            return false;
        }
        const auto found = level.find(part);
        if (found == level.end() || found->second.mode != kDirectoryMode) {
            *error = path + " is not a folder in this repository";
            return false;
        }
        cursor = found->second.id;
        if (slash == std::string::npos) {
            rest.clear();
        } else {
            rest = rest.substr(slash + 1);
        }
    }
    return repo.treeEntries(cursor, entries, error);
}

bool rowsAt(const git::Repo& repo, const std::string& path, std::vector<TreeRow>* rows,
            std::string* error) {
    rows->clear();
    const std::string head = repo.head();
    if (head.empty()) {
        return true;
    }
    git::Commit commit;
    if (!repo.readCommit(head, &commit, error)) {
        return false;
    }
    std::map<std::string, git::TreeEntry> entries;
    if (!treeAt(repo, commit.tree, path, &entries, error)) {
        return false;
    }
    for (const auto& item : entries) {
        TreeRow row;
        row.name = item.first;
        row.path = path.empty() ? item.first : path + "/" + item.first;
        row.directory = item.second.mode == kDirectoryMode;
        row.id = item.second.id;
        rows->push_back(row);
    }
    std::sort(rows->begin(), rows->end(), [](const TreeRow& left, const TreeRow& right) {
        if (left.directory != right.directory) {
            return left.directory;
        }
        return left.name < right.name;
    });
    return true;
}

// Stamps are stored as unix seconds; a history list is read for how long ago
// something happened, so that is what the page shows.
std::string agoText(const std::string& stamp) {
    const std::string seconds = stamp.substr(0, stamp.find(' '));
    if (seconds.empty() || seconds.find_first_not_of("0123456789") != std::string::npos) {
        return stamp;
    }
    const long long moment = std::stoll(seconds);
    const long long days = (static_cast<long long>(std::time(nullptr)) - moment) / 86400;
    if (days <= 0) {
        return "today";
    }
    if (days == 1) {
        return "yesterday";
    }
    return std::to_string(days) + " days ago";
}

// Issues keep the epoch in their file and are shown with the same wording as a
// commit, without writing that wording back to disk.
std::vector<IssueRow> displayIssues(const std::vector<IssueRow>& stored) {
    std::vector<IssueRow> out = stored;
    for (IssueRow& issue : out) {
        issue.when = agoText(issue.when);
    }
    return out;
}

std::vector<CommitRow> recentCommits(git::Repo& repo, std::size_t limit) {
    std::vector<CommitRow> rows;
    std::vector<git::Commit> commits;
    std::string error;
    if (!repo.log(&commits, limit, &error)) {
        return rows;
    }
    for (const git::Commit& commit : commits) {
        CommitRow row;
        row.id = commit.id;
        row.author = commit.author;
        row.when = agoText(commit.authorWhen);
        const std::size_t end = commit.message.find('\n');
        row.summary = commit.message.substr(0, end == std::string::npos ? std::string::npos : end);
        rows.push_back(row);
    }
    return rows;
}

// Issues: one JSON array in the git directory, written after every post. It is
// deliberately not part of the repository - an issue is hub state, not something a
// pull request would carry.
std::vector<IssueRow> loadIssues(const std::string& path) {
    std::vector<IssueRow> issues;
    const std::string text = readText(path);
    if (text.empty()) {
        return issues;
    }
    std::string error;
    const web::Json document = web::Json::parse(text, &error);
    if (!document.isArray()) {
        return issues;
    }
    for (const web::Json& item : document.elements()) {
        IssueRow issue;
        issue.number = static_cast<int>(item.member("number").asInteger());
        issue.title = item.member("title").asString();
        issue.body = item.member("body").asString();
        issue.author = item.member("author").asString();
        issue.when = item.member("when").asString();
        issue.open = item.member("open").asBool(true);
        issues.push_back(issue);
    }
    return issues;
}

bool saveIssues(const std::string& path, const std::vector<IssueRow>& issues, std::string* error) {
    web::Json array = web::Json::makeArray();
    for (const IssueRow& issue : issues) {
        web::Json item = web::Json::makeObject();
        item.set("number", issue.number);
        item.set("title", issue.title);
        item.set("body", issue.body);
        item.set("author", issue.author);
        item.set("when", issue.when);
        item.set("open", issue.open);
        array.push(item);
    }
    return writeText(path, array.dump(), error);
}

std::string stampNow() {
    const std::time_t now = std::time(nullptr);
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%lld", static_cast<long long>(now));
    return std::string(buffer);
}

void writeHtml(HubPage* page, int status, const std::string& html) {
    page->status = status;
    page->contentType = "text/html; charset=utf-8";
    page->body = html;
}

}  // namespace

bool HubSite::open(const std::string& repoPath, const std::string& webRoot,
                   const std::string& mountPath, std::string* error) {
    if (!repo_.open(repoPath, error)) {
        return false;
    }
    name_ = fs::path(repo_.workTree()).filename().string();
    if (name_.empty()) {
        name_ = "repository";
    }
    root_ = repo_.workTree();
    issuesPath_ = repo_.gitDir() + "/hub-issues.json";
    // Read once: the stylesheet ships with the pages and does not change while the
    // program runs, and a missing one must not turn every page into a 404.
    if (!webRoot.empty()) {
        css_ = readText(webRoot + "/hub/hub.css");
    }
    setMountPath(mountPath);
    ready_ = true;
    return true;
}

RepoSummary HubSite::summary(const std::vector<IssueRow>& issues) {
    RepoSummary out;
    out.name = name_;
    out.branch = repo_.branch().empty() ? std::string("main") : repo_.branch();
    out.head = repo_.head();
    out.issues = issues.size();

    std::string error;
    std::vector<git::Commit> commits;
    if (repo_.log(&commits, 100000, &error)) {
        out.commits = commits.size();
    }
    std::map<std::string, git::TreeEntry> files;
    if (repo_.filesAtHead(&files, &error)) {
        out.files = files.size();
    }
    repo_.branches(&out.branches, &error);

    std::string readme;
    if (!out.head.empty() && repo_.fileAt(out.head, "README.md", &readme, &error)) {
        out.readme = readme;
    } else {
        out.readme = readText((fs::path(repo_.workTree()) / "README.md").string());
    }
    if (out.readme.size() > 20000) {
        out.readme = out.readme.substr(0, 20000) + "\n...";
    }
    out.description = firstHeading(out.readme);
    return out;
}

void HubSite::route(const std::string& method, const std::string& path, const std::string& body,
                    HubPage* page) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string error;
    std::vector<IssueRow> issues = loadIssues(issuesPath_);

    if (path == "/hub.css") {
        if (css_.empty()) {
            page->status = 404;
            page->contentType = "text/plain; charset=utf-8";
            page->body = "hub/hub.css is not beside the program";
            return;
        }
        page->status = 200;
        page->contentType = "text/css; charset=utf-8";
        page->body = css_;
        return;
    }

    if (path == "/" || path.empty()) {
        std::vector<TreeRow> rows;
        std::string failure;
        const bool ok = rowsAt(repo_, std::string(), &rows, &failure);
        const RepoSummary built = summary(issues);
        writeHtml(page, ok ? 200 : 404,
                  ok ? overview(built, rows, recentCommits(repo_, 8)) : missingPage(built, failure));
        return;
    }

    if (path.rfind("/tree/", 0) == 0 || path == "/tree") {
        const std::string folder = urlDecode(path.size() > 6 ? path.substr(6) : std::string());
        std::vector<TreeRow> rows;
        std::string failure;
        const RepoSummary built = summary(issues);
        if (!rowsAt(repo_, folder, &rows, &failure)) {
            writeHtml(page, 404, missingPage(built, failure));
            return;
        }
        writeHtml(page, 200, treePage(built, folder, rows));
        return;
    }

    if (path.rfind("/blob/", 0) == 0) {
        const std::string file = urlDecode(path.substr(6));
        const RepoSummary built = summary(issues);
        std::string contents;
        if (built.head.empty() || !repo_.fileAt(built.head, file, &contents, &error)) {
            writeHtml(page, 404, missingPage(built, file + " is not in HEAD"));
            return;
        }
        const bool truncated = contents.size() > kMaxBlobBytes;
        if (truncated) {
            contents.resize(kMaxBlobBytes);
        }
        writeHtml(page, 200, blobPage(built, file, contents, truncated));
        return;
    }

    if (path == "/commits") {
        const RepoSummary built = summary(issues);
        writeHtml(page, 200, commitsPage(built, recentCommits(repo_, 200)));
        return;
    }

    if (path.rfind("/commit/", 0) == 0) {
        const std::string revision = urlDecode(path.substr(8));
        const RepoSummary built = summary(issues);
        std::string commitId;
        git::Commit commit;
        if (!repo_.resolve(revision, &commitId, &error) ||
            !repo_.readCommit(commitId, &commit, &error)) {
            writeHtml(page, 404, missingPage(built, "No commit " + revision));
            return;
        }
        CommitRow row;
        row.id = commit.id;
        row.author = commit.author;
        row.when = agoText(commit.authorWhen);
        const std::size_t end = commit.message.find('\n');
        row.summary = commit.message.substr(0, end == std::string::npos ? std::string::npos : end);

        std::vector<ChangedRow> changes;
        std::vector<git::StatusEntry> rows;
        const std::string parent = commit.parents.empty() ? std::string() : commit.parents.front();
        if (repo_.changedPaths(parent, commit.id, &rows, &error)) {
            for (const git::StatusEntry& changed : rows) {
                ChangedRow entry;
                entry.state = changed.staged == ' ' ? 'M' : changed.staged;
                entry.path = changed.path;
                changes.push_back(entry);
            }
        }
        writeHtml(page, 200, commitPage(built, row, changes));
        return;
    }

    if (path == "/issues" && method == "POST") {
        const std::map<std::string, std::string> fields = parseForm(body);
        IssueRow issue;
        const auto title = fields.find("title");
        issue.title = title == fields.end() ? std::string() : title->second;
        const auto issueBody = fields.find("body");
        issue.body = issueBody == fields.end() ? std::string() : issueBody->second;
        const auto author = fields.find("author");
        issue.author = author == fields.end() || author->second.empty() ? "guest" : author->second;

        const RepoSummary built = summary(issues);
        if (issue.title.empty()) {
            writeHtml(page, 400, newIssuePage(built, "An issue needs a title."));
            return;
        }
        if (issue.title.size() > 200) {
            issue.title.resize(200);
        }
        if (issue.body.size() > 4000) {
            issue.body.resize(4000);
        }
        if (issue.author.size() > 60) {
            issue.author.resize(60);
        }
        int highest = 0;
        for (const IssueRow& existing : issues) {
            highest = std::max(highest, existing.number);
        }
        issue.number = highest + 1;
        issue.when = stampNow();
        issue.open = true;
        issues.push_back(issue);
        if (!saveIssues(issuesPath_, issues, &error)) {
            writeHtml(page, 500, newIssuePage(built, error));
            return;
        }
        // The form is a plain HTML post, so the answer is a redirect back to the
        // list rather than a page a refresh would post again.
        page->status = 303;
        page->contentType = "text/plain; charset=utf-8";
        page->body.clear();
        page->location = "/issues";
        return;
    }

    if (path == "/issues") {
        const RepoSummary built = summary(issues);
        writeHtml(page, 200, issuesPage(built, displayIssues(issues), std::string()));
        return;
    }

    if (path == "/issues/new") {
        writeHtml(page, 200, newIssuePage(summary(issues), std::string()));
        return;
    }

    writeHtml(page, 404, missingPage(summary(issues), path + " is not a page here"));
}

}  // namespace hub
}  // namespace rblx
