// dear linearui: Renderer Backend for OpenGL2 (legacy OpenGL, fixed pipeline)
// This needs to be used along with a Platform Backend (e.g. LINEARWIN, SDL, Win32, custom..)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'GLuint' OpenGL texture as texture identifier. Read the FAQ about LinearTextureID/LinearTextureRef!
//  [X] Renderer: Texture updates support for dynamic font atlas (LinearUIBackendFlags_RendererHasTextures).
//  [X] Renderer: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= LinearUIConfigFlags_ViewportsEnable'.
// Missing features or Issues:
//  [ ] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (LinearUIBackendFlags_RendererHasVtxOffset).
//  [ ] Renderer: Use of DrawCallback_SetSamplerLinear, DrawCallback_SetSamplerNearest is emulated by poking to glTexParameter(), as legacy OpenGL doesn't have glBindSampler().

// You can use unmodified linearui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire linearui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear LinearUI:
// - FAQ                  https://dearlinearui.com/faq
// - Getting Started      https://dearlinearui.com/getting-started
// - Documentation        https://dearlinearui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of linearui.cpp

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in linearui_impl_opengl3.cpp**
// This code is mostly provided as a reference to learn how LinearUI integration works, because it is shorter to read.
// If your code is using GL3+ context or any semi modern OpenGL calls, using this is likely to make everything more
// complicated, will require your code to reset every single OpenGL attributes to their initial state, and might
// confuse your GPU driver.
// The GL2 code is unable to reset attributes or even call e.g. "glUseProgram(0)" because they don't exist in that API.

#pragma once
#include "linearui.h"      // LINEARUI_IMPL_API
#ifndef LINEARUI_DISABLE

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
LINEARUI_IMPL_API bool     LinearUI_ImplOpenGL2_Init();
LINEARUI_IMPL_API void     LinearUI_ImplOpenGL2_Shutdown();
LINEARUI_IMPL_API void     LinearUI_ImplOpenGL2_NewFrame();
LINEARUI_IMPL_API void     LinearUI_ImplOpenGL2_RenderDrawData(LinearDrawData* draw_data);

// Called by Init/NewFrame/Shutdown
LINEARUI_IMPL_API bool     LinearUI_ImplOpenGL2_CreateDeviceObjects();
LINEARUI_IMPL_API void     LinearUI_ImplOpenGL2_DestroyDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting LinearDrawData::Textures = nullptr to handle this manually.
LINEARUI_IMPL_API void     LinearUI_ImplOpenGL2_UpdateTexture(LinearTextureData* tex);

#endif // #ifndef LINEARUI_DISABLE
