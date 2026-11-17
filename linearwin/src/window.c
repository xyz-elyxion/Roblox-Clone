//========================================================================
// LINEARWIN 3.3 - www.linearwin.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@linearwin.org>
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
//////                         LINEARWIN event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code that a window has lost or received input focus
//
void _linearwinInputWindowFocus(_LINEARWINwindow* window, LINEARWINbool focused)
{
    if (window->callbacks.focus)
        window->callbacks.focus((LINEARWINwindow*) window, focused);

    if (!focused)
    {
        int key, button;

        for (key = 0;  key <= LINEARWIN_KEY_LAST;  key++)
        {
            if (window->keys[key] == LINEARWIN_PRESS)
            {
                const int scancode = _linearwinPlatformGetKeyScancode(key);
                _linearwinInputKey(window, key, scancode, LINEARWIN_RELEASE, 0);
            }
        }

        for (button = 0;  button <= LINEARWIN_MOUSE_BUTTON_LAST;  button++)
        {
            if (window->mouseButtons[button] == LINEARWIN_PRESS)
                _linearwinInputMouseClick(window, button, LINEARWIN_RELEASE, 0);
        }
    }
}

// Notifies shared code that a window has moved
// The position is specified in content area relative screen coordinates
//
void _linearwinInputWindowPos(_LINEARWINwindow* window, int x, int y)
{
    if (window->callbacks.pos)
        window->callbacks.pos((LINEARWINwindow*) window, x, y);
}

// Notifies shared code that a window has been resized
// The size is specified in screen coordinates
//
void _linearwinInputWindowSize(_LINEARWINwindow* window, int width, int height)
{
    if (window->callbacks.size)
        window->callbacks.size((LINEARWINwindow*) window, width, height);
}

// Notifies shared code that a window has been iconified or restored
//
void _linearwinInputWindowIconify(_LINEARWINwindow* window, LINEARWINbool iconified)
{
    if (window->callbacks.iconify)
        window->callbacks.iconify((LINEARWINwindow*) window, iconified);
}

// Notifies shared code that a window has been maximized or restored
//
void _linearwinInputWindowMaximize(_LINEARWINwindow* window, LINEARWINbool maximized)
{
    if (window->callbacks.maximize)
        window->callbacks.maximize((LINEARWINwindow*) window, maximized);
}

// Notifies shared code that a window framebuffer has been resized
// The size is specified in pixels
//
void _linearwinInputFramebufferSize(_LINEARWINwindow* window, int width, int height)
{
    if (window->callbacks.fbsize)
        window->callbacks.fbsize((LINEARWINwindow*) window, width, height);
}

// Notifies shared code that a window content scale has changed
// The scale is specified as the ratio between the current and default DPI
//
void _linearwinInputWindowContentScale(_LINEARWINwindow* window, float xscale, float yscale)
{
    if (window->callbacks.scale)
        window->callbacks.scale((LINEARWINwindow*) window, xscale, yscale);
}

// Notifies shared code that the window contents needs updating
//
void _linearwinInputWindowDamage(_LINEARWINwindow* window)
{
    if (window->callbacks.refresh)
        window->callbacks.refresh((LINEARWINwindow*) window);
}

// Notifies shared code that the user wishes to close a window
//
void _linearwinInputWindowCloseRequest(_LINEARWINwindow* window)
{
    window->shouldClose = LINEARWIN_TRUE;

    if (window->callbacks.close)
        window->callbacks.close((LINEARWINwindow*) window);
}

// Notifies shared code that a window has changed its desired monitor
//
void _linearwinInputWindowMonitor(_LINEARWINwindow* window, _LINEARWINmonitor* monitor)
{
    window->monitor = monitor;
}

//////////////////////////////////////////////////////////////////////////
//////                        LINEARWIN public API                       //////
//////////////////////////////////////////////////////////////////////////

LINEARWINAPI LINEARWINwindow* linearwinCreateWindow(int width, int height,
                                     const char* title,
                                     LINEARWINmonitor* monitor,
                                     LINEARWINwindow* share)
{
    _LINEARWINfbconfig fbconfig;
    _LINEARWINctxconfig ctxconfig;
    _LINEARWINwndconfig wndconfig;
    _LINEARWINwindow* window;

    assert(title != NULL);
    assert(width >= 0);
    assert(height >= 0);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (width <= 0 || height <= 0)
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE,
                        "Invalid window size %ix%i",
                        width, height);

        return NULL;
    }

    fbconfig  = _linearwin.hints.framebuffer;
    ctxconfig = _linearwin.hints.context;
    wndconfig = _linearwin.hints.window;

    wndconfig.width   = width;
    wndconfig.height  = height;
    wndconfig.title   = title;
    ctxconfig.share   = (_LINEARWINwindow*) share;

    if (!_linearwinIsValidContextConfig(&ctxconfig))
        return NULL;

    window = calloc(1, sizeof(_LINEARWINwindow));
    window->next = _linearwin.windowListHead;
    _linearwin.windowListHead = window;

    window->videoMode.width       = width;
    window->videoMode.height      = height;
    window->videoMode.redBits     = fbconfig.redBits;
    window->videoMode.greenBits   = fbconfig.greenBits;
    window->videoMode.blueBits    = fbconfig.blueBits;
    window->videoMode.refreshRate = _linearwin.hints.refreshRate;

    window->monitor     = (_LINEARWINmonitor*) monitor;
    window->resizable   = wndconfig.resizable;
    window->decorated   = wndconfig.decorated;
    window->autoIconify = wndconfig.autoIconify;
    window->floating    = wndconfig.floating;
    window->focusOnShow = wndconfig.focusOnShow;
    window->cursorMode  = LINEARWIN_CURSOR_NORMAL;

    window->doublebuffer = fbconfig.doublebuffer;

    window->minwidth    = LINEARWIN_DONT_CARE;
    window->minheight   = LINEARWIN_DONT_CARE;
    window->maxwidth    = LINEARWIN_DONT_CARE;
    window->maxheight   = LINEARWIN_DONT_CARE;
    window->numer       = LINEARWIN_DONT_CARE;
    window->denom       = LINEARWIN_DONT_CARE;

    // Open the actual window and create its context
    if (!_linearwinPlatformCreateWindow(window, &wndconfig, &ctxconfig, &fbconfig))
    {
        linearwinDestroyWindow((LINEARWINwindow*) window);
        return NULL;
    }

    return (LINEARWINwindow*) window;
}

void linearwinDefaultWindowHints(void)
{
    _LINEARWIN_REQUIRE_INIT();

    // The default is OpenGL with minimum version 1.0
    memset(&_linearwin.hints.context, 0, sizeof(_linearwin.hints.context));
    _linearwin.hints.context.client = LINEARWIN_OPENGL_API;
    _linearwin.hints.context.source = LINEARWIN_NATIVE_CONTEXT_API;
    _linearwin.hints.context.major  = 1;
    _linearwin.hints.context.minor  = 0;

    // The default is a focused, visible, resizable window with decorations
    memset(&_linearwin.hints.window, 0, sizeof(_linearwin.hints.window));
    _linearwin.hints.window.resizable    = LINEARWIN_TRUE;
    _linearwin.hints.window.visible      = LINEARWIN_TRUE;
    _linearwin.hints.window.decorated    = LINEARWIN_TRUE;
    _linearwin.hints.window.focused      = LINEARWIN_TRUE;
    _linearwin.hints.window.autoIconify  = LINEARWIN_TRUE;
    _linearwin.hints.window.centerCursor = LINEARWIN_TRUE;
    _linearwin.hints.window.focusOnShow  = LINEARWIN_TRUE;

    // The default is 24 bits of color, 24 bits of depth and 8 bits of stencil,
    // double buffered
    memset(&_linearwin.hints.framebuffer, 0, sizeof(_linearwin.hints.framebuffer));
    _linearwin.hints.framebuffer.redBits      = 8;
    _linearwin.hints.framebuffer.greenBits    = 8;
    _linearwin.hints.framebuffer.blueBits     = 8;
    _linearwin.hints.framebuffer.alphaBits    = 8;
    _linearwin.hints.framebuffer.depthBits    = 24;
    _linearwin.hints.framebuffer.stencilBits  = 8;
    _linearwin.hints.framebuffer.doublebuffer = LINEARWIN_TRUE;

    // The default is to select the highest available refresh rate
    _linearwin.hints.refreshRate = LINEARWIN_DONT_CARE;

    // The default is to use full Retina resolution framebuffers
    _linearwin.hints.window.ns.retina = LINEARWIN_TRUE;
}

LINEARWINAPI void linearwinWindowHint(int hint, int value)
{
    _LINEARWIN_REQUIRE_INIT();

    switch (hint)
    {
        case LINEARWIN_RED_BITS:
            _linearwin.hints.framebuffer.redBits = value;
            return;
        case LINEARWIN_GREEN_BITS:
            _linearwin.hints.framebuffer.greenBits = value;
            return;
        case LINEARWIN_BLUE_BITS:
            _linearwin.hints.framebuffer.blueBits = value;
            return;
        case LINEARWIN_ALPHA_BITS:
            _linearwin.hints.framebuffer.alphaBits = value;
            return;
        case LINEARWIN_DEPTH_BITS:
            _linearwin.hints.framebuffer.depthBits = value;
            return;
        case LINEARWIN_STENCIL_BITS:
            _linearwin.hints.framebuffer.stencilBits = value;
            return;
        case LINEARWIN_ACCUM_RED_BITS:
            _linearwin.hints.framebuffer.accumRedBits = value;
            return;
        case LINEARWIN_ACCUM_GREEN_BITS:
            _linearwin.hints.framebuffer.accumGreenBits = value;
            return;
        case LINEARWIN_ACCUM_BLUE_BITS:
            _linearwin.hints.framebuffer.accumBlueBits = value;
            return;
        case LINEARWIN_ACCUM_ALPHA_BITS:
            _linearwin.hints.framebuffer.accumAlphaBits = value;
            return;
        case LINEARWIN_AUX_BUFFERS:
            _linearwin.hints.framebuffer.auxBuffers = value;
            return;
        case LINEARWIN_STEREO:
            _linearwin.hints.framebuffer.stereo = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_DOUBLEBUFFER:
            _linearwin.hints.framebuffer.doublebuffer = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_TRANSPARENT_FRAMEBUFFER:
            _linearwin.hints.framebuffer.transparent = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_SAMPLES:
            _linearwin.hints.framebuffer.samples = value;
            return;
        case LINEARWIN_SRGB_CAPABLE:
            _linearwin.hints.framebuffer.sRGB = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_RESIZABLE:
            _linearwin.hints.window.resizable = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_DECORATED:
            _linearwin.hints.window.decorated = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_FOCUSED:
            _linearwin.hints.window.focused = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_AUTO_ICONIFY:
            _linearwin.hints.window.autoIconify = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_FLOATING:
            _linearwin.hints.window.floating = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_MAXIMIZED:
            _linearwin.hints.window.maximized = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_VISIBLE:
            _linearwin.hints.window.visible = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_COCOA_RETINA_FRAMEBUFFER:
            _linearwin.hints.window.ns.retina = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_COCOA_GRAPHICS_SWITCHING:
            _linearwin.hints.context.nsgl.offline = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_SCALE_TO_MONITOR:
            _linearwin.hints.window.scaleToMonitor = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_CENTER_CURSOR:
            _linearwin.hints.window.centerCursor = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_FOCUS_ON_SHOW:
            _linearwin.hints.window.focusOnShow = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_CLIENT_API:
            _linearwin.hints.context.client = value;
            return;
        case LINEARWIN_CONTEXT_CREATION_API:
            _linearwin.hints.context.source = value;
            return;
        case LINEARWIN_CONTEXT_VERSION_MAJOR:
            _linearwin.hints.context.major = value;
            return;
        case LINEARWIN_CONTEXT_VERSION_MINOR:
            _linearwin.hints.context.minor = value;
            return;
        case LINEARWIN_CONTEXT_ROBUSTNESS:
            _linearwin.hints.context.robustness = value;
            return;
        case LINEARWIN_OPENGL_FORWARD_COMPAT:
            _linearwin.hints.context.forward = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_OPENGL_DEBUG_CONTEXT:
            _linearwin.hints.context.debug = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_CONTEXT_NO_ERROR:
            _linearwin.hints.context.noerror = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
            return;
        case LINEARWIN_OPENGL_PROFILE:
            _linearwin.hints.context.profile = value;
            return;
        case LINEARWIN_CONTEXT_RELEASE_BEHAVIOR:
            _linearwin.hints.context.release = value;
            return;
        case LINEARWIN_REFRESH_RATE:
            _linearwin.hints.refreshRate = value;
            return;
    }

    _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid window hint 0x%08X", hint);
}

LINEARWINAPI void linearwinWindowHintString(int hint, const char* value)
{
    assert(value != NULL);

    _LINEARWIN_REQUIRE_INIT();

    switch (hint)
    {
        case LINEARWIN_COCOA_FRAME_NAME:
            strncpy(_linearwin.hints.window.ns.frameName, value,
                    sizeof(_linearwin.hints.window.ns.frameName) - 1);
            return;
        case LINEARWIN_X11_CLASS_NAME:
            strncpy(_linearwin.hints.window.x11.className, value,
                    sizeof(_linearwin.hints.window.x11.className) - 1);
            return;
        case LINEARWIN_X11_INSTANCE_NAME:
            strncpy(_linearwin.hints.window.x11.instanceName, value,
                    sizeof(_linearwin.hints.window.x11.instanceName) - 1);
            return;
    }

    _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid window hint string 0x%08X", hint);
}

LINEARWINAPI void linearwinDestroyWindow(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;

    _LINEARWIN_REQUIRE_INIT();

    // Allow closing of NULL (to match the behavior of free)
    if (window == NULL)
        return;

    // Clear all callbacks to avoid exposing a half torn-down window object
    memset(&window->callbacks, 0, sizeof(window->callbacks));

    // The window's context must not be current on another thread when the
    // window is destroyed
    if (window == _linearwinPlatformGetTls(&_linearwin.contextSlot))
        linearwinMakeContextCurrent(NULL);

    _linearwinPlatformDestroyWindow(window);

    // Unlink window from global linked list
    {
        _LINEARWINwindow** prev = &_linearwin.windowListHead;

        while (*prev != window)
            prev = &((*prev)->next);

        *prev = window->next;
    }

    free(window);
}

LINEARWINAPI int linearwinWindowShouldClose(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(0);
    return window->shouldClose;
}

LINEARWINAPI void linearwinSetWindowShouldClose(LINEARWINwindow* handle, int value)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();
    window->shouldClose = value;
}

LINEARWINAPI void linearwinSetWindowTitle(LINEARWINwindow* handle, const char* title)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);
    assert(title != NULL);

    _LINEARWIN_REQUIRE_INIT();
    _linearwinPlatformSetWindowTitle(window, title);
}

LINEARWINAPI void linearwinSetWindowIcon(LINEARWINwindow* handle,
                               int count, const LINEARWINimage* images)
{
    int i;
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;

    assert(window != NULL);
    assert(count >= 0);
    assert(count == 0 || images != NULL);

    _LINEARWIN_REQUIRE_INIT();

    if (count < 0)
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE, "Invalid image count for window icon");
        return;
    }

    for (i = 0; i < count; i++)
    {
        assert(images[i].pixels != NULL);

        if (images[i].width <= 0 || images[i].height <= 0)
        {
            _linearwinInputError(LINEARWIN_INVALID_VALUE,
                            "Invalid image dimensions for window icon");
            return;
        }
    }

    _linearwinPlatformSetWindowIcon(window, count, images);
}

LINEARWINAPI void linearwinGetWindowPos(LINEARWINwindow* handle, int* xpos, int* ypos)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;

    _LINEARWIN_REQUIRE_INIT();
    _linearwinPlatformGetWindowPos(window, xpos, ypos);
}

LINEARWINAPI void linearwinSetWindowPos(LINEARWINwindow* handle, int xpos, int ypos)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();

    if (window->monitor)
        return;

    _linearwinPlatformSetWindowPos(window, xpos, ypos);
}

LINEARWINAPI void linearwinGetWindowSize(LINEARWINwindow* handle, int* width, int* height)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    if (width)
        *width = 0;
    if (height)
        *height = 0;

    _LINEARWIN_REQUIRE_INIT();
    _linearwinPlatformGetWindowSize(window, width, height);
}

LINEARWINAPI void linearwinSetWindowSize(LINEARWINwindow* handle, int width, int height)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);
    assert(width >= 0);
    assert(height >= 0);

    _LINEARWIN_REQUIRE_INIT();

    window->videoMode.width  = width;
    window->videoMode.height = height;

    _linearwinPlatformSetWindowSize(window, width, height);
}

LINEARWINAPI void linearwinSetWindowSizeLimits(LINEARWINwindow* handle,
                                     int minwidth, int minheight,
                                     int maxwidth, int maxheight)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();

    if (minwidth != LINEARWIN_DONT_CARE && minheight != LINEARWIN_DONT_CARE)
    {
        if (minwidth < 0 || minheight < 0)
        {
            _linearwinInputError(LINEARWIN_INVALID_VALUE,
                            "Invalid window minimum size %ix%i",
                            minwidth, minheight);
            return;
        }
    }

    if (maxwidth != LINEARWIN_DONT_CARE && maxheight != LINEARWIN_DONT_CARE)
    {
        if (maxwidth < 0 || maxheight < 0 ||
            maxwidth < minwidth || maxheight < minheight)
        {
            _linearwinInputError(LINEARWIN_INVALID_VALUE,
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

    _linearwinPlatformSetWindowSizeLimits(window,
                                     minwidth, minheight,
                                     maxwidth, maxheight);
}

LINEARWINAPI void linearwinSetWindowAspectRatio(LINEARWINwindow* handle, int numer, int denom)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);
    assert(numer != 0);
    assert(denom != 0);

    _LINEARWIN_REQUIRE_INIT();

    if (numer != LINEARWIN_DONT_CARE && denom != LINEARWIN_DONT_CARE)
    {
        if (numer <= 0 || denom <= 0)
        {
            _linearwinInputError(LINEARWIN_INVALID_VALUE,
                            "Invalid window aspect ratio %i:%i",
                            numer, denom);
            return;
        }
    }

    window->numer = numer;
    window->denom = denom;

    if (window->monitor || !window->resizable)
        return;

    _linearwinPlatformSetWindowAspectRatio(window, numer, denom);
}

LINEARWINAPI void linearwinGetFramebufferSize(LINEARWINwindow* handle, int* width, int* height)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    if (width)
        *width = 0;
    if (height)
        *height = 0;

    _LINEARWIN_REQUIRE_INIT();
    _linearwinPlatformGetFramebufferSize(window, width, height);
}

LINEARWINAPI void linearwinGetWindowFrameSize(LINEARWINwindow* handle,
                                    int* left, int* top,
                                    int* right, int* bottom)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    if (left)
        *left = 0;
    if (top)
        *top = 0;
    if (right)
        *right = 0;
    if (bottom)
        *bottom = 0;

    _LINEARWIN_REQUIRE_INIT();
    _linearwinPlatformGetWindowFrameSize(window, left, top, right, bottom);
}

LINEARWINAPI void linearwinGetWindowContentScale(LINEARWINwindow* handle,
                                       float* xscale, float* yscale)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    if (xscale)
        *xscale = 0.f;
    if (yscale)
        *yscale = 0.f;

    _LINEARWIN_REQUIRE_INIT();
    _linearwinPlatformGetWindowContentScale(window, xscale, yscale);
}

LINEARWINAPI float linearwinGetWindowOpacity(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(1.f);
    return _linearwinPlatformGetWindowOpacity(window);
}

LINEARWINAPI void linearwinSetWindowOpacity(LINEARWINwindow* handle, float opacity)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);
    assert(opacity == opacity);
    assert(opacity >= 0.f);
    assert(opacity <= 1.f);

    _LINEARWIN_REQUIRE_INIT();

    if (opacity != opacity || opacity < 0.f || opacity > 1.f)
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE, "Invalid window opacity %f", opacity);
        return;
    }

    _linearwinPlatformSetWindowOpacity(window, opacity);
}

LINEARWINAPI void linearwinIconifyWindow(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();
    _linearwinPlatformIconifyWindow(window);
}

LINEARWINAPI void linearwinRestoreWindow(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();
    _linearwinPlatformRestoreWindow(window);
}

LINEARWINAPI void linearwinMaximizeWindow(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();

    if (window->monitor)
        return;

    _linearwinPlatformMaximizeWindow(window);
}

LINEARWINAPI void linearwinShowWindow(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();

    if (window->monitor)
        return;

    _linearwinPlatformShowWindow(window);

    if (window->focusOnShow)
        _linearwinPlatformFocusWindow(window);
}

LINEARWINAPI void linearwinRequestWindowAttention(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();

    _linearwinPlatformRequestWindowAttention(window);
}

LINEARWINAPI void linearwinHideWindow(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();

    if (window->monitor)
        return;

    _linearwinPlatformHideWindow(window);
}

LINEARWINAPI void linearwinFocusWindow(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();

    _linearwinPlatformFocusWindow(window);
}

LINEARWINAPI int linearwinGetWindowAttrib(LINEARWINwindow* handle, int attrib)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(0);

    switch (attrib)
    {
        case LINEARWIN_FOCUSED:
            return _linearwinPlatformWindowFocused(window);
        case LINEARWIN_ICONIFIED:
            return _linearwinPlatformWindowIconified(window);
        case LINEARWIN_VISIBLE:
            return _linearwinPlatformWindowVisible(window);
        case LINEARWIN_MAXIMIZED:
            return _linearwinPlatformWindowMaximized(window);
        case LINEARWIN_HOVERED:
            return _linearwinPlatformWindowHovered(window);
        case LINEARWIN_FOCUS_ON_SHOW:
            return window->focusOnShow;
        case LINEARWIN_TRANSPARENT_FRAMEBUFFER:
            return _linearwinPlatformFramebufferTransparent(window);
        case LINEARWIN_RESIZABLE:
            return window->resizable;
        case LINEARWIN_DECORATED:
            return window->decorated;
        case LINEARWIN_FLOATING:
            return window->floating;
        case LINEARWIN_AUTO_ICONIFY:
            return window->autoIconify;
        case LINEARWIN_CLIENT_API:
            return window->context.client;
        case LINEARWIN_CONTEXT_CREATION_API:
            return window->context.source;
        case LINEARWIN_CONTEXT_VERSION_MAJOR:
            return window->context.major;
        case LINEARWIN_CONTEXT_VERSION_MINOR:
            return window->context.minor;
        case LINEARWIN_CONTEXT_REVISION:
            return window->context.revision;
        case LINEARWIN_CONTEXT_ROBUSTNESS:
            return window->context.robustness;
        case LINEARWIN_OPENGL_FORWARD_COMPAT:
            return window->context.forward;
        case LINEARWIN_OPENGL_DEBUG_CONTEXT:
            return window->context.debug;
        case LINEARWIN_OPENGL_PROFILE:
            return window->context.profile;
        case LINEARWIN_CONTEXT_RELEASE_BEHAVIOR:
            return window->context.release;
        case LINEARWIN_CONTEXT_NO_ERROR:
            return window->context.noerror;
    }

    _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid window attribute 0x%08X", attrib);
    return 0;
}

LINEARWINAPI void linearwinSetWindowAttrib(LINEARWINwindow* handle, int attrib, int value)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();

    value = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;

    if (attrib == LINEARWIN_AUTO_ICONIFY)
        window->autoIconify = value;
    else if (attrib == LINEARWIN_RESIZABLE)
    {
        if (window->resizable == value)
            return;

        window->resizable = value;
        if (!window->monitor)
            _linearwinPlatformSetWindowResizable(window, value);
    }
    else if (attrib == LINEARWIN_DECORATED)
    {
        if (window->decorated == value)
            return;

        window->decorated = value;
        if (!window->monitor)
            _linearwinPlatformSetWindowDecorated(window, value);
    }
    else if (attrib == LINEARWIN_FLOATING)
    {
        if (window->floating == value)
            return;

        window->floating = value;
        if (!window->monitor)
            _linearwinPlatformSetWindowFloating(window, value);
    }
    else if (attrib == LINEARWIN_FOCUS_ON_SHOW)
        window->focusOnShow = value;
    else
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid window attribute 0x%08X", attrib);
}

LINEARWINAPI LINEARWINmonitor* linearwinGetWindowMonitor(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return (LINEARWINmonitor*) window->monitor;
}

LINEARWINAPI void linearwinSetWindowMonitor(LINEARWINwindow* wh,
                                  LINEARWINmonitor* mh,
                                  int xpos, int ypos,
                                  int width, int height,
                                  int refreshRate)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) wh;
    _LINEARWINmonitor* monitor = (_LINEARWINmonitor*) mh;
    assert(window != NULL);
    assert(width >= 0);
    assert(height >= 0);

    _LINEARWIN_REQUIRE_INIT();

    if (width <= 0 || height <= 0)
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE,
                        "Invalid window size %ix%i",
                        width, height);
        return;
    }

    if (refreshRate < 0 && refreshRate != LINEARWIN_DONT_CARE)
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE,
                        "Invalid refresh rate %i",
                        refreshRate);
        return;
    }

    window->videoMode.width       = width;
    window->videoMode.height      = height;
    window->videoMode.refreshRate = refreshRate;

    _linearwinPlatformSetWindowMonitor(window, monitor,
                                  xpos, ypos, width, height,
                                  refreshRate);
}

LINEARWINAPI void linearwinSetWindowUserPointer(LINEARWINwindow* handle, void* pointer)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();
    window->userPointer = pointer;
}

LINEARWINAPI void* linearwinGetWindowUserPointer(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return window->userPointer;
}

LINEARWINAPI LINEARWINwindowposfun linearwinSetWindowPosCallback(LINEARWINwindow* handle,
                                                  LINEARWINwindowposfun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.pos, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINwindowsizefun linearwinSetWindowSizeCallback(LINEARWINwindow* handle,
                                                    LINEARWINwindowsizefun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.size, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINwindowclosefun linearwinSetWindowCloseCallback(LINEARWINwindow* handle,
                                                      LINEARWINwindowclosefun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.close, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINwindowrefreshfun linearwinSetWindowRefreshCallback(LINEARWINwindow* handle,
                                                          LINEARWINwindowrefreshfun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.refresh, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINwindowfocusfun linearwinSetWindowFocusCallback(LINEARWINwindow* handle,
                                                      LINEARWINwindowfocusfun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.focus, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINwindowiconifyfun linearwinSetWindowIconifyCallback(LINEARWINwindow* handle,
                                                          LINEARWINwindowiconifyfun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.iconify, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINwindowmaximizefun linearwinSetWindowMaximizeCallback(LINEARWINwindow* handle,
                                                            LINEARWINwindowmaximizefun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.maximize, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINframebuffersizefun linearwinSetFramebufferSizeCallback(LINEARWINwindow* handle,
                                                              LINEARWINframebuffersizefun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.fbsize, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINwindowcontentscalefun linearwinSetWindowContentScaleCallback(LINEARWINwindow* handle,
                                                                    LINEARWINwindowcontentscalefun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.scale, cbfun);
    return cbfun;
}

LINEARWINAPI void linearwinPollEvents(void)
{
    _LINEARWIN_REQUIRE_INIT();
    _linearwinPlatformPollEvents();
}

LINEARWINAPI void linearwinWaitEvents(void)
{
    _LINEARWIN_REQUIRE_INIT();
    _linearwinPlatformWaitEvents();
}

LINEARWINAPI void linearwinWaitEventsTimeout(double timeout)
{
    _LINEARWIN_REQUIRE_INIT();
    assert(timeout == timeout);
    assert(timeout >= 0.0);
    assert(timeout <= DBL_MAX);

    if (timeout != timeout || timeout < 0.0 || timeout > DBL_MAX)
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE, "Invalid time %f", timeout);
        return;
    }

    _linearwinPlatformWaitEventsTimeout(timeout);
}

LINEARWINAPI void linearwinPostEmptyEvent(void)
{
    _LINEARWIN_REQUIRE_INIT();
    _linearwinPlatformPostEmptyEvent();
}

