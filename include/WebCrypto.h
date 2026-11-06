#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "WebJson.h"

namespace rblx {
namespace web {

using Digest = std::array<std::uint8_t, 32>;

Digest sha256(const std::string& data);
Digest hmacSha256(const std::string& key, const std::string& data);
std::vector<std::uint8_t> pbkdf2Sha256(const std::string& password,
                                      const std::string& salt,
                                      int iterations,
                                      std::size_t length);
bool constantTimeEquals(const std::string& a, const std::string& b);

std::string base64UrlEncode(const std::uint8_t* data, std::size_t length);
std::string base64UrlEncode(const std::vector<std::uint8_t>& data);
bool base64UrlDecode(const std::string& text, std::vector<std::uint8_t>* out);

std::string hexEncode(const std::uint8_t* data, std::size_t length);
std::string hexEncode(const Digest& digest);

std::vector<std::uint8_t> randomBytes(std::size_t length);
std::string randomHex(std::size_t byteLength);
std::string randomUuid();

// Stored as "pbkdf2-sha256$<iterations>$<salt>$<derived key>", all base64url.
std::string hashPassword(const std::string& password, int iterations = 120000);
bool verifyPassword(const std::string& password, const std::string& stored);

// HS256. `payload` must already contain the desired claims; `verifyJwt`
// rejects a bad signature, a missing signature and an expired `exp` claim.
std::string signJwt(const Json& payload, const std::string& secret);
bool verifyJwt(const std::string& token, const std::string& secret, Json* payload);

// AES-256-GCM, the cipher the data file is sealed with. `key` must be exactly 32
// bytes and `aad` is authenticated but not stored, which is how a sealed record
// is bound to the row it belongs to: moving the ciphertext to another account
// makes it fail to open instead of decrypting into the wrong place.
//
// A sealed value is "aes256gcm.<nonce>.<ciphertext and tag>", both base64url,
// with a fresh random 12-byte nonce every time. Naming the algorithm inside the
// value is what lets a second cipher appear later without rewriting the file.
// `aes256GcmOpen` returns false for a short value, a bad prefix, a failed tag
// check or a wrong key, and writes nothing to `out` unless it returns true.
std::string aes256GcmSeal(const std::string& key, const std::string& plaintext,
                          const std::string& aad);
bool aes256GcmOpen(const std::string& key, const std::string& sealed, const std::string& aad,
                   std::string* out);

}  // namespace web
}  // namespace rblx
