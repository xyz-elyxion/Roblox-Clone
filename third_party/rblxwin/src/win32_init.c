//========================================================================
// RBLXWIN 3.3 Win32 - www.rblxwin.org
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

#include <stdlib.h>
#include <malloc.h>

static const GUID _rblxwin_GUID_DEVINTERFACE_HID =
    {0x4d1e55b2,0xf16f,0x11cf,{0x88,0xcb,0x00,0x11,0x11,0x00,0x00,0x30}};

#define GUID_DEVINTERFACE_HID _rblxwin_GUID_DEVINTERFACE_HID

#if defined(_RBLXWIN_USE_HYBRID_HPG) || defined(_RBLXWIN_USE_OPTIMUS_HPG)

#if defined(_RBLXWIN_BUILD_DLL)
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

#endif // _RBLXWIN_USE_HYBRID_HPG

#if defined(_RBLXWIN_BUILD_DLL)

// RBLXWIN DLL entry point
//
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    return TRUE;
}

#endif // _RBLXWIN_BUILD_DLL

// Load necessary libraries (DLLs)
//
static RBLXWINbool loadLibraries(void)
{
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (const WCHAR*) &_rblxwin,
                            (HMODULE*) &_rblxwin.win32.instance))
    {
        _rblxwinInputErrorWin32(RBLXWIN_PLATFORM_ERROR,
                             "Win32: Failed to retrieve own module handle");
        return RBLXWIN_FALSE;
    }

    _rblxwin.win32.user32.instance = LoadLibraryA("user32.dll");
    if (!_rblxwin.win32.user32.instance)
    {
        _rblxwinInputErrorWin32(RBLXWIN_PLATFORM_ERROR,
                             "Win32: Failed to load user32.dll");
        return RBLXWIN_FALSE;
    }

    _rblxwin.win32.user32.SetProcessDPIAware_ = (PFN_SetProcessDPIAware)
        GetProcAddress(_rblxwin.win32.user32.instance, "SetProcessDPIAware");
    _rblxwin.win32.user32.ChangeWindowMessageFilterEx_ = (PFN_ChangeWindowMessageFilterEx)
        GetProcAddress(_rblxwin.win32.user32.instance, "ChangeWindowMessageFilterEx");
    _rblxwin.win32.user32.EnableNonClientDpiScaling_ = (PFN_EnableNonClientDpiScaling)
        GetProcAddress(_rblxwin.win32.user32.instance, "EnableNonClientDpiScaling");
    _rblxwin.win32.user32.SetProcessDpiAwarenessContext_ = (PFN_SetProcessDpiAwarenessContext)
        GetProcAddress(_rblxwin.win32.user32.instance, "SetProcessDpiAwarenessContext");
    _rblxwin.win32.user32.GetDpiForWindow_ = (PFN_GetDpiForWindow)
        GetProcAddress(_rblxwin.win32.user32.instance, "GetDpiForWindow");
    _rblxwin.win32.user32.AdjustWindowRectExForDpi_ = (PFN_AdjustWindowRectExForDpi)
        GetProcAddress(_rblxwin.win32.user32.instance, "AdjustWindowRectExForDpi");
    _rblxwin.win32.user32.GetSystemMetricsForDpi_ = (PFN_GetSystemMetricsForDpi)
        GetProcAddress(_rblxwin.win32.user32.instance, "GetSystemMetricsForDpi");

    _rblxwin.win32.dinput8.instance = LoadLibraryA("dinput8.dll");
    if (_rblxwin.win32.dinput8.instance)
    {
        _rblxwin.win32.dinput8.Create = (PFN_DirectInput8Create)
            GetProcAddress(_rblxwin.win32.dinput8.instance, "DirectInput8Create");
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
            _rblxwin.win32.xinput.instance = LoadLibraryA(names[i]);
            if (_rblxwin.win32.xinput.instance)
            {
                _rblxwin.win32.xinput.GetCapabilities = (PFN_XInputGetCapabilities)
                    GetProcAddress(_rblxwin.win32.xinput.instance, "XInputGetCapabilities");
                _rblxwin.win32.xinput.GetState = (PFN_XInputGetState)
                    GetProcAddress(_rblxwin.win32.xinput.instance, "XInputGetState");

                break;
            }
        }
    }

    _rblxwin.win32.dwmapi.instance = LoadLibraryA("dwmapi.dll");
    if (_rblxwin.win32.dwmapi.instance)
    {
        _rblxwin.win32.dwmapi.IsCompositionEnabled = (PFN_DwmIsCompositionEnabled)
            GetProcAddress(_rblxwin.win32.dwmapi.instance, "DwmIsCompositionEnabled");
        _rblxwin.win32.dwmapi.Flush = (PFN_DwmFlush)
            GetProcAddress(_rblxwin.win32.dwmapi.instance, "DwmFlush");
        _rblxwin.win32.dwmapi.EnableBlurBehindWindow = (PFN_DwmEnableBlurBehindWindow)
            GetProcAddress(_rblxwin.win32.dwmapi.instance, "DwmEnableBlurBehindWindow");
        _rblxwin.win32.dwmapi.GetColorizationColor = (PFN_DwmGetColorizationColor)
            GetProcAddress(_rblxwin.win32.dwmapi.instance, "DwmGetColorizationColor");
    }

    _rblxwin.win32.shcore.instance = LoadLibraryA("shcore.dll");
    if (_rblxwin.win32.shcore.instance)
    {
        _rblxwin.win32.shcore.SetProcessDpiAwareness_ = (PFN_SetProcessDpiAwareness)
            GetProcAddress(_rblxwin.win32.shcore.instance, "SetProcessDpiAwareness");
        _rblxwin.win32.shcore.GetDpiForMonitor_ = (PFN_GetDpiForMonitor)
            GetProcAddress(_rblxwin.win32.shcore.instance, "GetDpiForMonitor");
    }

    _rblxwin.win32.ntdll.instance = LoadLibraryA("ntdll.dll");
    if (_rblxwin.win32.ntdll.instance)
    {
        _rblxwin.win32.ntdll.RtlVerifyVersionInfo_ = (PFN_RtlVerifyVersionInfo)
            GetProcAddress(_rblxwin.win32.ntdll.instance, "RtlVerifyVersionInfo");
    }

    return RBLXWIN_TRUE;
}

// Unload used libraries (DLLs)
//
static void freeLibraries(void)
{
    if (_rblxwin.win32.xinput.instance)
        FreeLibrary(_rblxwin.win32.xinput.instance);

    if (_rblxwin.win32.dinput8.instance)
        FreeLibrary(_rblxwin.win32.dinput8.instance);

    if (_rblxwin.win32.user32.instance)
        FreeLibrary(_rblxwin.win32.user32.instance);

    if (_rblxwin.win32.dwmapi.instance)
        FreeLibrary(_rblxwin.win32.dwmapi.instance);

    if (_rblxwin.win32.shcore.instance)
        FreeLibrary(_rblxwin.win32.shcore.instance);

    if (_rblxwin.win32.ntdll.instance)
        FreeLibrary(_rblxwin.win32.ntdll.instance);
}

// Create key code translation tables
//
static void createKeyTables(void)
{
    int scancode;

    memset(_rblxwin.win32.keycodes, -1, sizeof(_rblxwin.win32.keycodes));
    memset(_rblxwin.win32.scancodes, -1, sizeof(_rblxwin.win32.scancodes));

    _rblxwin.win32.keycodes[0x00B] = RBLXWIN_KEY_0;
    _rblxwin.win32.keycodes[0x002] = RBLXWIN_KEY_1;
    _rblxwin.win32.keycodes[0x003] = RBLXWIN_KEY_2;
    _rblxwin.win32.keycodes[0x004] = RBLXWIN_KEY_3;
    _rblxwin.win32.keycodes[0x005] = RBLXWIN_KEY_4;
    _rblxwin.win32.keycodes[0x006] = RBLXWIN_KEY_5;
    _rblxwin.win32.keycodes[0x007] = RBLXWIN_KEY_6;
    _rblxwin.win32.keycodes[0x008] = RBLXWIN_KEY_7;
    _rblxwin.win32.keycodes[0x009] = RBLXWIN_KEY_8;
    _rblxwin.win32.keycodes[0x00A] = RBLXWIN_KEY_9;
    _rblxwin.win32.keycodes[0x01E] = RBLXWIN_KEY_A;
    _rblxwin.win32.keycodes[0x030] = RBLXWIN_KEY_B;
    _rblxwin.win32.keycodes[0x02E] = RBLXWIN_KEY_C;
    _rblxwin.win32.keycodes[0x020] = RBLXWIN_KEY_D;
    _rblxwin.win32.keycodes[0x012] = RBLXWIN_KEY_E;
    _rblxwin.win32.keycodes[0x021] = RBLXWIN_KEY_F;
    _rblxwin.win32.keycodes[0x022] = RBLXWIN_KEY_G;
    _rblxwin.win32.keycodes[0x023] = RBLXWIN_KEY_H;
    _rblxwin.win32.keycodes[0x017] = RBLXWIN_KEY_I;
    _rblxwin.win32.keycodes[0x024] = RBLXWIN_KEY_J;
    _rblxwin.win32.keycodes[0x025] = RBLXWIN_KEY_K;
    _rblxwin.win32.keycodes[0x026] = RBLXWIN_KEY_L;
    _rblxwin.win32.keycodes[0x032] = RBLXWIN_KEY_M;
    _rblxwin.win32.keycodes[0x031] = RBLXWIN_KEY_N;
    _rblxwin.win32.keycodes[0x018] = RBLXWIN_KEY_O;
    _rblxwin.win32.keycodes[0x019] = RBLXWIN_KEY_P;
    _rblxwin.win32.keycodes[0x010] = RBLXWIN_KEY_Q;
    _rblxwin.win32.keycodes[0x013] = RBLXWIN_KEY_R;
    _rblxwin.win32.keycodes[0x01F] = RBLXWIN_KEY_S;
    _rblxwin.win32.keycodes[0x014] = RBLXWIN_KEY_T;
    _rblxwin.win32.keycodes[0x016] = RBLXWIN_KEY_U;
    _rblxwin.win32.keycodes[0x02F] = RBLXWIN_KEY_V;
    _rblxwin.win32.keycodes[0x011] = RBLXWIN_KEY_W;
    _rblxwin.win32.keycodes[0x02D] = RBLXWIN_KEY_X;
    _rblxwin.win32.keycodes[0x015] = RBLXWIN_KEY_Y;
    _rblxwin.win32.keycodes[0x02C] = RBLXWIN_KEY_Z;

    _rblxwin.win32.keycodes[0x028] = RBLXWIN_KEY_APOSTROPHE;
    _rblxwin.win32.keycodes[0x02B] = RBLXWIN_KEY_BACKSLASH;
    _rblxwin.win32.keycodes[0x033] = RBLXWIN_KEY_COMMA;
    _rblxwin.win32.keycodes[0x00D] = RBLXWIN_KEY_EQUAL;
    _rblxwin.win32.keycodes[0x029] = RBLXWIN_KEY_GRAVE_ACCENT;
    _rblxwin.win32.keycodes[0x01A] = RBLXWIN_KEY_LEFT_BRACKET;
    _rblxwin.win32.keycodes[0x00C] = RBLXWIN_KEY_MINUS;
    _rblxwin.win32.keycodes[0x034] = RBLXWIN_KEY_PERIOD;
    _rblxwin.win32.keycodes[0x01B] = RBLXWIN_KEY_RIGHT_BRACKET;
    _rblxwin.win32.keycodes[0x027] = RBLXWIN_KEY_SEMICOLON;
    _rblxwin.win32.keycodes[0x035] = RBLXWIN_KEY_SLASH;
    _rblxwin.win32.keycodes[0x056] = RBLXWIN_KEY_WORLD_2;

    _rblxwin.win32.keycodes[0x00E] = RBLXWIN_KEY_BACKSPACE;
    _rblxwin.win32.keycodes[0x153] = RBLXWIN_KEY_DELETE;
    _rblxwin.win32.keycodes[0x14F] = RBLXWIN_KEY_END;
    _rblxwin.win32.keycodes[0x01C] = RBLXWIN_KEY_ENTER;
    _rblxwin.win32.keycodes[0x001] = RBLXWIN_KEY_ESCAPE;
    _rblxwin.win32.keycodes[0x147] = RBLXWIN_KEY_HOME;
    _rblxwin.win32.keycodes[0x152] = RBLXWIN_KEY_INSERT;
    _rblxwin.win32.keycodes[0x15D] = RBLXWIN_KEY_MENU;
    _rblxwin.win32.keycodes[0x151] = RBLXWIN_KEY_PAGE_DOWN;
    _rblxwin.win32.keycodes[0x149] = RBLXWIN_KEY_PAGE_UP;
    _rblxwin.win32.keycodes[0x045] = RBLXWIN_KEY_PAUSE;
    _rblxwin.win32.keycodes[0x039] = RBLXWIN_KEY_SPACE;
    _rblxwin.win32.keycodes[0x00F] = RBLXWIN_KEY_TAB;
    _rblxwin.win32.keycodes[0x03A] = RBLXWIN_KEY_CAPS_LOCK;
    _rblxwin.win32.keycodes[0x145] = RBLXWIN_KEY_NUM_LOCK;
    _rblxwin.win32.keycodes[0x046] = RBLXWIN_KEY_SCROLL_LOCK;
    _rblxwin.win32.keycodes[0x03B] = RBLXWIN_KEY_F1;
    _rblxwin.win32.keycodes[0x03C] = RBLXWIN_KEY_F2;
    _rblxwin.win32.keycodes[0x03D] = RBLXWIN_KEY_F3;
    _rblxwin.win32.keycodes[0x03E] = RBLXWIN_KEY_F4;
    _rblxwin.win32.keycodes[0x03F] = RBLXWIN_KEY_F5;
    _rblxwin.win32.keycodes[0x040] = RBLXWIN_KEY_F6;
    _rblxwin.win32.keycodes[0x041] = RBLXWIN_KEY_F7;
    _rblxwin.win32.keycodes[0x042] = RBLXWIN_KEY_F8;
    _rblxwin.win32.keycodes[0x043] = RBLXWIN_KEY_F9;
    _rblxwin.win32.keycodes[0x044] = RBLXWIN_KEY_F10;
    _rblxwin.win32.keycodes[0x057] = RBLXWIN_KEY_F11;
    _rblxwin.win32.keycodes[0x058] = RBLXWIN_KEY_F12;
    _rblxwin.win32.keycodes[0x064] = RBLXWIN_KEY_F13;
    _rblxwin.win32.keycodes[0x065] = RBLXWIN_KEY_F14;
    _rblxwin.win32.keycodes[0x066] = RBLXWIN_KEY_F15;
    _rblxwin.win32.keycodes[0x067] = RBLXWIN_KEY_F16;
    _rblxwin.win32.keycodes[0x068] = RBLXWIN_KEY_F17;
    _rblxwin.win32.keycodes[0x069] = RBLXWIN_KEY_F18;
    _rblxwin.win32.keycodes[0x06A] = RBLXWIN_KEY_F19;
    _rblxwin.win32.keycodes[0x06B] = RBLXWIN_KEY_F20;
    _rblxwin.win32.keycodes[0x06C] = RBLXWIN_KEY_F21;
    _rblxwin.win32.keycodes[0x06D] = RBLXWIN_KEY_F22;
    _rblxwin.win32.keycodes[0x06E] = RBLXWIN_KEY_F23;
    _rblxwin.win32.keycodes[0x076] = RBLXWIN_KEY_F24;
    _rblxwin.win32.keycodes[0x038] = RBLXWIN_KEY_LEFT_ALT;
    _rblxwin.win32.keycodes[0x01D] = RBLXWIN_KEY_LEFT_CONTROL;
    _rblxwin.win32.keycodes[0x02A] = RBLXWIN_KEY_LEFT_SHIFT;
    _rblxwin.win32.keycodes[0x15B] = RBLXWIN_KEY_LEFT_SUPER;
    _rblxwin.win32.keycodes[0x137] = RBLXWIN_KEY_PRINT_SCREEN;
    _rblxwin.win32.keycodes[0x138] = RBLXWIN_KEY_RIGHT_ALT;
    _rblxwin.win32.keycodes[0x11D] = RBLXWIN_KEY_RIGHT_CONTROL;
    _rblxwin.win32.keycodes[0x036] = RBLXWIN_KEY_RIGHT_SHIFT;
    _rblxwin.win32.keycodes[0x15C] = RBLXWIN_KEY_RIGHT_SUPER;
    _rblxwin.win32.keycodes[0x150] = RBLXWIN_KEY_DOWN;
    _rblxwin.win32.keycodes[0x14B] = RBLXWIN_KEY_LEFT;
    _rblxwin.win32.keycodes[0x14D] = RBLXWIN_KEY_RIGHT;
    _rblxwin.win32.keycodes[0x148] = RBLXWIN_KEY_UP;

    _rblxwin.win32.keycodes[0x052] = RBLXWIN_KEY_KP_0;
    _rblxwin.win32.keycodes[0x04F] = RBLXWIN_KEY_KP_1;
    _rblxwin.win32.keycodes[0x050] = RBLXWIN_KEY_KP_2;
    _rblxwin.win32.keycodes[0x051] = RBLXWIN_KEY_KP_3;
    _rblxwin.win32.keycodes[0x04B] = RBLXWIN_KEY_KP_4;
    _rblxwin.win32.keycodes[0x04C] = RBLXWIN_KEY_KP_5;
    _rblxwin.win32.keycodes[0x04D] = RBLXWIN_KEY_KP_6;
    _rblxwin.win32.keycodes[0x047] = RBLXWIN_KEY_KP_7;
    _rblxwin.win32.keycodes[0x048] = RBLXWIN_KEY_KP_8;
    _rblxwin.win32.keycodes[0x049] = RBLXWIN_KEY_KP_9;
    _rblxwin.win32.keycodes[0x04E] = RBLXWIN_KEY_KP_ADD;
    _rblxwin.win32.keycodes[0x053] = RBLXWIN_KEY_KP_DECIMAL;
    _rblxwin.win32.keycodes[0x135] = RBLXWIN_KEY_KP_DIVIDE;
    _rblxwin.win32.keycodes[0x11C] = RBLXWIN_KEY_KP_ENTER;
    _rblxwin.win32.keycodes[0x059] = RBLXWIN_KEY_KP_EQUAL;
    _rblxwin.win32.keycodes[0x037] = RBLXWIN_KEY_KP_MULTIPLY;
    _rblxwin.win32.keycodes[0x04A] = RBLXWIN_KEY_KP_SUBTRACT;

    for (scancode = 0;  scancode < 512;  scancode++)
    {
        if (_rblxwin.win32.keycodes[scancode] > 0)
            _rblxwin.win32.scancodes[_rblxwin.win32.keycodes[scancode]] = scancode;
    }
}

// Creates a dummy window for behind-the-scenes work
//
static RBLXWINbool createHelperWindow(void)
{
    MSG msg;

    _rblxwin.win32.helperWindowHandle =
        CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
                        _RBLXWIN_WNDCLASSNAME,
                        L"RBLXWIN message window",
                        WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                        0, 0, 1, 1,
                        NULL, NULL,
                        _rblxwin.win32.instance,
                        NULL);

    if (!_rblxwin.win32.helperWindowHandle)
    {
        _rblxwinInputErrorWin32(RBLXWIN_PLATFORM_ERROR,
                             "Win32: Failed to create helper window");
        return RBLXWIN_FALSE;
    }

    // HACK: The command to the first ShowWindow call is ignored if the parent
    //       process passed along a STARTUPINFO, so clear that with a no-op call
    ShowWindow(_rblxwin.win32.helperWindowHandle, SW_HIDE);

    // Register for HID device notifications
    {
        DEV_BROADCAST_DEVICEINTERFACE_W dbi;
        ZeroMemory(&dbi, sizeof(dbi));
        dbi.dbcc_size = sizeof(dbi);
        dbi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        dbi.dbcc_classguid = GUID_DEVINTERFACE_HID;

        _rblxwin.win32.deviceNotificationHandle =
            RegisterDeviceNotificationW(_rblxwin.win32.helperWindowHandle,
                                        (DEV_BROADCAST_HDR*) &dbi,
                                        DEVICE_NOTIFY_WINDOW_HANDLE);
    }

    while (PeekMessageW(&msg, _rblxwin.win32.helperWindowHandle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

   return RBLXWIN_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Returns a wide string version of the specified UTF-8 string
//
WCHAR* _rblxwinCreateWideStringFromUTF8Win32(const char* source)
{
    WCHAR* target;
    int count;

    count = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
    if (!count)
    {
        _rblxwinInputErrorWin32(RBLXWIN_PLATFORM_ERROR,
                             "Win32: Failed to convert string from UTF-8");
        return NULL;
    }

    target = calloc(count, sizeof(WCHAR));

    if (!MultiByteToWideChar(CP_UTF8, 0, source, -1, target, count))
    {
        _rblxwinInputErrorWin32(RBLXWIN_PLATFORM_ERROR,
                             "Win32: Failed to convert string from UTF-8");
        free(target);
        return NULL;
    }

    return target;
}

// Returns a UTF-8 string version of the specified wide string
//
char* _rblxwinCreateUTF8FromWideStringWin32(const WCHAR* source)
{
    char* target;
    int size;

    size = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
    if (!size)
    {
        _rblxwinInputErrorWin32(RBLXWIN_PLATFORM_ERROR,
                             "Win32: Failed to convert string to UTF-8");
        return NULL;
    }

    target = calloc(size, 1);

    if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, size, NULL, NULL))
    {
        _rblxwinInputErrorWin32(RBLXWIN_PLATFORM_ERROR,
                             "Win32: Failed to convert string to UTF-8");
        free(target);
        return NULL;
    }

    return target;
}

// Reports the specified error, appending information about the last Win32 error
//
void _rblxwinInputErrorWin32(int error, const char* description)
{
    WCHAR buffer[_RBLXWIN_MESSAGE_SIZE] = L"";
    char message[_RBLXWIN_MESSAGE_SIZE] = "";

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

    _rblxwinInputError(error, "%s: %s", description, message);
}

// Updates key names according to the current keyboard layout
//
void _rblxwinUpdateKeyNamesWin32(void)
{
    int key;
    BYTE state[256] = {0};

    memset(_rblxwin.win32.keynames, 0, sizeof(_rblxwin.win32.keynames));

    for (key = RBLXWIN_KEY_SPACE;  key <= RBLXWIN_KEY_LAST;  key++)
    {
        UINT vk;
        int scancode, length;
        WCHAR chars[16];

        scancode = _rblxwin.win32.scancodes[key];
        if (scancode == -1)
            continue;

        if (key >= RBLXWIN_KEY_KP_0 && key <= RBLXWIN_KEY_KP_ADD)
        {
            const UINT vks[] = {
                VK_NUMPAD0,  VK_NUMPAD1,  VK_NUMPAD2, VK_NUMPAD3,
                VK_NUMPAD4,  VK_NUMPAD5,  VK_NUMPAD6, VK_NUMPAD7,
                VK_NUMPAD8,  VK_NUMPAD9,  VK_DECIMAL, VK_DIVIDE,
                VK_MULTIPLY, VK_SUBTRACT, VK_ADD
            };

            vk = vks[key - RBLXWIN_KEY_KP_0];
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
                            _rblxwin.win32.keynames[key],
                            sizeof(_rblxwin.win32.keynames[key]),
                            NULL, NULL);
    }
}

// Replacement for IsWindowsVersionOrGreater, as we cannot rely on the
// application having a correct embedded manifest
//
BOOL _rblxwinIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp)
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
BOOL _rblxwinIsWindows10BuildOrGreaterWin32(WORD build)
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
//////                       RBLXWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _rblxwinPlatformInit(void)
{
    // To make SetForegroundWindow work as we want, we need to fiddle
    // with the FOREGROUNDLOCKTIMEOUT system setting (we do this as early
    // as possible in the hope of still being the foreground process)
    SystemParametersInfoW(SPI_GETFOREGROUNDLOCKTIMEOUT, 0,
                          &_rblxwin.win32.foregroundLockTimeout, 0);
    SystemParametersInfoW(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, UIntToPtr(0),
                          SPIF_SENDCHANGE);

    if (!loadLibraries())
        return RBLXWIN_FALSE;

    createKeyTables();
    _rblxwinUpdateKeyNamesWin32();

    if (_rblxwinIsWindows10CreatorsUpdateOrGreaterWin32())
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    else if (IsWindows8Point1OrGreater())
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    else if (IsWindowsVistaOrGreater())
        SetProcessDPIAware();

    if (!_rblxwinRegisterWindowClassWin32())
        return RBLXWIN_FALSE;

    if (!createHelperWindow())
        return RBLXWIN_FALSE;

    _rblxwinInitTimerWin32();
    _rblxwinInitJoysticksWin32();

    _rblxwinPollMonitorsWin32();
    return RBLXWIN_TRUE;
}

void _rblxwinPlatformTerminate(void)
{
    if (_rblxwin.win32.deviceNotificationHandle)
        UnregisterDeviceNotification(_rblxwin.win32.deviceNotificationHandle);

    if (_rblxwin.win32.helperWindowHandle)
        DestroyWindow(_rblxwin.win32.helperWindowHandle);

    _rblxwinUnregisterWindowClassWin32();

    // Restore previous foreground lock timeout system setting
    SystemParametersInfoW(SPI_SETFOREGROUNDLOCKTIMEOUT, 0,
                          UIntToPtr(_rblxwin.win32.foregroundLockTimeout),
                          SPIF_SENDCHANGE);

    free(_rblxwin.win32.clipboardString);
    free(_rblxwin.win32.rawInput);

    _rblxwinTerminateWGL();
    _rblxwinTerminateEGL();
    _rblxwinTerminateOSMesa();

    _rblxwinTerminateJoysticksWin32();

    freeLibraries();
}

const char* _rblxwinPlatformGetVersionString(void)
{
    return _RBLXWIN_VERSION_NUMBER " Win32 WGL EGL OSMesa"
#if defined(__MINGW32__)
        " MinGW"
#elif defined(_MSC_VER)
        " VisualC"
#endif
#if defined(_RBLXWIN_USE_HYBRID_HPG) || defined(_RBLXWIN_USE_OPTIMUS_HPG)
        " hybrid-GPU"
#endif
#if defined(_RBLXWIN_BUILD_DLL)
        " DLL"
#endif
        ;
}

