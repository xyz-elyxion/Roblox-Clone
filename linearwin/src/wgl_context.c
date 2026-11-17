//========================================================================
// LINEARWIN 3.3 WGL - www.linearwin.org
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

#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

// Return the value corresponding to the specified attribute
//
static int findPixelFormatAttribValue(const int* attribs,
                                      int attribCount,
                                      const int* values,
                                      int attrib)
{
    int i;

    for (i = 0;  i < attribCount;  i++)
    {
        if (attribs[i] == attrib)
            return values[i];
    }

    _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                         "WGL: Unknown pixel format attribute requested");
    return 0;
}

#define addAttrib(a) \
{ \
    assert((size_t) attribCount < sizeof(attribs) / sizeof(attribs[0])); \
    attribs[attribCount++] = a; \
}
#define findAttribValue(a) \
    findPixelFormatAttribValue(attribs, attribCount, values, a)

// Return a list of available and usable framebuffer configs
//
static int choosePixelFormat(_LINEARWINwindow* window,
                             const _LINEARWINctxconfig* ctxconfig,
                             const _LINEARWINfbconfig* fbconfig)
{
    _LINEARWINfbconfig* usableConfigs;
    const _LINEARWINfbconfig* closest;
    int i, pixelFormat, nativeCount, usableCount = 0, attribCount = 0;
    int attribs[40];
    int values[sizeof(attribs) / sizeof(attribs[0])];

    if (_linearwin.wgl.ARB_pixel_format)
    {
        const int attrib = WGL_NUMBER_PIXEL_FORMATS_ARB;

        if (!wglGetPixelFormatAttribivARB(window->context.wgl.dc,
                                          1, 0, 1, &attrib, &nativeCount))
        {
            _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                                 "WGL: Failed to retrieve pixel format attribute");
            return 0;
        }

        addAttrib(WGL_SUPPORT_OPENGL_ARB);
        addAttrib(WGL_DRAW_TO_WINDOW_ARB);
        addAttrib(WGL_PIXEL_TYPE_ARB);
        addAttrib(WGL_ACCELERATION_ARB);
        addAttrib(WGL_RED_BITS_ARB);
        addAttrib(WGL_RED_SHIFT_ARB);
        addAttrib(WGL_GREEN_BITS_ARB);
        addAttrib(WGL_GREEN_SHIFT_ARB);
        addAttrib(WGL_BLUE_BITS_ARB);
        addAttrib(WGL_BLUE_SHIFT_ARB);
        addAttrib(WGL_ALPHA_BITS_ARB);
        addAttrib(WGL_ALPHA_SHIFT_ARB);
        addAttrib(WGL_DEPTH_BITS_ARB);
        addAttrib(WGL_STENCIL_BITS_ARB);
        addAttrib(WGL_ACCUM_BITS_ARB);
        addAttrib(WGL_ACCUM_RED_BITS_ARB);
        addAttrib(WGL_ACCUM_GREEN_BITS_ARB);
        addAttrib(WGL_ACCUM_BLUE_BITS_ARB);
        addAttrib(WGL_ACCUM_ALPHA_BITS_ARB);
        addAttrib(WGL_AUX_BUFFERS_ARB);
        addAttrib(WGL_STEREO_ARB);
        addAttrib(WGL_DOUBLE_BUFFER_ARB);

        if (_linearwin.wgl.ARB_multisample)
            addAttrib(WGL_SAMPLES_ARB);

        if (ctxconfig->client == LINEARWIN_OPENGL_API)
        {
            if (_linearwin.wgl.ARB_framebuffer_sRGB || _linearwin.wgl.EXT_framebuffer_sRGB)
                addAttrib(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB);
        }
        else
        {
            if (_linearwin.wgl.EXT_colorspace)
                addAttrib(WGL_COLORSPACE_EXT);
        }
    }
    else
    {
        nativeCount = DescribePixelFormat(window->context.wgl.dc,
                                          1,
                                          sizeof(PIXELFORMATDESCRIPTOR),
                                          NULL);
    }

    usableConfigs = calloc(nativeCount, sizeof(_LINEARWINfbconfig));

    for (i = 0;  i < nativeCount;  i++)
    {
        _LINEARWINfbconfig* u = usableConfigs + usableCount;
        pixelFormat = i + 1;

        if (_linearwin.wgl.ARB_pixel_format)
        {
            // Get pixel format attributes through "modern" extension

            if (!wglGetPixelFormatAttribivARB(window->context.wgl.dc,
                                              pixelFormat, 0,
                                              attribCount,
                                              attribs, values))
            {
                _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                                    "WGL: Failed to retrieve pixel format attributes");

                free(usableConfigs);
                return 0;
            }

            if (!findAttribValue(WGL_SUPPORT_OPENGL_ARB) ||
                !findAttribValue(WGL_DRAW_TO_WINDOW_ARB))
            {
                continue;
            }

            if (findAttribValue(WGL_PIXEL_TYPE_ARB) != WGL_TYPE_RGBA_ARB)
                continue;

            if (findAttribValue(WGL_ACCELERATION_ARB) == WGL_NO_ACCELERATION_ARB)
                continue;

            if (findAttribValue(WGL_DOUBLE_BUFFER_ARB) != fbconfig->doublebuffer)
                continue;

            u->redBits = findAttribValue(WGL_RED_BITS_ARB);
            u->greenBits = findAttribValue(WGL_GREEN_BITS_ARB);
            u->blueBits = findAttribValue(WGL_BLUE_BITS_ARB);
            u->alphaBits = findAttribValue(WGL_ALPHA_BITS_ARB);

            u->depthBits = findAttribValue(WGL_DEPTH_BITS_ARB);
            u->stencilBits = findAttribValue(WGL_STENCIL_BITS_ARB);

            u->accumRedBits = findAttribValue(WGL_ACCUM_RED_BITS_ARB);
            u->accumGreenBits = findAttribValue(WGL_ACCUM_GREEN_BITS_ARB);
            u->accumBlueBits = findAttribValue(WGL_ACCUM_BLUE_BITS_ARB);
            u->accumAlphaBits = findAttribValue(WGL_ACCUM_ALPHA_BITS_ARB);

            u->auxBuffers = findAttribValue(WGL_AUX_BUFFERS_ARB);

            if (findAttribValue(WGL_STEREO_ARB))
                u->stereo = LINEARWIN_TRUE;

            if (_linearwin.wgl.ARB_multisample)
                u->samples = findAttribValue(WGL_SAMPLES_ARB);

            if (ctxconfig->client == LINEARWIN_OPENGL_API)
            {
                if (_linearwin.wgl.ARB_framebuffer_sRGB ||
                    _linearwin.wgl.EXT_framebuffer_sRGB)
                {
                    if (findAttribValue(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB))
                        u->sRGB = LINEARWIN_TRUE;
                }
            }
            else
            {
                if (_linearwin.wgl.EXT_colorspace)
                {
                    if (findAttribValue(WGL_COLORSPACE_EXT) == WGL_COLORSPACE_SRGB_EXT)
                        u->sRGB = LINEARWIN_TRUE;
                }
            }
        }
        else
        {
            // Get pixel format attributes through legacy PFDs

            PIXELFORMATDESCRIPTOR pfd;

            if (!DescribePixelFormat(window->context.wgl.dc,
                                     pixelFormat,
                                     sizeof(PIXELFORMATDESCRIPTOR),
                                     &pfd))
            {
                _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                                    "WGL: Failed to describe pixel format");

                free(usableConfigs);
                return 0;
            }

            if (!(pfd.dwFlags & PFD_DRAW_TO_WINDOW) ||
                !(pfd.dwFlags & PFD_SUPPORT_OPENGL))
            {
                continue;
            }

            if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) &&
                (pfd.dwFlags & PFD_GENERIC_FORMAT))
            {
                continue;
            }

            if (pfd.iPixelType != PFD_TYPE_RGBA)
                continue;

            if (!!(pfd.dwFlags & PFD_DOUBLEBUFFER) != fbconfig->doublebuffer)
                continue;

            u->redBits = pfd.cRedBits;
            u->greenBits = pfd.cGreenBits;
            u->blueBits = pfd.cBlueBits;
            u->alphaBits = pfd.cAlphaBits;

            u->depthBits = pfd.cDepthBits;
            u->stencilBits = pfd.cStencilBits;

            u->accumRedBits = pfd.cAccumRedBits;
            u->accumGreenBits = pfd.cAccumGreenBits;
            u->accumBlueBits = pfd.cAccumBlueBits;
            u->accumAlphaBits = pfd.cAccumAlphaBits;

            u->auxBuffers = pfd.cAuxBuffers;

            if (pfd.dwFlags & PFD_STEREO)
                u->stereo = LINEARWIN_TRUE;
        }

        u->handle = pixelFormat;
        usableCount++;
    }

    if (!usableCount)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                        "WGL: The driver does not appear to support OpenGL");

        free(usableConfigs);
        return 0;
    }

    closest = _linearwinChooseFBConfig(fbconfig, usableConfigs, usableCount);
    if (!closest)
    {
        _linearwinInputError(LINEARWIN_FORMAT_UNAVAILABLE,
                        "WGL: Failed to find a suitable pixel format");

        free(usableConfigs);
        return 0;
    }

    pixelFormat = (int) closest->handle;
    free(usableConfigs);

    return pixelFormat;
}

#undef addAttrib
#undef findAttribValue

static void makeContextCurrentWGL(_LINEARWINwindow* window)
{
    if (window)
    {
        if (wglMakeCurrent(window->context.wgl.dc, window->context.wgl.handle))
            _linearwinPlatformSetTls(&_linearwin.contextSlot, window);
        else
        {
            _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                                 "WGL: Failed to make context current");
            _linearwinPlatformSetTls(&_linearwin.contextSlot, NULL);
        }
    }
    else
    {
        if (!wglMakeCurrent(NULL, NULL))
        {
            _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                                 "WGL: Failed to clear current context");
        }

        _linearwinPlatformSetTls(&_linearwin.contextSlot, NULL);
    }
}

static void swapBuffersWGL(_LINEARWINwindow* window)
{
    if (!window->monitor)
    {
        if (IsWindowsVistaOrGreater())
        {
            // DWM Composition is always enabled on Win8+
            BOOL enabled = IsWindows8OrGreater();

            // HACK: Use DwmFlush when desktop composition is enabled
            if (enabled ||
                (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled))
            {
                int count = abs(window->context.wgl.interval);
                while (count--)
                    DwmFlush();
            }
        }
    }

    SwapBuffers(window->context.wgl.dc);
}

static void swapIntervalWGL(int interval)
{
    _LINEARWINwindow* window = _linearwinPlatformGetTls(&_linearwin.contextSlot);

    window->context.wgl.interval = interval;

    if (!window->monitor)
    {
        if (IsWindowsVistaOrGreater())
        {
            // DWM Composition is always enabled on Win8+
            BOOL enabled = IsWindows8OrGreater();

            // HACK: Disable WGL swap interval when desktop composition is enabled to
            //       avoid interfering with DWM vsync
            if (enabled ||
                (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled))
                interval = 0;
        }
    }

    if (_linearwin.wgl.EXT_swap_control)
        wglSwapIntervalEXT(interval);
}

static int extensionSupportedWGL(const char* extension)
{
    const char* extensions = NULL;

    if (_linearwin.wgl.GetExtensionsStringARB)
        extensions = wglGetExtensionsStringARB(wglGetCurrentDC());
    else if (_linearwin.wgl.GetExtensionsStringEXT)
        extensions = wglGetExtensionsStringEXT();

    if (!extensions)
        return LINEARWIN_FALSE;

    return _linearwinStringInExtensionString(extension, extensions);
}

static LINEARWINglproc getProcAddressWGL(const char* procname)
{
    const LINEARWINglproc proc = (LINEARWINglproc) wglGetProcAddress(procname);
    if (proc)
        return proc;

    return (LINEARWINglproc) GetProcAddress(_linearwin.wgl.instance, procname);
}

static void destroyContextWGL(_LINEARWINwindow* window)
{
    if (window->context.wgl.handle)
    {
        wglDeleteContext(window->context.wgl.handle);
        window->context.wgl.handle = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize WGL
//
LINEARWINbool _linearwinInitWGL(void)
{
    PIXELFORMATDESCRIPTOR pfd;
    HGLRC prc, rc;
    HDC pdc, dc;

    if (_linearwin.wgl.instance)
        return LINEARWIN_TRUE;

    _linearwin.wgl.instance = LoadLibraryA("opengl32.dll");
    if (!_linearwin.wgl.instance)
    {
        _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                             "WGL: Failed to load opengl32.dll");
        return LINEARWIN_FALSE;
    }

    _linearwin.wgl.CreateContext = (PFN_wglCreateContext)
        GetProcAddress(_linearwin.wgl.instance, "wglCreateContext");
    _linearwin.wgl.DeleteContext = (PFN_wglDeleteContext)
        GetProcAddress(_linearwin.wgl.instance, "wglDeleteContext");
    _linearwin.wgl.GetProcAddress = (PFN_wglGetProcAddress)
        GetProcAddress(_linearwin.wgl.instance, "wglGetProcAddress");
    _linearwin.wgl.GetCurrentDC = (PFN_wglGetCurrentDC)
        GetProcAddress(_linearwin.wgl.instance, "wglGetCurrentDC");
    _linearwin.wgl.GetCurrentContext = (PFN_wglGetCurrentContext)
        GetProcAddress(_linearwin.wgl.instance, "wglGetCurrentContext");
    _linearwin.wgl.MakeCurrent = (PFN_wglMakeCurrent)
        GetProcAddress(_linearwin.wgl.instance, "wglMakeCurrent");
    _linearwin.wgl.ShareLists = (PFN_wglShareLists)
        GetProcAddress(_linearwin.wgl.instance, "wglShareLists");

    // NOTE: A dummy context has to be created for opengl32.dll to load the
    //       OpenGL ICD, from which we can then query WGL extensions
    // NOTE: This code will accept the Microsoft GDI ICD; accelerated context
    //       creation failure occurs during manual pixel format enumeration

    dc = GetDC(_linearwin.win32.helperWindowHandle);

    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;

    if (!SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd))
    {
        _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                             "WGL: Failed to set pixel format for dummy context");
        return LINEARWIN_FALSE;
    }

    rc = wglCreateContext(dc);
    if (!rc)
    {
        _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                             "WGL: Failed to create dummy context");
        return LINEARWIN_FALSE;
    }

    pdc = wglGetCurrentDC();
    prc = wglGetCurrentContext();

    if (!wglMakeCurrent(dc, rc))
    {
        _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                             "WGL: Failed to make dummy context current");
        wglMakeCurrent(pdc, prc);
        wglDeleteContext(rc);
        return LINEARWIN_FALSE;
    }

    // NOTE: Functions must be loaded first as they're needed to retrieve the
    //       extension string that tells us whether the functions are supported
    _linearwin.wgl.GetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)
        wglGetProcAddress("wglGetExtensionsStringEXT");
    _linearwin.wgl.GetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)
        wglGetProcAddress("wglGetExtensionsStringARB");
    _linearwin.wgl.CreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
        wglGetProcAddress("wglCreateContextAttribsARB");
    _linearwin.wgl.SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
        wglGetProcAddress("wglSwapIntervalEXT");
    _linearwin.wgl.GetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)
        wglGetProcAddress("wglGetPixelFormatAttribivARB");

    // NOTE: WGL_ARB_extensions_string and WGL_EXT_extensions_string are not
    //       checked below as we are already using them
    _linearwin.wgl.ARB_multisample =
        extensionSupportedWGL("WGL_ARB_multisample");
    _linearwin.wgl.ARB_framebuffer_sRGB =
        extensionSupportedWGL("WGL_ARB_framebuffer_sRGB");
    _linearwin.wgl.EXT_framebuffer_sRGB =
        extensionSupportedWGL("WGL_EXT_framebuffer_sRGB");
    _linearwin.wgl.ARB_create_context =
        extensionSupportedWGL("WGL_ARB_create_context");
    _linearwin.wgl.ARB_create_context_profile =
        extensionSupportedWGL("WGL_ARB_create_context_profile");
    _linearwin.wgl.EXT_create_context_es2_profile =
        extensionSupportedWGL("WGL_EXT_create_context_es2_profile");
    _linearwin.wgl.ARB_create_context_robustness =
        extensionSupportedWGL("WGL_ARB_create_context_robustness");
    _linearwin.wgl.ARB_create_context_no_error =
        extensionSupportedWGL("WGL_ARB_create_context_no_error");
    _linearwin.wgl.EXT_swap_control =
        extensionSupportedWGL("WGL_EXT_swap_control");
    _linearwin.wgl.EXT_colorspace =
        extensionSupportedWGL("WGL_EXT_colorspace");
    _linearwin.wgl.ARB_pixel_format =
        extensionSupportedWGL("WGL_ARB_pixel_format");
    _linearwin.wgl.ARB_context_flush_control =
        extensionSupportedWGL("WGL_ARB_context_flush_control");

    wglMakeCurrent(pdc, prc);
    wglDeleteContext(rc);
    return LINEARWIN_TRUE;
}

// Terminate WGL
//
void _linearwinTerminateWGL(void)
{
    if (_linearwin.wgl.instance)
        FreeLibrary(_linearwin.wgl.instance);
}

#define setAttrib(a, v) \
{ \
    assert(((size_t) index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
    attribs[index++] = a; \
    attribs[index++] = v; \
}

// Create the OpenGL or OpenGL ES context
//
LINEARWINbool _linearwinCreateContextWGL(_LINEARWINwindow* window,
                               const _LINEARWINctxconfig* ctxconfig,
                               const _LINEARWINfbconfig* fbconfig)
{
    int attribs[40];
    int pixelFormat;
    PIXELFORMATDESCRIPTOR pfd;
    HGLRC share = NULL;

    if (ctxconfig->share)
        share = ctxconfig->share->context.wgl.handle;

    window->context.wgl.dc = GetDC(window->win32.handle);
    if (!window->context.wgl.dc)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "WGL: Failed to retrieve DC for window");
        return LINEARWIN_FALSE;
    }

    pixelFormat = choosePixelFormat(window, ctxconfig, fbconfig);
    if (!pixelFormat)
        return LINEARWIN_FALSE;

    if (!DescribePixelFormat(window->context.wgl.dc,
                             pixelFormat, sizeof(pfd), &pfd))
    {
        _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                             "WGL: Failed to retrieve PFD for selected pixel format");
        return LINEARWIN_FALSE;
    }

    if (!SetPixelFormat(window->context.wgl.dc, pixelFormat, &pfd))
    {
        _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                             "WGL: Failed to set selected pixel format");
        return LINEARWIN_FALSE;
    }

    if (ctxconfig->client == LINEARWIN_OPENGL_API)
    {
        if (ctxconfig->forward)
        {
            if (!_linearwin.wgl.ARB_create_context)
            {
                _linearwinInputError(LINEARWIN_VERSION_UNAVAILABLE,
                                "WGL: A forward compatible OpenGL context requested but WGL_ARB_create_context is unavailable");
                return LINEARWIN_FALSE;
            }
        }

        if (ctxconfig->profile)
        {
            if (!_linearwin.wgl.ARB_create_context_profile)
            {
                _linearwinInputError(LINEARWIN_VERSION_UNAVAILABLE,
                                "WGL: OpenGL profile requested but WGL_ARB_create_context_profile is unavailable");
                return LINEARWIN_FALSE;
            }
        }
    }
    else
    {
        if (!_linearwin.wgl.ARB_create_context ||
            !_linearwin.wgl.ARB_create_context_profile ||
            !_linearwin.wgl.EXT_create_context_es2_profile)
        {
            _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                            "WGL: OpenGL ES requested but WGL_ARB_create_context_es2_profile is unavailable");
            return LINEARWIN_FALSE;
        }
    }

    if (_linearwin.wgl.ARB_create_context)
    {
        int index = 0, mask = 0, flags = 0;

        if (ctxconfig->client == LINEARWIN_OPENGL_API)
        {
            if (ctxconfig->forward)
                flags |= WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

            if (ctxconfig->profile == LINEARWIN_OPENGL_CORE_PROFILE)
                mask |= WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
            else if (ctxconfig->profile == LINEARWIN_OPENGL_COMPAT_PROFILE)
                mask |= WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
        }
        else
            mask |= WGL_CONTEXT_ES2_PROFILE_BIT_EXT;

        if (ctxconfig->debug)
            flags |= WGL_CONTEXT_DEBUG_BIT_ARB;

        if (ctxconfig->robustness)
        {
            if (_linearwin.wgl.ARB_create_context_robustness)
            {
                if (ctxconfig->robustness == LINEARWIN_NO_RESET_NOTIFICATION)
                {
                    setAttrib(WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
                              WGL_NO_RESET_NOTIFICATION_ARB);
                }
                else if (ctxconfig->robustness == LINEARWIN_LOSE_CONTEXT_ON_RESET)
                {
                    setAttrib(WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
                              WGL_LOSE_CONTEXT_ON_RESET_ARB);
                }

                flags |= WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB;
            }
        }

        if (ctxconfig->release)
        {
            if (_linearwin.wgl.ARB_context_flush_control)
            {
                if (ctxconfig->release == LINEARWIN_RELEASE_BEHAVIOR_NONE)
                {
                    setAttrib(WGL_CONTEXT_RELEASE_BEHAVIOR_ARB,
                              WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB);
                }
                else if (ctxconfig->release == LINEARWIN_RELEASE_BEHAVIOR_FLUSH)
                {
                    setAttrib(WGL_CONTEXT_RELEASE_BEHAVIOR_ARB,
                              WGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB);
                }
            }
        }

        if (ctxconfig->noerror)
        {
            if (_linearwin.wgl.ARB_create_context_no_error)
                setAttrib(WGL_CONTEXT_OPENGL_NO_ERROR_ARB, LINEARWIN_TRUE);
        }

        // NOTE: Only request an explicitly versioned context when necessary, as
        //       explicitly requesting version 1.0 does not always return the
        //       highest version supported by the driver
        if (ctxconfig->major != 1 || ctxconfig->minor != 0)
        {
            setAttrib(WGL_CONTEXT_MAJOR_VERSION_ARB, ctxconfig->major);
            setAttrib(WGL_CONTEXT_MINOR_VERSION_ARB, ctxconfig->minor);
        }

        if (flags)
            setAttrib(WGL_CONTEXT_FLAGS_ARB, flags);

        if (mask)
            setAttrib(WGL_CONTEXT_PROFILE_MASK_ARB, mask);

        setAttrib(0, 0);

        window->context.wgl.handle =
            wglCreateContextAttribsARB(window->context.wgl.dc, share, attribs);
        if (!window->context.wgl.handle)
        {
            const DWORD error = GetLastError();

            if (error == (0xc0070000 | ERROR_INVALID_VERSION_ARB))
            {
                if (ctxconfig->client == LINEARWIN_OPENGL_API)
                {
                    _linearwinInputError(LINEARWIN_VERSION_UNAVAILABLE,
                                    "WGL: Driver does not support OpenGL version %i.%i",
                                    ctxconfig->major,
                                    ctxconfig->minor);
                }
                else
                {
                    _linearwinInputError(LINEARWIN_VERSION_UNAVAILABLE,
                                    "WGL: Driver does not support OpenGL ES version %i.%i",
                                    ctxconfig->major,
                                    ctxconfig->minor);
                }
            }
            else if (error == (0xc0070000 | ERROR_INVALID_PROFILE_ARB))
            {
                _linearwinInputError(LINEARWIN_VERSION_UNAVAILABLE,
                                "WGL: Driver does not support the requested OpenGL profile");
            }
            else if (error == (0xc0070000 | ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB))
            {
                _linearwinInputError(LINEARWIN_INVALID_VALUE,
                                "WGL: The share context is not compatible with the requested context");
            }
            else
            {
                if (ctxconfig->client == LINEARWIN_OPENGL_API)
                {
                    _linearwinInputError(LINEARWIN_VERSION_UNAVAILABLE,
                                    "WGL: Failed to create OpenGL context");
                }
                else
                {
                    _linearwinInputError(LINEARWIN_VERSION_UNAVAILABLE,
                                    "WGL: Failed to create OpenGL ES context");
                }
            }

            return LINEARWIN_FALSE;
        }
    }
    else
    {
        window->context.wgl.handle = wglCreateContext(window->context.wgl.dc);
        if (!window->context.wgl.handle)
        {
            _linearwinInputErrorWin32(LINEARWIN_VERSION_UNAVAILABLE,
                                 "WGL: Failed to create OpenGL context");
            return LINEARWIN_FALSE;
        }

        if (share)
        {
            if (!wglShareLists(share, window->context.wgl.handle))
            {
                _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                                     "WGL: Failed to enable sharing with specified OpenGL context");
                return LINEARWIN_FALSE;
            }
        }
    }

    window->context.makeCurrent = makeContextCurrentWGL;
    window->context.swapBuffers = swapBuffersWGL;
    window->context.swapInterval = swapIntervalWGL;
    window->context.extensionSupported = extensionSupportedWGL;
    window->context.getProcAddress = getProcAddressWGL;
    window->context.destroy = destroyContextWGL;

    return LINEARWIN_TRUE;
}

#undef setAttrib


//////////////////////////////////////////////////////////////////////////
//////                        LINEARWIN native API                       //////
//////////////////////////////////////////////////////////////////////////

LINEARWINAPI HGLRC linearwinGetWGLContext(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (window->context.source != LINEARWIN_NATIVE_CONTEXT_API)
    {
        _linearwinInputError(LINEARWIN_NO_WINDOW_CONTEXT, NULL);
        return NULL;
    }

    return window->context.wgl.handle;
}

