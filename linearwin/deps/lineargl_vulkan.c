#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lineargl/vulkan.h>

#ifndef LINEARGL_IMPL_UTIL_C_
#define LINEARGL_IMPL_UTIL_C_

#ifdef _MSC_VER
#define LINEARGL_IMPL_UTIL_SSCANF sscanf_s
#else
#define LINEARGL_IMPL_UTIL_SSCANF sscanf
#endif

#endif /* LINEARGL_IMPL_UTIL_C_ */

#ifdef __cplusplus
extern "C" {
#endif



int LINEARGL_VK_VERSION_1_0 = 0;
int LINEARGL_VK_VERSION_1_1 = 0;
int LINEARGL_VK_VERSION_1_2 = 0;
int LINEARGL_VK_VERSION_1_3 = 0;
int LINEARGL_VK_EXT_debug_report = 0;
int LINEARGL_VK_KHR_portability_enumeration = 0;
int LINEARGL_VK_KHR_surface = 0;
int LINEARGL_VK_KHR_swapchain = 0;



PFN_vkAcquireNextImage2KHR lineargl_vkAcquireNextImage2KHR = NULL;
PFN_vkAcquireNextImageKHR lineargl_vkAcquireNextImageKHR = NULL;
PFN_vkAllocateCommandBuffers lineargl_vkAllocateCommandBuffers = NULL;
PFN_vkAllocateDescriptorSets lineargl_vkAllocateDescriptorSets = NULL;
PFN_vkAllocateMemory lineargl_vkAllocateMemory = NULL;
PFN_vkBeginCommandBuffer lineargl_vkBeginCommandBuffer = NULL;
PFN_vkBindBufferMemory lineargl_vkBindBufferMemory = NULL;
PFN_vkBindBufferMemory2 lineargl_vkBindBufferMemory2 = NULL;
PFN_vkBindImageMemory lineargl_vkBindImageMemory = NULL;
PFN_vkBindImageMemory2 lineargl_vkBindImageMemory2 = NULL;
PFN_vkCmdBeginQuery lineargl_vkCmdBeginQuery = NULL;
PFN_vkCmdBeginRenderPass lineargl_vkCmdBeginRenderPass = NULL;
PFN_vkCmdBeginRenderPass2 lineargl_vkCmdBeginRenderPass2 = NULL;
PFN_vkCmdBeginRendering lineargl_vkCmdBeginRendering = NULL;
PFN_vkCmdBindDescriptorSets lineargl_vkCmdBindDescriptorSets = NULL;
PFN_vkCmdBindIndexBuffer lineargl_vkCmdBindIndexBuffer = NULL;
PFN_vkCmdBindPipeline lineargl_vkCmdBindPipeline = NULL;
PFN_vkCmdBindVertexBuffers lineargl_vkCmdBindVertexBuffers = NULL;
PFN_vkCmdBindVertexBuffers2 lineargl_vkCmdBindVertexBuffers2 = NULL;
PFN_vkCmdBlitImage lineargl_vkCmdBlitImage = NULL;
PFN_vkCmdBlitImage2 lineargl_vkCmdBlitImage2 = NULL;
PFN_vkCmdClearAttachments lineargl_vkCmdClearAttachments = NULL;
PFN_vkCmdClearColorImage lineargl_vkCmdClearColorImage = NULL;
PFN_vkCmdClearDepthStencilImage lineargl_vkCmdClearDepthStencilImage = NULL;
PFN_vkCmdCopyBuffer lineargl_vkCmdCopyBuffer = NULL;
PFN_vkCmdCopyBuffer2 lineargl_vkCmdCopyBuffer2 = NULL;
PFN_vkCmdCopyBufferToImage lineargl_vkCmdCopyBufferToImage = NULL;
PFN_vkCmdCopyBufferToImage2 lineargl_vkCmdCopyBufferToImage2 = NULL;
PFN_vkCmdCopyImage lineargl_vkCmdCopyImage = NULL;
PFN_vkCmdCopyImage2 lineargl_vkCmdCopyImage2 = NULL;
PFN_vkCmdCopyImageToBuffer lineargl_vkCmdCopyImageToBuffer = NULL;
PFN_vkCmdCopyImageToBuffer2 lineargl_vkCmdCopyImageToBuffer2 = NULL;
PFN_vkCmdCopyQueryPoolResults lineargl_vkCmdCopyQueryPoolResults = NULL;
PFN_vkCmdDispatch lineargl_vkCmdDispatch = NULL;
PFN_vkCmdDispatchBase lineargl_vkCmdDispatchBase = NULL;
PFN_vkCmdDispatchIndirect lineargl_vkCmdDispatchIndirect = NULL;
PFN_vkCmdDraw lineargl_vkCmdDraw = NULL;
PFN_vkCmdDrawIndexed lineargl_vkCmdDrawIndexed = NULL;
PFN_vkCmdDrawIndexedIndirect lineargl_vkCmdDrawIndexedIndirect = NULL;
PFN_vkCmdDrawIndexedIndirectCount lineargl_vkCmdDrawIndexedIndirectCount = NULL;
PFN_vkCmdDrawIndirect lineargl_vkCmdDrawIndirect = NULL;
PFN_vkCmdDrawIndirectCount lineargl_vkCmdDrawIndirectCount = NULL;
PFN_vkCmdEndQuery lineargl_vkCmdEndQuery = NULL;
PFN_vkCmdEndRenderPass lineargl_vkCmdEndRenderPass = NULL;
PFN_vkCmdEndRenderPass2 lineargl_vkCmdEndRenderPass2 = NULL;
PFN_vkCmdEndRendering lineargl_vkCmdEndRendering = NULL;
PFN_vkCmdExecuteCommands lineargl_vkCmdExecuteCommands = NULL;
PFN_vkCmdFillBuffer lineargl_vkCmdFillBuffer = NULL;
PFN_vkCmdNextSubpass lineargl_vkCmdNextSubpass = NULL;
PFN_vkCmdNextSubpass2 lineargl_vkCmdNextSubpass2 = NULL;
PFN_vkCmdPipelineBarrier lineargl_vkCmdPipelineBarrier = NULL;
PFN_vkCmdPipelineBarrier2 lineargl_vkCmdPipelineBarrier2 = NULL;
PFN_vkCmdPushConstants lineargl_vkCmdPushConstants = NULL;
PFN_vkCmdResetEvent lineargl_vkCmdResetEvent = NULL;
PFN_vkCmdResetEvent2 lineargl_vkCmdResetEvent2 = NULL;
PFN_vkCmdResetQueryPool lineargl_vkCmdResetQueryPool = NULL;
PFN_vkCmdResolveImage lineargl_vkCmdResolveImage = NULL;
PFN_vkCmdResolveImage2 lineargl_vkCmdResolveImage2 = NULL;
PFN_vkCmdSetBlendConstants lineargl_vkCmdSetBlendConstants = NULL;
PFN_vkCmdSetCullMode lineargl_vkCmdSetCullMode = NULL;
PFN_vkCmdSetDepthBias lineargl_vkCmdSetDepthBias = NULL;
PFN_vkCmdSetDepthBiasEnable lineargl_vkCmdSetDepthBiasEnable = NULL;
PFN_vkCmdSetDepthBounds lineargl_vkCmdSetDepthBounds = NULL;
PFN_vkCmdSetDepthBoundsTestEnable lineargl_vkCmdSetDepthBoundsTestEnable = NULL;
PFN_vkCmdSetDepthCompareOp lineargl_vkCmdSetDepthCompareOp = NULL;
PFN_vkCmdSetDepthTestEnable lineargl_vkCmdSetDepthTestEnable = NULL;
PFN_vkCmdSetDepthWriteEnable lineargl_vkCmdSetDepthWriteEnable = NULL;
PFN_vkCmdSetDeviceMask lineargl_vkCmdSetDeviceMask = NULL;
PFN_vkCmdSetEvent lineargl_vkCmdSetEvent = NULL;
PFN_vkCmdSetEvent2 lineargl_vkCmdSetEvent2 = NULL;
PFN_vkCmdSetFrontFace lineargl_vkCmdSetFrontFace = NULL;
PFN_vkCmdSetLineWidth lineargl_vkCmdSetLineWidth = NULL;
PFN_vkCmdSetPrimitiveRestartEnable lineargl_vkCmdSetPrimitiveRestartEnable = NULL;
PFN_vkCmdSetPrimitiveTopology lineargl_vkCmdSetPrimitiveTopology = NULL;
PFN_vkCmdSetRasterizerDiscardEnable lineargl_vkCmdSetRasterizerDiscardEnable = NULL;
PFN_vkCmdSetScissor lineargl_vkCmdSetScissor = NULL;
PFN_vkCmdSetScissorWithCount lineargl_vkCmdSetScissorWithCount = NULL;
PFN_vkCmdSetStencilCompareMask lineargl_vkCmdSetStencilCompareMask = NULL;
PFN_vkCmdSetStencilOp lineargl_vkCmdSetStencilOp = NULL;
PFN_vkCmdSetStencilReference lineargl_vkCmdSetStencilReference = NULL;
PFN_vkCmdSetStencilTestEnable lineargl_vkCmdSetStencilTestEnable = NULL;
PFN_vkCmdSetStencilWriteMask lineargl_vkCmdSetStencilWriteMask = NULL;
PFN_vkCmdSetViewport lineargl_vkCmdSetViewport = NULL;
PFN_vkCmdSetViewportWithCount lineargl_vkCmdSetViewportWithCount = NULL;
PFN_vkCmdUpdateBuffer lineargl_vkCmdUpdateBuffer = NULL;
PFN_vkCmdWaitEvents lineargl_vkCmdWaitEvents = NULL;
PFN_vkCmdWaitEvents2 lineargl_vkCmdWaitEvents2 = NULL;
PFN_vkCmdWriteTimestamp lineargl_vkCmdWriteTimestamp = NULL;
PFN_vkCmdWriteTimestamp2 lineargl_vkCmdWriteTimestamp2 = NULL;
PFN_vkCreateBuffer lineargl_vkCreateBuffer = NULL;
PFN_vkCreateBufferView lineargl_vkCreateBufferView = NULL;
PFN_vkCreateCommandPool lineargl_vkCreateCommandPool = NULL;
PFN_vkCreateComputePipelines lineargl_vkCreateComputePipelines = NULL;
PFN_vkCreateDebugReportCallbackEXT lineargl_vkCreateDebugReportCallbackEXT = NULL;
PFN_vkCreateDescriptorPool lineargl_vkCreateDescriptorPool = NULL;
PFN_vkCreateDescriptorSetLayout lineargl_vkCreateDescriptorSetLayout = NULL;
PFN_vkCreateDescriptorUpdateTemplate lineargl_vkCreateDescriptorUpdateTemplate = NULL;
PFN_vkCreateDevice lineargl_vkCreateDevice = NULL;
PFN_vkCreateEvent lineargl_vkCreateEvent = NULL;
PFN_vkCreateFence lineargl_vkCreateFence = NULL;
PFN_vkCreateFramebuffer lineargl_vkCreateFramebuffer = NULL;
PFN_vkCreateGraphicsPipelines lineargl_vkCreateGraphicsPipelines = NULL;
PFN_vkCreateImage lineargl_vkCreateImage = NULL;
PFN_vkCreateImageView lineargl_vkCreateImageView = NULL;
PFN_vkCreateInstance lineargl_vkCreateInstance = NULL;
PFN_vkCreatePipelineCache lineargl_vkCreatePipelineCache = NULL;
PFN_vkCreatePipelineLayout lineargl_vkCreatePipelineLayout = NULL;
PFN_vkCreatePrivateDataSlot lineargl_vkCreatePrivateDataSlot = NULL;
PFN_vkCreateQueryPool lineargl_vkCreateQueryPool = NULL;
PFN_vkCreateRenderPass lineargl_vkCreateRenderPass = NULL;
PFN_vkCreateRenderPass2 lineargl_vkCreateRenderPass2 = NULL;
PFN_vkCreateSampler lineargl_vkCreateSampler = NULL;
PFN_vkCreateSamplerYcbcrConversion lineargl_vkCreateSamplerYcbcrConversion = NULL;
PFN_vkCreateSemaphore lineargl_vkCreateSemaphore = NULL;
PFN_vkCreateShaderModule lineargl_vkCreateShaderModule = NULL;
PFN_vkCreateSwapchainKHR lineargl_vkCreateSwapchainKHR = NULL;
PFN_vkDebugReportMessageEXT lineargl_vkDebugReportMessageEXT = NULL;
PFN_vkDestroyBuffer lineargl_vkDestroyBuffer = NULL;
PFN_vkDestroyBufferView lineargl_vkDestroyBufferView = NULL;
PFN_vkDestroyCommandPool lineargl_vkDestroyCommandPool = NULL;
PFN_vkDestroyDebugReportCallbackEXT lineargl_vkDestroyDebugReportCallbackEXT = NULL;
PFN_vkDestroyDescriptorPool lineargl_vkDestroyDescriptorPool = NULL;
PFN_vkDestroyDescriptorSetLayout lineargl_vkDestroyDescriptorSetLayout = NULL;
PFN_vkDestroyDescriptorUpdateTemplate lineargl_vkDestroyDescriptorUpdateTemplate = NULL;
PFN_vkDestroyDevice lineargl_vkDestroyDevice = NULL;
PFN_vkDestroyEvent lineargl_vkDestroyEvent = NULL;
PFN_vkDestroyFence lineargl_vkDestroyFence = NULL;
PFN_vkDestroyFramebuffer lineargl_vkDestroyFramebuffer = NULL;
PFN_vkDestroyImage lineargl_vkDestroyImage = NULL;
PFN_vkDestroyImageView lineargl_vkDestroyImageView = NULL;
PFN_vkDestroyInstance lineargl_vkDestroyInstance = NULL;
PFN_vkDestroyPipeline lineargl_vkDestroyPipeline = NULL;
PFN_vkDestroyPipelineCache lineargl_vkDestroyPipelineCache = NULL;
PFN_vkDestroyPipelineLayout lineargl_vkDestroyPipelineLayout = NULL;
PFN_vkDestroyPrivateDataSlot lineargl_vkDestroyPrivateDataSlot = NULL;
PFN_vkDestroyQueryPool lineargl_vkDestroyQueryPool = NULL;
PFN_vkDestroyRenderPass lineargl_vkDestroyRenderPass = NULL;
PFN_vkDestroySampler lineargl_vkDestroySampler = NULL;
PFN_vkDestroySamplerYcbcrConversion lineargl_vkDestroySamplerYcbcrConversion = NULL;
PFN_vkDestroySemaphore lineargl_vkDestroySemaphore = NULL;
PFN_vkDestroyShaderModule lineargl_vkDestroyShaderModule = NULL;
PFN_vkDestroySurfaceKHR lineargl_vkDestroySurfaceKHR = NULL;
PFN_vkDestroySwapchainKHR lineargl_vkDestroySwapchainKHR = NULL;
PFN_vkDeviceWaitIdle lineargl_vkDeviceWaitIdle = NULL;
PFN_vkEndCommandBuffer lineargl_vkEndCommandBuffer = NULL;
PFN_vkEnumerateDeviceExtensionProperties lineargl_vkEnumerateDeviceExtensionProperties = NULL;
PFN_vkEnumerateDeviceLayerProperties lineargl_vkEnumerateDeviceLayerProperties = NULL;
PFN_vkEnumerateInstanceExtensionProperties lineargl_vkEnumerateInstanceExtensionProperties = NULL;
PFN_vkEnumerateInstanceLayerProperties lineargl_vkEnumerateInstanceLayerProperties = NULL;
PFN_vkEnumerateInstanceVersion lineargl_vkEnumerateInstanceVersion = NULL;
PFN_vkEnumeratePhysicalDeviceGroups lineargl_vkEnumeratePhysicalDeviceGroups = NULL;
PFN_vkEnumeratePhysicalDevices lineargl_vkEnumeratePhysicalDevices = NULL;
PFN_vkFlushMappedMemoryRanges lineargl_vkFlushMappedMemoryRanges = NULL;
PFN_vkFreeCommandBuffers lineargl_vkFreeCommandBuffers = NULL;
PFN_vkFreeDescriptorSets lineargl_vkFreeDescriptorSets = NULL;
PFN_vkFreeMemory lineargl_vkFreeMemory = NULL;
PFN_vkGetBufferDeviceAddress lineargl_vkGetBufferDeviceAddress = NULL;
PFN_vkGetBufferMemoryRequirements lineargl_vkGetBufferMemoryRequirements = NULL;
PFN_vkGetBufferMemoryRequirements2 lineargl_vkGetBufferMemoryRequirements2 = NULL;
PFN_vkGetBufferOpaqueCaptureAddress lineargl_vkGetBufferOpaqueCaptureAddress = NULL;
PFN_vkGetDescriptorSetLayoutSupport lineargl_vkGetDescriptorSetLayoutSupport = NULL;
PFN_vkGetDeviceBufferMemoryRequirements lineargl_vkGetDeviceBufferMemoryRequirements = NULL;
PFN_vkGetDeviceGroupPeerMemoryFeatures lineargl_vkGetDeviceGroupPeerMemoryFeatures = NULL;
PFN_vkGetDeviceGroupPresentCapabilitiesKHR lineargl_vkGetDeviceGroupPresentCapabilitiesKHR = NULL;
PFN_vkGetDeviceGroupSurfacePresentModesKHR lineargl_vkGetDeviceGroupSurfacePresentModesKHR = NULL;
PFN_vkGetDeviceImageMemoryRequirements lineargl_vkGetDeviceImageMemoryRequirements = NULL;
PFN_vkGetDeviceImageSparseMemoryRequirements lineargl_vkGetDeviceImageSparseMemoryRequirements = NULL;
PFN_vkGetDeviceMemoryCommitment lineargl_vkGetDeviceMemoryCommitment = NULL;
PFN_vkGetDeviceMemoryOpaqueCaptureAddress lineargl_vkGetDeviceMemoryOpaqueCaptureAddress = NULL;
PFN_vkGetDeviceProcAddr lineargl_vkGetDeviceProcAddr = NULL;
PFN_vkGetDeviceQueue lineargl_vkGetDeviceQueue = NULL;
PFN_vkGetDeviceQueue2 lineargl_vkGetDeviceQueue2 = NULL;
PFN_vkGetEventStatus lineargl_vkGetEventStatus = NULL;
PFN_vkGetFenceStatus lineargl_vkGetFenceStatus = NULL;
PFN_vkGetImageMemoryRequirements lineargl_vkGetImageMemoryRequirements = NULL;
PFN_vkGetImageMemoryRequirements2 lineargl_vkGetImageMemoryRequirements2 = NULL;
PFN_vkGetImageSparseMemoryRequirements lineargl_vkGetImageSparseMemoryRequirements = NULL;
PFN_vkGetImageSparseMemoryRequirements2 lineargl_vkGetImageSparseMemoryRequirements2 = NULL;
PFN_vkGetImageSubresourceLayout lineargl_vkGetImageSubresourceLayout = NULL;
PFN_vkGetInstanceProcAddr lineargl_vkGetInstanceProcAddr = NULL;
PFN_vkGetPhysicalDeviceExternalBufferProperties lineargl_vkGetPhysicalDeviceExternalBufferProperties = NULL;
PFN_vkGetPhysicalDeviceExternalFenceProperties lineargl_vkGetPhysicalDeviceExternalFenceProperties = NULL;
PFN_vkGetPhysicalDeviceExternalSemaphoreProperties lineargl_vkGetPhysicalDeviceExternalSemaphoreProperties = NULL;
PFN_vkGetPhysicalDeviceFeatures lineargl_vkGetPhysicalDeviceFeatures = NULL;
PFN_vkGetPhysicalDeviceFeatures2 lineargl_vkGetPhysicalDeviceFeatures2 = NULL;
PFN_vkGetPhysicalDeviceFormatProperties lineargl_vkGetPhysicalDeviceFormatProperties = NULL;
PFN_vkGetPhysicalDeviceFormatProperties2 lineargl_vkGetPhysicalDeviceFormatProperties2 = NULL;
PFN_vkGetPhysicalDeviceImageFormatProperties lineargl_vkGetPhysicalDeviceImageFormatProperties = NULL;
PFN_vkGetPhysicalDeviceImageFormatProperties2 lineargl_vkGetPhysicalDeviceImageFormatProperties2 = NULL;
PFN_vkGetPhysicalDeviceMemoryProperties lineargl_vkGetPhysicalDeviceMemoryProperties = NULL;
PFN_vkGetPhysicalDeviceMemoryProperties2 lineargl_vkGetPhysicalDeviceMemoryProperties2 = NULL;
PFN_vkGetPhysicalDevicePresentRectanglesKHR lineargl_vkGetPhysicalDevicePresentRectanglesKHR = NULL;
PFN_vkGetPhysicalDeviceProperties lineargl_vkGetPhysicalDeviceProperties = NULL;
PFN_vkGetPhysicalDeviceProperties2 lineargl_vkGetPhysicalDeviceProperties2 = NULL;
PFN_vkGetPhysicalDeviceQueueFamilyProperties lineargl_vkGetPhysicalDeviceQueueFamilyProperties = NULL;
PFN_vkGetPhysicalDeviceQueueFamilyProperties2 lineargl_vkGetPhysicalDeviceQueueFamilyProperties2 = NULL;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties lineargl_vkGetPhysicalDeviceSparseImageFormatProperties = NULL;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 lineargl_vkGetPhysicalDeviceSparseImageFormatProperties2 = NULL;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR lineargl_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR lineargl_vkGetPhysicalDeviceSurfaceFormatsKHR = NULL;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR lineargl_vkGetPhysicalDeviceSurfacePresentModesKHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR lineargl_vkGetPhysicalDeviceSurfaceSupportKHR = NULL;
PFN_vkGetPhysicalDeviceToolProperties lineargl_vkGetPhysicalDeviceToolProperties = NULL;
PFN_vkGetPipelineCacheData lineargl_vkGetPipelineCacheData = NULL;
PFN_vkGetPrivateData lineargl_vkGetPrivateData = NULL;
PFN_vkGetQueryPoolResults lineargl_vkGetQueryPoolResults = NULL;
PFN_vkGetRenderAreaGranularity lineargl_vkGetRenderAreaGranularity = NULL;
PFN_vkGetSemaphoreCounterValue lineargl_vkGetSemaphoreCounterValue = NULL;
PFN_vkGetSwapchainImagesKHR lineargl_vkGetSwapchainImagesKHR = NULL;
PFN_vkInvalidateMappedMemoryRanges lineargl_vkInvalidateMappedMemoryRanges = NULL;
PFN_vkMapMemory lineargl_vkMapMemory = NULL;
PFN_vkMergePipelineCaches lineargl_vkMergePipelineCaches = NULL;
PFN_vkQueueBindSparse lineargl_vkQueueBindSparse = NULL;
PFN_vkQueuePresentKHR lineargl_vkQueuePresentKHR = NULL;
PFN_vkQueueSubmit lineargl_vkQueueSubmit = NULL;
PFN_vkQueueSubmit2 lineargl_vkQueueSubmit2 = NULL;
PFN_vkQueueWaitIdle lineargl_vkQueueWaitIdle = NULL;
PFN_vkResetCommandBuffer lineargl_vkResetCommandBuffer = NULL;
PFN_vkResetCommandPool lineargl_vkResetCommandPool = NULL;
PFN_vkResetDescriptorPool lineargl_vkResetDescriptorPool = NULL;
PFN_vkResetEvent lineargl_vkResetEvent = NULL;
PFN_vkResetFences lineargl_vkResetFences = NULL;
PFN_vkResetQueryPool lineargl_vkResetQueryPool = NULL;
PFN_vkSetEvent lineargl_vkSetEvent = NULL;
PFN_vkSetPrivateData lineargl_vkSetPrivateData = NULL;
PFN_vkSignalSemaphore lineargl_vkSignalSemaphore = NULL;
PFN_vkTrimCommandPool lineargl_vkTrimCommandPool = NULL;
PFN_vkUnmapMemory lineargl_vkUnmapMemory = NULL;
PFN_vkUpdateDescriptorSetWithTemplate lineargl_vkUpdateDescriptorSetWithTemplate = NULL;
PFN_vkUpdateDescriptorSets lineargl_vkUpdateDescriptorSets = NULL;
PFN_vkWaitForFences lineargl_vkWaitForFences = NULL;
PFN_vkWaitSemaphores lineargl_vkWaitSemaphores = NULL;


static void lineargl_vk_load_VK_VERSION_1_0( LINEARGLuserptrloadfunc load, void* userptr) {
    if(!LINEARGL_VK_VERSION_1_0) return;
    lineargl_vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers) load(userptr, "vkAllocateCommandBuffers");
    lineargl_vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets) load(userptr, "vkAllocateDescriptorSets");
    lineargl_vkAllocateMemory = (PFN_vkAllocateMemory) load(userptr, "vkAllocateMemory");
    lineargl_vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer) load(userptr, "vkBeginCommandBuffer");
    lineargl_vkBindBufferMemory = (PFN_vkBindBufferMemory) load(userptr, "vkBindBufferMemory");
    lineargl_vkBindImageMemory = (PFN_vkBindImageMemory) load(userptr, "vkBindImageMemory");
    lineargl_vkCmdBeginQuery = (PFN_vkCmdBeginQuery) load(userptr, "vkCmdBeginQuery");
    lineargl_vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass) load(userptr, "vkCmdBeginRenderPass");
    lineargl_vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets) load(userptr, "vkCmdBindDescriptorSets");
    lineargl_vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer) load(userptr, "vkCmdBindIndexBuffer");
    lineargl_vkCmdBindPipeline = (PFN_vkCmdBindPipeline) load(userptr, "vkCmdBindPipeline");
    lineargl_vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers) load(userptr, "vkCmdBindVertexBuffers");
    lineargl_vkCmdBlitImage = (PFN_vkCmdBlitImage) load(userptr, "vkCmdBlitImage");
    lineargl_vkCmdClearAttachments = (PFN_vkCmdClearAttachments) load(userptr, "vkCmdClearAttachments");
    lineargl_vkCmdClearColorImage = (PFN_vkCmdClearColorImage) load(userptr, "vkCmdClearColorImage");
    lineargl_vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage) load(userptr, "vkCmdClearDepthStencilImage");
    lineargl_vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer) load(userptr, "vkCmdCopyBuffer");
    lineargl_vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage) load(userptr, "vkCmdCopyBufferToImage");
    lineargl_vkCmdCopyImage = (PFN_vkCmdCopyImage) load(userptr, "vkCmdCopyImage");
    lineargl_vkCmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer) load(userptr, "vkCmdCopyImageToBuffer");
    lineargl_vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults) load(userptr, "vkCmdCopyQueryPoolResults");
    lineargl_vkCmdDispatch = (PFN_vkCmdDispatch) load(userptr, "vkCmdDispatch");
    lineargl_vkCmdDispatchIndirect = (PFN_vkCmdDispatchIndirect) load(userptr, "vkCmdDispatchIndirect");
    lineargl_vkCmdDraw = (PFN_vkCmdDraw) load(userptr, "vkCmdDraw");
    lineargl_vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed) load(userptr, "vkCmdDrawIndexed");
    lineargl_vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect) load(userptr, "vkCmdDrawIndexedIndirect");
    lineargl_vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect) load(userptr, "vkCmdDrawIndirect");
    lineargl_vkCmdEndQuery = (PFN_vkCmdEndQuery) load(userptr, "vkCmdEndQuery");
    lineargl_vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass) load(userptr, "vkCmdEndRenderPass");
    lineargl_vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands) load(userptr, "vkCmdExecuteCommands");
    lineargl_vkCmdFillBuffer = (PFN_vkCmdFillBuffer) load(userptr, "vkCmdFillBuffer");
    lineargl_vkCmdNextSubpass = (PFN_vkCmdNextSubpass) load(userptr, "vkCmdNextSubpass");
    lineargl_vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier) load(userptr, "vkCmdPipelineBarrier");
    lineargl_vkCmdPushConstants = (PFN_vkCmdPushConstants) load(userptr, "vkCmdPushConstants");
    lineargl_vkCmdResetEvent = (PFN_vkCmdResetEvent) load(userptr, "vkCmdResetEvent");
    lineargl_vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool) load(userptr, "vkCmdResetQueryPool");
    lineargl_vkCmdResolveImage = (PFN_vkCmdResolveImage) load(userptr, "vkCmdResolveImage");
    lineargl_vkCmdSetBlendConstants = (PFN_vkCmdSetBlendConstants) load(userptr, "vkCmdSetBlendConstants");
    lineargl_vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias) load(userptr, "vkCmdSetDepthBias");
    lineargl_vkCmdSetDepthBounds = (PFN_vkCmdSetDepthBounds) load(userptr, "vkCmdSetDepthBounds");
    lineargl_vkCmdSetEvent = (PFN_vkCmdSetEvent) load(userptr, "vkCmdSetEvent");
    lineargl_vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth) load(userptr, "vkCmdSetLineWidth");
    lineargl_vkCmdSetScissor = (PFN_vkCmdSetScissor) load(userptr, "vkCmdSetScissor");
    lineargl_vkCmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask) load(userptr, "vkCmdSetStencilCompareMask");
    lineargl_vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference) load(userptr, "vkCmdSetStencilReference");
    lineargl_vkCmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask) load(userptr, "vkCmdSetStencilWriteMask");
    lineargl_vkCmdSetViewport = (PFN_vkCmdSetViewport) load(userptr, "vkCmdSetViewport");
    lineargl_vkCmdUpdateBuffer = (PFN_vkCmdUpdateBuffer) load(userptr, "vkCmdUpdateBuffer");
    lineargl_vkCmdWaitEvents = (PFN_vkCmdWaitEvents) load(userptr, "vkCmdWaitEvents");
    lineargl_vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp) load(userptr, "vkCmdWriteTimestamp");
    lineargl_vkCreateBuffer = (PFN_vkCreateBuffer) load(userptr, "vkCreateBuffer");
    lineargl_vkCreateBufferView = (PFN_vkCreateBufferView) load(userptr, "vkCreateBufferView");
    lineargl_vkCreateCommandPool = (PFN_vkCreateCommandPool) load(userptr, "vkCreateCommandPool");
    lineargl_vkCreateComputePipelines = (PFN_vkCreateComputePipelines) load(userptr, "vkCreateComputePipelines");
    lineargl_vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool) load(userptr, "vkCreateDescriptorPool");
    lineargl_vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout) load(userptr, "vkCreateDescriptorSetLayout");
    lineargl_vkCreateDevice = (PFN_vkCreateDevice) load(userptr, "vkCreateDevice");
    lineargl_vkCreateEvent = (PFN_vkCreateEvent) load(userptr, "vkCreateEvent");
    lineargl_vkCreateFence = (PFN_vkCreateFence) load(userptr, "vkCreateFence");
    lineargl_vkCreateFramebuffer = (PFN_vkCreateFramebuffer) load(userptr, "vkCreateFramebuffer");
    lineargl_vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines) load(userptr, "vkCreateGraphicsPipelines");
    lineargl_vkCreateImage = (PFN_vkCreateImage) load(userptr, "vkCreateImage");
    lineargl_vkCreateImageView = (PFN_vkCreateImageView) load(userptr, "vkCreateImageView");
    lineargl_vkCreateInstance = (PFN_vkCreateInstance) load(userptr, "vkCreateInstance");
    lineargl_vkCreatePipelineCache = (PFN_vkCreatePipelineCache) load(userptr, "vkCreatePipelineCache");
    lineargl_vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout) load(userptr, "vkCreatePipelineLayout");
    lineargl_vkCreateQueryPool = (PFN_vkCreateQueryPool) load(userptr, "vkCreateQueryPool");
    lineargl_vkCreateRenderPass = (PFN_vkCreateRenderPass) load(userptr, "vkCreateRenderPass");
    lineargl_vkCreateSampler = (PFN_vkCreateSampler) load(userptr, "vkCreateSampler");
    lineargl_vkCreateSemaphore = (PFN_vkCreateSemaphore) load(userptr, "vkCreateSemaphore");
    lineargl_vkCreateShaderModule = (PFN_vkCreateShaderModule) load(userptr, "vkCreateShaderModule");
    lineargl_vkDestroyBuffer = (PFN_vkDestroyBuffer) load(userptr, "vkDestroyBuffer");
    lineargl_vkDestroyBufferView = (PFN_vkDestroyBufferView) load(userptr, "vkDestroyBufferView");
    lineargl_vkDestroyCommandPool = (PFN_vkDestroyCommandPool) load(userptr, "vkDestroyCommandPool");
    lineargl_vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool) load(userptr, "vkDestroyDescriptorPool");
    lineargl_vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout) load(userptr, "vkDestroyDescriptorSetLayout");
    lineargl_vkDestroyDevice = (PFN_vkDestroyDevice) load(userptr, "vkDestroyDevice");
    lineargl_vkDestroyEvent = (PFN_vkDestroyEvent) load(userptr, "vkDestroyEvent");
    lineargl_vkDestroyFence = (PFN_vkDestroyFence) load(userptr, "vkDestroyFence");
    lineargl_vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer) load(userptr, "vkDestroyFramebuffer");
    lineargl_vkDestroyImage = (PFN_vkDestroyImage) load(userptr, "vkDestroyImage");
    lineargl_vkDestroyImageView = (PFN_vkDestroyImageView) load(userptr, "vkDestroyImageView");
    lineargl_vkDestroyInstance = (PFN_vkDestroyInstance) load(userptr, "vkDestroyInstance");
    lineargl_vkDestroyPipeline = (PFN_vkDestroyPipeline) load(userptr, "vkDestroyPipeline");
    lineargl_vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache) load(userptr, "vkDestroyPipelineCache");
    lineargl_vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout) load(userptr, "vkDestroyPipelineLayout");
    lineargl_vkDestroyQueryPool = (PFN_vkDestroyQueryPool) load(userptr, "vkDestroyQueryPool");
    lineargl_vkDestroyRenderPass = (PFN_vkDestroyRenderPass) load(userptr, "vkDestroyRenderPass");
    lineargl_vkDestroySampler = (PFN_vkDestroySampler) load(userptr, "vkDestroySampler");
    lineargl_vkDestroySemaphore = (PFN_vkDestroySemaphore) load(userptr, "vkDestroySemaphore");
    lineargl_vkDestroyShaderModule = (PFN_vkDestroyShaderModule) load(userptr, "vkDestroyShaderModule");
    lineargl_vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle) load(userptr, "vkDeviceWaitIdle");
    lineargl_vkEndCommandBuffer = (PFN_vkEndCommandBuffer) load(userptr, "vkEndCommandBuffer");
    lineargl_vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) load(userptr, "vkEnumerateDeviceExtensionProperties");
    lineargl_vkEnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties) load(userptr, "vkEnumerateDeviceLayerProperties");
    lineargl_vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties) load(userptr, "vkEnumerateInstanceExtensionProperties");
    lineargl_vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties) load(userptr, "vkEnumerateInstanceLayerProperties");
    lineargl_vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) load(userptr, "vkEnumeratePhysicalDevices");
    lineargl_vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges) load(userptr, "vkFlushMappedMemoryRanges");
    lineargl_vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers) load(userptr, "vkFreeCommandBuffers");
    lineargl_vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets) load(userptr, "vkFreeDescriptorSets");
    lineargl_vkFreeMemory = (PFN_vkFreeMemory) load(userptr, "vkFreeMemory");
    lineargl_vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements) load(userptr, "vkGetBufferMemoryRequirements");
    lineargl_vkGetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment) load(userptr, "vkGetDeviceMemoryCommitment");
    lineargl_vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr) load(userptr, "vkGetDeviceProcAddr");
    lineargl_vkGetDeviceQueue = (PFN_vkGetDeviceQueue) load(userptr, "vkGetDeviceQueue");
    lineargl_vkGetEventStatus = (PFN_vkGetEventStatus) load(userptr, "vkGetEventStatus");
    lineargl_vkGetFenceStatus = (PFN_vkGetFenceStatus) load(userptr, "vkGetFenceStatus");
    lineargl_vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements) load(userptr, "vkGetImageMemoryRequirements");
    lineargl_vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements) load(userptr, "vkGetImageSparseMemoryRequirements");
    lineargl_vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout) load(userptr, "vkGetImageSubresourceLayout");
    lineargl_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) load(userptr, "vkGetInstanceProcAddr");
    lineargl_vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) load(userptr, "vkGetPhysicalDeviceFeatures");
    lineargl_vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties) load(userptr, "vkGetPhysicalDeviceFormatProperties");
    lineargl_vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties) load(userptr, "vkGetPhysicalDeviceImageFormatProperties");
    lineargl_vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties) load(userptr, "vkGetPhysicalDeviceMemoryProperties");
    lineargl_vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) load(userptr, "vkGetPhysicalDeviceProperties");
    lineargl_vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) load(userptr, "vkGetPhysicalDeviceQueueFamilyProperties");
    lineargl_vkGetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties) load(userptr, "vkGetPhysicalDeviceSparseImageFormatProperties");
    lineargl_vkGetPipelineCacheData = (PFN_vkGetPipelineCacheData) load(userptr, "vkGetPipelineCacheData");
    lineargl_vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults) load(userptr, "vkGetQueryPoolResults");
    lineargl_vkGetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity) load(userptr, "vkGetRenderAreaGranularity");
    lineargl_vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges) load(userptr, "vkInvalidateMappedMemoryRanges");
    lineargl_vkMapMemory = (PFN_vkMapMemory) load(userptr, "vkMapMemory");
    lineargl_vkMergePipelineCaches = (PFN_vkMergePipelineCaches) load(userptr, "vkMergePipelineCaches");
    lineargl_vkQueueBindSparse = (PFN_vkQueueBindSparse) load(userptr, "vkQueueBindSparse");
    lineargl_vkQueueSubmit = (PFN_vkQueueSubmit) load(userptr, "vkQueueSubmit");
    lineargl_vkQueueWaitIdle = (PFN_vkQueueWaitIdle) load(userptr, "vkQueueWaitIdle");
    lineargl_vkResetCommandBuffer = (PFN_vkResetCommandBuffer) load(userptr, "vkResetCommandBuffer");
    lineargl_vkResetCommandPool = (PFN_vkResetCommandPool) load(userptr, "vkResetCommandPool");
    lineargl_vkResetDescriptorPool = (PFN_vkResetDescriptorPool) load(userptr, "vkResetDescriptorPool");
    lineargl_vkResetEvent = (PFN_vkResetEvent) load(userptr, "vkResetEvent");
    lineargl_vkResetFences = (PFN_vkResetFences) load(userptr, "vkResetFences");
    lineargl_vkSetEvent = (PFN_vkSetEvent) load(userptr, "vkSetEvent");
    lineargl_vkUnmapMemory = (PFN_vkUnmapMemory) load(userptr, "vkUnmapMemory");
    lineargl_vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets) load(userptr, "vkUpdateDescriptorSets");
    lineargl_vkWaitForFences = (PFN_vkWaitForFences) load(userptr, "vkWaitForFences");
}
static void lineargl_vk_load_VK_VERSION_1_1( LINEARGLuserptrloadfunc load, void* userptr) {
    if(!LINEARGL_VK_VERSION_1_1) return;
    lineargl_vkBindBufferMemory2 = (PFN_vkBindBufferMemory2) load(userptr, "vkBindBufferMemory2");
    lineargl_vkBindImageMemory2 = (PFN_vkBindImageMemory2) load(userptr, "vkBindImageMemory2");
    lineargl_vkCmdDispatchBase = (PFN_vkCmdDispatchBase) load(userptr, "vkCmdDispatchBase");
    lineargl_vkCmdSetDeviceMask = (PFN_vkCmdSetDeviceMask) load(userptr, "vkCmdSetDeviceMask");
    lineargl_vkCreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate) load(userptr, "vkCreateDescriptorUpdateTemplate");
    lineargl_vkCreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion) load(userptr, "vkCreateSamplerYcbcrConversion");
    lineargl_vkDestroyDescriptorUpdateTemplate = (PFN_vkDestroyDescriptorUpdateTemplate) load(userptr, "vkDestroyDescriptorUpdateTemplate");
    lineargl_vkDestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion) load(userptr, "vkDestroySamplerYcbcrConversion");
    lineargl_vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion) load(userptr, "vkEnumerateInstanceVersion");
    lineargl_vkEnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups) load(userptr, "vkEnumeratePhysicalDeviceGroups");
    lineargl_vkGetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2) load(userptr, "vkGetBufferMemoryRequirements2");
    lineargl_vkGetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport) load(userptr, "vkGetDescriptorSetLayoutSupport");
    lineargl_vkGetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures) load(userptr, "vkGetDeviceGroupPeerMemoryFeatures");
    lineargl_vkGetDeviceQueue2 = (PFN_vkGetDeviceQueue2) load(userptr, "vkGetDeviceQueue2");
    lineargl_vkGetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2) load(userptr, "vkGetImageMemoryRequirements2");
    lineargl_vkGetImageSparseMemoryRequirements2 = (PFN_vkGetImageSparseMemoryRequirements2) load(userptr, "vkGetImageSparseMemoryRequirements2");
    lineargl_vkGetPhysicalDeviceExternalBufferProperties = (PFN_vkGetPhysicalDeviceExternalBufferProperties) load(userptr, "vkGetPhysicalDeviceExternalBufferProperties");
    lineargl_vkGetPhysicalDeviceExternalFenceProperties = (PFN_vkGetPhysicalDeviceExternalFenceProperties) load(userptr, "vkGetPhysicalDeviceExternalFenceProperties");
    lineargl_vkGetPhysicalDeviceExternalSemaphoreProperties = (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties) load(userptr, "vkGetPhysicalDeviceExternalSemaphoreProperties");
    lineargl_vkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2) load(userptr, "vkGetPhysicalDeviceFeatures2");
    lineargl_vkGetPhysicalDeviceFormatProperties2 = (PFN_vkGetPhysicalDeviceFormatProperties2) load(userptr, "vkGetPhysicalDeviceFormatProperties2");
    lineargl_vkGetPhysicalDeviceImageFormatProperties2 = (PFN_vkGetPhysicalDeviceImageFormatProperties2) load(userptr, "vkGetPhysicalDeviceImageFormatProperties2");
    lineargl_vkGetPhysicalDeviceMemoryProperties2 = (PFN_vkGetPhysicalDeviceMemoryProperties2) load(userptr, "vkGetPhysicalDeviceMemoryProperties2");
    lineargl_vkGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2) load(userptr, "vkGetPhysicalDeviceProperties2");
    lineargl_vkGetPhysicalDeviceQueueFamilyProperties2 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2) load(userptr, "vkGetPhysicalDeviceQueueFamilyProperties2");
    lineargl_vkGetPhysicalDeviceSparseImageFormatProperties2 = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2) load(userptr, "vkGetPhysicalDeviceSparseImageFormatProperties2");
    lineargl_vkTrimCommandPool = (PFN_vkTrimCommandPool) load(userptr, "vkTrimCommandPool");
    lineargl_vkUpdateDescriptorSetWithTemplate = (PFN_vkUpdateDescriptorSetWithTemplate) load(userptr, "vkUpdateDescriptorSetWithTemplate");
}
static void lineargl_vk_load_VK_VERSION_1_2( LINEARGLuserptrloadfunc load, void* userptr) {
    if(!LINEARGL_VK_VERSION_1_2) return;
    lineargl_vkCmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2) load(userptr, "vkCmdBeginRenderPass2");
    lineargl_vkCmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount) load(userptr, "vkCmdDrawIndexedIndirectCount");
    lineargl_vkCmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount) load(userptr, "vkCmdDrawIndirectCount");
    lineargl_vkCmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2) load(userptr, "vkCmdEndRenderPass2");
    lineargl_vkCmdNextSubpass2 = (PFN_vkCmdNextSubpass2) load(userptr, "vkCmdNextSubpass2");
    lineargl_vkCreateRenderPass2 = (PFN_vkCreateRenderPass2) load(userptr, "vkCreateRenderPass2");
    lineargl_vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress) load(userptr, "vkGetBufferDeviceAddress");
    lineargl_vkGetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress) load(userptr, "vkGetBufferOpaqueCaptureAddress");
    lineargl_vkGetDeviceMemoryOpaqueCaptureAddress = (PFN_vkGetDeviceMemoryOpaqueCaptureAddress) load(userptr, "vkGetDeviceMemoryOpaqueCaptureAddress");
    lineargl_vkGetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue) load(userptr, "vkGetSemaphoreCounterValue");
    lineargl_vkResetQueryPool = (PFN_vkResetQueryPool) load(userptr, "vkResetQueryPool");
    lineargl_vkSignalSemaphore = (PFN_vkSignalSemaphore) load(userptr, "vkSignalSemaphore");
    lineargl_vkWaitSemaphores = (PFN_vkWaitSemaphores) load(userptr, "vkWaitSemaphores");
}
static void lineargl_vk_load_VK_VERSION_1_3( LINEARGLuserptrloadfunc load, void* userptr) {
    if(!LINEARGL_VK_VERSION_1_3) return;
    lineargl_vkCmdBeginRendering = (PFN_vkCmdBeginRendering) load(userptr, "vkCmdBeginRendering");
    lineargl_vkCmdBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2) load(userptr, "vkCmdBindVertexBuffers2");
    lineargl_vkCmdBlitImage2 = (PFN_vkCmdBlitImage2) load(userptr, "vkCmdBlitImage2");
    lineargl_vkCmdCopyBuffer2 = (PFN_vkCmdCopyBuffer2) load(userptr, "vkCmdCopyBuffer2");
    lineargl_vkCmdCopyBufferToImage2 = (PFN_vkCmdCopyBufferToImage2) load(userptr, "vkCmdCopyBufferToImage2");
    lineargl_vkCmdCopyImage2 = (PFN_vkCmdCopyImage2) load(userptr, "vkCmdCopyImage2");
    lineargl_vkCmdCopyImageToBuffer2 = (PFN_vkCmdCopyImageToBuffer2) load(userptr, "vkCmdCopyImageToBuffer2");
    lineargl_vkCmdEndRendering = (PFN_vkCmdEndRendering) load(userptr, "vkCmdEndRendering");
    lineargl_vkCmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2) load(userptr, "vkCmdPipelineBarrier2");
    lineargl_vkCmdResetEvent2 = (PFN_vkCmdResetEvent2) load(userptr, "vkCmdResetEvent2");
    lineargl_vkCmdResolveImage2 = (PFN_vkCmdResolveImage2) load(userptr, "vkCmdResolveImage2");
    lineargl_vkCmdSetCullMode = (PFN_vkCmdSetCullMode) load(userptr, "vkCmdSetCullMode");
    lineargl_vkCmdSetDepthBiasEnable = (PFN_vkCmdSetDepthBiasEnable) load(userptr, "vkCmdSetDepthBiasEnable");
    lineargl_vkCmdSetDepthBoundsTestEnable = (PFN_vkCmdSetDepthBoundsTestEnable) load(userptr, "vkCmdSetDepthBoundsTestEnable");
    lineargl_vkCmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp) load(userptr, "vkCmdSetDepthCompareOp");
    lineargl_vkCmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable) load(userptr, "vkCmdSetDepthTestEnable");
    lineargl_vkCmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable) load(userptr, "vkCmdSetDepthWriteEnable");
    lineargl_vkCmdSetEvent2 = (PFN_vkCmdSetEvent2) load(userptr, "vkCmdSetEvent2");
    lineargl_vkCmdSetFrontFace = (PFN_vkCmdSetFrontFace) load(userptr, "vkCmdSetFrontFace");
    lineargl_vkCmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable) load(userptr, "vkCmdSetPrimitiveRestartEnable");
    lineargl_vkCmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology) load(userptr, "vkCmdSetPrimitiveTopology");
    lineargl_vkCmdSetRasterizerDiscardEnable = (PFN_vkCmdSetRasterizerDiscardEnable) load(userptr, "vkCmdSetRasterizerDiscardEnable");
    lineargl_vkCmdSetScissorWithCount = (PFN_vkCmdSetScissorWithCount) load(userptr, "vkCmdSetScissorWithCount");
    lineargl_vkCmdSetStencilOp = (PFN_vkCmdSetStencilOp) load(userptr, "vkCmdSetStencilOp");
    lineargl_vkCmdSetStencilTestEnable = (PFN_vkCmdSetStencilTestEnable) load(userptr, "vkCmdSetStencilTestEnable");
    lineargl_vkCmdSetViewportWithCount = (PFN_vkCmdSetViewportWithCount) load(userptr, "vkCmdSetViewportWithCount");
    lineargl_vkCmdWaitEvents2 = (PFN_vkCmdWaitEvents2) load(userptr, "vkCmdWaitEvents2");
    lineargl_vkCmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2) load(userptr, "vkCmdWriteTimestamp2");
    lineargl_vkCreatePrivateDataSlot = (PFN_vkCreatePrivateDataSlot) load(userptr, "vkCreatePrivateDataSlot");
    lineargl_vkDestroyPrivateDataSlot = (PFN_vkDestroyPrivateDataSlot) load(userptr, "vkDestroyPrivateDataSlot");
    lineargl_vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements) load(userptr, "vkGetDeviceBufferMemoryRequirements");
    lineargl_vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements) load(userptr, "vkGetDeviceImageMemoryRequirements");
    lineargl_vkGetDeviceImageSparseMemoryRequirements = (PFN_vkGetDeviceImageSparseMemoryRequirements) load(userptr, "vkGetDeviceImageSparseMemoryRequirements");
    lineargl_vkGetPhysicalDeviceToolProperties = (PFN_vkGetPhysicalDeviceToolProperties) load(userptr, "vkGetPhysicalDeviceToolProperties");
    lineargl_vkGetPrivateData = (PFN_vkGetPrivateData) load(userptr, "vkGetPrivateData");
    lineargl_vkQueueSubmit2 = (PFN_vkQueueSubmit2) load(userptr, "vkQueueSubmit2");
    lineargl_vkSetPrivateData = (PFN_vkSetPrivateData) load(userptr, "vkSetPrivateData");
}
static void lineargl_vk_load_VK_EXT_debug_report( LINEARGLuserptrloadfunc load, void* userptr) {
    if(!LINEARGL_VK_EXT_debug_report) return;
    lineargl_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) load(userptr, "vkCreateDebugReportCallbackEXT");
    lineargl_vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT) load(userptr, "vkDebugReportMessageEXT");
    lineargl_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) load(userptr, "vkDestroyDebugReportCallbackEXT");
}
static void lineargl_vk_load_VK_KHR_surface( LINEARGLuserptrloadfunc load, void* userptr) {
    if(!LINEARGL_VK_KHR_surface) return;
    lineargl_vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR) load(userptr, "vkDestroySurfaceKHR");
    lineargl_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) load(userptr, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    lineargl_vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) load(userptr, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    lineargl_vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) load(userptr, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    lineargl_vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR) load(userptr, "vkGetPhysicalDeviceSurfaceSupportKHR");
}
static void lineargl_vk_load_VK_KHR_swapchain( LINEARGLuserptrloadfunc load, void* userptr) {
    if(!LINEARGL_VK_KHR_swapchain) return;
    lineargl_vkAcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR) load(userptr, "vkAcquireNextImage2KHR");
    lineargl_vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) load(userptr, "vkAcquireNextImageKHR");
    lineargl_vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) load(userptr, "vkCreateSwapchainKHR");
    lineargl_vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR) load(userptr, "vkDestroySwapchainKHR");
    lineargl_vkGetDeviceGroupPresentCapabilitiesKHR = (PFN_vkGetDeviceGroupPresentCapabilitiesKHR) load(userptr, "vkGetDeviceGroupPresentCapabilitiesKHR");
    lineargl_vkGetDeviceGroupSurfacePresentModesKHR = (PFN_vkGetDeviceGroupSurfacePresentModesKHR) load(userptr, "vkGetDeviceGroupSurfacePresentModesKHR");
    lineargl_vkGetPhysicalDevicePresentRectanglesKHR = (PFN_vkGetPhysicalDevicePresentRectanglesKHR) load(userptr, "vkGetPhysicalDevicePresentRectanglesKHR");
    lineargl_vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) load(userptr, "vkGetSwapchainImagesKHR");
    lineargl_vkQueuePresentKHR = (PFN_vkQueuePresentKHR) load(userptr, "vkQueuePresentKHR");
}



static int lineargl_vk_get_extensions( VkPhysicalDevice physical_device, uint32_t *out_extension_count, char ***out_extensions) {
    uint32_t i;
    uint32_t instance_extension_count = 0;
    uint32_t device_extension_count = 0;
    uint32_t max_extension_count = 0;
    uint32_t total_extension_count = 0;
    char **extensions = NULL;
    VkExtensionProperties *ext_properties = NULL;
    VkResult result;

    if (lineargl_vkEnumerateInstanceExtensionProperties == NULL || (physical_device != NULL && lineargl_vkEnumerateDeviceExtensionProperties == NULL)) {
        return 0;
    }

    result = lineargl_vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
    if (result != VK_SUCCESS) {
        return 0;
    }

    if (physical_device != NULL) {
        result = lineargl_vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extension_count, NULL);
        if (result != VK_SUCCESS) {
            return 0;
        }
    }

    total_extension_count = instance_extension_count + device_extension_count;
    if (total_extension_count <= 0) {
        return 0;
    }

    max_extension_count = instance_extension_count > device_extension_count
        ? instance_extension_count : device_extension_count;

    ext_properties = (VkExtensionProperties*) malloc(max_extension_count * sizeof(VkExtensionProperties));
    if (ext_properties == NULL) {
        goto lineargl_vk_get_extensions_error;
    }

    result = lineargl_vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, ext_properties);
    if (result != VK_SUCCESS) {
        goto lineargl_vk_get_extensions_error;
    }

    extensions = (char**) calloc(total_extension_count, sizeof(char*));
    if (extensions == NULL) {
        goto lineargl_vk_get_extensions_error;
    }

    for (i = 0; i < instance_extension_count; ++i) {
        VkExtensionProperties ext = ext_properties[i];

        size_t extension_name_length = strlen(ext.extensionName) + 1;
        extensions[i] = (char*) malloc(extension_name_length * sizeof(char));
        if (extensions[i] == NULL) {
            goto lineargl_vk_get_extensions_error;
        }
        memcpy(extensions[i], ext.extensionName, extension_name_length * sizeof(char));
    }

    if (physical_device != NULL) {
        result = lineargl_vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extension_count, ext_properties);
        if (result != VK_SUCCESS) {
            goto lineargl_vk_get_extensions_error;
        }

        for (i = 0; i < device_extension_count; ++i) {
            VkExtensionProperties ext = ext_properties[i];

            size_t extension_name_length = strlen(ext.extensionName) + 1;
            extensions[instance_extension_count + i] = (char*) malloc(extension_name_length * sizeof(char));
            if (extensions[instance_extension_count + i] == NULL) {
                goto lineargl_vk_get_extensions_error;
            }
            memcpy(extensions[instance_extension_count + i], ext.extensionName, extension_name_length * sizeof(char));
        }
    }

    free((void*) ext_properties);

    *out_extension_count = total_extension_count;
    *out_extensions = extensions;

    return 1;

lineargl_vk_get_extensions_error:
    free((void*) ext_properties);
    if (extensions != NULL) {
        for (i = 0; i < total_extension_count; ++i) {
            free((void*) extensions[i]);
        }
        free(extensions);
    }
    return 0;
}

static void lineargl_vk_free_extensions(uint32_t extension_count, char **extensions) {
    uint32_t i;

    for(i = 0; i < extension_count ; ++i) {
        free((void*) (extensions[i]));
    }

    free((void*) extensions);
}

static int lineargl_vk_has_extension(const char *name, uint32_t extension_count, char **extensions) {
    uint32_t i;

    for (i = 0; i < extension_count; ++i) {
        if(extensions[i] != NULL && strcmp(name, extensions[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

static LINEARGLapiproc lineargl_vk_get_proc_from_userptr(void *userptr, const char* name) {
    return (LINEARGL_GNUC_EXTENSION (LINEARGLapiproc (*)(const char *name)) userptr)(name);
}

static int lineargl_vk_find_extensions_vulkan( VkPhysicalDevice physical_device) {
    uint32_t extension_count = 0;
    char **extensions = NULL;
    if (!lineargl_vk_get_extensions(physical_device, &extension_count, &extensions)) return 0;

    LINEARGL_VK_EXT_debug_report = lineargl_vk_has_extension("VK_EXT_debug_report", extension_count, extensions);
    LINEARGL_VK_KHR_portability_enumeration = lineargl_vk_has_extension("VK_KHR_portability_enumeration", extension_count, extensions);
    LINEARGL_VK_KHR_surface = lineargl_vk_has_extension("VK_KHR_surface", extension_count, extensions);
    LINEARGL_VK_KHR_swapchain = lineargl_vk_has_extension("VK_KHR_swapchain", extension_count, extensions);

    (void) lineargl_vk_has_extension;

    lineargl_vk_free_extensions(extension_count, extensions);

    return 1;
}

static int lineargl_vk_find_core_vulkan( VkPhysicalDevice physical_device) {
    int major = 1;
    int minor = 0;

#ifdef VK_VERSION_1_1
    if (lineargl_vkEnumerateInstanceVersion != NULL) {
        uint32_t version;
        VkResult result;

        result = lineargl_vkEnumerateInstanceVersion(&version);
        if (result == VK_SUCCESS) {
            major = (int) VK_VERSION_MAJOR(version);
            minor = (int) VK_VERSION_MINOR(version);
        }
    }
#endif

    if (physical_device != NULL && lineargl_vkGetPhysicalDeviceProperties != NULL) {
        VkPhysicalDeviceProperties properties;
        lineargl_vkGetPhysicalDeviceProperties(physical_device, &properties);

        major = (int) VK_VERSION_MAJOR(properties.apiVersion);
        minor = (int) VK_VERSION_MINOR(properties.apiVersion);
    }

    LINEARGL_VK_VERSION_1_0 = (major == 1 && minor >= 0) || major > 1;
    LINEARGL_VK_VERSION_1_1 = (major == 1 && minor >= 1) || major > 1;
    LINEARGL_VK_VERSION_1_2 = (major == 1 && minor >= 2) || major > 1;
    LINEARGL_VK_VERSION_1_3 = (major == 1 && minor >= 3) || major > 1;

    return LINEARGL_MAKE_VERSION(major, minor);
}

int linearglLoadVulkanUserPtr( VkPhysicalDevice physical_device, LINEARGLuserptrloadfunc load, void *userptr) {
    int version;

#ifdef VK_VERSION_1_1
    lineargl_vkEnumerateInstanceVersion  = (PFN_vkEnumerateInstanceVersion) load(userptr, "vkEnumerateInstanceVersion");
#endif
    version = lineargl_vk_find_core_vulkan( physical_device);
    if (!version) {
        return 0;
    }

    lineargl_vk_load_VK_VERSION_1_0(load, userptr);
    lineargl_vk_load_VK_VERSION_1_1(load, userptr);
    lineargl_vk_load_VK_VERSION_1_2(load, userptr);
    lineargl_vk_load_VK_VERSION_1_3(load, userptr);

    if (!lineargl_vk_find_extensions_vulkan( physical_device)) return 0;
    lineargl_vk_load_VK_EXT_debug_report(load, userptr);
    lineargl_vk_load_VK_KHR_surface(load, userptr);
    lineargl_vk_load_VK_KHR_swapchain(load, userptr);


    return version;
}


int linearglLoadVulkan( VkPhysicalDevice physical_device, LINEARGLloadfunc load) {
    return linearglLoadVulkanUserPtr( physical_device, lineargl_vk_get_proc_from_userptr, LINEARGL_GNUC_EXTENSION (void*) load);
}



 


#ifdef __cplusplus
}
#endif
