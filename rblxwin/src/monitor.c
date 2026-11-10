//========================================================================
// RBLXWIN 3.3 - www.rblxwin.org
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
// Please use C89 style variable declarations in this file because VS 2010
//========================================================================

#include "internal.h"

#include <assert.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>


// Lexically compare video modes, used by qsort
//
static int compareVideoModes(const void* fp, const void* sp)
{
    const RBLXWINvidmode* fm = fp;
    const RBLXWINvidmode* sm = sp;
    const int fbpp = fm->redBits + fm->greenBits + fm->blueBits;
    const int sbpp = sm->redBits + sm->greenBits + sm->blueBits;
    const int farea = fm->width * fm->height;
    const int sarea = sm->width * sm->height;

    // First sort on color bits per pixel
    if (fbpp != sbpp)
        return fbpp - sbpp;

    // Then sort on screen area
    if (farea != sarea)
        return farea - sarea;

    // Then sort on width
    if (fm->width != sm->width)
        return fm->width - sm->width;

    // Lastly sort on refresh rate
    return fm->refreshRate - sm->refreshRate;
}

// Retrieves the available modes for the specified monitor
//
static RBLXWINbool refreshVideoModes(_RBLXWINmonitor* monitor)
{
    int modeCount;
    RBLXWINvidmode* modes;

    if (monitor->modes)
        return RBLXWIN_TRUE;

    modes = _rblxwinPlatformGetVideoModes(monitor, &modeCount);
    if (!modes)
        return RBLXWIN_FALSE;

    qsort(modes, modeCount, sizeof(RBLXWINvidmode), compareVideoModes);

    free(monitor->modes);
    monitor->modes = modes;
    monitor->modeCount = modeCount;

    return RBLXWIN_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                         RBLXWIN event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code of a monitor connection or disconnection
//
void _rblxwinInputMonitor(_RBLXWINmonitor* monitor, int action, int placement)
{
    if (action == RBLXWIN_CONNECTED)
    {
        _rblxwin.monitorCount++;
        _rblxwin.monitors =
            realloc(_rblxwin.monitors, sizeof(_RBLXWINmonitor*) * _rblxwin.monitorCount);

        if (placement == _RBLXWIN_INSERT_FIRST)
        {
            memmove(_rblxwin.monitors + 1,
                    _rblxwin.monitors,
                    ((size_t) _rblxwin.monitorCount - 1) * sizeof(_RBLXWINmonitor*));
            _rblxwin.monitors[0] = monitor;
        }
        else
            _rblxwin.monitors[_rblxwin.monitorCount - 1] = monitor;
    }
    else if (action == RBLXWIN_DISCONNECTED)
    {
        int i;
        _RBLXWINwindow* window;

        for (window = _rblxwin.windowListHead;  window;  window = window->next)
        {
            if (window->monitor == monitor)
            {
                int width, height, xoff, yoff;
                _rblxwinPlatformGetWindowSize(window, &width, &height);
                _rblxwinPlatformSetWindowMonitor(window, NULL, 0, 0, width, height, 0);
                _rblxwinPlatformGetWindowFrameSize(window, &xoff, &yoff, NULL, NULL);
                _rblxwinPlatformSetWindowPos(window, xoff, yoff);
            }
        }

        for (i = 0;  i < _rblxwin.monitorCount;  i++)
        {
            if (_rblxwin.monitors[i] == monitor)
            {
                _rblxwin.monitorCount--;
                memmove(_rblxwin.monitors + i,
                        _rblxwin.monitors + i + 1,
                        ((size_t) _rblxwin.monitorCount - i) * sizeof(_RBLXWINmonitor*));
                break;
            }
        }
    }

    if (_rblxwin.callbacks.monitor)
        _rblxwin.callbacks.monitor((RBLXWINmonitor*) monitor, action);

    if (action == RBLXWIN_DISCONNECTED)
        _rblxwinFreeMonitor(monitor);
}

// Notifies shared code that a full screen window has acquired or released
// a monitor
//
void _rblxwinInputMonitorWindow(_RBLXWINmonitor* monitor, _RBLXWINwindow* window)
{
    monitor->window = window;
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Allocates and returns a monitor object with the specified name and dimensions
//
_RBLXWINmonitor* _rblxwinAllocMonitor(const char* name, int widthMM, int heightMM)
{
    _RBLXWINmonitor* monitor = calloc(1, sizeof(_RBLXWINmonitor));
    monitor->widthMM = widthMM;
    monitor->heightMM = heightMM;

    strncpy(monitor->name, name, sizeof(monitor->name) - 1);

    return monitor;
}

// Frees a monitor object and any data associated with it
//
void _rblxwinFreeMonitor(_RBLXWINmonitor* monitor)
{
    if (monitor == NULL)
        return;

    _rblxwinPlatformFreeMonitor(monitor);

    _rblxwinFreeGammaArrays(&monitor->originalRamp);
    _rblxwinFreeGammaArrays(&monitor->currentRamp);

    free(monitor->modes);
    free(monitor);
}

// Allocates red, green and blue value arrays of the specified size
//
void _rblxwinAllocGammaArrays(RBLXWINgammaramp* ramp, unsigned int size)
{
    ramp->red = calloc(size, sizeof(unsigned short));
    ramp->green = calloc(size, sizeof(unsigned short));
    ramp->blue = calloc(size, sizeof(unsigned short));
    ramp->size = size;
}

// Frees the red, green and blue value arrays and clears the struct
//
void _rblxwinFreeGammaArrays(RBLXWINgammaramp* ramp)
{
    free(ramp->red);
    free(ramp->green);
    free(ramp->blue);

    memset(ramp, 0, sizeof(RBLXWINgammaramp));
}

// Chooses the video mode most closely matching the desired one
//
const RBLXWINvidmode* _rblxwinChooseVideoMode(_RBLXWINmonitor* monitor,
                                        const RBLXWINvidmode* desired)
{
    int i;
    unsigned int sizeDiff, leastSizeDiff = UINT_MAX;
    unsigned int rateDiff, leastRateDiff = UINT_MAX;
    unsigned int colorDiff, leastColorDiff = UINT_MAX;
    const RBLXWINvidmode* current;
    const RBLXWINvidmode* closest = NULL;

    if (!refreshVideoModes(monitor))
        return NULL;

    for (i = 0;  i < monitor->modeCount;  i++)
    {
        current = monitor->modes + i;

        colorDiff = 0;

        if (desired->redBits != RBLXWIN_DONT_CARE)
            colorDiff += abs(current->redBits - desired->redBits);
        if (desired->greenBits != RBLXWIN_DONT_CARE)
            colorDiff += abs(current->greenBits - desired->greenBits);
        if (desired->blueBits != RBLXWIN_DONT_CARE)
            colorDiff += abs(current->blueBits - desired->blueBits);

        sizeDiff = abs((current->width - desired->width) *
                       (current->width - desired->width) +
                       (current->height - desired->height) *
                       (current->height - desired->height));

        if (desired->refreshRate != RBLXWIN_DONT_CARE)
            rateDiff = abs(current->refreshRate - desired->refreshRate);
        else
            rateDiff = UINT_MAX - current->refreshRate;

        if ((colorDiff < leastColorDiff) ||
            (colorDiff == leastColorDiff && sizeDiff < leastSizeDiff) ||
            (colorDiff == leastColorDiff && sizeDiff == leastSizeDiff && rateDiff < leastRateDiff))
        {
            closest = current;
            leastSizeDiff = sizeDiff;
            leastRateDiff = rateDiff;
            leastColorDiff = colorDiff;
        }
    }

    return closest;
}

// Performs lexical comparison between two @ref RBLXWINvidmode structures
//
int _rblxwinCompareVideoModes(const RBLXWINvidmode* fm, const RBLXWINvidmode* sm)
{
    return compareVideoModes(fm, sm);
}

// Splits a color depth into red, green and blue bit depths
//
void _rblxwinSplitBPP(int bpp, int* red, int* green, int* blue)
{
    int delta;

    // We assume that by 32 the user really meant 24
    if (bpp == 32)
        bpp = 24;

    // Convert "bits per pixel" to red, green & blue sizes

    *red = *green = *blue = bpp / 3;
    delta = bpp - (*red * 3);
    if (delta >= 1)
        *green = *green + 1;

    if (delta == 2)
        *red = *red + 1;
}


//////////////////////////////////////////////////////////////////////////
//////                        RBLXWIN public API                       //////
//////////////////////////////////////////////////////////////////////////

RBLXWINAPI RBLXWINmonitor** rblxwinGetMonitors(int* count)
{
    assert(count != NULL);

    *count = 0;

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    *count = _rblxwin.monitorCount;
    return (RBLXWINmonitor**) _rblxwin.monitors;
}

RBLXWINAPI RBLXWINmonitor* rblxwinGetPrimaryMonitor(void)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_rblxwin.monitorCount)
        return NULL;

    return (RBLXWINmonitor*) _rblxwin.monitors[0];
}

RBLXWINAPI void rblxwinGetMonitorPos(RBLXWINmonitor* handle, int* xpos, int* ypos)
{
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) handle;
    assert(monitor != NULL);

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;

    _RBLXWIN_REQUIRE_INIT();

    _rblxwinPlatformGetMonitorPos(monitor, xpos, ypos);
}

RBLXWINAPI void rblxwinGetMonitorWorkarea(RBLXWINmonitor* handle,
                                    int* xpos, int* ypos,
                                    int* width, int* height)
{
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) handle;
    assert(monitor != NULL);

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;
    if (width)
        *width = 0;
    if (height)
        *height = 0;

    _RBLXWIN_REQUIRE_INIT();

    _rblxwinPlatformGetMonitorWorkarea(monitor, xpos, ypos, width, height);
}

RBLXWINAPI void rblxwinGetMonitorPhysicalSize(RBLXWINmonitor* handle, int* widthMM, int* heightMM)
{
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) handle;
    assert(monitor != NULL);

    if (widthMM)
        *widthMM = 0;
    if (heightMM)
        *heightMM = 0;

    _RBLXWIN_REQUIRE_INIT();

    if (widthMM)
        *widthMM = monitor->widthMM;
    if (heightMM)
        *heightMM = monitor->heightMM;
}

RBLXWINAPI void rblxwinGetMonitorContentScale(RBLXWINmonitor* handle,
                                        float* xscale, float* yscale)
{
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) handle;
    assert(monitor != NULL);

    if (xscale)
        *xscale = 0.f;
    if (yscale)
        *yscale = 0.f;

    _RBLXWIN_REQUIRE_INIT();
    _rblxwinPlatformGetMonitorContentScale(monitor, xscale, yscale);
}

RBLXWINAPI const char* rblxwinGetMonitorName(RBLXWINmonitor* handle)
{
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) handle;
    assert(monitor != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->name;
}

RBLXWINAPI void rblxwinSetMonitorUserPointer(RBLXWINmonitor* handle, void* pointer)
{
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) handle;
    assert(monitor != NULL);

    _RBLXWIN_REQUIRE_INIT();
    monitor->userPointer = pointer;
}

RBLXWINAPI void* rblxwinGetMonitorUserPointer(RBLXWINmonitor* handle)
{
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) handle;
    assert(monitor != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->userPointer;
}

RBLXWINAPI RBLXWINmonitorfun rblxwinSetMonitorCallback(RBLXWINmonitorfun cbfun)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(_rblxwin.callbacks.monitor, cbfun);
    return cbfun;
}

RBLXWINAPI const RBLXWINvidmode* rblxwinGetVideoModes(RBLXWINmonitor* handle, int* count)
{
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) handle;
    assert(monitor != NULL);
    assert(count != NULL);

    *count = 0;

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (!refreshVideoModes(monitor))
        return NULL;

    *count = monitor->modeCount;
    return monitor->modes;
}

RBLXWINAPI const RBLXWINvidmode* rblxwinGetVideoMode(RBLXWINmonitor* handle)
{
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) handle;
    assert(monitor != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    _rblxwinPlatformGetVideoMode(monitor, &monitor->currentMode);
    return &monitor->currentMode;
}

RBLXWINAPI void rblxwinSetGamma(RBLXWINmonitor* handle, float gamma)
{
    unsigned int i;
    unsigned short* values;
    RBLXWINgammaramp ramp;
    const RBLXWINgammaramp* original;
    assert(handle != NULL);
    assert(gamma > 0.f);
    assert(gamma <= FLT_MAX);

    _RBLXWIN_REQUIRE_INIT();

    if (gamma != gamma || gamma <= 0.f || gamma > FLT_MAX)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE, "Invalid gamma value %f", gamma);
        return;
    }

    original = rblxwinGetGammaRamp(handle);
    if (!original)
        return;

    values = calloc(original->size, sizeof(unsigned short));

    for (i = 0;  i < original->size;  i++)
    {
        float value;

        // Calculate intensity
        value = i / (float) (original->size - 1);
        // Apply gamma curve
        value = powf(value, 1.f / gamma) * 65535.f + 0.5f;
        // Clamp to value range
        value = _rblxwin_fminf(value, 65535.f);

        values[i] = (unsigned short) value;
    }

    ramp.red = values;
    ramp.green = values;
    ramp.blue = values;
    ramp.size = original->size;

    rblxwinSetGammaRamp(handle, &ramp);
    free(values);
}

RBLXWINAPI const RBLXWINgammaramp* rblxwinGetGammaRamp(RBLXWINmonitor* handle)
{
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) handle;
    assert(monitor != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    _rblxwinFreeGammaArrays(&monitor->currentRamp);
    if (!_rblxwinPlatformGetGammaRamp(monitor, &monitor->currentRamp))
        return NULL;

    return &monitor->currentRamp;
}

RBLXWINAPI void rblxwinSetGammaRamp(RBLXWINmonitor* handle, const RBLXWINgammaramp* ramp)
{
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) handle;
    assert(monitor != NULL);
    assert(ramp != NULL);
    assert(ramp->size > 0);
    assert(ramp->red != NULL);
    assert(ramp->green != NULL);
    assert(ramp->blue != NULL);

    _RBLXWIN_REQUIRE_INIT();

    if (ramp->size <= 0)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                        "Invalid gamma ramp size %i",
                        ramp->size);
        return;
    }

    if (!monitor->originalRamp.size)
    {
        if (!_rblxwinPlatformGetGammaRamp(monitor, &monitor->originalRamp))
            return;
    }

    _rblxwinPlatformSetGammaRamp(monitor, ramp);
}

