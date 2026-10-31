#pragma once

#include <array>
#include <cstddef>
#include <string>

// String obfuscation for the few identifiers that would otherwise point a
// reverse engineer straight at the client's anti-cheat: the log file name, the
// list of parent processes the client expects, and its startup messages.
//
// A plain string literal lives in .rdata and is one `strings | grep anti` away
// from telling someone where to patch. These are stored XOR-masked at compile
// time and decoded onto the stack at the point of use, so the plaintext never
// exists in the image and the decoder cannot be folded back into a literal by
// the optimiser (the masked bytes are read through a volatile pointer).
//
// This is not a security boundary. It raises the effort needed to find the
// anti-cheat and to learn the process names it accepts; anything that actually
// has to be trusted is checked on the server, which the player cannot read.

namespace rblx::obf {

    // One byte per string position, so repeating characters do not produce
    // repeating ciphertext and the mask cannot be guessed from a single known
    // string.
    constexpr unsigned char maskFor(std::size_t index) {
        return static_cast<unsigned char>((0x5Bu + index * 0x9Du) & 0xFFu);
    }

    template <typename CharT, std::size_t N>
    class Literal {
    public:
        constexpr explicit Literal(const CharT (&text)[N]) {
            for (std::size_t i = 0; i < N; ++i) {
                encoded_[i] = static_cast<CharT>(static_cast<unsigned>(text[i]) ^ maskFor(i));
            }
        }

        std::basic_string<CharT> str() const {
            std::array<CharT, N> plain{};
            const volatile CharT* source = encoded_.data();
            for (std::size_t i = 0; i < N; ++i) {
                plain[i] = static_cast<CharT>(
                    static_cast<unsigned char>(source[i]) ^ maskFor(i));
            }

            std::basic_string<CharT> out(plain.data(), N - 1);

            // Do not leave the decoded copy on the stack for a memory scan to
            // pick up after the call.
            volatile CharT* wipe = plain.data();
            for (std::size_t i = 0; i < N; ++i) {
                wipe[i] = CharT{};
            }
            return out;
        }

    private:
        std::array<CharT, N> encoded_{};
    };

}

// RBLX_OBF("text")        -> std::string
// RBLX_OBF_W(L"text")     -> std::wstring
// Each use is its own constexpr instance, so the masked bytes stay in .rdata
// and only the decode runs at runtime.
#define RBLX_OBF(text)                                                                 \
    ([] {                                                                              \
        static constexpr ::rblx::obf::Literal<char, sizeof(text)> literal(text);        \
        return literal.str();                                                          \
    }())

#define RBLX_OBF_W(text)                                                               \
    ([] {                                                                              \
        static constexpr ::rblx::obf::Literal<wchar_t, sizeof(text) / sizeof(wchar_t)>  \
            literal(text);                                                             \
        return literal.str();                                                          \
    }())
