//========================================================================
// LINEARWIN 3.3 EGL - www.linearwin.org
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
    eglGetConfigAttrib(_linearwin.egl.display, config, attrib, &value);
    return value;
}

// Return the EGLConfig most closely matching the specified hints
//
static LINEARWINbool chooseEGLConfig(const _LINEARWINctxconfig* ctxconfig,
                                const _LINEARWINfbconfig* desired,
                                EGLConfig* result)
{
    EGLConfig* nativeConfigs;
    _LINEARWINfbconfig* usableConfigs;
    const _LINEARWINfbconfig* closest;
    int i, nativeCount, usableCount;

    eglGetConfigs(_linearwin.egl.display, NULL, 0, &nativeCount);
    if (!nativeCount)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE, "EGL: No EGLConfigs returned");
        return LINEARWIN_FALSE;
    }

    nativeConfigs = calloc(nativeCount, sizeof(EGLConfig));
    eglGetConfigs(_linearwin.egl.display, nativeConfigs, nativeCount, &nativeCount);

    usableConfigs = calloc(nativeCount, sizeof(_LINEARWINfbconfig));
    usableCount = 0;

    for (i = 0;  i < nativeCount;  i++)
    {
        const EGLConfig n = nativeConfigs[i];
        _LINEARWINfbconfig* u = usableConfigs + usableCount;

        // Only consider RGB(A) EGLConfigs
        if (getEGLConfigAttrib(n, EGL_COLOR_BUFFER_TYPE) != EGL_RGB_BUFFER)
            continue;

        // Only consider window EGLConfigs
        if (!(getEGLConfigAttrib(n, EGL_SURFACE_TYPE) & EGL_WINDOW_BIT))
            continue;

#if defined(_LINEARWIN_X11)
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
                    XGetVisualInfo(_linearwin.x11.display, VisualIDMask, &vi, &count);
                if (vis)
                {
                    u->transparent = _linearwinIsVisualTransparentX11(vis[0].visual);
                    XFree(vis);
                }
            }
        }
#endif // _LINEARWIN_X11

        if (ctxconfig->client == LINEARWIN_OPENGL_ES_API)
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
        else if (ctxconfig->client == LINEARWIN_OPENGL_API)
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

    closest = _linearwinChooseFBConfig(desired, usableConfigs, usableCount);
    if (closest)
        *result = (EGLConfig) closest->handle;

    free(nativeConfigs);
    free(usableConfigs);

    return closest != NULL;
}

static void makeContextCurrentEGL(_LINEARWINwindow* window)
{
    if (window)
    {
        if (!eglMakeCurrent(_linearwin.egl.display,
                            window->context.egl.surface,
                            window->context.egl.surface,
                            window->context.egl.handle))
        {
            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "EGL: Failed to make context current: %s",
                            getEGLErrorString(eglGetError()));
            return;
        }
    }
    else
    {
        if (!eglMakeCurrent(_linearwin.egl.display,
                            EGL_NO_SURFACE,
                            EGL_NO_SURFACE,
                            EGL_NO_CONTEXT))
        {
            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "EGL: Failed to clear current context: %s",
                            getEGLErrorString(eglGetError()));
            return;
        }
    }

    _linearwinPlatformSetTls(&_linearwin.contextSlot, window);
}

static void swapBuffersEGL(_LINEARWINwindow* window)
{
    if (window != _linearwinPlatformGetTls(&_linearwin.contextSlot))
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "EGL: The context must be current on the calling thread when swapping buffers");
        return;
    }

#if defined(_LINEARWIN_WAYLAND)
    // NOTE: Swapping buffers on a hidden window on Wayland makes it visible
    if (!window->wl.visible)
        return;
#endif

    eglSwapBuffers(_linearwin.egl.display, window->context.egl.surface);
}

static void swapIntervalEGL(int interval)
{
    eglSwapInterval(_linearwin.egl.display, interval);
}

static int extensionSupportedEGL(const char* extension)
{
    const char* extensions = eglQueryString(_linearwin.egl.display, EGL_EXTENSIONS);
    if (extensions)
    {
        if (_linearwinStringInExtensionString(extension, extensions))
            return LINEARWIN_TRUE;
    }

    return LINEARWIN_FALSE;
}

static LINEARWINglproc getProcAddressEGL(const char* procname)
{
    _LINEARWINwindow* window = _linearwinPlatformGetTls(&_linearwin.contextSlot);

    if (window->context.egl.client)
    {
        LINEARWINglproc proc = (LINEARWINglproc) _linearwin_dlsym(window->context.egl.client,
                                                   procname);
        if (proc)
            return proc;
    }

    return eglGetProcAddress(procname);
}

static void destroyContextEGL(_LINEARWINwindow* window)
{
#if defined(_LINEARWIN_X11)
    // NOTE: Do not unload libGL.so.1 while the X11 display is still open,
    //       as it will make XCloseDisplay segfault
    if (window->context.client != LINEARWIN_OPENGL_API)
#endif // _LINEARWIN_X11
    {
        if (window->context.egl.client)
        {
            _linearwin_dlclose(window->context.egl.client);
            window->context.egl.client = NULL;
        }
    }

    if (window->context.egl.surface)
    {
        eglDestroySurface(_linearwin.egl.display, window->context.egl.surface);
        window->context.egl.surface = EGL_NO_SURFACE;
    }

    if (window->context.egl.handle)
    {
        eglDestroyContext(_linearwin.egl.display, window->context.egl.handle);
        window->context.egl.handle = EGL_NO_CONTEXT;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize EGL
//
LINEARWINbool _linearwinInitEGL(void)
{
    int i;
    const char* sonames[] =
    {
#if defined(_LINEARWIN_EGL_LIBRARY)
        _LINEARWIN_EGL_LIBRARY,
#elif defined(_LINEARWIN_WIN32)
        "libEGL.dll",
        "EGL.dll",
#elif defined(_LINEARWIN_COCOA)
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

    if (_linearwin.egl.handle)
        return LINEARWIN_TRUE;

    for (i = 0;  sonames[i];  i++)
    {
        _linearwin.egl.handle = _linearwin_dlopen(sonames[i]);
        if (_linearwin.egl.handle)
            break;
    }

    if (!_linearwin.egl.handle)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE, "EGL: Library not found");
        return LINEARWIN_FALSE;
    }

    _linearwin.egl.prefix = (strncmp(sonames[i], "lib", 3) == 0);

    _linearwin.egl.GetConfigAttrib = (PFN_eglGetConfigAttrib)
        _linearwin_dlsym(_linearwin.egl.handle, "eglGetConfigAttrib");
    _linearwin.egl.GetConfigs = (PFN_eglGetConfigs)
        _linearwin_dlsym(_linearwin.egl.handle, "eglGetConfigs");
    _linearwin.egl.GetDisplay = (PFN_eglGetDisplay)
        _linearwin_dlsym(_linearwin.egl.handle, "eglGetDisplay");
    _linearwin.egl.GetError = (PFN_eglGetError)
        _linearwin_dlsym(_linearwin.egl.handle, "eglGetError");
    _linearwin.egl.Initialize = (PFN_eglInitialize)
        _linearwin_dlsym(_linearwin.egl.handle, "eglInitialize");
    _linearwin.egl.Terminate = (PFN_eglTerminate)
        _linearwin_dlsym(_linearwin.egl.handle, "eglTerminate");
    _linearwin.egl.BindAPI = (PFN_eglBindAPI)
        _linearwin_dlsym(_linearwin.egl.handle, "eglBindAPI");
    _linearwin.egl.CreateContext = (PFN_eglCreateContext)
        _linearwin_dlsym(_linearwin.egl.handle, "eglCreateContext");
    _linearwin.egl.DestroySurface = (PFN_eglDestroySurface)
        _linearwin_dlsym(_linearwin.egl.handle, "eglDestroySurface");
    _linearwin.egl.DestroyContext = (PFN_eglDestroyContext)
        _linearwin_dlsym(_linearwin.egl.handle, "eglDestroyContext");
    _linearwin.egl.CreateWindowSurface = (PFN_eglCreateWindowSurface)
        _linearwin_dlsym(_linearwin.egl.handle, "eglCreateWindowSurface");
    _linearwin.egl.MakeCurrent = (PFN_eglMakeCurrent)
        _linearwin_dlsym(_linearwin.egl.handle, "eglMakeCurrent");
    _linearwin.egl.SwapBuffers = (PFN_eglSwapBuffers)
        _linearwin_dlsym(_linearwin.egl.handle, "eglSwapBuffers");
    _linearwin.egl.SwapInterval = (PFN_eglSwapInterval)
        _linearwin_dlsym(_linearwin.egl.handle, "eglSwapInterval");
    _linearwin.egl.QueryString = (PFN_eglQueryString)
        _linearwin_dlsym(_linearwin.egl.handle, "eglQueryString");
    _linearwin.egl.GetProcAddress = (PFN_eglGetProcAddress)
        _linearwin_dlsym(_linearwin.egl.handle, "eglGetProcAddress");

    if (!_linearwin.egl.GetConfigAttrib ||
        !_linearwin.egl.GetConfigs ||
        !_linearwin.egl.GetDisplay ||
        !_linearwin.egl.GetError ||
        !_linearwin.egl.Initialize ||
        !_linearwin.egl.Terminate ||
        !_linearwin.egl.BindAPI ||
        !_linearwin.egl.CreateContext ||
        !_linearwin.egl.DestroySurface ||
        !_linearwin.egl.DestroyContext ||
        !_linearwin.egl.CreateWindowSurface ||
        !_linearwin.egl.MakeCurrent ||
        !_linearwin.egl.SwapBuffers ||
        !_linearwin.egl.SwapInterval ||
        !_linearwin.egl.QueryString ||
        !_linearwin.egl.GetProcAddress)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "EGL: Failed to load required entry points");

        _linearwinTerminateEGL();
        return LINEARWIN_FALSE;
    }

    _linearwin.egl.display = eglGetDisplay(_LINEARWIN_EGL_NATIVE_DISPLAY);
    if (_linearwin.egl.display == EGL_NO_DISPLAY)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                        "EGL: Failed to get EGL display: %s",
                        getEGLErrorString(eglGetError()));

        _linearwinTerminateEGL();
        return LINEARWIN_FALSE;
    }

    if (!eglInitialize(_linearwin.egl.display, &_linearwin.egl.major, &_linearwin.egl.minor))
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                        "EGL: Failed to initialize EGL: %s",
                        getEGLErrorString(eglGetError()));

        _linearwinTerminateEGL();
        return LINEARWIN_FALSE;
    }

    _linearwin.egl.KHR_create_context =
        extensionSupportedEGL("EGL_KHR_create_context");
    _linearwin.egl.KHR_create_context_no_error =
        extensionSupportedEGL("EGL_KHR_create_context_no_error");
    _linearwin.egl.KHR_gl_colorspace =
        extensionSupportedEGL("EGL_KHR_gl_colorspace");
    _linearwin.egl.KHR_get_all_proc_addresses =
        extensionSupportedEGL("EGL_KHR_get_all_proc_addresses");
    _linearwin.egl.KHR_context_flush_control =
        extensionSupportedEGL("EGL_KHR_context_flush_control");
    _linearwin.egl.EXT_present_opaque =
        extensionSupportedEGL("EGL_EXT_present_opaque");

    return LINEARWIN_TRUE;
}

// Terminate EGL
//
void _linearwinTerminateEGL(void)
{
    if (_linearwin.egl.display)
    {
        eglTerminate(_linearwin.egl.display);
        _linearwin.egl.display = EGL_NO_DISPLAY;
    }

    if (_linearwin.egl.handle)
    {
        _linearwin_dlclose(_linearwin.egl.handle);
        _linearwin.egl.handle = NULL;
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
LINEARWINbool _linearwinCreateContextEGL(_LINEARWINwindow* window,
                               const _LINEARWINctxconfig* ctxconfig,
                               const _LINEARWINfbconfig* fbconfig)
{
    EGLint attribs[40];
    EGLConfig config;
    EGLContext share = NULL;
    int index = 0;

    if (!_linearwin.egl.display)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE, "EGL: API not available");
        return LINEARWIN_FALSE;
    }

    if (ctxconfig->share)
        share = ctxconfig->share->context.egl.handle;

    if (!chooseEGLConfig(ctxconfig, fbconfig, &config))
    {
        _linearwinInputError(LINEARWIN_FORMAT_UNAVAILABLE,
                        "EGL: Failed to find a suitable EGLConfig");
        return LINEARWIN_FALSE;
    }

    if (ctxconfig->client == LINEARWIN_OPENGL_ES_API)
    {
        if (!eglBindAPI(EGL_OPENGL_ES_API))
        {
            _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                            "EGL: Failed to bind OpenGL ES: %s",
                            getEGLErrorString(eglGetError()));
            return LINEARWIN_FALSE;
        }
    }
    else
    {
        if (!eglBindAPI(EGL_OPENGL_API))
        {
            _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                            "EGL: Failed to bind OpenGL: %s",
                            getEGLErrorString(eglGetError()));
            return LINEARWIN_FALSE;
        }
    }

    if (_linearwin.egl.KHR_create_context)
    {
        int mask = 0, flags = 0;

        if (ctxconfig->client == LINEARWIN_OPENGL_API)
        {
            if (ctxconfig->forward)
                flags |= EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR;

            if (ctxconfig->profile == LINEARWIN_OPENGL_CORE_PROFILE)
                mask |= EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR;
            else if (ctxconfig->profile == LINEARWIN_OPENGL_COMPAT_PROFILE)
                mask |= EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR;
        }

        if (ctxconfig->debug)
            flags |= EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;

        if (ctxconfig->robustness)
        {
            if (ctxconfig->robustness == LINEARWIN_NO_RESET_NOTIFICATION)
            {
                setAttrib(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR,
                          EGL_NO_RESET_NOTIFICATION_KHR);
            }
            else if (ctxconfig->robustness == LINEARWIN_LOSE_CONTEXT_ON_RESET)
            {
                setAttrib(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR,
                          EGL_LOSE_CONTEXT_ON_RESET_KHR);
            }

            flags |= EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR;
        }

        if (ctxconfig->noerror)
        {
            if (_linearwin.egl.KHR_create_context_no_error)
                setAttrib(EGL_CONTEXT_OPENGL_NO_ERROR_KHR, LINEARWIN_TRUE);
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
        if (ctxconfig->client == LINEARWIN_OPENGL_ES_API)
            setAttrib(EGL_CONTEXT_CLIENT_VERSION, ctxconfig->major);
    }

    if (_linearwin.egl.KHR_context_flush_control)
    {
        if (ctxconfig->release == LINEARWIN_RELEASE_BEHAVIOR_NONE)
        {
            setAttrib(EGL_CONTEXT_RELEASE_BEHAVIOR_KHR,
                      EGL_CONTEXT_RELEASE_BEHAVIOR_NONE_KHR);
        }
        else if (ctxconfig->release == LINEARWIN_RELEASE_BEHAVIOR_FLUSH)
        {
            setAttrib(EGL_CONTEXT_RELEASE_BEHAVIOR_KHR,
                      EGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_KHR);
        }
    }

    setAttrib(EGL_NONE, EGL_NONE);

    window->context.egl.handle = eglCreateContext(_linearwin.egl.display,
                                                  config, share, attribs);

    if (window->context.egl.handle == EGL_NO_CONTEXT)
    {
        _linearwinInputError(LINEARWIN_VERSION_UNAVAILABLE,
                        "EGL: Failed to create context: %s",
                        getEGLErrorString(eglGetError()));
        return LINEARWIN_FALSE;
    }

    // Set up attributes for surface creation
    index = 0;

    if (fbconfig->sRGB)
    {
        if (_linearwin.egl.KHR_gl_colorspace)
            setAttrib(EGL_GL_COLORSPACE_KHR, EGL_GL_COLORSPACE_SRGB_KHR);
    }

    if (!fbconfig->doublebuffer)
        setAttrib(EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER);

    if (_linearwin.egl.EXT_present_opaque)
        setAttrib(EGL_PRESENT_OPAQUE_EXT, !fbconfig->transparent);

    setAttrib(EGL_NONE, EGL_NONE);

    window->context.egl.surface =
        eglCreateWindowSurface(_linearwin.egl.display,
                               config,
                               _LINEARWIN_EGL_NATIVE_WINDOW,
                               attribs);
    if (window->context.egl.surface == EGL_NO_SURFACE)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "EGL: Failed to create window surface: %s",
                        getEGLErrorString(eglGetError()));
        return LINEARWIN_FALSE;
    }

    window->context.egl.config = config;

    // Load the appropriate client library
    if (!_linearwin.egl.KHR_get_all_proc_addresses)
    {
        int i;
        const char** sonames;
        const char* es1sonames[] =
        {
#if defined(_LINEARWIN_GLESV1_LIBRARY)
            _LINEARWIN_GLESV1_LIBRARY,
#elif defined(_LINEARWIN_WIN32)
            "GLESv1_CM.dll",
            "libGLES_CM.dll",
#elif defined(_LINEARWIN_COCOA)
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
#if defined(_LINEARWIN_GLESV2_LIBRARY)
            _LINEARWIN_GLESV2_LIBRARY,
#elif defined(_LINEARWIN_WIN32)
            "GLESv2.dll",
            "libGLESv2.dll",
#elif defined(_LINEARWIN_COCOA)
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
#if defined(_LINEARWIN_OPENGL_LIBRARY)
            _LINEARWIN_OPENGL_LIBRARY,
#elif defined(_LINEARWIN_WIN32)
#elif defined(_LINEARWIN_COCOA)
#elif defined(__OpenBSD__) || defined(__NetBSD__)
            "libGL.so",
#else
            "libGL.so.1",
#endif
            NULL
        };

        if (ctxconfig->client == LINEARWIN_OPENGL_ES_API)
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
            if (_linearwin.egl.prefix != (strncmp(sonames[i], "lib", 3) == 0))
                continue;

            window->context.egl.client = _linearwin_dlopen(sonames[i]);
            if (window->context.egl.client)
                break;
        }

        if (!window->context.egl.client)
        {
            _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                            "EGL: Failed to load client library");
            return LINEARWIN_FALSE;
        }
    }

    window->context.makeCurrent = makeContextCurrentEGL;
    window->context.swapBuffers = swapBuffersEGL;
    window->context.swapInterval = swapIntervalEGL;
    window->context.extensionSupported = extensionSupportedEGL;
    window->context.getProcAddress = getProcAddressEGL;
    window->context.destroy = destroyContextEGL;

    return LINEARWIN_TRUE;
}

#undef setAttrib

// Returns the Visual and depth of the chosen EGLConfig
//
#if defined(_LINEARWIN_X11)
LINEARWINbool _linearwinChooseVisualEGL(const _LINEARWINwndconfig* wndconfig,
                              const _LINEARWINctxconfig* ctxconfig,
                              const _LINEARWINfbconfig* fbconfig,
                              Visual** visual, int* depth)
{
    XVisualInfo* result;
    XVisualInfo desired;
    EGLConfig native;
    EGLint visualID = 0, count = 0;
    const long vimask = VisualScreenMask | VisualIDMask;

    if (!chooseEGLConfig(ctxconfig, fbconfig, &native))
    {
        _linearwinInputError(LINEARWIN_FORMAT_UNAVAILABLE,
                        "EGL: Failed to find a suitable EGLConfig");
        return LINEARWIN_FALSE;
    }

    eglGetConfigAttrib(_linearwin.egl.display, native,
                       EGL_NATIVE_VISUAL_ID, &visualID);

    desired.screen = _linearwin.x11.screen;
    desired.visualid = visualID;

    result = XGetVisualInfo(_linearwin.x11.display, vimask, &desired, &count);
    if (!result)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "EGL: Failed to retrieve Visual for EGLConfig");
        return LINEARWIN_FALSE;
    }

    *visual = result->visual;
    *depth = result->depth;

    XFree(result);
    return LINEARWIN_TRUE;
}
#endif // _LINEARWIN_X11


//////////////////////////////////////////////////////////////////////////
//////                        LINEARWIN native API                       //////
//////////////////////////////////////////////////////////////////////////

LINEARWINAPI EGLDisplay linearwinGetEGLDisplay(void)
{
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(EGL_NO_DISPLAY);
    return _linearwin.egl.display;
}

LINEARWINAPI EGLContext linearwinGetEGLContext(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(EGL_NO_CONTEXT);

    if (window->context.source != LINEARWIN_EGL_CONTEXT_API)
    {
        _linearwinInputError(LINEARWIN_NO_WINDOW_CONTEXT, NULL);
        return EGL_NO_CONTEXT;
    }

    return window->context.egl.handle;
}

LINEARWINAPI EGLSurface linearwinGetEGLSurface(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(EGL_NO_SURFACE);

    if (window->context.source != LINEARWIN_EGL_CONTEXT_API)
    {
        _linearwinInputError(LINEARWIN_NO_WINDOW_CONTEXT, NULL);
        return EGL_NO_SURFACE;
    }

    return window->context.egl.surface;
}

