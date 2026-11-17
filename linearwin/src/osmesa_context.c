//========================================================================
// LINEARWIN 3.3 OSMesa - www.linearwin.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2017 Camilla Löwy <elmindreda@linearwin.org>
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "internal.h"


static void makeContextCurrentOSMesa(_LINEARWINwindow* window)
{
    if (window)
    {
        int width, height;
        _linearwinPlatformGetFramebufferSize(window, &width, &height);

        // Check to see if we need to allocate a new buffer
        if ((window->context.osmesa.buffer == NULL) ||
            (width != window->context.osmesa.width) ||
            (height != window->context.osmesa.height))
        {
            free(window->context.osmesa.buffer);

            // Allocate the new buffer (width * height * 8-bit RGBA)
            window->context.osmesa.buffer = calloc(4, (size_t) width * height);
            window->context.osmesa.width  = width;
            window->context.osmesa.height = height;
        }

        if (!OSMesaMakeCurrent(window->context.osmesa.handle,
                               window->context.osmesa.buffer,
                               GL_UNSIGNED_BYTE,
                               width, height))
        {
            _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                            "OSMesa: Failed to make context current");
            return;
        }
    }

    _linearwinPlatformSetTls(&_linearwin.contextSlot, window);
}

static LINEARWINglproc getProcAddressOSMesa(const char* procname)
{
    return (LINEARWINglproc) OSMesaGetProcAddress(procname);
}

static void destroyContextOSMesa(_LINEARWINwindow* window)
{
    if (window->context.osmesa.handle)
    {
        OSMesaDestroyContext(window->context.osmesa.handle);
        window->context.osmesa.handle = NULL;
    }

    if (window->context.osmesa.buffer)
    {
        free(window->context.osmesa.buffer);
        window->context.osmesa.width = 0;
        window->context.osmesa.height = 0;
    }
}

static void swapBuffersOSMesa(_LINEARWINwindow* window)
{
    // No double buffering on OSMesa
}

static void swapIntervalOSMesa(int interval)
{
    // No swap interval on OSMesa
}

static int extensionSupportedOSMesa(const char* extension)
{
    // OSMesa does not have extensions
    return LINEARWIN_FALSE;
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

LINEARWINbool _linearwinInitOSMesa(void)
{
    int i;
    const char* sonames[] =
    {
#if defined(_LINEARWIN_OSMESA_LIBRARY)
        _LINEARWIN_OSMESA_LIBRARY,
#elif defined(_WIN32)
        "libOSMesa.dll",
        "OSMesa.dll",
#elif defined(__APPLE__)
        "libOSMesa.8.dylib",
#elif defined(__CYGWIN__)
        "libOSMesa-8.so",
#elif defined(__OpenBSD__) || defined(__NetBSD__)
        "libOSMesa.so",
#else
        "libOSMesa.so.8",
        "libOSMesa.so.6",
#endif
        NULL
    };

    if (_linearwin.osmesa.handle)
        return LINEARWIN_TRUE;

    for (i = 0;  sonames[i];  i++)
    {
        _linearwin.osmesa.handle = _linearwin_dlopen(sonames[i]);
        if (_linearwin.osmesa.handle)
            break;
    }

    if (!_linearwin.osmesa.handle)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE, "OSMesa: Library not found");
        return LINEARWIN_FALSE;
    }

    _linearwin.osmesa.CreateContextExt = (PFN_OSMesaCreateContextExt)
        _linearwin_dlsym(_linearwin.osmesa.handle, "OSMesaCreateContextExt");
    _linearwin.osmesa.CreateContextAttribs = (PFN_OSMesaCreateContextAttribs)
        _linearwin_dlsym(_linearwin.osmesa.handle, "OSMesaCreateContextAttribs");
    _linearwin.osmesa.DestroyContext = (PFN_OSMesaDestroyContext)
        _linearwin_dlsym(_linearwin.osmesa.handle, "OSMesaDestroyContext");
    _linearwin.osmesa.MakeCurrent = (PFN_OSMesaMakeCurrent)
        _linearwin_dlsym(_linearwin.osmesa.handle, "OSMesaMakeCurrent");
    _linearwin.osmesa.GetColorBuffer = (PFN_OSMesaGetColorBuffer)
        _linearwin_dlsym(_linearwin.osmesa.handle, "OSMesaGetColorBuffer");
    _linearwin.osmesa.GetDepthBuffer = (PFN_OSMesaGetDepthBuffer)
        _linearwin_dlsym(_linearwin.osmesa.handle, "OSMesaGetDepthBuffer");
    _linearwin.osmesa.GetProcAddress = (PFN_OSMesaGetProcAddress)
        _linearwin_dlsym(_linearwin.osmesa.handle, "OSMesaGetProcAddress");

    if (!_linearwin.osmesa.CreateContextExt ||
        !_linearwin.osmesa.DestroyContext ||
        !_linearwin.osmesa.MakeCurrent ||
        !_linearwin.osmesa.GetColorBuffer ||
        !_linearwin.osmesa.GetDepthBuffer ||
        !_linearwin.osmesa.GetProcAddress)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "OSMesa: Failed to load required entry points");

        _linearwinTerminateOSMesa();
        return LINEARWIN_FALSE;
    }

    return LINEARWIN_TRUE;
}

void _linearwinTerminateOSMesa(void)
{
    if (_linearwin.osmesa.handle)
    {
        _linearwin_dlclose(_linearwin.osmesa.handle);
        _linearwin.osmesa.handle = NULL;
    }
}

#define setAttrib(a, v) \
{ \
    assert(((size_t) index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
    attribs[index++] = a; \
    attribs[index++] = v; \
}

LINEARWINbool _linearwinCreateContextOSMesa(_LINEARWINwindow* window,
                                  const _LINEARWINctxconfig* ctxconfig,
                                  const _LINEARWINfbconfig* fbconfig)
{
    OSMesaContext share = NULL;
    const int accumBits = fbconfig->accumRedBits +
                          fbconfig->accumGreenBits +
                          fbconfig->accumBlueBits +
                          fbconfig->accumAlphaBits;

    if (ctxconfig->client == LINEARWIN_OPENGL_ES_API)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                        "OSMesa: OpenGL ES is not available on OSMesa");
        return LINEARWIN_FALSE;
    }

    if (ctxconfig->share)
        share = ctxconfig->share->context.osmesa.handle;

    if (OSMesaCreateContextAttribs)
    {
        int index = 0, attribs[40];

        setAttrib(OSMESA_FORMAT, OSMESA_RGBA);
        setAttrib(OSMESA_DEPTH_BITS, fbconfig->depthBits);
        setAttrib(OSMESA_STENCIL_BITS, fbconfig->stencilBits);
        setAttrib(OSMESA_ACCUM_BITS, accumBits);

        if (ctxconfig->profile == LINEARWIN_OPENGL_CORE_PROFILE)
        {
            setAttrib(OSMESA_PROFILE, OSMESA_CORE_PROFILE);
        }
        else if (ctxconfig->profile == LINEARWIN_OPENGL_COMPAT_PROFILE)
        {
            setAttrib(OSMESA_PROFILE, OSMESA_COMPAT_PROFILE);
        }

        if (ctxconfig->major != 1 || ctxconfig->minor != 0)
        {
            setAttrib(OSMESA_CONTEXT_MAJOR_VERSION, ctxconfig->major);
            setAttrib(OSMESA_CONTEXT_MINOR_VERSION, ctxconfig->minor);
        }

        if (ctxconfig->forward)
        {
            _linearwinInputError(LINEARWIN_VERSION_UNAVAILABLE,
                            "OSMesa: Forward-compatible contexts not supported");
            return LINEARWIN_FALSE;
        }

        setAttrib(0, 0);

        window->context.osmesa.handle =
            OSMesaCreateContextAttribs(attribs, share);
    }
    else
    {
        if (ctxconfig->profile)
        {
            _linearwinInputError(LINEARWIN_VERSION_UNAVAILABLE,
                            "OSMesa: OpenGL profiles unavailable");
            return LINEARWIN_FALSE;
        }

        window->context.osmesa.handle =
            OSMesaCreateContextExt(OSMESA_RGBA,
                                   fbconfig->depthBits,
                                   fbconfig->stencilBits,
                                   accumBits,
                                   share);
    }

    if (window->context.osmesa.handle == NULL)
    {
        _linearwinInputError(LINEARWIN_VERSION_UNAVAILABLE,
                        "OSMesa: Failed to create context");
        return LINEARWIN_FALSE;
    }

    window->context.makeCurrent = makeContextCurrentOSMesa;
    window->context.swapBuffers = swapBuffersOSMesa;
    window->context.swapInterval = swapIntervalOSMesa;
    window->context.extensionSupported = extensionSupportedOSMesa;
    window->context.getProcAddress = getProcAddressOSMesa;
    window->context.destroy = destroyContextOSMesa;

    return LINEARWIN_TRUE;
}

#undef setAttrib


//////////////////////////////////////////////////////////////////////////
//////                        LINEARWIN native API                       //////
//////////////////////////////////////////////////////////////////////////

LINEARWINAPI int linearwinGetOSMesaColorBuffer(LINEARWINwindow* handle, int* width,
                                     int* height, int* format, void** buffer)
{
    void* mesaBuffer;
    GLint mesaWidth, mesaHeight, mesaFormat;
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(LINEARWIN_FALSE);

    if (window->context.source != LINEARWIN_OSMESA_CONTEXT_API)
    {
        _linearwinInputError(LINEARWIN_NO_WINDOW_CONTEXT, NULL);
        return LINEARWIN_FALSE;
    }

    if (!OSMesaGetColorBuffer(window->context.osmesa.handle,
                              &mesaWidth, &mesaHeight,
                              &mesaFormat, &mesaBuffer))
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "OSMesa: Failed to retrieve color buffer");
        return LINEARWIN_FALSE;
    }

    if (width)
        *width = mesaWidth;
    if (height)
        *height = mesaHeight;
    if (format)
        *format = mesaFormat;
    if (buffer)
        *buffer = mesaBuffer;

    return LINEARWIN_TRUE;
}

LINEARWINAPI int linearwinGetOSMesaDepthBuffer(LINEARWINwindow* handle,
                                     int* width, int* height,
                                     int* bytesPerValue,
                                     void** buffer)
{
    void* mesaBuffer;
    GLint mesaWidth, mesaHeight, mesaBytes;
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(window != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(LINEARWIN_FALSE);

    if (window->context.source != LINEARWIN_OSMESA_CONTEXT_API)
    {
        _linearwinInputError(LINEARWIN_NO_WINDOW_CONTEXT, NULL);
        return LINEARWIN_FALSE;
    }

    if (!OSMesaGetDepthBuffer(window->context.osmesa.handle,
                              &mesaWidth, &mesaHeight,
                              &mesaBytes, &mesaBuffer))
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "OSMesa: Failed to retrieve depth buffer");
        return LINEARWIN_FALSE;
    }

    if (width)
        *width = mesaWidth;
    if (height)
        *height = mesaHeight;
    if (bytesPerValue)
        *bytesPerValue = mesaBytes;
    if (buffer)
        *buffer = mesaBuffer;

    return LINEARWIN_TRUE;
}

LINEARWINAPI OSMesaContext linearwinGetOSMesaContext(LINEARWINwindow* handle)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (window->context.source != LINEARWIN_OSMESA_CONTEXT_API)
    {
        _linearwinInputError(LINEARWIN_NO_WINDOW_CONTEXT, NULL);
        return NULL;
    }

    return window->context.osmesa.handle;
}

