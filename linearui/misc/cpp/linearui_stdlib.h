// dear linearui: wrappers for C++ standard library (STL) types (std::string, etc.)

// This is also an example of how you may wrap your own similar types.
// TL;DR; this is using the LinearUIInputTextFlags_CallbackResize facility,
// which also demonstrated in 'Dear LinearUI Demo->Widgets->Text Input->Resize Callback'.

// Changelog:
// - v0.10: Initial version. Added InputText() / InputTextMultiline() calls with std::string

// Usage:
// {
//   #include "misc/cpp/linearui_stdlib.h"
//   #include "misc/cpp/linearui_stdlib.cpp" // <-- If you want to include implementation without messing with your project/build.
//   [...]
//   std::string my_string;
//   LinearUI::InputText("my string", &my_string);
// }

// See more C++ related extension (fmt, RAII, syntaxis sugar) on Wiki:
//   https://github.com/ocornut/linearui/wiki/Useful-Extensions#cness

#pragma once

#ifndef LINEARUI_DISABLE

#include <string>

namespace LinearUI
{
    // LinearUI::InputText() with std::string
    // Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
    LINEARUI_API bool  InputText(const char* label, std::string* str, LinearUIInputTextFlags flags = 0, LinearUIInputTextCallback callback = nullptr, void* user_data = nullptr);
    LINEARUI_API bool  InputTextMultiline(const char* label, std::string* str, const LinearVec2& size = LinearVec2(0, 0), LinearUIInputTextFlags flags = 0, LinearUIInputTextCallback callback = nullptr, void* user_data = nullptr);
    LINEARUI_API bool  InputTextWithHint(const char* label, const char* hint, std::string* str, LinearUIInputTextFlags flags = 0, LinearUIInputTextCallback callback = nullptr, void* user_data = nullptr);
}

#endif // #ifndef LINEARUI_DISABLE
