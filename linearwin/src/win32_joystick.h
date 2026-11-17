//========================================================================
// LINEARWIN 3.3 Win32 - www.linearwin.org
//------------------------------------------------------------------------
// Copyright (c) 2006-2017 Camilla Löwy <elmindreda@linearwin.org>
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

#define _LINEARWIN_PLATFORM_JOYSTICK_STATE         _LINEARWINjoystickWin32 win32
#define _LINEARWIN_PLATFORM_LIBRARY_JOYSTICK_STATE struct { int dummyLibraryJoystick; }

#define _LINEARWIN_PLATFORM_MAPPING_NAME "Windows"
#define LINEARWIN_BUILD_WIN32_MAPPINGS

// Joystick element (axis, button or slider)
//
typedef struct _LINEARWINjoyobjectWin32
{
    int                     offset;
    int                     type;
} _LINEARWINjoyobjectWin32;

// Win32-specific per-joystick data
//
typedef struct _LINEARWINjoystickWin32
{
    _LINEARWINjoyobjectWin32*    objects;
    int                     objectCount;
    IDirectInputDevice8W*   device;
    DWORD                   index;
    GUID                    guid;
} _LINEARWINjoystickWin32;


void _linearwinInitJoysticksWin32(void);
void _linearwinTerminateJoysticksWin32(void);
void _linearwinDetectJoystickConnectionWin32(void);
void _linearwinDetectJoystickDisconnectionWin32(void);

