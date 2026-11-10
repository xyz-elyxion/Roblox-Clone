#include "GitIndex.h"

#include "GitSha1.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>

namespace rblx {
namespace git {
namespace {

const std::size_t kEntryFixed = 62;  // ten 32-bit fields, the digest and the flags

std::uint32_t readU32(const unsigned char* bytes) {
    return (static_cast<std::uint32_t>(bytes[0]) << 24) |
           (static_cast<std::uint32_t>(bytes[1]) << 16) |
           (static_cast<std::uint32_t>(bytes[2]) << 8) | static_cast<std::uint32_t>(bytes[3]);
}

void appendU32(std::string* out, std::uint32_t value) {
    out->push_back(static_cast<char>((value >> 24) & 0xff));
    out->push_back(static_cast<char>((value >> 16) & 0xff));
    out->push_back(static_cast<char>((value >> 8) & 0xff));
    out->push_back(static_cast<char>(value & 0xff));
}

std::string hexOf(const unsigned char* bytes) {
    static const char* digits = "0123456789abcdef";
    std::string out;
    out.reserve(40);
    for (int i = 0; i < 20; ++i) {
        out.push_back(digits[bytes[i] >> 4]);
        out.push_back(digits[bytes[i] & 0x0f]);
    }
    return out;
}

bool bytesOf(const std::string& hex, unsigned char out[20]) {
    if (hex.size() != 40) {
        return false;
    }
    for (int i = 0; i < 20; ++i) {
        unsigned value = 0;
        if (std::sscanf(hex.c_str() + i * 2, "%2x", &value) != 1) {
            return false;
        }
        out[i] = static_cast<unsigned char>(value);
    }
    return true;
}

}  // namespace

bool Index::read(const std::string& path, std::string* error) {
    entries_.clear();
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        // A repository with no index is a repository with nothing staged, which is
        // the state `git init` leaves behind, so this is not an error.
        return true;
    }
    std::string raw((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(raw.data());
    if (raw.size() < 12 || std::memcmp(bytes, "DIRC", 4) != 0) {
        *error = "the index does not start with DIRC";
        return false;
    }
    const std::uint32_t version = readU32(bytes + 4);
    if (version != 2 && version != 3) {
        // Version 4 compresses path names against their neighbours; refusing it is
        // honest where pretending to understand it would lose files.
        *error = "the index is version " + std::to_string(version) + ", which is not read here";
        return false;
    }
    const std::uint32_t count = readU32(bytes + 8);

    std::size_t offset = 12;
    for (std::uint32_t i = 0; i < count; ++i) {
        if (offset + kEntryFixed > raw.size()) {
            *error = "the index ends in the middle of an entry";
            return false;
        }
        const unsigned char* entry = bytes + offset;
        IndexEntry parsed;
        parsed.mode = readU32(entry + 24);
        parsed.size = readU32(entry + 36);
        parsed.id = hexOf(entry + 40);
        const std::uint16_t flags = static_cast<std::uint16_t>((entry[60] << 8) | entry[61]);

        std::size_t cursor = offset + kEntryFixed;
        if (version == 3 && (flags & 0x4000) != 0) {
            cursor += 2;  // extended flags; nothing in them matters here
        }
        if (cursor >= raw.size()) {
            *error = "the index ends in the middle of a path";
            return false;
        }
        const char* name = raw.c_str() + cursor;
        const std::size_t maximum = raw.size() - cursor;
        const std::size_t length = strnlen(name, maximum);
        if (length == maximum) {
            *error = "the index has a path with no terminator";
            return false;
        }
        parsed.path.assign(name, length);
        entries_.push_back(parsed);

        const std::size_t used = kEntryFixed + length;
        offset += used + (8 - (used % 8));
    }

    if (raw.size() >= offset + 20) {
        const std::string computed = sha1Hex(raw.data(), raw.size() - 20);
        if (computed != hexOf(bytes + raw.size() - 20)) {
            *error = "the index digest does not match its contents";
            entries_.clear();
            return false;
        }
    }
    return true;
}

bool Index::write(const std::string& path, std::string* error) const {
    std::string out;
    out.append("DIRC", 4);
    appendU32(&out, 2);
    appendU32(&out, static_cast<std::uint32_t>(entries_.size()));

    for (const IndexEntry& entry : entries_) {
        const std::size_t start = out.size();
        appendU32(&out, 0);                  // ctime seconds
        appendU32(&out, 0);                  // ctime nanoseconds
        appendU32(&out, 0);                  // mtime seconds
        appendU32(&out, 0);                  // mtime nanoseconds
        appendU32(&out, 0);                  // device
        appendU32(&out, 0);                  // inode
        appendU32(&out, entry.mode);
        appendU32(&out, 0);                  // uid
        appendU32(&out, 0);                  // gid
        appendU32(&out, entry.size);

        unsigned char digest[20];
        if (!bytesOf(entry.id, digest)) {
            *error = "the index entry for " + entry.path + " has an unusable object name";
            return false;
        }
        out.append(reinterpret_cast<const char*>(digest), 20);

        const std::uint16_t flags = static_cast<std::uint16_t>(
            std::min<std::size_t>(entry.path.size(), 0x0fff));
        out.push_back(static_cast<char>((flags >> 8) & 0xff));
        out.push_back(static_cast<char>(flags & 0xff));
        out.append(entry.path);

        const std::size_t used = out.size() - start;
        out.append(8 - (used % 8), '\0');
    }

    const std::string digest = sha1Hex(out);
    unsigned char raw[20];
    if (!bytesOf(digest, raw)) {
        *error = "cannot digest the index";
        return false;
    }
    out.append(reinterpret_cast<const char*>(raw), 20);

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        *error = "cannot write " + path;
        return false;
    }
    file.write(out.data(), static_cast<std::streamsize>(out.size()));
    return static_cast<bool>(file);
}

const IndexEntry* Index::find(const std::string& path) const {
    const auto it = std::lower_bound(entries_.begin(), entries_.end(), path,
                                     [](const IndexEntry& entry, const std::string& key) {
                                         return entry.path < key;
                                     });
    if (it == entries_.end() || it->path != path) {
        return nullptr;
    }
    return &*it;
}

void Index::insert(const IndexEntry& entry) {
    const auto it = std::lower_bound(entries_.begin(), entries_.end(), entry.path,
                                     [](const IndexEntry& existing, const std::string& key) {
                                         return existing.path < key;
                                     });
    if (it != entries_.end() && it->path == entry.path) {
        *it = entry;
    } else {
        entries_.insert(it, entry);
    }
}

bool Index::erase(const std::string& path) {
    const auto it = std::lower_bound(entries_.begin(), entries_.end(), path,
                                     [](const IndexEntry& entry, const std::string& key) {
                                         return entry.path < key;
                                     });
    if (it == entries_.end() || it->path != path) {
        return false;
    }
    entries_.erase(it);
    return true;
}

}  // namespace git
}  // namespace rblx
