//========================================================================
// LINEARWIN 3.3 Win32 - www.linearwin.org
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

#include <stdlib.h>
#include <malloc.h>

static const GUID _linearwin_GUID_DEVINTERFACE_HID =
    {0x4d1e55b2,0xf16f,0x11cf,{0x88,0xcb,0x00,0x11,0x11,0x00,0x00,0x30}};

#define GUID_DEVINTERFACE_HID _linearwin_GUID_DEVINTERFACE_HID

#if defined(_LINEARWIN_USE_HYBRID_HPG) || defined(_LINEARWIN_USE_OPTIMUS_HPG)

#if defined(_LINEARWIN_BUILD_DLL)
 #pragma message("These symbols must be exported by the executable and have no effect in a DLL")
#endif

// Executables (but not DLLs) exporting this symbol with this value will be
// automatically directed to the high-performance GPU on Nvidia Optimus systems
// with up-to-date drivers
//
__declspec(dllexport) DWORD NvOptimusEnablement = 1;

// Executables (but not DLLs) exporting this symbol with this value will be
// automatically directed to the high-performance GPU on AMD PowerXpress systems
// with up-to-date drivers
//
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#endif // _LINEARWIN_USE_HYBRID_HPG

#if defined(_LINEARWIN_BUILD_DLL)

// LINEARWIN DLL entry point
//
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    return TRUE;
}

#endif // _LINEARWIN_BUILD_DLL

// Load necessary libraries (DLLs)
//
static LINEARWINbool loadLibraries(void)
{
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (const WCHAR*) &_linearwin,
                            (HMODULE*) &_linearwin.win32.instance))
    {
        _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                             "Win32: Failed to retrieve own module handle");
        return LINEARWIN_FALSE;
    }

    _linearwin.win32.user32.instance = LoadLibraryA("user32.dll");
    if (!_linearwin.win32.user32.instance)
    {
        _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                             "Win32: Failed to load user32.dll");
        return LINEARWIN_FALSE;
    }

    _linearwin.win32.user32.SetProcessDPIAware_ = (PFN_SetProcessDPIAware)
        GetProcAddress(_linearwin.win32.user32.instance, "SetProcessDPIAware");
    _linearwin.win32.user32.ChangeWindowMessageFilterEx_ = (PFN_ChangeWindowMessageFilterEx)
        GetProcAddress(_linearwin.win32.user32.instance, "ChangeWindowMessageFilterEx");
    _linearwin.win32.user32.EnableNonClientDpiScaling_ = (PFN_EnableNonClientDpiScaling)
        GetProcAddress(_linearwin.win32.user32.instance, "EnableNonClientDpiScaling");
    _linearwin.win32.user32.SetProcessDpiAwarenessContext_ = (PFN_SetProcessDpiAwarenessContext)
        GetProcAddress(_linearwin.win32.user32.instance, "SetProcessDpiAwarenessContext");
    _linearwin.win32.user32.GetDpiForWindow_ = (PFN_GetDpiForWindow)
        GetProcAddress(_linearwin.win32.user32.instance, "GetDpiForWindow");
    _linearwin.win32.user32.AdjustWindowRectExForDpi_ = (PFN_AdjustWindowRectExForDpi)
        GetProcAddress(_linearwin.win32.user32.instance, "AdjustWindowRectExForDpi");
    _linearwin.win32.user32.GetSystemMetricsForDpi_ = (PFN_GetSystemMetricsForDpi)
        GetProcAddress(_linearwin.win32.user32.instance, "GetSystemMetricsForDpi");

    _linearwin.win32.dinput8.instance = LoadLibraryA("dinput8.dll");
    if (_linearwin.win32.dinput8.instance)
    {
        _linearwin.win32.dinput8.Create = (PFN_DirectInput8Create)
            GetProcAddress(_linearwin.win32.dinput8.instance, "DirectInput8Create");
    }

    {
        int i;
        const char* names[] =
        {
            "xinput1_4.dll",
            "xinput1_3.dll",
            "xinput9_1_0.dll",
            "xinput1_2.dll",
            "xinput1_1.dll",
            NULL
        };

        for (i = 0;  names[i];  i++)
        {
            _linearwin.win32.xinput.instance = LoadLibraryA(names[i]);
            if (_linearwin.win32.xinput.instance)
            {
                _linearwin.win32.xinput.GetCapabilities = (PFN_XInputGetCapabilities)
                    GetProcAddress(_linearwin.win32.xinput.instance, "XInputGetCapabilities");
                _linearwin.win32.xinput.GetState = (PFN_XInputGetState)
                    GetProcAddress(_linearwin.win32.xinput.instance, "XInputGetState");

                break;
            }
        }
    }

    _linearwin.win32.dwmapi.instance = LoadLibraryA("dwmapi.dll");
    if (_linearwin.win32.dwmapi.instance)
    {
        _linearwin.win32.dwmapi.IsCompositionEnabled = (PFN_DwmIsCompositionEnabled)
            GetProcAddress(_linearwin.win32.dwmapi.instance, "DwmIsCompositionEnabled");
        _linearwin.win32.dwmapi.Flush = (PFN_DwmFlush)
            GetProcAddress(_linearwin.win32.dwmapi.instance, "DwmFlush");
        _linearwin.win32.dwmapi.EnableBlurBehindWindow = (PFN_DwmEnableBlurBehindWindow)
            GetProcAddress(_linearwin.win32.dwmapi.instance, "DwmEnableBlurBehindWindow");
        _linearwin.win32.dwmapi.GetColorizationColor = (PFN_DwmGetColorizationColor)
            GetProcAddress(_linearwin.win32.dwmapi.instance, "DwmGetColorizationColor");
    }

    _linearwin.win32.shcore.instance = LoadLibraryA("shcore.dll");
    if (_linearwin.win32.shcore.instance)
    {
        _linearwin.win32.shcore.SetProcessDpiAwareness_ = (PFN_SetProcessDpiAwareness)
            GetProcAddress(_linearwin.win32.shcore.instance, "SetProcessDpiAwareness");
        _linearwin.win32.shcore.GetDpiForMonitor_ = (PFN_GetDpiForMonitor)
            GetProcAddress(_linearwin.win32.shcore.instance, "GetDpiForMonitor");
    }

    _linearwin.win32.ntdll.instance = LoadLibraryA("ntdll.dll");
    if (_linearwin.win32.ntdll.instance)
    {
        _linearwin.win32.ntdll.RtlVerifyVersionInfo_ = (PFN_RtlVerifyVersionInfo)
            GetProcAddress(_linearwin.win32.ntdll.instance, "RtlVerifyVersionInfo");
    }

    return LINEARWIN_TRUE;
}

// Unload used libraries (DLLs)
//
static void freeLibraries(void)
{
    if (_linearwin.win32.xinput.instance)
        FreeLibrary(_linearwin.win32.xinput.instance);

    if (_linearwin.win32.dinput8.instance)
        FreeLibrary(_linearwin.win32.dinput8.instance);

    if (_linearwin.win32.user32.instance)
        FreeLibrary(_linearwin.win32.user32.instance);

    if (_linearwin.win32.dwmapi.instance)
        FreeLibrary(_linearwin.win32.dwmapi.instance);

    if (_linearwin.win32.shcore.instance)
        FreeLibrary(_linearwin.win32.shcore.instance);

    if (_linearwin.win32.ntdll.instance)
        FreeLibrary(_linearwin.win32.ntdll.instance);
}

// Create key code translation tables
//
static void createKeyTables(void)
{
    int scancode;

    memset(_linearwin.win32.keycodes, -1, sizeof(_linearwin.win32.keycodes));
    memset(_linearwin.win32.scancodes, -1, sizeof(_linearwin.win32.scancodes));

    _linearwin.win32.keycodes[0x00B] = LINEARWIN_KEY_0;
    _linearwin.win32.keycodes[0x002] = LINEARWIN_KEY_1;
    _linearwin.win32.keycodes[0x003] = LINEARWIN_KEY_2;
    _linearwin.win32.keycodes[0x004] = LINEARWIN_KEY_3;
    _linearwin.win32.keycodes[0x005] = LINEARWIN_KEY_4;
    _linearwin.win32.keycodes[0x006] = LINEARWIN_KEY_5;
    _linearwin.win32.keycodes[0x007] = LINEARWIN_KEY_6;
    _linearwin.win32.keycodes[0x008] = LINEARWIN_KEY_7;
    _linearwin.win32.keycodes[0x009] = LINEARWIN_KEY_8;
    _linearwin.win32.keycodes[0x00A] = LINEARWIN_KEY_9;
    _linearwin.win32.keycodes[0x01E] = LINEARWIN_KEY_A;
    _linearwin.win32.keycodes[0x030] = LINEARWIN_KEY_B;
    _linearwin.win32.keycodes[0x02E] = LINEARWIN_KEY_C;
    _linearwin.win32.keycodes[0x020] = LINEARWIN_KEY_D;
    _linearwin.win32.keycodes[0x012] = LINEARWIN_KEY_E;
    _linearwin.win32.keycodes[0x021] = LINEARWIN_KEY_F;
    _linearwin.win32.keycodes[0x022] = LINEARWIN_KEY_G;
    _linearwin.win32.keycodes[0x023] = LINEARWIN_KEY_H;
    _linearwin.win32.keycodes[0x017] = LINEARWIN_KEY_I;
    _linearwin.win32.keycodes[0x024] = LINEARWIN_KEY_J;
    _linearwin.win32.keycodes[0x025] = LINEARWIN_KEY_K;
    _linearwin.win32.keycodes[0x026] = LINEARWIN_KEY_L;
    _linearwin.win32.keycodes[0x032] = LINEARWIN_KEY_M;
    _linearwin.win32.keycodes[0x031] = LINEARWIN_KEY_N;
    _linearwin.win32.keycodes[0x018] = LINEARWIN_KEY_O;
    _linearwin.win32.keycodes[0x019] = LINEARWIN_KEY_P;
    _linearwin.win32.keycodes[0x010] = LINEARWIN_KEY_Q;
    _linearwin.win32.keycodes[0x013] = LINEARWIN_KEY_R;
    _linearwin.win32.keycodes[0x01F] = LINEARWIN_KEY_S;
    _linearwin.win32.keycodes[0x014] = LINEARWIN_KEY_T;
    _linearwin.win32.keycodes[0x016] = LINEARWIN_KEY_U;
    _linearwin.win32.keycodes[0x02F] = LINEARWIN_KEY_V;
    _linearwin.win32.keycodes[0x011] = LINEARWIN_KEY_W;
    _linearwin.win32.keycodes[0x02D] = LINEARWIN_KEY_X;
    _linearwin.win32.keycodes[0x015] = LINEARWIN_KEY_Y;
    _linearwin.win32.keycodes[0x02C] = LINEARWIN_KEY_Z;

    _linearwin.win32.keycodes[0x028] = LINEARWIN_KEY_APOSTROPHE;
    _linearwin.win32.keycodes[0x02B] = LINEARWIN_KEY_BACKSLASH;
    _linearwin.win32.keycodes[0x033] = LINEARWIN_KEY_COMMA;
    _linearwin.win32.keycodes[0x00D] = LINEARWIN_KEY_EQUAL;
    _linearwin.win32.keycodes[0x029] = LINEARWIN_KEY_GRAVE_ACCENT;
    _linearwin.win32.keycodes[0x01A] = LINEARWIN_KEY_LEFT_BRACKET;
    _linearwin.win32.keycodes[0x00C] = LINEARWIN_KEY_MINUS;
    _linearwin.win32.keycodes[0x034] = LINEARWIN_KEY_PERIOD;
    _linearwin.win32.keycodes[0x01B] = LINEARWIN_KEY_RIGHT_BRACKET;
    _linearwin.win32.keycodes[0x027] = LINEARWIN_KEY_SEMICOLON;
    _linearwin.win32.keycodes[0x035] = LINEARWIN_KEY_SLASH;
    _linearwin.win32.keycodes[0x056] = LINEARWIN_KEY_WORLD_2;

    _linearwin.win32.keycodes[0x00E] = LINEARWIN_KEY_BACKSPACE;
    _linearwin.win32.keycodes[0x153] = LINEARWIN_KEY_DELETE;
    _linearwin.win32.keycodes[0x14F] = LINEARWIN_KEY_END;
    _linearwin.win32.keycodes[0x01C] = LINEARWIN_KEY_ENTER;
    _linearwin.win32.keycodes[0x001] = LINEARWIN_KEY_ESCAPE;
    _linearwin.win32.keycodes[0x147] = LINEARWIN_KEY_HOME;
    _linearwin.win32.keycodes[0x152] = LINEARWIN_KEY_INSERT;
    _linearwin.win32.keycodes[0x15D] = LINEARWIN_KEY_MENU;
    _linearwin.win32.keycodes[0x151] = LINEARWIN_KEY_PAGE_DOWN;
    _linearwin.win32.keycodes[0x149] = LINEARWIN_KEY_PAGE_UP;
    _linearwin.win32.keycodes[0x045] = LINEARWIN_KEY_PAUSE;
    _linearwin.win32.keycodes[0x039] = LINEARWIN_KEY_SPACE;
    _linearwin.win32.keycodes[0x00F] = LINEARWIN_KEY_TAB;
    _linearwin.win32.keycodes[0x03A] = LINEARWIN_KEY_CAPS_LOCK;
    _linearwin.win32.keycodes[0x145] = LINEARWIN_KEY_NUM_LOCK;
    _linearwin.win32.keycodes[0x046] = LINEARWIN_KEY_SCROLL_LOCK;
    _linearwin.win32.keycodes[0x03B] = LINEARWIN_KEY_F1;
    _linearwin.win32.keycodes[0x03C] = LINEARWIN_KEY_F2;
    _linearwin.win32.keycodes[0x03D] = LINEARWIN_KEY_F3;
    _linearwin.win32.keycodes[0x03E] = LINEARWIN_KEY_F4;
    _linearwin.win32.keycodes[0x03F] = LINEARWIN_KEY_F5;
    _linearwin.win32.keycodes[0x040] = LINEARWIN_KEY_F6;
    _linearwin.win32.keycodes[0x041] = LINEARWIN_KEY_F7;
    _linearwin.win32.keycodes[0x042] = LINEARWIN_KEY_F8;
    _linearwin.win32.keycodes[0x043] = LINEARWIN_KEY_F9;
    _linearwin.win32.keycodes[0x044] = LINEARWIN_KEY_F10;
    _linearwin.win32.keycodes[0x057] = LINEARWIN_KEY_F11;
    _linearwin.win32.keycodes[0x058] = LINEARWIN_KEY_F12;
    _linearwin.win32.keycodes[0x064] = LINEARWIN_KEY_F13;
    _linearwin.win32.keycodes[0x065] = LINEARWIN_KEY_F14;
    _linearwin.win32.keycodes[0x066] = LINEARWIN_KEY_F15;
    _linearwin.win32.keycodes[0x067] = LINEARWIN_KEY_F16;
    _linearwin.win32.keycodes[0x068] = LINEARWIN_KEY_F17;
    _linearwin.win32.keycodes[0x069] = LINEARWIN_KEY_F18;
    _linearwin.win32.keycodes[0x06A] = LINEARWIN_KEY_F19;
    _linearwin.win32.keycodes[0x06B] = LINEARWIN_KEY_F20;
    _linearwin.win32.keycodes[0x06C] = LINEARWIN_KEY_F21;
    _linearwin.win32.keycodes[0x06D] = LINEARWIN_KEY_F22;
    _linearwin.win32.keycodes[0x06E] = LINEARWIN_KEY_F23;
    _linearwin.win32.keycodes[0x076] = LINEARWIN_KEY_F24;
    _linearwin.win32.keycodes[0x038] = LINEARWIN_KEY_LEFT_ALT;
    _linearwin.win32.keycodes[0x01D] = LINEARWIN_KEY_LEFT_CONTROL;
    _linearwin.win32.keycodes[0x02A] = LINEARWIN_KEY_LEFT_SHIFT;
    _linearwin.win32.keycodes[0x15B] = LINEARWIN_KEY_LEFT_SUPER;
    _linearwin.win32.keycodes[0x137] = LINEARWIN_KEY_PRINT_SCREEN;
    _linearwin.win32.keycodes[0x138] = LINEARWIN_KEY_RIGHT_ALT;
    _linearwin.win32.keycodes[0x11D] = LINEARWIN_KEY_RIGHT_CONTROL;
    _linearwin.win32.keycodes[0x036] = LINEARWIN_KEY_RIGHT_SHIFT;
    _linearwin.win32.keycodes[0x15C] = LINEARWIN_KEY_RIGHT_SUPER;
    _linearwin.win32.keycodes[0x150] = LINEARWIN_KEY_DOWN;
    _linearwin.win32.keycodes[0x14B] = LINEARWIN_KEY_LEFT;
    _linearwin.win32.keycodes[0x14D] = LINEARWIN_KEY_RIGHT;
    _linearwin.win32.keycodes[0x148] = LINEARWIN_KEY_UP;

    _linearwin.win32.keycodes[0x052] = LINEARWIN_KEY_KP_0;
    _linearwin.win32.keycodes[0x04F] = LINEARWIN_KEY_KP_1;
    _linearwin.win32.keycodes[0x050] = LINEARWIN_KEY_KP_2;
    _linearwin.win32.keycodes[0x051] = LINEARWIN_KEY_KP_3;
    _linearwin.win32.keycodes[0x04B] = LINEARWIN_KEY_KP_4;
    _linearwin.win32.keycodes[0x04C] = LINEARWIN_KEY_KP_5;
    _linearwin.win32.keycodes[0x04D] = LINEARWIN_KEY_KP_6;
    _linearwin.win32.keycodes[0x047] = LINEARWIN_KEY_KP_7;
    _linearwin.win32.keycodes[0x048] = LINEARWIN_KEY_KP_8;
    _linearwin.win32.keycodes[0x049] = LINEARWIN_KEY_KP_9;
    _linearwin.win32.keycodes[0x04E] = LINEARWIN_KEY_KP_ADD;
    _linearwin.win32.keycodes[0x053] = LINEARWIN_KEY_KP_DECIMAL;
    _linearwin.win32.keycodes[0x135] = LINEARWIN_KEY_KP_DIVIDE;
    _linearwin.win32.keycodes[0x11C] = LINEARWIN_KEY_KP_ENTER;
    _linearwin.win32.keycodes[0x059] = LINEARWIN_KEY_KP_EQUAL;
    _linearwin.win32.keycodes[0x037] = LINEARWIN_KEY_KP_MULTIPLY;
    _linearwin.win32.keycodes[0x04A] = LINEARWIN_KEY_KP_SUBTRACT;

    for (scancode = 0;  scancode < 512;  scancode++)
    {
        if (_linearwin.win32.keycodes[scancode] > 0)
            _linearwin.win32.scancodes[_linearwin.win32.keycodes[scancode]] = scancode;
    }
}

// Creates a dummy window for behind-the-scenes work
//
static LINEARWINbool createHelperWindow(void)
{
    MSG msg;

    _linearwin.win32.helperWindowHandle =
        CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
                        _LINEARWIN_WNDCLASSNAME,
                        L"LINEARWIN message window",
                        WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                        0, 0, 1, 1,
                        NULL, NULL,
                        _linearwin.win32.instance,
                        NULL);

    if (!_linearwin.win32.helperWindowHandle)
    {
        _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                             "Win32: Failed to create helper window");
        return LINEARWIN_FALSE;
    }

    // HACK: The command to the first ShowWindow call is ignored if the parent
    //       process passed along a STARTUPINFO, so clear that with a no-op call
    ShowWindow(_linearwin.win32.helperWindowHandle, SW_HIDE);

    // Register for HID device notifications
    {
        DEV_BROADCAST_DEVICEINTERFACE_W dbi;
        ZeroMemory(&dbi, sizeof(dbi));
        dbi.dbcc_size = sizeof(dbi);
        dbi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        dbi.dbcc_classguid = GUID_DEVINTERFACE_HID;

        _linearwin.win32.deviceNotificationHandle =
            RegisterDeviceNotificationW(_linearwin.win32.helperWindowHandle,
                                        (DEV_BROADCAST_HDR*) &dbi,
                                        DEVICE_NOTIFY_WINDOW_HANDLE);
    }

    while (PeekMessageW(&msg, _linearwin.win32.helperWindowHandle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

   return LINEARWIN_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Returns a wide string version of the specified UTF-8 string
//
WCHAR* _linearwinCreateWideStringFromUTF8Win32(const char* source)
{
    WCHAR* target;
    int count;

    count = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
    if (!count)
    {
        _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                             "Win32: Failed to convert string from UTF-8");
        return NULL;
    }

    target = calloc(count, sizeof(WCHAR));

    if (!MultiByteToWideChar(CP_UTF8, 0, source, -1, target, count))
    {
        _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                             "Win32: Failed to convert string from UTF-8");
        free(target);
        return NULL;
    }

    return target;
}

// Returns a UTF-8 string version of the specified wide string
//
char* _linearwinCreateUTF8FromWideStringWin32(const WCHAR* source)
{
    char* target;
    int size;

    size = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
    if (!size)
    {
        _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                             "Win32: Failed to convert string to UTF-8");
        return NULL;
    }

    target = calloc(size, 1);

    if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, size, NULL, NULL))
    {
        _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                             "Win32: Failed to convert string to UTF-8");
        free(target);
        return NULL;
    }

    return target;
}

// Reports the specified error, appending information about the last Win32 error
//
void _linearwinInputErrorWin32(int error, const char* description)
{
    WCHAR buffer[_LINEARWIN_MESSAGE_SIZE] = L"";
    char message[_LINEARWIN_MESSAGE_SIZE] = "";

    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS |
                       FORMAT_MESSAGE_MAX_WIDTH_MASK,
                   NULL,
                   GetLastError() & 0xffff,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   buffer,
                   sizeof(buffer) / sizeof(WCHAR),
                   NULL);
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, message, sizeof(message), NULL, NULL);

    _linearwinInputError(error, "%s: %s", description, message);
}

// Updates key names according to the current keyboard layout
//
void _linearwinUpdateKeyNamesWin32(void)
{
    int key;
    BYTE state[256] = {0};

    memset(_linearwin.win32.keynames, 0, sizeof(_linearwin.win32.keynames));

    for (key = LINEARWIN_KEY_SPACE;  key <= LINEARWIN_KEY_LAST;  key++)
    {
        UINT vk;
        int scancode, length;
        WCHAR chars[16];

        scancode = _linearwin.win32.scancodes[key];
        if (scancode == -1)
            continue;

        if (key >= LINEARWIN_KEY_KP_0 && key <= LINEARWIN_KEY_KP_ADD)
        {
            const UINT vks[] = {
                VK_NUMPAD0,  VK_NUMPAD1,  VK_NUMPAD2, VK_NUMPAD3,
                VK_NUMPAD4,  VK_NUMPAD5,  VK_NUMPAD6, VK_NUMPAD7,
                VK_NUMPAD8,  VK_NUMPAD9,  VK_DECIMAL, VK_DIVIDE,
                VK_MULTIPLY, VK_SUBTRACT, VK_ADD
            };

            vk = vks[key - LINEARWIN_KEY_KP_0];
        }
        else
            vk = MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK);

        length = ToUnicode(vk, scancode, state,
                           chars, sizeof(chars) / sizeof(WCHAR),
                           0);

        if (length == -1)
        {
            // This is a dead key, so we need a second simulated key press
            // to make it output its own character (usually a diacritic)
            length = ToUnicode(vk, scancode, state,
                               chars, sizeof(chars) / sizeof(WCHAR),
                               0);
        }

        if (length < 1)
            continue;

        WideCharToMultiByte(CP_UTF8, 0, chars, 1,
                            _linearwin.win32.keynames[key],
                            sizeof(_linearwin.win32.keynames[key]),
                            NULL, NULL);
    }
}

// Replacement for IsWindowsVersionOrGreater, as we cannot rely on the
// application having a correct embedded manifest
//
BOOL _linearwinIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), major, minor, 0, 0, {0}, sp };
    DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR;
    ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    // HACK: Use RtlVerifyVersionInfo instead of VerifyVersionInfoW as the
    //       latter lies unless the user knew to embed a non-default manifest
    //       announcing support for Windows 10 via supportedOS GUID
    return RtlVerifyVersionInfo(&osvi, mask, cond) == 0;
}

// Checks whether we are on at least the specified build of Windows 10
//
BOOL _linearwinIsWindows10BuildOrGreaterWin32(WORD build)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), 10, 0, build };
    DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER;
    ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_BUILDNUMBER, VER_GREATER_EQUAL);
    // HACK: Use RtlVerifyVersionInfo instead of VerifyVersionInfoW as the
    //       latter lies unless the user knew to embed a non-default manifest
    //       announcing support for Windows 10 via supportedOS GUID
    return RtlVerifyVersionInfo(&osvi, mask, cond) == 0;
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _linearwinPlatformInit(void)
{
    // To make SetForegroundWindow work as we want, we need to fiddle
    // with the FOREGROUNDLOCKTIMEOUT system setting (we do this as early
    // as possible in the hope of still being the foreground process)
    SystemParametersInfoW(SPI_GETFOREGROUNDLOCKTIMEOUT, 0,
                          &_linearwin.win32.foregroundLockTimeout, 0);
    SystemParametersInfoW(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, UIntToPtr(0),
                          SPIF_SENDCHANGE);

    if (!loadLibraries())
        return LINEARWIN_FALSE;

    createKeyTables();
    _linearwinUpdateKeyNamesWin32();

    if (_linearwinIsWindows10CreatorsUpdateOrGreaterWin32())
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    else if (IsWindows8Point1OrGreater())
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    else if (IsWindowsVistaOrGreater())
        SetProcessDPIAware();

    if (!_linearwinRegisterWindowClassWin32())
        return LINEARWIN_FALSE;

    if (!createHelperWindow())
        return LINEARWIN_FALSE;

    _linearwinInitTimerWin32();
    _linearwinInitJoysticksWin32();

    _linearwinPollMonitorsWin32();
    return LINEARWIN_TRUE;
}

void _linearwinPlatformTerminate(void)
{
    if (_linearwin.win32.deviceNotificationHandle)
        UnregisterDeviceNotification(_linearwin.win32.deviceNotificationHandle);

    if (_linearwin.win32.helperWindowHandle)
        DestroyWindow(_linearwin.win32.helperWindowHandle);

    _linearwinUnregisterWindowClassWin32();

    // Restore previous foreground lock timeout system setting
    SystemParametersInfoW(SPI_SETFOREGROUNDLOCKTIMEOUT, 0,
                          UIntToPtr(_linearwin.win32.foregroundLockTimeout),
                          SPIF_SENDCHANGE);

    free(_linearwin.win32.clipboardString);
    free(_linearwin.win32.rawInput);

    _linearwinTerminateWGL();
    _linearwinTerminateEGL();
    _linearwinTerminateOSMesa();

    _linearwinTerminateJoysticksWin32();

    freeLibraries();
}

const char* _linearwinPlatformGetVersionString(void)
{
    return _LINEARWIN_VERSION_NUMBER " Win32 WGL EGL OSMesa"
#if defined(__MINGW32__)
        " MinGW"
#elif defined(_MSC_VER)
        " VisualC"
#endif
#if defined(_LINEARWIN_USE_HYBRID_HPG) || defined(_LINEARWIN_USE_OPTIMUS_HPG)
        " hybrid-GPU"
#endif
#if defined(_LINEARWIN_BUILD_DLL)
        " DLL"
#endif
        ;
}

