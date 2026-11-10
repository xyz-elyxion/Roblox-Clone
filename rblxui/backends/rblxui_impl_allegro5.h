// dear rblxui: Renderer + Platform Backend for Allegro 5
// (Info: Allegro 5 is a cross-platform general purpose library for handling windows, inputs, graphics, etc.)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'ALLEGRO_BITMAP*' as texture identifier. Read the FAQ about RblxTextureID/RblxTextureRef!
//  [X] Renderer: Texture updates support for dynamic font atlas (RblxUIBackendFlags_RendererHasTextures).
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass RblxUIKey values to all key functions e.g. RblxUI::IsKeyPressed(RblxUIKey_Space). [Legacy ALLEGRO_KEY_* values are obsolete since 1.87 and not supported since 1.91.5]
//  [X] Platform: Clipboard support (from Allegro 5.1.12).
//  [X] Platform: Mouse cursor shape and visibility (RblxUIBackendFlags_HasMouseCursors). Disable with 'io.ConfigFlags |= RblxUIConfigFlags_NoMouseCursorChange'.
// Missing features or Issues:
//  [ ] Renderer: The renderer is suboptimal as we need to unindex our buffers and convert vertices manually.
//  [ ] Renderer: Missing support for DrawCallback_SetSamplerLinear, DrawCallback_SetSamplerNearest callbacks: Allegro5 cannot enable/disable LINEAR bitmap flags after creation.
//  [ ] Platform: Missing gamepad support.
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

struct ALLEGRO_DISPLAY;
union ALLEGRO_EVENT;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool     RblxUI_ImplAllegro5_Init(ALLEGRO_DISPLAY* display);
RBLXUI_IMPL_API void     RblxUI_ImplAllegro5_Shutdown();
RBLXUI_IMPL_API void     RblxUI_ImplAllegro5_NewFrame();
RBLXUI_IMPL_API void     RblxUI_ImplAllegro5_RenderDrawData(RblxDrawData* draw_data);
RBLXUI_IMPL_API bool     RblxUI_ImplAllegro5_ProcessEvent(ALLEGRO_EVENT* event);
RBLXUI_IMPL_API void     RblxUI_ImplAllegro5_SetDisplay(ALLEGRO_DISPLAY* display);

// Use if you want to reset your rendering device without losing Dear RblxUI state.
RBLXUI_IMPL_API bool     RblxUI_ImplAllegro5_CreateDeviceObjects();
RBLXUI_IMPL_API void     RblxUI_ImplAllegro5_InvalidateDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting RblxDrawData::Textures = nullptr to handle this manually.
RBLXUI_IMPL_API void     RblxUI_ImplAllegro5_UpdateTexture(RblxTextureData* tex);

#endif // #ifndef RBLXUI_DISABLE
