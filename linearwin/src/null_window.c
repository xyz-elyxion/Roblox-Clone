//========================================================================
// LINEARWIN 3.3 - www.linearwin.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2019 Camilla Löwy <elmindreda@linearwin.org>
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


static int createNativeWindow(_LINEARWINwindow* window,
                              const _LINEARWINwndconfig* wndconfig)
{
    window->null.width = wndconfig->width;
    window->null.height = wndconfig->height;

    return LINEARWIN_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _linearwinPlatformCreateWindow(_LINEARWINwindow* window,
                              const _LINEARWINwndconfig* wndconfig,
                              const _LINEARWINctxconfig* ctxconfig,
                              const _LINEARWINfbconfig* fbconfig)
{
    if (!createNativeWindow(window, wndconfig))
        return LINEARWIN_FALSE;

    if (ctxconfig->client != LINEARWIN_NO_API)
    {
        if (ctxconfig->source == LINEARWIN_NATIVE_CONTEXT_API ||
            ctxconfig->source == LINEARWIN_OSMESA_CONTEXT_API)
        {
            if (!_linearwinInitOSMesa())
                return LINEARWIN_FALSE;
            if (!_linearwinCreateContextOSMesa(window, ctxconfig, fbconfig))
                return LINEARWIN_FALSE;
        }
        else
        {
            _linearwinInputError(LINEARWIN_API_UNAVAILABLE, "Null: EGL not available");
            return LINEARWIN_FALSE;
        }

        if (!_linearwinRefreshContextAttribs(window, ctxconfig))
            return LINEARWIN_FALSE;
    }

    return LINEARWIN_TRUE;
}

void _linearwinPlatformDestroyWindow(_LINEARWINwindow* window)
{
    if (window->context.destroy)
        window->context.destroy(window);
}

void _linearwinPlatformSetWindowTitle(_LINEARWINwindow* window, const char* title)
{
}

void _linearwinPlatformSetWindowIcon(_LINEARWINwindow* window, int count,
                                const LINEARWINimage* images)
{
}

void _linearwinPlatformSetWindowMonitor(_LINEARWINwindow* window,
                                   _LINEARWINmonitor* monitor,
                                   int xpos, int ypos,
                                   int width, int height,
                                   int refreshRate)
{
}

void _linearwinPlatformGetWindowPos(_LINEARWINwindow* window, int* xpos, int* ypos)
{
}

void _linearwinPlatformSetWindowPos(_LINEARWINwindow* window, int xpos, int ypos)
{
}

void _linearwinPlatformGetWindowSize(_LINEARWINwindow* window, int* width, int* height)
{
    if (width)
        *width = window->null.width;
    if (height)
        *height = window->null.height;
}

void _linearwinPlatformSetWindowSize(_LINEARWINwindow* window, int width, int height)
{
    window->null.width = width;
    window->null.height = height;
}

void _linearwinPlatformSetWindowSizeLimits(_LINEARWINwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight)
{
}

void _linearwinPlatformSetWindowAspectRatio(_LINEARWINwindow* window, int n, int d)
{
}

void _linearwinPlatformGetFramebufferSize(_LINEARWINwindow* window, int* width, int* height)
{
    if (width)
        *width = window->null.width;
    if (height)
        *height = window->null.height;
}

void _linearwinPlatformGetWindowFrameSize(_LINEARWINwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom)
{
}

void _linearwinPlatformGetWindowContentScale(_LINEARWINwindow* window,
                                        float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.f;
    if (yscale)
        *yscale = 1.f;
}

void _linearwinPlatformIconifyWindow(_LINEARWINwindow* window)
{
}

void _linearwinPlatformRestoreWindow(_LINEARWINwindow* window)
{
}

void _linearwinPlatformMaximizeWindow(_LINEARWINwindow* window)
{
}

int _linearwinPlatformWindowMaximized(_LINEARWINwindow* window)
{
    return LINEARWIN_FALSE;
}

int _linearwinPlatformWindowHovered(_LINEARWINwindow* window)
{
    return LINEARWIN_FALSE;
}

int _linearwinPlatformFramebufferTransparent(_LINEARWINwindow* window)
{
    return LINEARWIN_FALSE;
}

void _linearwinPlatformSetWindowResizable(_LINEARWINwindow* window, LINEARWINbool enabled)
{
}

void _linearwinPlatformSetWindowDecorated(_LINEARWINwindow* window, LINEARWINbool enabled)
{
}

void _linearwinPlatformSetWindowFloating(_LINEARWINwindow* window, LINEARWINbool enabled)
{
}

float _linearwinPlatformGetWindowOpacity(_LINEARWINwindow* window)
{
    return 1.f;
}

void _linearwinPlatformSetWindowOpacity(_LINEARWINwindow* window, float opacity)
{
}

void _linearwinPlatformSetRawMouseMotion(_LINEARWINwindow *window, LINEARWINbool enabled)
{
}

LINEARWINbool _linearwinPlatformRawMouseMotionSupported(void)
{
    return LINEARWIN_FALSE;
}

void _linearwinPlatformShowWindow(_LINEARWINwindow* window)
{
}


void _linearwinPlatformRequestWindowAttention(_LINEARWINwindow* window)
{
}

void _linearwinPlatformUnhideWindow(_LINEARWINwindow* window)
{
}

void _linearwinPlatformHideWindow(_LINEARWINwindow* window)
{
}

void _linearwinPlatformFocusWindow(_LINEARWINwindow* window)
{
}

int _linearwinPlatformWindowFocused(_LINEARWINwindow* window)
{
    return LINEARWIN_FALSE;
}

int _linearwinPlatformWindowIconified(_LINEARWINwindow* window)
{
    return LINEARWIN_FALSE;
}

int _linearwinPlatformWindowVisible(_LINEARWINwindow* window)
{
    return LINEARWIN_FALSE;
}

void _linearwinPlatformPollEvents(void)
{
}

void _linearwinPlatformWaitEvents(void)
{
}

void _linearwinPlatformWaitEventsTimeout(double timeout)
{
}

void _linearwinPlatformPostEmptyEvent(void)
{
}

void _linearwinPlatformGetCursorPos(_LINEARWINwindow* window, double* xpos, double* ypos)
{
}

void _linearwinPlatformSetCursorPos(_LINEARWINwindow* window, double x, double y)
{
}

void _linearwinPlatformSetCursorMode(_LINEARWINwindow* window, int mode)
{
}

int _linearwinPlatformCreateCursor(_LINEARWINcursor* cursor,
                              const LINEARWINimage* image,
                              int xhot, int yhot)
{
    return LINEARWIN_TRUE;
}

int _linearwinPlatformCreateStandardCursor(_LINEARWINcursor* cursor, int shape)
{
    return LINEARWIN_TRUE;
}

void _linearwinPlatformDestroyCursor(_LINEARWINcursor* cursor)
{
}

void _linearwinPlatformSetCursor(_LINEARWINwindow* window, _LINEARWINcursor* cursor)
{
}

void _linearwinPlatformSetClipboardString(const char* string)
{
}

const char* _linearwinPlatformGetClipboardString(void)
{
    return NULL;
}

const char* _linearwinPlatformGetScancodeName(int scancode)
{
    return "";
}

int _linearwinPlatformGetKeyScancode(int key)
{
    return -1;
}

void _linearwinPlatformGetRequiredInstanceExtensions(char** extensions)
{
}

int _linearwinPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily)
{
    return LINEARWIN_FALSE;
}

VkResult _linearwinPlatformCreateWindowSurface(VkInstance instance,
                                          _LINEARWINwindow* window,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR* surface)
{
    // This seems like the most appropriate error to return here
    return VK_ERROR_INITIALIZATION_FAILED;
}

