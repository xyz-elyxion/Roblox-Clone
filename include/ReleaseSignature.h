#pragma once

#include <cstddef>
#include <string>

// Release signing. The updater talks plain HTTP because this project has no TLS
// stack, so the manifest it downloads is only as trustworthy as the key that
// signed it: without that, whoever answers the request decides what gets written
// into the install folder and launched from it.
//
// The scheme is RSA-2048 with PKCS#1 v1.5 and SHA-256, which every packaging
// machine can already produce (OpenSSL on any host, .NET on Windows) and which
// needs no dependency here. Keys are written as lowercase or uppercase hex so a
// key can live in update.json, a file that belongs to the machine and that an
// update never rewrites.
//
// Verification fails closed: a missing key, a short key, a malformed signature
// or a digest that does not match all return false with a reason.
namespace rblx {
namespace update {

// True when `modulusHex` is a 2048-bit odd modulus with its top bit set and
// `exponentHex` is an odd exponent greater than one. The error says which part
// did not hold.
bool isValidRsaKey(const std::string& modulusHex, const std::string& exponentHex,
                   std::string* error);

// Verifies a base64 PKCS#1 v1.5 SHA-256 signature over `length` bytes at
// `message`. Returns false with a reason for every failure.
bool verifyRsaSha256(const void* message, std::size_t length,
                     const std::string& signatureBase64,
                     const std::string& modulusHex,
                     const std::string& exponentHex,
                     std::string* error);

// Produces the signature the packaging step writes into manifest.json. Only the
// private exponent is needed, which is why a signing key is a file on the
// release machine and never part of this repository.
bool signRsaSha256(const void* message, std::size_t length,
                   const std::string& modulusHex,
                   const std::string& privateExponentHex,
                   std::string* signatureBase64,
                   std::string* error);

// Standard base64, as `openssl dgst -sign ... | base64` produces. The URL-safe
// alphabet is accepted too, so a key or signature copied out of JSON works
// either way.
std::string base64Encode(const std::string& bytes);
bool base64Decode(const std::string& text, std::string* bytes, std::string* error);

}  // namespace update
}  // namespace rblx
