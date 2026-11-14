// CodeGitTests: the Source control panel's work, against a real repository.
//
// The panel stages everything and commits it, which is the part of the editor that
// can leave a repository in a state somebody has to fix by hand. So the checks run
// the same helpers against a repository in a temporary folder and then read the
// result back through the same Repo the panel uses.

#include "CodeGit.h"
#include "GitRepo.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace code = rblx::code;
namespace git = rblx::git;
namespace fs = std::filesystem;

namespace {

int g_failures = 0;

void check(bool ok, const std::string& what) {
    if (!ok) {
        std::printf("FAIL %s\n", what.c_str());
        ++g_failures;
    }
}

void writeFile(const fs::path& path, const std::string& text) {
    std::ofstream stream(path.string().c_str(), std::ios::binary | std::ios::trunc);
    stream << text;
}

}  // namespace

int main() {
    const fs::path root = fs::temp_directory_path() / "rblx-code-git-tests";
    std::error_code error;
    fs::remove_all(root, error);
    fs::create_directories(root, error);
    writeFile(root / "README.md", "# test\n");

    git::Repo repo;
    std::string message;
    check(repo.init(root.string(), &message), "a repository is created: " + message);

    // A new file, a staged file and a modified file, each labelled the way git
    // prints them.
    {
        std::vector<code::ChangeRow> rows;
        check(code::loadChanges(repo, &rows, &message), "status on a fresh repository: " + message);
        check(rows.size() == 1, "the untracked file is listed, got " + std::to_string(rows.size()));
        if (!rows.empty()) {
            check(rows[0].path == "README.md", "the path is relative: " + rows[0].path);
            check(code::changeLabel(rows[0]) == "??", "an untracked file is ??");
            check(code::isUntracked(rows[0]), "and it is reported as untracked");
        }
        check(code::changesSummary(rows) == "1 changed path, 1 new",
              "the summary counts it: " + code::changesSummary(rows));

        code::ChangeRow staged;
        staged.path = "src/main.cpp";
        staged.index = 'A';
        check(code::changeLabel(staged) == "A ", "a staged new file is 'A '");
        code::ChangeRow modified;
        modified.path = "src/main.cpp";
        modified.index = 'M';
        modified.worktree = 'M';
        check(code::changeLabel(modified) == "MM", "a file changed both ways is 'MM'");
        check(!code::isUntracked(modified), "a staged file is not untracked");
    }

    // A commit with no message is refused, and nothing is staged by the attempt.
    {
        std::string id;
        std::size_t committed = 0;
        check(!code::commitEverything(repo, "   \n\t ", &id, &committed, &message),
              "a blank message is refused");
        check(message.find("message") != std::string::npos, "the refusal says why: " + message);
        std::vector<code::ChangeRow> rows;
        check(code::loadChanges(repo, &rows, &message), "status after the refusal: " + message);
        check(rows.size() == 1 && code::isUntracked(rows[0]),
              "the file is still untracked after a refused commit");
    }

    // The commit the button makes: everything staged, one message, one commit.
    {
        writeFile(root / "main.cpp", "int main() { return 0; }\n");
        std::string id;
        std::size_t committed = 0;
        check(code::commitEverything(repo, "  First commit from the editor  ", &id, &committed,
                                     &message),
              "the commit succeeds: " + message);
        check(!id.empty(), "it produced a commit id");
        check(committed == 2, "both paths went in, got " + std::to_string(committed));

        std::vector<code::ChangeRow> rows;
        check(code::loadChanges(repo, &rows, &message), "status after the commit: " + message);
        check(rows.empty(), "the checkout is clean afterwards, " + std::to_string(rows.size()) +
                                " row(s) left");
        check(code::changesSummary(rows) == "nothing to commit", "and the summary says so");

        std::vector<git::Commit> commits;
        check(repo.log(&commits, 10, &message), "the log reads: " + message);
        check(commits.size() == 1, "one commit exists, got " + std::to_string(commits.size()));
        if (!commits.empty()) {
            // The repository keeps the trailing newline git writes at the end of a
            // message body; what the panel typed was already trimmed.
            std::string stored = commits[0].message;
            while (!stored.empty() && (stored.back() == '\n' || stored.back() == '\r')) {
                stored.pop_back();
            }
            check(stored == "First commit from the editor",
                  "the message is trimmed and kept: " + commits[0].message);
            check(commits[0].author.rfind(code::authorName(), 0) == 0,
                  "the author is the configured one: " + commits[0].author);
        }

        // Committing a clean tree is refused rather than recording an empty commit.
        check(!code::commitEverything(repo, "nothing changed", &id, &committed, &message),
              "a clean checkout is not committed again");
        check(message == "nothing to commit", "and it says so: " + message);
    }

    // A folder that is not a repository is answered, not guessed at.
    {
        const fs::path plain = root / "plain";
        fs::create_directories(plain, error);
        git::Repo nowhere;
        std::vector<code::ChangeRow> rows;
        check(!code::loadChanges(nowhere, &rows, &message), "a plain folder is refused");
        check(message.find("not a repository") != std::string::npos,
              "the refusal is readable: " + message);
    }

    // A second commit on top, then a modification, so the panel's two columns are
    // exercised against the real repository rather than a hand-built row.
    {
        writeFile(root / "main.cpp", "int main() { return 1; }\n");
        std::vector<code::ChangeRow> rows;
        check(code::loadChanges(repo, &rows, &message), "status sees the edit: " + message);
        check(rows.size() == 1 && rows[0].worktree == 'M' && rows[0].index == ' ',
              "an edited tracked file is ' M'");
        std::string id;
        std::size_t committed = 0;
        check(code::commitEverything(repo, "Change the answer", &id, &committed, &message),
              "the second commit succeeds: " + message);
        std::vector<git::Commit> commits;
        check(repo.log(&commits, 10, &message) && commits.size() == 2,
              "there are two commits now");
    }

    fs::remove_all(root, error);

    if (g_failures == 0) {
        std::printf("CodeGitTests: all checks passed\n");
        return 0;
    }
    std::printf("CodeGitTests: %d check(s) failed\n", g_failures);
    return 1;
}
