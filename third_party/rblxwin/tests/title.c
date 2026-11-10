//========================================================================
// UTF-8 window title test
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
// This test sets a UTF-8 window title
//
//========================================================================

#include <rblxgl/gl.h>
#define RBLXWIN_INCLUDE_NONE
#include <RBLXWIN/rblxwin.h>

#include <stdio.h>
#include <stdlib.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main(void)
{
    RBLXWINwindow* window;

    rblxwinSetErrorCallback(error_callback);

    if (!rblxwinInit())
        exit(EXIT_FAILURE);

    window = rblxwinCreateWindow(400, 400, "English 日本語 русский язык 官話", NULL, NULL);
    if (!window)
    {
        rblxwinTerminate();
        exit(EXIT_FAILURE);
    }

    rblxwinMakeContextCurrent(window);
    rblxglLoadGL(rblxwinGetProcAddress);
    rblxwinSwapInterval(1);

    while (!rblxwinWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        rblxwinSwapBuffers(window);
        rblxwinWaitEvents();
    }

    rblxwinTerminate();
    exit(EXIT_SUCCESS);
}

