//========================================================================
// RBLXWIN 3.3 - www.rblxwin.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2018 Camilla Löwy <elmindreda@rblxwin.org>
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


// NOTE: The global variables below comprise all mutable global data in RBLXWIN
//       Any other mutable global variable is a bug

// This contains all mutable state shared between compilation units of RBLXWIN
//
_RBLXWINlibrary _rblxwin = { RBLXWIN_FALSE };

// These are outside of _rblxwin so they can be used before initialization and
// after termination without special handling when _rblxwin is cleared to zero
//
static _RBLXWINerror _rblxwinMainThreadError;
static RBLXWINerrorfun _rblxwinErrorCallback;
static _RBLXWINinitconfig _rblxwinInitHints =
{
    RBLXWIN_TRUE,      // hat buttons
    {
        RBLXWIN_TRUE,  // macOS menu bar
        RBLXWIN_TRUE   // macOS bundle chdir
    }
};

// Terminate the library
//
static void terminate(void)
{
    int i;

    memset(&_rblxwin.callbacks, 0, sizeof(_rblxwin.callbacks));

    while (_rblxwin.windowListHead)
        rblxwinDestroyWindow((RBLXWINwindow*) _rblxwin.windowListHead);

    while (_rblxwin.cursorListHead)
        rblxwinDestroyCursor((RBLXWINcursor*) _rblxwin.cursorListHead);

    for (i = 0;  i < _rblxwin.monitorCount;  i++)
    {
        _RBLXWINmonitor* monitor = _rblxwin.monitors[i];
        if (monitor->originalRamp.size)
            _rblxwinPlatformSetGammaRamp(monitor, &monitor->originalRamp);
        _rblxwinFreeMonitor(monitor);
    }

    free(_rblxwin.monitors);
    _rblxwin.monitors = NULL;
    _rblxwin.monitorCount = 0;

    free(_rblxwin.mappings);
    _rblxwin.mappings = NULL;
    _rblxwin.mappingCount = 0;

    _rblxwinTerminateVulkan();
    _rblxwinPlatformTerminate();

    _rblxwin.initialized = RBLXWIN_FALSE;

    while (_rblxwin.errorListHead)
    {
        _RBLXWINerror* error = _rblxwin.errorListHead;
        _rblxwin.errorListHead = error->next;
        free(error);
    }

    _rblxwinPlatformDestroyTls(&_rblxwin.contextSlot);
    _rblxwinPlatformDestroyTls(&_rblxwin.errorSlot);
    _rblxwinPlatformDestroyMutex(&_rblxwin.errorLock);

    memset(&_rblxwin, 0, sizeof(_rblxwin));
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Encode a Unicode code point to a UTF-8 stream
// Based on cutef8 by Jeff Bezanson (Public Domain)
//
size_t _rblxwinEncodeUTF8(char* s, uint32_t codepoint)
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
char** _rblxwinParseUriList(char* text, int* count)
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

char* _rblxwin_strdup(const char* source)
{
    const size_t length = strlen(source);
    char* result = calloc(length + 1, 1);
    strcpy(result, source);
    return result;
}

int _rblxwin_min(int a, int b)
{
    return a < b ? a : b;
}

int _rblxwin_max(int a, int b)
{
    return a > b ? a : b;
}

float _rblxwin_fminf(float a, float b)
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

float _rblxwin_fmaxf(float a, float b)
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
//////                         RBLXWIN event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code of an error
//
void _rblxwinInputError(int code, const char* format, ...)
{
    _RBLXWINerror* error;
    char description[_RBLXWIN_MESSAGE_SIZE];

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
        if (code == RBLXWIN_NOT_INITIALIZED)
            strcpy(description, "The RBLXWIN library is not initialized");
        else if (code == RBLXWIN_NO_CURRENT_CONTEXT)
            strcpy(description, "There is no current context");
        else if (code == RBLXWIN_INVALID_ENUM)
            strcpy(description, "Invalid argument for enum parameter");
        else if (code == RBLXWIN_INVALID_VALUE)
            strcpy(description, "Invalid value for parameter");
        else if (code == RBLXWIN_OUT_OF_MEMORY)
            strcpy(description, "Out of memory");
        else if (code == RBLXWIN_API_UNAVAILABLE)
            strcpy(description, "The requested API is unavailable");
        else if (code == RBLXWIN_VERSION_UNAVAILABLE)
            strcpy(description, "The requested API version is unavailable");
        else if (code == RBLXWIN_PLATFORM_ERROR)
            strcpy(description, "A platform-specific error occurred");
        else if (code == RBLXWIN_FORMAT_UNAVAILABLE)
            strcpy(description, "The requested format is unavailable");
        else if (code == RBLXWIN_NO_WINDOW_CONTEXT)
            strcpy(description, "The specified window has no context");
        else
            strcpy(description, "ERROR: UNKNOWN RBLXWIN ERROR");
    }

    if (_rblxwin.initialized)
    {
        error = _rblxwinPlatformGetTls(&_rblxwin.errorSlot);
        if (!error)
        {
            error = calloc(1, sizeof(_RBLXWINerror));
            _rblxwinPlatformSetTls(&_rblxwin.errorSlot, error);
            _rblxwinPlatformLockMutex(&_rblxwin.errorLock);
            error->next = _rblxwin.errorListHead;
            _rblxwin.errorListHead = error;
            _rblxwinPlatformUnlockMutex(&_rblxwin.errorLock);
        }
    }
    else
        error = &_rblxwinMainThreadError;

    error->code = code;
    strcpy(error->description, description);

    if (_rblxwinErrorCallback)
        _rblxwinErrorCallback(code, description);
}


//////////////////////////////////////////////////////////////////////////
//////                        RBLXWIN public API                       //////
//////////////////////////////////////////////////////////////////////////

RBLXWINAPI int rblxwinInit(void)
{
    if (_rblxwin.initialized)
        return RBLXWIN_TRUE;

    memset(&_rblxwin, 0, sizeof(_rblxwin));
    _rblxwin.hints.init = _rblxwinInitHints;

    if (!_rblxwinPlatformInit())
    {
        terminate();
        return RBLXWIN_FALSE;
    }

    if (!_rblxwinPlatformCreateMutex(&_rblxwin.errorLock) ||
        !_rblxwinPlatformCreateTls(&_rblxwin.errorSlot) ||
        !_rblxwinPlatformCreateTls(&_rblxwin.contextSlot))
    {
        terminate();
        return RBLXWIN_FALSE;
    }

    _rblxwinPlatformSetTls(&_rblxwin.errorSlot, &_rblxwinMainThreadError);

    _rblxwinInitGamepadMappings();

    _rblxwin.initialized = RBLXWIN_TRUE;
    _rblxwin.timer.offset = _rblxwinPlatformGetTimerValue();

    rblxwinDefaultWindowHints();
    return RBLXWIN_TRUE;
}

RBLXWINAPI void rblxwinTerminate(void)
{
    if (!_rblxwin.initialized)
        return;

    terminate();
}

RBLXWINAPI void rblxwinInitHint(int hint, int value)
{
    switch (hint)
    {
        case RBLXWIN_JOYSTICK_HAT_BUTTONS:
            _rblxwinInitHints.hatButtons = value;
            return;
        case RBLXWIN_COCOA_CHDIR_RESOURCES:
            _rblxwinInitHints.ns.chdir = value;
            return;
        case RBLXWIN_COCOA_MENUBAR:
            _rblxwinInitHints.ns.menubar = value;
            return;
    }

    _rblxwinInputError(RBLXWIN_INVALID_ENUM,
                    "Invalid init hint 0x%08X", hint);
}

RBLXWINAPI void rblxwinGetVersion(int* major, int* minor, int* rev)
{
    if (major != NULL)
        *major = RBLXWIN_VERSION_MAJOR;
    if (minor != NULL)
        *minor = RBLXWIN_VERSION_MINOR;
    if (rev != NULL)
        *rev = RBLXWIN_VERSION_REVISION;
}

RBLXWINAPI const char* rblxwinGetVersionString(void)
{
    return _rblxwinPlatformGetVersionString();
}

RBLXWINAPI int rblxwinGetError(const char** description)
{
    _RBLXWINerror* error;
    int code = RBLXWIN_NO_ERROR;

    if (description)
        *description = NULL;

    if (_rblxwin.initialized)
        error = _rblxwinPlatformGetTls(&_rblxwin.errorSlot);
    else
        error = &_rblxwinMainThreadError;

    if (error)
    {
        code = error->code;
        error->code = RBLXWIN_NO_ERROR;
        if (description && code)
            *description = error->description;
    }

    return code;
}

RBLXWINAPI RBLXWINerrorfun rblxwinSetErrorCallback(RBLXWINerrorfun cbfun)
{
    _RBLXWIN_SWAP_POINTERS(_rblxwinErrorCallback, cbfun);
    return cbfun;
}

