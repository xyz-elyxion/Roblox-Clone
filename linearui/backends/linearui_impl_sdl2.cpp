// dear linearui: Platform Backend for SDL2
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3, Vulkan..)
// (Info: SDL2 is a cross-platform general purpose library for handling windows, inputs, graphics context creation, etc.)
// (Prefer SDL 2.0.5+ for full feature support.)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass LinearUIKey values to all key functions e.g. LinearUI::IsKeyPressed(LinearUIKey_Space). [Legacy SDL_SCANCODE_* values are obsolete since 1.87 and not supported since 1.91.5]
//  [X] Platform: Gamepad support.
//  [X] Platform: Mouse cursor shape and visibility (LinearUIBackendFlags_HasMouseCursors). Disable with 'io.ConfigFlags |= LinearUIConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Basic IME support. App needs to call 'SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");' before SDL_CreateWindow()!.
//  [X] Platform: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= LinearUIConfigFlags_ViewportsEnable'.
// Missing features or Issues:
//  [ ] Platform: Multi-viewport: Minimized windows seems to break mouse wheel events (at least under Windows).
//  [ ] Platform: Multi-viewport: Missing LinearUIBackendFlags_HasParentViewport support. The viewport->ParentViewportID field is ignored, and therefore io.ConfigViewportsNoDefaultParent has no effect either.

// You can use unmodified linearui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire linearui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear LinearUI:
// - FAQ                  https://dearlinearui.com/faq
// - Getting Started      https://dearlinearui.com/getting-started
// - Documentation        https://dearlinearui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of linearui.cpp

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2026-XX-XX: Platform: Added support for multiple windows via the LinearUIPlatformIO interface.
//  2026-04-16: Made LinearUI_ImplSDL2_GetContentScaleForWindow(), LinearUI_ImplSDL2_GetContentScaleForDisplay() helpers return a minimum of 1.0f, as some Linux setup seems to report <1.0f value and this breaks scaling border size. (#9369)
//  2026-03-09: [Docking] Fixed an issue dated 2025/04/09 (1.92 WIP) where a refactor+merge caused LinearUIBackendFlags_HasMouseHoveredViewport to never be set, causing foreign windows to be ignored when deciding of hovered viewport. (#9284)
//  2026-02-13: Inputs: systems other than X11 are back to starting mouse capture on mouse down (reverts 2025-02-26 change). Only X11 requires waiting for a drag by default (not ideal, but a better default for X11 users). Added LinearUI_ImplSDL2_SetMouseCaptureMode() for X11 debugger users. (#3650, #6410, #9235)
//  2026-01-15: Changed GetClipboardText() handler to return nullptr on error aka clipboard contents is not text. Consistent with other backends. (#9168)
//  2025-09-24: Skip using the SDL_GetGlobalMouseState() state when one of our window is hovered, as the SDL_MOUSEMOTION data is reliable. Fix macOS notch mouse coordinates issue in fullscreen mode + better perf on X11. (#7919, #7786)
//  2025-09-18: Call platform_io.ClearPlatformHandlers() on shutdown.
//  2025-09-15: Content Scales are always reported as 1.0 on Wayland. (#8921)
//  2025-07-08: Made LinearUI_ImplSDL2_GetContentScaleForWindow(), LinearUI_ImplSDL2_GetContentScaleForDisplay() helpers return 1.0f on Emscripten and Android platforms, matching macOS logic. (#8742, #8733)
//  2025-06-11: Added LinearUI_ImplSDL2_GetContentScaleForWindow(SDL_Window* window) and LinearUI_ImplSDL2_GetContentScaleForDisplay(int display_index) helper to facilitate making DPI-aware apps.
//  2025-05-15: [Docking] Add Platform_GetWindowFramebufferScale() handler, to allow varying Retina display density on multiple monitors.
//  2025-04-09: [Docking] Revert update monitors and work areas information every frame. Only do it on Windows. (#8415, #8558)
//  2025-04-09: Don't attempt to call SDL_CaptureMouse() on drivers where we don't call SDL_GetGlobalMouseState(). (#8561)
//  2025-03-21: Fill gamepad inputs and set LinearUIBackendFlags_HasGamepad regardless of LinearUIConfigFlags_NavEnableGamepad being set.
//  2025-03-10: When dealing with OEM keys, use scancodes instead of translated keycodes to choose LinearUIKey values. (#7136, #7201, #7206, #7306, #7670, #7672, #8468)
//  2025-02-26: Only start SDL_CaptureMouse() when mouse is being dragged, to mitigate issues with e.g. Linux debuggers not claiming capture back. (#6410, #3650)
//  2025-02-25: [Docking] Revert to use SDL_GetDisplayBounds() for WorkPos/WorkRect if SDL_GetDisplayUsableBounds() failed.
//  2025-02-24: Avoid calling SDL_GetGlobalMouseState() when mouse is in relative mode.
//  2025-02-21: [Docking] Update monitors and work areas information every frame, as the later may change regardless of monitor changes. (#8415)
//  2025-02-18: Added LinearUIMouseCursor_Wait and LinearUIMouseCursor_Progress mouse cursor support.
//  2025-02-10: Using SDL_OpenURL() in platform_io.Platform_OpenInShellFn handler.
//  2025-01-20: Made LinearUI_ImplSDL2_SetGamepadMode(LinearUI_ImplSDL2_GamepadMode_Manual) accept an empty array.
//  2024-10-24: Emscripten: from SDL 2.30.9, SDL_EVENT_MOUSE_WHEEL event doesn't require dividing by 100.0f.
//  2024-09-09: use SDL_Vulkan_GetDrawableSize() when available. (#7967, #3190)
//  2024-08-22: moved some OS/backend related function pointers from LinearUIIO to LinearUIPlatformIO:
//               - io.GetClipboardTextFn    -> platform_io.Platform_GetClipboardTextFn
//               - io.SetClipboardTextFn    -> platform_io.Platform_SetClipboardTextFn
//               - io.PlatformOpenInShellFn -> platform_io.Platform_OpenInShellFn
//               - io.PlatformSetImeDataFn  -> platform_io.Platform_SetImeDataFn
//  2024-08-19: Storing SDL's Uint32 WindowID inside LinearUIViewport::PlatformHandle instead of SDL_Window*.
//  2024-08-19: LinearUI_ImplSDL2_ProcessEvent() now ignores events intended for other SDL windows. (#7853)
//  2024-07-02: Emscripten: Added io.PlatformOpenInShellFn() handler for Emscripten versions.
//  2024-07-02: Update for io.SetPlatformImeDataFn() -> io.PlatformSetImeDataFn() renaming in main library.
//  2024-02-14: Inputs: Handle gamepad disconnection. Added LinearUI_ImplSDL2_SetGamepadMode().
//  2023-10-05: Inputs: Added support for extra LinearUIKey values: F13 to F24 function keys, app back/forward keys.
//  2023-04-06: Inputs: Avoid calling SDL_StartTextInput()/SDL_StopTextInput() as they don't only pertain to IME. It's unclear exactly what their relation is to IME. (#6306)
//  2023-04-04: Inputs: Added support for io.AddMouseSourceEvent() to discriminate LinearUIMouseSource_Mouse/LinearUIMouseSource_TouchScreen. (#2702)
//  2023-02-23: Accept SDL_GetPerformanceCounter() not returning a monotonically increasing value. (#6189, #6114, #3644)
//  2023-02-07: Implement IME handler (io.SetPlatformImeDataFn will call SDL_SetTextInputRect()/SDL_StartTextInput()).
//  2023-02-07: *BREAKING CHANGE* Renamed this backend file from linearui_impl_sdl.cpp/.h to linearui_impl_sdl2.cpp/.h in prevision for the future release of SDL3.
//  2023-02-02: Avoid calling SDL_SetCursor() when cursor has not changed, as the function is surprisingly costly on Mac with latest SDL (may be fixed in next SDL version).
//  2023-02-02: Added support for SDL 2.0.18+ preciseX/preciseY mouse wheel data for smooth scrolling + Scaling X value on Emscripten (bug?). (#4019, #6096)
//  2023-02-02: Removed SDL_MOUSEWHEEL value clamping, as values seem correct in latest Emscripten. (#4019)
//  2023-02-01: Flipping SDL_MOUSEWHEEL 'wheel.x' value to match other backends and offer consistent horizontal scrolling direction. (#4019, #6096, #1463)
//  2022-10-11: Using 'nullptr' instead of 'NULL' as per our switch to C++11.
//  2022-09-26: Inputs: Disable SDL 2.0.22 new "auto capture" (SDL_HINT_MOUSE_AUTO_CAPTURE) which prevents drag and drop across windows for multi-viewport support + don't capture when drag and dropping. (#5710)
//  2022-09-26: Inputs: Renamed LinearUIKey_ModXXX introduced in 1.87 to LinearUIMod_XXX (old names still supported).
//  2022-03-22: Inputs: Fix mouse position issues when dragging outside of boundaries. SDL_CaptureMouse() erroneously still gives out LEAVE events when hovering OS decorations.
//  2022-03-22: Inputs: Added support for extra mouse buttons (SDL_BUTTON_X1/SDL_BUTTON_X2).
//  2022-02-04: Added SDL_Renderer* parameter to LinearUI_ImplSDL2_InitForSDLRenderer(), so we can use SDL_GetRendererOutputSize() instead of SDL_GL_GetDrawableSize() when bound to a SDL_Renderer.
//  2022-01-26: Inputs: replaced short-lived io.AddKeyModsEvent() (added two weeks ago) with io.AddKeyEvent() using LinearUIKey_ModXXX flags. Sorry for the confusion.
//  2021-01-20: Inputs: calling new io.AddKeyAnalogEvent() for gamepad support, instead of writing directly to io.NavInputs[].
//  2022-01-17: Inputs: calling new io.AddMousePosEvent(), io.AddMouseButtonEvent(), io.AddMouseWheelEvent() API (1.87+).
//  2022-01-17: Inputs: always update key mods next and before key event (not in NewFrame) to fix input queue with very low framerates.
//  2022-01-12: Update mouse inputs using SDL_MOUSEMOTION/SDL_WINDOWEVENT_LEAVE + fallback to provide it when focused but not hovered/captured. More standard and will allow us to pass it to future input queue API.
//  2022-01-12: Maintain our own copy of MouseButtonsDown mask instead of using LinearUI::IsAnyMouseDown() which will be obsoleted.
//  2022-01-10: Inputs: calling new io.AddKeyEvent(), io.AddKeyModsEvent() + io.SetKeyEventNativeData() API (1.87+). Support for full LinearUIKey range.
//  2021-08-17: Calling io.AddFocusEvent() on SDL_WINDOWEVENT_FOCUS_GAINED/SDL_WINDOWEVENT_FOCUS_LOST.
//  2021-07-29: Inputs: MousePos is correctly reported when the host platform window is hovered but not focused (using SDL_GetMouseFocus() + SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, requires SDL 2.0.5+)
//  2021-06:29: *BREAKING CHANGE* Removed 'SDL_Window* window' parameter to LinearUI_ImplSDL2_NewFrame() which was unnecessary.
//  2021-06-29: Reorganized backend to pull data from a single structure to facilitate usage with multiple-contexts (all g_XXXX access changed to bd->XXXX).
//  2021-03-22: Rework global mouse pos availability check listing supported platforms explicitly, effectively fixing mouse access on Raspberry Pi. (#2837, #3950)
//  2020-05-25: Misc: Report a zero display-size when window is minimized, to be consistent with other backends.
//  2020-02-20: Inputs: Fixed mapping for LinearUIKey_KeyPadEnter (using SDL_SCANCODE_KP_ENTER instead of SDL_SCANCODE_RETURN2).
//  2019-12-17: Inputs: On Wayland, use SDL_GetMouseState (because there is no global mouse state).
//  2019-12-05: Inputs: Added support for LinearUIMouseCursor_NotAllowed mouse cursor.
//  2019-07-21: Inputs: Added mapping for LinearUIKey_KeyPadEnter.
//  2019-04-23: Inputs: Added support for SDL_GameController (if LinearUIConfigFlags_NavEnableGamepad is set by user application).
//  2019-03-12: Misc: Preserve DisplayFramebufferScale when main window is minimized.
//  2018-12-21: Inputs: Workaround for Android/iOS which don't seem to handle focus related calls.
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-11-14: Changed the signature of LinearUI_ImplSDL2_ProcessEvent() to take a 'const SDL_Event*'.
//  2018-08-01: Inputs: Workaround for Emscripten which doesn't seem to handle focus related calls.
//  2018-06-29: Inputs: Added support for the LinearUIMouseCursor_Hand cursor.
//  2018-06-08: Misc: Extracted linearui_impl_sdl.cpp/.h away from the old combined SDL2+OpenGL/Vulkan examples.
//  2018-06-08: Misc: LinearUI_ImplSDL2_InitForOpenGL() now takes a SDL_GLContext parameter.
//  2018-05-09: Misc: Fixed clipboard paste memory leak (we didn't call SDL_FreeMemory on the data returned by SDL_GetClipboardText).
//  2018-03-20: Misc: Setup io.BackendFlags LinearUIBackendFlags_HasMouseCursors flag + honor LinearUIConfigFlags_NoMouseCursorChange flag.
//  2018-02-16: Inputs: Added support for mouse cursors, honoring LinearUI::GetMouseCursor() value.
//  2018-02-06: Misc: Removed call to LinearUI::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.
//  2018-02-06: Inputs: Added mapping for LinearUIKey_Space.
//  2018-02-05: Misc: Using SDL_GetPerformanceCounter() instead of SDL_GetTicks() to be able to handle very high framerate (1000+ FPS).
//  2018-02-05: Inputs: Keyboard mapping is using scancodes everywhere instead of a confusing mixture of keycodes and scancodes.
//  2018-01-20: Inputs: Added Horizontal Mouse Wheel support.
//  2018-01-19: Inputs: When available (SDL 2.0.4+) using SDL_CaptureMouse() to retrieve coordinates outside of client area when dragging. Otherwise (SDL 2.0.3 and before) testing for SDL_WINDOW_INPUT_FOCUS instead of SDL_WINDOW_MOUSE_FOCUS.
//  2018-01-18: Inputs: Added mapping for LinearUIKey_Insert.
//  2017-08-25: Inputs: MousePos set to -FLT_MAX,-FLT_MAX when mouse is unavailable/missing (instead of -1,-1).
//  2016-10-15: Misc: Added a void* user_data parameter to Clipboard function handlers.

#include "linearui.h"
#ifndef LINEARUI_DISABLE
#include "linearui_impl_sdl2.h"

// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#endif

// SDL
// (the multi-viewports feature requires SDL features supported from SDL 2.0.4+. SDL 2.0.5+ is highly recommended)
#include <SDL.h>
#include <SDL_syswm.h>
#include <stdio.h>              // for snprintf()
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten/em_js.h>
#endif
#undef Status // X11 headers are leaking this.

#if SDL_VERSION_ATLEAST(2,0,4) && !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) && !(defined(__APPLE__) && TARGET_OS_IOS) && !defined(__amigaos4__)
#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE    1
#else
#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE    0
#endif
#define SDL_HAS_WINDOW_ALPHA                SDL_VERSION_ATLEAST(2,0,5)
#define SDL_HAS_ALWAYS_ON_TOP               SDL_VERSION_ATLEAST(2,0,5)
#define SDL_HAS_USABLE_DISPLAY_BOUNDS       SDL_VERSION_ATLEAST(2,0,5)
#define SDL_HAS_PER_MONITOR_DPI             SDL_VERSION_ATLEAST(2,0,4)
#define SDL_HAS_VULKAN                      SDL_VERSION_ATLEAST(2,0,6)
#define SDL_HAS_DISPLAY_EVENT               SDL_VERSION_ATLEAST(2,0,9)
#define SDL_HAS_OPEN_URL                    SDL_VERSION_ATLEAST(2,0,14)
#define SDL_HAS_SHOW_WINDOW_ACTIVATION_HINT SDL_VERSION_ATLEAST(2,0,18)
#if SDL_HAS_VULKAN
#include <SDL_vulkan.h>
#else
static const Uint32 SDL_WINDOW_VULKAN = 0x10000000;
#endif

// SDL Data
struct LinearUI_ImplSDL2_Data
{
    SDL_Window*             Window;
    Uint32                  WindowID;       // Stored in LinearUIViewport::PlatformHandle. Use SDL_GetWindowFromID() to get SDL_Window* from Uint32 WindowID.
    SDL_Renderer*           Renderer;
    Uint64                  Time;
    char*                   ClipboardTextData;
    char                    BackendPlatformName[48];
    bool                    UseVulkan;
    bool                    WantUpdateMonitors;

    // Mouse handling
    Uint32                  MouseWindowID;
    int                     MouseButtonsDown;
    SDL_Cursor*             MouseCursors[LinearUIMouseCursor_COUNT];
    SDL_Cursor*             MouseLastCursor;
    int                     MouseLastLeaveFrame;
    bool                    MouseCanUseGlobalState;
    bool                    MouseCanReportHoveredViewport;  // This is hard to use/unreliable on SDL so we'll set LinearUIBackendFlags_HasMouseHoveredViewport dynamically based on state.
    LinearUI_ImplSDL2_MouseCaptureMode MouseCaptureMode;

    // Gamepad handling
    LinearVector<SDL_GameController*> Gamepads;
    LinearUI_ImplSDL2_GamepadMode    GamepadMode;
    bool                          WantUpdateGamepadsList;

    LinearUI_ImplSDL2_Data()   { memset((void*)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear LinearUI contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear LinearUI context + multiple windows) instead of multiple Dear LinearUI contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
// FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled when using multi-context.
static LinearUI_ImplSDL2_Data* LinearUI_ImplSDL2_GetBackendData()
{
    return LinearUI::GetCurrentContext() ? (LinearUI_ImplSDL2_Data*)LinearUI::GetIO().BackendPlatformUserData : nullptr;
}

// Forward Declarations
static void LinearUI_ImplSDL2_UpdateMonitors();
static void LinearUI_ImplSDL2_InitMultiViewportSupport(SDL_Window* window, void* sdl_gl_context);
static void LinearUI_ImplSDL2_ShutdownMultiViewportSupport();

// Functions
static const char* LinearUI_ImplSDL2_GetClipboardText(LinearUIContext*)
{
    LinearUI_ImplSDL2_Data* bd = LinearUI_ImplSDL2_GetBackendData();
    if (bd->ClipboardTextData)
        SDL_free(bd->ClipboardTextData);
    if (SDL_HasClipboardText())
        bd->ClipboardTextData = SDL_GetClipboardText();
    else
        bd->ClipboardTextData = nullptr;
    return bd->ClipboardTextData;
}

static void LinearUI_ImplSDL2_SetClipboardText(LinearUIContext*, const char* text)
{
    SDL_SetClipboardText(text);
}

// Note: native IME will only display if user calls SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1") _before_ SDL_CreateWindow().
static void LinearUI_ImplSDL2_PlatformSetImeData(LinearUIContext*, LinearUIViewport* viewport, LinearUIPlatformImeData* data)
{
    if (data->WantVisible)
    {
        SDL_Rect r;
        r.x = (int)(data->InputPos.x - viewport->Pos.x);
        r.y = (int)(data->InputPos.y - viewport->Pos.y);
        r.w = 1;
        r.h = (int)data->InputLineHeight;
        SDL_SetTextInputRect(&r);
    }
}

// Not static to allow third-party code to use that if they want to (but undocumented)
LinearUIKey LinearUI_ImplSDL2_KeyEventToLinearUIKey(SDL_Keycode keycode, SDL_Scancode scancode);
LinearUIKey LinearUI_ImplSDL2_KeyEventToLinearUIKey(SDL_Keycode keycode, SDL_Scancode scancode)
{
    switch (keycode)
    {
        case SDLK_TAB: return LinearUIKey_Tab;
        case SDLK_LEFT: return LinearUIKey_LeftArrow;
        case SDLK_RIGHT: return LinearUIKey_RightArrow;
        case SDLK_UP: return LinearUIKey_UpArrow;
        case SDLK_DOWN: return LinearUIKey_DownArrow;
        case SDLK_PAGEUP: return LinearUIKey_PageUp;
        case SDLK_PAGEDOWN: return LinearUIKey_PageDown;
        case SDLK_HOME: return LinearUIKey_Home;
        case SDLK_END: return LinearUIKey_End;
        case SDLK_INSERT: return LinearUIKey_Insert;
        case SDLK_DELETE: return LinearUIKey_Delete;
        case SDLK_BACKSPACE: return LinearUIKey_Backspace;
        case SDLK_SPACE: return LinearUIKey_Space;
        case SDLK_RETURN: return LinearUIKey_Enter;
        case SDLK_ESCAPE: return LinearUIKey_Escape;
        //case SDLK_QUOTE: return LinearUIKey_Apostrophe;
        case SDLK_COMMA: return LinearUIKey_Comma;
        //case SDLK_MINUS: return LinearUIKey_Minus;
        case SDLK_PERIOD: return LinearUIKey_Period;
        //case SDLK_SLASH: return LinearUIKey_Slash;
        case SDLK_SEMICOLON: return LinearUIKey_Semicolon;
        //case SDLK_EQUALS: return LinearUIKey_Equal;
        //case SDLK_LEFTBRACKET: return LinearUIKey_LeftBracket;
        //case SDLK_BACKSLASH: return LinearUIKey_Backslash;
        //case SDLK_RIGHTBRACKET: return LinearUIKey_RightBracket;
        //case SDLK_BACKQUOTE: return LinearUIKey_GraveAccent;
        case SDLK_CAPSLOCK: return LinearUIKey_CapsLock;
        case SDLK_SCROLLLOCK: return LinearUIKey_ScrollLock;
        case SDLK_NUMLOCKCLEAR: return LinearUIKey_NumLock;
        case SDLK_PRINTSCREEN: return LinearUIKey_PrintScreen;
        case SDLK_PAUSE: return LinearUIKey_Pause;
        case SDLK_KP_0: return LinearUIKey_Keypad0;
        case SDLK_KP_1: return LinearUIKey_Keypad1;
        case SDLK_KP_2: return LinearUIKey_Keypad2;
        case SDLK_KP_3: return LinearUIKey_Keypad3;
        case SDLK_KP_4: return LinearUIKey_Keypad4;
        case SDLK_KP_5: return LinearUIKey_Keypad5;
        case SDLK_KP_6: return LinearUIKey_Keypad6;
        case SDLK_KP_7: return LinearUIKey_Keypad7;
        case SDLK_KP_8: return LinearUIKey_Keypad8;
        case SDLK_KP_9: return LinearUIKey_Keypad9;
        case SDLK_KP_PERIOD: return LinearUIKey_KeypadDecimal;
        case SDLK_KP_DIVIDE: return LinearUIKey_KeypadDivide;
        case SDLK_KP_MULTIPLY: return LinearUIKey_KeypadMultiply;
        case SDLK_KP_MINUS: return LinearUIKey_KeypadSubtract;
        case SDLK_KP_PLUS: return LinearUIKey_KeypadAdd;
        case SDLK_KP_ENTER: return LinearUIKey_KeypadEnter;
        case SDLK_KP_EQUALS: return LinearUIKey_KeypadEqual;
        case SDLK_LCTRL: return LinearUIKey_LeftCtrl;
        case SDLK_LSHIFT: return LinearUIKey_LeftShift;
        case SDLK_LALT: return LinearUIKey_LeftAlt;
        case SDLK_LGUI: return LinearUIKey_LeftSuper;
        case SDLK_RCTRL: return LinearUIKey_RightCtrl;
        case SDLK_RSHIFT: return LinearUIKey_RightShift;
        case SDLK_RALT: return LinearUIKey_RightAlt;
        case SDLK_RGUI: return LinearUIKey_RightSuper;
        case SDLK_APPLICATION: return LinearUIKey_Menu;
        case SDLK_0: return LinearUIKey_0;
        case SDLK_1: return LinearUIKey_1;
        case SDLK_2: return LinearUIKey_2;
        case SDLK_3: return LinearUIKey_3;
        case SDLK_4: return LinearUIKey_4;
        case SDLK_5: return LinearUIKey_5;
        case SDLK_6: return LinearUIKey_6;
        case SDLK_7: return LinearUIKey_7;
        case SDLK_8: return LinearUIKey_8;
        case SDLK_9: return LinearUIKey_9;
        case SDLK_a: return LinearUIKey_A;
        case SDLK_b: return LinearUIKey_B;
        case SDLK_c: return LinearUIKey_C;
        case SDLK_d: return LinearUIKey_D;
        case SDLK_e: return LinearUIKey_E;
        case SDLK_f: return LinearUIKey_F;
        case SDLK_g: return LinearUIKey_G;
        case SDLK_h: return LinearUIKey_H;
        case SDLK_i: return LinearUIKey_I;
        case SDLK_j: return LinearUIKey_J;
        case SDLK_k: return LinearUIKey_K;
        case SDLK_l: return LinearUIKey_L;
        case SDLK_m: return LinearUIKey_M;
        case SDLK_n: return LinearUIKey_N;
        case SDLK_o: return LinearUIKey_O;
        case SDLK_p: return LinearUIKey_P;
        case SDLK_q: return LinearUIKey_Q;
        case SDLK_r: return LinearUIKey_R;
        case SDLK_s: return LinearUIKey_S;
        case SDLK_t: return LinearUIKey_T;
        case SDLK_u: return LinearUIKey_U;
        case SDLK_v: return LinearUIKey_V;
        case SDLK_w: return LinearUIKey_W;
        case SDLK_x: return LinearUIKey_X;
        case SDLK_y: return LinearUIKey_Y;
        case SDLK_z: return LinearUIKey_Z;
        case SDLK_F1: return LinearUIKey_F1;
        case SDLK_F2: return LinearUIKey_F2;
        case SDLK_F3: return LinearUIKey_F3;
        case SDLK_F4: return LinearUIKey_F4;
        case SDLK_F5: return LinearUIKey_F5;
        case SDLK_F6: return LinearUIKey_F6;
        case SDLK_F7: return LinearUIKey_F7;
        case SDLK_F8: return LinearUIKey_F8;
        case SDLK_F9: return LinearUIKey_F9;
        case SDLK_F10: return LinearUIKey_F10;
        case SDLK_F11: return LinearUIKey_F11;
        case SDLK_F12: return LinearUIKey_F12;
        case SDLK_F13: return LinearUIKey_F13;
        case SDLK_F14: return LinearUIKey_F14;
        case SDLK_F15: return LinearUIKey_F15;
        case SDLK_F16: return LinearUIKey_F16;
        case SDLK_F17: return LinearUIKey_F17;
        case SDLK_F18: return LinearUIKey_F18;
        case SDLK_F19: return LinearUIKey_F19;
        case SDLK_F20: return LinearUIKey_F20;
        case SDLK_F21: return LinearUIKey_F21;
        case SDLK_F22: return LinearUIKey_F22;
        case SDLK_F23: return LinearUIKey_F23;
        case SDLK_F24: return LinearUIKey_F24;
        case SDLK_AC_BACK: return LinearUIKey_AppBack;
        case SDLK_AC_FORWARD: return LinearUIKey_AppForward;
        default: break;
    }

    // Fallback to scancode
    switch (scancode)
    {
    case SDL_SCANCODE_GRAVE: return LinearUIKey_GraveAccent;
    case SDL_SCANCODE_MINUS: return LinearUIKey_Minus;
    case SDL_SCANCODE_EQUALS: return LinearUIKey_Equal;
    case SDL_SCANCODE_LEFTBRACKET: return LinearUIKey_LeftBracket;
    case SDL_SCANCODE_RIGHTBRACKET: return LinearUIKey_RightBracket;
    case SDL_SCANCODE_NONUSBACKSLASH: return LinearUIKey_Oem102;
    case SDL_SCANCODE_BACKSLASH: return LinearUIKey_Backslash;
    case SDL_SCANCODE_SEMICOLON: return LinearUIKey_Semicolon;
    case SDL_SCANCODE_APOSTROPHE: return LinearUIKey_Apostrophe;
    case SDL_SCANCODE_COMMA: return LinearUIKey_Comma;
    case SDL_SCANCODE_PERIOD: return LinearUIKey_Period;
    case SDL_SCANCODE_SLASH: return LinearUIKey_Slash;
    default: break;
    }
    return LinearUIKey_None;
}

static void LinearUI_ImplSDL2_UpdateKeyModifiers(SDL_Keymod sdl_key_mods)
{
    LinearUIIO& io = LinearUI::GetIO();
    io.AddKeyEvent(LinearUIMod_Ctrl, (sdl_key_mods & KMOD_CTRL) != 0);
    io.AddKeyEvent(LinearUIMod_Shift, (sdl_key_mods & KMOD_SHIFT) != 0);
    io.AddKeyEvent(LinearUIMod_Alt, (sdl_key_mods & KMOD_ALT) != 0);
    io.AddKeyEvent(LinearUIMod_Super, (sdl_key_mods & KMOD_GUI) != 0);
}

static LinearUIViewport* LinearUI_ImplSDL2_GetViewportForWindowID(Uint32 window_id)
{
    return LinearUI::FindViewportByPlatformHandle((void*)(intptr_t)window_id);
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear linearui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear linearui, and hide them from your application based on those two flags.
bool LinearUI_ImplSDL2_ProcessEvent(const SDL_Event* event)
{
    LinearUI_ImplSDL2_Data* bd = LinearUI_ImplSDL2_GetBackendData();
    LINEARUI_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call LinearUI_ImplSDL2_Init()?");
    LinearUIIO& io = LinearUI::GetIO();

    switch (event->type)
    {
        case SDL_MOUSEMOTION:
        {
            if (LinearUI_ImplSDL2_GetViewportForWindowID(event->motion.windowID) == nullptr)
                return false;
            LinearVec2 mouse_pos((float)event->motion.x, (float)event->motion.y);
            if (io.ConfigFlags & LinearUIConfigFlags_ViewportsEnable)
            {
                int window_x, window_y;
                SDL_GetWindowPosition(SDL_GetWindowFromID(event->motion.windowID), &window_x, &window_y);
                mouse_pos.x += window_x;
                mouse_pos.y += window_y;
            }
            io.AddMouseSourceEvent(event->motion.which == SDL_TOUCH_MOUSEID ? LinearUIMouseSource_TouchScreen : LinearUIMouseSource_Mouse);
            io.AddMousePosEvent(mouse_pos.x, mouse_pos.y);
            return true;
        }
        case SDL_MOUSEWHEEL:
        {
            if (LinearUI_ImplSDL2_GetViewportForWindowID(event->wheel.windowID) == nullptr)
                return false;
            //LINEARUI_DEBUG_LOG("wheel %.2f %.2f, precise %.2f %.2f\n", (float)event->wheel.x, (float)event->wheel.y, event->wheel.preciseX, event->wheel.preciseY);
#if SDL_VERSION_ATLEAST(2,0,18) // If this fails to compile on Emscripten: update to latest Emscripten!
            float wheel_x = -event->wheel.preciseX;
            float wheel_y = event->wheel.preciseY;
#else
            float wheel_x = -(float)event->wheel.x;
            float wheel_y = (float)event->wheel.y;
#endif
#if defined(__EMSCRIPTEN__) && !SDL_VERSION_ATLEAST(2,31,0)
            wheel_x /= 100.0f;
#endif
            io.AddMouseSourceEvent(event->wheel.which == SDL_TOUCH_MOUSEID ? LinearUIMouseSource_TouchScreen : LinearUIMouseSource_Mouse);
            io.AddMouseWheelEvent(wheel_x, wheel_y);
            return true;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            if (LinearUI_ImplSDL2_GetViewportForWindowID(event->button.windowID) == nullptr)
                return false;
            int mouse_button = -1;
            if (event->button.button == SDL_BUTTON_LEFT) { mouse_button = 0; }
            if (event->button.button == SDL_BUTTON_RIGHT) { mouse_button = 1; }
            if (event->button.button == SDL_BUTTON_MIDDLE) { mouse_button = 2; }
            if (event->button.button == SDL_BUTTON_X1) { mouse_button = 3; }
            if (event->button.button == SDL_BUTTON_X2) { mouse_button = 4; }
            if (mouse_button == -1)
                break;
            io.AddMouseSourceEvent(event->button.which == SDL_TOUCH_MOUSEID ? LinearUIMouseSource_TouchScreen : LinearUIMouseSource_Mouse);
            io.AddMouseButtonEvent(mouse_button, (event->type == SDL_MOUSEBUTTONDOWN));
            bd->MouseButtonsDown = (event->type == SDL_MOUSEBUTTONDOWN) ? (bd->MouseButtonsDown | (1 << mouse_button)) : (bd->MouseButtonsDown & ~(1 << mouse_button));
            return true;
        }
        case SDL_TEXTINPUT:
        {
            if (LinearUI_ImplSDL2_GetViewportForWindowID(event->text.windowID) == nullptr)
                return false;
            io.AddInputCharactersUTF8(event->text.text);
            return true;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            LinearUIViewport* viewport = LinearUI_ImplSDL2_GetViewportForWindowID(event->key.windowID);
            if (viewport == nullptr)
                return false;
            //LINEARUI_DEBUG_LOG("SDL_KEY%s : key=0x%08X ('%s'), scancode=%d ('%s'), mod=%X, windowID=%d, viewport=%08X\n",
            //    (event->type == SDL_KEYDOWN) ? "DOWN" : "UP  ", event->key.keysym.sym, SDL_GetKeyName(event->key.keysym.sym), event->key.keysym.scancode, SDL_GetScancodeName(event->key.keysym.scancode), event->key.keysym.mod, event->key.windowID, viewport ? viewport->ID : 0);
            LinearUI_ImplSDL2_UpdateKeyModifiers((SDL_Keymod)event->key.keysym.mod);
            LinearUIKey key = LinearUI_ImplSDL2_KeyEventToLinearUIKey(event->key.keysym.sym, event->key.keysym.scancode);
            io.AddKeyEvent(key, (event->type == SDL_KEYDOWN));
            io.SetKeyEventNativeData(key, (int)event->key.keysym.sym, (int)event->key.keysym.scancode, (int)event->key.keysym.scancode); // To support legacy indexing (<1.87 user code). Legacy backend uses SDLK_*** as indices to IsKeyXXX() functions.
            return true;
        }
#if SDL_HAS_DISPLAY_EVENT
        case SDL_DISPLAYEVENT:
        {
            // 2.0.26 has SDL_DISPLAYEVENT_CONNECTED/SDL_DISPLAYEVENT_DISCONNECTED/SDL_DISPLAYEVENT_ORIENTATION,
            // so change of DPI/Scaling are not reflected in this event. (SDL3 has it)
            bd->WantUpdateMonitors = true;
            return true;
        }
#endif
        case SDL_WINDOWEVENT:
        {
            LinearUIViewport* viewport = LinearUI_ImplSDL2_GetViewportForWindowID(event->window.windowID);
            if (viewport == NULL)
                return false;

            // - When capturing mouse, SDL will send a bunch of conflicting LEAVE/ENTER event on every mouse move, but the final ENTER tends to be right.
            // - However we won't get a correct LEAVE event for a captured window.
            // - In some cases, when detaching a window from main viewport SDL may send SDL_WINDOWEVENT_ENTER one frame too late,
            //   causing SDL_WINDOWEVENT_LEAVE on previous frame to interrupt drag operation by clear mouse position. This is why
            //   we delay process the SDL_WINDOWEVENT_LEAVE events by one frame. See issue #5012 for details.
            Uint8 window_event = event->window.event;
            if (window_event == SDL_WINDOWEVENT_ENTER)
            {
                bd->MouseWindowID = event->window.windowID;
                bd->MouseLastLeaveFrame = 0;
            }
            if (window_event == SDL_WINDOWEVENT_LEAVE)
                bd->MouseLastLeaveFrame = LinearUI::GetFrameCount() + 1;
            //if (window_event == SDL_WINDOWEVENT_FOCUS_GAINED) { LINEARUI_DEBUG_LOG("SDL_WINDOWEVENT_FOCUS_GAINED: windowId %d, viewport: %08X\n", event->window.windowID, viewport ? viewport->ID : 0); }
            //if (window_event == SDL_WINDOWEVENT_FOCUS_LOST) { LINEARUI_DEBUG_LOG("SDL_WINDOWEVENT_FOCUS_LOST: windowId %d, viewport: %08X\n", event->window.windowID, viewport ? viewport->ID : 0); }
            if (window_event == SDL_WINDOWEVENT_FOCUS_GAINED)
                io.AddFocusEvent(true);
            if (window_event == SDL_WINDOWEVENT_FOCUS_LOST)
                io.AddFocusEvent(false);
            if (window_event == SDL_WINDOWEVENT_CLOSE)
                viewport->PlatformRequestClose = true;
            if (window_event == SDL_WINDOWEVENT_MOVED)
                viewport->PlatformRequestMove = true;
            if (window_event == SDL_WINDOWEVENT_RESIZED)
                viewport->PlatformRequestResize = true;
            return true;
        }
        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
        {
            bd->WantUpdateGamepadsList = true;
            return true;
        }
        default:
            break;
    }
    return false;
}

#ifdef __EMSCRIPTEN__
EM_JS(void, LinearUI_ImplSDL2_EmscriptenOpenURL, (char const* url), { url = url ? UTF8ToString(url) : null; if (url) window.open(url, '_blank'); });
#endif

static bool LinearUI_ImplSDL2_Init(SDL_Window* window, SDL_Renderer* renderer, void* sdl_gl_context)
{
    LinearUIIO& io = LinearUI::GetIO();
    LINEARUI_CHECKVERSION();
    LINEARUI_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");
    //SDL_SetHint(SDL_HINT_EVENT_LOGGING, "2");

    // Obtain compiled and runtime versions
    SDL_version ver_compiled;
    SDL_version ver_runtime;
    SDL_VERSION(&ver_compiled);
    SDL_GetVersion(&ver_runtime);

    // Setup backend capabilities flags
    LinearUI_ImplSDL2_Data* bd = LINEARUI_NEW(LinearUI_ImplSDL2_Data)();
    snprintf(bd->BackendPlatformName, sizeof(bd->BackendPlatformName), "linearui_impl_sdl2 (%u.%u.%u, %u.%u.%u)",
        ver_compiled.major, ver_compiled.minor, ver_compiled.patch, ver_runtime.major, ver_runtime.minor, ver_runtime.patch);
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = bd->BackendPlatformName;
    io.BackendFlags |= LinearUIBackendFlags_HasMouseCursors;           // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= LinearUIBackendFlags_HasSetMousePos;            // We can honor io.WantSetMousePos requests (optional, rarely used)
    // (LinearUIBackendFlags_PlatformHasViewports may be set just below)

    bd->Window = window;
    bd->WindowID = SDL_GetWindowID(window);
    bd->Renderer = renderer;

    // Check and store if we are on a SDL backend that supports SDL_GetGlobalMouseState() and SDL_CaptureMouse()
    // ("wayland" and "rpi" don't support it, but we chose to use a white-list instead of a black-list)
    bd->MouseCanUseGlobalState = false;
    bd->MouseCaptureMode = LinearUI_ImplSDL2_MouseCaptureMode_Disabled;
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
    const char* sdl_backend = SDL_GetCurrentVideoDriver();
    const char* capture_and_global_state_whitelist[] = { "windows", "cocoa", "x11", "DIVE", "VMAN" };
    for (const char* item : capture_and_global_state_whitelist)
        if (strncmp(sdl_backend, item, strlen(item)) == 0)
        {
            bd->MouseCanUseGlobalState = true;
            bd->MouseCaptureMode = (strcmp(item, "x11") == 0) ? LinearUI_ImplSDL2_MouseCaptureMode_EnabledAfterDrag : LinearUI_ImplSDL2_MouseCaptureMode_Enabled;
        }
#endif
    if (bd->MouseCanUseGlobalState)
        io.BackendFlags |= LinearUIBackendFlags_PlatformHasViewports;  // We can create multi-viewports on the Platform side (optional)

    // SDL on Linux/OSX doesn't report events for unfocused windows (see https://github.com/ocornut/linearui/issues/4960)
    // We will use 'MouseCanReportHoveredViewport' to set 'LinearUIBackendFlags_HasMouseHoveredViewport' dynamically each frame.
#ifndef __APPLE__
    bd->MouseCanReportHoveredViewport = bd->MouseCanUseGlobalState;
#else
    bd->MouseCanReportHoveredViewport = false;
#endif

    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
    platform_io.Platform_SetClipboardTextFn = LinearUI_ImplSDL2_SetClipboardText;
    platform_io.Platform_GetClipboardTextFn = LinearUI_ImplSDL2_GetClipboardText;
    platform_io.Platform_ClipboardUserData = nullptr;
    platform_io.Platform_SetImeDataFn = LinearUI_ImplSDL2_PlatformSetImeData;
#ifdef __EMSCRIPTEN__
    platform_io.Platform_OpenInShellFn = [](LinearUIContext*, const char* url) { LinearUI_ImplSDL2_EmscriptenOpenURL(url); return true; };
#elif SDL_HAS_OPEN_URL
    platform_io.Platform_OpenInShellFn = [](LinearUIContext*, const char* url) { return SDL_OpenURL(url) == 0; };
#endif

    // Update monitor a first time during init
    LinearUI_ImplSDL2_UpdateMonitors();

    // Gamepad handling
    bd->GamepadMode = LinearUI_ImplSDL2_GamepadMode_AutoFirst;
    bd->WantUpdateGamepadsList = true;

    // Load mouse cursors
    bd->MouseCursors[LinearUIMouseCursor_Arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    bd->MouseCursors[LinearUIMouseCursor_TextInput] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
    bd->MouseCursors[LinearUIMouseCursor_ResizeAll] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
    bd->MouseCursors[LinearUIMouseCursor_ResizeNS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
    bd->MouseCursors[LinearUIMouseCursor_ResizeEW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
    bd->MouseCursors[LinearUIMouseCursor_ResizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
    bd->MouseCursors[LinearUIMouseCursor_ResizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
    bd->MouseCursors[LinearUIMouseCursor_Hand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    bd->MouseCursors[LinearUIMouseCursor_Wait] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
    bd->MouseCursors[LinearUIMouseCursor_Progress] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
    bd->MouseCursors[LinearUIMouseCursor_NotAllowed] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);

    // Set platform dependent data in viewport
    // Our mouse update function expect PlatformHandle to be filled for the main viewport
    LinearUIViewport* main_viewport = LinearUI::GetMainViewport();
    main_viewport->PlatformHandle = (void*)(intptr_t)bd->WindowID;
    main_viewport->PlatformHandleRaw = nullptr;
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(window, &info))
    {
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
        main_viewport->PlatformHandleRaw = (void*)info.info.win.window;
#elif defined(__APPLE__) && defined(SDL_VIDEO_DRIVER_COCOA)
        main_viewport->PlatformHandleRaw = (void*)info.info.cocoa.window;
#endif
    }

    // From 2.0.5: Set SDL hint to receive mouse click events on window focus, otherwise SDL doesn't emit the event.
    // Without this, when clicking to gain focus, our widgets wouldn't activate even though they showed as hovered.
    // (This is unfortunately a global SDL setting, so enabling it might have a side-effect on your application.
    // It is unlikely to make a difference, but if your app absolutely needs to ignore the initial on-focus click:
    // you can ignore SDL_MOUSEBUTTONDOWN events coming right after a SDL_WINDOWEVENT_FOCUS_GAINED)
#ifdef SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
#endif

    // From 2.0.18: Enable native IME.
    // IMPORTANT: This is used at the time of SDL_CreateWindow() so this will only affects secondary windows, if any.
    // For the main window to be affected, your application needs to call this manually before calling SDL_CreateWindow().
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // From 2.0.22: Disable auto-capture, this is preventing drag and drop across multiple windows (see #5710)
#ifdef SDL_HINT_MOUSE_AUTO_CAPTURE
    SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "0");
#endif

    // We need SDL_CaptureMouse(), SDL_GetGlobalMouseState() from SDL 2.0.4+ to support multiple viewports.
    // We left the call to LinearUI_ImplSDL2_InitMultiViewportSupport() outside of #ifdef to avoid unused-function warnings.
    if (io.BackendFlags & LinearUIBackendFlags_PlatformHasViewports)
        LinearUI_ImplSDL2_InitMultiViewportSupport(window, sdl_gl_context);

    return true;
}

bool LinearUI_ImplSDL2_InitForOpenGL(SDL_Window* window, void* sdl_gl_context)
{
    return LinearUI_ImplSDL2_Init(window, nullptr, sdl_gl_context);
}

bool LinearUI_ImplSDL2_InitForVulkan(SDL_Window* window)
{
#if !SDL_HAS_VULKAN
    LINEARUI_ASSERT(0 && "Unsupported");
#endif
    if (!LinearUI_ImplSDL2_Init(window, nullptr, nullptr))
        return false;
    LinearUI_ImplSDL2_Data* bd = LinearUI_ImplSDL2_GetBackendData();
    bd->UseVulkan = true;
    return true;
}

bool LinearUI_ImplSDL2_InitForD3D(SDL_Window* window)
{
#if !defined(_WIN32)
    LINEARUI_ASSERT(0 && "Unsupported");
#endif
    return LinearUI_ImplSDL2_Init(window, nullptr, nullptr);
}

bool LinearUI_ImplSDL2_InitForMetal(SDL_Window* window)
{
    return LinearUI_ImplSDL2_Init(window, nullptr, nullptr);
}

bool LinearUI_ImplSDL2_InitForSDLRenderer(SDL_Window* window, SDL_Renderer* renderer)
{
    return LinearUI_ImplSDL2_Init(window, renderer, nullptr);
}

bool LinearUI_ImplSDL2_InitForOther(SDL_Window* window)
{
    return LinearUI_ImplSDL2_Init(window, nullptr, nullptr);
}

static void LinearUI_ImplSDL2_CloseGamepads();

void LinearUI_ImplSDL2_Shutdown()
{
    LinearUI_ImplSDL2_Data* bd = LinearUI_ImplSDL2_GetBackendData();
    LINEARUI_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
    LinearUIIO& io = LinearUI::GetIO();
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();

    LinearUI_ImplSDL2_ShutdownMultiViewportSupport();
    if (bd->ClipboardTextData)
        SDL_free(bd->ClipboardTextData);
    for (LinearUIMouseCursor cursor_n = 0; cursor_n < LinearUIMouseCursor_COUNT; cursor_n++)
        SDL_FreeCursor(bd->MouseCursors[cursor_n]);
    LinearUI_ImplSDL2_CloseGamepads();

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(LinearUIBackendFlags_HasMouseCursors | LinearUIBackendFlags_HasSetMousePos | LinearUIBackendFlags_HasGamepad | LinearUIBackendFlags_PlatformHasViewports | LinearUIBackendFlags_HasMouseHoveredViewport);
    platform_io.ClearPlatformHandlers();
    LINEARUI_DELETE(bd);
}

void LinearUI_ImplSDL2_SetMouseCaptureMode(LinearUI_ImplSDL2_MouseCaptureMode mode)
{
    LinearUI_ImplSDL2_Data* bd = LinearUI_ImplSDL2_GetBackendData();
    if (mode == LinearUI_ImplSDL2_MouseCaptureMode_Disabled && bd->MouseCaptureMode != LinearUI_ImplSDL2_MouseCaptureMode_Disabled)
        SDL_CaptureMouse(SDL_FALSE);
    bd->MouseCaptureMode = mode;
}

// This code is rather messy because some of the functions we need for full viewport support are not available in SDL < 2.0.4.
static void LinearUI_ImplSDL2_UpdateMouseData()
{
    LinearUI_ImplSDL2_Data* bd = LinearUI_ImplSDL2_GetBackendData();
    LinearUIIO& io = LinearUI::GetIO();

    // We forward mouse input when hovered or captured (via SDL_MOUSEMOTION) or when focused (below)
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
    // - SDL_CaptureMouse() let the OS know e.g. that our drags can extend outside of parent boundaries (we want updated position) and shouldn't trigger other operations outside.
    // - Debuggers under Linux tends to leave captured mouse on break, which may be very inconvenient, so to mitigate the issue on X11 we we wait until mouse has moved to begin capture.
    if (bd->MouseCaptureMode == LinearUI_ImplSDL2_MouseCaptureMode_Enabled)
    {
        SDL_CaptureMouse((bd->MouseButtonsDown != 0) ? SDL_TRUE : SDL_FALSE);
    }
    else if (bd->MouseCaptureMode == LinearUI_ImplSDL2_MouseCaptureMode_EnabledAfterDrag)
    {
        bool want_capture = false;
        for (int button_n = 0; button_n < LinearUIMouseButton_COUNT && !want_capture; button_n++)
            if (LinearUI::IsMouseDragging(button_n, 1.0f))
                want_capture = true;
        SDL_CaptureMouse(want_capture ? SDL_TRUE : SDL_FALSE);
    }

    SDL_Window* focused_window = SDL_GetKeyboardFocus();
    const bool is_app_focused = (focused_window && (bd->Window == focused_window || LinearUI_ImplSDL2_GetViewportForWindowID(SDL_GetWindowID(focused_window)) != NULL));
#else
    SDL_Window* focused_window = bd->Window;
    const bool is_app_focused = (SDL_GetWindowFlags(bd->Window) & SDL_WINDOW_INPUT_FOCUS) != 0; // SDL 2.0.3 and non-windowed systems: single-viewport only
#endif

    if (is_app_focused)
    {
        // (Optional) Set OS mouse position from Dear LinearUI if requested (rarely used, only when io.ConfigNavMoveSetMousePos is enabled by user)
        if (io.WantSetMousePos)
        {
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
            if (io.ConfigFlags & LinearUIConfigFlags_ViewportsEnable)
                SDL_WarpMouseGlobal((int)io.MousePos.x, (int)io.MousePos.y);
            else
#endif
                SDL_WarpMouseInWindow(bd->Window, (int)io.MousePos.x, (int)io.MousePos.y);
        }

        // (Optional) Fallback to provide unclamped mouse position when focused but not hovered (SDL_MOUSEMOTION already provides this when hovered or captured)
        // Note that SDL_GetGlobalMouseState() is in theory slow on X11, but this only runs on rather specific cases. If a problem we may provide a way to opt-out this feature.
        SDL_Window* hovered_window = SDL_GetMouseFocus();
        const bool is_relative_mouse_mode = SDL_GetRelativeMouseMode() != 0;
        if (hovered_window == nullptr && bd->MouseCanUseGlobalState && bd->MouseButtonsDown == 0 && !is_relative_mouse_mode)
        {
            // Single-viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window)
            // Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
            int mouse_x, mouse_y;
            int window_x, window_y;
            SDL_GetGlobalMouseState(&mouse_x, &mouse_y);
            if (!(io.ConfigFlags & LinearUIConfigFlags_ViewportsEnable))
            {
                SDL_GetWindowPosition(focused_window, &window_x, &window_y);
                mouse_x -= window_x;
                mouse_y -= window_y;
            }
            io.AddMousePosEvent((float)mouse_x, (float)mouse_y);
        }
    }

    // (Optional) When using multiple viewports: call io.AddMouseViewportEvent() with the viewport the OS mouse cursor is hovering.
    // If LinearUIBackendFlags_HasMouseHoveredViewport is not set by the backend, Dear imGui will ignore this field and infer the information using its flawed heuristic.
    // - [!] SDL backend does NOT correctly ignore viewports with the _NoInputs flag.
    //       Some backend are not able to handle that correctly. If a backend report an hovered viewport that has the _NoInputs flag (e.g. when dragging a window
    //       for docking, the viewport has the _NoInputs flag in order to allow us to find the viewport under), then Dear LinearUI is forced to ignore the value reported
    //       by the backend, and use its flawed heuristic to guess the viewport behind.
    // - [X] SDL backend correctly reports this regardless of another viewport behind focused and dragged from (we need this to find a useful drag and drop target).
    if (io.BackendFlags & LinearUIBackendFlags_HasMouseHoveredViewport)
    {
        LinearUIID mouse_viewport_id = 0;
        if (LinearUIViewport* mouse_viewport = LinearUI_ImplSDL2_GetViewportForWindowID(bd->MouseWindowID))
            mouse_viewport_id = mouse_viewport->ID;
        io.AddMouseViewportEvent(mouse_viewport_id);
    }
}

static void LinearUI_ImplSDL2_UpdateMouseCursor()
{
    LinearUIIO& io = LinearUI::GetIO();
    if (io.ConfigFlags & LinearUIConfigFlags_NoMouseCursorChange)
        return;
    LinearUI_ImplSDL2_Data* bd = LinearUI_ImplSDL2_GetBackendData();

    LinearUIMouseCursor linearui_cursor = LinearUI::GetMouseCursor();
    if (io.MouseDrawCursor || linearui_cursor == LinearUIMouseCursor_None)
    {
        // Hide OS mouse cursor if linearui is drawing it or if it wants no cursor
        SDL_ShowCursor(SDL_FALSE);
    }
    else
    {
        // Show OS mouse cursor
        SDL_Cursor* expected_cursor = bd->MouseCursors[linearui_cursor] ? bd->MouseCursors[linearui_cursor] : bd->MouseCursors[LinearUIMouseCursor_Arrow];
        if (bd->MouseLastCursor != expected_cursor)
        {
            SDL_SetCursor(expected_cursor); // SDL function doesn't have an early out (see #6113)
            bd->MouseLastCursor = expected_cursor;
        }
        SDL_ShowCursor(SDL_TRUE);
    }
}

// - On Windows the process needs to be marked DPI-aware!! SDL2 doesn't do it by default. You can call ::SetProcessDPIAware() or call LinearUI_ImplWin32_EnableDpiAwareness() from Win32 backend.
// - Apple platforms use FramebufferScale so we always return 1.0f.
// - Some accessibility applications are declaring virtual monitors with a DPI of 0.0f, see #7902. We preserve this value for caller to handle.
float LinearUI_ImplSDL2_GetContentScaleForWindow(SDL_Window* window)
{
    return LinearUI_ImplSDL2_GetContentScaleForDisplay(SDL_GetWindowDisplayIndex(window));
}

// SDL_GetDisplayDPI() seems rather unreliable on Linux.
float LinearUI_ImplSDL2_GetContentScaleForDisplay(int display_index)
{
    const char* sdl_driver = SDL_GetCurrentVideoDriver();
    if (sdl_driver && strcmp(sdl_driver, "wayland") == 0)
        return 1.0f;
#if SDL_HAS_PER_MONITOR_DPI
#if !defined(__APPLE__) && !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
    float dpi = 0.0f;
    if (SDL_GetDisplayDPI(display_index, &dpi, nullptr, nullptr) == 0)
    {
        if (dpi < 96.0f)
            dpi = 96.0f;
        return dpi / 96.0f;
    }
#endif
#endif
    LINEARUI_UNUSED(display_index);
    return 1.0f;
}

static void LinearUI_ImplSDL2_CloseGamepads()
{
    LinearUI_ImplSDL2_Data* bd = LinearUI_ImplSDL2_GetBackendData();
    if (bd->GamepadMode != LinearUI_ImplSDL2_GamepadMode_Manual)
        for (SDL_GameController* gamepad : bd->Gamepads)
            SDL_GameControllerClose(gamepad);
    bd->Gamepads.resize(0);
}

void LinearUI_ImplSDL2_SetGamepadMode(LinearUI_ImplSDL2_GamepadMode mode, struct _SDL_GameController** manual_gamepads_array, int manual_gamepads_count)
{
    LinearUI_ImplSDL2_Data* bd = LinearUI_ImplSDL2_GetBackendData();
    LinearUI_ImplSDL2_CloseGamepads();
    if (mode == LinearUI_ImplSDL2_GamepadMode_Manual)
    {
        LINEARUI_ASSERT(manual_gamepads_array != nullptr || manual_gamepads_count <= 0);
        for (int n = 0; n < manual_gamepads_count; n++)
            bd->Gamepads.push_back(manual_gamepads_array[n]);
    }
    else
    {
        LINEARUI_ASSERT(manual_gamepads_array == nullptr && manual_gamepads_count <= 0);
        bd->WantUpdateGamepadsList = true;
    }
    bd->GamepadMode = mode;
}

static void LinearUI_ImplSDL2_UpdateGamepadButton(LinearUI_ImplSDL2_Data* bd, LinearUIIO& io, LinearUIKey key, SDL_GameControllerButton button_no)
{
    bool merged_value = false;
    for (SDL_GameController* gamepad : bd->Gamepads)
        merged_value |= SDL_GameControllerGetButton(gamepad, button_no) != 0;
    io.AddKeyEvent(key, merged_value);
}

static inline float Saturate(float v) { return v < 0.0f ? 0.0f : v  > 1.0f ? 1.0f : v; }
static void LinearUI_ImplSDL2_UpdateGamepadAnalog(LinearUI_ImplSDL2_Data* bd, LinearUIIO& io, LinearUIKey key, SDL_GameControllerAxis axis_no, float v0, float v1)
{
    float merged_value = 0.0f;
    for (SDL_GameController* gamepad : bd->Gamepads)
    {
        float vn = Saturate((float)(SDL_GameControllerGetAxis(gamepad, axis_no) - v0) / (float)(v1 - v0));
        if (merged_value < vn)
            merged_value = vn;
    }
    io.AddKeyAnalogEvent(key, merged_value > 0.1f, merged_value);
}

static void LinearUI_ImplSDL2_UpdateGamepads()
{
    LinearUI_ImplSDL2_Data* bd = LinearUI_ImplSDL2_GetBackendData();
    LinearUIIO& io = LinearUI::GetIO();

    // Update list of controller(s) to use
    if (bd->WantUpdateGamepadsList && bd->GamepadMode != LinearUI_ImplSDL2_GamepadMode_Manual)
    {
        LinearUI_ImplSDL2_CloseGamepads();
        int joystick_count = SDL_NumJoysticks();
        for (int n = 0; n < joystick_count; n++)
            if (SDL_IsGameController(n))
                if (SDL_GameController* gamepad = SDL_GameControllerOpen(n))
                {
                    bd->Gamepads.push_back(gamepad);
                    if (bd->GamepadMode == LinearUI_ImplSDL2_GamepadMode_AutoFirst)
                        break;
                }
        bd->WantUpdateGamepadsList = false;
    }

    io.BackendFlags &= ~LinearUIBackendFlags_HasGamepad;
    if (bd->Gamepads.Size == 0)
        return;
    io.BackendFlags |= LinearUIBackendFlags_HasGamepad;

    // Update gamepad inputs
    const int thumb_dead_zone = 8000; // SDL_gamecontroller.h suggests using this value.
    LinearUI_ImplSDL2_UpdateGamepadButton(bd, io, LinearUIKey_GamepadStart,       SDL_CONTROLLER_BUTTON_START);
    LinearUI_ImplSDL2_UpdateGamepadButton(bd, io, LinearUIKey_GamepadBack,        SDL_CONTROLLER_BUTTON_BACK);
    LinearUI_ImplSDL2_UpdateGamepadButton(bd, io, LinearUIKey_GamepadFaceLeft,    SDL_CONTROLLER_BUTTON_X);              // Xbox X, PS Square
    LinearUI_ImplSDL2_UpdateGamepadButton(bd, io, LinearUIKey_GamepadFaceRight,   SDL_CONTROLLER_BUTTON_B);              // Xbox B, PS Circle
    LinearUI_ImplSDL2_UpdateGamepadButton(bd, io, LinearUIKey_GamepadFaceUp,      SDL_CONTROLLER_BUTTON_Y);              // Xbox Y, PS Triangle
    LinearUI_ImplSDL2_UpdateGamepadButton(bd, io, LinearUIKey_GamepadFaceDown,    SDL_CONTROLLER_BUTTON_A);              // Xbox A, PS Cross
    LinearUI_ImplSDL2_UpdateGamepadButton(bd, io, LinearUIKey_GamepadDpadLeft,    SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    LinearUI_ImplSDL2_UpdateGamepadButton(bd, io, LinearUIKey_GamepadDpadRight,   SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    LinearUI_ImplSDL2_UpdateGamepadButton(bd, io, LinearUIKey_GamepadDpadUp,      SDL_CONTROLLER_BUTTON_DPAD_UP);
    LinearUI_ImplSDL2_UpdateGamepadButton(bd, io, LinearUIKey_GamepadDpadDown,    SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    LinearUI_ImplSDL2_UpdateGamepadButton(bd, io, LinearUIKey_GamepadL1,          SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    LinearUI_ImplSDL2_UpdateGamepadButton(bd, io, LinearUIKey_GamepadR1,          SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    LinearUI_ImplSDL2_UpdateGamepadAnalog(bd, io, LinearUIKey_GamepadL2,          SDL_CONTROLLER_AXIS_TRIGGERLEFT,  0.0f, 32767);
    LinearUI_ImplSDL2_UpdateGamepadAnalog(bd, io, LinearUIKey_GamepadR2,          SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 0.0f, 32767);
    LinearUI_ImplSDL2_UpdateGamepadButton(bd, io, LinearUIKey_GamepadL3,          SDL_CONTROLLER_BUTTON_LEFTSTICK);
    LinearUI_ImplSDL2_UpdateGamepadButton(bd, io, LinearUIKey_GamepadR3,          SDL_CONTROLLER_BUTTON_RIGHTSTICK);
    LinearUI_ImplSDL2_UpdateGamepadAnalog(bd, io, LinearUIKey_GamepadLStickLeft,  SDL_CONTROLLER_AXIS_LEFTX,  -thumb_dead_zone, -32768);
    LinearUI_ImplSDL2_UpdateGamepadAnalog(bd, io, LinearUIKey_GamepadLStickRight, SDL_CONTROLLER_AXIS_LEFTX,  +thumb_dead_zone, +32767);
    LinearUI_ImplSDL2_UpdateGamepadAnalog(bd, io, LinearUIKey_GamepadLStickUp,    SDL_CONTROLLER_AXIS_LEFTY,  -thumb_dead_zone, -32768);
    LinearUI_ImplSDL2_UpdateGamepadAnalog(bd, io, LinearUIKey_GamepadLStickDown,  SDL_CONTROLLER_AXIS_LEFTY,  +thumb_dead_zone, +32767);
    LinearUI_ImplSDL2_UpdateGamepadAnalog(bd, io, LinearUIKey_GamepadRStickLeft,  SDL_CONTROLLER_AXIS_RIGHTX, -thumb_dead_zone, -32768);
    LinearUI_ImplSDL2_UpdateGamepadAnalog(bd, io, LinearUIKey_GamepadRStickRight, SDL_CONTROLLER_AXIS_RIGHTX, +thumb_dead_zone, +32767);
    LinearUI_ImplSDL2_UpdateGamepadAnalog(bd, io, LinearUIKey_GamepadRStickUp,    SDL_CONTROLLER_AXIS_RIGHTY, -thumb_dead_zone, -32768);
    LinearUI_ImplSDL2_UpdateGamepadAnalog(bd, io, LinearUIKey_GamepadRStickDown,  SDL_CONTROLLER_AXIS_RIGHTY, +thumb_dead_zone, +32767);
}

// FIXME: Note that doesn't update with DPI/Scaling change only as SDL2 doesn't have an event for it (SDL3 has).
static void LinearUI_ImplSDL2_UpdateMonitors()
{
    LinearUI_ImplSDL2_Data* bd = LinearUI_ImplSDL2_GetBackendData();
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
    platform_io.Monitors.resize(0);
    bd->WantUpdateMonitors = false;
    int display_count = SDL_GetNumVideoDisplays();
    for (int n = 0; n < display_count; n++)
    {
        // Warning: the validity of monitor DPI information on Windows depends on the application DPI awareness settings, which generally needs to be set in the manifest or at runtime.
        LinearUIPlatformMonitor monitor;
        SDL_Rect r;
        SDL_GetDisplayBounds(n, &r);
        monitor.MainPos = monitor.WorkPos = LinearVec2((float)r.x, (float)r.y);
        monitor.MainSize = monitor.WorkSize = LinearVec2((float)r.w, (float)r.h);
#if SDL_HAS_USABLE_DISPLAY_BOUNDS
        if (SDL_GetDisplayUsableBounds(n, &r) == 0 && r.w > 0 && r.h > 0)
        {
            monitor.WorkPos = LinearVec2((float)r.x, (float)r.y);
            monitor.WorkSize = LinearVec2((float)r.w, (float)r.h);
        }
#endif
        float dpi_scale = LinearUI_ImplSDL2_GetContentScaleForDisplay(n);
        if (dpi_scale <= 0.0f)
            continue; // Some accessibility applications are declaring virtual monitors with a DPI of 0, see #7902.
        monitor.DpiScale = dpi_scale;
        monitor.PlatformHandle = (void*)(intptr_t)n;
        platform_io.Monitors.push_back(monitor);
    }
}

static void LinearUI_ImplSDL2_GetWindowSizeAndFramebufferScale(SDL_Window* window, SDL_Renderer* renderer, LinearVec2* out_size, LinearVec2* out_framebuffer_scale)
{
    int w, h;
    int display_w, display_h;
    SDL_GetWindowSize(window, &w, &h);
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        w = h = 0;
    if (renderer != nullptr)
        SDL_GetRendererOutputSize(renderer, &display_w, &display_h);
#if SDL_HAS_VULKAN
    else if (SDL_GetWindowFlags(window) & SDL_WINDOW_VULKAN)
        SDL_Vulkan_GetDrawableSize(window, &display_w, &display_h);
#endif
    else
        SDL_GL_GetDrawableSize(window, &display_w, &display_h);
    if (out_size != nullptr)
        *out_size = LinearVec2((float)w, (float)h);
    if (out_framebuffer_scale != nullptr)
        *out_framebuffer_scale = (w > 0 && h > 0) ? LinearVec2((float)display_w / (float)w, (float)display_h / (float)h) : LinearVec2(1.0f, 1.0f);
}

void LinearUI_ImplSDL2_NewFrame()
{
    LinearUI_ImplSDL2_Data* bd = LinearUI_ImplSDL2_GetBackendData();
    LINEARUI_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call LinearUI_ImplSDL2_Init()?");
    LinearUIIO& io = LinearUI::GetIO();

    // Setup main viewport size (every frame to accommodate for window resizing)
    LinearUI_ImplSDL2_GetWindowSizeAndFramebufferScale(bd->Window, bd->Renderer, &io.DisplaySize, &io.DisplayFramebufferScale);

    // Update monitors
#ifdef WIN32
    bd->WantUpdateMonitors = true; // Keep polling under Windows to handle changes of work area when resizing task-bar (#8415)
#endif
    if (bd->WantUpdateMonitors)
        LinearUI_ImplSDL2_UpdateMonitors();

    // Setup time step (we don't use SDL_GetTicks() because it is using millisecond resolution)
    // (Accept SDL_GetPerformanceCounter() not returning a monotonically increasing value. Happens in VMs and Emscripten, see #6189, #6114, #3644)
    static Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 current_time = SDL_GetPerformanceCounter();
    if (current_time <= bd->Time)
        current_time = bd->Time + 1;
    io.DeltaTime = bd->Time > 0 ? (float)((double)(current_time - bd->Time) / (double)frequency) : (float)(1.0f / 60.0f);
    bd->Time = current_time;

    if (bd->MouseLastLeaveFrame && bd->MouseLastLeaveFrame >= LinearUI::GetFrameCount() && bd->MouseButtonsDown == 0)
    {
        bd->MouseWindowID = 0;
        bd->MouseLastLeaveFrame = 0;
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }

    // Our io.AddMouseViewportEvent() calls will only be valid when not capturing.
    // Technically speaking testing for 'bd->MouseButtonsDown == 0' would be more rigorous, but testing for payload reduces noise and potential side-effects.
    if (bd->MouseCanReportHoveredViewport && LinearUI::GetDragDropPayload() == nullptr)
        io.BackendFlags |= LinearUIBackendFlags_HasMouseHoveredViewport;
    else
        io.BackendFlags &= ~LinearUIBackendFlags_HasMouseHoveredViewport;

    LinearUI_ImplSDL2_UpdateMouseData();
    LinearUI_ImplSDL2_UpdateMouseCursor();

    // Update game controllers (if enabled and available)
    LinearUI_ImplSDL2_UpdateGamepads();
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously.
// If you are new to dear linearui or creating a new binding for dear linearui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

// Helper structure we store in the void* PlatformUserData field of each LinearUIViewport to easily retrieve our backend data.
struct LinearUI_ImplSDL2_ViewportData
{
    SDL_Window*     Window;
    Uint32          WindowID;       // Stored in LinearUIViewport::PlatformHandle. Use SDL_GetWindowFromID() to get SDL_Window* from Uint32 WindowID.
    bool            WindowOwned;
    SDL_GLContext   GLContext;

    LinearUI_ImplSDL2_ViewportData()   { Window = nullptr; WindowID = 0; WindowOwned = false; GLContext = nullptr; }
    ~LinearUI_ImplSDL2_ViewportData()  { LINEARUI_ASSERT(Window == nullptr && GLContext == nullptr); }
};

static void LinearUI_ImplSDL2_CreateWindow(LinearUIViewport* viewport)
{
    LinearUI_ImplSDL2_Data* bd = LinearUI_ImplSDL2_GetBackendData();
    LinearUI_ImplSDL2_ViewportData* vd = LINEARUI_NEW(LinearUI_ImplSDL2_ViewportData)();
    viewport->PlatformUserData = vd;

    LinearUIViewport* main_viewport = LinearUI::GetMainViewport();
    LinearUI_ImplSDL2_ViewportData* main_viewport_data = (LinearUI_ImplSDL2_ViewportData*)main_viewport->PlatformUserData;

    // Share GL resources with main context
    bool use_opengl = (main_viewport_data->GLContext != nullptr);
    SDL_GLContext backup_context = nullptr;
    if (use_opengl)
    {
        backup_context = SDL_GL_GetCurrentContext();
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
        SDL_GL_MakeCurrent(main_viewport_data->Window, main_viewport_data->GLContext);
    }

    Uint32 sdl_flags = 0;
    sdl_flags |= use_opengl ? SDL_WINDOW_OPENGL : (bd->UseVulkan ? SDL_WINDOW_VULKAN : 0);
    sdl_flags |= SDL_GetWindowFlags(bd->Window) & SDL_WINDOW_ALLOW_HIGHDPI;
    sdl_flags |= SDL_WINDOW_HIDDEN;
    sdl_flags |= (viewport->Flags & LinearUIViewportFlags_NoDecoration) ? SDL_WINDOW_BORDERLESS : 0;
    sdl_flags |= (viewport->Flags & LinearUIViewportFlags_NoDecoration) ? 0 : SDL_WINDOW_RESIZABLE;
#if !defined(_WIN32)
    // See SDL hack in LinearUI_ImplSDL2_ShowWindow().
    sdl_flags |= (viewport->Flags & LinearUIViewportFlags_NoTaskBarIcon) ? SDL_WINDOW_SKIP_TASKBAR : 0;
#endif
#if SDL_HAS_ALWAYS_ON_TOP
    sdl_flags |= (viewport->Flags & LinearUIViewportFlags_TopMost) ? SDL_WINDOW_ALWAYS_ON_TOP : 0;
#endif
    vd->Window = SDL_CreateWindow("No Title Yet", (int)viewport->Pos.x, (int)viewport->Pos.y, (int)viewport->Size.x, (int)viewport->Size.y, sdl_flags);
    vd->WindowOwned = true;
    if (use_opengl)
    {
        vd->GLContext = SDL_GL_CreateContext(vd->Window);
        SDL_GL_SetSwapInterval(0);
    }
    if (use_opengl && backup_context)
        SDL_GL_MakeCurrent(vd->Window, backup_context);

    viewport->PlatformHandle = (void*)(intptr_t)SDL_GetWindowID(vd->Window);
    viewport->PlatformHandleRaw = nullptr;
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(vd->Window, &info))
    {
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
        viewport->PlatformHandleRaw = info.info.win.window;
#elif defined(__APPLE__) && defined(SDL_VIDEO_DRIVER_COCOA)
        viewport->PlatformHandleRaw = (void*)info.info.cocoa.window;
#endif
    }
}

static void LinearUI_ImplSDL2_DestroyWindow(LinearUIViewport* viewport)
{
    if (LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData)
    {
        if (vd->GLContext && vd->WindowOwned)
            SDL_GL_DeleteContext(vd->GLContext);
        if (vd->Window && vd->WindowOwned)
            SDL_DestroyWindow(vd->Window);
        vd->GLContext = nullptr;
        vd->Window = nullptr;
        LINEARUI_DELETE(vd);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

static void LinearUI_ImplSDL2_ShowWindow(LinearUIViewport* viewport)
{
    LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData;
#if defined(_WIN32) && !(defined(WINAPI_FAMILY) && ((defined(WINAPI_FAMILY_APP) && WINAPI_FAMILY == WINAPI_FAMILY_APP) || (defined(WINAPI_FAMILY_GAMES) && WINAPI_FAMILY == WINAPI_FAMILY_GAMES)))
    HWND hwnd = (HWND)viewport->PlatformHandleRaw;

    // SDL hack: Hide icon from task bar
    // Note: SDL 2.0.6+ has a SDL_WINDOW_SKIP_TASKBAR flag which is supported under Windows but the way it create the window breaks our seamless transition.
    if (viewport->Flags & LinearUIViewportFlags_NoTaskBarIcon)
    {
        LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        ex_style &= ~WS_EX_APPWINDOW;
        ex_style |= WS_EX_TOOLWINDOW;
        ::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
    }
#endif

#if SDL_HAS_SHOW_WINDOW_ACTIVATION_HINT
    SDL_SetHint(SDL_HINT_WINDOW_NO_ACTIVATION_WHEN_SHOWN, (viewport->Flags & LinearUIViewportFlags_NoFocusOnAppearing) ? "1" : "0");
#elif defined(_WIN32)
    // SDL hack: SDL always activate/focus windows :/
    if (viewport->Flags & LinearUIViewportFlags_NoFocusOnAppearing)
    {
        ::ShowWindow(hwnd, SW_SHOWNA);
        return;
    }
#endif
    SDL_ShowWindow(vd->Window);
}

static LinearVec2 LinearUI_ImplSDL2_GetWindowPos(LinearUIViewport* viewport)
{
    LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData;
    int x = 0, y = 0;
    SDL_GetWindowPosition(vd->Window, &x, &y);
    return LinearVec2((float)x, (float)y);
}

static void LinearUI_ImplSDL2_SetWindowPos(LinearUIViewport* viewport, LinearVec2 pos)
{
    LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData;
    SDL_SetWindowPosition(vd->Window, (int)pos.x, (int)pos.y);
}

static LinearVec2 LinearUI_ImplSDL2_GetWindowSize(LinearUIViewport* viewport)
{
    LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData;
    int w = 0, h = 0;
    SDL_GetWindowSize(vd->Window, &w, &h);
    return LinearVec2((float)w, (float)h);
}

static void LinearUI_ImplSDL2_SetWindowSize(LinearUIViewport* viewport, LinearVec2 size)
{
    LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData;
    SDL_SetWindowSize(vd->Window, (int)size.x, (int)size.y);
}

static LinearVec2 LinearUI_ImplSDL2_GetWindowFramebufferScale(LinearUIViewport* viewport)
{
    // FIXME: SDL_Renderer does not support multi-viewport.
    LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData;
    LinearVec2 framebuffer_scale;
    LinearUI_ImplSDL2_GetWindowSizeAndFramebufferScale(vd->Window, nullptr, nullptr, &framebuffer_scale);
    return framebuffer_scale;
}

static void LinearUI_ImplSDL2_SetWindowTitle(LinearUIViewport* viewport, const char* title)
{
    LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData;
    SDL_SetWindowTitle(vd->Window, title);
}

#if SDL_HAS_WINDOW_ALPHA
static void LinearUI_ImplSDL2_SetWindowAlpha(LinearUIViewport* viewport, float alpha)
{
    LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData;
    SDL_SetWindowOpacity(vd->Window, alpha);
}
#endif

static void LinearUI_ImplSDL2_SetWindowFocus(LinearUIViewport* viewport)
{
    LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData;
    SDL_RaiseWindow(vd->Window);
}

static bool LinearUI_ImplSDL2_GetWindowFocus(LinearUIViewport* viewport)
{
    LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData;
    return (SDL_GetWindowFlags(vd->Window) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

static bool LinearUI_ImplSDL2_GetWindowMinimized(LinearUIViewport* viewport)
{
    LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData;
    return (SDL_GetWindowFlags(vd->Window) & SDL_WINDOW_MINIMIZED) != 0;
}

static void LinearUI_ImplSDL2_RenderWindow(LinearUIViewport* viewport, void*)
{
    LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData;
    if (vd->GLContext)
        SDL_GL_MakeCurrent(vd->Window, vd->GLContext);
}

static void LinearUI_ImplSDL2_SwapBuffers(LinearUIViewport* viewport, void*)
{
    LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData;
    if (vd->GLContext)
    {
        SDL_GL_MakeCurrent(vd->Window, vd->GLContext);
        SDL_GL_SwapWindow(vd->Window);
    }
}

// Vulkan support (the Vulkan renderer needs to call a platform-side support function to create the surface)
// SDL is graceful enough to _not_ need <vulkan/vulkan.h> so we can safely include this.
#if SDL_HAS_VULKAN
#include <SDL_vulkan.h>
static int LinearUI_ImplSDL2_CreateVkSurface(LinearUIViewport* viewport, LinearU64 vk_instance, const void* vk_allocator, LinearU64* out_vk_surface)
{
    LinearUI_ImplSDL2_ViewportData* vd = (LinearUI_ImplSDL2_ViewportData*)viewport->PlatformUserData;
    (void)vk_allocator;
    SDL_bool ret = SDL_Vulkan_CreateSurface(vd->Window, (VkInstance)vk_instance, (VkSurfaceKHR*)out_vk_surface);
    return ret ? 0 : 1; // ret ? VK_SUCCESS : VK_NOT_READY
}
#endif // SDL_HAS_VULKAN

static void LinearUI_ImplSDL2_InitMultiViewportSupport(SDL_Window* window, void* sdl_gl_context)
{
    // Register platform interface (will be coupled with a renderer interface)
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
    platform_io.Platform_CreateWindow = LinearUI_ImplSDL2_CreateWindow;
    platform_io.Platform_DestroyWindow = LinearUI_ImplSDL2_DestroyWindow;
    platform_io.Platform_ShowWindow = LinearUI_ImplSDL2_ShowWindow;
    platform_io.Platform_SetWindowPos = LinearUI_ImplSDL2_SetWindowPos;
    platform_io.Platform_GetWindowPos = LinearUI_ImplSDL2_GetWindowPos;
    platform_io.Platform_SetWindowSize = LinearUI_ImplSDL2_SetWindowSize;
    platform_io.Platform_GetWindowSize = LinearUI_ImplSDL2_GetWindowSize;
    platform_io.Platform_GetWindowFramebufferScale = LinearUI_ImplSDL2_GetWindowFramebufferScale;
    platform_io.Platform_SetWindowFocus = LinearUI_ImplSDL2_SetWindowFocus;
    platform_io.Platform_GetWindowFocus = LinearUI_ImplSDL2_GetWindowFocus;
    platform_io.Platform_GetWindowMinimized = LinearUI_ImplSDL2_GetWindowMinimized;
    platform_io.Platform_SetWindowTitle = LinearUI_ImplSDL2_SetWindowTitle;
    platform_io.Platform_RenderWindow = LinearUI_ImplSDL2_RenderWindow;
    platform_io.Platform_SwapBuffers = LinearUI_ImplSDL2_SwapBuffers;
#if SDL_HAS_WINDOW_ALPHA
    platform_io.Platform_SetWindowAlpha = LinearUI_ImplSDL2_SetWindowAlpha;
#endif
#if SDL_HAS_VULKAN
    platform_io.Platform_CreateVkSurface = LinearUI_ImplSDL2_CreateVkSurface;
#endif

    // Register main window handle (which is owned by the main application, not by us)
    // This is mostly for simplicity and consistency, so that our code (e.g. mouse handling etc.) can use same logic for main and secondary viewports.
    LinearUIViewport* main_viewport = LinearUI::GetMainViewport();
    LinearUI_ImplSDL2_ViewportData* vd = LINEARUI_NEW(LinearUI_ImplSDL2_ViewportData)();
    vd->Window = window;
    vd->WindowID = SDL_GetWindowID(window);
    vd->WindowOwned = false;
    vd->GLContext = sdl_gl_context;
    main_viewport->PlatformUserData = vd;
    main_viewport->PlatformHandle = (void*)(intptr_t)vd->WindowID;
}

static void LinearUI_ImplSDL2_ShutdownMultiViewportSupport()
{
    LinearUI::DestroyPlatformWindows();
}

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef LINEARUI_DISABLE
