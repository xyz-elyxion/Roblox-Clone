#include "ReleaseSignature.h"

#include "WebCrypto.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace rblx {
namespace update {
namespace {

// Limbs are 16 bits wide because a 16x16 product plus a carry fits in a 64-bit
// accumulator on every compiler this project ships for. A 2048-bit modulus is
// 128 limbs; the extra two hold the carries CIOS needs while it reduces.
using Limb = std::uint16_t;
const std::uint32_t kLimbBase = 0x10000u;
const std::size_t kModulusBits = 2048;
const std::size_t kModulusBytes = kModulusBits / 8;
const std::size_t kModulusLimbs = kModulusBytes / 2;

std::vector<Limb> toLimbs(const std::string& hex, std::string* error) {
    std::string digits = hex;
    if (!digits.empty() && digits.size() % 2 != 0) {
        digits.insert(digits.begin(), '0');
    }
    if (digits.empty()) {
        if (error) *error = "the number is empty";
        return std::vector<Limb>();
    }

    std::vector<Limb> limbs;
    limbs.reserve(digits.size() / 4 + 1);
    // Big-endian hex becomes little-endian limbs, which is the order every
    // multiply below expects.
    for (std::size_t end = digits.size(); end > 0;) {
        const std::size_t start = end >= 4 ? end - 4 : 0;
        unsigned value = 0;
        for (std::size_t i = start; i < end; ++i) {
            const char c = digits[i];
            unsigned digit = 0;
            if (c >= '0' && c <= '9') {
                digit = static_cast<unsigned>(c - '0');
            } else if (c >= 'a' && c <= 'f') {
                digit = static_cast<unsigned>(c - 'a' + 10);
            } else if (c >= 'A' && c <= 'F') {
                digit = static_cast<unsigned>(c - 'A' + 10);
            } else {
                if (error) *error = "the number is not hexadecimal";
                return std::vector<Limb>();
            }
            value = (value << 4) | digit;
        }
        limbs.push_back(static_cast<Limb>(value & 0xffffu));
        end = start;
    }
    while (!limbs.empty() && limbs.back() == 0) {
        limbs.pop_back();
    }
    return limbs;
}

std::string fromLimbs(const std::vector<Limb>& limbs, std::size_t byteCount) {
    std::string out(byteCount, '\0');
    for (std::size_t i = 0; i < byteCount; ++i) {
        const std::size_t limb = i / 2;
        const Limb value = limb < limbs.size() ? limbs[limb] : static_cast<Limb>(0);
        out[byteCount - 1 - i] =
            static_cast<char>(i % 2 == 0 ? (value & 0xffu) : ((value >> 8) & 0xffu));
    }
    return out;
}

int compare(const std::vector<Limb>& a, const std::vector<Limb>& b, std::size_t length) {
    for (std::size_t i = length; i-- > 0;) {
        const Limb left = i < a.size() ? a[i] : static_cast<Limb>(0);
        const Limb right = i < b.size() ? b[i] : static_cast<Limb>(0);
        if (left != right) {
            return left < right ? -1 : 1;
        }
    }
    return 0;
}

// a -= b over `length` limbs; the caller has already established a >= b.
void subtract(std::vector<Limb>* a, const std::vector<Limb>& b, std::size_t length) {
    std::int64_t borrow = 0;
    for (std::size_t i = 0; i < length; ++i) {
        const std::int64_t right = i < b.size() ? static_cast<std::int64_t>(b[i]) : 0;
        std::int64_t value = static_cast<std::int64_t>((*a)[i]) - right - borrow;
        if (value < 0) {
            value += static_cast<std::int64_t>(kLimbBase);
            borrow = 1;
        } else {
            borrow = 0;
        }
        (*a)[i] = static_cast<Limb>(value);
    }
}

// Doubling modulo n, used to build R mod n and R^2 mod n at startup of every
// exponentiation. It keeps the multiplication below free of a division.
void doubleModulo(std::vector<Limb>* value, const std::vector<Limb>& modulus) {
    std::uint32_t carry = 0;
    for (std::size_t i = 0; i < value->size(); ++i) {
        const std::uint32_t doubled = static_cast<std::uint32_t>((*value)[i]) * 2u + carry;
        (*value)[i] = static_cast<Limb>(doubled & 0xffffu);
        carry = doubled >> 16;
    }
    if (carry != 0 || compare(*value, modulus, modulus.size()) >= 0) {
        subtract(value, modulus, modulus.size());
    }
}

struct Montgomery {
    std::vector<Limb> modulus;
    // -n^-1 mod 2^16, which makes the low limb cancel on every reduction step.
    Limb negativeInverse = 0;
    // R mod n and R^2 mod n, with R = 2^2048.
    std::vector<Limb> rModulo;
    std::vector<Limb> rSquared;
    bool ready = false;
};

bool prepare(const std::string& modulusHex, Montgomery* context, std::string* error) {
    std::vector<Limb> modulus = toLimbs(modulusHex, error);
    if (modulus.empty()) {
        return false;
    }
    if (modulus.size() > kModulusLimbs) {
        if (error) *error = "the modulus is larger than 2048 bits";
        return false;
    }
    if ((modulus[0] & 1u) == 0) {
        if (error) *error = "the modulus is even";
        return false;
    }
    modulus.resize(kModulusLimbs, 0);
    if (modulus.back() == 0) {
        if (error) *error = "the modulus is not 2048 bits";
        return false;
    }

    std::uint32_t inverse = 1;
    for (int i = 0; i < 5; ++i) {
        inverse = inverse * (2u - static_cast<std::uint32_t>(modulus[0]) * inverse);
    }
    context->modulus = modulus;
    context->negativeInverse =
        static_cast<Limb>((kLimbBase - (inverse & 0xffffu)) & 0xffffu);

    std::vector<Limb> value(kModulusLimbs, 0);
    value[0] = 1;
    for (std::size_t i = 0; i < kModulusBits; ++i) {
        doubleModulo(&value, modulus);
    }
    context->rModulo = value;
    for (std::size_t i = 0; i < kModulusBits; ++i) {
        doubleModulo(&value, modulus);
    }
    context->rSquared = value;
    context->ready = true;
    return true;
}

// Montgomery multiplication (CIOS): a*b*R^-1 mod n, with a and b in Montgomery
// form and the result in Montgomery form.
std::vector<Limb> multiply(const Montgomery& context, const std::vector<Limb>& a,
                           const std::vector<Limb>& b) {
    const std::size_t k = kModulusLimbs;
    std::vector<Limb> t(k + 2, 0);

    for (std::size_t i = 0; i < k; ++i) {
        std::uint64_t carry = 0;
        for (std::size_t j = 0; j < k; ++j) {
            const std::uint64_t sum = t[j] +
                                      static_cast<std::uint64_t>(a[j]) * b[i] + carry;
            t[j] = static_cast<Limb>(sum & 0xffffu);
            carry = sum >> 16;
        }
        std::uint64_t sum = t[k] + carry;
        t[k] = static_cast<Limb>(sum & 0xffffu);
        t[k + 1] = static_cast<Limb>(t[k + 1] + (sum >> 16));

        const Limb factor =
            static_cast<Limb>((t[0] * context.negativeInverse) & 0xffffu);
        carry = 0;
        for (std::size_t j = 0; j < k; ++j) {
            const std::uint64_t reduced = t[j] +
                                          static_cast<std::uint64_t>(factor) *
                                              context.modulus[j] + carry;
            t[j] = static_cast<Limb>(reduced & 0xffffu);
            carry = reduced >> 16;
        }
        sum = t[k] + carry;
        t[k] = static_cast<Limb>(sum & 0xffffu);
        t[k + 1] = static_cast<Limb>(t[k + 1] + (sum >> 16));

        // The low limb is now zero; dropping it is the division by 2^16.
        std::memmove(&t[0], &t[1], k * sizeof(Limb));
        t[k] = t[k + 1];
        t[k + 1] = 0;
    }

    std::vector<Limb> result(t.begin(), t.begin() + static_cast<std::ptrdiff_t>(k));
    if (t[k] != 0 || compare(result, context.modulus, k) >= 0) {
        subtract(&result, context.modulus, k);
    }
    return result;
}

std::vector<Limb> toMontgomery(const Montgomery& context, const std::vector<Limb>& value) {
    return multiply(context, value, context.rSquared);
}

std::vector<Limb> fromMontgomery(const Montgomery& context, const std::vector<Limb>& value) {
    std::vector<Limb> one(kModulusLimbs, 0);
    one[0] = 1;
    return multiply(context, value, one);
}

// base^exponent mod n. The exponent is only ever 65537 or a private exponent, so
// a plain square-and-multiply is the right amount of machinery.
std::vector<Limb> power(const Montgomery& context, const std::vector<Limb>& base,
                        const std::vector<Limb>& exponent) {
    std::vector<Limb> accumulator(kModulusLimbs, 0);
    accumulator[0] = 1;
    accumulator = multiply(context, accumulator, context.rSquared);

    for (std::size_t i = exponent.size(); i-- > 0;) {
        for (int bit = 15; bit >= 0; --bit) {
            accumulator = multiply(context, accumulator, accumulator);
            if (((exponent[i] >> bit) & 1u) != 0) {
                accumulator = multiply(context, accumulator, base);
            }
        }
    }
    return fromMontgomery(context, accumulator);
}

std::vector<Limb> fromBytes(const std::string& bytes) {
    std::vector<Limb> limbs;
    limbs.reserve(kModulusLimbs);
    for (std::size_t end = bytes.size(); end > 0;) {
        const std::size_t start = end >= 2 ? end - 2 : 0;
        unsigned value = 0;
        for (std::size_t i = start; i < end; ++i) {
            value = (value << 8) | static_cast<unsigned char>(bytes[i]);
        }
        limbs.push_back(static_cast<Limb>(value));
        end = start;
    }
    return limbs;
}

// The DER prefix of a SHA-256 DigestInfo, which is what PKCS#1 v1.5 signs.
const unsigned char kSha256DigestInfo[] = {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86,
                                           0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05,
                                           0x00, 0x04, 0x20};

// 0x00 0x01 0xff... 0x00 || DigestInfo || digest, exactly k bytes long.
std::string encodeForSignature(const std::string& message) {
    const std::string digest = web::hexEncode(web::sha256(message));
    std::string digestBytes;
    digestBytes.reserve(32);
    for (std::size_t i = 0; i + 1 < digest.size(); i += 2) {
        const auto value = static_cast<unsigned char>(
            std::stoul(digest.substr(i, 2), nullptr, 16));
        digestBytes.push_back(static_cast<char>(value));
    }

    const std::size_t trailer = sizeof(kSha256DigestInfo) + digestBytes.size();
    std::string encoded(kModulusBytes, '\0');
    encoded[0] = '\0';
    encoded[1] = '\x01';
    const std::size_t padding = kModulusBytes - trailer - 3;
    std::fill(encoded.begin() + 2, encoded.begin() + 2 + static_cast<std::ptrdiff_t>(padding),
              '\xff');
    encoded[2 + padding] = '\0';
    std::memcpy(&encoded[3 + padding], kSha256DigestInfo, sizeof(kSha256DigestInfo));
    std::memcpy(&encoded[3 + padding + sizeof(kSha256DigestInfo)], digestBytes.data(),
                digestBytes.size());
    return encoded;
}

}  // namespace

std::string base64Encode(const std::string& bytes) {
    static const char kAlphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((bytes.size() + 2) / 3) * 4);
    std::size_t i = 0;
    while (i + 3 <= bytes.size()) {
        const std::uint32_t chunk =
            (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[i])) << 16) |
            (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[i + 1])) << 8) |
            static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[i + 2]));
        out.push_back(kAlphabet[(chunk >> 18) & 0x3fu]);
        out.push_back(kAlphabet[(chunk >> 12) & 0x3fu]);
        out.push_back(kAlphabet[(chunk >> 6) & 0x3fu]);
        out.push_back(kAlphabet[chunk & 0x3fu]);
        i += 3;
    }
    if (bytes.size() - i == 1) {
        const std::uint32_t chunk =
            static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[i])) << 16;
        out.push_back(kAlphabet[(chunk >> 18) & 0x3fu]);
        out.push_back(kAlphabet[(chunk >> 12) & 0x3fu]);
        out.push_back('=');
        out.push_back('=');
    } else if (bytes.size() - i == 2) {
        const std::uint32_t chunk =
            (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[i])) << 16) |
            (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[i + 1])) << 8);
        out.push_back(kAlphabet[(chunk >> 18) & 0x3fu]);
        out.push_back(kAlphabet[(chunk >> 12) & 0x3fu]);
        out.push_back(kAlphabet[(chunk >> 6) & 0x3fu]);
        out.push_back('=');
    }
    return out;
}

bool base64Decode(const std::string& text, std::string* bytes, std::string* error) {
    if (!bytes) {
        if (error) *error = "nowhere to put the decoded bytes";
        return false;
    }
    bytes->clear();

    auto valueOf = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+' || c == '-') return 62;
        if (c == '/' || c == '_') return 63;
        return -1;
    };

    std::uint32_t accumulator = 0;
    int bits = 0;
    for (const char c : text) {
        if (c == '=' || c == '\n' || c == '\r' || c == ' ' || c == '\t') {
            continue;
        }
        const int value = valueOf(c);
        if (value < 0) {
            if (error) *error = "the signature is not base64";
            return false;
        }
        accumulator = (accumulator << 6) | static_cast<std::uint32_t>(value);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            bytes->push_back(static_cast<char>((accumulator >> bits) & 0xffu));
        }
    }
    return true;
}

bool isValidRsaKey(const std::string& modulusHex, const std::string& exponentHex,
                   std::string* error) {
    std::string reason;
    std::vector<Limb> modulus = toLimbs(modulusHex, &reason);
    if (modulus.empty()) {
        if (error) *error = "the release key: " + reason;
        return false;
    }
    if (modulus.size() != kModulusLimbs || (modulus.back() & 0x8000u) == 0) {
        if (error) *error = "the release key is not a 2048-bit modulus";
        return false;
    }
    if ((modulus[0] & 1u) == 0) {
        if (error) *error = "the release key modulus is even";
        return false;
    }

    std::vector<Limb> exponent = toLimbs(exponentHex, &reason);
    if (exponent.empty()) {
        if (error) *error = "the release key exponent: " + reason;
        return false;
    }
    if (exponent.size() > 2 || exponent.back() == 0) {
        if (error) *error = "the release key exponent is out of range";
        return false;
    }
    const std::uint32_t value = exponent.size() == 2
                                    ? (static_cast<std::uint32_t>(exponent[1]) << 16) | exponent[0]
                                    : exponent[0];
    if ((value & 1u) == 0 || value < 3) {
        if (error) *error = "the release key exponent must be odd and at least 3";
        return false;
    }
    return true;
}

bool verifyRsaSha256(const void* message, std::size_t length,
                     const std::string& signatureBase64,
                     const std::string& modulusHex, const std::string& exponentHex,
                     std::string* error) {
    if (!isValidRsaKey(modulusHex, exponentHex, error)) {
        return false;
    }

    std::string signature;
    if (!base64Decode(signatureBase64, &signature, error)) {
        return false;
    }
    if (signature.size() != kModulusBytes) {
        if (error) *error = "the signature is not 2048 bits";
        return false;
    }

    Montgomery context;
    if (!prepare(modulusHex, &context, error)) {
        return false;
    }
    const std::vector<Limb> modulus = context.modulus;
    const std::vector<Limb> value = fromBytes(signature);
    if (compare(value, modulus, kModulusLimbs) >= 0) {
        if (error) *error = "the signature is not smaller than the modulus";
        return false;
    }

    std::vector<Limb> exponent = toLimbs(exponentHex, error);
    if (exponent.empty()) {
        return false;
    }
    exponent.resize(kModulusLimbs, 0);

    std::string signedMessage;
    const std::vector<Limb> recovered = power(context, toMontgomery(context, value), exponent);
    signedMessage = fromLimbs(recovered, kModulusBytes);

    const std::string expected =
        encodeForSignature(std::string(static_cast<const char*>(message), length));
    if (signedMessage.size() != expected.size()) {
        if (error) *error = "the signature does not describe this manifest";
        return false;
    }
    unsigned char difference = 0;
    for (std::size_t i = 0; i < expected.size(); ++i) {
        difference = static_cast<unsigned char>(
            difference | (static_cast<unsigned char>(signedMessage[i]) ^
                          static_cast<unsigned char>(expected[i])));
    }
    if (difference != 0) {
        if (error) *error = "the signature does not match the release manifest";
        return false;
    }
    return true;
}

bool signRsaSha256(const void* message, std::size_t length,
                   const std::string& modulusHex, const std::string& privateExponentHex,
                   std::string* signatureBase64, std::string* error) {
    if (!signatureBase64) {
        if (error) *error = "nowhere to put the signature";
        return false;
    }

    Montgomery context;
    if (!prepare(modulusHex, &context, error)) {
        return false;
    }
    std::vector<Limb> privateExponent = toLimbs(privateExponentHex, error);
    if (privateExponent.empty()) {
        return false;
    }
    if (privateExponent.size() > kModulusLimbs) {
        if (error) *error = "the private exponent is larger than the modulus";
        return false;
    }
    privateExponent.resize(kModulusLimbs, 0);

    // The padded digest is smaller than the modulus, so it goes into Montgomery
    // form as-is and comes back as (EM^d mod n) bytes.
    const std::string encoded =
        encodeForSignature(std::string(static_cast<const char*>(message), length));
    const std::vector<Limb> value = fromBytes(encoded);
    const std::vector<Limb> result =
        power(context, toMontgomery(context, value), privateExponent);
    *signatureBase64 = base64Encode(fromLimbs(result, kModulusBytes));
    if (error) error->clear();
    return true;
}

}  // namespace update
}  // namespace rblx
