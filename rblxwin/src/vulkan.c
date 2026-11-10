//========================================================================
// RBLXWIN 3.3 - www.rblxwin.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2018 Camilla Löwy <elmindreda@rblxwin.org>
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

#define _RBLXWIN_FIND_LOADER    1
#define _RBLXWIN_REQUIRE_LOADER 2


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

RBLXWINbool _rblxwinInitVulkan(int mode)
{
    VkResult err;
    VkExtensionProperties* ep;
    uint32_t i, count;

    if (_rblxwin.vk.available)
        return RBLXWIN_TRUE;

#if !defined(_RBLXWIN_VULKAN_STATIC)
#if defined(_RBLXWIN_VULKAN_LIBRARY)
    _rblxwin.vk.handle = _rblxwin_dlopen(_RBLXWIN_VULKAN_LIBRARY);
#elif defined(_RBLXWIN_WIN32)
    _rblxwin.vk.handle = _rblxwin_dlopen("vulkan-1.dll");
#elif defined(_RBLXWIN_COCOA)
    _rblxwin.vk.handle = _rblxwin_dlopen("libvulkan.1.dylib");
    if (!_rblxwin.vk.handle)
        _rblxwin.vk.handle = _rblxwinLoadLocalVulkanLoaderNS();
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    _rblxwin.vk.handle = _rblxwin_dlopen("libvulkan.so");
#else
    _rblxwin.vk.handle = _rblxwin_dlopen("libvulkan.so.1");
#endif
    if (!_rblxwin.vk.handle)
    {
        if (mode == _RBLXWIN_REQUIRE_LOADER)
            _rblxwinInputError(RBLXWIN_API_UNAVAILABLE, "Vulkan: Loader not found");

        return RBLXWIN_FALSE;
    }

    _rblxwin.vk.GetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)
        _rblxwin_dlsym(_rblxwin.vk.handle, "vkGetInstanceProcAddr");
    if (!_rblxwin.vk.GetInstanceProcAddr)
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                        "Vulkan: Loader does not export vkGetInstanceProcAddr");

        _rblxwinTerminateVulkan();
        return RBLXWIN_FALSE;
    }

    _rblxwin.vk.EnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)
        vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceExtensionProperties");
    if (!_rblxwin.vk.EnumerateInstanceExtensionProperties)
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                        "Vulkan: Failed to retrieve vkEnumerateInstanceExtensionProperties");

        _rblxwinTerminateVulkan();
        return RBLXWIN_FALSE;
    }
#endif // _RBLXWIN_VULKAN_STATIC

    err = vkEnumerateInstanceExtensionProperties(NULL, &count, NULL);
    if (err)
    {
        // NOTE: This happens on systems with a loader but without any Vulkan ICD
        if (mode == _RBLXWIN_REQUIRE_LOADER)
        {
            _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                            "Vulkan: Failed to query instance extension count: %s",
                            _rblxwinGetVulkanResultString(err));
        }

        _rblxwinTerminateVulkan();
        return RBLXWIN_FALSE;
    }

    ep = calloc(count, sizeof(VkExtensionProperties));

    err = vkEnumerateInstanceExtensionProperties(NULL, &count, ep);
    if (err)
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                        "Vulkan: Failed to query instance extensions: %s",
                        _rblxwinGetVulkanResultString(err));

        free(ep);
        _rblxwinTerminateVulkan();
        return RBLXWIN_FALSE;
    }

    for (i = 0;  i < count;  i++)
    {
        if (strcmp(ep[i].extensionName, "VK_KHR_surface") == 0)
            _rblxwin.vk.KHR_surface = RBLXWIN_TRUE;
#if defined(_RBLXWIN_WIN32)
        else if (strcmp(ep[i].extensionName, "VK_KHR_win32_surface") == 0)
            _rblxwin.vk.KHR_win32_surface = RBLXWIN_TRUE;
#elif defined(_RBLXWIN_COCOA)
        else if (strcmp(ep[i].extensionName, "VK_MVK_macos_surface") == 0)
            _rblxwin.vk.MVK_macos_surface = RBLXWIN_TRUE;
        else if (strcmp(ep[i].extensionName, "VK_EXT_metal_surface") == 0)
            _rblxwin.vk.EXT_metal_surface = RBLXWIN_TRUE;
#elif defined(_RBLXWIN_X11)
        else if (strcmp(ep[i].extensionName, "VK_KHR_xlib_surface") == 0)
            _rblxwin.vk.KHR_xlib_surface = RBLXWIN_TRUE;
        else if (strcmp(ep[i].extensionName, "VK_KHR_xcb_surface") == 0)
            _rblxwin.vk.KHR_xcb_surface = RBLXWIN_TRUE;
#elif defined(_RBLXWIN_WAYLAND)
        else if (strcmp(ep[i].extensionName, "VK_KHR_wayland_surface") == 0)
            _rblxwin.vk.KHR_wayland_surface = RBLXWIN_TRUE;
#endif
    }

    free(ep);

    _rblxwin.vk.available = RBLXWIN_TRUE;

    _rblxwinPlatformGetRequiredInstanceExtensions(_rblxwin.vk.extensions);

    return RBLXWIN_TRUE;
}

void _rblxwinTerminateVulkan(void)
{
#if !defined(_RBLXWIN_VULKAN_STATIC)
    if (_rblxwin.vk.handle)
        _rblxwin_dlclose(_rblxwin.vk.handle);
#endif
}

const char* _rblxwinGetVulkanResultString(VkResult result)
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
//////                        RBLXWIN public API                       //////
//////////////////////////////////////////////////////////////////////////

RBLXWINAPI int rblxwinVulkanSupported(void)
{
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(RBLXWIN_FALSE);
    return _rblxwinInitVulkan(_RBLXWIN_FIND_LOADER);
}

RBLXWINAPI const char** rblxwinGetRequiredInstanceExtensions(uint32_t* count)
{
    assert(count != NULL);

    *count = 0;

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_rblxwinInitVulkan(_RBLXWIN_REQUIRE_LOADER))
        return NULL;

    if (!_rblxwin.vk.extensions[0])
        return NULL;

    *count = 2;
    return (const char**) _rblxwin.vk.extensions;
}

RBLXWINAPI RBLXWINvkproc rblxwinGetInstanceProcAddress(VkInstance instance,
                                              const char* procname)
{
    RBLXWINvkproc proc;
    assert(procname != NULL);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_rblxwinInitVulkan(_RBLXWIN_REQUIRE_LOADER))
        return NULL;

    proc = (RBLXWINvkproc) vkGetInstanceProcAddr(instance, procname);
#if defined(_RBLXWIN_VULKAN_STATIC)
    if (!proc)
    {
        if (strcmp(procname, "vkGetInstanceProcAddr") == 0)
            return (RBLXWINvkproc) vkGetInstanceProcAddr;
    }
#else
    if (!proc)
        proc = (RBLXWINvkproc) _rblxwin_dlsym(_rblxwin.vk.handle, procname);
#endif

    return proc;
}

RBLXWINAPI int rblxwinGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                     VkPhysicalDevice device,
                                                     uint32_t queuefamily)
{
    assert(instance != VK_NULL_HANDLE);
    assert(device != VK_NULL_HANDLE);

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(RBLXWIN_FALSE);

    if (!_rblxwinInitVulkan(_RBLXWIN_REQUIRE_LOADER))
        return RBLXWIN_FALSE;

    if (!_rblxwin.vk.extensions[0])
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                        "Vulkan: Window surface creation extensions not found");
        return RBLXWIN_FALSE;
    }

    return _rblxwinPlatformGetPhysicalDevicePresentationSupport(instance,
                                                             device,
                                                             queuefamily);
}

RBLXWINAPI VkResult rblxwinCreateWindowSurface(VkInstance instance,
                                         RBLXWINwindow* handle,
                                         const VkAllocationCallbacks* allocator,
                                         VkSurfaceKHR* surface)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    assert(instance != VK_NULL_HANDLE);
    assert(window != NULL);
    assert(surface != NULL);

    *surface = VK_NULL_HANDLE;

    _RBLXWIN_REQUIRE_INIT_OR_RETURN(VK_ERROR_INITIALIZATION_FAILED);

    if (!_rblxwinInitVulkan(_RBLXWIN_REQUIRE_LOADER))
        return VK_ERROR_INITIALIZATION_FAILED;

    if (!_rblxwin.vk.extensions[0])
    {
        _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                        "Vulkan: Window surface creation extensions not found");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    if (window->context.client != RBLXWIN_NO_API)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE,
                        "Vulkan: Window surface creation requires the window to have the client API set to RBLXWIN_NO_API");
        return VK_ERROR_NATIVE_WINDOW_IN_USE_KHR;
    }

    return _rblxwinPlatformCreateWindowSurface(instance, window, allocator, surface);
}

