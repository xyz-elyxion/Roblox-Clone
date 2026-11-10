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

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2022-09-26: Inputs: Renamed RblxUIKey_ModXXX introduced in 1.87 to RblxUIMod_XXX (old names still supported).
//  2022-01-26: Inputs: replaced short-lived io.AddKeyModsEvent() (added two weeks ago) with io.AddKeyEvent() using RblxUIKey_ModXXX flags. Sorry for the confusion.
//  2022-01-17: Inputs: calling new io.AddMousePosEvent(), io.AddMouseButtonEvent(), io.AddMouseWheelEvent() API (1.87+).
//  2022-01-10: Inputs: calling new io.AddKeyEvent(), io.AddKeyModsEvent() + io.SetKeyEventNativeData() API (1.87+). Support for full RblxUIKey range.
//  2021-03-04: Initial version.

#include "rblxui.h"
#ifndef RBLXUI_DISABLE
#include "rblxui_impl_android.h"
#include <time.h>
#include <android/native_window.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <android/log.h>

// Android data
static double                                   g_Time = 0.0;
static ANativeWindow*                           g_Window;
static char                                     g_LogTag[] = "RblxUIExample";

static RblxUIKey RblxUI_ImplAndroid_KeyCodeToRblxUIKey(int32_t key_code)
{
    switch (key_code)
    {
        case AKEYCODE_TAB:                  return RblxUIKey_Tab;
        case AKEYCODE_DPAD_LEFT:            return RblxUIKey_LeftArrow;
        case AKEYCODE_DPAD_RIGHT:           return RblxUIKey_RightArrow;
        case AKEYCODE_DPAD_UP:              return RblxUIKey_UpArrow;
        case AKEYCODE_DPAD_DOWN:            return RblxUIKey_DownArrow;
        case AKEYCODE_PAGE_UP:              return RblxUIKey_PageUp;
        case AKEYCODE_PAGE_DOWN:            return RblxUIKey_PageDown;
        case AKEYCODE_MOVE_HOME:            return RblxUIKey_Home;
        case AKEYCODE_MOVE_END:             return RblxUIKey_End;
        case AKEYCODE_INSERT:               return RblxUIKey_Insert;
        case AKEYCODE_FORWARD_DEL:          return RblxUIKey_Delete;
        case AKEYCODE_DEL:                  return RblxUIKey_Backspace;
        case AKEYCODE_SPACE:                return RblxUIKey_Space;
        case AKEYCODE_ENTER:                return RblxUIKey_Enter;
        case AKEYCODE_ESCAPE:               return RblxUIKey_Escape;
        case AKEYCODE_APOSTROPHE:           return RblxUIKey_Apostrophe;
        case AKEYCODE_COMMA:                return RblxUIKey_Comma;
        case AKEYCODE_MINUS:                return RblxUIKey_Minus;
        case AKEYCODE_PERIOD:               return RblxUIKey_Period;
        case AKEYCODE_SLASH:                return RblxUIKey_Slash;
        case AKEYCODE_SEMICOLON:            return RblxUIKey_Semicolon;
        case AKEYCODE_EQUALS:               return RblxUIKey_Equal;
        case AKEYCODE_LEFT_BRACKET:         return RblxUIKey_LeftBracket;
        case AKEYCODE_BACKSLASH:            return RblxUIKey_Backslash;
        case AKEYCODE_RIGHT_BRACKET:        return RblxUIKey_RightBracket;
        case AKEYCODE_GRAVE:                return RblxUIKey_GraveAccent;
        case AKEYCODE_CAPS_LOCK:            return RblxUIKey_CapsLock;
        case AKEYCODE_SCROLL_LOCK:          return RblxUIKey_ScrollLock;
        case AKEYCODE_NUM_LOCK:             return RblxUIKey_NumLock;
        case AKEYCODE_SYSRQ:                return RblxUIKey_PrintScreen;
        case AKEYCODE_BREAK:                return RblxUIKey_Pause;
        case AKEYCODE_NUMPAD_0:             return RblxUIKey_Keypad0;
        case AKEYCODE_NUMPAD_1:             return RblxUIKey_Keypad1;
        case AKEYCODE_NUMPAD_2:             return RblxUIKey_Keypad2;
        case AKEYCODE_NUMPAD_3:             return RblxUIKey_Keypad3;
        case AKEYCODE_NUMPAD_4:             return RblxUIKey_Keypad4;
        case AKEYCODE_NUMPAD_5:             return RblxUIKey_Keypad5;
        case AKEYCODE_NUMPAD_6:             return RblxUIKey_Keypad6;
        case AKEYCODE_NUMPAD_7:             return RblxUIKey_Keypad7;
        case AKEYCODE_NUMPAD_8:             return RblxUIKey_Keypad8;
        case AKEYCODE_NUMPAD_9:             return RblxUIKey_Keypad9;
        case AKEYCODE_NUMPAD_DOT:           return RblxUIKey_KeypadDecimal;
        case AKEYCODE_NUMPAD_DIVIDE:        return RblxUIKey_KeypadDivide;
        case AKEYCODE_NUMPAD_MULTIPLY:      return RblxUIKey_KeypadMultiply;
        case AKEYCODE_NUMPAD_SUBTRACT:      return RblxUIKey_KeypadSubtract;
        case AKEYCODE_NUMPAD_ADD:           return RblxUIKey_KeypadAdd;
        case AKEYCODE_NUMPAD_ENTER:         return RblxUIKey_KeypadEnter;
        case AKEYCODE_NUMPAD_EQUALS:        return RblxUIKey_KeypadEqual;
        case AKEYCODE_CTRL_LEFT:            return RblxUIKey_LeftCtrl;
        case AKEYCODE_SHIFT_LEFT:           return RblxUIKey_LeftShift;
        case AKEYCODE_ALT_LEFT:             return RblxUIKey_LeftAlt;
        case AKEYCODE_META_LEFT:            return RblxUIKey_LeftSuper;
        case AKEYCODE_CTRL_RIGHT:           return RblxUIKey_RightCtrl;
        case AKEYCODE_SHIFT_RIGHT:          return RblxUIKey_RightShift;
        case AKEYCODE_ALT_RIGHT:            return RblxUIKey_RightAlt;
        case AKEYCODE_META_RIGHT:           return RblxUIKey_RightSuper;
        case AKEYCODE_MENU:                 return RblxUIKey_Menu;
        case AKEYCODE_0:                    return RblxUIKey_0;
        case AKEYCODE_1:                    return RblxUIKey_1;
        case AKEYCODE_2:                    return RblxUIKey_2;
        case AKEYCODE_3:                    return RblxUIKey_3;
        case AKEYCODE_4:                    return RblxUIKey_4;
        case AKEYCODE_5:                    return RblxUIKey_5;
        case AKEYCODE_6:                    return RblxUIKey_6;
        case AKEYCODE_7:                    return RblxUIKey_7;
        case AKEYCODE_8:                    return RblxUIKey_8;
        case AKEYCODE_9:                    return RblxUIKey_9;
        case AKEYCODE_A:                    return RblxUIKey_A;
        case AKEYCODE_B:                    return RblxUIKey_B;
        case AKEYCODE_C:                    return RblxUIKey_C;
        case AKEYCODE_D:                    return RblxUIKey_D;
        case AKEYCODE_E:                    return RblxUIKey_E;
        case AKEYCODE_F:                    return RblxUIKey_F;
        case AKEYCODE_G:                    return RblxUIKey_G;
        case AKEYCODE_H:                    return RblxUIKey_H;
        case AKEYCODE_I:                    return RblxUIKey_I;
        case AKEYCODE_J:                    return RblxUIKey_J;
        case AKEYCODE_K:                    return RblxUIKey_K;
        case AKEYCODE_L:                    return RblxUIKey_L;
        case AKEYCODE_M:                    return RblxUIKey_M;
        case AKEYCODE_N:                    return RblxUIKey_N;
        case AKEYCODE_O:                    return RblxUIKey_O;
        case AKEYCODE_P:                    return RblxUIKey_P;
        case AKEYCODE_Q:                    return RblxUIKey_Q;
        case AKEYCODE_R:                    return RblxUIKey_R;
        case AKEYCODE_S:                    return RblxUIKey_S;
        case AKEYCODE_T:                    return RblxUIKey_T;
        case AKEYCODE_U:                    return RblxUIKey_U;
        case AKEYCODE_V:                    return RblxUIKey_V;
        case AKEYCODE_W:                    return RblxUIKey_W;
        case AKEYCODE_X:                    return RblxUIKey_X;
        case AKEYCODE_Y:                    return RblxUIKey_Y;
        case AKEYCODE_Z:                    return RblxUIKey_Z;
        case AKEYCODE_F1:                   return RblxUIKey_F1;
        case AKEYCODE_F2:                   return RblxUIKey_F2;
        case AKEYCODE_F3:                   return RblxUIKey_F3;
        case AKEYCODE_F4:                   return RblxUIKey_F4;
        case AKEYCODE_F5:                   return RblxUIKey_F5;
        case AKEYCODE_F6:                   return RblxUIKey_F6;
        case AKEYCODE_F7:                   return RblxUIKey_F7;
        case AKEYCODE_F8:                   return RblxUIKey_F8;
        case AKEYCODE_F9:                   return RblxUIKey_F9;
        case AKEYCODE_F10:                  return RblxUIKey_F10;
        case AKEYCODE_F11:                  return RblxUIKey_F11;
        case AKEYCODE_F12:                  return RblxUIKey_F12;
        default:                            return RblxUIKey_None;
    }
}

int32_t RblxUI_ImplAndroid_HandleInputEvent(const AInputEvent* input_event)
{
    RblxUIIO& io = RblxUI::GetIO();
    int32_t event_type = AInputEvent_getType(input_event);
    switch (event_type)
    {
    case AINPUT_EVENT_TYPE_KEY:
    {
        int32_t event_key_code = AKeyEvent_getKeyCode(input_event);
        int32_t event_scan_code = AKeyEvent_getScanCode(input_event);
        int32_t event_action = AKeyEvent_getAction(input_event);
        int32_t event_meta_state = AKeyEvent_getMetaState(input_event);

        io.AddKeyEvent(RblxUIMod_Ctrl,  (event_meta_state & AMETA_CTRL_ON)  != 0);
        io.AddKeyEvent(RblxUIMod_Shift, (event_meta_state & AMETA_SHIFT_ON) != 0);
        io.AddKeyEvent(RblxUIMod_Alt,   (event_meta_state & AMETA_ALT_ON)   != 0);
        io.AddKeyEvent(RblxUIMod_Super, (event_meta_state & AMETA_META_ON)  != 0);

        switch (event_action)
        {
        // FIXME: AKEY_EVENT_ACTION_DOWN and AKEY_EVENT_ACTION_UP occur at once as soon as a touch pointer
        // goes up from a key. We use a simple key event queue/ and process one event per key per frame in
        // RblxUI_ImplAndroid_NewFrame()...or consider using IO queue, if suitable: https://github.com/ocornut/rblxui/issues/2787
        case AKEY_EVENT_ACTION_DOWN:
        case AKEY_EVENT_ACTION_UP:
        {
            RblxUIKey key = RblxUI_ImplAndroid_KeyCodeToRblxUIKey(event_key_code);
            if (key != RblxUIKey_None)
            {
                io.AddKeyEvent(key, event_action == AKEY_EVENT_ACTION_DOWN);
                io.SetKeyEventNativeData(key, event_key_code, event_scan_code);
            }

            break;
        }
        default:
            break;
        }
        break;
    }
    case AINPUT_EVENT_TYPE_MOTION:
    {
        int32_t event_action = AMotionEvent_getAction(input_event);
        int32_t event_pointer_index = (event_action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        event_action &= AMOTION_EVENT_ACTION_MASK;

        switch (AMotionEvent_getToolType(input_event, event_pointer_index))
        {
        case AMOTION_EVENT_TOOL_TYPE_MOUSE:
            io.AddMouseSourceEvent(RblxUIMouseSource_Mouse);
            break;
        case AMOTION_EVENT_TOOL_TYPE_STYLUS:
        case AMOTION_EVENT_TOOL_TYPE_ERASER:
            io.AddMouseSourceEvent(RblxUIMouseSource_Pen);
            break;
        case AMOTION_EVENT_TOOL_TYPE_FINGER:
        default:
            io.AddMouseSourceEvent(RblxUIMouseSource_TouchScreen);
            break;
        }

        switch (event_action)
        {
        case AMOTION_EVENT_ACTION_DOWN:
        case AMOTION_EVENT_ACTION_UP:
        {
            // Physical mouse buttons (and probably other physical devices) also invoke the actions AMOTION_EVENT_ACTION_DOWN/_UP,
            // but we have to process them separately to identify the actual button pressed. This is done below via
            // AMOTION_EVENT_ACTION_BUTTON_PRESS/_RELEASE. Here, we only process "FINGER" input (and "UNKNOWN", as a fallback).
            int tool_type = AMotionEvent_getToolType(input_event, event_pointer_index);
            if (tool_type == AMOTION_EVENT_TOOL_TYPE_FINGER || tool_type == AMOTION_EVENT_TOOL_TYPE_UNKNOWN)
            {
                io.AddMousePosEvent(AMotionEvent_getX(input_event, event_pointer_index), AMotionEvent_getY(input_event, event_pointer_index));
                io.AddMouseButtonEvent(0, event_action == AMOTION_EVENT_ACTION_DOWN);
            }
            break;
        }
        case AMOTION_EVENT_ACTION_BUTTON_PRESS:
        case AMOTION_EVENT_ACTION_BUTTON_RELEASE:
        {
            int32_t button_state = AMotionEvent_getButtonState(input_event);
            io.AddMouseButtonEvent(0, (button_state & AMOTION_EVENT_BUTTON_PRIMARY) != 0);
            io.AddMouseButtonEvent(1, (button_state & AMOTION_EVENT_BUTTON_SECONDARY) != 0);
            io.AddMouseButtonEvent(2, (button_state & AMOTION_EVENT_BUTTON_TERTIARY) != 0);
            break;
        }
        case AMOTION_EVENT_ACTION_HOVER_MOVE: // Hovering: Tool moves while NOT pressed (such as a physical mouse)
        case AMOTION_EVENT_ACTION_MOVE:       // Touch pointer moves while DOWN
            io.AddMousePosEvent(AMotionEvent_getX(input_event, event_pointer_index), AMotionEvent_getY(input_event, event_pointer_index));
            break;
        case AMOTION_EVENT_ACTION_SCROLL:
            io.AddMouseWheelEvent(AMotionEvent_getAxisValue(input_event, AMOTION_EVENT_AXIS_HSCROLL, event_pointer_index), AMotionEvent_getAxisValue(input_event, AMOTION_EVENT_AXIS_VSCROLL, event_pointer_index));
            break;
        default:
            break;
        }
    }
        return 1;
    default:
        break;
    }

    return 0;
}

bool RblxUI_ImplAndroid_Init(ANativeWindow* window)
{
    RBLXUI_CHECKVERSION();

    g_Window = window;
    g_Time = 0.0;

    // Setup backend capabilities flags
    RblxUIIO& io = RblxUI::GetIO();
    io.BackendPlatformName = "rblxui_impl_android";

    return true;
}

void RblxUI_ImplAndroid_Shutdown()
{
    RblxUIIO& io = RblxUI::GetIO();
    io.BackendPlatformName = nullptr;
}

void RblxUI_ImplAndroid_NewFrame()
{
    RblxUIIO& io = RblxUI::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int32_t window_width = ANativeWindow_getWidth(g_Window);
    int32_t window_height = ANativeWindow_getHeight(g_Window);
    int display_width = window_width;
    int display_height = window_height;

    io.DisplaySize = RblxVec2((float)window_width, (float)window_height);
    if (window_width > 0 && window_height > 0)
        io.DisplayFramebufferScale = RblxVec2((float)display_width / window_width, (float)display_height / window_height);

    // Setup time step
    struct timespec current_timespec;
    clock_gettime(CLOCK_MONOTONIC, &current_timespec);
    double current_time = (double)(current_timespec.tv_sec) + (current_timespec.tv_nsec / 1000000000.0);
    io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
    g_Time = current_time;
}

//-----------------------------------------------------------------------------

#endif // #ifndef RBLXUI_DISABLE
