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

#define _GNU_SOURCE

#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <signal.h>
#include <time.h>

#define LINEARWIN_BORDER_SIZE    4
#define LINEARWIN_CAPTION_HEIGHT 24

static int createTmpfileCloexec(char* tmpname)
{
    int fd;

    fd = mkostemp(tmpname, O_CLOEXEC);
    if (fd >= 0)
        unlink(tmpname);

    return fd;
}

/*
 * Create a new, unique, anonymous file of the given size, and
 * return the file descriptor for it. The file descriptor is set
 * CLOEXEC. The file is immediately suitable for mmap()'ing
 * the given size at offset zero.
 *
 * The file should not have a permanent backing store like a disk,
 * but may have if XDG_RUNTIME_DIR is not properly implemented in OS.
 *
 * The file name is deleted from the file system.
 *
 * The file is suitable for buffer sharing between processes by
 * transmitting the file descriptor over Unix sockets using the
 * SCM_RIGHTS methods.
 *
 * posix_fallocate() is used to guarantee that disk space is available
 * for the file at the given size. If disk space is insufficient, errno
 * is set to ENOSPC. If posix_fallocate() is not supported, program may
 * receive SIGBUS on accessing mmap()'ed file contents instead.
 */
static int createAnonymousFile(off_t size)
{
    static const char template[] = "/linearwin-shared-XXXXXX";
    const char* path;
    char* name;
    int fd;
    int ret;

#ifdef HAVE_MEMFD_CREATE
    fd = memfd_create("linearwin-shared", MFD_CLOEXEC | MFD_ALLOW_SEALING);
    if (fd >= 0)
    {
        // We can add this seal before calling posix_fallocate(), as the file
        // is currently zero-sized anyway.
        //
        // There is also no need to check for the return value, we couldn’t do
        // anything with it anyway.
        fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK | F_SEAL_SEAL);
    }
    else
#elif defined(SHM_ANON)
    fd = shm_open(SHM_ANON, O_RDWR | O_CLOEXEC, 0600);
    if (fd < 0)
#endif
    {
        path = getenv("XDG_RUNTIME_DIR");
        if (!path)
        {
            errno = ENOENT;
            return -1;
        }

        name = calloc(strlen(path) + sizeof(template), 1);
        strcpy(name, path);
        strcat(name, template);

        fd = createTmpfileCloexec(name);
        free(name);
        if (fd < 0)
            return -1;
    }

#if defined(SHM_ANON)
    // posix_fallocate does not work on SHM descriptors
    ret = ftruncate(fd, size);
#else
    ret = posix_fallocate(fd, 0, size);
#endif
    if (ret != 0)
    {
        close(fd);
        errno = ret;
        return -1;
    }
    return fd;
}

static struct wl_buffer* createShmBuffer(const LINEARWINimage* image)
{
    struct wl_shm_pool* pool;
    struct wl_buffer* buffer;
    int stride = image->width * 4;
    int length = image->width * image->height * 4;
    void* data;
    int fd, i;

    fd = createAnonymousFile(length);
    if (fd < 0)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to create buffer file of size %d: %s",
                        length, strerror(errno));
        return NULL;
    }

    data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to map file: %s", strerror(errno));
        close(fd);
        return NULL;
    }

    pool = wl_shm_create_pool(_linearwin.wl.shm, fd, length);

    close(fd);
    unsigned char* source = (unsigned char*) image->pixels;
    unsigned char* target = data;
    for (i = 0;  i < image->width * image->height;  i++, source += 4)
    {
        unsigned int alpha = source[3];

        *target++ = (unsigned char) ((source[2] * alpha) / 255);
        *target++ = (unsigned char) ((source[1] * alpha) / 255);
        *target++ = (unsigned char) ((source[0] * alpha) / 255);
        *target++ = (unsigned char) alpha;
    }

    buffer =
        wl_shm_pool_create_buffer(pool, 0,
                                  image->width,
                                  image->height,
                                  stride, WL_SHM_FORMAT_ARGB8888);
    munmap(data, length);
    wl_shm_pool_destroy(pool);

    return buffer;
}

// Wait for data to arrive on any of the specified file descriptors
//
static LINEARWINbool waitForData(struct pollfd* fds, nfds_t count, double* timeout)
{
    for (;;)
    {
        if (timeout)
        {
            const uint64_t base = _linearwinPlatformGetTimerValue();

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__CYGWIN__)
            const time_t seconds = (time_t) *timeout;
            const long nanoseconds = (long) ((*timeout - seconds) * 1e9);
            const struct timespec ts = { seconds, nanoseconds };
            const int result = ppoll(fds, count, &ts, NULL);
#elif defined(__NetBSD__)
            const time_t seconds = (time_t) *timeout;
            const long nanoseconds = (long) ((*timeout - seconds) * 1e9);
            const struct timespec ts = { seconds, nanoseconds };
            const int result = pollts(fds, count, &ts, NULL);
#else
            const int milliseconds = (int) (*timeout * 1e3);
            const int result = poll(fds, count, milliseconds);
#endif
            const int error = errno; // clock_gettime may overwrite our error

            *timeout -= (_linearwinPlatformGetTimerValue() - base) /
                (double) _linearwinPlatformGetTimerFrequency();

            if (result > 0)
                return LINEARWIN_TRUE;
            else if (result == -1 && error != EINTR && error != EAGAIN)
                return LINEARWIN_FALSE;
            else if (*timeout <= 0.0)
                return LINEARWIN_FALSE;
        }
        else
        {
            const int result = poll(fds, count, -1);
            if (result > 0)
                return LINEARWIN_TRUE;
            else if (result == -1 && errno != EINTR && errno != EAGAIN)
                return LINEARWIN_FALSE;
        }
    }
}

static void createFallbackDecoration(_LINEARWINdecorationWayland* decoration,
                                     struct wl_surface* parent,
                                     struct wl_buffer* buffer,
                                     int x, int y,
                                     int width, int height)
{
    decoration->surface = wl_compositor_create_surface(_linearwin.wl.compositor);
    decoration->subsurface =
        wl_subcompositor_get_subsurface(_linearwin.wl.subcompositor,
                                        decoration->surface, parent);
    wl_subsurface_set_position(decoration->subsurface, x, y);
    decoration->viewport = wp_viewporter_get_viewport(_linearwin.wl.viewporter,
                                                      decoration->surface);
    wp_viewport_set_destination(decoration->viewport, width, height);
    wl_surface_attach(decoration->surface, buffer, 0, 0);

    struct wl_region* region = wl_compositor_create_region(_linearwin.wl.compositor);
    wl_region_add(region, 0, 0, width, height);
    wl_surface_set_opaque_region(decoration->surface, region);
    wl_surface_commit(decoration->surface);
    wl_region_destroy(region);
}

static void createFallbackDecorations(_LINEARWINwindow* window)
{
    unsigned char data[] = { 224, 224, 224, 255 };
    const LINEARWINimage image = { 1, 1, data };

    if (!_linearwin.wl.viewporter)
        return;

    if (!window->wl.decorations.buffer)
        window->wl.decorations.buffer = createShmBuffer(&image);
    if (!window->wl.decorations.buffer)
        return;

    createFallbackDecoration(&window->wl.decorations.top, window->wl.surface,
                             window->wl.decorations.buffer,
                             0, -LINEARWIN_CAPTION_HEIGHT,
                             window->wl.width, LINEARWIN_CAPTION_HEIGHT);
    createFallbackDecoration(&window->wl.decorations.left, window->wl.surface,
                             window->wl.decorations.buffer,
                             -LINEARWIN_BORDER_SIZE, -LINEARWIN_CAPTION_HEIGHT,
                             LINEARWIN_BORDER_SIZE, window->wl.height + LINEARWIN_CAPTION_HEIGHT);
    createFallbackDecoration(&window->wl.decorations.right, window->wl.surface,
                             window->wl.decorations.buffer,
                             window->wl.width, -LINEARWIN_CAPTION_HEIGHT,
                             LINEARWIN_BORDER_SIZE, window->wl.height + LINEARWIN_CAPTION_HEIGHT);
    createFallbackDecoration(&window->wl.decorations.bottom, window->wl.surface,
                             window->wl.decorations.buffer,
                             -LINEARWIN_BORDER_SIZE, window->wl.height,
                             window->wl.width + LINEARWIN_BORDER_SIZE * 2, LINEARWIN_BORDER_SIZE);
}

static void destroyFallbackDecoration(_LINEARWINdecorationWayland* decoration)
{
    if (decoration->subsurface)
        wl_subsurface_destroy(decoration->subsurface);
    if (decoration->surface)
        wl_surface_destroy(decoration->surface);
    if (decoration->viewport)
        wp_viewport_destroy(decoration->viewport);
    decoration->surface = NULL;
    decoration->subsurface = NULL;
    decoration->viewport = NULL;
}

static void destroyFallbackDecorations(_LINEARWINwindow* window)
{
    destroyFallbackDecoration(&window->wl.decorations.top);
    destroyFallbackDecoration(&window->wl.decorations.left);
    destroyFallbackDecoration(&window->wl.decorations.right);
    destroyFallbackDecoration(&window->wl.decorations.bottom);
}

static void xdgDecorationHandleConfigure(void* userData,
                                         struct zxdg_toplevel_decoration_v1* decoration,
                                         uint32_t mode)
{
    _LINEARWINwindow* window = userData;

    window->wl.xdg.decorationMode = mode;

    if (mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE)
    {
        if (window->decorated && !window->monitor)
            createFallbackDecorations(window);
    }
    else
        destroyFallbackDecorations(window);
}

static const struct zxdg_toplevel_decoration_v1_listener xdgDecorationListener =
{
    xdgDecorationHandleConfigure,
};

// Makes the surface considered as XRGB instead of ARGB.
static void setContentAreaOpaque(_LINEARWINwindow* window)
{
    struct wl_region* region;

    region = wl_compositor_create_region(_linearwin.wl.compositor);
    if (!region)
        return;

    wl_region_add(region, 0, 0, window->wl.width, window->wl.height);
    wl_surface_set_opaque_region(window->wl.surface, region);
    wl_region_destroy(region);
}


static void resizeWindow(_LINEARWINwindow* window)
{
    int scale = window->wl.scale;
    int scaledWidth = window->wl.width * scale;
    int scaledHeight = window->wl.height * scale;

    if (window->wl.egl.window)
        wl_egl_window_resize(window->wl.egl.window, scaledWidth, scaledHeight, 0, 0);
    if (!window->wl.transparent)
        setContentAreaOpaque(window);
    _linearwinInputFramebufferSize(window, scaledWidth, scaledHeight);

    if (!window->wl.decorations.top.surface)
        return;

    wp_viewport_set_destination(window->wl.decorations.top.viewport,
                                window->wl.width, LINEARWIN_CAPTION_HEIGHT);
    wl_surface_commit(window->wl.decorations.top.surface);

    wp_viewport_set_destination(window->wl.decorations.left.viewport,
                                LINEARWIN_BORDER_SIZE, window->wl.height + LINEARWIN_CAPTION_HEIGHT);
    wl_surface_commit(window->wl.decorations.left.surface);

    wl_subsurface_set_position(window->wl.decorations.right.subsurface,
                               window->wl.width, -LINEARWIN_CAPTION_HEIGHT);
    wp_viewport_set_destination(window->wl.decorations.right.viewport,
                                LINEARWIN_BORDER_SIZE, window->wl.height + LINEARWIN_CAPTION_HEIGHT);
    wl_surface_commit(window->wl.decorations.right.surface);

    wl_subsurface_set_position(window->wl.decorations.bottom.subsurface,
                               -LINEARWIN_BORDER_SIZE, window->wl.height);
    wp_viewport_set_destination(window->wl.decorations.bottom.viewport,
                                window->wl.width + LINEARWIN_BORDER_SIZE * 2, LINEARWIN_BORDER_SIZE);
    wl_surface_commit(window->wl.decorations.bottom.surface);
}

void _linearwinUpdateContentScaleWayland(_LINEARWINwindow* window)
{
    if (_linearwin.wl.compositorVersion < WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION)
        return;

    // Get the scale factor from the highest scale monitor.
    int maxScale = 1;

    for (int i = 0; i < window->wl.monitorsCount; i++)
        maxScale = _linearwin_max(window->wl.monitors[i]->wl.scale, maxScale);

    // Only change the framebuffer size if the scale changed.
    if (window->wl.scale != maxScale)
    {
        window->wl.scale = maxScale;
        wl_surface_set_buffer_scale(window->wl.surface, maxScale);
        _linearwinInputWindowContentScale(window, maxScale, maxScale);
        resizeWindow(window);
    }
}

static void surfaceHandleEnter(void* userData,
                               struct wl_surface* surface,
                               struct wl_output* output)
{
    _LINEARWINwindow* window = userData;
    _LINEARWINmonitor* monitor = wl_output_get_user_data(output);

    if (window->wl.monitorsCount + 1 > window->wl.monitorsSize)
    {
        ++window->wl.monitorsSize;
        window->wl.monitors =
            realloc(window->wl.monitors,
                    window->wl.monitorsSize * sizeof(_LINEARWINmonitor*));
    }

    window->wl.monitors[window->wl.monitorsCount++] = monitor;

    _linearwinUpdateContentScaleWayland(window);
}

static void surfaceHandleLeave(void* userData,
                               struct wl_surface* surface,
                               struct wl_output* output)
{
    _LINEARWINwindow* window = userData;
    _LINEARWINmonitor* monitor = wl_output_get_user_data(output);
    LINEARWINbool found;
    int i;

    for (i = 0, found = LINEARWIN_FALSE; i < window->wl.monitorsCount - 1; ++i)
    {
        if (monitor == window->wl.monitors[i])
            found = LINEARWIN_TRUE;
        if (found)
            window->wl.monitors[i] = window->wl.monitors[i + 1];
    }
    window->wl.monitors[--window->wl.monitorsCount] = NULL;

    _linearwinUpdateContentScaleWayland(window);
}

static const struct wl_surface_listener surfaceListener =
{
    surfaceHandleEnter,
    surfaceHandleLeave
};

static void setIdleInhibitor(_LINEARWINwindow* window, LINEARWINbool enable)
{
    if (enable && !window->wl.idleInhibitor && _linearwin.wl.idleInhibitManager)
    {
        window->wl.idleInhibitor =
            zwp_idle_inhibit_manager_v1_create_inhibitor(
                _linearwin.wl.idleInhibitManager, window->wl.surface);
        if (!window->wl.idleInhibitor)
            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "Wayland: Failed to create idle inhibitor");
    }
    else if (!enable && window->wl.idleInhibitor)
    {
        zwp_idle_inhibitor_v1_destroy(window->wl.idleInhibitor);
        window->wl.idleInhibitor = NULL;
    }
}

// Make the specified window and its video mode active on its monitor
//
static void acquireMonitor(_LINEARWINwindow* window)
{
    if (window->wl.xdg.toplevel)
    {
        xdg_toplevel_set_fullscreen(window->wl.xdg.toplevel,
                                    window->monitor->wl.output);
    }

    setIdleInhibitor(window, LINEARWIN_TRUE);

    if (window->wl.decorations.top.surface)
        destroyFallbackDecorations(window);
}

// Remove the window and restore the original video mode
//
static void releaseMonitor(_LINEARWINwindow* window)
{
    if (window->wl.xdg.toplevel)
        xdg_toplevel_unset_fullscreen(window->wl.xdg.toplevel);

    setIdleInhibitor(window, LINEARWIN_FALSE);

    if (window->wl.xdg.decorationMode != ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE)
    {
        if (window->decorated)
            createFallbackDecorations(window);
    }
}

static void xdgToplevelHandleConfigure(void* userData,
                                       struct xdg_toplevel* toplevel,
                                       int32_t width,
                                       int32_t height,
                                       struct wl_array* states)
{
    _LINEARWINwindow* window = userData;
    uint32_t* state;

    window->wl.pending.activated  = LINEARWIN_FALSE;
    window->wl.pending.maximized  = LINEARWIN_FALSE;
    window->wl.pending.fullscreen = LINEARWIN_FALSE;

    wl_array_for_each(state, states)
    {
        switch (*state)
        {
            case XDG_TOPLEVEL_STATE_MAXIMIZED:
                window->wl.pending.maximized = LINEARWIN_TRUE;
                break;
            case XDG_TOPLEVEL_STATE_FULLSCREEN:
                window->wl.pending.fullscreen = LINEARWIN_TRUE;
                break;
            case XDG_TOPLEVEL_STATE_RESIZING:
                break;
            case XDG_TOPLEVEL_STATE_ACTIVATED:
                window->wl.pending.activated = LINEARWIN_TRUE;
                break;
        }
    }

    if (width && height)
    {
        if (window->wl.decorations.top.surface)
        {
            window->wl.pending.width  = _linearwin_max(0, width - LINEARWIN_BORDER_SIZE * 2);
            window->wl.pending.height =
                _linearwin_max(0, height - LINEARWIN_BORDER_SIZE - LINEARWIN_CAPTION_HEIGHT);
        }
        else
        {
            window->wl.pending.width  = width;
            window->wl.pending.height = height;
        }
    }
    else
    {
        window->wl.pending.width  = window->wl.width;
        window->wl.pending.height = window->wl.height;
    }
}

static void xdgToplevelHandleClose(void* userData,
                                   struct xdg_toplevel* toplevel)
{
    _LINEARWINwindow* window = userData;
    _linearwinInputWindowCloseRequest(window);
}

static const struct xdg_toplevel_listener xdgToplevelListener =
{
    xdgToplevelHandleConfigure,
    xdgToplevelHandleClose
};

static void xdgSurfaceHandleConfigure(void* userData,
                                      struct xdg_surface* surface,
                                      uint32_t serial)
{
    _LINEARWINwindow* window = userData;

    xdg_surface_ack_configure(surface, serial);

    if (window->wl.activated != window->wl.pending.activated)
    {
        window->wl.activated = window->wl.pending.activated;
        if (!window->wl.activated)
        {
            if (window->monitor && window->autoIconify)
                xdg_toplevel_set_minimized(window->wl.xdg.toplevel);
        }
    }

    if (window->wl.maximized != window->wl.pending.maximized)
    {
        window->wl.maximized = window->wl.pending.maximized;
        _linearwinInputWindowMaximize(window, window->wl.maximized);
    }

    window->wl.fullscreen = window->wl.pending.fullscreen;

    int width  = window->wl.pending.width;
    int height = window->wl.pending.height;

    if (!window->wl.maximized && !window->wl.fullscreen)
    {
        if (window->numer != LINEARWIN_DONT_CARE && window->denom != LINEARWIN_DONT_CARE)
        {
            const float aspectRatio = (float) width / (float) height;
            const float targetRatio = (float) window->numer / (float) window->denom;
            if (aspectRatio < targetRatio)
                height = width / targetRatio;
            else if (aspectRatio > targetRatio)
                width = height * targetRatio;
        }
    }

    if (width != window->wl.width || height != window->wl.height)
    {
        window->wl.width = width;
        window->wl.height = height;
        resizeWindow(window);

        _linearwinInputWindowSize(window, width, height);

        if (window->wl.visible)
            _linearwinInputWindowDamage(window);
    }

    if (!window->wl.visible)
    {
        // Allow the window to be mapped only if it either has no XDG
        // decorations or they have already received a configure event
        if (!window->wl.xdg.decoration || window->wl.xdg.decorationMode)
        {
            window->wl.visible = LINEARWIN_TRUE;
            _linearwinInputWindowDamage(window);
        }
    }
}

static const struct xdg_surface_listener xdgSurfaceListener =
{
    xdgSurfaceHandleConfigure
};

static LINEARWINbool createShellObjects(_LINEARWINwindow* window)
{
    window->wl.xdg.surface = xdg_wm_base_get_xdg_surface(_linearwin.wl.wmBase,
                                                         window->wl.surface);
    if (!window->wl.xdg.surface)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to create xdg-surface for window");
        return LINEARWIN_FALSE;
    }

    xdg_surface_add_listener(window->wl.xdg.surface, &xdgSurfaceListener, window);

    window->wl.xdg.toplevel = xdg_surface_get_toplevel(window->wl.xdg.surface);
    if (!window->wl.xdg.toplevel)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to create xdg-toplevel for window");
        return LINEARWIN_FALSE;
    }

    xdg_toplevel_add_listener(window->wl.xdg.toplevel, &xdgToplevelListener, window);

    if (window->wl.title)
        xdg_toplevel_set_title(window->wl.xdg.toplevel, window->wl.title);

    if (window->monitor)
    {
        xdg_toplevel_set_fullscreen(window->wl.xdg.toplevel, window->monitor->wl.output);
        setIdleInhibitor(window, LINEARWIN_TRUE);
    }
    else
    {
        if (window->wl.maximized)
            xdg_toplevel_set_maximized(window->wl.xdg.toplevel);

        setIdleInhibitor(window, LINEARWIN_FALSE);

        if (_linearwin.wl.decorationManager)
        {
            window->wl.xdg.decoration =
                zxdg_decoration_manager_v1_get_toplevel_decoration(
                    _linearwin.wl.decorationManager, window->wl.xdg.toplevel);
            zxdg_toplevel_decoration_v1_add_listener(window->wl.xdg.decoration,
                                                     &xdgDecorationListener,
                                                     window);

            uint32_t mode;

            if (window->decorated)
                mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
            else
                mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;

            zxdg_toplevel_decoration_v1_set_mode(window->wl.xdg.decoration, mode);
        }
        else
        {
            if (window->decorated)
                createFallbackDecorations(window);
        }
    }

    if (window->minwidth != LINEARWIN_DONT_CARE && window->minheight != LINEARWIN_DONT_CARE)
    {
        int minwidth  = window->minwidth;
        int minheight = window->minheight;

        if (window->wl.decorations.top.surface)
        {
            minwidth  += LINEARWIN_BORDER_SIZE * 2;
            minheight += LINEARWIN_CAPTION_HEIGHT + LINEARWIN_BORDER_SIZE;
        }

        xdg_toplevel_set_min_size(window->wl.xdg.toplevel, minwidth, minheight);
    }

    if (window->maxwidth != LINEARWIN_DONT_CARE && window->maxheight != LINEARWIN_DONT_CARE)
    {
        int maxwidth  = window->maxwidth;
        int maxheight = window->maxheight;

        if (window->wl.decorations.top.surface)
        {
            maxwidth  += LINEARWIN_BORDER_SIZE * 2;
            maxheight += LINEARWIN_CAPTION_HEIGHT + LINEARWIN_BORDER_SIZE;
        }

        xdg_toplevel_set_max_size(window->wl.xdg.toplevel, maxwidth, maxheight);
    }

    wl_surface_commit(window->wl.surface);
    wl_display_roundtrip(_linearwin.wl.display);

    return LINEARWIN_TRUE;
}

static void destroyShellObjects(_LINEARWINwindow* window)
{
    destroyFallbackDecorations(window);

    if (window->wl.xdg.decoration)
        zxdg_toplevel_decoration_v1_destroy(window->wl.xdg.decoration);

    if (window->wl.xdg.toplevel)
        xdg_toplevel_destroy(window->wl.xdg.toplevel);

    if (window->wl.xdg.surface)
        xdg_surface_destroy(window->wl.xdg.surface);

    window->wl.xdg.decoration = NULL;
    window->wl.xdg.decorationMode = 0;
    window->wl.xdg.toplevel = NULL;
    window->wl.xdg.surface = NULL;
}

static LINEARWINbool createNativeSurface(_LINEARWINwindow* window,
                                    const _LINEARWINwndconfig* wndconfig,
                                    const _LINEARWINfbconfig* fbconfig)
{
    window->wl.surface = wl_compositor_create_surface(_linearwin.wl.compositor);
    if (!window->wl.surface)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR, "Wayland: Failed to create window surface");
        return LINEARWIN_FALSE;
    }

    wl_surface_add_listener(window->wl.surface,
                            &surfaceListener,
                            window);

    wl_surface_set_user_data(window->wl.surface, window);

    window->wl.width = wndconfig->width;
    window->wl.height = wndconfig->height;
    window->wl.scale = 1;
    window->wl.title = _linearwin_strdup(wndconfig->title);

    window->wl.maximized = wndconfig->maximized;

    window->wl.transparent = fbconfig->transparent;
    if (!window->wl.transparent)
        setContentAreaOpaque(window);

    return LINEARWIN_TRUE;
}

static void setCursorImage(_LINEARWINwindow* window,
                           _LINEARWINcursorWayland* cursorWayland)
{
    struct itimerspec timer = {};
    struct wl_cursor* wlCursor = cursorWayland->cursor;
    struct wl_cursor_image* image;
    struct wl_buffer* buffer;
    struct wl_surface* surface = _linearwin.wl.cursorSurface;
    int scale = 1;

    if (!wlCursor)
        buffer = cursorWayland->buffer;
    else
    {
        if (window->wl.scale > 1 && cursorWayland->cursorHiDPI)
        {
            wlCursor = cursorWayland->cursorHiDPI;
            scale = 2;
        }

        image = wlCursor->images[cursorWayland->currentImage];
        buffer = wl_cursor_image_get_buffer(image);
        if (!buffer)
            return;

        timer.it_value.tv_sec = image->delay / 1000;
        timer.it_value.tv_nsec = (image->delay % 1000) * 1000000;
        timerfd_settime(_linearwin.wl.cursorTimerfd, 0, &timer, NULL);

        cursorWayland->width = image->width;
        cursorWayland->height = image->height;
        cursorWayland->xhot = image->hotspot_x;
        cursorWayland->yhot = image->hotspot_y;
    }

    wl_pointer_set_cursor(_linearwin.wl.pointer, _linearwin.wl.pointerEnterSerial,
                          surface,
                          cursorWayland->xhot / scale,
                          cursorWayland->yhot / scale);
    wl_surface_set_buffer_scale(surface, scale);
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage(surface, 0, 0,
                      cursorWayland->width, cursorWayland->height);
    wl_surface_commit(surface);
}

static void incrementCursorImage(_LINEARWINwindow* window)
{
    _LINEARWINcursor* cursor;

    if (!window || window->wl.decorations.focus != mainWindow)
        return;

    cursor = window->wl.currentCursor;
    if (cursor && cursor->wl.cursor)
    {
        cursor->wl.currentImage += 1;
        cursor->wl.currentImage %= cursor->wl.cursor->image_count;
        setCursorImage(window, &cursor->wl);
    }
}

static LINEARWINbool flushDisplay(void)
{
    while (wl_display_flush(_linearwin.wl.display) == -1)
    {
        if (errno != EAGAIN)
            return LINEARWIN_FALSE;

        struct pollfd fd = { wl_display_get_fd(_linearwin.wl.display), POLLOUT };

        while (poll(&fd, 1, -1) == -1)
        {
            if (errno != EINTR && errno != EAGAIN)
                return LINEARWIN_FALSE;
        }
    }

    return LINEARWIN_TRUE;
}

static void handleEvents(double* timeout)
{
    LINEARWINbool event = LINEARWIN_FALSE;
    struct pollfd fds[] =
    {
        { wl_display_get_fd(_linearwin.wl.display), POLLIN },
        { _linearwin.wl.timerfd, POLLIN },
        { _linearwin.wl.cursorTimerfd, POLLIN },
    };

    while (!event)
    {
        while (wl_display_prepare_read(_linearwin.wl.display) != 0)
            wl_display_dispatch_pending(_linearwin.wl.display);

        // If an error other than EAGAIN happens, we have likely been disconnected
        // from the Wayland session; try to handle that the best we can.
        if (!flushDisplay())
        {
            wl_display_cancel_read(_linearwin.wl.display);

            _LINEARWINwindow* window = _linearwin.windowListHead;
            while (window)
            {
                _linearwinInputWindowCloseRequest(window);
                window = window->next;
            }

            return;
        }

        if (!waitForData(fds, 3, timeout))
        {
            wl_display_cancel_read(_linearwin.wl.display);
            return;
        }

        if (fds[0].revents & POLLIN)
        {
            wl_display_read_events(_linearwin.wl.display);
            if (wl_display_dispatch_pending(_linearwin.wl.display) > 0)
                event = LINEARWIN_TRUE;
        }
        else
            wl_display_cancel_read(_linearwin.wl.display);

        if (fds[1].revents & POLLIN)
        {
            uint64_t repeats;

            if (read(_linearwin.wl.timerfd, &repeats, sizeof(repeats)) == 8)
            {
                for (uint64_t i = 0; i < repeats; i++)
                {
                    _linearwinInputKey(_linearwin.wl.keyboardFocus,
                                  _linearwin.wl.keyboardLastKey,
                                  _linearwin.wl.keyboardLastScancode,
                                  LINEARWIN_PRESS,
                                  _linearwin.wl.xkb.modifiers);
                    _linearwinInputTextWayland(_linearwin.wl.keyboardFocus,
                                          _linearwin.wl.keyboardLastScancode);
                }

                event = LINEARWIN_TRUE;
            }
        }

        if (fds[2].revents & POLLIN)
        {
            uint64_t repeats;

            if (read(_linearwin.wl.cursorTimerfd, &repeats, sizeof(repeats)) == 8)
            {
                incrementCursorImage(_linearwin.wl.pointerFocus);
                event = LINEARWIN_TRUE;
            }
        }
    }
}

// Reads the specified data offer as the specified MIME type
//
static char* readDataOfferAsString(struct wl_data_offer* offer, const char* mimeType)
{
    int fds[2];

    if (pipe2(fds, O_CLOEXEC) == -1)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to create pipe for data offer: %s",
                        strerror(errno));
        return NULL;
    }

    wl_data_offer_receive(offer, mimeType, fds[1]);
    flushDisplay();
    close(fds[1]);

    char* string = NULL;
    size_t size = 0;
    size_t length = 0;

    for (;;)
    {
        const size_t readSize = 4096;
        const size_t requiredSize = length + readSize + 1;
        if (requiredSize > size)
        {
            char* longer = realloc(string, requiredSize);
            if (!longer)
            {
                _linearwinInputError(LINEARWIN_OUT_OF_MEMORY, NULL);
                close(fds[0]);
                return NULL;
            }

            string = longer;
            size = requiredSize;
        }

        const ssize_t result = read(fds[0], string + length, readSize);
        if (result == 0)
            break;
        else if (result == -1)
        {
            if (errno == EINTR)
                continue;

            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "Wayland: Failed to read from data offer pipe: %s",
                            strerror(errno));
            close(fds[0]);
            return NULL;
        }

        length += result;
    }

    close(fds[0]);

    string[length] = '\0';
    return string;
}

static _LINEARWINwindow* findWindowFromDecorationSurface(struct wl_surface* surface,
                                                    _LINEARWINdecorationSideWayland* which)
{
    _LINEARWINdecorationSideWayland focus;
    _LINEARWINwindow* window = _linearwin.windowListHead;
    if (!which)
        which = &focus;
    while (window)
    {
        if (surface == window->wl.decorations.top.surface)
        {
            *which = topDecoration;
            break;
        }
        if (surface == window->wl.decorations.left.surface)
        {
            *which = leftDecoration;
            break;
        }
        if (surface == window->wl.decorations.right.surface)
        {
            *which = rightDecoration;
            break;
        }
        if (surface == window->wl.decorations.bottom.surface)
        {
            *which = bottomDecoration;
            break;
        }
        window = window->next;
    }
    return window;
}

static void pointerHandleEnter(void* userData,
                               struct wl_pointer* pointer,
                               uint32_t serial,
                               struct wl_surface* surface,
                               wl_fixed_t sx,
                               wl_fixed_t sy)
{
    // Happens in the case we just destroyed the surface.
    if (!surface)
        return;

    _LINEARWINdecorationSideWayland focus = mainWindow;
    _LINEARWINwindow* window = wl_surface_get_user_data(surface);
    if (!window)
    {
        window = findWindowFromDecorationSurface(surface, &focus);
        if (!window)
            return;
    }

    window->wl.decorations.focus = focus;
    _linearwin.wl.serial = serial;
    _linearwin.wl.pointerEnterSerial = serial;
    _linearwin.wl.pointerFocus = window;

    window->wl.hovered = LINEARWIN_TRUE;

    _linearwinPlatformSetCursor(window, window->wl.currentCursor);
    _linearwinInputCursorEnter(window, LINEARWIN_TRUE);
}

static void pointerHandleLeave(void* userData,
                               struct wl_pointer* pointer,
                               uint32_t serial,
                               struct wl_surface* surface)
{
    _LINEARWINwindow* window = _linearwin.wl.pointerFocus;

    if (!window)
        return;

    window->wl.hovered = LINEARWIN_FALSE;

    _linearwin.wl.serial = serial;
    _linearwin.wl.pointerFocus = NULL;
    _linearwinInputCursorEnter(window, LINEARWIN_FALSE);
    _linearwin.wl.cursorPreviousName = NULL;
}

static void setCursor(_LINEARWINwindow* window, const char* name)
{
    struct wl_buffer* buffer;
    struct wl_cursor* cursor;
    struct wl_cursor_image* image;
    struct wl_surface* surface = _linearwin.wl.cursorSurface;
    struct wl_cursor_theme* theme = _linearwin.wl.cursorTheme;
    int scale = 1;

    if (window->wl.scale > 1 && _linearwin.wl.cursorThemeHiDPI)
    {
        // We only support up to scale=2 for now, since libwayland-cursor
        // requires us to load a different theme for each size.
        scale = 2;
        theme = _linearwin.wl.cursorThemeHiDPI;
    }

    cursor = wl_cursor_theme_get_cursor(theme, name);
    if (!cursor)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Standard cursor shape unavailable");
        return;
    }
    // TODO: handle animated cursors too.
    image = cursor->images[0];

    if (!image)
        return;

    buffer = wl_cursor_image_get_buffer(image);
    if (!buffer)
        return;
    wl_pointer_set_cursor(_linearwin.wl.pointer, _linearwin.wl.pointerEnterSerial,
                          surface,
                          image->hotspot_x / scale,
                          image->hotspot_y / scale);
    wl_surface_set_buffer_scale(surface, scale);
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage(surface, 0, 0,
                      image->width, image->height);
    wl_surface_commit(surface);
    _linearwin.wl.cursorPreviousName = name;
}

static void pointerHandleMotion(void* userData,
                                struct wl_pointer* pointer,
                                uint32_t time,
                                wl_fixed_t sx,
                                wl_fixed_t sy)
{
    _LINEARWINwindow* window = _linearwin.wl.pointerFocus;
    const char* cursorName = NULL;
    double x, y;

    if (!window)
        return;

    if (window->cursorMode == LINEARWIN_CURSOR_DISABLED)
        return;
    x = wl_fixed_to_double(sx);
    y = wl_fixed_to_double(sy);
    window->wl.cursorPosX = x;
    window->wl.cursorPosY = y;

    switch (window->wl.decorations.focus)
    {
        case mainWindow:
            _linearwinInputCursorPos(window, x, y);
            _linearwin.wl.cursorPreviousName = NULL;
            return;
        case topDecoration:
            if (y < LINEARWIN_BORDER_SIZE)
                cursorName = "n-resize";
            else
                cursorName = "left_ptr";
            break;
        case leftDecoration:
            if (y < LINEARWIN_BORDER_SIZE)
                cursorName = "nw-resize";
            else
                cursorName = "w-resize";
            break;
        case rightDecoration:
            if (y < LINEARWIN_BORDER_SIZE)
                cursorName = "ne-resize";
            else
                cursorName = "e-resize";
            break;
        case bottomDecoration:
            if (x < LINEARWIN_BORDER_SIZE)
                cursorName = "sw-resize";
            else if (x > window->wl.width + LINEARWIN_BORDER_SIZE)
                cursorName = "se-resize";
            else
                cursorName = "s-resize";
            break;
        default:
            assert(0);
    }
    if (_linearwin.wl.cursorPreviousName != cursorName)
        setCursor(window, cursorName);
}

static void pointerHandleButton(void* userData,
                                struct wl_pointer* pointer,
                                uint32_t serial,
                                uint32_t time,
                                uint32_t button,
                                uint32_t state)
{
    _LINEARWINwindow* window = _linearwin.wl.pointerFocus;
    int linearwinButton;

    uint32_t edges = XDG_TOPLEVEL_RESIZE_EDGE_NONE;

    if (!window)
        return;
    if (button == BTN_LEFT)
    {
        switch (window->wl.decorations.focus)
        {
            case mainWindow:
                break;
            case topDecoration:
                if (window->wl.cursorPosY < LINEARWIN_BORDER_SIZE)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP;
                else
                {
                    xdg_toplevel_move(window->wl.xdg.toplevel, _linearwin.wl.seat, serial);
                }
                break;
            case leftDecoration:
                if (window->wl.cursorPosY < LINEARWIN_BORDER_SIZE)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
                else
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
                break;
            case rightDecoration:
                if (window->wl.cursorPosY < LINEARWIN_BORDER_SIZE)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
                else
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
                break;
            case bottomDecoration:
                if (window->wl.cursorPosX < LINEARWIN_BORDER_SIZE)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
                else if (window->wl.cursorPosX > window->wl.width + LINEARWIN_BORDER_SIZE)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
                else
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
                break;
            default:
                assert(0);
        }
        if (edges != XDG_TOPLEVEL_RESIZE_EDGE_NONE)
        {
            xdg_toplevel_resize(window->wl.xdg.toplevel, _linearwin.wl.seat,
                                serial, edges);
            return;
        }
    }
    else if (button == BTN_RIGHT)
    {
        if (window->wl.decorations.focus != mainWindow && window->wl.xdg.toplevel)
        {
            xdg_toplevel_show_window_menu(window->wl.xdg.toplevel,
                                          _linearwin.wl.seat, serial,
                                          window->wl.cursorPosX,
                                          window->wl.cursorPosY);
            return;
        }
    }

    // Don’t pass the button to the user if it was related to a decoration.
    if (window->wl.decorations.focus != mainWindow)
        return;

    _linearwin.wl.serial = serial;

    /* Makes left, right and middle 0, 1 and 2. Overall order follows evdev
     * codes. */
    linearwinButton = button - BTN_LEFT;

    _linearwinInputMouseClick(window,
                         linearwinButton,
                         state == WL_POINTER_BUTTON_STATE_PRESSED
                                ? LINEARWIN_PRESS
                                : LINEARWIN_RELEASE,
                         _linearwin.wl.xkb.modifiers);
}

static void pointerHandleAxis(void* userData,
                              struct wl_pointer* pointer,
                              uint32_t time,
                              uint32_t axis,
                              wl_fixed_t value)
{
    _LINEARWINwindow* window = _linearwin.wl.pointerFocus;
    double x = 0.0, y = 0.0;
    // Wayland scroll events are in pointer motion coordinate space (think two
    // finger scroll).  The factor 10 is commonly used to convert to "scroll
    // step means 1.0.
    const double scrollFactor = 1.0 / 10.0;

    if (!window)
        return;

    assert(axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL ||
           axis == WL_POINTER_AXIS_VERTICAL_SCROLL);

    if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
        x = -wl_fixed_to_double(value) * scrollFactor;
    else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
        y = -wl_fixed_to_double(value) * scrollFactor;

    _linearwinInputScroll(window, x, y);
}

static const struct wl_pointer_listener pointerListener =
{
    pointerHandleEnter,
    pointerHandleLeave,
    pointerHandleMotion,
    pointerHandleButton,
    pointerHandleAxis,
};

static void keyboardHandleKeymap(void* userData,
                                 struct wl_keyboard* keyboard,
                                 uint32_t format,
                                 int fd,
                                 uint32_t size)
{
    struct xkb_keymap* keymap;
    struct xkb_state* state;
    struct xkb_compose_table* composeTable;
    struct xkb_compose_state* composeState;
    char* mapStr;
    const char* locale;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
    {
        close(fd);
        return;
    }

    mapStr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (mapStr == MAP_FAILED) {
        close(fd);
        return;
    }

    keymap = xkb_keymap_new_from_string(_linearwin.wl.xkb.context,
                                        mapStr,
                                        XKB_KEYMAP_FORMAT_TEXT_V1,
                                        0);
    munmap(mapStr, size);
    close(fd);

    if (!keymap)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to compile keymap");
        return;
    }

    state = xkb_state_new(keymap);
    if (!state)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to create XKB state");
        xkb_keymap_unref(keymap);
        return;
    }

    // Look up the preferred locale, falling back to "C" as default.
    locale = getenv("LC_ALL");
    if (!locale)
        locale = getenv("LC_CTYPE");
    if (!locale)
        locale = getenv("LANG");
    if (!locale)
        locale = "C";

    composeTable =
        xkb_compose_table_new_from_locale(_linearwin.wl.xkb.context, locale,
                                          XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (composeTable)
    {
        composeState =
            xkb_compose_state_new(composeTable, XKB_COMPOSE_STATE_NO_FLAGS);
        xkb_compose_table_unref(composeTable);
        if (composeState)
            _linearwin.wl.xkb.composeState = composeState;
        else
            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "Wayland: Failed to create XKB compose state");
    }
    else
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to create XKB compose table");
    }

    xkb_keymap_unref(_linearwin.wl.xkb.keymap);
    xkb_state_unref(_linearwin.wl.xkb.state);
    _linearwin.wl.xkb.keymap = keymap;
    _linearwin.wl.xkb.state = state;

    _linearwin.wl.xkb.controlIndex  = xkb_keymap_mod_get_index(_linearwin.wl.xkb.keymap, "Control");
    _linearwin.wl.xkb.altIndex      = xkb_keymap_mod_get_index(_linearwin.wl.xkb.keymap, "Mod1");
    _linearwin.wl.xkb.shiftIndex    = xkb_keymap_mod_get_index(_linearwin.wl.xkb.keymap, "Shift");
    _linearwin.wl.xkb.superIndex    = xkb_keymap_mod_get_index(_linearwin.wl.xkb.keymap, "Mod4");
    _linearwin.wl.xkb.capsLockIndex = xkb_keymap_mod_get_index(_linearwin.wl.xkb.keymap, "Lock");
    _linearwin.wl.xkb.numLockIndex  = xkb_keymap_mod_get_index(_linearwin.wl.xkb.keymap, "Mod2");
}

static void keyboardHandleEnter(void* userData,
                                struct wl_keyboard* keyboard,
                                uint32_t serial,
                                struct wl_surface* surface,
                                struct wl_array* keys)
{
    // Happens in the case we just destroyed the surface.
    if (!surface)
        return;

    _LINEARWINwindow* window = wl_surface_get_user_data(surface);
    if (!window)
    {
        window = findWindowFromDecorationSurface(surface, NULL);
        if (!window)
            return;
    }

    _linearwin.wl.serial = serial;
    _linearwin.wl.keyboardFocus = window;
    _linearwinInputWindowFocus(window, LINEARWIN_TRUE);
}

static void keyboardHandleLeave(void* userData,
                                struct wl_keyboard* keyboard,
                                uint32_t serial,
                                struct wl_surface* surface)
{
    _LINEARWINwindow* window = _linearwin.wl.keyboardFocus;

    if (!window)
        return;

    struct itimerspec timer = {};
    timerfd_settime(_linearwin.wl.timerfd, 0, &timer, NULL);

    _linearwin.wl.serial = serial;
    _linearwin.wl.keyboardFocus = NULL;
    _linearwinInputWindowFocus(window, LINEARWIN_FALSE);
}

static int translateKey(uint32_t scancode)
{
    if (scancode < sizeof(_linearwin.wl.keycodes) / sizeof(_linearwin.wl.keycodes[0]))
        return _linearwin.wl.keycodes[scancode];

    return LINEARWIN_KEY_UNKNOWN;
}

static xkb_keysym_t composeSymbol(xkb_keysym_t sym)
{
    if (sym == XKB_KEY_NoSymbol || !_linearwin.wl.xkb.composeState)
        return sym;
    if (xkb_compose_state_feed(_linearwin.wl.xkb.composeState, sym)
            != XKB_COMPOSE_FEED_ACCEPTED)
        return sym;
    switch (xkb_compose_state_get_status(_linearwin.wl.xkb.composeState))
    {
        case XKB_COMPOSE_COMPOSED:
            return xkb_compose_state_get_one_sym(_linearwin.wl.xkb.composeState);
        case XKB_COMPOSE_COMPOSING:
        case XKB_COMPOSE_CANCELLED:
            return XKB_KEY_NoSymbol;
        case XKB_COMPOSE_NOTHING:
        default:
            return sym;
    }
}

LINEARWINbool _linearwinInputTextWayland(_LINEARWINwindow* window, uint32_t scancode)
{
    const xkb_keysym_t* keysyms;
    const xkb_keycode_t keycode = scancode + 8;

    if (xkb_state_key_get_syms(_linearwin.wl.xkb.state, keycode, &keysyms) == 1)
    {
        const xkb_keysym_t keysym = composeSymbol(keysyms[0]);
        const uint32_t codepoint = _linearwinKeySym2Unicode(keysym);
        if (codepoint != LINEARWIN_INVALID_CODEPOINT)
        {
            const int mods = _linearwin.wl.xkb.modifiers;
            const int plain = !(mods & (LINEARWIN_MOD_CONTROL | LINEARWIN_MOD_ALT));
            _linearwinInputChar(window, codepoint, mods, plain);
        }
    }

    return xkb_keymap_key_repeats(_linearwin.wl.xkb.keymap, keycode);
}

static void keyboardHandleKey(void* userData,
                              struct wl_keyboard* keyboard,
                              uint32_t serial,
                              uint32_t time,
                              uint32_t scancode,
                              uint32_t state)
{
    _LINEARWINwindow* window = _linearwin.wl.keyboardFocus;
    if (!window)
        return;

    const int key = translateKey(scancode);
    const int action =
        state == WL_KEYBOARD_KEY_STATE_PRESSED ? LINEARWIN_PRESS : LINEARWIN_RELEASE;

    _linearwin.wl.serial = serial;
    _linearwinInputKey(window, key, scancode, action, _linearwin.wl.xkb.modifiers);

    struct itimerspec timer = {};

    if (action == LINEARWIN_PRESS)
    {
        const LINEARWINbool shouldRepeat = _linearwinInputTextWayland(window, scancode);

        if (shouldRepeat && _linearwin.wl.keyboardRepeatRate > 0)
        {
            _linearwin.wl.keyboardLastKey = key;
            _linearwin.wl.keyboardLastScancode = scancode;
            if (_linearwin.wl.keyboardRepeatRate > 1)
                timer.it_interval.tv_nsec = 1000000000 / _linearwin.wl.keyboardRepeatRate;
            else
                timer.it_interval.tv_sec = 1;

            timer.it_value.tv_sec = _linearwin.wl.keyboardRepeatDelay / 1000;
            timer.it_value.tv_nsec = (_linearwin.wl.keyboardRepeatDelay % 1000) * 1000000;
        }
    }

    timerfd_settime(_linearwin.wl.timerfd, 0, &timer, NULL);
}

static void keyboardHandleModifiers(void* userData,
                                    struct wl_keyboard* keyboard,
                                    uint32_t serial,
                                    uint32_t modsDepressed,
                                    uint32_t modsLatched,
                                    uint32_t modsLocked,
                                    uint32_t group)
{
    _linearwin.wl.serial = serial;

    if (!_linearwin.wl.xkb.keymap)
        return;

    xkb_state_update_mask(_linearwin.wl.xkb.state,
                          modsDepressed,
                          modsLatched,
                          modsLocked,
                          0,
                          0,
                          group);

    _linearwin.wl.xkb.modifiers = 0;

    struct
    {
        xkb_mod_index_t index;
        unsigned int bit;
    } modifiers[] =
    {
        { _linearwin.wl.xkb.controlIndex,  LINEARWIN_MOD_CONTROL },
        { _linearwin.wl.xkb.altIndex,      LINEARWIN_MOD_ALT },
        { _linearwin.wl.xkb.shiftIndex,    LINEARWIN_MOD_SHIFT },
        { _linearwin.wl.xkb.superIndex,    LINEARWIN_MOD_SUPER },
        { _linearwin.wl.xkb.capsLockIndex, LINEARWIN_MOD_CAPS_LOCK },
        { _linearwin.wl.xkb.numLockIndex,  LINEARWIN_MOD_NUM_LOCK }
    };

    for (size_t i = 0; i < sizeof(modifiers) / sizeof(modifiers[0]); i++)
    {
        if (xkb_state_mod_index_is_active(_linearwin.wl.xkb.state,
                                          modifiers[i].index,
                                          XKB_STATE_MODS_EFFECTIVE) == 1)
        {
            _linearwin.wl.xkb.modifiers |= modifiers[i].bit;
        }
    }
}

#ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
static void keyboardHandleRepeatInfo(void* userData,
                                     struct wl_keyboard* keyboard,
                                     int32_t rate,
                                     int32_t delay)
{
    if (keyboard != _linearwin.wl.keyboard)
        return;

    _linearwin.wl.keyboardRepeatRate = rate;
    _linearwin.wl.keyboardRepeatDelay = delay;
}
#endif

static const struct wl_keyboard_listener keyboardListener =
{
    keyboardHandleKeymap,
    keyboardHandleEnter,
    keyboardHandleLeave,
    keyboardHandleKey,
    keyboardHandleModifiers,
#ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
    keyboardHandleRepeatInfo,
#endif
};

static void seatHandleCapabilities(void* userData,
                                   struct wl_seat* seat,
                                   enum wl_seat_capability caps)
{
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !_linearwin.wl.pointer)
    {
        _linearwin.wl.pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(_linearwin.wl.pointer, &pointerListener, NULL);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && _linearwin.wl.pointer)
    {
        wl_pointer_destroy(_linearwin.wl.pointer);
        _linearwin.wl.pointer = NULL;
    }

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !_linearwin.wl.keyboard)
    {
        _linearwin.wl.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(_linearwin.wl.keyboard, &keyboardListener, NULL);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && _linearwin.wl.keyboard)
    {
        wl_keyboard_destroy(_linearwin.wl.keyboard);
        _linearwin.wl.keyboard = NULL;
    }
}

static void seatHandleName(void* userData,
                           struct wl_seat* seat,
                           const char* name)
{
}

static const struct wl_seat_listener seatListener =
{
    seatHandleCapabilities,
    seatHandleName,
};

static void dataOfferHandleOffer(void* userData,
                                 struct wl_data_offer* offer,
                                 const char* mimeType)
{
    for (unsigned int i = 0; i < _linearwin.wl.offerCount; i++)
    {
        if (_linearwin.wl.offers[i].offer == offer)
        {
            if (strcmp(mimeType, "text/plain;charset=utf-8") == 0)
                _linearwin.wl.offers[i].text_plain_utf8 = LINEARWIN_TRUE;
            else if (strcmp(mimeType, "text/uri-list") == 0)
                _linearwin.wl.offers[i].text_uri_list = LINEARWIN_TRUE;

            break;
        }
    }
}

static const struct wl_data_offer_listener dataOfferListener =
{
    dataOfferHandleOffer
};

static void dataDeviceHandleDataOffer(void* userData,
                                      struct wl_data_device* device,
                                      struct wl_data_offer* offer)
{
    _LINEARWINofferWayland* offers =
        realloc(_linearwin.wl.offers, _linearwin.wl.offerCount + 1);
    if (!offers)
    {
        _linearwinInputError(LINEARWIN_OUT_OF_MEMORY, NULL);
        return;
    }

    _linearwin.wl.offers = offers;
    _linearwin.wl.offerCount++;

    _linearwin.wl.offers[_linearwin.wl.offerCount - 1] = (_LINEARWINofferWayland) { offer };
    wl_data_offer_add_listener(offer, &dataOfferListener, NULL);
}

static void dataDeviceHandleEnter(void* userData,
                                  struct wl_data_device* device,
                                  uint32_t serial,
                                  struct wl_surface* surface,
                                  wl_fixed_t x,
                                  wl_fixed_t y,
                                  struct wl_data_offer* offer)
{
    if (_linearwin.wl.dragOffer)
    {
        wl_data_offer_destroy(_linearwin.wl.dragOffer);
        _linearwin.wl.dragOffer = NULL;
        _linearwin.wl.dragFocus = NULL;
    }

    for (unsigned int i = 0; i < _linearwin.wl.offerCount; i++)
    {
        if (_linearwin.wl.offers[i].offer == offer)
        {
            _LINEARWINwindow* window = NULL;

            if (surface)
                window = wl_surface_get_user_data(surface);

            if (window && _linearwin.wl.offers[i].text_uri_list)
            {
                _linearwin.wl.dragOffer = offer;
                _linearwin.wl.dragFocus = window;
                _linearwin.wl.dragSerial = serial;
            }

            _linearwin.wl.offers[i] = _linearwin.wl.offers[_linearwin.wl.offerCount - 1];
            _linearwin.wl.offerCount--;
            break;
        }
    }

    if (_linearwin.wl.dragOffer)
        wl_data_offer_accept(offer, serial, "text/uri-list");
    else
    {
        wl_data_offer_accept(offer, serial, NULL);
        wl_data_offer_destroy(offer);
    }
}

static void dataDeviceHandleLeave(void* userData,
                                  struct wl_data_device* device)
{
    if (_linearwin.wl.dragOffer)
    {
        wl_data_offer_destroy(_linearwin.wl.dragOffer);
        _linearwin.wl.dragOffer = NULL;
        _linearwin.wl.dragFocus = NULL;
    }
}

static void dataDeviceHandleMotion(void* userData,
                                   struct wl_data_device* device,
                                   uint32_t time,
                                   wl_fixed_t x,
                                   wl_fixed_t y)
{
}

static void dataDeviceHandleDrop(void* userData,
                                 struct wl_data_device* device)
{
    if (!_linearwin.wl.dragOffer)
        return;

    char* string = readDataOfferAsString(_linearwin.wl.dragOffer, "text/uri-list");
    if (string)
    {
        int count;
        char** paths = _linearwinParseUriList(string, &count);
        if (paths)
            _linearwinInputDrop(_linearwin.wl.dragFocus, count, (const char**) paths);

        for (int i = 0; i < count; i++)
            free(paths[i]);

        free(paths);
    }

    free(string);
}

static void dataDeviceHandleSelection(void* userData,
                                      struct wl_data_device* device,
                                      struct wl_data_offer* offer)
{
    if (_linearwin.wl.selectionOffer)
    {
        wl_data_offer_destroy(_linearwin.wl.selectionOffer);
        _linearwin.wl.selectionOffer = NULL;
    }

    for (unsigned int i = 0; i < _linearwin.wl.offerCount; i++)
    {
        if (_linearwin.wl.offers[i].offer == offer)
        {
            if (_linearwin.wl.offers[i].text_plain_utf8)
                _linearwin.wl.selectionOffer = offer;
            else
                wl_data_offer_destroy(offer);

            _linearwin.wl.offers[i] = _linearwin.wl.offers[_linearwin.wl.offerCount - 1];
            _linearwin.wl.offerCount--;
            break;
        }
    }
}

const struct wl_data_device_listener dataDeviceListener =
{
    dataDeviceHandleDataOffer,
    dataDeviceHandleEnter,
    dataDeviceHandleLeave,
    dataDeviceHandleMotion,
    dataDeviceHandleDrop,
    dataDeviceHandleSelection,
};

// Translates a LINEARWIN standard cursor to a theme cursor name
//
static char *translateCursorShape(int shape)
{
    switch (shape)
    {
        case LINEARWIN_ARROW_CURSOR:
            return "left_ptr";
        case LINEARWIN_IBEAM_CURSOR:
            return "xterm";
        case LINEARWIN_CROSSHAIR_CURSOR:
            return "crosshair";
        case LINEARWIN_HAND_CURSOR:
            return "hand2";
        case LINEARWIN_HRESIZE_CURSOR:
            return "sb_h_double_arrow";
        case LINEARWIN_VRESIZE_CURSOR:
            return "sb_v_double_arrow";
    }
    return NULL;
}

void _linearwinAddSeatListenerWayland(struct wl_seat* seat)
{
    wl_seat_add_listener(seat, &seatListener, NULL);
}

void _linearwinAddDataDeviceListenerWayland(struct wl_data_device* device)
{
    wl_data_device_add_listener(device, &dataDeviceListener, NULL);
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _linearwinPlatformCreateWindow(_LINEARWINwindow* window,
                              const _LINEARWINwndconfig* wndconfig,
                              const _LINEARWINctxconfig* ctxconfig,
                              const _LINEARWINfbconfig* fbconfig)
{
    if (!createNativeSurface(window, wndconfig, fbconfig))
        return LINEARWIN_FALSE;

    if (ctxconfig->client != LINEARWIN_NO_API)
    {
        if (ctxconfig->source == LINEARWIN_EGL_CONTEXT_API ||
            ctxconfig->source == LINEARWIN_NATIVE_CONTEXT_API)
        {
            window->wl.egl.window = wl_egl_window_create(window->wl.surface,
                                                         wndconfig->width,
                                                         wndconfig->height);
            if (!window->wl.egl.window)
            {
                _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                                "Wayland: Failed to create EGL window");
                return LINEARWIN_FALSE;
            }

            if (!_linearwinInitEGL())
                return LINEARWIN_FALSE;
            if (!_linearwinCreateContextEGL(window, ctxconfig, fbconfig))
                return LINEARWIN_FALSE;
        }
        else if (ctxconfig->source == LINEARWIN_OSMESA_CONTEXT_API)
        {
            if (!_linearwinInitOSMesa())
                return LINEARWIN_FALSE;
            if (!_linearwinCreateContextOSMesa(window, ctxconfig, fbconfig))
                return LINEARWIN_FALSE;
        }

        if (!_linearwinRefreshContextAttribs(window, ctxconfig))
            return LINEARWIN_FALSE;
    }

    if (window->monitor || wndconfig->visible)
    {
        if (!createShellObjects(window))
            return LINEARWIN_FALSE;
    }

    return LINEARWIN_TRUE;
}

void _linearwinPlatformDestroyWindow(_LINEARWINwindow* window)
{
    if (window == _linearwin.wl.pointerFocus)
    {
        _linearwin.wl.pointerFocus = NULL;
        _linearwinInputCursorEnter(window, LINEARWIN_FALSE);
    }
    if (window == _linearwin.wl.keyboardFocus)
    {
        _linearwin.wl.keyboardFocus = NULL;
        _linearwinInputWindowFocus(window, LINEARWIN_FALSE);
    }

    if (window->wl.idleInhibitor)
        zwp_idle_inhibitor_v1_destroy(window->wl.idleInhibitor);

    if (window->context.destroy)
        window->context.destroy(window);

    destroyShellObjects(window);

    if (window->wl.decorations.buffer)
        wl_buffer_destroy(window->wl.decorations.buffer);

    if (window->wl.egl.window)
        wl_egl_window_destroy(window->wl.egl.window);

    if (window->wl.surface)
        wl_surface_destroy(window->wl.surface);

    free(window->wl.title);
    free(window->wl.monitors);
}

void _linearwinPlatformSetWindowTitle(_LINEARWINwindow* window, const char* title)
{
    if (window->wl.title)
        free(window->wl.title);
    window->wl.title = _linearwin_strdup(title);
    if (window->wl.xdg.toplevel)
        xdg_toplevel_set_title(window->wl.xdg.toplevel, title);
}

void _linearwinPlatformSetWindowIcon(_LINEARWINwindow* window,
                                int count, const LINEARWINimage* images)
{
    _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                    "Wayland: Setting window icon not supported");
}

void _linearwinPlatformGetWindowPos(_LINEARWINwindow* window, int* xpos, int* ypos)
{
    // A Wayland client is not aware of its position, so just warn and leave it
    // as (0, 0)

    _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                    "Wayland: Window position retrieval not supported");
}

void _linearwinPlatformSetWindowPos(_LINEARWINwindow* window, int xpos, int ypos)
{
    // A Wayland client can not set its position, so just warn

    _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                    "Wayland: Window position setting not supported");
}

void _linearwinPlatformGetWindowSize(_LINEARWINwindow* window, int* width, int* height)
{
    if (width)
        *width = window->wl.width;
    if (height)
        *height = window->wl.height;
}

void _linearwinPlatformSetWindowSize(_LINEARWINwindow* window, int width, int height)
{
    if (window->monitor)
    {
        // Video mode setting is not available on Wayland
    }
    else
    {
        window->wl.width = width;
        window->wl.height = height;
        resizeWindow(window);
    }
}

void _linearwinPlatformSetWindowSizeLimits(_LINEARWINwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight)
{
    if (window->wl.xdg.toplevel)
    {
        if (minwidth == LINEARWIN_DONT_CARE || minheight == LINEARWIN_DONT_CARE)
            minwidth = minheight = 0;
        else
        {
            if (window->wl.decorations.top.surface)
            {
                minwidth  += LINEARWIN_BORDER_SIZE * 2;
                minheight += LINEARWIN_CAPTION_HEIGHT + LINEARWIN_BORDER_SIZE;
            }
        }

        if (maxwidth == LINEARWIN_DONT_CARE || maxheight == LINEARWIN_DONT_CARE)
            maxwidth = maxheight = 0;
        else
        {
            if (window->wl.decorations.top.surface)
            {
                maxwidth  += LINEARWIN_BORDER_SIZE * 2;
                maxheight += LINEARWIN_CAPTION_HEIGHT + LINEARWIN_BORDER_SIZE;
            }
        }

        xdg_toplevel_set_min_size(window->wl.xdg.toplevel, minwidth, minheight);
        xdg_toplevel_set_max_size(window->wl.xdg.toplevel, maxwidth, maxheight);
        wl_surface_commit(window->wl.surface);
    }
}

void _linearwinPlatformSetWindowAspectRatio(_LINEARWINwindow* window,
                                       int numer, int denom)
{
    if (window->wl.maximized || window->wl.fullscreen)
        return;

    if (numer != LINEARWIN_DONT_CARE && denom != LINEARWIN_DONT_CARE)
    {
        const float aspectRatio = (float) window->wl.width / (float) window->wl.height;
        const float targetRatio = (float) numer / (float) denom;
        if (aspectRatio < targetRatio)
            window->wl.height = window->wl.width / targetRatio;
        else if (aspectRatio > targetRatio)
            window->wl.width = window->wl.height * targetRatio;

        resizeWindow(window);
    }
}

void _linearwinPlatformGetFramebufferSize(_LINEARWINwindow* window,
                                     int* width, int* height)
{
    _linearwinPlatformGetWindowSize(window, width, height);
    if (width)
        *width *= window->wl.scale;
    if (height)
        *height *= window->wl.scale;
}

void _linearwinPlatformGetWindowFrameSize(_LINEARWINwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom)
{
    if (window->decorated && !window->monitor && window->wl.decorations.top.surface)
    {
        if (top)
            *top = LINEARWIN_CAPTION_HEIGHT;
        if (left)
            *left = LINEARWIN_BORDER_SIZE;
        if (right)
            *right = LINEARWIN_BORDER_SIZE;
        if (bottom)
            *bottom = LINEARWIN_BORDER_SIZE;
    }
}

void _linearwinPlatformGetWindowContentScale(_LINEARWINwindow* window,
                                        float* xscale, float* yscale)
{
    if (xscale)
        *xscale = (float) window->wl.scale;
    if (yscale)
        *yscale = (float) window->wl.scale;
}

void _linearwinPlatformIconifyWindow(_LINEARWINwindow* window)
{
    if (window->wl.xdg.toplevel)
        xdg_toplevel_set_minimized(window->wl.xdg.toplevel);
}

void _linearwinPlatformRestoreWindow(_LINEARWINwindow* window)
{
    if (window->monitor)
    {
        // There is no way to unset minimized, or even to know if we are
        // minimized, so there is nothing to do here.
    }
    else
    {
        // We assume we are not minimized and act only on maximization

        if (window->wl.maximized)
        {
            if (window->wl.xdg.toplevel)
                xdg_toplevel_unset_maximized(window->wl.xdg.toplevel);
            else
                window->wl.maximized = LINEARWIN_FALSE;
        }
    }
}

void _linearwinPlatformMaximizeWindow(_LINEARWINwindow* window)
{
    if (window->wl.xdg.toplevel)
        xdg_toplevel_set_maximized(window->wl.xdg.toplevel);
    else
        window->wl.maximized = LINEARWIN_TRUE;
}

void _linearwinPlatformShowWindow(_LINEARWINwindow* window)
{
    if (!window->wl.xdg.toplevel)
    {
        // NOTE: The XDG/shell surface is created here so command-line applications
        //       with off-screen windows do not appear in for example the Unity dock
        createShellObjects(window);
    }
}

void _linearwinPlatformHideWindow(_LINEARWINwindow* window)
{
    if (window->wl.visible)
    {
        window->wl.visible = LINEARWIN_FALSE;
        destroyShellObjects(window);

        wl_surface_attach(window->wl.surface, NULL, 0, 0);
        wl_surface_commit(window->wl.surface);
    }
}

void _linearwinPlatformRequestWindowAttention(_LINEARWINwindow* window)
{
    // TODO
    _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                    "Wayland: Window attention request not implemented yet");
}

void _linearwinPlatformFocusWindow(_LINEARWINwindow* window)
{
    _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                    "Wayland: Focusing a window requires user interaction");
}

void _linearwinPlatformSetWindowMonitor(_LINEARWINwindow* window,
                                   _LINEARWINmonitor* monitor,
                                   int xpos, int ypos,
                                   int width, int height,
                                   int refreshRate)
{
    if (window->monitor == monitor)
    {
        if (!monitor)
            _linearwinPlatformSetWindowSize(window, width, height);

        return;
    }

    if (window->monitor)
        releaseMonitor(window);

    _linearwinInputWindowMonitor(window, monitor);

    if (window->monitor)
        acquireMonitor(window);
    else
        _linearwinPlatformSetWindowSize(window, width, height);
}

int _linearwinPlatformWindowFocused(_LINEARWINwindow* window)
{
    return _linearwin.wl.keyboardFocus == window;
}

int _linearwinPlatformWindowIconified(_LINEARWINwindow* window)
{
    // xdg-shell doesn’t give any way to request whether a surface is iconified
    return LINEARWIN_FALSE;
}

int _linearwinPlatformWindowVisible(_LINEARWINwindow* window)
{
    return window->wl.visible;
}

int _linearwinPlatformWindowMaximized(_LINEARWINwindow* window)
{
    return window->wl.maximized;
}

int _linearwinPlatformWindowHovered(_LINEARWINwindow* window)
{
    return window->wl.hovered;
}

int _linearwinPlatformFramebufferTransparent(_LINEARWINwindow* window)
{
    return window->wl.transparent;
}

void _linearwinPlatformSetWindowResizable(_LINEARWINwindow* window, LINEARWINbool enabled)
{
    // TODO
    _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                    "Wayland: Window attribute setting not implemented yet");
}

void _linearwinPlatformSetWindowDecorated(_LINEARWINwindow* window, LINEARWINbool enabled)
{
    if (window->wl.xdg.decoration)
    {
        uint32_t mode;

        if (enabled)
            mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
        else
            mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;

        zxdg_toplevel_decoration_v1_set_mode(window->wl.xdg.decoration, mode);
    }
    else
    {
        if (enabled)
            createFallbackDecorations(window);
        else
            destroyFallbackDecorations(window);
    }
}

void _linearwinPlatformSetWindowFloating(_LINEARWINwindow* window, LINEARWINbool enabled)
{
    // TODO
    _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                    "Wayland: Window attribute setting not implemented yet");
}

float _linearwinPlatformGetWindowOpacity(_LINEARWINwindow* window)
{
    return 1.f;
}

void _linearwinPlatformSetWindowOpacity(_LINEARWINwindow* window, float opacity)
{
}

void _linearwinPlatformSetRawMouseMotion(_LINEARWINwindow* window, LINEARWINbool enabled)
{
    // This is handled in relativePointerHandleRelativeMotion
}

LINEARWINbool _linearwinPlatformRawMouseMotionSupported(void)
{
    return LINEARWIN_TRUE;
}

void _linearwinPlatformPollEvents(void)
{
    double timeout = 0.0;
    handleEvents(&timeout);
}

void _linearwinPlatformWaitEvents(void)
{
    handleEvents(NULL);
}

void _linearwinPlatformWaitEventsTimeout(double timeout)
{
    handleEvents(&timeout);
}

void _linearwinPlatformPostEmptyEvent(void)
{
    wl_display_sync(_linearwin.wl.display);
    flushDisplay();
}

void _linearwinPlatformGetCursorPos(_LINEARWINwindow* window, double* xpos, double* ypos)
{
    if (xpos)
        *xpos = window->wl.cursorPosX;
    if (ypos)
        *ypos = window->wl.cursorPosY;
}

static LINEARWINbool isPointerLocked(_LINEARWINwindow* window);

void _linearwinPlatformSetCursorPos(_LINEARWINwindow* window, double x, double y)
{
    if (isPointerLocked(window))
    {
        zwp_locked_pointer_v1_set_cursor_position_hint(
            window->wl.pointerLock.lockedPointer,
            wl_fixed_from_double(x), wl_fixed_from_double(y));
    }
}

void _linearwinPlatformSetCursorMode(_LINEARWINwindow* window, int mode)
{
    _linearwinPlatformSetCursor(window, window->wl.currentCursor);
}

const char* _linearwinPlatformGetScancodeName(int scancode)
{
    if (scancode < 0 || scancode > 255 ||
        _linearwin.wl.keycodes[scancode] == LINEARWIN_KEY_UNKNOWN)
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE,
                        "Wayland: Invalid scancode %i",
                        scancode);
        return NULL;
    }

    const int key = _linearwin.wl.keycodes[scancode];
    const xkb_keycode_t keycode = scancode + 8;
    const xkb_layout_index_t layout =
        xkb_state_key_get_layout(_linearwin.wl.xkb.state, keycode);
    if (layout == XKB_LAYOUT_INVALID)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to retrieve layout for key name");
        return NULL;
    }

    const xkb_keysym_t* keysyms = NULL;
    xkb_keymap_key_get_syms_by_level(_linearwin.wl.xkb.keymap,
                                     keycode,
                                     layout,
                                     0,
                                     &keysyms);
    if (keysyms == NULL)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to retrieve keysym for key name");
        return NULL;
    }

    const uint32_t codepoint = _linearwinKeySym2Unicode(keysyms[0]);
    if (codepoint == LINEARWIN_INVALID_CODEPOINT)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to retrieve codepoint for key name");
        return NULL;
    }

    const size_t count = _linearwinEncodeUTF8(_linearwin.wl.keynames[key],  codepoint);
    if (count == 0)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to encode codepoint for key name");
        return NULL;
    }

    _linearwin.wl.keynames[key][count] = '\0';
    return _linearwin.wl.keynames[key];
}

int _linearwinPlatformGetKeyScancode(int key)
{
    return _linearwin.wl.scancodes[key];
}

int _linearwinPlatformCreateCursor(_LINEARWINcursor* cursor,
                              const LINEARWINimage* image,
                              int xhot, int yhot)
{
    cursor->wl.buffer = createShmBuffer(image);
    if (!cursor->wl.buffer)
        return LINEARWIN_FALSE;

    cursor->wl.width = image->width;
    cursor->wl.height = image->height;
    cursor->wl.xhot = xhot;
    cursor->wl.yhot = yhot;
    return LINEARWIN_TRUE;
}

int _linearwinPlatformCreateStandardCursor(_LINEARWINcursor* cursor, int shape)
{
    struct wl_cursor* standardCursor;

    standardCursor = wl_cursor_theme_get_cursor(_linearwin.wl.cursorTheme,
                                                translateCursorShape(shape));
    if (!standardCursor)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Standard cursor \"%s\" not found",
                        translateCursorShape(shape));
        return LINEARWIN_FALSE;
    }

    cursor->wl.cursor = standardCursor;
    cursor->wl.currentImage = 0;

    if (_linearwin.wl.cursorThemeHiDPI)
    {
        standardCursor = wl_cursor_theme_get_cursor(_linearwin.wl.cursorThemeHiDPI,
                                                    translateCursorShape(shape));
        cursor->wl.cursorHiDPI = standardCursor;
    }

    return LINEARWIN_TRUE;
}

void _linearwinPlatformDestroyCursor(_LINEARWINcursor* cursor)
{
    // If it's a standard cursor we don't need to do anything here
    if (cursor->wl.cursor)
        return;

    if (cursor->wl.buffer)
        wl_buffer_destroy(cursor->wl.buffer);
}

static void relativePointerHandleRelativeMotion(void* userData,
                                                struct zwp_relative_pointer_v1* pointer,
                                                uint32_t timeHi,
                                                uint32_t timeLo,
                                                wl_fixed_t dx,
                                                wl_fixed_t dy,
                                                wl_fixed_t dxUnaccel,
                                                wl_fixed_t dyUnaccel)
{
    _LINEARWINwindow* window = userData;
    double xpos = window->virtualCursorPosX;
    double ypos = window->virtualCursorPosY;

    if (window->cursorMode != LINEARWIN_CURSOR_DISABLED)
        return;

    if (window->rawMouseMotion)
    {
        xpos += wl_fixed_to_double(dxUnaccel);
        ypos += wl_fixed_to_double(dyUnaccel);
    }
    else
    {
        xpos += wl_fixed_to_double(dx);
        ypos += wl_fixed_to_double(dy);
    }

    _linearwinInputCursorPos(window, xpos, ypos);
}

static const struct zwp_relative_pointer_v1_listener relativePointerListener =
{
    relativePointerHandleRelativeMotion
};

static void lockedPointerHandleLocked(void* userData,
                                      struct zwp_locked_pointer_v1* lockedPointer)
{
}

static void unlockPointer(_LINEARWINwindow* window)
{
    struct zwp_relative_pointer_v1* relativePointer =
        window->wl.pointerLock.relativePointer;
    struct zwp_locked_pointer_v1* lockedPointer =
        window->wl.pointerLock.lockedPointer;

    zwp_relative_pointer_v1_destroy(relativePointer);
    zwp_locked_pointer_v1_destroy(lockedPointer);

    window->wl.pointerLock.relativePointer = NULL;
    window->wl.pointerLock.lockedPointer = NULL;
}

static void lockPointer(_LINEARWINwindow* window);

static void lockedPointerHandleUnlocked(void* userData,
                                        struct zwp_locked_pointer_v1* lockedPointer)
{
}

static const struct zwp_locked_pointer_v1_listener lockedPointerListener =
{
    lockedPointerHandleLocked,
    lockedPointerHandleUnlocked
};

static void lockPointer(_LINEARWINwindow* window)
{
    struct zwp_relative_pointer_v1* relativePointer;
    struct zwp_locked_pointer_v1* lockedPointer;

    if (!_linearwin.wl.relativePointerManager)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: no relative pointer manager");
        return;
    }

    relativePointer =
        zwp_relative_pointer_manager_v1_get_relative_pointer(
            _linearwin.wl.relativePointerManager,
            _linearwin.wl.pointer);
    zwp_relative_pointer_v1_add_listener(relativePointer,
                                         &relativePointerListener,
                                         window);

    lockedPointer =
        zwp_pointer_constraints_v1_lock_pointer(
            _linearwin.wl.pointerConstraints,
            window->wl.surface,
            _linearwin.wl.pointer,
            NULL,
            ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
    zwp_locked_pointer_v1_add_listener(lockedPointer,
                                       &lockedPointerListener,
                                       window);

    window->wl.pointerLock.relativePointer = relativePointer;
    window->wl.pointerLock.lockedPointer = lockedPointer;

    wl_pointer_set_cursor(_linearwin.wl.pointer, _linearwin.wl.pointerEnterSerial,
                          NULL, 0, 0);
}

static LINEARWINbool isPointerLocked(_LINEARWINwindow* window)
{
    return window->wl.pointerLock.lockedPointer != NULL;
}

void _linearwinPlatformSetCursor(_LINEARWINwindow* window, _LINEARWINcursor* cursor)
{
    struct wl_cursor* defaultCursor;
    struct wl_cursor* defaultCursorHiDPI = NULL;

    if (!_linearwin.wl.pointer)
        return;

    window->wl.currentCursor = cursor;

    // If we're not in the correct window just save the cursor
    // the next time the pointer enters the window the cursor will change
    if (window != _linearwin.wl.pointerFocus || window->wl.decorations.focus != mainWindow)
        return;

    // Unlock possible pointer lock if no longer disabled.
    if (window->cursorMode != LINEARWIN_CURSOR_DISABLED && isPointerLocked(window))
        unlockPointer(window);

    if (window->cursorMode == LINEARWIN_CURSOR_NORMAL)
    {
        if (cursor)
            setCursorImage(window, &cursor->wl);
        else
        {
            defaultCursor = wl_cursor_theme_get_cursor(_linearwin.wl.cursorTheme,
                                                       "left_ptr");
            if (!defaultCursor)
            {
                _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                                "Wayland: Standard cursor not found");
                return;
            }
            if (_linearwin.wl.cursorThemeHiDPI)
                defaultCursorHiDPI =
                    wl_cursor_theme_get_cursor(_linearwin.wl.cursorThemeHiDPI,
                                               "left_ptr");
            _LINEARWINcursorWayland cursorWayland = {
                defaultCursor,
                defaultCursorHiDPI,
                NULL,
                0, 0,
                0, 0,
                0
            };
            setCursorImage(window, &cursorWayland);
        }
    }
    else if (window->cursorMode == LINEARWIN_CURSOR_DISABLED)
    {
        if (!isPointerLocked(window))
            lockPointer(window);
    }
    else if (window->cursorMode == LINEARWIN_CURSOR_HIDDEN)
    {
        wl_pointer_set_cursor(_linearwin.wl.pointer, _linearwin.wl.pointerEnterSerial, NULL, 0, 0);
    }
}

static void dataSourceHandleTarget(void* userData,
                                   struct wl_data_source* source,
                                   const char* mimeType)
{
    if (_linearwin.wl.selectionSource != source)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Unknown clipboard data source");
        return;
    }
}

static void dataSourceHandleSend(void* userData,
                                 struct wl_data_source* source,
                                 const char* mimeType,
                                 int fd)
{
    // Ignore it if this is an outdated or invalid request
    if (_linearwin.wl.selectionSource != source ||
        strcmp(mimeType, "text/plain;charset=utf-8") != 0)
    {
        close(fd);
        return;
    }

    char* string = _linearwin.wl.clipboardString;
    size_t length = strlen(string);

    while (length > 0)
    {
        const ssize_t result = write(fd, string, length);
        if (result == -1)
        {
            if (errno == EINTR)
                continue;

            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "Wayland: Error while writing the clipboard: %s",
                            strerror(errno));
            break;
        }

        length -= result;
        string += result;
    }

    close(fd);
}

static void dataSourceHandleCancelled(void* userData,
                                      struct wl_data_source* source)
{
    wl_data_source_destroy(source);

    if (_linearwin.wl.selectionSource != source)
        return;

    _linearwin.wl.selectionSource = NULL;
}

static const struct wl_data_source_listener dataSourceListener =
{
    dataSourceHandleTarget,
    dataSourceHandleSend,
    dataSourceHandleCancelled,
};

void _linearwinPlatformSetClipboardString(const char* string)
{
    if (_linearwin.wl.selectionSource)
    {
        wl_data_source_destroy(_linearwin.wl.selectionSource);
        _linearwin.wl.selectionSource = NULL;
    }

    char* copy = _linearwin_strdup(string);
    if (!copy)
    {
        _linearwinInputError(LINEARWIN_OUT_OF_MEMORY, NULL);
        return;
    }

    free(_linearwin.wl.clipboardString);
    _linearwin.wl.clipboardString = copy;

    _linearwin.wl.selectionSource =
        wl_data_device_manager_create_data_source(_linearwin.wl.dataDeviceManager);
    if (!_linearwin.wl.selectionSource)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to create clipboard data source");
        return;
    }
    wl_data_source_add_listener(_linearwin.wl.selectionSource,
                                &dataSourceListener,
                                NULL);
    wl_data_source_offer(_linearwin.wl.selectionSource, "text/plain;charset=utf-8");
    wl_data_device_set_selection(_linearwin.wl.dataDevice,
                                 _linearwin.wl.selectionSource,
                                 _linearwin.wl.serial);
}

const char* _linearwinPlatformGetClipboardString(void)
{
    if (!_linearwin.wl.selectionOffer)
    {
        _linearwinInputError(LINEARWIN_FORMAT_UNAVAILABLE,
                        "Wayland: No clipboard data available");
        return NULL;
    }

    if (_linearwin.wl.selectionSource)
        return _linearwin.wl.clipboardString;

    free(_linearwin.wl.clipboardString);
    _linearwin.wl.clipboardString =
        readDataOfferAsString(_linearwin.wl.selectionOffer, "text/plain;charset=utf-8");
    return _linearwin.wl.clipboardString;
}

void _linearwinPlatformGetRequiredInstanceExtensions(char** extensions)
{
    if (!_linearwin.vk.KHR_surface || !_linearwin.vk.KHR_wayland_surface)
        return;

    extensions[0] = "VK_KHR_surface";
    extensions[1] = "VK_KHR_wayland_surface";
}

int _linearwinPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily)
{
    PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR
        vkGetPhysicalDeviceWaylandPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)
        vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
    if (!vkGetPhysicalDeviceWaylandPresentationSupportKHR)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                        "Wayland: Vulkan instance missing VK_KHR_wayland_surface extension");
        return VK_NULL_HANDLE;
    }

    return vkGetPhysicalDeviceWaylandPresentationSupportKHR(device,
                                                            queuefamily,
                                                            _linearwin.wl.display);
}

VkResult _linearwinPlatformCreateWindowSurface(VkInstance instance,
                                          _LINEARWINwindow* window,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR* surface)
{
    VkResult err;
    VkWaylandSurfaceCreateInfoKHR sci;
    PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;

    vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)
        vkGetInstanceProcAddr(instance, "vkCreateWaylandSurfaceKHR");
    if (!vkCreateWaylandSurfaceKHR)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                        "Wayland: Vulkan instance missing VK_KHR_wayland_surface extension");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    memset(&sci, 0, sizeof(sci));
    sci.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    sci.display = _linearwin.wl.display;
    sci.surface = window->wl.surface;

    err = vkCreateWaylandSurfaceKHR(instance, &sci, allocator, surface);
    if (err)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Wayland: Failed to create Vulkan surface: %s",
                        _linearwinGetVulkanResultString(err));
    }

    return err;
}


//////////////////////////////////////////////////////////////////////////
//////                        LINEARWIN native API                       //////
//////////////////////////////////////////////////////////////////////////

LINEARWINAPI struct wl_display* linearwinGetWaylandDisplay(void)
{
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return _linearwin.wl.display;
}

LINEARWINAPI struct wl_surface* linearwinGetWaylandWindow(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return window->wl.surface;
}

