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

// About Emscripten support:
// - Emscripten provides its own LINEARWIN (3.2.1) implementation (syntax: "-sUSE_LINEARWIN=3"), but Joystick is broken and several features are not supported (multiple windows, clipboard, timer, etc.)
// - A third-party Emscripten LINEARWIN (3.4.0) implementation (syntax: "--use-port=contrib.linearwin") fixes the Joystick issue and implements all relevant features for the browser.
// See https://github.com/pongasoft/emscripten-linearwin/blob/master/docs/Comparison.md for details.

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2026-XX-XX: Platform: Added support for multiple windows via the LinearUIPlatformIO interface.
//  2026-04-21: Added a Win32-specific implementation of LinearUI_ImplLinearWin_GetContentScaleXXXX functions for legacy LINEARWIN 3.2.
//  2026-03-25: Mouse cursor is properly restored if changed by user app/code while using linearwinSetInputMode(..., LINEARWIN_CURSOR_DISABLED) or LinearUIConfigFlags_NoMouseCursorChange. Amend change from 2025-12-10.
//  2026-02-10: Try to set LINEARUI_IMPL_LINEARWIN_DISABLE_X11 / LINEARUI_IMPL_LINEARWIN_DISABLE_WAYLAND automatically if corresponding headers are not accessible. (#9225)
//  2026-01-25: [Docking] Improve workarounds for cases where LINEARWIN is unable to provide any reliable monitor info. Preserve existing monitor list when none of the new one is valid. (#9195, #7902, #5683)
//  2026-01-18: [Docking] Dynamically load X11 functions to avoid -lx11 linking requirement introduced on 2025-09-10.
//  2025-12-12: Added LINEARUI_IMPL_LINEARWIN_DISABLE_X11 / LINEARUI_IMPL_LINEARWIN_DISABLE_WAYLAND to forcefully disable either.
//  2025-12-10: Avoid repeated linearwinSetCursor()/linearwinSetInputMode() calls when unnecessary. Lowers overhead for very high framerates (e.g. 10k+ FPS).
//  2025-11-06: Lower minimum requirement to LINEARWIN 3.0. Though a recent version e.g LINEARWIN 3.4 is highly recommended.
//  2025-09-18: Call platform_io.ClearPlatformHandlers() on shutdown.
//  2025-09-15: Content Scales are always reported as 1.0 on Wayland. FramebufferScale are always reported as 1.0 on X11. (#8920, #8921)
//  2025-09-10: [Docking] Improve multi-viewport behavior in tiling WMs on X11 via the LinearUI_ImplLinearWin_SetWindowFloating() function. Note: using LINEARWIN backend on Linux/BSD etc. requires linking with -lX11. (#8884, #8474, #8289)
//  2025-07-08: Made LinearUI_ImplLinearWin_GetContentScaleForWindow(), LinearUI_ImplLinearWin_GetContentScaleForMonitor() helpers return 1.0f on Emscripten and Android platforms, matching macOS logic. (#8742, #8733)
//  2025-06-18: Added support for multiple Dear LinearUI contexts. (#8676, #8239, #8069)
//  2025-06-11: Added LinearUI_ImplLinearWin_GetContentScaleForWindow(LINEARWINwindow* window) and LinearUI_ImplLinearWin_GetContentScaleForMonitor(LINEARWINmonitor* monitor) helper to facilitate making DPI-aware apps.
//  2025-05-15: [Docking] Add Platform_GetWindowFramebufferScale() handler, to allow varying Retina display density on multiple monitors.
//  2025-04-26: [Docking] Disable multi-viewports under Wayland. (#8587)
//  2025-03-10: Map LINEARWIN_KEY_WORLD_1 and LINEARWIN_KEY_WORLD_2 into LinearUIKey_Oem102.
//  2025-03-03: Fixed clipboard handler assertion when using LINEARWIN <= 3.2.1 compiled with asserts enabled.
//  2025-02-21: [Docking] Update monitors and work areas information every frame, as the later may change regardless of monitor changes. (#8415)
//  2024-11-05: [Docking] Added Linux workaround for spurious mouse up events emitted while dragging and creating new viewport. (#3158, #7733, #7922)
//  2024-08-22: Moved some OS/backend related function pointers from LinearUIIO to LinearUIPlatformIO:
//               - io.GetClipboardTextFn    -> platform_io.Platform_GetClipboardTextFn
//               - io.SetClipboardTextFn    -> platform_io.Platform_SetClipboardTextFn
//               - io.PlatformOpenInShellFn -> platform_io.Platform_OpenInShellFn
//  2024-07-31: Added LinearUI_ImplLinearWin_Sleep() helper function for usage by our examples app, since LINEARWIN doesn't provide one.
//  2024-07-08: *BREAKING* Renamed LinearUI_ImplLinearWin_InstallEmscriptenCanvasResizeCallback to LinearUI_ImplLinearWin_InstallEmscriptenCallbacks(), added LINEARWINWindow* parameter.
//  2024-07-08: Emscripten: Added support for LINEARWIN3 contrib port (LINEARWIN 3.4.0 features + bug fixes): to enable, replace -sUSE_LINEARWIN=3 with --use-port=contrib.linearwin (requires emscripten 3.1.59+) (https://github.com/pongasoft/emscripten-linearwin)
//  2024-07-02: Emscripten: Added io.PlatformOpenInShellFn() handler for Emscripten versions.
//  2023-12-19: Emscripten: Added LinearUI_ImplLinearWin_InstallEmscriptenCanvasResizeCallback() to register canvas selector and auto-resize LINEARWIN window.
//  2023-10-05: Inputs: Added support for extra LinearUIKey values: F13 to F24 function keys.
//  2023-07-18: Inputs: Revert ignoring mouse data on LINEARWIN_CURSOR_DISABLED as it can be used differently. User may set LinearUIConfigFLags_NoMouse if desired. (#5625, #6609)
//  2023-06-12: Accept linearwinGetTime() not returning a monotonically increasing value. This seems to happens on some Windows setup when peripherals disconnect, and is likely to also happen on browser + Emscripten. (#6491)
//  2023-04-04: Inputs: Added support for io.AddMouseSourceEvent() to discriminate LinearUIMouseSource_Mouse/LinearUIMouseSource_TouchScreen/LinearUIMouseSource_Pen on Windows ONLY, using a custom WndProc hook. (#2702)
//  2023-03-16: Inputs: Fixed key modifiers handling on secondary viewports (docking branch). Broken on 2023/01/04. (#6248, #6034)
//  2023-03-14: Emscripten: Avoid using linearwinGetError() and linearwinGetGamepadState() which are not correctly implemented in Emscripten emulation. (#6240)
//  2023-02-03: Emscripten: Registering custom low-level mouse wheel handler to get more accurate scrolling impulses on Emscripten. (#4019, #6096)
//  2023-01-18: Handle unsupported linearwinGetVideoMode() call on e.g. Emscripten.
//  2023-01-04: Inputs: Fixed mods state on Linux when using Alt-GR text input (e.g. German keyboard layout), could lead to broken text input. Revert a 2022/01/17 change were we resumed using mods provided by LINEARWIN, turns out they were faulty.
//  2022-11-22: Perform a dummy linearwinGetError() read to cancel missing names with linearwinGetKeyName(). (#5908)
//  2022-10-18: Perform a dummy linearwinGetError() read to cancel missing mouse cursors errors. Using LINEARWIN_VERSION_COMBINED directly. (#5785)
//  2022-10-11: Using 'nullptr' instead of 'NULL' as per our switch to C++11.
//  2022-09-26: Inputs: Renamed LinearUIKey_ModXXX introduced in 1.87 to LinearUIMod_XXX (old names still supported).
//  2022-09-01: Inputs: Honor LINEARWIN_CURSOR_DISABLED by not setting mouse position *EDIT* Reverted 2023-07-18.
//  2022-04-30: Inputs: Fixed LinearUI_ImplLinearWin_TranslateUntranslatedKey() for lower case letters on OSX.
//  2022-03-23: Inputs: Fixed a regression in 1.87 which resulted in keyboard modifiers events being reported incorrectly on Linux/X11.
//  2022-02-07: Added LinearUI_ImplLinearWin_InstallCallbacks()/LinearUI_ImplLinearWin_RestoreCallbacks() helpers to facilitate user installing callbacks after initializing backend.
//  2022-01-26: Inputs: replaced short-lived io.AddKeyModsEvent() (added two weeks ago) with io.AddKeyEvent() using LinearUIKey_ModXXX flags. Sorry for the confusion.
//  2021-01-20: Inputs: calling new io.AddKeyAnalogEvent() for gamepad support, instead of writing directly to io.NavInputs[].
//  2022-01-17: Inputs: calling new io.AddMousePosEvent(), io.AddMouseButtonEvent(), io.AddMouseWheelEvent() API (1.87+).
//  2022-01-17: Inputs: always update key mods next and before key event (not in NewFrame) to fix input queue with very low framerates.
//  2022-01-12: *BREAKING CHANGE*: Now using linearwinSetCursorPosCallback(). If you called LinearUI_ImplLinearWin_InitXXX() with install_callbacks = false, you MUST install linearwinSetCursorPosCallback() and forward it to the backend via LinearUI_ImplLinearWin_CursorPosCallback().
//  2022-01-10: Inputs: calling new io.AddKeyEvent(), io.AddKeyModsEvent() + io.SetKeyEventNativeData() API (1.87+). Support for full LinearUIKey range.
//  2022-01-05: Inputs: Converting LINEARWIN untranslated keycodes back to translated keycodes (in the LinearUI_ImplLinearWin_KeyCallback() function) in order to match the behavior of every other backend, and facilitate the use of LINEARWIN with lettered-shortcuts API.
//  2021-08-17: *BREAKING CHANGE*: Now using linearwinSetWindowFocusCallback() to calling io.AddFocusEvent(). If you called LinearUI_ImplLinearWin_InitXXX() with install_callbacks = false, you MUST install linearwinSetWindowFocusCallback() and forward it to the backend via LinearUI_ImplLinearWin_WindowFocusCallback().
//  2021-07-29: *BREAKING CHANGE*: Now using linearwinSetCursorEnterCallback(). MousePos is correctly reported when the host platform window is hovered but not focused. If you called LinearUI_ImplLinearWin_InitXXX() with install_callbacks = false, you MUST install linearwinSetWindowFocusCallback() callback and forward it to the backend via LinearUI_ImplLinearWin_CursorEnterCallback().
//  2021-06-29: Reorganized backend to pull data from a single structure to facilitate usage with multiple-contexts (all g_XXXX access changed to bd->XXXX).
//  2020-01-17: Inputs: Disable error callback while assigning mouse cursors because some X11 setup don't have them and it generates errors.
//  2019-12-05: Inputs: Added support for new mouse cursors added in LINEARWIN 3.4+ (resizing cursors, not allowed cursor).
//  2019-10-18: Misc: Previously installed user callbacks are now restored on shutdown.
//  2019-07-21: Inputs: Added mapping for LinearUIKey_KeyPadEnter.
//  2019-05-11: Inputs: Don't filter value from character callback before calling AddInputCharacter().
//  2019-03-12: Misc: Preserve DisplayFramebufferScale when main window is minimized.
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-11-07: Inputs: When installing our LINEARWIN callbacks, we save user's previously installed ones - if any - and chain call them.
//  2018-08-01: Inputs: Workaround for Emscripten which doesn't seem to handle focus related calls.
//  2018-06-29: Inputs: Added support for the LinearUIMouseCursor_Hand cursor.
//  2018-06-08: Misc: Extracted linearui_impl_linearwin.cpp/.h away from the old combined LINEARWIN+OpenGL/Vulkan examples.
//  2018-03-20: Misc: Setup io.BackendFlags LinearUIBackendFlags_HasMouseCursors flag + honor LinearUIConfigFlags_NoMouseCursorChange flag.
//  2018-02-20: Inputs: Added support for mouse cursors (LinearUI::GetMouseCursor() value, passed to linearwinSetCursor()).
//  2018-02-06: Misc: Removed call to LinearUI::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.
//  2018-02-06: Inputs: Added mapping for LinearUIKey_Space.
//  2018-01-25: Inputs: Added gamepad support if LinearUIConfigFlags_NavEnableGamepad is set.
//  2018-01-25: Inputs: Honoring the io.WantSetMousePos by repositioning the mouse (when using navigation and LinearUIConfigFlags_NavMoveMouse is set).
//  2018-01-20: Inputs: Added Horizontal Mouse Wheel support.
//  2018-01-18: Inputs: Added mapping for LinearUIKey_Insert.
//  2017-08-25: Inputs: MousePos set to -FLT_MAX,-FLT_MAX when mouse is unavailable/missing (instead of -1,-1).
//  2016-10-15: Misc: Added a void* user_data parameter to Clipboard function handlers.

#include "linearui.h"
#ifndef LINEARUI_DISABLE
#include "linearui_impl_linearwin.h"

// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning: use of old-style cast
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning: implicit conversion changes signedness
#pragma clang diagnostic ignored "-Wexit-time-destructors"  // warning: declaration requires an exit-time destructor     // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. LinearUI coding style welcomes static/globals.
#pragma clang diagnostic ignored "-Wglobal-constructors"    // warning: declaration requires a global destructor         // similar to above, not sure what the exact difference is.
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wfloat-equal"              // warning: comparing floating-point with '==' or '!=' is unsafe
#endif

#if defined(__has_include)
#if !__has_include(<X11/Xlib.h>) || !__has_include(<X11/extensions/Xrandr.h>)
#define LINEARUI_IMPL_LINEARWIN_DISABLE_X11
#endif
#if !__has_include(<wayland-client.h>)
#define LINEARUI_IMPL_LINEARWIN_DISABLE_WAYLAND
#endif
#endif

// LINEARWIN
#if !defined(LINEARUI_IMPL_LINEARWIN_DISABLE_X11) && (defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__))
#define LINEARWIN_HAS_X11        1
#else
#define LINEARWIN_HAS_X11        0
#endif
#if !defined(LINEARUI_IMPL_LINEARWIN_DISABLE_WAYLAND) && (defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__))
#define LINEARWIN_HAS_WAYLAND    1
#else
#define LINEARWIN_HAS_WAYLAND    0
#endif
#include <LINEARWIN/linearwin.h>
#ifdef _WIN32
#undef APIENTRY
#ifndef LINEARWIN_EXPOSE_NATIVE_WIN32    // for linearwinGetWin32Window()
#define LINEARWIN_EXPOSE_NATIVE_WIN32
#endif
#include <LINEARWIN/linearwinnative.h>
#elif defined(__APPLE__)
#ifndef LINEARWIN_EXPOSE_NATIVE_COCOA    // for linearwinGetCocoaWindow()
#define LINEARWIN_EXPOSE_NATIVE_COCOA
#endif
#include <LINEARWIN/linearwinnative.h>
#elif LINEARWIN_HAS_X11
#ifndef LINEARWIN_EXPOSE_NATIVE_X11      // for linearwinGetX11Display(), linearwinGetX11Window() on Freedesktop (Linux, BSD, etc.)
#define LINEARWIN_EXPOSE_NATIVE_X11
#include <X11/Xatom.h>
#include <dlfcn.h>              // for dlopen()
#endif
#include <LINEARWIN/linearwinnative.h>
#undef Status                   // X11 headers are leaking this.
#endif

#ifndef _WIN32
#include <unistd.h>             // for usleep()
#endif
#include <stdio.h>              // for snprintf()

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#ifdef EMSCRIPTEN_USE_PORT_CONTRIB_LINEARWIN3
#include <LINEARWIN/emscripten_linearwin.h>
#else
#define EMSCRIPTEN_USE_EMBEDDED_LINEARWIN3
#endif
#endif

// We gather version tests as define in order to easily see which features are version-dependent.
#define LINEARWIN_VERSION_COMBINED           (LINEARWIN_VERSION_MAJOR * 1000 + LINEARWIN_VERSION_MINOR * 100 + LINEARWIN_VERSION_REVISION)
#define LINEARWIN_HAS_CREATECURSOR           (LINEARWIN_VERSION_COMBINED >= 3100) // 3.1+ linearwinCreateCursor()
#define LINEARWIN_HAS_WINDOW_TOPMOST         (LINEARWIN_VERSION_COMBINED >= 3200) // 3.2+ LINEARWIN_FLOATING
#define LINEARWIN_HAS_WINDOW_HOVERED         (LINEARWIN_VERSION_COMBINED >= 3300) // 3.3+ LINEARWIN_HOVERED
#define LINEARWIN_HAS_WINDOW_ALPHA           (LINEARWIN_VERSION_COMBINED >= 3300) // 3.3+ linearwinSetWindowOpacity
#define LINEARWIN_HAS_PER_MONITOR_DPI        (LINEARWIN_VERSION_COMBINED >= 3300) // 3.3+ linearwinGetMonitorContentScale
#if defined(__EMSCRIPTEN__) || defined(__SWITCH__)                      // no Vulkan support in LINEARWIN for Emscripten or homebrew Nintendo Switch
#define LINEARWIN_HAS_VULKAN                 (0)
#else
#define LINEARWIN_HAS_VULKAN                 (LINEARWIN_VERSION_COMBINED >= 3200) // 3.2+ linearwinCreateWindowSurface
#endif
#define LINEARWIN_HAS_FOCUS_WINDOW           (LINEARWIN_VERSION_COMBINED >= 3200) // 3.2+ linearwinFocusWindow
#define LINEARWIN_HAS_FOCUS_ON_SHOW          (LINEARWIN_VERSION_COMBINED >= 3300) // 3.3+ LINEARWIN_FOCUS_ON_SHOW
#define LINEARWIN_HAS_MONITOR_WORK_AREA      (LINEARWIN_VERSION_COMBINED >= 3300) // 3.3+ linearwinGetMonitorWorkarea
#define LINEARWIN_HAS_OSX_WINDOW_POS_FIX     (LINEARWIN_VERSION_COMBINED >= 3301) // 3.3.1+ Fixed: Resizing window repositions it on MacOS #1553
#ifdef LINEARWIN_RESIZE_NESW_CURSOR          // Let's be nice to people who pulled LINEARWIN between 2019-04-16 (3.4 define) and 2019-11-29 (cursors defines) // FIXME: Remove when LINEARWIN 3.4 is released?
#define LINEARWIN_HAS_NEW_CURSORS            (LINEARWIN_VERSION_COMBINED >= 3400) // 3.4+ LINEARWIN_RESIZE_ALL_CURSOR, LINEARWIN_RESIZE_NESW_CURSOR, LINEARWIN_RESIZE_NWSE_CURSOR, LINEARWIN_NOT_ALLOWED_CURSOR
#else
#define LINEARWIN_HAS_NEW_CURSORS            (0)
#endif
#ifdef LINEARWIN_MOUSE_PASSTHROUGH           // Let's be nice to people who pulled LINEARWIN between 2019-04-16 (3.4 define) and 2020-07-17 (passthrough)
#define LINEARWIN_HAS_MOUSE_PASSTHROUGH      (LINEARWIN_VERSION_COMBINED >= 3400) // 3.4+ LINEARWIN_MOUSE_PASSTHROUGH
#else
#define LINEARWIN_HAS_MOUSE_PASSTHROUGH      (0)
#endif
#define LINEARWIN_HAS_GAMEPAD_API            (LINEARWIN_VERSION_COMBINED >= 3300) // 3.3+ linearwinGetGamepadState() new api
#define LINEARWIN_HAS_GETKEYNAME             (LINEARWIN_VERSION_COMBINED >= 3200) // 3.2+ linearwinGetKeyName()
#define LINEARWIN_HAS_GETERROR               (LINEARWIN_VERSION_COMBINED >= 3300) // 3.3+ linearwinGetError()
#define LINEARWIN_HAS_GETPLATFORM            (LINEARWIN_VERSION_COMBINED >= 3400) // 3.4+ linearwinGetPlatform()

// Map LINEARWINWindow* to LinearUIContext*.
// - Would be simpler if we could use linearwinSetWindowUserPointer()/linearwinGetWindowUserPointer(), but this is a single and shared resource.
// - Would be simpler if we could use e.g. std::map<> as well. But we don't.
// - This is not particularly optimized as we expect size to be small and queries to be rare.
struct LinearUI_ImplLinearWin_WindowToContext { LINEARWINwindow* Window; LinearUIContext* Context; };
static LinearVector<LinearUI_ImplLinearWin_WindowToContext> g_ContextMap;
static void LinearUI_ImplLinearWin_ContextMap_Add(LINEARWINwindow* window, LinearUIContext* ctx) { g_ContextMap.push_back(LinearUI_ImplLinearWin_WindowToContext{ window, ctx }); }
static void LinearUI_ImplLinearWin_ContextMap_Remove(LINEARWINwindow* window)                 { for (LinearUI_ImplLinearWin_WindowToContext& entry : g_ContextMap) if (entry.Window == window) { g_ContextMap.erase_unsorted(&entry); if (g_ContextMap.empty()) g_ContextMap.clear(); return; } }
static LinearUIContext* LinearUI_ImplLinearWin_ContextMap_Get(LINEARWINwindow* window)           { for (LinearUI_ImplLinearWin_WindowToContext& entry : g_ContextMap) if (entry.Window == window) return entry.Context; return nullptr; }

enum LinearWinClientApi
{
    LinearWinClientApi_OpenGL,
    LinearWinClientApi_Vulkan,
    LinearWinClientApi_Unknown,  // Anything else fits here.
};

#if LINEARWIN_HAS_X11
typedef Atom (*PFN_XInternAtom)(Display*, const char* ,Bool);
typedef int  (*PFN_XChangeProperty)(Display*, Window, Atom, Atom, int, int, const unsigned char*, int);
typedef int  (*PFN_XChangeWindowAttributes)(Display*, Window, unsigned long, XSetWindowAttributes*);
typedef int  (*PFN_XFlush)(Display*);
#endif

// LINEARWIN data
struct LinearUI_ImplLinearWin_Data
{
    LinearUIContext*           Context;
    LINEARWINwindow*             Window;
    LinearWinClientApi           ClientApi;
    double                  Time;
    LINEARWINwindow*             MouseWindow;
#if LINEARWIN_HAS_CREATECURSOR
    LINEARWINcursor*             MouseCursors[LinearUIMouseCursor_COUNT];
    LINEARWINcursor*             LastMouseCursor;
#endif
    bool                    MouseIgnoreButtonUpWaitForFocusLoss;
    bool                    MouseIgnoreButtonUp;
    LinearVec2                  LastValidMousePos;
    LINEARWINwindow*             KeyOwnerWindows[LINEARWIN_KEY_LAST];
    bool                    IsWayland;
    bool                    InstalledCallbacks;
    bool                    CallbacksChainForAllWindows;
    char                    BackendPlatformName[32];
#ifdef EMSCRIPTEN_USE_EMBEDDED_LINEARWIN3
    const char*             CanvasSelector;
#endif

    // Chain LINEARWIN callbacks: our callbacks will call the user's previously installed callbacks, if any.
    LINEARWINwindowfocusfun      PrevUserCallbackWindowFocus;
    LINEARWINcursorposfun        PrevUserCallbackCursorPos;
    LINEARWINcursorenterfun      PrevUserCallbackCursorEnter;
    LINEARWINmousebuttonfun      PrevUserCallbackMousebutton;
    LINEARWINscrollfun           PrevUserCallbackScroll;
    LINEARWINkeyfun              PrevUserCallbackKey;
    LINEARWINcharfun             PrevUserCallbackChar;
    LINEARWINmonitorfun          PrevUserCallbackMonitor;
#ifdef _WIN32
    WNDPROC                 PrevWndProc;
#endif

#if LINEARWIN_HAS_X11
    // Module and function pointers loaded at initialization to avoid linking statically with X11.
    void*                       X11Module;
    PFN_XInternAtom             XInternAtom;
    PFN_XChangeProperty         XChangeProperty;
    PFN_XChangeWindowAttributes XChangeWindowAttributes;
    PFN_XFlush                  XFlush;
#endif

    LinearUI_ImplLinearWin_Data()   { memset((void*)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear LinearUI contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear LinearUI context + multiple windows) instead of multiple Dear LinearUI contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
// - Because linearwinPollEvents() process all windows and some events may be called outside of it, you will need to register your own callbacks
//   (passing install_callbacks=false in LinearUI_ImplLinearWin_InitXXX functions), set the current dear linearui context and then call our callbacks.
// - Otherwise we may need to store a LINEARWINWindow* -> LinearUIContext* map and handle this in the backend, adding a little bit of extra complexity to it.
// FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled when using multi-context.
namespace LinearUI { extern LinearUIIO& GetIO(LinearUIContext*); }
static LinearUI_ImplLinearWin_Data* LinearUI_ImplLinearWin_GetBackendData()
{
    // Get data for current context
    return LinearUI::GetCurrentContext() ? (LinearUI_ImplLinearWin_Data*)LinearUI::GetIO().BackendPlatformUserData : nullptr;
}
static LinearUI_ImplLinearWin_Data* LinearUI_ImplLinearWin_GetBackendData(LINEARWINwindow* window)
{
    // Get data for a given LINEARWIN window, regardless of current context (since LINEARWIN events are sent together)
    LinearUIContext* ctx = LinearUI_ImplLinearWin_ContextMap_Get(window);
    return (LinearUI_ImplLinearWin_Data*)LinearUI::GetIO(ctx).BackendPlatformUserData;
}

// Forward Declarations
static void LinearUI_ImplLinearWin_UpdateMonitors();
static void LinearUI_ImplLinearWin_InitMultiViewportSupport();
static void LinearUI_ImplLinearWin_ShutdownMultiViewportSupport();

// Functions
static bool LinearUI_ImplLinearWin_IsWayland()
{
#if !LINEARWIN_HAS_WAYLAND
    return false;
#elif LINEARWIN_HAS_GETPLATFORM
    return linearwinGetPlatform() == LINEARWIN_PLATFORM_WAYLAND;
#else
    const char* version = linearwinGetVersionString();
    if (strstr(version, "Wayland") == nullptr) // e.g. Ubuntu 22.04 ships with LINEARWIN 3.3.6 compiled without Wayland
        return false;
#ifdef LINEARWIN_EXPOSE_NATIVE_X11
    if (linearwinGetX11Display() != nullptr)
        return false;
#endif
    return true;
#endif
}

// Not static to allow third-party code to use that if they want to (but undocumented)
LinearUIKey LinearUI_ImplLinearWin_KeyToLinearUIKey(int keycode, int scancode);
LinearUIKey LinearUI_ImplLinearWin_KeyToLinearUIKey(int keycode, int scancode)
{
    LINEARUI_UNUSED(scancode);
    switch (keycode)
    {
        case LINEARWIN_KEY_TAB: return LinearUIKey_Tab;
        case LINEARWIN_KEY_LEFT: return LinearUIKey_LeftArrow;
        case LINEARWIN_KEY_RIGHT: return LinearUIKey_RightArrow;
        case LINEARWIN_KEY_UP: return LinearUIKey_UpArrow;
        case LINEARWIN_KEY_DOWN: return LinearUIKey_DownArrow;
        case LINEARWIN_KEY_PAGE_UP: return LinearUIKey_PageUp;
        case LINEARWIN_KEY_PAGE_DOWN: return LinearUIKey_PageDown;
        case LINEARWIN_KEY_HOME: return LinearUIKey_Home;
        case LINEARWIN_KEY_END: return LinearUIKey_End;
        case LINEARWIN_KEY_INSERT: return LinearUIKey_Insert;
        case LINEARWIN_KEY_DELETE: return LinearUIKey_Delete;
        case LINEARWIN_KEY_BACKSPACE: return LinearUIKey_Backspace;
        case LINEARWIN_KEY_SPACE: return LinearUIKey_Space;
        case LINEARWIN_KEY_ENTER: return LinearUIKey_Enter;
        case LINEARWIN_KEY_ESCAPE: return LinearUIKey_Escape;
        case LINEARWIN_KEY_APOSTROPHE: return LinearUIKey_Apostrophe;
        case LINEARWIN_KEY_COMMA: return LinearUIKey_Comma;
        case LINEARWIN_KEY_MINUS: return LinearUIKey_Minus;
        case LINEARWIN_KEY_PERIOD: return LinearUIKey_Period;
        case LINEARWIN_KEY_SLASH: return LinearUIKey_Slash;
        case LINEARWIN_KEY_SEMICOLON: return LinearUIKey_Semicolon;
        case LINEARWIN_KEY_EQUAL: return LinearUIKey_Equal;
        case LINEARWIN_KEY_LEFT_BRACKET: return LinearUIKey_LeftBracket;
        case LINEARWIN_KEY_BACKSLASH: return LinearUIKey_Backslash;
        case LINEARWIN_KEY_WORLD_1: return LinearUIKey_Oem102;
        case LINEARWIN_KEY_WORLD_2: return LinearUIKey_Oem102;
        case LINEARWIN_KEY_RIGHT_BRACKET: return LinearUIKey_RightBracket;
        case LINEARWIN_KEY_GRAVE_ACCENT: return LinearUIKey_GraveAccent;
        case LINEARWIN_KEY_CAPS_LOCK: return LinearUIKey_CapsLock;
        case LINEARWIN_KEY_SCROLL_LOCK: return LinearUIKey_ScrollLock;
        case LINEARWIN_KEY_NUM_LOCK: return LinearUIKey_NumLock;
        case LINEARWIN_KEY_PRINT_SCREEN: return LinearUIKey_PrintScreen;
        case LINEARWIN_KEY_PAUSE: return LinearUIKey_Pause;
        case LINEARWIN_KEY_KP_0: return LinearUIKey_Keypad0;
        case LINEARWIN_KEY_KP_1: return LinearUIKey_Keypad1;
        case LINEARWIN_KEY_KP_2: return LinearUIKey_Keypad2;
        case LINEARWIN_KEY_KP_3: return LinearUIKey_Keypad3;
        case LINEARWIN_KEY_KP_4: return LinearUIKey_Keypad4;
        case LINEARWIN_KEY_KP_5: return LinearUIKey_Keypad5;
        case LINEARWIN_KEY_KP_6: return LinearUIKey_Keypad6;
        case LINEARWIN_KEY_KP_7: return LinearUIKey_Keypad7;
        case LINEARWIN_KEY_KP_8: return LinearUIKey_Keypad8;
        case LINEARWIN_KEY_KP_9: return LinearUIKey_Keypad9;
        case LINEARWIN_KEY_KP_DECIMAL: return LinearUIKey_KeypadDecimal;
        case LINEARWIN_KEY_KP_DIVIDE: return LinearUIKey_KeypadDivide;
        case LINEARWIN_KEY_KP_MULTIPLY: return LinearUIKey_KeypadMultiply;
        case LINEARWIN_KEY_KP_SUBTRACT: return LinearUIKey_KeypadSubtract;
        case LINEARWIN_KEY_KP_ADD: return LinearUIKey_KeypadAdd;
        case LINEARWIN_KEY_KP_ENTER: return LinearUIKey_KeypadEnter;
        case LINEARWIN_KEY_KP_EQUAL: return LinearUIKey_KeypadEqual;
        case LINEARWIN_KEY_LEFT_SHIFT: return LinearUIKey_LeftShift;
        case LINEARWIN_KEY_LEFT_CONTROL: return LinearUIKey_LeftCtrl;
        case LINEARWIN_KEY_LEFT_ALT: return LinearUIKey_LeftAlt;
        case LINEARWIN_KEY_LEFT_SUPER: return LinearUIKey_LeftSuper;
        case LINEARWIN_KEY_RIGHT_SHIFT: return LinearUIKey_RightShift;
        case LINEARWIN_KEY_RIGHT_CONTROL: return LinearUIKey_RightCtrl;
        case LINEARWIN_KEY_RIGHT_ALT: return LinearUIKey_RightAlt;
        case LINEARWIN_KEY_RIGHT_SUPER: return LinearUIKey_RightSuper;
        case LINEARWIN_KEY_MENU: return LinearUIKey_Menu;
        case LINEARWIN_KEY_0: return LinearUIKey_0;
        case LINEARWIN_KEY_1: return LinearUIKey_1;
        case LINEARWIN_KEY_2: return LinearUIKey_2;
        case LINEARWIN_KEY_3: return LinearUIKey_3;
        case LINEARWIN_KEY_4: return LinearUIKey_4;
        case LINEARWIN_KEY_5: return LinearUIKey_5;
        case LINEARWIN_KEY_6: return LinearUIKey_6;
        case LINEARWIN_KEY_7: return LinearUIKey_7;
        case LINEARWIN_KEY_8: return LinearUIKey_8;
        case LINEARWIN_KEY_9: return LinearUIKey_9;
        case LINEARWIN_KEY_A: return LinearUIKey_A;
        case LINEARWIN_KEY_B: return LinearUIKey_B;
        case LINEARWIN_KEY_C: return LinearUIKey_C;
        case LINEARWIN_KEY_D: return LinearUIKey_D;
        case LINEARWIN_KEY_E: return LinearUIKey_E;
        case LINEARWIN_KEY_F: return LinearUIKey_F;
        case LINEARWIN_KEY_G: return LinearUIKey_G;
        case LINEARWIN_KEY_H: return LinearUIKey_H;
        case LINEARWIN_KEY_I: return LinearUIKey_I;
        case LINEARWIN_KEY_J: return LinearUIKey_J;
        case LINEARWIN_KEY_K: return LinearUIKey_K;
        case LINEARWIN_KEY_L: return LinearUIKey_L;
        case LINEARWIN_KEY_M: return LinearUIKey_M;
        case LINEARWIN_KEY_N: return LinearUIKey_N;
        case LINEARWIN_KEY_O: return LinearUIKey_O;
        case LINEARWIN_KEY_P: return LinearUIKey_P;
        case LINEARWIN_KEY_Q: return LinearUIKey_Q;
        case LINEARWIN_KEY_R: return LinearUIKey_R;
        case LINEARWIN_KEY_S: return LinearUIKey_S;
        case LINEARWIN_KEY_T: return LinearUIKey_T;
        case LINEARWIN_KEY_U: return LinearUIKey_U;
        case LINEARWIN_KEY_V: return LinearUIKey_V;
        case LINEARWIN_KEY_W: return LinearUIKey_W;
        case LINEARWIN_KEY_X: return LinearUIKey_X;
        case LINEARWIN_KEY_Y: return LinearUIKey_Y;
        case LINEARWIN_KEY_Z: return LinearUIKey_Z;
        case LINEARWIN_KEY_F1: return LinearUIKey_F1;
        case LINEARWIN_KEY_F2: return LinearUIKey_F2;
        case LINEARWIN_KEY_F3: return LinearUIKey_F3;
        case LINEARWIN_KEY_F4: return LinearUIKey_F4;
        case LINEARWIN_KEY_F5: return LinearUIKey_F5;
        case LINEARWIN_KEY_F6: return LinearUIKey_F6;
        case LINEARWIN_KEY_F7: return LinearUIKey_F7;
        case LINEARWIN_KEY_F8: return LinearUIKey_F8;
        case LINEARWIN_KEY_F9: return LinearUIKey_F9;
        case LINEARWIN_KEY_F10: return LinearUIKey_F10;
        case LINEARWIN_KEY_F11: return LinearUIKey_F11;
        case LINEARWIN_KEY_F12: return LinearUIKey_F12;
        case LINEARWIN_KEY_F13: return LinearUIKey_F13;
        case LINEARWIN_KEY_F14: return LinearUIKey_F14;
        case LINEARWIN_KEY_F15: return LinearUIKey_F15;
        case LINEARWIN_KEY_F16: return LinearUIKey_F16;
        case LINEARWIN_KEY_F17: return LinearUIKey_F17;
        case LINEARWIN_KEY_F18: return LinearUIKey_F18;
        case LINEARWIN_KEY_F19: return LinearUIKey_F19;
        case LINEARWIN_KEY_F20: return LinearUIKey_F20;
        case LINEARWIN_KEY_F21: return LinearUIKey_F21;
        case LINEARWIN_KEY_F22: return LinearUIKey_F22;
        case LINEARWIN_KEY_F23: return LinearUIKey_F23;
        case LINEARWIN_KEY_F24: return LinearUIKey_F24;
        default: return LinearUIKey_None;
    }
}

// X11 does not include current pressed/released modifier key in 'mods' flags submitted by LINEARWIN
// See https://github.com/ocornut/linearui/issues/6034 and https://github.com/linearwin/linearwin/issues/1630
static void LinearUI_ImplLinearWin_UpdateKeyModifiers(LinearUIIO& io, LINEARWINwindow* window)
{
    io.AddKeyEvent(LinearUIMod_Ctrl,  (linearwinGetKey(window, LINEARWIN_KEY_LEFT_CONTROL) == LINEARWIN_PRESS) || (linearwinGetKey(window, LINEARWIN_KEY_RIGHT_CONTROL) == LINEARWIN_PRESS));
    io.AddKeyEvent(LinearUIMod_Shift, (linearwinGetKey(window, LINEARWIN_KEY_LEFT_SHIFT)   == LINEARWIN_PRESS) || (linearwinGetKey(window, LINEARWIN_KEY_RIGHT_SHIFT)   == LINEARWIN_PRESS));
    io.AddKeyEvent(LinearUIMod_Alt,   (linearwinGetKey(window, LINEARWIN_KEY_LEFT_ALT)     == LINEARWIN_PRESS) || (linearwinGetKey(window, LINEARWIN_KEY_RIGHT_ALT)     == LINEARWIN_PRESS));
    io.AddKeyEvent(LinearUIMod_Super, (linearwinGetKey(window, LINEARWIN_KEY_LEFT_SUPER)   == LINEARWIN_PRESS) || (linearwinGetKey(window, LINEARWIN_KEY_RIGHT_SUPER)   == LINEARWIN_PRESS));
}

static bool LinearUI_ImplLinearWin_ShouldChainCallback(LinearUI_ImplLinearWin_Data* bd, LINEARWINwindow* window)
{
    return bd->CallbacksChainForAllWindows ? true : (window == bd->Window);
}

void LinearUI_ImplLinearWin_MouseButtonCallback(LINEARWINwindow* window, int button, int action, int mods)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData(window);
    if (bd->PrevUserCallbackMousebutton != nullptr && LinearUI_ImplLinearWin_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackMousebutton(window, button, action, mods);

    // Workaround for Linux: ignore mouse up events which are following an focus loss following a viewport creation
    if (bd->MouseIgnoreButtonUp && action == LINEARWIN_RELEASE)
        return;

    LinearUIIO& io = LinearUI::GetIO(bd->Context);
    LinearUI_ImplLinearWin_UpdateKeyModifiers(io, window);
    if (button >= 0 && button < LinearUIMouseButton_COUNT)
        io.AddMouseButtonEvent(button, action == LINEARWIN_PRESS);
}

void LinearUI_ImplLinearWin_ScrollCallback(LINEARWINwindow* window, double xoffset, double yoffset)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData(window);
    if (bd->PrevUserCallbackScroll != nullptr && LinearUI_ImplLinearWin_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackScroll(window, xoffset, yoffset);

#ifdef EMSCRIPTEN_USE_EMBEDDED_LINEARWIN3
    // Ignore LINEARWIN events: will be processed in LinearUI_ImplEmscripten_WheelCallback().
    return;
#endif

    LinearUIIO& io = LinearUI::GetIO(bd->Context);
    io.AddMouseWheelEvent((float)xoffset, (float)yoffset);
}

// FIXME: should this be baked into LinearUI_ImplLinearWin_KeyToLinearUIKey()? then what about the values passed to io.SetKeyEventNativeData()?
static int LinearUI_ImplLinearWin_TranslateUntranslatedKey(int key, int scancode)
{
#if LINEARWIN_HAS_GETKEYNAME && !defined(EMSCRIPTEN_USE_EMBEDDED_LINEARWIN3)
    // LINEARWIN 3.1+ attempts to "untranslate" keys, which goes the opposite of what every other framework does, making using lettered shortcuts difficult.
    // (It had reasons to do so: namely LINEARWIN is/was more likely to be used for WASD-type game controls rather than lettered shortcuts, but IHMO the 3.1 change could have been done differently)
    // See https://github.com/linearwin/linearwin/issues/1502 for details.
    // Adding a workaround to undo this (so our keys are translated->untranslated->translated, likely a lossy process).
    // This won't cover edge cases but this is at least going to cover common cases.
    if (key >= LINEARWIN_KEY_KP_0 && key <= LINEARWIN_KEY_KP_EQUAL)
        return key;
    LINEARWINerrorfun prev_error_callback = linearwinSetErrorCallback(nullptr);
    const char* key_name = linearwinGetKeyName(key, scancode);
    linearwinSetErrorCallback(prev_error_callback);
#if LINEARWIN_HAS_GETERROR && !defined(EMSCRIPTEN_USE_EMBEDDED_LINEARWIN3) // Eat errors (see #5908)
    (void)linearwinGetError(nullptr);
#endif
    if (key_name && key_name[0] != 0 && key_name[1] == 0)
    {
        const char char_names[] = "`-=[]\\,;\'./";
        const int char_keys[] = { LINEARWIN_KEY_GRAVE_ACCENT, LINEARWIN_KEY_MINUS, LINEARWIN_KEY_EQUAL, LINEARWIN_KEY_LEFT_BRACKET, LINEARWIN_KEY_RIGHT_BRACKET, LINEARWIN_KEY_BACKSLASH, LINEARWIN_KEY_COMMA, LINEARWIN_KEY_SEMICOLON, LINEARWIN_KEY_APOSTROPHE, LINEARWIN_KEY_PERIOD, LINEARWIN_KEY_SLASH, 0 };
        LINEARUI_ASSERT(LINEARUI_COUNTOF(char_names) == LINEARUI_COUNTOF(char_keys));
        if (key_name[0] >= '0' && key_name[0] <= '9')               { key = LINEARWIN_KEY_0 + (key_name[0] - '0'); }
        else if (key_name[0] >= 'A' && key_name[0] <= 'Z')          { key = LINEARWIN_KEY_A + (key_name[0] - 'A'); }
        else if (key_name[0] >= 'a' && key_name[0] <= 'z')          { key = LINEARWIN_KEY_A + (key_name[0] - 'a'); }
        else if (const char* p = strchr(char_names, key_name[0]))   { key = char_keys[p - char_names]; }
    }
    // if (action == LINEARWIN_PRESS) printf("key %d scancode %d name '%s'\n", key, scancode, key_name);
#else
    LINEARUI_UNUSED(scancode);
#endif
    return key;
}

void LinearUI_ImplLinearWin_KeyCallback(LINEARWINwindow* window, int keycode, int scancode, int action, int mods)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData(window);
    if (bd->PrevUserCallbackKey != nullptr && LinearUI_ImplLinearWin_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackKey(window, keycode, scancode, action, mods);

    if (action != LINEARWIN_PRESS && action != LINEARWIN_RELEASE)
        return;

    LinearUIIO& io = LinearUI::GetIO(bd->Context);
    LinearUI_ImplLinearWin_UpdateKeyModifiers(io, window);

    if (keycode >= 0 && keycode < LINEARUI_COUNTOF(bd->KeyOwnerWindows))
        bd->KeyOwnerWindows[keycode] = (action == LINEARWIN_PRESS) ? window : nullptr;

    keycode = LinearUI_ImplLinearWin_TranslateUntranslatedKey(keycode, scancode);

    LinearUIKey linearui_key = LinearUI_ImplLinearWin_KeyToLinearUIKey(keycode, scancode);
    io.AddKeyEvent(linearui_key, (action == LINEARWIN_PRESS));
    io.SetKeyEventNativeData(linearui_key, keycode, scancode); // To support legacy indexing (<1.87 user code)
}

void LinearUI_ImplLinearWin_WindowFocusCallback(LINEARWINwindow* window, int focused)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData(window);
    if (bd->PrevUserCallbackWindowFocus != nullptr && LinearUI_ImplLinearWin_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackWindowFocus(window, focused);

    // Workaround for Linux: when losing focus with MouseIgnoreButtonUpWaitForFocusLoss set, we will temporarily ignore subsequent Mouse Up events
    bd->MouseIgnoreButtonUp = (bd->MouseIgnoreButtonUpWaitForFocusLoss && focused == 0);
    bd->MouseIgnoreButtonUpWaitForFocusLoss = false;

    LinearUIIO& io = LinearUI::GetIO(bd->Context);
    io.AddFocusEvent(focused != 0);
}

void LinearUI_ImplLinearWin_CursorPosCallback(LINEARWINwindow* window, double x, double y)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData(window);
    if (bd->PrevUserCallbackCursorPos != nullptr && LinearUI_ImplLinearWin_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackCursorPos(window, x, y);

    LinearUIIO& io = LinearUI::GetIO(bd->Context);
    if (io.ConfigFlags & LinearUIConfigFlags_ViewportsEnable)
    {
        int window_x, window_y;
        linearwinGetWindowPos(window, &window_x, &window_y);
        x += window_x;
        y += window_y;
    }
    io.AddMousePosEvent((float)x, (float)y);
    bd->LastValidMousePos = LinearVec2((float)x, (float)y);
}

// Workaround: X11 seems to send spurious Leave/Enter events which would make us lose our position,
// so we back it up and restore on Leave/Enter (see https://github.com/ocornut/linearui/issues/4984)
void LinearUI_ImplLinearWin_CursorEnterCallback(LINEARWINwindow* window, int entered)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData(window);
    if (bd->PrevUserCallbackCursorEnter != nullptr && LinearUI_ImplLinearWin_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackCursorEnter(window, entered);

    LinearUIIO& io = LinearUI::GetIO(bd->Context);
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

void LinearUI_ImplLinearWin_CharCallback(LINEARWINwindow* window, unsigned int c)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData(window);
    if (bd->PrevUserCallbackChar != nullptr && LinearUI_ImplLinearWin_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackChar(window, c);

    LinearUIIO& io = LinearUI::GetIO(bd->Context);
    io.AddInputCharacter(c);
}

void LinearUI_ImplLinearWin_MonitorCallback(LINEARWINmonitor*, int)
{
    // This function is technically part of the API even if we stopped using the callback, so leaving it around.
}

#ifdef EMSCRIPTEN_USE_EMBEDDED_LINEARWIN3
static EM_BOOL LinearUI_ImplEmscripten_WheelCallback(int, const EmscriptenWheelEvent* ev, void* user_data)
{
    // Mimic Emscripten_HandleWheel() in SDL.
    // Corresponding equivalent in LINEARWIN JS emulation layer has incorrect quantizing preventing small values. See #6096
    LinearUI_ImplLinearWin_Data* bd = (LinearUI_ImplLinearWin_Data*)user_data;
    float multiplier = 0.0f;
    if (ev->deltaMode == DOM_DELTA_PIXEL)       { multiplier = 1.0f / 100.0f; } // 100 pixels make up a step.
    else if (ev->deltaMode == DOM_DELTA_LINE)   { multiplier = 1.0f / 3.0f; }   // 3 lines make up a step.
    else if (ev->deltaMode == DOM_DELTA_PAGE)   { multiplier = 80.0f; }         // A page makes up 80 steps.
    float wheel_x = ev->deltaX * -multiplier;
    float wheel_y = ev->deltaY * -multiplier;
    LinearUIIO& io = LinearUI::GetIO(bd->Context);
    io.AddMouseWheelEvent(wheel_x, wheel_y);
    //LINEARUI_DEBUG_LOG("[Emsc] mode %d dx: %.2f, dy: %.2f, dz: %.2f --> feed %.2f %.2f\n", (int)ev->deltaMode, ev->deltaX, ev->deltaY, ev->deltaZ, wheel_x, wheel_y);
    return EM_TRUE;
}
#endif

#ifdef _WIN32
static LRESULT CALLBACK LinearUI_ImplLinearWin_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

void LinearUI_ImplLinearWin_InstallCallbacks(LINEARWINwindow* window)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData(window);
    LINEARUI_ASSERT(bd->InstalledCallbacks == false && "Callbacks already installed!");
    LINEARUI_ASSERT(bd->Window == window);

    bd->PrevUserCallbackWindowFocus = linearwinSetWindowFocusCallback(window, LinearUI_ImplLinearWin_WindowFocusCallback);
    bd->PrevUserCallbackCursorEnter = linearwinSetCursorEnterCallback(window, LinearUI_ImplLinearWin_CursorEnterCallback);
    bd->PrevUserCallbackCursorPos = linearwinSetCursorPosCallback(window, LinearUI_ImplLinearWin_CursorPosCallback);
    bd->PrevUserCallbackMousebutton = linearwinSetMouseButtonCallback(window, LinearUI_ImplLinearWin_MouseButtonCallback);
    bd->PrevUserCallbackScroll = linearwinSetScrollCallback(window, LinearUI_ImplLinearWin_ScrollCallback);
    bd->PrevUserCallbackKey = linearwinSetKeyCallback(window, LinearUI_ImplLinearWin_KeyCallback);
    bd->PrevUserCallbackChar = linearwinSetCharCallback(window, LinearUI_ImplLinearWin_CharCallback);
    bd->PrevUserCallbackMonitor = linearwinSetMonitorCallback(LinearUI_ImplLinearWin_MonitorCallback);
    bd->InstalledCallbacks = true;
}

void LinearUI_ImplLinearWin_RestoreCallbacks(LINEARWINwindow* window)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData(window);
    LINEARUI_ASSERT(bd->InstalledCallbacks == true && "Callbacks not installed!");
    LINEARUI_ASSERT(bd->Window == window);

    linearwinSetWindowFocusCallback(window, bd->PrevUserCallbackWindowFocus);
    linearwinSetCursorEnterCallback(window, bd->PrevUserCallbackCursorEnter);
    linearwinSetCursorPosCallback(window, bd->PrevUserCallbackCursorPos);
    linearwinSetMouseButtonCallback(window, bd->PrevUserCallbackMousebutton);
    linearwinSetScrollCallback(window, bd->PrevUserCallbackScroll);
    linearwinSetKeyCallback(window, bd->PrevUserCallbackKey);
    linearwinSetCharCallback(window, bd->PrevUserCallbackChar);
    linearwinSetMonitorCallback(bd->PrevUserCallbackMonitor);
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
void LinearUI_ImplLinearWin_SetCallbacksChainForAllWindows(bool chain_for_all_windows)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData();
    bd->CallbacksChainForAllWindows = chain_for_all_windows;
}

#ifdef __EMSCRIPTEN__
#if EMSCRIPTEN_USE_PORT_CONTRIB_LINEARWIN3 >= 34020240817
void LinearUI_ImplLinearWin_EmscriptenOpenURL(const char* url) { if (url) emscripten::linearwin::OpenURL(url); }
#else
EM_JS(void, LinearUI_ImplLinearWin_EmscriptenOpenURL, (const char* url), { url = url ? UTF8ToString(url) : null; if (url) window.open(url, '_blank'); });
#endif
#endif

static bool LinearUI_ImplLinearWin_Init(LINEARWINwindow* window, bool install_callbacks, LinearWinClientApi client_api)
{
    LinearUIIO& io = LinearUI::GetIO();
    LINEARUI_CHECKVERSION();
    LINEARUI_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");
    //printf("LINEARWIN_VERSION: %d.%d.%d (%d)", LINEARWIN_VERSION_MAJOR, LINEARWIN_VERSION_MINOR, LINEARWIN_VERSION_REVISION, LINEARWIN_VERSION_COMBINED);

    // Setup backend capabilities flags
    LinearUI_ImplLinearWin_Data* bd = LINEARUI_NEW(LinearUI_ImplLinearWin_Data)();
    snprintf(bd->BackendPlatformName, sizeof(bd->BackendPlatformName), "linearui_impl_linearwin (%d)", LINEARWIN_VERSION_COMBINED);
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = bd->BackendPlatformName;
#if LINEARWIN_HAS_CREATECURSOR
    io.BackendFlags |= LinearUIBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
#endif
    io.BackendFlags |= LinearUIBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)

    bool has_viewports = false;
#ifndef __EMSCRIPTEN__
    has_viewports = true;
#if LINEARWIN_HAS_GETPLATFORM
    if (linearwinGetPlatform() == LINEARWIN_PLATFORM_WAYLAND)
        has_viewports = false;
#endif
    if (has_viewports)
        io.BackendFlags |= LinearUIBackendFlags_PlatformHasViewports;  // We can create multi-viewports on the Platform side (optional)
#endif
#if LINEARWIN_HAS_MOUSE_PASSTHROUGH || LINEARWIN_HAS_WINDOW_HOVERED
    io.BackendFlags |= LinearUIBackendFlags_HasMouseHoveredViewport; // We can call io.AddMouseViewportEvent() with correct data (optional)
#endif

    bd->Context = LinearUI::GetCurrentContext();
    bd->Window = window;
    bd->Time = 0.0;
    bd->IsWayland = LinearUI_ImplLinearWin_IsWayland();
    LinearUI_ImplLinearWin_ContextMap_Add(window, bd->Context);

    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
#if LINEARWIN_VERSION_COMBINED < 3300
    platform_io.Platform_SetClipboardTextFn = [](LinearUIContext*, const char* text) { linearwinSetClipboardString(LinearUI_ImplLinearWin_GetBackendData()->Window, text); };
    platform_io.Platform_GetClipboardTextFn = [](LinearUIContext*) { return linearwinGetClipboardString(LinearUI_ImplLinearWin_GetBackendData()->Window); };
#else
    platform_io.Platform_SetClipboardTextFn = [](LinearUIContext*, const char* text) { linearwinSetClipboardString(nullptr, text); };
    platform_io.Platform_GetClipboardTextFn = [](LinearUIContext*) { return linearwinGetClipboardString(nullptr); };
#endif

#ifdef __EMSCRIPTEN__
    platform_io.Platform_OpenInShellFn = [](LinearUIContext*, const char* url) { LinearUI_ImplLinearWin_EmscriptenOpenURL(url); return true; };
#endif

    // Create mouse cursors
    // (By design, on X11 cursors are user configurable and some cursors may be missing. When a cursor doesn't exist,
    // LINEARWIN will emit an error which will often be printed by the app, so we temporarily disable error reporting.
    // Missing cursors will return nullptr and our _UpdateMouseCursor() function will use the Arrow cursor instead.)
#if LINEARWIN_HAS_CREATECURSOR
    LINEARWINerrorfun prev_error_callback = linearwinSetErrorCallback(nullptr);
    bd->MouseCursors[LinearUIMouseCursor_Arrow] = linearwinCreateStandardCursor(LINEARWIN_ARROW_CURSOR);
    bd->MouseCursors[LinearUIMouseCursor_TextInput] = linearwinCreateStandardCursor(LINEARWIN_IBEAM_CURSOR);
    bd->MouseCursors[LinearUIMouseCursor_ResizeNS] = linearwinCreateStandardCursor(LINEARWIN_VRESIZE_CURSOR);
    bd->MouseCursors[LinearUIMouseCursor_ResizeEW] = linearwinCreateStandardCursor(LINEARWIN_HRESIZE_CURSOR);
    bd->MouseCursors[LinearUIMouseCursor_Hand] = linearwinCreateStandardCursor(LINEARWIN_HAND_CURSOR);
#if LINEARWIN_HAS_NEW_CURSORS
    bd->MouseCursors[LinearUIMouseCursor_ResizeAll] = linearwinCreateStandardCursor(LINEARWIN_RESIZE_ALL_CURSOR);
    bd->MouseCursors[LinearUIMouseCursor_ResizeNESW] = linearwinCreateStandardCursor(LINEARWIN_RESIZE_NESW_CURSOR);
    bd->MouseCursors[LinearUIMouseCursor_ResizeNWSE] = linearwinCreateStandardCursor(LINEARWIN_RESIZE_NWSE_CURSOR);
    bd->MouseCursors[LinearUIMouseCursor_NotAllowed] = linearwinCreateStandardCursor(LINEARWIN_NOT_ALLOWED_CURSOR);
#else
    bd->MouseCursors[LinearUIMouseCursor_ResizeAll] = linearwinCreateStandardCursor(LINEARWIN_ARROW_CURSOR);
    bd->MouseCursors[LinearUIMouseCursor_ResizeNESW] = linearwinCreateStandardCursor(LINEARWIN_ARROW_CURSOR);
    bd->MouseCursors[LinearUIMouseCursor_ResizeNWSE] = linearwinCreateStandardCursor(LINEARWIN_ARROW_CURSOR);
    bd->MouseCursors[LinearUIMouseCursor_NotAllowed] = linearwinCreateStandardCursor(LINEARWIN_ARROW_CURSOR);
#endif
    linearwinSetErrorCallback(prev_error_callback);
#endif
#if LINEARWIN_HAS_GETERROR && !defined(__EMSCRIPTEN__) // Eat errors (see #5908)
    (void)linearwinGetError(nullptr);
#endif

    // Chain LINEARWIN callbacks: our callbacks will call the user's previously installed callbacks, if any.
    if (install_callbacks)
        LinearUI_ImplLinearWin_InstallCallbacks(window);

    // Update monitor a first time during init
    // (note: monitor callback are broken in LINEARWIN 3.2 and earlier, see github.com/linearwin/linearwin/issues/784)
    LinearUI_ImplLinearWin_UpdateMonitors();
    linearwinSetMonitorCallback(LinearUI_ImplLinearWin_MonitorCallback);

    // Set platform dependent data in viewport
    LinearUIViewport* main_viewport = LinearUI::GetMainViewport();
    main_viewport->PlatformHandle = (void*)bd->Window;
#ifdef _WIN32
    main_viewport->PlatformHandleRaw = linearwinGetWin32Window(bd->Window);
#elif defined(__APPLE__)
    main_viewport->PlatformHandleRaw = (void*)linearwinGetCocoaWindow(bd->Window);
#else
    LINEARUI_UNUSED(main_viewport);
#endif
    if (has_viewports)
        LinearUI_ImplLinearWin_InitMultiViewportSupport();

    // Windows: register a WndProc hook so we can intercept some messages.
#ifdef _WIN32
    HWND hwnd = (HWND)main_viewport->PlatformHandleRaw;
    ::SetPropA(hwnd, "LINEARUI_BACKEND_DATA", bd);
    bd->PrevWndProc = (WNDPROC)::GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    LINEARUI_ASSERT(bd->PrevWndProc != nullptr);
    ::SetWindowLongPtrW((HWND)main_viewport->PlatformHandleRaw, GWLP_WNDPROC, (LONG_PTR)LinearUI_ImplLinearWin_WndProc);
#endif

#if LINEARWIN_HAS_X11
    if (!bd->IsWayland)
    {
        // Load X11 module dynamically. Copied from the way that LINEARWIN does it in x11_init.c
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
        LINEARUI_ASSERT(bd->XInternAtom != nullptr && bd->XChangeProperty != nullptr && bd->XChangeWindowAttributes != nullptr && bd->XFlush != nullptr);
    }
#endif

    // Emscripten: the same application can run on various platforms, so we detect the Apple platform at runtime
    // to override io.ConfigMacOSXBehaviors from its default (which is always false in Emscripten).
#ifdef __EMSCRIPTEN__
#if EMSCRIPTEN_USE_PORT_CONTRIB_LINEARWIN3 >= 34020240817
    if (emscripten::linearwin::IsRuntimePlatformApple())
    {
        io.ConfigMacOSXBehaviors = true;

        // Due to how the browser (poorly) handles the Meta Key, this line essentially disables repeats when used.
        // This means that Meta + V only registers a single key-press, even if the keys are held.
        // This is a compromise for dealing with this issue in LinearUI since LinearUI implements key repeat itself.
        // See https://github.com/pongasoft/emscripten-linearwin/blob/v3.4.0.20240817/docs/Usage.md#the-problem-of-the-super-key
        emscripten::linearwin::SetSuperPlusKeyTimeouts(10, 10);
    }
#endif
#endif

    bd->ClientApi = client_api;
    return true;
}

bool LinearUI_ImplLinearWin_InitForOpenGL(LINEARWINwindow* window, bool install_callbacks)
{
    return LinearUI_ImplLinearWin_Init(window, install_callbacks, LinearWinClientApi_OpenGL);
}

bool LinearUI_ImplLinearWin_InitForVulkan(LINEARWINwindow* window, bool install_callbacks)
{
    return LinearUI_ImplLinearWin_Init(window, install_callbacks, LinearWinClientApi_Vulkan);
}

bool LinearUI_ImplLinearWin_InitForOther(LINEARWINwindow* window, bool install_callbacks)
{
    return LinearUI_ImplLinearWin_Init(window, install_callbacks, LinearWinClientApi_Unknown);
}

void LinearUI_ImplLinearWin_Shutdown()
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData();
    LINEARUI_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");

    LinearUIIO& io = LinearUI::GetIO();
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();

    LinearUI_ImplLinearWin_ShutdownMultiViewportSupport();
    if (bd->InstalledCallbacks)
        LinearUI_ImplLinearWin_RestoreCallbacks(bd->Window);
#ifdef EMSCRIPTEN_USE_EMBEDDED_LINEARWIN3
    if (bd->CanvasSelector)
        emscripten_set_wheel_callback(bd->CanvasSelector, nullptr, false, nullptr);
#endif
#if LINEARWIN_HAS_CREATECURSOR
    for (LinearUIMouseCursor cursor_n = 0; cursor_n < LinearUIMouseCursor_COUNT; cursor_n++)
        linearwinDestroyCursor(bd->MouseCursors[cursor_n]);
#endif
    // Windows: restore our WndProc hook
#ifdef _WIN32
    LinearUIViewport* main_viewport = LinearUI::GetMainViewport();
    ::SetPropA((HWND)main_viewport->PlatformHandleRaw, "LINEARUI_BACKEND_DATA", nullptr);
    ::SetWindowLongPtrW((HWND)main_viewport->PlatformHandleRaw, GWLP_WNDPROC, (LONG_PTR)bd->PrevWndProc);
    bd->PrevWndProc = nullptr;
#endif

#if LINEARWIN_HAS_X11
    if (bd->X11Module != nullptr)
        dlclose(bd->X11Module);
#endif

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(LinearUIBackendFlags_HasMouseCursors | LinearUIBackendFlags_HasSetMousePos | LinearUIBackendFlags_HasGamepad | LinearUIBackendFlags_PlatformHasViewports | LinearUIBackendFlags_HasMouseHoveredViewport);
    platform_io.ClearPlatformHandlers();
    LinearUI_ImplLinearWin_ContextMap_Remove(bd->Window);
    LINEARUI_DELETE(bd);
}

static void LinearUI_ImplLinearWin_UpdateMouseData()
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData();
    LinearUIIO& io = LinearUI::GetIO();
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();

    LinearUIID mouse_viewport_id = 0;
    const LinearVec2 mouse_pos_prev = io.MousePos;
    for (int n = 0; n < platform_io.Viewports.Size; n++)
    {
        LinearUIViewport* viewport = platform_io.Viewports[n];
        LINEARWINwindow* window = (LINEARWINwindow*)viewport->PlatformHandle;

#ifdef EMSCRIPTEN_USE_EMBEDDED_LINEARWIN3
        const bool is_window_focused = true;
#else
        const bool is_window_focused = linearwinGetWindowAttrib(window, LINEARWIN_FOCUSED) != 0;
#endif
        if (is_window_focused)
        {
            // (Optional) Set OS mouse position from Dear LinearUI if requested (rarely used, only when io.ConfigNavMoveSetMousePos is enabled by user)
            // When multi-viewports are enabled, all Dear LinearUI positions are same as OS positions.
            if (io.WantSetMousePos)
                linearwinSetCursorPos(window, (double)(mouse_pos_prev.x - viewport->Pos.x), (double)(mouse_pos_prev.y - viewport->Pos.y));

            // (Optional) Fallback to provide mouse position when focused (LinearUI_ImplLinearWin_CursorPosCallback already provides this when hovered or captured)
            if (bd->MouseWindow == nullptr)
            {
                double mouse_x, mouse_y;
                linearwinGetCursorPos(window, &mouse_x, &mouse_y);
                if (io.ConfigFlags & LinearUIConfigFlags_ViewportsEnable)
                {
                    // Single viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window)
                    // Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
                    int window_x, window_y;
                    linearwinGetWindowPos(window, &window_x, &window_y);
                    mouse_x += window_x;
                    mouse_y += window_y;
                }
                bd->LastValidMousePos = LinearVec2((float)mouse_x, (float)mouse_y);
                io.AddMousePosEvent((float)mouse_x, (float)mouse_y);
            }
        }

        // (Optional) When using multiple viewports: call io.AddMouseViewportEvent() with the viewport the OS mouse cursor is hovering.
        // If LinearUIBackendFlags_HasMouseHoveredViewport is not set by the backend, Dear imGui will ignore this field and infer the information using its flawed heuristic.
        // - [X] LINEARWIN >= 3.3 backend ON WINDOWS ONLY does correctly ignore viewports with the _NoInputs flag (since we implement hit via our WndProc hook)
        //       On other platforms we rely on the library fallbacking to its own search when reporting a viewport with _NoInputs flag.
        // - [!] LINEARWIN <= 3.2 backend CANNOT correctly ignore viewports with the _NoInputs flag, and CANNOT reported Hovered Viewport because of mouse capture.
        //       Some backend are not able to handle that correctly. If a backend report an hovered viewport that has the _NoInputs flag (e.g. when dragging a window
        //       for docking, the viewport has the _NoInputs flag in order to allow us to find the viewport under), then Dear LinearUI is forced to ignore the value reported
        //       by the backend, and use its flawed heuristic to guess the viewport behind.
        // - [X] LINEARWIN backend correctly reports this regardless of another viewport behind focused and dragged from (we need this to find a useful drag and drop target).
        // FIXME: This is currently only correct on Win32. See what we do below with the WM_NCHITTEST, missing an equivalent for other systems.
        // See https://github.com/linearwin/linearwin/issues/1236 if you want to help in making this a LINEARWIN feature.
#if LINEARWIN_HAS_MOUSE_PASSTHROUGH
        const bool window_no_input = (viewport->Flags & LinearUIViewportFlags_NoInputs) != 0;
        linearwinSetWindowAttrib(window, LINEARWIN_MOUSE_PASSTHROUGH, window_no_input);
#endif
#if LINEARWIN_HAS_MOUSE_PASSTHROUGH || LINEARWIN_HAS_WINDOW_HOVERED
        if (linearwinGetWindowAttrib(window, LINEARWIN_HOVERED))
            mouse_viewport_id = viewport->ID;
#else
        // We cannot use bd->MouseWindow maintained from CursorEnter/Leave callbacks, because it is locked to the window capturing mouse.
#endif
    }

    if (io.BackendFlags & LinearUIBackendFlags_HasMouseHoveredViewport)
        io.AddMouseViewportEvent(mouse_viewport_id);
}

static void LinearUI_ImplLinearWin_UpdateMouseCursor()
{
    LinearUIIO& io = LinearUI::GetIO();
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData();
    if ((io.ConfigFlags & LinearUIConfigFlags_NoMouseCursorChange) || linearwinGetInputMode(bd->Window, LINEARWIN_CURSOR) == LINEARWIN_CURSOR_DISABLED)
    {
        bd->LastMouseCursor = nullptr;  // Invalidate so that if user changes underlying cursor we will update it next time we can.
        return;
    }

    LinearUIMouseCursor linearui_cursor = LinearUI::GetMouseCursor();
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
    for (int n = 0; n < platform_io.Viewports.Size; n++)
    {
        LINEARWINwindow* window = (LINEARWINwindow*)platform_io.Viewports[n]->PlatformHandle;
        if (linearui_cursor == LinearUIMouseCursor_None || io.MouseDrawCursor)
        {
            if (bd->LastMouseCursor != nullptr)
            {
                // Hide OS mouse cursor if linearui is drawing it or if it wants no cursor
                linearwinSetInputMode(window, LINEARWIN_CURSOR, LINEARWIN_CURSOR_HIDDEN);
                bd->LastMouseCursor = nullptr;
            }
        }
        else
        {
            // Show OS mouse cursor
            // FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with LINEARWIN 3.2, but 3.3 works here.
#if LINEARWIN_HAS_CREATECURSOR
            LINEARWINcursor* cursor = bd->MouseCursors[linearui_cursor] ? bd->MouseCursors[linearui_cursor] : bd->MouseCursors[LinearUIMouseCursor_Arrow];
            if (bd->LastMouseCursor != cursor)
            {
                linearwinSetCursor(window, cursor);
                bd->LastMouseCursor = cursor;
            }
#endif
            linearwinSetInputMode(window, LINEARWIN_CURSOR, LINEARWIN_CURSOR_NORMAL);
        }
    }
}

// Update gamepad inputs
static inline float Saturate(float v) { return v < 0.0f ? 0.0f : v  > 1.0f ? 1.0f : v; }
static void LinearUI_ImplLinearWin_UpdateGamepads()
{
    LinearUIIO& io = LinearUI::GetIO();
    if ((io.ConfigFlags & LinearUIConfigFlags_NavEnableGamepad) == 0) // FIXME: Technically feeding gamepad shouldn't depend on this now that they are regular inputs, but see #8075
        return;

    io.BackendFlags &= ~LinearUIBackendFlags_HasGamepad;
#if LINEARWIN_HAS_GAMEPAD_API && !defined(EMSCRIPTEN_USE_EMBEDDED_LINEARWIN3)
    LINEARWINgamepadstate gamepad;
    if (!linearwinGetGamepadState(LINEARWIN_JOYSTICK_1, &gamepad))
        return;
    #define MAP_BUTTON(KEY_NO, BUTTON_NO, _UNUSED)          do { io.AddKeyEvent(KEY_NO, gamepad.buttons[BUTTON_NO] != 0); } while (0)
    #define MAP_ANALOG(KEY_NO, AXIS_NO, _UNUSED, V0, V1)    do { float v = gamepad.axes[AXIS_NO]; v = (v - V0) / (V1 - V0); io.AddKeyAnalogEvent(KEY_NO, v > 0.10f, Saturate(v)); } while (0)
#else
    int axes_count = 0, buttons_count = 0;
    const float* axes = linearwinGetJoystickAxes(LINEARWIN_JOYSTICK_1, &axes_count);
    const unsigned char* buttons = linearwinGetJoystickButtons(LINEARWIN_JOYSTICK_1, &buttons_count);
    if (axes_count == 0 || buttons_count == 0)
        return;
    #define MAP_BUTTON(KEY_NO, _UNUSED, BUTTON_NO)          do { io.AddKeyEvent(KEY_NO, (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == LINEARWIN_PRESS)); } while (0)
    #define MAP_ANALOG(KEY_NO, _UNUSED, AXIS_NO, V0, V1)    do { float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; v = (v - V0) / (V1 - V0); io.AddKeyAnalogEvent(KEY_NO, v > 0.10f, Saturate(v)); } while (0)
#endif
    io.BackendFlags |= LinearUIBackendFlags_HasGamepad;
    MAP_BUTTON(LinearUIKey_GamepadStart,       LINEARWIN_GAMEPAD_BUTTON_START,          7);
    MAP_BUTTON(LinearUIKey_GamepadBack,        LINEARWIN_GAMEPAD_BUTTON_BACK,           6);
    MAP_BUTTON(LinearUIKey_GamepadFaceLeft,    LINEARWIN_GAMEPAD_BUTTON_X,              2);     // Xbox X, PS Square
    MAP_BUTTON(LinearUIKey_GamepadFaceRight,   LINEARWIN_GAMEPAD_BUTTON_B,              1);     // Xbox B, PS Circle
    MAP_BUTTON(LinearUIKey_GamepadFaceUp,      LINEARWIN_GAMEPAD_BUTTON_Y,              3);     // Xbox Y, PS Triangle
    MAP_BUTTON(LinearUIKey_GamepadFaceDown,    LINEARWIN_GAMEPAD_BUTTON_A,              0);     // Xbox A, PS Cross
    MAP_BUTTON(LinearUIKey_GamepadDpadLeft,    LINEARWIN_GAMEPAD_BUTTON_DPAD_LEFT,      13);
    MAP_BUTTON(LinearUIKey_GamepadDpadRight,   LINEARWIN_GAMEPAD_BUTTON_DPAD_RIGHT,     11);
    MAP_BUTTON(LinearUIKey_GamepadDpadUp,      LINEARWIN_GAMEPAD_BUTTON_DPAD_UP,        10);
    MAP_BUTTON(LinearUIKey_GamepadDpadDown,    LINEARWIN_GAMEPAD_BUTTON_DPAD_DOWN,      12);
    MAP_BUTTON(LinearUIKey_GamepadL1,          LINEARWIN_GAMEPAD_BUTTON_LEFT_BUMPER,    4);
    MAP_BUTTON(LinearUIKey_GamepadR1,          LINEARWIN_GAMEPAD_BUTTON_RIGHT_BUMPER,   5);
    MAP_ANALOG(LinearUIKey_GamepadL2,          LINEARWIN_GAMEPAD_AXIS_LEFT_TRIGGER,     4,      -0.75f,  +1.0f);
    MAP_ANALOG(LinearUIKey_GamepadR2,          LINEARWIN_GAMEPAD_AXIS_RIGHT_TRIGGER,    5,      -0.75f,  +1.0f);
    MAP_BUTTON(LinearUIKey_GamepadL3,          LINEARWIN_GAMEPAD_BUTTON_LEFT_THUMB,     8);
    MAP_BUTTON(LinearUIKey_GamepadR3,          LINEARWIN_GAMEPAD_BUTTON_RIGHT_THUMB,    9);
    MAP_ANALOG(LinearUIKey_GamepadLStickLeft,  LINEARWIN_GAMEPAD_AXIS_LEFT_X,           0,      -0.25f,  -1.0f);
    MAP_ANALOG(LinearUIKey_GamepadLStickRight, LINEARWIN_GAMEPAD_AXIS_LEFT_X,           0,      +0.25f,  +1.0f);
    MAP_ANALOG(LinearUIKey_GamepadLStickUp,    LINEARWIN_GAMEPAD_AXIS_LEFT_Y,           1,      -0.25f,  -1.0f);
    MAP_ANALOG(LinearUIKey_GamepadLStickDown,  LINEARWIN_GAMEPAD_AXIS_LEFT_Y,           1,      +0.25f,  +1.0f);
    MAP_ANALOG(LinearUIKey_GamepadRStickLeft,  LINEARWIN_GAMEPAD_AXIS_RIGHT_X,          2,      -0.25f,  -1.0f);
    MAP_ANALOG(LinearUIKey_GamepadRStickRight, LINEARWIN_GAMEPAD_AXIS_RIGHT_X,          2,      +0.25f,  +1.0f);
    MAP_ANALOG(LinearUIKey_GamepadRStickUp,    LINEARWIN_GAMEPAD_AXIS_RIGHT_Y,          3,      -0.25f,  -1.0f);
    MAP_ANALOG(LinearUIKey_GamepadRStickDown,  LINEARWIN_GAMEPAD_AXIS_RIGHT_Y,          3,      +0.25f,  +1.0f);
    #undef MAP_BUTTON
    #undef MAP_ANALOG
}

static void LinearUI_ImplLinearWin_UpdateMonitors()
{
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
    int monitors_count = 0;
    LINEARWINmonitor** linearwin_monitors = linearwinGetMonitors(&monitors_count);

    bool updated_monitors = false;
    for (int n = 0; n < monitors_count; n++)
    {
        LinearUIPlatformMonitor monitor;
        int x, y;
        linearwinGetMonitorPos(linearwin_monitors[n], &x, &y);
        const LINEARWINvidmode* vid_mode = linearwinGetVideoMode(linearwin_monitors[n]);
        if (vid_mode == nullptr)
            continue; // Failed to get Video mode (e.g. Emscripten does not support this function)
        if (vid_mode->width <= 0 || vid_mode->height <= 0)
            continue; // Failed to query suitable monitor info (#9195)
        monitor.MainPos = monitor.WorkPos = LinearVec2((float)x, (float)y);
        monitor.MainSize = monitor.WorkSize = LinearVec2((float)vid_mode->width, (float)vid_mode->height);
#if LINEARWIN_HAS_MONITOR_WORK_AREA
        int w, h;
        linearwinGetMonitorWorkarea(linearwin_monitors[n], &x, &y, &w, &h);
        if (w > 0 && h > 0) // Workaround a small LINEARWIN issue reporting zero on monitor changes: https://github.com/linearwin/linearwin/pull/1761
        {
            monitor.WorkPos = LinearVec2((float)x, (float)y);
            monitor.WorkSize = LinearVec2((float)w, (float)h);
        }
#endif
        float scale = LinearUI_ImplLinearWin_GetContentScaleForMonitor(linearwin_monitors[n]);
        if (scale == 0.0f)
            continue; // Some accessibility applications are declaring virtual monitors with a DPI of 0 (#7902)
        monitor.DpiScale = scale;
        monitor.PlatformHandle = (void*)linearwin_monitors[n]; // [...] LINEARWIN doc states: "guaranteed to be valid only until the monitor configuration changes"

        // Preserve existing monitor list until a valid one is added.
        // Happens on macOS sleeping (#5683) and seemingly occasionally on Windows (#9195)
        if (!updated_monitors)
            platform_io.Monitors.resize(0);
        updated_monitors = true;
        platform_io.Monitors.push_back(monitor);
    }
}

// For GFLW 3.2 + Windows: include a simplified non-monitor aware version of LinearUI_ImplWin32_GetDpiScaleForMonitor().
// This is merely a band-aid to make using LINEARWIN 3.2 a little bit nicer, but prefer to use LINEARWIN 3.3+ or the full correct functions from the Win32 backend.
#if !LINEARWIN_HAS_PER_MONITOR_DPI && defined(_WIN32) && !defined(NOGDI)
static float   LinearUI_ImplWin32_GetLegacyDpiScale()   { const HDC dc = ::GetDC(nullptr); UINT xdpi = ::GetDeviceCaps(dc, LOGPIXELSX); ::ReleaseDC(nullptr, dc); return (float)xdpi / 96.0f; }
static void    linearwinGetWindowContentScale(LINEARWINwindow*, float* x_scale, float* y_scale)   { *x_scale = *y_scale = LinearUI_ImplWin32_GetLegacyDpiScale(); }
static void    linearwinGetMonitorContentScale(LINEARWINmonitor*, float* x_scale, float* y_scale) { *x_scale = *y_scale = LinearUI_ImplWin32_GetLegacyDpiScale(); }
#undef LINEARWIN_HAS_PER_MONITOR_DPI
#define LINEARWIN_HAS_PER_MONITOR_DPI 1
#endif

// - On Windows the process needs to be marked DPI-aware!! SDL2 doesn't do it by default. You can call ::SetProcessDPIAware() or call LinearUI_ImplWin32_EnableDpiAwareness() from Win32 backend.
// - Apple platforms use FramebufferScale so we always return 1.0f.
// - Some accessibility applications are declaring virtual monitors with a DPI of 0.0f, see #7902. We preserve this value for caller to handle.
float LinearUI_ImplLinearWin_GetContentScaleForWindow(LINEARWINwindow* window)
{
#if LINEARWIN_HAS_WAYLAND
    if (LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData(window))
        if (bd->IsWayland)
            return 1.0f;
#endif
#if LINEARWIN_HAS_PER_MONITOR_DPI && !(defined(__APPLE__) || defined(__EMSCRIPTEN__) || defined(__ANDROID__))
    float x_scale, y_scale;
    linearwinGetWindowContentScale(window, &x_scale, &y_scale);
    return x_scale;
#else
    LINEARUI_UNUSED(window);
    return 1.0f;
#endif
}

float LinearUI_ImplLinearWin_GetContentScaleForMonitor(LINEARWINmonitor* monitor)
{
#if LINEARWIN_HAS_WAYLAND
    if (LinearUI_ImplLinearWin_IsWayland()) // We can't access our bd->IsWayland cache for a monitor.
        return 1.0f;
#endif
#if LINEARWIN_HAS_PER_MONITOR_DPI && !(defined(__APPLE__) || defined(__EMSCRIPTEN__) || defined(__ANDROID__))
    float x_scale, y_scale;
    linearwinGetMonitorContentScale(monitor, &x_scale, &y_scale);
    return x_scale;
#else
    LINEARUI_UNUSED(monitor);
    return 1.0f;
#endif
}

static void LinearUI_ImplLinearWin_GetWindowSizeAndFramebufferScale(LINEARWINwindow* window, LinearVec2* out_size, LinearVec2* out_framebuffer_scale)
{
    int w, h;
    int display_w, display_h;
    linearwinGetWindowSize(window, &w, &h);
    linearwinGetFramebufferSize(window, &display_w, &display_h);
    float fb_scale_x = (w > 0) ? (float)display_w / (float)w : 1.0f;
    float fb_scale_y = (h > 0) ? (float)display_h / (float)h : 1.0f;
#if LINEARWIN_HAS_WAYLAND
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData(window);
    if (!bd->IsWayland)
        fb_scale_x = fb_scale_y = 1.0f;
#endif
    if (out_size != nullptr)
        *out_size = LinearVec2((float)w, (float)h);
    if (out_framebuffer_scale != nullptr)
        *out_framebuffer_scale = LinearVec2(fb_scale_x, fb_scale_y);
}

void LinearUI_ImplLinearWin_NewFrame()
{
    LinearUIIO& io = LinearUI::GetIO();
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData();
    LINEARUI_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call LinearUI_ImplLinearWin_InitForXXX()?");

    // Setup main viewport size (every frame to accommodate for window resizing)
    LinearUI_ImplLinearWin_GetWindowSizeAndFramebufferScale(bd->Window, &io.DisplaySize, &io.DisplayFramebufferScale);
    LinearUI_ImplLinearWin_UpdateMonitors();

    // Setup time step
    // (Accept linearwinGetTime() not returning a monotonically increasing value. Seems to happens on disconnecting peripherals and probably on VMs and Emscripten, see #6491, #6189, #6114, #3644)
    double current_time = linearwinGetTime();
    if (current_time <= bd->Time)
        current_time = bd->Time + 0.00001;
    io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
    bd->Time = current_time;

    bd->MouseIgnoreButtonUp = false;
    LinearUI_ImplLinearWin_UpdateMouseData();
    LinearUI_ImplLinearWin_UpdateMouseCursor();

    // Update game controllers (if enabled and available)
    LinearUI_ImplLinearWin_UpdateGamepads();
}

// LINEARWIN doesn't provide a portable sleep function
void LinearUI_ImplLinearWin_Sleep(int milliseconds)
{
#ifdef _WIN32
    ::Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

#ifdef EMSCRIPTEN_USE_EMBEDDED_LINEARWIN3
static EM_BOOL LinearUI_ImplLinearWin_OnCanvasSizeChange(int event_type, const EmscriptenUiEvent* event, void* user_data)
{
    LinearUI_ImplLinearWin_Data* bd = (LinearUI_ImplLinearWin_Data*)user_data;
    double canvas_width, canvas_height;
    emscripten_get_element_css_size(bd->CanvasSelector, &canvas_width, &canvas_height);
    linearwinSetWindowSize(bd->Window, (int)canvas_width, (int)canvas_height);
    return true;
}

static EM_BOOL LinearUI_ImplEmscripten_FullscreenChangeCallback(int event_type, const EmscriptenFullscreenChangeEvent* event, void* user_data)
{
    LinearUI_ImplLinearWin_Data* bd = (LinearUI_ImplLinearWin_Data*)user_data;
    double canvas_width, canvas_height;
    emscripten_get_element_css_size(bd->CanvasSelector, &canvas_width, &canvas_height);
    linearwinSetWindowSize(bd->Window, (int)canvas_width, (int)canvas_height);
    return true;
}

// 'canvas_selector' is a CSS selector. The event listener is applied to the first element that matches the query.
// STRING MUST PERSIST FOR THE APPLICATION DURATION. PLEASE USE A STRING LITERAL OR ENSURE POINTER WILL STAY VALID.
void LinearUI_ImplLinearWin_InstallEmscriptenCallbacks(LINEARWINwindow*, const char* canvas_selector)
{
    LINEARUI_ASSERT(canvas_selector != nullptr);
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData();
    LINEARUI_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call LinearUI_ImplLinearWin_InitForXXX()?");

    bd->CanvasSelector = canvas_selector;
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, bd, false, LinearUI_ImplLinearWin_OnCanvasSizeChange);
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, bd, false, LinearUI_ImplEmscripten_FullscreenChangeCallback);

    // Change the size of the LINEARWIN window according to the size of the canvas
    LinearUI_ImplLinearWin_OnCanvasSizeChange(EMSCRIPTEN_EVENT_RESIZE, {}, bd);

    // Register Emscripten Wheel callback to workaround issue in Emscripten LINEARWIN Emulation (#6096)
    // We intentionally do not check 'if (install_callbacks)' here, as some users may set it to false and call LINEARWIN callback themselves.
    // FIXME: May break chaining in case user registered their own Emscripten callback?
    emscripten_set_wheel_callback(bd->CanvasSelector, bd, false, LinearUI_ImplEmscripten_WheelCallback);
}
#elif defined(EMSCRIPTEN_USE_PORT_CONTRIB_LINEARWIN3)
// When using --use-port=contrib.linearwin for the LINEARWIN implementation, you can override the behavior of this call
// by invoking emscripten_linearwin_make_canvas_resizable afterward.
// See https://github.com/pongasoft/emscripten-linearwin/blob/master/docs/Usage.md#how-to-make-the-canvas-resizable-by-the-user for an explanation
void LinearUI_ImplLinearWin_InstallEmscriptenCallbacks(LINEARWINwindow* window, const char* canvas_selector)
{
  LINEARWINwindow* w = (LINEARWINwindow*)(EM_ASM_INT({ return Module.linearwinGetWindow(UTF8ToString($0)); }, canvas_selector));
  LINEARUI_ASSERT(window == w); // Sanity check
  LINEARUI_UNUSED(w);
  emscripten_linearwin_make_canvas_resizable(window, "window", nullptr);
}
#endif // #ifdef EMSCRIPTEN_USE_PORT_CONTRIB_LINEARWIN3


//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously.
// If you are new to dear linearui or creating a new binding for dear linearui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

// Helper structure we store in the void* PlatformUserData field of each LinearUIViewport to easily retrieve our backend data.
struct LinearUI_ImplLinearWin_ViewportData
{
    LINEARWINwindow* Window;             // Stored in LinearUIViewport::PlatformHandle
    bool        WindowOwned;
    int         IgnoreWindowPosEventFrame;
    int         IgnoreWindowSizeEventFrame;
#ifdef _WIN32
    WNDPROC     PrevWndProc;
#endif

    LinearUI_ImplLinearWin_ViewportData()  { memset((void*)this, 0, sizeof(*this)); IgnoreWindowSizeEventFrame = IgnoreWindowPosEventFrame = -1; }
    ~LinearUI_ImplLinearWin_ViewportData() { LINEARUI_ASSERT(Window == nullptr); }
};

static void LinearUI_ImplLinearWin_WindowCloseCallback(LINEARWINwindow* window)
{
    if (LinearUIViewport* viewport = LinearUI::FindViewportByPlatformHandle(window))
        viewport->PlatformRequestClose = true;
}

// LINEARWIN may dispatch window pos/size events after calling linearwinSetWindowPos()/linearwinSetWindowSize().
// However: depending on the platform the callback may be invoked at different time:
// - on Windows it appears to be called within the linearwinSetWindowPos()/linearwinSetWindowSize() call
// - on Linux it is queued and invoked during linearwinPollEvents()
// Because the event doesn't always fire on linearwinSetWindowXXX() we use a frame counter tag to only
// ignore recent linearwinSetWindowXXX() calls.
static void LinearUI_ImplLinearWin_WindowPosCallback(LINEARWINwindow* window, int, int)
{
    if (LinearUIViewport* viewport = LinearUI::FindViewportByPlatformHandle(window))
    {
        if (LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData)
        {
            bool ignore_event = (LinearUI::GetFrameCount() <= vd->IgnoreWindowPosEventFrame + 1);
            //data->IgnoreWindowPosEventFrame = -1;
            if (ignore_event)
                return;
        }
        viewport->PlatformRequestMove = true;
    }
}

static void LinearUI_ImplLinearWin_WindowSizeCallback(LINEARWINwindow* window, int, int)
{
    if (LinearUIViewport* viewport = LinearUI::FindViewportByPlatformHandle(window))
    {
        if (LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData)
        {
            bool ignore_event = (LinearUI::GetFrameCount() <= vd->IgnoreWindowSizeEventFrame + 1);
            //data->IgnoreWindowSizeEventFrame = -1;
            if (ignore_event)
                return;
        }
        viewport->PlatformRequestResize = true;
    }
}

#if !defined(__APPLE__) && !defined(_WIN32) && !defined(__EMSCRIPTEN__) && LINEARWIN_HAS_GETPLATFORM
#define LINEARUI_LINEARWIN_HAS_SETWINDOWFLOATING
static void LinearUI_ImplLinearWin_SetWindowFloating(LinearUI_ImplLinearWin_Data* bd, LINEARWINwindow* window)
{
#ifdef LINEARWIN_EXPOSE_NATIVE_X11
    if (linearwinGetPlatform() == LINEARWIN_PLATFORM_X11)
    {
        Display* display = linearwinGetX11Display();
        Window xwindow = linearwinGetX11Window(window);
        Atom wm_type = bd->XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
        Atom wm_type_dialog = bd->XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
        bd->XChangeProperty(display, xwindow, wm_type, XA_ATOM, 32, PropModeReplace, (unsigned char*)&wm_type_dialog, 1);
        XSetWindowAttributes attrs;
        attrs.override_redirect = False;
        bd->XChangeWindowAttributes(display, xwindow, CWOverrideRedirect, &attrs);
        bd->XFlush(display);
    }
#endif // LINEARWIN_EXPOSE_NATIVE_X11
#ifdef LINEARWIN_EXPOSE_NATIVE_WAYLAND
    // FIXME: Help needed, see #8884, #8474 for discussions about this.
#endif // LINEARWIN_EXPOSE_NATIVE_X11
}
#endif // LINEARUI_LINEARWIN_HAS_SETWINDOWFLOATING

static void LinearUI_ImplLinearWin_CreateWindow(LinearUIViewport* viewport)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData();
    LinearUI_ImplLinearWin_ViewportData* vd = LINEARUI_NEW(LinearUI_ImplLinearWin_ViewportData)();
    viewport->PlatformUserData = vd;

    // Workaround for Linux: ignore mouse up events corresponding to losing focus of the previously focused window (#7733, #3158, #7922)
#ifdef __linux__
    bd->MouseIgnoreButtonUpWaitForFocusLoss = true;
#endif

    // LINEARWIN 3.2 unfortunately always set focus on linearwinCreateWindow() if LINEARWIN_VISIBLE is set, regardless of LINEARWIN_FOCUSED
    // With LINEARWIN 3.3, the hint LINEARWIN_FOCUS_ON_SHOW fixes this problem
    linearwinWindowHint(LINEARWIN_VISIBLE, false);
    linearwinWindowHint(LINEARWIN_FOCUSED, false);
#if LINEARWIN_HAS_FOCUS_ON_SHOW
    linearwinWindowHint(LINEARWIN_FOCUS_ON_SHOW, false);
#endif
    linearwinWindowHint(LINEARWIN_DECORATED, (viewport->Flags & LinearUIViewportFlags_NoDecoration) ? false : true);
#if LINEARWIN_HAS_WINDOW_TOPMOST
    linearwinWindowHint(LINEARWIN_FLOATING, (viewport->Flags & LinearUIViewportFlags_TopMost) ? true : false);
#endif
    LINEARWINwindow* share_window = (bd->ClientApi == LinearWinClientApi_OpenGL) ? bd->Window : nullptr;
    vd->Window = linearwinCreateWindow((int)viewport->Size.x, (int)viewport->Size.y, "No Title Yet", nullptr, share_window);
    vd->WindowOwned = true;
    LinearUI_ImplLinearWin_ContextMap_Add(vd->Window, bd->Context);
    viewport->PlatformHandle = (void*)vd->Window;
#ifdef LINEARUI_LINEARWIN_HAS_SETWINDOWFLOATING
    LinearUI_ImplLinearWin_SetWindowFloating(bd, vd->Window);
#endif
#ifdef _WIN32
    viewport->PlatformHandleRaw = linearwinGetWin32Window(vd->Window);
    ::SetPropA((HWND)viewport->PlatformHandleRaw, "LINEARUI_BACKEND_DATA", bd);
#elif defined(__APPLE__)
    viewport->PlatformHandleRaw = (void*)linearwinGetCocoaWindow(vd->Window);
#endif
    linearwinSetWindowPos(vd->Window, (int)viewport->Pos.x, (int)viewport->Pos.y);

    // Install LINEARWIN callbacks for secondary viewports
    linearwinSetWindowFocusCallback(vd->Window, LinearUI_ImplLinearWin_WindowFocusCallback);
    linearwinSetCursorEnterCallback(vd->Window, LinearUI_ImplLinearWin_CursorEnterCallback);
    linearwinSetCursorPosCallback(vd->Window, LinearUI_ImplLinearWin_CursorPosCallback);
    linearwinSetMouseButtonCallback(vd->Window, LinearUI_ImplLinearWin_MouseButtonCallback);
    linearwinSetScrollCallback(vd->Window, LinearUI_ImplLinearWin_ScrollCallback);
    linearwinSetKeyCallback(vd->Window, LinearUI_ImplLinearWin_KeyCallback);
    linearwinSetCharCallback(vd->Window, LinearUI_ImplLinearWin_CharCallback);
    linearwinSetWindowCloseCallback(vd->Window, LinearUI_ImplLinearWin_WindowCloseCallback);
    linearwinSetWindowPosCallback(vd->Window, LinearUI_ImplLinearWin_WindowPosCallback);
    linearwinSetWindowSizeCallback(vd->Window, LinearUI_ImplLinearWin_WindowSizeCallback);
    if (bd->ClientApi == LinearWinClientApi_OpenGL)
    {
        linearwinMakeContextCurrent(vd->Window);
        linearwinSwapInterval(0);
    }
}

static void LinearUI_ImplLinearWin_DestroyWindow(LinearUIViewport* viewport)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData();
    if (LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData)
    {
        if (vd->WindowOwned)
        {
#if !LINEARWIN_HAS_MOUSE_PASSTHROUGH && LINEARWIN_HAS_WINDOW_HOVERED && defined(_WIN32)
            HWND hwnd = (HWND)viewport->PlatformHandleRaw;
            ::RemovePropA(hwnd, "LINEARUI_VIEWPORT");
#endif

            // Release any keys that were pressed in the window being destroyed and are still held down,
            // because we will not receive any release events after window is destroyed.
            for (int i = 0; i < LINEARUI_COUNTOF(bd->KeyOwnerWindows); i++)
                if (bd->KeyOwnerWindows[i] == vd->Window)
                    LinearUI_ImplLinearWin_KeyCallback(vd->Window, i, 0, LINEARWIN_RELEASE, 0); // Later params are only used for main viewport, on which this function is never called.

            LinearUI_ImplLinearWin_ContextMap_Remove(vd->Window);
            linearwinDestroyWindow(vd->Window);
        }
        vd->Window = nullptr;
        LINEARUI_DELETE(vd);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

static void LinearUI_ImplLinearWin_ShowWindow(LinearUIViewport* viewport)
{
    LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData;

#if defined(_WIN32)
    // LINEARWIN hack: Hide icon from task bar
    HWND hwnd = (HWND)viewport->PlatformHandleRaw;
    if (viewport->Flags & LinearUIViewportFlags_NoTaskBarIcon)
    {
        LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        ex_style &= ~WS_EX_APPWINDOW;
        ex_style |= WS_EX_TOOLWINDOW;
        ::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
    }

    // LINEARWIN hack: install WndProc for mouse source event and WM_NCHITTEST message handler.
    ::SetPropA(hwnd, "LINEARUI_VIEWPORT", viewport);
    vd->PrevWndProc = (WNDPROC)::GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    ::SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)LinearUI_ImplLinearWin_WndProc);

#if !LINEARWIN_HAS_FOCUS_ON_SHOW
    // LINEARWIN hack: LINEARWIN 3.2 has a bug where linearwinShowWindow() also activates/focus the window.
    // The fix was pushed to LINEARWIN repository on 2018/01/09 and should be included in LINEARWIN 3.3 via a LINEARWIN_FOCUS_ON_SHOW window attribute.
    // See https://github.com/linearwin/linearwin/issues/1189
    // FIXME-VIEWPORT: Implement same work-around for Linux/OSX in the meanwhile.
    if (viewport->Flags & LinearUIViewportFlags_NoFocusOnAppearing)
    {
        ::ShowWindow(hwnd, SW_SHOWNA);
        return;
    }
#endif
#endif

    linearwinShowWindow(vd->Window);
}

static LinearVec2 LinearUI_ImplLinearWin_GetWindowPos(LinearUIViewport* viewport)
{
    LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData;
    int x = 0, y = 0;
    linearwinGetWindowPos(vd->Window, &x, &y);
    return LinearVec2((float)x, (float)y);
}

static void LinearUI_ImplLinearWin_SetWindowPos(LinearUIViewport* viewport, LinearVec2 pos)
{
    LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData;
    vd->IgnoreWindowPosEventFrame = LinearUI::GetFrameCount();
    linearwinSetWindowPos(vd->Window, (int)pos.x, (int)pos.y);
}

static LinearVec2 LinearUI_ImplLinearWin_GetWindowSize(LinearUIViewport* viewport)
{
    LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData;
    int w = 0, h = 0;
    linearwinGetWindowSize(vd->Window, &w, &h);
    return LinearVec2((float)w, (float)h);
}

static void LinearUI_ImplLinearWin_SetWindowSize(LinearUIViewport* viewport, LinearVec2 size)
{
    LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData;
#if defined(__APPLE__) && !LINEARWIN_HAS_OSX_WINDOW_POS_FIX
    // Native OS windows are positioned from the bottom-left corner on macOS, whereas on other platforms they are
    // positioned from the upper-left corner. LINEARWIN makes an effort to convert macOS style coordinates, however it
    // doesn't handle it when changing size. We are manually moving the window in order for changes of size to be based
    // on the upper-left corner.
    int x, y, width, height;
    linearwinGetWindowPos(vd->Window, &x, &y);
    linearwinGetWindowSize(vd->Window, &width, &height);
    linearwinSetWindowPos(vd->Window, x, y - height + size.y);
#endif
    vd->IgnoreWindowSizeEventFrame = LinearUI::GetFrameCount();
    linearwinSetWindowSize(vd->Window, (int)size.x, (int)size.y);
}

static LinearVec2 LinearUI_ImplLinearWin_GetWindowFramebufferScale(LinearUIViewport* viewport)
{
    LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData;
    LinearVec2 framebuffer_scale;
    LinearUI_ImplLinearWin_GetWindowSizeAndFramebufferScale(vd->Window, nullptr, &framebuffer_scale);
    return framebuffer_scale;
}

static void LinearUI_ImplLinearWin_SetWindowTitle(LinearUIViewport* viewport, const char* title)
{
    LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData;
    linearwinSetWindowTitle(vd->Window, title);
}

static void LinearUI_ImplLinearWin_SetWindowFocus(LinearUIViewport* viewport)
{
#if LINEARWIN_HAS_FOCUS_WINDOW
    LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData;
    linearwinFocusWindow(vd->Window);
#else
    // FIXME: What are the effect of not having this function? At the moment linearui doesn't actually call SetWindowFocus - we set that up ahead, will answer that question later.
    (void)viewport;
#endif
}

static bool LinearUI_ImplLinearWin_GetWindowFocus(LinearUIViewport* viewport)
{
    LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData;
    return linearwinGetWindowAttrib(vd->Window, LINEARWIN_FOCUSED) != 0;
}

static bool LinearUI_ImplLinearWin_GetWindowMinimized(LinearUIViewport* viewport)
{
    LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData;
    return linearwinGetWindowAttrib(vd->Window, LINEARWIN_ICONIFIED) != 0;
}

#if LINEARWIN_HAS_WINDOW_ALPHA
static void LinearUI_ImplLinearWin_SetWindowAlpha(LinearUIViewport* viewport, float alpha)
{
    LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData;
    linearwinSetWindowOpacity(vd->Window, alpha);
}
#endif

static void LinearUI_ImplLinearWin_RenderWindow(LinearUIViewport* viewport, void*)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData();
    LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData;
    if (bd->ClientApi == LinearWinClientApi_OpenGL)
        linearwinMakeContextCurrent(vd->Window);
}

static void LinearUI_ImplLinearWin_SwapBuffers(LinearUIViewport* viewport, void*)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData();
    LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData;
    if (bd->ClientApi == LinearWinClientApi_OpenGL)
    {
        linearwinMakeContextCurrent(vd->Window);
        linearwinSwapBuffers(vd->Window);
    }
}

//--------------------------------------------------------------------------------------------------------
// Vulkan support (the Vulkan renderer needs to call a platform-side support function to create the surface)
//--------------------------------------------------------------------------------------------------------

// Avoid including <vulkan.h> so we can build without it
#if LINEARWIN_HAS_VULKAN
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
extern "C" { extern LINEARWINAPI VkResult linearwinCreateWindowSurface(VkInstance instance, LINEARWINwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface); }
static int LinearUI_ImplLinearWin_CreateVkSurface(LinearUIViewport* viewport, LinearU64 vk_instance, const void* vk_allocator, LinearU64* out_vk_surface)
{
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData();
    LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData;
    LINEARUI_UNUSED(bd);
    LINEARUI_ASSERT(bd->ClientApi == LinearWinClientApi_Vulkan);
    VkResult err = linearwinCreateWindowSurface((VkInstance)vk_instance, vd->Window, (const VkAllocationCallbacks*)vk_allocator, (VkSurfaceKHR*)out_vk_surface);
    return (int)err;
}
#endif // LINEARWIN_HAS_VULKAN

static void LinearUI_ImplLinearWin_InitMultiViewportSupport()
{
    // Register platform interface (will be coupled with a renderer interface)
    LinearUI_ImplLinearWin_Data* bd = LinearUI_ImplLinearWin_GetBackendData();
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
    platform_io.Platform_CreateWindow = LinearUI_ImplLinearWin_CreateWindow;
    platform_io.Platform_DestroyWindow = LinearUI_ImplLinearWin_DestroyWindow;
    platform_io.Platform_ShowWindow = LinearUI_ImplLinearWin_ShowWindow;
    platform_io.Platform_SetWindowPos = LinearUI_ImplLinearWin_SetWindowPos;
    platform_io.Platform_GetWindowPos = LinearUI_ImplLinearWin_GetWindowPos;
    platform_io.Platform_SetWindowSize = LinearUI_ImplLinearWin_SetWindowSize;
    platform_io.Platform_GetWindowSize = LinearUI_ImplLinearWin_GetWindowSize;
    platform_io.Platform_GetWindowFramebufferScale = LinearUI_ImplLinearWin_GetWindowFramebufferScale;
    platform_io.Platform_SetWindowFocus = LinearUI_ImplLinearWin_SetWindowFocus;
    platform_io.Platform_GetWindowFocus = LinearUI_ImplLinearWin_GetWindowFocus;
    platform_io.Platform_GetWindowMinimized = LinearUI_ImplLinearWin_GetWindowMinimized;
    platform_io.Platform_SetWindowTitle = LinearUI_ImplLinearWin_SetWindowTitle;
    platform_io.Platform_RenderWindow = LinearUI_ImplLinearWin_RenderWindow;
    platform_io.Platform_SwapBuffers = LinearUI_ImplLinearWin_SwapBuffers;
#if LINEARWIN_HAS_WINDOW_ALPHA
    platform_io.Platform_SetWindowAlpha = LinearUI_ImplLinearWin_SetWindowAlpha;
#endif
#if LINEARWIN_HAS_VULKAN
    platform_io.Platform_CreateVkSurface = LinearUI_ImplLinearWin_CreateVkSurface;
#endif

    // Register main window handle (which is owned by the main application, not by us)
    // This is mostly for simplicity and consistency, so that our code (e.g. mouse handling etc.) can use same logic for main and secondary viewports.
    LinearUIViewport* main_viewport = LinearUI::GetMainViewport();
    LinearUI_ImplLinearWin_ViewportData* vd = LINEARUI_NEW(LinearUI_ImplLinearWin_ViewportData)();
    vd->Window = bd->Window;
    vd->WindowOwned = false;
    main_viewport->PlatformUserData = vd;
    main_viewport->PlatformHandle = (void*)bd->Window;
}

static void LinearUI_ImplLinearWin_ShutdownMultiViewportSupport()
{
    LinearUI::DestroyPlatformWindows();
}

//-----------------------------------------------------------------------------

// WndProc hook (declared here because we will need access to LinearUI_ImplLinearWin_ViewportData)
#ifdef _WIN32
static LinearUIMouseSource GetMouseSourceFromMessageExtraInfo()
{
    LPARAM extra_info = ::GetMessageExtraInfo();
    if ((extra_info & 0xFFFFFF80) == 0xFF515700)
        return LinearUIMouseSource_Pen;
    if ((extra_info & 0xFFFFFF80) == 0xFF515780)
        return LinearUIMouseSource_TouchScreen;
    return LinearUIMouseSource_Mouse;
}
static LRESULT CALLBACK LinearUI_ImplLinearWin_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LinearUI_ImplLinearWin_Data* bd = (LinearUI_ImplLinearWin_Data*)::GetPropA(hWnd, "LINEARUI_BACKEND_DATA");
    LinearUIIO& io = LinearUI::GetIO(bd->Context);

    WNDPROC prev_wndproc = bd->PrevWndProc;
    LinearUIViewport* viewport = (LinearUIViewport*)::GetPropA(hWnd, "LINEARUI_VIEWPORT");
    if (viewport != NULL)
        if (LinearUI_ImplLinearWin_ViewportData* vd = (LinearUI_ImplLinearWin_ViewportData*)viewport->PlatformUserData)
            prev_wndproc = vd->PrevWndProc;

    switch (msg)
    {
        // LINEARWIN doesn't allow to distinguish Mouse vs TouchScreen vs Pen.
        // Add support for Win32 (based on linearui_impl_win32), because we rely on _TouchScreen info to trickle inputs differently.
    case WM_MOUSEMOVE: case WM_NCMOUSEMOVE:
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK: case WM_LBUTTONUP:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK: case WM_RBUTTONUP:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK: case WM_MBUTTONUP:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK: case WM_XBUTTONUP:
        io.AddMouseSourceEvent(GetMouseSourceFromMessageExtraInfo());
        break;

        // We have submitted https://github.com/linearwin/linearwin/pull/1568 to allow LINEARWIN to support "transparent inputs".
        // In the meanwhile we implement custom per-platform workarounds here (FIXME-VIEWPORT: Implement same work-around for Linux/OSX!)
#if !LINEARWIN_HAS_MOUSE_PASSTHROUGH && LINEARWIN_HAS_WINDOW_HOVERED
    case WM_NCHITTEST:
    {
        // Let mouse pass-through the window. This will allow the backend to call io.AddMouseViewportEvent() properly (which is OPTIONAL).
        // The LinearUIViewportFlags_NoInputs flag is set while dragging a viewport, as want to detect the window behind the one we are dragging.
        // If you cannot easily access those viewport flags from your windowing/event code: you may manually synchronize its state e.g. in
        // your main loop after calling UpdatePlatformWindows(). Iterate all viewports/platform windows and pass the flag to your windowing system.
        if (viewport && (viewport->Flags & LinearUIViewportFlags_NoInputs))
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

#endif // #ifndef LINEARUI_DISABLE
