// dear rblxui: Renderer Backend for DirectX11
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'ID3D11ShaderResourceView*' as texture identifier. Read the FAQ about RblxTextureID/RblxTextureRef!
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (RblxUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Texture updates support for dynamic font atlas (RblxUIBackendFlags_RendererHasTextures).
//  [X] Renderer: Expose selected render state for draw callbacks to use. Access in '(RblxUI_ImplXXXX_RenderState*)GetPlatformIO().Renderer_RenderState'.
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

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11SamplerState;
struct ID3D11Buffer;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool     RblxUI_ImplDX11_Init(ID3D11Device* device, ID3D11DeviceContext* device_context);
RBLXUI_IMPL_API void     RblxUI_ImplDX11_Shutdown();
RBLXUI_IMPL_API void     RblxUI_ImplDX11_NewFrame();
RBLXUI_IMPL_API void     RblxUI_ImplDX11_RenderDrawData(RblxDrawData* draw_data);

// Use if you want to reset your rendering device without losing Dear RblxUI state.
RBLXUI_IMPL_API bool     RblxUI_ImplDX11_CreateDeviceObjects();
RBLXUI_IMPL_API void     RblxUI_ImplDX11_InvalidateDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting RblxDrawData::Textures = nullptr to handle this manually.
RBLXUI_IMPL_API void     RblxUI_ImplDX11_UpdateTexture(RblxTextureData* tex);

// [BETA] Selected render state data shared with callbacks.
// This is temporarily stored in GetPlatformIO().Renderer_RenderState during the RblxUI_ImplDX11_RenderDrawData() call.
// (Please open an issue if you feel you need access to more data)
struct RblxUI_ImplDX11_RenderState
{
    ID3D11Device*           Device;
    ID3D11DeviceContext*    DeviceContext;
    ID3D11Buffer*           VertexConstantBuffer;
    //ID3D11SamplerState*   SamplerLinear;          // Use RblxDrawList::AddCallback(RblxUI::GetPlatform().DrawCallback_SetSamplerLinear)
    //ID3D11SamplerState*   SamplerNearest;         // Use RblxDrawList::AddCallback(RblxUI::GetPlatform().DrawCallback_SetSamplerNearest)
};

#endif // #ifndef RBLXUI_DISABLE
