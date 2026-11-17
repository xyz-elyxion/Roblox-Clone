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

#pragma once

#if defined(_LINEARWIN_USE_CONFIG_H)
 #include "linearwin_config.h"
#endif

#if defined(LINEARWIN_INCLUDE_GLCOREARB) || \
    defined(LINEARWIN_INCLUDE_ES1)       || \
    defined(LINEARWIN_INCLUDE_ES2)       || \
    defined(LINEARWIN_INCLUDE_ES3)       || \
    defined(LINEARWIN_INCLUDE_ES31)      || \
    defined(LINEARWIN_INCLUDE_ES32)      || \
    defined(LINEARWIN_INCLUDE_NONE)      || \
    defined(LINEARWIN_INCLUDE_GLEXT)     || \
    defined(LINEARWIN_INCLUDE_GLU)       || \
    defined(LINEARWIN_INCLUDE_VULKAN)    || \
    defined(LINEARWIN_DLL)
 #error "You must not define any header option macros when compiling LINEARWIN"
#endif

#define LINEARWIN_INCLUDE_NONE
#include "../include/LINEARWIN/linearwin.h"

#define _LINEARWIN_INSERT_FIRST      0
#define _LINEARWIN_INSERT_LAST       1

#define _LINEARWIN_POLL_PRESENCE     0
#define _LINEARWIN_POLL_AXES         1
#define _LINEARWIN_POLL_BUTTONS      2
#define _LINEARWIN_POLL_ALL          (_LINEARWIN_POLL_AXES | _LINEARWIN_POLL_BUTTONS)

#define _LINEARWIN_MESSAGE_SIZE      1024

typedef int LINEARWINbool;

typedef struct _LINEARWINerror       _LINEARWINerror;
typedef struct _LINEARWINinitconfig  _LINEARWINinitconfig;
typedef struct _LINEARWINwndconfig   _LINEARWINwndconfig;
typedef struct _LINEARWINctxconfig   _LINEARWINctxconfig;
typedef struct _LINEARWINfbconfig    _LINEARWINfbconfig;
typedef struct _LINEARWINcontext     _LINEARWINcontext;
typedef struct _LINEARWINwindow      _LINEARWINwindow;
typedef struct _LINEARWINlibrary     _LINEARWINlibrary;
typedef struct _LINEARWINmonitor     _LINEARWINmonitor;
typedef struct _LINEARWINcursor      _LINEARWINcursor;
typedef struct _LINEARWINmapelement  _LINEARWINmapelement;
typedef struct _LINEARWINmapping     _LINEARWINmapping;
typedef struct _LINEARWINjoystick    _LINEARWINjoystick;
typedef struct _LINEARWINtls         _LINEARWINtls;
typedef struct _LINEARWINmutex       _LINEARWINmutex;

typedef void (* _LINEARWINmakecontextcurrentfun)(_LINEARWINwindow*);
typedef void (* _LINEARWINswapbuffersfun)(_LINEARWINwindow*);
typedef void (* _LINEARWINswapintervalfun)(int);
typedef int (* _LINEARWINextensionsupportedfun)(const char*);
typedef LINEARWINglproc (* _LINEARWINgetprocaddressfun)(const char*);
typedef void (* _LINEARWINdestroycontextfun)(_LINEARWINwindow*);

#define GL_VERSION 0x1f02
#define GL_NONE 0
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_UNSIGNED_BYTE 0x1401
#define GL_EXTENSIONS 0x1f03
#define GL_NUM_EXTENSIONS 0x821d
#define GL_CONTEXT_FLAGS 0x821e
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x00000001
#define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#define GL_CONTEXT_PROFILE_MASK 0x9126
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#define GL_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
#define GL_LOSE_CONTEXT_ON_RESET_ARB 0x8252
#define GL_NO_RESET_NOTIFICATION_ARB 0x8261
#define GL_CONTEXT_RELEASE_BEHAVIOR 0x82fb
#define GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH 0x82fc
#define GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR 0x00000008

typedef int GLint;
typedef unsigned int GLuint;
typedef unsigned int GLenum;
typedef unsigned int GLbitfield;
typedef unsigned char GLubyte;

typedef void (APIENTRY * PFNGLCLEARPROC)(GLbitfield);
typedef const GLubyte* (APIENTRY * PFNGLGETSTRINGPROC)(GLenum);
typedef void (APIENTRY * PFNGLGETINTEGERVPROC)(GLenum,GLint*);
typedef const GLubyte* (APIENTRY * PFNGLGETSTRINGIPROC)(GLenum,GLuint);

#define VK_NULL_HANDLE 0

typedef void* VkInstance;
typedef void* VkPhysicalDevice;
typedef uint64_t VkSurfaceKHR;
typedef uint32_t VkFlags;
typedef uint32_t VkBool32;

typedef enum VkStructureType
{
    VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR = 1000004000,
    VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR = 1000005000,
    VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR = 1000006000,
    VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR = 1000009000,
    VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK = 1000123000,
    VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT = 1000217000,
    VK_STRUCTURE_TYPE_MAX_ENUM = 0x7FFFFFFF
} VkStructureType;

typedef enum VkResult
{
    VK_SUCCESS = 0,
    VK_NOT_READY = 1,
    VK_TIMEOUT = 2,
    VK_EVENT_SET = 3,
    VK_EVENT_RESET = 4,
    VK_INCOMPLETE = 5,
    VK_ERROR_OUT_OF_HOST_MEMORY = -1,
    VK_ERROR_OUT_OF_DEVICE_MEMORY = -2,
    VK_ERROR_INITIALIZATION_FAILED = -3,
    VK_ERROR_DEVICE_LOST = -4,
    VK_ERROR_MEMORY_MAP_FAILED = -5,
    VK_ERROR_LAYER_NOT_PRESENT = -6,
    VK_ERROR_EXTENSION_NOT_PRESENT = -7,
    VK_ERROR_FEATURE_NOT_PRESENT = -8,
    VK_ERROR_INCOMPATIBLE_DRIVER = -9,
    VK_ERROR_TOO_MANY_OBJECTS = -10,
    VK_ERROR_FORMAT_NOT_SUPPORTED = -11,
    VK_ERROR_SURFACE_LOST_KHR = -1000000000,
    VK_SUBOPTIMAL_KHR = 1000001003,
    VK_ERROR_OUT_OF_DATE_KHR = -1000001004,
    VK_ERROR_INCOMPATIBLE_DISPLAY_KHR = -1000003001,
    VK_ERROR_NATIVE_WINDOW_IN_USE_KHR = -1000000001,
    VK_ERROR_VALIDATION_FAILED_EXT = -1000011001,
    VK_RESULT_MAX_ENUM = 0x7FFFFFFF
} VkResult;

typedef struct VkAllocationCallbacks VkAllocationCallbacks;

typedef struct VkExtensionProperties
{
    char            extensionName[256];
    uint32_t        specVersion;
} VkExtensionProperties;

typedef void (APIENTRY * PFN_vkVoidFunction)(void);

#if defined(_LINEARWIN_VULKAN_STATIC)
  PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance,const char*);
  VkResult vkEnumerateInstanceExtensionProperties(const char*,uint32_t*,VkExtensionProperties*);
#else
  typedef PFN_vkVoidFunction (APIENTRY * PFN_vkGetInstanceProcAddr)(VkInstance,const char*);
  typedef VkResult (APIENTRY * PFN_vkEnumerateInstanceExtensionProperties)(const char*,uint32_t*,VkExtensionProperties*);
  #define vkEnumerateInstanceExtensionProperties _linearwin.vk.EnumerateInstanceExtensionProperties
  #define vkGetInstanceProcAddr _linearwin.vk.GetInstanceProcAddr
#endif

#if defined(_LINEARWIN_COCOA)
 #include "cocoa_platform.h"
#elif defined(_LINEARWIN_WIN32)
 #include "win32_platform.h"
#elif defined(_LINEARWIN_X11)
 #include "x11_platform.h"
#elif defined(_LINEARWIN_WAYLAND)
 #include "wl_platform.h"
#elif defined(_LINEARWIN_OSMESA)
 #include "null_platform.h"
#else
 #error "No supported window creation API selected"
#endif

// Constructs a version number string from the public header macros
#define _LINEARWIN_CONCAT_VERSION(m, n, r) #m "." #n "." #r
#define _LINEARWIN_MAKE_VERSION(m, n, r) _LINEARWIN_CONCAT_VERSION(m, n, r)
#define _LINEARWIN_VERSION_NUMBER _LINEARWIN_MAKE_VERSION(LINEARWIN_VERSION_MAJOR, \
                                                LINEARWIN_VERSION_MINOR, \
                                                LINEARWIN_VERSION_REVISION)

// Checks for whether the library has been initialized
#define _LINEARWIN_REQUIRE_INIT()                         \
    if (!_linearwin.initialized)                          \
    {                                                \
        _linearwinInputError(LINEARWIN_NOT_INITIALIZED, NULL); \
        return;                                      \
    }
#define _LINEARWIN_REQUIRE_INIT_OR_RETURN(x)              \
    if (!_linearwin.initialized)                          \
    {                                                \
        _linearwinInputError(LINEARWIN_NOT_INITIALIZED, NULL); \
        return x;                                    \
    }

// Swaps the provided pointers
#define _LINEARWIN_SWAP_POINTERS(x, y) \
    {                             \
        void* t;                  \
        t = x;                    \
        x = y;                    \
        y = t;                    \
    }

// Per-thread error structure
//
struct _LINEARWINerror
{
    _LINEARWINerror*     next;
    int             code;
    char            description[_LINEARWIN_MESSAGE_SIZE];
};

// Initialization configuration
//
// Parameters relating to the initialization of the library
//
struct _LINEARWINinitconfig
{
    LINEARWINbool      hatButtons;
    struct {
        LINEARWINbool  menubar;
        LINEARWINbool  chdir;
    } ns;
};

// Window configuration
//
// Parameters relating to the creation of the window but not directly related
// to the framebuffer.  This is used to pass window creation parameters from
// shared code to the platform API.
//
struct _LINEARWINwndconfig
{
    int           width;
    int           height;
    const char*   title;
    LINEARWINbool      resizable;
    LINEARWINbool      visible;
    LINEARWINbool      decorated;
    LINEARWINbool      focused;
    LINEARWINbool      autoIconify;
    LINEARWINbool      floating;
    LINEARWINbool      maximized;
    LINEARWINbool      centerCursor;
    LINEARWINbool      focusOnShow;
    LINEARWINbool      scaleToMonitor;
    struct {
        LINEARWINbool  retina;
        char      frameName[256];
    } ns;
    struct {
        char      className[256];
        char      instanceName[256];
    } x11;
};

// Context configuration
//
// Parameters relating to the creation of the context but not directly related
// to the framebuffer.  This is used to pass context creation parameters from
// shared code to the platform API.
//
struct _LINEARWINctxconfig
{
    int           client;
    int           source;
    int           major;
    int           minor;
    LINEARWINbool      forward;
    LINEARWINbool      debug;
    LINEARWINbool      noerror;
    int           profile;
    int           robustness;
    int           release;
    _LINEARWINwindow*  share;
    struct {
        LINEARWINbool  offline;
    } nsgl;
};

// Framebuffer configuration
//
// This describes buffers and their sizes.  It also contains
// a platform-specific ID used to map back to the backend API object.
//
// It is used to pass framebuffer parameters from shared code to the platform
// API and also to enumerate and select available framebuffer configs.
//
struct _LINEARWINfbconfig
{
    int         redBits;
    int         greenBits;
    int         blueBits;
    int         alphaBits;
    int         depthBits;
    int         stencilBits;
    int         accumRedBits;
    int         accumGreenBits;
    int         accumBlueBits;
    int         accumAlphaBits;
    int         auxBuffers;
    LINEARWINbool    stereo;
    int         samples;
    LINEARWINbool    sRGB;
    LINEARWINbool    doublebuffer;
    LINEARWINbool    transparent;
    uintptr_t   handle;
};

// Context structure
//
struct _LINEARWINcontext
{
    int                 client;
    int                 source;
    int                 major, minor, revision;
    LINEARWINbool            forward, debug, noerror;
    int                 profile;
    int                 robustness;
    int                 release;

    PFNGLGETSTRINGIPROC  GetStringi;
    PFNGLGETINTEGERVPROC GetIntegerv;
    PFNGLGETSTRINGPROC   GetString;

    _LINEARWINmakecontextcurrentfun  makeCurrent;
    _LINEARWINswapbuffersfun         swapBuffers;
    _LINEARWINswapintervalfun        swapInterval;
    _LINEARWINextensionsupportedfun  extensionSupported;
    _LINEARWINgetprocaddressfun      getProcAddress;
    _LINEARWINdestroycontextfun      destroy;

    // This is defined in the context API's context.h
    _LINEARWIN_PLATFORM_CONTEXT_STATE;
    // This is defined in egl_context.h
    _LINEARWIN_EGL_CONTEXT_STATE;
    // This is defined in osmesa_context.h
    _LINEARWIN_OSMESA_CONTEXT_STATE;
};

// Window and context structure
//
struct _LINEARWINwindow
{
    struct _LINEARWINwindow* next;

    // Window settings and state
    LINEARWINbool            resizable;
    LINEARWINbool            decorated;
    LINEARWINbool            autoIconify;
    LINEARWINbool            floating;
    LINEARWINbool            focusOnShow;
    LINEARWINbool            shouldClose;
    void*               userPointer;
    LINEARWINbool            doublebuffer;
    LINEARWINvidmode         videoMode;
    _LINEARWINmonitor*       monitor;
    _LINEARWINcursor*        cursor;

    int                 minwidth, minheight;
    int                 maxwidth, maxheight;
    int                 numer, denom;

    LINEARWINbool            stickyKeys;
    LINEARWINbool            stickyMouseButtons;
    LINEARWINbool            lockKeyMods;
    int                 cursorMode;
    char                mouseButtons[LINEARWIN_MOUSE_BUTTON_LAST + 1];
    char                keys[LINEARWIN_KEY_LAST + 1];
    // Virtual cursor position when cursor is disabled
    double              virtualCursorPosX, virtualCursorPosY;
    LINEARWINbool            rawMouseMotion;

    _LINEARWINcontext        context;

    struct {
        LINEARWINwindowposfun          pos;
        LINEARWINwindowsizefun         size;
        LINEARWINwindowclosefun        close;
        LINEARWINwindowrefreshfun      refresh;
        LINEARWINwindowfocusfun        focus;
        LINEARWINwindowiconifyfun      iconify;
        LINEARWINwindowmaximizefun     maximize;
        LINEARWINframebuffersizefun    fbsize;
        LINEARWINwindowcontentscalefun scale;
        LINEARWINmousebuttonfun        mouseButton;
        LINEARWINcursorposfun          cursorPos;
        LINEARWINcursorenterfun        cursorEnter;
        LINEARWINscrollfun             scroll;
        LINEARWINkeyfun                key;
        LINEARWINcharfun               character;
        LINEARWINcharmodsfun           charmods;
        LINEARWINdropfun               drop;
    } callbacks;

    // This is defined in the window API's platform.h
    _LINEARWIN_PLATFORM_WINDOW_STATE;
};

// Monitor structure
//
struct _LINEARWINmonitor
{
    char            name[128];
    void*           userPointer;

    // Physical dimensions in millimeters.
    int             widthMM, heightMM;

    // The window whose video mode is current on this monitor
    _LINEARWINwindow*    window;

    LINEARWINvidmode*    modes;
    int             modeCount;
    LINEARWINvidmode     currentMode;

    LINEARWINgammaramp   originalRamp;
    LINEARWINgammaramp   currentRamp;

    // This is defined in the window API's platform.h
    _LINEARWIN_PLATFORM_MONITOR_STATE;
};

// Cursor structure
//
struct _LINEARWINcursor
{
    _LINEARWINcursor*    next;

    // This is defined in the window API's platform.h
    _LINEARWIN_PLATFORM_CURSOR_STATE;
};

// Gamepad mapping element structure
//
struct _LINEARWINmapelement
{
    uint8_t         type;
    uint8_t         index;
    int8_t          axisScale;
    int8_t          axisOffset;
};

// Gamepad mapping structure
//
struct _LINEARWINmapping
{
    char            name[128];
    char            guid[33];
    _LINEARWINmapelement buttons[15];
    _LINEARWINmapelement axes[6];
};

// Joystick structure
//
struct _LINEARWINjoystick
{
    LINEARWINbool        allocated;
    LINEARWINbool        connected;
    float*          axes;
    int             axisCount;
    unsigned char*  buttons;
    int             buttonCount;
    unsigned char*  hats;
    int             hatCount;
    char            name[128];
    void*           userPointer;
    char            guid[33];
    _LINEARWINmapping*   mapping;

    // This is defined in the joystick API's joystick.h
    _LINEARWIN_PLATFORM_JOYSTICK_STATE;
};

// Thread local storage structure
//
struct _LINEARWINtls
{
    // This is defined in the platform's thread.h
    _LINEARWIN_PLATFORM_TLS_STATE;
};

// Mutex structure
//
struct _LINEARWINmutex
{
    // This is defined in the platform's thread.h
    _LINEARWIN_PLATFORM_MUTEX_STATE;
};

// Library global data
//
struct _LINEARWINlibrary
{
    LINEARWINbool            initialized;

    struct {
        _LINEARWINinitconfig init;
        _LINEARWINfbconfig   framebuffer;
        _LINEARWINwndconfig  window;
        _LINEARWINctxconfig  context;
        int             refreshRate;
    } hints;

    _LINEARWINerror*         errorListHead;
    _LINEARWINcursor*        cursorListHead;
    _LINEARWINwindow*        windowListHead;

    _LINEARWINmonitor**      monitors;
    int                 monitorCount;

    _LINEARWINjoystick       joysticks[LINEARWIN_JOYSTICK_LAST + 1];
    _LINEARWINmapping*       mappings;
    int                 mappingCount;

    _LINEARWINtls            errorSlot;
    _LINEARWINtls            contextSlot;
    _LINEARWINmutex          errorLock;

    struct {
        uint64_t        offset;
        // This is defined in the platform's time.h
        _LINEARWIN_PLATFORM_LIBRARY_TIMER_STATE;
    } timer;

    struct {
        LINEARWINbool        available;
        void*           handle;
        char*           extensions[2];
#if !defined(_LINEARWIN_VULKAN_STATIC)
        PFN_vkEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties;
        PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
#endif
        LINEARWINbool        KHR_surface;
#if defined(_LINEARWIN_WIN32)
        LINEARWINbool        KHR_win32_surface;
#elif defined(_LINEARWIN_COCOA)
        LINEARWINbool        MVK_macos_surface;
        LINEARWINbool        EXT_metal_surface;
#elif defined(_LINEARWIN_X11)
        LINEARWINbool        KHR_xlib_surface;
        LINEARWINbool        KHR_xcb_surface;
#elif defined(_LINEARWIN_WAYLAND)
        LINEARWINbool        KHR_wayland_surface;
#endif
    } vk;

    struct {
        LINEARWINmonitorfun  monitor;
        LINEARWINjoystickfun joystick;
    } callbacks;

    // This is defined in the window API's platform.h
    _LINEARWIN_PLATFORM_LIBRARY_WINDOW_STATE;
    // This is defined in the context API's context.h
    _LINEARWIN_PLATFORM_LIBRARY_CONTEXT_STATE;
    // This is defined in the platform's joystick.h
    _LINEARWIN_PLATFORM_LIBRARY_JOYSTICK_STATE;
    // This is defined in egl_context.h
    _LINEARWIN_EGL_LIBRARY_CONTEXT_STATE;
    // This is defined in osmesa_context.h
    _LINEARWIN_OSMESA_LIBRARY_CONTEXT_STATE;
};

// Global state shared between compilation units of LINEARWIN
//
extern _LINEARWINlibrary _linearwin;


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _linearwinPlatformInit(void);
void _linearwinPlatformTerminate(void);
const char* _linearwinPlatformGetVersionString(void);

void _linearwinPlatformGetCursorPos(_LINEARWINwindow* window, double* xpos, double* ypos);
void _linearwinPlatformSetCursorPos(_LINEARWINwindow* window, double xpos, double ypos);
void _linearwinPlatformSetCursorMode(_LINEARWINwindow* window, int mode);
void _linearwinPlatformSetRawMouseMotion(_LINEARWINwindow *window, LINEARWINbool enabled);
LINEARWINbool _linearwinPlatformRawMouseMotionSupported(void);
int _linearwinPlatformCreateCursor(_LINEARWINcursor* cursor,
                              const LINEARWINimage* image, int xhot, int yhot);
int _linearwinPlatformCreateStandardCursor(_LINEARWINcursor* cursor, int shape);
void _linearwinPlatformDestroyCursor(_LINEARWINcursor* cursor);
void _linearwinPlatformSetCursor(_LINEARWINwindow* window, _LINEARWINcursor* cursor);

const char* _linearwinPlatformGetScancodeName(int scancode);
int _linearwinPlatformGetKeyScancode(int key);

void _linearwinPlatformFreeMonitor(_LINEARWINmonitor* monitor);
void _linearwinPlatformGetMonitorPos(_LINEARWINmonitor* monitor, int* xpos, int* ypos);
void _linearwinPlatformGetMonitorContentScale(_LINEARWINmonitor* monitor,
                                         float* xscale, float* yscale);
void _linearwinPlatformGetMonitorWorkarea(_LINEARWINmonitor* monitor, int* xpos, int* ypos, int *width, int *height);
LINEARWINvidmode* _linearwinPlatformGetVideoModes(_LINEARWINmonitor* monitor, int* count);
void _linearwinPlatformGetVideoMode(_LINEARWINmonitor* monitor, LINEARWINvidmode* mode);
LINEARWINbool _linearwinPlatformGetGammaRamp(_LINEARWINmonitor* monitor, LINEARWINgammaramp* ramp);
void _linearwinPlatformSetGammaRamp(_LINEARWINmonitor* monitor, const LINEARWINgammaramp* ramp);

void _linearwinPlatformSetClipboardString(const char* string);
const char* _linearwinPlatformGetClipboardString(void);

int _linearwinPlatformPollJoystick(_LINEARWINjoystick* js, int mode);
void _linearwinPlatformUpdateGamepadGUID(char* guid);

uint64_t _linearwinPlatformGetTimerValue(void);
uint64_t _linearwinPlatformGetTimerFrequency(void);

int _linearwinPlatformCreateWindow(_LINEARWINwindow* window,
                              const _LINEARWINwndconfig* wndconfig,
                              const _LINEARWINctxconfig* ctxconfig,
                              const _LINEARWINfbconfig* fbconfig);
void _linearwinPlatformDestroyWindow(_LINEARWINwindow* window);
void _linearwinPlatformSetWindowTitle(_LINEARWINwindow* window, const char* title);
void _linearwinPlatformSetWindowIcon(_LINEARWINwindow* window,
                                int count, const LINEARWINimage* images);
void _linearwinPlatformGetWindowPos(_LINEARWINwindow* window, int* xpos, int* ypos);
void _linearwinPlatformSetWindowPos(_LINEARWINwindow* window, int xpos, int ypos);
void _linearwinPlatformGetWindowSize(_LINEARWINwindow* window, int* width, int* height);
void _linearwinPlatformSetWindowSize(_LINEARWINwindow* window, int width, int height);
void _linearwinPlatformSetWindowSizeLimits(_LINEARWINwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight);
void _linearwinPlatformSetWindowAspectRatio(_LINEARWINwindow* window, int numer, int denom);
void _linearwinPlatformGetFramebufferSize(_LINEARWINwindow* window, int* width, int* height);
void _linearwinPlatformGetWindowFrameSize(_LINEARWINwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom);
void _linearwinPlatformGetWindowContentScale(_LINEARWINwindow* window,
                                        float* xscale, float* yscale);
void _linearwinPlatformIconifyWindow(_LINEARWINwindow* window);
void _linearwinPlatformRestoreWindow(_LINEARWINwindow* window);
void _linearwinPlatformMaximizeWindow(_LINEARWINwindow* window);
void _linearwinPlatformShowWindow(_LINEARWINwindow* window);
void _linearwinPlatformHideWindow(_LINEARWINwindow* window);
void _linearwinPlatformRequestWindowAttention(_LINEARWINwindow* window);
void _linearwinPlatformFocusWindow(_LINEARWINwindow* window);
void _linearwinPlatformSetWindowMonitor(_LINEARWINwindow* window, _LINEARWINmonitor* monitor,
                                   int xpos, int ypos, int width, int height,
                                   int refreshRate);
int _linearwinPlatformWindowFocused(_LINEARWINwindow* window);
int _linearwinPlatformWindowIconified(_LINEARWINwindow* window);
int _linearwinPlatformWindowVisible(_LINEARWINwindow* window);
int _linearwinPlatformWindowMaximized(_LINEARWINwindow* window);
int _linearwinPlatformWindowHovered(_LINEARWINwindow* window);
int _linearwinPlatformFramebufferTransparent(_LINEARWINwindow* window);
float _linearwinPlatformGetWindowOpacity(_LINEARWINwindow* window);
void _linearwinPlatformSetWindowResizable(_LINEARWINwindow* window, LINEARWINbool enabled);
void _linearwinPlatformSetWindowDecorated(_LINEARWINwindow* window, LINEARWINbool enabled);
void _linearwinPlatformSetWindowFloating(_LINEARWINwindow* window, LINEARWINbool enabled);
void _linearwinPlatformSetWindowOpacity(_LINEARWINwindow* window, float opacity);

void _linearwinPlatformPollEvents(void);
void _linearwinPlatformWaitEvents(void);
void _linearwinPlatformWaitEventsTimeout(double timeout);
void _linearwinPlatformPostEmptyEvent(void);

void _linearwinPlatformGetRequiredInstanceExtensions(char** extensions);
int _linearwinPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily);
VkResult _linearwinPlatformCreateWindowSurface(VkInstance instance,
                                          _LINEARWINwindow* window,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR* surface);

LINEARWINbool _linearwinPlatformCreateTls(_LINEARWINtls* tls);
void _linearwinPlatformDestroyTls(_LINEARWINtls* tls);
void* _linearwinPlatformGetTls(_LINEARWINtls* tls);
void _linearwinPlatformSetTls(_LINEARWINtls* tls, void* value);

LINEARWINbool _linearwinPlatformCreateMutex(_LINEARWINmutex* mutex);
void _linearwinPlatformDestroyMutex(_LINEARWINmutex* mutex);
void _linearwinPlatformLockMutex(_LINEARWINmutex* mutex);
void _linearwinPlatformUnlockMutex(_LINEARWINmutex* mutex);


//////////////////////////////////////////////////////////////////////////
//////                         LINEARWIN event API                       //////
//////////////////////////////////////////////////////////////////////////

void _linearwinInputWindowFocus(_LINEARWINwindow* window, LINEARWINbool focused);
void _linearwinInputWindowPos(_LINEARWINwindow* window, int xpos, int ypos);
void _linearwinInputWindowSize(_LINEARWINwindow* window, int width, int height);
void _linearwinInputFramebufferSize(_LINEARWINwindow* window, int width, int height);
void _linearwinInputWindowContentScale(_LINEARWINwindow* window,
                                  float xscale, float yscale);
void _linearwinInputWindowIconify(_LINEARWINwindow* window, LINEARWINbool iconified);
void _linearwinInputWindowMaximize(_LINEARWINwindow* window, LINEARWINbool maximized);
void _linearwinInputWindowDamage(_LINEARWINwindow* window);
void _linearwinInputWindowCloseRequest(_LINEARWINwindow* window);
void _linearwinInputWindowMonitor(_LINEARWINwindow* window, _LINEARWINmonitor* monitor);

void _linearwinInputKey(_LINEARWINwindow* window,
                   int key, int scancode, int action, int mods);
void _linearwinInputChar(_LINEARWINwindow* window,
                    uint32_t codepoint, int mods, LINEARWINbool plain);
void _linearwinInputScroll(_LINEARWINwindow* window, double xoffset, double yoffset);
void _linearwinInputMouseClick(_LINEARWINwindow* window, int button, int action, int mods);
void _linearwinInputCursorPos(_LINEARWINwindow* window, double xpos, double ypos);
void _linearwinInputCursorEnter(_LINEARWINwindow* window, LINEARWINbool entered);
void _linearwinInputDrop(_LINEARWINwindow* window, int count, const char** names);
void _linearwinInputJoystick(_LINEARWINjoystick* js, int event);
void _linearwinInputJoystickAxis(_LINEARWINjoystick* js, int axis, float value);
void _linearwinInputJoystickButton(_LINEARWINjoystick* js, int button, char value);
void _linearwinInputJoystickHat(_LINEARWINjoystick* js, int hat, char value);

void _linearwinInputMonitor(_LINEARWINmonitor* monitor, int action, int placement);
void _linearwinInputMonitorWindow(_LINEARWINmonitor* monitor, _LINEARWINwindow* window);

#if defined(__GNUC__)
void _linearwinInputError(int code, const char* format, ...)
    __attribute__((format(printf, 2, 3)));
#else
void _linearwinInputError(int code, const char* format, ...);
#endif


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

LINEARWINbool _linearwinStringInExtensionString(const char* string, const char* extensions);
const _LINEARWINfbconfig* _linearwinChooseFBConfig(const _LINEARWINfbconfig* desired,
                                         const _LINEARWINfbconfig* alternatives,
                                         unsigned int count);
LINEARWINbool _linearwinRefreshContextAttribs(_LINEARWINwindow* window,
                                    const _LINEARWINctxconfig* ctxconfig);
LINEARWINbool _linearwinIsValidContextConfig(const _LINEARWINctxconfig* ctxconfig);

const LINEARWINvidmode* _linearwinChooseVideoMode(_LINEARWINmonitor* monitor,
                                        const LINEARWINvidmode* desired);
int _linearwinCompareVideoModes(const LINEARWINvidmode* first, const LINEARWINvidmode* second);
_LINEARWINmonitor* _linearwinAllocMonitor(const char* name, int widthMM, int heightMM);
void _linearwinFreeMonitor(_LINEARWINmonitor* monitor);
void _linearwinAllocGammaArrays(LINEARWINgammaramp* ramp, unsigned int size);
void _linearwinFreeGammaArrays(LINEARWINgammaramp* ramp);
void _linearwinSplitBPP(int bpp, int* red, int* green, int* blue);

void _linearwinInitGamepadMappings(void);
_LINEARWINjoystick* _linearwinAllocJoystick(const char* name,
                                  const char* guid,
                                  int axisCount,
                                  int buttonCount,
                                  int hatCount);
void _linearwinFreeJoystick(_LINEARWINjoystick* js);
void _linearwinCenterCursorInContentArea(_LINEARWINwindow* window);

LINEARWINbool _linearwinInitVulkan(int mode);
void _linearwinTerminateVulkan(void);
const char* _linearwinGetVulkanResultString(VkResult result);

size_t _linearwinEncodeUTF8(char* s, uint32_t codepoint);
char** _linearwinParseUriList(char* text, int* count);

char* _linearwin_strdup(const char* source);
int _linearwin_min(int a, int b);
int _linearwin_max(int a, int b);
float _linearwin_fminf(float a, float b);
float _linearwin_fmaxf(float a, float b);

