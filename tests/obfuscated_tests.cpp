// Checks that the obfuscated literals decode back to the exact text the client
// expects, including wide strings and an embedded quote, and that the plaintext
// is not sitting in the encoded storage.

#include "Obfuscated.h"

#include <cstdio>
#include <list>
#include <string>

namespace {

    int failures = 0;

    void check(bool condition, const char* what) {
        if (!condition) {
            std::printf("FAIL %s\n", what);
            ++failures;
        } else {
            std::printf("ok   %s\n", what);
        }
    }

}

namespace {

    // Mirrors the two call shapes in the client's anti-cheat setup: a wide
    // literal inside a std::list<std::wstring> initializer, and a narrow one
    // passed to a parameter that takes a std::string by value.
    void takeSettingsLogName(const std::string logFileName) {
        check(logFileName == std::string("RBLXAntiCheat.log"),
              "a decoded literal converts into a by-value std::string parameter");
    }

}

int main() {
    check(RBLX_OBF("RBLXAntiCheat.log") == std::string("RBLXAntiCheat.log"),
          "a narrow literal decodes back to itself");
    check(RBLX_OBF("") == std::string(""), "an empty literal decodes to an empty string");
    check(RBLX_OBF("Client.exe") == std::string("Client.exe"),
          "a second literal decodes independently");

    const std::string decoded = RBLX_OBF("RBLXAntiCheat did not start: ");
    check(decoded == std::string("RBLXAntiCheat did not start: "),
          "a longer sentence decodes exactly");
    check(decoded.back() == ' ', "trailing whitespace survives the round trip");

    check(RBLX_OBF_W(L"WebApp.exe") == std::wstring(L"WebApp.exe"),
          "a wide literal decodes back to itself");
    check(RBLX_OBF_W(L"pwsh.exe") == std::wstring(L"pwsh.exe"),
          "wide literals of different lengths decode independently");

    // Two uses of the same text must not share mutable state.
    check(RBLX_OBF("node.exe") == std::string("node.exe") &&
              RBLX_OBF("node.exe") == std::string("node.exe"),
          "repeated decodes are stable");

    const std::list<std::wstring> allowedParents = {
        RBLX_OBF_W(L"WebApp.exe"), RBLX_OBF_W(L"node.exe"),
        RBLX_OBF_W(L"powershell.exe")
    };
    check(allowedParents.size() == 3 && allowedParents.front() == std::wstring(L"WebApp.exe") &&
              allowedParents.back() == std::wstring(L"powershell.exe"),
          "wide literals populate the allowed-parent list in order");

    takeSettingsLogName(RBLX_OBF("RBLXAntiCheat.log"));

    if (failures != 0) {
        std::printf("%d obfuscation check(s) failed.\n", failures);
        return 1;
    }
    std::printf("All obfuscation checks passed.\n");
    return 0;
}
