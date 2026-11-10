//========================================================================
// RBLXWIN 3.3 OSMesa - www.rblxwin.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2017 Camilla Löwy <elmindreda@rblxwin.org>
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


static void makeContextCurrentOSMesa(_RBLXWINwindow* window)
{
    if (window)
    {
        int width, height;
        _rblxwinPlatformGetFramebufferSize(window, &width, &height);

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
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "OSMesa: Failed to make context current");
            return;
        }
    }

    _rblxwinPlatformSetTls(&_rblxwin.contextSlot, window);
}

static RBLXWINglproc getProcAddressOSMesa(const char* procname)
{
    return (RBLXWINglproc) OSMesaGetProcAddress(procname);
}

static void destroyContextOSMesa(_RBLXWINwindow* window)
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

static void swapBuffersOSMesa(_RBLXWINwindow* window)
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
    return RBLXWIN_FALSE;
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

RBLXWINbool _rblxwinInitOSMesa(void)
{
    int i;
    const char* sonames[] =
    {
#if defined(_RBLXWIN_OSMESA_LIBRARY)
        _RBLXWIN_OSMESA_LIBRARY,
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

    if (_rblxwin.osmesa.handle)
        return RBLXWIN_TRUE;

    for (i = 0;  sonames[i];  i++)
    {
        _rblxwin.osmesa.handle = _rblxwin_dlopen(sonames[i]);
        if (_rblxwin.osmesa.handle)
            break;
    }

    if (!_rblxwin.osmesa.handle)
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE, "OSMesa: Library not found");
        return RBLXWIN_FALSE;
    }

    _rblxwin.osmesa.CreateContextExt = (PFN_OSMesaCreateContextExt)
        _rblxwin_dlsym(_rblxwin.osmesa.handle, "OSMesaCreateContextExt");
    _rblxwin.osmesa.CreateContextAttribs = (PFN_OSMesaCreateContextAttribs)
        _rblxwin_dlsym(_rblxwin.osmesa.handle, "OSMesaCreateContextAttribs");
    _rblxwin.osmesa.DestroyContext = (PFN_OSMesaDestroyContext)
        _rblxwin_dlsym(_rblxwin.osmesa.handle, "OSMesaDestroyContext");
    _rblxwin.osmesa.MakeCurrent = (PFN_OSMesaMakeCurrent)
        _rblxwin_dlsym(_rblxwin.osmesa.handle, "OSMesaMakeCurrent");
    _rblxwin.osmesa.GetColorBuffer = (PFN_OSMesaGetColorBuffer)
        _rblxwin_dlsym(_rblxwin.osmesa.handle, "OSMesaGetColorBuffer");
    _rblxwin.osmesa.GetDepthBuffer = (PFN_OSMesaGetDepthBuffer)
        _rblxwin_dlsym(_rblxwin.osmesa.handle, "OSMesaGetDepthBuffer");
    _rblxwin.osmesa.GetProcAddress = (PFN_OSMesaGetProcAddress)
        _rblxwin_dlsym(_rblxwin.osmesa.handle, "OSMesaGetProcAddress");

    if (!_rblxwin.osmesa.CreateContextExt ||
        !_rblxwin.osmesa.DestroyContext ||
        !_rblxwin.osmesa.MakeCurrent ||
        !_rblxwin.osmesa.GetColorBuffer ||
        !_rblxwin.osmesa.GetDepthBuffer ||
        !_rblxwin.osmesa.GetProcAddress)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "OSMesa: Failed to load required entry points");

        _rblxwinTerminateOSMesa();
        return RBLXWIN_FALSE;
    }

    return RBLXWIN_TRUE;
}

void _rblxwinTerminateOSMesa(void)
{
    if (_rblxwin.osmesa.handle)
    {
        _rblxwin_dlclose(_rblxwin.osmesa.handle);
        _rblxwin.osmesa.handle = NULL;
    }
}

#define setAttrib(a, v) \
{ \
    assert(((size_t) index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
    attribs[index++] = a; \
    attribs[index++] = v; \
}

RBLXWINbool _rblxwinCreateContextOSMesa(_RBLXWINwindow* window,
                                  const _RBLXWINctxconfig* ctxconfig,
                                  const _RBLXWINfbconfig* fbconfig)
{
    OSMesaContext share = NULL;
    const int accumBits = fbconfig->accumRedBits +
                          fbconfig->accumGreenBits +
                          fbconfig->accumBlueBits +
                          fbconfig->accumAlphaBits;

    if (ctxconfig->client == RBLXWIN_OPENGL_ES_API)
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                        "OSMesa: OpenGL ES is not available on OSMesa");
        return RBLXWIN_FALSE;
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

        if (ctxconfig->profile == RBLXWIN_OPENGL_CORE_PROFILE)
        {
            setAttrib(OSMESA_PROFILE, OSMESA_CORE_PROFILE);
        }
        else if (ctxconfig->profile == RBLXWIN_OPENGL_COMPAT_PROFILE)
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
            _rblxwinInputError(RBLXWIN_VERSION_UNAVAILABLE,
                            "OSMesa: Forward-compatible contexts not supported");
            return RBLXWIN_FALSE;
        }

        setAttrib(0, 0);

        window->context.osmesa.handle =
            OSMesaCreateContextAttribs(attribs, share);
    }
    else
    {
        if (ctxconfig->profile)
        {
            _rblxwinInputError(RBLXWIN_VERSION_UNAVAILABLE,
                            "OSMesa: OpenGL profiles unavailable");
            return RBLXWIN_FALSE;
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
        _rblxwinInputError(RBLXWIN_VERSION_UNAVAILABLE,
                        "OSMesa: Failed to create context");
        return RBLXWIN_FALSE;
    }

    window->context.makeCurrent = makeContextCurrentOSMesa;
    window->context.swapBuffers = swapBuffersOSMesa;
    window->context.swapInterval = swapIntervalOSMesa;
    window->context.extensionSupported = extensionSupportedOSMesa;
    window->context.getProcAddress = getProcAddressOSMesa;
    window->context.destroy = destroyContextOSMesa;

    return RBLXWIN_TRUE;
}

#undef setAttrib


//////////////////////////////////////////////////////////////////////////
//////                        RBLXWIN native API                       //////
//////////////////////////////////////////////////////////////////////////

RBLXWINAPI int rblxwinGetOSMesaColorBuffer(RBLXWINwindow* handle, int* width,
                                     int* height, int* format, void** buffer)
{
    void* mesaBuffer;
    GLint mesaWidth, mesaHeight, mesaFormat;
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(RBLXWIN_FALSE);

    if (window->context.source != RBLXWIN_OSMESA_CONTEXT_API)
    {
        _rblxwinInputError(RBLXWIN_NO_WINDOW_CONTEXT, NULL);
        return RBLXWIN_FALSE;
    }

    if (!OSMesaGetColorBuffer(window->context.osmesa.handle,
                              &mesaWidth, &mesaHeight,
                              &mesaFormat, &mesaBuffer))
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "OSMesa: Failed to retrieve color buffer");
        return RBLXWIN_FALSE;
    }

    if (width)
        *width = mesaWidth;
    if (height)
        *height = mesaHeight;
    if (format)
        *format = mesaFormat;
    if (buffer)
        *buffer = mesaBuffer;

    return RBLXWIN_TRUE;
}

RBLXWINAPI int rblxwinGetOSMesaDepthBuffer(RBLXWINwindow* handle,
                                     int* width, int* height,
                                     int* bytesPerValue,
                                     void** buffer)
{
    void* mesaBuffer;
    GLint mesaWidth, mesaHeight, mesaBytes;
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(RBLXWIN_FALSE);

    if (window->context.source != RBLXWIN_OSMESA_CONTEXT_API)
    {
        _rblxwinInputError(RBLXWIN_NO_WINDOW_CONTEXT, NULL);
        return RBLXWIN_FALSE;
    }

    if (!OSMesaGetDepthBuffer(window->context.osmesa.handle,
                              &mesaWidth, &mesaHeight,
                              &mesaBytes, &mesaBuffer))
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "OSMesa: Failed to retrieve depth buffer");
        return RBLXWIN_FALSE;
    }

    if (width)
        *width = mesaWidth;
    if (height)
        *height = mesaHeight;
    if (bytesPerValue)
        *bytesPerValue = mesaBytes;
    if (buffer)
        *buffer = mesaBuffer;

    return RBLXWIN_TRUE;
}

RBLXWINAPI OSMesaContext rblxwinGetOSMesaContext(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (window->context.source != RBLXWIN_OSMESA_CONTEXT_API)
    {
        _rblxwinInputError(RBLXWIN_NO_WINDOW_CONTEXT, NULL);
        return NULL;
    }

    return window->context.osmesa.handle;
}

