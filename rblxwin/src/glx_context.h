//========================================================================
// RBLXWIN 3.3 GLX - www.rblxwin.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2017 Camilla Löwy <elmindreda@rblxwin.org>
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

#define GLX_VENDOR 1
#define GLX_RGBA_BIT 0x00000001
#define GLX_WINDOW_BIT 0x00000001
#define GLX_DRAWABLE_TYPE 0x8010
#define GLX_RENDER_TYPE 0x8011
#define GLX_RGBA_TYPE 0x8014
#define GLX_DOUBLEBUFFER 5
#define GLX_STEREO 6
#define GLX_AUX_BUFFERS 7
#define GLX_RED_SIZE 8
#define GLX_GREEN_SIZE 9
#define GLX_BLUE_SIZE 10
#define GLX_ALPHA_SIZE 11
#define GLX_DEPTH_SIZE 12
#define GLX_STENCIL_SIZE 13
#define GLX_ACCUM_RED_SIZE 14
#define GLX_ACCUM_GREEN_SIZE 15
#define GLX_ACCUM_BLUE_SIZE 16
#define GLX_ACCUM_ALPHA_SIZE 17
#define GLX_SAMPLES 0x186a1
#define GLX_VISUAL_ID 0x800b

#define GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20b2
#define GLX_CONTEXT_DEBUG_BIT_ARB 0x00000001
#define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define GLX_CONTEXT_PROFILE_MASK_ARB 0x9126
#define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
#define GLX_CONTEXT_FLAGS_ARB 0x2094
#define GLX_CONTEXT_ES2_PROFILE_BIT_EXT 0x00000004
#define GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB 0x00000004
#define GLX_LOSE_CONTEXT_ON_RESET_ARB 0x8252
#define GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
#define GLX_NO_RESET_NOTIFICATION_ARB 0x8261
#define GLX_CONTEXT_RELEASE_BEHAVIOR_ARB 0x2097
#define GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB 0
#define GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB 0x2098
#define GLX_CONTEXT_OPENGL_NO_ERROR_ARB 0x31b3

typedef XID GLXWindow;
typedef XID GLXDrawable;
typedef struct __GLXFBConfig* GLXFBConfig;
typedef struct __GLXcontext* GLXContext;
typedef void (*__GLXextproc)(void);

typedef int (*PFNGLXGETFBCONFIGATTRIBPROC)(Display*,GLXFBConfig,int,int*);
typedef const char* (*PFNGLXGETCLIENTSTRINGPROC)(Display*,int);
typedef Bool (*PFNGLXQUERYEXTENSIONPROC)(Display*,int*,int*);
typedef Bool (*PFNGLXQUERYVERSIONPROC)(Display*,int*,int*);
typedef void (*PFNGLXDESTROYCONTEXTPROC)(Display*,GLXContext);
typedef Bool (*PFNGLXMAKECURRENTPROC)(Display*,GLXDrawable,GLXContext);
typedef void (*PFNGLXSWAPBUFFERSPROC)(Display*,GLXDrawable);
typedef const char* (*PFNGLXQUERYEXTENSIONSSTRINGPROC)(Display*,int);
typedef GLXFBConfig* (*PFNGLXGETFBCONFIGSPROC)(Display*,int,int*);
typedef GLXContext (*PFNGLXCREATENEWCONTEXTPROC)(Display*,GLXFBConfig,int,GLXContext,Bool);
typedef __GLXextproc (* PFNGLXGETPROCADDRESSPROC)(const GLubyte *procName);
typedef void (*PFNGLXSWAPINTERVALEXTPROC)(Display*,GLXDrawable,int);
typedef XVisualInfo* (*PFNGLXGETVISUALFROMFBCONFIGPROC)(Display*,GLXFBConfig);
typedef GLXWindow (*PFNGLXCREATEWINDOWPROC)(Display*,GLXFBConfig,Window,const int*);
typedef void (*PFNGLXDESTROYWINDOWPROC)(Display*,GLXWindow);

typedef int (*PFNGLXSWAPINTERVALMESAPROC)(int);
typedef int (*PFNGLXSWAPINTERVALSGIPROC)(int);
typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC)(Display*,GLXFBConfig,GLXContext,Bool,const int*);

// libGL.so function pointer typedefs
#define glXGetFBConfigs _rblxwin.glx.GetFBConfigs
#define glXGetFBConfigAttrib _rblxwin.glx.GetFBConfigAttrib
#define glXGetClientString _rblxwin.glx.GetClientString
#define glXQueryExtension _rblxwin.glx.QueryExtension
#define glXQueryVersion _rblxwin.glx.QueryVersion
#define glXDestroyContext _rblxwin.glx.DestroyContext
#define glXMakeCurrent _rblxwin.glx.MakeCurrent
#define glXSwapBuffers _rblxwin.glx.SwapBuffers
#define glXQueryExtensionsString _rblxwin.glx.QueryExtensionsString
#define glXCreateNewContext _rblxwin.glx.CreateNewContext
#define glXGetVisualFromFBConfig _rblxwin.glx.GetVisualFromFBConfig
#define glXCreateWindow _rblxwin.glx.CreateWindow
#define glXDestroyWindow _rblxwin.glx.DestroyWindow

#define _RBLXWIN_PLATFORM_CONTEXT_STATE            _RBLXWINcontextGLX glx
#define _RBLXWIN_PLATFORM_LIBRARY_CONTEXT_STATE    _RBLXWINlibraryGLX glx


// GLX-specific per-context data
//
typedef struct _RBLXWINcontextGLX
{
    GLXContext      handle;
    GLXWindow       window;
} _RBLXWINcontextGLX;

// GLX-specific global data
//
typedef struct _RBLXWINlibraryGLX
{
    int             major, minor;
    int             eventBase;
    int             errorBase;

    // dlopen handle for libGL.so.1
    void*           handle;

    // GLX 1.3 functions
    PFNGLXGETFBCONFIGSPROC              GetFBConfigs;
    PFNGLXGETFBCONFIGATTRIBPROC         GetFBConfigAttrib;
    PFNGLXGETCLIENTSTRINGPROC           GetClientString;
    PFNGLXQUERYEXTENSIONPROC            QueryExtension;
    PFNGLXQUERYVERSIONPROC              QueryVersion;
    PFNGLXDESTROYCONTEXTPROC            DestroyContext;
    PFNGLXMAKECURRENTPROC               MakeCurrent;
    PFNGLXSWAPBUFFERSPROC               SwapBuffers;
    PFNGLXQUERYEXTENSIONSSTRINGPROC     QueryExtensionsString;
    PFNGLXCREATENEWCONTEXTPROC          CreateNewContext;
    PFNGLXGETVISUALFROMFBCONFIGPROC     GetVisualFromFBConfig;
    PFNGLXCREATEWINDOWPROC              CreateWindow;
    PFNGLXDESTROYWINDOWPROC             DestroyWindow;

    // GLX 1.4 and extension functions
    PFNGLXGETPROCADDRESSPROC            GetProcAddress;
    PFNGLXGETPROCADDRESSPROC            GetProcAddressARB;
    PFNGLXSWAPINTERVALSGIPROC           SwapIntervalSGI;
    PFNGLXSWAPINTERVALEXTPROC           SwapIntervalEXT;
    PFNGLXSWAPINTERVALMESAPROC          SwapIntervalMESA;
    PFNGLXCREATECONTEXTATTRIBSARBPROC   CreateContextAttribsARB;
    RBLXWINbool        SGI_swap_control;
    RBLXWINbool        EXT_swap_control;
    RBLXWINbool        MESA_swap_control;
    RBLXWINbool        ARB_multisample;
    RBLXWINbool        ARB_framebuffer_sRGB;
    RBLXWINbool        EXT_framebuffer_sRGB;
    RBLXWINbool        ARB_create_context;
    RBLXWINbool        ARB_create_context_profile;
    RBLXWINbool        ARB_create_context_robustness;
    RBLXWINbool        EXT_create_context_es2_profile;
    RBLXWINbool        ARB_create_context_no_error;
    RBLXWINbool        ARB_context_flush_control;
} _RBLXWINlibraryGLX;

RBLXWINbool _rblxwinInitGLX(void);
void _rblxwinTerminateGLX(void);
RBLXWINbool _rblxwinCreateContextGLX(_RBLXWINwindow* window,
                               const _RBLXWINctxconfig* ctxconfig,
                               const _RBLXWINfbconfig* fbconfig);
void _rblxwinDestroyContextGLX(_RBLXWINwindow* window);
RBLXWINbool _rblxwinChooseVisualGLX(const _RBLXWINwndconfig* wndconfig,
                              const _RBLXWINctxconfig* ctxconfig,
                              const _RBLXWINfbconfig* fbconfig,
                              Visual** visual, int* depth);

