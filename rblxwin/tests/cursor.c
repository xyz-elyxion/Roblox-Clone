//========================================================================
// Cursor & input mode tests
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
// This test provides an interface to the cursor image and cursor mode
// parts of the API.
//
// Custom cursor image generation by urraka.
//
//========================================================================

#include <rblxgl/gl.h>
#define RBLXWIN_INCLUDE_NONE
#include <RBLXWIN/rblxwin.h>

#if defined(_MSC_VER)
 // Make MS math.h define M_PI
 #define _USE_MATH_DEFINES
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "linmath.h"

#define CURSOR_FRAME_COUNT 60

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

static double cursor_x;
static double cursor_y;
static int swap_interval = 1;
static int wait_events = RBLXWIN_TRUE;
static int animate_cursor = RBLXWIN_FALSE;
static int track_cursor = RBLXWIN_FALSE;
static RBLXWINcursor* standard_cursors[6];
static RBLXWINcursor* tracking_cursor = NULL;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static float star(int x, int y, float t)
{
    const float c = 64 / 2.f;

    const float i = (0.25f * (float) sin(2.f * M_PI * t) + 0.75f);
    const float k = 64 * 0.046875f * i;

    const float dist = (float) sqrt((x - c) * (x - c) + (y - c) * (y - c));

    const float salpha = 1.f - dist / c;
    const float xalpha = (float) x == c ? c : k / (float) fabs(x - c);
    const float yalpha = (float) y == c ? c : k / (float) fabs(y - c);

    return (float) fmax(0.f, fmin(1.f, i * salpha * 0.2f + salpha * xalpha * yalpha));
}

static RBLXWINcursor* create_cursor_frame(float t)
{
    int i = 0, x, y;
    unsigned char buffer[64 * 64 * 4];
    const RBLXWINimage image = { 64, 64, buffer };

    for (y = 0;  y < image.width;  y++)
    {
        for (x = 0;  x < image.height;  x++)
        {
            buffer[i++] = 255;
            buffer[i++] = 255;
            buffer[i++] = 255;
            buffer[i++] = (unsigned char) (255 * star(x, y, t));
        }
    }

    return rblxwinCreateCursor(&image, image.width / 2, image.height / 2);
}

static RBLXWINcursor* create_tracking_cursor(void)
{
    int i = 0, x, y;
    unsigned char buffer[32 * 32 * 4];
    const RBLXWINimage image = { 32, 32, buffer };

    for (y = 0;  y < image.width;  y++)
    {
        for (x = 0;  x < image.height;  x++)
        {
            if (x == 7 || y == 7)
            {
                buffer[i++] = 255;
                buffer[i++] = 0;
                buffer[i++] = 0;
                buffer[i++] = 255;
            }
            else
            {
                buffer[i++] = 0;
                buffer[i++] = 0;
                buffer[i++] = 0;
                buffer[i++] = 0;
            }
        }
    }

    return rblxwinCreateCursor(&image, 7, 7);
}

static void cursor_position_callback(RBLXWINwindow* window, double x, double y)
{
    printf("%0.3f: Cursor position: %f %f (%+f %+f)\n",
           rblxwinGetTime(),
           x, y, x - cursor_x, y - cursor_y);

    cursor_x = x;
    cursor_y = y;
}

static void key_callback(RBLXWINwindow* window, int key, int scancode, int action, int mods)
{
    if (action != RBLXWIN_PRESS)
        return;

    switch (key)
    {
        case RBLXWIN_KEY_A:
        {
            animate_cursor = !animate_cursor;
            if (!animate_cursor)
                rblxwinSetCursor(window, NULL);

            break;
        }

        case RBLXWIN_KEY_ESCAPE:
        {
            if (rblxwinGetInputMode(window, RBLXWIN_CURSOR) != RBLXWIN_CURSOR_DISABLED)
            {
                rblxwinSetWindowShouldClose(window, RBLXWIN_TRUE);
                break;
            }

            /* FALLTHROUGH */
        }

        case RBLXWIN_KEY_N:
            rblxwinSetInputMode(window, RBLXWIN_CURSOR, RBLXWIN_CURSOR_NORMAL);
            rblxwinGetCursorPos(window, &cursor_x, &cursor_y);
            printf("(( cursor is normal ))\n");
            break;

        case RBLXWIN_KEY_D:
            rblxwinSetInputMode(window, RBLXWIN_CURSOR, RBLXWIN_CURSOR_DISABLED);
            printf("(( cursor is disabled ))\n");
            break;

        case RBLXWIN_KEY_H:
            rblxwinSetInputMode(window, RBLXWIN_CURSOR, RBLXWIN_CURSOR_HIDDEN);
            printf("(( cursor is hidden ))\n");
            break;

        case RBLXWIN_KEY_R:
            if (!rblxwinRawMouseMotionSupported())
                break;

            if (rblxwinGetInputMode(window, RBLXWIN_RAW_MOUSE_MOTION))
            {
                rblxwinSetInputMode(window, RBLXWIN_RAW_MOUSE_MOTION, RBLXWIN_FALSE);
                printf("(( raw input is disabled ))\n");
            }
            else
            {
                rblxwinSetInputMode(window, RBLXWIN_RAW_MOUSE_MOTION, RBLXWIN_TRUE);
                printf("(( raw input is enabled ))\n");
            }
            break;

        case RBLXWIN_KEY_SPACE:
            swap_interval = 1 - swap_interval;
            printf("(( swap interval: %i ))\n", swap_interval);
            rblxwinSwapInterval(swap_interval);
            break;

        case RBLXWIN_KEY_W:
            wait_events = !wait_events;
            printf("(( %sing for events ))\n", wait_events ? "wait" : "poll");
            break;

        case RBLXWIN_KEY_T:
            track_cursor = !track_cursor;
            if (track_cursor)
                rblxwinSetCursor(window, tracking_cursor);
            else
                rblxwinSetCursor(window, NULL);

            break;

        case RBLXWIN_KEY_P:
        {
            double x, y;
            rblxwinGetCursorPos(window, &x, &y);

            printf("Query before set: %f %f (%+f %+f)\n",
                   x, y, x - cursor_x, y - cursor_y);
            cursor_x = x;
            cursor_y = y;

            rblxwinSetCursorPos(window, cursor_x, cursor_y);
            rblxwinGetCursorPos(window, &x, &y);

            printf("Query after set: %f %f (%+f %+f)\n",
                   x, y, x - cursor_x, y - cursor_y);
            cursor_x = x;
            cursor_y = y;
            break;
        }

        case RBLXWIN_KEY_UP:
            rblxwinSetCursorPos(window, 0, 0);
            rblxwinGetCursorPos(window, &cursor_x, &cursor_y);
            break;

        case RBLXWIN_KEY_DOWN:
        {
            int width, height;
            rblxwinGetWindowSize(window, &width, &height);
            rblxwinSetCursorPos(window, width - 1, height - 1);
            rblxwinGetCursorPos(window, &cursor_x, &cursor_y);
            break;
        }

        case RBLXWIN_KEY_0:
            rblxwinSetCursor(window, NULL);
            break;

        case RBLXWIN_KEY_1:
            rblxwinSetCursor(window, standard_cursors[0]);
            break;

        case RBLXWIN_KEY_2:
            rblxwinSetCursor(window, standard_cursors[1]);
            break;

        case RBLXWIN_KEY_3:
            rblxwinSetCursor(window, standard_cursors[2]);
            break;

        case RBLXWIN_KEY_4:
            rblxwinSetCursor(window, standard_cursors[3]);
            break;

        case RBLXWIN_KEY_5:
            rblxwinSetCursor(window, standard_cursors[4]);
            break;

        case RBLXWIN_KEY_6:
            rblxwinSetCursor(window, standard_cursors[5]);
            break;

        case RBLXWIN_KEY_F11:
        case RBLXWIN_KEY_ENTER:
        {
            static int x, y, width, height;

            if (mods != RBLXWIN_MOD_ALT)
                return;

            if (rblxwinGetWindowMonitor(window))
                rblxwinSetWindowMonitor(window, NULL, x, y, width, height, 0);
            else
            {
                RBLXWINmonitor* monitor = rblxwinGetPrimaryMonitor();
                const RBLXWINvidmode* mode = rblxwinGetVideoMode(monitor);
                rblxwinGetWindowPos(window, &x, &y);
                rblxwinGetWindowSize(window, &width, &height);
                rblxwinSetWindowMonitor(window, monitor,
                                     0, 0, mode->width, mode->height,
                                     mode->refreshRate);
            }

            rblxwinGetCursorPos(window, &cursor_x, &cursor_y);
            break;
        }
    }
}

int main(void)
{
    int i;
    RBLXWINwindow* window;
    RBLXWINcursor* star_cursors[CURSOR_FRAME_COUNT];
    RBLXWINcursor* current_frame = NULL;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location;

    rblxwinSetErrorCallback(error_callback);

    if (!rblxwinInit())
        exit(EXIT_FAILURE);

    tracking_cursor = create_tracking_cursor();
    if (!tracking_cursor)
    {
        rblxwinTerminate();
        exit(EXIT_FAILURE);
    }

    for (i = 0;  i < CURSOR_FRAME_COUNT;  i++)
    {
        star_cursors[i] = create_cursor_frame(i / (float) CURSOR_FRAME_COUNT);
        if (!star_cursors[i])
        {
            rblxwinTerminate();
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0;  i < sizeof(standard_cursors) / sizeof(standard_cursors[0]);  i++)
    {
        const int shapes[] = {
            RBLXWIN_ARROW_CURSOR,
            RBLXWIN_IBEAM_CURSOR,
            RBLXWIN_CROSSHAIR_CURSOR,
            RBLXWIN_HAND_CURSOR,
            RBLXWIN_HRESIZE_CURSOR,
            RBLXWIN_VRESIZE_CURSOR
        };

        standard_cursors[i] = rblxwinCreateStandardCursor(shapes[i]);
        if (!standard_cursors[i])
        {
            rblxwinTerminate();
            exit(EXIT_FAILURE);
        }
    }

    rblxwinWindowHint(RBLXWIN_CONTEXT_VERSION_MAJOR, 2);
    rblxwinWindowHint(RBLXWIN_CONTEXT_VERSION_MINOR, 0);

    window = rblxwinCreateWindow(640, 480, "Cursor Test", NULL, NULL);
    if (!window)
    {
        rblxwinTerminate();
        exit(EXIT_FAILURE);
    }

    rblxwinMakeContextCurrent(window);
    rblxglLoadGL(rblxwinGetProcAddress);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

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

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vec2), (void*) 0);
    glUseProgram(program);

    rblxwinGetCursorPos(window, &cursor_x, &cursor_y);
    printf("Cursor position: %f %f\n", cursor_x, cursor_y);

    rblxwinSetCursorPosCallback(window, cursor_position_callback);
    rblxwinSetKeyCallback(window, key_callback);

    while (!rblxwinWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        if (track_cursor)
        {
            int wnd_width, wnd_height, fb_width, fb_height;
            float scale;
            vec2 vertices[4];
            mat4x4 mvp;

            rblxwinGetWindowSize(window, &wnd_width, &wnd_height);
            rblxwinGetFramebufferSize(window, &fb_width, &fb_height);

            glViewport(0, 0, fb_width, fb_height);

            scale = (float) fb_width / (float) wnd_width;
            vertices[0][0] = 0.5f;
            vertices[0][1] = (float) (fb_height - floor(cursor_y * scale) - 1.f + 0.5f);
            vertices[1][0] = (float) fb_width + 0.5f;
            vertices[1][1] = (float) (fb_height - floor(cursor_y * scale) - 1.f + 0.5f);
            vertices[2][0] = (float) floor(cursor_x * scale) + 0.5f;
            vertices[2][1] = 0.5f;
            vertices[3][0] = (float) floor(cursor_x * scale) + 0.5f;
            vertices[3][1] = (float) fb_height + 0.5f;

            glBufferData(GL_ARRAY_BUFFER,
                         sizeof(vertices),
                         vertices,
                         GL_STREAM_DRAW);

            mat4x4_ortho(mvp, 0.f, (float) fb_width, 0.f, (float) fb_height, 0.f, 1.f);
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);

            glDrawArrays(GL_LINES, 0, 4);
        }

        rblxwinSwapBuffers(window);

        if (animate_cursor)
        {
            const int i = (int) (rblxwinGetTime() * 30.0) % CURSOR_FRAME_COUNT;
            if (current_frame != star_cursors[i])
            {
                rblxwinSetCursor(window, star_cursors[i]);
                current_frame = star_cursors[i];
            }
        }
        else
            current_frame = NULL;

        if (wait_events)
        {
            if (animate_cursor)
                rblxwinWaitEventsTimeout(1.0 / 30.0);
            else
                rblxwinWaitEvents();
        }
        else
            rblxwinPollEvents();

        // Workaround for an issue with msvcrt and mintty
        fflush(stdout);
    }

    rblxwinDestroyWindow(window);

    for (i = 0;  i < CURSOR_FRAME_COUNT;  i++)
        rblxwinDestroyCursor(star_cursors[i]);

    for (i = 0;  i < sizeof(standard_cursors) / sizeof(standard_cursors[0]);  i++)
        rblxwinDestroyCursor(standard_cursors[i]);

    rblxwinTerminate();
    exit(EXIT_SUCCESS);
}

