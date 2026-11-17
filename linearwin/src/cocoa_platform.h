//========================================================================
// LINEARWIN 3.3 macOS - www.linearwin.org
//------------------------------------------------------------------------
// Copyright (c) 2009-2019 Camilla Löwy <elmindreda@linearwin.org>
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

#include <stdint.h>
#include <dlfcn.h>

#include <Carbon/Carbon.h>

// NOTE: All of NSGL was deprecated in the 10.14 SDK
//       This disables the pointless warnings for every symbol we use
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif

#if defined(__OBJC__)
#import <Cocoa/Cocoa.h>
#else
typedef void* id;
#endif

// NOTE: Many Cocoa enum values have been renamed and we need to build across
//       SDK versions where one is unavailable or deprecated.
//       We use the newer names in code and replace them with the older names if
//       the base SDK does not provide the newer names.

#if MAC_OS_X_VERSION_MAX_ALLOWED < 101200
 #define NSBitmapFormatAlphaNonpremultiplied NSAlphaNonpremultipliedBitmapFormat
 #define NSEventMaskAny NSAnyEventMask
 #define NSEventMaskKeyUp NSKeyUpMask
 #define NSEventModifierFlagCapsLock NSAlphaShiftKeyMask
 #define NSEventModifierFlagCommand NSCommandKeyMask
 #define NSEventModifierFlagControl NSControlKeyMask
 #define NSEventModifierFlagDeviceIndependentFlagsMask NSDeviceIndependentModifierFlagsMask
 #define NSEventModifierFlagOption NSAlternateKeyMask
 #define NSEventModifierFlagShift NSShiftKeyMask
 #define NSEventTypeApplicationDefined NSApplicationDefined
 #define NSWindowStyleMaskBorderless NSBorderlessWindowMask
 #define NSWindowStyleMaskClosable NSClosableWindowMask
 #define NSWindowStyleMaskMiniaturizable NSMiniaturizableWindowMask
 #define NSWindowStyleMaskResizable NSResizableWindowMask
 #define NSWindowStyleMaskTitled NSTitledWindowMask
#endif

// NOTE: Many Cocoa dynamically linked constants have been renamed and we need
//       to build across SDK versions where one is unavailable or deprecated.
//       We use the newer names in code and replace them with the older names if
//       the deployment target is older than the newer names.

#if MAC_OS_X_VERSION_MIN_REQUIRED < 101300
 #define NSPasteboardTypeURL NSURLPboardType
#endif

typedef VkFlags VkMacOSSurfaceCreateFlagsMVK;
typedef VkFlags VkMetalSurfaceCreateFlagsEXT;

typedef struct VkMacOSSurfaceCreateInfoMVK
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkMacOSSurfaceCreateFlagsMVK    flags;
    const void*                     pView;
} VkMacOSSurfaceCreateInfoMVK;

typedef struct VkMetalSurfaceCreateInfoEXT
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkMetalSurfaceCreateFlagsEXT    flags;
    const void*                     pLayer;
} VkMetalSurfaceCreateInfoEXT;

typedef VkResult (APIENTRY *PFN_vkCreateMacOSSurfaceMVK)(VkInstance,const VkMacOSSurfaceCreateInfoMVK*,const VkAllocationCallbacks*,VkSurfaceKHR*);
typedef VkResult (APIENTRY *PFN_vkCreateMetalSurfaceEXT)(VkInstance,const VkMetalSurfaceCreateInfoEXT*,const VkAllocationCallbacks*,VkSurfaceKHR*);

#include "posix_thread.h"
#include "cocoa_joystick.h"
#include "nsgl_context.h"
#include "egl_context.h"
#include "osmesa_context.h"

#define _linearwin_dlopen(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL)
#define _linearwin_dlclose(handle) dlclose(handle)
#define _linearwin_dlsym(handle, name) dlsym(handle, name)

#define _LINEARWIN_EGL_NATIVE_WINDOW  ((EGLNativeWindowType) window->ns.layer)
#define _LINEARWIN_EGL_NATIVE_DISPLAY EGL_DEFAULT_DISPLAY

#define _LINEARWIN_PLATFORM_WINDOW_STATE         _LINEARWINwindowNS  ns
#define _LINEARWIN_PLATFORM_LIBRARY_WINDOW_STATE _LINEARWINlibraryNS ns
#define _LINEARWIN_PLATFORM_LIBRARY_TIMER_STATE  _LINEARWINtimerNS   ns
#define _LINEARWIN_PLATFORM_MONITOR_STATE        _LINEARWINmonitorNS ns
#define _LINEARWIN_PLATFORM_CURSOR_STATE         _LINEARWINcursorNS  ns

// HIToolbox.framework pointer typedefs
#define kTISPropertyUnicodeKeyLayoutData _linearwin.ns.tis.kPropertyUnicodeKeyLayoutData
typedef TISInputSourceRef (*PFN_TISCopyCurrentKeyboardLayoutInputSource)(void);
#define TISCopyCurrentKeyboardLayoutInputSource _linearwin.ns.tis.CopyCurrentKeyboardLayoutInputSource
typedef void* (*PFN_TISGetInputSourceProperty)(TISInputSourceRef,CFStringRef);
#define TISGetInputSourceProperty _linearwin.ns.tis.GetInputSourceProperty
typedef UInt8 (*PFN_LMGetKbdType)(void);
#define LMGetKbdType _linearwin.ns.tis.GetKbdType


// Cocoa-specific per-window data
//
typedef struct _LINEARWINwindowNS
{
    id              object;
    id              delegate;
    id              view;
    id              layer;

    LINEARWINbool        maximized;
    LINEARWINbool        occluded;
    LINEARWINbool        retina;

    // Cached window properties to filter out duplicate events
    int             width, height;
    int             fbWidth, fbHeight;
    float           xscale, yscale;

    // The total sum of the distances the cursor has been warped
    // since the last cursor motion event was processed
    // This is kept to counteract Cocoa doing the same internally
    double          cursorWarpDeltaX, cursorWarpDeltaY;
} _LINEARWINwindowNS;

// Cocoa-specific global data
//
typedef struct _LINEARWINlibraryNS
{
    CGEventSourceRef    eventSource;
    id                  delegate;
    LINEARWINbool            finishedLaunching;
    LINEARWINbool            cursorHidden;
    TISInputSourceRef   inputSource;
    IOHIDManagerRef     hidManager;
    id                  unicodeData;
    id                  helper;
    id                  keyUpMonitor;
    id                  nibObjects;

    char                keynames[LINEARWIN_KEY_LAST + 1][17];
    short int           keycodes[256];
    short int           scancodes[LINEARWIN_KEY_LAST + 1];
    char*               clipboardString;
    CGPoint             cascadePoint;
    // Where to place the cursor when re-enabled
    double              restoreCursorPosX, restoreCursorPosY;
    // The window whose disabled cursor mode is active
    _LINEARWINwindow*        disabledCursorWindow;

    struct {
        CFBundleRef     bundle;
        PFN_TISCopyCurrentKeyboardLayoutInputSource CopyCurrentKeyboardLayoutInputSource;
        PFN_TISGetInputSourceProperty GetInputSourceProperty;
        PFN_LMGetKbdType GetKbdType;
        CFStringRef     kPropertyUnicodeKeyLayoutData;
    } tis;
} _LINEARWINlibraryNS;

// Cocoa-specific per-monitor data
//
typedef struct _LINEARWINmonitorNS
{
    CGDirectDisplayID   displayID;
    CGDisplayModeRef    previousMode;
    uint32_t            unitNumber;
    id                  screen;
    double              fallbackRefreshRate;
} _LINEARWINmonitorNS;

// Cocoa-specific per-cursor data
//
typedef struct _LINEARWINcursorNS
{
    id              object;
} _LINEARWINcursorNS;

// Cocoa-specific global timer data
//
typedef struct _LINEARWINtimerNS
{
    uint64_t        frequency;
} _LINEARWINtimerNS;


void _linearwinInitTimerNS(void);

void _linearwinPollMonitorsNS(void);
void _linearwinSetVideoModeNS(_LINEARWINmonitor* monitor, const LINEARWINvidmode* desired);
void _linearwinRestoreVideoModeNS(_LINEARWINmonitor* monitor);

float _linearwinTransformYNS(float y);

void* _linearwinLoadLocalVulkanLoaderNS(void);

