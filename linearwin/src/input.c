//========================================================================
// LINEARWIN 3.3 - www.linearwin.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@linearwin.org>
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
#include "mappings.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Internal key state used for sticky keys
#define _LINEARWIN_STICK 3

// Internal constants for gamepad mapping source types
#define _LINEARWIN_JOYSTICK_AXIS     1
#define _LINEARWIN_JOYSTICK_BUTTON   2
#define _LINEARWIN_JOYSTICK_HATBIT   3

// Finds a mapping based on joystick GUID
//
static _LINEARWINmapping* findMapping(const char* guid)
{
    int i;

    for (i = 0;  i < _linearwin.mappingCount;  i++)
    {
        if (strcmp(_linearwin.mappings[i].guid, guid) == 0)
            return _linearwin.mappings + i;
    }

    return NULL;
}

// Checks whether a gamepad mapping element is present in the hardware
//
static LINEARWINbool isValidElementForJoystick(const _LINEARWINmapelement* e,
                                          const _LINEARWINjoystick* js)
{
    if (e->type == _LINEARWIN_JOYSTICK_HATBIT && (e->index >> 4) >= js->hatCount)
        return LINEARWIN_FALSE;
    else if (e->type == _LINEARWIN_JOYSTICK_BUTTON && e->index >= js->buttonCount)
        return LINEARWIN_FALSE;
    else if (e->type == _LINEARWIN_JOYSTICK_AXIS && e->index >= js->axisCount)
        return LINEARWIN_FALSE;

    return LINEARWIN_TRUE;
}

// Finds a mapping based on joystick GUID and verifies element indices
//
static _LINEARWINmapping* findValidMapping(const _LINEARWINjoystick* js)
{
    _LINEARWINmapping* mapping = findMapping(js->guid);
    if (mapping)
    {
        int i;

        for (i = 0;  i <= LINEARWIN_GAMEPAD_BUTTON_LAST;  i++)
        {
            if (!isValidElementForJoystick(mapping->buttons + i, js))
                return NULL;
        }

        for (i = 0;  i <= LINEARWIN_GAMEPAD_AXIS_LAST;  i++)
        {
            if (!isValidElementForJoystick(mapping->axes + i, js))
                return NULL;
        }
    }

    return mapping;
}

// Parses an SDL_GameControllerDB line and adds it to the mapping list
//
static LINEARWINbool parseMapping(_LINEARWINmapping* mapping, const char* string)
{
    const char* c = string;
    size_t i, length;
    struct
    {
        const char* name;
        _LINEARWINmapelement* element;
    } fields[] =
    {
        { "platform",      NULL },
        { "a",             mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_A },
        { "b",             mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_B },
        { "x",             mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_X },
        { "y",             mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_Y },
        { "back",          mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_BACK },
        { "start",         mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_START },
        { "guide",         mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_GUIDE },
        { "leftshoulder",  mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_LEFT_BUMPER },
        { "rightshoulder", mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_RIGHT_BUMPER },
        { "leftstick",     mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_LEFT_THUMB },
        { "rightstick",    mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_RIGHT_THUMB },
        { "dpup",          mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_DPAD_UP },
        { "dpright",       mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_DPAD_RIGHT },
        { "dpdown",        mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_DPAD_DOWN },
        { "dpleft",        mapping->buttons + LINEARWIN_GAMEPAD_BUTTON_DPAD_LEFT },
        { "lefttrigger",   mapping->axes + LINEARWIN_GAMEPAD_AXIS_LEFT_TRIGGER },
        { "righttrigger",  mapping->axes + LINEARWIN_GAMEPAD_AXIS_RIGHT_TRIGGER },
        { "leftx",         mapping->axes + LINEARWIN_GAMEPAD_AXIS_LEFT_X },
        { "lefty",         mapping->axes + LINEARWIN_GAMEPAD_AXIS_LEFT_Y },
        { "rightx",        mapping->axes + LINEARWIN_GAMEPAD_AXIS_RIGHT_X },
        { "righty",        mapping->axes + LINEARWIN_GAMEPAD_AXIS_RIGHT_Y }
    };

    length = strcspn(c, ",");
    if (length != 32 || c[length] != ',')
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE, NULL);
        return LINEARWIN_FALSE;
    }

    memcpy(mapping->guid, c, length);
    c += length + 1;

    length = strcspn(c, ",");
    if (length >= sizeof(mapping->name) || c[length] != ',')
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE, NULL);
        return LINEARWIN_FALSE;
    }

    memcpy(mapping->name, c, length);
    c += length + 1;

    while (*c)
    {
        // TODO: Implement output modifiers
        if (*c == '+' || *c == '-')
            return LINEARWIN_FALSE;

        for (i = 0;  i < sizeof(fields) / sizeof(fields[0]);  i++)
        {
            length = strlen(fields[i].name);
            if (strncmp(c, fields[i].name, length) != 0 || c[length] != ':')
                continue;

            c += length + 1;

            if (fields[i].element)
            {
                _LINEARWINmapelement* e = fields[i].element;
                int8_t minimum = -1;
                int8_t maximum = 1;

                if (*c == '+')
                {
                    minimum = 0;
                    c += 1;
                }
                else if (*c == '-')
                {
                    maximum = 0;
                    c += 1;
                }

                if (*c == 'a')
                    e->type = _LINEARWIN_JOYSTICK_AXIS;
                else if (*c == 'b')
                    e->type = _LINEARWIN_JOYSTICK_BUTTON;
                else if (*c == 'h')
                    e->type = _LINEARWIN_JOYSTICK_HATBIT;
                else
                    break;

                if (e->type == _LINEARWIN_JOYSTICK_HATBIT)
                {
                    const unsigned long hat = strtoul(c + 1, (char**) &c, 10);
                    const unsigned long bit = strtoul(c + 1, (char**) &c, 10);
                    e->index = (uint8_t) ((hat << 4) | bit);
                }
                else
                    e->index = (uint8_t) strtoul(c + 1, (char**) &c, 10);

                if (e->type == _LINEARWIN_JOYSTICK_AXIS)
                {
                    e->axisScale = 2 / (maximum - minimum);
                    e->axisOffset = -(maximum + minimum);

                    if (*c == '~')
                    {
                        e->axisScale = -e->axisScale;
                        e->axisOffset = -e->axisOffset;
                    }
                }
            }
            else
            {
                length = strlen(_LINEARWIN_PLATFORM_MAPPING_NAME);
                if (strncmp(c, _LINEARWIN_PLATFORM_MAPPING_NAME, length) != 0)
                    return LINEARWIN_FALSE;
            }

            break;
        }

        c += strcspn(c, ",");
        c += strspn(c, ",");
    }

    for (i = 0;  i < 32;  i++)
    {
        if (mapping->guid[i] >= 'A' && mapping->guid[i] <= 'F')
            mapping->guid[i] += 'a' - 'A';
    }

    _linearwinPlatformUpdateGamepadGUID(mapping->guid);
    return LINEARWIN_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                         LINEARWIN event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code of a physical key event
//
void _linearwinInputKey(_LINEARWINwindow* window, int key, int scancode, int action, int mods)
{
    if (key >= 0 && key <= LINEARWIN_KEY_LAST)
    {
        LINEARWINbool repeated = LINEARWIN_FALSE;

        if (action == LINEARWIN_RELEASE && window->keys[key] == LINEARWIN_RELEASE)
            return;

        if (action == LINEARWIN_PRESS && window->keys[key] == LINEARWIN_PRESS)
            repeated = LINEARWIN_TRUE;

        if (action == LINEARWIN_RELEASE && window->stickyKeys)
            window->keys[key] = _LINEARWIN_STICK;
        else
            window->keys[key] = (char) action;

        if (repeated)
            action = LINEARWIN_REPEAT;
    }

    if (!window->lockKeyMods)
        mods &= ~(LINEARWIN_MOD_CAPS_LOCK | LINEARWIN_MOD_NUM_LOCK);

    if (window->callbacks.key)
        window->callbacks.key((LINEARWINwindow*) window, key, scancode, action, mods);
}

// Notifies shared code of a Unicode codepoint input event
// The 'plain' parameter determines whether to emit a regular character event
//
void _linearwinInputChar(_LINEARWINwindow* window, uint32_t codepoint, int mods, LINEARWINbool plain)
{
    if (codepoint < 32 || (codepoint > 126 && codepoint < 160))
        return;

    if (!window->lockKeyMods)
        mods &= ~(LINEARWIN_MOD_CAPS_LOCK | LINEARWIN_MOD_NUM_LOCK);

    if (window->callbacks.charmods)
        window->callbacks.charmods((LINEARWINwindow*) window, codepoint, mods);

    if (plain)
    {
        if (window->callbacks.character)
            window->callbacks.character((LINEARWINwindow*) window, codepoint);
    }
}

// Notifies shared code of a scroll event
//
void _linearwinInputScroll(_LINEARWINwindow* window, double xoffset, double yoffset)
{
    if (window->callbacks.scroll)
        window->callbacks.scroll((LINEARWINwindow*) window, xoffset, yoffset);
}

// Notifies shared code of a mouse button click event
//
void _linearwinInputMouseClick(_LINEARWINwindow* window, int button, int action, int mods)
{
    if (button < 0 || button > LINEARWIN_MOUSE_BUTTON_LAST)
        return;

    if (!window->lockKeyMods)
        mods &= ~(LINEARWIN_MOD_CAPS_LOCK | LINEARWIN_MOD_NUM_LOCK);

    if (action == LINEARWIN_RELEASE && window->stickyMouseButtons)
        window->mouseButtons[button] = _LINEARWIN_STICK;
    else
        window->mouseButtons[button] = (char) action;

    if (window->callbacks.mouseButton)
        window->callbacks.mouseButton((LINEARWINwindow*) window, button, action, mods);
}

// Notifies shared code of a cursor motion event
// The position is specified in content area relative screen coordinates
//
void _linearwinInputCursorPos(_LINEARWINwindow* window, double xpos, double ypos)
{
    if (window->virtualCursorPosX == xpos && window->virtualCursorPosY == ypos)
        return;

    window->virtualCursorPosX = xpos;
    window->virtualCursorPosY = ypos;

    if (window->callbacks.cursorPos)
        window->callbacks.cursorPos((LINEARWINwindow*) window, xpos, ypos);
}

// Notifies shared code of a cursor enter/leave event
//
void _linearwinInputCursorEnter(_LINEARWINwindow* window, LINEARWINbool entered)
{
    if (window->callbacks.cursorEnter)
        window->callbacks.cursorEnter((LINEARWINwindow*) window, entered);
}

// Notifies shared code of files or directories dropped on a window
//
void _linearwinInputDrop(_LINEARWINwindow* window, int count, const char** paths)
{
    if (window->callbacks.drop)
        window->callbacks.drop((LINEARWINwindow*) window, count, paths);
}

// Notifies shared code of a joystick connection or disconnection
//
void _linearwinInputJoystick(_LINEARWINjoystick* js, int event)
{
    const int jid = (int) (js - _linearwin.joysticks);

    if (event == LINEARWIN_CONNECTED)
        js->connected = LINEARWIN_TRUE;
    else if (event == LINEARWIN_DISCONNECTED)
        js->connected = LINEARWIN_FALSE;

    if (_linearwin.callbacks.joystick)
        _linearwin.callbacks.joystick(jid, event);
}

// Notifies shared code of the new value of a joystick axis
//
void _linearwinInputJoystickAxis(_LINEARWINjoystick* js, int axis, float value)
{
    js->axes[axis] = value;
}

// Notifies shared code of the new value of a joystick button
//
void _linearwinInputJoystickButton(_LINEARWINjoystick* js, int button, char value)
{
    js->buttons[button] = value;
}

// Notifies shared code of the new value of a joystick hat
//
void _linearwinInputJoystickHat(_LINEARWINjoystick* js, int hat, char value)
{
    const int base = js->buttonCount + hat * 4;

    js->buttons[base + 0] = (value & 0x01) ? LINEARWIN_PRESS : LINEARWIN_RELEASE;
    js->buttons[base + 1] = (value & 0x02) ? LINEARWIN_PRESS : LINEARWIN_RELEASE;
    js->buttons[base + 2] = (value & 0x04) ? LINEARWIN_PRESS : LINEARWIN_RELEASE;
    js->buttons[base + 3] = (value & 0x08) ? LINEARWIN_PRESS : LINEARWIN_RELEASE;

    js->hats[hat] = value;
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Adds the built-in set of gamepad mappings
//
void _linearwinInitGamepadMappings(void)
{
    int jid;
    size_t i;
    const size_t count = sizeof(_linearwinDefaultMappings) / sizeof(char*);
    _linearwin.mappings = calloc(count, sizeof(_LINEARWINmapping));

    for (i = 0;  i < count;  i++)
    {
        if (parseMapping(&_linearwin.mappings[_linearwin.mappingCount], _linearwinDefaultMappings[i]))
            _linearwin.mappingCount++;
    }

    for (jid = 0;  jid <= LINEARWIN_JOYSTICK_LAST;  jid++)
    {
        _LINEARWINjoystick* js = _linearwin.joysticks + jid;
        if (js->connected)
            js->mapping = findValidMapping(js);
    }
}

// Returns an available joystick object with arrays and name allocated
//
_LINEARWINjoystick* _linearwinAllocJoystick(const char* name,
                                  const char* guid,
                                  int axisCount,
                                  int buttonCount,
                                  int hatCount)
{
    int jid;
    _LINEARWINjoystick* js;

    for (jid = 0;  jid <= LINEARWIN_JOYSTICK_LAST;  jid++)
    {
        if (!_linearwin.joysticks[jid].allocated)
            break;
    }

    if (jid > LINEARWIN_JOYSTICK_LAST)
        return NULL;

    js = _linearwin.joysticks + jid;
    js->allocated   = LINEARWIN_TRUE;
    js->axes        = calloc(axisCount, sizeof(float));
    js->buttons     = calloc(buttonCount + (size_t) hatCount * 4, 1);
    js->hats        = calloc(hatCount, 1);
    js->axisCount   = axisCount;
    js->buttonCount = buttonCount;
    js->hatCount    = hatCount;

    strncpy(js->name, name, sizeof(js->name) - 1);
    strncpy(js->guid, guid, sizeof(js->guid) - 1);
    js->mapping = findValidMapping(js);

    return js;
}

// Frees arrays and name and flags the joystick object as unused
//
void _linearwinFreeJoystick(_LINEARWINjoystick* js)
{
    free(js->axes);
    free(js->buttons);
    free(js->hats);
    memset(js, 0, sizeof(_LINEARWINjoystick));
}

// Center the cursor in the content area of the specified window
//
void _linearwinCenterCursorInContentArea(_LINEARWINwindow* window)
{
    int width, height;

    _linearwinPlatformGetWindowSize(window, &width, &height);
    _linearwinPlatformSetCursorPos(window, width / 2.0, height / 2.0);
}


//////////////////////////////////////////////////////////////////////////
//////                        LINEARWIN public API                       //////
//////////////////////////////////////////////////////////////////////////

LINEARWINAPI int linearwinGetInputMode(LINEARWINwindow* handle, int mode)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(0);

    switch (mode)
    {
        case LINEARWIN_CURSOR:
            return window->cursorMode;
        case LINEARWIN_STICKY_KEYS:
            return window->stickyKeys;
        case LINEARWIN_STICKY_MOUSE_BUTTONS:
            return window->stickyMouseButtons;
        case LINEARWIN_LOCK_KEY_MODS:
            return window->lockKeyMods;
        case LINEARWIN_RAW_MOUSE_MOTION:
            return window->rawMouseMotion;
    }

    _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid input mode 0x%08X", mode);
    return 0;
}

LINEARWINAPI void linearwinSetInputMode(LINEARWINwindow* handle, int mode, int value)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();

    if (mode == LINEARWIN_CURSOR)
    {
        if (value != LINEARWIN_CURSOR_NORMAL &&
            value != LINEARWIN_CURSOR_HIDDEN &&
            value != LINEARWIN_CURSOR_DISABLED)
        {
            _linearwinInputError(LINEARWIN_INVALID_ENUM,
                            "Invalid cursor mode 0x%08X",
                            value);
            return;
        }

        if (window->cursorMode == value)
            return;

        window->cursorMode = value;

        _linearwinPlatformGetCursorPos(window,
                                  &window->virtualCursorPosX,
                                  &window->virtualCursorPosY);
        _linearwinPlatformSetCursorMode(window, value);
    }
    else if (mode == LINEARWIN_STICKY_KEYS)
    {
        value = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
        if (window->stickyKeys == value)
            return;

        if (!value)
        {
            int i;

            // Release all sticky keys
            for (i = 0;  i <= LINEARWIN_KEY_LAST;  i++)
            {
                if (window->keys[i] == _LINEARWIN_STICK)
                    window->keys[i] = LINEARWIN_RELEASE;
            }
        }

        window->stickyKeys = value;
    }
    else if (mode == LINEARWIN_STICKY_MOUSE_BUTTONS)
    {
        value = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
        if (window->stickyMouseButtons == value)
            return;

        if (!value)
        {
            int i;

            // Release all sticky mouse buttons
            for (i = 0;  i <= LINEARWIN_MOUSE_BUTTON_LAST;  i++)
            {
                if (window->mouseButtons[i] == _LINEARWIN_STICK)
                    window->mouseButtons[i] = LINEARWIN_RELEASE;
            }
        }

        window->stickyMouseButtons = value;
    }
    else if (mode == LINEARWIN_LOCK_KEY_MODS)
    {
        window->lockKeyMods = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
    }
    else if (mode == LINEARWIN_RAW_MOUSE_MOTION)
    {
        if (!_linearwinPlatformRawMouseMotionSupported())
        {
            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "Raw mouse motion is not supported on this system");
            return;
        }

        value = value ? LINEARWIN_TRUE : LINEARWIN_FALSE;
        if (window->rawMouseMotion == value)
            return;

        window->rawMouseMotion = value;
        _linearwinPlatformSetRawMouseMotion(window, value);
    }
    else
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid input mode 0x%08X", mode);
}

LINEARWINAPI int linearwinRawMouseMotionSupported(void)
{
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(LINEARWIN_FALSE);
    return _linearwinPlatformRawMouseMotionSupported();
}

LINEARWINAPI const char* linearwinGetKeyName(int key, int scancode)
{
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (key != LINEARWIN_KEY_UNKNOWN)
    {
        if (key != LINEARWIN_KEY_KP_EQUAL &&
            (key < LINEARWIN_KEY_KP_0 || key > LINEARWIN_KEY_KP_ADD) &&
            (key < LINEARWIN_KEY_APOSTROPHE || key > LINEARWIN_KEY_WORLD_2))
        {
            return NULL;
        }

        scancode = _linearwinPlatformGetKeyScancode(key);
    }

    return _linearwinPlatformGetScancodeName(scancode);
}

LINEARWINAPI int linearwinGetKeyScancode(int key)
{
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(-1);

    if (key < LINEARWIN_KEY_SPACE || key > LINEARWIN_KEY_LAST)
    {
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid key %i", key);
        return LINEARWIN_RELEASE;
    }

    return _linearwinPlatformGetKeyScancode(key);
}

LINEARWINAPI int linearwinGetKey(LINEARWINwindow* handle, int key)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(LINEARWIN_RELEASE);

    if (key < LINEARWIN_KEY_SPACE || key > LINEARWIN_KEY_LAST)
    {
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid key %i", key);
        return LINEARWIN_RELEASE;
    }

    if (window->keys[key] == _LINEARWIN_STICK)
    {
        // Sticky mode: release key now
        window->keys[key] = LINEARWIN_RELEASE;
        return LINEARWIN_PRESS;
    }

    return (int) window->keys[key];
}

LINEARWINAPI int linearwinGetMouseButton(LINEARWINwindow* handle, int button)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(LINEARWIN_RELEASE);

    if (button < LINEARWIN_MOUSE_BUTTON_1 || button > LINEARWIN_MOUSE_BUTTON_LAST)
    {
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid mouse button %i", button);
        return LINEARWIN_RELEASE;
    }

    if (window->mouseButtons[button] == _LINEARWIN_STICK)
    {
        // Sticky mode: release mouse button now
        window->mouseButtons[button] = LINEARWIN_RELEASE;
        return LINEARWIN_PRESS;
    }

    return (int) window->mouseButtons[button];
}

LINEARWINAPI void linearwinGetCursorPos(LINEARWINwindow* handle, double* xpos, double* ypos)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;

    _LINEARWIN_REQUIRE_INIT();

    if (window->cursorMode == LINEARWIN_CURSOR_DISABLED)
    {
        if (xpos)
            *xpos = window->virtualCursorPosX;
        if (ypos)
            *ypos = window->virtualCursorPosY;
    }
    else
        _linearwinPlatformGetCursorPos(window, xpos, ypos);
}

LINEARWINAPI void linearwinSetCursorPos(LINEARWINwindow* handle, double xpos, double ypos)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();

    if (xpos != xpos || xpos < -DBL_MAX || xpos > DBL_MAX ||
        ypos != ypos || ypos < -DBL_MAX || ypos > DBL_MAX)
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE,
                        "Invalid cursor position %f %f",
                        xpos, ypos);
        return;
    }

    if (!_linearwinPlatformWindowFocused(window))
        return;

    if (window->cursorMode == LINEARWIN_CURSOR_DISABLED)
    {
        // Only update the accumulated position if the cursor is disabled
        window->virtualCursorPosX = xpos;
        window->virtualCursorPosY = ypos;
    }
    else
    {
        // Update system cursor position
        _linearwinPlatformSetCursorPos(window, xpos, ypos);
    }
}

LINEARWINAPI LINEARWINcursor* linearwinCreateCursor(const LINEARWINimage* image, int xhot, int yhot)
{
    _LINEARWINcursor* cursor;

    assert(image != NULL);
    assert(image->pixels != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (image->width <= 0 || image->height <= 0)
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE, "Invalid image dimensions for cursor");
        return NULL;
    }

    cursor = calloc(1, sizeof(_LINEARWINcursor));
    cursor->next = _linearwin.cursorListHead;
    _linearwin.cursorListHead = cursor;

    if (!_linearwinPlatformCreateCursor(cursor, image, xhot, yhot))
    {
        linearwinDestroyCursor((LINEARWINcursor*) cursor);
        return NULL;
    }

    return (LINEARWINcursor*) cursor;
}

LINEARWINAPI LINEARWINcursor* linearwinCreateStandardCursor(int shape)
{
    _LINEARWINcursor* cursor;

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (shape != LINEARWIN_ARROW_CURSOR &&
        shape != LINEARWIN_IBEAM_CURSOR &&
        shape != LINEARWIN_CROSSHAIR_CURSOR &&
        shape != LINEARWIN_HAND_CURSOR &&
        shape != LINEARWIN_HRESIZE_CURSOR &&
        shape != LINEARWIN_VRESIZE_CURSOR)
    {
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid standard cursor 0x%08X", shape);
        return NULL;
    }

    cursor = calloc(1, sizeof(_LINEARWINcursor));
    cursor->next = _linearwin.cursorListHead;
    _linearwin.cursorListHead = cursor;

    if (!_linearwinPlatformCreateStandardCursor(cursor, shape))
    {
        linearwinDestroyCursor((LINEARWINcursor*) cursor);
        return NULL;
    }

    return (LINEARWINcursor*) cursor;
}

LINEARWINAPI void linearwinDestroyCursor(LINEARWINcursor* handle)
{
    _LINEARWINcursor* cursor = (_LINEARWINcursor*) handle;

    _LINEARWIN_REQUIRE_INIT();

    if (cursor == NULL)
        return;

    // Make sure the cursor is not being used by any window
    {
        _LINEARWINwindow* window;

        for (window = _linearwin.windowListHead;  window;  window = window->next)
        {
            if (window->cursor == cursor)
                linearwinSetCursor((LINEARWINwindow*) window, NULL);
        }
    }

    _linearwinPlatformDestroyCursor(cursor);

    // Unlink cursor from global linked list
    {
        _LINEARWINcursor** prev = &_linearwin.cursorListHead;

        while (*prev != cursor)
            prev = &((*prev)->next);

        *prev = cursor->next;
    }

    free(cursor);
}

LINEARWINAPI void linearwinSetCursor(LINEARWINwindow* windowHandle, LINEARWINcursor* cursorHandle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) windowHandle;
    _LINEARWINcursor* cursor = (_LINEARWINcursor*) cursorHandle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT();

    window->cursor = cursor;

    _linearwinPlatformSetCursor(window, cursor);
}

LINEARWINAPI LINEARWINkeyfun linearwinSetKeyCallback(LINEARWINwindow* handle, LINEARWINkeyfun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.key, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINcharfun linearwinSetCharCallback(LINEARWINwindow* handle, LINEARWINcharfun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.character, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINcharmodsfun linearwinSetCharModsCallback(LINEARWINwindow* handle, LINEARWINcharmodsfun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.charmods, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINmousebuttonfun linearwinSetMouseButtonCallback(LINEARWINwindow* handle,
                                                      LINEARWINmousebuttonfun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.mouseButton, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINcursorposfun linearwinSetCursorPosCallback(LINEARWINwindow* handle,
                                                  LINEARWINcursorposfun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.cursorPos, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINcursorenterfun linearwinSetCursorEnterCallback(LINEARWINwindow* handle,
                                                      LINEARWINcursorenterfun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.cursorEnter, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINscrollfun linearwinSetScrollCallback(LINEARWINwindow* handle,
                                            LINEARWINscrollfun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.scroll, cbfun);
    return cbfun;
}

LINEARWINAPI LINEARWINdropfun linearwinSetDropCallback(LINEARWINwindow* handle, LINEARWINdropfun cbfun)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(window->callbacks.drop, cbfun);
    return cbfun;
}

LINEARWINAPI int linearwinJoystickPresent(int jid)
{
    _LINEARWINjoystick* js;

    assert(jid >= LINEARWIN_JOYSTICK_1);
    assert(jid <= LINEARWIN_JOYSTICK_LAST);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(LINEARWIN_FALSE);

    if (jid < 0 || jid > LINEARWIN_JOYSTICK_LAST)
    {
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return LINEARWIN_FALSE;
    }

    js = _linearwin.joysticks + jid;
    if (!js->connected)
        return LINEARWIN_FALSE;

    return _linearwinPlatformPollJoystick(js, _LINEARWIN_POLL_PRESENCE);
}

LINEARWINAPI const float* linearwinGetJoystickAxes(int jid, int* count)
{
    _LINEARWINjoystick* js;

    assert(jid >= LINEARWIN_JOYSTICK_1);
    assert(jid <= LINEARWIN_JOYSTICK_LAST);
    assert(count != NULL);

    *count = 0;

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > LINEARWIN_JOYSTICK_LAST)
    {
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    js = _linearwin.joysticks + jid;
    if (!js->connected)
        return NULL;

    if (!_linearwinPlatformPollJoystick(js, _LINEARWIN_POLL_AXES))
        return NULL;

    *count = js->axisCount;
    return js->axes;
}

LINEARWINAPI const unsigned char* linearwinGetJoystickButtons(int jid, int* count)
{
    _LINEARWINjoystick* js;

    assert(jid >= LINEARWIN_JOYSTICK_1);
    assert(jid <= LINEARWIN_JOYSTICK_LAST);
    assert(count != NULL);

    *count = 0;

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > LINEARWIN_JOYSTICK_LAST)
    {
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    js = _linearwin.joysticks + jid;
    if (!js->connected)
        return NULL;

    if (!_linearwinPlatformPollJoystick(js, _LINEARWIN_POLL_BUTTONS))
        return NULL;

    if (_linearwin.hints.init.hatButtons)
        *count = js->buttonCount + js->hatCount * 4;
    else
        *count = js->buttonCount;

    return js->buttons;
}

LINEARWINAPI const unsigned char* linearwinGetJoystickHats(int jid, int* count)
{
    _LINEARWINjoystick* js;

    assert(jid >= LINEARWIN_JOYSTICK_1);
    assert(jid <= LINEARWIN_JOYSTICK_LAST);
    assert(count != NULL);

    *count = 0;

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > LINEARWIN_JOYSTICK_LAST)
    {
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    js = _linearwin.joysticks + jid;
    if (!js->connected)
        return NULL;

    if (!_linearwinPlatformPollJoystick(js, _LINEARWIN_POLL_BUTTONS))
        return NULL;

    *count = js->hatCount;
    return js->hats;
}

LINEARWINAPI const char* linearwinGetJoystickName(int jid)
{
    _LINEARWINjoystick* js;

    assert(jid >= LINEARWIN_JOYSTICK_1);
    assert(jid <= LINEARWIN_JOYSTICK_LAST);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > LINEARWIN_JOYSTICK_LAST)
    {
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    js = _linearwin.joysticks + jid;
    if (!js->connected)
        return NULL;

    if (!_linearwinPlatformPollJoystick(js, _LINEARWIN_POLL_PRESENCE))
        return NULL;

    return js->name;
}

LINEARWINAPI const char* linearwinGetJoystickGUID(int jid)
{
    _LINEARWINjoystick* js;

    assert(jid >= LINEARWIN_JOYSTICK_1);
    assert(jid <= LINEARWIN_JOYSTICK_LAST);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > LINEARWIN_JOYSTICK_LAST)
    {
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    js = _linearwin.joysticks + jid;
    if (!js->connected)
        return NULL;

    if (!_linearwinPlatformPollJoystick(js, _LINEARWIN_POLL_PRESENCE))
        return NULL;

    return js->guid;
}

LINEARWINAPI void linearwinSetJoystickUserPointer(int jid, void* pointer)
{
    _LINEARWINjoystick* js;

    assert(jid >= LINEARWIN_JOYSTICK_1);
    assert(jid <= LINEARWIN_JOYSTICK_LAST);

    _LINEARWIN_REQUIRE_INIT();

    js = _linearwin.joysticks + jid;
    if (!js->allocated)
        return;

    js->userPointer = pointer;
}

LINEARWINAPI void* linearwinGetJoystickUserPointer(int jid)
{
    _LINEARWINjoystick* js;

    assert(jid >= LINEARWIN_JOYSTICK_1);
    assert(jid <= LINEARWIN_JOYSTICK_LAST);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    js = _linearwin.joysticks + jid;
    if (!js->allocated)
        return NULL;

    return js->userPointer;
}

LINEARWINAPI LINEARWINjoystickfun linearwinSetJoystickCallback(LINEARWINjoystickfun cbfun)
{
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _LINEARWIN_SWAP_POINTERS(_linearwin.callbacks.joystick, cbfun);
    return cbfun;
}

LINEARWINAPI int linearwinUpdateGamepadMappings(const char* string)
{
    int jid;
    const char* c = string;

    assert(string != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(LINEARWIN_FALSE);

    while (*c)
    {
        if ((*c >= '0' && *c <= '9') ||
            (*c >= 'a' && *c <= 'f') ||
            (*c >= 'A' && *c <= 'F'))
        {
            char line[1024];

            const size_t length = strcspn(c, "\r\n");
            if (length < sizeof(line))
            {
                _LINEARWINmapping mapping = {{0}};

                memcpy(line, c, length);
                line[length] = '\0';

                if (parseMapping(&mapping, line))
                {
                    _LINEARWINmapping* previous = findMapping(mapping.guid);
                    if (previous)
                        *previous = mapping;
                    else
                    {
                        _linearwin.mappingCount++;
                        _linearwin.mappings =
                            realloc(_linearwin.mappings,
                                    sizeof(_LINEARWINmapping) * _linearwin.mappingCount);
                        _linearwin.mappings[_linearwin.mappingCount - 1] = mapping;
                    }
                }
            }

            c += length;
        }
        else
        {
            c += strcspn(c, "\r\n");
            c += strspn(c, "\r\n");
        }
    }

    for (jid = 0;  jid <= LINEARWIN_JOYSTICK_LAST;  jid++)
    {
        _LINEARWINjoystick* js = _linearwin.joysticks + jid;
        if (js->connected)
            js->mapping = findValidMapping(js);
    }

    return LINEARWIN_TRUE;
}

LINEARWINAPI int linearwinJoystickIsGamepad(int jid)
{
    _LINEARWINjoystick* js;

    assert(jid >= LINEARWIN_JOYSTICK_1);
    assert(jid <= LINEARWIN_JOYSTICK_LAST);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(LINEARWIN_FALSE);

    if (jid < 0 || jid > LINEARWIN_JOYSTICK_LAST)
    {
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return LINEARWIN_FALSE;
    }

    js = _linearwin.joysticks + jid;
    if (!js->connected)
        return LINEARWIN_FALSE;

    if (!_linearwinPlatformPollJoystick(js, _LINEARWIN_POLL_PRESENCE))
        return LINEARWIN_FALSE;

    return js->mapping != NULL;
}

LINEARWINAPI const char* linearwinGetGamepadName(int jid)
{
    _LINEARWINjoystick* js;

    assert(jid >= LINEARWIN_JOYSTICK_1);
    assert(jid <= LINEARWIN_JOYSTICK_LAST);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > LINEARWIN_JOYSTICK_LAST)
    {
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    js = _linearwin.joysticks + jid;
    if (!js->connected)
        return NULL;

    if (!_linearwinPlatformPollJoystick(js, _LINEARWIN_POLL_PRESENCE))
        return NULL;

    if (!js->mapping)
        return NULL;

    return js->mapping->name;
}

LINEARWINAPI int linearwinGetGamepadState(int jid, LINEARWINgamepadstate* state)
{
    int i;
    _LINEARWINjoystick* js;

    assert(jid >= LINEARWIN_JOYSTICK_1);
    assert(jid <= LINEARWIN_JOYSTICK_LAST);
    assert(state != NULL);

    memset(state, 0, sizeof(LINEARWINgamepadstate));

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(LINEARWIN_FALSE);

    if (jid < 0 || jid > LINEARWIN_JOYSTICK_LAST)
    {
        _linearwinInputError(LINEARWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return LINEARWIN_FALSE;
    }

    js = _linearwin.joysticks + jid;
    if (!js->connected)
        return LINEARWIN_FALSE;

    if (!_linearwinPlatformPollJoystick(js, _LINEARWIN_POLL_ALL))
        return LINEARWIN_FALSE;

    if (!js->mapping)
        return LINEARWIN_FALSE;

    for (i = 0;  i <= LINEARWIN_GAMEPAD_BUTTON_LAST;  i++)
    {
        const _LINEARWINmapelement* e = js->mapping->buttons + i;
        if (e->type == _LINEARWIN_JOYSTICK_AXIS)
        {
            const float value = js->axes[e->index] * e->axisScale + e->axisOffset;
            // HACK: This should be baked into the value transform
            // TODO: Bake into transform when implementing output modifiers
            if (e->axisOffset < 0 || (e->axisOffset == 0 && e->axisScale > 0))
            {
                if (value >= 0.f)
                    state->buttons[i] = LINEARWIN_PRESS;
            }
            else
            {
                if (value <= 0.f)
                    state->buttons[i] = LINEARWIN_PRESS;
            }
        }
        else if (e->type == _LINEARWIN_JOYSTICK_HATBIT)
        {
            const unsigned int hat = e->index >> 4;
            const unsigned int bit = e->index & 0xf;
            if (js->hats[hat] & bit)
                state->buttons[i] = LINEARWIN_PRESS;
        }
        else if (e->type == _LINEARWIN_JOYSTICK_BUTTON)
            state->buttons[i] = js->buttons[e->index];
    }

    for (i = 0;  i <= LINEARWIN_GAMEPAD_AXIS_LAST;  i++)
    {
        const _LINEARWINmapelement* e = js->mapping->axes + i;
        if (e->type == _LINEARWIN_JOYSTICK_AXIS)
        {
            const float value = js->axes[e->index] * e->axisScale + e->axisOffset;
            state->axes[i] = _linearwin_fminf(_linearwin_fmaxf(value, -1.f), 1.f);
        }
        else if (e->type == _LINEARWIN_JOYSTICK_HATBIT)
        {
            const unsigned int hat = e->index >> 4;
            const unsigned int bit = e->index & 0xf;
            if (js->hats[hat] & bit)
                state->axes[i] = 1.f;
            else
                state->axes[i] = -1.f;
        }
        else if (e->type == _LINEARWIN_JOYSTICK_BUTTON)
            state->axes[i] = js->buttons[e->index] * 2.f - 1.f;
    }

    return LINEARWIN_TRUE;
}

LINEARWINAPI void linearwinSetClipboardString(LINEARWINwindow* handle, const char* string)
{
    assert(string != NULL);

    _LINEARWIN_REQUIRE_INIT();
    _linearwinPlatformSetClipboardString(string);
}

LINEARWINAPI const char* linearwinGetClipboardString(LINEARWINwindow* handle)
{
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return _linearwinPlatformGetClipboardString();
}

LINEARWINAPI double linearwinGetTime(void)
{
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(0.0);
    return (double) (_linearwinPlatformGetTimerValue() - _linearwin.timer.offset) /
        _linearwinPlatformGetTimerFrequency();
}

LINEARWINAPI void linearwinSetTime(double time)
{
    _LINEARWIN_REQUIRE_INIT();

    if (time != time || time < 0.0 || time > 18446744073.0)
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE, "Invalid time %f", time);
        return;
    }

    _linearwin.timer.offset = _linearwinPlatformGetTimerValue() -
        (uint64_t) (time * _linearwinPlatformGetTimerFrequency());
}

LINEARWINAPI uint64_t linearwinGetTimerValue(void)
{
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(0);
    return _linearwinPlatformGetTimerValue();
}

LINEARWINAPI uint64_t linearwinGetTimerFrequency(void)
{
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(0);
    return _linearwinPlatformGetTimerFrequency();
}

