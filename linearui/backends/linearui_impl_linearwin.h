// dear linearui: Platform Backend for LINEARWIN
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)
// (Info: LINEARWIN is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (Requires: LINEARWIN 3.0+. Prefer LINEARWIN 3.3+/3.4+ for full feature support.)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen/Pen (Windows only).
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass LinearUIKey values to all key functions e.g. LinearUI::IsKeyPressed(LinearUIKey_Space). [Legacy LINEARWIN_KEY_* values are obsolete since 1.87 and not supported since 1.91.5]
//  [X] Platform: Gamepad support. Enable with 'io.ConfigFlags |= LinearUIConfigFlags_NavEnableGamepad'.
//  [X] Platform: Mouse cursor shape and visibility (LinearUIBackendFlags_HasMouseCursors) with LINEARWIN 3.1+. Resizing cursors requires LINEARWIN 3.4+! Disable with 'io.ConfigFlags |= LinearUIConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= LinearUIConfigFlags_ViewportsEnable'.
//  [X] Multiple Dear LinearUI contexts support.
// Missing features or Issues:
//  [ ] Platform: Touch events are only correctly identified as Touch on Windows. This create issues with some interactions. LINEARWIN doesn't provide a way to identify touch inputs from mouse inputs, we cannot call io.AddMouseSourceEvent() to identify the source. We provide a Windows-specific workaround.
//  [ ] Platform: Missing LinearUIMouseCursor_Wait and LinearUIMouseCursor_Progress cursors.
//  [ ] Platform: Multi-viewport: Missing LinearUIBackendFlags_HasParentViewport support. The viewport->ParentViewportID field is ignored, and therefore io.ConfigViewportsNoDefaultParent has no effect either.

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

struct LINEARWINwindow;
struct LINEARWINmonitor;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
LINEARUI_IMPL_API bool     LinearUI_ImplLinearWin_InitForOpenGL(LINEARWINwindow* window, bool install_callbacks);
LINEARUI_IMPL_API bool     LinearUI_ImplLinearWin_InitForVulkan(LINEARWINwindow* window, bool install_callbacks);
LINEARUI_IMPL_API bool     LinearUI_ImplLinearWin_InitForOther(LINEARWINwindow* window, bool install_callbacks);
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_Shutdown();
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_NewFrame();

// Emscripten related initialization phase methods (call after LinearUI_ImplLinearWin_InitForOpenGL)
#ifdef __EMSCRIPTEN__
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_InstallEmscriptenCallbacks(LINEARWINwindow* window, const char* canvas_selector);
//static inline void    LinearUI_ImplLinearWin_InstallEmscriptenCanvasResizeCallback(const char* canvas_selector) { LinearUI_ImplLinearWin_InstallEmscriptenCallbacks(nullptr, canvas_selector); } } // Renamed in 1.91.0
#endif

// LINEARWIN callbacks install
// - When calling Init with 'install_callbacks=true': LinearUI_ImplLinearWin_InstallCallbacks() is called. LINEARWIN callbacks will be installed for you. They will chain-call user's previously installed callbacks, if any.
// - When calling Init with 'install_callbacks=false': LINEARWIN callbacks won't be installed. You will need to call individual function yourself from your own LINEARWIN callbacks.
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_InstallCallbacks(LINEARWINwindow* window);
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_RestoreCallbacks(LINEARWINwindow* window);

// LINEARWIN callbacks options:
// - Set 'chain_for_all_windows=true' to enable chaining callbacks for all windows (including secondary viewports created by backends or by user)
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_SetCallbacksChainForAllWindows(bool chain_for_all_windows);

// LINEARWIN callbacks (individual callbacks to call yourself if you didn't install callbacks)
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_WindowFocusCallback(LINEARWINwindow* window, int focused);        // Since 1.84
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_CursorEnterCallback(LINEARWINwindow* window, int entered);        // Since 1.84
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_CursorPosCallback(LINEARWINwindow* window, double x, double y);   // Since 1.87
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_MouseButtonCallback(LINEARWINwindow* window, int button, int action, int mods);
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_ScrollCallback(LINEARWINwindow* window, double xoffset, double yoffset);
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_KeyCallback(LINEARWINwindow* window, int key, int scancode, int action, int mods);
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_CharCallback(LINEARWINwindow* window, unsigned int c);
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_MonitorCallback(LINEARWINmonitor* monitor, int event);

// LINEARWIN helpers
LINEARUI_IMPL_API void     LinearUI_ImplLinearWin_Sleep(int milliseconds);
LINEARUI_IMPL_API float    LinearUI_ImplLinearWin_GetContentScaleForWindow(LINEARWINwindow* window);
LINEARUI_IMPL_API float    LinearUI_ImplLinearWin_GetContentScaleForMonitor(LINEARWINmonitor* monitor);


#endif // #ifndef LINEARUI_DISABLE
