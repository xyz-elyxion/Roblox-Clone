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

#define RBLXWIN_BORDER_SIZE    4
#define RBLXWIN_CAPTION_HEIGHT 24

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
    static const char template[] = "/rblxwin-shared-XXXXXX";
    const char* path;
    char* name;
    int fd;
    int ret;

#ifdef HAVE_MEMFD_CREATE
    fd = memfd_create("rblxwin-shared", MFD_CLOEXEC | MFD_ALLOW_SEALING);
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

static struct wl_buffer* createShmBuffer(const RBLXWINimage* image)
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
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to create buffer file of size %d: %s",
                        length, strerror(errno));
        return NULL;
    }

    data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to map file: %s", strerror(errno));
        close(fd);
        return NULL;
    }

    pool = wl_shm_create_pool(_rblxwin.wl.shm, fd, length);

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
static RBLXWINbool waitForData(struct pollfd* fds, nfds_t count, double* timeout)
{
    for (;;)
    {
        if (timeout)
        {
            const uint64_t base = _rblxwinPlatformGetTimerValue();

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

            *timeout -= (_rblxwinPlatformGetTimerValue() - base) /
                (double) _rblxwinPlatformGetTimerFrequency();

            if (result > 0)
                return RBLXWIN_TRUE;
            else if (result == -1 && error != EINTR && error != EAGAIN)
                return RBLXWIN_FALSE;
            else if (*timeout <= 0.0)
                return RBLXWIN_FALSE;
        }
        else
        {
            const int result = poll(fds, count, -1);
            if (result > 0)
                return RBLXWIN_TRUE;
            else if (result == -1 && errno != EINTR && errno != EAGAIN)
                return RBLXWIN_FALSE;
        }
    }
}

static void createFallbackDecoration(_RBLXWINdecorationWayland* decoration,
                                     struct wl_surface* parent,
                                     struct wl_buffer* buffer,
                                     int x, int y,
                                     int width, int height)
{
    decoration->surface = wl_compositor_create_surface(_rblxwin.wl.compositor);
    decoration->subsurface =
        wl_subcompositor_get_subsurface(_rblxwin.wl.subcompositor,
                                        decoration->surface, parent);
    wl_subsurface_set_position(decoration->subsurface, x, y);
    decoration->viewport = wp_viewporter_get_viewport(_rblxwin.wl.viewporter,
                                                      decoration->surface);
    wp_viewport_set_destination(decoration->viewport, width, height);
    wl_surface_attach(decoration->surface, buffer, 0, 0);

    struct wl_region* region = wl_compositor_create_region(_rblxwin.wl.compositor);
    wl_region_add(region, 0, 0, width, height);
    wl_surface_set_opaque_region(decoration->surface, region);
    wl_surface_commit(decoration->surface);
    wl_region_destroy(region);
}

static void createFallbackDecorations(_RBLXWINwindow* window)
{
    unsigned char data[] = { 224, 224, 224, 255 };
    const RBLXWINimage image = { 1, 1, data };

    if (!_rblxwin.wl.viewporter)
        return;

    if (!window->wl.decorations.buffer)
        window->wl.decorations.buffer = createShmBuffer(&image);
    if (!window->wl.decorations.buffer)
        return;

    createFallbackDecoration(&window->wl.decorations.top, window->wl.surface,
                             window->wl.decorations.buffer,
                             0, -RBLXWIN_CAPTION_HEIGHT,
                             window->wl.width, RBLXWIN_CAPTION_HEIGHT);
    createFallbackDecoration(&window->wl.decorations.left, window->wl.surface,
                             window->wl.decorations.buffer,
                             -RBLXWIN_BORDER_SIZE, -RBLXWIN_CAPTION_HEIGHT,
                             RBLXWIN_BORDER_SIZE, window->wl.height + RBLXWIN_CAPTION_HEIGHT);
    createFallbackDecoration(&window->wl.decorations.right, window->wl.surface,
                             window->wl.decorations.buffer,
                             window->wl.width, -RBLXWIN_CAPTION_HEIGHT,
                             RBLXWIN_BORDER_SIZE, window->wl.height + RBLXWIN_CAPTION_HEIGHT);
    createFallbackDecoration(&window->wl.decorations.bottom, window->wl.surface,
                             window->wl.decorations.buffer,
                             -RBLXWIN_BORDER_SIZE, window->wl.height,
                             window->wl.width + RBLXWIN_BORDER_SIZE * 2, RBLXWIN_BORDER_SIZE);
}

static void destroyFallbackDecoration(_RBLXWINdecorationWayland* decoration)
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

static void destroyFallbackDecorations(_RBLXWINwindow* window)
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
    _RBLXWINwindow* window = userData;

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
static void setContentAreaOpaque(_RBLXWINwindow* window)
{
    struct wl_region* region;

    region = wl_compositor_create_region(_rblxwin.wl.compositor);
    if (!region)
        return;

    wl_region_add(region, 0, 0, window->wl.width, window->wl.height);
    wl_surface_set_opaque_region(window->wl.surface, region);
    wl_region_destroy(region);
}


static void resizeWindow(_RBLXWINwindow* window)
{
    int scale = window->wl.scale;
    int scaledWidth = window->wl.width * scale;
    int scaledHeight = window->wl.height * scale;

    if (window->wl.egl.window)
        wl_egl_window_resize(window->wl.egl.window, scaledWidth, scaledHeight, 0, 0);
    if (!window->wl.transparent)
        setContentAreaOpaque(window);
    _rblxwinInputFramebufferSize(window, scaledWidth, scaledHeight);

    if (!window->wl.decorations.top.surface)
        return;

    wp_viewport_set_destination(window->wl.decorations.top.viewport,
                                window->wl.width, RBLXWIN_CAPTION_HEIGHT);
    wl_surface_commit(window->wl.decorations.top.surface);

    wp_viewport_set_destination(window->wl.decorations.left.viewport,
                                RBLXWIN_BORDER_SIZE, window->wl.height + RBLXWIN_CAPTION_HEIGHT);
    wl_surface_commit(window->wl.decorations.left.surface);

    wl_subsurface_set_position(window->wl.decorations.right.subsurface,
                               window->wl.width, -RBLXWIN_CAPTION_HEIGHT);
    wp_viewport_set_destination(window->wl.decorations.right.viewport,
                                RBLXWIN_BORDER_SIZE, window->wl.height + RBLXWIN_CAPTION_HEIGHT);
    wl_surface_commit(window->wl.decorations.right.surface);

    wl_subsurface_set_position(window->wl.decorations.bottom.subsurface,
                               -RBLXWIN_BORDER_SIZE, window->wl.height);
    wp_viewport_set_destination(window->wl.decorations.bottom.viewport,
                                window->wl.width + RBLXWIN_BORDER_SIZE * 2, RBLXWIN_BORDER_SIZE);
    wl_surface_commit(window->wl.decorations.bottom.surface);
}

void _rblxwinUpdateContentScaleWayland(_RBLXWINwindow* window)
{
    if (_rblxwin.wl.compositorVersion < WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION)
        return;

    // Get the scale factor from the highest scale monitor.
    int maxScale = 1;

    for (int i = 0; i < window->wl.monitorsCount; i++)
        maxScale = _rblxwin_max(window->wl.monitors[i]->wl.scale, maxScale);

    // Only change the framebuffer size if the scale changed.
    if (window->wl.scale != maxScale)
    {
        window->wl.scale = maxScale;
        wl_surface_set_buffer_scale(window->wl.surface, maxScale);
        _rblxwinInputWindowContentScale(window, maxScale, maxScale);
        resizeWindow(window);
    }
}

static void surfaceHandleEnter(void* userData,
                               struct wl_surface* surface,
                               struct wl_output* output)
{
    _RBLXWINwindow* window = userData;
    _RBLXWINmonitor* monitor = wl_output_get_user_data(output);

    if (window->wl.monitorsCount + 1 > window->wl.monitorsSize)
    {
        ++window->wl.monitorsSize;
        window->wl.monitors =
            realloc(window->wl.monitors,
                    window->wl.monitorsSize * sizeof(_RBLXWINmonitor*));
    }

    window->wl.monitors[window->wl.monitorsCount++] = monitor;

    _rblxwinUpdateContentScaleWayland(window);
}

static void surfaceHandleLeave(void* userData,
                               struct wl_surface* surface,
                               struct wl_output* output)
{
    _RBLXWINwindow* window = userData;
    _RBLXWINmonitor* monitor = wl_output_get_user_data(output);
    RBLXWINbool found;
    int i;

    for (i = 0, found = RBLXWIN_FALSE; i < window->wl.monitorsCount - 1; ++i)
    {
        if (monitor == window->wl.monitors[i])
            found = RBLXWIN_TRUE;
        if (found)
            window->wl.monitors[i] = window->wl.monitors[i + 1];
    }
    window->wl.monitors[--window->wl.monitorsCount] = NULL;

    _rblxwinUpdateContentScaleWayland(window);
}

static const struct wl_surface_listener surfaceListener =
{
    surfaceHandleEnter,
    surfaceHandleLeave
};

static void setIdleInhibitor(_RBLXWINwindow* window, RBLXWINbool enable)
{
    if (enable && !window->wl.idleInhibitor && _rblxwin.wl.idleInhibitManager)
    {
        window->wl.idleInhibitor =
            zwp_idle_inhibit_manager_v1_create_inhibitor(
                _rblxwin.wl.idleInhibitManager, window->wl.surface);
        if (!window->wl.idleInhibitor)
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
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
static void acquireMonitor(_RBLXWINwindow* window)
{
    if (window->wl.xdg.toplevel)
    {
        xdg_toplevel_set_fullscreen(window->wl.xdg.toplevel,
                                    window->monitor->wl.output);
    }

    setIdleInhibitor(window, RBLXWIN_TRUE);

    if (window->wl.decorations.top.surface)
        destroyFallbackDecorations(window);
}

// Remove the window and restore the original video mode
//
static void releaseMonitor(_RBLXWINwindow* window)
{
    if (window->wl.xdg.toplevel)
        xdg_toplevel_unset_fullscreen(window->wl.xdg.toplevel);

    setIdleInhibitor(window, RBLXWIN_FALSE);

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
    _RBLXWINwindow* window = userData;
    uint32_t* state;

    window->wl.pending.activated  = RBLXWIN_FALSE;
    window->wl.pending.maximized  = RBLXWIN_FALSE;
    window->wl.pending.fullscreen = RBLXWIN_FALSE;

    wl_array_for_each(state, states)
    {
        switch (*state)
        {
            case XDG_TOPLEVEL_STATE_MAXIMIZED:
                window->wl.pending.maximized = RBLXWIN_TRUE;
                break;
            case XDG_TOPLEVEL_STATE_FULLSCREEN:
                window->wl.pending.fullscreen = RBLXWIN_TRUE;
                break;
            case XDG_TOPLEVEL_STATE_RESIZING:
                break;
            case XDG_TOPLEVEL_STATE_ACTIVATED:
                window->wl.pending.activated = RBLXWIN_TRUE;
                break;
        }
    }

    if (width && height)
    {
        if (window->wl.decorations.top.surface)
        {
            window->wl.pending.width  = _rblxwin_max(0, width - RBLXWIN_BORDER_SIZE * 2);
            window->wl.pending.height =
                _rblxwin_max(0, height - RBLXWIN_BORDER_SIZE - RBLXWIN_CAPTION_HEIGHT);
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
    _RBLXWINwindow* window = userData;
    _rblxwinInputWindowCloseRequest(window);
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
    _RBLXWINwindow* window = userData;

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
        _rblxwinInputWindowMaximize(window, window->wl.maximized);
    }

    window->wl.fullscreen = window->wl.pending.fullscreen;

    int width  = window->wl.pending.width;
    int height = window->wl.pending.height;

    if (!window->wl.maximized && !window->wl.fullscreen)
    {
        if (window->numer != RBLXWIN_DONT_CARE && window->denom != RBLXWIN_DONT_CARE)
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

        _rblxwinInputWindowSize(window, width, height);

        if (window->wl.visible)
            _rblxwinInputWindowDamage(window);
    }

    if (!window->wl.visible)
    {
        // Allow the window to be mapped only if it either has no XDG
        // decorations or they have already received a configure event
        if (!window->wl.xdg.decoration || window->wl.xdg.decorationMode)
        {
            window->wl.visible = RBLXWIN_TRUE;
            _rblxwinInputWindowDamage(window);
        }
    }
}

static const struct xdg_surface_listener xdgSurfaceListener =
{
    xdgSurfaceHandleConfigure
};

static RBLXWINbool createShellObjects(_RBLXWINwindow* window)
{
    window->wl.xdg.surface = xdg_wm_base_get_xdg_surface(_rblxwin.wl.wmBase,
                                                         window->wl.surface);
    if (!window->wl.xdg.surface)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to create xdg-surface for window");
        return RBLXWIN_FALSE;
    }

    xdg_surface_add_listener(window->wl.xdg.surface, &xdgSurfaceListener, window);

    window->wl.xdg.toplevel = xdg_surface_get_toplevel(window->wl.xdg.surface);
    if (!window->wl.xdg.toplevel)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to create xdg-toplevel for window");
        return RBLXWIN_FALSE;
    }

    xdg_toplevel_add_listener(window->wl.xdg.toplevel, &xdgToplevelListener, window);

    if (window->wl.title)
        xdg_toplevel_set_title(window->wl.xdg.toplevel, window->wl.title);

    if (window->monitor)
    {
        xdg_toplevel_set_fullscreen(window->wl.xdg.toplevel, window->monitor->wl.output);
        setIdleInhibitor(window, RBLXWIN_TRUE);
    }
    else
    {
        if (window->wl.maximized)
            xdg_toplevel_set_maximized(window->wl.xdg.toplevel);

        setIdleInhibitor(window, RBLXWIN_FALSE);

        if (_rblxwin.wl.decorationManager)
        {
            window->wl.xdg.decoration =
                zxdg_decoration_manager_v1_get_toplevel_decoration(
                    _rblxwin.wl.decorationManager, window->wl.xdg.toplevel);
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

    if (window->minwidth != RBLXWIN_DONT_CARE && window->minheight != RBLXWIN_DONT_CARE)
    {
        int minwidth  = window->minwidth;
        int minheight = window->minheight;

        if (window->wl.decorations.top.surface)
        {
            minwidth  += RBLXWIN_BORDER_SIZE * 2;
            minheight += RBLXWIN_CAPTION_HEIGHT + RBLXWIN_BORDER_SIZE;
        }

        xdg_toplevel_set_min_size(window->wl.xdg.toplevel, minwidth, minheight);
    }

    if (window->maxwidth != RBLXWIN_DONT_CARE && window->maxheight != RBLXWIN_DONT_CARE)
    {
        int maxwidth  = window->maxwidth;
        int maxheight = window->maxheight;

        if (window->wl.decorations.top.surface)
        {
            maxwidth  += RBLXWIN_BORDER_SIZE * 2;
            maxheight += RBLXWIN_CAPTION_HEIGHT + RBLXWIN_BORDER_SIZE;
        }

        xdg_toplevel_set_max_size(window->wl.xdg.toplevel, maxwidth, maxheight);
    }

    wl_surface_commit(window->wl.surface);
    wl_display_roundtrip(_rblxwin.wl.display);

    return RBLXWIN_TRUE;
}

static void destroyShellObjects(_RBLXWINwindow* window)
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

static RBLXWINbool createNativeSurface(_RBLXWINwindow* window,
                                    const _RBLXWINwndconfig* wndconfig,
                                    const _RBLXWINfbconfig* fbconfig)
{
    window->wl.surface = wl_compositor_create_surface(_rblxwin.wl.compositor);
    if (!window->wl.surface)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR, "Wayland: Failed to create window surface");
        return RBLXWIN_FALSE;
    }

    wl_surface_add_listener(window->wl.surface,
                            &surfaceListener,
                            window);

    wl_surface_set_user_data(window->wl.surface, window);

    window->wl.width = wndconfig->width;
    window->wl.height = wndconfig->height;
    window->wl.scale = 1;
    window->wl.title = _rblxwin_strdup(wndconfig->title);

    window->wl.maximized = wndconfig->maximized;

    window->wl.transparent = fbconfig->transparent;
    if (!window->wl.transparent)
        setContentAreaOpaque(window);

    return RBLXWIN_TRUE;
}

static void setCursorImage(_RBLXWINwindow* window,
                           _RBLXWINcursorWayland* cursorWayland)
{
    struct itimerspec timer = {};
    struct wl_cursor* wlCursor = cursorWayland->cursor;
    struct wl_cursor_image* image;
    struct wl_buffer* buffer;
    struct wl_surface* surface = _rblxwin.wl.cursorSurface;
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
        timerfd_settime(_rblxwin.wl.cursorTimerfd, 0, &timer, NULL);

        cursorWayland->width = image->width;
        cursorWayland->height = image->height;
        cursorWayland->xhot = image->hotspot_x;
        cursorWayland->yhot = image->hotspot_y;
    }

    wl_pointer_set_cursor(_rblxwin.wl.pointer, _rblxwin.wl.pointerEnterSerial,
                          surface,
                          cursorWayland->xhot / scale,
                          cursorWayland->yhot / scale);
    wl_surface_set_buffer_scale(surface, scale);
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage(surface, 0, 0,
                      cursorWayland->width, cursorWayland->height);
    wl_surface_commit(surface);
}

static void incrementCursorImage(_RBLXWINwindow* window)
{
    _RBLXWINcursor* cursor;

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

static RBLXWINbool flushDisplay(void)
{
    while (wl_display_flush(_rblxwin.wl.display) == -1)
    {
        if (errno != EAGAIN)
            return RBLXWIN_FALSE;

        struct pollfd fd = { wl_display_get_fd(_rblxwin.wl.display), POLLOUT };

        while (poll(&fd, 1, -1) == -1)
        {
            if (errno != EINTR && errno != EAGAIN)
                return RBLXWIN_FALSE;
        }
    }

    return RBLXWIN_TRUE;
}

static void handleEvents(double* timeout)
{
    RBLXWINbool event = RBLXWIN_FALSE;
    struct pollfd fds[] =
    {
        { wl_display_get_fd(_rblxwin.wl.display), POLLIN },
        { _rblxwin.wl.timerfd, POLLIN },
        { _rblxwin.wl.cursorTimerfd, POLLIN },
    };

    while (!event)
    {
        while (wl_display_prepare_read(_rblxwin.wl.display) != 0)
            wl_display_dispatch_pending(_rblxwin.wl.display);

        // If an error other than EAGAIN happens, we have likely been disconnected
        // from the Wayland session; try to handle that the best we can.
        if (!flushDisplay())
        {
            wl_display_cancel_read(_rblxwin.wl.display);

            _RBLXWINwindow* window = _rblxwin.windowListHead;
            while (window)
            {
                _rblxwinInputWindowCloseRequest(window);
                window = window->next;
            }

            return;
        }

        if (!waitForData(fds, 3, timeout))
        {
            wl_display_cancel_read(_rblxwin.wl.display);
            return;
        }

        if (fds[0].revents & POLLIN)
        {
            wl_display_read_events(_rblxwin.wl.display);
            if (wl_display_dispatch_pending(_rblxwin.wl.display) > 0)
                event = RBLXWIN_TRUE;
        }
        else
            wl_display_cancel_read(_rblxwin.wl.display);

        if (fds[1].revents & POLLIN)
        {
            uint64_t repeats;

            if (read(_rblxwin.wl.timerfd, &repeats, sizeof(repeats)) == 8)
            {
                for (uint64_t i = 0; i < repeats; i++)
                {
                    _rblxwinInputKey(_rblxwin.wl.keyboardFocus,
                                  _rblxwin.wl.keyboardLastKey,
                                  _rblxwin.wl.keyboardLastScancode,
                                  RBLXWIN_PRESS,
                                  _rblxwin.wl.xkb.modifiers);
                    _rblxwinInputTextWayland(_rblxwin.wl.keyboardFocus,
                                          _rblxwin.wl.keyboardLastScancode);
                }

                event = RBLXWIN_TRUE;
            }
        }

        if (fds[2].revents & POLLIN)
        {
            uint64_t repeats;

            if (read(_rblxwin.wl.cursorTimerfd, &repeats, sizeof(repeats)) == 8)
            {
                incrementCursorImage(_rblxwin.wl.pointerFocus);
                event = RBLXWIN_TRUE;
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
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
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
                _rblxwinInputError(RBLXWIN_OUT_OF_MEMORY, NULL);
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

            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
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

static _RBLXWINwindow* findWindowFromDecorationSurface(struct wl_surface* surface,
                                                    _RBLXWINdecorationSideWayland* which)
{
    _RBLXWINdecorationSideWayland focus;
    _RBLXWINwindow* window = _rblxwin.windowListHead;
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

    _RBLXWINdecorationSideWayland focus = mainWindow;
    _RBLXWINwindow* window = wl_surface_get_user_data(surface);
    if (!window)
    {
        window = findWindowFromDecorationSurface(surface, &focus);
        if (!window)
            return;
    }

    window->wl.decorations.focus = focus;
    _rblxwin.wl.serial = serial;
    _rblxwin.wl.pointerEnterSerial = serial;
    _rblxwin.wl.pointerFocus = window;

    window->wl.hovered = RBLXWIN_TRUE;

    _rblxwinPlatformSetCursor(window, window->wl.currentCursor);
    _rblxwinInputCursorEnter(window, RBLXWIN_TRUE);
}

static void pointerHandleLeave(void* userData,
                               struct wl_pointer* pointer,
                               uint32_t serial,
                               struct wl_surface* surface)
{
    _RBLXWINwindow* window = _rblxwin.wl.pointerFocus;

    if (!window)
        return;

    window->wl.hovered = RBLXWIN_FALSE;

    _rblxwin.wl.serial = serial;
    _rblxwin.wl.pointerFocus = NULL;
    _rblxwinInputCursorEnter(window, RBLXWIN_FALSE);
    _rblxwin.wl.cursorPreviousName = NULL;
}

static void setCursor(_RBLXWINwindow* window, const char* name)
{
    struct wl_buffer* buffer;
    struct wl_cursor* cursor;
    struct wl_cursor_image* image;
    struct wl_surface* surface = _rblxwin.wl.cursorSurface;
    struct wl_cursor_theme* theme = _rblxwin.wl.cursorTheme;
    int scale = 1;

    if (window->wl.scale > 1 && _rblxwin.wl.cursorThemeHiDPI)
    {
        // We only support up to scale=2 for now, since libwayland-cursor
        // requires us to load a different theme for each size.
        scale = 2;
        theme = _rblxwin.wl.cursorThemeHiDPI;
    }

    cursor = wl_cursor_theme_get_cursor(theme, name);
    if (!cursor)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
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
    wl_pointer_set_cursor(_rblxwin.wl.pointer, _rblxwin.wl.pointerEnterSerial,
                          surface,
                          image->hotspot_x / scale,
                          image->hotspot_y / scale);
    wl_surface_set_buffer_scale(surface, scale);
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage(surface, 0, 0,
                      image->width, image->height);
    wl_surface_commit(surface);
    _rblxwin.wl.cursorPreviousName = name;
}

static void pointerHandleMotion(void* userData,
                                struct wl_pointer* pointer,
                                uint32_t time,
                                wl_fixed_t sx,
                                wl_fixed_t sy)
{
    _RBLXWINwindow* window = _rblxwin.wl.pointerFocus;
    const char* cursorName = NULL;
    double x, y;

    if (!window)
        return;

    if (window->cursorMode == RBLXWIN_CURSOR_DISABLED)
        return;
    x = wl_fixed_to_double(sx);
    y = wl_fixed_to_double(sy);
    window->wl.cursorPosX = x;
    window->wl.cursorPosY = y;

    switch (window->wl.decorations.focus)
    {
        case mainWindow:
            _rblxwinInputCursorPos(window, x, y);
            _rblxwin.wl.cursorPreviousName = NULL;
            return;
        case topDecoration:
            if (y < RBLXWIN_BORDER_SIZE)
                cursorName = "n-resize";
            else
                cursorName = "left_ptr";
            break;
        case leftDecoration:
            if (y < RBLXWIN_BORDER_SIZE)
                cursorName = "nw-resize";
            else
                cursorName = "w-resize";
            break;
        case rightDecoration:
            if (y < RBLXWIN_BORDER_SIZE)
                cursorName = "ne-resize";
            else
                cursorName = "e-resize";
            break;
        case bottomDecoration:
            if (x < RBLXWIN_BORDER_SIZE)
                cursorName = "sw-resize";
            else if (x > window->wl.width + RBLXWIN_BORDER_SIZE)
                cursorName = "se-resize";
            else
                cursorName = "s-resize";
            break;
        default:
            assert(0);
    }
    if (_rblxwin.wl.cursorPreviousName != cursorName)
        setCursor(window, cursorName);
}

static void pointerHandleButton(void* userData,
                                struct wl_pointer* pointer,
                                uint32_t serial,
                                uint32_t time,
                                uint32_t button,
                                uint32_t state)
{
    _RBLXWINwindow* window = _rblxwin.wl.pointerFocus;
    int rblxwinButton;

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
                if (window->wl.cursorPosY < RBLXWIN_BORDER_SIZE)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP;
                else
                {
                    xdg_toplevel_move(window->wl.xdg.toplevel, _rblxwin.wl.seat, serial);
                }
                break;
            case leftDecoration:
                if (window->wl.cursorPosY < RBLXWIN_BORDER_SIZE)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
                else
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
                break;
            case rightDecoration:
                if (window->wl.cursorPosY < RBLXWIN_BORDER_SIZE)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
                else
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
                break;
            case bottomDecoration:
                if (window->wl.cursorPosX < RBLXWIN_BORDER_SIZE)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
                else if (window->wl.cursorPosX > window->wl.width + RBLXWIN_BORDER_SIZE)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
                else
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
                break;
            default:
                assert(0);
        }
        if (edges != XDG_TOPLEVEL_RESIZE_EDGE_NONE)
        {
            xdg_toplevel_resize(window->wl.xdg.toplevel, _rblxwin.wl.seat,
                                serial, edges);
            return;
        }
    }
    else if (button == BTN_RIGHT)
    {
        if (window->wl.decorations.focus != mainWindow && window->wl.xdg.toplevel)
        {
            xdg_toplevel_show_window_menu(window->wl.xdg.toplevel,
                                          _rblxwin.wl.seat, serial,
                                          window->wl.cursorPosX,
                                          window->wl.cursorPosY);
            return;
        }
    }

    // Don’t pass the button to the user if it was related to a decoration.
    if (window->wl.decorations.focus != mainWindow)
        return;

    _rblxwin.wl.serial = serial;

    /* Makes left, right and middle 0, 1 and 2. Overall order follows evdev
     * codes. */
    rblxwinButton = button - BTN_LEFT;

    _rblxwinInputMouseClick(window,
                         rblxwinButton,
                         state == WL_POINTER_BUTTON_STATE_PRESSED
                                ? RBLXWIN_PRESS
                                : RBLXWIN_RELEASE,
                         _rblxwin.wl.xkb.modifiers);
}

static void pointerHandleAxis(void* userData,
                              struct wl_pointer* pointer,
                              uint32_t time,
                              uint32_t axis,
                              wl_fixed_t value)
{
    _RBLXWINwindow* window = _rblxwin.wl.pointerFocus;
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

    _rblxwinInputScroll(window, x, y);
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

    keymap = xkb_keymap_new_from_string(_rblxwin.wl.xkb.context,
                                        mapStr,
                                        XKB_KEYMAP_FORMAT_TEXT_V1,
                                        0);
    munmap(mapStr, size);
    close(fd);

    if (!keymap)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to compile keymap");
        return;
    }

    state = xkb_state_new(keymap);
    if (!state)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
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
        xkb_compose_table_new_from_locale(_rblxwin.wl.xkb.context, locale,
                                          XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (composeTable)
    {
        composeState =
            xkb_compose_state_new(composeTable, XKB_COMPOSE_STATE_NO_FLAGS);
        xkb_compose_table_unref(composeTable);
        if (composeState)
            _rblxwin.wl.xkb.composeState = composeState;
        else
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "Wayland: Failed to create XKB compose state");
    }
    else
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to create XKB compose table");
    }

    xkb_keymap_unref(_rblxwin.wl.xkb.keymap);
    xkb_state_unref(_rblxwin.wl.xkb.state);
    _rblxwin.wl.xkb.keymap = keymap;
    _rblxwin.wl.xkb.state = state;

    _rblxwin.wl.xkb.controlIndex  = xkb_keymap_mod_get_index(_rblxwin.wl.xkb.keymap, "Control");
    _rblxwin.wl.xkb.altIndex      = xkb_keymap_mod_get_index(_rblxwin.wl.xkb.keymap, "Mod1");
    _rblxwin.wl.xkb.shiftIndex    = xkb_keymap_mod_get_index(_rblxwin.wl.xkb.keymap, "Shift");
    _rblxwin.wl.xkb.superIndex    = xkb_keymap_mod_get_index(_rblxwin.wl.xkb.keymap, "Mod4");
    _rblxwin.wl.xkb.capsLockIndex = xkb_keymap_mod_get_index(_rblxwin.wl.xkb.keymap, "Lock");
    _rblxwin.wl.xkb.numLockIndex  = xkb_keymap_mod_get_index(_rblxwin.wl.xkb.keymap, "Mod2");
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

    _RBLXWINwindow* window = wl_surface_get_user_data(surface);
    if (!window)
    {
        window = findWindowFromDecorationSurface(surface, NULL);
        if (!window)
            return;
    }

    _rblxwin.wl.serial = serial;
    _rblxwin.wl.keyboardFocus = window;
    _rblxwinInputWindowFocus(window, RBLXWIN_TRUE);
}

static void keyboardHandleLeave(void* userData,
                                struct wl_keyboard* keyboard,
                                uint32_t serial,
                                struct wl_surface* surface)
{
    _RBLXWINwindow* window = _rblxwin.wl.keyboardFocus;

    if (!window)
        return;

    struct itimerspec timer = {};
    timerfd_settime(_rblxwin.wl.timerfd, 0, &timer, NULL);

    _rblxwin.wl.serial = serial;
    _rblxwin.wl.keyboardFocus = NULL;
    _rblxwinInputWindowFocus(window, RBLXWIN_FALSE);
}

static int translateKey(uint32_t scancode)
{
    if (scancode < sizeof(_rblxwin.wl.keycodes) / sizeof(_rblxwin.wl.keycodes[0]))
        return _rblxwin.wl.keycodes[scancode];

    return RBLXWIN_KEY_UNKNOWN;
}

static xkb_keysym_t composeSymbol(xkb_keysym_t sym)
{
    if (sym == XKB_KEY_NoSymbol || !_rblxwin.wl.xkb.composeState)
        return sym;
    if (xkb_compose_state_feed(_rblxwin.wl.xkb.composeState, sym)
            != XKB_COMPOSE_FEED_ACCEPTED)
        return sym;
    switch (xkb_compose_state_get_status(_rblxwin.wl.xkb.composeState))
    {
        case XKB_COMPOSE_COMPOSED:
            return xkb_compose_state_get_one_sym(_rblxwin.wl.xkb.composeState);
        case XKB_COMPOSE_COMPOSING:
        case XKB_COMPOSE_CANCELLED:
            return XKB_KEY_NoSymbol;
        case XKB_COMPOSE_NOTHING:
        default:
            return sym;
    }
}

RBLXWINbool _rblxwinInputTextWayland(_RBLXWINwindow* window, uint32_t scancode)
{
    const xkb_keysym_t* keysyms;
    const xkb_keycode_t keycode = scancode + 8;

    if (xkb_state_key_get_syms(_rblxwin.wl.xkb.state, keycode, &keysyms) == 1)
    {
        const xkb_keysym_t keysym = composeSymbol(keysyms[0]);
        const uint32_t codepoint = _rblxwinKeySym2Unicode(keysym);
        if (codepoint != RBLXWIN_INVALID_CODEPOINT)
        {
            const int mods = _rblxwin.wl.xkb.modifiers;
            const int plain = !(mods & (RBLXWIN_MOD_CONTROL | RBLXWIN_MOD_ALT));
            _rblxwinInputChar(window, codepoint, mods, plain);
        }
    }

    return xkb_keymap_key_repeats(_rblxwin.wl.xkb.keymap, keycode);
}

static void keyboardHandleKey(void* userData,
                              struct wl_keyboard* keyboard,
                              uint32_t serial,
                              uint32_t time,
                              uint32_t scancode,
                              uint32_t state)
{
    _RBLXWINwindow* window = _rblxwin.wl.keyboardFocus;
    if (!window)
        return;

    const int key = translateKey(scancode);
    const int action =
        state == WL_KEYBOARD_KEY_STATE_PRESSED ? RBLXWIN_PRESS : RBLXWIN_RELEASE;

    _rblxwin.wl.serial = serial;
    _rblxwinInputKey(window, key, scancode, action, _rblxwin.wl.xkb.modifiers);

    struct itimerspec timer = {};

    if (action == RBLXWIN_PRESS)
    {
        const RBLXWINbool shouldRepeat = _rblxwinInputTextWayland(window, scancode);

        if (shouldRepeat && _rblxwin.wl.keyboardRepeatRate > 0)
        {
            _rblxwin.wl.keyboardLastKey = key;
            _rblxwin.wl.keyboardLastScancode = scancode;
            if (_rblxwin.wl.keyboardRepeatRate > 1)
                timer.it_interval.tv_nsec = 1000000000 / _rblxwin.wl.keyboardRepeatRate;
            else
                timer.it_interval.tv_sec = 1;

            timer.it_value.tv_sec = _rblxwin.wl.keyboardRepeatDelay / 1000;
            timer.it_value.tv_nsec = (_rblxwin.wl.keyboardRepeatDelay % 1000) * 1000000;
        }
    }

    timerfd_settime(_rblxwin.wl.timerfd, 0, &timer, NULL);
}

static void keyboardHandleModifiers(void* userData,
                                    struct wl_keyboard* keyboard,
                                    uint32_t serial,
                                    uint32_t modsDepressed,
                                    uint32_t modsLatched,
                                    uint32_t modsLocked,
                                    uint32_t group)
{
    _rblxwin.wl.serial = serial;

    if (!_rblxwin.wl.xkb.keymap)
        return;

    xkb_state_update_mask(_rblxwin.wl.xkb.state,
                          modsDepressed,
                          modsLatched,
                          modsLocked,
                          0,
                          0,
                          group);

    _rblxwin.wl.xkb.modifiers = 0;

    struct
    {
        xkb_mod_index_t index;
        unsigned int bit;
    } modifiers[] =
    {
        { _rblxwin.wl.xkb.controlIndex,  RBLXWIN_MOD_CONTROL },
        { _rblxwin.wl.xkb.altIndex,      RBLXWIN_MOD_ALT },
        { _rblxwin.wl.xkb.shiftIndex,    RBLXWIN_MOD_SHIFT },
        { _rblxwin.wl.xkb.superIndex,    RBLXWIN_MOD_SUPER },
        { _rblxwin.wl.xkb.capsLockIndex, RBLXWIN_MOD_CAPS_LOCK },
        { _rblxwin.wl.xkb.numLockIndex,  RBLXWIN_MOD_NUM_LOCK }
    };

    for (size_t i = 0; i < sizeof(modifiers) / sizeof(modifiers[0]); i++)
    {
        if (xkb_state_mod_index_is_active(_rblxwin.wl.xkb.state,
                                          modifiers[i].index,
                                          XKB_STATE_MODS_EFFECTIVE) == 1)
        {
            _rblxwin.wl.xkb.modifiers |= modifiers[i].bit;
        }
    }
}

#ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
static void keyboardHandleRepeatInfo(void* userData,
                                     struct wl_keyboard* keyboard,
                                     int32_t rate,
                                     int32_t delay)
{
    if (keyboard != _rblxwin.wl.keyboard)
        return;

    _rblxwin.wl.keyboardRepeatRate = rate;
    _rblxwin.wl.keyboardRepeatDelay = delay;
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
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !_rblxwin.wl.pointer)
    {
        _rblxwin.wl.pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(_rblxwin.wl.pointer, &pointerListener, NULL);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && _rblxwin.wl.pointer)
    {
        wl_pointer_destroy(_rblxwin.wl.pointer);
        _rblxwin.wl.pointer = NULL;
    }

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !_rblxwin.wl.keyboard)
    {
        _rblxwin.wl.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(_rblxwin.wl.keyboard, &keyboardListener, NULL);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && _rblxwin.wl.keyboard)
    {
        wl_keyboard_destroy(_rblxwin.wl.keyboard);
        _rblxwin.wl.keyboard = NULL;
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
    for (unsigned int i = 0; i < _rblxwin.wl.offerCount; i++)
    {
        if (_rblxwin.wl.offers[i].offer == offer)
        {
            if (strcmp(mimeType, "text/plain;charset=utf-8") == 0)
                _rblxwin.wl.offers[i].text_plain_utf8 = RBLXWIN_TRUE;
            else if (strcmp(mimeType, "text/uri-list") == 0)
                _rblxwin.wl.offers[i].text_uri_list = RBLXWIN_TRUE;

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
    _RBLXWINofferWayland* offers =
        realloc(_rblxwin.wl.offers, _rblxwin.wl.offerCount + 1);
    if (!offers)
    {
        _rblxwinInputError(RBLXWIN_OUT_OF_MEMORY, NULL);
        return;
    }

    _rblxwin.wl.offers = offers;
    _rblxwin.wl.offerCount++;

    _rblxwin.wl.offers[_rblxwin.wl.offerCount - 1] = (_RBLXWINofferWayland) { offer };
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
    if (_rblxwin.wl.dragOffer)
    {
        wl_data_offer_destroy(_rblxwin.wl.dragOffer);
        _rblxwin.wl.dragOffer = NULL;
        _rblxwin.wl.dragFocus = NULL;
    }

    for (unsigned int i = 0; i < _rblxwin.wl.offerCount; i++)
    {
        if (_rblxwin.wl.offers[i].offer == offer)
        {
            _RBLXWINwindow* window = NULL;

            if (surface)
                window = wl_surface_get_user_data(surface);

            if (window && _rblxwin.wl.offers[i].text_uri_list)
            {
                _rblxwin.wl.dragOffer = offer;
                _rblxwin.wl.dragFocus = window;
                _rblxwin.wl.dragSerial = serial;
            }

            _rblxwin.wl.offers[i] = _rblxwin.wl.offers[_rblxwin.wl.offerCount - 1];
            _rblxwin.wl.offerCount--;
            break;
        }
    }

    if (_rblxwin.wl.dragOffer)
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
    if (_rblxwin.wl.dragOffer)
    {
        wl_data_offer_destroy(_rblxwin.wl.dragOffer);
        _rblxwin.wl.dragOffer = NULL;
        _rblxwin.wl.dragFocus = NULL;
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
    if (!_rblxwin.wl.dragOffer)
        return;

    char* string = readDataOfferAsString(_rblxwin.wl.dragOffer, "text/uri-list");
    if (string)
    {
        int count;
        char** paths = _rblxwinParseUriList(string, &count);
        if (paths)
            _rblxwinInputDrop(_rblxwin.wl.dragFocus, count, (const char**) paths);

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
    if (_rblxwin.wl.selectionOffer)
    {
        wl_data_offer_destroy(_rblxwin.wl.selectionOffer);
        _rblxwin.wl.selectionOffer = NULL;
    }

    for (unsigned int i = 0; i < _rblxwin.wl.offerCount; i++)
    {
        if (_rblxwin.wl.offers[i].offer == offer)
        {
            if (_rblxwin.wl.offers[i].text_plain_utf8)
                _rblxwin.wl.selectionOffer = offer;
            else
                wl_data_offer_destroy(offer);

            _rblxwin.wl.offers[i] = _rblxwin.wl.offers[_rblxwin.wl.offerCount - 1];
            _rblxwin.wl.offerCount--;
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

// Translates a RBLXWIN standard cursor to a theme cursor name
//
static char *translateCursorShape(int shape)
{
    switch (shape)
    {
        case RBLXWIN_ARROW_CURSOR:
            return "left_ptr";
        case RBLXWIN_IBEAM_CURSOR:
            return "xterm";
        case RBLXWIN_CROSSHAIR_CURSOR:
            return "crosshair";
        case RBLXWIN_HAND_CURSOR:
            return "hand2";
        case RBLXWIN_HRESIZE_CURSOR:
            return "sb_h_double_arrow";
        case RBLXWIN_VRESIZE_CURSOR:
            return "sb_v_double_arrow";
    }
    return NULL;
}

void _rblxwinAddSeatListenerWayland(struct wl_seat* seat)
{
    wl_seat_add_listener(seat, &seatListener, NULL);
}

void _rblxwinAddDataDeviceListenerWayland(struct wl_data_device* device)
{
    wl_data_device_add_listener(device, &dataDeviceListener, NULL);
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _rblxwinPlatformCreateWindow(_RBLXWINwindow* window,
                              const _RBLXWINwndconfig* wndconfig,
                              const _RBLXWINctxconfig* ctxconfig,
                              const _RBLXWINfbconfig* fbconfig)
{
    if (!createNativeSurface(window, wndconfig, fbconfig))
        return RBLXWIN_FALSE;

    if (ctxconfig->client != RBLXWIN_NO_API)
    {
        if (ctxconfig->source == RBLXWIN_EGL_CONTEXT_API ||
            ctxconfig->source == RBLXWIN_NATIVE_CONTEXT_API)
        {
            window->wl.egl.window = wl_egl_window_create(window->wl.surface,
                                                         wndconfig->width,
                                                         wndconfig->height);
            if (!window->wl.egl.window)
            {
                _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                                "Wayland: Failed to create EGL window");
                return RBLXWIN_FALSE;
            }

            if (!_rblxwinInitEGL())
                return RBLXWIN_FALSE;
            if (!_rblxwinCreateContextEGL(window, ctxconfig, fbconfig))
                return RBLXWIN_FALSE;
        }
        else if (ctxconfig->source == RBLXWIN_OSMESA_CONTEXT_API)
        {
            if (!_rblxwinInitOSMesa())
                return RBLXWIN_FALSE;
            if (!_rblxwinCreateContextOSMesa(window, ctxconfig, fbconfig))
                return RBLXWIN_FALSE;
        }

        if (!_rblxwinRefreshContextAttribs(window, ctxconfig))
            return RBLXWIN_FALSE;
    }

    if (window->monitor || wndconfig->visible)
    {
        if (!createShellObjects(window))
            return RBLXWIN_FALSE;
    }

    return RBLXWIN_TRUE;
}

void _rblxwinPlatformDestroyWindow(_RBLXWINwindow* window)
{
    if (window == _rblxwin.wl.pointerFocus)
    {
        _rblxwin.wl.pointerFocus = NULL;
        _rblxwinInputCursorEnter(window, RBLXWIN_FALSE);
    }
    if (window == _rblxwin.wl.keyboardFocus)
    {
        _rblxwin.wl.keyboardFocus = NULL;
        _rblxwinInputWindowFocus(window, RBLXWIN_FALSE);
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

void _rblxwinPlatformSetWindowTitle(_RBLXWINwindow* window, const char* title)
{
    if (window->wl.title)
        free(window->wl.title);
    window->wl.title = _rblxwin_strdup(title);
    if (window->wl.xdg.toplevel)
        xdg_toplevel_set_title(window->wl.xdg.toplevel, title);
}

void _rblxwinPlatformSetWindowIcon(_RBLXWINwindow* window,
                                int count, const RBLXWINimage* images)
{
    _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                    "Wayland: Setting window icon not supported");
}

void _rblxwinPlatformGetWindowPos(_RBLXWINwindow* window, int* xpos, int* ypos)
{
    // A Wayland client is not aware of its position, so just warn and leave it
    // as (0, 0)

    _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                    "Wayland: Window position retrieval not supported");
}

void _rblxwinPlatformSetWindowPos(_RBLXWINwindow* window, int xpos, int ypos)
{
    // A Wayland client can not set its position, so just warn

    _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                    "Wayland: Window position setting not supported");
}

void _rblxwinPlatformGetWindowSize(_RBLXWINwindow* window, int* width, int* height)
{
    if (width)
        *width = window->wl.width;
    if (height)
        *height = window->wl.height;
}

void _rblxwinPlatformSetWindowSize(_RBLXWINwindow* window, int width, int height)
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

void _rblxwinPlatformSetWindowSizeLimits(_RBLXWINwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight)
{
    if (window->wl.xdg.toplevel)
    {
        if (minwidth == RBLXWIN_DONT_CARE || minheight == RBLXWIN_DONT_CARE)
            minwidth = minheight = 0;
        else
        {
            if (window->wl.decorations.top.surface)
            {
                minwidth  += RBLXWIN_BORDER_SIZE * 2;
                minheight += RBLXWIN_CAPTION_HEIGHT + RBLXWIN_BORDER_SIZE;
            }
        }

        if (maxwidth == RBLXWIN_DONT_CARE || maxheight == RBLXWIN_DONT_CARE)
            maxwidth = maxheight = 0;
        else
        {
            if (window->wl.decorations.top.surface)
            {
                maxwidth  += RBLXWIN_BORDER_SIZE * 2;
                maxheight += RBLXWIN_CAPTION_HEIGHT + RBLXWIN_BORDER_SIZE;
            }
        }

        xdg_toplevel_set_min_size(window->wl.xdg.toplevel, minwidth, minheight);
        xdg_toplevel_set_max_size(window->wl.xdg.toplevel, maxwidth, maxheight);
        wl_surface_commit(window->wl.surface);
    }
}

void _rblxwinPlatformSetWindowAspectRatio(_RBLXWINwindow* window,
                                       int numer, int denom)
{
    if (window->wl.maximized || window->wl.fullscreen)
        return;

    if (numer != RBLXWIN_DONT_CARE && denom != RBLXWIN_DONT_CARE)
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

void _rblxwinPlatformGetFramebufferSize(_RBLXWINwindow* window,
                                     int* width, int* height)
{
    _rblxwinPlatformGetWindowSize(window, width, height);
    if (width)
        *width *= window->wl.scale;
    if (height)
        *height *= window->wl.scale;
}

void _rblxwinPlatformGetWindowFrameSize(_RBLXWINwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom)
{
    if (window->decorated && !window->monitor && window->wl.decorations.top.surface)
    {
        if (top)
            *top = RBLXWIN_CAPTION_HEIGHT;
        if (left)
            *left = RBLXWIN_BORDER_SIZE;
        if (right)
            *right = RBLXWIN_BORDER_SIZE;
        if (bottom)
            *bottom = RBLXWIN_BORDER_SIZE;
    }
}

void _rblxwinPlatformGetWindowContentScale(_RBLXWINwindow* window,
                                        float* xscale, float* yscale)
{
    if (xscale)
        *xscale = (float) window->wl.scale;
    if (yscale)
        *yscale = (float) window->wl.scale;
}

void _rblxwinPlatformIconifyWindow(_RBLXWINwindow* window)
{
    if (window->wl.xdg.toplevel)
        xdg_toplevel_set_minimized(window->wl.xdg.toplevel);
}

void _rblxwinPlatformRestoreWindow(_RBLXWINwindow* window)
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
                window->wl.maximized = RBLXWIN_FALSE;
        }
    }
}

void _rblxwinPlatformMaximizeWindow(_RBLXWINwindow* window)
{
    if (window->wl.xdg.toplevel)
        xdg_toplevel_set_maximized(window->wl.xdg.toplevel);
    else
        window->wl.maximized = RBLXWIN_TRUE;
}

void _rblxwinPlatformShowWindow(_RBLXWINwindow* window)
{
    if (!window->wl.xdg.toplevel)
    {
        // NOTE: The XDG/shell surface is created here so command-line applications
        //       with off-screen windows do not appear in for example the Unity dock
        createShellObjects(window);
    }
}

void _rblxwinPlatformHideWindow(_RBLXWINwindow* window)
{
    if (window->wl.visible)
    {
        window->wl.visible = RBLXWIN_FALSE;
        destroyShellObjects(window);

        wl_surface_attach(window->wl.surface, NULL, 0, 0);
        wl_surface_commit(window->wl.surface);
    }
}

void _rblxwinPlatformRequestWindowAttention(_RBLXWINwindow* window)
{
    // TODO
    _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                    "Wayland: Window attention request not implemented yet");
}

void _rblxwinPlatformFocusWindow(_RBLXWINwindow* window)
{
    _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                    "Wayland: Focusing a window requires user interaction");
}

void _rblxwinPlatformSetWindowMonitor(_RBLXWINwindow* window,
                                   _RBLXWINmonitor* monitor,
                                   int xpos, int ypos,
                                   int width, int height,
                                   int refreshRate)
{
    if (window->monitor == monitor)
    {
        if (!monitor)
            _rblxwinPlatformSetWindowSize(window, width, height);

        return;
    }

    if (window->monitor)
        releaseMonitor(window);

    _rblxwinInputWindowMonitor(window, monitor);

    if (window->monitor)
        acquireMonitor(window);
    else
        _rblxwinPlatformSetWindowSize(window, width, height);
}

int _rblxwinPlatformWindowFocused(_RBLXWINwindow* window)
{
    return _rblxwin.wl.keyboardFocus == window;
}

int _rblxwinPlatformWindowIconified(_RBLXWINwindow* window)
{
    // xdg-shell doesn’t give any way to request whether a surface is iconified
    return RBLXWIN_FALSE;
}

int _rblxwinPlatformWindowVisible(_RBLXWINwindow* window)
{
    return window->wl.visible;
}

int _rblxwinPlatformWindowMaximized(_RBLXWINwindow* window)
{
    return window->wl.maximized;
}

int _rblxwinPlatformWindowHovered(_RBLXWINwindow* window)
{
    return window->wl.hovered;
}

int _rblxwinPlatformFramebufferTransparent(_RBLXWINwindow* window)
{
    return window->wl.transparent;
}

void _rblxwinPlatformSetWindowResizable(_RBLXWINwindow* window, RBLXWINbool enabled)
{
    // TODO
    _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                    "Wayland: Window attribute setting not implemented yet");
}

void _rblxwinPlatformSetWindowDecorated(_RBLXWINwindow* window, RBLXWINbool enabled)
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

void _rblxwinPlatformSetWindowFloating(_RBLXWINwindow* window, RBLXWINbool enabled)
{
    // TODO
    _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                    "Wayland: Window attribute setting not implemented yet");
}

float _rblxwinPlatformGetWindowOpacity(_RBLXWINwindow* window)
{
    return 1.f;
}

void _rblxwinPlatformSetWindowOpacity(_RBLXWINwindow* window, float opacity)
{
}

void _rblxwinPlatformSetRawMouseMotion(_RBLXWINwindow* window, RBLXWINbool enabled)
{
    // This is handled in relativePointerHandleRelativeMotion
}

RBLXWINbool _rblxwinPlatformRawMouseMotionSupported(void)
{
    return RBLXWIN_TRUE;
}

void _rblxwinPlatformPollEvents(void)
{
    double timeout = 0.0;
    handleEvents(&timeout);
}

void _rblxwinPlatformWaitEvents(void)
{
    handleEvents(NULL);
}

void _rblxwinPlatformWaitEventsTimeout(double timeout)
{
    handleEvents(&timeout);
}

void _rblxwinPlatformPostEmptyEvent(void)
{
    wl_display_sync(_rblxwin.wl.display);
    flushDisplay();
}

void _rblxwinPlatformGetCursorPos(_RBLXWINwindow* window, double* xpos, double* ypos)
{
    if (xpos)
        *xpos = window->wl.cursorPosX;
    if (ypos)
        *ypos = window->wl.cursorPosY;
}

static RBLXWINbool isPointerLocked(_RBLXWINwindow* window);

void _rblxwinPlatformSetCursorPos(_RBLXWINwindow* window, double x, double y)
{
    if (isPointerLocked(window))
    {
        zwp_locked_pointer_v1_set_cursor_position_hint(
            window->wl.pointerLock.lockedPointer,
            wl_fixed_from_double(x), wl_fixed_from_double(y));
    }
}

void _rblxwinPlatformSetCursorMode(_RBLXWINwindow* window, int mode)
{
    _rblxwinPlatformSetCursor(window, window->wl.currentCursor);
}

const char* _rblxwinPlatformGetScancodeName(int scancode)
{
    if (scancode < 0 || scancode > 255 ||
        _rblxwin.wl.keycodes[scancode] == RBLXWIN_KEY_UNKNOWN)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                        "Wayland: Invalid scancode %i",
                        scancode);
        return NULL;
    }

    const int key = _rblxwin.wl.keycodes[scancode];
    const xkb_keycode_t keycode = scancode + 8;
    const xkb_layout_index_t layout =
        xkb_state_key_get_layout(_rblxwin.wl.xkb.state, keycode);
    if (layout == XKB_LAYOUT_INVALID)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to retrieve layout for key name");
        return NULL;
    }

    const xkb_keysym_t* keysyms = NULL;
    xkb_keymap_key_get_syms_by_level(_rblxwin.wl.xkb.keymap,
                                     keycode,
                                     layout,
                                     0,
                                     &keysyms);
    if (keysyms == NULL)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to retrieve keysym for key name");
        return NULL;
    }

    const uint32_t codepoint = _rblxwinKeySym2Unicode(keysyms[0]);
    if (codepoint == RBLXWIN_INVALID_CODEPOINT)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to retrieve codepoint for key name");
        return NULL;
    }

    const size_t count = _rblxwinEncodeUTF8(_rblxwin.wl.keynames[key],  codepoint);
    if (count == 0)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to encode codepoint for key name");
        return NULL;
    }

    _rblxwin.wl.keynames[key][count] = '\0';
    return _rblxwin.wl.keynames[key];
}

int _rblxwinPlatformGetKeyScancode(int key)
{
    return _rblxwin.wl.scancodes[key];
}

int _rblxwinPlatformCreateCursor(_RBLXWINcursor* cursor,
                              const RBLXWINimage* image,
                              int xhot, int yhot)
{
    cursor->wl.buffer = createShmBuffer(image);
    if (!cursor->wl.buffer)
        return RBLXWIN_FALSE;

    cursor->wl.width = image->width;
    cursor->wl.height = image->height;
    cursor->wl.xhot = xhot;
    cursor->wl.yhot = yhot;
    return RBLXWIN_TRUE;
}

int _rblxwinPlatformCreateStandardCursor(_RBLXWINcursor* cursor, int shape)
{
    struct wl_cursor* standardCursor;

    standardCursor = wl_cursor_theme_get_cursor(_rblxwin.wl.cursorTheme,
                                                translateCursorShape(shape));
    if (!standardCursor)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Standard cursor \"%s\" not found",
                        translateCursorShape(shape));
        return RBLXWIN_FALSE;
    }

    cursor->wl.cursor = standardCursor;
    cursor->wl.currentImage = 0;

    if (_rblxwin.wl.cursorThemeHiDPI)
    {
        standardCursor = wl_cursor_theme_get_cursor(_rblxwin.wl.cursorThemeHiDPI,
                                                    translateCursorShape(shape));
        cursor->wl.cursorHiDPI = standardCursor;
    }

    return RBLXWIN_TRUE;
}

void _rblxwinPlatformDestroyCursor(_RBLXWINcursor* cursor)
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
    _RBLXWINwindow* window = userData;
    double xpos = window->virtualCursorPosX;
    double ypos = window->virtualCursorPosY;

    if (window->cursorMode != RBLXWIN_CURSOR_DISABLED)
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

    _rblxwinInputCursorPos(window, xpos, ypos);
}

static const struct zwp_relative_pointer_v1_listener relativePointerListener =
{
    relativePointerHandleRelativeMotion
};

static void lockedPointerHandleLocked(void* userData,
                                      struct zwp_locked_pointer_v1* lockedPointer)
{
}

static void unlockPointer(_RBLXWINwindow* window)
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

static void lockPointer(_RBLXWINwindow* window);

static void lockedPointerHandleUnlocked(void* userData,
                                        struct zwp_locked_pointer_v1* lockedPointer)
{
}

static const struct zwp_locked_pointer_v1_listener lockedPointerListener =
{
    lockedPointerHandleLocked,
    lockedPointerHandleUnlocked
};

static void lockPointer(_RBLXWINwindow* window)
{
    struct zwp_relative_pointer_v1* relativePointer;
    struct zwp_locked_pointer_v1* lockedPointer;

    if (!_rblxwin.wl.relativePointerManager)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: no relative pointer manager");
        return;
    }

    relativePointer =
        zwp_relative_pointer_manager_v1_get_relative_pointer(
            _rblxwin.wl.relativePointerManager,
            _rblxwin.wl.pointer);
    zwp_relative_pointer_v1_add_listener(relativePointer,
                                         &relativePointerListener,
                                         window);

    lockedPointer =
        zwp_pointer_constraints_v1_lock_pointer(
            _rblxwin.wl.pointerConstraints,
            window->wl.surface,
            _rblxwin.wl.pointer,
            NULL,
            ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
    zwp_locked_pointer_v1_add_listener(lockedPointer,
                                       &lockedPointerListener,
                                       window);

    window->wl.pointerLock.relativePointer = relativePointer;
    window->wl.pointerLock.lockedPointer = lockedPointer;

    wl_pointer_set_cursor(_rblxwin.wl.pointer, _rblxwin.wl.pointerEnterSerial,
                          NULL, 0, 0);
}

static RBLXWINbool isPointerLocked(_RBLXWINwindow* window)
{
    return window->wl.pointerLock.lockedPointer != NULL;
}

void _rblxwinPlatformSetCursor(_RBLXWINwindow* window, _RBLXWINcursor* cursor)
{
    struct wl_cursor* defaultCursor;
    struct wl_cursor* defaultCursorHiDPI = NULL;

    if (!_rblxwin.wl.pointer)
        return;

    window->wl.currentCursor = cursor;

    // If we're not in the correct window just save the cursor
    // the next time the pointer enters the window the cursor will change
    if (window != _rblxwin.wl.pointerFocus || window->wl.decorations.focus != mainWindow)
        return;

    // Unlock possible pointer lock if no longer disabled.
    if (window->cursorMode != RBLXWIN_CURSOR_DISABLED && isPointerLocked(window))
        unlockPointer(window);

    if (window->cursorMode == RBLXWIN_CURSOR_NORMAL)
    {
        if (cursor)
            setCursorImage(window, &cursor->wl);
        else
        {
            defaultCursor = wl_cursor_theme_get_cursor(_rblxwin.wl.cursorTheme,
                                                       "left_ptr");
            if (!defaultCursor)
            {
                _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                                "Wayland: Standard cursor not found");
                return;
            }
            if (_rblxwin.wl.cursorThemeHiDPI)
                defaultCursorHiDPI =
                    wl_cursor_theme_get_cursor(_rblxwin.wl.cursorThemeHiDPI,
                                               "left_ptr");
            _RBLXWINcursorWayland cursorWayland = {
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
    else if (window->cursorMode == RBLXWIN_CURSOR_DISABLED)
    {
        if (!isPointerLocked(window))
            lockPointer(window);
    }
    else if (window->cursorMode == RBLXWIN_CURSOR_HIDDEN)
    {
        wl_pointer_set_cursor(_rblxwin.wl.pointer, _rblxwin.wl.pointerEnterSerial, NULL, 0, 0);
    }
}

static void dataSourceHandleTarget(void* userData,
                                   struct wl_data_source* source,
                                   const char* mimeType)
{
    if (_rblxwin.wl.selectionSource != source)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
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
    if (_rblxwin.wl.selectionSource != source ||
        strcmp(mimeType, "text/plain;charset=utf-8") != 0)
    {
        close(fd);
        return;
    }

    char* string = _rblxwin.wl.clipboardString;
    size_t length = strlen(string);

    while (length > 0)
    {
        const ssize_t result = write(fd, string, length);
        if (result == -1)
        {
            if (errno == EINTR)
                continue;

            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
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

    if (_rblxwin.wl.selectionSource != source)
        return;

    _rblxwin.wl.selectionSource = NULL;
}

static const struct wl_data_source_listener dataSourceListener =
{
    dataSourceHandleTarget,
    dataSourceHandleSend,
    dataSourceHandleCancelled,
};

void _rblxwinPlatformSetClipboardString(const char* string)
{
    if (_rblxwin.wl.selectionSource)
    {
        wl_data_source_destroy(_rblxwin.wl.selectionSource);
        _rblxwin.wl.selectionSource = NULL;
    }

    char* copy = _rblxwin_strdup(string);
    if (!copy)
    {
        _rblxwinInputError(RBLXWIN_OUT_OF_MEMORY, NULL);
        return;
    }

    free(_rblxwin.wl.clipboardString);
    _rblxwin.wl.clipboardString = copy;

    _rblxwin.wl.selectionSource =
        wl_data_device_manager_create_data_source(_rblxwin.wl.dataDeviceManager);
    if (!_rblxwin.wl.selectionSource)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to create clipboard data source");
        return;
    }
    wl_data_source_add_listener(_rblxwin.wl.selectionSource,
                                &dataSourceListener,
                                NULL);
    wl_data_source_offer(_rblxwin.wl.selectionSource, "text/plain;charset=utf-8");
    wl_data_device_set_selection(_rblxwin.wl.dataDevice,
                                 _rblxwin.wl.selectionSource,
                                 _rblxwin.wl.serial);
}

const char* _rblxwinPlatformGetClipboardString(void)
{
    if (!_rblxwin.wl.selectionOffer)
    {
        _rblxwinInputError(RBLXWIN_FORMAT_UNAVAILABLE,
                        "Wayland: No clipboard data available");
        return NULL;
    }

    if (_rblxwin.wl.selectionSource)
        return _rblxwin.wl.clipboardString;

    free(_rblxwin.wl.clipboardString);
    _rblxwin.wl.clipboardString =
        readDataOfferAsString(_rblxwin.wl.selectionOffer, "text/plain;charset=utf-8");
    return _rblxwin.wl.clipboardString;
}

void _rblxwinPlatformGetRequiredInstanceExtensions(char** extensions)
{
    if (!_rblxwin.vk.KHR_surface || !_rblxwin.vk.KHR_wayland_surface)
        return;

    extensions[0] = "VK_KHR_surface";
    extensions[1] = "VK_KHR_wayland_surface";
}

int _rblxwinPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily)
{
    PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR
        vkGetPhysicalDeviceWaylandPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)
        vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
    if (!vkGetPhysicalDeviceWaylandPresentationSupportKHR)
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                        "Wayland: Vulkan instance missing VK_KHR_wayland_surface extension");
        return VK_NULL_HANDLE;
    }

    return vkGetPhysicalDeviceWaylandPresentationSupportKHR(device,
                                                            queuefamily,
                                                            _rblxwin.wl.display);
}

VkResult _rblxwinPlatformCreateWindowSurface(VkInstance instance,
                                          _RBLXWINwindow* window,
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
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                        "Wayland: Vulkan instance missing VK_KHR_wayland_surface extension");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    memset(&sci, 0, sizeof(sci));
    sci.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    sci.display = _rblxwin.wl.display;
    sci.surface = window->wl.surface;

    err = vkCreateWaylandSurfaceKHR(instance, &sci, allocator, surface);
    if (err)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Failed to create Vulkan surface: %s",
                        _rblxwinGetVulkanResultString(err));
    }

    return err;
}


//////////////////////////////////////////////////////////////////////////
//////                        RBLXWIN native API                       //////
//////////////////////////////////////////////////////////////////////////

RBLXWINAPI struct wl_display* rblxwinGetWaylandDisplay(void)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return _rblxwin.wl.display;
}

RBLXWINAPI struct wl_surface* rblxwinGetWaylandWindow(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return window->wl.surface;
}

