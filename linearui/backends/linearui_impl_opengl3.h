// dear linearui: Renderer Backend for modern OpenGL with shaders / programmatic pipeline
// - Desktop GL: 2.x 3.x 4.x
// - Embedded GL: ES 2.0 (WebGL 1.0), ES 3.0 (WebGL 2.0)
// This needs to be used along with a Platform Backend (e.g. LINEARWIN, SDL, Win32, custom..)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'GLuint' OpenGL texture as texture identifier. Read the FAQ about LinearTextureID/LinearTextureRef!
//  [x] Renderer: Large meshes support (64k+ vertices) even with 16-bit indices (LinearUIBackendFlags_RendererHasVtxOffset) [Desktop OpenGL only!]
//  [X] Renderer: Texture updates support for dynamic font atlas (LinearUIBackendFlags_RendererHasTextures).
//  [X] Renderer: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= LinearUIConfigFlags_ViewportsEnable'.

// About WebGL/ES:
// - You need to '#define LINEARUI_IMPL_OPENGL_ES2' or '#define LINEARUI_IMPL_OPENGL_ES3' to use WebGL or OpenGL ES.
// - This is done automatically on iOS, Android and Emscripten targets.
// - For other targets, the define needs to be visible from the linearui_impl_opengl3.cpp compilation unit. If unsure, define globally or in linearuiconfig.h.

// You can use unmodified linearui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire linearui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear LinearUI:
// - FAQ                  https://dearlinearui.com/faq
// - Getting Started      https://dearlinearui.com/getting-started
// - Documentation        https://dearlinearui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of linearui.cpp

// About GLSL version:
//  The 'glsl_version' initialization parameter should be nullptr (default) or a "#version XXX" string.
//  On computer platform the GLSL version default to "#version 130". On OpenGL ES 3 platform it defaults to "#version 300 es"
//  Only override if your GL version doesn't handle this GLSL version. See GLSL version table at the top of linearui_impl_opengl3.cpp.

#pragma once
#include "linearui.h"      // LINEARUI_IMPL_API
#ifndef LINEARUI_DISABLE

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
LINEARUI_IMPL_API bool     LinearUI_ImplOpenGL3_Init(const char* glsl_version = nullptr);
LINEARUI_IMPL_API void     LinearUI_ImplOpenGL3_Shutdown();
LINEARUI_IMPL_API void     LinearUI_ImplOpenGL3_NewFrame();
LINEARUI_IMPL_API void     LinearUI_ImplOpenGL3_RenderDrawData(LinearDrawData* draw_data);

// (Optional) Called by Init/NewFrame/Shutdown
LINEARUI_IMPL_API bool     LinearUI_ImplOpenGL3_CreateDeviceObjects();
LINEARUI_IMPL_API void     LinearUI_ImplOpenGL3_DestroyDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting LinearDrawData::Textures = nullptr to handle this manually.
LINEARUI_IMPL_API void     LinearUI_ImplOpenGL3_UpdateTexture(LinearTextureData* tex);

// Configuration flags to add in your linearuiconfig file:
//#define LINEARUI_IMPL_OPENGL_ES2     // Enable ES 2 (Auto-detected on Emscripten)
//#define LINEARUI_IMPL_OPENGL_ES3     // Enable ES 3 (Auto-detected on iOS/Android)

// You can explicitly select GLES2 or GLES3 API by using one of the '#define LINEARUI_IMPL_OPENGL_LOADER_XXX' in linearuiconfig.h or compiler command-line.
#if !defined(LINEARUI_IMPL_OPENGL_ES2) \
 && !defined(LINEARUI_IMPL_OPENGL_ES3)

// Try to detect GLES on matching platforms
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV)) || (defined(__ANDROID__))
#define LINEARUI_IMPL_OPENGL_ES3               // iOS, Android  -> GL ES 3, "#version 300 es"
#elif defined(__EMSCRIPTEN__) || defined(__amigaos4__)
#define LINEARUI_IMPL_OPENGL_ES2               // Emscripten    -> GL ES 2, "#version 100"
#else
// Otherwise linearui_impl_opengl3_loader.h will be used.
#endif

#endif

#endif // #ifndef LINEARUI_DISABLE
