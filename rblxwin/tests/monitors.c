//========================================================================
// Monitor information tool
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
// This test prints monitor and video mode information or verifies video
// modes
//
//========================================================================

#include <rblxgl/gl.h>
#define RBLXWIN_INCLUDE_NONE
#include <RBLXWIN/rblxwin.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "getopt.h"

enum Mode
{
    LIST_MODE,
    TEST_MODE
};

static void usage(void)
{
    printf("Usage: monitors [-t]\n");
    printf("       monitors -h\n");
}

static int euclid(int a, int b)
{
    return b ? euclid(b, a % b) : a;
}

static const char* format_mode(const RBLXWINvidmode* mode)
{
    static char buffer[512];
    const int gcd = euclid(mode->width, mode->height);

    snprintf(buffer,
             sizeof(buffer),
             "%i x %i x %i (%i:%i) (%i %i %i) %i Hz",
             mode->width, mode->height,
             mode->redBits + mode->greenBits + mode->blueBits,
             mode->width / gcd, mode->height / gcd,
             mode->redBits, mode->greenBits, mode->blueBits,
             mode->refreshRate);

    buffer[sizeof(buffer) - 1] = '\0';
    return buffer;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(RBLXWINwindow* window, int width, int height)
{
    printf("Framebuffer resized to %ix%i\n", width, height);

    glViewport(0, 0, width, height);
}

static void key_callback(RBLXWINwindow* window, int key, int scancode, int action, int mods)
{
    if (key == RBLXWIN_KEY_ESCAPE)
        rblxwinSetWindowShouldClose(window, RBLXWIN_TRUE);
}

static void list_modes(RBLXWINmonitor* monitor)
{
    int count, x, y, width_mm, height_mm, i;
    int workarea_x, workarea_y, workarea_width, workarea_height;
    float xscale, yscale;

    const RBLXWINvidmode* mode = rblxwinGetVideoMode(monitor);
    const RBLXWINvidmode* modes = rblxwinGetVideoModes(monitor, &count);

    rblxwinGetMonitorPos(monitor, &x, &y);
    rblxwinGetMonitorPhysicalSize(monitor, &width_mm, &height_mm);
    rblxwinGetMonitorContentScale(monitor, &xscale, &yscale);
    rblxwinGetMonitorWorkarea(monitor, &workarea_x, &workarea_y, &workarea_width, &workarea_height);

    printf("Name: %s (%s)\n",
           rblxwinGetMonitorName(monitor),
           rblxwinGetPrimaryMonitor() == monitor ? "primary" : "secondary");
    printf("Current mode: %s\n", format_mode(mode));
    printf("Virtual position: %i, %i\n", x, y);
    printf("Content scale: %f x %f\n", xscale, yscale);

    printf("Physical size: %i x %i mm (%0.2f dpi at %i x %i)\n",
           width_mm, height_mm, mode->width * 25.4f / width_mm, mode->width, mode->height);
    printf("Monitor work area: %i x %i starting at %i, %i\n",
            workarea_width, workarea_height, workarea_x, workarea_y);

    printf("Modes:\n");

    for (i = 0;  i < count;  i++)
    {
        printf("%3u: %s", (unsigned int) i, format_mode(modes + i));

        if (memcmp(mode, modes + i, sizeof(RBLXWINvidmode)) == 0)
            printf(" (current mode)");

        putchar('\n');
    }
}

static void test_modes(RBLXWINmonitor* monitor)
{
    int i, count;
    RBLXWINwindow* window;
    const RBLXWINvidmode* modes = rblxwinGetVideoModes(monitor, &count);

    for (i = 0;  i < count;  i++)
    {
        const RBLXWINvidmode* mode = modes + i;
        RBLXWINvidmode current;

        rblxwinWindowHint(RBLXWIN_RED_BITS, mode->redBits);
        rblxwinWindowHint(RBLXWIN_GREEN_BITS, mode->greenBits);
        rblxwinWindowHint(RBLXWIN_BLUE_BITS, mode->blueBits);
        rblxwinWindowHint(RBLXWIN_REFRESH_RATE, mode->refreshRate);

        printf("Testing mode %u on monitor %s: %s\n",
               (unsigned int) i,
               rblxwinGetMonitorName(monitor),
               format_mode(mode));

        window = rblxwinCreateWindow(mode->width, mode->height,
                                  "Video Mode Test",
                                  rblxwinGetPrimaryMonitor(),
                                  NULL);
        if (!window)
        {
            printf("Failed to enter mode %u: %s\n",
                   (unsigned int) i,
                   format_mode(mode));
            continue;
        }

        rblxwinSetFramebufferSizeCallback(window, framebuffer_size_callback);
        rblxwinSetKeyCallback(window, key_callback);

        rblxwinMakeContextCurrent(window);
        rblxglLoadGL(rblxwinGetProcAddress);
        rblxwinSwapInterval(1);

        rblxwinSetTime(0.0);

        while (rblxwinGetTime() < 5.0)
        {
            glClear(GL_COLOR_BUFFER_BIT);
            rblxwinSwapBuffers(window);
            rblxwinPollEvents();

            if (rblxwinWindowShouldClose(window))
            {
                printf("User terminated program\n");

                rblxwinTerminate();
                exit(EXIT_SUCCESS);
            }
        }

        glGetIntegerv(GL_RED_BITS, &current.redBits);
        glGetIntegerv(GL_GREEN_BITS, &current.greenBits);
        glGetIntegerv(GL_BLUE_BITS, &current.blueBits);

        rblxwinGetWindowSize(window, &current.width, &current.height);

        if (current.redBits != mode->redBits ||
            current.greenBits != mode->greenBits ||
            current.blueBits != mode->blueBits)
        {
            printf("*** Color bit mismatch: (%i %i %i) instead of (%i %i %i)\n",
                   current.redBits, current.greenBits, current.blueBits,
                   mode->redBits, mode->greenBits, mode->blueBits);
        }

        if (current.width != mode->width || current.height != mode->height)
        {
            printf("*** Size mismatch: %ix%i instead of %ix%i\n",
                   current.width, current.height,
                   mode->width, mode->height);
        }

        printf("Closing window\n");

        rblxwinDestroyWindow(window);
        window = NULL;

        rblxwinPollEvents();
    }
}

int main(int argc, char** argv)
{
    int ch, i, count, mode = LIST_MODE;
    RBLXWINmonitor** monitors;

    while ((ch = getopt(argc, argv, "th")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 't':
                mode = TEST_MODE;
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    rblxwinSetErrorCallback(error_callback);

    if (!rblxwinInit())
        exit(EXIT_FAILURE);

    monitors = rblxwinGetMonitors(&count);

    for (i = 0;  i < count;  i++)
    {
        if (mode == LIST_MODE)
            list_modes(monitors[i]);
        else if (mode == TEST_MODE)
            test_modes(monitors[i]);
    }

    rblxwinTerminate();
    exit(EXIT_SUCCESS);
}

