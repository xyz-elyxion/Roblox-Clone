// dear rblxui: Renderer Backend for Vulkan
// This needs to be used along with a Platform Backend (e.g. RBLXWIN, SDL, Win32, custom..)

// Implemented features:
//  [!] Renderer: User texture binding. Use a VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE 'VkDescriptorSet' as texture identifier. Call RblxUI_ImplVulkan_AddTexture() to register one. Read the FAQ about RblxTextureID/RblxTextureRef + https://github.com/ocornut/rblxui/pull/914 for discussions.
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (RblxUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Texture updates support for dynamic font atlas (RblxUIBackendFlags_RendererHasTextures).
//  [X] Renderer: Expose selected render state for draw callbacks to use. Access in '(RblxUI_ImplXXXX_RenderState*)GetPlatformIO().Renderer_RenderState'.
//  [x] Renderer: Multi-viewport / platform windows. With issues (flickering when creating a new viewport).

// The aim of rblxui_impl_vulkan.h/.cpp is to be usable in your engine without any modification.
// IF YOU FEEL YOU NEED TO MAKE ANY CHANGE TO THIS CODE, please share them and your feedback at https://github.com/ocornut/rblxui/

// You can use unmodified rblxui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire rblxui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear RblxUI:
// - FAQ                  https://dearrblxui.com/faq
// - Getting Started      https://dearrblxui.com/getting-started
// - Documentation        https://dearrblxui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of rblxui.cpp

// Important note to the reader who wish to integrate rblxui_impl_vulkan.cpp/.h in their own engine/app.
// - Common RblxUI_ImplVulkan_XXX functions and structures are used to interface with rblxui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your engine/app.
// - Helper RblxUI_ImplVulkanH_XXX functions and structures are only used by this example (main.cpp) and by
//   the backend itself (rblxui_impl_vulkan.cpp), but should PROBABLY NOT be used by your own engine/app code.
// Read comments in rblxui_impl_vulkan.h.

#pragma once
#ifndef RBLXUI_DISABLE
#include "rblxui.h"      // RBLXUI_IMPL_API

// [Configuration] in order to use a custom Vulkan function loader:
// (1) You'll need to disable default Vulkan function prototypes.
//     We provide a '#define RBLXUI_IMPL_VULKAN_NO_PROTOTYPES' convenience configuration flag.
//     In order to make sure this is visible from the rblxui_impl_vulkan.cpp compilation unit:
//     - Add '#define RBLXUI_IMPL_VULKAN_NO_PROTOTYPES' in your rblxuiconfig.h file
//     - Or as a compilation flag in your build system
//     - Or uncomment here (not recommended because you'd be modifying rblxui sources!)
//     - Do not simply add it in a .cpp file!
// (2) Call RblxUI_ImplVulkan_LoadFunctions() before RblxUI_ImplVulkan_Init() with your custom function.
// If you have no idea what this is, leave it alone!
//#define RBLXUI_IMPL_VULKAN_NO_PROTOTYPES

// [Configuration] Convenience support for Volk
// (you can also technically use RBLXUI_IMPL_VULKAN_NO_PROTOTYPES + wrap Volk via RblxUI_ImplVulkan_LoadFunctions().)
// (When using Volk from directory outside your include directories list you can specify full path to the volk.h header,
//  for example when using Volk from VulkanSDK and using include_directories(${Vulkan_INCLUDE_DIRS})' from 'find_package(Vulkan REQUIRED)')
//#define RBLXUI_IMPL_VULKAN_USE_VOLK
//#define RBLXUI_IMPL_VULKAN_VOLK_FILENAME    <Volk/volk.h>
//#define RBLXUI_IMPL_VULKAN_VOLK_FILENAME    <volk.h>       // Default
// Reminder: make those changes in your rblxuiconfig.h file, not here!

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast" // warning: use of old-style cast
#endif

#if defined(RBLXUI_IMPL_VULKAN_NO_PROTOTYPES) && !defined(VK_NO_PROTOTYPES)
#define VK_NO_PROTOTYPES
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR) && !defined(NOMINMAX)
#define NOMINMAX
#endif

// Vulkan includes
#ifdef RBLXUI_IMPL_VULKAN_USE_VOLK
#ifdef RBLXUI_IMPL_VULKAN_VOLK_FILENAME
#include RBLXUI_IMPL_VULKAN_VOLK_FILENAME
#else
#include <volk.h>
#endif
#else
#include <vulkan/vulkan.h>
#endif
#if defined(VK_VERSION_1_3) || defined(VK_KHR_dynamic_rendering)
#define RBLXUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
#endif

// Backend uses a small number of descriptors per font atlas + as many as additional calls done to RblxUI_ImplVulkan_AddTexture().
#define RBLXUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE   (8)     // Minimum per atlas
#define RBLXUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE         (2)     // Minimum for linear + nearest

// Specify settings to create pipeline and swapchain
struct RblxUI_ImplVulkan_PipelineInfo
{
    // For Main viewport only
    VkRenderPass                    RenderPass;                     // Ignored if using dynamic rendering

    // For Main and Secondary viewports
    uint32_t                        Subpass;                        //
    VkSampleCountFlagBits           MSAASamples = {};               // 0 defaults to VK_SAMPLE_COUNT_1_BIT
    RblxVector<VkDynamicState>        ExtraDynamicStates;             // Optional, allows to insert more dynamic states into our VkPipeline
#ifdef RBLXUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
    VkPipelineRenderingCreateInfoKHR PipelineRenderingCreateInfo;   // Optional, valid if .sType == VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR
#endif

    // For Secondary viewports only (created/managed by backend)
    VkImageUsageFlags               SwapChainImageUsage;            // Extra flags for vkCreateSwapchainKHR() calls for secondary viewports. We automatically add VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT. You can add e.g. VK_IMAGE_USAGE_TRANSFER_SRC_BIT if you need to capture from viewports.
};

// Initialization data, for RblxUI_ImplVulkan_Init()
// [Please zero-clear before use!]
// - About descriptor pool:
//   - A VkDescriptorPool should be created with VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
//     and must contain a pool size large enough to hold a small number of VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER descriptors.
//   - As an convenience, by setting DescriptorPoolSize > 0 the backend will create one for you.
// - About dynamic rendering:
//   - When using dynamic rendering, set UseDynamicRendering=true + fill PipelineInfoMain.PipelineRenderingCreateInfo structure.
struct RblxUI_ImplVulkan_InitInfo
{
    uint32_t                        ApiVersion;                 // Fill with API version of Instance, e.g. VK_API_VERSION_1_3 or your value of VkApplicationInfo::apiVersion. May be lower than header version (VK_HEADER_VERSION_COMPLETE)
    VkInstance                      Instance;
    VkPhysicalDevice                PhysicalDevice;
    VkDevice                        Device;
    uint32_t                        QueueFamily;
    VkQueue                         Queue;
    VkDescriptorPool                DescriptorPool;             // See requirements in note above; ignored if using DescriptorPoolSize > 0
    uint32_t                        DescriptorPoolSize;         // Optional: set to create internal ImageView descriptor pool automatically instead of using DescriptorPool.
    uint32_t                        MinImageCount;              // >= 2
    uint32_t                        ImageCount;                 // >= MinImageCount
    VkPipelineCache                 PipelineCache;              // Optional

    // Pipeline
    RblxUI_ImplVulkan_PipelineInfo   PipelineInfoMain;           // Infos for Main Viewport (created by app/user)
    RblxUI_ImplVulkan_PipelineInfo   PipelineInfoForViewports;   // Infos for Secondary Viewports (created by backend)
    //VkRenderPass                  RenderPass;                 // --> Since 2025/09/26: set 'PipelineInfoMain.RenderPass' instead
    //uint32_t                      Subpass;                    // --> Since 2025/09/26: set 'PipelineInfoMain.Subpass' instead
    //VkSampleCountFlagBits         MSAASamples;                // --> Since 2025/09/26: set 'PipelineInfoMain.MSAASamples' instead
    //VkPipelineRenderingCreateInfoKHR PipelineRenderingCreateInfo; // Since 2025/09/26: set 'PipelineInfoMain.PipelineRenderingCreateInfo' instead

    // (Optional) Dynamic Rendering
    // Need to explicitly enable VK_KHR_dynamic_rendering extension to use this, even for Vulkan 1.3 + setup PipelineInfoMain.PipelineRenderingCreateInfo and PipelineInfoViewports.PipelineRenderingCreateInfo.
    bool                            UseDynamicRendering;

    // (Optional) Allocation, Debugging
    const VkAllocationCallbacks*    Allocator;
    void                            (*CheckVkResultFn)(VkResult err);
    VkDeviceSize                    MinAllocationSize;          // Minimum allocation size. Set to 1024*1024 to satisfy zealous best practices validation layer and waste a little memory.

    // (Optional) Customize default vertex/fragment shaders.
    // - if .sType == VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO we use specified structs, otherwise we use defaults.
    // - Shader inputs/outputs need to match ours. Code/data pointed to by the structure needs to survive for whole during of backend usage.
    VkShaderModuleCreateInfo        CustomShaderVertCreateInfo;
    VkShaderModuleCreateInfo        CustomShaderFragCreateInfo;
};

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool             RblxUI_ImplVulkan_Init(RblxUI_ImplVulkan_InitInfo* info);
RBLXUI_IMPL_API void             RblxUI_ImplVulkan_Shutdown();
RBLXUI_IMPL_API void             RblxUI_ImplVulkan_NewFrame();
RBLXUI_IMPL_API void             RblxUI_ImplVulkan_RenderDrawData(RblxDrawData* draw_data, VkCommandBuffer command_buffer, VkPipeline pipeline = VK_NULL_HANDLE);
RBLXUI_IMPL_API void             RblxUI_ImplVulkan_SetMinImageCount(uint32_t min_image_count); // To override MinImageCount after initialization (e.g. if swap chain is recreated)

// (Advanced) Use e.g. if you need to recreate pipeline without reinitializing the backend (see #8110, #8111)
// The main window pipeline will be created by RblxUI_ImplVulkan_Init() if possible (== RenderPass xor (UseDynamicRendering && PipelineRenderingCreateInfo->sType == VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR))
// Else, the pipeline can be created, or re-created, using RblxUI_ImplVulkan_CreateMainPipeline() before rendering.
RBLXUI_IMPL_API void             RblxUI_ImplVulkan_CreateMainPipeline(const RblxUI_ImplVulkan_PipelineInfo* info);

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting RblxDrawData::Textures = nullptr to handle this manually.
RBLXUI_IMPL_API void             RblxUI_ImplVulkan_UpdateTexture(RblxTextureData* tex);

// Register a texture (VkDescriptorSet for a VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE == RblxTextureID)
RBLXUI_IMPL_API VkDescriptorSet  RblxUI_ImplVulkan_AddTexture(VkImageView image_view, VkImageLayout image_layout);
RBLXUI_IMPL_API void             RblxUI_ImplVulkan_RemoveTexture(VkDescriptorSet descriptor_set);

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
RBLXUI_IMPL_API VkDescriptorSet  RblxUI_ImplVulkan_AddTexture(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout); // Ignore VkSampler
#endif

// Optional: load Vulkan functions with a custom function loader
// This is only useful with RBLXUI_IMPL_VULKAN_NO_PROTOTYPES / VK_NO_PROTOTYPES
RBLXUI_IMPL_API bool             RblxUI_ImplVulkan_LoadFunctions(uint32_t api_version, PFN_vkVoidFunction(*loader_func)(const char* function_name, void* user_data), void* user_data = nullptr);

// [BETA] Selected render state data shared with callbacks.
// This is temporarily stored in GetPlatformIO().Renderer_RenderState during the RblxUI_ImplVulkan_RenderDrawData() call.
// (Please open an issue if you feel you need access to more data)
struct RblxUI_ImplVulkan_RenderState
{
    VkCommandBuffer     CommandBuffer;
    VkPipeline          Pipeline;
    VkPipelineLayout    PipelineLayout;
};

//-------------------------------------------------------------------------
// Internal / Miscellaneous Vulkan Helpers
//-------------------------------------------------------------------------
// Used by example's main.cpp. Used by multi-viewport features. PROBABLY NOT used by your own engine/app.
//
// You probably do NOT need to use or care about those functions.
// WE DO NOT PROVIDE STRONG GUARANTEES OF BACKWARD/FORWARD COMPATIBILITY.
// Those functions only exist because:
//   1) they facilitate the readability and maintenance of the multiple main.cpp examples files.
//   2) the multi-viewport / platform window implementation needs them internally.
// Generally we avoid exposing any kind of superfluous high-level helpers in the backends,
// but it is too much code to duplicate everywhere so we exceptionally expose them.
//
// Your engine/app will likely _already_ have code to setup all that stuff (swap chain,
// render pass, frame buffers, etc.). You may read this code if you are curious, but
// it is recommended you use your own custom tailored code to do equivalent work.
//
// The RblxUI_ImplVulkanH_XXX functions should NOT interact with any of the state used
// by the regular RblxUI_ImplVulkan_XXX functions.
//-------------------------------------------------------------------------

struct RblxUI_ImplVulkanH_Frame;
struct RblxUI_ImplVulkanH_Window;

// Helpers
RBLXUI_IMPL_API void                 RblxUI_ImplVulkanH_CreateOrResizeWindow(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, RblxUI_ImplVulkanH_Window* wd, uint32_t queue_family, const VkAllocationCallbacks* allocator, int w, int h, uint32_t min_image_count, VkImageUsageFlags image_usage);
RBLXUI_IMPL_API void                 RblxUI_ImplVulkanH_DestroyWindow(VkInstance instance, VkDevice device, RblxUI_ImplVulkanH_Window* wd, const VkAllocationCallbacks* allocator);
RBLXUI_IMPL_API VkSurfaceFormatKHR   RblxUI_ImplVulkanH_SelectSurfaceFormat(VkPhysicalDevice physical_device, VkSurfaceKHR surface, const VkFormat* request_formats, int request_formats_count, VkColorSpaceKHR request_color_space);
RBLXUI_IMPL_API VkPresentModeKHR     RblxUI_ImplVulkanH_SelectPresentMode(VkPhysicalDevice physical_device, VkSurfaceKHR surface, const VkPresentModeKHR* request_modes, int request_modes_count);
RBLXUI_IMPL_API VkPhysicalDevice     RblxUI_ImplVulkanH_SelectPhysicalDevice(VkInstance instance);
RBLXUI_IMPL_API uint32_t             RblxUI_ImplVulkanH_SelectQueueFamilyIndex(VkPhysicalDevice physical_device);
RBLXUI_IMPL_API int                  RblxUI_ImplVulkanH_GetMinImageCountFromPresentMode(VkPresentModeKHR present_mode);
RBLXUI_IMPL_API RblxUI_ImplVulkanH_Window* RblxUI_ImplVulkanH_GetWindowDataFromViewport(RblxUIViewport* viewport); // Access to Vulkan objects associated with a viewport (e.g to export a screenshot)

// Helper structure to hold the data needed by one rendering frame
// (Used by example's main.cpp. Used by multi-viewport features. Probably NOT used by your own engine/app.)
// [Please zero-clear before use!]
struct RblxUI_ImplVulkanH_Frame
{
    VkCommandPool       CommandPool;
    VkCommandBuffer     CommandBuffer;
    VkFence             Fence;
    VkImage             Backbuffer;
    VkImageView         BackbufferView;
    VkFramebuffer       Framebuffer;
};

struct RblxUI_ImplVulkanH_FrameSemaphores
{
    VkSemaphore         ImageAcquiredSemaphore;
    VkSemaphore         RenderCompleteSemaphore;
};

// Helper structure to hold the data needed by one rendering context into one OS window
// (Used by example's main.cpp. Used by multi-viewport features. Probably NOT used by your own engine/app.)
struct RblxUI_ImplVulkanH_Window
{
    // Input
    bool                    UseDynamicRendering;
    VkSurfaceKHR            Surface;            // Surface created and destroyed by caller.
    VkSurfaceFormatKHR      SurfaceFormat;
    VkPresentModeKHR        PresentMode;
    VkAttachmentDescription AttachmentDesc;     // RenderPass creation: main attachment description.
    VkClearValue            ClearValue;         // RenderPass creation: clear value when using VK_ATTACHMENT_LOAD_OP_CLEAR.

    // Internal
    int                     Width;              // Generally same as passed to RblxUI_ImplVulkanH_CreateOrResizeWindow()
    int                     Height;
    VkSwapchainKHR          Swapchain;
    VkRenderPass            RenderPass;
    uint32_t                FrameIndex;         // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
    uint32_t                ImageCount;         // Number of simultaneous in-flight frames (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count)
    uint32_t                SemaphoreCount;     // Number of simultaneous in-flight frames + 1, to be able to use it in vkAcquireNextImageKHR
    uint32_t                SemaphoreIndex;     // Current set of swapchain wait semaphores we're using (needs to be distinct from per frame data)
    RblxVector<RblxUI_ImplVulkanH_Frame>           Frames;
    RblxVector<RblxUI_ImplVulkanH_FrameSemaphores> FrameSemaphores;

    RblxUI_ImplVulkanH_Window()
    {
        memset((void*)this, 0, sizeof(*this));

        // Parameters to create SwapChain
        PresentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;     // Ensure we get an error if user doesn't set this.

        // Parameters to create RenderPass
        AttachmentDesc.format = VK_FORMAT_UNDEFINED;    // Will automatically use wd->SurfaceFormat.format.
        AttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        AttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        AttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        AttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        AttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        AttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        AttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
};

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef RBLXUI_DISABLE
