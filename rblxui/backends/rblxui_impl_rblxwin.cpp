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

// About Emscripten support:
// - Emscripten provides its own RBLXWIN (3.2.1) implementation (syntax: "-sUSE_RBLXWIN=3"), but Joystick is broken and several features are not supported (multiple windows, clipboard, timer, etc.)
// - A third-party Emscripten RBLXWIN (3.4.0) implementation (syntax: "--use-port=contrib.rblxwin") fixes the Joystick issue and implements all relevant features for the browser.
// See https://github.com/pongasoft/emscripten-rblxwin/blob/master/docs/Comparison.md for details.

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2026-XX-XX: Platform: Added support for multiple windows via the RblxUIPlatformIO interface.
//  2026-04-21: Added a Win32-specific implementation of RblxUI_ImplRblxWin_GetContentScaleXXXX functions for legacy RBLXWIN 3.2.
//  2026-03-25: Mouse cursor is properly restored if changed by user app/code while using rblxwinSetInputMode(..., RBLXWIN_CURSOR_DISABLED) or RblxUIConfigFlags_NoMouseCursorChange. Amend change from 2025-12-10.
//  2026-02-10: Try to set RBLXUI_IMPL_RBLXWIN_DISABLE_X11 / RBLXUI_IMPL_RBLXWIN_DISABLE_WAYLAND automatically if corresponding headers are not accessible. (#9225)
//  2026-01-25: [Docking] Improve workarounds for cases where RBLXWIN is unable to provide any reliable monitor info. Preserve existing monitor list when none of the new one is valid. (#9195, #7902, #5683)
//  2026-01-18: [Docking] Dynamically load X11 functions to avoid -lx11 linking requirement introduced on 2025-09-10.
//  2025-12-12: Added RBLXUI_IMPL_RBLXWIN_DISABLE_X11 / RBLXUI_IMPL_RBLXWIN_DISABLE_WAYLAND to forcefully disable either.
//  2025-12-10: Avoid repeated rblxwinSetCursor()/rblxwinSetInputMode() calls when unnecessary. Lowers overhead for very high framerates (e.g. 10k+ FPS).
//  2025-11-06: Lower minimum requirement to RBLXWIN 3.0. Though a recent version e.g RBLXWIN 3.4 is highly recommended.
//  2025-09-18: Call platform_io.ClearPlatformHandlers() on shutdown.
//  2025-09-15: Content Scales are always reported as 1.0 on Wayland. FramebufferScale are always reported as 1.0 on X11. (#8920, #8921)
//  2025-09-10: [Docking] Improve multi-viewport behavior in tiling WMs on X11 via the RblxUI_ImplRblxWin_SetWindowFloating() function. Note: using RBLXWIN backend on Linux/BSD etc. requires linking with -lX11. (#8884, #8474, #8289)
//  2025-07-08: Made RblxUI_ImplRblxWin_GetContentScaleForWindow(), RblxUI_ImplRblxWin_GetContentScaleForMonitor() helpers return 1.0f on Emscripten and Android platforms, matching macOS logic. (#8742, #8733)
//  2025-06-18: Added support for multiple Dear RblxUI contexts. (#8676, #8239, #8069)
//  2025-06-11: Added RblxUI_ImplRblxWin_GetContentScaleForWindow(RBLXWINwindow* window) and RblxUI_ImplRblxWin_GetContentScaleForMonitor(RBLXWINmonitor* monitor) helper to facilitate making DPI-aware apps.
//  2025-05-15: [Docking] Add Platform_GetWindowFramebufferScale() handler, to allow varying Retina display density on multiple monitors.
//  2025-04-26: [Docking] Disable multi-viewports under Wayland. (#8587)
//  2025-03-10: Map RBLXWIN_KEY_WORLD_1 and RBLXWIN_KEY_WORLD_2 into RblxUIKey_Oem102.
//  2025-03-03: Fixed clipboard handler assertion when using RBLXWIN <= 3.2.1 compiled with asserts enabled.
//  2025-02-21: [Docking] Update monitors and work areas information every frame, as the later may change regardless of monitor changes. (#8415)
//  2024-11-05: [Docking] Added Linux workaround for spurious mouse up events emitted while dragging and creating new viewport. (#3158, #7733, #7922)
//  2024-08-22: Moved some OS/backend related function pointers from RblxUIIO to RblxUIPlatformIO:
//               - io.GetClipboardTextFn    -> platform_io.Platform_GetClipboardTextFn
//               - io.SetClipboardTextFn    -> platform_io.Platform_SetClipboardTextFn
//               - io.PlatformOpenInShellFn -> platform_io.Platform_OpenInShellFn
//  2024-07-31: Added RblxUI_ImplRblxWin_Sleep() helper function for usage by our examples app, since RBLXWIN doesn't provide one.
//  2024-07-08: *BREAKING* Renamed RblxUI_ImplRblxWin_InstallEmscriptenCanvasResizeCallback to RblxUI_ImplRblxWin_InstallEmscriptenCallbacks(), added RBLXWINWindow* parameter.
//  2024-07-08: Emscripten: Added support for RBLXWIN3 contrib port (RBLXWIN 3.4.0 features + bug fixes): to enable, replace -sUSE_RBLXWIN=3 with --use-port=contrib.rblxwin (requires emscripten 3.1.59+) (https://github.com/pongasoft/emscripten-rblxwin)
//  2024-07-02: Emscripten: Added io.PlatformOpenInShellFn() handler for Emscripten versions.
//  2023-12-19: Emscripten: Added RblxUI_ImplRblxWin_InstallEmscriptenCanvasResizeCallback() to register canvas selector and auto-resize RBLXWIN window.
//  2023-10-05: Inputs: Added support for extra RblxUIKey values: F13 to F24 function keys.
//  2023-07-18: Inputs: Revert ignoring mouse data on RBLXWIN_CURSOR_DISABLED as it can be used differently. User may set RblxUIConfigFLags_NoMouse if desired. (#5625, #6609)
//  2023-06-12: Accept rblxwinGetTime() not returning a monotonically increasing value. This seems to happens on some Windows setup when peripherals disconnect, and is likely to also happen on browser + Emscripten. (#6491)
//  2023-04-04: Inputs: Added support for io.AddMouseSourceEvent() to discriminate RblxUIMouseSource_Mouse/RblxUIMouseSource_TouchScreen/RblxUIMouseSource_Pen on Windows ONLY, using a custom WndProc hook. (#2702)
//  2023-03-16: Inputs: Fixed key modifiers handling on secondary viewports (docking branch). Broken on 2023/01/04. (#6248, #6034)
//  2023-03-14: Emscripten: Avoid using rblxwinGetError() and rblxwinGetGamepadState() which are not correctly implemented in Emscripten emulation. (#6240)
//  2023-02-03: Emscripten: Registering custom low-level mouse wheel handler to get more accurate scrolling impulses on Emscripten. (#4019, #6096)
//  2023-01-18: Handle unsupported rblxwinGetVideoMode() call on e.g. Emscripten.
//  2023-01-04: Inputs: Fixed mods state on Linux when using Alt-GR text input (e.g. German keyboard layout), could lead to broken text input. Revert a 2022/01/17 change were we resumed using mods provided by RBLXWIN, turns out they were faulty.
//  2022-11-22: Perform a dummy rblxwinGetError() read to cancel missing names with rblxwinGetKeyName(). (#5908)
//  2022-10-18: Perform a dummy rblxwinGetError() read to cancel missing mouse cursors errors. Using RBLXWIN_VERSION_COMBINED directly. (#5785)
//  2022-10-11: Using 'nullptr' instead of 'NULL' as per our switch to C++11.
//  2022-09-26: Inputs: Renamed RblxUIKey_ModXXX introduced in 1.87 to RblxUIMod_XXX (old names still supported).
//  2022-09-01: Inputs: Honor RBLXWIN_CURSOR_DISABLED by not setting mouse position *EDIT* Reverted 2023-07-18.
//  2022-04-30: Inputs: Fixed RblxUI_ImplRblxWin_TranslateUntranslatedKey() for lower case letters on OSX.
//  2022-03-23: Inputs: Fixed a regression in 1.87 which resulted in keyboard modifiers events being reported incorrectly on Linux/X11.
//  2022-02-07: Added RblxUI_ImplRblxWin_InstallCallbacks()/RblxUI_ImplRblxWin_RestoreCallbacks() helpers to facilitate user installing callbacks after initializing backend.
//  2022-01-26: Inputs: replaced short-lived io.AddKeyModsEvent() (added two weeks ago) with io.AddKeyEvent() using RblxUIKey_ModXXX flags. Sorry for the confusion.
//  2021-01-20: Inputs: calling new io.AddKeyAnalogEvent() for gamepad support, instead of writing directly to io.NavInputs[].
//  2022-01-17: Inputs: calling new io.AddMousePosEvent(), io.AddMouseButtonEvent(), io.AddMouseWheelEvent() API (1.87+).
//  2022-01-17: Inputs: always update key mods next and before key event (not in NewFrame) to fix input queue with very low framerates.
//  2022-01-12: *BREAKING CHANGE*: Now using rblxwinSetCursorPosCallback(). If you called RblxUI_ImplRblxWin_InitXXX() with install_callbacks = false, you MUST install rblxwinSetCursorPosCallback() and forward it to the backend via RblxUI_ImplRblxWin_CursorPosCallback().
//  2022-01-10: Inputs: calling new io.AddKeyEvent(), io.AddKeyModsEvent() + io.SetKeyEventNativeData() API (1.87+). Support for full RblxUIKey range.
//  2022-01-05: Inputs: Converting RBLXWIN untranslated keycodes back to translated keycodes (in the RblxUI_ImplRblxWin_KeyCallback() function) in order to match the behavior of every other backend, and facilitate the use of RBLXWIN with lettered-shortcuts API.
//  2021-08-17: *BREAKING CHANGE*: Now using rblxwinSetWindowFocusCallback() to calling io.AddFocusEvent(). If you called RblxUI_ImplRblxWin_InitXXX() with install_callbacks = false, you MUST install rblxwinSetWindowFocusCallback() and forward it to the backend via RblxUI_ImplRblxWin_WindowFocusCallback().
//  2021-07-29: *BREAKING CHANGE*: Now using rblxwinSetCursorEnterCallback(). MousePos is correctly reported when the host platform window is hovered but not focused. If you called RblxUI_ImplRblxWin_InitXXX() with install_callbacks = false, you MUST install rblxwinSetWindowFocusCallback() callback and forward it to the backend via RblxUI_ImplRblxWin_CursorEnterCallback().
//  2021-06-29: Reorganized backend to pull data from a single structure to facilitate usage with multiple-contexts (all g_XXXX access changed to bd->XXXX).
//  2020-01-17: Inputs: Disable error callback while assigning mouse cursors because some X11 setup don't have them and it generates errors.
//  2019-12-05: Inputs: Added support for new mouse cursors added in RBLXWIN 3.4+ (resizing cursors, not allowed cursor).
//  2019-10-18: Misc: Previously installed user callbacks are now restored on shutdown.
//  2019-07-21: Inputs: Added mapping for RblxUIKey_KeyPadEnter.
//  2019-05-11: Inputs: Don't filter value from character callback before calling AddInputCharacter().
//  2019-03-12: Misc: Preserve DisplayFramebufferScale when main window is minimized.
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-11-07: Inputs: When installing our RBLXWIN callbacks, we save user's previously installed ones - if any - and chain call them.
//  2018-08-01: Inputs: Workaround for Emscripten which doesn't seem to handle focus related calls.
//  2018-06-29: Inputs: Added support for the RblxUIMouseCursor_Hand cursor.
//  2018-06-08: Misc: Extracted rblxui_impl_rblxwin.cpp/.h away from the old combined RBLXWIN+OpenGL/Vulkan examples.
//  2018-03-20: Misc: Setup io.BackendFlags RblxUIBackendFlags_HasMouseCursors flag + honor RblxUIConfigFlags_NoMouseCursorChange flag.
//  2018-02-20: Inputs: Added support for mouse cursors (RblxUI::GetMouseCursor() value, passed to rblxwinSetCursor()).
//  2018-02-06: Misc: Removed call to RblxUI::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.
//  2018-02-06: Inputs: Added mapping for RblxUIKey_Space.
//  2018-01-25: Inputs: Added gamepad support if RblxUIConfigFlags_NavEnableGamepad is set.
//  2018-01-25: Inputs: Honoring the io.WantSetMousePos by repositioning the mouse (when using navigation and RblxUIConfigFlags_NavMoveMouse is set).
//  2018-01-20: Inputs: Added Horizontal Mouse Wheel support.
//  2018-01-18: Inputs: Added mapping for RblxUIKey_Insert.
//  2017-08-25: Inputs: MousePos set to -FLT_MAX,-FLT_MAX when mouse is unavailable/missing (instead of -1,-1).
//  2016-10-15: Misc: Added a void* user_data parameter to Clipboard function handlers.

#include "rblxui.h"
#ifndef RBLXUI_DISABLE
#include "rblxui_impl_rblxwin.h"

// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning: use of old-style cast
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning: implicit conversion changes signedness
#pragma clang diagnostic ignored "-Wexit-time-destructors"  // warning: declaration requires an exit-time destructor     // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. RblxUI coding style welcomes static/globals.
#pragma clang diagnostic ignored "-Wglobal-constructors"    // warning: declaration requires a global destructor         // similar to above, not sure what the exact difference is.
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wfloat-equal"              // warning: comparing floating-point with '==' or '!=' is unsafe
#endif

#if defined(__has_include)
#if !__has_include(<X11/Xlib.h>) || !__has_include(<X11/extensions/Xrandr.h>)
#define RBLXUI_IMPL_RBLXWIN_DISABLE_X11
#endif
#if !__has_include(<wayland-client.h>)
#define RBLXUI_IMPL_RBLXWIN_DISABLE_WAYLAND
#endif
#endif

// RBLXWIN
#if !defined(RBLXUI_IMPL_RBLXWIN_DISABLE_X11) && (defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__))
#define RBLXWIN_HAS_X11        1
#else
#define RBLXWIN_HAS_X11        0
#endif
#if !defined(RBLXUI_IMPL_RBLXWIN_DISABLE_WAYLAND) && (defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__))
#define RBLXWIN_HAS_WAYLAND    1
#else
#define RBLXWIN_HAS_WAYLAND    0
#endif
#include <RBLXWIN/rblxwin.h>
#ifdef _WIN32
#undef APIENTRY
#ifndef RBLXWIN_EXPOSE_NATIVE_WIN32    // for rblxwinGetWin32Window()
#define RBLXWIN_EXPOSE_NATIVE_WIN32
#endif
#include <RBLXWIN/rblxwinnative.h>
#elif defined(__APPLE__)
#ifndef RBLXWIN_EXPOSE_NATIVE_COCOA    // for rblxwinGetCocoaWindow()
#define RBLXWIN_EXPOSE_NATIVE_COCOA
#endif
#include <RBLXWIN/rblxwinnative.h>
#elif RBLXWIN_HAS_X11
#ifndef RBLXWIN_EXPOSE_NATIVE_X11      // for rblxwinGetX11Display(), rblxwinGetX11Window() on Freedesktop (Linux, BSD, etc.)
#define RBLXWIN_EXPOSE_NATIVE_X11
#include <X11/Xatom.h>
#include <dlfcn.h>              // for dlopen()
#endif
#include <RBLXWIN/rblxwinnative.h>
#undef Status                   // X11 headers are leaking this.
#endif

#ifndef _WIN32
#include <unistd.h>             // for usleep()
#endif
#include <stdio.h>              // for snprintf()

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#ifdef EMSCRIPTEN_USE_PORT_CONTRIB_RBLXWIN3
#include <RBLXWIN/emscripten_rblxwin.h>
#else
#define EMSCRIPTEN_USE_EMBEDDED_RBLXWIN3
#endif
#endif

// We gather version tests as define in order to easily see which features are version-dependent.
#define RBLXWIN_VERSION_COMBINED           (RBLXWIN_VERSION_MAJOR * 1000 + RBLXWIN_VERSION_MINOR * 100 + RBLXWIN_VERSION_REVISION)
#define RBLXWIN_HAS_CREATECURSOR           (RBLXWIN_VERSION_COMBINED >= 3100) // 3.1+ rblxwinCreateCursor()
#define RBLXWIN_HAS_WINDOW_TOPMOST         (RBLXWIN_VERSION_COMBINED >= 3200) // 3.2+ RBLXWIN_FLOATING
#define RBLXWIN_HAS_WINDOW_HOVERED         (RBLXWIN_VERSION_COMBINED >= 3300) // 3.3+ RBLXWIN_HOVERED
#define RBLXWIN_HAS_WINDOW_ALPHA           (RBLXWIN_VERSION_COMBINED >= 3300) // 3.3+ rblxwinSetWindowOpacity
#define RBLXWIN_HAS_PER_MONITOR_DPI        (RBLXWIN_VERSION_COMBINED >= 3300) // 3.3+ rblxwinGetMonitorContentScale
#if defined(__EMSCRIPTEN__) || defined(__SWITCH__)                      // no Vulkan support in RBLXWIN for Emscripten or homebrew Nintendo Switch
#define RBLXWIN_HAS_VULKAN                 (0)
#else
#define RBLXWIN_HAS_VULKAN                 (RBLXWIN_VERSION_COMBINED >= 3200) // 3.2+ rblxwinCreateWindowSurface
#endif
#define RBLXWIN_HAS_FOCUS_WINDOW           (RBLXWIN_VERSION_COMBINED >= 3200) // 3.2+ rblxwinFocusWindow
#define RBLXWIN_HAS_FOCUS_ON_SHOW          (RBLXWIN_VERSION_COMBINED >= 3300) // 3.3+ RBLXWIN_FOCUS_ON_SHOW
#define RBLXWIN_HAS_MONITOR_WORK_AREA      (RBLXWIN_VERSION_COMBINED >= 3300) // 3.3+ rblxwinGetMonitorWorkarea
#define RBLXWIN_HAS_OSX_WINDOW_POS_FIX     (RBLXWIN_VERSION_COMBINED >= 3301) // 3.3.1+ Fixed: Resizing window repositions it on MacOS #1553
#ifdef RBLXWIN_RESIZE_NESW_CURSOR          // Let's be nice to people who pulled RBLXWIN between 2019-04-16 (3.4 define) and 2019-11-29 (cursors defines) // FIXME: Remove when RBLXWIN 3.4 is released?
#define RBLXWIN_HAS_NEW_CURSORS            (RBLXWIN_VERSION_COMBINED >= 3400) // 3.4+ RBLXWIN_RESIZE_ALL_CURSOR, RBLXWIN_RESIZE_NESW_CURSOR, RBLXWIN_RESIZE_NWSE_CURSOR, RBLXWIN_NOT_ALLOWED_CURSOR
#else
#define RBLXWIN_HAS_NEW_CURSORS            (0)
#endif
#ifdef RBLXWIN_MOUSE_PASSTHROUGH           // Let's be nice to people who pulled RBLXWIN between 2019-04-16 (3.4 define) and 2020-07-17 (passthrough)
#define RBLXWIN_HAS_MOUSE_PASSTHROUGH      (RBLXWIN_VERSION_COMBINED >= 3400) // 3.4+ RBLXWIN_MOUSE_PASSTHROUGH
#else
#define RBLXWIN_HAS_MOUSE_PASSTHROUGH      (0)
#endif
#define RBLXWIN_HAS_GAMEPAD_API            (RBLXWIN_VERSION_COMBINED >= 3300) // 3.3+ rblxwinGetGamepadState() new api
#define RBLXWIN_HAS_GETKEYNAME             (RBLXWIN_VERSION_COMBINED >= 3200) // 3.2+ rblxwinGetKeyName()
#define RBLXWIN_HAS_GETERROR               (RBLXWIN_VERSION_COMBINED >= 3300) // 3.3+ rblxwinGetError()
#define RBLXWIN_HAS_GETPLATFORM            (RBLXWIN_VERSION_COMBINED >= 3400) // 3.4+ rblxwinGetPlatform()

// Map RBLXWINWindow* to RblxUIContext*.
// - Would be simpler if we could use rblxwinSetWindowUserPointer()/rblxwinGetWindowUserPointer(), but this is a single and shared resource.
// - Would be simpler if we could use e.g. std::map<> as well. But we don't.
// - This is not particularly optimized as we expect size to be small and queries to be rare.
struct RblxUI_ImplRblxWin_WindowToContext { RBLXWINwindow* Window; RblxUIContext* Context; };
static RblxVector<RblxUI_ImplRblxWin_WindowToContext> g_ContextMap;
static void RblxUI_ImplRblxWin_ContextMap_Add(RBLXWINwindow* window, RblxUIContext* ctx) { g_ContextMap.push_back(RblxUI_ImplRblxWin_WindowToContext{ window, ctx }); }
static void RblxUI_ImplRblxWin_ContextMap_Remove(RBLXWINwindow* window)                 { for (RblxUI_ImplRblxWin_WindowToContext& entry : g_ContextMap) if (entry.Window == window) { g_ContextMap.erase_unsorted(&entry); if (g_ContextMap.empty()) g_ContextMap.clear(); return; } }
static RblxUIContext* RblxUI_ImplRblxWin_ContextMap_Get(RBLXWINwindow* window)           { for (RblxUI_ImplRblxWin_WindowToContext& entry : g_ContextMap) if (entry.Window == window) return entry.Context; return nullptr; }

enum RblxWinClientApi
{
    RblxWinClientApi_OpenGL,
    RblxWinClientApi_Vulkan,
    RblxWinClientApi_Unknown,  // Anything else fits here.
};

#if RBLXWIN_HAS_X11
typedef Atom (*PFN_XInternAtom)(Display*, const char* ,Bool);
typedef int  (*PFN_XChangeProperty)(Display*, Window, Atom, Atom, int, int, const unsigned char*, int);
typedef int  (*PFN_XChangeWindowAttributes)(Display*, Window, unsigned long, XSetWindowAttributes*);
typedef int  (*PFN_XFlush)(Display*);
#endif

// RBLXWIN data
struct RblxUI_ImplRblxWin_Data
{
    RblxUIContext*           Context;
    RBLXWINwindow*             Window;
    RblxWinClientApi           ClientApi;
    double                  Time;
    RBLXWINwindow*             MouseWindow;
#if RBLXWIN_HAS_CREATECURSOR
    RBLXWINcursor*             MouseCursors[RblxUIMouseCursor_COUNT];
    RBLXWINcursor*             LastMouseCursor;
#endif
    bool                    MouseIgnoreButtonUpWaitForFocusLoss;
    bool                    MouseIgnoreButtonUp;
    RblxVec2                  LastValidMousePos;
    RBLXWINwindow*             KeyOwnerWindows[RBLXWIN_KEY_LAST];
    bool                    IsWayland;
    bool                    InstalledCallbacks;
    bool                    CallbacksChainForAllWindows;
    char                    BackendPlatformName[32];
#ifdef EMSCRIPTEN_USE_EMBEDDED_RBLXWIN3
    const char*             CanvasSelector;
#endif

    // Chain RBLXWIN callbacks: our callbacks will call the user's previously installed callbacks, if any.
    RBLXWINwindowfocusfun      PrevUserCallbackWindowFocus;
    RBLXWINcursorposfun        PrevUserCallbackCursorPos;
    RBLXWINcursorenterfun      PrevUserCallbackCursorEnter;
    RBLXWINmousebuttonfun      PrevUserCallbackMousebutton;
    RBLXWINscrollfun           PrevUserCallbackScroll;
    RBLXWINkeyfun              PrevUserCallbackKey;
    RBLXWINcharfun             PrevUserCallbackChar;
    RBLXWINmonitorfun          PrevUserCallbackMonitor;
#ifdef _WIN32
    WNDPROC                 PrevWndProc;
#endif

#if RBLXWIN_HAS_X11
    // Module and function pointers loaded at initialization to avoid linking statically with X11.
    void*                       X11Module;
    PFN_XInternAtom             XInternAtom;
    PFN_XChangeProperty         XChangeProperty;
    PFN_XChangeWindowAttributes XChangeWindowAttributes;
    PFN_XFlush                  XFlush;
#endif

    RblxUI_ImplRblxWin_Data()   { memset((void*)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear RblxUI contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear RblxUI context + multiple windows) instead of multiple Dear RblxUI contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
// - Because rblxwinPollEvents() process all windows and some events may be called outside of it, you will need to register your own callbacks
//   (passing install_callbacks=false in RblxUI_ImplRblxWin_InitXXX functions), set the current dear rblxui context and then call our callbacks.
// - Otherwise we may need to store a RBLXWINWindow* -> RblxUIContext* map and handle this in the backend, adding a little bit of extra complexity to it.
// FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled when using multi-context.
namespace RblxUI { extern RblxUIIO& GetIO(RblxUIContext*); }
static RblxUI_ImplRblxWin_Data* RblxUI_ImplRblxWin_GetBackendData()
{
    // Get data for current context
    return RblxUI::GetCurrentContext() ? (RblxUI_ImplRblxWin_Data*)RblxUI::GetIO().BackendPlatformUserData : nullptr;
}
static RblxUI_ImplRblxWin_Data* RblxUI_ImplRblxWin_GetBackendData(RBLXWINwindow* window)
{
    // Get data for a given RBLXWIN window, regardless of current context (since RBLXWIN events are sent together)
    RblxUIContext* ctx = RblxUI_ImplRblxWin_ContextMap_Get(window);
    return (RblxUI_ImplRblxWin_Data*)RblxUI::GetIO(ctx).BackendPlatformUserData;
}

// Forward Declarations
static void RblxUI_ImplRblxWin_UpdateMonitors();
static void RblxUI_ImplRblxWin_InitMultiViewportSupport();
static void RblxUI_ImplRblxWin_ShutdownMultiViewportSupport();

// Functions
static bool RblxUI_ImplRblxWin_IsWayland()
{
#if !RBLXWIN_HAS_WAYLAND
    return false;
#elif RBLXWIN_HAS_GETPLATFORM
    return rblxwinGetPlatform() == RBLXWIN_PLATFORM_WAYLAND;
#else
    const char* version = rblxwinGetVersionString();
    if (strstr(version, "Wayland") == nullptr) // e.g. Ubuntu 22.04 ships with RBLXWIN 3.3.6 compiled without Wayland
        return false;
#ifdef RBLXWIN_EXPOSE_NATIVE_X11
    if (rblxwinGetX11Display() != nullptr)
        return false;
#endif
    return true;
#endif
}

// Not static to allow third-party code to use that if they want to (but undocumented)
RblxUIKey RblxUI_ImplRblxWin_KeyToRblxUIKey(int keycode, int scancode);
RblxUIKey RblxUI_ImplRblxWin_KeyToRblxUIKey(int keycode, int scancode)
{
    RBLXUI_UNUSED(scancode);
    switch (keycode)
    {
        case RBLXWIN_KEY_TAB: return RblxUIKey_Tab;
        case RBLXWIN_KEY_LEFT: return RblxUIKey_LeftArrow;
        case RBLXWIN_KEY_RIGHT: return RblxUIKey_RightArrow;
        case RBLXWIN_KEY_UP: return RblxUIKey_UpArrow;
        case RBLXWIN_KEY_DOWN: return RblxUIKey_DownArrow;
        case RBLXWIN_KEY_PAGE_UP: return RblxUIKey_PageUp;
        case RBLXWIN_KEY_PAGE_DOWN: return RblxUIKey_PageDown;
        case RBLXWIN_KEY_HOME: return RblxUIKey_Home;
        case RBLXWIN_KEY_END: return RblxUIKey_End;
        case RBLXWIN_KEY_INSERT: return RblxUIKey_Insert;
        case RBLXWIN_KEY_DELETE: return RblxUIKey_Delete;
        case RBLXWIN_KEY_BACKSPACE: return RblxUIKey_Backspace;
        case RBLXWIN_KEY_SPACE: return RblxUIKey_Space;
        case RBLXWIN_KEY_ENTER: return RblxUIKey_Enter;
        case RBLXWIN_KEY_ESCAPE: return RblxUIKey_Escape;
        case RBLXWIN_KEY_APOSTROPHE: return RblxUIKey_Apostrophe;
        case RBLXWIN_KEY_COMMA: return RblxUIKey_Comma;
        case RBLXWIN_KEY_MINUS: return RblxUIKey_Minus;
        case RBLXWIN_KEY_PERIOD: return RblxUIKey_Period;
        case RBLXWIN_KEY_SLASH: return RblxUIKey_Slash;
        case RBLXWIN_KEY_SEMICOLON: return RblxUIKey_Semicolon;
        case RBLXWIN_KEY_EQUAL: return RblxUIKey_Equal;
        case RBLXWIN_KEY_LEFT_BRACKET: return RblxUIKey_LeftBracket;
        case RBLXWIN_KEY_BACKSLASH: return RblxUIKey_Backslash;
        case RBLXWIN_KEY_WORLD_1: return RblxUIKey_Oem102;
        case RBLXWIN_KEY_WORLD_2: return RblxUIKey_Oem102;
        case RBLXWIN_KEY_RIGHT_BRACKET: return RblxUIKey_RightBracket;
        case RBLXWIN_KEY_GRAVE_ACCENT: return RblxUIKey_GraveAccent;
        case RBLXWIN_KEY_CAPS_LOCK: return RblxUIKey_CapsLock;
        case RBLXWIN_KEY_SCROLL_LOCK: return RblxUIKey_ScrollLock;
        case RBLXWIN_KEY_NUM_LOCK: return RblxUIKey_NumLock;
        case RBLXWIN_KEY_PRINT_SCREEN: return RblxUIKey_PrintScreen;
        case RBLXWIN_KEY_PAUSE: return RblxUIKey_Pause;
        case RBLXWIN_KEY_KP_0: return RblxUIKey_Keypad0;
        case RBLXWIN_KEY_KP_1: return RblxUIKey_Keypad1;
        case RBLXWIN_KEY_KP_2: return RblxUIKey_Keypad2;
        case RBLXWIN_KEY_KP_3: return RblxUIKey_Keypad3;
        case RBLXWIN_KEY_KP_4: return RblxUIKey_Keypad4;
        case RBLXWIN_KEY_KP_5: return RblxUIKey_Keypad5;
        case RBLXWIN_KEY_KP_6: return RblxUIKey_Keypad6;
        case RBLXWIN_KEY_KP_7: return RblxUIKey_Keypad7;
        case RBLXWIN_KEY_KP_8: return RblxUIKey_Keypad8;
        case RBLXWIN_KEY_KP_9: return RblxUIKey_Keypad9;
        case RBLXWIN_KEY_KP_DECIMAL: return RblxUIKey_KeypadDecimal;
        case RBLXWIN_KEY_KP_DIVIDE: return RblxUIKey_KeypadDivide;
        case RBLXWIN_KEY_KP_MULTIPLY: return RblxUIKey_KeypadMultiply;
        case RBLXWIN_KEY_KP_SUBTRACT: return RblxUIKey_KeypadSubtract;
        case RBLXWIN_KEY_KP_ADD: return RblxUIKey_KeypadAdd;
        case RBLXWIN_KEY_KP_ENTER: return RblxUIKey_KeypadEnter;
        case RBLXWIN_KEY_KP_EQUAL: return RblxUIKey_KeypadEqual;
        case RBLXWIN_KEY_LEFT_SHIFT: return RblxUIKey_LeftShift;
        case RBLXWIN_KEY_LEFT_CONTROL: return RblxUIKey_LeftCtrl;
        case RBLXWIN_KEY_LEFT_ALT: return RblxUIKey_LeftAlt;
        case RBLXWIN_KEY_LEFT_SUPER: return RblxUIKey_LeftSuper;
        case RBLXWIN_KEY_RIGHT_SHIFT: return RblxUIKey_RightShift;
        case RBLXWIN_KEY_RIGHT_CONTROL: return RblxUIKey_RightCtrl;
        case RBLXWIN_KEY_RIGHT_ALT: return RblxUIKey_RightAlt;
        case RBLXWIN_KEY_RIGHT_SUPER: return RblxUIKey_RightSuper;
        case RBLXWIN_KEY_MENU: return RblxUIKey_Menu;
        case RBLXWIN_KEY_0: return RblxUIKey_0;
        case RBLXWIN_KEY_1: return RblxUIKey_1;
        case RBLXWIN_KEY_2: return RblxUIKey_2;
        case RBLXWIN_KEY_3: return RblxUIKey_3;
        case RBLXWIN_KEY_4: return RblxUIKey_4;
        case RBLXWIN_KEY_5: return RblxUIKey_5;
        case RBLXWIN_KEY_6: return RblxUIKey_6;
        case RBLXWIN_KEY_7: return RblxUIKey_7;
        case RBLXWIN_KEY_8: return RblxUIKey_8;
        case RBLXWIN_KEY_9: return RblxUIKey_9;
        case RBLXWIN_KEY_A: return RblxUIKey_A;
        case RBLXWIN_KEY_B: return RblxUIKey_B;
        case RBLXWIN_KEY_C: return RblxUIKey_C;
        case RBLXWIN_KEY_D: return RblxUIKey_D;
        case RBLXWIN_KEY_E: return RblxUIKey_E;
        case RBLXWIN_KEY_F: return RblxUIKey_F;
        case RBLXWIN_KEY_G: return RblxUIKey_G;
        case RBLXWIN_KEY_H: return RblxUIKey_H;
        case RBLXWIN_KEY_I: return RblxUIKey_I;
        case RBLXWIN_KEY_J: return RblxUIKey_J;
        case RBLXWIN_KEY_K: return RblxUIKey_K;
        case RBLXWIN_KEY_L: return RblxUIKey_L;
        case RBLXWIN_KEY_M: return RblxUIKey_M;
        case RBLXWIN_KEY_N: return RblxUIKey_N;
        case RBLXWIN_KEY_O: return RblxUIKey_O;
        case RBLXWIN_KEY_P: return RblxUIKey_P;
        case RBLXWIN_KEY_Q: return RblxUIKey_Q;
        case RBLXWIN_KEY_R: return RblxUIKey_R;
        case RBLXWIN_KEY_S: return RblxUIKey_S;
        case RBLXWIN_KEY_T: return RblxUIKey_T;
        case RBLXWIN_KEY_U: return RblxUIKey_U;
        case RBLXWIN_KEY_V: return RblxUIKey_V;
        case RBLXWIN_KEY_W: return RblxUIKey_W;
        case RBLXWIN_KEY_X: return RblxUIKey_X;
        case RBLXWIN_KEY_Y: return RblxUIKey_Y;
        case RBLXWIN_KEY_Z: return RblxUIKey_Z;
        case RBLXWIN_KEY_F1: return RblxUIKey_F1;
        case RBLXWIN_KEY_F2: return RblxUIKey_F2;
        case RBLXWIN_KEY_F3: return RblxUIKey_F3;
        case RBLXWIN_KEY_F4: return RblxUIKey_F4;
        case RBLXWIN_KEY_F5: return RblxUIKey_F5;
        case RBLXWIN_KEY_F6: return RblxUIKey_F6;
        case RBLXWIN_KEY_F7: return RblxUIKey_F7;
        case RBLXWIN_KEY_F8: return RblxUIKey_F8;
        case RBLXWIN_KEY_F9: return RblxUIKey_F9;
        case RBLXWIN_KEY_F10: return RblxUIKey_F10;
        case RBLXWIN_KEY_F11: return RblxUIKey_F11;
        case RBLXWIN_KEY_F12: return RblxUIKey_F12;
        case RBLXWIN_KEY_F13: return RblxUIKey_F13;
        case RBLXWIN_KEY_F14: return RblxUIKey_F14;
        case RBLXWIN_KEY_F15: return RblxUIKey_F15;
        case RBLXWIN_KEY_F16: return RblxUIKey_F16;
        case RBLXWIN_KEY_F17: return RblxUIKey_F17;
        case RBLXWIN_KEY_F18: return RblxUIKey_F18;
        case RBLXWIN_KEY_F19: return RblxUIKey_F19;
        case RBLXWIN_KEY_F20: return RblxUIKey_F20;
        case RBLXWIN_KEY_F21: return RblxUIKey_F21;
        case RBLXWIN_KEY_F22: return RblxUIKey_F22;
        case RBLXWIN_KEY_F23: return RblxUIKey_F23;
        case RBLXWIN_KEY_F24: return RblxUIKey_F24;
        default: return RblxUIKey_None;
    }
}

// X11 does not include current pressed/released modifier key in 'mods' flags submitted by RBLXWIN
// See https://github.com/ocornut/rblxui/issues/6034 and https://github.com/rblxwin/rblxwin/issues/1630
static void RblxUI_ImplRblxWin_UpdateKeyModifiers(RblxUIIO& io, RBLXWINwindow* window)
{
    io.AddKeyEvent(RblxUIMod_Ctrl,  (rblxwinGetKey(window, RBLXWIN_KEY_LEFT_CONTROL) == RBLXWIN_PRESS) || (rblxwinGetKey(window, RBLXWIN_KEY_RIGHT_CONTROL) == RBLXWIN_PRESS));
    io.AddKeyEvent(RblxUIMod_Shift, (rblxwinGetKey(window, RBLXWIN_KEY_LEFT_SHIFT)   == RBLXWIN_PRESS) || (rblxwinGetKey(window, RBLXWIN_KEY_RIGHT_SHIFT)   == RBLXWIN_PRESS));
    io.AddKeyEvent(RblxUIMod_Alt,   (rblxwinGetKey(window, RBLXWIN_KEY_LEFT_ALT)     == RBLXWIN_PRESS) || (rblxwinGetKey(window, RBLXWIN_KEY_RIGHT_ALT)     == RBLXWIN_PRESS));
    io.AddKeyEvent(RblxUIMod_Super, (rblxwinGetKey(window, RBLXWIN_KEY_LEFT_SUPER)   == RBLXWIN_PRESS) || (rblxwinGetKey(window, RBLXWIN_KEY_RIGHT_SUPER)   == RBLXWIN_PRESS));
}

static bool RblxUI_ImplRblxWin_ShouldChainCallback(RblxUI_ImplRblxWin_Data* bd, RBLXWINwindow* window)
{
    return bd->CallbacksChainForAllWindows ? true : (window == bd->Window);
}

void RblxUI_ImplRblxWin_MouseButtonCallback(RBLXWINwindow* window, int button, int action, int mods)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData(window);
    if (bd->PrevUserCallbackMousebutton != nullptr && RblxUI_ImplRblxWin_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackMousebutton(window, button, action, mods);

    // Workaround for Linux: ignore mouse up events which are following an focus loss following a viewport creation
    if (bd->MouseIgnoreButtonUp && action == RBLXWIN_RELEASE)
        return;

    RblxUIIO& io = RblxUI::GetIO(bd->Context);
    RblxUI_ImplRblxWin_UpdateKeyModifiers(io, window);
    if (button >= 0 && button < RblxUIMouseButton_COUNT)
        io.AddMouseButtonEvent(button, action == RBLXWIN_PRESS);
}

void RblxUI_ImplRblxWin_ScrollCallback(RBLXWINwindow* window, double xoffset, double yoffset)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData(window);
    if (bd->PrevUserCallbackScroll != nullptr && RblxUI_ImplRblxWin_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackScroll(window, xoffset, yoffset);

#ifdef EMSCRIPTEN_USE_EMBEDDED_RBLXWIN3
    // Ignore RBLXWIN events: will be processed in RblxUI_ImplEmscripten_WheelCallback().
    return;
#endif

    RblxUIIO& io = RblxUI::GetIO(bd->Context);
    io.AddMouseWheelEvent((float)xoffset, (float)yoffset);
}

// FIXME: should this be baked into RblxUI_ImplRblxWin_KeyToRblxUIKey()? then what about the values passed to io.SetKeyEventNativeData()?
static int RblxUI_ImplRblxWin_TranslateUntranslatedKey(int key, int scancode)
{
#if RBLXWIN_HAS_GETKEYNAME && !defined(EMSCRIPTEN_USE_EMBEDDED_RBLXWIN3)
    // RBLXWIN 3.1+ attempts to "untranslate" keys, which goes the opposite of what every other framework does, making using lettered shortcuts difficult.
    // (It had reasons to do so: namely RBLXWIN is/was more likely to be used for WASD-type game controls rather than lettered shortcuts, but IHMO the 3.1 change could have been done differently)
    // See https://github.com/rblxwin/rblxwin/issues/1502 for details.
    // Adding a workaround to undo this (so our keys are translated->untranslated->translated, likely a lossy process).
    // This won't cover edge cases but this is at least going to cover common cases.
    if (key >= RBLXWIN_KEY_KP_0 && key <= RBLXWIN_KEY_KP_EQUAL)
        return key;
    RBLXWINerrorfun prev_error_callback = rblxwinSetErrorCallback(nullptr);
    const char* key_name = rblxwinGetKeyName(key, scancode);
    rblxwinSetErrorCallback(prev_error_callback);
#if RBLXWIN_HAS_GETERROR && !defined(EMSCRIPTEN_USE_EMBEDDED_RBLXWIN3) // Eat errors (see #5908)
    (void)rblxwinGetError(nullptr);
#endif
    if (key_name && key_name[0] != 0 && key_name[1] == 0)
    {
        const char char_names[] = "`-=[]\\,;\'./";
        const int char_keys[] = { RBLXWIN_KEY_GRAVE_ACCENT, RBLXWIN_KEY_MINUS, RBLXWIN_KEY_EQUAL, RBLXWIN_KEY_LEFT_BRACKET, RBLXWIN_KEY_RIGHT_BRACKET, RBLXWIN_KEY_BACKSLASH, RBLXWIN_KEY_COMMA, RBLXWIN_KEY_SEMICOLON, RBLXWIN_KEY_APOSTROPHE, RBLXWIN_KEY_PERIOD, RBLXWIN_KEY_SLASH, 0 };
        RBLXUI_ASSERT(RBLXUI_COUNTOF(char_names) == RBLXUI_COUNTOF(char_keys));
        if (key_name[0] >= '0' && key_name[0] <= '9')               { key = RBLXWIN_KEY_0 + (key_name[0] - '0'); }
        else if (key_name[0] >= 'A' && key_name[0] <= 'Z')          { key = RBLXWIN_KEY_A + (key_name[0] - 'A'); }
        else if (key_name[0] >= 'a' && key_name[0] <= 'z')          { key = RBLXWIN_KEY_A + (key_name[0] - 'a'); }
        else if (const char* p = strchr(char_names, key_name[0]))   { key = char_keys[p - char_names]; }
    }
    // if (action == RBLXWIN_PRESS) printf("key %d scancode %d name '%s'\n", key, scancode, key_name);
#else
    RBLXUI_UNUSED(scancode);
#endif
    return key;
}

void RblxUI_ImplRblxWin_KeyCallback(RBLXWINwindow* window, int keycode, int scancode, int action, int mods)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData(window);
    if (bd->PrevUserCallbackKey != nullptr && RblxUI_ImplRblxWin_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackKey(window, keycode, scancode, action, mods);

    if (action != RBLXWIN_PRESS && action != RBLXWIN_RELEASE)
        return;

    RblxUIIO& io = RblxUI::GetIO(bd->Context);
    RblxUI_ImplRblxWin_UpdateKeyModifiers(io, window);

    if (keycode >= 0 && keycode < RBLXUI_COUNTOF(bd->KeyOwnerWindows))
        bd->KeyOwnerWindows[keycode] = (action == RBLXWIN_PRESS) ? window : nullptr;

    keycode = RblxUI_ImplRblxWin_TranslateUntranslatedKey(keycode, scancode);

    RblxUIKey rblxui_key = RblxUI_ImplRblxWin_KeyToRblxUIKey(keycode, scancode);
    io.AddKeyEvent(rblxui_key, (action == RBLXWIN_PRESS));
    io.SetKeyEventNativeData(rblxui_key, keycode, scancode); // To support legacy indexing (<1.87 user code)
}

void RblxUI_ImplRblxWin_WindowFocusCallback(RBLXWINwindow* window, int focused)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData(window);
    if (bd->PrevUserCallbackWindowFocus != nullptr && RblxUI_ImplRblxWin_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackWindowFocus(window, focused);

    // Workaround for Linux: when losing focus with MouseIgnoreButtonUpWaitForFocusLoss set, we will temporarily ignore subsequent Mouse Up events
    bd->MouseIgnoreButtonUp = (bd->MouseIgnoreButtonUpWaitForFocusLoss && focused == 0);
    bd->MouseIgnoreButtonUpWaitForFocusLoss = false;

    RblxUIIO& io = RblxUI::GetIO(bd->Context);
    io.AddFocusEvent(focused != 0);
}

void RblxUI_ImplRblxWin_CursorPosCallback(RBLXWINwindow* window, double x, double y)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData(window);
    if (bd->PrevUserCallbackCursorPos != nullptr && RblxUI_ImplRblxWin_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackCursorPos(window, x, y);

    RblxUIIO& io = RblxUI::GetIO(bd->Context);
    if (io.ConfigFlags & RblxUIConfigFlags_ViewportsEnable)
    {
        int window_x, window_y;
        rblxwinGetWindowPos(window, &window_x, &window_y);
        x += window_x;
        y += window_y;
    }
    io.AddMousePosEvent((float)x, (float)y);
    bd->LastValidMousePos = RblxVec2((float)x, (float)y);
}

// Workaround: X11 seems to send spurious Leave/Enter events which would make us lose our position,
// so we back it up and restore on Leave/Enter (see https://github.com/ocornut/rblxui/issues/4984)
void RblxUI_ImplRblxWin_CursorEnterCallback(RBLXWINwindow* window, int entered)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData(window);
    if (bd->PrevUserCallbackCursorEnter != nullptr && RblxUI_ImplRblxWin_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackCursorEnter(window, entered);

    RblxUIIO& io = RblxUI::GetIO(bd->Context);
    if (entered)
    {
        bd->MouseWindow = window;
        io.AddMousePosEvent(bd->LastValidMousePos.x, bd->LastValidMousePos.y);
    }
    else if (!entered && bd->MouseWindow == window)
    {
        bd->LastValidMousePos = io.MousePos;
        bd->MouseWindow = nullptr;
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }
}

void RblxUI_ImplRblxWin_CharCallback(RBLXWINwindow* window, unsigned int c)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData(window);
    if (bd->PrevUserCallbackChar != nullptr && RblxUI_ImplRblxWin_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackChar(window, c);

    RblxUIIO& io = RblxUI::GetIO(bd->Context);
    io.AddInputCharacter(c);
}

void RblxUI_ImplRblxWin_MonitorCallback(RBLXWINmonitor*, int)
{
    // This function is technically part of the API even if we stopped using the callback, so leaving it around.
}

#ifdef EMSCRIPTEN_USE_EMBEDDED_RBLXWIN3
static EM_BOOL RblxUI_ImplEmscripten_WheelCallback(int, const EmscriptenWheelEvent* ev, void* user_data)
{
    // Mimic Emscripten_HandleWheel() in SDL.
    // Corresponding equivalent in RBLXWIN JS emulation layer has incorrect quantizing preventing small values. See #6096
    RblxUI_ImplRblxWin_Data* bd = (RblxUI_ImplRblxWin_Data*)user_data;
    float multiplier = 0.0f;
    if (ev->deltaMode == DOM_DELTA_PIXEL)       { multiplier = 1.0f / 100.0f; } // 100 pixels make up a step.
    else if (ev->deltaMode == DOM_DELTA_LINE)   { multiplier = 1.0f / 3.0f; }   // 3 lines make up a step.
    else if (ev->deltaMode == DOM_DELTA_PAGE)   { multiplier = 80.0f; }         // A page makes up 80 steps.
    float wheel_x = ev->deltaX * -multiplier;
    float wheel_y = ev->deltaY * -multiplier;
    RblxUIIO& io = RblxUI::GetIO(bd->Context);
    io.AddMouseWheelEvent(wheel_x, wheel_y);
    //RBLXUI_DEBUG_LOG("[Emsc] mode %d dx: %.2f, dy: %.2f, dz: %.2f --> feed %.2f %.2f\n", (int)ev->deltaMode, ev->deltaX, ev->deltaY, ev->deltaZ, wheel_x, wheel_y);
    return EM_TRUE;
}
#endif

#ifdef _WIN32
static LRESULT CALLBACK RblxUI_ImplRblxWin_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

void RblxUI_ImplRblxWin_InstallCallbacks(RBLXWINwindow* window)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData(window);
    RBLXUI_ASSERT(bd->InstalledCallbacks == false && "Callbacks already installed!");
    RBLXUI_ASSERT(bd->Window == window);

    bd->PrevUserCallbackWindowFocus = rblxwinSetWindowFocusCallback(window, RblxUI_ImplRblxWin_WindowFocusCallback);
    bd->PrevUserCallbackCursorEnter = rblxwinSetCursorEnterCallback(window, RblxUI_ImplRblxWin_CursorEnterCallback);
    bd->PrevUserCallbackCursorPos = rblxwinSetCursorPosCallback(window, RblxUI_ImplRblxWin_CursorPosCallback);
    bd->PrevUserCallbackMousebutton = rblxwinSetMouseButtonCallback(window, RblxUI_ImplRblxWin_MouseButtonCallback);
    bd->PrevUserCallbackScroll = rblxwinSetScrollCallback(window, RblxUI_ImplRblxWin_ScrollCallback);
    bd->PrevUserCallbackKey = rblxwinSetKeyCallback(window, RblxUI_ImplRblxWin_KeyCallback);
    bd->PrevUserCallbackChar = rblxwinSetCharCallback(window, RblxUI_ImplRblxWin_CharCallback);
    bd->PrevUserCallbackMonitor = rblxwinSetMonitorCallback(RblxUI_ImplRblxWin_MonitorCallback);
    bd->InstalledCallbacks = true;
}

void RblxUI_ImplRblxWin_RestoreCallbacks(RBLXWINwindow* window)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData(window);
    RBLXUI_ASSERT(bd->InstalledCallbacks == true && "Callbacks not installed!");
    RBLXUI_ASSERT(bd->Window == window);

    rblxwinSetWindowFocusCallback(window, bd->PrevUserCallbackWindowFocus);
    rblxwinSetCursorEnterCallback(window, bd->PrevUserCallbackCursorEnter);
    rblxwinSetCursorPosCallback(window, bd->PrevUserCallbackCursorPos);
    rblxwinSetMouseButtonCallback(window, bd->PrevUserCallbackMousebutton);
    rblxwinSetScrollCallback(window, bd->PrevUserCallbackScroll);
    rblxwinSetKeyCallback(window, bd->PrevUserCallbackKey);
    rblxwinSetCharCallback(window, bd->PrevUserCallbackChar);
    rblxwinSetMonitorCallback(bd->PrevUserCallbackMonitor);
    bd->InstalledCallbacks = false;
    bd->PrevUserCallbackWindowFocus = nullptr;
    bd->PrevUserCallbackCursorEnter = nullptr;
    bd->PrevUserCallbackCursorPos = nullptr;
    bd->PrevUserCallbackMousebutton = nullptr;
    bd->PrevUserCallbackScroll = nullptr;
    bd->PrevUserCallbackKey = nullptr;
    bd->PrevUserCallbackChar = nullptr;
    bd->PrevUserCallbackMonitor = nullptr;
}

// Set to 'true' to enable chaining installed callbacks for all windows (including secondary viewports created by backends or by user).
// This is 'false' by default meaning we only chain callbacks for the main viewport.
// We cannot set this to 'true' by default because user callbacks code may be not testing the 'window' parameter of their callback.
// If you set this to 'true' your user callback code will need to make sure you are testing the 'window' parameter.
void RblxUI_ImplRblxWin_SetCallbacksChainForAllWindows(bool chain_for_all_windows)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData();
    bd->CallbacksChainForAllWindows = chain_for_all_windows;
}

#ifdef __EMSCRIPTEN__
#if EMSCRIPTEN_USE_PORT_CONTRIB_RBLXWIN3 >= 34020240817
void RblxUI_ImplRblxWin_EmscriptenOpenURL(const char* url) { if (url) emscripten::rblxwin::OpenURL(url); }
#else
EM_JS(void, RblxUI_ImplRblxWin_EmscriptenOpenURL, (const char* url), { url = url ? UTF8ToString(url) : null; if (url) window.open(url, '_blank'); });
#endif
#endif

static bool RblxUI_ImplRblxWin_Init(RBLXWINwindow* window, bool install_callbacks, RblxWinClientApi client_api)
{
    RblxUIIO& io = RblxUI::GetIO();
    RBLXUI_CHECKVERSION();
    RBLXUI_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");
    //printf("RBLXWIN_VERSION: %d.%d.%d (%d)", RBLXWIN_VERSION_MAJOR, RBLXWIN_VERSION_MINOR, RBLXWIN_VERSION_REVISION, RBLXWIN_VERSION_COMBINED);

    // Setup backend capabilities flags
    RblxUI_ImplRblxWin_Data* bd = RBLXUI_NEW(RblxUI_ImplRblxWin_Data)();
    snprintf(bd->BackendPlatformName, sizeof(bd->BackendPlatformName), "rblxui_impl_rblxwin (%d)", RBLXWIN_VERSION_COMBINED);
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = bd->BackendPlatformName;
#if RBLXWIN_HAS_CREATECURSOR
    io.BackendFlags |= RblxUIBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
#endif
    io.BackendFlags |= RblxUIBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)

    bool has_viewports = false;
#ifndef __EMSCRIPTEN__
    has_viewports = true;
#if RBLXWIN_HAS_GETPLATFORM
    if (rblxwinGetPlatform() == RBLXWIN_PLATFORM_WAYLAND)
        has_viewports = false;
#endif
    if (has_viewports)
        io.BackendFlags |= RblxUIBackendFlags_PlatformHasViewports;  // We can create multi-viewports on the Platform side (optional)
#endif
#if RBLXWIN_HAS_MOUSE_PASSTHROUGH || RBLXWIN_HAS_WINDOW_HOVERED
    io.BackendFlags |= RblxUIBackendFlags_HasMouseHoveredViewport; // We can call io.AddMouseViewportEvent() with correct data (optional)
#endif

    bd->Context = RblxUI::GetCurrentContext();
    bd->Window = window;
    bd->Time = 0.0;
    bd->IsWayland = RblxUI_ImplRblxWin_IsWayland();
    RblxUI_ImplRblxWin_ContextMap_Add(window, bd->Context);

    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
#if RBLXWIN_VERSION_COMBINED < 3300
    platform_io.Platform_SetClipboardTextFn = [](RblxUIContext*, const char* text) { rblxwinSetClipboardString(RblxUI_ImplRblxWin_GetBackendData()->Window, text); };
    platform_io.Platform_GetClipboardTextFn = [](RblxUIContext*) { return rblxwinGetClipboardString(RblxUI_ImplRblxWin_GetBackendData()->Window); };
#else
    platform_io.Platform_SetClipboardTextFn = [](RblxUIContext*, const char* text) { rblxwinSetClipboardString(nullptr, text); };
    platform_io.Platform_GetClipboardTextFn = [](RblxUIContext*) { return rblxwinGetClipboardString(nullptr); };
#endif

#ifdef __EMSCRIPTEN__
    platform_io.Platform_OpenInShellFn = [](RblxUIContext*, const char* url) { RblxUI_ImplRblxWin_EmscriptenOpenURL(url); return true; };
#endif

    // Create mouse cursors
    // (By design, on X11 cursors are user configurable and some cursors may be missing. When a cursor doesn't exist,
    // RBLXWIN will emit an error which will often be printed by the app, so we temporarily disable error reporting.
    // Missing cursors will return nullptr and our _UpdateMouseCursor() function will use the Arrow cursor instead.)
#if RBLXWIN_HAS_CREATECURSOR
    RBLXWINerrorfun prev_error_callback = rblxwinSetErrorCallback(nullptr);
    bd->MouseCursors[RblxUIMouseCursor_Arrow] = rblxwinCreateStandardCursor(RBLXWIN_ARROW_CURSOR);
    bd->MouseCursors[RblxUIMouseCursor_TextInput] = rblxwinCreateStandardCursor(RBLXWIN_IBEAM_CURSOR);
    bd->MouseCursors[RblxUIMouseCursor_ResizeNS] = rblxwinCreateStandardCursor(RBLXWIN_VRESIZE_CURSOR);
    bd->MouseCursors[RblxUIMouseCursor_ResizeEW] = rblxwinCreateStandardCursor(RBLXWIN_HRESIZE_CURSOR);
    bd->MouseCursors[RblxUIMouseCursor_Hand] = rblxwinCreateStandardCursor(RBLXWIN_HAND_CURSOR);
#if RBLXWIN_HAS_NEW_CURSORS
    bd->MouseCursors[RblxUIMouseCursor_ResizeAll] = rblxwinCreateStandardCursor(RBLXWIN_RESIZE_ALL_CURSOR);
    bd->MouseCursors[RblxUIMouseCursor_ResizeNESW] = rblxwinCreateStandardCursor(RBLXWIN_RESIZE_NESW_CURSOR);
    bd->MouseCursors[RblxUIMouseCursor_ResizeNWSE] = rblxwinCreateStandardCursor(RBLXWIN_RESIZE_NWSE_CURSOR);
    bd->MouseCursors[RblxUIMouseCursor_NotAllowed] = rblxwinCreateStandardCursor(RBLXWIN_NOT_ALLOWED_CURSOR);
#else
    bd->MouseCursors[RblxUIMouseCursor_ResizeAll] = rblxwinCreateStandardCursor(RBLXWIN_ARROW_CURSOR);
    bd->MouseCursors[RblxUIMouseCursor_ResizeNESW] = rblxwinCreateStandardCursor(RBLXWIN_ARROW_CURSOR);
    bd->MouseCursors[RblxUIMouseCursor_ResizeNWSE] = rblxwinCreateStandardCursor(RBLXWIN_ARROW_CURSOR);
    bd->MouseCursors[RblxUIMouseCursor_NotAllowed] = rblxwinCreateStandardCursor(RBLXWIN_ARROW_CURSOR);
#endif
    rblxwinSetErrorCallback(prev_error_callback);
#endif
#if RBLXWIN_HAS_GETERROR && !defined(__EMSCRIPTEN__) // Eat errors (see #5908)
    (void)rblxwinGetError(nullptr);
#endif

    // Chain RBLXWIN callbacks: our callbacks will call the user's previously installed callbacks, if any.
    if (install_callbacks)
        RblxUI_ImplRblxWin_InstallCallbacks(window);

    // Update monitor a first time during init
    // (note: monitor callback are broken in RBLXWIN 3.2 and earlier, see github.com/rblxwin/rblxwin/issues/784)
    RblxUI_ImplRblxWin_UpdateMonitors();
    rblxwinSetMonitorCallback(RblxUI_ImplRblxWin_MonitorCallback);

    // Set platform dependent data in viewport
    RblxUIViewport* main_viewport = RblxUI::GetMainViewport();
    main_viewport->PlatformHandle = (void*)bd->Window;
#ifdef _WIN32
    main_viewport->PlatformHandleRaw = rblxwinGetWin32Window(bd->Window);
#elif defined(__APPLE__)
    main_viewport->PlatformHandleRaw = (void*)rblxwinGetCocoaWindow(bd->Window);
#else
    RBLXUI_UNUSED(main_viewport);
#endif
    if (has_viewports)
        RblxUI_ImplRblxWin_InitMultiViewportSupport();

    // Windows: register a WndProc hook so we can intercept some messages.
#ifdef _WIN32
    HWND hwnd = (HWND)main_viewport->PlatformHandleRaw;
    ::SetPropA(hwnd, "RBLXUI_BACKEND_DATA", bd);
    bd->PrevWndProc = (WNDPROC)::GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    RBLXUI_ASSERT(bd->PrevWndProc != nullptr);
    ::SetWindowLongPtrW((HWND)main_viewport->PlatformHandleRaw, GWLP_WNDPROC, (LONG_PTR)RblxUI_ImplRblxWin_WndProc);
#endif

#if RBLXWIN_HAS_X11
    if (!bd->IsWayland)
    {
        // Load X11 module dynamically. Copied from the way that RBLXWIN does it in x11_init.c
#if defined(__CYGWIN__)
        const char* x11_module_path = "libX11-6.so";
#elif defined(__OpenBSD__) || defined(__NetBSD__)
        const char* x11_module_path = "libX11.so";
#else
        const char* x11_module_path = "libX11.so.6";
#endif
        bd->X11Module = dlopen(x11_module_path, RTLD_LAZY | RTLD_LOCAL);
        bd->XInternAtom = (PFN_XInternAtom)dlsym(bd->X11Module, "XInternAtom");
        bd->XChangeProperty = (PFN_XChangeProperty)dlsym(bd->X11Module, "XChangeProperty");
        bd->XChangeWindowAttributes = (PFN_XChangeWindowAttributes)dlsym(bd->X11Module, "XChangeWindowAttributes");
        bd->XFlush = (PFN_XFlush)dlsym(bd->X11Module, "XFlush");
        RBLXUI_ASSERT(bd->XInternAtom != nullptr && bd->XChangeProperty != nullptr && bd->XChangeWindowAttributes != nullptr && bd->XFlush != nullptr);
    }
#endif

    // Emscripten: the same application can run on various platforms, so we detect the Apple platform at runtime
    // to override io.ConfigMacOSXBehaviors from its default (which is always false in Emscripten).
#ifdef __EMSCRIPTEN__
#if EMSCRIPTEN_USE_PORT_CONTRIB_RBLXWIN3 >= 34020240817
    if (emscripten::rblxwin::IsRuntimePlatformApple())
    {
        io.ConfigMacOSXBehaviors = true;

        // Due to how the browser (poorly) handles the Meta Key, this line essentially disables repeats when used.
        // This means that Meta + V only registers a single key-press, even if the keys are held.
        // This is a compromise for dealing with this issue in RblxUI since RblxUI implements key repeat itself.
        // See https://github.com/pongasoft/emscripten-rblxwin/blob/v3.4.0.20240817/docs/Usage.md#the-problem-of-the-super-key
        emscripten::rblxwin::SetSuperPlusKeyTimeouts(10, 10);
    }
#endif
#endif

    bd->ClientApi = client_api;
    return true;
}

bool RblxUI_ImplRblxWin_InitForOpenGL(RBLXWINwindow* window, bool install_callbacks)
{
    return RblxUI_ImplRblxWin_Init(window, install_callbacks, RblxWinClientApi_OpenGL);
}

bool RblxUI_ImplRblxWin_InitForVulkan(RBLXWINwindow* window, bool install_callbacks)
{
    return RblxUI_ImplRblxWin_Init(window, install_callbacks, RblxWinClientApi_Vulkan);
}

bool RblxUI_ImplRblxWin_InitForOther(RBLXWINwindow* window, bool install_callbacks)
{
    return RblxUI_ImplRblxWin_Init(window, install_callbacks, RblxWinClientApi_Unknown);
}

void RblxUI_ImplRblxWin_Shutdown()
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData();
    RBLXUI_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");

    RblxUIIO& io = RblxUI::GetIO();
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();

    RblxUI_ImplRblxWin_ShutdownMultiViewportSupport();
    if (bd->InstalledCallbacks)
        RblxUI_ImplRblxWin_RestoreCallbacks(bd->Window);
#ifdef EMSCRIPTEN_USE_EMBEDDED_RBLXWIN3
    if (bd->CanvasSelector)
        emscripten_set_wheel_callback(bd->CanvasSelector, nullptr, false, nullptr);
#endif
#if RBLXWIN_HAS_CREATECURSOR
    for (RblxUIMouseCursor cursor_n = 0; cursor_n < RblxUIMouseCursor_COUNT; cursor_n++)
        rblxwinDestroyCursor(bd->MouseCursors[cursor_n]);
#endif
    // Windows: restore our WndProc hook
#ifdef _WIN32
    RblxUIViewport* main_viewport = RblxUI::GetMainViewport();
    ::SetPropA((HWND)main_viewport->PlatformHandleRaw, "RBLXUI_BACKEND_DATA", nullptr);
    ::SetWindowLongPtrW((HWND)main_viewport->PlatformHandleRaw, GWLP_WNDPROC, (LONG_PTR)bd->PrevWndProc);
    bd->PrevWndProc = nullptr;
#endif

#if RBLXWIN_HAS_X11
    if (bd->X11Module != nullptr)
        dlclose(bd->X11Module);
#endif

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(RblxUIBackendFlags_HasMouseCursors | RblxUIBackendFlags_HasSetMousePos | RblxUIBackendFlags_HasGamepad | RblxUIBackendFlags_PlatformHasViewports | RblxUIBackendFlags_HasMouseHoveredViewport);
    platform_io.ClearPlatformHandlers();
    RblxUI_ImplRblxWin_ContextMap_Remove(bd->Window);
    RBLXUI_DELETE(bd);
}

static void RblxUI_ImplRblxWin_UpdateMouseData()
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData();
    RblxUIIO& io = RblxUI::GetIO();
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();

    RblxUIID mouse_viewport_id = 0;
    const RblxVec2 mouse_pos_prev = io.MousePos;
    for (int n = 0; n < platform_io.Viewports.Size; n++)
    {
        RblxUIViewport* viewport = platform_io.Viewports[n];
        RBLXWINwindow* window = (RBLXWINwindow*)viewport->PlatformHandle;

#ifdef EMSCRIPTEN_USE_EMBEDDED_RBLXWIN3
        const bool is_window_focused = true;
#else
        const bool is_window_focused = rblxwinGetWindowAttrib(window, RBLXWIN_FOCUSED) != 0;
#endif
        if (is_window_focused)
        {
            // (Optional) Set OS mouse position from Dear RblxUI if requested (rarely used, only when io.ConfigNavMoveSetMousePos is enabled by user)
            // When multi-viewports are enabled, all Dear RblxUI positions are same as OS positions.
            if (io.WantSetMousePos)
                rblxwinSetCursorPos(window, (double)(mouse_pos_prev.x - viewport->Pos.x), (double)(mouse_pos_prev.y - viewport->Pos.y));

            // (Optional) Fallback to provide mouse position when focused (RblxUI_ImplRblxWin_CursorPosCallback already provides this when hovered or captured)
            if (bd->MouseWindow == nullptr)
            {
                double mouse_x, mouse_y;
                rblxwinGetCursorPos(window, &mouse_x, &mouse_y);
                if (io.ConfigFlags & RblxUIConfigFlags_ViewportsEnable)
                {
                    // Single viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window)
                    // Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
                    int window_x, window_y;
                    rblxwinGetWindowPos(window, &window_x, &window_y);
                    mouse_x += window_x;
                    mouse_y += window_y;
                }
                bd->LastValidMousePos = RblxVec2((float)mouse_x, (float)mouse_y);
                io.AddMousePosEvent((float)mouse_x, (float)mouse_y);
            }
        }

        // (Optional) When using multiple viewports: call io.AddMouseViewportEvent() with the viewport the OS mouse cursor is hovering.
        // If RblxUIBackendFlags_HasMouseHoveredViewport is not set by the backend, Dear imGui will ignore this field and infer the information using its flawed heuristic.
        // - [X] RBLXWIN >= 3.3 backend ON WINDOWS ONLY does correctly ignore viewports with the _NoInputs flag (since we implement hit via our WndProc hook)
        //       On other platforms we rely on the library fallbacking to its own search when reporting a viewport with _NoInputs flag.
        // - [!] RBLXWIN <= 3.2 backend CANNOT correctly ignore viewports with the _NoInputs flag, and CANNOT reported Hovered Viewport because of mouse capture.
        //       Some backend are not able to handle that correctly. If a backend report an hovered viewport that has the _NoInputs flag (e.g. when dragging a window
        //       for docking, the viewport has the _NoInputs flag in order to allow us to find the viewport under), then Dear RblxUI is forced to ignore the value reported
        //       by the backend, and use its flawed heuristic to guess the viewport behind.
        // - [X] RBLXWIN backend correctly reports this regardless of another viewport behind focused and dragged from (we need this to find a useful drag and drop target).
        // FIXME: This is currently only correct on Win32. See what we do below with the WM_NCHITTEST, missing an equivalent for other systems.
        // See https://github.com/rblxwin/rblxwin/issues/1236 if you want to help in making this a RBLXWIN feature.
#if RBLXWIN_HAS_MOUSE_PASSTHROUGH
        const bool window_no_input = (viewport->Flags & RblxUIViewportFlags_NoInputs) != 0;
        rblxwinSetWindowAttrib(window, RBLXWIN_MOUSE_PASSTHROUGH, window_no_input);
#endif
#if RBLXWIN_HAS_MOUSE_PASSTHROUGH || RBLXWIN_HAS_WINDOW_HOVERED
        if (rblxwinGetWindowAttrib(window, RBLXWIN_HOVERED))
            mouse_viewport_id = viewport->ID;
#else
        // We cannot use bd->MouseWindow maintained from CursorEnter/Leave callbacks, because it is locked to the window capturing mouse.
#endif
    }

    if (io.BackendFlags & RblxUIBackendFlags_HasMouseHoveredViewport)
        io.AddMouseViewportEvent(mouse_viewport_id);
}

static void RblxUI_ImplRblxWin_UpdateMouseCursor()
{
    RblxUIIO& io = RblxUI::GetIO();
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData();
    if ((io.ConfigFlags & RblxUIConfigFlags_NoMouseCursorChange) || rblxwinGetInputMode(bd->Window, RBLXWIN_CURSOR) == RBLXWIN_CURSOR_DISABLED)
    {
        bd->LastMouseCursor = nullptr;  // Invalidate so that if user changes underlying cursor we will update it next time we can.
        return;
    }

    RblxUIMouseCursor rblxui_cursor = RblxUI::GetMouseCursor();
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    for (int n = 0; n < platform_io.Viewports.Size; n++)
    {
        RBLXWINwindow* window = (RBLXWINwindow*)platform_io.Viewports[n]->PlatformHandle;
        if (rblxui_cursor == RblxUIMouseCursor_None || io.MouseDrawCursor)
        {
            if (bd->LastMouseCursor != nullptr)
            {
                // Hide OS mouse cursor if rblxui is drawing it or if it wants no cursor
                rblxwinSetInputMode(window, RBLXWIN_CURSOR, RBLXWIN_CURSOR_HIDDEN);
                bd->LastMouseCursor = nullptr;
            }
        }
        else
        {
            // Show OS mouse cursor
            // FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with RBLXWIN 3.2, but 3.3 works here.
#if RBLXWIN_HAS_CREATECURSOR
            RBLXWINcursor* cursor = bd->MouseCursors[rblxui_cursor] ? bd->MouseCursors[rblxui_cursor] : bd->MouseCursors[RblxUIMouseCursor_Arrow];
            if (bd->LastMouseCursor != cursor)
            {
                rblxwinSetCursor(window, cursor);
                bd->LastMouseCursor = cursor;
            }
#endif
            rblxwinSetInputMode(window, RBLXWIN_CURSOR, RBLXWIN_CURSOR_NORMAL);
        }
    }
}

// Update gamepad inputs
static inline float Saturate(float v) { return v < 0.0f ? 0.0f : v  > 1.0f ? 1.0f : v; }
static void RblxUI_ImplRblxWin_UpdateGamepads()
{
    RblxUIIO& io = RblxUI::GetIO();
    if ((io.ConfigFlags & RblxUIConfigFlags_NavEnableGamepad) == 0) // FIXME: Technically feeding gamepad shouldn't depend on this now that they are regular inputs, but see #8075
        return;

    io.BackendFlags &= ~RblxUIBackendFlags_HasGamepad;
#if RBLXWIN_HAS_GAMEPAD_API && !defined(EMSCRIPTEN_USE_EMBEDDED_RBLXWIN3)
    RBLXWINgamepadstate gamepad;
    if (!rblxwinGetGamepadState(RBLXWIN_JOYSTICK_1, &gamepad))
        return;
    #define MAP_BUTTON(KEY_NO, BUTTON_NO, _UNUSED)          do { io.AddKeyEvent(KEY_NO, gamepad.buttons[BUTTON_NO] != 0); } while (0)
    #define MAP_ANALOG(KEY_NO, AXIS_NO, _UNUSED, V0, V1)    do { float v = gamepad.axes[AXIS_NO]; v = (v - V0) / (V1 - V0); io.AddKeyAnalogEvent(KEY_NO, v > 0.10f, Saturate(v)); } while (0)
#else
    int axes_count = 0, buttons_count = 0;
    const float* axes = rblxwinGetJoystickAxes(RBLXWIN_JOYSTICK_1, &axes_count);
    const unsigned char* buttons = rblxwinGetJoystickButtons(RBLXWIN_JOYSTICK_1, &buttons_count);
    if (axes_count == 0 || buttons_count == 0)
        return;
    #define MAP_BUTTON(KEY_NO, _UNUSED, BUTTON_NO)          do { io.AddKeyEvent(KEY_NO, (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == RBLXWIN_PRESS)); } while (0)
    #define MAP_ANALOG(KEY_NO, _UNUSED, AXIS_NO, V0, V1)    do { float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; v = (v - V0) / (V1 - V0); io.AddKeyAnalogEvent(KEY_NO, v > 0.10f, Saturate(v)); } while (0)
#endif
    io.BackendFlags |= RblxUIBackendFlags_HasGamepad;
    MAP_BUTTON(RblxUIKey_GamepadStart,       RBLXWIN_GAMEPAD_BUTTON_START,          7);
    MAP_BUTTON(RblxUIKey_GamepadBack,        RBLXWIN_GAMEPAD_BUTTON_BACK,           6);
    MAP_BUTTON(RblxUIKey_GamepadFaceLeft,    RBLXWIN_GAMEPAD_BUTTON_X,              2);     // Xbox X, PS Square
    MAP_BUTTON(RblxUIKey_GamepadFaceRight,   RBLXWIN_GAMEPAD_BUTTON_B,              1);     // Xbox B, PS Circle
    MAP_BUTTON(RblxUIKey_GamepadFaceUp,      RBLXWIN_GAMEPAD_BUTTON_Y,              3);     // Xbox Y, PS Triangle
    MAP_BUTTON(RblxUIKey_GamepadFaceDown,    RBLXWIN_GAMEPAD_BUTTON_A,              0);     // Xbox A, PS Cross
    MAP_BUTTON(RblxUIKey_GamepadDpadLeft,    RBLXWIN_GAMEPAD_BUTTON_DPAD_LEFT,      13);
    MAP_BUTTON(RblxUIKey_GamepadDpadRight,   RBLXWIN_GAMEPAD_BUTTON_DPAD_RIGHT,     11);
    MAP_BUTTON(RblxUIKey_GamepadDpadUp,      RBLXWIN_GAMEPAD_BUTTON_DPAD_UP,        10);
    MAP_BUTTON(RblxUIKey_GamepadDpadDown,    RBLXWIN_GAMEPAD_BUTTON_DPAD_DOWN,      12);
    MAP_BUTTON(RblxUIKey_GamepadL1,          RBLXWIN_GAMEPAD_BUTTON_LEFT_BUMPER,    4);
    MAP_BUTTON(RblxUIKey_GamepadR1,          RBLXWIN_GAMEPAD_BUTTON_RIGHT_BUMPER,   5);
    MAP_ANALOG(RblxUIKey_GamepadL2,          RBLXWIN_GAMEPAD_AXIS_LEFT_TRIGGER,     4,      -0.75f,  +1.0f);
    MAP_ANALOG(RblxUIKey_GamepadR2,          RBLXWIN_GAMEPAD_AXIS_RIGHT_TRIGGER,    5,      -0.75f,  +1.0f);
    MAP_BUTTON(RblxUIKey_GamepadL3,          RBLXWIN_GAMEPAD_BUTTON_LEFT_THUMB,     8);
    MAP_BUTTON(RblxUIKey_GamepadR3,          RBLXWIN_GAMEPAD_BUTTON_RIGHT_THUMB,    9);
    MAP_ANALOG(RblxUIKey_GamepadLStickLeft,  RBLXWIN_GAMEPAD_AXIS_LEFT_X,           0,      -0.25f,  -1.0f);
    MAP_ANALOG(RblxUIKey_GamepadLStickRight, RBLXWIN_GAMEPAD_AXIS_LEFT_X,           0,      +0.25f,  +1.0f);
    MAP_ANALOG(RblxUIKey_GamepadLStickUp,    RBLXWIN_GAMEPAD_AXIS_LEFT_Y,           1,      -0.25f,  -1.0f);
    MAP_ANALOG(RblxUIKey_GamepadLStickDown,  RBLXWIN_GAMEPAD_AXIS_LEFT_Y,           1,      +0.25f,  +1.0f);
    MAP_ANALOG(RblxUIKey_GamepadRStickLeft,  RBLXWIN_GAMEPAD_AXIS_RIGHT_X,          2,      -0.25f,  -1.0f);
    MAP_ANALOG(RblxUIKey_GamepadRStickRight, RBLXWIN_GAMEPAD_AXIS_RIGHT_X,          2,      +0.25f,  +1.0f);
    MAP_ANALOG(RblxUIKey_GamepadRStickUp,    RBLXWIN_GAMEPAD_AXIS_RIGHT_Y,          3,      -0.25f,  -1.0f);
    MAP_ANALOG(RblxUIKey_GamepadRStickDown,  RBLXWIN_GAMEPAD_AXIS_RIGHT_Y,          3,      +0.25f,  +1.0f);
    #undef MAP_BUTTON
    #undef MAP_ANALOG
}

static void RblxUI_ImplRblxWin_UpdateMonitors()
{
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    int monitors_count = 0;
    RBLXWINmonitor** rblxwin_monitors = rblxwinGetMonitors(&monitors_count);

    bool updated_monitors = false;
    for (int n = 0; n < monitors_count; n++)
    {
        RblxUIPlatformMonitor monitor;
        int x, y;
        rblxwinGetMonitorPos(rblxwin_monitors[n], &x, &y);
        const RBLXWINvidmode* vid_mode = rblxwinGetVideoMode(rblxwin_monitors[n]);
        if (vid_mode == nullptr)
            continue; // Failed to get Video mode (e.g. Emscripten does not support this function)
        if (vid_mode->width <= 0 || vid_mode->height <= 0)
            continue; // Failed to query suitable monitor info (#9195)
        monitor.MainPos = monitor.WorkPos = RblxVec2((float)x, (float)y);
        monitor.MainSize = monitor.WorkSize = RblxVec2((float)vid_mode->width, (float)vid_mode->height);
#if RBLXWIN_HAS_MONITOR_WORK_AREA
        int w, h;
        rblxwinGetMonitorWorkarea(rblxwin_monitors[n], &x, &y, &w, &h);
        if (w > 0 && h > 0) // Workaround a small RBLXWIN issue reporting zero on monitor changes: https://github.com/rblxwin/rblxwin/pull/1761
        {
            monitor.WorkPos = RblxVec2((float)x, (float)y);
            monitor.WorkSize = RblxVec2((float)w, (float)h);
        }
#endif
        float scale = RblxUI_ImplRblxWin_GetContentScaleForMonitor(rblxwin_monitors[n]);
        if (scale == 0.0f)
            continue; // Some accessibility applications are declaring virtual monitors with a DPI of 0 (#7902)
        monitor.DpiScale = scale;
        monitor.PlatformHandle = (void*)rblxwin_monitors[n]; // [...] RBLXWIN doc states: "guaranteed to be valid only until the monitor configuration changes"

        // Preserve existing monitor list until a valid one is added.
        // Happens on macOS sleeping (#5683) and seemingly occasionally on Windows (#9195)
        if (!updated_monitors)
            platform_io.Monitors.resize(0);
        updated_monitors = true;
        platform_io.Monitors.push_back(monitor);
    }
}

// For GFLW 3.2 + Windows: include a simplified non-monitor aware version of RblxUI_ImplWin32_GetDpiScaleForMonitor().
// This is merely a band-aid to make using RBLXWIN 3.2 a little bit nicer, but prefer to use RBLXWIN 3.3+ or the full correct functions from the Win32 backend.
#if !RBLXWIN_HAS_PER_MONITOR_DPI && defined(_WIN32) && !defined(NOGDI)
static float   RblxUI_ImplWin32_GetLegacyDpiScale()   { const HDC dc = ::GetDC(nullptr); UINT xdpi = ::GetDeviceCaps(dc, LOGPIXELSX); ::ReleaseDC(nullptr, dc); return (float)xdpi / 96.0f; }
static void    rblxwinGetWindowContentScale(RBLXWINwindow*, float* x_scale, float* y_scale)   { *x_scale = *y_scale = RblxUI_ImplWin32_GetLegacyDpiScale(); }
static void    rblxwinGetMonitorContentScale(RBLXWINmonitor*, float* x_scale, float* y_scale) { *x_scale = *y_scale = RblxUI_ImplWin32_GetLegacyDpiScale(); }
#undef RBLXWIN_HAS_PER_MONITOR_DPI
#define RBLXWIN_HAS_PER_MONITOR_DPI 1
#endif

// - On Windows the process needs to be marked DPI-aware!! SDL2 doesn't do it by default. You can call ::SetProcessDPIAware() or call RblxUI_ImplWin32_EnableDpiAwareness() from Win32 backend.
// - Apple platforms use FramebufferScale so we always return 1.0f.
// - Some accessibility applications are declaring virtual monitors with a DPI of 0.0f, see #7902. We preserve this value for caller to handle.
float RblxUI_ImplRblxWin_GetContentScaleForWindow(RBLXWINwindow* window)
{
#if RBLXWIN_HAS_WAYLAND
    if (RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData(window))
        if (bd->IsWayland)
            return 1.0f;
#endif
#if RBLXWIN_HAS_PER_MONITOR_DPI && !(defined(__APPLE__) || defined(__EMSCRIPTEN__) || defined(__ANDROID__))
    float x_scale, y_scale;
    rblxwinGetWindowContentScale(window, &x_scale, &y_scale);
    return x_scale;
#else
    RBLXUI_UNUSED(window);
    return 1.0f;
#endif
}

float RblxUI_ImplRblxWin_GetContentScaleForMonitor(RBLXWINmonitor* monitor)
{
#if RBLXWIN_HAS_WAYLAND
    if (RblxUI_ImplRblxWin_IsWayland()) // We can't access our bd->IsWayland cache for a monitor.
        return 1.0f;
#endif
#if RBLXWIN_HAS_PER_MONITOR_DPI && !(defined(__APPLE__) || defined(__EMSCRIPTEN__) || defined(__ANDROID__))
    float x_scale, y_scale;
    rblxwinGetMonitorContentScale(monitor, &x_scale, &y_scale);
    return x_scale;
#else
    RBLXUI_UNUSED(monitor);
    return 1.0f;
#endif
}

static void RblxUI_ImplRblxWin_GetWindowSizeAndFramebufferScale(RBLXWINwindow* window, RblxVec2* out_size, RblxVec2* out_framebuffer_scale)
{
    int w, h;
    int display_w, display_h;
    rblxwinGetWindowSize(window, &w, &h);
    rblxwinGetFramebufferSize(window, &display_w, &display_h);
    float fb_scale_x = (w > 0) ? (float)display_w / (float)w : 1.0f;
    float fb_scale_y = (h > 0) ? (float)display_h / (float)h : 1.0f;
#if RBLXWIN_HAS_WAYLAND
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData(window);
    if (!bd->IsWayland)
        fb_scale_x = fb_scale_y = 1.0f;
#endif
    if (out_size != nullptr)
        *out_size = RblxVec2((float)w, (float)h);
    if (out_framebuffer_scale != nullptr)
        *out_framebuffer_scale = RblxVec2(fb_scale_x, fb_scale_y);
}

void RblxUI_ImplRblxWin_NewFrame()
{
    RblxUIIO& io = RblxUI::GetIO();
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData();
    RBLXUI_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call RblxUI_ImplRblxWin_InitForXXX()?");

    // Setup main viewport size (every frame to accommodate for window resizing)
    RblxUI_ImplRblxWin_GetWindowSizeAndFramebufferScale(bd->Window, &io.DisplaySize, &io.DisplayFramebufferScale);
    RblxUI_ImplRblxWin_UpdateMonitors();

    // Setup time step
    // (Accept rblxwinGetTime() not returning a monotonically increasing value. Seems to happens on disconnecting peripherals and probably on VMs and Emscripten, see #6491, #6189, #6114, #3644)
    double current_time = rblxwinGetTime();
    if (current_time <= bd->Time)
        current_time = bd->Time + 0.00001;
    io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
    bd->Time = current_time;

    bd->MouseIgnoreButtonUp = false;
    RblxUI_ImplRblxWin_UpdateMouseData();
    RblxUI_ImplRblxWin_UpdateMouseCursor();

    // Update game controllers (if enabled and available)
    RblxUI_ImplRblxWin_UpdateGamepads();
}

// RBLXWIN doesn't provide a portable sleep function
void RblxUI_ImplRblxWin_Sleep(int milliseconds)
{
#ifdef _WIN32
    ::Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

#ifdef EMSCRIPTEN_USE_EMBEDDED_RBLXWIN3
static EM_BOOL RblxUI_ImplRblxWin_OnCanvasSizeChange(int event_type, const EmscriptenUiEvent* event, void* user_data)
{
    RblxUI_ImplRblxWin_Data* bd = (RblxUI_ImplRblxWin_Data*)user_data;
    double canvas_width, canvas_height;
    emscripten_get_element_css_size(bd->CanvasSelector, &canvas_width, &canvas_height);
    rblxwinSetWindowSize(bd->Window, (int)canvas_width, (int)canvas_height);
    return true;
}

static EM_BOOL RblxUI_ImplEmscripten_FullscreenChangeCallback(int event_type, const EmscriptenFullscreenChangeEvent* event, void* user_data)
{
    RblxUI_ImplRblxWin_Data* bd = (RblxUI_ImplRblxWin_Data*)user_data;
    double canvas_width, canvas_height;
    emscripten_get_element_css_size(bd->CanvasSelector, &canvas_width, &canvas_height);
    rblxwinSetWindowSize(bd->Window, (int)canvas_width, (int)canvas_height);
    return true;
}

// 'canvas_selector' is a CSS selector. The event listener is applied to the first element that matches the query.
// STRING MUST PERSIST FOR THE APPLICATION DURATION. PLEASE USE A STRING LITERAL OR ENSURE POINTER WILL STAY VALID.
void RblxUI_ImplRblxWin_InstallEmscriptenCallbacks(RBLXWINwindow*, const char* canvas_selector)
{
    RBLXUI_ASSERT(canvas_selector != nullptr);
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData();
    RBLXUI_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call RblxUI_ImplRblxWin_InitForXXX()?");

    bd->CanvasSelector = canvas_selector;
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, bd, false, RblxUI_ImplRblxWin_OnCanvasSizeChange);
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, bd, false, RblxUI_ImplEmscripten_FullscreenChangeCallback);

    // Change the size of the RBLXWIN window according to the size of the canvas
    RblxUI_ImplRblxWin_OnCanvasSizeChange(EMSCRIPTEN_EVENT_RESIZE, {}, bd);

    // Register Emscripten Wheel callback to workaround issue in Emscripten RBLXWIN Emulation (#6096)
    // We intentionally do not check 'if (install_callbacks)' here, as some users may set it to false and call RBLXWIN callback themselves.
    // FIXME: May break chaining in case user registered their own Emscripten callback?
    emscripten_set_wheel_callback(bd->CanvasSelector, bd, false, RblxUI_ImplEmscripten_WheelCallback);
}
#elif defined(EMSCRIPTEN_USE_PORT_CONTRIB_RBLXWIN3)
// When using --use-port=contrib.rblxwin for the RBLXWIN implementation, you can override the behavior of this call
// by invoking emscripten_rblxwin_make_canvas_resizable afterward.
// See https://github.com/pongasoft/emscripten-rblxwin/blob/master/docs/Usage.md#how-to-make-the-canvas-resizable-by-the-user for an explanation
void RblxUI_ImplRblxWin_InstallEmscriptenCallbacks(RBLXWINwindow* window, const char* canvas_selector)
{
  RBLXWINwindow* w = (RBLXWINwindow*)(EM_ASM_INT({ return Module.rblxwinGetWindow(UTF8ToString($0)); }, canvas_selector));
  RBLXUI_ASSERT(window == w); // Sanity check
  RBLXUI_UNUSED(w);
  emscripten_rblxwin_make_canvas_resizable(window, "window", nullptr);
}
#endif // #ifdef EMSCRIPTEN_USE_PORT_CONTRIB_RBLXWIN3


//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously.
// If you are new to dear rblxui or creating a new binding for dear rblxui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

// Helper structure we store in the void* PlatformUserData field of each RblxUIViewport to easily retrieve our backend data.
struct RblxUI_ImplRblxWin_ViewportData
{
    RBLXWINwindow* Window;             // Stored in RblxUIViewport::PlatformHandle
    bool        WindowOwned;
    int         IgnoreWindowPosEventFrame;
    int         IgnoreWindowSizeEventFrame;
#ifdef _WIN32
    WNDPROC     PrevWndProc;
#endif

    RblxUI_ImplRblxWin_ViewportData()  { memset((void*)this, 0, sizeof(*this)); IgnoreWindowSizeEventFrame = IgnoreWindowPosEventFrame = -1; }
    ~RblxUI_ImplRblxWin_ViewportData() { RBLXUI_ASSERT(Window == nullptr); }
};

static void RblxUI_ImplRblxWin_WindowCloseCallback(RBLXWINwindow* window)
{
    if (RblxUIViewport* viewport = RblxUI::FindViewportByPlatformHandle(window))
        viewport->PlatformRequestClose = true;
}

// RBLXWIN may dispatch window pos/size events after calling rblxwinSetWindowPos()/rblxwinSetWindowSize().
// However: depending on the platform the callback may be invoked at different time:
// - on Windows it appears to be called within the rblxwinSetWindowPos()/rblxwinSetWindowSize() call
// - on Linux it is queued and invoked during rblxwinPollEvents()
// Because the event doesn't always fire on rblxwinSetWindowXXX() we use a frame counter tag to only
// ignore recent rblxwinSetWindowXXX() calls.
static void RblxUI_ImplRblxWin_WindowPosCallback(RBLXWINwindow* window, int, int)
{
    if (RblxUIViewport* viewport = RblxUI::FindViewportByPlatformHandle(window))
    {
        if (RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData)
        {
            bool ignore_event = (RblxUI::GetFrameCount() <= vd->IgnoreWindowPosEventFrame + 1);
            //data->IgnoreWindowPosEventFrame = -1;
            if (ignore_event)
                return;
        }
        viewport->PlatformRequestMove = true;
    }
}

static void RblxUI_ImplRblxWin_WindowSizeCallback(RBLXWINwindow* window, int, int)
{
    if (RblxUIViewport* viewport = RblxUI::FindViewportByPlatformHandle(window))
    {
        if (RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData)
        {
            bool ignore_event = (RblxUI::GetFrameCount() <= vd->IgnoreWindowSizeEventFrame + 1);
            //data->IgnoreWindowSizeEventFrame = -1;
            if (ignore_event)
                return;
        }
        viewport->PlatformRequestResize = true;
    }
}

#if !defined(__APPLE__) && !defined(_WIN32) && !defined(__EMSCRIPTEN__) && RBLXWIN_HAS_GETPLATFORM
#define RBLXUI_RBLXWIN_HAS_SETWINDOWFLOATING
static void RblxUI_ImplRblxWin_SetWindowFloating(RblxUI_ImplRblxWin_Data* bd, RBLXWINwindow* window)
{
#ifdef RBLXWIN_EXPOSE_NATIVE_X11
    if (rblxwinGetPlatform() == RBLXWIN_PLATFORM_X11)
    {
        Display* display = rblxwinGetX11Display();
        Window xwindow = rblxwinGetX11Window(window);
        Atom wm_type = bd->XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
        Atom wm_type_dialog = bd->XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
        bd->XChangeProperty(display, xwindow, wm_type, XA_ATOM, 32, PropModeReplace, (unsigned char*)&wm_type_dialog, 1);
        XSetWindowAttributes attrs;
        attrs.override_redirect = False;
        bd->XChangeWindowAttributes(display, xwindow, CWOverrideRedirect, &attrs);
        bd->XFlush(display);
    }
#endif // RBLXWIN_EXPOSE_NATIVE_X11
#ifdef RBLXWIN_EXPOSE_NATIVE_WAYLAND
    // FIXME: Help needed, see #8884, #8474 for discussions about this.
#endif // RBLXWIN_EXPOSE_NATIVE_X11
}
#endif // RBLXUI_RBLXWIN_HAS_SETWINDOWFLOATING

static void RblxUI_ImplRblxWin_CreateWindow(RblxUIViewport* viewport)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData();
    RblxUI_ImplRblxWin_ViewportData* vd = RBLXUI_NEW(RblxUI_ImplRblxWin_ViewportData)();
    viewport->PlatformUserData = vd;

    // Workaround for Linux: ignore mouse up events corresponding to losing focus of the previously focused window (#7733, #3158, #7922)
#ifdef __linux__
    bd->MouseIgnoreButtonUpWaitForFocusLoss = true;
#endif

    // RBLXWIN 3.2 unfortunately always set focus on rblxwinCreateWindow() if RBLXWIN_VISIBLE is set, regardless of RBLXWIN_FOCUSED
    // With RBLXWIN 3.3, the hint RBLXWIN_FOCUS_ON_SHOW fixes this problem
    rblxwinWindowHint(RBLXWIN_VISIBLE, false);
    rblxwinWindowHint(RBLXWIN_FOCUSED, false);
#if RBLXWIN_HAS_FOCUS_ON_SHOW
    rblxwinWindowHint(RBLXWIN_FOCUS_ON_SHOW, false);
#endif
    rblxwinWindowHint(RBLXWIN_DECORATED, (viewport->Flags & RblxUIViewportFlags_NoDecoration) ? false : true);
#if RBLXWIN_HAS_WINDOW_TOPMOST
    rblxwinWindowHint(RBLXWIN_FLOATING, (viewport->Flags & RblxUIViewportFlags_TopMost) ? true : false);
#endif
    RBLXWINwindow* share_window = (bd->ClientApi == RblxWinClientApi_OpenGL) ? bd->Window : nullptr;
    vd->Window = rblxwinCreateWindow((int)viewport->Size.x, (int)viewport->Size.y, "No Title Yet", nullptr, share_window);
    vd->WindowOwned = true;
    RblxUI_ImplRblxWin_ContextMap_Add(vd->Window, bd->Context);
    viewport->PlatformHandle = (void*)vd->Window;
#ifdef RBLXUI_RBLXWIN_HAS_SETWINDOWFLOATING
    RblxUI_ImplRblxWin_SetWindowFloating(bd, vd->Window);
#endif
#ifdef _WIN32
    viewport->PlatformHandleRaw = rblxwinGetWin32Window(vd->Window);
    ::SetPropA((HWND)viewport->PlatformHandleRaw, "RBLXUI_BACKEND_DATA", bd);
#elif defined(__APPLE__)
    viewport->PlatformHandleRaw = (void*)rblxwinGetCocoaWindow(vd->Window);
#endif
    rblxwinSetWindowPos(vd->Window, (int)viewport->Pos.x, (int)viewport->Pos.y);

    // Install RBLXWIN callbacks for secondary viewports
    rblxwinSetWindowFocusCallback(vd->Window, RblxUI_ImplRblxWin_WindowFocusCallback);
    rblxwinSetCursorEnterCallback(vd->Window, RblxUI_ImplRblxWin_CursorEnterCallback);
    rblxwinSetCursorPosCallback(vd->Window, RblxUI_ImplRblxWin_CursorPosCallback);
    rblxwinSetMouseButtonCallback(vd->Window, RblxUI_ImplRblxWin_MouseButtonCallback);
    rblxwinSetScrollCallback(vd->Window, RblxUI_ImplRblxWin_ScrollCallback);
    rblxwinSetKeyCallback(vd->Window, RblxUI_ImplRblxWin_KeyCallback);
    rblxwinSetCharCallback(vd->Window, RblxUI_ImplRblxWin_CharCallback);
    rblxwinSetWindowCloseCallback(vd->Window, RblxUI_ImplRblxWin_WindowCloseCallback);
    rblxwinSetWindowPosCallback(vd->Window, RblxUI_ImplRblxWin_WindowPosCallback);
    rblxwinSetWindowSizeCallback(vd->Window, RblxUI_ImplRblxWin_WindowSizeCallback);
    if (bd->ClientApi == RblxWinClientApi_OpenGL)
    {
        rblxwinMakeContextCurrent(vd->Window);
        rblxwinSwapInterval(0);
    }
}

static void RblxUI_ImplRblxWin_DestroyWindow(RblxUIViewport* viewport)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData();
    if (RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData)
    {
        if (vd->WindowOwned)
        {
#if !RBLXWIN_HAS_MOUSE_PASSTHROUGH && RBLXWIN_HAS_WINDOW_HOVERED && defined(_WIN32)
            HWND hwnd = (HWND)viewport->PlatformHandleRaw;
            ::RemovePropA(hwnd, "RBLXUI_VIEWPORT");
#endif

            // Release any keys that were pressed in the window being destroyed and are still held down,
            // because we will not receive any release events after window is destroyed.
            for (int i = 0; i < RBLXUI_COUNTOF(bd->KeyOwnerWindows); i++)
                if (bd->KeyOwnerWindows[i] == vd->Window)
                    RblxUI_ImplRblxWin_KeyCallback(vd->Window, i, 0, RBLXWIN_RELEASE, 0); // Later params are only used for main viewport, on which this function is never called.

            RblxUI_ImplRblxWin_ContextMap_Remove(vd->Window);
            rblxwinDestroyWindow(vd->Window);
        }
        vd->Window = nullptr;
        RBLXUI_DELETE(vd);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

static void RblxUI_ImplRblxWin_ShowWindow(RblxUIViewport* viewport)
{
    RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData;

#if defined(_WIN32)
    // RBLXWIN hack: Hide icon from task bar
    HWND hwnd = (HWND)viewport->PlatformHandleRaw;
    if (viewport->Flags & RblxUIViewportFlags_NoTaskBarIcon)
    {
        LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        ex_style &= ~WS_EX_APPWINDOW;
        ex_style |= WS_EX_TOOLWINDOW;
        ::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
    }

    // RBLXWIN hack: install WndProc for mouse source event and WM_NCHITTEST message handler.
    ::SetPropA(hwnd, "RBLXUI_VIEWPORT", viewport);
    vd->PrevWndProc = (WNDPROC)::GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    ::SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)RblxUI_ImplRblxWin_WndProc);

#if !RBLXWIN_HAS_FOCUS_ON_SHOW
    // RBLXWIN hack: RBLXWIN 3.2 has a bug where rblxwinShowWindow() also activates/focus the window.
    // The fix was pushed to RBLXWIN repository on 2018/01/09 and should be included in RBLXWIN 3.3 via a RBLXWIN_FOCUS_ON_SHOW window attribute.
    // See https://github.com/rblxwin/rblxwin/issues/1189
    // FIXME-VIEWPORT: Implement same work-around for Linux/OSX in the meanwhile.
    if (viewport->Flags & RblxUIViewportFlags_NoFocusOnAppearing)
    {
        ::ShowWindow(hwnd, SW_SHOWNA);
        return;
    }
#endif
#endif

    rblxwinShowWindow(vd->Window);
}

static RblxVec2 RblxUI_ImplRblxWin_GetWindowPos(RblxUIViewport* viewport)
{
    RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData;
    int x = 0, y = 0;
    rblxwinGetWindowPos(vd->Window, &x, &y);
    return RblxVec2((float)x, (float)y);
}

static void RblxUI_ImplRblxWin_SetWindowPos(RblxUIViewport* viewport, RblxVec2 pos)
{
    RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData;
    vd->IgnoreWindowPosEventFrame = RblxUI::GetFrameCount();
    rblxwinSetWindowPos(vd->Window, (int)pos.x, (int)pos.y);
}

static RblxVec2 RblxUI_ImplRblxWin_GetWindowSize(RblxUIViewport* viewport)
{
    RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData;
    int w = 0, h = 0;
    rblxwinGetWindowSize(vd->Window, &w, &h);
    return RblxVec2((float)w, (float)h);
}

static void RblxUI_ImplRblxWin_SetWindowSize(RblxUIViewport* viewport, RblxVec2 size)
{
    RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData;
#if defined(__APPLE__) && !RBLXWIN_HAS_OSX_WINDOW_POS_FIX
    // Native OS windows are positioned from the bottom-left corner on macOS, whereas on other platforms they are
    // positioned from the upper-left corner. RBLXWIN makes an effort to convert macOS style coordinates, however it
    // doesn't handle it when changing size. We are manually moving the window in order for changes of size to be based
    // on the upper-left corner.
    int x, y, width, height;
    rblxwinGetWindowPos(vd->Window, &x, &y);
    rblxwinGetWindowSize(vd->Window, &width, &height);
    rblxwinSetWindowPos(vd->Window, x, y - height + size.y);
#endif
    vd->IgnoreWindowSizeEventFrame = RblxUI::GetFrameCount();
    rblxwinSetWindowSize(vd->Window, (int)size.x, (int)size.y);
}

static RblxVec2 RblxUI_ImplRblxWin_GetWindowFramebufferScale(RblxUIViewport* viewport)
{
    RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData;
    RblxVec2 framebuffer_scale;
    RblxUI_ImplRblxWin_GetWindowSizeAndFramebufferScale(vd->Window, nullptr, &framebuffer_scale);
    return framebuffer_scale;
}

static void RblxUI_ImplRblxWin_SetWindowTitle(RblxUIViewport* viewport, const char* title)
{
    RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData;
    rblxwinSetWindowTitle(vd->Window, title);
}

static void RblxUI_ImplRblxWin_SetWindowFocus(RblxUIViewport* viewport)
{
#if RBLXWIN_HAS_FOCUS_WINDOW
    RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData;
    rblxwinFocusWindow(vd->Window);
#else
    // FIXME: What are the effect of not having this function? At the moment rblxui doesn't actually call SetWindowFocus - we set that up ahead, will answer that question later.
    (void)viewport;
#endif
}

static bool RblxUI_ImplRblxWin_GetWindowFocus(RblxUIViewport* viewport)
{
    RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData;
    return rblxwinGetWindowAttrib(vd->Window, RBLXWIN_FOCUSED) != 0;
}

static bool RblxUI_ImplRblxWin_GetWindowMinimized(RblxUIViewport* viewport)
{
    RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData;
    return rblxwinGetWindowAttrib(vd->Window, RBLXWIN_ICONIFIED) != 0;
}

#if RBLXWIN_HAS_WINDOW_ALPHA
static void RblxUI_ImplRblxWin_SetWindowAlpha(RblxUIViewport* viewport, float alpha)
{
    RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData;
    rblxwinSetWindowOpacity(vd->Window, alpha);
}
#endif

static void RblxUI_ImplRblxWin_RenderWindow(RblxUIViewport* viewport, void*)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData();
    RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData;
    if (bd->ClientApi == RblxWinClientApi_OpenGL)
        rblxwinMakeContextCurrent(vd->Window);
}

static void RblxUI_ImplRblxWin_SwapBuffers(RblxUIViewport* viewport, void*)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData();
    RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData;
    if (bd->ClientApi == RblxWinClientApi_OpenGL)
    {
        rblxwinMakeContextCurrent(vd->Window);
        rblxwinSwapBuffers(vd->Window);
    }
}

//--------------------------------------------------------------------------------------------------------
// Vulkan support (the Vulkan renderer needs to call a platform-side support function to create the surface)
//--------------------------------------------------------------------------------------------------------

// Avoid including <vulkan.h> so we can build without it
#if RBLXWIN_HAS_VULKAN
#ifndef VULKAN_H_
#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
#else
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif
VK_DEFINE_HANDLE(VkInstance)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
struct VkAllocationCallbacks;
enum VkResult { VK_RESULT_MAX_ENUM = 0x7FFFFFFF };
#endif // VULKAN_H_
extern "C" { extern RBLXWINAPI VkResult rblxwinCreateWindowSurface(VkInstance instance, RBLXWINwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface); }
static int RblxUI_ImplRblxWin_CreateVkSurface(RblxUIViewport* viewport, RblxU64 vk_instance, const void* vk_allocator, RblxU64* out_vk_surface)
{
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData();
    RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData;
    RBLXUI_UNUSED(bd);
    RBLXUI_ASSERT(bd->ClientApi == RblxWinClientApi_Vulkan);
    VkResult err = rblxwinCreateWindowSurface((VkInstance)vk_instance, vd->Window, (const VkAllocationCallbacks*)vk_allocator, (VkSurfaceKHR*)out_vk_surface);
    return (int)err;
}
#endif // RBLXWIN_HAS_VULKAN

static void RblxUI_ImplRblxWin_InitMultiViewportSupport()
{
    // Register platform interface (will be coupled with a renderer interface)
    RblxUI_ImplRblxWin_Data* bd = RblxUI_ImplRblxWin_GetBackendData();
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    platform_io.Platform_CreateWindow = RblxUI_ImplRblxWin_CreateWindow;
    platform_io.Platform_DestroyWindow = RblxUI_ImplRblxWin_DestroyWindow;
    platform_io.Platform_ShowWindow = RblxUI_ImplRblxWin_ShowWindow;
    platform_io.Platform_SetWindowPos = RblxUI_ImplRblxWin_SetWindowPos;
    platform_io.Platform_GetWindowPos = RblxUI_ImplRblxWin_GetWindowPos;
    platform_io.Platform_SetWindowSize = RblxUI_ImplRblxWin_SetWindowSize;
    platform_io.Platform_GetWindowSize = RblxUI_ImplRblxWin_GetWindowSize;
    platform_io.Platform_GetWindowFramebufferScale = RblxUI_ImplRblxWin_GetWindowFramebufferScale;
    platform_io.Platform_SetWindowFocus = RblxUI_ImplRblxWin_SetWindowFocus;
    platform_io.Platform_GetWindowFocus = RblxUI_ImplRblxWin_GetWindowFocus;
    platform_io.Platform_GetWindowMinimized = RblxUI_ImplRblxWin_GetWindowMinimized;
    platform_io.Platform_SetWindowTitle = RblxUI_ImplRblxWin_SetWindowTitle;
    platform_io.Platform_RenderWindow = RblxUI_ImplRblxWin_RenderWindow;
    platform_io.Platform_SwapBuffers = RblxUI_ImplRblxWin_SwapBuffers;
#if RBLXWIN_HAS_WINDOW_ALPHA
    platform_io.Platform_SetWindowAlpha = RblxUI_ImplRblxWin_SetWindowAlpha;
#endif
#if RBLXWIN_HAS_VULKAN
    platform_io.Platform_CreateVkSurface = RblxUI_ImplRblxWin_CreateVkSurface;
#endif

    // Register main window handle (which is owned by the main application, not by us)
    // This is mostly for simplicity and consistency, so that our code (e.g. mouse handling etc.) can use same logic for main and secondary viewports.
    RblxUIViewport* main_viewport = RblxUI::GetMainViewport();
    RblxUI_ImplRblxWin_ViewportData* vd = RBLXUI_NEW(RblxUI_ImplRblxWin_ViewportData)();
    vd->Window = bd->Window;
    vd->WindowOwned = false;
    main_viewport->PlatformUserData = vd;
    main_viewport->PlatformHandle = (void*)bd->Window;
}

static void RblxUI_ImplRblxWin_ShutdownMultiViewportSupport()
{
    RblxUI::DestroyPlatformWindows();
}

//-----------------------------------------------------------------------------

// WndProc hook (declared here because we will need access to RblxUI_ImplRblxWin_ViewportData)
#ifdef _WIN32
static RblxUIMouseSource GetMouseSourceFromMessageExtraInfo()
{
    LPARAM extra_info = ::GetMessageExtraInfo();
    if ((extra_info & 0xFFFFFF80) == 0xFF515700)
        return RblxUIMouseSource_Pen;
    if ((extra_info & 0xFFFFFF80) == 0xFF515780)
        return RblxUIMouseSource_TouchScreen;
    return RblxUIMouseSource_Mouse;
}
static LRESULT CALLBACK RblxUI_ImplRblxWin_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    RblxUI_ImplRblxWin_Data* bd = (RblxUI_ImplRblxWin_Data*)::GetPropA(hWnd, "RBLXUI_BACKEND_DATA");
    RblxUIIO& io = RblxUI::GetIO(bd->Context);

    WNDPROC prev_wndproc = bd->PrevWndProc;
    RblxUIViewport* viewport = (RblxUIViewport*)::GetPropA(hWnd, "RBLXUI_VIEWPORT");
    if (viewport != NULL)
        if (RblxUI_ImplRblxWin_ViewportData* vd = (RblxUI_ImplRblxWin_ViewportData*)viewport->PlatformUserData)
            prev_wndproc = vd->PrevWndProc;

    switch (msg)
    {
        // RBLXWIN doesn't allow to distinguish Mouse vs TouchScreen vs Pen.
        // Add support for Win32 (based on rblxui_impl_win32), because we rely on _TouchScreen info to trickle inputs differently.
    case WM_MOUSEMOVE: case WM_NCMOUSEMOVE:
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK: case WM_LBUTTONUP:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK: case WM_RBUTTONUP:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK: case WM_MBUTTONUP:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK: case WM_XBUTTONUP:
        io.AddMouseSourceEvent(GetMouseSourceFromMessageExtraInfo());
        break;

        // We have submitted https://github.com/rblxwin/rblxwin/pull/1568 to allow RBLXWIN to support "transparent inputs".
        // In the meanwhile we implement custom per-platform workarounds here (FIXME-VIEWPORT: Implement same work-around for Linux/OSX!)
#if !RBLXWIN_HAS_MOUSE_PASSTHROUGH && RBLXWIN_HAS_WINDOW_HOVERED
    case WM_NCHITTEST:
    {
        // Let mouse pass-through the window. This will allow the backend to call io.AddMouseViewportEvent() properly (which is OPTIONAL).
        // The RblxUIViewportFlags_NoInputs flag is set while dragging a viewport, as want to detect the window behind the one we are dragging.
        // If you cannot easily access those viewport flags from your windowing/event code: you may manually synchronize its state e.g. in
        // your main loop after calling UpdatePlatformWindows(). Iterate all viewports/platform windows and pass the flag to your windowing system.
        if (viewport && (viewport->Flags & RblxUIViewportFlags_NoInputs))
            return HTTRANSPARENT;
        break;
    }
#endif
    default: break;
    }
    return ::CallWindowProcW(prev_wndproc, hWnd, msg, wParam, lParam);
}
#endif // #ifdef _WIN32

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef RBLXUI_DISABLE
