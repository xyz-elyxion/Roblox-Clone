// RBLXGit: the command line over the repository implementation in GitRepo. It
// writes the same layout git does, so a folder it commits in can be opened by git
// and the other way round.

#include "GitRepo.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

void usage() {
    std::printf(
        "RBLXGit - a git command line\n"
        "\n"
        "  RBLXGit init [path]         create a repository (default: this folder)\n"
        "  RBLXGit status              staged changes, working tree changes, new files\n"
        "  RBLXGit add <path>...       stage files or folders; . stages everything\n"
        "  RBLXGit commit -m <text>    record the staged tree as a commit\n"
        "  RBLXGit log [-n <count>]    list commits, newest first\n"
        "  RBLXGit branch [name]       list branches, or switch to and create one\n"
        "  RBLXGit show <rev>:<path>   print a file as it was in a commit\n"
        "\n"
        "Author name and email come from RBLX_GIT_NAME and RBLX_GIT_EMAIL.\n");
}

std::string envOr(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return value != nullptr && *value != '\0' ? std::string(value) : fallback;
}

// Commit times are stored the way git stores them: unix seconds plus an offset.
// Rendering them in UTC keeps this free of the C runtime's local time functions,
// which differ in name and safety between platforms.
std::string utcText(long long seconds) {
    long long days = seconds / 86400;
    long long rest = seconds % 86400;
    if (rest < 0) {
        rest += 86400;
        --days;
    }
    const long long shifted = days + 719468;
    const long long era = (shifted >= 0 ? shifted : shifted - 146096) / 146097;
    const unsigned long long dayOfEra = static_cast<unsigned long long>(shifted - era * 146097);
    const unsigned long long yearOfEra =
        (dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096) / 365;
    const long long yearBase = static_cast<long long>(yearOfEra) + era * 400;
    const unsigned long long dayOfYear = dayOfEra - (365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100);
    const unsigned long long monthPart = (5 * dayOfYear + 2) / 153;
    const unsigned long long day = dayOfYear - (153 * monthPart + 2) / 5 + 1;
    const unsigned long long month = monthPart < 10 ? monthPart + 3 : monthPart - 9;
    const long long year = yearBase + (month <= 2 ? 1 : 0);

    char buffer[96];
    std::snprintf(buffer, sizeof(buffer), "%04lld-%02u-%02u %02d:%02d:%02d UTC", year,
                  static_cast<unsigned>(month), static_cast<unsigned>(day),
                  static_cast<int>(rest / 3600), static_cast<int>((rest / 60) % 60),
                  static_cast<int>(rest % 60));
    return buffer;
}

std::string whenText(const std::string& authorWhen) {
    const std::string seconds = authorWhen.substr(0, authorWhen.find(' '));
    if (seconds.empty() || seconds.find_first_not_of("0123456789") != std::string::npos) {
        return authorWhen;
    }
    return utcText(std::stoll(seconds));
}

std::string firstLineOf(const std::string& text) {
    const std::size_t end = text.find('\n');
    return end == std::string::npos ? text : text.substr(0, end);
}

const char* stagedLabel(char state) {
    switch (state) {
        case 'A':
            return "new file:  ";
        case 'M':
            return "modified:  ";
        case 'D':
            return "deleted:   ";
        default:
            return "           ";
    }
}

int fail(const std::string& message) {
    std::fprintf(stderr, "error: %s\n", message.c_str());
    return 1;
}

int showStatus(rblx::git::Repo& repo) {
    std::vector<rblx::git::StatusEntry> rows;
    std::string error;
    if (!repo.status(&rows, &error)) {
        return fail(error);
    }

    const std::string branch = repo.branch();
    if (repo.head().empty()) {
        std::printf("On branch %s, no commits yet\n", branch.empty() ? "main" : branch.c_str());
    } else {
        std::printf("On branch %s\n", branch.empty() ? "(detached)" : branch.c_str());
    }

    bool staged = false;
    bool worktree = false;
    bool untracked = false;
    for (const auto& row : rows) {
        staged = staged || row.staged != ' ';
        worktree = worktree || row.worktree == 'M' || row.worktree == 'D';
        untracked = untracked || row.worktree == '?';
    }
    if (staged) {
        std::printf("\nChanges to be committed:\n");
        for (const auto& row : rows) {
            if (row.staged != ' ') {
                std::printf("  %s%s\n", stagedLabel(row.staged), row.path.c_str());
            }
        }
    }
    if (worktree) {
        std::printf("\nChanges not staged for commit:\n");
        for (const auto& row : rows) {
            if (row.worktree == 'M' || row.worktree == 'D') {
                std::printf("  %s%s\n", row.worktree == 'M' ? "modified:  " : "deleted:   ",
                            row.path.c_str());
            }
        }
    }
    if (untracked) {
        std::printf("\nUntracked files:\n");
        for (const auto& row : rows) {
            if (row.worktree == '?') {
                std::printf("  %s\n", row.path.c_str());
            }
        }
    }
    if (!staged && !worktree && !untracked) {
        std::printf("nothing to commit, working tree clean\n");
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty()) {
        usage();
        return 1;
    }
    const std::string command = args[0];
    if (command == "help" || command == "--help" || command == "-h") {
        usage();
        return 0;
    }

    const std::string name = envOr("RBLX_GIT_NAME", "RBLX Git");
    const std::string email = envOr("RBLX_GIT_EMAIL", "git@rblx.local");

    rblx::git::Repo repo;
    std::string error;

    if (command == "init") {
        const std::string where = args.size() > 1 ? args[1] : ".";
        if (!repo.init(where, &error)) {
            return fail(error);
        }
        std::printf("Initialised an empty repository in %s\n", repo.gitDir().c_str());
        return 0;
    }

    if (!repo.open(".", &error)) {
        return fail(error);
    }

    if (command == "status") {
        return showStatus(repo);
    }

    if (command == "add") {
        if (args.size() < 2) {
            return fail("add needs at least one path");
        }
        for (std::size_t i = 1; i < args.size(); ++i) {
            if (!repo.add(args[i], &error)) {
                return fail(error);
            }
        }
        return 0;
    }

    if (command == "commit") {
        std::string message;
        for (std::size_t i = 1; i < args.size(); ++i) {
            if (args[i] == "-m") {
                if (i + 1 >= args.size()) {
                    return fail("-m needs a message");
                }
                message += (message.empty() ? "" : "\n\n");
                message += args[++i];
            } else {
                message += (message.empty() ? "" : " ");
                message += args[i];
            }
        }
        if (message.empty()) {
            return fail("a commit needs a message: RBLXGit commit -m \"what changed\"");
        }

        std::vector<rblx::git::StatusEntry> rows;
        if (!repo.status(&rows, &error)) {
            return fail(error);
        }
        std::size_t staged = 0;
        for (const auto& row : rows) {
            if (row.staged != ' ') {
                ++staged;
            }
        }

        std::string id;
        if (!repo.commit(message, name, email, &id, &error)) {
            return fail(error);
        }
        std::printf("[%s %s] %s\n%zu path%s changed\n",
                    repo.branch().empty() ? "detached" : repo.branch().c_str(), id.substr(0, 7).c_str(),
                    firstLineOf(message).c_str(), staged, staged == 1 ? "" : "s");
        return 0;
    }

    if (command == "log") {
        std::size_t limit = 20;
        for (std::size_t i = 1; i < args.size(); ++i) {
            if (args[i] == "-n" && i + 1 < args.size()) {
                limit = static_cast<std::size_t>(std::stoul(args[++i]));
            } else if (args[i].rfind("-n", 0) == 0 && args[i].size() > 2) {
                limit = static_cast<std::size_t>(std::stoul(args[i].substr(2)));
            }
        }
        std::vector<rblx::git::Commit> commits;
        if (!repo.log(&commits, limit, &error)) {
            return fail(error);
        }
        if (commits.empty()) {
            std::printf("no commits yet\n");
            return 0;
        }
        for (const auto& commit : commits) {
            std::printf("commit %s\nAuthor: %s\nDate:   %s\n\n", commit.id.c_str(),
                        commit.author.c_str(), whenText(commit.authorWhen).c_str());
            std::size_t start = 0;
            while (start <= commit.message.size()) {
                const std::size_t end = commit.message.find('\n', start);
                const std::string line = commit.message.substr(
                    start, end == std::string::npos ? std::string::npos : end - start);
                if (!line.empty() || start < commit.message.size()) {
                    std::printf("    %s\n", line.c_str());
                }
                if (end == std::string::npos) {
                    break;
                }
                start = end + 1;
            }
            std::printf("\n");
        }
        return 0;
    }

    if (command == "branch") {
        if (args.size() > 1) {
            if (!repo.setBranch(args[1], &error)) {
                return fail(error);
            }
            std::printf("Switched to branch %s\n", args[1].c_str());
            return 0;
        }
        std::vector<std::string> names;
        if (!repo.branches(&names, &error)) {
            return fail(error);
        }
        if (names.empty()) {
            std::printf("no branches yet\n");
            return 0;
        }
        const std::string current = repo.branch();
        for (const std::string& one : names) {
            std::printf("%s%s\n", one == current ? "* " : "  ", one.c_str());
        }
        return 0;
    }

    if (command == "show") {
        if (args.size() < 2) {
            return fail("show needs <revision>:<path>");
        }
        const std::size_t colon = args[1].find(':');
        if (colon == std::string::npos) {
            return fail("show needs <revision>:<path>");
        }
        std::string id;
        if (!repo.resolve(args[1].substr(0, colon), &id, &error)) {
            return fail(error);
        }
        std::string contents;
        if (!repo.fileAt(id, args[1].substr(colon + 1), &contents, &error)) {
            return fail(error);
        }
        std::fwrite(contents.data(), 1, contents.size(), stdout);
        return 0;
    }

    usage();
    return fail("unknown command '" + command + "'");
}
