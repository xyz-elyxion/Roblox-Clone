//========================================================================
// RBLXWIN 3.3 Wayland - www.rblxwin.org
//------------------------------------------------------------------------
// Copyright (c) 2014 Jonas Ådahl <jadahl@gmail.com>
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

#define _POSIX_C_SOURCE 200809L

#include "internal.h"

#include <errno.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <wayland-client.h>

static void wmBaseHandlePing(void* userData,
                             struct xdg_wm_base* wmBase,
                             uint32_t serial)
{
    xdg_wm_base_pong(wmBase, serial);
}

static const struct xdg_wm_base_listener wmBaseListener =
{
    wmBaseHandlePing
};

static void registryHandleGlobal(void* userData,
                                 struct wl_registry* registry,
                                 uint32_t name,
                                 const char* interface,
                                 uint32_t version)
{
    if (strcmp(interface, "wl_compositor") == 0)
    {
        _rblxwin.wl.compositorVersion = _rblxwin_min(3, version);
        _rblxwin.wl.compositor =
            wl_registry_bind(registry, name, &wl_compositor_interface,
                             _rblxwin.wl.compositorVersion);
    }
    else if (strcmp(interface, "wl_subcompositor") == 0)
    {
        _rblxwin.wl.subcompositor =
            wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);
    }
    else if (strcmp(interface, "wl_shm") == 0)
    {
        _rblxwin.wl.shm =
            wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }
    else if (strcmp(interface, "wl_output") == 0)
    {
        _rblxwinAddOutputWayland(name, version);
    }
    else if (strcmp(interface, "wl_seat") == 0)
    {
        if (!_rblxwin.wl.seat)
        {
            _rblxwin.wl.seatVersion = _rblxwin_min(4, version);
            _rblxwin.wl.seat =
                wl_registry_bind(registry, name, &wl_seat_interface,
                                 _rblxwin.wl.seatVersion);
            _rblxwinAddSeatListenerWayland(_rblxwin.wl.seat);
        }
    }
    else if (strcmp(interface, "wl_data_device_manager") == 0)
    {
        if (!_rblxwin.wl.dataDeviceManager)
        {
            _rblxwin.wl.dataDeviceManager =
                wl_registry_bind(registry, name,
                                 &wl_data_device_manager_interface, 1);
        }
    }
    else if (strcmp(interface, "xdg_wm_base") == 0)
    {
        _rblxwin.wl.wmBase =
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(_rblxwin.wl.wmBase, &wmBaseListener, NULL);
    }
    else if (strcmp(interface, "zxdg_decoration_manager_v1") == 0)
    {
        _rblxwin.wl.decorationManager =
            wl_registry_bind(registry, name,
                             &zxdg_decoration_manager_v1_interface,
                             1);
    }
    else if (strcmp(interface, "wp_viewporter") == 0)
    {
        _rblxwin.wl.viewporter =
            wl_registry_bind(registry, name, &wp_viewporter_interface, 1);
    }
    else if (strcmp(interface, "zwp_relative_pointer_manager_v1") == 0)
    {
        _rblxwin.wl.relativePointerManager =
            wl_registry_bind(registry, name,
                             &zwp_relative_pointer_manager_v1_interface,
                             1);
    }
    else if (strcmp(interface, "zwp_pointer_constraints_v1") == 0)
    {
        _rblxwin.wl.pointerConstraints =
            wl_registry_bind(registry, name,
                             &zwp_pointer_constraints_v1_interface,
                             1);
    }
    else if (strcmp(interface, "zwp_idle_inhibit_manager_v1") == 0)
    {
        _rblxwin.wl.idleInhibitManager =
            wl_registry_bind(registry, name,
                             &zwp_idle_inhibit_manager_v1_interface,
                             1);
    }
}

static void registryHandleGlobalRemove(void* userData,
                                       struct wl_registry* registry,
                                       uint32_t name)
{
    int i;

    for (i = 0; i < _rblxwin.monitorCount; ++i)
    {
        _RBLXWINmonitor* monitor = _rblxwin.monitors[i];
        if (monitor->wl.name == name)
        {
            _rblxwinInputMonitor(monitor, RBLXWIN_DISCONNECTED, 0);
            return;
        }
    }
}


static const struct wl_registry_listener registryListener =
{
    registryHandleGlobal,
    registryHandleGlobalRemove
};

// Create key code translation tables
//
static void createKeyTables(void)
{
    int scancode;

    memset(_rblxwin.wl.keycodes, -1, sizeof(_rblxwin.wl.keycodes));
    memset(_rblxwin.wl.scancodes, -1, sizeof(_rblxwin.wl.scancodes));

    _rblxwin.wl.keycodes[KEY_GRAVE]      = RBLXWIN_KEY_GRAVE_ACCENT;
    _rblxwin.wl.keycodes[KEY_1]          = RBLXWIN_KEY_1;
    _rblxwin.wl.keycodes[KEY_2]          = RBLXWIN_KEY_2;
    _rblxwin.wl.keycodes[KEY_3]          = RBLXWIN_KEY_3;
    _rblxwin.wl.keycodes[KEY_4]          = RBLXWIN_KEY_4;
    _rblxwin.wl.keycodes[KEY_5]          = RBLXWIN_KEY_5;
    _rblxwin.wl.keycodes[KEY_6]          = RBLXWIN_KEY_6;
    _rblxwin.wl.keycodes[KEY_7]          = RBLXWIN_KEY_7;
    _rblxwin.wl.keycodes[KEY_8]          = RBLXWIN_KEY_8;
    _rblxwin.wl.keycodes[KEY_9]          = RBLXWIN_KEY_9;
    _rblxwin.wl.keycodes[KEY_0]          = RBLXWIN_KEY_0;
    _rblxwin.wl.keycodes[KEY_SPACE]      = RBLXWIN_KEY_SPACE;
    _rblxwin.wl.keycodes[KEY_MINUS]      = RBLXWIN_KEY_MINUS;
    _rblxwin.wl.keycodes[KEY_EQUAL]      = RBLXWIN_KEY_EQUAL;
    _rblxwin.wl.keycodes[KEY_Q]          = RBLXWIN_KEY_Q;
    _rblxwin.wl.keycodes[KEY_W]          = RBLXWIN_KEY_W;
    _rblxwin.wl.keycodes[KEY_E]          = RBLXWIN_KEY_E;
    _rblxwin.wl.keycodes[KEY_R]          = RBLXWIN_KEY_R;
    _rblxwin.wl.keycodes[KEY_T]          = RBLXWIN_KEY_T;
    _rblxwin.wl.keycodes[KEY_Y]          = RBLXWIN_KEY_Y;
    _rblxwin.wl.keycodes[KEY_U]          = RBLXWIN_KEY_U;
    _rblxwin.wl.keycodes[KEY_I]          = RBLXWIN_KEY_I;
    _rblxwin.wl.keycodes[KEY_O]          = RBLXWIN_KEY_O;
    _rblxwin.wl.keycodes[KEY_P]          = RBLXWIN_KEY_P;
    _rblxwin.wl.keycodes[KEY_LEFTBRACE]  = RBLXWIN_KEY_LEFT_BRACKET;
    _rblxwin.wl.keycodes[KEY_RIGHTBRACE] = RBLXWIN_KEY_RIGHT_BRACKET;
    _rblxwin.wl.keycodes[KEY_A]          = RBLXWIN_KEY_A;
    _rblxwin.wl.keycodes[KEY_S]          = RBLXWIN_KEY_S;
    _rblxwin.wl.keycodes[KEY_D]          = RBLXWIN_KEY_D;
    _rblxwin.wl.keycodes[KEY_F]          = RBLXWIN_KEY_F;
    _rblxwin.wl.keycodes[KEY_G]          = RBLXWIN_KEY_G;
    _rblxwin.wl.keycodes[KEY_H]          = RBLXWIN_KEY_H;
    _rblxwin.wl.keycodes[KEY_J]          = RBLXWIN_KEY_J;
    _rblxwin.wl.keycodes[KEY_K]          = RBLXWIN_KEY_K;
    _rblxwin.wl.keycodes[KEY_L]          = RBLXWIN_KEY_L;
    _rblxwin.wl.keycodes[KEY_SEMICOLON]  = RBLXWIN_KEY_SEMICOLON;
    _rblxwin.wl.keycodes[KEY_APOSTROPHE] = RBLXWIN_KEY_APOSTROPHE;
    _rblxwin.wl.keycodes[KEY_Z]          = RBLXWIN_KEY_Z;
    _rblxwin.wl.keycodes[KEY_X]          = RBLXWIN_KEY_X;
    _rblxwin.wl.keycodes[KEY_C]          = RBLXWIN_KEY_C;
    _rblxwin.wl.keycodes[KEY_V]          = RBLXWIN_KEY_V;
    _rblxwin.wl.keycodes[KEY_B]          = RBLXWIN_KEY_B;
    _rblxwin.wl.keycodes[KEY_N]          = RBLXWIN_KEY_N;
    _rblxwin.wl.keycodes[KEY_M]          = RBLXWIN_KEY_M;
    _rblxwin.wl.keycodes[KEY_COMMA]      = RBLXWIN_KEY_COMMA;
    _rblxwin.wl.keycodes[KEY_DOT]        = RBLXWIN_KEY_PERIOD;
    _rblxwin.wl.keycodes[KEY_SLASH]      = RBLXWIN_KEY_SLASH;
    _rblxwin.wl.keycodes[KEY_BACKSLASH]  = RBLXWIN_KEY_BACKSLASH;
    _rblxwin.wl.keycodes[KEY_ESC]        = RBLXWIN_KEY_ESCAPE;
    _rblxwin.wl.keycodes[KEY_TAB]        = RBLXWIN_KEY_TAB;
    _rblxwin.wl.keycodes[KEY_LEFTSHIFT]  = RBLXWIN_KEY_LEFT_SHIFT;
    _rblxwin.wl.keycodes[KEY_RIGHTSHIFT] = RBLXWIN_KEY_RIGHT_SHIFT;
    _rblxwin.wl.keycodes[KEY_LEFTCTRL]   = RBLXWIN_KEY_LEFT_CONTROL;
    _rblxwin.wl.keycodes[KEY_RIGHTCTRL]  = RBLXWIN_KEY_RIGHT_CONTROL;
    _rblxwin.wl.keycodes[KEY_LEFTALT]    = RBLXWIN_KEY_LEFT_ALT;
    _rblxwin.wl.keycodes[KEY_RIGHTALT]   = RBLXWIN_KEY_RIGHT_ALT;
    _rblxwin.wl.keycodes[KEY_LEFTMETA]   = RBLXWIN_KEY_LEFT_SUPER;
    _rblxwin.wl.keycodes[KEY_RIGHTMETA]  = RBLXWIN_KEY_RIGHT_SUPER;
    _rblxwin.wl.keycodes[KEY_COMPOSE]    = RBLXWIN_KEY_MENU;
    _rblxwin.wl.keycodes[KEY_NUMLOCK]    = RBLXWIN_KEY_NUM_LOCK;
    _rblxwin.wl.keycodes[KEY_CAPSLOCK]   = RBLXWIN_KEY_CAPS_LOCK;
    _rblxwin.wl.keycodes[KEY_PRINT]      = RBLXWIN_KEY_PRINT_SCREEN;
    _rblxwin.wl.keycodes[KEY_SCROLLLOCK] = RBLXWIN_KEY_SCROLL_LOCK;
    _rblxwin.wl.keycodes[KEY_PAUSE]      = RBLXWIN_KEY_PAUSE;
    _rblxwin.wl.keycodes[KEY_DELETE]     = RBLXWIN_KEY_DELETE;
    _rblxwin.wl.keycodes[KEY_BACKSPACE]  = RBLXWIN_KEY_BACKSPACE;
    _rblxwin.wl.keycodes[KEY_ENTER]      = RBLXWIN_KEY_ENTER;
    _rblxwin.wl.keycodes[KEY_HOME]       = RBLXWIN_KEY_HOME;
    _rblxwin.wl.keycodes[KEY_END]        = RBLXWIN_KEY_END;
    _rblxwin.wl.keycodes[KEY_PAGEUP]     = RBLXWIN_KEY_PAGE_UP;
    _rblxwin.wl.keycodes[KEY_PAGEDOWN]   = RBLXWIN_KEY_PAGE_DOWN;
    _rblxwin.wl.keycodes[KEY_INSERT]     = RBLXWIN_KEY_INSERT;
    _rblxwin.wl.keycodes[KEY_LEFT]       = RBLXWIN_KEY_LEFT;
    _rblxwin.wl.keycodes[KEY_RIGHT]      = RBLXWIN_KEY_RIGHT;
    _rblxwin.wl.keycodes[KEY_DOWN]       = RBLXWIN_KEY_DOWN;
    _rblxwin.wl.keycodes[KEY_UP]         = RBLXWIN_KEY_UP;
    _rblxwin.wl.keycodes[KEY_F1]         = RBLXWIN_KEY_F1;
    _rblxwin.wl.keycodes[KEY_F2]         = RBLXWIN_KEY_F2;
    _rblxwin.wl.keycodes[KEY_F3]         = RBLXWIN_KEY_F3;
    _rblxwin.wl.keycodes[KEY_F4]         = RBLXWIN_KEY_F4;
    _rblxwin.wl.keycodes[KEY_F5]         = RBLXWIN_KEY_F5;
    _rblxwin.wl.keycodes[KEY_F6]         = RBLXWIN_KEY_F6;
    _rblxwin.wl.keycodes[KEY_F7]         = RBLXWIN_KEY_F7;
    _rblxwin.wl.keycodes[KEY_F8]         = RBLXWIN_KEY_F8;
    _rblxwin.wl.keycodes[KEY_F9]         = RBLXWIN_KEY_F9;
    _rblxwin.wl.keycodes[KEY_F10]        = RBLXWIN_KEY_F10;
    _rblxwin.wl.keycodes[KEY_F11]        = RBLXWIN_KEY_F11;
    _rblxwin.wl.keycodes[KEY_F12]        = RBLXWIN_KEY_F12;
    _rblxwin.wl.keycodes[KEY_F13]        = RBLXWIN_KEY_F13;
    _rblxwin.wl.keycodes[KEY_F14]        = RBLXWIN_KEY_F14;
    _rblxwin.wl.keycodes[KEY_F15]        = RBLXWIN_KEY_F15;
    _rblxwin.wl.keycodes[KEY_F16]        = RBLXWIN_KEY_F16;
    _rblxwin.wl.keycodes[KEY_F17]        = RBLXWIN_KEY_F17;
    _rblxwin.wl.keycodes[KEY_F18]        = RBLXWIN_KEY_F18;
    _rblxwin.wl.keycodes[KEY_F19]        = RBLXWIN_KEY_F19;
    _rblxwin.wl.keycodes[KEY_F20]        = RBLXWIN_KEY_F20;
    _rblxwin.wl.keycodes[KEY_F21]        = RBLXWIN_KEY_F21;
    _rblxwin.wl.keycodes[KEY_F22]        = RBLXWIN_KEY_F22;
    _rblxwin.wl.keycodes[KEY_F23]        = RBLXWIN_KEY_F23;
    _rblxwin.wl.keycodes[KEY_F24]        = RBLXWIN_KEY_F24;
    _rblxwin.wl.keycodes[KEY_KPSLASH]    = RBLXWIN_KEY_KP_DIVIDE;
    _rblxwin.wl.keycodes[KEY_KPASTERISK] = RBLXWIN_KEY_KP_MULTIPLY;
    _rblxwin.wl.keycodes[KEY_KPMINUS]    = RBLXWIN_KEY_KP_SUBTRACT;
    _rblxwin.wl.keycodes[KEY_KPPLUS]     = RBLXWIN_KEY_KP_ADD;
    _rblxwin.wl.keycodes[KEY_KP0]        = RBLXWIN_KEY_KP_0;
    _rblxwin.wl.keycodes[KEY_KP1]        = RBLXWIN_KEY_KP_1;
    _rblxwin.wl.keycodes[KEY_KP2]        = RBLXWIN_KEY_KP_2;
    _rblxwin.wl.keycodes[KEY_KP3]        = RBLXWIN_KEY_KP_3;
    _rblxwin.wl.keycodes[KEY_KP4]        = RBLXWIN_KEY_KP_4;
    _rblxwin.wl.keycodes[KEY_KP5]        = RBLXWIN_KEY_KP_5;
    _rblxwin.wl.keycodes[KEY_KP6]        = RBLXWIN_KEY_KP_6;
    _rblxwin.wl.keycodes[KEY_KP7]        = RBLXWIN_KEY_KP_7;
    _rblxwin.wl.keycodes[KEY_KP8]        = RBLXWIN_KEY_KP_8;
    _rblxwin.wl.keycodes[KEY_KP9]        = RBLXWIN_KEY_KP_9;
    _rblxwin.wl.keycodes[KEY_KPDOT]      = RBLXWIN_KEY_KP_DECIMAL;
    _rblxwin.wl.keycodes[KEY_KPEQUAL]    = RBLXWIN_KEY_KP_EQUAL;
    _rblxwin.wl.keycodes[KEY_KPENTER]    = RBLXWIN_KEY_KP_ENTER;
    _rblxwin.wl.keycodes[KEY_102ND]      = RBLXWIN_KEY_WORLD_2;

    for (scancode = 0;  scancode < 256;  scancode++)
    {
        if (_rblxwin.wl.keycodes[scancode] > 0)
            _rblxwin.wl.scancodes[_rblxwin.wl.keycodes[scancode]] = scancode;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _rblxwinPlatformInit(void)
{
    const char* cursorTheme;
    const char* cursorSizeStr;
    char* cursorSizeEnd;
    long cursorSizeLong;
    int cursorSize;

    // These must be set before any failure checks
    _rblxwin.wl.timerfd = -1;
    _rblxwin.wl.cursorTimerfd = -1;

    _rblxwin.wl.cursor.handle = _rblxwin_dlopen("libwayland-cursor.so.0");
    if (!_rblxwin.wl.cursor.handle)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to load libwayland-cursor");
        return RBLXWIN_FALSE;
    }

    _rblxwin.wl.cursor.theme_load = (PFN_wl_cursor_theme_load)
        _rblxwin_dlsym(_rblxwin.wl.cursor.handle, "wl_cursor_theme_load");
    _rblxwin.wl.cursor.theme_destroy = (PFN_wl_cursor_theme_destroy)
        _rblxwin_dlsym(_rblxwin.wl.cursor.handle, "wl_cursor_theme_destroy");
    _rblxwin.wl.cursor.theme_get_cursor = (PFN_wl_cursor_theme_get_cursor)
        _rblxwin_dlsym(_rblxwin.wl.cursor.handle, "wl_cursor_theme_get_cursor");
    _rblxwin.wl.cursor.image_get_buffer = (PFN_wl_cursor_image_get_buffer)
        _rblxwin_dlsym(_rblxwin.wl.cursor.handle, "wl_cursor_image_get_buffer");

    _rblxwin.wl.egl.handle = _rblxwin_dlopen("libwayland-egl.so.1");
    if (!_rblxwin.wl.egl.handle)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to load libwayland-egl");
        return RBLXWIN_FALSE;
    }

    _rblxwin.wl.egl.window_create = (PFN_wl_egl_window_create)
        _rblxwin_dlsym(_rblxwin.wl.egl.handle, "wl_egl_window_create");
    _rblxwin.wl.egl.window_destroy = (PFN_wl_egl_window_destroy)
        _rblxwin_dlsym(_rblxwin.wl.egl.handle, "wl_egl_window_destroy");
    _rblxwin.wl.egl.window_resize = (PFN_wl_egl_window_resize)
        _rblxwin_dlsym(_rblxwin.wl.egl.handle, "wl_egl_window_resize");

    _rblxwin.wl.xkb.handle = _rblxwin_dlopen("libxkbcommon.so.0");
    if (!_rblxwin.wl.xkb.handle)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to load libxkbcommon");
        return RBLXWIN_FALSE;
    }

    _rblxwin.wl.xkb.context_new = (PFN_xkb_context_new)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_context_new");
    _rblxwin.wl.xkb.context_unref = (PFN_xkb_context_unref)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_context_unref");
    _rblxwin.wl.xkb.keymap_new_from_string = (PFN_xkb_keymap_new_from_string)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_keymap_new_from_string");
    _rblxwin.wl.xkb.keymap_unref = (PFN_xkb_keymap_unref)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_keymap_unref");
    _rblxwin.wl.xkb.keymap_mod_get_index = (PFN_xkb_keymap_mod_get_index)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_keymap_mod_get_index");
    _rblxwin.wl.xkb.keymap_key_repeats = (PFN_xkb_keymap_key_repeats)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_keymap_key_repeats");
    _rblxwin.wl.xkb.keymap_key_get_syms_by_level = (PFN_xkb_keymap_key_get_syms_by_level)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_keymap_key_get_syms_by_level");
    _rblxwin.wl.xkb.state_new = (PFN_xkb_state_new)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_state_new");
    _rblxwin.wl.xkb.state_unref = (PFN_xkb_state_unref)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_state_unref");
    _rblxwin.wl.xkb.state_key_get_syms = (PFN_xkb_state_key_get_syms)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_state_key_get_syms");
    _rblxwin.wl.xkb.state_update_mask = (PFN_xkb_state_update_mask)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_state_update_mask");
    _rblxwin.wl.xkb.state_key_get_layout = (PFN_xkb_state_key_get_layout)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_state_key_get_layout");
    _rblxwin.wl.xkb.state_mod_index_is_active = (PFN_xkb_state_mod_index_is_active)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_state_mod_index_is_active");

    _rblxwin.wl.xkb.compose_table_new_from_locale = (PFN_xkb_compose_table_new_from_locale)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_compose_table_new_from_locale");
    _rblxwin.wl.xkb.compose_table_unref = (PFN_xkb_compose_table_unref)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_compose_table_unref");
    _rblxwin.wl.xkb.compose_state_new = (PFN_xkb_compose_state_new)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_compose_state_new");
    _rblxwin.wl.xkb.compose_state_unref = (PFN_xkb_compose_state_unref)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_compose_state_unref");
    _rblxwin.wl.xkb.compose_state_feed = (PFN_xkb_compose_state_feed)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_compose_state_feed");
    _rblxwin.wl.xkb.compose_state_get_status = (PFN_xkb_compose_state_get_status)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_compose_state_get_status");
    _rblxwin.wl.xkb.compose_state_get_one_sym = (PFN_xkb_compose_state_get_one_sym)
        _rblxwin_dlsym(_rblxwin.wl.xkb.handle, "xkb_compose_state_get_one_sym");

    _rblxwin.wl.display = wl_display_connect(NULL);
    if (!_rblxwin.wl.display)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to connect to display");
        return RBLXWIN_FALSE;
    }

    _rblxwin.wl.registry = wl_display_get_registry(_rblxwin.wl.display);
    wl_registry_add_listener(_rblxwin.wl.registry, &registryListener, NULL);

    createKeyTables();

    _rblxwin.wl.xkb.context = xkb_context_new(0);
    if (!_rblxwin.wl.xkb.context)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to initialize xkb context");
        return RBLXWIN_FALSE;
    }

    // Sync so we got all registry objects
    wl_display_roundtrip(_rblxwin.wl.display);

    // Sync so we got all initial output events
    wl_display_roundtrip(_rblxwin.wl.display);

#ifdef __linux__
    if (!_rblxwinInitJoysticksLinux())
        return RBLXWIN_FALSE;
#endif

    _rblxwinInitTimerPOSIX();

#ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
    if (_rblxwin.wl.seatVersion >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
        _rblxwin.wl.timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
#endif

    if (!_rblxwin.wl.wmBase)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to find xdg-shell in your compositor");
        return RBLXWIN_FALSE;
    }

    if (_rblxwin.wl.pointer && _rblxwin.wl.shm)
    {
        cursorTheme = getenv("XCURSOR_THEME");
        cursorSizeStr = getenv("XCURSOR_SIZE");
        cursorSize = 32;
        if (cursorSizeStr)
        {
            errno = 0;
            cursorSizeLong = strtol(cursorSizeStr, &cursorSizeEnd, 10);
            if (!*cursorSizeEnd && !errno && cursorSizeLong > 0 && cursorSizeLong <= INT_MAX)
                cursorSize = (int)cursorSizeLong;
        }
        _rblxwin.wl.cursorTheme =
            wl_cursor_theme_load(cursorTheme, cursorSize, _rblxwin.wl.shm);
        if (!_rblxwin.wl.cursorTheme)
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "Wayland: Failed to load default cursor theme");
            return RBLXWIN_FALSE;
        }
        // If this happens to be NULL, we just fallback to the scale=1 version.
        _rblxwin.wl.cursorThemeHiDPI =
            wl_cursor_theme_load(cursorTheme, 2 * cursorSize, _rblxwin.wl.shm);
        _rblxwin.wl.cursorSurface =
            wl_compositor_create_surface(_rblxwin.wl.compositor);
        _rblxwin.wl.cursorTimerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    }

    if (_rblxwin.wl.seat && _rblxwin.wl.dataDeviceManager)
    {
        _rblxwin.wl.dataDevice =
            wl_data_device_manager_get_data_device(_rblxwin.wl.dataDeviceManager,
                                                   _rblxwin.wl.seat);
        _rblxwinAddDataDeviceListenerWayland(_rblxwin.wl.dataDevice);
    }

    return RBLXWIN_TRUE;
}

void _rblxwinPlatformTerminate(void)
{
#ifdef __linux__
    _rblxwinTerminateJoysticksLinux();
#endif
    _rblxwinTerminateEGL();
    _rblxwinTerminateOSMesa();

    if (_rblxwin.wl.egl.handle)
    {
        _rblxwin_dlclose(_rblxwin.wl.egl.handle);
        _rblxwin.wl.egl.handle = NULL;
    }

    if (_rblxwin.wl.xkb.composeState)
        xkb_compose_state_unref(_rblxwin.wl.xkb.composeState);
    if (_rblxwin.wl.xkb.keymap)
        xkb_keymap_unref(_rblxwin.wl.xkb.keymap);
    if (_rblxwin.wl.xkb.state)
        xkb_state_unref(_rblxwin.wl.xkb.state);
    if (_rblxwin.wl.xkb.context)
        xkb_context_unref(_rblxwin.wl.xkb.context);
    if (_rblxwin.wl.xkb.handle)
    {
        _rblxwin_dlclose(_rblxwin.wl.xkb.handle);
        _rblxwin.wl.xkb.handle = NULL;
    }

    if (_rblxwin.wl.cursorTheme)
        wl_cursor_theme_destroy(_rblxwin.wl.cursorTheme);
    if (_rblxwin.wl.cursorThemeHiDPI)
        wl_cursor_theme_destroy(_rblxwin.wl.cursorThemeHiDPI);
    if (_rblxwin.wl.cursor.handle)
    {
        _rblxwin_dlclose(_rblxwin.wl.cursor.handle);
        _rblxwin.wl.cursor.handle = NULL;
    }

    for (unsigned int i = 0; i < _rblxwin.wl.offerCount; i++)
        wl_data_offer_destroy(_rblxwin.wl.offers[i].offer);

    free(_rblxwin.wl.offers);

    if (_rblxwin.wl.cursorSurface)
        wl_surface_destroy(_rblxwin.wl.cursorSurface);
    if (_rblxwin.wl.subcompositor)
        wl_subcompositor_destroy(_rblxwin.wl.subcompositor);
    if (_rblxwin.wl.compositor)
        wl_compositor_destroy(_rblxwin.wl.compositor);
    if (_rblxwin.wl.shm)
        wl_shm_destroy(_rblxwin.wl.shm);
    if (_rblxwin.wl.viewporter)
        wp_viewporter_destroy(_rblxwin.wl.viewporter);
    if (_rblxwin.wl.decorationManager)
        zxdg_decoration_manager_v1_destroy(_rblxwin.wl.decorationManager);
    if (_rblxwin.wl.wmBase)
        xdg_wm_base_destroy(_rblxwin.wl.wmBase);
    if (_rblxwin.wl.selectionOffer)
        wl_data_offer_destroy(_rblxwin.wl.selectionOffer);
    if (_rblxwin.wl.dragOffer)
        wl_data_offer_destroy(_rblxwin.wl.dragOffer);
    if (_rblxwin.wl.selectionSource)
        wl_data_source_destroy(_rblxwin.wl.selectionSource);
    if (_rblxwin.wl.dataDevice)
        wl_data_device_destroy(_rblxwin.wl.dataDevice);
    if (_rblxwin.wl.dataDeviceManager)
        wl_data_device_manager_destroy(_rblxwin.wl.dataDeviceManager);
    if (_rblxwin.wl.pointer)
        wl_pointer_destroy(_rblxwin.wl.pointer);
    if (_rblxwin.wl.keyboard)
        wl_keyboard_destroy(_rblxwin.wl.keyboard);
    if (_rblxwin.wl.seat)
        wl_seat_destroy(_rblxwin.wl.seat);
    if (_rblxwin.wl.relativePointerManager)
        zwp_relative_pointer_manager_v1_destroy(_rblxwin.wl.relativePointerManager);
    if (_rblxwin.wl.pointerConstraints)
        zwp_pointer_constraints_v1_destroy(_rblxwin.wl.pointerConstraints);
    if (_rblxwin.wl.idleInhibitManager)
        zwp_idle_inhibit_manager_v1_destroy(_rblxwin.wl.idleInhibitManager);
    if (_rblxwin.wl.registry)
        wl_registry_destroy(_rblxwin.wl.registry);
    if (_rblxwin.wl.display)
    {
        wl_display_flush(_rblxwin.wl.display);
        wl_display_disconnect(_rblxwin.wl.display);
    }

    if (_rblxwin.wl.timerfd >= 0)
        close(_rblxwin.wl.timerfd);
    if (_rblxwin.wl.cursorTimerfd >= 0)
        close(_rblxwin.wl.cursorTimerfd);

    free(_rblxwin.wl.clipboardString);
}

const char* _rblxwinPlatformGetVersionString(void)
{
    return _RBLXWIN_VERSION_NUMBER " Wayland EGL OSMesa"
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
        " clock_gettime"
#else
        " gettimeofday"
#endif
        " evdev"
#if defined(_RBLXWIN_BUILD_DLL)
        " shared"
#endif
        ;
}
