//========================================================================
// RBLXWIN 3.3 - www.rblxwin.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@rblxwin.org>
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
#define _RBLXWIN_STICK 3

// Internal constants for gamepad mapping source types
#define _RBLXWIN_JOYSTICK_AXIS     1
#define _RBLXWIN_JOYSTICK_BUTTON   2
#define _RBLXWIN_JOYSTICK_HATBIT   3

// Finds a mapping based on joystick GUID
//
static _RBLXWINmapping* findMapping(const char* guid)
{
    int i;

    for (i = 0;  i < _rblxwin.mappingCount;  i++)
    {
        if (strcmp(_rblxwin.mappings[i].guid, guid) == 0)
            return _rblxwin.mappings + i;
    }

    return NULL;
}

// Checks whether a gamepad mapping element is present in the hardware
//
static RBLXWINbool isValidElementForJoystick(const _RBLXWINmapelement* e,
                                          const _RBLXWINjoystick* js)
{
    if (e->type == _RBLXWIN_JOYSTICK_HATBIT && (e->index >> 4) >= js->hatCount)
        return RBLXWIN_FALSE;
    else if (e->type == _RBLXWIN_JOYSTICK_BUTTON && e->index >= js->buttonCount)
        return RBLXWIN_FALSE;
    else if (e->type == _RBLXWIN_JOYSTICK_AXIS && e->index >= js->axisCount)
        return RBLXWIN_FALSE;

    return RBLXWIN_TRUE;
}

// Finds a mapping based on joystick GUID and verifies element indices
//
static _RBLXWINmapping* findValidMapping(const _RBLXWINjoystick* js)
{
    _RBLXWINmapping* mapping = findMapping(js->guid);
    if (mapping)
    {
        int i;

        for (i = 0;  i <= RBLXWIN_GAMEPAD_BUTTON_LAST;  i++)
        {
            if (!isValidElementForJoystick(mapping->buttons + i, js))
                return NULL;
        }

        for (i = 0;  i <= RBLXWIN_GAMEPAD_AXIS_LAST;  i++)
        {
            if (!isValidElementForJoystick(mapping->axes + i, js))
                return NULL;
        }
    }

    return mapping;
}

// Parses an SDL_GameControllerDB line and adds it to the mapping list
//
static RBLXWINbool parseMapping(_RBLXWINmapping* mapping, const char* string)
{
    const char* c = string;
    size_t i, length;
    struct
    {
        const char* name;
        _RBLXWINmapelement* element;
    } fields[] =
    {
        { "platform",      NULL },
        { "a",             mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_A },
        { "b",             mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_B },
        { "x",             mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_X },
        { "y",             mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_Y },
        { "back",          mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_BACK },
        { "start",         mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_START },
        { "guide",         mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_GUIDE },
        { "leftshoulder",  mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_LEFT_BUMPER },
        { "rightshoulder", mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_RIGHT_BUMPER },
        { "leftstick",     mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_LEFT_THUMB },
        { "rightstick",    mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_RIGHT_THUMB },
        { "dpup",          mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_DPAD_UP },
        { "dpright",       mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_DPAD_RIGHT },
        { "dpdown",        mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_DPAD_DOWN },
        { "dpleft",        mapping->buttons + RBLXWIN_GAMEPAD_BUTTON_DPAD_LEFT },
        { "lefttrigger",   mapping->axes + RBLXWIN_GAMEPAD_AXIS_LEFT_TRIGGER },
        { "righttrigger",  mapping->axes + RBLXWIN_GAMEPAD_AXIS_RIGHT_TRIGGER },
        { "leftx",         mapping->axes + RBLXWIN_GAMEPAD_AXIS_LEFT_X },
        { "lefty",         mapping->axes + RBLXWIN_GAMEPAD_AXIS_LEFT_Y },
        { "rightx",        mapping->axes + RBLXWIN_GAMEPAD_AXIS_RIGHT_X },
        { "righty",        mapping->axes + RBLXWIN_GAMEPAD_AXIS_RIGHT_Y }
    };

    length = strcspn(c, ",");
    if (length != 32 || c[length] != ',')
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE, NULL);
        return RBLXWIN_FALSE;
    }

    memcpy(mapping->guid, c, length);
    c += length + 1;

    length = strcspn(c, ",");
    if (length >= sizeof(mapping->name) || c[length] != ',')
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE, NULL);
        return RBLXWIN_FALSE;
    }

    memcpy(mapping->name, c, length);
    c += length + 1;

    while (*c)
    {
        // TODO: Implement output modifiers
        if (*c == '+' || *c == '-')
            return RBLXWIN_FALSE;

        for (i = 0;  i < sizeof(fields) / sizeof(fields[0]);  i++)
        {
            length = strlen(fields[i].name);
            if (strncmp(c, fields[i].name, length) != 0 || c[length] != ':')
                continue;

            c += length + 1;

            if (fields[i].element)
            {
                _RBLXWINmapelement* e = fields[i].element;
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
                    e->type = _RBLXWIN_JOYSTICK_AXIS;
                else if (*c == 'b')
                    e->type = _RBLXWIN_JOYSTICK_BUTTON;
                else if (*c == 'h')
                    e->type = _RBLXWIN_JOYSTICK_HATBIT;
                else
                    break;

                if (e->type == _RBLXWIN_JOYSTICK_HATBIT)
                {
                    const unsigned long hat = strtoul(c + 1, (char**) &c, 10);
                    const unsigned long bit = strtoul(c + 1, (char**) &c, 10);
                    e->index = (uint8_t) ((hat << 4) | bit);
                }
                else
                    e->index = (uint8_t) strtoul(c + 1, (char**) &c, 10);

                if (e->type == _RBLXWIN_JOYSTICK_AXIS)
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
                length = strlen(_RBLXWIN_PLATFORM_MAPPING_NAME);
                if (strncmp(c, _RBLXWIN_PLATFORM_MAPPING_NAME, length) != 0)
                    return RBLXWIN_FALSE;
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

    _rblxwinPlatformUpdateGamepadGUID(mapping->guid);
    return RBLXWIN_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                         RBLXWIN event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code of a physical key event
//
void _rblxwinInputKey(_RBLXWINwindow* window, int key, int scancode, int action, int mods)
{
    if (key >= 0 && key <= RBLXWIN_KEY_LAST)
    {
        RBLXWINbool repeated = RBLXWIN_FALSE;

        if (action == RBLXWIN_RELEASE && window->keys[key] == RBLXWIN_RELEASE)
            return;

        if (action == RBLXWIN_PRESS && window->keys[key] == RBLXWIN_PRESS)
            repeated = RBLXWIN_TRUE;

        if (action == RBLXWIN_RELEASE && window->stickyKeys)
            window->keys[key] = _RBLXWIN_STICK;
        else
            window->keys[key] = (char) action;

        if (repeated)
            action = RBLXWIN_REPEAT;
    }

    if (!window->lockKeyMods)
        mods &= ~(RBLXWIN_MOD_CAPS_LOCK | RBLXWIN_MOD_NUM_LOCK);

    if (window->callbacks.key)
        window->callbacks.key((RBLXWINwindow*) window, key, scancode, action, mods);
}

// Notifies shared code of a Unicode codepoint input event
// The 'plain' parameter determines whether to emit a regular character event
//
void _rblxwinInputChar(_RBLXWINwindow* window, uint32_t codepoint, int mods, RBLXWINbool plain)
{
    if (codepoint < 32 || (codepoint > 126 && codepoint < 160))
        return;

    if (!window->lockKeyMods)
        mods &= ~(RBLXWIN_MOD_CAPS_LOCK | RBLXWIN_MOD_NUM_LOCK);

    if (window->callbacks.charmods)
        window->callbacks.charmods((RBLXWINwindow*) window, codepoint, mods);

    if (plain)
    {
        if (window->callbacks.character)
            window->callbacks.character((RBLXWINwindow*) window, codepoint);
    }
}

// Notifies shared code of a scroll event
//
void _rblxwinInputScroll(_RBLXWINwindow* window, double xoffset, double yoffset)
{
    if (window->callbacks.scroll)
        window->callbacks.scroll((RBLXWINwindow*) window, xoffset, yoffset);
}

// Notifies shared code of a mouse button click event
//
void _rblxwinInputMouseClick(_RBLXWINwindow* window, int button, int action, int mods)
{
    if (button < 0 || button > RBLXWIN_MOUSE_BUTTON_LAST)
        return;

    if (!window->lockKeyMods)
        mods &= ~(RBLXWIN_MOD_CAPS_LOCK | RBLXWIN_MOD_NUM_LOCK);

    if (action == RBLXWIN_RELEASE && window->stickyMouseButtons)
        window->mouseButtons[button] = _RBLXWIN_STICK;
    else
        window->mouseButtons[button] = (char) action;

    if (window->callbacks.mouseButton)
        window->callbacks.mouseButton((RBLXWINwindow*) window, button, action, mods);
}

// Notifies shared code of a cursor motion event
// The position is specified in content area relative screen coordinates
//
void _rblxwinInputCursorPos(_RBLXWINwindow* window, double xpos, double ypos)
{
    if (window->virtualCursorPosX == xpos && window->virtualCursorPosY == ypos)
        return;

    window->virtualCursorPosX = xpos;
    window->virtualCursorPosY = ypos;

    if (window->callbacks.cursorPos)
        window->callbacks.cursorPos((RBLXWINwindow*) window, xpos, ypos);
}

// Notifies shared code of a cursor enter/leave event
//
void _rblxwinInputCursorEnter(_RBLXWINwindow* window, RBLXWINbool entered)
{
    if (window->callbacks.cursorEnter)
        window->callbacks.cursorEnter((RBLXWINwindow*) window, entered);
}

// Notifies shared code of files or directories dropped on a window
//
void _rblxwinInputDrop(_RBLXWINwindow* window, int count, const char** paths)
{
    if (window->callbacks.drop)
        window->callbacks.drop((RBLXWINwindow*) window, count, paths);
}

// Notifies shared code of a joystick connection or disconnection
//
void _rblxwinInputJoystick(_RBLXWINjoystick* js, int event)
{
    const int jid = (int) (js - _rblxwin.joysticks);

    if (event == RBLXWIN_CONNECTED)
        js->connected = RBLXWIN_TRUE;
    else if (event == RBLXWIN_DISCONNECTED)
        js->connected = RBLXWIN_FALSE;

    if (_rblxwin.callbacks.joystick)
        _rblxwin.callbacks.joystick(jid, event);
}

// Notifies shared code of the new value of a joystick axis
//
void _rblxwinInputJoystickAxis(_RBLXWINjoystick* js, int axis, float value)
{
    js->axes[axis] = value;
}

// Notifies shared code of the new value of a joystick button
//
void _rblxwinInputJoystickButton(_RBLXWINjoystick* js, int button, char value)
{
    js->buttons[button] = value;
}

// Notifies shared code of the new value of a joystick hat
//
void _rblxwinInputJoystickHat(_RBLXWINjoystick* js, int hat, char value)
{
    const int base = js->buttonCount + hat * 4;

    js->buttons[base + 0] = (value & 0x01) ? RBLXWIN_PRESS : RBLXWIN_RELEASE;
    js->buttons[base + 1] = (value & 0x02) ? RBLXWIN_PRESS : RBLXWIN_RELEASE;
    js->buttons[base + 2] = (value & 0x04) ? RBLXWIN_PRESS : RBLXWIN_RELEASE;
    js->buttons[base + 3] = (value & 0x08) ? RBLXWIN_PRESS : RBLXWIN_RELEASE;

    js->hats[hat] = value;
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Adds the built-in set of gamepad mappings
//
void _rblxwinInitGamepadMappings(void)
{
    int jid;
    size_t i;
    const size_t count = sizeof(_rblxwinDefaultMappings) / sizeof(char*);
    _rblxwin.mappings = calloc(count, sizeof(_RBLXWINmapping));

    for (i = 0;  i < count;  i++)
    {
        if (parseMapping(&_rblxwin.mappings[_rblxwin.mappingCount], _rblxwinDefaultMappings[i]))
            _rblxwin.mappingCount++;
    }

    for (jid = 0;  jid <= RBLXWIN_JOYSTICK_LAST;  jid++)
    {
        _RBLXWINjoystick* js = _rblxwin.joysticks + jid;
        if (js->connected)
            js->mapping = findValidMapping(js);
    }
}

// Returns an available joystick object with arrays and name allocated
//
_RBLXWINjoystick* _rblxwinAllocJoystick(const char* name,
                                  const char* guid,
                                  int axisCount,
                                  int buttonCount,
                                  int hatCount)
{
    int jid;
    _RBLXWINjoystick* js;

    for (jid = 0;  jid <= RBLXWIN_JOYSTICK_LAST;  jid++)
    {
        if (!_rblxwin.joysticks[jid].allocated)
            break;
    }

    if (jid > RBLXWIN_JOYSTICK_LAST)
        return NULL;

    js = _rblxwin.joysticks + jid;
    js->allocated   = RBLXWIN_TRUE;
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
void _rblxwinFreeJoystick(_RBLXWINjoystick* js)
{
    free(js->axes);
    free(js->buttons);
    free(js->hats);
    memset(js, 0, sizeof(_RBLXWINjoystick));
}

// Center the cursor in the content area of the specified window
//
void _rblxwinCenterCursorInContentArea(_RBLXWINwindow* window)
{
    int width, height;

    _rblxwinPlatformGetWindowSize(window, &width, &height);
    _rblxwinPlatformSetCursorPos(window, width / 2.0, height / 2.0);
}


//////////////////////////////////////////////////////////////////////////
//////                        RBLXWIN public API                       //////
//////////////////////////////////////////////////////////////////////////

RBLXWINAPI int rblxwinGetInputMode(RBLXWINwindow* handle, int mode)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(0);

    switch (mode)
    {
        case RBLXWIN_CURSOR:
            return window->cursorMode;
        case RBLXWIN_STICKY_KEYS:
            return window->stickyKeys;
        case RBLXWIN_STICKY_MOUSE_BUTTONS:
            return window->stickyMouseButtons;
        case RBLXWIN_LOCK_KEY_MODS:
            return window->lockKeyMods;
        case RBLXWIN_RAW_MOUSE_MOTION:
            return window->rawMouseMotion;
    }

    _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid input mode 0x%08X", mode);
    return 0;
}

RBLXWINAPI void rblxwinSetInputMode(RBLXWINwindow* handle, int mode, int value)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();

    if (mode == RBLXWIN_CURSOR)
    {
        if (value != RBLXWIN_CURSOR_NORMAL &&
            value != RBLXWIN_CURSOR_HIDDEN &&
            value != RBLXWIN_CURSOR_DISABLED)
        {
            _rblxwinInputError(RBLXWIN_INVALID_ENUM,
                            "Invalid cursor mode 0x%08X",
                            value);
            return;
        }

        if (window->cursorMode == value)
            return;

        window->cursorMode = value;

        _rblxwinPlatformGetCursorPos(window,
                                  &window->virtualCursorPosX,
                                  &window->virtualCursorPosY);
        _rblxwinPlatformSetCursorMode(window, value);
    }
    else if (mode == RBLXWIN_STICKY_KEYS)
    {
        value = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
        if (window->stickyKeys == value)
            return;

        if (!value)
        {
            int i;

            // Release all sticky keys
            for (i = 0;  i <= RBLXWIN_KEY_LAST;  i++)
            {
                if (window->keys[i] == _RBLXWIN_STICK)
                    window->keys[i] = RBLXWIN_RELEASE;
            }
        }

        window->stickyKeys = value;
    }
    else if (mode == RBLXWIN_STICKY_MOUSE_BUTTONS)
    {
        value = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
        if (window->stickyMouseButtons == value)
            return;

        if (!value)
        {
            int i;

            // Release all sticky mouse buttons
            for (i = 0;  i <= RBLXWIN_MOUSE_BUTTON_LAST;  i++)
            {
                if (window->mouseButtons[i] == _RBLXWIN_STICK)
                    window->mouseButtons[i] = RBLXWIN_RELEASE;
            }
        }

        window->stickyMouseButtons = value;
    }
    else if (mode == RBLXWIN_LOCK_KEY_MODS)
    {
        window->lockKeyMods = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
    }
    else if (mode == RBLXWIN_RAW_MOUSE_MOTION)
    {
        if (!_rblxwinPlatformRawMouseMotionSupported())
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "Raw mouse motion is not supported on this system");
            return;
        }

        value = value ? RBLXWIN_TRUE : RBLXWIN_FALSE;
        if (window->rawMouseMotion == value)
            return;

        window->rawMouseMotion = value;
        _rblxwinPlatformSetRawMouseMotion(window, value);
    }
    else
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid input mode 0x%08X", mode);
}

RBLXWINAPI int rblxwinRawMouseMotionSupported(void)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(RBLXWIN_FALSE);
    return _rblxwinPlatformRawMouseMotionSupported();
}

RBLXWINAPI const char* rblxwinGetKeyName(int key, int scancode)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (key != RBLXWIN_KEY_UNKNOWN)
    {
        if (key != RBLXWIN_KEY_KP_EQUAL &&
            (key < RBLXWIN_KEY_KP_0 || key > RBLXWIN_KEY_KP_ADD) &&
            (key < RBLXWIN_KEY_APOSTROPHE || key > RBLXWIN_KEY_WORLD_2))
        {
            return NULL;
        }

        scancode = _rblxwinPlatformGetKeyScancode(key);
    }

    return _rblxwinPlatformGetScancodeName(scancode);
}

RBLXWINAPI int rblxwinGetKeyScancode(int key)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(-1);

    if (key < RBLXWIN_KEY_SPACE || key > RBLXWIN_KEY_LAST)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid key %i", key);
        return RBLXWIN_RELEASE;
    }

    return _rblxwinPlatformGetKeyScancode(key);
}

RBLXWINAPI int rblxwinGetKey(RBLXWINwindow* handle, int key)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(RBLXWIN_RELEASE);

    if (key < RBLXWIN_KEY_SPACE || key > RBLXWIN_KEY_LAST)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid key %i", key);
        return RBLXWIN_RELEASE;
    }

    if (window->keys[key] == _RBLXWIN_STICK)
    {
        // Sticky mode: release key now
        window->keys[key] = RBLXWIN_RELEASE;
        return RBLXWIN_PRESS;
    }

    return (int) window->keys[key];
}

RBLXWINAPI int rblxwinGetMouseButton(RBLXWINwindow* handle, int button)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(RBLXWIN_RELEASE);

    if (button < RBLXWIN_MOUSE_BUTTON_1 || button > RBLXWIN_MOUSE_BUTTON_LAST)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid mouse button %i", button);
        return RBLXWIN_RELEASE;
    }

    if (window->mouseButtons[button] == _RBLXWIN_STICK)
    {
        // Sticky mode: release mouse button now
        window->mouseButtons[button] = RBLXWIN_RELEASE;
        return RBLXWIN_PRESS;
    }

    return (int) window->mouseButtons[button];
}

RBLXWINAPI void rblxwinGetCursorPos(RBLXWINwindow* handle, double* xpos, double* ypos)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;

    _RBLXWIN_REQUIRE_INIT();

    if (window->cursorMode == RBLXWIN_CURSOR_DISABLED)
    {
        if (xpos)
            *xpos = window->virtualCursorPosX;
        if (ypos)
            *ypos = window->virtualCursorPosY;
    }
    else
        _rblxwinPlatformGetCursorPos(window, xpos, ypos);
}

RBLXWINAPI void rblxwinSetCursorPos(RBLXWINwindow* handle, double xpos, double ypos)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();

    if (xpos != xpos || xpos < -DBL_MAX || xpos > DBL_MAX ||
        ypos != ypos || ypos < -DBL_MAX || ypos > DBL_MAX)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                        "Invalid cursor position %f %f",
                        xpos, ypos);
        return;
    }

    if (!_rblxwinPlatformWindowFocused(window))
        return;

    if (window->cursorMode == RBLXWIN_CURSOR_DISABLED)
    {
        // Only update the accumulated position if the cursor is disabled
        window->virtualCursorPosX = xpos;
        window->virtualCursorPosY = ypos;
    }
    else
    {
        // Update system cursor position
        _rblxwinPlatformSetCursorPos(window, xpos, ypos);
    }
}

RBLXWINAPI RBLXWINcursor* rblxwinCreateCursor(const RBLXWINimage* image, int xhot, int yhot)
{
    _RBLXWINcursor* cursor;

    assert(image != NULL);
    assert(image->pixels != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (image->width <= 0 || image->height <= 0)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE, "Invalid image dimensions for cursor");
        return NULL;
    }

    cursor = calloc(1, sizeof(_RBLXWINcursor));
    cursor->next = _rblxwin.cursorListHead;
    _rblxwin.cursorListHead = cursor;

    if (!_rblxwinPlatformCreateCursor(cursor, image, xhot, yhot))
    {
        rblxwinDestroyCursor((RBLXWINcursor*) cursor);
        return NULL;
    }

    return (RBLXWINcursor*) cursor;
}

RBLXWINAPI RBLXWINcursor* rblxwinCreateStandardCursor(int shape)
{
    _RBLXWINcursor* cursor;

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (shape != RBLXWIN_ARROW_CURSOR &&
        shape != RBLXWIN_IBEAM_CURSOR &&
        shape != RBLXWIN_CROSSHAIR_CURSOR &&
        shape != RBLXWIN_HAND_CURSOR &&
        shape != RBLXWIN_HRESIZE_CURSOR &&
        shape != RBLXWIN_VRESIZE_CURSOR)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid standard cursor 0x%08X", shape);
        return NULL;
    }

    cursor = calloc(1, sizeof(_RBLXWINcursor));
    cursor->next = _rblxwin.cursorListHead;
    _rblxwin.cursorListHead = cursor;

    if (!_rblxwinPlatformCreateStandardCursor(cursor, shape))
    {
        rblxwinDestroyCursor((RBLXWINcursor*) cursor);
        return NULL;
    }

    return (RBLXWINcursor*) cursor;
}

RBLXWINAPI void rblxwinDestroyCursor(RBLXWINcursor* handle)
{
    _RBLXWINcursor* cursor = (_RBLXWINcursor*) handle;

    _RBLXWIN_REQUIRE_INIT();

    if (cursor == NULL)
        return;

    // Make sure the cursor is not being used by any window
    {
        _RBLXWINwindow* window;

        for (window = _rblxwin.windowListHead;  window;  window = window->next)
        {
            if (window->cursor == cursor)
                rblxwinSetCursor((RBLXWINwindow*) window, NULL);
        }
    }

    _rblxwinPlatformDestroyCursor(cursor);

    // Unlink cursor from global linked list
    {
        _RBLXWINcursor** prev = &_rblxwin.cursorListHead;

        while (*prev != cursor)
            prev = &((*prev)->next);

        *prev = cursor->next;
    }

    free(cursor);
}

RBLXWINAPI void rblxwinSetCursor(RBLXWINwindow* windowHandle, RBLXWINcursor* cursorHandle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) windowHandle;
    _RBLXWINcursor* cursor = (_RBLXWINcursor*) cursorHandle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();

    window->cursor = cursor;

    _rblxwinPlatformSetCursor(window, cursor);
}

RBLXWINAPI RBLXWINkeyfun rblxwinSetKeyCallback(RBLXWINwindow* handle, RBLXWINkeyfun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.key, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINcharfun rblxwinSetCharCallback(RBLXWINwindow* handle, RBLXWINcharfun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.character, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINcharmodsfun rblxwinSetCharModsCallback(RBLXWINwindow* handle, RBLXWINcharmodsfun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.charmods, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINmousebuttonfun rblxwinSetMouseButtonCallback(RBLXWINwindow* handle,
                                                      RBLXWINmousebuttonfun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.mouseButton, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINcursorposfun rblxwinSetCursorPosCallback(RBLXWINwindow* handle,
                                                  RBLXWINcursorposfun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.cursorPos, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINcursorenterfun rblxwinSetCursorEnterCallback(RBLXWINwindow* handle,
                                                      RBLXWINcursorenterfun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.cursorEnter, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINscrollfun rblxwinSetScrollCallback(RBLXWINwindow* handle,
                                            RBLXWINscrollfun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.scroll, cbfun);
    return cbfun;
}

RBLXWINAPI RBLXWINdropfun rblxwinSetDropCallback(RBLXWINwindow* handle, RBLXWINdropfun cbfun)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(window->callbacks.drop, cbfun);
    return cbfun;
}

RBLXWINAPI int rblxwinJoystickPresent(int jid)
{
    _RBLXWINjoystick* js;

    assert(jid >= RBLXWIN_JOYSTICK_1);
    assert(jid <= RBLXWIN_JOYSTICK_LAST);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(RBLXWIN_FALSE);

    if (jid < 0 || jid > RBLXWIN_JOYSTICK_LAST)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return RBLXWIN_FALSE;
    }

    js = _rblxwin.joysticks + jid;
    if (!js->connected)
        return RBLXWIN_FALSE;

    return _rblxwinPlatformPollJoystick(js, _RBLXWIN_POLL_PRESENCE);
}

RBLXWINAPI const float* rblxwinGetJoystickAxes(int jid, int* count)
{
    _RBLXWINjoystick* js;

    assert(jid >= RBLXWIN_JOYSTICK_1);
    assert(jid <= RBLXWIN_JOYSTICK_LAST);
    assert(count != NULL);

    *count = 0;

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > RBLXWIN_JOYSTICK_LAST)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    js = _rblxwin.joysticks + jid;
    if (!js->connected)
        return NULL;

    if (!_rblxwinPlatformPollJoystick(js, _RBLXWIN_POLL_AXES))
        return NULL;

    *count = js->axisCount;
    return js->axes;
}

RBLXWINAPI const unsigned char* rblxwinGetJoystickButtons(int jid, int* count)
{
    _RBLXWINjoystick* js;

    assert(jid >= RBLXWIN_JOYSTICK_1);
    assert(jid <= RBLXWIN_JOYSTICK_LAST);
    assert(count != NULL);

    *count = 0;

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > RBLXWIN_JOYSTICK_LAST)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    js = _rblxwin.joysticks + jid;
    if (!js->connected)
        return NULL;

    if (!_rblxwinPlatformPollJoystick(js, _RBLXWIN_POLL_BUTTONS))
        return NULL;

    if (_rblxwin.hints.init.hatButtons)
        *count = js->buttonCount + js->hatCount * 4;
    else
        *count = js->buttonCount;

    return js->buttons;
}

RBLXWINAPI const unsigned char* rblxwinGetJoystickHats(int jid, int* count)
{
    _RBLXWINjoystick* js;

    assert(jid >= RBLXWIN_JOYSTICK_1);
    assert(jid <= RBLXWIN_JOYSTICK_LAST);
    assert(count != NULL);

    *count = 0;

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > RBLXWIN_JOYSTICK_LAST)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    js = _rblxwin.joysticks + jid;
    if (!js->connected)
        return NULL;

    if (!_rblxwinPlatformPollJoystick(js, _RBLXWIN_POLL_BUTTONS))
        return NULL;

    *count = js->hatCount;
    return js->hats;
}

RBLXWINAPI const char* rblxwinGetJoystickName(int jid)
{
    _RBLXWINjoystick* js;

    assert(jid >= RBLXWIN_JOYSTICK_1);
    assert(jid <= RBLXWIN_JOYSTICK_LAST);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > RBLXWIN_JOYSTICK_LAST)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    js = _rblxwin.joysticks + jid;
    if (!js->connected)
        return NULL;

    if (!_rblxwinPlatformPollJoystick(js, _RBLXWIN_POLL_PRESENCE))
        return NULL;

    return js->name;
}

RBLXWINAPI const char* rblxwinGetJoystickGUID(int jid)
{
    _RBLXWINjoystick* js;

    assert(jid >= RBLXWIN_JOYSTICK_1);
    assert(jid <= RBLXWIN_JOYSTICK_LAST);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > RBLXWIN_JOYSTICK_LAST)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    js = _rblxwin.joysticks + jid;
    if (!js->connected)
        return NULL;

    if (!_rblxwinPlatformPollJoystick(js, _RBLXWIN_POLL_PRESENCE))
        return NULL;

    return js->guid;
}

RBLXWINAPI void rblxwinSetJoystickUserPointer(int jid, void* pointer)
{
    _RBLXWINjoystick* js;

    assert(jid >= RBLXWIN_JOYSTICK_1);
    assert(jid <= RBLXWIN_JOYSTICK_LAST);

    _RBLXWIN_REQUIRE_INIT();

    js = _rblxwin.joysticks + jid;
    if (!js->allocated)
        return;

    js->userPointer = pointer;
}

RBLXWINAPI void* rblxwinGetJoystickUserPointer(int jid)
{
    _RBLXWINjoystick* js;

    assert(jid >= RBLXWIN_JOYSTICK_1);
    assert(jid <= RBLXWIN_JOYSTICK_LAST);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    js = _rblxwin.joysticks + jid;
    if (!js->allocated)
        return NULL;

    return js->userPointer;
}

RBLXWINAPI RBLXWINjoystickfun rblxwinSetJoystickCallback(RBLXWINjoystickfun cbfun)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    _RBLXWIN_SWAP_POINTERS(_rblxwin.callbacks.joystick, cbfun);
    return cbfun;
}

RBLXWINAPI int rblxwinUpdateGamepadMappings(const char* string)
{
    int jid;
    const char* c = string;

    assert(string != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(RBLXWIN_FALSE);

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
                _RBLXWINmapping mapping = {{0}};

                memcpy(line, c, length);
                line[length] = '\0';

                if (parseMapping(&mapping, line))
                {
                    _RBLXWINmapping* previous = findMapping(mapping.guid);
                    if (previous)
                        *previous = mapping;
                    else
                    {
                        _rblxwin.mappingCount++;
                        _rblxwin.mappings =
                            realloc(_rblxwin.mappings,
                                    sizeof(_RBLXWINmapping) * _rblxwin.mappingCount);
                        _rblxwin.mappings[_rblxwin.mappingCount - 1] = mapping;
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

    for (jid = 0;  jid <= RBLXWIN_JOYSTICK_LAST;  jid++)
    {
        _RBLXWINjoystick* js = _rblxwin.joysticks + jid;
        if (js->connected)
            js->mapping = findValidMapping(js);
    }

    return RBLXWIN_TRUE;
}

RBLXWINAPI int rblxwinJoystickIsGamepad(int jid)
{
    _RBLXWINjoystick* js;

    assert(jid >= RBLXWIN_JOYSTICK_1);
    assert(jid <= RBLXWIN_JOYSTICK_LAST);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(RBLXWIN_FALSE);

    if (jid < 0 || jid > RBLXWIN_JOYSTICK_LAST)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return RBLXWIN_FALSE;
    }

    js = _rblxwin.joysticks + jid;
    if (!js->connected)
        return RBLXWIN_FALSE;

    if (!_rblxwinPlatformPollJoystick(js, _RBLXWIN_POLL_PRESENCE))
        return RBLXWIN_FALSE;

    return js->mapping != NULL;
}

RBLXWINAPI const char* rblxwinGetGamepadName(int jid)
{
    _RBLXWINjoystick* js;

    assert(jid >= RBLXWIN_JOYSTICK_1);
    assert(jid <= RBLXWIN_JOYSTICK_LAST);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > RBLXWIN_JOYSTICK_LAST)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    js = _rblxwin.joysticks + jid;
    if (!js->connected)
        return NULL;

    if (!_rblxwinPlatformPollJoystick(js, _RBLXWIN_POLL_PRESENCE))
        return NULL;

    if (!js->mapping)
        return NULL;

    return js->mapping->name;
}

RBLXWINAPI int rblxwinGetGamepadState(int jid, RBLXWINgamepadstate* state)
{
    int i;
    _RBLXWINjoystick* js;

    assert(jid >= RBLXWIN_JOYSTICK_1);
    assert(jid <= RBLXWIN_JOYSTICK_LAST);
    assert(state != NULL);

    memset(state, 0, sizeof(RBLXWINgamepadstate));

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(RBLXWIN_FALSE);

    if (jid < 0 || jid > RBLXWIN_JOYSTICK_LAST)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return RBLXWIN_FALSE;
    }

    js = _rblxwin.joysticks + jid;
    if (!js->connected)
        return RBLXWIN_FALSE;

    if (!_rblxwinPlatformPollJoystick(js, _RBLXWIN_POLL_ALL))
        return RBLXWIN_FALSE;

    if (!js->mapping)
        return RBLXWIN_FALSE;

    for (i = 0;  i <= RBLXWIN_GAMEPAD_BUTTON_LAST;  i++)
    {
        const _RBLXWINmapelement* e = js->mapping->buttons + i;
        if (e->type == _RBLXWIN_JOYSTICK_AXIS)
        {
            const float value = js->axes[e->index] * e->axisScale + e->axisOffset;
            // HACK: This should be baked into the value transform
            // TODO: Bake into transform when implementing output modifiers
            if (e->axisOffset < 0 || (e->axisOffset == 0 && e->axisScale > 0))
            {
                if (value >= 0.f)
                    state->buttons[i] = RBLXWIN_PRESS;
            }
            else
            {
                if (value <= 0.f)
                    state->buttons[i] = RBLXWIN_PRESS;
            }
        }
        else if (e->type == _RBLXWIN_JOYSTICK_HATBIT)
        {
            const unsigned int hat = e->index >> 4;
            const unsigned int bit = e->index & 0xf;
            if (js->hats[hat] & bit)
                state->buttons[i] = RBLXWIN_PRESS;
        }
        else if (e->type == _RBLXWIN_JOYSTICK_BUTTON)
            state->buttons[i] = js->buttons[e->index];
    }

    for (i = 0;  i <= RBLXWIN_GAMEPAD_AXIS_LAST;  i++)
    {
        const _RBLXWINmapelement* e = js->mapping->axes + i;
        if (e->type == _RBLXWIN_JOYSTICK_AXIS)
        {
            const float value = js->axes[e->index] * e->axisScale + e->axisOffset;
            state->axes[i] = _rblxwin_fminf(_rblxwin_fmaxf(value, -1.f), 1.f);
        }
        else if (e->type == _RBLXWIN_JOYSTICK_HATBIT)
        {
            const unsigned int hat = e->index >> 4;
            const unsigned int bit = e->index & 0xf;
            if (js->hats[hat] & bit)
                state->axes[i] = 1.f;
            else
                state->axes[i] = -1.f;
        }
        else if (e->type == _RBLXWIN_JOYSTICK_BUTTON)
            state->axes[i] = js->buttons[e->index] * 2.f - 1.f;
    }

    return RBLXWIN_TRUE;
}

RBLXWINAPI void rblxwinSetClipboardString(RBLXWINwindow* handle, const char* string)
{
    assert(string != NULL);

    _RBLXWIN_REQUIRE_INIT();
    _rblxwinPlatformSetClipboardString(string);
}

RBLXWINAPI const char* rblxwinGetClipboardString(RBLXWINwindow* handle)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return _rblxwinPlatformGetClipboardString();
}

RBLXWINAPI double rblxwinGetTime(void)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(0.0);
    return (double) (_rblxwinPlatformGetTimerValue() - _rblxwin.timer.offset) /
        _rblxwinPlatformGetTimerFrequency();
}

RBLXWINAPI void rblxwinSetTime(double time)
{
    _RBLXWIN_REQUIRE_INIT();

    if (time != time || time < 0.0 || time > 18446744073.0)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE, "Invalid time %f", time);
        return;
    }

    _rblxwin.timer.offset = _rblxwinPlatformGetTimerValue() -
        (uint64_t) (time * _rblxwinPlatformGetTimerFrequency());
}

RBLXWINAPI uint64_t rblxwinGetTimerValue(void)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(0);
    return _rblxwinPlatformGetTimerValue();
}

RBLXWINAPI uint64_t rblxwinGetTimerFrequency(void)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(0);
    return _rblxwinPlatformGetTimerFrequency();
}

