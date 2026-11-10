// dear rblxui: Renderer for WebGPU
// This needs to be used along with a Platform Binding (e.g. RBLXWIN, SDL2, SDL3)
// (Please note that WebGPU is a recent API, may not be supported by all browser, and its ecosystem is generally a mess)

// When targeting native platforms:
//  - One of RBLXUI_IMPL_WEBGPU_BACKEND_DAWN, RBLXUI_IMPL_WEBGPU_BACKEND_WGPU or RBLXUI_IMPL_WEBGPU_BACKEND_WGVK *must* be provided.
// When targeting Emscripten:
//  - We now defaults to RBLXUI_IMPL_WEBGPU_BACKEND_DAWN and requires Emscripten 4.0.10+, which correspond to using Emscripten '--use-port=emdawnwebgpu'.
//  - Emscripten < 4.0.10 is not supported anymore (old '-sUSE_WEBGPU=1' option).
//  - We can still define RBLXUI_IMPL_WEBGPU_BACKEND_WGPU to use Emscripten '-s USE_WEBGPU=1' which is marked as obsolete by Emscripten.
// Add #define to your rblxuiconfig.h file, or as a compilation flag in your build system.
// This requirement may be removed once WebGPU stabilizes and backends converge on a unified interface.
//#define RBLXUI_IMPL_WEBGPU_BACKEND_DAWN
//#define RBLXUI_IMPL_WEBGPU_BACKEND_WGPU
//#define RBLXUI_IMPL_WEBGPU_BACKEND_WGVK

// Implemented features:
//  [X] Renderer: User texture binding. Use 'WGPUTextureView' as RblxTextureID. Read the FAQ about RblxTextureID/RblxTextureRef!
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (RblxUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Expose selected render state for draw callbacks to use. Access in '(RblxUI_ImplXXXX_RenderState*)GetPlatformIO().Renderer_RenderState'.
//  [X] Renderer: Texture updates support for dynamic font system (RblxUIBackendFlags_RendererHasTextures).
// Missing features or Issues:
//  [ ] Renderer: Multi-viewport support (multiple windows), useful for desktop.

// You can use unmodified rblxui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire rblxui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear RblxUI:
// - FAQ                  https://dearrblxui.com/faq
// - Getting Started      https://dearrblxui.com/getting-started
// - Documentation        https://dearrblxui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of rblxui.cpp

#pragma once
#include "rblxui.h"          // RBLXUI_IMPL_API
#ifndef RBLXUI_DISABLE

// Setup Emscripten default if not specified.
#if defined(__EMSCRIPTEN__) && !defined(RBLXUI_IMPL_WEBGPU_BACKEND_DAWN) && !defined(RBLXUI_IMPL_WEBGPU_BACKEND_WGPU)
#include <emscripten/version.h>
#define RBLXUI_IMPL_WEBGPU_BACKEND_DAWN
#endif

#include <webgpu/webgpu.h>
#if defined(RBLXUI_IMPL_WEBGPU_BACKEND_WGPU) && !defined(__EMSCRIPTEN__)
#include <webgpu/wgpu.h>        // WGPULogLevel
#endif

// Initialization data, for RblxUI_ImplWGPU_Init()
struct RblxUI_ImplWGPU_InitInfo
{
    WGPUDevice              Device = nullptr;
    int                     NumFramesInFlight = 3;
    WGPUTextureFormat       RenderTargetFormat = WGPUTextureFormat_Undefined;
    WGPUTextureFormat       DepthStencilFormat = WGPUTextureFormat_Undefined;
    WGPUMultisampleState    PipelineMultisampleState = {};

    RblxUI_ImplWGPU_InitInfo()
    {
        PipelineMultisampleState.count = 1;
        PipelineMultisampleState.mask = UINT32_MAX;
        PipelineMultisampleState.alphaToCoverageEnabled = false;
    }
};

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool RblxUI_ImplWGPU_Init(RblxUI_ImplWGPU_InitInfo* init_info);
RBLXUI_IMPL_API void RblxUI_ImplWGPU_Shutdown();
RBLXUI_IMPL_API void RblxUI_ImplWGPU_NewFrame();
RBLXUI_IMPL_API void RblxUI_ImplWGPU_RenderDrawData(RblxDrawData* draw_data, WGPURenderPassEncoder pass_encoder);

// Use if you want to reset your rendering device without losing Dear RblxUI state.
RBLXUI_IMPL_API bool RblxUI_ImplWGPU_CreateDeviceObjects();
RBLXUI_IMPL_API void RblxUI_ImplWGPU_InvalidateDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting RblxDrawData::Textures = nullptr to handle this manually.
RBLXUI_IMPL_API void RblxUI_ImplWGPU_UpdateTexture(RblxTextureData* tex);

// [BETA] Selected render state data shared with callbacks.
// This is temporarily stored in GetPlatformIO().Renderer_RenderState during the RblxUI_ImplWGPU_RenderDrawData() call.
// (Please open an issue if you feel you need access to more data)
struct RblxUI_ImplWGPU_RenderState
{
    WGPUDevice                  Device;
    WGPURenderPassEncoder       RenderPassEncoder;
};

//-------------------------------------------------------------------------
// Internal Helpers
// Those are currently used by our example applications.
//-------------------------------------------------------------------------

// (Optional) Helper to wrap some of the Dawn/WGPU/Emscripten quirks
bool        RblxUI_ImplWGPU_IsSurfaceStatusError(WGPUSurfaceGetCurrentTextureStatus status);
bool        RblxUI_ImplWGPU_IsSurfaceStatusSubOptimal(WGPUSurfaceGetCurrentTextureStatus status);    // Return whether the texture is suboptimal and may need to be recreated.

// (Optional) Helper for debugging/logging
void        RblxUI_ImplWGPU_DebugPrintAdapterInfo(const WGPUAdapter& adapter);
const char* RblxUI_ImplWGPU_GetBackendTypeName(WGPUBackendType type);
const char* RblxUI_ImplWGPU_GetAdapterTypeName(WGPUAdapterType type);
#if defined(RBLXUI_IMPL_WEBGPU_BACKEND_DAWN)
const char* RblxUI_ImplWGPU_GetDeviceLostReasonName(WGPUDeviceLostReason type);
const char* RblxUI_ImplWGPU_GetErrorTypeName(WGPUErrorType type);
#elif defined(RBLXUI_IMPL_WEBGPU_BACKEND_WGPU)
const char* RblxUI_ImplWGPU_GetLogLevelName(WGPULogLevel level);
#endif

// (Optional) Helper to create a surface on macOS/Wayland/X11/Window
#ifndef __EMSCRIPTEN__
struct RblxUI_ImplWGPU_CreateSurfaceInfo
{
    WGPUInstance    Instance;
    const char*     System;      // "cocoa"   | "wayland"   | "x11"     | "win32"
    void*           RawWindow;   // NSWindow* | 0           | Window    | HWND
    void*           RawDisplay;  // 0         | wl_display* | Display*  | 0
    void*           RawSurface;  //           | wl_surface* | 0         | 0
    void*           RawInstance; // 0         | 0           | 0         | HINSTANCE
};
WGPUSurface RblxUI_ImplWGPU_CreateWGPUSurfaceHelper(RblxUI_ImplWGPU_CreateSurfaceInfo* info);
#endif // #ifndef __EMSCRIPTEN__

#endif // #ifndef RBLXUI_DISABLE
