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

}  // namespace web
}  // namespace rblx
