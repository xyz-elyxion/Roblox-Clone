//========================================================================
// Clipboard test program
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
// This program is used to test the clipboard functionality.
//
//========================================================================

#include <rblxgl/gl.h>
#define RBLXWIN_INCLUDE_NONE
#include <RBLXWIN/rblxwin.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

#if defined(__APPLE__)
 #define MODIFIER RBLXWIN_MOD_SUPER
#else
 #define MODIFIER RBLXWIN_MOD_CONTROL
#endif

static void usage(void)
{
    printf("Usage: clipboard [-h]\n");
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
        case RBLXWIN_KEY_ESCAPE:
            rblxwinSetWindowShouldClose(window, RBLXWIN_TRUE);
            break;

        case RBLXWIN_KEY_V:
            if (mods == MODIFIER)
            {
                const char* string;

                string = rblxwinGetClipboardString(NULL);
                if (string)
                    printf("Clipboard contains \"%s\"\n", string);
                else
                    printf("Clipboard does not contain a string\n");
            }
            break;

        case RBLXWIN_KEY_C:
            if (mods == MODIFIER)
            {
                const char* string = "Hello RBLXWIN World!";
                rblxwinSetClipboardString(NULL, string);
                printf("Setting clipboard to \"%s\"\n", string);
            }
            break;
    }
}

int main(int argc, char** argv)
{
    int ch;
    RBLXWINwindow* window;

    while ((ch = getopt(argc, argv, "h")) != -1)
    {
        switch (ch)
        {
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
    {
        fprintf(stderr, "Failed to initialize RBLXWIN\n");
        exit(EXIT_FAILURE);
    }

    window = rblxwinCreateWindow(200, 200, "Clipboard Test", NULL, NULL);
    if (!window)
    {
        rblxwinTerminate();

        fprintf(stderr, "Failed to open RBLXWIN window\n");
        exit(EXIT_FAILURE);
    }

    rblxwinMakeContextCurrent(window);
    rblxglLoadGL(rblxwinGetProcAddress);
    rblxwinSwapInterval(1);

    rblxwinSetKeyCallback(window, key_callback);

    glClearColor(0.5f, 0.5f, 0.5f, 0);

    while (!rblxwinWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        rblxwinSwapBuffers(window);
        rblxwinWaitEvents();
    }

    rblxwinTerminate();
    exit(EXIT_SUCCESS);
}

