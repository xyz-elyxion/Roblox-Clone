// dear rblxui: Platform Backend for RBLXWIN
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)
// (Info: RBLXWIN is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (Requires: RBLXWIN 3.0+. Prefer RBLXWIN 3.3+/3.4+ for full feature support.)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen/Pen (Windows only).
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass RblxUIKey values to all key functions e.g. RblxUI::IsKeyPressed(RblxUIKey_Space). [Legacy RBLXWIN_KEY_* values are obsolete since 1.87 and not supported since 1.91.5]
//  [X] Platform: Gamepad support. Enable with 'io.ConfigFlags |= RblxUIConfigFlags_NavEnableGamepad'.
//  [X] Platform: Mouse cursor shape and visibility (RblxUIBackendFlags_HasMouseCursors) with RBLXWIN 3.1+. Resizing cursors requires RBLXWIN 3.4+! Disable with 'io.ConfigFlags |= RblxUIConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= RblxUIConfigFlags_ViewportsEnable'.
//  [X] Multiple Dear RblxUI contexts support.
// Missing features or Issues:
//  [ ] Platform: Touch events are only correctly identified as Touch on Windows. This create issues with some interactions. RBLXWIN doesn't provide a way to identify touch inputs from mouse inputs, we cannot call io.AddMouseSourceEvent() to identify the source. We provide a Windows-specific workaround.
//  [ ] Platform: Missing RblxUIMouseCursor_Wait and RblxUIMouseCursor_Progress cursors.
//  [ ] Platform: Multi-viewport: Missing RblxUIBackendFlags_HasParentViewport support. The viewport->ParentViewportID field is ignored, and therefore io.ConfigViewportsNoDefaultParent has no effect either.

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

struct RBLXWINwindow;
struct RBLXWINmonitor;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool     RblxUI_ImplRblxWin_InitForOpenGL(RBLXWINwindow* window, bool install_callbacks);
RBLXUI_IMPL_API bool     RblxUI_ImplRblxWin_InitForVulkan(RBLXWINwindow* window, bool install_callbacks);
RBLXUI_IMPL_API bool     RblxUI_ImplRblxWin_InitForOther(RBLXWINwindow* window, bool install_callbacks);
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_Shutdown();
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_NewFrame();

// Emscripten related initialization phase methods (call after RblxUI_ImplRblxWin_InitForOpenGL)
#ifdef __EMSCRIPTEN__
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_InstallEmscriptenCallbacks(RBLXWINwindow* window, const char* canvas_selector);
//static inline void    RblxUI_ImplRblxWin_InstallEmscriptenCanvasResizeCallback(const char* canvas_selector) { RblxUI_ImplRblxWin_InstallEmscriptenCallbacks(nullptr, canvas_selector); } } // Renamed in 1.91.0
#endif

// RBLXWIN callbacks install
// - When calling Init with 'install_callbacks=true': RblxUI_ImplRblxWin_InstallCallbacks() is called. RBLXWIN callbacks will be installed for you. They will chain-call user's previously installed callbacks, if any.
// - When calling Init with 'install_callbacks=false': RBLXWIN callbacks won't be installed. You will need to call individual function yourself from your own RBLXWIN callbacks.
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_InstallCallbacks(RBLXWINwindow* window);
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_RestoreCallbacks(RBLXWINwindow* window);

// RBLXWIN callbacks options:
// - Set 'chain_for_all_windows=true' to enable chaining callbacks for all windows (including secondary viewports created by backends or by user)
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_SetCallbacksChainForAllWindows(bool chain_for_all_windows);

// RBLXWIN callbacks (individual callbacks to call yourself if you didn't install callbacks)
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_WindowFocusCallback(RBLXWINwindow* window, int focused);        // Since 1.84
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_CursorEnterCallback(RBLXWINwindow* window, int entered);        // Since 1.84
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_CursorPosCallback(RBLXWINwindow* window, double x, double y);   // Since 1.87
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_MouseButtonCallback(RBLXWINwindow* window, int button, int action, int mods);
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_ScrollCallback(RBLXWINwindow* window, double xoffset, double yoffset);
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_KeyCallback(RBLXWINwindow* window, int key, int scancode, int action, int mods);
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_CharCallback(RBLXWINwindow* window, unsigned int c);
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_MonitorCallback(RBLXWINmonitor* monitor, int event);

// RBLXWIN helpers
RBLXUI_IMPL_API void     RblxUI_ImplRblxWin_Sleep(int milliseconds);
RBLXUI_IMPL_API float    RblxUI_ImplRblxWin_GetContentScaleForWindow(RBLXWINwindow* window);
RBLXUI_IMPL_API float    RblxUI_ImplRblxWin_GetContentScaleForMonitor(RBLXWINmonitor* monitor);


#endif // #ifndef RBLXUI_DISABLE
