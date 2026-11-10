//========================================================================
// Window re-opener (open/close stress test)
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
// This test came about as the result of bug #1262773
//
// It closes and re-opens the RBLXWIN window every five seconds, alternating
// between windowed and full screen mode
//
// It also times and logs opening and closing actions and attempts to separate
// user initiated window closing from its own
//
//========================================================================

#include <rblxgl/gl.h>
#define RBLXWIN_INCLUDE_NONE
#include <RBLXWIN/rblxwin.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "linmath.h"

static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec2 vPos;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"}\n";

static const char* fragment_shader_text =
"#version 110\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(1.0);\n"
"}\n";

static const vec2 vertices[4] =
{
    { -0.5f, -0.5f },
    {  0.5f, -0.5f },
    {  0.5f,  0.5f },
    { -0.5f,  0.5f }
};

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void window_close_callback(RBLXWINwindow* window)
{
    printf("Close callback triggered\n");
}

static void key_callback(RBLXWINwindow* window, int key, int scancode, int action, int mods)
{
    if (action != RBLXWIN_PRESS)
        return;

    switch (key)
    {
        case RBLXWIN_KEY_Q:
        case RBLXWIN_KEY_ESCAPE:
            rblxwinSetWindowShouldClose(window, RBLXWIN_TRUE);
            break;
    }
}

static void close_window(RBLXWINwindow* window)
{
    double base = rblxwinGetTime();
    rblxwinDestroyWindow(window);
    printf("Closing window took %0.3f seconds\n", rblxwinGetTime() - base);
}

int main(int argc, char** argv)
{
    int count = 0;
    double base;
    RBLXWINwindow* window;

    srand((unsigned int) time(NULL));

    rblxwinSetErrorCallback(error_callback);

    if (!rblxwinInit())
        exit(EXIT_FAILURE);

    rblxwinWindowHint(RBLXWIN_CONTEXT_VERSION_MAJOR, 2);
    rblxwinWindowHint(RBLXWIN_CONTEXT_VERSION_MINOR, 0);

    for (;;)
    {
        int width, height;
        RBLXWINmonitor* monitor = NULL;
        GLuint vertex_shader, fragment_shader, program, vertex_buffer;
        GLint mvp_location, vpos_location;

        if (count & 1)
        {
            int monitorCount;
            RBLXWINmonitor** monitors = rblxwinGetMonitors(&monitorCount);
            monitor = monitors[rand() % monitorCount];
        }

        if (monitor)
        {
            const RBLXWINvidmode* mode = rblxwinGetVideoMode(monitor);
            width = mode->width;
            height = mode->height;
        }
        else
        {
            width = 640;
            height = 480;
        }

        base = rblxwinGetTime();

        window = rblxwinCreateWindow(width, height, "Window Re-opener", monitor, NULL);
        if (!window)
        {
            rblxwinTerminate();
            exit(EXIT_FAILURE);
        }

        if (monitor)
        {
            printf("Opening full screen window on monitor %s took %0.3f seconds\n",
                   rblxwinGetMonitorName(monitor),
                   rblxwinGetTime() - base);
        }
        else
        {
            printf("Opening regular window took %0.3f seconds\n",
                   rblxwinGetTime() - base);
        }

        rblxwinSetWindowCloseCallback(window, window_close_callback);
        rblxwinSetKeyCallback(window, key_callback);

        rblxwinMakeContextCurrent(window);
        rblxglLoadGL(rblxwinGetProcAddress);
        rblxwinSwapInterval(1);

        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
        glCompileShader(vertex_shader);

        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
        glCompileShader(fragment_shader);

        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        mvp_location = glGetUniformLocation(program, "MVP");
        vpos_location = glGetAttribLocation(program, "vPos");

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                              sizeof(vertices[0]), (void*) 0);

        rblxwinSetTime(0.0);

        while (rblxwinGetTime() < 5.0)
        {
            float ratio;
            int width, height;
            mat4x4 m, p, mvp;

            rblxwinGetFramebufferSize(window, &width, &height);
            ratio = width / (float) height;

            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT);

            mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 0.f, 1.f);

            mat4x4_identity(m);
            mat4x4_rotate_Z(m, m, (float) rblxwinGetTime());
            mat4x4_mul(mvp, p, m);

            glUseProgram(program);
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            rblxwinSwapBuffers(window);
            rblxwinPollEvents();

            if (rblxwinWindowShouldClose(window))
            {
                close_window(window);
                printf("User closed window\n");

                rblxwinTerminate();
                exit(EXIT_SUCCESS);
            }
        }

        printf("Closing window\n");
        close_window(window);

        count++;
    }

    rblxwinTerminate();
}

