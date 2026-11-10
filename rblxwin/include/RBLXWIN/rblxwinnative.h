/*************************************************************************
 * RBLXWIN 3.3 - www.rblxwin.org
 * A library for OpenGL, window and input
 *------------------------------------------------------------------------
 * Copyright (c) 2002-2006 Marcus Geelnard
 * Copyright (c) 2006-2018 Camilla Löwy <elmindreda@rblxwin.org>
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

#ifndef _rblxwin_native_h_
#define _rblxwin_native_h_

#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************************
 * Doxygen documentation
 *************************************************************************/

/*! @file rblxwinnative.h
 *  @brief The header of the native access functions.
 *
 *  This is the header file of the native access functions.  See @ref native for
 *  more information.
 */
/*! @defgroup native Native access
 *  @brief Functions related to accessing native handles.
 *
 *  **By using the native access functions you assert that you know what you're
 *  doing and how to fix problems caused by using them.  If you don't, you
 *  shouldn't be using them.**
 *
 *  Before the inclusion of @ref rblxwinnative.h, you may define zero or more
 *  window system API macro and zero or more context creation API macros.
 *
 *  The chosen backends must match those the library was compiled for.  Failure
 *  to do this will cause a link-time error.
 *
 *  The available window API macros are:
 *  * `RBLXWIN_EXPOSE_NATIVE_WIN32`
 *  * `RBLXWIN_EXPOSE_NATIVE_COCOA`
 *  * `RBLXWIN_EXPOSE_NATIVE_X11`
 *  * `RBLXWIN_EXPOSE_NATIVE_WAYLAND`
 *
 *  The available context API macros are:
 *  * `RBLXWIN_EXPOSE_NATIVE_WGL`
 *  * `RBLXWIN_EXPOSE_NATIVE_NSGL`
 *  * `RBLXWIN_EXPOSE_NATIVE_GLX`
 *  * `RBLXWIN_EXPOSE_NATIVE_EGL`
 *  * `RBLXWIN_EXPOSE_NATIVE_OSMESA`
 *
 *  These macros select which of the native access functions that are declared
 *  and which platform-specific headers to include.  It is then up your (by
 *  definition platform-specific) code to handle which of these should be
 *  defined.
 *
 *  If you do not want the platform-specific headers to be included, define
 *  `RBLXWIN_NATIVE_INCLUDE_NONE` before including the @ref rblxwinnative.h header.
 *
 *  @code
 *  #define RBLXWIN_EXPOSE_NATIVE_WIN32
 *  #define RBLXWIN_EXPOSE_NATIVE_WGL
 *  #define RBLXWIN_NATIVE_INCLUDE_NONE
 *  #include <RBLXWIN/rblxwinnative.h>
 *  @endcode
 */


/*************************************************************************
 * System headers and types
 *************************************************************************/

#if !defined(RBLXWIN_NATIVE_INCLUDE_NONE)

 #if defined(RBLXWIN_EXPOSE_NATIVE_WIN32) || defined(RBLXWIN_EXPOSE_NATIVE_WGL)
  /* This is a workaround for the fact that rblxwin.h needs to export APIENTRY (for
   * example to allow applications to correctly declare a GL_KHR_debug callback)
   * but windows.h assumes no one will define APIENTRY before it does
   */
  #if defined(RBLXWIN_APIENTRY_DEFINED)
   #undef APIENTRY
   #undef RBLXWIN_APIENTRY_DEFINED
  #endif
  #include <windows.h>
 #elif defined(RBLXWIN_EXPOSE_NATIVE_COCOA) || defined(RBLXWIN_EXPOSE_NATIVE_NSGL)
  #if defined(__OBJC__)
   #import <Cocoa/Cocoa.h>
  #else
   #include <ApplicationServices/ApplicationServices.h>
   #include <objc/objc.h>
  #endif
 #elif defined(RBLXWIN_EXPOSE_NATIVE_X11) || defined(RBLXWIN_EXPOSE_NATIVE_GLX)
  #include <X11/Xlib.h>
  #include <X11/extensions/Xrandr.h>
 #elif defined(RBLXWIN_EXPOSE_NATIVE_WAYLAND)
  #include <wayland-client.h>
 #endif

 #if defined(RBLXWIN_EXPOSE_NATIVE_WGL)
  /* WGL is declared by windows.h */
 #endif
 #if defined(RBLXWIN_EXPOSE_NATIVE_NSGL)
  /* NSGL is declared by Cocoa.h */
 #endif
 #if defined(RBLXWIN_EXPOSE_NATIVE_GLX)
  /* This is a workaround for the fact that rblxwin.h defines GLAPIENTRY because by
   * default it also acts as an OpenGL header
   * However, glx.h will include gl.h, which will define it unconditionally
   */
  #if defined(RBLXWIN_GLAPIENTRY_DEFINED)
   #undef GLAPIENTRY
   #undef RBLXWIN_GLAPIENTRY_DEFINED
  #endif
  #include <GL/glx.h>
 #endif
 #if defined(RBLXWIN_EXPOSE_NATIVE_EGL)
  #include <EGL/egl.h>
 #endif
 #if defined(RBLXWIN_EXPOSE_NATIVE_OSMESA)
  /* This is a workaround for the fact that rblxwin.h defines GLAPIENTRY because by
   * default it also acts as an OpenGL header
   * However, osmesa.h will include gl.h, which will define it unconditionally
   */
  #if defined(RBLXWIN_GLAPIENTRY_DEFINED)
   #undef GLAPIENTRY
   #undef RBLXWIN_GLAPIENTRY_DEFINED
  #endif
  #include <GL/osmesa.h>
 #endif

#endif /*RBLXWIN_NATIVE_INCLUDE_NONE*/


/*************************************************************************
 * Functions
 *************************************************************************/

#if defined(RBLXWIN_EXPOSE_NATIVE_WIN32)
/*! @brief Returns the adapter device name of the specified monitor.
 *
 *  @return The UTF-8 encoded adapter device name (for example `\\.\DISPLAY1`)
 *  of the specified monitor, or `NULL` if an [error](@ref error_handling)
 *  occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup native
 */
RBLXWINAPI const char* rblxwinGetWin32Adapter(RBLXWINmonitor* monitor);

/*! @brief Returns the display device name of the specified monitor.
 *
 *  @return The UTF-8 encoded display device name (for example
 *  `\\.\DISPLAY1\Monitor0`) of the specified monitor, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup native
 */
RBLXWINAPI const char* rblxwinGetWin32Monitor(RBLXWINmonitor* monitor);

/*! @brief Returns the `HWND` of the specified window.
 *
 *  @return The `HWND` of the specified window, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @remark The `HDC` associated with the window can be queried with the
 *  [GetDC](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdc)
 *  function.
 *  @code
 *  HDC dc = GetDC(rblxwinGetWin32Window(window));
 *  @endcode
 *  This DC is private and does not need to be released.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup native
 */
RBLXWINAPI HWND rblxwinGetWin32Window(RBLXWINwindow* window);
#endif

#if defined(RBLXWIN_EXPOSE_NATIVE_WGL)
/*! @brief Returns the `HGLRC` of the specified window.
 *
 *  @return The `HGLRC` of the specified window, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NO_WINDOW_CONTEXT and @ref
 *  RBLXWIN_NOT_INITIALIZED.
 *
 *  @remark The `HDC` associated with the window can be queried with the
 *  [GetDC](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdc)
 *  function.
 *  @code
 *  HDC dc = GetDC(rblxwinGetWin32Window(window));
 *  @endcode
 *  This DC is private and does not need to be released.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup native
 */
RBLXWINAPI HGLRC rblxwinGetWGLContext(RBLXWINwindow* window);
#endif

#if defined(RBLXWIN_EXPOSE_NATIVE_COCOA)
/*! @brief Returns the `CGDirectDisplayID` of the specified monitor.
 *
 *  @return The `CGDirectDisplayID` of the specified monitor, or
 *  `kCGNullDirectDisplay` if an [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup native
 */
RBLXWINAPI CGDirectDisplayID rblxwinGetCocoaMonitor(RBLXWINmonitor* monitor);

/*! @brief Returns the `NSWindow` of the specified window.
 *
 *  @return The `NSWindow` of the specified window, or `nil` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup native
 */
RBLXWINAPI id rblxwinGetCocoaWindow(RBLXWINwindow* window);
#endif

#if defined(RBLXWIN_EXPOSE_NATIVE_NSGL)
/*! @brief Returns the `NSOpenGLContext` of the specified window.
 *
 *  @return The `NSOpenGLContext` of the specified window, or `nil` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NO_WINDOW_CONTEXT and @ref
 *  RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup native
 */
RBLXWINAPI id rblxwinGetNSGLContext(RBLXWINwindow* window);
#endif

#if defined(RBLXWIN_EXPOSE_NATIVE_X11)
/*! @brief Returns the `Display` used by RBLXWIN.
 *
 *  @return The `Display` used by RBLXWIN, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup native
 */
RBLXWINAPI Display* rblxwinGetX11Display(void);

/*! @brief Returns the `RRCrtc` of the specified monitor.
 *
 *  @return The `RRCrtc` of the specified monitor, or `None` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup native
 */
RBLXWINAPI RRCrtc rblxwinGetX11Adapter(RBLXWINmonitor* monitor);

/*! @brief Returns the `RROutput` of the specified monitor.
 *
 *  @return The `RROutput` of the specified monitor, or `None` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.1.
 *
 *  @ingroup native
 */
RBLXWINAPI RROutput rblxwinGetX11Monitor(RBLXWINmonitor* monitor);

/*! @brief Returns the `Window` of the specified window.
 *
 *  @return The `Window` of the specified window, or `None` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup native
 */
RBLXWINAPI Window rblxwinGetX11Window(RBLXWINwindow* window);

/*! @brief Sets the current primary selection to the specified string.
 *
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
 *  @sa rblxwinGetX11SelectionString
 *  @sa rblxwinSetClipboardString
 *
 *  @since Added in version 3.3.
 *
 *  @ingroup native
 */
RBLXWINAPI void rblxwinSetX11SelectionString(const char* string);

/*! @brief Returns the contents of the current primary selection as a string.
 *
 *  If the selection is empty or if its contents cannot be converted, `NULL`
 *  is returned and a @ref RBLXWIN_FORMAT_UNAVAILABLE error is generated.
 *
 *  @return The contents of the selection as a UTF-8 encoded string, or `NULL`
 *  if an [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED and @ref
 *  RBLXWIN_PLATFORM_ERROR.
 *
 *  @pointer_lifetime The returned string is allocated and freed by RBLXWIN. You
 *  should not free it yourself. It is valid until the next call to @ref
 *  rblxwinGetX11SelectionString or @ref rblxwinSetX11SelectionString, or until the
 *  library is terminated.
 *
 *  @thread_safety This function must only be called from the main thread.
 *
 *  @sa @ref clipboard
 *  @sa rblxwinSetX11SelectionString
 *  @sa rblxwinGetClipboardString
 *
 *  @since Added in version 3.3.
 *
 *  @ingroup native
 */
RBLXWINAPI const char* rblxwinGetX11SelectionString(void);
#endif

#if defined(RBLXWIN_EXPOSE_NATIVE_GLX)
/*! @brief Returns the `GLXContext` of the specified window.
 *
 *  @return The `GLXContext` of the specified window, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NO_WINDOW_CONTEXT and @ref
 *  RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup native
 */
RBLXWINAPI GLXContext rblxwinGetGLXContext(RBLXWINwindow* window);

/*! @brief Returns the `GLXWindow` of the specified window.
 *
 *  @return The `GLXWindow` of the specified window, or `None` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NO_WINDOW_CONTEXT and @ref
 *  RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup native
 */
RBLXWINAPI GLXWindow rblxwinGetGLXWindow(RBLXWINwindow* window);
#endif

#if defined(RBLXWIN_EXPOSE_NATIVE_WAYLAND)
/*! @brief Returns the `struct wl_display*` used by RBLXWIN.
 *
 *  @return The `struct wl_display*` used by RBLXWIN, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup native
 */
RBLXWINAPI struct wl_display* rblxwinGetWaylandDisplay(void);

/*! @brief Returns the `struct wl_output*` of the specified monitor.
 *
 *  @return The `struct wl_output*` of the specified monitor, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup native
 */
RBLXWINAPI struct wl_output* rblxwinGetWaylandMonitor(RBLXWINmonitor* monitor);

/*! @brief Returns the main `struct wl_surface*` of the specified window.
 *
 *  @return The main `struct wl_surface*` of the specified window, or `NULL` if
 *  an [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.2.
 *
 *  @ingroup native
 */
RBLXWINAPI struct wl_surface* rblxwinGetWaylandWindow(RBLXWINwindow* window);
#endif

#if defined(RBLXWIN_EXPOSE_NATIVE_EGL)
/*! @brief Returns the `EGLDisplay` used by RBLXWIN.
 *
 *  @return The `EGLDisplay` used by RBLXWIN, or `EGL_NO_DISPLAY` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NOT_INITIALIZED.
 *
 *  @remark Because EGL is initialized on demand, this function will return
 *  `EGL_NO_DISPLAY` until the first context has been created via EGL.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup native
 */
RBLXWINAPI EGLDisplay rblxwinGetEGLDisplay(void);

/*! @brief Returns the `EGLContext` of the specified window.
 *
 *  @return The `EGLContext` of the specified window, or `EGL_NO_CONTEXT` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NO_WINDOW_CONTEXT and @ref
 *  RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup native
 */
RBLXWINAPI EGLContext rblxwinGetEGLContext(RBLXWINwindow* window);

/*! @brief Returns the `EGLSurface` of the specified window.
 *
 *  @return The `EGLSurface` of the specified window, or `EGL_NO_SURFACE` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NO_WINDOW_CONTEXT and @ref
 *  RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.0.
 *
 *  @ingroup native
 */
RBLXWINAPI EGLSurface rblxwinGetEGLSurface(RBLXWINwindow* window);
#endif

#if defined(RBLXWIN_EXPOSE_NATIVE_OSMESA)
/*! @brief Retrieves the color buffer associated with the specified window.
 *
 *  @param[in] window The window whose color buffer to retrieve.
 *  @param[out] width Where to store the width of the color buffer, or `NULL`.
 *  @param[out] height Where to store the height of the color buffer, or `NULL`.
 *  @param[out] format Where to store the OSMesa pixel format of the color
 *  buffer, or `NULL`.
 *  @param[out] buffer Where to store the address of the color buffer, or
 *  `NULL`.
 *  @return `RBLXWIN_TRUE` if successful, or `RBLXWIN_FALSE` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NO_WINDOW_CONTEXT and @ref
 *  RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.3.
 *
 *  @ingroup native
 */
RBLXWINAPI int rblxwinGetOSMesaColorBuffer(RBLXWINwindow* window, int* width, int* height, int* format, void** buffer);

/*! @brief Retrieves the depth buffer associated with the specified window.
 *
 *  @param[in] window The window whose depth buffer to retrieve.
 *  @param[out] width Where to store the width of the depth buffer, or `NULL`.
 *  @param[out] height Where to store the height of the depth buffer, or `NULL`.
 *  @param[out] bytesPerValue Where to store the number of bytes per depth
 *  buffer element, or `NULL`.
 *  @param[out] buffer Where to store the address of the depth buffer, or
 *  `NULL`.
 *  @return `RBLXWIN_TRUE` if successful, or `RBLXWIN_FALSE` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NO_WINDOW_CONTEXT and @ref
 *  RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.3.
 *
 *  @ingroup native
 */
RBLXWINAPI int rblxwinGetOSMesaDepthBuffer(RBLXWINwindow* window, int* width, int* height, int* bytesPerValue, void** buffer);

/*! @brief Returns the `OSMesaContext` of the specified window.
 *
 *  @return The `OSMesaContext` of the specified window, or `NULL` if an
 *  [error](@ref error_handling) occurred.
 *
 *  @errors Possible errors include @ref RBLXWIN_NO_WINDOW_CONTEXT and @ref
 *  RBLXWIN_NOT_INITIALIZED.
 *
 *  @thread_safety This function may be called from any thread.  Access is not
 *  synchronized.
 *
 *  @since Added in version 3.3.
 *
 *  @ingroup native
 */
RBLXWINAPI OSMesaContext rblxwinGetOSMesaContext(RBLXWINwindow* window);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _rblxwin_native_h_ */

