//========================================================================
// Event wait timeout test
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
// This test is intended to verify that waiting for events with timeout works
//
//========================================================================

#include <rblxgl/gl.h>
#define RBLXWIN_INCLUDE_NONE
#include <RBLXWIN/rblxwin.h>

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(RBLXWINwindow* window, int key, int scancode, int action, int mods)
{
    if (key == RBLXWIN_KEY_ESCAPE && action == RBLXWIN_PRESS)
        rblxwinSetWindowShouldClose(window, RBLXWIN_TRUE);
}

static float nrand(void)
{
    return (float) rand() / (float) RAND_MAX;
}

int main(void)
{
    RBLXWINwindow* window;

    srand((unsigned int) time(NULL));

    rblxwinSetErrorCallback(error_callback);

    if (!rblxwinInit())
        exit(EXIT_FAILURE);

    window = rblxwinCreateWindow(640, 480, "Event Wait Timeout Test", NULL, NULL);
    if (!window)
    {
        rblxwinTerminate();
        exit(EXIT_FAILURE);
    }

    rblxwinMakeContextCurrent(window);
    rblxglLoadGL(rblxwinGetProcAddress);
    rblxwinSetKeyCallback(window, key_callback);

    while (!rblxwinWindowShouldClose(window))
    {
        int width, height;
        float r = nrand(), g = nrand(), b = nrand();
        float l = (float) sqrt(r * r + g * g + b * b);

        rblxwinGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClearColor(r / l, g / l, b / l, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        rblxwinSwapBuffers(window);

        rblxwinWaitEventsTimeout(1.0);
    }

    rblxwinDestroyWindow(window);

    rblxwinTerminate();
    exit(EXIT_SUCCESS);
}

