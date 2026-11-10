// The git implementation, checked against git's own answers: the published SHA-1
// vectors, a deflate stream zlib wrote, and the object names git computes for two
// files whose contents are fixed below. A changed name means the repository this
// program writes is no longer readable by git, which is the one thing this unit
// must not do.

#include "GitIndex.h"
#include "GitRepo.h"
#include "GitSha1.h"
#include "GitZlib.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;
namespace git = rblx::git;

namespace {

int failures = 0;

void check(bool ok, const std::string& what) {
    if (!ok) {
        std::printf("  failed: %s\n", what.c_str());
        ++failures;
    }
}

void writeFile(const fs::path& path, const std::string& text) {
    fs::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(text.data(), static_cast<std::streamsize>(text.size()));
}

std::string readFile(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

std::string fromHex(const std::string& hex) {
    std::string out;
    for (std::size_t i = 0; i + 1 < hex.size(); i += 2) {
        out.push_back(static_cast<char>(std::stoul(hex.substr(i, 2), nullptr, 16)));
    }
    return out;
}

void testDigest() {
    check(git::sha1Hex(std::string()) == "da39a3ee5e6b4b0d3255bfef95601890afd80709",
          "the digest of nothing");
    check(git::sha1Hex("abc") == "a9993e364706816aba3e25717850c26c9cd0d89d", "the digest of abc");
    check(git::sha1Hex("The quick brown fox jumps over the lazy dog") ==
              "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12",
          "the digest of the fox");

    // A million characters is where a streaming implementation usually shows a
    // block-boundary mistake.
    std::string large(1000000, 'a');
    check(git::sha1Hex(large) == "34aa973cd4c4daa4f61eeb2bdbad27316534016f",
          "the digest of a million a's");
}

void testCompression() {
    std::string error;
    // Written by Python's zlib: a dynamic Huffman stream, not the stored form this
    // program produces, so inflating it proves the reader handles real input.
    const std::string stream = fromHex(
        "78da0b72f2895048cf2c51c84fca4a4d2e2956482c4a55a8cac94c52282e294a4dcc2dd6e30200d9aa0c27");
    std::string inflated;
    check(git::inflateZlib(stream, &inflated, &error), "a zlib stream inflates: " + error);
    check(inflated == "RBLX git objects are zlib streams.\n", "the inflated text matches");

    std::string stored;
    check(git::deflateZlibStored(inflated, &stored, &error), "a stored stream is written: " + error);
    std::string back;
    check(git::inflateZlib(stored, &back, &error) && back == inflated, "the stored stream round trips");

    std::string damaged = stored;
    damaged[damaged.size() / 2] = static_cast<char>(damaged[damaged.size() / 2] ^ 0x40);
    check(!git::inflateZlib(damaged, &back, &error), "a damaged stream is refused");
    check(!git::inflateZlib(stored.substr(0, stored.size() / 2), &back, &error),
          "a truncated stream is refused");
}

void testIndex() {
    const fs::path dir = fs::temp_directory_path() / "rblx-git-tests-index";
    fs::remove_all(dir);
    fs::create_directories(dir);

    git::Index index;
    git::IndexEntry second;
    second.mode = 0100755;
    second.id = std::string(40, 'b');
    second.path = "b.txt";
    second.size = 12;
    git::IndexEntry first;
    first.id = std::string(40, 'a');
    first.path = "src/a.txt";
    first.size = 6;
    index.insert(second);
    index.insert(first);  // deliberately out of order
    check(index.entries().front().path == "b.txt", "entries are kept sorted");
    check(index.size() == 2, "both entries are kept");

    const std::string path = (dir / "index").string();
    std::string error;
    check(index.write(path, &error), "the index writes: " + error);

    git::Index loaded;
    check(loaded.read(path, &error), "the index reads back: " + error);
    const git::IndexEntry* found = loaded.find("src/a.txt");
    check(found != nullptr, "a nested path survives");
    check(found != nullptr && found->id == std::string(40, 'a') && found->size == 6,
          "the object name and size survive");
    const git::IndexEntry* executable = loaded.find("b.txt");
    check(executable != nullptr && executable->mode == 0100755, "the mode survives");

    // git trusts the trailing digest instead of re-hashing entries, so a flipped
    // byte has to be refused rather than half-read.
    std::string raw = readFile(path);
    raw[20] = static_cast<char>(raw[20] ^ 0x01);
    writeFile(dir / "broken.index", raw);
    git::Index broken;
    check(!broken.read((dir / "broken.index").string(), &error), "a damaged index is refused");

    fs::remove_all(dir);
}

void testRepository() {
    const fs::path dir = fs::temp_directory_path() / "rblx-git-tests-repo";
    fs::remove_all(dir);
    fs::create_directories(dir);

    git::Repo repo;
    std::string error;
    check(repo.init(dir.string(), &error), "init: " + error);
    check(repo.branch() == "main", "a new repository is on main");
    check(repo.head().empty(), "a new repository has no commit");

    writeFile(dir / "a.txt", "alpha\n");
    writeFile(dir / "sub" / "b.txt", "beta\n");

    std::string id;
    check(!repo.commit("nothing staged", "Tester", "t@example.com", &id, &error),
          "an empty index cannot be committed");

    check(repo.addAll(&error), "add all: " + error);
    std::vector<git::StatusEntry> rows;
    check(repo.status(&rows, &error), "status: " + error);
    check(rows.size() == 2 && rows[0].path == "a.txt" && rows[0].staged == 'A',
          "both files are staged as new");

    // The names git itself gives these two files.
    std::string blob;
    check(repo.writeObject(git::ObjectKind::Blob, "alpha\n", &blob, &error), "a blob writes: " + error);
    check(blob == "4a58007052a65fbc2fc3f910f2855f45a4058e74", "the blob name is git's");
    check(repo.writeObject(git::ObjectKind::Blob, "beta\n", &blob, &error), "the second blob writes");
    check(blob == "65b2df87f7df3aeedef04be96703e55ac19c2cfb", "the second blob name is git's");

    std::string first;
    check(repo.commit("first commit", "Tester", "t@example.com", &first, &error), "commit: " + error);

    git::Commit commit;
    check(repo.readCommit(first, &commit, &error), "the commit reads back: " + error);
    check(commit.tree == "067bc99639cc141f3e5041022e822dfa8de165e9", "the root tree name is git's");
    check(commit.message == "first commit\n", "the message is stored");
    check(commit.author == "Tester <t@example.com>", "the author is stored");
    check(commit.parents.empty(), "the first commit has no parent");

    std::map<std::string, git::TreeEntry> root;
    check(repo.treeEntries(commit.tree, &root, &error), "the root tree reads: " + error);
    check(root.size() == 2 && root.count("sub") == 1 && root["sub"].mode == 0040000,
          "the folder is a tree entry");
    check(root.count("sub") == 1 && root["sub"].id == "23b08af3548c6d2c1611b1671385a25e9a9fe1eb",
          "the folder tree name is git's");

    writeFile(dir / "a.txt", "alpha changed\n");
    rows.clear();
    check(repo.status(&rows, &error), "status after the change: " + error);
    check(rows.size() == 1 && rows[0].path == "a.txt" && rows[0].staged == ' ' &&
              rows[0].worktree == 'M',
          "the change is unstaged");
    check(!repo.commit("second", "Tester", "t@example.com", &id, &error),
          "an unchanged tree cannot be committed");

    check(repo.add("a.txt", &error), "the change is staged: " + error);
    std::string second;
    check(repo.commit("second commit", "Tester", "t@example.com", &second, &error),
          "second commit: " + error);

    std::vector<git::Commit> log;
    check(repo.log(&log, 10, &error), "log: " + error);
    check(log.size() == 2 && log[0].id == second && log[1].id == first,
          "log walks back through the parent");

    std::vector<git::StatusEntry> changes;
    check(repo.changedPaths(first, second, &changes, &error), "changed paths: " + error);
    check(changes.size() == 1 && changes[0].path == "a.txt" && changes[0].staged == 'M',
          "one modified path between the commits");

    std::string contents;
    check(repo.fileAt(first, "sub/b.txt", &contents, &error) && contents == "beta\n",
          "an older revision still reads");
    check(repo.fileAt(second, "a.txt", &contents, &error) && contents == "alpha changed\n",
          "the newer revision reads");

    check(repo.setBranch("feature", &error), "switch branch: " + error);
    check(repo.branch() == "feature", "the branch switched");
    check(repo.resolve("main", &id, &error) && id == second, "a branch name resolves to its commit");

    fs::remove(dir / "sub" / "b.txt");
    check(repo.add("sub/b.txt", &error), "a deletion is staged: " + error);
    std::string third;
    check(repo.commit("drop b", "Tester", "t@example.com", &third, &error), "third commit: " + error);

    std::map<std::string, git::TreeEntry> files;
    check(repo.filesAt(third, &files, &error), "the new tree reads: " + error);
    check(files.size() == 1 && files.count("a.txt") == 1, "the deleted file is gone");

    std::vector<std::string> branches;
    check(repo.branches(&branches, &error), "branches: " + error);
    check(std::find(branches.begin(), branches.end(), "feature") != branches.end(),
          "the new branch has a ref");
    check(std::find(branches.begin(), branches.end(), "main") != branches.end(),
          "the old branch is still there");

    // Untracked files are reported without being staged.
    writeFile(dir / "new.txt", "new\n");
    rows.clear();
    check(repo.status(&rows, &error), "status with an untracked file: " + error);
    check(rows.size() == 1 && rows[0].path == "new.txt" && rows[0].worktree == '?',
          "the new file is untracked");

    fs::remove_all(dir);
}

}  // namespace

int main() {
    testDigest();
    testCompression();
    testIndex();
    testRepository();

    if (failures == 0) {
        std::printf("GitTests: all checks passed\n");
        return 0;
    }
    std::printf("GitTests: %d checks failed\n", failures);
    return 1;
}
