#include "WebCrypto.h"

// NOMINMAX is defined for every target in RBLXBuildLists.txt; defining it here
// again is what MSVC reports as a macro redefinition.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <bcrypt.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ctime>

namespace rblx {
namespace web {
namespace {

const std::uint32_t kSha256InitialState[8] = {
    0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
    0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
};

const std::uint32_t kSha256RoundConstants[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u,
    0x923f82a4u, 0xab1c5ed5u, 0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u, 0xe49b69c1u, 0xefbe4786u,
    0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u,
    0x06ca6351u, 0x14292967u, 0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u, 0xa2bfe8a1u, 0xa81a664bu,
    0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au,
    0x5b9cca4fu, 0x682e6ff3u, 0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

inline std::uint32_t rotateRight(std::uint32_t value, int bits) {
    return (value >> bits) | (value << (32 - bits));
}

void sha256Transform(std::uint32_t state[8], const std::uint8_t block[64]) {
    std::uint32_t schedule[64];
    for (int i = 0; i < 16; ++i) {
        schedule[i] = (static_cast<std::uint32_t>(block[i * 4]) << 24) |
                      (static_cast<std::uint32_t>(block[i * 4 + 1]) << 16) |
                      (static_cast<std::uint32_t>(block[i * 4 + 2]) << 8) |
                      static_cast<std::uint32_t>(block[i * 4 + 3]);
    }
    for (int i = 16; i < 64; ++i) {
        const std::uint32_t s0 = rotateRight(schedule[i - 15], 7) ^
                                 rotateRight(schedule[i - 15], 18) ^ (schedule[i - 15] >> 3);
        const std::uint32_t s1 = rotateRight(schedule[i - 2], 17) ^
                                 rotateRight(schedule[i - 2], 19) ^ (schedule[i - 2] >> 10);
        schedule[i] = schedule[i - 16] + s0 + schedule[i - 7] + s1;
    }

    std::uint32_t a = state[0];
    std::uint32_t b = state[1];
    std::uint32_t c = state[2];
    std::uint32_t d = state[3];
    std::uint32_t e = state[4];
    std::uint32_t f = state[5];
    std::uint32_t g = state[6];
    std::uint32_t h = state[7];

    for (int i = 0; i < 64; ++i) {
        const std::uint32_t sum1 = rotateRight(e, 6) ^ rotateRight(e, 11) ^ rotateRight(e, 25);
        const std::uint32_t choice = (e & f) ^ ((~e) & g);
        const std::uint32_t temp1 = h + sum1 + choice + kSha256RoundConstants[i] + schedule[i];
        const std::uint32_t sum0 = rotateRight(a, 2) ^ rotateRight(a, 13) ^ rotateRight(a, 22);
        const std::uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
        const std::uint32_t temp2 = sum0 + majority;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

const char kBase64UrlAlphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

int base64UrlValue(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 26;
    }
    if (c >= '0' && c <= '9') {
        return c - '0' + 52;
    }
    if (c == '-') {
        return 62;
    }
    if (c == '_') {
        return 63;
    }
    return -1;
}

}  // namespace

Digest sha256(const std::string& data) {
    std::uint32_t state[8];
    std::memcpy(state, kSha256InitialState, sizeof(state));

    const std::size_t totalLength = data.size();
    const std::size_t fullBlocks = totalLength / 64;
    for (std::size_t block = 0; block < fullBlocks; ++block) {
        sha256Transform(state, reinterpret_cast<const std::uint8_t*>(data.data()) + block * 64);
    }

    std::uint8_t tail[128];
    std::memset(tail, 0, sizeof(tail));
    const std::size_t remainder = totalLength % 64;
    if (remainder > 0) {
        std::memcpy(tail, data.data() + fullBlocks * 64, remainder);
    }
    tail[remainder] = 0x80;
    const std::size_t tailLength = remainder < 56 ? 64 : 128;
    const std::uint64_t bitLength = static_cast<std::uint64_t>(totalLength) * 8u;
    for (int i = 0; i < 8; ++i) {
        tail[tailLength - 1 - static_cast<std::size_t>(i)] =
            static_cast<std::uint8_t>((bitLength >> (8 * i)) & 0xFFu);
    }
    for (std::size_t offset = 0; offset < tailLength; offset += 64) {
        sha256Transform(state, tail + offset);
    }

    Digest digest{};
    for (int i = 0; i < 8; ++i) {
        digest[static_cast<std::size_t>(i) * 4] = static_cast<std::uint8_t>((state[i] >> 24) & 0xFFu);
        digest[static_cast<std::size_t>(i) * 4 + 1] = static_cast<std::uint8_t>((state[i] >> 16) & 0xFFu);
        digest[static_cast<std::size_t>(i) * 4 + 2] = static_cast<std::uint8_t>((state[i] >> 8) & 0xFFu);
        digest[static_cast<std::size_t>(i) * 4 + 3] = static_cast<std::uint8_t>(state[i] & 0xFFu);
    }
    return digest;
}

Digest hmacSha256(const std::string& key, const std::string& data) {
    std::uint8_t block[64];
    std::memset(block, 0, sizeof(block));
    if (key.size() > sizeof(block)) {
        const Digest hashedKey = sha256(key);
        std::memcpy(block, hashedKey.data(), hashedKey.size());
    } else if (!key.empty()) {
        std::memcpy(block, key.data(), key.size());
    }

    std::uint8_t innerPad[64];
    std::uint8_t outerPad[64];
    for (std::size_t i = 0; i < sizeof(block); ++i) {
        innerPad[i] = static_cast<std::uint8_t>(block[i] ^ 0x36u);
        outerPad[i] = static_cast<std::uint8_t>(block[i] ^ 0x5Cu);
    }

    std::string inner;
    inner.reserve(sizeof(innerPad) + data.size());
    inner.append(reinterpret_cast<const char*>(innerPad), sizeof(innerPad));
    inner += data;
    const Digest innerDigest = sha256(inner);

    std::string outer;
    outer.reserve(sizeof(outerPad) + innerDigest.size());
    outer.append(reinterpret_cast<const char*>(outerPad), sizeof(outerPad));
    outer.append(reinterpret_cast<const char*>(innerDigest.data()), innerDigest.size());
    return sha256(outer);
}

std::vector<std::uint8_t> pbkdf2Sha256(const std::string& password,
                                      const std::string& salt,
                                      int iterations,
                                      std::size_t length) {
    std::vector<std::uint8_t> derived;
    if (iterations < 1 || length == 0) {
        return derived;
    }
    derived.reserve(length);

    std::uint32_t blockIndex = 1;
    while (derived.size() < length) {
        std::string saltBlock = salt;
        saltBlock.push_back(static_cast<char>((blockIndex >> 24) & 0xFFu));
        saltBlock.push_back(static_cast<char>((blockIndex >> 16) & 0xFFu));
        saltBlock.push_back(static_cast<char>((blockIndex >> 8) & 0xFFu));
        saltBlock.push_back(static_cast<char>(blockIndex & 0xFFu));

        Digest current = hmacSha256(password, saltBlock);
        Digest accumulator = current;
        for (int round = 1; round < iterations; ++round) {
            current = hmacSha256(password,
                                 std::string(reinterpret_cast<const char*>(current.data()),
                                             current.size()));
            for (std::size_t i = 0; i < accumulator.size(); ++i) {
                accumulator[i] = static_cast<std::uint8_t>(accumulator[i] ^ current[i]);
            }
        }

        const std::size_t take = std::min(accumulator.size(), length - derived.size());
        derived.insert(derived.end(), accumulator.begin(),
                       accumulator.begin() + static_cast<std::ptrdiff_t>(take));
        ++blockIndex;
    }
    return derived;
}

bool constantTimeEquals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }
    unsigned char difference = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        difference = static_cast<unsigned char>(
            difference | (static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i])));
    }
    return difference == 0;
}

std::string base64UrlEncode(const std::uint8_t* data, std::size_t length) {
    std::string out;
    out.reserve(((length + 2) / 3) * 4);
    std::size_t i = 0;
    while (i + 3 <= length) {
        const std::uint32_t chunk = (static_cast<std::uint32_t>(data[i]) << 16) |
                                    (static_cast<std::uint32_t>(data[i + 1]) << 8) |
                                    static_cast<std::uint32_t>(data[i + 2]);
        out.push_back(kBase64UrlAlphabet[(chunk >> 18) & 0x3Fu]);
        out.push_back(kBase64UrlAlphabet[(chunk >> 12) & 0x3Fu]);
        out.push_back(kBase64UrlAlphabet[(chunk >> 6) & 0x3Fu]);
        out.push_back(kBase64UrlAlphabet[chunk & 0x3Fu]);
        i += 3;
    }
    if (length - i == 1) {
        const std::uint32_t chunk = static_cast<std::uint32_t>(data[i]) << 16;
        out.push_back(kBase64UrlAlphabet[(chunk >> 18) & 0x3Fu]);
        out.push_back(kBase64UrlAlphabet[(chunk >> 12) & 0x3Fu]);
    } else if (length - i == 2) {
        const std::uint32_t chunk = (static_cast<std::uint32_t>(data[i]) << 16) |
                                    (static_cast<std::uint32_t>(data[i + 1]) << 8);
        out.push_back(kBase64UrlAlphabet[(chunk >> 18) & 0x3Fu]);
        out.push_back(kBase64UrlAlphabet[(chunk >> 12) & 0x3Fu]);
        out.push_back(kBase64UrlAlphabet[(chunk >> 6) & 0x3Fu]);
    }
    return out;
}

std::string base64UrlEncode(const std::vector<std::uint8_t>& data) {
    return base64UrlEncode(data.data(), data.size());
}

bool base64UrlDecode(const std::string& text, std::vector<std::uint8_t>* out) {
    if (!out) {
        return false;
    }
    out->clear();
    std::uint32_t accumulator = 0;
    int bits = 0;
    for (const char c : text) {
        if (c == '=') {
            break;
        }
        const int value = base64UrlValue(c);
        if (value < 0) {
            return false;
        }
        accumulator = (accumulator << 6) | static_cast<std::uint32_t>(value);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out->push_back(static_cast<std::uint8_t>((accumulator >> bits) & 0xFFu));
        }
    }
    return true;
}

std::string hexEncode(const std::uint8_t* data, std::size_t length) {
    static const char kDigits[] = "0123456789abcdef";
    std::string out;
    out.reserve(length * 2);
    for (std::size_t i = 0; i < length; ++i) {
        out.push_back(kDigits[(data[i] >> 4) & 0x0Fu]);
        out.push_back(kDigits[data[i] & 0x0Fu]);
    }
    return out;
}

std::string hexEncode(const Digest& digest) {
    return hexEncode(digest.data(), digest.size());
}

std::vector<std::uint8_t> randomBytes(std::size_t length) {
    std::vector<std::uint8_t> bytes(length);
    if (bytes.empty()) {
        return bytes;
    }
    // The system generator, because these bytes become recovery codes, developer
    // tokens, game-instance tokens and AES-GCM nonces. A seeded engine is wrong
    // here twice over: its state can be recovered from its own output, and on a
    // toolchain whose std::random_device is not a device at all (libstdc++ on
    // MinGW has no /dev/urandom) the seed collapses to the clock.
    const NTSTATUS status =
        ::BCryptGenRandom(nullptr, bytes.data(), static_cast<ULONG>(bytes.size()),
                          BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (!BCRYPT_SUCCESS(status)) {
        // Fail closed: an empty result cannot be mistaken for a valid token, and
        // falling back to a predictable stream would be worse than refusing.
        bytes.clear();
    }
    return bytes;
}

std::string randomHex(std::size_t byteLength) {
    const std::vector<std::uint8_t> bytes = randomBytes(byteLength);
    return hexEncode(bytes.data(), bytes.size());
}

std::string randomUuid() {
    std::vector<std::uint8_t> bytes = randomBytes(16);
    bytes[6] = static_cast<std::uint8_t>((bytes[6] & 0x0Fu) | 0x40u);
    bytes[8] = static_cast<std::uint8_t>((bytes[8] & 0x3Fu) | 0x80u);
    const std::string hex = hexEncode(bytes.data(), bytes.size());
    return hex.substr(0, 8) + "-" + hex.substr(8, 4) + "-" + hex.substr(12, 4) + "-" +
           hex.substr(16, 4) + "-" + hex.substr(20, 12);
}

std::string hashPassword(const std::string& password, int iterations) {
    const int rounds = iterations > 0 ? iterations : 120000;
    const std::vector<std::uint8_t> salt = randomBytes(16);
    const std::vector<std::uint8_t> key =
        pbkdf2Sha256(password, std::string(reinterpret_cast<const char*>(salt.data()), salt.size()),
                     rounds, 32);
    return "pbkdf2-sha256$" + std::to_string(rounds) + "$" + base64UrlEncode(salt) + "$" +
           base64UrlEncode(key);
}

bool verifyPassword(const std::string& password, const std::string& stored) {
    const std::string prefix = "pbkdf2-sha256$";
    if (stored.compare(0, prefix.size(), prefix) != 0) {
        return false;
    }

    const std::size_t roundsEnd = stored.find('$', prefix.size());
    if (roundsEnd == std::string::npos) {
        return false;
    }
    const std::size_t saltEnd = stored.find('$', roundsEnd + 1);
    if (saltEnd == std::string::npos) {
        return false;
    }

    int rounds = 0;
    try {
        rounds = std::stoi(stored.substr(prefix.size(), roundsEnd - prefix.size()));
    } catch (...) {
        return false;
    }
    if (rounds < 1 || rounds > 10000000) {
        return false;
    }

    std::vector<std::uint8_t> salt;
    std::vector<std::uint8_t> expected;
    if (!base64UrlDecode(stored.substr(roundsEnd + 1, saltEnd - roundsEnd - 1), &salt) ||
        !base64UrlDecode(stored.substr(saltEnd + 1), &expected) || expected.empty()) {
        return false;
    }

    const std::vector<std::uint8_t> actual =
        pbkdf2Sha256(password, std::string(reinterpret_cast<const char*>(salt.data()), salt.size()),
                     rounds, expected.size());
    if (actual.size() != expected.size()) {
        return false;
    }
    unsigned char difference = 0;
    for (std::size_t i = 0; i < actual.size(); ++i) {
        difference = static_cast<unsigned char>(difference | (actual[i] ^ expected[i]));
    }
    return difference == 0;
}

std::string signJwt(const Json& payload, const std::string& secret) {
    Json header = Json::makeObject();
    header.set("alg", "HS256");
    header.set("typ", "JWT");

    const std::string encodedHeader =
        base64UrlEncode(reinterpret_cast<const std::uint8_t*>(header.dump().data()),
                        header.dump().size());
    const std::string serializedPayload = payload.dump();
    const std::string encodedPayload =
        base64UrlEncode(reinterpret_cast<const std::uint8_t*>(serializedPayload.data()),
                        serializedPayload.size());

    const std::string signingInput = encodedHeader + "." + encodedPayload;
    const Digest signature = hmacSha256(secret, signingInput);
    return signingInput + "." + base64UrlEncode(signature.data(), signature.size());
}

bool verifyJwt(const std::string& token, const std::string& secret, Json* payload) {
    const std::size_t firstDot = token.find('.');
    if (firstDot == std::string::npos) {
        return false;
    }
    const std::size_t secondDot = token.find('.', firstDot + 1);
    if (secondDot == std::string::npos) {
        return false;
    }
    if (token.find('.', secondDot + 1) != std::string::npos) {
        return false;
    }

    const std::string signingInput = token.substr(0, secondDot);
    std::vector<std::uint8_t> providedSignature;
    if (!base64UrlDecode(token.substr(secondDot + 1), &providedSignature)) {
        return false;
    }

    const Digest expected = hmacSha256(secret, signingInput);
    if (providedSignature.size() != expected.size()) {
        return false;
    }
    if (!constantTimeEquals(std::string(reinterpret_cast<const char*>(providedSignature.data()),
                                        providedSignature.size()),
                            std::string(reinterpret_cast<const char*>(expected.data()),
                                        expected.size()))) {
        return false;
    }

    std::vector<std::uint8_t> headerBytes;
    if (!base64UrlDecode(token.substr(0, firstDot), &headerBytes)) {
        return false;
    }
    std::string parseError;
    const Json header = Json::parse(
        std::string(reinterpret_cast<const char*>(headerBytes.data()), headerBytes.size()),
        &parseError);
    if (!header.isObject() || header.member("alg").asString() != "HS256") {
        return false;
    }

    std::vector<std::uint8_t> payloadBytes;
    if (!base64UrlDecode(token.substr(firstDot + 1, secondDot - firstDot - 1), &payloadBytes)) {
        return false;
    }
    const Json claims = Json::parse(
        std::string(reinterpret_cast<const char*>(payloadBytes.data()), payloadBytes.size()),
        &parseError);
    if (!claims.isObject()) {
        return false;
    }

    if (claims.has("exp")) {
        const long long expiresAt = claims.member("exp").asInteger(0);
        const long long now = static_cast<long long>(std::time(nullptr));
        if (expiresAt <= now) {
            return false;
        }
    }

    if (payload) {
        *payload = claims;
    }
    return true;
}

namespace {

// AES-256 as FIPS-197 defines it, forward direction only. GCM never runs the
// cipher backwards: it encrypts a counter for the data and the hash key for the
// tag, so the inverse cipher and its tables are not built at all.
const std::uint8_t kAesSbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// Round constants for the key schedule, one per ten-word block of AES-256.
const std::uint8_t kAesRcon[8] = { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40 };

// 15 round keys of 16 bytes for a 256-bit key (Nr = 14).
struct Aes256Key {
    std::uint8_t roundKeys[240];
};

std::uint8_t aesXtime(std::uint8_t value) {
    return static_cast<std::uint8_t>((value << 1) ^ ((value & 0x80) ? 0x1b : 0x00));
}

void aes256ExpandKey(const std::uint8_t key[32], Aes256Key* expanded) {
    std::memcpy(expanded->roundKeys, key, 32);

    int generated = 32;
    while (generated < 240) {
        std::uint8_t word[4];
        std::memcpy(word, expanded->roundKeys + generated - 4, 4);

        if (generated % 32 == 0) {
            const std::uint8_t first = word[0];
            word[0] = static_cast<std::uint8_t>(kAesSbox[word[1]] ^ kAesRcon[generated / 32]);
            word[1] = kAesSbox[word[2]];
            word[2] = kAesSbox[word[3]];
            word[3] = kAesSbox[first];
        } else if (generated % 32 == 16) {
            for (int i = 0; i < 4; ++i) {
                word[i] = kAesSbox[word[i]];
            }
        }

        for (int i = 0; i < 4; ++i) {
            expanded->roundKeys[generated] =
                static_cast<std::uint8_t>(expanded->roundKeys[generated - 32] ^ word[i]);
            ++generated;
        }
    }
}

void aesAddRoundKey(std::uint8_t state[16], const std::uint8_t* roundKey) {
    for (int i = 0; i < 16; ++i) {
        state[i] = static_cast<std::uint8_t>(state[i] ^ roundKey[i]);
    }
}

void aesSubBytes(std::uint8_t state[16]) {
    for (int i = 0; i < 16; ++i) {
        state[i] = kAesSbox[state[i]];
    }
}

// Row r of the state rotates left by r. The state is column-major, so byte
// (column * 4 + row) moves to ((column + row) % 4) * 4 + row.
void aesShiftRows(std::uint8_t state[16]) {
    std::uint8_t copy[16];
    std::memcpy(copy, state, 16);
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            state[column * 4 + row] = copy[((column + row) % 4) * 4 + row];
        }
    }
}

void aesMixColumns(std::uint8_t state[16]) {
    for (int column = 0; column < 4; ++column) {
        std::uint8_t* s = state + column * 4;
        const std::uint8_t a0 = s[0];
        const std::uint8_t a1 = s[1];
        const std::uint8_t a2 = s[2];
        const std::uint8_t a3 = s[3];
        s[0] = static_cast<std::uint8_t>(aesXtime(a0) ^ (aesXtime(a1) ^ a1) ^ a2 ^ a3);
        s[1] = static_cast<std::uint8_t>(a0 ^ aesXtime(a1) ^ (aesXtime(a2) ^ a2) ^ a3);
        s[2] = static_cast<std::uint8_t>(a0 ^ a1 ^ aesXtime(a2) ^ (aesXtime(a3) ^ a3));
        s[3] = static_cast<std::uint8_t>((aesXtime(a0) ^ a0) ^ a1 ^ a2 ^ aesXtime(a3));
    }
}

void aes256EncryptBlock(const Aes256Key& key, const std::uint8_t input[16],
                        std::uint8_t output[16]) {
    std::uint8_t state[16];
    std::memcpy(state, input, 16);

    aesAddRoundKey(state, key.roundKeys);
    for (int round = 1; round < 14; ++round) {
        aesSubBytes(state);
        aesShiftRows(state);
        aesMixColumns(state);
        aesAddRoundKey(state, key.roundKeys + round * 16);
    }
    aesSubBytes(state);
    aesShiftRows(state);
    aesAddRoundKey(state, key.roundKeys + 14 * 16);

    std::memcpy(output, state, 16);
}

// One GHASH step: y = (y xor block) * H in GF(2^128). The field is walked one
// bit at a time here rather than with a precomputed table, because a table would
// have to be built per key and the data file is small enough that the simpler
// version costs nothing measurable.
void ghashBlock(std::uint8_t y[16], const std::uint8_t h[16], const std::uint8_t block[16]) {
    std::uint8_t v[16];
    std::uint8_t product[16];
    std::memcpy(v, h, 16);
    std::memset(product, 0, 16);

    for (int byte = 0; byte < 16; ++byte) {
        const std::uint8_t value = static_cast<std::uint8_t>(y[byte] ^ block[byte]);
        for (int bit = 0; bit < 8; ++bit) {
            if (value & (0x80 >> bit)) {
                for (int i = 0; i < 16; ++i) {
                    product[i] = static_cast<std::uint8_t>(product[i] ^ v[i]);
                }
            }
            const bool carry = (v[15] & 0x01) != 0;
            for (int i = 15; i > 0; --i) {
                v[i] = static_cast<std::uint8_t>((v[i] >> 1) | ((v[i - 1] & 0x01) << 7));
            }
            v[0] = static_cast<std::uint8_t>(v[0] >> 1);
            if (carry) {
                v[0] = static_cast<std::uint8_t>(v[0] ^ 0xe1);
            }
        }
    }

    std::memcpy(y, product, 16);
}

void ghashAbsorb(std::uint8_t y[16], const std::uint8_t h[16], const std::uint8_t* data,
                 std::size_t length) {
    std::size_t offset = 0;
    while (offset + 16 <= length) {
        ghashBlock(y, h, data + offset);
        offset += 16;
    }
    if (offset < length) {
        // The last partial block is zero padded, as GCM specifies.
        std::uint8_t tail[16] = { 0 };
        std::memcpy(tail, data + offset, length - offset);
        ghashBlock(y, h, tail);
    }
}

void gcmIncrementCounter(std::uint8_t counter[16]) {
    for (int i = 15; i >= 12; --i) {
        if (++counter[i] != 0) {
            break;
        }
    }
}

// The GCM data pass: the same keystream serves encryption and decryption.
void gcmCounterCrypt(const Aes256Key& key, const std::uint8_t j0[16], const std::uint8_t* input,
                     std::size_t length, std::uint8_t* output) {
    std::uint8_t counter[16];
    std::uint8_t stream[16];
    std::memcpy(counter, j0, 16);

    for (std::size_t offset = 0; offset < length; offset += 16) {
        gcmIncrementCounter(counter);
        aes256EncryptBlock(key, counter, stream);
        const std::size_t chunk = (length - offset) < 16 ? (length - offset) : 16;
        for (std::size_t i = 0; i < chunk; ++i) {
            output[offset + i] = static_cast<std::uint8_t>(input[offset + i] ^ stream[i]);
        }
    }
}

void gcmTag(const Aes256Key& key, const std::uint8_t j0[16], const std::uint8_t h[16],
            const std::string& aad, const std::uint8_t* ciphertext, std::size_t length,
            std::uint8_t tag[16]) {
    std::uint8_t y[16] = { 0 };
    ghashAbsorb(y, h, reinterpret_cast<const std::uint8_t*>(aad.data()), aad.size());
    ghashAbsorb(y, h, ciphertext, length);

    // The final block is both lengths in bits, big-endian 64-bit each.
    std::uint8_t lengths[16];
    const std::uint64_t aadBits = static_cast<std::uint64_t>(aad.size()) * 8;
    const std::uint64_t textBits = static_cast<std::uint64_t>(length) * 8;
    for (int i = 0; i < 8; ++i) {
        lengths[i] = static_cast<std::uint8_t>((aadBits >> (56 - 8 * i)) & 0xff);
        lengths[8 + i] = static_cast<std::uint8_t>((textBits >> (56 - 8 * i)) & 0xff);
    }
    ghashBlock(y, h, lengths);

    std::uint8_t mask[16];
    aes256EncryptBlock(key, j0, mask);
    for (int i = 0; i < 16; ++i) {
        tag[i] = static_cast<std::uint8_t>(mask[i] ^ y[i]);
    }
}

}  // namespace

std::string aes256GcmSeal(const std::string& key, const std::string& plaintext,
                          const std::string& aad) {
    if (key.size() != 32) {
        return std::string();
    }

    std::uint8_t keyBytes[32];
    std::memcpy(keyBytes, key.data(), 32);
    Aes256Key expanded;
    aes256ExpandKey(keyBytes, &expanded);

    const std::vector<std::uint8_t> nonce = randomBytes(12);
    std::uint8_t j0[16];
    std::memcpy(j0, nonce.data(), 12);
    j0[12] = 0;
    j0[13] = 0;
    j0[14] = 0;
    j0[15] = 1;

    std::uint8_t hashKey[16];
    const std::uint8_t zeros[16] = { 0 };
    aes256EncryptBlock(expanded, zeros, hashKey);

    std::vector<std::uint8_t> ciphertext(plaintext.size());
    gcmCounterCrypt(expanded, j0,
                    reinterpret_cast<const std::uint8_t*>(plaintext.data()), plaintext.size(),
                    ciphertext.data());

    std::uint8_t tag[16];
    gcmTag(expanded, j0, hashKey, aad, ciphertext.data(), ciphertext.size(), tag);
    ciphertext.insert(ciphertext.end(), tag, tag + 16);

    return "aes256gcm." + base64UrlEncode(nonce) + "." + base64UrlEncode(ciphertext);
}

bool aes256GcmOpen(const std::string& key, const std::string& sealed, const std::string& aad,
                   std::string* out) {
    if (key.size() != 32) {
        return false;
    }

    const std::string prefix = "aes256gcm.";
    if (sealed.size() <= prefix.size() || sealed.compare(0, prefix.size(), prefix) != 0) {
        return false;
    }
    const std::size_t separator = sealed.find('.', prefix.size());
    if (separator == std::string::npos) {
        return false;
    }

    std::vector<std::uint8_t> nonce;
    std::vector<std::uint8_t> payload;
    if (!base64UrlDecode(sealed.substr(prefix.size(), separator - prefix.size()), &nonce) ||
        !base64UrlDecode(sealed.substr(separator + 1), &payload)) {
        return false;
    }
    if (nonce.size() != 12 || payload.size() < 16) {
        return false;
    }

    std::uint8_t keyBytes[32];
    std::memcpy(keyBytes, key.data(), 32);
    Aes256Key expanded;
    aes256ExpandKey(keyBytes, &expanded);

    std::uint8_t j0[16];
    std::memcpy(j0, nonce.data(), 12);
    j0[12] = 0;
    j0[13] = 0;
    j0[14] = 0;
    j0[15] = 1;

    std::uint8_t hashKey[16];
    const std::uint8_t zeros[16] = { 0 };
    aes256EncryptBlock(expanded, zeros, hashKey);

    const std::size_t textLength = payload.size() - 16;
    std::uint8_t expected[16];
    gcmTag(expanded, j0, hashKey, aad, payload.data(), textLength, expected);

    // Compared without an early exit, so a caller cannot learn how much of the tag
    // it got right. A tag mismatch means the value was changed or the key is
    // wrong; either way nothing is decrypted.
    std::uint8_t difference = 0;
    for (int i = 0; i < 16; ++i) {
        difference = static_cast<std::uint8_t>(difference | (expected[i] ^ payload[textLength + i]));
    }
    if (difference != 0) {
        return false;
    }

    std::vector<std::uint8_t> plaintext(textLength);
    gcmCounterCrypt(expanded, j0, payload.data(), textLength, plaintext.data());
    if (out) {
        out->assign(reinterpret_cast<const char*>(plaintext.data()), plaintext.size());
    }
    return true;
}

}  // namespace web
}  // namespace rblx
