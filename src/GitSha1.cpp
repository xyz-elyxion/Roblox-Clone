#include "GitSha1.h"

#include <cstring>

namespace rblx {
namespace git {
namespace {

std::uint32_t rotateLeft(std::uint32_t value, int bits) {
    return (value << bits) | (value >> (32 - bits));
}

}  // namespace

Sha1::Sha1() {
    state_[0] = 0x67452301u;
    state_[1] = 0xEFCDAB89u;
    state_[2] = 0x98BADCFEu;
    state_[3] = 0x10325476u;
    state_[4] = 0xC3D2E1F0u;
}

void Sha1::block(const unsigned char* data) {
    std::uint32_t words[80];
    for (int i = 0; i < 16; ++i) {
        words[i] = (static_cast<std::uint32_t>(data[i * 4]) << 24) |
                   (static_cast<std::uint32_t>(data[i * 4 + 1]) << 16) |
                   (static_cast<std::uint32_t>(data[i * 4 + 2]) << 8) |
                   static_cast<std::uint32_t>(data[i * 4 + 3]);
    }
    for (int i = 16; i < 80; ++i) {
        words[i] = rotateLeft(words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16], 1);
    }

    std::uint32_t a = state_[0];
    std::uint32_t b = state_[1];
    std::uint32_t c = state_[2];
    std::uint32_t d = state_[3];
    std::uint32_t e = state_[4];

    for (int i = 0; i < 80; ++i) {
        std::uint32_t f = 0;
        std::uint32_t k = 0;
        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999u;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1u;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDCu;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6u;
        }
        const std::uint32_t temp = rotateLeft(a, 5) + f + e + k + words[i];
        e = d;
        d = c;
        c = rotateLeft(b, 30);
        b = a;
        a = temp;
    }

    state_[0] += a;
    state_[1] += b;
    state_[2] += c;
    state_[3] += d;
    state_[4] += e;
}

void Sha1::update(const void* data, std::size_t size) {
    if (finished_ || size == 0) {
        return;
    }
    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    total_ += size;
    while (size > 0) {
        const std::size_t room = 64 - buffered_;
        const std::size_t take = size < room ? size : room;
        std::memcpy(buffer_ + buffered_, bytes, take);
        buffered_ += take;
        bytes += take;
        size -= take;
        if (buffered_ == 64) {
            block(buffer_);
            buffered_ = 0;
        }
    }
}

void Sha1::finish(unsigned char out[20]) {
    if (finished_) {
        std::memcpy(out, digest_, 20);
        return;
    }
    // The length is appended in bits, big-endian, after a 1 bit and enough zero
    // padding to leave room for it - the padding is part of the standard, so it
    // cannot be skipped when the message is already aligned.
    const std::uint64_t bits = total_ * 8;
    const unsigned char one = 0x80;
    update(&one, 1);
    const unsigned char zero = 0;
    while (buffered_ != 56) {
        update(&zero, 1);
    }
    unsigned char length[8];
    for (int i = 0; i < 8; ++i) {
        length[i] = static_cast<unsigned char>((bits >> (56 - i * 8)) & 0xff);
    }
    // update() refuses to work once finished_, which it is not yet, so the length
    // goes in through the block path directly.
    std::memcpy(buffer_ + buffered_, length, 8);
    block(buffer_);
    buffered_ = 0;

    for (int i = 0; i < 5; ++i) {
        digest_[i * 4] = static_cast<unsigned char>((state_[i] >> 24) & 0xff);
        digest_[i * 4 + 1] = static_cast<unsigned char>((state_[i] >> 16) & 0xff);
        digest_[i * 4 + 2] = static_cast<unsigned char>((state_[i] >> 8) & 0xff);
        digest_[i * 4 + 3] = static_cast<unsigned char>(state_[i] & 0xff);
    }
    finished_ = true;
    std::memcpy(out, digest_, 20);
}

std::string Sha1::hex() {
    unsigned char digest[20];
    finish(digest);
    static const char* digits = "0123456789abcdef";
    std::string out;
    out.reserve(40);
    for (unsigned char byte : digest) {
        out.push_back(digits[byte >> 4]);
        out.push_back(digits[byte & 0x0f]);
    }
    return out;
}

std::string sha1Hex(const void* data, std::size_t size) {
    Sha1 hash;
    hash.update(data, size);
    return hash.hex();
}

std::string sha1Hex(const std::string& data) {
    return sha1Hex(data.data(), data.size());
}

}  // namespace git
}  // namespace rblx
