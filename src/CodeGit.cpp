#include "CodeGit.h"

#include "Environment.h"
#include "GitRepo.h"

#include <cctype>
#include <cstdlib>

namespace rblx {
namespace code {
namespace {

std::string trim(const std::string& text) {
    std::size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) ++start;
    std::size_t end = text.size();
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(start, end - start);
}

std::string envOr(const char* name, const char* fallback) {
    const std::string value = rblx::environment::get(name);
    return value.empty() ? std::string(fallback) : value;
}

}  // namespace

std::string changeLabel(const ChangeRow& row) {
    std::string label;
    if (row.worktree == '?' && row.index == ' ') {
        return "??";
    }
    label.push_back(row.index == ' ' ? ' ' : row.index);
    label.push_back(row.worktree == ' ' ? ' ' : row.worktree);
    return label;
}

bool isUntracked(const ChangeRow& row) { return row.worktree == '?' && row.index == ' '; }

bool loadChanges(git::Repo& repo, std::vector<ChangeRow>* rows, std::string* error) {
    if (rows == nullptr) return false;
    rows->clear();
    if (!repo.isOpen()) {
        if (error != nullptr) *error = "this folder is not a repository";
        return false;
    }
    std::vector<git::StatusEntry> entries;
    if (!repo.status(&entries, error)) return false;

    rows->reserve(entries.size());
    for (const git::StatusEntry& entry : entries) {
        ChangeRow row;
        row.path = entry.path;
        row.index = entry.staged;
        row.worktree = entry.worktree;
        rows->push_back(row);
    }
    return true;
}

bool commitEverything(git::Repo& repo, const std::string& message, std::string* commitId,
                      std::size_t* committed, std::string* error) {
    const std::string text = trim(message);
    if (text.empty()) {
        if (error != nullptr) *error = "a commit needs a message";
        return false;
    }
    if (!repo.isOpen()) {
        if (error != nullptr) *error = "this folder is not a repository";
        return false;
    }

    std::vector<git::StatusEntry> before;
    if (!repo.status(&before, error)) return false;
    std::size_t changed = 0;
    for (const git::StatusEntry& row : before) {
        if (row.staged != ' ' || row.worktree != ' ') ++changed;
    }
    if (changed == 0) {
        if (error != nullptr) *error = "nothing to commit";
        return false;
    }

    if (!repo.addAll(error)) return false;
    std::string id;
    if (!repo.commit(text, authorName(), authorEmail(), &id, error)) return false;
    if (commitId != nullptr) *commitId = id;
    if (committed != nullptr) *committed = changed;
    return true;
}

std::string authorName() { return envOr("RBLX_GIT_NAME", "RBLX Code"); }

std::string authorEmail() { return envOr("RBLX_GIT_EMAIL", "editor@rblx.local"); }

std::string changesSummary(const std::vector<ChangeRow>& rows) {
    if (rows.empty()) return "nothing to commit";
    std::size_t staged = 0;
    std::size_t untracked = 0;
    for (const ChangeRow& row : rows) {
        if (row.index != ' ') ++staged;
        if (isUntracked(row)) ++untracked;
    }
    std::string summary = std::to_string(rows.size()) +
                          (rows.size() == 1 ? " changed path" : " changed paths");
    if (staged > 0) summary += ", " + std::to_string(staged) + " staged";
    if (untracked > 0) summary += ", " + std::to_string(untracked) + " new";
    return summary;
}

}  // namespace code
}  // namespace rblx
