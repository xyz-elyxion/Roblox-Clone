//========================================================================
// Iconify/restore test program
// Copyright (c) Camilla Löwy <elmindreda@rblxwin.org>
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
//
// This program is used to test the iconify/restore functionality for
// both full screen and windowed mode windows
//
//========================================================================

#include <rblxgl/gl.h>
#define RBLXWIN_INCLUDE_NONE
#include <RBLXWIN/rblxwin.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

static int windowed_xpos, windowed_ypos, windowed_width = 640, windowed_height = 480;

static void usage(void)
{
    printf("Usage: iconify [-h] [-f [-a] [-n]]\n");
    printf("Options:\n");
    printf("  -a create windows for all monitors\n");
    printf("  -f create full screen window(s)\n");
    printf("  -h show this help\n");
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(RBLXWINwindow* window, int key, int scancode, int action, int mods)
{
    printf("%0.2f Key %s\n",
           rblxwinGetTime(),
           action == RBLXWIN_PRESS ? "pressed" : "released");

    if (action != RBLXWIN_PRESS)
        return;

    switch (key)
    {
        case RBLXWIN_KEY_I:
            rblxwinIconifyWindow(window);
            break;
        case RBLXWIN_KEY_M:
            rblxwinMaximizeWindow(window);
            break;
        case RBLXWIN_KEY_R:
            rblxwinRestoreWindow(window);
            break;
        case RBLXWIN_KEY_ESCAPE:
            rblxwinSetWindowShouldClose(window, RBLXWIN_TRUE);
            break;
        case RBLXWIN_KEY_A:
            rblxwinSetWindowAttrib(window, RBLXWIN_AUTO_ICONIFY, !rblxwinGetWindowAttrib(window, RBLXWIN_AUTO_ICONIFY));
            break;
        case RBLXWIN_KEY_B:
            rblxwinSetWindowAttrib(window, RBLXWIN_RESIZABLE, !rblxwinGetWindowAttrib(window, RBLXWIN_RESIZABLE));
            break;
        case RBLXWIN_KEY_D:
            rblxwinSetWindowAttrib(window, RBLXWIN_DECORATED, !rblxwinGetWindowAttrib(window, RBLXWIN_DECORATED));
            break;
        case RBLXWIN_KEY_F:
            rblxwinSetWindowAttrib(window, RBLXWIN_FLOATING, !rblxwinGetWindowAttrib(window, RBLXWIN_FLOATING));
            break;
        case RBLXWIN_KEY_F11:
        case RBLXWIN_KEY_ENTER:
        {
            if (mods != RBLXWIN_MOD_ALT)
                return;

            if (rblxwinGetWindowMonitor(window))
            {
                rblxwinSetWindowMonitor(window, NULL,
                                     windowed_xpos, windowed_ypos,
                                     windowed_width, windowed_height,
                                     0);
            }
            else
            {
                RBLXWINmonitor* monitor = rblxwinGetPrimaryMonitor();
                if (monitor)
                {
                    const RBLXWINvidmode* mode = rblxwinGetVideoMode(monitor);
                    rblxwinGetWindowPos(window, &windowed_xpos, &windowed_ypos);
                    rblxwinGetWindowSize(window, &windowed_width, &windowed_height);
                    rblxwinSetWindowMonitor(window, monitor,
                                         0, 0, mode->width, mode->height,
                                         mode->refreshRate);
                }
            }

            break;
        }
    }
}

static void window_size_callback(RBLXWINwindow* window, int width, int height)
{
    printf("%0.2f Window resized to %ix%i\n", rblxwinGetTime(), width, height);
}

static void framebuffer_size_callback(RBLXWINwindow* window, int width, int height)
{
    printf("%0.2f Framebuffer resized to %ix%i\n", rblxwinGetTime(), width, height);
}

static void window_focus_callback(RBLXWINwindow* window, int focused)
{
    printf("%0.2f Window %s\n",
           rblxwinGetTime(),
           focused ? "focused" : "defocused");
}

static void window_iconify_callback(RBLXWINwindow* window, int iconified)
{
    printf("%0.2f Window %s\n",
           rblxwinGetTime(),
           iconified ? "iconified" : "uniconified");
}

static void window_maximize_callback(RBLXWINwindow* window, int maximized)
{
    printf("%0.2f Window %s\n",
           rblxwinGetTime(),
           maximized ? "maximized" : "unmaximized");
}

static void window_refresh_callback(RBLXWINwindow* window)
{
    printf("%0.2f Window refresh\n", rblxwinGetTime());

    rblxwinMakeContextCurrent(window);

    glClear(GL_COLOR_BUFFER_BIT);
    rblxwinSwapBuffers(window);
}

static RBLXWINwindow* create_window(RBLXWINmonitor* monitor)
{
    int width, height;
    RBLXWINwindow* window;

    if (monitor)
    {
        const RBLXWINvidmode* mode = rblxwinGetVideoMode(monitor);

        rblxwinWindowHint(RBLXWIN_REFRESH_RATE, mode->refreshRate);
        rblxwinWindowHint(RBLXWIN_RED_BITS, mode->redBits);
        rblxwinWindowHint(RBLXWIN_GREEN_BITS, mode->greenBits);
        rblxwinWindowHint(RBLXWIN_BLUE_BITS, mode->blueBits);

        width = mode->width;
        height = mode->height;
    }
    else
    {
        width = windowed_width;
        height = windowed_height;
    }

    window = rblxwinCreateWindow(width, height, "Iconify", monitor, NULL);
    if (!window)
    {
        rblxwinTerminate();
        exit(EXIT_FAILURE);
    }

    rblxwinMakeContextCurrent(window);
    rblxglLoadGL(rblxwinGetProcAddress);

    return window;
}

int main(int argc, char** argv)
{
    int ch, i, window_count;
    int fullscreen = RBLXWIN_FALSE, all_monitors = RBLXWIN_FALSE;
    RBLXWINwindow** windows;

    while ((ch = getopt(argc, argv, "afhn")) != -1)
    {
        switch (ch)
        {
            case 'a':
                all_monitors = RBLXWIN_TRUE;
                break;

            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 'f':
                fullscreen = RBLXWIN_TRUE;
                break;

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    rblxwinSetErrorCallback(error_callback);

    if (!rblxwinInit())
        exit(EXIT_FAILURE);

    if (fullscreen && all_monitors)
    {
        int monitor_count;
        RBLXWINmonitor** monitors = rblxwinGetMonitors(&monitor_count);

        window_count = monitor_count;
        windows = calloc(window_count, sizeof(RBLXWINwindow*));

        for (i = 0;  i < monitor_count;  i++)
        {
            windows[i] = create_window(monitors[i]);
            if (!windows[i])
                break;
        }
    }
    else
    {
        RBLXWINmonitor* monitor = NULL;

        if (fullscreen)
            monitor = rblxwinGetPrimaryMonitor();

        window_count = 1;
        windows = calloc(window_count, sizeof(RBLXWINwindow*));
        windows[0] = create_window(monitor);
    }

    for (i = 0;  i < window_count;  i++)
    {
        rblxwinSetKeyCallback(windows[i], key_callback);
        rblxwinSetFramebufferSizeCallback(windows[i], framebuffer_size_callback);
        rblxwinSetWindowSizeCallback(windows[i], window_size_callback);
        rblxwinSetWindowFocusCallback(windows[i], window_focus_callback);
        rblxwinSetWindowIconifyCallback(windows[i], window_iconify_callback);
        rblxwinSetWindowMaximizeCallback(windows[i], window_maximize_callback);
        rblxwinSetWindowRefreshCallback(windows[i], window_refresh_callback);

        window_refresh_callback(windows[i]);

        printf("Window is %s and %s\n",
            rblxwinGetWindowAttrib(windows[i], RBLXWIN_ICONIFIED) ? "iconified" : "restored",
            rblxwinGetWindowAttrib(windows[i], RBLXWIN_FOCUSED) ? "focused" : "defocused");
    }

    for (;;)
    {
        rblxwinWaitEvents();

        for (i = 0;  i < window_count;  i++)
        {
            if (rblxwinWindowShouldClose(windows[i]))
                break;
        }

        if (i < window_count)
            break;

        // Workaround for an issue with msvcrt and mintty
        fflush(stdout);
    }

    rblxwinTerminate();
    exit(EXIT_SUCCESS);
}

