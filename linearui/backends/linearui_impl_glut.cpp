// dear linearui: Platform Backend for GLUT/FreeGLUT
// This needs to be used along with a Renderer (e.g. OpenGL2)

// !!! GLUT/FreeGLUT IS OBSOLETE PREHISTORIC SOFTWARE. Using GLUT is not recommended unless you really miss the 90's. !!!
// !!! If someone or something is teaching you GLUT today, you are being abused. Please show some resistance. !!!
// !!! Nowadays, prefer using LINEARWIN or SDL instead!

// Implemented features:
//  [X] Platform: Partial keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass LinearUIKey values to all key functions e.g. LinearUI::IsKeyPressed(LinearUIKey_Space). [Legacy GLUT values are obsolete since 1.87 and not supported since 1.91.5]
// Missing features or Issues:
//  [ ] Platform: GLUT is unable to distinguish e.g. Backspace from CTRL+H or TAB from CTRL+I
//  [ ] Platform: Missing horizontal mouse wheel support.
//  [ ] Platform: Missing mouse cursor shape/visibility support.
//  [ ] Platform: Missing clipboard support (not supported by Glut).
//  [ ] Platform: Missing gamepad support.
//  [ ] Platform: Missing multi-viewport support (multiple windows).

// You can use unmodified linearui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire linearui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear LinearUI:
// - FAQ                  https://dearlinearui.com/faq
// - Getting Started      https://dearlinearui.com/getting-started
// - Documentation        https://dearlinearui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of linearui.cpp

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2023-04-17: BREAKING: Removed call to LinearUI::NewFrame() from LinearUI_ImplGLUT_NewFrame(). Needs to be called from the main application loop, like with every other backends.
//  2022-09-26: Inputs: Renamed LinearUIKey_ModXXX introduced in 1.87 to LinearUIMod_XXX (old names still supported).
//  2022-01-26: Inputs: replaced short-lived io.AddKeyModsEvent() (added two weeks ago) with io.AddKeyEvent() using LinearUIKey_ModXXX flags. Sorry for the confusion.
//  2022-01-17: Inputs: calling new io.AddMousePosEvent(), io.AddMouseButtonEvent(), io.AddMouseWheelEvent() API (1.87+).
//  2022-01-10: Inputs: calling new io.AddKeyEvent(), io.AddKeyModsEvent() + io.SetKeyEventNativeData() API (1.87+). Support for full LinearUIKey range.
//  2019-04-03: Misc: Renamed linearui_impl_freeglut.cpp/.h to linearui_impl_glut.cpp/.h.
//  2019-03-25: Misc: Made io.DeltaTime always above zero.
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-03-22: Added GLUT Platform binding.

#include "linearui.h"
#ifndef LINEARUI_DISABLE
#include "linearui_impl_glut.h"
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed (linearstb stuff)
#endif

static int g_Time = 0;          // Current time, in milliseconds

// Glut has one function for characters and one for "special keys". We map the characters in the 0..255 range and the keys above.
static LinearUIKey LinearUI_ImplGLUT_KeyToLinearUIKey(int key)
{
    switch (key)
    {
        case '\t':                      return LinearUIKey_Tab;
        case 256 + GLUT_KEY_LEFT:       return LinearUIKey_LeftArrow;
        case 256 + GLUT_KEY_RIGHT:      return LinearUIKey_RightArrow;
        case 256 + GLUT_KEY_UP:         return LinearUIKey_UpArrow;
        case 256 + GLUT_KEY_DOWN:       return LinearUIKey_DownArrow;
        case 256 + GLUT_KEY_PAGE_UP:    return LinearUIKey_PageUp;
        case 256 + GLUT_KEY_PAGE_DOWN:  return LinearUIKey_PageDown;
        case 256 + GLUT_KEY_HOME:       return LinearUIKey_Home;
        case 256 + GLUT_KEY_END:        return LinearUIKey_End;
        case 256 + GLUT_KEY_INSERT:     return LinearUIKey_Insert;
        case 127:                       return LinearUIKey_Delete;
        case 8:                         return LinearUIKey_Backspace;
        case ' ':                       return LinearUIKey_Space;
        case 13:                        return LinearUIKey_Enter;
        case 27:                        return LinearUIKey_Escape;
        case 39:                        return LinearUIKey_Apostrophe;
        case 44:                        return LinearUIKey_Comma;
        case 45:                        return LinearUIKey_Minus;
        case 46:                        return LinearUIKey_Period;
        case 47:                        return LinearUIKey_Slash;
        case 59:                        return LinearUIKey_Semicolon;
        case 61:                        return LinearUIKey_Equal;
        case 91:                        return LinearUIKey_LeftBracket;
        case 92:                        return LinearUIKey_Backslash;
        case 93:                        return LinearUIKey_RightBracket;
        case 96:                        return LinearUIKey_GraveAccent;
        //case 0:                         return LinearUIKey_CapsLock;
        //case 0:                         return LinearUIKey_ScrollLock;
        case 256 + 0x006D:              return LinearUIKey_NumLock;
        //case 0:                         return LinearUIKey_PrintScreen;
        //case 0:                         return LinearUIKey_Pause;
        //case '0':                       return LinearUIKey_Keypad0;
        //case '1':                       return LinearUIKey_Keypad1;
        //case '2':                       return LinearUIKey_Keypad2;
        //case '3':                       return LinearUIKey_Keypad3;
        //case '4':                       return LinearUIKey_Keypad4;
        //case '5':                       return LinearUIKey_Keypad5;
        //case '6':                       return LinearUIKey_Keypad6;
        //case '7':                       return LinearUIKey_Keypad7;
        //case '8':                       return LinearUIKey_Keypad8;
        //case '9':                       return LinearUIKey_Keypad9;
        //case 46:                        return LinearUIKey_KeypadDecimal;
        //case 47:                        return LinearUIKey_KeypadDivide;
        case 42:                        return LinearUIKey_KeypadMultiply;
        //case 45:                        return LinearUIKey_KeypadSubtract;
        case 43:                        return LinearUIKey_KeypadAdd;
        //case 13:                        return LinearUIKey_KeypadEnter;
        //case 0:                         return LinearUIKey_KeypadEqual;
        case 256 + 0x0072:              return LinearUIKey_LeftCtrl;
        case 256 + 0x0070:              return LinearUIKey_LeftShift;
        case 256 + 0x0074:              return LinearUIKey_LeftAlt;
        //case 0:                         return LinearUIKey_LeftSuper;
        case 256 + 0x0073:              return LinearUIKey_RightCtrl;
        case 256 + 0x0071:              return LinearUIKey_RightShift;
        case 256 + 0x0075:              return LinearUIKey_RightAlt;
        //case 0:                         return LinearUIKey_RightSuper;
        //case 0:                         return LinearUIKey_Menu;
        case '0':                       return LinearUIKey_0;
        case '1':                       return LinearUIKey_1;
        case '2':                       return LinearUIKey_2;
        case '3':                       return LinearUIKey_3;
        case '4':                       return LinearUIKey_4;
        case '5':                       return LinearUIKey_5;
        case '6':                       return LinearUIKey_6;
        case '7':                       return LinearUIKey_7;
        case '8':                       return LinearUIKey_8;
        case '9':                       return LinearUIKey_9;
        case 'A': case 'a':             return LinearUIKey_A;
        case 'B': case 'b':             return LinearUIKey_B;
        case 'C': case 'c':             return LinearUIKey_C;
        case 'D': case 'd':             return LinearUIKey_D;
        case 'E': case 'e':             return LinearUIKey_E;
        case 'F': case 'f':             return LinearUIKey_F;
        case 'G': case 'g':             return LinearUIKey_G;
        case 'H': case 'h':             return LinearUIKey_H;
        case 'I': case 'i':             return LinearUIKey_I;
        case 'J': case 'j':             return LinearUIKey_J;
        case 'K': case 'k':             return LinearUIKey_K;
        case 'L': case 'l':             return LinearUIKey_L;
        case 'M': case 'm':             return LinearUIKey_M;
        case 'N': case 'n':             return LinearUIKey_N;
        case 'O': case 'o':             return LinearUIKey_O;
        case 'P': case 'p':             return LinearUIKey_P;
        case 'Q': case 'q':             return LinearUIKey_Q;
        case 'R': case 'r':             return LinearUIKey_R;
        case 'S': case 's':             return LinearUIKey_S;
        case 'T': case 't':             return LinearUIKey_T;
        case 'U': case 'u':             return LinearUIKey_U;
        case 'V': case 'v':             return LinearUIKey_V;
        case 'W': case 'w':             return LinearUIKey_W;
        case 'X': case 'x':             return LinearUIKey_X;
        case 'Y': case 'y':             return LinearUIKey_Y;
        case 'Z': case 'z':             return LinearUIKey_Z;
        case 256 + GLUT_KEY_F1:         return LinearUIKey_F1;
        case 256 + GLUT_KEY_F2:         return LinearUIKey_F2;
        case 256 + GLUT_KEY_F3:         return LinearUIKey_F3;
        case 256 + GLUT_KEY_F4:         return LinearUIKey_F4;
        case 256 + GLUT_KEY_F5:         return LinearUIKey_F5;
        case 256 + GLUT_KEY_F6:         return LinearUIKey_F6;
        case 256 + GLUT_KEY_F7:         return LinearUIKey_F7;
        case 256 + GLUT_KEY_F8:         return LinearUIKey_F8;
        case 256 + GLUT_KEY_F9:         return LinearUIKey_F9;
        case 256 + GLUT_KEY_F10:        return LinearUIKey_F10;
        case 256 + GLUT_KEY_F11:        return LinearUIKey_F11;
        case 256 + GLUT_KEY_F12:        return LinearUIKey_F12;
        default:                        return LinearUIKey_None;
    }
}

bool LinearUI_ImplGLUT_Init()
{
    LinearUIIO& io = LinearUI::GetIO();
    LINEARUI_CHECKVERSION();

#ifdef FREEGLUT
    io.BackendPlatformName = "linearui_impl_glut (freeglut)";
#else
    io.BackendPlatformName = "linearui_impl_glut";
#endif
    g_Time = 0;

    return true;
}

void LinearUI_ImplGLUT_InstallFuncs()
{
    glutReshapeFunc(LinearUI_ImplGLUT_ReshapeFunc);
    glutMotionFunc(LinearUI_ImplGLUT_MotionFunc);
    glutPassiveMotionFunc(LinearUI_ImplGLUT_MotionFunc);
    glutMouseFunc(LinearUI_ImplGLUT_MouseFunc);
#ifdef __FREEGLUT_EXT_H__
    glutMouseWheelFunc(LinearUI_ImplGLUT_MouseWheelFunc);
#endif
    glutKeyboardFunc(LinearUI_ImplGLUT_KeyboardFunc);
    glutKeyboardUpFunc(LinearUI_ImplGLUT_KeyboardUpFunc);
    glutSpecialFunc(LinearUI_ImplGLUT_SpecialFunc);
    glutSpecialUpFunc(LinearUI_ImplGLUT_SpecialUpFunc);
}

void LinearUI_ImplGLUT_Shutdown()
{
    LinearUIIO& io = LinearUI::GetIO();
    io.BackendPlatformName = nullptr;
}

void LinearUI_ImplGLUT_NewFrame()
{
    // Setup time step
    LinearUIIO& io = LinearUI::GetIO();
    int current_time = glutGet(GLUT_ELAPSED_TIME);
    int delta_time_ms = (current_time - g_Time);
    if (delta_time_ms <= 0)
        delta_time_ms = 1;
    io.DeltaTime = delta_time_ms / 1000.0f;
    g_Time = current_time;
}

static void LinearUI_ImplGLUT_UpdateKeyModifiers()
{
    LinearUIIO& io = LinearUI::GetIO();
    int glut_key_mods = glutGetModifiers();
    io.AddKeyEvent(LinearUIMod_Ctrl, (glut_key_mods & GLUT_ACTIVE_CTRL) != 0);
    io.AddKeyEvent(LinearUIMod_Shift, (glut_key_mods & GLUT_ACTIVE_SHIFT) != 0);
    io.AddKeyEvent(LinearUIMod_Alt, (glut_key_mods & GLUT_ACTIVE_ALT) != 0);
}

static void LinearUI_ImplGLUT_AddKeyEvent(LinearUIKey key, bool down, int native_keycode)
{
    LinearUIIO& io = LinearUI::GetIO();
    io.AddKeyEvent(key, down);
    io.SetKeyEventNativeData(key, native_keycode, -1); // To support legacy indexing (<1.87 user code)
}

void LinearUI_ImplGLUT_KeyboardFunc(unsigned char c, int x, int y)
{
    // Send character to linearui
    //printf("char_down_func %d '%c'\n", c, c);
    LinearUIIO& io = LinearUI::GetIO();
    if (c >= 32)
        io.AddInputCharacter((unsigned int)c);

    LinearUIKey key = LinearUI_ImplGLUT_KeyToLinearUIKey(c);
    LinearUI_ImplGLUT_AddKeyEvent(key, true, c);
    LinearUI_ImplGLUT_UpdateKeyModifiers();
    (void)x; (void)y; // Unused
}

void LinearUI_ImplGLUT_KeyboardUpFunc(unsigned char c, int x, int y)
{
    //printf("char_up_func %d '%c'\n", c, c);
    LinearUIKey key = LinearUI_ImplGLUT_KeyToLinearUIKey(c);
    LinearUI_ImplGLUT_AddKeyEvent(key, false, c);
    LinearUI_ImplGLUT_UpdateKeyModifiers();
    (void)x; (void)y; // Unused
}

void LinearUI_ImplGLUT_SpecialFunc(int key, int x, int y)
{
    //printf("key_down_func %d\n", key);
    LinearUIKey linearui_key = LinearUI_ImplGLUT_KeyToLinearUIKey(key + 256);
    LinearUI_ImplGLUT_AddKeyEvent(linearui_key, true, key + 256);
    LinearUI_ImplGLUT_UpdateKeyModifiers();
    (void)x; (void)y; // Unused
}

void LinearUI_ImplGLUT_SpecialUpFunc(int key, int x, int y)
{
    //printf("key_up_func %d\n", key);
    LinearUIKey linearui_key = LinearUI_ImplGLUT_KeyToLinearUIKey(key + 256);
    LinearUI_ImplGLUT_AddKeyEvent(linearui_key, false, key + 256);
    LinearUI_ImplGLUT_UpdateKeyModifiers();
    (void)x; (void)y; // Unused
}

void LinearUI_ImplGLUT_MouseFunc(int glut_button, int state, int x, int y)
{
    LinearUIIO& io = LinearUI::GetIO();
    io.AddMousePosEvent((float)x, (float)y);
    int button = -1;
    if (glut_button == GLUT_LEFT_BUTTON) button = 0;
    if (glut_button == GLUT_RIGHT_BUTTON) button = 1;
    if (glut_button == GLUT_MIDDLE_BUTTON) button = 2;
    if (button != -1 && (state == GLUT_DOWN || state == GLUT_UP))
        io.AddMouseButtonEvent(button, state == GLUT_DOWN);
}

#ifdef __FREEGLUT_EXT_H__
void LinearUI_ImplGLUT_MouseWheelFunc(int button, int dir, int x, int y)
{
    LinearUIIO& io = LinearUI::GetIO();
    io.AddMousePosEvent((float)x, (float)y);
    if (dir != 0)
        io.AddMouseWheelEvent(0.0f, dir > 0 ? 1.0f : -1.0f);
    (void)button; // Unused
}
#endif

void LinearUI_ImplGLUT_ReshapeFunc(int w, int h)
{
    LinearUIIO& io = LinearUI::GetIO();
    io.DisplaySize = LinearVec2((float)w, (float)h);
}

void LinearUI_ImplGLUT_MotionFunc(int x, int y)
{
    LinearUIIO& io = LinearUI::GetIO();
    io.AddMousePosEvent((float)x, (float)y);
}

//-----------------------------------------------------------------------------

#endif // #ifndef LINEARUI_DISABLE
