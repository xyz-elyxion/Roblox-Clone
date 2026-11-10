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
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <wchar.h>


// Callback for EnumDisplayMonitors in createMonitor
//
static BOOL CALLBACK monitorCallback(HMONITOR handle,
                                     HDC dc,
                                     RECT* rect,
                                     LPARAM data)
{
    MONITORINFOEXW mi;
    ZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);

    if (GetMonitorInfoW(handle, (MONITORINFO*) &mi))
    {
        _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) data;
        if (wcscmp(mi.szDevice, monitor->win32.adapterName) == 0)
            monitor->win32.handle = handle;
    }

    return TRUE;
}

// Create monitor from an adapter and (optionally) a display
//
static _RBLXWINmonitor* createMonitor(DISPLAY_DEVICEW* adapter,
                                   DISPLAY_DEVICEW* display)
{
    _RBLXWINmonitor* monitor;
    int widthMM, heightMM;
    char* name;
    HDC dc;
    DEVMODEW dm;
    RECT rect;

    if (display)
        name = _rblxwinCreateUTF8FromWideStringWin32(display->DeviceString);
    else
        name = _rblxwinCreateUTF8FromWideStringWin32(adapter->DeviceString);
    if (!name)
        return NULL;

    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);
    EnumDisplaySettingsW(adapter->DeviceName, ENUM_CURRENT_SETTINGS, &dm);

    dc = CreateDCW(L"DISPLAY", adapter->DeviceName, NULL, NULL);

    if (IsWindows8Point1OrGreater())
    {
        widthMM  = GetDeviceCaps(dc, HORZSIZE);
        heightMM = GetDeviceCaps(dc, VERTSIZE);
    }
    else
    {
        widthMM  = (int) (dm.dmPelsWidth * 25.4f / GetDeviceCaps(dc, LOGPIXELSX));
        heightMM = (int) (dm.dmPelsHeight * 25.4f / GetDeviceCaps(dc, LOGPIXELSY));
    }

    DeleteDC(dc);

    monitor = _rblxwinAllocMonitor(name, widthMM, heightMM);
    free(name);

    if (adapter->StateFlags & DISPLAY_DEVICE_MODESPRUNED)
        monitor->win32.modesPruned = RBLXWIN_TRUE;

    wcscpy(monitor->win32.adapterName, adapter->DeviceName);
    WideCharToMultiByte(CP_UTF8, 0,
                        adapter->DeviceName, -1,
                        monitor->win32.publicAdapterName,
                        sizeof(monitor->win32.publicAdapterName),
                        NULL, NULL);

    if (display)
    {
        wcscpy(monitor->win32.displayName, display->DeviceName);
        WideCharToMultiByte(CP_UTF8, 0,
                            display->DeviceName, -1,
                            monitor->win32.publicDisplayName,
                            sizeof(monitor->win32.publicDisplayName),
                            NULL, NULL);
    }

    rect.left   = dm.dmPosition.x;
    rect.top    = dm.dmPosition.y;
    rect.right  = dm.dmPosition.x + dm.dmPelsWidth;
    rect.bottom = dm.dmPosition.y + dm.dmPelsHeight;

    EnumDisplayMonitors(NULL, &rect, monitorCallback, (LPARAM) monitor);
    return monitor;
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Poll for changes in the set of connected monitors
//
void _rblxwinPollMonitorsWin32(void)
{
    int i, disconnectedCount;
    _RBLXWINmonitor** disconnected = NULL;
    DWORD adapterIndex, displayIndex;
    DISPLAY_DEVICEW adapter, display;
    _RBLXWINmonitor* monitor;

    disconnectedCount = _rblxwin.monitorCount;
    if (disconnectedCount)
    {
        disconnected = calloc(_rblxwin.monitorCount, sizeof(_RBLXWINmonitor*));
        memcpy(disconnected,
               _rblxwin.monitors,
               _rblxwin.monitorCount * sizeof(_RBLXWINmonitor*));
    }

    for (adapterIndex = 0;  ;  adapterIndex++)
    {
        int type = _RBLXWIN_INSERT_LAST;

        ZeroMemory(&adapter, sizeof(adapter));
        adapter.cb = sizeof(adapter);

        if (!EnumDisplayDevicesW(NULL, adapterIndex, &adapter, 0))
            break;

        if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE))
            continue;

        if (adapter.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
            type = _RBLXWIN_INSERT_FIRST;

        for (displayIndex = 0;  ;  displayIndex++)
        {
            ZeroMemory(&display, sizeof(display));
            display.cb = sizeof(display);

            if (!EnumDisplayDevicesW(adapter.DeviceName, displayIndex, &display, 0))
                break;

            if (!(display.StateFlags & DISPLAY_DEVICE_ACTIVE))
                continue;

            for (i = 0;  i < disconnectedCount;  i++)
            {
                if (disconnected[i] &&
                    wcscmp(disconnected[i]->win32.displayName,
                           display.DeviceName) == 0)
                {
                    disconnected[i] = NULL;
                    // handle may have changed, update
                    EnumDisplayMonitors(NULL, NULL, monitorCallback, (LPARAM) _rblxwin.monitors[i]);
                    break;
                }
            }

            if (i < disconnectedCount)
                continue;

            monitor = createMonitor(&adapter, &display);
            if (!monitor)
            {
                free(disconnected);
                return;
            }

            _rblxwinInputMonitor(monitor, RBLXWIN_CONNECTED, type);

            type = _RBLXWIN_INSERT_LAST;
        }

        // HACK: If an active adapter does not have any display devices
        //       (as sometimes happens), add it directly as a monitor
        if (displayIndex == 0)
        {
            for (i = 0;  i < disconnectedCount;  i++)
            {
                if (disconnected[i] &&
                    wcscmp(disconnected[i]->win32.adapterName,
                           adapter.DeviceName) == 0)
                {
                    disconnected[i] = NULL;
                    break;
                }
            }

            if (i < disconnectedCount)
                continue;

            monitor = createMonitor(&adapter, NULL);
            if (!monitor)
            {
                free(disconnected);
                return;
            }

            _rblxwinInputMonitor(monitor, RBLXWIN_CONNECTED, type);
        }
    }

    for (i = 0;  i < disconnectedCount;  i++)
    {
        if (disconnected[i])
            _rblxwinInputMonitor(disconnected[i], RBLXWIN_DISCONNECTED, 0);
    }

    free(disconnected);
}

// Change the current video mode
//
void _rblxwinSetVideoModeWin32(_RBLXWINmonitor* monitor, const RBLXWINvidmode* desired)
{
    RBLXWINvidmode current;
    const RBLXWINvidmode* best;
    DEVMODEW dm;
    LONG result;

    best = _rblxwinChooseVideoMode(monitor, desired);
    _rblxwinPlatformGetVideoMode(monitor, &current);
    if (_rblxwinCompareVideoModes(&current, best) == 0)
        return;

    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);
    dm.dmFields           = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL |
                            DM_DISPLAYFREQUENCY;
    dm.dmPelsWidth        = best->width;
    dm.dmPelsHeight       = best->height;
    dm.dmBitsPerPel       = best->redBits + best->greenBits + best->blueBits;
    dm.dmDisplayFrequency = best->refreshRate;

    if (dm.dmBitsPerPel < 15 || dm.dmBitsPerPel >= 24)
        dm.dmBitsPerPel = 32;

    result = ChangeDisplaySettingsExW(monitor->win32.adapterName,
                                      &dm,
                                      NULL,
                                      CDS_FULLSCREEN,
                                      NULL);
    if (result == DISP_CHANGE_SUCCESSFUL)
        monitor->win32.modeChanged = RBLXWIN_TRUE;
    else
    {
        const char* description = "Unknown error";

        if (result == DISP_CHANGE_BADDUALVIEW)
            description = "The system uses DualView";
        else if (result == DISP_CHANGE_BADFLAGS)
            description = "Invalid flags";
        else if (result == DISP_CHANGE_BADMODE)
            description = "Graphics mode not supported";
        else if (result == DISP_CHANGE_BADPARAM)
            description = "Invalid parameter";
        else if (result == DISP_CHANGE_FAILED)
            description = "Graphics mode failed";
        else if (result == DISP_CHANGE_NOTUPDATED)
            description = "Failed to write to registry";
        else if (result == DISP_CHANGE_RESTART)
            description = "Computer restart required";

        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Win32: Failed to set video mode: %s",
                        description);
    }
}

// Restore the previously saved (original) video mode
//
void _rblxwinRestoreVideoModeWin32(_RBLXWINmonitor* monitor)
{
    if (monitor->win32.modeChanged)
    {
        ChangeDisplaySettingsExW(monitor->win32.adapterName,
                                 NULL, NULL, CDS_FULLSCREEN, NULL);
        monitor->win32.modeChanged = RBLXWIN_FALSE;
    }
}

void _rblxwinGetMonitorContentScaleWin32(HMONITOR handle, float* xscale, float* yscale)
{
    UINT xdpi, ydpi;

    if (xscale)
        *xscale = 0.f;
    if (yscale)
        *yscale = 0.f;

    if (IsWindows8Point1OrGreater())
    {
        if (GetDpiForMonitor(handle, MDT_EFFECTIVE_DPI, &xdpi, &ydpi) != S_OK)
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR, "Win32: Failed to query monitor DPI");
            return;
        }
    }
    else
    {
        const HDC dc = GetDC(NULL);
        xdpi = GetDeviceCaps(dc, LOGPIXELSX);
        ydpi = GetDeviceCaps(dc, LOGPIXELSY);
        ReleaseDC(NULL, dc);
    }

    if (xscale)
        *xscale = xdpi / (float) USER_DEFAULT_SCREEN_DPI;
    if (yscale)
        *yscale = ydpi / (float) USER_DEFAULT_SCREEN_DPI;
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _rblxwinPlatformFreeMonitor(_RBLXWINmonitor* monitor)
{
}

void _rblxwinPlatformGetMonitorPos(_RBLXWINmonitor* monitor, int* xpos, int* ypos)
{
    DEVMODEW dm;
    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);

    EnumDisplaySettingsExW(monitor->win32.adapterName,
                           ENUM_CURRENT_SETTINGS,
                           &dm,
                           EDS_ROTATEDMODE);

    if (xpos)
        *xpos = dm.dmPosition.x;
    if (ypos)
        *ypos = dm.dmPosition.y;
}

void _rblxwinPlatformGetMonitorContentScale(_RBLXWINmonitor* monitor,
                                         float* xscale, float* yscale)
{
    _rblxwinGetMonitorContentScaleWin32(monitor->win32.handle, xscale, yscale);
}

void _rblxwinPlatformGetMonitorWorkarea(_RBLXWINmonitor* monitor,
                                     int* xpos, int* ypos,
                                     int* width, int* height)
{
    MONITORINFO mi = { sizeof(mi) };
    GetMonitorInfoW(monitor->win32.handle, &mi);

    if (xpos)
        *xpos = mi.rcWork.left;
    if (ypos)
        *ypos = mi.rcWork.top;
    if (width)
        *width = mi.rcWork.right - mi.rcWork.left;
    if (height)
        *height = mi.rcWork.bottom - mi.rcWork.top;
}

RBLXWINvidmode* _rblxwinPlatformGetVideoModes(_RBLXWINmonitor* monitor, int* count)
{
    int modeIndex = 0, size = 0;
    RBLXWINvidmode* result = NULL;

    *count = 0;

    for (;;)
    {
        int i;
        RBLXWINvidmode mode;
        DEVMODEW dm;

        ZeroMemory(&dm, sizeof(dm));
        dm.dmSize = sizeof(dm);

        if (!EnumDisplaySettingsW(monitor->win32.adapterName, modeIndex, &dm))
            break;

        modeIndex++;

        // Skip modes with less than 15 BPP
        if (dm.dmBitsPerPel < 15)
            continue;

        mode.width  = dm.dmPelsWidth;
        mode.height = dm.dmPelsHeight;
        mode.refreshRate = dm.dmDisplayFrequency;
        _rblxwinSplitBPP(dm.dmBitsPerPel,
                      &mode.redBits,
                      &mode.greenBits,
                      &mode.blueBits);

        for (i = 0;  i < *count;  i++)
        {
            if (_rblxwinCompareVideoModes(result + i, &mode) == 0)
                break;
        }

        // Skip duplicate modes
        if (i < *count)
            continue;

        if (monitor->win32.modesPruned)
        {
            // Skip modes not supported by the connected displays
            if (ChangeDisplaySettingsExW(monitor->win32.adapterName,
                                         &dm,
                                         NULL,
                                         CDS_TEST,
                                         NULL) != DISP_CHANGE_SUCCESSFUL)
            {
                continue;
            }
        }

        if (*count == size)
        {
            size += 128;
            result = (RBLXWINvidmode*) realloc(result, size * sizeof(RBLXWINvidmode));
        }

        (*count)++;
        result[*count - 1] = mode;
    }

    if (!*count)
    {
        // HACK: Report the current mode if no valid modes were found
        result = calloc(1, sizeof(RBLXWINvidmode));
        _rblxwinPlatformGetVideoMode(monitor, result);
        *count = 1;
    }

    return result;
}

void _rblxwinPlatformGetVideoMode(_RBLXWINmonitor* monitor, RBLXWINvidmode* mode)
{
    DEVMODEW dm;
    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);

    EnumDisplaySettingsW(monitor->win32.adapterName, ENUM_CURRENT_SETTINGS, &dm);

    mode->width  = dm.dmPelsWidth;
    mode->height = dm.dmPelsHeight;
    mode->refreshRate = dm.dmDisplayFrequency;
    _rblxwinSplitBPP(dm.dmBitsPerPel,
                  &mode->redBits,
                  &mode->greenBits,
                  &mode->blueBits);
}

RBLXWINbool _rblxwinPlatformGetGammaRamp(_RBLXWINmonitor* monitor, RBLXWINgammaramp* ramp)
{
    HDC dc;
    WORD values[3][256];

    dc = CreateDCW(L"DISPLAY", monitor->win32.adapterName, NULL, NULL);
    GetDeviceGammaRamp(dc, values);
    DeleteDC(dc);

    _rblxwinAllocGammaArrays(ramp, 256);

    memcpy(ramp->red,   values[0], sizeof(values[0]));
    memcpy(ramp->green, values[1], sizeof(values[1]));
    memcpy(ramp->blue,  values[2], sizeof(values[2]));

    return RBLXWIN_TRUE;
}

void _rblxwinPlatformSetGammaRamp(_RBLXWINmonitor* monitor, const RBLXWINgammaramp* ramp)
{
    HDC dc;
    WORD values[3][256];

    if (ramp->size != 256)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Win32: Gamma ramp size must be 256");
        return;
    }

    memcpy(values[0], ramp->red,   sizeof(values[0]));
    memcpy(values[1], ramp->green, sizeof(values[1]));
    memcpy(values[2], ramp->blue,  sizeof(values[2]));

    dc = CreateDCW(L"DISPLAY", monitor->win32.adapterName, NULL, NULL);
    SetDeviceGammaRamp(dc, values);
    DeleteDC(dc);
}


//////////////////////////////////////////////////////////////////////////
//////                        RBLXWIN native API                       //////
//////////////////////////////////////////////////////////////////////////

RBLXWINAPI const char* rblxwinGetWin32Adapter(RBLXWINmonitor* handle)
{
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) handle;
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->win32.publicAdapterName;
}

RBLXWINAPI const char* rblxwinGetWin32Monitor(RBLXWINmonitor* handle)
{
    _RBLXWINmonitor* monitor = (_RBLXWINmonitor*) handle;
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->win32.publicDisplayName;
}

