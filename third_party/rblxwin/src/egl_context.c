//========================================================================
// RBLXWIN 3.3 EGL - www.rblxwin.org
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


// Return a description of the specified EGL error
//
static const char* getEGLErrorString(EGLint error)
{
    switch (error)
    {
        case EGL_SUCCESS:
            return "Success";
        case EGL_NOT_INITIALIZED:
            return "EGL is not or could not be initialized";
        case EGL_BAD_ACCESS:
            return "EGL cannot access a requested resource";
        case EGL_BAD_ALLOC:
            return "EGL failed to allocate resources for the requested operation";
        case EGL_BAD_ATTRIBUTE:
            return "An unrecognized attribute or attribute value was passed in the attribute list";
        case EGL_BAD_CONTEXT:
            return "An EGLContext argument does not name a valid EGL rendering context";
        case EGL_BAD_CONFIG:
            return "An EGLConfig argument does not name a valid EGL frame buffer configuration";
        case EGL_BAD_CURRENT_SURFACE:
            return "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid";
        case EGL_BAD_DISPLAY:
            return "An EGLDisplay argument does not name a valid EGL display connection";
        case EGL_BAD_SURFACE:
            return "An EGLSurface argument does not name a valid surface configured for GL rendering";
        case EGL_BAD_MATCH:
            return "Arguments are inconsistent";
        case EGL_BAD_PARAMETER:
            return "One or more argument values are invalid";
        case EGL_BAD_NATIVE_PIXMAP:
            return "A NativePixmapType argument does not refer to a valid native pixmap";
        case EGL_BAD_NATIVE_WINDOW:
            return "A NativeWindowType argument does not refer to a valid native window";
        case EGL_CONTEXT_LOST:
            return "The application must destroy all contexts and reinitialise";
        default:
            return "ERROR: UNKNOWN EGL ERROR";
    }
}

// Returns the specified attribute of the specified EGLConfig
//
static int getEGLConfigAttrib(EGLConfig config, int attrib)
{
    int value;
    eglGetConfigAttrib(_rblxwin.egl.display, config, attrib, &value);
    return value;
}

// Return the EGLConfig most closely matching the specified hints
//
static RBLXWINbool chooseEGLConfig(const _RBLXWINctxconfig* ctxconfig,
                                const _RBLXWINfbconfig* desired,
                                EGLConfig* result)
{
    EGLConfig* nativeConfigs;
    _RBLXWINfbconfig* usableConfigs;
    const _RBLXWINfbconfig* closest;
    int i, nativeCount, usableCount;

    eglGetConfigs(_rblxwin.egl.display, NULL, 0, &nativeCount);
    if (!nativeCount)
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE, "EGL: No EGLConfigs returned");
        return RBLXWIN_FALSE;
    }

    nativeConfigs = calloc(nativeCount, sizeof(EGLConfig));
    eglGetConfigs(_rblxwin.egl.display, nativeConfigs, nativeCount, &nativeCount);

    usableConfigs = calloc(nativeCount, sizeof(_RBLXWINfbconfig));
    usableCount = 0;

    for (i = 0;  i < nativeCount;  i++)
    {
        const EGLConfig n = nativeConfigs[i];
        _RBLXWINfbconfig* u = usableConfigs + usableCount;

        // Only consider RGB(A) EGLConfigs
        if (getEGLConfigAttrib(n, EGL_COLOR_BUFFER_TYPE) != EGL_RGB_BUFFER)
            continue;

        // Only consider window EGLConfigs
        if (!(getEGLConfigAttrib(n, EGL_SURFACE_TYPE) & EGL_WINDOW_BIT))
            continue;

#if defined(_RBLXWIN_X11)
        {
            XVisualInfo vi = {0};

            // Only consider EGLConfigs with associated Visuals
            vi.visualid = getEGLConfigAttrib(n, EGL_NATIVE_VISUAL_ID);
            if (!vi.visualid)
                continue;

            if (desired->transparent)
            {
                int count;
                XVisualInfo* vis =
                    XGetVisualInfo(_rblxwin.x11.display, VisualIDMask, &vi, &count);
                if (vis)
                {
                    u->transparent = _rblxwinIsVisualTransparentX11(vis[0].visual);
                    XFree(vis);
                }
            }
        }
#endif // _RBLXWIN_X11

        if (ctxconfig->client == RBLXWIN_OPENGL_ES_API)
        {
            if (ctxconfig->major == 1)
            {
                if (!(getEGLConfigAttrib(n, EGL_RENDERABLE_TYPE) & EGL_OPENGL_ES_BIT))
                    continue;
            }
            else
            {
                if (!(getEGLConfigAttrib(n, EGL_RENDERABLE_TYPE) & EGL_OPENGL_ES2_BIT))
                    continue;
            }
        }
        else if (ctxconfig->client == RBLXWIN_OPENGL_API)
        {
            if (!(getEGLConfigAttrib(n, EGL_RENDERABLE_TYPE) & EGL_OPENGL_BIT))
                continue;
        }

        u->redBits = getEGLConfigAttrib(n, EGL_RED_SIZE);
        u->greenBits = getEGLConfigAttrib(n, EGL_GREEN_SIZE);
        u->blueBits = getEGLConfigAttrib(n, EGL_BLUE_SIZE);

        u->alphaBits = getEGLConfigAttrib(n, EGL_ALPHA_SIZE);
        u->depthBits = getEGLConfigAttrib(n, EGL_DEPTH_SIZE);
        u->stencilBits = getEGLConfigAttrib(n, EGL_STENCIL_SIZE);

        u->samples = getEGLConfigAttrib(n, EGL_SAMPLES);
        u->doublebuffer = desired->doublebuffer;

        u->handle = (uintptr_t) n;
        usableCount++;
    }

    closest = _rblxwinChooseFBConfig(desired, usableConfigs, usableCount);
    if (closest)
        *result = (EGLConfig) closest->handle;

    free(nativeConfigs);
    free(usableConfigs);

    return closest != NULL;
}

static void makeContextCurrentEGL(_RBLXWINwindow* window)
{
    if (window)
    {
        if (!eglMakeCurrent(_rblxwin.egl.display,
                            window->context.egl.surface,
                            window->context.egl.surface,
                            window->context.egl.handle))
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "EGL: Failed to make context current: %s",
                            getEGLErrorString(eglGetError()));
            return;
        }
    }
    else
    {
        if (!eglMakeCurrent(_rblxwin.egl.display,
                            EGL_NO_SURFACE,
                            EGL_NO_SURFACE,
                            EGL_NO_CONTEXT))
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "EGL: Failed to clear current context: %s",
                            getEGLErrorString(eglGetError()));
            return;
        }
    }

    _rblxwinPlatformSetTls(&_rblxwin.contextSlot, window);
}

static void swapBuffersEGL(_RBLXWINwindow* window)
{
    if (window != _rblxwinPlatformGetTls(&_rblxwin.contextSlot))
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "EGL: The context must be current on the calling thread when swapping buffers");
        return;
    }

#if defined(_RBLXWIN_WAYLAND)
    // NOTE: Swapping buffers on a hidden window on Wayland makes it visible
    if (!window->wl.visible)
        return;
#endif

    eglSwapBuffers(_rblxwin.egl.display, window->context.egl.surface);
}

static void swapIntervalEGL(int interval)
{
    eglSwapInterval(_rblxwin.egl.display, interval);
}

static int extensionSupportedEGL(const char* extension)
{
    const char* extensions = eglQueryString(_rblxwin.egl.display, EGL_EXTENSIONS);
    if (extensions)
    {
        if (_rblxwinStringInExtensionString(extension, extensions))
            return RBLXWIN_TRUE;
    }

    return RBLXWIN_FALSE;
}

static RBLXWINglproc getProcAddressEGL(const char* procname)
{
    _RBLXWINwindow* window = _rblxwinPlatformGetTls(&_rblxwin.contextSlot);

    if (window->context.egl.client)
    {
        RBLXWINglproc proc = (RBLXWINglproc) _rblxwin_dlsym(window->context.egl.client,
                                                   procname);
        if (proc)
            return proc;
    }

    return eglGetProcAddress(procname);
}

static void destroyContextEGL(_RBLXWINwindow* window)
{
#if defined(_RBLXWIN_X11)
    // NOTE: Do not unload libGL.so.1 while the X11 display is still open,
    //       as it will make XCloseDisplay segfault
    if (window->context.client != RBLXWIN_OPENGL_API)
#endif // _RBLXWIN_X11
    {
        if (window->context.egl.client)
        {
            _rblxwin_dlclose(window->context.egl.client);
            window->context.egl.client = NULL;
        }
    }

    if (window->context.egl.surface)
    {
        eglDestroySurface(_rblxwin.egl.display, window->context.egl.surface);
        window->context.egl.surface = EGL_NO_SURFACE;
    }

    if (window->context.egl.handle)
    {
        eglDestroyContext(_rblxwin.egl.display, window->context.egl.handle);
        window->context.egl.handle = EGL_NO_CONTEXT;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize EGL
//
RBLXWINbool _rblxwinInitEGL(void)
{
    int i;
    const char* sonames[] =
    {
#if defined(_RBLXWIN_EGL_LIBRARY)
        _RBLXWIN_EGL_LIBRARY,
#elif defined(_RBLXWIN_WIN32)
        "libEGL.dll",
        "EGL.dll",
#elif defined(_RBLXWIN_COCOA)
        "libEGL.dylib",
#elif defined(__CYGWIN__)
        "libEGL-1.so",
#elif defined(__OpenBSD__) || defined(__NetBSD__)
        "libEGL.so",
#else
        "libEGL.so.1",
#endif
        NULL
    };

    if (_rblxwin.egl.handle)
        return RBLXWIN_TRUE;

    for (i = 0;  sonames[i];  i++)
    {
        _rblxwin.egl.handle = _rblxwin_dlopen(sonames[i]);
        if (_rblxwin.egl.handle)
            break;
    }

    if (!_rblxwin.egl.handle)
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE, "EGL: Library not found");
        return RBLXWIN_FALSE;
    }

    _rblxwin.egl.prefix = (strncmp(sonames[i], "lib", 3) == 0);

    _rblxwin.egl.GetConfigAttrib = (PFN_eglGetConfigAttrib)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglGetConfigAttrib");
    _rblxwin.egl.GetConfigs = (PFN_eglGetConfigs)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglGetConfigs");
    _rblxwin.egl.GetDisplay = (PFN_eglGetDisplay)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglGetDisplay");
    _rblxwin.egl.GetError = (PFN_eglGetError)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglGetError");
    _rblxwin.egl.Initialize = (PFN_eglInitialize)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglInitialize");
    _rblxwin.egl.Terminate = (PFN_eglTerminate)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglTerminate");
    _rblxwin.egl.BindAPI = (PFN_eglBindAPI)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglBindAPI");
    _rblxwin.egl.CreateContext = (PFN_eglCreateContext)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglCreateContext");
    _rblxwin.egl.DestroySurface = (PFN_eglDestroySurface)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglDestroySurface");
    _rblxwin.egl.DestroyContext = (PFN_eglDestroyContext)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglDestroyContext");
    _rblxwin.egl.CreateWindowSurface = (PFN_eglCreateWindowSurface)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglCreateWindowSurface");
    _rblxwin.egl.MakeCurrent = (PFN_eglMakeCurrent)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglMakeCurrent");
    _rblxwin.egl.SwapBuffers = (PFN_eglSwapBuffers)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglSwapBuffers");
    _rblxwin.egl.SwapInterval = (PFN_eglSwapInterval)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglSwapInterval");
    _rblxwin.egl.QueryString = (PFN_eglQueryString)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglQueryString");
    _rblxwin.egl.GetProcAddress = (PFN_eglGetProcAddress)
        _rblxwin_dlsym(_rblxwin.egl.handle, "eglGetProcAddress");

    if (!_rblxwin.egl.GetConfigAttrib ||
        !_rblxwin.egl.GetConfigs ||
        !_rblxwin.egl.GetDisplay ||
        !_rblxwin.egl.GetError ||
        !_rblxwin.egl.Initialize ||
        !_rblxwin.egl.Terminate ||
        !_rblxwin.egl.BindAPI ||
        !_rblxwin.egl.CreateContext ||
        !_rblxwin.egl.DestroySurface ||
        !_rblxwin.egl.DestroyContext ||
        !_rblxwin.egl.CreateWindowSurface ||
        !_rblxwin.egl.MakeCurrent ||
        !_rblxwin.egl.SwapBuffers ||
        !_rblxwin.egl.SwapInterval ||
        !_rblxwin.egl.QueryString ||
        !_rblxwin.egl.GetProcAddress)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "EGL: Failed to load required entry points");

        _rblxwinTerminateEGL();
        return RBLXWIN_FALSE;
    }

    _rblxwin.egl.display = eglGetDisplay(_RBLXWIN_EGL_NATIVE_DISPLAY);
    if (_rblxwin.egl.display == EGL_NO_DISPLAY)
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                        "EGL: Failed to get EGL display: %s",
                        getEGLErrorString(eglGetError()));

        _rblxwinTerminateEGL();
        return RBLXWIN_FALSE;
    }

    if (!eglInitialize(_rblxwin.egl.display, &_rblxwin.egl.major, &_rblxwin.egl.minor))
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                        "EGL: Failed to initialize EGL: %s",
                        getEGLErrorString(eglGetError()));

        _rblxwinTerminateEGL();
        return RBLXWIN_FALSE;
    }

    _rblxwin.egl.KHR_create_context =
        extensionSupportedEGL("EGL_KHR_create_context");
    _rblxwin.egl.KHR_create_context_no_error =
        extensionSupportedEGL("EGL_KHR_create_context_no_error");
    _rblxwin.egl.KHR_gl_colorspace =
        extensionSupportedEGL("EGL_KHR_gl_colorspace");
    _rblxwin.egl.KHR_get_all_proc_addresses =
        extensionSupportedEGL("EGL_KHR_get_all_proc_addresses");
    _rblxwin.egl.KHR_context_flush_control =
        extensionSupportedEGL("EGL_KHR_context_flush_control");
    _rblxwin.egl.EXT_present_opaque =
        extensionSupportedEGL("EGL_EXT_present_opaque");

    return RBLXWIN_TRUE;
}

// Terminate EGL
//
void _rblxwinTerminateEGL(void)
{
    if (_rblxwin.egl.display)
    {
        eglTerminate(_rblxwin.egl.display);
        _rblxwin.egl.display = EGL_NO_DISPLAY;
    }

    if (_rblxwin.egl.handle)
    {
        _rblxwin_dlclose(_rblxwin.egl.handle);
        _rblxwin.egl.handle = NULL;
    }
}

#define setAttrib(a, v) \
{ \
    assert(((size_t) index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
    attribs[index++] = a; \
    attribs[index++] = v; \
}

// Create the OpenGL or OpenGL ES context
//
RBLXWINbool _rblxwinCreateContextEGL(_RBLXWINwindow* window,
                               const _RBLXWINctxconfig* ctxconfig,
                               const _RBLXWINfbconfig* fbconfig)
{
    EGLint attribs[40];
    EGLConfig config;
    EGLContext share = NULL;
    int index = 0;

    if (!_rblxwin.egl.display)
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE, "EGL: API not available");
        return RBLXWIN_FALSE;
    }

    if (ctxconfig->share)
        share = ctxconfig->share->context.egl.handle;

    if (!chooseEGLConfig(ctxconfig, fbconfig, &config))
    {
        _rblxwinInputError(RBLXWIN_FORMAT_UNAVAILABLE,
                        "EGL: Failed to find a suitable EGLConfig");
        return RBLXWIN_FALSE;
    }

    if (ctxconfig->client == RBLXWIN_OPENGL_ES_API)
    {
        if (!eglBindAPI(EGL_OPENGL_ES_API))
        {
            _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                            "EGL: Failed to bind OpenGL ES: %s",
                            getEGLErrorString(eglGetError()));
            return RBLXWIN_FALSE;
        }
    }
    else
    {
        if (!eglBindAPI(EGL_OPENGL_API))
        {
            _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                            "EGL: Failed to bind OpenGL: %s",
                            getEGLErrorString(eglGetError()));
            return RBLXWIN_FALSE;
        }
    }

    if (_rblxwin.egl.KHR_create_context)
    {
        int mask = 0, flags = 0;

        if (ctxconfig->client == RBLXWIN_OPENGL_API)
        {
            if (ctxconfig->forward)
                flags |= EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR;

            if (ctxconfig->profile == RBLXWIN_OPENGL_CORE_PROFILE)
                mask |= EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR;
            else if (ctxconfig->profile == RBLXWIN_OPENGL_COMPAT_PROFILE)
                mask |= EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR;
        }

        if (ctxconfig->debug)
            flags |= EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;

        if (ctxconfig->robustness)
        {
            if (ctxconfig->robustness == RBLXWIN_NO_RESET_NOTIFICATION)
            {
                setAttrib(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR,
                          EGL_NO_RESET_NOTIFICATION_KHR);
            }
            else if (ctxconfig->robustness == RBLXWIN_LOSE_CONTEXT_ON_RESET)
            {
                setAttrib(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR,
                          EGL_LOSE_CONTEXT_ON_RESET_KHR);
            }

            flags |= EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR;
        }

        if (ctxconfig->noerror)
        {
            if (_rblxwin.egl.KHR_create_context_no_error)
                setAttrib(EGL_CONTEXT_OPENGL_NO_ERROR_KHR, RBLXWIN_TRUE);
        }

        if (ctxconfig->major != 1 || ctxconfig->minor != 0)
        {
            setAttrib(EGL_CONTEXT_MAJOR_VERSION_KHR, ctxconfig->major);
            setAttrib(EGL_CONTEXT_MINOR_VERSION_KHR, ctxconfig->minor);
        }

        if (mask)
            setAttrib(EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, mask);

        if (flags)
            setAttrib(EGL_CONTEXT_FLAGS_KHR, flags);
    }
    else
    {
        if (ctxconfig->client == RBLXWIN_OPENGL_ES_API)
            setAttrib(EGL_CONTEXT_CLIENT_VERSION, ctxconfig->major);
    }

    if (_rblxwin.egl.KHR_context_flush_control)
    {
        if (ctxconfig->release == RBLXWIN_RELEASE_BEHAVIOR_NONE)
        {
            setAttrib(EGL_CONTEXT_RELEASE_BEHAVIOR_KHR,
                      EGL_CONTEXT_RELEASE_BEHAVIOR_NONE_KHR);
        }
        else if (ctxconfig->release == RBLXWIN_RELEASE_BEHAVIOR_FLUSH)
        {
            setAttrib(EGL_CONTEXT_RELEASE_BEHAVIOR_KHR,
                      EGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_KHR);
        }
    }

    setAttrib(EGL_NONE, EGL_NONE);

    window->context.egl.handle = eglCreateContext(_rblxwin.egl.display,
                                                  config, share, attribs);

    if (window->context.egl.handle == EGL_NO_CONTEXT)
    {
        _rblxwinInputError(RBLXWIN_VERSION_UNAVAILABLE,
                        "EGL: Failed to create context: %s",
                        getEGLErrorString(eglGetError()));
        return RBLXWIN_FALSE;
    }

    // Set up attributes for surface creation
    index = 0;

    if (fbconfig->sRGB)
    {
        if (_rblxwin.egl.KHR_gl_colorspace)
            setAttrib(EGL_GL_COLORSPACE_KHR, EGL_GL_COLORSPACE_SRGB_KHR);
    }

    if (!fbconfig->doublebuffer)
        setAttrib(EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER);

    if (_rblxwin.egl.EXT_present_opaque)
        setAttrib(EGL_PRESENT_OPAQUE_EXT, !fbconfig->transparent);

    setAttrib(EGL_NONE, EGL_NONE);

    window->context.egl.surface =
        eglCreateWindowSurface(_rblxwin.egl.display,
                               config,
                               _RBLXWIN_EGL_NATIVE_WINDOW,
                               attribs);
    if (window->context.egl.surface == EGL_NO_SURFACE)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "EGL: Failed to create window surface: %s",
                        getEGLErrorString(eglGetError()));
        return RBLXWIN_FALSE;
    }

    window->context.egl.config = config;

    // Load the appropriate client library
    if (!_rblxwin.egl.KHR_get_all_proc_addresses)
    {
        int i;
        const char** sonames;
        const char* es1sonames[] =
        {
#if defined(_RBLXWIN_GLESV1_LIBRARY)
            _RBLXWIN_GLESV1_LIBRARY,
#elif defined(_RBLXWIN_WIN32)
            "GLESv1_CM.dll",
            "libGLES_CM.dll",
#elif defined(_RBLXWIN_COCOA)
            "libGLESv1_CM.dylib",
#elif defined(__OpenBSD__) || defined(__NetBSD__)
            "libGLESv1_CM.so",
#else
            "libGLESv1_CM.so.1",
            "libGLES_CM.so.1",
#endif
            NULL
        };
        const char* es2sonames[] =
        {
#if defined(_RBLXWIN_GLESV2_LIBRARY)
            _RBLXWIN_GLESV2_LIBRARY,
#elif defined(_RBLXWIN_WIN32)
            "GLESv2.dll",
            "libGLESv2.dll",
#elif defined(_RBLXWIN_COCOA)
            "libGLESv2.dylib",
#elif defined(__CYGWIN__)
            "libGLESv2-2.so",
#elif defined(__OpenBSD__) || defined(__NetBSD__)
            "libGLESv2.so",
#else
            "libGLESv2.so.2",
#endif
            NULL
        };
        const char* glsonames[] =
        {
#if defined(_RBLXWIN_OPENGL_LIBRARY)
            _RBLXWIN_OPENGL_LIBRARY,
#elif defined(_RBLXWIN_WIN32)
#elif defined(_RBLXWIN_COCOA)
#elif defined(__OpenBSD__) || defined(__NetBSD__)
            "libGL.so",
#else
            "libGL.so.1",
#endif
            NULL
        };

        if (ctxconfig->client == RBLXWIN_OPENGL_ES_API)
        {
            if (ctxconfig->major == 1)
                sonames = es1sonames;
            else
                sonames = es2sonames;
        }
        else
            sonames = glsonames;

        for (i = 0;  sonames[i];  i++)
        {
            // HACK: Match presence of lib prefix to increase chance of finding
            //       a matching pair in the jungle that is Win32 EGL/GLES
            if (_rblxwin.egl.prefix != (strncmp(sonames[i], "lib", 3) == 0))
                continue;

            window->context.egl.client = _rblxwin_dlopen(sonames[i]);
            if (window->context.egl.client)
                break;
        }

        if (!window->context.egl.client)
        {
            _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                            "EGL: Failed to load client library");
            return RBLXWIN_FALSE;
        }
    }

    window->context.makeCurrent = makeContextCurrentEGL;
    window->context.swapBuffers = swapBuffersEGL;
    window->context.swapInterval = swapIntervalEGL;
    window->context.extensionSupported = extensionSupportedEGL;
    window->context.getProcAddress = getProcAddressEGL;
    window->context.destroy = destroyContextEGL;

    return RBLXWIN_TRUE;
}

#undef setAttrib

// Returns the Visual and depth of the chosen EGLConfig
//
#if defined(_RBLXWIN_X11)
RBLXWINbool _rblxwinChooseVisualEGL(const _RBLXWINwndconfig* wndconfig,
                              const _RBLXWINctxconfig* ctxconfig,
                              const _RBLXWINfbconfig* fbconfig,
                              Visual** visual, int* depth)
{
    XVisualInfo* result;
    XVisualInfo desired;
    EGLConfig native;
    EGLint visualID = 0, count = 0;
    const long vimask = VisualScreenMask | VisualIDMask;

    if (!chooseEGLConfig(ctxconfig, fbconfig, &native))
    {
        _rblxwinInputError(RBLXWIN_FORMAT_UNAVAILABLE,
                        "EGL: Failed to find a suitable EGLConfig");
        return RBLXWIN_FALSE;
    }

    eglGetConfigAttrib(_rblxwin.egl.display, native,
                       EGL_NATIVE_VISUAL_ID, &visualID);

    desired.screen = _rblxwin.x11.screen;
    desired.visualid = visualID;

    result = XGetVisualInfo(_rblxwin.x11.display, vimask, &desired, &count);
    if (!result)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "EGL: Failed to retrieve Visual for EGLConfig");
        return RBLXWIN_FALSE;
    }

    *visual = result->visual;
    *depth = result->depth;

    XFree(result);
    return RBLXWIN_TRUE;
}
#endif // _RBLXWIN_X11


//////////////////////////////////////////////////////////////////////////
//////                        RBLXWIN native API                       //////
//////////////////////////////////////////////////////////////////////////

RBLXWINAPI EGLDisplay rblxwinGetEGLDisplay(void)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(EGL_NO_DISPLAY);
    return _rblxwin.egl.display;
}

RBLXWINAPI EGLContext rblxwinGetEGLContext(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(EGL_NO_CONTEXT);

    if (window->context.source != RBLXWIN_EGL_CONTEXT_API)
    {
        _rblxwinInputError(RBLXWIN_NO_WINDOW_CONTEXT, NULL);
        return EGL_NO_CONTEXT;
    }

    return window->context.egl.handle;
}

RBLXWINAPI EGLSurface rblxwinGetEGLSurface(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(EGL_NO_SURFACE);

    if (window->context.source != RBLXWIN_EGL_CONTEXT_API)
    {
        _rblxwinInputError(RBLXWIN_NO_WINDOW_CONTEXT, NULL);
        return EGL_NO_SURFACE;
    }

    return window->context.egl.surface;
}

