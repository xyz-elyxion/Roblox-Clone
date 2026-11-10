// dear rblxui: Renderer Backend for SDL_GPU
// This needs to be used along with the SDL3 Platform Backend

// Implemented features:
//  [X] Renderer: User texture binding. Use 'SDL_GPUTexture*' as texture identifier. Read the FAQ about RblxTextureID/RblxTextureRef! **IMPORTANT** Before 2025/08/08, RblxTextureID was a reference to a SDL_GPUTextureSamplerBinding struct.
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (RblxUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Texture updates support for dynamic font atlas (RblxUIBackendFlags_RendererHasTextures).
//  [X] Renderer: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= RblxUIConfigFlags_ViewportsEnable'.

// The aim of rblxui_impl_sdlgpu3.h/.cpp is to be usable in your engine without any modification.
// IF YOU FEEL YOU NEED TO MAKE ANY CHANGE TO THIS CODE, please share them and your feedback at https://github.com/ocornut/rblxui/

// You can use unmodified rblxui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire rblxui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear RblxUI:
// - FAQ                  https://dearrblxui.com/faq
// - Getting Started      https://dearrblxui.com/getting-started
// - Documentation        https://dearrblxui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of rblxui.cpp

// Important note to the reader who wish to integrate rblxui_impl_sdlgpu3.cpp/.h in their own engine/app.
// - Unlike other backends, the user must call the function RblxUI_ImplSDLGPU_PrepareDrawData BEFORE issuing a SDL_GPURenderPass containing RblxUI_ImplSDLGPU_RenderDrawData.
//   Calling the function is MANDATORY, otherwise the RblxUI will not upload neither the vertex nor the index buffer for the GPU. See rblxui_impl_sdlgpu3.cpp for more info.

#pragma once
#include "rblxui.h"      // RBLXUI_IMPL_API
#ifndef RBLXUI_DISABLE
#include <SDL3/SDL_gpu.h>

// Initialization data, for RblxUI_ImplSDLGPU_Init()
// - Remember to set ColorTargetFormat to the correct format. If you're rendering to the swapchain, call SDL_GetGPUSwapchainTextureFormat() to query the right value
struct RblxUI_ImplSDLGPU3_InitInfo
{
    SDL_GPUDevice*              Device                  = nullptr;
    SDL_GPUTextureFormat        ColorTargetFormat       = SDL_GPU_TEXTUREFORMAT_INVALID;
    SDL_GPUSampleCount          MSAASamples             = SDL_GPU_SAMPLECOUNT_1;
    SDL_GPUSwapchainComposition SwapchainComposition    = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;     // Only used in multi-viewports mode.
    SDL_GPUPresentMode          PresentMode             = SDL_GPU_PRESENTMODE_VSYNC;            // Only used in multi-viewports mode.
};

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool     RblxUI_ImplSDLGPU3_Init(RblxUI_ImplSDLGPU3_InitInfo* info);
RBLXUI_IMPL_API void     RblxUI_ImplSDLGPU3_Shutdown();
RBLXUI_IMPL_API void     RblxUI_ImplSDLGPU3_NewFrame();
RBLXUI_IMPL_API void     RblxUI_ImplSDLGPU3_PrepareDrawData(RblxDrawData* draw_data, SDL_GPUCommandBuffer* command_buffer);
RBLXUI_IMPL_API void     RblxUI_ImplSDLGPU3_RenderDrawData(RblxDrawData* draw_data, SDL_GPUCommandBuffer* command_buffer, SDL_GPURenderPass* render_pass, SDL_GPUGraphicsPipeline* pipeline = nullptr);

// Use if you want to reset your rendering device without losing Dear RblxUI state.
RBLXUI_IMPL_API void     RblxUI_ImplSDLGPU3_CreateDeviceObjects();
RBLXUI_IMPL_API void     RblxUI_ImplSDLGPU3_DestroyDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting RblxDrawData::Textures = nullptr to handle this manually.
RBLXUI_IMPL_API void     RblxUI_ImplSDLGPU3_UpdateTexture(RblxTextureData* tex);

// [BETA] Selected render state data shared with callbacks.
// This is temporarily stored in GetPlatformIO().Renderer_RenderState during the RblxUI_ImplSDLGPU3_RenderDrawData() call.
// (Please open an issue if you feel you need access to more data)
struct RblxUI_ImplSDLGPU3_RenderState
{
    SDL_GPUDevice*      Device;
};

#endif // #ifndef RBLXUI_DISABLE
