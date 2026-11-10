#pragma once

#include "GitIndex.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace rblx {
namespace git {

enum class ObjectKind { Blob, Tree, Commit };

std::string kindName(ObjectKind kind);

struct TreeEntry {
    std::uint32_t mode = 0;
    std::string id;
};

struct Commit {
    std::string id;
    std::string tree;
    std::vector<std::string> parents;
    std::string author;
    std::string authorWhen;
    std::string message;
};

// One row of `status`: what the index says compared with HEAD, and what the
// working tree says compared with the index. The two are separate because a file
// can be changed in both places at once.
struct StatusEntry {
    char staged = ' ';
    char worktree = ' ';
    std::string path;
};

// A repository on disk in the layout git defines: loose objects, refs, HEAD and
// an index. Nothing here is this project's own format, so a repository this class
// writes is readable by git and the other way round.
class Repo {
public:
    bool open(const std::string& start, std::string* error);
    bool init(const std::string& workTree, std::string* error);

    bool isOpen() const { return !gitDir_.empty(); }
    const std::string& workTree() const { return workTree_; }
    const std::string& gitDir() const { return gitDir_; }

    std::string branch() const;
    std::string head() const;
    bool setBranch(const std::string& name, std::string* error);
    bool branches(std::vector<std::string>* names, std::string* error) const;

    bool readObject(const std::string& id, ObjectKind* kind, std::string* payload,
                    std::string* error) const;
    bool writeObject(ObjectKind kind, const std::string& payload, std::string* id,
                     std::string* error) const;
    bool readCommit(const std::string& id, Commit* commit, std::string* error) const;
    bool resolve(const std::string& revision, std::string* id, std::string* error) const;

    bool treeEntries(const std::string& treeId, std::map<std::string, TreeEntry>* entries,
                     std::string* error) const;
    bool headTree(std::string* treeId, std::string* error) const;
    bool filesAtHead(std::map<std::string, TreeEntry>* files, std::string* error) const;
    bool filesAt(const std::string& commitId, std::map<std::string, TreeEntry>* files,
                 std::string* error) const;
    bool fileAt(const std::string& commitId, const std::string& path, std::string* contents,
                std::string* error) const;
    bool changedPaths(const std::string& fromCommit, const std::string& toCommit,
                      std::vector<StatusEntry>* changes, std::string* error) const;

    bool loadIndex(std::string* error);
    const Index& index() const { return index_; }
    bool add(const std::string& path, std::string* error);
    bool addAll(std::string* error);
    bool commit(const std::string& message, const std::string& name, const std::string& email,
                std::string* id, std::string* error);
    bool log(std::vector<Commit>* commits, std::size_t limit, std::string* error) const;
    bool status(std::vector<StatusEntry>* entries, std::string* error);

private:
    std::string absolute(const std::string& relative) const;
    std::string relative(const std::string& absolutePath) const;
    std::string indexPath() const { return gitDir_ + "/index"; }

    bool addFile(const std::string& relativePath, std::string* error);
    bool stagePath(const std::string& relativePath, std::string* error);
    bool saveIndex(std::string* error);
    bool hashFile(const std::string& relativePath, std::string* id, std::uint32_t* size,
                  std::string* error) const;
    bool walkTree(const std::string& treeId, const std::string& prefix,
                  std::map<std::string, TreeEntry>* files, std::string* error) const;
    bool readRef(const std::string& name, std::string* id) const;
    bool writeRef(const std::string& name, const std::string& id, std::string* error) const;
    void appendReflog(const std::string& ref, const std::string& from, const std::string& to,
                      const std::string& name, const std::string& email,
                      const std::string& message) const;

    std::string workTree_;
    std::string gitDir_;
    Index index_;
};

}  // namespace git
}  // namespace rblx
