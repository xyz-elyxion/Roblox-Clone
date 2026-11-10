// dear rblxui: Null Platform+Renderer Backends
// This is designed if you need to use a blind Dear Imgui context with no input and no output.

// You can use unmodified rblxui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire rblxui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear RblxUI:
// - FAQ                  https://dearrblxui.com/faq
// - Getting Started      https://dearrblxui.com/getting-started
// - Documentation        https://dearrblxui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of rblxui.cpp

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2025-11-17: Initial version.

#include "rblxui.h"
#ifndef RBLXUI_DISABLE
#include "rblxui_impl_null.h"

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning: use of old-style cast                            // yes, they are more terse.
#endif

RBLXUI_IMPL_API bool RblxUI_ImplNull_Init()
{
    RblxUI_ImplNullPlatform_Init();
    RblxUI_ImplNullRender_Init();
    return true;
}

RBLXUI_IMPL_API void RblxUI_ImplNull_Shutdown()
{
    RblxUI_ImplNullRender_Shutdown();
    RblxUI_ImplNullPlatform_Shutdown();
}

RBLXUI_IMPL_API void RblxUI_ImplNull_NewFrame()
{
    RblxUI_ImplNullPlatform_NewFrame();
    RblxUI_ImplNullRender_NewFrame();
}

RBLXUI_IMPL_API bool RblxUI_ImplNullPlatform_Init()
{
    RblxUIIO& io = RblxUI::GetIO();
    io.BackendFlags |= RblxUIBackendFlags_HasMouseCursors;
    return true;
}

RBLXUI_IMPL_API void RblxUI_ImplNullPlatform_Shutdown()
{
    RblxUIIO& io = RblxUI::GetIO();
    io.BackendFlags &= ~RblxUIBackendFlags_HasMouseCursors;
}

RBLXUI_IMPL_API void RblxUI_ImplNullPlatform_NewFrame()
{
    RblxUIIO& io = RblxUI::GetIO();
    io.DisplaySize = RblxVec2(1920, 1080);
    io.DeltaTime = 1.0f / 60.0f;
}

RBLXUI_IMPL_API bool RblxUI_ImplNullRender_Init()
{
    RblxUIIO& io = RblxUI::GetIO();
    io.BackendFlags |= RblxUIBackendFlags_RendererHasVtxOffset;
    io.BackendFlags |= RblxUIBackendFlags_RendererHasTextures;
    return true;
}

RBLXUI_IMPL_API void RblxUI_ImplNullRender_Shutdown()
{
    RblxUIIO& io = RblxUI::GetIO();
    io.BackendFlags &= ~RblxUIBackendFlags_RendererHasVtxOffset;
    io.BackendFlags &= ~RblxUIBackendFlags_RendererHasTextures;
}

RBLXUI_IMPL_API void RblxUI_ImplNullRender_NewFrame()
{
}

static void RblxUI_ImplNullRender_UpdateTexture(RblxTextureData* tex)
{
    if (tex->Status == RblxTextureStatus_WantCreate || tex->Status == RblxTextureStatus_WantDestroy)
        tex->SetStatus(RblxTextureStatus_OK);
    if (tex->Status == RblxTextureStatus_WantDestroy)
    {
        tex->SetTexID(RblxTextureID_Invalid);
        tex->SetStatus(RblxTextureStatus_Destroyed);
    }
}

RBLXUI_IMPL_API void RblxUI_ImplNullRender_RenderDrawData(RblxDrawData* draw_data)
{
    if (draw_data->Textures != nullptr)
        for (RblxTextureData* tex : *draw_data->Textures)
            if (tex->Status != RblxTextureStatus_OK)
                RblxUI_ImplNullRender_UpdateTexture(tex);
}

#endif // #ifndef RBLXUI_DISABLE
