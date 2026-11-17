// dear linearui: Renderer Backend for DirectX9
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'LPDIRECT3DTEXTURE9' as texture identifier. Read the FAQ about LinearTextureID/LinearTextureRef!
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (LinearUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Texture updates support for dynamic font atlas (LinearUIBackendFlags_RendererHasTextures).
//  [X] Renderer: LINEARUI_USE_BGRA_PACKED_COLOR support, as this is the optimal color encoding for DirectX9.
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

struct IDirect3DDevice9;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
LINEARUI_IMPL_API bool     LinearUI_ImplDX9_Init(IDirect3DDevice9* device);
LINEARUI_IMPL_API void     LinearUI_ImplDX9_Shutdown();
LINEARUI_IMPL_API void     LinearUI_ImplDX9_NewFrame();
LINEARUI_IMPL_API void     LinearUI_ImplDX9_RenderDrawData(LinearDrawData* draw_data);

// Use if you want to reset your rendering device without losing Dear LinearUI state.
LINEARUI_IMPL_API bool     LinearUI_ImplDX9_CreateDeviceObjects();
LINEARUI_IMPL_API void     LinearUI_ImplDX9_InvalidateDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting LinearDrawData::Textures = nullptr to handle this manually.
LINEARUI_IMPL_API void     LinearUI_ImplDX9_UpdateTexture(LinearTextureData* tex);

#endif // #ifndef LINEARUI_DISABLE
