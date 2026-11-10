// https://github.com/CedricGuillemet/RblxGizmo
// v1.92.5 WIP
//
// The MIT License(MIT)
//
// Copyright(c) 2016-2026 Cedric Guillemet and contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#pragma once

#include <stdint.h>

// ---------------------------------------------------------------------------
// Platform headers and glGetProcAddress
// ---------------------------------------------------------------------------
#if defined(_WIN32)
#pragma comment(lib,"opengl32.lib")
#define WINDOWS_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <GL/GL.h>
#define glGetProcAddress(name) (void *)wglGetProcAddress((LPCSTR)name)

#elif defined(__APPLE__)
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif
#include <OpenGL/gl3.h>
#include <objc/objc.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <objc/NSObjCRuntime.h>
#include <Carbon/Carbon.h>
#include <dlfcn.h>
#include <cstring>
#define glGetProcAddress(name) dlsym(RTLD_DEFAULT, name)

#elif defined(__linux__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <time.h>
#include <cstring>
#define glGetProcAddress(name) (void*)glXGetProcAddressARB((const GLubyte*)(name))

#endif

// ---------------------------------------------------------------------------
// GL type definitions (needed when system headers don't provide them)
// ---------------------------------------------------------------------------
#ifndef GL_VERSION_1_5
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
#endif
#ifndef GL_VERSION_2_0
typedef char GLchar;
#endif

#if !defined(APIENTRY)
#if defined(_WIN32)
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif

// On some platforms these are already defined
#ifndef GL_DEPTH_BUFFER_BIT
#define GL_DEPTH_BUFFER_BIT               0x00000100
#endif
#ifndef GL_STENCIL_BUFFER_BIT
#define GL_STENCIL_BUFFER_BIT             0x00000400
#endif
#ifndef GL_COLOR_BUFFER_BIT
#define GL_COLOR_BUFFER_BIT               0x00004000
#endif
#ifndef GL_FALSE
#define GL_FALSE                          0
#endif
#ifndef GL_TRUE
#define GL_TRUE                           1
#endif
#ifndef GL_TRIANGLES
#define GL_TRIANGLES                      0x0004
#endif
#ifndef GL_CULL_FACE
#define GL_CULL_FACE                      0x0B44
#endif
#ifndef GL_DEPTH_TEST
#define GL_DEPTH_TEST                     0x0B71
#endif
#ifndef GL_SCISSOR_TEST
#define GL_SCISSOR_TEST                   0x0C11
#endif
#ifndef GL_BLEND
#define GL_BLEND                          0x0BE2
#endif
#ifndef GL_TEXTURE_2D
#define GL_TEXTURE_2D                     0x0DE1
#endif
#ifndef GL_UNSIGNED_BYTE
#define GL_UNSIGNED_BYTE                  0x1401
#endif
#ifndef GL_UNSIGNED_SHORT
#define GL_UNSIGNED_SHORT                 0x1403
#endif
#ifndef GL_UNSIGNED_INT
#define GL_UNSIGNED_INT                   0x1405
#endif
#ifndef GL_FLOAT
#define GL_FLOAT                          0x1406
#endif
#ifndef GL_RGBA
#define GL_RGBA                           0x1908
#endif
#ifndef GL_FILL
#define GL_FILL                           0x1B02
#endif
#ifndef GL_NEAREST
#define GL_NEAREST                        0x2600
#endif
#ifndef GL_LINEAR
#define GL_LINEAR                         0x2601
#endif
#ifndef GL_TEXTURE_MAG_FILTER
#define GL_TEXTURE_MAG_FILTER             0x2800
#endif
#ifndef GL_TEXTURE_MIN_FILTER
#define GL_TEXTURE_MIN_FILTER             0x2801
#endif
#ifndef GL_SRC_ALPHA
#define GL_SRC_ALPHA                      0x0302
#endif
#ifndef GL_ONE_MINUS_SRC_ALPHA
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#endif
#ifndef GL_FRONT_AND_BACK
#define GL_FRONT_AND_BACK                 0x0408
#endif
#ifndef GL_TEXTURE0
#define GL_TEXTURE0                       0x84C0
#endif
#ifndef GL_ACTIVE_TEXTURE
#define GL_ACTIVE_TEXTURE                 0x84E0
#endif
#ifndef GL_FUNC_ADD
#define GL_FUNC_ADD                       0x8006
#endif
#ifndef GL_BLEND_SRC_RGB
#define GL_BLEND_SRC_RGB                  0x80C9
#endif
#ifndef GL_BLEND_DST_RGB
#define GL_BLEND_DST_RGB                  0x80C8
#endif
#ifndef GL_BLEND_SRC_ALPHA
#define GL_BLEND_SRC_ALPHA                0x80CB
#endif
#ifndef GL_BLEND_DST_ALPHA
#define GL_BLEND_DST_ALPHA                0x80CA
#endif
#ifndef GL_BLEND_EQUATION_RGB
#define GL_BLEND_EQUATION_RGB             0x8009
#endif
#ifndef GL_BLEND_EQUATION_ALPHA
#define GL_BLEND_EQUATION_ALPHA           0x883D
#endif
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER                   0x8892
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#endif
#ifndef GL_ARRAY_BUFFER_BINDING
#define GL_ARRAY_BUFFER_BINDING           0x8894
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER_BINDING
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
#endif
#ifndef GL_STREAM_DRAW
#define GL_STREAM_DRAW                    0x88E0
#endif
#ifndef GL_CURRENT_PROGRAM
#define GL_CURRENT_PROGRAM                0x8B8D
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER                0x8B30
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER                  0x8B31
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS                 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS                    0x8B82
#endif
#ifndef GL_TEXTURE_BINDING_2D
#define GL_TEXTURE_BINDING_2D             0x8069
#endif
#ifndef GL_SAMPLER_BINDING
#define GL_SAMPLER_BINDING                0x8919
#endif
#ifndef GL_VERTEX_ARRAY_BINDING
#define GL_VERTEX_ARRAY_BINDING           0x85B5
#endif
#ifndef GL_POLYGON_MODE
#define GL_POLYGON_MODE                   0x0B40
#endif
#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER                    0x8D40
#endif
#ifndef GL_RENDERBUFFER
#define GL_RENDERBUFFER                   0x8D41
#endif
#ifndef GL_FRAMEBUFFER_BINDING
#define GL_FRAMEBUFFER_BINDING            0x8CA6
#endif
#ifndef GL_DRAW_FRAMEBUFFER_BINDING
#define GL_DRAW_FRAMEBUFFER_BINDING       GL_FRAMEBUFFER_BINDING
#endif
#ifndef GL_RENDERBUFFER_BINDING
#define GL_RENDERBUFFER_BINDING           0x8CA7
#endif
#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER               0x8CA8
#endif
#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#endif
#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0              0x8CE0
#endif
#ifndef GL_DEPTH_ATTACHMENT
#define GL_DEPTH_ATTACHMENT               0x8D00
#endif
#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#endif
#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24              0x81A6
#endif
#ifndef GL_RGBA8
#define GL_RGBA8                          0x8058
#endif
#ifndef GL_RGB
#define GL_RGB                            0x1907
#endif
#ifndef GL_RED
#define GL_RED                            0x1903
#endif
#ifndef GL_RGB8
#define GL_RGB8                           0x8051
#endif

// GL function pointer types
typedef void   (APIENTRY* PFNGLACTIVETEXTUREPROC)(unsigned int texture);
typedef void   (APIENTRY* PFNGLBLENDFUNCSEPARATEPROC)(unsigned int sfactorRGB, unsigned int dfactorRGB, unsigned int sfactorAlpha, unsigned int dfactorAlpha);
typedef void   (APIENTRY* PFNGLBLENDEQUATIONSEPARATEPROC)(unsigned int modeRGB, unsigned int modeAlpha);
typedef void   (APIENTRY* PFNGLBLENDEQUATIONPROC)(unsigned int mode);

// ---------------------------------------------------------------------------
// GL extension function pointers
// ---------------------------------------------------------------------------
#ifndef IMAPP_IMPL
#define GLEXTERN extern
#else
#define GLEXTERN
#endif

#if defined(_WIN32)
GLEXTERN PFNGLACTIVETEXTUREPROC glActiveTexture_fn;
#define glActiveTexture glActiveTexture_fn
#endif

GLEXTERN void(APIENTRY* glUniform1i_fn) (int location, int v0);
GLEXTERN void(APIENTRY* glUniformMatrix3fv_fn) (int location, int count, unsigned char transpose, const float* value);
GLEXTERN void(APIENTRY* glBindFramebuffer_fn) (unsigned int target, unsigned int framebuffer);
GLEXTERN void(APIENTRY* glDeleteFramebuffers_fn) (int n, const unsigned int* framebuffers);
GLEXTERN void(APIENTRY* glDeleteRenderbuffers_fn) (int n, const unsigned int* renderbuffers);
GLEXTERN void(APIENTRY* glFramebufferTexture2D_fn) (unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, int level);
GLEXTERN void(APIENTRY* glFramebufferRenderbuffer_fn) (unsigned int target, unsigned int attachment, unsigned int renderbuffertarget, unsigned int renderbuffer);
GLEXTERN void(APIENTRY* glRenderbufferStorage_fn) (unsigned int target, unsigned int internalformat, int width, int height);
GLEXTERN void(APIENTRY* glGenFramebuffers_fn) (int n, unsigned int* framebuffers);
GLEXTERN void(APIENTRY* glGenRenderbuffers_fn) (int n, unsigned int* renderbuffers);
GLEXTERN void(APIENTRY* glBindRenderbuffer_fn) (unsigned int target, unsigned int renderbuffer);
GLEXTERN unsigned int(APIENTRY* glCheckFramebufferStatus_fn) (unsigned int target);
GLEXTERN void(APIENTRY* glGenerateMipmap_fn) (unsigned int target);
GLEXTERN void(APIENTRY* glBufferData_fn) (unsigned int target, ptrdiff_t size, const void* data, unsigned int usage);
GLEXTERN void(APIENTRY* glUseProgram_fn) (unsigned int program);
GLEXTERN int(APIENTRY* glGetUniformLocation_fn) (unsigned int program, const char* name);
GLEXTERN int(APIENTRY* glGetAttribLocation_fn) (unsigned int program, const char* name);
GLEXTERN void(APIENTRY* glDeleteBuffers_fn) (int n, const unsigned int* buffers);
GLEXTERN void(APIENTRY* glDeleteVertexArrays_fn) (int n, const unsigned int* arrays);
GLEXTERN void(APIENTRY* glEnableVertexAttribArray_fn) (unsigned int);
GLEXTERN void(APIENTRY* glVertexAttribPointer_fn) (unsigned int index, int size, unsigned int type, unsigned char normalized, int stride, const void* pointer);
GLEXTERN void(APIENTRY* glGenBuffers_fn) (int n, unsigned int* buffers);
GLEXTERN void(APIENTRY* glBindBuffer_fn) (unsigned int target, unsigned int buffer);
GLEXTERN unsigned int(APIENTRY* glCreateShader_fn) (unsigned int type);
GLEXTERN void(APIENTRY* glShaderSource_fn) (unsigned int shader, int count, const char** strings, const int* lengths);
GLEXTERN void(APIENTRY* glCompileShader_fn) (unsigned int shader);
GLEXTERN unsigned int(APIENTRY* glCreateProgram_fn) (void);
GLEXTERN void(APIENTRY* glAttachShader_fn) (unsigned int program, unsigned int shader);
GLEXTERN void(APIENTRY* glDeleteProgram_fn) (unsigned int program);
GLEXTERN void(APIENTRY* glDeleteShader_fn) (unsigned int shader);
GLEXTERN void(APIENTRY* glDisableVertexAttribArray_fn) (unsigned int);
GLEXTERN void(APIENTRY* glBindAttribLocation_fn) (unsigned int program, unsigned int index, const char* name);
GLEXTERN void(APIENTRY* glVertexAttribDivisor_fn) (unsigned int index, unsigned int divisor);
GLEXTERN void(APIENTRY* glUniformMatrix4fv_fn) (int location, int count, unsigned char transpose, const float* value);
GLEXTERN void(APIENTRY* glGetShaderiv_fn) (unsigned int shader, unsigned int pname, int* param);
GLEXTERN void(APIENTRY* glLinkProgram_fn) (unsigned int program);
GLEXTERN void(APIENTRY* glGetProgramiv_fn) (unsigned int program, unsigned int pname, int* param);
GLEXTERN void(APIENTRY* glBindVertexArray_fn) (unsigned int array);
GLEXTERN void(APIENTRY* glUniform2fv_fn) (int location, int count, const float* value);
GLEXTERN void(APIENTRY* glUniform3f_fn) (int location, float v0, float v1, float v2);
GLEXTERN void(APIENTRY* glUniform3fv_fn) (int location, int count, const float* value);
GLEXTERN void(APIENTRY* glUniform4fv_fn) (int location, int count, const float* value);
GLEXTERN void(APIENTRY* glBufferSubData_fn) (unsigned int target, ptrdiff_t offset, ptrdiff_t size, const void* data);
GLEXTERN void(APIENTRY* glGenVertexArrays_fn) (int n, const unsigned int* arrays);
GLEXTERN void(APIENTRY* glGetShaderInfoLog_fn) (unsigned int shader, int bufSize, int* length, char* infoLog);
GLEXTERN void(APIENTRY* glGetProgramInfoLog_fn) (unsigned int program, int bufSize, int* length, char* infoLog);
GLEXTERN unsigned int(APIENTRY* glGetUniformBlockIndex_fn) (unsigned int program, const char* uniformBlockName);
GLEXTERN void(APIENTRY* glUniformBlockBinding_fn) (unsigned int program, unsigned int uniformBlockIndex, unsigned int uniformBlockBinding);
GLEXTERN void(APIENTRY* glBindBufferBase_fn) (unsigned int target, unsigned int index, unsigned int buffer);
GLEXTERN void(APIENTRY* glTransformFeedbackVaryings_fn) (unsigned int, int, const char**, unsigned int);
GLEXTERN void* (APIENTRY* glMapBuffer_fn) (unsigned int target, unsigned int access);
GLEXTERN unsigned char(APIENTRY* glUnmapBuffer_fn) (unsigned int target);
GLEXTERN void(APIENTRY* glDrawElementsInstanced_fn) (unsigned int, int, unsigned int, const void*, int);
GLEXTERN void(APIENTRY* glDrawArraysInstanced_fn) (unsigned int, int, int, int);
GLEXTERN void(APIENTRY* glDrawElementsInstancedBaseVertex_fn) (unsigned int mode, int count, unsigned int type, const void* indices, int primcount, int basevertex);
GLEXTERN void(APIENTRY* glBeginTransformFeedback_fn) (unsigned int);
GLEXTERN void(APIENTRY* glEndTransformFeedback_fn) (void);
GLEXTERN void(APIENTRY* glUniform1f_fn) (int location, float v0);
GLEXTERN void(APIENTRY* glUniform2f_fn) (int location, float v0, float v1);
GLEXTERN PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate_fn;
GLEXTERN PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate_fn;
GLEXTERN void(APIENTRY* glGetBufferSubData_fn) (unsigned int target, ptrdiff_t offset, ptrdiff_t size, void* data);
GLEXTERN void(APIENTRY* glGetShaderSource_fn) (unsigned int shader, int bufSize, int* length, char* source);
GLEXTERN unsigned char(APIENTRY* glIsProgram_fn)(unsigned int program);
GLEXTERN void(APIENTRY* glGetAttachedShaders_fn)(unsigned int program, int maxCount, int* count, unsigned int* shaders);
GLEXTERN void(APIENTRY* glDrawBuffers_fn)(int n, const unsigned int* bufs);
GLEXTERN void(APIENTRY* glBlitFramebuffer_fn)(int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, unsigned int mask, unsigned int filter);
GLEXTERN PFNGLBLENDEQUATIONPROC glBlendEquation_fn;
GLEXTERN void(APIENTRY* glBindSampler_fn)(unsigned int unit, unsigned int sampler);
GLEXTERN void(APIENTRY* glDetachShader_fn)(unsigned int program, unsigned int shader);

// Aliases for compatibility with code that uses the short names
#ifndef glUniform1i
#define glUniform1i glUniform1i_fn
#endif
#ifndef glUniformMatrix3fv
#define glUniformMatrix3fv glUniformMatrix3fv_fn
#endif
#define glBindFramebuffer glBindFramebuffer_fn
#define glDeleteFramebuffers glDeleteFramebuffers_fn
#define glDeleteRenderbuffers glDeleteRenderbuffers_fn
#define glFramebufferTexture2D glFramebufferTexture2D_fn
#define glFramebufferRenderbuffer glFramebufferRenderbuffer_fn
#define glRenderbufferStorage glRenderbufferStorage_fn
#define glGenFramebuffers glGenFramebuffers_fn
#define glGenRenderbuffers glGenRenderbuffers_fn
#define glBindRenderbuffer glBindRenderbuffer_fn
#define glCheckFramebufferStatus glCheckFramebufferStatus_fn
#define glGenerateMipmap glGenerateMipmap_fn
#define glBufferData glBufferData_fn
#define glUseProgram glUseProgram_fn
#define glGetUniformLocation glGetUniformLocation_fn
#define glGetAttribLocation glGetAttribLocation_fn
#define glDeleteBuffers glDeleteBuffers_fn
#define glDeleteVertexArrays glDeleteVertexArrays_fn
#define glEnableVertexAttribArray glEnableVertexAttribArray_fn
#define glVertexAttribPointer glVertexAttribPointer_fn
#define glGenBuffers glGenBuffers_fn
#define glBindBuffer glBindBuffer_fn
#define glCreateShader glCreateShader_fn
#define glShaderSource glShaderSource_fn
#define glCompileShader glCompileShader_fn
#define glCreateProgram glCreateProgram_fn
#define glAttachShader glAttachShader_fn
#define glDeleteProgram glDeleteProgram_fn
#define glDeleteShader glDeleteShader_fn
#define glDisableVertexAttribArray glDisableVertexAttribArray_fn
#define glBindAttribLocation glBindAttribLocation_fn
#define glVertexAttribDivisor glVertexAttribDivisor_fn
#define glUniformMatrix4fv glUniformMatrix4fv_fn
#define glGetShaderiv glGetShaderiv_fn
#define glLinkProgram glLinkProgram_fn
#define glGetProgramiv glGetProgramiv_fn
#define glBindVertexArray glBindVertexArray_fn
#define glUniform2fv glUniform2fv_fn
#define glUniform3f glUniform3f_fn
#define glUniform3fv glUniform3fv_fn
#define glUniform4fv glUniform4fv_fn
#define glBufferSubData glBufferSubData_fn
#define glGenVertexArrays glGenVertexArrays_fn
#define glGetShaderInfoLog glGetShaderInfoLog_fn
#define glGetProgramInfoLog glGetProgramInfoLog_fn
#define glGetUniformBlockIndex glGetUniformBlockIndex_fn
#define glUniformBlockBinding glUniformBlockBinding_fn
#define glBindBufferBase glBindBufferBase_fn
#define glTransformFeedbackVaryings glTransformFeedbackVaryings_fn
#define glMapBuffer glMapBuffer_fn
#define glUnmapBuffer glUnmapBuffer_fn
#define glDrawElementsInstanced glDrawElementsInstanced_fn
#define glDrawArraysInstanced glDrawArraysInstanced_fn
#define glDrawElementsInstancedBaseVertex glDrawElementsInstancedBaseVertex_fn
#define glBeginTransformFeedback glBeginTransformFeedback_fn
#define glEndTransformFeedback glEndTransformFeedback_fn
#define glUniform1f glUniform1f_fn
#define glUniform2f glUniform2f_fn
#define glBlendEquationSeparate glBlendEquationSeparate_fn
#define glBlendFuncSeparate glBlendFuncSeparate_fn
#define glGetBufferSubData glGetBufferSubData_fn
#define glGetShaderSource glGetShaderSource_fn
#define glIsProgram glIsProgram_fn
#define glGetAttachedShaders glGetAttachedShaders_fn
#define glDrawBuffers glDrawBuffers_fn
#define glBlitFramebuffer glBlitFramebuffer_fn
#define glBlendEquation glBlendEquation_fn
#define glBindSampler glBindSampler_fn
#define glDetachShader glDetachShader_fn

// ---------------------------------------------------------------------------
// RblxApp namespace
// ---------------------------------------------------------------------------
namespace RblxApp
{
   struct Config
   {
      Config() : mWidth(1280), mHeight(720), mFullscreen(false) {}
      int mWidth;
      int mHeight;
      bool mFullscreen;
   };

   struct RblxApp
   {
      RblxApp() : mInitExtensionsDone(false), mExtensionsPresent(false), mDone(true)
      {
         mInstance = this;
      }
      ~RblxApp()
      {
         mInstance = NULL;
      }
      static RblxApp* Instance() { return mInstance; }

      int Init(const Config& config = Config());
      void NewFrame();
      void EndFrame();
      void Finish();
      bool Done() { return mDone; }

      void LoadBanks(int, const char**) {}
      void PlayEvent(const char*) {}

   protected:
      static RblxApp* mInstance;
      Config mConfig;
      bool mInitExtensionsDone;
      bool mExtensionsPresent;
      bool mDone;

      bool loadExtension(const char* extensionName, void** functionPtr)
      {
         *functionPtr = glGetProcAddress(extensionName);
         return (*functionPtr != NULL);
      }
#define LE(x) mExtensionsPresent &= loadExtension(#x, (void**)&x##_fn);

      bool InitExtension()
      {
         if (mInitExtensionsDone)
            return true;
         mExtensionsPresent = true;
         LE(glUniform1i);
         LE(glUniformMatrix3fv);
#ifdef _WIN32
         LE(glActiveTexture);
#endif
         LE(glBindFramebuffer);
         LE(glDeleteFramebuffers);
         LE(glDeleteRenderbuffers);
         LE(glFramebufferTexture2D);
         LE(glFramebufferRenderbuffer);
         LE(glRenderbufferStorage);
         LE(glGenFramebuffers);
         LE(glGenRenderbuffers);
         LE(glBindRenderbuffer);
         LE(glCheckFramebufferStatus);
         LE(glGenerateMipmap);
         LE(glBufferData);
         LE(glUseProgram);
         LE(glGetUniformLocation);
         LE(glGetAttribLocation);
         LE(glDeleteBuffers);
         LE(glDeleteVertexArrays);
         LE(glEnableVertexAttribArray);
         LE(glVertexAttribPointer);
         LE(glGenBuffers);
         LE(glBindBuffer);
         LE(glCreateShader);
         LE(glShaderSource);
         LE(glCompileShader);
         LE(glCreateProgram);
         LE(glAttachShader);
         LE(glDeleteProgram);
         LE(glDeleteShader);
         LE(glDisableVertexAttribArray);
         LE(glBindAttribLocation);
         LE(glVertexAttribDivisor);
         LE(glUniformMatrix4fv);
         LE(glGetShaderiv);
         LE(glLinkProgram);
         LE(glGetProgramiv);
         LE(glBindVertexArray);
         LE(glUniform2fv);
         LE(glUniform3f);
         LE(glUniform3fv);
         LE(glUniform4fv);
         LE(glBufferSubData);
         LE(glGenVertexArrays);
         LE(glGetShaderInfoLog);
         LE(glGetProgramInfoLog);
         LE(glGetUniformBlockIndex);
         LE(glUniformBlockBinding);
         LE(glBindBufferBase);
         LE(glTransformFeedbackVaryings);
         LE(glMapBuffer);
         LE(glUnmapBuffer);
         LE(glDrawElementsInstanced);
         LE(glDrawArraysInstanced);
         LE(glDrawElementsInstancedBaseVertex);
         LE(glBeginTransformFeedback);
         LE(glEndTransformFeedback);
         LE(glUniform1f);
         LE(glUniform2f);
         LE(glBlendEquationSeparate);
         LE(glBlendFuncSeparate);
         LE(glGetBufferSubData);
         LE(glGetShaderSource);
         LE(glIsProgram);
         LE(glGetAttachedShaders);
         LE(glDrawBuffers);
         LE(glBlitFramebuffer);
         LE(glBlendEquation);
         LE(glBindSampler);
         LE(glDetachShader);
         mInitExtensionsDone = true;
         return mExtensionsPresent;
      }

#ifdef RBLXUI_API
      static double       g_Time;
      static bool         g_MousePressed[3];
      static float        g_MouseWheel;
      static unsigned int g_FontTexture;
      static int          g_ShaderHandle, g_VertHandle, g_FragHandle;
      static int          g_AttribLocationTex, g_AttribLocationProjMtx;
      static int          g_AttribLocationPosition, g_AttribLocationUV, g_AttribLocationColor;
      static unsigned int g_VboHandle, g_VaoHandle, g_ElementsHandle;

      static bool IsAnyMouseButtonDown()
      {
         RblxUIIO& io = RblxUI::GetIO();
         for (int n = 0; n < RBLXUI_ARRAYSIZE(io.MouseDown); n++)
            if (io.MouseDown[n])
               return true;
         return false;
      }

      static void RblxUI_RenderDrawLists(RblxDrawData* draw_data)
      {
         RblxUIIO& io = RblxUI::GetIO();
         int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
         int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
         if (fb_width == 0 || fb_height == 0)
            return;
         draw_data->ScaleClipRects(io.DisplayFramebufferScale);

         unsigned int last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (int*)&last_active_texture);
         glActiveTexture(GL_TEXTURE0);
         int last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
         int last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
         int last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
         int last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
         int last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
         int last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#if !defined(__APPLE__)
         int last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
         int last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
         int last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
         unsigned int last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (int*)&last_blend_src_rgb);
         unsigned int last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (int*)&last_blend_dst_rgb);
         unsigned int last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (int*)&last_blend_src_alpha);
         unsigned int last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (int*)&last_blend_dst_alpha);
         unsigned int last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (int*)&last_blend_equation_rgb);
         unsigned int last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (int*)&last_blend_equation_alpha);
         unsigned char last_enable_blend = glIsEnabled(GL_BLEND);
         unsigned char last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
         unsigned char last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
         unsigned char last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

         glEnable(GL_BLEND);
         glBlendEquation(GL_FUNC_ADD);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         glDisable(GL_CULL_FACE);
         glDisable(GL_DEPTH_TEST);
         glEnable(GL_SCISSOR_TEST);
#if !defined(__APPLE__)
         glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

         glViewport(0, 0, (int)fb_width, (int)fb_height);
         const float ortho_projection[4][4] =
         {
             { 2.0f / io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
             { 0.0f,                  2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
             { 0.0f,                  0.0f,                  -1.0f, 0.0f },
             { -1.0f,                  1.0f,                   0.0f, 1.0f },
         };
         glUseProgram(g_ShaderHandle);
         glUniform1i(g_AttribLocationTex, 0);
         glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
         glBindVertexArray(g_VaoHandle);
         glBindSampler(0, 0);

         for (int n = 0; n < draw_data->CmdListsCount; n++)
         {
            const RblxDrawList* cmd_list = draw_data->CmdLists[n];
            const RblxDrawIdx* idx_buffer_offset = 0;
            glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
            glBufferData(GL_ARRAY_BUFFER, (ptrdiff_t)cmd_list->VtxBuffer.Size * sizeof(RblxDrawVert), (const void*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (ptrdiff_t)cmd_list->IdxBuffer.Size * sizeof(RblxDrawIdx), (const void*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
            {
               const RblxDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
               if (pcmd->UserCallback)
               {
                  pcmd->UserCallback(cmd_list, pcmd);
               }
               else
               {
                  glBindTexture(GL_TEXTURE_2D, (unsigned int)(intptr_t)pcmd->GetTexID());
                  glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                  glDrawElements(GL_TRIANGLES, (int)pcmd->ElemCount, sizeof(RblxDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
               }
               idx_buffer_offset += pcmd->ElemCount;
            }
         }

         glUseProgram(last_program);
         glBindTexture(GL_TEXTURE_2D, last_texture);
         glBindSampler(0, last_sampler);
         glActiveTexture(last_active_texture);
         glBindVertexArray(last_vertex_array);
         glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
         glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
         glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
         if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
         if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
         if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
         if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
#if !defined(__APPLE__)
         glPolygonMode(GL_FRONT_AND_BACK, last_polygon_mode[0]);
#endif
         glViewport(last_viewport[0], last_viewport[1], (int)last_viewport[2], (int)last_viewport[3]);
         glScissor(last_scissor_box[0], last_scissor_box[1], (int)last_scissor_box[2], (int)last_scissor_box[3]);
      }

      bool RblxUI_CreateFontsTexture()
      {
         RblxUIIO& io = RblxUI::GetIO();
         unsigned char* pixels;
         int width, height;
         io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

         int last_texture;
         glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
         glGenTextures(1, &g_FontTexture);
         glBindTexture(GL_TEXTURE_2D, g_FontTexture);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

         io.Fonts->TexID = (void*)(intptr_t)g_FontTexture;
         glBindTexture(GL_TEXTURE_2D, last_texture);
         return true;
      }

      bool RblxUI_CreateDeviceObjects()
      {
         int last_texture, last_array_buffer, last_vertex_array;
         glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
         glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
         glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

#ifdef __APPLE__
         const char* glslVersion = "#version 150\n";
#else
         const char* glslVersion = "#version 330\n";
#endif
         const char* vertexShaderBody =
            "uniform mat4 ProjMtx;\n"
            "in vec2 Position;\n"
            "in vec2 UV;\n"
            "in vec4 Color;\n"
            "out vec2 Frag_UV;\n"
            "out vec4 Frag_Color;\n"
            "void main()\n"
            "{\n"
            "    Frag_UV = UV;\n"
            "    Frag_Color = Color;\n"
            "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
            "}\n";

         const char* fragmentShaderBody =
            "uniform sampler2D Texture;\n"
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "out vec4 Out_Color;\n"
            "void main()\n"
            "{\n"
            "    Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
            "}\n";

         char vertexShader[1024];
         char fragmentShader[1024];
         snprintf(vertexShader, sizeof(vertexShader), "%s%s", glslVersion, vertexShaderBody);
         snprintf(fragmentShader, sizeof(fragmentShader), "%s%s", glslVersion, fragmentShaderBody);

         g_ShaderHandle = glCreateProgram();
         g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
         g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
         const char* vs = vertexShader;
         const char* fs = fragmentShader;
         glShaderSource(g_VertHandle, 1, &vs, 0);
         glShaderSource(g_FragHandle, 1, &fs, 0);
         glCompileShader(g_VertHandle);
         glCompileShader(g_FragHandle);
         glAttachShader(g_ShaderHandle, g_VertHandle);
         glAttachShader(g_ShaderHandle, g_FragHandle);
         glLinkProgram(g_ShaderHandle);

         g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
         g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
         g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
         g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
         g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

         glGenBuffers(1, &g_VboHandle);
         glGenBuffers(1, &g_ElementsHandle);
         glGenVertexArrays(1, &g_VaoHandle);
         glBindVertexArray(g_VaoHandle);
         glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
         glEnableVertexAttribArray(g_AttribLocationPosition);
         glEnableVertexAttribArray(g_AttribLocationUV);
         glEnableVertexAttribArray(g_AttribLocationColor);
#ifndef OFFSETOF
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
#endif
         glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(RblxDrawVert), (void*)OFFSETOF(RblxDrawVert, pos));
         glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(RblxDrawVert), (void*)OFFSETOF(RblxDrawVert, uv));
         glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(RblxDrawVert), (void*)OFFSETOF(RblxDrawVert, col));
#undef OFFSETOF

         RblxUI_CreateFontsTexture();

         glBindTexture(GL_TEXTURE_2D, last_texture);
         glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
         glBindVertexArray(last_vertex_array);
         return true;
      }

      void RblxUI_InvalidateDeviceObjects()
      {
         if (g_VaoHandle) glDeleteVertexArrays(1, &g_VaoHandle);
         if (g_VboHandle) glDeleteBuffers(1, &g_VboHandle);
         if (g_ElementsHandle) glDeleteBuffers(1, &g_ElementsHandle);
         g_VaoHandle = g_VboHandle = g_ElementsHandle = 0;

         if (g_ShaderHandle && g_VertHandle) glDetachShader(g_ShaderHandle, g_VertHandle);
         if (g_VertHandle) glDeleteShader(g_VertHandle);
         g_VertHandle = 0;

         if (g_ShaderHandle && g_FragHandle) glDetachShader(g_ShaderHandle, g_FragHandle);
         if (g_FragHandle) glDeleteShader(g_FragHandle);
         g_FragHandle = 0;

         if (g_ShaderHandle) glDeleteProgram(g_ShaderHandle);
         g_ShaderHandle = 0;

         if (g_FontTexture)
         {
            glDeleteTextures(1, &g_FontTexture);
            g_FontTexture = 0;
         }
      }

      void RblxUI_Shutdown()
      {
         RblxUI_InvalidateDeviceObjects();
      }
#endif // RBLXUI_API

      // =====================================================================
      // Platform-specific members and methods
      // =====================================================================
#if defined(_WIN32)
      typedef struct {
         HINSTANCE   hInstance;
         HDC         hDC;
         HGLRC       hRC;
         HWND        hWnd;
         char        wndclass[4];
      } WININFO;

      WININFO wininfo;

      static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
      void WindowEnd(WININFO* info);
      int WindowInit(WININFO* info);

#ifdef RBLXUI_API
      static RblxUIKey RblxUI_ImplWin32_KeyEventToRblxUIKey(WPARAM wParam, LPARAM lParam);
      static LRESULT RblxUI_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
      void RblxUI_NewFrame();
#endif

#elif defined(__linux__)
      Display*     mDisplay;
      Window       mWindow;
      GLXContext   mGLContext;
      Atom         mWmDeleteMessage;
      Colormap     mColormap;

#ifdef RBLXUI_API
      static RblxUIKey X11_KeySymToRblxUIKey(KeySym keysym);
      void RblxUI_NewFrame();
#endif

#elif defined(__APPLE__)
      id mNSApp;
      id mAppWindow;
      id mGLContext;
      id mWindowDelegate;

#ifdef RBLXUI_API
      static RblxUIKey MacOS_KeyToRblxUIKey(unsigned short keyCode);
      void RblxUI_NewFrame();
#endif
#endif
   }; // RblxApp

// =========================================================================
// IMPLEMENTATION
// =========================================================================
#ifdef IMAPP_IMPL
   RblxApp* RblxApp::mInstance = NULL;
#ifdef RBLXUI_API
   double       RblxApp::g_Time = 0.0f;
   bool         RblxApp::g_MousePressed[3] = { false, false, false };
   float        RblxApp::g_MouseWheel = 0.0f;
   unsigned int RblxApp::g_FontTexture = 0;
   int          RblxApp::g_ShaderHandle = 0, RblxApp::g_VertHandle = 0, RblxApp::g_FragHandle = 0;
   int          RblxApp::g_AttribLocationTex = 0, RblxApp::g_AttribLocationProjMtx = 0;
   int          RblxApp::g_AttribLocationPosition = 0, RblxApp::g_AttribLocationUV = 0, RblxApp::g_AttribLocationColor = 0;
   unsigned int RblxApp::g_VboHandle = 0, RblxApp::g_VaoHandle = 0, RblxApp::g_ElementsHandle = 0;
#endif

// =========================================================================
// WINDOWS IMPLEMENTATION
// =========================================================================
#if defined(_WIN32)

   LRESULT WINAPI RblxApp::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
   {
#ifdef RBLXUI_API
      if (RblxUI_WndProcHandler(hWnd, msg, wParam, lParam))
         return true;
#endif
      switch (msg)
      {
      case WM_SIZE:
      {
#ifdef RBLXUI_API
         RblxUIIO& io = RblxUI::GetIO();
         int w = LOWORD(lParam);
         int h = HIWORD(lParam);
         io.DisplaySize = RblxVec2((float)w, (float)h);
         io.DisplayFramebufferScale = RblxVec2(w > 0 ? 1.0f : 0.0f, h > 0 ? 1.0f : 0.0f);
#endif
      }
      return 0;
      case WM_SYSCOMMAND:
         if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
         break;
      case WM_DESTROY:
         PostQuitMessage(0);
         return 0;
      }
      return DefWindowProc(hWnd, msg, wParam, lParam);
   }

   void RblxApp::WindowEnd(WININFO* info)
   {
      if (info->hRC)
      {
         wglMakeCurrent(0, 0);
         wglDeleteContext(info->hRC);
      }
      if (info->hDC) ReleaseDC(info->hWnd, info->hDC);
      if (info->hWnd) DestroyWindow(info->hWnd);
      UnregisterClassA(info->wndclass, info->hInstance);
      if (mConfig.mFullscreen)
      {
         ChangeDisplaySettings(0, 0);
         while (ShowCursor(1) < 0);
      }
   }

   int RblxApp::WindowInit(WININFO* info)
   {
      unsigned int PixelFormat;
      DWORD dwExStyle, dwStyle;
      RECT rec;

      WNDCLASSEXA WndClsEx;
      WndClsEx.cbSize = sizeof(WNDCLASSEX);
      WndClsEx.style = CS_HREDRAW | CS_VREDRAW;
      WndClsEx.lpfnWndProc = WndProc;
      WndClsEx.cbClsExtra = 0;
      WndClsEx.cbWndExtra = 0;
      WndClsEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
      WndClsEx.hCursor = LoadCursor(NULL, IDC_ARROW);
      WndClsEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
      WndClsEx.lpszMenuName = NULL;
      WndClsEx.lpszClassName = "RblxGizmoWndClass";
      WndClsEx.hInstance = GetModuleHandle(NULL);
      WndClsEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

      if (!RegisterClassExA(&WndClsEx))
         return 0;

      if (mConfig.mFullscreen)
      {
         DEVMODE dmScreenSettings;
         dmScreenSettings.dmSize = sizeof(DEVMODE);
         dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
         dmScreenSettings.dmBitsPerPel = 32;
         dmScreenSettings.dmPelsWidth = mConfig.mWidth;
         dmScreenSettings.dmPelsHeight = mConfig.mHeight;
         if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            return 0;
         dwExStyle = WS_EX_APPWINDOW;
         dwStyle = WS_VISIBLE | WS_POPUP;
      }
      else
      {
         dwExStyle = 0;
         dwStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_POPUP;
      }

      rec.left = 0;
      rec.top = 0;
      rec.right = mConfig.mWidth;
      rec.bottom = mConfig.mHeight;
      AdjustWindowRect(&rec, dwStyle, 0);

      info->hWnd = CreateWindowExA(dwExStyle, "RblxGizmoWndClass", " ", dwStyle | WS_MAXIMIZE,
         (GetSystemMetrics(SM_CXSCREEN) - rec.right + rec.left) >> 1,
         (GetSystemMetrics(SM_CYSCREEN) - rec.bottom + rec.top) >> 1,
         rec.right - rec.left, rec.bottom - rec.top,
         0, 0, info->hInstance, 0);

      if (!info->hWnd) return 0;
      if (!(info->hDC = GetDC(info->hWnd))) return 0;

      static PIXELFORMATDESCRIPTOR pfd =
      {
          sizeof(PIXELFORMATDESCRIPTOR), 1,
          PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
          PFD_TYPE_RGBA, 32,
          0, 0, 0, 0, 0, 0, 8, 0,
          0, 0, 0, 0, 0, 32, 0, 0,
          PFD_MAIN_PLANE, 0, 0, 0, 0
      };

      if (!(PixelFormat = ChoosePixelFormat(info->hDC, &pfd))) return 0;
      if (!SetPixelFormat(info->hDC, PixelFormat, &pfd)) return 0;
      if (!(info->hRC = wglCreateContext(info->hDC))) return 0;
      if (!wglMakeCurrent(info->hDC, info->hRC)) return 0;
      return 1;
   }

   int RblxApp::Init(const Config& config)
   {
      mConfig = config;
      wininfo = WININFO{ 0,0,0,0, { 'c','X','d',0 } };
      WININFO* info = &wininfo;
      info->hInstance = GetModuleHandle(0);

#ifdef RBLXUI_API
      RblxUI::CreateContext();
#endif
      if (!WindowInit(info))
      {
         WindowEnd(info);
         return 0;
      }
      if (!InitExtension())
         return 0;
      mDone = false;
      return 1;
   }

   void RblxApp::NewFrame()
   {
      MSG msg;
      while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
      {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
         if (msg.message == WM_QUIT)
            mDone = true;
      }
#ifdef RBLXUI_API
      RblxUI_NewFrame();
#endif
   }

   void RblxApp::EndFrame()
   {
#ifdef RBLXUI_API
      RblxUI::Render();
      RblxUI_RenderDrawLists(RblxUI::GetDrawData());
#endif
      SwapBuffers(wininfo.hDC);
   }

   void RblxApp::Finish()
   {
#ifdef RBLXUI_API
      RblxUI_Shutdown();
#endif
   }

#ifdef RBLXUI_API
   LRESULT RblxApp::RblxUI_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
   {
      RblxUIIO& io = RblxUI::GetIO();
      switch (msg)
      {
      case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN:
      {
         int button = 0;
         if (msg == WM_RBUTTONDOWN) button = 1;
         if (msg == WM_MBUTTONDOWN) button = 2;
         if (!IsAnyMouseButtonDown() && GetCapture() == NULL)
            SetCapture(hwnd);
         io.MouseDown[button] = true;
         return 0;
      }
      case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP:
      {
         int button = 0;
         if (msg == WM_RBUTTONUP) button = 1;
         if (msg == WM_MBUTTONUP) button = 2;
         io.MouseDown[button] = false;
         if (!IsAnyMouseButtonDown() && GetCapture() == hwnd)
            ReleaseCapture();
         return 0;
      }
      case WM_MOUSEWHEEL:
         io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
         return 0;
      case WM_MOUSEMOVE:
         io.MousePos.x = (signed short)(lParam);
         io.MousePos.y = (signed short)(lParam >> 16);
         return 0;
      case WM_KEYDOWN: case WM_SYSKEYDOWN:
         if (wParam < 256)
         {
            const RblxUIKey key = RblxUI_ImplWin32_KeyEventToRblxUIKey(wParam, lParam);
            io.AddKeyEvent(key, true);
         }
         return 0;
      case WM_KEYUP: case WM_SYSKEYUP:
         if (wParam < 256)
         {
            const RblxUIKey key = RblxUI_ImplWin32_KeyEventToRblxUIKey(wParam, lParam);
            io.AddKeyEvent(key, false);
         }
         return 0;
      case WM_CHAR:
         if (wParam > 0 && wParam < 0x10000)
            io.AddInputCharacter((unsigned short)wParam);
         return 0;
      }
      return 0;
   }

   RblxUIKey RblxApp::RblxUI_ImplWin32_KeyEventToRblxUIKey(WPARAM wParam, LPARAM lParam)
   {
      if ((wParam == VK_RETURN) && (HIWORD(lParam) & KF_EXTENDED))
         return RblxUIKey_KeypadEnter;
      switch (wParam)
      {
      case VK_TAB: return RblxUIKey_Tab;
      case VK_LEFT: return RblxUIKey_LeftArrow;
      case VK_RIGHT: return RblxUIKey_RightArrow;
      case VK_UP: return RblxUIKey_UpArrow;
      case VK_DOWN: return RblxUIKey_DownArrow;
      case VK_PRIOR: return RblxUIKey_PageUp;
      case VK_NEXT: return RblxUIKey_PageDown;
      case VK_HOME: return RblxUIKey_Home;
      case VK_END: return RblxUIKey_End;
      case VK_INSERT: return RblxUIKey_Insert;
      case VK_DELETE: return RblxUIKey_Delete;
      case VK_BACK: return RblxUIKey_Backspace;
      case VK_SPACE: return RblxUIKey_Space;
      case VK_RETURN: return RblxUIKey_Enter;
      case VK_ESCAPE: return RblxUIKey_Escape;
      case VK_OEM_COMMA: return RblxUIKey_Comma;
      case VK_OEM_PERIOD: return RblxUIKey_Period;
      case VK_CAPITAL: return RblxUIKey_CapsLock;
      case VK_SCROLL: return RblxUIKey_ScrollLock;
      case VK_NUMLOCK: return RblxUIKey_NumLock;
      case VK_SNAPSHOT: return RblxUIKey_PrintScreen;
      case VK_PAUSE: return RblxUIKey_Pause;
      case VK_NUMPAD0: return RblxUIKey_Keypad0;
      case VK_NUMPAD1: return RblxUIKey_Keypad1;
      case VK_NUMPAD2: return RblxUIKey_Keypad2;
      case VK_NUMPAD3: return RblxUIKey_Keypad3;
      case VK_NUMPAD4: return RblxUIKey_Keypad4;
      case VK_NUMPAD5: return RblxUIKey_Keypad5;
      case VK_NUMPAD6: return RblxUIKey_Keypad6;
      case VK_NUMPAD7: return RblxUIKey_Keypad7;
      case VK_NUMPAD8: return RblxUIKey_Keypad8;
      case VK_NUMPAD9: return RblxUIKey_Keypad9;
      case VK_DECIMAL: return RblxUIKey_KeypadDecimal;
      case VK_DIVIDE: return RblxUIKey_KeypadDivide;
      case VK_MULTIPLY: return RblxUIKey_KeypadMultiply;
      case VK_SUBTRACT: return RblxUIKey_KeypadSubtract;
      case VK_ADD: return RblxUIKey_KeypadAdd;
      case VK_LSHIFT: return RblxUIKey_LeftShift;
      case VK_LCONTROL: return RblxUIKey_LeftCtrl;
      case VK_LMENU: return RblxUIKey_LeftAlt;
      case VK_LWIN: return RblxUIKey_LeftSuper;
      case VK_RSHIFT: return RblxUIKey_RightShift;
      case VK_RCONTROL: return RblxUIKey_RightCtrl;
      case VK_RMENU: return RblxUIKey_RightAlt;
      case VK_RWIN: return RblxUIKey_RightSuper;
      case VK_APPS: return RblxUIKey_Menu;
      case '0': return RblxUIKey_0;
      case '1': return RblxUIKey_1;
      case '2': return RblxUIKey_2;
      case '3': return RblxUIKey_3;
      case '4': return RblxUIKey_4;
      case '5': return RblxUIKey_5;
      case '6': return RblxUIKey_6;
      case '7': return RblxUIKey_7;
      case '8': return RblxUIKey_8;
      case '9': return RblxUIKey_9;
      case 'A': return RblxUIKey_A;
      case 'B': return RblxUIKey_B;
      case 'C': return RblxUIKey_C;
      case 'D': return RblxUIKey_D;
      case 'E': return RblxUIKey_E;
      case 'F': return RblxUIKey_F;
      case 'G': return RblxUIKey_G;
      case 'H': return RblxUIKey_H;
      case 'I': return RblxUIKey_I;
      case 'J': return RblxUIKey_J;
      case 'K': return RblxUIKey_K;
      case 'L': return RblxUIKey_L;
      case 'M': return RblxUIKey_M;
      case 'N': return RblxUIKey_N;
      case 'O': return RblxUIKey_O;
      case 'P': return RblxUIKey_P;
      case 'Q': return RblxUIKey_Q;
      case 'R': return RblxUIKey_R;
      case 'S': return RblxUIKey_S;
      case 'T': return RblxUIKey_T;
      case 'U': return RblxUIKey_U;
      case 'V': return RblxUIKey_V;
      case 'W': return RblxUIKey_W;
      case 'X': return RblxUIKey_X;
      case 'Y': return RblxUIKey_Y;
      case 'Z': return RblxUIKey_Z;
      case VK_F1: return RblxUIKey_F1;
      case VK_F2: return RblxUIKey_F2;
      case VK_F3: return RblxUIKey_F3;
      case VK_F4: return RblxUIKey_F4;
      case VK_F5: return RblxUIKey_F5;
      case VK_F6: return RblxUIKey_F6;
      case VK_F7: return RblxUIKey_F7;
      case VK_F8: return RblxUIKey_F8;
      case VK_F9: return RblxUIKey_F9;
      case VK_F10: return RblxUIKey_F10;
      case VK_F11: return RblxUIKey_F11;
      case VK_F12: return RblxUIKey_F12;
      default: break;
      }
      return RblxUIKey_None;
   }

   void RblxApp::RblxUI_NewFrame()
   {
      if (!g_FontTexture)
         RblxUI_CreateDeviceObjects();
      RblxUIIO& io = RblxUI::GetIO();
      io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
      io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
      io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
      io.KeySuper = false;
      RblxUI::NewFrame();
   }
#endif // RBLXUI_API

// =========================================================================
// LINUX / X11 IMPLEMENTATION
// =========================================================================
#elif defined(__linux__)

#ifdef RBLXUI_API
   RblxUIKey RblxApp::X11_KeySymToRblxUIKey(KeySym keysym)
   {
      switch (keysym)
      {
      case XK_Tab: return RblxUIKey_Tab;
      case XK_Left: return RblxUIKey_LeftArrow;
      case XK_Right: return RblxUIKey_RightArrow;
      case XK_Up: return RblxUIKey_UpArrow;
      case XK_Down: return RblxUIKey_DownArrow;
      case XK_Page_Up: return RblxUIKey_PageUp;
      case XK_Page_Down: return RblxUIKey_PageDown;
      case XK_Home: return RblxUIKey_Home;
      case XK_End: return RblxUIKey_End;
      case XK_Insert: return RblxUIKey_Insert;
      case XK_Delete: return RblxUIKey_Delete;
      case XK_BackSpace: return RblxUIKey_Backspace;
      case XK_space: return RblxUIKey_Space;
      case XK_Return: return RblxUIKey_Enter;
      case XK_Escape: return RblxUIKey_Escape;
      case XK_comma: return RblxUIKey_Comma;
      case XK_period: return RblxUIKey_Period;
      case XK_minus: return RblxUIKey_Minus;
      case XK_equal: return RblxUIKey_Equal;
      case XK_semicolon: return RblxUIKey_Semicolon;
      case XK_slash: return RblxUIKey_Slash;
      case XK_grave: return RblxUIKey_GraveAccent;
      case XK_bracketleft: return RblxUIKey_LeftBracket;
      case XK_backslash: return RblxUIKey_Backslash;
      case XK_bracketright: return RblxUIKey_RightBracket;
      case XK_apostrophe: return RblxUIKey_Apostrophe;
      case XK_Caps_Lock: return RblxUIKey_CapsLock;
      case XK_Scroll_Lock: return RblxUIKey_ScrollLock;
      case XK_Num_Lock: return RblxUIKey_NumLock;
      case XK_Print: return RblxUIKey_PrintScreen;
      case XK_Pause: return RblxUIKey_Pause;
      case XK_KP_0: return RblxUIKey_Keypad0;
      case XK_KP_1: return RblxUIKey_Keypad1;
      case XK_KP_2: return RblxUIKey_Keypad2;
      case XK_KP_3: return RblxUIKey_Keypad3;
      case XK_KP_4: return RblxUIKey_Keypad4;
      case XK_KP_5: return RblxUIKey_Keypad5;
      case XK_KP_6: return RblxUIKey_Keypad6;
      case XK_KP_7: return RblxUIKey_Keypad7;
      case XK_KP_8: return RblxUIKey_Keypad8;
      case XK_KP_9: return RblxUIKey_Keypad9;
      case XK_KP_Decimal: return RblxUIKey_KeypadDecimal;
      case XK_KP_Divide: return RblxUIKey_KeypadDivide;
      case XK_KP_Multiply: return RblxUIKey_KeypadMultiply;
      case XK_KP_Subtract: return RblxUIKey_KeypadSubtract;
      case XK_KP_Add: return RblxUIKey_KeypadAdd;
      case XK_KP_Enter: return RblxUIKey_KeypadEnter;
      case XK_Shift_L: return RblxUIKey_LeftShift;
      case XK_Control_L: return RblxUIKey_LeftCtrl;
      case XK_Alt_L: return RblxUIKey_LeftAlt;
      case XK_Super_L: return RblxUIKey_LeftSuper;
      case XK_Shift_R: return RblxUIKey_RightShift;
      case XK_Control_R: return RblxUIKey_RightCtrl;
      case XK_Alt_R: return RblxUIKey_RightAlt;
      case XK_Super_R: return RblxUIKey_RightSuper;
      case XK_Menu: return RblxUIKey_Menu;
      case XK_0: return RblxUIKey_0;
      case XK_1: return RblxUIKey_1;
      case XK_2: return RblxUIKey_2;
      case XK_3: return RblxUIKey_3;
      case XK_4: return RblxUIKey_4;
      case XK_5: return RblxUIKey_5;
      case XK_6: return RblxUIKey_6;
      case XK_7: return RblxUIKey_7;
      case XK_8: return RblxUIKey_8;
      case XK_9: return RblxUIKey_9;
      case XK_a: case XK_A: return RblxUIKey_A;
      case XK_b: case XK_B: return RblxUIKey_B;
      case XK_c: case XK_C: return RblxUIKey_C;
      case XK_d: case XK_D: return RblxUIKey_D;
      case XK_e: case XK_E: return RblxUIKey_E;
      case XK_f: case XK_F: return RblxUIKey_F;
      case XK_g: case XK_G: return RblxUIKey_G;
      case XK_h: case XK_H: return RblxUIKey_H;
      case XK_i: case XK_I: return RblxUIKey_I;
      case XK_j: case XK_J: return RblxUIKey_J;
      case XK_k: case XK_K: return RblxUIKey_K;
      case XK_l: case XK_L: return RblxUIKey_L;
      case XK_m: case XK_M: return RblxUIKey_M;
      case XK_n: case XK_N: return RblxUIKey_N;
      case XK_o: case XK_O: return RblxUIKey_O;
      case XK_p: case XK_P: return RblxUIKey_P;
      case XK_q: case XK_Q: return RblxUIKey_Q;
      case XK_r: case XK_R: return RblxUIKey_R;
      case XK_s: case XK_S: return RblxUIKey_S;
      case XK_t: case XK_T: return RblxUIKey_T;
      case XK_u: case XK_U: return RblxUIKey_U;
      case XK_v: case XK_V: return RblxUIKey_V;
      case XK_w: case XK_W: return RblxUIKey_W;
      case XK_x: case XK_X: return RblxUIKey_X;
      case XK_y: case XK_Y: return RblxUIKey_Y;
      case XK_z: case XK_Z: return RblxUIKey_Z;
      case XK_F1: return RblxUIKey_F1;
      case XK_F2: return RblxUIKey_F2;
      case XK_F3: return RblxUIKey_F3;
      case XK_F4: return RblxUIKey_F4;
      case XK_F5: return RblxUIKey_F5;
      case XK_F6: return RblxUIKey_F6;
      case XK_F7: return RblxUIKey_F7;
      case XK_F8: return RblxUIKey_F8;
      case XK_F9: return RblxUIKey_F9;
      case XK_F10: return RblxUIKey_F10;
      case XK_F11: return RblxUIKey_F11;
      case XK_F12: return RblxUIKey_F12;
      default: return RblxUIKey_None;
      }
   }
#endif

   int RblxApp::Init(const Config& config)
   {
      mConfig = config;
      mDisplay = XOpenDisplay(NULL);
      if (!mDisplay)
         return 0;

      int screen = DefaultScreen(mDisplay);

      int attribs[] = {
         GLX_RGBA,
         GLX_DOUBLEBUFFER,
         GLX_DEPTH_SIZE, 24,
         GLX_RED_SIZE, 8,
         GLX_GREEN_SIZE, 8,
         GLX_BLUE_SIZE, 8,
         GLX_ALPHA_SIZE, 8,
         None
      };

      XVisualInfo* vi = glXChooseVisual(mDisplay, screen, attribs);
      if (!vi)
         return 0;

      mColormap = XCreateColormap(mDisplay, RootWindow(mDisplay, screen), vi->visual, AllocNone);

      XSetWindowAttributes swa;
      swa.colormap = mColormap;
      swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
         ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
         StructureNotifyMask | FocusChangeMask;

      mWindow = XCreateWindow(mDisplay, RootWindow(mDisplay, screen),
         0, 0, config.mWidth, config.mHeight, 0,
         vi->depth, InputOutput, vi->visual,
         CWColormap | CWEventMask, &swa);

      XStoreName(mDisplay, mWindow, "RblxGizmo");
      XMapWindow(mDisplay, mWindow);

      mWmDeleteMessage = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
      XSetWMProtocols(mDisplay, mWindow, &mWmDeleteMessage, 1);

      mGLContext = glXCreateContext(mDisplay, vi, NULL, GL_TRUE);
      XFree(vi);

      if (!mGLContext)
         return 0;

      glXMakeCurrent(mDisplay, mWindow, mGLContext);

      if (!InitExtension())
         return 0;

#ifdef RBLXUI_API
      RblxUI::CreateContext();
      RblxUIIO& io = RblxUI::GetIO();
      io.DisplaySize = RblxVec2((float)config.mWidth, (float)config.mHeight);
      io.DisplayFramebufferScale = RblxVec2(1.0f, 1.0f);
#endif

      mDone = false;
      return 1;
   }

   void RblxApp::NewFrame()
   {
#ifdef RBLXUI_API
      RblxUIIO& io = RblxUI::GetIO();
#endif
      while (XPending(mDisplay))
      {
         XEvent event;
         XNextEvent(mDisplay, &event);

         switch (event.type)
         {
         case ConfigureNotify:
#ifdef RBLXUI_API
            io.DisplaySize = RblxVec2((float)event.xconfigure.width, (float)event.xconfigure.height);
#endif
            break;

         case ClientMessage:
            if ((Atom)event.xclient.data.l[0] == mWmDeleteMessage)
               mDone = true;
            break;

#ifdef RBLXUI_API
         case KeyPress:
         case KeyRelease:
         {
            KeySym keysym = XLookupKeysym(&event.xkey, 0);
            RblxUIKey key = X11_KeySymToRblxUIKey(keysym);
            io.AddKeyEvent(key, event.type == KeyPress);

            io.KeyCtrl = (event.xkey.state & ControlMask) != 0;
            io.KeyShift = (event.xkey.state & ShiftMask) != 0;
            io.KeyAlt = (event.xkey.state & Mod1Mask) != 0;
            io.KeySuper = (event.xkey.state & Mod4Mask) != 0;

            if (event.type == KeyPress)
            {
               char buf[32];
               int len = XLookupString(&event.xkey, buf, sizeof(buf), NULL, NULL);
               for (int i = 0; i < len; i++)
               {
                  if ((unsigned char)buf[i] >= 32)
                     io.AddInputCharacter((unsigned char)buf[i]);
               }
            }
            break;
         }
         case ButtonPress:
         case ButtonRelease:
         {
            bool down = (event.type == ButtonPress);
            if (event.xbutton.button == Button1) io.MouseDown[0] = down;
            else if (event.xbutton.button == Button2) io.MouseDown[2] = down;
            else if (event.xbutton.button == Button3) io.MouseDown[1] = down;
            else if (down && event.xbutton.button == Button4) io.MouseWheel += 1.0f;
            else if (down && event.xbutton.button == Button5) io.MouseWheel -= 1.0f;
            break;
         }
         case MotionNotify:
            io.MousePos = RblxVec2((float)event.xmotion.x, (float)event.xmotion.y);
            break;
#endif
         }
      }

#ifdef RBLXUI_API
      RblxUI_NewFrame();
#endif
   }

   void RblxApp::EndFrame()
   {
#ifdef RBLXUI_API
      RblxUI::Render();
      RblxUI_RenderDrawLists(RblxUI::GetDrawData());
#endif
      glXSwapBuffers(mDisplay, mWindow);
   }

   void RblxApp::Finish()
   {
#ifdef RBLXUI_API
      RblxUI_Shutdown();
#endif
      glXMakeCurrent(mDisplay, None, NULL);
      glXDestroyContext(mDisplay, mGLContext);
      XDestroyWindow(mDisplay, mWindow);
      XFreeColormap(mDisplay, mColormap);
      XCloseDisplay(mDisplay);
   }

#ifdef RBLXUI_API
   void RblxApp::RblxUI_NewFrame()
   {
      if (!g_FontTexture)
         RblxUI_CreateDeviceObjects();
      RblxUI::NewFrame();
   }
#endif

// =========================================================================
// macOS / COCOA IMPLEMENTATION (via Objective-C runtime)
// =========================================================================
#elif defined(__APPLE__)

   // Objective-C runtime helpers
   static id objc_cls(const char* name) { return (id)objc_getClass(name); }
   static SEL objc_sel(const char* name) { return sel_registerName(name); }

   // Typed objc_msgSend wrappers
   typedef id   (*msg_id)(id, SEL);
   typedef id   (*msg_id_id)(id, SEL, id);
   typedef id   (*msg_id_id_id)(id, SEL, id, id);
   typedef id   (*msg_id_bool)(id, SEL, BOOL);
   typedef id   (*msg_id_int)(id, SEL, NSInteger);
   typedef id   (*msg_id_uint)(id, SEL, NSUInteger);
   typedef void (*msg_void)(id, SEL);
   typedef void (*msg_void_id)(id, SEL, id);
   typedef void (*msg_void_bool)(id, SEL, BOOL);
   typedef BOOL (*msg_bool)(id, SEL);
   typedef NSInteger (*msg_nsint)(id, SEL);

   struct NSRect { double x, y, w, h; };
   struct NSPoint { double x, y; };
   struct NSSize { double w, h; };

   typedef id (*msg_id_nsrect_uint_uint_bool)(id, SEL, NSRect, NSUInteger, NSUInteger, BOOL);
   typedef id (*msg_id_pattr)(id, SEL, const uint32_t*);
   typedef id (*msg_id_nsrect)(id, SEL, NSRect);
   typedef void (*msg_void_nsrect)(id, SEL, NSRect);
   typedef NSRect (*msg_nsrect_from_id)(id, SEL);
   typedef double (*msg_double)(id, SEL);
   typedef unsigned short (*msg_ushort)(id, SEL);
   typedef NSUInteger (*msg_nsuint)(id, SEL);
   typedef NSPoint (*msg_nspoint)(id, SEL);

   // NSEvent type constants
   enum {
      NSEventTypeLeftMouseDown    = 1,
      NSEventTypeLeftMouseUp      = 2,
      NSEventTypeRightMouseDown   = 3,
      NSEventTypeRightMouseUp     = 4,
      NSEventTypeMouseMoved       = 5,
      NSEventTypeLeftMouseDragged = 6,
      NSEventTypeRightMouseDragged= 7,
      NSEventTypeKeyDown          = 10,
      NSEventTypeKeyUp            = 11,
      NSEventTypeFlagsChanged     = 12,
      NSEventTypeScrollWheel      = 22,
      NSEventTypeOtherMouseDown   = 25,
      NSEventTypeOtherMouseUp     = 26,
      NSEventTypeOtherMouseDragged= 27,
   };

   // NSWindow style mask
   enum {
      NSWindowStyleMaskTitled         = 1,
      NSWindowStyleMaskClosable       = 2,
      NSWindowStyleMaskMiniaturizable = 4,
      NSWindowStyleMaskResizable      = 8,
   };

   // NSOpenGL pixel format attributes
   enum {
      NSOpenGLPFADoubleBuffer         = 5,
      NSOpenGLPFAColorSize            = 8,
      NSOpenGLPFAAlphaSize            = 11,
      NSOpenGLPFADepthSize            = 12,
      NSOpenGLPFAOpenGLProfile        = 99,
      NSOpenGLProfileVersion3_2Core   = 0x3200,
   };

   // NSEvent modifier flags
   enum {
      NSEventModifierFlagShift    = 1 << 17,
      NSEventModifierFlagControl  = 1 << 18,
      NSEventModifierFlagOption   = 1 << 19,
      NSEventModifierFlagCommand  = 1 << 20,
   };

   static BOOL RblxApp_windowShouldClose(id self, SEL, id)
   {
      RblxApp* app = RblxApp::Instance();
      if (app)
      {
         // Access mDone through the instance - we set it via a helper
         // Since mDone is protected, we rely on the public Done() / setting mDone
         // We'll use a simple global flag
      }
      return YES;
   }

   static bool s_macOSDone = false;

   static BOOL RblxApp_windowShouldClose_impl(id, SEL, id)
   {
      s_macOSDone = true;
      return YES;
   }

#ifdef RBLXUI_API
   RblxUIKey RblxApp::MacOS_KeyToRblxUIKey(unsigned short keyCode)
   {
      switch (keyCode)
      {
      case kVK_Tab: return RblxUIKey_Tab;
      case kVK_LeftArrow: return RblxUIKey_LeftArrow;
      case kVK_RightArrow: return RblxUIKey_RightArrow;
      case kVK_UpArrow: return RblxUIKey_UpArrow;
      case kVK_DownArrow: return RblxUIKey_DownArrow;
      case kVK_PageUp: return RblxUIKey_PageUp;
      case kVK_PageDown: return RblxUIKey_PageDown;
      case kVK_Home: return RblxUIKey_Home;
      case kVK_End: return RblxUIKey_End;
      case kVK_ForwardDelete: return RblxUIKey_Delete;
      case kVK_Delete: return RblxUIKey_Backspace;
      case kVK_Space: return RblxUIKey_Space;
      case kVK_Return: return RblxUIKey_Enter;
      case kVK_Escape: return RblxUIKey_Escape;
      case kVK_ANSI_Comma: return RblxUIKey_Comma;
      case kVK_ANSI_Period: return RblxUIKey_Period;
      case kVK_ANSI_Minus: return RblxUIKey_Minus;
      case kVK_ANSI_Equal: return RblxUIKey_Equal;
      case kVK_ANSI_Semicolon: return RblxUIKey_Semicolon;
      case kVK_ANSI_Slash: return RblxUIKey_Slash;
      case kVK_ANSI_Grave: return RblxUIKey_GraveAccent;
      case kVK_ANSI_LeftBracket: return RblxUIKey_LeftBracket;
      case kVK_ANSI_Backslash: return RblxUIKey_Backslash;
      case kVK_ANSI_RightBracket: return RblxUIKey_RightBracket;
      case kVK_ANSI_Quote: return RblxUIKey_Apostrophe;
      case kVK_CapsLock: return RblxUIKey_CapsLock;
      case kVK_ANSI_Keypad0: return RblxUIKey_Keypad0;
      case kVK_ANSI_Keypad1: return RblxUIKey_Keypad1;
      case kVK_ANSI_Keypad2: return RblxUIKey_Keypad2;
      case kVK_ANSI_Keypad3: return RblxUIKey_Keypad3;
      case kVK_ANSI_Keypad4: return RblxUIKey_Keypad4;
      case kVK_ANSI_Keypad5: return RblxUIKey_Keypad5;
      case kVK_ANSI_Keypad6: return RblxUIKey_Keypad6;
      case kVK_ANSI_Keypad7: return RblxUIKey_Keypad7;
      case kVK_ANSI_Keypad8: return RblxUIKey_Keypad8;
      case kVK_ANSI_Keypad9: return RblxUIKey_Keypad9;
      case kVK_ANSI_KeypadDecimal: return RblxUIKey_KeypadDecimal;
      case kVK_ANSI_KeypadDivide: return RblxUIKey_KeypadDivide;
      case kVK_ANSI_KeypadMultiply: return RblxUIKey_KeypadMultiply;
      case kVK_ANSI_KeypadMinus: return RblxUIKey_KeypadSubtract;
      case kVK_ANSI_KeypadPlus: return RblxUIKey_KeypadAdd;
      case kVK_ANSI_KeypadEnter: return RblxUIKey_KeypadEnter;
      case kVK_Shift: return RblxUIKey_LeftShift;
      case kVK_Control: return RblxUIKey_LeftCtrl;
      case kVK_Option: return RblxUIKey_LeftAlt;
      case kVK_Command: return RblxUIKey_LeftSuper;
      case kVK_RightShift: return RblxUIKey_RightShift;
      case kVK_RightControl: return RblxUIKey_RightCtrl;
      case kVK_RightOption: return RblxUIKey_RightAlt;
      case kVK_RightCommand: return RblxUIKey_RightSuper;
      case kVK_ANSI_0: return RblxUIKey_0;
      case kVK_ANSI_1: return RblxUIKey_1;
      case kVK_ANSI_2: return RblxUIKey_2;
      case kVK_ANSI_3: return RblxUIKey_3;
      case kVK_ANSI_4: return RblxUIKey_4;
      case kVK_ANSI_5: return RblxUIKey_5;
      case kVK_ANSI_6: return RblxUIKey_6;
      case kVK_ANSI_7: return RblxUIKey_7;
      case kVK_ANSI_8: return RblxUIKey_8;
      case kVK_ANSI_9: return RblxUIKey_9;
      case kVK_ANSI_A: return RblxUIKey_A;
      case kVK_ANSI_B: return RblxUIKey_B;
      case kVK_ANSI_C: return RblxUIKey_C;
      case kVK_ANSI_D: return RblxUIKey_D;
      case kVK_ANSI_E: return RblxUIKey_E;
      case kVK_ANSI_F: return RblxUIKey_F;
      case kVK_ANSI_G: return RblxUIKey_G;
      case kVK_ANSI_H: return RblxUIKey_H;
      case kVK_ANSI_I: return RblxUIKey_I;
      case kVK_ANSI_J: return RblxUIKey_J;
      case kVK_ANSI_K: return RblxUIKey_K;
      case kVK_ANSI_L: return RblxUIKey_L;
      case kVK_ANSI_M: return RblxUIKey_M;
      case kVK_ANSI_N: return RblxUIKey_N;
      case kVK_ANSI_O: return RblxUIKey_O;
      case kVK_ANSI_P: return RblxUIKey_P;
      case kVK_ANSI_Q: return RblxUIKey_Q;
      case kVK_ANSI_R: return RblxUIKey_R;
      case kVK_ANSI_S: return RblxUIKey_S;
      case kVK_ANSI_T: return RblxUIKey_T;
      case kVK_ANSI_U: return RblxUIKey_U;
      case kVK_ANSI_V: return RblxUIKey_V;
      case kVK_ANSI_W: return RblxUIKey_W;
      case kVK_ANSI_X: return RblxUIKey_X;
      case kVK_ANSI_Y: return RblxUIKey_Y;
      case kVK_ANSI_Z: return RblxUIKey_Z;
      case kVK_F1: return RblxUIKey_F1;
      case kVK_F2: return RblxUIKey_F2;
      case kVK_F3: return RblxUIKey_F3;
      case kVK_F4: return RblxUIKey_F4;
      case kVK_F5: return RblxUIKey_F5;
      case kVK_F6: return RblxUIKey_F6;
      case kVK_F7: return RblxUIKey_F7;
      case kVK_F8: return RblxUIKey_F8;
      case kVK_F9: return RblxUIKey_F9;
      case kVK_F10: return RblxUIKey_F10;
      case kVK_F11: return RblxUIKey_F11;
      case kVK_F12: return RblxUIKey_F12;
      default: return RblxUIKey_None;
      }
   }
#endif

   int RblxApp::Init(const Config& config)
   {
      mConfig = config;
      s_macOSDone = false;

      // [NSApplication sharedApplication]
      mNSApp = ((msg_id)objc_msgSend)(objc_cls("NSApplication"), objc_sel("sharedApplication"));

      // [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular]
      ((msg_id_int)objc_msgSend)(mNSApp, objc_sel("setActivationPolicy:"), 0);

      // Create window delegate class
      Class delegateClass = objc_allocateClassPair((Class)objc_cls("NSObject"), "RblxAppDelegate", 0);
      class_addMethod(delegateClass, objc_sel("windowShouldClose:"),
         (IMP)RblxApp_windowShouldClose_impl, "B@:@");
      objc_registerClassPair(delegateClass);

      mWindowDelegate = ((msg_id)objc_msgSend)((id)delegateClass, objc_sel("alloc"));
      mWindowDelegate = ((msg_id)objc_msgSend)(mWindowDelegate, objc_sel("init"));

      // Create NSWindow
      NSRect contentRect = { 0, 0, (double)config.mWidth, (double)config.mHeight };
      NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                             NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;

      mAppWindow = ((msg_id)objc_msgSend)(objc_cls("NSWindow"), objc_sel("alloc"));
      mAppWindow = ((msg_id_nsrect_uint_uint_bool)objc_msgSend)(mAppWindow,
         objc_sel("initWithContentRect:styleMask:backing:defer:"),
         contentRect, styleMask, 2 /* NSBackingStoreBuffered */, NO);

      // [window setDelegate:delegate]
      ((msg_void_id)objc_msgSend)(mAppWindow, objc_sel("setDelegate:"), mWindowDelegate);

      // [window setTitle:@"RblxGizmo"]
      id title = ((msg_id_id)objc_msgSend)(objc_cls("NSString"),
         objc_sel("stringWithUTF8String:"), (id)"RblxGizmo");
      ((msg_void_id)objc_msgSend)(mAppWindow, objc_sel("setTitle:"), title);

      // Create NSOpenGLPixelFormat
      uint32_t pixelFormatAttrs[] = {
         NSOpenGLPFADoubleBuffer,
         NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
         NSOpenGLPFAColorSize, 24,
         NSOpenGLPFAAlphaSize, 8,
         NSOpenGLPFADepthSize, 24,
         0
      };

      id pixelFormat = ((msg_id)objc_msgSend)(objc_cls("NSOpenGLPixelFormat"), objc_sel("alloc"));
      pixelFormat = ((msg_id_pattr)objc_msgSend)(pixelFormat,
         objc_sel("initWithAttributes:"), pixelFormatAttrs);

      if (!pixelFormat)
         return 0;

      // Create NSOpenGLContext
      mGLContext = ((msg_id)objc_msgSend)(objc_cls("NSOpenGLContext"), objc_sel("alloc"));
      mGLContext = ((msg_id_id_id)objc_msgSend)(mGLContext,
         objc_sel("initWithFormat:shareContext:"), pixelFormat, (id)nil);

      ((msg_void)objc_msgSend)(pixelFormat, objc_sel("release"));

      if (!mGLContext)
         return 0;

      // Get content view and set GL context's view
      id contentView = ((msg_id)objc_msgSend)(mAppWindow, objc_sel("contentView"));
      ((msg_void_id)objc_msgSend)(mGLContext, objc_sel("setView:"), contentView);

      // [window setAcceptsMouseMovedEvents:YES]
      ((msg_void_bool)objc_msgSend)(mAppWindow, objc_sel("setAcceptsMouseMovedEvents:"), YES);

      // [context makeCurrentContext]
      ((msg_void)objc_msgSend)(mGLContext, objc_sel("makeCurrentContext"));

      // Show window
      ((msg_void_id)objc_msgSend)(mAppWindow, objc_sel("makeKeyAndOrderFront:"), (id)nil);
      ((msg_void_bool)objc_msgSend)(mNSApp, objc_sel("activateIgnoringOtherApps:"), YES);

      // Finish launching
      ((msg_void)objc_msgSend)(mNSApp, objc_sel("finishLaunching"));

      if (!InitExtension())
         return 0;

#ifdef RBLXUI_API
      RblxUI::CreateContext();
      RblxUIIO& io = RblxUI::GetIO();
      io.DisplaySize = RblxVec2((float)config.mWidth, (float)config.mHeight);

      // Get backing scale factor for Retina
      id screen = ((msg_id)objc_msgSend)(mAppWindow, objc_sel("screen"));
      double scaleFactor = 1.0;
      if (screen)
         scaleFactor = ((msg_double)objc_msgSend)(screen, objc_sel("backingScaleFactor"));
      io.DisplayFramebufferScale = RblxVec2((float)scaleFactor, (float)scaleFactor);
#endif

      mDone = false;
      return 1;
   }

   void RblxApp::NewFrame()
   {
      if (s_macOSDone)
         mDone = true;

      // NSUInteger mask = NSEventMaskAny (NSUIntegerMax)
      // id distantPast = [NSDate distantPast]
      id distantPast = ((msg_id)objc_msgSend)(objc_cls("NSDate"), objc_sel("distantPast"));

      // NSDefaultRunLoopMode — load from Foundation
      id NSDefaultRunLoopMode = *(id*)dlsym(RTLD_DEFAULT, "NSDefaultRunLoopMode");

      while (true)
      {
         // [NSApp nextEventMatchingMask:untilDate:inMode:dequeue:]
         typedef id (*msg_nextevent)(id, SEL, NSUInteger, id, id, BOOL);
         id event = ((msg_nextevent)objc_msgSend)(mNSApp,
            objc_sel("nextEventMatchingMask:untilDate:inMode:dequeue:"),
            (NSUInteger)~(NSUInteger)0, distantPast, NSDefaultRunLoopMode, YES);

         if (!event)
            break;

         NSUInteger eventType = ((msg_nsuint)objc_msgSend)(event, objc_sel("type"));

#ifdef RBLXUI_API
         RblxUIIO& io = RblxUI::GetIO();

         switch (eventType)
         {
         case NSEventTypeLeftMouseDown:
            io.MouseDown[0] = true;
            break;
         case NSEventTypeLeftMouseUp:
            io.MouseDown[0] = false;
            break;
         case NSEventTypeRightMouseDown:
            io.MouseDown[1] = true;
            break;
         case NSEventTypeRightMouseUp:
            io.MouseDown[1] = false;
            break;
         case NSEventTypeOtherMouseDown:
            io.MouseDown[2] = true;
            break;
         case NSEventTypeOtherMouseUp:
            io.MouseDown[2] = false;
            break;
         case NSEventTypeMouseMoved:
         case NSEventTypeLeftMouseDragged:
         case NSEventTypeRightMouseDragged:
         case NSEventTypeOtherMouseDragged:
         {
            // Get mouse location in window coordinates
            typedef NSPoint (*msg_loc)(id, SEL);
            NSPoint loc = ((msg_loc)objc_msgSend)(event, objc_sel("locationInWindow"));
            // macOS origin is bottom-left, flip Y
            io.MousePos = RblxVec2((float)loc.x, io.DisplaySize.y - (float)loc.y);
            break;
         }
         case NSEventTypeScrollWheel:
         {
            double deltaY = ((msg_double)objc_msgSend)(event, objc_sel("deltaY"));
            double deltaX = ((msg_double)objc_msgSend)(event, objc_sel("deltaX"));
            io.MouseWheel += (float)deltaY;
            io.MouseWheelH += (float)deltaX;
            break;
         }
         case NSEventTypeKeyDown:
         case NSEventTypeKeyUp:
         {
            unsigned short keyCode = ((msg_ushort)objc_msgSend)(event, objc_sel("keyCode"));
            RblxUIKey key = MacOS_KeyToRblxUIKey(keyCode);
            io.AddKeyEvent(key, eventType == NSEventTypeKeyDown);

            if (eventType == NSEventTypeKeyDown)
            {
               id characters = ((msg_id)objc_msgSend)(event, objc_sel("characters"));
               if (characters)
               {
                  const char* utf8 = ((const char*(*)(id, SEL))objc_msgSend)(characters, objc_sel("UTF8String"));
                  if (utf8)
                     io.AddInputCharactersUTF8(utf8);
               }
            }
            break;
         }
         case NSEventTypeFlagsChanged:
         {
            NSUInteger flags = ((msg_nsuint)objc_msgSend)(event, objc_sel("modifierFlags"));
            io.KeyShift = (flags & NSEventModifierFlagShift) != 0;
            io.KeyCtrl = (flags & NSEventModifierFlagControl) != 0;
            io.KeyAlt = (flags & NSEventModifierFlagOption) != 0;
            io.KeySuper = (flags & NSEventModifierFlagCommand) != 0;
            break;
         }
         }
#endif
         // Forward event for standard handling
         ((msg_void_id)objc_msgSend)(mNSApp, objc_sel("sendEvent:"), event);
      }

      ((msg_void)objc_msgSend)(mNSApp, objc_sel("updateWindows"));

      // Update viewport on resize
      id contentView = ((msg_id)objc_msgSend)(mAppWindow, objc_sel("contentView"));
      NSRect frame = ((msg_nsrect_from_id)objc_msgSend)(contentView, objc_sel("frame"));
      ((msg_void)objc_msgSend)(mGLContext, objc_sel("update"));
      glViewport(0, 0, (int)frame.w, (int)frame.h);

#ifdef RBLXUI_API
      RblxUIIO& io2 = RblxUI::GetIO();
      io2.DisplaySize = RblxVec2((float)frame.w, (float)frame.h);
      RblxUI_NewFrame();
#endif
   }

   void RblxApp::EndFrame()
   {
#ifdef RBLXUI_API
      RblxUI::Render();
      RblxUI_RenderDrawLists(RblxUI::GetDrawData());
#endif
      // [mGLContext flushBuffer]
      ((msg_void)objc_msgSend)(mGLContext, objc_sel("flushBuffer"));
   }

   void RblxApp::Finish()
   {
#ifdef RBLXUI_API
      RblxUI_Shutdown();
#endif
      // [NSOpenGLContext clearCurrentContext]
      ((msg_void)objc_msgSend)(objc_cls("NSOpenGLContext"), objc_sel("clearCurrentContext"));
      ((msg_void)objc_msgSend)(mGLContext, objc_sel("release"));
      ((msg_void)objc_msgSend)(mAppWindow, objc_sel("close"));
      ((msg_void)objc_msgSend)(mWindowDelegate, objc_sel("release"));
   }

#ifdef RBLXUI_API
   void RblxApp::RblxUI_NewFrame()
   {
      if (!g_FontTexture)
         RblxUI_CreateDeviceObjects();
      RblxUI::NewFrame();
   }
#endif

#endif // platform

#endif // IMAPP_IMPL
}
