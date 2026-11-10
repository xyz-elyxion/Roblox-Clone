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

// We don't need all the fancy stuff
#ifndef NOMINMAX
 #define NOMINMAX
#endif

#ifndef VC_EXTRALEAN
 #define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN
 #define WIN32_LEAN_AND_MEAN
#endif

// This is a workaround for the fact that rblxwin.h needs to export APIENTRY (for
// example to allow applications to correctly declare a GL_KHR_debug callback)
// but windows.h assumes no one will define APIENTRY before it does
#undef APIENTRY

// RBLXWIN on Windows is Unicode only and does not work in MBCS mode
#ifndef UNICODE
 #define UNICODE
#endif

// RBLXWIN requires Windows XP or later
#if WINVER < 0x0501
 #undef WINVER
 #define WINVER 0x0501
#endif
#if _WIN32_WINNT < 0x0501
 #undef _WIN32_WINNT
 #define _WIN32_WINNT 0x0501
#endif

// RBLXWIN uses DirectInput8 interfaces
#define DIRECTINPUT_VERSION 0x0800

// RBLXWIN uses OEM cursor resources
#define OEMRESOURCE

#include <wctype.h>
#include <windows.h>
#include <dinput.h>
#include <xinput.h>
#include <dbt.h>

// HACK: Define macros that some windows.h variants don't
#ifndef WM_MOUSEHWHEEL
 #define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef WM_DWMCOMPOSITIONCHANGED
 #define WM_DWMCOMPOSITIONCHANGED 0x031E
#endif
#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
 #define WM_DWMCOLORIZATIONCOLORCHANGED 0x0320
#endif
#ifndef WM_COPYGLOBALDATA
 #define WM_COPYGLOBALDATA 0x0049
#endif
#ifndef WM_UNICHAR
 #define WM_UNICHAR 0x0109
#endif
#ifndef UNICODE_NOCHAR
 #define UNICODE_NOCHAR 0xFFFF
#endif
#ifndef WM_DPICHANGED
 #define WM_DPICHANGED 0x02E0
#endif
#ifndef GET_XBUTTON_WPARAM
 #define GET_XBUTTON_WPARAM(w) (HIWORD(w))
#endif
#ifndef EDS_ROTATEDMODE
 #define EDS_ROTATEDMODE 0x00000004
#endif
#ifndef DISPLAY_DEVICE_ACTIVE
 #define DISPLAY_DEVICE_ACTIVE 0x00000001
#endif
#ifndef _WIN32_WINNT_WINBLUE
 #define _WIN32_WINNT_WINBLUE 0x0603
#endif
#ifndef _WIN32_WINNT_WIN8
 #define _WIN32_WINNT_WIN8 0x0602
#endif
#ifndef WM_GETDPISCALEDSIZE
 #define WM_GETDPISCALEDSIZE 0x02e4
#endif
#ifndef USER_DEFAULT_SCREEN_DPI
 #define USER_DEFAULT_SCREEN_DPI 96
#endif
#ifndef OCR_HAND
 #define OCR_HAND 32649
#endif

#if WINVER < 0x0601
typedef struct
{
    DWORD cbSize;
    DWORD ExtStatus;
} CHANGEFILTERSTRUCT;
#ifndef MSGFLT_ALLOW
 #define MSGFLT_ALLOW 1
#endif
#endif /*Windows 7*/

#if WINVER < 0x0600
#define DWM_BB_ENABLE 0x00000001
#define DWM_BB_BLURREGION 0x00000002
typedef struct
{
    DWORD dwFlags;
    BOOL fEnable;
    HRGN hRgnBlur;
    BOOL fTransitionOnMaximized;
} DWM_BLURBEHIND;
#else
 #include <dwmapi.h>
#endif /*Windows Vista*/

#ifndef DPI_ENUMS_DECLARED
typedef enum
{
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
typedef enum
{
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI = 1,
    MDT_RAW_DPI = 2,
    MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;
#endif /*DPI_ENUMS_DECLARED*/

#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((HANDLE) -4)
#endif /*DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2*/

// Replacement for versionhelpers.h macros, as we cannot rely on the
// application having a correct embedded manifest
//
#define IsWindowsXPOrGreater()                                 \
    _rblxwinIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WINXP),   \
                                        LOBYTE(_WIN32_WINNT_WINXP), 0)
#define IsWindowsVistaOrGreater()                                     \
    _rblxwinIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_VISTA),   \
                                        LOBYTE(_WIN32_WINNT_VISTA), 0)
#define IsWindows7OrGreater()                                         \
    _rblxwinIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WIN7),    \
                                        LOBYTE(_WIN32_WINNT_WIN7), 0)
#define IsWindows8OrGreater()                                         \
    _rblxwinIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WIN8),    \
                                        LOBYTE(_WIN32_WINNT_WIN8), 0)
#define IsWindows8Point1OrGreater()                                   \
    _rblxwinIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WINBLUE), \
                                        LOBYTE(_WIN32_WINNT_WINBLUE), 0)

#define _rblxwinIsWindows10AnniversaryUpdateOrGreaterWin32() \
    _rblxwinIsWindows10BuildOrGreaterWin32(14393)
#define _rblxwinIsWindows10CreatorsUpdateOrGreaterWin32() \
    _rblxwinIsWindows10BuildOrGreaterWin32(15063)

// HACK: Define macros that some xinput.h variants don't
#ifndef XINPUT_CAPS_WIRELESS
 #define XINPUT_CAPS_WIRELESS 0x0002
#endif
#ifndef XINPUT_DEVSUBTYPE_WHEEL
 #define XINPUT_DEVSUBTYPE_WHEEL 0x02
#endif
#ifndef XINPUT_DEVSUBTYPE_ARCADE_STICK
 #define XINPUT_DEVSUBTYPE_ARCADE_STICK 0x03
#endif
#ifndef XINPUT_DEVSUBTYPE_FLIGHT_STICK
 #define XINPUT_DEVSUBTYPE_FLIGHT_STICK 0x04
#endif
#ifndef XINPUT_DEVSUBTYPE_DANCE_PAD
 #define XINPUT_DEVSUBTYPE_DANCE_PAD 0x05
#endif
#ifndef XINPUT_DEVSUBTYPE_GUITAR
 #define XINPUT_DEVSUBTYPE_GUITAR 0x06
#endif
#ifndef XINPUT_DEVSUBTYPE_DRUM_KIT
 #define XINPUT_DEVSUBTYPE_DRUM_KIT 0x08
#endif
#ifndef XINPUT_DEVSUBTYPE_ARCADE_PAD
 #define XINPUT_DEVSUBTYPE_ARCADE_PAD 0x13
#endif
#ifndef XUSER_MAX_COUNT
 #define XUSER_MAX_COUNT 4
#endif

// HACK: Define macros that some dinput.h variants don't
#ifndef DIDFT_OPTIONAL
 #define DIDFT_OPTIONAL 0x80000000
#endif

// xinput.dll function pointer typedefs
typedef DWORD (WINAPI * PFN_XInputGetCapabilities)(DWORD,DWORD,XINPUT_CAPABILITIES*);
typedef DWORD (WINAPI * PFN_XInputGetState)(DWORD,XINPUT_STATE*);
#define XInputGetCapabilities _rblxwin.win32.xinput.GetCapabilities
#define XInputGetState _rblxwin.win32.xinput.GetState

// dinput8.dll function pointer typedefs
typedef HRESULT (WINAPI * PFN_DirectInput8Create)(HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN);
#define DirectInput8Create _rblxwin.win32.dinput8.Create

// user32.dll function pointer typedefs
typedef BOOL (WINAPI * PFN_SetProcessDPIAware)(void);
typedef BOOL (WINAPI * PFN_ChangeWindowMessageFilterEx)(HWND,UINT,DWORD,CHANGEFILTERSTRUCT*);
typedef BOOL (WINAPI * PFN_EnableNonClientDpiScaling)(HWND);
typedef BOOL (WINAPI * PFN_SetProcessDpiAwarenessContext)(HANDLE);
typedef UINT (WINAPI * PFN_GetDpiForWindow)(HWND);
typedef BOOL (WINAPI * PFN_AdjustWindowRectExForDpi)(LPRECT,DWORD,BOOL,DWORD,UINT);
typedef int (WINAPI * PFN_GetSystemMetricsForDpi)(int,UINT);
#define SetProcessDPIAware _rblxwin.win32.user32.SetProcessDPIAware_
#define ChangeWindowMessageFilterEx _rblxwin.win32.user32.ChangeWindowMessageFilterEx_
#define EnableNonClientDpiScaling _rblxwin.win32.user32.EnableNonClientDpiScaling_
#define SetProcessDpiAwarenessContext _rblxwin.win32.user32.SetProcessDpiAwarenessContext_
#define GetDpiForWindow _rblxwin.win32.user32.GetDpiForWindow_
#define AdjustWindowRectExForDpi _rblxwin.win32.user32.AdjustWindowRectExForDpi_
#define GetSystemMetricsForDpi _rblxwin.win32.user32.GetSystemMetricsForDpi_

// dwmapi.dll function pointer typedefs
typedef HRESULT (WINAPI * PFN_DwmIsCompositionEnabled)(BOOL*);
typedef HRESULT (WINAPI * PFN_DwmFlush)(VOID);
typedef HRESULT(WINAPI * PFN_DwmEnableBlurBehindWindow)(HWND,const DWM_BLURBEHIND*);
typedef HRESULT (WINAPI * PFN_DwmGetColorizationColor)(DWORD*,BOOL*);
#define DwmIsCompositionEnabled _rblxwin.win32.dwmapi.IsCompositionEnabled
#define DwmFlush _rblxwin.win32.dwmapi.Flush
#define DwmEnableBlurBehindWindow _rblxwin.win32.dwmapi.EnableBlurBehindWindow
#define DwmGetColorizationColor _rblxwin.win32.dwmapi.GetColorizationColor

// shcore.dll function pointer typedefs
typedef HRESULT (WINAPI * PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);
typedef HRESULT (WINAPI * PFN_GetDpiForMonitor)(HMONITOR,MONITOR_DPI_TYPE,UINT*,UINT*);
#define SetProcessDpiAwareness _rblxwin.win32.shcore.SetProcessDpiAwareness_
#define GetDpiForMonitor _rblxwin.win32.shcore.GetDpiForMonitor_

// ntdll.dll function pointer typedefs
typedef LONG (WINAPI * PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*,ULONG,ULONGLONG);
#define RtlVerifyVersionInfo _rblxwin.win32.ntdll.RtlVerifyVersionInfo_

typedef VkFlags VkWin32SurfaceCreateFlagsKHR;

typedef struct VkWin32SurfaceCreateInfoKHR
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkWin32SurfaceCreateFlagsKHR    flags;
    HINSTANCE                       hinstance;
    HWND                            hwnd;
} VkWin32SurfaceCreateInfoKHR;

typedef VkResult (APIENTRY *PFN_vkCreateWin32SurfaceKHR)(VkInstance,const VkWin32SurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
typedef VkBool32 (APIENTRY *PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)(VkPhysicalDevice,uint32_t);

#include "win32_joystick.h"
#include "wgl_context.h"
#include "egl_context.h"
#include "osmesa_context.h"

#if !defined(_RBLXWIN_WNDCLASSNAME)
 #define _RBLXWIN_WNDCLASSNAME L"RBLXWIN30"
#endif

#define _rblxwin_dlopen(name) LoadLibraryA(name)
#define _rblxwin_dlclose(handle) FreeLibrary((HMODULE) handle)
#define _rblxwin_dlsym(handle, name) GetProcAddress((HMODULE) handle, name)

#define _RBLXWIN_EGL_NATIVE_WINDOW  ((EGLNativeWindowType) window->win32.handle)
#define _RBLXWIN_EGL_NATIVE_DISPLAY EGL_DEFAULT_DISPLAY

#define _RBLXWIN_PLATFORM_WINDOW_STATE         _RBLXWINwindowWin32  win32
#define _RBLXWIN_PLATFORM_LIBRARY_WINDOW_STATE _RBLXWINlibraryWin32 win32
#define _RBLXWIN_PLATFORM_LIBRARY_TIMER_STATE  _RBLXWINtimerWin32   win32
#define _RBLXWIN_PLATFORM_MONITOR_STATE        _RBLXWINmonitorWin32 win32
#define _RBLXWIN_PLATFORM_CURSOR_STATE         _RBLXWINcursorWin32  win32
#define _RBLXWIN_PLATFORM_TLS_STATE            _RBLXWINtlsWin32     win32
#define _RBLXWIN_PLATFORM_MUTEX_STATE          _RBLXWINmutexWin32   win32


// Win32-specific per-window data
//
typedef struct _RBLXWINwindowWin32
{
    HWND                handle;
    HICON               bigIcon;
    HICON               smallIcon;

    RBLXWINbool            cursorTracked;
    RBLXWINbool            frameAction;
    RBLXWINbool            iconified;
    RBLXWINbool            maximized;
    // Whether to enable framebuffer transparency on DWM
    RBLXWINbool            transparent;
    RBLXWINbool            scaleToMonitor;

    // Cached size used to filter out duplicate events
    int                 width, height;

    // The last received cursor position, regardless of source
    int                 lastCursorPosX, lastCursorPosY;
    // The last recevied high surrogate when decoding pairs of UTF-16 messages
    WCHAR               highSurrogate;
} _RBLXWINwindowWin32;

// Win32-specific global data
//
typedef struct _RBLXWINlibraryWin32
{
    HINSTANCE           instance;
    HWND                helperWindowHandle;
    HDEVNOTIFY          deviceNotificationHandle;
    DWORD               foregroundLockTimeout;
    int                 acquiredMonitorCount;
    char*               clipboardString;
    short int           keycodes[512];
    short int           scancodes[RBLXWIN_KEY_LAST + 1];
    char                keynames[RBLXWIN_KEY_LAST + 1][5];
    // Where to place the cursor when re-enabled
    double              restoreCursorPosX, restoreCursorPosY;
    // The window whose disabled cursor mode is active
    _RBLXWINwindow*        disabledCursorWindow;
    RAWINPUT*           rawInput;
    int                 rawInputSize;
    UINT                mouseTrailSize;

    struct {
        HINSTANCE                       instance;
        PFN_DirectInput8Create          Create;
        IDirectInput8W*                 api;
    } dinput8;

    struct {
        HINSTANCE                       instance;
        PFN_XInputGetCapabilities       GetCapabilities;
        PFN_XInputGetState              GetState;
    } xinput;

    struct {
        HINSTANCE                       instance;
        PFN_SetProcessDPIAware          SetProcessDPIAware_;
        PFN_ChangeWindowMessageFilterEx ChangeWindowMessageFilterEx_;
        PFN_EnableNonClientDpiScaling   EnableNonClientDpiScaling_;
        PFN_SetProcessDpiAwarenessContext SetProcessDpiAwarenessContext_;
        PFN_GetDpiForWindow             GetDpiForWindow_;
        PFN_AdjustWindowRectExForDpi    AdjustWindowRectExForDpi_;
        PFN_GetSystemMetricsForDpi      GetSystemMetricsForDpi_;
    } user32;

    struct {
        HINSTANCE                       instance;
        PFN_DwmIsCompositionEnabled     IsCompositionEnabled;
        PFN_DwmFlush                    Flush;
        PFN_DwmEnableBlurBehindWindow   EnableBlurBehindWindow;
        PFN_DwmGetColorizationColor     GetColorizationColor;
    } dwmapi;

    struct {
        HINSTANCE                       instance;
        PFN_SetProcessDpiAwareness      SetProcessDpiAwareness_;
        PFN_GetDpiForMonitor            GetDpiForMonitor_;
    } shcore;

    struct {
        HINSTANCE                       instance;
        PFN_RtlVerifyVersionInfo        RtlVerifyVersionInfo_;
    } ntdll;
} _RBLXWINlibraryWin32;

// Win32-specific per-monitor data
//
typedef struct _RBLXWINmonitorWin32
{
    HMONITOR            handle;
    // This size matches the static size of DISPLAY_DEVICE.DeviceName
    WCHAR               adapterName[32];
    WCHAR               displayName[32];
    char                publicAdapterName[32];
    char                publicDisplayName[32];
    RBLXWINbool            modesPruned;
    RBLXWINbool            modeChanged;
} _RBLXWINmonitorWin32;

// Win32-specific per-cursor data
//
typedef struct _RBLXWINcursorWin32
{
    HCURSOR             handle;
} _RBLXWINcursorWin32;

// Win32-specific global timer data
//
typedef struct _RBLXWINtimerWin32
{
    uint64_t            frequency;
} _RBLXWINtimerWin32;

// Win32-specific thread local storage data
//
typedef struct _RBLXWINtlsWin32
{
    RBLXWINbool            allocated;
    DWORD               index;
} _RBLXWINtlsWin32;

// Win32-specific mutex data
//
typedef struct _RBLXWINmutexWin32
{
    RBLXWINbool            allocated;
    CRITICAL_SECTION    section;
} _RBLXWINmutexWin32;


RBLXWINbool _rblxwinRegisterWindowClassWin32(void);
void _rblxwinUnregisterWindowClassWin32(void);

WCHAR* _rblxwinCreateWideStringFromUTF8Win32(const char* source);
char* _rblxwinCreateUTF8FromWideStringWin32(const WCHAR* source);
BOOL _rblxwinIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp);
BOOL _rblxwinIsWindows10BuildOrGreaterWin32(WORD build);
void _rblxwinInputErrorWin32(int error, const char* description);
void _rblxwinUpdateKeyNamesWin32(void);

void _rblxwinInitTimerWin32(void);

void _rblxwinPollMonitorsWin32(void);
void _rblxwinSetVideoModeWin32(_RBLXWINmonitor* monitor, const RBLXWINvidmode* desired);
void _rblxwinRestoreVideoModeWin32(_RBLXWINmonitor* monitor);
void _rblxwinGetMonitorContentScaleWin32(HMONITOR handle, float* xscale, float* yscale);

