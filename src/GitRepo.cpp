#include "GitRepo.h"

#include "GitSha1.h"
#include "GitZlib.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <set>
#include <utility>

namespace fs = std::filesystem;

namespace rblx {
namespace git {
namespace {

const std::uint32_t kDirectoryMode = 0040000;

std::string readFile(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

bool writeFile(const fs::path& path, const std::string& contents, std::string* error) {
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        *error = "cannot write " + path.string();
        return false;
    }
    out.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    return static_cast<bool>(out);
}

bool looksLikeObjectId(const std::string& text) {
    if (text.size() < 4 || text.size() > 40) {
        return false;
    }
    return text.find_first_not_of("0123456789abcdef") == std::string::npos;
}

std::string modeText(std::uint32_t mode) {
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%o", mode);
    return buffer;
}

std::string firstLine(const std::string& message) {
    const std::size_t end = message.find('\n');
    return end == std::string::npos ? message : message.substr(0, end);
}

// The order git stores tree entries in: plain byte order of the name, except that
// a subdirectory compares as if its name ended in '/'. Getting this wrong produces
// a tree object git accepts but can never find entries in, so it is not cosmetic.
std::string sortKey(const std::string& name, std::uint32_t mode) {
    return mode == kDirectoryMode ? name + "/" : name;
}

std::string parentOf(const std::string& path) {
    const std::size_t slash = path.rfind('/');
    return slash == std::string::npos ? std::string() : path.substr(0, slash);
}

std::string leafOf(const std::string& path) {
    const std::size_t slash = path.rfind('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

}  // namespace

std::string kindName(ObjectKind kind) {
    switch (kind) {
        case ObjectKind::Blob:
            return "blob";
        case ObjectKind::Tree:
            return "tree";
        case ObjectKind::Commit:
            return "commit";
    }
    return "unknown";
}

bool Repo::open(const std::string& start, std::string* error) {
    std::error_code ec;
    fs::path place = fs::absolute(start, ec);
    if (ec) {
        *error = "cannot resolve " + start;
        return false;
    }
    while (!place.empty()) {
        if (fs::is_directory(place / ".git", ec)) {
            // Canonical, so that opening "." does not leave "/./" inside every
            // path built from the working tree afterwards.
            workTree_ = fs::weakly_canonical(place, ec).string();
            gitDir_ = (place / ".git").string();
            if (!loadIndex(error)) {
                return false;
            }
            return true;
        }
        if (place == place.parent_path()) {
            break;
        }
        place = place.parent_path();
    }
    *error = "no repository found in " + start + " or any folder above it";
    return false;
}

bool Repo::init(const std::string& workTree, std::string* error) {
    std::error_code ec;
    fs::create_directories(fs::path(workTree) / ".git" / "objects", ec);
    fs::create_directories(fs::path(workTree) / ".git" / "refs" / "heads", ec);
    if (ec) {
        *error = "cannot create the repository in " + workTree;
        return false;
    }
    workTree_ = fs::weakly_canonical(fs::path(workTree), ec).string();
    if (workTree_.empty()) {
        workTree_ = fs::absolute(workTree, ec).string();
    }
    gitDir_ = (fs::path(workTree_) / ".git").string();

    const fs::path head = fs::path(gitDir_) / "HEAD";
    if (!fs::exists(head, ec)) {
        if (!writeFile(head, "ref: refs/heads/main\n", error)) {
            return false;
        }
    }
    const fs::path config = fs::path(gitDir_) / "config";
    if (!fs::exists(config, ec)) {
        const std::string contents =
            "[core]\n"
            "\trepositoryformatversion = 0\n"
            "\tfilemode = false\n"
            "\tbare = false\n"
            "\tlogallrefupdates = true\n";
        if (!writeFile(config, contents, error)) {
            return false;
        }
    }
    index_ = Index();
    return true;
}

std::string Repo::absolute(const std::string& relativePath) const {
    return (fs::path(workTree_) / relativePath).string();
}

std::string Repo::relative(const std::string& absolutePath) const {
    std::error_code ec;
    const fs::path rel = fs::relative(absolutePath, workTree_, ec);
    if (ec) {
        return std::string();
    }
    const std::string text = rel.generic_string();
    return text == "." ? std::string() : text;
}

bool Repo::readRef(const std::string& name, std::string* id) const {
    const std::string text = readFile(fs::path(gitDir_) / name);
    if (text.size() < 40) {
        id->clear();
        return false;
    }
    *id = text.substr(0, 40);
    return true;
}

bool Repo::writeRef(const std::string& name, const std::string& id, std::string* error) const {
    return writeFile(fs::path(gitDir_) / name, id + "\n", error);
}

std::string Repo::branch() const {
    const std::string head = readFile(fs::path(gitDir_) / "HEAD");
    const std::string prefix = "ref: refs/heads/";
    if (head.rfind(prefix, 0) != 0) {
        return std::string();
    }
    std::string name = head.substr(prefix.size());
    while (!name.empty() && (name.back() == '\n' || name.back() == '\r')) {
        name.pop_back();
    }
    return name;
}

std::string Repo::head() const {
    const std::string text = readFile(fs::path(gitDir_) / "HEAD");
    if (text.rfind("ref: ", 0) == 0) {
        std::string name = text.substr(5);
        while (!name.empty() && (name.back() == '\n' || name.back() == '\r')) {
            name.pop_back();
        }
        std::string id;
        if (!readRef(name, &id)) {
            return std::string();  // an unborn branch: no commits yet
        }
        return id;
    }
    return text.substr(0, text.find_first_of("\r\n"));
}

bool Repo::setBranch(const std::string& name, std::string* error) {
    if (name.empty() || name.find("..") != std::string::npos || name.find(' ') != std::string::npos) {
        *error = "'" + name + "' is not a usable branch name";
        return false;
    }
    return writeFile(fs::path(gitDir_) / "HEAD", "ref: refs/heads/" + name + "\n", error);
}

bool Repo::branches(std::vector<std::string>* names, std::string* error) const {
    (void)error;  // listing a missing refs directory is an empty list, not a failure
    names->clear();
    std::error_code ec;
    const fs::path root = fs::path(gitDir_) / "refs" / "heads";
    if (!fs::exists(root, ec)) {
        return true;
    }
    for (const auto& entry : fs::recursive_directory_iterator(root, ec)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        names->push_back(fs::relative(entry.path(), root, ec).generic_string());
    }
    std::sort(names->begin(), names->end());
    return true;
}

bool Repo::readObject(const std::string& id, ObjectKind* kind, std::string* payload,
                      std::string* error) const {
    if (id.size() != 40) {
        *error = "'" + id + "' is not an object name";
        return false;
    }
    const fs::path path = fs::path(gitDir_) / "objects" / id.substr(0, 2) / id.substr(2);
    const std::string packed = readFile(path);
    if (packed.empty()) {
        *error = "object " + id + " is not in this repository (it may be inside a pack file)";
        return false;
    }
    std::string inflated;
    if (!inflateZlib(packed, &inflated, error)) {
        *error = "object " + id + ": " + *error;
        return false;
    }

    const std::size_t space = inflated.find(' ');
    const std::size_t nul = inflated.find('\0');
    if (space == std::string::npos || nul == std::string::npos || nul < space) {
        *error = "object " + id + " has no usable header";
        return false;
    }
    const std::string type = inflated.substr(0, space);
    const std::string size = inflated.substr(space + 1, nul - space - 1);
    if (type == "blob") {
        *kind = ObjectKind::Blob;
    } else if (type == "tree") {
        *kind = ObjectKind::Tree;
    } else if (type == "commit") {
        *kind = ObjectKind::Commit;
    } else {
        *error = "object " + id + " is a " + type + ", which is not read here";
        return false;
    }
    const std::string body = inflated.substr(nul + 1);
    if (std::to_string(body.size()) != size) {
        *error = "object " + id + " does not match the size in its header";
        return false;
    }
    *payload = body;
    return true;
}

bool Repo::writeObject(ObjectKind kind, const std::string& payload, std::string* id,
                       std::string* error) const {
    const std::string header = kindName(kind) + " " + std::to_string(payload.size()) + '\0';
    const std::string full = header + payload;
    *id = sha1Hex(full);

    const fs::path path = fs::path(gitDir_) / "objects" / id->substr(0, 2) / id->substr(2);
    std::error_code ec;
    if (fs::exists(path, ec)) {
        return true;  // objects are content addressed, so an existing file is correct
    }
    std::string compressed;
    if (!deflateZlibStored(full, &compressed, error)) {
        return false;
    }
    return writeFile(path, compressed, error);
}

bool Repo::readCommit(const std::string& id, Commit* commit, std::string* error) const {
    ObjectKind kind = ObjectKind::Blob;
    std::string payload;
    if (!readObject(id, &kind, &payload, error)) {
        return false;
    }
    if (kind != ObjectKind::Commit) {
        *error = id + " is a " + kindName(kind) + ", not a commit";
        return false;
    }
    *commit = Commit();
    commit->id = id;

    std::size_t lineStart = 0;
    while (lineStart < payload.size()) {
        const std::size_t end = payload.find('\n', lineStart);
        const std::string line = payload.substr(lineStart, end == std::string::npos ? std::string::npos
                                                                                   : end - lineStart);
        if (line.empty()) {
            const std::size_t body = end == std::string::npos ? payload.size() : end + 1;
            commit->message = payload.substr(body);
            break;
        }
        if (line.rfind("tree ", 0) == 0) {
            commit->tree = line.substr(5);
        } else if (line.rfind("parent ", 0) == 0) {
            commit->parents.push_back(line.substr(7));
        } else if (line.rfind("author ", 0) == 0) {
            const std::string rest = line.substr(7);
            const std::size_t angle = rest.find('>');
            if (angle != std::string::npos) {
                commit->author = rest.substr(0, angle + 1);
                commit->authorWhen = rest.substr(std::min(angle + 2, rest.size()));
            } else {
                commit->author = rest;
            }
        }
        if (end == std::string::npos) {
            break;
        }
        lineStart = end + 1;
    }
    return true;
}

bool Repo::resolve(const std::string& revision, std::string* id, std::string* error) const {
    if (revision.empty() || revision == "HEAD") {
        const std::string found = head();
        if (found.empty()) {
            *error = "HEAD does not point at a commit yet";
            return false;
        }
        *id = found;
        return true;
    }
    std::string branchRef;
    if (readRef("refs/heads/" + revision, &branchRef)) {
        *id = branchRef;
        return true;
    }
    if (looksLikeObjectId(revision)) {
        std::error_code ec;
        const fs::path objects = fs::path(gitDir_) / "objects";
        if (revision.size() == 40 &&
            fs::exists(objects / revision.substr(0, 2) / revision.substr(2), ec)) {
            *id = revision;
            return true;
        }
        // A short name is a prefix search, the same way git lets you type eight
        // characters; the first match wins only when it is unambiguous.
        std::string found;
        int matches = 0;
        for (const auto& entry : fs::directory_iterator(objects, ec)) {
            if (!entry.is_directory()) {
                continue;
            }
            const std::string prefix = entry.path().filename().string();
            if (prefix.size() != 2 || prefix.rfind(revision.substr(0, 2), 0) != 0) {
                continue;
            }
            for (const auto& object : fs::directory_iterator(entry.path(), ec)) {
                const std::string candidate = prefix + object.path().filename().string();
                if (candidate.rfind(revision, 0) == 0 && candidate.size() == 40) {
                    found = candidate;
                    ++matches;
                }
            }
        }
        if (matches == 1) {
            *id = found;
            return true;
        }
        *error = matches == 0 ? "no object matches " + revision + " here"
                              : "'" + revision + "' matches more than one object";
        return false;
    }
    *error = "cannot resolve '" + revision + "'";
    return false;
}

bool Repo::treeEntries(const std::string& treeId, std::map<std::string, TreeEntry>* entries,
                       std::string* error) const {
    entries->clear();
    ObjectKind kind = ObjectKind::Blob;
    std::string payload;
    if (!readObject(treeId, &kind, &payload, error)) {
        return false;
    }
    if (kind != ObjectKind::Tree) {
        *error = treeId + " is a " + kindName(kind) + ", not a tree";
        return false;
    }
    std::size_t offset = 0;
    while (offset < payload.size()) {
        const std::size_t space = payload.find(' ', offset);
        const std::size_t nul = payload.find('\0', offset);
        if (space == std::string::npos || nul == std::string::npos || nul + 21 > payload.size()) {
            *error = "tree " + treeId + " is malformed";
            return false;
        }
        TreeEntry entry;
        entry.mode = static_cast<std::uint32_t>(std::stoul(payload.substr(offset, space - offset), nullptr, 8));
        const std::string name = payload.substr(space + 1, nul - space - 1);
        static const char* digits = "0123456789abcdef";
        entry.id.reserve(40);
        for (int i = 0; i < 20; ++i) {
            const unsigned char byte = static_cast<unsigned char>(payload[nul + 1 + i]);
            entry.id.push_back(digits[byte >> 4]);
            entry.id.push_back(digits[byte & 0x0f]);
        }
        (*entries)[name] = entry;
        offset = nul + 21;
    }
    return true;
}

bool Repo::walkTree(const std::string& treeId, const std::string& prefix,
                    std::map<std::string, TreeEntry>* files, std::string* error) const {
    std::map<std::string, TreeEntry> entries;
    if (!treeEntries(treeId, &entries, error)) {
        return false;
    }
    for (const auto& item : entries) {
        const std::string path = prefix.empty() ? item.first : prefix + "/" + item.first;
        if (item.second.mode == kDirectoryMode) {
            if (!walkTree(item.second.id, path, files, error)) {
                return false;
            }
        } else {
            (*files)[path] = item.second;
        }
    }
    return true;
}

bool Repo::headTree(std::string* treeId, std::string* error) const {
    const std::string commitId = head();
    if (commitId.empty()) {
        treeId->clear();
        return true;
    }
    Commit commit;
    if (!readCommit(commitId, &commit, error)) {
        return false;
    }
    *treeId = commit.tree;
    return true;
}

bool Repo::filesAt(const std::string& commitId, std::map<std::string, TreeEntry>* files,
                   std::string* error) const {
    files->clear();
    if (commitId.empty()) {
        return true;
    }
    Commit commit;
    if (!readCommit(commitId, &commit, error)) {
        return false;
    }
    return walkTree(commit.tree, std::string(), files, error);
}

bool Repo::filesAtHead(std::map<std::string, TreeEntry>* files, std::string* error) const {
    return filesAt(head(), files, error);
}

bool Repo::fileAt(const std::string& commitId, const std::string& path, std::string* contents,
                  std::string* error) const {
    std::map<std::string, TreeEntry> files;
    if (!filesAt(commitId, &files, error)) {
        return false;
    }
    const auto it = files.find(path);
    if (it == files.end()) {
        *error = path + " is not in that commit";
        return false;
    }
    ObjectKind kind = ObjectKind::Blob;
    return readObject(it->second.id, &kind, contents, error);
}

bool Repo::changedPaths(const std::string& fromCommit, const std::string& toCommit,
                        std::vector<StatusEntry>* changes, std::string* error) const {
    changes->clear();
    std::map<std::string, TreeEntry> before;
    std::map<std::string, TreeEntry> after;
    if (!filesAt(fromCommit, &before, error) || !filesAt(toCommit, &after, error)) {
        return false;
    }
    std::set<std::string> paths;
    for (const auto& item : before) {
        paths.insert(item.first);
    }
    for (const auto& item : after) {
        paths.insert(item.first);
    }
    for (const std::string& path : paths) {
        const auto old = before.find(path);
        const auto now = after.find(path);
        StatusEntry entry;
        entry.path = path;
        if (old == before.end()) {
            entry.staged = 'A';
        } else if (now == after.end()) {
            entry.staged = 'D';
        } else if (old->second.id != now->second.id) {
            entry.staged = 'M';
        } else {
            continue;
        }
        changes->push_back(entry);
    }
    return true;
}

bool Repo::loadIndex(std::string* error) {
    return index_.read(indexPath(), error);
}

bool Repo::hashFile(const std::string& relativePath, std::string* id, std::uint32_t* size,
                    std::string* error) const {
    std::ifstream in(absolute(relativePath), std::ios::binary);
    if (!in) {
        *error = "cannot read " + relativePath;
        return false;
    }
    const std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    *size = static_cast<std::uint32_t>(data.size());
    return writeObject(ObjectKind::Blob, data, id, error);
}

bool Repo::addFile(const std::string& relativePath, std::string* error) {
    IndexEntry entry;
    if (!hashFile(relativePath, &entry.id, &entry.size, error)) {
        return false;
    }
    entry.path = relativePath;
    const IndexEntry* existing = index_.find(relativePath);
    entry.mode = existing != nullptr ? existing->mode : 0100644;
    index_.insert(entry);
    return true;
}

bool Repo::saveIndex(std::string* error) {
    return index_.write(indexPath(), error);
}

bool Repo::add(const std::string& given, std::string* error) {
    std::string path = given;
    std::replace(path.begin(), path.end(), '\\', '/');
    while (path.rfind("./", 0) == 0) {
        path.erase(0, 2);
    }
    if (path.empty() || path == ".") {
        return addAll(error);
    }
    // Staging only edits the index in memory, while git reads the file on disk, so
    // every staging command has to end by writing it out.
    if (!stagePath(path, error)) {
        return false;
    }
    return saveIndex(error);
}

bool Repo::stagePath(const std::string& path, std::string* error) {
    std::error_code ec;
    const fs::path target = absolute(path);
    if (!fs::exists(target, ec)) {
        // Staging a deletion is a real operation: the path stays out of the next
        // commit even though there is nothing to hash.
        if (index_.erase(path)) {
            return true;
        }
        *error = path + " does not exist";
        return false;
    }
    if (fs::is_directory(target, ec)) {
        for (const auto& item : fs::recursive_directory_iterator(target, ec)) {
            if (!item.is_regular_file() || item.path().filename() == ".git") {
                continue;
            }
            const std::string child = relative(item.path().string());
            if (child.empty() || child.rfind(".git/", 0) == 0) {
                continue;
            }
            if (!addFile(child, error)) {
                return false;
            }
        }
        return true;
    }
    return addFile(path, error);
}

bool Repo::addAll(std::string* error) {
    std::error_code ec;
    for (const auto& item : fs::recursive_directory_iterator(workTree_, ec)) {
        if (!item.is_regular_file()) {
            continue;
        }
        const std::string path = relative(item.path().string());
        if (path.empty() || path.rfind(".git/", 0) == 0) {
            continue;
        }
        if (!addFile(path, error)) {
            return false;
        }
    }
    return saveIndex(error);
}

bool Repo::commit(const std::string& message, const std::string& name, const std::string& email,
                  std::string* id, std::string* error) {
    if (message.empty()) {
        *error = "a commit needs a message";
        return false;
    }
    if (index_.empty()) {
        *error = "nothing is staged";
        return false;
    }

    // Group the index by directory so each tree object can be written once, from
    // the deepest directory up; parents cannot be serialised before their children
    // have names.
    std::map<std::string, std::vector<std::pair<std::string, TreeEntry>>> byDirectory;
    std::set<std::string> directories;
    directories.insert(std::string());
    for (const IndexEntry& entry : index_.entries()) {
        const std::string dir = parentOf(entry.path);
        byDirectory[dir].push_back({leafOf(entry.path), TreeEntry{entry.mode, entry.id}});
        for (std::string current = dir; !current.empty(); current = parentOf(current)) {
            directories.insert(current);
        }
    }

    std::map<std::string, std::string> treeIds;
    for (auto it = directories.rbegin(); it != directories.rend(); ++it) {
        const std::string& dir = *it;
        std::vector<std::pair<std::string, TreeEntry>>& children = byDirectory[dir];
        std::sort(children.begin(), children.end(),
                  [](const auto& left, const auto& right) {
                      return sortKey(left.first, left.second.mode) <
                             sortKey(right.first, right.second.mode);
                  });

        std::string payload;
        for (const auto& child : children) {
            payload += modeText(child.second.mode);
            payload.push_back(' ');
            payload += child.first;
            payload.push_back('\0');
            for (int i = 0; i < 20; ++i) {
                const unsigned value = static_cast<unsigned>(
                    std::stoul(child.second.id.substr(i * 2, 2), nullptr, 16));
                payload.push_back(static_cast<char>(value));
            }
        }
        std::string treeId;
        if (!writeObject(ObjectKind::Tree, payload, &treeId, error)) {
            return false;
        }
        treeIds[dir] = treeId;
        if (!dir.empty()) {
            byDirectory[parentOf(dir)].push_back({leafOf(dir), TreeEntry{kDirectoryMode, treeId}});
        }
    }

    const std::string tree = treeIds[std::string()];
    std::string parent;
    std::string parentTree;
    if (!head().empty()) {
        Commit previous;
        if (!readCommit(head(), &previous, error)) {
            return false;
        }
        parent = previous.id;
        parentTree = previous.tree;
    }
    if (tree == parentTree) {
        *error = "nothing to commit: the staged tree is what HEAD already has";
        return false;
    }

    const std::time_t now = std::time(nullptr);
    const std::string stamp = std::to_string(static_cast<long long>(now)) + " +0000";
    std::string payload = "tree " + tree + "\n";
    if (!parent.empty()) {
        payload += "parent " + parent + "\n";
    }
    const std::string who = name + " <" + email + ">";
    payload += "author " + who + " " + stamp + "\n";
    payload += "committer " + who + " " + stamp + "\n\n";
    payload += message;
    if (payload.back() != '\n') {
        payload.push_back('\n');
    }

    std::string commitId;
    if (!writeObject(ObjectKind::Commit, payload, &commitId, error)) {
        return false;
    }

    std::string branchName = branch();
    if (branchName.empty()) {
        branchName = "main";
        if (!setBranch(branchName, error)) {
            return false;
        }
    }
    if (!writeRef("refs/heads/" + branchName, commitId, error)) {
        return false;
    }
    appendReflog("HEAD", parent, commitId, name, email, message);
    appendReflog("refs/heads/" + branchName, parent, commitId, name, email, message);

    *id = commitId;
    return true;
}

void Repo::appendReflog(const std::string& ref, const std::string& from, const std::string& to,
                        const std::string& name, const std::string& email,
                        const std::string& message) const {
    // git treats a commit with no reflog entry as one nobody made, so the log is
    // written even though this program never reads it back.
    const fs::path path = fs::path(gitDir_) / "logs" / ref;
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::binary | std::ios::app);
    if (!out) {
        return;
    }
    const std::time_t now = std::time(nullptr);
    out << (from.empty() ? std::string(40, '0') : from) << ' ' << to << ' ' << name << " <" << email
        << "> " << static_cast<long long>(now) << " +0000\tcommit: " << firstLine(message) << '\n';
}

bool Repo::log(std::vector<Commit>* commits, std::size_t limit, std::string* error) const {
    commits->clear();
    std::string cursor = head();
    while (!cursor.empty() && commits->size() < limit) {
        Commit commit;
        if (!readCommit(cursor, &commit, error)) {
            return false;
        }
        commits->push_back(commit);
        cursor = commit.parents.empty() ? std::string() : commit.parents.front();
    }
    return true;
}

bool Repo::status(std::vector<StatusEntry>* entries, std::string* error) {
    entries->clear();
    std::map<std::string, TreeEntry> committed;
    if (!filesAtHead(&committed, error)) {
        return false;
    }

    std::set<std::string> seen;
    for (const IndexEntry& entry : index_.entries()) {
        seen.insert(entry.path);
        StatusEntry row;
        row.path = entry.path;
        const auto old = committed.find(entry.path);
        if (old == committed.end()) {
            row.staged = 'A';
        } else if (old->second.id != entry.id) {
            row.staged = 'M';
        }

        std::error_code ec;
        const fs::path file = absolute(entry.path);
        if (!fs::exists(file, ec)) {
            row.worktree = 'D';
        } else if (static_cast<std::uint32_t>(fs::file_size(file, ec)) != entry.size) {
            row.worktree = 'M';
        } else {
            std::string id;
            std::uint32_t size = 0;
            if (!hashFile(entry.path, &id, &size, error)) {
                return false;
            }
            if (id != entry.id) {
                row.worktree = 'M';
            }
        }
        if (row.staged != ' ' || row.worktree != ' ') {
            entries->push_back(row);
        }
    }

    for (const auto& item : committed) {
        if (seen.count(item.first) == 0) {
            StatusEntry row;
            row.path = item.first;
            row.staged = 'D';
            entries->push_back(row);
        }
    }

    std::error_code ec;
    for (const auto& item : fs::recursive_directory_iterator(workTree_, ec)) {
        if (!item.is_regular_file()) {
            continue;
        }
        const std::string path = relative(item.path().string());
        if (path.empty() || path.rfind(".git/", 0) == 0 || seen.count(path) != 0) {
            continue;
        }
        StatusEntry row;
        row.path = path;
        row.worktree = '?';
        entries->push_back(row);
    }

    std::sort(entries->begin(), entries->end(),
              [](const StatusEntry& left, const StatusEntry& right) { return left.path < right.path; });
    return true;
}

}  // namespace git
}  // namespace rblx
