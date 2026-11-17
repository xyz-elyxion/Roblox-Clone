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

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2022-09-26: Inputs: Renamed LinearUIKey_ModXXX introduced in 1.87 to LinearUIMod_XXX (old names still supported).
//  2022-01-26: Inputs: replaced short-lived io.AddKeyModsEvent() (added two weeks ago) with io.AddKeyEvent() using LinearUIKey_ModXXX flags. Sorry for the confusion.
//  2022-01-17: Inputs: calling new io.AddMousePosEvent(), io.AddMouseButtonEvent(), io.AddMouseWheelEvent() API (1.87+).
//  2022-01-10: Inputs: calling new io.AddKeyEvent(), io.AddKeyModsEvent() + io.SetKeyEventNativeData() API (1.87+). Support for full LinearUIKey range.
//  2021-03-04: Initial version.

#include "linearui.h"
#ifndef LINEARUI_DISABLE
#include "linearui_impl_android.h"
#include <time.h>
#include <android/native_window.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <android/log.h>

// Android data
static double                                   g_Time = 0.0;
static ANativeWindow*                           g_Window;
static char                                     g_LogTag[] = "LinearUIExample";

static LinearUIKey LinearUI_ImplAndroid_KeyCodeToLinearUIKey(int32_t key_code)
{
    switch (key_code)
    {
        case AKEYCODE_TAB:                  return LinearUIKey_Tab;
        case AKEYCODE_DPAD_LEFT:            return LinearUIKey_LeftArrow;
        case AKEYCODE_DPAD_RIGHT:           return LinearUIKey_RightArrow;
        case AKEYCODE_DPAD_UP:              return LinearUIKey_UpArrow;
        case AKEYCODE_DPAD_DOWN:            return LinearUIKey_DownArrow;
        case AKEYCODE_PAGE_UP:              return LinearUIKey_PageUp;
        case AKEYCODE_PAGE_DOWN:            return LinearUIKey_PageDown;
        case AKEYCODE_MOVE_HOME:            return LinearUIKey_Home;
        case AKEYCODE_MOVE_END:             return LinearUIKey_End;
        case AKEYCODE_INSERT:               return LinearUIKey_Insert;
        case AKEYCODE_FORWARD_DEL:          return LinearUIKey_Delete;
        case AKEYCODE_DEL:                  return LinearUIKey_Backspace;
        case AKEYCODE_SPACE:                return LinearUIKey_Space;
        case AKEYCODE_ENTER:                return LinearUIKey_Enter;
        case AKEYCODE_ESCAPE:               return LinearUIKey_Escape;
        case AKEYCODE_APOSTROPHE:           return LinearUIKey_Apostrophe;
        case AKEYCODE_COMMA:                return LinearUIKey_Comma;
        case AKEYCODE_MINUS:                return LinearUIKey_Minus;
        case AKEYCODE_PERIOD:               return LinearUIKey_Period;
        case AKEYCODE_SLASH:                return LinearUIKey_Slash;
        case AKEYCODE_SEMICOLON:            return LinearUIKey_Semicolon;
        case AKEYCODE_EQUALS:               return LinearUIKey_Equal;
        case AKEYCODE_LEFT_BRACKET:         return LinearUIKey_LeftBracket;
        case AKEYCODE_BACKSLASH:            return LinearUIKey_Backslash;
        case AKEYCODE_RIGHT_BRACKET:        return LinearUIKey_RightBracket;
        case AKEYCODE_GRAVE:                return LinearUIKey_GraveAccent;
        case AKEYCODE_CAPS_LOCK:            return LinearUIKey_CapsLock;
        case AKEYCODE_SCROLL_LOCK:          return LinearUIKey_ScrollLock;
        case AKEYCODE_NUM_LOCK:             return LinearUIKey_NumLock;
        case AKEYCODE_SYSRQ:                return LinearUIKey_PrintScreen;
        case AKEYCODE_BREAK:                return LinearUIKey_Pause;
        case AKEYCODE_NUMPAD_0:             return LinearUIKey_Keypad0;
        case AKEYCODE_NUMPAD_1:             return LinearUIKey_Keypad1;
        case AKEYCODE_NUMPAD_2:             return LinearUIKey_Keypad2;
        case AKEYCODE_NUMPAD_3:             return LinearUIKey_Keypad3;
        case AKEYCODE_NUMPAD_4:             return LinearUIKey_Keypad4;
        case AKEYCODE_NUMPAD_5:             return LinearUIKey_Keypad5;
        case AKEYCODE_NUMPAD_6:             return LinearUIKey_Keypad6;
        case AKEYCODE_NUMPAD_7:             return LinearUIKey_Keypad7;
        case AKEYCODE_NUMPAD_8:             return LinearUIKey_Keypad8;
        case AKEYCODE_NUMPAD_9:             return LinearUIKey_Keypad9;
        case AKEYCODE_NUMPAD_DOT:           return LinearUIKey_KeypadDecimal;
        case AKEYCODE_NUMPAD_DIVIDE:        return LinearUIKey_KeypadDivide;
        case AKEYCODE_NUMPAD_MULTIPLY:      return LinearUIKey_KeypadMultiply;
        case AKEYCODE_NUMPAD_SUBTRACT:      return LinearUIKey_KeypadSubtract;
        case AKEYCODE_NUMPAD_ADD:           return LinearUIKey_KeypadAdd;
        case AKEYCODE_NUMPAD_ENTER:         return LinearUIKey_KeypadEnter;
        case AKEYCODE_NUMPAD_EQUALS:        return LinearUIKey_KeypadEqual;
        case AKEYCODE_CTRL_LEFT:            return LinearUIKey_LeftCtrl;
        case AKEYCODE_SHIFT_LEFT:           return LinearUIKey_LeftShift;
        case AKEYCODE_ALT_LEFT:             return LinearUIKey_LeftAlt;
        case AKEYCODE_META_LEFT:            return LinearUIKey_LeftSuper;
        case AKEYCODE_CTRL_RIGHT:           return LinearUIKey_RightCtrl;
        case AKEYCODE_SHIFT_RIGHT:          return LinearUIKey_RightShift;
        case AKEYCODE_ALT_RIGHT:            return LinearUIKey_RightAlt;
        case AKEYCODE_META_RIGHT:           return LinearUIKey_RightSuper;
        case AKEYCODE_MENU:                 return LinearUIKey_Menu;
        case AKEYCODE_0:                    return LinearUIKey_0;
        case AKEYCODE_1:                    return LinearUIKey_1;
        case AKEYCODE_2:                    return LinearUIKey_2;
        case AKEYCODE_3:                    return LinearUIKey_3;
        case AKEYCODE_4:                    return LinearUIKey_4;
        case AKEYCODE_5:                    return LinearUIKey_5;
        case AKEYCODE_6:                    return LinearUIKey_6;
        case AKEYCODE_7:                    return LinearUIKey_7;
        case AKEYCODE_8:                    return LinearUIKey_8;
        case AKEYCODE_9:                    return LinearUIKey_9;
        case AKEYCODE_A:                    return LinearUIKey_A;
        case AKEYCODE_B:                    return LinearUIKey_B;
        case AKEYCODE_C:                    return LinearUIKey_C;
        case AKEYCODE_D:                    return LinearUIKey_D;
        case AKEYCODE_E:                    return LinearUIKey_E;
        case AKEYCODE_F:                    return LinearUIKey_F;
        case AKEYCODE_G:                    return LinearUIKey_G;
        case AKEYCODE_H:                    return LinearUIKey_H;
        case AKEYCODE_I:                    return LinearUIKey_I;
        case AKEYCODE_J:                    return LinearUIKey_J;
        case AKEYCODE_K:                    return LinearUIKey_K;
        case AKEYCODE_L:                    return LinearUIKey_L;
        case AKEYCODE_M:                    return LinearUIKey_M;
        case AKEYCODE_N:                    return LinearUIKey_N;
        case AKEYCODE_O:                    return LinearUIKey_O;
        case AKEYCODE_P:                    return LinearUIKey_P;
        case AKEYCODE_Q:                    return LinearUIKey_Q;
        case AKEYCODE_R:                    return LinearUIKey_R;
        case AKEYCODE_S:                    return LinearUIKey_S;
        case AKEYCODE_T:                    return LinearUIKey_T;
        case AKEYCODE_U:                    return LinearUIKey_U;
        case AKEYCODE_V:                    return LinearUIKey_V;
        case AKEYCODE_W:                    return LinearUIKey_W;
        case AKEYCODE_X:                    return LinearUIKey_X;
        case AKEYCODE_Y:                    return LinearUIKey_Y;
        case AKEYCODE_Z:                    return LinearUIKey_Z;
        case AKEYCODE_F1:                   return LinearUIKey_F1;
        case AKEYCODE_F2:                   return LinearUIKey_F2;
        case AKEYCODE_F3:                   return LinearUIKey_F3;
        case AKEYCODE_F4:                   return LinearUIKey_F4;
        case AKEYCODE_F5:                   return LinearUIKey_F5;
        case AKEYCODE_F6:                   return LinearUIKey_F6;
        case AKEYCODE_F7:                   return LinearUIKey_F7;
        case AKEYCODE_F8:                   return LinearUIKey_F8;
        case AKEYCODE_F9:                   return LinearUIKey_F9;
        case AKEYCODE_F10:                  return LinearUIKey_F10;
        case AKEYCODE_F11:                  return LinearUIKey_F11;
        case AKEYCODE_F12:                  return LinearUIKey_F12;
        default:                            return LinearUIKey_None;
    }
}

int32_t LinearUI_ImplAndroid_HandleInputEvent(const AInputEvent* input_event)
{
    LinearUIIO& io = LinearUI::GetIO();
    int32_t event_type = AInputEvent_getType(input_event);
    switch (event_type)
    {
    case AINPUT_EVENT_TYPE_KEY:
    {
        int32_t event_key_code = AKeyEvent_getKeyCode(input_event);
        int32_t event_scan_code = AKeyEvent_getScanCode(input_event);
        int32_t event_action = AKeyEvent_getAction(input_event);
        int32_t event_meta_state = AKeyEvent_getMetaState(input_event);

        io.AddKeyEvent(LinearUIMod_Ctrl,  (event_meta_state & AMETA_CTRL_ON)  != 0);
        io.AddKeyEvent(LinearUIMod_Shift, (event_meta_state & AMETA_SHIFT_ON) != 0);
        io.AddKeyEvent(LinearUIMod_Alt,   (event_meta_state & AMETA_ALT_ON)   != 0);
        io.AddKeyEvent(LinearUIMod_Super, (event_meta_state & AMETA_META_ON)  != 0);

        switch (event_action)
        {
        // FIXME: AKEY_EVENT_ACTION_DOWN and AKEY_EVENT_ACTION_UP occur at once as soon as a touch pointer
        // goes up from a key. We use a simple key event queue/ and process one event per key per frame in
        // LinearUI_ImplAndroid_NewFrame()...or consider using IO queue, if suitable: https://github.com/ocornut/linearui/issues/2787
        case AKEY_EVENT_ACTION_DOWN:
        case AKEY_EVENT_ACTION_UP:
        {
            LinearUIKey key = LinearUI_ImplAndroid_KeyCodeToLinearUIKey(event_key_code);
            if (key != LinearUIKey_None)
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
            io.AddMouseSourceEvent(LinearUIMouseSource_Mouse);
            break;
        case AMOTION_EVENT_TOOL_TYPE_STYLUS:
        case AMOTION_EVENT_TOOL_TYPE_ERASER:
            io.AddMouseSourceEvent(LinearUIMouseSource_Pen);
            break;
        case AMOTION_EVENT_TOOL_TYPE_FINGER:
        default:
            io.AddMouseSourceEvent(LinearUIMouseSource_TouchScreen);
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

bool LinearUI_ImplAndroid_Init(ANativeWindow* window)
{
    LINEARUI_CHECKVERSION();

    g_Window = window;
    g_Time = 0.0;

    // Setup backend capabilities flags
    LinearUIIO& io = LinearUI::GetIO();
    io.BackendPlatformName = "linearui_impl_android";

    return true;
}

void LinearUI_ImplAndroid_Shutdown()
{
    LinearUIIO& io = LinearUI::GetIO();
    io.BackendPlatformName = nullptr;
}

void LinearUI_ImplAndroid_NewFrame()
{
    LinearUIIO& io = LinearUI::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int32_t window_width = ANativeWindow_getWidth(g_Window);
    int32_t window_height = ANativeWindow_getHeight(g_Window);
    int display_width = window_width;
    int display_height = window_height;

    io.DisplaySize = LinearVec2((float)window_width, (float)window_height);
    if (window_width > 0 && window_height > 0)
        io.DisplayFramebufferScale = LinearVec2((float)display_width / window_width, (float)display_height / window_height);

    // Setup time step
    struct timespec current_timespec;
    clock_gettime(CLOCK_MONOTONIC, &current_timespec);
    double current_time = (double)(current_timespec.tv_sec) + (current_timespec.tv_nsec / 1000000000.0);
    io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
    g_Time = current_time;
}

//-----------------------------------------------------------------------------

#endif // #ifndef LINEARUI_DISABLE
