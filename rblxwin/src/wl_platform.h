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

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <dlfcn.h>

typedef VkFlags VkWaylandSurfaceCreateFlagsKHR;

typedef struct VkWaylandSurfaceCreateInfoKHR
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkWaylandSurfaceCreateFlagsKHR  flags;
    struct wl_display*              display;
    struct wl_surface*              surface;
} VkWaylandSurfaceCreateInfoKHR;

typedef VkResult (APIENTRY *PFN_vkCreateWaylandSurfaceKHR)(VkInstance,const VkWaylandSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
typedef VkBool32 (APIENTRY *PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)(VkPhysicalDevice,uint32_t,struct wl_display*);

#include "posix_thread.h"
#include "posix_time.h"
#ifdef __linux__
#include "linux_joystick.h"
#else
#include "null_joystick.h"
#endif
#include "xkb_unicode.h"
#include "egl_context.h"
#include "osmesa_context.h"

#include "wayland-xdg-shell-client-protocol.h"
#include "wayland-xdg-decoration-client-protocol.h"
#include "wayland-viewporter-client-protocol.h"
#include "wayland-relative-pointer-unstable-v1-client-protocol.h"
#include "wayland-pointer-constraints-unstable-v1-client-protocol.h"
#include "wayland-idle-inhibit-unstable-v1-client-protocol.h"

#define _rblxwin_dlopen(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL)
#define _rblxwin_dlclose(handle) dlclose(handle)
#define _rblxwin_dlsym(handle, name) dlsym(handle, name)

#define _RBLXWIN_EGL_NATIVE_WINDOW         ((EGLNativeWindowType) window->wl.egl.window)
#define _RBLXWIN_EGL_NATIVE_DISPLAY        ((EGLNativeDisplayType) _rblxwin.wl.display)

#define _RBLXWIN_PLATFORM_WINDOW_STATE         _RBLXWINwindowWayland  wl
#define _RBLXWIN_PLATFORM_LIBRARY_WINDOW_STATE _RBLXWINlibraryWayland wl
#define _RBLXWIN_PLATFORM_MONITOR_STATE        _RBLXWINmonitorWayland wl
#define _RBLXWIN_PLATFORM_CURSOR_STATE         _RBLXWINcursorWayland  wl

#define _RBLXWIN_PLATFORM_CONTEXT_STATE         struct { int dummyContext; }
#define _RBLXWIN_PLATFORM_LIBRARY_CONTEXT_STATE struct { int dummyLibraryContext; }

struct wl_cursor_image {
    uint32_t width;
    uint32_t height;
    uint32_t hotspot_x;
    uint32_t hotspot_y;
    uint32_t delay;
};
struct wl_cursor {
    unsigned int image_count;
    struct wl_cursor_image** images;
    char* name;
};
typedef struct wl_cursor_theme* (* PFN_wl_cursor_theme_load)(const char*, int, struct wl_shm*);
typedef void (* PFN_wl_cursor_theme_destroy)(struct wl_cursor_theme*);
typedef struct wl_cursor* (* PFN_wl_cursor_theme_get_cursor)(struct wl_cursor_theme*, const char*);
typedef struct wl_buffer* (* PFN_wl_cursor_image_get_buffer)(struct wl_cursor_image*);
#define wl_cursor_theme_load _rblxwin.wl.cursor.theme_load
#define wl_cursor_theme_destroy _rblxwin.wl.cursor.theme_destroy
#define wl_cursor_theme_get_cursor _rblxwin.wl.cursor.theme_get_cursor
#define wl_cursor_image_get_buffer _rblxwin.wl.cursor.image_get_buffer

typedef struct wl_egl_window* (* PFN_wl_egl_window_create)(struct wl_surface*, int, int);
typedef void (* PFN_wl_egl_window_destroy)(struct wl_egl_window*);
typedef void (* PFN_wl_egl_window_resize)(struct wl_egl_window*, int, int, int, int);
#define wl_egl_window_create _rblxwin.wl.egl.window_create
#define wl_egl_window_destroy _rblxwin.wl.egl.window_destroy
#define wl_egl_window_resize _rblxwin.wl.egl.window_resize

typedef struct xkb_context* (* PFN_xkb_context_new)(enum xkb_context_flags);
typedef void (* PFN_xkb_context_unref)(struct xkb_context*);
typedef struct xkb_keymap* (* PFN_xkb_keymap_new_from_string)(struct xkb_context*, const char*, enum xkb_keymap_format, enum xkb_keymap_compile_flags);
typedef void (* PFN_xkb_keymap_unref)(struct xkb_keymap*);
typedef xkb_mod_index_t (* PFN_xkb_keymap_mod_get_index)(struct xkb_keymap*, const char*);
typedef int (* PFN_xkb_keymap_key_repeats)(struct xkb_keymap*, xkb_keycode_t);
typedef int (* PFN_xkb_keymap_key_get_syms_by_level)(struct xkb_keymap*,xkb_keycode_t,xkb_layout_index_t,xkb_level_index_t,const xkb_keysym_t**);
typedef struct xkb_state* (* PFN_xkb_state_new)(struct xkb_keymap*);
typedef void (* PFN_xkb_state_unref)(struct xkb_state*);
typedef int (* PFN_xkb_state_key_get_syms)(struct xkb_state*, xkb_keycode_t, const xkb_keysym_t**);
typedef enum xkb_state_component (* PFN_xkb_state_update_mask)(struct xkb_state*, xkb_mod_mask_t, xkb_mod_mask_t, xkb_mod_mask_t, xkb_layout_index_t, xkb_layout_index_t, xkb_layout_index_t);
typedef xkb_layout_index_t (* PFN_xkb_state_key_get_layout)(struct xkb_state*,xkb_keycode_t);
typedef int (* PFN_xkb_state_mod_index_is_active)(struct xkb_state*,xkb_mod_index_t,enum xkb_state_component);
#define xkb_context_new _rblxwin.wl.xkb.context_new
#define xkb_context_unref _rblxwin.wl.xkb.context_unref
#define xkb_keymap_new_from_string _rblxwin.wl.xkb.keymap_new_from_string
#define xkb_keymap_unref _rblxwin.wl.xkb.keymap_unref
#define xkb_keymap_mod_get_index _rblxwin.wl.xkb.keymap_mod_get_index
#define xkb_keymap_key_repeats _rblxwin.wl.xkb.keymap_key_repeats
#define xkb_keymap_key_get_syms_by_level _rblxwin.wl.xkb.keymap_key_get_syms_by_level
#define xkb_state_new _rblxwin.wl.xkb.state_new
#define xkb_state_unref _rblxwin.wl.xkb.state_unref
#define xkb_state_key_get_syms _rblxwin.wl.xkb.state_key_get_syms
#define xkb_state_update_mask _rblxwin.wl.xkb.state_update_mask
#define xkb_state_key_get_layout _rblxwin.wl.xkb.state_key_get_layout
#define xkb_state_mod_index_is_active _rblxwin.wl.xkb.state_mod_index_is_active

typedef struct xkb_compose_table* (* PFN_xkb_compose_table_new_from_locale)(struct xkb_context*, const char*, enum xkb_compose_compile_flags);
typedef void (* PFN_xkb_compose_table_unref)(struct xkb_compose_table*);
typedef struct xkb_compose_state* (* PFN_xkb_compose_state_new)(struct xkb_compose_table*, enum xkb_compose_state_flags);
typedef void (* PFN_xkb_compose_state_unref)(struct xkb_compose_state*);
typedef enum xkb_compose_feed_result (* PFN_xkb_compose_state_feed)(struct xkb_compose_state*, xkb_keysym_t);
typedef enum xkb_compose_status (* PFN_xkb_compose_state_get_status)(struct xkb_compose_state*);
typedef xkb_keysym_t (* PFN_xkb_compose_state_get_one_sym)(struct xkb_compose_state*);
#define xkb_compose_table_new_from_locale _rblxwin.wl.xkb.compose_table_new_from_locale
#define xkb_compose_table_unref _rblxwin.wl.xkb.compose_table_unref
#define xkb_compose_state_new _rblxwin.wl.xkb.compose_state_new
#define xkb_compose_state_unref _rblxwin.wl.xkb.compose_state_unref
#define xkb_compose_state_feed _rblxwin.wl.xkb.compose_state_feed
#define xkb_compose_state_get_status _rblxwin.wl.xkb.compose_state_get_status
#define xkb_compose_state_get_one_sym _rblxwin.wl.xkb.compose_state_get_one_sym

typedef enum _RBLXWINdecorationSideWayland
{
    mainWindow,
    topDecoration,
    leftDecoration,
    rightDecoration,
    bottomDecoration,
} _RBLXWINdecorationSideWayland;

typedef struct _RBLXWINdecorationWayland
{
    struct wl_surface*          surface;
    struct wl_subsurface*       subsurface;
    struct wp_viewport*         viewport;
} _RBLXWINdecorationWayland;

typedef struct _RBLXWINofferWayland
{
    struct wl_data_offer*       offer;
    RBLXWINbool                    text_plain_utf8;
    RBLXWINbool                    text_uri_list;
} _RBLXWINofferWayland;

// Wayland-specific per-window data
//
typedef struct _RBLXWINwindowWayland
{
    int                         width, height;
    RBLXWINbool                    visible;
    RBLXWINbool                    maximized;
    RBLXWINbool                    activated;
    RBLXWINbool                    fullscreen;
    RBLXWINbool                    hovered;
    RBLXWINbool                    transparent;
    struct wl_surface*          surface;
    struct wl_callback*         callback;

    struct {
        struct wl_egl_window*   window;
    } egl;

    struct {
        int                     width, height;
        RBLXWINbool                maximized;
        RBLXWINbool                iconified;
        RBLXWINbool                activated;
        RBLXWINbool                fullscreen;
    } pending;

    struct {
        struct xdg_surface*     surface;
        struct xdg_toplevel*    toplevel;
        struct zxdg_toplevel_decoration_v1* decoration;
        uint32_t                decorationMode;
    } xdg;

    _RBLXWINcursor*                currentCursor;
    double                      cursorPosX, cursorPosY;

    char*                       title;

    // We need to track the monitors the window spans on to calculate the
    // optimal scaling factor.
    int                         scale;
    _RBLXWINmonitor**              monitors;
    int                         monitorsCount;
    int                         monitorsSize;

    struct {
        struct zwp_relative_pointer_v1*    relativePointer;
        struct zwp_locked_pointer_v1*      lockedPointer;
    } pointerLock;

    struct zwp_idle_inhibitor_v1*          idleInhibitor;

    struct {
        struct wl_buffer*                  buffer;
        _RBLXWINdecorationWayland             top, left, right, bottom;
        _RBLXWINdecorationSideWayland         focus;
    } decorations;
} _RBLXWINwindowWayland;

// Wayland-specific global data
//
typedef struct _RBLXWINlibraryWayland
{
    struct wl_display*          display;
    struct wl_registry*         registry;
    struct wl_compositor*       compositor;
    struct wl_subcompositor*    subcompositor;
    struct wl_shm*              shm;
    struct wl_seat*             seat;
    struct wl_pointer*          pointer;
    struct wl_keyboard*         keyboard;
    struct wl_data_device_manager*          dataDeviceManager;
    struct wl_data_device*      dataDevice;
    struct xdg_wm_base*         wmBase;
    struct zxdg_decoration_manager_v1*      decorationManager;
    struct wp_viewporter*       viewporter;
    struct zwp_relative_pointer_manager_v1* relativePointerManager;
    struct zwp_pointer_constraints_v1*      pointerConstraints;
    struct zwp_idle_inhibit_manager_v1*     idleInhibitManager;

    _RBLXWINofferWayland*          offers;
    unsigned int                offerCount;

    struct wl_data_offer*       selectionOffer;
    struct wl_data_source*      selectionSource;

    struct wl_data_offer*       dragOffer;
    _RBLXWINwindow*                dragFocus;
    uint32_t                    dragSerial;

    int                         compositorVersion;
    int                         seatVersion;

    struct wl_cursor_theme*     cursorTheme;
    struct wl_cursor_theme*     cursorThemeHiDPI;
    struct wl_surface*          cursorSurface;
    const char*                 cursorPreviousName;
    int                         cursorTimerfd;
    uint32_t                    serial;
    uint32_t                    pointerEnterSerial;

    int32_t                     keyboardRepeatRate;
    int32_t                     keyboardRepeatDelay;
    int                         keyboardLastKey;
    int                         keyboardLastScancode;
    char*                       clipboardString;
    int                         timerfd;
    short int                   keycodes[256];
    short int                   scancodes[RBLXWIN_KEY_LAST + 1];
    char                        keynames[RBLXWIN_KEY_LAST + 1][5];

    struct {
        void*                   handle;
        struct xkb_context*     context;
        struct xkb_keymap*      keymap;
        struct xkb_state*       state;
        struct xkb_compose_state* composeState;

        xkb_mod_index_t         controlIndex;
        xkb_mod_index_t         altIndex;
        xkb_mod_index_t         shiftIndex;
        xkb_mod_index_t         superIndex;
        xkb_mod_index_t         capsLockIndex;
        xkb_mod_index_t         numLockIndex;
        unsigned int            modifiers;

        PFN_xkb_context_new context_new;
        PFN_xkb_context_unref context_unref;
        PFN_xkb_keymap_new_from_string keymap_new_from_string;
        PFN_xkb_keymap_unref keymap_unref;
        PFN_xkb_keymap_mod_get_index keymap_mod_get_index;
        PFN_xkb_keymap_key_repeats keymap_key_repeats;
        PFN_xkb_keymap_key_get_syms_by_level keymap_key_get_syms_by_level;
        PFN_xkb_state_new state_new;
        PFN_xkb_state_unref state_unref;
        PFN_xkb_state_key_get_syms state_key_get_syms;
        PFN_xkb_state_update_mask state_update_mask;
        PFN_xkb_state_key_get_layout state_key_get_layout;
        PFN_xkb_state_mod_index_is_active state_mod_index_is_active;

        PFN_xkb_compose_table_new_from_locale compose_table_new_from_locale;
        PFN_xkb_compose_table_unref compose_table_unref;
        PFN_xkb_compose_state_new compose_state_new;
        PFN_xkb_compose_state_unref compose_state_unref;
        PFN_xkb_compose_state_feed compose_state_feed;
        PFN_xkb_compose_state_get_status compose_state_get_status;
        PFN_xkb_compose_state_get_one_sym compose_state_get_one_sym;
    } xkb;

    _RBLXWINwindow*                pointerFocus;
    _RBLXWINwindow*                keyboardFocus;

    struct {
        void*                   handle;

        PFN_wl_cursor_theme_load theme_load;
        PFN_wl_cursor_theme_destroy theme_destroy;
        PFN_wl_cursor_theme_get_cursor theme_get_cursor;
        PFN_wl_cursor_image_get_buffer image_get_buffer;
    } cursor;

    struct {
        void*                   handle;

        PFN_wl_egl_window_create window_create;
        PFN_wl_egl_window_destroy window_destroy;
        PFN_wl_egl_window_resize window_resize;
    } egl;
} _RBLXWINlibraryWayland;

// Wayland-specific per-monitor data
//
typedef struct _RBLXWINmonitorWayland
{
    struct wl_output*           output;
    uint32_t                    name;
    int                         currentMode;

    int                         x;
    int                         y;
    int                         scale;
} _RBLXWINmonitorWayland;

// Wayland-specific per-cursor data
//
typedef struct _RBLXWINcursorWayland
{
    struct wl_cursor*           cursor;
    struct wl_cursor*           cursorHiDPI;
    struct wl_buffer*           buffer;
    int                         width, height;
    int                         xhot, yhot;
    int                         currentImage;
} _RBLXWINcursorWayland;

void _rblxwinAddOutputWayland(uint32_t name, uint32_t version);
void _rblxwinUpdateContentScaleWayland(_RBLXWINwindow* window);
RBLXWINbool _rblxwinInputTextWayland(_RBLXWINwindow* window, uint32_t scancode);

void _rblxwinAddSeatListenerWayland(struct wl_seat* seat);
void _rblxwinAddDataDeviceListenerWayland(struct wl_data_device* device);

