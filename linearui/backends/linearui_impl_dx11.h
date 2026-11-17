// dear linearui: Renderer Backend for DirectX11
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'ID3D11ShaderResourceView*' as texture identifier. Read the FAQ about LinearTextureID/LinearTextureRef!
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (LinearUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Texture updates support for dynamic font atlas (LinearUIBackendFlags_RendererHasTextures).
//  [X] Renderer: Expose selected render state for draw callbacks to use. Access in '(LinearUI_ImplXXXX_RenderState*)GetPlatformIO().Renderer_RenderState'.
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

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11SamplerState;
struct ID3D11Buffer;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
LINEARUI_IMPL_API bool     LinearUI_ImplDX11_Init(ID3D11Device* device, ID3D11DeviceContext* device_context);
LINEARUI_IMPL_API void     LinearUI_ImplDX11_Shutdown();
LINEARUI_IMPL_API void     LinearUI_ImplDX11_NewFrame();
LINEARUI_IMPL_API void     LinearUI_ImplDX11_RenderDrawData(LinearDrawData* draw_data);

// Use if you want to reset your rendering device without losing Dear LinearUI state.
LINEARUI_IMPL_API bool     LinearUI_ImplDX11_CreateDeviceObjects();
LINEARUI_IMPL_API void     LinearUI_ImplDX11_InvalidateDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting LinearDrawData::Textures = nullptr to handle this manually.
LINEARUI_IMPL_API void     LinearUI_ImplDX11_UpdateTexture(LinearTextureData* tex);

// [BETA] Selected render state data shared with callbacks.
// This is temporarily stored in GetPlatformIO().Renderer_RenderState during the LinearUI_ImplDX11_RenderDrawData() call.
// (Please open an issue if you feel you need access to more data)
struct LinearUI_ImplDX11_RenderState
{
    ID3D11Device*           Device;
    ID3D11DeviceContext*    DeviceContext;
    ID3D11Buffer*           VertexConstantBuffer;
    //ID3D11SamplerState*   SamplerLinear;          // Use LinearDrawList::AddCallback(LinearUI::GetPlatform().DrawCallback_SetSamplerLinear)
    //ID3D11SamplerState*   SamplerNearest;         // Use LinearDrawList::AddCallback(LinearUI::GetPlatform().DrawCallback_SetSamplerNearest)
};

#endif // #ifndef LINEARUI_DISABLE
