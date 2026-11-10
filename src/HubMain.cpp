// RBLXHub: a repository browser for the repository on this machine.
//
// It reads the repository through GitRepo - the same code RBLXGit commits with -
// and renders every page as a string with HubViews. There is no template engine,
// no JavaScript and no database: issues live in one JSON file beside the reflog,
// and the pages are HTML that a browser can read with scripting switched off.
//
//   RBLXHub                      the repository this folder is inside, 127.0.0.1:3100
//   RBLXHub --repo .. --port 8080 --host 0.0.0.0
//   RBLXHub --help

#include "GitRepo.h"
#include "HubViews.h"
#include "WebHttp.h"
#include "WebJson.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace fs = std::filesystem;
namespace git = rblx::git;
namespace hub = rblx::hub;

namespace {

const std::uint32_t kDirectoryMode = 0040000;
const std::size_t kMaxBlobBytes = 512 * 1024;

rblx::web::HttpServer* g_server = nullptr;

BOOL WINAPI consoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT || signal == CTRL_CLOSE_EVENT) {
        if (g_server != nullptr) {
            g_server->stop();
        }
        return TRUE;
    }
    return FALSE;
}

std::string executableDirectory() {
    std::vector<char> buffer(1024);
    const DWORD length = ::GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0) {
        return std::string(".");
    }
    const std::string path(buffer.data(), length);
    const std::size_t slash = path.find_last_of("/\\");
    return slash == std::string::npos ? std::string(".") : path.substr(0, slash);
}

// web/hub, found the way RBLXHosts finds its panel and WebApp finds the site:
// next to the executables first, then by walking up from them and from the folder
// this program was started in. The package keeps web/ beside the executables, so
// the first step usually wins.
std::string findWebDirectory() {
    std::vector<std::string> bases{executableDirectory(), std::string(".")};
    for (const std::string& base : bases) {
        std::string candidate = base;
        for (int depth = 0; depth < 5 && !candidate.empty(); ++depth) {
            for (const std::string& root : {candidate + "/web", candidate}) {
                std::ifstream probe(root + "/hub/hub.css");
                if (probe) {
                    return root + "/hub";
                }
            }
            const std::size_t slash = candidate.find_last_of("/\\");
            if (slash == std::string::npos) {
                break;
            }
            candidate = candidate.substr(0, slash);
        }
    }
    return std::string();
}

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
        const std::string pair = body.substr(start, end == std::string::npos ? std::string::npos
                                                                            : end - start);
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

// Everything that describes the repository for the pages: name, counts, branches
// and the README. It is rebuilt per request on purpose - the repository can change
// under a running server, and a few dozen small files cost nothing to re-read.
struct Hub {
    git::Repo repo;
    std::string name;
    std::string webDirectory;
    std::string issuesPath;

    hub::RepoSummary summary(const std::vector<hub::IssueRow>& issues) const {
        hub::RepoSummary out;
        out.name = name;
        out.branch = repo.branch().empty() ? std::string("main") : repo.branch();
        out.head = repo.head();
        out.issues = issues.size();

        std::string error;
        std::vector<git::Commit> commits;
        if (repo.log(&commits, 100000, &error)) {
            out.commits = commits.size();
        }
        std::map<std::string, git::TreeEntry> files;
        if (repo.filesAtHead(&files, &error)) {
            out.files = files.size();
        }
        repo.branches(&out.branches, &error);

        std::string readme;
        if (!out.head.empty() && repo.fileAt(out.head, "README.md", &readme, &error)) {
            out.readme = readme;
        } else {
            out.readme = readText((fs::path(repo.workTree()) / "README.md").string());
        }
        if (out.readme.size() > 20000) {
            out.readme = out.readme.substr(0, 20000) + "\n...";
        }
        out.description = firstHeading(out.readme);
        return out;
    }

    static std::string firstHeading(const std::string& readme) {
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
};

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

bool rowsAt(const git::Repo& repo, const std::string& path, std::vector<hub::TreeRow>* rows,
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
        hub::TreeRow row;
        row.name = item.first;
        row.path = path.empty() ? item.first : path + "/" + item.first;
        row.directory = item.second.mode == kDirectoryMode;
        row.id = item.second.id;
        rows->push_back(row);
    }
    std::sort(rows->begin(), rows->end(), [](const hub::TreeRow& left, const hub::TreeRow& right) {
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
std::vector<hub::IssueRow> displayIssues(const std::vector<hub::IssueRow>& stored) {
    std::vector<hub::IssueRow> out = stored;
    for (hub::IssueRow& issue : out) {
        issue.when = agoText(issue.when);
    }
    return out;
}

std::vector<hub::CommitRow> recentCommits(git::Repo& repo, std::size_t limit) {
    std::vector<hub::CommitRow> rows;
    std::vector<git::Commit> commits;
    std::string error;
    if (!repo.log(&commits, limit, &error)) {
        return rows;
    }
    for (const git::Commit& commit : commits) {
        hub::CommitRow row;
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
std::vector<hub::IssueRow> loadIssues(const std::string& path) {
    std::vector<hub::IssueRow> issues;
    const std::string text = readText(path);
    if (text.empty()) {
        return issues;
    }
    std::string error;
    const rblx::web::Json document = rblx::web::Json::parse(text, &error);
    if (!document.isArray()) {
        return issues;
    }
    for (const rblx::web::Json& item : document.elements()) {
        hub::IssueRow issue;
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

bool saveIssues(const std::string& path, const std::vector<hub::IssueRow>& issues, std::string* error) {
    rblx::web::Json array = rblx::web::Json::makeArray();
    for (const hub::IssueRow& issue : issues) {
        rblx::web::Json item = rblx::web::Json::makeObject();
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

void sendHtml(rblx::web::Response& response, int status, const std::string& body) {
    response.text(status, body, "text/html; charset=utf-8");
    // No scripts anywhere: the pages are static HTML, so the browser is told that
    // nothing but this origin's stylesheet may load or run.
    response.headers["Content-Security-Policy"] =
        "default-src 'none'; style-src 'self'; form-action 'self'";
    response.headers["X-Content-Type-Options"] = "nosniff";
    response.headers["Referrer-Policy"] = "no-referrer";
}

}  // namespace

int main(int argc, char** argv) {
    std::string repoPath = ".";
    std::string host = "127.0.0.1";
    int port = 3100;

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--help" || argument == "-h" || argument == "/?") {
            std::printf(
                "RBLXHub - browse the git repository in this folder\n"
                "\n"
                "  RBLXHub [repository] [--host address] [--port number]\n"
                "\n"
                "Start the repository with RBLXGit init and commit something first; the\n"
                "hub shows the commits, the files at HEAD and the issues kept beside the\n"
                "repository. It listens on loopback only unless --host says otherwise.\n");
            return 0;
        }
        if (argument == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (argument == "--port" && i + 1 < argc) {
            try {
                port = std::stoi(argv[++i]);
            } catch (...) {
                std::fprintf(stderr, "RBLXHub: --port needs a number\n");
                return 2;
            }
        } else if (argument.rfind("--", 0) == 0) {
            std::fprintf(stderr, "RBLXHub: unknown option %s (try --help)\n", argument.c_str());
            return 2;
        } else {
            repoPath = argument;
        }
    }
    if (port < 1 || port > 65535) {
        std::fprintf(stderr, "RBLXHub: --port must be between 1 and 65535\n");
        return 2;
    }

    std::string error;
    Hub hubState;
    if (!hubState.repo.open(repoPath, &error)) {
        std::fprintf(stderr, "RBLXHub: %s\n", error.c_str());
        std::fprintf(stderr, "RBLXHub: run RBLXGit init and make a commit first.\n");
        return 2;
    }
    hubState.name = fs::path(hubState.repo.workTree()).filename().string();
    if (hubState.name.empty()) {
        hubState.name = "repository";
    }
    hubState.webDirectory = findWebDirectory();
    hubState.issuesPath = hubState.repo.gitDir() + "/hub-issues.json";

    rblx::web::HttpServer server;
    // A form post is a title and a few lines of text; anything larger is someone
    // pushing on the socket rather than using the page.
    server.setBodyLimitBytes(64 * 1024);

    server.setFallback([&hubState](rblx::web::Request& request, rblx::web::Response& response) {
        // One connection per thread: a shared error string would be written by
        // two of them at once, so each request carries its own.
        std::string error;
        const std::string path = request.path;
        std::vector<hub::IssueRow> issues = loadIssues(hubState.issuesPath);

        if (path == "/hub.css") {
            if (hubState.webDirectory.empty()) {
                response.text(404, "hub.css is not beside the executable", "text/plain; charset=utf-8");
                return true;
            }
            response.file(200, hubState.webDirectory + "/hub.css", "text/css; charset=utf-8");
            return true;
        }

        if (path == "/" || path.empty()) {
            std::vector<hub::TreeRow> rows;
            std::string failure;
            const bool ok = rowsAt(hubState.repo, std::string(), &rows, &failure);
            const hub::RepoSummary summary = hubState.summary(issues);
            sendHtml(response, 200, ok ? hub::overview(summary, rows, recentCommits(hubState.repo, 8))
                                       : hub::missingPage(summary, failure));
            return true;
        }

        if (path.rfind("/tree/", 0) == 0 || path == "/tree") {
            const std::string folder = urlDecode(path.size() > 6 ? path.substr(6) : std::string());
            std::vector<hub::TreeRow> rows;
            std::string failure;
            const hub::RepoSummary summary = hubState.summary(issues);
            if (!rowsAt(hubState.repo, folder, &rows, &failure)) {
                sendHtml(response, 404, hub::missingPage(summary, failure));
                return true;
            }
            sendHtml(response, 200, hub::treePage(summary, folder, rows));
            return true;
        }

        if (path.rfind("/blob/", 0) == 0) {
            const std::string file = urlDecode(path.substr(6));
            const hub::RepoSummary summary = hubState.summary(issues);
            std::string contents;
            if (summary.head.empty() ||
                !hubState.repo.fileAt(summary.head, file, &contents, &error)) {
                sendHtml(response, 404, hub::missingPage(summary, file + " is not in HEAD"));
                return true;
            }
            const bool truncated = contents.size() > kMaxBlobBytes;
            if (truncated) {
                contents.resize(kMaxBlobBytes);
            }
            sendHtml(response, 200, hub::blobPage(summary, file, contents, truncated));
            return true;
        }

        if (path == "/commits") {
            const hub::RepoSummary summary = hubState.summary(issues);
            sendHtml(response, 200, hub::commitsPage(summary, recentCommits(hubState.repo, 200)));
            return true;
        }

        if (path.rfind("/commit/", 0) == 0) {
            const std::string revision = urlDecode(path.substr(8));
            const hub::RepoSummary summary = hubState.summary(issues);
            std::string commitId;
            git::Commit commit;
            if (!hubState.repo.resolve(revision, &commitId, &error) ||
                !hubState.repo.readCommit(commitId, &commit, &error)) {
                sendHtml(response, 404, hub::missingPage(summary, "No commit " + revision));
                return true;
            }
            hub::CommitRow row;
            row.id = commit.id;
            row.author = commit.author;
            row.when = agoText(commit.authorWhen);
            const std::size_t end = commit.message.find('\n');
            row.summary = commit.message.substr(0, end == std::string::npos ? std::string::npos : end);

            std::vector<hub::ChangedRow> changes;
            std::vector<git::StatusEntry> rows;
            const std::string parent = commit.parents.empty() ? std::string() : commit.parents.front();
            if (hubState.repo.changedPaths(parent, commit.id, &rows, &error)) {
                for (const git::StatusEntry& changed : rows) {
                    hub::ChangedRow entry;
                    entry.state = changed.staged == ' ' ? 'M' : changed.staged;
                    entry.path = changed.path;
                    changes.push_back(entry);
                }
            }
            sendHtml(response, 200, hub::commitPage(summary, row, changes));
            return true;
        }

        if (path == "/issues" && request.method == "POST") {
            const std::map<std::string, std::string> fields = parseForm(request.body);
            const auto title = fields.find("title");
            hub::IssueRow issue;
            issue.title = title == fields.end() ? std::string() : title->second;
            const auto body = fields.find("body");
            issue.body = body == fields.end() ? std::string() : body->second;
            const auto author = fields.find("author");
            issue.author = author == fields.end() || author->second.empty() ? "guest" : author->second;

            const hub::RepoSummary summary = hubState.summary(issues);
            if (issue.title.empty()) {
                sendHtml(response, 400, hub::newIssuePage(summary, "An issue needs a title."));
                return true;
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
            for (const hub::IssueRow& existing : issues) {
                highest = std::max(highest, existing.number);
            }
            issue.number = highest + 1;
            issue.when = stampNow();
            issue.open = true;
            issues.push_back(issue);
            if (!saveIssues(hubState.issuesPath, issues, &error)) {
                sendHtml(response, 500, hub::newIssuePage(summary, error));
                return true;
            }
            response.status = 303;
            response.headers["Location"] = "/issues";
            response.body.clear();
            response.contentType = "text/plain; charset=utf-8";
            return true;
        }

        if (path == "/issues") {
            const hub::RepoSummary summary = hubState.summary(issues);
            sendHtml(response, 200, hub::issuesPage(summary, displayIssues(issues), std::string()));
            return true;
        }

        if (path == "/issues/new") {
            sendHtml(response, 200, hub::newIssuePage(hubState.summary(issues), std::string()));
            return true;
        }

        sendHtml(response, 404, hub::missingPage(hubState.summary(issues), path + " is not a page here"));
        return true;
    });

    if (!server.listen(host, port, &error)) {
        std::fprintf(stderr, "RBLXHub: %s\n", error.c_str());
        return 1;
    }

    g_server = &server;
    ::SetConsoleCtrlHandler(consoleHandler, TRUE);

    std::printf("RBLXHub: %s on http://%s:%d\n", hubState.name.c_str(), host.c_str(), server.port());
    std::printf("RBLXHub: Ctrl+C stops the server. Pages are rendered from the repository.\n");
    std::fflush(stdout);

    server.serveForever();

    g_server = nullptr;
    std::printf("RBLXHub: stopped.\n");
    return 0;
}
