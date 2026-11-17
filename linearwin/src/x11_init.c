//========================================================================
// LINEARWIN 3.3 X11 - www.linearwin.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@linearwin.org>
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <X11/Xresource.h>

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <locale.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>


// Translate the X11 KeySyms for a key to a LINEARWIN key code
// NOTE: This is only used as a fallback, in case the XKB method fails
//       It is layout-dependent and will fail partially on most non-US layouts
//
static int translateKeySyms(const KeySym* keysyms, int width)
{
    if (width > 1)
    {
        switch (keysyms[1])
        {
            case XK_KP_0:           return LINEARWIN_KEY_KP_0;
            case XK_KP_1:           return LINEARWIN_KEY_KP_1;
            case XK_KP_2:           return LINEARWIN_KEY_KP_2;
            case XK_KP_3:           return LINEARWIN_KEY_KP_3;
            case XK_KP_4:           return LINEARWIN_KEY_KP_4;
            case XK_KP_5:           return LINEARWIN_KEY_KP_5;
            case XK_KP_6:           return LINEARWIN_KEY_KP_6;
            case XK_KP_7:           return LINEARWIN_KEY_KP_7;
            case XK_KP_8:           return LINEARWIN_KEY_KP_8;
            case XK_KP_9:           return LINEARWIN_KEY_KP_9;
            case XK_KP_Separator:
            case XK_KP_Decimal:     return LINEARWIN_KEY_KP_DECIMAL;
            case XK_KP_Equal:       return LINEARWIN_KEY_KP_EQUAL;
            case XK_KP_Enter:       return LINEARWIN_KEY_KP_ENTER;
            default:                break;
        }
    }

    switch (keysyms[0])
    {
        case XK_Escape:         return LINEARWIN_KEY_ESCAPE;
        case XK_Tab:            return LINEARWIN_KEY_TAB;
        case XK_Shift_L:        return LINEARWIN_KEY_LEFT_SHIFT;
        case XK_Shift_R:        return LINEARWIN_KEY_RIGHT_SHIFT;
        case XK_Control_L:      return LINEARWIN_KEY_LEFT_CONTROL;
        case XK_Control_R:      return LINEARWIN_KEY_RIGHT_CONTROL;
        case XK_Meta_L:
        case XK_Alt_L:          return LINEARWIN_KEY_LEFT_ALT;
        case XK_Mode_switch: // Mapped to Alt_R on many keyboards
        case XK_ISO_Level3_Shift: // AltGr on at least some machines
        case XK_Meta_R:
        case XK_Alt_R:          return LINEARWIN_KEY_RIGHT_ALT;
        case XK_Super_L:        return LINEARWIN_KEY_LEFT_SUPER;
        case XK_Super_R:        return LINEARWIN_KEY_RIGHT_SUPER;
        case XK_Menu:           return LINEARWIN_KEY_MENU;
        case XK_Num_Lock:       return LINEARWIN_KEY_NUM_LOCK;
        case XK_Caps_Lock:      return LINEARWIN_KEY_CAPS_LOCK;
        case XK_Print:          return LINEARWIN_KEY_PRINT_SCREEN;
        case XK_Scroll_Lock:    return LINEARWIN_KEY_SCROLL_LOCK;
        case XK_Pause:          return LINEARWIN_KEY_PAUSE;
        case XK_Delete:         return LINEARWIN_KEY_DELETE;
        case XK_BackSpace:      return LINEARWIN_KEY_BACKSPACE;
        case XK_Return:         return LINEARWIN_KEY_ENTER;
        case XK_Home:           return LINEARWIN_KEY_HOME;
        case XK_End:            return LINEARWIN_KEY_END;
        case XK_Page_Up:        return LINEARWIN_KEY_PAGE_UP;
        case XK_Page_Down:      return LINEARWIN_KEY_PAGE_DOWN;
        case XK_Insert:         return LINEARWIN_KEY_INSERT;
        case XK_Left:           return LINEARWIN_KEY_LEFT;
        case XK_Right:          return LINEARWIN_KEY_RIGHT;
        case XK_Down:           return LINEARWIN_KEY_DOWN;
        case XK_Up:             return LINEARWIN_KEY_UP;
        case XK_F1:             return LINEARWIN_KEY_F1;
        case XK_F2:             return LINEARWIN_KEY_F2;
        case XK_F3:             return LINEARWIN_KEY_F3;
        case XK_F4:             return LINEARWIN_KEY_F4;
        case XK_F5:             return LINEARWIN_KEY_F5;
        case XK_F6:             return LINEARWIN_KEY_F6;
        case XK_F7:             return LINEARWIN_KEY_F7;
        case XK_F8:             return LINEARWIN_KEY_F8;
        case XK_F9:             return LINEARWIN_KEY_F9;
        case XK_F10:            return LINEARWIN_KEY_F10;
        case XK_F11:            return LINEARWIN_KEY_F11;
        case XK_F12:            return LINEARWIN_KEY_F12;
        case XK_F13:            return LINEARWIN_KEY_F13;
        case XK_F14:            return LINEARWIN_KEY_F14;
        case XK_F15:            return LINEARWIN_KEY_F15;
        case XK_F16:            return LINEARWIN_KEY_F16;
        case XK_F17:            return LINEARWIN_KEY_F17;
        case XK_F18:            return LINEARWIN_KEY_F18;
        case XK_F19:            return LINEARWIN_KEY_F19;
        case XK_F20:            return LINEARWIN_KEY_F20;
        case XK_F21:            return LINEARWIN_KEY_F21;
        case XK_F22:            return LINEARWIN_KEY_F22;
        case XK_F23:            return LINEARWIN_KEY_F23;
        case XK_F24:            return LINEARWIN_KEY_F24;
        case XK_F25:            return LINEARWIN_KEY_F25;

        // Numeric keypad
        case XK_KP_Divide:      return LINEARWIN_KEY_KP_DIVIDE;
        case XK_KP_Multiply:    return LINEARWIN_KEY_KP_MULTIPLY;
        case XK_KP_Subtract:    return LINEARWIN_KEY_KP_SUBTRACT;
        case XK_KP_Add:         return LINEARWIN_KEY_KP_ADD;

        // These should have been detected in secondary keysym test above!
        case XK_KP_Insert:      return LINEARWIN_KEY_KP_0;
        case XK_KP_End:         return LINEARWIN_KEY_KP_1;
        case XK_KP_Down:        return LINEARWIN_KEY_KP_2;
        case XK_KP_Page_Down:   return LINEARWIN_KEY_KP_3;
        case XK_KP_Left:        return LINEARWIN_KEY_KP_4;
        case XK_KP_Right:       return LINEARWIN_KEY_KP_6;
        case XK_KP_Home:        return LINEARWIN_KEY_KP_7;
        case XK_KP_Up:          return LINEARWIN_KEY_KP_8;
        case XK_KP_Page_Up:     return LINEARWIN_KEY_KP_9;
        case XK_KP_Delete:      return LINEARWIN_KEY_KP_DECIMAL;
        case XK_KP_Equal:       return LINEARWIN_KEY_KP_EQUAL;
        case XK_KP_Enter:       return LINEARWIN_KEY_KP_ENTER;

        // Last resort: Check for printable keys (should not happen if the XKB
        // extension is available). This will give a layout dependent mapping
        // (which is wrong, and we may miss some keys, especially on non-US
        // keyboards), but it's better than nothing...
        case XK_a:              return LINEARWIN_KEY_A;
        case XK_b:              return LINEARWIN_KEY_B;
        case XK_c:              return LINEARWIN_KEY_C;
        case XK_d:              return LINEARWIN_KEY_D;
        case XK_e:              return LINEARWIN_KEY_E;
        case XK_f:              return LINEARWIN_KEY_F;
        case XK_g:              return LINEARWIN_KEY_G;
        case XK_h:              return LINEARWIN_KEY_H;
        case XK_i:              return LINEARWIN_KEY_I;
        case XK_j:              return LINEARWIN_KEY_J;
        case XK_k:              return LINEARWIN_KEY_K;
        case XK_l:              return LINEARWIN_KEY_L;
        case XK_m:              return LINEARWIN_KEY_M;
        case XK_n:              return LINEARWIN_KEY_N;
        case XK_o:              return LINEARWIN_KEY_O;
        case XK_p:              return LINEARWIN_KEY_P;
        case XK_q:              return LINEARWIN_KEY_Q;
        case XK_r:              return LINEARWIN_KEY_R;
        case XK_s:              return LINEARWIN_KEY_S;
        case XK_t:              return LINEARWIN_KEY_T;
        case XK_u:              return LINEARWIN_KEY_U;
        case XK_v:              return LINEARWIN_KEY_V;
        case XK_w:              return LINEARWIN_KEY_W;
        case XK_x:              return LINEARWIN_KEY_X;
        case XK_y:              return LINEARWIN_KEY_Y;
        case XK_z:              return LINEARWIN_KEY_Z;
        case XK_1:              return LINEARWIN_KEY_1;
        case XK_2:              return LINEARWIN_KEY_2;
        case XK_3:              return LINEARWIN_KEY_3;
        case XK_4:              return LINEARWIN_KEY_4;
        case XK_5:              return LINEARWIN_KEY_5;
        case XK_6:              return LINEARWIN_KEY_6;
        case XK_7:              return LINEARWIN_KEY_7;
        case XK_8:              return LINEARWIN_KEY_8;
        case XK_9:              return LINEARWIN_KEY_9;
        case XK_0:              return LINEARWIN_KEY_0;
        case XK_space:          return LINEARWIN_KEY_SPACE;
        case XK_minus:          return LINEARWIN_KEY_MINUS;
        case XK_equal:          return LINEARWIN_KEY_EQUAL;
        case XK_bracketleft:    return LINEARWIN_KEY_LEFT_BRACKET;
        case XK_bracketright:   return LINEARWIN_KEY_RIGHT_BRACKET;
        case XK_backslash:      return LINEARWIN_KEY_BACKSLASH;
        case XK_semicolon:      return LINEARWIN_KEY_SEMICOLON;
        case XK_apostrophe:     return LINEARWIN_KEY_APOSTROPHE;
        case XK_grave:          return LINEARWIN_KEY_GRAVE_ACCENT;
        case XK_comma:          return LINEARWIN_KEY_COMMA;
        case XK_period:         return LINEARWIN_KEY_PERIOD;
        case XK_slash:          return LINEARWIN_KEY_SLASH;
        case XK_less:           return LINEARWIN_KEY_WORLD_1; // At least in some layouts...
        default:                break;
    }

    // No matching translation was found
    return LINEARWIN_KEY_UNKNOWN;
}

// Create key code translation tables
//
static void createKeyTables(void)
{
    int scancode, scancodeMin, scancodeMax;

    memset(_linearwin.x11.keycodes, -1, sizeof(_linearwin.x11.keycodes));
    memset(_linearwin.x11.scancodes, -1, sizeof(_linearwin.x11.scancodes));

    if (_linearwin.x11.xkb.available)
    {
        // Use XKB to determine physical key locations independently of the
        // current keyboard layout

        XkbDescPtr desc = XkbGetMap(_linearwin.x11.display, 0, XkbUseCoreKbd);
        XkbGetNames(_linearwin.x11.display, XkbKeyNamesMask | XkbKeyAliasesMask, desc);

        scancodeMin = desc->min_key_code;
        scancodeMax = desc->max_key_code;

        const struct
        {
            int key;
            char* name;
        } keymap[] =
        {
            { LINEARWIN_KEY_GRAVE_ACCENT, "TLDE" },
            { LINEARWIN_KEY_1, "AE01" },
            { LINEARWIN_KEY_2, "AE02" },
            { LINEARWIN_KEY_3, "AE03" },
            { LINEARWIN_KEY_4, "AE04" },
            { LINEARWIN_KEY_5, "AE05" },
            { LINEARWIN_KEY_6, "AE06" },
            { LINEARWIN_KEY_7, "AE07" },
            { LINEARWIN_KEY_8, "AE08" },
            { LINEARWIN_KEY_9, "AE09" },
            { LINEARWIN_KEY_0, "AE10" },
            { LINEARWIN_KEY_MINUS, "AE11" },
            { LINEARWIN_KEY_EQUAL, "AE12" },
            { LINEARWIN_KEY_Q, "AD01" },
            { LINEARWIN_KEY_W, "AD02" },
            { LINEARWIN_KEY_E, "AD03" },
            { LINEARWIN_KEY_R, "AD04" },
            { LINEARWIN_KEY_T, "AD05" },
            { LINEARWIN_KEY_Y, "AD06" },
            { LINEARWIN_KEY_U, "AD07" },
            { LINEARWIN_KEY_I, "AD08" },
            { LINEARWIN_KEY_O, "AD09" },
            { LINEARWIN_KEY_P, "AD10" },
            { LINEARWIN_KEY_LEFT_BRACKET, "AD11" },
            { LINEARWIN_KEY_RIGHT_BRACKET, "AD12" },
            { LINEARWIN_KEY_A, "AC01" },
            { LINEARWIN_KEY_S, "AC02" },
            { LINEARWIN_KEY_D, "AC03" },
            { LINEARWIN_KEY_F, "AC04" },
            { LINEARWIN_KEY_G, "AC05" },
            { LINEARWIN_KEY_H, "AC06" },
            { LINEARWIN_KEY_J, "AC07" },
            { LINEARWIN_KEY_K, "AC08" },
            { LINEARWIN_KEY_L, "AC09" },
            { LINEARWIN_KEY_SEMICOLON, "AC10" },
            { LINEARWIN_KEY_APOSTROPHE, "AC11" },
            { LINEARWIN_KEY_Z, "AB01" },
            { LINEARWIN_KEY_X, "AB02" },
            { LINEARWIN_KEY_C, "AB03" },
            { LINEARWIN_KEY_V, "AB04" },
            { LINEARWIN_KEY_B, "AB05" },
            { LINEARWIN_KEY_N, "AB06" },
            { LINEARWIN_KEY_M, "AB07" },
            { LINEARWIN_KEY_COMMA, "AB08" },
            { LINEARWIN_KEY_PERIOD, "AB09" },
            { LINEARWIN_KEY_SLASH, "AB10" },
            { LINEARWIN_KEY_BACKSLASH, "BKSL" },
            { LINEARWIN_KEY_WORLD_1, "LSGT" },
            { LINEARWIN_KEY_SPACE, "SPCE" },
            { LINEARWIN_KEY_ESCAPE, "ESC" },
            { LINEARWIN_KEY_ENTER, "RTRN" },
            { LINEARWIN_KEY_TAB, "TAB" },
            { LINEARWIN_KEY_BACKSPACE, "BKSP" },
            { LINEARWIN_KEY_INSERT, "INS" },
            { LINEARWIN_KEY_DELETE, "DELE" },
            { LINEARWIN_KEY_RIGHT, "RGHT" },
            { LINEARWIN_KEY_LEFT, "LEFT" },
            { LINEARWIN_KEY_DOWN, "DOWN" },
            { LINEARWIN_KEY_UP, "UP" },
            { LINEARWIN_KEY_PAGE_UP, "PGUP" },
            { LINEARWIN_KEY_PAGE_DOWN, "PGDN" },
            { LINEARWIN_KEY_HOME, "HOME" },
            { LINEARWIN_KEY_END, "END" },
            { LINEARWIN_KEY_CAPS_LOCK, "CAPS" },
            { LINEARWIN_KEY_SCROLL_LOCK, "SCLK" },
            { LINEARWIN_KEY_NUM_LOCK, "NMLK" },
            { LINEARWIN_KEY_PRINT_SCREEN, "PRSC" },
            { LINEARWIN_KEY_PAUSE, "PAUS" },
            { LINEARWIN_KEY_F1, "FK01" },
            { LINEARWIN_KEY_F2, "FK02" },
            { LINEARWIN_KEY_F3, "FK03" },
            { LINEARWIN_KEY_F4, "FK04" },
            { LINEARWIN_KEY_F5, "FK05" },
            { LINEARWIN_KEY_F6, "FK06" },
            { LINEARWIN_KEY_F7, "FK07" },
            { LINEARWIN_KEY_F8, "FK08" },
            { LINEARWIN_KEY_F9, "FK09" },
            { LINEARWIN_KEY_F10, "FK10" },
            { LINEARWIN_KEY_F11, "FK11" },
            { LINEARWIN_KEY_F12, "FK12" },
            { LINEARWIN_KEY_F13, "FK13" },
            { LINEARWIN_KEY_F14, "FK14" },
            { LINEARWIN_KEY_F15, "FK15" },
            { LINEARWIN_KEY_F16, "FK16" },
            { LINEARWIN_KEY_F17, "FK17" },
            { LINEARWIN_KEY_F18, "FK18" },
            { LINEARWIN_KEY_F19, "FK19" },
            { LINEARWIN_KEY_F20, "FK20" },
            { LINEARWIN_KEY_F21, "FK21" },
            { LINEARWIN_KEY_F22, "FK22" },
            { LINEARWIN_KEY_F23, "FK23" },
            { LINEARWIN_KEY_F24, "FK24" },
            { LINEARWIN_KEY_F25, "FK25" },
            { LINEARWIN_KEY_KP_0, "KP0" },
            { LINEARWIN_KEY_KP_1, "KP1" },
            { LINEARWIN_KEY_KP_2, "KP2" },
            { LINEARWIN_KEY_KP_3, "KP3" },
            { LINEARWIN_KEY_KP_4, "KP4" },
            { LINEARWIN_KEY_KP_5, "KP5" },
            { LINEARWIN_KEY_KP_6, "KP6" },
            { LINEARWIN_KEY_KP_7, "KP7" },
            { LINEARWIN_KEY_KP_8, "KP8" },
            { LINEARWIN_KEY_KP_9, "KP9" },
            { LINEARWIN_KEY_KP_DECIMAL, "KPDL" },
            { LINEARWIN_KEY_KP_DIVIDE, "KPDV" },
            { LINEARWIN_KEY_KP_MULTIPLY, "KPMU" },
            { LINEARWIN_KEY_KP_SUBTRACT, "KPSU" },
            { LINEARWIN_KEY_KP_ADD, "KPAD" },
            { LINEARWIN_KEY_KP_ENTER, "KPEN" },
            { LINEARWIN_KEY_KP_EQUAL, "KPEQ" },
            { LINEARWIN_KEY_LEFT_SHIFT, "LFSH" },
            { LINEARWIN_KEY_LEFT_CONTROL, "LCTL" },
            { LINEARWIN_KEY_LEFT_ALT, "LALT" },
            { LINEARWIN_KEY_LEFT_SUPER, "LWIN" },
            { LINEARWIN_KEY_RIGHT_SHIFT, "RTSH" },
            { LINEARWIN_KEY_RIGHT_CONTROL, "RCTL" },
            { LINEARWIN_KEY_RIGHT_ALT, "RALT" },
            { LINEARWIN_KEY_RIGHT_ALT, "LVL3" },
            { LINEARWIN_KEY_RIGHT_ALT, "MDSW" },
            { LINEARWIN_KEY_RIGHT_SUPER, "RWIN" },
            { LINEARWIN_KEY_MENU, "MENU" }
        };

        // Find the X11 key code -> LINEARWIN key code mapping
        for (scancode = scancodeMin;  scancode <= scancodeMax;  scancode++)
        {
            int key = LINEARWIN_KEY_UNKNOWN;

            // Map the key name to a LINEARWIN key code. Note: We use the US
            // keyboard layout. Because function keys aren't mapped correctly
            // when using traditional KeySym translations, they are mapped
            // here instead.
            for (int i = 0;  i < sizeof(keymap) / sizeof(keymap[0]);  i++)
            {
                if (strncmp(desc->names->keys[scancode].name,
                            keymap[i].name,
                            XkbKeyNameLength) == 0)
                {
                    key = keymap[i].key;
                    break;
                }
            }

            // Fall back to key aliases in case the key name did not match
            for (int i = 0;  i < desc->names->num_key_aliases;  i++)
            {
                if (key != LINEARWIN_KEY_UNKNOWN)
                    break;

                if (strncmp(desc->names->key_aliases[i].real,
                            desc->names->keys[scancode].name,
                            XkbKeyNameLength) != 0)
                {
                    continue;
                }

                for (int j = 0;  j < sizeof(keymap) / sizeof(keymap[0]);  j++)
                {
                    if (strncmp(desc->names->key_aliases[i].alias,
                                keymap[j].name,
                                XkbKeyNameLength) == 0)
                    {
                        key = keymap[j].key;
                        break;
                    }
                }
            }

            _linearwin.x11.keycodes[scancode] = key;
        }

        XkbFreeNames(desc, XkbKeyNamesMask, True);
        XkbFreeKeyboard(desc, 0, True);
    }
    else
        XDisplayKeycodes(_linearwin.x11.display, &scancodeMin, &scancodeMax);

    int width;
    KeySym* keysyms = XGetKeyboardMapping(_linearwin.x11.display,
                                          scancodeMin,
                                          scancodeMax - scancodeMin + 1,
                                          &width);

    for (scancode = scancodeMin;  scancode <= scancodeMax;  scancode++)
    {
        // Translate the un-translated key codes using traditional X11 KeySym
        // lookups
        if (_linearwin.x11.keycodes[scancode] < 0)
        {
            const size_t base = (scancode - scancodeMin) * width;
            _linearwin.x11.keycodes[scancode] = translateKeySyms(&keysyms[base], width);
        }

        // Store the reverse translation for faster key name lookup
        if (_linearwin.x11.keycodes[scancode] > 0)
            _linearwin.x11.scancodes[_linearwin.x11.keycodes[scancode]] = scancode;
    }

    XFree(keysyms);
}

// Check whether the IM has a usable style
//
static LINEARWINbool hasUsableInputMethodStyle(void)
{
    LINEARWINbool found = LINEARWIN_FALSE;
    XIMStyles* styles = NULL;

    if (XGetIMValues(_linearwin.x11.im, XNQueryInputStyle, &styles, NULL) != NULL)
        return LINEARWIN_FALSE;

    for (unsigned int i = 0;  i < styles->count_styles;  i++)
    {
        if (styles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing))
        {
            found = LINEARWIN_TRUE;
            break;
        }
    }

    XFree(styles);
    return found;
}

// Check whether the specified atom is supported
//
static Atom getAtomIfSupported(Atom* supportedAtoms,
                               unsigned long atomCount,
                               const char* atomName)
{
    const Atom atom = XInternAtom(_linearwin.x11.display, atomName, False);

    for (unsigned long i = 0;  i < atomCount;  i++)
    {
        if (supportedAtoms[i] == atom)
            return atom;
    }

    return None;
}

// Check whether the running window manager is EWMH-compliant
//
static void detectEWMH(void)
{
    // First we read the _NET_SUPPORTING_WM_CHECK property on the root window

    Window* windowFromRoot = NULL;
    if (!_linearwinGetWindowPropertyX11(_linearwin.x11.root,
                                   _linearwin.x11.NET_SUPPORTING_WM_CHECK,
                                   XA_WINDOW,
                                   (unsigned char**) &windowFromRoot))
    {
        return;
    }

    _linearwinGrabErrorHandlerX11();

    // If it exists, it should be the XID of a top-level window
    // Then we look for the same property on that window

    Window* windowFromChild = NULL;
    if (!_linearwinGetWindowPropertyX11(*windowFromRoot,
                                   _linearwin.x11.NET_SUPPORTING_WM_CHECK,
                                   XA_WINDOW,
                                   (unsigned char**) &windowFromChild))
    {
        XFree(windowFromRoot);
        return;
    }

    _linearwinReleaseErrorHandlerX11();

    // If the property exists, it should contain the XID of the window

    if (*windowFromRoot != *windowFromChild)
    {
        XFree(windowFromRoot);
        XFree(windowFromChild);
        return;
    }

    XFree(windowFromRoot);
    XFree(windowFromChild);

    // We are now fairly sure that an EWMH-compliant WM is currently running
    // We can now start querying the WM about what features it supports by
    // looking in the _NET_SUPPORTED property on the root window
    // It should contain a list of supported EWMH protocol and state atoms

    Atom* supportedAtoms = NULL;
    const unsigned long atomCount =
        _linearwinGetWindowPropertyX11(_linearwin.x11.root,
                                  _linearwin.x11.NET_SUPPORTED,
                                  XA_ATOM,
                                  (unsigned char**) &supportedAtoms);

    // See which of the atoms we support that are supported by the WM

    _linearwin.x11.NET_WM_STATE =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE");
    _linearwin.x11.NET_WM_STATE_ABOVE =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_ABOVE");
    _linearwin.x11.NET_WM_STATE_FULLSCREEN =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_FULLSCREEN");
    _linearwin.x11.NET_WM_STATE_MAXIMIZED_VERT =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_MAXIMIZED_VERT");
    _linearwin.x11.NET_WM_STATE_MAXIMIZED_HORZ =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_MAXIMIZED_HORZ");
    _linearwin.x11.NET_WM_STATE_DEMANDS_ATTENTION =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_DEMANDS_ATTENTION");
    _linearwin.x11.NET_WM_FULLSCREEN_MONITORS =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_FULLSCREEN_MONITORS");
    _linearwin.x11.NET_WM_WINDOW_TYPE =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_WINDOW_TYPE");
    _linearwin.x11.NET_WM_WINDOW_TYPE_NORMAL =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_WINDOW_TYPE_NORMAL");
    _linearwin.x11.NET_WORKAREA =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WORKAREA");
    _linearwin.x11.NET_CURRENT_DESKTOP =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_CURRENT_DESKTOP");
    _linearwin.x11.NET_ACTIVE_WINDOW =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_ACTIVE_WINDOW");
    _linearwin.x11.NET_FRAME_EXTENTS =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_FRAME_EXTENTS");
    _linearwin.x11.NET_REQUEST_FRAME_EXTENTS =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_REQUEST_FRAME_EXTENTS");

    if (supportedAtoms)
        XFree(supportedAtoms);
}

// Look for and initialize supported X11 extensions
//
static LINEARWINbool initExtensions(void)
{
#if defined(__OpenBSD__) || defined(__NetBSD__)
    _linearwin.x11.vidmode.handle = _linearwin_dlopen("libXxf86vm.so");
#else
    _linearwin.x11.vidmode.handle = _linearwin_dlopen("libXxf86vm.so.1");
#endif
    if (_linearwin.x11.vidmode.handle)
    {
        _linearwin.x11.vidmode.QueryExtension = (PFN_XF86VidModeQueryExtension)
            _linearwin_dlsym(_linearwin.x11.vidmode.handle, "XF86VidModeQueryExtension");
        _linearwin.x11.vidmode.GetGammaRamp = (PFN_XF86VidModeGetGammaRamp)
            _linearwin_dlsym(_linearwin.x11.vidmode.handle, "XF86VidModeGetGammaRamp");
        _linearwin.x11.vidmode.SetGammaRamp = (PFN_XF86VidModeSetGammaRamp)
            _linearwin_dlsym(_linearwin.x11.vidmode.handle, "XF86VidModeSetGammaRamp");
        _linearwin.x11.vidmode.GetGammaRampSize = (PFN_XF86VidModeGetGammaRampSize)
            _linearwin_dlsym(_linearwin.x11.vidmode.handle, "XF86VidModeGetGammaRampSize");

        _linearwin.x11.vidmode.available =
            XF86VidModeQueryExtension(_linearwin.x11.display,
                                      &_linearwin.x11.vidmode.eventBase,
                                      &_linearwin.x11.vidmode.errorBase);
    }

#if defined(__CYGWIN__)
    _linearwin.x11.xi.handle = _linearwin_dlopen("libXi-6.so");
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    _linearwin.x11.xi.handle = _linearwin_dlopen("libXi.so");
#else
    _linearwin.x11.xi.handle = _linearwin_dlopen("libXi.so.6");
#endif
    if (_linearwin.x11.xi.handle)
    {
        _linearwin.x11.xi.QueryVersion = (PFN_XIQueryVersion)
            _linearwin_dlsym(_linearwin.x11.xi.handle, "XIQueryVersion");
        _linearwin.x11.xi.SelectEvents = (PFN_XISelectEvents)
            _linearwin_dlsym(_linearwin.x11.xi.handle, "XISelectEvents");

        if (XQueryExtension(_linearwin.x11.display,
                            "XInputExtension",
                            &_linearwin.x11.xi.majorOpcode,
                            &_linearwin.x11.xi.eventBase,
                            &_linearwin.x11.xi.errorBase))
        {
            _linearwin.x11.xi.major = 2;
            _linearwin.x11.xi.minor = 0;

            if (XIQueryVersion(_linearwin.x11.display,
                               &_linearwin.x11.xi.major,
                               &_linearwin.x11.xi.minor) == Success)
            {
                _linearwin.x11.xi.available = LINEARWIN_TRUE;
            }
        }
    }

#if defined(__CYGWIN__)
    _linearwin.x11.randr.handle = _linearwin_dlopen("libXrandr-2.so");
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    _linearwin.x11.randr.handle = _linearwin_dlopen("libXrandr.so");
#else
    _linearwin.x11.randr.handle = _linearwin_dlopen("libXrandr.so.2");
#endif
    if (_linearwin.x11.randr.handle)
    {
        _linearwin.x11.randr.AllocGamma = (PFN_XRRAllocGamma)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRAllocGamma");
        _linearwin.x11.randr.FreeGamma = (PFN_XRRFreeGamma)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRFreeGamma");
        _linearwin.x11.randr.FreeCrtcInfo = (PFN_XRRFreeCrtcInfo)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRFreeCrtcInfo");
        _linearwin.x11.randr.FreeGamma = (PFN_XRRFreeGamma)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRFreeGamma");
        _linearwin.x11.randr.FreeOutputInfo = (PFN_XRRFreeOutputInfo)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRFreeOutputInfo");
        _linearwin.x11.randr.FreeScreenResources = (PFN_XRRFreeScreenResources)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRFreeScreenResources");
        _linearwin.x11.randr.GetCrtcGamma = (PFN_XRRGetCrtcGamma)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRGetCrtcGamma");
        _linearwin.x11.randr.GetCrtcGammaSize = (PFN_XRRGetCrtcGammaSize)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRGetCrtcGammaSize");
        _linearwin.x11.randr.GetCrtcInfo = (PFN_XRRGetCrtcInfo)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRGetCrtcInfo");
        _linearwin.x11.randr.GetOutputInfo = (PFN_XRRGetOutputInfo)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRGetOutputInfo");
        _linearwin.x11.randr.GetOutputPrimary = (PFN_XRRGetOutputPrimary)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRGetOutputPrimary");
        _linearwin.x11.randr.GetScreenResourcesCurrent = (PFN_XRRGetScreenResourcesCurrent)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRGetScreenResourcesCurrent");
        _linearwin.x11.randr.QueryExtension = (PFN_XRRQueryExtension)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRQueryExtension");
        _linearwin.x11.randr.QueryVersion = (PFN_XRRQueryVersion)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRQueryVersion");
        _linearwin.x11.randr.SelectInput = (PFN_XRRSelectInput)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRSelectInput");
        _linearwin.x11.randr.SetCrtcConfig = (PFN_XRRSetCrtcConfig)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRSetCrtcConfig");
        _linearwin.x11.randr.SetCrtcGamma = (PFN_XRRSetCrtcGamma)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRSetCrtcGamma");
        _linearwin.x11.randr.UpdateConfiguration = (PFN_XRRUpdateConfiguration)
            _linearwin_dlsym(_linearwin.x11.randr.handle, "XRRUpdateConfiguration");

        if (XRRQueryExtension(_linearwin.x11.display,
                              &_linearwin.x11.randr.eventBase,
                              &_linearwin.x11.randr.errorBase))
        {
            if (XRRQueryVersion(_linearwin.x11.display,
                                &_linearwin.x11.randr.major,
                                &_linearwin.x11.randr.minor))
            {
                // The LINEARWIN RandR path requires at least version 1.3
                if (_linearwin.x11.randr.major > 1 || _linearwin.x11.randr.minor >= 3)
                    _linearwin.x11.randr.available = LINEARWIN_TRUE;
            }
            else
            {
                _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                                "X11: Failed to query RandR version");
            }
        }
    }

    if (_linearwin.x11.randr.available)
    {
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_linearwin.x11.display,
                                                              _linearwin.x11.root);

        if (!sr->ncrtc || !XRRGetCrtcGammaSize(_linearwin.x11.display, sr->crtcs[0]))
        {
            // This is likely an older Nvidia driver with broken gamma support
            // Flag it as useless and fall back to xf86vm gamma, if available
            _linearwin.x11.randr.gammaBroken = LINEARWIN_TRUE;
        }

        if (!sr->ncrtc)
        {
            // A system without CRTCs is likely a system with broken RandR
            // Disable the RandR monitor path and fall back to core functions
            _linearwin.x11.randr.monitorBroken = LINEARWIN_TRUE;
        }

        XRRFreeScreenResources(sr);
    }

    if (_linearwin.x11.randr.available && !_linearwin.x11.randr.monitorBroken)
    {
        XRRSelectInput(_linearwin.x11.display, _linearwin.x11.root,
                       RROutputChangeNotifyMask);
    }

#if defined(__CYGWIN__)
    _linearwin.x11.xcursor.handle = _linearwin_dlopen("libXcursor-1.so");
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    _linearwin.x11.xcursor.handle = _linearwin_dlopen("libXcursor.so");
#else
    _linearwin.x11.xcursor.handle = _linearwin_dlopen("libXcursor.so.1");
#endif
    if (_linearwin.x11.xcursor.handle)
    {
        _linearwin.x11.xcursor.ImageCreate = (PFN_XcursorImageCreate)
            _linearwin_dlsym(_linearwin.x11.xcursor.handle, "XcursorImageCreate");
        _linearwin.x11.xcursor.ImageDestroy = (PFN_XcursorImageDestroy)
            _linearwin_dlsym(_linearwin.x11.xcursor.handle, "XcursorImageDestroy");
        _linearwin.x11.xcursor.ImageLoadCursor = (PFN_XcursorImageLoadCursor)
            _linearwin_dlsym(_linearwin.x11.xcursor.handle, "XcursorImageLoadCursor");
    }

#if defined(__CYGWIN__)
    _linearwin.x11.xinerama.handle = _linearwin_dlopen("libXinerama-1.so");
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    _linearwin.x11.xinerama.handle = _linearwin_dlopen("libXinerama.so");
#else
    _linearwin.x11.xinerama.handle = _linearwin_dlopen("libXinerama.so.1");
#endif
    if (_linearwin.x11.xinerama.handle)
    {
        _linearwin.x11.xinerama.IsActive = (PFN_XineramaIsActive)
            _linearwin_dlsym(_linearwin.x11.xinerama.handle, "XineramaIsActive");
        _linearwin.x11.xinerama.QueryExtension = (PFN_XineramaQueryExtension)
            _linearwin_dlsym(_linearwin.x11.xinerama.handle, "XineramaQueryExtension");
        _linearwin.x11.xinerama.QueryScreens = (PFN_XineramaQueryScreens)
            _linearwin_dlsym(_linearwin.x11.xinerama.handle, "XineramaQueryScreens");

        if (XineramaQueryExtension(_linearwin.x11.display,
                                   &_linearwin.x11.xinerama.major,
                                   &_linearwin.x11.xinerama.minor))
        {
            if (XineramaIsActive(_linearwin.x11.display))
                _linearwin.x11.xinerama.available = LINEARWIN_TRUE;
        }
    }

    _linearwin.x11.xkb.major = 1;
    _linearwin.x11.xkb.minor = 0;
    _linearwin.x11.xkb.available =
        XkbQueryExtension(_linearwin.x11.display,
                          &_linearwin.x11.xkb.majorOpcode,
                          &_linearwin.x11.xkb.eventBase,
                          &_linearwin.x11.xkb.errorBase,
                          &_linearwin.x11.xkb.major,
                          &_linearwin.x11.xkb.minor);

    if (_linearwin.x11.xkb.available)
    {
        Bool supported;

        if (XkbSetDetectableAutoRepeat(_linearwin.x11.display, True, &supported))
        {
            if (supported)
                _linearwin.x11.xkb.detectable = LINEARWIN_TRUE;
        }

        XkbStateRec state;
        if (XkbGetState(_linearwin.x11.display, XkbUseCoreKbd, &state) == Success)
            _linearwin.x11.xkb.group = (unsigned int)state.group;

        XkbSelectEventDetails(_linearwin.x11.display, XkbUseCoreKbd, XkbStateNotify,
                              XkbGroupStateMask, XkbGroupStateMask);
    }

#if defined(__CYGWIN__)
    _linearwin.x11.x11xcb.handle = _linearwin_dlopen("libX11-xcb-1.so");
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    _linearwin.x11.x11xcb.handle = _linearwin_dlopen("libX11-xcb.so");
#else
    _linearwin.x11.x11xcb.handle = _linearwin_dlopen("libX11-xcb.so.1");
#endif
    if (_linearwin.x11.x11xcb.handle)
    {
        _linearwin.x11.x11xcb.GetXCBConnection = (PFN_XGetXCBConnection)
            _linearwin_dlsym(_linearwin.x11.x11xcb.handle, "XGetXCBConnection");
    }

#if defined(__CYGWIN__)
    _linearwin.x11.xrender.handle = _linearwin_dlopen("libXrender-1.so");
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    _linearwin.x11.xrender.handle = _linearwin_dlopen("libXrender.so");
#else
    _linearwin.x11.xrender.handle = _linearwin_dlopen("libXrender.so.1");
#endif
    if (_linearwin.x11.xrender.handle)
    {
        _linearwin.x11.xrender.QueryExtension = (PFN_XRenderQueryExtension)
            _linearwin_dlsym(_linearwin.x11.xrender.handle, "XRenderQueryExtension");
        _linearwin.x11.xrender.QueryVersion = (PFN_XRenderQueryVersion)
            _linearwin_dlsym(_linearwin.x11.xrender.handle, "XRenderQueryVersion");
        _linearwin.x11.xrender.FindVisualFormat = (PFN_XRenderFindVisualFormat)
            _linearwin_dlsym(_linearwin.x11.xrender.handle, "XRenderFindVisualFormat");

        if (XRenderQueryExtension(_linearwin.x11.display,
                                  &_linearwin.x11.xrender.errorBase,
                                  &_linearwin.x11.xrender.eventBase))
        {
            if (XRenderQueryVersion(_linearwin.x11.display,
                                    &_linearwin.x11.xrender.major,
                                    &_linearwin.x11.xrender.minor))
            {
                _linearwin.x11.xrender.available = LINEARWIN_TRUE;
            }
        }
    }

    // Update the key code LUT
    // FIXME: We should listen to XkbMapNotify events to track changes to
    // the keyboard mapping.
    createKeyTables();

    // String format atoms
    _linearwin.x11.NULL_ = XInternAtom(_linearwin.x11.display, "NULL", False);
    _linearwin.x11.UTF8_STRING = XInternAtom(_linearwin.x11.display, "UTF8_STRING", False);
    _linearwin.x11.ATOM_PAIR = XInternAtom(_linearwin.x11.display, "ATOM_PAIR", False);

    // Custom selection property atom
    _linearwin.x11.LINEARWIN_SELECTION =
        XInternAtom(_linearwin.x11.display, "LINEARWIN_SELECTION", False);

    // ICCCM standard clipboard atoms
    _linearwin.x11.TARGETS = XInternAtom(_linearwin.x11.display, "TARGETS", False);
    _linearwin.x11.MULTIPLE = XInternAtom(_linearwin.x11.display, "MULTIPLE", False);
    _linearwin.x11.PRIMARY = XInternAtom(_linearwin.x11.display, "PRIMARY", False);
    _linearwin.x11.INCR = XInternAtom(_linearwin.x11.display, "INCR", False);
    _linearwin.x11.CLIPBOARD = XInternAtom(_linearwin.x11.display, "CLIPBOARD", False);

    // Clipboard manager atoms
    _linearwin.x11.CLIPBOARD_MANAGER =
        XInternAtom(_linearwin.x11.display, "CLIPBOARD_MANAGER", False);
    _linearwin.x11.SAVE_TARGETS =
        XInternAtom(_linearwin.x11.display, "SAVE_TARGETS", False);

    // Xdnd (drag and drop) atoms
    _linearwin.x11.XdndAware = XInternAtom(_linearwin.x11.display, "XdndAware", False);
    _linearwin.x11.XdndEnter = XInternAtom(_linearwin.x11.display, "XdndEnter", False);
    _linearwin.x11.XdndPosition = XInternAtom(_linearwin.x11.display, "XdndPosition", False);
    _linearwin.x11.XdndStatus = XInternAtom(_linearwin.x11.display, "XdndStatus", False);
    _linearwin.x11.XdndActionCopy = XInternAtom(_linearwin.x11.display, "XdndActionCopy", False);
    _linearwin.x11.XdndDrop = XInternAtom(_linearwin.x11.display, "XdndDrop", False);
    _linearwin.x11.XdndFinished = XInternAtom(_linearwin.x11.display, "XdndFinished", False);
    _linearwin.x11.XdndSelection = XInternAtom(_linearwin.x11.display, "XdndSelection", False);
    _linearwin.x11.XdndTypeList = XInternAtom(_linearwin.x11.display, "XdndTypeList", False);
    _linearwin.x11.text_uri_list = XInternAtom(_linearwin.x11.display, "text/uri-list", False);

    // ICCCM, EWMH and Motif window property atoms
    // These can be set safely even without WM support
    // The EWMH atoms that require WM support are handled in detectEWMH
    _linearwin.x11.WM_PROTOCOLS =
        XInternAtom(_linearwin.x11.display, "WM_PROTOCOLS", False);
    _linearwin.x11.WM_STATE =
        XInternAtom(_linearwin.x11.display, "WM_STATE", False);
    _linearwin.x11.WM_DELETE_WINDOW =
        XInternAtom(_linearwin.x11.display, "WM_DELETE_WINDOW", False);
    _linearwin.x11.NET_SUPPORTED =
        XInternAtom(_linearwin.x11.display, "_NET_SUPPORTED", False);
    _linearwin.x11.NET_SUPPORTING_WM_CHECK =
        XInternAtom(_linearwin.x11.display, "_NET_SUPPORTING_WM_CHECK", False);
    _linearwin.x11.NET_WM_ICON =
        XInternAtom(_linearwin.x11.display, "_NET_WM_ICON", False);
    _linearwin.x11.NET_WM_PING =
        XInternAtom(_linearwin.x11.display, "_NET_WM_PING", False);
    _linearwin.x11.NET_WM_PID =
        XInternAtom(_linearwin.x11.display, "_NET_WM_PID", False);
    _linearwin.x11.NET_WM_NAME =
        XInternAtom(_linearwin.x11.display, "_NET_WM_NAME", False);
    _linearwin.x11.NET_WM_ICON_NAME =
        XInternAtom(_linearwin.x11.display, "_NET_WM_ICON_NAME", False);
    _linearwin.x11.NET_WM_BYPASS_COMPOSITOR =
        XInternAtom(_linearwin.x11.display, "_NET_WM_BYPASS_COMPOSITOR", False);
    _linearwin.x11.NET_WM_WINDOW_OPACITY =
        XInternAtom(_linearwin.x11.display, "_NET_WM_WINDOW_OPACITY", False);
    _linearwin.x11.MOTIF_WM_HINTS =
        XInternAtom(_linearwin.x11.display, "_MOTIF_WM_HINTS", False);

    // The compositing manager selection name contains the screen number
    {
        char name[32];
        snprintf(name, sizeof(name), "_NET_WM_CM_S%u", _linearwin.x11.screen);
        _linearwin.x11.NET_WM_CM_Sx = XInternAtom(_linearwin.x11.display, name, False);
    }

    // Detect whether an EWMH-conformant window manager is running
    detectEWMH();

    return LINEARWIN_TRUE;
}

// Retrieve system content scale via folklore heuristics
//
static void getSystemContentScale(float* xscale, float* yscale)
{
    // Start by assuming the default X11 DPI
    // NOTE: Some desktop environments (KDE) may remove the Xft.dpi field when it
    //       would be set to 96, so assume that is the case if we cannot find it
    float xdpi = 96.f, ydpi = 96.f;

    // NOTE: Basing the scale on Xft.dpi where available should provide the most
    //       consistent user experience (matches Qt, Gtk, etc), although not
    //       always the most accurate one
    char* rms = XResourceManagerString(_linearwin.x11.display);
    if (rms)
    {
        XrmDatabase db = XrmGetStringDatabase(rms);
        if (db)
        {
            XrmValue value;
            char* type = NULL;

            if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value))
            {
                if (type && strcmp(type, "String") == 0)
                    xdpi = ydpi = atof(value.addr);
            }

            XrmDestroyDatabase(db);
        }
    }

    *xscale = xdpi / 96.f;
    *yscale = ydpi / 96.f;
}

// Create a blank cursor for hidden and disabled cursor modes
//
static Cursor createHiddenCursor(void)
{
    unsigned char pixels[16 * 16 * 4] = { 0 };
    LINEARWINimage image = { 16, 16, pixels };
    return _linearwinCreateCursorX11(&image, 0, 0);
}

// Create a helper window for IPC
//
static Window createHelperWindow(void)
{
    XSetWindowAttributes wa;
    wa.event_mask = PropertyChangeMask;

    return XCreateWindow(_linearwin.x11.display, _linearwin.x11.root,
                         0, 0, 1, 1, 0, 0,
                         InputOnly,
                         DefaultVisual(_linearwin.x11.display, _linearwin.x11.screen),
                         CWEventMask, &wa);
}

// Create the pipe for empty events without assumuing the OS has pipe2(2)
//
static LINEARWINbool createEmptyEventPipe(void)
{
    if (pipe(_linearwin.x11.emptyEventPipe) != 0)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "X11: Failed to create empty event pipe: %s",
                        strerror(errno));
        return LINEARWIN_FALSE;
    }

    for (int i = 0; i < 2; i++)
    {
        const int sf = fcntl(_linearwin.x11.emptyEventPipe[i], F_GETFL, 0);
        const int df = fcntl(_linearwin.x11.emptyEventPipe[i], F_GETFD, 0);

        if (sf == -1 || df == -1 ||
            fcntl(_linearwin.x11.emptyEventPipe[i], F_SETFL, sf | O_NONBLOCK) == -1 ||
            fcntl(_linearwin.x11.emptyEventPipe[i], F_SETFD, df | FD_CLOEXEC) == -1)
        {
            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "X11: Failed to set flags for empty event pipe: %s",
                            strerror(errno));
            return LINEARWIN_FALSE;
        }
    }

    return LINEARWIN_TRUE;
}

// X error handler
//
static int errorHandler(Display *display, XErrorEvent* event)
{
    if (_linearwin.x11.display != display)
        return 0;

    _linearwin.x11.errorCode = event->error_code;
    return 0;
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Sets the X error handler callback
//
void _linearwinGrabErrorHandlerX11(void)
{
    assert(_linearwin.x11.errorHandler == NULL);
    _linearwin.x11.errorCode = Success;
    _linearwin.x11.errorHandler = XSetErrorHandler(errorHandler);
}

// Clears the X error handler callback
//
void _linearwinReleaseErrorHandlerX11(void)
{
    // Synchronize to make sure all commands are processed
    XSync(_linearwin.x11.display, False);
    XSetErrorHandler(_linearwin.x11.errorHandler);
    _linearwin.x11.errorHandler = NULL;
}

// Reports the specified error, appending information about the last X error
//
void _linearwinInputErrorX11(int error, const char* message)
{
    char buffer[_LINEARWIN_MESSAGE_SIZE];
    XGetErrorText(_linearwin.x11.display, _linearwin.x11.errorCode,
                  buffer, sizeof(buffer));

    _linearwinInputError(error, "%s: %s", message, buffer);
}

// Creates a native cursor object from the specified image and hotspot
//
Cursor _linearwinCreateCursorX11(const LINEARWINimage* image, int xhot, int yhot)
{
    int i;
    Cursor cursor;

    if (!_linearwin.x11.xcursor.handle)
        return None;

    XcursorImage* native = XcursorImageCreate(image->width, image->height);
    if (native == NULL)
        return None;

    native->xhot = xhot;
    native->yhot = yhot;

    unsigned char* source = (unsigned char*) image->pixels;
    XcursorPixel* target = native->pixels;

    for (i = 0;  i < image->width * image->height;  i++, target++, source += 4)
    {
        unsigned int alpha = source[3];

        *target = (alpha << 24) |
                  ((unsigned char) ((source[0] * alpha) / 255) << 16) |
                  ((unsigned char) ((source[1] * alpha) / 255) <<  8) |
                  ((unsigned char) ((source[2] * alpha) / 255) <<  0);
    }

    cursor = XcursorImageLoadCursor(_linearwin.x11.display, native);
    XcursorImageDestroy(native);

    return cursor;
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _linearwinPlatformInit(void)
{
    // HACK: If the application has left the locale as "C" then both wide
    //       character text input and explicit UTF-8 input via XIM will break
    //       This sets the CTYPE part of the current locale from the environment
    //       in the hope that it is set to something more sane than "C"
    if (strcmp(setlocale(LC_CTYPE, NULL), "C") == 0)
        setlocale(LC_CTYPE, "");

    XInitThreads();
    XrmInitialize();

    _linearwin.x11.display = XOpenDisplay(NULL);
    if (!_linearwin.x11.display)
    {
        const char* display = getenv("DISPLAY");
        if (display)
        {
            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "X11: Failed to open display %s", display);
        }
        else
        {
            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "X11: The DISPLAY environment variable is missing");
        }

        return LINEARWIN_FALSE;
    }

    _linearwin.x11.screen = DefaultScreen(_linearwin.x11.display);
    _linearwin.x11.root = RootWindow(_linearwin.x11.display, _linearwin.x11.screen);
    _linearwin.x11.context = XUniqueContext();

    getSystemContentScale(&_linearwin.x11.contentScaleX, &_linearwin.x11.contentScaleY);

    if (!createEmptyEventPipe())
        return LINEARWIN_FALSE;

    if (!initExtensions())
        return LINEARWIN_FALSE;

    _linearwin.x11.helperWindowHandle = createHelperWindow();
    _linearwin.x11.hiddenCursorHandle = createHiddenCursor();

    if (XSupportsLocale())
    {
        XSetLocaleModifiers("");

        _linearwin.x11.im = XOpenIM(_linearwin.x11.display, 0, NULL, NULL);
        if (_linearwin.x11.im)
        {
            if (!hasUsableInputMethodStyle())
            {
                XCloseIM(_linearwin.x11.im);
                _linearwin.x11.im = NULL;
            }
        }
    }

#if defined(__linux__)
    if (!_linearwinInitJoysticksLinux())
        return LINEARWIN_FALSE;
#endif

    _linearwinInitTimerPOSIX();

    _linearwinPollMonitorsX11();
    return LINEARWIN_TRUE;
}

void _linearwinPlatformTerminate(void)
{
    if (_linearwin.x11.helperWindowHandle)
    {
        if (XGetSelectionOwner(_linearwin.x11.display, _linearwin.x11.CLIPBOARD) ==
            _linearwin.x11.helperWindowHandle)
        {
            _linearwinPushSelectionToManagerX11();
        }

        XDestroyWindow(_linearwin.x11.display, _linearwin.x11.helperWindowHandle);
        _linearwin.x11.helperWindowHandle = None;
    }

    if (_linearwin.x11.hiddenCursorHandle)
    {
        XFreeCursor(_linearwin.x11.display, _linearwin.x11.hiddenCursorHandle);
        _linearwin.x11.hiddenCursorHandle = (Cursor) 0;
    }

    free(_linearwin.x11.primarySelectionString);
    free(_linearwin.x11.clipboardString);

    if (_linearwin.x11.im)
    {
        XCloseIM(_linearwin.x11.im);
        _linearwin.x11.im = NULL;
    }

    if (_linearwin.x11.display)
    {
        XCloseDisplay(_linearwin.x11.display);
        _linearwin.x11.display = NULL;
    }

    if (_linearwin.x11.x11xcb.handle)
    {
        _linearwin_dlclose(_linearwin.x11.x11xcb.handle);
        _linearwin.x11.x11xcb.handle = NULL;
    }

    if (_linearwin.x11.xcursor.handle)
    {
        _linearwin_dlclose(_linearwin.x11.xcursor.handle);
        _linearwin.x11.xcursor.handle = NULL;
    }

    if (_linearwin.x11.randr.handle)
    {
        _linearwin_dlclose(_linearwin.x11.randr.handle);
        _linearwin.x11.randr.handle = NULL;
    }

    if (_linearwin.x11.xinerama.handle)
    {
        _linearwin_dlclose(_linearwin.x11.xinerama.handle);
        _linearwin.x11.xinerama.handle = NULL;
    }

    if (_linearwin.x11.xrender.handle)
    {
        _linearwin_dlclose(_linearwin.x11.xrender.handle);
        _linearwin.x11.xrender.handle = NULL;
    }

    if (_linearwin.x11.vidmode.handle)
    {
        _linearwin_dlclose(_linearwin.x11.vidmode.handle);
        _linearwin.x11.vidmode.handle = NULL;
    }

    if (_linearwin.x11.xi.handle)
    {
        _linearwin_dlclose(_linearwin.x11.xi.handle);
        _linearwin.x11.xi.handle = NULL;
    }

    _linearwinTerminateOSMesa();
    // NOTE: These need to be unloaded after XCloseDisplay, as they register
    //       cleanup callbacks that get called by that function
    _linearwinTerminateEGL();
    _linearwinTerminateGLX();

#if defined(__linux__)
    _linearwinTerminateJoysticksLinux();
#endif

    if (_linearwin.x11.emptyEventPipe[0] || _linearwin.x11.emptyEventPipe[1])
    {
        close(_linearwin.x11.emptyEventPipe[0]);
        close(_linearwin.x11.emptyEventPipe[1]);
    }
}

const char* _linearwinPlatformGetVersionString(void)
{
    return _LINEARWIN_VERSION_NUMBER " X11 GLX EGL OSMesa"
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
        " clock_gettime"
#else
        " gettimeofday"
#endif
#if defined(__linux__)
        " evdev"
#endif
#if defined(_LINEARWIN_BUILD_DLL)
        " shared"
#endif
        ;
}

