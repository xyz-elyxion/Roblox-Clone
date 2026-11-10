//========================================================================
// Simple multi-window test
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
// This test creates four windows and clears each in a different color
//
//========================================================================

#include <rblxgl/gl.h>
#define RBLXWIN_INCLUDE_NONE
#include <RBLXWIN/rblxwin.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

static const char* titles[] =
{
    "Red",
    "Green",
    "Blue",
    "Yellow"
};

static const struct
{
    float r, g, b;
} colors[] =
{
    { 0.95f, 0.32f, 0.11f },
    { 0.50f, 0.80f, 0.16f },
    {   0.f, 0.68f, 0.94f },
    { 0.98f, 0.74f, 0.04f }
};

static void usage(void)
{
    printf("Usage: windows [-h] [-b] [-f] \n");
    printf("Options:\n");
    printf("  -b create decorated windows\n");
    printf("  -f set focus on show off for all but first window\n");
    printf("  -h show this help\n");
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(RBLXWINwindow* window, int key, int scancode, int action, int mods)
{
    if (action != RBLXWIN_PRESS)
        return;

    switch (key)
    {
        case RBLXWIN_KEY_SPACE:
        {
            int xpos, ypos;
            rblxwinGetWindowPos(window, &xpos, &ypos);
            rblxwinSetWindowPos(window, xpos, ypos);
            break;
        }

        case RBLXWIN_KEY_ESCAPE:
            rblxwinSetWindowShouldClose(window, RBLXWIN_TRUE);
            break;
    }
}

int main(int argc, char** argv)
{
    int i, ch;
    int decorated = RBLXWIN_FALSE;
    int focusOnShow = RBLXWIN_TRUE;
    int running = RBLXWIN_TRUE;
    RBLXWINwindow* windows[4];

    while ((ch = getopt(argc, argv, "bfh")) != -1)
    {
        switch (ch)
        {
            case 'b':
                decorated = RBLXWIN_TRUE;
                break;
            case 'f':
                focusOnShow = RBLXWIN_FALSE;
                break;
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    rblxwinSetErrorCallback(error_callback);

    if (!rblxwinInit())
        exit(EXIT_FAILURE);

    rblxwinWindowHint(RBLXWIN_DECORATED, decorated);
    rblxwinWindowHint(RBLXWIN_VISIBLE, RBLXWIN_FALSE);

    for (i = 0;  i < 4;  i++)
    {
        int left, top, right, bottom;
        if (i)
            rblxwinWindowHint(RBLXWIN_FOCUS_ON_SHOW, focusOnShow);

        windows[i] = rblxwinCreateWindow(200, 200, titles[i], NULL, NULL);
        if (!windows[i])
        {
            rblxwinTerminate();
            exit(EXIT_FAILURE);
        }

        rblxwinSetKeyCallback(windows[i], key_callback);

        rblxwinMakeContextCurrent(windows[i]);
        rblxglLoadGL(rblxwinGetProcAddress);
        glClearColor(colors[i].r, colors[i].g, colors[i].b, 1.f);

        rblxwinGetWindowFrameSize(windows[i], &left, &top, &right, &bottom);
        rblxwinSetWindowPos(windows[i],
                         100 + (i & 1) * (200 + left + right),
                         100 + (i >> 1) * (200 + top + bottom));
    }

    for (i = 0;  i < 4;  i++)
        rblxwinShowWindow(windows[i]);

    while (running)
    {
        for (i = 0;  i < 4;  i++)
        {
            rblxwinMakeContextCurrent(windows[i]);
            glClear(GL_COLOR_BUFFER_BIT);
            rblxwinSwapBuffers(windows[i]);

            if (rblxwinWindowShouldClose(windows[i]))
                running = RBLXWIN_FALSE;
        }

        rblxwinWaitEvents();
    }

    rblxwinTerminate();
    exit(EXIT_SUCCESS);
}

