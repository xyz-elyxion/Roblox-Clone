// dear linearui: Null Platform+Renderer Backends
// This is designed if you need to use a blind Dear Imgui context with no input and no output.

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

// Follow "Getting Started" link and check examples/ folder to learn about using backends!

// Null = NullPlatform + NullRender
LINEARUI_IMPL_API bool     LinearUI_ImplNull_Init();
LINEARUI_IMPL_API void     LinearUI_ImplNull_Shutdown();
LINEARUI_IMPL_API void     LinearUI_ImplNull_NewFrame();

// Null platform only (single screen, fixed timestep, no inputs)
LINEARUI_IMPL_API bool     LinearUI_ImplNullPlatform_Init();
LINEARUI_IMPL_API void     LinearUI_ImplNullPlatform_Shutdown();
LINEARUI_IMPL_API void     LinearUI_ImplNullPlatform_NewFrame();

// Null renderer only (no output)
LINEARUI_IMPL_API bool     LinearUI_ImplNullRender_Init();
LINEARUI_IMPL_API void     LinearUI_ImplNullRender_Shutdown();
LINEARUI_IMPL_API void     LinearUI_ImplNullRender_NewFrame();
LINEARUI_IMPL_API void     LinearUI_ImplNullRender_RenderDrawData(LinearDrawData* draw_data);

#endif // #ifndef LINEARUI_DISABLE
