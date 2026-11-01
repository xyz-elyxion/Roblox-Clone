#pragma once

// Captchas for the account endpoints, in two flavours.
//
// Human: four digits drawn from a 5x7 bitmap font into an SVG data URI. The
// answer exists in the image as pixels, not as text, so a script cannot read it
// back out of the markup the way it could with an SVG <text> element.
//
// Machine: SHA-256 proof of work. The client finds a nonce whose digest starts
// with `difficulty` hex zeros. That is trivial for a program and useless to a
// person, which is exactly the case the desktop client's publish login needs.
//
// Header-only and free of sockets, so the same code runs in the website backend
// and in the native clients that have to solve the machine challenge.

#include <array>
#include <cctype>
#include <cstdint>
#include <string>
#include <vector>

#include "WebCrypto.h"

namespace rblx {
namespace web {

inline constexpr int kCaptchaDigits = 4;
// Every extra hex zero multiplies the work by 16. Four is a fraction of a second
// for the C++ client and still a real gate for a script that has to hash.
inline constexpr int kCaptchaPowDifficulty = 4;
inline constexpr long long kCaptchaTtlMs = 5 * 60 * 1000;

// '1' is an inked pixel. Ten digits is all the human challenge needs, which
// keeps the font small enough to keep beside the code that draws it.
inline const std::array<std::array<const char*, 7>, 10>& captchaDigitGlyphs() {
    static const std::array<std::array<const char*, 7>, 10> glyphs = {{
        {{"01110", "10001", "10011", "10101", "11001", "10001", "01110"}},  // 0
        {{"00100", "01100", "00100", "00100", "00100", "00100", "01110"}},  // 1
        {{"01110", "10001", "00001", "00010", "00100", "01000", "11111"}},  // 2
        {{"01110", "10001", "00001", "00110", "00001", "10001", "01110"}},  // 3
        {{"00010", "00110", "01010", "10010", "11111", "00010", "00010"}},  // 4
        {{"11111", "10000", "11110", "00001", "00001", "10001", "01110"}},  // 5
        {{"00110", "01000", "10000", "11110", "10001", "10001", "01110"}},  // 6
        {{"11111", "00001", "00010", "00100", "01000", "01000", "01000"}},  // 7
        {{"01110", "10001", "10001", "01110", "10001", "10001", "01110"}},  // 8
        {{"01110", "10001", "10001", "01111", "00001", "00010", "01100"}},  // 9
    }};
    return glyphs;
}

inline std::string makeCaptchaCode() {
    const std::vector<std::uint8_t> bytes = randomBytes(kCaptchaDigits);
    std::string code;
    code.reserve(kCaptchaDigits);
    for (std::uint8_t byte : bytes) {
        code.push_back(static_cast<char>('0' + (byte % 10)));
    }
    return code;
}

inline std::string renderHumanCaptchaSvg(const std::string& code) {
    const int cell = 5;        // pixels per glyph pixel
    const int glyphWidth = 5 * cell;
    const int gap = 10;
    const int padX = 16;
    const int height = 64;
    const int count = static_cast<int>(code.size());
    const int width = padX * 2 + count * glyphWidth + (count > 0 ? (count - 1) * gap : 0);

    std::uint64_t seed = 0;
    for (std::uint8_t byte : randomBytes(8)) {
        seed = (seed << 8) | byte;
    }
    if (seed == 0) {
        seed = 0x9e3779b97f4a7c15ULL;
    }
    // xorshift64: the jitter only has to look random, and this keeps the renderer
    // free of any dependency beyond the standard library.
    const auto next = [&seed]() {
        seed ^= seed << 13;
        seed ^= seed >> 7;
        seed ^= seed << 17;
        return seed;
    };
    const auto range = [&next](int low, int high) {
        return low + static_cast<int>(next() % static_cast<std::uint64_t>(high - low + 1));
    };
    const std::array<const char*, 3> colors = {"#26303a", "#3a2a26", "#243a2c"};

    std::string svg;
    svg.reserve(4096);
    svg += "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" + std::to_string(width) +
           "\" height=\"" + std::to_string(height) + "\" viewBox=\"0 0 " + std::to_string(width) +
           " " + std::to_string(height) + "\"><rect width=\"" + std::to_string(width) +
           "\" height=\"" + std::to_string(height) + "\" fill=\"#f5f2ec\"/>";

    // Noise goes underneath so the digits stay readable through it.
    for (int i = 0; i < 5; ++i) {
        svg += "<line x1=\"" + std::to_string(range(0, width)) + "\" y1=\"" +
               std::to_string(range(0, height)) + "\" x2=\"" + std::to_string(range(0, width)) +
               "\" y2=\"" + std::to_string(range(0, height)) + "\" stroke=\"#b9b2a4\" stroke-width=\"" +
               std::to_string(range(1, 2)) + "\"/>";
    }

    for (int index = 0; index < count; ++index) {
        const int digit = code[static_cast<std::size_t>(index)] - '0';
        if (digit < 0 || digit > 9) {
            continue;
        }
        const auto& glyph = captchaDigitGlyphs()[static_cast<std::size_t>(digit)];
        const int x0 = padX + index * (glyphWidth + gap) + range(-2, 2);
        const int y0 = 12 + range(-4, 4);
        const int centerX = x0 + glyphWidth / 2;
        const int centerY = y0 + (7 * cell) / 2;
        svg += "<g transform=\"rotate(" + std::to_string(range(-11, 11)) + " " +
               std::to_string(centerX) + " " + std::to_string(centerY) + ")\" fill=\"" +
               colors[next() % colors.size()] + "\">";
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if (glyph[static_cast<std::size_t>(row)][col] != '1') {
                    continue;
                }
                svg += "<rect x=\"" + std::to_string(x0 + col * cell) + "\" y=\"" +
                       std::to_string(y0 + row * cell) + "\" width=\"" + std::to_string(cell) +
                       "\" height=\"" + std::to_string(cell) + "\"/>";
            }
        }
        svg += "</g>";
    }

    // A few dots over the glyphs, so a simple threshold pass cannot read the
    // character cells straight off the image.
    for (int i = 0; i < 40; ++i) {
        svg += "<circle cx=\"" + std::to_string(range(0, width)) + "\" cy=\"" +
               std::to_string(range(0, height)) + "\" r=\"" + std::to_string(range(1, 2)) +
               "\" fill=\"#8d8778\"/>";
    }
    svg += "</svg>";
    return svg;
}

// Percent-encoded so the markup can be used directly as an <img> source; '#' and
// '%' only mean something to a URL parser, so everything outside the unreserved
// set is escaped instead of picking a safe subset by hand.
inline std::string captchaImageDataUri(const std::string& svg) {
    static const char* hex = "0123456789ABCDEF";
    std::string uri = "data:image/svg+xml;charset=utf-8,";
    uri.reserve(svg.size() * 3);
    for (unsigned char c : svg) {
        const bool unreserved = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                                (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' ||
                                c == '~';
        if (unreserved) {
            uri.push_back(static_cast<char>(c));
        } else {
            uri.push_back('%');
            uri.push_back(hex[c >> 4]);
            uri.push_back(hex[c & 0x0f]);
        }
    }
    return uri;
}

// Typing "1 2 3 4" or "12-34" still answers the same challenge; anything that is
// not a digit is dropped rather than treated as a wrong answer.
inline std::string normalizeCaptchaAnswer(const std::string& input) {
    std::string out;
    for (char c : input) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            out.push_back(c);
        }
    }
    return out;
}

inline bool captchaAnswerMatches(const std::string& expected, const std::string& input) {
    if (expected.empty()) {
        return false;
    }
    return constantTimeEquals(normalizeCaptchaAnswer(input), expected);
}

// The machine challenge: sha256("<challenge>:<nonce>") must start with
// `difficulty` hex zeros. The nonce is decimal so both ends build the same string.
inline bool verifyCaptchaPow(const std::string& challenge, const std::string& nonce,
                             int difficulty) {
    if (challenge.empty() || nonce.empty() || nonce.size() > 24 || difficulty <= 0 ||
        difficulty > 8) {
        return false;
    }
    for (char c : nonce) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    const std::string digest = hexEncode(sha256(challenge + ":" + nonce));
    for (int i = 0; i < difficulty; ++i) {
        if (digest[static_cast<std::size_t>(i)] != '0') {
            return false;
        }
    }
    return true;
}

inline bool solveCaptchaPow(const std::string& challenge, int difficulty, std::string* nonce) {
    if (nonce == nullptr || difficulty <= 0) {
        return false;
    }
    // Four hex zeros averages about 65k hashes; the ceiling is only here so a
    // larger difficulty cannot spin forever in a client that cannot reach it.
    constexpr long long kMaxAttempts = 40000000;
    for (long long attempt = 0; attempt < kMaxAttempts; ++attempt) {
        const std::string candidate = std::to_string(attempt);
        if (verifyCaptchaPow(challenge, candidate, difficulty)) {
            *nonce = candidate;
            return true;
        }
    }
    return false;
}

}  // namespace web
}  // namespace rblx
