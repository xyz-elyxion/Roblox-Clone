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

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2026-XX-XX: Platform: Added support for multiple windows via the LinearUIPlatformIO interface.
//  2026-04-23: Added support for standard draw callbacks (in platform_io): DrawCallback_ResetRenderState, DrawCallback_SetSamplerLinear, DrawCallback_SetSamplerNearest. (#9378)
//  2026-03-19: Fixed issue in LinearUI_ImplDX9_UpdateTexture() if LinearTextureID_Invalid is defined to be != 0, which became the default since 2026-03-12. (#9295, #9310)
//  2025-09-18: Call platform_io.ClearRendererHandlers() on shutdown.
//  2025-06-11: DirectX9: Added support for LinearUIBackendFlags_RendererHasTextures, for dynamic font atlas.
//  2024-10-07: DirectX9: Changed default texture sampler to Clamp instead of Repeat/Wrap.
//  2024-02-12: DirectX9: Using RGBA format when supported by the driver to avoid CPU side conversion. (#6575)
//  2022-10-11: Using 'nullptr' instead of 'NULL' as per our switch to C++11.
//  2021-06-29: Reorganized backend to pull data from a single structure to facilitate usage with multiple-contexts (all g_XXXX access changed to bd->XXXX).
//  2021-06-25: DirectX9: Explicitly disable texture state stages after >= 1.
//  2021-05-19: DirectX9: Replaced direct access to LinearDrawCmd::TextureId with a call to LinearDrawCmd::GetTexID(). (will become a requirement)
//  2021-04-23: DirectX9: Explicitly setting up more graphics states to increase compatibility with unusual non-default states.
//  2021-03-18: DirectX9: Calling IDirect3DStateBlock9::Capture() after CreateStateBlock() as a workaround for state restoring issues (see #3857).
//  2021-03-03: DirectX9: Added support for LINEARUI_USE_BGRA_PACKED_COLOR in user's linearuiconfig file.
//  2021-02-18: DirectX9: Change blending equation to preserve alpha in output buffer.
//  2019-05-29: DirectX9: Added support for large mesh (64K+ vertices), enable LinearUIBackendFlags_RendererHasVtxOffset flag.
//  2019-04-30: DirectX9: Added support for special LinearDrawCallback_ResetRenderState callback to reset render state.
//  2019-03-29: Misc: Fixed erroneous assert in LinearUI_ImplDX9_InvalidateDeviceObjects().
//  2019-01-16: Misc: Disabled fog before drawing UI's. Fixes issue #2288.
//  2018-11-30: Misc: Setting up io.BackendRendererName so it can be displayed in the About Window.
//  2018-06-08: Misc: Extracted linearui_impl_dx9.cpp/.h away from the old combined DX9+Win32 example.
//  2018-06-08: DirectX9: Use draw_data->DisplayPos and draw_data->DisplaySize to setup projection matrix and clipping rectangle.
//  2018-05-07: Render: Saving/restoring Transform because they don't seem to be included in the StateBlock. Setting shading mode to Gouraud.
//  2018-02-16: Misc: Obsoleted the io.RenderDrawListsFn callback and exposed LinearUI_ImplDX9_RenderDrawData() in the .h file so you can call it yourself.
//  2018-02-06: Misc: Removed call to LinearUI::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.

#include "linearui.h"
#ifndef LINEARUI_DISABLE
#include "linearui_impl_dx9.h"

// DirectX
#include <d3d9.h>

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning: use of old-style cast                            // yes, they are more terse.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning: implicit conversion changes signedness
#endif

// DirectX data
struct LinearUI_ImplDX9_Data
{
    LPDIRECT3DDEVICE9           pd3dDevice;
    LPDIRECT3DVERTEXBUFFER9     pVB;
    LPDIRECT3DINDEXBUFFER9      pIB;
    int                         VertexBufferSize;
    int                         IndexBufferSize;
    bool                        HasRgbaSupport;

    LinearUI_ImplDX9_Data()        { memset((void*)this, 0, sizeof(*this)); VertexBufferSize = 5000; IndexBufferSize = 10000; }
};

struct CUSTOMVERTEX
{
    float    pos[3];
    D3DCOLOR col;
    float    uv[2];
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

#ifdef LINEARUI_USE_BGRA_PACKED_COLOR
#define LINEARUI_COL_TO_DX9_ARGB(_COL)     (_COL)
#else
#define LINEARUI_COL_TO_DX9_ARGB(_COL)     (((_COL) & 0xFF00FF00) | (((_COL) & 0xFF0000) >> 16) | (((_COL) & 0xFF) << 16))
#endif

// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear LinearUI contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear LinearUI context + multiple windows) instead of multiple Dear LinearUI contexts.
static LinearUI_ImplDX9_Data* LinearUI_ImplDX9_GetBackendData()
{
    return LinearUI::GetCurrentContext() ? (LinearUI_ImplDX9_Data*)LinearUI::GetIO().BackendRendererUserData : nullptr;
}

// Forward Declarations
static void LinearUI_ImplDX9_InitMultiViewportSupport();
static void LinearUI_ImplDX9_ShutdownMultiViewportSupport();
static void LinearUI_ImplDX9_CreateDeviceObjectsForPlatformWindows();
static void LinearUI_ImplDX9_InvalidateDeviceObjectsForPlatformWindows();

// Functions
static void LinearUI_ImplDX9_SetupRenderState(LinearDrawData* draw_data)
{
    LinearUI_ImplDX9_Data* bd = LinearUI_ImplDX9_GetBackendData();

    // Setup viewport
    D3DVIEWPORT9 vp;
    vp.X = vp.Y = 0;
    vp.Width = (DWORD)draw_data->DisplaySize.x;
    vp.Height = (DWORD)draw_data->DisplaySize.y;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;

    LPDIRECT3DDEVICE9 device = bd->pd3dDevice;
    device->SetViewport(&vp);

    // Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing, shade mode (for gradient), bilinear sampling.
    device->SetPixelShader(nullptr);
    device->SetVertexShader(nullptr);
    device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
    device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
    device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    device->SetRenderState(D3DRS_FOGENABLE, FALSE);
    device->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
    device->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    device->SetRenderState(D3DRS_CLIPPING, TRUE);
    device->SetRenderState(D3DRS_LIGHTING, FALSE);
    device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    // Setup orthographic projection matrix
    // Our visible linearui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    // Being agnostic of whether <d3dx9.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
    {
        float L = draw_data->DisplayPos.x + 0.5f;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x + 0.5f;
        float T = draw_data->DisplayPos.y + 0.5f;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y + 0.5f;
        D3DMATRIX mat_identity = { { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } } };
        D3DMATRIX mat_projection =
        { { {
            2.0f/(R-L),   0.0f,         0.0f,  0.0f,
            0.0f,         2.0f/(T-B),   0.0f,  0.0f,
            0.0f,         0.0f,         0.5f,  0.0f,
            (L+R)/(L-R),  (T+B)/(B-T),  0.5f,  1.0f
        } } };
        device->SetTransform(D3DTS_WORLD, &mat_identity);
        device->SetTransform(D3DTS_VIEW, &mat_identity);
        device->SetTransform(D3DTS_PROJECTION, &mat_projection);
    }
}

// Draw callbacks
static void LinearUI_ImplDX9_DrawCallback_ResetRenderState(const LinearDrawList*, const LinearDrawCmd*)    {} // Intentionally empty. Used as an identifier for rendering loop to call its code. Simpler to implement this way.
static void LinearUI_ImplDX9_DrawCallback_SetSamplerLinear(const LinearDrawList*, const LinearDrawCmd*)    { LinearUI_ImplDX9_Data* bd = LinearUI_ImplDX9_GetBackendData(); bd->pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR); bd->pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR); }
static void LinearUI_ImplDX9_DrawCallback_SetSamplerNearest(const LinearDrawList*, const LinearDrawCmd*)   { LinearUI_ImplDX9_Data* bd = LinearUI_ImplDX9_GetBackendData(); bd->pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT); bd->pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT); }

// Render function.
void LinearUI_ImplDX9_RenderDrawData(LinearDrawData* draw_data)
{
    // Avoid rendering when minimized
    if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    LinearUI_ImplDX9_Data* bd = LinearUI_ImplDX9_GetBackendData();
    LPDIRECT3DDEVICE9 device = bd->pd3dDevice;

    // Catch up with texture updates. Most of the times, the list will have 1 element with an OK status, aka nothing to do.
    // (This almost always points to LinearUI::GetPlatformIO().Textures[] but is part of LinearDrawData to allow overriding or disabling texture updates).
    if (draw_data->Textures != nullptr)
        for (LinearTextureData* tex : *draw_data->Textures)
            if (tex->Status != LinearTextureStatus_OK)
                LinearUI_ImplDX9_UpdateTexture(tex);

    // Create and grow buffers if needed
    if (!bd->pVB || bd->VertexBufferSize < draw_data->TotalVtxCount)
    {
        if (bd->pVB) { bd->pVB->Release(); bd->pVB = nullptr; }
        bd->VertexBufferSize = draw_data->TotalVtxCount + 5000;
        if (device->CreateVertexBuffer((UINT)bd->VertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &bd->pVB, nullptr) < 0)
            return;
    }
    if (!bd->pIB || bd->IndexBufferSize < draw_data->TotalIdxCount)
    {
        if (bd->pIB) { bd->pIB->Release(); bd->pIB = nullptr; }
        bd->IndexBufferSize = draw_data->TotalIdxCount + 10000;
        if (device->CreateIndexBuffer((UINT)bd->IndexBufferSize * sizeof(LinearDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(LinearDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &bd->pIB, nullptr) < 0)
            return;
    }

    // Backup the DX9 state
    IDirect3DStateBlock9* state_block = nullptr;
    if (device->CreateStateBlock(D3DSBT_ALL, &state_block) < 0)
        return;
    if (state_block->Capture() < 0)
    {
        state_block->Release();
        return;
    }

    // Backup the DX9 transform (DX9 documentation suggests that it is included in the StateBlock but it doesn't appear to)
    D3DMATRIX last_world, last_view, last_projection;
    device->GetTransform(D3DTS_WORLD, &last_world);
    device->GetTransform(D3DTS_VIEW, &last_view);
    device->GetTransform(D3DTS_PROJECTION, &last_projection);

    // Allocate buffers
    CUSTOMVERTEX* vtx_dst;
    LinearDrawIdx* idx_dst;
    if (bd->pVB->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(CUSTOMVERTEX)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
    {
        state_block->Release();
        return;
    }
    if (bd->pIB->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(LinearDrawIdx)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
    {
        bd->pVB->Unlock();
        state_block->Release();
        return;
    }

    // Copy and convert all vertices into a single contiguous buffer, convert colors to DX9 default format.
    // FIXME-OPT: This is a minor waste of resource, the ideal is to use linearuiconfig.h and
    //  1) to avoid repacking colors:   #define LINEARUI_USE_BGRA_PACKED_COLOR
    //  2) to avoid repacking vertices: #define LINEARUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT struct LinearDrawVert { LinearVec2 pos; float z; LinearU32 col; LinearVec2 uv; }
    for (const LinearDrawList* draw_list : draw_data->CmdLists)
    {
        const LinearDrawVert* vtx_src = draw_list->VtxBuffer.Data;
        for (int i = 0; i < draw_list->VtxBuffer.Size; i++)
        {
            vtx_dst->pos[0] = vtx_src->pos.x;
            vtx_dst->pos[1] = vtx_src->pos.y;
            vtx_dst->pos[2] = 0.0f;
            vtx_dst->col = LINEARUI_COL_TO_DX9_ARGB(vtx_src->col);
            vtx_dst->uv[0] = vtx_src->uv.x;
            vtx_dst->uv[1] = vtx_src->uv.y;
            vtx_dst++;
            vtx_src++;
        }
        memcpy(idx_dst, draw_list->IdxBuffer.Data, draw_list->IdxBuffer.Size * sizeof(LinearDrawIdx));
        idx_dst += draw_list->IdxBuffer.Size;
    }
    bd->pVB->Unlock();
    bd->pIB->Unlock();
    device->SetStreamSource(0, bd->pVB, 0, sizeof(CUSTOMVERTEX));
    device->SetIndices(bd->pIB);
    device->SetFVF(D3DFVF_CUSTOMVERTEX);

    // Setup desired DX state
    LinearUI_ImplDX9_SetupRenderState(draw_data);

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_vtx_offset = 0;
    int global_idx_offset = 0;
    LinearVec2 clip_off = draw_data->DisplayPos;
    for (const LinearDrawList* draw_list : draw_data->CmdLists)
    {
        for (int cmd_i = 0; cmd_i < draw_list->CmdBuffer.Size; cmd_i++)
        {
            const LinearDrawCmd* pcmd = &draw_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr)
            {
                // User callback, registered via LinearDrawList::AddCallback()
                if (pcmd->UserCallback == LinearUI_ImplDX9_DrawCallback_ResetRenderState)
                    LinearUI_ImplDX9_SetupRenderState(draw_data);
                else
                    pcmd->UserCallback(draw_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                LinearVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
                LinearVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                // Apply scissor/clipping rectangle
                const RECT r = { (LONG)clip_min.x, (LONG)clip_min.y, (LONG)clip_max.x, (LONG)clip_max.y };
                device->SetScissorRect(&r);

                // Bind texture, Draw
                const LPDIRECT3DTEXTURE9 texture = (LPDIRECT3DTEXTURE9)pcmd->GetTexID();
                device->SetTexture(0, texture);
                device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, pcmd->VtxOffset + global_vtx_offset, 0, (UINT)draw_list->VtxBuffer.Size, pcmd->IdxOffset + global_idx_offset, pcmd->ElemCount / 3);
            }
        }
        global_idx_offset += draw_list->IdxBuffer.Size;
        global_vtx_offset += draw_list->VtxBuffer.Size;
    }

    // When using multi-viewports, it appears that there's an odd logic in DirectX9 which prevent subsequent windows
    // from rendering until the first window submits at least one draw call, even once. That's our workaround. (see #2560)
    if (global_vtx_offset == 0)
        bd->pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 0, 0, 0);

    // Restore the DX9 transform
    device->SetTransform(D3DTS_WORLD, &last_world);
    device->SetTransform(D3DTS_VIEW, &last_view);
    device->SetTransform(D3DTS_PROJECTION, &last_projection);

    // Restore the DX9 state
    state_block->Apply();
    state_block->Release();
}

static bool LinearUI_ImplDX9_CheckFormatSupport(LPDIRECT3DDEVICE9 pDevice, D3DFORMAT format)
{
    LPDIRECT3D9 pd3d = nullptr;
    if (pDevice->GetDirect3D(&pd3d) != D3D_OK)
        return false;
    D3DDEVICE_CREATION_PARAMETERS param = {};
    D3DDISPLAYMODE mode = {};
    if (pDevice->GetCreationParameters(&param) != D3D_OK || pDevice->GetDisplayMode(0, &mode) != D3D_OK)
    {
        pd3d->Release();
        return false;
    }
    // Font texture should support linear filter, color blend and write to render-target
    bool support = (pd3d->CheckDeviceFormat(param.AdapterOrdinal, param.DeviceType, mode.Format, D3DUSAGE_DYNAMIC | D3DUSAGE_QUERY_FILTER | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, format)) == D3D_OK;
    pd3d->Release();
    return support;
}

// Convert RGBA32 to BGRA32 (because RGBA32 is not well supported by DX9 devices)
static void LinearUI_ImplDX9_CopyTextureRegion(bool tex_use_colors, const LinearU32* src, int src_pitch, LinearU32* dst, int dst_pitch, int w, int h)
{
#ifndef LINEARUI_USE_BGRA_PACKED_COLOR
    LinearUI_ImplDX9_Data* bd = LinearUI_ImplDX9_GetBackendData();
    const bool convert_rgba_to_bgra = (!bd->HasRgbaSupport && tex_use_colors);
#else
    const bool convert_rgba_to_bgra = false;
    LINEARUI_UNUSED(tex_use_colors);
#endif
    for (int y = 0; y < h; y++)
    {
        const LinearU32* src_p = (const LinearU32*)(const void*)((const unsigned char*)src + src_pitch * y);
        LinearU32* dst_p = (LinearU32*)(void*)((unsigned char*)dst + dst_pitch * y);
        if (convert_rgba_to_bgra)
            for (int x = w; x > 0; x--, src_p++, dst_p++) // Convert copy
                *dst_p = LINEARUI_COL_TO_DX9_ARGB(*src_p);
        else
            memcpy(dst_p, src_p, w * 4); // Raw copy
    }
}

void LinearUI_ImplDX9_UpdateTexture(LinearTextureData* tex)
{
    LinearUI_ImplDX9_Data* bd = LinearUI_ImplDX9_GetBackendData();

    if (tex->Status == LinearTextureStatus_WantCreate)
    {
        // Create and upload new texture to graphics system
        //LINEARUI_DEBUG_LOG("UpdateTexture #%03d: WantCreate %dx%d\n", tex->UniqueID, tex->Width, tex->Height);
        LINEARUI_ASSERT(tex->TexID == LinearTextureID_Invalid && tex->BackendUserData == nullptr);
        LINEARUI_ASSERT(tex->Format == LinearTextureFormat_RGBA32);
        LPDIRECT3DTEXTURE9 dx_tex = nullptr;
        HRESULT hr = bd->pd3dDevice->CreateTexture(tex->Width, tex->Height, 1, D3DUSAGE_DYNAMIC, bd->HasRgbaSupport ? D3DFMT_A8B8G8R8 : D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &dx_tex, nullptr);
        if (hr < 0)
        {
            LINEARUI_ASSERT(hr >= 0 && "Backend failed to create texture!");
            return;
        }

        D3DLOCKED_RECT locked_rect;
        if (dx_tex->LockRect(0, &locked_rect, nullptr, 0) == D3D_OK)
        {
            LinearUI_ImplDX9_CopyTextureRegion(tex->UseColors, (LinearU32*)tex->GetPixels(), tex->Width * 4, (LinearU32*)locked_rect.pBits, (LinearU32)locked_rect.Pitch, tex->Width, tex->Height);
            dx_tex->UnlockRect(0);
        }

        // Store identifiers
        tex->SetTexID((LinearTextureID)(intptr_t)dx_tex);
        tex->SetStatus(LinearTextureStatus_OK);
    }
    else if (tex->Status == LinearTextureStatus_WantUpdates)
    {
        // Update selected blocks. We only ever write to textures regions which have never been used before!
        // This backend choose to use tex->Updates[] but you can use tex->UpdateRect to upload a single region.
        LPDIRECT3DTEXTURE9 backend_tex = (LPDIRECT3DTEXTURE9)(intptr_t)tex->TexID;
        RECT update_rect = { (LONG)tex->UpdateRect.x, (LONG)tex->UpdateRect.y, (LONG)(tex->UpdateRect.x + tex->UpdateRect.w), (LONG)(tex->UpdateRect.y + tex->UpdateRect.h) };
        D3DLOCKED_RECT locked_rect;
        if (backend_tex->LockRect(0, &locked_rect, &update_rect, 0) == D3D_OK)
            for (LinearTextureRect& r : tex->Updates)
                LinearUI_ImplDX9_CopyTextureRegion(tex->UseColors, (LinearU32*)tex->GetPixelsAt(r.x, r.y), tex->Width * 4,
                    (LinearU32*)locked_rect.pBits + (r.x - update_rect.left) + (r.y - update_rect.top) * (locked_rect.Pitch / 4), (int)locked_rect.Pitch, r.w, r.h);
        backend_tex->UnlockRect(0);
        tex->SetStatus(LinearTextureStatus_OK);
    }
    else if (tex->Status == LinearTextureStatus_WantDestroy)
    {
        if (tex->TexID != LinearTextureID_Invalid)
            if (LPDIRECT3DTEXTURE9 backend_tex = (LPDIRECT3DTEXTURE9)tex->TexID)
            {
                LINEARUI_ASSERT(tex->TexID == (LinearTextureID)(intptr_t)backend_tex);
                backend_tex->Release();

                // Clear identifiers and mark as destroyed (in order to allow e.g. calling InvalidateDeviceObjects while running)
                tex->SetTexID(LinearTextureID_Invalid);
            }
        tex->SetStatus(LinearTextureStatus_Destroyed);
    }
}

bool LinearUI_ImplDX9_CreateDeviceObjects()
{
    LinearUI_ImplDX9_Data* bd = LinearUI_ImplDX9_GetBackendData();
    if (!bd || !bd->pd3dDevice)
        return false;
    LinearUI_ImplDX9_CreateDeviceObjectsForPlatformWindows();
    return true;
}

void LinearUI_ImplDX9_InvalidateDeviceObjects()
{
    LinearUI_ImplDX9_Data* bd = LinearUI_ImplDX9_GetBackendData();
    if (!bd || !bd->pd3dDevice)
        return;

    // Destroy all textures
    for (LinearTextureData* tex : LinearUI::GetPlatformIO().Textures)
        if (tex->RefCount == 1)
        {
            tex->SetStatus(LinearTextureStatus_WantDestroy);
            LinearUI_ImplDX9_UpdateTexture(tex);
        }
    if (bd->pVB) { bd->pVB->Release(); bd->pVB = nullptr; }
    if (bd->pIB) { bd->pIB->Release(); bd->pIB = nullptr; }
    LinearUI_ImplDX9_InvalidateDeviceObjectsForPlatformWindows();
}

void LinearUI_ImplDX9_NewFrame()
{
    LinearUI_ImplDX9_Data* bd = LinearUI_ImplDX9_GetBackendData();
    LINEARUI_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call LinearUI_ImplDX9_Init()?");
    LINEARUI_UNUSED(bd);
}

bool LinearUI_ImplDX9_Init(IDirect3DDevice9* device)
{
    LinearUIIO& io = LinearUI::GetIO();
    LINEARUI_CHECKVERSION();
    LINEARUI_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    LinearUI_ImplDX9_Data* bd = LINEARUI_NEW(LinearUI_ImplDX9_Data)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = "linearui_impl_dx9";
    io.BackendFlags |= LinearUIBackendFlags_RendererHasVtxOffset;  // We can honor the LinearDrawCmd::VtxOffset field, allowing for large meshes.
    io.BackendFlags |= LinearUIBackendFlags_RendererHasTextures;   // We can honor LinearUIPlatformIO::Textures[] requests during render.
    io.BackendFlags |= LinearUIBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)

    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
    platform_io.Renderer_TextureMaxWidth = platform_io.Renderer_TextureMaxHeight = 4096;
    platform_io.DrawCallback_ResetRenderState = LinearUI_ImplDX9_DrawCallback_ResetRenderState;
    platform_io.DrawCallback_SetSamplerLinear = LinearUI_ImplDX9_DrawCallback_SetSamplerLinear;
    platform_io.DrawCallback_SetSamplerNearest = LinearUI_ImplDX9_DrawCallback_SetSamplerNearest;

    bd->pd3dDevice = device;
    bd->pd3dDevice->AddRef();
    bd->HasRgbaSupport = LinearUI_ImplDX9_CheckFormatSupport(bd->pd3dDevice, D3DFMT_A8B8G8R8);

    LinearUI_ImplDX9_InitMultiViewportSupport();

    return true;
}

void LinearUI_ImplDX9_Shutdown()
{
    LinearUI_ImplDX9_Data* bd = LinearUI_ImplDX9_GetBackendData();
    LINEARUI_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
    LinearUIIO& io = LinearUI::GetIO();
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();

    LinearUI_ImplDX9_ShutdownMultiViewportSupport();
    LinearUI_ImplDX9_InvalidateDeviceObjects();
    if (bd->pd3dDevice) { bd->pd3dDevice->Release(); }

    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags &= ~(LinearUIBackendFlags_RendererHasVtxOffset | LinearUIBackendFlags_RendererHasTextures | LinearUIBackendFlags_RendererHasViewports);
    platform_io.ClearRendererHandlers();
    LINEARUI_DELETE(bd);
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously.
// If you are new to dear linearui or creating a new binding for dear linearui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

// Helper structure we store in the void* RendererUserData field of each LinearUIViewport to easily retrieve our backend data.
struct LinearUI_ImplDX9_ViewportData
{
    IDirect3DSwapChain9*    SwapChain;
    D3DPRESENT_PARAMETERS   d3dpp;

    LinearUI_ImplDX9_ViewportData()  { SwapChain = nullptr; ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS)); }
    ~LinearUI_ImplDX9_ViewportData() { LINEARUI_ASSERT(SwapChain == nullptr); }
};

static void LinearUI_ImplDX9_CreateWindow(LinearUIViewport* viewport)
{
    LinearUI_ImplDX9_Data* bd = LinearUI_ImplDX9_GetBackendData();
    LinearUI_ImplDX9_ViewportData* vd = LINEARUI_NEW(LinearUI_ImplDX9_ViewportData)();
    viewport->RendererUserData = vd;

    // PlatformHandleRaw should always be a HWND, whereas PlatformHandle might be a higher-level handle (e.g. LINEARWINWindow*, SDL's WindowID).
    // Some backends will leave PlatformHandleRaw == 0, in which case we assume PlatformHandle will contain the HWND.
    HWND hwnd = viewport->PlatformHandleRaw ? (HWND)viewport->PlatformHandleRaw : (HWND)viewport->PlatformHandle;
    LINEARUI_ASSERT(hwnd != 0);

    ZeroMemory(&vd->d3dpp, sizeof(D3DPRESENT_PARAMETERS));
    vd->d3dpp.Windowed = TRUE;
    vd->d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    vd->d3dpp.BackBufferWidth = (UINT)viewport->Size.x;
    vd->d3dpp.BackBufferHeight = (UINT)viewport->Size.y;
    vd->d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    vd->d3dpp.hDeviceWindow = hwnd;
    vd->d3dpp.EnableAutoDepthStencil = FALSE;
    vd->d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    vd->d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync

    HRESULT hr = bd->pd3dDevice->CreateAdditionalSwapChain(&vd->d3dpp, &vd->SwapChain); LINEARUI_UNUSED(hr);
    LINEARUI_ASSERT(hr == D3D_OK);
    LINEARUI_ASSERT(vd->SwapChain != nullptr);
}

static void LinearUI_ImplDX9_DestroyWindow(LinearUIViewport* viewport)
{
    // The main viewport (owned by the application) will always have RendererUserData == 0 since we didn't create the data for it.
    if (LinearUI_ImplDX9_ViewportData* vd = (LinearUI_ImplDX9_ViewportData*)viewport->RendererUserData)
    {
        if (vd->SwapChain)
            vd->SwapChain->Release();
        vd->SwapChain = nullptr;
        ZeroMemory(&vd->d3dpp, sizeof(D3DPRESENT_PARAMETERS));
        LINEARUI_DELETE(vd);
    }
    viewport->RendererUserData = nullptr;
}

static void LinearUI_ImplDX9_SetWindowSize(LinearUIViewport* viewport, LinearVec2 size)
{
    LinearUI_ImplDX9_Data* bd = LinearUI_ImplDX9_GetBackendData();
    LinearUI_ImplDX9_ViewportData* vd = (LinearUI_ImplDX9_ViewportData*)viewport->RendererUserData;
    if (vd->SwapChain)
    {
        vd->SwapChain->Release();
        vd->SwapChain = nullptr;
        vd->d3dpp.BackBufferWidth = (UINT)size.x;
        vd->d3dpp.BackBufferHeight = (UINT)size.y;
        HRESULT hr = bd->pd3dDevice->CreateAdditionalSwapChain(&vd->d3dpp, &vd->SwapChain); LINEARUI_UNUSED(hr);
        LINEARUI_ASSERT(hr == D3D_OK);
    }
}

static void LinearUI_ImplDX9_RenderWindow(LinearUIViewport* viewport, void*)
{
    LinearUI_ImplDX9_Data* bd = LinearUI_ImplDX9_GetBackendData();
    LinearUI_ImplDX9_ViewportData* vd = (LinearUI_ImplDX9_ViewportData*)viewport->RendererUserData;
    LinearVec4 clear_color = LinearVec4(0.0f, 0.0f, 0.0f, 1.0f);

    LPDIRECT3DSURFACE9 render_target = nullptr;
    LPDIRECT3DSURFACE9 last_render_target = nullptr;
    LPDIRECT3DSURFACE9 last_depth_stencil = nullptr;
    vd->SwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &render_target);
    bd->pd3dDevice->GetRenderTarget(0, &last_render_target);
    bd->pd3dDevice->GetDepthStencilSurface(&last_depth_stencil);
    bd->pd3dDevice->SetRenderTarget(0, render_target);
    bd->pd3dDevice->SetDepthStencilSurface(nullptr);

    if (!(viewport->Flags & LinearUIViewportFlags_NoRendererClear))
    {
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x*255.0f), (int)(clear_color.y*255.0f), (int)(clear_color.z*255.0f), (int)(clear_color.w*255.0f));
        bd->pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, clear_col_dx, 1.0f, 0);
    }

    LinearUI_ImplDX9_RenderDrawData(viewport->DrawData);

    // Restore render target
    bd->pd3dDevice->SetRenderTarget(0, last_render_target);
    bd->pd3dDevice->SetDepthStencilSurface(last_depth_stencil);
    render_target->Release();
    last_render_target->Release();
    if (last_depth_stencil) last_depth_stencil->Release();
}

static void LinearUI_ImplDX9_SwapBuffers(LinearUIViewport* viewport, void*)
{
    LinearUI_ImplDX9_ViewportData* vd = (LinearUI_ImplDX9_ViewportData*)viewport->RendererUserData;
    HRESULT hr = vd->SwapChain->Present(nullptr, nullptr, vd->d3dpp.hDeviceWindow, nullptr, 0);
    // Let main application handle D3DERR_DEVICELOST by resetting the device.
    LINEARUI_ASSERT(SUCCEEDED(hr) || hr == D3DERR_DEVICELOST);
}

static void LinearUI_ImplDX9_InitMultiViewportSupport()
{
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
    platform_io.Renderer_CreateWindow = LinearUI_ImplDX9_CreateWindow;
    platform_io.Renderer_DestroyWindow = LinearUI_ImplDX9_DestroyWindow;
    platform_io.Renderer_SetWindowSize = LinearUI_ImplDX9_SetWindowSize;
    platform_io.Renderer_RenderWindow = LinearUI_ImplDX9_RenderWindow;
    platform_io.Renderer_SwapBuffers = LinearUI_ImplDX9_SwapBuffers;
}

static void LinearUI_ImplDX9_ShutdownMultiViewportSupport()
{
    LinearUI::DestroyPlatformWindows();
}

static void LinearUI_ImplDX9_CreateDeviceObjectsForPlatformWindows()
{
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
    for (int i = 1; i < platform_io.Viewports.Size; i++)
        if (!platform_io.Viewports[i]->RendererUserData)
            LinearUI_ImplDX9_CreateWindow(platform_io.Viewports[i]);
}

static void LinearUI_ImplDX9_InvalidateDeviceObjectsForPlatformWindows()
{
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
    for (int i = 1; i < platform_io.Viewports.Size; i++)
        if (platform_io.Viewports[i]->RendererUserData)
            LinearUI_ImplDX9_DestroyWindow(platform_io.Viewports[i]);
}

//-----------------------------------------------------------------------------

#endif // #ifndef LINEARUI_DISABLE
