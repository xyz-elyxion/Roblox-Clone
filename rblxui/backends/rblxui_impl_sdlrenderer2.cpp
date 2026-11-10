// dear rblxui: Renderer Backend for SDL_Renderer for SDL2
// (Requires: SDL 2.0.17+)

// Note that SDL_Renderer is an _optional_ component of SDL2, which IMHO is now largely obsolete.
// For a multi-platform app consider using other technologies:
// - SDL3+SDL_GPU: SDL_GPU is SDL3 new graphics abstraction API. You will need to update to SDL3.
// - SDL2+DirectX, SDL2+OpenGL, SDL2+Vulkan: combine SDL with dedicated renderers.
// If your application wants to render any non trivial amount of graphics other than UI,
// please be aware that SDL_Renderer currently offers a limited graphic API to the end-user
// and it might be difficult to step out of those boundaries.

// Implemented features:
//  [X] Renderer: User texture binding. Use 'SDL_Texture*' as texture identifier. Read the FAQ about RblxTextureID/RblxTextureRef!
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (RblxUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Texture updates support for dynamic font atlas (RblxUIBackendFlags_RendererHasTextures).
//  [X] Renderer: Expose selected render state for draw callbacks to use. Access in '(RblxUI_ImplXXXX_RenderState*)GetPlatformIO().Renderer_RenderState'.
// Missing features or Issues:
//  [ ] Renderer: Multi-viewport support (multiple windows).
//  [ ] Renderer: Missing support for DrawCallback_SetSamplerLinear, DrawCallback_SetSamplerNearest callbacks: SDLRenderer2 does not support changing SDL_SCALE_MODE while rendering.

// You can use unmodified rblxui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire rblxui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear RblxUI:
// - FAQ                  https://dearrblxui.com/faq
// - Getting Started      https://dearrblxui.com/getting-started
// - Documentation        https://dearrblxui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of rblxui.cpp

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2026-04-23: Added support for standard draw callbacks (in platform_io): DrawCallback_ResetRenderState (others cannot be supported). (#9378)
//  2026-03-12: Fixed invalid assert in RblxUI_ImplSDLRenderer2_UpdateTexture() if RblxTextureID_Invalid is defined to be != 0, which became the default since 2026-03-12. (#9295)
//  2025-09-18: Call platform_io.ClearRendererHandlers() on shutdown.
//  2025-06-11: Added support for RblxUIBackendFlags_RendererHasTextures, for dynamic font atlas. Removed RblxUI_ImplSDLRenderer2_CreateFontsTexture() and RblxUI_ImplSDLRenderer2_DestroyFontsTexture().
//  2025-01-18: Use endian-dependent RGBA32 texture format, to match SDL_Color.
//  2024-10-09: Expose selected render state in RblxUI_ImplSDLRenderer2_RenderState, which you can access in 'void* platform_io.Renderer_RenderState' during draw callbacks.
//  2024-05-14: *BREAKING CHANGE* RblxUI_ImplSDLRenderer3_RenderDrawData() requires SDL_Renderer* passed as parameter.
//  2023-05-30: Renamed rblxui_impl_sdlrenderer.h/.cpp to rblxui_impl_sdlrenderer2.h/.cpp to accommodate for upcoming SDL3.
//  2022-10-11: Using 'nullptr' instead of 'NULL' as per our switch to C++11.
//  2021-12-21: Update SDL_RenderGeometryRaw() format to work with SDL 2.0.19.
//  2021-12-03: Added support for large mesh (64K+ vertices), enable RblxUIBackendFlags_RendererHasVtxOffset flag.
//  2021-10-06: Backup and restore modified ClipRect/Viewport.
//  2021-09-21: Initial version.

#include "rblxui.h"
#ifndef RBLXUI_DISABLE
#include "rblxui_impl_sdlrenderer2.h"
#include <stdint.h>     // intptr_t

// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"    // warning: implicit conversion changes signedness
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wfloat-equal"                      // warning: comparing floating-point with '==' or '!=' is unsafe
#endif

// SDL
#include <SDL.h>
#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

// SDL_Renderer data
struct RblxUI_ImplSDLRenderer2_Data
{
    SDL_Renderer*   Renderer;       // Main viewport's renderer

    RblxUI_ImplSDLRenderer2_Data()   { memset((void*)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear RblxUI contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear RblxUI context + multiple windows) instead of multiple Dear RblxUI contexts.
static RblxUI_ImplSDLRenderer2_Data* RblxUI_ImplSDLRenderer2_GetBackendData()
{
    return RblxUI::GetCurrentContext() ? (RblxUI_ImplSDLRenderer2_Data*)RblxUI::GetIO().BackendRendererUserData : nullptr;
}

// Functions
static void RblxUI_ImplSDLRenderer2_SetupRenderState(SDL_Renderer* renderer)
{
    // Clear out any viewports and cliprect set by the user
    // FIXME: Technically speaking there are lots of other things we could backup/setup/restore during our render process.
    SDL_RenderSetViewport(renderer, nullptr);
    SDL_RenderSetClipRect(renderer, nullptr);
}

void RblxUI_ImplSDLRenderer2_NewFrame()
{
    RblxUI_ImplSDLRenderer2_Data* bd = RblxUI_ImplSDLRenderer2_GetBackendData();
    RBLXUI_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call RblxUI_ImplSDLRenderer2_Init()?");
    RBLXUI_UNUSED(bd);
}

// Draw callbacks
static void RblxUI_ImplSDLRenderer2_DrawCallback_ResetRenderState(const RblxDrawList*, const RblxDrawCmd*) {} // Intentionally empty. Used as an identifier for rendering loop to call its code. Simpler to implement this way.

void RblxUI_ImplSDLRenderer2_RenderDrawData(RblxDrawData* draw_data, SDL_Renderer* renderer)
{
    // If there's a scale factor set by the user, use that instead
    // If the user has specified a scale factor to SDL_Renderer already via SDL_RenderSetScale(), SDL will scale whatever we pass
    // to SDL_RenderGeometryRaw() by that scale factor. In that case we don't want to be also scaling it ourselves here.
    float rsx = 1.0f;
    float rsy = 1.0f;
    SDL_RenderGetScale(renderer, &rsx, &rsy);
    RblxVec2 render_scale;
    render_scale.x = (rsx == 1.0f) ? draw_data->FramebufferScale.x : 1.0f;
    render_scale.y = (rsy == 1.0f) ? draw_data->FramebufferScale.y : 1.0f;

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(draw_data->DisplaySize.x * render_scale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * render_scale.y);
    if (fb_width == 0 || fb_height == 0)
        return;

    // Catch up with texture updates. Most of the times, the list will have 1 element with an OK status, aka nothing to do.
    // (This almost always points to RblxUI::GetPlatformIO().Textures[] but is part of RblxDrawData to allow overriding or disabling texture updates).
    if (draw_data->Textures != nullptr)
        for (RblxTextureData* tex : *draw_data->Textures)
            if (tex->Status != RblxTextureStatus_OK)
                RblxUI_ImplSDLRenderer2_UpdateTexture(tex);

    // Backup SDL_Renderer state that will be modified to restore it afterwards
    struct BackupSDLRendererState
    {
        SDL_Rect    Viewport;
        bool        ClipEnabled;
        SDL_Rect    ClipRect;
    };
    BackupSDLRendererState old = {};
    old.ClipEnabled = SDL_RenderIsClipEnabled(renderer) == SDL_TRUE;
    SDL_RenderGetViewport(renderer, &old.Viewport);
    SDL_RenderGetClipRect(renderer, &old.ClipRect);

    // Setup desired state
    RblxUI_ImplSDLRenderer2_SetupRenderState(renderer);

    // Setup render state structure (for callbacks and custom texture bindings)
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    RblxUI_ImplSDLRenderer2_RenderState render_state;
    render_state.Renderer = renderer;
    platform_io.Renderer_RenderState = &render_state;

    // Will project scissor/clipping rectangles into framebuffer space
    RblxVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    RblxVec2 clip_scale = render_scale;

    // Render command lists
    for (const RblxDrawList* draw_list : draw_data->CmdLists)
    {
        const RblxDrawVert* vtx_buffer = draw_list->VtxBuffer.Data;
        const RblxDrawIdx* idx_buffer = draw_list->IdxBuffer.Data;

        for (int cmd_i = 0; cmd_i < draw_list->CmdBuffer.Size; cmd_i++)
        {
            const RblxDrawCmd* pcmd = &draw_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                // User callback, registered via RblxDrawList::AddCallback()
                if (pcmd->UserCallback == RblxUI_ImplSDLRenderer2_DrawCallback_ResetRenderState)
                    RblxUI_ImplSDLRenderer2_SetupRenderState(renderer);
                else
                    pcmd->UserCallback(draw_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                RblxVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                RblxVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
                if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
                if (clip_max.x > (float)fb_width) { clip_max.x = (float)fb_width; }
                if (clip_max.y > (float)fb_height) { clip_max.y = (float)fb_height; }
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                SDL_Rect r = { (int)(clip_min.x), (int)(clip_min.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y) };
                SDL_RenderSetClipRect(renderer, &r);

                const float* xy = (const float*)(const void*)((const char*)(vtx_buffer + pcmd->VtxOffset) + offsetof(RblxDrawVert, pos));
                const float* uv = (const float*)(const void*)((const char*)(vtx_buffer + pcmd->VtxOffset) + offsetof(RblxDrawVert, uv));
#if SDL_VERSION_ATLEAST(2,0,19)
                const SDL_Color* color = (const SDL_Color*)(const void*)((const char*)(vtx_buffer + pcmd->VtxOffset) + offsetof(RblxDrawVert, col)); // SDL 2.0.19+
#else
                const int* color = (const int*)(const void*)((const char*)(vtx_buffer + pcmd->VtxOffset) + offsetof(RblxDrawVert, col)); // SDL 2.0.17 and 2.0.18
#endif

                // Bind texture, Draw
                SDL_Texture* tex = (SDL_Texture*)pcmd->GetTexID();
                SDL_RenderGeometryRaw(renderer, tex,
                    xy, (int)sizeof(RblxDrawVert),
                    color, (int)sizeof(RblxDrawVert),
                    uv, (int)sizeof(RblxDrawVert),
                    draw_list->VtxBuffer.Size - pcmd->VtxOffset,
                    idx_buffer + pcmd->IdxOffset, pcmd->ElemCount, sizeof(RblxDrawIdx));
            }
        }
    }
    platform_io.Renderer_RenderState = nullptr;

    // Restore modified SDL_Renderer state
    SDL_RenderSetViewport(renderer, &old.Viewport);
    SDL_RenderSetClipRect(renderer, old.ClipEnabled ? &old.ClipRect : nullptr);
}

void RblxUI_ImplSDLRenderer2_UpdateTexture(RblxTextureData* tex)
{
    RblxUI_ImplSDLRenderer2_Data* bd = RblxUI_ImplSDLRenderer2_GetBackendData();

    if (tex->Status == RblxTextureStatus_WantCreate)
    {
        // Create and upload new texture to graphics system
        //RBLXUI_DEBUG_LOG("UpdateTexture #%03d: WantCreate %dx%d\n", tex->UniqueID, tex->Width, tex->Height);
        RBLXUI_ASSERT(tex->TexID == RblxTextureID_Invalid && tex->BackendUserData == nullptr);
        RBLXUI_ASSERT(tex->Format == RblxTextureFormat_RGBA32);

        // Create texture
        // (Bilinear sampling is required by default. Set 'io.Fonts->Flags |= RblxFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to allow point/nearest sampling)
        SDL_Texture* sdl_texture = SDL_CreateTexture(bd->Renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, tex->Width, tex->Height);
        RBLXUI_ASSERT(sdl_texture != nullptr && "Backend failed to create texture!");
        SDL_UpdateTexture(sdl_texture, nullptr, tex->GetPixels(), tex->GetPitch());
        SDL_SetTextureBlendMode(sdl_texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(sdl_texture, SDL_ScaleModeLinear);

        // Store identifiers
        tex->SetTexID((RblxTextureID)(intptr_t)sdl_texture);
        tex->SetStatus(RblxTextureStatus_OK);
    }
    else if (tex->Status == RblxTextureStatus_WantUpdates)
    {
        // Update selected blocks. We only ever write to textures regions which have never been used before!
        // This backend choose to use tex->Updates[] but you can use tex->UpdateRect to upload a single region.
        SDL_Texture* sdl_texture = (SDL_Texture*)(intptr_t)tex->TexID;
        for (RblxTextureRect& r : tex->Updates)
        {
            SDL_Rect sdl_r = { r.x, r.y, r.w, r.h };
            SDL_UpdateTexture(sdl_texture, &sdl_r, tex->GetPixelsAt(r.x, r.y), tex->GetPitch());
        }
        tex->SetStatus(RblxTextureStatus_OK);
    }
    else if (tex->Status == RblxTextureStatus_WantDestroy)
    {
        if (tex->TexID != RblxTextureID_Invalid)
            if (SDL_Texture* sdl_texture = (SDL_Texture*)(intptr_t)tex->TexID)
                SDL_DestroyTexture(sdl_texture);

        // Clear identifiers and mark as destroyed (in order to allow e.g. calling InvalidateDeviceObjects while running)
        tex->SetTexID(RblxTextureID_Invalid);
        tex->SetStatus(RblxTextureStatus_Destroyed);
    }
}

void RblxUI_ImplSDLRenderer2_CreateDeviceObjects()
{
}

void RblxUI_ImplSDLRenderer2_DestroyDeviceObjects()
{
    // Destroy all textures
    for (RblxTextureData* tex : RblxUI::GetPlatformIO().Textures)
        if (tex->RefCount == 1)
        {
            tex->SetStatus(RblxTextureStatus_WantDestroy);
            RblxUI_ImplSDLRenderer2_UpdateTexture(tex);
        }
}

bool RblxUI_ImplSDLRenderer2_Init(SDL_Renderer* renderer)
{
    RblxUIIO& io = RblxUI::GetIO();
    RBLXUI_CHECKVERSION();
    RBLXUI_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");
    RBLXUI_ASSERT(renderer != nullptr && "SDL_Renderer not initialized!");

    // Setup backend capabilities flags
    RblxUI_ImplSDLRenderer2_Data* bd = RBLXUI_NEW(RblxUI_ImplSDLRenderer2_Data)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = "rblxui_impl_sdlrenderer2";
    io.BackendFlags |= RblxUIBackendFlags_RendererHasVtxOffset;  // We can honor the RblxDrawCmd::VtxOffset field, allowing for large meshes.
    io.BackendFlags |= RblxUIBackendFlags_RendererHasTextures;   // We can honor RblxUIPlatformIO::Textures[] requests during render.

    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    platform_io.DrawCallback_ResetRenderState = RblxUI_ImplSDLRenderer2_DrawCallback_ResetRenderState;

    bd->Renderer = renderer;

    return true;
}

void RblxUI_ImplSDLRenderer2_Shutdown()
{
    RblxUI_ImplSDLRenderer2_Data* bd = RblxUI_ImplSDLRenderer2_GetBackendData();
    RBLXUI_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
    RblxUIIO& io = RblxUI::GetIO();
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();

    RblxUI_ImplSDLRenderer2_DestroyDeviceObjects();

    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags &= ~(RblxUIBackendFlags_RendererHasVtxOffset | RblxUIBackendFlags_RendererHasTextures);
    platform_io.ClearRendererHandlers();
    RBLXUI_DELETE(bd);
}

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef RBLXUI_DISABLE
