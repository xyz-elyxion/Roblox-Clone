// rblxstb_glprog v0.02 public domain         functions to reduce GLSL boilerplate
// http://nothings.org/rblxstb/rblxstb_glprog.h   especially with GL1 + ARB extensions
//
// Following defines *before* including have following effects:
//
//     RBLXSTB_GLPROG_IMPLEMENTATION
//           creates the implementation
//
//     RBLXSTB_GLPROG_STATIC
//           forces the implementation to be static (private to file that creates it)
//
//     RBLXSTB_GLPROG_ARB
//           uses ARB extension names for GLSL functions and enumerants instead of core names
//
//     RBLXSTB_GLPROG_ARB_DEFINE_EXTENSIONS
//           instantiates function pointers needed, static to implementing file
//           to avoid collisions (but will collide if implementing file also
//           defines any; best to isolate this to its own file in this case).
//           This will try to automatically #include glext.h, but if it's not
//           in the default include directories you'll need to include it
//           yourself and define the next macro.
//
//     RBLXSTB_GLPROG_SUPPRESS_GLEXT_INCLUDE
//           disables the automatic #include of glext.h which is normally
//           forced by RBLXSTB_GLPROG_ARB_DEFINE_EXTENSIONS
//
// So, e.g., sample usage on an old Windows compiler:
//
//     #define RBLXSTB_GLPROG_IMPLEMENTATION
//     #define RBLXSTB_GLPROG_ARB_DEFINE_EXTENSIONS
//     #include <windows.h>
//     #include "gl/gl.h"
//     #include "rblxstb_glprog.h"
//
// Note though that the header-file version of this (when you don't define
// RBLXSTB_GLPROG_IMPLEMENTATION) still uses GLint and such, so you basically
// can only include it in places where you're already including GL, especially
// on Windows where including "gl.h" requires (some of) "windows.h".
//
// See following comment blocks for function documentation.
//
// Version history:
//    2013-12-08   v0.02   slightly simplified API and reduced GL resource usage (@rygorous)
//    2013-12-08   v0.01   initial release


// header file section starts here
#if !defined(INCLUDE_STB_GLPROG_H)
#define INCLUDE_STB_GLPROG_H

#ifndef RBLXSTB_GLPROG_STATIC
#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////

/////////////    SHADER CREATION


/// EASY API

extern GLuint rblxstbgl_create_program(char const **vertex_source, char const **frag_source, char const **binds, char *error, int error_buflen);
// This function returns a compiled program or 0 if there's an error.
// To free the created program, call rblxstbgl_delete_program.
//
//     rblxstbgl_create_program(
//             char **vertex_source,   // NULL or one or more strings with the vertex shader source, with a final NULL
//             char **frag_source,     // NULL or one or more strings with the fragment shader source, with a final NULL
//             char **binds,           // NULL or zero or more strings with attribute bind names, with a final NULL
//             char *error,            // output location where compile error message is placed
//             int error_buflen)       // length of error output buffer
//
// Returns a GLuint with the GL program object handle.
//
// If an individual bind string is "", no name is bound to that slot (this
// allows you to create binds that aren't continuous integers starting at 0).
//
// If the vertex shader is NULL, then fixed-function vertex pipeline
// is used, if that's legal in your version of GL.
//
// If the fragment shader is NULL, then fixed-function fragment pipeline
// is used, if that's legal in your version of GL.

extern void stgbl_delete_program(GLuint program);
// deletes a program created by rblxstbgl_create_program or rblxstbgl_link_program


/// FLEXIBLE API

extern GLuint rblxstbgl_compile_shader(GLenum type, char const **sources, int num_sources, char *error, int error_buflen);
// compiles a shader. returns the shader on success or 0 on failure.
//
//    type          either:  GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
//                     or    GL_VERTEX_SHADER_ARB or GL_FRAGMENT_SHADER_ARB
//                     or    RBLXSTBGL_VERTEX_SHADER or RBLXSTBGL_FRAGMENT_SHADER
//    sources       array of strings containing the shader source
//    num_sources   number of string in sources, or -1 meaning sources is NULL-terminated
//    error         string to output compiler error to
//    error_buflen  length of error buffer in chars

extern GLuint rblxstbgl_link_program(GLuint vertex_shader, GLuint fragment_shader, char const **binds, int num_binds, char *error, int error_buflen);
// links a shader. returns the linked program on success or 0 on failure.
//
//    vertex_shader    a compiled vertex shader from rblxstbgl_compile_shader, or 0 for fixed-function (if legal)
//    fragment_shader  a compiled fragment shader from rblxstbgl_compile_shader, or 0 for fixed-function (if legal)
//

extern void rblxstbgl_delete_shader(GLuint shader);
// deletes a shader created by rblxstbgl_compile_shader


/////////////    RENDERING WITH SHADERS

extern GLint rblxstbgl_find_uniform(GLuint prog, char *uniform);

extern void rblxstbgl_find_uniforms(GLuint prog, GLint *locations, char const **uniforms, int num_uniforms);
// Given the locations array that is num_uniforms long, fills out
// the locations of each of those uniforms for the specified program.
// If num_uniforms is -1, then uniforms[] must be NULL-terminated

// the following functions just wrap the difference in naming between GL2+ and ARB,
// so you don't need them unless you're using both ARB and GL2+ in the same codebase,
// or you're relying on this lib to provide the extensions
extern void rblxstbglUseProgram(GLuint program);
extern void rblxstbglVertexAttribPointer(GLuint index,  GLint size,  GLenum type,  GLboolean normalized,  GLsizei stride,  const GLvoid * pointer);
extern void rblxstbglEnableVertexAttribArray(GLuint index);
extern void rblxstbglDisableVertexAttribArray(GLuint index);
extern void rblxstbglUniform1fv(GLint loc, GLsizei count, const GLfloat *v);
extern void rblxstbglUniform2fv(GLint loc, GLsizei count, const GLfloat *v);
extern void rblxstbglUniform3fv(GLint loc, GLsizei count, const GLfloat *v);
extern void rblxstbglUniform4fv(GLint loc, GLsizei count, const GLfloat *v);
extern void rblxstbglUniform1iv(GLint loc, GLsizei count, const GLint *v);
extern void rblxstbglUniform2iv(GLint loc, GLsizei count, const GLint *v);
extern void rblxstbglUniform3iv(GLint loc, GLsizei count, const GLint *v);
extern void rblxstbglUniform4iv(GLint loc, GLsizei count, const GLint *v);
extern void rblxstbglUniform1f(GLint loc, float v0);
extern void rblxstbglUniform2f(GLint loc, float v0, float v1);
extern void rblxstbglUniform3f(GLint loc, float v0, float v1, float v2);
extern void rblxstbglUniform4f(GLint loc, float v0, float v1, float v2, float v3);
extern void rblxstbglUniform1i(GLint loc, GLint v0);
extern void rblxstbglUniform2i(GLint loc, GLint v0, GLint v1);
extern void rblxstbglUniform3i(GLint loc, GLint v0, GLint v1, GLint v2);
extern void rblxstbglUniform4i(GLint loc, GLint v0, GLint v1, GLint v2, GLint v3);


//////////////     END OF FUNCTIONS

//////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
#endif // RBLXSTB_GLPROG_STATIC

#ifdef RBLXSTB_GLPROG_ARB
#define RBLXSTBGL_VERTEX_SHADER    GL_VERTEX_SHADER_ARB
#define RBLXSTBGL_FRAGMENT_SHADER  GL_FRAGMENT_SHADER_ARB
#else
#define RBLXSTBGL_VERTEX_SHADER    GL_VERTEX_SHADER
#define RBLXSTBGL_FRAGMENT_SHADER  GL_FRAGMENT_SHADER
#endif

#endif // INCLUDE_STB_GLPROG_H


/////////    header file section ends here


#ifdef RBLXSTB_GLPROG_IMPLEMENTATION
#include <string.h> // strncpy

#ifdef RBLXSTB_GLPROG_STATIC
#define RBLXSTB_GLPROG_DECLARE static
#else
#define RBLXSTB_GLPROG_DECLARE extern
#endif

// check if user wants this file to define the GL extensions itself
#ifdef RBLXSTB_GLPROG_ARB_DEFINE_EXTENSIONS
#define RBLXSTB_GLPROG_ARB  // make sure later code uses the extensions

#ifndef RBLXSTB_GLPROG_SUPPRESS_GLEXT_INCLUDE
#include "glext.h"
#endif

#define RBLXSTB_GLPROG_EXTENSIONS                                 \
   RBLXSTB_GLPROG_FUNC(ATTACHOBJECT        , AttachObject        ) \
   RBLXSTB_GLPROG_FUNC(BINDATTRIBLOCATION  , BindAttribLocation  ) \
   RBLXSTB_GLPROG_FUNC(COMPILESHADER       , CompileShader       ) \
   RBLXSTB_GLPROG_FUNC(CREATEPROGRAMOBJECT , CreateProgramObject ) \
   RBLXSTB_GLPROG_FUNC(CREATESHADEROBJECT  , CreateShaderObject  ) \
   RBLXSTB_GLPROG_FUNC(DELETEOBJECT        , DeleteObject        ) \
   RBLXSTB_GLPROG_FUNC(DETACHOBJECT        , DetachObject        ) \
   RBLXSTB_GLPROG_FUNC(DISABLEVERTEXATTRIBARRAY, DisableVertexAttribArray) \
   RBLXSTB_GLPROG_FUNC(ENABLEVERTEXATTRIBARRAY,  EnableVertexAttribArray ) \
   RBLXSTB_GLPROG_FUNC(GETATTACHEDOBJECTS  , GetAttachedObjects  ) \
   RBLXSTB_GLPROG_FUNC(GETOBJECTPARAMETERIV, GetObjectParameteriv) \
   RBLXSTB_GLPROG_FUNC(GETINFOLOG          , GetInfoLog          ) \
   RBLXSTB_GLPROG_FUNC(GETUNIFORMLOCATION  , GetUniformLocation  ) \
   RBLXSTB_GLPROG_FUNC(LINKPROGRAM         , LinkProgram         ) \
   RBLXSTB_GLPROG_FUNC(SHADERSOURCE        , ShaderSource        ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM1F           , Uniform1f           ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM2F           , Uniform2f           ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM3F           , Uniform3f           ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM4F           , Uniform4f           ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM1I           , Uniform1i           ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM2I           , Uniform2i           ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM3I           , Uniform3i           ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM4I           , Uniform4i           ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM1FV          , Uniform1fv          ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM2FV          , Uniform2fv          ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM3FV          , Uniform3fv          ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM4FV          , Uniform4fv          ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM1IV          , Uniform1iv          ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM2IV          , Uniform2iv          ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM3IV          , Uniform3iv          ) \
   RBLXSTB_GLPROG_FUNC(UNIFORM4IV          , Uniform4iv          ) \
   RBLXSTB_GLPROG_FUNC(USEPROGRAMOBJECT    , UseProgramObject    ) \
   RBLXSTB_GLPROG_FUNC(VERTEXATTRIBPOINTER , VertexAttribPointer )

// define the static function pointers

#define RBLXSTB_GLPROG_FUNC(x,y)  static PFNGL##x##ARBPROC gl##y##ARB;
RBLXSTB_GLPROG_EXTENSIONS
#undef RBLXSTB_GLPROG_FUNC

// define the GetProcAddress

#ifdef _WIN32
#ifndef WINGDIAPI
#ifndef RBLXSTB__HAS_WGLPROC
typedef int (__stdcall *rblxstbgl__voidfunc)(void);
static __declspec(dllimport) rblxstbgl__voidfunc wglGetProcAddress(char *);
#endif
#endif
#define RBLXSTBGL__GET_FUNC(x)   wglGetProcAddress(x)
#else
#error "need to define how this platform gets extensions"
#endif

// create a function that fills out the function pointers

static void rblxstb_glprog_init(void)
{
   static int initialized = 0; // not thread safe!
   if (initialized) return;
   #define RBLXSTB_GLPROG_FUNC(x,y) gl##y##ARB = (PFNGL##x##ARBPROC) RBLXSTBGL__GET_FUNC("gl" #y "ARB");
   RBLXSTB_GLPROG_EXTENSIONS
   #undef RBLXSTB_GLPROG_FUNC
}
#undef RBLXSTB_GLPROG_EXTENSIONS

#else
static void rblxstb_glprog_init(void)
{
}
#endif


// define generic names for many of the gl functions or extensions for later use;
// note that in some cases there are two functions in core and one function in ARB
#ifdef RBLXSTB_GLPROG_ARB
#define rblxstbglCreateShader          glCreateShaderObjectARB
#define rblxstbglDeleteShader          glDeleteObjectARB
#define rblxstbglAttachShader          glAttachObjectARB
#define rblxstbglDetachShader          glDetachObjectARB
#define rblxstbglShaderSource          glShaderSourceARB
#define rblxstbglCompileShader         glCompileShaderARB
#define rblxstbglGetShaderStatus(a,b)  glGetObjectParameterivARB(a, GL_OBJECT_COMPILE_STATUS_ARB, b)
#define rblxstbglGetShaderInfoLog      glGetInfoLogARB
#define rblxstbglCreateProgram         glCreateProgramObjectARB
#define rblxstbglDeleteProgram         glDeleteObjectARB
#define rblxstbglLinkProgram           glLinkProgramARB
#define rblxstbglGetProgramStatus(a,b) glGetObjectParameterivARB(a, GL_OBJECT_LINK_STATUS_ARB, b)
#define rblxstbglGetProgramInfoLog     glGetInfoLogARB
#define rblxstbglGetAttachedShaders    glGetAttachedObjectsARB
#define rblxstbglBindAttribLocation    glBindAttribLocationARB
#define rblxstbglGetUniformLocation    glGetUniformLocationARB
#define rblxstbgl_UseProgram           glUseProgramObjectARB
#else
#define rblxstbglCreateShader          glCreateShader
#define rblxstbglDeleteShader          glDeleteShader
#define rblxstbglAttachShader          glAttachShader
#define rblxstbglDetachShader          glDetachShader
#define rblxstbglShaderSource          glShaderSource
#define rblxstbglCompileShader         glCompileShader
#define rblxstbglGetShaderStatus(a,b)  glGetShaderiv(a, GL_COMPILE_STATUS, b)
#define rblxstbglGetShaderInfoLog      glGetShaderInfoLog
#define rblxstbglCreateProgram         glCreateProgram
#define rblxstbglDeleteProgram         glDeleteProgram
#define rblxstbglLinkProgram           glLinkProgram
#define rblxstbglGetProgramStatus(a,b) glGetProgramiv(a, GL_LINK_STATUS, b)
#define rblxstbglGetProgramInfoLog     glGetProgramInfoLog
#define rblxstbglGetAttachedShaders    glGetAttachedShaders
#define rblxstbglBindAttribLocation    glBindAttribLocation
#define rblxstbglGetUniformLocation    glGetUniformLocation
#define rblxstbgl_UseProgram           glUseProgram
#endif


// perform a safe strcat of 3 strings, given that we can't rely on portable snprintf
// if you need to break on error, this is the best place to place a breakpoint
static void rblxstb_glprog_error(char *error, int error_buflen, char *str1, char *str2, char *str3)
{
   int n = strlen(str1);
   strncpy(error, str1, error_buflen);
   if (n < error_buflen && str2) {
      strncpy(error+n, str2, error_buflen - n);
      n += strlen(str2);
      if (n < error_buflen && str3) {
         strncpy(error+n, str3, error_buflen - n);
      }
   }
   error[error_buflen-1] = 0;
}

RBLXSTB_GLPROG_DECLARE GLuint rblxstbgl_compile_shader(GLenum type, char const **sources, int num_sources, char *error, int error_buflen)
{
   char *typename = (type == RBLXSTBGL_VERTEX_SHADER ? "vertex" : "fragment");
   int len;
   GLint result;
   GLuint shader;

   // initialize the extensions if we haven't already
   rblxstb_glprog_init();

   // allocate

   shader = rblxstbglCreateShader(type);
   if (!shader) {
      rblxstb_glprog_error(error, error_buflen, "Couldn't allocate shader object in rblxstbgl_compile_shader for ", typename, NULL);
      return 0;
   }

   // compile

   // if num_sources is negative, assume source is NULL-terminated and count the non-NULL ones
   if (num_sources < 0)
      for (num_sources = 0; sources[num_sources] != NULL; ++num_sources)
         ;
   rblxstbglShaderSource(shader, num_sources, sources, NULL);
   rblxstbglCompileShader(shader);
   rblxstbglGetShaderStatus(shader, &result);
   if (result)
      return shader;

   // errors

   rblxstb_glprog_error(error, error_buflen, "Compile error for ", typename, " shader: ");
   len = strlen(error);
   if (len < error_buflen)
      rblxstbglGetShaderInfoLog(shader, error_buflen-len, NULL, error+len);

   rblxstbglDeleteShader(shader);
   return 0;
}

RBLXSTB_GLPROG_DECLARE GLuint rblxstbgl_link_program(GLuint vertex_shader, GLuint fragment_shader, char const **binds, int num_binds, char *error, int error_buflen)
{
   int len;
   GLint result;

   // allocate

   GLuint prog = rblxstbglCreateProgram();
   if (!prog) {
      rblxstb_glprog_error(error, error_buflen, "Couldn't allocate program object in rblxstbgl_link_program", NULL, NULL);
      return 0;
   }

   // attach

   if (vertex_shader)
      rblxstbglAttachShader(prog, vertex_shader);
   if (fragment_shader)
      rblxstbglAttachShader(prog, fragment_shader);

   // attribute binds

   if (binds) {
      int i;
      // if num_binds is negative, then it is NULL terminated
      if (num_binds < 0)
         for (num_binds=0; binds[num_binds]; ++num_binds)
            ;
      for (i=0; i < num_binds; ++i)
         if (binds[i] && binds[i][0]) // empty binds can be NULL or ""
            rblxstbglBindAttribLocation(prog, i, binds[i]);
   }

   // link

   rblxstbglLinkProgram(prog);

   // detach

   if (vertex_shader)
      rblxstbglDetachShader(prog, vertex_shader);
   if (fragment_shader)
      rblxstbglDetachShader(prog, fragment_shader);

   // errors

   rblxstbglGetProgramStatus(prog, &result);
   if (result)
      return prog;

   rblxstb_glprog_error(error, error_buflen, "Link error: ", NULL, NULL);
   len = strlen(error);
   if (len < error_buflen)
      rblxstbglGetProgramInfoLog(prog, error_buflen-len, NULL, error+len);

   rblxstbglDeleteProgram(prog);
   return 0;   
}

RBLXSTB_GLPROG_DECLARE GLuint rblxstbgl_create_program(char const **vertex_source, char const **frag_source, char const **binds, char *error, int error_buflen)
{
   GLuint vertex, fragment, prog=0;
   vertex = rblxstbgl_compile_shader(RBLXSTBGL_VERTEX_SHADER, vertex_source, -1, error, error_buflen);
   if (vertex) {
      fragment = rblxstbgl_compile_shader(RBLXSTBGL_FRAGMENT_SHADER, frag_source, -1, error, error_buflen);
      if (fragment)
         prog = rblxstbgl_link_program(vertex, fragment, binds, -1, error, error_buflen);
      if (fragment)
         rblxstbglDeleteShader(fragment);
      rblxstbglDeleteShader(vertex);
   }
   return prog;
}

RBLXSTB_GLPROG_DECLARE void rblxstbgl_delete_shader(GLuint shader)
{
   rblxstbglDeleteShader(shader);
}

RBLXSTB_GLPROG_DECLARE void stgbl_delete_program(GLuint program)
{
   rblxstbglDeleteProgram(program);
}

GLint rblxstbgl_find_uniform(GLuint prog, char *uniform)
{
   return rblxstbglGetUniformLocation(prog, uniform);
}

RBLXSTB_GLPROG_DECLARE void rblxstbgl_find_uniforms(GLuint prog, GLint *locations, char const **uniforms, int num_uniforms)
{
   int i;
   if (num_uniforms < 0)
      num_uniforms = 999999;
   for (i=0; i < num_uniforms && uniforms[i]; ++i)
      locations[i] = rblxstbglGetUniformLocation(prog, uniforms[i]);
}

RBLXSTB_GLPROG_DECLARE void rblxstbglUseProgram(GLuint program)
{
   rblxstbgl_UseProgram(program);
}

#ifdef RBLXSTB_GLPROG_ARB
#define RBLXSTBGL_ARBIFY(name)   name##ARB
#else
#define RBLXSTBGL_ARBIFY(name)   name
#endif

RBLXSTB_GLPROG_DECLARE void rblxstbglVertexAttribPointer(GLuint index,  GLint size,  GLenum type,  GLboolean normalized,  GLsizei stride,  const GLvoid * pointer)
{
   RBLXSTBGL_ARBIFY(glVertexAttribPointer)(index, size, type, normalized, stride, pointer);
}

RBLXSTB_GLPROG_DECLARE void rblxstbglEnableVertexAttribArray (GLuint index) { RBLXSTBGL_ARBIFY(glEnableVertexAttribArray )(index); }
RBLXSTB_GLPROG_DECLARE void rblxstbglDisableVertexAttribArray(GLuint index) { RBLXSTBGL_ARBIFY(glDisableVertexAttribArray)(index); }

RBLXSTB_GLPROG_DECLARE void rblxstbglUniform1fv(GLint loc, GLsizei count, const GLfloat *v) { RBLXSTBGL_ARBIFY(glUniform1fv)(loc,count,v); }
RBLXSTB_GLPROG_DECLARE void rblxstbglUniform2fv(GLint loc, GLsizei count, const GLfloat *v) { RBLXSTBGL_ARBIFY(glUniform2fv)(loc,count,v); }
RBLXSTB_GLPROG_DECLARE void rblxstbglUniform3fv(GLint loc, GLsizei count, const GLfloat *v) { RBLXSTBGL_ARBIFY(glUniform3fv)(loc,count,v); }
RBLXSTB_GLPROG_DECLARE void rblxstbglUniform4fv(GLint loc, GLsizei count, const GLfloat *v) { RBLXSTBGL_ARBIFY(glUniform4fv)(loc,count,v); }

RBLXSTB_GLPROG_DECLARE void rblxstbglUniform1iv(GLint loc, GLsizei count, const GLint   *v) { RBLXSTBGL_ARBIFY(glUniform1iv)(loc,count,v); }
RBLXSTB_GLPROG_DECLARE void rblxstbglUniform2iv(GLint loc, GLsizei count, const GLint   *v) { RBLXSTBGL_ARBIFY(glUniform2iv)(loc,count,v); }
RBLXSTB_GLPROG_DECLARE void rblxstbglUniform3iv(GLint loc, GLsizei count, const GLint   *v) { RBLXSTBGL_ARBIFY(glUniform3iv)(loc,count,v); }
RBLXSTB_GLPROG_DECLARE void rblxstbglUniform4iv(GLint loc, GLsizei count, const GLint   *v) { RBLXSTBGL_ARBIFY(glUniform4iv)(loc,count,v); }

RBLXSTB_GLPROG_DECLARE void rblxstbglUniform1f(GLint loc, float v0)
    { RBLXSTBGL_ARBIFY(glUniform1f)(loc,v0); }
RBLXSTB_GLPROG_DECLARE void rblxstbglUniform2f(GLint loc, float v0, float v1)
    { RBLXSTBGL_ARBIFY(glUniform2f)(loc,v0,v1); }
RBLXSTB_GLPROG_DECLARE void rblxstbglUniform3f(GLint loc, float v0, float v1, float v2)
    { RBLXSTBGL_ARBIFY(glUniform3f)(loc,v0,v1,v2); }
RBLXSTB_GLPROG_DECLARE void rblxstbglUniform4f(GLint loc, float v0, float v1, float v2, float v3)
    { RBLXSTBGL_ARBIFY(glUniform4f)(loc,v0,v1,v2,v3); }

RBLXSTB_GLPROG_DECLARE void rblxstbglUniform1i(GLint loc, GLint v0)
    { RBLXSTBGL_ARBIFY(glUniform1i)(loc,v0); }
RBLXSTB_GLPROG_DECLARE void rblxstbglUniform2i(GLint loc, GLint v0, GLint v1)
    { RBLXSTBGL_ARBIFY(glUniform2i)(loc,v0,v1); }
RBLXSTB_GLPROG_DECLARE void rblxstbglUniform3i(GLint loc, GLint v0, GLint v1, GLint v2)
    { RBLXSTBGL_ARBIFY(glUniform3i)(loc,v0,v1,v2); }
RBLXSTB_GLPROG_DECLARE void rblxstbglUniform4i(GLint loc, GLint v0, GLint v1, GLint v2, GLint v3)
    { RBLXSTBGL_ARBIFY(glUniform4i)(loc,v0,v1,v2,v3); }

#endif
