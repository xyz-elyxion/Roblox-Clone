//========================================================================
// Event linter (event spewer)
// Copyright (c) Camilla Löwy <elmindreda@rblxwin.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//
// This test hooks every available callback and outputs their arguments
//
// Log messages go to stdout, error messages to stderr
//
// Every event also gets a (sequential) number to aid discussion of logs
//
//========================================================================

#include <rblxgl/gl.h>
#define RBLXWIN_INCLUDE_NONE
#include <RBLXWIN/rblxwin.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>

#include "getopt.h"

// Event index
static unsigned int counter = 0;

typedef struct
{
    RBLXWINwindow* window;
    int number;
    int closeable;
} Slot;

static void usage(void)
{
    printf("Usage: events [-f] [-h] [-n WINDOWS]\n");
    printf("Options:\n");
    printf("  -f use full screen\n");
    printf("  -h show this help\n");
    printf("  -n the number of windows to create\n");
}

static const char* get_key_name(int key)
{
    switch (key)
    {
        // Printable keys
        case RBLXWIN_KEY_A:            return "A";
        case RBLXWIN_KEY_B:            return "B";
        case RBLXWIN_KEY_C:            return "C";
        case RBLXWIN_KEY_D:            return "D";
        case RBLXWIN_KEY_E:            return "E";
        case RBLXWIN_KEY_F:            return "F";
        case RBLXWIN_KEY_G:            return "G";
        case RBLXWIN_KEY_H:            return "H";
        case RBLXWIN_KEY_I:            return "I";
        case RBLXWIN_KEY_J:            return "J";
        case RBLXWIN_KEY_K:            return "K";
        case RBLXWIN_KEY_L:            return "L";
        case RBLXWIN_KEY_M:            return "M";
        case RBLXWIN_KEY_N:            return "N";
        case RBLXWIN_KEY_O:            return "O";
        case RBLXWIN_KEY_P:            return "P";
        case RBLXWIN_KEY_Q:            return "Q";
        case RBLXWIN_KEY_R:            return "R";
        case RBLXWIN_KEY_S:            return "S";
        case RBLXWIN_KEY_T:            return "T";
        case RBLXWIN_KEY_U:            return "U";
        case RBLXWIN_KEY_V:            return "V";
        case RBLXWIN_KEY_W:            return "W";
        case RBLXWIN_KEY_X:            return "X";
        case RBLXWIN_KEY_Y:            return "Y";
        case RBLXWIN_KEY_Z:            return "Z";
        case RBLXWIN_KEY_1:            return "1";
        case RBLXWIN_KEY_2:            return "2";
        case RBLXWIN_KEY_3:            return "3";
        case RBLXWIN_KEY_4:            return "4";
        case RBLXWIN_KEY_5:            return "5";
        case RBLXWIN_KEY_6:            return "6";
        case RBLXWIN_KEY_7:            return "7";
        case RBLXWIN_KEY_8:            return "8";
        case RBLXWIN_KEY_9:            return "9";
        case RBLXWIN_KEY_0:            return "0";
        case RBLXWIN_KEY_SPACE:        return "SPACE";
        case RBLXWIN_KEY_MINUS:        return "MINUS";
        case RBLXWIN_KEY_EQUAL:        return "EQUAL";
        case RBLXWIN_KEY_LEFT_BRACKET: return "LEFT BRACKET";
        case RBLXWIN_KEY_RIGHT_BRACKET: return "RIGHT BRACKET";
        case RBLXWIN_KEY_BACKSLASH:    return "BACKSLASH";
        case RBLXWIN_KEY_SEMICOLON:    return "SEMICOLON";
        case RBLXWIN_KEY_APOSTROPHE:   return "APOSTROPHE";
        case RBLXWIN_KEY_GRAVE_ACCENT: return "GRAVE ACCENT";
        case RBLXWIN_KEY_COMMA:        return "COMMA";
        case RBLXWIN_KEY_PERIOD:       return "PERIOD";
        case RBLXWIN_KEY_SLASH:        return "SLASH";
        case RBLXWIN_KEY_WORLD_1:      return "WORLD 1";
        case RBLXWIN_KEY_WORLD_2:      return "WORLD 2";

        // Function keys
        case RBLXWIN_KEY_ESCAPE:       return "ESCAPE";
        case RBLXWIN_KEY_F1:           return "F1";
        case RBLXWIN_KEY_F2:           return "F2";
        case RBLXWIN_KEY_F3:           return "F3";
        case RBLXWIN_KEY_F4:           return "F4";
        case RBLXWIN_KEY_F5:           return "F5";
        case RBLXWIN_KEY_F6:           return "F6";
        case RBLXWIN_KEY_F7:           return "F7";
        case RBLXWIN_KEY_F8:           return "F8";
        case RBLXWIN_KEY_F9:           return "F9";
        case RBLXWIN_KEY_F10:          return "F10";
        case RBLXWIN_KEY_F11:          return "F11";
        case RBLXWIN_KEY_F12:          return "F12";
        case RBLXWIN_KEY_F13:          return "F13";
        case RBLXWIN_KEY_F14:          return "F14";
        case RBLXWIN_KEY_F15:          return "F15";
        case RBLXWIN_KEY_F16:          return "F16";
        case RBLXWIN_KEY_F17:          return "F17";
        case RBLXWIN_KEY_F18:          return "F18";
        case RBLXWIN_KEY_F19:          return "F19";
        case RBLXWIN_KEY_F20:          return "F20";
        case RBLXWIN_KEY_F21:          return "F21";
        case RBLXWIN_KEY_F22:          return "F22";
        case RBLXWIN_KEY_F23:          return "F23";
        case RBLXWIN_KEY_F24:          return "F24";
        case RBLXWIN_KEY_F25:          return "F25";
        case RBLXWIN_KEY_UP:           return "UP";
        case RBLXWIN_KEY_DOWN:         return "DOWN";
        case RBLXWIN_KEY_LEFT:         return "LEFT";
        case RBLXWIN_KEY_RIGHT:        return "RIGHT";
        case RBLXWIN_KEY_LEFT_SHIFT:   return "LEFT SHIFT";
        case RBLXWIN_KEY_RIGHT_SHIFT:  return "RIGHT SHIFT";
        case RBLXWIN_KEY_LEFT_CONTROL: return "LEFT CONTROL";
        case RBLXWIN_KEY_RIGHT_CONTROL: return "RIGHT CONTROL";
        case RBLXWIN_KEY_LEFT_ALT:     return "LEFT ALT";
        case RBLXWIN_KEY_RIGHT_ALT:    return "RIGHT ALT";
        case RBLXWIN_KEY_TAB:          return "TAB";
        case RBLXWIN_KEY_ENTER:        return "ENTER";
        case RBLXWIN_KEY_BACKSPACE:    return "BACKSPACE";
        case RBLXWIN_KEY_INSERT:       return "INSERT";
        case RBLXWIN_KEY_DELETE:       return "DELETE";
        case RBLXWIN_KEY_PAGE_UP:      return "PAGE UP";
        case RBLXWIN_KEY_PAGE_DOWN:    return "PAGE DOWN";
        case RBLXWIN_KEY_HOME:         return "HOME";
        case RBLXWIN_KEY_END:          return "END";
        case RBLXWIN_KEY_KP_0:         return "KEYPAD 0";
        case RBLXWIN_KEY_KP_1:         return "KEYPAD 1";
        case RBLXWIN_KEY_KP_2:         return "KEYPAD 2";
        case RBLXWIN_KEY_KP_3:         return "KEYPAD 3";
        case RBLXWIN_KEY_KP_4:         return "KEYPAD 4";
        case RBLXWIN_KEY_KP_5:         return "KEYPAD 5";
        case RBLXWIN_KEY_KP_6:         return "KEYPAD 6";
        case RBLXWIN_KEY_KP_7:         return "KEYPAD 7";
        case RBLXWIN_KEY_KP_8:         return "KEYPAD 8";
        case RBLXWIN_KEY_KP_9:         return "KEYPAD 9";
        case RBLXWIN_KEY_KP_DIVIDE:    return "KEYPAD DIVIDE";
        case RBLXWIN_KEY_KP_MULTIPLY:  return "KEYPAD MULTIPLY";
        case RBLXWIN_KEY_KP_SUBTRACT:  return "KEYPAD SUBTRACT";
        case RBLXWIN_KEY_KP_ADD:       return "KEYPAD ADD";
        case RBLXWIN_KEY_KP_DECIMAL:   return "KEYPAD DECIMAL";
        case RBLXWIN_KEY_KP_EQUAL:     return "KEYPAD EQUAL";
        case RBLXWIN_KEY_KP_ENTER:     return "KEYPAD ENTER";
        case RBLXWIN_KEY_PRINT_SCREEN: return "PRINT SCREEN";
        case RBLXWIN_KEY_NUM_LOCK:     return "NUM LOCK";
        case RBLXWIN_KEY_CAPS_LOCK:    return "CAPS LOCK";
        case RBLXWIN_KEY_SCROLL_LOCK:  return "SCROLL LOCK";
        case RBLXWIN_KEY_PAUSE:        return "PAUSE";
        case RBLXWIN_KEY_LEFT_SUPER:   return "LEFT SUPER";
        case RBLXWIN_KEY_RIGHT_SUPER:  return "RIGHT SUPER";
        case RBLXWIN_KEY_MENU:         return "MENU";

        default:                    return "UNKNOWN";
    }
}

static const char* get_action_name(int action)
{
    switch (action)
    {
        case RBLXWIN_PRESS:
            return "pressed";
        case RBLXWIN_RELEASE:
            return "released";
        case RBLXWIN_REPEAT:
            return "repeated";
    }

    return "caused unknown action";
}

static const char* get_button_name(int button)
{
    switch (button)
    {
        case RBLXWIN_MOUSE_BUTTON_LEFT:
            return "left";
        case RBLXWIN_MOUSE_BUTTON_RIGHT:
            return "right";
        case RBLXWIN_MOUSE_BUTTON_MIDDLE:
            return "middle";
        default:
        {
            static char name[16];
            snprintf(name, sizeof(name), "%i", button);
            return name;
        }
    }
}

static const char* get_mods_name(int mods)
{
    static char name[512];

    if (mods == 0)
        return " no mods";

    name[0] = '\0';

    if (mods & RBLXWIN_MOD_SHIFT)
        strcat(name, " shift");
    if (mods & RBLXWIN_MOD_CONTROL)
        strcat(name, " control");
    if (mods & RBLXWIN_MOD_ALT)
        strcat(name, " alt");
    if (mods & RBLXWIN_MOD_SUPER)
        strcat(name, " super");
    if (mods & RBLXWIN_MOD_CAPS_LOCK)
        strcat(name, " capslock-on");
    if (mods & RBLXWIN_MOD_NUM_LOCK)
        strcat(name, " numlock-on");

    return name;
}

static size_t encode_utf8(char* s, unsigned int ch)
{
    size_t count = 0;

    if (ch < 0x80)
        s[count++] = (char) ch;
    else if (ch < 0x800)
    {
        s[count++] = (ch >> 6) | 0xc0;
        s[count++] = (ch & 0x3f) | 0x80;
    }
    else if (ch < 0x10000)
    {
        s[count++] = (ch >> 12) | 0xe0;
        s[count++] = ((ch >> 6) & 0x3f) | 0x80;
        s[count++] = (ch & 0x3f) | 0x80;
    }
    else if (ch < 0x110000)
    {
        s[count++] = (ch >> 18) | 0xf0;
        s[count++] = ((ch >> 12) & 0x3f) | 0x80;
        s[count++] = ((ch >> 6) & 0x3f) | 0x80;
        s[count++] = (ch & 0x3f) | 0x80;
    }

    return count;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void window_pos_callback(RBLXWINwindow* window, int x, int y)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window position: %i %i\n",
           counter++, slot->number, rblxwinGetTime(), x, y);
}

static void window_size_callback(RBLXWINwindow* window, int width, int height)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window size: %i %i\n",
           counter++, slot->number, rblxwinGetTime(), width, height);
}

static void framebuffer_size_callback(RBLXWINwindow* window, int width, int height)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Framebuffer size: %i %i\n",
           counter++, slot->number, rblxwinGetTime(), width, height);
}

static void window_content_scale_callback(RBLXWINwindow* window, float xscale, float yscale)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window content scale: %0.3f %0.3f\n",
           counter++, slot->number, rblxwinGetTime(), xscale, yscale);
}

static void window_close_callback(RBLXWINwindow* window)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window close\n",
           counter++, slot->number, rblxwinGetTime());

    rblxwinSetWindowShouldClose(window, slot->closeable);
}

static void window_refresh_callback(RBLXWINwindow* window)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window refresh\n",
           counter++, slot->number, rblxwinGetTime());

    rblxwinMakeContextCurrent(window);
    glClear(GL_COLOR_BUFFER_BIT);
    rblxwinSwapBuffers(window);
}

static void window_focus_callback(RBLXWINwindow* window, int focused)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window %s\n",
           counter++, slot->number, rblxwinGetTime(),
           focused ? "focused" : "defocused");
}

static void window_iconify_callback(RBLXWINwindow* window, int iconified)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window was %s\n",
           counter++, slot->number, rblxwinGetTime(),
           iconified ? "iconified" : "uniconified");
}

static void window_maximize_callback(RBLXWINwindow* window, int maximized)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window was %s\n",
           counter++, slot->number, rblxwinGetTime(),
           maximized ? "maximized" : "unmaximized");
}

static void mouse_button_callback(RBLXWINwindow* window, int button, int action, int mods)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Mouse button %i (%s) (with%s) was %s\n",
           counter++, slot->number, rblxwinGetTime(), button,
           get_button_name(button),
           get_mods_name(mods),
           get_action_name(action));
}

static void cursor_position_callback(RBLXWINwindow* window, double x, double y)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Cursor position: %f %f\n",
           counter++, slot->number, rblxwinGetTime(), x, y);
}

static void cursor_enter_callback(RBLXWINwindow* window, int entered)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Cursor %s window\n",
           counter++, slot->number, rblxwinGetTime(),
           entered ? "entered" : "left");
}

static void scroll_callback(RBLXWINwindow* window, double x, double y)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Scroll: %0.3f %0.3f\n",
           counter++, slot->number, rblxwinGetTime(), x, y);
}

static void key_callback(RBLXWINwindow* window, int key, int scancode, int action, int mods)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    const char* name = rblxwinGetKeyName(key, scancode);

    if (name)
    {
        printf("%08x to %i at %0.3f: Key 0x%04x Scancode 0x%04x (%s) (%s) (with%s) was %s\n",
               counter++, slot->number, rblxwinGetTime(), key, scancode,
               get_key_name(key),
               name,
               get_mods_name(mods),
               get_action_name(action));
    }
    else
    {
        printf("%08x to %i at %0.3f: Key 0x%04x Scancode 0x%04x (%s) (with%s) was %s\n",
               counter++, slot->number, rblxwinGetTime(), key, scancode,
               get_key_name(key),
               get_mods_name(mods),
               get_action_name(action));
    }

    if (action != RBLXWIN_PRESS)
        return;

    switch (key)
    {
        case RBLXWIN_KEY_C:
        {
            slot->closeable = !slot->closeable;

            printf("(( closing %s ))\n", slot->closeable ? "enabled" : "disabled");
            break;
        }

        case RBLXWIN_KEY_L:
        {
            const int state = rblxwinGetInputMode(window, RBLXWIN_LOCK_KEY_MODS);
            rblxwinSetInputMode(window, RBLXWIN_LOCK_KEY_MODS, !state);

            printf("(( lock key mods %s ))\n", !state ? "enabled" : "disabled");
            break;
        }
    }
}

static void char_callback(RBLXWINwindow* window, unsigned int codepoint)
{
    Slot* slot = rblxwinGetWindowUserPointer(window);
    char string[5] = "";

    encode_utf8(string, codepoint);
    printf("%08x to %i at %0.3f: Character 0x%08x (%s) input\n",
           counter++, slot->number, rblxwinGetTime(), codepoint, string);
}

static void drop_callback(RBLXWINwindow* window, int count, const char* paths[])
{
    int i;
    Slot* slot = rblxwinGetWindowUserPointer(window);

    printf("%08x to %i at %0.3f: Drop input\n",
           counter++, slot->number, rblxwinGetTime());

    for (i = 0;  i < count;  i++)
        printf("  %i: \"%s\"\n", i, paths[i]);
}

static void monitor_callback(RBLXWINmonitor* monitor, int event)
{
    if (event == RBLXWIN_CONNECTED)
    {
        int x, y, widthMM, heightMM;
        const RBLXWINvidmode* mode = rblxwinGetVideoMode(monitor);

        rblxwinGetMonitorPos(monitor, &x, &y);
        rblxwinGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);

        printf("%08x at %0.3f: Monitor %s (%ix%i at %ix%i, %ix%i mm) was connected\n",
               counter++,
               rblxwinGetTime(),
               rblxwinGetMonitorName(monitor),
               mode->width, mode->height,
               x, y,
               widthMM, heightMM);
    }
    else if (event == RBLXWIN_DISCONNECTED)
    {
        printf("%08x at %0.3f: Monitor %s was disconnected\n",
               counter++,
               rblxwinGetTime(),
               rblxwinGetMonitorName(monitor));
    }
}

static void joystick_callback(int jid, int event)
{
    if (event == RBLXWIN_CONNECTED)
    {
        int axisCount, buttonCount, hatCount;

        rblxwinGetJoystickAxes(jid, &axisCount);
        rblxwinGetJoystickButtons(jid, &buttonCount);
        rblxwinGetJoystickHats(jid, &hatCount);

        printf("%08x at %0.3f: Joystick %i (%s) was connected with %i axes, %i buttons, and %i hats\n",
               counter++, rblxwinGetTime(),
               jid,
               rblxwinGetJoystickName(jid),
               axisCount,
               buttonCount,
               hatCount);
    }
    else
    {
        printf("%08x at %0.3f: Joystick %i was disconnected\n",
               counter++, rblxwinGetTime(), jid);
    }
}

int main(int argc, char** argv)
{
    Slot* slots;
    RBLXWINmonitor* monitor = NULL;
    int ch, i, width, height, count = 1;

    rblxwinSetErrorCallback(error_callback);

    if (!rblxwinInit())
        exit(EXIT_FAILURE);

    printf("Library initialized\n");

    rblxwinSetMonitorCallback(monitor_callback);
    rblxwinSetJoystickCallback(joystick_callback);

    while ((ch = getopt(argc, argv, "hfn:")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 'f':
                monitor = rblxwinGetPrimaryMonitor();
                break;

            case 'n':
                count = (int) strtoul(optarg, NULL, 10);
                break;

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    if (monitor)
    {
        const RBLXWINvidmode* mode = rblxwinGetVideoMode(monitor);

        rblxwinWindowHint(RBLXWIN_REFRESH_RATE, mode->refreshRate);
        rblxwinWindowHint(RBLXWIN_RED_BITS, mode->redBits);
        rblxwinWindowHint(RBLXWIN_GREEN_BITS, mode->greenBits);
        rblxwinWindowHint(RBLXWIN_BLUE_BITS, mode->blueBits);

        width = mode->width;
        height = mode->height;
    }
    else
    {
        width  = 640;
        height = 480;
    }

    slots = calloc(count, sizeof(Slot));

    for (i = 0;  i < count;  i++)
    {
        char title[128];

        slots[i].closeable = RBLXWIN_TRUE;
        slots[i].number = i + 1;

        snprintf(title, sizeof(title), "Event Linter (Window %i)", slots[i].number);

        if (monitor)
        {
            printf("Creating full screen window %i (%ix%i on %s)\n",
                   slots[i].number,
                   width, height,
                   rblxwinGetMonitorName(monitor));
        }
        else
        {
            printf("Creating windowed mode window %i (%ix%i)\n",
                   slots[i].number,
                   width, height);
        }

        slots[i].window = rblxwinCreateWindow(width, height, title, monitor, NULL);
        if (!slots[i].window)
        {
            free(slots);
            rblxwinTerminate();
            exit(EXIT_FAILURE);
        }

        rblxwinSetWindowUserPointer(slots[i].window, slots + i);

        rblxwinSetWindowPosCallback(slots[i].window, window_pos_callback);
        rblxwinSetWindowSizeCallback(slots[i].window, window_size_callback);
        rblxwinSetFramebufferSizeCallback(slots[i].window, framebuffer_size_callback);
        rblxwinSetWindowContentScaleCallback(slots[i].window, window_content_scale_callback);
        rblxwinSetWindowCloseCallback(slots[i].window, window_close_callback);
        rblxwinSetWindowRefreshCallback(slots[i].window, window_refresh_callback);
        rblxwinSetWindowFocusCallback(slots[i].window, window_focus_callback);
        rblxwinSetWindowIconifyCallback(slots[i].window, window_iconify_callback);
        rblxwinSetWindowMaximizeCallback(slots[i].window, window_maximize_callback);
        rblxwinSetMouseButtonCallback(slots[i].window, mouse_button_callback);
        rblxwinSetCursorPosCallback(slots[i].window, cursor_position_callback);
        rblxwinSetCursorEnterCallback(slots[i].window, cursor_enter_callback);
        rblxwinSetScrollCallback(slots[i].window, scroll_callback);
        rblxwinSetKeyCallback(slots[i].window, key_callback);
        rblxwinSetCharCallback(slots[i].window, char_callback);
        rblxwinSetDropCallback(slots[i].window, drop_callback);

        rblxwinMakeContextCurrent(slots[i].window);
        rblxglLoadGL(rblxwinGetProcAddress);
        rblxwinSwapInterval(1);
    }

    printf("Main loop starting\n");

    for (;;)
    {
        for (i = 0;  i < count;  i++)
        {
            if (rblxwinWindowShouldClose(slots[i].window))
                break;
        }

        if (i < count)
            break;

        rblxwinWaitEvents();

        // Workaround for an issue with msvcrt and mintty
        fflush(stdout);
    }

    free(slots);
    rblxwinTerminate();
    exit(EXIT_SUCCESS);
}

