// dear linearui: Renderer Backend for Metal
// This needs to be used along with a Platform Backend (e.g. OSX)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'MTLTexture' as texture identifier. Read the FAQ about LinearTextureID/LinearTextureRef!
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (LinearUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Texture updates support for dynamic font atlas (LinearUIBackendFlags_RendererHasTextures).
//  [X] Renderer: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= LinearUIConfigFlags_ViewportsEnable'.

// You can use unmodified linearui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire linearui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear LinearUI:
// - FAQ                  https://dearlinearui.com/faq
// - Getting Started      https://dearlinearui.com/getting-started
// - Documentation        https://dearlinearui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of linearui.cpp

#pragma once
#include "linearui.h"      // LINEARUI_IMPL_API
#ifndef LINEARUI_DISABLE

//-----------------------------------------------------------------------------
// ObjC API
//-----------------------------------------------------------------------------

#ifdef __OBJC__

@class MTLRenderPassDescriptor;
@protocol MTLDevice, MTLCommandBuffer, MTLRenderCommandEncoder;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
LINEARUI_IMPL_API bool LinearUI_ImplMetal_Init(id<MTLDevice> device);
LINEARUI_IMPL_API void LinearUI_ImplMetal_Shutdown();
LINEARUI_IMPL_API void LinearUI_ImplMetal_NewFrame(MTLRenderPassDescriptor* renderPassDescriptor);
LINEARUI_IMPL_API void LinearUI_ImplMetal_RenderDrawData(LinearDrawData* drawData,
                                                   id<MTLCommandBuffer> commandBuffer,
                                                   id<MTLRenderCommandEncoder> commandEncoder);

// Called by Init/NewFrame/Shutdown
LINEARUI_IMPL_API bool LinearUI_ImplMetal_CreateDeviceObjects(id<MTLDevice> device);
LINEARUI_IMPL_API void LinearUI_ImplMetal_DestroyDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting LinearDrawData::Textures = nullptr to handle this manually.
LINEARUI_IMPL_API void LinearUI_ImplMetal_UpdateTexture(LinearTextureData* tex);

#endif

//-----------------------------------------------------------------------------
// C++ API
//-----------------------------------------------------------------------------

// Enable Metal C++ binding support with '#define LINEARUI_IMPL_METAL_CPP' in your linearuiconfig.h file
// More info about using Metal from C++: https://developer.apple.com/metal/cpp/

#ifdef LINEARUI_IMPL_METAL_CPP
#include <Metal/Metal.hpp>
#ifndef __OBJC__

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
LINEARUI_IMPL_API bool LinearUI_ImplMetal_Init(MTL::Device* device);
LINEARUI_IMPL_API void LinearUI_ImplMetal_Shutdown();
LINEARUI_IMPL_API void LinearUI_ImplMetal_NewFrame(MTL::RenderPassDescriptor* renderPassDescriptor);
LINEARUI_IMPL_API void LinearUI_ImplMetal_RenderDrawData(LinearDrawData* draw_data,
                                                   MTL::CommandBuffer* commandBuffer,
                                                   MTL::RenderCommandEncoder* commandEncoder);

// Called by Init/NewFrame/Shutdown
LINEARUI_IMPL_API bool LinearUI_ImplMetal_CreateDeviceObjects(MTL::Device* device);
LINEARUI_IMPL_API void LinearUI_ImplMetal_DestroyDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting LinearDrawData::Textures = nullptr to handle this manually.
LINEARUI_IMPL_API void LinearUI_ImplMetal_UpdateTexture(LinearTextureData* tex);

#endif
#endif

//-----------------------------------------------------------------------------

#endif // #ifndef LINEARUI_DISABLE
