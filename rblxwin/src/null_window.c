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


static int createNativeWindow(_RBLXWINwindow* window,
                              const _RBLXWINwndconfig* wndconfig)
{
    window->null.width = wndconfig->width;
    window->null.height = wndconfig->height;

    return RBLXWIN_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _rblxwinPlatformCreateWindow(_RBLXWINwindow* window,
                              const _RBLXWINwndconfig* wndconfig,
                              const _RBLXWINctxconfig* ctxconfig,
                              const _RBLXWINfbconfig* fbconfig)
{
    if (!createNativeWindow(window, wndconfig))
        return RBLXWIN_FALSE;

    if (ctxconfig->client != RBLXWIN_NO_API)
    {
        if (ctxconfig->source == RBLXWIN_NATIVE_CONTEXT_API ||
            ctxconfig->source == RBLXWIN_OSMESA_CONTEXT_API)
        {
            if (!_rblxwinInitOSMesa())
                return RBLXWIN_FALSE;
            if (!_rblxwinCreateContextOSMesa(window, ctxconfig, fbconfig))
                return RBLXWIN_FALSE;
        }
        else
        {
            _rblxwinInputError(RBLXWIN_API_UNAVAILABLE, "Null: EGL not available");
            return RBLXWIN_FALSE;
        }

        if (!_rblxwinRefreshContextAttribs(window, ctxconfig))
            return RBLXWIN_FALSE;
    }

    return RBLXWIN_TRUE;
}

void _rblxwinPlatformDestroyWindow(_RBLXWINwindow* window)
{
    if (window->context.destroy)
        window->context.destroy(window);
}

void _rblxwinPlatformSetWindowTitle(_RBLXWINwindow* window, const char* title)
{
}

void _rblxwinPlatformSetWindowIcon(_RBLXWINwindow* window, int count,
                                const RBLXWINimage* images)
{
}

void _rblxwinPlatformSetWindowMonitor(_RBLXWINwindow* window,
                                   _RBLXWINmonitor* monitor,
                                   int xpos, int ypos,
                                   int width, int height,
                                   int refreshRate)
{
}

void _rblxwinPlatformGetWindowPos(_RBLXWINwindow* window, int* xpos, int* ypos)
{
}

void _rblxwinPlatformSetWindowPos(_RBLXWINwindow* window, int xpos, int ypos)
{
}

void _rblxwinPlatformGetWindowSize(_RBLXWINwindow* window, int* width, int* height)
{
    if (width)
        *width = window->null.width;
    if (height)
        *height = window->null.height;
}

void _rblxwinPlatformSetWindowSize(_RBLXWINwindow* window, int width, int height)
{
    window->null.width = width;
    window->null.height = height;
}

void _rblxwinPlatformSetWindowSizeLimits(_RBLXWINwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight)
{
}

void _rblxwinPlatformSetWindowAspectRatio(_RBLXWINwindow* window, int n, int d)
{
}

void _rblxwinPlatformGetFramebufferSize(_RBLXWINwindow* window, int* width, int* height)
{
    if (width)
        *width = window->null.width;
    if (height)
        *height = window->null.height;
}

void _rblxwinPlatformGetWindowFrameSize(_RBLXWINwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom)
{
}

void _rblxwinPlatformGetWindowContentScale(_RBLXWINwindow* window,
                                        float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.f;
    if (yscale)
        *yscale = 1.f;
}

void _rblxwinPlatformIconifyWindow(_RBLXWINwindow* window)
{
}

void _rblxwinPlatformRestoreWindow(_RBLXWINwindow* window)
{
}

void _rblxwinPlatformMaximizeWindow(_RBLXWINwindow* window)
{
}

int _rblxwinPlatformWindowMaximized(_RBLXWINwindow* window)
{
    return RBLXWIN_FALSE;
}

int _rblxwinPlatformWindowHovered(_RBLXWINwindow* window)
{
    return RBLXWIN_FALSE;
}

int _rblxwinPlatformFramebufferTransparent(_RBLXWINwindow* window)
{
    return RBLXWIN_FALSE;
}

void _rblxwinPlatformSetWindowResizable(_RBLXWINwindow* window, RBLXWINbool enabled)
{
}

void _rblxwinPlatformSetWindowDecorated(_RBLXWINwindow* window, RBLXWINbool enabled)
{
}

void _rblxwinPlatformSetWindowFloating(_RBLXWINwindow* window, RBLXWINbool enabled)
{
}

float _rblxwinPlatformGetWindowOpacity(_RBLXWINwindow* window)
{
    return 1.f;
}

void _rblxwinPlatformSetWindowOpacity(_RBLXWINwindow* window, float opacity)
{
}

void _rblxwinPlatformSetRawMouseMotion(_RBLXWINwindow *window, RBLXWINbool enabled)
{
}

RBLXWINbool _rblxwinPlatformRawMouseMotionSupported(void)
{
    return RBLXWIN_FALSE;
}

void _rblxwinPlatformShowWindow(_RBLXWINwindow* window)
{
}


void _rblxwinPlatformRequestWindowAttention(_RBLXWINwindow* window)
{
}

void _rblxwinPlatformUnhideWindow(_RBLXWINwindow* window)
{
}

void _rblxwinPlatformHideWindow(_RBLXWINwindow* window)
{
}

void _rblxwinPlatformFocusWindow(_RBLXWINwindow* window)
{
}

int _rblxwinPlatformWindowFocused(_RBLXWINwindow* window)
{
    return RBLXWIN_FALSE;
}

int _rblxwinPlatformWindowIconified(_RBLXWINwindow* window)
{
    return RBLXWIN_FALSE;
}

int _rblxwinPlatformWindowVisible(_RBLXWINwindow* window)
{
    return RBLXWIN_FALSE;
}

void _rblxwinPlatformPollEvents(void)
{
}

void _rblxwinPlatformWaitEvents(void)
{
}

void _rblxwinPlatformWaitEventsTimeout(double timeout)
{
}

void _rblxwinPlatformPostEmptyEvent(void)
{
}

void _rblxwinPlatformGetCursorPos(_RBLXWINwindow* window, double* xpos, double* ypos)
{
}

void _rblxwinPlatformSetCursorPos(_RBLXWINwindow* window, double x, double y)
{
}

void _rblxwinPlatformSetCursorMode(_RBLXWINwindow* window, int mode)
{
}

int _rblxwinPlatformCreateCursor(_RBLXWINcursor* cursor,
                              const RBLXWINimage* image,
                              int xhot, int yhot)
{
    return RBLXWIN_TRUE;
}

int _rblxwinPlatformCreateStandardCursor(_RBLXWINcursor* cursor, int shape)
{
    return RBLXWIN_TRUE;
}

void _rblxwinPlatformDestroyCursor(_RBLXWINcursor* cursor)
{
}

void _rblxwinPlatformSetCursor(_RBLXWINwindow* window, _RBLXWINcursor* cursor)
{
}

void _rblxwinPlatformSetClipboardString(const char* string)
{
}

const char* _rblxwinPlatformGetClipboardString(void)
{
    return NULL;
}

const char* _rblxwinPlatformGetScancodeName(int scancode)
{
    return "";
}

int _rblxwinPlatformGetKeyScancode(int key)
{
    return -1;
}

void _rblxwinPlatformGetRequiredInstanceExtensions(char** extensions)
{
}

int _rblxwinPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily)
{
    return RBLXWIN_FALSE;
}

VkResult _rblxwinPlatformCreateWindowSurface(VkInstance instance,
                                          _RBLXWINwindow* window,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR* surface)
{
    // This seems like the most appropriate error to return here
    return VK_ERROR_INITIALIZATION_FAILED;
}

