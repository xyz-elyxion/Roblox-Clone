//========================================================================
// LINEARWIN 3.3 - www.linearwin.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2018 Camilla Löwy <elmindreda@linearwin.org>
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
// Please use C89 style variable declarations in this file because VS 2010
//========================================================================

#include "internal.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>


// NOTE: The global variables below comprise all mutable global data in LINEARWIN
//       Any other mutable global variable is a bug

// This contains all mutable state shared between compilation units of LINEARWIN
//
_LINEARWINlibrary _linearwin = { LINEARWIN_FALSE };

// These are outside of _linearwin so they can be used before initialization and
// after termination without special handling when _linearwin is cleared to zero
//
static _LINEARWINerror _linearwinMainThreadError;
static LINEARWINerrorfun _linearwinErrorCallback;
static _LINEARWINinitconfig _linearwinInitHints =
{
    LINEARWIN_TRUE,      // hat buttons
    {
        LINEARWIN_TRUE,  // macOS menu bar
        LINEARWIN_TRUE   // macOS bundle chdir
    }
};

// Terminate the library
//
static void terminate(void)
{
    int i;

    memset(&_linearwin.callbacks, 0, sizeof(_linearwin.callbacks));

    while (_linearwin.windowListHead)
        linearwinDestroyWindow((LINEARWINwindow*) _linearwin.windowListHead);

    while (_linearwin.cursorListHead)
        linearwinDestroyCursor((LINEARWINcursor*) _linearwin.cursorListHead);

    for (i = 0;  i < _linearwin.monitorCount;  i++)
    {
        _LINEARWINmonitor* monitor = _linearwin.monitors[i];
        if (monitor->originalRamp.size)
            _linearwinPlatformSetGammaRamp(monitor, &monitor->originalRamp);
        _linearwinFreeMonitor(monitor);
    }

    free(_linearwin.monitors);
    _linearwin.monitors = NULL;
    _linearwin.monitorCount = 0;

    free(_linearwin.mappings);
    _linearwin.mappings = NULL;
    _linearwin.mappingCount = 0;

    _linearwinTerminateVulkan();
    _linearwinPlatformTerminate();

    _linearwin.initialized = LINEARWIN_FALSE;

    while (_linearwin.errorListHead)
    {
        _LINEARWINerror* error = _linearwin.errorListHead;
        _linearwin.errorListHead = error->next;
        free(error);
    }

    _linearwinPlatformDestroyTls(&_linearwin.contextSlot);
    _linearwinPlatformDestroyTls(&_linearwin.errorSlot);
    _linearwinPlatformDestroyMutex(&_linearwin.errorLock);

    memset(&_linearwin, 0, sizeof(_linearwin));
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Encode a Unicode code point to a UTF-8 stream
// Based on cutef8 by Jeff Bezanson (Public Domain)
//
size_t _linearwinEncodeUTF8(char* s, uint32_t codepoint)
{
    size_t count = 0;

    if (codepoint < 0x80)
        s[count++] = (char) codepoint;
    else if (codepoint < 0x800)
    {
        s[count++] = (codepoint >> 6) | 0xc0;
        s[count++] = (codepoint & 0x3f) | 0x80;
    }
    else if (codepoint < 0x10000)
    {
        s[count++] = (codepoint >> 12) | 0xe0;
        s[count++] = ((codepoint >> 6) & 0x3f) | 0x80;
        s[count++] = (codepoint & 0x3f) | 0x80;
    }
    else if (codepoint < 0x110000)
    {
        s[count++] = (codepoint >> 18) | 0xf0;
        s[count++] = ((codepoint >> 12) & 0x3f) | 0x80;
        s[count++] = ((codepoint >> 6) & 0x3f) | 0x80;
        s[count++] = (codepoint & 0x3f) | 0x80;
    }

    return count;
}

// Splits and translates a text/uri-list into separate file paths
// NOTE: This function destroys the provided string
//
char** _linearwinParseUriList(char* text, int* count)
{
    const char* prefix = "file://";
    char** paths = NULL;
    char* line;

    *count = 0;

    while ((line = strtok(text, "\r\n")))
    {
        char* path;

        text = NULL;

        if (line[0] == '#')
            continue;

        if (strncmp(line, prefix, strlen(prefix)) == 0)
        {
            line += strlen(prefix);
            // TODO: Validate hostname
            while (*line != '/')
                line++;
        }

        (*count)++;

        path = calloc(strlen(line) + 1, 1);
        paths = realloc(paths, *count * sizeof(char*));
        paths[*count - 1] = path;

        while (*line)
        {
            if (line[0] == '%' && line[1] && line[2])
            {
                const char digits[3] = { line[1], line[2], '\0' };
                *path = (char) strtol(digits, NULL, 16);
                line += 2;
            }
            else
                *path = *line;

            path++;
            line++;
        }
    }

    return paths;
}

char* _linearwin_strdup(const char* source)
{
    const size_t length = strlen(source);
    char* result = calloc(length + 1, 1);
    strcpy(result, source);
    return result;
}

int _linearwin_min(int a, int b)
{
    return a < b ? a : b;
}

int _linearwin_max(int a, int b)
{
    return a > b ? a : b;
}

float _linearwin_fminf(float a, float b)
{
    if (a != a)
        return b;
    else if (b != b)
        return a;
    else if (a < b)
        return a;
    else
        return b;
}

float _linearwin_fmaxf(float a, float b)
{
    if (a != a)
        return b;
    else if (b != b)
        return a;
    else if (a > b)
        return a;
    else
        return b;
}


//////////////////////////////////////////////////////////////////////////
//////                         LINEARWIN event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code of an error
//
void _linearwinInputError(int code, const char* format, ...)
{
    _LINEARWINerror* error;
    char description[_LINEARWIN_MESSAGE_SIZE];

    if (format)
    {
        va_list vl;

        va_start(vl, format);
        vsnprintf(description, sizeof(description), format, vl);
        va_end(vl);

        description[sizeof(description) - 1] = '\0';
    }
    else
    {
        if (code == LINEARWIN_NOT_INITIALIZED)
            strcpy(description, "The LINEARWIN library is not initialized");
        else if (code == LINEARWIN_NO_CURRENT_CONTEXT)
            strcpy(description, "There is no current context");
        else if (code == LINEARWIN_INVALID_ENUM)
            strcpy(description, "Invalid argument for enum parameter");
        else if (code == LINEARWIN_INVALID_VALUE)
            strcpy(description, "Invalid value for parameter");
        else if (code == LINEARWIN_OUT_OF_MEMORY)
            strcpy(description, "Out of memory");
        else if (code == LINEARWIN_API_UNAVAILABLE)
            strcpy(description, "The requested API is unavailable");
        else if (code == LINEARWIN_VERSION_UNAVAILABLE)
            strcpy(description, "The requested API version is unavailable");
        else if (code == LINEARWIN_PLATFORM_ERROR)
            strcpy(description, "A platform-specific error occurred");
        else if (code == LINEARWIN_FORMAT_UNAVAILABLE)
            strcpy(description, "The requested format is unavailable");
        else if (code == LINEARWIN_NO_WINDOW_CONTEXT)
            strcpy(description, "The specified window has no context");
        else
            strcpy(description, "ERROR: UNKNOWN LINEARWIN ERROR");
    }

    if (_linearwin.initialized)
    {
        error = _linearwinPlatformGetTls(&_linearwin.errorSlot);
        if (!error)
        {
            error = calloc(1, sizeof(_LINEARWINerror));
            _linearwinPlatformSetTls(&_linearwin.errorSlot, error);
            _linearwinPlatformLockMutex(&_linearwin.errorLock);
            error->next = _linearwin.errorListHead;
            _linearwin.errorListHead = error;
            _linearwinPlatformUnlockMutex(&_linearwin.errorLock);
        }
    }
    else
        error = &_linearwinMainThreadError;

    error->code = code;
    strcpy(error->description, description);

    if (_linearwinErrorCallback)
        _linearwinErrorCallback(code, description);
}


//////////////////////////////////////////////////////////////////////////
//////                        LINEARWIN public API                       //////
//////////////////////////////////////////////////////////////////////////

LINEARWINAPI int linearwinInit(void)
{
    if (_linearwin.initialized)
        return LINEARWIN_TRUE;

    memset(&_linearwin, 0, sizeof(_linearwin));
    _linearwin.hints.init = _linearwinInitHints;

    if (!_linearwinPlatformInit())
    {
        terminate();
        return LINEARWIN_FALSE;
    }

    if (!_linearwinPlatformCreateMutex(&_linearwin.errorLock) ||
        !_linearwinPlatformCreateTls(&_linearwin.errorSlot) ||
        !_linearwinPlatformCreateTls(&_linearwin.contextSlot))
    {
        terminate();
        return LINEARWIN_FALSE;
    }

    _linearwinPlatformSetTls(&_linearwin.errorSlot, &_linearwinMainThreadError);

    _linearwinInitGamepadMappings();

    _linearwin.initialized = LINEARWIN_TRUE;
    _linearwin.timer.offset = _linearwinPlatformGetTimerValue();

    linearwinDefaultWindowHints();
    return LINEARWIN_TRUE;
}

LINEARWINAPI void linearwinTerminate(void)
{
    if (!_linearwin.initialized)
        return;

    terminate();
}

LINEARWINAPI void linearwinInitHint(int hint, int value)
{
    switch (hint)
    {
        case LINEARWIN_JOYSTICK_HAT_BUTTONS:
            _linearwinInitHints.hatButtons = value;
            return;
        case LINEARWIN_COCOA_CHDIR_RESOURCES:
            _linearwinInitHints.ns.chdir = value;
            return;
        case LINEARWIN_COCOA_MENUBAR:
            _linearwinInitHints.ns.menubar = value;
            return;
    }

    _linearwinInputError(LINEARWIN_INVALID_ENUM,
                    "Invalid init hint 0x%08X", hint);
}

LINEARWINAPI void linearwinGetVersion(int* major, int* minor, int* rev)
{
    if (major != NULL)
        *major = LINEARWIN_VERSION_MAJOR;
    if (minor != NULL)
        *minor = LINEARWIN_VERSION_MINOR;
    if (rev != NULL)
        *rev = LINEARWIN_VERSION_REVISION;
}

LINEARWINAPI const char* linearwinGetVersionString(void)
{
    return _linearwinPlatformGetVersionString();
}

LINEARWINAPI int linearwinGetError(const char** description)
{
    _LINEARWINerror* error;
    int code = LINEARWIN_NO_ERROR;

    if (description)
        *description = NULL;

    if (_linearwin.initialized)
        error = _linearwinPlatformGetTls(&_linearwin.errorSlot);
    else
        error = &_linearwinMainThreadError;

    if (error)
    {
        code = error->code;
        error->code = LINEARWIN_NO_ERROR;
        if (description && code)
            *description = error->description;
    }

    return code;
}

LINEARWINAPI LINEARWINerrorfun linearwinSetErrorCallback(LINEARWINerrorfun cbfun)
{
    _LINEARWIN_SWAP_POINTERS(_linearwinErrorCallback, cbfun);
    return cbfun;
}

