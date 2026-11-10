// dear rblxui: Renderer Backend for SDL_Renderer for SDL3
// (Requires: SDL 3.1.8+)

// Note that SDL_Renderer is an _optional_ component of SDL3, which IMHO is now largely obsolete.
// For a multi-platform app consider using other technologies:
// - SDL3+SDL_GPU: SDL_GPU is SDL3 new graphics abstraction API.
// - SDL3+DirectX, SDL3+OpenGL, SDL3+Vulkan: combine SDL with dedicated renderers.
// If your application wants to render any non trivial amount of graphics other than UI,
// please be aware that SDL_Renderer currently offers a limited graphic API to the end-user
// and it might be difficult to step out of those boundaries.

// Implemented features:
//  [X] Renderer: User texture binding. Use 'SDL_Texture*' as texture identifier. Read the FAQ about RblxTextureID/RblxTextureRef!
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (RblxUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Texture updates support for dynamic font atlas (RblxUIBackendFlags_RendererHasTextures).
//  [X] Renderer: Expose selected render state for draw callbacks to use. Access in '(RblxUI_ImplXXXX_RenderState*)GetPlatformIO().Renderer_RenderState'.
// Missing features:
//  [ ] Renderer: Multi-viewport support (multiple windows).

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

struct SDL_Renderer;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool     RblxUI_ImplSDLRenderer3_Init(SDL_Renderer* renderer);
RBLXUI_IMPL_API void     RblxUI_ImplSDLRenderer3_Shutdown();
RBLXUI_IMPL_API void     RblxUI_ImplSDLRenderer3_NewFrame();
RBLXUI_IMPL_API void     RblxUI_ImplSDLRenderer3_RenderDrawData(RblxDrawData* draw_data, SDL_Renderer* renderer);

// Called by Init/NewFrame/Shutdown
RBLXUI_IMPL_API void     RblxUI_ImplSDLRenderer3_CreateDeviceObjects();
RBLXUI_IMPL_API void     RblxUI_ImplSDLRenderer3_DestroyDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting RblxDrawData::Textures = nullptr to handle this manually.
RBLXUI_IMPL_API void     RblxUI_ImplSDLRenderer3_UpdateTexture(RblxTextureData* tex);

// [BETA] Selected render state data shared with callbacks.
// This is temporarily stored in GetPlatformIO().Renderer_RenderState during the RblxUI_ImplSDLRenderer3_RenderDrawData() call.
// (Please open an issue if you feel you need access to more data)
struct RblxUI_ImplSDLRenderer3_RenderState
{
    SDL_Renderer*       Renderer;
};

#endif // #ifndef RBLXUI_DISABLE
