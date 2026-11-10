#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace rblx {
namespace git {

// A git object name is the SHA-1 of the object header and its bytes, so the
// digest is part of the on-disk format: an object stored under the wrong name is
// invisible to git, not merely unverified.
class Sha1 {
public:
    Sha1();

    void update(const void* data, std::size_t size);
    // Reads the digest. Calling update afterwards has no effect, because a hash
    // that kept absorbing bytes would name something nobody can find.
    void finish(unsigned char out[20]);

    std::string hex();

private:
    void block(const unsigned char* data);

    std::uint32_t state_[5];
    unsigned char buffer_[64];
    std::size_t buffered_ = 0;
    std::uint64_t total_ = 0;
    bool finished_ = false;
    unsigned char digest_[20] = {};
};

std::string sha1Hex(const void* data, std::size_t size);
std::string sha1Hex(const std::string& data);

}  // namespace git
}  // namespace rblx
