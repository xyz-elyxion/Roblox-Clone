// dear rblxui: Null Platform+Renderer Backends
// This is designed if you need to use a blind Dear Imgui context with no input and no output.

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

// Follow "Getting Started" link and check examples/ folder to learn about using backends!

// Null = NullPlatform + NullRender
RBLXUI_IMPL_API bool     RblxUI_ImplNull_Init();
RBLXUI_IMPL_API void     RblxUI_ImplNull_Shutdown();
RBLXUI_IMPL_API void     RblxUI_ImplNull_NewFrame();

// Null platform only (single screen, fixed timestep, no inputs)
RBLXUI_IMPL_API bool     RblxUI_ImplNullPlatform_Init();
RBLXUI_IMPL_API void     RblxUI_ImplNullPlatform_Shutdown();
RBLXUI_IMPL_API void     RblxUI_ImplNullPlatform_NewFrame();

// Null renderer only (no output)
RBLXUI_IMPL_API bool     RblxUI_ImplNullRender_Init();
RBLXUI_IMPL_API void     RblxUI_ImplNullRender_Shutdown();
RBLXUI_IMPL_API void     RblxUI_ImplNullRender_NewFrame();
RBLXUI_IMPL_API void     RblxUI_ImplNullRender_RenderDrawData(RblxDrawData* draw_data);

#endif // #ifndef RBLXUI_DISABLE
