// dear rblxui: Platform Backend for SDL3
// This needs to be used along with a Renderer (e.g. SDL_GPU, DirectX11, OpenGL3, Vulkan..)
// (Info: SDL3 is a cross-platform general purpose library for handling windows, inputs, graphics context creation, etc.)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass RblxUIKey values to all key functions e.g. RblxUI::IsKeyPressed(RblxUIKey_Space). [Legacy SDL_SCANCODE_* values are obsolete since 1.87 and not supported since 1.91.5]
//  [X] Platform: Gamepad support.
//  [X] Platform: Mouse cursor shape and visibility (RblxUIBackendFlags_HasMouseCursors). Disable with 'io.ConfigFlags |= RblxUIConfigFlags_NoMouseCursorChange'.
//  [x] Platform: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= RblxUIConfigFlags_ViewportsEnable' -> the OS animation effect when window gets created/destroyed is problematic. SDL2 backend doesn't have issue.
// Missing features or Issues:
//  [ ] Platform: Multi-viewport: Minimized windows seems to break mouse wheel events (at least under Windows).
//  [x] Platform: IME support. Position somehow broken in SDL3 + app needs to call 'SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");' before SDL_CreateWindow()!.

// You can use unmodified rblxui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire rblxui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear RblxUI:
// - FAQ                  https://dearrblxui.com/faq
// - Getting Started      https://dearrblxui.com/getting-started
// - Documentation        https://dearrblxui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of rblxui.cpp

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2026-XX-XX: Platform: Added support for multiple windows via the RblxUIPlatformIO interface.
//  2026-03-25: [Docking] Use SDL_HAS_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED (SDL 3.4.0+) when available to avoid refreshing monitor work area every frame on Windows. (#8415)
//  2026-03-09: [Docking] Fixed an issue dated 2025/04/09 (1.92 WIP) where a refactor+merge caused RblxUIBackendFlags_HasMouseHoveredViewport to never be set, causing foreign windows to be ignored when deciding of hovered viewport. (#9284)
//  2026-02-13: Inputs: systems other than X11 are back to starting mouse capture on mouse down (reverts 2025-02-26 change). Only X11 requires waiting for a drag by default (not ideal, but a better default for X11 users). Added RblxUI_ImplSDL3_SetMouseCaptureMode() for X11 debugger users. (#3650, #6410, #9235)
//  2026-01-15: Changed GetClipboardText() handler to return nullptr on error aka clipboard contents is not text. Consistent with other backends. (#9168)
//  2025-11-05: Fixed an issue with missing characters events when an already active text field changes viewports. (#9054)
//  2025-10-22: Fixed Platform_OpenInShellFn() return value (unused in core).
//  2025-09-24: Skip using the SDL_GetGlobalMouseState() state when one of our window is hovered, as the SDL_EVENT_MOUSE_MOTION data is reliable. Fix macOS notch mouse coordinates issue in fullscreen mode + better perf on X11. (#7919, #7786)
//  2025-09-18: Call platform_io.ClearPlatformHandlers() on shutdown.
//  2025-09-15: Use SDL_GetWindowDisplayScale() on Mac to output DisplayFrameBufferScale. The function is more reliable during resolution changes e.g. going fullscreen. (#8703, #4414)
//  2025-06-27: IME: avoid calling SDL_StartTextInput() again if already active. (#8727)
//  2025-05-15: [Docking] Add Platform_GetWindowFramebufferScale() handler, to allow varying Retina display density on multiple monitors.
//  2025-05-06: [Docking] macOS: fixed secondary viewports not appearing on other monitors before of parenting.
//  2025-04-09: [Docking] Revert update monitors and work areas information every frame. Only do it on Windows. (#8415, #8558)
//  2025-04-22: IME: honor RblxUIPlatformImeData->WantTextInput as an alternative way to call SDL_StartTextInput(), without IME being necessarily visible.
//  2025-04-09: Don't attempt to call SDL_CaptureMouse() on drivers where we don't call SDL_GetGlobalMouseState(). (#8561)
//  2025-03-30: Update for SDL3 api changes: Revert SDL_GetClipboardText() memory ownership change. (#8530, #7801)
//  2025-03-21: Fill gamepad inputs and set RblxUIBackendFlags_HasGamepad regardless of RblxUIConfigFlags_NavEnableGamepad being set.
//  2025-03-10: When dealing with OEM keys, use scancodes instead of translated keycodes to choose RblxUIKey values. (#7136, #7201, #7206, #7306, #7670, #7672, #8468)
//  2025-02-26: Only start SDL_CaptureMouse() when mouse is being dragged, to mitigate issues with e.g. Linux debuggers not claiming capture back. (#6410, #3650)
//  2025-02-25: [Docking] Revert to use SDL_GetDisplayBounds() for WorkPos/WorkRect if SDL_GetDisplayUsableBounds() failed.
//  2025-02-24: Avoid calling SDL_GetGlobalMouseState() when mouse is in relative mode.
//  2025-02-21: [Docking] Update monitors and work areas information every frame, as the later may change regardless of monitor changes. (#8415)
//  2025-02-18: Added RblxUIMouseCursor_Wait and RblxUIMouseCursor_Progress mouse cursor support.
//  2025-02-10: Using SDL_OpenURL() in platform_io.Platform_OpenInShellFn handler.
//  2025-01-20: Made RblxUI_ImplSDL3_SetGamepadMode(RblxUI_ImplSDL3_GamepadMode_Manual) accept an empty array.
//  2024-10-24: Emscripten: SDL_EVENT_MOUSE_WHEEL event doesn't require dividing by 100.0f on Emscripten.
//  2024-09-11: (Docking) Added support for viewport->ParentViewportId field to support parenting at OS level. (#7973)
//  2024-09-03: Update for SDL3 api changes: SDL_GetGamepads() memory ownership revert. (#7918, #7898, #7807)
//  2024-08-22: moved some OS/backend related function pointers from RblxUIIO to RblxUIPlatformIO:
//               - io.GetClipboardTextFn    -> platform_io.Platform_GetClipboardTextFn
//               - io.SetClipboardTextFn    -> platform_io.Platform_SetClipboardTextFn
//               - io.PlatformSetImeDataFn  -> platform_io.Platform_SetImeDataFn
//  2024-08-19: Storing SDL_WindowID inside RblxUIViewport::PlatformHandle instead of SDL_Window*.
//  2024-08-19: RblxUI_ImplSDL3_ProcessEvent() now ignores events intended for other SDL windows. (#7853)
//  2024-07-22: Update for SDL3 api changes: SDL_GetGamepads() memory ownership change. (#7807)
//  2024-07-18: Update for SDL3 api changes: SDL_GetClipboardText() memory ownership change. (#7801)
//  2024-07-15: Update for SDL3 api changes: SDL_GetProperty() change to SDL_GetPointerProperty(). (#7794)
//  2024-07-02: Update for SDL3 api changes: SDLK_x renames and SDLK_KP_x removals (#7761, #7762).
//  2024-07-01: Update for SDL3 api changes: SDL_SetTextInputRect() changed to SDL_SetTextInputArea().
//  2024-06-26: Update for SDL3 api changes: SDL_StartTextInput()/SDL_StopTextInput()/SDL_SetTextInputRect() functions signatures.
//  2024-06-24: Update for SDL3 api changes: SDL_EVENT_KEY_DOWN/SDL_EVENT_KEY_UP contents.
//  2024-06-03; Update for SDL3 api changes: SDL_SYSTEM_CURSOR_ renames.
//  2024-05-15: Update for SDL3 api changes: SDLK_ renames.
//  2024-04-15: Inputs: Re-enable calling SDL_StartTextInput()/SDL_StopTextInput() as SDL3 no longer enables it by default and should play nicer with IME.
//  2024-02-13: Inputs: Fixed gamepad support. Handle gamepad disconnection. Added RblxUI_ImplSDL3_SetGamepadMode().
//  2023-11-13: Updated for recent SDL3 API changes.
//  2023-10-05: Inputs: Added support for extra RblxUIKey values: F13 to F24 function keys, app back/forward keys.
//  2023-05-04: Fixed build on Emscripten/iOS/Android. (#6391)
//  2023-04-06: Inputs: Avoid calling SDL_StartTextInput()/SDL_StopTextInput() as they don't only pertain to IME. It's unclear exactly what their relation is to IME. (#6306)
//  2023-04-04: Inputs: Added support for io.AddMouseSourceEvent() to discriminate RblxUIMouseSource_Mouse/RblxUIMouseSource_TouchScreen. (#2702)
//  2023-02-23: Accept SDL_GetPerformanceCounter() not returning a monotonically increasing value. (#6189, #6114, #3644)
//  2023-02-07: Forked "rblxui_impl_sdl2" into "rblxui_impl_sdl3". Removed version checks for old feature. Refer to rblxui_impl_sdl2.cpp for older changelog.

#include "rblxui.h"
#ifndef RBLXUI_DISABLE
#include "rblxui_impl_sdl3.h"

// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#endif

// SDL
#include <SDL3/SDL.h>
#include <stdio.h>              // for snprintf()
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) && !(defined(__APPLE__) && TARGET_OS_IOS) && !defined(__amigaos4__)
#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE    1
#else
#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE    0
#endif
#define SDL_HAS_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED     SDL_VERSION_ATLEAST(3,4,0)

// FIXME-LEGACY: remove when SDL 3.1.3 preview is released.
#ifndef SDLK_APOSTROPHE
#define SDLK_APOSTROPHE SDLK_QUOTE
#endif
#ifndef SDLK_GRAVE
#define SDLK_GRAVE SDLK_BACKQUOTE
#endif

// SDL Data
struct RblxUI_ImplSDL3_Data
{
    SDL_Window*             Window;
    SDL_WindowID            WindowID;
    SDL_Renderer*           Renderer;
    Uint64                  Time;
    char*                   ClipboardTextData;
    char                    BackendPlatformName[48];
    bool                    UseVulkan;
    bool                    WantUpdateMonitors;

    // IME handling
    SDL_Window*             ImeWindow;
    RblxUIPlatformImeData    ImeData;
    bool                    ImeDirty;

    // Mouse handling
    Uint32                  MouseWindowID;
    int                     MouseButtonsDown;
    SDL_Cursor*             MouseCursors[RblxUIMouseCursor_COUNT];
    SDL_Cursor*             MouseLastCursor;
    int                     MousePendingLeaveFrame;
    bool                    MouseCanUseGlobalState;
    bool                    MouseCanReportHoveredViewport;  // This is hard to use/unreliable on SDL so we'll set RblxUIBackendFlags_HasMouseHoveredViewport dynamically based on state.
    RblxUI_ImplSDL3_MouseCaptureMode MouseCaptureMode;

    // Gamepad handling
    RblxVector<SDL_Gamepad*>  Gamepads;
    RblxUI_ImplSDL3_GamepadMode  GamepadMode;
    bool                    WantUpdateGamepadsList;

    RblxUI_ImplSDL3_Data()   { memset((void*)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear RblxUI contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear RblxUI context + multiple windows) instead of multiple Dear RblxUI contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
// FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled when using multi-context.
static RblxUI_ImplSDL3_Data* RblxUI_ImplSDL3_GetBackendData()
{
    return RblxUI::GetCurrentContext() ? (RblxUI_ImplSDL3_Data*)RblxUI::GetIO().BackendPlatformUserData : nullptr;
}

// Forward Declarations
static void RblxUI_ImplSDL3_UpdateIme();
static void RblxUI_ImplSDL3_UpdateMonitors();
static void RblxUI_ImplSDL3_InitMultiViewportSupport(SDL_Window* window, void* sdl_gl_context);
static void RblxUI_ImplSDL3_ShutdownMultiViewportSupport();

// Functions
static const char* RblxUI_ImplSDL3_GetClipboardText(RblxUIContext*)
{
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();
    if (bd->ClipboardTextData)
        SDL_free(bd->ClipboardTextData);
    if (SDL_HasClipboardText())
        bd->ClipboardTextData = SDL_GetClipboardText();
    else
        bd->ClipboardTextData = nullptr;
    return bd->ClipboardTextData;
}

static void RblxUI_ImplSDL3_SetClipboardText(RblxUIContext*, const char* text)
{
    SDL_SetClipboardText(text);
}

static RblxUIViewport* RblxUI_ImplSDL3_GetViewportForWindowID(SDL_WindowID window_id)
{
    return RblxUI::FindViewportByPlatformHandle((void*)(intptr_t)window_id);
}

static void RblxUI_ImplSDL3_PlatformSetImeData(RblxUIContext*, RblxUIViewport*, RblxUIPlatformImeData* data)
{
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();
    bd->ImeData = *data;
    bd->ImeDirty = true;
    RblxUI_ImplSDL3_UpdateIme();
}

// We discard viewport passed via RblxUIPlatformImeData and always call SDL_StartTextInput() on SDL_GetKeyboardFocus().
static void RblxUI_ImplSDL3_UpdateIme()
{
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();
    RblxUIPlatformImeData* data = &bd->ImeData;
    SDL_Window* window = SDL_GetKeyboardFocus();

    // Stop previous input
    if ((!(data->WantVisible || data->WantTextInput) || bd->ImeWindow != window) && bd->ImeWindow != nullptr)
    {
        SDL_StopTextInput(bd->ImeWindow);
        bd->ImeWindow = nullptr;
    }
    if ((!bd->ImeDirty && bd->ImeWindow == window) || (window == nullptr))
        return;

    // Start/update current input
    bd->ImeDirty = false;
    if (data->WantVisible)
    {
        RblxVec2 viewport_pos;
        if (RblxUIViewport* viewport = RblxUI_ImplSDL3_GetViewportForWindowID(SDL_GetWindowID(window)))
            viewport_pos = viewport->Pos;
        SDL_Rect r;
        r.x = (int)(data->InputPos.x - viewport_pos.x);
        r.y = (int)(data->InputPos.y - viewport_pos.y);
        r.w = 1;
        r.h = (int)data->InputLineHeight;
        SDL_SetTextInputArea(window, &r, 0);
        bd->ImeWindow = window;
    }
    if (!SDL_TextInputActive(window) && (data->WantVisible || data->WantTextInput))
        SDL_StartTextInput(window);
}

// Not static to allow third-party code to use that if they want to (but undocumented)
RblxUIKey RblxUI_ImplSDL3_KeyEventToRblxUIKey(SDL_Keycode keycode, SDL_Scancode scancode);
RblxUIKey RblxUI_ImplSDL3_KeyEventToRblxUIKey(SDL_Keycode keycode, SDL_Scancode scancode)
{
    // Keypad doesn't have individual key values in SDL3
    switch (scancode)
    {
        case SDL_SCANCODE_KP_0: return RblxUIKey_Keypad0;
        case SDL_SCANCODE_KP_1: return RblxUIKey_Keypad1;
        case SDL_SCANCODE_KP_2: return RblxUIKey_Keypad2;
        case SDL_SCANCODE_KP_3: return RblxUIKey_Keypad3;
        case SDL_SCANCODE_KP_4: return RblxUIKey_Keypad4;
        case SDL_SCANCODE_KP_5: return RblxUIKey_Keypad5;
        case SDL_SCANCODE_KP_6: return RblxUIKey_Keypad6;
        case SDL_SCANCODE_KP_7: return RblxUIKey_Keypad7;
        case SDL_SCANCODE_KP_8: return RblxUIKey_Keypad8;
        case SDL_SCANCODE_KP_9: return RblxUIKey_Keypad9;
        case SDL_SCANCODE_KP_PERIOD: return RblxUIKey_KeypadDecimal;
        case SDL_SCANCODE_KP_DIVIDE: return RblxUIKey_KeypadDivide;
        case SDL_SCANCODE_KP_MULTIPLY: return RblxUIKey_KeypadMultiply;
        case SDL_SCANCODE_KP_MINUS: return RblxUIKey_KeypadSubtract;
        case SDL_SCANCODE_KP_PLUS: return RblxUIKey_KeypadAdd;
        case SDL_SCANCODE_KP_ENTER: return RblxUIKey_KeypadEnter;
        case SDL_SCANCODE_KP_EQUALS: return RblxUIKey_KeypadEqual;
        default: break;
    }
    switch (keycode)
    {
        case SDLK_TAB: return RblxUIKey_Tab;
        case SDLK_LEFT: return RblxUIKey_LeftArrow;
        case SDLK_RIGHT: return RblxUIKey_RightArrow;
        case SDLK_UP: return RblxUIKey_UpArrow;
        case SDLK_DOWN: return RblxUIKey_DownArrow;
        case SDLK_PAGEUP: return RblxUIKey_PageUp;
        case SDLK_PAGEDOWN: return RblxUIKey_PageDown;
        case SDLK_HOME: return RblxUIKey_Home;
        case SDLK_END: return RblxUIKey_End;
        case SDLK_INSERT: return RblxUIKey_Insert;
        case SDLK_DELETE: return RblxUIKey_Delete;
        case SDLK_BACKSPACE: return RblxUIKey_Backspace;
        case SDLK_SPACE: return RblxUIKey_Space;
        case SDLK_RETURN: return RblxUIKey_Enter;
        case SDLK_ESCAPE: return RblxUIKey_Escape;
        //case SDLK_APOSTROPHE: return RblxUIKey_Apostrophe;
        case SDLK_COMMA: return RblxUIKey_Comma;
        //case SDLK_MINUS: return RblxUIKey_Minus;
        case SDLK_PERIOD: return RblxUIKey_Period;
        //case SDLK_SLASH: return RblxUIKey_Slash;
        case SDLK_SEMICOLON: return RblxUIKey_Semicolon;
        //case SDLK_EQUALS: return RblxUIKey_Equal;
        //case SDLK_LEFTBRACKET: return RblxUIKey_LeftBracket;
        //case SDLK_BACKSLASH: return RblxUIKey_Backslash;
        //case SDLK_RIGHTBRACKET: return RblxUIKey_RightBracket;
        //case SDLK_GRAVE: return RblxUIKey_GraveAccent;
        case SDLK_CAPSLOCK: return RblxUIKey_CapsLock;
        case SDLK_SCROLLLOCK: return RblxUIKey_ScrollLock;
        case SDLK_NUMLOCKCLEAR: return RblxUIKey_NumLock;
        case SDLK_PRINTSCREEN: return RblxUIKey_PrintScreen;
        case SDLK_PAUSE: return RblxUIKey_Pause;
        case SDLK_LCTRL: return RblxUIKey_LeftCtrl;
        case SDLK_LSHIFT: return RblxUIKey_LeftShift;
        case SDLK_LALT: return RblxUIKey_LeftAlt;
        case SDLK_LGUI: return RblxUIKey_LeftSuper;
        case SDLK_RCTRL: return RblxUIKey_RightCtrl;
        case SDLK_RSHIFT: return RblxUIKey_RightShift;
        case SDLK_RALT: return RblxUIKey_RightAlt;
        case SDLK_RGUI: return RblxUIKey_RightSuper;
        case SDLK_APPLICATION: return RblxUIKey_Menu;
        case SDLK_0: return RblxUIKey_0;
        case SDLK_1: return RblxUIKey_1;
        case SDLK_2: return RblxUIKey_2;
        case SDLK_3: return RblxUIKey_3;
        case SDLK_4: return RblxUIKey_4;
        case SDLK_5: return RblxUIKey_5;
        case SDLK_6: return RblxUIKey_6;
        case SDLK_7: return RblxUIKey_7;
        case SDLK_8: return RblxUIKey_8;
        case SDLK_9: return RblxUIKey_9;
        case SDLK_A: return RblxUIKey_A;
        case SDLK_B: return RblxUIKey_B;
        case SDLK_C: return RblxUIKey_C;
        case SDLK_D: return RblxUIKey_D;
        case SDLK_E: return RblxUIKey_E;
        case SDLK_F: return RblxUIKey_F;
        case SDLK_G: return RblxUIKey_G;
        case SDLK_H: return RblxUIKey_H;
        case SDLK_I: return RblxUIKey_I;
        case SDLK_J: return RblxUIKey_J;
        case SDLK_K: return RblxUIKey_K;
        case SDLK_L: return RblxUIKey_L;
        case SDLK_M: return RblxUIKey_M;
        case SDLK_N: return RblxUIKey_N;
        case SDLK_O: return RblxUIKey_O;
        case SDLK_P: return RblxUIKey_P;
        case SDLK_Q: return RblxUIKey_Q;
        case SDLK_R: return RblxUIKey_R;
        case SDLK_S: return RblxUIKey_S;
        case SDLK_T: return RblxUIKey_T;
        case SDLK_U: return RblxUIKey_U;
        case SDLK_V: return RblxUIKey_V;
        case SDLK_W: return RblxUIKey_W;
        case SDLK_X: return RblxUIKey_X;
        case SDLK_Y: return RblxUIKey_Y;
        case SDLK_Z: return RblxUIKey_Z;
        case SDLK_F1: return RblxUIKey_F1;
        case SDLK_F2: return RblxUIKey_F2;
        case SDLK_F3: return RblxUIKey_F3;
        case SDLK_F4: return RblxUIKey_F4;
        case SDLK_F5: return RblxUIKey_F5;
        case SDLK_F6: return RblxUIKey_F6;
        case SDLK_F7: return RblxUIKey_F7;
        case SDLK_F8: return RblxUIKey_F8;
        case SDLK_F9: return RblxUIKey_F9;
        case SDLK_F10: return RblxUIKey_F10;
        case SDLK_F11: return RblxUIKey_F11;
        case SDLK_F12: return RblxUIKey_F12;
        case SDLK_F13: return RblxUIKey_F13;
        case SDLK_F14: return RblxUIKey_F14;
        case SDLK_F15: return RblxUIKey_F15;
        case SDLK_F16: return RblxUIKey_F16;
        case SDLK_F17: return RblxUIKey_F17;
        case SDLK_F18: return RblxUIKey_F18;
        case SDLK_F19: return RblxUIKey_F19;
        case SDLK_F20: return RblxUIKey_F20;
        case SDLK_F21: return RblxUIKey_F21;
        case SDLK_F22: return RblxUIKey_F22;
        case SDLK_F23: return RblxUIKey_F23;
        case SDLK_F24: return RblxUIKey_F24;
        case SDLK_AC_BACK: return RblxUIKey_AppBack;
        case SDLK_AC_FORWARD: return RblxUIKey_AppForward;
        default: break;
    }

    // Fallback to scancode
    switch (scancode)
    {
    case SDL_SCANCODE_GRAVE: return RblxUIKey_GraveAccent;
    case SDL_SCANCODE_MINUS: return RblxUIKey_Minus;
    case SDL_SCANCODE_EQUALS: return RblxUIKey_Equal;
    case SDL_SCANCODE_LEFTBRACKET: return RblxUIKey_LeftBracket;
    case SDL_SCANCODE_RIGHTBRACKET: return RblxUIKey_RightBracket;
    case SDL_SCANCODE_NONUSBACKSLASH: return RblxUIKey_Oem102;
    case SDL_SCANCODE_BACKSLASH: return RblxUIKey_Backslash;
    case SDL_SCANCODE_SEMICOLON: return RblxUIKey_Semicolon;
    case SDL_SCANCODE_APOSTROPHE: return RblxUIKey_Apostrophe;
    case SDL_SCANCODE_COMMA: return RblxUIKey_Comma;
    case SDL_SCANCODE_PERIOD: return RblxUIKey_Period;
    case SDL_SCANCODE_SLASH: return RblxUIKey_Slash;
    default: break;
    }
    return RblxUIKey_None;
}

static void RblxUI_ImplSDL3_UpdateKeyModifiers(SDL_Keymod sdl_key_mods)
{
    RblxUIIO& io = RblxUI::GetIO();
    io.AddKeyEvent(RblxUIMod_Ctrl, (sdl_key_mods & SDL_KMOD_CTRL) != 0);
    io.AddKeyEvent(RblxUIMod_Shift, (sdl_key_mods & SDL_KMOD_SHIFT) != 0);
    io.AddKeyEvent(RblxUIMod_Alt, (sdl_key_mods & SDL_KMOD_ALT) != 0);
    io.AddKeyEvent(RblxUIMod_Super, (sdl_key_mods & SDL_KMOD_GUI) != 0);
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear rblxui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear rblxui, and hide them from your application based on those two flags.
bool RblxUI_ImplSDL3_ProcessEvent(const SDL_Event* event)
{
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();
    RBLXUI_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call RblxUI_ImplSDL3_Init()?");
    RblxUIIO& io = RblxUI::GetIO();

    switch (event->type)
    {
        case SDL_EVENT_MOUSE_MOTION:
        {
            if (RblxUI_ImplSDL3_GetViewportForWindowID(event->motion.windowID) == nullptr)
                return false;
            RblxVec2 mouse_pos((float)event->motion.x, (float)event->motion.y);
            if (io.ConfigFlags & RblxUIConfigFlags_ViewportsEnable)
            {
                int window_x, window_y;
                SDL_GetWindowPosition(SDL_GetWindowFromID(event->motion.windowID), &window_x, &window_y);
                mouse_pos.x += window_x;
                mouse_pos.y += window_y;
            }
            io.AddMouseSourceEvent(event->motion.which == SDL_TOUCH_MOUSEID ? RblxUIMouseSource_TouchScreen : RblxUIMouseSource_Mouse);
            io.AddMousePosEvent(mouse_pos.x, mouse_pos.y);
            return true;
        }
        case SDL_EVENT_MOUSE_WHEEL:
        {
            if (RblxUI_ImplSDL3_GetViewportForWindowID(event->wheel.windowID) == nullptr)
                return false;
            //RBLXUI_DEBUG_LOG("wheel %.2f %.2f, precise %.2f %.2f\n", (float)event->wheel.x, (float)event->wheel.y, event->wheel.preciseX, event->wheel.preciseY);
            float wheel_x = -event->wheel.x;
            float wheel_y = event->wheel.y;
            io.AddMouseSourceEvent(event->wheel.which == SDL_TOUCH_MOUSEID ? RblxUIMouseSource_TouchScreen : RblxUIMouseSource_Mouse);
            io.AddMouseWheelEvent(wheel_x, wheel_y);
            return true;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            if (RblxUI_ImplSDL3_GetViewportForWindowID(event->button.windowID) == nullptr)
                return false;
            int mouse_button = -1;
            if (event->button.button == SDL_BUTTON_LEFT) { mouse_button = 0; }
            if (event->button.button == SDL_BUTTON_RIGHT) { mouse_button = 1; }
            if (event->button.button == SDL_BUTTON_MIDDLE) { mouse_button = 2; }
            if (event->button.button == SDL_BUTTON_X1) { mouse_button = 3; }
            if (event->button.button == SDL_BUTTON_X2) { mouse_button = 4; }
            if (mouse_button == -1)
                break;
            io.AddMouseSourceEvent(event->button.which == SDL_TOUCH_MOUSEID ? RblxUIMouseSource_TouchScreen : RblxUIMouseSource_Mouse);
            io.AddMouseButtonEvent(mouse_button, (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN));
            bd->MouseButtonsDown = (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? (bd->MouseButtonsDown | (1 << mouse_button)) : (bd->MouseButtonsDown & ~(1 << mouse_button));
            return true;
        }
        case SDL_EVENT_TEXT_INPUT:
        {
            if (RblxUI_ImplSDL3_GetViewportForWindowID(event->text.windowID) == nullptr)
                return false;
            io.AddInputCharactersUTF8(event->text.text);
            return true;
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        {
            RblxUIViewport* viewport = RblxUI_ImplSDL3_GetViewportForWindowID(event->key.windowID);
            if (viewport == nullptr)
                return false;
            //RBLXUI_DEBUG_LOG("SDL_EVENT_KEY_%s : key=0x%08X ('%s'), scancode=%d ('%s'), mod=%X, windowID=%d, viewport=%08X\n",
            //    (event->type == SDL_EVENT_KEY_DOWN) ? "DOWN" : "UP  ", event->key.key, SDL_GetKeyName(event->key.key), event->key.scancode, SDL_GetScancodeName(event->key.scancode), event->key.mod, event->key.windowID, viewport ? viewport->ID : 0);
            RblxUI_ImplSDL3_UpdateKeyModifiers((SDL_Keymod)event->key.mod);
            RblxUIKey key = RblxUI_ImplSDL3_KeyEventToRblxUIKey(event->key.key, event->key.scancode);
            io.AddKeyEvent(key, (event->type == SDL_EVENT_KEY_DOWN));
            io.SetKeyEventNativeData(key, (int)event->key.key, (int)event->key.scancode, (int)event->key.scancode); // To support legacy indexing (<1.87 user code). Legacy backend uses SDLK_*** as indices to IsKeyXXX() functions.
            return true;
        }
        case SDL_EVENT_DISPLAY_ORIENTATION:
        case SDL_EVENT_DISPLAY_ADDED:
        case SDL_EVENT_DISPLAY_REMOVED:
        case SDL_EVENT_DISPLAY_MOVED:
        case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED:
#if SDL_HAS_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED
        case SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED:
#endif
        {
            bd->WantUpdateMonitors = true;
            return true;
        }
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
        {
            if (RblxUI_ImplSDL3_GetViewportForWindowID(event->window.windowID) == nullptr)
                return false;
            bd->MouseWindowID = event->window.windowID;
            bd->MousePendingLeaveFrame = 0;
            return true;
        }
        // - In some cases, when detaching a window from main viewport SDL may send SDL_WINDOWEVENT_ENTER one frame too late,
        //   causing SDL_WINDOWEVENT_LEAVE on previous frame to interrupt drag operation by clear mouse position. This is why
        //   we delay process the SDL_WINDOWEVENT_LEAVE events by one frame. See issue #5012 for details.
        // FIXME: Unconfirmed whether this is still needed with SDL3.
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        {
            if (RblxUI_ImplSDL3_GetViewportForWindowID(event->window.windowID) == nullptr)
                return false;
            bd->MousePendingLeaveFrame = RblxUI::GetFrameCount() + 1;
            return true;
        }
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
        {
            RblxUIViewport* viewport = RblxUI_ImplSDL3_GetViewportForWindowID(event->window.windowID);
            if (viewport == nullptr)
                return false;
            //RBLXUI_DEBUG_LOG("%s: windowId %d, viewport: %08X\n", (event->type == SDL_EVENT_WINDOW_FOCUS_GAINED) ? "SDL_EVENT_WINDOW_FOCUS_GAINED" : "SDL_WINDOWEVENT_FOCUS_LOST", event->window.windowID, viewport ? viewport->ID : 0);
            io.AddFocusEvent(event->type == SDL_EVENT_WINDOW_FOCUS_GAINED);
            return true;
        }
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        case SDL_EVENT_WINDOW_MOVED:
        case SDL_EVENT_WINDOW_RESIZED:
        {
            RblxUIViewport* viewport = RblxUI_ImplSDL3_GetViewportForWindowID(event->window.windowID);
            if (viewport == NULL)
                return false;
            if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
                viewport->PlatformRequestClose = true;
            if (event->type == SDL_EVENT_WINDOW_MOVED)
                viewport->PlatformRequestMove = true;
            if (event->type == SDL_EVENT_WINDOW_RESIZED)
                viewport->PlatformRequestResize = true;
            return true;
        }
        case SDL_EVENT_GAMEPAD_ADDED:
        case SDL_EVENT_GAMEPAD_REMOVED:
        {
            bd->WantUpdateGamepadsList = true;
            return true;
        }
        default:
            break;
    }
    return false;
}

static void RblxUI_ImplSDL3_SetupPlatformHandles(RblxUIViewport* viewport, SDL_Window* window)
{
    viewport->PlatformHandle = (void*)(intptr_t)SDL_GetWindowID(window);
    viewport->PlatformHandleRaw = nullptr;
#if defined(_WIN32) && !defined(__WINRT__)
    viewport->PlatformHandleRaw = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif defined(__APPLE__)
    viewport->PlatformHandleRaw = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#endif
}

static bool RblxUI_ImplSDL3_Init(SDL_Window* window, SDL_Renderer* renderer, void* sdl_gl_context)
{
    RblxUIIO& io = RblxUI::GetIO();
    RBLXUI_CHECKVERSION();
    RBLXUI_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");
    RBLXUI_UNUSED(sdl_gl_context); // Unused in this branch
    //SDL_SetHint(SDL_HINT_EVENT_LOGGING, "2");

    const int ver_linked = SDL_GetVersion();

    // Setup backend capabilities flags
    RblxUI_ImplSDL3_Data* bd = RBLXUI_NEW(RblxUI_ImplSDL3_Data)();
    snprintf(bd->BackendPlatformName, sizeof(bd->BackendPlatformName), "rblxui_impl_sdl3 (%d.%d.%d; %d.%d.%d)",
        SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION, SDL_VERSIONNUM_MAJOR(ver_linked), SDL_VERSIONNUM_MINOR(ver_linked), SDL_VERSIONNUM_MICRO(ver_linked));
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = bd->BackendPlatformName;
    io.BackendFlags |= RblxUIBackendFlags_HasMouseCursors;           // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= RblxUIBackendFlags_HasSetMousePos;            // We can honor io.WantSetMousePos requests (optional, rarely used)
    // (RblxUIBackendFlags_PlatformHasViewports and RblxUIBackendFlags_HasParentViewport may be set just below)
    // (RblxUIBackendFlags_HasMouseHoveredViewport is set dynamically in our _NewFrame function)

    bd->Window = window;
    bd->WindowID = SDL_GetWindowID(window);
    bd->Renderer = renderer;

    // Check and store if we are on a SDL backend that supports SDL_GetGlobalMouseState() and SDL_CaptureMouse()
    // ("wayland" and "rpi" don't support it, but we chose to use a white-list instead of a black-list)
    bd->MouseCanUseGlobalState = false;
    bd->MouseCaptureMode = RblxUI_ImplSDL3_MouseCaptureMode_Disabled;
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
    const char* sdl_backend = SDL_GetCurrentVideoDriver();
    const char* capture_and_global_state_whitelist[] = { "windows", "cocoa", "x11", "DIVE", "VMAN" };
    for (const char* item : capture_and_global_state_whitelist)
        if (strncmp(sdl_backend, item, strlen(item)) == 0)
        {
            bd->MouseCanUseGlobalState = true;
            bd->MouseCaptureMode = (strcmp(item, "x11") == 0) ? RblxUI_ImplSDL3_MouseCaptureMode_EnabledAfterDrag : RblxUI_ImplSDL3_MouseCaptureMode_Enabled;
        }
#endif
    if (bd->MouseCanUseGlobalState)
    {
        io.BackendFlags |= RblxUIBackendFlags_PlatformHasViewports;  // We can create multi-viewports on the Platform side (optional)
        io.BackendFlags |= RblxUIBackendFlags_HasParentViewport;     // We can honor viewport->ParentViewportId by applying the corresponding parent/child relationship at platform level (optional)
    }

    // SDL on Linux/OSX doesn't report events for unfocused windows (see https://github.com/ocornut/rblxui/issues/4960)
    // We will use 'MouseCanReportHoveredViewport' to set 'RblxUIBackendFlags_HasMouseHoveredViewport' dynamically each frame.
#ifndef __APPLE__
    bd->MouseCanReportHoveredViewport = bd->MouseCanUseGlobalState;
#else
    bd->MouseCanReportHoveredViewport = false;
#endif

    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    platform_io.Platform_SetClipboardTextFn = RblxUI_ImplSDL3_SetClipboardText;
    platform_io.Platform_GetClipboardTextFn = RblxUI_ImplSDL3_GetClipboardText;
    platform_io.Platform_SetImeDataFn = RblxUI_ImplSDL3_PlatformSetImeData;
    platform_io.Platform_OpenInShellFn = [](RblxUIContext*, const char* url) { return SDL_OpenURL(url); };

    // Update monitor a first time during init
    RblxUI_ImplSDL3_UpdateMonitors();

    // Gamepad handling
    bd->GamepadMode = RblxUI_ImplSDL3_GamepadMode_AutoFirst;
    bd->WantUpdateGamepadsList = true;

    // Load mouse cursors
    bd->MouseCursors[RblxUIMouseCursor_Arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    bd->MouseCursors[RblxUIMouseCursor_TextInput] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
    bd->MouseCursors[RblxUIMouseCursor_ResizeAll] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
    bd->MouseCursors[RblxUIMouseCursor_ResizeNS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);
    bd->MouseCursors[RblxUIMouseCursor_ResizeEW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
    bd->MouseCursors[RblxUIMouseCursor_ResizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NESW_RESIZE);
    bd->MouseCursors[RblxUIMouseCursor_ResizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NWSE_RESIZE);
    bd->MouseCursors[RblxUIMouseCursor_Hand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
    bd->MouseCursors[RblxUIMouseCursor_Wait] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
    bd->MouseCursors[RblxUIMouseCursor_Progress] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_PROGRESS);
    bd->MouseCursors[RblxUIMouseCursor_NotAllowed] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NOT_ALLOWED);

    // Set platform dependent data in viewport
    // Our mouse update function expect PlatformHandle to be filled for the main viewport
    RblxUIViewport* main_viewport = RblxUI::GetMainViewport();
    RblxUI_ImplSDL3_SetupPlatformHandles(main_viewport, window);

    // From 2.0.5: Set SDL hint to receive mouse click events on window focus, otherwise SDL doesn't emit the event.
    // Without this, when clicking to gain focus, our widgets wouldn't activate even though they showed as hovered.
    // (This is unfortunately a global SDL setting, so enabling it might have a side-effect on your application.
    // It is unlikely to make a difference, but if your app absolutely needs to ignore the initial on-focus click:
    // you can ignore SDL_EVENT_MOUSE_BUTTON_DOWN events coming right after a SDL_EVENT_WINDOW_FOCUS_GAINED)
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

    // From 2.0.22: Disable auto-capture, this is preventing drag and drop across multiple windows (see #5710)
    SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "0");

    // SDL 3.x : see https://github.com/libsdl-org/SDL/issues/6659
    SDL_SetHint("SDL_BORDERLESS_WINDOWED_STYLE", "0");

    // We need SDL_CaptureMouse(), SDL_GetGlobalMouseState() from SDL 2.0.4+ to support multiple viewports.
    // We left the call to RblxUI_ImplSDL3_InitPlatformInterface() outside of #ifdef to avoid unused-function warnings.
    if (io.BackendFlags & RblxUIBackendFlags_PlatformHasViewports)
        RblxUI_ImplSDL3_InitMultiViewportSupport(window, sdl_gl_context);

    return true;
}

// Should technically be a SDL_GLContext but due to typedef it is sane to keep it void* in public interface.
bool RblxUI_ImplSDL3_InitForOpenGL(SDL_Window* window, void* sdl_gl_context)
{
    return RblxUI_ImplSDL3_Init(window, nullptr, sdl_gl_context);
}

bool RblxUI_ImplSDL3_InitForVulkan(SDL_Window* window)
{
    if (!RblxUI_ImplSDL3_Init(window, nullptr, nullptr))
        return false;
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();
    bd->UseVulkan = true;
    return true;
}

bool RblxUI_ImplSDL3_InitForD3D(SDL_Window* window)
{
#if !defined(_WIN32)
    RBLXUI_ASSERT(0 && "Unsupported");
#endif
    return RblxUI_ImplSDL3_Init(window, nullptr, nullptr);
}

bool RblxUI_ImplSDL3_InitForMetal(SDL_Window* window)
{
    return RblxUI_ImplSDL3_Init(window, nullptr, nullptr);
}

bool RblxUI_ImplSDL3_InitForSDLRenderer(SDL_Window* window, SDL_Renderer* renderer)
{
    return RblxUI_ImplSDL3_Init(window, renderer, nullptr);
}

bool RblxUI_ImplSDL3_InitForSDLGPU(SDL_Window* window)
{
    return RblxUI_ImplSDL3_Init(window, nullptr, nullptr);
}

bool RblxUI_ImplSDL3_InitForOther(SDL_Window* window)
{
    return RblxUI_ImplSDL3_Init(window, nullptr, nullptr);
}

static void RblxUI_ImplSDL3_CloseGamepads();

void RblxUI_ImplSDL3_Shutdown()
{
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();
    RBLXUI_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
    RblxUIIO& io = RblxUI::GetIO();
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();

    RblxUI_ImplSDL3_ShutdownMultiViewportSupport();
    if (bd->ClipboardTextData)
        SDL_free(bd->ClipboardTextData);
    for (RblxUIMouseCursor cursor_n = 0; cursor_n < RblxUIMouseCursor_COUNT; cursor_n++)
        SDL_DestroyCursor(bd->MouseCursors[cursor_n]);
    RblxUI_ImplSDL3_CloseGamepads();

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(RblxUIBackendFlags_HasMouseCursors | RblxUIBackendFlags_HasSetMousePos | RblxUIBackendFlags_HasGamepad | RblxUIBackendFlags_PlatformHasViewports | RblxUIBackendFlags_HasMouseHoveredViewport | RblxUIBackendFlags_HasParentViewport);
    platform_io.ClearPlatformHandlers();
    RBLXUI_DELETE(bd);
}

void RblxUI_ImplSDL3_SetMouseCaptureMode(RblxUI_ImplSDL3_MouseCaptureMode mode)
{
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();
    if (mode == RblxUI_ImplSDL3_MouseCaptureMode_Disabled && bd->MouseCaptureMode != RblxUI_ImplSDL3_MouseCaptureMode_Disabled)
        SDL_CaptureMouse(false);
    bd->MouseCaptureMode = mode;
}

static void RblxUI_ImplSDL3_UpdateMouseData()
{
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();
    RblxUIIO& io = RblxUI::GetIO();

    // We forward mouse input when hovered or captured (via SDL_EVENT_MOUSE_MOTION) or when focused (below)
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
    // - SDL_CaptureMouse() let the OS know e.g. that our drags can extend outside of parent boundaries (we want updated position) and shouldn't trigger other operations outside.
    // - Debuggers under Linux tends to leave captured mouse on break, which may be very inconvenient, so to mitigate the issue on X11 we we wait until mouse has moved to begin capture.
    if (bd->MouseCaptureMode == RblxUI_ImplSDL3_MouseCaptureMode_Enabled)
    {
        SDL_CaptureMouse(bd->MouseButtonsDown != 0);
    }
    else if (bd->MouseCaptureMode == RblxUI_ImplSDL3_MouseCaptureMode_EnabledAfterDrag)
    {
        bool want_capture = false;
        for (int button_n = 0; button_n < RblxUIMouseButton_COUNT && !want_capture; button_n++)
            if (RblxUI::IsMouseDragging(button_n, 1.0f))
                want_capture = true;
        SDL_CaptureMouse(want_capture);
    }

    SDL_Window* focused_window = SDL_GetKeyboardFocus();
    const bool is_app_focused = (focused_window && (bd->Window == focused_window || RblxUI_ImplSDL3_GetViewportForWindowID(SDL_GetWindowID(focused_window)) != NULL));
#else
    SDL_Window* focused_window = bd->Window;
    const bool is_app_focused = (SDL_GetWindowFlags(bd->Window) & SDL_WINDOW_INPUT_FOCUS) != 0; // SDL 2.0.3 and non-windowed systems: single-viewport only
#endif
    if (is_app_focused)
    {
        // (Optional) Set OS mouse position from Dear RblxUI if requested (rarely used, only when io.ConfigNavMoveSetMousePos is enabled by user)
        if (io.WantSetMousePos)
        {
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
            if (io.ConfigFlags & RblxUIConfigFlags_ViewportsEnable)
                SDL_WarpMouseGlobal(io.MousePos.x, io.MousePos.y);
            else
#endif
                SDL_WarpMouseInWindow(bd->Window, io.MousePos.x, io.MousePos.y);
        }

        // (Optional) Fallback to provide unclamped mouse position when focused but not hovered (SDL_EVENT_MOUSE_MOTION already provides this when hovered or captured)
        // Note that SDL_GetGlobalMouseState() is in theory slow on X11, but this only runs on rather specific cases. If a problem we may provide a way to opt-out this feature.
        SDL_Window* hovered_window = SDL_GetMouseFocus();
        const bool is_relative_mouse_mode = SDL_GetWindowRelativeMouseMode(bd->Window);
        if (hovered_window == nullptr && bd->MouseCanUseGlobalState && bd->MouseButtonsDown == 0 && !is_relative_mouse_mode)
        {
            // Single-viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window)
            // Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
            float mouse_x, mouse_y;
            int window_x, window_y;
            SDL_GetGlobalMouseState(&mouse_x, &mouse_y);
            if (!(io.ConfigFlags & RblxUIConfigFlags_ViewportsEnable))
            {
                SDL_GetWindowPosition(focused_window, &window_x, &window_y);
                mouse_x -= (float)window_x;
                mouse_y -= (float)window_y;
            }
            io.AddMousePosEvent(mouse_x, mouse_y);
        }
    }

    // (Optional) When using multiple viewports: call io.AddMouseViewportEvent() with the viewport the OS mouse cursor is hovering.
    // If RblxUIBackendFlags_HasMouseHoveredViewport is not set by the backend, Dear imGui will ignore this field and infer the information using its flawed heuristic.
    // - [!] SDL backend does NOT correctly ignore viewports with the _NoInputs flag.
    //       Some backend are not able to handle that correctly. If a backend report an hovered viewport that has the _NoInputs flag (e.g. when dragging a window
    //       for docking, the viewport has the _NoInputs flag in order to allow us to find the viewport under), then Dear RblxUI is forced to ignore the value reported
    //       by the backend, and use its flawed heuristic to guess the viewport behind.
    // - [X] SDL backend correctly reports this regardless of another viewport behind focused and dragged from (we need this to find a useful drag and drop target).
    if (io.BackendFlags & RblxUIBackendFlags_HasMouseHoveredViewport)
    {
        RblxUIID mouse_viewport_id = 0;
        if (RblxUIViewport* mouse_viewport = RblxUI_ImplSDL3_GetViewportForWindowID(bd->MouseWindowID))
            mouse_viewport_id = mouse_viewport->ID;
        io.AddMouseViewportEvent(mouse_viewport_id);
    }
}

static void RblxUI_ImplSDL3_UpdateMouseCursor()
{
    RblxUIIO& io = RblxUI::GetIO();
    if (io.ConfigFlags & RblxUIConfigFlags_NoMouseCursorChange)
        return;
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();

    RblxUIMouseCursor rblxui_cursor = RblxUI::GetMouseCursor();
    if (io.MouseDrawCursor || rblxui_cursor == RblxUIMouseCursor_None)
    {
        // Hide OS mouse cursor if rblxui is drawing it or if it wants no cursor
        SDL_HideCursor();
    }
    else
    {
        // Show OS mouse cursor
        SDL_Cursor* expected_cursor = bd->MouseCursors[rblxui_cursor] ? bd->MouseCursors[rblxui_cursor] : bd->MouseCursors[RblxUIMouseCursor_Arrow];
        if (bd->MouseLastCursor != expected_cursor)
        {
            SDL_SetCursor(expected_cursor); // SDL function doesn't have an early out (see #6113)
            bd->MouseLastCursor = expected_cursor;
        }
        SDL_ShowCursor();
    }
}

static void RblxUI_ImplSDL3_CloseGamepads()
{
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();
    if (bd->GamepadMode != RblxUI_ImplSDL3_GamepadMode_Manual)
        for (SDL_Gamepad* gamepad : bd->Gamepads)
            SDL_CloseGamepad(gamepad);
    bd->Gamepads.resize(0);
}

void RblxUI_ImplSDL3_SetGamepadMode(RblxUI_ImplSDL3_GamepadMode mode, SDL_Gamepad** manual_gamepads_array, int manual_gamepads_count)
{
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();
    RblxUI_ImplSDL3_CloseGamepads();
    if (mode == RblxUI_ImplSDL3_GamepadMode_Manual)
    {
        RBLXUI_ASSERT(manual_gamepads_array != nullptr || manual_gamepads_count <= 0);
        for (int n = 0; n < manual_gamepads_count; n++)
            bd->Gamepads.push_back(manual_gamepads_array[n]);
    }
    else
    {
        RBLXUI_ASSERT(manual_gamepads_array == nullptr && manual_gamepads_count <= 0);
        bd->WantUpdateGamepadsList = true;
    }
    bd->GamepadMode = mode;
}

static void RblxUI_ImplSDL3_UpdateGamepadButton(RblxUI_ImplSDL3_Data* bd, RblxUIIO& io, RblxUIKey key, SDL_GamepadButton button_no)
{
    bool merged_value = false;
    for (SDL_Gamepad* gamepad : bd->Gamepads)
        merged_value |= SDL_GetGamepadButton(gamepad, button_no) != 0;
    io.AddKeyEvent(key, merged_value);
}

static inline float Saturate(float v) { return v < 0.0f ? 0.0f : v  > 1.0f ? 1.0f : v; }
static void RblxUI_ImplSDL3_UpdateGamepadAnalog(RblxUI_ImplSDL3_Data* bd, RblxUIIO& io, RblxUIKey key, SDL_GamepadAxis axis_no, float v0, float v1)
{
    float merged_value = 0.0f;
    for (SDL_Gamepad* gamepad : bd->Gamepads)
    {
        float vn = Saturate((float)(SDL_GetGamepadAxis(gamepad, axis_no) - v0) / (float)(v1 - v0));
        if (merged_value < vn)
            merged_value = vn;
    }
    io.AddKeyAnalogEvent(key, merged_value > 0.1f, merged_value);
}

static void RblxUI_ImplSDL3_UpdateGamepads()
{
    RblxUIIO& io = RblxUI::GetIO();
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();

    // Update list of gamepads to use
    if (bd->WantUpdateGamepadsList && bd->GamepadMode != RblxUI_ImplSDL3_GamepadMode_Manual)
    {
        RblxUI_ImplSDL3_CloseGamepads();
        int sdl_gamepads_count = 0;
        SDL_JoystickID* sdl_gamepads = SDL_GetGamepads(&sdl_gamepads_count);
        for (int n = 0; n < sdl_gamepads_count; n++)
            if (SDL_Gamepad* gamepad = SDL_OpenGamepad(sdl_gamepads[n]))
            {
                bd->Gamepads.push_back(gamepad);
                if (bd->GamepadMode == RblxUI_ImplSDL3_GamepadMode_AutoFirst)
                    break;
            }
        bd->WantUpdateGamepadsList = false;
        SDL_free(sdl_gamepads);
    }

    io.BackendFlags &= ~RblxUIBackendFlags_HasGamepad;
    if (bd->Gamepads.Size == 0)
        return;
    io.BackendFlags |= RblxUIBackendFlags_HasGamepad;

    // Update gamepad inputs
    const int thumb_dead_zone = 8000;           // SDL_gamepad.h suggests using this value.
    RblxUI_ImplSDL3_UpdateGamepadButton(bd, io, RblxUIKey_GamepadStart,       SDL_GAMEPAD_BUTTON_START);
    RblxUI_ImplSDL3_UpdateGamepadButton(bd, io, RblxUIKey_GamepadBack,        SDL_GAMEPAD_BUTTON_BACK);
    RblxUI_ImplSDL3_UpdateGamepadButton(bd, io, RblxUIKey_GamepadFaceLeft,    SDL_GAMEPAD_BUTTON_WEST);           // Xbox X, PS Square
    RblxUI_ImplSDL3_UpdateGamepadButton(bd, io, RblxUIKey_GamepadFaceRight,   SDL_GAMEPAD_BUTTON_EAST);           // Xbox B, PS Circle
    RblxUI_ImplSDL3_UpdateGamepadButton(bd, io, RblxUIKey_GamepadFaceUp,      SDL_GAMEPAD_BUTTON_NORTH);          // Xbox Y, PS Triangle
    RblxUI_ImplSDL3_UpdateGamepadButton(bd, io, RblxUIKey_GamepadFaceDown,    SDL_GAMEPAD_BUTTON_SOUTH);          // Xbox A, PS Cross
    RblxUI_ImplSDL3_UpdateGamepadButton(bd, io, RblxUIKey_GamepadDpadLeft,    SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    RblxUI_ImplSDL3_UpdateGamepadButton(bd, io, RblxUIKey_GamepadDpadRight,   SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
    RblxUI_ImplSDL3_UpdateGamepadButton(bd, io, RblxUIKey_GamepadDpadUp,      SDL_GAMEPAD_BUTTON_DPAD_UP);
    RblxUI_ImplSDL3_UpdateGamepadButton(bd, io, RblxUIKey_GamepadDpadDown,    SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    RblxUI_ImplSDL3_UpdateGamepadButton(bd, io, RblxUIKey_GamepadL1,          SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    RblxUI_ImplSDL3_UpdateGamepadButton(bd, io, RblxUIKey_GamepadR1,          SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    RblxUI_ImplSDL3_UpdateGamepadAnalog(bd, io, RblxUIKey_GamepadL2,          SDL_GAMEPAD_AXIS_LEFT_TRIGGER,  0.0f, 32767);
    RblxUI_ImplSDL3_UpdateGamepadAnalog(bd, io, RblxUIKey_GamepadR2,          SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 0.0f, 32767);
    RblxUI_ImplSDL3_UpdateGamepadButton(bd, io, RblxUIKey_GamepadL3,          SDL_GAMEPAD_BUTTON_LEFT_STICK);
    RblxUI_ImplSDL3_UpdateGamepadButton(bd, io, RblxUIKey_GamepadR3,          SDL_GAMEPAD_BUTTON_RIGHT_STICK);
    RblxUI_ImplSDL3_UpdateGamepadAnalog(bd, io, RblxUIKey_GamepadLStickLeft,  SDL_GAMEPAD_AXIS_LEFTX,  -thumb_dead_zone, -32768);
    RblxUI_ImplSDL3_UpdateGamepadAnalog(bd, io, RblxUIKey_GamepadLStickRight, SDL_GAMEPAD_AXIS_LEFTX,  +thumb_dead_zone, +32767);
    RblxUI_ImplSDL3_UpdateGamepadAnalog(bd, io, RblxUIKey_GamepadLStickUp,    SDL_GAMEPAD_AXIS_LEFTY,  -thumb_dead_zone, -32768);
    RblxUI_ImplSDL3_UpdateGamepadAnalog(bd, io, RblxUIKey_GamepadLStickDown,  SDL_GAMEPAD_AXIS_LEFTY,  +thumb_dead_zone, +32767);
    RblxUI_ImplSDL3_UpdateGamepadAnalog(bd, io, RblxUIKey_GamepadRStickLeft,  SDL_GAMEPAD_AXIS_RIGHTX, -thumb_dead_zone, -32768);
    RblxUI_ImplSDL3_UpdateGamepadAnalog(bd, io, RblxUIKey_GamepadRStickRight, SDL_GAMEPAD_AXIS_RIGHTX, +thumb_dead_zone, +32767);
    RblxUI_ImplSDL3_UpdateGamepadAnalog(bd, io, RblxUIKey_GamepadRStickUp,    SDL_GAMEPAD_AXIS_RIGHTY, -thumb_dead_zone, -32768);
    RblxUI_ImplSDL3_UpdateGamepadAnalog(bd, io, RblxUIKey_GamepadRStickDown,  SDL_GAMEPAD_AXIS_RIGHTY, +thumb_dead_zone, +32767);
}

static void RblxUI_ImplSDL3_UpdateMonitors()
{
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    platform_io.Monitors.resize(0);
    bd->WantUpdateMonitors = false;

    int display_count;
    SDL_DisplayID* displays = SDL_GetDisplays(&display_count);
    for (int n = 0; n < display_count; n++)
    {
        // Warning: the validity of monitor DPI information on Windows depends on the application DPI awareness settings, which generally needs to be set in the manifest or at runtime.
        SDL_DisplayID display_id = displays[n];
        RblxUIPlatformMonitor monitor;
        SDL_Rect r;
        SDL_GetDisplayBounds(display_id, &r);
        monitor.MainPos = monitor.WorkPos = RblxVec2((float)r.x, (float)r.y);
        monitor.MainSize = monitor.WorkSize = RblxVec2((float)r.w, (float)r.h);
        if (SDL_GetDisplayUsableBounds(display_id, &r) && r.w > 0 && r.h > 0)
        {
            monitor.WorkPos = RblxVec2((float)r.x, (float)r.y);
            monitor.WorkSize = RblxVec2((float)r.w, (float)r.h);
        }
        monitor.DpiScale = SDL_GetDisplayContentScale(display_id); // See https://wiki.libsdl.org/SDL3/README-highdpi for details.
        monitor.PlatformHandle = (void*)(intptr_t)n;
        if (monitor.DpiScale <= 0.0f)
            continue; // Some accessibility applications are declaring virtual monitors with a DPI of 0, see #7902.
        platform_io.Monitors.push_back(monitor);
    }
    SDL_free(displays);
}

static void RblxUI_ImplSDL3_GetWindowSizeAndFramebufferScale(SDL_Window* window, RblxVec2* out_size, RblxVec2* out_framebuffer_scale)
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        w = h = 0;

#if defined(__APPLE__)
    float fb_scale_x = SDL_GetWindowDisplayScale(window); // Seems more reliable during resolution change (#8703)
    float fb_scale_y = fb_scale_x;
#else
    int display_w, display_h;
    SDL_GetWindowSizeInPixels(window, &display_w, &display_h);
    float fb_scale_x = (w > 0) ? (float)display_w / (float)w : 1.0f;
    float fb_scale_y = (h > 0) ? (float)display_h / (float)h : 1.0f;
#endif

    if (out_size != nullptr)
        *out_size = RblxVec2((float)w, (float)h);
    if (out_framebuffer_scale != nullptr)
        *out_framebuffer_scale = RblxVec2(fb_scale_x, fb_scale_y);
}

void RblxUI_ImplSDL3_NewFrame()
{
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();
    RBLXUI_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call RblxUI_ImplSDL3_Init()?");
    RblxUIIO& io = RblxUI::GetIO();

    // Setup main viewport size (every frame to accommodate for window resizing)
    RblxUI_ImplSDL3_GetWindowSizeAndFramebufferScale(bd->Window, &io.DisplaySize, &io.DisplayFramebufferScale);

    // Update monitors
#if defined(WIN32) && !SDL_HAS_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED
    bd->WantUpdateMonitors = true; // Keep polling under Windows to handle changes of work area when resizing task-bar (#8415)
#endif
    if (bd->WantUpdateMonitors)
        RblxUI_ImplSDL3_UpdateMonitors();

    // Setup time step (we could also use SDL_GetTicksNS() available since SDL3)
    // (Accept SDL_GetPerformanceCounter() not returning a monotonically increasing value. Happens in VMs and Emscripten, see #6189, #6114, #3644)
    static Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 current_time = SDL_GetPerformanceCounter();
    if (current_time <= bd->Time)
        current_time = bd->Time + 1;
    io.DeltaTime = bd->Time > 0 ? (float)((double)(current_time - bd->Time) / (double)frequency) : (float)(1.0f / 60.0f);
    bd->Time = current_time;

    if (bd->MousePendingLeaveFrame && bd->MousePendingLeaveFrame >= RblxUI::GetFrameCount() && bd->MouseButtonsDown == 0)
    {
        bd->MouseWindowID = 0;
        bd->MousePendingLeaveFrame = 0;
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }

    // Our io.AddMouseViewportEvent() calls will only be valid when not capturing.
    // Technically speaking testing for 'bd->MouseButtonsDown == 0' would be more rigorous, but testing for payload reduces noise and potential side-effects.
    if (bd->MouseCanReportHoveredViewport && RblxUI::GetDragDropPayload() == nullptr)
        io.BackendFlags |= RblxUIBackendFlags_HasMouseHoveredViewport;
    else
        io.BackendFlags &= ~RblxUIBackendFlags_HasMouseHoveredViewport;

    RblxUI_ImplSDL3_UpdateMouseData();
    RblxUI_ImplSDL3_UpdateMouseCursor();
    RblxUI_ImplSDL3_UpdateIme();

    // Update game controllers (if enabled and available)
    RblxUI_ImplSDL3_UpdateGamepads();
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously.
// If you are new to dear rblxui or creating a new binding for dear rblxui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

// Helper structure we store in the void* PlatformUserData field of each RblxUIViewport to easily retrieve our backend data.
struct RblxUI_ImplSDL3_ViewportData
{
    SDL_Window*     Window;
    SDL_Window*     ParentWindow;
    Uint32          WindowID;       // Stored in RblxUIViewport::PlatformHandle. Use SDL_GetWindowFromID() to get SDL_Window* from Uint32 WindowID.
    bool            WindowOwned;
    SDL_GLContext   GLContext;

    RblxUI_ImplSDL3_ViewportData()   { Window = ParentWindow = nullptr; WindowID = 0; WindowOwned = false; GLContext = nullptr; }
    ~RblxUI_ImplSDL3_ViewportData()  { RBLXUI_ASSERT(Window == nullptr && GLContext == nullptr); }
};

static SDL_Window* RblxUI_ImplSDL3_GetSDLWindowFromViewport(RblxUIViewport* viewport)
{
    if (viewport != nullptr)
    {
        SDL_WindowID window_id = (SDL_WindowID)(intptr_t)viewport->PlatformHandle;
        return SDL_GetWindowFromID(window_id);
    }
    return nullptr;
}

static void RblxUI_ImplSDL3_CreateWindow(RblxUIViewport* viewport)
{
    RblxUI_ImplSDL3_Data* bd = RblxUI_ImplSDL3_GetBackendData();
    RblxUI_ImplSDL3_ViewportData* vd = RBLXUI_NEW(RblxUI_ImplSDL3_ViewportData)();
    viewport->PlatformUserData = vd;

    vd->ParentWindow = RblxUI_ImplSDL3_GetSDLWindowFromViewport(viewport->ParentViewport);

    RblxUIViewport* main_viewport = RblxUI::GetMainViewport();
    RblxUI_ImplSDL3_ViewportData* main_viewport_data = (RblxUI_ImplSDL3_ViewportData*)main_viewport->PlatformUserData;

    // Share GL resources with main context
    bool use_opengl = (main_viewport_data->GLContext != nullptr);
    SDL_GLContext backup_context = nullptr;
    if (use_opengl)
    {
        backup_context = SDL_GL_GetCurrentContext();
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
        SDL_GL_MakeCurrent(main_viewport_data->Window, main_viewport_data->GLContext);
    }

    SDL_WindowFlags sdl_flags = 0;
    sdl_flags |= SDL_WINDOW_HIDDEN;
    sdl_flags |= use_opengl ? SDL_WINDOW_OPENGL : (bd->UseVulkan ? SDL_WINDOW_VULKAN : 0);
    sdl_flags |= SDL_GetWindowFlags(bd->Window) & SDL_WINDOW_HIGH_PIXEL_DENSITY;
    sdl_flags |= (viewport->Flags & RblxUIViewportFlags_NoDecoration) ? SDL_WINDOW_BORDERLESS : 0;
    sdl_flags |= (viewport->Flags & RblxUIViewportFlags_NoDecoration) ? 0 : SDL_WINDOW_RESIZABLE;
    sdl_flags |= (viewport->Flags & RblxUIViewportFlags_NoTaskBarIcon) ? SDL_WINDOW_UTILITY : 0;
    sdl_flags |= (viewport->Flags & RblxUIViewportFlags_TopMost) ? SDL_WINDOW_ALWAYS_ON_TOP : 0;
    vd->Window = SDL_CreateWindow("No Title Yet", (int)viewport->Size.x, (int)viewport->Size.y, sdl_flags);
#ifndef __APPLE__ // On Mac, SDL3 Parenting appears to prevent viewport from appearing in another monitor
    SDL_SetWindowParent(vd->Window, vd->ParentWindow);
#endif
    SDL_SetWindowPosition(vd->Window, (int)viewport->Pos.x, (int)viewport->Pos.y);
    vd->WindowOwned = true;
    if (use_opengl)
    {
        vd->GLContext = SDL_GL_CreateContext(vd->Window);
        SDL_GL_SetSwapInterval(0);
    }
    if (use_opengl && backup_context)
        SDL_GL_MakeCurrent(vd->Window, backup_context);

    RblxUI_ImplSDL3_SetupPlatformHandles(viewport, vd->Window);
}

static void RblxUI_ImplSDL3_DestroyWindow(RblxUIViewport* viewport)
{
    if (RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData)
    {
        if (vd->GLContext && vd->WindowOwned)
            SDL_GL_DestroyContext(vd->GLContext);
        if (vd->Window && vd->WindowOwned)
            SDL_DestroyWindow(vd->Window);
        vd->GLContext = nullptr;
        vd->Window = nullptr;
        RBLXUI_DELETE(vd);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

static void RblxUI_ImplSDL3_ShowWindow(RblxUIViewport* viewport)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
#if defined(_WIN32) && !(defined(WINAPI_FAMILY) && ((defined(WINAPI_FAMILY_APP) && WINAPI_FAMILY == WINAPI_FAMILY_APP) || (defined(WINAPI_FAMILY_GAMES) && WINAPI_FAMILY == WINAPI_FAMILY_GAMES)))
    HWND hwnd = (HWND)viewport->PlatformHandleRaw;

    // SDL hack: Show icon in task bar (#7989)
    // Note: SDL_WINDOW_UTILITY can be used to control task bar visibility, but on Windows, it does not affect child windows.
    if (!(viewport->Flags & RblxUIViewportFlags_NoTaskBarIcon))
    {
        LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        ex_style |= WS_EX_APPWINDOW;
        ex_style &= ~WS_EX_TOOLWINDOW;
        ::ShowWindow(hwnd, SW_HIDE);
        ::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
    }
#endif

#ifdef __APPLE__
    SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "1"); // Otherwise new window appear under
#else
    SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, (viewport->Flags & RblxUIViewportFlags_NoFocusOnAppearing) ? "0" : "1");
#endif
    SDL_ShowWindow(vd->Window);
}

static void RblxUI_ImplSDL3_UpdateWindow(RblxUIViewport* viewport)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
    RBLXUI_UNUSED(vd);

#ifndef __APPLE__ // On Mac, SDL3 Parenting appears to prevent viewport from appearing in another monitor
    // Update SDL3 parent if it changed _after_ creation.
    // This is for advanced apps that are manipulating ParentViewportID manually.
    SDL_Window* new_parent = RblxUI_ImplSDL3_GetSDLWindowFromViewport(viewport->ParentViewport);
    if (new_parent != vd->ParentWindow)
    {
        vd->ParentWindow = new_parent;
        SDL_SetWindowParent(vd->Window, vd->ParentWindow);
    }
#endif
}

static RblxVec2 RblxUI_ImplSDL3_GetWindowPos(RblxUIViewport* viewport)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
    int x = 0, y = 0;
    SDL_GetWindowPosition(vd->Window, &x, &y);
    return RblxVec2((float)x, (float)y);
}

static void RblxUI_ImplSDL3_SetWindowPos(RblxUIViewport* viewport, RblxVec2 pos)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
    SDL_SetWindowPosition(vd->Window, (int)pos.x, (int)pos.y);
}

static RblxVec2 RblxUI_ImplSDL3_GetWindowSize(RblxUIViewport* viewport)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
    int w = 0, h = 0;
    SDL_GetWindowSize(vd->Window, &w, &h);
    return RblxVec2((float)w, (float)h);
}

static void RblxUI_ImplSDL3_SetWindowSize(RblxUIViewport* viewport, RblxVec2 size)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
    SDL_SetWindowSize(vd->Window, (int)size.x, (int)size.y);
}

static RblxVec2 RblxUI_ImplSDL3_GetWindowFramebufferScale(RblxUIViewport* viewport)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
    RblxVec2 framebuffer_scale;
    RblxUI_ImplSDL3_GetWindowSizeAndFramebufferScale(vd->Window, nullptr, &framebuffer_scale);
    return framebuffer_scale;
}

static void RblxUI_ImplSDL3_SetWindowTitle(RblxUIViewport* viewport, const char* title)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
    SDL_SetWindowTitle(vd->Window, title);
}

static void RblxUI_ImplSDL3_SetWindowAlpha(RblxUIViewport* viewport, float alpha)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
    SDL_SetWindowOpacity(vd->Window, alpha);
}

static void RblxUI_ImplSDL3_SetWindowFocus(RblxUIViewport* viewport)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
    SDL_RaiseWindow(vd->Window);
}

static bool RblxUI_ImplSDL3_GetWindowFocus(RblxUIViewport* viewport)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
    return (SDL_GetWindowFlags(vd->Window) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

static bool RblxUI_ImplSDL3_GetWindowMinimized(RblxUIViewport* viewport)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
    return (SDL_GetWindowFlags(vd->Window) & SDL_WINDOW_MINIMIZED) != 0;
}

static void RblxUI_ImplSDL3_RenderWindow(RblxUIViewport* viewport, void*)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
    if (vd->GLContext)
        SDL_GL_MakeCurrent(vd->Window, vd->GLContext);
}

static void RblxUI_ImplSDL3_SwapBuffers(RblxUIViewport* viewport, void*)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
    if (vd->GLContext)
    {
        SDL_GL_MakeCurrent(vd->Window, vd->GLContext);
        SDL_GL_SwapWindow(vd->Window);
    }
}

// Vulkan support (the Vulkan renderer needs to call a platform-side support function to create the surface)
// SDL is graceful enough to _not_ need <vulkan/vulkan.h> so we can safely include this.
#include <SDL3/SDL_vulkan.h>
static int RblxUI_ImplSDL3_CreateVkSurface(RblxUIViewport* viewport, RblxU64 vk_instance, const void* vk_allocator, RblxU64* out_vk_surface)
{
    RblxUI_ImplSDL3_ViewportData* vd = (RblxUI_ImplSDL3_ViewportData*)viewport->PlatformUserData;
    (void)vk_allocator;
    bool ret = SDL_Vulkan_CreateSurface(vd->Window, (VkInstance)vk_instance, (const VkAllocationCallbacks*)vk_allocator, (VkSurfaceKHR*)out_vk_surface);
    return ret ? 0 : 1; // ret ? VK_SUCCESS : VK_NOT_READY
}

static void RblxUI_ImplSDL3_InitMultiViewportSupport(SDL_Window* window, void* sdl_gl_context)
{
    // Register platform interface (will be coupled with a renderer interface)
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    platform_io.Platform_CreateWindow = RblxUI_ImplSDL3_CreateWindow;
    platform_io.Platform_DestroyWindow = RblxUI_ImplSDL3_DestroyWindow;
    platform_io.Platform_ShowWindow = RblxUI_ImplSDL3_ShowWindow;
    platform_io.Platform_UpdateWindow = RblxUI_ImplSDL3_UpdateWindow;
    platform_io.Platform_SetWindowPos = RblxUI_ImplSDL3_SetWindowPos;
    platform_io.Platform_GetWindowPos = RblxUI_ImplSDL3_GetWindowPos;
    platform_io.Platform_SetWindowSize = RblxUI_ImplSDL3_SetWindowSize;
    platform_io.Platform_GetWindowSize = RblxUI_ImplSDL3_GetWindowSize;
    platform_io.Platform_GetWindowFramebufferScale = RblxUI_ImplSDL3_GetWindowFramebufferScale;
    platform_io.Platform_SetWindowFocus = RblxUI_ImplSDL3_SetWindowFocus;
    platform_io.Platform_GetWindowFocus = RblxUI_ImplSDL3_GetWindowFocus;
    platform_io.Platform_GetWindowMinimized = RblxUI_ImplSDL3_GetWindowMinimized;
    platform_io.Platform_SetWindowTitle = RblxUI_ImplSDL3_SetWindowTitle;
    platform_io.Platform_RenderWindow = RblxUI_ImplSDL3_RenderWindow;
    platform_io.Platform_SwapBuffers = RblxUI_ImplSDL3_SwapBuffers;
    platform_io.Platform_SetWindowAlpha = RblxUI_ImplSDL3_SetWindowAlpha;
    platform_io.Platform_CreateVkSurface = RblxUI_ImplSDL3_CreateVkSurface;

    // Register main window handle (which is owned by the main application, not by us)
    // This is mostly for simplicity and consistency, so that our code (e.g. mouse handling etc.) can use same logic for main and secondary viewports.
    RblxUIViewport* main_viewport = RblxUI::GetMainViewport();
    RblxUI_ImplSDL3_ViewportData* vd = RBLXUI_NEW(RblxUI_ImplSDL3_ViewportData)();
    vd->Window = window;
    vd->WindowID = SDL_GetWindowID(window);
    vd->WindowOwned = false;
    vd->GLContext = (SDL_GLContext)sdl_gl_context;
    main_viewport->PlatformUserData = vd;
    main_viewport->PlatformHandle = (void*)(intptr_t)vd->WindowID;
}

static void RblxUI_ImplSDL3_ShutdownMultiViewportSupport()
{
    RblxUI::DestroyPlatformWindows();
}

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef RBLXUI_DISABLE
