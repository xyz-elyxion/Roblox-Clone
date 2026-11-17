//========================================================================
// LINEARWIN 3.3 GLX - www.linearwin.org
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifndef GLXBadProfileARB
 #define GLXBadProfileARB 13
#endif


// Returns the specified attribute of the specified GLXFBConfig
//
static int getGLXFBConfigAttrib(GLXFBConfig fbconfig, int attrib)
{
    int value;
    glXGetFBConfigAttrib(_linearwin.x11.display, fbconfig, attrib, &value);
    return value;
}

// Return the GLXFBConfig most closely matching the specified hints
//
static LINEARWINbool chooseGLXFBConfig(const _LINEARWINfbconfig* desired,
                                  GLXFBConfig* result)
{
    GLXFBConfig* nativeConfigs;
    _LINEARWINfbconfig* usableConfigs;
    const _LINEARWINfbconfig* closest;
    int i, nativeCount, usableCount;
    const char* vendor;
    LINEARWINbool trustWindowBit = LINEARWIN_TRUE;

    // HACK: This is a (hopefully temporary) workaround for Chromium
    //       (VirtualBox GL) not setting the window bit on any GLXFBConfigs
    vendor = glXGetClientString(_linearwin.x11.display, GLX_VENDOR);
    if (vendor && strcmp(vendor, "Chromium") == 0)
        trustWindowBit = LINEARWIN_FALSE;

    nativeConfigs =
        glXGetFBConfigs(_linearwin.x11.display, _linearwin.x11.screen, &nativeCount);
    if (!nativeConfigs || !nativeCount)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE, "GLX: No GLXFBConfigs returned");
        return LINEARWIN_FALSE;
    }

    usableConfigs = calloc(nativeCount, sizeof(_LINEARWINfbconfig));
    usableCount = 0;

    for (i = 0;  i < nativeCount;  i++)
    {
        const GLXFBConfig n = nativeConfigs[i];
        _LINEARWINfbconfig* u = usableConfigs + usableCount;

        // Only consider RGBA GLXFBConfigs
        if (!(getGLXFBConfigAttrib(n, GLX_RENDER_TYPE) & GLX_RGBA_BIT))
            continue;

        // Only consider window GLXFBConfigs
        if (!(getGLXFBConfigAttrib(n, GLX_DRAWABLE_TYPE) & GLX_WINDOW_BIT))
        {
            if (trustWindowBit)
                continue;
        }

        if (getGLXFBConfigAttrib(n, GLX_DOUBLEBUFFER) != desired->doublebuffer)
            continue;

        if (desired->transparent)
        {
            XVisualInfo* vi = glXGetVisualFromFBConfig(_linearwin.x11.display, n);
            if (vi)
            {
                u->transparent = _linearwinIsVisualTransparentX11(vi->visual);
                XFree(vi);
            }
        }

        u->redBits = getGLXFBConfigAttrib(n, GLX_RED_SIZE);
        u->greenBits = getGLXFBConfigAttrib(n, GLX_GREEN_SIZE);
        u->blueBits = getGLXFBConfigAttrib(n, GLX_BLUE_SIZE);

        u->alphaBits = getGLXFBConfigAttrib(n, GLX_ALPHA_SIZE);
        u->depthBits = getGLXFBConfigAttrib(n, GLX_DEPTH_SIZE);
        u->stencilBits = getGLXFBConfigAttrib(n, GLX_STENCIL_SIZE);

        u->accumRedBits = getGLXFBConfigAttrib(n, GLX_ACCUM_RED_SIZE);
        u->accumGreenBits = getGLXFBConfigAttrib(n, GLX_ACCUM_GREEN_SIZE);
        u->accumBlueBits = getGLXFBConfigAttrib(n, GLX_ACCUM_BLUE_SIZE);
        u->accumAlphaBits = getGLXFBConfigAttrib(n, GLX_ACCUM_ALPHA_SIZE);

        u->auxBuffers = getGLXFBConfigAttrib(n, GLX_AUX_BUFFERS);

        if (getGLXFBConfigAttrib(n, GLX_STEREO))
            u->stereo = LINEARWIN_TRUE;

        if (_linearwin.glx.ARB_multisample)
            u->samples = getGLXFBConfigAttrib(n, GLX_SAMPLES);

        if (_linearwin.glx.ARB_framebuffer_sRGB || _linearwin.glx.EXT_framebuffer_sRGB)
            u->sRGB = getGLXFBConfigAttrib(n, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB);

        u->handle = (uintptr_t) n;
        usableCount++;
    }

    closest = _linearwinChooseFBConfig(desired, usableConfigs, usableCount);
    if (closest)
        *result = (GLXFBConfig) closest->handle;

    XFree(nativeConfigs);
    free(usableConfigs);

    return closest != NULL;
}

// Create the OpenGL context using legacy API
//
static GLXContext createLegacyContextGLX(_LINEARWINwindow* window,
                                         GLXFBConfig fbconfig,
                                         GLXContext share)
{
    return glXCreateNewContext(_linearwin.x11.display,
                               fbconfig,
                               GLX_RGBA_TYPE,
                               share,
                               True);
}

static void makeContextCurrentGLX(_LINEARWINwindow* window)
{
    if (window)
    {
        if (!glXMakeCurrent(_linearwin.x11.display,
                            window->context.glx.window,
                            window->context.glx.handle))
        {
            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "GLX: Failed to make context current");
            return;
        }
    }
    else
    {
        if (!glXMakeCurrent(_linearwin.x11.display, None, NULL))
        {
            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "GLX: Failed to clear current context");
            return;
        }
    }

    _linearwinPlatformSetTls(&_linearwin.contextSlot, window);
}

static void swapBuffersGLX(_LINEARWINwindow* window)
{
    glXSwapBuffers(_linearwin.x11.display, window->context.glx.window);
}

static void swapIntervalGLX(int interval)
{
    _LINEARWINwindow* window = _linearwinPlatformGetTls(&_linearwin.contextSlot);

    if (_linearwin.glx.EXT_swap_control)
    {
        _linearwin.glx.SwapIntervalEXT(_linearwin.x11.display,
                                  window->context.glx.window,
                                  interval);
    }
    else if (_linearwin.glx.MESA_swap_control)
        _linearwin.glx.SwapIntervalMESA(interval);
    else if (_linearwin.glx.SGI_swap_control)
    {
        if (interval > 0)
            _linearwin.glx.SwapIntervalSGI(interval);
    }
}

static int extensionSupportedGLX(const char* extension)
{
    const char* extensions =
        glXQueryExtensionsString(_linearwin.x11.display, _linearwin.x11.screen);
    if (extensions)
    {
        if (_linearwinStringInExtensionString(extension, extensions))
            return LINEARWIN_TRUE;
    }

    return LINEARWIN_FALSE;
}

static LINEARWINglproc getProcAddressGLX(const char* procname)
{
    if (_linearwin.glx.GetProcAddress)
        return _linearwin.glx.GetProcAddress((const GLubyte*) procname);
    else if (_linearwin.glx.GetProcAddressARB)
        return _linearwin.glx.GetProcAddressARB((const GLubyte*) procname);
    else
        return _linearwin_dlsym(_linearwin.glx.handle, procname);
}

static void destroyContextGLX(_LINEARWINwindow* window)
{
    if (window->context.glx.window)
    {
        glXDestroyWindow(_linearwin.x11.display, window->context.glx.window);
        window->context.glx.window = None;
    }

    if (window->context.glx.handle)
    {
        glXDestroyContext(_linearwin.x11.display, window->context.glx.handle);
        window->context.glx.handle = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize GLX
//
LINEARWINbool _linearwinInitGLX(void)
{
    int i;
    const char* sonames[] =
    {
#if defined(_LINEARWIN_GLX_LIBRARY)
        _LINEARWIN_GLX_LIBRARY,
#elif defined(__CYGWIN__)
        "libGL-1.so",
#elif defined(__OpenBSD__) || defined(__NetBSD__)
        "libGL.so",
#else
        "libGL.so.1",
        "libGL.so",
#endif
        NULL
    };

    if (_linearwin.glx.handle)
        return LINEARWIN_TRUE;

    for (i = 0;  sonames[i];  i++)
    {
        _linearwin.glx.handle = _linearwin_dlopen(sonames[i]);
        if (_linearwin.glx.handle)
            break;
    }

    if (!_linearwin.glx.handle)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE, "GLX: Failed to load GLX");
        return LINEARWIN_FALSE;
    }

    _linearwin.glx.GetFBConfigs =
        _linearwin_dlsym(_linearwin.glx.handle, "glXGetFBConfigs");
    _linearwin.glx.GetFBConfigAttrib =
        _linearwin_dlsym(_linearwin.glx.handle, "glXGetFBConfigAttrib");
    _linearwin.glx.GetClientString =
        _linearwin_dlsym(_linearwin.glx.handle, "glXGetClientString");
    _linearwin.glx.QueryExtension =
        _linearwin_dlsym(_linearwin.glx.handle, "glXQueryExtension");
    _linearwin.glx.QueryVersion =
        _linearwin_dlsym(_linearwin.glx.handle, "glXQueryVersion");
    _linearwin.glx.DestroyContext =
        _linearwin_dlsym(_linearwin.glx.handle, "glXDestroyContext");
    _linearwin.glx.MakeCurrent =
        _linearwin_dlsym(_linearwin.glx.handle, "glXMakeCurrent");
    _linearwin.glx.SwapBuffers =
        _linearwin_dlsym(_linearwin.glx.handle, "glXSwapBuffers");
    _linearwin.glx.QueryExtensionsString =
        _linearwin_dlsym(_linearwin.glx.handle, "glXQueryExtensionsString");
    _linearwin.glx.CreateNewContext =
        _linearwin_dlsym(_linearwin.glx.handle, "glXCreateNewContext");
    _linearwin.glx.CreateWindow =
        _linearwin_dlsym(_linearwin.glx.handle, "glXCreateWindow");
    _linearwin.glx.DestroyWindow =
        _linearwin_dlsym(_linearwin.glx.handle, "glXDestroyWindow");
    _linearwin.glx.GetVisualFromFBConfig =
        _linearwin_dlsym(_linearwin.glx.handle, "glXGetVisualFromFBConfig");

    if (!_linearwin.glx.GetFBConfigs ||
        !_linearwin.glx.GetFBConfigAttrib ||
        !_linearwin.glx.GetClientString ||
        !_linearwin.glx.QueryExtension ||
        !_linearwin.glx.QueryVersion ||
        !_linearwin.glx.DestroyContext ||
        !_linearwin.glx.MakeCurrent ||
        !_linearwin.glx.SwapBuffers ||
        !_linearwin.glx.QueryExtensionsString ||
        !_linearwin.glx.CreateNewContext ||
        !_linearwin.glx.CreateWindow ||
        !_linearwin.glx.DestroyWindow ||
        !_linearwin.glx.GetVisualFromFBConfig)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "GLX: Failed to load required entry points");
        return LINEARWIN_FALSE;
    }

    // NOTE: Unlike GLX 1.3 entry points these are not required to be present
    _linearwin.glx.GetProcAddress = (PFNGLXGETPROCADDRESSPROC)
        _linearwin_dlsym(_linearwin.glx.handle, "glXGetProcAddress");
    _linearwin.glx.GetProcAddressARB = (PFNGLXGETPROCADDRESSPROC)
        _linearwin_dlsym(_linearwin.glx.handle, "glXGetProcAddressARB");

    if (!glXQueryExtension(_linearwin.x11.display,
                           &_linearwin.glx.errorBase,
                           &_linearwin.glx.eventBase))
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE, "GLX: GLX extension not found");
        return LINEARWIN_FALSE;
    }

    if (!glXQueryVersion(_linearwin.x11.display, &_linearwin.glx.major, &_linearwin.glx.minor))
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                        "GLX: Failed to query GLX version");
        return LINEARWIN_FALSE;
    }

    if (_linearwin.glx.major == 1 && _linearwin.glx.minor < 3)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                        "GLX: GLX version 1.3 is required");
        return LINEARWIN_FALSE;
    }

    if (extensionSupportedGLX("GLX_EXT_swap_control"))
    {
        _linearwin.glx.SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)
            getProcAddressGLX("glXSwapIntervalEXT");

        if (_linearwin.glx.SwapIntervalEXT)
            _linearwin.glx.EXT_swap_control = LINEARWIN_TRUE;
    }

    if (extensionSupportedGLX("GLX_SGI_swap_control"))
    {
        _linearwin.glx.SwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)
            getProcAddressGLX("glXSwapIntervalSGI");

        if (_linearwin.glx.SwapIntervalSGI)
            _linearwin.glx.SGI_swap_control = LINEARWIN_TRUE;
    }

    if (extensionSupportedGLX("GLX_MESA_swap_control"))
    {
        _linearwin.glx.SwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)
            getProcAddressGLX("glXSwapIntervalMESA");

        if (_linearwin.glx.SwapIntervalMESA)
            _linearwin.glx.MESA_swap_control = LINEARWIN_TRUE;
    }

    if (extensionSupportedGLX("GLX_ARB_multisample"))
        _linearwin.glx.ARB_multisample = LINEARWIN_TRUE;

    if (extensionSupportedGLX("GLX_ARB_framebuffer_sRGB"))
        _linearwin.glx.ARB_framebuffer_sRGB = LINEARWIN_TRUE;

    if (extensionSupportedGLX("GLX_EXT_framebuffer_sRGB"))
        _linearwin.glx.EXT_framebuffer_sRGB = LINEARWIN_TRUE;

    if (extensionSupportedGLX("GLX_ARB_create_context"))
    {
        _linearwin.glx.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
            getProcAddressGLX("glXCreateContextAttribsARB");

        if (_linearwin.glx.CreateContextAttribsARB)
            _linearwin.glx.ARB_create_context = LINEARWIN_TRUE;
    }

    if (extensionSupportedGLX("GLX_ARB_create_context_robustness"))
        _linearwin.glx.ARB_create_context_robustness = LINEARWIN_TRUE;

    if (extensionSupportedGLX("GLX_ARB_create_context_profile"))
        _linearwin.glx.ARB_create_context_profile = LINEARWIN_TRUE;

    if (extensionSupportedGLX("GLX_EXT_create_context_es2_profile"))
        _linearwin.glx.EXT_create_context_es2_profile = LINEARWIN_TRUE;

    if (extensionSupportedGLX("GLX_ARB_create_context_no_error"))
        _linearwin.glx.ARB_create_context_no_error = LINEARWIN_TRUE;

    if (extensionSupportedGLX("GLX_ARB_context_flush_control"))
        _linearwin.glx.ARB_context_flush_control = LINEARWIN_TRUE;

    return LINEARWIN_TRUE;
}

// Terminate GLX
//
void _linearwinTerminateGLX(void)
{
    // NOTE: This function must not call any X11 functions, as it is called
    //       after XCloseDisplay (see _linearwinPlatformTerminate for details)

    if (_linearwin.glx.handle)
    {
        _linearwin_dlclose(_linearwin.glx.handle);
        _linearwin.glx.handle = NULL;
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
LINEARWINbool _linearwinCreateContextGLX(_LINEARWINwindow* window,
                               const _LINEARWINctxconfig* ctxconfig,
                               const _LINEARWINfbconfig* fbconfig)
{
    int attribs[40];
    GLXFBConfig native = NULL;
    GLXContext share = NULL;

    if (ctxconfig->share)
        share = ctxconfig->share->context.glx.handle;

    if (!chooseGLXFBConfig(fbconfig, &native))
    {
        _linearwinInputError(LINEARWIN_FORMAT_UNAVAILABLE,
                        "GLX: Failed to find a suitable GLXFBConfig");
        return LINEARWIN_FALSE;
    }

    if (ctxconfig->client == LINEARWIN_OPENGL_ES_API)
    {
        if (!_linearwin.glx.ARB_create_context ||
            !_linearwin.glx.ARB_create_context_profile ||
            !_linearwin.glx.EXT_create_context_es2_profile)
        {
            _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                            "GLX: OpenGL ES requested but GLX_EXT_create_context_es2_profile is unavailable");
            return LINEARWIN_FALSE;
        }
    }

    if (ctxconfig->forward)
    {
        if (!_linearwin.glx.ARB_create_context)
        {
            _linearwinInputError(LINEARWIN_VERSION_UNAVAILABLE,
                            "GLX: Forward compatibility requested but GLX_ARB_create_context_profile is unavailable");
            return LINEARWIN_FALSE;
        }
    }

    if (ctxconfig->profile)
    {
        if (!_linearwin.glx.ARB_create_context ||
            !_linearwin.glx.ARB_create_context_profile)
        {
            _linearwinInputError(LINEARWIN_VERSION_UNAVAILABLE,
                            "GLX: An OpenGL profile requested but GLX_ARB_create_context_profile is unavailable");
            return LINEARWIN_FALSE;
        }
    }

    _linearwinGrabErrorHandlerX11();

    if (_linearwin.glx.ARB_create_context)
    {
        int index = 0, mask = 0, flags = 0;

        if (ctxconfig->client == LINEARWIN_OPENGL_API)
        {
            if (ctxconfig->forward)
                flags |= GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

            if (ctxconfig->profile == LINEARWIN_OPENGL_CORE_PROFILE)
                mask |= GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
            else if (ctxconfig->profile == LINEARWIN_OPENGL_COMPAT_PROFILE)
                mask |= GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
        }
        else
            mask |= GLX_CONTEXT_ES2_PROFILE_BIT_EXT;

        if (ctxconfig->debug)
            flags |= GLX_CONTEXT_DEBUG_BIT_ARB;

        if (ctxconfig->robustness)
        {
            if (_linearwin.glx.ARB_create_context_robustness)
            {
                if (ctxconfig->robustness == LINEARWIN_NO_RESET_NOTIFICATION)
                {
                    setAttrib(GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
                              GLX_NO_RESET_NOTIFICATION_ARB);
                }
                else if (ctxconfig->robustness == LINEARWIN_LOSE_CONTEXT_ON_RESET)
                {
                    setAttrib(GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
                              GLX_LOSE_CONTEXT_ON_RESET_ARB);
                }

                flags |= GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB;
            }
        }

        if (ctxconfig->release)
        {
            if (_linearwin.glx.ARB_context_flush_control)
            {
                if (ctxconfig->release == LINEARWIN_RELEASE_BEHAVIOR_NONE)
                {
                    setAttrib(GLX_CONTEXT_RELEASE_BEHAVIOR_ARB,
                              GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB);
                }
                else if (ctxconfig->release == LINEARWIN_RELEASE_BEHAVIOR_FLUSH)
                {
                    setAttrib(GLX_CONTEXT_RELEASE_BEHAVIOR_ARB,
                              GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB);
                }
            }
        }

        if (ctxconfig->noerror)
        {
            if (_linearwin.glx.ARB_create_context_no_error)
                setAttrib(GLX_CONTEXT_OPENGL_NO_ERROR_ARB, LINEARWIN_TRUE);
        }

        // NOTE: Only request an explicitly versioned context when necessary, as
        //       explicitly requesting version 1.0 does not always return the
        //       highest version supported by the driver
        if (ctxconfig->major != 1 || ctxconfig->minor != 0)
        {
            setAttrib(GLX_CONTEXT_MAJOR_VERSION_ARB, ctxconfig->major);
            setAttrib(GLX_CONTEXT_MINOR_VERSION_ARB, ctxconfig->minor);
        }

        if (mask)
            setAttrib(GLX_CONTEXT_PROFILE_MASK_ARB, mask);

        if (flags)
            setAttrib(GLX_CONTEXT_FLAGS_ARB, flags);

        setAttrib(None, None);

        window->context.glx.handle =
            _linearwin.glx.CreateContextAttribsARB(_linearwin.x11.display,
                                              native,
                                              share,
                                              True,
                                              attribs);

        // HACK: This is a fallback for broken versions of the Mesa
        //       implementation of GLX_ARB_create_context_profile that fail
        //       default 1.0 context creation with a GLXBadProfileARB error in
        //       violation of the extension spec
        if (!window->context.glx.handle)
        {
            if (_linearwin.x11.errorCode == _linearwin.glx.errorBase + GLXBadProfileARB &&
                ctxconfig->client == LINEARWIN_OPENGL_API &&
                ctxconfig->profile == LINEARWIN_OPENGL_ANY_PROFILE &&
                ctxconfig->forward == LINEARWIN_FALSE)
            {
                window->context.glx.handle =
                    createLegacyContextGLX(window, native, share);
            }
        }
    }
    else
    {
        window->context.glx.handle =
            createLegacyContextGLX(window, native, share);
    }

    _linearwinReleaseErrorHandlerX11();

    if (!window->context.glx.handle)
    {
        _linearwinInputErrorX11(LINEARWIN_VERSION_UNAVAILABLE, "GLX: Failed to create context");
        return LINEARWIN_FALSE;
    }

    window->context.glx.window =
        glXCreateWindow(_linearwin.x11.display, native, window->x11.handle, NULL);
    if (!window->context.glx.window)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR, "GLX: Failed to create window");
        return LINEARWIN_FALSE;
    }

    window->context.makeCurrent = makeContextCurrentGLX;
    window->context.swapBuffers = swapBuffersGLX;
    window->context.swapInterval = swapIntervalGLX;
    window->context.extensionSupported = extensionSupportedGLX;
    window->context.getProcAddress = getProcAddressGLX;
    window->context.destroy = destroyContextGLX;

    return LINEARWIN_TRUE;
}

#undef setAttrib

// Returns the Visual and depth of the chosen GLXFBConfig
//
LINEARWINbool _linearwinChooseVisualGLX(const _LINEARWINwndconfig* wndconfig,
                              const _LINEARWINctxconfig* ctxconfig,
                              const _LINEARWINfbconfig* fbconfig,
                              Visual** visual, int* depth)
{
    GLXFBConfig native;
    XVisualInfo* result;

    if (!chooseGLXFBConfig(fbconfig, &native))
    {
        _linearwinInputError(LINEARWIN_FORMAT_UNAVAILABLE,
                        "GLX: Failed to find a suitable GLXFBConfig");
        return LINEARWIN_FALSE;
    }

    result = glXGetVisualFromFBConfig(_linearwin.x11.display, native);
    if (!result)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "GLX: Failed to retrieve Visual for GLXFBConfig");
        return LINEARWIN_FALSE;
    }

    *visual = result->visual;
    *depth  = result->depth;

    XFree(result);
    return LINEARWIN_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                        LINEARWIN native API                       //////
//////////////////////////////////////////////////////////////////////////

LINEARWINAPI GLXContext linearwinGetGLXContext(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (window->context.source != LINEARWIN_NATIVE_CONTEXT_API)
    {
        _linearwinInputError(LINEARWIN_NO_WINDOW_CONTEXT, NULL);
        return NULL;
    }

    return window->context.glx.handle;
}

LINEARWINAPI GLXWindow linearwinGetGLXWindow(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(None);

    if (window->context.source != LINEARWIN_NATIVE_CONTEXT_API)
    {
        _linearwinInputError(LINEARWIN_NO_WINDOW_CONTEXT, NULL);
        return None;
    }

    return window->context.glx.window;
}

