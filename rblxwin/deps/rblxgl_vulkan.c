#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rblxgl/vulkan.h>

#ifndef RBLXGL_IMPL_UTIL_C_
#define RBLXGL_IMPL_UTIL_C_

#ifdef _MSC_VER
#define RBLXGL_IMPL_UTIL_SSCANF sscanf_s
#else
#define RBLXGL_IMPL_UTIL_SSCANF sscanf
#endif

#endif /* RBLXGL_IMPL_UTIL_C_ */

#ifdef __cplusplus
extern "C" {
#endif



int RBLXGL_VK_VERSION_1_0 = 0;
int RBLXGL_VK_VERSION_1_1 = 0;
int RBLXGL_VK_VERSION_1_2 = 0;
int RBLXGL_VK_VERSION_1_3 = 0;
int RBLXGL_VK_EXT_debug_report = 0;
int RBLXGL_VK_KHR_portability_enumeration = 0;
int RBLXGL_VK_KHR_surface = 0;
int RBLXGL_VK_KHR_swapchain = 0;



PFN_vkAcquireNextImage2KHR rblxgl_vkAcquireNextImage2KHR = NULL;
PFN_vkAcquireNextImageKHR rblxgl_vkAcquireNextImageKHR = NULL;
PFN_vkAllocateCommandBuffers rblxgl_vkAllocateCommandBuffers = NULL;
PFN_vkAllocateDescriptorSets rblxgl_vkAllocateDescriptorSets = NULL;
PFN_vkAllocateMemory rblxgl_vkAllocateMemory = NULL;
PFN_vkBeginCommandBuffer rblxgl_vkBeginCommandBuffer = NULL;
PFN_vkBindBufferMemory rblxgl_vkBindBufferMemory = NULL;
PFN_vkBindBufferMemory2 rblxgl_vkBindBufferMemory2 = NULL;
PFN_vkBindImageMemory rblxgl_vkBindImageMemory = NULL;
PFN_vkBindImageMemory2 rblxgl_vkBindImageMemory2 = NULL;
PFN_vkCmdBeginQuery rblxgl_vkCmdBeginQuery = NULL;
PFN_vkCmdBeginRenderPass rblxgl_vkCmdBeginRenderPass = NULL;
PFN_vkCmdBeginRenderPass2 rblxgl_vkCmdBeginRenderPass2 = NULL;
PFN_vkCmdBeginRendering rblxgl_vkCmdBeginRendering = NULL;
PFN_vkCmdBindDescriptorSets rblxgl_vkCmdBindDescriptorSets = NULL;
PFN_vkCmdBindIndexBuffer rblxgl_vkCmdBindIndexBuffer = NULL;
PFN_vkCmdBindPipeline rblxgl_vkCmdBindPipeline = NULL;
PFN_vkCmdBindVertexBuffers rblxgl_vkCmdBindVertexBuffers = NULL;
PFN_vkCmdBindVertexBuffers2 rblxgl_vkCmdBindVertexBuffers2 = NULL;
PFN_vkCmdBlitImage rblxgl_vkCmdBlitImage = NULL;
PFN_vkCmdBlitImage2 rblxgl_vkCmdBlitImage2 = NULL;
PFN_vkCmdClearAttachments rblxgl_vkCmdClearAttachments = NULL;
PFN_vkCmdClearColorImage rblxgl_vkCmdClearColorImage = NULL;
PFN_vkCmdClearDepthStencilImage rblxgl_vkCmdClearDepthStencilImage = NULL;
PFN_vkCmdCopyBuffer rblxgl_vkCmdCopyBuffer = NULL;
PFN_vkCmdCopyBuffer2 rblxgl_vkCmdCopyBuffer2 = NULL;
PFN_vkCmdCopyBufferToImage rblxgl_vkCmdCopyBufferToImage = NULL;
PFN_vkCmdCopyBufferToImage2 rblxgl_vkCmdCopyBufferToImage2 = NULL;
PFN_vkCmdCopyImage rblxgl_vkCmdCopyImage = NULL;
PFN_vkCmdCopyImage2 rblxgl_vkCmdCopyImage2 = NULL;
PFN_vkCmdCopyImageToBuffer rblxgl_vkCmdCopyImageToBuffer = NULL;
PFN_vkCmdCopyImageToBuffer2 rblxgl_vkCmdCopyImageToBuffer2 = NULL;
PFN_vkCmdCopyQueryPoolResults rblxgl_vkCmdCopyQueryPoolResults = NULL;
PFN_vkCmdDispatch rblxgl_vkCmdDispatch = NULL;
PFN_vkCmdDispatchBase rblxgl_vkCmdDispatchBase = NULL;
PFN_vkCmdDispatchIndirect rblxgl_vkCmdDispatchIndirect = NULL;
PFN_vkCmdDraw rblxgl_vkCmdDraw = NULL;
PFN_vkCmdDrawIndexed rblxgl_vkCmdDrawIndexed = NULL;
PFN_vkCmdDrawIndexedIndirect rblxgl_vkCmdDrawIndexedIndirect = NULL;
PFN_vkCmdDrawIndexedIndirectCount rblxgl_vkCmdDrawIndexedIndirectCount = NULL;
PFN_vkCmdDrawIndirect rblxgl_vkCmdDrawIndirect = NULL;
PFN_vkCmdDrawIndirectCount rblxgl_vkCmdDrawIndirectCount = NULL;
PFN_vkCmdEndQuery rblxgl_vkCmdEndQuery = NULL;
PFN_vkCmdEndRenderPass rblxgl_vkCmdEndRenderPass = NULL;
PFN_vkCmdEndRenderPass2 rblxgl_vkCmdEndRenderPass2 = NULL;
PFN_vkCmdEndRendering rblxgl_vkCmdEndRendering = NULL;
PFN_vkCmdExecuteCommands rblxgl_vkCmdExecuteCommands = NULL;
PFN_vkCmdFillBuffer rblxgl_vkCmdFillBuffer = NULL;
PFN_vkCmdNextSubpass rblxgl_vkCmdNextSubpass = NULL;
PFN_vkCmdNextSubpass2 rblxgl_vkCmdNextSubpass2 = NULL;
PFN_vkCmdPipelineBarrier rblxgl_vkCmdPipelineBarrier = NULL;
PFN_vkCmdPipelineBarrier2 rblxgl_vkCmdPipelineBarrier2 = NULL;
PFN_vkCmdPushConstants rblxgl_vkCmdPushConstants = NULL;
PFN_vkCmdResetEvent rblxgl_vkCmdResetEvent = NULL;
PFN_vkCmdResetEvent2 rblxgl_vkCmdResetEvent2 = NULL;
PFN_vkCmdResetQueryPool rblxgl_vkCmdResetQueryPool = NULL;
PFN_vkCmdResolveImage rblxgl_vkCmdResolveImage = NULL;
PFN_vkCmdResolveImage2 rblxgl_vkCmdResolveImage2 = NULL;
PFN_vkCmdSetBlendConstants rblxgl_vkCmdSetBlendConstants = NULL;
PFN_vkCmdSetCullMode rblxgl_vkCmdSetCullMode = NULL;
PFN_vkCmdSetDepthBias rblxgl_vkCmdSetDepthBias = NULL;
PFN_vkCmdSetDepthBiasEnable rblxgl_vkCmdSetDepthBiasEnable = NULL;
PFN_vkCmdSetDepthBounds rblxgl_vkCmdSetDepthBounds = NULL;
PFN_vkCmdSetDepthBoundsTestEnable rblxgl_vkCmdSetDepthBoundsTestEnable = NULL;
PFN_vkCmdSetDepthCompareOp rblxgl_vkCmdSetDepthCompareOp = NULL;
PFN_vkCmdSetDepthTestEnable rblxgl_vkCmdSetDepthTestEnable = NULL;
PFN_vkCmdSetDepthWriteEnable rblxgl_vkCmdSetDepthWriteEnable = NULL;
PFN_vkCmdSetDeviceMask rblxgl_vkCmdSetDeviceMask = NULL;
PFN_vkCmdSetEvent rblxgl_vkCmdSetEvent = NULL;
PFN_vkCmdSetEvent2 rblxgl_vkCmdSetEvent2 = NULL;
PFN_vkCmdSetFrontFace rblxgl_vkCmdSetFrontFace = NULL;
PFN_vkCmdSetLineWidth rblxgl_vkCmdSetLineWidth = NULL;
PFN_vkCmdSetPrimitiveRestartEnable rblxgl_vkCmdSetPrimitiveRestartEnable = NULL;
PFN_vkCmdSetPrimitiveTopology rblxgl_vkCmdSetPrimitiveTopology = NULL;
PFN_vkCmdSetRasterizerDiscardEnable rblxgl_vkCmdSetRasterizerDiscardEnable = NULL;
PFN_vkCmdSetScissor rblxgl_vkCmdSetScissor = NULL;
PFN_vkCmdSetScissorWithCount rblxgl_vkCmdSetScissorWithCount = NULL;
PFN_vkCmdSetStencilCompareMask rblxgl_vkCmdSetStencilCompareMask = NULL;
PFN_vkCmdSetStencilOp rblxgl_vkCmdSetStencilOp = NULL;
PFN_vkCmdSetStencilReference rblxgl_vkCmdSetStencilReference = NULL;
PFN_vkCmdSetStencilTestEnable rblxgl_vkCmdSetStencilTestEnable = NULL;
PFN_vkCmdSetStencilWriteMask rblxgl_vkCmdSetStencilWriteMask = NULL;
PFN_vkCmdSetViewport rblxgl_vkCmdSetViewport = NULL;
PFN_vkCmdSetViewportWithCount rblxgl_vkCmdSetViewportWithCount = NULL;
PFN_vkCmdUpdateBuffer rblxgl_vkCmdUpdateBuffer = NULL;
PFN_vkCmdWaitEvents rblxgl_vkCmdWaitEvents = NULL;
PFN_vkCmdWaitEvents2 rblxgl_vkCmdWaitEvents2 = NULL;
PFN_vkCmdWriteTimestamp rblxgl_vkCmdWriteTimestamp = NULL;
PFN_vkCmdWriteTimestamp2 rblxgl_vkCmdWriteTimestamp2 = NULL;
PFN_vkCreateBuffer rblxgl_vkCreateBuffer = NULL;
PFN_vkCreateBufferView rblxgl_vkCreateBufferView = NULL;
PFN_vkCreateCommandPool rblxgl_vkCreateCommandPool = NULL;
PFN_vkCreateComputePipelines rblxgl_vkCreateComputePipelines = NULL;
PFN_vkCreateDebugReportCallbackEXT rblxgl_vkCreateDebugReportCallbackEXT = NULL;
PFN_vkCreateDescriptorPool rblxgl_vkCreateDescriptorPool = NULL;
PFN_vkCreateDescriptorSetLayout rblxgl_vkCreateDescriptorSetLayout = NULL;
PFN_vkCreateDescriptorUpdateTemplate rblxgl_vkCreateDescriptorUpdateTemplate = NULL;
PFN_vkCreateDevice rblxgl_vkCreateDevice = NULL;
PFN_vkCreateEvent rblxgl_vkCreateEvent = NULL;
PFN_vkCreateFence rblxgl_vkCreateFence = NULL;
PFN_vkCreateFramebuffer rblxgl_vkCreateFramebuffer = NULL;
PFN_vkCreateGraphicsPipelines rblxgl_vkCreateGraphicsPipelines = NULL;
PFN_vkCreateImage rblxgl_vkCreateImage = NULL;
PFN_vkCreateImageView rblxgl_vkCreateImageView = NULL;
PFN_vkCreateInstance rblxgl_vkCreateInstance = NULL;
PFN_vkCreatePipelineCache rblxgl_vkCreatePipelineCache = NULL;
PFN_vkCreatePipelineLayout rblxgl_vkCreatePipelineLayout = NULL;
PFN_vkCreatePrivateDataSlot rblxgl_vkCreatePrivateDataSlot = NULL;
PFN_vkCreateQueryPool rblxgl_vkCreateQueryPool = NULL;
PFN_vkCreateRenderPass rblxgl_vkCreateRenderPass = NULL;
PFN_vkCreateRenderPass2 rblxgl_vkCreateRenderPass2 = NULL;
PFN_vkCreateSampler rblxgl_vkCreateSampler = NULL;
PFN_vkCreateSamplerYcbcrConversion rblxgl_vkCreateSamplerYcbcrConversion = NULL;
PFN_vkCreateSemaphore rblxgl_vkCreateSemaphore = NULL;
PFN_vkCreateShaderModule rblxgl_vkCreateShaderModule = NULL;
PFN_vkCreateSwapchainKHR rblxgl_vkCreateSwapchainKHR = NULL;
PFN_vkDebugReportMessageEXT rblxgl_vkDebugReportMessageEXT = NULL;
PFN_vkDestroyBuffer rblxgl_vkDestroyBuffer = NULL;
PFN_vkDestroyBufferView rblxgl_vkDestroyBufferView = NULL;
PFN_vkDestroyCommandPool rblxgl_vkDestroyCommandPool = NULL;
PFN_vkDestroyDebugReportCallbackEXT rblxgl_vkDestroyDebugReportCallbackEXT = NULL;
PFN_vkDestroyDescriptorPool rblxgl_vkDestroyDescriptorPool = NULL;
PFN_vkDestroyDescriptorSetLayout rblxgl_vkDestroyDescriptorSetLayout = NULL;
PFN_vkDestroyDescriptorUpdateTemplate rblxgl_vkDestroyDescriptorUpdateTemplate = NULL;
PFN_vkDestroyDevice rblxgl_vkDestroyDevice = NULL;
PFN_vkDestroyEvent rblxgl_vkDestroyEvent = NULL;
PFN_vkDestroyFence rblxgl_vkDestroyFence = NULL;
PFN_vkDestroyFramebuffer rblxgl_vkDestroyFramebuffer = NULL;
PFN_vkDestroyImage rblxgl_vkDestroyImage = NULL;
PFN_vkDestroyImageView rblxgl_vkDestroyImageView = NULL;
PFN_vkDestroyInstance rblxgl_vkDestroyInstance = NULL;
PFN_vkDestroyPipeline rblxgl_vkDestroyPipeline = NULL;
PFN_vkDestroyPipelineCache rblxgl_vkDestroyPipelineCache = NULL;
PFN_vkDestroyPipelineLayout rblxgl_vkDestroyPipelineLayout = NULL;
PFN_vkDestroyPrivateDataSlot rblxgl_vkDestroyPrivateDataSlot = NULL;
PFN_vkDestroyQueryPool rblxgl_vkDestroyQueryPool = NULL;
PFN_vkDestroyRenderPass rblxgl_vkDestroyRenderPass = NULL;
PFN_vkDestroySampler rblxgl_vkDestroySampler = NULL;
PFN_vkDestroySamplerYcbcrConversion rblxgl_vkDestroySamplerYcbcrConversion = NULL;
PFN_vkDestroySemaphore rblxgl_vkDestroySemaphore = NULL;
PFN_vkDestroyShaderModule rblxgl_vkDestroyShaderModule = NULL;
PFN_vkDestroySurfaceKHR rblxgl_vkDestroySurfaceKHR = NULL;
PFN_vkDestroySwapchainKHR rblxgl_vkDestroySwapchainKHR = NULL;
PFN_vkDeviceWaitIdle rblxgl_vkDeviceWaitIdle = NULL;
PFN_vkEndCommandBuffer rblxgl_vkEndCommandBuffer = NULL;
PFN_vkEnumerateDeviceExtensionProperties rblxgl_vkEnumerateDeviceExtensionProperties = NULL;
PFN_vkEnumerateDeviceLayerProperties rblxgl_vkEnumerateDeviceLayerProperties = NULL;
PFN_vkEnumerateInstanceExtensionProperties rblxgl_vkEnumerateInstanceExtensionProperties = NULL;
PFN_vkEnumerateInstanceLayerProperties rblxgl_vkEnumerateInstanceLayerProperties = NULL;
PFN_vkEnumerateInstanceVersion rblxgl_vkEnumerateInstanceVersion = NULL;
PFN_vkEnumeratePhysicalDeviceGroups rblxgl_vkEnumeratePhysicalDeviceGroups = NULL;
PFN_vkEnumeratePhysicalDevices rblxgl_vkEnumeratePhysicalDevices = NULL;
PFN_vkFlushMappedMemoryRanges rblxgl_vkFlushMappedMemoryRanges = NULL;
PFN_vkFreeCommandBuffers rblxgl_vkFreeCommandBuffers = NULL;
PFN_vkFreeDescriptorSets rblxgl_vkFreeDescriptorSets = NULL;
PFN_vkFreeMemory rblxgl_vkFreeMemory = NULL;
PFN_vkGetBufferDeviceAddress rblxgl_vkGetBufferDeviceAddress = NULL;
PFN_vkGetBufferMemoryRequirements rblxgl_vkGetBufferMemoryRequirements = NULL;
PFN_vkGetBufferMemoryRequirements2 rblxgl_vkGetBufferMemoryRequirements2 = NULL;
PFN_vkGetBufferOpaqueCaptureAddress rblxgl_vkGetBufferOpaqueCaptureAddress = NULL;
PFN_vkGetDescriptorSetLayoutSupport rblxgl_vkGetDescriptorSetLayoutSupport = NULL;
PFN_vkGetDeviceBufferMemoryRequirements rblxgl_vkGetDeviceBufferMemoryRequirements = NULL;
PFN_vkGetDeviceGroupPeerMemoryFeatures rblxgl_vkGetDeviceGroupPeerMemoryFeatures = NULL;
PFN_vkGetDeviceGroupPresentCapabilitiesKHR rblxgl_vkGetDeviceGroupPresentCapabilitiesKHR = NULL;
PFN_vkGetDeviceGroupSurfacePresentModesKHR rblxgl_vkGetDeviceGroupSurfacePresentModesKHR = NULL;
PFN_vkGetDeviceImageMemoryRequirements rblxgl_vkGetDeviceImageMemoryRequirements = NULL;
PFN_vkGetDeviceImageSparseMemoryRequirements rblxgl_vkGetDeviceImageSparseMemoryRequirements = NULL;
PFN_vkGetDeviceMemoryCommitment rblxgl_vkGetDeviceMemoryCommitment = NULL;
PFN_vkGetDeviceMemoryOpaqueCaptureAddress rblxgl_vkGetDeviceMemoryOpaqueCaptureAddress = NULL;
PFN_vkGetDeviceProcAddr rblxgl_vkGetDeviceProcAddr = NULL;
PFN_vkGetDeviceQueue rblxgl_vkGetDeviceQueue = NULL;
PFN_vkGetDeviceQueue2 rblxgl_vkGetDeviceQueue2 = NULL;
PFN_vkGetEventStatus rblxgl_vkGetEventStatus = NULL;
PFN_vkGetFenceStatus rblxgl_vkGetFenceStatus = NULL;
PFN_vkGetImageMemoryRequirements rblxgl_vkGetImageMemoryRequirements = NULL;
PFN_vkGetImageMemoryRequirements2 rblxgl_vkGetImageMemoryRequirements2 = NULL;
PFN_vkGetImageSparseMemoryRequirements rblxgl_vkGetImageSparseMemoryRequirements = NULL;
PFN_vkGetImageSparseMemoryRequirements2 rblxgl_vkGetImageSparseMemoryRequirements2 = NULL;
PFN_vkGetImageSubresourceLayout rblxgl_vkGetImageSubresourceLayout = NULL;
PFN_vkGetInstanceProcAddr rblxgl_vkGetInstanceProcAddr = NULL;
PFN_vkGetPhysicalDeviceExternalBufferProperties rblxgl_vkGetPhysicalDeviceExternalBufferProperties = NULL;
PFN_vkGetPhysicalDeviceExternalFenceProperties rblxgl_vkGetPhysicalDeviceExternalFenceProperties = NULL;
PFN_vkGetPhysicalDeviceExternalSemaphoreProperties rblxgl_vkGetPhysicalDeviceExternalSemaphoreProperties = NULL;
PFN_vkGetPhysicalDeviceFeatures rblxgl_vkGetPhysicalDeviceFeatures = NULL;
PFN_vkGetPhysicalDeviceFeatures2 rblxgl_vkGetPhysicalDeviceFeatures2 = NULL;
PFN_vkGetPhysicalDeviceFormatProperties rblxgl_vkGetPhysicalDeviceFormatProperties = NULL;
PFN_vkGetPhysicalDeviceFormatProperties2 rblxgl_vkGetPhysicalDeviceFormatProperties2 = NULL;
PFN_vkGetPhysicalDeviceImageFormatProperties rblxgl_vkGetPhysicalDeviceImageFormatProperties = NULL;
PFN_vkGetPhysicalDeviceImageFormatProperties2 rblxgl_vkGetPhysicalDeviceImageFormatProperties2 = NULL;
PFN_vkGetPhysicalDeviceMemoryProperties rblxgl_vkGetPhysicalDeviceMemoryProperties = NULL;
PFN_vkGetPhysicalDeviceMemoryProperties2 rblxgl_vkGetPhysicalDeviceMemoryProperties2 = NULL;
PFN_vkGetPhysicalDevicePresentRectanglesKHR rblxgl_vkGetPhysicalDevicePresentRectanglesKHR = NULL;
PFN_vkGetPhysicalDeviceProperties rblxgl_vkGetPhysicalDeviceProperties = NULL;
PFN_vkGetPhysicalDeviceProperties2 rblxgl_vkGetPhysicalDeviceProperties2 = NULL;
PFN_vkGetPhysicalDeviceQueueFamilyProperties rblxgl_vkGetPhysicalDeviceQueueFamilyProperties = NULL;
PFN_vkGetPhysicalDeviceQueueFamilyProperties2 rblxgl_vkGetPhysicalDeviceQueueFamilyProperties2 = NULL;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties rblxgl_vkGetPhysicalDeviceSparseImageFormatProperties = NULL;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 rblxgl_vkGetPhysicalDeviceSparseImageFormatProperties2 = NULL;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR rblxgl_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR rblxgl_vkGetPhysicalDeviceSurfaceFormatsKHR = NULL;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR rblxgl_vkGetPhysicalDeviceSurfacePresentModesKHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR rblxgl_vkGetPhysicalDeviceSurfaceSupportKHR = NULL;
PFN_vkGetPhysicalDeviceToolProperties rblxgl_vkGetPhysicalDeviceToolProperties = NULL;
PFN_vkGetPipelineCacheData rblxgl_vkGetPipelineCacheData = NULL;
PFN_vkGetPrivateData rblxgl_vkGetPrivateData = NULL;
PFN_vkGetQueryPoolResults rblxgl_vkGetQueryPoolResults = NULL;
PFN_vkGetRenderAreaGranularity rblxgl_vkGetRenderAreaGranularity = NULL;
PFN_vkGetSemaphoreCounterValue rblxgl_vkGetSemaphoreCounterValue = NULL;
PFN_vkGetSwapchainImagesKHR rblxgl_vkGetSwapchainImagesKHR = NULL;
PFN_vkInvalidateMappedMemoryRanges rblxgl_vkInvalidateMappedMemoryRanges = NULL;
PFN_vkMapMemory rblxgl_vkMapMemory = NULL;
PFN_vkMergePipelineCaches rblxgl_vkMergePipelineCaches = NULL;
PFN_vkQueueBindSparse rblxgl_vkQueueBindSparse = NULL;
PFN_vkQueuePresentKHR rblxgl_vkQueuePresentKHR = NULL;
PFN_vkQueueSubmit rblxgl_vkQueueSubmit = NULL;
PFN_vkQueueSubmit2 rblxgl_vkQueueSubmit2 = NULL;
PFN_vkQueueWaitIdle rblxgl_vkQueueWaitIdle = NULL;
PFN_vkResetCommandBuffer rblxgl_vkResetCommandBuffer = NULL;
PFN_vkResetCommandPool rblxgl_vkResetCommandPool = NULL;
PFN_vkResetDescriptorPool rblxgl_vkResetDescriptorPool = NULL;
PFN_vkResetEvent rblxgl_vkResetEvent = NULL;
PFN_vkResetFences rblxgl_vkResetFences = NULL;
PFN_vkResetQueryPool rblxgl_vkResetQueryPool = NULL;
PFN_vkSetEvent rblxgl_vkSetEvent = NULL;
PFN_vkSetPrivateData rblxgl_vkSetPrivateData = NULL;
PFN_vkSignalSemaphore rblxgl_vkSignalSemaphore = NULL;
PFN_vkTrimCommandPool rblxgl_vkTrimCommandPool = NULL;
PFN_vkUnmapMemory rblxgl_vkUnmapMemory = NULL;
PFN_vkUpdateDescriptorSetWithTemplate rblxgl_vkUpdateDescriptorSetWithTemplate = NULL;
PFN_vkUpdateDescriptorSets rblxgl_vkUpdateDescriptorSets = NULL;
PFN_vkWaitForFences rblxgl_vkWaitForFences = NULL;
PFN_vkWaitSemaphores rblxgl_vkWaitSemaphores = NULL;


static void rblxgl_vk_load_VK_VERSION_1_0( RBLXGLuserptrloadfunc load, void* userptr) {
    if(!RBLXGL_VK_VERSION_1_0) return;
    rblxgl_vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers) load(userptr, "vkAllocateCommandBuffers");
    rblxgl_vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets) load(userptr, "vkAllocateDescriptorSets");
    rblxgl_vkAllocateMemory = (PFN_vkAllocateMemory) load(userptr, "vkAllocateMemory");
    rblxgl_vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer) load(userptr, "vkBeginCommandBuffer");
    rblxgl_vkBindBufferMemory = (PFN_vkBindBufferMemory) load(userptr, "vkBindBufferMemory");
    rblxgl_vkBindImageMemory = (PFN_vkBindImageMemory) load(userptr, "vkBindImageMemory");
    rblxgl_vkCmdBeginQuery = (PFN_vkCmdBeginQuery) load(userptr, "vkCmdBeginQuery");
    rblxgl_vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass) load(userptr, "vkCmdBeginRenderPass");
    rblxgl_vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets) load(userptr, "vkCmdBindDescriptorSets");
    rblxgl_vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer) load(userptr, "vkCmdBindIndexBuffer");
    rblxgl_vkCmdBindPipeline = (PFN_vkCmdBindPipeline) load(userptr, "vkCmdBindPipeline");
    rblxgl_vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers) load(userptr, "vkCmdBindVertexBuffers");
    rblxgl_vkCmdBlitImage = (PFN_vkCmdBlitImage) load(userptr, "vkCmdBlitImage");
    rblxgl_vkCmdClearAttachments = (PFN_vkCmdClearAttachments) load(userptr, "vkCmdClearAttachments");
    rblxgl_vkCmdClearColorImage = (PFN_vkCmdClearColorImage) load(userptr, "vkCmdClearColorImage");
    rblxgl_vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage) load(userptr, "vkCmdClearDepthStencilImage");
    rblxgl_vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer) load(userptr, "vkCmdCopyBuffer");
    rblxgl_vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage) load(userptr, "vkCmdCopyBufferToImage");
    rblxgl_vkCmdCopyImage = (PFN_vkCmdCopyImage) load(userptr, "vkCmdCopyImage");
    rblxgl_vkCmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer) load(userptr, "vkCmdCopyImageToBuffer");
    rblxgl_vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults) load(userptr, "vkCmdCopyQueryPoolResults");
    rblxgl_vkCmdDispatch = (PFN_vkCmdDispatch) load(userptr, "vkCmdDispatch");
    rblxgl_vkCmdDispatchIndirect = (PFN_vkCmdDispatchIndirect) load(userptr, "vkCmdDispatchIndirect");
    rblxgl_vkCmdDraw = (PFN_vkCmdDraw) load(userptr, "vkCmdDraw");
    rblxgl_vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed) load(userptr, "vkCmdDrawIndexed");
    rblxgl_vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect) load(userptr, "vkCmdDrawIndexedIndirect");
    rblxgl_vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect) load(userptr, "vkCmdDrawIndirect");
    rblxgl_vkCmdEndQuery = (PFN_vkCmdEndQuery) load(userptr, "vkCmdEndQuery");
    rblxgl_vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass) load(userptr, "vkCmdEndRenderPass");
    rblxgl_vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands) load(userptr, "vkCmdExecuteCommands");
    rblxgl_vkCmdFillBuffer = (PFN_vkCmdFillBuffer) load(userptr, "vkCmdFillBuffer");
    rblxgl_vkCmdNextSubpass = (PFN_vkCmdNextSubpass) load(userptr, "vkCmdNextSubpass");
    rblxgl_vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier) load(userptr, "vkCmdPipelineBarrier");
    rblxgl_vkCmdPushConstants = (PFN_vkCmdPushConstants) load(userptr, "vkCmdPushConstants");
    rblxgl_vkCmdResetEvent = (PFN_vkCmdResetEvent) load(userptr, "vkCmdResetEvent");
    rblxgl_vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool) load(userptr, "vkCmdResetQueryPool");
    rblxgl_vkCmdResolveImage = (PFN_vkCmdResolveImage) load(userptr, "vkCmdResolveImage");
    rblxgl_vkCmdSetBlendConstants = (PFN_vkCmdSetBlendConstants) load(userptr, "vkCmdSetBlendConstants");
    rblxgl_vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias) load(userptr, "vkCmdSetDepthBias");
    rblxgl_vkCmdSetDepthBounds = (PFN_vkCmdSetDepthBounds) load(userptr, "vkCmdSetDepthBounds");
    rblxgl_vkCmdSetEvent = (PFN_vkCmdSetEvent) load(userptr, "vkCmdSetEvent");
    rblxgl_vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth) load(userptr, "vkCmdSetLineWidth");
    rblxgl_vkCmdSetScissor = (PFN_vkCmdSetScissor) load(userptr, "vkCmdSetScissor");
    rblxgl_vkCmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask) load(userptr, "vkCmdSetStencilCompareMask");
    rblxgl_vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference) load(userptr, "vkCmdSetStencilReference");
    rblxgl_vkCmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask) load(userptr, "vkCmdSetStencilWriteMask");
    rblxgl_vkCmdSetViewport = (PFN_vkCmdSetViewport) load(userptr, "vkCmdSetViewport");
    rblxgl_vkCmdUpdateBuffer = (PFN_vkCmdUpdateBuffer) load(userptr, "vkCmdUpdateBuffer");
    rblxgl_vkCmdWaitEvents = (PFN_vkCmdWaitEvents) load(userptr, "vkCmdWaitEvents");
    rblxgl_vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp) load(userptr, "vkCmdWriteTimestamp");
    rblxgl_vkCreateBuffer = (PFN_vkCreateBuffer) load(userptr, "vkCreateBuffer");
    rblxgl_vkCreateBufferView = (PFN_vkCreateBufferView) load(userptr, "vkCreateBufferView");
    rblxgl_vkCreateCommandPool = (PFN_vkCreateCommandPool) load(userptr, "vkCreateCommandPool");
    rblxgl_vkCreateComputePipelines = (PFN_vkCreateComputePipelines) load(userptr, "vkCreateComputePipelines");
    rblxgl_vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool) load(userptr, "vkCreateDescriptorPool");
    rblxgl_vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout) load(userptr, "vkCreateDescriptorSetLayout");
    rblxgl_vkCreateDevice = (PFN_vkCreateDevice) load(userptr, "vkCreateDevice");
    rblxgl_vkCreateEvent = (PFN_vkCreateEvent) load(userptr, "vkCreateEvent");
    rblxgl_vkCreateFence = (PFN_vkCreateFence) load(userptr, "vkCreateFence");
    rblxgl_vkCreateFramebuffer = (PFN_vkCreateFramebuffer) load(userptr, "vkCreateFramebuffer");
    rblxgl_vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines) load(userptr, "vkCreateGraphicsPipelines");
    rblxgl_vkCreateImage = (PFN_vkCreateImage) load(userptr, "vkCreateImage");
    rblxgl_vkCreateImageView = (PFN_vkCreateImageView) load(userptr, "vkCreateImageView");
    rblxgl_vkCreateInstance = (PFN_vkCreateInstance) load(userptr, "vkCreateInstance");
    rblxgl_vkCreatePipelineCache = (PFN_vkCreatePipelineCache) load(userptr, "vkCreatePipelineCache");
    rblxgl_vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout) load(userptr, "vkCreatePipelineLayout");
    rblxgl_vkCreateQueryPool = (PFN_vkCreateQueryPool) load(userptr, "vkCreateQueryPool");
    rblxgl_vkCreateRenderPass = (PFN_vkCreateRenderPass) load(userptr, "vkCreateRenderPass");
    rblxgl_vkCreateSampler = (PFN_vkCreateSampler) load(userptr, "vkCreateSampler");
    rblxgl_vkCreateSemaphore = (PFN_vkCreateSemaphore) load(userptr, "vkCreateSemaphore");
    rblxgl_vkCreateShaderModule = (PFN_vkCreateShaderModule) load(userptr, "vkCreateShaderModule");
    rblxgl_vkDestroyBuffer = (PFN_vkDestroyBuffer) load(userptr, "vkDestroyBuffer");
    rblxgl_vkDestroyBufferView = (PFN_vkDestroyBufferView) load(userptr, "vkDestroyBufferView");
    rblxgl_vkDestroyCommandPool = (PFN_vkDestroyCommandPool) load(userptr, "vkDestroyCommandPool");
    rblxgl_vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool) load(userptr, "vkDestroyDescriptorPool");
    rblxgl_vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout) load(userptr, "vkDestroyDescriptorSetLayout");
    rblxgl_vkDestroyDevice = (PFN_vkDestroyDevice) load(userptr, "vkDestroyDevice");
    rblxgl_vkDestroyEvent = (PFN_vkDestroyEvent) load(userptr, "vkDestroyEvent");
    rblxgl_vkDestroyFence = (PFN_vkDestroyFence) load(userptr, "vkDestroyFence");
    rblxgl_vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer) load(userptr, "vkDestroyFramebuffer");
    rblxgl_vkDestroyImage = (PFN_vkDestroyImage) load(userptr, "vkDestroyImage");
    rblxgl_vkDestroyImageView = (PFN_vkDestroyImageView) load(userptr, "vkDestroyImageView");
    rblxgl_vkDestroyInstance = (PFN_vkDestroyInstance) load(userptr, "vkDestroyInstance");
    rblxgl_vkDestroyPipeline = (PFN_vkDestroyPipeline) load(userptr, "vkDestroyPipeline");
    rblxgl_vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache) load(userptr, "vkDestroyPipelineCache");
    rblxgl_vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout) load(userptr, "vkDestroyPipelineLayout");
    rblxgl_vkDestroyQueryPool = (PFN_vkDestroyQueryPool) load(userptr, "vkDestroyQueryPool");
    rblxgl_vkDestroyRenderPass = (PFN_vkDestroyRenderPass) load(userptr, "vkDestroyRenderPass");
    rblxgl_vkDestroySampler = (PFN_vkDestroySampler) load(userptr, "vkDestroySampler");
    rblxgl_vkDestroySemaphore = (PFN_vkDestroySemaphore) load(userptr, "vkDestroySemaphore");
    rblxgl_vkDestroyShaderModule = (PFN_vkDestroyShaderModule) load(userptr, "vkDestroyShaderModule");
    rblxgl_vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle) load(userptr, "vkDeviceWaitIdle");
    rblxgl_vkEndCommandBuffer = (PFN_vkEndCommandBuffer) load(userptr, "vkEndCommandBuffer");
    rblxgl_vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) load(userptr, "vkEnumerateDeviceExtensionProperties");
    rblxgl_vkEnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties) load(userptr, "vkEnumerateDeviceLayerProperties");
    rblxgl_vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties) load(userptr, "vkEnumerateInstanceExtensionProperties");
    rblxgl_vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties) load(userptr, "vkEnumerateInstanceLayerProperties");
    rblxgl_vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) load(userptr, "vkEnumeratePhysicalDevices");
    rblxgl_vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges) load(userptr, "vkFlushMappedMemoryRanges");
    rblxgl_vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers) load(userptr, "vkFreeCommandBuffers");
    rblxgl_vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets) load(userptr, "vkFreeDescriptorSets");
    rblxgl_vkFreeMemory = (PFN_vkFreeMemory) load(userptr, "vkFreeMemory");
    rblxgl_vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements) load(userptr, "vkGetBufferMemoryRequirements");
    rblxgl_vkGetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment) load(userptr, "vkGetDeviceMemoryCommitment");
    rblxgl_vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr) load(userptr, "vkGetDeviceProcAddr");
    rblxgl_vkGetDeviceQueue = (PFN_vkGetDeviceQueue) load(userptr, "vkGetDeviceQueue");
    rblxgl_vkGetEventStatus = (PFN_vkGetEventStatus) load(userptr, "vkGetEventStatus");
    rblxgl_vkGetFenceStatus = (PFN_vkGetFenceStatus) load(userptr, "vkGetFenceStatus");
    rblxgl_vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements) load(userptr, "vkGetImageMemoryRequirements");
    rblxgl_vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements) load(userptr, "vkGetImageSparseMemoryRequirements");
    rblxgl_vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout) load(userptr, "vkGetImageSubresourceLayout");
    rblxgl_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) load(userptr, "vkGetInstanceProcAddr");
    rblxgl_vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) load(userptr, "vkGetPhysicalDeviceFeatures");
    rblxgl_vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties) load(userptr, "vkGetPhysicalDeviceFormatProperties");
    rblxgl_vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties) load(userptr, "vkGetPhysicalDeviceImageFormatProperties");
    rblxgl_vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties) load(userptr, "vkGetPhysicalDeviceMemoryProperties");
    rblxgl_vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) load(userptr, "vkGetPhysicalDeviceProperties");
    rblxgl_vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) load(userptr, "vkGetPhysicalDeviceQueueFamilyProperties");
    rblxgl_vkGetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties) load(userptr, "vkGetPhysicalDeviceSparseImageFormatProperties");
    rblxgl_vkGetPipelineCacheData = (PFN_vkGetPipelineCacheData) load(userptr, "vkGetPipelineCacheData");
    rblxgl_vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults) load(userptr, "vkGetQueryPoolResults");
    rblxgl_vkGetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity) load(userptr, "vkGetRenderAreaGranularity");
    rblxgl_vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges) load(userptr, "vkInvalidateMappedMemoryRanges");
    rblxgl_vkMapMemory = (PFN_vkMapMemory) load(userptr, "vkMapMemory");
    rblxgl_vkMergePipelineCaches = (PFN_vkMergePipelineCaches) load(userptr, "vkMergePipelineCaches");
    rblxgl_vkQueueBindSparse = (PFN_vkQueueBindSparse) load(userptr, "vkQueueBindSparse");
    rblxgl_vkQueueSubmit = (PFN_vkQueueSubmit) load(userptr, "vkQueueSubmit");
    rblxgl_vkQueueWaitIdle = (PFN_vkQueueWaitIdle) load(userptr, "vkQueueWaitIdle");
    rblxgl_vkResetCommandBuffer = (PFN_vkResetCommandBuffer) load(userptr, "vkResetCommandBuffer");
    rblxgl_vkResetCommandPool = (PFN_vkResetCommandPool) load(userptr, "vkResetCommandPool");
    rblxgl_vkResetDescriptorPool = (PFN_vkResetDescriptorPool) load(userptr, "vkResetDescriptorPool");
    rblxgl_vkResetEvent = (PFN_vkResetEvent) load(userptr, "vkResetEvent");
    rblxgl_vkResetFences = (PFN_vkResetFences) load(userptr, "vkResetFences");
    rblxgl_vkSetEvent = (PFN_vkSetEvent) load(userptr, "vkSetEvent");
    rblxgl_vkUnmapMemory = (PFN_vkUnmapMemory) load(userptr, "vkUnmapMemory");
    rblxgl_vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets) load(userptr, "vkUpdateDescriptorSets");
    rblxgl_vkWaitForFences = (PFN_vkWaitForFences) load(userptr, "vkWaitForFences");
}
static void rblxgl_vk_load_VK_VERSION_1_1( RBLXGLuserptrloadfunc load, void* userptr) {
    if(!RBLXGL_VK_VERSION_1_1) return;
    rblxgl_vkBindBufferMemory2 = (PFN_vkBindBufferMemory2) load(userptr, "vkBindBufferMemory2");
    rblxgl_vkBindImageMemory2 = (PFN_vkBindImageMemory2) load(userptr, "vkBindImageMemory2");
    rblxgl_vkCmdDispatchBase = (PFN_vkCmdDispatchBase) load(userptr, "vkCmdDispatchBase");
    rblxgl_vkCmdSetDeviceMask = (PFN_vkCmdSetDeviceMask) load(userptr, "vkCmdSetDeviceMask");
    rblxgl_vkCreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate) load(userptr, "vkCreateDescriptorUpdateTemplate");
    rblxgl_vkCreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion) load(userptr, "vkCreateSamplerYcbcrConversion");
    rblxgl_vkDestroyDescriptorUpdateTemplate = (PFN_vkDestroyDescriptorUpdateTemplate) load(userptr, "vkDestroyDescriptorUpdateTemplate");
    rblxgl_vkDestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion) load(userptr, "vkDestroySamplerYcbcrConversion");
    rblxgl_vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion) load(userptr, "vkEnumerateInstanceVersion");
    rblxgl_vkEnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups) load(userptr, "vkEnumeratePhysicalDeviceGroups");
    rblxgl_vkGetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2) load(userptr, "vkGetBufferMemoryRequirements2");
    rblxgl_vkGetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport) load(userptr, "vkGetDescriptorSetLayoutSupport");
    rblxgl_vkGetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures) load(userptr, "vkGetDeviceGroupPeerMemoryFeatures");
    rblxgl_vkGetDeviceQueue2 = (PFN_vkGetDeviceQueue2) load(userptr, "vkGetDeviceQueue2");
    rblxgl_vkGetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2) load(userptr, "vkGetImageMemoryRequirements2");
    rblxgl_vkGetImageSparseMemoryRequirements2 = (PFN_vkGetImageSparseMemoryRequirements2) load(userptr, "vkGetImageSparseMemoryRequirements2");
    rblxgl_vkGetPhysicalDeviceExternalBufferProperties = (PFN_vkGetPhysicalDeviceExternalBufferProperties) load(userptr, "vkGetPhysicalDeviceExternalBufferProperties");
    rblxgl_vkGetPhysicalDeviceExternalFenceProperties = (PFN_vkGetPhysicalDeviceExternalFenceProperties) load(userptr, "vkGetPhysicalDeviceExternalFenceProperties");
    rblxgl_vkGetPhysicalDeviceExternalSemaphoreProperties = (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties) load(userptr, "vkGetPhysicalDeviceExternalSemaphoreProperties");
    rblxgl_vkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2) load(userptr, "vkGetPhysicalDeviceFeatures2");
    rblxgl_vkGetPhysicalDeviceFormatProperties2 = (PFN_vkGetPhysicalDeviceFormatProperties2) load(userptr, "vkGetPhysicalDeviceFormatProperties2");
    rblxgl_vkGetPhysicalDeviceImageFormatProperties2 = (PFN_vkGetPhysicalDeviceImageFormatProperties2) load(userptr, "vkGetPhysicalDeviceImageFormatProperties2");
    rblxgl_vkGetPhysicalDeviceMemoryProperties2 = (PFN_vkGetPhysicalDeviceMemoryProperties2) load(userptr, "vkGetPhysicalDeviceMemoryProperties2");
    rblxgl_vkGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2) load(userptr, "vkGetPhysicalDeviceProperties2");
    rblxgl_vkGetPhysicalDeviceQueueFamilyProperties2 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2) load(userptr, "vkGetPhysicalDeviceQueueFamilyProperties2");
    rblxgl_vkGetPhysicalDeviceSparseImageFormatProperties2 = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2) load(userptr, "vkGetPhysicalDeviceSparseImageFormatProperties2");
    rblxgl_vkTrimCommandPool = (PFN_vkTrimCommandPool) load(userptr, "vkTrimCommandPool");
    rblxgl_vkUpdateDescriptorSetWithTemplate = (PFN_vkUpdateDescriptorSetWithTemplate) load(userptr, "vkUpdateDescriptorSetWithTemplate");
}
static void rblxgl_vk_load_VK_VERSION_1_2( RBLXGLuserptrloadfunc load, void* userptr) {
    if(!RBLXGL_VK_VERSION_1_2) return;
    rblxgl_vkCmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2) load(userptr, "vkCmdBeginRenderPass2");
    rblxgl_vkCmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount) load(userptr, "vkCmdDrawIndexedIndirectCount");
    rblxgl_vkCmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount) load(userptr, "vkCmdDrawIndirectCount");
    rblxgl_vkCmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2) load(userptr, "vkCmdEndRenderPass2");
    rblxgl_vkCmdNextSubpass2 = (PFN_vkCmdNextSubpass2) load(userptr, "vkCmdNextSubpass2");
    rblxgl_vkCreateRenderPass2 = (PFN_vkCreateRenderPass2) load(userptr, "vkCreateRenderPass2");
    rblxgl_vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress) load(userptr, "vkGetBufferDeviceAddress");
    rblxgl_vkGetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress) load(userptr, "vkGetBufferOpaqueCaptureAddress");
    rblxgl_vkGetDeviceMemoryOpaqueCaptureAddress = (PFN_vkGetDeviceMemoryOpaqueCaptureAddress) load(userptr, "vkGetDeviceMemoryOpaqueCaptureAddress");
    rblxgl_vkGetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue) load(userptr, "vkGetSemaphoreCounterValue");
    rblxgl_vkResetQueryPool = (PFN_vkResetQueryPool) load(userptr, "vkResetQueryPool");
    rblxgl_vkSignalSemaphore = (PFN_vkSignalSemaphore) load(userptr, "vkSignalSemaphore");
    rblxgl_vkWaitSemaphores = (PFN_vkWaitSemaphores) load(userptr, "vkWaitSemaphores");
}
static void rblxgl_vk_load_VK_VERSION_1_3( RBLXGLuserptrloadfunc load, void* userptr) {
    if(!RBLXGL_VK_VERSION_1_3) return;
    rblxgl_vkCmdBeginRendering = (PFN_vkCmdBeginRendering) load(userptr, "vkCmdBeginRendering");
    rblxgl_vkCmdBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2) load(userptr, "vkCmdBindVertexBuffers2");
    rblxgl_vkCmdBlitImage2 = (PFN_vkCmdBlitImage2) load(userptr, "vkCmdBlitImage2");
    rblxgl_vkCmdCopyBuffer2 = (PFN_vkCmdCopyBuffer2) load(userptr, "vkCmdCopyBuffer2");
    rblxgl_vkCmdCopyBufferToImage2 = (PFN_vkCmdCopyBufferToImage2) load(userptr, "vkCmdCopyBufferToImage2");
    rblxgl_vkCmdCopyImage2 = (PFN_vkCmdCopyImage2) load(userptr, "vkCmdCopyImage2");
    rblxgl_vkCmdCopyImageToBuffer2 = (PFN_vkCmdCopyImageToBuffer2) load(userptr, "vkCmdCopyImageToBuffer2");
    rblxgl_vkCmdEndRendering = (PFN_vkCmdEndRendering) load(userptr, "vkCmdEndRendering");
    rblxgl_vkCmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2) load(userptr, "vkCmdPipelineBarrier2");
    rblxgl_vkCmdResetEvent2 = (PFN_vkCmdResetEvent2) load(userptr, "vkCmdResetEvent2");
    rblxgl_vkCmdResolveImage2 = (PFN_vkCmdResolveImage2) load(userptr, "vkCmdResolveImage2");
    rblxgl_vkCmdSetCullMode = (PFN_vkCmdSetCullMode) load(userptr, "vkCmdSetCullMode");
    rblxgl_vkCmdSetDepthBiasEnable = (PFN_vkCmdSetDepthBiasEnable) load(userptr, "vkCmdSetDepthBiasEnable");
    rblxgl_vkCmdSetDepthBoundsTestEnable = (PFN_vkCmdSetDepthBoundsTestEnable) load(userptr, "vkCmdSetDepthBoundsTestEnable");
    rblxgl_vkCmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp) load(userptr, "vkCmdSetDepthCompareOp");
    rblxgl_vkCmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable) load(userptr, "vkCmdSetDepthTestEnable");
    rblxgl_vkCmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable) load(userptr, "vkCmdSetDepthWriteEnable");
    rblxgl_vkCmdSetEvent2 = (PFN_vkCmdSetEvent2) load(userptr, "vkCmdSetEvent2");
    rblxgl_vkCmdSetFrontFace = (PFN_vkCmdSetFrontFace) load(userptr, "vkCmdSetFrontFace");
    rblxgl_vkCmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable) load(userptr, "vkCmdSetPrimitiveRestartEnable");
    rblxgl_vkCmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology) load(userptr, "vkCmdSetPrimitiveTopology");
    rblxgl_vkCmdSetRasterizerDiscardEnable = (PFN_vkCmdSetRasterizerDiscardEnable) load(userptr, "vkCmdSetRasterizerDiscardEnable");
    rblxgl_vkCmdSetScissorWithCount = (PFN_vkCmdSetScissorWithCount) load(userptr, "vkCmdSetScissorWithCount");
    rblxgl_vkCmdSetStencilOp = (PFN_vkCmdSetStencilOp) load(userptr, "vkCmdSetStencilOp");
    rblxgl_vkCmdSetStencilTestEnable = (PFN_vkCmdSetStencilTestEnable) load(userptr, "vkCmdSetStencilTestEnable");
    rblxgl_vkCmdSetViewportWithCount = (PFN_vkCmdSetViewportWithCount) load(userptr, "vkCmdSetViewportWithCount");
    rblxgl_vkCmdWaitEvents2 = (PFN_vkCmdWaitEvents2) load(userptr, "vkCmdWaitEvents2");
    rblxgl_vkCmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2) load(userptr, "vkCmdWriteTimestamp2");
    rblxgl_vkCreatePrivateDataSlot = (PFN_vkCreatePrivateDataSlot) load(userptr, "vkCreatePrivateDataSlot");
    rblxgl_vkDestroyPrivateDataSlot = (PFN_vkDestroyPrivateDataSlot) load(userptr, "vkDestroyPrivateDataSlot");
    rblxgl_vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements) load(userptr, "vkGetDeviceBufferMemoryRequirements");
    rblxgl_vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements) load(userptr, "vkGetDeviceImageMemoryRequirements");
    rblxgl_vkGetDeviceImageSparseMemoryRequirements = (PFN_vkGetDeviceImageSparseMemoryRequirements) load(userptr, "vkGetDeviceImageSparseMemoryRequirements");
    rblxgl_vkGetPhysicalDeviceToolProperties = (PFN_vkGetPhysicalDeviceToolProperties) load(userptr, "vkGetPhysicalDeviceToolProperties");
    rblxgl_vkGetPrivateData = (PFN_vkGetPrivateData) load(userptr, "vkGetPrivateData");
    rblxgl_vkQueueSubmit2 = (PFN_vkQueueSubmit2) load(userptr, "vkQueueSubmit2");
    rblxgl_vkSetPrivateData = (PFN_vkSetPrivateData) load(userptr, "vkSetPrivateData");
}
static void rblxgl_vk_load_VK_EXT_debug_report( RBLXGLuserptrloadfunc load, void* userptr) {
    if(!RBLXGL_VK_EXT_debug_report) return;
    rblxgl_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) load(userptr, "vkCreateDebugReportCallbackEXT");
    rblxgl_vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT) load(userptr, "vkDebugReportMessageEXT");
    rblxgl_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) load(userptr, "vkDestroyDebugReportCallbackEXT");
}
static void rblxgl_vk_load_VK_KHR_surface( RBLXGLuserptrloadfunc load, void* userptr) {
    if(!RBLXGL_VK_KHR_surface) return;
    rblxgl_vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR) load(userptr, "vkDestroySurfaceKHR");
    rblxgl_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) load(userptr, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    rblxgl_vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) load(userptr, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    rblxgl_vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) load(userptr, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    rblxgl_vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR) load(userptr, "vkGetPhysicalDeviceSurfaceSupportKHR");
}
static void rblxgl_vk_load_VK_KHR_swapchain( RBLXGLuserptrloadfunc load, void* userptr) {
    if(!RBLXGL_VK_KHR_swapchain) return;
    rblxgl_vkAcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR) load(userptr, "vkAcquireNextImage2KHR");
    rblxgl_vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) load(userptr, "vkAcquireNextImageKHR");
    rblxgl_vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) load(userptr, "vkCreateSwapchainKHR");
    rblxgl_vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR) load(userptr, "vkDestroySwapchainKHR");
    rblxgl_vkGetDeviceGroupPresentCapabilitiesKHR = (PFN_vkGetDeviceGroupPresentCapabilitiesKHR) load(userptr, "vkGetDeviceGroupPresentCapabilitiesKHR");
    rblxgl_vkGetDeviceGroupSurfacePresentModesKHR = (PFN_vkGetDeviceGroupSurfacePresentModesKHR) load(userptr, "vkGetDeviceGroupSurfacePresentModesKHR");
    rblxgl_vkGetPhysicalDevicePresentRectanglesKHR = (PFN_vkGetPhysicalDevicePresentRectanglesKHR) load(userptr, "vkGetPhysicalDevicePresentRectanglesKHR");
    rblxgl_vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) load(userptr, "vkGetSwapchainImagesKHR");
    rblxgl_vkQueuePresentKHR = (PFN_vkQueuePresentKHR) load(userptr, "vkQueuePresentKHR");
}



static int rblxgl_vk_get_extensions( VkPhysicalDevice physical_device, uint32_t *out_extension_count, char ***out_extensions) {
    uint32_t i;
    uint32_t instance_extension_count = 0;
    uint32_t device_extension_count = 0;
    uint32_t max_extension_count = 0;
    uint32_t total_extension_count = 0;
    char **extensions = NULL;
    VkExtensionProperties *ext_properties = NULL;
    VkResult result;

    if (rblxgl_vkEnumerateInstanceExtensionProperties == NULL || (physical_device != NULL && rblxgl_vkEnumerateDeviceExtensionProperties == NULL)) {
        return 0;
    }

    result = rblxgl_vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
    if (result != VK_SUCCESS) {
        return 0;
    }

    if (physical_device != NULL) {
        result = rblxgl_vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extension_count, NULL);
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
        goto rblxgl_vk_get_extensions_error;
    }

    result = rblxgl_vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, ext_properties);
    if (result != VK_SUCCESS) {
        goto rblxgl_vk_get_extensions_error;
    }

    extensions = (char**) calloc(total_extension_count, sizeof(char*));
    if (extensions == NULL) {
        goto rblxgl_vk_get_extensions_error;
    }

    for (i = 0; i < instance_extension_count; ++i) {
        VkExtensionProperties ext = ext_properties[i];

        size_t extension_name_length = strlen(ext.extensionName) + 1;
        extensions[i] = (char*) malloc(extension_name_length * sizeof(char));
        if (extensions[i] == NULL) {
            goto rblxgl_vk_get_extensions_error;
        }
        memcpy(extensions[i], ext.extensionName, extension_name_length * sizeof(char));
    }

    if (physical_device != NULL) {
        result = rblxgl_vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extension_count, ext_properties);
        if (result != VK_SUCCESS) {
            goto rblxgl_vk_get_extensions_error;
        }

        for (i = 0; i < device_extension_count; ++i) {
            VkExtensionProperties ext = ext_properties[i];

            size_t extension_name_length = strlen(ext.extensionName) + 1;
            extensions[instance_extension_count + i] = (char*) malloc(extension_name_length * sizeof(char));
            if (extensions[instance_extension_count + i] == NULL) {
                goto rblxgl_vk_get_extensions_error;
            }
            memcpy(extensions[instance_extension_count + i], ext.extensionName, extension_name_length * sizeof(char));
        }
    }

    free((void*) ext_properties);

    *out_extension_count = total_extension_count;
    *out_extensions = extensions;

    return 1;

rblxgl_vk_get_extensions_error:
    free((void*) ext_properties);
    if (extensions != NULL) {
        for (i = 0; i < total_extension_count; ++i) {
            free((void*) extensions[i]);
        }
        free(extensions);
    }
    return 0;
}

static void rblxgl_vk_free_extensions(uint32_t extension_count, char **extensions) {
    uint32_t i;

    for(i = 0; i < extension_count ; ++i) {
        free((void*) (extensions[i]));
    }

    free((void*) extensions);
}

static int rblxgl_vk_has_extension(const char *name, uint32_t extension_count, char **extensions) {
    uint32_t i;

    for (i = 0; i < extension_count; ++i) {
        if(extensions[i] != NULL && strcmp(name, extensions[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

static RBLXGLapiproc rblxgl_vk_get_proc_from_userptr(void *userptr, const char* name) {
    return (RBLXGL_GNUC_EXTENSION (RBLXGLapiproc (*)(const char *name)) userptr)(name);
}

static int rblxgl_vk_find_extensions_vulkan( VkPhysicalDevice physical_device) {
    uint32_t extension_count = 0;
    char **extensions = NULL;
    if (!rblxgl_vk_get_extensions(physical_device, &extension_count, &extensions)) return 0;

    RBLXGL_VK_EXT_debug_report = rblxgl_vk_has_extension("VK_EXT_debug_report", extension_count, extensions);
    RBLXGL_VK_KHR_portability_enumeration = rblxgl_vk_has_extension("VK_KHR_portability_enumeration", extension_count, extensions);
    RBLXGL_VK_KHR_surface = rblxgl_vk_has_extension("VK_KHR_surface", extension_count, extensions);
    RBLXGL_VK_KHR_swapchain = rblxgl_vk_has_extension("VK_KHR_swapchain", extension_count, extensions);

    (void) rblxgl_vk_has_extension;

    rblxgl_vk_free_extensions(extension_count, extensions);

    return 1;
}

static int rblxgl_vk_find_core_vulkan( VkPhysicalDevice physical_device) {
    int major = 1;
    int minor = 0;

#ifdef VK_VERSION_1_1
    if (rblxgl_vkEnumerateInstanceVersion != NULL) {
        uint32_t version;
        VkResult result;

        result = rblxgl_vkEnumerateInstanceVersion(&version);
        if (result == VK_SUCCESS) {
            major = (int) VK_VERSION_MAJOR(version);
            minor = (int) VK_VERSION_MINOR(version);
        }
    }
#endif

    if (physical_device != NULL && rblxgl_vkGetPhysicalDeviceProperties != NULL) {
        VkPhysicalDeviceProperties properties;
        rblxgl_vkGetPhysicalDeviceProperties(physical_device, &properties);

        major = (int) VK_VERSION_MAJOR(properties.apiVersion);
        minor = (int) VK_VERSION_MINOR(properties.apiVersion);
    }

    RBLXGL_VK_VERSION_1_0 = (major == 1 && minor >= 0) || major > 1;
    RBLXGL_VK_VERSION_1_1 = (major == 1 && minor >= 1) || major > 1;
    RBLXGL_VK_VERSION_1_2 = (major == 1 && minor >= 2) || major > 1;
    RBLXGL_VK_VERSION_1_3 = (major == 1 && minor >= 3) || major > 1;

    return RBLXGL_MAKE_VERSION(major, minor);
}

int rblxglLoadVulkanUserPtr( VkPhysicalDevice physical_device, RBLXGLuserptrloadfunc load, void *userptr) {
    int version;

#ifdef VK_VERSION_1_1
    rblxgl_vkEnumerateInstanceVersion  = (PFN_vkEnumerateInstanceVersion) load(userptr, "vkEnumerateInstanceVersion");
#endif
    version = rblxgl_vk_find_core_vulkan( physical_device);
    if (!version) {
        return 0;
    }

    rblxgl_vk_load_VK_VERSION_1_0(load, userptr);
    rblxgl_vk_load_VK_VERSION_1_1(load, userptr);
    rblxgl_vk_load_VK_VERSION_1_2(load, userptr);
    rblxgl_vk_load_VK_VERSION_1_3(load, userptr);

    if (!rblxgl_vk_find_extensions_vulkan( physical_device)) return 0;
    rblxgl_vk_load_VK_EXT_debug_report(load, userptr);
    rblxgl_vk_load_VK_KHR_surface(load, userptr);
    rblxgl_vk_load_VK_KHR_swapchain(load, userptr);


    return version;
}


int rblxglLoadVulkan( VkPhysicalDevice physical_device, RBLXGLloadfunc load) {
    return rblxglLoadVulkanUserPtr( physical_device, rblxgl_vk_get_proc_from_userptr, RBLXGL_GNUC_EXTENSION (void*) load);
}



 


#ifdef __cplusplus
}
#endif
