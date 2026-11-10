#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace rblx {
namespace git {

// What the index remembers about one path. The size is kept because it is the
// cheap half of deciding whether a file still matches: a different size settles
// it without reading the file, and the same size only means it has to be hashed.
struct IndexEntry {
    std::uint32_t mode = 0100644;
    std::string id;  // forty hex characters
    std::string path;
    std::uint32_t size = 0;
};

// `.git/index`, version 2, which is what git itself writes and what `git status`
// reads. Writing a different format would make the repository readable only by
// this program, so the layout is reproduced exactly, including the entry padding
// and the trailing digest of everything before it.
class Index {
public:
    bool read(const std::string& path, std::string* error);
    bool write(const std::string& path, std::string* error) const;

    const IndexEntry* find(const std::string& path) const;
    void insert(const IndexEntry& entry);
    bool erase(const std::string& path);

    const std::vector<IndexEntry>& entries() const { return entries_; }
    std::vector<IndexEntry>& entries() { return entries_; }
    bool empty() const { return entries_.empty(); }
    std::size_t size() const { return entries_.size(); }

private:
    std::vector<IndexEntry> entries_;  // sorted by path, as git requires
};

}  // namespace git
}  // namespace rblx
