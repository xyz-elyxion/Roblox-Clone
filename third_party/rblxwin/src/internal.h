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

#pragma once

#if defined(_RBLXWIN_USE_CONFIG_H)
 #include "rblxwin_config.h"
#endif

#if defined(RBLXWIN_INCLUDE_GLCOREARB) || \
    defined(RBLXWIN_INCLUDE_ES1)       || \
    defined(RBLXWIN_INCLUDE_ES2)       || \
    defined(RBLXWIN_INCLUDE_ES3)       || \
    defined(RBLXWIN_INCLUDE_ES31)      || \
    defined(RBLXWIN_INCLUDE_ES32)      || \
    defined(RBLXWIN_INCLUDE_NONE)      || \
    defined(RBLXWIN_INCLUDE_GLEXT)     || \
    defined(RBLXWIN_INCLUDE_GLU)       || \
    defined(RBLXWIN_INCLUDE_VULKAN)    || \
    defined(RBLXWIN_DLL)
 #error "You must not define any header option macros when compiling RBLXWIN"
#endif

#define RBLXWIN_INCLUDE_NONE
#include "../include/RBLXWIN/rblxwin.h"

#define _RBLXWIN_INSERT_FIRST      0
#define _RBLXWIN_INSERT_LAST       1

#define _RBLXWIN_POLL_PRESENCE     0
#define _RBLXWIN_POLL_AXES         1
#define _RBLXWIN_POLL_BUTTONS      2
#define _RBLXWIN_POLL_ALL          (_RBLXWIN_POLL_AXES | _RBLXWIN_POLL_BUTTONS)

#define _RBLXWIN_MESSAGE_SIZE      1024

typedef int RBLXWINbool;

typedef struct _RBLXWINerror       _RBLXWINerror;
typedef struct _RBLXWINinitconfig  _RBLXWINinitconfig;
typedef struct _RBLXWINwndconfig   _RBLXWINwndconfig;
typedef struct _RBLXWINctxconfig   _RBLXWINctxconfig;
typedef struct _RBLXWINfbconfig    _RBLXWINfbconfig;
typedef struct _RBLXWINcontext     _RBLXWINcontext;
typedef struct _RBLXWINwindow      _RBLXWINwindow;
typedef struct _RBLXWINlibrary     _RBLXWINlibrary;
typedef struct _RBLXWINmonitor     _RBLXWINmonitor;
typedef struct _RBLXWINcursor      _RBLXWINcursor;
typedef struct _RBLXWINmapelement  _RBLXWINmapelement;
typedef struct _RBLXWINmapping     _RBLXWINmapping;
typedef struct _RBLXWINjoystick    _RBLXWINjoystick;
typedef struct _RBLXWINtls         _RBLXWINtls;
typedef struct _RBLXWINmutex       _RBLXWINmutex;

typedef void (* _RBLXWINmakecontextcurrentfun)(_RBLXWINwindow*);
typedef void (* _RBLXWINswapbuffersfun)(_RBLXWINwindow*);
typedef void (* _RBLXWINswapintervalfun)(int);
typedef int (* _RBLXWINextensionsupportedfun)(const char*);
typedef RBLXWINglproc (* _RBLXWINgetprocaddressfun)(const char*);
typedef void (* _RBLXWINdestroycontextfun)(_RBLXWINwindow*);

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

#if defined(_RBLXWIN_VULKAN_STATIC)
  PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance,const char*);
  VkResult vkEnumerateInstanceExtensionProperties(const char*,uint32_t*,VkExtensionProperties*);
#else
  typedef PFN_vkVoidFunction (APIENTRY * PFN_vkGetInstanceProcAddr)(VkInstance,const char*);
  typedef VkResult (APIENTRY * PFN_vkEnumerateInstanceExtensionProperties)(const char*,uint32_t*,VkExtensionProperties*);
  #define vkEnumerateInstanceExtensionProperties _rblxwin.vk.EnumerateInstanceExtensionProperties
  #define vkGetInstanceProcAddr _rblxwin.vk.GetInstanceProcAddr
#endif

#if defined(_RBLXWIN_COCOA)
 #include "cocoa_platform.h"
#elif defined(_RBLXWIN_WIN32)
 #include "win32_platform.h"
#elif defined(_RBLXWIN_X11)
 #include "x11_platform.h"
#elif defined(_RBLXWIN_WAYLAND)
 #include "wl_platform.h"
#elif defined(_RBLXWIN_OSMESA)
 #include "null_platform.h"
#else
 #error "No supported window creation API selected"
#endif

// Constructs a version number string from the public header macros
#define _RBLXWIN_CONCAT_VERSION(m, n, r) #m "." #n "." #r
#define _RBLXWIN_MAKE_VERSION(m, n, r) _RBLXWIN_CONCAT_VERSION(m, n, r)
#define _RBLXWIN_VERSION_NUMBER _RBLXWIN_MAKE_VERSION(RBLXWIN_VERSION_MAJOR, \
                                                RBLXWIN_VERSION_MINOR, \
                                                RBLXWIN_VERSION_REVISION)

// Checks for whether the library has been initialized
#define _RBLXWIN_REQUIRE_INIT()                         \
    if (!_rblxwin.initialized)                          \
    {                                                \
        _rblxwinInputError(RBLXWIN_NOT_INITIALIZED, NULL); \
        return;                                      \
    }
#define _RBLXWIN_REQUIRE_INIT_OR_RETURN(x)              \
    if (!_rblxwin.initialized)                          \
    {                                                \
        _rblxwinInputError(RBLXWIN_NOT_INITIALIZED, NULL); \
        return x;                                    \
    }

// Swaps the provided pointers
#define _RBLXWIN_SWAP_POINTERS(x, y) \
    {                             \
        void* t;                  \
        t = x;                    \
        x = y;                    \
        y = t;                    \
    }

// Per-thread error structure
//
struct _RBLXWINerror
{
    _RBLXWINerror*     next;
    int             code;
    char            description[_RBLXWIN_MESSAGE_SIZE];
};

// Initialization configuration
//
// Parameters relating to the initialization of the library
//
struct _RBLXWINinitconfig
{
    RBLXWINbool      hatButtons;
    struct {
        RBLXWINbool  menubar;
        RBLXWINbool  chdir;
    } ns;
};

// Window configuration
//
// Parameters relating to the creation of the window but not directly related
// to the framebuffer.  This is used to pass window creation parameters from
// shared code to the platform API.
//
struct _RBLXWINwndconfig
{
    int           width;
    int           height;
    const char*   title;
    RBLXWINbool      resizable;
    RBLXWINbool      visible;
    RBLXWINbool      decorated;
    RBLXWINbool      focused;
    RBLXWINbool      autoIconify;
    RBLXWINbool      floating;
    RBLXWINbool      maximized;
    RBLXWINbool      centerCursor;
    RBLXWINbool      focusOnShow;
    RBLXWINbool      scaleToMonitor;
    struct {
        RBLXWINbool  retina;
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
struct _RBLXWINctxconfig
{
    int           client;
    int           source;
    int           major;
    int           minor;
    RBLXWINbool      forward;
    RBLXWINbool      debug;
    RBLXWINbool      noerror;
    int           profile;
    int           robustness;
    int           release;
    _RBLXWINwindow*  share;
    struct {
        RBLXWINbool  offline;
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
struct _RBLXWINfbconfig
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
    RBLXWINbool    stereo;
    int         samples;
    RBLXWINbool    sRGB;
    RBLXWINbool    doublebuffer;
    RBLXWINbool    transparent;
    uintptr_t   handle;
};

// Context structure
//
struct _RBLXWINcontext
{
    int                 client;
    int                 source;
    int                 major, minor, revision;
    RBLXWINbool            forward, debug, noerror;
    int                 profile;
    int                 robustness;
    int                 release;

    PFNGLGETSTRINGIPROC  GetStringi;
    PFNGLGETINTEGERVPROC GetIntegerv;
    PFNGLGETSTRINGPROC   GetString;

    _RBLXWINmakecontextcurrentfun  makeCurrent;
    _RBLXWINswapbuffersfun         swapBuffers;
    _RBLXWINswapintervalfun        swapInterval;
    _RBLXWINextensionsupportedfun  extensionSupported;
    _RBLXWINgetprocaddressfun      getProcAddress;
    _RBLXWINdestroycontextfun      destroy;

    // This is defined in the context API's context.h
    _RBLXWIN_PLATFORM_CONTEXT_STATE;
    // This is defined in egl_context.h
    _RBLXWIN_EGL_CONTEXT_STATE;
    // This is defined in osmesa_context.h
    _RBLXWIN_OSMESA_CONTEXT_STATE;
};

// Window and context structure
//
struct _RBLXWINwindow
{
    struct _RBLXWINwindow* next;

    // Window settings and state
    RBLXWINbool            resizable;
    RBLXWINbool            decorated;
    RBLXWINbool            autoIconify;
    RBLXWINbool            floating;
    RBLXWINbool            focusOnShow;
    RBLXWINbool            shouldClose;
    void*               userPointer;
    RBLXWINbool            doublebuffer;
    RBLXWINvidmode         videoMode;
    _RBLXWINmonitor*       monitor;
    _RBLXWINcursor*        cursor;

    int                 minwidth, minheight;
    int                 maxwidth, maxheight;
    int                 numer, denom;

    RBLXWINbool            stickyKeys;
    RBLXWINbool            stickyMouseButtons;
    RBLXWINbool            lockKeyMods;
    int                 cursorMode;
    char                mouseButtons[RBLXWIN_MOUSE_BUTTON_LAST + 1];
    char                keys[RBLXWIN_KEY_LAST + 1];
    // Virtual cursor position when cursor is disabled
    double              virtualCursorPosX, virtualCursorPosY;
    RBLXWINbool            rawMouseMotion;

    _RBLXWINcontext        context;

    struct {
        RBLXWINwindowposfun          pos;
        RBLXWINwindowsizefun         size;
        RBLXWINwindowclosefun        close;
        RBLXWINwindowrefreshfun      refresh;
        RBLXWINwindowfocusfun        focus;
        RBLXWINwindowiconifyfun      iconify;
        RBLXWINwindowmaximizefun     maximize;
        RBLXWINframebuffersizefun    fbsize;
        RBLXWINwindowcontentscalefun scale;
        RBLXWINmousebuttonfun        mouseButton;
        RBLXWINcursorposfun          cursorPos;
        RBLXWINcursorenterfun        cursorEnter;
        RBLXWINscrollfun             scroll;
        RBLXWINkeyfun                key;
        RBLXWINcharfun               character;
        RBLXWINcharmodsfun           charmods;
        RBLXWINdropfun               drop;
    } callbacks;

    // This is defined in the window API's platform.h
    _RBLXWIN_PLATFORM_WINDOW_STATE;
};

// Monitor structure
//
struct _RBLXWINmonitor
{
    char            name[128];
    void*           userPointer;

    // Physical dimensions in millimeters.
    int             widthMM, heightMM;

    // The window whose video mode is current on this monitor
    _RBLXWINwindow*    window;

    RBLXWINvidmode*    modes;
    int             modeCount;
    RBLXWINvidmode     currentMode;

    RBLXWINgammaramp   originalRamp;
    RBLXWINgammaramp   currentRamp;

    // This is defined in the window API's platform.h
    _RBLXWIN_PLATFORM_MONITOR_STATE;
};

// Cursor structure
//
struct _RBLXWINcursor
{
    _RBLXWINcursor*    next;

    // This is defined in the window API's platform.h
    _RBLXWIN_PLATFORM_CURSOR_STATE;
};

// Gamepad mapping element structure
//
struct _RBLXWINmapelement
{
    uint8_t         type;
    uint8_t         index;
    int8_t          axisScale;
    int8_t          axisOffset;
};

// Gamepad mapping structure
//
struct _RBLXWINmapping
{
    char            name[128];
    char            guid[33];
    _RBLXWINmapelement buttons[15];
    _RBLXWINmapelement axes[6];
};

// Joystick structure
//
struct _RBLXWINjoystick
{
    RBLXWINbool        allocated;
    RBLXWINbool        connected;
    float*          axes;
    int             axisCount;
    unsigned char*  buttons;
    int             buttonCount;
    unsigned char*  hats;
    int             hatCount;
    char            name[128];
    void*           userPointer;
    char            guid[33];
    _RBLXWINmapping*   mapping;

    // This is defined in the joystick API's joystick.h
    _RBLXWIN_PLATFORM_JOYSTICK_STATE;
};

// Thread local storage structure
//
struct _RBLXWINtls
{
    // This is defined in the platform's thread.h
    _RBLXWIN_PLATFORM_TLS_STATE;
};

// Mutex structure
//
struct _RBLXWINmutex
{
    // This is defined in the platform's thread.h
    _RBLXWIN_PLATFORM_MUTEX_STATE;
};

// Library global data
//
struct _RBLXWINlibrary
{
    RBLXWINbool            initialized;

    struct {
        _RBLXWINinitconfig init;
        _RBLXWINfbconfig   framebuffer;
        _RBLXWINwndconfig  window;
        _RBLXWINctxconfig  context;
        int             refreshRate;
    } hints;

    _RBLXWINerror*         errorListHead;
    _RBLXWINcursor*        cursorListHead;
    _RBLXWINwindow*        windowListHead;

    _RBLXWINmonitor**      monitors;
    int                 monitorCount;

    _RBLXWINjoystick       joysticks[RBLXWIN_JOYSTICK_LAST + 1];
    _RBLXWINmapping*       mappings;
    int                 mappingCount;

    _RBLXWINtls            errorSlot;
    _RBLXWINtls            contextSlot;
    _RBLXWINmutex          errorLock;

    struct {
        uint64_t        offset;
        // This is defined in the platform's time.h
        _RBLXWIN_PLATFORM_LIBRARY_TIMER_STATE;
    } timer;

    struct {
        RBLXWINbool        available;
        void*           handle;
        char*           extensions[2];
#if !defined(_RBLXWIN_VULKAN_STATIC)
        PFN_vkEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties;
        PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
#endif
        RBLXWINbool        KHR_surface;
#if defined(_RBLXWIN_WIN32)
        RBLXWINbool        KHR_win32_surface;
#elif defined(_RBLXWIN_COCOA)
        RBLXWINbool        MVK_macos_surface;
        RBLXWINbool        EXT_metal_surface;
#elif defined(_RBLXWIN_X11)
        RBLXWINbool        KHR_xlib_surface;
        RBLXWINbool        KHR_xcb_surface;
#elif defined(_RBLXWIN_WAYLAND)
        RBLXWINbool        KHR_wayland_surface;
#endif
    } vk;

    struct {
        RBLXWINmonitorfun  monitor;
        RBLXWINjoystickfun joystick;
    } callbacks;

    // This is defined in the window API's platform.h
    _RBLXWIN_PLATFORM_LIBRARY_WINDOW_STATE;
    // This is defined in the context API's context.h
    _RBLXWIN_PLATFORM_LIBRARY_CONTEXT_STATE;
    // This is defined in the platform's joystick.h
    _RBLXWIN_PLATFORM_LIBRARY_JOYSTICK_STATE;
    // This is defined in egl_context.h
    _RBLXWIN_EGL_LIBRARY_CONTEXT_STATE;
    // This is defined in osmesa_context.h
    _RBLXWIN_OSMESA_LIBRARY_CONTEXT_STATE;
};

// Global state shared between compilation units of RBLXWIN
//
extern _RBLXWINlibrary _rblxwin;


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _rblxwinPlatformInit(void);
void _rblxwinPlatformTerminate(void);
const char* _rblxwinPlatformGetVersionString(void);

void _rblxwinPlatformGetCursorPos(_RBLXWINwindow* window, double* xpos, double* ypos);
void _rblxwinPlatformSetCursorPos(_RBLXWINwindow* window, double xpos, double ypos);
void _rblxwinPlatformSetCursorMode(_RBLXWINwindow* window, int mode);
void _rblxwinPlatformSetRawMouseMotion(_RBLXWINwindow *window, RBLXWINbool enabled);
RBLXWINbool _rblxwinPlatformRawMouseMotionSupported(void);
int _rblxwinPlatformCreateCursor(_RBLXWINcursor* cursor,
                              const RBLXWINimage* image, int xhot, int yhot);
int _rblxwinPlatformCreateStandardCursor(_RBLXWINcursor* cursor, int shape);
void _rblxwinPlatformDestroyCursor(_RBLXWINcursor* cursor);
void _rblxwinPlatformSetCursor(_RBLXWINwindow* window, _RBLXWINcursor* cursor);

const char* _rblxwinPlatformGetScancodeName(int scancode);
int _rblxwinPlatformGetKeyScancode(int key);

void _rblxwinPlatformFreeMonitor(_RBLXWINmonitor* monitor);
void _rblxwinPlatformGetMonitorPos(_RBLXWINmonitor* monitor, int* xpos, int* ypos);
void _rblxwinPlatformGetMonitorContentScale(_RBLXWINmonitor* monitor,
                                         float* xscale, float* yscale);
void _rblxwinPlatformGetMonitorWorkarea(_RBLXWINmonitor* monitor, int* xpos, int* ypos, int *width, int *height);
RBLXWINvidmode* _rblxwinPlatformGetVideoModes(_RBLXWINmonitor* monitor, int* count);
void _rblxwinPlatformGetVideoMode(_RBLXWINmonitor* monitor, RBLXWINvidmode* mode);
RBLXWINbool _rblxwinPlatformGetGammaRamp(_RBLXWINmonitor* monitor, RBLXWINgammaramp* ramp);
void _rblxwinPlatformSetGammaRamp(_RBLXWINmonitor* monitor, const RBLXWINgammaramp* ramp);

void _rblxwinPlatformSetClipboardString(const char* string);
const char* _rblxwinPlatformGetClipboardString(void);

int _rblxwinPlatformPollJoystick(_RBLXWINjoystick* js, int mode);
void _rblxwinPlatformUpdateGamepadGUID(char* guid);

uint64_t _rblxwinPlatformGetTimerValue(void);
uint64_t _rblxwinPlatformGetTimerFrequency(void);

int _rblxwinPlatformCreateWindow(_RBLXWINwindow* window,
                              const _RBLXWINwndconfig* wndconfig,
                              const _RBLXWINctxconfig* ctxconfig,
                              const _RBLXWINfbconfig* fbconfig);
void _rblxwinPlatformDestroyWindow(_RBLXWINwindow* window);
void _rblxwinPlatformSetWindowTitle(_RBLXWINwindow* window, const char* title);
void _rblxwinPlatformSetWindowIcon(_RBLXWINwindow* window,
                                int count, const RBLXWINimage* images);
void _rblxwinPlatformGetWindowPos(_RBLXWINwindow* window, int* xpos, int* ypos);
void _rblxwinPlatformSetWindowPos(_RBLXWINwindow* window, int xpos, int ypos);
void _rblxwinPlatformGetWindowSize(_RBLXWINwindow* window, int* width, int* height);
void _rblxwinPlatformSetWindowSize(_RBLXWINwindow* window, int width, int height);
void _rblxwinPlatformSetWindowSizeLimits(_RBLXWINwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight);
void _rblxwinPlatformSetWindowAspectRatio(_RBLXWINwindow* window, int numer, int denom);
void _rblxwinPlatformGetFramebufferSize(_RBLXWINwindow* window, int* width, int* height);
void _rblxwinPlatformGetWindowFrameSize(_RBLXWINwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom);
void _rblxwinPlatformGetWindowContentScale(_RBLXWINwindow* window,
                                        float* xscale, float* yscale);
void _rblxwinPlatformIconifyWindow(_RBLXWINwindow* window);
void _rblxwinPlatformRestoreWindow(_RBLXWINwindow* window);
void _rblxwinPlatformMaximizeWindow(_RBLXWINwindow* window);
void _rblxwinPlatformShowWindow(_RBLXWINwindow* window);
void _rblxwinPlatformHideWindow(_RBLXWINwindow* window);
void _rblxwinPlatformRequestWindowAttention(_RBLXWINwindow* window);
void _rblxwinPlatformFocusWindow(_RBLXWINwindow* window);
void _rblxwinPlatformSetWindowMonitor(_RBLXWINwindow* window, _RBLXWINmonitor* monitor,
                                   int xpos, int ypos, int width, int height,
                                   int refreshRate);
int _rblxwinPlatformWindowFocused(_RBLXWINwindow* window);
int _rblxwinPlatformWindowIconified(_RBLXWINwindow* window);
int _rblxwinPlatformWindowVisible(_RBLXWINwindow* window);
int _rblxwinPlatformWindowMaximized(_RBLXWINwindow* window);
int _rblxwinPlatformWindowHovered(_RBLXWINwindow* window);
int _rblxwinPlatformFramebufferTransparent(_RBLXWINwindow* window);
float _rblxwinPlatformGetWindowOpacity(_RBLXWINwindow* window);
void _rblxwinPlatformSetWindowResizable(_RBLXWINwindow* window, RBLXWINbool enabled);
void _rblxwinPlatformSetWindowDecorated(_RBLXWINwindow* window, RBLXWINbool enabled);
void _rblxwinPlatformSetWindowFloating(_RBLXWINwindow* window, RBLXWINbool enabled);
void _rblxwinPlatformSetWindowOpacity(_RBLXWINwindow* window, float opacity);

void _rblxwinPlatformPollEvents(void);
void _rblxwinPlatformWaitEvents(void);
void _rblxwinPlatformWaitEventsTimeout(double timeout);
void _rblxwinPlatformPostEmptyEvent(void);

void _rblxwinPlatformGetRequiredInstanceExtensions(char** extensions);
int _rblxwinPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily);
VkResult _rblxwinPlatformCreateWindowSurface(VkInstance instance,
                                          _RBLXWINwindow* window,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR* surface);

RBLXWINbool _rblxwinPlatformCreateTls(_RBLXWINtls* tls);
void _rblxwinPlatformDestroyTls(_RBLXWINtls* tls);
void* _rblxwinPlatformGetTls(_RBLXWINtls* tls);
void _rblxwinPlatformSetTls(_RBLXWINtls* tls, void* value);

RBLXWINbool _rblxwinPlatformCreateMutex(_RBLXWINmutex* mutex);
void _rblxwinPlatformDestroyMutex(_RBLXWINmutex* mutex);
void _rblxwinPlatformLockMutex(_RBLXWINmutex* mutex);
void _rblxwinPlatformUnlockMutex(_RBLXWINmutex* mutex);


//////////////////////////////////////////////////////////////////////////
//////                         RBLXWIN event API                       //////
//////////////////////////////////////////////////////////////////////////

void _rblxwinInputWindowFocus(_RBLXWINwindow* window, RBLXWINbool focused);
void _rblxwinInputWindowPos(_RBLXWINwindow* window, int xpos, int ypos);
void _rblxwinInputWindowSize(_RBLXWINwindow* window, int width, int height);
void _rblxwinInputFramebufferSize(_RBLXWINwindow* window, int width, int height);
void _rblxwinInputWindowContentScale(_RBLXWINwindow* window,
                                  float xscale, float yscale);
void _rblxwinInputWindowIconify(_RBLXWINwindow* window, RBLXWINbool iconified);
void _rblxwinInputWindowMaximize(_RBLXWINwindow* window, RBLXWINbool maximized);
void _rblxwinInputWindowDamage(_RBLXWINwindow* window);
void _rblxwinInputWindowCloseRequest(_RBLXWINwindow* window);
void _rblxwinInputWindowMonitor(_RBLXWINwindow* window, _RBLXWINmonitor* monitor);

void _rblxwinInputKey(_RBLXWINwindow* window,
                   int key, int scancode, int action, int mods);
void _rblxwinInputChar(_RBLXWINwindow* window,
                    uint32_t codepoint, int mods, RBLXWINbool plain);
void _rblxwinInputScroll(_RBLXWINwindow* window, double xoffset, double yoffset);
void _rblxwinInputMouseClick(_RBLXWINwindow* window, int button, int action, int mods);
void _rblxwinInputCursorPos(_RBLXWINwindow* window, double xpos, double ypos);
void _rblxwinInputCursorEnter(_RBLXWINwindow* window, RBLXWINbool entered);
void _rblxwinInputDrop(_RBLXWINwindow* window, int count, const char** names);
void _rblxwinInputJoystick(_RBLXWINjoystick* js, int event);
void _rblxwinInputJoystickAxis(_RBLXWINjoystick* js, int axis, float value);
void _rblxwinInputJoystickButton(_RBLXWINjoystick* js, int button, char value);
void _rblxwinInputJoystickHat(_RBLXWINjoystick* js, int hat, char value);

void _rblxwinInputMonitor(_RBLXWINmonitor* monitor, int action, int placement);
void _rblxwinInputMonitorWindow(_RBLXWINmonitor* monitor, _RBLXWINwindow* window);

#if defined(__GNUC__)
void _rblxwinInputError(int code, const char* format, ...)
    __attribute__((format(printf, 2, 3)));
#else
void _rblxwinInputError(int code, const char* format, ...);
#endif


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

RBLXWINbool _rblxwinStringInExtensionString(const char* string, const char* extensions);
const _RBLXWINfbconfig* _rblxwinChooseFBConfig(const _RBLXWINfbconfig* desired,
                                         const _RBLXWINfbconfig* alternatives,
                                         unsigned int count);
RBLXWINbool _rblxwinRefreshContextAttribs(_RBLXWINwindow* window,
                                    const _RBLXWINctxconfig* ctxconfig);
RBLXWINbool _rblxwinIsValidContextConfig(const _RBLXWINctxconfig* ctxconfig);

const RBLXWINvidmode* _rblxwinChooseVideoMode(_RBLXWINmonitor* monitor,
                                        const RBLXWINvidmode* desired);
int _rblxwinCompareVideoModes(const RBLXWINvidmode* first, const RBLXWINvidmode* second);
_RBLXWINmonitor* _rblxwinAllocMonitor(const char* name, int widthMM, int heightMM);
void _rblxwinFreeMonitor(_RBLXWINmonitor* monitor);
void _rblxwinAllocGammaArrays(RBLXWINgammaramp* ramp, unsigned int size);
void _rblxwinFreeGammaArrays(RBLXWINgammaramp* ramp);
void _rblxwinSplitBPP(int bpp, int* red, int* green, int* blue);

void _rblxwinInitGamepadMappings(void);
_RBLXWINjoystick* _rblxwinAllocJoystick(const char* name,
                                  const char* guid,
                                  int axisCount,
                                  int buttonCount,
                                  int hatCount);
void _rblxwinFreeJoystick(_RBLXWINjoystick* js);
void _rblxwinCenterCursorInContentArea(_RBLXWINwindow* window);

RBLXWINbool _rblxwinInitVulkan(int mode);
void _rblxwinTerminateVulkan(void);
const char* _rblxwinGetVulkanResultString(VkResult result);

size_t _rblxwinEncodeUTF8(char* s, uint32_t codepoint);
char** _rblxwinParseUriList(char* text, int* count);

char* _rblxwin_strdup(const char* source);
int _rblxwin_min(int a, int b);
int _rblxwin_max(int a, int b);
float _rblxwin_fminf(float a, float b);
float _rblxwin_fmaxf(float a, float b);

