//========================================================================
// RBLXWIN 3.3 X11 - www.rblxwin.org
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#define _GNU_SOURCE

#include "internal.h"

#include <X11/cursorfont.h>
#include <X11/Xmd.h>

#include <poll.h>
#include <signal.h>
#include <time.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>

// Action for EWMH client messages
#define _NET_WM_STATE_REMOVE        0
#define _NET_WM_STATE_ADD           1
#define _NET_WM_STATE_TOGGLE        2

// Additional mouse button names for XButtonEvent
#define Button6            6
#define Button7            7

// Motif WM hints flags
#define MWM_HINTS_DECORATIONS   2
#define MWM_DECOR_ALL           1

#define _RBLXWIN_XDND_VERSION 5

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

// Wait for event data to arrive on the X11 display socket
// This avoids blocking other threads via the per-display Xlib lock that also
// covers GLX functions
//
static RBLXWINbool waitForX11Event(double* timeout)
{
    struct pollfd fd = { ConnectionNumber(_rblxwin.x11.display), POLLIN };

    while (!XPending(_rblxwin.x11.display))
    {
        if (!waitForData(&fd, 1, timeout))
            return RBLXWIN_FALSE;
    }

    return RBLXWIN_TRUE;
}

// Wait for event data to arrive on any event file descriptor
// This avoids blocking other threads via the per-display Xlib lock that also
// covers GLX functions
//
static RBLXWINbool waitForAnyEvent(double* timeout)
{
    nfds_t count = 2;
    struct pollfd fds[3] =
    {
        { ConnectionNumber(_rblxwin.x11.display), POLLIN },
        { _rblxwin.x11.emptyEventPipe[0], POLLIN }
    };

#if defined(__linux__)
    if (_rblxwin.linjs.inotify > 0)
        fds[count++] = (struct pollfd) { _rblxwin.linjs.inotify, POLLIN };
#endif

    while (!XPending(_rblxwin.x11.display))
    {
        if (!waitForData(fds, count, timeout))
            return RBLXWIN_FALSE;

        for (int i = 1; i < count; i++)
        {
            if (fds[i].revents & POLLIN)
                return RBLXWIN_TRUE;
        }
    }

    return RBLXWIN_TRUE;
}

// Writes a byte to the empty event pipe
//
static void writeEmptyEvent(void)
{
    for (;;)
    {
        const char byte = 0;
        const ssize_t result = write(_rblxwin.x11.emptyEventPipe[1], &byte, 1);
        if (result == 1 || (result == -1 && errno != EINTR))
            break;
    }
}

// Drains available data from the empty event pipe
//
static void drainEmptyEvents(void)
{
    for (;;)
    {
        char dummy[64];
        const ssize_t result = read(_rblxwin.x11.emptyEventPipe[0], dummy, sizeof(dummy));
        if (result == -1 && errno != EINTR)
            break;
    }
}

// Waits until a VisibilityNotify event arrives for the specified window or the
// timeout period elapses (ICCCM section 4.2.2)
//
static RBLXWINbool waitForVisibilityNotify(_RBLXWINwindow* window)
{
    XEvent dummy;
    double timeout = 0.1;

    while (!XCheckTypedWindowEvent(_rblxwin.x11.display,
                                   window->x11.handle,
                                   VisibilityNotify,
                                   &dummy))
    {
        if (!waitForX11Event(&timeout))
            return RBLXWIN_FALSE;
    }

    return RBLXWIN_TRUE;
}

// Returns whether the window is iconified
//
static int getWindowState(_RBLXWINwindow* window)
{
    int result = WithdrawnState;
    struct {
        CARD32 state;
        Window icon;
    } *state = NULL;

    if (_rblxwinGetWindowPropertyX11(window->x11.handle,
                                  _rblxwin.x11.WM_STATE,
                                  _rblxwin.x11.WM_STATE,
                                  (unsigned char**) &state) >= 2)
    {
        result = state->state;
    }

    if (state)
        XFree(state);

    return result;
}

// Returns whether the event is a selection event
//
static Bool isSelectionEvent(Display* display, XEvent* event, XPointer pointer)
{
    if (event->xany.window != _rblxwin.x11.helperWindowHandle)
        return False;

    return event->type == SelectionRequest ||
           event->type == SelectionNotify ||
           event->type == SelectionClear;
}

// Returns whether it is a _NET_FRAME_EXTENTS event for the specified window
//
static Bool isFrameExtentsEvent(Display* display, XEvent* event, XPointer pointer)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) pointer;
    return event->type == PropertyNotify &&
           event->xproperty.state == PropertyNewValue &&
           event->xproperty.window == window->x11.handle &&
           event->xproperty.atom == _rblxwin.x11.NET_FRAME_EXTENTS;
}

// Returns whether it is a property event for the specified selection transfer
//
static Bool isSelPropNewValueNotify(Display* display, XEvent* event, XPointer pointer)
{
    XEvent* notification = (XEvent*) pointer;
    return event->type == PropertyNotify &&
           event->xproperty.state == PropertyNewValue &&
           event->xproperty.window == notification->xselection.requestor &&
           event->xproperty.atom == notification->xselection.property;
}

// Translates an X event modifier state mask
//
static int translateState(int state)
{
    int mods = 0;

    if (state & ShiftMask)
        mods |= RBLXWIN_MOD_SHIFT;
    if (state & ControlMask)
        mods |= RBLXWIN_MOD_CONTROL;
    if (state & Mod1Mask)
        mods |= RBLXWIN_MOD_ALT;
    if (state & Mod4Mask)
        mods |= RBLXWIN_MOD_SUPER;
    if (state & LockMask)
        mods |= RBLXWIN_MOD_CAPS_LOCK;
    if (state & Mod2Mask)
        mods |= RBLXWIN_MOD_NUM_LOCK;

    return mods;
}

// Translates an X11 key code to a RBLXWIN key token
//
static int translateKey(int scancode)
{
    // Use the pre-filled LUT (see createKeyTables() in x11_init.c)
    if (scancode < 0 || scancode > 255)
        return RBLXWIN_KEY_UNKNOWN;

    return _rblxwin.x11.keycodes[scancode];
}

// Sends an EWMH or ICCCM event to the window manager
//
static void sendEventToWM(_RBLXWINwindow* window, Atom type,
                          long a, long b, long c, long d, long e)
{
    XEvent event = { ClientMessage };
    event.xclient.window = window->x11.handle;
    event.xclient.format = 32; // Data is 32-bit longs
    event.xclient.message_type = type;
    event.xclient.data.l[0] = a;
    event.xclient.data.l[1] = b;
    event.xclient.data.l[2] = c;
    event.xclient.data.l[3] = d;
    event.xclient.data.l[4] = e;

    XSendEvent(_rblxwin.x11.display, _rblxwin.x11.root,
               False,
               SubstructureNotifyMask | SubstructureRedirectMask,
               &event);
}

// Updates the normal hints according to the window settings
//
static void updateNormalHints(_RBLXWINwindow* window, int width, int height)
{
    XSizeHints* hints = XAllocSizeHints();

    if (!window->monitor)
    {
        if (window->resizable)
        {
            if (window->minwidth != RBLXWIN_DONT_CARE &&
                window->minheight != RBLXWIN_DONT_CARE)
            {
                hints->flags |= PMinSize;
                hints->min_width = window->minwidth;
                hints->min_height = window->minheight;
            }

            if (window->maxwidth != RBLXWIN_DONT_CARE &&
                window->maxheight != RBLXWIN_DONT_CARE)
            {
                hints->flags |= PMaxSize;
                hints->max_width = window->maxwidth;
                hints->max_height = window->maxheight;
            }

            if (window->numer != RBLXWIN_DONT_CARE &&
                window->denom != RBLXWIN_DONT_CARE)
            {
                hints->flags |= PAspect;
                hints->min_aspect.x = hints->max_aspect.x = window->numer;
                hints->min_aspect.y = hints->max_aspect.y = window->denom;
            }
        }
        else
        {
            hints->flags |= (PMinSize | PMaxSize);
            hints->min_width  = hints->max_width  = width;
            hints->min_height = hints->max_height = height;
        }
    }

    hints->flags |= PWinGravity;
    hints->win_gravity = StaticGravity;

    XSetWMNormalHints(_rblxwin.x11.display, window->x11.handle, hints);
    XFree(hints);
}

// Updates the full screen status of the window
//
static void updateWindowMode(_RBLXWINwindow* window)
{
    if (window->monitor)
    {
        if (_rblxwin.x11.xinerama.available &&
            _rblxwin.x11.NET_WM_FULLSCREEN_MONITORS)
        {
            sendEventToWM(window,
                          _rblxwin.x11.NET_WM_FULLSCREEN_MONITORS,
                          window->monitor->x11.index,
                          window->monitor->x11.index,
                          window->monitor->x11.index,
                          window->monitor->x11.index,
                          0);
        }

        if (_rblxwin.x11.NET_WM_STATE && _rblxwin.x11.NET_WM_STATE_FULLSCREEN)
        {
            sendEventToWM(window,
                          _rblxwin.x11.NET_WM_STATE,
                          _NET_WM_STATE_ADD,
                          _rblxwin.x11.NET_WM_STATE_FULLSCREEN,
                          0, 1, 0);
        }
        else
        {
            // This is the butcher's way of removing window decorations
            // Setting the override-redirect attribute on a window makes the
            // window manager ignore the window completely (ICCCM, section 4)
            // The good thing is that this makes undecorated full screen windows
            // easy to do; the bad thing is that we have to do everything
            // manually and some things (like iconify/restore) won't work at
            // all, as those are tasks usually performed by the window manager

            XSetWindowAttributes attributes;
            attributes.override_redirect = True;
            XChangeWindowAttributes(_rblxwin.x11.display,
                                    window->x11.handle,
                                    CWOverrideRedirect,
                                    &attributes);

            window->x11.overrideRedirect = RBLXWIN_TRUE;
        }

        // Enable compositor bypass
        if (!window->x11.transparent)
        {
            const unsigned long value = 1;

            XChangeProperty(_rblxwin.x11.display,  window->x11.handle,
                            _rblxwin.x11.NET_WM_BYPASS_COMPOSITOR, XA_CARDINAL, 32,
                            PropModeReplace, (unsigned char*) &value, 1);
        }
    }
    else
    {
        if (_rblxwin.x11.xinerama.available &&
            _rblxwin.x11.NET_WM_FULLSCREEN_MONITORS)
        {
            XDeleteProperty(_rblxwin.x11.display, window->x11.handle,
                            _rblxwin.x11.NET_WM_FULLSCREEN_MONITORS);
        }

        if (_rblxwin.x11.NET_WM_STATE && _rblxwin.x11.NET_WM_STATE_FULLSCREEN)
        {
            sendEventToWM(window,
                          _rblxwin.x11.NET_WM_STATE,
                          _NET_WM_STATE_REMOVE,
                          _rblxwin.x11.NET_WM_STATE_FULLSCREEN,
                          0, 1, 0);
        }
        else
        {
            XSetWindowAttributes attributes;
            attributes.override_redirect = False;
            XChangeWindowAttributes(_rblxwin.x11.display,
                                    window->x11.handle,
                                    CWOverrideRedirect,
                                    &attributes);

            window->x11.overrideRedirect = RBLXWIN_FALSE;
        }

        // Disable compositor bypass
        if (!window->x11.transparent)
        {
            XDeleteProperty(_rblxwin.x11.display, window->x11.handle,
                            _rblxwin.x11.NET_WM_BYPASS_COMPOSITOR);
        }
    }
}

// Decode a Unicode code point from a UTF-8 stream
// Based on cutef8 by Jeff Bezanson (Public Domain)
//
#if defined(X_HAVE_UTF8_STRING)
static uint32_t decodeUTF8(const char** s)
{
    uint32_t codepoint = 0, count = 0;
    static const uint32_t offsets[] =
    {
        0x00000000u, 0x00003080u, 0x000e2080u,
        0x03c82080u, 0xfa082080u, 0x82082080u
    };

    do
    {
        codepoint = (codepoint << 6) + (unsigned char) **s;
        (*s)++;
        count++;
    } while ((**s & 0xc0) == 0x80);

    assert(count <= 6);
    return codepoint - offsets[count - 1];
}
#endif /*X_HAVE_UTF8_STRING*/

// Convert the specified Latin-1 string to UTF-8
//
static char* convertLatin1toUTF8(const char* source)
{
    size_t size = 1;
    const char* sp;

    for (sp = source;  *sp;  sp++)
        size += (*sp & 0x80) ? 2 : 1;

    char* target = calloc(size, 1);
    char* tp = target;

    for (sp = source;  *sp;  sp++)
        tp += _rblxwinEncodeUTF8(tp, *sp);

    return target;
}

// Updates the cursor image according to its cursor mode
//
static void updateCursorImage(_RBLXWINwindow* window)
{
    if (window->cursorMode == RBLXWIN_CURSOR_NORMAL)
    {
        if (window->cursor)
        {
            XDefineCursor(_rblxwin.x11.display, window->x11.handle,
                          window->cursor->x11.handle);
        }
        else
            XUndefineCursor(_rblxwin.x11.display, window->x11.handle);
    }
    else
    {
        XDefineCursor(_rblxwin.x11.display, window->x11.handle,
                      _rblxwin.x11.hiddenCursorHandle);
    }
}

// Enable XI2 raw mouse motion events
//
static void enableRawMouseMotion(_RBLXWINwindow* window)
{
    XIEventMask em;
    unsigned char mask[XIMaskLen(XI_RawMotion)] = { 0 };

    em.deviceid = XIAllMasterDevices;
    em.mask_len = sizeof(mask);
    em.mask = mask;
    XISetMask(mask, XI_RawMotion);

    XISelectEvents(_rblxwin.x11.display, _rblxwin.x11.root, &em, 1);
}

// Disable XI2 raw mouse motion events
//
static void disableRawMouseMotion(_RBLXWINwindow* window)
{
    XIEventMask em;
    unsigned char mask[] = { 0 };

    em.deviceid = XIAllMasterDevices;
    em.mask_len = sizeof(mask);
    em.mask = mask;

    XISelectEvents(_rblxwin.x11.display, _rblxwin.x11.root, &em, 1);
}

// Apply disabled cursor mode to a focused window
//
static void disableCursor(_RBLXWINwindow* window)
{
    if (window->rawMouseMotion)
        enableRawMouseMotion(window);

    _rblxwin.x11.disabledCursorWindow = window;
    _rblxwinPlatformGetCursorPos(window,
                              &_rblxwin.x11.restoreCursorPosX,
                              &_rblxwin.x11.restoreCursorPosY);
    updateCursorImage(window);
    _rblxwinCenterCursorInContentArea(window);
    XGrabPointer(_rblxwin.x11.display, window->x11.handle, True,
                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync,
                 window->x11.handle,
                 _rblxwin.x11.hiddenCursorHandle,
                 CurrentTime);
}

// Exit disabled cursor mode for the specified window
//
static void enableCursor(_RBLXWINwindow* window)
{
    if (window->rawMouseMotion)
        disableRawMouseMotion(window);

    _rblxwin.x11.disabledCursorWindow = NULL;
    XUngrabPointer(_rblxwin.x11.display, CurrentTime);
    _rblxwinPlatformSetCursorPos(window,
                              _rblxwin.x11.restoreCursorPosX,
                              _rblxwin.x11.restoreCursorPosY);
    updateCursorImage(window);
}

// Create the X11 window (and its colormap)
//
static RBLXWINbool createNativeWindow(_RBLXWINwindow* window,
                                   const _RBLXWINwndconfig* wndconfig,
                                   Visual* visual, int depth)
{
    int width = wndconfig->width;
    int height = wndconfig->height;

    if (wndconfig->scaleToMonitor)
    {
        width *= _rblxwin.x11.contentScaleX;
        height *= _rblxwin.x11.contentScaleY;
    }

    // Create a colormap based on the visual used by the current context
    window->x11.colormap = XCreateColormap(_rblxwin.x11.display,
                                           _rblxwin.x11.root,
                                           visual,
                                           AllocNone);

    window->x11.transparent = _rblxwinIsVisualTransparentX11(visual);

    XSetWindowAttributes wa = { 0 };
    wa.colormap = window->x11.colormap;
    wa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
                    PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                    ExposureMask | FocusChangeMask | VisibilityChangeMask |
                    EnterWindowMask | LeaveWindowMask | PropertyChangeMask;

    _rblxwinGrabErrorHandlerX11();

    window->x11.parent = _rblxwin.x11.root;
    window->x11.handle = XCreateWindow(_rblxwin.x11.display,
                                       _rblxwin.x11.root,
                                       0, 0,   // Position
                                       width, height,
                                       0,      // Border width
                                       depth,  // Color depth
                                       InputOutput,
                                       visual,
                                       CWBorderPixel | CWColormap | CWEventMask,
                                       &wa);

    _rblxwinReleaseErrorHandlerX11();

    if (!window->x11.handle)
    {
        _rblxwinInputErrorX11(RBLXWIN_PLATFORM_ERROR,
                           "X11: Failed to create window");
        return RBLXWIN_FALSE;
    }

    XSaveContext(_rblxwin.x11.display,
                 window->x11.handle,
                 _rblxwin.x11.context,
                 (XPointer) window);

    if (!wndconfig->decorated)
        _rblxwinPlatformSetWindowDecorated(window, RBLXWIN_FALSE);

    if (_rblxwin.x11.NET_WM_STATE && !window->monitor)
    {
        Atom states[3];
        int count = 0;

        if (wndconfig->floating)
        {
            if (_rblxwin.x11.NET_WM_STATE_ABOVE)
                states[count++] = _rblxwin.x11.NET_WM_STATE_ABOVE;
        }

        if (wndconfig->maximized)
        {
            if (_rblxwin.x11.NET_WM_STATE_MAXIMIZED_VERT &&
                _rblxwin.x11.NET_WM_STATE_MAXIMIZED_HORZ)
            {
                states[count++] = _rblxwin.x11.NET_WM_STATE_MAXIMIZED_VERT;
                states[count++] = _rblxwin.x11.NET_WM_STATE_MAXIMIZED_HORZ;
                window->x11.maximized = RBLXWIN_TRUE;
            }
        }

        if (count)
        {
            XChangeProperty(_rblxwin.x11.display, window->x11.handle,
                            _rblxwin.x11.NET_WM_STATE, XA_ATOM, 32,
                            PropModeReplace, (unsigned char*) states, count);
        }
    }

    // Declare the WM protocols supported by RBLXWIN
    {
        Atom protocols[] =
        {
            _rblxwin.x11.WM_DELETE_WINDOW,
            _rblxwin.x11.NET_WM_PING
        };

        XSetWMProtocols(_rblxwin.x11.display, window->x11.handle,
                        protocols, sizeof(protocols) / sizeof(Atom));
    }

    // Declare our PID
    {
        const long pid = getpid();

        XChangeProperty(_rblxwin.x11.display,  window->x11.handle,
                        _rblxwin.x11.NET_WM_PID, XA_CARDINAL, 32,
                        PropModeReplace,
                        (unsigned char*) &pid, 1);
    }

    if (_rblxwin.x11.NET_WM_WINDOW_TYPE && _rblxwin.x11.NET_WM_WINDOW_TYPE_NORMAL)
    {
        Atom type = _rblxwin.x11.NET_WM_WINDOW_TYPE_NORMAL;
        XChangeProperty(_rblxwin.x11.display,  window->x11.handle,
                        _rblxwin.x11.NET_WM_WINDOW_TYPE, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*) &type, 1);
    }

    // Set ICCCM WM_HINTS property
    {
        XWMHints* hints = XAllocWMHints();
        if (!hints)
        {
            _rblxwinInputError(RBLXWIN_OUT_OF_MEMORY,
                            "X11: Failed to allocate WM hints");
            return RBLXWIN_FALSE;
        }

        hints->flags = StateHint;
        hints->initial_state = NormalState;

        XSetWMHints(_rblxwin.x11.display, window->x11.handle, hints);
        XFree(hints);
    }

    updateNormalHints(window, width, height);

    // Set ICCCM WM_CLASS property
    {
        XClassHint* hint = XAllocClassHint();

        if (strlen(wndconfig->x11.instanceName) &&
            strlen(wndconfig->x11.className))
        {
            hint->res_name = (char*) wndconfig->x11.instanceName;
            hint->res_class = (char*) wndconfig->x11.className;
        }
        else
        {
            const char* resourceName = getenv("RESOURCE_NAME");
            if (resourceName && strlen(resourceName))
                hint->res_name = (char*) resourceName;
            else if (strlen(wndconfig->title))
                hint->res_name = (char*) wndconfig->title;
            else
                hint->res_name = (char*) "rblxwin-application";

            if (strlen(wndconfig->title))
                hint->res_class = (char*) wndconfig->title;
            else
                hint->res_class = (char*) "RBLXWIN-Application";
        }

        XSetClassHint(_rblxwin.x11.display, window->x11.handle, hint);
        XFree(hint);
    }

    // Announce support for Xdnd (drag and drop)
    {
        const Atom version = _RBLXWIN_XDND_VERSION;
        XChangeProperty(_rblxwin.x11.display, window->x11.handle,
                        _rblxwin.x11.XdndAware, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*) &version, 1);
    }

    _rblxwinPlatformSetWindowTitle(window, wndconfig->title);

    if (_rblxwin.x11.im)
    {
        window->x11.ic = XCreateIC(_rblxwin.x11.im,
                                   XNInputStyle,
                                   XIMPreeditNothing | XIMStatusNothing,
                                   XNClientWindow,
                                   window->x11.handle,
                                   XNFocusWindow,
                                   window->x11.handle,
                                   NULL);
    }

    if (window->x11.ic)
    {
        unsigned long filter = 0;
        if (XGetICValues(window->x11.ic, XNFilterEvents, &filter, NULL) == NULL)
            XSelectInput(_rblxwin.x11.display, window->x11.handle, wa.event_mask | filter);
    }

    _rblxwinPlatformGetWindowPos(window, &window->x11.xpos, &window->x11.ypos);
    _rblxwinPlatformGetWindowSize(window, &window->x11.width, &window->x11.height);

    return RBLXWIN_TRUE;
}

// Set the specified property to the selection converted to the requested target
//
static Atom writeTargetToProperty(const XSelectionRequestEvent* request)
{
    int i;
    char* selectionString = NULL;
    const Atom formats[] = { _rblxwin.x11.UTF8_STRING, XA_STRING };
    const int formatCount = sizeof(formats) / sizeof(formats[0]);

    if (request->selection == _rblxwin.x11.PRIMARY)
        selectionString = _rblxwin.x11.primarySelectionString;
    else
        selectionString = _rblxwin.x11.clipboardString;

    if (request->property == None)
    {
        // The requester is a legacy client (ICCCM section 2.2)
        // We don't support legacy clients, so fail here
        return None;
    }

    if (request->target == _rblxwin.x11.TARGETS)
    {
        // The list of supported targets was requested

        const Atom targets[] = { _rblxwin.x11.TARGETS,
                                 _rblxwin.x11.MULTIPLE,
                                 _rblxwin.x11.UTF8_STRING,
                                 XA_STRING };

        XChangeProperty(_rblxwin.x11.display,
                        request->requestor,
                        request->property,
                        XA_ATOM,
                        32,
                        PropModeReplace,
                        (unsigned char*) targets,
                        sizeof(targets) / sizeof(targets[0]));

        return request->property;
    }

    if (request->target == _rblxwin.x11.MULTIPLE)
    {
        // Multiple conversions were requested

        Atom* targets;
        unsigned long i, count;

        count = _rblxwinGetWindowPropertyX11(request->requestor,
                                          request->property,
                                          _rblxwin.x11.ATOM_PAIR,
                                          (unsigned char**) &targets);

        for (i = 0;  i < count;  i += 2)
        {
            int j;

            for (j = 0;  j < formatCount;  j++)
            {
                if (targets[i] == formats[j])
                    break;
            }

            if (j < formatCount)
            {
                XChangeProperty(_rblxwin.x11.display,
                                request->requestor,
                                targets[i + 1],
                                targets[i],
                                8,
                                PropModeReplace,
                                (unsigned char *) selectionString,
                                strlen(selectionString));
            }
            else
                targets[i + 1] = None;
        }

        XChangeProperty(_rblxwin.x11.display,
                        request->requestor,
                        request->property,
                        _rblxwin.x11.ATOM_PAIR,
                        32,
                        PropModeReplace,
                        (unsigned char*) targets,
                        count);

        XFree(targets);

        return request->property;
    }

    if (request->target == _rblxwin.x11.SAVE_TARGETS)
    {
        // The request is a check whether we support SAVE_TARGETS
        // It should be handled as a no-op side effect target

        XChangeProperty(_rblxwin.x11.display,
                        request->requestor,
                        request->property,
                        _rblxwin.x11.NULL_,
                        32,
                        PropModeReplace,
                        NULL,
                        0);

        return request->property;
    }

    // Conversion to a data target was requested

    for (i = 0;  i < formatCount;  i++)
    {
        if (request->target == formats[i])
        {
            // The requested target is one we support

            XChangeProperty(_rblxwin.x11.display,
                            request->requestor,
                            request->property,
                            request->target,
                            8,
                            PropModeReplace,
                            (unsigned char *) selectionString,
                            strlen(selectionString));

            return request->property;
        }
    }

    // The requested target is not supported

    return None;
}

static void handleSelectionRequest(XEvent* event)
{
    const XSelectionRequestEvent* request = &event->xselectionrequest;

    XEvent reply = { SelectionNotify };
    reply.xselection.property = writeTargetToProperty(request);
    reply.xselection.display = request->display;
    reply.xselection.requestor = request->requestor;
    reply.xselection.selection = request->selection;
    reply.xselection.target = request->target;
    reply.xselection.time = request->time;

    XSendEvent(_rblxwin.x11.display, request->requestor, False, 0, &reply);
}

static const char* getSelectionString(Atom selection)
{
    char** selectionString = NULL;
    const Atom targets[] = { _rblxwin.x11.UTF8_STRING, XA_STRING };
    const size_t targetCount = sizeof(targets) / sizeof(targets[0]);

    if (selection == _rblxwin.x11.PRIMARY)
        selectionString = &_rblxwin.x11.primarySelectionString;
    else
        selectionString = &_rblxwin.x11.clipboardString;

    if (XGetSelectionOwner(_rblxwin.x11.display, selection) ==
        _rblxwin.x11.helperWindowHandle)
    {
        // Instead of doing a large number of X round-trips just to put this
        // string into a window property and then read it back, just return it
        return *selectionString;
    }

    free(*selectionString);
    *selectionString = NULL;

    for (size_t i = 0;  i < targetCount;  i++)
    {
        char* data;
        Atom actualType;
        int actualFormat;
        unsigned long itemCount, bytesAfter;
        XEvent notification, dummy;

        XConvertSelection(_rblxwin.x11.display,
                          selection,
                          targets[i],
                          _rblxwin.x11.RBLXWIN_SELECTION,
                          _rblxwin.x11.helperWindowHandle,
                          CurrentTime);

        while (!XCheckTypedWindowEvent(_rblxwin.x11.display,
                                       _rblxwin.x11.helperWindowHandle,
                                       SelectionNotify,
                                       &notification))
        {
            waitForX11Event(NULL);
        }

        if (notification.xselection.property == None)
            continue;

        XCheckIfEvent(_rblxwin.x11.display,
                      &dummy,
                      isSelPropNewValueNotify,
                      (XPointer) &notification);

        XGetWindowProperty(_rblxwin.x11.display,
                           notification.xselection.requestor,
                           notification.xselection.property,
                           0,
                           LONG_MAX,
                           True,
                           AnyPropertyType,
                           &actualType,
                           &actualFormat,
                           &itemCount,
                           &bytesAfter,
                           (unsigned char**) &data);

        if (actualType == _rblxwin.x11.INCR)
        {
            size_t size = 1;
            char* string = NULL;

            for (;;)
            {
                while (!XCheckIfEvent(_rblxwin.x11.display,
                                      &dummy,
                                      isSelPropNewValueNotify,
                                      (XPointer) &notification))
                {
                    waitForX11Event(NULL);
                }

                XFree(data);
                XGetWindowProperty(_rblxwin.x11.display,
                                   notification.xselection.requestor,
                                   notification.xselection.property,
                                   0,
                                   LONG_MAX,
                                   True,
                                   AnyPropertyType,
                                   &actualType,
                                   &actualFormat,
                                   &itemCount,
                                   &bytesAfter,
                                   (unsigned char**) &data);

                if (itemCount)
                {
                    size += itemCount;
                    string = realloc(string, size);
                    string[size - itemCount - 1] = '\0';
                    strcat(string, data);
                }

                if (!itemCount)
                {
                    if (string)
                    {
                        if (targets[i] == XA_STRING)
                        {
                            *selectionString = convertLatin1toUTF8(string);
                            free(string);
                        }
                        else
                            *selectionString = string;
                    }

                    break;
                }
            }
        }
        else if (actualType == targets[i])
        {
            if (targets[i] == XA_STRING)
                *selectionString = convertLatin1toUTF8(data);
            else
                *selectionString = _rblxwin_strdup(data);
        }

        XFree(data);

        if (*selectionString)
            break;
    }

    if (!*selectionString)
    {
        _rblxwinInputError(RBLXWIN_FORMAT_UNAVAILABLE,
                        "X11: Failed to convert selection to string");
    }

    return *selectionString;
}

// Make the specified window and its video mode active on its monitor
//
static void acquireMonitor(_RBLXWINwindow* window)
{
    if (_rblxwin.x11.saver.count == 0)
    {
        // Remember old screen saver settings
        XGetScreenSaver(_rblxwin.x11.display,
                        &_rblxwin.x11.saver.timeout,
                        &_rblxwin.x11.saver.interval,
                        &_rblxwin.x11.saver.blanking,
                        &_rblxwin.x11.saver.exposure);

        // Disable screen saver
        XSetScreenSaver(_rblxwin.x11.display, 0, 0, DontPreferBlanking,
                        DefaultExposures);
    }

    if (!window->monitor->window)
        _rblxwin.x11.saver.count++;

    _rblxwinSetVideoModeX11(window->monitor, &window->videoMode);

    if (window->x11.overrideRedirect)
    {
        int xpos, ypos;
        RBLXWINvidmode mode;

        // Manually position the window over its monitor
        _rblxwinPlatformGetMonitorPos(window->monitor, &xpos, &ypos);
        _rblxwinPlatformGetVideoMode(window->monitor, &mode);

        XMoveResizeWindow(_rblxwin.x11.display, window->x11.handle,
                          xpos, ypos, mode.width, mode.height);
    }

    _rblxwinInputMonitorWindow(window->monitor, window);
}

// Remove the window and restore the original video mode
//
static void releaseMonitor(_RBLXWINwindow* window)
{
    if (window->monitor->window != window)
        return;

    _rblxwinInputMonitorWindow(window->monitor, NULL);
    _rblxwinRestoreVideoModeX11(window->monitor);

    _rblxwin.x11.saver.count--;

    if (_rblxwin.x11.saver.count == 0)
    {
        // Restore old screen saver settings
        XSetScreenSaver(_rblxwin.x11.display,
                        _rblxwin.x11.saver.timeout,
                        _rblxwin.x11.saver.interval,
                        _rblxwin.x11.saver.blanking,
                        _rblxwin.x11.saver.exposure);
    }
}

// Process the specified X event
//
static void processEvent(XEvent *event)
{
    int keycode = 0;
    Bool filtered = False;

    // HACK: Save scancode as some IMs clear the field in XFilterEvent
    if (event->type == KeyPress || event->type == KeyRelease)
        keycode = event->xkey.keycode;

    if (_rblxwin.x11.im)
        filtered = XFilterEvent(event, None);

    if (_rblxwin.x11.randr.available)
    {
        if (event->type == _rblxwin.x11.randr.eventBase + RRNotify)
        {
            XRRUpdateConfiguration(event);
            _rblxwinPollMonitorsX11();
            return;
        }
    }

    if (_rblxwin.x11.xkb.available)
    {
        if (event->type == _rblxwin.x11.xkb.eventBase + XkbEventCode)
        {
            if (((XkbEvent*) event)->any.xkb_type == XkbStateNotify &&
                (((XkbEvent*) event)->state.changed & XkbGroupStateMask))
            {
                _rblxwin.x11.xkb.group = ((XkbEvent*) event)->state.group;
            }

            return;
        }
    }

    if (event->type == GenericEvent)
    {
        if (_rblxwin.x11.xi.available)
        {
            _RBLXWINwindow* window = _rblxwin.x11.disabledCursorWindow;

            if (window &&
                window->rawMouseMotion &&
                event->xcookie.extension == _rblxwin.x11.xi.majorOpcode &&
                XGetEventData(_rblxwin.x11.display, &event->xcookie) &&
                event->xcookie.evtype == XI_RawMotion)
            {
                XIRawEvent* re = event->xcookie.data;
                if (re->valuators.mask_len)
                {
                    const double* values = re->raw_values;
                    double xpos = window->virtualCursorPosX;
                    double ypos = window->virtualCursorPosY;

                    if (XIMaskIsSet(re->valuators.mask, 0))
                    {
                        xpos += *values;
                        values++;
                    }

                    if (XIMaskIsSet(re->valuators.mask, 1))
                        ypos += *values;

                    _rblxwinInputCursorPos(window, xpos, ypos);
                }
            }

            XFreeEventData(_rblxwin.x11.display, &event->xcookie);
        }

        return;
    }

    if (event->type == SelectionRequest)
    {
        handleSelectionRequest(event);
        return;
    }

    _RBLXWINwindow* window = NULL;
    if (XFindContext(_rblxwin.x11.display,
                     event->xany.window,
                     _rblxwin.x11.context,
                     (XPointer*) &window) != 0)
    {
        // This is an event for a window that has already been destroyed
        return;
    }

    switch (event->type)
    {
        case ReparentNotify:
        {
            window->x11.parent = event->xreparent.parent;
            return;
        }

        case KeyPress:
        {
            const int key = translateKey(keycode);
            const int mods = translateState(event->xkey.state);
            const int plain = !(mods & (RBLXWIN_MOD_CONTROL | RBLXWIN_MOD_ALT));

            if (window->x11.ic)
            {
                // HACK: Do not report the key press events duplicated by XIM
                //       Duplicate key releases are filtered out implicitly by
                //       the RBLXWIN key repeat logic in _rblxwinInputKey
                //       A timestamp per key is used to handle simultaneous keys
                // NOTE: Always allow the first event for each key through
                //       (the server never sends a timestamp of zero)
                // NOTE: Timestamp difference is compared to handle wrap-around
                Time diff = event->xkey.time - window->x11.keyPressTimes[keycode];
                if (diff == event->xkey.time || (diff > 0 && diff < ((Time)1 << 31)))
                {
                    if (keycode)
                        _rblxwinInputKey(window, key, keycode, RBLXWIN_PRESS, mods);

                    window->x11.keyPressTimes[keycode] = event->xkey.time;
                }

                if (!filtered)
                {
                    int count;
                    Status status;
#if defined(X_HAVE_UTF8_STRING)
                    char buffer[100];
                    char* chars = buffer;

                    count = Xutf8LookupString(window->x11.ic,
                                              &event->xkey,
                                              buffer, sizeof(buffer) - 1,
                                              NULL, &status);

                    if (status == XBufferOverflow)
                    {
                        chars = calloc(count + 1, 1);
                        count = Xutf8LookupString(window->x11.ic,
                                                  &event->xkey,
                                                  chars, count,
                                                  NULL, &status);
                    }

                    if (status == XLookupChars || status == XLookupBoth)
                    {
                        const char* c = chars;
                        chars[count] = '\0';
                        while (c - chars < count)
                            _rblxwinInputChar(window, decodeUTF8(&c), mods, plain);
                    }
#else /*X_HAVE_UTF8_STRING*/
                    wchar_t buffer[16];
                    wchar_t* chars = buffer;

                    count = XwcLookupString(window->x11.ic,
                                            &event->xkey,
                                            buffer,
                                            sizeof(buffer) / sizeof(wchar_t),
                                            NULL,
                                            &status);

                    if (status == XBufferOverflow)
                    {
                        chars = calloc(count, sizeof(wchar_t));
                        count = XwcLookupString(window->x11.ic,
                                                &event->xkey,
                                                chars, count,
                                                NULL, &status);
                    }

                    if (status == XLookupChars || status == XLookupBoth)
                    {
                        int i;
                        for (i = 0;  i < count;  i++)
                            _rblxwinInputChar(window, chars[i], mods, plain);
                    }
#endif /*X_HAVE_UTF8_STRING*/

                    if (chars != buffer)
                        free(chars);
                }
            }
            else
            {
                KeySym keysym;
                XLookupString(&event->xkey, NULL, 0, &keysym, NULL);

                _rblxwinInputKey(window, key, keycode, RBLXWIN_PRESS, mods);

                const uint32_t codepoint = _rblxwinKeySym2Unicode(keysym);
                if (codepoint != RBLXWIN_INVALID_CODEPOINT)
                    _rblxwinInputChar(window, codepoint, mods, plain);
            }

            return;
        }

        case KeyRelease:
        {
            const int key = translateKey(keycode);
            const int mods = translateState(event->xkey.state);

            if (!_rblxwin.x11.xkb.detectable)
            {
                // HACK: Key repeat events will arrive as KeyRelease/KeyPress
                //       pairs with similar or identical time stamps
                //       The key repeat logic in _rblxwinInputKey expects only key
                //       presses to repeat, so detect and discard release events
                if (XEventsQueued(_rblxwin.x11.display, QueuedAfterReading))
                {
                    XEvent next;
                    XPeekEvent(_rblxwin.x11.display, &next);

                    if (next.type == KeyPress &&
                        next.xkey.window == event->xkey.window &&
                        next.xkey.keycode == keycode)
                    {
                        // HACK: The time of repeat events sometimes doesn't
                        //       match that of the press event, so add an
                        //       epsilon
                        //       Toshiyuki Takahashi can press a button
                        //       16 times per second so it's fairly safe to
                        //       assume that no human is pressing the key 50
                        //       times per second (value is ms)
                        if ((next.xkey.time - event->xkey.time) < 20)
                        {
                            // This is very likely a server-generated key repeat
                            // event, so ignore it
                            return;
                        }
                    }
                }
            }

            _rblxwinInputKey(window, key, keycode, RBLXWIN_RELEASE, mods);
            return;
        }

        case ButtonPress:
        {
            const int mods = translateState(event->xbutton.state);

            if (event->xbutton.button == Button1)
                _rblxwinInputMouseClick(window, RBLXWIN_MOUSE_BUTTON_LEFT, RBLXWIN_PRESS, mods);
            else if (event->xbutton.button == Button2)
                _rblxwinInputMouseClick(window, RBLXWIN_MOUSE_BUTTON_MIDDLE, RBLXWIN_PRESS, mods);
            else if (event->xbutton.button == Button3)
                _rblxwinInputMouseClick(window, RBLXWIN_MOUSE_BUTTON_RIGHT, RBLXWIN_PRESS, mods);

            // Modern X provides scroll events as mouse button presses
            else if (event->xbutton.button == Button4)
                _rblxwinInputScroll(window, 0.0, 1.0);
            else if (event->xbutton.button == Button5)
                _rblxwinInputScroll(window, 0.0, -1.0);
            else if (event->xbutton.button == Button6)
                _rblxwinInputScroll(window, 1.0, 0.0);
            else if (event->xbutton.button == Button7)
                _rblxwinInputScroll(window, -1.0, 0.0);

            else
            {
                // Additional buttons after 7 are treated as regular buttons
                // We subtract 4 to fill the gap left by scroll input above
                _rblxwinInputMouseClick(window,
                                     event->xbutton.button - Button1 - 4,
                                     RBLXWIN_PRESS,
                                     mods);
            }

            return;
        }

        case ButtonRelease:
        {
            const int mods = translateState(event->xbutton.state);

            if (event->xbutton.button == Button1)
            {
                _rblxwinInputMouseClick(window,
                                     RBLXWIN_MOUSE_BUTTON_LEFT,
                                     RBLXWIN_RELEASE,
                                     mods);
            }
            else if (event->xbutton.button == Button2)
            {
                _rblxwinInputMouseClick(window,
                                     RBLXWIN_MOUSE_BUTTON_MIDDLE,
                                     RBLXWIN_RELEASE,
                                     mods);
            }
            else if (event->xbutton.button == Button3)
            {
                _rblxwinInputMouseClick(window,
                                     RBLXWIN_MOUSE_BUTTON_RIGHT,
                                     RBLXWIN_RELEASE,
                                     mods);
            }
            else if (event->xbutton.button > Button7)
            {
                // Additional buttons after 7 are treated as regular buttons
                // We subtract 4 to fill the gap left by scroll input above
                _rblxwinInputMouseClick(window,
                                     event->xbutton.button - Button1 - 4,
                                     RBLXWIN_RELEASE,
                                     mods);
            }

            return;
        }

        case EnterNotify:
        {
            // XEnterWindowEvent is XCrossingEvent
            const int x = event->xcrossing.x;
            const int y = event->xcrossing.y;

            // HACK: This is a workaround for WMs (KWM, Fluxbox) that otherwise
            //       ignore the defined cursor for hidden cursor mode
            if (window->cursorMode == RBLXWIN_CURSOR_HIDDEN)
                updateCursorImage(window);

            _rblxwinInputCursorEnter(window, RBLXWIN_TRUE);
            _rblxwinInputCursorPos(window, x, y);

            window->x11.lastCursorPosX = x;
            window->x11.lastCursorPosY = y;
            return;
        }

        case LeaveNotify:
        {
            _rblxwinInputCursorEnter(window, RBLXWIN_FALSE);
            return;
        }

        case MotionNotify:
        {
            const int x = event->xmotion.x;
            const int y = event->xmotion.y;

            if (x != window->x11.warpCursorPosX ||
                y != window->x11.warpCursorPosY)
            {
                // The cursor was moved by something other than RBLXWIN

                if (window->cursorMode == RBLXWIN_CURSOR_DISABLED)
                {
                    if (_rblxwin.x11.disabledCursorWindow != window)
                        return;
                    if (window->rawMouseMotion)
                        return;

                    const int dx = x - window->x11.lastCursorPosX;
                    const int dy = y - window->x11.lastCursorPosY;

                    _rblxwinInputCursorPos(window,
                                        window->virtualCursorPosX + dx,
                                        window->virtualCursorPosY + dy);
                }
                else
                    _rblxwinInputCursorPos(window, x, y);
            }

            window->x11.lastCursorPosX = x;
            window->x11.lastCursorPosY = y;
            return;
        }

        case ConfigureNotify:
        {
            if (event->xconfigure.width != window->x11.width ||
                event->xconfigure.height != window->x11.height)
            {
                _rblxwinInputFramebufferSize(window,
                                          event->xconfigure.width,
                                          event->xconfigure.height);

                _rblxwinInputWindowSize(window,
                                     event->xconfigure.width,
                                     event->xconfigure.height);

                window->x11.width = event->xconfigure.width;
                window->x11.height = event->xconfigure.height;
            }

            int xpos = event->xconfigure.x;
            int ypos = event->xconfigure.y;

            // NOTE: ConfigureNotify events from the server are in local
            //       coordinates, so if we are reparented we need to translate
            //       the position into root (screen) coordinates
            if (!event->xany.send_event && window->x11.parent != _rblxwin.x11.root)
            {
                _rblxwinGrabErrorHandlerX11();

                Window dummy;
                XTranslateCoordinates(_rblxwin.x11.display,
                                      window->x11.parent,
                                      _rblxwin.x11.root,
                                      xpos, ypos,
                                      &xpos, &ypos,
                                      &dummy);

                _rblxwinReleaseErrorHandlerX11();
                if (_rblxwin.x11.errorCode == BadWindow)
                    return;
            }

            if (xpos != window->x11.xpos || ypos != window->x11.ypos)
            {
                _rblxwinInputWindowPos(window, xpos, ypos);
                window->x11.xpos = xpos;
                window->x11.ypos = ypos;
            }

            return;
        }

        case ClientMessage:
        {
            // Custom client message, probably from the window manager

            if (filtered)
                return;

            if (event->xclient.message_type == None)
                return;

            if (event->xclient.message_type == _rblxwin.x11.WM_PROTOCOLS)
            {
                const Atom protocol = event->xclient.data.l[0];
                if (protocol == None)
                    return;

                if (protocol == _rblxwin.x11.WM_DELETE_WINDOW)
                {
                    // The window manager was asked to close the window, for
                    // example by the user pressing a 'close' window decoration
                    // button
                    _rblxwinInputWindowCloseRequest(window);
                }
                else if (protocol == _rblxwin.x11.NET_WM_PING)
                {
                    // The window manager is pinging the application to ensure
                    // it's still responding to events

                    XEvent reply = *event;
                    reply.xclient.window = _rblxwin.x11.root;

                    XSendEvent(_rblxwin.x11.display, _rblxwin.x11.root,
                               False,
                               SubstructureNotifyMask | SubstructureRedirectMask,
                               &reply);
                }
            }
            else if (event->xclient.message_type == _rblxwin.x11.XdndEnter)
            {
                // A drag operation has entered the window
                unsigned long i, count;
                Atom* formats = NULL;
                const RBLXWINbool list = event->xclient.data.l[1] & 1;

                _rblxwin.x11.xdnd.source  = event->xclient.data.l[0];
                _rblxwin.x11.xdnd.version = event->xclient.data.l[1] >> 24;
                _rblxwin.x11.xdnd.format  = None;

                if (_rblxwin.x11.xdnd.version > _RBLXWIN_XDND_VERSION)
                    return;

                if (list)
                {
                    count = _rblxwinGetWindowPropertyX11(_rblxwin.x11.xdnd.source,
                                                      _rblxwin.x11.XdndTypeList,
                                                      XA_ATOM,
                                                      (unsigned char**) &formats);
                }
                else
                {
                    count = 3;
                    formats = (Atom*) event->xclient.data.l + 2;
                }

                for (i = 0;  i < count;  i++)
                {
                    if (formats[i] == _rblxwin.x11.text_uri_list)
                    {
                        _rblxwin.x11.xdnd.format = _rblxwin.x11.text_uri_list;
                        break;
                    }
                }

                if (list && formats)
                    XFree(formats);
            }
            else if (event->xclient.message_type == _rblxwin.x11.XdndDrop)
            {
                // The drag operation has finished by dropping on the window
                Time time = CurrentTime;

                if (_rblxwin.x11.xdnd.version > _RBLXWIN_XDND_VERSION)
                    return;

                if (_rblxwin.x11.xdnd.format)
                {
                    if (_rblxwin.x11.xdnd.version >= 1)
                        time = event->xclient.data.l[2];

                    // Request the chosen format from the source window
                    XConvertSelection(_rblxwin.x11.display,
                                      _rblxwin.x11.XdndSelection,
                                      _rblxwin.x11.xdnd.format,
                                      _rblxwin.x11.XdndSelection,
                                      window->x11.handle,
                                      time);
                }
                else if (_rblxwin.x11.xdnd.version >= 2)
                {
                    XEvent reply = { ClientMessage };
                    reply.xclient.window = _rblxwin.x11.xdnd.source;
                    reply.xclient.message_type = _rblxwin.x11.XdndFinished;
                    reply.xclient.format = 32;
                    reply.xclient.data.l[0] = window->x11.handle;
                    reply.xclient.data.l[1] = 0; // The drag was rejected
                    reply.xclient.data.l[2] = None;

                    XSendEvent(_rblxwin.x11.display, _rblxwin.x11.xdnd.source,
                               False, NoEventMask, &reply);
                    XFlush(_rblxwin.x11.display);
                }
            }
            else if (event->xclient.message_type == _rblxwin.x11.XdndPosition)
            {
                // The drag operation has moved over the window
                const int xabs = (event->xclient.data.l[2] >> 16) & 0xffff;
                const int yabs = (event->xclient.data.l[2]) & 0xffff;
                Window dummy;
                int xpos, ypos;

                if (_rblxwin.x11.xdnd.version > _RBLXWIN_XDND_VERSION)
                    return;

                XTranslateCoordinates(_rblxwin.x11.display,
                                      _rblxwin.x11.root,
                                      window->x11.handle,
                                      xabs, yabs,
                                      &xpos, &ypos,
                                      &dummy);

                _rblxwinInputCursorPos(window, xpos, ypos);

                XEvent reply = { ClientMessage };
                reply.xclient.window = _rblxwin.x11.xdnd.source;
                reply.xclient.message_type = _rblxwin.x11.XdndStatus;
                reply.xclient.format = 32;
                reply.xclient.data.l[0] = window->x11.handle;
                reply.xclient.data.l[2] = 0; // Specify an empty rectangle
                reply.xclient.data.l[3] = 0;

                if (_rblxwin.x11.xdnd.format)
                {
                    // Reply that we are ready to copy the dragged data
                    reply.xclient.data.l[1] = 1; // Accept with no rectangle
                    if (_rblxwin.x11.xdnd.version >= 2)
                        reply.xclient.data.l[4] = _rblxwin.x11.XdndActionCopy;
                }

                XSendEvent(_rblxwin.x11.display, _rblxwin.x11.xdnd.source,
                           False, NoEventMask, &reply);
                XFlush(_rblxwin.x11.display);
            }

            return;
        }

        case SelectionNotify:
        {
            if (event->xselection.property == _rblxwin.x11.XdndSelection)
            {
                // The converted data from the drag operation has arrived
                char* data;
                const unsigned long result =
                    _rblxwinGetWindowPropertyX11(event->xselection.requestor,
                                              event->xselection.property,
                                              event->xselection.target,
                                              (unsigned char**) &data);

                if (result)
                {
                    int i, count;
                    char** paths = _rblxwinParseUriList(data, &count);

                    _rblxwinInputDrop(window, count, (const char**) paths);

                    for (i = 0;  i < count;  i++)
                        free(paths[i]);
                    free(paths);
                }

                if (data)
                    XFree(data);

                if (_rblxwin.x11.xdnd.version >= 2)
                {
                    XEvent reply = { ClientMessage };
                    reply.xclient.window = _rblxwin.x11.xdnd.source;
                    reply.xclient.message_type = _rblxwin.x11.XdndFinished;
                    reply.xclient.format = 32;
                    reply.xclient.data.l[0] = window->x11.handle;
                    reply.xclient.data.l[1] = result;
                    reply.xclient.data.l[2] = _rblxwin.x11.XdndActionCopy;

                    XSendEvent(_rblxwin.x11.display, _rblxwin.x11.xdnd.source,
                               False, NoEventMask, &reply);
                    XFlush(_rblxwin.x11.display);
                }
            }

            return;
        }

        case FocusIn:
        {
            if (event->xfocus.mode == NotifyGrab ||
                event->xfocus.mode == NotifyUngrab)
            {
                // Ignore focus events from popup indicator windows, window menu
                // key chords and window dragging
                return;
            }

            if (window->cursorMode == RBLXWIN_CURSOR_DISABLED)
                disableCursor(window);

            if (window->x11.ic)
                XSetICFocus(window->x11.ic);

            _rblxwinInputWindowFocus(window, RBLXWIN_TRUE);
            return;
        }

        case FocusOut:
        {
            if (event->xfocus.mode == NotifyGrab ||
                event->xfocus.mode == NotifyUngrab)
            {
                // Ignore focus events from popup indicator windows, window menu
                // key chords and window dragging
                return;
            }

            if (window->cursorMode == RBLXWIN_CURSOR_DISABLED)
                enableCursor(window);

            if (window->x11.ic)
                XUnsetICFocus(window->x11.ic);

            if (window->monitor && window->autoIconify)
                _rblxwinPlatformIconifyWindow(window);

            _rblxwinInputWindowFocus(window, RBLXWIN_FALSE);
            return;
        }

        case Expose:
        {
            _rblxwinInputWindowDamage(window);
            return;
        }

        case PropertyNotify:
        {
            if (event->xproperty.state != PropertyNewValue)
                return;

            if (event->xproperty.atom == _rblxwin.x11.WM_STATE)
            {
                const int state = getWindowState(window);
                if (state != IconicState && state != NormalState)
                    return;

                const RBLXWINbool iconified = (state == IconicState);
                if (window->x11.iconified != iconified)
                {
                    if (window->monitor)
                    {
                        if (iconified)
                            releaseMonitor(window);
                        else
                            acquireMonitor(window);
                    }

                    window->x11.iconified = iconified;
                    _rblxwinInputWindowIconify(window, iconified);
                }
            }
            else if (event->xproperty.atom == _rblxwin.x11.NET_WM_STATE)
            {
                const RBLXWINbool maximized = _rblxwinPlatformWindowMaximized(window);
                if (window->x11.maximized != maximized)
                {
                    window->x11.maximized = maximized;
                    _rblxwinInputWindowMaximize(window, maximized);
                }
            }

            return;
        }

        case DestroyNotify:
            return;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Retrieve a single window property of the specified type
// Inspired by fghGetWindowProperty from freeglut
//
unsigned long _rblxwinGetWindowPropertyX11(Window window,
                                        Atom property,
                                        Atom type,
                                        unsigned char** value)
{
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;

    XGetWindowProperty(_rblxwin.x11.display,
                       window,
                       property,
                       0,
                       LONG_MAX,
                       False,
                       type,
                       &actualType,
                       &actualFormat,
                       &itemCount,
                       &bytesAfter,
                       value);

    return itemCount;
}

RBLXWINbool _rblxwinIsVisualTransparentX11(Visual* visual)
{
    if (!_rblxwin.x11.xrender.available)
        return RBLXWIN_FALSE;

    XRenderPictFormat* pf = XRenderFindVisualFormat(_rblxwin.x11.display, visual);
    return pf && pf->direct.alphaMask;
}

// Push contents of our selection to clipboard manager
//
void _rblxwinPushSelectionToManagerX11(void)
{
    XConvertSelection(_rblxwin.x11.display,
                      _rblxwin.x11.CLIPBOARD_MANAGER,
                      _rblxwin.x11.SAVE_TARGETS,
                      None,
                      _rblxwin.x11.helperWindowHandle,
                      CurrentTime);

    for (;;)
    {
        XEvent event;

        while (XCheckIfEvent(_rblxwin.x11.display, &event, isSelectionEvent, NULL))
        {
            switch (event.type)
            {
                case SelectionRequest:
                    handleSelectionRequest(&event);
                    break;

                case SelectionNotify:
                {
                    if (event.xselection.target == _rblxwin.x11.SAVE_TARGETS)
                    {
                        // This means one of two things; either the selection
                        // was not owned, which means there is no clipboard
                        // manager, or the transfer to the clipboard manager has
                        // completed
                        // In either case, it means we are done here
                        return;
                    }

                    break;
                }
            }
        }

        waitForX11Event(NULL);
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _rblxwinPlatformCreateWindow(_RBLXWINwindow* window,
                              const _RBLXWINwndconfig* wndconfig,
                              const _RBLXWINctxconfig* ctxconfig,
                              const _RBLXWINfbconfig* fbconfig)
{
    Visual* visual = NULL;
    int depth;

    if (ctxconfig->client != RBLXWIN_NO_API)
    {
        if (ctxconfig->source == RBLXWIN_NATIVE_CONTEXT_API)
        {
            if (!_rblxwinInitGLX())
                return RBLXWIN_FALSE;
            if (!_rblxwinChooseVisualGLX(wndconfig, ctxconfig, fbconfig, &visual, &depth))
                return RBLXWIN_FALSE;
        }
        else if (ctxconfig->source == RBLXWIN_EGL_CONTEXT_API)
        {
            if (!_rblxwinInitEGL())
                return RBLXWIN_FALSE;
            if (!_rblxwinChooseVisualEGL(wndconfig, ctxconfig, fbconfig, &visual, &depth))
                return RBLXWIN_FALSE;
        }
        else if (ctxconfig->source == RBLXWIN_OSMESA_CONTEXT_API)
        {
            if (!_rblxwinInitOSMesa())
                return RBLXWIN_FALSE;
        }
    }

    if (!visual)
    {
        visual = DefaultVisual(_rblxwin.x11.display, _rblxwin.x11.screen);
        depth = DefaultDepth(_rblxwin.x11.display, _rblxwin.x11.screen);
    }

    if (!createNativeWindow(window, wndconfig, visual, depth))
        return RBLXWIN_FALSE;

    if (ctxconfig->client != RBLXWIN_NO_API)
    {
        if (ctxconfig->source == RBLXWIN_NATIVE_CONTEXT_API)
        {
            if (!_rblxwinCreateContextGLX(window, ctxconfig, fbconfig))
                return RBLXWIN_FALSE;
        }
        else if (ctxconfig->source == RBLXWIN_EGL_CONTEXT_API)
        {
            if (!_rblxwinCreateContextEGL(window, ctxconfig, fbconfig))
                return RBLXWIN_FALSE;
        }
        else if (ctxconfig->source == RBLXWIN_OSMESA_CONTEXT_API)
        {
            if (!_rblxwinCreateContextOSMesa(window, ctxconfig, fbconfig))
                return RBLXWIN_FALSE;
        }

        if (!_rblxwinRefreshContextAttribs(window, ctxconfig))
            return RBLXWIN_FALSE;
    }

    if (window->monitor)
    {
        _rblxwinPlatformShowWindow(window);
        updateWindowMode(window);
        acquireMonitor(window);

        if (wndconfig->centerCursor)
            _rblxwinCenterCursorInContentArea(window);
    }
    else
    {
        if (wndconfig->visible)
        {
            _rblxwinPlatformShowWindow(window);
            if (wndconfig->focused)
                _rblxwinPlatformFocusWindow(window);
        }
    }

    XFlush(_rblxwin.x11.display);
    return RBLXWIN_TRUE;
}

void _rblxwinPlatformDestroyWindow(_RBLXWINwindow* window)
{
    if (_rblxwin.x11.disabledCursorWindow == window)
        _rblxwin.x11.disabledCursorWindow = NULL;

    if (window->monitor)
        releaseMonitor(window);

    if (window->x11.ic)
    {
        XDestroyIC(window->x11.ic);
        window->x11.ic = NULL;
    }

    if (window->context.destroy)
        window->context.destroy(window);

    if (window->x11.handle)
    {
        XDeleteContext(_rblxwin.x11.display, window->x11.handle, _rblxwin.x11.context);
        XUnmapWindow(_rblxwin.x11.display, window->x11.handle);
        XDestroyWindow(_rblxwin.x11.display, window->x11.handle);
        window->x11.handle = (Window) 0;
    }

    if (window->x11.colormap)
    {
        XFreeColormap(_rblxwin.x11.display, window->x11.colormap);
        window->x11.colormap = (Colormap) 0;
    }

    XFlush(_rblxwin.x11.display);
}

void _rblxwinPlatformSetWindowTitle(_RBLXWINwindow* window, const char* title)
{
#if defined(X_HAVE_UTF8_STRING)
    Xutf8SetWMProperties(_rblxwin.x11.display,
                         window->x11.handle,
                         title, title,
                         NULL, 0,
                         NULL, NULL, NULL);
#else
    // This may be a slightly better fallback than using XStoreName and
    // XSetIconName, which always store their arguments using STRING
    XmbSetWMProperties(_rblxwin.x11.display,
                       window->x11.handle,
                       title, title,
                       NULL, 0,
                       NULL, NULL, NULL);
#endif

    XChangeProperty(_rblxwin.x11.display,  window->x11.handle,
                    _rblxwin.x11.NET_WM_NAME, _rblxwin.x11.UTF8_STRING, 8,
                    PropModeReplace,
                    (unsigned char*) title, strlen(title));

    XChangeProperty(_rblxwin.x11.display,  window->x11.handle,
                    _rblxwin.x11.NET_WM_ICON_NAME, _rblxwin.x11.UTF8_STRING, 8,
                    PropModeReplace,
                    (unsigned char*) title, strlen(title));

    XFlush(_rblxwin.x11.display);
}

void _rblxwinPlatformSetWindowIcon(_RBLXWINwindow* window,
                                int count, const RBLXWINimage* images)
{
    if (count)
    {
        int i, j, longCount = 0;

        for (i = 0;  i < count;  i++)
            longCount += 2 + images[i].width * images[i].height;

        unsigned long* icon = calloc(longCount, sizeof(unsigned long));
        unsigned long* target = icon;

        for (i = 0;  i < count;  i++)
        {
            *target++ = images[i].width;
            *target++ = images[i].height;

            for (j = 0;  j < images[i].width * images[i].height;  j++)
            {
                *target++ = (((unsigned long) images[i].pixels[j * 4 + 0]) << 16) |
                            (((unsigned long) images[i].pixels[j * 4 + 1]) <<  8) |
                            (((unsigned long) images[i].pixels[j * 4 + 2]) <<  0) |
                            (((unsigned long) images[i].pixels[j * 4 + 3]) << 24);
            }
        }

        // NOTE: XChangeProperty expects 32-bit values like the image data above to be
        //       placed in the 32 least significant bits of individual longs.  This is
        //       true even if long is 64-bit and a WM protocol calls for "packed" data.
        //       This is because of a historical mistake that then became part of the Xlib
        //       ABI.  Xlib will pack these values into a regular array of 32-bit values
        //       before sending it over the wire.
        XChangeProperty(_rblxwin.x11.display, window->x11.handle,
                        _rblxwin.x11.NET_WM_ICON,
                        XA_CARDINAL, 32,
                        PropModeReplace,
                        (unsigned char*) icon,
                        longCount);

        free(icon);
    }
    else
    {
        XDeleteProperty(_rblxwin.x11.display, window->x11.handle,
                        _rblxwin.x11.NET_WM_ICON);
    }

    XFlush(_rblxwin.x11.display);
}

void _rblxwinPlatformGetWindowPos(_RBLXWINwindow* window, int* xpos, int* ypos)
{
    Window dummy;
    int x, y;

    XTranslateCoordinates(_rblxwin.x11.display, window->x11.handle, _rblxwin.x11.root,
                          0, 0, &x, &y, &dummy);

    if (xpos)
        *xpos = x;
    if (ypos)
        *ypos = y;
}

void _rblxwinPlatformSetWindowPos(_RBLXWINwindow* window, int xpos, int ypos)
{
    // HACK: Explicitly setting PPosition to any value causes some WMs, notably
    //       Compiz and Metacity, to honor the position of unmapped windows
    if (!_rblxwinPlatformWindowVisible(window))
    {
        long supplied;
        XSizeHints* hints = XAllocSizeHints();

        if (XGetWMNormalHints(_rblxwin.x11.display, window->x11.handle, hints, &supplied))
        {
            hints->flags |= PPosition;
            hints->x = hints->y = 0;

            XSetWMNormalHints(_rblxwin.x11.display, window->x11.handle, hints);
        }

        XFree(hints);
    }

    XMoveWindow(_rblxwin.x11.display, window->x11.handle, xpos, ypos);
    XFlush(_rblxwin.x11.display);
}

void _rblxwinPlatformGetWindowSize(_RBLXWINwindow* window, int* width, int* height)
{
    XWindowAttributes attribs;
    XGetWindowAttributes(_rblxwin.x11.display, window->x11.handle, &attribs);

    if (width)
        *width = attribs.width;
    if (height)
        *height = attribs.height;
}

void _rblxwinPlatformSetWindowSize(_RBLXWINwindow* window, int width, int height)
{
    if (window->monitor)
    {
        if (window->monitor->window == window)
            acquireMonitor(window);
    }
    else
    {
        if (!window->resizable)
            updateNormalHints(window, width, height);

        XResizeWindow(_rblxwin.x11.display, window->x11.handle, width, height);
    }

    XFlush(_rblxwin.x11.display);
}

void _rblxwinPlatformSetWindowSizeLimits(_RBLXWINwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight)
{
    int width, height;
    _rblxwinPlatformGetWindowSize(window, &width, &height);
    updateNormalHints(window, width, height);
    XFlush(_rblxwin.x11.display);
}

void _rblxwinPlatformSetWindowAspectRatio(_RBLXWINwindow* window, int numer, int denom)
{
    int width, height;
    _rblxwinPlatformGetWindowSize(window, &width, &height);
    updateNormalHints(window, width, height);
    XFlush(_rblxwin.x11.display);
}

void _rblxwinPlatformGetFramebufferSize(_RBLXWINwindow* window, int* width, int* height)
{
    _rblxwinPlatformGetWindowSize(window, width, height);
}

void _rblxwinPlatformGetWindowFrameSize(_RBLXWINwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom)
{
    long* extents = NULL;

    if (window->monitor || !window->decorated)
        return;

    if (_rblxwin.x11.NET_FRAME_EXTENTS == None)
        return;

    if (!_rblxwinPlatformWindowVisible(window) &&
        _rblxwin.x11.NET_REQUEST_FRAME_EXTENTS)
    {
        XEvent event;
        double timeout = 0.5;

        // Ensure _NET_FRAME_EXTENTS is set, allowing rblxwinGetWindowFrameSize to
        // function before the window is mapped
        sendEventToWM(window, _rblxwin.x11.NET_REQUEST_FRAME_EXTENTS,
                      0, 0, 0, 0, 0);

        // HACK: Use a timeout because earlier versions of some window managers
        //       (at least Unity, Fluxbox and Xfwm) failed to send the reply
        //       They have been fixed but broken versions are still in the wild
        //       If you are affected by this and your window manager is NOT
        //       listed above, PLEASE report it to their and our issue trackers
        while (!XCheckIfEvent(_rblxwin.x11.display,
                              &event,
                              isFrameExtentsEvent,
                              (XPointer) window))
        {
            if (!waitForX11Event(&timeout))
            {
                _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                                "X11: The window manager has a broken _NET_REQUEST_FRAME_EXTENTS implementation; please report this issue");
                return;
            }
        }
    }

    if (_rblxwinGetWindowPropertyX11(window->x11.handle,
                                  _rblxwin.x11.NET_FRAME_EXTENTS,
                                  XA_CARDINAL,
                                  (unsigned char**) &extents) == 4)
    {
        if (left)
            *left = extents[0];
        if (top)
            *top = extents[2];
        if (right)
            *right = extents[1];
        if (bottom)
            *bottom = extents[3];
    }

    if (extents)
        XFree(extents);
}

void _rblxwinPlatformGetWindowContentScale(_RBLXWINwindow* window,
                                        float* xscale, float* yscale)
{
    if (xscale)
        *xscale = _rblxwin.x11.contentScaleX;
    if (yscale)
        *yscale = _rblxwin.x11.contentScaleY;
}

void _rblxwinPlatformIconifyWindow(_RBLXWINwindow* window)
{
    if (window->x11.overrideRedirect)
    {
        // Override-redirect windows cannot be iconified or restored, as those
        // tasks are performed by the window manager
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "X11: Iconification of full screen windows requires a WM that supports EWMH full screen");
        return;
    }

    XIconifyWindow(_rblxwin.x11.display, window->x11.handle, _rblxwin.x11.screen);
    XFlush(_rblxwin.x11.display);
}

void _rblxwinPlatformRestoreWindow(_RBLXWINwindow* window)
{
    if (window->x11.overrideRedirect)
    {
        // Override-redirect windows cannot be iconified or restored, as those
        // tasks are performed by the window manager
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "X11: Iconification of full screen windows requires a WM that supports EWMH full screen");
        return;
    }

    if (_rblxwinPlatformWindowIconified(window))
    {
        XMapWindow(_rblxwin.x11.display, window->x11.handle);
        waitForVisibilityNotify(window);
    }
    else if (_rblxwinPlatformWindowVisible(window))
    {
        if (_rblxwin.x11.NET_WM_STATE &&
            _rblxwin.x11.NET_WM_STATE_MAXIMIZED_VERT &&
            _rblxwin.x11.NET_WM_STATE_MAXIMIZED_HORZ)
        {
            sendEventToWM(window,
                          _rblxwin.x11.NET_WM_STATE,
                          _NET_WM_STATE_REMOVE,
                          _rblxwin.x11.NET_WM_STATE_MAXIMIZED_VERT,
                          _rblxwin.x11.NET_WM_STATE_MAXIMIZED_HORZ,
                          1, 0);
        }
    }

    XFlush(_rblxwin.x11.display);
}

void _rblxwinPlatformMaximizeWindow(_RBLXWINwindow* window)
{
    if (!_rblxwin.x11.NET_WM_STATE ||
        !_rblxwin.x11.NET_WM_STATE_MAXIMIZED_VERT ||
        !_rblxwin.x11.NET_WM_STATE_MAXIMIZED_HORZ)
    {
        return;
    }

    if (_rblxwinPlatformWindowVisible(window))
    {
        sendEventToWM(window,
                    _rblxwin.x11.NET_WM_STATE,
                    _NET_WM_STATE_ADD,
                    _rblxwin.x11.NET_WM_STATE_MAXIMIZED_VERT,
                    _rblxwin.x11.NET_WM_STATE_MAXIMIZED_HORZ,
                    1, 0);
    }
    else
    {
        Atom* states = NULL;
        unsigned long count =
            _rblxwinGetWindowPropertyX11(window->x11.handle,
                                      _rblxwin.x11.NET_WM_STATE,
                                      XA_ATOM,
                                      (unsigned char**) &states);

        // NOTE: We don't check for failure as this property may not exist yet
        //       and that's fine (and we'll create it implicitly with append)

        Atom missing[2] =
        {
            _rblxwin.x11.NET_WM_STATE_MAXIMIZED_VERT,
            _rblxwin.x11.NET_WM_STATE_MAXIMIZED_HORZ
        };
        unsigned long missingCount = 2;

        for (unsigned long i = 0;  i < count;  i++)
        {
            for (unsigned long j = 0;  j < missingCount;  j++)
            {
                if (states[i] == missing[j])
                {
                    missing[j] = missing[missingCount - 1];
                    missingCount--;
                }
            }
        }

        if (states)
            XFree(states);

        if (!missingCount)
            return;

        XChangeProperty(_rblxwin.x11.display, window->x11.handle,
                        _rblxwin.x11.NET_WM_STATE, XA_ATOM, 32,
                        PropModeAppend,
                        (unsigned char*) missing,
                        missingCount);
    }

    XFlush(_rblxwin.x11.display);
}

void _rblxwinPlatformShowWindow(_RBLXWINwindow* window)
{
    if (_rblxwinPlatformWindowVisible(window))
        return;

    XMapWindow(_rblxwin.x11.display, window->x11.handle);
    waitForVisibilityNotify(window);
}

void _rblxwinPlatformHideWindow(_RBLXWINwindow* window)
{
    XUnmapWindow(_rblxwin.x11.display, window->x11.handle);
    XFlush(_rblxwin.x11.display);
}

void _rblxwinPlatformRequestWindowAttention(_RBLXWINwindow* window)
{
    if (!_rblxwin.x11.NET_WM_STATE || !_rblxwin.x11.NET_WM_STATE_DEMANDS_ATTENTION)
        return;

    sendEventToWM(window,
                  _rblxwin.x11.NET_WM_STATE,
                  _NET_WM_STATE_ADD,
                  _rblxwin.x11.NET_WM_STATE_DEMANDS_ATTENTION,
                  0, 1, 0);
}

void _rblxwinPlatformFocusWindow(_RBLXWINwindow* window)
{
    if (_rblxwin.x11.NET_ACTIVE_WINDOW)
        sendEventToWM(window, _rblxwin.x11.NET_ACTIVE_WINDOW, 1, 0, 0, 0, 0);
    else if (_rblxwinPlatformWindowVisible(window))
    {
        XRaiseWindow(_rblxwin.x11.display, window->x11.handle);
        XSetInputFocus(_rblxwin.x11.display, window->x11.handle,
                       RevertToParent, CurrentTime);
    }

    XFlush(_rblxwin.x11.display);
}

void _rblxwinPlatformSetWindowMonitor(_RBLXWINwindow* window,
                                   _RBLXWINmonitor* monitor,
                                   int xpos, int ypos,
                                   int width, int height,
                                   int refreshRate)
{
    if (window->monitor == monitor)
    {
        if (monitor)
        {
            if (monitor->window == window)
                acquireMonitor(window);
        }
        else
        {
            if (!window->resizable)
                updateNormalHints(window, width, height);

            XMoveResizeWindow(_rblxwin.x11.display, window->x11.handle,
                              xpos, ypos, width, height);
        }

        XFlush(_rblxwin.x11.display);
        return;
    }

    if (window->monitor)
    {
        _rblxwinPlatformSetWindowDecorated(window, window->decorated);
        _rblxwinPlatformSetWindowFloating(window, window->floating);
        releaseMonitor(window);
    }

    _rblxwinInputWindowMonitor(window, monitor);
    updateNormalHints(window, width, height);

    if (window->monitor)
    {
        if (!_rblxwinPlatformWindowVisible(window))
        {
            XMapRaised(_rblxwin.x11.display, window->x11.handle);
            waitForVisibilityNotify(window);
        }

        updateWindowMode(window);
        acquireMonitor(window);
    }
    else
    {
        updateWindowMode(window);
        XMoveResizeWindow(_rblxwin.x11.display, window->x11.handle,
                          xpos, ypos, width, height);
    }

    XFlush(_rblxwin.x11.display);
}

int _rblxwinPlatformWindowFocused(_RBLXWINwindow* window)
{
    Window focused;
    int state;

    XGetInputFocus(_rblxwin.x11.display, &focused, &state);
    return window->x11.handle == focused;
}

int _rblxwinPlatformWindowIconified(_RBLXWINwindow* window)
{
    return getWindowState(window) == IconicState;
}

int _rblxwinPlatformWindowVisible(_RBLXWINwindow* window)
{
    XWindowAttributes wa;
    XGetWindowAttributes(_rblxwin.x11.display, window->x11.handle, &wa);
    return wa.map_state == IsViewable;
}

int _rblxwinPlatformWindowMaximized(_RBLXWINwindow* window)
{
    Atom* states;
    unsigned long i;
    RBLXWINbool maximized = RBLXWIN_FALSE;

    if (!_rblxwin.x11.NET_WM_STATE ||
        !_rblxwin.x11.NET_WM_STATE_MAXIMIZED_VERT ||
        !_rblxwin.x11.NET_WM_STATE_MAXIMIZED_HORZ)
    {
        return maximized;
    }

    const unsigned long count =
        _rblxwinGetWindowPropertyX11(window->x11.handle,
                                  _rblxwin.x11.NET_WM_STATE,
                                  XA_ATOM,
                                  (unsigned char**) &states);

    for (i = 0;  i < count;  i++)
    {
        if (states[i] == _rblxwin.x11.NET_WM_STATE_MAXIMIZED_VERT ||
            states[i] == _rblxwin.x11.NET_WM_STATE_MAXIMIZED_HORZ)
        {
            maximized = RBLXWIN_TRUE;
            break;
        }
    }

    if (states)
        XFree(states);

    return maximized;
}

int _rblxwinPlatformWindowHovered(_RBLXWINwindow* window)
{
    Window w = _rblxwin.x11.root;
    while (w)
    {
        Window root;
        int rootX, rootY, childX, childY;
        unsigned int mask;

        _rblxwinGrabErrorHandlerX11();

        const Bool result = XQueryPointer(_rblxwin.x11.display, w,
                                          &root, &w, &rootX, &rootY,
                                          &childX, &childY, &mask);

        _rblxwinReleaseErrorHandlerX11();

        if (_rblxwin.x11.errorCode == BadWindow)
            w = _rblxwin.x11.root;
        else if (!result)
            return RBLXWIN_FALSE;
        else if (w == window->x11.handle)
            return RBLXWIN_TRUE;
    }

    return RBLXWIN_FALSE;
}

int _rblxwinPlatformFramebufferTransparent(_RBLXWINwindow* window)
{
    if (!window->x11.transparent)
        return RBLXWIN_FALSE;

    return XGetSelectionOwner(_rblxwin.x11.display, _rblxwin.x11.NET_WM_CM_Sx) != None;
}

void _rblxwinPlatformSetWindowResizable(_RBLXWINwindow* window, RBLXWINbool enabled)
{
    int width, height;
    _rblxwinPlatformGetWindowSize(window, &width, &height);
    updateNormalHints(window, width, height);
}

void _rblxwinPlatformSetWindowDecorated(_RBLXWINwindow* window, RBLXWINbool enabled)
{
    struct
    {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long input_mode;
        unsigned long status;
    } hints = {0};

    hints.flags = MWM_HINTS_DECORATIONS;
    hints.decorations = enabled ? MWM_DECOR_ALL : 0;

    XChangeProperty(_rblxwin.x11.display, window->x11.handle,
                    _rblxwin.x11.MOTIF_WM_HINTS,
                    _rblxwin.x11.MOTIF_WM_HINTS, 32,
                    PropModeReplace,
                    (unsigned char*) &hints,
                    sizeof(hints) / sizeof(long));
}

void _rblxwinPlatformSetWindowFloating(_RBLXWINwindow* window, RBLXWINbool enabled)
{
    if (!_rblxwin.x11.NET_WM_STATE || !_rblxwin.x11.NET_WM_STATE_ABOVE)
        return;

    if (_rblxwinPlatformWindowVisible(window))
    {
        const long action = enabled ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
        sendEventToWM(window,
                      _rblxwin.x11.NET_WM_STATE,
                      action,
                      _rblxwin.x11.NET_WM_STATE_ABOVE,
                      0, 1, 0);
    }
    else
    {
        Atom* states = NULL;
        unsigned long i, count;

        count = _rblxwinGetWindowPropertyX11(window->x11.handle,
                                          _rblxwin.x11.NET_WM_STATE,
                                          XA_ATOM,
                                          (unsigned char**) &states);

        // NOTE: We don't check for failure as this property may not exist yet
        //       and that's fine (and we'll create it implicitly with append)

        if (enabled)
        {
            for (i = 0;  i < count;  i++)
            {
                if (states[i] == _rblxwin.x11.NET_WM_STATE_ABOVE)
                    break;
            }

            if (i == count)
            {
                XChangeProperty(_rblxwin.x11.display, window->x11.handle,
                                _rblxwin.x11.NET_WM_STATE, XA_ATOM, 32,
                                PropModeAppend,
                                (unsigned char*) &_rblxwin.x11.NET_WM_STATE_ABOVE,
                                1);
            }
        }
        else if (states)
        {
            for (i = 0;  i < count;  i++)
            {
                if (states[i] == _rblxwin.x11.NET_WM_STATE_ABOVE)
                    break;
            }

            if (i < count)
            {
                states[i] = states[count - 1];
                count--;

                XChangeProperty(_rblxwin.x11.display, window->x11.handle,
                                _rblxwin.x11.NET_WM_STATE, XA_ATOM, 32,
                                PropModeReplace, (unsigned char*) states, count);
            }
        }

        if (states)
            XFree(states);
    }

    XFlush(_rblxwin.x11.display);
}

float _rblxwinPlatformGetWindowOpacity(_RBLXWINwindow* window)
{
    float opacity = 1.f;

    if (XGetSelectionOwner(_rblxwin.x11.display, _rblxwin.x11.NET_WM_CM_Sx))
    {
        CARD32* value = NULL;

        if (_rblxwinGetWindowPropertyX11(window->x11.handle,
                                      _rblxwin.x11.NET_WM_WINDOW_OPACITY,
                                      XA_CARDINAL,
                                      (unsigned char**) &value))
        {
            opacity = (float) (*value / (double) 0xffffffffu);
        }

        if (value)
            XFree(value);
    }

    return opacity;
}

void _rblxwinPlatformSetWindowOpacity(_RBLXWINwindow* window, float opacity)
{
    const CARD32 value = (CARD32) (0xffffffffu * (double) opacity);
    XChangeProperty(_rblxwin.x11.display, window->x11.handle,
                    _rblxwin.x11.NET_WM_WINDOW_OPACITY, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char*) &value, 1);
}

void _rblxwinPlatformSetRawMouseMotion(_RBLXWINwindow *window, RBLXWINbool enabled)
{
    if (!_rblxwin.x11.xi.available)
        return;

    if (_rblxwin.x11.disabledCursorWindow != window)
        return;

    if (enabled)
        enableRawMouseMotion(window);
    else
        disableRawMouseMotion(window);
}

RBLXWINbool _rblxwinPlatformRawMouseMotionSupported(void)
{
    return _rblxwin.x11.xi.available;
}

void _rblxwinPlatformPollEvents(void)
{
    drainEmptyEvents();

#if defined(__linux__)
    _rblxwinDetectJoystickConnectionLinux();
#endif
    XPending(_rblxwin.x11.display);

    while (XQLength(_rblxwin.x11.display))
    {
        XEvent event;
        XNextEvent(_rblxwin.x11.display, &event);
        processEvent(&event);
    }

    _RBLXWINwindow* window = _rblxwin.x11.disabledCursorWindow;
    if (window)
    {
        int width, height;
        _rblxwinPlatformGetWindowSize(window, &width, &height);

        // NOTE: Re-center the cursor only if it has moved since the last call,
        //       to avoid breaking rblxwinWaitEvents with MotionNotify
        if (window->x11.lastCursorPosX != width / 2 ||
            window->x11.lastCursorPosY != height / 2)
        {
            _rblxwinPlatformSetCursorPos(window, width / 2, height / 2);
        }
    }

    XFlush(_rblxwin.x11.display);
}

void _rblxwinPlatformWaitEvents(void)
{
    waitForAnyEvent(NULL);
    _rblxwinPlatformPollEvents();
}

void _rblxwinPlatformWaitEventsTimeout(double timeout)
{
    waitForAnyEvent(&timeout);
    _rblxwinPlatformPollEvents();
}

void _rblxwinPlatformPostEmptyEvent(void)
{
    writeEmptyEvent();
}

void _rblxwinPlatformGetCursorPos(_RBLXWINwindow* window, double* xpos, double* ypos)
{
    Window root, child;
    int rootX, rootY, childX, childY;
    unsigned int mask;

    XQueryPointer(_rblxwin.x11.display, window->x11.handle,
                  &root, &child,
                  &rootX, &rootY, &childX, &childY,
                  &mask);

    if (xpos)
        *xpos = childX;
    if (ypos)
        *ypos = childY;
}

void _rblxwinPlatformSetCursorPos(_RBLXWINwindow* window, double x, double y)
{
    // Store the new position so it can be recognized later
    window->x11.warpCursorPosX = (int) x;
    window->x11.warpCursorPosY = (int) y;

    XWarpPointer(_rblxwin.x11.display, None, window->x11.handle,
                 0,0,0,0, (int) x, (int) y);
    XFlush(_rblxwin.x11.display);
}

void _rblxwinPlatformSetCursorMode(_RBLXWINwindow* window, int mode)
{
    if (mode == RBLXWIN_CURSOR_DISABLED)
    {
        if (_rblxwinPlatformWindowFocused(window))
            disableCursor(window);
    }
    else if (_rblxwin.x11.disabledCursorWindow == window)
        enableCursor(window);
    else
        updateCursorImage(window);

    XFlush(_rblxwin.x11.display);
}

const char* _rblxwinPlatformGetScancodeName(int scancode)
{
    if (!_rblxwin.x11.xkb.available)
        return NULL;

    if (scancode < 0 || scancode > 0xff ||
        _rblxwin.x11.keycodes[scancode] == RBLXWIN_KEY_UNKNOWN)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE, "Invalid scancode %i", scancode);
        return NULL;
    }

    const int key = _rblxwin.x11.keycodes[scancode];
    const KeySym keysym = XkbKeycodeToKeysym(_rblxwin.x11.display,
                                             scancode, _rblxwin.x11.xkb.group, 0);
    if (keysym == NoSymbol)
        return NULL;

    const uint32_t codepoint = _rblxwinKeySym2Unicode(keysym);
    if (codepoint == RBLXWIN_INVALID_CODEPOINT)
        return NULL;

    const size_t count = _rblxwinEncodeUTF8(_rblxwin.x11.keynames[key], codepoint);
    if (count == 0)
        return NULL;

    _rblxwin.x11.keynames[key][count] = '\0';
    return _rblxwin.x11.keynames[key];
}

int _rblxwinPlatformGetKeyScancode(int key)
{
    return _rblxwin.x11.scancodes[key];
}

int _rblxwinPlatformCreateCursor(_RBLXWINcursor* cursor,
                              const RBLXWINimage* image,
                              int xhot, int yhot)
{
    cursor->x11.handle = _rblxwinCreateCursorX11(image, xhot, yhot);
    if (!cursor->x11.handle)
        return RBLXWIN_FALSE;

    return RBLXWIN_TRUE;
}

int _rblxwinPlatformCreateStandardCursor(_RBLXWINcursor* cursor, int shape)
{
    int native = 0;

    if (shape == RBLXWIN_ARROW_CURSOR)
        native = XC_left_ptr;
    else if (shape == RBLXWIN_IBEAM_CURSOR)
        native = XC_xterm;
    else if (shape == RBLXWIN_CROSSHAIR_CURSOR)
        native = XC_crosshair;
    else if (shape == RBLXWIN_HAND_CURSOR)
        native = XC_hand2;
    else if (shape == RBLXWIN_HRESIZE_CURSOR)
        native = XC_sb_h_double_arrow;
    else if (shape == RBLXWIN_VRESIZE_CURSOR)
        native = XC_sb_v_double_arrow;
    else
        return RBLXWIN_FALSE;

    cursor->x11.handle = XCreateFontCursor(_rblxwin.x11.display, native);
    if (!cursor->x11.handle)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "X11: Failed to create standard cursor");
        return RBLXWIN_FALSE;
    }

    return RBLXWIN_TRUE;
}

void _rblxwinPlatformDestroyCursor(_RBLXWINcursor* cursor)
{
    if (cursor->x11.handle)
        XFreeCursor(_rblxwin.x11.display, cursor->x11.handle);
}

void _rblxwinPlatformSetCursor(_RBLXWINwindow* window, _RBLXWINcursor* cursor)
{
    if (window->cursorMode == RBLXWIN_CURSOR_NORMAL)
    {
        updateCursorImage(window);
        XFlush(_rblxwin.x11.display);
    }
}

void _rblxwinPlatformSetClipboardString(const char* string)
{
    char* copy = _rblxwin_strdup(string);
    free(_rblxwin.x11.clipboardString);
    _rblxwin.x11.clipboardString = copy;

    XSetSelectionOwner(_rblxwin.x11.display,
                       _rblxwin.x11.CLIPBOARD,
                       _rblxwin.x11.helperWindowHandle,
                       CurrentTime);

    if (XGetSelectionOwner(_rblxwin.x11.display, _rblxwin.x11.CLIPBOARD) !=
        _rblxwin.x11.helperWindowHandle)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "X11: Failed to become owner of clipboard selection");
    }
}

const char* _rblxwinPlatformGetClipboardString(void)
{
    return getSelectionString(_rblxwin.x11.CLIPBOARD);
}

void _rblxwinPlatformGetRequiredInstanceExtensions(char** extensions)
{
    if (!_rblxwin.vk.KHR_surface)
        return;

    if (!_rblxwin.vk.KHR_xcb_surface || !_rblxwin.x11.x11xcb.handle)
    {
        if (!_rblxwin.vk.KHR_xlib_surface)
            return;
    }

    extensions[0] = "VK_KHR_surface";

    // NOTE: VK_KHR_xcb_surface is preferred due to some early ICDs exposing but
    //       not correctly implementing VK_KHR_xlib_surface
    if (_rblxwin.vk.KHR_xcb_surface && _rblxwin.x11.x11xcb.handle)
        extensions[1] = "VK_KHR_xcb_surface";
    else
        extensions[1] = "VK_KHR_xlib_surface";
}

int _rblxwinPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily)
{
    VisualID visualID = XVisualIDFromVisual(DefaultVisual(_rblxwin.x11.display,
                                                          _rblxwin.x11.screen));

    if (_rblxwin.vk.KHR_xcb_surface && _rblxwin.x11.x11xcb.handle)
    {
        PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR
            vkGetPhysicalDeviceXcbPresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)
            vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
        if (!vkGetPhysicalDeviceXcbPresentationSupportKHR)
        {
            _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                            "X11: Vulkan instance missing VK_KHR_xcb_surface extension");
            return RBLXWIN_FALSE;
        }

        xcb_connection_t* connection = XGetXCBConnection(_rblxwin.x11.display);
        if (!connection)
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "X11: Failed to retrieve XCB connection");
            return RBLXWIN_FALSE;
        }

        return vkGetPhysicalDeviceXcbPresentationSupportKHR(device,
                                                            queuefamily,
                                                            connection,
                                                            visualID);
    }
    else
    {
        PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR
            vkGetPhysicalDeviceXlibPresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)
            vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
        if (!vkGetPhysicalDeviceXlibPresentationSupportKHR)
        {
            _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                            "X11: Vulkan instance missing VK_KHR_xlib_surface extension");
            return RBLXWIN_FALSE;
        }

        return vkGetPhysicalDeviceXlibPresentationSupportKHR(device,
                                                             queuefamily,
                                                             _rblxwin.x11.display,
                                                             visualID);
    }
}

VkResult _rblxwinPlatformCreateWindowSurface(VkInstance instance,
                                          _RBLXWINwindow* window,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR* surface)
{
    if (_rblxwin.vk.KHR_xcb_surface && _rblxwin.x11.x11xcb.handle)
    {
        VkResult err;
        VkXcbSurfaceCreateInfoKHR sci;
        PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR;

        xcb_connection_t* connection = XGetXCBConnection(_rblxwin.x11.display);
        if (!connection)
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "X11: Failed to retrieve XCB connection");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)
            vkGetInstanceProcAddr(instance, "vkCreateXcbSurfaceKHR");
        if (!vkCreateXcbSurfaceKHR)
        {
            _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                            "X11: Vulkan instance missing VK_KHR_xcb_surface extension");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        memset(&sci, 0, sizeof(sci));
        sci.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        sci.connection = connection;
        sci.window = window->x11.handle;

        err = vkCreateXcbSurfaceKHR(instance, &sci, allocator, surface);
        if (err)
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "X11: Failed to create Vulkan XCB surface: %s",
                            _rblxwinGetVulkanResultString(err));
        }

        return err;
    }
    else
    {
        VkResult err;
        VkXlibSurfaceCreateInfoKHR sci;
        PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;

        vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)
            vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
        if (!vkCreateXlibSurfaceKHR)
        {
            _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                            "X11: Vulkan instance missing VK_KHR_xlib_surface extension");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        memset(&sci, 0, sizeof(sci));
        sci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        sci.dpy = _rblxwin.x11.display;
        sci.window = window->x11.handle;

        err = vkCreateXlibSurfaceKHR(instance, &sci, allocator, surface);
        if (err)
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "X11: Failed to create Vulkan X11 surface: %s",
                            _rblxwinGetVulkanResultString(err));
        }

        return err;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                        RBLXWIN native API                       //////
//////////////////////////////////////////////////////////////////////////

RBLXWINAPI Display* rblxwinGetX11Display(void)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return _rblxwin.x11.display;
}

RBLXWINAPI Window rblxwinGetX11Window(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(None);
    return window->x11.handle;
}

RBLXWINAPI void rblxwinSetX11SelectionString(const char* string)
{
    _RBLXWIN_REQUIRE_INIT();

    free(_rblxwin.x11.primarySelectionString);
    _rblxwin.x11.primarySelectionString = _rblxwin_strdup(string);

    XSetSelectionOwner(_rblxwin.x11.display,
                       _rblxwin.x11.PRIMARY,
                       _rblxwin.x11.helperWindowHandle,
                       CurrentTime);

    if (XGetSelectionOwner(_rblxwin.x11.display, _rblxwin.x11.PRIMARY) !=
        _rblxwin.x11.helperWindowHandle)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "X11: Failed to become owner of primary selection");
    }
}

RBLXWINAPI const char* rblxwinGetX11SelectionString(void)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return getSelectionString(_rblxwin.x11.PRIMARY);
}

