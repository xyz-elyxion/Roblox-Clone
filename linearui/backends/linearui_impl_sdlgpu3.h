// dear linearui: Renderer Backend for SDL_GPU
// This needs to be used along with the SDL3 Platform Backend

// Implemented features:
//  [X] Renderer: User texture binding. Use 'SDL_GPUTexture*' as texture identifier. Read the FAQ about LinearTextureID/LinearTextureRef! **IMPORTANT** Before 2025/08/08, LinearTextureID was a reference to a SDL_GPUTextureSamplerBinding struct.
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (LinearUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Texture updates support for dynamic font atlas (LinearUIBackendFlags_RendererHasTextures).
//  [X] Renderer: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= LinearUIConfigFlags_ViewportsEnable'.

// The aim of linearui_impl_sdlgpu3.h/.cpp is to be usable in your engine without any modification.
// IF YOU FEEL YOU NEED TO MAKE ANY CHANGE TO THIS CODE, please share them and your feedback at https://github.com/ocornut/linearui/

// You can use unmodified linearui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire linearui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear LinearUI:
// - FAQ                  https://dearlinearui.com/faq
// - Getting Started      https://dearlinearui.com/getting-started
// - Documentation        https://dearlinearui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of linearui.cpp

// Important note to the reader who wish to integrate linearui_impl_sdlgpu3.cpp/.h in their own engine/app.
// - Unlike other backends, the user must call the function LinearUI_ImplSDLGPU_PrepareDrawData BEFORE issuing a SDL_GPURenderPass containing LinearUI_ImplSDLGPU_RenderDrawData.
//   Calling the function is MANDATORY, otherwise the LinearUI will not upload neither the vertex nor the index buffer for the GPU. See linearui_impl_sdlgpu3.cpp for more info.

#pragma once
#include "linearui.h"      // LINEARUI_IMPL_API
#ifndef LINEARUI_DISABLE
#include <SDL3/SDL_gpu.h>

// Initialization data, for LinearUI_ImplSDLGPU_Init()
// - Remember to set ColorTargetFormat to the correct format. If you're rendering to the swapchain, call SDL_GetGPUSwapchainTextureFormat() to query the right value
struct LinearUI_ImplSDLGPU3_InitInfo
{
    SDL_GPUDevice*              Device                  = nullptr;
    SDL_GPUTextureFormat        ColorTargetFormat       = SDL_GPU_TEXTUREFORMAT_INVALID;
    SDL_GPUSampleCount          MSAASamples             = SDL_GPU_SAMPLECOUNT_1;
    SDL_GPUSwapchainComposition SwapchainComposition    = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;     // Only used in multi-viewports mode.
    SDL_GPUPresentMode          PresentMode             = SDL_GPU_PRESENTMODE_VSYNC;            // Only used in multi-viewports mode.
};

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
LINEARUI_IMPL_API bool     LinearUI_ImplSDLGPU3_Init(LinearUI_ImplSDLGPU3_InitInfo* info);
LINEARUI_IMPL_API void     LinearUI_ImplSDLGPU3_Shutdown();
LINEARUI_IMPL_API void     LinearUI_ImplSDLGPU3_NewFrame();
LINEARUI_IMPL_API void     LinearUI_ImplSDLGPU3_PrepareDrawData(LinearDrawData* draw_data, SDL_GPUCommandBuffer* command_buffer);
LINEARUI_IMPL_API void     LinearUI_ImplSDLGPU3_RenderDrawData(LinearDrawData* draw_data, SDL_GPUCommandBuffer* command_buffer, SDL_GPURenderPass* render_pass, SDL_GPUGraphicsPipeline* pipeline = nullptr);

// Use if you want to reset your rendering device without losing Dear LinearUI state.
LINEARUI_IMPL_API void     LinearUI_ImplSDLGPU3_CreateDeviceObjects();
LINEARUI_IMPL_API void     LinearUI_ImplSDLGPU3_DestroyDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting LinearDrawData::Textures = nullptr to handle this manually.
LINEARUI_IMPL_API void     LinearUI_ImplSDLGPU3_UpdateTexture(LinearTextureData* tex);

// [BETA] Selected render state data shared with callbacks.
// This is temporarily stored in GetPlatformIO().Renderer_RenderState during the LinearUI_ImplSDLGPU3_RenderDrawData() call.
// (Please open an issue if you feel you need access to more data)
struct LinearUI_ImplSDLGPU3_RenderState
{
    SDL_GPUDevice*      Device;
};

#endif // #ifndef LINEARUI_DISABLE
