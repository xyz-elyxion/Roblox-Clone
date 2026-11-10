// dear rblxui: Renderer Backend for Metal
// This needs to be used along with a Platform Backend (e.g. OSX)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'MTLTexture' as texture identifier. Read the FAQ about RblxTextureID/RblxTextureRef!
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (RblxUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Texture updates support for dynamic font atlas (RblxUIBackendFlags_RendererHasTextures).
//  [X] Renderer: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= RblxUIConfigFlags_ViewportsEnable'.

// You can use unmodified rblxui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire rblxui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear RblxUI:
// - FAQ                  https://dearrblxui.com/faq
// - Getting Started      https://dearrblxui.com/getting-started
// - Documentation        https://dearrblxui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of rblxui.cpp

#pragma once
#include "rblxui.h"      // RBLXUI_IMPL_API
#ifndef RBLXUI_DISABLE

//-----------------------------------------------------------------------------
// ObjC API
//-----------------------------------------------------------------------------

#ifdef __OBJC__

@class MTLRenderPassDescriptor;
@protocol MTLDevice, MTLCommandBuffer, MTLRenderCommandEncoder;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool RblxUI_ImplMetal_Init(id<MTLDevice> device);
RBLXUI_IMPL_API void RblxUI_ImplMetal_Shutdown();
RBLXUI_IMPL_API void RblxUI_ImplMetal_NewFrame(MTLRenderPassDescriptor* renderPassDescriptor);
RBLXUI_IMPL_API void RblxUI_ImplMetal_RenderDrawData(RblxDrawData* drawData,
                                                   id<MTLCommandBuffer> commandBuffer,
                                                   id<MTLRenderCommandEncoder> commandEncoder);

// Called by Init/NewFrame/Shutdown
RBLXUI_IMPL_API bool RblxUI_ImplMetal_CreateDeviceObjects(id<MTLDevice> device);
RBLXUI_IMPL_API void RblxUI_ImplMetal_DestroyDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting RblxDrawData::Textures = nullptr to handle this manually.
RBLXUI_IMPL_API void RblxUI_ImplMetal_UpdateTexture(RblxTextureData* tex);

#endif

//-----------------------------------------------------------------------------
// C++ API
//-----------------------------------------------------------------------------

// Enable Metal C++ binding support with '#define RBLXUI_IMPL_METAL_CPP' in your rblxuiconfig.h file
// More info about using Metal from C++: https://developer.apple.com/metal/cpp/

#ifdef RBLXUI_IMPL_METAL_CPP
#include <Metal/Metal.hpp>
#ifndef __OBJC__

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool RblxUI_ImplMetal_Init(MTL::Device* device);
RBLXUI_IMPL_API void RblxUI_ImplMetal_Shutdown();
RBLXUI_IMPL_API void RblxUI_ImplMetal_NewFrame(MTL::RenderPassDescriptor* renderPassDescriptor);
RBLXUI_IMPL_API void RblxUI_ImplMetal_RenderDrawData(RblxDrawData* draw_data,
                                                   MTL::CommandBuffer* commandBuffer,
                                                   MTL::RenderCommandEncoder* commandEncoder);

// Called by Init/NewFrame/Shutdown
RBLXUI_IMPL_API bool RblxUI_ImplMetal_CreateDeviceObjects(MTL::Device* device);
RBLXUI_IMPL_API void RblxUI_ImplMetal_DestroyDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting RblxDrawData::Textures = nullptr to handle this manually.
RBLXUI_IMPL_API void RblxUI_ImplMetal_UpdateTexture(RblxTextureData* tex);

#endif
#endif

//-----------------------------------------------------------------------------

#endif // #ifndef RBLXUI_DISABLE
