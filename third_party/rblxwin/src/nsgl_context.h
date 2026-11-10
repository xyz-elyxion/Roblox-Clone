//========================================================================
// RBLXWIN 3.3 macOS - www.rblxwin.org
//------------------------------------------------------------------------
// Copyright (c) 2009-2019 Camilla Löwy <elmindreda@rblxwin.org>
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

// NOTE: Many Cocoa enum values have been renamed and we need to build across
//       SDK versions where one is unavailable or deprecated.
//       We use the newer names in code and replace them with the older names if
//       the base SDK does not provide the newer names.

#if MAC_OS_X_VERSION_MAX_ALLOWED < 101400
 #define NSOpenGLContextParameterSwapInterval NSOpenGLCPSwapInterval
 #define NSOpenGLContextParameterSurfaceOpacity NSOpenGLCPSurfaceOpacity
#endif

#define _RBLXWIN_PLATFORM_CONTEXT_STATE            _RBLXWINcontextNSGL nsgl
#define _RBLXWIN_PLATFORM_LIBRARY_CONTEXT_STATE    _RBLXWINlibraryNSGL nsgl

#include <stdatomic.h>


// NSGL-specific per-context data
//
typedef struct _RBLXWINcontextNSGL
{
    id                pixelFormat;
    id                object;
} _RBLXWINcontextNSGL;

// NSGL-specific global data
//
typedef struct _RBLXWINlibraryNSGL
{
    // dlopen handle for OpenGL.framework (for rblxwinGetProcAddress)
    CFBundleRef     framework;
} _RBLXWINlibraryNSGL;


RBLXWINbool _rblxwinInitNSGL(void);
void _rblxwinTerminateNSGL(void);
RBLXWINbool _rblxwinCreateContextNSGL(_RBLXWINwindow* window,
                                const _RBLXWINctxconfig* ctxconfig,
                                const _RBLXWINfbconfig* fbconfig);
void _rblxwinDestroyContextNSGL(_RBLXWINwindow* window);

