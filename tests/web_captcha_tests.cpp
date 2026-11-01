#include "WebCaptcha.h"

#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(bool condition, const char* what) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << what << '\n';
    }
}

}  // namespace

int main() {
    using namespace rblx::web;

    {
        const std::string code = makeCaptchaCode();
        check(static_cast<int>(code.size()) == kCaptchaDigits, "a human challenge is four digits");
        bool digitsOnly = true;
        for (char c : code) {
            digitsOnly = digitsOnly && c >= '0' && c <= '9';
        }
        check(digitsOnly, "the challenge is digits only");
    }

    {
        const std::string svg = renderHumanCaptchaSvg("1234");
        check(svg.rfind("<svg", 0) == 0, "the challenge renders as svg");
        check(svg.find("<text") == std::string::npos,
              "the answer is drawn as pixels, not as a text element");
        check(svg.find(">1234") == std::string::npos,
              "the answer does not appear as text in the markup");
        check(svg.find("<rect") != std::string::npos, "the glyphs are made of rects");

        const std::string uri = captchaImageDataUri(svg);
        check(uri.rfind("data:image/svg+xml;charset=utf-8,", 0) == 0, "the image is a data uri");
        check(uri.find('%') != std::string::npos, "the markup is percent-encoded");
        check(uri.find('#') == std::string::npos, "no raw fragment marker survives encoding");
    }

    {
        check(captchaAnswerMatches("1234", "1234"), "the exact answer matches");
        check(captchaAnswerMatches("1234", " 1 2-34 "), "spacing and dashes are ignored");
        check(!captchaAnswerMatches("1234", "4321"), "the wrong order does not match");
        check(!captchaAnswerMatches("1234", ""), "an empty answer does not match");
        check(!captchaAnswerMatches("", ""), "a challenge with no answer never matches");
    }

    {
        const std::string challenge = "f47ac10b-58cc-4372-a567-0e02b2c3d479";
        std::string nonce;
        check(solveCaptchaPow(challenge, kCaptchaPowDifficulty, &nonce),
              "the machine challenge is solvable");
        check(verifyCaptchaPow(challenge, nonce, kCaptchaPowDifficulty),
              "the solved nonce verifies");

        std::string badNonce;
        for (int attempt = 0; attempt < 1000 && badNonce.empty(); ++attempt) {
            const std::string candidate = std::to_string(attempt);
            if (!verifyCaptchaPow(challenge, candidate, kCaptchaPowDifficulty)) {
                badNonce = candidate;
            }
        }
        check(!badNonce.empty(), "an unsolved nonce exists to test against");
        check(!verifyCaptchaPow(challenge, badNonce, kCaptchaPowDifficulty),
              "a nonce that does not reach the difficulty fails");
        check(!verifyCaptchaPow(challenge, "abc", kCaptchaPowDifficulty),
              "a nonce that is not decimal fails");
        check(!verifyCaptchaPow("", nonce, kCaptchaPowDifficulty), "an empty challenge fails");
        check(!verifyCaptchaPow(challenge, nonce, 0), "a zero difficulty fails");
        check(!verifyCaptchaPow(challenge, nonce, 9), "a difficulty past the digest fails");
    }

    if (failures != 0) {
        std::cerr << failures << " captcha check(s) failed\n";
        return 1;
    }

    std::cout << "captcha: all checks passed\n";
    return 0;
}
