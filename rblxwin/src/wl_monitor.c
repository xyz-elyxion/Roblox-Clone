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

#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>


static void outputHandleGeometry(void* userData,
                                 struct wl_output* output,
                                 int32_t x,
                                 int32_t y,
                                 int32_t physicalWidth,
                                 int32_t physicalHeight,
                                 int32_t subpixel,
                                 const char* make,
                                 const char* model,
                                 int32_t transform)
{
    struct _RBLXWINmonitor* monitor = userData;

    monitor->wl.x = x;
    monitor->wl.y = y;
    monitor->widthMM = physicalWidth;
    monitor->heightMM = physicalHeight;

    if (strlen(monitor->name) == 0)
        snprintf(monitor->name, sizeof(monitor->name), "%s %s", make, model);
}

static void outputHandleMode(void* userData,
                             struct wl_output* output,
                             uint32_t flags,
                             int32_t width,
                             int32_t height,
                             int32_t refresh)
{
    struct _RBLXWINmonitor* monitor = userData;
    RBLXWINvidmode mode;

    mode.width = width;
    mode.height = height;
    mode.redBits = 8;
    mode.greenBits = 8;
    mode.blueBits = 8;
    mode.refreshRate = (int) round(refresh / 1000.0);

    monitor->modeCount++;
    monitor->modes =
        realloc(monitor->modes, monitor->modeCount * sizeof(RBLXWINvidmode));
    monitor->modes[monitor->modeCount - 1] = mode;

    if (flags & WL_OUTPUT_MODE_CURRENT)
        monitor->wl.currentMode = monitor->modeCount - 1;
}

static void outputHandleDone(void* userData, struct wl_output* output)
{
    struct _RBLXWINmonitor* monitor = userData;

    if (monitor->widthMM <= 0 || monitor->heightMM <= 0)
    {
        // If Wayland does not provide a physical size, assume the default 96 DPI
        const RBLXWINvidmode* mode = &monitor->modes[monitor->wl.currentMode];
        monitor->widthMM  = (int) (mode->width * 25.4f / 96.f);
        monitor->heightMM = (int) (mode->height * 25.4f / 96.f);
    }

    for (int i = 0; i < _rblxwin.monitorCount; i++)
    {
        if (_rblxwin.monitors[i] == monitor)
            return;
    }

    _rblxwinInputMonitor(monitor, RBLXWIN_CONNECTED, _RBLXWIN_INSERT_LAST);
}

static void outputHandleScale(void* userData,
                              struct wl_output* output,
                              int32_t factor)
{
    struct _RBLXWINmonitor* monitor = userData;

    monitor->wl.scale = factor;

    for (_RBLXWINwindow* window = _rblxwin.windowListHead; window; window = window->next)
    {
        for (int i = 0; i < window->wl.monitorsCount; i++)
        {
            if (window->wl.monitors[i] == monitor)
            {
                _rblxwinUpdateContentScaleWayland(window);
                break;
            }
        }
    }
}

#ifdef WL_OUTPUT_NAME_SINCE_VERSION

void outputHandleName(void* userData, struct wl_output* wl_output, const char* name)
{
    struct _RBLXWINmonitor* monitor = userData;

    strncpy(monitor->name, name, sizeof(monitor->name) - 1);
}

void outputHandleDescription(void* userData,
                             struct wl_output* wl_output,
                             const char* description)
{
}

#endif // WL_OUTPUT_NAME_SINCE_VERSION

static const struct wl_output_listener outputListener =
{
    outputHandleGeometry,
    outputHandleMode,
    outputHandleDone,
    outputHandleScale,
#ifdef WL_OUTPUT_NAME_SINCE_VERSION
    outputHandleName,
    outputHandleDescription,
#endif
};


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

void _rblxwinAddOutputWayland(uint32_t name, uint32_t version)
{
    if (version < 2)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Wayland: Unsupported output interface version");
        return;
    }

#ifdef WL_OUTPUT_NAME_SINCE_VERSION
    version = _rblxwin_min(version, WL_OUTPUT_NAME_SINCE_VERSION);
#else
    version = 2;
#endif

    struct wl_output* output = wl_registry_bind(_rblxwin.wl.registry,
                                                name,
                                                &wl_output_interface,
                                                version);
    if (!output)
        return;

    // The actual name of this output will be set in the geometry handler
    _RBLXWINmonitor* monitor = _rblxwinAllocMonitor("", 0, 0);
    monitor->wl.scale = 1;
    monitor->wl.output = output;
    monitor->wl.name = name;

    wl_output_add_listener(output, &outputListener, monitor);
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _rblxwinPlatformFreeMonitor(_RBLXWINmonitor* monitor)
{
    if (monitor->wl.output)
        wl_output_destroy(monitor->wl.output);
}

void _rblxwinPlatformGetMonitorPos(_RBLXWINmonitor* monitor, int* xpos, int* ypos)
{
    if (xpos)
        *xpos = monitor->wl.x;
    if (ypos)
        *ypos = monitor->wl.y;
}

void _rblxwinPlatformGetMonitorContentScale(_RBLXWINmonitor* monitor,
                                         float* xscale, float* yscale)
{
    if (xscale)
        *xscale = (float) monitor->wl.scale;
    if (yscale)
        *yscale = (float) monitor->wl.scale;
}

void _rblxwinPlatformGetMonitorWorkarea(_RBLXWINmonitor* monitor,
                                     int* xpos, int* ypos,
                                     int* width, int* height)
{
    if (xpos)
        *xpos = monitor->wl.x;
    if (ypos)
        *ypos = monitor->wl.y;
    if (width)
        *width = monitor->modes[monitor->wl.currentMode].width;
    if (height)
        *height = monitor->modes[monitor->wl.currentMode].height;
}

RBLXWINvidmode* _rblxwinPlatformGetVideoModes(_RBLXWINmonitor* monitor, int* found)
{
    *found = monitor->modeCount;
    return monitor->modes;
}

void _rblxwinPlatformGetVideoMode(_RBLXWINmonitor* monitor, RBLXWINvidmode* mode)
{
    *mode = monitor->modes[monitor->wl.currentMode];
}

RBLXWINbool _rblxwinPlatformGetGammaRamp(_RBLXWINmonitor* monitor, RBLXWINgammaramp* ramp)
{
    _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                    "Wayland: Gamma ramp access is not available");
    return RBLXWIN_FALSE;
}

void _rblxwinPlatformSetGammaRamp(_RBLXWINmonitor* monitor,
                               const RBLXWINgammaramp* ramp)
{
    _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                    "Wayland: Gamma ramp access is not available");
}


//////////////////////////////////////////////////////////////////////////
//////                        RBLXWIN native API                       //////
//////////////////////////////////////////////////////////////////////////

RBLXWINAPI struct wl_output* rblxwinGetWaylandMonitor(RBLXWINmonitor* handle)
{
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) handle;
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->wl.output;
}

