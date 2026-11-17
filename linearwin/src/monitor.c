//========================================================================
// LINEARWIN 3.3 - www.linearwin.org
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
    const LINEARWINvidmode* fm = fp;
    const LINEARWINvidmode* sm = sp;
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
static LINEARWINbool refreshVideoModes(_LINEARWINmonitor* monitor)
{
    int modeCount;
    LINEARWINvidmode* modes;

    if (monitor->modes)
        return LINEARWIN_TRUE;

    modes = _linearwinPlatformGetVideoModes(monitor, &modeCount);
    if (!modes)
        return LINEARWIN_FALSE;

    qsort(modes, modeCount, sizeof(LINEARWINvidmode), compareVideoModes);

    free(monitor->modes);
    monitor->modes = modes;
    monitor->modeCount = modeCount;

    return LINEARWIN_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                         LINEARWIN event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code of a monitor connection or disconnection
//
void _linearwinInputMonitor(_LINEARWINmonitor* monitor, int action, int placement)
{
    if (action == LINEARWIN_CONNECTED)
    {
        _linearwin.monitorCount++;
        _linearwin.monitors =
            realloc(_linearwin.monitors, sizeof(_LINEARWINmonitor*) * _linearwin.monitorCount);

        if (placement == _LINEARWIN_INSERT_FIRST)
        {
            memmove(_linearwin.monitors + 1,
                    _linearwin.monitors,
                    ((size_t) _linearwin.monitorCount - 1) * sizeof(_LINEARWINmonitor*));
            _linearwin.monitors[0] = monitor;
        }
        else
            _linearwin.monitors[_linearwin.monitorCount - 1] = monitor;
    }
    else if (action == LINEARWIN_DISCONNECTED)
    {
        int i;
        _LINEARWINwindow* window;

        for (window = _linearwin.windowListHead;  window;  window = window->next)
        {
            if (window->monitor == monitor)
            {
                int width, height, xoff, yoff;
                _linearwinPlatformGetWindowSize(window, &width, &height);
                _linearwinPlatformSetWindowMonitor(window, NULL, 0, 0, width, height, 0);
                _linearwinPlatformGetWindowFrameSize(window, &xoff, &yoff, NULL, NULL);
                _linearwinPlatformSetWindowPos(window, xoff, yoff);
            }
        }

        for (i = 0;  i < _linearwin.monitorCount;  i++)
        {
            if (_linearwin.monitors[i] == monitor)
            {
                _linearwin.monitorCount--;
                memmove(_linearwin.monitors + i,
                        _linearwin.monitors + i + 1,
                        ((size_t) _linearwin.monitorCount - i) * sizeof(_LINEARWINmonitor*));
                break;
            }
        }
    }

    if (_linearwin.callbacks.monitor)
        _linearwin.callbacks.monitor((LINEARWINmonitor*) monitor, action);

    if (action == LINEARWIN_DISCONNECTED)
        _linearwinFreeMonitor(monitor);
}

// Notifies shared code that a full screen window has acquired or released
// a monitor
//
void _linearwinInputMonitorWindow(_LINEARWINmonitor* monitor, _LINEARWINwindow* window)
{
    monitor->window = window;
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Allocates and returns a monitor object with the specified name and dimensions
//
_LINEARWINmonitor* _linearwinAllocMonitor(const char* name, int widthMM, int heightMM)
{
    _LINEARWINmonitor* monitor = calloc(1, sizeof(_LINEARWINmonitor));
    monitor->widthMM = widthMM;
    monitor->heightMM = heightMM;

    strncpy(monitor->name, name, sizeof(monitor->name) - 1);

    return monitor;
}

// Frees a monitor object and any data associated with it
//
void _linearwinFreeMonitor(_LINEARWINmonitor* monitor)
{
    if (monitor == NULL)
        return;

    _linearwinPlatformFreeMonitor(monitor);

    _linearwinFreeGammaArrays(&monitor->originalRamp);
    _linearwinFreeGammaArrays(&monitor->currentRamp);

    free(monitor->modes);
    free(monitor);
}

// Allocates red, green and blue value arrays of the specified size
//
void _linearwinAllocGammaArrays(LINEARWINgammaramp* ramp, unsigned int size)
{
    ramp->red = calloc(size, sizeof(unsigned short));
    ramp->green = calloc(size, sizeof(unsigned short));
    ramp->blue = calloc(size, sizeof(unsigned short));
    ramp->size = size;
}

// Frees the red, green and blue value arrays and clears the struct
//
void _linearwinFreeGammaArrays(LINEARWINgammaramp* ramp)
{
    free(ramp->red);
    free(ramp->green);
    free(ramp->blue);

    memset(ramp, 0, sizeof(LINEARWINgammaramp));
}

// Chooses the video mode most closely matching the desired one
//
const LINEARWINvidmode* _linearwinChooseVideoMode(_LINEARWINmonitor* monitor,
                                        const LINEARWINvidmode* desired)
{
    int i;
    unsigned int sizeDiff, leastSizeDiff = UINT_MAX;
    unsigned int rateDiff, leastRateDiff = UINT_MAX;
    unsigned int colorDiff, leastColorDiff = UINT_MAX;
    const LINEARWINvidmode* current;
    const LINEARWINvidmode* closest = NULL;

    if (!refreshVideoModes(monitor))
        return NULL;

    for (i = 0;  i < monitor->modeCount;  i++)
    {
        current = monitor->modes + i;

        colorDiff = 0;

        if (desired->redBits != LINEARWIN_DONT_CARE)
            colorDiff += abs(current->redBits - desired->redBits);
        if (desired->greenBits != LINEARWIN_DONT_CARE)
            colorDiff += abs(current->greenBits - desired->greenBits);
        if (desired->blueBits != LINEARWIN_DONT_CARE)
            colorDiff += abs(current->blueBits - desired->blueBits);

        sizeDiff = abs((current->width - desired->width) *
                       (current->width - desired->width) +
                       (current->height - desired->height) *
                       (current->height - desired->height));

        if (desired->refreshRate != LINEARWIN_DONT_CARE)
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

// Performs lexical comparison between two @ref LINEARWINvidmode structures
//
int _linearwinCompareVideoModes(const LINEARWINvidmode* fm, const LINEARWINvidmode* sm)
{
    return compareVideoModes(fm, sm);
}

// Splits a color depth into red, green and blue bit depths
//
void _linearwinSplitBPP(int bpp, int* red, int* green, int* blue)
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
//////                        LINEARWIN public API                       //////
//////////////////////////////////////////////////////////////////////////

LINEARWINAPI LINEARWINmonitor** linearwinGetMonitors(int* count)
{
    assert(count != NULL);

    *count = 0;

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    *count = _linearwin.monitorCount;
    return (LINEARWINmonitor**) _linearwin.monitors;
}

LINEARWINAPI LINEARWINmonitor* linearwinGetPrimaryMonitor(void)
{
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_linearwin.monitorCount)
        return NULL;

    return (LINEARWINmonitor*) _linearwin.monitors[0];
}

LINEARWINAPI void linearwinGetMonitorPos(LINEARWINmonitor* handle, int* xpos, int* ypos)
{
    _LINEARWINmonitor* monitor = (_LINEARWINmonitor*) handle;
    assert(monitor != NULL);

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;

    _LINEARWIN_REQUIRE_INIT();

    _linearwinPlatformGetMonitorPos(monitor, xpos, ypos);
}

LINEARWINAPI void linearwinGetMonitorWorkarea(LINEARWINmonitor* handle,
                                    int* xpos, int* ypos,
                                    int* width, int* height)
{
    _LINEARWINmonitor* monitor = (_LINEARWINmonitor*) handle;
    assert(monitor != NULL);

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;
    if (width)
        *width = 0;
    if (height)
        *height = 0;

    _LINEARWIN_REQUIRE_INIT();

    _linearwinPlatformGetMonitorWorkarea(monitor, xpos, ypos, width, height);
}

LINEARWINAPI void linearwinGetMonitorPhysicalSize(LINEARWINmonitor* handle, int* widthMM, int* heightMM)
{
    _LINEARWINmonitor* monitor = (_LINEARWINmonitor*) handle;
    assert(monitor != NULL);

    if (widthMM)
        *widthMM = 0;
    if (heightMM)
        *heightMM = 0;

    _LINEARWIN_REQUIRE_INIT();

    if (widthMM)
        *widthMM = monitor->widthMM;
    if (heightMM)
        *heightMM = monitor->heightMM;
}

LINEARWINAPI void linearwinGetMonitorContentScale(LINEARWINmonitor* handle,
                                        float* xscale, float* yscale)
{
    _LINEARWINmonitor* monitor = (_LINEARWINmonitor*) handle;
    assert(monitor != NULL);

    if (xscale)
        *xscale = 0.f;
    if (yscale)
        *yscale = 0.f;

    _LINEARWIN_REQUIRE_INIT();
    _linearwinPlatformGetMonitorContentScale(monitor, xscale, yscale);
}

LINEARWINAPI const char* linearwinGetMonitorName(LINEARWINmonitor* handle)
{
    _LINEARWINmonitor* monitor = (_LINEARWINmonitor*) handle;
    assert(monitor != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->name;
}

LINEARWINAPI void linearwinSetMonitorUserPointer(LINEARWINmonitor* handle, void* pointer)
{
    _LINEARWINmonitor* monitor = (_LINEARWINmonitor*) handle;
    assert(monitor != NULL);

    _LINEARWIN_REQUIRE_INIT();
    monitor->userPointer = pointer;
}

LINEARWINAPI void* linearwinGetMonitorUserPointer(LINEARWINmonitor* handle)
{
    _LINEARWINmonitor* monitor = (_LINEARWINmonitor*) handle;
    assert(monitor != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->userPointer;
}

LINEARWINAPI LINEARWINmonitorfun linearwinSetMonitorCallback(LINEARWINmonitorfun cbfun)
{
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(_linearwin.callbacks.monitor, cbfun);
    return cbfun;
}

LINEARWINAPI const LINEARWINvidmode* linearwinGetVideoModes(LINEARWINmonitor* handle, int* count)
{
    _LINEARWINmonitor* monitor = (_LINEARWINmonitor*) handle;
    assert(monitor != NULL);
    assert(count != NULL);

    *count = 0;

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (!refreshVideoModes(monitor))
        return NULL;

    *count = monitor->modeCount;
    return monitor->modes;
}

LINEARWINAPI const LINEARWINvidmode* linearwinGetVideoMode(LINEARWINmonitor* handle)
{
    _LINEARWINmonitor* monitor = (_LINEARWINmonitor*) handle;
    assert(monitor != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    _linearwinPlatformGetVideoMode(monitor, &monitor->currentMode);
    return &monitor->currentMode;
}

LINEARWINAPI void linearwinSetGamma(LINEARWINmonitor* handle, float gamma)
{
    unsigned int i;
    unsigned short* values;
    LINEARWINgammaramp ramp;
    const LINEARWINgammaramp* original;
    assert(handle != NULL);
    assert(gamma > 0.f);
    assert(gamma <= FLT_MAX);

    _LINEARWIN_REQUIRE_INIT();

    if (gamma != gamma || gamma <= 0.f || gamma > FLT_MAX)
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE, "Invalid gamma value %f", gamma);
        return;
    }

    original = linearwinGetGammaRamp(handle);
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
        value = _linearwin_fminf(value, 65535.f);

        values[i] = (unsigned short) value;
    }

    ramp.red = values;
    ramp.green = values;
    ramp.blue = values;
    ramp.size = original->size;

    linearwinSetGammaRamp(handle, &ramp);
    free(values);
}

LINEARWINAPI const LINEARWINgammaramp* linearwinGetGammaRamp(LINEARWINmonitor* handle)
{
    _LINEARWINmonitor* monitor = (_LINEARWINmonitor*) handle;
    assert(monitor != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    _linearwinFreeGammaArrays(&monitor->currentRamp);
    if (!_linearwinPlatformGetGammaRamp(monitor, &monitor->currentRamp))
        return NULL;

    return &monitor->currentRamp;
}

LINEARWINAPI void linearwinSetGammaRamp(LINEARWINmonitor* handle, const LINEARWINgammaramp* ramp)
{
    _LINEARWINmonitor* monitor = (_LINEARWINmonitor*) handle;
    assert(monitor != NULL);
    assert(ramp != NULL);
    assert(ramp->size > 0);
    assert(ramp->red != NULL);
    assert(ramp->green != NULL);
    assert(ramp->blue != NULL);

    _LINEARWIN_REQUIRE_INIT();

    if (ramp->size <= 0)
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE,
                        "Invalid gamma ramp size %i",
                        ramp->size);
        return;
    }

    if (!monitor->originalRamp.size)
    {
        if (!_linearwinPlatformGetGammaRamp(monitor, &monitor->originalRamp))
            return;
    }

    _linearwinPlatformSetGammaRamp(monitor, ramp);
}

