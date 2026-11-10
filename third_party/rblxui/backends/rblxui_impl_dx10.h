// dear rblxui: Renderer Backend for DirectX10
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'ID3D10ShaderResourceView*' as texture identifier. Read the FAQ about RblxTextureID/RblxTextureRef!
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

struct ID3D10Device;
struct ID3D10SamplerState;
struct ID3D10Buffer;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool     RblxUI_ImplDX10_Init(ID3D10Device* device);
RBLXUI_IMPL_API void     RblxUI_ImplDX10_Shutdown();
RBLXUI_IMPL_API void     RblxUI_ImplDX10_NewFrame();
RBLXUI_IMPL_API void     RblxUI_ImplDX10_RenderDrawData(RblxDrawData* draw_data);

// Use if you want to reset your rendering device without losing Dear RblxUI state.
RBLXUI_IMPL_API bool     RblxUI_ImplDX10_CreateDeviceObjects();
RBLXUI_IMPL_API void     RblxUI_ImplDX10_InvalidateDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting RblxDrawData::Textures = nullptr to handle this manually.
RBLXUI_IMPL_API void     RblxUI_ImplDX10_UpdateTexture(RblxTextureData* tex);

// [BETA] Selected render state data shared with callbacks.
// This is temporarily stored in GetPlatformIO().Renderer_RenderState during the RblxUI_ImplDX10_RenderDrawData() call.
// (Please open an issue if you feel you need access to more data)
struct RblxUI_ImplDX10_RenderState
{
    ID3D10Device*           Device;
    ID3D10Buffer*           VertexConstantBuffer;
    //ID3D10SamplerState*   SamplerLinear;          // Use RblxDrawList::AddCallback(RblxUI::GetPlatform().DrawCallback_SetSamplerLinear)
    //ID3D10SamplerState*   SamplerNearest;         // Use RblxDrawList::AddCallback(RblxUI::GetPlatform().DrawCallback_SetSamplerNearest)
};

#endif // #ifndef RBLXUI_DISABLE
