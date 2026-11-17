// dear linearui: Renderer Backend for SDL_Renderer for SDL2
// (Requires: SDL 2.0.17+)

// Note that SDL_Renderer is an _optional_ component of SDL2, which IMHO is now largely obsolete.
// For a multi-platform app consider using other technologies:
// - SDL3+SDL_GPU: SDL_GPU is SDL3 new graphics abstraction API. You will need to update to SDL3.
// - SDL2+DirectX, SDL2+OpenGL, SDL2+Vulkan: combine SDL with dedicated renderers.
// If your application wants to render any non trivial amount of graphics other than UI,
// please be aware that SDL_Renderer currently offers a limited graphic API to the end-user
// and it might be difficult to step out of those boundaries.

// Implemented features:
//  [X] Renderer: User texture binding. Use 'SDL_Texture*' as texture identifier. Read the FAQ about LinearTextureID/LinearTextureRef!
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (LinearUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Texture updates support for dynamic font atlas (LinearUIBackendFlags_RendererHasTextures).
//  [X] Renderer: Expose selected render state for draw callbacks to use. Access in '(LinearUI_ImplXXXX_RenderState*)GetPlatformIO().Renderer_RenderState'.
// Missing features or Issues:
//  [ ] Renderer: Multi-viewport support (multiple windows).
//  [ ] Renderer: Missing support for DrawCallback_SetSamplerLinear, DrawCallback_SetSamplerNearest callbacks: SDLRenderer2 does not support changing SDL_SCALE_MODE while rendering.

// You can use unmodified linearui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire linearui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear LinearUI:
// - FAQ                  https://dearlinearui.com/faq
// - Getting Started      https://dearlinearui.com/getting-started
// - Documentation        https://dearlinearui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of linearui.cpp

#pragma once
#ifndef LINEARUI_DISABLE
#include "linearui.h"      // LINEARUI_IMPL_API

struct SDL_Renderer;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
LINEARUI_IMPL_API bool     LinearUI_ImplSDLRenderer2_Init(SDL_Renderer* renderer);
LINEARUI_IMPL_API void     LinearUI_ImplSDLRenderer2_Shutdown();
LINEARUI_IMPL_API void     LinearUI_ImplSDLRenderer2_NewFrame();
LINEARUI_IMPL_API void     LinearUI_ImplSDLRenderer2_RenderDrawData(LinearDrawData* draw_data, SDL_Renderer* renderer);

// Called by Init/NewFrame/Shutdown
LINEARUI_IMPL_API void     LinearUI_ImplSDLRenderer2_CreateDeviceObjects();
LINEARUI_IMPL_API void     LinearUI_ImplSDLRenderer2_DestroyDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting LinearDrawData::Textures = nullptr to handle this manually.
LINEARUI_IMPL_API void     LinearUI_ImplSDLRenderer2_UpdateTexture(LinearTextureData* tex);

// [BETA] Selected render state data shared with callbacks.
// This is temporarily stored in GetPlatformIO().Renderer_RenderState during the LinearUI_ImplSDLRenderer2_RenderDrawData() call.
// (Please open an issue if you feel you need access to more data)
struct LinearUI_ImplSDLRenderer2_RenderState
{
    SDL_Renderer*       Renderer;
};

#endif // #ifndef LINEARUI_DISABLE
