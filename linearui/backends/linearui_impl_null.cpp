// dear linearui: Null Platform+Renderer Backends
// This is designed if you need to use a blind Dear Imgui context with no input and no output.

// You can use unmodified linearui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire linearui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear LinearUI:
// - FAQ                  https://dearlinearui.com/faq
// - Getting Started      https://dearlinearui.com/getting-started
// - Documentation        https://dearlinearui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of linearui.cpp

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2025-11-17: Initial version.

#include "linearui.h"
#ifndef LINEARUI_DISABLE
#include "linearui_impl_null.h"

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning: use of old-style cast                            // yes, they are more terse.
#endif

LINEARUI_IMPL_API bool LinearUI_ImplNull_Init()
{
    LinearUI_ImplNullPlatform_Init();
    LinearUI_ImplNullRender_Init();
    return true;
}

LINEARUI_IMPL_API void LinearUI_ImplNull_Shutdown()
{
    LinearUI_ImplNullRender_Shutdown();
    LinearUI_ImplNullPlatform_Shutdown();
}

LINEARUI_IMPL_API void LinearUI_ImplNull_NewFrame()
{
    LinearUI_ImplNullPlatform_NewFrame();
    LinearUI_ImplNullRender_NewFrame();
}

LINEARUI_IMPL_API bool LinearUI_ImplNullPlatform_Init()
{
    LinearUIIO& io = LinearUI::GetIO();
    io.BackendFlags |= LinearUIBackendFlags_HasMouseCursors;
    return true;
}

LINEARUI_IMPL_API void LinearUI_ImplNullPlatform_Shutdown()
{
    LinearUIIO& io = LinearUI::GetIO();
    io.BackendFlags &= ~LinearUIBackendFlags_HasMouseCursors;
}

LINEARUI_IMPL_API void LinearUI_ImplNullPlatform_NewFrame()
{
    LinearUIIO& io = LinearUI::GetIO();
    io.DisplaySize = LinearVec2(1920, 1080);
    io.DeltaTime = 1.0f / 60.0f;
}

LINEARUI_IMPL_API bool LinearUI_ImplNullRender_Init()
{
    LinearUIIO& io = LinearUI::GetIO();
    io.BackendFlags |= LinearUIBackendFlags_RendererHasVtxOffset;
    io.BackendFlags |= LinearUIBackendFlags_RendererHasTextures;
    return true;
}

LINEARUI_IMPL_API void LinearUI_ImplNullRender_Shutdown()
{
    LinearUIIO& io = LinearUI::GetIO();
    io.BackendFlags &= ~LinearUIBackendFlags_RendererHasVtxOffset;
    io.BackendFlags &= ~LinearUIBackendFlags_RendererHasTextures;
}

LINEARUI_IMPL_API void LinearUI_ImplNullRender_NewFrame()
{
}

static void LinearUI_ImplNullRender_UpdateTexture(LinearTextureData* tex)
{
    if (tex->Status == LinearTextureStatus_WantCreate || tex->Status == LinearTextureStatus_WantDestroy)
        tex->SetStatus(LinearTextureStatus_OK);
    if (tex->Status == LinearTextureStatus_WantDestroy)
    {
        tex->SetTexID(LinearTextureID_Invalid);
        tex->SetStatus(LinearTextureStatus_Destroyed);
    }
}

LINEARUI_IMPL_API void LinearUI_ImplNullRender_RenderDrawData(LinearDrawData* draw_data)
{
    if (draw_data->Textures != nullptr)
        for (LinearTextureData* tex : *draw_data->Textures)
            if (tex->Status != LinearTextureStatus_OK)
                LinearUI_ImplNullRender_UpdateTexture(tex);
}

#endif // #ifndef LINEARUI_DISABLE
