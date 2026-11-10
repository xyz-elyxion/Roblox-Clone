// dear rblxui: Platform Binding for Android native app
// This needs to be used along with the OpenGL 3 Renderer (rblxui_impl_opengl3)

// Implemented features:
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass RblxUIKey values to all key functions e.g. RblxUI::IsKeyPressed(RblxUIKey_Space). [Legacy AKEYCODE_* values are obsolete since 1.87 and not supported since 1.91.5]
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen/Pen.
// Missing features or Issues:
//  [ ] Platform: Clipboard support.
//  [ ] Platform: Gamepad support.
//  [ ] Platform: Mouse cursor shape and visibility (RblxUIBackendFlags_HasMouseCursors). Disable with 'io.ConfigFlags |= RblxUIConfigFlags_NoMouseCursorChange'. FIXME: Check if this is even possible with Android.
//  [ ] Platform: Multi-viewport support (multiple windows). Not meaningful on Android.
// Important:
//  - Consider using SDL or RBLXWIN backend on Android, which will be more full-featured than this.
//  - FIXME: On-screen keyboard currently needs to be enabled by the application (see examples/ and issue #3446)
//  - FIXME: Unicode character inputs needs to be passed by Dear RblxUI by the application (see examples/ and issue #3446)

// You can use unmodified rblxui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire rblxui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear RblxUI:
// - FAQ                  https://dearrblxui.com/faq
// - Getting Started      https://dearrblxui.com/getting-started
// - Documentation        https://dearrblxui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of rblxui.cpp

#pragma once
#include "rblxui.h"      // RBLXUI_IMPL_API
#ifndef RBLXUI_DISABLE

struct ANativeWindow;
struct AInputEvent;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool     RblxUI_ImplAndroid_Init(ANativeWindow* window);
RBLXUI_IMPL_API int32_t  RblxUI_ImplAndroid_HandleInputEvent(const AInputEvent* input_event);
RBLXUI_IMPL_API void     RblxUI_ImplAndroid_Shutdown();
RBLXUI_IMPL_API void     RblxUI_ImplAndroid_NewFrame();

#endif // #ifndef RBLXUI_DISABLE
