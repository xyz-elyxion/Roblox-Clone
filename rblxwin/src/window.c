//========================================================================
// RBLXWIN 3.3 - www.rblxwin.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@rblxwin.org>
// Copyright (c) 2012 Torsten Walluhn <tw@mad-cad.net>
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
#include <string.h>
#include <stdlib.h>
#include <float.h>


//////////////////////////////////////////////////////////////////////////
//////                         RBLXWIN event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code that a window has lost or received input focus
//
void _rblxwinInputWindowFocus(_RBLXWINwindow* window, RBLXWINbool focused)
{
    if (window->callbacks.focus)
        window->callbacks.focus((RBLXWINwindow*) window, focused);

    if (!focused)
    {
        int key, button;

        for (key = 0;  key <= RBLXWIN_KEY_LAST;  key++)
        {
            if (window->keys[key] == RBLXWIN_PRESS)
            {
                const int scancode = _rblxwinPlatformGetKeyScancode(key);
                _rblxwinInputKey(window, key, scancode, RBLXWIN_RELEASE, 0);
            }
        }

        for (button = 0;  button <= RBLXWIN_MOUSE_BUTTON_LAST;  button++)
        {
            if (window->mouseButtons[button] == RBLXWIN_PRESS)
                _rblxwinInputMouseClick(window, button, RBLXWIN_RELEASE, 0);
        }
    }
}

// Notifies shared code that a window has moved
// The position is specified in content area relative screen coordinates
//
void _rblxwinInputWindowPos(_RBLXWINwindow* window, int x, int y)
{
    if (window->callbacks.pos)
        window->callbacks.pos((RBLXWINwindow*) window, x, y);
}

// Notifies shared code that a window has been resized
// The size is specified in screen coordinates
//
void _rblxwinInputWindowSize(_RBLXWINwindow* window, int width, int height)
{
    if (window->callbacks.size)
        window->callbacks.size((RBLXWINwindow*) window, width, height);
}

// Notifies shared code that a window has been iconified or restored
//
void _rblxwinInputWindowIconify(_RBLXWINwindow* window, RBLXWINbool iconified)
{
    if (window->callbacks.iconify)
        window->callbacks.iconify((RBLXWINwindow*) window, iconified);
}

// Notifies shared code that a window has been maximized or restored
//
void _rblxwinInputWindowMaximize(_RBLXWINwindow* window, RBLXWINbool maximized)
{
    if (window->callbacks.maximize)
        window->callbacks.maximize((RBLXWINwindow*) window, maximized);
}

// Notifies shared code that a window framebuffer has been resized
// The size is specified in pixels
//
void _rblxwinInputFramebufferSize(_RBLXWINwindow* window, int width, int height)
{
    if (window->callbacks.fbsize)
        window->callbacks.fbsize((RBLXWINwindow*) window, width, height);
}

// Notifies shared code that a window content scale has changed
// The scale is specified as the ratio between the current and default DPI
//
void _rblxwinInputWindowContentScale(_RBLXWINwindow* window, float xscale, float yscale)
{
    if (window->callbacks.scale)
        window->callbacks.scale((RBLXWINwindow*) window, xscale, yscale);
}

// Notifies shared code that the window contents needs updating
//
void _rblxwinInputWindowDamage(_RBLXWINwindow* window)
{
    if (window->callbacks.refresh)
        window->callbacks.refresh((RBLXWINwindow*) window);
}

// Notifies shared code that the user wishes to close a window
//
void _rblxwinInputWindowCloseRequest(_RBLXWINwindow* window)
{
    window->shouldClose = RBLXWIN_TRUE;

    if (window->callbacks.close)
        window->callbacks.close((RBLXWINwindow*) window);
}

// Notifies shared code that a window has changed its desired monitor
//
void _rblxwinInputWindowMonitor(_RBLXWINwindow* window, _RBLXWINmonitor* monitor)
{
    window->monitor = monitor;
}

//////////////////////////////////////////////////////////////////////////
//////                        RBLXWIN public API                       //////
//////////////////////////////////////////////////////////////////////////

RBLXWINAPI RBLXWINwindow* rblxwinCreateWindow(int width, int height,
                                     const char* title,
                                     RBLXWINmonitor* monitor,
                                     RBLXWINwindow* share)
{
    _RBLXWINfbconfig fbconfig;
    _RBLXWINctxconfig ctxconfig;
    _RBLXWINwndconfig wndconfig;
    _RBLXWINwindow* window;

    assert(title != NULL);
    assert(width >= 0);
    assert(height >= 0);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (width <= 0 || height <= 0)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                        "Invalid window size %ix%i",
                        width, height);

        return NULL;
    }

    fbconfig  = _rblxwin.hints.framebuffer;
    ctxconfig = _rblxwin.hints.context;
    wndconfig = _rblxwin.hints.window;

    wndconfig.width   = width;
    wndconfig.height  = height;
    wndconfig.title   = title;
    ctxconfig.share   = (_RBLXWINwindow*) share;

    if (!_rblxwinIsValidContextConfig(&ctxconfig))
        return NULL;

    window = calloc(1, sizeof(_RBLXWINwindow));
    window->next = _rblxwin.windowListHead;
    _rblxwin.windowListHead = window;

    window->videoMode.width       = width;
    window->videoMode.height      = height;
    window->videoMode.redBits     = fbconfig.redBits;
    window->videoMode.greenBits   = fbconfig.greenBits;
    window->videoMode.blueBits    = fbconfig.blueBits;
    window->videoMode.refreshRate = _rblxwin.hints.refreshRate;

    window->monitor     = (_RBLXWINmonitor*) monitor;
    window->resizable   = wndconfig.resizable;
    window->decorated   = wndconfig.decorated;
    window->autoIconify = wndconfig.autoIconify;
    window->floating    = wndconfig.floating;
    window->focusOnShow = wndconfig.focusOnShow;
    window->cursorMode  = RBLXWIN_CURSOR_NORMAL;

    window->doublebuffer = fbconfig.doublebuffer;

    window->minwidth    = RBLXWIN_DONT_CARE;
    window->minheight   = RBLXWIN_DONT_CARE;
    window->maxwidth    = RBLXWIN_DONT_CARE;
    window->maxheight   = RBLXWIN_DONT_CARE;
    window->numer       = RBLXWIN_DONT_CARE;
    window->denom       = RBLXWIN_DONT_CARE;

    // Open the actual window and create its context
    if (!_rblxwinPlatformCreateWindow(window, &wndconfig, &ctxconfig, &fbconfig))
    {
        rblxwinDestroyWindow((RBLXWINwindow*) window);
        return NULL;
    }

    return (RBLXWINwindow*) window;
}

void rblxwinDefaultWindowHints(void)
{
    _RBLXWIN_REQUIRE_INIT();

    // The default is OpenGL with minimum version 1.0
    memset(&_rblxwin.hints.context, 0, sizeof(_rblxwin.hints.context));
    _rblxwin.hints.context.client = RBLXWIN_OPENGL_API;
    _rblxwin.hints.context.source = RBLXWIN_NATIVE_CONTEXT_API;
    _rblxwin.hints.context.major  = 1;
    _rblxwin.hints.context.minor  = 0;

    // The default is a focused, visible, resizable window with decorations
    memset(&_rblxwin.hints.window, 0, sizeof(_rblxwin.hints.window));
    _rblxwin.hints.window.resizable    = RBLXWIN_TRUE;
    _rblxwin.hints.window.visible      = RBLXWIN_TRUE;
    _rblxwin.hints.window.decorated    = RBLXWIN_TRUE;
    _rblxwin.hints.window.focused      = RBLXWIN_TRUE;
    _rblxwin.hints.window.autoIconify  = RBLXWIN_TRUE;
    _rblxwin.hints.window.centerCursor = RBLXWIN_TRUE;
    _rblxwin.hints.window.focusOnShow  = RBLXWIN_TRUE;

    // The default is 24 bits of color, 24 bits of depth and 8 bits of stencil,
    // double buffered
    memset(&_rblxwin.hints.framebuffer, 0, sizeof(_rblxwin.hints.framebuffer));
    _rblxwin.hints.framebuffer.redBits      = 8;
    _rblxwin.hints.framebuffer.greenBits    = 8;
    _rblxwin.hints.framebuffer.blueBits     = 8;
    _rblxwin.hints.framebuffer.alphaBits    = 8;
    _rblxwin.hints.framebuffer.depthBits    = 24;
    _rblxwin.hints.framebuffer.stencilBits  = 8;
    _rblxwin.hints.framebuffer.doublebuffer = RBLXWIN_TRUE;

    // The default is to select the highest available refresh rate
    _rblxwin.hints.refreshRate = RBLXWIN_DONT_CARE;

    // The default is to use full Retina resolution framebuffers
    _rblxwin.hints.window.ns.retina = RBLXWIN_TRUE;
}

RBLXWINAPI void rblxwinWindowHint(int hint, int value)
{
    _RBLXWIN_REQUIRE_INIT();

    switch (hint)
    {
        case RBLXWIN_RED_BITS:
            _rblxwin.hints.framebuffer.redBits = value;
            return;
        case RBLXWIN_GREEN_BITS:
            _rblxwin.hints.framebuffer.greenBits = value;
            return;
        case RBLXWIN_BLUE_BITS:
            _rblxwin.hints.framebuffer.blueBits = value;
            return;
        case RBLXWIN_ALPHA_BITS:
            _rblxwin.hints.framebuffer.alphaBits = value;
            return;
        case RBLXWIN_DEPTH_BITS:
            _rblxwin.hints.framebuffer.depthBits = value;
            return;
        case RBLXWIN_STENCIL_BITS:
            _rblxwin.hints.framebuffer.stencilBits = value;
            return;
        case RBLXWIN_ACCUM_RED_BITS:
            _rblxwin.hints.framebuffer.accumRedBits = value;
            return;
        case RBLXWIN_ACCUM_GREEN_BITS:
            _rblxwin.hints.framebuffer.accumGreenBits = value;
            return;
        case RBLXWIN_ACCUM_BLUE_BITS:
            _rblxwin.hints.framebuffer.accumBlueBits = value;
            return;
        case RBLXWIN_ACCUM_ALPHA_BITS:
            _rblxwin.hints.framebuffer.accumAlphaBits = value;
            return;
        case RBLXWIN_AUX_BUFFERS:
            _rblxwin.hints.framebuffer.auxBuffers = value;
            return;
        case RBLXWIN_STEREO:
            _rblxwin.hints.framebuffer.stereo = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_DOUBLEBUFFER:
            _rblxwin.hints.framebuffer.doublebuffer = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_TRANSPARENT_FRAMEBUFFER:
            _rblxwin.hints.framebuffer.transparent = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_SAMPLES:
            _rblxwin.hints.framebuffer.samples = value;
            return;
        case RBLXWIN_SRGB_CAPABLE:
            _rblxwin.hints.framebuffer.sRGB = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_RESIZABLE:
            _rblxwin.hints.window.resizable = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_DECORATED:
            _rblxwin.hints.window.decorated = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_FOCUSED:
            _rblxwin.hints.window.focused = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_AUTO_ICONIFY:
            _rblxwin.hints.window.autoIconify = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_FLOATING:
            _rblxwin.hints.window.floating = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_MAXIMIZED:
            _rblxwin.hints.window.maximized = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_VISIBLE:
            _rblxwin.hints.window.visible = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_COCOA_RETINA_FRAMEBUFFER:
            _rblxwin.hints.window.ns.retina = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_COCOA_GRAPHICS_SWITCHING:
            _rblxwin.hints.context.nsgl.offline = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_SCALE_TO_MONITOR:
            _rblxwin.hints.window.scaleToMonitor = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_CENTER_CURSOR:
            _rblxwin.hints.window.centerCursor = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_FOCUS_ON_SHOW:
            _rblxwin.hints.window.focusOnShow = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_CLIENT_API:
            _rblxwin.hints.context.client = value;
            return;
        case RBLXWIN_CONTEXT_CREATION_API:
            _rblxwin.hints.context.source = value;
            return;
        case RBLXWIN_CONTEXT_VERSION_MAJOR:
            _rblxwin.hints.context.major = value;
            return;
        case RBLXWIN_CONTEXT_VERSION_MINOR:
            _rblxwin.hints.context.minor = value;
            return;
        case RBLXWIN_CONTEXT_ROBUSTNESS:
            _rblxwin.hints.context.robustness = value;
            return;
        case RBLXWIN_OPENGL_FORWARD_COMPAT:
            _rblxwin.hints.context.forward = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_OPENGL_DEBUG_CONTEXT:
            _rblxwin.hints.context.debug = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_CONTEXT_NO_ERROR:
            _rblxwin.hints.context.noerror = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
            return;
        case RBLXWIN_OPENGL_PROFILE:
            _rblxwin.hints.context.profile = value;
            return;
        case RBLXWIN_CONTEXT_RELEASE_BEHAVIOR:
            _rblxwin.hints.context.release = value;
            return;
        case RBLXWIN_REFRESH_RATE:
            _rblxwin.hints.refreshRate = value;
            return;
    }

    _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid window hint 0x%08X", hint);
}

RBLXWINAPI void rblxwinWindowHintString(int hint, const char* value)
{
    assert(value != NULL);

    _RBLXWIN_REQUIRE_INIT();

    switch (hint)
    {
        case RBLXWIN_COCOA_FRAME_NAME:
            strncpy(_rblxwin.hints.window.ns.frameName, value,
                    sizeof(_rblxwin.hints.window.ns.frameName) - 1);
            return;
        case RBLXWIN_X11_CLASS_NAME:
            strncpy(_rblxwin.hints.window.x11.className, value,
                    sizeof(_rblxwin.hints.window.x11.className) - 1);
            return;
        case RBLXWIN_X11_INSTANCE_NAME:
            strncpy(_rblxwin.hints.window.x11.instanceName, value,
                    sizeof(_rblxwin.hints.window.x11.instanceName) - 1);
            return;
    }

    _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid window hint string 0x%08X", hint);
}

RBLXWINAPI void rblxwinDestroyWindow(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;

    _RBLXWIN_REQUIRE_INIT();

    // Allow closing of NULL (to match the behavior of free)
    if (window == NULL)
        return;

    // Clear all callbacks to avoid exposing a half torn-down window object
    memset(&window->callbacks, 0, sizeof(window->callbacks));

    // The window's context must not be current on another thread when the
    // window is destroyed
    if (window == _rblxwinPlatformGetTls(&_rblxwin.contextSlot))
        rblxwinMakeContextCurrent(NULL);

    _rblxwinPlatformDestroyWindow(window);

    // Unlink window from global linked list
    {
        _RBLXWINwindow** prev = &_rblxwin.windowListHead;

        while (*prev != window)
            prev = &((*prev)->next);

        *prev = window->next;
    }

    free(window);
}

RBLXWINAPI int rblxwinWindowShouldClose(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(0);
    return window->shouldClose;
}

RBLXWINAPI void rblxwinSetWindowShouldClose(RBLXWINwindow* handle, int value)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();
    window->shouldClose = value;
}

RBLXWINAPI void rblxwinSetWindowTitle(RBLXWINwindow* handle, const char* title)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);
    assert(title != NULL);

    _RBLXWIN_REQUIRE_INIT();
    _rblxwinPlatformSetWindowTitle(window, title);
}

RBLXWINAPI void rblxwinSetWindowIcon(RBLXWINwindow* handle,
                               int count, const RBLXWINimage* images)
{
    int i;
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;

    assert(window != NULL);
    assert(count >= 0);
    assert(count == 0 || images != NULL);

    _RBLXWIN_REQUIRE_INIT();

    if (count < 0)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE, "Invalid image count for window icon");
        return;
    }

    for (i = 0; i < count; i++)
    {
        assert(images[i].pixels != NULL);

        if (images[i].width <= 0 || images[i].height <= 0)
        {
            _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                            "Invalid image dimensions for window icon");
            return;
        }
    }

    _rblxwinPlatformSetWindowIcon(window, count, images);
}

RBLXWINAPI void rblxwinGetWindowPos(RBLXWINwindow* handle, int* xpos, int* ypos)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;

    _RBLXWIN_REQUIRE_INIT();
    _rblxwinPlatformGetWindowPos(window, xpos, ypos);
}

RBLXWINAPI void rblxwinSetWindowPos(RBLXWINwindow* handle, int xpos, int ypos)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();

    if (window->monitor)
        return;

    _rblxwinPlatformSetWindowPos(window, xpos, ypos);
}

RBLXWINAPI void rblxwinGetWindowSize(RBLXWINwindow* handle, int* width, int* height)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    if (width)
        *width = 0;
    if (height)
        *height = 0;

    _RBLXWIN_REQUIRE_INIT();
    _rblxwinPlatformGetWindowSize(window, width, height);
}

RBLXWINAPI void rblxwinSetWindowSize(RBLXWINwindow* handle, int width, int height)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);
    assert(width >= 0);
    assert(height >= 0);

    _RBLXWIN_REQUIRE_INIT();

    window->videoMode.width  = width;
    window->videoMode.height = height;

    _rblxwinPlatformSetWindowSize(window, width, height);
}

RBLXWINAPI void rblxwinSetWindowSizeLimits(RBLXWINwindow* handle,
                                     int minwidth, int minheight,
                                     int maxwidth, int maxheight)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();

    if (minwidth != RBLXWIN_DONT_CARE && minheight != RBLXWIN_DONT_CARE)
    {
        if (minwidth < 0 || minheight < 0)
        {
            _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                            "Invalid window minimum size %ix%i",
                            minwidth, minheight);
            return;
        }
    }

    if (maxwidth != RBLXWIN_DONT_CARE && maxheight != RBLXWIN_DONT_CARE)
    {
        if (maxwidth < 0 || maxheight < 0 ||
            maxwidth < minwidth || maxheight < minheight)
        {
            _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                            "Invalid window maximum size %ix%i",
                            maxwidth, maxheight);
            return;
        }
    }

    window->minwidth  = minwidth;
    window->minheight = minheight;
    window->maxwidth  = maxwidth;
    window->maxheight = maxheight;

    if (window->monitor || !window->resizable)
        return;

    _rblxwinPlatformSetWindowSizeLimits(window,
                                     minwidth, minheight,
                                     maxwidth, maxheight);
}

RBLXWINAPI void rblxwinSetWindowAspectRatio(RBLXWINwindow* handle, int numer, int denom)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);
    assert(numer != 0);
    assert(denom != 0);

    _RBLXWIN_REQUIRE_INIT();

    if (numer != RBLXWIN_DONT_CARE && denom != RBLXWIN_DONT_CARE)
    {
        if (numer <= 0 || denom <= 0)
        {
            _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                            "Invalid window aspect ratio %i:%i",
                            numer, denom);
            return;
        }
    }

    window->numer = numer;
    window->denom = denom;

    if (window->monitor || !window->resizable)
        return;

    _rblxwinPlatformSetWindowAspectRatio(window, numer, denom);
}

RBLXWINAPI void rblxwinGetFramebufferSize(RBLXWINwindow* handle, int* width, int* height)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    if (width)
        *width = 0;
    if (height)
        *height = 0;

    _RBLXWIN_REQUIRE_INIT();
    _rblxwinPlatformGetFramebufferSize(window, width, height);
}

RBLXWINAPI void rblxwinGetWindowFrameSize(RBLXWINwindow* handle,
                                    int* left, int* top,
                                    int* right, int* bottom)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    if (left)
        *left = 0;
    if (top)
        *top = 0;
    if (right)
        *right = 0;
    if (bottom)
        *bottom = 0;

    _RBLXWIN_REQUIRE_INIT();
    _rblxwinPlatformGetWindowFrameSize(window, left, top, right, bottom);
}

RBLXWINAPI void rblxwinGetWindowContentScale(RBLXWINwindow* handle,
                                       float* xscale, float* yscale)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    if (xscale)
        *xscale = 0.f;
    if (yscale)
        *yscale = 0.f;

    _RBLXWIN_REQUIRE_INIT();
    _rblxwinPlatformGetWindowContentScale(window, xscale, yscale);
}

RBLXWINAPI float rblxwinGetWindowOpacity(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(1.f);
    return _rblxwinPlatformGetWindowOpacity(window);
}

RBLXWINAPI void rblxwinSetWindowOpacity(RBLXWINwindow* handle, float opacity)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);
    assert(opacity == opacity);
    assert(opacity >= 0.f);
    assert(opacity <= 1.f);

    _RBLXWIN_REQUIRE_INIT();

    if (opacity != opacity || opacity < 0.f || opacity > 1.f)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE, "Invalid window opacity %f", opacity);
        return;
    }

    _rblxwinPlatformSetWindowOpacity(window, opacity);
}

RBLXWINAPI void rblxwinIconifyWindow(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();
    _rblxwinPlatformIconifyWindow(window);
}

RBLXWINAPI void rblxwinRestoreWindow(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();
    _rblxwinPlatformRestoreWindow(window);
}

RBLXWINAPI void rblxwinMaximizeWindow(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();

    if (window->monitor)
        return;

    _rblxwinPlatformMaximizeWindow(window);
}

RBLXWINAPI void rblxwinShowWindow(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();

    if (window->monitor)
        return;

    _rblxwinPlatformShowWindow(window);

    if (window->focusOnShow)
        _rblxwinPlatformFocusWindow(window);
}

RBLXWINAPI void rblxwinRequestWindowAttention(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();

    _rblxwinPlatformRequestWindowAttention(window);
}

RBLXWINAPI void rblxwinHideWindow(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();

    if (window->monitor)
        return;

    _rblxwinPlatformHideWindow(window);
}

RBLXWINAPI void rblxwinFocusWindow(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();

    _rblxwinPlatformFocusWindow(window);
}

RBLXWINAPI int rblxwinGetWindowAttrib(RBLXWINwindow* handle, int attrib)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(0);

    switch (attrib)
    {
        case RBLXWIN_FOCUSED:
            return _rblxwinPlatformWindowFocused(window);
        case RBLXWIN_ICONIFIED:
            return _rblxwinPlatformWindowIconified(window);
        case RBLXWIN_VISIBLE:
            return _rblxwinPlatformWindowVisible(window);
        case RBLXWIN_MAXIMIZED:
            return _rblxwinPlatformWindowMaximized(window);
        case RBLXWIN_HOVERED:
            return _rblxwinPlatformWindowHovered(window);
        case RBLXWIN_FOCUS_ON_SHOW:
            return window->focusOnShow;
        case RBLXWIN_TRANSPARENT_FRAMEBUFFER:
            return _rblxwinPlatformFramebufferTransparent(window);
        case RBLXWIN_RESIZABLE:
            return window->resizable;
        case RBLXWIN_DECORATED:
            return window->decorated;
        case RBLXWIN_FLOATING:
            return window->floating;
        case RBLXWIN_AUTO_ICONIFY:
            return window->autoIconify;
        case RBLXWIN_CLIENT_API:
            return window->context.client;
        case RBLXWIN_CONTEXT_CREATION_API:
            return window->context.source;
        case RBLXWIN_CONTEXT_VERSION_MAJOR:
            return window->context.major;
        case RBLXWIN_CONTEXT_VERSION_MINOR:
            return window->context.minor;
        case RBLXWIN_CONTEXT_REVISION:
            return window->context.revision;
        case RBLXWIN_CONTEXT_ROBUSTNESS:
            return window->context.robustness;
        case RBLXWIN_OPENGL_FORWARD_COMPAT:
            return window->context.forward;
        case RBLXWIN_OPENGL_DEBUG_CONTEXT:
            return window->context.debug;
        case RBLXWIN_OPENGL_PROFILE:
            return window->context.profile;
        case RBLXWIN_CONTEXT_RELEASE_BEHAVIOR:
            return window->context.release;
        case RBLXWIN_CONTEXT_NO_ERROR:
            return window->context.noerror;
    }

    _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid window attribute 0x%08X", attrib);
    return 0;
}

RBLXWINAPI void rblxwinSetWindowAttrib(RBLXWINwindow* handle, int attrib, int value)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();

    value = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;

    if (attrib == RBLXWIN_AUTO_ICONIFY)
        window->autoIconify = value;
    else if (attrib == RBLXWIN_RESIZABLE)
    {
        if (window->resizable == value)
            return;

        window->resizable = value;
        if (!window->monitor)
            _rblxwinPlatformSetWindowResizable(window, value);
    }
    else if (attrib == RBLXWIN_DECORATED)
    {
        if (window->decorated == value)
            return;

        window->decorated = value;
        if (!window->monitor)
            _rblxwinPlatformSetWindowDecorated(window, value);
    }
    else if (attrib == RBLXWIN_FLOATING)
    {
        if (window->floating == value)
            return;

        window->floating = value;
        if (!window->monitor)
            _rblxwinPlatformSetWindowFloating(window, value);
    }
    else if (attrib == RBLXWIN_FOCUS_ON_SHOW)
        window->focusOnShow = value;
    else
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid window attribute 0x%08X", attrib);
}

RBLXWINAPI RBLXWINmonitor* rblxwinGetWindowMonitor(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return (RBLXWINmonitor*) window->monitor;
}

RBLXWINAPI void rblxwinSetWindowMonitor(RBLXWINwindow* wh,
                                  RBLXWINmonitor* mh,
                                  int xpos, int ypos,
                                  int width, int height,
                                  int refreshRate)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) wh;
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) mh;
    assert(window != NULL);
    assert(width >= 0);
    assert(height >= 0);

    _RBLXWIN_REQUIRE_INIT();

    if (width <= 0 || height <= 0)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                        "Invalid window size %ix%i",
                        width, height);
        return;
    }

    if (refreshRate < 0 && refreshRate != RBLXWIN_DONT_CARE)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                        "Invalid refresh rate %i",
                        refreshRate);
        return;
    }

    window->videoMode.width       = width;
    window->videoMode.height      = height;
    window->videoMode.refreshRate = refreshRate;

    _rblxwinPlatformSetWindowMonitor(window, monitor,
                                  xpos, ypos, width, height,
                                  refreshRate);
}

RBLXWINAPI void rblxwinSetWindowUserPointer(RBLXWINwindow* handle, void* pointer)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();
    window->userPointer = pointer;
}

RBLXWINAPI void* rblxwinGetWindowUserPointer(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return window->userPointer;
}

RBLXWINAPI RBLXWINwindowposfun rblxwinSetWindowPosCallback(RBLXWINwindow* handle,
                                                  RBLXWINwindowposfun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.pos, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINwindowsizefun rblxwinSetWindowSizeCallback(RBLXWINwindow* handle,
                                                    RBLXWINwindowsizefun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.size, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINwindowclosefun rblxwinSetWindowCloseCallback(RBLXWINwindow* handle,
                                                      RBLXWINwindowclosefun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.close, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINwindowrefreshfun rblxwinSetWindowRefreshCallback(RBLXWINwindow* handle,
                                                          RBLXWINwindowrefreshfun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.refresh, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINwindowfocusfun rblxwinSetWindowFocusCallback(RBLXWINwindow* handle,
                                                      RBLXWINwindowfocusfun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.focus, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINwindowiconifyfun rblxwinSetWindowIconifyCallback(RBLXWINwindow* handle,
                                                          RBLXWINwindowiconifyfun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.iconify, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINwindowmaximizefun rblxwinSetWindowMaximizeCallback(RBLXWINwindow* handle,
                                                            RBLXWINwindowmaximizefun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.maximize, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINframebuffersizefun rblxwinSetFramebufferSizeCallback(RBLXWINwindow* handle,
                                                              RBLXWINframebuffersizefun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.fbsize, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINwindowcontentscalefun rblxwinSetWindowContentScaleCallback(RBLXWINwindow* handle,
                                                                    RBLXWINwindowcontentscalefun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.scale, cbfun);
    return cbfun;
}

RBLXWINAPI void rblxwinPollEvents(void)
{
    _RBLXWIN_REQUIRE_INIT();
    _rblxwinPlatformPollEvents();
}

RBLXWINAPI void rblxwinWaitEvents(void)
{
    _RBLXWIN_REQUIRE_INIT();
    _rblxwinPlatformWaitEvents();
}

RBLXWINAPI void rblxwinWaitEventsTimeout(double timeout)
{
    _RBLXWIN_REQUIRE_INIT();
    assert(timeout == timeout);
    assert(timeout >= 0.0);
    assert(timeout <= DBL_MAX);

    if (timeout != timeout || timeout < 0.0 || timeout > DBL_MAX)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE, "Invalid time %f", timeout);
        return;
    }

    _rblxwinPlatformWaitEventsTimeout(timeout);
}

RBLXWINAPI void rblxwinPostEmptyEvent(void)
{
    _RBLXWIN_REQUIRE_INIT();
    _rblxwinPlatformPostEmptyEvent();
}

