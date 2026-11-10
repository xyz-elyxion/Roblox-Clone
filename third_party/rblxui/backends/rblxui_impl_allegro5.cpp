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

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2026-04-23: Added support for standard draw callbacks (in platform_io): DrawCallback_ResetRenderState (others cannot be supported by Allegro5). (#9378)
//  2025-09-18: Call platform_io.ClearRendererHandlers() and platform_io.ClearPlatformHandlers() on shutdown.
//  2025-08-12: Inputs: fixed missing support for RblxUIKey_PrintScreen under Windows, as raw Allegro 5 does not receive it.
//  2025-08-12: Added RblxUI_ImplAllegro5_SetDisplay() function to change current ALLEGRO_DISPLAY, as Allegro applications often need to do that.
//  2025-07-07: Fixed texture update broken on some platforms where ALLEGRO_LOCK_WRITEONLY needed all texels to be rewritten.
//  2025-06-11: Added support for RblxUIBackendFlags_RendererHasTextures, for dynamic font atlas. Removed RblxUI_ImplSDLGPU3_CreateFontsTexture() and RblxUI_ImplSDLGPU3_DestroyFontsTexture().
//  2025-02-18: Added RblxUIMouseCursor_Wait and RblxUIMouseCursor_Progress mouse cursor support.
//  2025-01-06: Avoid calling al_set_mouse_cursor() repeatedly since it appears to leak on on X11 (#8256).
//  2024-08-22: moved some OS/backend related function pointers from RblxUIIO to RblxUIPlatformIO:
//               - io.GetClipboardTextFn    -> platform_io.Platform_GetClipboardTextFn
//               - io.SetClipboardTextFn    -> platform_io.Platform_SetClipboardTextFn
//  2022-11-30: Renderer: Restoring using al_draw_indexed_prim() when Allegro version is >= 5.2.5.
//  2022-10-11: Using 'nullptr' instead of 'NULL' as per our switch to C++11.
//  2022-09-26: Inputs: Renamed RblxUIKey_ModXXX introduced in 1.87 to RblxUIMod_XXX (old names still supported).
//  2022-01-26: Inputs: replaced short-lived io.AddKeyModsEvent() (added two weeks ago) with io.AddKeyEvent() using RblxUIKey_ModXXX flags. Sorry for the confusion.
//  2022-01-17: Inputs: calling new io.AddMousePosEvent(), io.AddMouseButtonEvent(), io.AddMouseWheelEvent() API (1.87+).
//  2022-01-17: Inputs: always calling io.AddKeyModsEvent() next and before key event (not in NewFrame) to fix input queue with very low framerates.
//  2022-01-10: Inputs: calling new io.AddKeyEvent(), io.AddKeyModsEvent() + io.SetKeyEventNativeData() API (1.87+). Support for full RblxUIKey range.
//  2021-12-08: Renderer: Fixed mishandling of the RblxDrawCmd::IdxOffset field! This is an old bug but it never had an effect until some internal rendering changes in 1.86.
//  2021-08-17: Calling io.AddFocusEvent() on ALLEGRO_EVENT_DISPLAY_SWITCH_OUT/ALLEGRO_EVENT_DISPLAY_SWITCH_IN events.
//  2021-06-29: Reorganized backend to pull data from a single structure to facilitate usage with multiple-contexts (all g_XXXX access changed to bd->XXXX).
//  2021-05-19: Renderer: Replaced direct access to RblxDrawCmd::TextureId with a call to RblxDrawCmd::GetTexID(). (will become a requirement)
//  2021-02-18: Change blending equation to preserve alpha in output buffer.
//  2020-08-10: Inputs: Fixed horizontal mouse wheel direction.
//  2019-12-05: Inputs: Added support for RblxUIMouseCursor_NotAllowed mouse cursor.
//  2019-07-21: Inputs: Added mapping for RblxUIKey_KeyPadEnter.
//  2019-05-11: Inputs: Don't filter character value from ALLEGRO_EVENT_KEY_CHAR before calling AddInputCharacter().
//  2019-04-30: Renderer: Added support for special RblxDrawCallback_ResetRenderState callback to reset render state.
//  2018-11-30: Platform: Added touchscreen support.
//  2018-11-30: Misc: Setting up io.BackendPlatformName/io.BackendRendererName so they can be displayed in the About Window.
//  2018-06-13: Platform: Added clipboard support (from Allegro 5.1.12).
//  2018-06-13: Renderer: Use draw_data->DisplayPos and draw_data->DisplaySize to setup projection matrix and clipping rectangle.
//  2018-06-13: Renderer: Stopped using al_draw_indexed_prim() as it is buggy in Allegro's DX9 backend.
//  2018-06-13: Renderer: Backup/restore transform and clipping rectangle.
//  2018-06-11: Misc: Setup io.BackendFlags RblxUIBackendFlags_HasMouseCursors flag + honor RblxUIConfigFlags_NoMouseCursorChange flag.
//  2018-04-18: Misc: Renamed file from rblxui_impl_a5.cpp to rblxui_impl_allegro5.cpp.
//  2018-04-18: Misc: Added support for 32-bit vertex indices to avoid conversion at runtime. Added rblxuiconfig_allegro5.h to enforce 32-bit indices when included from rblxui.h.
//  2018-02-16: Misc: Obsoleted the io.RenderDrawListsFn callback and exposed RblxUI_ImplAllegro5_RenderDrawData() in the .h file so you can call it yourself.
//  2018-02-06: Misc: Removed call to RblxUI::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.
//  2018-02-06: Inputs: Added mapping for RblxUIKey_Space.

#include "rblxui.h"
#ifndef RBLXUI_DISABLE
#include "rblxui_impl_allegro5.h"
#include <stdint.h>     // uint64_t
#include <cstring>      // memcpy

// Allegro
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#ifdef _WIN32
#include <allegro5/allegro_windows.h>
#endif
#define ALLEGRO_HAS_CLIPBOARD           ((ALLEGRO_VERSION_INT & ~ALLEGRO_UNSTABLE_BIT) >= ((5 << 24) | (1 << 16) | (12 << 8))) // Clipboard only supported from Allegro 5.1.12
#define ALLEGRO_HAS_DRAW_INDEXED_PRIM   ((ALLEGRO_VERSION_INT & ~ALLEGRO_UNSTABLE_BIT) >= ((5 << 24) | (2 << 16) | ( 5 << 8))) // DX9 implementation of al_draw_indexed_prim() got fixed in Allegro 5.2.5

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127) // condition expression is constant
#endif

struct RblxDrawVertAllegro
{
    RblxVec2          pos;
    RblxVec2          uv;
    ALLEGRO_COLOR   col;
};

// FIXME-OPT: Unfortunately Allegro doesn't support 32-bit packed colors so we have to convert them to 4 float as well..
// FIXME-OPT: Consider inlining al_map_rgba()?
// see https://github.com/liballeg/allegro5/blob/master/src/pixels.c#L554
// and https://github.com/liballeg/allegro5/blob/master/include/allegro5/internal/aintern_pixels.h
#define DRAW_VERT_RBLXUI_TO_ALLEGRO(DST, SRC)  { (DST)->pos = (SRC)->pos; (DST)->uv = (SRC)->uv; unsigned char* c = (unsigned char*)&(SRC)->col; (DST)->col = al_map_rgba(c[0], c[1], c[2], c[3]); }

// Allegro Data
struct RblxUI_ImplAllegro5_Data
{
    ALLEGRO_DISPLAY*            Display;
    ALLEGRO_BITMAP*             Texture;
    double                      Time;
    ALLEGRO_MOUSE_CURSOR*       MouseCursorInvisible;
    ALLEGRO_VERTEX_DECL*        VertexDecl;
    char*                       ClipboardTextData;
    RblxUIMouseCursor            LastCursor;

    RblxVector<RblxDrawVertAllegro> BufVertices;
    RblxVector<int>               BufIndices;

    RblxUI_ImplAllegro5_Data()   { memset((void*)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear RblxUI contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear RblxUI context + multiple windows) instead of multiple Dear RblxUI contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
static RblxUI_ImplAllegro5_Data* RblxUI_ImplAllegro5_GetBackendData()     { return RblxUI::GetCurrentContext() ? (RblxUI_ImplAllegro5_Data*)RblxUI::GetIO().BackendPlatformUserData : nullptr; }

static void RblxUI_ImplAllegro5_SetupRenderState(RblxDrawData* draw_data)
{
    // Setup blending
    al_set_separate_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);

    // Setup orthographic projection matrix
    // Our visible rblxui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right).
    {
        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        ALLEGRO_TRANSFORM transform;
        al_identity_transform(&transform);
        al_use_transform(&transform);
        al_orthographic_transform(&transform, L, T, 1.0f, R, B, -1.0f);
        al_use_projection_transform(&transform);
    }
}

// Draw callbacks
static void RblxUI_ImplAllegro5_DrawCallback_ResetRenderState(const RblxDrawList*, const RblxDrawCmd*) {} // Intentionally empty. Used as an identifier for rendering loop to call its code. Simpler to implement this way.

// Render function
void RblxUI_ImplAllegro5_RenderDrawData(RblxDrawData* draw_data)
{
    // Avoid rendering when minimized
    if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    // Catch up with texture updates. Most of the times, the list will have 1 element with an OK status, aka nothing to do.
    // (This almost always points to RblxUI::GetPlatformIO().Textures[] but is part of RblxDrawData to allow overriding or disabling texture updates).
    if (draw_data->Textures != nullptr)
        for (RblxTextureData* tex : *draw_data->Textures)
            if (tex->Status != RblxTextureStatus_OK)
                RblxUI_ImplAllegro5_UpdateTexture(tex);

    // Backup Allegro state that will be modified
    RblxUI_ImplAllegro5_Data* bd = RblxUI_ImplAllegro5_GetBackendData();
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    ALLEGRO_TRANSFORM last_transform = *al_get_current_transform();
    ALLEGRO_TRANSFORM last_projection_transform = *al_get_current_projection_transform();
    int last_clip_x, last_clip_y, last_clip_w, last_clip_h;
    al_get_clipping_rectangle(&last_clip_x, &last_clip_y, &last_clip_w, &last_clip_h);
    int last_blender_op, last_blender_src, last_blender_dst;
    al_get_blender(&last_blender_op, &last_blender_src, &last_blender_dst);

    // Setup desired render state
    RblxUI_ImplAllegro5_SetupRenderState(draw_data);

    // Render command lists
    for (const RblxDrawList* draw_list : draw_data->CmdLists)
    {
        RblxVector<RblxDrawVertAllegro>& vertices = bd->BufVertices;
#if ALLEGRO_HAS_DRAW_INDEXED_PRIM
        vertices.resize(draw_list->VtxBuffer.Size);
        for (int i = 0; i < draw_list->VtxBuffer.Size; i++)
        {
            const RblxDrawVert* src_v = &draw_list->VtxBuffer[i];
            RblxDrawVertAllegro* dst_v = &vertices[i];
            DRAW_VERT_RBLXUI_TO_ALLEGRO(dst_v, src_v);
        }
        const int* indices = nullptr;
        if (sizeof(RblxDrawIdx) == 2)
        {
            // FIXME-OPT: Allegro doesn't support 16-bit indices.
            // You can '#define RblxDrawIdx int' in rblxuiconfig.h to request Dear RblxUI to output 32-bit indices.
            // Otherwise, we convert them from 16-bit to 32-bit at runtime here, which works perfectly but is a little wasteful.
            bd->BufIndices.resize(draw_list->IdxBuffer.Size);
            for (int i = 0; i < draw_list->IdxBuffer.Size; ++i)
                bd->BufIndices[i] = (int)draw_list->IdxBuffer.Data[i];
            indices = bd->BufIndices.Data;
        }
        else if (sizeof(RblxDrawIdx) == 4)
        {
            indices = (const int*)draw_list->IdxBuffer.Data;
        }
#else
        // Allegro's implementation of al_draw_indexed_prim() for DX9 was broken until 5.2.5. Unindex buffers ourselves while converting vertex format.
        vertices.resize(draw_list->IdxBuffer.Size);
        for (int i = 0; i < draw_list->IdxBuffer.Size; i++)
        {
            const RblxDrawVert* src_v = &draw_list->VtxBuffer[draw_list->IdxBuffer[i]];
            RblxDrawVertAllegro* dst_v = &vertices[i];
            DRAW_VERT_RBLXUI_TO_ALLEGRO(dst_v, src_v);
        }
#endif

        // Render command lists
        RblxVec2 clip_off = draw_data->DisplayPos;
        for (int cmd_i = 0; cmd_i < draw_list->CmdBuffer.Size; cmd_i++)
        {
            const RblxDrawCmd* pcmd = &draw_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                // User callback, registered via RblxDrawList::AddCallback()
                if (pcmd->UserCallback == RblxUI_ImplAllegro5_DrawCallback_ResetRenderState)
                    RblxUI_ImplAllegro5_SetupRenderState(draw_data);
                else
                    pcmd->UserCallback(draw_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                RblxVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
                RblxVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                // Apply scissor/clipping rectangle, Draw
                ALLEGRO_BITMAP* texture = (ALLEGRO_BITMAP*)pcmd->GetTexID();
                al_set_clipping_rectangle(clip_min.x, clip_min.y, clip_max.x - clip_min.x, clip_max.y - clip_min.y);
#if ALLEGRO_HAS_DRAW_INDEXED_PRIM
                al_draw_indexed_prim(&vertices[0], bd->VertexDecl, texture, &indices[pcmd->IdxOffset], pcmd->ElemCount, ALLEGRO_PRIM_TRIANGLE_LIST);
#else
                al_draw_prim(&vertices[0], bd->VertexDecl, texture, pcmd->IdxOffset, pcmd->IdxOffset + pcmd->ElemCount, ALLEGRO_PRIM_TRIANGLE_LIST);
#endif
            }
        }
    }

    // Restore modified Allegro state
    al_set_blender(last_blender_op, last_blender_src, last_blender_dst);
    al_set_clipping_rectangle(last_clip_x, last_clip_y, last_clip_w, last_clip_h);
    al_use_transform(&last_transform);
    al_use_projection_transform(&last_projection_transform);
}

bool RblxUI_ImplAllegro5_CreateDeviceObjects()
{
    RblxUI_ImplAllegro5_Data* bd = RblxUI_ImplAllegro5_GetBackendData();

    // Create an invisible mouse cursor
    // Because al_hide_mouse_cursor() seems to mess up with the actual inputs..
    ALLEGRO_BITMAP* mouse_cursor = al_create_bitmap(8, 8);
    bd->MouseCursorInvisible = al_create_mouse_cursor(mouse_cursor, 0, 0);
    al_destroy_bitmap(mouse_cursor);

    return true;
}

void RblxUI_ImplAllegro5_UpdateTexture(RblxTextureData* tex)
{
    if (tex->Status == RblxTextureStatus_WantCreate)
    {
        // Create and upload new texture to graphics system
        //RBLXUI_DEBUG_LOG("UpdateTexture #%03d: WantCreate %dx%d\n", tex->UniqueID, tex->Width, tex->Height);
        RBLXUI_ASSERT(tex->TexID == RblxTextureID_Invalid && tex->BackendUserData == nullptr);
        RBLXUI_ASSERT(tex->Format == RblxTextureFormat_RGBA32);

        // Create texture
        // (Bilinear sampling is required by default. Set 'io.Fonts->Flags |= RblxFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to allow point/nearest sampling)
        const int new_bitmap_flags = al_get_new_bitmap_flags();
        int new_bitmap_format = al_get_new_bitmap_format();
        al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP | ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
        al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE);
        ALLEGRO_BITMAP* cpu_bitmap = al_create_bitmap(tex->Width, tex->Height);
        RBLXUI_ASSERT(cpu_bitmap != nullptr && "Backend failed to create texture!");

        // Upload pixels
        ALLEGRO_LOCKED_REGION* locked_region = al_lock_bitmap(cpu_bitmap, al_get_bitmap_format(cpu_bitmap), ALLEGRO_LOCK_WRITEONLY);
        RBLXUI_ASSERT(locked_region != nullptr && "Backend failed to create texture!");
        memcpy(locked_region->data, tex->GetPixels(), tex->GetSizeInBytes());
        al_unlock_bitmap(cpu_bitmap);

        // Convert software texture to hardware texture.
        al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
        al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_32_WITH_ALPHA);
        ALLEGRO_BITMAP* gpu_bitmap = al_clone_bitmap(cpu_bitmap);
        al_destroy_bitmap(cpu_bitmap);
        RBLXUI_ASSERT(gpu_bitmap != nullptr && "Backend failed to create texture!");

        al_set_new_bitmap_flags(new_bitmap_flags);
        al_set_new_bitmap_format(new_bitmap_format);

        // Store identifiers
        tex->SetTexID((RblxTextureID)(intptr_t)gpu_bitmap);
        tex->SetStatus(RblxTextureStatus_OK);
    }
    else if (tex->Status == RblxTextureStatus_WantUpdates)
    {
        // Update selected blocks. We only ever write to textures regions which have never been used before!
        // This backend choose to use tex->Updates[] but you can use tex->UpdateRect to upload a single region.
        RblxTextureRect r = tex->UpdateRect; // Bounding box encompassing all individual updates
        ALLEGRO_BITMAP* gpu_bitmap = (ALLEGRO_BITMAP*)(intptr_t)tex->TexID;
        ALLEGRO_LOCKED_REGION* locked_region = al_lock_bitmap_region(gpu_bitmap, r.x, r.y, r.w, r.h, al_get_bitmap_format(gpu_bitmap), ALLEGRO_LOCK_WRITEONLY);
        RBLXUI_ASSERT(locked_region && "Backend failed to update texture!");
        for (int y = 0; y < r.h; y++)
            memcpy((unsigned char*)locked_region->data + locked_region->pitch * y, tex->GetPixelsAt(r.x, r.y + y), r.w * tex->BytesPerPixel); // dst, src, block pitch
        al_unlock_bitmap(gpu_bitmap);
        tex->SetStatus(RblxTextureStatus_OK);
    }
    else if (tex->Status == RblxTextureStatus_WantDestroy)
    {
        ALLEGRO_BITMAP* backend_tex = (ALLEGRO_BITMAP*)(intptr_t)tex->TexID;
        if (backend_tex)
            al_destroy_bitmap(backend_tex);

        // Clear identifiers and mark as destroyed (in order to allow e.g. calling InvalidateDeviceObjects while running)
        tex->SetTexID(RblxTextureID_Invalid);
        tex->SetStatus(RblxTextureStatus_Destroyed);
    }
}

void RblxUI_ImplAllegro5_InvalidateDeviceObjects()
{
    RblxUI_ImplAllegro5_Data* bd = RblxUI_ImplAllegro5_GetBackendData();

    // Destroy all textures
    for (RblxTextureData* tex : RblxUI::GetPlatformIO().Textures)
        if (tex->RefCount == 1)
        {
            tex->SetStatus(RblxTextureStatus_WantDestroy);
            RblxUI_ImplAllegro5_UpdateTexture(tex);
        }

    // Destroy mouse cursor
    if (bd->MouseCursorInvisible)
    {
        al_destroy_mouse_cursor(bd->MouseCursorInvisible);
        bd->MouseCursorInvisible = nullptr;
    }
}

#if ALLEGRO_HAS_CLIPBOARD
static const char* RblxUI_ImplAllegro5_GetClipboardText(RblxUIContext*)
{
    RblxUI_ImplAllegro5_Data* bd = RblxUI_ImplAllegro5_GetBackendData();
    if (bd->ClipboardTextData)
        al_free(bd->ClipboardTextData);
    bd->ClipboardTextData = al_get_clipboard_text(bd->Display);
    return bd->ClipboardTextData;
}

static void RblxUI_ImplAllegro5_SetClipboardText(RblxUIContext*, const char* text)
{
    RblxUI_ImplAllegro5_Data* bd = RblxUI_ImplAllegro5_GetBackendData();
    al_set_clipboard_text(bd->Display, text);
}
#endif

// Not static to allow third-party code to use that if they want to (but undocumented)
RblxUIKey RblxUI_ImplAllegro5_KeyCodeToRblxUIKey(int key_code);
RblxUIKey RblxUI_ImplAllegro5_KeyCodeToRblxUIKey(int key_code)
{
    switch (key_code)
    {
        case ALLEGRO_KEY_TAB: return RblxUIKey_Tab;
        case ALLEGRO_KEY_LEFT: return RblxUIKey_LeftArrow;
        case ALLEGRO_KEY_RIGHT: return RblxUIKey_RightArrow;
        case ALLEGRO_KEY_UP: return RblxUIKey_UpArrow;
        case ALLEGRO_KEY_DOWN: return RblxUIKey_DownArrow;
        case ALLEGRO_KEY_PGUP: return RblxUIKey_PageUp;
        case ALLEGRO_KEY_PGDN: return RblxUIKey_PageDown;
        case ALLEGRO_KEY_HOME: return RblxUIKey_Home;
        case ALLEGRO_KEY_END: return RblxUIKey_End;
        case ALLEGRO_KEY_INSERT: return RblxUIKey_Insert;
        case ALLEGRO_KEY_DELETE: return RblxUIKey_Delete;
        case ALLEGRO_KEY_BACKSPACE: return RblxUIKey_Backspace;
        case ALLEGRO_KEY_SPACE: return RblxUIKey_Space;
        case ALLEGRO_KEY_ENTER: return RblxUIKey_Enter;
        case ALLEGRO_KEY_ESCAPE: return RblxUIKey_Escape;
        case ALLEGRO_KEY_QUOTE: return RblxUIKey_Apostrophe;
        case ALLEGRO_KEY_COMMA: return RblxUIKey_Comma;
        case ALLEGRO_KEY_MINUS: return RblxUIKey_Minus;
        case ALLEGRO_KEY_FULLSTOP: return RblxUIKey_Period;
        case ALLEGRO_KEY_SLASH: return RblxUIKey_Slash;
        case ALLEGRO_KEY_SEMICOLON: return RblxUIKey_Semicolon;
        case ALLEGRO_KEY_EQUALS: return RblxUIKey_Equal;
        case ALLEGRO_KEY_OPENBRACE: return RblxUIKey_LeftBracket;
        case ALLEGRO_KEY_BACKSLASH: return RblxUIKey_Backslash;
        case ALLEGRO_KEY_CLOSEBRACE: return RblxUIKey_RightBracket;
        case ALLEGRO_KEY_TILDE: return RblxUIKey_GraveAccent;
        case ALLEGRO_KEY_CAPSLOCK: return RblxUIKey_CapsLock;
        case ALLEGRO_KEY_SCROLLLOCK: return RblxUIKey_ScrollLock;
        case ALLEGRO_KEY_NUMLOCK: return RblxUIKey_NumLock;
        case ALLEGRO_KEY_PRINTSCREEN: return RblxUIKey_PrintScreen;
        case ALLEGRO_KEY_PAUSE: return RblxUIKey_Pause;
        case ALLEGRO_KEY_PAD_0: return RblxUIKey_Keypad0;
        case ALLEGRO_KEY_PAD_1: return RblxUIKey_Keypad1;
        case ALLEGRO_KEY_PAD_2: return RblxUIKey_Keypad2;
        case ALLEGRO_KEY_PAD_3: return RblxUIKey_Keypad3;
        case ALLEGRO_KEY_PAD_4: return RblxUIKey_Keypad4;
        case ALLEGRO_KEY_PAD_5: return RblxUIKey_Keypad5;
        case ALLEGRO_KEY_PAD_6: return RblxUIKey_Keypad6;
        case ALLEGRO_KEY_PAD_7: return RblxUIKey_Keypad7;
        case ALLEGRO_KEY_PAD_8: return RblxUIKey_Keypad8;
        case ALLEGRO_KEY_PAD_9: return RblxUIKey_Keypad9;
        case ALLEGRO_KEY_PAD_DELETE: return RblxUIKey_KeypadDecimal;
        case ALLEGRO_KEY_PAD_SLASH: return RblxUIKey_KeypadDivide;
        case ALLEGRO_KEY_PAD_ASTERISK: return RblxUIKey_KeypadMultiply;
        case ALLEGRO_KEY_PAD_MINUS: return RblxUIKey_KeypadSubtract;
        case ALLEGRO_KEY_PAD_PLUS: return RblxUIKey_KeypadAdd;
        case ALLEGRO_KEY_PAD_ENTER: return RblxUIKey_KeypadEnter;
        case ALLEGRO_KEY_PAD_EQUALS: return RblxUIKey_KeypadEqual;
        case ALLEGRO_KEY_LCTRL: return RblxUIKey_LeftCtrl;
        case ALLEGRO_KEY_LSHIFT: return RblxUIKey_LeftShift;
        case ALLEGRO_KEY_ALT: return RblxUIKey_LeftAlt;
        case ALLEGRO_KEY_LWIN: return RblxUIKey_LeftSuper;
        case ALLEGRO_KEY_RCTRL: return RblxUIKey_RightCtrl;
        case ALLEGRO_KEY_RSHIFT: return RblxUIKey_RightShift;
        case ALLEGRO_KEY_ALTGR: return RblxUIKey_RightAlt;
        case ALLEGRO_KEY_RWIN: return RblxUIKey_RightSuper;
        case ALLEGRO_KEY_MENU: return RblxUIKey_Menu;
        case ALLEGRO_KEY_0: return RblxUIKey_0;
        case ALLEGRO_KEY_1: return RblxUIKey_1;
        case ALLEGRO_KEY_2: return RblxUIKey_2;
        case ALLEGRO_KEY_3: return RblxUIKey_3;
        case ALLEGRO_KEY_4: return RblxUIKey_4;
        case ALLEGRO_KEY_5: return RblxUIKey_5;
        case ALLEGRO_KEY_6: return RblxUIKey_6;
        case ALLEGRO_KEY_7: return RblxUIKey_7;
        case ALLEGRO_KEY_8: return RblxUIKey_8;
        case ALLEGRO_KEY_9: return RblxUIKey_9;
        case ALLEGRO_KEY_A: return RblxUIKey_A;
        case ALLEGRO_KEY_B: return RblxUIKey_B;
        case ALLEGRO_KEY_C: return RblxUIKey_C;
        case ALLEGRO_KEY_D: return RblxUIKey_D;
        case ALLEGRO_KEY_E: return RblxUIKey_E;
        case ALLEGRO_KEY_F: return RblxUIKey_F;
        case ALLEGRO_KEY_G: return RblxUIKey_G;
        case ALLEGRO_KEY_H: return RblxUIKey_H;
        case ALLEGRO_KEY_I: return RblxUIKey_I;
        case ALLEGRO_KEY_J: return RblxUIKey_J;
        case ALLEGRO_KEY_K: return RblxUIKey_K;
        case ALLEGRO_KEY_L: return RblxUIKey_L;
        case ALLEGRO_KEY_M: return RblxUIKey_M;
        case ALLEGRO_KEY_N: return RblxUIKey_N;
        case ALLEGRO_KEY_O: return RblxUIKey_O;
        case ALLEGRO_KEY_P: return RblxUIKey_P;
        case ALLEGRO_KEY_Q: return RblxUIKey_Q;
        case ALLEGRO_KEY_R: return RblxUIKey_R;
        case ALLEGRO_KEY_S: return RblxUIKey_S;
        case ALLEGRO_KEY_T: return RblxUIKey_T;
        case ALLEGRO_KEY_U: return RblxUIKey_U;
        case ALLEGRO_KEY_V: return RblxUIKey_V;
        case ALLEGRO_KEY_W: return RblxUIKey_W;
        case ALLEGRO_KEY_X: return RblxUIKey_X;
        case ALLEGRO_KEY_Y: return RblxUIKey_Y;
        case ALLEGRO_KEY_Z: return RblxUIKey_Z;
        case ALLEGRO_KEY_F1: return RblxUIKey_F1;
        case ALLEGRO_KEY_F2: return RblxUIKey_F2;
        case ALLEGRO_KEY_F3: return RblxUIKey_F3;
        case ALLEGRO_KEY_F4: return RblxUIKey_F4;
        case ALLEGRO_KEY_F5: return RblxUIKey_F5;
        case ALLEGRO_KEY_F6: return RblxUIKey_F6;
        case ALLEGRO_KEY_F7: return RblxUIKey_F7;
        case ALLEGRO_KEY_F8: return RblxUIKey_F8;
        case ALLEGRO_KEY_F9: return RblxUIKey_F9;
        case ALLEGRO_KEY_F10: return RblxUIKey_F10;
        case ALLEGRO_KEY_F11: return RblxUIKey_F11;
        case ALLEGRO_KEY_F12: return RblxUIKey_F12;
        default: return RblxUIKey_None;
    }
}

bool RblxUI_ImplAllegro5_Init(ALLEGRO_DISPLAY* display)
{
    RblxUIIO& io = RblxUI::GetIO();
    RBLXUI_CHECKVERSION();
    RBLXUI_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

    // Setup backend capabilities flags
    RblxUI_ImplAllegro5_Data* bd = RBLXUI_NEW(RblxUI_ImplAllegro5_Data)();
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = io.BackendRendererName = "rblxui_impl_allegro5";
    io.BackendFlags |= RblxUIBackendFlags_HasMouseCursors;       // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= RblxUIBackendFlags_RendererHasTextures;   // We can honor RblxUIPlatformIO::Textures[] requests during render.

    bd->LastCursor = ALLEGRO_SYSTEM_MOUSE_CURSOR_NONE;

    RblxUI_ImplAllegro5_SetDisplay(display);

    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
#if ALLEGRO_HAS_CLIPBOARD
    platform_io.Platform_SetClipboardTextFn = RblxUI_ImplAllegro5_SetClipboardText;
    platform_io.Platform_GetClipboardTextFn = RblxUI_ImplAllegro5_GetClipboardText;
#endif
    platform_io.DrawCallback_ResetRenderState = RblxUI_ImplAllegro5_DrawCallback_ResetRenderState;

    return true;
}

void RblxUI_ImplAllegro5_Shutdown()
{
    RblxUI_ImplAllegro5_Data* bd = RblxUI_ImplAllegro5_GetBackendData();
    RBLXUI_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
    RblxUIIO& io = RblxUI::GetIO();
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();

    RblxUI_ImplAllegro5_InvalidateDeviceObjects();
    if (bd->VertexDecl)
        al_destroy_vertex_decl(bd->VertexDecl);
    if (bd->ClipboardTextData)
        al_free(bd->ClipboardTextData);

    io.BackendPlatformName = io.BackendRendererName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(RblxUIBackendFlags_HasMouseCursors | RblxUIBackendFlags_RendererHasTextures);
    platform_io.ClearRendererHandlers();
    platform_io.ClearPlatformHandlers();
    RBLXUI_DELETE(bd);
}

void RblxUI_ImplAllegro5_SetDisplay(ALLEGRO_DISPLAY* display)
{
    RblxUI_ImplAllegro5_Data* bd = RblxUI_ImplAllegro5_GetBackendData();
    bd->Display = display;

    if (bd->VertexDecl)
    {
        al_destroy_vertex_decl(bd->VertexDecl);
        bd->VertexDecl = nullptr;
    }

    if (bd->Display && !bd->VertexDecl)
    {
        // Create custom vertex declaration.
        // Unfortunately Allegro doesn't support 32-bits packed colors so we have to convert them to 4 floats.
        // We still use a custom declaration to use 'ALLEGRO_PRIM_TEX_COORD' instead of 'ALLEGRO_PRIM_TEX_COORD_PIXEL' else we can't do a reliable conversion.
        ALLEGRO_VERTEX_ELEMENT elems[] =
        {
            { ALLEGRO_PRIM_POSITION, ALLEGRO_PRIM_FLOAT_2, offsetof(RblxDrawVertAllegro, pos) },
            { ALLEGRO_PRIM_TEX_COORD, ALLEGRO_PRIM_FLOAT_2, offsetof(RblxDrawVertAllegro, uv) },
            { ALLEGRO_PRIM_COLOR_ATTR, 0, offsetof(RblxDrawVertAllegro, col) },
            { 0, 0, 0 }
        };
        bd->VertexDecl = al_create_vertex_decl(elems, sizeof(RblxDrawVertAllegro));
    }
}

// ev->keyboard.modifiers seems always zero so using that...
static void RblxUI_ImplAllegro5_UpdateKeyModifiers()
{
    RblxUIIO& io = RblxUI::GetIO();
    ALLEGRO_KEYBOARD_STATE keys;
    al_get_keyboard_state(&keys);
    io.AddKeyEvent(RblxUIMod_Ctrl, al_key_down(&keys, ALLEGRO_KEY_LCTRL) || al_key_down(&keys, ALLEGRO_KEY_RCTRL));
    io.AddKeyEvent(RblxUIMod_Shift, al_key_down(&keys, ALLEGRO_KEY_LSHIFT) || al_key_down(&keys, ALLEGRO_KEY_RSHIFT));
    io.AddKeyEvent(RblxUIMod_Alt, al_key_down(&keys, ALLEGRO_KEY_ALT) || al_key_down(&keys, ALLEGRO_KEY_ALTGR));
    io.AddKeyEvent(RblxUIMod_Super, al_key_down(&keys, ALLEGRO_KEY_LWIN) || al_key_down(&keys, ALLEGRO_KEY_RWIN));
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear rblxui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear rblxui, and hide them from your application based on those two flags.
bool RblxUI_ImplAllegro5_ProcessEvent(ALLEGRO_EVENT* ev)
{
    RblxUI_ImplAllegro5_Data* bd = RblxUI_ImplAllegro5_GetBackendData();
    RBLXUI_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call RblxUI_ImplAllegro5_Init()?");
    RblxUIIO& io = RblxUI::GetIO();

    switch (ev->type)
    {
    case ALLEGRO_EVENT_MOUSE_AXES:
        if (ev->mouse.display == bd->Display)
        {
            io.AddMousePosEvent(ev->mouse.x, ev->mouse.y);
            io.AddMouseWheelEvent(-ev->mouse.dw, ev->mouse.dz);
        }
        return true;
    case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
    case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
        if (ev->mouse.display == bd->Display && ev->mouse.button > 0 && ev->mouse.button <= 5)
            io.AddMouseButtonEvent(ev->mouse.button - 1, ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN);
        return true;
    case ALLEGRO_EVENT_TOUCH_MOVE:
        if (ev->touch.display == bd->Display)
            io.AddMousePosEvent(ev->touch.x, ev->touch.y);
        return true;
    case ALLEGRO_EVENT_TOUCH_BEGIN:
    case ALLEGRO_EVENT_TOUCH_END:
    case ALLEGRO_EVENT_TOUCH_CANCEL:
        if (ev->touch.display == bd->Display && ev->touch.primary)
            io.AddMouseButtonEvent(0, ev->type == ALLEGRO_EVENT_TOUCH_BEGIN);
        return true;
    case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
        if (ev->mouse.display == bd->Display)
            io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        return true;
    case ALLEGRO_EVENT_KEY_CHAR:
        if (ev->keyboard.display == bd->Display)
            if (ev->keyboard.unichar != 0)
                io.AddInputCharacter((unsigned int)ev->keyboard.unichar);
        return true;
    case ALLEGRO_EVENT_KEY_DOWN:
    case ALLEGRO_EVENT_KEY_UP:
        if (ev->keyboard.display == bd->Display)
        {
            RblxUI_ImplAllegro5_UpdateKeyModifiers();
            RblxUIKey key = RblxUI_ImplAllegro5_KeyCodeToRblxUIKey(ev->keyboard.keycode);
            io.AddKeyEvent(key, (ev->type == ALLEGRO_EVENT_KEY_DOWN));
            io.SetKeyEventNativeData(key, ev->keyboard.keycode, -1); // To support legacy indexing (<1.87 user code)
        }
        return true;
    case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
        if (ev->display.source == bd->Display)
            io.AddFocusEvent(false);
        return true;
    case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
        if (ev->display.source == bd->Display)
        {
            io.AddFocusEvent(true);
#if defined(ALLEGRO_UNSTABLE)
            al_clear_keyboard_state(bd->Display);
#endif
        }
        return true;
    }
    return false;
}

static void RblxUI_ImplAllegro5_UpdateMouseCursor()
{
    RblxUIIO& io = RblxUI::GetIO();
    if (io.ConfigFlags & RblxUIConfigFlags_NoMouseCursorChange)
        return;

    RblxUI_ImplAllegro5_Data* bd = RblxUI_ImplAllegro5_GetBackendData();
    RblxUIMouseCursor rblxui_cursor = RblxUI::GetMouseCursor();

    // Hide OS mouse cursor if rblxui is drawing it
    if (io.MouseDrawCursor)
        rblxui_cursor = RblxUIMouseCursor_None;

    if (bd->LastCursor == rblxui_cursor)
        return;
    bd->LastCursor = rblxui_cursor;
    if (rblxui_cursor == RblxUIMouseCursor_None)
    {
        al_set_mouse_cursor(bd->Display, bd->MouseCursorInvisible);
    }
    else
    {
        ALLEGRO_SYSTEM_MOUSE_CURSOR cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT;
        switch (rblxui_cursor)
        {
        case RblxUIMouseCursor_TextInput:    cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_EDIT; break;
        case RblxUIMouseCursor_ResizeAll:    cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_MOVE; break;
        case RblxUIMouseCursor_ResizeNS:     cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_N; break;
        case RblxUIMouseCursor_ResizeEW:     cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_E; break;
        case RblxUIMouseCursor_ResizeNESW:   cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_NE; break;
        case RblxUIMouseCursor_ResizeNWSE:   cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_NW; break;
        case RblxUIMouseCursor_Wait:         cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_BUSY; break;
        case RblxUIMouseCursor_Progress:     cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_PROGRESS; break;
        case RblxUIMouseCursor_NotAllowed:   cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_UNAVAILABLE; break;
        }
        al_set_system_mouse_cursor(bd->Display, cursor_id);
    }
}

void RblxUI_ImplAllegro5_NewFrame()
{
    RblxUI_ImplAllegro5_Data* bd = RblxUI_ImplAllegro5_GetBackendData();
    RBLXUI_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call RblxUI_ImplAllegro5_Init()?");

    if (!bd->MouseCursorInvisible)
        RblxUI_ImplAllegro5_CreateDeviceObjects();

    // Setup display size (every frame to accommodate for window resizing)
    RblxUIIO& io = RblxUI::GetIO();
    int w, h;
    w = al_get_display_width(bd->Display);
    h = al_get_display_height(bd->Display);
    io.DisplaySize = RblxVec2((float)w, (float)h);

    // Setup time step
    double current_time = al_get_time();
    io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
    bd->Time = current_time;

    // Allegro 5 doesn't receive PrintScreen under Windows
#ifdef _WIN32
    io.AddKeyEvent(RblxUIKey_PrintScreen, (::GetAsyncKeyState(VK_SNAPSHOT) & 0x8000) != 0);
#endif

    // Setup mouse cursor shape
    RblxUI_ImplAllegro5_UpdateMouseCursor();
}

//-----------------------------------------------------------------------------

#endif // #ifndef RBLXUI_DISABLE
