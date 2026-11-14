#pragma once

// The repository, from the editor.
//
// The editor already showed the branch in its status bar; this is what makes it a
// place to work rather than a place to read: the changed files, one button that
// stages everything, and a commit with a message. The repository code is the same
// GitRepo RBLXGit drives, so what the editor writes git can read and the other way
// round - there is no editor-only format.
//
// Everything here is filesystem and text, so it is checked against a real
// repository in CodeGitTests instead of only in a window.

#include <string>
#include <vector>

namespace rblx {

namespace git {
class Repo;
}

namespace code {

// One changed path, as the Source control panel lists it.
struct ChangeRow {
    std::string path;     // relative to the checkout, '/' separators
    char index = ' ';     // staged state: 'A', 'M', 'D', or ' ' for nothing staged
    char worktree = ' ';  // unstaged state: 'M', 'D', '?' for untracked, or ' '
};

// The two-letter column git prints, with new files as "??". The panel colours by
// this, and the label is what a reader compares with the git status they know.
std::string changeLabel(const ChangeRow& row);
bool isUntracked(const ChangeRow& row);

// Every changed path in the checkout, staged and unstaged and untracked, sorted by
// path. The repository has to be open.
bool loadChanges(git::Repo& repo, std::vector<ChangeRow>* rows, std::string* error);

// Stage everything and record it under one message. A message that is only spaces
// is refused: a commit nobody can read is worse than no commit. On success
// `commitId` is the new commit and `committed` says how many paths went into it.
bool commitEverything(git::Repo& repo, const std::string& message, std::string* commitId,
                      std::size_t* committed, std::string* error);

// Who the commit is from: RBLX_GIT_NAME and RBLX_GIT_EMAIL, the same two variables
// RBLXGit reads, so the editor and the command line write the same author.
std::string authorName();
std::string authorEmail();

// One line for the panel: "3 changed, 1 staged", or "nothing to commit".
std::string changesSummary(const std::vector<ChangeRow>& rows);

}  // namespace code
}  // namespace rblx
