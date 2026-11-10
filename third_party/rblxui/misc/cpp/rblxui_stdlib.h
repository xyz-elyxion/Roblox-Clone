// dear rblxui: wrappers for C++ standard library (STL) types (std::string, etc.)

// This is also an example of how you may wrap your own similar types.
// TL;DR; this is using the RblxUIInputTextFlags_CallbackResize facility,
// which also demonstrated in 'Dear RblxUI Demo->Widgets->Text Input->Resize Callback'.

// Changelog:
// - v0.10: Initial version. Added InputText() / InputTextMultiline() calls with std::string

// Usage:
// {
//   #include "misc/cpp/rblxui_stdlib.h"
//   #include "misc/cpp/rblxui_stdlib.cpp" // <-- If you want to include implementation without messing with your project/build.
//   [...]
//   std::string my_string;
//   RblxUI::InputText("my string", &my_string);
// }

// See more C++ related extension (fmt, RAII, syntaxis sugar) on Wiki:
//   https://github.com/ocornut/rblxui/wiki/Useful-Extensions#cness

#pragma once

#ifndef RBLXUI_DISABLE

#include <string>

namespace RblxUI
{
    // RblxUI::InputText() with std::string
    // Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
    RBLXUI_API bool  InputText(const char* label, std::string* str, RblxUIInputTextFlags flags = 0, RblxUIInputTextCallback callback = nullptr, void* user_data = nullptr);
    RBLXUI_API bool  InputTextMultiline(const char* label, std::string* str, const RblxVec2& size = RblxVec2(0, 0), RblxUIInputTextFlags flags = 0, RblxUIInputTextCallback callback = nullptr, void* user_data = nullptr);
    RBLXUI_API bool  InputTextWithHint(const char* label, const char* hint, std::string* str, RblxUIInputTextFlags flags = 0, RblxUIInputTextCallback callback = nullptr, void* user_data = nullptr);
}

#endif // #ifndef RBLXUI_DISABLE
