//========================================================================
// LINEARWIN 3.3 - www.linearwin.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2018 Camilla Löwy <elmindreda@linearwin.org>
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
#include <string.h>
#include <stdlib.h>

#define _LINEARWIN_FIND_LOADER    1
#define _LINEARWIN_REQUIRE_LOADER 2


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

LINEARWINbool _linearwinInitVulkan(int mode)
{
    VkResult err;
    VkExtensionProperties* ep;
    uint32_t i, count;

    if (_linearwin.vk.available)
        return LINEARWIN_TRUE;

#if !defined(_LINEARWIN_VULKAN_STATIC)
#if defined(_LINEARWIN_VULKAN_LIBRARY)
    _linearwin.vk.handle = _linearwin_dlopen(_LINEARWIN_VULKAN_LIBRARY);
#elif defined(_LINEARWIN_WIN32)
    _linearwin.vk.handle = _linearwin_dlopen("vulkan-1.dll");
#elif defined(_LINEARWIN_COCOA)
    _linearwin.vk.handle = _linearwin_dlopen("libvulkan.1.dylib");
    if (!_linearwin.vk.handle)
        _linearwin.vk.handle = _linearwinLoadLocalVulkanLoaderNS();
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    _linearwin.vk.handle = _linearwin_dlopen("libvulkan.so");
#else
    _linearwin.vk.handle = _linearwin_dlopen("libvulkan.so.1");
#endif
    if (!_linearwin.vk.handle)
    {
        if (mode == _LINEARWIN_REQUIRE_LOADER)
            _linearwinInputError(LINEARWIN_API_UNAVAILABLE, "Vulkan: Loader not found");

        return LINEARWIN_FALSE;
    }

    _linearwin.vk.GetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)
        _linearwin_dlsym(_linearwin.vk.handle, "vkGetInstanceProcAddr");
    if (!_linearwin.vk.GetInstanceProcAddr)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                        "Vulkan: Loader does not export vkGetInstanceProcAddr");

        _linearwinTerminateVulkan();
        return LINEARWIN_FALSE;
    }

    _linearwin.vk.EnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)
        vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceExtensionProperties");
    if (!_linearwin.vk.EnumerateInstanceExtensionProperties)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                        "Vulkan: Failed to retrieve vkEnumerateInstanceExtensionProperties");

        _linearwinTerminateVulkan();
        return LINEARWIN_FALSE;
    }
#endif // _LINEARWIN_VULKAN_STATIC

    err = vkEnumerateInstanceExtensionProperties(NULL, &count, NULL);
    if (err)
    {
        // NOTE: This happens on systems with a loader but without any Vulkan ICD
        if (mode == _LINEARWIN_REQUIRE_LOADER)
        {
            _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                            "Vulkan: Failed to query instance extension count: %s",
                            _linearwinGetVulkanResultString(err));
        }

        _linearwinTerminateVulkan();
        return LINEARWIN_FALSE;
    }

    ep = calloc(count, sizeof(VkExtensionProperties));

    err = vkEnumerateInstanceExtensionProperties(NULL, &count, ep);
    if (err)
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                        "Vulkan: Failed to query instance extensions: %s",
                        _linearwinGetVulkanResultString(err));

        free(ep);
        _linearwinTerminateVulkan();
        return LINEARWIN_FALSE;
    }

    for (i = 0;  i < count;  i++)
    {
        if (strcmp(ep[i].extensionName, "VK_KHR_surface") == 0)
            _linearwin.vk.KHR_surface = LINEARWIN_TRUE;
#if defined(_LINEARWIN_WIN32)
        else if (strcmp(ep[i].extensionName, "VK_KHR_win32_surface") == 0)
            _linearwin.vk.KHR_win32_surface = LINEARWIN_TRUE;
#elif defined(_LINEARWIN_COCOA)
        else if (strcmp(ep[i].extensionName, "VK_MVK_macos_surface") == 0)
            _linearwin.vk.MVK_macos_surface = LINEARWIN_TRUE;
        else if (strcmp(ep[i].extensionName, "VK_EXT_metal_surface") == 0)
            _linearwin.vk.EXT_metal_surface = LINEARWIN_TRUE;
#elif defined(_LINEARWIN_X11)
        else if (strcmp(ep[i].extensionName, "VK_KHR_xlib_surface") == 0)
            _linearwin.vk.KHR_xlib_surface = LINEARWIN_TRUE;
        else if (strcmp(ep[i].extensionName, "VK_KHR_xcb_surface") == 0)
            _linearwin.vk.KHR_xcb_surface = LINEARWIN_TRUE;
#elif defined(_LINEARWIN_WAYLAND)
        else if (strcmp(ep[i].extensionName, "VK_KHR_wayland_surface") == 0)
            _linearwin.vk.KHR_wayland_surface = LINEARWIN_TRUE;
#endif
    }

    free(ep);

    _linearwin.vk.available = LINEARWIN_TRUE;

    _linearwinPlatformGetRequiredInstanceExtensions(_linearwin.vk.extensions);

    return LINEARWIN_TRUE;
}

void _linearwinTerminateVulkan(void)
{
#if !defined(_LINEARWIN_VULKAN_STATIC)
    if (_linearwin.vk.handle)
        _linearwin_dlclose(_linearwin.vk.handle);
#endif
}

const char* _linearwinGetVulkanResultString(VkResult result)
{
    switch (result)
    {
        case VK_SUCCESS:
            return "Success";
        case VK_NOT_READY:
            return "A fence or query has not yet completed";
        case VK_TIMEOUT:
            return "A wait operation has not completed in the specified time";
        case VK_EVENT_SET:
            return "An event is signaled";
        case VK_EVENT_RESET:
            return "An event is unsignaled";
        case VK_INCOMPLETE:
            return "A return array was too small for the result";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "A host memory allocation has failed";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "A device memory allocation has failed";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "Initialization of an object could not be completed for implementation-specific reasons";
        case VK_ERROR_DEVICE_LOST:
            return "The logical or physical device has been lost";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "Mapping of a memory object has failed";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "A requested layer is not present or could not be loaded";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "A requested extension is not supported";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "A requested feature is not supported";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "Too many objects of the type have already been created";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "A requested format is not supported on this device";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "A surface is no longer available";
        case VK_SUBOPTIMAL_KHR:
            return "A swapchain no longer matches the surface properties exactly, but can still be used";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "A surface has changed in such a way that it is no longer compatible with the swapchain";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "The display used by a swapchain does not use the same presentable image layout";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "A validation layer found an error";
        default:
            return "ERROR: UNKNOWN VULKAN ERROR";
    }
}


//////////////////////////////////////////////////////////////////////////
//////                        LINEARWIN public API                       //////
//////////////////////////////////////////////////////////////////////////

LINEARWINAPI int linearwinVulkanSupported(void)
{
    _LINEARWIN_REQUIRE_INIT_OR_RETURN(LINEARWIN_FALSE);
    return _linearwinInitVulkan(_LINEARWIN_FIND_LOADER);
}

LINEARWINAPI const char** linearwinGetRequiredInstanceExtensions(uint32_t* count)
{
    assert(count != NULL);

    *count = 0;

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_linearwinInitVulkan(_LINEARWIN_REQUIRE_LOADER))
        return NULL;

    if (!_linearwin.vk.extensions[0])
        return NULL;

    *count = 2;
    return (const char**) _linearwin.vk.extensions;
}

LINEARWINAPI LINEARWINvkproc linearwinGetInstanceProcAddress(VkInstance instance,
                                              const char* procname)
{
    LINEARWINvkproc proc;
    assert(procname != NULL);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_linearwinInitVulkan(_LINEARWIN_REQUIRE_LOADER))
        return NULL;

    proc = (LINEARWINvkproc) vkGetInstanceProcAddr(instance, procname);
#if defined(_LINEARWIN_VULKAN_STATIC)
    if (!proc)
    {
        if (strcmp(procname, "vkGetInstanceProcAddr") == 0)
            return (LINEARWINvkproc) vkGetInstanceProcAddr;
    }
#else
    if (!proc)
        proc = (LINEARWINvkproc) _linearwin_dlsym(_linearwin.vk.handle, procname);
#endif

    return proc;
}

LINEARWINAPI int linearwinGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                     VkPhysicalDevice device,
                                                     uint32_t queuefamily)
{
    assert(instance != VK_NULL_HANDLE);
    assert(device != VK_NULL_HANDLE);

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(LINEARWIN_FALSE);

    if (!_linearwinInitVulkan(_LINEARWIN_REQUIRE_LOADER))
        return LINEARWIN_FALSE;

    if (!_linearwin.vk.extensions[0])
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                        "Vulkan: Window surface creation extensions not found");
        return LINEARWIN_FALSE;
    }

    return _linearwinPlatformGetPhysicalDevicePresentationSupport(instance,
                                                             device,
                                                             queuefamily);
}

LINEARWINAPI VkResult linearwinCreateWindowSurface(VkInstance instance,
                                         LINEARWINwindow* handle,
                                         const VkAllocationCallbacks* allocator,
                                         VkSurfaceKHR* surface)
{
    _LINEARWINwindow* window = (_LINEARWINwindow*) handle;
    assert(instance != VK_NULL_HANDLE);
    assert(window != NULL);
    assert(surface != NULL);

    *surface = VK_NULL_HANDLE;

    _LINEARWIN_REQUIRE_INIT_OR_RETURN(VK_ERROR_INITIALIZATION_FAILED);

    if (!_linearwinInitVulkan(_LINEARWIN_REQUIRE_LOADER))
        return VK_ERROR_INITIALIZATION_FAILED;

    if (!_linearwin.vk.extensions[0])
    {
        _linearwinInputError(LINEARWIN_API_UNAVAILABLE,
                        "Vulkan: Window surface creation extensions not found");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    if (window->context.client != LINEARWIN_NO_API)
    {
        _linearwinInputError(LINEARWIN_INVALID_VALUE,
                        "Vulkan: Window surface creation requires the window to have the client API set to LINEARWIN_NO_API");
        return VK_ERROR_NATIVE_WINDOW_IN_USE_KHR;
    }

    return _linearwinPlatformCreateWindowSurface(instance, window, allocator, surface);
}

