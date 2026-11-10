// dear rblxui: Platform Backend for SDL2
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3, Vulkan..)
// (Info: SDL2 is a cross-platform general purpose library for handling windows, inputs, graphics context creation, etc.)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass RblxUIKey values to all key functions e.g. RblxUI::IsKeyPressed(RblxUIKey_Space). [Legacy SDL_SCANCODE_* values are obsolete since 1.87 and not supported since 1.91.5]
//  [X] Platform: Gamepad support.
//  [X] Platform: Mouse cursor shape and visibility (RblxUIBackendFlags_HasMouseCursors). Disable with 'io.ConfigFlags |= RblxUIConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Basic IME support. App needs to call 'SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");' before SDL_CreateWindow()!.
//  [X] Platform: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= RblxUIConfigFlags_ViewportsEnable'.
// Missing features or Issues:
//  [ ] Platform: Multi-viewport: Minimized windows seems to break mouse wheel events (at least under Windows).
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

struct SDL_Window;
struct SDL_Renderer;
struct _SDL_GameController;
typedef union SDL_Event SDL_Event;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool     RblxUI_ImplSDL2_InitForOpenGL(SDL_Window* window, void* sdl_gl_context);
RBLXUI_IMPL_API bool     RblxUI_ImplSDL2_InitForVulkan(SDL_Window* window);
RBLXUI_IMPL_API bool     RblxUI_ImplSDL2_InitForD3D(SDL_Window* window);
RBLXUI_IMPL_API bool     RblxUI_ImplSDL2_InitForMetal(SDL_Window* window);
RBLXUI_IMPL_API bool     RblxUI_ImplSDL2_InitForSDLRenderer(SDL_Window* window, SDL_Renderer* renderer);
RBLXUI_IMPL_API bool     RblxUI_ImplSDL2_InitForOther(SDL_Window* window);
RBLXUI_IMPL_API void     RblxUI_ImplSDL2_Shutdown();
RBLXUI_IMPL_API void     RblxUI_ImplSDL2_NewFrame();
RBLXUI_IMPL_API bool     RblxUI_ImplSDL2_ProcessEvent(const SDL_Event* event);

// DPI-related helpers (optional)
RBLXUI_IMPL_API float    RblxUI_ImplSDL2_GetContentScaleForWindow(SDL_Window* window);
RBLXUI_IMPL_API float    RblxUI_ImplSDL2_GetContentScaleForDisplay(int display_index);

// Gamepad selection automatically starts in AutoFirst mode, picking first available SDL_Gamepad. You may override this.
// When using manual mode, caller is responsible for opening/closing gamepad.
enum RblxUI_ImplSDL2_GamepadMode { RblxUI_ImplSDL2_GamepadMode_AutoFirst, RblxUI_ImplSDL2_GamepadMode_AutoAll, RblxUI_ImplSDL2_GamepadMode_Manual };
RBLXUI_IMPL_API void     RblxUI_ImplSDL2_SetGamepadMode(RblxUI_ImplSDL2_GamepadMode mode, struct _SDL_GameController** manual_gamepads_array = nullptr, int manual_gamepads_count = -1);

// (Advanced, for X11 users) Override Mouse Capture mode. Mouse capture allows receiving updated mouse position after clicking inside our window and dragging outside it.
// Having this 'Enabled' is in theory always better. But, on X11 if you crash/break to debugger while capture is active you may temporarily lose access to your mouse.
// The best solution is to setup your debugger to automatically release capture, e.g. 'setxkbmap -option grab:break_actions && xdotool key XF86Ungrab' or via a GDB script. See #3650.
// But you may independently decide on X11, when a debugger is attached, to set this value to RblxUI_ImplSDL2_MouseCaptureMode_Disabled.
enum RblxUI_ImplSDL2_MouseCaptureMode { RblxUI_ImplSDL2_MouseCaptureMode_Enabled, RblxUI_ImplSDL2_MouseCaptureMode_EnabledAfterDrag, RblxUI_ImplSDL2_MouseCaptureMode_Disabled };
RBLXUI_IMPL_API void     RblxUI_ImplSDL2_SetMouseCaptureMode(RblxUI_ImplSDL2_MouseCaptureMode mode);

#endif // #ifndef RBLXUI_DISABLE
