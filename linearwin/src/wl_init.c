//========================================================================
// LINEARWIN 3.3 Wayland - www.linearwin.org
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
        _linearwin.wl.compositorVersion = _linearwin_min(3, version);
        _linearwin.wl.compositor =
            wl_registry_bind(registry, name, &wl_compositor_interface,
                             _linearwin.wl.compositorVersion);
    }
    else if (strcmp(interface, "wl_subcompositor") == 0)
    {
        _linearwin.wl.subcompositor =
            wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);
    }
    else if (strcmp(interface, "wl_shm") == 0)
    {
        _linearwin.wl.shm =
            wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }
    else if (strcmp(interface, "wl_output") == 0)
    {
        _linearwinAddOutputWayland(name, version);
    }
    else if (strcmp(interface, "wl_seat") == 0)
    {
        if (!_linearwin.wl.seat)
        {
            _linearwin.wl.seatVersion = _linearwin_min(4, version);
            _linearwin.wl.seat =
                wl_registry_bind(registry, name, &wl_seat_interface,
                                 _linearwin.wl.seatVersion);
            _linearwinAddSeatListenerWayland(_linearwin.wl.seat);
        }
    }
    else if (strcmp(interface, "wl_data_device_manager") == 0)
    {
        if (!_linearwin.wl.dataDeviceManager)
        {
            _linearwin.wl.dataDeviceManager =
                wl_registry_bind(registry, name,
                                 &wl_data_device_manager_interface, 1);
        }
    }
    else if (strcmp(interface, "xdg_wm_base") == 0)
    {
        _linearwin.wl.wmBase =
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(_linearwin.wl.wmBase, &wmBaseListener, NULL);
    }
    else if (strcmp(interface, "zxdg_decoration_manager_v1") == 0)
    {
        _linearwin.wl.decorationManager =
            wl_registry_bind(registry, name,
                             &zxdg_decoration_manager_v1_interface,
                             1);
    }
    else if (strcmp(interface, "wp_viewporter") == 0)
    {
        _linearwin.wl.viewporter =
            wl_registry_bind(registry, name, &wp_viewporter_interface, 1);
    }
    else if (strcmp(interface, "zwp_relative_pointer_manager_v1") == 0)
    {
        _linearwin.wl.relativePointerManager =
            wl_registry_bind(registry, name,
                             &zwp_relative_pointer_manager_v1_interface,
                             1);
    }
    else if (strcmp(interface, "zwp_pointer_constraints_v1") == 0)
    {
        _linearwin.wl.pointerConstraints =
            wl_registry_bind(registry, name,
                             &zwp_pointer_constraints_v1_interface,
                             1);
    }
    else if (strcmp(interface, "zwp_idle_inhibit_manager_v1") == 0)
    {
        _linearwin.wl.idleInhibitManager =
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

    for (i = 0; i < _linearwin.monitorCount; ++i)
    {
        _LINEARWINmonitor* monitor = _linearwin.monitors[i];
        if (monitor->wl.name == name)
        {
            _linearwinInputMonitor(monitor, LINEARWIN_DISCONNECTED, 0);
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

    memset(_linearwin.wl.keycodes, -1, sizeof(_linearwin.wl.keycodes));
    memset(_linearwin.wl.scancodes, -1, sizeof(_linearwin.wl.scancodes));

    _linearwin.wl.keycodes[KEY_GRAVE]      = LINEARWIN_KEY_GRAVE_ACCENT;
    _linearwin.wl.keycodes[KEY_1]          = LINEARWIN_KEY_1;
    _linearwin.wl.keycodes[KEY_2]          = LINEARWIN_KEY_2;
    _linearwin.wl.keycodes[KEY_3]          = LINEARWIN_KEY_3;
    _linearwin.wl.keycodes[KEY_4]          = LINEARWIN_KEY_4;
    _linearwin.wl.keycodes[KEY_5]          = LINEARWIN_KEY_5;
    _linearwin.wl.keycodes[KEY_6]          = LINEARWIN_KEY_6;
    _linearwin.wl.keycodes[KEY_7]          = LINEARWIN_KEY_7;
    _linearwin.wl.keycodes[KEY_8]          = LINEARWIN_KEY_8;
    _linearwin.wl.keycodes[KEY_9]          = LINEARWIN_KEY_9;
    _linearwin.wl.keycodes[KEY_0]          = LINEARWIN_KEY_0;
    _linearwin.wl.keycodes[KEY_SPACE]      = LINEARWIN_KEY_SPACE;
    _linearwin.wl.keycodes[KEY_MINUS]      = LINEARWIN_KEY_MINUS;
    _linearwin.wl.keycodes[KEY_EQUAL]      = LINEARWIN_KEY_EQUAL;
    _linearwin.wl.keycodes[KEY_Q]          = LINEARWIN_KEY_Q;
    _linearwin.wl.keycodes[KEY_W]          = LINEARWIN_KEY_W;
    _linearwin.wl.keycodes[KEY_E]          = LINEARWIN_KEY_E;
    _linearwin.wl.keycodes[KEY_R]          = LINEARWIN_KEY_R;
    _linearwin.wl.keycodes[KEY_T]          = LINEARWIN_KEY_T;
    _linearwin.wl.keycodes[KEY_Y]          = LINEARWIN_KEY_Y;
    _linearwin.wl.keycodes[KEY_U]          = LINEARWIN_KEY_U;
    _linearwin.wl.keycodes[KEY_I]          = LINEARWIN_KEY_I;
    _linearwin.wl.keycodes[KEY_O]          = LINEARWIN_KEY_O;
    _linearwin.wl.keycodes[KEY_P]          = LINEARWIN_KEY_P;
    _linearwin.wl.keycodes[KEY_LEFTBRACE]  = LINEARWIN_KEY_LEFT_BRACKET;
    _linearwin.wl.keycodes[KEY_RIGHTBRACE] = LINEARWIN_KEY_RIGHT_BRACKET;
    _linearwin.wl.keycodes[KEY_A]          = LINEARWIN_KEY_A;
    _linearwin.wl.keycodes[KEY_S]          = LINEARWIN_KEY_S;
    _linearwin.wl.keycodes[KEY_D]          = LINEARWIN_KEY_D;
    _linearwin.wl.keycodes[KEY_F]          = LINEARWIN_KEY_F;
    _linearwin.wl.keycodes[KEY_G]          = LINEARWIN_KEY_G;
    _linearwin.wl.keycodes[KEY_H]          = LINEARWIN_KEY_H;
    _linearwin.wl.keycodes[KEY_J]          = LINEARWIN_KEY_J;
    _linearwin.wl.keycodes[KEY_K]          = LINEARWIN_KEY_K;
    _linearwin.wl.keycodes[KEY_L]          = LINEARWIN_KEY_L;
    _linearwin.wl.keycodes[KEY_SEMICOLON]  = LINEARWIN_KEY_SEMICOLON;
    _linearwin.wl.keycodes[KEY_APOSTROPHE] = LINEARWIN_KEY_APOSTROPHE;
    _linearwin.wl.keycodes[KEY_Z]          = LINEARWIN_KEY_Z;
    _linearwin.wl.keycodes[KEY_X]          = LINEARWIN_KEY_X;
    _linearwin.wl.keycodes[KEY_C]          = LINEARWIN_KEY_C;
    _linearwin.wl.keycodes[KEY_V]          = LINEARWIN_KEY_V;
    _linearwin.wl.keycodes[KEY_B]          = LINEARWIN_KEY_B;
    _linearwin.wl.keycodes[KEY_N]          = LINEARWIN_KEY_N;
    _linearwin.wl.keycodes[KEY_M]          = LINEARWIN_KEY_M;
    _linearwin.wl.keycodes[KEY_COMMA]      = LINEARWIN_KEY_COMMA;
    _linearwin.wl.keycodes[KEY_DOT]        = LINEARWIN_KEY_PERIOD;
    _linearwin.wl.keycodes[KEY_SLASH]      = LINEARWIN_KEY_SLASH;
    _linearwin.wl.keycodes[KEY_BACKSLASH]  = LINEARWIN_KEY_BACKSLASH;
    _linearwin.wl.keycodes[KEY_ESC]        = LINEARWIN_KEY_ESCAPE;
    _linearwin.wl.keycodes[KEY_TAB]        = LINEARWIN_KEY_TAB;
    _linearwin.wl.keycodes[KEY_LEFTSHIFT]  = LINEARWIN_KEY_LEFT_SHIFT;
    _linearwin.wl.keycodes[KEY_RIGHTSHIFT] = LINEARWIN_KEY_RIGHT_SHIFT;
    _linearwin.wl.keycodes[KEY_LEFTCTRL]   = LINEARWIN_KEY_LEFT_CONTROL;
    _linearwin.wl.keycodes[KEY_RIGHTCTRL]  = LINEARWIN_KEY_RIGHT_CONTROL;
    _linearwin.wl.keycodes[KEY_LEFTALT]    = LINEARWIN_KEY_LEFT_ALT;
    _linearwin.wl.keycodes[KEY_RIGHTALT]   = LINEARWIN_KEY_RIGHT_ALT;
    _linearwin.wl.keycodes[KEY_LEFTMETA]   = LINEARWIN_KEY_LEFT_SUPER;
    _linearwin.wl.keycodes[KEY_RIGHTMETA]  = LINEARWIN_KEY_RIGHT_SUPER;
    _linearwin.wl.keycodes[KEY_COMPOSE]    = LINEARWIN_KEY_MENU;
    _linearwin.wl.keycodes[KEY_NUMLOCK]    = LINEARWIN_KEY_NUM_LOCK;
    _linearwin.wl.keycodes[KEY_CAPSLOCK]   = LINEARWIN_KEY_CAPS_LOCK;
    _linearwin.wl.keycodes[KEY_PRINT]      = LINEARWIN_KEY_PRINT_SCREEN;
    _linearwin.wl.keycodes[KEY_SCROLLLOCK] = LINEARWIN_KEY_SCROLL_LOCK;
    _linearwin.wl.keycodes[KEY_PAUSE]      = LINEARWIN_KEY_PAUSE;
    _linearwin.wl.keycodes[KEY_DELETE]     = LINEARWIN_KEY_DELETE;
    _linearwin.wl.keycodes[KEY_BACKSPACE]  = LINEARWIN_KEY_BACKSPACE;
    _linearwin.wl.keycodes[KEY_ENTER]      = LINEARWIN_KEY_ENTER;
    _linearwin.wl.keycodes[KEY_HOME]       = LINEARWIN_KEY_HOME;
    _linearwin.wl.keycodes[KEY_END]        = LINEARWIN_KEY_END;
    _linearwin.wl.keycodes[KEY_PAGEUP]     = LINEARWIN_KEY_PAGE_UP;
    _linearwin.wl.keycodes[KEY_PAGEDOWN]   = LINEARWIN_KEY_PAGE_DOWN;
    _linearwin.wl.keycodes[KEY_INSERT]     = LINEARWIN_KEY_INSERT;
    _linearwin.wl.keycodes[KEY_LEFT]       = LINEARWIN_KEY_LEFT;
    _linearwin.wl.keycodes[KEY_RIGHT]      = LINEARWIN_KEY_RIGHT;
    _linearwin.wl.keycodes[KEY_DOWN]       = LINEARWIN_KEY_DOWN;
    _linearwin.wl.keycodes[KEY_UP]         = LINEARWIN_KEY_UP;
    _linearwin.wl.keycodes[KEY_F1]         = LINEARWIN_KEY_F1;
    _linearwin.wl.keycodes[KEY_F2]         = LINEARWIN_KEY_F2;
    _linearwin.wl.keycodes[KEY_F3]         = LINEARWIN_KEY_F3;
    _linearwin.wl.keycodes[KEY_F4]         = LINEARWIN_KEY_F4;
    _linearwin.wl.keycodes[KEY_F5]         = LINEARWIN_KEY_F5;
    _linearwin.wl.keycodes[KEY_F6]         = LINEARWIN_KEY_F6;
    _linearwin.wl.keycodes[KEY_F7]         = LINEARWIN_KEY_F7;
    _linearwin.wl.keycodes[KEY_F8]         = LINEARWIN_KEY_F8;
    _linearwin.wl.keycodes[KEY_F9]         = LINEARWIN_KEY_F9;
    _linearwin.wl.keycodes[KEY_F10]        = LINEARWIN_KEY_F10;
    _linearwin.wl.keycodes[KEY_F11]        = LINEARWIN_KEY_F11;
    _linearwin.wl.keycodes[KEY_F12]        = LINEARWIN_KEY_F12;
    _linearwin.wl.keycodes[KEY_F13]        = LINEARWIN_KEY_F13;
    _linearwin.wl.keycodes[KEY_F14]        = LINEARWIN_KEY_F14;
    _linearwin.wl.keycodes[KEY_F15]        = LINEARWIN_KEY_F15;
    _linearwin.wl.keycodes[KEY_F16]        = LINEARWIN_KEY_F16;
    _linearwin.wl.keycodes[KEY_F17]        = LINEARWIN_KEY_F17;
    _linearwin.wl.keycodes[KEY_F18]        = LINEARWIN_KEY_F18;
    _linearwin.wl.keycodes[KEY_F19]        = LINEARWIN_KEY_F19;
    _linearwin.wl.keycodes[KEY_F20]        = LINEARWIN_KEY_F20;
    _linearwin.wl.keycodes[KEY_F21]        = LINEARWIN_KEY_F21;
    _linearwin.wl.keycodes[KEY_F22]        = LINEARWIN_KEY_F22;
    _linearwin.wl.keycodes[KEY_F23]        = LINEARWIN_KEY_F23;
    _linearwin.wl.keycodes[KEY_F24]        = LINEARWIN_KEY_F24;
    _linearwin.wl.keycodes[KEY_KPSLASH]    = LINEARWIN_KEY_KP_DIVIDE;
    _linearwin.wl.keycodes[KEY_KPASTERISK] = LINEARWIN_KEY_KP_MULTIPLY;
    _linearwin.wl.keycodes[KEY_KPMINUS]    = LINEARWIN_KEY_KP_SUBTRACT;
    _linearwin.wl.keycodes[KEY_KPPLUS]     = LINEARWIN_KEY_KP_ADD;
    _linearwin.wl.keycodes[KEY_KP0]        = LINEARWIN_KEY_KP_0;
    _linearwin.wl.keycodes[KEY_KP1]        = LINEARWIN_KEY_KP_1;
    _linearwin.wl.keycodes[KEY_KP2]        = LINEARWIN_KEY_KP_2;
    _linearwin.wl.keycodes[KEY_KP3]        = LINEARWIN_KEY_KP_3;
    _linearwin.wl.keycodes[KEY_KP4]        = LINEARWIN_KEY_KP_4;
    _linearwin.wl.keycodes[KEY_KP5]        = LINEARWIN_KEY_KP_5;
    _linearwin.wl.keycodes[KEY_KP6]        = LINEARWIN_KEY_KP_6;
    _linearwin.wl.keycodes[KEY_KP7]        = LINEARWIN_KEY_KP_7;
    _linearwin.wl.keycodes[KEY_KP8]        = LINEARWIN_KEY_KP_8;
    _linearwin.wl.keycodes[KEY_KP9]        = LINEARWIN_KEY_KP_9;
    _linearwin.wl.keycodes[KEY_KPDOT]      = LINEARWIN_KEY_KP_DECIMAL;
    _linearwin.wl.keycodes[KEY_KPEQUAL]    = LINEARWIN_KEY_KP_EQUAL;
    _linearwin.wl.keycodes[KEY_KPENTER]    = LINEARWIN_KEY_KP_ENTER;
    _linearwin.wl.keycodes[KEY_102ND]      = LINEARWIN_KEY_WORLD_2;

    for (scancode = 0;  scancode < 256;  scancode++)
    {
        if (_linearwin.wl.keycodes[scancode] > 0)
            _linearwin.wl.scancodes[_linearwin.wl.keycodes[scancode]] = scancode;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _linearwinPlatformInit(void)
{
    const char* cursorTheme;
    const char* cursorSizeStr;
    char* cursorSizeEnd;
    long cursorSizeLong;
    int cursorSize;

    // These must be set before any failure checks
    _linearwin.wl.timerfd = -1;
    _linearwin.wl.cursorTimerfd = -1;

    _linearwin.wl.cursor.handle = _linearwin_dlopen("libwayland-cursor.so.0");
    if (!_linearwin.wl.cursor.handle)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to load libwayland-cursor");
        return LINEARWIN_FALSE;
    }

    _linearwin.wl.cursor.theme_load = (PFN_wl_cursor_theme_load)
        _linearwin_dlsym(_linearwin.wl.cursor.handle, "wl_cursor_theme_load");
    _linearwin.wl.cursor.theme_destroy = (PFN_wl_cursor_theme_destroy)
        _linearwin_dlsym(_linearwin.wl.cursor.handle, "wl_cursor_theme_destroy");
    _linearwin.wl.cursor.theme_get_cursor = (PFN_wl_cursor_theme_get_cursor)
        _linearwin_dlsym(_linearwin.wl.cursor.handle, "wl_cursor_theme_get_cursor");
    _linearwin.wl.cursor.image_get_buffer = (PFN_wl_cursor_image_get_buffer)
        _linearwin_dlsym(_linearwin.wl.cursor.handle, "wl_cursor_image_get_buffer");

    _linearwin.wl.egl.handle = _linearwin_dlopen("libwayland-egl.so.1");
    if (!_linearwin.wl.egl.handle)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to load libwayland-egl");
        return LINEARWIN_FALSE;
    }

    _linearwin.wl.egl.window_create = (PFN_wl_egl_window_create)
        _linearwin_dlsym(_linearwin.wl.egl.handle, "wl_egl_window_create");
    _linearwin.wl.egl.window_destroy = (PFN_wl_egl_window_destroy)
        _linearwin_dlsym(_linearwin.wl.egl.handle, "wl_egl_window_destroy");
    _linearwin.wl.egl.window_resize = (PFN_wl_egl_window_resize)
        _linearwin_dlsym(_linearwin.wl.egl.handle, "wl_egl_window_resize");

    _linearwin.wl.xkb.handle = _linearwin_dlopen("libxkbcommon.so.0");
    if (!_linearwin.wl.xkb.handle)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to load libxkbcommon");
        return LINEARWIN_FALSE;
    }

    _linearwin.wl.xkb.context_new = (PFN_xkb_context_new)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_context_new");
    _linearwin.wl.xkb.context_unref = (PFN_xkb_context_unref)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_context_unref");
    _linearwin.wl.xkb.keymap_new_from_string = (PFN_xkb_keymap_new_from_string)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_keymap_new_from_string");
    _linearwin.wl.xkb.keymap_unref = (PFN_xkb_keymap_unref)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_keymap_unref");
    _linearwin.wl.xkb.keymap_mod_get_index = (PFN_xkb_keymap_mod_get_index)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_keymap_mod_get_index");
    _linearwin.wl.xkb.keymap_key_repeats = (PFN_xkb_keymap_key_repeats)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_keymap_key_repeats");
    _linearwin.wl.xkb.keymap_key_get_syms_by_level = (PFN_xkb_keymap_key_get_syms_by_level)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_keymap_key_get_syms_by_level");
    _linearwin.wl.xkb.state_new = (PFN_xkb_state_new)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_state_new");
    _linearwin.wl.xkb.state_unref = (PFN_xkb_state_unref)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_state_unref");
    _linearwin.wl.xkb.state_key_get_syms = (PFN_xkb_state_key_get_syms)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_state_key_get_syms");
    _linearwin.wl.xkb.state_update_mask = (PFN_xkb_state_update_mask)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_state_update_mask");
    _linearwin.wl.xkb.state_key_get_layout = (PFN_xkb_state_key_get_layout)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_state_key_get_layout");
    _linearwin.wl.xkb.state_mod_index_is_active = (PFN_xkb_state_mod_index_is_active)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_state_mod_index_is_active");

    _linearwin.wl.xkb.compose_table_new_from_locale = (PFN_xkb_compose_table_new_from_locale)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_compose_table_new_from_locale");
    _linearwin.wl.xkb.compose_table_unref = (PFN_xkb_compose_table_unref)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_compose_table_unref");
    _linearwin.wl.xkb.compose_state_new = (PFN_xkb_compose_state_new)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_compose_state_new");
    _linearwin.wl.xkb.compose_state_unref = (PFN_xkb_compose_state_unref)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_compose_state_unref");
    _linearwin.wl.xkb.compose_state_feed = (PFN_xkb_compose_state_feed)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_compose_state_feed");
    _linearwin.wl.xkb.compose_state_get_status = (PFN_xkb_compose_state_get_status)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_compose_state_get_status");
    _linearwin.wl.xkb.compose_state_get_one_sym = (PFN_xkb_compose_state_get_one_sym)
        _linearwin_dlsym(_linearwin.wl.xkb.handle, "xkb_compose_state_get_one_sym");

    _linearwin.wl.display = wl_display_connect(NULL);
    if (!_linearwin.wl.display)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to connect to display");
        return LINEARWIN_FALSE;
    }

    _linearwin.wl.registry = wl_display_get_registry(_linearwin.wl.display);
    wl_registry_add_listener(_linearwin.wl.registry, &registryListener, NULL);

    createKeyTables();

    _linearwin.wl.xkb.context = xkb_context_new(0);
    if (!_linearwin.wl.xkb.context)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to initialize xkb context");
        return LINEARWIN_FALSE;
    }

    // Sync so we got all registry objects
    wl_display_roundtrip(_linearwin.wl.display);

    // Sync so we got all initial output events
    wl_display_roundtrip(_linearwin.wl.display);

#ifdef __linux__
    if (!_linearwinInitJoysticksLinux())
        return LINEARWIN_FALSE;
#endif

    _linearwinInitTimerPOSIX();

#ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
    if (_linearwin.wl.seatVersion >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
        _linearwin.wl.timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
#endif

    if (!_linearwin.wl.wmBase)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to find xdg-shell in your compositor");
        return LINEARWIN_FALSE;
    }

    if (_linearwin.wl.pointer && _linearwin.wl.shm)
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
        _linearwin.wl.cursorTheme =
            wl_cursor_theme_load(cursorTheme, cursorSize, _linearwin.wl.shm);
        if (!_linearwin.wl.cursorTheme)
        {
            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "Wayland: Failed to load default cursor theme");
            return LINEARWIN_FALSE;
        }
        // If this happens to be NULL, we just fallback to the scale=1 version.
        _linearwin.wl.cursorThemeHiDPI =
            wl_cursor_theme_load(cursorTheme, 2 * cursorSize, _linearwin.wl.shm);
        _linearwin.wl.cursorSurface =
            wl_compositor_create_surface(_linearwin.wl.compositor);
        _linearwin.wl.cursorTimerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    }

    if (_linearwin.wl.seat && _linearwin.wl.dataDeviceManager)
    {
        _linearwin.wl.dataDevice =
            wl_data_device_manager_get_data_device(_linearwin.wl.dataDeviceManager,
                                                   _linearwin.wl.seat);
        _linearwinAddDataDeviceListenerWayland(_linearwin.wl.dataDevice);
    }

    return LINEARWIN_TRUE;
}

void _linearwinPlatformTerminate(void)
{
#ifdef __linux__
    _linearwinTerminateJoysticksLinux();
#endif
    _linearwinTerminateEGL();
    _linearwinTerminateOSMesa();

    if (_linearwin.wl.egl.handle)
    {
        _linearwin_dlclose(_linearwin.wl.egl.handle);
        _linearwin.wl.egl.handle = NULL;
    }

    if (_linearwin.wl.xkb.composeState)
        xkb_compose_state_unref(_linearwin.wl.xkb.composeState);
    if (_linearwin.wl.xkb.keymap)
        xkb_keymap_unref(_linearwin.wl.xkb.keymap);
    if (_linearwin.wl.xkb.state)
        xkb_state_unref(_linearwin.wl.xkb.state);
    if (_linearwin.wl.xkb.context)
        xkb_context_unref(_linearwin.wl.xkb.context);
    if (_linearwin.wl.xkb.handle)
    {
        _linearwin_dlclose(_linearwin.wl.xkb.handle);
        _linearwin.wl.xkb.handle = NULL;
    }

    if (_linearwin.wl.cursorTheme)
        wl_cursor_theme_destroy(_linearwin.wl.cursorTheme);
    if (_linearwin.wl.cursorThemeHiDPI)
        wl_cursor_theme_destroy(_linearwin.wl.cursorThemeHiDPI);
    if (_linearwin.wl.cursor.handle)
    {
        _linearwin_dlclose(_linearwin.wl.cursor.handle);
        _linearwin.wl.cursor.handle = NULL;
    }

    for (unsigned int i = 0; i < _linearwin.wl.offerCount; i++)
        wl_data_offer_destroy(_linearwin.wl.offers[i].offer);

    free(_linearwin.wl.offers);

    if (_linearwin.wl.cursorSurface)
        wl_surface_destroy(_linearwin.wl.cursorSurface);
    if (_linearwin.wl.subcompositor)
        wl_subcompositor_destroy(_linearwin.wl.subcompositor);
    if (_linearwin.wl.compositor)
        wl_compositor_destroy(_linearwin.wl.compositor);
    if (_linearwin.wl.shm)
        wl_shm_destroy(_linearwin.wl.shm);
    if (_linearwin.wl.viewporter)
        wp_viewporter_destroy(_linearwin.wl.viewporter);
    if (_linearwin.wl.decorationManager)
        zxdg_decoration_manager_v1_destroy(_linearwin.wl.decorationManager);
    if (_linearwin.wl.wmBase)
        xdg_wm_base_destroy(_linearwin.wl.wmBase);
    if (_linearwin.wl.selectionOffer)
        wl_data_offer_destroy(_linearwin.wl.selectionOffer);
    if (_linearwin.wl.dragOffer)
        wl_data_offer_destroy(_linearwin.wl.dragOffer);
    if (_linearwin.wl.selectionSource)
        wl_data_source_destroy(_linearwin.wl.selectionSource);
    if (_linearwin.wl.dataDevice)
        wl_data_device_destroy(_linearwin.wl.dataDevice);
    if (_linearwin.wl.dataDeviceManager)
        wl_data_device_manager_destroy(_linearwin.wl.dataDeviceManager);
    if (_linearwin.wl.pointer)
        wl_pointer_destroy(_linearwin.wl.pointer);
    if (_linearwin.wl.keyboard)
        wl_keyboard_destroy(_linearwin.wl.keyboard);
    if (_linearwin.wl.seat)
        wl_seat_destroy(_linearwin.wl.seat);
    if (_linearwin.wl.relativePointerManager)
        zwp_relative_pointer_manager_v1_destroy(_linearwin.wl.relativePointerManager);
    if (_linearwin.wl.pointerConstraints)
        zwp_pointer_constraints_v1_destroy(_linearwin.wl.pointerConstraints);
    if (_linearwin.wl.idleInhibitManager)
        zwp_idle_inhibit_manager_v1_destroy(_linearwin.wl.idleInhibitManager);
    if (_linearwin.wl.registry)
        wl_registry_destroy(_linearwin.wl.registry);
    if (_linearwin.wl.display)
    {
        wl_display_flush(_linearwin.wl.display);
        wl_display_disconnect(_linearwin.wl.display);
    }

    if (_linearwin.wl.timerfd >= 0)
        close(_linearwin.wl.timerfd);
    if (_linearwin.wl.cursorTimerfd >= 0)
        close(_linearwin.wl.cursorTimerfd);

    free(_linearwin.wl.clipboardString);
}

const char* _linearwinPlatformGetVersionString(void)
{
    return _LINEARWIN_VERSION_NUMBER " Wayland EGL OSMesa"
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
        " clock_gettime"
#else
        " gettimeofday"
#endif
        " evdev"
#if defined(_LINEARWIN_BUILD_DLL)
        " shared"
#endif
        ;
}
