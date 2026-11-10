// dear rblxui: Renderer for WebGPU
// This needs to be used along with a Platform Binding (e.g. RBLXWIN, SDL2, SDL3)
// (Please note that WebGPU is a recent API, may not be supported by all browser, and its ecosystem is generally a mess)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'WGPUTextureView' as RblxTextureID. Read the FAQ about RblxTextureID/RblxTextureRef!
//  [X] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (RblxUIBackendFlags_RendererHasVtxOffset).
//  [X] Renderer: Expose selected render state for draw callbacks to use. Access in '(RblxUI_ImplXXXX_RenderState*)GetPlatformIO().Renderer_RenderState'.
//  [X] Renderer: Texture updates support for dynamic font system (RblxUIBackendFlags_RendererHasTextures).
// Missing features or Issues:
//  [ ] Renderer: Multi-viewport support (multiple windows), useful for desktop.

// Read rblxui_impl_wgpu.h about how to use the RBLXUI_IMPL_WEBGPU_BACKEND_WGPU or RBLXUI_IMPL_WEBGPU_BACKEND_DAWN flags.

// You can use unmodified rblxui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire rblxui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear RblxUI:
// - FAQ                  https://dearrblxui.com/faq
// - Getting Started      https://dearrblxui.com/getting-started
// - Documentation        https://dearrblxui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of rblxui.cpp

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2026-04-23: Added support for standard draw callbacks (in platform_io): DrawCallback_ResetRenderState, DrawCallback_SetSamplerLinear, DrawCallback_SetSamplerNearest. (#9378)
//  2026-03-25: Added support for WGVK native backend via RBLXUI_IMPL_WEBGPU_BACKEND_WGVK define, with SPIRV shaders if WGSL is not available. (#9316, #9246, #9257)
//  2026-03-09: Removed support for Emscripten < 4.0.10. (#9281)
//  2025-10-16: Update to compile with Dawn and Emscripten's 4.0.10+ '--use-port=emdawnwebgpu' ports. (#8381, #8898)
//  2025-09-18: Call platform_io.ClearRendererHandlers() on shutdown.
//  2025-06-12: Added support for RblxUIBackendFlags_RendererHasTextures, for dynamic font atlas. (#8465)
//  2025-02-26: Recreate image bind groups during render. (#8426, #8046, #7765, #8027) + Update for latest webgpu-native changes.
//  2024-10-14: Update Dawn support for change of string usages. (#8082, #8083)
//  2024-10-07: Expose selected render state in RblxUI_ImplWGPU_RenderState, which you can access in 'void* platform_io.Renderer_RenderState' during draw callbacks.
//  2024-10-07: Changed default texture sampler to Clamp instead of Repeat/Wrap.
//  2024-09-16: Added support for optional RBLXUI_IMPL_WEBGPU_BACKEND_DAWN / RBLXUI_IMPL_WEBGPU_BACKEND_WGPU define to handle ever-changing native implementations. (#7977)
//  2024-01-22: Added configurable PipelineMultisampleState struct. (#7240)
//  2024-01-22: (Breaking) RblxUI_ImplWGPU_Init() now takes a RblxUI_ImplWGPU_InitInfo structure instead of variety of parameters, allowing for easier further changes.
//  2024-01-22: Fixed pipeline layout leak. (#7245)
//  2024-01-17: Explicitly fill all of WGPUDepthStencilState since standard removed defaults.
//  2023-07-13: Use WGPUShaderModuleWGSLDescriptor's code instead of source. use WGPUMipmapFilterMode_Linear instead of WGPUFilterMode_Linear. (#6602)
//  2023-04-11: Align buffer sizes. Use WGSL shaders instead of precompiled SPIR-V.
//  2023-04-11: Reorganized backend to pull data from a single structure to facilitate usage with multiple-contexts (all g_XXXX access changed to bd->XXXX).
//  2023-01-25: Revert automatic pipeline layout generation (see https://github.com/gpuweb/gpuweb/issues/2470)
//  2022-11-24: Fixed validation error with default depth buffer settings.
//  2022-11-10: Fixed rendering when a depth buffer is enabled. Added 'WGPUTextureFormat depth_format' parameter to RblxUI_ImplWGPU_Init().
//  2022-10-11: Using 'nullptr' instead of 'NULL' as per our switch to C++11.
//  2021-11-29: Passing explicit buffer sizes to wgpuRenderPassEncoderSetVertexBuffer()/wgpuRenderPassEncoderSetIndexBuffer().
//  2021-08-24: Fixed for latest specs.
//  2021-05-24: Add support for draw_data->FramebufferScale.
//  2021-05-19: Replaced direct access to RblxDrawCmd::TextureId with a call to RblxDrawCmd::GetTexID(). (will become a requirement)
//  2021-05-16: Update to latest WebGPU specs (compatible with Emscripten 2.0.20 and Chrome Canary 92).
//  2021-02-18: Change blending equation to preserve alpha in output buffer.
//  2021-01-28: Initial version.

#include "rblxui.h"

#ifndef RBLXUI_DISABLE
#include "rblxui_impl_wgpu.h"
#include <limits.h>
#include <stdio.h>

// One of RBLXUI_IMPL_WEBGPU_BACKEND_DAWN or RBLXUI_IMPL_WEBGPU_BACKEND_WGPU must be provided. See rblxui_impl_wgpu.h for more details.
#if !defined(RBLXUI_IMPL_WEBGPU_BACKEND_DAWN) && !defined(RBLXUI_IMPL_WEBGPU_BACKEND_WGPU) && !defined(RBLXUI_IMPL_WEBGPU_BACKEND_WGVK)
#error Exactly one of RBLXUI_IMPL_WEBGPU_BACKEND_DAWN, RBLXUI_IMPL_WEBGPU_BACKEND_WGPU or RBLXUI_IMPL_WEBGPU_BACKEND_WGVK must be defined!
#endif
#if defined(__EMSCRIPTEN__) && defined(RBLXUI_IMPL_WEBGPU_BACKEND_WGPU)
#error Emscripten <4.0.10 with '-sUSE_WEBGPU=1' is not supported anymore.
#endif

#if defined RBLXUI_IMPL_WEBGPU_BACKEND_DAWN || defined RBLXUI_IMPL_WEBGPU_BACKEND_WGVK
// Dawn renamed WGPUProgrammableStageDescriptor to WGPUComputeState (see: https://github.com/webgpu-native/webgpu-headers/pull/413)
// Using type alias until WGPU adopts the same naming convention (#8369)
using WGPUProgrammableStageDescriptor = WGPUComputeState;
#endif

// Dear RblxUI prototypes from rblxui_internal.h
extern RblxUIID RblxHashData(const void* data_p, size_t data_size, RblxU32 seed);
#define MEMALIGN(_SIZE,_ALIGN)        (((_SIZE) + ((_ALIGN) - 1)) & ~((_ALIGN) - 1))    // Memory align (copied from RBLXUI_ALIGN() macro).

// WebGPU data
struct RblxUI_ImplWGPU_Texture
{
    WGPUTexture         Texture = nullptr;
    WGPUTextureView     TextureView = nullptr;
};

struct RenderResources
{
    WGPUSampler         SamplerLinear = nullptr;            // Bilinear sampler
    WGPUSampler         SamplerNearest = nullptr;           // Nearest/point sampler
    WGPUBuffer          Uniforms = nullptr;                 // Shader uniforms
    WGPUBindGroup       CommonBindGroupLinear = nullptr;    // Common bind-group bound to group 0 (uniforms + SamplerLinear)
    WGPUBindGroup       CommonBindGroupNearest = nullptr;   // Common bind-group bound to group 0 (uniforms + SamplerNearest)
    RblxUIStorage        ImageBindGroups;                    // Resources bind-group to bind the font/image resources to pipeline (this is a key->value map)
    WGPUBindGroupLayout ImageBindGroupLayout = nullptr;     // Cache layout used for the image bind group. Avoids allocating unnecessary JS objects when working with WebASM
};

struct FrameResources
{
    WGPUBuffer  IndexBuffer;
    WGPUBuffer  VertexBuffer;
    RblxDrawIdx*  IndexBufferHost;
    RblxDrawVert* VertexBufferHost;
    int         IndexBufferSize;
    int         VertexBufferSize;
};

struct Uniforms
{
    float MVP[4][4];
    float Gamma;
};

struct RblxUI_ImplWGPU_Data
{
    RblxUI_ImplWGPU_InitInfo initInfo;
    WGPUDevice              wgpuDevice = nullptr;
    WGPUQueue               defaultQueue = nullptr;
    WGPUTextureFormat       renderTargetFormat = WGPUTextureFormat_Undefined;
    WGPUTextureFormat       depthStencilFormat = WGPUTextureFormat_Undefined;
    WGPURenderPipeline      pipelineState = nullptr;
    RblxUI_ImplWGPU_RenderState* RenderState = nullptr;  // == RblxUI::GetPlatformIO().Renderer_RenderState during rendering.

    RenderResources         renderResources;
    FrameResources*         pFrameResources = nullptr;
    unsigned int            numFramesInFlight = 0;
    unsigned int            frameIndex = UINT_MAX;
};

// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear RblxUI contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear RblxUI context + multiple windows) instead of multiple Dear RblxUI contexts.
static RblxUI_ImplWGPU_Data* RblxUI_ImplWGPU_GetBackendData()
{
    return RblxUI::GetCurrentContext() ? (RblxUI_ImplWGPU_Data*)RblxUI::GetIO().BackendRendererUserData : nullptr;
}

//-----------------------------------------------------------------------------
// SHADERS
//-----------------------------------------------------------------------------

static const char __shader_vert_wgsl[] = R"(
struct VertexInput {
    @location(0) position: vec2<f32>,
    @location(1) uv: vec2<f32>,
    @location(2) color: vec4<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) uv: vec2<f32>,
};

struct Uniforms {
    mvp: mat4x4<f32>,
    gamma: f32,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = uniforms.mvp * vec4<f32>(in.position, 0.0, 1.0);
    out.color = in.color;
    out.uv = in.uv;
    return out;
}
)";

static const char __shader_frag_wgsl[] = R"(
struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) uv: vec2<f32>,
};

struct Uniforms {
    mvp: mat4x4<f32>,
    gamma: f32,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@group(0) @binding(1) var s: sampler;
@group(1) @binding(0) var t: texture_2d<f32>;

@fragment
fn main(in: VertexOutput) -> @location(0) vec4<f32> {
    let color = in.color * textureSample(t, s, in.uv);
    let corrected_color = pow(color.rgb, vec3<f32>(uniforms.gamma));
    return vec4<f32>(corrected_color, color.a);
}
)";

// Same shader as __shader_vert_wgsl[] but compiled as SPIRV.
// 'wgslc -o vert.spv vert.wgsl' + 'binary_to_compressed_c -u8 -nocompress vert.spv'
static const unsigned char __shader_vert_spirv[1996] =
{
    3,2,35,7,0,3,1,0,1,0,23,0,90,0,0,0,0,0,0,0,17,0,2,0,1,0,0,0,14,0,3,0,0,0,0,0,1,0,0,0,15,0,12,0,0,0,0,0,78,0,0,0,109,97,105,110,0,0,0,0,8,0,0,0,12,0,0,0,13,0,0,0,16,0,0,0,18,0,0,0,19,
    0,0,0,21,0,0,0,71,0,4,0,4,0,0,0,6,0,0,0,16,0,0,0,72,0,5,0,3,0,0,0,0,0,0,0,35,0,0,0,0,0,0,0,71,0,3,0,3,0,0,0,2,0,0,0,71,0,4,0,1,0,0,0,34,0,0,0,0,0,0,0,71,0,4,0,1,0,0,0,33,0,0,0,0,0,
    0,0,71,0,3,0,1,0,0,0,24,0,0,0,71,0,4,0,8,0,0,0,30,0,0,0,0,0,0,0,71,0,4,0,12,0,0,0,30,0,0,0,1,0,0,0,71,0,4,0,13,0,0,0,30,0,0,0,2,0,0,0,71,0,4,0,16,0,0,0,11,0,0,0,0,0,0,0,71,0,4,0,18,
    0,0,0,30,0,0,0,0,0,0,0,71,0,4,0,19,0,0,0,30,0,0,0,1,0,0,0,71,0,4,0,21,0,0,0,11,0,0,0,1,0,0,0,21,0,4,0,6,0,0,0,32,0,0,0,0,0,0,0,23,0,4,0,5,0,0,0,6,0,0,0,4,0,0,0,43,0,4,0,6,0,0,0,7,0,
    0,0,5,0,0,0,28,0,4,0,4,0,0,0,5,0,0,0,7,0,0,0,30,0,3,0,3,0,0,0,4,0,0,0,32,0,4,0,2,0,0,0,2,0,0,0,3,0,0,0,59,0,4,0,2,0,0,0,1,0,0,0,2,0,0,0,22,0,3,0,11,0,0,0,32,0,0,0,23,0,4,0,10,0,0,0,
    11,0,0,0,2,0,0,0,32,0,4,0,9,0,0,0,1,0,0,0,10,0,0,0,59,0,4,0,9,0,0,0,8,0,0,0,1,0,0,0,59,0,4,0,9,0,0,0,12,0,0,0,1,0,0,0,23,0,4,0,15,0,0,0,11,0,0,0,4,0,0,0,32,0,4,0,14,0,0,0,1,0,0,0,15,
    0,0,0,59,0,4,0,14,0,0,0,13,0,0,0,1,0,0,0,32,0,4,0,17,0,0,0,3,0,0,0,15,0,0,0,59,0,4,0,17,0,0,0,16,0,0,0,3,0,0,0,59,0,4,0,17,0,0,0,18,0,0,0,3,0,0,0,32,0,4,0,20,0,0,0,3,0,0,0,10,0,0,0,
    59,0,4,0,20,0,0,0,19,0,0,0,3,0,0,0,32,0,4,0,22,0,0,0,3,0,0,0,11,0,0,0,59,0,4,0,22,0,0,0,21,0,0,0,3,0,0,0,30,0,5,0,24,0,0,0,15,0,0,0,15,0,0,0,10,0,0,0,30,0,5,0,25,0,0,0,10,0,0,0,10,
    0,0,0,15,0,0,0,33,0,4,0,27,0,0,0,24,0,0,0,25,0,0,0,32,0,4,0,30,0,0,0,7,0,0,0,24,0,0,0,46,0,3,0,24,0,0,0,31,0,0,0,32,0,4,0,33,0,0,0,7,0,0,0,15,0,0,0,43,0,4,0,6,0,0,0,34,0,0,0,0,0,0,
    0,24,0,4,0,36,0,0,0,15,0,0,0,4,0,0,0,43,0,4,0,11,0,0,0,40,0,0,0,0,0,0,0,43,0,4,0,11,0,0,0,41,0,0,0,0,0,128,63,43,0,4,0,6,0,0,0,44,0,0,0,1,0,0,0,32,0,4,0,47,0,0,0,7,0,0,0,10,0,0,0,43,
    0,4,0,6,0,0,0,48,0,0,0,2,0,0,0,33,0,4,0,52,0,0,0,36,0,0,0,6,0,0,0,43,0,4,0,6,0,0,0,55,0,0,0,16,0,0,0,32,0,4,0,57,0,0,0,2,0,0,0,5,0,0,0,43,0,4,0,6,0,0,0,66,0,0,0,32,0,0,0,43,0,4,0,6,
    0,0,0,72,0,0,0,48,0,0,0,19,0,2,0,79,0,0,0,33,0,3,0,80,0,0,0,79,0,0,0,54,0,5,0,24,0,0,0,23,0,0,0,0,0,0,0,27,0,0,0,55,0,3,0,25,0,0,0,26,0,0,0,248,0,2,0,28,0,0,0,59,0,5,0,30,0,0,0,29,
    0,0,0,7,0,0,0,31,0,0,0,65,0,5,0,33,0,0,0,32,0,0,0,29,0,0,0,34,0,0,0,57,0,5,0,36,0,0,0,35,0,0,0,37,0,0,0,34,0,0,0,81,0,5,0,10,0,0,0,38,0,0,0,26,0,0,0,0,0,0,0,80,0,6,0,15,0,0,0,39,0,
    0,0,38,0,0,0,40,0,0,0,41,0,0,0,145,0,5,0,15,0,0,0,42,0,0,0,35,0,0,0,39,0,0,0,62,0,4,0,32,0,0,0,42,0,0,0,0,0,0,0,65,0,5,0,33,0,0,0,43,0,0,0,29,0,0,0,44,0,0,0,81,0,5,0,15,0,0,0,45,0,
    0,0,26,0,0,0,2,0,0,0,62,0,4,0,43,0,0,0,45,0,0,0,0,0,0,0,65,0,5,0,47,0,0,0,46,0,0,0,29,0,0,0,48,0,0,0,81,0,5,0,10,0,0,0,49,0,0,0,26,0,0,0,1,0,0,0,62,0,4,0,46,0,0,0,49,0,0,0,0,0,0,0,
    61,0,5,0,24,0,0,0,50,0,0,0,29,0,0,0,0,0,0,0,254,0,2,0,50,0,0,0,56,0,1,0,54,0,5,0,36,0,0,0,37,0,0,0,0,0,0,0,52,0,0,0,55,0,3,0,6,0,0,0,51,0,0,0,248,0,2,0,53,0,0,0,134,0,5,0,6,0,0,0,54,
    0,0,0,51,0,0,0,55,0,0,0,65,0,6,0,57,0,0,0,56,0,0,0,1,0,0,0,34,0,0,0,54,0,0,0,61,0,5,0,5,0,0,0,58,0,0,0,56,0,0,0,0,0,0,0,124,0,4,0,15,0,0,0,59,0,0,0,58,0,0,0,128,0,5,0,6,0,0,0,60,0,
    0,0,55,0,0,0,51,0,0,0,134,0,5,0,6,0,0,0,61,0,0,0,60,0,0,0,55,0,0,0,65,0,6,0,57,0,0,0,62,0,0,0,1,0,0,0,34,0,0,0,61,0,0,0,61,0,5,0,5,0,0,0,63,0,0,0,62,0,0,0,0,0,0,0,124,0,4,0,15,0,0,
    0,64,0,0,0,63,0,0,0,128,0,5,0,6,0,0,0,65,0,0,0,66,0,0,0,51,0,0,0,134,0,5,0,6,0,0,0,67,0,0,0,65,0,0,0,55,0,0,0,65,0,6,0,57,0,0,0,68,0,0,0,1,0,0,0,34,0,0,0,67,0,0,0,61,0,5,0,5,0,0,0,
    69,0,0,0,68,0,0,0,0,0,0,0,124,0,4,0,15,0,0,0,70,0,0,0,69,0,0,0,128,0,5,0,6,0,0,0,71,0,0,0,72,0,0,0,51,0,0,0,134,0,5,0,6,0,0,0,73,0,0,0,71,0,0,0,55,0,0,0,65,0,6,0,57,0,0,0,74,0,0,0,
    1,0,0,0,34,0,0,0,73,0,0,0,61,0,5,0,5,0,0,0,75,0,0,0,74,0,0,0,0,0,0,0,124,0,4,0,15,0,0,0,76,0,0,0,75,0,0,0,80,0,7,0,36,0,0,0,77,0,0,0,59,0,0,0,64,0,0,0,70,0,0,0,76,0,0,0,254,0,2,0,77,
    0,0,0,56,0,1,0,54,0,5,0,79,0,0,0,78,0,0,0,0,0,0,0,80,0,0,0,248,0,2,0,81,0,0,0,61,0,5,0,10,0,0,0,82,0,0,0,8,0,0,0,0,0,0,0,61,0,5,0,10,0,0,0,83,0,0,0,12,0,0,0,0,0,0,0,61,0,5,0,15,0,0,
    0,84,0,0,0,13,0,0,0,0,0,0,0,80,0,6,0,25,0,0,0,85,0,0,0,82,0,0,0,83,0,0,0,84,0,0,0,57,0,5,0,24,0,0,0,86,0,0,0,23,0,0,0,85,0,0,0,81,0,5,0,15,0,0,0,87,0,0,0,86,0,0,0,0,0,0,0,62,0,4,0,
    16,0,0,0,87,0,0,0,0,0,0,0,81,0,5,0,15,0,0,0,88,0,0,0,86,0,0,0,1,0,0,0,62,0,4,0,18,0,0,0,88,0,0,0,0,0,0,0,81,0,5,0,10,0,0,0,89,0,0,0,86,0,0,0,2,0,0,0,62,0,4,0,19,0,0,0,89,0,0,0,0,0,
    0,0,62,0,4,0,21,0,0,0,41,0,0,0,0,0,0,0,253,0,1,0,56,0,1,0,
};

// Same shader as __shader_frag_wgsl[] but compiled as SPIRV.
// 'wgslc -o frag.spv frag.wgsl' + 'binary_to_compressed_c -u8 -nocompress frag.spv'
static const unsigned char __shader_frag_spirv[1392] =
{
    3,2,35,7,0,3,1,0,1,0,23,0,60,0,0,0,0,0,0,0,17,0,2,0,1,0,0,0,11,0,6,0,48,0,0,0,71,76,83,76,46,115,116,100,46,52,53,48,0,0,0,0,14,0,3,0,0,0,0,0,1,0,0,0,15,0,9,0,4,0,0,0,51,0,0,0,109,
    97,105,110,0,0,0,0,15,0,0,0,18,0,0,0,19,0,0,0,22,0,0,0,16,0,3,0,51,0,0,0,7,0,0,0,71,0,4,0,4,0,0,0,6,0,0,0,16,0,0,0,72,0,5,0,3,0,0,0,0,0,0,0,35,0,0,0,0,0,0,0,71,0,3,0,3,0,0,0,2,0,0,
    0,71,0,4,0,1,0,0,0,34,0,0,0,0,0,0,0,71,0,4,0,1,0,0,0,33,0,0,0,0,0,0,0,71,0,3,0,1,0,0,0,24,0,0,0,71,0,4,0,8,0,0,0,34,0,0,0,0,0,0,0,71,0,4,0,8,0,0,0,33,0,0,0,1,0,0,0,71,0,4,0,11,0,0,
    0,34,0,0,0,1,0,0,0,71,0,4,0,11,0,0,0,33,0,0,0,0,0,0,0,71,0,4,0,15,0,0,0,11,0,0,0,15,0,0,0,71,0,4,0,18,0,0,0,30,0,0,0,0,0,0,0,71,0,4,0,19,0,0,0,30,0,0,0,1,0,0,0,71,0,4,0,22,0,0,0,30,
    0,0,0,0,0,0,0,21,0,4,0,6,0,0,0,32,0,0,0,0,0,0,0,23,0,4,0,5,0,0,0,6,0,0,0,4,0,0,0,43,0,4,0,6,0,0,0,7,0,0,0,5,0,0,0,28,0,4,0,4,0,0,0,5,0,0,0,7,0,0,0,30,0,3,0,3,0,0,0,4,0,0,0,32,0,4,0,
    2,0,0,0,2,0,0,0,3,0,0,0,59,0,4,0,2,0,0,0,1,0,0,0,2,0,0,0,26,0,2,0,10,0,0,0,32,0,4,0,9,0,0,0,0,0,0,0,10,0,0,0,59,0,4,0,9,0,0,0,8,0,0,0,0,0,0,0,22,0,3,0,14,0,0,0,32,0,0,0,25,0,9,0,13,
    0,0,0,14,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,32,0,4,0,12,0,0,0,0,0,0,0,13,0,0,0,59,0,4,0,12,0,0,0,11,0,0,0,0,0,0,0,23,0,4,0,17,0,0,0,14,0,0,0,4,0,0,0,32,0,4,0,16,
    0,0,0,1,0,0,0,17,0,0,0,59,0,4,0,16,0,0,0,15,0,0,0,1,0,0,0,59,0,4,0,16,0,0,0,18,0,0,0,1,0,0,0,23,0,4,0,21,0,0,0,14,0,0,0,2,0,0,0,32,0,4,0,20,0,0,0,1,0,0,0,21,0,0,0,59,0,4,0,20,0,0,0,
    19,0,0,0,1,0,0,0,32,0,4,0,23,0,0,0,3,0,0,0,17,0,0,0,59,0,4,0,23,0,0,0,22,0,0,0,3,0,0,0,30,0,5,0,25,0,0,0,17,0,0,0,17,0,0,0,21,0,0,0,33,0,4,0,27,0,0,0,17,0,0,0,25,0,0,0,27,0,3,0,34,
    0,0,0,13,0,0,0,23,0,4,0,38,0,0,0,14,0,0,0,3,0,0,0,32,0,4,0,40,0,0,0,2,0,0,0,5,0,0,0,43,0,4,0,6,0,0,0,41,0,0,0,0,0,0,0,43,0,4,0,6,0,0,0,42,0,0,0,4,0,0,0,19,0,2,0,52,0,0,0,33,0,3,0,53,
    0,0,0,52,0,0,0,54,0,5,0,17,0,0,0,24,0,0,0,0,0,0,0,27,0,0,0,55,0,3,0,25,0,0,0,26,0,0,0,248,0,2,0,28,0,0,0,81,0,5,0,17,0,0,0,29,0,0,0,26,0,0,0,1,0,0,0,61,0,5,0,13,0,0,0,30,0,0,0,11,0,
    0,0,0,0,0,0,61,0,5,0,10,0,0,0,31,0,0,0,8,0,0,0,0,0,0,0,81,0,5,0,21,0,0,0,32,0,0,0,26,0,0,0,2,0,0,0,86,0,5,0,34,0,0,0,33,0,0,0,30,0,0,0,31,0,0,0,87,0,6,0,17,0,0,0,35,0,0,0,33,0,0,0,
    32,0,0,0,0,0,0,0,133,0,5,0,17,0,0,0,36,0,0,0,29,0,0,0,35,0,0,0,79,0,8,0,38,0,0,0,37,0,0,0,36,0,0,0,36,0,0,0,0,0,0,0,1,0,0,0,2,0,0,0,65,0,6,0,40,0,0,0,39,0,0,0,1,0,0,0,41,0,0,0,42,0,
    0,0,61,0,5,0,5,0,0,0,43,0,0,0,39,0,0,0,0,0,0,0,81,0,5,0,6,0,0,0,44,0,0,0,43,0,0,0,0,0,0,0,124,0,4,0,14,0,0,0,45,0,0,0,44,0,0,0,80,0,6,0,38,0,0,0,46,0,0,0,45,0,0,0,45,0,0,0,45,0,0,0,
    12,0,7,0,38,0,0,0,47,0,0,0,48,0,0,0,26,0,0,0,37,0,0,0,46,0,0,0,81,0,5,0,14,0,0,0,49,0,0,0,36,0,0,0,3,0,0,0,80,0,5,0,17,0,0,0,50,0,0,0,47,0,0,0,49,0,0,0,254,0,2,0,50,0,0,0,56,0,1,0,
    54,0,5,0,52,0,0,0,51,0,0,0,0,0,0,0,53,0,0,0,248,0,2,0,54,0,0,0,61,0,5,0,17,0,0,0,55,0,0,0,15,0,0,0,0,0,0,0,61,0,5,0,17,0,0,0,56,0,0,0,18,0,0,0,0,0,0,0,61,0,5,0,21,0,0,0,57,0,0,0,19,
    0,0,0,0,0,0,0,80,0,6,0,25,0,0,0,58,0,0,0,55,0,0,0,56,0,0,0,57,0,0,0,57,0,5,0,17,0,0,0,59,0,0,0,24,0,0,0,58,0,0,0,62,0,4,0,22,0,0,0,59,0,0,0,0,0,0,0,253,0,1,0,56,0,1,0,
};

static void SafeRelease(RblxDrawIdx*& res)
{
    if (res)
        delete[] res;
    res = nullptr;
}
static void SafeRelease(RblxDrawVert*& res)
{
    if (res)
        delete[] res;
    res = nullptr;
}
static void SafeRelease(WGPUBindGroupLayout& res)
{
    if (res)
        wgpuBindGroupLayoutRelease(res);
    res = nullptr;
}
static void SafeRelease(WGPUBindGroup& res)
{
    if (res)
        wgpuBindGroupRelease(res);
    res = nullptr;
}
static void SafeRelease(WGPUBuffer& res)
{
    if (res)
        wgpuBufferRelease(res);
    res = nullptr;
}
static void SafeRelease(WGPUPipelineLayout& res)
{
    if (res)
        wgpuPipelineLayoutRelease(res);
    res = nullptr;
}
static void SafeRelease(WGPURenderPipeline& res)
{
    if (res)
        wgpuRenderPipelineRelease(res);
    res = nullptr;
}
static void SafeRelease(WGPUSampler& res)
{
    if (res)
        wgpuSamplerRelease(res);
    res = nullptr;
}
static void SafeRelease(WGPUShaderModule& res)
{
    if (res)
        wgpuShaderModuleRelease(res);
    res = nullptr;
}
static void SafeRelease(RenderResources& res)
{
    SafeRelease(res.SamplerLinear);
    SafeRelease(res.SamplerNearest);
    SafeRelease(res.Uniforms);
    SafeRelease(res.CommonBindGroupLinear);
    SafeRelease(res.CommonBindGroupNearest);
    SafeRelease(res.ImageBindGroupLayout);
};

static void SafeRelease(FrameResources& res)
{
    SafeRelease(res.IndexBuffer);
    SafeRelease(res.VertexBuffer);
    SafeRelease(res.IndexBufferHost);
    SafeRelease(res.VertexBufferHost);
}

static WGPUProgrammableStageDescriptor RblxUI_ImplWGPU_CreateShaderModuleWGSL(const char* wgsl_source)
{
    RblxUI_ImplWGPU_Data* bd = RblxUI_ImplWGPU_GetBackendData();
    RBLXUI_UNUSED(bd);

    WGPUShaderSourceWGSL wgsl_desc = {};
    wgsl_desc.chain.sType = WGPUSType_ShaderSourceWGSL;
    wgsl_desc.code = { wgsl_source, WGPU_STRLEN };

    WGPUShaderModuleDescriptor desc = {};
    desc.nextInChain = (WGPUChainedStruct*)&wgsl_desc;

    // Detect shader compilation errors by using an error scope.
    // Flag to be passed into the validation callback `userdata1` pointer.
    int validation_error = 0;
    wgpuDevicePushErrorScope(bd->wgpuDevice, WGPUErrorFilter_Validation);
    WGPUShaderModule module = wgpuDeviceCreateShaderModule(bd->wgpuDevice, &desc);
    WGPUPopErrorScopeCallbackInfo pop_cb = {};
    pop_cb.mode = WGPUCallbackMode_AllowSpontaneous;
    pop_cb.callback = [](WGPUPopErrorScopeStatus, WGPUErrorType type, WGPUStringView, void* userdata1, void*)
    {
        if (type == WGPUErrorType_Validation)
            *static_cast<int*>(userdata1) = 1;
    };
    pop_cb.userdata1 = &validation_error;
    wgpuDevicePopErrorScope(bd->wgpuDevice, pop_cb);

    WGPUProgrammableStageDescriptor stage_desc = {};
    if (module && !validation_error)
    {
        stage_desc.module = module;
        stage_desc.entryPoint = { "main", WGPU_STRLEN };
    }
    else if (module)
    {
        wgpuShaderModuleRelease(module);
    }
    return stage_desc;
}

static WGPUProgrammableStageDescriptor RblxUI_ImplWGPU_CreateShaderModuleSPIRV(const void* spirv_binary, size_t spirv_length)
{
    RblxUI_ImplWGPU_Data* bd = RblxUI_ImplWGPU_GetBackendData();

    WGPUShaderSourceSPIRV spirv_desc = {};
    spirv_desc.chain.sType = WGPUSType_ShaderSourceSPIRV;
    spirv_desc.code = (const uint32_t *)spirv_binary;
    spirv_desc.codeSize = ((uint32_t)(spirv_length / 4));

    WGPUShaderModuleDescriptor desc = {};
    desc.nextInChain = (WGPUChainedStruct*)&spirv_desc;

    WGPUProgrammableStageDescriptor stage_desc = {};
    stage_desc.module = wgpuDeviceCreateShaderModule(bd->wgpuDevice, &desc);

    stage_desc.entryPoint = { "main", WGPU_STRLEN };
    return stage_desc;
}

static WGPUBindGroup RblxUI_ImplWGPU_CreateImageBindGroup(WGPUBindGroupLayout layout, WGPUTextureView texture)
{
    RblxUI_ImplWGPU_Data* bd = RblxUI_ImplWGPU_GetBackendData();
    WGPUBindGroupEntry image_bg_entries[] = { { nullptr, 0, 0, 0, 0, 0, texture } };

    WGPUBindGroupDescriptor image_bg_descriptor = {};
    image_bg_descriptor.layout = layout;
    image_bg_descriptor.entryCount = sizeof(image_bg_entries) / sizeof(WGPUBindGroupEntry);
    image_bg_descriptor.entries = image_bg_entries;
    return wgpuDeviceCreateBindGroup(bd->wgpuDevice, &image_bg_descriptor);
}

static void RblxUI_ImplWGPU_SetupRenderState(RblxDrawData* draw_data, WGPURenderPassEncoder ctx, FrameResources* fr)
{
    RblxUI_ImplWGPU_Data* bd = RblxUI_ImplWGPU_GetBackendData();

    // Setup orthographic projection matrix into our constant buffer
    // Our visible rblxui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right).
    {
        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        float mvp[4][4] =
        {
            { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
            { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
        };
        wgpuQueueWriteBuffer(bd->defaultQueue, bd->renderResources.Uniforms, offsetof(Uniforms, MVP), mvp, sizeof(Uniforms::MVP));
        float gamma;
        switch (bd->renderTargetFormat)
        {
        case WGPUTextureFormat_ASTC10x10UnormSrgb:
        case WGPUTextureFormat_ASTC10x5UnormSrgb:
        case WGPUTextureFormat_ASTC10x6UnormSrgb:
        case WGPUTextureFormat_ASTC10x8UnormSrgb:
        case WGPUTextureFormat_ASTC12x10UnormSrgb:
        case WGPUTextureFormat_ASTC12x12UnormSrgb:
        case WGPUTextureFormat_ASTC4x4UnormSrgb:
        case WGPUTextureFormat_ASTC5x5UnormSrgb:
        case WGPUTextureFormat_ASTC6x5UnormSrgb:
        case WGPUTextureFormat_ASTC6x6UnormSrgb:
        case WGPUTextureFormat_ASTC8x5UnormSrgb:
        case WGPUTextureFormat_ASTC8x6UnormSrgb:
        case WGPUTextureFormat_ASTC8x8UnormSrgb:
        case WGPUTextureFormat_BC1RGBAUnormSrgb:
        case WGPUTextureFormat_BC2RGBAUnormSrgb:
        case WGPUTextureFormat_BC3RGBAUnormSrgb:
        case WGPUTextureFormat_BC7RGBAUnormSrgb:
        case WGPUTextureFormat_BGRA8UnormSrgb:
        case WGPUTextureFormat_ETC2RGB8A1UnormSrgb:
        case WGPUTextureFormat_ETC2RGB8UnormSrgb:
        case WGPUTextureFormat_ETC2RGBA8UnormSrgb:
        case WGPUTextureFormat_RGBA8UnormSrgb:
            gamma = 2.2f;
            break;
        default:
            gamma = 1.0f;
        }
        wgpuQueueWriteBuffer(bd->defaultQueue, bd->renderResources.Uniforms, offsetof(Uniforms, Gamma), &gamma, sizeof(Uniforms::Gamma));
    }

    // Setup viewport
    wgpuRenderPassEncoderSetViewport(ctx, 0, 0, draw_data->FramebufferScale.x * draw_data->DisplaySize.x, draw_data->FramebufferScale.y * draw_data->DisplaySize.y, 0, 1);

    // Bind shader and vertex buffers
    wgpuRenderPassEncoderSetVertexBuffer(ctx, 0, fr->VertexBuffer, 0, fr->VertexBufferSize * sizeof(RblxDrawVert));
    wgpuRenderPassEncoderSetIndexBuffer(ctx, fr->IndexBuffer, sizeof(RblxDrawIdx) == 2 ? WGPUIndexFormat_Uint16 : WGPUIndexFormat_Uint32, 0, fr->IndexBufferSize * sizeof(RblxDrawIdx));
    wgpuRenderPassEncoderSetPipeline(ctx, bd->pipelineState);
    wgpuRenderPassEncoderSetBindGroup(ctx, 0, bd->renderResources.CommonBindGroupLinear, 0, nullptr);

    // Setup blend factor
    WGPUColor blend_color = { 0.f, 0.f, 0.f, 0.f };
    wgpuRenderPassEncoderSetBlendConstant(ctx, &blend_color);
}

// Draw callbacks
static void RblxUI_ImplWGPU_DrawCallback_ResetRenderState(const RblxDrawList*, const RblxDrawCmd*)   {} // Intentionally empty. Used as an identifier for rendering loop to call its code. Simpler to implement this way.
static void RblxUI_ImplWGPU_DrawCallback_SetSamplerLinear(const RblxDrawList*, const RblxDrawCmd*)   { RblxUI_ImplWGPU_Data* bd = RblxUI_ImplWGPU_GetBackendData(); wgpuRenderPassEncoderSetBindGroup(bd->RenderState->RenderPassEncoder, 0, bd->renderResources.CommonBindGroupLinear, 0, nullptr); }
static void RblxUI_ImplWGPU_DrawCallback_SetSamplerNearest(const RblxDrawList*, const RblxDrawCmd*)  { RblxUI_ImplWGPU_Data* bd = RblxUI_ImplWGPU_GetBackendData(); wgpuRenderPassEncoderSetBindGroup(bd->RenderState->RenderPassEncoder, 0, bd->renderResources.CommonBindGroupNearest, 0, nullptr); }

// Render function
void RblxUI_ImplWGPU_RenderDrawData(RblxDrawData* draw_data, WGPURenderPassEncoder pass_encoder)
{
    // Avoid rendering when minimized
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0 || draw_data->CmdLists.Size == 0)
        return;

    // Catch up with texture updates. Most of the times, the list will have 1 element with an OK status, aka nothing to do.
    // (This almost always points to RblxUI::GetPlatformIO().Textures[] but is part of RblxDrawData to allow overriding or disabling texture updates).
    if (draw_data->Textures != nullptr)
        for (RblxTextureData* tex : *draw_data->Textures)
            if (tex->Status != RblxTextureStatus_OK)
                RblxUI_ImplWGPU_UpdateTexture(tex);

    // FIXME: Assuming that this only gets called once per frame!
    // If not, we can't just re-allocate the IB or VB, we'll have to do a proper allocator.
    RblxUI_ImplWGPU_Data* bd = RblxUI_ImplWGPU_GetBackendData();
    bd->frameIndex = bd->frameIndex + 1;
    FrameResources* fr = &bd->pFrameResources[bd->frameIndex % bd->numFramesInFlight];

    // Create and grow vertex/index buffers if needed
    if (fr->VertexBuffer == nullptr || fr->VertexBufferSize < draw_data->TotalVtxCount)
    {
        if (fr->VertexBuffer)
        {
            wgpuBufferDestroy(fr->VertexBuffer);
            wgpuBufferRelease(fr->VertexBuffer);
        }
        SafeRelease(fr->VertexBufferHost);
        fr->VertexBufferSize = draw_data->TotalVtxCount + 5000;

        WGPUBufferDescriptor vb_desc =
        {
            nullptr,
            { "Dear RblxUI Vertex buffer", WGPU_STRLEN, },
            WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex,
            MEMALIGN(fr->VertexBufferSize * sizeof(RblxDrawVert), 4),
            false
        };
        fr->VertexBuffer = wgpuDeviceCreateBuffer(bd->wgpuDevice, &vb_desc);
        if (!fr->VertexBuffer)
            return;

        fr->VertexBufferHost = new RblxDrawVert[fr->VertexBufferSize];
    }
    if (fr->IndexBuffer == nullptr || fr->IndexBufferSize < draw_data->TotalIdxCount)
    {
        if (fr->IndexBuffer)
        {
            wgpuBufferDestroy(fr->IndexBuffer);
            wgpuBufferRelease(fr->IndexBuffer);
        }
        SafeRelease(fr->IndexBufferHost);
        fr->IndexBufferSize = draw_data->TotalIdxCount + 10000;

        WGPUBufferDescriptor ib_desc =
        {
            nullptr,
            { "Dear RblxUI Index buffer", WGPU_STRLEN, },
            WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index,
            MEMALIGN(fr->IndexBufferSize * sizeof(RblxDrawIdx), 4),
            false
        };
        fr->IndexBuffer = wgpuDeviceCreateBuffer(bd->wgpuDevice, &ib_desc);
        if (!fr->IndexBuffer)
            return;

        fr->IndexBufferHost = new RblxDrawIdx[fr->IndexBufferSize];
    }

    // Upload vertex/index data into a single contiguous GPU buffer
    RblxDrawVert* vtx_dst = (RblxDrawVert*)fr->VertexBufferHost;
    RblxDrawIdx* idx_dst = (RblxDrawIdx*)fr->IndexBufferHost;
    for (const RblxDrawList* draw_list : draw_data->CmdLists)
    {
        memcpy(vtx_dst, draw_list->VtxBuffer.Data, draw_list->VtxBuffer.Size * sizeof(RblxDrawVert));
        memcpy(idx_dst, draw_list->IdxBuffer.Data, draw_list->IdxBuffer.Size * sizeof(RblxDrawIdx));
        vtx_dst += draw_list->VtxBuffer.Size;
        idx_dst += draw_list->IdxBuffer.Size;
    }
    int64_t vb_write_size = MEMALIGN((char*)vtx_dst - (char*)fr->VertexBufferHost, 4);
    int64_t ib_write_size = MEMALIGN((char*)idx_dst - (char*)fr->IndexBufferHost, 4);
    wgpuQueueWriteBuffer(bd->defaultQueue, fr->VertexBuffer, 0, fr->VertexBufferHost, (size_t)vb_write_size);
    wgpuQueueWriteBuffer(bd->defaultQueue, fr->IndexBuffer,  0, fr->IndexBufferHost, (size_t)ib_write_size);

    // Setup desired render state
    RblxUI_ImplWGPU_SetupRenderState(draw_data, pass_encoder, fr);

    // Setup render state structure (for callbacks and custom texture bindings)
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    RblxUI_ImplWGPU_RenderState render_state;
    render_state.Device = bd->wgpuDevice;
    render_state.RenderPassEncoder = pass_encoder;
    platform_io.Renderer_RenderState = bd->RenderState = &render_state;

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_vtx_offset = 0;
    int global_idx_offset = 0;
    RblxVec2 clip_scale = draw_data->FramebufferScale;
    RblxVec2 clip_off = draw_data->DisplayPos;
    for (const RblxDrawList* draw_list : draw_data->CmdLists)
    {
        for (int cmd_i = 0; cmd_i < draw_list->CmdBuffer.Size; cmd_i++)
        {
            const RblxDrawCmd* pcmd = &draw_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr)
            {
                // User callback, registered via RblxDrawList::AddCallback()
                if (pcmd->UserCallback == RblxUI_ImplWGPU_DrawCallback_ResetRenderState)
                    RblxUI_ImplWGPU_SetupRenderState(draw_data, pass_encoder, fr);
                else
                    pcmd->UserCallback(draw_list, pcmd);
            }
            else
            {
                // Bind custom texture
                RblxTextureID tex_id = pcmd->GetTexID();
                RblxUIID tex_id_hash = RblxHashData(&tex_id, sizeof(tex_id), 0);
                WGPUBindGroup bind_group = (WGPUBindGroup)bd->renderResources.ImageBindGroups.GetVoidPtr(tex_id_hash);
                if (!bind_group && tex_id != 0)
                    if ((bind_group = RblxUI_ImplWGPU_CreateImageBindGroup(bd->renderResources.ImageBindGroupLayout, (WGPUTextureView)tex_id)) != 0)
                        bd->renderResources.ImageBindGroups.SetVoidPtr(tex_id_hash, bind_group);
                if (bind_group)
                    wgpuRenderPassEncoderSetBindGroup(pass_encoder, 1, (WGPUBindGroup)bind_group, 0, nullptr);

                // Project scissor/clipping rectangles into framebuffer space
                RblxVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                RblxVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

                // Clamp to viewport as wgpuRenderPassEncoderSetScissorRect() won't accept values that are off bounds
                if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
                if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
                if (clip_max.x > (float)fb_width) { clip_max.x = (float)fb_width; }
                if (clip_max.y > (float)fb_height) { clip_max.y = (float)fb_height; }
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                // Apply scissor/clipping rectangle, Draw
                wgpuRenderPassEncoderSetScissorRect(pass_encoder, (uint32_t)clip_min.x, (uint32_t)clip_min.y, (uint32_t)(clip_max.x - clip_min.x), (uint32_t)(clip_max.y - clip_min.y));
                wgpuRenderPassEncoderDrawIndexed(pass_encoder, pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
            }
        }
        global_idx_offset += draw_list->IdxBuffer.Size;
        global_vtx_offset += draw_list->VtxBuffer.Size;
    }

    // Remove all ImageBindGroups
    RblxUIStorage& image_bind_groups = bd->renderResources.ImageBindGroups;
    for (int i = 0; i < image_bind_groups.Data.Size; i++)
    {
        WGPUBindGroup bind_group = (WGPUBindGroup)image_bind_groups.Data[i].val_p;
        SafeRelease(bind_group);
    }
    image_bind_groups.Data.resize(0);

    platform_io.Renderer_RenderState = bd->RenderState = nullptr;
}

static void RblxUI_ImplWGPU_DestroyTexture(RblxTextureData* tex)
{
    if (RblxUI_ImplWGPU_Texture* backend_tex = (RblxUI_ImplWGPU_Texture*)tex->BackendUserData)
    {
        RBLXUI_ASSERT(backend_tex->TextureView == (WGPUTextureView)(intptr_t)tex->TexID);
        wgpuTextureViewRelease(backend_tex->TextureView);
        wgpuTextureRelease(backend_tex->Texture);
        RBLXUI_DELETE(backend_tex);

        // Clear identifiers and mark as destroyed (in order to allow e.g. calling InvalidateDeviceObjects while running)
        tex->SetTexID(RblxTextureID_Invalid);
        tex->BackendUserData = nullptr;
    }
    tex->SetStatus(RblxTextureStatus_Destroyed);
}

void RblxUI_ImplWGPU_UpdateTexture(RblxTextureData* tex)
{
    RblxUI_ImplWGPU_Data* bd = RblxUI_ImplWGPU_GetBackendData();
    if (tex->Status == RblxTextureStatus_WantCreate)
    {
        // Create and upload new texture to graphics system
        //RBLXUI_DEBUG_LOG("UpdateTexture #%03d: WantCreate %dx%d\n", tex->UniqueID, tex->Width, tex->Height);
        RBLXUI_ASSERT(tex->TexID == RblxTextureID_Invalid && tex->BackendUserData == nullptr);
        RBLXUI_ASSERT(tex->Format == RblxTextureFormat_RGBA32);
        RblxUI_ImplWGPU_Texture* backend_tex = RBLXUI_NEW(RblxUI_ImplWGPU_Texture)();

        // Create texture
        WGPUTextureDescriptor tex_desc = {};
        tex_desc.label = { "Dear RblxUI Texture", WGPU_STRLEN };
        tex_desc.dimension = WGPUTextureDimension_2D;
        tex_desc.size.width = tex->Width;
        tex_desc.size.height = tex->Height;
        tex_desc.size.depthOrArrayLayers = 1;
        tex_desc.sampleCount = 1;
        tex_desc.format = WGPUTextureFormat_RGBA8Unorm;
        tex_desc.mipLevelCount = 1;
        tex_desc.usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding;
        backend_tex->Texture = wgpuDeviceCreateTexture(bd->wgpuDevice, &tex_desc);

        // Create texture view
        WGPUTextureViewDescriptor tex_view_desc = {};
        tex_view_desc.format = WGPUTextureFormat_RGBA8Unorm;
        tex_view_desc.dimension = WGPUTextureViewDimension_2D;
        tex_view_desc.baseMipLevel = 0;
        tex_view_desc.mipLevelCount = 1;
        tex_view_desc.baseArrayLayer = 0;
        tex_view_desc.arrayLayerCount = 1;
        tex_view_desc.aspect = WGPUTextureAspect_All;
        backend_tex->TextureView = wgpuTextureCreateView(backend_tex->Texture, &tex_view_desc);

        // Store identifiers
        tex->SetTexID((RblxTextureID)(intptr_t)backend_tex->TextureView);
        tex->BackendUserData = backend_tex;
        // We don't set tex->Status to RblxTextureStatus_OK to let the code fallthrough below.
    }

    if (tex->Status == RblxTextureStatus_WantCreate || tex->Status == RblxTextureStatus_WantUpdates)
    {
        RblxUI_ImplWGPU_Texture* backend_tex = (RblxUI_ImplWGPU_Texture*)tex->BackendUserData;
        RBLXUI_ASSERT(tex->Format == RblxTextureFormat_RGBA32);

        // We could use the smaller rect on _WantCreate but using the full rect allows us to clear the texture.
        const int upload_x = (tex->Status == RblxTextureStatus_WantCreate) ? 0 : tex->UpdateRect.x;
        const int upload_y = (tex->Status == RblxTextureStatus_WantCreate) ? 0 : tex->UpdateRect.y;
        const int upload_w = (tex->Status == RblxTextureStatus_WantCreate) ? tex->Width : tex->UpdateRect.w;
        const int upload_h = (tex->Status == RblxTextureStatus_WantCreate) ? tex->Height : tex->UpdateRect.h;

        // Update full texture or selected blocks. We only ever write to textures regions which have never been used before!
        // This backend choose to use tex->UpdateRect but you can use tex->Updates[] to upload individual regions.
        WGPUTexelCopyTextureInfo dst_view = {};
        dst_view.texture = backend_tex->Texture;
        dst_view.mipLevel = 0;
        dst_view.origin = { (uint32_t)upload_x, (uint32_t)upload_y, 0 };
        dst_view.aspect = WGPUTextureAspect_All;
        WGPUTexelCopyBufferLayout layout = {};
        layout.offset = 0;
        layout.bytesPerRow = tex->Width * tex->BytesPerPixel;
        layout.rowsPerImage = upload_h;
        WGPUExtent3D write_size = { (uint32_t)upload_w, (uint32_t)upload_h, 1 };
        wgpuQueueWriteTexture(bd->defaultQueue, &dst_view, tex->GetPixelsAt(upload_x, upload_y), (uint32_t)(tex->Width * upload_h * tex->BytesPerPixel), &layout, &write_size);
        tex->SetStatus(RblxTextureStatus_OK);
    }
    if (tex->Status == RblxTextureStatus_WantDestroy && tex->UnusedFrames > 0)
        RblxUI_ImplWGPU_DestroyTexture(tex);
}

static void RblxUI_ImplWGPU_CreateUniformBuffer()
{
    RblxUI_ImplWGPU_Data* bd = RblxUI_ImplWGPU_GetBackendData();
    WGPUBufferDescriptor ub_desc =
    {
        nullptr,
        { "Dear RblxUI Uniform buffer", WGPU_STRLEN, },
        WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform,
        MEMALIGN(sizeof(Uniforms), 16),
        false
    };
    bd->renderResources.Uniforms = wgpuDeviceCreateBuffer(bd->wgpuDevice, &ub_desc);
}

bool RblxUI_ImplWGPU_CreateDeviceObjects()
{
    RblxUI_ImplWGPU_Data* bd = RblxUI_ImplWGPU_GetBackendData();
    if (!bd->wgpuDevice)
        return false;
    if (bd->pipelineState)
        RblxUI_ImplWGPU_InvalidateDeviceObjects();

    // Create render pipeline
    WGPURenderPipelineDescriptor graphics_pipeline_desc = {};
    graphics_pipeline_desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    graphics_pipeline_desc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    graphics_pipeline_desc.primitive.frontFace = WGPUFrontFace_CW;
    graphics_pipeline_desc.primitive.cullMode = WGPUCullMode_None;
    graphics_pipeline_desc.multisample = bd->initInfo.PipelineMultisampleState;

    // Bind group layouts
    WGPUBindGroupLayoutEntry common_bg_layout_entries[2] = {};
    common_bg_layout_entries[0].binding = 0;
    common_bg_layout_entries[0].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    common_bg_layout_entries[0].buffer.type = WGPUBufferBindingType_Uniform;
    common_bg_layout_entries[1].binding = 1;
    common_bg_layout_entries[1].visibility = WGPUShaderStage_Fragment;
    common_bg_layout_entries[1].sampler.type = WGPUSamplerBindingType_Filtering;

    WGPUBindGroupLayoutEntry image_bg_layout_entries[1] = {};
    image_bg_layout_entries[0].binding = 0;
    image_bg_layout_entries[0].visibility = WGPUShaderStage_Fragment;
    image_bg_layout_entries[0].texture.sampleType = WGPUTextureSampleType_Float;
    image_bg_layout_entries[0].texture.viewDimension = WGPUTextureViewDimension_2D;

    WGPUBindGroupLayoutDescriptor common_bg_layout_desc = {};
    common_bg_layout_desc.entryCount = 2;
    common_bg_layout_desc.entries = common_bg_layout_entries;

    WGPUBindGroupLayoutDescriptor image_bg_layout_desc = {};
    image_bg_layout_desc.entryCount = 1;
    image_bg_layout_desc.entries = image_bg_layout_entries;

    WGPUBindGroupLayout bg_layouts[2];
    bg_layouts[0] = wgpuDeviceCreateBindGroupLayout(bd->wgpuDevice, &common_bg_layout_desc);
    bg_layouts[1] = wgpuDeviceCreateBindGroupLayout(bd->wgpuDevice, &image_bg_layout_desc);

    WGPUPipelineLayoutDescriptor layout_desc = {};
    layout_desc.bindGroupLayoutCount = 2;
    layout_desc.bindGroupLayouts = bg_layouts;
    graphics_pipeline_desc.layout = wgpuDeviceCreatePipelineLayout(bd->wgpuDevice, &layout_desc);

    // Create the vertex shader
    WGPUProgrammableStageDescriptor vertex_shader_desc = RblxUI_ImplWGPU_CreateShaderModuleWGSL(__shader_vert_wgsl);
    if (!vertex_shader_desc.module) vertex_shader_desc = RblxUI_ImplWGPU_CreateShaderModuleSPIRV(__shader_vert_spirv, sizeof(__shader_vert_spirv));
    graphics_pipeline_desc.vertex.module = vertex_shader_desc.module;
    graphics_pipeline_desc.vertex.entryPoint = vertex_shader_desc.entryPoint;

    // Vertex input configuration
    WGPUVertexAttribute attribute_desc[] =
    {
#if defined RBLXUI_IMPL_WEBGPU_BACKEND_DAWN || defined RBLXUI_IMPL_WEBGPU_BACKEND_WGVK
        { nullptr, WGPUVertexFormat_Float32x2, (uint64_t)offsetof(RblxDrawVert, pos), 0 },
        { nullptr, WGPUVertexFormat_Float32x2, (uint64_t)offsetof(RblxDrawVert, uv),  1 },
        { nullptr, WGPUVertexFormat_Unorm8x4,  (uint64_t)offsetof(RblxDrawVert, col), 2 },
#else
        { WGPUVertexFormat_Float32x2, (uint64_t)offsetof(RblxDrawVert, pos), 0 },
        { WGPUVertexFormat_Float32x2, (uint64_t)offsetof(RblxDrawVert, uv),  1 },
        { WGPUVertexFormat_Unorm8x4,  (uint64_t)offsetof(RblxDrawVert, col), 2 },
#endif
    };

    WGPUVertexBufferLayout buffer_layouts[1];
    buffer_layouts[0].arrayStride = sizeof(RblxDrawVert);
    buffer_layouts[0].stepMode = WGPUVertexStepMode_Vertex;
    buffer_layouts[0].attributeCount = 3;
    buffer_layouts[0].attributes = attribute_desc;

    graphics_pipeline_desc.vertex.bufferCount = 1;
    graphics_pipeline_desc.vertex.buffers = buffer_layouts;

    // Create the pixel shader
    WGPUProgrammableStageDescriptor pixel_shader_desc = RblxUI_ImplWGPU_CreateShaderModuleWGSL(__shader_frag_wgsl);
    if (!pixel_shader_desc.module)  pixel_shader_desc = RblxUI_ImplWGPU_CreateShaderModuleSPIRV(__shader_frag_spirv, sizeof(__shader_frag_spirv));

    // Create the blending setup
    WGPUBlendState blend_state = {};
    blend_state.alpha.operation = WGPUBlendOperation_Add;
    blend_state.alpha.srcFactor = WGPUBlendFactor_One;
    blend_state.alpha.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blend_state.color.operation = WGPUBlendOperation_Add;
    blend_state.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blend_state.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;

    WGPUColorTargetState color_state = {};
    color_state.format = bd->renderTargetFormat;
    color_state.blend = &blend_state;
    color_state.writeMask = WGPUColorWriteMask_All;

    WGPUFragmentState fragment_state = {};
    fragment_state.module = pixel_shader_desc.module;
    fragment_state.entryPoint = pixel_shader_desc.entryPoint;
    fragment_state.targetCount = 1;
    fragment_state.targets = &color_state;

    graphics_pipeline_desc.fragment = &fragment_state;

    // Create depth-stencil State
    WGPUDepthStencilState depth_stencil_state = {};
    depth_stencil_state.format = bd->depthStencilFormat;
    depth_stencil_state.depthWriteEnabled = WGPUOptionalBool_False;
    depth_stencil_state.depthCompare = WGPUCompareFunction_Always;
    depth_stencil_state.stencilFront.compare = WGPUCompareFunction_Always;
    depth_stencil_state.stencilFront.failOp = WGPUStencilOperation_Keep;
    depth_stencil_state.stencilFront.depthFailOp = WGPUStencilOperation_Keep;
    depth_stencil_state.stencilFront.passOp = WGPUStencilOperation_Keep;
    depth_stencil_state.stencilBack.compare = WGPUCompareFunction_Always;
    depth_stencil_state.stencilBack.failOp = WGPUStencilOperation_Keep;
    depth_stencil_state.stencilBack.depthFailOp = WGPUStencilOperation_Keep;
    depth_stencil_state.stencilBack.passOp = WGPUStencilOperation_Keep;

    // Configure disabled depth-stencil state
    graphics_pipeline_desc.depthStencil = (bd->depthStencilFormat == WGPUTextureFormat_Undefined) ? nullptr :  &depth_stencil_state;

    bd->pipelineState = wgpuDeviceCreateRenderPipeline(bd->wgpuDevice, &graphics_pipeline_desc);

    RblxUI_ImplWGPU_CreateUniformBuffer();

    // Create samplers (Linear/Nearest)
    // (Bilinear sampling is required by default. Set 'io.Fonts->Flags |= RblxFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to allow point/nearest sampling)
    WGPUSamplerDescriptor sampler_desc = {};
    sampler_desc.addressModeU = WGPUAddressMode_ClampToEdge;
    sampler_desc.addressModeV = WGPUAddressMode_ClampToEdge;
    sampler_desc.addressModeW = WGPUAddressMode_ClampToEdge;
    sampler_desc.maxAnisotropy = 1;
    sampler_desc.minFilter = WGPUFilterMode_Linear;
    sampler_desc.magFilter = WGPUFilterMode_Linear;
    sampler_desc.mipmapFilter = WGPUMipmapFilterMode_Linear;
    bd->renderResources.SamplerLinear = wgpuDeviceCreateSampler(bd->wgpuDevice, &sampler_desc);
    sampler_desc.minFilter = WGPUFilterMode_Nearest;
    sampler_desc.magFilter = WGPUFilterMode_Nearest;
    sampler_desc.mipmapFilter = WGPUMipmapFilterMode_Nearest;
    bd->renderResources.SamplerNearest = wgpuDeviceCreateSampler(bd->wgpuDevice, &sampler_desc);

    // Create resource bind groups (one per sampler, otherwise identical)
    WGPUBindGroupEntry common_bg_entries[] =
    {
        { nullptr, 0, bd->renderResources.Uniforms, 0, MEMALIGN(sizeof(Uniforms), 16), 0, 0 },
        { nullptr, 1, 0, 0, 0, bd->renderResources.SamplerLinear, 0 },
    };
    WGPUBindGroupDescriptor common_bg_descriptor = {};
    common_bg_descriptor.layout = bg_layouts[0];
    common_bg_descriptor.entryCount = sizeof(common_bg_entries) / sizeof(WGPUBindGroupEntry);
    common_bg_descriptor.entries = common_bg_entries;
    bd->renderResources.CommonBindGroupLinear = wgpuDeviceCreateBindGroup(bd->wgpuDevice, &common_bg_descriptor);
    common_bg_entries[1].sampler = bd->renderResources.SamplerNearest;
    bd->renderResources.CommonBindGroupNearest = wgpuDeviceCreateBindGroup(bd->wgpuDevice, &common_bg_descriptor);
    bd->renderResources.ImageBindGroupLayout = bg_layouts[1];

    SafeRelease(vertex_shader_desc.module);
    SafeRelease(pixel_shader_desc.module);
    SafeRelease(graphics_pipeline_desc.layout);
    SafeRelease(bg_layouts[0]);

    return true;
}

void RblxUI_ImplWGPU_InvalidateDeviceObjects()
{
    RblxUI_ImplWGPU_Data* bd = RblxUI_ImplWGPU_GetBackendData();
    if (!bd->wgpuDevice)
        return;

    SafeRelease(bd->pipelineState);
    SafeRelease(bd->renderResources);

    // Destroy all textures
    for (RblxTextureData* tex : RblxUI::GetPlatformIO().Textures)
        if (tex->RefCount == 1)
            RblxUI_ImplWGPU_DestroyTexture(tex);

    for (unsigned int i = 0; i < bd->numFramesInFlight; i++)
        SafeRelease(bd->pFrameResources[i]);
}

bool RblxUI_ImplWGPU_Init(RblxUI_ImplWGPU_InitInfo* init_info)
{
    RblxUIIO& io = RblxUI::GetIO();
    RBLXUI_CHECKVERSION();
    RBLXUI_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    RblxUI_ImplWGPU_Data* bd = RBLXUI_NEW(RblxUI_ImplWGPU_Data)();
    io.BackendRendererUserData = (void*)bd;
#if defined(RBLXUI_IMPL_WEBGPU_BACKEND_DAWN)
#if defined(__EMSCRIPTEN__)
    io.BackendRendererName = "rblxui_impl_wgpu (Dawn, Emscripten)"; // compiled & linked using EMSCRIPTEN with "--use-port=emdawnwebgpu" flag
#else
    io.BackendRendererName = "rblxui_impl_wgpu (Dawn, Native)";
#endif
#elif defined(RBLXUI_IMPL_WEBGPU_BACKEND_WGPU)
    io.BackendRendererName = "rblxui_impl_wgpu (WGPU, Native)";
#elif defined(RBLXUI_IMPL_WEBGPU_BACKEND_WGVK)
    io.BackendRendererName = "rblxui_impl_wgpu (WGVK, Native)";
#endif
    io.BackendFlags |= RblxUIBackendFlags_RendererHasVtxOffset;  // We can honor the RblxDrawCmd::VtxOffset field, allowing for large meshes.
    io.BackendFlags |= RblxUIBackendFlags_RendererHasTextures;   // We can honor RblxUIPlatformIO::Textures[] requests during render.

    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    platform_io.DrawCallback_ResetRenderState = RblxUI_ImplWGPU_DrawCallback_ResetRenderState;
    platform_io.DrawCallback_SetSamplerLinear = RblxUI_ImplWGPU_DrawCallback_SetSamplerLinear;
    platform_io.DrawCallback_SetSamplerNearest = RblxUI_ImplWGPU_DrawCallback_SetSamplerNearest;

    bd->initInfo = *init_info;
    bd->wgpuDevice = init_info->Device;
    bd->defaultQueue = wgpuDeviceGetQueue(bd->wgpuDevice);
    bd->renderTargetFormat = init_info->RenderTargetFormat;
    bd->depthStencilFormat = init_info->DepthStencilFormat;
    bd->numFramesInFlight = init_info->NumFramesInFlight;
    bd->frameIndex = UINT_MAX;

    bd->renderResources.ImageBindGroups.Data.reserve(100);

    // Create buffers with a default size (they will later be grown as needed)
    bd->pFrameResources = new FrameResources[bd->numFramesInFlight];
    for (unsigned int i = 0; i < bd->numFramesInFlight; i++)
    {
        FrameResources* fr = &bd->pFrameResources[i];
        fr->IndexBuffer = nullptr;
        fr->VertexBuffer = nullptr;
        fr->IndexBufferHost = nullptr;
        fr->VertexBufferHost = nullptr;
        fr->IndexBufferSize = 10000;
        fr->VertexBufferSize = 5000;
    }

    return true;
}

void RblxUI_ImplWGPU_Shutdown()
{
    RblxUI_ImplWGPU_Data* bd = RblxUI_ImplWGPU_GetBackendData();
    RBLXUI_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
    RblxUIIO& io = RblxUI::GetIO();
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();

    RblxUI_ImplWGPU_InvalidateDeviceObjects();
    delete[] bd->pFrameResources;
    bd->pFrameResources = nullptr;
    wgpuQueueRelease(bd->defaultQueue);
    bd->wgpuDevice = nullptr;
    bd->numFramesInFlight = 0;
    bd->frameIndex = UINT_MAX;

    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags &= ~(RblxUIBackendFlags_RendererHasVtxOffset | RblxUIBackendFlags_RendererHasTextures);
    platform_io.ClearRendererHandlers();
    RBLXUI_DELETE(bd);
}

void RblxUI_ImplWGPU_NewFrame()
{
    RblxUI_ImplWGPU_Data* bd = RblxUI_ImplWGPU_GetBackendData();
    if (!bd->pipelineState)
        if (!RblxUI_ImplWGPU_CreateDeviceObjects())
            RBLXUI_ASSERT(0 && "RblxUI_ImplWGPU_CreateDeviceObjects() failed!");
}

//-------------------------------------------------------------------------
// Internal Helpers
// Those are currently used by our example applications.
//-------------------------------------------------------------------------

bool RblxUI_ImplWGPU_IsSurfaceStatusError(WGPUSurfaceGetCurrentTextureStatus status)
{
#if defined(RBLXUI_IMPL_WEBGPU_BACKEND_DAWN) || defined(RBLXUI_IMPL_WEBGPU_BACKEND_WGVK)
    return (status == WGPUSurfaceGetCurrentTextureStatus_Error);
#else
    return (status == WGPUSurfaceGetCurrentTextureStatus_OutOfMemory || status == WGPUSurfaceGetCurrentTextureStatus_DeviceLost);
#endif
}

bool RblxUI_ImplWGPU_IsSurfaceStatusSubOptimal(WGPUSurfaceGetCurrentTextureStatus status)
{
#if defined(__EMSCRIPTEN__) && !defined(RBLXUI_IMPL_WEBGPU_BACKEND_DAWN)
    return (status == WGPUSurfaceGetCurrentTextureStatus_Timeout || status == WGPUSurfaceGetCurrentTextureStatus_Outdated || status == WGPUSurfaceGetCurrentTextureStatus_Lost);
#else
    return (status == WGPUSurfaceGetCurrentTextureStatus_Timeout || status == WGPUSurfaceGetCurrentTextureStatus_Outdated || status == WGPUSurfaceGetCurrentTextureStatus_Lost || status == WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal);
#endif
}

// Helpers to obtain a string
#if defined(RBLXUI_IMPL_WEBGPU_BACKEND_DAWN) || defined(RBLXUI_IMPL_WEBGPU_BACKEND_WGVK)
const char* RblxUI_ImplWGPU_GetErrorTypeName(WGPUErrorType type)
{
    switch (type)
    {
    case WGPUErrorType_Validation: return "Validation";
    case WGPUErrorType_OutOfMemory: return "OutOfMemory";
    case WGPUErrorType_Unknown: return "Unknown";
    case WGPUErrorType_Internal: return "Internal";
    default: return "Unknown";
    }
}
const char* RblxUI_ImplWGPU_GetDeviceLostReasonName(WGPUDeviceLostReason type)
{
    switch (type)
    {
    case WGPUDeviceLostReason_Unknown: return "Unknown";
    case WGPUDeviceLostReason_Destroyed: return "Destroyed";
    case WGPUDeviceLostReason_CallbackCancelled: return "CallbackCancelled";
    case WGPUDeviceLostReason_FailedCreation: return "FailedCreation";
    default: return "Unknown";
    }
}
#elif !defined(__EMSCRIPTEN__)
const char* RblxUI_ImplWGPU_GetLogLevelName(WGPULogLevel level)
{
    switch (level)
    {
    case WGPULogLevel_Error: return "Error"; 
    case WGPULogLevel_Warn: return "Warn";
    case WGPULogLevel_Info: return "Info";
    case WGPULogLevel_Debug: return "Debug";
    case WGPULogLevel_Trace: return "Trace";
    default: return "Unknown";
    }
}
#endif

const char* RblxUI_ImplWGPU_GetBackendTypeName(WGPUBackendType type)
{
    switch (type)
    {
    case WGPUBackendType_WebGPU: return "WebGPU";
    case WGPUBackendType_D3D11: return "D3D11";
    case WGPUBackendType_D3D12: return "D3D12";
    case WGPUBackendType_Metal: return "Metal";
    case WGPUBackendType_Vulkan: return "Vulkan";
    case WGPUBackendType_OpenGL: return "OpenGL";
    case WGPUBackendType_OpenGLES: return "OpenGLES";
    default: return "Unknown";
    }
}

const char* RblxUI_ImplWGPU_GetAdapterTypeName(WGPUAdapterType type)
{
    switch (type)
    {
    case WGPUAdapterType_DiscreteGPU: return "DiscreteGPU";
    case WGPUAdapterType_IntegratedGPU: return "IntegratedGPU";
    case WGPUAdapterType_CPU: return "CPU";
    default: return "Unknown";
    }
}

void RblxUI_ImplWGPU_DebugPrintAdapterInfo(const WGPUAdapter& adapter)
{
    WGPUAdapterInfo info = {};
    wgpuAdapterGetInfo(adapter, &info);
    printf("description: \"%.*s\"\n", (int)info.description.length, info.description.data);
    printf("vendor: \"%.*s\", vendorID: %x\n", (int)info.vendor.length, info.vendor.data, info.vendorID);
    printf("architecture: \"%.*s\"\n", (int) info.architecture.length, info.architecture.data);
    printf("device: \"%.*s\", deviceID: %x\n", (int)info.device.length, info.device.data, info.deviceID);
    printf("backendType: \"%s\"\n", RblxUI_ImplWGPU_GetBackendTypeName(info.backendType));
    printf("adapterType: \"%s\"\n", RblxUI_ImplWGPU_GetAdapterTypeName(info.adapterType));
    wgpuAdapterInfoFreeMembers(info);
}

#ifndef __EMSCRIPTEN__

#if defined(__APPLE__)
// MacOS specific: is necessary to compile with "-x objective-c++" flags
// (e.g. using cmake: set_source_files_properties(${RBLXUI_DIR}/backends/rblxui_impl_wgpu.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++") )
#include <TargetConditionals.h>
#if TARGET_OS_OSX
#include <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>
#endif
#endif

WGPUSurface RblxUI_ImplWGPU_CreateWGPUSurfaceHelper(RblxUI_ImplWGPU_CreateSurfaceInfo* info)
{
    WGPUSurfaceDescriptor surface_descriptor = {};
    WGPUSurface surface = {};
#if defined(__APPLE__) && TARGET_OS_OSX
    if (strcmp(info->System, "cocoa") == 0)
    {
        RBLXUI_ASSERT(info->RawWindow != nullptr);
        NSWindow* ns_window = (NSWindow*)info->RawWindow;
        id metal_layer = [CAMetalLayer layer];
        [ns_window.contentView setWantsLayer : YES] ;
        [ns_window.contentView setLayer : metal_layer] ;
        WGPUSurfaceSourceMetalLayer surface_src_metal = {};
        surface_src_metal.chain.sType = WGPUSType_SurfaceSourceMetalLayer;
        surface_src_metal.layer = metal_layer;
        surface_descriptor.nextInChain = &surface_src_metal.chain;
        surface = wgpuInstanceCreateSurface(info->Instance, &surface_descriptor);
    }
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
    if (strcmp(info->System, "wayland") == 0)
    {
        RBLXUI_ASSERT(info->RawDisplay != nullptr && info->RawSurface != nullptr);
        WGPUSurfaceSourceWaylandSurface surface_src_wayland = {};
        surface_src_wayland.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
        surface_src_wayland.display = info->RawDisplay;
        surface_src_wayland.surface = info->RawSurface;
        surface_descriptor.nextInChain = &surface_src_wayland.chain;
        surface = wgpuInstanceCreateSurface(info->Instance, &surface_descriptor);
    }
    else if (strcmp(info->System, "x11") == 0)
    {
        RBLXUI_ASSERT(info->RawDisplay != nullptr && info->RawWindow != nullptr);
        WGPUSurfaceSourceXlibWindow surface_src_xlib = {};
        surface_src_xlib.chain.sType = WGPUSType_SurfaceSourceXlibWindow;
        surface_src_xlib.display = info->RawDisplay;
        surface_src_xlib.window = (uint64_t)info->RawWindow;
        surface_descriptor.nextInChain = &surface_src_xlib.chain;
        surface = wgpuInstanceCreateSurface(info->Instance, &surface_descriptor);
    }
#elif defined(_WIN32)
    if (strcmp(info->System, "win32") == 0)
    {
        RBLXUI_ASSERT(info->RawWindow != nullptr && info->RawInstance != nullptr);
        WGPUSurfaceSourceWindowsHWND surface_src_hwnd = {};
        surface_src_hwnd.chain.sType = WGPUSType_SurfaceSourceWindowsHWND;
        surface_src_hwnd.hinstance = info->RawInstance;
        surface_src_hwnd.hwnd = info->RawWindow;
        surface_descriptor.nextInChain = &surface_src_hwnd.chain;
        surface = wgpuInstanceCreateSurface(info->Instance, &surface_descriptor);
    }
#else
    RBLXUI_ASSERT(0 && "Unsupported WebGPU native platform!");
#endif
    return surface;
}
#endif // #ifndef __EMSCRIPTEN__

//-----------------------------------------------------------------------------

#endif // #ifndef RBLXUI_DISABLE
