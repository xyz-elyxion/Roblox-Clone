#include "WebCrypto.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <random>

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

std::string randomDeviceSeed() {
    std::random_device device;
    std::string seed;
    seed.reserve(64);
    for (int i = 0; i < 8; ++i) {
        seed += std::to_string(static_cast<unsigned long long>(device()));
    }
    return seed;
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
    static std::mt19937_64 generator([]() {
        std::uint64_t seed = static_cast<std::uint64_t>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count());
        const std::string deviceSeed = randomDeviceSeed();
        for (const char c : deviceSeed) {
            seed = seed * 1099511628211ull + static_cast<unsigned char>(c);
        }
        std::random_device device;
        seed ^= static_cast<std::uint64_t>(device()) << 17;
        return seed;
    }());

    std::vector<std::uint8_t> bytes(length);
    for (std::size_t i = 0; i < length; ++i) {
        bytes[i] = static_cast<std::uint8_t>(generator() & 0xFFu);
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

}  // namespace web
}  // namespace rblx
