#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace rblx {
namespace hub {

// The hub is a set of pages over a repository, and every page is assembled from
// strings so the whole thing can be checked without a socket, a browser or a
// repository on disk. HubMain fills these structures; nothing here reads files.

std::string escapeHtml(const std::string& text);
// Percent-encodes a repository path for use inside an href.
std::string urlPath(const std::string& path);

// Where the pages are mounted. RBLXHub serves them at the root of its own server,
// WebApp under /hub, and every absolute link in a page is built from this so the
// same rendering works for both. Empty (the default) means the root.
void setMountPath(const std::string& mountPath);

struct RepoSummary {
    std::string name;
    std::string description;
    std::string branch = "main";
    std::string head;
    std::string readme;
    std::size_t commits = 0;
    std::size_t files = 0;
    std::size_t issues = 0;
    std::vector<std::string> branches;
};

struct TreeRow {
    std::string name;
    std::string path;
    bool directory = false;
    std::string id;
};

struct CommitRow {
    std::string id;
    std::string author;
    std::string when;
    std::string summary;
};

struct ChangedRow {
    char state = 'M';
    std::string path;
};

struct IssueRow {
    int number = 0;
    std::string title;
    std::string body;
    std::string author;
    std::string when;
    bool open = true;
};

std::string page(const std::string& title, const std::string& body);
std::string breadcrumbs(const std::string& path);

std::string overview(const RepoSummary& repo, const std::vector<TreeRow>& rows,
                     const std::vector<CommitRow>& commits);
std::string treePage(const RepoSummary& repo, const std::string& path,
                     const std::vector<TreeRow>& rows);
std::string blobPage(const RepoSummary& repo, const std::string& path, const std::string& content,
                     bool truncated);
std::string commitsPage(const RepoSummary& repo, const std::vector<CommitRow>& commits);
std::string commitPage(const RepoSummary& repo, const CommitRow& commit,
                       const std::vector<ChangedRow>& changes);
std::string issuesPage(const RepoSummary& repo, const std::vector<IssueRow>& issues,
                       const std::string& notice);
std::string newIssuePage(const RepoSummary& repo, const std::string& error);
std::string missingPage(const RepoSummary& repo, const std::string& what);

}  // namespace hub
}  // namespace rblx
