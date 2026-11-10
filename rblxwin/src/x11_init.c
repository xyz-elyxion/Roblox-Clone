//========================================================================
// RBLXWIN 3.3 X11 - www.rblxwin.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@rblxwin.org>
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


// Translate the X11 KeySyms for a key to a RBLXWIN key code
// NOTE: This is only used as a fallback, in case the XKB method fails
//       It is layout-dependent and will fail partially on most non-US layouts
//
static int translateKeySyms(const KeySym* keysyms, int width)
{
    if (width > 1)
    {
        switch (keysyms[1])
        {
            case XK_KP_0:           return RBLXWIN_KEY_KP_0;
            case XK_KP_1:           return RBLXWIN_KEY_KP_1;
            case XK_KP_2:           return RBLXWIN_KEY_KP_2;
            case XK_KP_3:           return RBLXWIN_KEY_KP_3;
            case XK_KP_4:           return RBLXWIN_KEY_KP_4;
            case XK_KP_5:           return RBLXWIN_KEY_KP_5;
            case XK_KP_6:           return RBLXWIN_KEY_KP_6;
            case XK_KP_7:           return RBLXWIN_KEY_KP_7;
            case XK_KP_8:           return RBLXWIN_KEY_KP_8;
            case XK_KP_9:           return RBLXWIN_KEY_KP_9;
            case XK_KP_Separator:
            case XK_KP_Decimal:     return RBLXWIN_KEY_KP_DECIMAL;
            case XK_KP_Equal:       return RBLXWIN_KEY_KP_EQUAL;
            case XK_KP_Enter:       return RBLXWIN_KEY_KP_ENTER;
            default:                break;
        }
    }

    switch (keysyms[0])
    {
        case XK_Escape:         return RBLXWIN_KEY_ESCAPE;
        case XK_Tab:            return RBLXWIN_KEY_TAB;
        case XK_Shift_L:        return RBLXWIN_KEY_LEFT_SHIFT;
        case XK_Shift_R:        return RBLXWIN_KEY_RIGHT_SHIFT;
        case XK_Control_L:      return RBLXWIN_KEY_LEFT_CONTROL;
        case XK_Control_R:      return RBLXWIN_KEY_RIGHT_CONTROL;
        case XK_Meta_L:
        case XK_Alt_L:          return RBLXWIN_KEY_LEFT_ALT;
        case XK_Mode_switch: // Mapped to Alt_R on many keyboards
        case XK_ISO_Level3_Shift: // AltGr on at least some machines
        case XK_Meta_R:
        case XK_Alt_R:          return RBLXWIN_KEY_RIGHT_ALT;
        case XK_Super_L:        return RBLXWIN_KEY_LEFT_SUPER;
        case XK_Super_R:        return RBLXWIN_KEY_RIGHT_SUPER;
        case XK_Menu:           return RBLXWIN_KEY_MENU;
        case XK_Num_Lock:       return RBLXWIN_KEY_NUM_LOCK;
        case XK_Caps_Lock:      return RBLXWIN_KEY_CAPS_LOCK;
        case XK_Print:          return RBLXWIN_KEY_PRINT_SCREEN;
        case XK_Scroll_Lock:    return RBLXWIN_KEY_SCROLL_LOCK;
        case XK_Pause:          return RBLXWIN_KEY_PAUSE;
        case XK_Delete:         return RBLXWIN_KEY_DELETE;
        case XK_BackSpace:      return RBLXWIN_KEY_BACKSPACE;
        case XK_Return:         return RBLXWIN_KEY_ENTER;
        case XK_Home:           return RBLXWIN_KEY_HOME;
        case XK_End:            return RBLXWIN_KEY_END;
        case XK_Page_Up:        return RBLXWIN_KEY_PAGE_UP;
        case XK_Page_Down:      return RBLXWIN_KEY_PAGE_DOWN;
        case XK_Insert:         return RBLXWIN_KEY_INSERT;
        case XK_Left:           return RBLXWIN_KEY_LEFT;
        case XK_Right:          return RBLXWIN_KEY_RIGHT;
        case XK_Down:           return RBLXWIN_KEY_DOWN;
        case XK_Up:             return RBLXWIN_KEY_UP;
        case XK_F1:             return RBLXWIN_KEY_F1;
        case XK_F2:             return RBLXWIN_KEY_F2;
        case XK_F3:             return RBLXWIN_KEY_F3;
        case XK_F4:             return RBLXWIN_KEY_F4;
        case XK_F5:             return RBLXWIN_KEY_F5;
        case XK_F6:             return RBLXWIN_KEY_F6;
        case XK_F7:             return RBLXWIN_KEY_F7;
        case XK_F8:             return RBLXWIN_KEY_F8;
        case XK_F9:             return RBLXWIN_KEY_F9;
        case XK_F10:            return RBLXWIN_KEY_F10;
        case XK_F11:            return RBLXWIN_KEY_F11;
        case XK_F12:            return RBLXWIN_KEY_F12;
        case XK_F13:            return RBLXWIN_KEY_F13;
        case XK_F14:            return RBLXWIN_KEY_F14;
        case XK_F15:            return RBLXWIN_KEY_F15;
        case XK_F16:            return RBLXWIN_KEY_F16;
        case XK_F17:            return RBLXWIN_KEY_F17;
        case XK_F18:            return RBLXWIN_KEY_F18;
        case XK_F19:            return RBLXWIN_KEY_F19;
        case XK_F20:            return RBLXWIN_KEY_F20;
        case XK_F21:            return RBLXWIN_KEY_F21;
        case XK_F22:            return RBLXWIN_KEY_F22;
        case XK_F23:            return RBLXWIN_KEY_F23;
        case XK_F24:            return RBLXWIN_KEY_F24;
        case XK_F25:            return RBLXWIN_KEY_F25;

        // Numeric keypad
        case XK_KP_Divide:      return RBLXWIN_KEY_KP_DIVIDE;
        case XK_KP_Multiply:    return RBLXWIN_KEY_KP_MULTIPLY;
        case XK_KP_Subtract:    return RBLXWIN_KEY_KP_SUBTRACT;
        case XK_KP_Add:         return RBLXWIN_KEY_KP_ADD;

        // These should have been detected in secondary keysym test above!
        case XK_KP_Insert:      return RBLXWIN_KEY_KP_0;
        case XK_KP_End:         return RBLXWIN_KEY_KP_1;
        case XK_KP_Down:        return RBLXWIN_KEY_KP_2;
        case XK_KP_Page_Down:   return RBLXWIN_KEY_KP_3;
        case XK_KP_Left:        return RBLXWIN_KEY_KP_4;
        case XK_KP_Right:       return RBLXWIN_KEY_KP_6;
        case XK_KP_Home:        return RBLXWIN_KEY_KP_7;
        case XK_KP_Up:          return RBLXWIN_KEY_KP_8;
        case XK_KP_Page_Up:     return RBLXWIN_KEY_KP_9;
        case XK_KP_Delete:      return RBLXWIN_KEY_KP_DECIMAL;
        case XK_KP_Equal:       return RBLXWIN_KEY_KP_EQUAL;
        case XK_KP_Enter:       return RBLXWIN_KEY_KP_ENTER;

        // Last resort: Check for printable keys (should not happen if the XKB
        // extension is available). This will give a layout dependent mapping
        // (which is wrong, and we may miss some keys, especially on non-US
        // keyboards), but it's better than nothing...
        case XK_a:              return RBLXWIN_KEY_A;
        case XK_b:              return RBLXWIN_KEY_B;
        case XK_c:              return RBLXWIN_KEY_C;
        case XK_d:              return RBLXWIN_KEY_D;
        case XK_e:              return RBLXWIN_KEY_E;
        case XK_f:              return RBLXWIN_KEY_F;
        case XK_g:              return RBLXWIN_KEY_G;
        case XK_h:              return RBLXWIN_KEY_H;
        case XK_i:              return RBLXWIN_KEY_I;
        case XK_j:              return RBLXWIN_KEY_J;
        case XK_k:              return RBLXWIN_KEY_K;
        case XK_l:              return RBLXWIN_KEY_L;
        case XK_m:              return RBLXWIN_KEY_M;
        case XK_n:              return RBLXWIN_KEY_N;
        case XK_o:              return RBLXWIN_KEY_O;
        case XK_p:              return RBLXWIN_KEY_P;
        case XK_q:              return RBLXWIN_KEY_Q;
        case XK_r:              return RBLXWIN_KEY_R;
        case XK_s:              return RBLXWIN_KEY_S;
        case XK_t:              return RBLXWIN_KEY_T;
        case XK_u:              return RBLXWIN_KEY_U;
        case XK_v:              return RBLXWIN_KEY_V;
        case XK_w:              return RBLXWIN_KEY_W;
        case XK_x:              return RBLXWIN_KEY_X;
        case XK_y:              return RBLXWIN_KEY_Y;
        case XK_z:              return RBLXWIN_KEY_Z;
        case XK_1:              return RBLXWIN_KEY_1;
        case XK_2:              return RBLXWIN_KEY_2;
        case XK_3:              return RBLXWIN_KEY_3;
        case XK_4:              return RBLXWIN_KEY_4;
        case XK_5:              return RBLXWIN_KEY_5;
        case XK_6:              return RBLXWIN_KEY_6;
        case XK_7:              return RBLXWIN_KEY_7;
        case XK_8:              return RBLXWIN_KEY_8;
        case XK_9:              return RBLXWIN_KEY_9;
        case XK_0:              return RBLXWIN_KEY_0;
        case XK_space:          return RBLXWIN_KEY_SPACE;
        case XK_minus:          return RBLXWIN_KEY_MINUS;
        case XK_equal:          return RBLXWIN_KEY_EQUAL;
        case XK_bracketleft:    return RBLXWIN_KEY_LEFT_BRACKET;
        case XK_bracketright:   return RBLXWIN_KEY_RIGHT_BRACKET;
        case XK_backslash:      return RBLXWIN_KEY_BACKSLASH;
        case XK_semicolon:      return RBLXWIN_KEY_SEMICOLON;
        case XK_apostrophe:     return RBLXWIN_KEY_APOSTROPHE;
        case XK_grave:          return RBLXWIN_KEY_GRAVE_ACCENT;
        case XK_comma:          return RBLXWIN_KEY_COMMA;
        case XK_period:         return RBLXWIN_KEY_PERIOD;
        case XK_slash:          return RBLXWIN_KEY_SLASH;
        case XK_less:           return RBLXWIN_KEY_WORLD_1; // At least in some layouts...
        default:                break;
    }

    // No matching translation was found
    return RBLXWIN_KEY_UNKNOWN;
}

// Create key code translation tables
//
static void createKeyTables(void)
{
    int scancode, scancodeMin, scancodeMax;

    memset(_rblxwin.x11.keycodes, -1, sizeof(_rblxwin.x11.keycodes));
    memset(_rblxwin.x11.scancodes, -1, sizeof(_rblxwin.x11.scancodes));

    if (_rblxwin.x11.xkb.available)
    {
        // Use XKB to determine physical key locations independently of the
        // current keyboard layout

        XkbDescPtr desc = XkbGetMap(_rblxwin.x11.display, 0, XkbUseCoreKbd);
        XkbGetNames(_rblxwin.x11.display, XkbKeyNamesMask | XkbKeyAliasesMask, desc);

        scancodeMin = desc->min_key_code;
        scancodeMax = desc->max_key_code;

        const struct
        {
            int key;
            char* name;
        } keymap[] =
        {
            { RBLXWIN_KEY_GRAVE_ACCENT, "TLDE" },
            { RBLXWIN_KEY_1, "AE01" },
            { RBLXWIN_KEY_2, "AE02" },
            { RBLXWIN_KEY_3, "AE03" },
            { RBLXWIN_KEY_4, "AE04" },
            { RBLXWIN_KEY_5, "AE05" },
            { RBLXWIN_KEY_6, "AE06" },
            { RBLXWIN_KEY_7, "AE07" },
            { RBLXWIN_KEY_8, "AE08" },
            { RBLXWIN_KEY_9, "AE09" },
            { RBLXWIN_KEY_0, "AE10" },
            { RBLXWIN_KEY_MINUS, "AE11" },
            { RBLXWIN_KEY_EQUAL, "AE12" },
            { RBLXWIN_KEY_Q, "AD01" },
            { RBLXWIN_KEY_W, "AD02" },
            { RBLXWIN_KEY_E, "AD03" },
            { RBLXWIN_KEY_R, "AD04" },
            { RBLXWIN_KEY_T, "AD05" },
            { RBLXWIN_KEY_Y, "AD06" },
            { RBLXWIN_KEY_U, "AD07" },
            { RBLXWIN_KEY_I, "AD08" },
            { RBLXWIN_KEY_O, "AD09" },
            { RBLXWIN_KEY_P, "AD10" },
            { RBLXWIN_KEY_LEFT_BRACKET, "AD11" },
            { RBLXWIN_KEY_RIGHT_BRACKET, "AD12" },
            { RBLXWIN_KEY_A, "AC01" },
            { RBLXWIN_KEY_S, "AC02" },
            { RBLXWIN_KEY_D, "AC03" },
            { RBLXWIN_KEY_F, "AC04" },
            { RBLXWIN_KEY_G, "AC05" },
            { RBLXWIN_KEY_H, "AC06" },
            { RBLXWIN_KEY_J, "AC07" },
            { RBLXWIN_KEY_K, "AC08" },
            { RBLXWIN_KEY_L, "AC09" },
            { RBLXWIN_KEY_SEMICOLON, "AC10" },
            { RBLXWIN_KEY_APOSTROPHE, "AC11" },
            { RBLXWIN_KEY_Z, "AB01" },
            { RBLXWIN_KEY_X, "AB02" },
            { RBLXWIN_KEY_C, "AB03" },
            { RBLXWIN_KEY_V, "AB04" },
            { RBLXWIN_KEY_B, "AB05" },
            { RBLXWIN_KEY_N, "AB06" },
            { RBLXWIN_KEY_M, "AB07" },
            { RBLXWIN_KEY_COMMA, "AB08" },
            { RBLXWIN_KEY_PERIOD, "AB09" },
            { RBLXWIN_KEY_SLASH, "AB10" },
            { RBLXWIN_KEY_BACKSLASH, "BKSL" },
            { RBLXWIN_KEY_WORLD_1, "LSGT" },
            { RBLXWIN_KEY_SPACE, "SPCE" },
            { RBLXWIN_KEY_ESCAPE, "ESC" },
            { RBLXWIN_KEY_ENTER, "RTRN" },
            { RBLXWIN_KEY_TAB, "TAB" },
            { RBLXWIN_KEY_BACKSPACE, "BKSP" },
            { RBLXWIN_KEY_INSERT, "INS" },
            { RBLXWIN_KEY_DELETE, "DELE" },
            { RBLXWIN_KEY_RIGHT, "RGHT" },
            { RBLXWIN_KEY_LEFT, "LEFT" },
            { RBLXWIN_KEY_DOWN, "DOWN" },
            { RBLXWIN_KEY_UP, "UP" },
            { RBLXWIN_KEY_PAGE_UP, "PGUP" },
            { RBLXWIN_KEY_PAGE_DOWN, "PGDN" },
            { RBLXWIN_KEY_HOME, "HOME" },
            { RBLXWIN_KEY_END, "END" },
            { RBLXWIN_KEY_CAPS_LOCK, "CAPS" },
            { RBLXWIN_KEY_SCROLL_LOCK, "SCLK" },
            { RBLXWIN_KEY_NUM_LOCK, "NMLK" },
            { RBLXWIN_KEY_PRINT_SCREEN, "PRSC" },
            { RBLXWIN_KEY_PAUSE, "PAUS" },
            { RBLXWIN_KEY_F1, "FK01" },
            { RBLXWIN_KEY_F2, "FK02" },
            { RBLXWIN_KEY_F3, "FK03" },
            { RBLXWIN_KEY_F4, "FK04" },
            { RBLXWIN_KEY_F5, "FK05" },
            { RBLXWIN_KEY_F6, "FK06" },
            { RBLXWIN_KEY_F7, "FK07" },
            { RBLXWIN_KEY_F8, "FK08" },
            { RBLXWIN_KEY_F9, "FK09" },
            { RBLXWIN_KEY_F10, "FK10" },
            { RBLXWIN_KEY_F11, "FK11" },
            { RBLXWIN_KEY_F12, "FK12" },
            { RBLXWIN_KEY_F13, "FK13" },
            { RBLXWIN_KEY_F14, "FK14" },
            { RBLXWIN_KEY_F15, "FK15" },
            { RBLXWIN_KEY_F16, "FK16" },
            { RBLXWIN_KEY_F17, "FK17" },
            { RBLXWIN_KEY_F18, "FK18" },
            { RBLXWIN_KEY_F19, "FK19" },
            { RBLXWIN_KEY_F20, "FK20" },
            { RBLXWIN_KEY_F21, "FK21" },
            { RBLXWIN_KEY_F22, "FK22" },
            { RBLXWIN_KEY_F23, "FK23" },
            { RBLXWIN_KEY_F24, "FK24" },
            { RBLXWIN_KEY_F25, "FK25" },
            { RBLXWIN_KEY_KP_0, "KP0" },
            { RBLXWIN_KEY_KP_1, "KP1" },
            { RBLXWIN_KEY_KP_2, "KP2" },
            { RBLXWIN_KEY_KP_3, "KP3" },
            { RBLXWIN_KEY_KP_4, "KP4" },
            { RBLXWIN_KEY_KP_5, "KP5" },
            { RBLXWIN_KEY_KP_6, "KP6" },
            { RBLXWIN_KEY_KP_7, "KP7" },
            { RBLXWIN_KEY_KP_8, "KP8" },
            { RBLXWIN_KEY_KP_9, "KP9" },
            { RBLXWIN_KEY_KP_DECIMAL, "KPDL" },
            { RBLXWIN_KEY_KP_DIVIDE, "KPDV" },
            { RBLXWIN_KEY_KP_MULTIPLY, "KPMU" },
            { RBLXWIN_KEY_KP_SUBTRACT, "KPSU" },
            { RBLXWIN_KEY_KP_ADD, "KPAD" },
            { RBLXWIN_KEY_KP_ENTER, "KPEN" },
            { RBLXWIN_KEY_KP_EQUAL, "KPEQ" },
            { RBLXWIN_KEY_LEFT_SHIFT, "LFSH" },
            { RBLXWIN_KEY_LEFT_CONTROL, "LCTL" },
            { RBLXWIN_KEY_LEFT_ALT, "LALT" },
            { RBLXWIN_KEY_LEFT_SUPER, "LWIN" },
            { RBLXWIN_KEY_RIGHT_SHIFT, "RTSH" },
            { RBLXWIN_KEY_RIGHT_CONTROL, "RCTL" },
            { RBLXWIN_KEY_RIGHT_ALT, "RALT" },
            { RBLXWIN_KEY_RIGHT_ALT, "LVL3" },
            { RBLXWIN_KEY_RIGHT_ALT, "MDSW" },
            { RBLXWIN_KEY_RIGHT_SUPER, "RWIN" },
            { RBLXWIN_KEY_MENU, "MENU" }
        };

        // Find the X11 key code -> RBLXWIN key code mapping
        for (scancode = scancodeMin;  scancode <= scancodeMax;  scancode++)
        {
            int key = RBLXWIN_KEY_UNKNOWN;

            // Map the key name to a RBLXWIN key code. Note: We use the US
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
                if (key != RBLXWIN_KEY_UNKNOWN)
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

            _rblxwin.x11.keycodes[scancode] = key;
        }

        XkbFreeNames(desc, XkbKeyNamesMask, True);
        XkbFreeKeyboard(desc, 0, True);
    }
    else
        XDisplayKeycodes(_rblxwin.x11.display, &scancodeMin, &scancodeMax);

    int width;
    KeySym* keysyms = XGetKeyboardMapping(_rblxwin.x11.display,
                                          scancodeMin,
                                          scancodeMax - scancodeMin + 1,
                                          &width);

    for (scancode = scancodeMin;  scancode <= scancodeMax;  scancode++)
    {
        // Translate the un-translated key codes using traditional X11 KeySym
        // lookups
        if (_rblxwin.x11.keycodes[scancode] < 0)
        {
            const size_t base = (scancode - scancodeMin) * width;
            _rblxwin.x11.keycodes[scancode] = translateKeySyms(&keysyms[base], width);
        }

        // Store the reverse translation for faster key name lookup
        if (_rblxwin.x11.keycodes[scancode] > 0)
            _rblxwin.x11.scancodes[_rblxwin.x11.keycodes[scancode]] = scancode;
    }

    XFree(keysyms);
}

// Check whether the IM has a usable style
//
static RBLXWINbool hasUsableInputMethodStyle(void)
{
    RBLXWINbool found = RBLXWIN_FALSE;
    XIMStyles* styles = NULL;

    if (XGetIMValues(_rblxwin.x11.im, XNQueryInputStyle, &styles, NULL) != NULL)
        return RBLXWIN_FALSE;

    for (unsigned int i = 0;  i < styles->count_styles;  i++)
    {
        if (styles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing))
        {
            found = RBLXWIN_TRUE;
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
    const Atom atom = XInternAtom(_rblxwin.x11.display, atomName, False);

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
    if (!_rblxwinGetWindowPropertyX11(_rblxwin.x11.root,
                                   _rblxwin.x11.NET_SUPPORTING_WM_CHECK,
                                   XA_WINDOW,
                                   (unsigned char**) &windowFromRoot))
    {
        return;
    }

    _rblxwinGrabErrorHandlerX11();

    // If it exists, it should be the XID of a top-level window
    // Then we look for the same property on that window

    Window* windowFromChild = NULL;
    if (!_rblxwinGetWindowPropertyX11(*windowFromRoot,
                                   _rblxwin.x11.NET_SUPPORTING_WM_CHECK,
                                   XA_WINDOW,
                                   (unsigned char**) &windowFromChild))
    {
        XFree(windowFromRoot);
        return;
    }

    _rblxwinReleaseErrorHandlerX11();

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
        _rblxwinGetWindowPropertyX11(_rblxwin.x11.root,
                                  _rblxwin.x11.NET_SUPPORTED,
                                  XA_ATOM,
                                  (unsigned char**) &supportedAtoms);

    // See which of the atoms we support that are supported by the WM

    _rblxwin.x11.NET_WM_STATE =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE");
    _rblxwin.x11.NET_WM_STATE_ABOVE =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_ABOVE");
    _rblxwin.x11.NET_WM_STATE_FULLSCREEN =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_FULLSCREEN");
    _rblxwin.x11.NET_WM_STATE_MAXIMIZED_VERT =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_MAXIMIZED_VERT");
    _rblxwin.x11.NET_WM_STATE_MAXIMIZED_HORZ =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_MAXIMIZED_HORZ");
    _rblxwin.x11.NET_WM_STATE_DEMANDS_ATTENTION =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_DEMANDS_ATTENTION");
    _rblxwin.x11.NET_WM_FULLSCREEN_MONITORS =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_FULLSCREEN_MONITORS");
    _rblxwin.x11.NET_WM_WINDOW_TYPE =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_WINDOW_TYPE");
    _rblxwin.x11.NET_WM_WINDOW_TYPE_NORMAL =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_WINDOW_TYPE_NORMAL");
    _rblxwin.x11.NET_WORKAREA =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WORKAREA");
    _rblxwin.x11.NET_CURRENT_DESKTOP =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_CURRENT_DESKTOP");
    _rblxwin.x11.NET_ACTIVE_WINDOW =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_ACTIVE_WINDOW");
    _rblxwin.x11.NET_FRAME_EXTENTS =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_FRAME_EXTENTS");
    _rblxwin.x11.NET_REQUEST_FRAME_EXTENTS =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_REQUEST_FRAME_EXTENTS");

    if (supportedAtoms)
        XFree(supportedAtoms);
}

// Look for and initialize supported X11 extensions
//
static RBLXWINbool initExtensions(void)
{
#if defined(__OpenBSD__) || defined(__NetBSD__)
    _rblxwin.x11.vidmode.handle = _rblxwin_dlopen("libXxf86vm.so");
#else
    _rblxwin.x11.vidmode.handle = _rblxwin_dlopen("libXxf86vm.so.1");
#endif
    if (_rblxwin.x11.vidmode.handle)
    {
        _rblxwin.x11.vidmode.QueryExtension = (PFN_XF86VidModeQueryExtension)
            _rblxwin_dlsym(_rblxwin.x11.vidmode.handle, "XF86VidModeQueryExtension");
        _rblxwin.x11.vidmode.GetGammaRamp = (PFN_XF86VidModeGetGammaRamp)
            _rblxwin_dlsym(_rblxwin.x11.vidmode.handle, "XF86VidModeGetGammaRamp");
        _rblxwin.x11.vidmode.SetGammaRamp = (PFN_XF86VidModeSetGammaRamp)
            _rblxwin_dlsym(_rblxwin.x11.vidmode.handle, "XF86VidModeSetGammaRamp");
        _rblxwin.x11.vidmode.GetGammaRampSize = (PFN_XF86VidModeGetGammaRampSize)
            _rblxwin_dlsym(_rblxwin.x11.vidmode.handle, "XF86VidModeGetGammaRampSize");

        _rblxwin.x11.vidmode.available =
            XF86VidModeQueryExtension(_rblxwin.x11.display,
                                      &_rblxwin.x11.vidmode.eventBase,
                                      &_rblxwin.x11.vidmode.errorBase);
    }

#if defined(__CYGWIN__)
    _rblxwin.x11.xi.handle = _rblxwin_dlopen("libXi-6.so");
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    _rblxwin.x11.xi.handle = _rblxwin_dlopen("libXi.so");
#else
    _rblxwin.x11.xi.handle = _rblxwin_dlopen("libXi.so.6");
#endif
    if (_rblxwin.x11.xi.handle)
    {
        _rblxwin.x11.xi.QueryVersion = (PFN_XIQueryVersion)
            _rblxwin_dlsym(_rblxwin.x11.xi.handle, "XIQueryVersion");
        _rblxwin.x11.xi.SelectEvents = (PFN_XISelectEvents)
            _rblxwin_dlsym(_rblxwin.x11.xi.handle, "XISelectEvents");

        if (XQueryExtension(_rblxwin.x11.display,
                            "XInputExtension",
                            &_rblxwin.x11.xi.majorOpcode,
                            &_rblxwin.x11.xi.eventBase,
                            &_rblxwin.x11.xi.errorBase))
        {
            _rblxwin.x11.xi.major = 2;
            _rblxwin.x11.xi.minor = 0;

            if (XIQueryVersion(_rblxwin.x11.display,
                               &_rblxwin.x11.xi.major,
                               &_rblxwin.x11.xi.minor) == Success)
            {
                _rblxwin.x11.xi.available = RBLXWIN_TRUE;
            }
        }
    }

#if defined(__CYGWIN__)
    _rblxwin.x11.randr.handle = _rblxwin_dlopen("libXrandr-2.so");
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    _rblxwin.x11.randr.handle = _rblxwin_dlopen("libXrandr.so");
#else
    _rblxwin.x11.randr.handle = _rblxwin_dlopen("libXrandr.so.2");
#endif
    if (_rblxwin.x11.randr.handle)
    {
        _rblxwin.x11.randr.AllocGamma = (PFN_XRRAllocGamma)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRAllocGamma");
        _rblxwin.x11.randr.FreeGamma = (PFN_XRRFreeGamma)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRFreeGamma");
        _rblxwin.x11.randr.FreeCrtcInfo = (PFN_XRRFreeCrtcInfo)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRFreeCrtcInfo");
        _rblxwin.x11.randr.FreeGamma = (PFN_XRRFreeGamma)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRFreeGamma");
        _rblxwin.x11.randr.FreeOutputInfo = (PFN_XRRFreeOutputInfo)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRFreeOutputInfo");
        _rblxwin.x11.randr.FreeScreenResources = (PFN_XRRFreeScreenResources)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRFreeScreenResources");
        _rblxwin.x11.randr.GetCrtcGamma = (PFN_XRRGetCrtcGamma)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRGetCrtcGamma");
        _rblxwin.x11.randr.GetCrtcGammaSize = (PFN_XRRGetCrtcGammaSize)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRGetCrtcGammaSize");
        _rblxwin.x11.randr.GetCrtcInfo = (PFN_XRRGetCrtcInfo)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRGetCrtcInfo");
        _rblxwin.x11.randr.GetOutputInfo = (PFN_XRRGetOutputInfo)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRGetOutputInfo");
        _rblxwin.x11.randr.GetOutputPrimary = (PFN_XRRGetOutputPrimary)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRGetOutputPrimary");
        _rblxwin.x11.randr.GetScreenResourcesCurrent = (PFN_XRRGetScreenResourcesCurrent)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRGetScreenResourcesCurrent");
        _rblxwin.x11.randr.QueryExtension = (PFN_XRRQueryExtension)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRQueryExtension");
        _rblxwin.x11.randr.QueryVersion = (PFN_XRRQueryVersion)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRQueryVersion");
        _rblxwin.x11.randr.SelectInput = (PFN_XRRSelectInput)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRSelectInput");
        _rblxwin.x11.randr.SetCrtcConfig = (PFN_XRRSetCrtcConfig)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRSetCrtcConfig");
        _rblxwin.x11.randr.SetCrtcGamma = (PFN_XRRSetCrtcGamma)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRSetCrtcGamma");
        _rblxwin.x11.randr.UpdateConfiguration = (PFN_XRRUpdateConfiguration)
            _rblxwin_dlsym(_rblxwin.x11.randr.handle, "XRRUpdateConfiguration");

        if (XRRQueryExtension(_rblxwin.x11.display,
                              &_rblxwin.x11.randr.eventBase,
                              &_rblxwin.x11.randr.errorBase))
        {
            if (XRRQueryVersion(_rblxwin.x11.display,
                                &_rblxwin.x11.randr.major,
                                &_rblxwin.x11.randr.minor))
            {
                // The RBLXWIN RandR path requires at least version 1.3
                if (_rblxwin.x11.randr.major > 1 || _rblxwin.x11.randr.minor >= 3)
                    _rblxwin.x11.randr.available = RBLXWIN_TRUE;
            }
            else
            {
                _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                                "X11: Failed to query RandR version");
            }
        }
    }

    if (_rblxwin.x11.randr.available)
    {
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_rblxwin.x11.display,
                                                              _rblxwin.x11.root);

        if (!sr->ncrtc || !XRRGetCrtcGammaSize(_rblxwin.x11.display, sr->crtcs[0]))
        {
            // This is likely an older Nvidia driver with broken gamma support
            // Flag it as useless and fall back to xf86vm gamma, if available
            _rblxwin.x11.randr.gammaBroken = RBLXWIN_TRUE;
        }

        if (!sr->ncrtc)
        {
            // A system without CRTCs is likely a system with broken RandR
            // Disable the RandR monitor path and fall back to core functions
            _rblxwin.x11.randr.monitorBroken = RBLXWIN_TRUE;
        }

        XRRFreeScreenResources(sr);
    }

    if (_rblxwin.x11.randr.available && !_rblxwin.x11.randr.monitorBroken)
    {
        XRRSelectInput(_rblxwin.x11.display, _rblxwin.x11.root,
                       RROutputChangeNotifyMask);
    }

#if defined(__CYGWIN__)
    _rblxwin.x11.xcursor.handle = _rblxwin_dlopen("libXcursor-1.so");
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    _rblxwin.x11.xcursor.handle = _rblxwin_dlopen("libXcursor.so");
#else
    _rblxwin.x11.xcursor.handle = _rblxwin_dlopen("libXcursor.so.1");
#endif
    if (_rblxwin.x11.xcursor.handle)
    {
        _rblxwin.x11.xcursor.ImageCreate = (PFN_XcursorImageCreate)
            _rblxwin_dlsym(_rblxwin.x11.xcursor.handle, "XcursorImageCreate");
        _rblxwin.x11.xcursor.ImageDestroy = (PFN_XcursorImageDestroy)
            _rblxwin_dlsym(_rblxwin.x11.xcursor.handle, "XcursorImageDestroy");
        _rblxwin.x11.xcursor.ImageLoadCursor = (PFN_XcursorImageLoadCursor)
            _rblxwin_dlsym(_rblxwin.x11.xcursor.handle, "XcursorImageLoadCursor");
    }

#if defined(__CYGWIN__)
    _rblxwin.x11.xinerama.handle = _rblxwin_dlopen("libXinerama-1.so");
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    _rblxwin.x11.xinerama.handle = _rblxwin_dlopen("libXinerama.so");
#else
    _rblxwin.x11.xinerama.handle = _rblxwin_dlopen("libXinerama.so.1");
#endif
    if (_rblxwin.x11.xinerama.handle)
    {
        _rblxwin.x11.xinerama.IsActive = (PFN_XineramaIsActive)
            _rblxwin_dlsym(_rblxwin.x11.xinerama.handle, "XineramaIsActive");
        _rblxwin.x11.xinerama.QueryExtension = (PFN_XineramaQueryExtension)
            _rblxwin_dlsym(_rblxwin.x11.xinerama.handle, "XineramaQueryExtension");
        _rblxwin.x11.xinerama.QueryScreens = (PFN_XineramaQueryScreens)
            _rblxwin_dlsym(_rblxwin.x11.xinerama.handle, "XineramaQueryScreens");

        if (XineramaQueryExtension(_rblxwin.x11.display,
                                   &_rblxwin.x11.xinerama.major,
                                   &_rblxwin.x11.xinerama.minor))
        {
            if (XineramaIsActive(_rblxwin.x11.display))
                _rblxwin.x11.xinerama.available = RBLXWIN_TRUE;
        }
    }

    _rblxwin.x11.xkb.major = 1;
    _rblxwin.x11.xkb.minor = 0;
    _rblxwin.x11.xkb.available =
        XkbQueryExtension(_rblxwin.x11.display,
                          &_rblxwin.x11.xkb.majorOpcode,
                          &_rblxwin.x11.xkb.eventBase,
                          &_rblxwin.x11.xkb.errorBase,
                          &_rblxwin.x11.xkb.major,
                          &_rblxwin.x11.xkb.minor);

    if (_rblxwin.x11.xkb.available)
    {
        Bool supported;

        if (XkbSetDetectableAutoRepeat(_rblxwin.x11.display, True, &supported))
        {
            if (supported)
                _rblxwin.x11.xkb.detectable = RBLXWIN_TRUE;
        }

        XkbStateRec state;
        if (XkbGetState(_rblxwin.x11.display, XkbUseCoreKbd, &state) == Success)
            _rblxwin.x11.xkb.group = (unsigned int)state.group;

        XkbSelectEventDetails(_rblxwin.x11.display, XkbUseCoreKbd, XkbStateNotify,
                              XkbGroupStateMask, XkbGroupStateMask);
    }

#if defined(__CYGWIN__)
    _rblxwin.x11.x11xcb.handle = _rblxwin_dlopen("libX11-xcb-1.so");
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    _rblxwin.x11.x11xcb.handle = _rblxwin_dlopen("libX11-xcb.so");
#else
    _rblxwin.x11.x11xcb.handle = _rblxwin_dlopen("libX11-xcb.so.1");
#endif
    if (_rblxwin.x11.x11xcb.handle)
    {
        _rblxwin.x11.x11xcb.GetXCBConnection = (PFN_XGetXCBConnection)
            _rblxwin_dlsym(_rblxwin.x11.x11xcb.handle, "XGetXCBConnection");
    }

#if defined(__CYGWIN__)
    _rblxwin.x11.xrender.handle = _rblxwin_dlopen("libXrender-1.so");
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    _rblxwin.x11.xrender.handle = _rblxwin_dlopen("libXrender.so");
#else
    _rblxwin.x11.xrender.handle = _rblxwin_dlopen("libXrender.so.1");
#endif
    if (_rblxwin.x11.xrender.handle)
    {
        _rblxwin.x11.xrender.QueryExtension = (PFN_XRenderQueryExtension)
            _rblxwin_dlsym(_rblxwin.x11.xrender.handle, "XRenderQueryExtension");
        _rblxwin.x11.xrender.QueryVersion = (PFN_XRenderQueryVersion)
            _rblxwin_dlsym(_rblxwin.x11.xrender.handle, "XRenderQueryVersion");
        _rblxwin.x11.xrender.FindVisualFormat = (PFN_XRenderFindVisualFormat)
            _rblxwin_dlsym(_rblxwin.x11.xrender.handle, "XRenderFindVisualFormat");

        if (XRenderQueryExtension(_rblxwin.x11.display,
                                  &_rblxwin.x11.xrender.errorBase,
                                  &_rblxwin.x11.xrender.eventBase))
        {
            if (XRenderQueryVersion(_rblxwin.x11.display,
                                    &_rblxwin.x11.xrender.major,
                                    &_rblxwin.x11.xrender.minor))
            {
                _rblxwin.x11.xrender.available = RBLXWIN_TRUE;
            }
        }
    }

    // Update the key code LUT
    // FIXME: We should listen to XkbMapNotify events to track changes to
    // the keyboard mapping.
    createKeyTables();

    // String format atoms
    _rblxwin.x11.NULL_ = XInternAtom(_rblxwin.x11.display, "NULL", False);
    _rblxwin.x11.UTF8_STRING = XInternAtom(_rblxwin.x11.display, "UTF8_STRING", False);
    _rblxwin.x11.ATOM_PAIR = XInternAtom(_rblxwin.x11.display, "ATOM_PAIR", False);

    // Custom selection property atom
    _rblxwin.x11.RBLXWIN_SELECTION =
        XInternAtom(_rblxwin.x11.display, "RBLXWIN_SELECTION", False);

    // ICCCM standard clipboard atoms
    _rblxwin.x11.TARGETS = XInternAtom(_rblxwin.x11.display, "TARGETS", False);
    _rblxwin.x11.MULTIPLE = XInternAtom(_rblxwin.x11.display, "MULTIPLE", False);
    _rblxwin.x11.PRIMARY = XInternAtom(_rblxwin.x11.display, "PRIMARY", False);
    _rblxwin.x11.INCR = XInternAtom(_rblxwin.x11.display, "INCR", False);
    _rblxwin.x11.CLIPBOARD = XInternAtom(_rblxwin.x11.display, "CLIPBOARD", False);

    // Clipboard manager atoms
    _rblxwin.x11.CLIPBOARD_MANAGER =
        XInternAtom(_rblxwin.x11.display, "CLIPBOARD_MANAGER", False);
    _rblxwin.x11.SAVE_TARGETS =
        XInternAtom(_rblxwin.x11.display, "SAVE_TARGETS", False);

    // Xdnd (drag and drop) atoms
    _rblxwin.x11.XdndAware = XInternAtom(_rblxwin.x11.display, "XdndAware", False);
    _rblxwin.x11.XdndEnter = XInternAtom(_rblxwin.x11.display, "XdndEnter", False);
    _rblxwin.x11.XdndPosition = XInternAtom(_rblxwin.x11.display, "XdndPosition", False);
    _rblxwin.x11.XdndStatus = XInternAtom(_rblxwin.x11.display, "XdndStatus", False);
    _rblxwin.x11.XdndActionCopy = XInternAtom(_rblxwin.x11.display, "XdndActionCopy", False);
    _rblxwin.x11.XdndDrop = XInternAtom(_rblxwin.x11.display, "XdndDrop", False);
    _rblxwin.x11.XdndFinished = XInternAtom(_rblxwin.x11.display, "XdndFinished", False);
    _rblxwin.x11.XdndSelection = XInternAtom(_rblxwin.x11.display, "XdndSelection", False);
    _rblxwin.x11.XdndTypeList = XInternAtom(_rblxwin.x11.display, "XdndTypeList", False);
    _rblxwin.x11.text_uri_list = XInternAtom(_rblxwin.x11.display, "text/uri-list", False);

    // ICCCM, EWMH and Motif window property atoms
    // These can be set safely even without WM support
    // The EWMH atoms that require WM support are handled in detectEWMH
    _rblxwin.x11.WM_PROTOCOLS =
        XInternAtom(_rblxwin.x11.display, "WM_PROTOCOLS", False);
    _rblxwin.x11.WM_STATE =
        XInternAtom(_rblxwin.x11.display, "WM_STATE", False);
    _rblxwin.x11.WM_DELETE_WINDOW =
        XInternAtom(_rblxwin.x11.display, "WM_DELETE_WINDOW", False);
    _rblxwin.x11.NET_SUPPORTED =
        XInternAtom(_rblxwin.x11.display, "_NET_SUPPORTED", False);
    _rblxwin.x11.NET_SUPPORTING_WM_CHECK =
        XInternAtom(_rblxwin.x11.display, "_NET_SUPPORTING_WM_CHECK", False);
    _rblxwin.x11.NET_WM_ICON =
        XInternAtom(_rblxwin.x11.display, "_NET_WM_ICON", False);
    _rblxwin.x11.NET_WM_PING =
        XInternAtom(_rblxwin.x11.display, "_NET_WM_PING", False);
    _rblxwin.x11.NET_WM_PID =
        XInternAtom(_rblxwin.x11.display, "_NET_WM_PID", False);
    _rblxwin.x11.NET_WM_NAME =
        XInternAtom(_rblxwin.x11.display, "_NET_WM_NAME", False);
    _rblxwin.x11.NET_WM_ICON_NAME =
        XInternAtom(_rblxwin.x11.display, "_NET_WM_ICON_NAME", False);
    _rblxwin.x11.NET_WM_BYPASS_COMPOSITOR =
        XInternAtom(_rblxwin.x11.display, "_NET_WM_BYPASS_COMPOSITOR", False);
    _rblxwin.x11.NET_WM_WINDOW_OPACITY =
        XInternAtom(_rblxwin.x11.display, "_NET_WM_WINDOW_OPACITY", False);
    _rblxwin.x11.MOTIF_WM_HINTS =
        XInternAtom(_rblxwin.x11.display, "_MOTIF_WM_HINTS", False);

    // The compositing manager selection name contains the screen number
    {
        char name[32];
        snprintf(name, sizeof(name), "_NET_WM_CM_S%u", _rblxwin.x11.screen);
        _rblxwin.x11.NET_WM_CM_Sx = XInternAtom(_rblxwin.x11.display, name, False);
    }

    // Detect whether an EWMH-conformant window manager is running
    detectEWMH();

    return RBLXWIN_TRUE;
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
    char* rms = XResourceManagerString(_rblxwin.x11.display);
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
    RBLXWINimage image = { 16, 16, pixels };
    return _rblxwinCreateCursorX11(&image, 0, 0);
}

// Create a helper window for IPC
//
static Window createHelperWindow(void)
{
    XSetWindowAttributes wa;
    wa.event_mask = PropertyChangeMask;

    return XCreateWindow(_rblxwin.x11.display, _rblxwin.x11.root,
                         0, 0, 1, 1, 0, 0,
                         InputOnly,
                         DefaultVisual(_rblxwin.x11.display, _rblxwin.x11.screen),
                         CWEventMask, &wa);
}

// Create the pipe for empty events without assumuing the OS has pipe2(2)
//
static RBLXWINbool createEmptyEventPipe(void)
{
    if (pipe(_rblxwin.x11.emptyEventPipe) != 0)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "X11: Failed to create empty event pipe: %s",
                        strerror(errno));
        return RBLXWIN_FALSE;
    }

    for (int i = 0; i < 2; i++)
    {
        const int sf = fcntl(_rblxwin.x11.emptyEventPipe[i], F_GETFL, 0);
        const int df = fcntl(_rblxwin.x11.emptyEventPipe[i], F_GETFD, 0);

        if (sf == -1 || df == -1 ||
            fcntl(_rblxwin.x11.emptyEventPipe[i], F_SETFL, sf | O_NONBLOCK) == -1 ||
            fcntl(_rblxwin.x11.emptyEventPipe[i], F_SETFD, df | FD_CLOEXEC) == -1)
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "X11: Failed to set flags for empty event pipe: %s",
                            strerror(errno));
            return RBLXWIN_FALSE;
        }
    }

    return RBLXWIN_TRUE;
}

// X error handler
//
static int errorHandler(Display *display, XErrorEvent* event)
{
    if (_rblxwin.x11.display != display)
        return 0;

    _rblxwin.x11.errorCode = event->error_code;
    return 0;
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Sets the X error handler callback
//
void _rblxwinGrabErrorHandlerX11(void)
{
    assert(_rblxwin.x11.errorHandler == NULL);
    _rblxwin.x11.errorCode = Success;
    _rblxwin.x11.errorHandler = XSetErrorHandler(errorHandler);
}

// Clears the X error handler callback
//
void _rblxwinReleaseErrorHandlerX11(void)
{
    // Synchronize to make sure all commands are processed
    XSync(_rblxwin.x11.display, False);
    XSetErrorHandler(_rblxwin.x11.errorHandler);
    _rblxwin.x11.errorHandler = NULL;
}

// Reports the specified error, appending information about the last X error
//
void _rblxwinInputErrorX11(int error, const char* message)
{
    char buffer[_RBLXWIN_MESSAGE_SIZE];
    XGetErrorText(_rblxwin.x11.display, _rblxwin.x11.errorCode,
                  buffer, sizeof(buffer));

    _rblxwinInputError(error, "%s: %s", message, buffer);
}

// Creates a native cursor object from the specified image and hotspot
//
Cursor _rblxwinCreateCursorX11(const RBLXWINimage* image, int xhot, int yhot)
{
    int i;
    Cursor cursor;

    if (!_rblxwin.x11.xcursor.handle)
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

    cursor = XcursorImageLoadCursor(_rblxwin.x11.display, native);
    XcursorImageDestroy(native);

    return cursor;
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _rblxwinPlatformInit(void)
{
    // HACK: If the application has left the locale as "C" then both wide
    //       character text input and explicit UTF-8 input via XIM will break
    //       This sets the CTYPE part of the current locale from the environment
    //       in the hope that it is set to something more sane than "C"
    if (strcmp(setlocale(LC_CTYPE, NULL), "C") == 0)
        setlocale(LC_CTYPE, "");

    XInitThreads();
    XrmInitialize();

    _rblxwin.x11.display = XOpenDisplay(NULL);
    if (!_rblxwin.x11.display)
    {
        const char* display = getenv("DISPLAY");
        if (display)
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "X11: Failed to open display %s", display);
        }
        else
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "X11: The DISPLAY environment variable is missing");
        }

        return RBLXWIN_FALSE;
    }

    _rblxwin.x11.screen = DefaultScreen(_rblxwin.x11.display);
    _rblxwin.x11.root = RootWindow(_rblxwin.x11.display, _rblxwin.x11.screen);
    _rblxwin.x11.context = XUniqueContext();

    getSystemContentScale(&_rblxwin.x11.contentScaleX, &_rblxwin.x11.contentScaleY);

    if (!createEmptyEventPipe())
        return RBLXWIN_FALSE;

    if (!initExtensions())
        return RBLXWIN_FALSE;

    _rblxwin.x11.helperWindowHandle = createHelperWindow();
    _rblxwin.x11.hiddenCursorHandle = createHiddenCursor();

    if (XSupportsLocale())
    {
        XSetLocaleModifiers("");

        _rblxwin.x11.im = XOpenIM(_rblxwin.x11.display, 0, NULL, NULL);
        if (_rblxwin.x11.im)
        {
            if (!hasUsableInputMethodStyle())
            {
                XCloseIM(_rblxwin.x11.im);
                _rblxwin.x11.im = NULL;
            }
        }
    }

#if defined(__linux__)
    if (!_rblxwinInitJoysticksLinux())
        return RBLXWIN_FALSE;
#endif

    _rblxwinInitTimerPOSIX();

    _rblxwinPollMonitorsX11();
    return RBLXWIN_TRUE;
}

void _rblxwinPlatformTerminate(void)
{
    if (_rblxwin.x11.helperWindowHandle)
    {
        if (XGetSelectionOwner(_rblxwin.x11.display, _rblxwin.x11.CLIPBOARD) ==
            _rblxwin.x11.helperWindowHandle)
        {
            _rblxwinPushSelectionToManagerX11();
        }

        XDestroyWindow(_rblxwin.x11.display, _rblxwin.x11.helperWindowHandle);
        _rblxwin.x11.helperWindowHandle = None;
    }

    if (_rblxwin.x11.hiddenCursorHandle)
    {
        XFreeCursor(_rblxwin.x11.display, _rblxwin.x11.hiddenCursorHandle);
        _rblxwin.x11.hiddenCursorHandle = (Cursor) 0;
    }

    free(_rblxwin.x11.primarySelectionString);
    free(_rblxwin.x11.clipboardString);

    if (_rblxwin.x11.im)
    {
        XCloseIM(_rblxwin.x11.im);
        _rblxwin.x11.im = NULL;
    }

    if (_rblxwin.x11.display)
    {
        XCloseDisplay(_rblxwin.x11.display);
        _rblxwin.x11.display = NULL;
    }

    if (_rblxwin.x11.x11xcb.handle)
    {
        _rblxwin_dlclose(_rblxwin.x11.x11xcb.handle);
        _rblxwin.x11.x11xcb.handle = NULL;
    }

    if (_rblxwin.x11.xcursor.handle)
    {
        _rblxwin_dlclose(_rblxwin.x11.xcursor.handle);
        _rblxwin.x11.xcursor.handle = NULL;
    }

    if (_rblxwin.x11.randr.handle)
    {
        _rblxwin_dlclose(_rblxwin.x11.randr.handle);
        _rblxwin.x11.randr.handle = NULL;
    }

    if (_rblxwin.x11.xinerama.handle)
    {
        _rblxwin_dlclose(_rblxwin.x11.xinerama.handle);
        _rblxwin.x11.xinerama.handle = NULL;
    }

    if (_rblxwin.x11.xrender.handle)
    {
        _rblxwin_dlclose(_rblxwin.x11.xrender.handle);
        _rblxwin.x11.xrender.handle = NULL;
    }

    if (_rblxwin.x11.vidmode.handle)
    {
        _rblxwin_dlclose(_rblxwin.x11.vidmode.handle);
        _rblxwin.x11.vidmode.handle = NULL;
    }

    if (_rblxwin.x11.xi.handle)
    {
        _rblxwin_dlclose(_rblxwin.x11.xi.handle);
        _rblxwin.x11.xi.handle = NULL;
    }

    _rblxwinTerminateOSMesa();
    // NOTE: These need to be unloaded after XCloseDisplay, as they register
    //       cleanup callbacks that get called by that function
    _rblxwinTerminateEGL();
    _rblxwinTerminateGLX();

#if defined(__linux__)
    _rblxwinTerminateJoysticksLinux();
#endif

    if (_rblxwin.x11.emptyEventPipe[0] || _rblxwin.x11.emptyEventPipe[1])
    {
        close(_rblxwin.x11.emptyEventPipe[0]);
        close(_rblxwin.x11.emptyEventPipe[1]);
    }
}

const char* _rblxwinPlatformGetVersionString(void)
{
    return _RBLXWIN_VERSION_NUMBER " X11 GLX EGL OSMesa"
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
        " clock_gettime"
#else
        " gettimeofday"
#endif
#if defined(__linux__)
        " evdev"
#endif
#if defined(_RBLXWIN_BUILD_DLL)
        " shared"
#endif
        ;
}

