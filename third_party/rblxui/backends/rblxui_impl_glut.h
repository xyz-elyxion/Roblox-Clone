// dear rblxui: Platform Backend for GLUT/FreeGLUT
// This needs to be used along with a Renderer (e.g. OpenGL2)

// !!! GLUT/FreeGLUT IS OBSOLETE PREHISTORIC SOFTWARE. Using GLUT is not recommended unless you really miss the 90's. !!!
// !!! If someone or something is teaching you GLUT today, you are being abused. Please show some resistance. !!!
// !!! Nowadays, prefer using RBLXWIN or SDL instead!

// Implemented features:
//  [X] Platform: Partial keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass RblxUIKey values to all key functions e.g. RblxUI::IsKeyPressed(RblxUIKey_Space). [Legacy GLUT values are obsolete since 1.87 and not supported since 1.91.5]
// Missing features or Issues:
//  [ ] Platform: GLUT is unable to distinguish e.g. Backspace from CTRL+H or TAB from CTRL+I
//  [ ] Platform: Missing horizontal mouse wheel support.
//  [ ] Platform: Missing mouse cursor shape/visibility support.
//  [ ] Platform: Missing clipboard support (not supported by Glut).
//  [ ] Platform: Missing gamepad support.
//  [ ] Platform: Missing multi-viewport support (multiple windows).

// You can use unmodified rblxui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire rblxui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear RblxUI:
// - FAQ                  https://dearrblxui.com/faq
// - Getting Started      https://dearrblxui.com/getting-started
// - Documentation        https://dearrblxui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of rblxui.cpp

#pragma once
#ifndef RBLXUI_DISABLE
#include "rblxui.h"      // RBLXUI_IMPL_API

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
RBLXUI_IMPL_API bool     RblxUI_ImplGLUT_Init();
RBLXUI_IMPL_API void     RblxUI_ImplGLUT_InstallFuncs();
RBLXUI_IMPL_API void     RblxUI_ImplGLUT_Shutdown();
RBLXUI_IMPL_API void     RblxUI_ImplGLUT_NewFrame();

// You can call RblxUI_ImplGLUT_InstallFuncs() to get all those functions installed automatically,
// or call them yourself from your own GLUT handlers. We are using the same weird names as GLUT for consistency..
//------------------------------------ GLUT name ---------------------------------------------- Decent Name ---------
RBLXUI_IMPL_API void     RblxUI_ImplGLUT_ReshapeFunc(int w, int h);                           // ~ ResizeFunc
RBLXUI_IMPL_API void     RblxUI_ImplGLUT_MotionFunc(int x, int y);                            // ~ MouseMoveFunc
RBLXUI_IMPL_API void     RblxUI_ImplGLUT_MouseFunc(int button, int state, int x, int y);      // ~ MouseButtonFunc
RBLXUI_IMPL_API void     RblxUI_ImplGLUT_MouseWheelFunc(int button, int dir, int x, int y);   // ~ MouseWheelFunc
RBLXUI_IMPL_API void     RblxUI_ImplGLUT_KeyboardFunc(unsigned char c, int x, int y);         // ~ CharPressedFunc
RBLXUI_IMPL_API void     RblxUI_ImplGLUT_KeyboardUpFunc(unsigned char c, int x, int y);       // ~ CharReleasedFunc
RBLXUI_IMPL_API void     RblxUI_ImplGLUT_SpecialFunc(int key, int x, int y);                  // ~ KeyPressedFunc
RBLXUI_IMPL_API void     RblxUI_ImplGLUT_SpecialUpFunc(int key, int x, int y);                // ~ KeyReleasedFunc

#endif // #ifndef RBLXUI_DISABLE
