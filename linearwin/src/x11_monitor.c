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

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


// Check whether the display mode should be included in enumeration
//
static LINEARWINbool modeIsGood(const XRRModeInfo* mi)
{
    return (mi->modeFlags & RR_Interlace) == 0;
}

// Calculates the refresh rate, in Hz, from the specified RandR mode info
//
static int calculateRefreshRate(const XRRModeInfo* mi)
{
    if (mi->hTotal && mi->vTotal)
        return (int) round((double) mi->dotClock / ((double) mi->hTotal * (double) mi->vTotal));
    else
        return 0;
}

// Returns the mode info for a RandR mode XID
//
static const XRRModeInfo* getModeInfo(const XRRScreenResources* sr, RRMode id)
{
    for (int i = 0;  i < sr->nmode;  i++)
    {
        if (sr->modes[i].id == id)
            return sr->modes + i;
    }

    return NULL;
}

// Convert RandR mode info to LINEARWIN video mode
//
static LINEARWINvidmode vidmodeFromModeInfo(const XRRModeInfo* mi,
                                       const XRRCrtcInfo* ci)
{
    LINEARWINvidmode mode;

    if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
    {
        mode.width  = mi->height;
        mode.height = mi->width;
    }
    else
    {
        mode.width  = mi->width;
        mode.height = mi->height;
    }

    mode.refreshRate = calculateRefreshRate(mi);

    _linearwinSplitBPP(DefaultDepth(_linearwin.x11.display, _linearwin.x11.screen),
                  &mode.redBits, &mode.greenBits, &mode.blueBits);

    return mode;
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Poll for changes in the set of connected monitors
//
void _linearwinPollMonitorsX11(void)
{
    if (_linearwin.x11.randr.available && !_linearwin.x11.randr.monitorBroken)
    {
        int disconnectedCount, screenCount = 0;
        _LINEARWINmonitor** disconnected = NULL;
        XineramaScreenInfo* screens = NULL;
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_linearwin.x11.display,
                                                              _linearwin.x11.root);
        RROutput primary = XRRGetOutputPrimary(_linearwin.x11.display,
                                               _linearwin.x11.root);

        if (_linearwin.x11.xinerama.available)
            screens = XineramaQueryScreens(_linearwin.x11.display, &screenCount);

        disconnectedCount = _linearwin.monitorCount;
        if (disconnectedCount)
        {
            disconnected = calloc(_linearwin.monitorCount, sizeof(_LINEARWINmonitor*));
            memcpy(disconnected,
                   _linearwin.monitors,
                   _linearwin.monitorCount * sizeof(_LINEARWINmonitor*));
        }

        for (int i = 0;  i < sr->noutput;  i++)
        {
            int j, type, widthMM, heightMM;

            XRROutputInfo* oi = XRRGetOutputInfo(_linearwin.x11.display, sr, sr->outputs[i]);
            if (oi->connection != RR_Connected || oi->crtc == None)
            {
                XRRFreeOutputInfo(oi);
                continue;
            }

            for (j = 0;  j < disconnectedCount;  j++)
            {
                if (disconnected[j] &&
                    disconnected[j]->x11.output == sr->outputs[i])
                {
                    disconnected[j] = NULL;
                    break;
                }
            }

            if (j < disconnectedCount)
            {
                XRRFreeOutputInfo(oi);
                continue;
            }

            XRRCrtcInfo* ci = XRRGetCrtcInfo(_linearwin.x11.display, sr, oi->crtc);
            if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
            {
                widthMM  = oi->mm_height;
                heightMM = oi->mm_width;
            }
            else
            {
                widthMM  = oi->mm_width;
                heightMM = oi->mm_height;
            }

            if (widthMM <= 0 || heightMM <= 0)
            {
                // HACK: If RandR does not provide a physical size, assume the
                //       X11 default 96 DPI and calculate from the CRTC viewport
                // NOTE: These members are affected by rotation, unlike the mode
                //       info and output info members
                widthMM  = (int) (ci->width * 25.4f / 96.f);
                heightMM = (int) (ci->height * 25.4f / 96.f);
            }

            _LINEARWINmonitor* monitor = _linearwinAllocMonitor(oi->name, widthMM, heightMM);
            monitor->x11.output = sr->outputs[i];
            monitor->x11.crtc   = oi->crtc;

            for (j = 0;  j < screenCount;  j++)
            {
                if (screens[j].x_org == ci->x &&
                    screens[j].y_org == ci->y &&
                    screens[j].width == ci->width &&
                    screens[j].height == ci->height)
                {
                    monitor->x11.index = j;
                    break;
                }
            }

            if (monitor->x11.output == primary)
                type = _LINEARWIN_INSERT_FIRST;
            else
                type = _LINEARWIN_INSERT_LAST;

            _linearwinInputMonitor(monitor, LINEARWIN_CONNECTED, type);

            XRRFreeOutputInfo(oi);
            XRRFreeCrtcInfo(ci);
        }

        XRRFreeScreenResources(sr);

        if (screens)
            XFree(screens);

        for (int i = 0;  i < disconnectedCount;  i++)
        {
            if (disconnected[i])
                _linearwinInputMonitor(disconnected[i], LINEARWIN_DISCONNECTED, 0);
        }

        free(disconnected);
    }
    else
    {
        const int widthMM = DisplayWidthMM(_linearwin.x11.display, _linearwin.x11.screen);
        const int heightMM = DisplayHeightMM(_linearwin.x11.display, _linearwin.x11.screen);

        _linearwinInputMonitor(_linearwinAllocMonitor("Display", widthMM, heightMM),
                          LINEARWIN_CONNECTED,
                          _LINEARWIN_INSERT_FIRST);
    }
}

// Set the current video mode for the specified monitor
//
void _linearwinSetVideoModeX11(_LINEARWINmonitor* monitor, const LINEARWINvidmode* desired)
{
    if (_linearwin.x11.randr.available && !_linearwin.x11.randr.monitorBroken)
    {
        LINEARWINvidmode current;
        RRMode native = None;

        const LINEARWINvidmode* best = _linearwinChooseVideoMode(monitor, desired);
        _linearwinPlatformGetVideoMode(monitor, &current);
        if (_linearwinCompareVideoModes(&current, best) == 0)
            return;

        XRRScreenResources* sr =
            XRRGetScreenResourcesCurrent(_linearwin.x11.display, _linearwin.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_linearwin.x11.display, sr, monitor->x11.crtc);
        XRROutputInfo* oi = XRRGetOutputInfo(_linearwin.x11.display, sr, monitor->x11.output);

        for (int i = 0;  i < oi->nmode;  i++)
        {
            const XRRModeInfo* mi = getModeInfo(sr, oi->modes[i]);
            if (!modeIsGood(mi))
                continue;

            const LINEARWINvidmode mode = vidmodeFromModeInfo(mi, ci);
            if (_linearwinCompareVideoModes(best, &mode) == 0)
            {
                native = mi->id;
                break;
            }
        }

        if (native)
        {
            if (monitor->x11.oldMode == None)
                monitor->x11.oldMode = ci->mode;

            XRRSetCrtcConfig(_linearwin.x11.display,
                             sr, monitor->x11.crtc,
                             CurrentTime,
                             ci->x, ci->y,
                             native,
                             ci->rotation,
                             ci->outputs,
                             ci->noutput);
        }

        XRRFreeOutputInfo(oi);
        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
}

// Restore the saved (original) video mode for the specified monitor
//
void _linearwinRestoreVideoModeX11(_LINEARWINmonitor* monitor)
{
    if (_linearwin.x11.randr.available && !_linearwin.x11.randr.monitorBroken)
    {
        if (monitor->x11.oldMode == None)
            return;

        XRRScreenResources* sr =
            XRRGetScreenResourcesCurrent(_linearwin.x11.display, _linearwin.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_linearwin.x11.display, sr, monitor->x11.crtc);

        XRRSetCrtcConfig(_linearwin.x11.display,
                         sr, monitor->x11.crtc,
                         CurrentTime,
                         ci->x, ci->y,
                         monitor->x11.oldMode,
                         ci->rotation,
                         ci->outputs,
                         ci->noutput);

        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);

        monitor->x11.oldMode = None;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _linearwinPlatformFreeMonitor(_LINEARWINmonitor* monitor)
{
}

void _linearwinPlatformGetMonitorPos(_LINEARWINmonitor* monitor, int* xpos, int* ypos)
{
    if (_linearwin.x11.randr.available && !_linearwin.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr =
            XRRGetScreenResourcesCurrent(_linearwin.x11.display, _linearwin.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_linearwin.x11.display, sr, monitor->x11.crtc);

        if (ci)
        {
            if (xpos)
                *xpos = ci->x;
            if (ypos)
                *ypos = ci->y;

            XRRFreeCrtcInfo(ci);
        }

        XRRFreeScreenResources(sr);
    }
}

void _linearwinPlatformGetMonitorContentScale(_LINEARWINmonitor* monitor,
                                         float* xscale, float* yscale)
{
    if (xscale)
        *xscale = _linearwin.x11.contentScaleX;
    if (yscale)
        *yscale = _linearwin.x11.contentScaleY;
}

void _linearwinPlatformGetMonitorWorkarea(_LINEARWINmonitor* monitor, int* xpos, int* ypos, int* width, int* height)
{
    int areaX = 0, areaY = 0, areaWidth = 0, areaHeight = 0;

    if (_linearwin.x11.randr.available && !_linearwin.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr =
            XRRGetScreenResourcesCurrent(_linearwin.x11.display, _linearwin.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_linearwin.x11.display, sr, monitor->x11.crtc);

        areaX = ci->x;
        areaY = ci->y;

        const XRRModeInfo* mi = getModeInfo(sr, ci->mode);

        if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
        {
            areaWidth  = mi->height;
            areaHeight = mi->width;
        }
        else
        {
            areaWidth  = mi->width;
            areaHeight = mi->height;
        }

        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
    else
    {
        areaWidth  = DisplayWidth(_linearwin.x11.display, _linearwin.x11.screen);
        areaHeight = DisplayHeight(_linearwin.x11.display, _linearwin.x11.screen);
    }

    if (_linearwin.x11.NET_WORKAREA && _linearwin.x11.NET_CURRENT_DESKTOP)
    {
        Atom* extents = NULL;
        Atom* desktop = NULL;
        const unsigned long extentCount =
            _linearwinGetWindowPropertyX11(_linearwin.x11.root,
                                      _linearwin.x11.NET_WORKAREA,
                                      XA_CARDINAL,
                                      (unsigned char**) &extents);

        if (_linearwinGetWindowPropertyX11(_linearwin.x11.root,
                                      _linearwin.x11.NET_CURRENT_DESKTOP,
                                      XA_CARDINAL,
                                      (unsigned char**) &desktop) > 0)
        {
            if (extentCount >= 4 && *desktop < extentCount / 4)
            {
                const int globalX = extents[*desktop * 4 + 0];
                const int globalY = extents[*desktop * 4 + 1];
                const int globalWidth  = extents[*desktop * 4 + 2];
                const int globalHeight = extents[*desktop * 4 + 3];

                if (areaX < globalX)
                {
                    areaWidth -= globalX - areaX;
                    areaX = globalX;
                }

                if (areaY < globalY)
                {
                    areaHeight -= globalY - areaY;
                    areaY = globalY;
                }

                if (areaX + areaWidth > globalX + globalWidth)
                    areaWidth = globalX - areaX + globalWidth;
                if (areaY + areaHeight > globalY + globalHeight)
                    areaHeight = globalY - areaY + globalHeight;
            }
        }

        if (extents)
            XFree(extents);
        if (desktop)
            XFree(desktop);
    }

    if (xpos)
        *xpos = areaX;
    if (ypos)
        *ypos = areaY;
    if (width)
        *width = areaWidth;
    if (height)
        *height = areaHeight;
}

LINEARWINvidmode* _linearwinPlatformGetVideoModes(_LINEARWINmonitor* monitor, int* count)
{
    LINEARWINvidmode* result;

    *count = 0;

    if (_linearwin.x11.randr.available && !_linearwin.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr =
            XRRGetScreenResourcesCurrent(_linearwin.x11.display, _linearwin.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_linearwin.x11.display, sr, monitor->x11.crtc);
        XRROutputInfo* oi = XRRGetOutputInfo(_linearwin.x11.display, sr, monitor->x11.output);

        result = calloc(oi->nmode, sizeof(LINEARWINvidmode));

        for (int i = 0;  i < oi->nmode;  i++)
        {
            const XRRModeInfo* mi = getModeInfo(sr, oi->modes[i]);
            if (!modeIsGood(mi))
                continue;

            const LINEARWINvidmode mode = vidmodeFromModeInfo(mi, ci);
            int j;

            for (j = 0;  j < *count;  j++)
            {
                if (_linearwinCompareVideoModes(result + j, &mode) == 0)
                    break;
            }

            // Skip duplicate modes
            if (j < *count)
                continue;

            (*count)++;
            result[*count - 1] = mode;
        }

        XRRFreeOutputInfo(oi);
        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
    else
    {
        *count = 1;
        result = calloc(1, sizeof(LINEARWINvidmode));
        _linearwinPlatformGetVideoMode(monitor, result);
    }

    return result;
}

void _linearwinPlatformGetVideoMode(_LINEARWINmonitor* monitor, LINEARWINvidmode* mode)
{
    if (_linearwin.x11.randr.available && !_linearwin.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr =
            XRRGetScreenResourcesCurrent(_linearwin.x11.display, _linearwin.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_linearwin.x11.display, sr, monitor->x11.crtc);

        if (ci)
        {
            const XRRModeInfo* mi = getModeInfo(sr, ci->mode);
            if (mi)  // mi can be NULL if the monitor has been disconnected
                *mode = vidmodeFromModeInfo(mi, ci);

            XRRFreeCrtcInfo(ci);
        }

        XRRFreeScreenResources(sr);
    }
    else
    {
        mode->width = DisplayWidth(_linearwin.x11.display, _linearwin.x11.screen);
        mode->height = DisplayHeight(_linearwin.x11.display, _linearwin.x11.screen);
        mode->refreshRate = 0;

        _linearwinSplitBPP(DefaultDepth(_linearwin.x11.display, _linearwin.x11.screen),
                      &mode->redBits, &mode->greenBits, &mode->blueBits);
    }
}

LINEARWINbool _linearwinPlatformGetGammaRamp(_LINEARWINmonitor* monitor, LINEARWINgammaramp* ramp)
{
    if (_linearwin.x11.randr.available && !_linearwin.x11.randr.gammaBroken)
    {
        const size_t size = XRRGetCrtcGammaSize(_linearwin.x11.display,
                                                monitor->x11.crtc);
        XRRCrtcGamma* gamma = XRRGetCrtcGamma(_linearwin.x11.display,
                                              monitor->x11.crtc);

        _linearwinAllocGammaArrays(ramp, size);

        memcpy(ramp->red,   gamma->red,   size * sizeof(unsigned short));
        memcpy(ramp->green, gamma->green, size * sizeof(unsigned short));
        memcpy(ramp->blue,  gamma->blue,  size * sizeof(unsigned short));

        XRRFreeGamma(gamma);
        return LINEARWIN_TRUE;
    }
    else if (_linearwin.x11.vidmode.available)
    {
        int size;
        XF86VidModeGetGammaRampSize(_linearwin.x11.display, _linearwin.x11.screen, &size);

        _linearwinAllocGammaArrays(ramp, size);

        XF86VidModeGetGammaRamp(_linearwin.x11.display,
                                _linearwin.x11.screen,
                                ramp->size, ramp->red, ramp->green, ramp->blue);
        return LINEARWIN_TRUE;
    }
    else
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "X11: Gamma ramp access not supported by server");
        return LINEARWIN_FALSE;
    }
}

void _linearwinPlatformSetGammaRamp(_LINEARWINmonitor* monitor, const LINEARWINgammaramp* ramp)
{
    if (_linearwin.x11.randr.available && !_linearwin.x11.randr.gammaBroken)
    {
        if (XRRGetCrtcGammaSize(_linearwin.x11.display, monitor->x11.crtc) != ramp->size)
        {
            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "X11: Gamma ramp size must match current ramp size");
            return;
        }

        XRRCrtcGamma* gamma = XRRAllocGamma(ramp->size);

        memcpy(gamma->red,   ramp->red,   ramp->size * sizeof(unsigned short));
        memcpy(gamma->green, ramp->green, ramp->size * sizeof(unsigned short));
        memcpy(gamma->blue,  ramp->blue,  ramp->size * sizeof(unsigned short));

        XRRSetCrtcGamma(_linearwin.x11.display, monitor->x11.crtc, gamma);
        XRRFreeGamma(gamma);
    }
    else if (_linearwin.x11.vidmode.available)
    {
        XF86VidModeSetGammaRamp(_linearwin.x11.display,
                                _linearwin.x11.screen,
                                ramp->size,
                                (unsigned short*) ramp->red,
                                (unsigned short*) ramp->green,
                                (unsigned short*) ramp->blue);
    }
    else
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "X11: Gamma ramp access not supported by server");
    }
}


//////////////////////////////////////////////////////////////////////////
//////                        LINEARWIN native API                       //////
//////////////////////////////////////////////////////////////////////////

LINEARWINAPI RRCrtc linearwinGetX11Adapter(LINEARWINmonitor* handle)
{
    _LINEARWINmonitor* monitor = (_LINEARWINmonitor*) handle;
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(None);
    return monitor->x11.crtc;
}

LINEARWINAPI RROutput linearwinGetX11Monitor(LINEARWINmonitor* handle)
{
    _LINEARWINmonitor* monitor = (_LINEARWINmonitor*) handle;
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(None);
    return monitor->x11.output;
}

