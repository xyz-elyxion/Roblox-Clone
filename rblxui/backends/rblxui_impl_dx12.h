// dear rblxui: Renderer Backend for DirectX12
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'D3D12_GPU_DESCRIPTOR_HANDLE' as texture identifier. Read the FAQ about RblxTextureID/RblxTextureRef!
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (RblxUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Texture updates support for dynamic font atlas (RblxUIBackendFlags_RendererHasTextures).
//  [X] Renderer: Expose selected render state for draw callbacks to use. Access in '(RblxUI_ImplXXXX_RenderState*)GetPlatformIO().Renderer_RenderState'.
//  [X] Renderer: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= RblxUIConfigFlags_ViewportsEnable'.

// The aim of rblxui_impl_dx12.h/.cpp is to be usable in your engine without any modification.
// IF YOU FEEL YOU NEED TO MAKE ANY CHANGE TO THIS CODE, please share them and your feedback at https://github.com/ocornut/rblxui/

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
#include <dxgiformat.h> // DXGI_FORMAT
#include <d3d12.h>      // D3D12_CPU_DESCRIPTOR_HANDLE

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast" // warning: use of old-style cast
#endif

// Initialization data, for RblxUI_ImplDX12_Init()
struct RblxUI_ImplDX12_InitInfo
{
    ID3D12Device*               Device;
    ID3D12CommandQueue*         CommandQueue;       // Command queue used for queuing texture uploads.
    int                         NumFramesInFlight;
    DXGI_FORMAT                 RTVFormat;          // RenderTarget format.
    DXGI_FORMAT                 DSVFormat;          // DepthStencilView format.
    void*                       UserData;

    // Allocating SRV descriptors for textures is up to the application, so we provide callbacks.
    // (current version of the backend will only allocate one descriptor, from 1.92 the backend will need to allocate more)
    ID3D12DescriptorHeap*       SrvDescriptorHeap;
    void                        (*SrvDescriptorAllocFn)(RblxUI_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle);
    void                        (*SrvDescriptorFreeFn)(RblxUI_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_handle);
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    D3D12_CPU_DESCRIPTOR_HANDLE LegacySingleSrvCpuDescriptor; // To facilitate transition from single descriptor to allocator callback, you may use those.
    D3D12_GPU_DESCRIPTOR_HANDLE LegacySingleSrvGpuDescriptor;
#endif

    RblxUI_ImplDX12_InitInfo()   { memset((void*)this, 0, sizeof(*this)); }
};

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool     RblxUI_ImplDX12_Init(RblxUI_ImplDX12_InitInfo* info);
RBLXUI_IMPL_API void     RblxUI_ImplDX12_Shutdown();
RBLXUI_IMPL_API void     RblxUI_ImplDX12_NewFrame();
RBLXUI_IMPL_API void     RblxUI_ImplDX12_RenderDrawData(RblxDrawData* draw_data, ID3D12GraphicsCommandList* graphics_command_list);

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
// Legacy initialization API Obsoleted in 1.91.5
// - font_srv_cpu_desc_handle and font_srv_gpu_desc_handle are handles to a single SRV descriptor to use for the internal font texture, they must be in 'srv_descriptor_heap'
// - When we introduced the RblxUI_ImplDX12_InitInfo struct we also added a 'ID3D12CommandQueue* CommandQueue' field.
RBLXUI_IMPL_API bool     RblxUI_ImplDX12_Init(ID3D12Device* device, int num_frames_in_flight, DXGI_FORMAT rtv_format, ID3D12DescriptorHeap* srv_descriptor_heap, D3D12_CPU_DESCRIPTOR_HANDLE font_srv_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE font_srv_gpu_desc_handle);
#endif

// Use if you want to reset your rendering device without losing Dear RblxUI state.
RBLXUI_IMPL_API bool     RblxUI_ImplDX12_CreateDeviceObjects();
RBLXUI_IMPL_API void     RblxUI_ImplDX12_InvalidateDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting RblxDrawData::Textures = nullptr to handle this manually.
RBLXUI_IMPL_API void     RblxUI_ImplDX12_UpdateTexture(RblxTextureData* tex);

// [BETA] Selected render state data shared with callbacks.
// This is temporarily stored in GetPlatformIO().Renderer_RenderState during the RblxUI_ImplDX12_RenderDrawData() call.
// (Please open an issue if you feel you need access to more data)
struct RblxUI_ImplDX12_RenderState
{
    ID3D12Device*               Device;
    ID3D12GraphicsCommandList*  CommandList;
};

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef RBLXUI_DISABLE
