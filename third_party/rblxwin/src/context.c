//========================================================================
// RBLXWIN 3.3 - www.rblxwin.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2016 Camilla Löwy <elmindreda@rblxwin.org>
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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Checks whether the desired context attributes are valid
//
// This function checks things like whether the specified client API version
// exists and whether all relevant options have supported and non-conflicting
// values
//
RBLXWINbool _rblxwinIsValidContextConfig(const _RBLXWINctxconfig* ctxconfig)
{
    if (ctxconfig->share)
    {
        if (ctxconfig->client == RBLXWIN_NO_API ||
            ctxconfig->share->context.client == RBLXWIN_NO_API)
        {
            _rblxwinInputError(RBLXWIN_NO_WINDOW_CONTEXT, NULL);
            return RBLXWIN_FALSE;
        }
    }

    if (ctxconfig->source != RBLXWIN_NATIVE_CONTEXT_API &&
        ctxconfig->source != RBLXWIN_EGL_CONTEXT_API &&
        ctxconfig->source != RBLXWIN_OSMESA_CONTEXT_API)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM,
                        "Invalid context creation API 0x%08X",
                        ctxconfig->source);
        return RBLXWIN_FALSE;
    }

    if (ctxconfig->client != RBLXWIN_NO_API &&
        ctxconfig->client != RBLXWIN_OPENGL_API &&
        ctxconfig->client != RBLXWIN_OPENGL_ES_API)
    {
        _rblxwinInputError(RBLXWIN_INVALID_ENUM,
                        "Invalid client API 0x%08X",
                        ctxconfig->client);
        return RBLXWIN_FALSE;
    }

    if (ctxconfig->client == RBLXWIN_OPENGL_API)
    {
        if ((ctxconfig->major < 1 || ctxconfig->minor < 0) ||
            (ctxconfig->major == 1 && ctxconfig->minor > 5) ||
            (ctxconfig->major == 2 && ctxconfig->minor > 1) ||
            (ctxconfig->major == 3 && ctxconfig->minor > 3))
        {
            // OpenGL 1.0 is the smallest valid version
            // OpenGL 1.x series ended with version 1.5
            // OpenGL 2.x series ended with version 2.1
            // OpenGL 3.x series ended with version 3.3
            // For now, let everything else through

            _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                            "Invalid OpenGL version %i.%i",
                            ctxconfig->major, ctxconfig->minor);
            return RBLXWIN_FALSE;
        }

        if (ctxconfig->profile)
        {
            if (ctxconfig->profile != RBLXWIN_OPENGL_CORE_PROFILE &&
                ctxconfig->profile != RBLXWIN_OPENGL_COMPAT_PROFILE)
            {
                _rblxwinInputError(RBLXWIN_INVALID_ENUM,
                                "Invalid OpenGL profile 0x%08X",
                                ctxconfig->profile);
                return RBLXWIN_FALSE;
            }

            if (ctxconfig->major <= 2 ||
                (ctxconfig->major == 3 && ctxconfig->minor < 2))
            {
                // Desktop OpenGL context profiles are only defined for version 3.2
                // and above

                _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                                "Context profiles are only defined for OpenGL version 3.2 and above");
                return RBLXWIN_FALSE;
            }
        }

        if (ctxconfig->forward && ctxconfig->major <= 2)
        {
            // Forward-compatible contexts are only defined for OpenGL version 3.0 and above
            _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                            "Forward-compatibility is only defined for OpenGL version 3.0 and above");
            return RBLXWIN_FALSE;
        }
    }
    else if (ctxconfig->client == RBLXWIN_OPENGL_ES_API)
    {
        if (ctxconfig->major < 1 || ctxconfig->minor < 0 ||
            (ctxconfig->major == 1 && ctxconfig->minor > 1) ||
            (ctxconfig->major == 2 && ctxconfig->minor > 0))
        {
            // OpenGL ES 1.0 is the smallest valid version
            // OpenGL ES 1.x series ended with version 1.1
            // OpenGL ES 2.x series ended with version 2.0
            // For now, let everything else through

            _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                            "Invalid OpenGL ES version %i.%i",
                            ctxconfig->major, ctxconfig->minor);
            return RBLXWIN_FALSE;
        }
    }

    if (ctxconfig->robustness)
    {
        if (ctxconfig->robustness != RBLXWIN_NO_RESET_NOTIFICATION &&
            ctxconfig->robustness != RBLXWIN_LOSE_CONTEXT_ON_RESET)
        {
            _rblxwinInputError(RBLXWIN_INVALID_ENUM,
                            "Invalid context robustness mode 0x%08X",
                            ctxconfig->robustness);
            return RBLXWIN_FALSE;
        }
    }

    if (ctxconfig->release)
    {
        if (ctxconfig->release != RBLXWIN_RELEASE_BEHAVIOR_NONE &&
            ctxconfig->release != RBLXWIN_RELEASE_BEHAVIOR_FLUSH)
        {
            _rblxwinInputError(RBLXWIN_INVALID_ENUM,
                            "Invalid context release behavior 0x%08X",
                            ctxconfig->release);
            return RBLXWIN_FALSE;
        }
    }

    return RBLXWIN_TRUE;
}

// Chooses the framebuffer config that best matches the desired one
//
const _RBLXWINfbconfig* _rblxwinChooseFBConfig(const _RBLXWINfbconfig* desired,
                                         const _RBLXWINfbconfig* alternatives,
                                         unsigned int count)
{
    unsigned int i;
    unsigned int missing, leastMissing = UINT_MAX;
    unsigned int colorDiff, leastColorDiff = UINT_MAX;
    unsigned int extraDiff, leastExtraDiff = UINT_MAX;
    const _RBLXWINfbconfig* current;
    const _RBLXWINfbconfig* closest = NULL;

    for (i = 0;  i < count;  i++)
    {
        current = alternatives + i;

        if (desired->stereo > 0 && current->stereo == 0)
        {
            // Stereo is a hard constraint
            continue;
        }

        // Count number of missing buffers
        {
            missing = 0;

            if (desired->alphaBits > 0 && current->alphaBits == 0)
                missing++;

            if (desired->depthBits > 0 && current->depthBits == 0)
                missing++;

            if (desired->stencilBits > 0 && current->stencilBits == 0)
                missing++;

            if (desired->auxBuffers > 0 &&
                current->auxBuffers < desired->auxBuffers)
            {
                missing += desired->auxBuffers - current->auxBuffers;
            }

            if (desired->samples > 0 && current->samples == 0)
            {
                // Technically, several multisampling buffers could be
                // involved, but that's a lower level implementation detail and
                // not important to us here, so we count them as one
                missing++;
            }

            if (desired->transparent != current->transparent)
                missing++;
        }

        // These polynomials make many small channel size differences matter
        // less than one large channel size difference

        // Calculate color channel size difference value
        {
            colorDiff = 0;

            if (desired->redBits != RBLXWIN_DONT_CARE)
            {
                colorDiff += (desired->redBits - current->redBits) *
                             (desired->redBits - current->redBits);
            }

            if (desired->greenBits != RBLXWIN_DONT_CARE)
            {
                colorDiff += (desired->greenBits - current->greenBits) *
                             (desired->greenBits - current->greenBits);
            }

            if (desired->blueBits != RBLXWIN_DONT_CARE)
            {
                colorDiff += (desired->blueBits - current->blueBits) *
                             (desired->blueBits - current->blueBits);
            }
        }

        // Calculate non-color channel size difference value
        {
            extraDiff = 0;

            if (desired->alphaBits != RBLXWIN_DONT_CARE)
            {
                extraDiff += (desired->alphaBits - current->alphaBits) *
                             (desired->alphaBits - current->alphaBits);
            }

            if (desired->depthBits != RBLXWIN_DONT_CARE)
            {
                extraDiff += (desired->depthBits - current->depthBits) *
                             (desired->depthBits - current->depthBits);
            }

            if (desired->stencilBits != RBLXWIN_DONT_CARE)
            {
                extraDiff += (desired->stencilBits - current->stencilBits) *
                             (desired->stencilBits - current->stencilBits);
            }

            if (desired->accumRedBits != RBLXWIN_DONT_CARE)
            {
                extraDiff += (desired->accumRedBits - current->accumRedBits) *
                             (desired->accumRedBits - current->accumRedBits);
            }

            if (desired->accumGreenBits != RBLXWIN_DONT_CARE)
            {
                extraDiff += (desired->accumGreenBits - current->accumGreenBits) *
                             (desired->accumGreenBits - current->accumGreenBits);
            }

            if (desired->accumBlueBits != RBLXWIN_DONT_CARE)
            {
                extraDiff += (desired->accumBlueBits - current->accumBlueBits) *
                             (desired->accumBlueBits - current->accumBlueBits);
            }

            if (desired->accumAlphaBits != RBLXWIN_DONT_CARE)
            {
                extraDiff += (desired->accumAlphaBits - current->accumAlphaBits) *
                             (desired->accumAlphaBits - current->accumAlphaBits);
            }

            if (desired->samples != RBLXWIN_DONT_CARE)
            {
                extraDiff += (desired->samples - current->samples) *
                             (desired->samples - current->samples);
            }

            if (desired->sRGB && !current->sRGB)
                extraDiff++;
        }

        // Figure out if the current one is better than the best one found so far
        // Least number of missing buffers is the most important heuristic,
        // then color buffer size match and lastly size match for other buffers

        if (missing < leastMissing)
            closest = current;
        else if (missing == leastMissing)
        {
            if ((colorDiff < leastColorDiff) ||
                (colorDiff == leastColorDiff && extraDiff < leastExtraDiff))
            {
                closest = current;
            }
        }

        if (current == closest)
        {
            leastMissing = missing;
            leastColorDiff = colorDiff;
            leastExtraDiff = extraDiff;
        }
    }

    return closest;
}

// Retrieves the attributes of the current context
//
RBLXWINbool _rblxwinRefreshContextAttribs(_RBLXWINwindow* window,
                                    const _RBLXWINctxconfig* ctxconfig)
{
    int i;
    _RBLXWINwindow* previous;
    const char* version;
    const char* prefixes[] =
    {
        "OpenGL ES-CM ",
        "OpenGL ES-CL ",
        "OpenGL ES ",
        NULL
    };

    window->context.source = ctxconfig->source;
    window->context.client = RBLXWIN_OPENGL_API;

    previous = _rblxwinPlatformGetTls(&_rblxwin.contextSlot);
    rblxwinMakeContextCurrent((RBLXWINwindow*) window);

    window->context.GetIntegerv = (PFNGLGETINTEGERVPROC)
        window->context.getProcAddress("glGetIntegerv");
    window->context.GetString = (PFNGLGETSTRINGPROC)
        window->context.getProcAddress("glGetString");
    if (!window->context.GetIntegerv || !window->context.GetString)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR, "Entry point retrieval is broken");
        rblxwinMakeContextCurrent((RBLXWINwindow*) previous);
        return RBLXWIN_FALSE;
    }

    version = (const char*) window->context.GetString(GL_VERSION);
    if (!version)
    {
        if (ctxconfig->client == RBLXWIN_OPENGL_API)
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "OpenGL version string retrieval is broken");
        }
        else
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "OpenGL ES version string retrieval is broken");
        }

        rblxwinMakeContextCurrent((RBLXWINwindow*) previous);
        return RBLXWIN_FALSE;
    }

    for (i = 0;  prefixes[i];  i++)
    {
        const size_t length = strlen(prefixes[i]);

        if (strncmp(version, prefixes[i], length) == 0)
        {
            version += length;
            window->context.client = RBLXWIN_OPENGL_ES_API;
            break;
        }
    }

    if (!sscanf(version, "%d.%d.%d",
                &window->context.major,
                &window->context.minor,
                &window->context.revision))
    {
        if (window->context.client == RBLXWIN_OPENGL_API)
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "No version found in OpenGL version string");
        }
        else
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "No version found in OpenGL ES version string");
        }

        rblxwinMakeContextCurrent((RBLXWINwindow*) previous);
        return RBLXWIN_FALSE;
    }

    if (window->context.major < ctxconfig->major ||
        (window->context.major == ctxconfig->major &&
         window->context.minor < ctxconfig->minor))
    {
        // The desired OpenGL version is greater than the actual version
        // This only happens if the machine lacks {GLX|WGL}_ARB_create_context
        // /and/ the user has requested an OpenGL version greater than 1.0

        // For API consistency, we emulate the behavior of the
        // {GLX|WGL}_ARB_create_context extension and fail here

        if (window->context.client == RBLXWIN_OPENGL_API)
        {
            _rblxwinInputError(RBLXWIN_VERSION_UNAVAILABLE,
                            "Requested OpenGL version %i.%i, got version %i.%i",
                            ctxconfig->major, ctxconfig->minor,
                            window->context.major, window->context.minor);
        }
        else
        {
            _rblxwinInputError(RBLXWIN_VERSION_UNAVAILABLE,
                            "Requested OpenGL ES version %i.%i, got version %i.%i",
                            ctxconfig->major, ctxconfig->minor,
                            window->context.major, window->context.minor);
        }

        rblxwinMakeContextCurrent((RBLXWINwindow*) previous);
        return RBLXWIN_FALSE;
    }

    if (window->context.major >= 3)
    {
        // OpenGL 3.0+ uses a different function for extension string retrieval
        // We cache it here instead of in rblxwinExtensionSupported mostly to alert
        // users as early as possible that their build may be broken

        window->context.GetStringi = (PFNGLGETSTRINGIPROC)
            window->context.getProcAddress("glGetStringi");
        if (!window->context.GetStringi)
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "Entry point retrieval is broken");
            rblxwinMakeContextCurrent((RBLXWINwindow*) previous);
            return RBLXWIN_FALSE;
        }
    }

    if (window->context.client == RBLXWIN_OPENGL_API)
    {
        // Read back context flags (OpenGL 3.0 and above)
        if (window->context.major >= 3)
        {
            GLint flags;
            window->context.GetIntegerv(GL_CONTEXT_FLAGS, &flags);

            if (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
                window->context.forward = RBLXWIN_TRUE;

            if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
                window->context.debug = RBLXWIN_TRUE;
            else if (rblxwinExtensionSupported("GL_ARB_debug_output") &&
                     ctxconfig->debug)
            {
                // HACK: This is a workaround for older drivers (pre KHR_debug)
                //       not setting the debug bit in the context flags for
                //       debug contexts
                window->context.debug = RBLXWIN_TRUE;
            }

            if (flags & GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR)
                window->context.noerror = RBLXWIN_TRUE;
        }

        // Read back OpenGL context profile (OpenGL 3.2 and above)
        if (window->context.major >= 4 ||
            (window->context.major == 3 && window->context.minor >= 2))
        {
            GLint mask;
            window->context.GetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);

            if (mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
                window->context.profile = RBLXWIN_OPENGL_COMPAT_PROFILE;
            else if (mask & GL_CONTEXT_CORE_PROFILE_BIT)
                window->context.profile = RBLXWIN_OPENGL_CORE_PROFILE;
            else if (rblxwinExtensionSupported("GL_ARB_compatibility"))
            {
                // HACK: This is a workaround for the compatibility profile bit
                //       not being set in the context flags if an OpenGL 3.2+
                //       context was created without having requested a specific
                //       version
                window->context.profile = RBLXWIN_OPENGL_COMPAT_PROFILE;
            }
        }

        // Read back robustness strategy
        if (rblxwinExtensionSupported("GL_ARB_robustness"))
        {
            // NOTE: We avoid using the context flags for detection, as they are
            //       only present from 3.0 while the extension applies from 1.1

            GLint strategy;
            window->context.GetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_ARB,
                                        &strategy);

            if (strategy == GL_LOSE_CONTEXT_ON_RESET_ARB)
                window->context.robustness = RBLXWIN_LOSE_CONTEXT_ON_RESET;
            else if (strategy == GL_NO_RESET_NOTIFICATION_ARB)
                window->context.robustness = RBLXWIN_NO_RESET_NOTIFICATION;
        }
    }
    else
    {
        // Read back robustness strategy
        if (rblxwinExtensionSupported("GL_EXT_robustness"))
        {
            // NOTE: The values of these constants match those of the OpenGL ARB
            //       one, so we can reuse them here

            GLint strategy;
            window->context.GetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_ARB,
                                        &strategy);

            if (strategy == GL_LOSE_CONTEXT_ON_RESET_ARB)
                window->context.robustness = RBLXWIN_LOSE_CONTEXT_ON_RESET;
            else if (strategy == GL_NO_RESET_NOTIFICATION_ARB)
                window->context.robustness = RBLXWIN_NO_RESET_NOTIFICATION;
        }
    }

    if (rblxwinExtensionSupported("GL_KHR_context_flush_control"))
    {
        GLint behavior;
        window->context.GetIntegerv(GL_CONTEXT_RELEASE_BEHAVIOR, &behavior);

        if (behavior == GL_NONE)
            window->context.release = RBLXWIN_RELEASE_BEHAVIOR_NONE;
        else if (behavior == GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH)
            window->context.release = RBLXWIN_RELEASE_BEHAVIOR_FLUSH;
    }

    // Clearing the front buffer to black to avoid garbage pixels left over from
    // previous uses of our bit of VRAM
    {
        PFNGLCLEARPROC glClear = (PFNGLCLEARPROC)
            window->context.getProcAddress("glClear");
        glClear(GL_COLOR_BUFFER_BIT);

        if (window->doublebuffer)
            window->context.swapBuffers(window);
    }

    rblxwinMakeContextCurrent((RBLXWINwindow*) previous);
    return RBLXWIN_TRUE;
}

// Searches an extension string for the specified extension
//
RBLXWINbool _rblxwinStringInExtensionString(const char* string, const char* extensions)
{
    const char* start = extensions;

    for (;;)
    {
        const char* where;
        const char* terminator;

        where = strstr(start, string);
        if (!where)
            return RBLXWIN_FALSE;

        terminator = where + strlen(string);
        if (where == start || *(where - 1) == ' ')
        {
            if (*terminator == ' ' || *terminator == '\0')
                break;
        }

        start = terminator;
    }

    return RBLXWIN_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                        RBLXWIN public API                       //////
//////////////////////////////////////////////////////////////////////////

RBLXWINAPI void rblxwinMakeContextCurrent(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    _RBLXWINwindow* previous;

    _RBLXWIN_REQUIRE_INIT();

    previous = _rblxwinPlatformGetTls(&_rblxwin.contextSlot);

    if (window && window->context.client == RBLXWIN_NO_API)
    {
        _rblxwinInputError(RBLXWIN_NO_WINDOW_CONTEXT,
                        "Cannot make current with a window that has no OpenGL or OpenGL ES context");
        return;
    }

    if (previous)
    {
        if (!window || window->context.source != previous->context.source)
            previous->context.makeCurrent(NULL);
    }

    if (window)
        window->context.makeCurrent(window);
}

RBLXWINAPI RBLXWINwindow* rblxwinGetCurrentContext(void)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);
    return _rblxwinPlatformGetTls(&_rblxwin.contextSlot);
}

RBLXWINAPI void rblxwinSwapBuffers(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(window != NULL);

    _RBLXWIN_REQUIRE_INIT();

    if (window->context.client == RBLXWIN_NO_API)
    {
        _rblxwinInputError(RBLXWIN_NO_WINDOW_CONTEXT,
                        "Cannot swap buffers of a window that has no OpenGL or OpenGL ES context");
        return;
    }

    window->context.swapBuffers(window);
}

RBLXWINAPI void rblxwinSwapInterval(int interval)
{
    _RBLXWINwindow* window;

    _RBLXWIN_REQUIRE_INIT();

    window = _rblxwinPlatformGetTls(&_rblxwin.contextSlot);
    if (!window)
    {
        _rblxwinInputError(RBLXWIN_NO_CURRENT_CONTEXT,
                        "Cannot set swap interval without a current OpenGL or OpenGL ES context");
        return;
    }

    window->context.swapInterval(interval);
}

RBLXWINAPI int rblxwinExtensionSupported(const char* extension)
{
    _RBLXWINwindow* window;
    assert(extension != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(RBLXWIN_FALSE);

    window = _rblxwinPlatformGetTls(&_rblxwin.contextSlot);
    if (!window)
    {
        _rblxwinInputError(RBLXWIN_NO_CURRENT_CONTEXT,
                        "Cannot query extension without a current OpenGL or OpenGL ES context");
        return RBLXWIN_FALSE;
    }

    if (*extension == '\0')
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE, "Extension name cannot be an empty string");
        return RBLXWIN_FALSE;
    }

    if (window->context.major >= 3)
    {
        int i;
        GLint count;

        // Check if extension is in the modern OpenGL extensions string list

        window->context.GetIntegerv(GL_NUM_EXTENSIONS, &count);

        for (i = 0;  i < count;  i++)
        {
            const char* en = (const char*)
                window->context.GetStringi(GL_EXTENSIONS, i);
            if (!en)
            {
                _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                                "Extension string retrieval is broken");
                return RBLXWIN_FALSE;
            }

            if (strcmp(en, extension) == 0)
                return RBLXWIN_TRUE;
        }
    }
    else
    {
        // Check if extension is in the old style OpenGL extensions string

        const char* extensions = (const char*)
            window->context.GetString(GL_EXTENSIONS);
        if (!extensions)
        {
            _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                            "Extension string retrieval is broken");
            return RBLXWIN_FALSE;
        }

        if (_rblxwinStringInExtensionString(extension, extensions))
            return RBLXWIN_TRUE;
    }

    // Check if extension is in the platform-specific string
    return window->context.extensionSupported(extension);
}

RBLXWINAPI RBLXWINglproc rblxwinGetProcAddress(const char* procname)
{
    _RBLXWINwindow* window;
    assert(procname != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    window = _rblxwinPlatformGetTls(&_rblxwin.contextSlot);
    if (!window)
    {
        _rblxwinInputError(RBLXWIN_NO_CURRENT_CONTEXT,
                        "Cannot query entry point without a current OpenGL or OpenGL ES context");
        return NULL;
    }

    return window->context.getProcAddress(procname);
}

