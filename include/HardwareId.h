#pragma once

// A machine's handle for the ban list.
//
// An account ban only stops the account, and a new account costs a minute. What
// a player cannot change as easily is the computer it was banned on, so the
// client hashes what identifies that machine and sends the hash with the join.
// The site records the hash on the account, copies it onto the ban, and refuses
// a join from a barred machine - with any account, or none at all.
//
// The client is the one being watched, so a modified client can send whatever
// hash it likes; this raises the price of coming back from "make an account" to
// "edit the identifiers the hash is built from", not from zero to impossible. The
// identifiers themselves never leave the process, and the digest is taken over a
// versioned prefix so the scheme can be changed without colliding with the old
// one.
//
// Kept free of platform headers and of the digest itself: the website and the
// game server only ever validate a hash, and neither should have to link the
// crypto or see windows.h to do it.

#include <cstddef>
#include <string>

namespace rblx {
namespace hwid {

// Hex characters kept from the digest: 128 bits tells two machines apart and fits
// one packet field and one dashboard column.
constexpr std::size_t kHardwareIdLength = 32;

// Lowercase, keeping only [0-9a-f]. A hash that arrives with a stray space or in
// capitals is still the same machine on the next join, and a client cannot smuggle
// anything else into a stored id.
inline std::string normalize(const std::string& value) {
    std::string out;
    out.reserve(kHardwareIdLength);
    for (char c : value) {
        if (c >= 'A' && c <= 'F') {
            out.push_back(static_cast<char>(c - 'A' + 'a'));
        } else if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) {
            out.push_back(c);
        }
        if (out.size() >= kHardwareIdLength) {
            break;
        }
    }
    return out;
}

// Exactly kHardwareIdLength lowercase hex characters, which is what the client
// produces. Anything else is refused rather than stored, so an old client or a
// hand-written request cannot put a placeholder in the ban list.
inline bool valid(const std::string& value) {
    if (value.size() != kHardwareIdLength) {
        return false;
    }
    for (char c : value) {
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
            return false;
        }
    }
    return true;
}

// The digest of the identifier string, separated from gathering it so the hash can
// be checked without a machine underneath. Defined in src/HardwareId.cpp.
std::string hashInputs(const std::string& inputs);

// This machine's id: the digest of what identifies it, computed once because a
// join can be retried and the answer cannot change while the process runs.
std::string machineId();

}  // namespace hwid
}  // namespace rblx
