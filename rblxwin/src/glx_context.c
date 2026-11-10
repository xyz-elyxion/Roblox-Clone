//========================================================================
// RBLXWIN 3.3 GLX - www.rblxwin.org
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
    glXGetFBConfigAttrib(_rblxwin.x11.display, fbconfig, attrib, &value);
    return value;
}

// Return the GLXFBConfig most closely matching the specified hints
//
static RBLXWINbool chooseGLXFBConfig(const _RBLXWINfbconfig* desired,
                                  GLXFBConfig* result)
{
    GLXFBConfig* nativeConfigs;
    _RBLXWINfbconfig* usableConfigs;
    const _RBLXWINfbconfig* closest;
    int i, nativeCount, usableCount;
    const char* vendor;
    RBLXWINbool trustWindowBit = RBLXWIN_TRUE;

    // HACK: This is a (hopefully temporary) workaround for Chromium
    //       (VirtualBox GL) not setting the window bit on any GLXFBConfigs
    vendor = glXGetClientString(_rblxwin.x11.display, GLX_VENDOR);
    if (vendor && strcmp(vendor, "Chromium") == 0)
        trustWindowBit = RBLXWIN_FALSE;

    nativeConfigs =
        glXGetFBConfigs(_rblxwin.x11.display, _rblxwin.x11.screen, &nativeCount);
    if (!nativeConfigs || !nativeCount)
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE, "GLX: No GLXFBConfigs returned");
        return RBLXWIN_FALSE;
    }

    usableConfigs = calloc(nativeCount, sizeof(_RBLXWINfbconfig));
    usableCount = 0;

    for (i = 0;  i < nativeCount;  i++)
    {
        const GLXFBConfig n = nativeConfigs[i];
        _RBLXWINfbconfig* u = usableConfigs + usableCount;

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
            XVisualInfo* vi = glXGetVisualFromFBConfig(_rblxwin.x11.display, n);
            if (vi)
            {
                u->transparent = _rblxwinIsVisualTransparentX11(vi->visual);
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
            u->stereo = RBLXWIN_TRUE;

        if (_rblxwin.glx.ARB_multisample)
            u->samples = getGLXFBConfigAttrib(n, GLX_SAMPLES);

        if (_rblxwin.glx.ARB_framebuffer_sRGB || _rblxwin.glx.EXT_framebuffer_sRGB)
            u->sRGB = getGLXFBConfigAttrib(n, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB);

        u->handle = (uintptr_t) n;
        usableCount++;
    }

    closest = _rblxwinChooseFBConfig(desired, usableConfigs, usableCount);
    if (closest)
        *result = (GLXFBConfig) closest->handle;

    XFree(nativeConfigs);
    free(usableConfigs);

    return closest != NULL;
}

// Create the OpenGL context using legacy API
//
static GLXContext createLegacyContextGLX(_RBLXWINwindow* window,
                                         GLXFBConfig fbconfig,
                                         GLXContext share)
{
    return glXCreateNewContext(_rblxwin.x11.display,
                               fbconfig,
                               GLX_RGBA_TYPE,
                               share,
                               True);
}

static void makeContextCurrentGLX(_RBLXWINwindow* window)
{
    if (window)
    {
        if (!glXMakeCurrent(_rblxwin.x11.display,
                            window->context.glx.window,
                            window->context.glx.handle))
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "GLX: Failed to make context current");
            return;
        }
    }
    else
    {
        if (!glXMakeCurrent(_rblxwin.x11.display, None, NULL))
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "GLX: Failed to clear current context");
            return;
        }
    }

    _rblxwinPlatformSetTls(&_rblxwin.contextSlot, window);
}

static void swapBuffersGLX(_RBLXWINwindow* window)
{
    glXSwapBuffers(_rblxwin.x11.display, window->context.glx.window);
}

static void swapIntervalGLX(int interval)
{
    _RBLXWINwindow* window = _rblxwinPlatformGetTls(&_rblxwin.contextSlot);

    if (_rblxwin.glx.EXT_swap_control)
    {
        _rblxwin.glx.SwapIntervalEXT(_rblxwin.x11.display,
                                  window->context.glx.window,
                                  interval);
    }
    else if (_rblxwin.glx.MESA_swap_control)
        _rblxwin.glx.SwapIntervalMESA(interval);
    else if (_rblxwin.glx.SGI_swap_control)
    {
        if (interval > 0)
            _rblxwin.glx.SwapIntervalSGI(interval);
    }
}

static int extensionSupportedGLX(const char* extension)
{
    const char* extensions =
        glXQueryExtensionsString(_rblxwin.x11.display, _rblxwin.x11.screen);
    if (extensions)
    {
        if (_rblxwinStringInExtensionString(extension, extensions))
            return RBLXWIN_TRUE;
    }

    return RBLXWIN_FALSE;
}

static RBLXWINglproc getProcAddressGLX(const char* procname)
{
    if (_rblxwin.glx.GetProcAddress)
        return _rblxwin.glx.GetProcAddress((const GLubyte*) procname);
    else if (_rblxwin.glx.GetProcAddressARB)
        return _rblxwin.glx.GetProcAddressARB((const GLubyte*) procname);
    else
        return _rblxwin_dlsym(_rblxwin.glx.handle, procname);
}

static void destroyContextGLX(_RBLXWINwindow* window)
{
    if (window->context.glx.window)
    {
        glXDestroyWindow(_rblxwin.x11.display, window->context.glx.window);
        window->context.glx.window = None;
    }

    if (window->context.glx.handle)
    {
        glXDestroyContext(_rblxwin.x11.display, window->context.glx.handle);
        window->context.glx.handle = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize GLX
//
RBLXWINbool _rblxwinInitGLX(void)
{
    int i;
    const char* sonames[] =
    {
#if defined(_RBLXWIN_GLX_LIBRARY)
        _RBLXWIN_GLX_LIBRARY,
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

    if (_rblxwin.glx.handle)
        return RBLXWIN_TRUE;

    for (i = 0;  sonames[i];  i++)
    {
        _rblxwin.glx.handle = _rblxwin_dlopen(sonames[i]);
        if (_rblxwin.glx.handle)
            break;
    }

    if (!_rblxwin.glx.handle)
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE, "GLX: Failed to load GLX");
        return RBLXWIN_FALSE;
    }

    _rblxwin.glx.GetFBConfigs =
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXGetFBConfigs");
    _rblxwin.glx.GetFBConfigAttrib =
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXGetFBConfigAttrib");
    _rblxwin.glx.GetClientString =
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXGetClientString");
    _rblxwin.glx.QueryExtension =
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXQueryExtension");
    _rblxwin.glx.QueryVersion =
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXQueryVersion");
    _rblxwin.glx.DestroyContext =
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXDestroyContext");
    _rblxwin.glx.MakeCurrent =
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXMakeCurrent");
    _rblxwin.glx.SwapBuffers =
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXSwapBuffers");
    _rblxwin.glx.QueryExtensionsString =
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXQueryExtensionsString");
    _rblxwin.glx.CreateNewContext =
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXCreateNewContext");
    _rblxwin.glx.CreateWindow =
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXCreateWindow");
    _rblxwin.glx.DestroyWindow =
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXDestroyWindow");
    _rblxwin.glx.GetVisualFromFBConfig =
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXGetVisualFromFBConfig");

    if (!_rblxwin.glx.GetFBConfigs ||
        !_rblxwin.glx.GetFBConfigAttrib ||
        !_rblxwin.glx.GetClientString ||
        !_rblxwin.glx.QueryExtension ||
        !_rblxwin.glx.QueryVersion ||
        !_rblxwin.glx.DestroyContext ||
        !_rblxwin.glx.MakeCurrent ||
        !_rblxwin.glx.SwapBuffers ||
        !_rblxwin.glx.QueryExtensionsString ||
        !_rblxwin.glx.CreateNewContext ||
        !_rblxwin.glx.CreateWindow ||
        !_rblxwin.glx.DestroyWindow ||
        !_rblxwin.glx.GetVisualFromFBConfig)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "GLX: Failed to load required entry points");
        return RBLXWIN_FALSE;
    }

    // NOTE: Unlike GLX 1.3 entry points these are not required to be present
    _rblxwin.glx.GetProcAddress = (PFNGLXGETPROCADDRESSPROC)
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXGetProcAddress");
    _rblxwin.glx.GetProcAddressARB = (PFNGLXGETPROCADDRESSPROC)
        _rblxwin_dlsym(_rblxwin.glx.handle, "glXGetProcAddressARB");

    if (!glXQueryExtension(_rblxwin.x11.display,
                           &_rblxwin.glx.errorBase,
                           &_rblxwin.glx.eventBase))
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE, "GLX: GLX extension not found");
        return RBLXWIN_FALSE;
    }

    if (!glXQueryVersion(_rblxwin.x11.display, &_rblxwin.glx.major, &_rblxwin.glx.minor))
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                        "GLX: Failed to query GLX version");
        return RBLXWIN_FALSE;
    }

    if (_rblxwin.glx.major == 1 && _rblxwin.glx.minor < 3)
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                        "GLX: GLX version 1.3 is required");
        return RBLXWIN_FALSE;
    }

    if (extensionSupportedGLX("GLX_EXT_swap_control"))
    {
        _rblxwin.glx.SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)
            getProcAddressGLX("glXSwapIntervalEXT");

        if (_rblxwin.glx.SwapIntervalEXT)
            _rblxwin.glx.EXT_swap_control = RBLXWIN_TRUE;
    }

    if (extensionSupportedGLX("GLX_SGI_swap_control"))
    {
        _rblxwin.glx.SwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)
            getProcAddressGLX("glXSwapIntervalSGI");

        if (_rblxwin.glx.SwapIntervalSGI)
            _rblxwin.glx.SGI_swap_control = RBLXWIN_TRUE;
    }

    if (extensionSupportedGLX("GLX_MESA_swap_control"))
    {
        _rblxwin.glx.SwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)
            getProcAddressGLX("glXSwapIntervalMESA");

        if (_rblxwin.glx.SwapIntervalMESA)
            _rblxwin.glx.MESA_swap_control = RBLXWIN_TRUE;
    }

    if (extensionSupportedGLX("GLX_ARB_multisample"))
        _rblxwin.glx.ARB_multisample = RBLXWIN_TRUE;

    if (extensionSupportedGLX("GLX_ARB_framebuffer_sRGB"))
        _rblxwin.glx.ARB_framebuffer_sRGB = RBLXWIN_TRUE;

    if (extensionSupportedGLX("GLX_EXT_framebuffer_sRGB"))
        _rblxwin.glx.EXT_framebuffer_sRGB = RBLXWIN_TRUE;

    if (extensionSupportedGLX("GLX_ARB_create_context"))
    {
        _rblxwin.glx.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
            getProcAddressGLX("glXCreateContextAttribsARB");

        if (_rblxwin.glx.CreateContextAttribsARB)
            _rblxwin.glx.ARB_create_context = RBLXWIN_TRUE;
    }

    if (extensionSupportedGLX("GLX_ARB_create_context_robustness"))
        _rblxwin.glx.ARB_create_context_robustness = RBLXWIN_TRUE;

    if (extensionSupportedGLX("GLX_ARB_create_context_profile"))
        _rblxwin.glx.ARB_create_context_profile = RBLXWIN_TRUE;

    if (extensionSupportedGLX("GLX_EXT_create_context_es2_profile"))
        _rblxwin.glx.EXT_create_context_es2_profile = RBLXWIN_TRUE;

    if (extensionSupportedGLX("GLX_ARB_create_context_no_error"))
        _rblxwin.glx.ARB_create_context_no_error = RBLXWIN_TRUE;

    if (extensionSupportedGLX("GLX_ARB_context_flush_control"))
        _rblxwin.glx.ARB_context_flush_control = RBLXWIN_TRUE;

    return RBLXWIN_TRUE;
}

// Terminate GLX
//
void _rblxwinTerminateGLX(void)
{
    // NOTE: This function must not call any X11 functions, as it is called
    //       after XCloseDisplay (see _rblxwinPlatformTerminate for details)

    if (_rblxwin.glx.handle)
    {
        _rblxwin_dlclose(_rblxwin.glx.handle);
        _rblxwin.glx.handle = NULL;
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
RBLXWINbool _rblxwinCreateContextGLX(_RBLXWINwindow* window,
                               const _RBLXWINctxconfig* ctxconfig,
                               const _RBLXWINfbconfig* fbconfig)
{
    int attribs[40];
    GLXFBConfig native = NULL;
    GLXContext share = NULL;

    if (ctxconfig->share)
        share = ctxconfig->share->context.glx.handle;

    if (!chooseGLXFBConfig(fbconfig, &native))
    {
        _rblxwinInputError(RBLXWIN_FORMAT_UNAVAILABLE,
                        "GLX: Failed to find a suitable GLXFBConfig");
        return RBLXWIN_FALSE;
    }

    if (ctxconfig->client == RBLXWIN_OPENGL_ES_API)
    {
        if (!_rblxwin.glx.ARB_create_context ||
            !_rblxwin.glx.ARB_create_context_profile ||
            !_rblxwin.glx.EXT_create_context_es2_profile)
        {
            _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                            "GLX: OpenGL ES requested but GLX_EXT_create_context_es2_profile is unavailable");
            return RBLXWIN_FALSE;
        }
    }

    if (ctxconfig->forward)
    {
        if (!_rblxwin.glx.ARB_create_context)
        {
            _rblxwinInputError(RBLXWIN_VERSION_UNAVAILABLE,
                            "GLX: Forward compatibility requested but GLX_ARB_create_context_profile is unavailable");
            return RBLXWIN_FALSE;
        }
    }

    if (ctxconfig->profile)
    {
        if (!_rblxwin.glx.ARB_create_context ||
            !_rblxwin.glx.ARB_create_context_profile)
        {
            _rblxwinInputError(RBLXWIN_VERSION_UNAVAILABLE,
                            "GLX: An OpenGL profile requested but GLX_ARB_create_context_profile is unavailable");
            return RBLXWIN_FALSE;
        }
    }

    _rblxwinGrabErrorHandlerX11();

    if (_rblxwin.glx.ARB_create_context)
    {
        int index = 0, mask = 0, flags = 0;

        if (ctxconfig->client == RBLXWIN_OPENGL_API)
        {
            if (ctxconfig->forward)
                flags |= GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

            if (ctxconfig->profile == RBLXWIN_OPENGL_CORE_PROFILE)
                mask |= GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
            else if (ctxconfig->profile == RBLXWIN_OPENGL_COMPAT_PROFILE)
                mask |= GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
        }
        else
            mask |= GLX_CONTEXT_ES2_PROFILE_BIT_EXT;

        if (ctxconfig->debug)
            flags |= GLX_CONTEXT_DEBUG_BIT_ARB;

        if (ctxconfig->robustness)
        {
            if (_rblxwin.glx.ARB_create_context_robustness)
            {
                if (ctxconfig->robustness == RBLXWIN_NO_RESET_NOTIFICATION)
                {
                    setAttrib(GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
                              GLX_NO_RESET_NOTIFICATION_ARB);
                }
                else if (ctxconfig->robustness == RBLXWIN_LOSE_CONTEXT_ON_RESET)
                {
                    setAttrib(GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
                              GLX_LOSE_CONTEXT_ON_RESET_ARB);
                }

                flags |= GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB;
            }
        }

        if (ctxconfig->release)
        {
            if (_rblxwin.glx.ARB_context_flush_control)
            {
                if (ctxconfig->release == RBLXWIN_RELEASE_BEHAVIOR_NONE)
                {
                    setAttrib(GLX_CONTEXT_RELEASE_BEHAVIOR_ARB,
                              GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB);
                }
                else if (ctxconfig->release == RBLXWIN_RELEASE_BEHAVIOR_FLUSH)
                {
                    setAttrib(GLX_CONTEXT_RELEASE_BEHAVIOR_ARB,
                              GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB);
                }
            }
        }

        if (ctxconfig->noerror)
        {
            if (_rblxwin.glx.ARB_create_context_no_error)
                setAttrib(GLX_CONTEXT_OPENGL_NO_ERROR_ARB, RBLXWIN_TRUE);
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
            _rblxwin.glx.CreateContextAttribsARB(_rblxwin.x11.display,
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
            if (_rblxwin.x11.errorCode == _rblxwin.glx.errorBase + GLXBadProfileARB &&
                ctxconfig->client == RBLXWIN_OPENGL_API &&
                ctxconfig->profile == RBLXWIN_OPENGL_ANY_PROFILE &&
                ctxconfig->forward == RBLXWIN_FALSE)
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

    _rblxwinReleaseErrorHandlerX11();

    if (!window->context.glx.handle)
    {
        _rblxwinInputErrorX11(RBLXWIN_VERSION_UNAVAILABLE, "GLX: Failed to create context");
        return RBLXWIN_FALSE;
    }

    window->context.glx.window =
        glXCreateWindow(_rblxwin.x11.display, native, window->x11.handle, NULL);
    if (!window->context.glx.window)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR, "GLX: Failed to create window");
        return RBLXWIN_FALSE;
    }

    window->context.makeCurrent = makeContextCurrentGLX;
    window->context.swapBuffers = swapBuffersGLX;
    window->context.swapInterval = swapIntervalGLX;
    window->context.extensionSupported = extensionSupportedGLX;
    window->context.getProcAddress = getProcAddressGLX;
    window->context.destroy = destroyContextGLX;

    return RBLXWIN_TRUE;
}

#undef setAttrib

// Returns the Visual and depth of the chosen GLXFBConfig
//
RBLXWINbool _rblxwinChooseVisualGLX(const _RBLXWINwndconfig* wndconfig,
                              const _RBLXWINctxconfig* ctxconfig,
                              const _RBLXWINfbconfig* fbconfig,
                              Visual** visual, int* depth)
{
    GLXFBConfig native;
    XVisualInfo* result;

    if (!chooseGLXFBConfig(fbconfig, &native))
    {
        _rblxwinInputError(RBLXWIN_FORMAT_UNAVAILABLE,
                        "GLX: Failed to find a suitable GLXFBConfig");
        return RBLXWIN_FALSE;
    }

    result = glXGetVisualFromFBConfig(_rblxwin.x11.display, native);
    if (!result)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "GLX: Failed to retrieve Visual for GLXFBConfig");
        return RBLXWIN_FALSE;
    }

    *visual = result->visual;
    *depth  = result->depth;

    XFree(result);
    return RBLXWIN_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                        RBLXWIN native API                       //////
//////////////////////////////////////////////////////////////////////////

RBLXWINAPI GLXContext rblxwinGetGLXContext(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (window->context.source != RBLXWIN_NATIVE_CONTEXT_API)
    {
        _rblxwinInputError(RBLXWIN_NO_WINDOW_CONTEXT, NULL);
        return NULL;
    }

    return window->context.glx.handle;
}

RBLXWINAPI GLXWindow rblxwinGetGLXWindow(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(None);

    if (window->context.source != RBLXWIN_NATIVE_CONTEXT_API)
    {
        _rblxwinInputError(RBLXWIN_NO_WINDOW_CONTEXT, NULL);
        return None;
    }

    return window->context.glx.window;
}

