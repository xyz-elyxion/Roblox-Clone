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

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2023-04-17: BREAKING: Removed call to RblxUI::NewFrame() from RblxUI_ImplGLUT_NewFrame(). Needs to be called from the main application loop, like with every other backends.
//  2022-09-26: Inputs: Renamed RblxUIKey_ModXXX introduced in 1.87 to RblxUIMod_XXX (old names still supported).
//  2022-01-26: Inputs: replaced short-lived io.AddKeyModsEvent() (added two weeks ago) with io.AddKeyEvent() using RblxUIKey_ModXXX flags. Sorry for the confusion.
//  2022-01-17: Inputs: calling new io.AddMousePosEvent(), io.AddMouseButtonEvent(), io.AddMouseWheelEvent() API (1.87+).
//  2022-01-10: Inputs: calling new io.AddKeyEvent(), io.AddKeyModsEvent() + io.SetKeyEventNativeData() API (1.87+). Support for full RblxUIKey range.
//  2019-04-03: Misc: Renamed rblxui_impl_freeglut.cpp/.h to rblxui_impl_glut.cpp/.h.
//  2019-03-25: Misc: Made io.DeltaTime always above zero.
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-03-22: Added GLUT Platform binding.

#include "rblxui.h"
#ifndef RBLXUI_DISABLE
#include "rblxui_impl_glut.h"
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed (rblxstb stuff)
#endif

static int g_Time = 0;          // Current time, in milliseconds

// Glut has one function for characters and one for "special keys". We map the characters in the 0..255 range and the keys above.
static RblxUIKey RblxUI_ImplGLUT_KeyToRblxUIKey(int key)
{
    switch (key)
    {
        case '\t':                      return RblxUIKey_Tab;
        case 256 + GLUT_KEY_LEFT:       return RblxUIKey_LeftArrow;
        case 256 + GLUT_KEY_RIGHT:      return RblxUIKey_RightArrow;
        case 256 + GLUT_KEY_UP:         return RblxUIKey_UpArrow;
        case 256 + GLUT_KEY_DOWN:       return RblxUIKey_DownArrow;
        case 256 + GLUT_KEY_PAGE_UP:    return RblxUIKey_PageUp;
        case 256 + GLUT_KEY_PAGE_DOWN:  return RblxUIKey_PageDown;
        case 256 + GLUT_KEY_HOME:       return RblxUIKey_Home;
        case 256 + GLUT_KEY_END:        return RblxUIKey_End;
        case 256 + GLUT_KEY_INSERT:     return RblxUIKey_Insert;
        case 127:                       return RblxUIKey_Delete;
        case 8:                         return RblxUIKey_Backspace;
        case ' ':                       return RblxUIKey_Space;
        case 13:                        return RblxUIKey_Enter;
        case 27:                        return RblxUIKey_Escape;
        case 39:                        return RblxUIKey_Apostrophe;
        case 44:                        return RblxUIKey_Comma;
        case 45:                        return RblxUIKey_Minus;
        case 46:                        return RblxUIKey_Period;
        case 47:                        return RblxUIKey_Slash;
        case 59:                        return RblxUIKey_Semicolon;
        case 61:                        return RblxUIKey_Equal;
        case 91:                        return RblxUIKey_LeftBracket;
        case 92:                        return RblxUIKey_Backslash;
        case 93:                        return RblxUIKey_RightBracket;
        case 96:                        return RblxUIKey_GraveAccent;
        //case 0:                         return RblxUIKey_CapsLock;
        //case 0:                         return RblxUIKey_ScrollLock;
        case 256 + 0x006D:              return RblxUIKey_NumLock;
        //case 0:                         return RblxUIKey_PrintScreen;
        //case 0:                         return RblxUIKey_Pause;
        //case '0':                       return RblxUIKey_Keypad0;
        //case '1':                       return RblxUIKey_Keypad1;
        //case '2':                       return RblxUIKey_Keypad2;
        //case '3':                       return RblxUIKey_Keypad3;
        //case '4':                       return RblxUIKey_Keypad4;
        //case '5':                       return RblxUIKey_Keypad5;
        //case '6':                       return RblxUIKey_Keypad6;
        //case '7':                       return RblxUIKey_Keypad7;
        //case '8':                       return RblxUIKey_Keypad8;
        //case '9':                       return RblxUIKey_Keypad9;
        //case 46:                        return RblxUIKey_KeypadDecimal;
        //case 47:                        return RblxUIKey_KeypadDivide;
        case 42:                        return RblxUIKey_KeypadMultiply;
        //case 45:                        return RblxUIKey_KeypadSubtract;
        case 43:                        return RblxUIKey_KeypadAdd;
        //case 13:                        return RblxUIKey_KeypadEnter;
        //case 0:                         return RblxUIKey_KeypadEqual;
        case 256 + 0x0072:              return RblxUIKey_LeftCtrl;
        case 256 + 0x0070:              return RblxUIKey_LeftShift;
        case 256 + 0x0074:              return RblxUIKey_LeftAlt;
        //case 0:                         return RblxUIKey_LeftSuper;
        case 256 + 0x0073:              return RblxUIKey_RightCtrl;
        case 256 + 0x0071:              return RblxUIKey_RightShift;
        case 256 + 0x0075:              return RblxUIKey_RightAlt;
        //case 0:                         return RblxUIKey_RightSuper;
        //case 0:                         return RblxUIKey_Menu;
        case '0':                       return RblxUIKey_0;
        case '1':                       return RblxUIKey_1;
        case '2':                       return RblxUIKey_2;
        case '3':                       return RblxUIKey_3;
        case '4':                       return RblxUIKey_4;
        case '5':                       return RblxUIKey_5;
        case '6':                       return RblxUIKey_6;
        case '7':                       return RblxUIKey_7;
        case '8':                       return RblxUIKey_8;
        case '9':                       return RblxUIKey_9;
        case 'A': case 'a':             return RblxUIKey_A;
        case 'B': case 'b':             return RblxUIKey_B;
        case 'C': case 'c':             return RblxUIKey_C;
        case 'D': case 'd':             return RblxUIKey_D;
        case 'E': case 'e':             return RblxUIKey_E;
        case 'F': case 'f':             return RblxUIKey_F;
        case 'G': case 'g':             return RblxUIKey_G;
        case 'H': case 'h':             return RblxUIKey_H;
        case 'I': case 'i':             return RblxUIKey_I;
        case 'J': case 'j':             return RblxUIKey_J;
        case 'K': case 'k':             return RblxUIKey_K;
        case 'L': case 'l':             return RblxUIKey_L;
        case 'M': case 'm':             return RblxUIKey_M;
        case 'N': case 'n':             return RblxUIKey_N;
        case 'O': case 'o':             return RblxUIKey_O;
        case 'P': case 'p':             return RblxUIKey_P;
        case 'Q': case 'q':             return RblxUIKey_Q;
        case 'R': case 'r':             return RblxUIKey_R;
        case 'S': case 's':             return RblxUIKey_S;
        case 'T': case 't':             return RblxUIKey_T;
        case 'U': case 'u':             return RblxUIKey_U;
        case 'V': case 'v':             return RblxUIKey_V;
        case 'W': case 'w':             return RblxUIKey_W;
        case 'X': case 'x':             return RblxUIKey_X;
        case 'Y': case 'y':             return RblxUIKey_Y;
        case 'Z': case 'z':             return RblxUIKey_Z;
        case 256 + GLUT_KEY_F1:         return RblxUIKey_F1;
        case 256 + GLUT_KEY_F2:         return RblxUIKey_F2;
        case 256 + GLUT_KEY_F3:         return RblxUIKey_F3;
        case 256 + GLUT_KEY_F4:         return RblxUIKey_F4;
        case 256 + GLUT_KEY_F5:         return RblxUIKey_F5;
        case 256 + GLUT_KEY_F6:         return RblxUIKey_F6;
        case 256 + GLUT_KEY_F7:         return RblxUIKey_F7;
        case 256 + GLUT_KEY_F8:         return RblxUIKey_F8;
        case 256 + GLUT_KEY_F9:         return RblxUIKey_F9;
        case 256 + GLUT_KEY_F10:        return RblxUIKey_F10;
        case 256 + GLUT_KEY_F11:        return RblxUIKey_F11;
        case 256 + GLUT_KEY_F12:        return RblxUIKey_F12;
        default:                        return RblxUIKey_None;
    }
}

bool RblxUI_ImplGLUT_Init()
{
    RblxUIIO& io = RblxUI::GetIO();
    RBLXUI_CHECKVERSION();

#ifdef FREEGLUT
    io.BackendPlatformName = "rblxui_impl_glut (freeglut)";
#else
    io.BackendPlatformName = "rblxui_impl_glut";
#endif
    g_Time = 0;

    return true;
}

void RblxUI_ImplGLUT_InstallFuncs()
{
    glutReshapeFunc(RblxUI_ImplGLUT_ReshapeFunc);
    glutMotionFunc(RblxUI_ImplGLUT_MotionFunc);
    glutPassiveMotionFunc(RblxUI_ImplGLUT_MotionFunc);
    glutMouseFunc(RblxUI_ImplGLUT_MouseFunc);
#ifdef __FREEGLUT_EXT_H__
    glutMouseWheelFunc(RblxUI_ImplGLUT_MouseWheelFunc);
#endif
    glutKeyboardFunc(RblxUI_ImplGLUT_KeyboardFunc);
    glutKeyboardUpFunc(RblxUI_ImplGLUT_KeyboardUpFunc);
    glutSpecialFunc(RblxUI_ImplGLUT_SpecialFunc);
    glutSpecialUpFunc(RblxUI_ImplGLUT_SpecialUpFunc);
}

void RblxUI_ImplGLUT_Shutdown()
{
    RblxUIIO& io = RblxUI::GetIO();
    io.BackendPlatformName = nullptr;
}

void RblxUI_ImplGLUT_NewFrame()
{
    // Setup time step
    RblxUIIO& io = RblxUI::GetIO();
    int current_time = glutGet(GLUT_ELAPSED_TIME);
    int delta_time_ms = (current_time - g_Time);
    if (delta_time_ms <= 0)
        delta_time_ms = 1;
    io.DeltaTime = delta_time_ms / 1000.0f;
    g_Time = current_time;
}

static void RblxUI_ImplGLUT_UpdateKeyModifiers()
{
    RblxUIIO& io = RblxUI::GetIO();
    int glut_key_mods = glutGetModifiers();
    io.AddKeyEvent(RblxUIMod_Ctrl, (glut_key_mods & GLUT_ACTIVE_CTRL) != 0);
    io.AddKeyEvent(RblxUIMod_Shift, (glut_key_mods & GLUT_ACTIVE_SHIFT) != 0);
    io.AddKeyEvent(RblxUIMod_Alt, (glut_key_mods & GLUT_ACTIVE_ALT) != 0);
}

static void RblxUI_ImplGLUT_AddKeyEvent(RblxUIKey key, bool down, int native_keycode)
{
    RblxUIIO& io = RblxUI::GetIO();
    io.AddKeyEvent(key, down);
    io.SetKeyEventNativeData(key, native_keycode, -1); // To support legacy indexing (<1.87 user code)
}

void RblxUI_ImplGLUT_KeyboardFunc(unsigned char c, int x, int y)
{
    // Send character to rblxui
    //printf("char_down_func %d '%c'\n", c, c);
    RblxUIIO& io = RblxUI::GetIO();
    if (c >= 32)
        io.AddInputCharacter((unsigned int)c);

    RblxUIKey key = RblxUI_ImplGLUT_KeyToRblxUIKey(c);
    RblxUI_ImplGLUT_AddKeyEvent(key, true, c);
    RblxUI_ImplGLUT_UpdateKeyModifiers();
    (void)x; (void)y; // Unused
}

void RblxUI_ImplGLUT_KeyboardUpFunc(unsigned char c, int x, int y)
{
    //printf("char_up_func %d '%c'\n", c, c);
    RblxUIKey key = RblxUI_ImplGLUT_KeyToRblxUIKey(c);
    RblxUI_ImplGLUT_AddKeyEvent(key, false, c);
    RblxUI_ImplGLUT_UpdateKeyModifiers();
    (void)x; (void)y; // Unused
}

void RblxUI_ImplGLUT_SpecialFunc(int key, int x, int y)
{
    //printf("key_down_func %d\n", key);
    RblxUIKey rblxui_key = RblxUI_ImplGLUT_KeyToRblxUIKey(key + 256);
    RblxUI_ImplGLUT_AddKeyEvent(rblxui_key, true, key + 256);
    RblxUI_ImplGLUT_UpdateKeyModifiers();
    (void)x; (void)y; // Unused
}

void RblxUI_ImplGLUT_SpecialUpFunc(int key, int x, int y)
{
    //printf("key_up_func %d\n", key);
    RblxUIKey rblxui_key = RblxUI_ImplGLUT_KeyToRblxUIKey(key + 256);
    RblxUI_ImplGLUT_AddKeyEvent(rblxui_key, false, key + 256);
    RblxUI_ImplGLUT_UpdateKeyModifiers();
    (void)x; (void)y; // Unused
}

void RblxUI_ImplGLUT_MouseFunc(int glut_button, int state, int x, int y)
{
    RblxUIIO& io = RblxUI::GetIO();
    io.AddMousePosEvent((float)x, (float)y);
    int button = -1;
    if (glut_button == GLUT_LEFT_BUTTON) button = 0;
    if (glut_button == GLUT_RIGHT_BUTTON) button = 1;
    if (glut_button == GLUT_MIDDLE_BUTTON) button = 2;
    if (button != -1 && (state == GLUT_DOWN || state == GLUT_UP))
        io.AddMouseButtonEvent(button, state == GLUT_DOWN);
}

#ifdef __FREEGLUT_EXT_H__
void RblxUI_ImplGLUT_MouseWheelFunc(int button, int dir, int x, int y)
{
    RblxUIIO& io = RblxUI::GetIO();
    io.AddMousePosEvent((float)x, (float)y);
    if (dir != 0)
        io.AddMouseWheelEvent(0.0f, dir > 0 ? 1.0f : -1.0f);
    (void)button; // Unused
}
#endif

void RblxUI_ImplGLUT_ReshapeFunc(int w, int h)
{
    RblxUIIO& io = RblxUI::GetIO();
    io.DisplaySize = RblxVec2((float)w, (float)h);
}

void RblxUI_ImplGLUT_MotionFunc(int x, int y)
{
    RblxUIIO& io = RblxUI::GetIO();
    io.AddMousePosEvent((float)x, (float)y);
}

//-----------------------------------------------------------------------------

#endif // #ifndef RBLXUI_DISABLE
