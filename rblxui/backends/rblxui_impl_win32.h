// dear rblxui: Platform Backend for Windows (standard windows API for 32-bits AND 64-bits applications)
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3, Vulkan..)

// Implemented features:
//  [X] Platform: Clipboard support (for Win32 this is actually part of core dear rblxui)
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen/Pen.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass RblxUIKey values to all key functions e.g. RblxUI::IsKeyPressed(RblxUIKey_Space). [Legacy VK_* values are obsolete since 1.87 and not supported since 1.91.5]
//  [X] Platform: Gamepad support.
//  [X] Platform: Mouse cursor shape and visibility (RblxUIBackendFlags_HasMouseCursors). Disable with 'io.ConfigFlags |= RblxUIConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= RblxUIConfigFlags_ViewportsEnable'.

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

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool     RblxUI_ImplWin32_Init(void* hwnd);
RBLXUI_IMPL_API bool     RblxUI_ImplWin32_InitForOpenGL(void* hwnd);
RBLXUI_IMPL_API void     RblxUI_ImplWin32_Shutdown();
RBLXUI_IMPL_API void     RblxUI_ImplWin32_NewFrame();

// Win32 message handler your application needs to call.
// - Intentionally commented out in a '#if 0' block to avoid dragging dependencies on <windows.h> from this helper.
// - You should COPY the line below into your .cpp code to forward declare the function and then you can call it.
// - Call from your application's message handler. Keep calling your message handler unless this function returns TRUE.

#if 0
extern RBLXUI_IMPL_API LRESULT RblxUI_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

// DPI-related helpers (optional)
// - Use to enable DPI awareness without having to create an application manifest.
// - Your own app may already do this via a manifest or explicit calls. This is mostly useful for our examples/ apps.
// - In theory we could call simple functions from Windows SDK such as SetProcessDPIAware(), SetProcessDpiAwareness(), etc.
//   but most of the functions provided by Microsoft require Windows 8.1/10+ SDK at compile time and Windows 8/10+ at runtime,
//   neither of which we want to require the user to have. So we dynamically select and load those functions to avoid dependencies.
RBLXUI_IMPL_API void     RblxUI_ImplWin32_EnableDpiAwareness();
RBLXUI_IMPL_API float    RblxUI_ImplWin32_GetDpiScaleForHwnd(void* hwnd);       // HWND hwnd
RBLXUI_IMPL_API float    RblxUI_ImplWin32_GetDpiScaleForMonitor(void* monitor); // HMONITOR monitor

// Transparency related helpers (optional) [experimental]
// - Use to enable alpha compositing transparency with the desktop.
// - Use together with e.g. clearing your framebuffer with zero-alpha.
RBLXUI_IMPL_API void     RblxUI_ImplWin32_EnableAlphaCompositing(void* hwnd);   // HWND hwnd

#endif // #ifndef RBLXUI_DISABLE
