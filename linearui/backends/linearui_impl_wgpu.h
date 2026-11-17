// dear linearui: Renderer for WebGPU
// This needs to be used along with a Platform Binding (e.g. LINEARWIN, SDL2, SDL3)
// (Please note that WebGPU is a recent API, may not be supported by all browser, and its ecosystem is generally a mess)

// When targeting native platforms:
//  - One of LINEARUI_IMPL_WEBGPU_BACKEND_DAWN, LINEARUI_IMPL_WEBGPU_BACKEND_WGPU or LINEARUI_IMPL_WEBGPU_BACKEND_WGVK *must* be provided.
// When targeting Emscripten:
//  - We now defaults to LINEARUI_IMPL_WEBGPU_BACKEND_DAWN and requires Emscripten 4.0.10+, which correspond to using Emscripten '--use-port=emdawnwebgpu'.
//  - Emscripten < 4.0.10 is not supported anymore (old '-sUSE_WEBGPU=1' option).
//  - We can still define LINEARUI_IMPL_WEBGPU_BACKEND_WGPU to use Emscripten '-s USE_WEBGPU=1' which is marked as obsolete by Emscripten.
// Add #define to your linearuiconfig.h file, or as a compilation flag in your build system.
// This requirement may be removed once WebGPU stabilizes and backends converge on a unified interface.
//#define LINEARUI_IMPL_WEBGPU_BACKEND_DAWN
//#define LINEARUI_IMPL_WEBGPU_BACKEND_WGPU
//#define LINEARUI_IMPL_WEBGPU_BACKEND_WGVK

// Implemented features:
//  [X] Renderer: User texture binding. Use 'WGPUTextureView' as LinearTextureID. Read the FAQ about LinearTextureID/LinearTextureRef!
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (LinearUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Expose selected render state for draw callbacks to use. Access in '(LinearUI_ImplXXXX_RenderState*)GetPlatformIO().Renderer_RenderState'.
//  [X] Renderer: Texture updates support for dynamic font system (LinearUIBackendFlags_RendererHasTextures).
// Missing features or Issues:
//  [ ] Renderer: Multi-viewport support (multiple windows), useful for desktop.

// You can use unmodified linearui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire linearui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear LinearUI:
// - FAQ                  https://dearlinearui.com/faq
// - Getting Started      https://dearlinearui.com/getting-started
// - Documentation        https://dearlinearui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of linearui.cpp

#pragma once
#include "linearui.h"          // LINEARUI_IMPL_API
#ifndef LINEARUI_DISABLE

// Setup Emscripten default if not specified.
#if defined(__EMSCRIPTEN__) && !defined(LINEARUI_IMPL_WEBGPU_BACKEND_DAWN) && !defined(LINEARUI_IMPL_WEBGPU_BACKEND_WGPU)
#include <emscripten/version.h>
#define LINEARUI_IMPL_WEBGPU_BACKEND_DAWN
#endif

#include <webgpu/webgpu.h>
#if defined(LINEARUI_IMPL_WEBGPU_BACKEND_WGPU) && !defined(__EMSCRIPTEN__)
#include <webgpu/wgpu.h>        // WGPULogLevel
#endif

// Initialization data, for LinearUI_ImplWGPU_Init()
struct LinearUI_ImplWGPU_InitInfo
{
    WGPUDevice              Device = nullptr;
    int                     NumFramesInFlight = 3;
    WGPUTextureFormat       RenderTargetFormat = WGPUTextureFormat_Undefined;
    WGPUTextureFormat       DepthStencilFormat = WGPUTextureFormat_Undefined;
    WGPUMultisampleState    PipelineMultisampleState = {};

    LinearUI_ImplWGPU_InitInfo()
    {
        PipelineMultisampleState.count = 1;
        PipelineMultisampleState.mask = UINT32_MAX;
        PipelineMultisampleState.alphaToCoverageEnabled = false;
    }
};

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
LINEARUI_IMPL_API bool LinearUI_ImplWGPU_Init(LinearUI_ImplWGPU_InitInfo* init_info);
LINEARUI_IMPL_API void LinearUI_ImplWGPU_Shutdown();
LINEARUI_IMPL_API void LinearUI_ImplWGPU_NewFrame();
LINEARUI_IMPL_API void LinearUI_ImplWGPU_RenderDrawData(LinearDrawData* draw_data, WGPURenderPassEncoder pass_encoder);

// Use if you want to reset your rendering device without losing Dear LinearUI state.
LINEARUI_IMPL_API bool LinearUI_ImplWGPU_CreateDeviceObjects();
LINEARUI_IMPL_API void LinearUI_ImplWGPU_InvalidateDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting LinearDrawData::Textures = nullptr to handle this manually.
LINEARUI_IMPL_API void LinearUI_ImplWGPU_UpdateTexture(LinearTextureData* tex);

// [BETA] Selected render state data shared with callbacks.
// This is temporarily stored in GetPlatformIO().Renderer_RenderState during the LinearUI_ImplWGPU_RenderDrawData() call.
// (Please open an issue if you feel you need access to more data)
struct LinearUI_ImplWGPU_RenderState
{
    WGPUDevice                  Device;
    WGPURenderPassEncoder       RenderPassEncoder;
};

//-------------------------------------------------------------------------
// Internal Helpers
// Those are currently used by our example applications.
//-------------------------------------------------------------------------

// (Optional) Helper to wrap some of the Dawn/WGPU/Emscripten quirks
bool        LinearUI_ImplWGPU_IsSurfaceStatusError(WGPUSurfaceGetCurrentTextureStatus status);
bool        LinearUI_ImplWGPU_IsSurfaceStatusSubOptimal(WGPUSurfaceGetCurrentTextureStatus status);    // Return whether the texture is suboptimal and may need to be recreated.

// (Optional) Helper for debugging/logging
void        LinearUI_ImplWGPU_DebugPrintAdapterInfo(const WGPUAdapter& adapter);
const char* LinearUI_ImplWGPU_GetBackendTypeName(WGPUBackendType type);
const char* LinearUI_ImplWGPU_GetAdapterTypeName(WGPUAdapterType type);
#if defined(LINEARUI_IMPL_WEBGPU_BACKEND_DAWN)
const char* LinearUI_ImplWGPU_GetDeviceLostReasonName(WGPUDeviceLostReason type);
const char* LinearUI_ImplWGPU_GetErrorTypeName(WGPUErrorType type);
#elif defined(LINEARUI_IMPL_WEBGPU_BACKEND_WGPU)
const char* LinearUI_ImplWGPU_GetLogLevelName(WGPULogLevel level);
#endif

// (Optional) Helper to create a surface on macOS/Wayland/X11/Window
#ifndef __EMSCRIPTEN__
struct LinearUI_ImplWGPU_CreateSurfaceInfo
{
    WGPUInstance    Instance;
    const char*     System;      // "cocoa"   | "wayland"   | "x11"     | "win32"
    void*           RawWindow;   // NSWindow* | 0           | Window    | HWND
    void*           RawDisplay;  // 0         | wl_display* | Display*  | 0
    void*           RawSurface;  //           | wl_surface* | 0         | 0
    void*           RawInstance; // 0         | 0           | 0         | HINSTANCE
};
WGPUSurface LinearUI_ImplWGPU_CreateWGPUSurfaceHelper(LinearUI_ImplWGPU_CreateSurfaceInfo* info);
#endif // #ifndef __EMSCRIPTEN__

#endif // #ifndef LINEARUI_DISABLE
