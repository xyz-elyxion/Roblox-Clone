// dear linearui, v1.92.9 WIP
// (demo code)

// Help:
// - Read FAQ at http://dearlinearui.com/faq
// - Call and read LinearUI::ShowDemoWindow() in linearui_demo.cpp. All applications in examples/ are doing that.
// - Need help integrating Dear LinearUI in your codebase?
//   - Read Getting Started https://github.com/ocornut/linearui/wiki/Getting-Started
//   - Read 'Programmer guide' in linearui.cpp for notes on how to setup Dear LinearUI in your codebase.
// Read top of linearui.cpp and linearui.h for many details, documentation, comments, links.
// Get the latest version at https://github.com/ocornut/linearui

// How to easily locate code?
// - Use Tools->Item Picker to debug break in code by clicking any widgets: https://github.com/ocornut/linearui/wiki/Debug-Tools
// - Browse pthom's online linearui_explorer: web version the demo w/ source code browser: https://pthom.github.io/linearui_explorer
// - Find a visible string and search for it in the code!

//---------------------------------------------------
// PLEASE DO NOT REMOVE THIS FILE FROM YOUR PROJECT!
//---------------------------------------------------
// Message to the person tempted to delete this file when integrating Dear LinearUI into their codebase:
// Think again! It is the most useful reference code that you and other coders will want to refer to and call.
// Have the LinearUI::ShowDemoWindow() function wired in an always-available debug menu of your game/app!
// Also include Metrics! ItemPicker! DebugLog! and other debug features.
// Removing this file from your project is hindering access to documentation for everyone in your team,
// likely leading you to poorer usage of the library.
// Everything in this file will be stripped out by the linker if you don't call LinearUI::ShowDemoWindow().
// If you want to link core Dear LinearUI in your shipped builds but want a thorough guarantee that the demo will not be
// linked, you can setup your linearuiconfig.h with #define LINEARUI_DISABLE_DEMO_WINDOWS and those functions will be empty.
// In another situation, whenever you have Dear LinearUI available you probably want this to be available for reference.
// Thank you,
// -Your beloved friend, linearui_demo.cpp (which you won't delete)

//--------------------------------------------
// ABOUT THE MEANING OF THE 'static' KEYWORD:
//--------------------------------------------
// In this demo code, we frequently use 'static' variables inside functions.
// A static variable persists across calls. It is essentially a global variable but declared inside the scope of the function.
// Think of "static int n = 0;" as "global int n = 0;" !
// We do this IN THE DEMO because we want:
// - to gather code and data in the same place.
// - to make the demo source code faster to read, faster to change, smaller in size.
// - it is also a convenient way of storing simple UI related information as long as your function
//   doesn't need to be reentrant or used in multiple threads.
// This might be a pattern you will want to use in your code, but most of the data you would be working
// with in a complex codebase is likely going to be stored outside your functions.

//-----------------------------------------
// ABOUT THE CODING STYLE OF OUR DEMO CODE
//-----------------------------------------
// The Demo code in this file is designed to be easy to copy-and-paste into your application!
// Because of this:
// - We never omit the LinearUI:: prefix when calling functions, even though most code here is in the same namespace.
// - We try to declare static variables in the local scope, as close as possible to the code using them.
// - We never use any of the helpers/facilities used internally by Dear LinearUI, unless available in the public API.
// - We never use maths operators on LinearVec2/LinearVec4. For our other sources files we use them, and they are provided
//   by linearui.h using the LINEARUI_DEFINE_MATH_OPERATORS define. For your own sources file they are optional
//   and require you either enable those, either provide your own via LINEARUI_VEC2_CLASS_EXTRA in linearuiconfig.h.
//   Because we can't assume anything about your support of maths operators, we cannot use them in linearui_demo.cpp.

// Navigating this file:
// - In Visual Studio: Ctrl+Comma ("Edit.GoToAll") can follow symbols inside comments, whereas Ctrl+F12 ("Edit.GoToImplementation") cannot.
// - In Visual Studio w/ Visual Assist installed: Alt+G ("VAssistX.GoToImplementation") can also follow symbols inside comments.
// - In VS Code, CLion, etc.: Ctrl+Click can follow symbols inside comments.
// - You can search/grep for all sections listed in the index to find the section.

/*

Index of this file:

// [SECTION] Forward Declarations
// [SECTION] Helpers
// [SECTION] Demo Window / ShowDemoWindow()
// [SECTION] DemoWindowMenuBar()
// [SECTION] Helpers: ExampleTreeNode, ExampleMemberInfo (for use by Property Editor & Multi-Select demos)
// [SECTION] Helpers: ExampleImageViewer
// [SECTION] DemoWindowWidgetsBasic()
// [SECTION] DemoWindowWidgetsBullets()
// [SECTION] DemoWindowWidgetsCollapsingHeaders()
// [SECTION] DemoWindowWidgetsComboBoxes()
// [SECTION] DemoWindowWidgetsColorAndPickers()
// [SECTION] DemoWindowWidgetsDataTypes()
// [SECTION] DemoWindowWidgetsDisableBlocks()
// [SECTION] DemoWindowWidgetsDragAndDrop()
// [SECTION] DemoWindowWidgetsDragsAndSliders()
// [SECTION] DemoWindowWidgetsFonts()
// [SECTION] DemoWindowWidgetsImages()
// [SECTION] DemoWindowWidgetsListBoxes()
// [SECTION] DemoWindowWidgetsMultiComponents()
// [SECTION] DemoWindowWidgetsPlotting()
// [SECTION] DemoWindowWidgetsProgressBars()
// [SECTION] DemoWindowWidgetsQueryingStatuses()
// [SECTION] DemoWindowWidgetsSelectables()
// [SECTION] DemoWindowWidgetsSelectionAndMultiSelect()
// [SECTION] DemoWindowWidgetsTabs()
// [SECTION] DemoWindowWidgetsText()
// [SECTION] DemoWindowWidgetsTextFilter()
// [SECTION] DemoWindowWidgetsTextInput()
// [SECTION] DemoWindowWidgetsTooltips()
// [SECTION] DemoWindowWidgetsTreeNodes()
// [SECTION] DemoWindowWidgetsVerticalSliders()
// [SECTION] DemoWindowWidgets()
// [SECTION] DemoWindowLayout()
// [SECTION] DemoWindowPopups()
// [SECTION] DemoWindowTables()
// [SECTION] DemoWindowInputs()
// [SECTION] About Window / ShowAboutWindow()
// [SECTION] Style Editor / ShowStyleEditor()
// [SECTION] User Guide / ShowUserGuide()
// [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
// [SECTION] Example App: Image Viewer / ShowExampleAppImageViewer()
// [SECTION] Example App: Debug Log / ShowExampleAppLog()
// [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
// [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
// [SECTION] Example App: Long Text / ShowExampleAppLongText()
// [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
// [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
// [SECTION] Example App: Simple overlay / ShowExampleAppSimpleOverlay()
// [SECTION] Example App: Fullscreen window / ShowExampleAppFullscreen()
// [SECTION] Example App: Manipulating window titles / ShowExampleAppWindowTitles()
// [SECTION] Example App: Custom Rendering using LinearDrawList API / ShowExampleAppCustomRendering()
// [SECTION] Example App: Docking, DockSpace / ShowExampleAppDockSpace()
// [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()
// [SECTION] Example App: Assets Browser / ShowExampleAppAssetsBrowser()

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "linearui.h"
#ifndef LINEARUI_DISABLE

// System includes
#include <ctype.h>          // toupper
#include <limits.h>         // INT_MIN, INT_MAX
#include <math.h>           // sqrtf, powf, cosf, sinf, floorf, ceilf
#include <stdio.h>          // vsnprintf, sscanf, printf
#include <stdlib.h>         // NULL, malloc, free, atoi
#include <stdint.h>         // intptr_t
#if !defined(_MSC_VER) || _MSC_VER >= 1800
#include <inttypes.h>       // PRId64/PRIu64, not avail in some MinGW headers.
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten/version.h>     // __EMSCRIPTEN_MAJOR__ etc.
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127)     // condition expression is constant
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#pragma warning (disable: 26451)    // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to an 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'                     // not all warnings are known by all Clang versions and they tend to be rename-happy.. so ignoring warnings triggers new warnings on some configuration. Great!
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast                           // yes, they are more terse.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"        // warning: 'xx' is deprecated: The POSIX name for this..   // for strdup used in demo code (so user can copy & paste the code)
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"       // warning: cast to 'void *' from smaller integer type
#pragma clang diagnostic ignored "-Wformat"                         // warning: format specifies type 'int' but the argument has type 'unsigned int'
#pragma clang diagnostic ignored "-Wformat-security"                // warning: format string is not a string literal
#pragma clang diagnostic ignored "-Wexit-time-destructors"          // warning: declaration requires an exit-time destructor    // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. LinearUI coding style welcomes static/globals.
#pragma clang diagnostic ignored "-Wunused-macros"                  // warning: macro is not used                               // we define snprintf/vsnprintf on Windows so they are available, but not always used.
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant                   // some standard header variations use #define NULL 0
#pragma clang diagnostic ignored "-Wdouble-promotion"               // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#pragma clang diagnostic ignored "-Wreserved-id-macro"              // warning: macro name is a reserved identifier
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"            // warning: 'xxx' is an unsafe pointer used for buffer access
#pragma clang diagnostic ignored "-Wswitch-default"                 // warning: 'switch' missing 'default' label
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                          // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wfloat-equal"                      // warning: comparing floating-point with '==' or '!=' is unsafe
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"              // warning: cast to pointer from integer of different size
#pragma GCC diagnostic ignored "-Wformat"                           // warning: format '%p' expects argument of type 'int'/'void*', but argument X has type 'unsigned int'/'LinearUIWindow*'
#pragma GCC diagnostic ignored "-Wformat-security"                  // warning: format string is not a string literal (potentially insecure)
#pragma GCC diagnostic ignored "-Wdouble-promotion"                 // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"                       // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wmisleading-indentation"           // [__GNUC__ >= 6] warning: this 'if' clause does not guard this statement      // GCC 6.0+ only. See #883 on GitHub.
#pragma GCC diagnostic ignored "-Wstrict-overflow"                  // warning: assuming signed overflow does not occur when simplifying division / ..when changing X +- C1 cmp C2 to X cmp C2 -+ C1
#pragma GCC diagnostic ignored "-Wcast-qual"                        // warning: cast from type 'const xxxx *' to type 'xxxx *' casts away qualifiers
#endif

// Play it nice with Windows users (Update: May 2018, Notepad now supports Unix-style carriage returns!)
#ifdef _WIN32
#define LINEARUI_NEWLINE  "\r\n"
#else
#define LINEARUI_NEWLINE  "\n"
#endif

// Helpers
#if defined(_MSC_VER) && !defined(snprintf)
#define snprintf    _snprintf
#endif
#if defined(_MSC_VER) && !defined(vsnprintf)
#define vsnprintf   _vsnprintf
#endif

// Format specifiers for 64-bit values (hasn't been decently standardized before VS2013)
#if !defined(PRId64) && defined(_MSC_VER)
#define PRId64 "I64d"
#define PRIu64 "I64u"
#elif !defined(PRId64)
#define PRId64 "lld"
#define PRIu64 "llu"
#endif

// Helpers macros
// We normally try to not use many helpers in linearui_demo.cpp in order to make code easier to copy and paste,
// but making an exception here as those are largely simplifying code...
// In other linearui sources we can use nicer internal functions from linearui_internal.h (LinearMin/LinearMax) but not in the demo.
#define LINEARUI_MIN(A, B)            (((A) < (B)) ? (A) : (B))
#define LINEARUI_MAX(A, B)            (((A) >= (B)) ? (A) : (B))
#define LINEARUI_CLAMP(V, MN, MX)     ((V) < (MN) ? (MN) : (V) > (MX) ? (MX) : (V))

// Enforce cdecl calling convention for functions called by the standard library,
// in case compilation settings changed the default to e.g. __vectorcall
#ifndef LINEARUI_CDECL
#ifdef _MSC_VER
#define LINEARUI_CDECL __cdecl
#else
#define LINEARUI_CDECL
#endif
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward Declarations
//-----------------------------------------------------------------------------

#if !defined(LINEARUI_DISABLE_DEMO_WINDOWS)

// Forward Declarations
struct LinearUIDemoWindowData;
static void ShowExampleAppMainMenuBar();
static void ShowExampleAppAssetsBrowser(bool* p_open);
static void ShowExampleAppConsole(bool* p_open);
static void ShowExampleAppCustomRendering(bool* p_open);
static void ShowExampleAppDockSpace(bool* p_open);
static void ShowExampleAppDocuments(bool* p_open);
static void ShowExampleAppImageViewer(bool* p_open);
static void ShowExampleAppLog(bool* p_open);
static void ShowExampleAppLayout(bool* p_open);
static void ShowExampleAppPropertyEditor(bool* p_open, LinearUIDemoWindowData* demo_data);
static void ShowExampleAppSimpleOverlay(bool* p_open);
static void ShowExampleAppAutoResize(bool* p_open);
static void ShowExampleAppConstrainedResize(bool* p_open);
static void ShowExampleAppFullscreen(bool* p_open);
static void ShowExampleAppLongText(bool* p_open);
static void ShowExampleAppWindowTitles(bool* p_open);
static void ShowExampleMenuFile();

// We split the contents of the big ShowDemoWindow() function into smaller functions
// (because the link time of very large functions tends to grow non-linearly)
static void DemoWindowMenuBar(LinearUIDemoWindowData* demo_data);
static void DemoWindowWidgets(LinearUIDemoWindowData* demo_data);
static void DemoWindowLayout();
static void DemoWindowPopups();
static void DemoWindowTables();
static void DemoWindowColumns();
static void DemoWindowInputs();

// Helper tree functions used by Property Editor & Multi-Select demos
struct ExampleTreeNode;
static ExampleTreeNode* ExampleTree_CreateNode(const char* name, int uid, ExampleTreeNode* parent);
static void             ExampleTree_DestroyNode(ExampleTreeNode* node);

//-----------------------------------------------------------------------------
// [SECTION] Helpers
//-----------------------------------------------------------------------------

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
    LinearUI::TextDisabled("(?)");
    if (LinearUI::BeginItemTooltip())
    {
        LinearUI::PushTextWrapPos(LinearUI::GetFontSize() * 35.0f);
        LinearUI::TextUnformatted(desc);
        LinearUI::PopTextWrapPos();
        LinearUI::EndTooltip();
    }
}

static void ShowDockingDisabledMessage()
{
    LinearUIIO& io = LinearUI::GetIO();
    LinearUI::Text("ERROR: Docking is not enabled! See Demo > Configuration.");
    LinearUI::Text("Set io.ConfigFlags |= LinearUIConfigFlags_DockingEnable in your code, or ");
    LinearUI::SameLine(0.0f, 0.0f);
    if (LinearUI::SmallButton("click here"))
        io.ConfigFlags |= LinearUIConfigFlags_DockingEnable;
}

// Helper to wire demo markers located in code to an interactive browser (e.g. https://pthom.github.io/linearui_explorer)
#if LINEARUI_VERSION_NUM >= 19263
namespace LinearUI { extern LINEARUI_API void DemoMarker(const char* file, int line, const char* section); }
#define LINEARUI_DEMO_MARKER(section)  do { LinearUI::DemoMarker("linearui_demo.cpp", __LINE__, section); } while (0)
#endif

// Sneakily forward declare functions which aren't worth putting in public API yet
namespace LinearUI
{
    LINEARUI_API void ShowFontAtlas(LinearFontAtlas* atlas);
    LINEARUI_API void TreeNodeSetOpen(LinearUIID storage_id, bool is_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Demo Window / ShowDemoWindow()
//-----------------------------------------------------------------------------

// Data to be shared across different functions of the demo.
struct LinearUIDemoWindowData
{
    // Examples Apps (accessible from the "Examples" menu)
    bool ShowMainMenuBar = false;
    bool ShowAppAssetsBrowser = false;
    bool ShowAppConsole = false;
    bool ShowAppCustomRendering = false;
    bool ShowAppDocuments = false;
    bool ShowAppDockSpace = false;
    bool ShowAppImageViewer = false;
    bool ShowAppLog = false;
    bool ShowAppLayout = false;
    bool ShowAppPropertyEditor = false;
    bool ShowAppSimpleOverlay = false;
    bool ShowAppAutoResize = false;
    bool ShowAppConstrainedResize = false;
    bool ShowAppFullscreen = false;
    bool ShowAppLongText = false;
    bool ShowAppWindowTitles = false;

    // Dear LinearUI Tools (accessible from the "Tools" menu)
    bool ShowMetrics = false;
    bool ShowDebugLog = false;
    bool ShowIDStackTool = false;
    bool ShowStyleEditor = false;
    bool ShowAbout = false;

    // Other data
    bool DisableSections = false;
    ExampleTreeNode* DemoTree = NULL;

    ~LinearUIDemoWindowData() { if (DemoTree) ExampleTree_DestroyNode(DemoTree); }
};

// Demonstrate most Dear LinearUI features (this is big function!)
// You may execute this function to experiment with the UI and understand what it does.
// You may then search for keywords in the code when you are interested by a specific feature.
void LinearUI::ShowDemoWindow(bool* p_open)
{
    // Exceptionally add an extra assert here for people confused about initial Dear LinearUI setup
    // Most functions would normally just assert/crash if the context is missing.
    LINEARUI_ASSERT(LinearUI::GetCurrentContext() != NULL && "Missing Dear LinearUI context. Refer to examples app!");

    // Verify ABI compatibility between caller code and compiled version of Dear LinearUI. This helps detects some build issues.
    LINEARUI_CHECKVERSION();

    // Stored data
    static LinearUIDemoWindowData demo_data;

    // Examples Apps (accessible from the "Examples" menu)
    if (demo_data.ShowMainMenuBar)          { ShowExampleAppMainMenuBar(); }
    if (demo_data.ShowAppDockSpace)         { ShowExampleAppDockSpace(&demo_data.ShowAppDockSpace); } // Important: Process the Docking app first, as explicit DockSpace() nodes needs to be submitted early (read comments near the DockSpace function)
    if (demo_data.ShowAppDocuments)         { ShowExampleAppDocuments(&demo_data.ShowAppDocuments); } // ...process the Document app next, as it may also use a DockSpace()
    if (demo_data.ShowAppAssetsBrowser)     { ShowExampleAppAssetsBrowser(&demo_data.ShowAppAssetsBrowser); }
    if (demo_data.ShowAppConsole)           { ShowExampleAppConsole(&demo_data.ShowAppConsole); }
    if (demo_data.ShowAppCustomRendering)   { ShowExampleAppCustomRendering(&demo_data.ShowAppCustomRendering); }
    if (demo_data.ShowAppImageViewer)       { ShowExampleAppImageViewer(&demo_data.ShowAppImageViewer); }
    if (demo_data.ShowAppLog)               { ShowExampleAppLog(&demo_data.ShowAppLog); }
    if (demo_data.ShowAppLayout)            { ShowExampleAppLayout(&demo_data.ShowAppLayout); }
    if (demo_data.ShowAppPropertyEditor)    { ShowExampleAppPropertyEditor(&demo_data.ShowAppPropertyEditor, &demo_data); }
    if (demo_data.ShowAppSimpleOverlay)     { ShowExampleAppSimpleOverlay(&demo_data.ShowAppSimpleOverlay); }
    if (demo_data.ShowAppAutoResize)        { ShowExampleAppAutoResize(&demo_data.ShowAppAutoResize); }
    if (demo_data.ShowAppConstrainedResize) { ShowExampleAppConstrainedResize(&demo_data.ShowAppConstrainedResize); }
    if (demo_data.ShowAppFullscreen)        { ShowExampleAppFullscreen(&demo_data.ShowAppFullscreen); }
    if (demo_data.ShowAppLongText)          { ShowExampleAppLongText(&demo_data.ShowAppLongText); }
    if (demo_data.ShowAppWindowTitles)      { ShowExampleAppWindowTitles(&demo_data.ShowAppWindowTitles); }

    // Dear LinearUI Tools (accessible from the "Tools" menu)
    if (demo_data.ShowMetrics)              { LinearUI::ShowMetricsWindow(&demo_data.ShowMetrics); }
    if (demo_data.ShowDebugLog)             { LinearUI::ShowDebugLogWindow(&demo_data.ShowDebugLog); }
    if (demo_data.ShowIDStackTool)          { LinearUI::ShowIDStackToolWindow(&demo_data.ShowIDStackTool); }
    if (demo_data.ShowAbout)                { LinearUI::ShowAboutWindow(&demo_data.ShowAbout); }
    if (demo_data.ShowStyleEditor)
    {
        LinearUI::Begin("Dear LinearUI Style Editor", &demo_data.ShowStyleEditor);
        LinearUI::ShowStyleEditor();
        LinearUI::End();
    }

    // Demonstrate the various window flags. Typically you would just use the default!
    static bool no_titlebar = false;
    static bool no_scrollbar = false;
    static bool no_menu = false;
    static bool no_move = false;
    static bool no_resize = false;
    static bool no_collapse = false;
    static bool no_close = false;
    static bool no_nav = false;
    static bool no_background = false;
    static bool no_bring_to_front = false;
    static bool no_docking = false;
    static bool unsaved_document = false;

    LinearUIWindowFlags window_flags = 0;
    if (no_titlebar)        window_flags |= LinearUIWindowFlags_NoTitleBar;
    if (no_scrollbar)       window_flags |= LinearUIWindowFlags_NoScrollbar;
    if (!no_menu)           window_flags |= LinearUIWindowFlags_MenuBar;
    if (no_move)            window_flags |= LinearUIWindowFlags_NoMove;
    if (no_resize)          window_flags |= LinearUIWindowFlags_NoResize;
    if (no_collapse)        window_flags |= LinearUIWindowFlags_NoCollapse;
    if (no_nav)             window_flags |= LinearUIWindowFlags_NoNav;
    if (no_background)      window_flags |= LinearUIWindowFlags_NoBackground;
    if (no_bring_to_front)  window_flags |= LinearUIWindowFlags_NoBringToFrontOnFocus;
    if (no_docking)         window_flags |= LinearUIWindowFlags_NoDocking;
    if (unsaved_document)   window_flags |= LinearUIWindowFlags_UnsavedDocument;
    if (no_close)           p_open = NULL; // Don't pass our bool* to Begin

    // We specify a default position/size in case there's no data in the .ini file.
    // We only do it to make the demo applications a little more welcoming, but typically this isn't required.
    const LinearUIViewport* main_viewport = LinearUI::GetMainViewport();
    LinearUI::SetNextWindowPos(LinearVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), LinearUICond_FirstUseEver);
    LinearUI::SetNextWindowSize(LinearVec2(550, 680), LinearUICond_FirstUseEver);

    // Main body of the Demo window starts here.
    if (!LinearUI::Begin("Dear LinearUI Demo", p_open, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        LinearUI::End();
        return;
    }

    // Most framed widgets share a common width settings. Remaining width is used for the label.
    // The width of the frame may be changed with PushItemWidth() or SetNextItemWidth().
    // - Positive value for absolute size, negative value for right-alignment.
    // - The default value is about GetWindowWidth() * 0.65f.
    // - See 'Demo->Layout->Widgets Width' for details.
    // Here we change the frame width based on how much width we want to give to the label.
    const float label_width_base = LinearUI::GetFontSize() * 12;               // Some amount of width for label, based on font size.
    const float label_width_max = LinearUI::GetContentRegionAvail().x * 0.40f; // ...but always leave some room for framed widgets.
    const float label_width = LINEARUI_MIN(label_width_base, label_width_max);
    LinearUI::PushItemWidth(-label_width);                                     // Right-align: framed items will leave 'label_width' available for the label.
    //LinearUI::PushItemWidth(LinearUI::GetContentRegionAvail().x * 0.40f);       // e.g. Use 40% width for framed widgets, leaving 60% width for labels.
    //LinearUI::PushItemWidth(-LinearUI::GetContentRegionAvail().x * 0.40f);      // e.g. Use 40% width for labels, leaving 60% width for framed widgets.
    //LinearUI::PushItemWidth(LinearUI::GetFontSize() * -12);                     // e.g. Use XXX width for labels, leaving the rest for framed widgets.

    // Menu Bar
    DemoWindowMenuBar(&demo_data);

    LinearUI::Text("dear linearui says hello! (%s) (%d)", LINEARUI_VERSION, LINEARUI_VERSION_NUM);
    LinearUI::Spacing();

    if (LinearUI::CollapsingHeader("Help"))
    {
        LINEARUI_DEMO_MARKER("Help");
        LinearUI::SeparatorText("ABOUT THIS DEMO:");
        LinearUI::BulletText("Sections below are demonstrating many aspects of the library.");
        LinearUI::BulletText("The \"Examples\" menu above leads to more demo contents.");
        LinearUI::BulletText("The \"Tools\" menu above gives access to: About Box, Style Editor,\n"
                          "and Metrics/Debugger (general purpose Dear LinearUI debugging tool).");
        LinearUI::BulletText("Web demo (w/ source code browser): ");
        LinearUI::SameLine(0, 0);
        LinearUI::TextLinkOpenURL("https://pthom.github.io/linearui_explorer");

        LinearUI::SeparatorText("PROGRAMMER GUIDE:");
        LinearUI::BulletText("See the ShowDemoWindow() code in linearui_demo.cpp. <- you are here!");
        LinearUI::BulletText("See comments in linearui.cpp.");
        LinearUI::BulletText("See example applications in the examples/ folder.");
        LinearUI::BulletText("Read the FAQ at ");
        LinearUI::SameLine(0, 0);
        LinearUI::TextLinkOpenURL("https://www.dearlinearui.com/faq/");
        LinearUI::BulletText("Set 'io.ConfigFlags |= NavEnableKeyboard' for keyboard controls.");
        LinearUI::BulletText("Set 'io.ConfigFlags |= NavEnableGamepad' for gamepad controls.");

        LinearUI::SeparatorText("USER GUIDE:");
        LinearUI::ShowUserGuide();
    }

    if (LinearUI::CollapsingHeader("Configuration"))
    {
        LinearUIIO& io = LinearUI::GetIO();

        if (LinearUI::TreeNode("Configuration##2"))
        {
            LINEARUI_DEMO_MARKER("Configuration");
            LinearUI::SeparatorText("General");
            LinearUI::CheckboxFlags("io.ConfigFlags: NavEnableKeyboard",    &io.ConfigFlags, LinearUIConfigFlags_NavEnableKeyboard);
            LinearUI::SameLine(); HelpMarker("Enable keyboard controls.");
            LinearUI::CheckboxFlags("io.ConfigFlags: NavEnableGamepad",     &io.ConfigFlags, LinearUIConfigFlags_NavEnableGamepad);
            LinearUI::SameLine(); HelpMarker("Enable gamepad controls. Require backend to set io.BackendFlags |= LinearUIBackendFlags_HasGamepad.\n\nRead instructions in linearui.cpp for details.");
            LinearUI::CheckboxFlags("io.ConfigFlags: NoMouse",              &io.ConfigFlags, LinearUIConfigFlags_NoMouse);
            LinearUI::SameLine(); HelpMarker("Instruct dear linearui to disable mouse inputs and interactions.");

            // The "NoMouse" option can get us stuck with a disabled mouse! Let's provide an alternative way to fix it:
            if (io.ConfigFlags & LinearUIConfigFlags_NoMouse)
            {
                if (fmodf((float)LinearUI::GetTime(), 0.40f) < 0.20f)
                {
                    LinearUI::SameLine();
                    LinearUI::Text("<<PRESS SPACE TO DISABLE>>");
                }
                // Prevent both being checked
                if (LinearUI::IsKeyPressed(LinearUIKey_Space) || (io.ConfigFlags & LinearUIConfigFlags_NoKeyboard))
                    io.ConfigFlags &= ~LinearUIConfigFlags_NoMouse;
            }

            LinearUI::CheckboxFlags("io.ConfigFlags: NoMouseCursorChange",  &io.ConfigFlags, LinearUIConfigFlags_NoMouseCursorChange);
            LinearUI::SameLine(); HelpMarker("Instruct backend to not alter mouse cursor shape and visibility.");
            LinearUI::CheckboxFlags("io.ConfigFlags: NoKeyboard", &io.ConfigFlags, LinearUIConfigFlags_NoKeyboard);
            LinearUI::SameLine(); HelpMarker("Instruct dear linearui to disable keyboard inputs and interactions.");

            LinearUI::Checkbox("io.ConfigInputTrickleEventQueue", &io.ConfigInputTrickleEventQueue);
            LinearUI::SameLine(); HelpMarker("Enable input queue trickling: some types of events submitted during the same frame (e.g. button down + up) will be spread over multiple frames, improving interactions with low framerates.");
            LinearUI::Checkbox("io.MouseDrawCursor", &io.MouseDrawCursor);
            LinearUI::SameLine(); HelpMarker("Instruct Dear LinearUI to render a mouse cursor itself. Note that a mouse cursor rendered via your application GPU rendering path will feel more laggy than hardware cursor, but will be more in sync with your other visuals.\n\nSome desktop applications may use both kinds of cursors (e.g. enable software cursor only when resizing/dragging something).");

            LinearUI::SeparatorText("Keyboard/Gamepad Navigation");
            LinearUI::Checkbox("io.ConfigNavSwapGamepadButtons", &io.ConfigNavSwapGamepadButtons);
            LinearUI::Checkbox("io.ConfigNavMoveSetMousePos", &io.ConfigNavMoveSetMousePos);
            LinearUI::SameLine(); HelpMarker("Directional/tabbing navigation teleports the mouse cursor. May be useful on TV/console systems where moving a virtual mouse is difficult");
            LinearUI::Checkbox("io.ConfigNavCaptureKeyboard", &io.ConfigNavCaptureKeyboard);
            LinearUI::Checkbox("io.ConfigNavEscapeClearFocusItem", &io.ConfigNavEscapeClearFocusItem);
            LinearUI::SameLine(); HelpMarker("Pressing Escape clears focused item.");
            LinearUI::Checkbox("io.ConfigNavEscapeClearFocusWindow", &io.ConfigNavEscapeClearFocusWindow);
            LinearUI::SameLine(); HelpMarker("Pressing Escape clears focused window.");
            LinearUI::Checkbox("io.ConfigNavCursorVisibleAuto", &io.ConfigNavCursorVisibleAuto);
            LinearUI::SameLine(); HelpMarker("Using directional navigation key makes the cursor visible. Mouse click hides the cursor.");
            LinearUI::Checkbox("io.ConfigNavCursorVisibleAlways", &io.ConfigNavCursorVisibleAlways);
            LinearUI::SameLine(); HelpMarker("Navigation cursor is always visible.");

            LinearUI::SeparatorText("Docking");
            LinearUI::CheckboxFlags("io.ConfigFlags: DockingEnable", &io.ConfigFlags, LinearUIConfigFlags_DockingEnable);
            LinearUI::SameLine();
            if (io.ConfigDockingWithShift)
                HelpMarker("Drag from window title bar or their tab to dock/undock. Hold SHIFT to enable docking.\n\nDrag from window menu button (upper-left button) to undock an entire node (all windows).");
            else
                HelpMarker("Drag from window title bar or their tab to dock/undock. Hold SHIFT to disable docking.\n\nDrag from window menu button (upper-left button) to undock an entire node (all windows).");
            if (io.ConfigFlags & LinearUIConfigFlags_DockingEnable)
            {
                LinearUI::Indent();
                LinearUI::Checkbox("io.ConfigDockingNoSplit", &io.ConfigDockingNoSplit);
                LinearUI::SameLine(); HelpMarker("Simplified docking mode: disable window splitting, so docking is limited to merging multiple windows together into tab-bars.");
                LinearUI::Checkbox("io.ConfigDockingNoDockingOver", &io.ConfigDockingNoDockingOver);
                LinearUI::SameLine(); HelpMarker("Simplified docking mode: disable window merging into a same tab-bar, so docking is limited to splitting windows.");
                LinearUI::Checkbox("io.ConfigDockingWithShift", &io.ConfigDockingWithShift);
                LinearUI::SameLine(); HelpMarker("Enable docking when holding Shift only (allow to drop in wider space, reduce visual noise)");
                LinearUI::Checkbox("io.ConfigDockingAlwaysTabBar", &io.ConfigDockingAlwaysTabBar);
                LinearUI::SameLine(); HelpMarker("Create a docking node and tab-bar on single floating windows.");
                LinearUI::Checkbox("io.ConfigDockingTransparentPayload", &io.ConfigDockingTransparentPayload);
                LinearUI::SameLine(); HelpMarker("Make window or viewport transparent when docking and only display docking boxes on the target viewport. Useful if rendering of multiple viewport cannot be synced. Best used with ConfigViewportsNoAutoMerge.");
                LinearUI::Unindent();
            }

            LinearUI::SeparatorText("Multi-viewports");
            LinearUI::CheckboxFlags("io.ConfigFlags: ViewportsEnable", &io.ConfigFlags, LinearUIConfigFlags_ViewportsEnable);
            LinearUI::SameLine(); HelpMarker("[beta] Enable beta multi-viewports support. See LinearUIPlatformIO for details.");
            if (io.ConfigFlags & LinearUIConfigFlags_ViewportsEnable)
            {
                LinearUI::Indent();
                LinearUI::Checkbox("io.ConfigViewportsNoAutoMerge", &io.ConfigViewportsNoAutoMerge);
                LinearUI::SameLine(); HelpMarker("Set to make all floating linearui windows always create their own viewport. Otherwise, they are merged into the main host viewports when overlapping it.");
                LinearUI::Checkbox("io.ConfigViewportsNoTaskBarIcon", &io.ConfigViewportsNoTaskBarIcon);
                LinearUI::SameLine(); HelpMarker("(note: some platform backends may not reflect a change of this value for existing viewports, and may need the viewport to be recreated)");
                LinearUI::Checkbox("io.ConfigViewportsNoDecoration", &io.ConfigViewportsNoDecoration);
                LinearUI::SameLine(); HelpMarker("(note: some platform backends may not reflect a change of this value for existing viewports, and may need the viewport to be recreated)");
                LinearUI::Checkbox("io.ConfigViewportsNoDefaultParent", &io.ConfigViewportsNoDefaultParent);
                LinearUI::SameLine(); HelpMarker("(note: some platform backends may not reflect a change of this value for existing viewports, and may need the viewport to be recreated)");
                LinearUI::Checkbox("io.ConfigViewportsPlatformFocusSetsLinearUIFocus", &io.ConfigViewportsPlatformFocusSetsLinearUIFocus);
                LinearUI::SameLine(); HelpMarker("When a platform window is focused (e.g. using Alt+Tab, clicking Platform Title Bar), apply corresponding focus on linearui windows (may clear focus/active id from linearui windows location in other platform windows). In principle this is better enabled but we provide an opt-out, because some Linux window managers tend to eagerly focus windows (e.g. on mouse hover, or even a simple window pos/size change).");
                LinearUI::Unindent();
            }

            //LinearUI::SeparatorText("DPI/Scaling");
            //LinearUI::Checkbox("io.ConfigDpiScaleFonts", &io.ConfigDpiScaleFonts);
            //LinearUI::SameLine(); HelpMarker("Experimental: Automatically update style.FontScaleDpi when Monitor DPI changes. This will scale fonts but NOT style sizes/padding for now.");
            //LinearUI::Checkbox("io.ConfigDpiScaleViewports", &io.ConfigDpiScaleViewports);
            //LinearUI::SameLine(); HelpMarker("Experimental: Scale Dear LinearUI and Platform Windows when Monitor DPI changes.");

            LinearUI::SeparatorText("Windows");
            LinearUI::Checkbox("io.ConfigWindowsResizeFromEdges", &io.ConfigWindowsResizeFromEdges);
            LinearUI::SameLine(); HelpMarker("Enable resizing of windows from their edges and from the lower-left corner.\nThis requires LinearUIBackendFlags_HasMouseCursors for better mouse cursor feedback.");
            LinearUI::Checkbox("io.ConfigWindowsMoveFromTitleBarOnly", &io.ConfigWindowsMoveFromTitleBarOnly);
            LinearUI::Checkbox("io.ConfigWindowsCopyContentsWithCtrlC", &io.ConfigWindowsCopyContentsWithCtrlC); // [EXPERIMENTAL]
            LinearUI::SameLine(); HelpMarker("*EXPERIMENTAL* Ctrl+C copy the contents of focused window into the clipboard.\n\nExperimental because:\n- (1) has known issues with nested Begin/End pairs.\n- (2) text output quality varies.\n- (3) text output is in submission order rather than spatial order.");
            LinearUI::Checkbox("io.ConfigScrollbarScrollByPage", &io.ConfigScrollbarScrollByPage);
            LinearUI::SameLine(); HelpMarker("Enable scrolling page by page when clicking outside the scrollbar grab.\nWhen disabled, always scroll to clicked location.\nWhen enabled, Shift+Click scrolls to clicked location.");

            LinearUI::SeparatorText("Widgets");
            LinearUI::Checkbox("io.ConfigInputTextCursorBlink", &io.ConfigInputTextCursorBlink);
            LinearUI::SameLine(); HelpMarker("Enable blinking cursor (optional as some users consider it to be distracting).");
            LinearUI::Checkbox("io.ConfigInputTextEnterKeepActive", &io.ConfigInputTextEnterKeepActive);
            LinearUI::SameLine(); HelpMarker("Pressing Enter will reactivate item and select all text (single-line only).");
            LinearUI::Checkbox("io.ConfigDragClickToInputText", &io.ConfigDragClickToInputText);
            LinearUI::SameLine(); HelpMarker("Enable turning DragXXX widgets into text input with a simple mouse click-release (without moving).");
            LinearUI::Checkbox("io.ConfigMacOSXBehaviors", &io.ConfigMacOSXBehaviors);
            LinearUI::SameLine(); HelpMarker("Swap Cmd<>Ctrl keys, enable various MacOS style behaviors.");
            LinearUI::Text("Also see Style->Rendering for rendering options.");

            // Also read: https://github.com/ocornut/linearui/wiki/Error-Handling
            LinearUI::SeparatorText("Error Handling");

            LinearUI::Checkbox("io.ConfigErrorRecovery", &io.ConfigErrorRecovery);
            LinearUI::SameLine(); HelpMarker(
                "Options to configure how we handle recoverable errors.\n"
                "- Error recovery is not perfect nor guaranteed! It is a feature to ease development.\n"
                "- You not are not supposed to rely on it in the course of a normal application run.\n"
                "- Possible usage: facilitate recovery from errors triggered from a scripting language or after specific exceptions handlers.\n"
                "- Always ensure that on programmers seat you have at minimum Asserts or Tooltips enabled when making direct linearui API call! "
                "Otherwise it would severely hinder your ability to catch and correct mistakes!");
            LinearUI::Checkbox("io.ConfigErrorRecoveryEnableAssert", &io.ConfigErrorRecoveryEnableAssert);
            LinearUI::Checkbox("io.ConfigErrorRecoveryEnableDebugLog", &io.ConfigErrorRecoveryEnableDebugLog);
            LinearUI::Checkbox("io.ConfigErrorRecoveryEnableTooltip", &io.ConfigErrorRecoveryEnableTooltip);
            if (!io.ConfigErrorRecoveryEnableAssert && !io.ConfigErrorRecoveryEnableDebugLog && !io.ConfigErrorRecoveryEnableTooltip)
                io.ConfigErrorRecoveryEnableAssert = io.ConfigErrorRecoveryEnableDebugLog = io.ConfigErrorRecoveryEnableTooltip = true;

            // Also read: https://github.com/ocornut/linearui/wiki/Debug-Tools
            LinearUI::SeparatorText("Debug");
            LinearUI::Checkbox("io.ConfigDebugIsDebuggerPresent", &io.ConfigDebugIsDebuggerPresent);
            LinearUI::SameLine(); HelpMarker("Enable various tools calling LINEARUI_DEBUG_BREAK().\n\nRequires a debugger being attached, otherwise LINEARUI_DEBUG_BREAK() options will appear to crash your application.");
            LinearUI::Checkbox("io.ConfigDebugHighlightIdConflicts", &io.ConfigDebugHighlightIdConflicts);
            LinearUI::SameLine(); HelpMarker("Highlight and show an error message when multiple items have conflicting identifiers.");
            LinearUI::BeginDisabled();
            LinearUI::Checkbox("io.ConfigDebugBeginReturnValueOnce", &io.ConfigDebugBeginReturnValueOnce);
            LinearUI::EndDisabled();
            LinearUI::SameLine(); HelpMarker("First calls to Begin()/BeginChild() will return false.\n\nTHIS OPTION IS DISABLED because it needs to be set at application boot-time to make sense. Showing the disabled option is a way to make this feature easier to discover.");
            LinearUI::Checkbox("io.ConfigDebugBeginReturnValueLoop", &io.ConfigDebugBeginReturnValueLoop);
            LinearUI::SameLine(); HelpMarker("Some calls to Begin()/BeginChild() will return false.\n\nWill cycle through window depths then repeat. Windows should be flickering while running.");
            LinearUI::Checkbox("io.ConfigDebugIgnoreFocusLoss", &io.ConfigDebugIgnoreFocusLoss);
            LinearUI::SameLine(); HelpMarker("Option to deactivate io.AddFocusEvent(false) handling. May facilitate interactions with a debugger when focus loss leads to clearing inputs data.");
            LinearUI::Checkbox("io.ConfigDebugIniSettings", &io.ConfigDebugIniSettings);
            LinearUI::SameLine(); HelpMarker("Option to save .ini data with extra comments (particularly helpful for Docking, but makes saving slower).");

            LinearUI::TreePop();
            LinearUI::Spacing();
        }

        if (LinearUI::TreeNode("Backend Flags"))
        {
            LINEARUI_DEMO_MARKER("Configuration/Backend Flags");
            HelpMarker(
                "Those flags are set by the backends (linearui_impl_xxx files) to specify their capabilities.\n"
                "Here we expose them as read-only fields to avoid breaking interactions with your backend.");

            // Make a local copy to avoid modifying actual backend flags.
            // FIXME: Maybe we need a BeginReadonly() equivalent to keep label bright?
            LinearUI::BeginDisabled();
            LinearUI::CheckboxFlags("io.BackendFlags: HasGamepad",             &io.BackendFlags, LinearUIBackendFlags_HasGamepad);
            LinearUI::CheckboxFlags("io.BackendFlags: HasMouseCursors",        &io.BackendFlags, LinearUIBackendFlags_HasMouseCursors);
            LinearUI::CheckboxFlags("io.BackendFlags: HasSetMousePos",         &io.BackendFlags, LinearUIBackendFlags_HasSetMousePos);
            LinearUI::CheckboxFlags("io.BackendFlags: PlatformHasViewports",   &io.BackendFlags, LinearUIBackendFlags_PlatformHasViewports);
            LinearUI::CheckboxFlags("io.BackendFlags: HasMouseHoveredViewport",&io.BackendFlags, LinearUIBackendFlags_HasMouseHoveredViewport);
            LinearUI::CheckboxFlags("io.BackendFlags: HasParentViewport",      &io.BackendFlags, LinearUIBackendFlags_HasParentViewport);
            LinearUI::CheckboxFlags("io.BackendFlags: RendererHasVtxOffset",   &io.BackendFlags, LinearUIBackendFlags_RendererHasVtxOffset);
            LinearUI::CheckboxFlags("io.BackendFlags: RendererHasTextures",    &io.BackendFlags, LinearUIBackendFlags_RendererHasTextures);
            LinearUI::CheckboxFlags("io.BackendFlags: RendererHasViewports",   &io.BackendFlags, LinearUIBackendFlags_RendererHasViewports);
            LinearUI::EndDisabled();

            LinearUI::TreePop();
            LinearUI::Spacing();
        }

        if (LinearUI::TreeNode("Style, Fonts"))
        {
            LINEARUI_DEMO_MARKER("Configuration/Style, Fonts");
            LinearUI::Checkbox("Style Editor", &demo_data.ShowStyleEditor);
            LinearUI::SameLine();
            HelpMarker("The same contents can be accessed in 'Tools->Style Editor' or by calling the ShowStyleEditor() function.");
            LinearUI::TreePop();
            LinearUI::Spacing();
        }

        if (LinearUI::TreeNode("Capture/Logging"))
        {
            LINEARUI_DEMO_MARKER("Configuration/Capture, Logging");
            HelpMarker(
                "The logging API redirects all text output so you can easily capture the content of "
                "a window or a block. Tree nodes can be automatically expanded.\n"
                "Try opening any of the contents below in this window and then click one of the \"Log To\" button.");
            LinearUI::LogButtons();

            HelpMarker("You can also call LinearUI::LogText() to output directly to the log without a visual output.");
            if (LinearUI::Button("Copy \"Hello, world!\" to clipboard"))
            {
                LinearUI::LogToClipboard();
                LinearUI::LogText("Hello, world!");
                LinearUI::LogFinish();
            }
            LinearUI::TreePop();
        }
    }

    if (LinearUI::CollapsingHeader("Window options"))
    {
        LINEARUI_DEMO_MARKER("Window options");
        if (LinearUI::BeginTable("split", 3))
        {
            LinearUI::TableNextColumn(); LinearUI::Checkbox("No titlebar", &no_titlebar);
            LinearUI::TableNextColumn(); LinearUI::Checkbox("No scrollbar", &no_scrollbar);
            LinearUI::TableNextColumn(); LinearUI::Checkbox("No menu", &no_menu);
            LinearUI::TableNextColumn(); LinearUI::Checkbox("No move", &no_move);
            LinearUI::TableNextColumn(); LinearUI::Checkbox("No resize", &no_resize);
            LinearUI::TableNextColumn(); LinearUI::Checkbox("No collapse", &no_collapse);
            LinearUI::TableNextColumn(); LinearUI::Checkbox("No close", &no_close);
            LinearUI::TableNextColumn(); LinearUI::Checkbox("No nav", &no_nav);
            LinearUI::TableNextColumn(); LinearUI::Checkbox("No background", &no_background);
            LinearUI::TableNextColumn(); LinearUI::Checkbox("No bring to front", &no_bring_to_front);
            LinearUI::TableNextColumn(); LinearUI::Checkbox("No docking", &no_docking);
            LinearUI::TableNextColumn(); LinearUI::Checkbox("Unsaved document", &unsaved_document);
            LinearUI::EndTable();
        }
    }

    // All demo contents
    DemoWindowWidgets(&demo_data);
    DemoWindowLayout();
    DemoWindowPopups();
    DemoWindowTables();
    DemoWindowInputs();

    // End of ShowDemoWindow()
    LinearUI::PopItemWidth();
    LinearUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowMenuBar()
//-----------------------------------------------------------------------------

static void DemoWindowMenuBar(LinearUIDemoWindowData* demo_data)
{
    if (LinearUI::BeginMenuBar())
    {
        if (LinearUI::BeginMenu("Menu"))
        {
            LINEARUI_DEMO_MARKER("Menu/File");
            ShowExampleMenuFile();
            LinearUI::EndMenu();
        }
        if (LinearUI::BeginMenu("Examples"))
        {
            LINEARUI_DEMO_MARKER("Menu/Examples");
            LinearUI::MenuItem("Main menu bar", NULL, &demo_data->ShowMainMenuBar);

            LinearUI::SeparatorText("Mini apps");
            LinearUI::MenuItem("Assets Browser", NULL, &demo_data->ShowAppAssetsBrowser);
            LinearUI::MenuItem("Console", NULL, &demo_data->ShowAppConsole);
            LinearUI::MenuItem("Custom rendering", NULL, &demo_data->ShowAppCustomRendering);
            LinearUI::MenuItem("Documents", NULL, &demo_data->ShowAppDocuments);
            LinearUI::MenuItem("Dockspace", NULL, &demo_data->ShowAppDockSpace);
            LinearUI::MenuItem("Image Viewer", NULL, &demo_data->ShowAppImageViewer);
            LinearUI::MenuItem("Log", NULL, &demo_data->ShowAppLog);
            LinearUI::MenuItem("Property editor", NULL, &demo_data->ShowAppPropertyEditor);
            LinearUI::MenuItem("Simple layout", NULL, &demo_data->ShowAppLayout);
            LinearUI::MenuItem("Simple overlay", NULL, &demo_data->ShowAppSimpleOverlay);

            LinearUI::SeparatorText("Concepts");
            LinearUI::MenuItem("Auto-resizing window", NULL, &demo_data->ShowAppAutoResize);
            LinearUI::MenuItem("Constrained-resizing window", NULL, &demo_data->ShowAppConstrainedResize);
            LinearUI::MenuItem("Fullscreen window", NULL, &demo_data->ShowAppFullscreen);
            LinearUI::MenuItem("Long text display", NULL, &demo_data->ShowAppLongText);
            LinearUI::MenuItem("Manipulating window titles", NULL, &demo_data->ShowAppWindowTitles);

            LinearUI::EndMenu();
        }
        //if (LinearUI::MenuItem("MenuItem")) {} // You can also use MenuItem() inside a menu bar!
        if (LinearUI::BeginMenu("Tools"))
        {
            LINEARUI_DEMO_MARKER("Menu/Tools");
            LinearUIIO& io = LinearUI::GetIO();
#ifndef LINEARUI_DISABLE_DEBUG_TOOLS
            const bool has_debug_tools = true;
#else
            const bool has_debug_tools = false;
#endif
            LinearUI::MenuItem("Metrics/Debugger", NULL, &demo_data->ShowMetrics, has_debug_tools);
            if (LinearUI::BeginMenu("Debug Options"))
            {
                LinearUI::BeginDisabled(!has_debug_tools);
                LinearUI::Checkbox("Highlight ID Conflicts", &io.ConfigDebugHighlightIdConflicts);
                LinearUI::EndDisabled();
                LinearUI::Checkbox("Assert on error recovery", &io.ConfigErrorRecoveryEnableAssert);
                LinearUI::TextDisabled("(see Demo->Configuration for more)");
                LinearUI::EndMenu();
            }
            LinearUI::MenuItem("Debug Log", NULL, &demo_data->ShowDebugLog, has_debug_tools);
            LinearUI::MenuItem("ID Stack Tool", NULL, &demo_data->ShowIDStackTool, has_debug_tools);
            bool is_debugger_present = io.ConfigDebugIsDebuggerPresent;
            if (LinearUI::MenuItem("Item Picker", NULL, false, has_debug_tools))// && is_debugger_present))
                LinearUI::DebugStartItemPicker();
            if (!is_debugger_present)
                LinearUI::SetItemTooltip("Requires io.ConfigDebugIsDebuggerPresent=true to be set.\n\nWe otherwise disable some extra features to avoid casual users crashing the application.");
            LinearUI::MenuItem("Style Editor", NULL, &demo_data->ShowStyleEditor);
            LinearUI::MenuItem("About Dear LinearUI", NULL, &demo_data->ShowAbout);

            LinearUI::EndMenu();
        }
        LinearUI::EndMenuBar();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Helpers: ExampleTreeNode, ExampleMemberInfo (for use by Property Editor & Multi-Select demos)
//-----------------------------------------------------------------------------

// Simple representation for a tree
// (this is designed to be simple to understand for our demos, not to be fancy or efficient etc.)
struct ExampleTreeNode
{
    // Tree structure
    char                        Name[28] = "";
    int                         UID = 0;
    ExampleTreeNode*            Parent = NULL;
    LinearVector<ExampleTreeNode*>  Childs;
    int                         IndexInParent = 0;  // Maintaining this allows us to implement linear traversal more easily

    // Leaf Data
    bool                        HasData = false;    // All leaves have data
    bool                        DataMyBool = true;
    int                         DataMyInt = 128;
    LinearVec2                      DataMyVec2 = LinearVec2(0.0f, 3.141592f);
};

// Simple representation of struct metadata/serialization data.
// (this is a minimal version of what a typical advanced application may provide)
struct ExampleMemberInfo
{
    const char*     Name;       // Member name
    LinearUIDataType   DataType;   // Member type
    int             DataCount;  // Member count (1 when scalar)
    int             Offset;     // Offset inside parent structure
};

// Metadata description of ExampleTreeNode struct.
static const ExampleMemberInfo ExampleTreeNodeMemberInfos[]
{
    { "MyName",     LinearUIDataType_String,  1, offsetof(ExampleTreeNode, Name) },
    { "MyBool",     LinearUIDataType_Bool,    1, offsetof(ExampleTreeNode, DataMyBool) },
    { "MyInt",      LinearUIDataType_S32,     1, offsetof(ExampleTreeNode, DataMyInt) },
    { "MyVec2",     LinearUIDataType_Float,   2, offsetof(ExampleTreeNode, DataMyVec2) },
};

static ExampleTreeNode* ExampleTree_CreateNode(const char* name, int uid, ExampleTreeNode* parent)
{
    ExampleTreeNode* node = LINEARUI_NEW(ExampleTreeNode);
    snprintf(node->Name, LINEARUI_COUNTOF(node->Name), "%s", name);
    node->UID = uid;
    node->Parent = parent;
    node->IndexInParent = parent ? parent->Childs.Size : 0;
    if (parent)
        parent->Childs.push_back(node);
    return node;
}

static void ExampleTree_DestroyNode(ExampleTreeNode* node)
{
    for (ExampleTreeNode* child_node : node->Childs)
        ExampleTree_DestroyNode(child_node);
    LINEARUI_DELETE(node);
}

// Create example tree data
// (warning: this can allocates MANY MANY more times than other code in all of Dear LinearUI + demo combined)
// (a real application managing one million nodes would likely store its tree data differently)
static ExampleTreeNode* ExampleTree_CreateDemoTree()
{
    //     20 root nodes ->    211 total nodes,   ~261 allocs.
    //   1000 root nodes ->   ~11K total nodes,   ~14K allocs.
    //  10000 root nodes ->  ~123K total nodes,  ~154K allocs.
    // 100000 root nodes -> ~1338K total nodes, ~1666K allocs.
    const int ROOT_ITEMS_COUNT = 20;

    static const char* category_names[] = { "Apple", "Banana", "Cherry", "Kiwi", "Mango", "Orange", "Pear", "Pineapple", "Strawberry", "Watermelon" };
    const int category_count = LINEARUI_COUNTOF(category_names);
    const size_t NAME_MAX_LEN = sizeof(ExampleTreeNode::Name);
    char name_buf[NAME_MAX_LEN];
    int uid = 0;
    ExampleTreeNode* node_L0 = ExampleTree_CreateNode("<ROOT>", ++uid, NULL);
    for (int idx_L0 = 0; idx_L0 < ROOT_ITEMS_COUNT; idx_L0++)
    {
        snprintf(name_buf, LINEARUI_COUNTOF(name_buf), "%s %d", category_names[idx_L0 / (ROOT_ITEMS_COUNT / category_count)], idx_L0 % (ROOT_ITEMS_COUNT / category_count));
        ExampleTreeNode* node_L1 = ExampleTree_CreateNode(name_buf, ++uid, node_L0);
        const int number_of_childs = (int)strlen(node_L1->Name);
        for (int idx_L1 = 0; idx_L1 < number_of_childs; idx_L1++)
        {
            snprintf(name_buf, LINEARUI_COUNTOF(name_buf), "Child %d", idx_L1);
            ExampleTreeNode* node_L2 = ExampleTree_CreateNode(name_buf, ++uid, node_L1);
            node_L2->HasData = true;
            if (idx_L1 == 0)
            {
                snprintf(name_buf, LINEARUI_COUNTOF(name_buf), "Sub-child %d", 0);
                ExampleTreeNode* node_L3 = ExampleTree_CreateNode(name_buf, ++uid, node_L2);
                node_L3->HasData = true;
            }
        }
    }
    return node_L0;
}

//-----------------------------------------------------------------------------
// [SECTION] Helpers: ExampleImageViewer
//-----------------------------------------------------------------------------

struct ExampleImageViewerData
{
    LinearU32   ImageBgColor = LINEARUI_COL32(100, 100, 100, 255);
    LinearU32   GridColor = LINEARUI_COL32(255, 255, 255, 100);
    bool    GridEnabled = true;
    bool    ViewReset = true;
    LinearVec2  ViewOffset; // in image space
    float   Zoom = 10.0f;
    float   ZoomMin = 1.0f;
    float   ZoomMax = 10000.0f;
};

static void ExampleImageViewer_DrawOptions(ExampleImageViewerData* data)
{
    LinearUI::SetNextItemShortcut(LinearUIKey_G, LinearUIInputFlags_Tooltip); // | LinearUIInputFlags_RouteGlobal
    LinearUI::Checkbox("Grid", &data->GridEnabled);
    LinearUI::SameLine();
    LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 10.0f);
    float zoom_100 = data->Zoom * 100.0f;
    if (LinearUI::DragFloat("##Zoom", &zoom_100, 5.0f, data->ZoomMin * 100.0f, data->ZoomMax * 100.0f, "%.0f%%", LinearUISliderFlags_AlwaysClamp))
        data->Zoom = zoom_100 / 100.0f;
}

static void ExampleImageViewer_DrawCanvas(ExampleImageViewerData* data, LinearVec2 canvas_size, LinearTextureRef image_tex_ref, int image_w, int image_h)
{
    LinearUIIO& io = LinearUI::GetIO();
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
    LinearDrawList* draw_list = LinearUI::GetWindowDrawList();
    LINEARUI_ASSERT(canvas_size.x >= 0.0f && canvas_size.y >= 0.0f);

    // Layout canvas
    LinearUI::InvisibleButton("##Canvas", canvas_size);
    LinearVec2 canvas_min = LinearUI::GetItemRectMin();
    LinearVec2 canvas_max = LinearUI::GetItemRectMax();

    if (data->ViewReset)
        data->ViewOffset = LinearVec2((canvas_size.x * 0.5f / data->Zoom) - 0.5f, (canvas_size.y * 0.5f / data->Zoom) - 0.5f); // Add half a pixel padding
    data->ViewReset = false;

    // Handle inputs
    if (LinearUI::SetItemKeyOwner(LinearUIKey_MouseWheelY))
        if (io.MouseWheel != 0.0f)
            data->Zoom = LINEARUI_CLAMP(data->Zoom * (1.0f + io.MouseWheel * 0.10f), data->ZoomMin, data->ZoomMax);
    float zoom = data->Zoom; // (float)(int)ViewZoom;
    if (LinearUI::IsItemActive() && LinearUI::IsMouseDragging(0))
    {
        data->ViewOffset.x -= io.MouseDelta.x / zoom;
        data->ViewOffset.y -= io.MouseDelta.y / zoom;
    }

    // Display image
    LinearVec2 image_min, image_max;
    image_min.x = (float)(int)((canvas_min.x - (data->ViewOffset.x * zoom)) + (canvas_size.x * 0.5f));
    image_min.y = (float)(int)((canvas_min.y - (data->ViewOffset.y * zoom)) + (canvas_size.y * 0.5f));
    image_max.x = (float)(int)(image_min.x + image_w * zoom);
    image_max.y = (float)(int)(image_min.y + image_h * zoom);
    draw_list->AddRect(LinearVec2(canvas_min.x - 1.0f, canvas_min.y - 1.0f), LinearVec2(canvas_max.x + 1.0f, canvas_max.y + 1.0f), LINEARUI_COL32(255, 255, 255, 255));
    draw_list->PushClipRect(canvas_min, canvas_max, true);
    draw_list->AddRectFilled(image_min, image_max, data->ImageBgColor);
    if (platform_io.DrawCallback_SetSamplerNearest != NULL)
        draw_list->AddCallback(platform_io.DrawCallback_SetSamplerNearest);
    draw_list->AddImage(image_tex_ref, image_min, image_max);
    if (platform_io.DrawCallback_SetSamplerLinear != NULL)
        draw_list->AddCallback(LinearUI::GetPlatformIO().DrawCallback_SetSamplerLinear);

    // Display grid lines for visible pixels
    if (data->GridEnabled && zoom > 6.0f)
    {
        const float step = (float)zoom;
        for (int px = (int)((canvas_min.x - image_min.x) / step); px <= (int)((canvas_max.x - image_min.x) / step); px++)
            draw_list->AddLineV(image_min.x + px * step, canvas_min.y, canvas_max.y, data->GridColor, 1.0f);
        for (int py = (int)((canvas_min.y - image_min.y) / step); py <= (int)((canvas_max.y - image_min.y) / step); py++)
            draw_list->AddLineH(canvas_min.x, canvas_max.x, image_min.y + py * step, data->GridColor, 1.0f);
    }
    draw_list->PopClipRect();
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsBasic()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsBasic()
{
    if (LinearUI::TreeNode("Basic"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Basic");
        LinearUI::SeparatorText("General");

        LINEARUI_DEMO_MARKER("Widgets/Basic/Button");
        static int clicked = 0;
        if (LinearUI::Button("Button"))
            clicked++;
        if (clicked & 1)
        {
            LinearUI::SameLine();
            LinearUI::Text("Thanks for clicking me!");
        }

        LINEARUI_DEMO_MARKER("Widgets/Basic/Checkbox");
        static bool check = true;
        LinearUI::Checkbox("checkbox", &check);

        LINEARUI_DEMO_MARKER("Widgets/Basic/RadioButton");
        static int e = 0;
        LinearUI::RadioButton("radio a", &e, 0); LinearUI::SameLine();
        LinearUI::RadioButton("radio b", &e, 1); LinearUI::SameLine();
        LinearUI::RadioButton("radio c", &e, 2);

        LinearUI::AlignTextToFramePadding();
        LinearUI::TextLinkOpenURL("Hyperlink", "https://github.com/ocornut/linearui/wiki/Error-Handling");

        // Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and changing style.
        LINEARUI_DEMO_MARKER("Widgets/Basic/Buttons (Colored)");
        for (int i = 0; i < 7; i++)
        {
            if (i > 0)
                LinearUI::SameLine();
            LinearUI::PushID(i);
            LinearUI::PushStyleColor(LinearUICol_Button, (LinearVec4)LinearColor::HSV(i / 7.0f, 0.6f, 0.6f));
            LinearUI::PushStyleColor(LinearUICol_ButtonHovered, (LinearVec4)LinearColor::HSV(i / 7.0f, 0.7f, 0.7f));
            LinearUI::PushStyleColor(LinearUICol_ButtonActive, (LinearVec4)LinearColor::HSV(i / 7.0f, 0.8f, 0.8f));
            LinearUI::Button("Click");
            LinearUI::PopStyleColor(3);
            LinearUI::PopID();
        }

        // Use AlignTextToFramePadding() to align text baseline to the baseline of framed widgets elements
        // (otherwise a Text+SameLine+Button sequence will have the text a little too high by default!)
        // See 'Demo->Layout->Text Baseline Alignment' for details.
        LinearUI::AlignTextToFramePadding();
        LinearUI::Text("Hold to repeat:");
        LinearUI::SameLine();

        // Arrow buttons with Repeater
        LINEARUI_DEMO_MARKER("Widgets/Basic/Buttons (Repeating)");
        static int counter = 0;
        float spacing = LinearUI::GetStyle().ItemInnerSpacing.x;
        LinearUI::PushItemFlag(LinearUIItemFlags_ButtonRepeat, true);
        if (LinearUI::ArrowButton("##left", LinearUIDir_Left)) { counter--; }
        LinearUI::SameLine(0.0f, spacing);
        if (LinearUI::ArrowButton("##right", LinearUIDir_Right)) { counter++; }
        LinearUI::PopItemFlag();
        LinearUI::SameLine();
        LinearUI::Text("%d", counter);

        LinearUI::Button("Tooltip");
        LinearUI::SetItemTooltip("I am a tooltip");

        LinearUI::LabelText("label", "Value");

        LinearUI::SeparatorText("Inputs");

        {
            // If you want to use InputText() with std::string or any custom dynamic string type:
            // - For std::string: use the wrapper in misc/cpp/linearui_stdlib.h/.cpp
            // - Otherwise, see the 'Dear LinearUI Demo->Widgets->Text Input->Resize Callback' for using LinearUIInputTextFlags_CallbackResize.
            LINEARUI_DEMO_MARKER("Widgets/Basic/InputText");
            static char str0[128] = "Hello, world!";
            LinearUI::InputText("input text", str0, LINEARUI_COUNTOF(str0));
            LinearUI::SameLine(); HelpMarker(
                "USER:\n"
                "Hold Shift or use mouse to select text.\n"
                "Ctrl+Left/Right to word jump.\n"
                "Ctrl+A or Double-Click to select all.\n"
                "Ctrl+X,Ctrl+C,Ctrl+V for clipboard.\n"
                "Ctrl+Z to undo, Ctrl+Y/Ctrl+Shift+Z to redo.\n"
                "Escape to revert.\n\n"
                "PROGRAMMER:\n"
                "You can use the LinearUIInputTextFlags_CallbackResize facility if you need to wire InputText() "
                "to a dynamic string type. See misc/cpp/linearui_stdlib.h for an example (this is not demonstrated "
                "in linearui_demo.cpp).");

            static char str1[128] = "";
            LinearUI::InputTextWithHint("input text (w/ hint)", "enter text here", str1, LINEARUI_COUNTOF(str1));

            LINEARUI_DEMO_MARKER("Widgets/Basic/InputInt, InputFloat");
            static int i0 = 123;
            LinearUI::InputInt("input int", &i0);

            static float f0 = 0.001f;
            LinearUI::InputFloat("input float", &f0, 0.01f, 1.0f, "%.3f");

            static double d0 = 999999.00000001;
            LinearUI::InputDouble("input double", &d0, 0.01f, 1.0f, "%.8f");

            static float f1 = 1.e10f;
            LinearUI::InputFloat("input scientific", &f1, 0.0f, 0.0f, "%e");
            LinearUI::SameLine(); HelpMarker(
                "You can input value using the scientific notation,\n"
                "  e.g. \"1e+8\" becomes \"100000000\".");

            static float vec4a[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
            LinearUI::InputFloat3("input float3", vec4a);
        }

        LinearUI::SeparatorText("Drags");

        {
            LINEARUI_DEMO_MARKER("Widgets/Basic/DragInt, DragFloat");
            static int i1 = 50, i2 = 42, i3 = 128;
            LinearUI::DragInt("drag int", &i1, 1);
            LinearUI::SameLine(); HelpMarker(
                "Click and drag to edit value.\n"
                "Hold Shift/Alt for faster/slower edit.\n"
                "Double-Click or Ctrl+Click to input value.");
            LinearUI::DragInt("drag int 0..100", &i2, 1, 0, 100, "%d%%", LinearUISliderFlags_AlwaysClamp);
            LinearUI::DragInt("drag int wrap 100..200", &i3, 1, 100, 200, "%d", LinearUISliderFlags_WrapAround);

            static float f1 = 1.00f, f2 = 0.0067f;
            LinearUI::DragFloat("drag float", &f1, 0.005f);
            LinearUI::DragFloat("drag small float", &f2, 0.0001f, 0.0f, 0.0f, "%.06f ns");
            //LinearUI::DragFloat("drag wrap -1..1", &f3, 0.005f, -1.0f, 1.0f, NULL, LinearUISliderFlags_WrapAround);
        }

        LinearUI::SeparatorText("Sliders");

        {
            LINEARUI_DEMO_MARKER("Widgets/Basic/SliderInt, SliderFloat");
            static int i1 = 0;
            LinearUI::SliderInt("slider int", &i1, -1, 3);
            LinearUI::SameLine(); HelpMarker("Ctrl+Click to input value.");

            static float f1 = 0.123f, f2 = 0.0f;
            LinearUI::SliderFloat("slider float", &f1, 0.0f, 1.0f, "ratio = %.3f");
            LinearUI::SliderFloat("slider float (log)", &f2, -10.0f, 10.0f, "%.4f", LinearUISliderFlags_Logarithmic);

            LINEARUI_DEMO_MARKER("Widgets/Basic/SliderAngle");
            static float angle = 0.0f;
            LinearUI::SliderAngle("slider angle", &angle);

            // Using the format string to display a name instead of an integer.
            // Here we completely omit '%d' from the format string, so it'll only display a name.
            // This technique can also be used with DragInt().
            LINEARUI_DEMO_MARKER("Widgets/Basic/Slider (enum)");
            enum Element { Element_Fire, Element_Earth, Element_Air, Element_Water, Element_COUNT };
            static int elem = Element_Fire;
            const char* elems_names[Element_COUNT] = { "Fire", "Earth", "Air", "Water" };
            const char* elem_name = (elem >= 0 && elem < Element_COUNT) ? elems_names[elem] : "Unknown";
            LinearUI::SliderInt("slider enum", &elem, 0, Element_COUNT - 1, elem_name); // Use LinearUISliderFlags_NoInput flag to disable Ctrl+Click here.
            LinearUI::SameLine(); HelpMarker("Using the format string parameter to display a name instead of the underlying integer.");
        }

        LinearUI::SeparatorText("Selectors/Pickers");

        {
            LINEARUI_DEMO_MARKER("Widgets/Basic/ColorEdit3, ColorEdit4");
            static float col1[3] = { 1.0f, 0.0f, 0.2f };
            static float col2[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
            LinearUI::ColorEdit3("color 1", col1);
            LinearUI::SameLine(); HelpMarker(
                "Click on the color square to open a color picker.\n"
                "Click and hold to use drag and drop.\n"
                "Right-Click on the color square to show options.\n"
                "Ctrl+Click on individual component to input value.\n");

            LinearUI::ColorEdit4("color 2", col2);
        }

        {
            // Using the _simplified_ one-liner Combo() api here
            // See "Combo" section for examples of how to use the more flexible BeginCombo()/EndCombo() api.
            LINEARUI_DEMO_MARKER("Widgets/Basic/Combo");
            const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIIIIII", "JJJJ", "KKKKKKK" };
            static int item_current = 0;
            LinearUI::Combo("combo", &item_current, items, LINEARUI_COUNTOF(items));
            LinearUI::SameLine(); HelpMarker(
                "Using the simplified one-liner Combo API here.\n"
                "Refer to the \"Combo\" section below for an explanation of how to use the more flexible and general BeginCombo/EndCombo API.");
        }

        {
            // Using the _simplified_ one-liner ListBox() api here
            // See "List boxes" section for examples of how to use the more flexible BeginListBox()/EndListBox() api.
            LINEARUI_DEMO_MARKER("Widgets/Basic/ListBox");
            const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi", "Mango", "Orange", "Pineapple", "Strawberry", "Watermelon" };
            static int item_current = 1;
            LinearUI::ListBox("listbox", &item_current, items, LINEARUI_COUNTOF(items), 4);
            LinearUI::SameLine(); HelpMarker(
                "Using the simplified one-liner ListBox API here.\n"
                "Refer to the \"List boxes\" section below for an explanation of how to use the more flexible and general BeginListBox/EndListBox API.");
        }

        // Testing LinearUIOnceUponAFrame helper.
        //static LinearUIOnceUponAFrame once;
        //for (int i = 0; i < 5; i++)
        //    if (once)
        //        LinearUI::Text("This will be displayed only once.");

        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsBullets()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsBullets()
{
    if (LinearUI::TreeNode("Bullets"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Bullets");
        LinearUI::BulletText("Bullet point 1");
        LinearUI::BulletText("Bullet point 2\nOn multiple lines");
        if (LinearUI::TreeNode("Tree node"))
        {
            LinearUI::BulletText("Another bullet point");
            LinearUI::TreePop();
        }
        LinearUI::Bullet(); LinearUI::Text("Bullet point 3 (two calls)");
        LinearUI::Bullet(); LinearUI::SmallButton("Button");
        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsCollapsingHeaders()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsCollapsingHeaders()
{
    if (LinearUI::TreeNode("Collapsing Headers"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Collapsing Headers");
        static bool closable_group = true;
        LinearUI::Checkbox("Show 2nd header", &closable_group);
        if (LinearUI::CollapsingHeader("Header", LinearUITreeNodeFlags_None))
        {
            LinearUI::Text("IsItemHovered: %d", LinearUI::IsItemHovered());
            for (int i = 0; i < 5; i++)
                LinearUI::Text("Some content %d", i);
        }
        if (LinearUI::CollapsingHeader("Header with a close button", &closable_group))
        {
            LinearUI::Text("IsItemHovered: %d", LinearUI::IsItemHovered());
            for (int i = 0; i < 5; i++)
                LinearUI::Text("More content %d", i);
        }
        /*
        if (LinearUI::CollapsingHeader("Header with a bullet", LinearUITreeNodeFlags_Bullet))
            LinearUI::Text("IsItemHovered: %d", LinearUI::IsItemHovered());
        */
        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsColorAndPickers()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsColorAndPickers()
{
    if (LinearUI::TreeNode("Color/Picker Widgets"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Color");
        static LinearVec4 color = LinearVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
        static LinearUIColorEditFlags base_flags = LinearUIColorEditFlags_None;

        LinearUI::SeparatorText("Options");
        LinearUI::CheckboxFlags("LinearUIColorEditFlags_NoAlpha", &base_flags, LinearUIColorEditFlags_NoAlpha);
        LinearUI::CheckboxFlags("LinearUIColorEditFlags_AlphaOpaque", &base_flags, LinearUIColorEditFlags_AlphaOpaque);
        LinearUI::CheckboxFlags("LinearUIColorEditFlags_AlphaNoBg", &base_flags, LinearUIColorEditFlags_AlphaNoBg);
        LinearUI::CheckboxFlags("LinearUIColorEditFlags_AlphaPreviewHalf", &base_flags, LinearUIColorEditFlags_AlphaPreviewHalf);
        LinearUI::CheckboxFlags("LinearUIColorEditFlags_NoOptions", &base_flags, LinearUIColorEditFlags_NoOptions); LinearUI::SameLine(); HelpMarker("Right-click on the individual color widget to show options.");
        LinearUI::CheckboxFlags("LinearUIColorEditFlags_NoDragDrop", &base_flags, LinearUIColorEditFlags_NoDragDrop);
        LinearUI::CheckboxFlags("LinearUIColorEditFlags_NoColorMarkers", &base_flags, LinearUIColorEditFlags_NoColorMarkers);
        LinearUI::CheckboxFlags("LinearUIColorEditFlags_HDR", &base_flags, LinearUIColorEditFlags_HDR); LinearUI::SameLine(); HelpMarker("Currently all this does is to lift the 0..1 limits on dragging widgets.");

        LINEARUI_DEMO_MARKER("Widgets/Color/ColorEdit");
        LinearUI::SeparatorText("Inline color editor");
        LinearUI::Text("Color widget:");
        LinearUI::SameLine(); HelpMarker(
            "Click on the color square to open a color picker.\n"
            "Ctrl+Click on individual component to input value.\n");
        LinearUI::ColorEdit3("MyColor##1", (float*)&color, base_flags);

        LINEARUI_DEMO_MARKER("Widgets/Color/ColorEdit (HSV, with Alpha)");
        LinearUI::Text("Color widget HSV with Alpha:");
        LinearUI::ColorEdit4("MyColor##2", (float*)&color, LinearUIColorEditFlags_DisplayHSV | base_flags);

        LINEARUI_DEMO_MARKER("Widgets/Color/ColorEdit (float display)");
        LinearUI::Text("Color widget with Float Display:");
        LinearUI::ColorEdit4("MyColor##2f", (float*)&color, LinearUIColorEditFlags_Float | base_flags);

        LINEARUI_DEMO_MARKER("Widgets/Color/ColorButton (with Picker)");
        LinearUI::Text("Color button with Picker:");
        LinearUI::SameLine(); HelpMarker(
            "With the LinearUIColorEditFlags_NoInputs flag you can hide all the slider/text inputs.\n"
            "With the LinearUIColorEditFlags_NoLabel flag you can pass a non-empty label which will only "
            "be used for the tooltip and picker popup.");
        LinearUI::ColorEdit4("MyColor##3", (float*)&color, LinearUIColorEditFlags_NoInputs | LinearUIColorEditFlags_NoLabel | base_flags);

        LINEARUI_DEMO_MARKER("Widgets/Color/ColorButton (with custom Picker popup)");
        LinearUI::Text("Color button with Custom Picker Popup:");

        // Generate a default palette. The palette will persist and can be edited.
        static bool saved_palette_init = true;
        static LinearVec4 saved_palette[32] = {};
        if (saved_palette_init)
        {
            for (int n = 0; n < LINEARUI_COUNTOF(saved_palette); n++)
            {
                LinearUI::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f,
                    saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
                saved_palette[n].w = 1.0f; // Alpha
            }
            saved_palette_init = false;
        }

        static LinearVec4 backup_color;
        bool open_popup = LinearUI::ColorButton("MyColor##3b", color, base_flags);
        LinearUI::SameLine(0, LinearUI::GetStyle().ItemInnerSpacing.x);
        open_popup |= LinearUI::Button("Palette");
        if (open_popup)
        {
            LinearUI::OpenPopup("mypicker");
            backup_color = color;
        }
        if (LinearUI::BeginPopup("mypicker"))
        {
            LinearUI::Text("MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!");
            LinearUI::Separator();
            LinearUI::ColorPicker4("##picker", (float*)&color, base_flags | LinearUIColorEditFlags_NoSidePreview | LinearUIColorEditFlags_NoSmallPreview);
            LinearUI::SameLine();

            LinearUI::BeginGroup(); // Lock X position
            LinearUI::Text("Current");
            LinearUI::ColorButton("##current", color, LinearUIColorEditFlags_NoPicker | LinearUIColorEditFlags_AlphaPreviewHalf, LinearVec2(60, 40));
            LinearUI::Text("Previous");
            if (LinearUI::ColorButton("##previous", backup_color, LinearUIColorEditFlags_NoPicker | LinearUIColorEditFlags_AlphaPreviewHalf, LinearVec2(60, 40)))
                color = backup_color;
            LinearUI::Separator();
            LinearUI::Text("Palette");
            for (int n = 0; n < LINEARUI_COUNTOF(saved_palette); n++)
            {
                LinearUI::PushID(n);
                if ((n % 8) != 0)
                    LinearUI::SameLine(0.0f, LinearUI::GetStyle().ItemSpacing.y);

                LinearUIColorEditFlags palette_button_flags = LinearUIColorEditFlags_NoAlpha | LinearUIColorEditFlags_NoPicker | LinearUIColorEditFlags_NoTooltip;
                if (LinearUI::ColorButton("##palette", saved_palette[n], palette_button_flags, LinearVec2(20, 20)))
                    color = LinearVec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, color.w); // Preserve alpha!

                // Allow user to drop colors into each palette entry. Note that ColorButton() is already a
                // drag source by default, unless specifying the LinearUIColorEditFlags_NoDragDrop flag.
                if (LinearUI::BeginDragDropTarget())
                {
                    if (const LinearUIPayload* payload = LinearUI::AcceptDragDropPayload(LINEARUI_PAYLOAD_TYPE_COLOR_3F))
                        memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 3);
                    if (const LinearUIPayload* payload = LinearUI::AcceptDragDropPayload(LINEARUI_PAYLOAD_TYPE_COLOR_4F))
                        memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 4);
                    LinearUI::EndDragDropTarget();
                }

                LinearUI::PopID();
            }
            LinearUI::EndGroup();
            LinearUI::EndPopup();
        }

        LINEARUI_DEMO_MARKER("Widgets/Color/ColorButton (simple)");
        LinearUI::Text("Color button only:");
        static bool no_border = false;
        LinearUI::Checkbox("LinearUIColorEditFlags_NoBorder", &no_border);
        LinearUI::ColorButton("MyColor##3c", *(LinearVec4*)&color, base_flags | (no_border ? LinearUIColorEditFlags_NoBorder : 0), LinearVec2(80, 80));

        LINEARUI_DEMO_MARKER("Widgets/Color/ColorPicker");
        LinearUI::SeparatorText("Color picker");

        static bool ref_color = false;
        static LinearVec4 ref_color_v(1.0f, 0.0f, 1.0f, 0.5f);
        static int picker_mode = 0;
        static int display_mode = 0;
        static LinearUIColorEditFlags color_picker_flags = LinearUIColorEditFlags_AlphaBar;

        LinearUI::PushID("Color picker");
        LinearUI::CheckboxFlags("LinearUIColorEditFlags_NoAlpha", &color_picker_flags, LinearUIColorEditFlags_NoAlpha);
        LinearUI::CheckboxFlags("LinearUIColorEditFlags_AlphaBar", &color_picker_flags, LinearUIColorEditFlags_AlphaBar);
        LinearUI::CheckboxFlags("LinearUIColorEditFlags_NoSidePreview", &color_picker_flags, LinearUIColorEditFlags_NoSidePreview);
        if (color_picker_flags & LinearUIColorEditFlags_NoSidePreview)
        {
            LinearUI::SameLine();
            LinearUI::Checkbox("With Ref Color", &ref_color);
            if (ref_color)
            {
                LinearUI::SameLine();
                LinearUI::ColorEdit4("##RefColor", &ref_color_v.x, LinearUIColorEditFlags_NoInputs | base_flags);
            }
        }

        LinearUI::Combo("Picker Mode", &picker_mode, "Auto/Current\0LinearUIColorEditFlags_PickerHueBar\0LinearUIColorEditFlags_PickerHueWheel\0");
        LinearUI::SameLine(); HelpMarker("When not specified explicitly, user can right-click the picker to change mode.");

        LinearUI::Combo("Display Mode", &display_mode, "Auto/Current\0LinearUIColorEditFlags_NoInputs\0LinearUIColorEditFlags_DisplayRGB\0LinearUIColorEditFlags_DisplayHSV\0LinearUIColorEditFlags_DisplayHex\0");
        LinearUI::SameLine(); HelpMarker(
            "ColorEdit defaults to displaying RGB inputs if you don't specify a display mode, "
            "but the user can change it with a right-click on those inputs.\n\nColorPicker defaults to displaying RGB+HSV+Hex "
            "if you don't specify a display mode.\n\nYou can change the defaults using SetColorEditOptions().");

        LinearUIColorEditFlags flags = base_flags | color_picker_flags;
        if (picker_mode == 1)  flags |= LinearUIColorEditFlags_PickerHueBar;
        if (picker_mode == 2)  flags |= LinearUIColorEditFlags_PickerHueWheel;
        if (display_mode == 1) flags |= LinearUIColorEditFlags_NoInputs;       // Disable all RGB/HSV/Hex displays
        if (display_mode == 2) flags |= LinearUIColorEditFlags_DisplayRGB;     // Override display mode
        if (display_mode == 3) flags |= LinearUIColorEditFlags_DisplayHSV;
        if (display_mode == 4) flags |= LinearUIColorEditFlags_DisplayHex;
        LinearUI::ColorPicker4("MyColor##4", (float*)&color, flags, ref_color ? &ref_color_v.x : NULL);

        LinearUI::Text("Set defaults in code:");
        LinearUI::SameLine(); HelpMarker(
            "SetColorEditOptions() is designed to allow you to set boot-time default.\n"
            "We don't have Push/Pop functions because you can force options on a per-widget basis if needed, "
            "and the user can change non-forced ones with the options menu.\nWe don't have a getter to avoid "
            "encouraging you to persistently save values that aren't forward-compatible.");
        if (LinearUI::Button("Default: Uint8 + HSV + Hue Bar"))
            LinearUI::SetColorEditOptions(LinearUIColorEditFlags_Uint8 | LinearUIColorEditFlags_DisplayHSV | LinearUIColorEditFlags_PickerHueBar);
        if (LinearUI::Button("Default: Float + HDR + Hue Wheel"))
            LinearUI::SetColorEditOptions(LinearUIColorEditFlags_Float | LinearUIColorEditFlags_HDR | LinearUIColorEditFlags_PickerHueWheel);

        // Always display a small version of both types of pickers
        // (that's in order to make it more visible in the demo to people who are skimming quickly through it)
        LinearUI::Text("Both types:");
        float w = (LinearUI::GetContentRegionAvail().x - LinearUI::GetStyle().ItemSpacing.y) * 0.40f;
        LinearUI::SetNextItemWidth(w);
        LinearUI::ColorPicker3("##MyColor##5", (float*)&color, LinearUIColorEditFlags_PickerHueBar | LinearUIColorEditFlags_NoSidePreview | LinearUIColorEditFlags_NoInputs | LinearUIColorEditFlags_NoAlpha);
        LinearUI::SameLine();
        LinearUI::SetNextItemWidth(w);
        LinearUI::ColorPicker3("##MyColor##6", (float*)&color, LinearUIColorEditFlags_PickerHueWheel | LinearUIColorEditFlags_NoSidePreview | LinearUIColorEditFlags_NoInputs | LinearUIColorEditFlags_NoAlpha);
        LinearUI::PopID();

        // HSV encoded support (to avoid RGB<>HSV round trips and singularities when S==0 or V==0)
        static LinearVec4 color_hsv(0.23f, 1.0f, 1.0f, 1.0f); // Stored as HSV!
        LinearUI::Spacing();
        LinearUI::Text("HSV encoded colors");
        LinearUI::SameLine(); HelpMarker(
            "By default, colors are given to ColorEdit and ColorPicker in RGB, but LinearUIColorEditFlags_InputHSV "
            "allows you to store colors as HSV and pass them to ColorEdit and ColorPicker as HSV. This comes with the "
            "added benefit that you can manipulate hue values with the picker even when saturation or value are zero.");
        LinearUI::Text("Color widget with InputHSV:");
        LinearUI::ColorEdit4("HSV shown as RGB##1", (float*)&color_hsv, LinearUIColorEditFlags_DisplayRGB | LinearUIColorEditFlags_InputHSV | LinearUIColorEditFlags_Float);
        LinearUI::ColorEdit4("HSV shown as HSV##1", (float*)&color_hsv, LinearUIColorEditFlags_DisplayHSV | LinearUIColorEditFlags_InputHSV | LinearUIColorEditFlags_Float);
        LinearUI::DragFloat4("Raw HSV values", (float*)&color_hsv, 0.01f, 0.0f, 1.0f);

        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsComboBoxes()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsComboBoxes()
{
    if (LinearUI::TreeNode("Combo"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Combo");
        // Combo Boxes are also called "Dropdown" in other systems
        // Expose flags as checkbox for the demo
        static LinearUIComboFlags flags = 0;
        LinearUI::CheckboxFlags("LinearUIComboFlags_PopupAlignLeft", &flags, LinearUIComboFlags_PopupAlignLeft);
        LinearUI::SameLine(); HelpMarker("Only makes a difference if the popup is larger than the combo");
        if (LinearUI::CheckboxFlags("LinearUIComboFlags_NoArrowButton", &flags, LinearUIComboFlags_NoArrowButton))
            flags &= ~LinearUIComboFlags_NoPreview;     // Clear incompatible flags
        if (LinearUI::CheckboxFlags("LinearUIComboFlags_NoPreview", &flags, LinearUIComboFlags_NoPreview))
            flags &= ~(LinearUIComboFlags_NoArrowButton | LinearUIComboFlags_WidthFitPreview); // Clear incompatible flags
        if (LinearUI::CheckboxFlags("LinearUIComboFlags_WidthFitPreview", &flags, LinearUIComboFlags_WidthFitPreview))
            flags &= ~LinearUIComboFlags_NoPreview;

        // Override default popup height
        if (LinearUI::CheckboxFlags("LinearUIComboFlags_HeightSmall", &flags, LinearUIComboFlags_HeightSmall))
            flags &= ~(LinearUIComboFlags_HeightMask_ & ~LinearUIComboFlags_HeightSmall);
        if (LinearUI::CheckboxFlags("LinearUIComboFlags_HeightRegular", &flags, LinearUIComboFlags_HeightRegular))
            flags &= ~(LinearUIComboFlags_HeightMask_ & ~LinearUIComboFlags_HeightRegular);
        if (LinearUI::CheckboxFlags("LinearUIComboFlags_HeightLargest", &flags, LinearUIComboFlags_HeightLargest))
            flags &= ~(LinearUIComboFlags_HeightMask_ & ~LinearUIComboFlags_HeightLargest);

        // Using the generic BeginCombo() API, you have full control over how to display the combo contents.
        // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
        // stored in the object itself, etc.)
        const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
        static int item_selected_idx = 0; // Here we store our selection data as an index.

        // Pass in the preview value visible before opening the combo (it could technically be different contents or not pulled from items[])
        const char* combo_preview_value = items[item_selected_idx];
        if (LinearUI::BeginCombo("combo 1", combo_preview_value, flags))
        {
            for (int n = 0; n < LINEARUI_COUNTOF(items); n++)
            {
                const bool is_selected = (item_selected_idx == n);
                if (LinearUI::Selectable(items[n], is_selected))
                    item_selected_idx = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    LinearUI::SetItemDefaultFocus();
            }
            LinearUI::EndCombo();
        }

        // Show case embedding a filter using a simple trick: displaying the filter inside combo contents.
        // See https://github.com/ocornut/linearui/issues/718 for advanced/esoteric alternatives.
        if (LinearUI::BeginCombo("combo 2 (w/ filter)", combo_preview_value, flags))
        {
            static LinearUITextFilter filter;
            if (LinearUI::IsWindowAppearing())
            {
                LinearUI::SetKeyboardFocusHere();
                filter.Clear();
            }
            LinearUI::SetNextItemShortcut(LinearUIMod_Ctrl | LinearUIKey_F);
            filter.Draw("##Filter", -FLT_MIN);

            for (int n = 0; n < LINEARUI_COUNTOF(items); n++)
            {
                const bool is_selected = (item_selected_idx == n);
                if (filter.PassFilter(items[n]))
                    if (LinearUI::Selectable(items[n], is_selected))
                        item_selected_idx = n;
            }
            LinearUI::EndCombo();
        }

        LinearUI::Spacing();
        LinearUI::SeparatorText("One-liner variants");
        HelpMarker("The Combo() function is not greatly useful apart from cases were you want to embed all options in a single strings.\nFlags above don't apply to this section.");

        // Simplified one-liner Combo() API, using values packed in a single constant string
        // This is a convenience for when the selection set is small and known at compile-time.
        static int item_current_2 = 0;
        LinearUI::Combo("combo 3 (one-liner)", &item_current_2, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");

        // Simplified one-liner Combo() using an array of const char*
        // This is not very useful (may obsolete): prefer using BeginCombo()/EndCombo() for full control.
        static int item_current_3 = -1; // If the selection isn't within 0..count, Combo won't display a preview
        LinearUI::Combo("combo 4 (array)", &item_current_3, items, LINEARUI_COUNTOF(items));

        // Simplified one-liner Combo() using an accessor function
        static int item_current_4 = 0;
        LinearUI::Combo("combo 5 (function)", &item_current_4, [](void* data, int n) { return ((const char**)data)[n]; }, items, LINEARUI_COUNTOF(items));

        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsDataTypes()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsDataTypes()
{
    if (LinearUI::TreeNode("Data Types"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Data Types");
        // DragScalar/InputScalar/SliderScalar functions allow various data types
        // - signed/unsigned
        // - 8/16/32/64-bits
        // - integer/float/double
        // To avoid polluting the public API with all possible combinations, we use the LinearUIDataType enum
        // to pass the type, and passing all arguments by pointer.
        // This is the reason the test code below creates local variables to hold "zero" "one" etc. for each type.
        // In practice, if you frequently use a given type that is not covered by the normal API entry points,
        // you can wrap it yourself inside a 1 line function which can take typed argument as value instead of void*,
        // and then pass their address to the generic function. For example:
        //   bool MySliderU64(const char *label, u64* value, u64 min = 0, u64 max = 0, const char* format = "%lld")
        //   {
        //      return SliderScalar(label, LinearUIDataType_U64, value, &min, &max, format);
        //   }

        // Setup limits (as helper variables so we can take their address, as explained above)
        // Note: SliderScalar() functions have a maximum usable range of half the natural type maximum, hence the /2.
        #ifndef LLONG_MIN
        LinearS64 LLONG_MIN = -9223372036854775807LL - 1;
        LinearS64 LLONG_MAX = 9223372036854775807LL;
        LinearU64 ULLONG_MAX = (2ULL * 9223372036854775807LL + 1);
        #endif
        const char    s8_zero  = 0,   s8_one  = 1,   s8_fifty  = 50, s8_min  = -128,        s8_max = 127;
        const LinearU8    u8_zero  = 0,   u8_one  = 1,   u8_fifty  = 50, u8_min  = 0,           u8_max = 255;
        const short   s16_zero = 0,   s16_one = 1,   s16_fifty = 50, s16_min = -32768,      s16_max = 32767;
        const LinearU16   u16_zero = 0,   u16_one = 1,   u16_fifty = 50, u16_min = 0,           u16_max = 65535;
        const LinearS32   s32_zero = 0,   s32_one = 1,   s32_fifty = 50, s32_min = INT_MIN/2,   s32_max = INT_MAX/2,    s32_hi_a = INT_MAX/2 - 100,    s32_hi_b = INT_MAX/2;
        const LinearU32   u32_zero = 0,   u32_one = 1,   u32_fifty = 50, u32_min = 0,           u32_max = UINT_MAX/2,   u32_hi_a = UINT_MAX/2 - 100,   u32_hi_b = UINT_MAX/2;
        const LinearS64   s64_zero = 0,   s64_one = 1,   s64_fifty = 50, s64_min = LLONG_MIN/2, s64_max = LLONG_MAX/2,  s64_hi_a = LLONG_MAX/2 - 100,  s64_hi_b = LLONG_MAX/2;
        const LinearU64   u64_zero = 0,   u64_one = 1,   u64_fifty = 50, u64_min = 0,           u64_max = ULLONG_MAX/2, u64_hi_a = ULLONG_MAX/2 - 100, u64_hi_b = ULLONG_MAX/2;
        const float   f32_zero = 0.f, f32_one = 1.f, f32_lo_a = -10000000000.0f, f32_hi_a = +10000000000.0f;
        const double  f64_zero = 0.,  f64_one = 1.,  f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

        // State
        static char   s8_v  = 127;
        static LinearU8   u8_v  = 255;
        static short  s16_v = 32767;
        static LinearU16  u16_v = 65535;
        static LinearS32  s32_v = -1;
        static LinearU32  u32_v = (LinearU32)-1;
        static LinearS64  s64_v = -1;
        static LinearU64  u64_v = (LinearU64)-1;
        static float  f32_v = 0.123f;
        static double f64_v = 90000.01234567890123456789;

        const float drag_speed = 0.2f;
        static bool drag_clamp = false;
        LINEARUI_DEMO_MARKER("Widgets/Data Types/Drags");
        LinearUI::SeparatorText("Drags");
        LinearUI::Checkbox("Clamp integers to 0..50", &drag_clamp);
        LinearUI::SameLine(); HelpMarker(
            "As with every widget in dear linearui, we never modify values unless there is a user interaction.\n"
            "You can override the clamping limits by using Ctrl+Click to input a value.");
        LinearUI::DragScalar("drag s8",        LinearUIDataType_S8,     &s8_v,  drag_speed, drag_clamp ? &s8_zero  : NULL, drag_clamp ? &s8_fifty  : NULL);
        LinearUI::DragScalar("drag u8",        LinearUIDataType_U8,     &u8_v,  drag_speed, drag_clamp ? &u8_zero  : NULL, drag_clamp ? &u8_fifty  : NULL, "%u ms");
        LinearUI::DragScalar("drag s16",       LinearUIDataType_S16,    &s16_v, drag_speed, drag_clamp ? &s16_zero : NULL, drag_clamp ? &s16_fifty : NULL);
        LinearUI::DragScalar("drag u16",       LinearUIDataType_U16,    &u16_v, drag_speed, drag_clamp ? &u16_zero : NULL, drag_clamp ? &u16_fifty : NULL, "%u ms");
        LinearUI::DragScalar("drag s32",       LinearUIDataType_S32,    &s32_v, drag_speed, drag_clamp ? &s32_zero : NULL, drag_clamp ? &s32_fifty : NULL);
        LinearUI::DragScalar("drag s32 hex",   LinearUIDataType_S32,    &s32_v, drag_speed, drag_clamp ? &s32_zero : NULL, drag_clamp ? &s32_fifty : NULL, "0x%08X");
        LinearUI::DragScalar("drag u32",       LinearUIDataType_U32,    &u32_v, drag_speed, drag_clamp ? &u32_zero : NULL, drag_clamp ? &u32_fifty : NULL, "%u ms");
        LinearUI::DragScalar("drag s64",       LinearUIDataType_S64,    &s64_v, drag_speed, drag_clamp ? &s64_zero : NULL, drag_clamp ? &s64_fifty : NULL);
        LinearUI::DragScalar("drag u64",       LinearUIDataType_U64,    &u64_v, drag_speed, drag_clamp ? &u64_zero : NULL, drag_clamp ? &u64_fifty : NULL);
        LinearUI::DragScalar("drag float",     LinearUIDataType_Float,  &f32_v, 0.005f,  &f32_zero, &f32_one, "%f");
        LinearUI::DragScalar("drag float log", LinearUIDataType_Float,  &f32_v, 0.005f,  &f32_zero, &f32_one, "%f", LinearUISliderFlags_Logarithmic);
        LinearUI::DragScalar("drag double",    LinearUIDataType_Double, &f64_v, 0.0005f, &f64_zero, NULL,     "%.10f grams");
        LinearUI::DragScalar("drag double log",LinearUIDataType_Double, &f64_v, 0.0005f, &f64_zero, &f64_one, "0 < %.10f < 1", LinearUISliderFlags_Logarithmic);

        LINEARUI_DEMO_MARKER("Widgets/Data Types/Sliders");
        LinearUI::SeparatorText("Sliders");
        LinearUI::SliderScalar("slider s8 full",       LinearUIDataType_S8,     &s8_v,  &s8_min,   &s8_max,   "%d");
        LinearUI::SliderScalar("slider u8 full",       LinearUIDataType_U8,     &u8_v,  &u8_min,   &u8_max,   "%u");
        LinearUI::SliderScalar("slider s16 full",      LinearUIDataType_S16,    &s16_v, &s16_min,  &s16_max,  "%d");
        LinearUI::SliderScalar("slider u16 full",      LinearUIDataType_U16,    &u16_v, &u16_min,  &u16_max,  "%u");
        LinearUI::SliderScalar("slider s32 low",       LinearUIDataType_S32,    &s32_v, &s32_zero, &s32_fifty,"%d");
        LinearUI::SliderScalar("slider s32 high",      LinearUIDataType_S32,    &s32_v, &s32_hi_a, &s32_hi_b, "%d");
        LinearUI::SliderScalar("slider s32 full",      LinearUIDataType_S32,    &s32_v, &s32_min,  &s32_max,  "%d");
        LinearUI::SliderScalar("slider s32 hex",       LinearUIDataType_S32,    &s32_v, &s32_zero, &s32_fifty, "0x%04X");
        LinearUI::SliderScalar("slider u32 low",       LinearUIDataType_U32,    &u32_v, &u32_zero, &u32_fifty,"%u");
        LinearUI::SliderScalar("slider u32 high",      LinearUIDataType_U32,    &u32_v, &u32_hi_a, &u32_hi_b, "%u");
        LinearUI::SliderScalar("slider u32 full",      LinearUIDataType_U32,    &u32_v, &u32_min,  &u32_max,  "%u");
        LinearUI::SliderScalar("slider s64 low",       LinearUIDataType_S64,    &s64_v, &s64_zero, &s64_fifty,"%" PRId64);
        LinearUI::SliderScalar("slider s64 high",      LinearUIDataType_S64,    &s64_v, &s64_hi_a, &s64_hi_b, "%" PRId64);
        LinearUI::SliderScalar("slider s64 full",      LinearUIDataType_S64,    &s64_v, &s64_min,  &s64_max,  "%" PRId64);
        LinearUI::SliderScalar("slider u64 low",       LinearUIDataType_U64,    &u64_v, &u64_zero, &u64_fifty,"%" PRIu64 " ms");
        LinearUI::SliderScalar("slider u64 high",      LinearUIDataType_U64,    &u64_v, &u64_hi_a, &u64_hi_b, "%" PRIu64 " ms");
        LinearUI::SliderScalar("slider u64 full",      LinearUIDataType_U64,    &u64_v, &u64_min,  &u64_max,  "%" PRIu64 " ms");
        LinearUI::SliderScalar("slider float low",     LinearUIDataType_Float,  &f32_v, &f32_zero, &f32_one);
        LinearUI::SliderScalar("slider float low log", LinearUIDataType_Float,  &f32_v, &f32_zero, &f32_one,  "%.10f", LinearUISliderFlags_Logarithmic);
        LinearUI::SliderScalar("slider float high",    LinearUIDataType_Float,  &f32_v, &f32_lo_a, &f32_hi_a, "%e");
        LinearUI::SliderScalar("slider double low",    LinearUIDataType_Double, &f64_v, &f64_zero, &f64_one,  "%.10f grams");
        LinearUI::SliderScalar("slider double low log",LinearUIDataType_Double, &f64_v, &f64_zero, &f64_one,  "%.10f", LinearUISliderFlags_Logarithmic);
        LinearUI::SliderScalar("slider double high",   LinearUIDataType_Double, &f64_v, &f64_lo_a, &f64_hi_a, "%e grams");

        LinearUI::SeparatorText("Sliders (reverse)");
        LinearUI::SliderScalar("slider s8 reverse",    LinearUIDataType_S8,   &s8_v,  &s8_max,    &s8_min,   "%d");
        LinearUI::SliderScalar("slider u8 reverse",    LinearUIDataType_U8,   &u8_v,  &u8_max,    &u8_min,   "%u");
        LinearUI::SliderScalar("slider s32 reverse",   LinearUIDataType_S32,  &s32_v, &s32_fifty, &s32_zero, "%d");
        LinearUI::SliderScalar("slider u32 reverse",   LinearUIDataType_U32,  &u32_v, &u32_fifty, &u32_zero, "%u");
        LinearUI::SliderScalar("slider s64 reverse",   LinearUIDataType_S64,  &s64_v, &s64_fifty, &s64_zero, "%" PRId64);
        LinearUI::SliderScalar("slider u64 reverse",   LinearUIDataType_U64,  &u64_v, &u64_fifty, &u64_zero, "%" PRIu64 " ms");

        LINEARUI_DEMO_MARKER("Widgets/Data Types/Inputs");
        static bool inputs_step = true;
        static LinearUIInputTextFlags flags = LinearUIInputTextFlags_None;
        LinearUI::SeparatorText("Inputs");
        LinearUI::Checkbox("Show step buttons", &inputs_step);
        LinearUI::CheckboxFlags("LinearUIInputTextFlags_ReadOnly", &flags, LinearUIInputTextFlags_ReadOnly);
        LinearUI::CheckboxFlags("LinearUIInputTextFlags_ParseEmptyRefVal", &flags, LinearUIInputTextFlags_ParseEmptyRefVal);
        LinearUI::CheckboxFlags("LinearUIInputTextFlags_DisplayEmptyRefVal", &flags, LinearUIInputTextFlags_DisplayEmptyRefVal);
        LinearUI::InputScalar("input s8",      LinearUIDataType_S8,     &s8_v,  inputs_step ? &s8_one  : NULL, NULL, "%d", flags);
        LinearUI::InputScalar("input u8",      LinearUIDataType_U8,     &u8_v,  inputs_step ? &u8_one  : NULL, NULL, "%u", flags);
        LinearUI::InputScalar("input s16",     LinearUIDataType_S16,    &s16_v, inputs_step ? &s16_one : NULL, NULL, "%d", flags);
        LinearUI::InputScalar("input u16",     LinearUIDataType_U16,    &u16_v, inputs_step ? &u16_one : NULL, NULL, "%u", flags);
        LinearUI::InputScalar("input s32",     LinearUIDataType_S32,    &s32_v, inputs_step ? &s32_one : NULL, NULL, "%d", flags);
        LinearUI::InputScalar("input s32 hex", LinearUIDataType_S32,    &s32_v, inputs_step ? &s32_one : NULL, NULL, "%04X", flags);
        LinearUI::InputScalar("input u32",     LinearUIDataType_U32,    &u32_v, inputs_step ? &u32_one : NULL, NULL, "%u", flags);
        LinearUI::InputScalar("input u32 hex", LinearUIDataType_U32,    &u32_v, inputs_step ? &u32_one : NULL, NULL, "%08X", flags);
        LinearUI::InputScalar("input s64",     LinearUIDataType_S64,    &s64_v, inputs_step ? &s64_one : NULL, NULL, NULL, flags);
        LinearUI::InputScalar("input u64",     LinearUIDataType_U64,    &u64_v, inputs_step ? &u64_one : NULL, NULL, NULL, flags);
        LinearUI::InputScalar("input float",   LinearUIDataType_Float,  &f32_v, inputs_step ? &f32_one : NULL, NULL, NULL, flags);
        LinearUI::InputScalar("input double",  LinearUIDataType_Double, &f64_v, inputs_step ? &f64_one : NULL, NULL, NULL, flags);

        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsDisableBlocks()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsDisableBlocks(LinearUIDemoWindowData* demo_data)
{
    if (LinearUI::TreeNode("Disable Blocks"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Disable Blocks");
        LinearUI::Checkbox("Disable entire section above", &demo_data->DisableSections);
        LinearUI::SameLine(); HelpMarker("Demonstrate using BeginDisabled()/EndDisabled() across other sections.");
        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsDragAndDrop()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsDragAndDrop()
{
    if (LinearUI::TreeNode("Drag and Drop"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Drag and drop");
        if (LinearUI::TreeNode("Drag and drop in standard widgets"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Drag and drop/Standard widgets");
            // ColorEdit widgets automatically act as drag source and drag target.
            // They are using standardized payload strings LINEARUI_PAYLOAD_TYPE_COLOR_3F and LINEARUI_PAYLOAD_TYPE_COLOR_4F
            // to allow your own widgets to use colors in their drag and drop interaction.
            // Also see 'Demo->Widgets->Color/Picker Widgets->Palette' demo.
            HelpMarker("You can drag from the color squares.");
            static float col1[3] = { 1.0f, 0.0f, 0.2f };
            static float col2[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
            LinearUI::ColorEdit3("color 1", col1);
            LinearUI::ColorEdit4("color 2", col2);
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Drag and drop to copy/swap items"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Drag and drop/Copy-swap items");
            enum Mode
            {
                Mode_Copy,
                Mode_Move,
                Mode_Swap
            };
            static int mode = 0;
            if (LinearUI::RadioButton("Copy", mode == Mode_Copy)) { mode = Mode_Copy; } LinearUI::SameLine();
            if (LinearUI::RadioButton("Move", mode == Mode_Move)) { mode = Mode_Move; } LinearUI::SameLine();
            if (LinearUI::RadioButton("Swap", mode == Mode_Swap)) { mode = Mode_Swap; }
            static const char* names[9] =
            {
                "Bobby", "Beatrice", "Betty",
                "Brianna", "Barry", "Bernard",
                "Bibi", "Blaine", "Bryn"
            };
            for (int n = 0; n < LINEARUI_COUNTOF(names); n++)
            {
                LinearUI::PushID(n);
                if ((n % 3) != 0)
                    LinearUI::SameLine();
                LinearUI::Button(names[n], LinearVec2(60, 60));

                // Our buttons are both drag sources and drag targets here!
                if (LinearUI::BeginDragDropSource(LinearUIDragDropFlags_None))
                {
                    // Set payload to carry the index of our item (could be anything)
                    LinearUI::SetDragDropPayload("DND_DEMO_CELL", &n, sizeof(int));

                    // Display preview (could be anything, e.g. when dragging an image we could decide to display
                    // the filename and a small preview of the image, etc.)
                    if (mode == Mode_Copy) { LinearUI::Text("Copy %s", names[n]); }
                    if (mode == Mode_Move) { LinearUI::Text("Move %s", names[n]); }
                    if (mode == Mode_Swap) { LinearUI::Text("Swap %s", names[n]); }
                    LinearUI::EndDragDropSource();
                }
                if (LinearUI::BeginDragDropTarget())
                {
                    if (const LinearUIPayload* payload = LinearUI::AcceptDragDropPayload("DND_DEMO_CELL"))
                    {
                        LINEARUI_ASSERT(payload->DataSize == sizeof(int));
                        int payload_n = *(const int*)payload->Data;
                        if (mode == Mode_Copy)
                        {
                            names[n] = names[payload_n];
                        }
                        if (mode == Mode_Move)
                        {
                            names[n] = names[payload_n];
                            names[payload_n] = "";
                        }
                        if (mode == Mode_Swap)
                        {
                            const char* tmp = names[n];
                            names[n] = names[payload_n];
                            names[payload_n] = tmp;
                        }
                    }
                    LinearUI::EndDragDropTarget();
                }
                LinearUI::PopID();
            }
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Drag to reorder items (simple)"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Drag and Drop/Drag to reorder items (simple)");
            // FIXME: there is temporary (usually single-frame) ID Conflict during reordering as a same item may be submitting twice.
            // This code was always slightly faulty but in a way which was not easily noticeable.
            // Until we fix this, enable LinearUIItemFlags_AllowDuplicateId to disable detecting the issue.
            LinearUI::PushItemFlag(LinearUIItemFlags_AllowDuplicateId, true);

            // Simple reordering
            HelpMarker(
                "We don't use the drag and drop api at all here! "
                "Instead we query when the item is held but not hovered, and order items accordingly.");
            static const char* item_names[] = { "Item One", "Item Two", "Item Three", "Item Four", "Item Five" };
            for (int n = 0; n < LINEARUI_COUNTOF(item_names); n++)
            {
                const char* item = item_names[n];
                LinearUI::Selectable(item);

                if (LinearUI::IsItemActive() && !LinearUI::IsItemHovered())
                {
                    int n_next = n + (LinearUI::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
                    if (n_next >= 0 && n_next < LINEARUI_COUNTOF(item_names))
                    {
                        item_names[n] = item_names[n_next];
                        item_names[n_next] = item;
                        LinearUI::ResetMouseDragDelta();
                    }
                }
            }

            LinearUI::PopItemFlag();
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Tooltip at target location"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Drag and Drop/Tooltip at target location");
            for (int n = 0; n < 2; n++)
            {
                // Drop targets
                LinearUI::Button(n ? "drop here##1" : "drop here##0");
                if (LinearUI::BeginDragDropTarget())
                {
                    LinearUIDragDropFlags drop_target_flags = LinearUIDragDropFlags_AcceptBeforeDelivery | LinearUIDragDropFlags_AcceptNoPreviewTooltip;
                    if (const LinearUIPayload* payload = LinearUI::AcceptDragDropPayload(LINEARUI_PAYLOAD_TYPE_COLOR_4F, drop_target_flags))
                    {
                        LINEARUI_UNUSED(payload);
                        LinearUI::SetMouseCursor(LinearUIMouseCursor_NotAllowed);
                        LinearUI::SetTooltip("Cannot drop here!");
                    }
                    LinearUI::EndDragDropTarget();
                }

                // Drop source
                static LinearVec4 col4 = { 1.0f, 0.0f, 0.2f, 1.0f };
                if (n == 0)
                    LinearUI::ColorButton("drag me", col4);

            }
            LinearUI::TreePop();
        }

        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsDragsAndSliders()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsDragsAndSliders()
{
    if (LinearUI::TreeNode("Drag/Slider Flags"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Drag and Slider Flags");
        // Demonstrate using advanced flags for DragXXX and SliderXXX functions. Note that the flags are the same!
        static LinearUISliderFlags flags = LinearUISliderFlags_None;
        LinearUI::CheckboxFlags("LinearUISliderFlags_AlwaysClamp", &flags, LinearUISliderFlags_AlwaysClamp);
        LinearUI::CheckboxFlags("LinearUISliderFlags_ClampOnInput", &flags, LinearUISliderFlags_ClampOnInput);
        LinearUI::SameLine(); HelpMarker("Clamp value to min/max bounds when input manually with Ctrl+Click. By default Ctrl+Click allows going out of bounds.");
        LinearUI::CheckboxFlags("LinearUISliderFlags_ClampZeroRange", &flags, LinearUISliderFlags_ClampZeroRange);
        LinearUI::SameLine(); HelpMarker("Clamp even if min==max==0.0f. Otherwise DragXXX functions don't clamp.");
        LinearUI::CheckboxFlags("LinearUISliderFlags_Logarithmic", &flags, LinearUISliderFlags_Logarithmic);
        LinearUI::SameLine(); HelpMarker("Enable logarithmic editing (more precision for small values).");
        LinearUI::CheckboxFlags("LinearUISliderFlags_NoRoundToFormat", &flags, LinearUISliderFlags_NoRoundToFormat);
        LinearUI::SameLine(); HelpMarker("Disable rounding underlying value to match precision of the format string (e.g. %.3f values are rounded to those 3 digits).");
        LinearUI::CheckboxFlags("LinearUISliderFlags_NoInput", &flags, LinearUISliderFlags_NoInput);
        LinearUI::SameLine(); HelpMarker("Disable Ctrl+Click or Enter key allowing to input text directly into the widget.");
        LinearUI::CheckboxFlags("LinearUISliderFlags_NoSpeedTweaks", &flags, LinearUISliderFlags_NoSpeedTweaks);
        LinearUI::SameLine(); HelpMarker("Disable keyboard modifiers altering tweak speed. Useful if you want to alter tweak speed yourself based on your own logic.");
        LinearUI::CheckboxFlags("LinearUISliderFlags_WrapAround", &flags, LinearUISliderFlags_WrapAround);
        LinearUI::SameLine(); HelpMarker("Enable wrapping around from max to min and from min to max (only supported by DragXXX() functions)");
        LinearUI::CheckboxFlags("LinearUISliderFlags_ColorMarkers", &flags, LinearUISliderFlags_ColorMarkers);

        // Drags
        static float drag_f = 0.5f;
        static float drag_f4[4];
        static int drag_i = 50;
        LinearUI::Text("Underlying float value: %f", drag_f);
        LinearUI::DragFloat("DragFloat (0 -> 1)", &drag_f, 0.005f, 0.0f, 1.0f, "%.3f", flags);
        LinearUI::DragFloat("DragFloat (0 -> +inf)", &drag_f, 0.005f, 0.0f, FLT_MAX, "%.3f", flags);
        LinearUI::DragFloat("DragFloat (-inf -> 1)", &drag_f, 0.005f, -FLT_MAX, 1.0f, "%.3f", flags);
        LinearUI::DragFloat("DragFloat (-inf -> +inf)", &drag_f, 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
        //LinearUI::DragFloat("DragFloat (0 -> 0)", &drag_f, 0.005f, 0.0f, 0.0f, "%.3f", flags);           // To test ClampZeroRange
        //LinearUI::DragFloat("DragFloat (100 -> 100)", &drag_f, 0.005f, 100.0f, 100.0f, "%.3f", flags);
        LinearUI::DragInt("DragInt (0 -> 100)", &drag_i, 0.5f, 0, 100, "%d", flags);
        LinearUI::DragFloat4("DragFloat4 (0 -> 1)", drag_f4, 0.005f, 0.0f, 1.0f, "%.3f", flags); // Multi-component item, mostly here to document the effect of LinearUISliderFlags_ColorMarkers.

        // Sliders
        static float slider_f = 0.5f;
        static float slider_f4[4];
        static int slider_i = 50;
        const LinearUISliderFlags flags_for_sliders = (flags & ~LinearUISliderFlags_WrapAround);
        LinearUI::Text("Underlying float value: %f", slider_f);
        LinearUI::SliderFloat("SliderFloat (0 -> 1)", &slider_f, 0.0f, 1.0f, "%.3f", flags_for_sliders);
        LinearUI::SliderInt("SliderInt (0 -> 100)", &slider_i, 0, 100, "%d", flags_for_sliders);
        LinearUI::SliderFloat4("SliderFloat4 (0 -> 1)", slider_f4, 0.0f, 1.0f, "%.3f", flags); // Multi-component item, mostly here to document the effect of LinearUISliderFlags_ColorMarkers.

        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsFonts()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsFonts()
{
    if (LinearUI::TreeNode("Fonts"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Fonts");
        LinearFontAtlas* atlas = LinearUI::GetIO().Fonts;
        LinearUI::ShowFontAtlas(atlas);
        // FIXME-NEWATLAS: Provide a demo to add/create a procedural font?
        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsImages()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsImages()
{
    if (LinearUI::TreeNode("Images"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Images");
        LinearUIIO& io = LinearUI::GetIO();
        LinearUI::TextWrapped(
            "Below we are displaying the font texture (which is the only texture we have access to in this demo). "
            "Use the 'LinearTextureID' type as storage to pass pointers or identifier to your own texture data. "
            "Hover the texture for a zoomed view!");

        // Below we are displaying the font texture because it is the only texture we have access to inside the demo!
        // Read description about LinearTextureID/LinearTextureRef and FAQ for details about texture identifiers.
        // If you use one of the default linearui_impl_XXXX.cpp rendering backend, they all have comments at the top
        // of their respective source file to specify what they are using as texture identifier, for example:
        // - The linearui_impl_dx11.cpp renderer expect a 'ID3D11ShaderResourceView*' pointer.
        // - The linearui_impl_opengl3.cpp renderer expect a GLuint OpenGL texture identifier, etc.
        // So with the DirectX11 backend, you call LinearUI::Image() with a 'ID3D11ShaderResourceView*' cast to LinearTextureID.
        // - If you decided that LinearTextureID = MyEngineTexture*, then you can pass your MyEngineTexture* pointers
        //   to LinearUI::Image(), and gather width/height through your own functions, etc.
        // - You can use ShowMetricsWindow() to inspect the draw data that are being passed to your renderer,
        //   it will help you debug issues if you are confused about it.
        // - Consider using the lower-level LinearDrawList::AddImage() API, via LinearUI::GetWindowDrawList()->AddImage().
        // - Read https://github.com/ocornut/linearui/blob/master/docs/FAQ.md
        // - Read https://github.com/ocornut/linearui/wiki/Image-Loading-and-Displaying-Examples

        // Grab the current texture identifier used by the font atlas.
        LinearFontAtlas* atlas = io.Fonts;
        LinearTextureRef my_tex_id = atlas->TexRef;
        float my_tex_w = (float)atlas->TexData->Width; // Regular user code should never have to care about TexData-> fields, but since we want to display the entire texture here, we pull Width/Height from it.
        float my_tex_h = (float)atlas->TexData->Height;
        LinearUI::Text("%.0fx%.0f", my_tex_w, my_tex_h);

        // Basic drawing
        LinearUI::SeparatorText("Image()/ImageWithBg() function");
        LinearVec2 uv_min = LinearVec2(0.0f, 0.0f); // Top-left
        LinearVec2 uv_max = LinearVec2(1.0f, 1.0f); // Lower-right
        LinearUI::PushStyleVar(LinearUIStyleVar_ImageBorderSize, LINEARUI_MAX(1.0f, LinearUI::GetStyle().ImageBorderSize));
        LinearUI::ImageWithBg(my_tex_id, LinearVec2(my_tex_w, my_tex_h), uv_min, uv_max, LinearVec4(0.0f, 0.0f, 0.0f, 1.0f));
        LinearUI::PopStyleVar();

        // Fancy widget
        LinearUI::SeparatorText("Interactive Image Viewer");
        static ExampleImageViewerData image_viewer;
        LinearVec2 canvas_size(LinearUI::GetContentRegionAvail().x, my_tex_h * 2.0f);
        ExampleImageViewer_DrawOptions(&image_viewer);
        ExampleImageViewer_DrawCanvas(&image_viewer, canvas_size, my_tex_id, (int)my_tex_w, (int)my_tex_h);

        LINEARUI_DEMO_MARKER("Widgets/Images/Textured buttons");
        LinearUI::SeparatorText("Textured Buttons");
        LinearUI::TextWrapped("And now some textured buttons..");
        static int pressed_count = 0;
        for (int i = 0; i < 8; i++)
        {
            // UV coordinates are often (0.0f, 0.0f) and (1.0f, 1.0f) to display an entire textures.
            // Here are trying to display only a 32x32 pixels area of the texture, hence the UV computation.
            // Read about UV coordinates here: https://github.com/ocornut/linearui/wiki/Image-Loading-and-Displaying-Examples
            LinearUI::PushID(i);
            if (i > 0)
                LinearUI::PushStyleVar(LinearUIStyleVar_FramePadding, LinearVec2(i - 1.0f, i - 1.0f));
            LinearVec2 size = LinearVec2(32.0f, 32.0f);                         // Size of the image we want to make visible
            LinearVec2 uv0 = LinearVec2(0.0f, 0.0f);                            // UV coordinates for lower-left
            LinearVec2 uv1 = LinearVec2(32.0f / my_tex_w, 32.0f / my_tex_h);    // UV coordinates for (32,32) in our texture
            LinearVec4 bg_col = LinearVec4(0.0f, 0.0f, 0.0f, 1.0f);             // Black background
            LinearVec4 tint_col = LinearVec4(1.0f, 1.0f, 1.0f, 1.0f);           // No tint
            if (LinearUI::ImageButton("", my_tex_id, size, uv0, uv1, bg_col, tint_col))
                pressed_count += 1;
            if (i > 0)
                LinearUI::PopStyleVar();
            LinearUI::PopID();
            LinearUI::SameLine();
        }
        LinearUI::NewLine();
        LinearUI::Text("Pressed %d times.", pressed_count);
        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsListBoxes()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsListBoxes()
{
    if (LinearUI::TreeNode("List Boxes"))
    {
        LINEARUI_DEMO_MARKER("Widgets/List Boxes");
        // BeginListBox() is essentially a thin wrapper to using BeginChild()/EndChild()
        // using the LinearUIChildFlags_FrameStyle flag for stylistic changes + displaying a label.
        // You may be tempted to simply use BeginChild() directly. However note that BeginChild() requires EndChild()
        // to always be called (inconsistent with BeginListBox()/EndListBox()).

        // Using the generic BeginListBox() API, you have full control over how to display the combo contents.
        // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
        // stored in the object itself, etc.)
        const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
        static int item_selected_idx = 0; // Here we store our selected data as an index.

        static bool item_highlight = false;
        int item_highlighted_idx = -1; // Here we store our highlighted data as an index.
        LinearUI::Checkbox("Highlight hovered item in second listbox", &item_highlight);

        if (LinearUI::BeginListBox("listbox 1"))
        {
            for (int n = 0; n < LINEARUI_COUNTOF(items); n++)
            {
                const bool is_selected = (item_selected_idx == n);
                if (LinearUI::Selectable(items[n], is_selected))
                    item_selected_idx = n;

                if (item_highlight && LinearUI::IsItemHovered())
                    item_highlighted_idx = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    LinearUI::SetItemDefaultFocus();
            }
            LinearUI::EndListBox();
        }
        LinearUI::SameLine(); HelpMarker("Here we are sharing selection state between both boxes.");

        // Custom size: use all width, 5 items tall
        LinearUI::Text("Full-width:");
        if (LinearUI::BeginListBox("##listbox 2", LinearVec2(-FLT_MIN, 5 * LinearUI::GetTextLineHeightWithSpacing())))
        {
            for (int n = 0; n < LINEARUI_COUNTOF(items); n++)
            {
                bool is_selected = (item_selected_idx == n);
                LinearUISelectableFlags flags = (item_highlighted_idx == n) ? LinearUISelectableFlags_Highlight : 0;
                if (LinearUI::Selectable(items[n], is_selected, flags))
                    item_selected_idx = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    LinearUI::SetItemDefaultFocus();
            }
            LinearUI::EndListBox();
        }

        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsMultiComponents()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsMultiComponents()
{
    if (LinearUI::TreeNode("Multi-component Widgets"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Multi-component Widgets");
        static float vec4f[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
        static int vec4i[4] = { 1, 5, 100, 255 };

        static LinearUISliderFlags flags = 0;
        LinearUI::CheckboxFlags("LinearUISliderFlags_ColorMarkers", &flags, LinearUISliderFlags_ColorMarkers); // Only passing this to Drag/Sliders

        LinearUI::SeparatorText("2-wide");
        LinearUI::InputFloat2("input float2", vec4f);
        LinearUI::InputInt2("input int2", vec4i);
        LinearUI::DragFloat2("drag float2", vec4f, 0.01f, 0.0f, 1.0f, NULL, flags);
        LinearUI::DragInt2("drag int2", vec4i, 1, 0, 255, NULL, flags);
        LinearUI::SliderFloat2("slider float2", vec4f, 0.0f, 1.0f, NULL, flags);
        LinearUI::SliderInt2("slider int2", vec4i, 0, 255, NULL, flags);

        LinearUI::SeparatorText("3-wide");
        LinearUI::InputFloat3("input float3", vec4f);
        LinearUI::InputInt3("input int3", vec4i);
        LinearUI::DragFloat3("drag float3", vec4f, 0.01f, 0.0f, 1.0f, NULL, flags);
        LinearUI::DragInt3("drag int3", vec4i, 1, 0, 255, NULL, flags);
        LinearUI::SliderFloat3("slider float3", vec4f, 0.0f, 1.0f, NULL, flags);
        LinearUI::SliderInt3("slider int3", vec4i, 0, 255, NULL, flags);

        LinearUI::SeparatorText("4-wide");
        LinearUI::InputFloat4("input float4", vec4f);
        LinearUI::InputInt4("input int4", vec4i);
        LinearUI::DragFloat4("drag float4", vec4f, 0.01f, 0.0f, 1.0f, NULL, flags);
        LinearUI::DragInt4("drag int4", vec4i, 1, 0, 255, NULL, flags);
        LinearUI::SliderFloat4("slider float4", vec4f, 0.0f, 1.0f, NULL, flags);
        LinearUI::SliderInt4("slider int4", vec4i, 0, 255, NULL, flags);

        LinearUI::SeparatorText("Ranges");
        static float begin = 10, end = 90;
        static int begin_i = 100, end_i = 1000;
        LinearUI::DragFloatRange2("range float", &begin, &end, 0.25f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%", LinearUISliderFlags_AlwaysClamp);
        LinearUI::DragIntRange2("range int", &begin_i, &end_i, 5, 0, 1000, "Min: %d units", "Max: %d units");
        LinearUI::DragIntRange2("range int (no bounds)", &begin_i, &end_i, 5, 0, 0, "Min: %d units", "Max: %d units");

        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsPlotting()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsPlotting()
{
    // Plot/Graph widgets are not very good.
// Consider using a third-party library such as LinearPlot: https://github.com/epezent/implot
// (see others https://github.com/ocornut/linearui/wiki/Useful-Extensions)
    if (LinearUI::TreeNode("Plotting"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Plotting");
        LinearUI::Text("Need better plotting and graphing? Consider using LinearPlot:");
        LinearUI::TextLinkOpenURL("https://github.com/epezent/implot");
        LinearUI::Separator();

        static bool animate = true;
        LinearUI::Checkbox("Animate", &animate);

        // Plot as lines and plot as histogram
        static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
        LinearUI::PlotLines("Frame Times", arr, LINEARUI_COUNTOF(arr));
        LinearUI::PlotHistogram("Histogram", arr, LINEARUI_COUNTOF(arr), 0, NULL, 0.0f, 1.0f, LinearVec2(0, 80.0f));
        //LinearUI::SameLine(); HelpMarker("Consider using LinearPlot instead!");

        // Fill an array of contiguous float values to plot
        // Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float
        // and the sizeof() of your structure in the "stride" parameter.
        static float values[90] = {};
        static int values_offset = 0;
        static double refresh_time = 0.0;
        if (!animate || refresh_time == 0.0)
            refresh_time = LinearUI::GetTime();
        while (refresh_time < LinearUI::GetTime()) // Create data at fixed 60 Hz rate for the demo
        {
            static float phase = 0.0f;
            values[values_offset] = cosf(phase);
            values_offset = (values_offset + 1) % LINEARUI_COUNTOF(values);
            phase += 0.10f * values_offset;
            refresh_time += 1.0f / 60.0f;
        }

        // Plots can display overlay texts
        // (in this example, we will display an average value)
        {
            float average = 0.0f;
            for (int n = 0; n < LINEARUI_COUNTOF(values); n++)
                average += values[n];
            average /= (float)LINEARUI_COUNTOF(values);
            char overlay[32];
            sprintf(overlay, "avg %f", average);
            LinearUI::PlotLines("Lines", values, LINEARUI_COUNTOF(values), values_offset, overlay, -1.0f, 1.0f, LinearVec2(0, 80.0f));
        }

        // Use functions to generate output
        // FIXME: This is actually VERY awkward because current plot API only pass in indices.
        // We probably want an API passing floats and user provide sample rate/count.
        struct Funcs
        {
            static float Sin(void*, int i) { return sinf(i * 0.1f); }
            static float Saw(void*, int i) { return (i & 1) ? 1.0f : -1.0f; }
        };
        static int func_type = 0, display_count = 70;
        LinearUI::SeparatorText("Functions");
        LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 8);
        LinearUI::Combo("func", &func_type, "Sin\0Saw\0");
        LinearUI::SameLine();
        LinearUI::SliderInt("Sample count", &display_count, 1, 400);
        float (*func)(void*, int) = (func_type == 0) ? Funcs::Sin : Funcs::Saw;
        LinearUI::PlotLines("Lines##2", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, LinearVec2(0, 80));
        LinearUI::PlotHistogram("Histogram##2", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, LinearVec2(0, 80));

        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsProgressBars()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsProgressBars()
{
    if (LinearUI::TreeNode("Progress Bars"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Progress Bars");
        // Animate a simple progress bar
        static float progress_accum = 0.0f, progress_dir = 1.0f;
        progress_accum += progress_dir * 0.4f * LinearUI::GetIO().DeltaTime;
        if (progress_accum >= +1.1f) { progress_accum = +1.1f; progress_dir *= -1.0f; }
        if (progress_accum <= -0.1f) { progress_accum = -0.1f; progress_dir *= -1.0f; }

        const float progress = LINEARUI_CLAMP(progress_accum, 0.0f, 1.0f);

        // Typically we would use LinearVec2(-1.0f,0.0f) or LinearVec2(-FLT_MIN,0.0f) to use all available width,
        // or LinearVec2(width,0.0f) for a specified width. LinearVec2(0.0f,0.0f) uses ItemWidth.
        LinearUI::ProgressBar(progress, LinearVec2(0.0f, 0.0f));
        LinearUI::SameLine(0.0f, LinearUI::GetStyle().ItemInnerSpacing.x);
        LinearUI::Text("Progress Bar");

        char buf[32];
        sprintf(buf, "%d/%d", (int)(progress * 1753), 1753);
        LinearUI::ProgressBar(progress, LinearVec2(0.f, 0.f), buf);

        // Pass an animated negative value, e.g. -1.0f * (float)LinearUI::GetTime() is the recommended value.
        // Adjust the factor if you want to adjust the animation speed.
        LinearUI::ProgressBar(-1.0f * (float)LinearUI::GetTime(), LinearVec2(0.0f, 0.0f), "Searching..");
        LinearUI::SameLine(0.0f, LinearUI::GetStyle().ItemInnerSpacing.x);
        LinearUI::Text("Indeterminate");

        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsQueryingStatuses()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsQueryingStatuses()
{
    if (LinearUI::TreeNode("Querying Item Status (Edited/Active/Hovered etc.)"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Querying Item Status (Edited,Active,Hovered etc.)");
        // Select an item type
        const char* item_names[] =
        {
            "Text", "Button", "Button (w/ repeat)", "Checkbox", "SliderFloat", "InputText", "InputTextMultiline", "InputFloat",
            "InputFloat3", "ColorEdit4", "Selectable", "MenuItem", "TreeNode", "TreeNode (w/ double-click)", "Combo", "ListBox"
        };
        static int item_type = 4;
        static bool item_disabled = false;
        LinearUI::Combo("Item Type", &item_type, item_names, LINEARUI_COUNTOF(item_names), LINEARUI_COUNTOF(item_names));
        LinearUI::SameLine();
        HelpMarker("Testing how various types of items are interacting with the IsItemXXX functions. Note that the bool return value of most LinearUI function is generally equivalent to calling LinearUI::IsItemHovered().");
        LinearUI::Checkbox("Item Disabled", &item_disabled);

        // Submit selected items so we can query their status in the code following it.
        bool ret = false;
        static bool b = false;
        static float col4f[4] = { 1.0f, 0.5, 0.0f, 1.0f };
        static char str[16] = {};
        if (item_disabled)
            LinearUI::BeginDisabled(true);
        if (item_type == 0) { LinearUI::Text("ITEM: Text"); }                                              // Testing text items with no identifier/interaction
        if (item_type == 1) { ret = LinearUI::Button("ITEM: Button"); }                                    // Testing button
        if (item_type == 2) { LinearUI::PushItemFlag(LinearUIItemFlags_ButtonRepeat, true); ret = LinearUI::Button("ITEM: Button"); LinearUI::PopItemFlag(); } // Testing button (with repeater)
        if (item_type == 3) { ret = LinearUI::Checkbox("ITEM: Checkbox", &b); }                            // Testing checkbox
        if (item_type == 4) { ret = LinearUI::SliderFloat("ITEM: SliderFloat", &col4f[0], 0.0f, 1.0f); }   // Testing basic item
        if (item_type == 5) { ret = LinearUI::InputText("ITEM: InputText", &str[0], LINEARUI_COUNTOF(str)); }  // Testing input text (which handles tabbing)
        if (item_type == 6) { ret = LinearUI::InputTextMultiline("ITEM: InputTextMultiline", &str[0], LINEARUI_COUNTOF(str)); } // Testing input text (which uses a child window)
        if (item_type == 7) { ret = LinearUI::InputFloat("ITEM: InputFloat", col4f, 1.0f); }               // Testing +/- buttons on scalar input
        if (item_type == 8) { ret = LinearUI::InputFloat3("ITEM: InputFloat3", col4f); }                   // Testing multi-component items (IsItemXXX flags are reported merged)
        if (item_type == 9) { ret = LinearUI::ColorEdit4("ITEM: ColorEdit4", col4f); }                     // Testing multi-component items (IsItemXXX flags are reported merged)
        if (item_type == 10) { ret = LinearUI::Selectable("ITEM: Selectable"); }                            // Testing selectable item
        if (item_type == 11) { ret = LinearUI::MenuItem("ITEM: MenuItem"); }                                // Testing menu item (they use LinearUIButtonFlags_PressedOnRelease button policy)
        if (item_type == 12) { ret = LinearUI::TreeNode("ITEM: TreeNode"); if (ret) LinearUI::TreePop(); }     // Testing tree node
        if (item_type == 13) { ret = LinearUI::TreeNodeEx("ITEM: TreeNode w/ LinearUITreeNodeFlags_OpenOnDoubleClick", LinearUITreeNodeFlags_OpenOnDoubleClick | LinearUITreeNodeFlags_NoTreePushOnOpen); } // Testing tree node with LinearUIButtonFlags_PressedOnDoubleClick button policy.
        if (item_type == 14) { const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi" }; static int current = 1; ret = LinearUI::Combo("ITEM: Combo", &current, items, LINEARUI_COUNTOF(items)); }
        if (item_type == 15) { const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi" }; static int current = 1; ret = LinearUI::ListBox("ITEM: ListBox", &current, items, LINEARUI_COUNTOF(items), LINEARUI_COUNTOF(items)); }

        bool hovered_delay_none = LinearUI::IsItemHovered();
        bool hovered_delay_stationary = LinearUI::IsItemHovered(LinearUIHoveredFlags_Stationary);
        bool hovered_delay_short = LinearUI::IsItemHovered(LinearUIHoveredFlags_DelayShort);
        bool hovered_delay_normal = LinearUI::IsItemHovered(LinearUIHoveredFlags_DelayNormal);
        bool hovered_delay_tooltip = LinearUI::IsItemHovered(LinearUIHoveredFlags_ForTooltip); // = Normal + Stationary

        // Display the values of IsItemHovered() and other common item state functions.
        // Note that the LinearUIHoveredFlags_XXX flags can be combined.
        // Because BulletText is an item itself and that would affect the output of IsItemXXX functions,
        // we query every state in a single call to avoid storing them and to simplify the code.
        LinearUI::BulletText(
            "Return value = %d\n"
            "IsItemFocused() = %d\n"
            "IsItemHovered() = %d\n"
            "IsItemHovered(_AllowWhenBlockedByPopup) = %d\n"
            "IsItemHovered(_AllowWhenBlockedByActiveItem) = %d\n"
            "IsItemHovered(_AllowWhenOverlappedByItem) = %d\n"
            "IsItemHovered(_AllowWhenOverlappedByWindow) = %d\n"
            "IsItemHovered(_AllowWhenDisabled) = %d\n"
            "IsItemHovered(_RectOnly) = %d\n"
            "IsItemActive() = %d\n"
            "IsItemEdited() = %d\n"
            "IsItemActivated() = %d\n"
            "IsItemDeactivated() = %d\n"
            "IsItemDeactivatedAfterEdit() = %d\n"
            "IsItemVisible() = %d\n"
            "IsItemClicked() = %d\n"
            "IsItemToggledOpen() = %d\n"
            "GetItemRectMin() = (%.1f, %.1f)\n"
            "GetItemRectMax() = (%.1f, %.1f)\n"
            "GetItemRectSize() = (%.1f, %.1f)",
            ret,
            LinearUI::IsItemFocused(),
            LinearUI::IsItemHovered(),
            LinearUI::IsItemHovered(LinearUIHoveredFlags_AllowWhenBlockedByPopup),
            LinearUI::IsItemHovered(LinearUIHoveredFlags_AllowWhenBlockedByActiveItem),
            LinearUI::IsItemHovered(LinearUIHoveredFlags_AllowWhenOverlappedByItem),
            LinearUI::IsItemHovered(LinearUIHoveredFlags_AllowWhenOverlappedByWindow),
            LinearUI::IsItemHovered(LinearUIHoveredFlags_AllowWhenDisabled),
            LinearUI::IsItemHovered(LinearUIHoveredFlags_RectOnly),
            LinearUI::IsItemActive(),
            LinearUI::IsItemEdited(),
            LinearUI::IsItemActivated(),
            LinearUI::IsItemDeactivated(),
            LinearUI::IsItemDeactivatedAfterEdit(),
            LinearUI::IsItemVisible(),
            LinearUI::IsItemClicked(),
            LinearUI::IsItemToggledOpen(),
            LinearUI::GetItemRectMin().x, LinearUI::GetItemRectMin().y,
            LinearUI::GetItemRectMax().x, LinearUI::GetItemRectMax().y,
            LinearUI::GetItemRectSize().x, LinearUI::GetItemRectSize().y
        );
        LinearUI::BulletText(
            "with Hovering Delay or Stationary test:\n"
            "IsItemHovered() = %d\n"
            "IsItemHovered(_Stationary) = %d\n"
            "IsItemHovered(_DelayShort) = %d\n"
            "IsItemHovered(_DelayNormal) = %d\n"
            "IsItemHovered(_Tooltip) = %d",
            hovered_delay_none, hovered_delay_stationary, hovered_delay_short, hovered_delay_normal, hovered_delay_tooltip);

        if (item_disabled)
            LinearUI::EndDisabled();

        char buf[1] = "";
        LinearUI::InputText("unused", buf, LINEARUI_COUNTOF(buf), LinearUIInputTextFlags_ReadOnly);
        LinearUI::SameLine();
        HelpMarker("This widget is only here to be able to tab-out of the widgets above and see e.g. Deactivated() status.");

        LinearUI::TreePop();
    }

    if (LinearUI::TreeNode("Querying Window Status (Focused/Hovered etc.)"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Querying Window Status (Focused,Hovered etc.)");
        static bool embed_all_inside_a_child_window = false;
        LinearUI::Checkbox("Embed everything inside a child window for testing _RootWindow flag.", &embed_all_inside_a_child_window);
        if (embed_all_inside_a_child_window)
            LinearUI::BeginChild("outer_child", LinearVec2(0, LinearUI::GetFontSize() * 20.0f), LinearUIChildFlags_Borders);

        // Testing IsWindowFocused() function with its various flags.
        LinearUI::BulletText(
            "IsWindowFocused() = %d\n"
            "IsWindowFocused(_ChildWindows) = %d\n"
            "IsWindowFocused(_ChildWindows|_NoPopupHierarchy) = %d\n"
            "IsWindowFocused(_ChildWindows|_DockHierarchy) = %d\n"
            "IsWindowFocused(_ChildWindows|_RootWindow) = %d\n"
            "IsWindowFocused(_ChildWindows|_RootWindow|_NoPopupHierarchy) = %d\n"
            "IsWindowFocused(_ChildWindows|_RootWindow|_DockHierarchy) = %d\n"
            "IsWindowFocused(_RootWindow) = %d\n"
            "IsWindowFocused(_RootWindow|_NoPopupHierarchy) = %d\n"
            "IsWindowFocused(_RootWindow|_DockHierarchy) = %d\n"
            "IsWindowFocused(_AnyWindow) = %d\n",
            LinearUI::IsWindowFocused(),
            LinearUI::IsWindowFocused(LinearUIFocusedFlags_ChildWindows),
            LinearUI::IsWindowFocused(LinearUIFocusedFlags_ChildWindows | LinearUIFocusedFlags_NoPopupHierarchy),
            LinearUI::IsWindowFocused(LinearUIFocusedFlags_ChildWindows | LinearUIFocusedFlags_DockHierarchy),
            LinearUI::IsWindowFocused(LinearUIFocusedFlags_ChildWindows | LinearUIFocusedFlags_RootWindow),
            LinearUI::IsWindowFocused(LinearUIFocusedFlags_ChildWindows | LinearUIFocusedFlags_RootWindow | LinearUIFocusedFlags_NoPopupHierarchy),
            LinearUI::IsWindowFocused(LinearUIFocusedFlags_ChildWindows | LinearUIFocusedFlags_RootWindow | LinearUIFocusedFlags_DockHierarchy),
            LinearUI::IsWindowFocused(LinearUIFocusedFlags_RootWindow),
            LinearUI::IsWindowFocused(LinearUIFocusedFlags_RootWindow | LinearUIFocusedFlags_NoPopupHierarchy),
            LinearUI::IsWindowFocused(LinearUIFocusedFlags_RootWindow | LinearUIFocusedFlags_DockHierarchy),
            LinearUI::IsWindowFocused(LinearUIFocusedFlags_AnyWindow));

        // Testing IsWindowHovered() function with its various flags.
        LinearUI::BulletText(
            "IsWindowHovered() = %d\n"
            "IsWindowHovered(_AllowWhenBlockedByPopup) = %d\n"
            "IsWindowHovered(_AllowWhenBlockedByActiveItem) = %d\n"
            "IsWindowHovered(_ChildWindows) = %d\n"
            "IsWindowHovered(_ChildWindows|_NoPopupHierarchy) = %d\n"
            "IsWindowHovered(_ChildWindows|_DockHierarchy) = %d\n"
            "IsWindowHovered(_ChildWindows|_RootWindow) = %d\n"
            "IsWindowHovered(_ChildWindows|_RootWindow|_NoPopupHierarchy) = %d\n"
            "IsWindowHovered(_ChildWindows|_RootWindow|_DockHierarchy) = %d\n"
            "IsWindowHovered(_RootWindow) = %d\n"
            "IsWindowHovered(_RootWindow|_NoPopupHierarchy) = %d\n"
            "IsWindowHovered(_RootWindow|_DockHierarchy) = %d\n"
            "IsWindowHovered(_ChildWindows|_AllowWhenBlockedByPopup) = %d\n"
            "IsWindowHovered(_AnyWindow) = %d\n"
            "IsWindowHovered(_Stationary) = %d\n",
            LinearUI::IsWindowHovered(),
            LinearUI::IsWindowHovered(LinearUIHoveredFlags_AllowWhenBlockedByPopup),
            LinearUI::IsWindowHovered(LinearUIHoveredFlags_AllowWhenBlockedByActiveItem),
            LinearUI::IsWindowHovered(LinearUIHoveredFlags_ChildWindows),
            LinearUI::IsWindowHovered(LinearUIHoveredFlags_ChildWindows | LinearUIHoveredFlags_NoPopupHierarchy),
            LinearUI::IsWindowHovered(LinearUIHoveredFlags_ChildWindows | LinearUIHoveredFlags_DockHierarchy),
            LinearUI::IsWindowHovered(LinearUIHoveredFlags_ChildWindows | LinearUIHoveredFlags_RootWindow),
            LinearUI::IsWindowHovered(LinearUIHoveredFlags_ChildWindows | LinearUIHoveredFlags_RootWindow | LinearUIHoveredFlags_NoPopupHierarchy),
            LinearUI::IsWindowHovered(LinearUIHoveredFlags_ChildWindows | LinearUIHoveredFlags_RootWindow | LinearUIHoveredFlags_DockHierarchy),
            LinearUI::IsWindowHovered(LinearUIHoveredFlags_RootWindow),
            LinearUI::IsWindowHovered(LinearUIHoveredFlags_RootWindow | LinearUIHoveredFlags_NoPopupHierarchy),
            LinearUI::IsWindowHovered(LinearUIHoveredFlags_RootWindow | LinearUIHoveredFlags_DockHierarchy),
            LinearUI::IsWindowHovered(LinearUIHoveredFlags_ChildWindows | LinearUIHoveredFlags_AllowWhenBlockedByPopup),
            LinearUI::IsWindowHovered(LinearUIHoveredFlags_AnyWindow),
            LinearUI::IsWindowHovered(LinearUIHoveredFlags_Stationary));

        LinearUI::BeginChild("child", LinearVec2(0, 50), LinearUIChildFlags_Borders);
        LinearUI::Text("This is another child window for testing the _ChildWindows flag.");
        LinearUI::EndChild();
        if (embed_all_inside_a_child_window)
            LinearUI::EndChild();

        // Calling IsItemHovered() after begin returns the hovered status of the title bar.
        // This is useful in particular if you want to create a context menu associated to the title bar of a window.
        // This will also work when docked into a Tab (the Tab replace the Title Bar and guarantee the same properties).
        static bool test_window = false;
        LinearUI::Checkbox("Hovered/Active tests after Begin() for title bar testing", &test_window);
        if (test_window)
        {
            // FIXME-DOCK: This window cannot be docked within the LinearUI Demo window, this will cause a feedback loop and get them stuck.
            // Could we fix this through an LinearUIWindowClass feature? Or an API call to tag our parent as "don't skip items"?
            LinearUI::Begin("Title bar Hovered/Active tests", &test_window);
            if (LinearUI::BeginPopupContextItem()) // <-- This is using IsItemHovered()
            {
                if (LinearUI::MenuItem("Close")) { test_window = false; }
                LinearUI::EndPopup();
            }
            LinearUI::Text(
                "IsItemHovered() after begin = %d (== is title bar hovered)\n"
                "IsItemActive() after begin = %d (== is window being clicked/moved)\n",
                LinearUI::IsItemHovered(), LinearUI::IsItemActive());
            LinearUI::End();
        }

        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsSelectables()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsSelectables()
{
    //LinearUI::SetNextItemOpen(true, LinearUICond_Once);
    if (LinearUI::TreeNode("Selectables"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Selectables");
        // Selectable() has 2 overloads:
        // - The one taking "bool selected" as a read-only selection information.
        //   When Selectable() has been clicked it returns true and you can alter selection state accordingly.
        // - The one taking "bool* p_selected" as a read-write selection information (convenient in some cases)
        // The earlier is more flexible, as in real application your selection may be stored in many different ways
        // and not necessarily inside a bool value (e.g. in flags within objects, as an external list, etc).
        LINEARUI_DEMO_MARKER("Widgets/Selectables/Basic");
        if (LinearUI::TreeNode("Basic"))
        {
            static bool selection[5] = { false, true, false, false };
            LinearUI::Selectable("1. I am selectable", &selection[0]);
            LinearUI::Selectable("2. I am selectable", &selection[1]);
            LinearUI::Selectable("3. I am selectable", &selection[2]);
            if (LinearUI::Selectable("4. I am double clickable", selection[3], LinearUISelectableFlags_AllowDoubleClick))
                if (LinearUI::IsMouseDoubleClicked(0))
                    selection[3] = !selection[3];
            LinearUI::TreePop();
        }

        LINEARUI_DEMO_MARKER("Widgets/Selectables/Rendering more items on the same line");
        if (LinearUI::TreeNode("Multiple items on the same line"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selectables/Multiple items on the same line");
            // - Using SetNextItemAllowOverlap()
            // - Using the Selectable() override that takes "bool* p_selected" parameter, the bool value is toggled automatically.
            {
                static bool selected[3] = {};
                LinearUI::SetNextItemAllowOverlap(); LinearUI::Selectable("main.c", &selected[0]); LinearUI::SameLine(); LinearUI::SmallButton("Link 1");
                LinearUI::SetNextItemAllowOverlap(); LinearUI::Selectable("hello.cpp", &selected[1]); LinearUI::SameLine(); LinearUI::SmallButton("Link 2");
                LinearUI::SetNextItemAllowOverlap(); LinearUI::Selectable("hello.h", &selected[2]); LinearUI::SameLine(); LinearUI::SmallButton("Link 3");
            }

            // (2)
            // - Using LinearUISelectableFlags_AllowOverlap is a shortcut for calling SetNextItemAllowOverlap()
            // - No visible label, display contents inside the selectable bounds.
            // - We don't maintain actual selection in this example to keep things simple.
            LinearUI::Spacing();
            {
                static bool checked[5] = {};
                static int selected_n = 0;
                const float color_marker_w = LinearUI::CalcTextSize("x").x;
                for (int n = 0; n < 5; n++)
                {
                    LinearUI::PushID(n);
                    LinearUI::AlignTextToFramePadding();
                    if (LinearUI::Selectable("##selectable", selected_n == n, LinearUISelectableFlags_AllowOverlap))
                        selected_n = n;
                    LinearUI::SameLine(0, 0);
                    LinearUI::Checkbox("##check", &checked[n]);
                    LinearUI::SameLine();
                    LinearVec4 color((n & 1) ? 1.0f : 0.2f, (n & 2) ? 1.0f : 0.2f, 0.2f, 1.0f);
                    LinearUI::ColorButton("##color", color, LinearUIColorEditFlags_NoTooltip, LinearVec2(color_marker_w, 0));
                    LinearUI::SameLine();
                    LinearUI::Text("Some label");
                    LinearUI::PopID();
                }
            }

            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("In Tables"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selectables/In Tables");
            static bool selected[10] = {};

            if (LinearUI::BeginTable("split1", 3, LinearUITableFlags_Resizable | LinearUITableFlags_NoSavedSettings | LinearUITableFlags_Borders))
            {
                for (int i = 0; i < 10; i++)
                {
                    char label[32];
                    sprintf(label, "Item %d", i);
                    LinearUI::TableNextColumn();
                    LinearUI::Selectable(label, &selected[i]); // FIXME-TABLE: Selection overlap
                }
                LinearUI::EndTable();
            }
            LinearUI::Spacing();
            if (LinearUI::BeginTable("split2", 3, LinearUITableFlags_Resizable | LinearUITableFlags_NoSavedSettings | LinearUITableFlags_Borders))
            {
                for (int i = 0; i < 10; i++)
                {
                    char label[32];
                    sprintf(label, "Item %d", i);
                    LinearUI::TableNextRow();
                    LinearUI::TableNextColumn();
                    LinearUI::Selectable(label, &selected[i], LinearUISelectableFlags_SpanAllColumns);
                    LinearUI::TableNextColumn();
                    LinearUI::Text("Some other contents");
                    LinearUI::TableNextColumn();
                    LinearUI::Text("123456");
                }
                LinearUI::EndTable();
            }
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Grid"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selectables/Grid");
            static char selected[4][4] = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };

            // Add in a bit of silly fun...
            const float time = (float)LinearUI::GetTime();
            const bool winning_state = memchr(selected, 0, sizeof(selected)) == NULL; // If all cells are selected...
            if (winning_state)
                LinearUI::PushStyleVar(LinearUIStyleVar_SelectableTextAlign, LinearVec2(0.5f + 0.5f * cosf(time * 2.0f), 0.5f + 0.5f * sinf(time * 3.0f)));

            const float size = LinearUI::CalcTextSize("Sailor").x;
            for (int y = 0; y < 4; y++)
                for (int x = 0; x < 4; x++)
                {
                    if (x > 0)
                        LinearUI::SameLine();
                    LinearUI::PushID(y * 4 + x);
                    if (LinearUI::Selectable("Sailor", selected[y][x] != 0, 0, LinearVec2(size, size)))
                    {
                        // Toggle clicked cell + toggle neighbors
                        selected[y][x] ^= 1;
                        if (x > 0) { selected[y][x - 1] ^= 1; }
                        if (x < 3) { selected[y][x + 1] ^= 1; }
                        if (y > 0) { selected[y - 1][x] ^= 1; }
                        if (y < 3) { selected[y + 1][x] ^= 1; }
                    }
                    LinearUI::PopID();
                }

            if (winning_state)
                LinearUI::PopStyleVar();
            LinearUI::TreePop();
        }
        if (LinearUI::TreeNode("Alignment"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selectables/Alignment");
            HelpMarker(
                "By default, Selectables uses style.SelectableTextAlign but it can be overridden on a per-item "
                "basis using PushStyleVar(). You'll probably want to always keep your default situation to "
                "left-align otherwise it becomes difficult to layout multiple items on a same line");

            static bool selected[3 * 3] = { true, false, true, false, true, false, true, false, true };
            const float size = LinearUI::CalcTextSize("(1.0,1.0)").x;
            for (int y = 0; y < 3; y++)
            {
                for (int x = 0; x < 3; x++)
                {
                    LinearVec2 alignment = LinearVec2((float)x / 2.0f, (float)y / 2.0f);
                    char name[32];
                    sprintf(name, "(%.1f,%.1f)", alignment.x, alignment.y);
                    if (x > 0) LinearUI::SameLine();
                    LinearUI::PushStyleVar(LinearUIStyleVar_SelectableTextAlign, alignment);
                    LinearUI::Selectable(name, &selected[3 * y + x], LinearUISelectableFlags_None, LinearVec2(size, size));
                    LinearUI::PopStyleVar();
                }
            }
            LinearUI::TreePop();
        }
        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsSelectionAndMultiSelect()
//-----------------------------------------------------------------------------
// Multi-selection demos
// Also read: https://github.com/ocornut/linearui/wiki/Multi-Select
//-----------------------------------------------------------------------------

static const char* ExampleNames[] =
{
    "Artichoke", "Arugula", "Asparagus", "Avocado", "Bamboo Shoots", "Bean Sprouts", "Beans", "Beet", "Belgian Endive", "Bell Pepper",
    "Bitter Gourd", "Bok Choy", "Broccoli", "Brussels Sprouts", "Burdock Root", "Cabbage", "Calabash", "Capers", "Carrot", "Cassava",
    "Cauliflower", "Celery", "Celery Root", "Celcuce", "Chayote", "Chinese Broccoli", "Corn", "Cucumber"
};

// Extra functions to add deletion support to LinearUISelectionBasicStorage
struct ExampleSelectionWithDeletion : LinearUISelectionBasicStorage
{
    // Find which item should be Focused after deletion.
    // Call _before_ item submission. Return an index in the before-deletion item list, your item loop should call SetKeyboardFocusHere() on it.
    // The subsequent ApplyDeletionPostLoop() code will use it to apply Selection.
    // - We cannot provide this logic in core Dear LinearUI because we don't have access to selection data.
    // - We don't actually manipulate the LinearVector<> here, only in ApplyDeletionPostLoop(), but using similar API for consistency and flexibility.
    // - Important: Deletion only works if the underlying LinearUIID for your items are stable: aka not depend on their index, but on e.g. item id/ptr.
    // FIXME-MULTISELECT: Doesn't take account of the possibility focus target will be moved during deletion. Need refocus or scroll offset.
    int ApplyDeletionPreLoop(LinearUIMultiSelectIO* ms_io, int items_count)
    {
        if (Size == 0)
            return -1;

        // If focused item is not selected...
        const int focused_idx = (int)ms_io->NavIdItem;  // Index of currently focused item
        if (ms_io->NavIdSelected == false)  // This is merely a shortcut, == Contains(adapter->IndexToStorage(items, focused_idx))
        {
            ms_io->RangeSrcReset = true;    // Request to recover RangeSrc from NavId next frame. Would be ok to reset even when NavIdSelected==true, but it would take an extra frame to recover RangeSrc when deleting a selected item.
            return focused_idx;             // Request to focus same item after deletion.
        }

        // If focused item is selected: land on first unselected item after focused item.
        for (int idx = focused_idx + 1; idx < items_count; idx++)
            if (!Contains(GetStorageIdFromIndex(idx)))
                return idx;

        // If focused item is selected: otherwise return last unselected item before focused item.
        for (int idx = LINEARUI_MIN(focused_idx, items_count) - 1; idx >= 0; idx--)
            if (!Contains(GetStorageIdFromIndex(idx)))
                return idx;

        return -1;
    }

    // Rewrite item list (delete items) + update selection.
    // - Call after EndMultiSelect()
    // - We cannot provide this logic in core Dear LinearUI because we don't have access to your items, nor to selection data.
    template<typename ITEM_TYPE>
    void ApplyDeletionPostLoop(LinearUIMultiSelectIO* ms_io, LinearVector<ITEM_TYPE>& items, int item_curr_idx_to_select)
    {
        // Rewrite item list (delete items) + convert old selection index (before deletion) to new selection index (after selection).
        // If NavId was not part of selection, we will stay on same item.
        LinearVector<ITEM_TYPE> new_items;
        new_items.reserve(items.Size - Size);
        int item_next_idx_to_select = -1;
        for (int idx = 0; idx < items.Size; idx++)
        {
            if (!Contains(GetStorageIdFromIndex(idx)))
                new_items.push_back(items[idx]);
            if (item_curr_idx_to_select == idx)
                item_next_idx_to_select = new_items.Size - 1;
        }
        items.swap(new_items);

        // Update selection
        Clear();
        if (item_next_idx_to_select != -1 && ms_io->NavIdSelected)
            SetItemSelected(GetStorageIdFromIndex(item_next_idx_to_select), true);
    }
};

// Example: Implement dual list box storage and interface
struct ExampleDualListBox
{
    LinearVector<LinearUIID>           Items[2];               // ID is index into ExampleName[]
    LinearUISelectionBasicStorage  Selections[2];          // Store ExampleItemId into selection
    bool                        OptKeepSorted = true;

    void MoveAll(int src, int dst)
    {
        LINEARUI_ASSERT((src == 0 && dst == 1) || (src == 1 && dst == 0));
        for (LinearUIID item_id : Items[src])
            Items[dst].push_back(item_id);
        Items[src].clear();
        SortItems(dst);
        Selections[src].Swap(Selections[dst]);
        Selections[src].Clear();
    }
    void MoveSelected(int src, int dst)
    {
        for (int src_n = 0; src_n < Items[src].Size; src_n++)
        {
            LinearUIID item_id = Items[src][src_n];
            if (!Selections[src].Contains(item_id))
                continue;
            Items[src].erase(&Items[src][src_n]); // FIXME-OPT: Could be implemented more optimally (rebuild src items and swap)
            Items[dst].push_back(item_id);
            src_n--;
        }
        if (OptKeepSorted)
            SortItems(dst);
        Selections[src].Swap(Selections[dst]);
        Selections[src].Clear();
    }
    void ApplySelectionRequests(LinearUIMultiSelectIO* ms_io, int side)
    {
        // In this example we store item id in selection (instead of item index)
        Selections[side].UserData = Items[side].Data;
        Selections[side].AdapterIndexToStorageId = [](LinearUISelectionBasicStorage* self, int idx) { LinearUIID* items = (LinearUIID*)self->UserData; return items[idx]; };
        Selections[side].ApplyRequests(ms_io);
    }
    static int LINEARUI_CDECL CompareItemsByValue(const void* lhs, const void* rhs)
    {
        const int* a = (const int*)lhs;
        const int* b = (const int*)rhs;
        return *a - *b;
    }
    void SortItems(int n)
    {
        qsort(Items[n].Data, (size_t)Items[n].Size, sizeof(Items[n][0]), CompareItemsByValue);
    }
    void Show()
    {
        //if (LinearUI::Checkbox("Sorted", &OptKeepSorted) && OptKeepSorted) { SortItems(0); SortItems(1); }
        if (LinearUI::BeginTable("split", 3, LinearUITableFlags_None))
        {
            LinearUI::TableSetupColumn("", LinearUITableColumnFlags_WidthStretch);    // Left side
            LinearUI::TableSetupColumn("", LinearUITableColumnFlags_WidthFixed);      // Buttons
            LinearUI::TableSetupColumn("", LinearUITableColumnFlags_WidthStretch);    // Right side
            LinearUI::TableNextRow();

            int request_move_selected = -1;
            int request_move_all = -1;
            float child_height_0 = 0.0f;
            for (int side = 0; side < 2; side++)
            {
                // FIXME-MULTISELECT: Dual List Box: Add context menus
                // FIXME-NAV: Using LinearUIWindowFlags_NavFlattened exhibit many issues.
                LinearVector<LinearUIID>& items = Items[side];
                LinearUISelectionBasicStorage& selection = Selections[side];

                LinearUI::TableSetColumnIndex((side == 0) ? 0 : 2);
                LinearUI::Text("%s (%d)", (side == 0) ? "Available" : "Basket", items.Size);

                // Submit scrolling range to avoid glitches on moving/deletion
                const float items_height = LinearUI::GetTextLineHeightWithSpacing();
                LinearUI::SetNextWindowContentSize(LinearVec2(0.0f, items.Size * items_height));

                bool child_visible;
                if (side == 0)
                {
                    // Left child is resizable
                    LinearUI::SetNextWindowSizeConstraints(LinearVec2(0.0f, LinearUI::GetFrameHeightWithSpacing() * 4), LinearVec2(FLT_MAX, FLT_MAX));
                    child_visible = LinearUI::BeginChild("0", LinearVec2(-FLT_MIN, LinearUI::GetFontSize() * 20), LinearUIChildFlags_FrameStyle | LinearUIChildFlags_ResizeY);
                    child_height_0 = LinearUI::GetWindowSize().y;
                }
                else
                {
                    // Right child use same height as left one
                    child_visible = LinearUI::BeginChild("1", LinearVec2(-FLT_MIN, child_height_0), LinearUIChildFlags_FrameStyle);
                }
                if (child_visible)
                {
                    LinearUIMultiSelectFlags flags = LinearUIMultiSelectFlags_BoxSelect1d;
                    LinearUIMultiSelectIO* ms_io = LinearUI::BeginMultiSelect(flags, selection.Size, items.Size);
                    ApplySelectionRequests(ms_io, side);

                    for (int item_n = 0; item_n < items.Size; item_n++)
                    {
                        LinearUIID item_id = items[item_n];
                        bool item_is_selected = selection.Contains(item_id);
                        LinearUI::SetNextItemSelectionUserData(item_n);
                        LinearUI::Selectable(ExampleNames[item_id], item_is_selected, LinearUISelectableFlags_AllowDoubleClick);
                        if (LinearUI::IsItemFocused())
                        {
                            // FIXME-MULTISELECT: Dual List Box: Transfer focus
                            if (LinearUI::IsKeyPressed(LinearUIKey_Enter) || LinearUI::IsKeyPressed(LinearUIKey_KeypadEnter))
                                request_move_selected = side;
                            if (LinearUI::IsMouseDoubleClicked(0)) // FIXME-MULTISELECT: Double-click on multi-selection?
                                request_move_selected = side;
                        }
                    }

                    ms_io = LinearUI::EndMultiSelect();
                    ApplySelectionRequests(ms_io, side);
                }
                LinearUI::EndChild();
            }

            // Buttons columns
            LinearUI::TableSetColumnIndex(1);
            LinearUI::NewLine();
            //LinearVec2 button_sz = { LinearUI::CalcTextSize(">>").x + LinearUI::GetStyle().FramePadding.x * 2.0f, LinearUI::GetFrameHeight() + padding.y * 2.0f };
            LinearVec2 button_sz = { LinearUI::GetFrameHeight(), LinearUI::GetFrameHeight() };

            // (Using BeginDisabled()/EndDisabled() works but feels distracting given how it is currently visualized)
            if (LinearUI::Button(">>", button_sz))
                request_move_all = 0;
            if (LinearUI::Button(">", button_sz))
                request_move_selected = 0;
            if (LinearUI::Button("<", button_sz))
                request_move_selected = 1;
            if (LinearUI::Button("<<", button_sz))
                request_move_all = 1;

            // Process requests
            if (request_move_all != -1)
                MoveAll(request_move_all, request_move_all ^ 1);
            if (request_move_selected != -1)
                MoveSelected(request_move_selected, request_move_selected ^ 1);

            // FIXME-MULTISELECT: Support action from outside
            /*
            if (OptKeepSorted == false)
            {
                LinearUI::NewLine();
                if (LinearUI::ArrowButton("MoveUp", LinearUIDir_Up)) {}
                if (LinearUI::ArrowButton("MoveDown", LinearUIDir_Down)) {}
            }
            */

            LinearUI::EndTable();
        }
    }
};

static void DemoWindowWidgetsSelectionAndMultiSelect(LinearUIDemoWindowData* demo_data)
{
    if (LinearUI::TreeNode("Selection State & Multi-Select"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Selection State & Multi-Select");
        HelpMarker("Selections can be built using Selectable(), TreeNode() or other widgets. Selection state is owned by application code/data.");

        LinearUI::BulletText("Wiki page:");
        LinearUI::SameLine();
        LinearUI::TextLinkOpenURL("linearui/wiki/Multi-Select", "https://github.com/ocornut/linearui/wiki/Multi-Select");

        // Without any fancy API: manage single-selection yourself.
        if (LinearUI::TreeNode("Single-Select"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selection State/Single-Select");
            static int selected = -1;
            for (int n = 0; n < 5; n++)
            {
                char buf[32];
                sprintf(buf, "Object %d", n);
                if (LinearUI::Selectable(buf, selected == n))
                    selected = n;
            }
            LinearUI::TreePop();
        }

        // Demonstrate implementation a most-basic form of multi-selection manually
        // This doesn't support the Shift modifier which requires BeginMultiSelect()!
        if (LinearUI::TreeNode("Multi-Select (manual/simplified, without BeginMultiSelect)"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (manual/simplified, without BeginMultiSelect)");
            HelpMarker("Hold Ctrl and Click to select multiple items.");
            static bool selection[5] = { false, false, false, false, false };
            for (int n = 0; n < 5; n++)
            {
                char buf[32];
                sprintf(buf, "Object %d", n);
                if (LinearUI::Selectable(buf, selection[n]))
                {
                    if (!LinearUI::GetIO().KeyCtrl) // Clear selection when Ctrl is not held
                        memset(selection, 0, sizeof(selection));
                    selection[n] ^= 1; // Toggle current item
                }
            }
            LinearUI::TreePop();
        }

        // Demonstrate handling proper multi-selection using the BeginMultiSelect/EndMultiSelect API.
        // Shift+Click w/ Ctrl and other standard features are supported.
        // We use the LinearUISelectionBasicStorage helper which you may freely reimplement.
        if (LinearUI::TreeNode("Multi-Select"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selection State/Multi-Select");
            LinearUI::Text("Supported features:");
            LinearUI::BulletText("Keyboard navigation (arrows, page up/down, home/end, space).");
            LinearUI::BulletText("Ctrl modifier to preserve and toggle selection.");
            LinearUI::BulletText("Shift modifier for range selection.");
            LinearUI::BulletText("Ctrl+A to select all.");
            LinearUI::BulletText("Escape to clear selection.");
            LinearUI::BulletText("Click and drag to box-select.");
            LinearUI::Text("Tip: Use 'Demo->Tools->Debug Log->Selection' to see selection requests as they happen.");

            // Use default selection.Adapter: Pass index to SetNextItemSelectionUserData(), store index in Selection
            const int ITEMS_COUNT = 50;
            static LinearUISelectionBasicStorage selection;
            LinearUI::Text("Selection: %d/%d", selection.Size, ITEMS_COUNT);

            // The BeginChild() has no purpose for selection logic, other that offering a scrolling region.
            if (LinearUI::BeginChild("##Basket", LinearVec2(-FLT_MIN, LinearUI::GetFontSize() * 20), LinearUIChildFlags_FrameStyle | LinearUIChildFlags_ResizeY))
            {
                LinearUIMultiSelectFlags flags = LinearUIMultiSelectFlags_ClearOnEscape | LinearUIMultiSelectFlags_BoxSelect1d;
                LinearUIMultiSelectIO* ms_io = LinearUI::BeginMultiSelect(flags, selection.Size, ITEMS_COUNT);
                selection.ApplyRequests(ms_io);

                for (int n = 0; n < ITEMS_COUNT; n++)
                {
                    char label[64];
                    sprintf(label, "Object %05d: %s", n, ExampleNames[n % LINEARUI_COUNTOF(ExampleNames)]);
                    bool item_is_selected = selection.Contains((LinearUIID)n);
                    LinearUI::SetNextItemSelectionUserData(n);
                    LinearUI::Selectable(label, item_is_selected);
                }

                ms_io = LinearUI::EndMultiSelect();
                selection.ApplyRequests(ms_io);
            }
            LinearUI::EndChild();
            LinearUI::TreePop();
        }

        // Demonstrate using the clipper with BeginMultiSelect()/EndMultiSelect()
        if (LinearUI::TreeNode("Multi-Select (with clipper)"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (with clipper)");
            // Use default selection.Adapter: Pass index to SetNextItemSelectionUserData(), store index in Selection
            static LinearUISelectionBasicStorage selection;

            LinearUI::Text("Added features:");
            LinearUI::BulletText("Using LinearUIListClipper.");

            const int ITEMS_COUNT = 10000;
            LinearUI::Text("Selection: %d/%d", selection.Size, ITEMS_COUNT);
            if (LinearUI::BeginChild("##Basket", LinearVec2(-FLT_MIN, LinearUI::GetFontSize() * 20), LinearUIChildFlags_FrameStyle | LinearUIChildFlags_ResizeY))
            {
                LinearUIMultiSelectFlags flags = LinearUIMultiSelectFlags_ClearOnEscape | LinearUIMultiSelectFlags_BoxSelect1d;
                LinearUIMultiSelectIO* ms_io = LinearUI::BeginMultiSelect(flags, selection.Size, ITEMS_COUNT);
                selection.ApplyRequests(ms_io);

                LinearUIListClipper clipper;
                clipper.Begin(ITEMS_COUNT);
                if (ms_io->RangeSrcItem != -1)
                    clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem); // Ensure RangeSrc item is not clipped.
                while (clipper.Step())
                {
                    for (int n = clipper.DisplayStart; n < clipper.DisplayEnd; n++)
                    {
                        char label[64];
                        sprintf(label, "Object %05d: %s", n, ExampleNames[n % LINEARUI_COUNTOF(ExampleNames)]);
                        bool item_is_selected = selection.Contains((LinearUIID)n);
                        LinearUI::SetNextItemSelectionUserData(n);
                        LinearUI::Selectable(label, item_is_selected);
                    }
                }

                ms_io = LinearUI::EndMultiSelect();
                selection.ApplyRequests(ms_io);
            }
            LinearUI::EndChild();
            LinearUI::TreePop();
        }

        // Demonstrate dynamic item list + deletion support using the BeginMultiSelect/EndMultiSelect API.
        // In order to support Deletion without any glitches you need to:
        // - (1) If items are submitted in their own scrolling area, submit contents size SetNextWindowContentSize() ahead of time to prevent one-frame readjustment of scrolling.
        // - (2) Items needs to have persistent ID Stack identifier = ID needs to not depends on their index. PushID(index) = KO. PushID(item_id) = OK. This is in order to focus items reliably after a selection.
        // - (3) BeginXXXX process
        // - (4) Focus process
        // - (5) EndXXXX process
        if (LinearUI::TreeNode("Multi-Select (with deletion)"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (with deletion)");
            // Storing items data separately from selection data.
            // (you may decide to store selection data inside your item (aka intrusive storage) if you don't need multiple views over same items)
            // Use a custom selection.Adapter: store item identifier in Selection (instead of index)
            static LinearVector<LinearUIID> items;
            static ExampleSelectionWithDeletion selection;
            selection.UserData = (void*)&items;
            selection.AdapterIndexToStorageId = [](LinearUISelectionBasicStorage* self, int idx) { LinearVector<LinearUIID>* p_items = (LinearVector<LinearUIID>*)self->UserData; return (*p_items)[idx]; }; // Index -> ID

            LinearUI::Text("Added features:");
            LinearUI::BulletText("Dynamic list with Delete key support.");
            LinearUI::Text("Selection size: %d/%d", selection.Size, items.Size);

            // Initialize default list with 50 items + button to add/remove items.
            static LinearUIID items_next_id = 0;
            if (items_next_id == 0)
                for (LinearUIID n = 0; n < 50; n++)
                    items.push_back(items_next_id++);
            if (LinearUI::SmallButton("Add 20 items"))     { for (int n = 0; n < 20; n++) { items.push_back(items_next_id++); } }
            LinearUI::SameLine();
            if (LinearUI::SmallButton("Remove 20 items"))  { for (int n = LINEARUI_MIN(20, items.Size); n > 0; n--) { selection.SetItemSelected(items.back(), false); items.pop_back(); } }

            // (1) Extra to support deletion: Submit scrolling range to avoid glitches on deletion
            const float items_height = LinearUI::GetTextLineHeightWithSpacing();
            LinearUI::SetNextWindowContentSize(LinearVec2(0.0f, items.Size * items_height));

            if (LinearUI::BeginChild("##Basket", LinearVec2(-FLT_MIN, LinearUI::GetFontSize() * 20), LinearUIChildFlags_FrameStyle | LinearUIChildFlags_ResizeY))
            {
                LinearUIMultiSelectFlags flags = LinearUIMultiSelectFlags_ClearOnEscape | LinearUIMultiSelectFlags_BoxSelect1d;
                LinearUIMultiSelectIO* ms_io = LinearUI::BeginMultiSelect(flags, selection.Size, items.Size);
                selection.ApplyRequests(ms_io);

                const bool want_delete = LinearUI::Shortcut(LinearUIKey_Delete, LinearUIInputFlags_Repeat) && (selection.Size > 0);
                const int item_curr_idx_to_focus = want_delete ? selection.ApplyDeletionPreLoop(ms_io, items.Size) : -1;

                for (int n = 0; n < items.Size; n++)
                {
                    const LinearUIID item_id = items[n];
                    char label[64];
                    sprintf(label, "Object %05u: %s", item_id, ExampleNames[item_id % LINEARUI_COUNTOF(ExampleNames)]);

                    bool item_is_selected = selection.Contains(item_id);
                    LinearUI::SetNextItemSelectionUserData(n);
                    LinearUI::Selectable(label, item_is_selected);
                    if (item_curr_idx_to_focus == n)
                        LinearUI::SetKeyboardFocusHere(-1);
                }

                // Apply multi-select requests
                ms_io = LinearUI::EndMultiSelect();
                selection.ApplyRequests(ms_io);
                if (want_delete)
                    selection.ApplyDeletionPostLoop(ms_io, items, item_curr_idx_to_focus);
            }
            LinearUI::EndChild();
            LinearUI::TreePop();
        }

        // Implement a Dual List Box (#6648)
        if (LinearUI::TreeNode("Multi-Select (dual list box)"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (dual list box)");
            // Init default state
            static ExampleDualListBox dlb;
            if (dlb.Items[0].Size == 0 && dlb.Items[1].Size == 0)
                for (int item_id = 0; item_id < LINEARUI_COUNTOF(ExampleNames); item_id++)
                    dlb.Items[0].push_back((LinearUIID)item_id);

            // Show
            dlb.Show();

            LinearUI::TreePop();
        }

        // Demonstrate using the clipper with BeginMultiSelect()/EndMultiSelect()
        if (LinearUI::TreeNode("Multi-Select (in a table)"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (in a table)");
            static LinearUISelectionBasicStorage selection;

            const int ITEMS_COUNT = 10000;
            LinearUI::Text("Selection: %d/%d", selection.Size, ITEMS_COUNT);
            if (LinearUI::BeginTable("##Basket", 2, LinearUITableFlags_ScrollY | LinearUITableFlags_RowBg | LinearUITableFlags_BordersOuter, LinearVec2(0.0f, LinearUI::GetFontSize() * 20)))
            {
                LinearUI::TableSetupColumn("Object");
                LinearUI::TableSetupColumn("Action");
                LinearUI::TableSetupScrollFreeze(0, 1);
                LinearUI::TableHeadersRow();

                LinearUIMultiSelectFlags flags = LinearUIMultiSelectFlags_ClearOnEscape | LinearUIMultiSelectFlags_BoxSelect1d;
                LinearUIMultiSelectIO* ms_io = LinearUI::BeginMultiSelect(flags, selection.Size, ITEMS_COUNT);
                selection.ApplyRequests(ms_io);

                LinearUIListClipper clipper;
                clipper.Begin(ITEMS_COUNT);
                if (ms_io->RangeSrcItem != -1)
                    clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem); // Ensure RangeSrc item is not clipped.
                while (clipper.Step())
                {
                    for (int n = clipper.DisplayStart; n < clipper.DisplayEnd; n++)
                    {
                        LinearUI::TableNextRow();
                        LinearUI::TableNextColumn();
                        LinearUI::PushID(n);
                        char label[64];
                        sprintf(label, "Object %05d: %s", n, ExampleNames[n % LINEARUI_COUNTOF(ExampleNames)]);
                        bool item_is_selected = selection.Contains((LinearUIID)n);
                        LinearUI::SetNextItemSelectionUserData(n);
                        LinearUI::Selectable(label, item_is_selected, LinearUISelectableFlags_SpanAllColumns | LinearUISelectableFlags_AllowOverlap);
                        LinearUI::TableNextColumn();
                        LinearUI::SmallButton("hello");
                        LinearUI::PopID();
                    }
                }

                ms_io = LinearUI::EndMultiSelect();
                selection.ApplyRequests(ms_io);
                LinearUI::EndTable();
            }
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Multi-Select (checkboxes)"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (checkboxes)");
            LinearUI::Text("In a list of checkboxes (not selectable):");
            LinearUI::BulletText("Using _NoAutoSelect + _NoAutoClear flags.");
            LinearUI::BulletText("Shift+Click to check multiple boxes.");
            LinearUI::BulletText("Shift+Keyboard to copy current value to other boxes.");

            // If you have an array of checkboxes, you may want to use NoAutoSelect + NoAutoClear and the LinearUISelectionExternalStorage helper.
            static bool items[20] = {};
            static LinearUIMultiSelectFlags flags = LinearUIMultiSelectFlags_NoAutoSelect | LinearUIMultiSelectFlags_NoAutoClear | LinearUIMultiSelectFlags_ClearOnEscape;
            LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_NoAutoSelect", &flags, LinearUIMultiSelectFlags_NoAutoSelect);
            LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_NoAutoClear", &flags, LinearUIMultiSelectFlags_NoAutoClear);
            LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_BoxSelect2d", &flags, LinearUIMultiSelectFlags_BoxSelect2d); // Cannot use LinearUIMultiSelectFlags_BoxSelect1d as checkboxes are varying width.

            if (LinearUI::BeginChild("##Basket", LinearVec2(-FLT_MIN, LinearUI::GetFontSize() * 20), LinearUIChildFlags_Borders | LinearUIChildFlags_ResizeY))
            {
                LinearUIMultiSelectIO* ms_io = LinearUI::BeginMultiSelect(flags, -1, LINEARUI_COUNTOF(items));
                LinearUISelectionExternalStorage storage_wrapper;
                storage_wrapper.UserData = (void*)items;
                storage_wrapper.AdapterSetItemSelected = [](LinearUISelectionExternalStorage* self, int n, bool selected) { bool* array = (bool*)self->UserData; array[n] = selected; };
                storage_wrapper.ApplyRequests(ms_io);
                for (int n = 0; n < 20; n++)
                {
                    char label[32];
                    sprintf(label, "Item %d", n);
                    LinearUI::SetNextItemSelectionUserData(n);
                    LinearUI::Checkbox(label, &items[n]);
                }
                ms_io = LinearUI::EndMultiSelect();
                storage_wrapper.ApplyRequests(ms_io);
            }
            LinearUI::EndChild();

            LinearUI::TreePop();
        }

        // Demonstrate individual selection scopes in same window
        if (LinearUI::TreeNode("Multi-Select (multiple scopes)"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (multiple scopes)");
            // Use default select: Pass index to SetNextItemSelectionUserData(), store index in Selection
            const int SCOPES_COUNT = 3;
            const int ITEMS_COUNT = 8; // Per scope
            static LinearUISelectionBasicStorage selections_data[SCOPES_COUNT];

            // Use LinearUIMultiSelectFlags_ScopeRect to not affect other selections in same window.
            static LinearUIMultiSelectFlags flags = LinearUIMultiSelectFlags_ScopeRect | LinearUIMultiSelectFlags_ClearOnEscape;// | LinearUIMultiSelectFlags_ClearOnClickVoid;
            if (LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_ScopeWindow", &flags, LinearUIMultiSelectFlags_ScopeWindow) && (flags & LinearUIMultiSelectFlags_ScopeWindow))
                flags &= ~LinearUIMultiSelectFlags_ScopeRect;
            if (LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_ScopeRect", &flags, LinearUIMultiSelectFlags_ScopeRect) && (flags & LinearUIMultiSelectFlags_ScopeRect))
                flags &= ~LinearUIMultiSelectFlags_ScopeWindow;
            LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_ClearOnClickVoid", &flags, LinearUIMultiSelectFlags_ClearOnClickVoid);
            LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_BoxSelect1d", &flags, LinearUIMultiSelectFlags_BoxSelect1d);

            for (int selection_scope_n = 0; selection_scope_n < SCOPES_COUNT; selection_scope_n++)
            {
                LinearUI::PushID(selection_scope_n);
                LinearUISelectionBasicStorage* selection = &selections_data[selection_scope_n];
                LinearUIMultiSelectIO* ms_io = LinearUI::BeginMultiSelect(flags, selection->Size, ITEMS_COUNT);
                selection->ApplyRequests(ms_io);

                LinearUI::SeparatorText("Selection scope");
                LinearUI::Text("Selection size: %d/%d", selection->Size, ITEMS_COUNT);

                for (int n = 0; n < ITEMS_COUNT; n++)
                {
                    char label[64];
                    sprintf(label, "Object %05d: %s", n, ExampleNames[n % LINEARUI_COUNTOF(ExampleNames)]);
                    bool item_is_selected = selection->Contains((LinearUIID)n);
                    LinearUI::SetNextItemSelectionUserData(n);
                    LinearUI::Selectable(label, item_is_selected);
                }

                // Apply multi-select requests
                ms_io = LinearUI::EndMultiSelect();
                selection->ApplyRequests(ms_io);
                LinearUI::PopID();
            }
            LinearUI::TreePop();
        }

        // See ShowExampleAppAssetsBrowser()
        if (LinearUI::TreeNode("Multi-Select (tiled assets browser)"))
        {
            LinearUI::Checkbox("Assets Browser", &demo_data->ShowAppAssetsBrowser);
            LinearUI::Text("(also access from 'Examples->Assets Browser' in menu)");
            LinearUI::TreePop();
        }

        // Demonstrate supporting multiple-selection in a tree.
        // - We don't use linear indices for selection user data, but our ExampleTreeNode* pointer directly!
        //   This showcase how SetNextItemSelectionUserData() never assume indices!
        // - The difficulty here is to "interpolate" from RangeSrcItem to RangeDstItem in the SetAll/SetRange request.
        //   We want this interpolation to match what the user sees: in visible order, skipping closed nodes.
        //   This is implemented by our TreeGetNextNodeInVisibleOrder() user-space helper.
        // - Important: In a real codebase aiming to implement full-featured selectable tree with custom filtering, you
        //   are more likely to build an array mapping sequential indices to visible tree nodes, since your
        //   filtering/search + clipping process will benefit from it. Having this will make this interpolation much easier.
        // - Consider this a prototype: we are working toward simplifying some of it.
        if (LinearUI::TreeNode("Multi-Select (trees)"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (trees)");
            HelpMarker(
                "This is rather advanced and experimental. If you are getting started with multi-select, "
                "please don't start by looking at how to use it for a tree!\n\n"
                "Future versions will try to simplify and formalize some of this.");

            struct ExampleTreeFuncs
            {
                static void DrawNode(ExampleTreeNode* node, LinearUISelectionBasicStorage* selection)
                {
                    LinearUITreeNodeFlags tree_node_flags = LinearUITreeNodeFlags_SpanAvailWidth | LinearUITreeNodeFlags_OpenOnArrow | LinearUITreeNodeFlags_OpenOnDoubleClick;
                    tree_node_flags |= LinearUITreeNodeFlags_NavLeftJumpsToParent; // Enable pressing left to jump to parent
                    if (node->Childs.Size == 0)
                        tree_node_flags |= LinearUITreeNodeFlags_Bullet | LinearUITreeNodeFlags_Leaf;
                    if (selection->Contains((LinearUIID)node->UID))
                        tree_node_flags |= LinearUITreeNodeFlags_Selected;

                    // Using SetNextItemStorageID() to specify storage id, so we can easily peek into
                    // the storage holding open/close stage, using our TreeNodeGetOpen/TreeNodeSetOpen() functions.
                    LinearUI::SetNextItemSelectionUserData((LinearUISelectionUserData)(intptr_t)node);
                    LinearUI::SetNextItemStorageID((LinearUIID)node->UID);
                    if (LinearUI::TreeNodeEx(node->Name, tree_node_flags))
                    {
                        for (ExampleTreeNode* child : node->Childs)
                            DrawNode(child, selection);
                        LinearUI::TreePop();
                    }
                    else if (LinearUI::IsItemToggledOpen())
                    {
                        TreeCloseAndUnselectChildNodes(node, selection);
                    }
                }

                // When closing a node: 1) close and unselect all child nodes, 2) select parent if any child was selected.
                // FIXME: This is currently handled by user logic but I'm hoping to eventually provide tree node
                // features to do this automatically, e.g. a LinearUITreeNodeFlags_AutoCloseChildNodes etc.
                static int TreeCloseAndUnselectChildNodes(ExampleTreeNode* node, LinearUISelectionBasicStorage* selection, int depth = 0)
                {
                    // Recursive close (the test for depth == 0 is because we call this on a node that was just closed!)
                    int unselected_count = selection->Contains((LinearUIID)node->UID) ? 1 : 0;
                    if (depth == 0 || LinearUI::TreeNodeGetOpen((LinearUIID)node->UID))
                    {
                        for (ExampleTreeNode* child : node->Childs)
                            unselected_count += TreeCloseAndUnselectChildNodes(child, selection, depth + 1);
                        LinearUI::TreeNodeSetOpen((LinearUIID)node->UID, false);
                    }

                    // Select root node if any of its child was selected, otherwise unselect
                    selection->SetItemSelected((LinearUIID)node->UID, (depth == 0 && unselected_count > 0));
                    return unselected_count;
                }

                // Apply multi-selection requests
                static void ApplySelectionRequests(LinearUIMultiSelectIO* ms_io, ExampleTreeNode* tree, LinearUISelectionBasicStorage* selection)
                {
                    for (LinearUISelectionRequest& req : ms_io->Requests)
                    {
                        if (req.Type == LinearUISelectionRequestType_SetAll)
                        {
                            if (req.Selected)
                                TreeSetAllInOpenNodes(tree, selection, req.Selected);
                            else
                                selection->Clear();
                        }
                        else if (req.Type == LinearUISelectionRequestType_SetRange)
                        {
                            ExampleTreeNode* first_node = (ExampleTreeNode*)(intptr_t)req.RangeFirstItem;
                            ExampleTreeNode* last_node = (ExampleTreeNode*)(intptr_t)req.RangeLastItem;
                            for (ExampleTreeNode* node = first_node; node != NULL; node = TreeGetNextNodeInVisibleOrder(node, last_node))
                                selection->SetItemSelected((LinearUIID)node->UID, req.Selected);
                        }
                    }
                }

                static void TreeSetAllInOpenNodes(ExampleTreeNode* node, LinearUISelectionBasicStorage* selection, bool selected)
                {
                    if (node->Parent != NULL) // Root node isn't visible nor selectable in our scheme
                        selection->SetItemSelected((LinearUIID)node->UID, selected);
                    if (node->Parent == NULL || LinearUI::TreeNodeGetOpen((LinearUIID)node->UID))
                        for (ExampleTreeNode* child : node->Childs)
                            TreeSetAllInOpenNodes(child, selection, selected);
                }

                // Interpolate in *user-visible order* AND only *over opened nodes*.
                // If you have a sequential mapping tables (e.g. generated after a filter/search pass) this would be simpler.
                // Here the tricks are that:
                // - we store/maintain ExampleTreeNode::IndexInParent which allows implementing a linear iterator easily, without searches, without recursion.
                //   this could be replaced by a search in parent, aka 'int index_in_parent = curr_node->Parent->Childs.find_index(curr_node)'
                //   which would only be called when crossing from child to a parent, aka not too much.
                // - we call SetNextItemStorageID() before our TreeNode() calls with an ID which doesn't relate to UI stack,
                //   making it easier to call TreeNodeGetOpen()/TreeNodeSetOpen() from any location.
                static ExampleTreeNode* TreeGetNextNodeInVisibleOrder(ExampleTreeNode* curr_node, ExampleTreeNode* last_node)
                {
                    // Reached last node
                    if (curr_node == last_node)
                        return NULL;

                    // Recurse into childs. Query storage to tell if the node is open.
                    if (curr_node->Childs.Size > 0 && LinearUI::TreeNodeGetOpen((LinearUIID)curr_node->UID))
                        return curr_node->Childs[0];

                    // Next sibling, then into our own parent
                    while (curr_node->Parent != NULL)
                    {
                        if (curr_node->IndexInParent + 1 < curr_node->Parent->Childs.Size)
                            return curr_node->Parent->Childs[curr_node->IndexInParent + 1];
                        curr_node = curr_node->Parent;
                    }
                    return NULL;
                }

            }; // ExampleTreeFuncs

            static LinearUISelectionBasicStorage selection;
            if (demo_data->DemoTree == NULL)
                demo_data->DemoTree = ExampleTree_CreateDemoTree(); // Create tree once
            LinearUI::Text("Selection size: %d", selection.Size);

            if (LinearUI::BeginChild("##Tree", LinearVec2(-FLT_MIN, LinearUI::GetFontSize() * 20), LinearUIChildFlags_FrameStyle | LinearUIChildFlags_ResizeY))
            {
                ExampleTreeNode* tree = demo_data->DemoTree;
                LinearUIMultiSelectFlags ms_flags = LinearUIMultiSelectFlags_ClearOnEscape | LinearUIMultiSelectFlags_BoxSelect2d;
                LinearUIMultiSelectIO* ms_io = LinearUI::BeginMultiSelect(ms_flags, selection.Size, -1);
                ExampleTreeFuncs::ApplySelectionRequests(ms_io, tree, &selection);
                for (ExampleTreeNode* node : tree->Childs)
                    ExampleTreeFuncs::DrawNode(node, &selection);
                ms_io = LinearUI::EndMultiSelect();
                ExampleTreeFuncs::ApplySelectionRequests(ms_io, tree, &selection);
            }
            LinearUI::EndChild();

            LinearUI::TreePop();
        }

        // Advanced demonstration of BeginMultiSelect()
        // - Showcase clipping.
        // - Showcase deletion.
        // - Showcase basic drag and drop.
        // - Showcase TreeNode variant (note that tree node don't expand in the demo: supporting expanding tree nodes + clipping a separate thing).
        // - Showcase using inside a table.
        //LinearUI::SetNextItemOpen(true, LinearUICond_Once);
        if (LinearUI::TreeNode("Multi-Select (advanced)"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (advanced)");
            // Options
            enum WidgetType { WidgetType_Selectable, WidgetType_TreeNode };
            static bool use_clipper = true;
            static bool use_deletion = true;
            static bool use_drag_drop = true;
            static bool show_in_table = false;
            static bool show_color_button = true;
            static LinearUIMultiSelectFlags flags = LinearUIMultiSelectFlags_ClearOnEscape | LinearUIMultiSelectFlags_BoxSelect1d;
            static WidgetType widget_type = WidgetType_Selectable;

            if (LinearUI::TreeNode("Options"))
            {
                if (LinearUI::RadioButton("Selectables", widget_type == WidgetType_Selectable)) { widget_type = WidgetType_Selectable; }
                LinearUI::SameLine();
                if (LinearUI::RadioButton("Tree nodes", widget_type == WidgetType_TreeNode)) { widget_type = WidgetType_TreeNode; }
                LinearUI::SameLine();
                HelpMarker("TreeNode() is technically supported but... using this correctly is more complicated (you need some sort of linear/random access to your tree, which is suited to advanced trees setups already implementing filters and clipper. We will work toward simplifying and demoing this.\n\nFor now the tree demo is actually a little bit meaningless because it is an empty tree with only root nodes.");
                LinearUI::Checkbox("Enable clipper", &use_clipper);
                LinearUI::Checkbox("Enable deletion", &use_deletion);
                LinearUI::Checkbox("Enable drag & drop", &use_drag_drop);
                LinearUI::Checkbox("Show in a table", &show_in_table);
                LinearUI::Checkbox("Show color button", &show_color_button);
                LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_SingleSelect", &flags, LinearUIMultiSelectFlags_SingleSelect);
                LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_NoSelectAll", &flags, LinearUIMultiSelectFlags_NoSelectAll);
                LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_NoRangeSelect", &flags, LinearUIMultiSelectFlags_NoRangeSelect);
                LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_NoAutoSelect", &flags, LinearUIMultiSelectFlags_NoAutoSelect);
                LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_NoAutoClear", &flags, LinearUIMultiSelectFlags_NoAutoClear);
                LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_NoAutoClearOnReselect", &flags, LinearUIMultiSelectFlags_NoAutoClearOnReselect);
                LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_NoSelectOnRightClick", &flags, LinearUIMultiSelectFlags_NoSelectOnRightClick);
                LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_BoxSelect1d", &flags, LinearUIMultiSelectFlags_BoxSelect1d);
                LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_BoxSelect2d", &flags, LinearUIMultiSelectFlags_BoxSelect2d);
                LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_BoxSelectNoScroll", &flags, LinearUIMultiSelectFlags_BoxSelectNoScroll);
                LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_ClearOnEscape", &flags, LinearUIMultiSelectFlags_ClearOnEscape);
                LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_ClearOnClickVoid", &flags, LinearUIMultiSelectFlags_ClearOnClickVoid);
                if (LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_ScopeWindow", &flags, LinearUIMultiSelectFlags_ScopeWindow) && (flags & LinearUIMultiSelectFlags_ScopeWindow))
                    flags &= ~LinearUIMultiSelectFlags_ScopeRect;
                if (LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_ScopeRect", &flags, LinearUIMultiSelectFlags_ScopeRect) && (flags & LinearUIMultiSelectFlags_ScopeRect))
                    flags &= ~LinearUIMultiSelectFlags_ScopeWindow;
                if (LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_SelectOnAuto", &flags, LinearUIMultiSelectFlags_SelectOnAuto))
                    flags &= ~(LinearUIMultiSelectFlags_SelectOnMask_ ^ LinearUIMultiSelectFlags_SelectOnAuto);
                LinearUI::SameLine(); HelpMarker("Apply selection on mouse down when clicking on unselected item, on mouse up when clicking on selected item. (Default)");
                if (LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_SelectOnClickAlways", &flags, LinearUIMultiSelectFlags_SelectOnClickAlways))
                    flags &= ~(LinearUIMultiSelectFlags_SelectOnMask_ ^ LinearUIMultiSelectFlags_SelectOnClickAlways);
                LinearUI::SameLine(); HelpMarker("Prevents Drag and Drop from being used on multi-selection, but allows e.g. BoxSelect to always reselect even when clicking inside an existing selection. (Excel style behavior)");
                if (LinearUI::CheckboxFlags("LinearUIMultiSelectFlags_SelectOnClickRelease", &flags, LinearUIMultiSelectFlags_SelectOnClickRelease))
                    flags &= ~(LinearUIMultiSelectFlags_SelectOnMask_ ^ LinearUIMultiSelectFlags_SelectOnClickRelease);
                LinearUI::SameLine(); HelpMarker("Allow dragging an unselected item without altering selection.");
                LinearUI::TreePop();
            }

            // Initialize default list with 1000 items.
            // Use default selection.Adapter: Pass index to SetNextItemSelectionUserData(), store index in Selection
            static LinearVector<int> items;
            static int items_next_id = 0;
            if (items_next_id == 0) { for (int n = 0; n < 1000; n++) { items.push_back(items_next_id++); } }
            static ExampleSelectionWithDeletion selection;
            static bool request_deletion_from_menu = false; // Queue deletion triggered from context menu

            LinearUI::Text("Selection size: %d/%d", selection.Size, items.Size);

            const float items_height = (widget_type == WidgetType_TreeNode) ? LinearUI::GetTextLineHeight() : LinearUI::GetTextLineHeightWithSpacing();
            LinearUI::SetNextWindowContentSize(LinearVec2(0.0f, items.Size * items_height));
            if (LinearUI::BeginChild("##Basket", LinearVec2(-FLT_MIN, LinearUI::GetFontSize() * 20), LinearUIChildFlags_FrameStyle | LinearUIChildFlags_ResizeY))
            {
                LinearVec2 color_button_sz(LinearUI::GetFontSize(), LinearUI::GetFontSize());
                if (widget_type == WidgetType_TreeNode)
                    LinearUI::PushStyleVarY(LinearUIStyleVar_ItemSpacing, 0.0f);

                LinearUIMultiSelectIO* ms_io = LinearUI::BeginMultiSelect(flags, selection.Size, items.Size);
                selection.ApplyRequests(ms_io);

                const bool want_delete = (LinearUI::Shortcut(LinearUIKey_Delete, LinearUIInputFlags_Repeat) && (selection.Size > 0)) || request_deletion_from_menu;
                const int item_curr_idx_to_focus = want_delete ? selection.ApplyDeletionPreLoop(ms_io, items.Size) : -1;
                request_deletion_from_menu = false;

                if (show_in_table)
                {
                    if (widget_type == WidgetType_TreeNode)
                        LinearUI::PushStyleVar(LinearUIStyleVar_CellPadding, LinearVec2(0.0f, 0.0f));
                    LinearUI::BeginTable("##Split", 2, LinearUITableFlags_Resizable | LinearUITableFlags_NoSavedSettings | LinearUITableFlags_NoPadOuterX);
                    LinearUI::TableSetupColumn("", LinearUITableColumnFlags_WidthStretch, 0.70f);
                    LinearUI::TableSetupColumn("", LinearUITableColumnFlags_WidthStretch, 0.30f);
                    //LinearUI::PushStyleVarY(LinearUIStyleVar_ItemSpacing, 0.0f);
                }

                LinearUIListClipper clipper;
                if (use_clipper)
                {
                    clipper.Begin(items.Size);
                    if (item_curr_idx_to_focus != -1)
                        clipper.IncludeItemByIndex(item_curr_idx_to_focus); // Ensure focused item is not clipped.
                    if (ms_io->RangeSrcItem != -1)
                        clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem); // Ensure RangeSrc item is not clipped.
                }

                while (!use_clipper || clipper.Step())
                {
                    const int item_begin = use_clipper ? clipper.DisplayStart : 0;
                    const int item_end = use_clipper ? clipper.DisplayEnd : items.Size;
                    for (int n = item_begin; n < item_end; n++)
                    {
                        if (show_in_table)
                            LinearUI::TableNextColumn();

                        const int item_id = items[n];
                        const char* item_category = ExampleNames[item_id % LINEARUI_COUNTOF(ExampleNames)];
                        char label[64];
                        sprintf(label, "Object %05d: %s", item_id, item_category);

                        // IMPORTANT: for deletion refocus to work we need object ID to be stable,
                        // aka not depend on their index in the list. Here we use our persistent item_id
                        // instead of index to build a unique ID that will persist.
                        // (If we used PushID(index) instead, focus wouldn't be restored correctly after deletion).
                        LinearUI::PushID(item_id);

                        // Emit a color button, to test that Shift+LeftArrow landing on an item that is not part
                        // of the selection scope doesn't erroneously alter our selection.
                        if (show_color_button)
                        {
                            LinearU32 dummy_col = (LinearU32)((unsigned int)n * 0xC250B74B) | LINEARUI_COL32_A_MASK;
                            LinearUI::ColorButton("##", LinearColor(dummy_col), LinearUIColorEditFlags_NoTooltip, color_button_sz);
                            LinearUI::SameLine();
                        }

                        // Submit item
                        bool item_is_selected = selection.Contains((LinearUIID)n);
                        bool item_is_open = false;
                        LinearUI::SetNextItemSelectionUserData(n);
                        if (widget_type == WidgetType_Selectable)
                        {
                            LinearUI::Selectable(label, item_is_selected, LinearUISelectableFlags_None);
                        }
                        else if (widget_type == WidgetType_TreeNode)
                        {
                            LinearUITreeNodeFlags tree_node_flags = LinearUITreeNodeFlags_SpanAvailWidth | LinearUITreeNodeFlags_OpenOnArrow | LinearUITreeNodeFlags_OpenOnDoubleClick;
                            if (item_is_selected)
                                tree_node_flags |= LinearUITreeNodeFlags_Selected;
                            item_is_open = LinearUI::TreeNodeEx(label, tree_node_flags);
                        }

                        // Focus (for after deletion)
                        if (item_curr_idx_to_focus == n)
                            LinearUI::SetKeyboardFocusHere(-1);

                        // Drag and Drop
                        if (use_drag_drop && LinearUI::BeginDragDropSource())
                        {
                            // Create payload with full selection OR single unselected item.
                            // (the later is only possible when using LinearUIMultiSelectFlags_SelectOnClickRelease)
                            if (LinearUI::GetDragDropPayload() == NULL)
                            {
                                LinearVector<int> payload_items;
                                void* it = NULL;
                                LinearUIID id = 0;
                                if (!item_is_selected)
                                    payload_items.push_back(item_id);
                                else
                                    while (selection.GetNextSelectedItem(&it, &id))
                                        payload_items.push_back((int)id);
                                LinearUI::SetDragDropPayload("MULTISELECT_DEMO_ITEMS", payload_items.Data, (size_t)payload_items.size_in_bytes());
                            }

                            // Display payload content in tooltip
                            const LinearUIPayload* payload = LinearUI::GetDragDropPayload();
                            const int* payload_items = (int*)payload->Data;
                            const int payload_count = (int)payload->DataSize / (int)sizeof(int);
                            if (payload_count == 1)
                                LinearUI::Text("Object %05d: %s", payload_items[0], ExampleNames[payload_items[0] % LINEARUI_COUNTOF(ExampleNames)]);
                            else
                                LinearUI::Text("Dragging %d objects", payload_count);

                            LinearUI::EndDragDropSource();
                        }

                        if (widget_type == WidgetType_TreeNode && item_is_open)
                            LinearUI::TreePop();

                        // Right-click: context menu
                        if (LinearUI::BeginPopupContextItem())
                        {
                            LinearUI::BeginDisabled(!use_deletion || selection.Size == 0);
                            sprintf(label, "Delete %d item(s)###DeleteSelected", selection.Size);
                            if (LinearUI::Selectable(label))
                                request_deletion_from_menu = true;
                            LinearUI::EndDisabled();
                            LinearUI::Selectable("Close");
                            LinearUI::EndPopup();
                        }

                        // Demo content within a table
                        if (show_in_table)
                        {
                            LinearUI::TableNextColumn();
                            LinearUI::SetNextItemWidth(-FLT_MIN);
                            LinearUI::PushStyleVar(LinearUIStyleVar_FramePadding, LinearVec2(0, 0));
                            LinearUI::InputText("##NoLabel", (char*)(void*)item_category, strlen(item_category), LinearUIInputTextFlags_ReadOnly);
                            LinearUI::PopStyleVar();
                        }

                        LinearUI::PopID();
                    }
                    if (!use_clipper)
                        break;
                }

                if (show_in_table)
                {
                    LinearUI::EndTable();
                    if (widget_type == WidgetType_TreeNode)
                        LinearUI::PopStyleVar();
                }

                // Apply multi-select requests
                ms_io = LinearUI::EndMultiSelect();
                selection.ApplyRequests(ms_io);
                if (want_delete)
                    selection.ApplyDeletionPostLoop(ms_io, items, item_curr_idx_to_focus);

                if (widget_type == WidgetType_TreeNode)
                    LinearUI::PopStyleVar();
            }
            LinearUI::EndChild();
            LinearUI::TreePop();
        }
        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTabs()
//-----------------------------------------------------------------------------

static void EditTabBarFittingPolicyFlags(LinearUITabBarFlags* p_flags)
{
    if ((*p_flags & LinearUITabBarFlags_FittingPolicyMask_) == 0)
        *p_flags |= LinearUITabBarFlags_FittingPolicyDefault_;
    if (LinearUI::CheckboxFlags("LinearUITabBarFlags_FittingPolicyMixed", p_flags, LinearUITabBarFlags_FittingPolicyMixed))
        *p_flags &= ~(LinearUITabBarFlags_FittingPolicyMask_ ^ LinearUITabBarFlags_FittingPolicyMixed);
    if (LinearUI::CheckboxFlags("LinearUITabBarFlags_FittingPolicyShrink", p_flags, LinearUITabBarFlags_FittingPolicyShrink))
        *p_flags &= ~(LinearUITabBarFlags_FittingPolicyMask_ ^ LinearUITabBarFlags_FittingPolicyShrink);
    if (LinearUI::CheckboxFlags("LinearUITabBarFlags_FittingPolicyScroll", p_flags, LinearUITabBarFlags_FittingPolicyScroll))
        *p_flags &= ~(LinearUITabBarFlags_FittingPolicyMask_ ^ LinearUITabBarFlags_FittingPolicyScroll);
}

static void DemoWindowWidgetsTabs()
{
    if (LinearUI::TreeNode("Tabs"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Tabs");
        if (LinearUI::TreeNode("Basic"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Tabs/Basic");
            LinearUITabBarFlags tab_bar_flags = LinearUITabBarFlags_None;
            if (LinearUI::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                if (LinearUI::BeginTabItem("Avocado"))
                {
                    LinearUI::Text("This is the Avocado tab!\nblah blah blah blah blah");
                    LinearUI::EndTabItem();
                }
                if (LinearUI::BeginTabItem("Broccoli"))
                {
                    LinearUI::Text("This is the Broccoli tab!\nblah blah blah blah blah");
                    LinearUI::EndTabItem();
                }
                if (LinearUI::BeginTabItem("Cucumber"))
                {
                    LinearUI::Text("This is the Cucumber tab!\nblah blah blah blah blah");
                    LinearUI::EndTabItem();
                }
                LinearUI::EndTabBar();
            }
            LinearUI::Separator();
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Advanced & Close Button"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Tabs/Advanced & Close Button");
            // Expose a couple of the available flags. In most cases you may just call BeginTabBar() with no flags (0).
            static LinearUITabBarFlags tab_bar_flags = LinearUITabBarFlags_Reorderable;
            LinearUI::CheckboxFlags("LinearUITabBarFlags_Reorderable", &tab_bar_flags, LinearUITabBarFlags_Reorderable);
            LinearUI::CheckboxFlags("LinearUITabBarFlags_AutoSelectNewTabs", &tab_bar_flags, LinearUITabBarFlags_AutoSelectNewTabs);
            LinearUI::CheckboxFlags("LinearUITabBarFlags_TabListPopupButton", &tab_bar_flags, LinearUITabBarFlags_TabListPopupButton);
            LinearUI::CheckboxFlags("LinearUITabBarFlags_NoCloseWithMiddleMouseButton", &tab_bar_flags, LinearUITabBarFlags_NoCloseWithMiddleMouseButton);
            LinearUI::CheckboxFlags("LinearUITabBarFlags_DrawSelectedOverline", &tab_bar_flags, LinearUITabBarFlags_DrawSelectedOverline);
            EditTabBarFittingPolicyFlags(&tab_bar_flags);

            // Tab Bar
            LinearUI::AlignTextToFramePadding();
            LinearUI::Text("Opened:");
            const char* names[4] = { "Artichoke", "Beetroot", "Celery", "Daikon" };
            static bool opened[4] = { true, true, true, true }; // Persistent user state
            for (int n = 0; n < LINEARUI_COUNTOF(opened); n++)
            {
                LinearUI::SameLine();
                LinearUI::Checkbox(names[n], &opened[n]);
            }

            // Passing a bool* to BeginTabItem() is similar to passing one to Begin():
            // the underlying bool will be set to false when the tab is closed.
            if (LinearUI::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                for (int n = 0; n < LINEARUI_COUNTOF(opened); n++)
                    if (opened[n] && LinearUI::BeginTabItem(names[n], &opened[n], LinearUITabItemFlags_None))
                    {
                        LinearUI::Text("This is the %s tab!", names[n]);
                        if (n & 1)
                            LinearUI::Text("I am an odd tab.");
                        LinearUI::EndTabItem();
                    }
                LinearUI::EndTabBar();
            }
            LinearUI::Separator();
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("TabItemButton & Leading/Trailing flags"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Tabs/TabItemButton & Leading-Trailing flags");
            static LinearVector<int> active_tabs;
            static int next_tab_id = 0;
            if (next_tab_id == 0) // Initialize with some default tabs
                for (int i = 0; i < 3; i++)
                    active_tabs.push_back(next_tab_id++);

            // TabItemButton() and Leading/Trailing flags are distinct features which we will demo together.
            // (It is possible to submit regular tabs with Leading/Trailing flags, or TabItemButton tabs without Leading/Trailing flags...
            // but they tend to make more sense together)
            static bool show_leading_button = true;
            static bool show_trailing_button = true;
            LinearUI::Checkbox("Show Leading TabItemButton()", &show_leading_button);
            LinearUI::Checkbox("Show Trailing TabItemButton()", &show_trailing_button);

            // Expose some other flags which are useful to showcase how they interact with Leading/Trailing tabs
            static LinearUITabBarFlags tab_bar_flags = LinearUITabBarFlags_AutoSelectNewTabs | LinearUITabBarFlags_Reorderable | LinearUITabBarFlags_FittingPolicyShrink;
            EditTabBarFittingPolicyFlags(&tab_bar_flags);

            if (LinearUI::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                // Demo a Leading TabItemButton(): click the "?" button to open a menu
                if (show_leading_button)
                    if (LinearUI::TabItemButton("?", LinearUITabItemFlags_Leading | LinearUITabItemFlags_NoTooltip))
                        LinearUI::OpenPopup("MyHelpMenu");
                if (LinearUI::BeginPopup("MyHelpMenu"))
                {
                    LinearUI::Selectable("Hello!");
                    LinearUI::EndPopup();
                }

                // Demo Trailing Tabs: click the "+" button to add a new tab.
                // (In your app you may want to use a font icon instead of the "+")
                // We submit it before the regular tabs, but thanks to the LinearUITabItemFlags_Trailing flag it will always appear at the end.
                if (show_trailing_button)
                    if (LinearUI::TabItemButton("+", LinearUITabItemFlags_Trailing | LinearUITabItemFlags_NoTooltip))
                        active_tabs.push_back(next_tab_id++); // Add new tab

                // Submit our regular tabs
                for (int n = 0; n < active_tabs.Size; )
                {
                    bool open = true;
                    char name[16];
                    snprintf(name, LINEARUI_COUNTOF(name), "%04d", active_tabs[n]);
                    if (LinearUI::BeginTabItem(name, &open, LinearUITabItemFlags_None))
                    {
                        LinearUI::Text("This is the %s tab!", name);
                        LinearUI::EndTabItem();
                    }

                    if (!open)
                        active_tabs.erase(active_tabs.Data + n);
                    else
                        n++;
                }

                LinearUI::EndTabBar();
            }
            LinearUI::Separator();
            LinearUI::TreePop();
        }
        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsText()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsText()
{
    if (LinearUI::TreeNode("Text"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Text");
        if (LinearUI::TreeNode("Colorful Text"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Text/Colored Text");
            // Using shortcut. You can use PushStyleColor()/PopStyleColor() for more flexibility.
            LinearUI::TextColored(LinearVec4(1.0f, 0.0f, 1.0f, 1.0f), "Pink");
            LinearUI::TextColored(LinearVec4(1.0f, 1.0f, 0.0f, 1.0f), "Yellow");
            LinearUI::TextDisabled("Disabled");
            LinearUI::SameLine(); HelpMarker("The TextDisabled color is stored in LinearUIStyle.");
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Font Size"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Text/Font Size");
            LinearUIStyle& style = LinearUI::GetStyle();
            const float global_scale = style.FontScaleMain * style.FontScaleDpi;
            LinearUI::Text("style.FontScaleMain = %0.2f", style.FontScaleMain);
            LinearUI::Text("style.FontScaleDpi = %0.2f", style.FontScaleDpi);
            LinearUI::Text("global_scale = ~%0.2f", global_scale); // This is not technically accurate as internal scales may apply, but conceptually let's pretend it is.
            LinearUI::Text("FontSize = %0.2f", LinearUI::GetFontSize());

            LinearUI::SeparatorText("");
            static float custom_size = 16.0f;
            LinearUI::SliderFloat("custom_size", &custom_size, 10.0f, 100.0f, "%.0f");
            LinearUI::Text("LinearUI::PushFont(nullptr, custom_size);");
            LinearUI::PushFont(NULL, custom_size);
            LinearUI::Text("FontSize = %.2f (== %.2f * global_scale)", LinearUI::GetFontSize(), custom_size);
            LinearUI::PopFont();

            LinearUI::SeparatorText("");
            static float custom_scale = 1.0f;
            LinearUI::SliderFloat("custom_scale", &custom_scale, 0.5f, 4.0f, "%.2f");
            LinearUI::Text("LinearUI::PushFont(nullptr, style.FontSizeBase * custom_scale);");
            LinearUI::PushFont(NULL, style.FontSizeBase * custom_scale);
            LinearUI::Text("FontSize = %.2f (== style.FontSizeBase * %.2f * global_scale)", LinearUI::GetFontSize(), custom_scale);
            LinearUI::PopFont();

            LinearUI::SeparatorText("");
            for (float scaling = 0.5f; scaling <= 4.0f; scaling += 0.5f)
            {
                LinearUI::PushFont(NULL, style.FontSizeBase * scaling);
                LinearUI::Text("FontSize = %.2f (== style.FontSizeBase * %.2f * global_scale)", LinearUI::GetFontSize(), scaling);
                LinearUI::PopFont();
            }

            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Word Wrapping"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Text/Word Wrapping");
            // Using shortcut. You can use PushTextWrapPos()/PopTextWrapPos() for more flexibility.
            LinearUI::TextWrapped(
                "This text should automatically wrap on the edge of the window. The current implementation "
                "for text wrapping follows simple rules suitable for English and possibly other languages.");
            LinearUI::Spacing();

            static float wrap_width = 200.0f;
            LinearUI::SliderFloat("Wrap width", &wrap_width, -20, 600, "%.0f");

            LinearDrawList* draw_list = LinearUI::GetWindowDrawList();
            for (int n = 0; n < 2; n++)
            {
                LinearUI::Text("Test paragraph %d:", n);
                LinearVec2 pos = LinearUI::GetCursorScreenPos();
                LinearVec2 marker_min = LinearVec2(pos.x + wrap_width, pos.y);
                LinearVec2 marker_max = LinearVec2(pos.x + wrap_width + 10, pos.y + LinearUI::GetTextLineHeight());
                LinearUI::PushTextWrapPos(LinearUI::GetCursorPos().x + wrap_width);
                if (n == 0)
                    LinearUI::Text("The lazy dog is a good dog. This paragraph should fit within %.0f pixels. Testing a 1 character word. The quick brown fox jumps over the lazy dog.", wrap_width);
                else
                    LinearUI::Text("aaaaaaaa bbbbbbbb, c cccccccc,dddddddd. d eeeeeeee   ffffffff. gggggggg!hhhhhhhh");

                // Draw actual text bounding box, following by marker of our expected limit (should not overlap!)
                draw_list->AddRect(LinearUI::GetItemRectMin(), LinearUI::GetItemRectMax(), LINEARUI_COL32(255, 255, 0, 255));
                draw_list->AddRectFilled(marker_min, marker_max, LINEARUI_COL32(255, 0, 255, 255));
                LinearUI::PopTextWrapPos();
            }

            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("UTF-8 Text"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Text/UTF-8 Text");
            // UTF-8 test with Japanese characters
            // (Needs a suitable font? Try "Google Noto" or "Arial Unicode". See docs/FONTS.md for details.)
            // - From C++11 you can use the u8"my text" syntax to encode literal strings as UTF-8
            // - For earlier compiler, you may be able to encode your sources as UTF-8 (e.g. in Visual Studio, you
            //   can save your source files as 'UTF-8 without signature').
            // - FOR THIS DEMO FILE ONLY, BECAUSE WE WANT TO SUPPORT OLD COMPILERS, WE ARE *NOT* INCLUDING RAW UTF-8
            //   CHARACTERS IN THIS SOURCE FILE. Instead we are encoding a few strings with hexadecimal constants.
            //   Don't do this in your application! Please use u8"text in any language" in your application!
            // Note that characters values are preserved even by InputText() if the font cannot be displayed,
            // so you can safely copy & paste garbled characters into another application.
            LinearUI::TextWrapped(
                "CJK text will only appear if the font was loaded with the appropriate CJK character ranges. "
                "Call io.Fonts->AddFontFromFileTTF() manually to load extra character ranges. "
                "Read docs/FONTS.md for details.");
            LinearUI::Text("Hiragana: \xe3\x81\x8b\xe3\x81\x8d\xe3\x81\x8f\xe3\x81\x91\xe3\x81\x93 (kakikukeko)");
            LinearUI::Text("Kanjis: \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e (nihongo)");
            static char buf[32] = "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e";
            //static char buf[32] = u8"NIHONGO"; // <- this is how you would write it with C++11, using real kanjis
            LinearUI::InputText("UTF-8 input", buf, LINEARUI_COUNTOF(buf));
            LinearUI::TreePop();
        }
        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTextFilter()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsTextFilter()
{
    if (LinearUI::TreeNode("Text Filter"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Text Filter");
        // Helper class to easy setup a text filter.
        // You may want to implement a more feature-full filtering scheme in your own application.
        HelpMarker("Not a widget per-se, but LinearUITextFilter is a helper to perform simple filtering on text strings.");
        static LinearUITextFilter filter;
        LinearUI::Text("Filter usage:\n"
            "  \"\"         display all lines\n"
            "  \"xxx\"      display lines containing \"xxx\"\n"
            "  \"xxx,yyy\"  display lines containing \"xxx\" or \"yyy\"\n"
            "  \"-xxx\"     hide lines containing \"xxx\"");
        filter.Draw();
        const char* lines[] = { "aaa1.c", "bbb1.c", "ccc1.c", "aaa2.cpp", "bbb2.cpp", "ccc2.cpp", "abc.h", "hello, world" };
        for (int i = 0; i < LINEARUI_COUNTOF(lines); i++)
            if (filter.PassFilter(lines[i]))
                LinearUI::BulletText("%s", lines[i]);
        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTextInput()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsTextInput()
{
    // To wire InputText() with std::string or any other custom string type,
    // see the "Text Input > Resize Callback" section of this demo, and the misc/cpp/linearui_stdlib.h file.
    if (LinearUI::TreeNode("Text Input"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Text Input");
        if (LinearUI::TreeNode("Multi-line Text Input"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Text Input/Multi-line Text Input");
            // WE ARE USING A FIXED-SIZE BUFFER FOR SIMPLICITY HERE.
            // If you want to use InputText() with std::string or any custom dynamic string type:
            // - For std::string: use the wrapper in misc/cpp/linearui_stdlib.h/.cpp
            // - Otherwise, see the 'Dear LinearUI Demo->Widgets->Text Input->Resize Callback' for using LinearUIInputTextFlags_CallbackResize.
            static char text[1024 * 16] =
                "/*\n"
                " The Pentium F00F bug, shorthand for F0 0F C7 C8,\n"
                " the hexadecimal encoding of one offending instruction,\n"
                " more formally, the invalid operand with locked CMPXCHG8B\n"
                " instruction bug, is a design flaw in the majority of\n"
                " Intel Pentium, Pentium MMX, and Pentium OverDrive\n"
                " processors (all in the P5 microarchitecture).\n"
                "*/\n\n"
                "label:\n"
                "\tlock cmpxchg8b eax\n";

            static LinearUIInputTextFlags flags = LinearUIInputTextFlags_AllowTabInput;
            HelpMarker("You can use the LinearUIInputTextFlags_CallbackResize facility if you need to wire InputTextMultiline() to a dynamic string type. See misc/cpp/linearui_stdlib.h for an example. (This is not demonstrated in linearui_demo.cpp because we don't want to include <string> in here)");
            LinearUI::CheckboxFlags("LinearUIInputTextFlags_ReadOnly", &flags, LinearUIInputTextFlags_ReadOnly);
            LinearUI::CheckboxFlags("LinearUIInputTextFlags_WordWrap", &flags, LinearUIInputTextFlags_WordWrap);
            LinearUI::SameLine(); HelpMarker("Feature is currently in Beta. Please read comments in linearui.h");
            LinearUI::CheckboxFlags("LinearUIInputTextFlags_AllowTabInput", &flags, LinearUIInputTextFlags_AllowTabInput);
            LinearUI::SameLine(); HelpMarker("When _AllowTabInput is set, passing through the widget with Tabbing doesn't automatically activate it, in order to also cycling through subsequent widgets.");
            LinearUI::CheckboxFlags("LinearUIInputTextFlags_CtrlEnterForNewLine", &flags, LinearUIInputTextFlags_CtrlEnterForNewLine);
            LinearUI::InputTextMultiline("##source", text, LINEARUI_COUNTOF(text), LinearVec2(-FLT_MIN, LinearUI::GetTextLineHeight() * 16), flags);
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Filtered Text Input"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Text Input/Filtered Text Input");
            struct TextFilters
            {
                // Modify character input by altering 'data->Eventchar' (LinearUIInputTextFlags_CallbackCharFilter callback)
                static int FilterCasingSwap(LinearUIInputTextCallbackData* data)
                {
                    if (data->EventChar >= 'a' && data->EventChar <= 'z') { data->EventChar -= 'a' - 'A'; } // Lowercase becomes uppercase
                    else if (data->EventChar >= 'A' && data->EventChar <= 'Z') { data->EventChar += 'a' - 'A'; } // Uppercase becomes lowercase
                    return 0;
                }

                // Return 0 (pass) if the character is 'i' or 'm' or 'g' or 'u' or 'i', otherwise return 1 (filter out)
                static int FilterLinearUILetters(LinearUIInputTextCallbackData* data)
                {
                    if (data->EventChar < 256 && strchr("linearui", (char)data->EventChar))
                        return 0;
                    return 1;
                }
            };

            static char buf1[32] = ""; LinearUI::InputText("default", buf1, LINEARUI_COUNTOF(buf1));
            static char buf2[32] = ""; LinearUI::InputText("decimal", buf2, LINEARUI_COUNTOF(buf2), LinearUIInputTextFlags_CharsDecimal);
            static char buf3[32] = ""; LinearUI::InputText("hexadecimal", buf3, LINEARUI_COUNTOF(buf3), LinearUIInputTextFlags_CharsHexadecimal | LinearUIInputTextFlags_CharsUppercase);
            static char buf4[32] = ""; LinearUI::InputText("uppercase", buf4, LINEARUI_COUNTOF(buf4), LinearUIInputTextFlags_CharsUppercase);
            static char buf5[32] = ""; LinearUI::InputText("no blank", buf5, LINEARUI_COUNTOF(buf5), LinearUIInputTextFlags_CharsNoBlank);
            static char buf6[32] = ""; LinearUI::InputText("casing swap", buf6, LINEARUI_COUNTOF(buf6), LinearUIInputTextFlags_CallbackCharFilter, TextFilters::FilterCasingSwap); // Use CharFilter callback to replace characters.
            static char buf7[32] = ""; LinearUI::InputText("\"linearui\"", buf7, LINEARUI_COUNTOF(buf7), LinearUIInputTextFlags_CallbackCharFilter, TextFilters::FilterLinearUILetters); // Use CharFilter callback to disable some characters.
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Password Input"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Text Input/Password input");
            static char password[64] = "password123";
            LinearUI::InputText("password", password, LINEARUI_COUNTOF(password), LinearUIInputTextFlags_Password);
            LinearUI::SameLine(); HelpMarker("Display all characters as '*'.\nDisable clipboard cut and copy.\nDisable logging.\n");
            LinearUI::InputTextWithHint("password (w/ hint)", "<password>", password, LINEARUI_COUNTOF(password), LinearUIInputTextFlags_Password);
            LinearUI::InputText("password (clear)", password, LINEARUI_COUNTOF(password));
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Completion, History, Edit Callbacks"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Text Input/Completion, History, Edit Callbacks");
            struct Funcs
            {
                static int MyCallback(LinearUIInputTextCallbackData* data)
                {
                    if (data->EventFlag == LinearUIInputTextFlags_CallbackCompletion)
                    {
                        data->InsertChars(data->CursorPos, "..");
                    }
                    else if (data->EventFlag == LinearUIInputTextFlags_CallbackHistory)
                    {
                        if (data->EventKey == LinearUIKey_UpArrow)
                        {
                            data->DeleteChars(0, data->BufTextLen);
                            data->InsertChars(0, "Pressed Up!");
                            data->SelectAll();
                        }
                        else if (data->EventKey == LinearUIKey_DownArrow)
                        {
                            data->DeleteChars(0, data->BufTextLen);
                            data->InsertChars(0, "Pressed Down!");
                            data->SelectAll();
                        }
                    }
                    else if (data->EventFlag == LinearUIInputTextFlags_CallbackEdit)
                    {
                        // Toggle casing of first character
                        char c = data->Buf[0];
                        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) data->Buf[0] ^= 32;
                        data->BufDirty = true;

                        // Increment a counter
                        int* p_int = (int*)data->UserData;
                        *p_int = *p_int + 1;
                    }
                    return 0;
                }
            };
            static char buf1[64];
            LinearUI::InputText("Completion", buf1, LINEARUI_COUNTOF(buf1), LinearUIInputTextFlags_CallbackCompletion, Funcs::MyCallback);
            LinearUI::SameLine(); HelpMarker(
                "Here we append \"..\" each time Tab is pressed. "
                "See 'Examples>Console' for a more meaningful demonstration of using this callback.");

            static char buf2[64];
            LinearUI::InputText("History", buf2, LINEARUI_COUNTOF(buf2), LinearUIInputTextFlags_CallbackHistory, Funcs::MyCallback);
            LinearUI::SameLine(); HelpMarker(
                "Here we replace and select text each time Up/Down are pressed. "
                "See 'Examples>Console' for a more meaningful demonstration of using this callback.");

            static char buf3[64];
            static int edit_count = 0;
            LinearUI::InputText("Edit", buf3, LINEARUI_COUNTOF(buf3), LinearUIInputTextFlags_CallbackEdit, Funcs::MyCallback, (void*)&edit_count);
            LinearUI::SameLine(); HelpMarker(
                "Here we toggle the casing of the first character on every edit + count edits.");
            LinearUI::SameLine(); LinearUI::Text("(%d)", edit_count);

            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Resize Callback"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Text Input/Resize Callback");
            // To wire InputText() with std::string or any other custom string type,
            // you can use the LinearUIInputTextFlags_CallbackResize flag + create a custom LinearUI::InputText() wrapper
            // using your preferred type. See misc/cpp/linearui_stdlib.h for an implementation of this using std::string.
            HelpMarker(
                "Using LinearUIInputTextFlags_CallbackResize to wire your custom string type to InputText().\n\n"
                "See misc/cpp/linearui_stdlib.h for an implementation of this for std::string.");
            struct Funcs
            {
                static int MyResizeCallback(LinearUIInputTextCallbackData* data)
                {
                    if (data->EventFlag == LinearUIInputTextFlags_CallbackResize)
                    {
                        LinearVector<char>* my_str = (LinearVector<char>*)data->UserData;
                        LINEARUI_ASSERT(my_str->begin() == data->Buf);
                        my_str->resize(data->BufSize); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
                        data->Buf = my_str->begin();
                    }
                    return 0;
                }

                // Note: Because LinearUI:: is a namespace you would typically add your own function into the namespace.
                // For example, you code may declare a function 'LinearUI::InputText(const char* label, MyString* my_str)'
                static bool MyInputTextMultiline(const char* label, LinearVector<char>* my_str, const LinearVec2& size = LinearVec2(0, 0), LinearUIInputTextFlags flags = 0)
                {
                    LINEARUI_ASSERT((flags & LinearUIInputTextFlags_CallbackResize) == 0);
                    return LinearUI::InputTextMultiline(label, my_str->begin(), (size_t)my_str->size(), size, flags | LinearUIInputTextFlags_CallbackResize, Funcs::MyResizeCallback, (void*)my_str);
                }
            };

            // For this demo we are using LinearVector as a string container.
            // Note that because we need to store a terminating zero character, our size/capacity are 1 more
            // than usually reported by a typical string class.
            static LinearUIInputTextFlags flags = LinearUIInputTextFlags_None;
            LinearUI::CheckboxFlags("LinearUIInputTextFlags_WordWrap", &flags, LinearUIInputTextFlags_WordWrap);

            static LinearVector<char> my_str;
            if (my_str.empty())
                my_str.push_back(0);
            Funcs::MyInputTextMultiline("##MyStr", &my_str, LinearVec2(-FLT_MIN, LinearUI::GetTextLineHeight() * 16), flags);
            LinearUI::Text("Data: %p\nSize: %d\nCapacity: %d", (void*)my_str.begin(), my_str.size(), my_str.capacity());
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Eliding, Alignment"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Text Input/Eliding, Alignment");
            static char buf1[128] = "/path/to/some/folder/with/long/filename.cpp";
            static LinearUIInputTextFlags flags = LinearUIInputTextFlags_ElideLeft;
            LinearUI::CheckboxFlags("LinearUIInputTextFlags_ElideLeft", &flags, LinearUIInputTextFlags_ElideLeft);
            LinearUI::InputText("Path", buf1, LINEARUI_COUNTOF(buf1), flags);
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Miscellaneous"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Text Input/Miscellaneous");
            static char buf1[16];
            static LinearUIInputTextFlags flags = LinearUIInputTextFlags_EscapeClearsAll;
            LinearUI::CheckboxFlags("LinearUIInputTextFlags_EscapeClearsAll", &flags, LinearUIInputTextFlags_EscapeClearsAll);
            LinearUI::CheckboxFlags("LinearUIInputTextFlags_ReadOnly", &flags, LinearUIInputTextFlags_ReadOnly);
            LinearUI::CheckboxFlags("LinearUIInputTextFlags_NoUndoRedo", &flags, LinearUIInputTextFlags_NoUndoRedo);
            LinearUI::InputText("Hello", buf1, LINEARUI_COUNTOF(buf1), flags);
            LinearUI::TreePop();
        }

        LinearUI::TreePop();
    }

}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTooltips()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsTooltips()
{
    if (LinearUI::TreeNode("Tooltips"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Tooltips");
        // Tooltips are windows following the mouse. They do not take focus away.
        LinearUI::SeparatorText("General");

        // Typical use cases:
        // - Short-form (text only):      SetItemTooltip("Hello");
        // - Short-form (any contents):   if (BeginItemTooltip()) { Text("Hello"); EndTooltip(); }

        // - Full-form (text only):       if (IsItemHovered(...)) { SetTooltip("Hello"); }
        // - Full-form (any contents):    if (IsItemHovered(...) && BeginTooltip()) { Text("Hello"); EndTooltip(); }

        HelpMarker(
            "Tooltip are typically created by using a IsItemHovered() + SetTooltip() sequence.\n\n"
            "We provide a helper SetItemTooltip() function to perform the two with standards flags.");

        LinearVec2 sz = LinearVec2(-FLT_MIN, 0.0f);

        LinearUI::Button("Basic", sz);
        LinearUI::SetItemTooltip("I am a tooltip");

        LinearUI::Button("Fancy", sz);
        if (LinearUI::BeginItemTooltip())
        {
            LinearUI::Text("I am a fancy tooltip");
            static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
            LinearUI::PlotLines("Curve", arr, LINEARUI_COUNTOF(arr));
            LinearUI::Text("Sin(time) = %f", sinf((float)LinearUI::GetTime()));
            LinearUI::EndTooltip();
        }

        LinearUI::SeparatorText("Always On");

        // Showcase NOT relying on a IsItemHovered() to emit a tooltip.
        // Here the tooltip is always emitted when 'always_on == true'.
        static int always_on = 0;
        LinearUI::RadioButton("Off", &always_on, 0);
        LinearUI::SameLine();
        LinearUI::RadioButton("Always On (Simple)", &always_on, 1);
        LinearUI::SameLine();
        LinearUI::RadioButton("Always On (Advanced)", &always_on, 2);
        if (always_on == 1)
            LinearUI::SetTooltip("I am following you around.");
        else if (always_on == 2 && LinearUI::BeginTooltip())
        {
            LinearUI::ProgressBar(sinf((float)LinearUI::GetTime()) * 0.5f + 0.5f, LinearVec2(LinearUI::GetFontSize() * 25, 0.0f));
            LinearUI::EndTooltip();
        }

        LinearUI::SeparatorText("Custom");

        HelpMarker(
            "Passing LinearUIHoveredFlags_ForTooltip to IsItemHovered() is the preferred way to standardize "
            "tooltip activation details across your application. You may however decide to use custom "
            "flags for a specific tooltip instance.");

        // The following examples are passed for documentation purpose but may not be useful to most users.
        // Passing LinearUIHoveredFlags_ForTooltip to IsItemHovered() will pull LinearUIHoveredFlags flags values from
        // 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav' depending on whether mouse or keyboard/gamepad is being used.
        // With default settings, LinearUIHoveredFlags_ForTooltip is equivalent to LinearUIHoveredFlags_DelayShort + LinearUIHoveredFlags_Stationary.
        LinearUI::Button("Manual", sz);
        if (LinearUI::IsItemHovered(LinearUIHoveredFlags_ForTooltip))
            LinearUI::SetTooltip("I am a manually emitted tooltip.");

        LinearUI::Button("DelayNone", sz);
        if (LinearUI::IsItemHovered(LinearUIHoveredFlags_DelayNone))
            LinearUI::SetTooltip("I am a tooltip with no delay.");

        LinearUI::Button("DelayShort", sz);
        if (LinearUI::IsItemHovered(LinearUIHoveredFlags_DelayShort | LinearUIHoveredFlags_NoSharedDelay))
            LinearUI::SetTooltip("I am a tooltip with a short delay (%0.2f sec).", LinearUI::GetStyle().HoverDelayShort);

        LinearUI::Button("DelayLong", sz);
        if (LinearUI::IsItemHovered(LinearUIHoveredFlags_DelayNormal | LinearUIHoveredFlags_NoSharedDelay))
            LinearUI::SetTooltip("I am a tooltip with a long delay (%0.2f sec).", LinearUI::GetStyle().HoverDelayNormal);

        LinearUI::Button("Stationary", sz);
        if (LinearUI::IsItemHovered(LinearUIHoveredFlags_Stationary))
            LinearUI::SetTooltip("I am a tooltip requiring mouse to be stationary before activating.");

        // Using LinearUIHoveredFlags_ForTooltip will pull flags from 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav',
        // which default value include the LinearUIHoveredFlags_AllowWhenDisabled flag.
        LinearUI::BeginDisabled();
        LinearUI::Button("Disabled item", sz);
        if (LinearUI::IsItemHovered(LinearUIHoveredFlags_ForTooltip))
            LinearUI::SetTooltip("I am a tooltip for a disabled item.");
        LinearUI::EndDisabled();

        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTreeNodes()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsTreeNodes()
{
    if (LinearUI::TreeNode("Tree Nodes"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Tree Nodes");
        // See see "Examples -> Property Editor" (ShowExampleAppPropertyEditor() function) for a fancier, data-driven tree.
        if (LinearUI::TreeNode("Basic Trees"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Tree Nodes/Basic Trees");
            for (int i = 0; i < 5; i++)
            {
                // Use SetNextItemOpen() so set the default state of a node to be open. We could
                // also use TreeNodeEx() with the LinearUITreeNodeFlags_DefaultOpen flag to achieve the same thing!
                if (i == 0)
                    LinearUI::SetNextItemOpen(true, LinearUICond_Once);

                // Here we use PushID() to generate a unique base ID, and then the "" used as TreeNode id won't conflict.
                // An alternative to using 'PushID() + TreeNode("", ...)' to generate a unique ID is to use 'TreeNode((void*)(intptr_t)i, ...)',
                // aka generate a dummy pointer-sized value to be hashed. The demo below uses that technique. Both are fine.
                LinearUI::PushID(i);
                if (LinearUI::TreeNode("", "Child %d", i))
                {
                    LinearUI::Text("blah blah");
                    LinearUI::SameLine();
                    if (LinearUI::SmallButton("button")) {}
                    LinearUI::TreePop();
                }
                LinearUI::PopID();
            }
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Hierarchy Lines"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Tree Nodes/Hierarchy Lines");
            static LinearUITreeNodeFlags base_flags = LinearUITreeNodeFlags_DrawLinesFull | LinearUITreeNodeFlags_DefaultOpen;
            HelpMarker("Default option for DrawLinesXXX is stored in style.TreeLinesFlags");
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_DrawLinesNone", &base_flags, LinearUITreeNodeFlags_DrawLinesNone);
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_DrawLinesFull", &base_flags, LinearUITreeNodeFlags_DrawLinesFull);
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_DrawLinesToNodes", &base_flags, LinearUITreeNodeFlags_DrawLinesToNodes);

            if (LinearUI::TreeNodeEx("Parent", base_flags))
            {
                if (LinearUI::TreeNodeEx("Child 1", base_flags))
                {
                    LinearUI::Button("Button for Child 1");
                    LinearUI::TreePop();
                }
                if (LinearUI::TreeNodeEx("Child 2", base_flags))
                {
                    LinearUI::Button("Button for Child 2");
                    LinearUI::TreePop();
                }
                LinearUI::Text("Remaining contents");
                LinearUI::Text("Remaining contents");
                LinearUI::TreePop();
            }

            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Clipping Large Trees"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Tree Nodes/Clipping Large Trees");
            LinearUI::TextWrapped(
                "- Using LinearUIListClipper with trees is a less easy than on arrays or grids.\n"
                "- Refer to 'Demo->Examples->Property Editor' for an example of how to do that.\n"
                "- Discuss in #3823");
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Selectable Nodes"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Tree Nodes/Selectable Nodes");
            HelpMarker(
                "Manually implemented selectable nodes.\n"
                "Click to select, Ctrl+Click to toggle, click on arrows or double-click to open.\n\n"
                "You may also use the multi-select API (see 'Demo->Widgets->Selection State & Multi-Select') for more advanced multi-selection features.");

            // Hold in 'selection_mask' a simple representation of what may be user-side selection state.
            // - You may retain selection state inside or outside your objects in whatever format you see fit.
            //   You may use LinearUISelectionBasicStorage which is conceptually close to a set<> of identifiers.
            // - We record which node was clicked and then apply selection at the end of the loop.
            // - This is a manual and simplified reimplementation of multi-selection, which the full
            //   BeginMultiSelect() API implements better, but which is not trivial to wire for trees.
            static int selection_mask = 0x00;
            int node_clicked_idx = -1;
            for (int node_n = 0; node_n < 6; node_n++)
            {
                // Disable the default "open on single-click behavior" + set Selected flag according to our selection.
                // To alter selection we use if 'IsItemClicked() && !IsItemToggledOpen()', so clicking on an arrow doesn't alter selection.
                // In a BeginMultiSelect()/EndMultiSelect() we could use IsItemToggledSelection() but here we reimplement and use our own logic.
                LinearUITreeNodeFlags flags = LinearUITreeNodeFlags_OpenOnArrow | LinearUITreeNodeFlags_OpenOnDoubleClick | LinearUITreeNodeFlags_SpanAvailWidth;
                if (selection_mask & (1 << node_n))
                    flags |= LinearUITreeNodeFlags_Selected;

                bool is_open = LinearUI::TreeNodeEx((void*)(intptr_t)node_n, flags, "Selectable Node %d", node_n);
                if (LinearUI::IsItemClicked() && !LinearUI::IsItemToggledOpen())
                    node_clicked_idx = node_n;
                if (is_open)
                {
                    LinearUI::BulletText("<Node contents here>");
                    LinearUI::TreePop();
                }
            }
            if (node_clicked_idx != -1)
            {
                // Update selection state (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
                if (LinearUI::GetIO().KeyCtrl)
                    selection_mask ^= (1 << node_clicked_idx);          // Ctrl+Click to toggle
                else //if (!(selection_mask & (1 << node_clicked_idx))) // Depending on selection behavior you want, may want to preserve selection when clicking on item that is part of the selection
                    selection_mask = (1 << node_clicked_idx);           // Click to single-select
            }
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Advanced"))
        {
            LINEARUI_DEMO_MARKER("Widgets/Tree Nodes/Advanced");
            static LinearUITreeNodeFlags base_flags = LinearUITreeNodeFlags_OpenOnArrow | LinearUITreeNodeFlags_OpenOnDoubleClick | LinearUITreeNodeFlags_SpanAvailWidth;
            static bool align_label_with_current_x_position = false;
            static bool use_drag_and_drop = false;
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_OpenOnArrow", &base_flags, LinearUITreeNodeFlags_OpenOnArrow);
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_OpenOnDoubleClick", &base_flags, LinearUITreeNodeFlags_OpenOnDoubleClick);
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_SpanAvailWidth", &base_flags, LinearUITreeNodeFlags_SpanAvailWidth); LinearUI::SameLine(); HelpMarker("Extend hit area to all available width instead of allowing more items to be laid out after the node.");
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_SpanFullWidth", &base_flags, LinearUITreeNodeFlags_SpanFullWidth);
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_SpanLabelWidth", &base_flags, LinearUITreeNodeFlags_SpanLabelWidth); LinearUI::SameLine(); HelpMarker("Reduce hit area to the text label and a bit of margin.");
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_SpanAllColumns", &base_flags, LinearUITreeNodeFlags_SpanAllColumns); LinearUI::SameLine(); HelpMarker("For use in Tables only.");
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_AllowOverlap", &base_flags, LinearUITreeNodeFlags_AllowOverlap);
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_Framed", &base_flags, LinearUITreeNodeFlags_Framed); LinearUI::SameLine(); HelpMarker("Draw frame with background (e.g. for CollapsingHeader)");
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_FramePadding", &base_flags, LinearUITreeNodeFlags_FramePadding);
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_NavLeftJumpsToParent", &base_flags, LinearUITreeNodeFlags_NavLeftJumpsToParent);

            HelpMarker("Default option for DrawLinesXXX is stored in style.TreeLinesFlags");
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_DrawLinesNone", &base_flags, LinearUITreeNodeFlags_DrawLinesNone);
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_DrawLinesFull", &base_flags, LinearUITreeNodeFlags_DrawLinesFull);
            LinearUI::CheckboxFlags("LinearUITreeNodeFlags_DrawLinesToNodes", &base_flags, LinearUITreeNodeFlags_DrawLinesToNodes);

            LinearUI::Checkbox("Align label with current X position", &align_label_with_current_x_position);
            LinearUI::Checkbox("Make Tree Nodes as drag & drop sources", &use_drag_and_drop);
            if (align_label_with_current_x_position)
                LinearUI::Unindent(LinearUI::GetTreeNodeToLabelSpacing());

            for (int node_n = 0; node_n < 6; node_n++)
            {
                LinearUITreeNodeFlags node_flags = base_flags;
                if (node_n < 3)
                {
                    // Items 0..2 are Tree Node
                    bool is_open = LinearUI::TreeNodeEx((void*)(intptr_t)node_n, node_flags, "Selectable Node %d", node_n);
                    if (use_drag_and_drop && LinearUI::BeginDragDropSource())
                    {
                        LinearUI::SetDragDropPayload("MY_TREENODE_PAYLOAD_TYPE", NULL, 0);
                        LinearUI::Text("This is a drag and drop source");
                        LinearUI::EndDragDropSource();
                    }
                    if (node_n == 2 && (base_flags & LinearUITreeNodeFlags_SpanLabelWidth))
                    {
                        // Item 2 has an additional inline button to help demonstrate SpanLabelWidth.
                        LinearUI::SameLine();
                        if (LinearUI::SmallButton("button")) {}
                    }
                    if (is_open)
                    {
                        LinearUI::BulletText("Blah blah\nBlah Blah");
                        LinearUI::SameLine();
                        LinearUI::SmallButton("Button");
                        LinearUI::TreePop();
                    }
                }
                else
                {
                    // Items 3..5 are Tree Leaves
                    // The only reason we use TreeNode at all is to allow selection of the leaf. Otherwise we can
                    // use BulletText() or advance the cursor by GetTreeNodeToLabelSpacing() and call Text().
                    node_flags |= LinearUITreeNodeFlags_Leaf | LinearUITreeNodeFlags_NoTreePushOnOpen; // LinearUITreeNodeFlags_Bullet
                    LinearUI::TreeNodeEx((void*)(intptr_t)node_n, node_flags, "Selectable Leaf %d", node_n);
                    if (use_drag_and_drop && LinearUI::BeginDragDropSource())
                    {
                        LinearUI::SetDragDropPayload("MY_TREENODE_PAYLOAD_TYPE", NULL, 0);
                        LinearUI::Text("This is a drag and drop source");
                        LinearUI::EndDragDropSource();
                    }
                }
            }
            if (align_label_with_current_x_position)
                LinearUI::Indent(LinearUI::GetTreeNodeToLabelSpacing());
            LinearUI::TreePop();
        }
        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsVerticalSliders()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsVerticalSliders()
{
    if (LinearUI::TreeNode("Vertical Sliders"))
    {
        LINEARUI_DEMO_MARKER("Widgets/Vertical Sliders");
        const float spacing = 4;
        LinearUI::PushStyleVar(LinearUIStyleVar_ItemSpacing, LinearVec2(spacing, spacing));

        static int int_value = 0;
        LinearUI::VSliderInt("##int", LinearVec2(18, 160), &int_value, 0, 5);
        LinearUI::SameLine();

        static float values[7] = { 0.0f, 0.60f, 0.35f, 0.9f, 0.70f, 0.20f, 0.0f };
        LinearUI::PushID("set1");
        for (int i = 0; i < 7; i++)
        {
            if (i > 0) LinearUI::SameLine();
            LinearUI::PushID(i);
            LinearUI::PushStyleColor(LinearUICol_FrameBg, (LinearVec4)LinearColor::HSV(i / 7.0f, 0.5f, 0.5f));
            LinearUI::PushStyleColor(LinearUICol_FrameBgHovered, (LinearVec4)LinearColor::HSV(i / 7.0f, 0.6f, 0.5f));
            LinearUI::PushStyleColor(LinearUICol_FrameBgActive, (LinearVec4)LinearColor::HSV(i / 7.0f, 0.7f, 0.5f));
            LinearUI::PushStyleColor(LinearUICol_SliderGrab, (LinearVec4)LinearColor::HSV(i / 7.0f, 0.9f, 0.9f));
            LinearUI::VSliderFloat("##v", LinearVec2(18, 160), &values[i], 0.0f, 1.0f, "");
            if (LinearUI::IsItemActive() || LinearUI::IsItemHovered())
                LinearUI::SetTooltip("%.3f", values[i]);
            LinearUI::PopStyleColor(4);
            LinearUI::PopID();
        }
        LinearUI::PopID();

        LinearUI::SameLine();
        LinearUI::PushID("set2");
        static float values2[4] = { 0.20f, 0.80f, 0.40f, 0.25f };
        const int rows = 3;
        const LinearVec2 small_slider_size(18, (float)(int)((160.0f - (rows - 1) * spacing) / rows));
        for (int nx = 0; nx < 4; nx++)
        {
            if (nx > 0) LinearUI::SameLine();
            LinearUI::BeginGroup();
            for (int ny = 0; ny < rows; ny++)
            {
                LinearUI::PushID(nx * rows + ny);
                LinearUI::VSliderFloat("##v", small_slider_size, &values2[nx], 0.0f, 1.0f, "");
                if (LinearUI::IsItemActive() || LinearUI::IsItemHovered())
                    LinearUI::SetTooltip("%.3f", values2[nx]);
                LinearUI::PopID();
            }
            LinearUI::EndGroup();
        }
        LinearUI::PopID();

        LinearUI::SameLine();
        LinearUI::PushID("set3");
        for (int i = 0; i < 4; i++)
        {
            if (i > 0) LinearUI::SameLine();
            LinearUI::PushID(i);
            LinearUI::PushStyleVar(LinearUIStyleVar_GrabMinSize, 40);
            LinearUI::VSliderFloat("##v", LinearVec2(40, 160), &values[i], 0.0f, 1.0f, "%.2f\nsec");
            LinearUI::PopStyleVar();
            LinearUI::PopID();
        }
        LinearUI::PopID();
        LinearUI::PopStyleVar();
        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgets()
//-----------------------------------------------------------------------------

static void DemoWindowWidgets(LinearUIDemoWindowData* demo_data)
{
    //LinearUI::SetNextItemOpen(true, LinearUICond_Once);
    if (!LinearUI::CollapsingHeader("Widgets"))
        return;
    // LINEARUI_DEMO_MARKER("Widgets");

    const bool disable_all = demo_data->DisableSections; // The Checkbox for that is inside the "Disabled" section at the bottom
    if (disable_all)
        LinearUI::BeginDisabled();

    DemoWindowWidgetsBasic();
    DemoWindowWidgetsBullets();
    DemoWindowWidgetsCollapsingHeaders();
    DemoWindowWidgetsComboBoxes();
    DemoWindowWidgetsColorAndPickers();
    DemoWindowWidgetsDataTypes();

    if (disable_all)
        LinearUI::EndDisabled();
    DemoWindowWidgetsDisableBlocks(demo_data);
    if (disable_all)
        LinearUI::BeginDisabled();

    DemoWindowWidgetsDragAndDrop();
    DemoWindowWidgetsDragsAndSliders();
    DemoWindowWidgetsFonts();
    DemoWindowWidgetsImages();
    DemoWindowWidgetsListBoxes();
    DemoWindowWidgetsMultiComponents();
    DemoWindowWidgetsPlotting();
    DemoWindowWidgetsProgressBars();
    DemoWindowWidgetsQueryingStatuses();
    DemoWindowWidgetsSelectables();
    DemoWindowWidgetsSelectionAndMultiSelect(demo_data);
    DemoWindowWidgetsTabs();
    DemoWindowWidgetsText();
    DemoWindowWidgetsTextFilter();
    DemoWindowWidgetsTextInput();
    DemoWindowWidgetsTooltips();
    DemoWindowWidgetsTreeNodes();
    DemoWindowWidgetsVerticalSliders();

    if (disable_all)
        LinearUI::EndDisabled();
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowLayout()
//-----------------------------------------------------------------------------

static void DemoWindowLayout()
{
    if (!LinearUI::CollapsingHeader("Layout & Scrolling"))
        return;

    if (LinearUI::TreeNode("Child windows"))
    {
        LINEARUI_DEMO_MARKER("Layout/Child windows");
        LinearUI::SeparatorText("Child windows");

        HelpMarker("Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window.");
        static bool disable_mouse_wheel = false;
        static bool disable_menu = false;
        LinearUI::Checkbox("Disable Mouse Wheel", &disable_mouse_wheel);
        LinearUI::Checkbox("Disable Menu", &disable_menu);

        // Child 1: no border, enable horizontal scrollbar
        {
            LinearUIWindowFlags window_flags = LinearUIWindowFlags_HorizontalScrollbar;
            if (disable_mouse_wheel)
                window_flags |= LinearUIWindowFlags_NoScrollWithMouse;
            LinearUI::BeginChild("ChildL", LinearVec2(LinearUI::GetContentRegionAvail().x * 0.5f, 260), LinearUIChildFlags_None, window_flags);
            for (int i = 0; i < 100; i++)
                LinearUI::Text("%04d: scrollable region", i);
            LinearUI::EndChild();
        }

        LinearUI::SameLine();

        // Child 2: rounded border
        {
            LinearUIWindowFlags window_flags = LinearUIWindowFlags_None;
            if (disable_mouse_wheel)
                window_flags |= LinearUIWindowFlags_NoScrollWithMouse;
            if (!disable_menu)
                window_flags |= LinearUIWindowFlags_MenuBar;
            LinearUI::PushStyleVar(LinearUIStyleVar_ChildRounding, 5.0f);
            LinearUI::BeginChild("ChildR", LinearVec2(0, 260), LinearUIChildFlags_Borders, window_flags);
            if (!disable_menu && LinearUI::BeginMenuBar())
            {
                if (LinearUI::BeginMenu("Menu"))
                {
                    ShowExampleMenuFile();
                    LinearUI::EndMenu();
                }
                LinearUI::EndMenuBar();
            }
            if (LinearUI::BeginTable("split", 2, LinearUITableFlags_Resizable | LinearUITableFlags_NoSavedSettings))
            {
                for (int i = 0; i < 100; i++)
                {
                    char buf[32];
                    sprintf(buf, "%03d", i);
                    LinearUI::TableNextColumn();
                    LinearUI::Button(buf, LinearVec2(-FLT_MIN, 0.0f));
                }
                LinearUI::EndTable();
            }
            LinearUI::EndChild();
            LinearUI::PopStyleVar();
        }

        // Child 3: manual-resize
        LinearUI::SeparatorText("Manual-resize");
        {
            HelpMarker("Drag bottom border to resize. Double-click bottom border to auto-fit to vertical contents.");
            //if (LinearUI::Button("Set Height to 200"))
            //    LinearUI::SetNextWindowSize(LinearVec2(-FLT_MIN, 200.0f));

            LinearUI::PushStyleColor(LinearUICol_ChildBg, LinearUI::GetStyleColorVec4(LinearUICol_FrameBg));
            if (LinearUI::BeginChild("ResizableChild", LinearVec2(-FLT_MIN, LinearUI::GetTextLineHeightWithSpacing() * 8), LinearUIChildFlags_Borders | LinearUIChildFlags_ResizeY))
                for (int n = 0; n < 10; n++)
                    LinearUI::Text("Line %04d", n);
            LinearUI::PopStyleColor();
            LinearUI::EndChild();
        }

        // Child 4: auto-resizing height with a limit
        LinearUI::SeparatorText("Auto-resize with constraints");
        {
            static int draw_lines = 3;
            static int max_height_in_lines = 10;
            LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 8);
            LinearUI::DragInt("Lines Count", &draw_lines, 0.2f);
            LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 8);
            LinearUI::DragInt("Max Height (in Lines)", &max_height_in_lines, 0.2f);

            LinearUI::SetNextWindowSizeConstraints(LinearVec2(0.0f, LinearUI::GetTextLineHeightWithSpacing() * 1), LinearVec2(FLT_MAX, LinearUI::GetTextLineHeightWithSpacing() * max_height_in_lines));
            if (LinearUI::BeginChild("ConstrainedChild", LinearVec2(-FLT_MIN, 0.0f), LinearUIChildFlags_Borders | LinearUIChildFlags_AutoResizeY))
                for (int n = 0; n < draw_lines; n++)
                    LinearUI::Text("Line %04d", n);
            LinearUI::EndChild();
        }

        LinearUI::SeparatorText("Misc/Advanced");

        // Demonstrate a few extra things
        // - Changing LinearUICol_ChildBg (which is transparent black in default styles)
        // - Using SetCursorPos() to position child window (the child window is an item from the POV of parent window)
        //   You can also call SetNextWindowPos() to position the child window. The parent window will effectively
        //   layout from this position.
        // - Using LinearUI::GetItemRectMin/Max() to query the "item" state (because the child window is an item from
        //   the POV of the parent window). See 'Demo->Querying Status (Edited/Active/Hovered etc.)' for details.
        {
            static int offset_x = 0;
            static bool override_bg_color = true;
            static LinearUIChildFlags child_flags = LinearUIChildFlags_Borders | LinearUIChildFlags_ResizeX | LinearUIChildFlags_ResizeY;
            LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 8);
            LinearUI::DragInt("Offset X", &offset_x, 1.0f, -1000, 1000);
            LinearUI::Checkbox("Override ChildBg color", &override_bg_color);
            LinearUI::CheckboxFlags("LinearUIChildFlags_Borders", &child_flags, LinearUIChildFlags_Borders);
            LinearUI::CheckboxFlags("LinearUIChildFlags_AlwaysUseWindowPadding", &child_flags, LinearUIChildFlags_AlwaysUseWindowPadding);
            LinearUI::CheckboxFlags("LinearUIChildFlags_ResizeX", &child_flags, LinearUIChildFlags_ResizeX);
            LinearUI::CheckboxFlags("LinearUIChildFlags_ResizeY", &child_flags, LinearUIChildFlags_ResizeY);
            LinearUI::CheckboxFlags("LinearUIChildFlags_FrameStyle", &child_flags, LinearUIChildFlags_FrameStyle);
            LinearUI::SameLine(); HelpMarker("Style the child window like a framed item: use FrameBg, FrameRounding, FrameBorderSize, FramePadding instead of ChildBg, ChildRounding, ChildBorderSize, WindowPadding.");
            if (child_flags & LinearUIChildFlags_FrameStyle)
                override_bg_color = false;

            LinearUI::SetCursorPosX(LinearUI::GetCursorPosX() + (float)offset_x);
            if (override_bg_color)
                LinearUI::PushStyleColor(LinearUICol_ChildBg, LINEARUI_COL32(255, 0, 0, 100));
            LinearUI::BeginChild("Red", LinearVec2(200, 100), child_flags, LinearUIWindowFlags_None);
            if (override_bg_color)
                LinearUI::PopStyleColor();

            for (int n = 0; n < 50; n++)
                LinearUI::Text("Some test %d", n);
            LinearUI::EndChild();
            bool child_is_hovered = LinearUI::IsItemHovered();
            LinearVec2 child_rect_min = LinearUI::GetItemRectMin();
            LinearVec2 child_rect_max = LinearUI::GetItemRectMax();
            LinearUI::Text("Hovered: %d", child_is_hovered);
            LinearUI::Text("Rect of child window is: (%.0f,%.0f) (%.0f,%.0f)", child_rect_min.x, child_rect_min.y, child_rect_max.x, child_rect_max.y);
        }

        LinearUI::TreePop();
    }

    if (LinearUI::TreeNode("Widgets Width"))
    {
        LINEARUI_DEMO_MARKER("Layout/Widgets Width");
        static float f = 0.0f;
        static bool show_indented_items = true;
        LinearUI::Checkbox("Show indented items", &show_indented_items);

        // Use SetNextItemWidth() to set the width of a single upcoming item.
        // Use PushItemWidth()/PopItemWidth() to set the width of a group of items.
        // In real code use you'll probably want to choose width values that are proportional to your font size
        // e.g. Using '20.0f * GetFontSize()' as width instead of '200.0f', etc.

        LinearUI::Text("SetNextItemWidth/PushItemWidth(100)");
        LinearUI::SameLine(); HelpMarker("Fixed width.");
        LinearUI::PushItemWidth(100);
        LinearUI::DragFloat("float##1b", &f);
        if (show_indented_items)
        {
            LinearUI::Indent();
            LinearUI::DragFloat("float (indented)##1b", &f);
            LinearUI::Unindent();
        }
        LinearUI::PopItemWidth();

        LinearUI::Text("SetNextItemWidth/PushItemWidth(-100)");
        LinearUI::SameLine(); HelpMarker("Align to right edge minus 100");
        LinearUI::PushItemWidth(-100);
        LinearUI::DragFloat("float##2a", &f);
        if (show_indented_items)
        {
            LinearUI::Indent();
            LinearUI::DragFloat("float (indented)##2b", &f);
            LinearUI::Unindent();
        }
        LinearUI::PopItemWidth();

        LinearUI::Text("SetNextItemWidth/PushItemWidth(GetContentRegionAvail().x * 0.5f)");
        LinearUI::SameLine(); HelpMarker("Half of available width.\n(~ right-cursor_pos)\n(works within a column set)");
        LinearUI::PushItemWidth(LinearUI::GetContentRegionAvail().x * 0.5f);
        LinearUI::DragFloat("float##3a", &f);
        if (show_indented_items)
        {
            LinearUI::Indent();
            LinearUI::DragFloat("float (indented)##3b", &f);
            LinearUI::Unindent();
        }
        LinearUI::PopItemWidth();

        LinearUI::Text("SetNextItemWidth/PushItemWidth(-GetContentRegionAvail().x * 0.5f)");
        LinearUI::SameLine(); HelpMarker("Align to right edge minus half");
        LinearUI::PushItemWidth(-LinearUI::GetContentRegionAvail().x * 0.5f);
        LinearUI::DragFloat("float##4a", &f);
        if (show_indented_items)
        {
            LinearUI::Indent();
            LinearUI::DragFloat("float (indented)##4b", &f);
            LinearUI::Unindent();
        }
        LinearUI::PopItemWidth();

        LinearUI::Text("SetNextItemWidth/PushItemWidth(-Min(GetContentRegionAvail().x * 0.40f, GetFontSize() * 12))");
        LinearUI::PushItemWidth(-LINEARUI_MIN(LinearUI::GetFontSize() * 12, LinearUI::GetContentRegionAvail().x * 0.40f));
        LinearUI::DragFloat("float##5a", &f);
        if (show_indented_items)
        {
            LinearUI::Indent();
            LinearUI::DragFloat("float (indented)##5b", &f);
            LinearUI::Unindent();
        }
        LinearUI::PopItemWidth();

        // Demonstrate using PushItemWidth to surround three items.
        // Calling SetNextItemWidth() before each of them would have the same effect.
        LinearUI::Text("SetNextItemWidth/PushItemWidth(-FLT_MIN)");
        LinearUI::SameLine(); HelpMarker("Align to right edge");
        LinearUI::PushItemWidth(-FLT_MIN);
        LinearUI::DragFloat("##float6a", &f);
        if (show_indented_items)
        {
            LinearUI::Indent();
            LinearUI::DragFloat("float (indented)##6b", &f);
            LinearUI::Unindent();
        }
        LinearUI::PopItemWidth();

        LinearUI::TreePop();
    }

    if (LinearUI::TreeNode("Basic Horizontal Layout"))
    {
        LINEARUI_DEMO_MARKER("Layout/Basic Horizontal Layout");
        LinearUI::TextWrapped("(Use LinearUI::SameLine() to keep adding items to the right of the preceding item)");

        // Text
        LINEARUI_DEMO_MARKER("Layout/Basic Horizontal Layout/SameLine");
        LinearUI::Text("Two items: Hello"); LinearUI::SameLine();
        LinearUI::TextColored(LinearVec4(1, 1, 0, 1), "Sailor");

        // Adjust spacing
        LinearUI::Text("More spacing: Hello"); LinearUI::SameLine(0, 20);
        LinearUI::TextColored(LinearVec4(1, 1, 0, 1), "Sailor");

        // Button
        LinearUI::AlignTextToFramePadding();
        LinearUI::Text("Normal buttons"); LinearUI::SameLine();
        LinearUI::Button("Banana"); LinearUI::SameLine();
        LinearUI::Button("Apple"); LinearUI::SameLine();
        LinearUI::Button("Corniflower");

        // Button
        LinearUI::Text("Small buttons"); LinearUI::SameLine();
        LinearUI::SmallButton("Like this one"); LinearUI::SameLine();
        LinearUI::Text("can fit within a text block.");

        // Aligned to arbitrary position. Easy/cheap column.
        LINEARUI_DEMO_MARKER("Layout/Basic Horizontal Layout/SameLine (with offset)");
        LinearUI::Text("Aligned");
        LinearUI::SameLine(150); LinearUI::Text("x=150");
        LinearUI::SameLine(300); LinearUI::Text("x=300");
        LinearUI::Text("Aligned");
        LinearUI::SameLine(150); LinearUI::SmallButton("x=150");
        LinearUI::SameLine(300); LinearUI::SmallButton("x=300");

        // Checkbox
        LINEARUI_DEMO_MARKER("Layout/Basic Horizontal Layout/SameLine (more)");
        static bool c1 = false, c2 = false, c3 = false, c4 = false;
        LinearUI::Checkbox("My", &c1); LinearUI::SameLine();
        LinearUI::Checkbox("Tailor", &c2); LinearUI::SameLine();
        LinearUI::Checkbox("Is", &c3); LinearUI::SameLine();
        LinearUI::Checkbox("Rich", &c4);

        // Various
        static float f0 = 1.0f, f1 = 2.0f, f2 = 3.0f;
        LinearUI::PushItemWidth(LinearUI::CalcTextSize("AAAAAAA").x);
        const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD" };
        static int item = -1;
        LinearUI::Combo("Combo", &item, items, LINEARUI_COUNTOF(items)); LinearUI::SameLine();
        LinearUI::SliderFloat("X", &f0, 0.0f, 5.0f); LinearUI::SameLine();
        LinearUI::SliderFloat("Y", &f1, 0.0f, 5.0f); LinearUI::SameLine();
        LinearUI::SliderFloat("Z", &f2, 0.0f, 5.0f);

        LinearUI::Text("Lists:");
        static int selection[4] = { 0, 1, 2, 3 };
        for (int i = 0; i < 4; i++)
        {
            if (i > 0) LinearUI::SameLine();
            LinearUI::PushID(i);
            LinearUI::ListBox("", &selection[i], items, LINEARUI_COUNTOF(items));
            LinearUI::PopID();
            //LinearUI::SetItemTooltip("ListBox %d hovered", i);
        }
        LinearUI::PopItemWidth();

        // Dummy
        LINEARUI_DEMO_MARKER("Layout/Basic Horizontal Layout/Dummy");
        LinearVec2 button_sz(40, 40);
        LinearUI::Button("A", button_sz); LinearUI::SameLine();
        LinearUI::Dummy(button_sz); LinearUI::SameLine();
        LinearUI::Button("B", button_sz);

        // Manually wrapping
        // (we should eventually provide this as an automatic layout feature, but for now you can do it manually)
        LINEARUI_DEMO_MARKER("Layout/Basic Horizontal Layout/Manual wrapping");
        LinearUI::Text("Manual wrapping:");
        LinearUIStyle& style = LinearUI::GetStyle();
        int buttons_count = 20;
        float window_visible_x2 = LinearUI::GetCursorScreenPos().x + LinearUI::GetContentRegionAvail().x;
        for (int n = 0; n < buttons_count; n++)
        {
            LinearUI::PushID(n);
            LinearUI::Button("Box", button_sz);
            float last_button_x2 = LinearUI::GetItemRectMax().x;
            float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
            if (n + 1 < buttons_count && next_button_x2 < window_visible_x2)
                LinearUI::SameLine();
            LinearUI::PopID();
        }

        LinearUI::TreePop();
    }

    if (LinearUI::TreeNode("Groups"))
    {
        LINEARUI_DEMO_MARKER("Layout/Groups");
        HelpMarker(
            "BeginGroup() basically locks the horizontal position for new line. "
            "EndGroup() bundles the whole group so that you can use \"item\" functions such as "
            "IsItemHovered()/IsItemActive() or SameLine() etc. on the whole group.");
        LinearUI::BeginGroup();
        {
            LinearUI::BeginGroup();
            LinearUI::Button("AAA");
            LinearUI::SameLine();
            LinearUI::Button("BBB");
            LinearUI::SameLine();
            LinearUI::BeginGroup();
            LinearUI::Button("CCC");
            LinearUI::Button("DDD");
            LinearUI::EndGroup();
            LinearUI::SameLine();
            LinearUI::Button("EEE");
            LinearUI::EndGroup();
            LinearUI::SetItemTooltip("First group hovered");
        }
        // Capture the group size and create widgets using the same size
        LinearVec2 size = LinearUI::GetItemRectSize();
        const float values[5] = { 0.5f, 0.20f, 0.80f, 0.60f, 0.25f };
        LinearUI::PlotHistogram("##values", values, LINEARUI_COUNTOF(values), 0, NULL, 0.0f, 1.0f, size);

        LinearUI::Button("ACTION", LinearVec2((size.x - LinearUI::GetStyle().ItemSpacing.x) * 0.5f, size.y));
        LinearUI::SameLine();
        LinearUI::Button("REACTION", LinearVec2((size.x - LinearUI::GetStyle().ItemSpacing.x) * 0.5f, size.y));
        LinearUI::EndGroup();
        LinearUI::SameLine();

        LinearUI::Button("LEVERAGE\nBUZZWORD", size);
        LinearUI::SameLine();

        if (LinearUI::BeginListBox("List", size))
        {
            LinearUI::Selectable("Selected", true);
            LinearUI::Selectable("Not Selected", false);
            LinearUI::EndListBox();
        }

        LinearUI::TreePop();
    }

    if (LinearUI::TreeNode("Text Baseline Alignment"))
    {
        LINEARUI_DEMO_MARKER("Layout/Text Baseline Alignment");
        {
            LinearUI::BulletText("Text baseline:");
            LinearUI::SameLine(); HelpMarker(
                "This is testing the vertical alignment that gets applied on text to keep it aligned with widgets. "
                "Lines only composed of text or \"small\" widgets use less vertical space than lines with framed widgets.");
            LinearUI::Indent();

            LinearUI::Text("KO Blahblah"); LinearUI::SameLine();
            LinearUI::Button("Some framed item"); LinearUI::SameLine();
            HelpMarker("Baseline of button will look misaligned with text..");

            // If your line starts with text, call AlignTextToFramePadding() to align text to upcoming widgets.
            // (because we don't know what's coming after the Text() statement, we need to move the text baseline
            // down by FramePadding.y ahead of time)
            LinearUI::AlignTextToFramePadding();
            LinearUI::Text("OK Blahblah"); LinearUI::SameLine();
            LinearUI::Button("Some framed item##2"); LinearUI::SameLine();
            HelpMarker("We call AlignTextToFramePadding() to vertically align the text baseline by +FramePadding.y");

            // SmallButton() uses the same vertical padding as Text
            LinearUI::Button("TEST##1"); LinearUI::SameLine();
            LinearUI::Text("TEST"); LinearUI::SameLine();
            LinearUI::SmallButton("TEST##2");

            // If your line starts with text, call AlignTextToFramePadding() to align text to upcoming widgets.
            LinearUI::AlignTextToFramePadding();
            LinearUI::Text("Text aligned to framed item"); LinearUI::SameLine();
            LinearUI::Button("Item##1"); LinearUI::SameLine();
            LinearUI::Text("Item"); LinearUI::SameLine();
            LinearUI::SmallButton("Item##2"); LinearUI::SameLine();
            LinearUI::Button("Item##3");

            LinearUI::Unindent();
        }

        LinearUI::Spacing();

        {
            LinearUI::BulletText("Multi-line text:");
            LinearUI::Indent();
            LinearUI::Text("One\nTwo\nThree"); LinearUI::SameLine();
            LinearUI::Text("Hello\nWorld"); LinearUI::SameLine();
            LinearUI::Text("Banana");

            LinearUI::Text("Banana"); LinearUI::SameLine();
            LinearUI::Text("Hello\nWorld"); LinearUI::SameLine();
            LinearUI::Text("One\nTwo\nThree");

            LinearUI::Button("HOP##1"); LinearUI::SameLine();
            LinearUI::Text("Banana"); LinearUI::SameLine();
            LinearUI::Text("Hello\nWorld"); LinearUI::SameLine();
            LinearUI::Text("Banana");

            LinearUI::Button("HOP##2"); LinearUI::SameLine();
            LinearUI::Text("Hello\nWorld"); LinearUI::SameLine();
            LinearUI::Text("Banana");
            LinearUI::Unindent();
        }

        LinearUI::Spacing();

        {
            LinearUI::BulletText("Misc items:");
            LinearUI::Indent();

            // SmallButton() sets FramePadding to zero. Text baseline is aligned to match baseline of previous Button.
            LinearUI::Button("80x80", LinearVec2(80, 80));
            LinearUI::SameLine();
            LinearUI::Button("50x50", LinearVec2(50, 50));
            LinearUI::SameLine();
            LinearUI::Button("Button()");
            LinearUI::SameLine();
            LinearUI::SmallButton("SmallButton()");

            // Tree
            // (here the node appears after a button and has odd intent, so we use LinearUITreeNodeFlags_DrawLinesNone to disable hierarchy outline)
            const float spacing = LinearUI::GetStyle().ItemInnerSpacing.x;
            LinearUI::Button("Button##1"); // Will make line higher
            LinearUI::SameLine(0.0f, spacing);
            if (LinearUI::TreeNodeEx("Node##1", LinearUITreeNodeFlags_DrawLinesNone))
            {
                // Placeholder tree data
                for (int i = 0; i < 6; i++)
                    LinearUI::BulletText("Item %d..", i);
                LinearUI::TreePop();
            }

            const float padding = (float)(int)(LinearUI::GetFontSize() * 1.20f); // Large padding
            LinearUI::PushStyleVarY(LinearUIStyleVar_FramePadding, padding);
            LinearUI::Button("Button##2");
            LinearUI::PopStyleVar();
            LinearUI::SameLine(0.0f, spacing);
            if (LinearUI::TreeNodeEx("Node##2", LinearUITreeNodeFlags_DrawLinesNone))
                LinearUI::TreePop();

            // Vertically align text node a bit lower so it'll be vertically centered with upcoming widget.
            // Otherwise you can use SmallButton() (smaller fit).
            LinearUI::AlignTextToFramePadding();

            // Common mistake to avoid: if we want to SameLine after TreeNode we need to do it before we add
            // other contents "inside" the node.
            bool node_open = LinearUI::TreeNode("Node##3");
            LinearUI::SameLine(0.0f, spacing); LinearUI::Button("Button##3");
            if (node_open)
            {
                // Placeholder tree data
                for (int i = 0; i < 6; i++)
                    LinearUI::BulletText("Item %d..", i);
                LinearUI::TreePop();
            }

            // Bullet
            LinearUI::Button("Button##4");
            LinearUI::SameLine(0.0f, spacing);
            LinearUI::BulletText("Bullet text");

            LinearUI::AlignTextToFramePadding();
            LinearUI::BulletText("Node");
            LinearUI::SameLine(0.0f, spacing); LinearUI::Button("Button##5");
            LinearUI::Unindent();
        }

        LinearUI::TreePop();
    }

    if (LinearUI::TreeNode("Scrolling"))
    {
        LINEARUI_DEMO_MARKER("Layout/Scrolling/Vertical");
        // Vertical scroll functions
        HelpMarker("Use SetScrollHereY() or SetScrollFromPosY() to scroll to a given vertical position.");

        static int track_item = 50;
        static bool enable_track = true;
        static bool enable_extra_decorations = false;
        static float scroll_to_off_px = 0.0f;
        static float scroll_to_pos_px = 200.0f;

        LinearUI::Checkbox("Decoration", &enable_extra_decorations);

        LinearUI::PushItemWidth(LinearUI::GetFontSize() * 10);
        enable_track |= LinearUI::DragInt("##item", &track_item, 0.25f, 0, 99, "Item = %d");
        LinearUI::SameLine();
        LinearUI::Checkbox("Track", &enable_track);

        bool scroll_to_off = LinearUI::DragFloat("##off", &scroll_to_off_px, 1.00f, 0, FLT_MAX, "+%.0f px");
        LinearUI::SameLine();
        scroll_to_off |= LinearUI::Button("Scroll Offset");

        bool scroll_to_pos = LinearUI::DragFloat("##pos", &scroll_to_pos_px, 1.00f, -10, FLT_MAX, "X/Y = %.0f px");
        LinearUI::SameLine();
        scroll_to_pos |= LinearUI::Button("Scroll To Pos");
        LinearUI::PopItemWidth();

        if (scroll_to_off || scroll_to_pos)
            enable_track = false;

        LinearUIStyle& style = LinearUI::GetStyle();
        float child_w = (LinearUI::GetContentRegionAvail().x - 4 * style.ItemSpacing.x) / 5;
        if (child_w < 1.0f)
            child_w = 1.0f;
        LinearUI::PushID("##VerticalScrolling");
        for (int i = 0; i < 5; i++)
        {
            if (i > 0) LinearUI::SameLine();
            LinearUI::BeginGroup();
            const char* names[] = { "Top", "25%", "Center", "75%", "Bottom" };
            LinearUI::TextUnformatted(names[i]);

            const LinearUIWindowFlags child_flags = enable_extra_decorations ? LinearUIWindowFlags_MenuBar : 0;
            const LinearUIID child_id = LinearUI::GetID((void*)(intptr_t)i);
            const bool child_is_visible = LinearUI::BeginChild(child_id, LinearVec2(child_w, 200.0f), LinearUIChildFlags_Borders, child_flags);
            if (LinearUI::BeginMenuBar())
            {
                LinearUI::TextUnformatted("abc");
                LinearUI::EndMenuBar();
            }
            if (scroll_to_off)
                LinearUI::SetScrollY(scroll_to_off_px);
            if (scroll_to_pos)
                LinearUI::SetScrollFromPosY(LinearUI::GetCursorStartPos().y + scroll_to_pos_px, i * 0.25f);
            if (child_is_visible) // Avoid calling SetScrollHereY when running with culled items
            {
                for (int item = 0; item < 100; item++)
                {
                    if (enable_track && item == track_item)
                    {
                        LinearUI::TextColored(LinearVec4(1, 1, 0, 1), "Item %d", item);
                        LinearUI::SetScrollHereY(i * 0.25f); // 0.0f:top, 0.5f:center, 1.0f:bottom
                    }
                    else
                    {
                        LinearUI::Text("Item %d", item);
                    }
                }
            }
            float scroll_y = LinearUI::GetScrollY();
            float scroll_max_y = LinearUI::GetScrollMaxY();
            LinearUI::EndChild();
            LinearUI::Text("%.0f/%.0f", scroll_y, scroll_max_y);
            LinearUI::EndGroup();
        }
        LinearUI::PopID();

        // Horizontal scroll functions
        LINEARUI_DEMO_MARKER("Layout/Scrolling/Horizontal");
        LinearUI::Spacing();
        HelpMarker(
            "Use SetScrollHereX() or SetScrollFromPosX() to scroll to a given horizontal position.\n\n"
            "Because the clipping rectangle of most window hides half worth of WindowPadding on the "
            "left/right, using SetScrollFromPosX(+1) will usually result in clipped text whereas the "
            "equivalent SetScrollFromPosY(+1) wouldn't.");
        LinearUI::PushID("##HorizontalScrolling");
        for (int i = 0; i < 5; i++)
        {
            float child_height = LinearUI::GetTextLineHeight() + style.ScrollbarSize + style.WindowPadding.y * 2.0f;
            LinearUIWindowFlags child_flags = LinearUIWindowFlags_HorizontalScrollbar | (enable_extra_decorations ? LinearUIWindowFlags_AlwaysVerticalScrollbar : 0);
            LinearUIID child_id = LinearUI::GetID((void*)(intptr_t)i);
            bool child_is_visible = LinearUI::BeginChild(child_id, LinearVec2(-100, child_height), LinearUIChildFlags_Borders, child_flags);
            if (scroll_to_off)
                LinearUI::SetScrollX(scroll_to_off_px);
            if (scroll_to_pos)
                LinearUI::SetScrollFromPosX(LinearUI::GetCursorStartPos().x + scroll_to_pos_px, i * 0.25f);
            if (child_is_visible) // Avoid calling SetScrollHereY when running with culled items
            {
                for (int item = 0; item < 100; item++)
                {
                    if (item > 0)
                        LinearUI::SameLine();
                    if (enable_track && item == track_item)
                    {
                        LinearUI::TextColored(LinearVec4(1, 1, 0, 1), "Item %d", item);
                        LinearUI::SetScrollHereX(i * 0.25f); // 0.0f:left, 0.5f:center, 1.0f:right
                    }
                    else
                    {
                        LinearUI::Text("Item %d", item);
                    }
                }
            }
            float scroll_x = LinearUI::GetScrollX();
            float scroll_max_x = LinearUI::GetScrollMaxX();
            LinearUI::EndChild();
            LinearUI::SameLine();
            const char* names[] = { "Left", "25%", "Center", "75%", "Right" };
            LinearUI::Text("%s\n%.0f/%.0f", names[i], scroll_x, scroll_max_x);
            LinearUI::Spacing();
        }
        LinearUI::PopID();

        // Miscellaneous Horizontal Scrolling Demo
        LINEARUI_DEMO_MARKER("Layout/Scrolling/Horizontal (more)");
        HelpMarker(
            "Horizontal scrolling for a window is enabled via the LinearUIWindowFlags_HorizontalScrollbar flag.\n\n"
            "You may want to also explicitly specify content width by using SetNextWindowContentWidth() before Begin().");
        static int lines = 7;
        LinearUI::SliderInt("Lines", &lines, 1, 15);
        LinearUI::PushStyleVar(LinearUIStyleVar_FrameRounding, 3.0f);
        LinearUI::PushStyleVar(LinearUIStyleVar_FramePadding, LinearVec2(2.0f, 1.0f));
        LinearVec2 scrolling_child_size = LinearVec2(0, LinearUI::GetFrameHeightWithSpacing() * 7 + 30);
        LinearUI::BeginChild("scrolling", scrolling_child_size, LinearUIChildFlags_Borders, LinearUIWindowFlags_HorizontalScrollbar);
        for (int line = 0; line < lines; line++)
        {
            // Display random stuff. For the sake of this trivial demo we are using basic Button() + SameLine()
            // If you want to create your own time line for a real application you may be better off manipulating
            // the cursor position yourself, aka using SetCursorPos/SetCursorScreenPos to position the widgets
            // yourself. You may also want to use the lower-level LinearDrawList API.
            int num_buttons = 10 + ((line & 1) ? line * 9 : line * 3);
            for (int n = 0; n < num_buttons; n++)
            {
                if (n > 0) LinearUI::SameLine();
                LinearUI::PushID(n + line * 1000);
                char num_buf[16];
                sprintf(num_buf, "%d", n);
                const char* label = (!(n % 15)) ? "FizzBuzz" : (!(n % 3)) ? "Fizz" : (!(n % 5)) ? "Buzz" : num_buf;
                float hue = n * 0.05f;
                LinearUI::PushStyleColor(LinearUICol_Button, (LinearVec4)LinearColor::HSV(hue, 0.6f, 0.6f));
                LinearUI::PushStyleColor(LinearUICol_ButtonHovered, (LinearVec4)LinearColor::HSV(hue, 0.7f, 0.7f));
                LinearUI::PushStyleColor(LinearUICol_ButtonActive, (LinearVec4)LinearColor::HSV(hue, 0.8f, 0.8f));
                LinearUI::Button(label, LinearVec2(40.0f + sinf((float)(line + n)) * 20.0f, 0.0f));
                LinearUI::PopStyleColor(3);
                LinearUI::PopID();
            }
        }
        float scroll_x = LinearUI::GetScrollX();
        float scroll_max_x = LinearUI::GetScrollMaxX();
        LinearUI::EndChild();
        LinearUI::PopStyleVar(2);
        float scroll_x_delta = 0.0f;
        LinearUI::SmallButton("<<");
        if (LinearUI::IsItemActive())
            scroll_x_delta = -LinearUI::GetIO().DeltaTime * 1000.0f;
        LinearUI::SameLine();
        LinearUI::Text("Scroll from code"); LinearUI::SameLine();
        LinearUI::SmallButton(">>");
        if (LinearUI::IsItemActive())
            scroll_x_delta = +LinearUI::GetIO().DeltaTime * 1000.0f;
        LinearUI::SameLine();
        LinearUI::Text("%.0f/%.0f", scroll_x, scroll_max_x);
        if (scroll_x_delta != 0.0f)
        {
            // Demonstrate a trick: you can use Begin to set yourself in the context of another window
            // (here we are already out of your child window)
            LinearUI::BeginChild("scrolling");
            LinearUI::SetScrollX(LinearUI::GetScrollX() + scroll_x_delta);
            LinearUI::EndChild();
        }
        LinearUI::Spacing();

        static bool show_horizontal_contents_size_demo_window = false;
        LinearUI::Checkbox("Show Horizontal contents size demo window", &show_horizontal_contents_size_demo_window);

        if (show_horizontal_contents_size_demo_window)
        {
            static bool show_h_scrollbar = true;
            static bool show_button = true;
            static bool show_tree_nodes = true;
            static bool show_text_wrapped = false;
            static bool show_columns = true;
            static bool show_tab_bar = true;
            static bool show_child = false;
            static bool explicit_content_size = false;
            static float contents_size_x = 300.0f;
            if (explicit_content_size)
                LinearUI::SetNextWindowContentSize(LinearVec2(contents_size_x, 0.0f));
            LinearUI::Begin("Horizontal contents size demo window", &show_horizontal_contents_size_demo_window, show_h_scrollbar ? LinearUIWindowFlags_HorizontalScrollbar : 0);
            LINEARUI_DEMO_MARKER("Layout/Scrolling/Horizontal contents size demo window");
            LinearUI::PushStyleVar(LinearUIStyleVar_ItemSpacing, LinearVec2(2, 0));
            LinearUI::PushStyleVar(LinearUIStyleVar_FramePadding, LinearVec2(2, 0));
            HelpMarker(
                "Test how different widgets react and impact the work rectangle growing when horizontal scrolling is enabled.\n\n"
                "Use 'Metrics->Tools->Show windows rectangles' to visualize rectangles.");
            LinearUI::Checkbox("H-scrollbar", &show_h_scrollbar);
            LinearUI::Checkbox("Button", &show_button);            // Will grow contents size (unless explicitly overwritten)
            LinearUI::Checkbox("Tree nodes", &show_tree_nodes);    // Will grow contents size and display highlight over full width
            LinearUI::Checkbox("Text wrapped", &show_text_wrapped);// Will grow and use contents size
            LinearUI::Checkbox("Columns", &show_columns);          // Will use contents size
            LinearUI::Checkbox("Tab bar", &show_tab_bar);          // Will use contents size
            LinearUI::Checkbox("Child", &show_child);              // Will grow and use contents size
            LinearUI::Checkbox("Explicit content size", &explicit_content_size);
            LinearUI::Text("Scroll %.1f/%.1f %.1f/%.1f", LinearUI::GetScrollX(), LinearUI::GetScrollMaxX(), LinearUI::GetScrollY(), LinearUI::GetScrollMaxY());
            if (explicit_content_size)
            {
                LinearUI::SameLine();
                LinearUI::SetNextItemWidth(LinearUI::CalcTextSize("123456").x);
                LinearUI::DragFloat("##csx", &contents_size_x);
                LinearVec2 p = LinearUI::GetCursorScreenPos();
                LinearUI::GetWindowDrawList()->AddRectFilled(p, LinearVec2(p.x + 10, p.y + 10), LINEARUI_COL32_WHITE);
                LinearUI::GetWindowDrawList()->AddRectFilled(LinearVec2(p.x + contents_size_x - 10, p.y), LinearVec2(p.x + contents_size_x, p.y + 10), LINEARUI_COL32_WHITE);
                LinearUI::Dummy(LinearVec2(0, 10));
            }
            LinearUI::PopStyleVar(2);
            LinearUI::Separator();
            if (show_button)
            {
                LinearUI::Button("this is a 300-wide button", LinearVec2(300, 0));
            }
            if (show_tree_nodes)
            {
                bool open = true;
                if (LinearUI::TreeNode("this is a tree node"))
                {
                    if (LinearUI::TreeNode("another one of those tree node..."))
                    {
                        LinearUI::Text("Some tree contents");
                        LinearUI::TreePop();
                    }
                    LinearUI::TreePop();
                }
                LinearUI::CollapsingHeader("CollapsingHeader", &open);
            }
            if (show_text_wrapped)
            {
                LinearUI::TextWrapped("This text should automatically wrap on the edge of the work rectangle.");
            }
            if (show_columns)
            {
                LinearUI::Text("Tables:");
                if (LinearUI::BeginTable("table", 4, LinearUITableFlags_Borders))
                {
                    for (int n = 0; n < 4; n++)
                    {
                        LinearUI::TableNextColumn();
                        LinearUI::Text("Width %.2f", LinearUI::GetContentRegionAvail().x);
                    }
                    LinearUI::EndTable();
                }
                LinearUI::Text("Columns:");
                LinearUI::Columns(4);
                for (int n = 0; n < 4; n++)
                {
                    LinearUI::Text("Width %.2f", LinearUI::GetColumnWidth());
                    LinearUI::NextColumn();
                }
                LinearUI::Columns(1);
            }
            if (show_tab_bar && LinearUI::BeginTabBar("Hello"))
            {
                if (LinearUI::BeginTabItem("OneOneOne")) { LinearUI::EndTabItem(); }
                if (LinearUI::BeginTabItem("TwoTwoTwo")) { LinearUI::EndTabItem(); }
                if (LinearUI::BeginTabItem("ThreeThreeThree")) { LinearUI::EndTabItem(); }
                if (LinearUI::BeginTabItem("FourFourFour")) { LinearUI::EndTabItem(); }
                LinearUI::EndTabBar();
            }
            if (show_child)
            {
                LinearUI::BeginChild("child", LinearVec2(0, 0), LinearUIChildFlags_Borders);
                LinearUI::EndChild();
            }
            LinearUI::End();
        }

        LinearUI::TreePop();
    }

    if (LinearUI::TreeNode("Text Clipping"))
    {
        LINEARUI_DEMO_MARKER("Layout/Text Clipping");
        static LinearVec2 size(100.0f, 100.0f);
        static LinearVec2 offset(30.0f, 30.0f);
        LinearUI::DragFloat2("size", (float*)&size, 0.5f, 1.0f, 200.0f, "%.0f");
        LinearUI::TextWrapped("(Click and drag to scroll)");

        HelpMarker(
            "(Left) Using LinearUI::PushClipRect():\n"
            "Will alter LinearUI hit-testing logic + LinearDrawList rendering.\n"
            "(use this if you want your clipping rectangle to affect interactions)\n\n"
            "(Center) Using LinearDrawList::PushClipRect():\n"
            "Will alter LinearDrawList rendering only.\n"
            "(use this as a shortcut if you are only using LinearDrawList calls)\n\n"
            "(Right) Using LinearDrawList::AddText() with a fine ClipRect:\n"
            "Will alter only this specific LinearDrawList::AddText() rendering.\n"
            "This is often used internally to avoid altering the clipping rectangle and minimize draw calls.");

        for (int n = 0; n < 3; n++)
        {
            if (n > 0)
                LinearUI::SameLine();

            LinearUI::PushID(n);
            LinearUI::InvisibleButton("##canvas", size);
            if (LinearUI::IsItemActive() && LinearUI::IsMouseDragging(LinearUIMouseButton_Left))
            {
                offset.x += LinearUI::GetIO().MouseDelta.x;
                offset.y += LinearUI::GetIO().MouseDelta.y;
            }
            LinearUI::PopID();
            if (!LinearUI::IsItemVisible()) // Skip rendering as LinearDrawList elements are not clipped.
                continue;

            const LinearVec2 p0 = LinearUI::GetItemRectMin();
            const LinearVec2 p1 = LinearUI::GetItemRectMax();
            const char* text_str = "Line 1 hello\nLine 2 clip me!";
            const LinearVec2 text_pos = LinearVec2(p0.x + offset.x, p0.y + offset.y);
            LinearDrawList* draw_list = LinearUI::GetWindowDrawList();
            switch (n)
            {
            case 0:
                LinearUI::PushClipRect(p0, p1, true);
                draw_list->AddRectFilled(p0, p1, LINEARUI_COL32(90, 90, 120, 255));
                draw_list->AddText(text_pos, LINEARUI_COL32_WHITE, text_str);
                LinearUI::PopClipRect();
                break;
            case 1:
                draw_list->PushClipRect(p0, p1, true);
                draw_list->AddRectFilled(p0, p1, LINEARUI_COL32(90, 90, 120, 255));
                draw_list->AddText(text_pos, LINEARUI_COL32_WHITE, text_str);
                draw_list->PopClipRect();
                break;
            case 2:
                LinearVec4 clip_rect(p0.x, p0.y, p1.x, p1.y); // AddText() takes a LinearVec4* here so let's convert.
                draw_list->AddRectFilled(p0, p1, LINEARUI_COL32(90, 90, 120, 255));
                draw_list->AddText(LinearUI::GetFont(), LinearUI::GetFontSize(), text_pos, LINEARUI_COL32_WHITE, text_str, NULL, 0.0f, &clip_rect);
                break;
            }
        }

        LinearUI::TreePop();
    }

    if (LinearUI::TreeNode("Overlap Mode"))
    {
        LINEARUI_DEMO_MARKER("Layout/Overlap Mode");
        static bool enable_allow_overlap = true;

        HelpMarker(
            "Hit-testing is by default performed in item submission order, which generally is perceived as 'back-to-front'.\n\n"
            "By using SetNextItemAllowOverlap() you can notify that an item may be overlapped by another. "
            "Doing so alters the hovering logic: items using AllowOverlap mode requires an extra frame to accept hovered state.");
        LinearUI::Checkbox("Enable AllowOverlap", &enable_allow_overlap);

        LinearVec2 button1_pos = LinearUI::GetCursorScreenPos();
        LinearVec2 button2_pos = LinearVec2(button1_pos.x + 50.0f, button1_pos.y + 50.0f);
        if (enable_allow_overlap)
            LinearUI::SetNextItemAllowOverlap();
        LinearUI::Button("Button 1", LinearVec2(80, 80));
        LinearUI::SetCursorScreenPos(button2_pos);
        LinearUI::Button("Button 2", LinearVec2(80, 80));

        // This is typically used with width-spanning items.
        // (note that Selectable() has a dedicated flag LinearUISelectableFlags_AllowOverlap, which is a shortcut
        // for using SetNextItemAllowOverlap(). For demo purpose we use SetNextItemAllowOverlap() here.)
        if (enable_allow_overlap)
            LinearUI::SetNextItemAllowOverlap();
        LinearUI::Selectable("Some Selectable", false);
        LinearUI::SameLine();
        LinearUI::SmallButton("++");

        LinearUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowPopups()
//-----------------------------------------------------------------------------

static void DemoWindowPopups()
{
    if (!LinearUI::CollapsingHeader("Popups & Modal windows"))
        return;

    // The properties of popups windows are:
    // - They block normal mouse hovering detection outside them. (*)
    // - Unless modal, they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
    // - Their visibility state (~bool) is held internally by Dear LinearUI instead of being held by the programmer as
    //   we are used to with regular Begin() calls. User can manipulate the visibility state by calling OpenPopup().
    // (*) One can use IsItemHovered(LinearUIHoveredFlags_AllowWhenBlockedByPopup) to bypass it and detect hovering even
    //     when normally blocked by a popup.
    // Those three properties are connected. The library needs to hold their visibility state BECAUSE it can close
    // popups at any time.

    // Typical use for regular windows:
    //   bool my_tool_is_active = false; if (LinearUI::Button("Open")) my_tool_is_active = true; [...] if (my_tool_is_active) Begin("My Tool", &my_tool_is_active) { [...] } End();
    // Typical use for popups:
    //   if (LinearUI::Button("Open")) LinearUI::OpenPopup("MyPopup"); if (LinearUI::BeginPopup("MyPopup")) { [...] EndPopup(); }

    // With popups we have to go through a library call (here OpenPopup) to manipulate the visibility state.
    // This may be a bit confusing at first but it should quickly make sense. Follow on the examples below.

    if (LinearUI::TreeNode("Popups"))
    {
        LINEARUI_DEMO_MARKER("Popups/Popups");
        LinearUI::TextWrapped(
            "When a popup is active, it inhibits interacting with windows that are behind the popup. "
            "Clicking outside the popup closes it.");

        static int selected_fish = -1;
        const char* names[] = { "Bream", "Haddock", "Mackerel", "Pollock", "Tilefish" };
        static bool toggles[] = { true, false, false, false, false };

        // Simple selection popup (if you want to show the current selection inside the Button itself,
        // you may want to build a string using the "###" operator to preserve a constant ID with a variable label)
        if (LinearUI::Button("Select.."))
            LinearUI::OpenPopup("my_select_popup");
        LinearUI::SameLine();
        LinearUI::TextUnformatted(selected_fish == -1 ? "<None>" : names[selected_fish]);
        if (LinearUI::BeginPopup("my_select_popup"))
        {
            LinearUI::SeparatorText("Aquarium");
            for (int i = 0; i < LINEARUI_COUNTOF(names); i++)
                if (LinearUI::Selectable(names[i]))
                    selected_fish = i;
            LinearUI::EndPopup();
        }

        // Showing a menu with toggles
        if (LinearUI::Button("Toggle.."))
            LinearUI::OpenPopup("my_toggle_popup");
        if (LinearUI::BeginPopup("my_toggle_popup"))
        {
            for (int i = 0; i < LINEARUI_COUNTOF(names); i++)
                LinearUI::MenuItem(names[i], "", &toggles[i]);
            if (LinearUI::BeginMenu("Sub-menu"))
            {
                LinearUI::MenuItem("Click me");
                LinearUI::EndMenu();
            }

            LinearUI::Separator();
            LinearUI::Text("Tooltip here");
            LinearUI::SetItemTooltip("I am a tooltip over a popup");

            if (LinearUI::Button("Stacked Popup"))
                LinearUI::OpenPopup("another popup");
            if (LinearUI::BeginPopup("another popup"))
            {
                for (int i = 0; i < LINEARUI_COUNTOF(names); i++)
                    LinearUI::MenuItem(names[i], "", &toggles[i]);
                if (LinearUI::BeginMenu("Sub-menu"))
                {
                    LinearUI::MenuItem("Click me");
                    if (LinearUI::Button("Stacked Popup"))
                        LinearUI::OpenPopup("another popup");
                    if (LinearUI::BeginPopup("another popup"))
                    {
                        LinearUI::Text("I am the last one here.");
                        LinearUI::EndPopup();
                    }
                    LinearUI::EndMenu();
                }
                LinearUI::EndPopup();
            }
            LinearUI::EndPopup();
        }

        // Call the more complete ShowExampleMenuFile which we use in various places of this demo
        if (LinearUI::Button("With a menu.."))
            LinearUI::OpenPopup("my_file_popup");
        if (LinearUI::BeginPopup("my_file_popup", LinearUIWindowFlags_MenuBar))
        {
            if (LinearUI::BeginMenuBar())
            {
                if (LinearUI::BeginMenu("File"))
                {
                    ShowExampleMenuFile();
                    LinearUI::EndMenu();
                }
                if (LinearUI::BeginMenu("Edit"))
                {
                    LinearUI::MenuItem("Dummy");
                    LinearUI::EndMenu();
                }
                LinearUI::EndMenuBar();
            }
            LinearUI::Text("Hello from popup!");
            LinearUI::Button("This is a dummy button..");
            LinearUI::EndPopup();
        }

        LinearUI::TreePop();
    }

    if (LinearUI::TreeNode("Context menus"))
    {
        LINEARUI_DEMO_MARKER("Popups/Context menus");
        HelpMarker("\"Context\" functions are simple helpers to associate a Popup to a given Item or Window identifier.");

        // BeginPopupContextItem() is a helper to provide common/simple popup behavior of essentially doing:
        //     if (id == 0)
        //         id = GetItemID(); // Use last item id
        //     if (IsItemHovered() && IsMouseReleased(LinearUIMouseButton_Right))
        //         OpenPopup(id);
        //     return BeginPopup(id);
        // For advanced uses you may want to replicate and customize this code.
        // See more details in BeginPopupContextItem().

        // Example 1
        // When used after an item that has an ID (e.g. Button), we can skip providing an ID to BeginPopupContextItem(),
        // and BeginPopupContextItem() will use the last item ID as the popup ID.
        {
            const char* names[5] = { "Label1", "Label2", "Label3", "Label4", "Label5" };
            static int selected = -1;
            for (int n = 0; n < 5; n++)
            {
                if (LinearUI::Selectable(names[n], selected == n))
                    selected = n;
                if (LinearUI::BeginPopupContextItem()) // <-- use last item id as popup id
                {
                    selected = n;
                    LinearUI::Text("This is a popup for \"%s\"!", names[n]);
                    if (LinearUI::Button("Close"))
                        LinearUI::CloseCurrentPopup();
                    LinearUI::EndPopup();
                }
                LinearUI::SetItemTooltip("Right-click to open popup");
            }
        }

        // Example 2
        // Popup on a Text() element which doesn't have an identifier: we need to provide an identifier to BeginPopupContextItem().
        // Using an explicit identifier is also convenient if you want to activate the popups from different locations.
        {
            HelpMarker("Text() elements don't have stable identifiers so we need to provide one.");
            static float value = 0.5f;
            LinearUI::Text("Value = %.3f <-- (1) right-click this text", value);
            if (LinearUI::BeginPopupContextItem("my popup"))
            {
                if (LinearUI::Selectable("Set to zero")) value = 0.0f;
                if (LinearUI::Selectable("Set to PI")) value = 3.1415f;
                LinearUI::SetNextItemWidth(-FLT_MIN);
                LinearUI::DragFloat("##Value", &value, 0.1f, 0.0f, 0.0f);
                LinearUI::EndPopup();
            }

            // We can also use OpenPopupOnItemClick() to toggle the visibility of a given popup.
            // Here we make it that right-clicking this other text element opens the same popup as above.
            // The popup itself will be submitted by the code above.
            LinearUI::Text("(2) Or right-click this text");
            LinearUI::OpenPopupOnItemClick("my popup", LinearUIPopupFlags_MouseButtonRight);

            // Back to square one: manually open the same popup.
            if (LinearUI::Button("(3) Or click this button"))
                LinearUI::OpenPopup("my popup");
        }

        // Example 3
        // When using BeginPopupContextItem() with an implicit identifier (NULL == use last item ID),
        // we need to make sure your item identifier is stable.
        // In this example we showcase altering the item label while preserving its identifier, using the ### operator (see FAQ).
        {
            HelpMarker("Showcase using a popup ID linked to item ID, with the item having a changing label + stable ID using the ### operator.");
            static char name[32] = "Label1";
            char buf[64];
            sprintf(buf, "Button: %s###Button", name); // ### operator override ID ignoring the preceding label
            LinearUI::Button(buf);
            if (LinearUI::BeginPopupContextItem())
            {
                LinearUI::Text("Edit name:");
                LinearUI::InputText("##edit", name, LINEARUI_COUNTOF(name));
                if (LinearUI::Button("Close"))
                    LinearUI::CloseCurrentPopup();
                LinearUI::EndPopup();
            }
            LinearUI::SameLine(); LinearUI::Text("(<-- right-click here)");
        }

        LinearUI::TreePop();
    }

    if (LinearUI::TreeNode("Modals"))
    {
        LINEARUI_DEMO_MARKER("Popups/Modals");
        LinearUI::TextWrapped("Modal windows are like popups but the user cannot close them by clicking outside.");

        if (LinearUI::Button("Delete.."))
            LinearUI::OpenPopup("Delete?");

        // Always center this window when appearing
        LinearVec2 center = LinearUI::GetMainViewport()->GetCenter();
        LinearUI::SetNextWindowPos(center, LinearUICond_Appearing, LinearVec2(0.5f, 0.5f));

        if (LinearUI::BeginPopupModal("Delete?", NULL, LinearUIWindowFlags_AlwaysAutoResize))
        {
            LinearUI::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!");
            LinearUI::Separator();

            //static int unused_i = 0;
            //LinearUI::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

            static bool dont_ask_me_next_time = false;
            LinearUI::PushStyleVar(LinearUIStyleVar_FramePadding, LinearVec2(0, 0));
            LinearUI::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
            LinearUI::PopStyleVar();

            if (LinearUI::Button("OK", LinearVec2(120, 0))) { LinearUI::CloseCurrentPopup(); }
            LinearUI::SetItemDefaultFocus();
            LinearUI::SameLine();
            if (LinearUI::Button("Cancel", LinearVec2(120, 0))) { LinearUI::CloseCurrentPopup(); }
            LinearUI::EndPopup();
        }

        if (LinearUI::Button("Stacked modals.."))
            LinearUI::OpenPopup("Stacked 1");
        if (LinearUI::BeginPopupModal("Stacked 1", NULL, LinearUIWindowFlags_MenuBar))
        {
            if (LinearUI::BeginMenuBar())
            {
                if (LinearUI::BeginMenu("File"))
                {
                    if (LinearUI::MenuItem("Some menu item")) {}
                    LinearUI::EndMenu();
                }
                LinearUI::EndMenuBar();
            }
            LinearUI::Text("Hello from Stacked The First\nUsing style.Colors[LinearUICol_ModalWindowDimBg] behind it.");

            // Testing behavior of widgets stacking their own regular popups over the modal.
            static int item = 1;
            static float color[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
            LinearUI::Combo("Combo", &item, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");
            LinearUI::ColorEdit4("Color", color);

            if (LinearUI::Button("Add another modal.."))
                LinearUI::OpenPopup("Stacked 2");

            // Also demonstrate passing a bool* to BeginPopupModal(), this will create a regular close button which
            // will close the popup. Note that the visibility state of popups is owned by linearui, so the input value
            // of the bool actually doesn't matter here.
            bool unused_open = true;
            if (LinearUI::BeginPopupModal("Stacked 2", &unused_open))
            {
                LinearUI::Text("Hello from Stacked The Second!");
                LinearUI::ColorEdit4("Color", color); // Allow opening another nested popup
                if (LinearUI::Button("Close"))
                    LinearUI::CloseCurrentPopup();
                LinearUI::EndPopup();
            }

            if (LinearUI::Button("Close"))
                LinearUI::CloseCurrentPopup();
            LinearUI::EndPopup();
        }

        LinearUI::TreePop();
    }

    if (LinearUI::TreeNode("Menus inside a regular window"))
    {
        LINEARUI_DEMO_MARKER("Popups/Menus inside a regular window");
        LinearUI::TextWrapped("Below we are testing adding menu items to a regular window. It's rather unusual but should work!");
        LinearUI::Separator();

        LinearUI::MenuItem("Menu item", "Ctrl+M");
        if (LinearUI::BeginMenu("Menu inside a regular window"))
        {
            ShowExampleMenuFile();
            LinearUI::EndMenu();
        }
        LinearUI::Separator();
        LinearUI::TreePop();
    }
}

// Dummy data structure that we use for the Table demo.
// (pre-C++11 doesn't allow us to instantiate LinearVector<MyItem> template if this structure is defined inside the demo function)
namespace
{
// We are passing our own identifier to TableSetupColumn() to facilitate identifying columns in the sorting code.
// This identifier will be passed down into LinearUITableSortSpec::ColumnUserID.
// But it is possible to omit the user id parameter of TableSetupColumn() and just use the column index instead! (LinearUITableSortSpec::ColumnIndex)
// If you don't use sorting, you will generally never care about giving column an ID!
enum MyItemColumnID
{
    MyItemColumnID_ID,
    MyItemColumnID_Name,
    MyItemColumnID_Action,
    MyItemColumnID_Quantity,
    MyItemColumnID_Description
};

struct MyItem
{
    int         ID;
    const char* Name;
    int         Quantity;

    // We have a problem which is affecting _only this demo_ and should not affect your code:
    // As we don't rely on std:: or other third-party library to compile dear linearui, we only have reliable access to qsort(),
    // however qsort doesn't allow passing user data to comparing function.
    // As a workaround, we are storing the sort specs in a static/global for the comparing function to access.
    // In your own use case you would probably pass the sort specs to your sorting/comparing functions directly and not use a global.
    // We could technically call LinearUI::TableGetSortSpecs() in CompareWithSortSpecs(), but considering that this function is called
    // very often by the sorting algorithm it would be a little wasteful.
    static const LinearUITableSortSpecs* s_current_sort_specs;

    static void SortWithSortSpecs(LinearUITableSortSpecs* sort_specs, MyItem* items, int items_count)
    {
        s_current_sort_specs = sort_specs; // Store in variable accessible by the sort function.
        if (items_count > 1)
            qsort(items, (size_t)items_count, sizeof(items[0]), MyItem::CompareWithSortSpecs);
        s_current_sort_specs = NULL;
    }

    // Compare function to be used by qsort()
    static int LINEARUI_CDECL CompareWithSortSpecs(const void* lhs, const void* rhs)
    {
        const MyItem* a = (const MyItem*)lhs;
        const MyItem* b = (const MyItem*)rhs;
        for (int n = 0; n < s_current_sort_specs->SpecsCount; n++)
        {
            // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
            // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
            const LinearUITableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
            int delta = 0;
            switch (sort_spec->ColumnUserID)
            {
            case MyItemColumnID_ID:             delta = (a->ID - b->ID);                break;
            case MyItemColumnID_Name:           delta = (strcmp(a->Name, b->Name));     break;
            case MyItemColumnID_Quantity:       delta = (a->Quantity - b->Quantity);    break;
            case MyItemColumnID_Description:    delta = (strcmp(a->Name, b->Name));     break;
            default: LINEARUI_ASSERT(0); break;
            }
            if (delta > 0)
                return (sort_spec->SortDirection == LinearUISortDirection_Ascending) ? +1 : -1;
            if (delta < 0)
                return (sort_spec->SortDirection == LinearUISortDirection_Ascending) ? -1 : +1;
        }

        // qsort() is instable so always return a way to differentiate items.
        // Your own compare function may want to avoid fallback on implicit sort specs.
        // e.g. a Name compare if it wasn't already part of the sort specs.
        return a->ID - b->ID;
    }
};
const LinearUITableSortSpecs* MyItem::s_current_sort_specs = NULL;
}

// Make the UI compact because there are so many fields
static void PushStyleCompact()
{
    LinearUIStyle& style = LinearUI::GetStyle();
    LinearUI::PushStyleVarY(LinearUIStyleVar_FramePadding, (float)(int)(style.FramePadding.y * 0.60f));
    LinearUI::PushStyleVarY(LinearUIStyleVar_ItemSpacing, (float)(int)(style.ItemSpacing.y * 0.60f));
}

static void PopStyleCompact()
{
    LinearUI::PopStyleVar(2);
}

// Show a combo box with a choice of sizing policies
static void EditTableSizingFlags(LinearUITableFlags* p_flags)
{
    struct EnumDesc { LinearUITableFlags Value; const char* Name; const char* Tooltip; };
    static const EnumDesc policies[] =
    {
        { LinearUITableFlags_None,               "Default",                            "Use default sizing policy:\n- LinearUITableFlags_SizingFixedFit if ScrollX is on or if host window has LinearUIWindowFlags_AlwaysAutoResize.\n- LinearUITableFlags_SizingStretchSame otherwise." },
        { LinearUITableFlags_SizingFixedFit,     "LinearUITableFlags_SizingFixedFit",     "Columns default to _WidthFixed (if resizable) or _WidthAuto (if not resizable), matching contents width." },
        { LinearUITableFlags_SizingFixedSame,    "LinearUITableFlags_SizingFixedSame",    "Columns are all the same width, matching the maximum contents width.\nImplicitly disable LinearUITableFlags_Resizable and enable LinearUITableFlags_NoKeepColumnsVisible." },
        { LinearUITableFlags_SizingStretchProp,  "LinearUITableFlags_SizingStretchProp",  "Columns default to _WidthStretch with weights proportional to their widths." },
        { LinearUITableFlags_SizingStretchSame,  "LinearUITableFlags_SizingStretchSame",  "Columns default to _WidthStretch with same weights." }
    };
    int idx;
    for (idx = 0; idx < LINEARUI_COUNTOF(policies); idx++)
        if (policies[idx].Value == (*p_flags & LinearUITableFlags_SizingMask_))
            break;
    const char* preview_text = (idx < LINEARUI_COUNTOF(policies)) ? policies[idx].Name + (idx > 0 ? strlen("LinearUITableFlags") : 0) : "";
    if (LinearUI::BeginCombo("Sizing Policy", preview_text))
    {
        for (int n = 0; n < LINEARUI_COUNTOF(policies); n++)
            if (LinearUI::Selectable(policies[n].Name, idx == n))
                *p_flags = (*p_flags & ~LinearUITableFlags_SizingMask_) | policies[n].Value;
        LinearUI::EndCombo();
    }
    LinearUI::SameLine();
    LinearUI::TextDisabled("(?)");
    if (LinearUI::BeginItemTooltip())
    {
        LinearUI::PushTextWrapPos(LinearUI::GetFontSize() * 50.0f);
        for (int m = 0; m < LINEARUI_COUNTOF(policies); m++)
        {
            LinearUI::Separator();
            LinearUI::Text("%s:", policies[m].Name);
            LinearUI::Separator();
            LinearUI::SetCursorPosX(LinearUI::GetCursorPosX() + LinearUI::GetStyle().IndentSpacing * 0.5f);
            LinearUI::TextUnformatted(policies[m].Tooltip);
        }
        LinearUI::PopTextWrapPos();
        LinearUI::EndTooltip();
    }
}

static void EditTableColumnsFlags(LinearUITableColumnFlags* p_flags)
{
    LinearUI::CheckboxFlags("_Disabled", p_flags, LinearUITableColumnFlags_Disabled); LinearUI::SameLine(); HelpMarker("Master disable flag (also hide from context menu)");
    LinearUI::CheckboxFlags("_DefaultHide", p_flags, LinearUITableColumnFlags_DefaultHide);
    LinearUI::CheckboxFlags("_DefaultSort", p_flags, LinearUITableColumnFlags_DefaultSort);
    if (LinearUI::CheckboxFlags("_WidthStretch", p_flags, LinearUITableColumnFlags_WidthStretch))
        *p_flags &= ~(LinearUITableColumnFlags_WidthMask_ ^ LinearUITableColumnFlags_WidthStretch);
    if (LinearUI::CheckboxFlags("_WidthFixed", p_flags, LinearUITableColumnFlags_WidthFixed))
        *p_flags &= ~(LinearUITableColumnFlags_WidthMask_ ^ LinearUITableColumnFlags_WidthFixed);
    LinearUI::CheckboxFlags("_NoResize", p_flags, LinearUITableColumnFlags_NoResize);
    LinearUI::CheckboxFlags("_NoReorder", p_flags, LinearUITableColumnFlags_NoReorder);
    LinearUI::CheckboxFlags("_NoHide", p_flags, LinearUITableColumnFlags_NoHide);
    LinearUI::CheckboxFlags("_NoClip", p_flags, LinearUITableColumnFlags_NoClip);
    LinearUI::CheckboxFlags("_NoSort", p_flags, LinearUITableColumnFlags_NoSort);
    LinearUI::CheckboxFlags("_NoSortAscending", p_flags, LinearUITableColumnFlags_NoSortAscending);
    LinearUI::CheckboxFlags("_NoSortDescending", p_flags, LinearUITableColumnFlags_NoSortDescending);
    LinearUI::CheckboxFlags("_NoHeaderLabel", p_flags, LinearUITableColumnFlags_NoHeaderLabel);
    LinearUI::CheckboxFlags("_NoHeaderWidth", p_flags, LinearUITableColumnFlags_NoHeaderWidth);
    LinearUI::CheckboxFlags("_PreferSortAscending", p_flags, LinearUITableColumnFlags_PreferSortAscending);
    LinearUI::CheckboxFlags("_PreferSortDescending", p_flags, LinearUITableColumnFlags_PreferSortDescending);
    LinearUI::CheckboxFlags("_IndentEnable", p_flags, LinearUITableColumnFlags_IndentEnable); LinearUI::SameLine(); HelpMarker("Default for column 0");
    LinearUI::CheckboxFlags("_IndentDisable", p_flags, LinearUITableColumnFlags_IndentDisable); LinearUI::SameLine(); HelpMarker("Default for column >0");
    LinearUI::CheckboxFlags("_AngledHeader", p_flags, LinearUITableColumnFlags_AngledHeader);
}

static void ShowTableColumnsStatusFlags(LinearUITableColumnFlags flags)
{
    LinearUI::CheckboxFlags("_IsEnabled", &flags, LinearUITableColumnFlags_IsEnabled);
    LinearUI::CheckboxFlags("_IsVisible", &flags, LinearUITableColumnFlags_IsVisible);
    LinearUI::CheckboxFlags("_IsSorted", &flags, LinearUITableColumnFlags_IsSorted);
    LinearUI::CheckboxFlags("_IsHovered", &flags, LinearUITableColumnFlags_IsHovered);
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowTables()
//-----------------------------------------------------------------------------

static void DemoWindowTables()
{
    //LinearUI::SetNextItemOpen(true, LinearUICond_Once);
    if (!LinearUI::CollapsingHeader("Tables & Columns"))
        return;

    // Using those as a base value to create width/height that are factor of the size of our font
    const float TEXT_BASE_WIDTH = LinearUI::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = LinearUI::GetTextLineHeightWithSpacing();

    LinearUI::PushID("Tables");

    int open_action = -1;
    if (LinearUI::Button("Expand all"))
        open_action = 1;
    LinearUI::SameLine();
    if (LinearUI::Button("Collapse all"))
        open_action = 0;
    LinearUI::SameLine();

    // Options
    static bool disable_indent = false;
    LinearUI::Checkbox("Disable tree indentation", &disable_indent);
    LinearUI::SameLine();
    HelpMarker("Disable the indenting of tree nodes so demo tables can use the full window width.");
    LinearUI::Separator();
    if (disable_indent)
        LinearUI::PushStyleVar(LinearUIStyleVar_IndentSpacing, 0.0f);

    // About Styling of tables
    // Most settings are configured on a per-table basis via the flags passed to BeginTable() and TableSetupColumns APIs.
    // There are however a few settings that a shared and part of the LinearUIStyle structure:
    //   style.CellPadding                          // Padding within each cell
    //   style.Colors[LinearUICol_TableHeaderBg]       // Table header background
    //   style.Colors[LinearUICol_TableBorderStrong]   // Table outer and header borders
    //   style.Colors[LinearUICol_TableBorderLight]    // Table inner borders
    //   style.Colors[LinearUICol_TableRowBg]          // Table row background when LinearUITableFlags_RowBg is enabled (even rows)
    //   style.Colors[LinearUICol_TableRowBgAlt]       // Table row background when LinearUITableFlags_RowBg is enabled (odds rows)

    // Demos
    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Basic"))
    {
        LINEARUI_DEMO_MARKER("Tables/Basic");
        // Here we will showcase three different ways to output a table.
        // They are very simple variations of a same thing!

        // [Method 1] Using TableNextRow() to create a new row, and TableSetColumnIndex() to select the column.
        // In many situations, this is the most flexible and easy to use pattern.
        HelpMarker("Using TableNextRow() + calling TableSetColumnIndex() _before_ each cell, in a loop.");
        if (LinearUI::BeginTable("table1", 3))
        {
            for (int row = 0; row < 4; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    LinearUI::TableSetColumnIndex(column);
                    LinearUI::Text("Row %d Column %d", row, column);
                }
            }
            LinearUI::EndTable();
        }

        // [Method 2] Using TableNextColumn() called multiple times, instead of using a for loop + TableSetColumnIndex().
        // This is generally more convenient when you have code manually submitting the contents of each column.
        HelpMarker("Using TableNextRow() + calling TableNextColumn() _before_ each cell, manually.");
        if (LinearUI::BeginTable("table2", 3))
        {
            for (int row = 0; row < 4; row++)
            {
                LinearUI::TableNextRow();
                LinearUI::TableNextColumn();
                LinearUI::Text("Row %d", row);
                LinearUI::TableNextColumn();
                LinearUI::Text("Some contents");
                LinearUI::TableNextColumn();
                LinearUI::Text("123.456");
            }
            LinearUI::EndTable();
        }

        // [Method 3] We call TableNextColumn() _before_ each cell. We never call TableNextRow(),
        // as TableNextColumn() will automatically wrap around and create new rows as needed.
        // This is generally more convenient when your cells all contains the same type of data.
        HelpMarker(
            "Only using TableNextColumn(), which tends to be convenient for tables where every cell contains "
            "the same type of contents.\n This is also more similar to the old NextColumn() function of the "
            "Columns API, and provided to facilitate the Columns->Tables API transition.");
        if (LinearUI::BeginTable("table3", 3))
        {
            for (int item = 0; item < 14; item++)
            {
                LinearUI::TableNextColumn();
                LinearUI::Text("Item %d", item);
            }
            LinearUI::EndTable();
        }

        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Borders, background"))
    {
        LINEARUI_DEMO_MARKER("Tables/Borders, background");
        // Expose a few Borders related flags interactively
        enum ContentsType { CT_Text, CT_FillButton };
        static LinearUITableFlags flags = LinearUITableFlags_Borders | LinearUITableFlags_RowBg;
        static bool display_headers = false;
        static int contents_type = CT_Text;

        PushStyleCompact();
        LinearUI::CheckboxFlags("LinearUITableFlags_RowBg", &flags, LinearUITableFlags_RowBg);
        LinearUI::CheckboxFlags("LinearUITableFlags_Borders", &flags, LinearUITableFlags_Borders);
        LinearUI::SameLine(); HelpMarker("LinearUITableFlags_Borders\n = LinearUITableFlags_BordersInnerV\n | LinearUITableFlags_BordersOuterV\n | LinearUITableFlags_BordersInnerH\n | LinearUITableFlags_BordersOuterH");
        LinearUI::Indent();

        LinearUI::CheckboxFlags("LinearUITableFlags_BordersH", &flags, LinearUITableFlags_BordersH);
        LinearUI::Indent();
        LinearUI::CheckboxFlags("LinearUITableFlags_BordersOuterH", &flags, LinearUITableFlags_BordersOuterH);
        LinearUI::CheckboxFlags("LinearUITableFlags_BordersInnerH", &flags, LinearUITableFlags_BordersInnerH);
        LinearUI::Unindent();

        LinearUI::CheckboxFlags("LinearUITableFlags_BordersV", &flags, LinearUITableFlags_BordersV);
        LinearUI::Indent();
        LinearUI::CheckboxFlags("LinearUITableFlags_BordersOuterV", &flags, LinearUITableFlags_BordersOuterV);
        LinearUI::CheckboxFlags("LinearUITableFlags_BordersInnerV", &flags, LinearUITableFlags_BordersInnerV);
        LinearUI::Unindent();

        LinearUI::CheckboxFlags("LinearUITableFlags_BordersOuter", &flags, LinearUITableFlags_BordersOuter);
        LinearUI::CheckboxFlags("LinearUITableFlags_BordersInner", &flags, LinearUITableFlags_BordersInner);
        LinearUI::Unindent();

        LinearUI::AlignTextToFramePadding(); LinearUI::Text("Cell contents:");
        LinearUI::SameLine(); LinearUI::RadioButton("Text", &contents_type, CT_Text);
        LinearUI::SameLine(); LinearUI::RadioButton("FillButton", &contents_type, CT_FillButton);
        LinearUI::Checkbox("Display headers", &display_headers);
        LinearUI::CheckboxFlags("LinearUITableFlags_NoBordersInBody", &flags, LinearUITableFlags_NoBordersInBody); LinearUI::SameLine(); HelpMarker("Disable vertical borders in columns Body (borders will always appear in Headers)");
        PopStyleCompact();

        if (LinearUI::BeginTable("table1", 3, flags))
        {
            // Display headers so we can inspect their interaction with borders
            // (Headers are not the main purpose of this section of the demo, so we are not elaborating on them now. See other sections for details)
            if (display_headers)
            {
                LinearUI::TableSetupColumn("One");
                LinearUI::TableSetupColumn("Two");
                LinearUI::TableSetupColumn("Three");
                LinearUI::TableHeadersRow();
            }

            for (int row = 0; row < 5; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    LinearUI::TableSetColumnIndex(column);
                    char buf[32];
                    sprintf(buf, "Hello %d,%d", column, row);
                    if (contents_type == CT_Text)
                        LinearUI::TextUnformatted(buf);
                    else if (contents_type == CT_FillButton)
                        LinearUI::Button(buf, LinearVec2(-FLT_MIN, 0.0f));
                }
            }
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Resizable, stretch"))
    {
        LINEARUI_DEMO_MARKER("Tables/Resizable, stretch");
        // By default, if we don't enable ScrollX the sizing policy for each column is "Stretch"
        // All columns maintain a sizing weight, and they will occupy all available width.
        static LinearUITableFlags flags = LinearUITableFlags_SizingStretchSame | LinearUITableFlags_Resizable | LinearUITableFlags_BordersOuter | LinearUITableFlags_BordersV | LinearUITableFlags_ContextMenuInBody;
        PushStyleCompact();
        LinearUI::CheckboxFlags("LinearUITableFlags_Resizable", &flags, LinearUITableFlags_Resizable);
        LinearUI::CheckboxFlags("LinearUITableFlags_BordersV", &flags, LinearUITableFlags_BordersV);
        LinearUI::SameLine(); HelpMarker(
            "Using the _Resizable flag automatically enables the _BordersInnerV flag as well, "
            "this is why the resize borders are still showing when unchecking this.");
        PopStyleCompact();

        if (LinearUI::BeginTable("table1", 3, flags))
        {
            for (int row = 0; row < 5; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    LinearUI::TableSetColumnIndex(column);
                    LinearUI::Text("Hello %d,%d", column, row);
                }
            }
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Resizable, fixed"))
    {
        LINEARUI_DEMO_MARKER("Tables/Resizable, fixed");
        // Here we use LinearUITableFlags_SizingFixedFit (even though _ScrollX is not set)
        // So columns will adopt the "Fixed" policy and will maintain a fixed width regardless of the whole available width (unless table is small)
        // If there is not enough available width to fit all columns, they will however be resized down.
        // FIXME-TABLE: Providing a stretch-on-init would make sense especially for tables which don't have saved settings
        HelpMarker(
            "Using _Resizable + _SizingFixedFit flags.\n"
            "Fixed-width columns generally makes more sense if you want to use horizontal scrolling.\n\n"
            "Double-click a column border to auto-fit the column to its contents.");
        PushStyleCompact();
        static LinearUITableFlags flags = LinearUITableFlags_SizingFixedFit | LinearUITableFlags_Resizable | LinearUITableFlags_BordersOuter | LinearUITableFlags_BordersV | LinearUITableFlags_ContextMenuInBody;
        LinearUI::CheckboxFlags("LinearUITableFlags_NoHostExtendX", &flags, LinearUITableFlags_NoHostExtendX);
        PopStyleCompact();

        if (LinearUI::BeginTable("table1", 3, flags))
        {
            for (int row = 0; row < 5; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    LinearUI::TableSetColumnIndex(column);
                    LinearUI::Text("Hello %d,%d", column, row);
                }
            }
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Resizable, mixed"))
    {
        LINEARUI_DEMO_MARKER("Tables/Resizable, mixed");
        HelpMarker(
            "Using TableSetupColumn() to alter resizing policy on a per-column basis.\n\n"
            "When combining Fixed and Stretch columns, generally you only want one, maybe two trailing columns to use _WidthStretch.");
        static LinearUITableFlags flags = LinearUITableFlags_SizingFixedFit | LinearUITableFlags_RowBg | LinearUITableFlags_Borders | LinearUITableFlags_Resizable | LinearUITableFlags_Reorderable | LinearUITableFlags_Hideable;

        if (LinearUI::BeginTable("table1", 3, flags))
        {
            LinearUI::TableSetupColumn("AAA", LinearUITableColumnFlags_WidthFixed);
            LinearUI::TableSetupColumn("BBB", LinearUITableColumnFlags_WidthFixed);
            LinearUI::TableSetupColumn("CCC", LinearUITableColumnFlags_WidthStretch);
            LinearUI::TableHeadersRow();
            for (int row = 0; row < 5; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    LinearUI::TableSetColumnIndex(column);
                    LinearUI::Text("%s %d,%d", (column == 2) ? "Stretch" : "Fixed", column, row);
                }
            }
            LinearUI::EndTable();
        }
        if (LinearUI::BeginTable("table2", 6, flags))
        {
            LinearUI::TableSetupColumn("AAA", LinearUITableColumnFlags_WidthFixed);
            LinearUI::TableSetupColumn("BBB", LinearUITableColumnFlags_WidthFixed);
            LinearUI::TableSetupColumn("CCC", LinearUITableColumnFlags_WidthFixed | LinearUITableColumnFlags_DefaultHide);
            LinearUI::TableSetupColumn("DDD", LinearUITableColumnFlags_WidthStretch);
            LinearUI::TableSetupColumn("EEE", LinearUITableColumnFlags_WidthStretch);
            LinearUI::TableSetupColumn("FFF", LinearUITableColumnFlags_WidthStretch | LinearUITableColumnFlags_DefaultHide);
            LinearUI::TableHeadersRow();
            for (int row = 0; row < 5; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 6; column++)
                {
                    LinearUI::TableSetColumnIndex(column);
                    LinearUI::Text("%s %d,%d", (column >= 3) ? "Stretch" : "Fixed", column, row);
                }
            }
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Reorderable, hideable, with headers"))
    {
        LINEARUI_DEMO_MARKER("Tables/Reorderable, hideable, with headers");
        HelpMarker(
            "Click and drag column headers to reorder columns.\n\n"
            "Right-click on a header to open a context menu.");
        static LinearUITableFlags flags = LinearUITableFlags_Resizable | LinearUITableFlags_Reorderable | LinearUITableFlags_Hideable | LinearUITableFlags_BordersOuter | LinearUITableFlags_BordersV;
        PushStyleCompact();
        LinearUI::CheckboxFlags("LinearUITableFlags_Resizable", &flags, LinearUITableFlags_Resizable);
        LinearUI::CheckboxFlags("LinearUITableFlags_Reorderable", &flags, LinearUITableFlags_Reorderable);
        LinearUI::CheckboxFlags("LinearUITableFlags_Hideable", &flags, LinearUITableFlags_Hideable);
        LinearUI::CheckboxFlags("LinearUITableFlags_NoBordersInBody", &flags, LinearUITableFlags_NoBordersInBody);
        LinearUI::CheckboxFlags("LinearUITableFlags_NoBordersInBodyUntilResize", &flags, LinearUITableFlags_NoBordersInBodyUntilResize); LinearUI::SameLine(); HelpMarker("Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers)");
        LinearUI::CheckboxFlags("LinearUITableFlags_HighlightHoveredColumn", &flags, LinearUITableFlags_HighlightHoveredColumn);
        PopStyleCompact();

        if (LinearUI::BeginTable("table1", 3, flags))
        {
            // Submit columns name with TableSetupColumn() and call TableHeadersRow() to create a row with a header in each column.
            // (Later we will show how TableSetupColumn() has other uses, optional flags, sizing weight etc.)
            LinearUI::TableSetupColumn("One");
            LinearUI::TableSetupColumn("Two");
            LinearUI::TableSetupColumn("Three");
            LinearUI::TableHeadersRow();
            for (int row = 0; row < 6; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    LinearUI::TableSetColumnIndex(column);
                    LinearUI::Text("Hello %d,%d", column, row);
                }
            }
            LinearUI::EndTable();
        }

        // Use outer_size.x == 0.0f instead of default to make the table as tight as possible
        // (only valid when no scrolling and no stretch column)
        if (LinearUI::BeginTable("table2", 3, flags | LinearUITableFlags_SizingFixedFit, LinearVec2(0.0f, 0.0f)))
        {
            LinearUI::TableSetupColumn("One");
            LinearUI::TableSetupColumn("Two");
            LinearUI::TableSetupColumn("Three");
            LinearUI::TableHeadersRow();
            for (int row = 0; row < 6; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    LinearUI::TableSetColumnIndex(column);
                    LinearUI::Text("Fixed %d,%d", column, row);
                }
            }
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Padding"))
    {
        LINEARUI_DEMO_MARKER("Tables/Padding");
        // First example: showcase use of padding flags and effect of BorderOuterV/BorderInnerV on X padding.
        // We don't expose BorderOuterH/BorderInnerH here because they have no effect on X padding.
        HelpMarker(
            "We often want outer padding activated when any using features which makes the edges of a column visible:\n"
            "e.g.:\n"
            "- BorderOuterV\n"
            "- any form of row selection\n"
            "Because of this, activating BorderOuterV sets the default to PadOuterX. "
            "Using PadOuterX or NoPadOuterX you can override the default.\n\n"
            "Actual padding values are using style.CellPadding.\n\n"
            "In this demo we don't show horizontal borders to emphasize how they don't affect default horizontal padding.");

        static LinearUITableFlags flags1 = LinearUITableFlags_BordersV;
        PushStyleCompact();
        LinearUI::CheckboxFlags("LinearUITableFlags_PadOuterX", &flags1, LinearUITableFlags_PadOuterX);
        LinearUI::SameLine(); HelpMarker("Enable outer-most padding (default if LinearUITableFlags_BordersOuterV is set)");
        LinearUI::CheckboxFlags("LinearUITableFlags_NoPadOuterX", &flags1, LinearUITableFlags_NoPadOuterX);
        LinearUI::SameLine(); HelpMarker("Disable outer-most padding (default if LinearUITableFlags_BordersOuterV is not set)");
        LinearUI::CheckboxFlags("LinearUITableFlags_NoPadInnerX", &flags1, LinearUITableFlags_NoPadInnerX);
        LinearUI::SameLine(); HelpMarker("Disable inner padding between columns (double inner padding if BordersOuterV is on, single inner padding if BordersOuterV is off)");
        LinearUI::CheckboxFlags("LinearUITableFlags_BordersOuterV", &flags1, LinearUITableFlags_BordersOuterV);
        LinearUI::CheckboxFlags("LinearUITableFlags_BordersInnerV", &flags1, LinearUITableFlags_BordersInnerV);
        static bool show_headers = false;
        LinearUI::Checkbox("show_headers", &show_headers);
        PopStyleCompact();

        if (LinearUI::BeginTable("table_padding", 3, flags1))
        {
            if (show_headers)
            {
                LinearUI::TableSetupColumn("One");
                LinearUI::TableSetupColumn("Two");
                LinearUI::TableSetupColumn("Three");
                LinearUI::TableHeadersRow();
            }

            for (int row = 0; row < 5; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    LinearUI::TableSetColumnIndex(column);
                    if (row == 0)
                    {
                        LinearUI::Text("Avail %.2f", LinearUI::GetContentRegionAvail().x);
                    }
                    else
                    {
                        char buf[32];
                        sprintf(buf, "Hello %d,%d", column, row);
                        LinearUI::Button(buf, LinearVec2(-FLT_MIN, 0.0f));
                    }
                    //if (LinearUI::TableGetColumnFlags() & LinearUITableColumnFlags_IsHovered)
                    //    LinearUI::TableSetBgColor(LinearUITableBgTarget_CellBg, LINEARUI_COL32(0, 100, 0, 255));
                }
            }
            LinearUI::EndTable();
        }

        // Second example: set style.CellPadding to (0.0) or a custom value.
        // FIXME-TABLE: Vertical border effectively not displayed the same way as horizontal one...
        HelpMarker("Setting style.CellPadding to (0,0) or a custom value.");
        static LinearUITableFlags flags2 = LinearUITableFlags_Borders | LinearUITableFlags_RowBg;
        static LinearVec2 cell_padding(0.0f, 0.0f);
        static bool show_widget_frame_bg = true;

        PushStyleCompact();
        LinearUI::CheckboxFlags("LinearUITableFlags_Borders", &flags2, LinearUITableFlags_Borders);
        LinearUI::CheckboxFlags("LinearUITableFlags_BordersH", &flags2, LinearUITableFlags_BordersH);
        LinearUI::CheckboxFlags("LinearUITableFlags_BordersV", &flags2, LinearUITableFlags_BordersV);
        LinearUI::CheckboxFlags("LinearUITableFlags_BordersInner", &flags2, LinearUITableFlags_BordersInner);
        LinearUI::CheckboxFlags("LinearUITableFlags_BordersOuter", &flags2, LinearUITableFlags_BordersOuter);
        LinearUI::CheckboxFlags("LinearUITableFlags_RowBg", &flags2, LinearUITableFlags_RowBg);
        LinearUI::CheckboxFlags("LinearUITableFlags_Resizable", &flags2, LinearUITableFlags_Resizable);
        LinearUI::Checkbox("show_widget_frame_bg", &show_widget_frame_bg);
        LinearUI::SliderFloat2("CellPadding", &cell_padding.x, 0.0f, 10.0f, "%.0f");
        PopStyleCompact();

        LinearUI::PushStyleVar(LinearUIStyleVar_CellPadding, cell_padding);
        if (LinearUI::BeginTable("table_padding_2", 3, flags2))
        {
            static char text_bufs[3 * 5][16]; // Mini text storage for 3x5 cells
            static bool init = true;
            if (!show_widget_frame_bg)
                LinearUI::PushStyleColor(LinearUICol_FrameBg, 0);
            for (int cell = 0; cell < 3 * 5; cell++)
            {
                LinearUI::TableNextColumn();
                if (init)
                    strcpy(text_bufs[cell], "edit me");
                LinearUI::SetNextItemWidth(-FLT_MIN);
                LinearUI::PushID(cell);
                LinearUI::InputText("##cell", text_bufs[cell], LINEARUI_COUNTOF(text_bufs[cell]));
                LinearUI::PopID();
            }
            if (!show_widget_frame_bg)
                LinearUI::PopStyleColor();
            init = false;
            LinearUI::EndTable();
        }
        LinearUI::PopStyleVar();

        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Sizing policies"))
    {
        LINEARUI_DEMO_MARKER("Tables/Explicit widths");
        static LinearUITableFlags flags1 = LinearUITableFlags_BordersV | LinearUITableFlags_BordersOuterH | LinearUITableFlags_RowBg | LinearUITableFlags_ContextMenuInBody;
        PushStyleCompact();
        LinearUI::CheckboxFlags("LinearUITableFlags_Resizable", &flags1, LinearUITableFlags_Resizable);
        LinearUI::CheckboxFlags("LinearUITableFlags_NoHostExtendX", &flags1, LinearUITableFlags_NoHostExtendX);
        PopStyleCompact();

        static LinearUITableFlags sizing_policy_flags[4] = { LinearUITableFlags_SizingFixedFit, LinearUITableFlags_SizingFixedSame, LinearUITableFlags_SizingStretchProp, LinearUITableFlags_SizingStretchSame };
        for (int table_n = 0; table_n < 4; table_n++)
        {
            LinearUI::PushID(table_n);
            LinearUI::SetNextItemWidth(TEXT_BASE_WIDTH * 30);
            EditTableSizingFlags(&sizing_policy_flags[table_n]);

            // To make it easier to understand the different sizing policy,
            // For each policy: we display one table where the columns have equal contents width,
            // and one where the columns have different contents width.
            if (LinearUI::BeginTable("table1", 3, sizing_policy_flags[table_n] | flags1))
            {
                for (int row = 0; row < 3; row++)
                {
                    LinearUI::TableNextRow();
                    LinearUI::TableNextColumn(); LinearUI::Text("Oh dear");
                    LinearUI::TableNextColumn(); LinearUI::Text("Oh dear");
                    LinearUI::TableNextColumn(); LinearUI::Text("Oh dear");
                }
                LinearUI::EndTable();
            }
            if (LinearUI::BeginTable("table2", 3, sizing_policy_flags[table_n] | flags1))
            {
                for (int row = 0; row < 3; row++)
                {
                    LinearUI::TableNextRow();
                    LinearUI::TableNextColumn(); LinearUI::Text("AAAA");
                    LinearUI::TableNextColumn(); LinearUI::Text("BBBBBBBB");
                    LinearUI::TableNextColumn(); LinearUI::Text("CCCCCCCCCCCC");
                }
                LinearUI::EndTable();
            }
            LinearUI::PopID();
        }

        LinearUI::Spacing();
        LinearUI::TextUnformatted("Advanced");
        LinearUI::SameLine();
        HelpMarker(
            "This section allows you to interact and see the effect of various sizing policies "
            "depending on whether Scroll is enabled and the contents of your columns.");

        enum ContentsType { CT_ShowWidth, CT_ShortText, CT_LongText, CT_Button, CT_FillButton, CT_InputText };
        static LinearUITableFlags flags = LinearUITableFlags_ScrollY | LinearUITableFlags_Borders | LinearUITableFlags_RowBg | LinearUITableFlags_Resizable;
        static int contents_type = CT_ShowWidth;
        static int column_count = 3;

        PushStyleCompact();
        LinearUI::PushID("Advanced");
        LinearUI::PushItemWidth(TEXT_BASE_WIDTH * 30);
        EditTableSizingFlags(&flags);
        LinearUI::Combo("Contents", &contents_type, "Show width\0Short Text\0Long Text\0Button\0Fill Button\0InputText\0");
        if (contents_type == CT_FillButton)
        {
            LinearUI::SameLine();
            HelpMarker(
                "Be mindful that using right-alignment (e.g. size.x = -FLT_MIN) creates a feedback loop "
                "where contents width can feed into auto-column width can feed into contents width.");
        }
        LinearUI::DragInt("Columns", &column_count, 0.1f, 1, 64, "%d", LinearUISliderFlags_AlwaysClamp);
        LinearUI::CheckboxFlags("LinearUITableFlags_Resizable", &flags, LinearUITableFlags_Resizable);
        LinearUI::CheckboxFlags("LinearUITableFlags_PreciseWidths", &flags, LinearUITableFlags_PreciseWidths);
        LinearUI::SameLine(); HelpMarker("Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.");
        LinearUI::CheckboxFlags("LinearUITableFlags_ScrollX", &flags, LinearUITableFlags_ScrollX);
        LinearUI::CheckboxFlags("LinearUITableFlags_ScrollY", &flags, LinearUITableFlags_ScrollY);
        LinearUI::CheckboxFlags("LinearUITableFlags_NoClip", &flags, LinearUITableFlags_NoClip);
        LinearUI::PopItemWidth();
        LinearUI::PopID();
        PopStyleCompact();

        if (LinearUI::BeginTable("table2", column_count, flags, LinearVec2(0.0f, TEXT_BASE_HEIGHT * 7)))
        {
            for (int cell = 0; cell < 10 * column_count; cell++)
            {
                LinearUI::TableNextColumn();
                int column = LinearUI::TableGetColumnIndex();
                int row = LinearUI::TableGetRowIndex();

                LinearUI::PushID(cell);
                char label[32];
                static char text_buf[32] = "";
                sprintf(label, "Hello %d,%d", column, row);
                switch (contents_type)
                {
                case CT_ShortText:  LinearUI::TextUnformatted(label); break;
                case CT_LongText:   LinearUI::Text("Some %s text %d,%d\nOver two lines..", column == 0 ? "long" : "longeeer", column, row); break;
                case CT_ShowWidth:  LinearUI::Text("W: %.1f", LinearUI::GetContentRegionAvail().x); break;
                case CT_Button:     LinearUI::Button(label); break;
                case CT_FillButton: LinearUI::Button(label, LinearVec2(-FLT_MIN, 0.0f)); break;
                case CT_InputText:  LinearUI::SetNextItemWidth(-FLT_MIN); LinearUI::InputText("##", text_buf, LINEARUI_COUNTOF(text_buf)); break;
                }
                LinearUI::PopID();
            }
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Vertical scrolling, with clipping"))
    {
        LINEARUI_DEMO_MARKER("Tables/Vertical scrolling, with clipping");
        HelpMarker(
            "Here we activate ScrollY, which will create a child window container to allow hosting scrollable contents.\n\n"
            "We also demonstrate using LinearUIListClipper to virtualize the submission of many items.");
        static LinearUITableFlags flags = LinearUITableFlags_ScrollY | LinearUITableFlags_RowBg | LinearUITableFlags_BordersOuter | LinearUITableFlags_BordersV | LinearUITableFlags_Resizable | LinearUITableFlags_Reorderable | LinearUITableFlags_Hideable;

        PushStyleCompact();
        LinearUI::CheckboxFlags("LinearUITableFlags_ScrollY", &flags, LinearUITableFlags_ScrollY);
        PopStyleCompact();

        // When using ScrollX or ScrollY we need to specify a size for our table container!
        // Otherwise by default the table will fit all available space, like a BeginChild() call.
        LinearVec2 outer_size = LinearVec2(0.0f, TEXT_BASE_HEIGHT * 8);
        if (LinearUI::BeginTable("table_scrolly", 3, flags, outer_size))
        {
            LinearUI::TableSetupScrollFreeze(0, 1); // Make top row always visible
            LinearUI::TableSetupColumn("One", LinearUITableColumnFlags_None);
            LinearUI::TableSetupColumn("Two", LinearUITableColumnFlags_None);
            LinearUI::TableSetupColumn("Three", LinearUITableColumnFlags_None);
            LinearUI::TableHeadersRow();

            // Demonstrate using clipper for large vertical lists
            LinearUIListClipper clipper;
            clipper.Begin(1000);
            while (clipper.Step())
            {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                {
                    LinearUI::TableNextRow();
                    for (int column = 0; column < 3; column++)
                    {
                        LinearUI::TableSetColumnIndex(column);
                        LinearUI::Text("Hello %d,%d", column, row);
                    }
                }
            }
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Horizontal scrolling"))
    {
        LINEARUI_DEMO_MARKER("Tables/Horizontal scrolling");
        HelpMarker(
            "When ScrollX is enabled, the default sizing policy becomes LinearUITableFlags_SizingFixedFit, "
            "as automatically stretching columns doesn't make much sense with horizontal scrolling.\n\n"
            "Also note that as of the current version, you will almost always want to enable ScrollY along with ScrollX, "
            "because the container window won't automatically extend vertically to fix contents "
            "(this may be improved in future versions).");
        static LinearUITableFlags flags = LinearUITableFlags_ScrollX | LinearUITableFlags_ScrollY | LinearUITableFlags_RowBg | LinearUITableFlags_BordersOuter | LinearUITableFlags_BordersV | LinearUITableFlags_Resizable | LinearUITableFlags_Reorderable | LinearUITableFlags_Hideable;
        static int freeze_cols = 1;
        static int freeze_rows = 1;

        PushStyleCompact();
        LinearUI::CheckboxFlags("LinearUITableFlags_Resizable", &flags, LinearUITableFlags_Resizable);
        LinearUI::CheckboxFlags("LinearUITableFlags_ScrollX", &flags, LinearUITableFlags_ScrollX);
        LinearUI::CheckboxFlags("LinearUITableFlags_ScrollY", &flags, LinearUITableFlags_ScrollY);
        LinearUI::SetNextItemWidth(LinearUI::GetFrameHeight());
        LinearUI::DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL, LinearUISliderFlags_NoInput);
        LinearUI::SetNextItemWidth(LinearUI::GetFrameHeight());
        LinearUI::DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL, LinearUISliderFlags_NoInput);
        PopStyleCompact();

        // When using ScrollX or ScrollY we need to specify a size for our table container!
        // Otherwise by default the table will fit all available space, like a BeginChild() call.
        LinearVec2 outer_size = LinearVec2(0.0f, TEXT_BASE_HEIGHT * 8);
        if (LinearUI::BeginTable("table_scrollx", 7, flags, outer_size))
        {
            LinearUI::TableSetupScrollFreeze(freeze_cols, freeze_rows);
            LinearUI::TableSetupColumn("Line #", LinearUITableColumnFlags_NoHide); // Make the first column not hideable to match our use of TableSetupScrollFreeze()
            LinearUI::TableSetupColumn("One");
            LinearUI::TableSetupColumn("Two");
            LinearUI::TableSetupColumn("Three");
            LinearUI::TableSetupColumn("Four");
            LinearUI::TableSetupColumn("Five");
            LinearUI::TableSetupColumn("Six");
            LinearUI::TableHeadersRow();
            for (int row = 0; row < 20; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 7; column++)
                {
                    // Both TableNextColumn() and TableSetColumnIndex() return true when a column is visible or performing width measurement.
                    // Because here we know that:
                    // - A) all our columns are contributing the same to row height
                    // - B) column 0 is always visible,
                    // We only always submit this one column and can skip others.
                    // More advanced per-column clipping behaviors may benefit from polling the status flags via TableGetColumnFlags().
                    if (!LinearUI::TableSetColumnIndex(column) && column > 0)
                        continue;
                    if (column == 0)
                        LinearUI::Text("Line %d", row);
                    else
                        LinearUI::Text("Hello world %d,%d", column, row);
                }
            }
            LinearUI::EndTable();
        }

        LinearUI::Spacing();
        LinearUI::TextUnformatted("Stretch + ScrollX");
        LinearUI::SameLine();
        HelpMarker(
            "Showcase using Stretch columns + ScrollX together: "
            "this is rather unusual and only makes sense when specifying an 'inner_width' for the table!\n"
            "Without an explicit value, inner_width is == outer_size.x and therefore using Stretch columns "
            "along with ScrollX doesn't make sense.");
        static LinearUITableFlags flags2 = LinearUITableFlags_SizingStretchSame | LinearUITableFlags_ScrollX | LinearUITableFlags_ScrollY | LinearUITableFlags_BordersOuter | LinearUITableFlags_RowBg | LinearUITableFlags_ContextMenuInBody;
        static float inner_width = 1000.0f;
        PushStyleCompact();
        LinearUI::PushID("flags3");
        LinearUI::PushItemWidth(TEXT_BASE_WIDTH * 30);
        LinearUI::CheckboxFlags("LinearUITableFlags_ScrollX", &flags2, LinearUITableFlags_ScrollX);
        LinearUI::DragFloat("inner_width", &inner_width, 1.0f, 0.0f, FLT_MAX, "%.1f");
        LinearUI::PopItemWidth();
        LinearUI::PopID();
        PopStyleCompact();
        if (LinearUI::BeginTable("table2", 7, flags2, outer_size, inner_width))
        {
            for (int cell = 0; cell < 20 * 7; cell++)
            {
                LinearUI::TableNextColumn();
                LinearUI::Text("Hello world %d,%d", LinearUI::TableGetColumnIndex(), LinearUI::TableGetRowIndex());
            }
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Columns flags"))
    {
        LINEARUI_DEMO_MARKER("Tables/Columns flags");
        // Create a first table just to show all the options/flags we want to make visible in our example!
        const int column_count = 3;
        const char* column_names[column_count] = { "One", "Two", "Three" };
        static LinearUITableColumnFlags column_flags[column_count] = { LinearUITableColumnFlags_DefaultSort, LinearUITableColumnFlags_None, LinearUITableColumnFlags_DefaultHide };
        static LinearUITableColumnFlags column_flags_out[column_count] = { 0, 0, 0 }; // Output from TableGetColumnFlags()

        if (LinearUI::BeginTable("table_columns_flags_checkboxes", column_count, LinearUITableFlags_None))
        {
            PushStyleCompact();
            for (int column = 0; column < column_count; column++)
            {
                LinearUI::TableNextColumn();
                LinearUI::PushID(column);
                LinearUI::AlignTextToFramePadding(); // FIXME-TABLE: Workaround for wrong text baseline propagation across columns
                LinearUI::Text("'%s'", column_names[column]);
                LinearUI::Spacing();
                LinearUI::Text("Input flags:");
                EditTableColumnsFlags(&column_flags[column]);
                LinearUI::Spacing();
                LinearUI::Text("Output flags:");
                LinearUI::BeginDisabled();
                ShowTableColumnsStatusFlags(column_flags_out[column]);
                LinearUI::EndDisabled();
                LinearUI::PopID();
            }
            PopStyleCompact();
            LinearUI::EndTable();
        }

        // Create the real table we care about for the example!
        // We use a scrolling table to be able to showcase the difference between the _IsEnabled and _IsVisible flags above,
        // otherwise in a non-scrolling table columns are always visible (unless using LinearUITableFlags_NoKeepColumnsVisible
        // + resizing the parent window down).
        const LinearUITableFlags flags
            = LinearUITableFlags_SizingFixedFit | LinearUITableFlags_ScrollX | LinearUITableFlags_ScrollY
            | LinearUITableFlags_RowBg | LinearUITableFlags_BordersOuter | LinearUITableFlags_BordersV
            | LinearUITableFlags_Resizable | LinearUITableFlags_Reorderable | LinearUITableFlags_Hideable | LinearUITableFlags_Sortable;
        LinearVec2 outer_size = LinearVec2(0.0f, TEXT_BASE_HEIGHT * 9);
        if (LinearUI::BeginTable("table_columns_flags", column_count, flags, outer_size))
        {
            bool has_angled_header = false;
            for (int column = 0; column < column_count; column++)
            {
                has_angled_header |= (column_flags[column] & LinearUITableColumnFlags_AngledHeader) != 0;
                LinearUI::TableSetupColumn(column_names[column], column_flags[column]);
            }
            if (has_angled_header)
                LinearUI::TableAngledHeadersRow();
            LinearUI::TableHeadersRow();
            for (int column = 0; column < column_count; column++)
                column_flags_out[column] = LinearUI::TableGetColumnFlags(column);
            float indent_step = (float)((int)TEXT_BASE_WIDTH / 2);
            for (int row = 0; row < 8; row++)
            {
                // Add some indentation to demonstrate usage of per-column IndentEnable/IndentDisable flags.
                LinearUI::Indent(indent_step);
                LinearUI::TableNextRow();
                for (int column = 0; column < column_count; column++)
                {
                    LinearUI::TableSetColumnIndex(column);
                    LinearUI::Text("%s %s", (column == 0) ? "Indented" : "Hello", LinearUI::TableGetColumnName(column));
                }
            }
            LinearUI::Unindent(indent_step * 8.0f);

            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Columns widths"))
    {
        LINEARUI_DEMO_MARKER("Tables/Columns widths");
        HelpMarker("Using TableSetupColumn() to setup default width.");

        static LinearUITableFlags flags1 = LinearUITableFlags_Borders | LinearUITableFlags_NoBordersInBodyUntilResize;
        PushStyleCompact();
        LinearUI::CheckboxFlags("LinearUITableFlags_Resizable", &flags1, LinearUITableFlags_Resizable);
        LinearUI::CheckboxFlags("LinearUITableFlags_NoBordersInBodyUntilResize", &flags1, LinearUITableFlags_NoBordersInBodyUntilResize);
        PopStyleCompact();
        if (LinearUI::BeginTable("table1", 3, flags1))
        {
            // We could also set LinearUITableFlags_SizingFixedFit on the table and all columns will default to LinearUITableColumnFlags_WidthFixed.
            LinearUI::TableSetupColumn("one", LinearUITableColumnFlags_WidthFixed, 100.0f); // Default to 100.0f
            LinearUI::TableSetupColumn("two", LinearUITableColumnFlags_WidthFixed, 200.0f); // Default to 200.0f
            LinearUI::TableSetupColumn("three", LinearUITableColumnFlags_WidthFixed);       // Default to auto
            LinearUI::TableHeadersRow();
            for (int row = 0; row < 4; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    LinearUI::TableSetColumnIndex(column);
                    if (row == 0)
                        LinearUI::Text("(w: %5.1f)", LinearUI::GetContentRegionAvail().x);
                    else
                        LinearUI::Text("Hello %d,%d", column, row);
                }
            }
            LinearUI::EndTable();
        }

        HelpMarker(
            "Using TableSetupColumn() to setup explicit width.\n\nUnless _NoKeepColumnsVisible is set, "
            "fixed columns with set width may still be shrunk down if there's not enough space in the host.");

        static LinearUITableFlags flags2 = LinearUITableFlags_None;
        PushStyleCompact();
        LinearUI::CheckboxFlags("LinearUITableFlags_NoKeepColumnsVisible", &flags2, LinearUITableFlags_NoKeepColumnsVisible);
        LinearUI::CheckboxFlags("LinearUITableFlags_BordersInnerV", &flags2, LinearUITableFlags_BordersInnerV);
        LinearUI::CheckboxFlags("LinearUITableFlags_BordersOuterV", &flags2, LinearUITableFlags_BordersOuterV);
        PopStyleCompact();
        if (LinearUI::BeginTable("table2", 4, flags2))
        {
            // We could also set LinearUITableFlags_SizingFixedFit on the table and then all columns
            // will default to LinearUITableColumnFlags_WidthFixed.
            LinearUI::TableSetupColumn("", LinearUITableColumnFlags_WidthFixed, 100.0f);
            LinearUI::TableSetupColumn("", LinearUITableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 15.0f);
            LinearUI::TableSetupColumn("", LinearUITableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 30.0f);
            LinearUI::TableSetupColumn("", LinearUITableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 15.0f);
            for (int row = 0; row < 5; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 4; column++)
                {
                    LinearUI::TableSetColumnIndex(column);
                    if (row == 0)
                        LinearUI::Text("(w: %5.1f)", LinearUI::GetContentRegionAvail().x);
                    else
                        LinearUI::Text("Hello %d,%d", column, row);
                }
            }
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Nested tables"))
    {
        LINEARUI_DEMO_MARKER("Tables/Nested tables");
        HelpMarker("This demonstrates embedding a table into another table cell.");

        if (LinearUI::BeginTable("table_nested1", 2, LinearUITableFlags_Borders | LinearUITableFlags_Resizable | LinearUITableFlags_Reorderable | LinearUITableFlags_Hideable))
        {
            LinearUI::TableSetupColumn("A0");
            LinearUI::TableSetupColumn("A1");
            LinearUI::TableHeadersRow();

            LinearUI::TableNextColumn();
            LinearUI::Text("A0 Row 0");
            {
                float rows_height = (TEXT_BASE_HEIGHT * 2.0f) + (LinearUI::GetStyle().CellPadding.y * 2.0f);
                if (LinearUI::BeginTable("table_nested2", 2, LinearUITableFlags_Borders | LinearUITableFlags_Resizable | LinearUITableFlags_Reorderable | LinearUITableFlags_Hideable))
                {
                    LinearUI::TableSetupColumn("B0");
                    LinearUI::TableSetupColumn("B1");
                    LinearUI::TableHeadersRow();

                    LinearUI::TableNextRow(LinearUITableRowFlags_None, rows_height);
                    LinearUI::TableNextColumn();
                    LinearUI::Text("B0 Row 0");
                    LinearUI::TableNextColumn();
                    LinearUI::Text("B1 Row 0");
                    LinearUI::TableNextRow(LinearUITableRowFlags_None, rows_height);
                    LinearUI::TableNextColumn();
                    LinearUI::Text("B0 Row 1");
                    LinearUI::TableNextColumn();
                    LinearUI::Text("B1 Row 1");

                    LinearUI::EndTable();
                }
            }
            LinearUI::TableNextColumn(); LinearUI::Text("A1 Row 0");
            LinearUI::TableNextColumn(); LinearUI::Text("A0 Row 1");
            LinearUI::TableNextColumn(); LinearUI::Text("A1 Row 1");
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Row height"))
    {
        LINEARUI_DEMO_MARKER("Tables/Row height");
        HelpMarker(
            "You can pass a 'min_row_height' to TableNextRow().\n\nRows are padded with 'style.CellPadding.y' on top and bottom, "
            "so effectively the minimum row height will always be >= 'style.CellPadding.y * 2.0f'.\n\n"
            "We cannot honor a _maximum_ row height as that would require a unique clipping rectangle per row.");
        if (LinearUI::BeginTable("table_row_height", 1, LinearUITableFlags_Borders))
        {
            for (int row = 0; row < 8; row++)
            {
                float min_row_height = (float)(int)(TEXT_BASE_HEIGHT * 0.30f * row + LinearUI::GetStyle().CellPadding.y * 2.0f);
                LinearUI::TableNextRow(LinearUITableRowFlags_None, min_row_height);
                LinearUI::TableNextColumn();
                LinearUI::Text("min_row_height = %.2f", min_row_height);
            }
            LinearUI::EndTable();
        }

        HelpMarker(
            "Showcase using SameLine(0,0) to share Current Line Height between cells.\n\n"
            "Please note that Tables Row Height is not the same thing as Current Line Height, "
            "as a table cell may contains multiple lines.");
        if (LinearUI::BeginTable("table_share_lineheight", 2, LinearUITableFlags_Borders))
        {
            LinearUI::TableNextRow();
            LinearUI::TableNextColumn();
            LinearUI::ColorButton("##1", LinearVec4(0.13f, 0.26f, 0.40f, 1.0f), LinearUIColorEditFlags_None, LinearVec2(40, 40));
            LinearUI::TableNextColumn();
            LinearUI::Text("Line 1");
            LinearUI::Text("Line 2");

            LinearUI::TableNextRow();
            LinearUI::TableNextColumn();
            LinearUI::ColorButton("##2", LinearVec4(0.13f, 0.26f, 0.40f, 1.0f), LinearUIColorEditFlags_None, LinearVec2(40, 40));
            LinearUI::TableNextColumn();
            LinearUI::SameLine(0.0f, 0.0f); // Reuse line height from previous column
            LinearUI::Text("Line 1, with SameLine(0,0)");
            LinearUI::Text("Line 2");

            LinearUI::EndTable();
        }

        HelpMarker("Showcase altering CellPadding.y between rows. Note that CellPadding.x is locked for the entire table.");
        if (LinearUI::BeginTable("table_changing_cellpadding_y", 1, LinearUITableFlags_Borders))
        {
            LinearUIStyle& style = LinearUI::GetStyle();
            for (int row = 0; row < 8; row++)
            {
                if ((row % 3) == 2)
                    LinearUI::PushStyleVarY(LinearUIStyleVar_CellPadding, 20.0f);
                LinearUI::TableNextRow(LinearUITableRowFlags_None);
                LinearUI::TableNextColumn();
                LinearUI::Text("CellPadding.y = %.2f", style.CellPadding.y);
                if ((row % 3) == 2)
                    LinearUI::PopStyleVar();
            }
            LinearUI::EndTable();
        }

        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Outer size"))
    {
        LINEARUI_DEMO_MARKER("Tables/Outer size");
        // Showcasing use of LinearUITableFlags_NoHostExtendX and LinearUITableFlags_NoHostExtendY
        // Important to that note how the two flags have slightly different behaviors!
        LinearUI::Text("Using NoHostExtendX and NoHostExtendY:");
        PushStyleCompact();
        static LinearUITableFlags flags = LinearUITableFlags_Borders | LinearUITableFlags_Resizable | LinearUITableFlags_ContextMenuInBody | LinearUITableFlags_RowBg | LinearUITableFlags_SizingFixedFit | LinearUITableFlags_NoHostExtendX;
        LinearUI::CheckboxFlags("LinearUITableFlags_NoHostExtendX", &flags, LinearUITableFlags_NoHostExtendX);
        LinearUI::SameLine(); HelpMarker("Make outer width auto-fit to columns, overriding outer_size.x value.\n\nOnly available when ScrollX/ScrollY are disabled and Stretch columns are not used.");
        LinearUI::CheckboxFlags("LinearUITableFlags_NoHostExtendY", &flags, LinearUITableFlags_NoHostExtendY);
        LinearUI::SameLine(); HelpMarker("Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit).\n\nOnly available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.");
        PopStyleCompact();

        LinearVec2 outer_size = LinearVec2(0.0f, TEXT_BASE_HEIGHT * 5.5f);
        if (LinearUI::BeginTable("table1", 3, flags, outer_size))
        {
            for (int row = 0; row < 10; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    LinearUI::TableNextColumn();
                    LinearUI::Text("Cell %d,%d", column, row);
                }
            }
            LinearUI::EndTable();
        }
        LinearUI::SameLine();
        LinearUI::Text("Hello!");

        LinearUI::Spacing();

        LinearUI::Text("Using explicit size:");
        if (LinearUI::BeginTable("table2", 3, LinearUITableFlags_Borders | LinearUITableFlags_RowBg, LinearVec2(TEXT_BASE_WIDTH * 30, 0.0f)))
        {
            for (int row = 0; row < 5; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    LinearUI::TableNextColumn();
                    LinearUI::Text("Cell %d,%d", column, row);
                }
            }
            LinearUI::EndTable();
        }
        LinearUI::SameLine();
        if (LinearUI::BeginTable("table3", 3, LinearUITableFlags_Borders | LinearUITableFlags_RowBg, LinearVec2(TEXT_BASE_WIDTH * 30, 0.0f)))
        {
            const float rows_height = TEXT_BASE_HEIGHT * 1.5f + LinearUI::GetStyle().CellPadding.y * 2.0f;
            for (int row = 0; row < 3; row++)
            {
                LinearUI::TableNextRow(0, rows_height);
                for (int column = 0; column < 3; column++)
                {
                    LinearUI::TableNextColumn();
                    LinearUI::Text("Cell %d,%d", column, row);
                }
            }
            LinearUI::EndTable();
        }

        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Background color"))
    {
        LINEARUI_DEMO_MARKER("Tables/Background color");
        static LinearUITableFlags flags = LinearUITableFlags_RowBg;
        static int row_bg_type = 1;
        static int row_bg_target = 1;
        static int cell_bg_type = 1;

        PushStyleCompact();
        LinearUI::CheckboxFlags("LinearUITableFlags_Borders", &flags, LinearUITableFlags_Borders);
        LinearUI::CheckboxFlags("LinearUITableFlags_RowBg", &flags, LinearUITableFlags_RowBg);
        LinearUI::SameLine(); HelpMarker("LinearUITableFlags_RowBg automatically sets RowBg0 to alternative colors pulled from the Style.");
        LinearUI::Combo("row bg type", (int*)&row_bg_type, "None\0Red\0Gradient\0");
        LinearUI::Combo("row bg target", (int*)&row_bg_target, "RowBg0\0RowBg1\0"); LinearUI::SameLine(); HelpMarker("Target RowBg0 to override the alternating odd/even colors,\nTarget RowBg1 to blend with them.");
        LinearUI::Combo("cell bg type", (int*)&cell_bg_type, "None\0Blue\0"); LinearUI::SameLine(); HelpMarker("We are colorizing cells to B1->C2 here.");
        LINEARUI_ASSERT(row_bg_type >= 0 && row_bg_type <= 2);
        LINEARUI_ASSERT(row_bg_target >= 0 && row_bg_target <= 1);
        LINEARUI_ASSERT(cell_bg_type >= 0 && cell_bg_type <= 1);
        PopStyleCompact();

        if (LinearUI::BeginTable("table1", 5, flags))
        {
            for (int row = 0; row < 6; row++)
            {
                LinearUI::TableNextRow();

                // Demonstrate setting a row background color with 'LinearUI::TableSetBgColor(LinearUITableBgTarget_RowBgX, ...)'
                // We use a transparent color so we can see the one behind in case our target is RowBg1 and RowBg0 was already targeted by the LinearUITableFlags_RowBg flag.
                if (row_bg_type != 0)
                {
                    LinearU32 row_bg_color = LinearUI::GetColorU32(row_bg_type == 1 ? LinearVec4(0.7f, 0.3f, 0.3f, 0.65f) : LinearVec4(0.2f + row * 0.1f, 0.2f, 0.2f, 0.65f)); // Flat or Gradient?
                    LinearUI::TableSetBgColor(LinearUITableBgTarget_RowBg0 + row_bg_target, row_bg_color);
                }

                // Fill cells
                for (int column = 0; column < 5; column++)
                {
                    LinearUI::TableSetColumnIndex(column);
                    LinearUI::Text("%c%c", 'A' + row, '0' + column);

                    // Change background of Cells B1->C2
                    // Demonstrate setting a cell background color with 'LinearUI::TableSetBgColor(LinearUITableBgTarget_CellBg, ...)'
                    // (the CellBg color will be blended over the RowBg and ColumnBg colors)
                    // We can also pass a column number as a third parameter to TableSetBgColor() and do this outside the column loop.
                    if (row >= 1 && row <= 2 && column >= 1 && column <= 2 && cell_bg_type == 1)
                    {
                        LinearU32 cell_bg_color = LinearUI::GetColorU32(LinearVec4(0.3f, 0.3f, 0.7f, 0.65f));
                        LinearUI::TableSetBgColor(LinearUITableBgTarget_CellBg, cell_bg_color);
                    }
                }
            }
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Tree view"))
    {
        LINEARUI_DEMO_MARKER("Tables/Tree view");
        static LinearUITableFlags table_flags = LinearUITableFlags_BordersV | LinearUITableFlags_BordersOuterH | LinearUITableFlags_Resizable | LinearUITableFlags_RowBg | LinearUITableFlags_NoBordersInBody;

        static LinearUITreeNodeFlags tree_node_flags_base = LinearUITreeNodeFlags_SpanAllColumns | LinearUITreeNodeFlags_DefaultOpen | LinearUITreeNodeFlags_DrawLinesFull;
        LinearUI::CheckboxFlags("LinearUITreeNodeFlags_SpanFullWidth",  &tree_node_flags_base, LinearUITreeNodeFlags_SpanFullWidth);
        LinearUI::CheckboxFlags("LinearUITreeNodeFlags_SpanLabelWidth",  &tree_node_flags_base, LinearUITreeNodeFlags_SpanLabelWidth);
        LinearUI::CheckboxFlags("LinearUITreeNodeFlags_SpanAllColumns", &tree_node_flags_base, LinearUITreeNodeFlags_SpanAllColumns);
        LinearUI::CheckboxFlags("LinearUITreeNodeFlags_LabelSpanAllColumns", &tree_node_flags_base, LinearUITreeNodeFlags_LabelSpanAllColumns);
        LinearUI::SameLine(); HelpMarker("Useful if you know that you aren't displaying contents in other columns");

        HelpMarker("See \"Columns flags\" section to configure how indentation is applied to individual columns.");
        if (LinearUI::BeginTable("3ways", 3, table_flags))
        {
            // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
            LinearUI::TableSetupColumn("Name", LinearUITableColumnFlags_NoHide);
            LinearUI::TableSetupColumn("Size", LinearUITableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
            LinearUI::TableSetupColumn("Type", LinearUITableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
            LinearUI::TableHeadersRow();

            // Simple storage to output a dummy file-system.
            struct MyTreeNode
            {
                const char*     Name;
                const char*     Type;
                int             Size;
                int             ChildIdx;
                int             ChildCount;
                static void DisplayNode(const MyTreeNode* node, const MyTreeNode* all_nodes)
                {
                    LinearUI::TableNextRow();
                    LinearUI::TableNextColumn();
                    const bool is_folder = (node->ChildCount > 0);

                    LinearUITreeNodeFlags node_flags = tree_node_flags_base;
                    if (node != &all_nodes[0])
                        node_flags &= ~LinearUITreeNodeFlags_LabelSpanAllColumns; // Only demonstrate this on the root node.

                    if (is_folder)
                    {
                        bool open = LinearUI::TreeNodeEx(node->Name, node_flags);
                        if ((node_flags & LinearUITreeNodeFlags_LabelSpanAllColumns) == 0)
                        {
                            LinearUI::TableNextColumn();
                            LinearUI::TextDisabled("--");
                            LinearUI::TableNextColumn();
                            LinearUI::TextUnformatted(node->Type);
                        }
                        if (open)
                        {
                            for (int child_n = 0; child_n < node->ChildCount; child_n++)
                                DisplayNode(&all_nodes[node->ChildIdx + child_n], all_nodes);
                            LinearUI::TreePop();
                        }
                    }
                    else
                    {
                        LinearUI::TreeNodeEx(node->Name, node_flags | LinearUITreeNodeFlags_Leaf | LinearUITreeNodeFlags_Bullet | LinearUITreeNodeFlags_NoTreePushOnOpen);
                        LinearUI::TableNextColumn();
                        LinearUI::Text("%d", node->Size);
                        LinearUI::TableNextColumn();
                        LinearUI::TextUnformatted(node->Type);
                    }
                }
            };
            static const MyTreeNode nodes[] =
            {
                { "Root with Long Name",          "Folder",       -1,       1, 3    }, // 0
                { "Music",                        "Folder",       -1,       4, 2    }, // 1
                { "Textures",                     "Folder",       -1,       6, 3    }, // 2
                { "desktop.ini",                  "System file",  1024,    -1,-1    }, // 3
                { "File1_a.wav",                  "Audio file",   123000,  -1,-1    }, // 4
                { "File1_b.wav",                  "Audio file",   456000,  -1,-1    }, // 5
                { "Image001.png",                 "Image file",   203128,  -1,-1    }, // 6
                { "Copy of Image001.png",         "Image file",   203256,  -1,-1    }, // 7
                { "Copy of Image001 (Final2).png","Image file",   203512,  -1,-1    }, // 8
            };

            MyTreeNode::DisplayNode(&nodes[0], nodes);

            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Item width"))
    {
        LINEARUI_DEMO_MARKER("Tables/Item width");
        HelpMarker(
            "Showcase using PushItemWidth() and how it is preserved on a per-column basis.\n\n"
            "Note that on auto-resizing non-resizable fixed columns, querying the content width for "
            "e.g. right-alignment doesn't make sense.");
        if (LinearUI::BeginTable("table_item_width", 3, LinearUITableFlags_Borders))
        {
            LinearUI::TableSetupColumn("small");
            LinearUI::TableSetupColumn("half");
            LinearUI::TableSetupColumn("right-align");
            LinearUI::TableHeadersRow();

            for (int row = 0; row < 3; row++)
            {
                LinearUI::TableNextRow();
                if (row == 0)
                {
                    // Setup ItemWidth once (instead of setting up every time, which is also possible but less efficient)
                    LinearUI::TableSetColumnIndex(0);
                    LinearUI::PushItemWidth(TEXT_BASE_WIDTH * 3.0f); // Small
                    LinearUI::TableSetColumnIndex(1);
                    LinearUI::PushItemWidth(-LinearUI::GetContentRegionAvail().x * 0.5f);
                    LinearUI::TableSetColumnIndex(2);
                    LinearUI::PushItemWidth(-FLT_MIN); // Right-aligned
                }

                // Draw our contents
                static float dummy_f = 0.0f;
                LinearUI::PushID(row);
                LinearUI::TableSetColumnIndex(0);
                LinearUI::SliderFloat("float0", &dummy_f, 0.0f, 1.0f);
                LinearUI::TableSetColumnIndex(1);
                LinearUI::SliderFloat("float1", &dummy_f, 0.0f, 1.0f);
                LinearUI::TableSetColumnIndex(2);
                LinearUI::SliderFloat("##float2", &dummy_f, 0.0f, 1.0f); // No visible label since right-aligned
                LinearUI::PopID();
            }
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    // Demonstrate using TableHeader() calls instead of TableHeadersRow()
    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Custom headers"))
    {
        LINEARUI_DEMO_MARKER("Tables/Custom headers");
        const int COLUMNS_COUNT = 3;
        if (LinearUI::BeginTable("table_custom_headers", COLUMNS_COUNT, LinearUITableFlags_Borders | LinearUITableFlags_Reorderable | LinearUITableFlags_Hideable))
        {
            LinearUI::TableSetupColumn("Apricot");
            LinearUI::TableSetupColumn("Banana");
            LinearUI::TableSetupColumn("Cherry");

            // Dummy entire-column selection storage
            // FIXME: It would be nice to actually demonstrate full-featured selection using those checkbox.
            static bool column_selected[3] = {};

            // Instead of calling TableHeadersRow() we'll submit custom headers ourselves.
            // (A different approach is also possible:
            //    - Specify LinearUITableColumnFlags_NoHeaderLabel in some TableSetupColumn() call.
            //    - Call TableHeadersRow() normally. This will submit TableHeader() with no name.
            //    - Then call TableSetColumnIndex() to position yourself in the column and submit your stuff e.g. Checkbox().)
            LinearUI::TableNextRow(LinearUITableRowFlags_Headers);
            for (int column = 0; column < COLUMNS_COUNT; column++)
            {
                LinearUI::TableSetColumnIndex(column);
                const char* column_name = LinearUI::TableGetColumnName(column); // Retrieve name passed to TableSetupColumn()
                LinearUI::PushID(column);
                LinearUI::PushStyleVar(LinearUIStyleVar_FramePadding, LinearVec2(0, 0));
                LinearUI::Checkbox("##checkall", &column_selected[column]);
                LinearUI::PopStyleVar();
                LinearUI::SameLine(0.0f, LinearUI::GetStyle().ItemInnerSpacing.x);
                LinearUI::TableHeader(column_name);
                LinearUI::PopID();
            }

            // Submit table contents
            for (int row = 0; row < 5; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    char buf[32];
                    sprintf(buf, "Cell %d,%d", column, row);
                    LinearUI::TableSetColumnIndex(column);
                    LinearUI::Selectable(buf, column_selected[column]);
                }
            }
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    // Demonstrate using LinearUITableColumnFlags_AngledHeader flag to create angled headers
    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Angled headers"))
    {
        LINEARUI_DEMO_MARKER("Tables/Angled headers");
        const char* column_names[] = { "Track", "cabasa", "ride", "smash", "tom-hi", "tom-mid", "tom-low", "hihat-o", "hihat-c", "snare-s", "snare-c", "clap", "rim", "kick" };
        const int columns_count = LINEARUI_COUNTOF(column_names);
        const int rows_count = 12;

        static LinearUITableFlags table_flags = LinearUITableFlags_SizingFixedFit | LinearUITableFlags_ScrollX | LinearUITableFlags_ScrollY | LinearUITableFlags_BordersOuter | LinearUITableFlags_BordersInnerH | LinearUITableFlags_Hideable | LinearUITableFlags_Resizable | LinearUITableFlags_Reorderable | LinearUITableFlags_HighlightHoveredColumn;
        static LinearUITableColumnFlags column_flags = LinearUITableColumnFlags_AngledHeader | LinearUITableColumnFlags_WidthFixed;
        static bool bools[columns_count * rows_count] = {}; // Dummy storage selection storage
        static int frozen_cols = 1;
        static int frozen_rows = 2;
        LinearUI::CheckboxFlags("_ScrollX", &table_flags, LinearUITableFlags_ScrollX);
        LinearUI::CheckboxFlags("_ScrollY", &table_flags, LinearUITableFlags_ScrollY);
        LinearUI::CheckboxFlags("_Resizable", &table_flags, LinearUITableFlags_Resizable);
        LinearUI::CheckboxFlags("_Sortable", &table_flags, LinearUITableFlags_Sortable);
        LinearUI::CheckboxFlags("_NoBordersInBody", &table_flags, LinearUITableFlags_NoBordersInBody);
        LinearUI::CheckboxFlags("_HighlightHoveredColumn", &table_flags, LinearUITableFlags_HighlightHoveredColumn);
        LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 8);
        LinearUI::SliderInt("Frozen columns", &frozen_cols, 0, 2);
        LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 8);
        LinearUI::SliderInt("Frozen rows", &frozen_rows, 0, 2);
        LinearUI::CheckboxFlags("Disable header contributing to column width", &column_flags, LinearUITableColumnFlags_NoHeaderWidth);

        if (LinearUI::TreeNode("Style settings"))
        {
            LinearUI::SameLine();
            HelpMarker("Giving access to some LinearUIStyle value in this demo for convenience.");
            LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 8);
            LinearUI::SliderAngle("style.TableAngledHeadersAngle", &LinearUI::GetStyle().TableAngledHeadersAngle, -50.0f, +50.0f);
            LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 8);
            LinearUI::SliderFloat2("style.TableAngledHeadersTextAlign", (float*)&LinearUI::GetStyle().TableAngledHeadersTextAlign, 0.0f, 1.0f, "%.2f");
            LinearUI::TreePop();
        }

        if (LinearUI::BeginTable("table_angled_headers", columns_count, table_flags, LinearVec2(0.0f, TEXT_BASE_HEIGHT * 12)))
        {
            LinearUI::TableSetupColumn(column_names[0], LinearUITableColumnFlags_NoHide | LinearUITableColumnFlags_NoReorder);
            for (int n = 1; n < columns_count; n++)
                LinearUI::TableSetupColumn(column_names[n], column_flags);
            LinearUI::TableSetupScrollFreeze(frozen_cols, frozen_rows);

            LinearUI::TableAngledHeadersRow(); // Draw angled headers for all columns with the LinearUITableColumnFlags_AngledHeader flag.
            LinearUI::TableHeadersRow();       // Draw remaining headers and allow access to context-menu and other functions.
            for (int row = 0; row < rows_count; row++)
            {
                LinearUI::PushID(row);
                LinearUI::TableNextRow();
                LinearUI::TableSetColumnIndex(0);
                LinearUI::AlignTextToFramePadding();
                LinearUI::Text("Track %d", row);
                for (int column = 1; column < columns_count; column++)
                    if (LinearUI::TableSetColumnIndex(column))
                    {
                        LinearUI::PushID(column);
                        LinearUI::Checkbox("", &bools[row * columns_count + column]);
                        LinearUI::PopID();
                    }
                LinearUI::PopID();
            }
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    // Demonstrate creating custom context menus inside columns,
    // while playing it nice with context menus provided by TableHeadersRow()/TableHeader()
    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Context menus"))
    {
        LINEARUI_DEMO_MARKER("Tables/Context menus");
        HelpMarker(
            "By default, right-clicking over a TableHeadersRow()/TableHeader() line will open the default context-menu.\n"
            "Using LinearUITableFlags_ContextMenuInBody we also allow right-clicking over columns body.");
        static LinearUITableFlags flags1 = LinearUITableFlags_Resizable | LinearUITableFlags_Reorderable | LinearUITableFlags_Hideable | LinearUITableFlags_Borders | LinearUITableFlags_ContextMenuInBody;

        PushStyleCompact();
        LinearUI::CheckboxFlags("LinearUITableFlags_ContextMenuInBody", &flags1, LinearUITableFlags_ContextMenuInBody);
        PopStyleCompact();

        // Context Menus: first example
        // [1.1] Right-click on the TableHeadersRow() line to open the default table context menu.
        // [1.2] Right-click in columns also open the default table context menu (if LinearUITableFlags_ContextMenuInBody is set)
        const int COLUMNS_COUNT = 3;
        if (LinearUI::BeginTable("table_context_menu", COLUMNS_COUNT, flags1))
        {
            LinearUI::TableSetupColumn("One");
            LinearUI::TableSetupColumn("Two");
            LinearUI::TableSetupColumn("Three");

            // [1.1]] Right-click on the TableHeadersRow() line to open the default table context menu.
            LinearUI::TableHeadersRow();

            // Submit dummy contents
            for (int row = 0; row < 4; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < COLUMNS_COUNT; column++)
                {
                    LinearUI::TableSetColumnIndex(column);
                    LinearUI::Text("Cell %d,%d", column, row);
                }
            }
            LinearUI::EndTable();
        }

        // Context Menus: second example
        // [2.1] Right-click on the TableHeadersRow() line to open the default table context menu.
        // [2.2] Right-click on the ".." to open a custom popup
        // [2.3] Right-click in columns to open another custom popup
        HelpMarker(
            "Demonstrate mixing table context menu (over header), item context button (over button) "
            "and custom per-column context menu (over column body).");
        LinearUITableFlags flags2 = LinearUITableFlags_Resizable | LinearUITableFlags_SizingFixedFit | LinearUITableFlags_Reorderable | LinearUITableFlags_Hideable | LinearUITableFlags_Borders;
        if (LinearUI::BeginTable("table_context_menu_2", COLUMNS_COUNT, flags2))
        {
            LinearUI::TableSetupColumn("One");
            LinearUI::TableSetupColumn("Two");
            LinearUI::TableSetupColumn("Three");

            // [2.1] Right-click on the TableHeadersRow() line to open the default table context menu.
            LinearUI::TableHeadersRow();
            for (int row = 0; row < 4; row++)
            {
                LinearUI::TableNextRow();
                for (int column = 0; column < COLUMNS_COUNT; column++)
                {
                    // Submit dummy contents
                    LinearUI::TableSetColumnIndex(column);
                    LinearUI::Text("Cell %d,%d", column, row);
                    LinearUI::SameLine();

                    // [2.2] Right-click on the ".." to open a custom popup
                    LinearUI::PushID(row * COLUMNS_COUNT + column);
                    LinearUI::SmallButton("..");
                    if (LinearUI::BeginPopupContextItem())
                    {
                        LinearUI::Text("This is the popup for Button(\"..\") in Cell %d,%d", column, row);
                        if (LinearUI::Button("Close"))
                            LinearUI::CloseCurrentPopup();
                        LinearUI::EndPopup();
                    }
                    LinearUI::PopID();
                }
            }

            // [2.3] Right-click anywhere in columns to open another custom popup
            // (instead of testing for !IsAnyItemHovered() we could also call OpenPopup() with LinearUIPopupFlags_NoOpenOverExistingPopup
            // to manage popup priority as the popups triggers, here "are we hovering a column" are overlapping)
            int hovered_column = -1;
            for (int column = 0; column < COLUMNS_COUNT + 1; column++)
            {
                LinearUI::PushID(column);
                if (LinearUI::TableGetColumnFlags(column) & LinearUITableColumnFlags_IsHovered)
                    hovered_column = column;
                if (hovered_column == column && !LinearUI::IsAnyItemHovered() && LinearUI::IsMouseReleased(1))
                    LinearUI::OpenPopup("MyPopup");
                if (LinearUI::BeginPopup("MyPopup"))
                {
                    if (column == COLUMNS_COUNT)
                        LinearUI::Text("This is a custom popup for unused space after the last column.");
                    else
                        LinearUI::Text("This is a custom popup for Column %d", column);
                    if (LinearUI::Button("Close"))
                        LinearUI::CloseCurrentPopup();
                    LinearUI::EndPopup();
                }
                LinearUI::PopID();
            }

            LinearUI::EndTable();
            LinearUI::Text("Hovered column: %d", hovered_column);
        }
        LinearUI::TreePop();
    }

    // Demonstrate creating multiple tables with the same ID
    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Synced instances"))
    {
        LINEARUI_DEMO_MARKER("Tables/Synced instances");
        HelpMarker("Multiple tables with the same identifier will share their settings, width, visibility, order etc.");

        static LinearUITableFlags flags = LinearUITableFlags_Resizable | LinearUITableFlags_Reorderable | LinearUITableFlags_Hideable | LinearUITableFlags_Borders | LinearUITableFlags_SizingFixedFit | LinearUITableFlags_NoSavedSettings;
        LinearUI::CheckboxFlags("LinearUITableFlags_Resizable", &flags, LinearUITableFlags_Resizable);
        LinearUI::CheckboxFlags("LinearUITableFlags_ScrollY", &flags, LinearUITableFlags_ScrollY);
        LinearUI::CheckboxFlags("LinearUITableFlags_SizingFixedFit", &flags, LinearUITableFlags_SizingFixedFit);
        LinearUI::CheckboxFlags("LinearUITableFlags_HighlightHoveredColumn", &flags, LinearUITableFlags_HighlightHoveredColumn);
        for (int n = 0; n < 3; n++)
        {
            char buf[32];
            sprintf(buf, "Synced Table %d", n);
            bool open = LinearUI::CollapsingHeader(buf, LinearUITreeNodeFlags_DefaultOpen);
            if (open && LinearUI::BeginTable("Table", 3, flags, LinearVec2(0.0f, LinearUI::GetTextLineHeightWithSpacing() * 5)))
            {
                LinearUI::TableSetupColumn("One");
                LinearUI::TableSetupColumn("Two");
                LinearUI::TableSetupColumn("Three");
                LinearUI::TableHeadersRow();
                const int cell_count = (n == 1) ? 27 : 9; // Make second table have a scrollbar to verify that additional decoration is not affecting column positions.
                for (int cell = 0; cell < cell_count; cell++)
                {
                    LinearUI::TableNextColumn();
                    LinearUI::Text("this cell %d", cell);
                }
                LinearUI::EndTable();
            }
        }
        LinearUI::TreePop();
    }

    // Demonstrate using Sorting facilities
    // This is a simplified version of the "Advanced" example, where we mostly focus on the code necessary to handle sorting.
    // Note that the "Advanced" example also showcase manually triggering a sort (e.g. if item quantities have been modified)
    static const char* template_items_names[] =
    {
        "Banana", "Apple", "Cherry", "Watermelon", "Grapefruit", "Strawberry", "Mango",
        "Kiwi", "Orange", "Pineapple", "Blueberry", "Plum", "Coconut", "Pear", "Apricot"
    };
    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Sorting"))
    {
        LINEARUI_DEMO_MARKER("Tables/Sorting");
        // Create item list
        static LinearVector<MyItem> items;
        if (items.Size == 0)
        {
            items.resize(50, MyItem());
            for (int n = 0; n < items.Size; n++)
            {
                const int template_n = n % LINEARUI_COUNTOF(template_items_names);
                MyItem& item = items[n];
                item.ID = n;
                item.Name = template_items_names[template_n];
                item.Quantity = (n * n - n) % 20; // Assign default quantities
            }
        }

        // Options
        static LinearUITableFlags flags =
            LinearUITableFlags_Resizable | LinearUITableFlags_Reorderable | LinearUITableFlags_Hideable | LinearUITableFlags_Sortable | LinearUITableFlags_SortMulti
            | LinearUITableFlags_RowBg | LinearUITableFlags_BordersOuter | LinearUITableFlags_BordersV | LinearUITableFlags_NoBordersInBody
            | LinearUITableFlags_ScrollY;
        PushStyleCompact();
        LinearUI::CheckboxFlags("LinearUITableFlags_SortMulti", &flags, LinearUITableFlags_SortMulti);
        LinearUI::SameLine(); HelpMarker("When sorting is enabled: hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).");
        LinearUI::CheckboxFlags("LinearUITableFlags_SortTristate", &flags, LinearUITableFlags_SortTristate);
        LinearUI::SameLine(); HelpMarker("When sorting is enabled: allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).");
        PopStyleCompact();

        if (LinearUI::BeginTable("table_sorting", 4, flags, LinearVec2(0.0f, TEXT_BASE_HEIGHT * 15), 0.0f))
        {
            // Declare columns
            // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
            // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
            // Demonstrate using a mixture of flags among available sort-related flags:
            // - LinearUITableColumnFlags_DefaultSort
            // - LinearUITableColumnFlags_NoSort / LinearUITableColumnFlags_NoSortAscending / LinearUITableColumnFlags_NoSortDescending
            // - LinearUITableColumnFlags_PreferSortAscending / LinearUITableColumnFlags_PreferSortDescending
            LinearUI::TableSetupColumn("ID",       LinearUITableColumnFlags_DefaultSort          | LinearUITableColumnFlags_WidthFixed,   0.0f, MyItemColumnID_ID);
            LinearUI::TableSetupColumn("Name",                                                  LinearUITableColumnFlags_WidthFixed,   0.0f, MyItemColumnID_Name);
            LinearUI::TableSetupColumn("Action",   LinearUITableColumnFlags_NoSort               | LinearUITableColumnFlags_WidthFixed,   0.0f, MyItemColumnID_Action);
            LinearUI::TableSetupColumn("Quantity", LinearUITableColumnFlags_PreferSortDescending | LinearUITableColumnFlags_WidthStretch, 0.0f, MyItemColumnID_Quantity);
            LinearUI::TableSetupScrollFreeze(0, 1); // Make row always visible
            LinearUI::TableHeadersRow();

            // Sort our data if sort specs have been changed!
            if (LinearUITableSortSpecs* sort_specs = LinearUI::TableGetSortSpecs())
                if (sort_specs->SpecsDirty)
                {
                    MyItem::SortWithSortSpecs(sort_specs, items.Data, items.Size);
                    sort_specs->SpecsDirty = false;
                }

            // Demonstrate using clipper for large vertical lists
            LinearUIListClipper clipper;
            clipper.Begin(items.Size);
            while (clipper.Step())
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    // Display a data item
                    MyItem* item = &items[row_n];
                    LinearUI::PushID(item->ID);
                    LinearUI::TableNextRow();
                    LinearUI::TableNextColumn();
                    LinearUI::Text("%04d", item->ID);
                    LinearUI::TableNextColumn();
                    LinearUI::TextUnformatted(item->Name);
                    LinearUI::TableNextColumn();
                    LinearUI::SmallButton("None");
                    LinearUI::TableNextColumn();
                    LinearUI::Text("%d", item->Quantity);
                    LinearUI::PopID();
                }
            LinearUI::EndTable();
        }
        LinearUI::TreePop();
    }

    // In this example we'll expose most table flags and settings.
    // For specific flags and settings refer to the corresponding section for more detailed explanation.
    // This section is mostly useful to experiment with combining certain flags or settings with each others.
    //LinearUI::SetNextItemOpen(true, LinearUICond_Once); // [DEBUG]
    if (open_action != -1)
        LinearUI::SetNextItemOpen(open_action != 0);
    if (LinearUI::TreeNode("Advanced"))
    {
        LINEARUI_DEMO_MARKER("Tables/Advanced");
        static LinearUITableFlags flags =
            LinearUITableFlags_Resizable | LinearUITableFlags_Reorderable | LinearUITableFlags_Hideable
            | LinearUITableFlags_Sortable | LinearUITableFlags_SortMulti
            | LinearUITableFlags_RowBg | LinearUITableFlags_Borders | LinearUITableFlags_NoBordersInBody
            | LinearUITableFlags_ScrollX | LinearUITableFlags_ScrollY
            | LinearUITableFlags_SizingFixedFit;
        static LinearUITableColumnFlags columns_base_flags = LinearUITableColumnFlags_None;

        enum ContentsType { CT_Text, CT_Button, CT_SmallButton, CT_FillButton, CT_Selectable, CT_SelectableSpanRow };
        static int contents_type = CT_SelectableSpanRow;
        const char* contents_type_names[] = { "Text", "Button", "SmallButton", "FillButton", "Selectable", "Selectable (span row)" };
        static int freeze_cols = 1;
        static int freeze_rows = 1;
        static int items_count = LINEARUI_COUNTOF(template_items_names) * 2;
        static LinearVec2 outer_size_value = LinearVec2(0.0f, TEXT_BASE_HEIGHT * 12);
        static float row_min_height = 0.0f; // Auto
        static float inner_width_with_scroll = 0.0f; // Auto-extend
        static bool outer_size_enabled = true;
        static bool show_headers = true;
        static bool show_wrapped_text = false;
        //static LinearUITextFilter filter;
        //LinearUI::SetNextItemOpen(true, LinearUICond_Once); // FIXME-TABLE: Enabling this results in initial clipped first pass on table which tend to affect column sizing
        if (LinearUI::TreeNode("Options"))
        {
            // Make the UI compact because there are so many fields
            PushStyleCompact();
            LinearUI::PushItemWidth(TEXT_BASE_WIDTH * 28.0f);

            if (LinearUI::TreeNodeEx("Features:", LinearUITreeNodeFlags_DefaultOpen))
            {
                LinearUI::CheckboxFlags("LinearUITableFlags_Resizable", &flags, LinearUITableFlags_Resizable);
                LinearUI::CheckboxFlags("LinearUITableFlags_Reorderable", &flags, LinearUITableFlags_Reorderable);
                LinearUI::CheckboxFlags("LinearUITableFlags_Hideable", &flags, LinearUITableFlags_Hideable);
                LinearUI::CheckboxFlags("LinearUITableFlags_Sortable", &flags, LinearUITableFlags_Sortable);
                LinearUI::CheckboxFlags("LinearUITableFlags_NoSavedSettings", &flags, LinearUITableFlags_NoSavedSettings);
                LinearUI::CheckboxFlags("LinearUITableFlags_ContextMenuInBody", &flags, LinearUITableFlags_ContextMenuInBody);
                LinearUI::TreePop();
            }

            if (LinearUI::TreeNodeEx("Decorations:", LinearUITreeNodeFlags_DefaultOpen))
            {
                LinearUI::CheckboxFlags("LinearUITableFlags_RowBg", &flags, LinearUITableFlags_RowBg);
                LinearUI::CheckboxFlags("LinearUITableFlags_BordersV", &flags, LinearUITableFlags_BordersV);
                LinearUI::CheckboxFlags("LinearUITableFlags_BordersOuterV", &flags, LinearUITableFlags_BordersOuterV);
                LinearUI::CheckboxFlags("LinearUITableFlags_BordersInnerV", &flags, LinearUITableFlags_BordersInnerV);
                LinearUI::CheckboxFlags("LinearUITableFlags_BordersH", &flags, LinearUITableFlags_BordersH);
                LinearUI::CheckboxFlags("LinearUITableFlags_BordersOuterH", &flags, LinearUITableFlags_BordersOuterH);
                LinearUI::CheckboxFlags("LinearUITableFlags_BordersInnerH", &flags, LinearUITableFlags_BordersInnerH);
                LinearUI::CheckboxFlags("LinearUITableFlags_NoBordersInBody", &flags, LinearUITableFlags_NoBordersInBody); LinearUI::SameLine(); HelpMarker("Disable vertical borders in columns Body (borders will always appear in Headers)");
                LinearUI::CheckboxFlags("LinearUITableFlags_NoBordersInBodyUntilResize", &flags, LinearUITableFlags_NoBordersInBodyUntilResize); LinearUI::SameLine(); HelpMarker("Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers)");
                LinearUI::TreePop();
            }

            if (LinearUI::TreeNodeEx("Sizing:", LinearUITreeNodeFlags_DefaultOpen))
            {
                EditTableSizingFlags(&flags);
                LinearUI::SameLine(); HelpMarker("In the Advanced demo we override the policy of each column so those table-wide settings have less effect that typical.");
                LinearUI::CheckboxFlags("LinearUITableFlags_NoHostExtendX", &flags, LinearUITableFlags_NoHostExtendX);
                LinearUI::SameLine(); HelpMarker("Make outer width auto-fit to columns, overriding outer_size.x value.\n\nOnly available when ScrollX/ScrollY are disabled and Stretch columns are not used.");
                LinearUI::CheckboxFlags("LinearUITableFlags_NoHostExtendY", &flags, LinearUITableFlags_NoHostExtendY);
                LinearUI::SameLine(); HelpMarker("Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit).\n\nOnly available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.");
                LinearUI::CheckboxFlags("LinearUITableFlags_NoKeepColumnsVisible", &flags, LinearUITableFlags_NoKeepColumnsVisible);
                LinearUI::SameLine(); HelpMarker("Only available if ScrollX is disabled.");
                LinearUI::CheckboxFlags("LinearUITableFlags_PreciseWidths", &flags, LinearUITableFlags_PreciseWidths);
                LinearUI::SameLine(); HelpMarker("Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.");
                LinearUI::CheckboxFlags("LinearUITableFlags_NoClip", &flags, LinearUITableFlags_NoClip);
                LinearUI::SameLine(); HelpMarker("Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with ScrollFreeze options.");
                LinearUI::TreePop();
            }

            if (LinearUI::TreeNodeEx("Padding:", LinearUITreeNodeFlags_DefaultOpen))
            {
                LinearUI::CheckboxFlags("LinearUITableFlags_PadOuterX", &flags, LinearUITableFlags_PadOuterX);
                LinearUI::CheckboxFlags("LinearUITableFlags_NoPadOuterX", &flags, LinearUITableFlags_NoPadOuterX);
                LinearUI::CheckboxFlags("LinearUITableFlags_NoPadInnerX", &flags, LinearUITableFlags_NoPadInnerX);
                LinearUI::TreePop();
            }

            if (LinearUI::TreeNodeEx("Scrolling:", LinearUITreeNodeFlags_DefaultOpen))
            {
                LinearUI::CheckboxFlags("LinearUITableFlags_ScrollX", &flags, LinearUITableFlags_ScrollX);
                LinearUI::SameLine();
                LinearUI::SetNextItemWidth(LinearUI::GetFrameHeight());
                LinearUI::DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL, LinearUISliderFlags_NoInput);
                LinearUI::CheckboxFlags("LinearUITableFlags_ScrollY", &flags, LinearUITableFlags_ScrollY);
                LinearUI::SameLine();
                LinearUI::SetNextItemWidth(LinearUI::GetFrameHeight());
                LinearUI::DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL, LinearUISliderFlags_NoInput);
                LinearUI::TreePop();
            }

            if (LinearUI::TreeNodeEx("Sorting:", LinearUITreeNodeFlags_DefaultOpen))
            {
                LinearUI::CheckboxFlags("LinearUITableFlags_SortMulti", &flags, LinearUITableFlags_SortMulti);
                LinearUI::SameLine(); HelpMarker("When sorting is enabled: hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).");
                LinearUI::CheckboxFlags("LinearUITableFlags_SortTristate", &flags, LinearUITableFlags_SortTristate);
                LinearUI::SameLine(); HelpMarker("When sorting is enabled: allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).");
                LinearUI::TreePop();
            }

            if (LinearUI::TreeNodeEx("Headers:", LinearUITreeNodeFlags_DefaultOpen))
            {
                LinearUI::Checkbox("show_headers", &show_headers);
                LinearUI::CheckboxFlags("LinearUITableFlags_HighlightHoveredColumn", &flags, LinearUITableFlags_HighlightHoveredColumn);
                LinearUI::CheckboxFlags("LinearUITableColumnFlags_AngledHeader", &columns_base_flags, LinearUITableColumnFlags_AngledHeader);
                LinearUI::SameLine(); HelpMarker("Enable AngledHeader on all columns. Best enabled on selected narrow columns (see \"Angled headers\" section of the demo).");
                LinearUI::TreePop();
            }

            if (LinearUI::TreeNodeEx("Other:", LinearUITreeNodeFlags_DefaultOpen))
            {
                LinearUI::Checkbox("show_wrapped_text", &show_wrapped_text);

                LinearUI::DragFloat2("##OuterSize", &outer_size_value.x);
                LinearUI::SameLine(0.0f, LinearUI::GetStyle().ItemInnerSpacing.x);
                LinearUI::Checkbox("outer_size", &outer_size_enabled);
                LinearUI::SameLine();
                HelpMarker("If scrolling is disabled (ScrollX and ScrollY not set):\n"
                    "- The table is output directly in the parent window.\n"
                    "- OuterSize.x < 0.0f will right-align the table.\n"
                    "- OuterSize.x = 0.0f will narrow fit the table unless there are any Stretch columns.\n"
                    "- OuterSize.y then becomes the minimum size for the table, which will extend vertically if there are more rows (unless NoHostExtendY is set).");

                // From a user point of view we will tend to use 'inner_width' differently depending on whether our table is embedding scrolling.
                // To facilitate toying with this demo we will actually pass 0.0f to the BeginTable() when ScrollX is disabled.
                LinearUI::DragFloat("inner_width (when ScrollX active)", &inner_width_with_scroll, 1.0f, 0.0f, FLT_MAX);

                LinearUI::DragFloat("row_min_height", &row_min_height, 1.0f, 0.0f, FLT_MAX);
                LinearUI::SameLine(); HelpMarker("Specify height of the Selectable item.");

                LinearUI::DragInt("items_count", &items_count, 0.1f, 0, 9999);
                LinearUI::Combo("items_type (first column)", &contents_type, contents_type_names, LINEARUI_COUNTOF(contents_type_names));
                //filter.Draw("filter");
                LinearUI::TreePop();
            }

            LinearUI::PopItemWidth();
            PopStyleCompact();
            LinearUI::Spacing();
            LinearUI::TreePop();
        }

        // Update item list if we changed the number of items
        static LinearVector<MyItem> items;
        static LinearVector<int> selection;
        static bool items_need_sort = false;
        if (items.Size != items_count)
        {
            items.resize(items_count, MyItem());
            for (int n = 0; n < items_count; n++)
            {
                const int template_n = n % LINEARUI_COUNTOF(template_items_names);
                MyItem& item = items[n];
                item.ID = n;
                item.Name = template_items_names[template_n];
                item.Quantity = (template_n == 3) ? 10 : (template_n == 4) ? 20 : 0; // Assign default quantities
            }
        }

        const LinearDrawList* parent_draw_list = LinearUI::GetWindowDrawList();
        const int parent_draw_list_draw_cmd_count = parent_draw_list->CmdBuffer.Size;
        LinearVec2 table_scroll_cur, table_scroll_max; // For debug display
        const LinearDrawList* table_draw_list = NULL;  // "

        // Submit table
        const float inner_width_to_use = (flags & LinearUITableFlags_ScrollX) ? inner_width_with_scroll : 0.0f;
        if (LinearUI::BeginTable("table_advanced", 6, flags, outer_size_enabled ? outer_size_value : LinearVec2(0, 0), inner_width_to_use))
        {
            // Declare columns
            // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
            // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
            LinearUI::TableSetupColumn("ID",           columns_base_flags | LinearUITableColumnFlags_DefaultSort | LinearUITableColumnFlags_WidthFixed | LinearUITableColumnFlags_NoHide, 0.0f, MyItemColumnID_ID);
            LinearUI::TableSetupColumn("Name",         columns_base_flags | LinearUITableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Name);
            LinearUI::TableSetupColumn("Action",       columns_base_flags | LinearUITableColumnFlags_NoSort | LinearUITableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Action);
            LinearUI::TableSetupColumn("Quantity",     columns_base_flags | LinearUITableColumnFlags_PreferSortDescending, 0.0f, MyItemColumnID_Quantity);
            LinearUI::TableSetupColumn("Description",  columns_base_flags | ((flags & LinearUITableFlags_NoHostExtendX) ? 0 : LinearUITableColumnFlags_WidthStretch), 0.0f, MyItemColumnID_Description);
            LinearUI::TableSetupColumn("Hidden",       columns_base_flags |  LinearUITableColumnFlags_DefaultHide | LinearUITableColumnFlags_NoSort);
            LinearUI::TableSetupScrollFreeze(freeze_cols, freeze_rows);

            // Sort our data if sort specs have been changed!
            LinearUITableSortSpecs* sort_specs = LinearUI::TableGetSortSpecs();
            if (sort_specs && sort_specs->SpecsDirty)
                items_need_sort = true;
            if (sort_specs && items_need_sort && items.Size > 1)
            {
                MyItem::SortWithSortSpecs(sort_specs, items.Data, items.Size);
                sort_specs->SpecsDirty = false;
            }
            items_need_sort = false;

            // Take note of whether we are currently sorting based on the Quantity field,
            // we will use this to trigger sorting when we know the data of this column has been modified.
            const bool sorts_specs_using_quantity = (LinearUI::TableGetColumnFlags(3) & LinearUITableColumnFlags_IsSorted) != 0;

            // Show headers
            if (show_headers && (columns_base_flags & LinearUITableColumnFlags_AngledHeader) != 0)
                LinearUI::TableAngledHeadersRow();
            if (show_headers)
                LinearUI::TableHeadersRow();

            // Show data
            // FIXME-TABLE FIXME-NAV: How we can get decent up/down even though we have the buttons here?
#if 1
            // Demonstrate using clipper for large vertical lists
            LinearUIListClipper clipper;
            clipper.Begin(items.Size);
            while (clipper.Step())
            {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
#else
            // Without clipper
            {
                for (int row_n = 0; row_n < items.Size; row_n++)
#endif
                {
                    MyItem* item = &items[row_n];
                    //if (!filter.PassFilter(item->Name))
                    //    continue;

                    const bool item_is_selected = selection.contains(item->ID);
                    LinearUI::PushID(item->ID);
                    LinearUI::TableNextRow(LinearUITableRowFlags_None, row_min_height);

                    // For the demo purpose we can select among different type of items submitted in the first column
                    LinearUI::TableSetColumnIndex(0);
                    char label[32];
                    sprintf(label, "%04d", item->ID);
                    if (contents_type == CT_Text)
                        LinearUI::TextUnformatted(label);
                    else if (contents_type == CT_Button)
                        LinearUI::Button(label);
                    else if (contents_type == CT_SmallButton)
                        LinearUI::SmallButton(label);
                    else if (contents_type == CT_FillButton)
                        LinearUI::Button(label, LinearVec2(-FLT_MIN, 0.0f));
                    else if (contents_type == CT_Selectable || contents_type == CT_SelectableSpanRow)
                    {
                        LinearUISelectableFlags selectable_flags = (contents_type == CT_SelectableSpanRow) ? LinearUISelectableFlags_SpanAllColumns | LinearUISelectableFlags_AllowOverlap : LinearUISelectableFlags_None;
                        if (LinearUI::Selectable(label, item_is_selected, selectable_flags, LinearVec2(0, row_min_height)))
                        {
                            if (LinearUI::GetIO().KeyCtrl)
                            {
                                if (item_is_selected)
                                    selection.find_erase_unsorted(item->ID);
                                else
                                    selection.push_back(item->ID);
                            }
                            else
                            {
                                selection.clear();
                                selection.push_back(item->ID);
                            }
                        }
                    }

                    if (LinearUI::TableSetColumnIndex(1))
                        LinearUI::TextUnformatted(item->Name);

                    // Here we demonstrate marking our data set as needing to be sorted again if we modified a quantity,
                    // and we are currently sorting on the column showing the Quantity.
                    // To avoid triggering a sort while holding the button, we only trigger it when the button has been released.
                    // You will probably need some extra logic if you want to automatically sort when a specific entry changes.
                    if (LinearUI::TableSetColumnIndex(2))
                    {
                        if (LinearUI::SmallButton("Chop")) { item->Quantity += 1; }
                        if (sorts_specs_using_quantity && LinearUI::IsItemDeactivated()) { items_need_sort = true; }
                        LinearUI::SameLine();
                        if (LinearUI::SmallButton("Eat")) { item->Quantity -= 1; }
                        if (sorts_specs_using_quantity && LinearUI::IsItemDeactivated()) { items_need_sort = true; }
                    }

                    if (LinearUI::TableSetColumnIndex(3))
                        LinearUI::Text("%d", item->Quantity);

                    LinearUI::TableSetColumnIndex(4);
                    if (show_wrapped_text)
                        LinearUI::TextWrapped("Lorem ipsum dolor sit amet");
                    else
                        LinearUI::Text("Lorem ipsum dolor sit amet");

                    if (LinearUI::TableSetColumnIndex(5))
                        LinearUI::Text("1234");

                    LinearUI::PopID();
                }
            }

            // Store some info to display debug details below
            table_scroll_cur = LinearVec2(LinearUI::GetScrollX(), LinearUI::GetScrollY());
            table_scroll_max = LinearVec2(LinearUI::GetScrollMaxX(), LinearUI::GetScrollMaxY());
            table_draw_list = LinearUI::GetWindowDrawList();
            LinearUI::EndTable();
        }
        static bool show_debug_details = false;
        LinearUI::Checkbox("Debug details", &show_debug_details);
        if (show_debug_details && table_draw_list)
        {
            LinearUI::SameLine(0.0f, 0.0f);
            const int table_draw_list_draw_cmd_count = table_draw_list->CmdBuffer.Size;
            if (table_draw_list == parent_draw_list)
                LinearUI::Text(": DrawCmd: +%d (in same window)",
                    table_draw_list_draw_cmd_count - parent_draw_list_draw_cmd_count);
            else
                LinearUI::Text(": DrawCmd: +%d (in child window), Scroll: (%.f/%.f) (%.f/%.f)",
                    table_draw_list_draw_cmd_count - 1, table_scroll_cur.x, table_scroll_max.x, table_scroll_cur.y, table_scroll_max.y);
        }
        LinearUI::TreePop();
    }

    LinearUI::PopID();

    DemoWindowColumns();

    if (disable_indent)
        LinearUI::PopStyleVar();
}

// Demonstrate old/legacy Columns API!
// [2020: Columns are under-featured and not maintained. Prefer using the more flexible and powerful BeginTable() API!]
static void DemoWindowColumns()
{
    bool open = LinearUI::TreeNode("Legacy Columns API");
    LinearUI::SameLine();
    HelpMarker("Columns() is an old API! Prefer using the more flexible and powerful BeginTable() API!");
    if (!open)
        return;

    // Basic columns
    if (LinearUI::TreeNode("Basic"))
    {
        LINEARUI_DEMO_MARKER("Columns (legacy API)/Basic");
        LinearUI::Text("Without border:");
        LinearUI::Columns(3, "mycolumns3", false);  // 3-ways, no border
        LinearUI::Separator();
        for (int n = 0; n < 14; n++)
        {
            char label[32];
            sprintf(label, "Item %d", n);
            if (LinearUI::Selectable(label)) {}
            //if (LinearUI::Button(label, LinearVec2(-FLT_MIN,0.0f))) {}
            LinearUI::NextColumn();
        }
        LinearUI::Columns(1);
        LinearUI::Separator();

        LinearUI::Text("With border:");
        LinearUI::Columns(4, "mycolumns"); // 4-ways, with border
        LinearUI::Separator();
        LinearUI::Text("ID"); LinearUI::NextColumn();
        LinearUI::Text("Name"); LinearUI::NextColumn();
        LinearUI::Text("Path"); LinearUI::NextColumn();
        LinearUI::Text("Hovered"); LinearUI::NextColumn();
        LinearUI::Separator();
        const char* names[3] = { "One", "Two", "Three" };
        const char* paths[3] = { "/path/one", "/path/two", "/path/three" };
        static int selected = -1;
        for (int i = 0; i < 3; i++)
        {
            char label[32];
            sprintf(label, "%04d", i);
            if (LinearUI::Selectable(label, selected == i, LinearUISelectableFlags_SpanAllColumns))
                selected = i;
            bool hovered = LinearUI::IsItemHovered();
            LinearUI::NextColumn();
            LinearUI::Text(names[i]); LinearUI::NextColumn();
            LinearUI::Text(paths[i]); LinearUI::NextColumn();
            LinearUI::Text("%d", hovered); LinearUI::NextColumn();
        }
        LinearUI::Columns(1);
        LinearUI::Separator();
        LinearUI::TreePop();
    }

    if (LinearUI::TreeNode("Borders"))
    {
        LINEARUI_DEMO_MARKER("Columns (legacy API)/Borders");
        // NB: Future columns API should allow automatic horizontal borders.
        static bool h_borders = true;
        static bool v_borders = true;
        static int columns_count = 4;
        const int lines_count = 3;
        LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 8);
        LinearUI::DragInt("##columns_count", &columns_count, 0.1f, 2, 10, "%d columns");
        if (columns_count < 2)
            columns_count = 2;
        LinearUI::SameLine();
        LinearUI::Checkbox("horizontal", &h_borders);
        LinearUI::SameLine();
        LinearUI::Checkbox("vertical", &v_borders);
        LinearUI::Columns(columns_count, NULL, v_borders);
        for (int i = 0; i < columns_count * lines_count; i++)
        {
            if (h_borders && LinearUI::GetColumnIndex() == 0)
                LinearUI::Separator();
            LinearUI::PushID(i);
            LinearUI::Text("%c%c%c", 'a' + i, 'a' + i, 'a' + i);
            LinearUI::Text("Width %.2f", LinearUI::GetColumnWidth());
            LinearUI::Text("Avail %.2f", LinearUI::GetContentRegionAvail().x);
            LinearUI::Text("Offset %.2f", LinearUI::GetColumnOffset());
            LinearUI::Text("Long text that is likely to clip");
            LinearUI::Button("Button", LinearVec2(-FLT_MIN, 0.0f));
            LinearUI::PopID();
            LinearUI::NextColumn();
        }
        LinearUI::Columns(1);
        if (h_borders)
            LinearUI::Separator();
        LinearUI::TreePop();
    }

    // Create multiple items in a same cell before switching to next column
    if (LinearUI::TreeNode("Mixed items"))
    {
        LINEARUI_DEMO_MARKER("Columns (legacy API)/Mixed items");
        LinearUI::Columns(3, "mixed");
        LinearUI::Separator();

        LinearUI::Text("Hello");
        LinearUI::Button("Banana");
        LinearUI::NextColumn();

        LinearUI::Text("LinearUI");
        LinearUI::Button("Apple");
        static float foo = 1.0f;
        LinearUI::InputFloat("red", &foo, 0.05f, 0, "%.3f");
        LinearUI::Text("An extra line here.");
        LinearUI::NextColumn();

        LinearUI::Text("Sailor");
        LinearUI::Button("Corniflower");
        static float bar = 1.0f;
        LinearUI::InputFloat("blue", &bar, 0.05f, 0, "%.3f");
        LinearUI::NextColumn();

        if (LinearUI::CollapsingHeader("Category A")) { LinearUI::Text("Blah blah blah"); } LinearUI::NextColumn();
        if (LinearUI::CollapsingHeader("Category B")) { LinearUI::Text("Blah blah blah"); } LinearUI::NextColumn();
        if (LinearUI::CollapsingHeader("Category C")) { LinearUI::Text("Blah blah blah"); } LinearUI::NextColumn();
        LinearUI::Columns(1);
        LinearUI::Separator();
        LinearUI::TreePop();
    }

    // Word wrapping
    if (LinearUI::TreeNode("Word-wrapping"))
    {
        LINEARUI_DEMO_MARKER("Columns (legacy API)/Word-wrapping");
        LinearUI::Columns(2, "word-wrapping");
        LinearUI::Separator();
        LinearUI::TextWrapped("The quick brown fox jumps over the lazy dog.");
        LinearUI::TextWrapped("Hello Left");
        LinearUI::NextColumn();
        LinearUI::TextWrapped("The quick brown fox jumps over the lazy dog.");
        LinearUI::TextWrapped("Hello Right");
        LinearUI::Columns(1);
        LinearUI::Separator();
        LinearUI::TreePop();
    }

    if (LinearUI::TreeNode("Horizontal Scrolling"))
    {
        LINEARUI_DEMO_MARKER("Columns (legacy API)/Horizontal Scrolling");
        LinearUI::SetNextWindowContentSize(LinearVec2(1500.0f, 0.0f));
        LinearVec2 child_size = LinearVec2(0, LinearUI::GetFontSize() * 20.0f);
        LinearUI::BeginChild("##ScrollingRegion", child_size, LinearUIChildFlags_None, LinearUIWindowFlags_HorizontalScrollbar);
        LinearUI::Columns(10);

        // Also demonstrate using clipper for large vertical lists
        int ITEMS_COUNT = 2000;
        LinearUIListClipper clipper;
        clipper.Begin(ITEMS_COUNT);
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                for (int j = 0; j < 10; j++)
                {
                    LinearUI::Text("Line %d Column %d...", i, j);
                    LinearUI::NextColumn();
                }
        }
        LinearUI::Columns(1);
        LinearUI::EndChild();
        LinearUI::TreePop();
    }

    if (LinearUI::TreeNode("Tree"))
    {
        LINEARUI_DEMO_MARKER("Columns (legacy API)/Tree");
        LinearUI::Columns(2, "tree", true);
        for (int x = 0; x < 3; x++)
        {
            bool open1 = LinearUI::TreeNode((void*)(intptr_t)x, "Node%d", x);
            LinearUI::NextColumn();
            LinearUI::Text("Node contents");
            LinearUI::NextColumn();
            if (open1)
            {
                for (int y = 0; y < 3; y++)
                {
                    bool open2 = LinearUI::TreeNode((void*)(intptr_t)y, "Node%d.%d", x, y);
                    LinearUI::NextColumn();
                    LinearUI::Text("Node contents");
                    if (open2)
                    {
                        LinearUI::Text("Even more contents");
                        if (LinearUI::TreeNode("Tree in column"))
                        {
                            LinearUI::Text("The quick brown fox jumps over the lazy dog");
                            LinearUI::TreePop();
                        }
                    }
                    LinearUI::NextColumn();
                    if (open2)
                        LinearUI::TreePop();
                }
                LinearUI::TreePop();
            }
        }
        LinearUI::Columns(1);
        LinearUI::TreePop();
    }

    LinearUI::TreePop();
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowInputs()
//-----------------------------------------------------------------------------

static void DemoWindowInputs()
{
    if (LinearUI::CollapsingHeader("Inputs & Focus"))
    {
        LinearUIIO& io = LinearUI::GetIO();

        // Display inputs submitted to LinearUIIO
        LinearUI::SetNextItemOpen(true, LinearUICond_Once);
        bool inputs_opened = LinearUI::TreeNode("Inputs");
        LinearUI::SameLine();
        HelpMarker(
            "This is a simplified view. See more detailed input state:\n"
            "- in 'Tools->Metrics/Debugger->Inputs'.\n"
            "- in 'Tools->Debug Log->IO'.");
        if (inputs_opened)
        {
            LINEARUI_DEMO_MARKER("Inputs & Focus/Inputs");
            if (LinearUI::IsMousePosValid())
                LinearUI::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
            else
                LinearUI::Text("Mouse pos: <INVALID>");
            LinearUI::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
            LinearUI::Text("Mouse down:");
            for (int i = 0; i < LINEARUI_COUNTOF(io.MouseDown); i++) if (LinearUI::IsMouseDown(i)) { LinearUI::SameLine(); LinearUI::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
            LinearUI::Text("Mouse wheel: %.1f", io.MouseWheel);
            LinearUI::Text("Mouse clicked count:");
            for (int i = 0; i < LINEARUI_COUNTOF(io.MouseDown); i++) if (io.MouseClickedCount[i] > 0) { LinearUI::SameLine(); LinearUI::Text("b%d: %d", i, io.MouseClickedCount[i]); }

            // We iterate both legacy native range and named LinearUIKey ranges. This is a little unusual/odd but this allows
            // displaying the data for old/new backends.
            // User code should never have to go through such hoops!
            // You can generally iterate between LinearUIKey_NamedKey_BEGIN and LinearUIKey_NamedKey_END.
            struct funcs { static bool IsLegacyNativeDupe(LinearUIKey) { return false; } };
            LinearUIKey start_key = LinearUIKey_NamedKey_BEGIN;
            LinearUI::Text("Keys down:");         for (LinearUIKey key = start_key; key < LinearUIKey_NamedKey_END; key = (LinearUIKey)(key + 1)) { if (funcs::IsLegacyNativeDupe(key) || !LinearUI::IsKeyDown(key)) continue; LinearUI::SameLine(); LinearUI::Text((key < LinearUIKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", LinearUI::GetKeyName(key), key); }
            LinearUI::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
            LinearUI::Text("Chars queue:");       for (int i = 0; i < io.InputQueueCharacters.Size; i++) { LinearWchar c = io.InputQueueCharacters[i]; LinearUI::SameLine();  LinearUI::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.

            LinearUI::TreePop();
        }

        // Display LinearUIIO output flags
        LinearUI::SetNextItemOpen(true, LinearUICond_Once);
        bool outputs_opened = LinearUI::TreeNode("Outputs");
        LinearUI::SameLine();
        HelpMarker(
            "The value of io.WantCaptureMouse and io.WantCaptureKeyboard are normally set by Dear LinearUI "
            "to instruct your application of how to route inputs. Typically, when a value is true, it means "
            "Dear LinearUI wants the corresponding inputs and we expect the underlying application to ignore them.\n\n"
            "The most typical case is: when hovering a window, Dear LinearUI set io.WantCaptureMouse to true, "
            "and underlying application should ignore mouse inputs (in practice there are many and more subtle "
            "rules leading to how those flags are set).");
        if (outputs_opened)
        {
            LINEARUI_DEMO_MARKER("Inputs & Focus/Outputs");
            LinearUI::Text("io.WantCaptureMouse: %d", io.WantCaptureMouse);
            LinearUI::Text("io.WantCaptureMouseUnlessPopupClose: %d", io.WantCaptureMouseUnlessPopupClose);
            LinearUI::Text("io.WantCaptureKeyboard: %d", io.WantCaptureKeyboard);
            LinearUI::Text("io.WantTextInput: %d", io.WantTextInput);
            LinearUI::Text("io.WantSetMousePos: %d", io.WantSetMousePos);
            LinearUI::Text("io.NavActive: %d, io.NavVisible: %d", io.NavActive, io.NavVisible);

            LINEARUI_DEMO_MARKER("Inputs & Focus/Outputs/WantCapture override");
            if (LinearUI::TreeNode("WantCapture override"))
            {
                HelpMarker(
                    "Hovering the colored canvas will override io.WantCaptureXXX fields.\n"
                    "Notice how normally (when set to none), the value of io.WantCaptureKeyboard would be false when hovering "
                    "and true when clicking.");
                static int capture_override_mouse = -1;
                static int capture_override_keyboard = -1;
                const char* capture_override_desc[] = { "None", "Set to false", "Set to true" };
                LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 15);
                LinearUI::SliderInt("SetNextFrameWantCaptureMouse() on hover", &capture_override_mouse, -1, +1, capture_override_desc[capture_override_mouse + 1], LinearUISliderFlags_AlwaysClamp);
                LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 15);
                LinearUI::SliderInt("SetNextFrameWantCaptureKeyboard() on hover", &capture_override_keyboard, -1, +1, capture_override_desc[capture_override_keyboard + 1], LinearUISliderFlags_AlwaysClamp);

                LinearUI::ColorButton("##panel", LinearVec4(0.7f, 0.1f, 0.7f, 1.0f), LinearUIColorEditFlags_NoTooltip | LinearUIColorEditFlags_NoDragDrop, LinearVec2(128.0f, 96.0f)); // Dummy item
                if (LinearUI::IsItemHovered() && capture_override_mouse != -1)
                    LinearUI::SetNextFrameWantCaptureMouse(capture_override_mouse == 1);
                if (LinearUI::IsItemHovered() && capture_override_keyboard != -1)
                    LinearUI::SetNextFrameWantCaptureKeyboard(capture_override_keyboard == 1);

                LinearUI::TreePop();
            }
            LinearUI::TreePop();
        }

        // Demonstrate using Shortcut() and Routing Policies.
        // The general flow is:
        // - Code interested in a chord (e.g. "Ctrl+A") declares their intent.
        // - Multiple locations may be interested in same chord! Routing helps find a winner.
        // - Every frame, we resolve all claims and assign one owner if the modifiers are matching.
        // - The lower-level function is 'bool SetShortcutRouting()', returns true when caller got the route.
        // - Most of the times, SetShortcutRouting() is not called directly. User mostly calls Shortcut() with routing flags.
        // - If you call Shortcut() WITHOUT any routing option, it uses LinearUIInputFlags_RouteFocused.
        // TL;DR: Most uses will simply be:
        // - Shortcut(LinearUIMod_Ctrl | LinearUIKey_A); // Use LinearUIInputFlags_RouteFocused policy.
        if (LinearUI::TreeNode("Shortcuts"))
        {
            LINEARUI_DEMO_MARKER("Inputs & Focus/Shortcuts");
            static LinearUIInputFlags route_options = LinearUIInputFlags_Repeat;
            static LinearUIInputFlags route_type = LinearUIInputFlags_RouteFocused;
            LinearUI::CheckboxFlags("LinearUIInputFlags_Repeat", &route_options, LinearUIInputFlags_Repeat);
            LinearUI::RadioButton("LinearUIInputFlags_RouteActive", &route_type, LinearUIInputFlags_RouteActive);
            LinearUI::RadioButton("LinearUIInputFlags_RouteFocused (default)", &route_type, LinearUIInputFlags_RouteFocused);
            LinearUI::Indent();
            LinearUI::BeginDisabled(route_type != LinearUIInputFlags_RouteFocused);
            LinearUI::CheckboxFlags("LinearUIInputFlags_RouteOverActive##0", &route_options, LinearUIInputFlags_RouteOverActive);
            LinearUI::EndDisabled();
            LinearUI::Unindent();
            LinearUI::RadioButton("LinearUIInputFlags_RouteGlobal", &route_type, LinearUIInputFlags_RouteGlobal);
            LinearUI::Indent();
            LinearUI::BeginDisabled(route_type != LinearUIInputFlags_RouteGlobal);
            LinearUI::CheckboxFlags("LinearUIInputFlags_RouteOverFocused", &route_options, LinearUIInputFlags_RouteOverFocused);
            LinearUI::CheckboxFlags("LinearUIInputFlags_RouteOverActive", &route_options, LinearUIInputFlags_RouteOverActive);
            LinearUI::CheckboxFlags("LinearUIInputFlags_RouteUnlessBgFocused", &route_options, LinearUIInputFlags_RouteUnlessBgFocused);
            LinearUI::EndDisabled();
            LinearUI::Unindent();
            LinearUI::RadioButton("LinearUIInputFlags_RouteAlways", &route_type, LinearUIInputFlags_RouteAlways);
            LinearUIInputFlags flags = route_type | route_options; // Merged flags
            if (route_type != LinearUIInputFlags_RouteGlobal)
                flags &= ~(LinearUIInputFlags_RouteOverFocused | LinearUIInputFlags_RouteOverActive | LinearUIInputFlags_RouteUnlessBgFocused);

            LinearUI::SeparatorText("Using SetNextItemShortcut()");
            LinearUI::Text("Ctrl+S");
            LinearUI::SetNextItemShortcut(LinearUIMod_Ctrl | LinearUIKey_S, flags | LinearUIInputFlags_Tooltip);
            LinearUI::Button("Save");
            LinearUI::Text("Alt+F");
            LinearUI::SetNextItemShortcut(LinearUIMod_Alt | LinearUIKey_F, flags | LinearUIInputFlags_Tooltip);
            static float f = 0.5f;
            LinearUI::SliderFloat("Factor", &f, 0.0f, 1.0f);

            LinearUI::SeparatorText("Using Shortcut()");
            const float line_height = LinearUI::GetTextLineHeightWithSpacing();
            const LinearUIKeyChord key_chord = LinearUIMod_Ctrl | LinearUIKey_A;

            LinearUI::Text("Ctrl+A");
            LinearUI::Text("IsWindowFocused: %d, Shortcut: %s", LinearUI::IsWindowFocused(), LinearUI::Shortcut(key_chord, flags) ? "PRESSED" : "...");

            LinearUI::PushStyleColor(LinearUICol_ChildBg, LinearVec4(1.0f, 0.0f, 1.0f, 0.1f));

            LinearUI::BeginChild("WindowA", LinearVec2(-FLT_MIN, line_height * 14), true);
            LinearUI::Text("Press Ctrl+A and see who receives it!");
            LinearUI::Separator();

            // 1: Window polling for Ctrl+A
            LinearUI::Text("(in WindowA)");
            LinearUI::Text("IsWindowFocused: %d, Shortcut: %s", LinearUI::IsWindowFocused(), LinearUI::Shortcut(key_chord, flags) ? "PRESSED" : "...");

            // 2: InputText also polling for Ctrl+A: it always uses _RouteFocused internally (gets priority when active)
            // (Commented because the owner-aware version of Shortcut() is still in linearui_internal.h)
            //char str[16] = "Press Ctrl+A";
            //LinearUI::Spacing();
            //LinearUI::InputText("InputTextB", str, LINEARUI_COUNTOF(str), LinearUIInputTextFlags_ReadOnly);
            //LinearUIID item_id = LinearUI::GetItemID();
            //LinearUI::SameLine(); HelpMarker("Internal widgets always use _RouteFocused");
            //LinearUI::Text("IsWindowFocused: %d, Shortcut: %s", LinearUI::IsWindowFocused(), LinearUI::Shortcut(key_chord, flags, item_id) ? "PRESSED" : "...");

            // 3: Dummy child is not claiming the route: focusing them shouldn't steal route away from WindowA
            LinearUI::BeginChild("ChildD", LinearVec2(-FLT_MIN, line_height * 4), true);
            LinearUI::Text("(in ChildD: not using same Shortcut)");
            LinearUI::Text("IsWindowFocused: %d", LinearUI::IsWindowFocused());
            LinearUI::EndChild();

            // 4: Child window polling for Ctrl+A. It is deeper than WindowA and gets priority when focused.
            LinearUI::BeginChild("ChildE", LinearVec2(-FLT_MIN, line_height * 4), true);
            LinearUI::Text("(in ChildE: using same Shortcut)");
            LinearUI::Text("IsWindowFocused: %d, Shortcut: %s", LinearUI::IsWindowFocused(), LinearUI::Shortcut(key_chord, flags) ? "PRESSED" : "...");
            LinearUI::EndChild();

            // 5: In a popup
            if (LinearUI::Button("Open Popup"))
                LinearUI::OpenPopup("PopupF");
            if (LinearUI::BeginPopup("PopupF"))
            {
                LinearUI::Text("(in PopupF)");
                LinearUI::Text("IsWindowFocused: %d, Shortcut: %s", LinearUI::IsWindowFocused(), LinearUI::Shortcut(key_chord, flags) ? "PRESSED" : "...");
                // (Commented because the owner-aware version of Shortcut() is still in linearui_internal.h)
                //LinearUI::InputText("InputTextG", str, LINEARUI_COUNTOF(str), LinearUIInputTextFlags_ReadOnly);
                //LinearUI::Text("IsWindowFocused: %d, Shortcut: %s", LinearUI::IsWindowFocused(), LinearUI::Shortcut(key_chord, flags, LinearUI::GetItemID()) ? "PRESSED" : "...");
                LinearUI::EndPopup();
            }
            LinearUI::EndChild();
            LinearUI::PopStyleColor();

            LinearUI::TreePop();
        }

        // Display mouse cursors
        if (LinearUI::TreeNode("Mouse Cursors"))
        {
            LINEARUI_DEMO_MARKER("Inputs & Focus/Mouse Cursors");
            const char* mouse_cursors_names[] = { "Arrow", "TextInput", "ResizeAll", "ResizeNS", "ResizeEW", "ResizeNESW", "ResizeNWSE", "Hand", "Wait", "Progress", "NotAllowed" };
            LINEARUI_ASSERT(LINEARUI_COUNTOF(mouse_cursors_names) == LinearUIMouseCursor_COUNT);

            LinearUIMouseCursor current = LinearUI::GetMouseCursor();
            const char* cursor_name = (current >= LinearUIMouseCursor_Arrow) && (current < LinearUIMouseCursor_COUNT) ? mouse_cursors_names[current] : "N/A";
            LinearUI::Text("Current mouse cursor = %d: %s", current, cursor_name);
            LinearUI::BeginDisabled(true);
            LinearUI::CheckboxFlags("io.BackendFlags: HasMouseCursors", &io.BackendFlags, LinearUIBackendFlags_HasMouseCursors);
            LinearUI::EndDisabled();

            LinearUI::Text("Hover to see mouse cursors:");
            LinearUI::SameLine(); HelpMarker(
                "Your application can render a different mouse cursor based on what LinearUI::GetMouseCursor() returns. "
                "If software cursor rendering (io.MouseDrawCursor) is set LinearUI will draw the right cursor for you, "
                "otherwise your backend needs to handle it.");
            for (int i = 0; i < LinearUIMouseCursor_COUNT; i++)
            {
                char label[32];
                sprintf(label, "Mouse cursor %d: %s", i, mouse_cursors_names[i]);
                LinearUI::Bullet(); LinearUI::Selectable(label, false);
                if (LinearUI::IsItemHovered())
                    LinearUI::SetMouseCursor(i);
            }
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Tabbing"))
        {
            LINEARUI_DEMO_MARKER("Inputs & Focus/Tabbing");
            LinearUI::Text("Use Tab/Shift+Tab to cycle through keyboard editable fields.");
            static char buf[32] = "hello";
            LinearUI::InputText("1", buf, LINEARUI_COUNTOF(buf));
            LinearUI::InputText("2", buf, LINEARUI_COUNTOF(buf));
            LinearUI::InputText("3", buf, LINEARUI_COUNTOF(buf));
            LinearUI::PushItemFlag(LinearUIItemFlags_NoTabStop, true);
            LinearUI::InputText("4 (tab skip)", buf, LINEARUI_COUNTOF(buf));
            LinearUI::SameLine(); HelpMarker("Item won't be cycled through when using TAB or Shift+Tab.");
            LinearUI::PopItemFlag();
            LinearUI::InputText("5", buf, LINEARUI_COUNTOF(buf));
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Focus from code"))
        {
            LINEARUI_DEMO_MARKER("Inputs & Focus/Focus from code");
            bool focus_1 = LinearUI::Button("Focus on 1"); LinearUI::SameLine();
            bool focus_2 = LinearUI::Button("Focus on 2"); LinearUI::SameLine();
            bool focus_3 = LinearUI::Button("Focus on 3");
            int has_focus = 0;
            static char buf[128] = "click on a button to set focus";

            if (focus_1) LinearUI::SetKeyboardFocusHere();
            LinearUI::InputText("1", buf, LINEARUI_COUNTOF(buf));
            if (LinearUI::IsItemActive()) has_focus = 1;

            if (focus_2) LinearUI::SetKeyboardFocusHere();
            LinearUI::InputText("2", buf, LINEARUI_COUNTOF(buf));
            if (LinearUI::IsItemActive()) has_focus = 2;

            LinearUI::PushItemFlag(LinearUIItemFlags_NoTabStop, true);
            if (focus_3) LinearUI::SetKeyboardFocusHere();
            LinearUI::InputText("3 (tab skip)", buf, LINEARUI_COUNTOF(buf));
            if (LinearUI::IsItemActive()) has_focus = 3;
            LinearUI::SameLine(); HelpMarker("Item won't be cycled through when using TAB or Shift+Tab.");
            LinearUI::PopItemFlag();

            if (has_focus)
                LinearUI::Text("Item with focus: %d", has_focus);
            else
                LinearUI::Text("Item with focus: <none>");

            // Use >= 0 parameter to SetKeyboardFocusHere() to focus an upcoming item
            static float f3[3] = { 0.0f, 0.0f, 0.0f };
            int focus_ahead = -1;
            if (LinearUI::Button("Focus on X")) { focus_ahead = 0; } LinearUI::SameLine();
            if (LinearUI::Button("Focus on Y")) { focus_ahead = 1; } LinearUI::SameLine();
            if (LinearUI::Button("Focus on Z")) { focus_ahead = 2; }
            if (focus_ahead != -1) LinearUI::SetKeyboardFocusHere(focus_ahead);
            LinearUI::SliderFloat3("Float3", &f3[0], 0.0f, 1.0f);

            LinearUI::TextWrapped("NB: Cursor & selection are preserved when refocusing last used item in code.");
            LinearUI::TreePop();
        }

        if (LinearUI::TreeNode("Dragging"))
        {
            LINEARUI_DEMO_MARKER("Inputs & Focus/Dragging");
            LinearUI::TextWrapped("You can use LinearUI::GetMouseDragDelta(0) to query for the dragged amount on any widget.");
            for (int button = 0; button < 3; button++)
            {
                LinearUI::Text("IsMouseDragging(%d):", button);
                LinearUI::Text("  w/ default threshold: %d,", LinearUI::IsMouseDragging(button));
                LinearUI::Text("  w/ zero threshold: %d,", LinearUI::IsMouseDragging(button, 0.0f));
                LinearUI::Text("  w/ large threshold: %d,", LinearUI::IsMouseDragging(button, 20.0f));
            }

            LinearUI::Button("Drag Me");
            if (LinearUI::IsItemActive())
                LinearUI::GetForegroundDrawList()->AddLine(io.MouseClickedPos[0], io.MousePos, LinearUI::GetColorU32(LinearUICol_Button), 4.0f); // Draw a line between the button and the mouse cursor

            // Drag operations gets "unlocked" when the mouse has moved past a certain threshold
            // (the default threshold is stored in io.MouseDragThreshold). You can request a lower or higher
            // threshold using the second parameter of IsMouseDragging() and GetMouseDragDelta().
            LinearVec2 value_raw = LinearUI::GetMouseDragDelta(0, 0.0f);
            LinearVec2 value_with_lock_threshold = LinearUI::GetMouseDragDelta(0);
            LinearVec2 mouse_delta = io.MouseDelta;
            LinearUI::Text("GetMouseDragDelta(0):");
            LinearUI::Text("  w/ default threshold: (%.1f, %.1f)", value_with_lock_threshold.x, value_with_lock_threshold.y);
            LinearUI::Text("  w/ zero threshold: (%.1f, %.1f)", value_raw.x, value_raw.y);
            LinearUI::Text("io.MouseDelta: (%.1f, %.1f)", mouse_delta.x, mouse_delta.y);
            LinearUI::TreePop();
        }
    }
}

//-----------------------------------------------------------------------------
// [SECTION] About Window / ShowAboutWindow()
// Access from Dear LinearUI Demo -> Tools -> About
//-----------------------------------------------------------------------------

void LinearUI::ShowAboutWindow(bool* p_open)
{
    if (!LinearUI::Begin("About Dear LinearUI", p_open, LinearUIWindowFlags_AlwaysAutoResize))
    {
        LinearUI::End();
        return;
    }
    LINEARUI_DEMO_MARKER("Tools/About Dear LinearUI");
    LinearUI::Text("Dear LinearUI %s (%d)", LINEARUI_VERSION, LINEARUI_VERSION_NUM);

    LinearUI::TextLinkOpenURL("Homepage", "https://github.com/ocornut/linearui");
    LinearUI::SameLine();
    LinearUI::TextLinkOpenURL("FAQ", "https://github.com/ocornut/linearui/blob/master/docs/FAQ.md");
    LinearUI::SameLine();
    LinearUI::TextLinkOpenURL("Wiki", "https://github.com/ocornut/linearui/wiki");
    LinearUI::SameLine();
    LinearUI::TextLinkOpenURL("Extensions", "https://github.com/ocornut/linearui/wiki/Useful-Extensions");
    LinearUI::SameLine();
    LinearUI::TextLinkOpenURL("Releases", "https://github.com/ocornut/linearui/releases");
    LinearUI::SameLine();
    LinearUI::TextLinkOpenURL("Funding", "https://github.com/ocornut/linearui/wiki/Funding");

    LinearUI::Separator();
    LinearUI::Text("(c) 2014-2026 Omar Cornut");
    LinearUI::Text("Developed by Omar Cornut and all Dear LinearUI contributors.");
    LinearUI::Text("Dear LinearUI is licensed under the MIT License, see LICENSE for more information.");
    LinearUI::Text("If your company uses this, please consider funding the project.");

    static bool show_config_info = false;
    LinearUI::Checkbox("Config/Build Information", &show_config_info);
    if (show_config_info)
    {
        LinearUIIO& io = LinearUI::GetIO();
        LinearUIStyle& style = LinearUI::GetStyle();

        bool copy_to_clipboard = LinearUI::Button("Copy to clipboard");
        LinearVec2 child_size = LinearVec2(0, LinearUI::GetTextLineHeightWithSpacing() * 18);
        LinearUI::BeginChild(LinearUI::GetID("cfg_infos"), child_size, LinearUIChildFlags_FrameStyle);
        if (copy_to_clipboard)
        {
            LinearUI::LogToClipboard();
            LinearUI::LogText("// (Copy from the next line. Keep the ``` markers for formatting.)\n");
            LinearUI::LogText("```cpp\n"); // Back quotes will make text appears without formatting when pasting on GitHub
        }

        LinearUI::Text("Dear LinearUI %s (%d)", LINEARUI_VERSION, LINEARUI_VERSION_NUM);
        LinearUI::Separator();
        LinearUI::Text("sizeof(size_t): %d, sizeof(LinearDrawIdx): %d, sizeof(LinearDrawVert): %d", (int)sizeof(size_t), (int)sizeof(LinearDrawIdx), (int)sizeof(LinearDrawVert));
        LinearUI::Text("define: __cplusplus=%d", (int)__cplusplus);
#ifdef LINEARUI_ENABLE_TEST_ENGINE
        LinearUI::Text("define: LINEARUI_ENABLE_TEST_ENGINE");
#endif
#ifdef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
        LinearUI::Text("define: LINEARUI_DISABLE_OBSOLETE_FUNCTIONS");
#endif
#ifdef LINEARUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
        LinearUI::Text("define: LINEARUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS");
#endif
#ifdef LINEARUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
        LinearUI::Text("define: LINEARUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS");
#endif
#ifdef LINEARUI_DISABLE_WIN32_FUNCTIONS
        LinearUI::Text("define: LINEARUI_DISABLE_WIN32_FUNCTIONS");
#endif
#ifdef LINEARUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
        LinearUI::Text("define: LINEARUI_DISABLE_DEFAULT_SHELL_FUNCTIONS");
#endif
#ifdef LINEARUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
        LinearUI::Text("define: LINEARUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS");
#endif
#ifdef LINEARUI_DISABLE_DEFAULT_MATH_FUNCTIONS
        LinearUI::Text("define: LINEARUI_DISABLE_DEFAULT_MATH_FUNCTIONS");
#endif
#ifdef LINEARUI_DISABLE_DEFAULT_FILE_FUNCTIONS
        LinearUI::Text("define: LINEARUI_DISABLE_DEFAULT_FILE_FUNCTIONS");
#endif
#ifdef LINEARUI_DISABLE_FILE_FUNCTIONS
        LinearUI::Text("define: LINEARUI_DISABLE_FILE_FUNCTIONS");
#endif
#ifdef LINEARUI_DISABLE_DEFAULT_ALLOCATORS
        LinearUI::Text("define: LINEARUI_DISABLE_DEFAULT_ALLOCATORS");
#endif
#ifdef LINEARUI_USE_BGRA_PACKED_COLOR
        LinearUI::Text("define: LINEARUI_USE_BGRA_PACKED_COLOR");
#endif
#ifdef _WIN32
        LinearUI::Text("define: _WIN32");
#endif
#ifdef _WIN64
        LinearUI::Text("define: _WIN64");
#endif
#ifdef __linux__
        LinearUI::Text("define: __linux__");
#endif
#ifdef __APPLE__
        LinearUI::Text("define: __APPLE__");
#endif
#ifdef _MSC_VER
        LinearUI::Text("define: _MSC_VER=%d", _MSC_VER);
#endif
#ifdef _MSVC_LANG
        LinearUI::Text("define: _MSVC_LANG=%d", (int)_MSVC_LANG);
#endif
#ifdef __MINGW32__
        LinearUI::Text("define: __MINGW32__");
#endif
#ifdef __MINGW64__
        LinearUI::Text("define: __MINGW64__");
#endif
#ifdef __GNUC__
        LinearUI::Text("define: __GNUC__=%d", (int)__GNUC__);
#endif
#ifdef __clang_version__
        LinearUI::Text("define: __clang_version__=%s", __clang_version__);
#endif
#ifdef __EMSCRIPTEN__
        LinearUI::Text("define: __EMSCRIPTEN__");
#ifdef __EMSCRIPTEN_MAJOR__
        LinearUI::Text("Emscripten: %d.%d.%d", __EMSCRIPTEN_MAJOR__, __EMSCRIPTEN_MINOR__, __EMSCRIPTEN_TINY__);
#else
        LinearUI::Text("Emscripten: %d.%d.%d", __EMSCRIPTEN_major__, __EMSCRIPTEN_minor__, __EMSCRIPTEN_tiny__);
#endif
#endif
#ifdef LINEARUI_HAS_VIEWPORT
        LinearUI::Text("define: LINEARUI_HAS_VIEWPORT");
#endif
#ifdef LINEARUI_HAS_DOCK
        LinearUI::Text("define: LINEARUI_HAS_DOCK");
#endif
#ifdef NDEBUG
        LinearUI::Text("define: NDEBUG");
#endif

        // Heuristic to detect no-op LINEARUI_ASSERT() macros
        // - This is designed so people opening bug reports would convey and notice that they have disabled asserts for Dear LinearUI code.
        // - 16 is > strlen("((void)(_EXPR))") which we suggested in our linearuiconfig.h template as a possible way to disable.
        int assert_runs_expression = 0;
        LINEARUI_ASSERT(++assert_runs_expression);
        int assert_expand_len = (int)strlen(LINEARUI_STRINGIFY((LINEARUI_ASSERT(true))));
        bool assert_maybe_disabled = (!assert_runs_expression || assert_expand_len <= 16);
        LinearUI::Text("LINEARUI_ASSERT: runs expression: %s. expand size: %s%s",
            assert_runs_expression ? "OK" : "KO", (assert_expand_len > 16) ? "OK" : "KO", assert_maybe_disabled ? " (MAYBE DISABLED?!)" : "");
        if (assert_maybe_disabled)
        {
            LinearUI::SameLine();
            HelpMarker("LINEARUI_ASSERT() calls assert() by default. Compiling with NDEBUG will usually strip out assert() to nothing, which is NOT recommended because we use asserts to notify of programmer mistakes!");
        }

        LinearUI::Separator();
        LinearUI::Text("io.BackendPlatformName: %s", io.BackendPlatformName ? io.BackendPlatformName : "NULL");
        LinearUI::Text("io.BackendRendererName: %s", io.BackendRendererName ? io.BackendRendererName : "NULL");
        LinearUI::Text("io.ConfigFlags: 0x%08X", io.ConfigFlags);
        if (io.ConfigFlags & LinearUIConfigFlags_NavEnableKeyboard)        LinearUI::Text(" NavEnableKeyboard");
        if (io.ConfigFlags & LinearUIConfigFlags_NavEnableGamepad)         LinearUI::Text(" NavEnableGamepad");
        if (io.ConfigFlags & LinearUIConfigFlags_NoMouse)                  LinearUI::Text(" NoMouse");
        if (io.ConfigFlags & LinearUIConfigFlags_NoMouseCursorChange)      LinearUI::Text(" NoMouseCursorChange");
        if (io.ConfigFlags & LinearUIConfigFlags_NoKeyboard)               LinearUI::Text(" NoKeyboard");
        if (io.ConfigFlags & LinearUIConfigFlags_DockingEnable)            LinearUI::Text(" DockingEnable");
        if (io.ConfigFlags & LinearUIConfigFlags_ViewportsEnable)          LinearUI::Text(" ViewportsEnable");
        if (io.MouseDrawCursor)                                         LinearUI::Text("io.MouseDrawCursor");
        if (io.ConfigDpiScaleFonts)                                     LinearUI::Text("io.ConfigDpiScaleFonts");
        if (io.ConfigDpiScaleViewports)                                 LinearUI::Text("io.ConfigDpiScaleViewports");
        if (io.ConfigViewportsNoAutoMerge)                              LinearUI::Text("io.ConfigViewportsNoAutoMerge");
        if (io.ConfigViewportsNoTaskBarIcon)                            LinearUI::Text("io.ConfigViewportsNoTaskBarIcon");
        if (io.ConfigViewportsNoDecoration)                             LinearUI::Text("io.ConfigViewportsNoDecoration");
        if (io.ConfigViewportsNoDefaultParent)                          LinearUI::Text("io.ConfigViewportsNoDefaultParent");
        if (io.ConfigDockingNoSplit)                                    LinearUI::Text("io.ConfigDockingNoSplit");
        if (io.ConfigDockingNoDockingOver)                              LinearUI::Text("io.ConfigDockingNoDockingOver");
        if (io.ConfigDockingWithShift)                                  LinearUI::Text("io.ConfigDockingWithShift");
        if (io.ConfigDockingAlwaysTabBar)                               LinearUI::Text("io.ConfigDockingAlwaysTabBar");
        if (io.ConfigDockingTransparentPayload)                         LinearUI::Text("io.ConfigDockingTransparentPayload");
        if (io.ConfigMacOSXBehaviors)                                   LinearUI::Text("io.ConfigMacOSXBehaviors");
        if (io.ConfigNavMoveSetMousePos)                                LinearUI::Text("io.ConfigNavMoveSetMousePos");
        if (io.ConfigNavCaptureKeyboard)                                LinearUI::Text("io.ConfigNavCaptureKeyboard");
        if (io.ConfigInputTextCursorBlink)                              LinearUI::Text("io.ConfigInputTextCursorBlink");
        if (io.ConfigWindowsResizeFromEdges)                            LinearUI::Text("io.ConfigWindowsResizeFromEdges");
        if (io.ConfigWindowsMoveFromTitleBarOnly)                       LinearUI::Text("io.ConfigWindowsMoveFromTitleBarOnly");
        if (io.ConfigMemoryCompactTimer >= 0.0f)                        LinearUI::Text("io.ConfigMemoryCompactTimer = %.1f", io.ConfigMemoryCompactTimer);
        LinearUI::Text("io.BackendFlags: 0x%08X", io.BackendFlags);
        if (io.BackendFlags & LinearUIBackendFlags_HasGamepad)             LinearUI::Text(" HasGamepad");
        if (io.BackendFlags & LinearUIBackendFlags_HasMouseCursors)        LinearUI::Text(" HasMouseCursors");
        if (io.BackendFlags & LinearUIBackendFlags_HasSetMousePos)         LinearUI::Text(" HasSetMousePos");
        if (io.BackendFlags & LinearUIBackendFlags_PlatformHasViewports)   LinearUI::Text(" PlatformHasViewports");
        if (io.BackendFlags & LinearUIBackendFlags_HasMouseHoveredViewport)LinearUI::Text(" HasMouseHoveredViewport");
        if (io.BackendFlags & LinearUIBackendFlags_HasParentViewport)      LinearUI::Text(" HasParentViewport");
        if (io.BackendFlags & LinearUIBackendFlags_RendererHasVtxOffset)   LinearUI::Text(" RendererHasVtxOffset");
        if (io.BackendFlags & LinearUIBackendFlags_RendererHasTextures)    LinearUI::Text(" RendererHasTextures");
        if (io.BackendFlags & LinearUIBackendFlags_RendererHasViewports)   LinearUI::Text(" RendererHasViewports");
        LinearUI::Separator();
        LinearUI::Text("io.Fonts: %d fonts, Flags: 0x%08X, TexSize: %d,%d", io.Fonts->Fonts.Size, io.Fonts->Flags, io.Fonts->TexData->Width, io.Fonts->TexData->Height);
        LinearUI::Text("io.Fonts->FontLoaderName: %s", io.Fonts->FontLoaderName ? io.Fonts->FontLoaderName : "NULL");
        LinearUI::Text("io.DisplaySize: %.2f,%.2f", io.DisplaySize.x, io.DisplaySize.y);
        LinearUI::Text("io.DisplayFramebufferScale: %.2f,%.2f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        LinearUI::Separator();
        LinearUI::Text("style.WindowPadding: %.2f,%.2f", style.WindowPadding.x, style.WindowPadding.y);
        LinearUI::Text("style.WindowBorderSize: %.2f", style.WindowBorderSize);
        LinearUI::Text("style.FramePadding: %.2f,%.2f", style.FramePadding.x, style.FramePadding.y);
        LinearUI::Text("style.FrameRounding: %.2f", style.FrameRounding);
        LinearUI::Text("style.FrameBorderSize: %.2f", style.FrameBorderSize);
        LinearUI::Text("style.ItemSpacing: %.2f,%.2f", style.ItemSpacing.x, style.ItemSpacing.y);
        LinearUI::Text("style.ItemInnerSpacing: %.2f,%.2f", style.ItemInnerSpacing.x, style.ItemInnerSpacing.y);

        if (copy_to_clipboard)
        {
            LinearUI::LogText("\n```\n");
            LinearUI::LogFinish();
        }
        LinearUI::EndChild();
    }
    LinearUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Style Editor / ShowStyleEditor()
//-----------------------------------------------------------------------------
// - ShowStyleSelector()
// - ShowStyleEditor()
//-----------------------------------------------------------------------------

// Demo helper function to select among default colors. See ShowStyleEditor() for more advanced options.
bool LinearUI::ShowStyleSelector(const char* label)
{
    // FIXME: This is a bit tricky to get right as style are functions, they don't register a name nor the fact that one is active.
    // So we keep track of last active one among our limited selection.
    static int style_idx = -1;
    const char* style_names[] = { "Dark", "Light", "Classic" };
    bool ret = false;
    if (LinearUI::BeginCombo(label, (style_idx >= 0 && style_idx < LINEARUI_COUNTOF(style_names)) ? style_names[style_idx] : ""))
    {
        for (int n = 0; n < LINEARUI_COUNTOF(style_names); n++)
        {
            if (LinearUI::Selectable(style_names[n], style_idx == n, LinearUISelectableFlags_SelectOnNav))
            {
                style_idx = n;
                ret = true;
                switch (style_idx)
                {
                case 0: LinearUI::StyleColorsDark(); break;
                case 1: LinearUI::StyleColorsLight(); break;
                case 2: LinearUI::StyleColorsClassic(); break;
                }
            }
            else if (style_idx == n)
                LinearUI::SetItemDefaultFocus();
        }
        LinearUI::EndCombo();
    }
    return ret;
}

static const char* GetTreeLinesFlagsName(LinearUITreeNodeFlags flags)
{
    if (flags == LinearUITreeNodeFlags_DrawLinesNone) return "DrawLinesNone";
    if (flags == LinearUITreeNodeFlags_DrawLinesFull) return "DrawLinesFull";
    if (flags == LinearUITreeNodeFlags_DrawLinesToNodes) return "DrawLinesToNodes";
    return "";
}

// We omit the LinearUI:: prefix in this function, as we don't expect user to be copy and pasting this code.
void LinearUI::ShowStyleEditor(LinearUIStyle* ref)
{
    LINEARUI_DEMO_MARKER("Tools/Style Editor");
    // You can pass in a reference LinearUIStyle structure to compare to, revert to and save to
    // (without a reference style pointer, we will use one compared locally as a reference)
    LinearUIStyle& style = GetStyle();
    static LinearUIStyle ref_saved_style;

    // Default to using internal storage as reference
    static bool init = true;
    if (init && ref == NULL)
        ref_saved_style = style;
    init = false;
    if (ref == NULL)
        ref = &ref_saved_style;

    // The logic behind dynamically changing 'max_border_size' is to not encourage people to increase border size too much: it'll likely reveal lots of subtle rendering artifacts and this isn't a priority right now.
    // Note that _MainScale is currently internal PLEASE DO NOT USE IN YOUR CODE.
    const float default_border_size = (float)(int)style._MainScale;
    const float max_border_size = LINEARUI_MAX(default_border_size, 2.0f);

    PushItemWidth(GetWindowWidth() * 0.50f);

    {
        // General
        SeparatorText("General");
        if ((GetIO().BackendFlags & LinearUIBackendFlags_RendererHasTextures) == 0)
        {
            BulletText("Warning: Font scaling will NOT be smooth, because\nLinearUIBackendFlags_RendererHasTextures is not set!");
            BulletText("For instructions, see:");
            SameLine();
            TextLinkOpenURL("docs/BACKENDS.md", "https://github.com/ocornut/linearui/blob/master/docs/BACKENDS.md");
        }

        if (ShowStyleSelector("Colors##Selector"))
            ref_saved_style = style;
        ShowFontSelector("Fonts##Selector");
        if (DragFloat("FontSizeBase", &style.FontSizeBase, 0.20f, 5.0f, 100.0f, "%.0f"))
            style._NextFrameFontSizeBase = style.FontSizeBase; // FIXME: Temporary hack until we finish remaining work.
        SameLine(0.0f, 0.0f); Text(" (out %.2f)", GetFontSize());
        DragFloat("FontScaleMain", &style.FontScaleMain, 0.02f, 0.5f, 4.0f);
        BeginDisabled(GetIO().ConfigDpiScaleFonts);
        DragFloat("FontScaleDpi", &style.FontScaleDpi, 0.02f, 0.5f, 4.0f);
        SetItemTooltip("When io.ConfigDpiScaleFonts is set, this value is automatically overwritten.");
        EndDisabled();

        // Simplified Settings (expose floating-pointer border sizes as boolean representing 0.0f or 1.0f)
        if (SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
            style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
        { bool border = (style.WindowBorderSize > 0.0f); if (Checkbox("WindowBorder", &border)) { style.WindowBorderSize = border ? default_border_size : 0.0f; } }
        SameLine();
        { bool border = (style.FrameBorderSize > 0.0f);  if (Checkbox("FrameBorder", &border)) { style.FrameBorderSize = border ? default_border_size : 0.0f; } }
        SameLine();
        { bool border = (style.PopupBorderSize > 0.0f);  if (Checkbox("PopupBorder", &border)) { style.PopupBorderSize = border ? default_border_size : 0.0f; } }
    }

    // Save/Revert button
    if (Button("Save Ref"))
        *ref = ref_saved_style = style;
    SameLine();
    if (Button("Revert Ref"))
        style = *ref;
    SameLine();
    HelpMarker(
        "Save/Revert in local non-persistent storage. Default Colors definition are not affected. "
        "Use \"Export\" below to save them somewhere.");

    SeparatorText("Details");
    if (BeginTabBar("##tabs", LinearUITabBarFlags_None))
    {
        if (BeginTabItem("Sizes"))
        {
            SeparatorText("Main");
            SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
            SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
            SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
            SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
            SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
            SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
            SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");

            SeparatorText("Borders");
            SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, max_border_size, "%.0f");
            SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, max_border_size, "%.0f");
            SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, max_border_size, "%.0f");
            SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, max_border_size, "%.0f");

            SeparatorText("Rounding");
            SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
            SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
            SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
            SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
            SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");

            SeparatorText("Scrollbar");
            SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
            SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
            SliderFloat("ScrollbarPadding", &style.ScrollbarPadding, 0.0f, 10.0f, "%.0f");

            SeparatorText("Tabs");
            SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, max_border_size, "%.0f");
            SliderFloat("TabBarBorderSize", &style.TabBarBorderSize, 0.0f, max_border_size, "%.0f");
            SliderFloat("TabBarOverlineSize", &style.TabBarOverlineSize, 0.0f, LINEARUI_MAX(3.0f, max_border_size), "%.0f");
            SameLine(); HelpMarker("Overline is only drawn over the selected tab when LinearUITabBarFlags_DrawSelectedOverline is set.");
            DragFloat("TabMinWidthBase", &style.TabMinWidthBase, 0.5f, 1.0f, 500.0f, "%.0f");
            DragFloat("TabMinWidthShrink", &style.TabMinWidthShrink, 0.5f, 1.0f, 500.0f, "%0.f");
            DragFloat("TabCloseButtonMinWidthSelected", &style.TabCloseButtonMinWidthSelected, 0.5f, -1.0f, 100.0f, (style.TabCloseButtonMinWidthSelected < 0.0f) ? "%.0f (Always)" : "%.0f");
            DragFloat("TabCloseButtonMinWidthUnselected", &style.TabCloseButtonMinWidthUnselected, 0.5f, -1.0f, 100.0f, (style.TabCloseButtonMinWidthUnselected < 0.0f) ? "%.0f (Always)" : "%.0f");
            SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");

            SeparatorText("Tables");
            SliderFloat2("CellPadding", (float*)&style.CellPadding, 0.0f, 20.0f, "%.0f");
            SliderAngle("TableAngledHeadersAngle", &style.TableAngledHeadersAngle, -50.0f, +50.0f);
            SliderFloat2("TableAngledHeadersTextAlign", (float*)&style.TableAngledHeadersTextAlign, 0.0f, 1.0f, "%.2f");

            SeparatorText("Trees");
            bool combo_open = BeginCombo("TreeLinesFlags", GetTreeLinesFlagsName(style.TreeLinesFlags));
            SameLine();
            HelpMarker("[Experimental] Tree lines may not work in all situations (e.g. using a clipper) and may incurs slight traversal overhead.\n\nLinearUITreeNodeFlags_DrawLinesFull is faster than LinearUITreeNodeFlags_DrawLinesToNode.");
            if (combo_open)
            {
                const LinearUITreeNodeFlags options[] = { LinearUITreeNodeFlags_DrawLinesNone, LinearUITreeNodeFlags_DrawLinesFull, LinearUITreeNodeFlags_DrawLinesToNodes };
                for (LinearUITreeNodeFlags option : options)
                    if (Selectable(GetTreeLinesFlagsName(option), style.TreeLinesFlags == option))
                        style.TreeLinesFlags = option;
                EndCombo();
            }
            SliderFloat("TreeLinesSize", &style.TreeLinesSize, 0.0f, max_border_size, "%.0f");
            SliderFloat("TreeLinesRounding", &style.TreeLinesRounding, 0.0f, 12.0f, "%.0f");

            SeparatorText("Windows");
            SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
            SliderFloat("WindowBorderHoverPadding", &style.WindowBorderHoverPadding, 1.0f, 20.0f, "%.0f");
            int window_menu_button_position = style.WindowMenuButtonPosition + 1;
            if (Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
                style.WindowMenuButtonPosition = (LinearUIDir)(window_menu_button_position - 1);

            SeparatorText("Widgets");
            SliderFloat("ColorMarkerSize", &style.ColorMarkerSize, 0.0f, 8.0f, "%.0f");
            Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
            SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
            SameLine(); HelpMarker("Alignment applies when a button is larger than its text content.");
            SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f");
            SameLine(); HelpMarker("Alignment applies when a selectable is larger than its text content.");
            SliderFloat("SeparatorSize", &style.SeparatorSize, 0.0f, 10.0f, "%.0f");
            SliderFloat("SeparatorTextBorderSize", &style.SeparatorTextBorderSize, 0.0f, 10.0f, "%.0f");
            SliderFloat2("SeparatorTextAlign", (float*)&style.SeparatorTextAlign, 0.0f, 1.0f, "%.2f");
            SliderFloat2("SeparatorTextPadding", (float*)&style.SeparatorTextPadding, 0.0f, 40.0f, "%.0f");
            SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.0f");
            SliderFloat("ImageRounding", &style.ImageRounding, 0.0f, 12.0f, "%.0f");
            SliderFloat("ImageBorderSize", &style.ImageBorderSize, 0.0f, max_border_size, "%.0f");

            SeparatorText("Docking");
            //SetCursorPosX(GetCursorPosX() + CalcItemWidth() - GetFrameHeight());
            Checkbox("DockingNodeHasCloseButton", &style.DockingNodeHasCloseButton);
            SliderFloat("DockingSeparatorSize", &style.DockingSeparatorSize, 0.0f, 12.0f, "%.0f");

            SeparatorText("Tooltips");
            for (int n = 0; n < 2; n++)
                if (TreeNodeEx(n == 0 ? "HoverFlagsForTooltipMouse" : "HoverFlagsForTooltipNav"))
                {
                    LinearUIHoveredFlags* p = (n == 0) ? &style.HoverFlagsForTooltipMouse : &style.HoverFlagsForTooltipNav;
                    CheckboxFlags("LinearUIHoveredFlags_DelayNone", p, LinearUIHoveredFlags_DelayNone);
                    CheckboxFlags("LinearUIHoveredFlags_DelayShort", p, LinearUIHoveredFlags_DelayShort);
                    CheckboxFlags("LinearUIHoveredFlags_DelayNormal", p, LinearUIHoveredFlags_DelayNormal);
                    CheckboxFlags("LinearUIHoveredFlags_Stationary", p, LinearUIHoveredFlags_Stationary);
                    CheckboxFlags("LinearUIHoveredFlags_NoSharedDelay", p, LinearUIHoveredFlags_NoSharedDelay);
                    TreePop();
                }

            SeparatorText("Misc");
            SliderFloat2("DisplayWindowPadding", (float*)&style.DisplayWindowPadding, 0.0f, 30.0f, "%.0f"); SameLine(); HelpMarker("Apply to regular windows: amount which we enforce to keep visible when moving near edges of your screen.");
            SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f"); SameLine(); HelpMarker("Apply to every windows, menus, popups, tooltips: amount where we avoid displaying contents. Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");

            EndTabItem();
        }

        if (BeginTabItem("Colors"))
        {
            static int output_dest = 0;
            static bool output_only_modified = true;
            if (Button("Export"))
            {
                if (output_dest == 0)
                    LogToClipboard();
                else
                    LogToTTY();
                LogText("LinearVec4* colors = GetStyle().Colors;" LINEARUI_NEWLINE);
                for (int i = 0; i < LinearUICol_COUNT; i++)
                {
                    const LinearVec4& col = style.Colors[i];
                    const char* name = GetStyleColorName(i);
                    if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(LinearVec4)) != 0)
                        LogText("colors[LinearUICol_%s]%*s= LinearVec4(%.2ff, %.2ff, %.2ff, %.2ff);" LINEARUI_NEWLINE,
                            name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
                }
                LogFinish();
            }
            SameLine(); SetNextItemWidth(GetFontSize() * 10); Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
            SameLine(); Checkbox("Only Modified Colors", &output_only_modified);

            static LinearUITextFilter filter;
            filter.Draw("Filter colors", GetFontSize() * 16);

            static LinearUIColorEditFlags alpha_flags = 0;
            if (RadioButton("Opaque", alpha_flags == LinearUIColorEditFlags_AlphaOpaque))       { alpha_flags = LinearUIColorEditFlags_AlphaOpaque; } SameLine();
            if (RadioButton("Alpha",  alpha_flags == LinearUIColorEditFlags_None))              { alpha_flags = LinearUIColorEditFlags_None; } SameLine();
            if (RadioButton("Both",   alpha_flags == LinearUIColorEditFlags_AlphaPreviewHalf))  { alpha_flags = LinearUIColorEditFlags_AlphaPreviewHalf; } SameLine();
            HelpMarker(
                "In the color list:\n"
                "Left-click on color square to open color picker,\n"
                "Right-click to open edit options menu.");

            SetNextWindowSizeConstraints(LinearVec2(0.0f, GetTextLineHeightWithSpacing() * 10), LinearVec2(FLT_MAX, FLT_MAX));
            BeginChild("##colors", LinearVec2(0, 0), LinearUIChildFlags_Borders | LinearUIChildFlags_NavFlattened, LinearUIWindowFlags_AlwaysVerticalScrollbar | LinearUIWindowFlags_AlwaysHorizontalScrollbar);
            PushItemWidth(GetFontSize() * -12);
            for (int i = 0; i < LinearUICol_COUNT; i++)
            {
                const char* name = GetStyleColorName(i);
                if (!filter.PassFilter(name))
                    continue;
                PushID(i);
#ifndef LINEARUI_DISABLE_DEBUG_TOOLS
                if (Button("?"))
                    DebugFlashStyleColor((LinearUICol)i);
                SetItemTooltip("Flash given color to identify places where it is used.");
                SameLine();
#endif
                ColorEdit4("##color", (float*)&style.Colors[i], LinearUIColorEditFlags_AlphaBar | alpha_flags);
                if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(LinearVec4)) != 0)
                {
                    // Tips: in a real user application, you may want to merge and use an icon font into the main font,
                    // so instead of "Save"/"Revert" you'd use icons!
                    // Read the FAQ and docs/FONTS.md about using icon fonts. It's really easy and super convenient!
                    SameLine(0.0f, style.ItemInnerSpacing.x); if (Button("Save")) { ref->Colors[i] = style.Colors[i]; }
                    SameLine(0.0f, style.ItemInnerSpacing.x); if (Button("Revert")) { style.Colors[i] = ref->Colors[i]; }
                }
                SameLine(0.0f, style.ItemInnerSpacing.x);
                TextUnformatted(name);
                PopID();
            }
            PopItemWidth();
            EndChild();

            EndTabItem();
        }

        if (BeginTabItem("Fonts"))
        {
            LinearUIIO& io = GetIO();
            LinearFontAtlas* atlas = io.Fonts;
            ShowFontAtlas(atlas);

            // Post-baking font scaling. Note that this is NOT the nice way of scaling fonts, read below.
            // (we enforce hard clamping manually as by default DragFloat/SliderFloat allows Ctrl+Click text to get out of bounds).
            /*
            SeparatorText("Legacy Scaling");
            const float MIN_SCALE = 0.3f;
            const float MAX_SCALE = 2.0f;
            HelpMarker(
                "Those are old settings provided for convenience.\n"
                "However, the _correct_ way of scaling your UI is currently to reload your font at the designed size, "
                "rebuild the font atlas, and call style.ScaleAllSizes() on a reference LinearUIStyle structure.\n"
                "Using those settings here will give you poor quality results.");
            PushItemWidth(GetFontSize() * 8);
            DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", LinearUISliderFlags_AlwaysClamp); // Scale everything
            //static float window_scale = 1.0f;
            //if (DragFloat("window scale", &window_scale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", LinearUISliderFlags_AlwaysClamp)) // Scale only this window
            //    SetWindowFontScale(window_scale);
            PopItemWidth();
            */

            EndTabItem();
        }

        if (BeginTabItem("Rendering"))
        {
            Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
            SameLine();
            HelpMarker("When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well.");

            Checkbox("Anti-aliased lines use texture", &style.AntiAliasedLinesUseTex);
            SameLine();
            HelpMarker("Faster lines using texture data. Require backend to render with bilinear filtering (not point/nearest filtering).");

            Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
            PushItemWidth(GetFontSize() * 8);
            DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
            if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;

            // When editing the "Circle Segment Max Error" value, draw a preview of its effect on auto-tessellated circles.
            DragFloat("Circle Tessellation Max Error", &style.CircleTessellationMaxError , 0.005f, 0.10f, 5.0f, "%.2f", LinearUISliderFlags_AlwaysClamp);
            const bool show_samples = IsItemActive();
            if (show_samples)
                SetNextWindowPos(GetCursorScreenPos());
            if (show_samples && BeginTooltip())
            {
                TextUnformatted("(R = radius, N = approx number of segments)");
                Spacing();
                LinearDrawList* draw_list = GetWindowDrawList();
                const float min_widget_width = CalcTextSize("R: MMM\nN: MMM").x;
                for (int n = 0; n < 8; n++)
                {
                    const float RAD_MIN = 5.0f;
                    const float RAD_MAX = 70.0f;
                    const float rad = RAD_MIN + (RAD_MAX - RAD_MIN) * (float)n / (8.0f - 1.0f);

                    BeginGroup();

                    // N is not always exact here due to how PathArcTo() function work internally
                    Text("R: %.f\nN: %d", rad, draw_list->_CalcCircleAutoSegmentCount(rad));

                    const float canvas_width = LINEARUI_MAX(min_widget_width, rad * 2.0f);
                    const float offset_x     = floorf(canvas_width * 0.5f);
                    const float offset_y     = floorf(RAD_MAX);

                    const LinearVec2 p1 = GetCursorScreenPos();
                    draw_list->AddCircle(LinearVec2(p1.x + offset_x, p1.y + offset_y), rad, GetColorU32(LinearUICol_Text));
                    Dummy(LinearVec2(canvas_width, RAD_MAX * 2));

                    /*
                    const LinearVec2 p2 = GetCursorScreenPos();
                    draw_list->AddCircleFilled(LinearVec2(p2.x + offset_x, p2.y + offset_y), rad, GetColorU32(LinearUICol_Text));
                    Dummy(LinearVec2(canvas_width, RAD_MAX * 2));
                    */

                    EndGroup();
                    SameLine();
                }
                EndTooltip();
            }
            SameLine();
            HelpMarker("When drawing circle primitives with \"num_segments == 0\" tessellation will be calculated automatically.");

            DragFloat("Global Alpha", &style.Alpha, 0.005f, 0.20f, 1.0f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
            DragFloat("Disabled Alpha", &style.DisabledAlpha, 0.005f, 0.0f, 1.0f, "%.2f"); SameLine(); HelpMarker("Additional alpha multiplier for disabled items (multiply over current value of Alpha).");
            PopItemWidth();

            EndTabItem();
        }

        EndTabBar();
    }
    PopItemWidth();
}

//-----------------------------------------------------------------------------
// [SECTION] User Guide / ShowUserGuide()
//-----------------------------------------------------------------------------

// We omit the LinearUI:: prefix in this function, as we don't expect user to be copy and pasting this code.
void LinearUI::ShowUserGuide()
{
    LinearUIIO& io = GetIO();
    BulletText("Double-click on title bar to collapse window.");
    BulletText(
        "Click and drag on lower corner or border to resize window.\n"
        "(double-click to auto fit window to its contents)");
    BulletText("Ctrl+Click on a slider or drag box to input value as text.");
    BulletText("Tab/Shift+Tab to cycle through keyboard editable fields.");
    BulletText("Ctrl+Tab/Ctrl+Shift+Tab to focus windows.");
    if (io.FontAllowUserScaling)
        BulletText("Ctrl+Mouse Wheel to zoom window contents.");
    BulletText("While inputting text:\n");
    Indent();
    BulletText("Ctrl+Left/Right to word jump.");
    BulletText("Ctrl+A or double-click to select all.");
    BulletText("Ctrl+X/C/V to use clipboard cut/copy/paste.");
    BulletText("Ctrl+Z to undo, Ctrl+Y/Ctrl+Shift+Z to redo.");
    BulletText("Escape to revert.");
    Unindent();
    BulletText("With Keyboard controls enabled:");
    Indent();
    BulletText("Arrow keys or Home/End/PageUp/PageDown to navigate.");
    BulletText("Space to activate a widget.");
    BulletText("Return to input text into a widget.");
    BulletText("Escape to deactivate a widget, close popup,\nexit a child window or the menu layer, clear focus.");
    BulletText("Alt to jump to the menu layer of a window.");
    BulletText("Menu or Shift+F10 to open a context menu.");
    Unindent();
    BulletText("With Gamepad controls enabled:");
    Indent();
    BulletText("D-Pad: Navigate / Tweak / Resize (in Windowing mode).");
    BulletText("%s Face button: Activate / Open / Toggle. Hold: activate with text input.", io.ConfigNavSwapGamepadButtons ? "East" : "South");
    BulletText("%s Face button: Cancel / Close / Exit.", io.ConfigNavSwapGamepadButtons ? "South" : "East");
    BulletText("West Face button: Toggle Menu. Hold for Windowing mode (Focus/Move/Resize windows).");
    BulletText("North Face button: Open Context Menu.");
    BulletText("L1/R1: Tweak Slower/Faster, Focus Previous/Next (in Windowing Mode).");
    Unindent();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
//-----------------------------------------------------------------------------
// - ShowExampleAppMainMenuBar()
// - ShowExampleMenuFile()
//-----------------------------------------------------------------------------

// Demonstrate creating a "main" fullscreen menu bar and populating it.
// Note the difference between BeginMainMenuBar() and BeginMenuBar():
// - BeginMenuBar() = menu-bar inside current window (which needs the LinearUIWindowFlags_MenuBar flag!)
// - BeginMainMenuBar() = helper to create menu-bar-sized window at the top of the main viewport + call BeginMenuBar() into it.
static void ShowExampleAppMainMenuBar()
{
    if (LinearUI::BeginMainMenuBar())
    {
        if (LinearUI::BeginMenu("File"))
        {
            LINEARUI_DEMO_MARKER("Menu/File");
            ShowExampleMenuFile();
            LinearUI::EndMenu();
        }
        if (LinearUI::BeginMenu("Edit"))
        {
            LINEARUI_DEMO_MARKER("Menu/Edit");
            if (LinearUI::MenuItem("Undo", "Ctrl+Z")) {}
            if (LinearUI::MenuItem("Redo", "Ctrl+Y", false, false)) {} // Disabled item
            LinearUI::Separator();
            if (LinearUI::MenuItem("Cut", "Ctrl+X")) {}
            if (LinearUI::MenuItem("Copy", "Ctrl+C")) {}
            if (LinearUI::MenuItem("Paste", "Ctrl+V")) {}
            LinearUI::EndMenu();
        }
        LinearUI::EndMainMenuBar();
    }
}

// Note that shortcuts are currently provided for display only
// (future version will add explicit flags to BeginMenu() to request processing shortcuts)
static void ShowExampleMenuFile()
{
    LINEARUI_DEMO_MARKER("Examples/Menu");
    LinearUI::MenuItem("(demo menu)", NULL, false, false);
    if (LinearUI::MenuItem("New")) {}
    if (LinearUI::MenuItem("Open", "Ctrl+O")) {}
    if (LinearUI::BeginMenu("Open Recent"))
    {
        LinearUI::MenuItem("fish_hat.c");
        LinearUI::MenuItem("fish_hat.inl");
        LinearUI::MenuItem("fish_hat.h");
        if (LinearUI::BeginMenu("More.."))
        {
            LinearUI::MenuItem("Hello");
            LinearUI::MenuItem("Sailor");
            if (LinearUI::BeginMenu("Recurse.."))
            {
                ShowExampleMenuFile();
                LinearUI::EndMenu();
            }
            LinearUI::EndMenu();
        }
        LinearUI::EndMenu();
    }
    if (LinearUI::MenuItem("Save", "Ctrl+S")) {}
    if (LinearUI::MenuItem("Save As..")) {}

    LinearUI::Separator();
    if (LinearUI::BeginMenu("Options"))
    {
        LINEARUI_DEMO_MARKER("Examples/Menu/Options");
        static bool enabled = true;
        LinearUI::MenuItem("Enabled", "", &enabled);
        LinearUI::BeginChild("child", LinearVec2(0, LinearUI::GetTextLineHeightWithSpacing() * 5.0f), LinearUIChildFlags_Borders);
        for (int i = 0; i < 10; i++)
            LinearUI::Text("Scrolling Text %d", i);
        LinearUI::EndChild();
        static float f = 0.5f;
        static int n = 0;
        LinearUI::SliderFloat("Value", &f, 0.0f, 1.0f);
        LinearUI::InputFloat("Input", &f, 0.1f);
        LinearUI::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        LinearUI::EndMenu();
    }

    if (LinearUI::BeginMenu("Colors"))
    {
        LINEARUI_DEMO_MARKER("Examples/Menu/Colors");
        float sz = LinearUI::GetTextLineHeight();
        for (int i = 0; i < LinearUICol_COUNT; i++)
        {
            const char* name = LinearUI::GetStyleColorName((LinearUICol)i);
            LinearVec2 p = LinearUI::GetCursorScreenPos();
            LinearUI::GetWindowDrawList()->AddRectFilled(p, LinearVec2(p.x + sz, p.y + sz), LinearUI::GetColorU32((LinearUICol)i));
            LinearUI::Dummy(LinearVec2(sz, sz));
            LinearUI::SameLine();
            LinearUI::MenuItem(name);
        }
        LinearUI::EndMenu();
    }

    // Here we demonstrate appending again to the "Options" menu (which we already created above)
    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
    // In a real code-base using it would make senses to use this feature from very different code locations.
    if (LinearUI::BeginMenu("Options")) // <-- Append!
    {
        LINEARUI_DEMO_MARKER("Examples/Menu/Append to an existing menu");
        static bool b = true;
        LinearUI::Checkbox("SomeOption", &b);
        LinearUI::EndMenu();
    }

    if (LinearUI::BeginMenu("Disabled", false)) // Disabled
    {
        LINEARUI_ASSERT(0);
    }
    if (LinearUI::MenuItem("Checked", NULL, true)) {}
    LinearUI::Separator();
    if (LinearUI::MenuItem("Quit", "Alt+F4")) {}
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple console window, with scrolling, filtering, completion and history.
// For the console example, we are using a more C++ like approach of declaring a class to hold both data and functions.
struct ExampleAppConsole
{
    char                  InputBuf[256];
    LinearVector<char*>       Items;
    LinearVector<const char*> Commands;
    LinearVector<char*>       History;
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    LinearUITextFilter       Filter;
    bool                  AutoScroll;
    bool                  ScrollToBottom;

    ExampleAppConsole()
    {
        ClearLog();
        memset(InputBuf, 0, sizeof(InputBuf));
        HistoryPos = -1;

        // "CLASSIFY" is here to provide the test case where "C"+[tab] completes to "CL" and display multiple matches.
        Commands.push_back("HELP");
        Commands.push_back("HISTORY");
        Commands.push_back("CLEAR");
        Commands.push_back("CLASSIFY");
        AutoScroll = true;
        ScrollToBottom = false;
        AddLog("Welcome to Dear LinearUI!");
    }
    ~ExampleAppConsole()
    {
        ClearLog();
        for (int i = 0; i < History.Size; i++)
            LinearUI::MemFree(History[i]);
    }

    // Portable helpers
    static int   Stricmp(const char* s1, const char* s2)         { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
    static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
    static char* Strdup(const char* s)                           { LINEARUI_ASSERT(s); size_t len = strlen(s) + 1; void* buf = LinearUI::MemAlloc(len); LINEARUI_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
    static void  Strtrim(char* s)                                { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

    void    ClearLog()
    {
        for (int i = 0; i < Items.Size; i++)
            LinearUI::MemFree(Items[i]);
        Items.clear();
    }

    void    AddLog(const char* fmt, ...) LINEARUI_FMTARGS(2)
    {
        // FIXME-OPT
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, LINEARUI_COUNTOF(buf), fmt, args);
        buf[LINEARUI_COUNTOF(buf)-1] = 0;
        va_end(args);
        Items.push_back(Strdup(buf));
    }

    void    Draw(const char* title, bool* p_open)
    {
        LinearUI::SetNextWindowSize(LinearVec2(520, 600), LinearUICond_FirstUseEver);
        if (!LinearUI::Begin(title, p_open))
        {
            LinearUI::End();
            return;
        }
        LINEARUI_DEMO_MARKER("Examples/Console");

        // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
        // So e.g. IsItemHovered() will return true when hovering the title bar.
        // Here we create a context menu only available from the title bar.
        if (LinearUI::BeginPopupContextItem())
        {
            if (LinearUI::MenuItem("Close Console"))
                *p_open = false;
            LinearUI::EndPopup();
        }

        LinearUI::TextWrapped(
            "This example implements a console with basic coloring, completion (TAB key) and history (Up/Down keys). A more elaborate "
            "implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
        LinearUI::TextWrapped("Enter 'HELP' for help.");

        // TODO: display items starting from the bottom

        if (LinearUI::SmallButton("Add Debug Text"))  { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); }
        LinearUI::SameLine();
        if (LinearUI::SmallButton("Add Debug Error")) { AddLog("[error] something went wrong"); }
        LinearUI::SameLine();
        if (LinearUI::SmallButton("Clear"))           { ClearLog(); }
        LinearUI::SameLine();
        bool copy_to_clipboard = LinearUI::SmallButton("Copy");
        //static float t = 0.0f; if (LinearUI::GetTime() - t > 0.02f) { t = LinearUI::GetTime(); AddLog("Spam %f", t); }

        LinearUI::Separator();

        // Options menu
        if (LinearUI::BeginPopup("Options"))
        {
            LinearUI::Checkbox("Auto-scroll", &AutoScroll);
            LinearUI::EndPopup();
        }

        // Options, Filter
        LinearUI::SetNextItemShortcut(LinearUIMod_Ctrl | LinearUIKey_O, LinearUIInputFlags_Tooltip);
        if (LinearUI::Button("Options"))
            LinearUI::OpenPopup("Options");
        LinearUI::SameLine();
        Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
        LinearUI::Separator();

        // Reserve enough left-over height for 1 separator + 1 input text
        LinearUIStyle& style = LinearUI::GetStyle();
        const float footer_height_to_reserve = style.SeparatorSize + style.ItemSpacing.y + LinearUI::GetFrameHeightWithSpacing();
        if (LinearUI::BeginChild("ScrollingRegion", LinearVec2(0, -footer_height_to_reserve), LinearUIChildFlags_NavFlattened, LinearUIWindowFlags_HorizontalScrollbar))
        {
            if (LinearUI::BeginPopupContextWindow())
            {
                if (LinearUI::Selectable("Clear")) ClearLog();
                LinearUI::EndPopup();
            }

            // Display every line as a separate entry so we can change their color or add custom widgets.
            // If you only want raw text you can use LinearUI::TextUnformatted(log.begin(), log.end());
            // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
            // to only process visible items. The clipper will automatically measure the height of your first item and then
            // "seek" to display only items in the visible area.
            // To use the clipper we can replace your standard loop:
            //      for (int i = 0; i < Items.Size; i++)
            //   With:
            //      LinearUIListClipper clipper;
            //      clipper.Begin(Items.Size);
            //      while (clipper.Step())
            //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            // - That your items are evenly spaced (same height)
            // - That you have cheap random access to your elements (you can access them given their index,
            //   without processing all the ones before)
            // You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
            // We would need random-access on the post-filtered list.
            // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
            // or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
            // and appending newly elements as they are inserted. This is left as a task to the user until we can manage
            // to improve this example code!
            // If your items are of variable height:
            // - Split them into same height items would be simpler and facilitate random-seeking into your list.
            // - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
            LinearUI::PushStyleVar(LinearUIStyleVar_ItemSpacing, LinearVec2(4, 1)); // Tighten spacing
            if (copy_to_clipboard)
                LinearUI::LogToClipboard();
            for (const char* item : Items)
            {
                if (!Filter.PassFilter(item))
                    continue;

                // Normally you would store more information in your item than just a string.
                // (e.g. make Items[] an array of structure, store color/type etc.)
                LinearVec4 color;
                bool has_color = false;
                if (strstr(item, "[error]")) { color = LinearVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
                else if (strncmp(item, "# ", 2) == 0) { color = LinearVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
                if (has_color)
                    LinearUI::PushStyleColor(LinearUICol_Text, color);
                LinearUI::TextUnformatted(item);
                if (has_color)
                    LinearUI::PopStyleColor();
            }
            if (copy_to_clipboard)
                LinearUI::LogFinish();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (ScrollToBottom || (AutoScroll && LinearUI::GetScrollY() >= LinearUI::GetScrollMaxY()))
                LinearUI::SetScrollHereY(1.0f);
            ScrollToBottom = false;

            LinearUI::PopStyleVar();
        }
        LinearUI::EndChild();
        LinearUI::Separator();

        // Command-line
        bool reclaim_focus = false;
        LinearUIInputTextFlags input_text_flags = LinearUIInputTextFlags_EnterReturnsTrue | LinearUIInputTextFlags_EscapeClearsAll | LinearUIInputTextFlags_CallbackCompletion | LinearUIInputTextFlags_CallbackHistory;
        if (LinearUI::InputText("Input", InputBuf, LINEARUI_COUNTOF(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
        {
            char* s = InputBuf;
            Strtrim(s);
            if (s[0])
                ExecCommand(s);
            strcpy(s, "");
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        LinearUI::SetItemDefaultFocus();
        if (reclaim_focus)
            LinearUI::SetKeyboardFocusHere(-1); // Auto focus previous widget

        LinearUI::End();
    }

    void    ExecCommand(const char* command_line)
    {
        AddLog("# %s\n", command_line);

        // Insert into history. First find match and delete it so it can be pushed to the back.
        // This isn't trying to be smart or optimal.
        HistoryPos = -1;
        for (int i = History.Size - 1; i >= 0; i--)
            if (Stricmp(History[i], command_line) == 0)
            {
                LinearUI::MemFree(History[i]);
                History.erase(History.begin() + i);
                break;
            }
        History.push_back(Strdup(command_line));

        // Process command
        if (Stricmp(command_line, "CLEAR") == 0)
        {
            ClearLog();
        }
        else if (Stricmp(command_line, "HELP") == 0)
        {
            AddLog("Commands:");
            for (int i = 0; i < Commands.Size; i++)
                AddLog("- %s", Commands[i]);
        }
        else if (Stricmp(command_line, "HISTORY") == 0)
        {
            int first = History.Size - 10;
            for (int i = first > 0 ? first : 0; i < History.Size; i++)
                AddLog("%3d: %s\n", i, History[i]);
        }
        else
        {
            AddLog("Unknown command: '%s'\n", command_line);
        }

        // On command input, we scroll to bottom even if AutoScroll==false
        ScrollToBottom = true;
    }

    // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
    static int TextEditCallbackStub(LinearUIInputTextCallbackData* data)
    {
        ExampleAppConsole* console = (ExampleAppConsole*)data->UserData;
        return console->TextEditCallback(data);
    }

    int     TextEditCallback(LinearUIInputTextCallbackData* data)
    {
        //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag)
        {
        case LinearUIInputTextFlags_CallbackCompletion:
            {
                // Example of TEXT COMPLETION

                // Locate beginning of current word
                const char* word_end = data->Buf + data->CursorPos;
                const char* word_start = word_end;
                while (word_start > data->Buf)
                {
                    const char c = word_start[-1];
                    if (c == ' ' || c == '\t' || c == ',' || c == ';')
                        break;
                    word_start--;
                }

                // Build a list of candidates
                LinearVector<const char*> candidates;
                for (int i = 0; i < Commands.Size; i++)
                    if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
                        candidates.push_back(Commands[i]);

                if (candidates.Size == 0)
                {
                    // No match
                    AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
                }
                else if (candidates.Size == 1)
                {
                    // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0]);
                    data->InsertChars(data->CursorPos, " ");
                }
                else
                {
                    // Multiple matches. Complete as much as we can..
                    // So inputting "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                    int match_len = (int)(word_end - word_start);
                    for (;;)
                    {
                        int c = 0;
                        bool all_candidates_matches = true;
                        for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                            if (i == 0)
                                c = toupper(candidates[i][match_len]);
                            else if (c == 0 || c != toupper(candidates[i][match_len]))
                                all_candidates_matches = false;
                        if (!all_candidates_matches)
                            break;
                        match_len++;
                    }

                    if (match_len > 0)
                    {
                        data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                        data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                    }

                    // List matches
                    AddLog("Possible matches:\n");
                    for (int i = 0; i < candidates.Size; i++)
                        AddLog("- %s\n", candidates[i]);
                }

                break;
            }
        case LinearUIInputTextFlags_CallbackHistory:
            {
                // Example of HISTORY
                const int prev_history_pos = HistoryPos;
                if (data->EventKey == LinearUIKey_UpArrow)
                {
                    if (HistoryPos == -1)
                        HistoryPos = History.Size - 1;
                    else if (HistoryPos > 0)
                        HistoryPos--;
                }
                else if (data->EventKey == LinearUIKey_DownArrow)
                {
                    if (HistoryPos != -1)
                        if (++HistoryPos >= History.Size)
                            HistoryPos = -1;
                }

                // A better implementation would preserve the data on the current input line along with cursor position.
                if (prev_history_pos != HistoryPos)
                {
                    const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, history_str);
                }
            }
        }
        return 0;
    }
};

static void ShowExampleAppConsole(bool* p_open)
{
    static ExampleAppConsole console;
    console.Draw("Example: Console", p_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Image Viewer / ShowExampleAppImageViewer()
//-----------------------------------------------------------------------------

static void ShowExampleAppImageViewer(bool* p_open)
{
    LinearFontAtlas* atlas = LinearUI::GetIO().Fonts;
    LinearTextureRef tex_ref = atlas->TexRef; // We don't have access to other textures in this demo!
    int tex_w = atlas->TexData->Width;
    int tex_h = atlas->TexData->Height;
    if (LinearUI::Begin("Example: Image Viewer", p_open))
    {
        static ExampleImageViewerData image_viewer;
        ExampleImageViewer_DrawOptions(&image_viewer);
        LinearVec2 canvas_size = LinearUI::GetContentRegionAvail();
        LinearVec2 canvas_min_size = LinearUI::IsWindowAppearing() ? LinearVec2(3.0f * tex_w, 4.0f * tex_h) : LinearVec2(1.0f, 1.0f);
        canvas_size = LinearVec2(LINEARUI_MAX(canvas_size.x, canvas_min_size.x), LINEARUI_MAX(canvas_size.y, canvas_min_size.y));
        ExampleImageViewer_DrawCanvas(&image_viewer, canvas_size, tex_ref, tex_w, tex_h);
    }
    LinearUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Log / ShowExampleAppLog()
//-----------------------------------------------------------------------------

// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
struct ExampleAppLog
{
    LinearUITextBuffer     Buf;
    LinearUITextFilter     Filter;
    LinearVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    bool                AutoScroll;  // Keep scrolling if already at the bottom.

    ExampleAppLog()
    {
        AutoScroll = true;
        Clear();
    }

    void    Clear()
    {
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
    }

    void    AddLog(const char* fmt, ...) LINEARUI_FMTARGS(2)
    {
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = Buf.size(); old_size < new_size; old_size++)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size + 1);
    }

    void    Draw(const char* title, bool* p_open = NULL)
    {
        if (!LinearUI::Begin(title, p_open))
        {
            LinearUI::End();
            return;
        }

        // Options menu
        if (LinearUI::BeginPopup("Options"))
        {
            LinearUI::Checkbox("Auto-scroll", &AutoScroll);
            LinearUI::EndPopup();
        }

        // Main window
        if (LinearUI::Button("Options"))
            LinearUI::OpenPopup("Options");
        LinearUI::SameLine();
        bool clear = LinearUI::Button("Clear");
        LinearUI::SameLine();
        bool copy = LinearUI::Button("Copy");
        LinearUI::SameLine();
        Filter.Draw("Filter", -100.0f);

        LinearUI::Separator();

        if (LinearUI::BeginChild("scrolling", LinearVec2(0, 0), LinearUIChildFlags_None, LinearUIWindowFlags_HorizontalScrollbar))
        {
            if (clear)
                Clear();
            if (copy)
                LinearUI::LogToClipboard();

            LinearUI::PushStyleVar(LinearUIStyleVar_ItemSpacing, LinearVec2(0, 0));
            const char* buf = Buf.begin();
            const char* buf_end = Buf.end();
            if (Filter.IsActive())
            {
                // In this example we don't use the clipper when Filter is enabled.
                // This is because we don't have random access to the result of our filter.
                // A real application processing logs with ten of thousands of entries may want to store the result of
                // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
                for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    if (Filter.PassFilter(line_start, line_end))
                        LinearUI::TextUnformatted(line_start, line_end);
                }
            }
            else
            {
                // The simplest and easy way to display the entire buffer:
                //   LinearUI::TextUnformatted(buf_begin, buf_end);
                // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
                // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
                // within the visible area.
                // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
                // on your side is recommended. Using LinearUIListClipper requires
                // - A) random access into your data
                // - B) items all being the  same height,
                // both of which we can handle since we have an array pointing to the beginning of each line of text.
                // When using the filter (in the block of code above) we don't have random access into the data to display
                // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
                // it possible (and would be recommended if you want to search through tens of thousands of entries).
                LinearUIListClipper clipper;
                clipper.Begin(LineOffsets.Size);
                while (clipper.Step())
                {
                    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                    {
                        const char* line_start = buf + LineOffsets[line_no];
                        const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                        LinearUI::TextUnformatted(line_start, line_end);
                    }
                }
                clipper.End();
            }
            LinearUI::PopStyleVar();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (AutoScroll && LinearUI::GetScrollY() >= LinearUI::GetScrollMaxY())
                LinearUI::SetScrollHereY(1.0f);
        }
        LinearUI::EndChild();
        LinearUI::End();
    }
};

// Demonstrate creating a simple log window with basic filtering.
static void ShowExampleAppLog(bool* p_open)
{
    static ExampleAppLog log;

    // For the demo: add a debug button _BEFORE_ the normal log window contents
    // We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
    // Most of the contents of the window will be added by the log.Draw() call.
    LinearUI::SetNextWindowSize(LinearVec2(500, 400), LinearUICond_FirstUseEver);
    LinearUI::Begin("Example: Log", p_open);
    LINEARUI_DEMO_MARKER("Examples/Log");
    if (LinearUI::SmallButton("[Debug] Add 5 entries"))
    {
        static int counter = 0;
        const char* categories[3] = { "info", "warn", "error" };
        const char* words[] = { "Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", "Pauciloquent" };
        for (int n = 0; n < 5; n++)
        {
            const char* category = categories[counter % LINEARUI_COUNTOF(categories)];
            const char* word = words[counter % LINEARUI_COUNTOF(words)];
            log.AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
                LinearUI::GetFrameCount(), category, LinearUI::GetTime(), word);
            counter++;
        }
    }
    LinearUI::End();

    // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
    log.Draw("Example: Log", p_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
//-----------------------------------------------------------------------------

// Demonstrate create a window with multiple child windows.
static void ShowExampleAppLayout(bool* p_open)
{
    LinearUI::SetNextWindowSize(LinearVec2(500, 440), LinearUICond_FirstUseEver);
    if (LinearUI::Begin("Example: Simple layout", p_open, LinearUIWindowFlags_MenuBar))
    {
        LINEARUI_DEMO_MARKER("Examples/Simple layout");
        if (LinearUI::BeginMenuBar())
        {
            if (LinearUI::BeginMenu("File"))
            {
                if (LinearUI::MenuItem("Close", "Ctrl+W")) { *p_open = false; }
                LinearUI::EndMenu();
            }
            LinearUI::EndMenuBar();
        }

        // Left
        static int selected = 0;
        {
            LinearUI::BeginChild("left pane", LinearVec2(150, 0), LinearUIChildFlags_Borders | LinearUIChildFlags_ResizeX);
            for (int i = 0; i < 100; i++)
            {
                char label[128];
                sprintf(label, "MyObject %d", i);
                if (LinearUI::Selectable(label, selected == i, LinearUISelectableFlags_SelectOnNav))
                    selected = i;
            }
            LinearUI::EndChild();
        }
        LinearUI::SameLine();

        // Right
        {
            LinearUI::BeginGroup();
            LinearUI::BeginChild("item view", LinearVec2(0, -LinearUI::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
            LinearUI::Text("MyObject: %d", selected);
            LinearUI::Separator();
            if (LinearUI::BeginTabBar("##Tabs", LinearUITabBarFlags_None))
            {
                if (LinearUI::BeginTabItem("Description"))
                {
                    LinearUI::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
                    LinearUI::EndTabItem();
                }
                if (LinearUI::BeginTabItem("Details"))
                {
                    LinearUI::Text("ID: 0123456789");
                    LinearUI::EndTabItem();
                }
                LinearUI::EndTabBar();
            }
            LinearUI::EndChild();
            if (LinearUI::Button("Revert")) {}
            LinearUI::SameLine();
            if (LinearUI::Button("Save")) {}
            LinearUI::EndGroup();
        }
    }
    LinearUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
//-----------------------------------------------------------------------------
// Some of the interactions are a bit lack-luster:
// - We would want pressing validating or leaving the filter to somehow restore focus.
// - We may want more advanced filtering (child nodes) and clipper support: both will need extra work.
// - We would want to customize some keyboard interactions to easily keyboard navigate between the tree and the properties.
//-----------------------------------------------------------------------------

struct ExampleAppPropertyEditor
{
    LinearUITextFilter     Filter;
    ExampleTreeNode*    SelectedNode = NULL;
    bool                UseClipper = false;

    void Draw(ExampleTreeNode* root_node)
    {
        LINEARUI_DEMO_MARKER("Examples/Property editor");

        // Left side: draw tree
        // - Currently using a table to benefit from RowBg feature
        // - Our tree node are all of equal height, facilitating the use of a clipper.
        if (LinearUI::BeginChild("##tree", LinearVec2(300, 0), LinearUIChildFlags_ResizeX | LinearUIChildFlags_Borders | LinearUIChildFlags_NavFlattened))
        {
            LinearUI::PushItemFlag(LinearUIItemFlags_NoNavDefaultFocus, true);
            LinearUI::Checkbox("Use Clipper", &UseClipper);
            LinearUI::SameLine();
            LinearUI::Text("(%d root nodes)", root_node->Childs.Size);
            LinearUI::SetNextItemWidth(-FLT_MIN);
            LinearUI::SetNextItemShortcut(LinearUIMod_Ctrl | LinearUIKey_F, LinearUIInputFlags_Tooltip);
            if (LinearUI::InputTextWithHint("##Filter", "incl,-excl", Filter.InputBuf, LINEARUI_COUNTOF(Filter.InputBuf), LinearUIInputTextFlags_EscapeClearsAll))
                Filter.Build();
            LinearUI::PopItemFlag();

            if (LinearUI::BeginTable("##list", 1, LinearUITableFlags_RowBg))
            {
                if (UseClipper)
                    DrawClippedTree(root_node);
                else
                    DrawTree(root_node);
                LinearUI::EndTable();
            }
        }
        LinearUI::EndChild();

        // Right side: draw properties
        LinearUI::SameLine();

        LinearUI::BeginGroup(); // Lock X position
        if (ExampleTreeNode* node = SelectedNode)
        {
            LinearUI::Text("%s", node->Name);
            LinearUI::TextDisabled("UID: 0x%08X", node->UID);
            LinearUI::Separator();
            if (LinearUI::BeginTable("##properties", 2, LinearUITableFlags_Resizable | LinearUITableFlags_ScrollY))
            {
                // Push object ID after we entered the table, so table is shared for all objects
                LinearUI::PushID((int)node->UID);
                LinearUI::TableSetupColumn("", LinearUITableColumnFlags_WidthFixed);
                LinearUI::TableSetupColumn("", LinearUITableColumnFlags_WidthStretch, 2.0f); // Default twice larger
                if (node->HasData)
                {
                    // In a typical application, the structure description would be derived from a data-driven system.
                    // - We try to mimic this with our ExampleMemberInfo structure and the ExampleTreeNodeMemberInfos[] array.
                    // - Limits and some details are hard-coded to simplify the demo.
                    for (const ExampleMemberInfo& field_desc : ExampleTreeNodeMemberInfos)
                    {
                        LinearUI::TableNextRow();
                        LinearUI::PushID(field_desc.Name);
                        LinearUI::TableNextColumn();
                        LinearUI::AlignTextToFramePadding();
                        LinearUI::TextUnformatted(field_desc.Name);
                        LinearUI::TableNextColumn();
                        void* field_ptr = (void*)(((unsigned char*)node) + field_desc.Offset);
                        switch (field_desc.DataType)
                        {
                        case LinearUIDataType_Bool:
                        {
                            LINEARUI_ASSERT(field_desc.DataCount == 1);
                            LinearUI::Checkbox("##Editor", (bool*)field_ptr);
                            break;
                        }
                        case LinearUIDataType_S32:
                        {
                            int v_min = INT_MIN, v_max = INT_MAX;
                            LinearUI::SetNextItemWidth(-FLT_MIN);
                            LinearUI::DragScalarN("##Editor", field_desc.DataType, field_ptr, field_desc.DataCount, 1.0f, &v_min, &v_max);
                            break;
                        }
                        case LinearUIDataType_Float:
                        {
                            float v_min = 0.0f, v_max = 1.0f;
                            LinearUI::SetNextItemWidth(-FLT_MIN);
                            LinearUI::SliderScalarN("##Editor", field_desc.DataType, field_ptr, field_desc.DataCount, &v_min, &v_max);
                            break;
                        }
                        case LinearUIDataType_String:
                        {
                            LinearUI::InputText("##Editor", reinterpret_cast<char*>(field_ptr), 28);
                            break;
                        }
                        }
                        LinearUI::PopID();
                    }
                }
                LinearUI::PopID();
                LinearUI::EndTable();
            }
        }
        LinearUI::EndGroup();
    }

    // Custom search filter
    // - Here we apply on root node only.
    // - This does a case insensitive stristr which is pretty heavy. In a real large-scale app you would likely store a filtered list which in turns would be trivial to linearize.
    inline bool IsNodePassingFilter(ExampleTreeNode* node)
    {
        return node->Parent->Parent != NULL || Filter.PassFilter(node->Name);
    }

    // Basic version, recursive. This is how you would generally draw a tree.
    // - Simple but going to be noticeably costly if you have a large amount of nodes as DrawTreeNode() is called for all of them.
    // - On my desktop PC (2020), for 10K nodes in an optimized build this takes ~1.2 ms
    // - Unlike arrays or grids which are very easy to clip, trees are currently more difficult to clip.
    void DrawTree(ExampleTreeNode* node)
    {
        for (ExampleTreeNode* child : node->Childs)
            if (IsNodePassingFilter(child) && DrawTreeNode(child))
            {
                DrawTree(child);
                LinearUI::TreePop();
            }
    }

    // More advanced version. Use a alternative clipping technique: fast-forwarding through non-visible chunks.
    // - On my desktop PC (2020), for 10K nodes in an optimized build this takes ~0.1 ms
    //   (in ExampleTree_CreateDemoTree(), change 'int ROOT_ITEMS_COUNT = 10000' to try with this amount of root nodes).
    // - 1. Use clipper with indeterminate count (items_count = INT_MAX): we need to call SeekCursorForItem() at the end once we know the count.
    // - 2. Use SetNextItemStorageID() to specify ID used for open/close storage, making it easy to call TreeNodeGetOpen() on any arbitrary node.
    // - 3. Linearize tree during traversal: our tree data structure makes it easy to access sibling and parents.
    // - Unlike clipping for a regular array or grid which may be done using random access limited to visible areas,
    //   this technique requires traversing most accessible nodes. This could be made more optimal with extra work,
    //   but this is a decent simplicity<>speed trade-off.
    // See https://github.com/ocornut/linearui/issues/3823 for discussions about this.
    void DrawClippedTree(ExampleTreeNode* root_node)
    {
        ExampleTreeNode* node = root_node->Childs[0]; // First node
        LinearUIListClipper clipper;
        clipper.Begin(INT_MAX);
        while (clipper.Step())
            while (clipper.UserIndex < clipper.DisplayEnd && node != NULL)
                node = DrawClippedTreeNodeAndAdvanceToNext(&clipper, node);

        // Keep going to count nodes and submit final count so we have a reliable scrollbar.
        // - One could consider caching this value and only refreshing it occasionally e.g. window is focused and an action occurs.
        // - Incorrect but cheap approximation would be to use 'clipper_current_idx = LINEARUI_MAX(clipper_current_idx, root_node->Childs.Size)' instead.
        // - If either of those is implemented, the general cost will approach zero when scrolling is at the top of the tree.
        while (node != NULL)
            node = DrawClippedTreeNodeAndAdvanceToNext(&clipper, node);
        //clipper.UserIndex = LINEARUI_MAX(clipper.UserIndex, root_node->Childs.Size); // <-- Cheap approximation instead of while() loop above.
        clipper.SeekCursorForItem(clipper.UserIndex);
    }

    ExampleTreeNode* DrawClippedTreeNodeAndAdvanceToNext(LinearUIListClipper* clipper, ExampleTreeNode* node)
    {
        if (IsNodePassingFilter(node))
        {
            // Draw node if within visible range
            bool is_open = false;
            if (clipper->UserIndex >= clipper->DisplayStart && clipper->UserIndex < clipper->DisplayEnd)
            {
                is_open = DrawTreeNode(node);
            }
            else
            {
                is_open = (node->Childs.Size > 0 && LinearUI::TreeNodeGetOpen((LinearUIID)node->UID));
                if (is_open)
                    LinearUI::TreePush(node->Name);
            }
            clipper->UserIndex++;

            // Next node: recurse into childs
            if (is_open)
                return node->Childs[0];
        }

        // Next node: next sibling, otherwise move back to parent
        while (node != NULL)
        {
            if (node->IndexInParent + 1 < node->Parent->Childs.Size)
                return node->Parent->Childs[node->IndexInParent + 1];
            node = node->Parent;
            if (node->Parent == NULL)
                break;
            LinearUI::TreePop();
        }
        return NULL;
    }

    // To support node with same name we incorporate node->UID into the item ID.
    // (this would more naturally be done using PushID(node->UID) + TreeNodeEx(node->Name, tree_flags),
    //   but it would require in DrawClippedTreeNodeAndAdvanceToNext() to add PushID() before TreePush(), and PopID() after TreePop(),
    //   so instead we use TreeNodeEx(node->UID, tree_flags, "%s", node->Name) here)
    bool DrawTreeNode(ExampleTreeNode* node)
    {
        LinearUI::TableNextRow();
        LinearUI::TableNextColumn();
        LinearUITreeNodeFlags tree_flags = LinearUITreeNodeFlags_None;
        tree_flags |= LinearUITreeNodeFlags_OpenOnArrow | LinearUITreeNodeFlags_OpenOnDoubleClick; // Standard opening mode as we are likely to want to add selection afterwards
        tree_flags |= LinearUITreeNodeFlags_NavLeftJumpsToParent;  // Left arrow support
        tree_flags |= LinearUITreeNodeFlags_SpanFullWidth;         // Span full width for easier mouse reach
        tree_flags |= LinearUITreeNodeFlags_DrawLinesToNodes;      // Always draw hierarchy outlines
        if (node == SelectedNode)
            tree_flags |= LinearUITreeNodeFlags_Selected;          // Draw selection highlight
        if (node->Childs.Size == 0)
            tree_flags |= LinearUITreeNodeFlags_Leaf | LinearUITreeNodeFlags_Bullet | LinearUITreeNodeFlags_NoTreePushOnOpen; // Use _NoTreePushOnOpen + set is_open=false to avoid unnecessarily push/pop on leaves.
        if (node->DataMyBool == false)
            LinearUI::PushStyleColor(LinearUICol_Text, LinearUI::GetStyle().Colors[LinearUICol_TextDisabled]);
        LinearUI::SetNextItemStorageID((LinearUIID)node->UID);        // Use node->UID as storage id
        bool is_open = LinearUI::TreeNodeEx((void*)(intptr_t)node->UID, tree_flags, "%s", node->Name);
        if (node->Childs.Size == 0)
            is_open = false;
        if (node->DataMyBool == false)
            LinearUI::PopStyleColor();
        if (LinearUI::IsItemFocused())
            SelectedNode = node;
        return is_open;
    }
};

// Demonstrate creating a simple property editor.
static void ShowExampleAppPropertyEditor(bool* p_open, LinearUIDemoWindowData* demo_data)
{
    LinearUI::SetNextWindowSize(LinearVec2(430, 450), LinearUICond_FirstUseEver);
    if (!LinearUI::Begin("Example: Property editor", p_open))
    {
        LinearUI::End();
        return;
    }

    LINEARUI_DEMO_MARKER("Examples/Property Editor");
    static ExampleAppPropertyEditor property_editor;
    if (demo_data->DemoTree == NULL)
        demo_data->DemoTree = ExampleTree_CreateDemoTree();
    property_editor.Draw(demo_data->DemoTree);

    LinearUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Long Text / ShowExampleAppLongText()
//-----------------------------------------------------------------------------

// Demonstrate/test rendering huge amount of text, and the incidence of clipping.
static void ShowExampleAppLongText(bool* p_open)
{
    LinearUI::SetNextWindowSize(LinearVec2(520, 600), LinearUICond_FirstUseEver);
    if (!LinearUI::Begin("Example: Long text display", p_open))
    {
        LinearUI::End();
        return;
    }
    LINEARUI_DEMO_MARKER("Examples/Long text display");

    static int test_type = 0;
    static LinearUITextBuffer log;
    static int lines = 0;
    LinearUI::Text("Printing unusually long amount of text.");
    LinearUI::Combo("Test type", &test_type,
        "Single call to TextUnformatted()\0"
        "Multiple calls to Text(), clipped\0"
        "Multiple calls to Text(), not clipped (slow)\0");
    LinearUI::Text("Buffer contents: %d lines, %d bytes", lines, log.size());
    if (LinearUI::Button("Clear")) { log.clear(); lines = 0; }
    LinearUI::SameLine();
    if (LinearUI::Button("Add 1000 lines"))
    {
        for (int i = 0; i < 1000; i++)
            log.appendf("%i The quick brown fox jumps over the lazy dog\n", lines + i);
        lines += 1000;
    }
    LinearUI::BeginChild("Log");
    switch (test_type)
    {
    case 0:
        // Single call to TextUnformatted() with a big buffer
        LinearUI::TextUnformatted(log.begin(), log.end());
        break;
    case 1:
        {
            // Multiple calls to Text(), manually coarsely clipped - demonstrate how to use the LinearUIListClipper helper.
            LinearUI::PushStyleVar(LinearUIStyleVar_ItemSpacing, LinearVec2(0, 0));
            LinearUIListClipper clipper;
            clipper.Begin(lines);
            while (clipper.Step())
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                    LinearUI::Text("%i The quick brown fox jumps over the lazy dog", i);
            LinearUI::PopStyleVar();
            break;
        }
    case 2:
        // Multiple calls to Text(), not clipped (slow)
        LinearUI::PushStyleVar(LinearUIStyleVar_ItemSpacing, LinearVec2(0, 0));
        for (int i = 0; i < lines; i++)
            LinearUI::Text("%i The quick brown fox jumps over the lazy dog", i);
        LinearUI::PopStyleVar();
        break;
    }
    LinearUI::EndChild();
    LinearUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
//-----------------------------------------------------------------------------

// Demonstrate creating a window which gets auto-resized according to its content.
static void ShowExampleAppAutoResize(bool* p_open)
{
    if (!LinearUI::Begin("Example: Auto-resizing window", p_open, LinearUIWindowFlags_AlwaysAutoResize))
    {
        LinearUI::End();
        return;
    }
    LINEARUI_DEMO_MARKER("Examples/Auto-resizing window");

    static int lines = 10;
    LinearUI::TextUnformatted(
        "Window will resize every-frame to the size of its content.\n"
        "Note that you probably don't want to query the window size to\n"
        "output your content because that would create a feedback loop.");
    LinearUI::SliderInt("Number of lines", &lines, 1, 20);
    for (int i = 0; i < lines; i++)
        LinearUI::Text("%*sThis is line %d", i * 4, "", i); // Pad with space to extend size horizontally
    LinearUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
//-----------------------------------------------------------------------------

// Demonstrate creating a window with custom resize constraints.
// Note that size constraints currently don't work on a docked window (when in 'docking' branch)
static void ShowExampleAppConstrainedResize(bool* p_open)
{
    struct CustomConstraints
    {
        // Helper functions to demonstrate programmatic constraints
        // FIXME: This doesn't take account of decoration size (e.g. title bar), library should make this easier.
        // FIXME: None of the three demos works consistently when resizing from borders.
        static void AspectRatio(LinearUISizeCallbackData* data)
        {
            float aspect_ratio = *(float*)data->UserData;
            data->DesiredSize.y = (float)(int)(data->DesiredSize.x / aspect_ratio);
        }
        static void Square(LinearUISizeCallbackData* data)
        {
            data->DesiredSize.x = data->DesiredSize.y = LINEARUI_MAX(data->DesiredSize.x, data->DesiredSize.y);
        }
        static void Step(LinearUISizeCallbackData* data)
        {
            float step = *(float*)data->UserData;
            data->DesiredSize = LinearVec2((int)(data->DesiredSize.x / step + 0.5f) * step, (int)(data->DesiredSize.y / step + 0.5f) * step);
        }
    };

    const char* test_desc[] =
    {
        "Between 100x100 and 500x500",
        "At least 100x100",
        "Resize vertical + lock current width",
        "Resize horizontal + lock current height",
        "Width Between 400 and 500",
        "Height at least 400",
        "Custom: Aspect Ratio 16:9",
        "Custom: Always Square",
        "Custom: Fixed Steps (100)",
    };

    // Options
    static bool auto_resize = false;
    static bool window_padding = true;
    static int type = 6; // Aspect Ratio
    static int display_lines = 10;

    // Submit constraint
    float aspect_ratio = 16.0f / 9.0f;
    float fixed_step = 100.0f;
    if (type == 0) LinearUI::SetNextWindowSizeConstraints(LinearVec2(100, 100), LinearVec2(500, 500));         // Between 100x100 and 500x500
    if (type == 1) LinearUI::SetNextWindowSizeConstraints(LinearVec2(100, 100), LinearVec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100
    if (type == 2) LinearUI::SetNextWindowSizeConstraints(LinearVec2(-1, 0),    LinearVec2(-1, FLT_MAX));      // Resize vertical + lock current width
    if (type == 3) LinearUI::SetNextWindowSizeConstraints(LinearVec2(0, -1),    LinearVec2(FLT_MAX, -1));      // Resize horizontal + lock current height
    if (type == 4) LinearUI::SetNextWindowSizeConstraints(LinearVec2(400, -1),  LinearVec2(500, -1));          // Width Between and 400 and 500
    if (type == 5) LinearUI::SetNextWindowSizeConstraints(LinearVec2(-1, 400),  LinearVec2(-1, FLT_MAX));      // Height at least 400
    if (type == 6) LinearUI::SetNextWindowSizeConstraints(LinearVec2(0, 0),     LinearVec2(FLT_MAX, FLT_MAX), CustomConstraints::AspectRatio, (void*)&aspect_ratio);   // Aspect ratio
    if (type == 7) LinearUI::SetNextWindowSizeConstraints(LinearVec2(0, 0),     LinearVec2(FLT_MAX, FLT_MAX), CustomConstraints::Square);                              // Always Square
    if (type == 8) LinearUI::SetNextWindowSizeConstraints(LinearVec2(0, 0),     LinearVec2(FLT_MAX, FLT_MAX), CustomConstraints::Step, (void*)&fixed_step);            // Fixed Step

    // Submit window
    if (!window_padding)
        LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0.0f, 0.0f));
    const LinearUIWindowFlags window_flags = auto_resize ? LinearUIWindowFlags_AlwaysAutoResize : 0;
    const bool window_open = LinearUI::Begin("Example: Constrained Resize", p_open, window_flags);
    if (!window_padding)
        LinearUI::PopStyleVar();
    LINEARUI_DEMO_MARKER("Examples/Constrained Resizing window");
    if (window_open)
    {
        if (LinearUI::GetIO().KeyShift)
        {
            // Display a dummy viewport (in your real app you would likely use ImageButton() to display a texture)
            LinearVec2 avail_size = LinearUI::GetContentRegionAvail();
            LinearVec2 pos = LinearUI::GetCursorScreenPos();
            LinearUI::ColorButton("viewport", LinearVec4(0.5f, 0.2f, 0.5f, 1.0f), LinearUIColorEditFlags_NoTooltip | LinearUIColorEditFlags_NoDragDrop, avail_size);
            LinearUI::SetCursorScreenPos(LinearVec2(pos.x + 10, pos.y + 10));
            LinearUI::Text("%.2f x %.2f", avail_size.x, avail_size.y);
        }
        else
        {
            LinearUI::Text("(Hold Shift to display a dummy viewport)");
            if (LinearUI::IsWindowDocked())
                LinearUI::Text("Warning: Sizing Constraints won't work if the window is docked!");
            if (LinearUI::Button("Set 200x200")) { LinearUI::SetWindowSize(LinearVec2(200, 200)); } LinearUI::SameLine();
            if (LinearUI::Button("Set 500x500")) { LinearUI::SetWindowSize(LinearVec2(500, 500)); } LinearUI::SameLine();
            if (LinearUI::Button("Set 800x200")) { LinearUI::SetWindowSize(LinearVec2(800, 200)); }
            LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 20);
            LinearUI::Combo("Constraint", &type, test_desc, LINEARUI_COUNTOF(test_desc));
            LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 20);
            LinearUI::DragInt("Lines", &display_lines, 0.2f, 1, 100);
            LinearUI::Checkbox("Auto-resize", &auto_resize);
            LinearUI::Checkbox("Window padding", &window_padding);
            for (int i = 0; i < display_lines; i++)
                LinearUI::Text("%*sHello, sailor! Making this line long enough for the example.", i * 4, "");
        }
    }
    LinearUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple overlay / ShowExampleAppSimpleOverlay()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple static window with no decoration
// + a context-menu to choose which corner of the screen to use.
static void ShowExampleAppSimpleOverlay(bool* p_open)
{
    static int location = 0;
    LinearUIIO& io = LinearUI::GetIO();
    LinearUIWindowFlags window_flags = LinearUIWindowFlags_NoDecoration | LinearUIWindowFlags_NoDocking | LinearUIWindowFlags_AlwaysAutoResize | LinearUIWindowFlags_NoSavedSettings | LinearUIWindowFlags_NoFocusOnAppearing | LinearUIWindowFlags_NoNav;
    if (location >= 0)
    {
        const float PAD = 10.0f;
        const LinearUIViewport* viewport = LinearUI::GetMainViewport();
        LinearVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        LinearVec2 work_size = viewport->WorkSize;
        LinearVec2 window_pos, window_pos_pivot;
        window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
        LinearUI::SetNextWindowPos(window_pos, LinearUICond_Always, window_pos_pivot);
        LinearUI::SetNextWindowViewport(viewport->ID);
        window_flags |= LinearUIWindowFlags_NoMove;
    }
    else if (location == -2)
    {
        // Center window
        LinearUI::SetNextWindowPos(LinearUI::GetMainViewport()->GetCenter(), LinearUICond_Always, LinearVec2(0.5f, 0.5f));
        window_flags |= LinearUIWindowFlags_NoMove;
    }
    LinearUI::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (LinearUI::Begin("Example: Simple overlay", p_open, window_flags))
    {
        LINEARUI_DEMO_MARKER("Examples/Simple overlay"); // Scroll up to the beginning of this function to see overlay flags
        LinearUI::Text("Simple overlay\n" "(right-click to change position)");
        LinearUI::Separator();
        if (LinearUI::IsMousePosValid())
            LinearUI::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        else
            LinearUI::Text("Mouse Position: <invalid>");
        if (LinearUI::BeginPopupContextWindow())
        {
            if (LinearUI::MenuItem("Custom",       NULL, location == -1)) location = -1;
            if (LinearUI::MenuItem("Center",       NULL, location == -2)) location = -2;
            if (LinearUI::MenuItem("Top-left",     NULL, location == 0)) location = 0;
            if (LinearUI::MenuItem("Top-right",    NULL, location == 1)) location = 1;
            if (LinearUI::MenuItem("Bottom-left",  NULL, location == 2)) location = 2;
            if (LinearUI::MenuItem("Bottom-right", NULL, location == 3)) location = 3;
            if (p_open && LinearUI::MenuItem("Close")) *p_open = false;
            LinearUI::EndPopup();
        }
    }
    LinearUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Fullscreen window / ShowExampleAppFullscreen()
//-----------------------------------------------------------------------------

// Demonstrate creating a window covering the entire screen/viewport
static void ShowExampleAppFullscreen(bool* p_open)
{
    static bool use_work_area = true;
    static LinearUIWindowFlags flags = LinearUIWindowFlags_NoDecoration | LinearUIWindowFlags_NoMove | LinearUIWindowFlags_NoSavedSettings;

    // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
    // Based on your use case you may want one or the other.
    const LinearUIViewport* viewport = LinearUI::GetMainViewport();
    LinearUI::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    LinearUI::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

    if (LinearUI::Begin("Example: Fullscreen window", p_open, flags))
    {
        LINEARUI_DEMO_MARKER("Examples/Fullscreen window");
        LinearUI::Checkbox("Use work area instead of main area", &use_work_area);
        LinearUI::SameLine();
        HelpMarker("Main Area = entire viewport,\nWork Area = entire viewport minus sections used by the main menu bars, task bars etc.\n\nEnable the main-menu bar in Examples menu to see the difference.");

        LinearUI::CheckboxFlags("LinearUIWindowFlags_NoBackground", &flags, LinearUIWindowFlags_NoBackground);
        LinearUI::CheckboxFlags("LinearUIWindowFlags_NoDecoration", &flags, LinearUIWindowFlags_NoDecoration);
        LinearUI::Indent();
        LinearUI::CheckboxFlags("LinearUIWindowFlags_NoTitleBar", &flags, LinearUIWindowFlags_NoTitleBar);
        LinearUI::CheckboxFlags("LinearUIWindowFlags_NoCollapse", &flags, LinearUIWindowFlags_NoCollapse);
        LinearUI::CheckboxFlags("LinearUIWindowFlags_NoScrollbar", &flags, LinearUIWindowFlags_NoScrollbar);
        LinearUI::Unindent();

        if (p_open && LinearUI::Button("Close this window"))
            *p_open = false;
    }
    LinearUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Manipulating Window Titles / ShowExampleAppWindowTitles()
//-----------------------------------------------------------------------------

// Demonstrate the use of "##" and "###" in identifiers to manipulate ID generation.
// This applies to all regular items as well.
// Read FAQ section "How can I have multiple widgets with the same label?" for details.
static void ShowExampleAppWindowTitles(bool*)
{
    const LinearUIViewport* viewport = LinearUI::GetMainViewport();
    const LinearVec2 base_pos = viewport->Pos;

    // By default, Windows are uniquely identified by their title.
    // You can use the "##" and "###" markers to manipulate the display/ID.

    // Using "##" to display same title but have unique identifier.
    LinearUI::SetNextWindowPos(LinearVec2(base_pos.x + 100, base_pos.y + 100), LinearUICond_FirstUseEver);
    LinearUI::Begin("Same title as another window##1");
    LINEARUI_DEMO_MARKER("Examples/Manipulating window titles##1");
    LinearUI::Text("This is window 1.\nMy title is the same as window 2, but my identifier is unique.");
    LinearUI::End();

    LinearUI::SetNextWindowPos(LinearVec2(base_pos.x + 100, base_pos.y + 200), LinearUICond_FirstUseEver);
    LinearUI::Begin("Same title as another window##2");
    LINEARUI_DEMO_MARKER("Examples/Manipulating window titles##2");
    LinearUI::Text("This is window 2.\nMy title is the same as window 1, but my identifier is unique.");
    LinearUI::End();

    // Using "###" to display a changing title but keep a static identifier "AnimatedTitle"
    char buf[128];
    sprintf(buf, "Animated title %c %d###AnimatedTitle", "|/-\\"[(int)(LinearUI::GetTime() / 0.25f) & 3], LinearUI::GetFrameCount());
    LinearUI::SetNextWindowPos(LinearVec2(base_pos.x + 100, base_pos.y + 300), LinearUICond_FirstUseEver);
    LinearUI::Begin(buf);
    LINEARUI_DEMO_MARKER("Examples/Manipulating window titles##3");
    LinearUI::Text("This window has a changing title.");
    LinearUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Custom Rendering using LinearDrawList API / ShowExampleAppCustomRendering()
//-----------------------------------------------------------------------------

// Add a |_| looking shape
static void PathConcaveShape(LinearDrawList* draw_list, float x, float y, float sz)
{
    const LinearVec2 pos_norms[] = { { 0.0f, 0.0f }, { 0.3f, 0.0f }, { 0.3f, 0.7f }, { 0.7f, 0.7f }, { 0.7f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
    for (const LinearVec2& p : pos_norms)
        draw_list->PathLineTo(LinearVec2(x + 0.5f + (int)(sz * p.x), y + 0.5f + (int)(sz * p.y)));
}

// Demonstrate using the low-level LinearDrawList to draw custom shapes.
static void ShowExampleAppCustomRendering(bool* p_open)
{
    if (!LinearUI::Begin("Example: Custom rendering", p_open))
    {
        LinearUI::End();
        return;
    }
    LINEARUI_DEMO_MARKER("Examples/Custom rendering");

    // Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of
    // overloaded operators, etc. Define LINEARUI_VEC2_CLASS_EXTRA in linearuiconfig.h to create implicit conversions between your
    // types and LinearVec2/LinearVec4. Dear LinearUI defines overloaded operators but they are internal to linearui.cpp and not
    // exposed outside (to avoid messing with your types) In this example we are not using the maths operators!

    if (LinearUI::BeginTabBar("##TabBar"))
    {
        if (LinearUI::BeginTabItem("Primitives"))
        {
            LINEARUI_DEMO_MARKER("Examples/Custom rendering/Primitives");
            LinearUI::PushItemWidth(-LinearUI::GetFontSize() * 15);
            LinearDrawList* draw_list = LinearUI::GetWindowDrawList();

            // Draw gradients
            // (note that those are currently exacerbating our sRGB/Linear issues)
            // Calling LinearUI::GetColorU32() multiplies the given colors by the current Style Alpha, but you may pass the LINEARUI_COL32() directly as well..
            LinearUI::Text("Gradients");
            LinearVec2 gradient_size = LinearVec2(LinearUI::CalcItemWidth(), LinearUI::GetFrameHeight());
            {
                LinearVec2 p0 = LinearUI::GetCursorScreenPos();
                LinearVec2 p1 = LinearVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
                LinearU32 col_a = LinearUI::GetColorU32(LINEARUI_COL32(0, 0, 0, 255));
                LinearU32 col_b = LinearUI::GetColorU32(LINEARUI_COL32(255, 255, 255, 255));
                draw_list->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
                LinearUI::InvisibleButton("##gradient1", gradient_size);
            }
            {
                LinearVec2 p0 = LinearUI::GetCursorScreenPos();
                LinearVec2 p1 = LinearVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
                LinearU32 col_a = LinearUI::GetColorU32(LINEARUI_COL32(0, 255, 0, 255));
                LinearU32 col_b = LinearUI::GetColorU32(LINEARUI_COL32(255, 0, 0, 255));
                draw_list->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
                LinearUI::InvisibleButton("##gradient2", gradient_size);
            }

            // Draw a bunch of primitives
            LinearUI::Text("All primitives");
            static float sz = 36.0f;
            static float thickness = 3.0f;
            static int ngon_sides = 6;
            static bool circle_segments_override = false;
            static int circle_segments_override_v = 12;
            static bool curve_segments_override = false;
            static int curve_segments_override_v = 8;
            static LinearVec4 colf = LinearVec4(1.0f, 1.0f, 0.4f, 1.0f);
            LinearUI::DragFloat("Size", &sz, 0.2f, 2.0f, 100.0f, "%.0f");
            LinearUI::DragFloat("Thickness", &thickness, 0.05f, 1.0f, 8.0f, "%.02f");
            LinearUI::SliderInt("N-gon sides", &ngon_sides, 3, 12);
            LinearUI::Checkbox("##circlesegmentoverride", &circle_segments_override);
            LinearUI::SameLine(0.0f, LinearUI::GetStyle().ItemInnerSpacing.x);
            circle_segments_override |= LinearUI::SliderInt("Circle segments override", &circle_segments_override_v, 3, 40);
            LinearUI::Checkbox("##curvessegmentoverride", &curve_segments_override);
            LinearUI::SameLine(0.0f, LinearUI::GetStyle().ItemInnerSpacing.x);
            curve_segments_override |= LinearUI::SliderInt("Curves segments override", &curve_segments_override_v, 3, 40);
            LinearUI::ColorEdit4("Color", &colf.x);

            const LinearVec2 p = LinearUI::GetCursorScreenPos();
            const LinearU32 col = LinearColor(colf);
            const float spacing = 10.0f;
            const LinearDrawFlags corners_tl_br = LinearDrawFlags_RoundCornersTopLeft | LinearDrawFlags_RoundCornersBottomRight;
            const float rounding = sz / 5.0f;
            const int circle_segments = circle_segments_override ? circle_segments_override_v : 0;
            const int curve_segments = curve_segments_override ? curve_segments_override_v : 0;
            const LinearVec2 cp3[3] = { LinearVec2(0.0f, sz * 0.6f), LinearVec2(sz * 0.5f, -sz * 0.4f), LinearVec2(sz, sz) }; // Control points for curves
            const LinearVec2 cp4[4] = { LinearVec2(0.0f, 0.0f), LinearVec2(sz * 1.3f, sz * 0.3f), LinearVec2(sz - sz * 1.3f, sz - sz * 0.3f), LinearVec2(sz, sz) };

            float x = p.x + 4.0f;
            float y = p.y + 4.0f;
            for (int n = 0; n < 2; n++)
            {
                // First line uses a thickness of 1.0f, second line uses the configurable thickness
                float th = (n == 0) ? 1.0f : thickness;
                draw_list->AddNgon(LinearVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, col, ngon_sides, th);                 x += sz + spacing;  // N-gon
                draw_list->AddCircle(LinearVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, col, circle_segments, th);          x += sz + spacing;  // Circle
                draw_list->AddEllipse(LinearVec2(x + sz*0.5f, y + sz*0.5f), LinearVec2(sz*0.5f, sz*0.3f), col, -0.3f, circle_segments, th); x += sz + spacing;  // Ellipse
                draw_list->AddRect(LinearVec2(x, y), LinearVec2(x + sz, y + sz), col, 0.0f, th);                            x += sz + spacing;  // Square
                draw_list->AddRect(LinearVec2(x, y), LinearVec2(x + sz, y + sz), col, rounding, th);                        x += sz + spacing;  // Square with all rounded corners
                draw_list->AddRect(LinearVec2(x, y), LinearVec2(x + sz, y + sz), col, rounding, th, corners_tl_br);         x += sz + spacing;  // Square with two rounded corners
                draw_list->AddTriangle(LinearVec2(x+sz*0.5f,y), LinearVec2(x+sz, y+sz-0.5f), LinearVec2(x, y+sz-0.5f), col, th);x += sz + spacing;  // Triangle
                //draw_list->AddTriangle(LinearVec2(x+sz*0.2f,y), LinearVec2(x, y+sz-0.5f), LinearVec2(x+sz*0.4f, y+sz-0.5f), col, th);x+= sz*0.4f + spacing; // Thin triangle
                PathConcaveShape(draw_list, x, y, sz); draw_list->PathStroke(col, th, LinearDrawFlags_Closed);          x += sz + spacing;  // Concave Shape
                //draw_list->AddPolyline(concave_shape, LINEARUI_COUNTOF(concave_shape), col, LinearDrawFlags_Closed, th);
                draw_list->AddLineH(x, x + sz, y, col, th);                                                         x += sz + spacing;  // Horizontal line (note: drawing a filled rectangle will be faster!)
                draw_list->AddLineV(x, y, y + sz, col, th);                                                         x += spacing;       // Vertical line (note: drawing a filled rectangle will be faster!)
                draw_list->AddLine(LinearVec2(x, y), LinearVec2(x + sz, y + sz), col, th);                                  x += sz + spacing;  // Diagonal line

                // Path
                draw_list->PathArcTo(LinearVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, 3.141592f, 3.141592f * -0.5f);
                draw_list->PathStroke(col, th);
                x += sz + spacing;

                // Quadratic Bezier Curve (3 control points)
                draw_list->AddBezierQuadratic(LinearVec2(x + cp3[0].x, y + cp3[0].y), LinearVec2(x + cp3[1].x, y + cp3[1].y), LinearVec2(x + cp3[2].x, y + cp3[2].y), col, th, curve_segments);
                x += sz + spacing;

                // Cubic Bezier Curve (4 control points)
                draw_list->AddBezierCubic(LinearVec2(x + cp4[0].x, y + cp4[0].y), LinearVec2(x + cp4[1].x, y + cp4[1].y), LinearVec2(x + cp4[2].x, y + cp4[2].y), LinearVec2(x + cp4[3].x, y + cp4[3].y), col, th, curve_segments);

                x = p.x + 4;
                y += sz + spacing;
            }

            // Filled shapes
            draw_list->AddNgonFilled(LinearVec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, col, ngon_sides);             x += sz + spacing;  // N-gon
            draw_list->AddCircleFilled(LinearVec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, col, circle_segments);      x += sz + spacing;  // Circle
            draw_list->AddEllipseFilled(LinearVec2(x + sz * 0.5f, y + sz * 0.5f), LinearVec2(sz * 0.5f, sz * 0.3f), col, -0.3f, circle_segments); x += sz + spacing;// Ellipse
            draw_list->AddRectFilled(LinearVec2(x, y), LinearVec2(x + sz, y + sz), col);                                    x += sz + spacing;  // Square
            draw_list->AddRectFilled(LinearVec2(x, y), LinearVec2(x + sz, y + sz), col, 10.0f);                             x += sz + spacing;  // Square with all rounded corners
            draw_list->AddRectFilled(LinearVec2(x, y), LinearVec2(x + sz, y + sz), col, 10.0f, corners_tl_br);              x += sz + spacing;  // Square with two rounded corners
            draw_list->AddTriangleFilled(LinearVec2(x+sz*0.5f,y), LinearVec2(x+sz, y+sz-0.5f), LinearVec2(x, y+sz-0.5f), col);  x += sz + spacing;  // Triangle
            //draw_list->AddTriangleFilled(LinearVec2(x+sz*0.2f,y), LinearVec2(x, y+sz-0.5f), LinearVec2(x+sz*0.4f, y+sz-0.5f), col); x += sz*0.4f + spacing; // Thin triangle
            PathConcaveShape(draw_list, x, y, sz); draw_list->PathFillConcave(col);                                 x += sz + spacing;  // Concave shape
            draw_list->AddRectFilled(LinearVec2(x, y), LinearVec2(x + sz, y + thickness), col);                             x += sz + spacing;  // Horizontal line (faster than AddLine, but only handle integer thickness)
            draw_list->AddRectFilled(LinearVec2(x, y), LinearVec2(x + thickness, y + sz), col);                             x += spacing * 2.0f;// Vertical line (faster than AddLine, but only handle integer thickness)
            draw_list->AddRectFilled(LinearVec2(x, y), LinearVec2(x + 1, y + 1), col);                                      x += sz;            // Pixel (faster than AddLine)

            // Path
            draw_list->PathArcTo(LinearVec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, 3.141592f * -0.5f, 3.141592f);
            draw_list->PathFillConvex(col);
            x += sz + spacing;

            // Quadratic Bezier Curve (3 control points)
            draw_list->PathLineTo(LinearVec2(x + cp3[0].x, y + cp3[0].y));
            draw_list->PathBezierQuadraticCurveTo(LinearVec2(x + cp3[1].x, y + cp3[1].y), LinearVec2(x + cp3[2].x, y + cp3[2].y), curve_segments);
            draw_list->PathFillConvex(col);
            x += sz + spacing;

            draw_list->AddRectFilledMultiColor(LinearVec2(x, y), LinearVec2(x + sz, y + sz), LINEARUI_COL32(0, 0, 0, 255), LINEARUI_COL32(255, 0, 0, 255), LINEARUI_COL32(255, 255, 0, 255), LINEARUI_COL32(0, 255, 0, 255));
            x += sz + spacing;

            LinearUI::Dummy(LinearVec2((sz + spacing) * 13.2f, (sz + spacing) * 3.0f));
            LinearUI::PopItemWidth();
            LinearUI::EndTabItem();
        }

        if (LinearUI::BeginTabItem("Canvas"))
        {
            LINEARUI_DEMO_MARKER("Examples/Custom rendering/Canvas");
            static LinearVector<LinearVec2> points;
            static LinearVec2 scrolling(0.0f, 0.0f);
            static bool opt_enable_grid = true;
            static bool opt_enable_context_menu = true;
            static bool adding_line = false;

            LinearUI::Checkbox("Enable grid", &opt_enable_grid);
            LinearUI::Checkbox("Enable context menu", &opt_enable_context_menu);
            LinearUI::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");

            // Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region + own scrolling.
            // Here we demonstrate that this can be replaced by simple offsetting + custom drawing + PushClipRect/PopClipRect() calls.
            // To use a child window instead we could use, e.g:
            //      LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0, 0));      // Disable padding
            //      LinearUI::PushStyleColor(LinearUICol_ChildBg, LINEARUI_COL32(50, 50, 50, 255));  // Set a background color
            //      LinearUI::BeginChild("canvas", LinearVec2(0.0f, 0.0f), LinearUIChildFlags_Borders, LinearUIWindowFlags_NoMove);
            //      LinearUI::PopStyleColor();
            //      LinearUI::PopStyleVar();
            //      [...]
            //      LinearUI::EndChild();

            // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
            LinearVec2 canvas_p0 = LinearUI::GetCursorScreenPos();      // LinearDrawList API uses screen coordinates!
            LinearVec2 canvas_sz = LinearUI::GetContentRegionAvail();   // Resize canvas to what's available
            if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
            if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
            LinearVec2 canvas_p1 = LinearVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

            // Draw border and background color
            LinearUIIO& io = LinearUI::GetIO();
            LinearDrawList* draw_list = LinearUI::GetWindowDrawList();
            draw_list->AddRectFilled(canvas_p0, canvas_p1, LINEARUI_COL32(50, 50, 50, 255));
            draw_list->AddRect(canvas_p0, canvas_p1, LINEARUI_COL32(255, 255, 255, 255));

            // This will catch our interactions
            LinearUI::InvisibleButton("canvas", canvas_sz, LinearUIButtonFlags_MouseButtonLeft | LinearUIButtonFlags_MouseButtonRight);
            const bool is_hovered = LinearUI::IsItemHovered(); // Hovered
            const bool is_active = LinearUI::IsItemActive();   // Held
            const LinearVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
            const LinearVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

            // Add first and second point
            if (is_hovered && !adding_line && LinearUI::IsMouseClicked(LinearUIMouseButton_Left))
            {
                points.push_back(mouse_pos_in_canvas);
                points.push_back(mouse_pos_in_canvas);
                adding_line = true;
            }
            if (adding_line)
            {
                points.back() = mouse_pos_in_canvas;
                if (!LinearUI::IsMouseDown(LinearUIMouseButton_Left))
                    adding_line = false;
            }

            // Pan (we use a zero mouse threshold when there's no context menu)
            // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
            const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
            if (is_active && LinearUI::IsMouseDragging(LinearUIMouseButton_Right, mouse_threshold_for_pan))
            {
                scrolling.x += io.MouseDelta.x;
                scrolling.y += io.MouseDelta.y;
            }

            // Context menu (under default mouse threshold)
            LinearVec2 drag_delta = LinearUI::GetMouseDragDelta(LinearUIMouseButton_Right);
            if (opt_enable_context_menu && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
                LinearUI::OpenPopupOnItemClick("context", LinearUIPopupFlags_MouseButtonRight);
            if (LinearUI::BeginPopup("context"))
            {
                if (adding_line)
                    points.resize(points.size() - 2);
                adding_line = false;
                if (LinearUI::MenuItem("Remove one", NULL, false, points.Size > 0)) { points.resize(points.size() - 2); }
                if (LinearUI::MenuItem("Remove all", NULL, false, points.Size > 0)) { points.clear(); }
                LinearUI::EndPopup();
            }

            // Draw grid + all lines in the canvas
            draw_list->PushClipRect(canvas_p0, canvas_p1, true);
            if (opt_enable_grid)
            {
                const float GRID_STEP = 64.0f;
                for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
                    draw_list->AddLineV(canvas_p0.x + x, canvas_p0.y, canvas_p1.y, LINEARUI_COL32(200, 200, 200, 40));
                for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
                    draw_list->AddLineH(canvas_p0.x, canvas_p1.x, canvas_p0.y + y, LINEARUI_COL32(200, 200, 200, 40));
            }
            for (int n = 0; n < points.Size; n += 2)
                draw_list->AddLine(LinearVec2(origin.x + points[n].x, origin.y + points[n].y), LinearVec2(origin.x + points[n + 1].x, origin.y + points[n + 1].y), LINEARUI_COL32(255, 255, 0, 255), 2.0f);
            draw_list->PopClipRect();

            LinearUI::EndTabItem();
        }

        if (LinearUI::BeginTabItem("BG/FG draw lists"))
        {
            LINEARUI_DEMO_MARKER("Examples/Custom rendering/BG & FG draw lists");
            static bool draw_bg = true;
            static bool draw_fg = true;
            LinearUI::Checkbox("Draw in Background draw list", &draw_bg);
            LinearUI::SameLine(); HelpMarker("The Background draw list will be rendered below every Dear LinearUI windows.");
            LinearUI::Checkbox("Draw in Foreground draw list", &draw_fg);
            LinearUI::SameLine(); HelpMarker("The Foreground draw list will be rendered over every Dear LinearUI windows.");
            LinearVec2 window_pos = LinearUI::GetWindowPos();
            LinearVec2 window_size = LinearUI::GetWindowSize();
            LinearVec2 window_center = LinearVec2(window_pos.x + window_size.x * 0.5f, window_pos.y + window_size.y * 0.5f);
            if (draw_bg)
                LinearUI::GetBackgroundDrawList()->AddCircle(window_center, window_size.x * 0.6f, LINEARUI_COL32(255, 0, 0, 200), 0, 10 + 4);
            if (draw_fg)
                LinearUI::GetForegroundDrawList()->AddCircle(window_center, window_size.y * 0.6f, LINEARUI_COL32(0, 255, 0, 200), 0, 10);
            LinearUI::EndTabItem();
        }

        // Demonstrate out-of-order rendering via channels splitting
        // We use functions in LinearDrawList as each draw list contains a convenience splitter,
        // but you can also instantiate your own LinearDrawListSplitter if you need to nest them.
        if (LinearUI::BeginTabItem("Draw Channels"))
        {
            LINEARUI_DEMO_MARKER("Examples/Custom rendering/Draw Channels");
            LinearDrawList* draw_list = LinearUI::GetWindowDrawList();
            {
                LinearUI::Text("Blue shape is drawn first: appears in back");
                LinearUI::Text("Red shape is drawn after: appears in front");
                LinearVec2 p0 = LinearUI::GetCursorScreenPos();
                draw_list->AddRectFilled(LinearVec2(p0.x, p0.y), LinearVec2(p0.x + 50, p0.y + 50), LINEARUI_COL32(0, 0, 255, 255)); // Blue
                draw_list->AddRectFilled(LinearVec2(p0.x + 25, p0.y + 25), LinearVec2(p0.x + 75, p0.y + 75), LINEARUI_COL32(255, 0, 0, 255)); // Red
                LinearUI::Dummy(LinearVec2(75, 75));
            }
            LinearUI::Separator();
            {
                LinearUI::Text("Blue shape is drawn first, into channel 1: appears in front");
                LinearUI::Text("Red shape is drawn after, into channel 0: appears in back");
                LinearVec2 p1 = LinearUI::GetCursorScreenPos();

                // Create 2 channels and draw a Blue shape THEN a Red shape.
                // You can create any number of channels. Tables API use 1 channel per column in order to better batch draw calls.
                draw_list->ChannelsSplit(2);
                draw_list->ChannelsSetCurrent(1);
                draw_list->AddRectFilled(LinearVec2(p1.x, p1.y), LinearVec2(p1.x + 50, p1.y + 50), LINEARUI_COL32(0, 0, 255, 255)); // Blue
                draw_list->ChannelsSetCurrent(0);
                draw_list->AddRectFilled(LinearVec2(p1.x + 25, p1.y + 25), LinearVec2(p1.x + 75, p1.y + 75), LINEARUI_COL32(255, 0, 0, 255)); // Red

                // Flatten/reorder channels. Red shape is in channel 0 and it appears below the Blue shape in channel 1.
                // This works by copying draw indices only (vertices are not copied).
                draw_list->ChannelsMerge();
                LinearUI::Dummy(LinearVec2(75, 75));
                LinearUI::Text("After reordering, contents of channel 0 appears below channel 1.");
            }
            LinearUI::EndTabItem();
        }

        LinearUI::EndTabBar();
    }

    LinearUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Docking, DockSpace / ShowExampleAppDockSpace()
//-----------------------------------------------------------------------------

struct LinearUIDemoDockspaceArgs
{
    bool                IsFullscreen = true;
    bool                KeepWindowPadding = false; // Keep WindowPadding to help understand that DockSpace() is a widget inside the window.
    LinearUIDockNodeFlags  DockSpaceFlags  = LinearUIDockNodeFlags_None;
};

// THIS IS A DEMO FOR ADVANCED USAGE OF DockSpace().
// MOST REGULAR APPLICATIONS WANTING TO ALLOW DOCKING WINDOWS ON THE EDGE OF YOUR SCREEN CAN SIMPLY USE:
//    LinearUI::NewFrame(); + LinearUI::DockSpaceOverViewport();                                                   // Create a dockspace in main viewport
// OR:
//    LinearUI::NewFrame(); + LinearUI::DockSpaceOverViewport(0, nullptr, LinearUIDockNodeFlags_PassthruCentralNode); // Create a dockspace in main viewport, where central node is transparent.
// Demonstrate using DockSpace() to create an explicit docking node within an existing window, with various options.
// Read https://github.com/ocornut/linearui/wiki/Docking for details.
// The reasons we do not use DockSpaceOverViewport() in this demo is because:
// - (1) we allow the host window to be floating/moveable instead of filling the viewport (when args->IsFullscreen == false)
//       which is mostly to showcase the idea that DockSpace() may be submitted anywhere.
//       Also see 'Demo->Examples->Documents' for a less abstract version of this.
// - (2) we allow the host window to have padding (when args->UsePadding == true)
// - (3) we expose variety of other flags.
static void ShowExampleAppDockSpaceAdvanced(LinearUIDemoDockspaceArgs* args, bool* p_open)
{
    LinearUIDockNodeFlags dockspace_flags = args->DockSpaceFlags;

    // We are using the LinearUIWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    LinearUIWindowFlags window_flags = LinearUIWindowFlags_NoDocking;
    if (args->IsFullscreen)
    {
        // Fullscreen dockspace: practically the same as calling DockSpaceOverViewport();
        const LinearUIViewport* viewport = LinearUI::GetMainViewport();
        LinearUI::SetNextWindowPos(viewport->WorkPos);
        LinearUI::SetNextWindowSize(viewport->WorkSize);
        LinearUI::SetNextWindowViewport(viewport->ID);
        LinearUI::PushStyleVar(LinearUIStyleVar_WindowRounding, 0.0f);
        LinearUI::PushStyleVar(LinearUIStyleVar_WindowBorderSize, 0.0f);
        window_flags |= LinearUIWindowFlags_NoTitleBar | LinearUIWindowFlags_NoCollapse | LinearUIWindowFlags_NoResize | LinearUIWindowFlags_NoMove;
        window_flags |= LinearUIWindowFlags_NoBringToFrontOnFocus | LinearUIWindowFlags_NoNavFocus;
        window_flags |= LinearUIWindowFlags_NoBackground;
    }
    else
    {
        // Floating dockspace
        dockspace_flags &= ~LinearUIDockNodeFlags_PassthruCentralNode;
    }

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!args->KeepWindowPadding)
        LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0.0f, 0.0f));
    LinearUI::Begin("Window with a DockSpace", p_open, window_flags);
    if (!args->KeepWindowPadding)
        LinearUI::PopStyleVar();

    if (args->IsFullscreen)
        LinearUI::PopStyleVar(2);

    // Submit the DockSpace widget inside our window
    // - Note that the id here is different from the one used by DockSpaceOverViewport(), so docking state won't get transfered between "Basic" and "Advanced" demos.
    // - If we made the ShowExampleAppDockSpaceBasic() calculate its own ID and pass it to DockSpaceOverViewport() the ID could easily match.
    LinearUIID dockspace_id = LinearUI::GetID("MyDockSpace");
    LinearUI::DockSpace(dockspace_id, LinearVec2(0.0f, 0.0f), dockspace_flags);

    LinearUI::End();
}

static void ShowExampleAppDockSpaceBasic(LinearUIDockNodeFlags flags)
{
    // Basic version which you can use in many apps:
    // e.g:
    //   LinearUI::DockSpaceOverViewport();
    // or:
    //   LinearUI::DockSpaceOverViewport(0, nullptr, LinearUIDockNodeFlags_PassthruCentralNode); // Central node will be transparent
    // or:
    //   LinearUIViewport* viewport = LinearUI::GetMainViewport();
    //   LinearUI::DockSpaceOverViewport(0, viewport, LinearUIDockNodeFlags_None);

    LinearUI::DockSpaceOverViewport(0, nullptr, flags);
}

void ShowExampleAppDockSpace(bool* p_open)
{
    static int opt_demo_mode = 0;
    static bool opt_demo_mode_changed = false;
    static LinearUIDemoDockspaceArgs args;

    if (opt_demo_mode == 0)
        ShowExampleAppDockSpaceBasic(args.DockSpaceFlags);
    else
        ShowExampleAppDockSpaceAdvanced(&args, p_open);

    // Refocus our window to minimize perceived loss of focus when changing mode (caused by the fact that each use a different window, which would not happen in a real app)
    if (opt_demo_mode_changed)
        LinearUI::SetNextWindowFocus();
    LinearUI::Begin("Examples: Dockspace", p_open, LinearUIWindowFlags_MenuBar);
    opt_demo_mode_changed = false;
    opt_demo_mode_changed |= LinearUI::RadioButton("Basic demo mode", &opt_demo_mode, 0);
    opt_demo_mode_changed |= LinearUI::RadioButton("Advanced demo mode", &opt_demo_mode, 1);

    LinearUI::SeparatorText("Options");

    if ((LinearUI::GetIO().ConfigFlags & LinearUIConfigFlags_DockingEnable) == 0)
    {
        ShowDockingDisabledMessage();
    }
    else if (opt_demo_mode == 0)
    {
        args.DockSpaceFlags &= LinearUIDockNodeFlags_PassthruCentralNode; // Allowed flags
        LinearUI::CheckboxFlags("Flag: PassthruCentralNode", &args.DockSpaceFlags, LinearUIDockNodeFlags_PassthruCentralNode);
    }
    else if (opt_demo_mode == 1)
    {
        LinearUI::Checkbox("Fullscreen", &args.IsFullscreen);
        LinearUI::Checkbox("Keep Window Padding", &args.KeepWindowPadding);
        LinearUI::SameLine();
        HelpMarker("This is mostly exposed to facilitate understanding that a DockSpace() is _inside_ a window.");
        LinearUI::BeginDisabled(args.IsFullscreen == false);
        LinearUI::CheckboxFlags("Flag: PassthruCentralNode",      &args.DockSpaceFlags, LinearUIDockNodeFlags_PassthruCentralNode);
        LinearUI::EndDisabled();
        LinearUI::CheckboxFlags("Flag: NoDockingOverCentralNode", &args.DockSpaceFlags, LinearUIDockNodeFlags_NoDockingOverCentralNode);
        LinearUI::CheckboxFlags("Flag: NoDockingSplit",           &args.DockSpaceFlags, LinearUIDockNodeFlags_NoDockingSplit);
        LinearUI::CheckboxFlags("Flag: NoUndocking",              &args.DockSpaceFlags, LinearUIDockNodeFlags_NoUndocking);
        LinearUI::CheckboxFlags("Flag: NoResize",                 &args.DockSpaceFlags, LinearUIDockNodeFlags_NoResize);
        LinearUI::CheckboxFlags("Flag: AutoHideTabBar",           &args.DockSpaceFlags, LinearUIDockNodeFlags_AutoHideTabBar);
    }

    // Show demo options and help
    if (LinearUI::BeginMenuBar())
    {
        if (LinearUI::BeginMenu("Help"))
        {
            LinearUI::TextUnformatted(
                "This demonstrates the use of LinearUI::DockSpace() which allows you to manually\ncreate a docking node _within_ another window." "\n"
                "The \"Basic\" version uses the LinearUI::DockSpaceOverViewport() helper. Most applications can probably use this.");
            LinearUI::Separator();
            LinearUI::TextUnformatted("When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
                "- Drag from window title bar or their tab to dock/undock." "\n"
                "- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
                "- Hold SHIFT to disable docking (if io.ConfigDockingWithShift == false, default)" "\n"
                "- Hold SHIFT to enable docking (if io.ConfigDockingWithShift == true)");
            LinearUI::Separator();
            LinearUI::TextUnformatted("More details:"); LinearUI::Bullet(); LinearUI::SameLine(); LinearUI::TextLinkOpenURL("Docking Wiki page", "https://github.com/ocornut/linearui/wiki/Docking");
            LinearUI::BulletText("Read comments in ShowExampleAppDockSpace()");
            LinearUI::EndMenu();
        }
        LinearUI::EndMenuBar();
    }

    LinearUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()
//-----------------------------------------------------------------------------

// Simplified structure to mimic a Document model
struct MyDocument
{
    char        Name[32];   // Document title
    int         UID;        // Unique ID (necessary as we can change title)
    bool        Open;       // Set when open (we keep an array of all available documents to simplify demo code!)
    bool        OpenPrev;   // Copy of Open from last update.
    bool        Dirty;      // Set when the document has been modified
    LinearVec4      Color;      // An arbitrary variable associated to the document

    MyDocument(int uid, const char* name, bool open = true, const LinearVec4& color = LinearVec4(1.0f, 1.0f, 1.0f, 1.0f))
    {
        UID = uid;
        snprintf(Name, sizeof(Name), "%s", name);
        Open = OpenPrev = open;
        Dirty = false;
        Color = color;
    }
    void DoOpen()       { Open = true; }
    void DoForceClose() { Open = false; Dirty = false; }
    void DoSave()       { Dirty = false; }
};

struct ExampleAppDocuments
{
    LinearVector<MyDocument>    Documents;
    LinearVector<MyDocument*>   CloseQueue;
    MyDocument*             RenamingDoc = NULL;
    bool                    RenamingStarted = false;

    ExampleAppDocuments()
    {
        Documents.push_back(MyDocument(0, "Lettuce",             true,  LinearVec4(0.4f, 0.8f, 0.4f, 1.0f)));
        Documents.push_back(MyDocument(1, "Eggplant",            true,  LinearVec4(0.8f, 0.5f, 1.0f, 1.0f)));
        Documents.push_back(MyDocument(2, "Carrot",              true,  LinearVec4(1.0f, 0.8f, 0.5f, 1.0f)));
        Documents.push_back(MyDocument(3, "Tomato",              false, LinearVec4(1.0f, 0.3f, 0.4f, 1.0f)));
        Documents.push_back(MyDocument(4, "A Rather Long Title", false, LinearVec4(0.4f, 0.8f, 0.8f, 1.0f)));
        Documents.push_back(MyDocument(5, "Some Document",       false, LinearVec4(0.8f, 0.8f, 1.0f, 1.0f)));
    }

    // As we allow to change document name, we append a never-changing document ID so tabs are stable
    void GetTabName(MyDocument* doc, char* out_buf, size_t out_buf_size)
    {
        snprintf(out_buf, out_buf_size, "%s###doc%d", doc->Name, doc->UID);
    }

    // Display placeholder contents for the Document
    void DisplayDocContents(MyDocument* doc)
    {
        LinearUI::PushID(doc);
        LinearUI::Text("Document \"%s\"", doc->Name);
        LinearUI::PushStyleColor(LinearUICol_Text, doc->Color);
        LinearUI::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");
        LinearUI::PopStyleColor();

        LinearUI::SetNextItemShortcut(LinearUIMod_Ctrl | LinearUIKey_R, LinearUIInputFlags_Tooltip);
        if (LinearUI::Button("Rename.."))
        {
            RenamingDoc = doc;
            RenamingStarted = true;
        }
        LinearUI::SameLine();

        LinearUI::SetNextItemShortcut(LinearUIMod_Ctrl | LinearUIKey_M, LinearUIInputFlags_Tooltip);
        if (LinearUI::Button("Modify"))
            doc->Dirty = true;

        LinearUI::SameLine();
        LinearUI::SetNextItemShortcut(LinearUIMod_Ctrl | LinearUIKey_S, LinearUIInputFlags_Tooltip);
        if (LinearUI::Button("Save"))
            doc->DoSave();

        LinearUI::SameLine();
        LinearUI::SetNextItemShortcut(LinearUIMod_Ctrl | LinearUIKey_W, LinearUIInputFlags_Tooltip);
        if (LinearUI::Button("Close"))
            CloseQueue.push_back(doc);
        LinearUI::ColorEdit3("color", &doc->Color.x);  // Useful to test drag and drop and hold-dragged-to-open-tab behavior.
        LinearUI::PopID();
    }

    // Display context menu for the Document
    void DisplayDocContextMenu(MyDocument* doc)
    {
        if (!LinearUI::BeginPopupContextItem())
            return;

        char buf[256];
        sprintf(buf, "Save %s", doc->Name);
        if (LinearUI::MenuItem(buf, "Ctrl+S", false, doc->Open))
            doc->DoSave();
        if (LinearUI::MenuItem("Rename...", "Ctrl+R", false, doc->Open))
            RenamingDoc = doc;
        if (LinearUI::MenuItem("Close", "Ctrl+W", false, doc->Open))
            CloseQueue.push_back(doc);
        LinearUI::EndPopup();
    }

    // [Optional] Notify the system of Tabs/Windows closure that happened outside the regular tab interface.
    // If a tab has been closed programmatically (aka closed from another source such as the Checkbox() in the demo,
    // as opposed to clicking on the regular tab closing button) and stops being submitted, it will take a frame for
    // the tab bar to notice its absence. During this frame there will be a gap in the tab bar, and if the tab that has
    // disappeared was the selected one, the tab bar will report no selected tab during the frame. This will effectively
    // give the impression of a flicker for one frame.
    // We call SetTabItemClosed() to manually notify the Tab Bar or Docking system of removed tabs to avoid this glitch.
    // Note that this completely optional, and only affect tab bars with the LinearUITabBarFlags_Reorderable flag.
    void NotifyOfDocumentsClosedElsewhere()
    {
        for (MyDocument& doc : Documents)
        {
            if (!doc.Open && doc.OpenPrev)
                LinearUI::SetTabItemClosed(doc.Name);
            doc.OpenPrev = doc.Open;
        }
    }
};

void ShowExampleAppDocuments(bool* p_open)
{
    static ExampleAppDocuments app;

    // Options
    enum Target
    {
        Target_None,
        Target_Tab,                 // Create documents as local tab into a local tab bar
        Target_DockSpaceAndWindow   // Create documents as regular windows, and create an embedded dockspace
    };
    static Target opt_target = Target_Tab;
    static bool opt_reorderable = true;
    static LinearUITabBarFlags opt_fitting_flags = LinearUITabBarFlags_FittingPolicyDefault_;

    // When (opt_target == Target_DockSpaceAndWindow) there is the possibily that one of our child Document window (e.g. "Eggplant")
    // that we emit gets docked into the same spot as the parent window ("Example: Documents").
    // This would create a problematic feedback loop because selecting the "Eggplant" tab would make the "Example: Documents" tab
    // not visible, which in turn would stop submitting the "Eggplant" window.
    // We avoid this problem by submitting our documents window even if our parent window is not currently visible.
    // Another solution may be to make the "Example: Documents" window use the LinearUIWindowFlags_NoDocking.

    bool window_contents_visible = LinearUI::Begin("Example: Documents", p_open, LinearUIWindowFlags_MenuBar);
    if (!window_contents_visible && opt_target != Target_DockSpaceAndWindow)
    {
        LinearUI::End();
        return;
    }
    LINEARUI_DEMO_MARKER("Examples/Documents");

    // Menu
    if (LinearUI::BeginMenuBar())
    {
        if (LinearUI::BeginMenu("File"))
        {
            int open_count = 0;
            for (MyDocument& doc : app.Documents)
                open_count += doc.Open ? 1 : 0;

            if (LinearUI::BeginMenu("Open", open_count < app.Documents.Size))
            {
                for (MyDocument& doc : app.Documents)
                    if (!doc.Open && LinearUI::MenuItem(doc.Name))
                        doc.DoOpen();
                LinearUI::EndMenu();
            }
            if (LinearUI::MenuItem("Close All Documents", NULL, false, open_count > 0))
                for (MyDocument& doc : app.Documents)
                    app.CloseQueue.push_back(&doc);
            if (LinearUI::MenuItem("Exit") && p_open)
                *p_open = false;
            LinearUI::EndMenu();
        }
        LinearUI::EndMenuBar();
    }

    // [Debug] List documents with one checkbox for each
    for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
    {
        MyDocument& doc = app.Documents[doc_n];
        if (doc_n > 0)
            LinearUI::SameLine();
        LinearUI::PushID(&doc);
        if (LinearUI::Checkbox(doc.Name, &doc.Open))
            if (!doc.Open)
                doc.DoForceClose();
        LinearUI::PopID();
    }
    LinearUI::PushItemWidth(LinearUI::GetFontSize() * 12);
    LinearUI::Combo("Output", (int*)&opt_target, "None\0TabBar+Tabs\0DockSpace+Window\0");
    LinearUI::PopItemWidth();
    bool redock_all = false;
    if (opt_target == Target_Tab)                { LinearUI::SameLine(); LinearUI::Checkbox("Reorderable Tabs", &opt_reorderable); }
    if (opt_target == Target_DockSpaceAndWindow) { LinearUI::SameLine(); redock_all = LinearUI::Button("Redock all"); }

    LinearUI::Separator();

    // About the LinearUIWindowFlags_UnsavedDocument / LinearUITabItemFlags_UnsavedDocument flags.
    // They have multiple effects:
    // - Display a dot next to the title.
    // - Tab is selected when clicking the X close button.
    // - Closure is not assumed (will wait for user to stop submitting the tab).
    //   Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
    //   We need to assume closure by default otherwise waiting for "lack of submission" on the next frame would leave an empty
    //   hole for one-frame, both in the tab-bar and in tab-contents when closing a tab/window.
    //   The rarely used SetTabItemClosed() function is a way to notify of programmatic closure to avoid the one-frame hole.

    // Tabs
    if (opt_target == Target_Tab)
    {
        LinearUITabBarFlags tab_bar_flags = (opt_fitting_flags) | (opt_reorderable ? LinearUITabBarFlags_Reorderable : 0);
        tab_bar_flags |= LinearUITabBarFlags_DrawSelectedOverline;
        if (LinearUI::BeginTabBar("##tabs", tab_bar_flags))
        {
            if (opt_reorderable)
                app.NotifyOfDocumentsClosedElsewhere();

            // [DEBUG] Stress tests
            //if ((LinearUI::GetFrameCount() % 30) == 0) docs[1].Open ^= 1;            // [DEBUG] Automatically show/hide a tab. Test various interactions e.g. dragging with this on.
            //if (LinearUI::GetIO().KeyCtrl) LinearUI::SetTabItemSelected(docs[1].Name);  // [DEBUG] Test SetTabItemSelected(), probably not very useful as-is anyway..

            // Submit Tabs
            for (MyDocument& doc : app.Documents)
            {
                if (!doc.Open)
                    continue;

                // As we allow to change document name, we append a never-changing document id so tabs are stable
                char doc_name_buf[64];
                app.GetTabName(&doc, doc_name_buf, sizeof(doc_name_buf));
                LinearUITabItemFlags tab_flags = (doc.Dirty ? LinearUITabItemFlags_UnsavedDocument : 0);
                bool visible = LinearUI::BeginTabItem(doc_name_buf, &doc.Open, tab_flags);

                // Cancel attempt to close when unsaved add to save queue so we can display a popup.
                if (!doc.Open && doc.Dirty)
                {
                    doc.Open = true;
                    app.CloseQueue.push_back(&doc);
                }

                app.DisplayDocContextMenu(&doc);
                if (visible)
                {
                    app.DisplayDocContents(&doc);
                    LinearUI::EndTabItem();
                }
            }

            LinearUI::EndTabBar();
        }
    }
    else if (opt_target == Target_DockSpaceAndWindow)
    {
        if (LinearUI::GetIO().ConfigFlags & LinearUIConfigFlags_DockingEnable)
        {
            app.NotifyOfDocumentsClosedElsewhere();

            // Create a DockSpace node where any window can be docked
            LinearUIID dockspace_id = LinearUI::GetID("MyDockSpace");
            LinearUI::DockSpace(dockspace_id);

            // Create Windows
            for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
            {
                MyDocument* doc = &app.Documents[doc_n];
                if (!doc->Open)
                    continue;

                LinearUI::SetNextWindowDockID(dockspace_id, redock_all ? LinearUICond_Always : LinearUICond_FirstUseEver);
                LinearUIWindowFlags window_flags = (doc->Dirty ? LinearUIWindowFlags_UnsavedDocument : 0);
                bool visible = LinearUI::Begin(doc->Name, &doc->Open, window_flags);

                // Cancel attempt to close when unsaved add to save queue so we can display a popup.
                if (!doc->Open && doc->Dirty)
                {
                    doc->Open = true;
                    app.CloseQueue.push_back(doc);
                }

                app.DisplayDocContextMenu(doc);
                if (visible)
                    app.DisplayDocContents(doc);

                LinearUI::End();
            }
        }
        else
        {
            ShowDockingDisabledMessage();
        }
    }

    // Early out other contents
    if (!window_contents_visible)
    {
        LinearUI::End();
        return;
    }

    // Display renaming UI
    if (app.RenamingDoc != NULL)
    {
        if (app.RenamingStarted)
            LinearUI::OpenPopup("Rename");
        if (LinearUI::BeginPopup("Rename"))
        {
            LinearUI::SetNextItemWidth(LinearUI::GetFontSize() * 30);
            if (LinearUI::InputText("###Name", app.RenamingDoc->Name, LINEARUI_COUNTOF(app.RenamingDoc->Name), LinearUIInputTextFlags_EnterReturnsTrue))
            {
                LinearUI::CloseCurrentPopup();
                app.RenamingDoc = NULL;
            }
            if (app.RenamingStarted)
                LinearUI::SetKeyboardFocusHere(-1);
            LinearUI::EndPopup();
        }
        else
        {
            app.RenamingDoc = NULL;
        }
        app.RenamingStarted = false;
    }

    // Display closing confirmation UI
    if (!app.CloseQueue.empty())
    {
        int close_queue_unsaved_documents = 0;
        for (int n = 0; n < app.CloseQueue.Size; n++)
            if (app.CloseQueue[n]->Dirty)
                close_queue_unsaved_documents++;

        if (close_queue_unsaved_documents == 0)
        {
            // Close documents when all are unsaved
            for (int n = 0; n < app.CloseQueue.Size; n++)
                app.CloseQueue[n]->DoForceClose();
            app.CloseQueue.clear();
        }
        else
        {
            if (!LinearUI::IsPopupOpen("Save?"))
                LinearUI::OpenPopup("Save?");
            if (LinearUI::BeginPopupModal("Save?", NULL, LinearUIWindowFlags_AlwaysAutoResize))
            {
                LinearUI::Text("Save change to the following items?");
                float item_height = LinearUI::GetTextLineHeightWithSpacing();
                if (LinearUI::BeginChild(LinearUI::GetID("frame"), LinearVec2(-FLT_MIN, 6.25f * item_height), LinearUIChildFlags_FrameStyle))
                    for (MyDocument* doc : app.CloseQueue)
                        if (doc->Dirty)
                            LinearUI::Text("%s", doc->Name);
                LinearUI::EndChild();

                LinearVec2 button_size(LinearUI::GetFontSize() * 7.0f, 0.0f);
                if (LinearUI::Button("Yes", button_size))
                {
                    for (MyDocument* doc : app.CloseQueue)
                    {
                        if (doc->Dirty)
                            doc->DoSave();
                        doc->DoForceClose();
                    }
                    app.CloseQueue.clear();
                    LinearUI::CloseCurrentPopup();
                }
                LinearUI::SameLine();
                if (LinearUI::Button("No", button_size))
                {
                    for (MyDocument* doc : app.CloseQueue)
                        doc->DoForceClose();
                    app.CloseQueue.clear();
                    LinearUI::CloseCurrentPopup();
                }
                LinearUI::SameLine();
                if (LinearUI::Button("Cancel", button_size))
                {
                    app.CloseQueue.clear();
                    LinearUI::CloseCurrentPopup();
                }
                LinearUI::EndPopup();
            }
        }
    }

    LinearUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Assets Browser / ShowExampleAppAssetsBrowser()
//-----------------------------------------------------------------------------

//#include "linearui_internal.h" // NavMoveRequestTryWrapping()

struct ExampleAsset
{
    LinearUIID ID;
    int     Type;

    ExampleAsset(LinearUIID id, int type) { ID = id; Type = type; }

    static const LinearUITableSortSpecs* s_current_sort_specs;

    static void SortWithSortSpecs(LinearUITableSortSpecs* sort_specs, ExampleAsset* items, int items_count)
    {
        s_current_sort_specs = sort_specs; // Store in variable accessible by the sort function.
        if (items_count > 1)
            qsort(items, (size_t)items_count, sizeof(items[0]), ExampleAsset::CompareWithSortSpecs);
        s_current_sort_specs = NULL;
    }

    // Compare function to be used by qsort()
    static int LINEARUI_CDECL CompareWithSortSpecs(const void* lhs, const void* rhs)
    {
        const ExampleAsset* a = (const ExampleAsset*)lhs;
        const ExampleAsset* b = (const ExampleAsset*)rhs;
        for (int n = 0; n < s_current_sort_specs->SpecsCount; n++)
        {
            const LinearUITableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
            int delta = 0;
            if (sort_spec->ColumnIndex == 0)
                delta = ((int)a->ID - (int)b->ID);
            else if (sort_spec->ColumnIndex == 1)
                delta = (a->Type - b->Type);
            if (delta > 0)
                return (sort_spec->SortDirection == LinearUISortDirection_Ascending) ? +1 : -1;
            if (delta < 0)
                return (sort_spec->SortDirection == LinearUISortDirection_Ascending) ? -1 : +1;
        }
        return (int)a->ID - (int)b->ID;
    }
};
const LinearUITableSortSpecs* ExampleAsset::s_current_sort_specs = NULL;

struct ExampleAssetsBrowser
{
    // Options
    bool            ShowTypeOverlay = true;
    bool            AllowSorting = true;
    bool            AllowBoxSelect = true;                  // Will set LinearUIMultiSelectFlags_BoxSelect2d
    bool            AllowBoxSelectInsideSelection = false;  // Will set LinearUIMultiSelectFlags_SelectOnClickAlways
    bool            AllowDragUnselected = false;            // Will set LinearUIMultiSelectFlags_SelectOnClickRelease
    float           IconSize = 0;
    int             IconSpacing = 10;
    int             IconHitSpacing = 4;                     // Increase hit-spacing if you want to make it possible to clear or box-select from gaps. Some spacing is required to able to amend with Shift+box-select. Value is small in Explorer.
    bool            StretchSpacing = true;
    bool            UseScrollX = false;                     // Debug: submit twice the number of items per line (overflow horizontally to exercise ScrollX + box-select)

    // State
    LinearVector<ExampleAsset> Items;               // Our items
    ExampleSelectionWithDeletion Selection;     // Our selection (LinearUISelectionBasicStorage + helper funcs to handle deletion)
    LinearUIID         NextItemId = 0;             // Unique identifier when creating new items
    bool            RequestDelete = false;      // Deferred deletion request
    bool            RequestSort = false;        // Deferred sort request
    float           ZoomWheelAccum = 0.0f;      // Mouse wheel accumulator to handle smooth wheels better

    // Calculated sizes for layout, output of UpdateLayoutSizes(). Could be locals but our code is simpler this way.
    LinearVec2          LayoutItemSize;
    LinearVec2          LayoutItemStep;             // == LayoutItemSize + LayoutItemSpacing
    float           LayoutItemSpacing = 0.0f;
    float           LayoutSelectableSpacing = 0.0f;
    float           LayoutOuterPadding = 0.0f;
    int             LayoutColumnCount = 0;
    int             LayoutLineCount = 0;

    // Functions
    ExampleAssetsBrowser()
    {
        AddItems(10000);
    }
    void AddItems(int count)
    {
        if (Items.Size == 0)
            NextItemId = 0;
        Items.reserve(Items.Size + count);
        for (int n = 0; n < count; n++, NextItemId++)
            Items.push_back(ExampleAsset(NextItemId, (NextItemId % 20) < 15 ? 0 : (NextItemId % 20) < 18 ? 1 : 2));
        RequestSort = true;
    }
    void ClearItems()
    {
        Items.clear();
        Selection.Clear();
    }

    // Logic would be written in the main code BeginChild() and outputting to local variables.
    // We extracted it into a function so we can call it easily from multiple places.
    void UpdateLayoutSizes(float avail_width)
    {
        // Layout: when not stretching: allow extending into right-most spacing.
        LayoutItemSpacing = (float)IconSpacing;
        if (StretchSpacing == false)
            avail_width += floorf(LayoutItemSpacing * 0.5f);

        // Layout: calculate number of icon per line and number of lines
        LayoutItemSize = LinearVec2(floorf(IconSize), floorf(IconSize));
        LayoutColumnCount = LINEARUI_MAX((int)(avail_width / (LayoutItemSize.x + LayoutItemSpacing)), 1);

        // Layout: when stretching: allocate remaining space to more spacing. Round before division, so item_spacing may be non-integer.
        if (StretchSpacing && LayoutColumnCount > 1)
            LayoutItemSpacing = floorf(avail_width - LayoutItemSize.x * LayoutColumnCount) / LayoutColumnCount;

        if (UseScrollX)
            LayoutColumnCount *= 2;
        LayoutLineCount = (Items.Size + LayoutColumnCount - 1) / LayoutColumnCount;

        LayoutItemStep = LinearVec2(LayoutItemSize.x + LayoutItemSpacing, LayoutItemSize.y + LayoutItemSpacing);
        LayoutSelectableSpacing = LINEARUI_MAX(floorf(LayoutItemSpacing) - IconHitSpacing, 0.0f);
        LayoutOuterPadding = floorf(LayoutItemSpacing * 0.5f);
    }

    void Draw(const char* title, bool* p_open)
    {
        if (IconSize <= 0.0f)
            IconSize = LinearUI::CalcTextSize("99999").x;

        LinearUI::SetNextWindowSize(LinearVec2(IconSize * 25, IconSize * 15), LinearUICond_FirstUseEver);
        if (!LinearUI::Begin(title, p_open, LinearUIWindowFlags_MenuBar))
        {
            LinearUI::End();
            return;
        }
        LINEARUI_DEMO_MARKER("Examples/Assets Browser");

        // Menu bar
        if (LinearUI::BeginMenuBar())
        {
            if (LinearUI::BeginMenu("File"))
            {
                if (LinearUI::MenuItem("Add 10000 items"))
                    AddItems(10000);
                if (LinearUI::MenuItem("Clear items"))
                    ClearItems();
                LinearUI::Separator();
                if (LinearUI::MenuItem("Close", NULL, false, p_open != NULL))
                    *p_open = false;
                LinearUI::EndMenu();
            }
            if (LinearUI::BeginMenu("Edit"))
            {
                if (LinearUI::MenuItem("Delete", "Del", false, Selection.Size > 0))
                    RequestDelete = true;
                LinearUI::EndMenu();
            }
            if (LinearUI::BeginMenu("Options"))
            {
                LinearUI::PushItemWidth(LinearUI::GetFontSize() * 10);

                LinearUI::SeparatorText("Contents");
                LinearUI::Checkbox("Show Type Overlay", &ShowTypeOverlay);
                LinearUI::Checkbox("Allow Sorting", &AllowSorting);

                LinearUI::SeparatorText("Selection Behavior");
                LinearUI::Checkbox("Allow box-selection", &AllowBoxSelect);
                if (LinearUI::Checkbox("Allow box-selection from selected items", &AllowBoxSelectInsideSelection) && AllowBoxSelectInsideSelection)
                    AllowDragUnselected = false;
                if (LinearUI::Checkbox("Allow dragging unselected item", &AllowDragUnselected) && AllowDragUnselected)
                    AllowBoxSelectInsideSelection = false;

                LinearUI::SeparatorText("Layout");
                LinearUI::SliderFloat("Icon Size", &IconSize, 16.0f, 128.0f, "%.0f");
                LinearUI::SameLine(); HelpMarker("Use Ctrl+Wheel to zoom");
                LinearUI::SliderInt("Icon Spacing", &IconSpacing, 0, 32);
                LinearUI::SliderInt("Icon Hit Spacing", &IconHitSpacing, 0, 32);
                LinearUI::Checkbox("Stretch Spacing", &StretchSpacing);
                LinearUI::Checkbox("Use ScrollX", &UseScrollX);
                LinearUI::PopItemWidth();
                LinearUI::EndMenu();
            }
            LinearUI::EndMenuBar();
        }

        // Show a table with ONLY one header row to showcase the idea/possibility of using this to provide a sorting UI
        if (AllowSorting)
        {
            LinearUI::PushStyleVar(LinearUIStyleVar_ItemSpacing, LinearVec2(0, 0));
            LinearUITableFlags table_flags_for_sort_specs = LinearUITableFlags_Sortable | LinearUITableFlags_SortMulti | LinearUITableFlags_SizingFixedFit | LinearUITableFlags_Borders;
            if (LinearUI::BeginTable("for_sort_specs_only", 2, table_flags_for_sort_specs, LinearVec2(0.0f, LinearUI::GetFrameHeight())))
            {
                LinearUI::TableSetupColumn("Index");
                LinearUI::TableSetupColumn("Type");
                LinearUI::TableHeadersRow();
                if (LinearUITableSortSpecs* sort_specs = LinearUI::TableGetSortSpecs())
                    if (sort_specs->SpecsDirty || RequestSort)
                    {
                        ExampleAsset::SortWithSortSpecs(sort_specs, Items.Data, Items.Size);
                        sort_specs->SpecsDirty = RequestSort = false;
                    }
                LinearUI::EndTable();
            }
            LinearUI::PopStyleVar();
        }

        LinearUIIO& io = LinearUI::GetIO();
        LinearUI::SetNextWindowContentSize(LinearVec2(0.0f, LayoutOuterPadding + LayoutLineCount * (LayoutItemSize.y + LayoutItemSpacing)));
        if (LinearUI::BeginChild("Assets", LinearVec2(0.0f, -LinearUI::GetTextLineHeightWithSpacing()), LinearUIChildFlags_Borders, LinearUIWindowFlags_NoMove | LinearUIWindowFlags_HorizontalScrollbar))
        {
            LinearDrawList* draw_list = LinearUI::GetWindowDrawList();

            const float avail_width = LinearUI::GetContentRegionAvail().x;
            UpdateLayoutSizes(avail_width);

            // Calculate and store start position.
            LinearVec2 start_pos = LinearUI::GetCursorScreenPos();
            start_pos = LinearVec2(start_pos.x + LayoutOuterPadding, start_pos.y + LayoutOuterPadding);
            LinearUI::SetCursorScreenPos(start_pos);

            // Multi-select
            LinearUIMultiSelectFlags ms_flags = LinearUIMultiSelectFlags_ClearOnEscape | LinearUIMultiSelectFlags_ClearOnClickVoid;

            // - Enable box-select (in 2D mode, so that changing box-select rectangle X1/X2 boundaries will affect clipped items)
            if (AllowBoxSelect)
                ms_flags |= LinearUIMultiSelectFlags_BoxSelect2d;

            // - Selection mode
            if (AllowDragUnselected)
                ms_flags |= LinearUIMultiSelectFlags_SelectOnClickRelease; // Rarely used: Allows dragging an unselected item without selecting it(rarely used)
            else if (AllowBoxSelectInsideSelection)
                ms_flags |= LinearUIMultiSelectFlags_SelectOnClickAlways; // Rarely used: Prevents Drag and Drop from being used on multiple-selection, but allows e.g. BoxSelect to always reselect even when clicking inside an existing selection.

            // - Enable keyboard wrapping on X axis
            // (FIXME-MULTISELECT: We haven't designed/exposed a general nav wrapping api yet, so this flag is provided as a courtesy to avoid doing:
            //    LinearUI::NavMoveRequestTryWrapping(LinearUI::GetCurrentWindow(), LinearUINavMoveFlags_WrapX);
            // When we finish implementing a more general API for this, we will obsolete this flag in favor of the new system)
            ms_flags |= LinearUIMultiSelectFlags_NavWrapX;

            LinearUIMultiSelectIO* ms_io = LinearUI::BeginMultiSelect(ms_flags, Selection.Size, Items.Size);

            // Use custom selection adapter: store ID in selection (recommended)
            Selection.UserData = this;
            Selection.AdapterIndexToStorageId = [](LinearUISelectionBasicStorage* self_, int idx) { ExampleAssetsBrowser* self = (ExampleAssetsBrowser*)self_->UserData; return self->Items[idx].ID; };
            Selection.ApplyRequests(ms_io);

            const bool want_delete = (LinearUI::Shortcut(LinearUIKey_Delete, LinearUIInputFlags_Repeat) && (Selection.Size > 0)) || RequestDelete;
            const int item_curr_idx_to_focus = want_delete ? Selection.ApplyDeletionPreLoop(ms_io, Items.Size) : -1;
            RequestDelete = false;

            // Push LayoutSelectableSpacing (which is LayoutItemSpacing minus hit-spacing, if we decide to have hit gaps between items)
            // Altering style ItemSpacing may seem unnecessary as we position every items using SetCursorScreenPos()...
            // But it is necessary for two reasons:
            // - Selectables uses it by default to visually fill the space between two items.
            // - The vertical spacing would be measured by Clipper to calculate line height if we didn't provide it explicitly (here we do).
            LinearUI::PushStyleVar(LinearUIStyleVar_ItemSpacing, LinearVec2(LayoutSelectableSpacing, LayoutSelectableSpacing));

            // Rendering parameters
            const LinearU32 icon_type_overlay_colors[3] = { 0, LINEARUI_COL32(200, 70, 70, 255), LINEARUI_COL32(70, 170, 70, 255) };
            const LinearU32 icon_bg_color = LinearUI::GetColorU32(LINEARUI_COL32(35, 35, 35, 220));
            const LinearVec2 icon_type_overlay_size = LinearVec2(4.0f, 4.0f);
            const bool display_label = (LayoutItemSize.x >= LinearUI::CalcTextSize("999").x);

            const int column_count = LayoutColumnCount;
            LinearUIListClipper clipper;
            clipper.Begin(LayoutLineCount, LayoutItemStep.y);
            if (item_curr_idx_to_focus != -1)
                clipper.IncludeItemByIndex(item_curr_idx_to_focus / column_count); // Ensure focused item line is not clipped.
            if (ms_io->RangeSrcItem != -1)
                clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem / column_count); // Ensure RangeSrc item line is not clipped.
            while (clipper.Step())
            {
                for (int line_idx = clipper.DisplayStart; line_idx < clipper.DisplayEnd; line_idx++)
                {
                    const int item_min_idx_for_current_line = line_idx * column_count;
                    const int item_max_idx_for_current_line = LINEARUI_MIN((line_idx + 1) * column_count, Items.Size);
                    for (int item_idx = item_min_idx_for_current_line; item_idx < item_max_idx_for_current_line; ++item_idx)
                    {
                        ExampleAsset* item_data = &Items[item_idx];
                        LinearUI::PushID((int)item_data->ID);

                        // Position item
                        LinearVec2 pos = LinearVec2(start_pos.x + (item_idx % column_count) * LayoutItemStep.x, start_pos.y + line_idx * LayoutItemStep.y);
                        LinearUI::SetCursorScreenPos(pos);

                        LinearUI::SetNextItemSelectionUserData(item_idx);
                        bool item_is_selected = Selection.Contains((LinearUIID)item_data->ID);
                        bool item_is_visible = LinearUI::IsRectVisible(LayoutItemSize);
                        LinearUI::Selectable("", item_is_selected, LinearUISelectableFlags_None, LayoutItemSize);

                        // Update our selection state immediately (without waiting for EndMultiSelect() requests)
                        // because we use this to alter the color of our text/icon.
                        if (LinearUI::IsItemToggledSelection())
                            item_is_selected = !item_is_selected;

                        // Focus (for after deletion)
                        if (item_curr_idx_to_focus == item_idx)
                            LinearUI::SetKeyboardFocusHere(-1);

                        // Drag and drop
                        if (LinearUI::BeginDragDropSource())
                        {
                            // Create payload with full selection OR single unselected item.
                            // (the later is only possible when using LinearUIMultiSelectFlags_SelectOnClickRelease)
                            if (LinearUI::GetDragDropPayload() == NULL)
                            {
                                LinearVector<LinearUIID> payload_items;
                                void* it = NULL;
                                LinearUIID id = 0;
                                if (!item_is_selected)
                                    payload_items.push_back(item_data->ID);
                                else
                                    while (Selection.GetNextSelectedItem(&it, &id))
                                        payload_items.push_back(id);
                                LinearUI::SetDragDropPayload("ASSETS_BROWSER_ITEMS", payload_items.Data, (size_t)payload_items.size_in_bytes());
                            }

                            // Display payload content in tooltip, by extracting it from the payload data
                            // (we could read from selection, but it is more correct and reusable to read from payload)
                            const LinearUIPayload* payload = LinearUI::GetDragDropPayload();
                            const int payload_count = (int)payload->DataSize / (int)sizeof(LinearUIID);
                            LinearUI::Text("%d assets", payload_count);

                            LinearUI::EndDragDropSource();
                        }

                        // Render icon (a real app would likely display an image/thumbnail here)
                        // Because we use LinearUIMultiSelectFlags_BoxSelect2d, clipping vertical may occasionally be larger, so we coarse-clip our rendering as well.
                        if (item_is_visible)
                        {
                            LinearVec2 box_min(pos.x - 1, pos.y - 1);
                            LinearVec2 box_max(box_min.x + LayoutItemSize.x + 2, box_min.y + LayoutItemSize.y + 2); // Dubious
                            draw_list->AddRectFilled(box_min, box_max, icon_bg_color); // Background color
                            if (ShowTypeOverlay && item_data->Type != 0)
                            {
                                LinearU32 type_col = icon_type_overlay_colors[item_data->Type % LINEARUI_COUNTOF(icon_type_overlay_colors)];
                                draw_list->AddRectFilled(LinearVec2(box_max.x - 2 - icon_type_overlay_size.x, box_min.y + 2), LinearVec2(box_max.x - 2, box_min.y + 2 + icon_type_overlay_size.y), type_col);
                            }
                            if (display_label)
                            {
                                LinearU32 label_col = LinearUI::GetColorU32(item_is_selected ? LinearUICol_Text : LinearUICol_TextDisabled);
                                char label[32];
                                sprintf(label, "%d", item_data->ID);
                                draw_list->AddText(LinearVec2(box_min.x, box_max.y - LinearUI::GetFontSize()), label_col, label);
                            }
                        }

                        LinearUI::PopID();
                    }
                }
            }
            clipper.End();
            if (Items.Size == 0)
                LinearUI::Dummy(LinearVec2(0, 0));
            LinearUI::PopStyleVar(); // LinearUIStyleVar_ItemSpacing

            // Context menu
            if (LinearUI::BeginPopupContextWindow())
            {
                LinearUI::Text("Selection: %d items", Selection.Size);
                LinearUI::Separator();
                if (LinearUI::MenuItem("Delete", "Del", false, Selection.Size > 0))
                    RequestDelete = true;
                LinearUI::EndPopup();
            }

            ms_io = LinearUI::EndMultiSelect();
            Selection.ApplyRequests(ms_io);
            if (want_delete)
                Selection.ApplyDeletionPostLoop(ms_io, Items, item_curr_idx_to_focus);

            // Zooming with Ctrl+Wheel
            if (LinearUI::IsWindowAppearing())
                ZoomWheelAccum = 0.0f;
            if (LinearUI::IsWindowHovered() && io.MouseWheel != 0.0f && LinearUI::IsKeyDown(LinearUIMod_Ctrl) && LinearUI::IsAnyItemActive() == false)
            {
                ZoomWheelAccum += io.MouseWheel;
                if (fabsf(ZoomWheelAccum) >= 1.0f)
                {
                    // Calculate hovered item index from mouse location
                    // FIXME: Locking aiming on 'hovered_item_idx' (with a cool-down timer) would ensure zoom keeps on it.
                    const float hovered_item_nx = (io.MousePos.x - start_pos.x + LayoutItemSpacing * 0.5f) / LayoutItemStep.x;
                    const float hovered_item_ny = (io.MousePos.y - start_pos.y + LayoutItemSpacing * 0.5f) / LayoutItemStep.y;
                    const int hovered_item_idx = ((int)hovered_item_ny * LayoutColumnCount) + (int)hovered_item_nx;
                    //LinearUI::SetTooltip("%f,%f -> item %d", hovered_item_nx, hovered_item_ny, hovered_item_idx); // Move those 4 lines in block above for easy debugging

                    // Zoom
                    IconSize *= powf(1.1f, (float)(int)ZoomWheelAccum);
                    IconSize = LINEARUI_CLAMP(IconSize, 16.0f, 128.0f);
                    ZoomWheelAccum -= (int)ZoomWheelAccum;
                    UpdateLayoutSizes(avail_width);

                    // Manipulate scroll to that we will land at the same Y location of currently hovered item.
                    // - Calculate next frame position of item under mouse
                    // - Set new scroll position to be used in next LinearUI::BeginChild() call.
                    float hovered_item_rel_pos_y = ((float)(hovered_item_idx / LayoutColumnCount) + fmodf(hovered_item_ny, 1.0f)) * LayoutItemStep.y;
                    hovered_item_rel_pos_y += LinearUI::GetStyle().WindowPadding.y;
                    float mouse_local_y = io.MousePos.y - LinearUI::GetWindowPos().y;
                    LinearUI::SetScrollY(hovered_item_rel_pos_y - mouse_local_y);
                }
            }
        }
        LinearUI::EndChild();

        LinearUI::Text("Selected: %d/%d items", Selection.Size, Items.Size);
        LinearUI::End();
    }
};

void ShowExampleAppAssetsBrowser(bool* p_open)
{
    LINEARUI_DEMO_MARKER("Examples/Assets Browser");
    static ExampleAssetsBrowser assets_browser;
    assets_browser.Draw("Example: Assets Browser", p_open);
}

// End of Demo code
#else

void LinearUI::ShowAboutWindow(bool*) {}
void LinearUI::ShowDemoWindow(bool*) {}
void LinearUI::ShowUserGuide() {}
void LinearUI::ShowStyleEditor(LinearUIStyle*) {}
bool LinearUI::ShowStyleSelector(const char*) { return false; }

#endif // #ifndef LINEARUI_DISABLE_DEMO_WINDOWS

#endif // #ifndef LINEARUI_DISABLE
