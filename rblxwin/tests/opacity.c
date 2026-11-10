//========================================================================
// Window opacity test program
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

#include <rblxgl/gl.h>
#define RBLXWIN_INCLUDE_NONE
#include <RBLXWIN/rblxwin.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_VARARGS
#include <nuklear.h>

#define NK_RBLXWIN_GL2_IMPLEMENTATION
#include <nuklear_rblxwin_gl2.h>

#include <stdio.h>
#include <stdlib.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char** argv)
{
    RBLXWINwindow* window;
    struct nk_context* nk;
    struct nk_font_atlas* atlas;

    rblxwinSetErrorCallback(error_callback);

    if (!rblxwinInit())
        exit(EXIT_FAILURE);

    rblxwinWindowHint(RBLXWIN_SCALE_TO_MONITOR, RBLXWIN_TRUE);

    window = rblxwinCreateWindow(400, 400, "Opacity", NULL, NULL);
    if (!window)
    {
        rblxwinTerminate();
        exit(EXIT_FAILURE);
    }

    rblxwinMakeContextCurrent(window);
    rblxglLoadGL(rblxwinGetProcAddress);
    rblxwinSwapInterval(1);

    nk = nk_rblxwin_init(window, NK_RBLXWIN3_INSTALL_CALLBACKS);
    nk_rblxwin_font_stash_begin(&atlas);
    nk_rblxwin_font_stash_end();

    while (!rblxwinWindowShouldClose(window))
    {
        int width, height;
        struct nk_rect area;

        rblxwinGetWindowSize(window, &width, &height);
        area = nk_rect(0.f, 0.f, (float) width, (float) height);

        glClear(GL_COLOR_BUFFER_BIT);
        nk_rblxwin_new_frame();
        if (nk_begin(nk, "", area, 0))
        {
            float opacity = rblxwinGetWindowOpacity(window);
            nk_layout_row_dynamic(nk, 30, 2);
            if (nk_slider_float(nk, 0.f, &opacity, 1.f, 0.001f))
                rblxwinSetWindowOpacity(window, opacity);
            nk_labelf(nk, NK_TEXT_LEFT, "%0.3f", opacity);
        }

        nk_end(nk);
        nk_rblxwin_render(NK_ANTI_ALIASING_ON);

        rblxwinSwapBuffers(window);
        rblxwinWaitEventsTimeout(1.0);
    }

    nk_rblxwin_shutdown();
    rblxwinTerminate();
    exit(EXIT_SUCCESS);
}

