//========================================================================
// RBLXWIN 3.3 - www.rblxwin.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2019 Camilla Löwy <elmindreda@rblxwin.org>
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


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _rblxwinPlatformFreeMonitor(_RBLXWINmonitor* monitor)
{
}

void _rblxwinPlatformGetMonitorPos(_RBLXWINmonitor* monitor, int* xpos, int* ypos)
{
}

void _rblxwinPlatformGetMonitorContentScale(_RBLXWINmonitor* monitor,
                                         float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.f;
    if (yscale)
        *yscale = 1.f;
}

void _rblxwinPlatformGetMonitorWorkarea(_RBLXWINmonitor* monitor,
                                     int* xpos, int* ypos,
                                     int* width, int* height)
{
}

RBLXWINvidmode* _rblxwinPlatformGetVideoModes(_RBLXWINmonitor* monitor, int* found)
{
    return NULL;
}

void _rblxwinPlatformGetVideoMode(_RBLXWINmonitor* monitor, RBLXWINvidmode* mode)
{
}

RBLXWINbool _rblxwinPlatformGetGammaRamp(_RBLXWINmonitor* monitor, RBLXWINgammaramp* ramp)
{
    return RBLXWIN_FALSE;
}

void _rblxwinPlatformSetGammaRamp(_RBLXWINmonitor* monitor, const RBLXWINgammaramp* ramp)
{
}

