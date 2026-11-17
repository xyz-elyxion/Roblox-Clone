// dear linearui: Platform Binding for Android native app
// This needs to be used along with the OpenGL 3 Renderer (linearui_impl_opengl3)

// Implemented features:
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass LinearUIKey values to all key functions e.g. LinearUI::IsKeyPressed(LinearUIKey_Space). [Legacy AKEYCODE_* values are obsolete since 1.87 and not supported since 1.91.5]
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen/Pen.
// Missing features or Issues:
//  [ ] Platform: Clipboard support.
//  [ ] Platform: Gamepad support.
//  [ ] Platform: Mouse cursor shape and visibility (LinearUIBackendFlags_HasMouseCursors). Disable with 'io.ConfigFlags |= LinearUIConfigFlags_NoMouseCursorChange'. FIXME: Check if this is even possible with Android.
//  [ ] Platform: Multi-viewport support (multiple windows). Not meaningful on Android.
// Important:
//  - Consider using SDL or LINEARWIN backend on Android, which will be more full-featured than this.
//  - FIXME: On-screen keyboard currently needs to be enabled by the application (see examples/ and issue #3446)
//  - FIXME: Unicode character inputs needs to be passed by Dear LinearUI by the application (see examples/ and issue #3446)

// You can use unmodified linearui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire linearui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear LinearUI:
// - FAQ                  https://dearlinearui.com/faq
// - Getting Started      https://dearlinearui.com/getting-started
// - Documentation        https://dearlinearui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of linearui.cpp

#pragma once
#include "linearui.h"      // LINEARUI_IMPL_API
#ifndef LINEARUI_DISABLE

struct ANativeWindow;
struct AInputEvent;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
LINEARUI_IMPL_API bool     LinearUI_ImplAndroid_Init(ANativeWindow* window);
LINEARUI_IMPL_API int32_t  LinearUI_ImplAndroid_HandleInputEvent(const AInputEvent* input_event);
LINEARUI_IMPL_API void     LinearUI_ImplAndroid_Shutdown();
LINEARUI_IMPL_API void     LinearUI_ImplAndroid_NewFrame();

#endif // #ifndef LINEARUI_DISABLE
