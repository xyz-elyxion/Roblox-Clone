#pragma once

// Reading an environment variable, in one place.
//
// Configuration is read this way all over the project - RBLX_THEME, RBLXBUILD,
// RBLX_GIT_NAME and the rest - and the rule is that no call site touches getenv
// itself. The CRT's getenv is also deprecated by MSVC as C4996, so the Win32 call
// is used instead: it reports the length of the value, and every program here is
// Windows.
//
// A variable that is unset and one that is set to nothing are both "not set" to a
// caller, because that is what falling back to a default means.

#include <string>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace rblx {
namespace environment {

// The value of `name`, or an empty string when it is unset or empty.
inline std::string get(const char* name) {
    if (name == nullptr || *name == '\0') {
        return std::string();
    }
    const DWORD needed = ::GetEnvironmentVariableA(name, nullptr, 0);
    if (needed == 0) {
        return std::string();
    }
    std::string value(needed, '\0');
    const DWORD written = ::GetEnvironmentVariableA(name, value.data(), needed);
    if (written == 0 || written >= needed) {
        return std::string();
    }
    value.resize(written);
    return value;
}

}  // namespace environment
}  // namespace rblx
