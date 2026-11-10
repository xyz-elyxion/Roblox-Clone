//========================================================================
// Multi-threading test
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
// This test is intended to verify whether the OpenGL context part of
// the RBLXWIN API is able to be used from multiple threads
//
//========================================================================

#include "tinycthread.h"

#include <rblxgl/gl.h>
#define RBLXWIN_INCLUDE_NONE
#include <RBLXWIN/rblxwin.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct
{
    RBLXWINwindow* window;
    const char* title;
    float r, g, b;
    thrd_t id;
} Thread;

static volatile int running = RBLXWIN_TRUE;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(RBLXWINwindow* window, int key, int scancode, int action, int mods)
{
    if (key == RBLXWIN_KEY_ESCAPE && action == RBLXWIN_PRESS)
        rblxwinSetWindowShouldClose(window, RBLXWIN_TRUE);
}

static int thread_main(void* data)
{
    const Thread* thread = data;

    rblxwinMakeContextCurrent(thread->window);
    rblxwinSwapInterval(1);

    while (running)
    {
        const float v = (float) fabs(sin(rblxwinGetTime() * 2.f));
        glClearColor(thread->r * v, thread->g * v, thread->b * v, 0.f);

        glClear(GL_COLOR_BUFFER_BIT);
        rblxwinSwapBuffers(thread->window);
    }

    rblxwinMakeContextCurrent(NULL);
    return 0;
}

int main(void)
{
    int i, result;
    Thread threads[] =
    {
        { NULL, "Red", 1.f, 0.f, 0.f, 0 },
        { NULL, "Green", 0.f, 1.f, 0.f, 0 },
        { NULL, "Blue", 0.f, 0.f, 1.f, 0 }
    };
    const int count = sizeof(threads) / sizeof(Thread);

    rblxwinSetErrorCallback(error_callback);

    if (!rblxwinInit())
        exit(EXIT_FAILURE);

    rblxwinWindowHint(RBLXWIN_VISIBLE, RBLXWIN_FALSE);

    for (i = 0;  i < count;  i++)
    {
        threads[i].window = rblxwinCreateWindow(200, 200,
                                             threads[i].title,
                                             NULL, NULL);
        if (!threads[i].window)
        {
            rblxwinTerminate();
            exit(EXIT_FAILURE);
        }

        rblxwinSetKeyCallback(threads[i].window, key_callback);

        rblxwinSetWindowPos(threads[i].window, 200 + 250 * i, 200);
        rblxwinShowWindow(threads[i].window);
    }

    rblxwinMakeContextCurrent(threads[0].window);
    rblxglLoadGL(rblxwinGetProcAddress);
    rblxwinMakeContextCurrent(NULL);

    for (i = 0;  i < count;  i++)
    {
        if (thrd_create(&threads[i].id, thread_main, threads + i) !=
            thrd_success)
        {
            fprintf(stderr, "Failed to create secondary thread\n");

            rblxwinTerminate();
            exit(EXIT_FAILURE);
        }
    }

    while (running)
    {
        rblxwinWaitEvents();

        for (i = 0;  i < count;  i++)
        {
            if (rblxwinWindowShouldClose(threads[i].window))
                running = RBLXWIN_FALSE;
        }
    }

    for (i = 0;  i < count;  i++)
        rblxwinHideWindow(threads[i].window);

    for (i = 0;  i < count;  i++)
        thrd_join(threads[i].id, &result);

    exit(EXIT_SUCCESS);
}

