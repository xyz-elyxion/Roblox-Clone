// dear linearui: Platform Backend for OSX / Cocoa
// This needs to be used along with a Renderer (e.g. OpenGL2, OpenGL3, Vulkan, Metal..)
// - Not well tested. If you want a portable application, prefer using the LINEARWIN or SDL platform Backends on Mac.
// - Requires linking with the GameController framework ("-framework GameController").

// Implemented features:
//  [X] Platform: Clipboard support is part of core Dear LinearUI (no specific code in this backend).
//  [X] Platform: Mouse support. Can discriminate Mouse/Pen.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass LinearUIKey values to all key functions e.g. LinearUI::IsKeyPressed(LinearUIKey_Space). [Legacy kVK_* values are obsolete since 1.87 and not supported since 1.91.5]
//  [X] Platform: Gamepad support.
//  [X] Platform: Mouse cursor shape and visibility (LinearUIBackendFlags_HasMouseCursors). Disable with 'io.ConfigFlags |= LinearUIConfigFlags_NoMouseCursorChange'.
//  [X] Platform: IME support.
//  [x] Platform: Multi-viewport / platform windows.
// Missing features or Issues:
//  [ ] Platform: Multi-viewport: Window size not correctly reported when enabling io.ConfigViewportsNoDecoration
//  [ ] Platform: Multi-viewport: Missing LinearUIBackendFlags_HasParentViewport support. The viewport->ParentViewportID field is ignored, and therefore io.ConfigViewportsNoDefaultParent has no effect either.

// You can use unmodified linearui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire linearui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear LinearUI:
// - FAQ                  https://dearlinearui.com/faq
// - Getting Started      https://dearlinearui.com/getting-started
// - Documentation        https://dearlinearui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of linearui.cpp

#pragma once
#include "linearui.h"      // LINEARUI_IMPL_API
#ifndef LINEARUI_DISABLE

#ifdef __OBJC__

@class NSEvent;
@class NSView;

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
LINEARUI_IMPL_API bool     LinearUI_ImplOSX_Init(NSView* _Nonnull view);
LINEARUI_IMPL_API void     LinearUI_ImplOSX_Shutdown();
LINEARUI_IMPL_API void     LinearUI_ImplOSX_NewFrame(NSView* _Nullable view);

#endif

//-----------------------------------------------------------------------------
// C++ API
//-----------------------------------------------------------------------------

#ifdef LINEARUI_IMPL_METAL_CPP_EXTENSIONS
// #include <AppKit/AppKit.hpp>
#ifndef __OBJC__

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
LINEARUI_IMPL_API bool     LinearUI_ImplOSX_Init(void* _Nonnull view);
LINEARUI_IMPL_API void     LinearUI_ImplOSX_Shutdown();
LINEARUI_IMPL_API void     LinearUI_ImplOSX_NewFrame(void* _Nullable view);

#endif
#endif

#endif // #ifndef LINEARUI_DISABLE
