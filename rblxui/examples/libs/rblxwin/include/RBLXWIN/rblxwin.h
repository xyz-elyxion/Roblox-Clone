/*************************************************************************
 * RBLXWIN 3.2 - www.rblxwin.org
 * A library for OpenGL, window and input
 *------------------------------------------------------------------------
 * Copyright (c) 2002-2006 Marcus Geelnard
 * Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 *************************************************************************/

#ifndef _rblxwin_h_
#define _rblxwin_h_

#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************************
 * Doxygen documentation
 *************************************************************************/

/*! @file rblxwin.h
 *  @brief The header of the RBLXWIN 3 API.
 *
 *  This is the header file of the RBLXWIN 3 API.  It defines all its types and
 *  declares all its functions.
 *
 *  For more information about how to use this file, see @ref build_include.
 */
/*! @defgroup context Context reference
 *
 *  This is the reference documentation for OpenGL and OpenGL ES context related
 *  functions.  For more task-oriented information, see the @ref context_guide.
 */
/*! @defgroup vulkan Vulkan reference
 *
 *  This is the reference documentation for Vulkan related functions and types.
 *  For more task-oriented information, see the @ref vulkan_guide.
 */
/*! @defgroup init Initialization, version and error reference
 *
 *  This is the reference documentation for initialization and termination of
 *  the library, version management and error handling.  For more task-oriented
 *  information, see the @ref intro_guide.
 */
/*! @defgroup input Input reference
 *
 *  This is the reference documentation for input related functions and types.
 *  For more task-oriented information, see the @ref input_guide.
 */
/*! @defgroup monitor Monitor reference
 *
 *  This is the reference documentation for monitor related functions and types.
 *  For more task-oriented information, see the @ref monitor_guide.
 */
/*! @defgroup window Window reference
 *
 *  This is the reference documentation for window related functions and types,
 *  including creation, deletion and event polling.  For more task-oriented
 *  information, see the @ref window_guide.
 */


/*************************************************************************
 * Compiler- and platform-specific preprocessor work
 *************************************************************************/

/* If we are we on Windows, we want a single define for it.
 */
#if !defined(_WIN32) && (defined(__WIN32__) || defined(WIN32) || defined(__MINGW32__))
 #define _WIN32
#endif /* _WIN32 */

/* It is customary to use APIENTRY for OpenGL function pointer declarations on
 * all platforms.  Additionally, the Windows OpenGL header needs APIENTRY.
 */
#ifndef APIENTRY
 #ifdef _WIN32
  #define APIENTRY __stdcall
 #else
  #define APIENTRY
 #endif
#endif /* APIENTRY */

/* Some Windows OpenGL headers need this.
 */
#if !defined(WINGDIAPI) && defined(_WIN32)
 #define WINGDIAPI __declspec(dllimport)
 #define RBLXWIN_WINGDIAPI_DEFINED
#endif /* WINGDIAPI */

/* Some Windows GLU headers need this.
 */
#if !defined(CALLBACK) && defined(_WIN32)
 #define CALLBACK __stdcall
 #define RBLXWIN_CALLBACK_DEFINED
#endif /* CALLBACK */

/* Most Windows GLU headers need wchar_t.
 * The OS X OpenGL header blocks the definition of ptrdiff_t by glext.h.
 * Include it unconditionally to avoid surprising side-effects.
 */
#include <stddef.h>
#include <stdint.h>

/* Include the chosen client API headers.
 */
#if defined(__APPLE__)
 #if defined(RBLXWIN_INCLUDE_GLCOREARB)
  #include <OpenGL/gl3.h>
  #if defined(RBLXWIN_INCLUDE_GLEXT)
   #include <OpenGL/gl3ext.h>
  #endif
 #elif !defined(RBLXWIN_INCLUDE_NONE)
  #if !defined(RBLXWIN_INCLUDE_GLEXT)
   #define GL_GLEXT_LEGACY
  #endif
  #include <OpenGL/gl.h>
 #endif
 #if defined(RBLXWIN_INCLUDE_GLU)
  #include <OpenGL/glu.h>
 #endif
#else
 #if defined(RBLXWIN_INCLUDE_GLCOREARB)
  #include <GL/glcorearb.h>
 #elif defined(RBLXWIN_INCLUDE_ES1)
  #include <GLES/gl.h>
  #if defined(RBLXWIN_INCLUDE_GLEXT)
   #include <GLES/glext.h>
  #endif
 #elif defined(RBLXWIN_INCLUDE_ES2)
  #include <GLES2/gl2.h>
  #if defined(RBLXWIN_INCLUDE_GLEXT)
   #include <GLES2/gl2ext.h>
  #endif
 #elif defined(RBLXWIN_INCLUDE_ES3)
  #include <GLES3/gl3.h>
  #if defined(RBLXWIN_INCLUDE_GLEXT)
   #include <GLES2/gl2ext.h>
  #endif
 #elif defined(RBLXWIN_INCLUDE_ES31)
  #include <GLES3/gl31.h>
  #if defined(RBLXWIN_INCLUDE_GLEXT)
   #include <GLES2/gl2ext.h>
  #endif
 #elif defined(RBLXWIN_INCLUDE_VULKAN)
  #include <vulkan/vulkan.h>
 #elif !defined(RBLXWIN_INCLUDE_NONE)
  #include <GL/gl.h>
  #if defined(RBLXWIN_INCLUDE_GLEXT)
   #include <GL/glext.h>
  #endif
 #endif
 #if defined(RBLXWIN_INCLUDE_GLU)
  #include <GL/glu.h>
 #endif
#endif

#if defined(RBLXWIN_DLL) && defined(_RBLXWIN_BUILD_DLL)
 /* RBLXWIN_DLL must be defined by applications that are linking against the DLL
  * version of the RBLXWIN library.  _RBLXWIN_BUILD_DLL is defined by the RBLXWIN
  * configuration header when compiling the DLL version of the library.
  */
 #error "You must not have both RBLXWIN_DLL and _RBLXWIN_BUILD_DLL defined"
#endif

/* RBLXWINAPI is used to declare public API functions for export
 * from the DLL / shared library / dynamic library.
 */
#if defined(_WIN32) && defined(_RBLXWIN_BUILD_DLL)
 /* We are building RBLXWIN as a Win32 DLL */
 #define RBLXWINAPI __declspec(dllexport)
#elif defined(_WIN32) && defined(RBLXWIN_DLL)
 /* We are calling RBLXWIN as a Win32 DLL */
 #define RBLXWINAPI __declspec(dllimport)
#elif defined(__GNUC__) && defined(_RBLXWIN_BUILD_DLL)
 /* We are building RBLXWIN as a shared / dynamic library */
 #define RBLXWINAPI __attribute__((visibility("default")))
#else
 /* We are building or calling RBLXWIN as a static library */
 #define RBLXWINAPI
#endif


/*************************************************************************
 * RBLXWIN API tokens
 *************************************************************************/

/*! @name RBLXWIN version macros
 *  @{ */
/*! @brief The major version number of the RBLXWIN library.
 *
 *  This is incremented when the API is changed in non-compatible ways.
 *  @ingroup init
 */
#define RBLXWIN_VERSION_MAJOR          3
/*! @brief The minor version number of the RBLXWIN library.
 *
 *  This is incremented when features are added to the API but it remains
 *  backward-compatible.
 *  @ingroup init
 */
#define RBLXWIN_VERSION_MINOR          2
/*! @brief The revision number of the RBLXWIN library.
 *
 *  This is incremented when a bug fix release is made that does not contain any
 *  API changes.
 *  @ingroup init
 */
#define RBLXWIN_VERSION_REVISION       0
/*! @} */

/*! @name Boolean values
 *  @{ */
/*! @brief One.
 *
 *  One.  Seriously.  You don't _need_ to use this symbol in your code.  It's
 *  just semantic sugar for the number 1.  You can use `1` or `true` or `_True`
 *  or `GL_TRUE` or whatever you want.
 */
#define RBLXWIN_TRUE                   1
/*! @brief Zero.
 *
 *  Zero.  Seriously.  You don't _need_ to use this symbol in your code.  It's
 *  just just semantic sugar for the number 0.  You can use `0` or `false` or
 *  `_False` or `GL_FALSE` or whatever you want.
 */
#define RBLXWIN_FALSE                  0
/*! @} */

/*! @name Key and button actions
 *  @{ */
/*! @brief The key or mouse button was released.
 *
 *  The key or mouse button was released.
 *
 *  @ingroup input
 */
#define RBLXWIN_RELEASE                0
/*! @brief The key or mouse button was pressed.
 *
 *  The key or mouse button was pressed.
 *
 *  @ingroup input
 */
#define RBLXWIN_PRESS                  1
/*! @brief The key was held down until it repeated.
 *
 *  The key was held down until it repeated.
 *
 *  @ingroup input
 */
#define RBLXWIN_REPEAT                 2
/*! @} */

/*! @defgroup keys Keyboard keys
 *
 *  See [key input](@ref input_key) for how these are used.
 *
 *  These key codes are inspired by the _USB HID Usage Tables v1.12_ (p. 53-60),
 *  but re-arranged to map to 7-bit ASCII for printable keys (function keys are
 *  put in the 256+ range).
 *
 *  The naming of the key codes follow these rules:
 *   - The US keyboard layout is used
 *   - Names of printable alpha-numeric characters are used (e.g. "A", "R",
 *     "3", etc.)
 *   - For non-alphanumeric characters, Unicode:ish names are used (e.g.
 *     "COMMA", "LEFT_SQUARE_BRACKET", etc.). Note that some names do not
 *     correspond to the Unicode standard (usually for brevity)
 *   - Keys that lack a clear US mapping are named "WORLD_x"
 *   - For non-printable keys, custom names are used (e.g. "F4",
 *     "BACKSPACE", etc.)
 *
 *  @ingroup input
 *  @{
 */

/* The unknown key */
#define RBLXWIN_KEY_UNKNOWN            -1

/* Printable keys */
#define RBLXWIN_KEY_SPACE              32
#define RBLXWIN_KEY_APOSTROPHE         39  /* ' */
#define RBLXWIN_KEY_COMMA              44  /* , */
#define RBLXWIN_KEY_MINUS              45  /* - */
#define RBLXWIN_KEY_PERIOD             46  /* . */
#define RBLXWIN_KEY_SLASH              47  /* / */
#define RBLXWIN_KEY_0                  48
#define RBLXWIN_KEY_1                  49
#define RBLXWIN_KEY_2                  50
#define RBLXWIN_KEY_3                  51
#define RBLXWIN_KEY_4                  52
#define RBLXWIN_KEY_5                  53
#define RBLXWIN_KEY_6                  54
#define RBLXWIN_KEY_7                  55
#define RBLXWIN_KEY_8                  56
#define RBLXWIN_KEY_9                  57
#define RBLXWIN_KEY_SEMICOLON          59  /* ; */
#define RBLXWIN_KEY_EQUAL              61  /* = */
#define RBLXWIN_KEY_A                  65
#define RBLXWIN_KEY_B                  66
#define RBLXWIN_KEY_C                  67
#define RBLXWIN_KEY_D                  68
#define RBLXWIN_KEY_E                  69
#define RBLXWIN_KEY_F                  70
#define RBLXWIN_KEY_G                  71
#define RBLXWIN_KEY_H                  72
#define RBLXWIN_KEY_I                  73
#define RBLXWIN_KEY_J                  74
#define RBLXWIN_KEY_K                  75
#define RBLXWIN_KEY_L                  76
#define RBLXWIN_KEY_M                  77
#define RBLXWIN_KEY_N                  78
#define RBLXWIN_KEY_O                  79
#define RBLXWIN_KEY_P                  80
#define RBLXWIN_KEY_Q                  81
#define RBLXWIN_KEY_R                  82
#define RBLXWIN_KEY_S                  83
#define RBLXWIN_KEY_T                  84
#define RBLXWIN_KEY_U                  85
#define RBLXWIN_KEY_V                  86
#define RBLXWIN_KEY_W                  87
#define RBLXWIN_KEY_X                  88
#define RBLXWIN_KEY_Y                  89
#define RBLXWIN_KEY_Z                  90
#define RBLXWIN_KEY_LEFT_BRACKET       91  /* [ */
#define RBLXWIN_KEY_BACKSLASH          92  /* \ */
#define RBLXWIN_KEY_RIGHT_BRACKET      93  /* ] */
#define RBLXWIN_KEY_GRAVE_ACCENT       96  /* ` */
#define RBLXWIN_KEY_WORLD_1            161 /* non-US #1 */
#define RBLXWIN_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define RBLXWIN_KEY_ESCAPE             256
#define RBLXWIN_KEY_ENTER              257
#define RBLXWIN_KEY_TAB                258
#define RBLXWIN_KEY_BACKSPACE          259
#define RBLXWIN_KEY_INSERT             260
#define RBLXWIN_KEY_DELETE             261
#define RBLXWIN_KEY_RIGHT              262
#define RBLXWIN_KEY_LEFT               263
#define RBLXWIN_KEY_DOWN               264
#define RBLXWIN_KEY_UP                 265
#define RBLXWIN_KEY_PAGE_UP            266
#define RBLXWIN_KEY_PAGE_DOWN          267
#define RBLXWIN_KEY_HOME               268
#define RBLXWIN_KEY_END                269
#define RBLXWIN_KEY_CAPS_LOCK          280
#define RBLXWIN_KEY_SCROLL_LOCK        281
#define RBLXWIN_KEY_NUM_LOCK           282
#define RBLXWIN_KEY_PRINT_SCREEN       283
#define RBLXWIN_KEY_PAUSE              284
#define RBLXWIN_KEY_F1                 290
#define RBLXWIN_KEY_F2                 291
#define RBLXWIN_KEY_F3                 292
#define RBLXWIN_KEY_F4                 293
#define RBLXWIN_KEY_F5                 294
#define RBLXWIN_KEY_F6                 295
#define RBLXWIN_KEY_F7                 296
#define RBLXWIN_KEY_F8                 297
#define RBLXWIN_KEY_F9                 298
#define RBLXWIN_KEY_F10                299
#define RBLXWIN_KEY_F11                300
#define RBLXWIN_KEY_F12                301
#define RBLXWIN_KEY_F13                302
#define RBLXWIN_KEY_F14                303
#define RBLXWIN_KEY_F15                304
#define RBLXWIN_KEY_F16                305
#define RBLXWIN_KEY_F17                306
#define RBLXWIN_KEY_F18                307
#define RBLXWIN_KEY_F19                308
#define RBLXWIN_KEY_F20                309
#define RBLXWIN_KEY_F21                310
#define RBLXWIN_KEY_F22                311
#define RBLXWIN_KEY_F23                312
#define RBLXWIN_KEY_F24                313
#define RBLXWIN_KEY_F25                314
#define RBLXWIN_KEY_KP_0               320
#define RBLXWIN_KEY_KP_1               321
#define RBLXWIN_KEY_KP_2               322
#define RBLXWIN_KEY_KP_3               323
#define RBLXWIN_KEY_KP_4               324
#define RBLXWIN_KEY_KP_5               325
#define RBLXWIN_KEY_KP_6               326
#define RBLXWIN_KEY_KP_7               327
#define RBLXWIN_KEY_KP_8               328
#define RBLXWIN_KEY_KP_9               329
#define RBLXWIN_KEY_KP_DECIMAL         330
#define RBLXWIN_KEY_KP_DIVIDE          331
#define RBLXWIN_KEY_KP_MULTIPLY        332
#define RBLXWIN_KEY_KP_SUBTRACT        333
#define RBLXWIN_KEY_KP_ADD             334
#define RBLXWIN_KEY_KP_ENTER           335
#define RBLXWIN_KEY_KP_EQUAL           336
#define RBLXWIN_KEY_LEFT_SHIFT         340
#define RBLXWIN_KEY_LEFT_CONTROL       341
#define RBLXWIN_KEY_LEFT_ALT           342
#define RBLXWIN_KEY_LEFT_SUPER         343
#define RBLXWIN_KEY_RIGHT_SHIFT        344
#define RBLXWIN_KEY_RIGHT_CONTROL      345
#define RBLXWIN_KEY_RIGHT_ALT          346
#define RBLXWIN_KEY_RIGHT_SUPER        347
#define RBLXWIN_KEY_MENU               348

#define RBLXWIN_KEY_LAST               RBLXWIN_KEY_MENU

/*! @} */

/*! @defgroup mods Modifier key flags
 *
 *  See [key input](@ref input_key) for how these are used.
 *
 *  @ingroup input
 *  @{ */

/*! @brief If this bit is set one or more Shift keys were held down.
 */
#define RBLXWIN_MOD_SHIFT           0x0001
/*! @brief If this bit is set one or more Control keys were held down.
 */
#define RBLXWIN_MOD_CONTROL         0x0002
/*! @brief If this bit is set one or more Alt keys were held down.
 */
#define RBLXWIN_MOD_ALT             0x0004
/*! @brief If this bit is set one or more Super keys were held down.
 */
#define RBLXWIN_MOD_SUPER           0x0008

/*! @} */

/*! @defgroup buttons Mouse buttons
 *
 *  See [mouse button input](@ref input_mouse_button) for how these are used.
 *
 *  @ingroup input
 *  @{ */
#define RBLXWIN_MOUSE_BUTTON_1         0
#define RBLXWIN_MOUSE_BUTTON_2         1
#define RBLXWIN_MOUSE_BUTTON_3         2
#define RBLXWIN_MOUSE_BUTTON_4         3
#define RBLXWIN_MOUSE_BUTTON_5         4
#define RBLXWIN_MOUSE_BUTTON_6         5
#define RBLXWIN_MOUSE_BUTTON_7         6
#define RBLXWIN_MOUSE_BUTTON_8         7
#define RBLXWIN_MOUSE_BUTTON_LAST      RBLXWIN_MOUSE_BUTTON_8
#define RBLXWIN_MOUSE_BUTTON_LEFT      RBLXWIN_MOUSE_BUTTON_1
#define RBLXWIN_MOUSE_BUTTON_RIGHT     RBLXWIN_MOUSE_BUTTON_2
#define RBLXWIN_MOUSE_BUTTON_MIDDLE    RBLXWIN_MOUSE_BUTTON_3
/*! @} */

/*! @defgroup joysticks Joysticks
 *
 *  See [joystick input](@ref joystick) for how these are used.
 *
 *  @ingroup input
 *  @{ */
#define RBLXWIN_JOYSTICK_1             0
#define RBLXWIN_JOYSTICK_2             1
#define RBLXWIN_JOYSTICK_3             2
#define RBLXWIN_JOYSTICK_4             3
#define RBLXWIN_JOYSTICK_5             4
#define RBLXWIN_JOYSTICK_6             5
#define RBLXWIN_JOYSTICK_7             6
#define RBLXWIN_JOYSTICK_8             7
#define RBLXWIN_JOYSTICK_9             8
#define RBLXWIN_JOYSTICK_10            9
#define RBLXWIN_JOYSTICK_11            10
#define RBLXWIN_JOYSTICK_12            11
#define RBLXWIN_JOYSTICK_13            12
#define RBLXWIN_JOYSTICK_14            13
#define RBLXWIN_JOYSTICK_15            14
#define RBLXWIN_JOYSTICK_16            15
#define RBLXWIN_JOYSTICK_LAST          RBLXWIN_JOYSTICK_16
/*! @} */

/*! @defgroup errors Error codes
 *
 *  See [error handling](@ref error_handling) for how these are used.
 *
 *  @ingroup init
 *  @{ */
/*! @brief RBLXWIN has not been initialized.
 *
 *  This occurs if a RBLXWIN function was called that must not be called unless the
 *  library is [initialized](@ref intro_init).
 *
 *  @analysis Application programmer error.  Initialize RBLXWIN before calling any
 *  function that requires initialization.
 */
#define RBLXWIN_NOT_INITIALIZED        0x00010001
/*! @brief No context is current for this thread.
 *
 *  This occurs if a RBLXWIN function was called that needs and operates on the
 *  current OpenGL or OpenGL ES context but no context is current on the calling
 *  thread.  One such function is @ref rblxwinSwapInterval.
 *
 *  @analysis Application programmer error.  Ensure a context is current before
 *  calling functions that require a current context.
 */
#define RBLXWIN_NO_CURRENT_CONTEXT     0x00010002
/*! @brief One of the arguments to the function was an invalid enum value.
 *
 *  One of the arguments to the function was an invalid enum value, for example
 *  requesting [RBLXWIN_RED_BITS](@ref window_hints_fb) with @ref
 *  rblxwinGetWindowAttrib.
 *
 *  @analysis Application programmer error.  Fix the offending call.
 */
#define RBLXWIN_INVALID_ENUM           0x00010003
/*! @brief One of the arguments to the function was an invalid value.
 *
 *  One of the arguments to the function was an invalid value, for example
 *  requesting a non-existent OpenGL or OpenGL ES version like 2.7.
 *
 *  Requesting a valid but unavailable OpenGL or OpenGL ES version will instead
 *  result in a @ref RBLXWIN_VERSION_UNAVAILABLE error.
 *
 *  @analysis Application programmer error.  Fix the offending call.
 */
#define RBLXWIN_INVALID_VALUE          0x00010004
/*! @brief A memory allocation failed.
 *
 *  A memory allocation failed.
 *
 *  @analysis A bug in RBLXWIN or the underlying operating system.  Report the bug
 *  to our [issue tracker](https://github.com/rblxwin/rblxwin/issues).
 */
#define RBLXWIN_OUT_OF_MEMORY          0x00010005
/*! @brief RBLXWIN could not find support for the requested API on the system.
 *
 *  RBLXWIN could not find support for the requested API on the system.
 *
 *  @analysis The installed graphics driver does not support the requested
 *  API, or does not support it via the chosen context creation backend.
 *  Below are a few examples.
 *
 *  @par
 *  Some pre-installed Windows graphics drivers do not support OpenGL.  AMD only
 *  supports OpenGL ES via EGL, while Nvidia and Intel only support it via
 *  a WGL or GLX extension.  OS X does not provide OpenGL ES at all.  The Mesa
 *  EGL, OpenGL and OpenGL ES libraries do not interface with the Nvidia binary
 *  driver.  Older graphics drivers do not support Vulkan.
 */
#define RBLXWIN_API_UNAVAILABLE        0x00010006
/*! @brief The requested OpenGL or OpenGL ES version is not available.
 *
 *  The requested OpenGL or OpenGL ES version (including any requested context
 *  or framebuffer hints) is not available on this machine.
 *
 *  @analysis The machine does not support your requirements.  If your
 *  application is sufficiently flexible, downgrade your requirements and try
 *  again.  Otherwise, inform the user that their machine does not match your
 *  requirements.
 *
 *  @par
 *  Future invalid OpenGL and OpenGL ES versions, for example OpenGL 4.8 if 5.0
 *  comes out before the 4.x series gets that far, also fail with this error and
 *  not @ref RBLXWIN_INVALID_VALUE, because RBLXWIN cannot know what future versions
 *  will exist.
 */
#define RBLXWIN_VERSION_UNAVAILABLE    0x00010007
/*! @brief A platform-specific error occurred that does not match any of the
 *  more specific categories.
 *
 *  A platform-specific error occurred that does not match any of the more
 *  specific categories.
 *
 *  @analysis A bug or configuration error in RBLXWIN, the underlying operating
 *  system or its drivers, or a lack of required resources.  Report the issue to
 *  our [issue tracker](https://github.com/rblxwin/rblxwin/issues).
 */
#define RBLXWIN_PLATFORM_ERROR         0x00010008
/*! @brief The requested format is not supported or available.
 *
 *  If emitted during window creation, the requested pixel format is not
 *  supported.
 *
 *  If emitted when querying the clipboard, the contents of the clipboard could
 *  not be converted to the requested format.
 *
 *  @analysis If emitted during window creation, one or more
 *  [hard constraints](@ref window_hints_hard) did not match any of the
 *  available pixel formats.  If your application is sufficiently flexible,
 *  downgrade your requirements and try again.  Otherwise, inform the user that
 *  their machine does not match your requirements.
 *
 *  @par
 *  If emitted when querying the clipboard, ignore the error or report it to
 *  the user, as appropriate.
 */
#define RBLXWIN_FORMAT_UNAVAILABLE     0x00010009
/*! @brief The specified window does not have an OpenGL or OpenGL ES context.
 *
 *  A window that does not have an OpenGL or OpenGL ES context was passed to
 *  a function that requires it to have one.
 *
 *  @analysis Application programmer error.  Fix the offending call.
 */
#define RBLXWIN_NO_WINDOW_CONTEXT      0x0001000A
/*! @} */

#define RBLXWIN_FOCUSED                0x00020001
#define RBLXWIN_ICONIFIED              0x00020002
#define RBLXWIN_RESIZABLE              0x00020003
#define RBLXWIN_VISIBLE                0x00020004
#define RBLXWIN_DECORATED              0x00020005
#define RBLXWIN_AUTO_ICONIFY           0x00020006
#define RBLXWIN_FLOATING               0x00020007
#define RBLXWIN_MAXIMIZED              0x00020008

#define RBLXWIN_RED_BITS               0x00021001
#define RBLXWIN_GREEN_BITS             0x00021002
#define RBLXWIN_BLUE_BITS              0x00021003
#define RBLXWIN_ALPHA_BITS             0x00021004
#define RBLXWIN_DEPTH_BITS             0x00021005
#define RBLXWIN_STENCIL_BITS           0x00021006
#define RBLXWIN_ACCUM_RED_BITS         0x00021007
#define RBLXWIN_ACCUM_GREEN_BITS       0x00021008
#define RBLXWIN_ACCUM_BLUE_BITS        0x00021009
#define RBLXWIN_ACCUM_ALPHA_BITS       0x0002100A
#define RBLXWIN_AUX_BUFFERS            0x0002100B
#define RBLXWIN_STEREO                 0x0002100C
#define RBLXWIN_SAMPLES                0x0002100D
#define RBLXWIN_SRGB_CAPABLE           0x0002100E
#define RBLXWIN_REFRESH_RATE           0x0002100F
#define RBLXWIN_DOUBLEBUFFER           0x00021010

#define RBLXWIN_CLIENT_API             0x00022001
#define RBLXWIN_CONTEXT_VERSION_MAJOR  0x00022002
#define RBLXWIN_CONTEXT_VERSION_MINOR  0x00022003
#define RBLXWIN_CONTEXT_REVISION       0x00022004
#define RBLXWIN_CONTEXT_ROBUSTNESS     0x00022005
#define RBLXWIN_OPENGL_FORWARD_COMPAT  0x00022006
#define RBLXWIN_OPENGL_DEBUG_CONTEXT   0x00022007
#define RBLXWIN_OPENGL_PROFILE         0x00022008
#define RBLXWIN_CONTEXT_RELEASE_BEHAVIOR 0x00022009
#define RBLXWIN_CONTEXT_NO_ERROR       0x0002200A

#define RBLXWIN_NO_API                          0
#define RBLXWIN_OPENGL_API             0x00030001
#define RBLXWIN_OPENGL_ES_API          0x00030002

#define RBLXWIN_NO_ROBUSTNESS                   0
#define RBLXWIN_NO_RESET_NOTIFICATION  0x00031001
#define RBLXWIN_LOSE_CONTEXT_ON_RESET  0x00031002

#define RBLXWIN_OPENGL_ANY_PROFILE              0
#define RBLXWIN_OPENGL_CORE_PROFILE    0x00032001
#define RBLXWIN_OPENGL_COMPAT_PROFILE  0x00032002

#define RBLXWIN_CURSOR                 0x00033001
#define RBLXWIN_STICKY_KEYS            0x00033002
#define RBLXWIN_STICKY_MOUSE_BUTTONS   0x00033003

#define RBLXWIN_CURSOR_NORMAL          0x00034001
#define RBLXWIN_CURSOR_HIDDEN          0x00034002
#define RBLXWIN_CURSOR_DISABLED        0x00034003

#define RBLXWIN_ANY_RELEASE_BEHAVIOR            0
#define RBLXWIN_RELEASE_BEHAVIOR_FLUSH 0x00035001
#define RBLXWIN_RELEASE_BEHAVIOR_NONE  0x00035002

/*! @defgroup shapes Standard cursor shapes
 *
 *  See [standard cursor creation](@ref cursor_standard) for how these are used.
 *
 *  @ingroup input
 *  @{ */

/*! @brief The regular arrow cursor shape.
 *
 *  The regular arrow cursor.
 */
#define RBLXWIN_ARROW_CURSOR           0x00036001
/*! @brief The text input I-beam cursor shape.
 *
 *  The text input I-beam cursor shape.
 */
#define RBLXWIN_IBEAM_CURSOR           0x00036002
/*! @brief The crosshair shape.
 *
 *  The crosshair shape.
 */
#define RBLXWIN_CROSSHAIR_CURSOR       0x00036003
/*! @brief The hand shape.
 *
 *  The hand shape.
 */
#define RBLXWIN_HAND_CURSOR            0x00036004
/*! @brief The horizontal resize arrow shape.
 *
 *  The horizontal resize arrow shape.
 */
#define RBLXWIN_HRESIZE_CURSOR         0x00036005
/*! @brief The vertical resize arrow shape.
 *
 *  The vertical resize arrow shape.
 */
#define RBLXWIN_VRESIZE_CURSOR         0x00036006
/*! @} */

#define RBLXWIN_CONNECTED              0x00040001
#define RBLXWIN_DISCONNECTED           0x00040002

#define RBLXWIN_DONT_CARE              -1


/*************************************************************************
 * RBLXWIN API types
 *************************************************************************/

/*! @brief Client API function pointer type.
 *
 *  Generic function pointer used for returning client API function pointers
 *  without forcing a cast from a regular pointer.
 *
 *  @sa @ref context_glext
 *  @sa rblxwinGetProcAddress
 *
 *  @since Added in version 3.0.
 
 *  @ingroup context
 */
typedef void (*RBLXWINglproc)(void);

/*! @brief Vulkan API function pointer type.
 *
 *  Generic function pointer used for returning Vulkan API function pointers
 *  without forcing a cast from a regular pointer.
 *
 *  @sa @ref vulkan_proc
 *  @sa rblxwinGetInstanceProcAddress
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup vulkan
 */
typedef void (*RBLXWINvkproc)(void);

/*! @brief Opaque monitor object.
 *
 *  Opaque monitor object.
 *
 *  @see @ref monitor_object
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup monitor
 */
typedef struct RBLXWINmonitor RBLXWINmonitor;

/*! @brief Opaque window object.
 *
 *  Opaque window object.
 *
 *  @see @ref window_object
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
typedef struct RBLXWINwindow RBLXWINwindow;

/*! @brief Opaque cursor object.
 *
 *  Opaque cursor object.
 *
 *  @see @ref cursor_object
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup cursor
 */
typedef struct RBLXWINcursor RBLXWINcursor;

/*! @brief The function signature for error callbacks.
 *
 *  This is the function signature for error callback functions.
 *
 *  @param[in] error An [error code](@ref errors).
 *  @param[in] description A UTF-8 encoded string describing the error.
 *
 *  @sa @ref error_handling
 *  @sa rblxwinSetErrorCallback
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup init
 */
typedef void (* RBLXWINerrorfun)(int,const char*);

/*! @brief The function signature for window position callbacks.
 *
 *  This is the function signature for window position callback functions.
 *
 *  @param[in] window The window that was moved.
 *  @param[in] xpos The new x-coordinate, in screen coordinates, of the
 *  upper-left corner of the client area of the window.
 *  @param[in] ypos The new y-coordinate, in screen coordinates, of the
 *  upper-left corner of the client area of the window.
 *
 *  @sa @ref window_pos
 *  @sa rblxwinSetWindowPosCallback
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
typedef void (* RBLXWINwindowposfun)(RBLXWINwindow*,int,int);

/*! @brief The function signature for window resize callbacks.
 *
 *  This is the function signature for window size callback functions.
 *
 *  @param[in] window The window that was resized.
 *  @param[in] width The new width, in screen coordinates, of the window.
 *  @param[in] height The new height, in screen coordinates, of the window.
 *
 *  @sa @ref window_size
 *  @sa rblxwinSetWindowSizeCallback
 *
 *  @since Added in version 1.0.
 *  @rblxwin Added window handle parameter.
 *
 *  @ingroup window
 */
typedef void (* RBLXWINwindowsizefun)(RBLXWINwindow*,int,int);

/*! @brief The function signature for window close callbacks.
 *
 *  This is the function signature for window close callback functions.
 *
 *  @param[in] window The window that the user attempted to close.
 *
 *  @sa @ref window_close
 *  @sa rblxwinSetWindowCloseCallback
 *
 *  @since Added in version 2.5.
 *  @rblxwin Added window handle parameter.
 *
 *  @ingroup window
 */
typedef void (* RBLXWINwindowclosefun)(RBLXWINwindow*);

/*! @brief The function signature for window content refresh callbacks.
 *
 *  This is the function signature for window refresh callback functions.
 *
 *  @param[in] window The window whose content needs to be refreshed.
 *
 *  @sa @ref window_refresh
 *  @sa rblxwinSetWindowRefreshCallback
 *
 *  @since Added in version 2.5.
 *  @rblxwin Added window handle parameter.
 *
 *  @ingroup window
 */
typedef void (* RBLXWINwindowrefreshfun)(RBLXWINwindow*);

/*! @brief The function signature for window focus/defocus callbacks.
 *
 *  This is the function signature for window focus callback functions.
 *
 *  @param[in] window The window that gained or lost input focus.
 *  @param[in] focused `RBLXWIN_TRUE` if the window was given input focus, or
 *  `RBLXWIN_FALSE` if it lost it.
 *
 *  @sa @ref window_focus
 *  @sa rblxwinSetWindowFocusCallback
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
typedef void (* RBLXWINwindowfocusfun)(RBLXWINwindow*,int);

/*! @brief The function signature for window iconify/restore callbacks.
 *
 *  This is the function signature for window iconify/restore callback
 *  functions.
 *
 *  @param[in] window The window that was iconified or restored.
 *  @param[in] iconified `RBLXWIN_TRUE` if the window was iconified, or
 *  `RBLXWIN_FALSE` if it was restored.
 *
 *  @sa @ref window_iconify
 *  @sa rblxwinSetWindowIconifyCallback
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
typedef void (* RBLXWINwindowiconifyfun)(RBLXWINwindow*,int);

/*! @brief The function signature for framebuffer resize callbacks.
 *
 *  This is the function signature for framebuffer resize callback
 *  functions.
 *
 *  @param[in] window The window whose framebuffer was resized.
 *  @param[in] width The new width, in pixels, of the framebuffer.
 *  @param[in] height The new height, in pixels, of the framebuffer.
 *
 *  @sa @ref window_fbsize
 *  @sa rblxwinSetFramebufferSizeCallback
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
typedef void (* RBLXWINframebuffersizefun)(RBLXWINwindow*,int,int);

/*! @brief The function signature for mouse button callbacks.
 *
 *  This is the function signature for mouse button callback functions.
 *
 *  @param[in] window The window that received the event.
 *  @param[in] button The [mouse button](@ref buttons) that was pressed or
 *  released.
 *  @param[in] action One of `RBLXWIN_PRESS` or `RBLXWIN_RELEASE`.
 *  @param[in] mods Bit field describing which [modifier keys](@ref mods) were
 *  held down.
 *
 *  @sa @ref input_mouse_button
 *  @sa rblxwinSetMouseButtonCallback
 *
 *  @since Added in version 1.0.
 *  @rblxwin Added window handle and modifier mask parameters.
 *
 *  @ingroup input
 */
typedef void (* RBLXWINmousebuttonfun)(RBLXWINwindow*,int,int,int);

/*! @brief The function signature for cursor position callbacks.
 *
 *  This is the function signature for cursor position callback functions.
 *
 *  @param[in] window The window that received the event.
 *  @param[in] xpos The new cursor x-coordinate, relative to the left edge of
 *  the client area.
 *  @param[in] ypos The new cursor y-coordinate, relative to the top edge of the
 *  client area.
 *
 *  @sa @ref cursor_pos
 *  @sa rblxwinSetCursorPosCallback
 *
 *  @since Added in version 3.0.  Replaces `RBLXWINmouseposfun`.
 *
 *  @ingroup input
 */
typedef void (* RBLXWINcursorposfun)(RBLXWINwindow*,double,double);

/*! @brief The function signature for cursor enter/leave callbacks.
 *
 *  This is the function signature for cursor enter/leave callback functions.
 *
 *  @param[in] window The window that received the event.
 *  @param[in] entered `RBLXWIN_TRUE` if the cursor entered the window's client
 *  area, or `RBLXWIN_FALSE` if it left it.
 *
 *  @sa @ref cursor_enter
 *  @sa rblxwinSetCursorEnterCallback
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup input
 */
typedef void (* RBLXWINcursorenterfun)(RBLXWINwindow*,int);

/*! @brief The function signature for scroll callbacks.
 *
 *  This is the function signature for scroll callback functions.
 *
 *  @param[in] window The window that received the event.
 *  @param[in] xoffset The scroll offset along the x-axis.
 *  @param[in] yoffset The scroll offset along the y-axis.
 *
 *  @sa @ref scrolling
 *  @sa rblxwinSetScrollCallback
 *
 *  @since Added in version 3.0.  Replaces `RBLXWINmousewheelfun`.
 *
 *  @ingroup input
 */
typedef void (* RBLXWINscrollfun)(RBLXWINwindow*,double,double);

/*! @brief The function signature for keyboard key callbacks.
 *
 *  This is the function signature for keyboard key callback functions.
 *
 *  @param[in] window The window that received the event.
 *  @param[in] key The [keyboard key](@ref keys) that was pressed or released.
 *  @param[in] scancode The system-specific scancode of the key.
 *  @param[in] action `RBLXWIN_PRESS`, `RBLXWIN_RELEASE` or `RBLXWIN_REPEAT`.
 *  @param[in] mods Bit field describing which [modifier keys](@ref mods) were
 *  held down.
 *
 *  @sa @ref input_key
 *  @sa rblxwinSetKeyCallback
 *
 *  @since Added in version 1.0.
 *  @rblxwin Added window handle, scancode and modifier mask parameters.
 *
 *  @ingroup input
 */
typedef void (* RBLXWINkeyfun)(RBLXWINwindow*,int,int,int,int);

/*! @brief The function signature for Unicode character callbacks.
 *
 *  This is the function signature for Unicode character callback functions.
 *
 *  @param[in] window The window that received the event.
 *  @param[in] codepoint The Unicode code point of the character.
 *
 *  @sa @ref input_char
 *  @sa rblxwinSetCharCallback
 *
 *  @since Added in version 2.4.
 *  @rblxwin Added window handle parameter.
 *
 *  @ingroup input
 */
typedef void (* RBLXWINcharfun)(RBLXWINwindow*,unsigned int);

/*! @brief The function signature for Unicode character with modifiers
 *  callbacks.
 *
 *  This is the function signature for Unicode character with modifiers callback
 *  functions.  It is called for each input character, regardless of what
 *  modifier keys are held down.
 *
 *  @param[in] window The window that received the event.
 *  @param[in] codepoint The Unicode code point of the character.
 *  @param[in] mods Bit field describing which [modifier keys](@ref mods) were
 *  held down.
 *
 *  @sa @ref input_char
 *  @sa rblxwinSetCharModsCallback
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup input
 */
typedef void (* RBLXWINcharmodsfun)(RBLXWINwindow*,unsigned int,int);

/*! @brief The function signature for file drop callbacks.
 *
 *  This is the function signature for file drop callbacks.
 *
 *  @param[in] window The window that received the event.
 *  @param[in] count The number of dropped files.
 *  @param[in] paths The UTF-8 encoded file and/or directory path names.
 *
 *  @sa @ref path_drop
 *  @sa rblxwinSetDropCallback
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup input
 */
typedef void (* RBLXWINdropfun)(RBLXWINwindow*,int,const char**);

/*! @brief The function signature for monitor configuration callbacks.
 *
 *  This is the function signature for monitor configuration callback functions.
 *
 *  @param[in] monitor The monitor that was connected or disconnected.
 *  @param[in] event One of `RBLXWIN_CONNECTED` or `RBLXWIN_DISCONNECTED`.
 *
 *  @sa @ref monitor_event
 *  @sa rblxwinSetMonitorCallback
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup monitor
 */
typedef void (* RBLXWINmonitorfun)(RBLXWINmonitor*,int);

/*! @brief The function signature for joystick configuration callbacks.
 *
 *  This is the function signature for joystick configuration callback
 *  functions.
 *
 *  @param[in] joy The joystick that was connected or disconnected.
 *  @param[in] event One of `RBLXWIN_CONNECTED` or `RBLXWIN_DISCONNECTED`.
 *
 *  @sa @ref joystick_event
 *  @sa rblxwinSetJoystickCallback
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup input
 */
typedef void (* RBLXWINjoystickfun)(int,int);

/*! @brief Video mode type.
 *
 *  This describes a single video mode.
 *
 *  @sa @ref monitor_modes
 *  @sa rblxwinGetVideoMode rblxwinGetVideoModes
 *
 *  @since Added in version 1.0.
 *  @rblxwin Added refresh rate member.
 *
 *  @ingroup monitor
 */
typedef struct RBLXWINvidmode
{
    /*! The width, in screen coordinates, of the video mode.
     */
    int width;
    /*! The height, in screen coordinates, of the video mode.
     */
    int height;
    /*! The bit depth of the red channel of the video mode.
     */
    int redBits;
    /*! The bit depth of the green channel of the video mode.
     */
    int greenBits;
    /*! The bit depth of the blue channel of the video mode.
     */
    int blueBits;
    /*! The refresh rate, in Hz, of the video mode.
     */
    int refreshRate;
} RBLXWINvidmode;

/*! @brief Gamma ramp.
 *
 *  This describes the gamma ramp for a monitor.
 *
 *  @sa @ref monitor_gamma
 *  @sa rblxwinGetGammaRamp rblxwinSetGammaRamp
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup monitor
 */
typedef struct RBLXWINgammaramp
{
    /*! An array of value describing the response of the red channel.
     */
    unsigned short* red;
    /*! An array of value describing the response of the green channel.
     */
    unsigned short* green;
    /*! An array of value describing the response of the blue channel.
     */
    unsigned short* blue;
    /*! The number of elements in each array.
     */
    unsigned int size;
} RBLXWINgammaramp;

/*! @brief Image data.
 *
 *  @sa @ref cursor_custom
 *
 *  @since Added in version 2.1.
 *  @rblxwin Removed format and bytes-per-pixel members.
 */
typedef struct RBLXWINimage
{
    /*! The width, in pixels, of this image.
     */
    int width;
    /*! The height, in pixels, of this image.
     */
    int height;
    /*! The pixel data of this image, arranged left-to-right, top-to-bottom.
     */
    unsigned char* pixels;
} RBLXWINimage;


/*************************************************************************
 * RBLXWIN API functions
 *************************************************************************/

/*! @brief Initializes the RBLXWIN library.
 *
 *  This function initializes the RBLXWIN library.  Before most RBLXWIN functions can
 *  be used, RBLXWIN must be initialized, and before an application terminates RBLXWIN
 *  should be terminated in order to free any resources allocated during or
 *  after initialization.
 *
 *  If this function fails, it calls @ref rblxwinTerminate before returning.  If it
 *  succeeds, you should call @ref rblxwinTerminate before the application exits.
 *
 *  Additional calls to this function after successful initialization but before
 *  termination will return `RBLXWIN_TRUE` immediately.
 *
 *  @return `RBLXWIN_TRUE` if successful, or `RBLXWIN_FALSE` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @remark @osx This function will change the current directory of the
 *  application to the `Contents/Resources` subdirectory of the application's
 *  bundle, if present.  This can be disabled with a
 *  [compile-time option](@ref compile_options_osx).
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref intro_init
 *  @sa rblxwinTerminate
 *
 *  @since Added in version 1.0.
 *
 *  @ingroup init
 */
RBLXWINAPI int rblxwinInit(void);

/*! @brief Terminates the RBLXWIN library.
 *
 *  This function destroys all remaining windows and cursors, restores any
 *  modified gamma ramps and frees any other allocated resources.  Once this
 *  function is called, you must again call @ref rblxwinInit successfully before
 *  you will be able to use most RBLXWIN functions.
 *
 *  If RBLXWIN has been successfully initialized, this function should be called
 *  before the application exits.  If initialization fails, there is no need to
 *  call this function, as it is called by @ref rblxwinInit before it returns
 *  failure.
 *
 *  @errors Possible errors include @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @remark This function may be called before @ref rblxwinInit.
 *
 *  @warning The contexts of any remaining windows must not be current on any
 *  other thread when this function is called.
 *
 *  @reentrancy This function must not be called from a callback.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref intro_init
 *  @sa rblxwinInit
 *
 *  @since Added in version 1.0.
 *
 *  @ingroup init
 */
RBLXWINAPI void rblxwinTerminate(void);

/*! @brief Retrieves the version of the RBLXWIN library.
 *
 *  This function retrieves the major, minor and revision numbers of the RBLXWIN
 *  library.  It is intended for when you are using RBLXWIN as a shared library and
 *  want to ensure that you are using the minimum required version.
 *
 *  Any or all of the version arguments may be `NULL`.
 *
 *  @param[out] major Where to store the major version number, or `NULL`.
 *  @param[out] minor Where to store the minor version number, or `NULL`.
 *  @param[out] rev Where to store the revision number, or `NULL`.
 *
 *  @errors None.
 *
 *  @remark This function may be called before @ref rblxwinInit.
 *
 *  @thread_safety This function may be called from any thread.
 *
 *  @sa @ref intro_version
 *  @sa rblxwinGetVersionString
 *
 *  @since Added in version 1.0.
 *
 *  @ingroup init
 */
RBLXWINAPI void rblxwinGetVersion(int* major, int* minor, int* rev);

/*! @brief Returns a string describing the compile-time configuration.
 *
 *  This function returns the compile-time generated
 *  [version string](@ref intro_version_string) of the RBLXWIN library binary.  It
 *  describes the version, platform, compiler and any platform-specific
 *  compile-time options.  It should not be confused with the OpenGL or OpenGL
 *  ES version string, queried with `glGetString`.
 *
 *  __Do not use the version string__ to parse the RBLXWIN library version.  The
 *  @ref rblxwinGetVersion function provides the version of the running library
 *  binary in numerical format.
 *
 *  @return The ASCII encoded RBLXWIN version string.
 *
 *  @errors None.
 *
 *  @remark This function may be called before @ref rblxwinInit.
 *
 *  @pointer_lifetime The returned string is static and compile-time generated.
 *
 *  @thread_safety This function may be called from any thread.
 *
 *  @sa @ref intro_version
 *  @sa rblxwinGetVersion
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup init
 */
RBLXWINAPI const char* rblxwinGetVersionString(void);

/*! @brief Sets the error callback.
 *
 *  This function sets the error callback, which is called with an error code
 *  and a human-readable description each time a RBLXWIN error occurs.
 *
 *  The error callback is called on the thread where the error occurred.  If you
 *  are using RBLXWIN from multiple threads, your error callback needs to be
 *  written accordingly.
 *
 *  Because the description string may have been generated specifically for that
 *  error, it is not guaranteed to be valid after the callback has returned.  If
 *  you wish to use it after the callback returns, you need to make a copy.
 *
 *  Once set, the error callback remains set even after the library has been
 *  terminated.
 *
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set.
 *
 *  @errors None.
 *
 *  @remark This function may be called before @ref rblxwinInit.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref error_handling
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup init
 */
RBLXWINAPI RBLXWINerrorfun rblxwinSetErrorCallback(RBLXWINerrorfun cbfun);

/*! @brief Returns the currently connected monitors.
 *
 *  This function returns an array of handles for all currently connected
 *  monitors.  The primary monitor is always first in the returned array.  If no
 *  monitors were found, this function returns `NULL`.
 *
 *  @param[out] count Where to store the number of monitors in the returned
 *  array.  This is set to zero if an error occurred.
 *  @return An array of monitor handles, or `NULL` if no monitors were found or
 *  if an [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @pointer_lifetime The returned array is allocated and freed by RBLXWIN.  You
 *  should not free it yourself.  It is guaranteed to be valid only until the
 *  monitor configuration changes or the library is terminated.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref monitor_monitors
 *  @sa @ref monitor_event
 *  @sa rblxwinGetPrimaryMonitor
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup monitor
 */
RBLXWINAPI RBLXWINmonitor** rblxwinGetMonitors(int* count);

/*! @brief Returns the primary monitor.
 *
 *  This function returns the primary monitor.  This is usually the monitor
 *  where elements like the task bar or global menu bar are located.
 *
 *  @return The primary monitor, or `NULL` if no monitors were found or if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @remark The primary monitor is always first in the array returned by @ref
 *  rblxwinGetMonitors.
 *
 *  @sa @ref monitor_monitors
 *  @sa rblxwinGetMonitors
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup monitor
 */
RBLXWINAPI RBLXWINmonitor* rblxwinGetPrimaryMonitor(void);

/*! @brief Returns the position of the monitor's viewport on the virtual screen.
 *
 *  This function returns the position, in screen coordinates, of the upper-left
 *  corner of the specified monitor.
 *
 *  Any or all of the position arguments may be `NULL`.  If an error occurs, all
 *  non-`NULL` position arguments will be set to zero.
 *
 *  @param[in] monitor The monitor to query.
 *  @param[out] xpos Where to store the monitor x-coordinate, or `NULL`.
 *  @param[out] ypos Where to store the monitor y-coordinate, or `NULL`.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref monitor_properties
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup monitor
 */
RBLXWINAPI void rblxwinGetMonitorPos(RBLXWINmonitor* monitor, int* xpos, int* ypos);

/*! @brief Returns the physical size of the monitor.
 *
 *  This function returns the size, in millimetres, of the display area of the
 *  specified monitor.
 *
 *  Some systems do not provide accurate monitor size information, either
 *  because the monitor
 *  [EDID](https://en.wikipedia.org/wiki/Extended_display_identification_data)
 *  data is incorrect or because the driver does not report it accurately.
 *
 *  Any or all of the size arguments may be `NULL`.  If an error occurs, all
 *  non-`NULL` size arguments will be set to zero.
 *
 *  @param[in] monitor The monitor to query.
 *  @param[out] widthMM Where to store the width, in millimetres, of the
 *  monitor's display area, or `NULL`.
 *  @param[out] heightMM Where to store the height, in millimetres, of the
 *  monitor's display area, or `NULL`.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @remark @win32 calculates the returned physical size from the
 *  current resolution and system DPI instead of querying the monitor EDID data.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref monitor_properties
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup monitor
 */
RBLXWINAPI void rblxwinGetMonitorPhysicalSize(RBLXWINmonitor* monitor, int* widthMM, int* heightMM);

/*! @brief Returns the name of the specified monitor.
 *
 *  This function returns a human-readable name, encoded as UTF-8, of the
 *  specified monitor.  The name typically reflects the make and model of the
 *  monitor and is not guaranteed to be unique among the connected monitors.
 *
 *  @param[in] monitor The monitor to query.
 *  @return The UTF-8 encoded name of the monitor, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @pointer_lifetime The returned string is allocated and freed by RBLXWIN.  You
 *  should not free it yourself.  It is valid until the specified monitor is
 *  disconnected or the library is terminated.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref monitor_properties
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup monitor
 */
RBLXWINAPI const char* rblxwinGetMonitorName(RBLXWINmonitor* monitor);

/*! @brief Sets the monitor configuration callback.
 *
 *  This function sets the monitor configuration callback, or removes the
 *  currently set callback.  This is called when a monitor is connected to or
 *  disconnected from the system.
 *
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref monitor_event
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup monitor
 */
RBLXWINAPI RBLXWINmonitorfun rblxwinSetMonitorCallback(RBLXWINmonitorfun cbfun);

/*! @brief Returns the available video modes for the specified monitor.
 *
 *  This function returns an array of all video modes supported by the specified
 *  monitor.  The returned array is sorted in ascending order, first by color
 *  bit depth (the sum of all channel depths) and then by resolution area (the
 *  product of width and height).
 *
 *  @param[in] monitor The monitor to query.
 *  @param[out] count Where to store the number of video modes in the returned
 *  array.  This is set to zero if an error occurred.
 *  @return An array of video modes, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @pointer_lifetime The returned array is allocated and freed by RBLXWIN.  You
 *  should not free it yourself.  It is valid until the specified monitor is
 *  disconnected, this function is called again for that monitor or the library
 *  is terminated.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref monitor_modes
 *  @sa rblxwinGetVideoMode
 *
 *  @since Added in version 1.0.
 *  @rblxwin Changed to return an array of modes for a specific monitor.
 *
 *  @ingroup monitor
 */
RBLXWINAPI const RBLXWINvidmode* rblxwinGetVideoModes(RBLXWINmonitor* monitor, int* count);

/*! @brief Returns the current mode of the specified monitor.
 *
 *  This function returns the current video mode of the specified monitor.  If
 *  you have created a full screen window for that monitor, the return value
 *  will depend on whether that window is iconified.
 *
 *  @param[in] monitor The monitor to query.
 *  @return The current mode of the monitor, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @pointer_lifetime The returned array is allocated and freed by RBLXWIN.  You
 *  should not free it yourself.  It is valid until the specified monitor is
 *  disconnected or the library is terminated.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref monitor_modes
 *  @sa rblxwinGetVideoModes
 *
 *  @since Added in version 3.0.  Replaces `rblxwinGetDesktopMode`.
 *
 *  @ingroup monitor
 */
RBLXWINAPI const RBLXWINvidmode* rblxwinGetVideoMode(RBLXWINmonitor* monitor);

/*! @brief Generates a gamma ramp and sets it for the specified monitor.
 *
 *  This function generates a 256-element gamma ramp from the specified exponent
 *  and then calls @ref rblxwinSetGammaRamp with it.  The value must be a finite
 *  number greater than zero.
 *
 *  @param[in] monitor The monitor whose gamma ramp to set.
 *  @param[in] gamma The desired exponent.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_INVALID_VALUE and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref monitor_gamma
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup monitor
 */
RBLXWINAPI void rblxwinSetGamma(RBLXWINmonitor* monitor, float gamma);

/*! @brief Returns the current gamma ramp for the specified monitor.
 *
 *  This function returns the current gamma ramp of the specified monitor.
 *
 *  @param[in] monitor The monitor to query.
 *  @return The current gamma ramp, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @pointer_lifetime The returned structure and its arrays are allocated and
 *  freed by RBLXWIN.  You should not free them yourself.  They are valid until the
 *  specified monitor is disconnected, this function is called again for that
 *  monitor or the library is terminated.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref monitor_gamma
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup monitor
 */
RBLXWINAPI const RBLXWINgammaramp* rblxwinGetGammaRamp(RBLXWINmonitor* monitor);

/*! @brief Sets the current gamma ramp for the specified monitor.
 *
 *  This function sets the current gamma ramp for the specified monitor.  The
 *  original gamma ramp for that monitor is saved by RBLXWIN the first time this
 *  function is called and is restored by @ref rblxwinTerminate.
 *
 *  @param[in] monitor The monitor whose gamma ramp to set.
 *  @param[in] ramp The gamma ramp to use.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @remark Gamma ramp sizes other than 256 are not supported by all platforms
 *  or graphics hardware.
 *
 *  @remark @win32 The gamma ramp size must be 256.
 *
 *  @pointer_lifetime The specified gamma ramp is copied before this function
 *  returns.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref monitor_gamma
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup monitor
 */
RBLXWINAPI void rblxwinSetGammaRamp(RBLXWINmonitor* monitor, const RBLXWINgammaramp* ramp);

/*! @brief Resets all window hints to their default values.
 *
 *  This function resets all window hints to their
 *  [default values](@ref window_hints_values).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_hints
 *  @sa rblxwinWindowHint
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinDefaultWindowHints(void);

/*! @brief Sets the specified window hint to the desired value.
 *
 *  This function sets hints for the next call to @ref rblxwinCreateWindow.  The
 *  hints, once set, retain their values until changed by a call to @ref
 *  rblxwinWindowHint or @ref rblxwinDefaultWindowHints, or until the library is
 *  terminated.
 *
 *  This function does not check whether the specified hint values are valid.
 *  If you set hints to invalid values this will instead be reported by the next
 *  call to @ref rblxwinCreateWindow.
 *
 *  @param[in] hint The [window hint](@ref window_hints) to set.
 *  @param[in] value The new value of the window hint.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_INVALID_ENUM.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_hints
 *  @sa rblxwinDefaultWindowHints
 *
 *  @since Added in version 3.0.  Replaces `rblxwinOpenWindowHint`.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinWindowHint(int hint, int value);

/*! @brief Creates a window and its associated context.
 *
 *  This function creates a window and its associated OpenGL or OpenGL ES
 *  context.  Most of the options controlling how the window and its context
 *  should be created are specified with [window hints](@ref window_hints).
 *
 *  Successful creation does not change which context is current.  Before you
 *  can use the newly created context, you need to
 *  [make it current](@ref context_current).  For information about the `share`
 *  parameter, see @ref context_sharing.
 *
 *  The created window, framebuffer and context may differ from what you
 *  requested, as not all parameters and hints are
 *  [hard constraints](@ref window_hints_hard).  This includes the size of the
 *  window, especially for full screen windows.  To query the actual attributes
 *  of the created window, framebuffer and context, see @ref
 *  rblxwinGetWindowAttrib, @ref rblxwinGetWindowSize and @ref rblxwinGetFramebufferSize.
 *
 *  To create a full screen window, you need to specify the monitor the window
 *  will cover.  If no monitor is specified, the window will be windowed mode.
 *  Unless you have a way for the user to choose a specific monitor, it is
 *  recommended that you pick the primary monitor.  For more information on how
 *  to query connected monitors, see @ref monitor_monitors.
 *
 *  For full screen windows, the specified size becomes the resolution of the
 *  window's _desired video mode_.  As long as a full screen window is not
 *  iconified, the supported video mode most closely matching the desired video
 *  mode is set for the specified monitor.  For more information about full
 *  screen windows, including the creation of so called _windowed full screen_
 *  or _borderless full screen_ windows, see @ref window_windowed_full_screen.
 *
 *  By default, newly created windows use the placement recommended by the
 *  window system.  To create the window at a specific position, make it
 *  initially invisible using the [RBLXWIN_VISIBLE](@ref window_hints_wnd) window
 *  hint, set its [position](@ref window_pos) and then [show](@ref window_hide)
 *  it.
 *
 *  As long as at least one full screen window is not iconified, the screensaver
 *  is prohibited from starting.
 *
 *  Window systems put limits on window sizes.  Very large or very small window
 *  dimensions may be overridden by the window system on creation.  Check the
 *  actual [size](@ref window_size) after creation.
 *
 *  The [swap interval](@ref buffer_swap) is not set during window creation and
 *  the initial value may vary depending on driver settings and defaults.
 *
 *  @param[in] width The desired width, in screen coordinates, of the window.
 *  This must be greater than zero.
 *  @param[in] height The desired height, in screen coordinates, of the window.
 *  This must be greater than zero.
 *  @param[in] title The initial, UTF-8 encoded window title.
 *  @param[in] monitor The monitor to use for full screen mode, or `NULL` for
 *  windowed mode.
 *  @param[in] share The window whose context to share resources with, or `NULL`
 *  to not share resources.
 *  @return The handle of the created window, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_INVALID_ENUM, @ref RBLXWIN_INVALID_VALUE, @ref RBLXWIN_API_UNAVAILABLE, @ref
 *  RBLXWIN_VERSION_UNAVAILABLE, @ref RBLXWIN_FORMAT_UNAVAILABLE and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @remark @win32 Window creation will fail if the Microsoft GDI software
 *  OpenGL implementation is the only one available.
 *
 *  @remark @win32 If the executable has an icon resource named `RBLXWIN_ICON,`
 *  it will be set as the icon for the window.  If no such icon is present, the
 *  `IDI_WINLOGO` icon will be used instead.
 *
 *  @remark @win32 The context to share resources with must not be current on
 *  any other thread.
 *
 *  @remark @osx The RBLXWIN window has no icon, as it is not a document
 *  window, but the dock icon will be the same as the application bundle's icon.
 *  For more information on bundles, see the
 *  [Bundle Programming Guide](https://developer.apple.com/library/mac/documentation/CoreFoundation/Conceptual/CFBundles/)
 *  in the Mac Developer Library.
 *
 *  @remark @osx The first time a window is created the menu bar is populated
 *  with common commands like Hide, Quit and About.  The About entry opens
 *  a minimal about dialog with information from the application's bundle.  The
 *  menu bar can be disabled with a
 *  [compile-time option](@ref compile_options_osx).
 *
 *  @remark @osx On OS X 10.10 and later the window frame will not be rendered
 *  at full resolution on Retina displays unless the `NSHighResolutionCapable`
 *  key is enabled in the application bundle's `Info.plist`.  For more
 *  information, see
 *  [High Resolution Guidelines for OS X](https://developer.apple.com/library/mac/documentation/GraphicsAnimation/Conceptual/HighResolutionOSX/Explained/Explained.html)
 *  in the Mac Developer Library.  The RBLXWIN test and example programs use
 *  a custom `Info.plist` template for this, which can be found as
 *  `CMake/MacOSXBundleInfo.plist.in` in the source tree.
 *
 *  @remark @x11 There is no mechanism for setting the window icon yet.
 *
 *  @remark @x11 Some window managers will not respect the placement of
 *  initially hidden windows.
 *
 *  @remark @x11 Due to the asynchronous nature of X11, it may take a moment for
 *  a window to reach its requested state.  This means you may not be able to
 *  query the final size, position or other attributes directly after window
 *  creation.
 *
 *  @reentrancy This function must not be called from a callback.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_creation
 *  @sa rblxwinDestroyWindow
 *
 *  @since Added in version 3.0.  Replaces `rblxwinOpenWindow`.
 *
 *  @ingroup window
 */
RBLXWINAPI RBLXWINwindow* rblxwinCreateWindow(int width, int height, const char* title, RBLXWINmonitor* monitor, RBLXWINwindow* share);

/*! @brief Destroys the specified window and its context.
 *
 *  This function destroys the specified window and its context.  On calling
 *  this function, no further callbacks will be called for that window.
 *
 *  If the context of the specified window is current on the main thread, it is
 *  detached before being destroyed.
 *
 *  @param[in] window The window to destroy.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @note The context of the specified window must not be current on any other
 *  thread when this function is called.
 *
 *  @reentrancy This function must not be called from a callback.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_creation
 *  @sa rblxwinCreateWindow
 *
 *  @since Added in version 3.0.  Replaces `rblxwinCloseWindow`.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinDestroyWindow(RBLXWINwindow* window);

/*! @brief Checks the close flag of the specified window.
 *
 *  This function returns the value of the close flag of the specified window.
 *
 *  @param[in] window The window to query.
 *  @return The value of the close flag.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @sa @ref window_close
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
RBLXWINAPI int rblxwinWindowShouldClose(RBLXWINwindow* window);

/*! @brief Sets the close flag of the specified window.
 *
 *  This function sets the value of the close flag of the specified window.
 *  This can be used to override the user's attempt to close the window, or
 *  to signal that it should be closed.
 *
 *  @param[in] window The window whose flag to change.
 *  @param[in] value The new value.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @sa @ref window_close
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinSetWindowShouldClose(RBLXWINwindow* window, int value);

/*! @brief Sets the title of the specified window.
 *
 *  This function sets the window title, encoded as UTF-8, of the specified
 *  window.
 *
 *  @param[in] window The window whose title to change.
 *  @param[in] title The UTF-8 encoded window title.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @remark @osx The window title will not be updated until the next time you
 *  process events.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_title
 *
 *  @since Added in version 1.0.
 *  @rblxwin Added window handle parameter.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinSetWindowTitle(RBLXWINwindow* window, const char* title);

/*! @brief Sets the icon for the specified window.
 *
 *  This function sets the icon of the specified window.  If passed an array of
 *  candidate images, those of or closest to the sizes desired by the system are
 *  selected.  If no images are specified, the window reverts to its default
 *  icon.
 *
 *  The desired image sizes varies depending on platform and system settings.
 *  The selected images will be rescaled as needed.  Good sizes include 16x16,
 *  32x32 and 48x48.
 *
 *  @param[in] window The window whose icon to set.
 *  @param[in] count The number of images in the specified array, or zero to
 *  revert to the default window icon.
 *  @param[in] images The images to create the icon from.  This is ignored if
 *  count is zero.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @pointer_lifetime The specified image data is copied before this function
 *  returns.
 *
 *  @remark @osx The RBLXWIN window has no icon, as it is not a document
 *  window, but the dock icon will be the same as the application bundle's icon.
 *  For more information on bundles, see the
 *  [Bundle Programming Guide](https://developer.apple.com/library/mac/documentation/CoreFoundation/Conceptual/CFBundles/)
 *  in the Mac Developer Library.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_icon
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinSetWindowIcon(RBLXWINwindow* window, int count, const RBLXWINimage* images);

/*! @brief Retrieves the position of the client area of the specified window.
 *
 *  This function retrieves the position, in screen coordinates, of the
 *  upper-left corner of the client area of the specified window.
 *
 *  Any or all of the position arguments may be `NULL`.  If an error occurs, all
 *  non-`NULL` position arguments will be set to zero.
 *
 *  @param[in] window The window to query.
 *  @param[out] xpos Where to store the x-coordinate of the upper-left corner of
 *  the client area, or `NULL`.
 *  @param[out] ypos Where to store the y-coordinate of the upper-left corner of
 *  the client area, or `NULL`.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_pos
 *  @sa rblxwinSetWindowPos
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinGetWindowPos(RBLXWINwindow* window, int* xpos, int* ypos);

/*! @brief Sets the position of the client area of the specified window.
 *
 *  This function sets the position, in screen coordinates, of the upper-left
 *  corner of the client area of the specified windowed mode window.  If the
 *  window is a full screen window, this function does nothing.
 *
 *  __Do not use this function__ to move an already visible window unless you
 *  have very good reasons for doing so, as it will confuse and annoy the user.
 *
 *  The window manager may put limits on what positions are allowed.  RBLXWIN
 *  cannot and should not override these limits.
 *
 *  @param[in] window The window to query.
 *  @param[in] xpos The x-coordinate of the upper-left corner of the client area.
 *  @param[in] ypos The y-coordinate of the upper-left corner of the client area.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_pos
 *  @sa rblxwinGetWindowPos
 *
 *  @since Added in version 1.0.
 *  @rblxwin Added window handle parameter.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinSetWindowPos(RBLXWINwindow* window, int xpos, int ypos);

/*! @brief Retrieves the size of the client area of the specified window.
 *
 *  This function retrieves the size, in screen coordinates, of the client area
 *  of the specified window.  If you wish to retrieve the size of the
 *  framebuffer of the window in pixels, see @ref rblxwinGetFramebufferSize.
 *
 *  Any or all of the size arguments may be `NULL`.  If an error occurs, all
 *  non-`NULL` size arguments will be set to zero.
 *
 *  @param[in] window The window whose size to retrieve.
 *  @param[out] width Where to store the width, in screen coordinates, of the
 *  client area, or `NULL`.
 *  @param[out] height Where to store the height, in screen coordinates, of the
 *  client area, or `NULL`.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_size
 *  @sa rblxwinSetWindowSize
 *
 *  @since Added in version 1.0.
 *  @rblxwin Added window handle parameter.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinGetWindowSize(RBLXWINwindow* window, int* width, int* height);

/*! @brief Sets the size limits of the specified window.
 *
 *  This function sets the size limits of the client area of the specified
 *  window.  If the window is full screen, the size limits only take effect if
 *  once it is made windowed.  If the window is not resizable, this function
 *  does nothing.
 *
 *  The size limits are applied immediately to a windowed mode window and may
 *  cause it to be resized.
 *
 *  @param[in] window The window to set limits for.
 *  @param[in] minwidth The minimum width, in screen coordinates, of the client
 *  area, or `RBLXWIN_DONT_CARE`.
 *  @param[in] minheight The minimum height, in screen coordinates, of the
 *  client area, or `RBLXWIN_DONT_CARE`.
 *  @param[in] maxwidth The maximum width, in screen coordinates, of the client
 *  area, or `RBLXWIN_DONT_CARE`.
 *  @param[in] maxheight The maximum height, in screen coordinates, of the
 *  client area, or `RBLXWIN_DONT_CARE`.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @remark If you set size limits and an aspect ratio that conflict, the
 *  results are undefined.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_sizelimits
 *  @sa rblxwinSetWindowAspectRatio
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinSetWindowSizeLimits(RBLXWINwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);

/*! @brief Sets the aspect ratio of the specified window.
 *
 *  This function sets the required aspect ratio of the client area of the
 *  specified window.  If the window is full screen, the aspect ratio only takes
 *  effect once it is made windowed.  If the window is not resizable, this
 *  function does nothing.
 *
 *  The aspect ratio is specified as a numerator and a denominator and both
 *  values must be greater than zero.  For example, the common 16:9 aspect ratio
 *  is specified as 16 and 9, respectively.
 *
 *  If the numerator and denominator is set to `RBLXWIN_DONT_CARE` then the aspect
 *  ratio limit is disabled.
 *
 *  The aspect ratio is applied immediately to a windowed mode window and may
 *  cause it to be resized.
 *
 *  @param[in] window The window to set limits for.
 *  @param[in] numer The numerator of the desired aspect ratio, or
 *  `RBLXWIN_DONT_CARE`.
 *  @param[in] denom The denominator of the desired aspect ratio, or
 *  `RBLXWIN_DONT_CARE`.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_INVALID_VALUE and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @remark If you set size limits and an aspect ratio that conflict, the
 *  results are undefined.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_sizelimits
 *  @sa rblxwinSetWindowSizeLimits
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinSetWindowAspectRatio(RBLXWINwindow* window, int numer, int denom);

/*! @brief Sets the size of the client area of the specified window.
 *
 *  This function sets the size, in screen coordinates, of the client area of
 *  the specified window.
 *
 *  For full screen windows, this function updates the resolution of its desired
 *  video mode and switches to the video mode closest to it, without affecting
 *  the window's context.  As the context is unaffected, the bit depths of the
 *  framebuffer remain unchanged.
 *
 *  If you wish to update the refresh rate of the desired video mode in addition
 *  to its resolution, see @ref rblxwinSetWindowMonitor.
 *
 *  The window manager may put limits on what sizes are allowed.  RBLXWIN cannot
 *  and should not override these limits.
 *
 *  @param[in] window The window to resize.
 *  @param[in] width The desired width, in screen coordinates, of the window
 *  client area.
 *  @param[in] height The desired height, in screen coordinates, of the window
 *  client area.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_size
 *  @sa rblxwinGetWindowSize
 *  @sa rblxwinSetWindowMonitor
 *
 *  @since Added in version 1.0.
 *  @rblxwin Added window handle parameter.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinSetWindowSize(RBLXWINwindow* window, int width, int height);

/*! @brief Retrieves the size of the framebuffer of the specified window.
 *
 *  This function retrieves the size, in pixels, of the framebuffer of the
 *  specified window.  If you wish to retrieve the size of the window in screen
 *  coordinates, see @ref rblxwinGetWindowSize.
 *
 *  Any or all of the size arguments may be `NULL`.  If an error occurs, all
 *  non-`NULL` size arguments will be set to zero.
 *
 *  @param[in] window The window whose framebuffer to query.
 *  @param[out] width Where to store the width, in pixels, of the framebuffer,
 *  or `NULL`.
 *  @param[out] height Where to store the height, in pixels, of the framebuffer,
 *  or `NULL`.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_fbsize
 *  @sa rblxwinSetFramebufferSizeCallback
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinGetFramebufferSize(RBLXWINwindow* window, int* width, int* height);

/*! @brief Retrieves the size of the frame of the window.
 *
 *  This function retrieves the size, in screen coordinates, of each edge of the
 *  frame of the specified window.  This size includes the title bar, if the
 *  window has one.  The size of the frame may vary depending on the
 *  [window-related hints](@ref window_hints_wnd) used to create it.
 *
 *  Because this function retrieves the size of each window frame edge and not
 *  the offset along a particular coordinate axis, the retrieved values will
 *  always be zero or positive.
 *
 *  Any or all of the size arguments may be `NULL`.  If an error occurs, all
 *  non-`NULL` size arguments will be set to zero.
 *
 *  @param[in] window The window whose frame size to query.
 *  @param[out] left Where to store the size, in screen coordinates, of the left
 *  edge of the window frame, or `NULL`.
 *  @param[out] top Where to store the size, in screen coordinates, of the top
 *  edge of the window frame, or `NULL`.
 *  @param[out] right Where to store the size, in screen coordinates, of the
 *  right edge of the window frame, or `NULL`.
 *  @param[out] bottom Where to store the size, in screen coordinates, of the
 *  bottom edge of the window frame, or `NULL`.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_size
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinGetWindowFrameSize(RBLXWINwindow* window, int* left, int* top, int* right, int* bottom);

/*! @brief Iconifies the specified window.
 *
 *  This function iconifies (minimizes) the specified window if it was
 *  previously restored.  If the window is already iconified, this function does
 *  nothing.
 *
 *  If the specified window is a full screen window, the original monitor
 *  resolution is restored until the window is restored.
 *
 *  @param[in] window The window to iconify.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_iconify
 *  @sa rblxwinRestoreWindow
 *  @sa rblxwinMaximizeWindow
 *
 *  @since Added in version 2.1.
 *  @rblxwin Added window handle parameter.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinIconifyWindow(RBLXWINwindow* window);

/*! @brief Restores the specified window.
 *
 *  This function restores the specified window if it was previously iconified
 *  (minimized) or maximized.  If the window is already restored, this function
 *  does nothing.
 *
 *  If the specified window is a full screen window, the resolution chosen for
 *  the window is restored on the selected monitor.
 *
 *  @param[in] window The window to restore.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_iconify
 *  @sa rblxwinIconifyWindow
 *  @sa rblxwinMaximizeWindow
 *
 *  @since Added in version 2.1.
 *  @rblxwin Added window handle parameter.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinRestoreWindow(RBLXWINwindow* window);

/*! @brief Maximizes the specified window.
 *
 *  This function maximizes the specified window if it was previously not
 *  maximized.  If the window is already maximized, this function does nothing.
 *
 *  If the specified window is a full screen window, this function does nothing.
 *
 *  @param[in] window The window to maximize.
 *
 *  @par Thread Safety
 *  This function may only be called from the main thread.
 *
 *  @sa @ref window_iconify
 *  @sa rblxwinIconifyWindow
 *  @sa rblxwinRestoreWindow
 *
 *  @since Added in RBLXWIN 3.2.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinMaximizeWindow(RBLXWINwindow* window);

/*! @brief Makes the specified window visible.
 *
 *  This function makes the specified window visible if it was previously
 *  hidden.  If the window is already visible or is in full screen mode, this
 *  function does nothing.
 *
 *  @param[in] window The window to make visible.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_hide
 *  @sa rblxwinHideWindow
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinShowWindow(RBLXWINwindow* window);

/*! @brief Hides the specified window.
 *
 *  This function hides the specified window if it was previously visible.  If
 *  the window is already hidden or is in full screen mode, this function does
 *  nothing.
 *
 *  @param[in] window The window to hide.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_hide
 *  @sa rblxwinShowWindow
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinHideWindow(RBLXWINwindow* window);

/*! @brief Brings the specified window to front and sets input focus.
 *
 *  This function brings the specified window to front and sets input focus.
 *  The window should already be visible and not iconified.
 *
 *  By default, both windowed and full screen mode windows are focused when
 *  initially created.  Set the [RBLXWIN_FOCUSED](@ref window_hints_wnd) to disable
 *  this behavior.
 *
 *  __Do not use this function__ to steal focus from other applications unless
 *  you are certain that is what the user wants.  Focus stealing can be
 *  extremely disruptive.
 *
 *  @param[in] window The window to give input focus.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_focus
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinFocusWindow(RBLXWINwindow* window);

/*! @brief Returns the monitor that the window uses for full screen mode.
 *
 *  This function returns the handle of the monitor that the specified window is
 *  in full screen on.
 *
 *  @param[in] window The window to query.
 *  @return The monitor, or `NULL` if the window is in windowed mode or an error
 *  occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_monitor
 *  @sa rblxwinSetWindowMonitor
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
RBLXWINAPI RBLXWINmonitor* rblxwinGetWindowMonitor(RBLXWINwindow* window);

/*! @brief Sets the mode, monitor, video mode and placement of a window.
 *
 *  This function sets the monitor that the window uses for full screen mode or,
 *  if the monitor is `NULL`, makes it windowed mode.
 *
 *  When setting a monitor, this function updates the width, height and refresh
 *  rate of the desired video mode and switches to the video mode closest to it.
 *  The window position is ignored when setting a monitor.
 *
 *  When the monitor is `NULL`, the position, width and height are used to
 *  place the window client area.  The refresh rate is ignored when no monitor
 *  is specified.
 *
 *  If you only wish to update the resolution of a full screen window or the
 *  size of a windowed mode window, see @ref rblxwinSetWindowSize.
 *
 *  When a window transitions from full screen to windowed mode, this function
 *  restores any previous window settings such as whether it is decorated,
 *  floating, resizable, has size or aspect ratio limits, etc..
 *
 *  @param[in] window The window whose monitor, size or video mode to set.
 *  @param[in] monitor The desired monitor, or `NULL` to set windowed mode.
 *  @param[in] xpos The desired x-coordinate of the upper-left corner of the
 *  client area.
 *  @param[in] ypos The desired y-coordinate of the upper-left corner of the
 *  client area.
 *  @param[in] width The desired with, in screen coordinates, of the client area
 *  or video mode.
 *  @param[in] height The desired height, in screen coordinates, of the client
 *  area or video mode.
 *  @param[in] refreshRate The desired refresh rate, in Hz, of the video mode.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_monitor
 *  @sa @ref window_full_screen
 *  @sa rblxwinGetWindowMonitor
 *  @sa rblxwinSetWindowSize
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinSetWindowMonitor(RBLXWINwindow* window, RBLXWINmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate);

/*! @brief Returns an attribute of the specified window.
 *
 *  This function returns the value of an attribute of the specified window or
 *  its OpenGL or OpenGL ES context.
 *
 *  @param[in] window The window to query.
 *  @param[in] attrib The [window attribute](@ref window_attribs) whose value to
 *  return.
 *  @return The value of the attribute, or zero if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_INVALID_ENUM and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @remark Framebuffer related hints are not window attributes.  See @ref
 *  window_attribs_fb for more information.
 *
 *  @remark Zero is a valid value for many window and context related
 *  attributes so you cannot use a return value of zero as an indication of
 *  errors.  However, this function should not fail as long as it is passed
 *  valid arguments and the library has been [initialized](@ref intro_init).
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_attribs
 *
 *  @since Added in version 3.0.  Replaces `rblxwinGetWindowParam` and
 *  `rblxwinGetGLVersion`.
 *
 *  @ingroup window
 */
RBLXWINAPI int rblxwinGetWindowAttrib(RBLXWINwindow* window, int attrib);

/*! @brief Sets the user pointer of the specified window.
 *
 *  This function sets the user-defined pointer of the specified window.  The
 *  current value is retained until the window is destroyed.  The initial value
 *  is `NULL`.
 *
 *  @param[in] window The window whose pointer to set.
 *  @param[in] pointer The new value.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @sa @ref window_userptr
 *  @sa rblxwinGetWindowUserPointer
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinSetWindowUserPointer(RBLXWINwindow* window, void* pointer);

/*! @brief Returns the user pointer of the specified window.
 *
 *  This function returns the current value of the user-defined pointer of the
 *  specified window.  The initial value is `NULL`.
 *
 *  @param[in] window The window whose pointer to return.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @sa @ref window_userptr
 *  @sa rblxwinSetWindowUserPointer
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
RBLXWINAPI void* rblxwinGetWindowUserPointer(RBLXWINwindow* window);

/*! @brief Sets the position callback for the specified window.
 *
 *  This function sets the position callback of the specified window, which is
 *  called when the window is moved.  The callback is provided with the screen
 *  position of the upper-left corner of the client area of the window.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_pos
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
RBLXWINAPI RBLXWINwindowposfun rblxwinSetWindowPosCallback(RBLXWINwindow* window, RBLXWINwindowposfun cbfun);

/*! @brief Sets the size callback for the specified window.
 *
 *  This function sets the size callback of the specified window, which is
 *  called when the window is resized.  The callback is provided with the size,
 *  in screen coordinates, of the client area of the window.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_size
 *
 *  @since Added in version 1.0.
 *  @rblxwin Added window handle parameter and return value.
 *
 *  @ingroup window
 */
RBLXWINAPI RBLXWINwindowsizefun rblxwinSetWindowSizeCallback(RBLXWINwindow* window, RBLXWINwindowsizefun cbfun);

/*! @brief Sets the close callback for the specified window.
 *
 *  This function sets the close callback of the specified window, which is
 *  called when the user attempts to close the window, for example by clicking
 *  the close widget in the title bar.
 *
 *  The close flag is set before this callback is called, but you can modify it
 *  at any time with @ref rblxwinSetWindowShouldClose.
 *
 *  The close callback is not triggered by @ref rblxwinDestroyWindow.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @remark @osx Selecting Quit from the application menu will trigger the close
 *  callback for all windows.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_close
 *
 *  @since Added in version 2.5.
 *  @rblxwin Added window handle parameter and return value.
 *
 *  @ingroup window
 */
RBLXWINAPI RBLXWINwindowclosefun rblxwinSetWindowCloseCallback(RBLXWINwindow* window, RBLXWINwindowclosefun cbfun);

/*! @brief Sets the refresh callback for the specified window.
 *
 *  This function sets the refresh callback of the specified window, which is
 *  called when the client area of the window needs to be redrawn, for example
 *  if the window has been exposed after having been covered by another window.
 *
 *  On compositing window systems such as Aero, Compiz or Aqua, where the window
 *  contents are saved off-screen, this callback may be called only very
 *  infrequently or never at all.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_refresh
 *
 *  @since Added in version 2.5.
 *  @rblxwin Added window handle parameter and return value.
 *
 *  @ingroup window
 */
RBLXWINAPI RBLXWINwindowrefreshfun rblxwinSetWindowRefreshCallback(RBLXWINwindow* window, RBLXWINwindowrefreshfun cbfun);

/*! @brief Sets the focus callback for the specified window.
 *
 *  This function sets the focus callback of the specified window, which is
 *  called when the window gains or loses input focus.
 *
 *  After the focus callback is called for a window that lost input focus,
 *  synthetic key and mouse button release events will be generated for all such
 *  that had been pressed.  For more information, see @ref rblxwinSetKeyCallback
 *  and @ref rblxwinSetMouseButtonCallback.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_focus
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
RBLXWINAPI RBLXWINwindowfocusfun rblxwinSetWindowFocusCallback(RBLXWINwindow* window, RBLXWINwindowfocusfun cbfun);

/*! @brief Sets the iconify callback for the specified window.
 *
 *  This function sets the iconification callback of the specified window, which
 *  is called when the window is iconified or restored.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_iconify
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
RBLXWINAPI RBLXWINwindowiconifyfun rblxwinSetWindowIconifyCallback(RBLXWINwindow* window, RBLXWINwindowiconifyfun cbfun);

/*! @brief Sets the framebuffer resize callback for the specified window.
 *
 *  This function sets the framebuffer resize callback of the specified window,
 *  which is called when the framebuffer of the specified window is resized.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref window_fbsize
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup window
 */
RBLXWINAPI RBLXWINframebuffersizefun rblxwinSetFramebufferSizeCallback(RBLXWINwindow* window, RBLXWINframebuffersizefun cbfun);

/*! @brief Processes all pending events.
 *
 *  This function processes only those events that are already in the event
 *  queue and then returns immediately.  Processing events will cause the window
 *  and input callbacks associated with those events to be called.
 *
 *  On some platforms, a window move, resize or menu operation will cause event
 *  processing to block.  This is due to how event processing is designed on
 *  those platforms.  You can use the
 *  [window refresh callback](@ref window_refresh) to redraw the contents of
 *  your window when necessary during such operations.
 *
 *  On some platforms, certain events are sent directly to the application
 *  without going through the event queue, causing callbacks to be called
 *  outside of a call to one of the event processing functions.
 *
 *  Event processing is not required for joystick input to work.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @reentrancy This function must not be called from a callback.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref events
 *  @sa rblxwinWaitEvents
 *  @sa rblxwinWaitEventsTimeout
 *
 *  @since Added in version 1.0.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinPollEvents(void);

/*! @brief Waits until events are queued and processes them.
 *
 *  This function puts the calling thread to sleep until at least one event is
 *  available in the event queue.  Once one or more events are available,
 *  it behaves exactly like @ref rblxwinPollEvents, i.e. the events in the queue
 *  are processed and the function then returns immediately.  Processing events
 *  will cause the window and input callbacks associated with those events to be
 *  called.
 *
 *  Since not all events are associated with callbacks, this function may return
 *  without a callback having been called even if you are monitoring all
 *  callbacks.
 *
 *  On some platforms, a window move, resize or menu operation will cause event
 *  processing to block.  This is due to how event processing is designed on
 *  those platforms.  You can use the
 *  [window refresh callback](@ref window_refresh) to redraw the contents of
 *  your window when necessary during such operations.
 *
 *  On some platforms, certain callbacks may be called outside of a call to one
 *  of the event processing functions.
 *
 *  If no windows exist, this function returns immediately.  For synchronization
 *  of threads in applications that do not create windows, use your threading
 *  library of choice.
 *
 *  Event processing is not required for joystick input to work.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @reentrancy This function must not be called from a callback.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref events
 *  @sa rblxwinPollEvents
 *  @sa rblxwinWaitEventsTimeout
 *
 *  @since Added in version 2.5.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinWaitEvents(void);

/*! @brief Waits with timeout until events are queued and processes them.
 *
 *  This function puts the calling thread to sleep until at least one event is
 *  available in the event queue, or until the specified timeout is reached.  If
 *  one or more events are available, it behaves exactly like @ref
 *  rblxwinPollEvents, i.e. the events in the queue are processed and the function
 *  then returns immediately.  Processing events will cause the window and input
 *  callbacks associated with those events to be called.
 *
 *  The timeout value must be a positive finite number.
 *
 *  Since not all events are associated with callbacks, this function may return
 *  without a callback having been called even if you are monitoring all
 *  callbacks.
 *
 *  On some platforms, a window move, resize or menu operation will cause event
 *  processing to block.  This is due to how event processing is designed on
 *  those platforms.  You can use the
 *  [window refresh callback](@ref window_refresh) to redraw the contents of
 *  your window when necessary during such operations.
 *
 *  On some platforms, certain callbacks may be called outside of a call to one
 *  of the event processing functions.
 *
 *  If no windows exist, this function returns immediately.  For synchronization
 *  of threads in applications that do not create windows, use your threading
 *  library of choice.
 *
 *  Event processing is not required for joystick input to work.
 *
 *  @param[in] timeout The maximum amount of time, in seconds, to wait.
 *
 *  @reentrancy This function must not be called from a callback.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref events
 *  @sa rblxwinPollEvents
 *  @sa rblxwinWaitEvents
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinWaitEventsTimeout(double timeout);

/*! @brief Posts an empty event to the event queue.
 *
 *  This function posts an empty event from the current thread to the event
 *  queue, causing @ref rblxwinWaitEvents to return.
 *
 *  If no windows exist, this function returns immediately.  For synchronization
 *  of threads in applications that do not create windows, use your threading
 *  library of choice.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function may be called from any thread.
 *
 *  @sa @ref events
 *  @sa rblxwinWaitEvents
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinPostEmptyEvent(void);

/*! @brief Returns the value of an input option for the specified window.
 *
 *  This function returns the value of an input option for the specified window.
 *  The mode must be one of `RBLXWIN_CURSOR`, `RBLXWIN_STICKY_KEYS` or
 *  `RBLXWIN_STICKY_MOUSE_BUTTONS`.
 *
 *  @param[in] window The window to query.
 *  @param[in] mode One of `RBLXWIN_CURSOR`, `RBLXWIN_STICKY_KEYS` or
 *  `RBLXWIN_STICKY_MOUSE_BUTTONS`.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_INVALID_ENUM.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa rblxwinSetInputMode
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup input
 */
RBLXWINAPI int rblxwinGetInputMode(RBLXWINwindow* window, int mode);

/*! @brief Sets an input option for the specified window.
 *
 *  This function sets an input mode option for the specified window.  The mode
 *  must be one of `RBLXWIN_CURSOR`, `RBLXWIN_STICKY_KEYS` or
 *  `RBLXWIN_STICKY_MOUSE_BUTTONS`.
 *
 *  If the mode is `RBLXWIN_CURSOR`, the value must be one of the following cursor
 *  modes:
 *  - `RBLXWIN_CURSOR_NORMAL` makes the cursor visible and behaving normally.
 *  - `RBLXWIN_CURSOR_HIDDEN` makes the cursor invisible when it is over the client
 *    area of the window but does not restrict the cursor from leaving.
 *  - `RBLXWIN_CURSOR_DISABLED` hides and grabs the cursor, providing virtual
 *    and unlimited cursor movement.  This is useful for implementing for
 *    example 3D camera controls.
 *
 *  If the mode is `RBLXWIN_STICKY_KEYS`, the value must be either `RBLXWIN_TRUE` to
 *  enable sticky keys, or `RBLXWIN_FALSE` to disable it.  If sticky keys are
 *  enabled, a key press will ensure that @ref rblxwinGetKey returns `RBLXWIN_PRESS`
 *  the next time it is called even if the key had been released before the
 *  call.  This is useful when you are only interested in whether keys have been
 *  pressed but not when or in which order.
 *
 *  If the mode is `RBLXWIN_STICKY_MOUSE_BUTTONS`, the value must be either
 *  `RBLXWIN_TRUE` to enable sticky mouse buttons, or `RBLXWIN_FALSE` to disable it.
 *  If sticky mouse buttons are enabled, a mouse button press will ensure that
 *  @ref rblxwinGetMouseButton returns `RBLXWIN_PRESS` the next time it is called even
 *  if the mouse button had been released before the call.  This is useful when
 *  you are only interested in whether mouse buttons have been pressed but not
 *  when or in which order.
 *
 *  @param[in] window The window whose input mode to set.
 *  @param[in] mode One of `RBLXWIN_CURSOR`, `RBLXWIN_STICKY_KEYS` or
 *  `RBLXWIN_STICKY_MOUSE_BUTTONS`.
 *  @param[in] value The new value of the specified input mode.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_INVALID_ENUM and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa rblxwinGetInputMode
 *
 *  @since Added in version 3.0.  Replaces `rblxwinEnable` and `rblxwinDisable`.
 *
 *  @ingroup input
 */
RBLXWINAPI void rblxwinSetInputMode(RBLXWINwindow* window, int mode, int value);

/*! @brief Returns the localized name of the specified printable key.
 *
 *  This function returns the localized name of the specified printable key.
 *  This is intended for displaying key bindings to the user.
 *
 *  If the key is `RBLXWIN_KEY_UNKNOWN`, the scancode is used instead, otherwise
 *  the scancode is ignored.  If a non-printable key or (if the key is
 *  `RBLXWIN_KEY_UNKNOWN`) a scancode that maps to a non-printable key is
 *  specified, this function returns `NULL`.          
 *
 *  This behavior allows you to pass in the arguments passed to the
 *  [key callback](@ref input_key) without modification.
 *
 *  The printable keys are:
 *  - `RBLXWIN_KEY_APOSTROPHE`
 *  - `RBLXWIN_KEY_COMMA`
 *  - `RBLXWIN_KEY_MINUS`
 *  - `RBLXWIN_KEY_PERIOD`
 *  - `RBLXWIN_KEY_SLASH`
 *  - `RBLXWIN_KEY_SEMICOLON`
 *  - `RBLXWIN_KEY_EQUAL`
 *  - `RBLXWIN_KEY_LEFT_BRACKET`
 *  - `RBLXWIN_KEY_RIGHT_BRACKET`
 *  - `RBLXWIN_KEY_BACKSLASH`
 *  - `RBLXWIN_KEY_WORLD_1`
 *  - `RBLXWIN_KEY_WORLD_2`
 *  - `RBLXWIN_KEY_0` to `RBLXWIN_KEY_9`
 *  - `RBLXWIN_KEY_A` to `RBLXWIN_KEY_Z`
 *  - `RBLXWIN_KEY_KP_0` to `RBLXWIN_KEY_KP_9`
 *  - `RBLXWIN_KEY_KP_DECIMAL`
 *  - `RBLXWIN_KEY_KP_DIVIDE`
 *  - `RBLXWIN_KEY_KP_MULTIPLY`
 *  - `RBLXWIN_KEY_KP_SUBTRACT`
 *  - `RBLXWIN_KEY_KP_ADD`
 *  - `RBLXWIN_KEY_KP_EQUAL`
 *
 *  @param[in] key The key to query, or `RBLXWIN_KEY_UNKNOWN`.
 *  @param[in] scancode The scancode of the key to query.
 *  @return The localized name of the key, or `NULL`.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @pointer_lifetime The returned string is allocated and freed by RBLXWIN.  You
 *  should not free it yourself.  It is valid until the next call to @ref
 *  rblxwinGetKeyName, or until the library is terminated.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref input_key_name
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup input
 */
RBLXWINAPI const char* rblxwinGetKeyName(int key, int scancode);

/*! @brief Returns the last reported state of a keyboard key for the specified
 *  window.
 *
 *  This function returns the last state reported for the specified key to the
 *  specified window.  The returned state is one of `RBLXWIN_PRESS` or
 *  `RBLXWIN_RELEASE`.  The higher-level action `RBLXWIN_REPEAT` is only reported to
 *  the key callback.
 *
 *  If the `RBLXWIN_STICKY_KEYS` input mode is enabled, this function returns
 *  `RBLXWIN_PRESS` the first time you call it for a key that was pressed, even if
 *  that key has already been released.
 *
 *  The key functions deal with physical keys, with [key tokens](@ref keys)
 *  named after their use on the standard US keyboard layout.  If you want to
 *  input text, use the Unicode character callback instead.
 *
 *  The [modifier key bit masks](@ref mods) are not key tokens and cannot be
 *  used with this function.
 *
 *  __Do not use this function__ to implement [text input](@ref input_char).
 *
 *  @param[in] window The desired window.
 *  @param[in] key The desired [keyboard key](@ref keys).  `RBLXWIN_KEY_UNKNOWN` is
 *  not a valid key for this function.
 *  @return One of `RBLXWIN_PRESS` or `RBLXWIN_RELEASE`.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_INVALID_ENUM.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref input_key
 *
 *  @since Added in version 1.0.
 *  @rblxwin Added window handle parameter.
 *
 *  @ingroup input
 */
RBLXWINAPI int rblxwinGetKey(RBLXWINwindow* window, int key);

/*! @brief Returns the last reported state of a mouse button for the specified
 *  window.
 *
 *  This function returns the last state reported for the specified mouse button
 *  to the specified window.  The returned state is one of `RBLXWIN_PRESS` or
 *  `RBLXWIN_RELEASE`.
 *
 *  If the `RBLXWIN_STICKY_MOUSE_BUTTONS` input mode is enabled, this function
 *  `RBLXWIN_PRESS` the first time you call it for a mouse button that was pressed,
 *  even if that mouse button has already been released.
 *
 *  @param[in] window The desired window.
 *  @param[in] button The desired [mouse button](@ref buttons).
 *  @return One of `RBLXWIN_PRESS` or `RBLXWIN_RELEASE`.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_INVALID_ENUM.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref input_mouse_button
 *
 *  @since Added in version 1.0.
 *  @rblxwin Added window handle parameter.
 *
 *  @ingroup input
 */
RBLXWINAPI int rblxwinGetMouseButton(RBLXWINwindow* window, int button);

/*! @brief Retrieves the position of the cursor relative to the client area of
 *  the window.
 *
 *  This function returns the position of the cursor, in screen coordinates,
 *  relative to the upper-left corner of the client area of the specified
 *  window.
 *
 *  If the cursor is disabled (with `RBLXWIN_CURSOR_DISABLED`) then the cursor
 *  position is unbounded and limited only by the minimum and maximum values of
 *  a `double`.
 *
 *  The coordinate can be converted to their integer equivalents with the
 *  `floor` function.  Casting directly to an integer type works for positive
 *  coordinates, but fails for negative ones.
 *
 *  Any or all of the position arguments may be `NULL`.  If an error occurs, all
 *  non-`NULL` position arguments will be set to zero.
 *
 *  @param[in] window The desired window.
 *  @param[out] xpos Where to store the cursor x-coordinate, relative to the
 *  left edge of the client area, or `NULL`.
 *  @param[out] ypos Where to store the cursor y-coordinate, relative to the to
 *  top edge of the client area, or `NULL`.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref cursor_pos
 *  @sa rblxwinSetCursorPos
 *
 *  @since Added in version 3.0.  Replaces `rblxwinGetMousePos`.
 *
 *  @ingroup input
 */
RBLXWINAPI void rblxwinGetCursorPos(RBLXWINwindow* window, double* xpos, double* ypos);

/*! @brief Sets the position of the cursor, relative to the client area of the
 *  window.
 *
 *  This function sets the position, in screen coordinates, of the cursor
 *  relative to the upper-left corner of the client area of the specified
 *  window.  The window must have input focus.  If the window does not have
 *  input focus when this function is called, it fails silently.
 *
 *  __Do not use this function__ to implement things like camera controls.  RBLXWIN
 *  already provides the `RBLXWIN_CURSOR_DISABLED` cursor mode that hides the
 *  cursor, transparently re-centers it and provides unconstrained cursor
 *  motion.  See @ref rblxwinSetInputMode for more information.
 *
 *  If the cursor mode is `RBLXWIN_CURSOR_DISABLED` then the cursor position is
 *  unconstrained and limited only by the minimum and maximum values of
 *  a `double`.
 *
 *  @param[in] window The desired window.
 *  @param[in] xpos The desired x-coordinate, relative to the left edge of the
 *  client area.
 *  @param[in] ypos The desired y-coordinate, relative to the top edge of the
 *  client area.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @remark @x11 Due to the asynchronous nature of X11, it may take a moment for
 *  the window focus event to arrive.  This means you may not be able to set the
 *  cursor position directly after window creation.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref cursor_pos
 *  @sa rblxwinGetCursorPos
 *
 *  @since Added in version 3.0.  Replaces `rblxwinSetMousePos`.
 *
 *  @ingroup input
 */
RBLXWINAPI void rblxwinSetCursorPos(RBLXWINwindow* window, double xpos, double ypos);

/*! @brief Creates a custom cursor.
 *
 *  Creates a new custom cursor image that can be set for a window with @ref
 *  rblxwinSetCursor.  The cursor can be destroyed with @ref rblxwinDestroyCursor.
 *  Any remaining cursors are destroyed by @ref rblxwinTerminate.
 *
 *  The pixels are 32-bit, little-endian, non-premultiplied RGBA, i.e. eight
 *  bits per channel.  They are arranged canonically as packed sequential rows,
 *  starting from the top-left corner.
 *
 *  The cursor hotspot is specified in pixels, relative to the upper-left corner
 *  of the cursor image.  Like all other coordinate systems in RBLXWIN, the X-axis
 *  points to the right and the Y-axis points down.
 *
 *  @param[in] image The desired cursor image.
 *  @param[in] xhot The desired x-coordinate, in pixels, of the cursor hotspot.
 *  @param[in] yhot The desired y-coordinate, in pixels, of the cursor hotspot.
 *  @return The handle of the created cursor, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @pointer_lifetime The specified image data is copied before this function
 *  returns.
 *
 *  @reentrancy This function must not be called from a callback.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref cursor_object
 *  @sa rblxwinDestroyCursor
 *  @sa rblxwinCreateStandardCursor
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup input
 */
RBLXWINAPI RBLXWINcursor* rblxwinCreateCursor(const RBLXWINimage* image, int xhot, int yhot);

/*! @brief Creates a cursor with a standard shape.
 *
 *  Returns a cursor with a [standard shape](@ref shapes), that can be set for
 *  a window with @ref rblxwinSetCursor.
 *
 *  @param[in] shape One of the [standard shapes](@ref shapes).
 *  @return A new cursor ready to use or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_INVALID_ENUM and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @reentrancy This function must not be called from a callback.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref cursor_object
 *  @sa rblxwinCreateCursor
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup input
 */
RBLXWINAPI RBLXWINcursor* rblxwinCreateStandardCursor(int shape);

/*! @brief Destroys a cursor.
 *
 *  This function destroys a cursor previously created with @ref
 *  rblxwinCreateCursor.  Any remaining cursors will be destroyed by @ref
 *  rblxwinTerminate.
 *
 *  @param[in] cursor The cursor object to destroy.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @reentrancy This function must not be called from a callback.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref cursor_object
 *  @sa rblxwinCreateCursor
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup input
 */
RBLXWINAPI void rblxwinDestroyCursor(RBLXWINcursor* cursor);

/*! @brief Sets the cursor for the window.
 *
 *  This function sets the cursor image to be used when the cursor is over the
 *  client area of the specified window.  The set cursor will only be visible
 *  when the [cursor mode](@ref cursor_mode) of the window is
 *  `RBLXWIN_CURSOR_NORMAL`.
 *
 *  On some platforms, the set cursor may not be visible unless the window also
 *  has input focus.
 *
 *  @param[in] window The window to set the cursor for.
 *  @param[in] cursor The cursor to set, or `NULL` to switch back to the default
 *  arrow cursor.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref cursor_object
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup input
 */
RBLXWINAPI void rblxwinSetCursor(RBLXWINwindow* window, RBLXWINcursor* cursor);

/*! @brief Sets the key callback.
 *
 *  This function sets the key callback of the specified window, which is called
 *  when a key is pressed, repeated or released.
 *
 *  The key functions deal with physical keys, with layout independent
 *  [key tokens](@ref keys) named after their values in the standard US keyboard
 *  layout.  If you want to input text, use the
 *  [character callback](@ref rblxwinSetCharCallback) instead.
 *
 *  When a window loses input focus, it will generate synthetic key release
 *  events for all pressed keys.  You can tell these events from user-generated
 *  events by the fact that the synthetic ones are generated after the focus
 *  loss event has been processed, i.e. after the
 *  [window focus callback](@ref rblxwinSetWindowFocusCallback) has been called.
 *
 *  The scancode of a key is specific to that platform or sometimes even to that
 *  machine.  Scancodes are intended to allow users to bind keys that don't have
 *  a RBLXWIN key token.  Such keys have `key` set to `RBLXWIN_KEY_UNKNOWN`, their
 *  state is not saved and so it cannot be queried with @ref rblxwinGetKey.
 *
 *  Sometimes RBLXWIN needs to generate synthetic key events, in which case the
 *  scancode may be zero.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new key callback, or `NULL` to remove the currently
 *  set callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref input_key
 *
 *  @since Added in version 1.0.
 *  @rblxwin Added window handle parameter and return value.
 *
 *  @ingroup input
 */
RBLXWINAPI RBLXWINkeyfun rblxwinSetKeyCallback(RBLXWINwindow* window, RBLXWINkeyfun cbfun);

/*! @brief Sets the Unicode character callback.
 *
 *  This function sets the character callback of the specified window, which is
 *  called when a Unicode character is input.
 *
 *  The character callback is intended for Unicode text input.  As it deals with
 *  characters, it is keyboard layout dependent, whereas the
 *  [key callback](@ref rblxwinSetKeyCallback) is not.  Characters do not map 1:1
 *  to physical keys, as a key may produce zero, one or more characters.  If you
 *  want to know whether a specific physical key was pressed or released, see
 *  the key callback instead.
 *
 *  The character callback behaves as system text input normally does and will
 *  not be called if modifier keys are held down that would prevent normal text
 *  input on that platform, for example a Super (Command) key on OS X or Alt key
 *  on Windows.  There is a
 *  [character with modifiers callback](@ref rblxwinSetCharModsCallback) that
 *  receives these events.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref input_char
 *
 *  @since Added in version 2.4.
 *  @rblxwin Added window handle parameter and return value.
 *
 *  @ingroup input
 */
RBLXWINAPI RBLXWINcharfun rblxwinSetCharCallback(RBLXWINwindow* window, RBLXWINcharfun cbfun);

/*! @brief Sets the Unicode character with modifiers callback.
 *
 *  This function sets the character with modifiers callback of the specified
 *  window, which is called when a Unicode character is input regardless of what
 *  modifier keys are used.
 *
 *  The character with modifiers callback is intended for implementing custom
 *  Unicode character input.  For regular Unicode text input, see the
 *  [character callback](@ref rblxwinSetCharCallback).  Like the character
 *  callback, the character with modifiers callback deals with characters and is
 *  keyboard layout dependent.  Characters do not map 1:1 to physical keys, as
 *  a key may produce zero, one or more characters.  If you want to know whether
 *  a specific physical key was pressed or released, see the
 *  [key callback](@ref rblxwinSetKeyCallback) instead.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set or an
 *  error occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref input_char
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup input
 */
RBLXWINAPI RBLXWINcharmodsfun rblxwinSetCharModsCallback(RBLXWINwindow* window, RBLXWINcharmodsfun cbfun);

/*! @brief Sets the mouse button callback.
 *
 *  This function sets the mouse button callback of the specified window, which
 *  is called when a mouse button is pressed or released.
 *
 *  When a window loses input focus, it will generate synthetic mouse button
 *  release events for all pressed mouse buttons.  You can tell these events
 *  from user-generated events by the fact that the synthetic ones are generated
 *  after the focus loss event has been processed, i.e. after the
 *  [window focus callback](@ref rblxwinSetWindowFocusCallback) has been called.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref input_mouse_button
 *
 *  @since Added in version 1.0.
 *  @rblxwin Added window handle parameter and return value.
 *
 *  @ingroup input
 */
RBLXWINAPI RBLXWINmousebuttonfun rblxwinSetMouseButtonCallback(RBLXWINwindow* window, RBLXWINmousebuttonfun cbfun);

/*! @brief Sets the cursor position callback.
 *
 *  This function sets the cursor position callback of the specified window,
 *  which is called when the cursor is moved.  The callback is provided with the
 *  position, in screen coordinates, relative to the upper-left corner of the
 *  client area of the window.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref cursor_pos
 *
 *  @since Added in version 3.0.  Replaces `rblxwinSetMousePosCallback`.
 *
 *  @ingroup input
 */
RBLXWINAPI RBLXWINcursorposfun rblxwinSetCursorPosCallback(RBLXWINwindow* window, RBLXWINcursorposfun cbfun);

/*! @brief Sets the cursor enter/exit callback.
 *
 *  This function sets the cursor boundary crossing callback of the specified
 *  window, which is called when the cursor enters or leaves the client area of
 *  the window.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref cursor_enter
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup input
 */
RBLXWINAPI RBLXWINcursorenterfun rblxwinSetCursorEnterCallback(RBLXWINwindow* window, RBLXWINcursorenterfun cbfun);

/*! @brief Sets the scroll callback.
 *
 *  This function sets the scroll callback of the specified window, which is
 *  called when a scrolling device is used, such as a mouse wheel or scrolling
 *  area of a touchpad.
 *
 *  The scroll callback receives all scrolling input, like that from a mouse
 *  wheel or a touchpad scrolling area.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new scroll callback, or `NULL` to remove the currently
 *  set callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref scrolling
 *
 *  @since Added in version 3.0.  Replaces `rblxwinSetMouseWheelCallback`.
 *
 *  @ingroup input
 */
RBLXWINAPI RBLXWINscrollfun rblxwinSetScrollCallback(RBLXWINwindow* window, RBLXWINscrollfun cbfun);

/*! @brief Sets the file drop callback.
 *
 *  This function sets the file drop callback of the specified window, which is
 *  called when one or more dragged files are dropped on the window.
 *
 *  Because the path array and its strings may have been generated specifically
 *  for that event, they are not guaranteed to be valid after the callback has
 *  returned.  If you wish to use them after the callback returns, you need to
 *  make a deep copy.
 *
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new file drop callback, or `NULL` to remove the
 *  currently set callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref path_drop
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup input
 */
RBLXWINAPI RBLXWINdropfun rblxwinSetDropCallback(RBLXWINwindow* window, RBLXWINdropfun cbfun);

/*! @brief Returns whether the specified joystick is present.
 *
 *  This function returns whether the specified joystick is present.
 *
 *  @param[in] joy The [joystick](@ref joysticks) to query.
 *  @return `RBLXWIN_TRUE` if the joystick is present, or `RBLXWIN_FALSE` otherwise.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_INVALID_ENUM and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref joystick
 *
 *  @since Added in version 3.0.  Replaces `rblxwinGetJoystickParam`.
 *
 *  @ingroup input
 */
RBLXWINAPI int rblxwinJoystickPresent(int joy);

/*! @brief Returns the values of all axes of the specified joystick.
 *
 *  This function returns the values of all axes of the specified joystick.
 *  Each element in the array is a value between -1.0 and 1.0.
 *
 *  Querying a joystick slot with no device present is not an error, but will
 *  cause this function to return `NULL`.  Call @ref rblxwinJoystickPresent to
 *  check device presence.
 *
 *  @param[in] joy The [joystick](@ref joysticks) to query.
 *  @param[out] count Where to store the number of axis values in the returned
 *  array.  This is set to zero if an error occurred.
 *  @return An array of axis values, or `NULL` if the joystick is not present.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_INVALID_ENUM and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @pointer_lifetime The returned array is allocated and freed by RBLXWIN.  You
 *  should not free it yourself.  It is valid until the specified joystick is
 *  disconnected, this function is called again for that joystick or the library
 *  is terminated.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref joystick_axis
 *
 *  @since Added in version 3.0.  Replaces `rblxwinGetJoystickPos`.
 *
 *  @ingroup input
 */
RBLXWINAPI const float* rblxwinGetJoystickAxes(int joy, int* count);

/*! @brief Returns the state of all buttons of the specified joystick.
 *
 *  This function returns the state of all buttons of the specified joystick.
 *  Each element in the array is either `RBLXWIN_PRESS` or `RBLXWIN_RELEASE`.
 *
 *  Querying a joystick slot with no device present is not an error, but will
 *  cause this function to return `NULL`.  Call @ref rblxwinJoystickPresent to
 *  check device presence.
 *
 *  @param[in] joy The [joystick](@ref joysticks) to query.
 *  @param[out] count Where to store the number of button states in the returned
 *  array.  This is set to zero if an error occurred.
 *  @return An array of button states, or `NULL` if the joystick is not present.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_INVALID_ENUM and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @pointer_lifetime The returned array is allocated and freed by RBLXWIN.  You
 *  should not free it yourself.  It is valid until the specified joystick is
 *  disconnected, this function is called again for that joystick or the library
 *  is terminated.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref joystick_button
 *
 *  @since Added in version 2.2.
 *  @rblxwin Changed to return a dynamic array.
 *
 *  @ingroup input
 */
RBLXWINAPI const unsigned char* rblxwinGetJoystickButtons(int joy, int* count);

/*! @brief Returns the name of the specified joystick.
 *
 *  This function returns the name, encoded as UTF-8, of the specified joystick.
 *  The returned string is allocated and freed by RBLXWIN.  You should not free it
 *  yourself.
 *
 *  Querying a joystick slot with no device present is not an error, but will
 *  cause this function to return `NULL`.  Call @ref rblxwinJoystickPresent to
 *  check device presence.
 *
 *  @param[in] joy The [joystick](@ref joysticks) to query.
 *  @return The UTF-8 encoded name of the joystick, or `NULL` if the joystick
 *  is not present.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_INVALID_ENUM and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @pointer_lifetime The returned string is allocated and freed by RBLXWIN.  You
 *  should not free it yourself.  It is valid until the specified joystick is
 *  disconnected, this function is called again for that joystick or the library
 *  is terminated.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref joystick_name
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup input
 */
RBLXWINAPI const char* rblxwinGetJoystickName(int joy);

/*! @brief Sets the joystick configuration callback.
 *
 *  This function sets the joystick configuration callback, or removes the
 *  currently set callback.  This is called when a joystick is connected to or
 *  disconnected from the system.
 *
 *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
 *  callback.
 *  @return The previously set callback, or `NULL` if no callback was set or the
 *  library had not been [initialized](@ref intro_init).
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref joystick_event
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup input
 */
RBLXWINAPI RBLXWINjoystickfun rblxwinSetJoystickCallback(RBLXWINjoystickfun cbfun);

/*! @brief Sets the clipboard to the specified string.
 *
 *  This function sets the system clipboard to the specified, UTF-8 encoded
 *  string.
 *
 *  @param[in] window The window that will own the clipboard contents.
 *  @param[in] string A UTF-8 encoded string.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @pointer_lifetime The specified string is copied before this function
 *  returns.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref clipboard
 *  @sa rblxwinGetClipboardString
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup input
 */
RBLXWINAPI void rblxwinSetClipboardString(RBLXWINwindow* window, const char* string);

/*! @brief Returns the contents of the clipboard as a string.
 *
 *  This function returns the contents of the system clipboard, if it contains
 *  or is convertible to a UTF-8 encoded string.  If the clipboard is empty or
 *  if its contents cannot be converted, `NULL` is returned and a @ref
 *  RBLXWIN_FORMAT_UNAVAILABLE error is generated.
 *
 *  @param[in] window The window that will request the clipboard contents.
 *  @return The contents of the clipboard as a UTF-8 encoded string, or `NULL`
 *  if an [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @pointer_lifetime The returned string is allocated and freed by RBLXWIN.  You
 *  should not free it yourself.  It is valid until the next call to @ref
 *  rblxwinGetClipboardString or @ref rblxwinSetClipboardString, or until the library
 *  is terminated.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref clipboard
 *  @sa rblxwinSetClipboardString
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup input
 */
RBLXWINAPI const char* rblxwinGetClipboardString(RBLXWINwindow* window);

/*! @brief Returns the value of the RBLXWIN timer.
 *
 *  This function returns the value of the RBLXWIN timer.  Unless the timer has
 *  been set using @ref rblxwinSetTime, the timer measures time elapsed since RBLXWIN
 *  was initialized.
 *
 *  The resolution of the timer is system dependent, but is usually on the order
 *  of a few micro- or nanoseconds.  It uses the highest-resolution monotonic
 *  time source on each supported platform.
 *
 *  @return The current value, in seconds, or zero if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Reading of the
 *  internal timer offset is not atomic.
 *
 *  @sa @ref time
 *
 *  @since Added in version 1.0.
 *
 *  @ingroup input
 */
RBLXWINAPI double rblxwinGetTime(void);

/*! @brief Sets the RBLXWIN timer.
 *
 *  This function sets the value of the RBLXWIN timer.  It then continues to count
 *  up from that value.  The value must be a positive finite number less than
 *  or equal to 18446744073.0, which is approximately 584.5 years.
 *
 *  @param[in] time The new value, in seconds.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_INVALID_VALUE.
 *
 *  @remark The upper limit of the timer is calculated as
 *  floor((2<sup>64</sup> - 1) / 10<sup>9</sup>) and is due to implementations
 *  storing nanoseconds in 64 bits.  The limit may be increased in the future.
 *
 *  @thread_safety This function may be called from any thread.  Writing of the
 *  internal timer offset is not atomic.
 *
 *  @sa @ref time
 *
 *  @since Added in version 2.2.
 *
 *  @ingroup input
 */
RBLXWINAPI void rblxwinSetTime(double time);

/*! @brief Returns the current value of the raw timer.
 *
 *  This function returns the current value of the raw timer, measured in
 *  1&nbsp;/&nbsp;frequency seconds.  To get the frequency, call @ref
 *  rblxwinGetTimerFrequency.
 *
 *  @return The value of the timer, or zero if an 
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.
 *
 *  @sa @ref time
 *  @sa rblxwinGetTimerFrequency
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup input
 */
RBLXWINAPI uint64_t rblxwinGetTimerValue(void);

/*! @brief Returns the frequency, in Hz, of the raw timer.
 *
 *  This function returns the frequency, in Hz, of the raw timer.
 *
 *  @return The frequency of the timer, in Hz, or zero if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.
 *
 *  @sa @ref time
 *  @sa rblxwinGetTimerValue
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup input
 */
RBLXWINAPI uint64_t rblxwinGetTimerFrequency(void);

/*! @brief Makes the context of the specified window current for the calling
 *  thread.
 *
 *  This function makes the OpenGL or OpenGL ES context of the specified window
 *  current on the calling thread.  A context can only be made current on
 *  a single thread at a time and each thread can have only a single current
 *  context at a time.
 *
 *  By default, making a context non-current implicitly forces a pipeline flush.
 *  On machines that support `GL_KHR_context_flush_control`, you can control
 *  whether a context performs this flush by setting the
 *  [RBLXWIN_CONTEXT_RELEASE_BEHAVIOR](@ref window_hints_ctx) window hint.
 *
 *  The specified window must have an OpenGL or OpenGL ES context.  Specifying
 *  a window without a context will generate a @ref RBLXWIN_NO_WINDOW_CONTEXT
 *  error.
 *
 *  @param[in] window The window whose context to make current, or `NULL` to
 *  detach the current context.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_NO_WINDOW_CONTEXT and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function may be called from any thread.
 *
 *  @sa @ref context_current
 *  @sa rblxwinGetCurrentContext
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup context
 */
RBLXWINAPI void rblxwinMakeContextCurrent(RBLXWINwindow* window);

/*! @brief Returns the window whose context is current on the calling thread.
 *
 *  This function returns the window whose OpenGL or OpenGL ES context is
 *  current on the calling thread.
 *
 *  @return The window whose context is current, or `NULL` if no window's
 *  context is current.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.
 *
 *  @sa @ref context_current
 *  @sa rblxwinMakeContextCurrent
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup context
 */
RBLXWINAPI RBLXWINwindow* rblxwinGetCurrentContext(void);

/*! @brief Swaps the front and back buffers of the specified window.
 *
 *  This function swaps the front and back buffers of the specified window when
 *  rendering with OpenGL or OpenGL ES.  If the swap interval is greater than
 *  zero, the GPU driver waits the specified number of screen updates before
 *  swapping the buffers.
 *
 *  The specified window must have an OpenGL or OpenGL ES context.  Specifying
 *  a window without a context will generate a @ref RBLXWIN_NO_WINDOW_CONTEXT
 *  error.
 *
 *  This function does not apply to Vulkan.  If you are rendering with Vulkan,
 *  see `vkQueuePresentKHR` instead.
 *
 *  @param[in] window The window whose buffers to swap.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_NO_WINDOW_CONTEXT and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @remark __EGL:__ The context of the specified window must be current on the
 *  calling thread.
 *
 *  @thread_safety This function may be called from any thread.
 *
 *  @sa @ref buffer_swap
 *  @sa rblxwinSwapInterval
 *
 *  @since Added in version 1.0.
 *  @rblxwin Added window handle parameter.
 *
 *  @ingroup window
 */
RBLXWINAPI void rblxwinSwapBuffers(RBLXWINwindow* window);

/*! @brief Sets the swap interval for the current context.
 *
 *  This function sets the swap interval for the current OpenGL or OpenGL ES
 *  context, i.e. the number of screen updates to wait from the time @ref
 *  rblxwinSwapBuffers was called before swapping the buffers and returning.  This
 *  is sometimes called _vertical synchronization_, _vertical retrace
 *  synchronization_ or just _vsync_.
 *
 *  Contexts that support either of the `WGL_EXT_swap_control_tear` and
 *  `GLX_EXT_swap_control_tear` extensions also accept negative swap intervals,
 *  which allow the driver to swap even if a frame arrives a little bit late.
 *  You can check for the presence of these extensions using @ref
 *  rblxwinExtensionSupported.  For more information about swap tearing, see the
 *  extension specifications.
 *
 *  A context must be current on the calling thread.  Calling this function
 *  without a current context will cause a @ref RBLXWIN_NO_CURRENT_CONTEXT error.
 *
 *  This function does not apply to Vulkan.  If you are rendering with Vulkan,
 *  see the present mode of your swapchain instead.
 *
 *  @param[in] interval The minimum number of screen updates to wait for
 *  until the buffers are swapped by @ref rblxwinSwapBuffers.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_NO_CURRENT_CONTEXT and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @remark This function is not called during context creation, leaving the
 *  swap interval set to whatever is the default on that platform.  This is done
 *  because some swap interval extensions used by RBLXWIN do not allow the swap
 *  interval to be reset to zero once it has been set to a non-zero value.
 *
 *  @remark Some GPU drivers do not honor the requested swap interval, either
 *  because of a user setting that overrides the application's request or due to
 *  bugs in the driver.
 *
 *  @thread_safety This function may be called from any thread.
 *
 *  @sa @ref buffer_swap
 *  @sa rblxwinSwapBuffers
 *
 *  @since Added in version 1.0.
 *
 *  @ingroup context
 */
RBLXWINAPI void rblxwinSwapInterval(int interval);

/*! @brief Returns whether the specified extension is available.
 *
 *  This function returns whether the specified
 *  [API extension](@ref context_glext) is supported by the current OpenGL or
 *  OpenGL ES context.  It searches both for client API extension and context
 *  creation API extensions.
 *
 *  A context must be current on the calling thread.  Calling this function
 *  without a current context will cause a @ref RBLXWIN_NO_CURRENT_CONTEXT error.
 *
 *  As this functions retrieves and searches one or more extension strings each
 *  call, it is recommended that you cache its results if it is going to be used
 *  frequently.  The extension strings will not change during the lifetime of
 *  a context, so there is no danger in doing this.
 *
 *  This function does not apply to Vulkan.  If you are using Vulkan, see @ref
 *  rblxwinGetRequiredInstanceExtensions, `vkEnumerateInstanceExtensionProperties`
 *  and `vkEnumerateDeviceExtensionProperties` instead.
 *
 *  @param[in] extension The ASCII encoded name of the extension.
 *  @return `RBLXWIN_TRUE` if the extension is available, or `RBLXWIN_FALSE`
 *  otherwise.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_NO_CURRENT_CONTEXT, @ref RBLXWIN_INVALID_VALUE and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function may be called from any thread.
 *
 *  @sa @ref context_glext
 *  @sa rblxwinGetProcAddress
 *
 *  @since Added in version 1.0.
 *
 *  @ingroup context
 */
RBLXWINAPI int rblxwinExtensionSupported(const char* extension);

/*! @brief Returns the address of the specified function for the current
 *  context.
 *
 *  This function returns the address of the specified OpenGL or OpenGL ES
 *  [core or extension function](@ref context_glext), if it is supported
 *  by the current context.
 *
 *  A context must be current on the calling thread.  Calling this function
 *  without a current context will cause a @ref RBLXWIN_NO_CURRENT_CONTEXT error.
 *
 *  This function does not apply to Vulkan.  If you are rendering with Vulkan,
 *  see @ref rblxwinGetInstanceProcAddress, `vkGetInstanceProcAddr` and
 *  `vkGetDeviceProcAddr` instead.
 *
 *  @param[in] procname The ASCII encoded name of the function.
 *  @return The address of the function, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_NO_CURRENT_CONTEXT and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @remark The address of a given function is not guaranteed to be the same
 *  between contexts.
 *
 *  @remark This function may return a non-`NULL` address despite the
 *  associated version or extension not being available.  Always check the
 *  context version or extension string first.
 *
 *  @pointer_lifetime The returned function pointer is valid until the context
 *  is destroyed or the library is terminated.
 *
 *  @thread_safety This function may be called from any thread.
 *
 *  @sa @ref context_glext
 *  @sa rblxwinExtensionSupported
 *
 *  @since Added in version 1.0.
 *
 *  @ingroup context
 */
RBLXWINAPI RBLXWINglproc rblxwinGetProcAddress(const char* procname);

/*! @brief Returns whether the Vulkan loader has been found.
 *
 *  This function returns whether the Vulkan loader has been found.  This check
 *  is performed by @ref rblxwinInit.
 *
 *  The availability of a Vulkan loader does not by itself guarantee that window
 *  surface creation or even device creation is possible.  Call @ref
 *  rblxwinGetRequiredInstanceExtensions to check whether the extensions necessary
 *  for Vulkan surface creation are available and @ref
 *  rblxwinGetPhysicalDevicePresentationSupport to check whether a queue family of
 *  a physical device supports image presentation.
 *
 *  @return `RBLXWIN_TRUE` if Vulkan is available, or `RBLXWIN_FALSE` otherwise.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.
 *
 *  @sa @ref vulkan_support
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup vulkan
 */
RBLXWINAPI int rblxwinVulkanSupported(void);

/*! @brief Returns the Vulkan instance extensions required by RBLXWIN.
 *
 *  This function returns an array of names of Vulkan instance extensions required
 *  by RBLXWIN for creating Vulkan surfaces for RBLXWIN windows.  If successful, the
 *  list will always contains `VK_KHR_surface`, so if you don't require any
 *  additional extensions you can pass this list directly to the
 *  `VkInstanceCreateInfo` struct.
 *
 *  If Vulkan is not available on the machine, this function returns `NULL` and
 *  generates a @ref RBLXWIN_API_UNAVAILABLE error.  Call @ref rblxwinVulkanSupported
 *  to check whether Vulkan is available.
 *
 *  If Vulkan is available but no set of extensions allowing window surface
 *  creation was found, this function returns `NULL`.  You may still use Vulkan
 *  for off-screen rendering and compute work.
 *
 *  @param[out] count Where to store the number of extensions in the returned
 *  array.  This is set to zero if an error occurred.
 *  @return An array of ASCII encoded extension names, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_API_UNAVAILABLE.
 *
 *  @remarks Additional extensions may be required by future versions of RBLXWIN.
 *  You should check if any extensions you wish to enable are already in the
 *  returned array, as it is an error to specify an extension more than once in
 *  the `VkInstanceCreateInfo` struct.
 *
 *  @pointer_lifetime The returned array is allocated and freed by RBLXWIN.  You
 *  should not free it yourself.  It is guaranteed to be valid only until the
 *  library is terminated.
 *
 *  @thread_safety This function may be called from any thread.
 *
 *  @sa @ref vulkan_ext
 *  @sa rblxwinCreateWindowSurface
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup vulkan
 */
RBLXWINAPI const char** rblxwinGetRequiredInstanceExtensions(uint32_t* count);

#if defined(VK_VERSION_1_0)

/*! @brief Returns the address of the specified Vulkan instance function.
 *
 *  This function returns the address of the specified Vulkan core or extension
 *  function for the specified instance.  If instance is set to `NULL` it can
 *  return any function exported from the Vulkan loader, including at least the
 *  following functions:
 *
 *  - `vkEnumerateInstanceExtensionProperties`
 *  - `vkEnumerateInstanceLayerProperties`
 *  - `vkCreateInstance`
 *  - `vkGetInstanceProcAddr`
 *
 *  If Vulkan is not available on the machine, this function returns `NULL` and
 *  generates a @ref RBLXWIN_API_UNAVAILABLE error.  Call @ref rblxwinVulkanSupported
 *  to check whether Vulkan is available.
 *
 *  This function is equivalent to calling `vkGetInstanceProcAddr` with
 *  a platform-specific query of the Vulkan loader as a fallback.
 *
 *  @param[in] instance The Vulkan instance to query, or `NULL` to retrieve
 *  functions related to instance creation.
 *  @param[in] procname The ASCII encoded name of the function.
 *  @return The address of the function, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_API_UNAVAILABLE.
 *
 *  @pointer_lifetime The returned function pointer is valid until the library
 *  is terminated.
 *
 *  @thread_safety This function may be called from any thread.
 *
 *  @sa @ref vulkan_proc
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup vulkan
 */
RBLXWINAPI RBLXWINvkproc rblxwinGetInstanceProcAddress(VkInstance instance, const char* procname);

/*! @brief Returns whether the specified queue family can present images.
 *
 *  This function returns whether the specified queue family of the specified
 *  physical device supports presentation to the platform RBLXWIN was built for.
 *
 *  If Vulkan or the required window surface creation instance extensions are
 *  not available on the machine, or if the specified instance was not created
 *  with the required extensions, this function returns `RBLXWIN_FALSE` and
 *  generates a @ref RBLXWIN_API_UNAVAILABLE error.  Call @ref rblxwinVulkanSupported
 *  to check whether Vulkan is available and @ref
 *  rblxwinGetRequiredInstanceExtensions to check what instance extensions are
 *  required.
 *
 *  @param[in] instance The instance that the physical device belongs to.
 *  @param[in] device The physical device that the queue family belongs to.
 *  @param[in] queuefamily The index of the queue family to query.
 *  @return `RBLXWIN_TRUE` if the queue family supports presentation, or
 *  `RBLXWIN_FALSE` otherwise.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_API_UNAVAILABLE and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @thread_safety This function may be called from any thread.  For
 *  synchronization details of Vulkan objects, see the Vulkan specification.
 *
 *  @sa @ref vulkan_present
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup vulkan
 */
RBLXWINAPI int rblxwinGetPhysicalDevicePresentationSupport(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily);

/*! @brief Creates a Vulkan surface for the specified window.
 *
 *  This function creates a Vulkan surface for the specified window.
 *
 *  If the Vulkan loader was not found at initialization, this function returns
 *  `VK_ERROR_INITIALIZATION_FAILED` and generates a @ref RBLXWIN_API_UNAVAILABLE
 *  error.  Call @ref rblxwinVulkanSupported to check whether the Vulkan loader was
 *  found.
 *
 *  If the required window surface creation instance extensions are not
 *  available or if the specified instance was not created with these extensions
 *  enabled, this function returns `VK_ERROR_EXTENSION_NOT_PRESENT` and
 *  generates a @ref RBLXWIN_API_UNAVAILABLE error.  Call @ref
 *  rblxwinGetRequiredInstanceExtensions to check what instance extensions are
 *  required.
 *
 *  The window surface must be destroyed before the specified Vulkan instance.
 *  It is the responsibility of the caller to destroy the window surface.  RBLXWIN
 *  does not destroy it for you.  Call `vkDestroySurfaceKHR` to destroy the
 *  surface.
 *
 *  @param[in] instance The Vulkan instance to create the surface in.
 *  @param[in] window The window to create the surface for.
 *  @param[in] allocator The allocator to use, or `NULL` to use the default
 *  allocator.
 *  @param[out] surface Where to store the handle of the surface.  This is set
 *  to `VK_NULL_HANDLE` if an error occurred.
 *  @return `VK_SUCCESS` if successful, or a Vulkan error code if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED, @ref
 *  RBLXWIN_API_UNAVAILABLE and @ref RBLXWIN_PLATFORM_ERROR.
 *
 *  @remarks If an error occurs before the creation call is made, RBLXWIN returns
 *  the Vulkan error code most appropriate for the error.  Appropriate use of
 *  @ref rblxwinVulkanSupported and @ref rblxwinGetRequiredInstanceExtensions should
 *  eliminate almost all occurrences of these errors.
 *
 *  @thread_safety This function may be called from any thread.  For
 *  synchronization details of Vulkan objects, see the Vulkan specification.
 *
 *  @sa @ref vulkan_surface
 *  @sa rblxwinGetRequiredInstanceExtensions
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup vulkan
 */
RBLXWINAPI VkResult rblxwinCreateWindowSurface(VkInstance instance, RBLXWINwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

#endif /*VK_VERSION_1_0*/


/*************************************************************************
 * Global definition cleanup
 *************************************************************************/

/* ------------------- BEGIN SYSTEM/COMPILER SPECIFIC -------------------- */

#ifdef RBLXWIN_WINGDIAPI_DEFINED
 #undef WINGDIAPI
 #undef RBLXWIN_WINGDIAPI_DEFINED
#endif

#ifdef RBLXWIN_CALLBACK_DEFINED
 #undef CALLBACK
 #undef RBLXWIN_CALLBACK_DEFINED
#endif

/* -------------------- END SYSTEM/COMPILER SPECIFIC --------------------- */


#ifdef __cplusplus
}
#endif

#endif /* _rblxwin_h_ */

