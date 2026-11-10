// dear rblxui, v1.92.9 WIP
// (demo code)

// Help:
// - Read FAQ at http://dearrblxui.com/faq
// - Call and read RblxUI::ShowDemoWindow() in rblxui_demo.cpp. All applications in examples/ are doing that.
// - Need help integrating Dear RblxUI in your codebase?
//   - Read Getting Started https://github.com/ocornut/rblxui/wiki/Getting-Started
//   - Read 'Programmer guide' in rblxui.cpp for notes on how to setup Dear RblxUI in your codebase.
// Read top of rblxui.cpp and rblxui.h for many details, documentation, comments, links.
// Get the latest version at https://github.com/ocornut/rblxui

// How to easily locate code?
// - Use Tools->Item Picker to debug break in code by clicking any widgets: https://github.com/ocornut/rblxui/wiki/Debug-Tools
// - Browse pthom's online rblxui_explorer: web version the demo w/ source code browser: https://pthom.github.io/rblxui_explorer
// - Find a visible string and search for it in the code!

//---------------------------------------------------
// PLEASE DO NOT REMOVE THIS FILE FROM YOUR PROJECT!
//---------------------------------------------------
// Message to the person tempted to delete this file when integrating Dear RblxUI into their codebase:
// Think again! It is the most useful reference code that you and other coders will want to refer to and call.
// Have the RblxUI::ShowDemoWindow() function wired in an always-available debug menu of your game/app!
// Also include Metrics! ItemPicker! DebugLog! and other debug features.
// Removing this file from your project is hindering access to documentation for everyone in your team,
// likely leading you to poorer usage of the library.
// Everything in this file will be stripped out by the linker if you don't call RblxUI::ShowDemoWindow().
// If you want to link core Dear RblxUI in your shipped builds but want a thorough guarantee that the demo will not be
// linked, you can setup your rblxuiconfig.h with #define RBLXUI_DISABLE_DEMO_WINDOWS and those functions will be empty.
// In another situation, whenever you have Dear RblxUI available you probably want this to be available for reference.
// Thank you,
// -Your beloved friend, rblxui_demo.cpp (which you won't delete)

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
// - We never omit the RblxUI:: prefix when calling functions, even though most code here is in the same namespace.
// - We try to declare static variables in the local scope, as close as possible to the code using them.
// - We never use any of the helpers/facilities used internally by Dear RblxUI, unless available in the public API.
// - We never use maths operators on RblxVec2/RblxVec4. For our other sources files we use them, and they are provided
//   by rblxui.h using the RBLXUI_DEFINE_MATH_OPERATORS define. For your own sources file they are optional
//   and require you either enable those, either provide your own via RBLXUI_VEC2_CLASS_EXTRA in rblxuiconfig.h.
//   Because we can't assume anything about your support of maths operators, we cannot use them in rblxui_demo.cpp.

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
// [SECTION] Example App: Custom Rendering using RblxDrawList API / ShowExampleAppCustomRendering()
// [SECTION] Example App: Docking, DockSpace / ShowExampleAppDockSpace()
// [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()
// [SECTION] Example App: Assets Browser / ShowExampleAppAssetsBrowser()

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "rblxui.h"
#ifndef RBLXUI_DISABLE

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
#pragma clang diagnostic ignored "-Wexit-time-destructors"          // warning: declaration requires an exit-time destructor    // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. RblxUI coding style welcomes static/globals.
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
#pragma GCC diagnostic ignored "-Wformat"                           // warning: format '%p' expects argument of type 'int'/'void*', but argument X has type 'unsigned int'/'RblxUIWindow*'
#pragma GCC diagnostic ignored "-Wformat-security"                  // warning: format string is not a string literal (potentially insecure)
#pragma GCC diagnostic ignored "-Wdouble-promotion"                 // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"                       // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wmisleading-indentation"           // [__GNUC__ >= 6] warning: this 'if' clause does not guard this statement      // GCC 6.0+ only. See #883 on GitHub.
#pragma GCC diagnostic ignored "-Wstrict-overflow"                  // warning: assuming signed overflow does not occur when simplifying division / ..when changing X +- C1 cmp C2 to X cmp C2 -+ C1
#pragma GCC diagnostic ignored "-Wcast-qual"                        // warning: cast from type 'const xxxx *' to type 'xxxx *' casts away qualifiers
#endif

// Play it nice with Windows users (Update: May 2018, Notepad now supports Unix-style carriage returns!)
#ifdef _WIN32
#define RBLXUI_NEWLINE  "\r\n"
#else
#define RBLXUI_NEWLINE  "\n"
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
// We normally try to not use many helpers in rblxui_demo.cpp in order to make code easier to copy and paste,
// but making an exception here as those are largely simplifying code...
// In other rblxui sources we can use nicer internal functions from rblxui_internal.h (RblxMin/RblxMax) but not in the demo.
#define RBLXUI_MIN(A, B)            (((A) < (B)) ? (A) : (B))
#define RBLXUI_MAX(A, B)            (((A) >= (B)) ? (A) : (B))
#define RBLXUI_CLAMP(V, MN, MX)     ((V) < (MN) ? (MN) : (V) > (MX) ? (MX) : (V))

// Enforce cdecl calling convention for functions called by the standard library,
// in case compilation settings changed the default to e.g. __vectorcall
#ifndef RBLXUI_CDECL
#ifdef _MSC_VER
#define RBLXUI_CDECL __cdecl
#else
#define RBLXUI_CDECL
#endif
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward Declarations
//-----------------------------------------------------------------------------

#if !defined(RBLXUI_DISABLE_DEMO_WINDOWS)

// Forward Declarations
struct RblxUIDemoWindowData;
static void ShowExampleAppMainMenuBar();
static void ShowExampleAppAssetsBrowser(bool* p_open);
static void ShowExampleAppConsole(bool* p_open);
static void ShowExampleAppCustomRendering(bool* p_open);
static void ShowExampleAppDockSpace(bool* p_open);
static void ShowExampleAppDocuments(bool* p_open);
static void ShowExampleAppImageViewer(bool* p_open);
static void ShowExampleAppLog(bool* p_open);
static void ShowExampleAppLayout(bool* p_open);
static void ShowExampleAppPropertyEditor(bool* p_open, RblxUIDemoWindowData* demo_data);
static void ShowExampleAppSimpleOverlay(bool* p_open);
static void ShowExampleAppAutoResize(bool* p_open);
static void ShowExampleAppConstrainedResize(bool* p_open);
static void ShowExampleAppFullscreen(bool* p_open);
static void ShowExampleAppLongText(bool* p_open);
static void ShowExampleAppWindowTitles(bool* p_open);
static void ShowExampleMenuFile();

// We split the contents of the big ShowDemoWindow() function into smaller functions
// (because the link time of very large functions tends to grow non-linearly)
static void DemoWindowMenuBar(RblxUIDemoWindowData* demo_data);
static void DemoWindowWidgets(RblxUIDemoWindowData* demo_data);
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
    RblxUI::TextDisabled("(?)");
    if (RblxUI::BeginItemTooltip())
    {
        RblxUI::PushTextWrapPos(RblxUI::GetFontSize() * 35.0f);
        RblxUI::TextUnformatted(desc);
        RblxUI::PopTextWrapPos();
        RblxUI::EndTooltip();
    }
}

static void ShowDockingDisabledMessage()
{
    RblxUIIO& io = RblxUI::GetIO();
    RblxUI::Text("ERROR: Docking is not enabled! See Demo > Configuration.");
    RblxUI::Text("Set io.ConfigFlags |= RblxUIConfigFlags_DockingEnable in your code, or ");
    RblxUI::SameLine(0.0f, 0.0f);
    if (RblxUI::SmallButton("click here"))
        io.ConfigFlags |= RblxUIConfigFlags_DockingEnable;
}

// Helper to wire demo markers located in code to an interactive browser (e.g. https://pthom.github.io/rblxui_explorer)
#if RBLXUI_VERSION_NUM >= 19263
namespace RblxUI { extern RBLXUI_API void DemoMarker(const char* file, int line, const char* section); }
#define RBLXUI_DEMO_MARKER(section)  do { RblxUI::DemoMarker("rblxui_demo.cpp", __LINE__, section); } while (0)
#endif

// Sneakily forward declare functions which aren't worth putting in public API yet
namespace RblxUI
{
    RBLXUI_API void ShowFontAtlas(RblxFontAtlas* atlas);
    RBLXUI_API void TreeNodeSetOpen(RblxUIID storage_id, bool is_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Demo Window / ShowDemoWindow()
//-----------------------------------------------------------------------------

// Data to be shared across different functions of the demo.
struct RblxUIDemoWindowData
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

    // Dear RblxUI Tools (accessible from the "Tools" menu)
    bool ShowMetrics = false;
    bool ShowDebugLog = false;
    bool ShowIDStackTool = false;
    bool ShowStyleEditor = false;
    bool ShowAbout = false;

    // Other data
    bool DisableSections = false;
    ExampleTreeNode* DemoTree = NULL;

    ~RblxUIDemoWindowData() { if (DemoTree) ExampleTree_DestroyNode(DemoTree); }
};

// Demonstrate most Dear RblxUI features (this is big function!)
// You may execute this function to experiment with the UI and understand what it does.
// You may then search for keywords in the code when you are interested by a specific feature.
void RblxUI::ShowDemoWindow(bool* p_open)
{
    // Exceptionally add an extra assert here for people confused about initial Dear RblxUI setup
    // Most functions would normally just assert/crash if the context is missing.
    RBLXUI_ASSERT(RblxUI::GetCurrentContext() != NULL && "Missing Dear RblxUI context. Refer to examples app!");

    // Verify ABI compatibility between caller code and compiled version of Dear RblxUI. This helps detects some build issues.
    RBLXUI_CHECKVERSION();

    // Stored data
    static RblxUIDemoWindowData demo_data;

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

    // Dear RblxUI Tools (accessible from the "Tools" menu)
    if (demo_data.ShowMetrics)              { RblxUI::ShowMetricsWindow(&demo_data.ShowMetrics); }
    if (demo_data.ShowDebugLog)             { RblxUI::ShowDebugLogWindow(&demo_data.ShowDebugLog); }
    if (demo_data.ShowIDStackTool)          { RblxUI::ShowIDStackToolWindow(&demo_data.ShowIDStackTool); }
    if (demo_data.ShowAbout)                { RblxUI::ShowAboutWindow(&demo_data.ShowAbout); }
    if (demo_data.ShowStyleEditor)
    {
        RblxUI::Begin("Dear RblxUI Style Editor", &demo_data.ShowStyleEditor);
        RblxUI::ShowStyleEditor();
        RblxUI::End();
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

    RblxUIWindowFlags window_flags = 0;
    if (no_titlebar)        window_flags |= RblxUIWindowFlags_NoTitleBar;
    if (no_scrollbar)       window_flags |= RblxUIWindowFlags_NoScrollbar;
    if (!no_menu)           window_flags |= RblxUIWindowFlags_MenuBar;
    if (no_move)            window_flags |= RblxUIWindowFlags_NoMove;
    if (no_resize)          window_flags |= RblxUIWindowFlags_NoResize;
    if (no_collapse)        window_flags |= RblxUIWindowFlags_NoCollapse;
    if (no_nav)             window_flags |= RblxUIWindowFlags_NoNav;
    if (no_background)      window_flags |= RblxUIWindowFlags_NoBackground;
    if (no_bring_to_front)  window_flags |= RblxUIWindowFlags_NoBringToFrontOnFocus;
    if (no_docking)         window_flags |= RblxUIWindowFlags_NoDocking;
    if (unsaved_document)   window_flags |= RblxUIWindowFlags_UnsavedDocument;
    if (no_close)           p_open = NULL; // Don't pass our bool* to Begin

    // We specify a default position/size in case there's no data in the .ini file.
    // We only do it to make the demo applications a little more welcoming, but typically this isn't required.
    const RblxUIViewport* main_viewport = RblxUI::GetMainViewport();
    RblxUI::SetNextWindowPos(RblxVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), RblxUICond_FirstUseEver);
    RblxUI::SetNextWindowSize(RblxVec2(550, 680), RblxUICond_FirstUseEver);

    // Main body of the Demo window starts here.
    if (!RblxUI::Begin("Dear RblxUI Demo", p_open, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        RblxUI::End();
        return;
    }

    // Most framed widgets share a common width settings. Remaining width is used for the label.
    // The width of the frame may be changed with PushItemWidth() or SetNextItemWidth().
    // - Positive value for absolute size, negative value for right-alignment.
    // - The default value is about GetWindowWidth() * 0.65f.
    // - See 'Demo->Layout->Widgets Width' for details.
    // Here we change the frame width based on how much width we want to give to the label.
    const float label_width_base = RblxUI::GetFontSize() * 12;               // Some amount of width for label, based on font size.
    const float label_width_max = RblxUI::GetContentRegionAvail().x * 0.40f; // ...but always leave some room for framed widgets.
    const float label_width = RBLXUI_MIN(label_width_base, label_width_max);
    RblxUI::PushItemWidth(-label_width);                                     // Right-align: framed items will leave 'label_width' available for the label.
    //RblxUI::PushItemWidth(RblxUI::GetContentRegionAvail().x * 0.40f);       // e.g. Use 40% width for framed widgets, leaving 60% width for labels.
    //RblxUI::PushItemWidth(-RblxUI::GetContentRegionAvail().x * 0.40f);      // e.g. Use 40% width for labels, leaving 60% width for framed widgets.
    //RblxUI::PushItemWidth(RblxUI::GetFontSize() * -12);                     // e.g. Use XXX width for labels, leaving the rest for framed widgets.

    // Menu Bar
    DemoWindowMenuBar(&demo_data);

    RblxUI::Text("dear rblxui says hello! (%s) (%d)", RBLXUI_VERSION, RBLXUI_VERSION_NUM);
    RblxUI::Spacing();

    if (RblxUI::CollapsingHeader("Help"))
    {
        RBLXUI_DEMO_MARKER("Help");
        RblxUI::SeparatorText("ABOUT THIS DEMO:");
        RblxUI::BulletText("Sections below are demonstrating many aspects of the library.");
        RblxUI::BulletText("The \"Examples\" menu above leads to more demo contents.");
        RblxUI::BulletText("The \"Tools\" menu above gives access to: About Box, Style Editor,\n"
                          "and Metrics/Debugger (general purpose Dear RblxUI debugging tool).");
        RblxUI::BulletText("Web demo (w/ source code browser): ");
        RblxUI::SameLine(0, 0);
        RblxUI::TextLinkOpenURL("https://pthom.github.io/rblxui_explorer");

        RblxUI::SeparatorText("PROGRAMMER GUIDE:");
        RblxUI::BulletText("See the ShowDemoWindow() code in rblxui_demo.cpp. <- you are here!");
        RblxUI::BulletText("See comments in rblxui.cpp.");
        RblxUI::BulletText("See example applications in the examples/ folder.");
        RblxUI::BulletText("Read the FAQ at ");
        RblxUI::SameLine(0, 0);
        RblxUI::TextLinkOpenURL("https://www.dearrblxui.com/faq/");
        RblxUI::BulletText("Set 'io.ConfigFlags |= NavEnableKeyboard' for keyboard controls.");
        RblxUI::BulletText("Set 'io.ConfigFlags |= NavEnableGamepad' for gamepad controls.");

        RblxUI::SeparatorText("USER GUIDE:");
        RblxUI::ShowUserGuide();
    }

    if (RblxUI::CollapsingHeader("Configuration"))
    {
        RblxUIIO& io = RblxUI::GetIO();

        if (RblxUI::TreeNode("Configuration##2"))
        {
            RBLXUI_DEMO_MARKER("Configuration");
            RblxUI::SeparatorText("General");
            RblxUI::CheckboxFlags("io.ConfigFlags: NavEnableKeyboard",    &io.ConfigFlags, RblxUIConfigFlags_NavEnableKeyboard);
            RblxUI::SameLine(); HelpMarker("Enable keyboard controls.");
            RblxUI::CheckboxFlags("io.ConfigFlags: NavEnableGamepad",     &io.ConfigFlags, RblxUIConfigFlags_NavEnableGamepad);
            RblxUI::SameLine(); HelpMarker("Enable gamepad controls. Require backend to set io.BackendFlags |= RblxUIBackendFlags_HasGamepad.\n\nRead instructions in rblxui.cpp for details.");
            RblxUI::CheckboxFlags("io.ConfigFlags: NoMouse",              &io.ConfigFlags, RblxUIConfigFlags_NoMouse);
            RblxUI::SameLine(); HelpMarker("Instruct dear rblxui to disable mouse inputs and interactions.");

            // The "NoMouse" option can get us stuck with a disabled mouse! Let's provide an alternative way to fix it:
            if (io.ConfigFlags & RblxUIConfigFlags_NoMouse)
            {
                if (fmodf((float)RblxUI::GetTime(), 0.40f) < 0.20f)
                {
                    RblxUI::SameLine();
                    RblxUI::Text("<<PRESS SPACE TO DISABLE>>");
                }
                // Prevent both being checked
                if (RblxUI::IsKeyPressed(RblxUIKey_Space) || (io.ConfigFlags & RblxUIConfigFlags_NoKeyboard))
                    io.ConfigFlags &= ~RblxUIConfigFlags_NoMouse;
            }

            RblxUI::CheckboxFlags("io.ConfigFlags: NoMouseCursorChange",  &io.ConfigFlags, RblxUIConfigFlags_NoMouseCursorChange);
            RblxUI::SameLine(); HelpMarker("Instruct backend to not alter mouse cursor shape and visibility.");
            RblxUI::CheckboxFlags("io.ConfigFlags: NoKeyboard", &io.ConfigFlags, RblxUIConfigFlags_NoKeyboard);
            RblxUI::SameLine(); HelpMarker("Instruct dear rblxui to disable keyboard inputs and interactions.");

            RblxUI::Checkbox("io.ConfigInputTrickleEventQueue", &io.ConfigInputTrickleEventQueue);
            RblxUI::SameLine(); HelpMarker("Enable input queue trickling: some types of events submitted during the same frame (e.g. button down + up) will be spread over multiple frames, improving interactions with low framerates.");
            RblxUI::Checkbox("io.MouseDrawCursor", &io.MouseDrawCursor);
            RblxUI::SameLine(); HelpMarker("Instruct Dear RblxUI to render a mouse cursor itself. Note that a mouse cursor rendered via your application GPU rendering path will feel more laggy than hardware cursor, but will be more in sync with your other visuals.\n\nSome desktop applications may use both kinds of cursors (e.g. enable software cursor only when resizing/dragging something).");

            RblxUI::SeparatorText("Keyboard/Gamepad Navigation");
            RblxUI::Checkbox("io.ConfigNavSwapGamepadButtons", &io.ConfigNavSwapGamepadButtons);
            RblxUI::Checkbox("io.ConfigNavMoveSetMousePos", &io.ConfigNavMoveSetMousePos);
            RblxUI::SameLine(); HelpMarker("Directional/tabbing navigation teleports the mouse cursor. May be useful on TV/console systems where moving a virtual mouse is difficult");
            RblxUI::Checkbox("io.ConfigNavCaptureKeyboard", &io.ConfigNavCaptureKeyboard);
            RblxUI::Checkbox("io.ConfigNavEscapeClearFocusItem", &io.ConfigNavEscapeClearFocusItem);
            RblxUI::SameLine(); HelpMarker("Pressing Escape clears focused item.");
            RblxUI::Checkbox("io.ConfigNavEscapeClearFocusWindow", &io.ConfigNavEscapeClearFocusWindow);
            RblxUI::SameLine(); HelpMarker("Pressing Escape clears focused window.");
            RblxUI::Checkbox("io.ConfigNavCursorVisibleAuto", &io.ConfigNavCursorVisibleAuto);
            RblxUI::SameLine(); HelpMarker("Using directional navigation key makes the cursor visible. Mouse click hides the cursor.");
            RblxUI::Checkbox("io.ConfigNavCursorVisibleAlways", &io.ConfigNavCursorVisibleAlways);
            RblxUI::SameLine(); HelpMarker("Navigation cursor is always visible.");

            RblxUI::SeparatorText("Docking");
            RblxUI::CheckboxFlags("io.ConfigFlags: DockingEnable", &io.ConfigFlags, RblxUIConfigFlags_DockingEnable);
            RblxUI::SameLine();
            if (io.ConfigDockingWithShift)
                HelpMarker("Drag from window title bar or their tab to dock/undock. Hold SHIFT to enable docking.\n\nDrag from window menu button (upper-left button) to undock an entire node (all windows).");
            else
                HelpMarker("Drag from window title bar or their tab to dock/undock. Hold SHIFT to disable docking.\n\nDrag from window menu button (upper-left button) to undock an entire node (all windows).");
            if (io.ConfigFlags & RblxUIConfigFlags_DockingEnable)
            {
                RblxUI::Indent();
                RblxUI::Checkbox("io.ConfigDockingNoSplit", &io.ConfigDockingNoSplit);
                RblxUI::SameLine(); HelpMarker("Simplified docking mode: disable window splitting, so docking is limited to merging multiple windows together into tab-bars.");
                RblxUI::Checkbox("io.ConfigDockingNoDockingOver", &io.ConfigDockingNoDockingOver);
                RblxUI::SameLine(); HelpMarker("Simplified docking mode: disable window merging into a same tab-bar, so docking is limited to splitting windows.");
                RblxUI::Checkbox("io.ConfigDockingWithShift", &io.ConfigDockingWithShift);
                RblxUI::SameLine(); HelpMarker("Enable docking when holding Shift only (allow to drop in wider space, reduce visual noise)");
                RblxUI::Checkbox("io.ConfigDockingAlwaysTabBar", &io.ConfigDockingAlwaysTabBar);
                RblxUI::SameLine(); HelpMarker("Create a docking node and tab-bar on single floating windows.");
                RblxUI::Checkbox("io.ConfigDockingTransparentPayload", &io.ConfigDockingTransparentPayload);
                RblxUI::SameLine(); HelpMarker("Make window or viewport transparent when docking and only display docking boxes on the target viewport. Useful if rendering of multiple viewport cannot be synced. Best used with ConfigViewportsNoAutoMerge.");
                RblxUI::Unindent();
            }

            RblxUI::SeparatorText("Multi-viewports");
            RblxUI::CheckboxFlags("io.ConfigFlags: ViewportsEnable", &io.ConfigFlags, RblxUIConfigFlags_ViewportsEnable);
            RblxUI::SameLine(); HelpMarker("[beta] Enable beta multi-viewports support. See RblxUIPlatformIO for details.");
            if (io.ConfigFlags & RblxUIConfigFlags_ViewportsEnable)
            {
                RblxUI::Indent();
                RblxUI::Checkbox("io.ConfigViewportsNoAutoMerge", &io.ConfigViewportsNoAutoMerge);
                RblxUI::SameLine(); HelpMarker("Set to make all floating rblxui windows always create their own viewport. Otherwise, they are merged into the main host viewports when overlapping it.");
                RblxUI::Checkbox("io.ConfigViewportsNoTaskBarIcon", &io.ConfigViewportsNoTaskBarIcon);
                RblxUI::SameLine(); HelpMarker("(note: some platform backends may not reflect a change of this value for existing viewports, and may need the viewport to be recreated)");
                RblxUI::Checkbox("io.ConfigViewportsNoDecoration", &io.ConfigViewportsNoDecoration);
                RblxUI::SameLine(); HelpMarker("(note: some platform backends may not reflect a change of this value for existing viewports, and may need the viewport to be recreated)");
                RblxUI::Checkbox("io.ConfigViewportsNoDefaultParent", &io.ConfigViewportsNoDefaultParent);
                RblxUI::SameLine(); HelpMarker("(note: some platform backends may not reflect a change of this value for existing viewports, and may need the viewport to be recreated)");
                RblxUI::Checkbox("io.ConfigViewportsPlatformFocusSetsRblxUIFocus", &io.ConfigViewportsPlatformFocusSetsRblxUIFocus);
                RblxUI::SameLine(); HelpMarker("When a platform window is focused (e.g. using Alt+Tab, clicking Platform Title Bar), apply corresponding focus on rblxui windows (may clear focus/active id from rblxui windows location in other platform windows). In principle this is better enabled but we provide an opt-out, because some Linux window managers tend to eagerly focus windows (e.g. on mouse hover, or even a simple window pos/size change).");
                RblxUI::Unindent();
            }

            //RblxUI::SeparatorText("DPI/Scaling");
            //RblxUI::Checkbox("io.ConfigDpiScaleFonts", &io.ConfigDpiScaleFonts);
            //RblxUI::SameLine(); HelpMarker("Experimental: Automatically update style.FontScaleDpi when Monitor DPI changes. This will scale fonts but NOT style sizes/padding for now.");
            //RblxUI::Checkbox("io.ConfigDpiScaleViewports", &io.ConfigDpiScaleViewports);
            //RblxUI::SameLine(); HelpMarker("Experimental: Scale Dear RblxUI and Platform Windows when Monitor DPI changes.");

            RblxUI::SeparatorText("Windows");
            RblxUI::Checkbox("io.ConfigWindowsResizeFromEdges", &io.ConfigWindowsResizeFromEdges);
            RblxUI::SameLine(); HelpMarker("Enable resizing of windows from their edges and from the lower-left corner.\nThis requires RblxUIBackendFlags_HasMouseCursors for better mouse cursor feedback.");
            RblxUI::Checkbox("io.ConfigWindowsMoveFromTitleBarOnly", &io.ConfigWindowsMoveFromTitleBarOnly);
            RblxUI::Checkbox("io.ConfigWindowsCopyContentsWithCtrlC", &io.ConfigWindowsCopyContentsWithCtrlC); // [EXPERIMENTAL]
            RblxUI::SameLine(); HelpMarker("*EXPERIMENTAL* Ctrl+C copy the contents of focused window into the clipboard.\n\nExperimental because:\n- (1) has known issues with nested Begin/End pairs.\n- (2) text output quality varies.\n- (3) text output is in submission order rather than spatial order.");
            RblxUI::Checkbox("io.ConfigScrollbarScrollByPage", &io.ConfigScrollbarScrollByPage);
            RblxUI::SameLine(); HelpMarker("Enable scrolling page by page when clicking outside the scrollbar grab.\nWhen disabled, always scroll to clicked location.\nWhen enabled, Shift+Click scrolls to clicked location.");

            RblxUI::SeparatorText("Widgets");
            RblxUI::Checkbox("io.ConfigInputTextCursorBlink", &io.ConfigInputTextCursorBlink);
            RblxUI::SameLine(); HelpMarker("Enable blinking cursor (optional as some users consider it to be distracting).");
            RblxUI::Checkbox("io.ConfigInputTextEnterKeepActive", &io.ConfigInputTextEnterKeepActive);
            RblxUI::SameLine(); HelpMarker("Pressing Enter will reactivate item and select all text (single-line only).");
            RblxUI::Checkbox("io.ConfigDragClickToInputText", &io.ConfigDragClickToInputText);
            RblxUI::SameLine(); HelpMarker("Enable turning DragXXX widgets into text input with a simple mouse click-release (without moving).");
            RblxUI::Checkbox("io.ConfigMacOSXBehaviors", &io.ConfigMacOSXBehaviors);
            RblxUI::SameLine(); HelpMarker("Swap Cmd<>Ctrl keys, enable various MacOS style behaviors.");
            RblxUI::Text("Also see Style->Rendering for rendering options.");

            // Also read: https://github.com/ocornut/rblxui/wiki/Error-Handling
            RblxUI::SeparatorText("Error Handling");

            RblxUI::Checkbox("io.ConfigErrorRecovery", &io.ConfigErrorRecovery);
            RblxUI::SameLine(); HelpMarker(
                "Options to configure how we handle recoverable errors.\n"
                "- Error recovery is not perfect nor guaranteed! It is a feature to ease development.\n"
                "- You not are not supposed to rely on it in the course of a normal application run.\n"
                "- Possible usage: facilitate recovery from errors triggered from a scripting language or after specific exceptions handlers.\n"
                "- Always ensure that on programmers seat you have at minimum Asserts or Tooltips enabled when making direct rblxui API call! "
                "Otherwise it would severely hinder your ability to catch and correct mistakes!");
            RblxUI::Checkbox("io.ConfigErrorRecoveryEnableAssert", &io.ConfigErrorRecoveryEnableAssert);
            RblxUI::Checkbox("io.ConfigErrorRecoveryEnableDebugLog", &io.ConfigErrorRecoveryEnableDebugLog);
            RblxUI::Checkbox("io.ConfigErrorRecoveryEnableTooltip", &io.ConfigErrorRecoveryEnableTooltip);
            if (!io.ConfigErrorRecoveryEnableAssert && !io.ConfigErrorRecoveryEnableDebugLog && !io.ConfigErrorRecoveryEnableTooltip)
                io.ConfigErrorRecoveryEnableAssert = io.ConfigErrorRecoveryEnableDebugLog = io.ConfigErrorRecoveryEnableTooltip = true;

            // Also read: https://github.com/ocornut/rblxui/wiki/Debug-Tools
            RblxUI::SeparatorText("Debug");
            RblxUI::Checkbox("io.ConfigDebugIsDebuggerPresent", &io.ConfigDebugIsDebuggerPresent);
            RblxUI::SameLine(); HelpMarker("Enable various tools calling RBLXUI_DEBUG_BREAK().\n\nRequires a debugger being attached, otherwise RBLXUI_DEBUG_BREAK() options will appear to crash your application.");
            RblxUI::Checkbox("io.ConfigDebugHighlightIdConflicts", &io.ConfigDebugHighlightIdConflicts);
            RblxUI::SameLine(); HelpMarker("Highlight and show an error message when multiple items have conflicting identifiers.");
            RblxUI::BeginDisabled();
            RblxUI::Checkbox("io.ConfigDebugBeginReturnValueOnce", &io.ConfigDebugBeginReturnValueOnce);
            RblxUI::EndDisabled();
            RblxUI::SameLine(); HelpMarker("First calls to Begin()/BeginChild() will return false.\n\nTHIS OPTION IS DISABLED because it needs to be set at application boot-time to make sense. Showing the disabled option is a way to make this feature easier to discover.");
            RblxUI::Checkbox("io.ConfigDebugBeginReturnValueLoop", &io.ConfigDebugBeginReturnValueLoop);
            RblxUI::SameLine(); HelpMarker("Some calls to Begin()/BeginChild() will return false.\n\nWill cycle through window depths then repeat. Windows should be flickering while running.");
            RblxUI::Checkbox("io.ConfigDebugIgnoreFocusLoss", &io.ConfigDebugIgnoreFocusLoss);
            RblxUI::SameLine(); HelpMarker("Option to deactivate io.AddFocusEvent(false) handling. May facilitate interactions with a debugger when focus loss leads to clearing inputs data.");
            RblxUI::Checkbox("io.ConfigDebugIniSettings", &io.ConfigDebugIniSettings);
            RblxUI::SameLine(); HelpMarker("Option to save .ini data with extra comments (particularly helpful for Docking, but makes saving slower).");

            RblxUI::TreePop();
            RblxUI::Spacing();
        }

        if (RblxUI::TreeNode("Backend Flags"))
        {
            RBLXUI_DEMO_MARKER("Configuration/Backend Flags");
            HelpMarker(
                "Those flags are set by the backends (rblxui_impl_xxx files) to specify their capabilities.\n"
                "Here we expose them as read-only fields to avoid breaking interactions with your backend.");

            // Make a local copy to avoid modifying actual backend flags.
            // FIXME: Maybe we need a BeginReadonly() equivalent to keep label bright?
            RblxUI::BeginDisabled();
            RblxUI::CheckboxFlags("io.BackendFlags: HasGamepad",             &io.BackendFlags, RblxUIBackendFlags_HasGamepad);
            RblxUI::CheckboxFlags("io.BackendFlags: HasMouseCursors",        &io.BackendFlags, RblxUIBackendFlags_HasMouseCursors);
            RblxUI::CheckboxFlags("io.BackendFlags: HasSetMousePos",         &io.BackendFlags, RblxUIBackendFlags_HasSetMousePos);
            RblxUI::CheckboxFlags("io.BackendFlags: PlatformHasViewports",   &io.BackendFlags, RblxUIBackendFlags_PlatformHasViewports);
            RblxUI::CheckboxFlags("io.BackendFlags: HasMouseHoveredViewport",&io.BackendFlags, RblxUIBackendFlags_HasMouseHoveredViewport);
            RblxUI::CheckboxFlags("io.BackendFlags: HasParentViewport",      &io.BackendFlags, RblxUIBackendFlags_HasParentViewport);
            RblxUI::CheckboxFlags("io.BackendFlags: RendererHasVtxOffset",   &io.BackendFlags, RblxUIBackendFlags_RendererHasVtxOffset);
            RblxUI::CheckboxFlags("io.BackendFlags: RendererHasTextures",    &io.BackendFlags, RblxUIBackendFlags_RendererHasTextures);
            RblxUI::CheckboxFlags("io.BackendFlags: RendererHasViewports",   &io.BackendFlags, RblxUIBackendFlags_RendererHasViewports);
            RblxUI::EndDisabled();

            RblxUI::TreePop();
            RblxUI::Spacing();
        }

        if (RblxUI::TreeNode("Style, Fonts"))
        {
            RBLXUI_DEMO_MARKER("Configuration/Style, Fonts");
            RblxUI::Checkbox("Style Editor", &demo_data.ShowStyleEditor);
            RblxUI::SameLine();
            HelpMarker("The same contents can be accessed in 'Tools->Style Editor' or by calling the ShowStyleEditor() function.");
            RblxUI::TreePop();
            RblxUI::Spacing();
        }

        if (RblxUI::TreeNode("Capture/Logging"))
        {
            RBLXUI_DEMO_MARKER("Configuration/Capture, Logging");
            HelpMarker(
                "The logging API redirects all text output so you can easily capture the content of "
                "a window or a block. Tree nodes can be automatically expanded.\n"
                "Try opening any of the contents below in this window and then click one of the \"Log To\" button.");
            RblxUI::LogButtons();

            HelpMarker("You can also call RblxUI::LogText() to output directly to the log without a visual output.");
            if (RblxUI::Button("Copy \"Hello, world!\" to clipboard"))
            {
                RblxUI::LogToClipboard();
                RblxUI::LogText("Hello, world!");
                RblxUI::LogFinish();
            }
            RblxUI::TreePop();
        }
    }

    if (RblxUI::CollapsingHeader("Window options"))
    {
        RBLXUI_DEMO_MARKER("Window options");
        if (RblxUI::BeginTable("split", 3))
        {
            RblxUI::TableNextColumn(); RblxUI::Checkbox("No titlebar", &no_titlebar);
            RblxUI::TableNextColumn(); RblxUI::Checkbox("No scrollbar", &no_scrollbar);
            RblxUI::TableNextColumn(); RblxUI::Checkbox("No menu", &no_menu);
            RblxUI::TableNextColumn(); RblxUI::Checkbox("No move", &no_move);
            RblxUI::TableNextColumn(); RblxUI::Checkbox("No resize", &no_resize);
            RblxUI::TableNextColumn(); RblxUI::Checkbox("No collapse", &no_collapse);
            RblxUI::TableNextColumn(); RblxUI::Checkbox("No close", &no_close);
            RblxUI::TableNextColumn(); RblxUI::Checkbox("No nav", &no_nav);
            RblxUI::TableNextColumn(); RblxUI::Checkbox("No background", &no_background);
            RblxUI::TableNextColumn(); RblxUI::Checkbox("No bring to front", &no_bring_to_front);
            RblxUI::TableNextColumn(); RblxUI::Checkbox("No docking", &no_docking);
            RblxUI::TableNextColumn(); RblxUI::Checkbox("Unsaved document", &unsaved_document);
            RblxUI::EndTable();
        }
    }

    // All demo contents
    DemoWindowWidgets(&demo_data);
    DemoWindowLayout();
    DemoWindowPopups();
    DemoWindowTables();
    DemoWindowInputs();

    // End of ShowDemoWindow()
    RblxUI::PopItemWidth();
    RblxUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowMenuBar()
//-----------------------------------------------------------------------------

static void DemoWindowMenuBar(RblxUIDemoWindowData* demo_data)
{
    if (RblxUI::BeginMenuBar())
    {
        if (RblxUI::BeginMenu("Menu"))
        {
            RBLXUI_DEMO_MARKER("Menu/File");
            ShowExampleMenuFile();
            RblxUI::EndMenu();
        }
        if (RblxUI::BeginMenu("Examples"))
        {
            RBLXUI_DEMO_MARKER("Menu/Examples");
            RblxUI::MenuItem("Main menu bar", NULL, &demo_data->ShowMainMenuBar);

            RblxUI::SeparatorText("Mini apps");
            RblxUI::MenuItem("Assets Browser", NULL, &demo_data->ShowAppAssetsBrowser);
            RblxUI::MenuItem("Console", NULL, &demo_data->ShowAppConsole);
            RblxUI::MenuItem("Custom rendering", NULL, &demo_data->ShowAppCustomRendering);
            RblxUI::MenuItem("Documents", NULL, &demo_data->ShowAppDocuments);
            RblxUI::MenuItem("Dockspace", NULL, &demo_data->ShowAppDockSpace);
            RblxUI::MenuItem("Image Viewer", NULL, &demo_data->ShowAppImageViewer);
            RblxUI::MenuItem("Log", NULL, &demo_data->ShowAppLog);
            RblxUI::MenuItem("Property editor", NULL, &demo_data->ShowAppPropertyEditor);
            RblxUI::MenuItem("Simple layout", NULL, &demo_data->ShowAppLayout);
            RblxUI::MenuItem("Simple overlay", NULL, &demo_data->ShowAppSimpleOverlay);

            RblxUI::SeparatorText("Concepts");
            RblxUI::MenuItem("Auto-resizing window", NULL, &demo_data->ShowAppAutoResize);
            RblxUI::MenuItem("Constrained-resizing window", NULL, &demo_data->ShowAppConstrainedResize);
            RblxUI::MenuItem("Fullscreen window", NULL, &demo_data->ShowAppFullscreen);
            RblxUI::MenuItem("Long text display", NULL, &demo_data->ShowAppLongText);
            RblxUI::MenuItem("Manipulating window titles", NULL, &demo_data->ShowAppWindowTitles);

            RblxUI::EndMenu();
        }
        //if (RblxUI::MenuItem("MenuItem")) {} // You can also use MenuItem() inside a menu bar!
        if (RblxUI::BeginMenu("Tools"))
        {
            RBLXUI_DEMO_MARKER("Menu/Tools");
            RblxUIIO& io = RblxUI::GetIO();
#ifndef RBLXUI_DISABLE_DEBUG_TOOLS
            const bool has_debug_tools = true;
#else
            const bool has_debug_tools = false;
#endif
            RblxUI::MenuItem("Metrics/Debugger", NULL, &demo_data->ShowMetrics, has_debug_tools);
            if (RblxUI::BeginMenu("Debug Options"))
            {
                RblxUI::BeginDisabled(!has_debug_tools);
                RblxUI::Checkbox("Highlight ID Conflicts", &io.ConfigDebugHighlightIdConflicts);
                RblxUI::EndDisabled();
                RblxUI::Checkbox("Assert on error recovery", &io.ConfigErrorRecoveryEnableAssert);
                RblxUI::TextDisabled("(see Demo->Configuration for more)");
                RblxUI::EndMenu();
            }
            RblxUI::MenuItem("Debug Log", NULL, &demo_data->ShowDebugLog, has_debug_tools);
            RblxUI::MenuItem("ID Stack Tool", NULL, &demo_data->ShowIDStackTool, has_debug_tools);
            bool is_debugger_present = io.ConfigDebugIsDebuggerPresent;
            if (RblxUI::MenuItem("Item Picker", NULL, false, has_debug_tools))// && is_debugger_present))
                RblxUI::DebugStartItemPicker();
            if (!is_debugger_present)
                RblxUI::SetItemTooltip("Requires io.ConfigDebugIsDebuggerPresent=true to be set.\n\nWe otherwise disable some extra features to avoid casual users crashing the application.");
            RblxUI::MenuItem("Style Editor", NULL, &demo_data->ShowStyleEditor);
            RblxUI::MenuItem("About Dear RblxUI", NULL, &demo_data->ShowAbout);

            RblxUI::EndMenu();
        }
        RblxUI::EndMenuBar();
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
    RblxVector<ExampleTreeNode*>  Childs;
    int                         IndexInParent = 0;  // Maintaining this allows us to implement linear traversal more easily

    // Leaf Data
    bool                        HasData = false;    // All leaves have data
    bool                        DataMyBool = true;
    int                         DataMyInt = 128;
    RblxVec2                      DataMyVec2 = RblxVec2(0.0f, 3.141592f);
};

// Simple representation of struct metadata/serialization data.
// (this is a minimal version of what a typical advanced application may provide)
struct ExampleMemberInfo
{
    const char*     Name;       // Member name
    RblxUIDataType   DataType;   // Member type
    int             DataCount;  // Member count (1 when scalar)
    int             Offset;     // Offset inside parent structure
};

// Metadata description of ExampleTreeNode struct.
static const ExampleMemberInfo ExampleTreeNodeMemberInfos[]
{
    { "MyName",     RblxUIDataType_String,  1, offsetof(ExampleTreeNode, Name) },
    { "MyBool",     RblxUIDataType_Bool,    1, offsetof(ExampleTreeNode, DataMyBool) },
    { "MyInt",      RblxUIDataType_S32,     1, offsetof(ExampleTreeNode, DataMyInt) },
    { "MyVec2",     RblxUIDataType_Float,   2, offsetof(ExampleTreeNode, DataMyVec2) },
};

static ExampleTreeNode* ExampleTree_CreateNode(const char* name, int uid, ExampleTreeNode* parent)
{
    ExampleTreeNode* node = RBLXUI_NEW(ExampleTreeNode);
    snprintf(node->Name, RBLXUI_COUNTOF(node->Name), "%s", name);
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
    RBLXUI_DELETE(node);
}

// Create example tree data
// (warning: this can allocates MANY MANY more times than other code in all of Dear RblxUI + demo combined)
// (a real application managing one million nodes would likely store its tree data differently)
static ExampleTreeNode* ExampleTree_CreateDemoTree()
{
    //     20 root nodes ->    211 total nodes,   ~261 allocs.
    //   1000 root nodes ->   ~11K total nodes,   ~14K allocs.
    //  10000 root nodes ->  ~123K total nodes,  ~154K allocs.
    // 100000 root nodes -> ~1338K total nodes, ~1666K allocs.
    const int ROOT_ITEMS_COUNT = 20;

    static const char* category_names[] = { "Apple", "Banana", "Cherry", "Kiwi", "Mango", "Orange", "Pear", "Pineapple", "Strawberry", "Watermelon" };
    const int category_count = RBLXUI_COUNTOF(category_names);
    const size_t NAME_MAX_LEN = sizeof(ExampleTreeNode::Name);
    char name_buf[NAME_MAX_LEN];
    int uid = 0;
    ExampleTreeNode* node_L0 = ExampleTree_CreateNode("<ROOT>", ++uid, NULL);
    for (int idx_L0 = 0; idx_L0 < ROOT_ITEMS_COUNT; idx_L0++)
    {
        snprintf(name_buf, RBLXUI_COUNTOF(name_buf), "%s %d", category_names[idx_L0 / (ROOT_ITEMS_COUNT / category_count)], idx_L0 % (ROOT_ITEMS_COUNT / category_count));
        ExampleTreeNode* node_L1 = ExampleTree_CreateNode(name_buf, ++uid, node_L0);
        const int number_of_childs = (int)strlen(node_L1->Name);
        for (int idx_L1 = 0; idx_L1 < number_of_childs; idx_L1++)
        {
            snprintf(name_buf, RBLXUI_COUNTOF(name_buf), "Child %d", idx_L1);
            ExampleTreeNode* node_L2 = ExampleTree_CreateNode(name_buf, ++uid, node_L1);
            node_L2->HasData = true;
            if (idx_L1 == 0)
            {
                snprintf(name_buf, RBLXUI_COUNTOF(name_buf), "Sub-child %d", 0);
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
    RblxU32   ImageBgColor = RBLXUI_COL32(100, 100, 100, 255);
    RblxU32   GridColor = RBLXUI_COL32(255, 255, 255, 100);
    bool    GridEnabled = true;
    bool    ViewReset = true;
    RblxVec2  ViewOffset; // in image space
    float   Zoom = 10.0f;
    float   ZoomMin = 1.0f;
    float   ZoomMax = 10000.0f;
};

static void ExampleImageViewer_DrawOptions(ExampleImageViewerData* data)
{
    RblxUI::SetNextItemShortcut(RblxUIKey_G, RblxUIInputFlags_Tooltip); // | RblxUIInputFlags_RouteGlobal
    RblxUI::Checkbox("Grid", &data->GridEnabled);
    RblxUI::SameLine();
    RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 10.0f);
    float zoom_100 = data->Zoom * 100.0f;
    if (RblxUI::DragFloat("##Zoom", &zoom_100, 5.0f, data->ZoomMin * 100.0f, data->ZoomMax * 100.0f, "%.0f%%", RblxUISliderFlags_AlwaysClamp))
        data->Zoom = zoom_100 / 100.0f;
}

static void ExampleImageViewer_DrawCanvas(ExampleImageViewerData* data, RblxVec2 canvas_size, RblxTextureRef image_tex_ref, int image_w, int image_h)
{
    RblxUIIO& io = RblxUI::GetIO();
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    RblxDrawList* draw_list = RblxUI::GetWindowDrawList();
    RBLXUI_ASSERT(canvas_size.x >= 0.0f && canvas_size.y >= 0.0f);

    // Layout canvas
    RblxUI::InvisibleButton("##Canvas", canvas_size);
    RblxVec2 canvas_min = RblxUI::GetItemRectMin();
    RblxVec2 canvas_max = RblxUI::GetItemRectMax();

    if (data->ViewReset)
        data->ViewOffset = RblxVec2((canvas_size.x * 0.5f / data->Zoom) - 0.5f, (canvas_size.y * 0.5f / data->Zoom) - 0.5f); // Add half a pixel padding
    data->ViewReset = false;

    // Handle inputs
    if (RblxUI::SetItemKeyOwner(RblxUIKey_MouseWheelY))
        if (io.MouseWheel != 0.0f)
            data->Zoom = RBLXUI_CLAMP(data->Zoom * (1.0f + io.MouseWheel * 0.10f), data->ZoomMin, data->ZoomMax);
    float zoom = data->Zoom; // (float)(int)ViewZoom;
    if (RblxUI::IsItemActive() && RblxUI::IsMouseDragging(0))
    {
        data->ViewOffset.x -= io.MouseDelta.x / zoom;
        data->ViewOffset.y -= io.MouseDelta.y / zoom;
    }

    // Display image
    RblxVec2 image_min, image_max;
    image_min.x = (float)(int)((canvas_min.x - (data->ViewOffset.x * zoom)) + (canvas_size.x * 0.5f));
    image_min.y = (float)(int)((canvas_min.y - (data->ViewOffset.y * zoom)) + (canvas_size.y * 0.5f));
    image_max.x = (float)(int)(image_min.x + image_w * zoom);
    image_max.y = (float)(int)(image_min.y + image_h * zoom);
    draw_list->AddRect(RblxVec2(canvas_min.x - 1.0f, canvas_min.y - 1.0f), RblxVec2(canvas_max.x + 1.0f, canvas_max.y + 1.0f), RBLXUI_COL32(255, 255, 255, 255));
    draw_list->PushClipRect(canvas_min, canvas_max, true);
    draw_list->AddRectFilled(image_min, image_max, data->ImageBgColor);
    if (platform_io.DrawCallback_SetSamplerNearest != NULL)
        draw_list->AddCallback(platform_io.DrawCallback_SetSamplerNearest);
    draw_list->AddImage(image_tex_ref, image_min, image_max);
    if (platform_io.DrawCallback_SetSamplerLinear != NULL)
        draw_list->AddCallback(RblxUI::GetPlatformIO().DrawCallback_SetSamplerLinear);

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
    if (RblxUI::TreeNode("Basic"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Basic");
        RblxUI::SeparatorText("General");

        RBLXUI_DEMO_MARKER("Widgets/Basic/Button");
        static int clicked = 0;
        if (RblxUI::Button("Button"))
            clicked++;
        if (clicked & 1)
        {
            RblxUI::SameLine();
            RblxUI::Text("Thanks for clicking me!");
        }

        RBLXUI_DEMO_MARKER("Widgets/Basic/Checkbox");
        static bool check = true;
        RblxUI::Checkbox("checkbox", &check);

        RBLXUI_DEMO_MARKER("Widgets/Basic/RadioButton");
        static int e = 0;
        RblxUI::RadioButton("radio a", &e, 0); RblxUI::SameLine();
        RblxUI::RadioButton("radio b", &e, 1); RblxUI::SameLine();
        RblxUI::RadioButton("radio c", &e, 2);

        RblxUI::AlignTextToFramePadding();
        RblxUI::TextLinkOpenURL("Hyperlink", "https://github.com/ocornut/rblxui/wiki/Error-Handling");

        // Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and changing style.
        RBLXUI_DEMO_MARKER("Widgets/Basic/Buttons (Colored)");
        for (int i = 0; i < 7; i++)
        {
            if (i > 0)
                RblxUI::SameLine();
            RblxUI::PushID(i);
            RblxUI::PushStyleColor(RblxUICol_Button, (RblxVec4)RblxColor::HSV(i / 7.0f, 0.6f, 0.6f));
            RblxUI::PushStyleColor(RblxUICol_ButtonHovered, (RblxVec4)RblxColor::HSV(i / 7.0f, 0.7f, 0.7f));
            RblxUI::PushStyleColor(RblxUICol_ButtonActive, (RblxVec4)RblxColor::HSV(i / 7.0f, 0.8f, 0.8f));
            RblxUI::Button("Click");
            RblxUI::PopStyleColor(3);
            RblxUI::PopID();
        }

        // Use AlignTextToFramePadding() to align text baseline to the baseline of framed widgets elements
        // (otherwise a Text+SameLine+Button sequence will have the text a little too high by default!)
        // See 'Demo->Layout->Text Baseline Alignment' for details.
        RblxUI::AlignTextToFramePadding();
        RblxUI::Text("Hold to repeat:");
        RblxUI::SameLine();

        // Arrow buttons with Repeater
        RBLXUI_DEMO_MARKER("Widgets/Basic/Buttons (Repeating)");
        static int counter = 0;
        float spacing = RblxUI::GetStyle().ItemInnerSpacing.x;
        RblxUI::PushItemFlag(RblxUIItemFlags_ButtonRepeat, true);
        if (RblxUI::ArrowButton("##left", RblxUIDir_Left)) { counter--; }
        RblxUI::SameLine(0.0f, spacing);
        if (RblxUI::ArrowButton("##right", RblxUIDir_Right)) { counter++; }
        RblxUI::PopItemFlag();
        RblxUI::SameLine();
        RblxUI::Text("%d", counter);

        RblxUI::Button("Tooltip");
        RblxUI::SetItemTooltip("I am a tooltip");

        RblxUI::LabelText("label", "Value");

        RblxUI::SeparatorText("Inputs");

        {
            // If you want to use InputText() with std::string or any custom dynamic string type:
            // - For std::string: use the wrapper in misc/cpp/rblxui_stdlib.h/.cpp
            // - Otherwise, see the 'Dear RblxUI Demo->Widgets->Text Input->Resize Callback' for using RblxUIInputTextFlags_CallbackResize.
            RBLXUI_DEMO_MARKER("Widgets/Basic/InputText");
            static char str0[128] = "Hello, world!";
            RblxUI::InputText("input text", str0, RBLXUI_COUNTOF(str0));
            RblxUI::SameLine(); HelpMarker(
                "USER:\n"
                "Hold Shift or use mouse to select text.\n"
                "Ctrl+Left/Right to word jump.\n"
                "Ctrl+A or Double-Click to select all.\n"
                "Ctrl+X,Ctrl+C,Ctrl+V for clipboard.\n"
                "Ctrl+Z to undo, Ctrl+Y/Ctrl+Shift+Z to redo.\n"
                "Escape to revert.\n\n"
                "PROGRAMMER:\n"
                "You can use the RblxUIInputTextFlags_CallbackResize facility if you need to wire InputText() "
                "to a dynamic string type. See misc/cpp/rblxui_stdlib.h for an example (this is not demonstrated "
                "in rblxui_demo.cpp).");

            static char str1[128] = "";
            RblxUI::InputTextWithHint("input text (w/ hint)", "enter text here", str1, RBLXUI_COUNTOF(str1));

            RBLXUI_DEMO_MARKER("Widgets/Basic/InputInt, InputFloat");
            static int i0 = 123;
            RblxUI::InputInt("input int", &i0);

            static float f0 = 0.001f;
            RblxUI::InputFloat("input float", &f0, 0.01f, 1.0f, "%.3f");

            static double d0 = 999999.00000001;
            RblxUI::InputDouble("input double", &d0, 0.01f, 1.0f, "%.8f");

            static float f1 = 1.e10f;
            RblxUI::InputFloat("input scientific", &f1, 0.0f, 0.0f, "%e");
            RblxUI::SameLine(); HelpMarker(
                "You can input value using the scientific notation,\n"
                "  e.g. \"1e+8\" becomes \"100000000\".");

            static float vec4a[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
            RblxUI::InputFloat3("input float3", vec4a);
        }

        RblxUI::SeparatorText("Drags");

        {
            RBLXUI_DEMO_MARKER("Widgets/Basic/DragInt, DragFloat");
            static int i1 = 50, i2 = 42, i3 = 128;
            RblxUI::DragInt("drag int", &i1, 1);
            RblxUI::SameLine(); HelpMarker(
                "Click and drag to edit value.\n"
                "Hold Shift/Alt for faster/slower edit.\n"
                "Double-Click or Ctrl+Click to input value.");
            RblxUI::DragInt("drag int 0..100", &i2, 1, 0, 100, "%d%%", RblxUISliderFlags_AlwaysClamp);
            RblxUI::DragInt("drag int wrap 100..200", &i3, 1, 100, 200, "%d", RblxUISliderFlags_WrapAround);

            static float f1 = 1.00f, f2 = 0.0067f;
            RblxUI::DragFloat("drag float", &f1, 0.005f);
            RblxUI::DragFloat("drag small float", &f2, 0.0001f, 0.0f, 0.0f, "%.06f ns");
            //RblxUI::DragFloat("drag wrap -1..1", &f3, 0.005f, -1.0f, 1.0f, NULL, RblxUISliderFlags_WrapAround);
        }

        RblxUI::SeparatorText("Sliders");

        {
            RBLXUI_DEMO_MARKER("Widgets/Basic/SliderInt, SliderFloat");
            static int i1 = 0;
            RblxUI::SliderInt("slider int", &i1, -1, 3);
            RblxUI::SameLine(); HelpMarker("Ctrl+Click to input value.");

            static float f1 = 0.123f, f2 = 0.0f;
            RblxUI::SliderFloat("slider float", &f1, 0.0f, 1.0f, "ratio = %.3f");
            RblxUI::SliderFloat("slider float (log)", &f2, -10.0f, 10.0f, "%.4f", RblxUISliderFlags_Logarithmic);

            RBLXUI_DEMO_MARKER("Widgets/Basic/SliderAngle");
            static float angle = 0.0f;
            RblxUI::SliderAngle("slider angle", &angle);

            // Using the format string to display a name instead of an integer.
            // Here we completely omit '%d' from the format string, so it'll only display a name.
            // This technique can also be used with DragInt().
            RBLXUI_DEMO_MARKER("Widgets/Basic/Slider (enum)");
            enum Element { Element_Fire, Element_Earth, Element_Air, Element_Water, Element_COUNT };
            static int elem = Element_Fire;
            const char* elems_names[Element_COUNT] = { "Fire", "Earth", "Air", "Water" };
            const char* elem_name = (elem >= 0 && elem < Element_COUNT) ? elems_names[elem] : "Unknown";
            RblxUI::SliderInt("slider enum", &elem, 0, Element_COUNT - 1, elem_name); // Use RblxUISliderFlags_NoInput flag to disable Ctrl+Click here.
            RblxUI::SameLine(); HelpMarker("Using the format string parameter to display a name instead of the underlying integer.");
        }

        RblxUI::SeparatorText("Selectors/Pickers");

        {
            RBLXUI_DEMO_MARKER("Widgets/Basic/ColorEdit3, ColorEdit4");
            static float col1[3] = { 1.0f, 0.0f, 0.2f };
            static float col2[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
            RblxUI::ColorEdit3("color 1", col1);
            RblxUI::SameLine(); HelpMarker(
                "Click on the color square to open a color picker.\n"
                "Click and hold to use drag and drop.\n"
                "Right-Click on the color square to show options.\n"
                "Ctrl+Click on individual component to input value.\n");

            RblxUI::ColorEdit4("color 2", col2);
        }

        {
            // Using the _simplified_ one-liner Combo() api here
            // See "Combo" section for examples of how to use the more flexible BeginCombo()/EndCombo() api.
            RBLXUI_DEMO_MARKER("Widgets/Basic/Combo");
            const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIIIIII", "JJJJ", "KKKKKKK" };
            static int item_current = 0;
            RblxUI::Combo("combo", &item_current, items, RBLXUI_COUNTOF(items));
            RblxUI::SameLine(); HelpMarker(
                "Using the simplified one-liner Combo API here.\n"
                "Refer to the \"Combo\" section below for an explanation of how to use the more flexible and general BeginCombo/EndCombo API.");
        }

        {
            // Using the _simplified_ one-liner ListBox() api here
            // See "List boxes" section for examples of how to use the more flexible BeginListBox()/EndListBox() api.
            RBLXUI_DEMO_MARKER("Widgets/Basic/ListBox");
            const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi", "Mango", "Orange", "Pineapple", "Strawberry", "Watermelon" };
            static int item_current = 1;
            RblxUI::ListBox("listbox", &item_current, items, RBLXUI_COUNTOF(items), 4);
            RblxUI::SameLine(); HelpMarker(
                "Using the simplified one-liner ListBox API here.\n"
                "Refer to the \"List boxes\" section below for an explanation of how to use the more flexible and general BeginListBox/EndListBox API.");
        }

        // Testing RblxUIOnceUponAFrame helper.
        //static RblxUIOnceUponAFrame once;
        //for (int i = 0; i < 5; i++)
        //    if (once)
        //        RblxUI::Text("This will be displayed only once.");

        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsBullets()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsBullets()
{
    if (RblxUI::TreeNode("Bullets"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Bullets");
        RblxUI::BulletText("Bullet point 1");
        RblxUI::BulletText("Bullet point 2\nOn multiple lines");
        if (RblxUI::TreeNode("Tree node"))
        {
            RblxUI::BulletText("Another bullet point");
            RblxUI::TreePop();
        }
        RblxUI::Bullet(); RblxUI::Text("Bullet point 3 (two calls)");
        RblxUI::Bullet(); RblxUI::SmallButton("Button");
        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsCollapsingHeaders()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsCollapsingHeaders()
{
    if (RblxUI::TreeNode("Collapsing Headers"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Collapsing Headers");
        static bool closable_group = true;
        RblxUI::Checkbox("Show 2nd header", &closable_group);
        if (RblxUI::CollapsingHeader("Header", RblxUITreeNodeFlags_None))
        {
            RblxUI::Text("IsItemHovered: %d", RblxUI::IsItemHovered());
            for (int i = 0; i < 5; i++)
                RblxUI::Text("Some content %d", i);
        }
        if (RblxUI::CollapsingHeader("Header with a close button", &closable_group))
        {
            RblxUI::Text("IsItemHovered: %d", RblxUI::IsItemHovered());
            for (int i = 0; i < 5; i++)
                RblxUI::Text("More content %d", i);
        }
        /*
        if (RblxUI::CollapsingHeader("Header with a bullet", RblxUITreeNodeFlags_Bullet))
            RblxUI::Text("IsItemHovered: %d", RblxUI::IsItemHovered());
        */
        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsColorAndPickers()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsColorAndPickers()
{
    if (RblxUI::TreeNode("Color/Picker Widgets"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Color");
        static RblxVec4 color = RblxVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
        static RblxUIColorEditFlags base_flags = RblxUIColorEditFlags_None;

        RblxUI::SeparatorText("Options");
        RblxUI::CheckboxFlags("RblxUIColorEditFlags_NoAlpha", &base_flags, RblxUIColorEditFlags_NoAlpha);
        RblxUI::CheckboxFlags("RblxUIColorEditFlags_AlphaOpaque", &base_flags, RblxUIColorEditFlags_AlphaOpaque);
        RblxUI::CheckboxFlags("RblxUIColorEditFlags_AlphaNoBg", &base_flags, RblxUIColorEditFlags_AlphaNoBg);
        RblxUI::CheckboxFlags("RblxUIColorEditFlags_AlphaPreviewHalf", &base_flags, RblxUIColorEditFlags_AlphaPreviewHalf);
        RblxUI::CheckboxFlags("RblxUIColorEditFlags_NoOptions", &base_flags, RblxUIColorEditFlags_NoOptions); RblxUI::SameLine(); HelpMarker("Right-click on the individual color widget to show options.");
        RblxUI::CheckboxFlags("RblxUIColorEditFlags_NoDragDrop", &base_flags, RblxUIColorEditFlags_NoDragDrop);
        RblxUI::CheckboxFlags("RblxUIColorEditFlags_NoColorMarkers", &base_flags, RblxUIColorEditFlags_NoColorMarkers);
        RblxUI::CheckboxFlags("RblxUIColorEditFlags_HDR", &base_flags, RblxUIColorEditFlags_HDR); RblxUI::SameLine(); HelpMarker("Currently all this does is to lift the 0..1 limits on dragging widgets.");

        RBLXUI_DEMO_MARKER("Widgets/Color/ColorEdit");
        RblxUI::SeparatorText("Inline color editor");
        RblxUI::Text("Color widget:");
        RblxUI::SameLine(); HelpMarker(
            "Click on the color square to open a color picker.\n"
            "Ctrl+Click on individual component to input value.\n");
        RblxUI::ColorEdit3("MyColor##1", (float*)&color, base_flags);

        RBLXUI_DEMO_MARKER("Widgets/Color/ColorEdit (HSV, with Alpha)");
        RblxUI::Text("Color widget HSV with Alpha:");
        RblxUI::ColorEdit4("MyColor##2", (float*)&color, RblxUIColorEditFlags_DisplayHSV | base_flags);

        RBLXUI_DEMO_MARKER("Widgets/Color/ColorEdit (float display)");
        RblxUI::Text("Color widget with Float Display:");
        RblxUI::ColorEdit4("MyColor##2f", (float*)&color, RblxUIColorEditFlags_Float | base_flags);

        RBLXUI_DEMO_MARKER("Widgets/Color/ColorButton (with Picker)");
        RblxUI::Text("Color button with Picker:");
        RblxUI::SameLine(); HelpMarker(
            "With the RblxUIColorEditFlags_NoInputs flag you can hide all the slider/text inputs.\n"
            "With the RblxUIColorEditFlags_NoLabel flag you can pass a non-empty label which will only "
            "be used for the tooltip and picker popup.");
        RblxUI::ColorEdit4("MyColor##3", (float*)&color, RblxUIColorEditFlags_NoInputs | RblxUIColorEditFlags_NoLabel | base_flags);

        RBLXUI_DEMO_MARKER("Widgets/Color/ColorButton (with custom Picker popup)");
        RblxUI::Text("Color button with Custom Picker Popup:");

        // Generate a default palette. The palette will persist and can be edited.
        static bool saved_palette_init = true;
        static RblxVec4 saved_palette[32] = {};
        if (saved_palette_init)
        {
            for (int n = 0; n < RBLXUI_COUNTOF(saved_palette); n++)
            {
                RblxUI::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f,
                    saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
                saved_palette[n].w = 1.0f; // Alpha
            }
            saved_palette_init = false;
        }

        static RblxVec4 backup_color;
        bool open_popup = RblxUI::ColorButton("MyColor##3b", color, base_flags);
        RblxUI::SameLine(0, RblxUI::GetStyle().ItemInnerSpacing.x);
        open_popup |= RblxUI::Button("Palette");
        if (open_popup)
        {
            RblxUI::OpenPopup("mypicker");
            backup_color = color;
        }
        if (RblxUI::BeginPopup("mypicker"))
        {
            RblxUI::Text("MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!");
            RblxUI::Separator();
            RblxUI::ColorPicker4("##picker", (float*)&color, base_flags | RblxUIColorEditFlags_NoSidePreview | RblxUIColorEditFlags_NoSmallPreview);
            RblxUI::SameLine();

            RblxUI::BeginGroup(); // Lock X position
            RblxUI::Text("Current");
            RblxUI::ColorButton("##current", color, RblxUIColorEditFlags_NoPicker | RblxUIColorEditFlags_AlphaPreviewHalf, RblxVec2(60, 40));
            RblxUI::Text("Previous");
            if (RblxUI::ColorButton("##previous", backup_color, RblxUIColorEditFlags_NoPicker | RblxUIColorEditFlags_AlphaPreviewHalf, RblxVec2(60, 40)))
                color = backup_color;
            RblxUI::Separator();
            RblxUI::Text("Palette");
            for (int n = 0; n < RBLXUI_COUNTOF(saved_palette); n++)
            {
                RblxUI::PushID(n);
                if ((n % 8) != 0)
                    RblxUI::SameLine(0.0f, RblxUI::GetStyle().ItemSpacing.y);

                RblxUIColorEditFlags palette_button_flags = RblxUIColorEditFlags_NoAlpha | RblxUIColorEditFlags_NoPicker | RblxUIColorEditFlags_NoTooltip;
                if (RblxUI::ColorButton("##palette", saved_palette[n], palette_button_flags, RblxVec2(20, 20)))
                    color = RblxVec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, color.w); // Preserve alpha!

                // Allow user to drop colors into each palette entry. Note that ColorButton() is already a
                // drag source by default, unless specifying the RblxUIColorEditFlags_NoDragDrop flag.
                if (RblxUI::BeginDragDropTarget())
                {
                    if (const RblxUIPayload* payload = RblxUI::AcceptDragDropPayload(RBLXUI_PAYLOAD_TYPE_COLOR_3F))
                        memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 3);
                    if (const RblxUIPayload* payload = RblxUI::AcceptDragDropPayload(RBLXUI_PAYLOAD_TYPE_COLOR_4F))
                        memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 4);
                    RblxUI::EndDragDropTarget();
                }

                RblxUI::PopID();
            }
            RblxUI::EndGroup();
            RblxUI::EndPopup();
        }

        RBLXUI_DEMO_MARKER("Widgets/Color/ColorButton (simple)");
        RblxUI::Text("Color button only:");
        static bool no_border = false;
        RblxUI::Checkbox("RblxUIColorEditFlags_NoBorder", &no_border);
        RblxUI::ColorButton("MyColor##3c", *(RblxVec4*)&color, base_flags | (no_border ? RblxUIColorEditFlags_NoBorder : 0), RblxVec2(80, 80));

        RBLXUI_DEMO_MARKER("Widgets/Color/ColorPicker");
        RblxUI::SeparatorText("Color picker");

        static bool ref_color = false;
        static RblxVec4 ref_color_v(1.0f, 0.0f, 1.0f, 0.5f);
        static int picker_mode = 0;
        static int display_mode = 0;
        static RblxUIColorEditFlags color_picker_flags = RblxUIColorEditFlags_AlphaBar;

        RblxUI::PushID("Color picker");
        RblxUI::CheckboxFlags("RblxUIColorEditFlags_NoAlpha", &color_picker_flags, RblxUIColorEditFlags_NoAlpha);
        RblxUI::CheckboxFlags("RblxUIColorEditFlags_AlphaBar", &color_picker_flags, RblxUIColorEditFlags_AlphaBar);
        RblxUI::CheckboxFlags("RblxUIColorEditFlags_NoSidePreview", &color_picker_flags, RblxUIColorEditFlags_NoSidePreview);
        if (color_picker_flags & RblxUIColorEditFlags_NoSidePreview)
        {
            RblxUI::SameLine();
            RblxUI::Checkbox("With Ref Color", &ref_color);
            if (ref_color)
            {
                RblxUI::SameLine();
                RblxUI::ColorEdit4("##RefColor", &ref_color_v.x, RblxUIColorEditFlags_NoInputs | base_flags);
            }
        }

        RblxUI::Combo("Picker Mode", &picker_mode, "Auto/Current\0RblxUIColorEditFlags_PickerHueBar\0RblxUIColorEditFlags_PickerHueWheel\0");
        RblxUI::SameLine(); HelpMarker("When not specified explicitly, user can right-click the picker to change mode.");

        RblxUI::Combo("Display Mode", &display_mode, "Auto/Current\0RblxUIColorEditFlags_NoInputs\0RblxUIColorEditFlags_DisplayRGB\0RblxUIColorEditFlags_DisplayHSV\0RblxUIColorEditFlags_DisplayHex\0");
        RblxUI::SameLine(); HelpMarker(
            "ColorEdit defaults to displaying RGB inputs if you don't specify a display mode, "
            "but the user can change it with a right-click on those inputs.\n\nColorPicker defaults to displaying RGB+HSV+Hex "
            "if you don't specify a display mode.\n\nYou can change the defaults using SetColorEditOptions().");

        RblxUIColorEditFlags flags = base_flags | color_picker_flags;
        if (picker_mode == 1)  flags |= RblxUIColorEditFlags_PickerHueBar;
        if (picker_mode == 2)  flags |= RblxUIColorEditFlags_PickerHueWheel;
        if (display_mode == 1) flags |= RblxUIColorEditFlags_NoInputs;       // Disable all RGB/HSV/Hex displays
        if (display_mode == 2) flags |= RblxUIColorEditFlags_DisplayRGB;     // Override display mode
        if (display_mode == 3) flags |= RblxUIColorEditFlags_DisplayHSV;
        if (display_mode == 4) flags |= RblxUIColorEditFlags_DisplayHex;
        RblxUI::ColorPicker4("MyColor##4", (float*)&color, flags, ref_color ? &ref_color_v.x : NULL);

        RblxUI::Text("Set defaults in code:");
        RblxUI::SameLine(); HelpMarker(
            "SetColorEditOptions() is designed to allow you to set boot-time default.\n"
            "We don't have Push/Pop functions because you can force options on a per-widget basis if needed, "
            "and the user can change non-forced ones with the options menu.\nWe don't have a getter to avoid "
            "encouraging you to persistently save values that aren't forward-compatible.");
        if (RblxUI::Button("Default: Uint8 + HSV + Hue Bar"))
            RblxUI::SetColorEditOptions(RblxUIColorEditFlags_Uint8 | RblxUIColorEditFlags_DisplayHSV | RblxUIColorEditFlags_PickerHueBar);
        if (RblxUI::Button("Default: Float + HDR + Hue Wheel"))
            RblxUI::SetColorEditOptions(RblxUIColorEditFlags_Float | RblxUIColorEditFlags_HDR | RblxUIColorEditFlags_PickerHueWheel);

        // Always display a small version of both types of pickers
        // (that's in order to make it more visible in the demo to people who are skimming quickly through it)
        RblxUI::Text("Both types:");
        float w = (RblxUI::GetContentRegionAvail().x - RblxUI::GetStyle().ItemSpacing.y) * 0.40f;
        RblxUI::SetNextItemWidth(w);
        RblxUI::ColorPicker3("##MyColor##5", (float*)&color, RblxUIColorEditFlags_PickerHueBar | RblxUIColorEditFlags_NoSidePreview | RblxUIColorEditFlags_NoInputs | RblxUIColorEditFlags_NoAlpha);
        RblxUI::SameLine();
        RblxUI::SetNextItemWidth(w);
        RblxUI::ColorPicker3("##MyColor##6", (float*)&color, RblxUIColorEditFlags_PickerHueWheel | RblxUIColorEditFlags_NoSidePreview | RblxUIColorEditFlags_NoInputs | RblxUIColorEditFlags_NoAlpha);
        RblxUI::PopID();

        // HSV encoded support (to avoid RGB<>HSV round trips and singularities when S==0 or V==0)
        static RblxVec4 color_hsv(0.23f, 1.0f, 1.0f, 1.0f); // Stored as HSV!
        RblxUI::Spacing();
        RblxUI::Text("HSV encoded colors");
        RblxUI::SameLine(); HelpMarker(
            "By default, colors are given to ColorEdit and ColorPicker in RGB, but RblxUIColorEditFlags_InputHSV "
            "allows you to store colors as HSV and pass them to ColorEdit and ColorPicker as HSV. This comes with the "
            "added benefit that you can manipulate hue values with the picker even when saturation or value are zero.");
        RblxUI::Text("Color widget with InputHSV:");
        RblxUI::ColorEdit4("HSV shown as RGB##1", (float*)&color_hsv, RblxUIColorEditFlags_DisplayRGB | RblxUIColorEditFlags_InputHSV | RblxUIColorEditFlags_Float);
        RblxUI::ColorEdit4("HSV shown as HSV##1", (float*)&color_hsv, RblxUIColorEditFlags_DisplayHSV | RblxUIColorEditFlags_InputHSV | RblxUIColorEditFlags_Float);
        RblxUI::DragFloat4("Raw HSV values", (float*)&color_hsv, 0.01f, 0.0f, 1.0f);

        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsComboBoxes()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsComboBoxes()
{
    if (RblxUI::TreeNode("Combo"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Combo");
        // Combo Boxes are also called "Dropdown" in other systems
        // Expose flags as checkbox for the demo
        static RblxUIComboFlags flags = 0;
        RblxUI::CheckboxFlags("RblxUIComboFlags_PopupAlignLeft", &flags, RblxUIComboFlags_PopupAlignLeft);
        RblxUI::SameLine(); HelpMarker("Only makes a difference if the popup is larger than the combo");
        if (RblxUI::CheckboxFlags("RblxUIComboFlags_NoArrowButton", &flags, RblxUIComboFlags_NoArrowButton))
            flags &= ~RblxUIComboFlags_NoPreview;     // Clear incompatible flags
        if (RblxUI::CheckboxFlags("RblxUIComboFlags_NoPreview", &flags, RblxUIComboFlags_NoPreview))
            flags &= ~(RblxUIComboFlags_NoArrowButton | RblxUIComboFlags_WidthFitPreview); // Clear incompatible flags
        if (RblxUI::CheckboxFlags("RblxUIComboFlags_WidthFitPreview", &flags, RblxUIComboFlags_WidthFitPreview))
            flags &= ~RblxUIComboFlags_NoPreview;

        // Override default popup height
        if (RblxUI::CheckboxFlags("RblxUIComboFlags_HeightSmall", &flags, RblxUIComboFlags_HeightSmall))
            flags &= ~(RblxUIComboFlags_HeightMask_ & ~RblxUIComboFlags_HeightSmall);
        if (RblxUI::CheckboxFlags("RblxUIComboFlags_HeightRegular", &flags, RblxUIComboFlags_HeightRegular))
            flags &= ~(RblxUIComboFlags_HeightMask_ & ~RblxUIComboFlags_HeightRegular);
        if (RblxUI::CheckboxFlags("RblxUIComboFlags_HeightLargest", &flags, RblxUIComboFlags_HeightLargest))
            flags &= ~(RblxUIComboFlags_HeightMask_ & ~RblxUIComboFlags_HeightLargest);

        // Using the generic BeginCombo() API, you have full control over how to display the combo contents.
        // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
        // stored in the object itself, etc.)
        const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
        static int item_selected_idx = 0; // Here we store our selection data as an index.

        // Pass in the preview value visible before opening the combo (it could technically be different contents or not pulled from items[])
        const char* combo_preview_value = items[item_selected_idx];
        if (RblxUI::BeginCombo("combo 1", combo_preview_value, flags))
        {
            for (int n = 0; n < RBLXUI_COUNTOF(items); n++)
            {
                const bool is_selected = (item_selected_idx == n);
                if (RblxUI::Selectable(items[n], is_selected))
                    item_selected_idx = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    RblxUI::SetItemDefaultFocus();
            }
            RblxUI::EndCombo();
        }

        // Show case embedding a filter using a simple trick: displaying the filter inside combo contents.
        // See https://github.com/ocornut/rblxui/issues/718 for advanced/esoteric alternatives.
        if (RblxUI::BeginCombo("combo 2 (w/ filter)", combo_preview_value, flags))
        {
            static RblxUITextFilter filter;
            if (RblxUI::IsWindowAppearing())
            {
                RblxUI::SetKeyboardFocusHere();
                filter.Clear();
            }
            RblxUI::SetNextItemShortcut(RblxUIMod_Ctrl | RblxUIKey_F);
            filter.Draw("##Filter", -FLT_MIN);

            for (int n = 0; n < RBLXUI_COUNTOF(items); n++)
            {
                const bool is_selected = (item_selected_idx == n);
                if (filter.PassFilter(items[n]))
                    if (RblxUI::Selectable(items[n], is_selected))
                        item_selected_idx = n;
            }
            RblxUI::EndCombo();
        }

        RblxUI::Spacing();
        RblxUI::SeparatorText("One-liner variants");
        HelpMarker("The Combo() function is not greatly useful apart from cases were you want to embed all options in a single strings.\nFlags above don't apply to this section.");

        // Simplified one-liner Combo() API, using values packed in a single constant string
        // This is a convenience for when the selection set is small and known at compile-time.
        static int item_current_2 = 0;
        RblxUI::Combo("combo 3 (one-liner)", &item_current_2, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");

        // Simplified one-liner Combo() using an array of const char*
        // This is not very useful (may obsolete): prefer using BeginCombo()/EndCombo() for full control.
        static int item_current_3 = -1; // If the selection isn't within 0..count, Combo won't display a preview
        RblxUI::Combo("combo 4 (array)", &item_current_3, items, RBLXUI_COUNTOF(items));

        // Simplified one-liner Combo() using an accessor function
        static int item_current_4 = 0;
        RblxUI::Combo("combo 5 (function)", &item_current_4, [](void* data, int n) { return ((const char**)data)[n]; }, items, RBLXUI_COUNTOF(items));

        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsDataTypes()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsDataTypes()
{
    if (RblxUI::TreeNode("Data Types"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Data Types");
        // DragScalar/InputScalar/SliderScalar functions allow various data types
        // - signed/unsigned
        // - 8/16/32/64-bits
        // - integer/float/double
        // To avoid polluting the public API with all possible combinations, we use the RblxUIDataType enum
        // to pass the type, and passing all arguments by pointer.
        // This is the reason the test code below creates local variables to hold "zero" "one" etc. for each type.
        // In practice, if you frequently use a given type that is not covered by the normal API entry points,
        // you can wrap it yourself inside a 1 line function which can take typed argument as value instead of void*,
        // and then pass their address to the generic function. For example:
        //   bool MySliderU64(const char *label, u64* value, u64 min = 0, u64 max = 0, const char* format = "%lld")
        //   {
        //      return SliderScalar(label, RblxUIDataType_U64, value, &min, &max, format);
        //   }

        // Setup limits (as helper variables so we can take their address, as explained above)
        // Note: SliderScalar() functions have a maximum usable range of half the natural type maximum, hence the /2.
        #ifndef LLONG_MIN
        RblxS64 LLONG_MIN = -9223372036854775807LL - 1;
        RblxS64 LLONG_MAX = 9223372036854775807LL;
        RblxU64 ULLONG_MAX = (2ULL * 9223372036854775807LL + 1);
        #endif
        const char    s8_zero  = 0,   s8_one  = 1,   s8_fifty  = 50, s8_min  = -128,        s8_max = 127;
        const RblxU8    u8_zero  = 0,   u8_one  = 1,   u8_fifty  = 50, u8_min  = 0,           u8_max = 255;
        const short   s16_zero = 0,   s16_one = 1,   s16_fifty = 50, s16_min = -32768,      s16_max = 32767;
        const RblxU16   u16_zero = 0,   u16_one = 1,   u16_fifty = 50, u16_min = 0,           u16_max = 65535;
        const RblxS32   s32_zero = 0,   s32_one = 1,   s32_fifty = 50, s32_min = INT_MIN/2,   s32_max = INT_MAX/2,    s32_hi_a = INT_MAX/2 - 100,    s32_hi_b = INT_MAX/2;
        const RblxU32   u32_zero = 0,   u32_one = 1,   u32_fifty = 50, u32_min = 0,           u32_max = UINT_MAX/2,   u32_hi_a = UINT_MAX/2 - 100,   u32_hi_b = UINT_MAX/2;
        const RblxS64   s64_zero = 0,   s64_one = 1,   s64_fifty = 50, s64_min = LLONG_MIN/2, s64_max = LLONG_MAX/2,  s64_hi_a = LLONG_MAX/2 - 100,  s64_hi_b = LLONG_MAX/2;
        const RblxU64   u64_zero = 0,   u64_one = 1,   u64_fifty = 50, u64_min = 0,           u64_max = ULLONG_MAX/2, u64_hi_a = ULLONG_MAX/2 - 100, u64_hi_b = ULLONG_MAX/2;
        const float   f32_zero = 0.f, f32_one = 1.f, f32_lo_a = -10000000000.0f, f32_hi_a = +10000000000.0f;
        const double  f64_zero = 0.,  f64_one = 1.,  f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

        // State
        static char   s8_v  = 127;
        static RblxU8   u8_v  = 255;
        static short  s16_v = 32767;
        static RblxU16  u16_v = 65535;
        static RblxS32  s32_v = -1;
        static RblxU32  u32_v = (RblxU32)-1;
        static RblxS64  s64_v = -1;
        static RblxU64  u64_v = (RblxU64)-1;
        static float  f32_v = 0.123f;
        static double f64_v = 90000.01234567890123456789;

        const float drag_speed = 0.2f;
        static bool drag_clamp = false;
        RBLXUI_DEMO_MARKER("Widgets/Data Types/Drags");
        RblxUI::SeparatorText("Drags");
        RblxUI::Checkbox("Clamp integers to 0..50", &drag_clamp);
        RblxUI::SameLine(); HelpMarker(
            "As with every widget in dear rblxui, we never modify values unless there is a user interaction.\n"
            "You can override the clamping limits by using Ctrl+Click to input a value.");
        RblxUI::DragScalar("drag s8",        RblxUIDataType_S8,     &s8_v,  drag_speed, drag_clamp ? &s8_zero  : NULL, drag_clamp ? &s8_fifty  : NULL);
        RblxUI::DragScalar("drag u8",        RblxUIDataType_U8,     &u8_v,  drag_speed, drag_clamp ? &u8_zero  : NULL, drag_clamp ? &u8_fifty  : NULL, "%u ms");
        RblxUI::DragScalar("drag s16",       RblxUIDataType_S16,    &s16_v, drag_speed, drag_clamp ? &s16_zero : NULL, drag_clamp ? &s16_fifty : NULL);
        RblxUI::DragScalar("drag u16",       RblxUIDataType_U16,    &u16_v, drag_speed, drag_clamp ? &u16_zero : NULL, drag_clamp ? &u16_fifty : NULL, "%u ms");
        RblxUI::DragScalar("drag s32",       RblxUIDataType_S32,    &s32_v, drag_speed, drag_clamp ? &s32_zero : NULL, drag_clamp ? &s32_fifty : NULL);
        RblxUI::DragScalar("drag s32 hex",   RblxUIDataType_S32,    &s32_v, drag_speed, drag_clamp ? &s32_zero : NULL, drag_clamp ? &s32_fifty : NULL, "0x%08X");
        RblxUI::DragScalar("drag u32",       RblxUIDataType_U32,    &u32_v, drag_speed, drag_clamp ? &u32_zero : NULL, drag_clamp ? &u32_fifty : NULL, "%u ms");
        RblxUI::DragScalar("drag s64",       RblxUIDataType_S64,    &s64_v, drag_speed, drag_clamp ? &s64_zero : NULL, drag_clamp ? &s64_fifty : NULL);
        RblxUI::DragScalar("drag u64",       RblxUIDataType_U64,    &u64_v, drag_speed, drag_clamp ? &u64_zero : NULL, drag_clamp ? &u64_fifty : NULL);
        RblxUI::DragScalar("drag float",     RblxUIDataType_Float,  &f32_v, 0.005f,  &f32_zero, &f32_one, "%f");
        RblxUI::DragScalar("drag float log", RblxUIDataType_Float,  &f32_v, 0.005f,  &f32_zero, &f32_one, "%f", RblxUISliderFlags_Logarithmic);
        RblxUI::DragScalar("drag double",    RblxUIDataType_Double, &f64_v, 0.0005f, &f64_zero, NULL,     "%.10f grams");
        RblxUI::DragScalar("drag double log",RblxUIDataType_Double, &f64_v, 0.0005f, &f64_zero, &f64_one, "0 < %.10f < 1", RblxUISliderFlags_Logarithmic);

        RBLXUI_DEMO_MARKER("Widgets/Data Types/Sliders");
        RblxUI::SeparatorText("Sliders");
        RblxUI::SliderScalar("slider s8 full",       RblxUIDataType_S8,     &s8_v,  &s8_min,   &s8_max,   "%d");
        RblxUI::SliderScalar("slider u8 full",       RblxUIDataType_U8,     &u8_v,  &u8_min,   &u8_max,   "%u");
        RblxUI::SliderScalar("slider s16 full",      RblxUIDataType_S16,    &s16_v, &s16_min,  &s16_max,  "%d");
        RblxUI::SliderScalar("slider u16 full",      RblxUIDataType_U16,    &u16_v, &u16_min,  &u16_max,  "%u");
        RblxUI::SliderScalar("slider s32 low",       RblxUIDataType_S32,    &s32_v, &s32_zero, &s32_fifty,"%d");
        RblxUI::SliderScalar("slider s32 high",      RblxUIDataType_S32,    &s32_v, &s32_hi_a, &s32_hi_b, "%d");
        RblxUI::SliderScalar("slider s32 full",      RblxUIDataType_S32,    &s32_v, &s32_min,  &s32_max,  "%d");
        RblxUI::SliderScalar("slider s32 hex",       RblxUIDataType_S32,    &s32_v, &s32_zero, &s32_fifty, "0x%04X");
        RblxUI::SliderScalar("slider u32 low",       RblxUIDataType_U32,    &u32_v, &u32_zero, &u32_fifty,"%u");
        RblxUI::SliderScalar("slider u32 high",      RblxUIDataType_U32,    &u32_v, &u32_hi_a, &u32_hi_b, "%u");
        RblxUI::SliderScalar("slider u32 full",      RblxUIDataType_U32,    &u32_v, &u32_min,  &u32_max,  "%u");
        RblxUI::SliderScalar("slider s64 low",       RblxUIDataType_S64,    &s64_v, &s64_zero, &s64_fifty,"%" PRId64);
        RblxUI::SliderScalar("slider s64 high",      RblxUIDataType_S64,    &s64_v, &s64_hi_a, &s64_hi_b, "%" PRId64);
        RblxUI::SliderScalar("slider s64 full",      RblxUIDataType_S64,    &s64_v, &s64_min,  &s64_max,  "%" PRId64);
        RblxUI::SliderScalar("slider u64 low",       RblxUIDataType_U64,    &u64_v, &u64_zero, &u64_fifty,"%" PRIu64 " ms");
        RblxUI::SliderScalar("slider u64 high",      RblxUIDataType_U64,    &u64_v, &u64_hi_a, &u64_hi_b, "%" PRIu64 " ms");
        RblxUI::SliderScalar("slider u64 full",      RblxUIDataType_U64,    &u64_v, &u64_min,  &u64_max,  "%" PRIu64 " ms");
        RblxUI::SliderScalar("slider float low",     RblxUIDataType_Float,  &f32_v, &f32_zero, &f32_one);
        RblxUI::SliderScalar("slider float low log", RblxUIDataType_Float,  &f32_v, &f32_zero, &f32_one,  "%.10f", RblxUISliderFlags_Logarithmic);
        RblxUI::SliderScalar("slider float high",    RblxUIDataType_Float,  &f32_v, &f32_lo_a, &f32_hi_a, "%e");
        RblxUI::SliderScalar("slider double low",    RblxUIDataType_Double, &f64_v, &f64_zero, &f64_one,  "%.10f grams");
        RblxUI::SliderScalar("slider double low log",RblxUIDataType_Double, &f64_v, &f64_zero, &f64_one,  "%.10f", RblxUISliderFlags_Logarithmic);
        RblxUI::SliderScalar("slider double high",   RblxUIDataType_Double, &f64_v, &f64_lo_a, &f64_hi_a, "%e grams");

        RblxUI::SeparatorText("Sliders (reverse)");
        RblxUI::SliderScalar("slider s8 reverse",    RblxUIDataType_S8,   &s8_v,  &s8_max,    &s8_min,   "%d");
        RblxUI::SliderScalar("slider u8 reverse",    RblxUIDataType_U8,   &u8_v,  &u8_max,    &u8_min,   "%u");
        RblxUI::SliderScalar("slider s32 reverse",   RblxUIDataType_S32,  &s32_v, &s32_fifty, &s32_zero, "%d");
        RblxUI::SliderScalar("slider u32 reverse",   RblxUIDataType_U32,  &u32_v, &u32_fifty, &u32_zero, "%u");
        RblxUI::SliderScalar("slider s64 reverse",   RblxUIDataType_S64,  &s64_v, &s64_fifty, &s64_zero, "%" PRId64);
        RblxUI::SliderScalar("slider u64 reverse",   RblxUIDataType_U64,  &u64_v, &u64_fifty, &u64_zero, "%" PRIu64 " ms");

        RBLXUI_DEMO_MARKER("Widgets/Data Types/Inputs");
        static bool inputs_step = true;
        static RblxUIInputTextFlags flags = RblxUIInputTextFlags_None;
        RblxUI::SeparatorText("Inputs");
        RblxUI::Checkbox("Show step buttons", &inputs_step);
        RblxUI::CheckboxFlags("RblxUIInputTextFlags_ReadOnly", &flags, RblxUIInputTextFlags_ReadOnly);
        RblxUI::CheckboxFlags("RblxUIInputTextFlags_ParseEmptyRefVal", &flags, RblxUIInputTextFlags_ParseEmptyRefVal);
        RblxUI::CheckboxFlags("RblxUIInputTextFlags_DisplayEmptyRefVal", &flags, RblxUIInputTextFlags_DisplayEmptyRefVal);
        RblxUI::InputScalar("input s8",      RblxUIDataType_S8,     &s8_v,  inputs_step ? &s8_one  : NULL, NULL, "%d", flags);
        RblxUI::InputScalar("input u8",      RblxUIDataType_U8,     &u8_v,  inputs_step ? &u8_one  : NULL, NULL, "%u", flags);
        RblxUI::InputScalar("input s16",     RblxUIDataType_S16,    &s16_v, inputs_step ? &s16_one : NULL, NULL, "%d", flags);
        RblxUI::InputScalar("input u16",     RblxUIDataType_U16,    &u16_v, inputs_step ? &u16_one : NULL, NULL, "%u", flags);
        RblxUI::InputScalar("input s32",     RblxUIDataType_S32,    &s32_v, inputs_step ? &s32_one : NULL, NULL, "%d", flags);
        RblxUI::InputScalar("input s32 hex", RblxUIDataType_S32,    &s32_v, inputs_step ? &s32_one : NULL, NULL, "%04X", flags);
        RblxUI::InputScalar("input u32",     RblxUIDataType_U32,    &u32_v, inputs_step ? &u32_one : NULL, NULL, "%u", flags);
        RblxUI::InputScalar("input u32 hex", RblxUIDataType_U32,    &u32_v, inputs_step ? &u32_one : NULL, NULL, "%08X", flags);
        RblxUI::InputScalar("input s64",     RblxUIDataType_S64,    &s64_v, inputs_step ? &s64_one : NULL, NULL, NULL, flags);
        RblxUI::InputScalar("input u64",     RblxUIDataType_U64,    &u64_v, inputs_step ? &u64_one : NULL, NULL, NULL, flags);
        RblxUI::InputScalar("input float",   RblxUIDataType_Float,  &f32_v, inputs_step ? &f32_one : NULL, NULL, NULL, flags);
        RblxUI::InputScalar("input double",  RblxUIDataType_Double, &f64_v, inputs_step ? &f64_one : NULL, NULL, NULL, flags);

        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsDisableBlocks()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsDisableBlocks(RblxUIDemoWindowData* demo_data)
{
    if (RblxUI::TreeNode("Disable Blocks"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Disable Blocks");
        RblxUI::Checkbox("Disable entire section above", &demo_data->DisableSections);
        RblxUI::SameLine(); HelpMarker("Demonstrate using BeginDisabled()/EndDisabled() across other sections.");
        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsDragAndDrop()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsDragAndDrop()
{
    if (RblxUI::TreeNode("Drag and Drop"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Drag and drop");
        if (RblxUI::TreeNode("Drag and drop in standard widgets"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Drag and drop/Standard widgets");
            // ColorEdit widgets automatically act as drag source and drag target.
            // They are using standardized payload strings RBLXUI_PAYLOAD_TYPE_COLOR_3F and RBLXUI_PAYLOAD_TYPE_COLOR_4F
            // to allow your own widgets to use colors in their drag and drop interaction.
            // Also see 'Demo->Widgets->Color/Picker Widgets->Palette' demo.
            HelpMarker("You can drag from the color squares.");
            static float col1[3] = { 1.0f, 0.0f, 0.2f };
            static float col2[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
            RblxUI::ColorEdit3("color 1", col1);
            RblxUI::ColorEdit4("color 2", col2);
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Drag and drop to copy/swap items"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Drag and drop/Copy-swap items");
            enum Mode
            {
                Mode_Copy,
                Mode_Move,
                Mode_Swap
            };
            static int mode = 0;
            if (RblxUI::RadioButton("Copy", mode == Mode_Copy)) { mode = Mode_Copy; } RblxUI::SameLine();
            if (RblxUI::RadioButton("Move", mode == Mode_Move)) { mode = Mode_Move; } RblxUI::SameLine();
            if (RblxUI::RadioButton("Swap", mode == Mode_Swap)) { mode = Mode_Swap; }
            static const char* names[9] =
            {
                "Bobby", "Beatrice", "Betty",
                "Brianna", "Barry", "Bernard",
                "Bibi", "Blaine", "Bryn"
            };
            for (int n = 0; n < RBLXUI_COUNTOF(names); n++)
            {
                RblxUI::PushID(n);
                if ((n % 3) != 0)
                    RblxUI::SameLine();
                RblxUI::Button(names[n], RblxVec2(60, 60));

                // Our buttons are both drag sources and drag targets here!
                if (RblxUI::BeginDragDropSource(RblxUIDragDropFlags_None))
                {
                    // Set payload to carry the index of our item (could be anything)
                    RblxUI::SetDragDropPayload("DND_DEMO_CELL", &n, sizeof(int));

                    // Display preview (could be anything, e.g. when dragging an image we could decide to display
                    // the filename and a small preview of the image, etc.)
                    if (mode == Mode_Copy) { RblxUI::Text("Copy %s", names[n]); }
                    if (mode == Mode_Move) { RblxUI::Text("Move %s", names[n]); }
                    if (mode == Mode_Swap) { RblxUI::Text("Swap %s", names[n]); }
                    RblxUI::EndDragDropSource();
                }
                if (RblxUI::BeginDragDropTarget())
                {
                    if (const RblxUIPayload* payload = RblxUI::AcceptDragDropPayload("DND_DEMO_CELL"))
                    {
                        RBLXUI_ASSERT(payload->DataSize == sizeof(int));
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
                    RblxUI::EndDragDropTarget();
                }
                RblxUI::PopID();
            }
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Drag to reorder items (simple)"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Drag and Drop/Drag to reorder items (simple)");
            // FIXME: there is temporary (usually single-frame) ID Conflict during reordering as a same item may be submitting twice.
            // This code was always slightly faulty but in a way which was not easily noticeable.
            // Until we fix this, enable RblxUIItemFlags_AllowDuplicateId to disable detecting the issue.
            RblxUI::PushItemFlag(RblxUIItemFlags_AllowDuplicateId, true);

            // Simple reordering
            HelpMarker(
                "We don't use the drag and drop api at all here! "
                "Instead we query when the item is held but not hovered, and order items accordingly.");
            static const char* item_names[] = { "Item One", "Item Two", "Item Three", "Item Four", "Item Five" };
            for (int n = 0; n < RBLXUI_COUNTOF(item_names); n++)
            {
                const char* item = item_names[n];
                RblxUI::Selectable(item);

                if (RblxUI::IsItemActive() && !RblxUI::IsItemHovered())
                {
                    int n_next = n + (RblxUI::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
                    if (n_next >= 0 && n_next < RBLXUI_COUNTOF(item_names))
                    {
                        item_names[n] = item_names[n_next];
                        item_names[n_next] = item;
                        RblxUI::ResetMouseDragDelta();
                    }
                }
            }

            RblxUI::PopItemFlag();
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Tooltip at target location"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Drag and Drop/Tooltip at target location");
            for (int n = 0; n < 2; n++)
            {
                // Drop targets
                RblxUI::Button(n ? "drop here##1" : "drop here##0");
                if (RblxUI::BeginDragDropTarget())
                {
                    RblxUIDragDropFlags drop_target_flags = RblxUIDragDropFlags_AcceptBeforeDelivery | RblxUIDragDropFlags_AcceptNoPreviewTooltip;
                    if (const RblxUIPayload* payload = RblxUI::AcceptDragDropPayload(RBLXUI_PAYLOAD_TYPE_COLOR_4F, drop_target_flags))
                    {
                        RBLXUI_UNUSED(payload);
                        RblxUI::SetMouseCursor(RblxUIMouseCursor_NotAllowed);
                        RblxUI::SetTooltip("Cannot drop here!");
                    }
                    RblxUI::EndDragDropTarget();
                }

                // Drop source
                static RblxVec4 col4 = { 1.0f, 0.0f, 0.2f, 1.0f };
                if (n == 0)
                    RblxUI::ColorButton("drag me", col4);

            }
            RblxUI::TreePop();
        }

        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsDragsAndSliders()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsDragsAndSliders()
{
    if (RblxUI::TreeNode("Drag/Slider Flags"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Drag and Slider Flags");
        // Demonstrate using advanced flags for DragXXX and SliderXXX functions. Note that the flags are the same!
        static RblxUISliderFlags flags = RblxUISliderFlags_None;
        RblxUI::CheckboxFlags("RblxUISliderFlags_AlwaysClamp", &flags, RblxUISliderFlags_AlwaysClamp);
        RblxUI::CheckboxFlags("RblxUISliderFlags_ClampOnInput", &flags, RblxUISliderFlags_ClampOnInput);
        RblxUI::SameLine(); HelpMarker("Clamp value to min/max bounds when input manually with Ctrl+Click. By default Ctrl+Click allows going out of bounds.");
        RblxUI::CheckboxFlags("RblxUISliderFlags_ClampZeroRange", &flags, RblxUISliderFlags_ClampZeroRange);
        RblxUI::SameLine(); HelpMarker("Clamp even if min==max==0.0f. Otherwise DragXXX functions don't clamp.");
        RblxUI::CheckboxFlags("RblxUISliderFlags_Logarithmic", &flags, RblxUISliderFlags_Logarithmic);
        RblxUI::SameLine(); HelpMarker("Enable logarithmic editing (more precision for small values).");
        RblxUI::CheckboxFlags("RblxUISliderFlags_NoRoundToFormat", &flags, RblxUISliderFlags_NoRoundToFormat);
        RblxUI::SameLine(); HelpMarker("Disable rounding underlying value to match precision of the format string (e.g. %.3f values are rounded to those 3 digits).");
        RblxUI::CheckboxFlags("RblxUISliderFlags_NoInput", &flags, RblxUISliderFlags_NoInput);
        RblxUI::SameLine(); HelpMarker("Disable Ctrl+Click or Enter key allowing to input text directly into the widget.");
        RblxUI::CheckboxFlags("RblxUISliderFlags_NoSpeedTweaks", &flags, RblxUISliderFlags_NoSpeedTweaks);
        RblxUI::SameLine(); HelpMarker("Disable keyboard modifiers altering tweak speed. Useful if you want to alter tweak speed yourself based on your own logic.");
        RblxUI::CheckboxFlags("RblxUISliderFlags_WrapAround", &flags, RblxUISliderFlags_WrapAround);
        RblxUI::SameLine(); HelpMarker("Enable wrapping around from max to min and from min to max (only supported by DragXXX() functions)");
        RblxUI::CheckboxFlags("RblxUISliderFlags_ColorMarkers", &flags, RblxUISliderFlags_ColorMarkers);

        // Drags
        static float drag_f = 0.5f;
        static float drag_f4[4];
        static int drag_i = 50;
        RblxUI::Text("Underlying float value: %f", drag_f);
        RblxUI::DragFloat("DragFloat (0 -> 1)", &drag_f, 0.005f, 0.0f, 1.0f, "%.3f", flags);
        RblxUI::DragFloat("DragFloat (0 -> +inf)", &drag_f, 0.005f, 0.0f, FLT_MAX, "%.3f", flags);
        RblxUI::DragFloat("DragFloat (-inf -> 1)", &drag_f, 0.005f, -FLT_MAX, 1.0f, "%.3f", flags);
        RblxUI::DragFloat("DragFloat (-inf -> +inf)", &drag_f, 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
        //RblxUI::DragFloat("DragFloat (0 -> 0)", &drag_f, 0.005f, 0.0f, 0.0f, "%.3f", flags);           // To test ClampZeroRange
        //RblxUI::DragFloat("DragFloat (100 -> 100)", &drag_f, 0.005f, 100.0f, 100.0f, "%.3f", flags);
        RblxUI::DragInt("DragInt (0 -> 100)", &drag_i, 0.5f, 0, 100, "%d", flags);
        RblxUI::DragFloat4("DragFloat4 (0 -> 1)", drag_f4, 0.005f, 0.0f, 1.0f, "%.3f", flags); // Multi-component item, mostly here to document the effect of RblxUISliderFlags_ColorMarkers.

        // Sliders
        static float slider_f = 0.5f;
        static float slider_f4[4];
        static int slider_i = 50;
        const RblxUISliderFlags flags_for_sliders = (flags & ~RblxUISliderFlags_WrapAround);
        RblxUI::Text("Underlying float value: %f", slider_f);
        RblxUI::SliderFloat("SliderFloat (0 -> 1)", &slider_f, 0.0f, 1.0f, "%.3f", flags_for_sliders);
        RblxUI::SliderInt("SliderInt (0 -> 100)", &slider_i, 0, 100, "%d", flags_for_sliders);
        RblxUI::SliderFloat4("SliderFloat4 (0 -> 1)", slider_f4, 0.0f, 1.0f, "%.3f", flags); // Multi-component item, mostly here to document the effect of RblxUISliderFlags_ColorMarkers.

        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsFonts()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsFonts()
{
    if (RblxUI::TreeNode("Fonts"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Fonts");
        RblxFontAtlas* atlas = RblxUI::GetIO().Fonts;
        RblxUI::ShowFontAtlas(atlas);
        // FIXME-NEWATLAS: Provide a demo to add/create a procedural font?
        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsImages()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsImages()
{
    if (RblxUI::TreeNode("Images"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Images");
        RblxUIIO& io = RblxUI::GetIO();
        RblxUI::TextWrapped(
            "Below we are displaying the font texture (which is the only texture we have access to in this demo). "
            "Use the 'RblxTextureID' type as storage to pass pointers or identifier to your own texture data. "
            "Hover the texture for a zoomed view!");

        // Below we are displaying the font texture because it is the only texture we have access to inside the demo!
        // Read description about RblxTextureID/RblxTextureRef and FAQ for details about texture identifiers.
        // If you use one of the default rblxui_impl_XXXX.cpp rendering backend, they all have comments at the top
        // of their respective source file to specify what they are using as texture identifier, for example:
        // - The rblxui_impl_dx11.cpp renderer expect a 'ID3D11ShaderResourceView*' pointer.
        // - The rblxui_impl_opengl3.cpp renderer expect a GLuint OpenGL texture identifier, etc.
        // So with the DirectX11 backend, you call RblxUI::Image() with a 'ID3D11ShaderResourceView*' cast to RblxTextureID.
        // - If you decided that RblxTextureID = MyEngineTexture*, then you can pass your MyEngineTexture* pointers
        //   to RblxUI::Image(), and gather width/height through your own functions, etc.
        // - You can use ShowMetricsWindow() to inspect the draw data that are being passed to your renderer,
        //   it will help you debug issues if you are confused about it.
        // - Consider using the lower-level RblxDrawList::AddImage() API, via RblxUI::GetWindowDrawList()->AddImage().
        // - Read https://github.com/ocornut/rblxui/blob/master/docs/FAQ.md
        // - Read https://github.com/ocornut/rblxui/wiki/Image-Loading-and-Displaying-Examples

        // Grab the current texture identifier used by the font atlas.
        RblxFontAtlas* atlas = io.Fonts;
        RblxTextureRef my_tex_id = atlas->TexRef;
        float my_tex_w = (float)atlas->TexData->Width; // Regular user code should never have to care about TexData-> fields, but since we want to display the entire texture here, we pull Width/Height from it.
        float my_tex_h = (float)atlas->TexData->Height;
        RblxUI::Text("%.0fx%.0f", my_tex_w, my_tex_h);

        // Basic drawing
        RblxUI::SeparatorText("Image()/ImageWithBg() function");
        RblxVec2 uv_min = RblxVec2(0.0f, 0.0f); // Top-left
        RblxVec2 uv_max = RblxVec2(1.0f, 1.0f); // Lower-right
        RblxUI::PushStyleVar(RblxUIStyleVar_ImageBorderSize, RBLXUI_MAX(1.0f, RblxUI::GetStyle().ImageBorderSize));
        RblxUI::ImageWithBg(my_tex_id, RblxVec2(my_tex_w, my_tex_h), uv_min, uv_max, RblxVec4(0.0f, 0.0f, 0.0f, 1.0f));
        RblxUI::PopStyleVar();

        // Fancy widget
        RblxUI::SeparatorText("Interactive Image Viewer");
        static ExampleImageViewerData image_viewer;
        RblxVec2 canvas_size(RblxUI::GetContentRegionAvail().x, my_tex_h * 2.0f);
        ExampleImageViewer_DrawOptions(&image_viewer);
        ExampleImageViewer_DrawCanvas(&image_viewer, canvas_size, my_tex_id, (int)my_tex_w, (int)my_tex_h);

        RBLXUI_DEMO_MARKER("Widgets/Images/Textured buttons");
        RblxUI::SeparatorText("Textured Buttons");
        RblxUI::TextWrapped("And now some textured buttons..");
        static int pressed_count = 0;
        for (int i = 0; i < 8; i++)
        {
            // UV coordinates are often (0.0f, 0.0f) and (1.0f, 1.0f) to display an entire textures.
            // Here are trying to display only a 32x32 pixels area of the texture, hence the UV computation.
            // Read about UV coordinates here: https://github.com/ocornut/rblxui/wiki/Image-Loading-and-Displaying-Examples
            RblxUI::PushID(i);
            if (i > 0)
                RblxUI::PushStyleVar(RblxUIStyleVar_FramePadding, RblxVec2(i - 1.0f, i - 1.0f));
            RblxVec2 size = RblxVec2(32.0f, 32.0f);                         // Size of the image we want to make visible
            RblxVec2 uv0 = RblxVec2(0.0f, 0.0f);                            // UV coordinates for lower-left
            RblxVec2 uv1 = RblxVec2(32.0f / my_tex_w, 32.0f / my_tex_h);    // UV coordinates for (32,32) in our texture
            RblxVec4 bg_col = RblxVec4(0.0f, 0.0f, 0.0f, 1.0f);             // Black background
            RblxVec4 tint_col = RblxVec4(1.0f, 1.0f, 1.0f, 1.0f);           // No tint
            if (RblxUI::ImageButton("", my_tex_id, size, uv0, uv1, bg_col, tint_col))
                pressed_count += 1;
            if (i > 0)
                RblxUI::PopStyleVar();
            RblxUI::PopID();
            RblxUI::SameLine();
        }
        RblxUI::NewLine();
        RblxUI::Text("Pressed %d times.", pressed_count);
        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsListBoxes()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsListBoxes()
{
    if (RblxUI::TreeNode("List Boxes"))
    {
        RBLXUI_DEMO_MARKER("Widgets/List Boxes");
        // BeginListBox() is essentially a thin wrapper to using BeginChild()/EndChild()
        // using the RblxUIChildFlags_FrameStyle flag for stylistic changes + displaying a label.
        // You may be tempted to simply use BeginChild() directly. However note that BeginChild() requires EndChild()
        // to always be called (inconsistent with BeginListBox()/EndListBox()).

        // Using the generic BeginListBox() API, you have full control over how to display the combo contents.
        // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
        // stored in the object itself, etc.)
        const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
        static int item_selected_idx = 0; // Here we store our selected data as an index.

        static bool item_highlight = false;
        int item_highlighted_idx = -1; // Here we store our highlighted data as an index.
        RblxUI::Checkbox("Highlight hovered item in second listbox", &item_highlight);

        if (RblxUI::BeginListBox("listbox 1"))
        {
            for (int n = 0; n < RBLXUI_COUNTOF(items); n++)
            {
                const bool is_selected = (item_selected_idx == n);
                if (RblxUI::Selectable(items[n], is_selected))
                    item_selected_idx = n;

                if (item_highlight && RblxUI::IsItemHovered())
                    item_highlighted_idx = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    RblxUI::SetItemDefaultFocus();
            }
            RblxUI::EndListBox();
        }
        RblxUI::SameLine(); HelpMarker("Here we are sharing selection state between both boxes.");

        // Custom size: use all width, 5 items tall
        RblxUI::Text("Full-width:");
        if (RblxUI::BeginListBox("##listbox 2", RblxVec2(-FLT_MIN, 5 * RblxUI::GetTextLineHeightWithSpacing())))
        {
            for (int n = 0; n < RBLXUI_COUNTOF(items); n++)
            {
                bool is_selected = (item_selected_idx == n);
                RblxUISelectableFlags flags = (item_highlighted_idx == n) ? RblxUISelectableFlags_Highlight : 0;
                if (RblxUI::Selectable(items[n], is_selected, flags))
                    item_selected_idx = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    RblxUI::SetItemDefaultFocus();
            }
            RblxUI::EndListBox();
        }

        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsMultiComponents()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsMultiComponents()
{
    if (RblxUI::TreeNode("Multi-component Widgets"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Multi-component Widgets");
        static float vec4f[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
        static int vec4i[4] = { 1, 5, 100, 255 };

        static RblxUISliderFlags flags = 0;
        RblxUI::CheckboxFlags("RblxUISliderFlags_ColorMarkers", &flags, RblxUISliderFlags_ColorMarkers); // Only passing this to Drag/Sliders

        RblxUI::SeparatorText("2-wide");
        RblxUI::InputFloat2("input float2", vec4f);
        RblxUI::InputInt2("input int2", vec4i);
        RblxUI::DragFloat2("drag float2", vec4f, 0.01f, 0.0f, 1.0f, NULL, flags);
        RblxUI::DragInt2("drag int2", vec4i, 1, 0, 255, NULL, flags);
        RblxUI::SliderFloat2("slider float2", vec4f, 0.0f, 1.0f, NULL, flags);
        RblxUI::SliderInt2("slider int2", vec4i, 0, 255, NULL, flags);

        RblxUI::SeparatorText("3-wide");
        RblxUI::InputFloat3("input float3", vec4f);
        RblxUI::InputInt3("input int3", vec4i);
        RblxUI::DragFloat3("drag float3", vec4f, 0.01f, 0.0f, 1.0f, NULL, flags);
        RblxUI::DragInt3("drag int3", vec4i, 1, 0, 255, NULL, flags);
        RblxUI::SliderFloat3("slider float3", vec4f, 0.0f, 1.0f, NULL, flags);
        RblxUI::SliderInt3("slider int3", vec4i, 0, 255, NULL, flags);

        RblxUI::SeparatorText("4-wide");
        RblxUI::InputFloat4("input float4", vec4f);
        RblxUI::InputInt4("input int4", vec4i);
        RblxUI::DragFloat4("drag float4", vec4f, 0.01f, 0.0f, 1.0f, NULL, flags);
        RblxUI::DragInt4("drag int4", vec4i, 1, 0, 255, NULL, flags);
        RblxUI::SliderFloat4("slider float4", vec4f, 0.0f, 1.0f, NULL, flags);
        RblxUI::SliderInt4("slider int4", vec4i, 0, 255, NULL, flags);

        RblxUI::SeparatorText("Ranges");
        static float begin = 10, end = 90;
        static int begin_i = 100, end_i = 1000;
        RblxUI::DragFloatRange2("range float", &begin, &end, 0.25f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%", RblxUISliderFlags_AlwaysClamp);
        RblxUI::DragIntRange2("range int", &begin_i, &end_i, 5, 0, 1000, "Min: %d units", "Max: %d units");
        RblxUI::DragIntRange2("range int (no bounds)", &begin_i, &end_i, 5, 0, 0, "Min: %d units", "Max: %d units");

        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsPlotting()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsPlotting()
{
    // Plot/Graph widgets are not very good.
// Consider using a third-party library such as RblxPlot: https://github.com/epezent/implot
// (see others https://github.com/ocornut/rblxui/wiki/Useful-Extensions)
    if (RblxUI::TreeNode("Plotting"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Plotting");
        RblxUI::Text("Need better plotting and graphing? Consider using RblxPlot:");
        RblxUI::TextLinkOpenURL("https://github.com/epezent/implot");
        RblxUI::Separator();

        static bool animate = true;
        RblxUI::Checkbox("Animate", &animate);

        // Plot as lines and plot as histogram
        static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
        RblxUI::PlotLines("Frame Times", arr, RBLXUI_COUNTOF(arr));
        RblxUI::PlotHistogram("Histogram", arr, RBLXUI_COUNTOF(arr), 0, NULL, 0.0f, 1.0f, RblxVec2(0, 80.0f));
        //RblxUI::SameLine(); HelpMarker("Consider using RblxPlot instead!");

        // Fill an array of contiguous float values to plot
        // Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float
        // and the sizeof() of your structure in the "stride" parameter.
        static float values[90] = {};
        static int values_offset = 0;
        static double refresh_time = 0.0;
        if (!animate || refresh_time == 0.0)
            refresh_time = RblxUI::GetTime();
        while (refresh_time < RblxUI::GetTime()) // Create data at fixed 60 Hz rate for the demo
        {
            static float phase = 0.0f;
            values[values_offset] = cosf(phase);
            values_offset = (values_offset + 1) % RBLXUI_COUNTOF(values);
            phase += 0.10f * values_offset;
            refresh_time += 1.0f / 60.0f;
        }

        // Plots can display overlay texts
        // (in this example, we will display an average value)
        {
            float average = 0.0f;
            for (int n = 0; n < RBLXUI_COUNTOF(values); n++)
                average += values[n];
            average /= (float)RBLXUI_COUNTOF(values);
            char overlay[32];
            sprintf(overlay, "avg %f", average);
            RblxUI::PlotLines("Lines", values, RBLXUI_COUNTOF(values), values_offset, overlay, -1.0f, 1.0f, RblxVec2(0, 80.0f));
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
        RblxUI::SeparatorText("Functions");
        RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 8);
        RblxUI::Combo("func", &func_type, "Sin\0Saw\0");
        RblxUI::SameLine();
        RblxUI::SliderInt("Sample count", &display_count, 1, 400);
        float (*func)(void*, int) = (func_type == 0) ? Funcs::Sin : Funcs::Saw;
        RblxUI::PlotLines("Lines##2", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, RblxVec2(0, 80));
        RblxUI::PlotHistogram("Histogram##2", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, RblxVec2(0, 80));

        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsProgressBars()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsProgressBars()
{
    if (RblxUI::TreeNode("Progress Bars"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Progress Bars");
        // Animate a simple progress bar
        static float progress_accum = 0.0f, progress_dir = 1.0f;
        progress_accum += progress_dir * 0.4f * RblxUI::GetIO().DeltaTime;
        if (progress_accum >= +1.1f) { progress_accum = +1.1f; progress_dir *= -1.0f; }
        if (progress_accum <= -0.1f) { progress_accum = -0.1f; progress_dir *= -1.0f; }

        const float progress = RBLXUI_CLAMP(progress_accum, 0.0f, 1.0f);

        // Typically we would use RblxVec2(-1.0f,0.0f) or RblxVec2(-FLT_MIN,0.0f) to use all available width,
        // or RblxVec2(width,0.0f) for a specified width. RblxVec2(0.0f,0.0f) uses ItemWidth.
        RblxUI::ProgressBar(progress, RblxVec2(0.0f, 0.0f));
        RblxUI::SameLine(0.0f, RblxUI::GetStyle().ItemInnerSpacing.x);
        RblxUI::Text("Progress Bar");

        char buf[32];
        sprintf(buf, "%d/%d", (int)(progress * 1753), 1753);
        RblxUI::ProgressBar(progress, RblxVec2(0.f, 0.f), buf);

        // Pass an animated negative value, e.g. -1.0f * (float)RblxUI::GetTime() is the recommended value.
        // Adjust the factor if you want to adjust the animation speed.
        RblxUI::ProgressBar(-1.0f * (float)RblxUI::GetTime(), RblxVec2(0.0f, 0.0f), "Searching..");
        RblxUI::SameLine(0.0f, RblxUI::GetStyle().ItemInnerSpacing.x);
        RblxUI::Text("Indeterminate");

        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsQueryingStatuses()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsQueryingStatuses()
{
    if (RblxUI::TreeNode("Querying Item Status (Edited/Active/Hovered etc.)"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Querying Item Status (Edited,Active,Hovered etc.)");
        // Select an item type
        const char* item_names[] =
        {
            "Text", "Button", "Button (w/ repeat)", "Checkbox", "SliderFloat", "InputText", "InputTextMultiline", "InputFloat",
            "InputFloat3", "ColorEdit4", "Selectable", "MenuItem", "TreeNode", "TreeNode (w/ double-click)", "Combo", "ListBox"
        };
        static int item_type = 4;
        static bool item_disabled = false;
        RblxUI::Combo("Item Type", &item_type, item_names, RBLXUI_COUNTOF(item_names), RBLXUI_COUNTOF(item_names));
        RblxUI::SameLine();
        HelpMarker("Testing how various types of items are interacting with the IsItemXXX functions. Note that the bool return value of most RblxUI function is generally equivalent to calling RblxUI::IsItemHovered().");
        RblxUI::Checkbox("Item Disabled", &item_disabled);

        // Submit selected items so we can query their status in the code following it.
        bool ret = false;
        static bool b = false;
        static float col4f[4] = { 1.0f, 0.5, 0.0f, 1.0f };
        static char str[16] = {};
        if (item_disabled)
            RblxUI::BeginDisabled(true);
        if (item_type == 0) { RblxUI::Text("ITEM: Text"); }                                              // Testing text items with no identifier/interaction
        if (item_type == 1) { ret = RblxUI::Button("ITEM: Button"); }                                    // Testing button
        if (item_type == 2) { RblxUI::PushItemFlag(RblxUIItemFlags_ButtonRepeat, true); ret = RblxUI::Button("ITEM: Button"); RblxUI::PopItemFlag(); } // Testing button (with repeater)
        if (item_type == 3) { ret = RblxUI::Checkbox("ITEM: Checkbox", &b); }                            // Testing checkbox
        if (item_type == 4) { ret = RblxUI::SliderFloat("ITEM: SliderFloat", &col4f[0], 0.0f, 1.0f); }   // Testing basic item
        if (item_type == 5) { ret = RblxUI::InputText("ITEM: InputText", &str[0], RBLXUI_COUNTOF(str)); }  // Testing input text (which handles tabbing)
        if (item_type == 6) { ret = RblxUI::InputTextMultiline("ITEM: InputTextMultiline", &str[0], RBLXUI_COUNTOF(str)); } // Testing input text (which uses a child window)
        if (item_type == 7) { ret = RblxUI::InputFloat("ITEM: InputFloat", col4f, 1.0f); }               // Testing +/- buttons on scalar input
        if (item_type == 8) { ret = RblxUI::InputFloat3("ITEM: InputFloat3", col4f); }                   // Testing multi-component items (IsItemXXX flags are reported merged)
        if (item_type == 9) { ret = RblxUI::ColorEdit4("ITEM: ColorEdit4", col4f); }                     // Testing multi-component items (IsItemXXX flags are reported merged)
        if (item_type == 10) { ret = RblxUI::Selectable("ITEM: Selectable"); }                            // Testing selectable item
        if (item_type == 11) { ret = RblxUI::MenuItem("ITEM: MenuItem"); }                                // Testing menu item (they use RblxUIButtonFlags_PressedOnRelease button policy)
        if (item_type == 12) { ret = RblxUI::TreeNode("ITEM: TreeNode"); if (ret) RblxUI::TreePop(); }     // Testing tree node
        if (item_type == 13) { ret = RblxUI::TreeNodeEx("ITEM: TreeNode w/ RblxUITreeNodeFlags_OpenOnDoubleClick", RblxUITreeNodeFlags_OpenOnDoubleClick | RblxUITreeNodeFlags_NoTreePushOnOpen); } // Testing tree node with RblxUIButtonFlags_PressedOnDoubleClick button policy.
        if (item_type == 14) { const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi" }; static int current = 1; ret = RblxUI::Combo("ITEM: Combo", &current, items, RBLXUI_COUNTOF(items)); }
        if (item_type == 15) { const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi" }; static int current = 1; ret = RblxUI::ListBox("ITEM: ListBox", &current, items, RBLXUI_COUNTOF(items), RBLXUI_COUNTOF(items)); }

        bool hovered_delay_none = RblxUI::IsItemHovered();
        bool hovered_delay_stationary = RblxUI::IsItemHovered(RblxUIHoveredFlags_Stationary);
        bool hovered_delay_short = RblxUI::IsItemHovered(RblxUIHoveredFlags_DelayShort);
        bool hovered_delay_normal = RblxUI::IsItemHovered(RblxUIHoveredFlags_DelayNormal);
        bool hovered_delay_tooltip = RblxUI::IsItemHovered(RblxUIHoveredFlags_ForTooltip); // = Normal + Stationary

        // Display the values of IsItemHovered() and other common item state functions.
        // Note that the RblxUIHoveredFlags_XXX flags can be combined.
        // Because BulletText is an item itself and that would affect the output of IsItemXXX functions,
        // we query every state in a single call to avoid storing them and to simplify the code.
        RblxUI::BulletText(
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
            RblxUI::IsItemFocused(),
            RblxUI::IsItemHovered(),
            RblxUI::IsItemHovered(RblxUIHoveredFlags_AllowWhenBlockedByPopup),
            RblxUI::IsItemHovered(RblxUIHoveredFlags_AllowWhenBlockedByActiveItem),
            RblxUI::IsItemHovered(RblxUIHoveredFlags_AllowWhenOverlappedByItem),
            RblxUI::IsItemHovered(RblxUIHoveredFlags_AllowWhenOverlappedByWindow),
            RblxUI::IsItemHovered(RblxUIHoveredFlags_AllowWhenDisabled),
            RblxUI::IsItemHovered(RblxUIHoveredFlags_RectOnly),
            RblxUI::IsItemActive(),
            RblxUI::IsItemEdited(),
            RblxUI::IsItemActivated(),
            RblxUI::IsItemDeactivated(),
            RblxUI::IsItemDeactivatedAfterEdit(),
            RblxUI::IsItemVisible(),
            RblxUI::IsItemClicked(),
            RblxUI::IsItemToggledOpen(),
            RblxUI::GetItemRectMin().x, RblxUI::GetItemRectMin().y,
            RblxUI::GetItemRectMax().x, RblxUI::GetItemRectMax().y,
            RblxUI::GetItemRectSize().x, RblxUI::GetItemRectSize().y
        );
        RblxUI::BulletText(
            "with Hovering Delay or Stationary test:\n"
            "IsItemHovered() = %d\n"
            "IsItemHovered(_Stationary) = %d\n"
            "IsItemHovered(_DelayShort) = %d\n"
            "IsItemHovered(_DelayNormal) = %d\n"
            "IsItemHovered(_Tooltip) = %d",
            hovered_delay_none, hovered_delay_stationary, hovered_delay_short, hovered_delay_normal, hovered_delay_tooltip);

        if (item_disabled)
            RblxUI::EndDisabled();

        char buf[1] = "";
        RblxUI::InputText("unused", buf, RBLXUI_COUNTOF(buf), RblxUIInputTextFlags_ReadOnly);
        RblxUI::SameLine();
        HelpMarker("This widget is only here to be able to tab-out of the widgets above and see e.g. Deactivated() status.");

        RblxUI::TreePop();
    }

    if (RblxUI::TreeNode("Querying Window Status (Focused/Hovered etc.)"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Querying Window Status (Focused,Hovered etc.)");
        static bool embed_all_inside_a_child_window = false;
        RblxUI::Checkbox("Embed everything inside a child window for testing _RootWindow flag.", &embed_all_inside_a_child_window);
        if (embed_all_inside_a_child_window)
            RblxUI::BeginChild("outer_child", RblxVec2(0, RblxUI::GetFontSize() * 20.0f), RblxUIChildFlags_Borders);

        // Testing IsWindowFocused() function with its various flags.
        RblxUI::BulletText(
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
            RblxUI::IsWindowFocused(),
            RblxUI::IsWindowFocused(RblxUIFocusedFlags_ChildWindows),
            RblxUI::IsWindowFocused(RblxUIFocusedFlags_ChildWindows | RblxUIFocusedFlags_NoPopupHierarchy),
            RblxUI::IsWindowFocused(RblxUIFocusedFlags_ChildWindows | RblxUIFocusedFlags_DockHierarchy),
            RblxUI::IsWindowFocused(RblxUIFocusedFlags_ChildWindows | RblxUIFocusedFlags_RootWindow),
            RblxUI::IsWindowFocused(RblxUIFocusedFlags_ChildWindows | RblxUIFocusedFlags_RootWindow | RblxUIFocusedFlags_NoPopupHierarchy),
            RblxUI::IsWindowFocused(RblxUIFocusedFlags_ChildWindows | RblxUIFocusedFlags_RootWindow | RblxUIFocusedFlags_DockHierarchy),
            RblxUI::IsWindowFocused(RblxUIFocusedFlags_RootWindow),
            RblxUI::IsWindowFocused(RblxUIFocusedFlags_RootWindow | RblxUIFocusedFlags_NoPopupHierarchy),
            RblxUI::IsWindowFocused(RblxUIFocusedFlags_RootWindow | RblxUIFocusedFlags_DockHierarchy),
            RblxUI::IsWindowFocused(RblxUIFocusedFlags_AnyWindow));

        // Testing IsWindowHovered() function with its various flags.
        RblxUI::BulletText(
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
            RblxUI::IsWindowHovered(),
            RblxUI::IsWindowHovered(RblxUIHoveredFlags_AllowWhenBlockedByPopup),
            RblxUI::IsWindowHovered(RblxUIHoveredFlags_AllowWhenBlockedByActiveItem),
            RblxUI::IsWindowHovered(RblxUIHoveredFlags_ChildWindows),
            RblxUI::IsWindowHovered(RblxUIHoveredFlags_ChildWindows | RblxUIHoveredFlags_NoPopupHierarchy),
            RblxUI::IsWindowHovered(RblxUIHoveredFlags_ChildWindows | RblxUIHoveredFlags_DockHierarchy),
            RblxUI::IsWindowHovered(RblxUIHoveredFlags_ChildWindows | RblxUIHoveredFlags_RootWindow),
            RblxUI::IsWindowHovered(RblxUIHoveredFlags_ChildWindows | RblxUIHoveredFlags_RootWindow | RblxUIHoveredFlags_NoPopupHierarchy),
            RblxUI::IsWindowHovered(RblxUIHoveredFlags_ChildWindows | RblxUIHoveredFlags_RootWindow | RblxUIHoveredFlags_DockHierarchy),
            RblxUI::IsWindowHovered(RblxUIHoveredFlags_RootWindow),
            RblxUI::IsWindowHovered(RblxUIHoveredFlags_RootWindow | RblxUIHoveredFlags_NoPopupHierarchy),
            RblxUI::IsWindowHovered(RblxUIHoveredFlags_RootWindow | RblxUIHoveredFlags_DockHierarchy),
            RblxUI::IsWindowHovered(RblxUIHoveredFlags_ChildWindows | RblxUIHoveredFlags_AllowWhenBlockedByPopup),
            RblxUI::IsWindowHovered(RblxUIHoveredFlags_AnyWindow),
            RblxUI::IsWindowHovered(RblxUIHoveredFlags_Stationary));

        RblxUI::BeginChild("child", RblxVec2(0, 50), RblxUIChildFlags_Borders);
        RblxUI::Text("This is another child window for testing the _ChildWindows flag.");
        RblxUI::EndChild();
        if (embed_all_inside_a_child_window)
            RblxUI::EndChild();

        // Calling IsItemHovered() after begin returns the hovered status of the title bar.
        // This is useful in particular if you want to create a context menu associated to the title bar of a window.
        // This will also work when docked into a Tab (the Tab replace the Title Bar and guarantee the same properties).
        static bool test_window = false;
        RblxUI::Checkbox("Hovered/Active tests after Begin() for title bar testing", &test_window);
        if (test_window)
        {
            // FIXME-DOCK: This window cannot be docked within the RblxUI Demo window, this will cause a feedback loop and get them stuck.
            // Could we fix this through an RblxUIWindowClass feature? Or an API call to tag our parent as "don't skip items"?
            RblxUI::Begin("Title bar Hovered/Active tests", &test_window);
            if (RblxUI::BeginPopupContextItem()) // <-- This is using IsItemHovered()
            {
                if (RblxUI::MenuItem("Close")) { test_window = false; }
                RblxUI::EndPopup();
            }
            RblxUI::Text(
                "IsItemHovered() after begin = %d (== is title bar hovered)\n"
                "IsItemActive() after begin = %d (== is window being clicked/moved)\n",
                RblxUI::IsItemHovered(), RblxUI::IsItemActive());
            RblxUI::End();
        }

        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsSelectables()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsSelectables()
{
    //RblxUI::SetNextItemOpen(true, RblxUICond_Once);
    if (RblxUI::TreeNode("Selectables"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Selectables");
        // Selectable() has 2 overloads:
        // - The one taking "bool selected" as a read-only selection information.
        //   When Selectable() has been clicked it returns true and you can alter selection state accordingly.
        // - The one taking "bool* p_selected" as a read-write selection information (convenient in some cases)
        // The earlier is more flexible, as in real application your selection may be stored in many different ways
        // and not necessarily inside a bool value (e.g. in flags within objects, as an external list, etc).
        RBLXUI_DEMO_MARKER("Widgets/Selectables/Basic");
        if (RblxUI::TreeNode("Basic"))
        {
            static bool selection[5] = { false, true, false, false };
            RblxUI::Selectable("1. I am selectable", &selection[0]);
            RblxUI::Selectable("2. I am selectable", &selection[1]);
            RblxUI::Selectable("3. I am selectable", &selection[2]);
            if (RblxUI::Selectable("4. I am double clickable", selection[3], RblxUISelectableFlags_AllowDoubleClick))
                if (RblxUI::IsMouseDoubleClicked(0))
                    selection[3] = !selection[3];
            RblxUI::TreePop();
        }

        RBLXUI_DEMO_MARKER("Widgets/Selectables/Rendering more items on the same line");
        if (RblxUI::TreeNode("Multiple items on the same line"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selectables/Multiple items on the same line");
            // - Using SetNextItemAllowOverlap()
            // - Using the Selectable() override that takes "bool* p_selected" parameter, the bool value is toggled automatically.
            {
                static bool selected[3] = {};
                RblxUI::SetNextItemAllowOverlap(); RblxUI::Selectable("main.c", &selected[0]); RblxUI::SameLine(); RblxUI::SmallButton("Link 1");
                RblxUI::SetNextItemAllowOverlap(); RblxUI::Selectable("hello.cpp", &selected[1]); RblxUI::SameLine(); RblxUI::SmallButton("Link 2");
                RblxUI::SetNextItemAllowOverlap(); RblxUI::Selectable("hello.h", &selected[2]); RblxUI::SameLine(); RblxUI::SmallButton("Link 3");
            }

            // (2)
            // - Using RblxUISelectableFlags_AllowOverlap is a shortcut for calling SetNextItemAllowOverlap()
            // - No visible label, display contents inside the selectable bounds.
            // - We don't maintain actual selection in this example to keep things simple.
            RblxUI::Spacing();
            {
                static bool checked[5] = {};
                static int selected_n = 0;
                const float color_marker_w = RblxUI::CalcTextSize("x").x;
                for (int n = 0; n < 5; n++)
                {
                    RblxUI::PushID(n);
                    RblxUI::AlignTextToFramePadding();
                    if (RblxUI::Selectable("##selectable", selected_n == n, RblxUISelectableFlags_AllowOverlap))
                        selected_n = n;
                    RblxUI::SameLine(0, 0);
                    RblxUI::Checkbox("##check", &checked[n]);
                    RblxUI::SameLine();
                    RblxVec4 color((n & 1) ? 1.0f : 0.2f, (n & 2) ? 1.0f : 0.2f, 0.2f, 1.0f);
                    RblxUI::ColorButton("##color", color, RblxUIColorEditFlags_NoTooltip, RblxVec2(color_marker_w, 0));
                    RblxUI::SameLine();
                    RblxUI::Text("Some label");
                    RblxUI::PopID();
                }
            }

            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("In Tables"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selectables/In Tables");
            static bool selected[10] = {};

            if (RblxUI::BeginTable("split1", 3, RblxUITableFlags_Resizable | RblxUITableFlags_NoSavedSettings | RblxUITableFlags_Borders))
            {
                for (int i = 0; i < 10; i++)
                {
                    char label[32];
                    sprintf(label, "Item %d", i);
                    RblxUI::TableNextColumn();
                    RblxUI::Selectable(label, &selected[i]); // FIXME-TABLE: Selection overlap
                }
                RblxUI::EndTable();
            }
            RblxUI::Spacing();
            if (RblxUI::BeginTable("split2", 3, RblxUITableFlags_Resizable | RblxUITableFlags_NoSavedSettings | RblxUITableFlags_Borders))
            {
                for (int i = 0; i < 10; i++)
                {
                    char label[32];
                    sprintf(label, "Item %d", i);
                    RblxUI::TableNextRow();
                    RblxUI::TableNextColumn();
                    RblxUI::Selectable(label, &selected[i], RblxUISelectableFlags_SpanAllColumns);
                    RblxUI::TableNextColumn();
                    RblxUI::Text("Some other contents");
                    RblxUI::TableNextColumn();
                    RblxUI::Text("123456");
                }
                RblxUI::EndTable();
            }
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Grid"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selectables/Grid");
            static char selected[4][4] = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };

            // Add in a bit of silly fun...
            const float time = (float)RblxUI::GetTime();
            const bool winning_state = memchr(selected, 0, sizeof(selected)) == NULL; // If all cells are selected...
            if (winning_state)
                RblxUI::PushStyleVar(RblxUIStyleVar_SelectableTextAlign, RblxVec2(0.5f + 0.5f * cosf(time * 2.0f), 0.5f + 0.5f * sinf(time * 3.0f)));

            const float size = RblxUI::CalcTextSize("Sailor").x;
            for (int y = 0; y < 4; y++)
                for (int x = 0; x < 4; x++)
                {
                    if (x > 0)
                        RblxUI::SameLine();
                    RblxUI::PushID(y * 4 + x);
                    if (RblxUI::Selectable("Sailor", selected[y][x] != 0, 0, RblxVec2(size, size)))
                    {
                        // Toggle clicked cell + toggle neighbors
                        selected[y][x] ^= 1;
                        if (x > 0) { selected[y][x - 1] ^= 1; }
                        if (x < 3) { selected[y][x + 1] ^= 1; }
                        if (y > 0) { selected[y - 1][x] ^= 1; }
                        if (y < 3) { selected[y + 1][x] ^= 1; }
                    }
                    RblxUI::PopID();
                }

            if (winning_state)
                RblxUI::PopStyleVar();
            RblxUI::TreePop();
        }
        if (RblxUI::TreeNode("Alignment"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selectables/Alignment");
            HelpMarker(
                "By default, Selectables uses style.SelectableTextAlign but it can be overridden on a per-item "
                "basis using PushStyleVar(). You'll probably want to always keep your default situation to "
                "left-align otherwise it becomes difficult to layout multiple items on a same line");

            static bool selected[3 * 3] = { true, false, true, false, true, false, true, false, true };
            const float size = RblxUI::CalcTextSize("(1.0,1.0)").x;
            for (int y = 0; y < 3; y++)
            {
                for (int x = 0; x < 3; x++)
                {
                    RblxVec2 alignment = RblxVec2((float)x / 2.0f, (float)y / 2.0f);
                    char name[32];
                    sprintf(name, "(%.1f,%.1f)", alignment.x, alignment.y);
                    if (x > 0) RblxUI::SameLine();
                    RblxUI::PushStyleVar(RblxUIStyleVar_SelectableTextAlign, alignment);
                    RblxUI::Selectable(name, &selected[3 * y + x], RblxUISelectableFlags_None, RblxVec2(size, size));
                    RblxUI::PopStyleVar();
                }
            }
            RblxUI::TreePop();
        }
        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsSelectionAndMultiSelect()
//-----------------------------------------------------------------------------
// Multi-selection demos
// Also read: https://github.com/ocornut/rblxui/wiki/Multi-Select
//-----------------------------------------------------------------------------

static const char* ExampleNames[] =
{
    "Artichoke", "Arugula", "Asparagus", "Avocado", "Bamboo Shoots", "Bean Sprouts", "Beans", "Beet", "Belgian Endive", "Bell Pepper",
    "Bitter Gourd", "Bok Choy", "Broccoli", "Brussels Sprouts", "Burdock Root", "Cabbage", "Calabash", "Capers", "Carrot", "Cassava",
    "Cauliflower", "Celery", "Celery Root", "Celcuce", "Chayote", "Chinese Broccoli", "Corn", "Cucumber"
};

// Extra functions to add deletion support to RblxUISelectionBasicStorage
struct ExampleSelectionWithDeletion : RblxUISelectionBasicStorage
{
    // Find which item should be Focused after deletion.
    // Call _before_ item submission. Return an index in the before-deletion item list, your item loop should call SetKeyboardFocusHere() on it.
    // The subsequent ApplyDeletionPostLoop() code will use it to apply Selection.
    // - We cannot provide this logic in core Dear RblxUI because we don't have access to selection data.
    // - We don't actually manipulate the RblxVector<> here, only in ApplyDeletionPostLoop(), but using similar API for consistency and flexibility.
    // - Important: Deletion only works if the underlying RblxUIID for your items are stable: aka not depend on their index, but on e.g. item id/ptr.
    // FIXME-MULTISELECT: Doesn't take account of the possibility focus target will be moved during deletion. Need refocus or scroll offset.
    int ApplyDeletionPreLoop(RblxUIMultiSelectIO* ms_io, int items_count)
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
        for (int idx = RBLXUI_MIN(focused_idx, items_count) - 1; idx >= 0; idx--)
            if (!Contains(GetStorageIdFromIndex(idx)))
                return idx;

        return -1;
    }

    // Rewrite item list (delete items) + update selection.
    // - Call after EndMultiSelect()
    // - We cannot provide this logic in core Dear RblxUI because we don't have access to your items, nor to selection data.
    template<typename ITEM_TYPE>
    void ApplyDeletionPostLoop(RblxUIMultiSelectIO* ms_io, RblxVector<ITEM_TYPE>& items, int item_curr_idx_to_select)
    {
        // Rewrite item list (delete items) + convert old selection index (before deletion) to new selection index (after selection).
        // If NavId was not part of selection, we will stay on same item.
        RblxVector<ITEM_TYPE> new_items;
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
    RblxVector<RblxUIID>           Items[2];               // ID is index into ExampleName[]
    RblxUISelectionBasicStorage  Selections[2];          // Store ExampleItemId into selection
    bool                        OptKeepSorted = true;

    void MoveAll(int src, int dst)
    {
        RBLXUI_ASSERT((src == 0 && dst == 1) || (src == 1 && dst == 0));
        for (RblxUIID item_id : Items[src])
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
            RblxUIID item_id = Items[src][src_n];
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
    void ApplySelectionRequests(RblxUIMultiSelectIO* ms_io, int side)
    {
        // In this example we store item id in selection (instead of item index)
        Selections[side].UserData = Items[side].Data;
        Selections[side].AdapterIndexToStorageId = [](RblxUISelectionBasicStorage* self, int idx) { RblxUIID* items = (RblxUIID*)self->UserData; return items[idx]; };
        Selections[side].ApplyRequests(ms_io);
    }
    static int RBLXUI_CDECL CompareItemsByValue(const void* lhs, const void* rhs)
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
        //if (RblxUI::Checkbox("Sorted", &OptKeepSorted) && OptKeepSorted) { SortItems(0); SortItems(1); }
        if (RblxUI::BeginTable("split", 3, RblxUITableFlags_None))
        {
            RblxUI::TableSetupColumn("", RblxUITableColumnFlags_WidthStretch);    // Left side
            RblxUI::TableSetupColumn("", RblxUITableColumnFlags_WidthFixed);      // Buttons
            RblxUI::TableSetupColumn("", RblxUITableColumnFlags_WidthStretch);    // Right side
            RblxUI::TableNextRow();

            int request_move_selected = -1;
            int request_move_all = -1;
            float child_height_0 = 0.0f;
            for (int side = 0; side < 2; side++)
            {
                // FIXME-MULTISELECT: Dual List Box: Add context menus
                // FIXME-NAV: Using RblxUIWindowFlags_NavFlattened exhibit many issues.
                RblxVector<RblxUIID>& items = Items[side];
                RblxUISelectionBasicStorage& selection = Selections[side];

                RblxUI::TableSetColumnIndex((side == 0) ? 0 : 2);
                RblxUI::Text("%s (%d)", (side == 0) ? "Available" : "Basket", items.Size);

                // Submit scrolling range to avoid glitches on moving/deletion
                const float items_height = RblxUI::GetTextLineHeightWithSpacing();
                RblxUI::SetNextWindowContentSize(RblxVec2(0.0f, items.Size * items_height));

                bool child_visible;
                if (side == 0)
                {
                    // Left child is resizable
                    RblxUI::SetNextWindowSizeConstraints(RblxVec2(0.0f, RblxUI::GetFrameHeightWithSpacing() * 4), RblxVec2(FLT_MAX, FLT_MAX));
                    child_visible = RblxUI::BeginChild("0", RblxVec2(-FLT_MIN, RblxUI::GetFontSize() * 20), RblxUIChildFlags_FrameStyle | RblxUIChildFlags_ResizeY);
                    child_height_0 = RblxUI::GetWindowSize().y;
                }
                else
                {
                    // Right child use same height as left one
                    child_visible = RblxUI::BeginChild("1", RblxVec2(-FLT_MIN, child_height_0), RblxUIChildFlags_FrameStyle);
                }
                if (child_visible)
                {
                    RblxUIMultiSelectFlags flags = RblxUIMultiSelectFlags_BoxSelect1d;
                    RblxUIMultiSelectIO* ms_io = RblxUI::BeginMultiSelect(flags, selection.Size, items.Size);
                    ApplySelectionRequests(ms_io, side);

                    for (int item_n = 0; item_n < items.Size; item_n++)
                    {
                        RblxUIID item_id = items[item_n];
                        bool item_is_selected = selection.Contains(item_id);
                        RblxUI::SetNextItemSelectionUserData(item_n);
                        RblxUI::Selectable(ExampleNames[item_id], item_is_selected, RblxUISelectableFlags_AllowDoubleClick);
                        if (RblxUI::IsItemFocused())
                        {
                            // FIXME-MULTISELECT: Dual List Box: Transfer focus
                            if (RblxUI::IsKeyPressed(RblxUIKey_Enter) || RblxUI::IsKeyPressed(RblxUIKey_KeypadEnter))
                                request_move_selected = side;
                            if (RblxUI::IsMouseDoubleClicked(0)) // FIXME-MULTISELECT: Double-click on multi-selection?
                                request_move_selected = side;
                        }
                    }

                    ms_io = RblxUI::EndMultiSelect();
                    ApplySelectionRequests(ms_io, side);
                }
                RblxUI::EndChild();
            }

            // Buttons columns
            RblxUI::TableSetColumnIndex(1);
            RblxUI::NewLine();
            //RblxVec2 button_sz = { RblxUI::CalcTextSize(">>").x + RblxUI::GetStyle().FramePadding.x * 2.0f, RblxUI::GetFrameHeight() + padding.y * 2.0f };
            RblxVec2 button_sz = { RblxUI::GetFrameHeight(), RblxUI::GetFrameHeight() };

            // (Using BeginDisabled()/EndDisabled() works but feels distracting given how it is currently visualized)
            if (RblxUI::Button(">>", button_sz))
                request_move_all = 0;
            if (RblxUI::Button(">", button_sz))
                request_move_selected = 0;
            if (RblxUI::Button("<", button_sz))
                request_move_selected = 1;
            if (RblxUI::Button("<<", button_sz))
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
                RblxUI::NewLine();
                if (RblxUI::ArrowButton("MoveUp", RblxUIDir_Up)) {}
                if (RblxUI::ArrowButton("MoveDown", RblxUIDir_Down)) {}
            }
            */

            RblxUI::EndTable();
        }
    }
};

static void DemoWindowWidgetsSelectionAndMultiSelect(RblxUIDemoWindowData* demo_data)
{
    if (RblxUI::TreeNode("Selection State & Multi-Select"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Selection State & Multi-Select");
        HelpMarker("Selections can be built using Selectable(), TreeNode() or other widgets. Selection state is owned by application code/data.");

        RblxUI::BulletText("Wiki page:");
        RblxUI::SameLine();
        RblxUI::TextLinkOpenURL("rblxui/wiki/Multi-Select", "https://github.com/ocornut/rblxui/wiki/Multi-Select");

        // Without any fancy API: manage single-selection yourself.
        if (RblxUI::TreeNode("Single-Select"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selection State/Single-Select");
            static int selected = -1;
            for (int n = 0; n < 5; n++)
            {
                char buf[32];
                sprintf(buf, "Object %d", n);
                if (RblxUI::Selectable(buf, selected == n))
                    selected = n;
            }
            RblxUI::TreePop();
        }

        // Demonstrate implementation a most-basic form of multi-selection manually
        // This doesn't support the Shift modifier which requires BeginMultiSelect()!
        if (RblxUI::TreeNode("Multi-Select (manual/simplified, without BeginMultiSelect)"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (manual/simplified, without BeginMultiSelect)");
            HelpMarker("Hold Ctrl and Click to select multiple items.");
            static bool selection[5] = { false, false, false, false, false };
            for (int n = 0; n < 5; n++)
            {
                char buf[32];
                sprintf(buf, "Object %d", n);
                if (RblxUI::Selectable(buf, selection[n]))
                {
                    if (!RblxUI::GetIO().KeyCtrl) // Clear selection when Ctrl is not held
                        memset(selection, 0, sizeof(selection));
                    selection[n] ^= 1; // Toggle current item
                }
            }
            RblxUI::TreePop();
        }

        // Demonstrate handling proper multi-selection using the BeginMultiSelect/EndMultiSelect API.
        // Shift+Click w/ Ctrl and other standard features are supported.
        // We use the RblxUISelectionBasicStorage helper which you may freely reimplement.
        if (RblxUI::TreeNode("Multi-Select"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selection State/Multi-Select");
            RblxUI::Text("Supported features:");
            RblxUI::BulletText("Keyboard navigation (arrows, page up/down, home/end, space).");
            RblxUI::BulletText("Ctrl modifier to preserve and toggle selection.");
            RblxUI::BulletText("Shift modifier for range selection.");
            RblxUI::BulletText("Ctrl+A to select all.");
            RblxUI::BulletText("Escape to clear selection.");
            RblxUI::BulletText("Click and drag to box-select.");
            RblxUI::Text("Tip: Use 'Demo->Tools->Debug Log->Selection' to see selection requests as they happen.");

            // Use default selection.Adapter: Pass index to SetNextItemSelectionUserData(), store index in Selection
            const int ITEMS_COUNT = 50;
            static RblxUISelectionBasicStorage selection;
            RblxUI::Text("Selection: %d/%d", selection.Size, ITEMS_COUNT);

            // The BeginChild() has no purpose for selection logic, other that offering a scrolling region.
            if (RblxUI::BeginChild("##Basket", RblxVec2(-FLT_MIN, RblxUI::GetFontSize() * 20), RblxUIChildFlags_FrameStyle | RblxUIChildFlags_ResizeY))
            {
                RblxUIMultiSelectFlags flags = RblxUIMultiSelectFlags_ClearOnEscape | RblxUIMultiSelectFlags_BoxSelect1d;
                RblxUIMultiSelectIO* ms_io = RblxUI::BeginMultiSelect(flags, selection.Size, ITEMS_COUNT);
                selection.ApplyRequests(ms_io);

                for (int n = 0; n < ITEMS_COUNT; n++)
                {
                    char label[64];
                    sprintf(label, "Object %05d: %s", n, ExampleNames[n % RBLXUI_COUNTOF(ExampleNames)]);
                    bool item_is_selected = selection.Contains((RblxUIID)n);
                    RblxUI::SetNextItemSelectionUserData(n);
                    RblxUI::Selectable(label, item_is_selected);
                }

                ms_io = RblxUI::EndMultiSelect();
                selection.ApplyRequests(ms_io);
            }
            RblxUI::EndChild();
            RblxUI::TreePop();
        }

        // Demonstrate using the clipper with BeginMultiSelect()/EndMultiSelect()
        if (RblxUI::TreeNode("Multi-Select (with clipper)"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (with clipper)");
            // Use default selection.Adapter: Pass index to SetNextItemSelectionUserData(), store index in Selection
            static RblxUISelectionBasicStorage selection;

            RblxUI::Text("Added features:");
            RblxUI::BulletText("Using RblxUIListClipper.");

            const int ITEMS_COUNT = 10000;
            RblxUI::Text("Selection: %d/%d", selection.Size, ITEMS_COUNT);
            if (RblxUI::BeginChild("##Basket", RblxVec2(-FLT_MIN, RblxUI::GetFontSize() * 20), RblxUIChildFlags_FrameStyle | RblxUIChildFlags_ResizeY))
            {
                RblxUIMultiSelectFlags flags = RblxUIMultiSelectFlags_ClearOnEscape | RblxUIMultiSelectFlags_BoxSelect1d;
                RblxUIMultiSelectIO* ms_io = RblxUI::BeginMultiSelect(flags, selection.Size, ITEMS_COUNT);
                selection.ApplyRequests(ms_io);

                RblxUIListClipper clipper;
                clipper.Begin(ITEMS_COUNT);
                if (ms_io->RangeSrcItem != -1)
                    clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem); // Ensure RangeSrc item is not clipped.
                while (clipper.Step())
                {
                    for (int n = clipper.DisplayStart; n < clipper.DisplayEnd; n++)
                    {
                        char label[64];
                        sprintf(label, "Object %05d: %s", n, ExampleNames[n % RBLXUI_COUNTOF(ExampleNames)]);
                        bool item_is_selected = selection.Contains((RblxUIID)n);
                        RblxUI::SetNextItemSelectionUserData(n);
                        RblxUI::Selectable(label, item_is_selected);
                    }
                }

                ms_io = RblxUI::EndMultiSelect();
                selection.ApplyRequests(ms_io);
            }
            RblxUI::EndChild();
            RblxUI::TreePop();
        }

        // Demonstrate dynamic item list + deletion support using the BeginMultiSelect/EndMultiSelect API.
        // In order to support Deletion without any glitches you need to:
        // - (1) If items are submitted in their own scrolling area, submit contents size SetNextWindowContentSize() ahead of time to prevent one-frame readjustment of scrolling.
        // - (2) Items needs to have persistent ID Stack identifier = ID needs to not depends on their index. PushID(index) = KO. PushID(item_id) = OK. This is in order to focus items reliably after a selection.
        // - (3) BeginXXXX process
        // - (4) Focus process
        // - (5) EndXXXX process
        if (RblxUI::TreeNode("Multi-Select (with deletion)"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (with deletion)");
            // Storing items data separately from selection data.
            // (you may decide to store selection data inside your item (aka intrusive storage) if you don't need multiple views over same items)
            // Use a custom selection.Adapter: store item identifier in Selection (instead of index)
            static RblxVector<RblxUIID> items;
            static ExampleSelectionWithDeletion selection;
            selection.UserData = (void*)&items;
            selection.AdapterIndexToStorageId = [](RblxUISelectionBasicStorage* self, int idx) { RblxVector<RblxUIID>* p_items = (RblxVector<RblxUIID>*)self->UserData; return (*p_items)[idx]; }; // Index -> ID

            RblxUI::Text("Added features:");
            RblxUI::BulletText("Dynamic list with Delete key support.");
            RblxUI::Text("Selection size: %d/%d", selection.Size, items.Size);

            // Initialize default list with 50 items + button to add/remove items.
            static RblxUIID items_next_id = 0;
            if (items_next_id == 0)
                for (RblxUIID n = 0; n < 50; n++)
                    items.push_back(items_next_id++);
            if (RblxUI::SmallButton("Add 20 items"))     { for (int n = 0; n < 20; n++) { items.push_back(items_next_id++); } }
            RblxUI::SameLine();
            if (RblxUI::SmallButton("Remove 20 items"))  { for (int n = RBLXUI_MIN(20, items.Size); n > 0; n--) { selection.SetItemSelected(items.back(), false); items.pop_back(); } }

            // (1) Extra to support deletion: Submit scrolling range to avoid glitches on deletion
            const float items_height = RblxUI::GetTextLineHeightWithSpacing();
            RblxUI::SetNextWindowContentSize(RblxVec2(0.0f, items.Size * items_height));

            if (RblxUI::BeginChild("##Basket", RblxVec2(-FLT_MIN, RblxUI::GetFontSize() * 20), RblxUIChildFlags_FrameStyle | RblxUIChildFlags_ResizeY))
            {
                RblxUIMultiSelectFlags flags = RblxUIMultiSelectFlags_ClearOnEscape | RblxUIMultiSelectFlags_BoxSelect1d;
                RblxUIMultiSelectIO* ms_io = RblxUI::BeginMultiSelect(flags, selection.Size, items.Size);
                selection.ApplyRequests(ms_io);

                const bool want_delete = RblxUI::Shortcut(RblxUIKey_Delete, RblxUIInputFlags_Repeat) && (selection.Size > 0);
                const int item_curr_idx_to_focus = want_delete ? selection.ApplyDeletionPreLoop(ms_io, items.Size) : -1;

                for (int n = 0; n < items.Size; n++)
                {
                    const RblxUIID item_id = items[n];
                    char label[64];
                    sprintf(label, "Object %05u: %s", item_id, ExampleNames[item_id % RBLXUI_COUNTOF(ExampleNames)]);

                    bool item_is_selected = selection.Contains(item_id);
                    RblxUI::SetNextItemSelectionUserData(n);
                    RblxUI::Selectable(label, item_is_selected);
                    if (item_curr_idx_to_focus == n)
                        RblxUI::SetKeyboardFocusHere(-1);
                }

                // Apply multi-select requests
                ms_io = RblxUI::EndMultiSelect();
                selection.ApplyRequests(ms_io);
                if (want_delete)
                    selection.ApplyDeletionPostLoop(ms_io, items, item_curr_idx_to_focus);
            }
            RblxUI::EndChild();
            RblxUI::TreePop();
        }

        // Implement a Dual List Box (#6648)
        if (RblxUI::TreeNode("Multi-Select (dual list box)"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (dual list box)");
            // Init default state
            static ExampleDualListBox dlb;
            if (dlb.Items[0].Size == 0 && dlb.Items[1].Size == 0)
                for (int item_id = 0; item_id < RBLXUI_COUNTOF(ExampleNames); item_id++)
                    dlb.Items[0].push_back((RblxUIID)item_id);

            // Show
            dlb.Show();

            RblxUI::TreePop();
        }

        // Demonstrate using the clipper with BeginMultiSelect()/EndMultiSelect()
        if (RblxUI::TreeNode("Multi-Select (in a table)"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (in a table)");
            static RblxUISelectionBasicStorage selection;

            const int ITEMS_COUNT = 10000;
            RblxUI::Text("Selection: %d/%d", selection.Size, ITEMS_COUNT);
            if (RblxUI::BeginTable("##Basket", 2, RblxUITableFlags_ScrollY | RblxUITableFlags_RowBg | RblxUITableFlags_BordersOuter, RblxVec2(0.0f, RblxUI::GetFontSize() * 20)))
            {
                RblxUI::TableSetupColumn("Object");
                RblxUI::TableSetupColumn("Action");
                RblxUI::TableSetupScrollFreeze(0, 1);
                RblxUI::TableHeadersRow();

                RblxUIMultiSelectFlags flags = RblxUIMultiSelectFlags_ClearOnEscape | RblxUIMultiSelectFlags_BoxSelect1d;
                RblxUIMultiSelectIO* ms_io = RblxUI::BeginMultiSelect(flags, selection.Size, ITEMS_COUNT);
                selection.ApplyRequests(ms_io);

                RblxUIListClipper clipper;
                clipper.Begin(ITEMS_COUNT);
                if (ms_io->RangeSrcItem != -1)
                    clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem); // Ensure RangeSrc item is not clipped.
                while (clipper.Step())
                {
                    for (int n = clipper.DisplayStart; n < clipper.DisplayEnd; n++)
                    {
                        RblxUI::TableNextRow();
                        RblxUI::TableNextColumn();
                        RblxUI::PushID(n);
                        char label[64];
                        sprintf(label, "Object %05d: %s", n, ExampleNames[n % RBLXUI_COUNTOF(ExampleNames)]);
                        bool item_is_selected = selection.Contains((RblxUIID)n);
                        RblxUI::SetNextItemSelectionUserData(n);
                        RblxUI::Selectable(label, item_is_selected, RblxUISelectableFlags_SpanAllColumns | RblxUISelectableFlags_AllowOverlap);
                        RblxUI::TableNextColumn();
                        RblxUI::SmallButton("hello");
                        RblxUI::PopID();
                    }
                }

                ms_io = RblxUI::EndMultiSelect();
                selection.ApplyRequests(ms_io);
                RblxUI::EndTable();
            }
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Multi-Select (checkboxes)"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (checkboxes)");
            RblxUI::Text("In a list of checkboxes (not selectable):");
            RblxUI::BulletText("Using _NoAutoSelect + _NoAutoClear flags.");
            RblxUI::BulletText("Shift+Click to check multiple boxes.");
            RblxUI::BulletText("Shift+Keyboard to copy current value to other boxes.");

            // If you have an array of checkboxes, you may want to use NoAutoSelect + NoAutoClear and the RblxUISelectionExternalStorage helper.
            static bool items[20] = {};
            static RblxUIMultiSelectFlags flags = RblxUIMultiSelectFlags_NoAutoSelect | RblxUIMultiSelectFlags_NoAutoClear | RblxUIMultiSelectFlags_ClearOnEscape;
            RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_NoAutoSelect", &flags, RblxUIMultiSelectFlags_NoAutoSelect);
            RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_NoAutoClear", &flags, RblxUIMultiSelectFlags_NoAutoClear);
            RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_BoxSelect2d", &flags, RblxUIMultiSelectFlags_BoxSelect2d); // Cannot use RblxUIMultiSelectFlags_BoxSelect1d as checkboxes are varying width.

            if (RblxUI::BeginChild("##Basket", RblxVec2(-FLT_MIN, RblxUI::GetFontSize() * 20), RblxUIChildFlags_Borders | RblxUIChildFlags_ResizeY))
            {
                RblxUIMultiSelectIO* ms_io = RblxUI::BeginMultiSelect(flags, -1, RBLXUI_COUNTOF(items));
                RblxUISelectionExternalStorage storage_wrapper;
                storage_wrapper.UserData = (void*)items;
                storage_wrapper.AdapterSetItemSelected = [](RblxUISelectionExternalStorage* self, int n, bool selected) { bool* array = (bool*)self->UserData; array[n] = selected; };
                storage_wrapper.ApplyRequests(ms_io);
                for (int n = 0; n < 20; n++)
                {
                    char label[32];
                    sprintf(label, "Item %d", n);
                    RblxUI::SetNextItemSelectionUserData(n);
                    RblxUI::Checkbox(label, &items[n]);
                }
                ms_io = RblxUI::EndMultiSelect();
                storage_wrapper.ApplyRequests(ms_io);
            }
            RblxUI::EndChild();

            RblxUI::TreePop();
        }

        // Demonstrate individual selection scopes in same window
        if (RblxUI::TreeNode("Multi-Select (multiple scopes)"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (multiple scopes)");
            // Use default select: Pass index to SetNextItemSelectionUserData(), store index in Selection
            const int SCOPES_COUNT = 3;
            const int ITEMS_COUNT = 8; // Per scope
            static RblxUISelectionBasicStorage selections_data[SCOPES_COUNT];

            // Use RblxUIMultiSelectFlags_ScopeRect to not affect other selections in same window.
            static RblxUIMultiSelectFlags flags = RblxUIMultiSelectFlags_ScopeRect | RblxUIMultiSelectFlags_ClearOnEscape;// | RblxUIMultiSelectFlags_ClearOnClickVoid;
            if (RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_ScopeWindow", &flags, RblxUIMultiSelectFlags_ScopeWindow) && (flags & RblxUIMultiSelectFlags_ScopeWindow))
                flags &= ~RblxUIMultiSelectFlags_ScopeRect;
            if (RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_ScopeRect", &flags, RblxUIMultiSelectFlags_ScopeRect) && (flags & RblxUIMultiSelectFlags_ScopeRect))
                flags &= ~RblxUIMultiSelectFlags_ScopeWindow;
            RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_ClearOnClickVoid", &flags, RblxUIMultiSelectFlags_ClearOnClickVoid);
            RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_BoxSelect1d", &flags, RblxUIMultiSelectFlags_BoxSelect1d);

            for (int selection_scope_n = 0; selection_scope_n < SCOPES_COUNT; selection_scope_n++)
            {
                RblxUI::PushID(selection_scope_n);
                RblxUISelectionBasicStorage* selection = &selections_data[selection_scope_n];
                RblxUIMultiSelectIO* ms_io = RblxUI::BeginMultiSelect(flags, selection->Size, ITEMS_COUNT);
                selection->ApplyRequests(ms_io);

                RblxUI::SeparatorText("Selection scope");
                RblxUI::Text("Selection size: %d/%d", selection->Size, ITEMS_COUNT);

                for (int n = 0; n < ITEMS_COUNT; n++)
                {
                    char label[64];
                    sprintf(label, "Object %05d: %s", n, ExampleNames[n % RBLXUI_COUNTOF(ExampleNames)]);
                    bool item_is_selected = selection->Contains((RblxUIID)n);
                    RblxUI::SetNextItemSelectionUserData(n);
                    RblxUI::Selectable(label, item_is_selected);
                }

                // Apply multi-select requests
                ms_io = RblxUI::EndMultiSelect();
                selection->ApplyRequests(ms_io);
                RblxUI::PopID();
            }
            RblxUI::TreePop();
        }

        // See ShowExampleAppAssetsBrowser()
        if (RblxUI::TreeNode("Multi-Select (tiled assets browser)"))
        {
            RblxUI::Checkbox("Assets Browser", &demo_data->ShowAppAssetsBrowser);
            RblxUI::Text("(also access from 'Examples->Assets Browser' in menu)");
            RblxUI::TreePop();
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
        if (RblxUI::TreeNode("Multi-Select (trees)"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (trees)");
            HelpMarker(
                "This is rather advanced and experimental. If you are getting started with multi-select, "
                "please don't start by looking at how to use it for a tree!\n\n"
                "Future versions will try to simplify and formalize some of this.");

            struct ExampleTreeFuncs
            {
                static void DrawNode(ExampleTreeNode* node, RblxUISelectionBasicStorage* selection)
                {
                    RblxUITreeNodeFlags tree_node_flags = RblxUITreeNodeFlags_SpanAvailWidth | RblxUITreeNodeFlags_OpenOnArrow | RblxUITreeNodeFlags_OpenOnDoubleClick;
                    tree_node_flags |= RblxUITreeNodeFlags_NavLeftJumpsToParent; // Enable pressing left to jump to parent
                    if (node->Childs.Size == 0)
                        tree_node_flags |= RblxUITreeNodeFlags_Bullet | RblxUITreeNodeFlags_Leaf;
                    if (selection->Contains((RblxUIID)node->UID))
                        tree_node_flags |= RblxUITreeNodeFlags_Selected;

                    // Using SetNextItemStorageID() to specify storage id, so we can easily peek into
                    // the storage holding open/close stage, using our TreeNodeGetOpen/TreeNodeSetOpen() functions.
                    RblxUI::SetNextItemSelectionUserData((RblxUISelectionUserData)(intptr_t)node);
                    RblxUI::SetNextItemStorageID((RblxUIID)node->UID);
                    if (RblxUI::TreeNodeEx(node->Name, tree_node_flags))
                    {
                        for (ExampleTreeNode* child : node->Childs)
                            DrawNode(child, selection);
                        RblxUI::TreePop();
                    }
                    else if (RblxUI::IsItemToggledOpen())
                    {
                        TreeCloseAndUnselectChildNodes(node, selection);
                    }
                }

                // When closing a node: 1) close and unselect all child nodes, 2) select parent if any child was selected.
                // FIXME: This is currently handled by user logic but I'm hoping to eventually provide tree node
                // features to do this automatically, e.g. a RblxUITreeNodeFlags_AutoCloseChildNodes etc.
                static int TreeCloseAndUnselectChildNodes(ExampleTreeNode* node, RblxUISelectionBasicStorage* selection, int depth = 0)
                {
                    // Recursive close (the test for depth == 0 is because we call this on a node that was just closed!)
                    int unselected_count = selection->Contains((RblxUIID)node->UID) ? 1 : 0;
                    if (depth == 0 || RblxUI::TreeNodeGetOpen((RblxUIID)node->UID))
                    {
                        for (ExampleTreeNode* child : node->Childs)
                            unselected_count += TreeCloseAndUnselectChildNodes(child, selection, depth + 1);
                        RblxUI::TreeNodeSetOpen((RblxUIID)node->UID, false);
                    }

                    // Select root node if any of its child was selected, otherwise unselect
                    selection->SetItemSelected((RblxUIID)node->UID, (depth == 0 && unselected_count > 0));
                    return unselected_count;
                }

                // Apply multi-selection requests
                static void ApplySelectionRequests(RblxUIMultiSelectIO* ms_io, ExampleTreeNode* tree, RblxUISelectionBasicStorage* selection)
                {
                    for (RblxUISelectionRequest& req : ms_io->Requests)
                    {
                        if (req.Type == RblxUISelectionRequestType_SetAll)
                        {
                            if (req.Selected)
                                TreeSetAllInOpenNodes(tree, selection, req.Selected);
                            else
                                selection->Clear();
                        }
                        else if (req.Type == RblxUISelectionRequestType_SetRange)
                        {
                            ExampleTreeNode* first_node = (ExampleTreeNode*)(intptr_t)req.RangeFirstItem;
                            ExampleTreeNode* last_node = (ExampleTreeNode*)(intptr_t)req.RangeLastItem;
                            for (ExampleTreeNode* node = first_node; node != NULL; node = TreeGetNextNodeInVisibleOrder(node, last_node))
                                selection->SetItemSelected((RblxUIID)node->UID, req.Selected);
                        }
                    }
                }

                static void TreeSetAllInOpenNodes(ExampleTreeNode* node, RblxUISelectionBasicStorage* selection, bool selected)
                {
                    if (node->Parent != NULL) // Root node isn't visible nor selectable in our scheme
                        selection->SetItemSelected((RblxUIID)node->UID, selected);
                    if (node->Parent == NULL || RblxUI::TreeNodeGetOpen((RblxUIID)node->UID))
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
                    if (curr_node->Childs.Size > 0 && RblxUI::TreeNodeGetOpen((RblxUIID)curr_node->UID))
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

            static RblxUISelectionBasicStorage selection;
            if (demo_data->DemoTree == NULL)
                demo_data->DemoTree = ExampleTree_CreateDemoTree(); // Create tree once
            RblxUI::Text("Selection size: %d", selection.Size);

            if (RblxUI::BeginChild("##Tree", RblxVec2(-FLT_MIN, RblxUI::GetFontSize() * 20), RblxUIChildFlags_FrameStyle | RblxUIChildFlags_ResizeY))
            {
                ExampleTreeNode* tree = demo_data->DemoTree;
                RblxUIMultiSelectFlags ms_flags = RblxUIMultiSelectFlags_ClearOnEscape | RblxUIMultiSelectFlags_BoxSelect2d;
                RblxUIMultiSelectIO* ms_io = RblxUI::BeginMultiSelect(ms_flags, selection.Size, -1);
                ExampleTreeFuncs::ApplySelectionRequests(ms_io, tree, &selection);
                for (ExampleTreeNode* node : tree->Childs)
                    ExampleTreeFuncs::DrawNode(node, &selection);
                ms_io = RblxUI::EndMultiSelect();
                ExampleTreeFuncs::ApplySelectionRequests(ms_io, tree, &selection);
            }
            RblxUI::EndChild();

            RblxUI::TreePop();
        }

        // Advanced demonstration of BeginMultiSelect()
        // - Showcase clipping.
        // - Showcase deletion.
        // - Showcase basic drag and drop.
        // - Showcase TreeNode variant (note that tree node don't expand in the demo: supporting expanding tree nodes + clipping a separate thing).
        // - Showcase using inside a table.
        //RblxUI::SetNextItemOpen(true, RblxUICond_Once);
        if (RblxUI::TreeNode("Multi-Select (advanced)"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (advanced)");
            // Options
            enum WidgetType { WidgetType_Selectable, WidgetType_TreeNode };
            static bool use_clipper = true;
            static bool use_deletion = true;
            static bool use_drag_drop = true;
            static bool show_in_table = false;
            static bool show_color_button = true;
            static RblxUIMultiSelectFlags flags = RblxUIMultiSelectFlags_ClearOnEscape | RblxUIMultiSelectFlags_BoxSelect1d;
            static WidgetType widget_type = WidgetType_Selectable;

            if (RblxUI::TreeNode("Options"))
            {
                if (RblxUI::RadioButton("Selectables", widget_type == WidgetType_Selectable)) { widget_type = WidgetType_Selectable; }
                RblxUI::SameLine();
                if (RblxUI::RadioButton("Tree nodes", widget_type == WidgetType_TreeNode)) { widget_type = WidgetType_TreeNode; }
                RblxUI::SameLine();
                HelpMarker("TreeNode() is technically supported but... using this correctly is more complicated (you need some sort of linear/random access to your tree, which is suited to advanced trees setups already implementing filters and clipper. We will work toward simplifying and demoing this.\n\nFor now the tree demo is actually a little bit meaningless because it is an empty tree with only root nodes.");
                RblxUI::Checkbox("Enable clipper", &use_clipper);
                RblxUI::Checkbox("Enable deletion", &use_deletion);
                RblxUI::Checkbox("Enable drag & drop", &use_drag_drop);
                RblxUI::Checkbox("Show in a table", &show_in_table);
                RblxUI::Checkbox("Show color button", &show_color_button);
                RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_SingleSelect", &flags, RblxUIMultiSelectFlags_SingleSelect);
                RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_NoSelectAll", &flags, RblxUIMultiSelectFlags_NoSelectAll);
                RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_NoRangeSelect", &flags, RblxUIMultiSelectFlags_NoRangeSelect);
                RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_NoAutoSelect", &flags, RblxUIMultiSelectFlags_NoAutoSelect);
                RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_NoAutoClear", &flags, RblxUIMultiSelectFlags_NoAutoClear);
                RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_NoAutoClearOnReselect", &flags, RblxUIMultiSelectFlags_NoAutoClearOnReselect);
                RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_NoSelectOnRightClick", &flags, RblxUIMultiSelectFlags_NoSelectOnRightClick);
                RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_BoxSelect1d", &flags, RblxUIMultiSelectFlags_BoxSelect1d);
                RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_BoxSelect2d", &flags, RblxUIMultiSelectFlags_BoxSelect2d);
                RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_BoxSelectNoScroll", &flags, RblxUIMultiSelectFlags_BoxSelectNoScroll);
                RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_ClearOnEscape", &flags, RblxUIMultiSelectFlags_ClearOnEscape);
                RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_ClearOnClickVoid", &flags, RblxUIMultiSelectFlags_ClearOnClickVoid);
                if (RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_ScopeWindow", &flags, RblxUIMultiSelectFlags_ScopeWindow) && (flags & RblxUIMultiSelectFlags_ScopeWindow))
                    flags &= ~RblxUIMultiSelectFlags_ScopeRect;
                if (RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_ScopeRect", &flags, RblxUIMultiSelectFlags_ScopeRect) && (flags & RblxUIMultiSelectFlags_ScopeRect))
                    flags &= ~RblxUIMultiSelectFlags_ScopeWindow;
                if (RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_SelectOnAuto", &flags, RblxUIMultiSelectFlags_SelectOnAuto))
                    flags &= ~(RblxUIMultiSelectFlags_SelectOnMask_ ^ RblxUIMultiSelectFlags_SelectOnAuto);
                RblxUI::SameLine(); HelpMarker("Apply selection on mouse down when clicking on unselected item, on mouse up when clicking on selected item. (Default)");
                if (RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_SelectOnClickAlways", &flags, RblxUIMultiSelectFlags_SelectOnClickAlways))
                    flags &= ~(RblxUIMultiSelectFlags_SelectOnMask_ ^ RblxUIMultiSelectFlags_SelectOnClickAlways);
                RblxUI::SameLine(); HelpMarker("Prevents Drag and Drop from being used on multi-selection, but allows e.g. BoxSelect to always reselect even when clicking inside an existing selection. (Excel style behavior)");
                if (RblxUI::CheckboxFlags("RblxUIMultiSelectFlags_SelectOnClickRelease", &flags, RblxUIMultiSelectFlags_SelectOnClickRelease))
                    flags &= ~(RblxUIMultiSelectFlags_SelectOnMask_ ^ RblxUIMultiSelectFlags_SelectOnClickRelease);
                RblxUI::SameLine(); HelpMarker("Allow dragging an unselected item without altering selection.");
                RblxUI::TreePop();
            }

            // Initialize default list with 1000 items.
            // Use default selection.Adapter: Pass index to SetNextItemSelectionUserData(), store index in Selection
            static RblxVector<int> items;
            static int items_next_id = 0;
            if (items_next_id == 0) { for (int n = 0; n < 1000; n++) { items.push_back(items_next_id++); } }
            static ExampleSelectionWithDeletion selection;
            static bool request_deletion_from_menu = false; // Queue deletion triggered from context menu

            RblxUI::Text("Selection size: %d/%d", selection.Size, items.Size);

            const float items_height = (widget_type == WidgetType_TreeNode) ? RblxUI::GetTextLineHeight() : RblxUI::GetTextLineHeightWithSpacing();
            RblxUI::SetNextWindowContentSize(RblxVec2(0.0f, items.Size * items_height));
            if (RblxUI::BeginChild("##Basket", RblxVec2(-FLT_MIN, RblxUI::GetFontSize() * 20), RblxUIChildFlags_FrameStyle | RblxUIChildFlags_ResizeY))
            {
                RblxVec2 color_button_sz(RblxUI::GetFontSize(), RblxUI::GetFontSize());
                if (widget_type == WidgetType_TreeNode)
                    RblxUI::PushStyleVarY(RblxUIStyleVar_ItemSpacing, 0.0f);

                RblxUIMultiSelectIO* ms_io = RblxUI::BeginMultiSelect(flags, selection.Size, items.Size);
                selection.ApplyRequests(ms_io);

                const bool want_delete = (RblxUI::Shortcut(RblxUIKey_Delete, RblxUIInputFlags_Repeat) && (selection.Size > 0)) || request_deletion_from_menu;
                const int item_curr_idx_to_focus = want_delete ? selection.ApplyDeletionPreLoop(ms_io, items.Size) : -1;
                request_deletion_from_menu = false;

                if (show_in_table)
                {
                    if (widget_type == WidgetType_TreeNode)
                        RblxUI::PushStyleVar(RblxUIStyleVar_CellPadding, RblxVec2(0.0f, 0.0f));
                    RblxUI::BeginTable("##Split", 2, RblxUITableFlags_Resizable | RblxUITableFlags_NoSavedSettings | RblxUITableFlags_NoPadOuterX);
                    RblxUI::TableSetupColumn("", RblxUITableColumnFlags_WidthStretch, 0.70f);
                    RblxUI::TableSetupColumn("", RblxUITableColumnFlags_WidthStretch, 0.30f);
                    //RblxUI::PushStyleVarY(RblxUIStyleVar_ItemSpacing, 0.0f);
                }

                RblxUIListClipper clipper;
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
                            RblxUI::TableNextColumn();

                        const int item_id = items[n];
                        const char* item_category = ExampleNames[item_id % RBLXUI_COUNTOF(ExampleNames)];
                        char label[64];
                        sprintf(label, "Object %05d: %s", item_id, item_category);

                        // IMPORTANT: for deletion refocus to work we need object ID to be stable,
                        // aka not depend on their index in the list. Here we use our persistent item_id
                        // instead of index to build a unique ID that will persist.
                        // (If we used PushID(index) instead, focus wouldn't be restored correctly after deletion).
                        RblxUI::PushID(item_id);

                        // Emit a color button, to test that Shift+LeftArrow landing on an item that is not part
                        // of the selection scope doesn't erroneously alter our selection.
                        if (show_color_button)
                        {
                            RblxU32 dummy_col = (RblxU32)((unsigned int)n * 0xC250B74B) | RBLXUI_COL32_A_MASK;
                            RblxUI::ColorButton("##", RblxColor(dummy_col), RblxUIColorEditFlags_NoTooltip, color_button_sz);
                            RblxUI::SameLine();
                        }

                        // Submit item
                        bool item_is_selected = selection.Contains((RblxUIID)n);
                        bool item_is_open = false;
                        RblxUI::SetNextItemSelectionUserData(n);
                        if (widget_type == WidgetType_Selectable)
                        {
                            RblxUI::Selectable(label, item_is_selected, RblxUISelectableFlags_None);
                        }
                        else if (widget_type == WidgetType_TreeNode)
                        {
                            RblxUITreeNodeFlags tree_node_flags = RblxUITreeNodeFlags_SpanAvailWidth | RblxUITreeNodeFlags_OpenOnArrow | RblxUITreeNodeFlags_OpenOnDoubleClick;
                            if (item_is_selected)
                                tree_node_flags |= RblxUITreeNodeFlags_Selected;
                            item_is_open = RblxUI::TreeNodeEx(label, tree_node_flags);
                        }

                        // Focus (for after deletion)
                        if (item_curr_idx_to_focus == n)
                            RblxUI::SetKeyboardFocusHere(-1);

                        // Drag and Drop
                        if (use_drag_drop && RblxUI::BeginDragDropSource())
                        {
                            // Create payload with full selection OR single unselected item.
                            // (the later is only possible when using RblxUIMultiSelectFlags_SelectOnClickRelease)
                            if (RblxUI::GetDragDropPayload() == NULL)
                            {
                                RblxVector<int> payload_items;
                                void* it = NULL;
                                RblxUIID id = 0;
                                if (!item_is_selected)
                                    payload_items.push_back(item_id);
                                else
                                    while (selection.GetNextSelectedItem(&it, &id))
                                        payload_items.push_back((int)id);
                                RblxUI::SetDragDropPayload("MULTISELECT_DEMO_ITEMS", payload_items.Data, (size_t)payload_items.size_in_bytes());
                            }

                            // Display payload content in tooltip
                            const RblxUIPayload* payload = RblxUI::GetDragDropPayload();
                            const int* payload_items = (int*)payload->Data;
                            const int payload_count = (int)payload->DataSize / (int)sizeof(int);
                            if (payload_count == 1)
                                RblxUI::Text("Object %05d: %s", payload_items[0], ExampleNames[payload_items[0] % RBLXUI_COUNTOF(ExampleNames)]);
                            else
                                RblxUI::Text("Dragging %d objects", payload_count);

                            RblxUI::EndDragDropSource();
                        }

                        if (widget_type == WidgetType_TreeNode && item_is_open)
                            RblxUI::TreePop();

                        // Right-click: context menu
                        if (RblxUI::BeginPopupContextItem())
                        {
                            RblxUI::BeginDisabled(!use_deletion || selection.Size == 0);
                            sprintf(label, "Delete %d item(s)###DeleteSelected", selection.Size);
                            if (RblxUI::Selectable(label))
                                request_deletion_from_menu = true;
                            RblxUI::EndDisabled();
                            RblxUI::Selectable("Close");
                            RblxUI::EndPopup();
                        }

                        // Demo content within a table
                        if (show_in_table)
                        {
                            RblxUI::TableNextColumn();
                            RblxUI::SetNextItemWidth(-FLT_MIN);
                            RblxUI::PushStyleVar(RblxUIStyleVar_FramePadding, RblxVec2(0, 0));
                            RblxUI::InputText("##NoLabel", (char*)(void*)item_category, strlen(item_category), RblxUIInputTextFlags_ReadOnly);
                            RblxUI::PopStyleVar();
                        }

                        RblxUI::PopID();
                    }
                    if (!use_clipper)
                        break;
                }

                if (show_in_table)
                {
                    RblxUI::EndTable();
                    if (widget_type == WidgetType_TreeNode)
                        RblxUI::PopStyleVar();
                }

                // Apply multi-select requests
                ms_io = RblxUI::EndMultiSelect();
                selection.ApplyRequests(ms_io);
                if (want_delete)
                    selection.ApplyDeletionPostLoop(ms_io, items, item_curr_idx_to_focus);

                if (widget_type == WidgetType_TreeNode)
                    RblxUI::PopStyleVar();
            }
            RblxUI::EndChild();
            RblxUI::TreePop();
        }
        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTabs()
//-----------------------------------------------------------------------------

static void EditTabBarFittingPolicyFlags(RblxUITabBarFlags* p_flags)
{
    if ((*p_flags & RblxUITabBarFlags_FittingPolicyMask_) == 0)
        *p_flags |= RblxUITabBarFlags_FittingPolicyDefault_;
    if (RblxUI::CheckboxFlags("RblxUITabBarFlags_FittingPolicyMixed", p_flags, RblxUITabBarFlags_FittingPolicyMixed))
        *p_flags &= ~(RblxUITabBarFlags_FittingPolicyMask_ ^ RblxUITabBarFlags_FittingPolicyMixed);
    if (RblxUI::CheckboxFlags("RblxUITabBarFlags_FittingPolicyShrink", p_flags, RblxUITabBarFlags_FittingPolicyShrink))
        *p_flags &= ~(RblxUITabBarFlags_FittingPolicyMask_ ^ RblxUITabBarFlags_FittingPolicyShrink);
    if (RblxUI::CheckboxFlags("RblxUITabBarFlags_FittingPolicyScroll", p_flags, RblxUITabBarFlags_FittingPolicyScroll))
        *p_flags &= ~(RblxUITabBarFlags_FittingPolicyMask_ ^ RblxUITabBarFlags_FittingPolicyScroll);
}

static void DemoWindowWidgetsTabs()
{
    if (RblxUI::TreeNode("Tabs"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Tabs");
        if (RblxUI::TreeNode("Basic"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Tabs/Basic");
            RblxUITabBarFlags tab_bar_flags = RblxUITabBarFlags_None;
            if (RblxUI::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                if (RblxUI::BeginTabItem("Avocado"))
                {
                    RblxUI::Text("This is the Avocado tab!\nblah blah blah blah blah");
                    RblxUI::EndTabItem();
                }
                if (RblxUI::BeginTabItem("Broccoli"))
                {
                    RblxUI::Text("This is the Broccoli tab!\nblah blah blah blah blah");
                    RblxUI::EndTabItem();
                }
                if (RblxUI::BeginTabItem("Cucumber"))
                {
                    RblxUI::Text("This is the Cucumber tab!\nblah blah blah blah blah");
                    RblxUI::EndTabItem();
                }
                RblxUI::EndTabBar();
            }
            RblxUI::Separator();
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Advanced & Close Button"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Tabs/Advanced & Close Button");
            // Expose a couple of the available flags. In most cases you may just call BeginTabBar() with no flags (0).
            static RblxUITabBarFlags tab_bar_flags = RblxUITabBarFlags_Reorderable;
            RblxUI::CheckboxFlags("RblxUITabBarFlags_Reorderable", &tab_bar_flags, RblxUITabBarFlags_Reorderable);
            RblxUI::CheckboxFlags("RblxUITabBarFlags_AutoSelectNewTabs", &tab_bar_flags, RblxUITabBarFlags_AutoSelectNewTabs);
            RblxUI::CheckboxFlags("RblxUITabBarFlags_TabListPopupButton", &tab_bar_flags, RblxUITabBarFlags_TabListPopupButton);
            RblxUI::CheckboxFlags("RblxUITabBarFlags_NoCloseWithMiddleMouseButton", &tab_bar_flags, RblxUITabBarFlags_NoCloseWithMiddleMouseButton);
            RblxUI::CheckboxFlags("RblxUITabBarFlags_DrawSelectedOverline", &tab_bar_flags, RblxUITabBarFlags_DrawSelectedOverline);
            EditTabBarFittingPolicyFlags(&tab_bar_flags);

            // Tab Bar
            RblxUI::AlignTextToFramePadding();
            RblxUI::Text("Opened:");
            const char* names[4] = { "Artichoke", "Beetroot", "Celery", "Daikon" };
            static bool opened[4] = { true, true, true, true }; // Persistent user state
            for (int n = 0; n < RBLXUI_COUNTOF(opened); n++)
            {
                RblxUI::SameLine();
                RblxUI::Checkbox(names[n], &opened[n]);
            }

            // Passing a bool* to BeginTabItem() is similar to passing one to Begin():
            // the underlying bool will be set to false when the tab is closed.
            if (RblxUI::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                for (int n = 0; n < RBLXUI_COUNTOF(opened); n++)
                    if (opened[n] && RblxUI::BeginTabItem(names[n], &opened[n], RblxUITabItemFlags_None))
                    {
                        RblxUI::Text("This is the %s tab!", names[n]);
                        if (n & 1)
                            RblxUI::Text("I am an odd tab.");
                        RblxUI::EndTabItem();
                    }
                RblxUI::EndTabBar();
            }
            RblxUI::Separator();
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("TabItemButton & Leading/Trailing flags"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Tabs/TabItemButton & Leading-Trailing flags");
            static RblxVector<int> active_tabs;
            static int next_tab_id = 0;
            if (next_tab_id == 0) // Initialize with some default tabs
                for (int i = 0; i < 3; i++)
                    active_tabs.push_back(next_tab_id++);

            // TabItemButton() and Leading/Trailing flags are distinct features which we will demo together.
            // (It is possible to submit regular tabs with Leading/Trailing flags, or TabItemButton tabs without Leading/Trailing flags...
            // but they tend to make more sense together)
            static bool show_leading_button = true;
            static bool show_trailing_button = true;
            RblxUI::Checkbox("Show Leading TabItemButton()", &show_leading_button);
            RblxUI::Checkbox("Show Trailing TabItemButton()", &show_trailing_button);

            // Expose some other flags which are useful to showcase how they interact with Leading/Trailing tabs
            static RblxUITabBarFlags tab_bar_flags = RblxUITabBarFlags_AutoSelectNewTabs | RblxUITabBarFlags_Reorderable | RblxUITabBarFlags_FittingPolicyShrink;
            EditTabBarFittingPolicyFlags(&tab_bar_flags);

            if (RblxUI::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                // Demo a Leading TabItemButton(): click the "?" button to open a menu
                if (show_leading_button)
                    if (RblxUI::TabItemButton("?", RblxUITabItemFlags_Leading | RblxUITabItemFlags_NoTooltip))
                        RblxUI::OpenPopup("MyHelpMenu");
                if (RblxUI::BeginPopup("MyHelpMenu"))
                {
                    RblxUI::Selectable("Hello!");
                    RblxUI::EndPopup();
                }

                // Demo Trailing Tabs: click the "+" button to add a new tab.
                // (In your app you may want to use a font icon instead of the "+")
                // We submit it before the regular tabs, but thanks to the RblxUITabItemFlags_Trailing flag it will always appear at the end.
                if (show_trailing_button)
                    if (RblxUI::TabItemButton("+", RblxUITabItemFlags_Trailing | RblxUITabItemFlags_NoTooltip))
                        active_tabs.push_back(next_tab_id++); // Add new tab

                // Submit our regular tabs
                for (int n = 0; n < active_tabs.Size; )
                {
                    bool open = true;
                    char name[16];
                    snprintf(name, RBLXUI_COUNTOF(name), "%04d", active_tabs[n]);
                    if (RblxUI::BeginTabItem(name, &open, RblxUITabItemFlags_None))
                    {
                        RblxUI::Text("This is the %s tab!", name);
                        RblxUI::EndTabItem();
                    }

                    if (!open)
                        active_tabs.erase(active_tabs.Data + n);
                    else
                        n++;
                }

                RblxUI::EndTabBar();
            }
            RblxUI::Separator();
            RblxUI::TreePop();
        }
        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsText()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsText()
{
    if (RblxUI::TreeNode("Text"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Text");
        if (RblxUI::TreeNode("Colorful Text"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Text/Colored Text");
            // Using shortcut. You can use PushStyleColor()/PopStyleColor() for more flexibility.
            RblxUI::TextColored(RblxVec4(1.0f, 0.0f, 1.0f, 1.0f), "Pink");
            RblxUI::TextColored(RblxVec4(1.0f, 1.0f, 0.0f, 1.0f), "Yellow");
            RblxUI::TextDisabled("Disabled");
            RblxUI::SameLine(); HelpMarker("The TextDisabled color is stored in RblxUIStyle.");
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Font Size"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Text/Font Size");
            RblxUIStyle& style = RblxUI::GetStyle();
            const float global_scale = style.FontScaleMain * style.FontScaleDpi;
            RblxUI::Text("style.FontScaleMain = %0.2f", style.FontScaleMain);
            RblxUI::Text("style.FontScaleDpi = %0.2f", style.FontScaleDpi);
            RblxUI::Text("global_scale = ~%0.2f", global_scale); // This is not technically accurate as internal scales may apply, but conceptually let's pretend it is.
            RblxUI::Text("FontSize = %0.2f", RblxUI::GetFontSize());

            RblxUI::SeparatorText("");
            static float custom_size = 16.0f;
            RblxUI::SliderFloat("custom_size", &custom_size, 10.0f, 100.0f, "%.0f");
            RblxUI::Text("RblxUI::PushFont(nullptr, custom_size);");
            RblxUI::PushFont(NULL, custom_size);
            RblxUI::Text("FontSize = %.2f (== %.2f * global_scale)", RblxUI::GetFontSize(), custom_size);
            RblxUI::PopFont();

            RblxUI::SeparatorText("");
            static float custom_scale = 1.0f;
            RblxUI::SliderFloat("custom_scale", &custom_scale, 0.5f, 4.0f, "%.2f");
            RblxUI::Text("RblxUI::PushFont(nullptr, style.FontSizeBase * custom_scale);");
            RblxUI::PushFont(NULL, style.FontSizeBase * custom_scale);
            RblxUI::Text("FontSize = %.2f (== style.FontSizeBase * %.2f * global_scale)", RblxUI::GetFontSize(), custom_scale);
            RblxUI::PopFont();

            RblxUI::SeparatorText("");
            for (float scaling = 0.5f; scaling <= 4.0f; scaling += 0.5f)
            {
                RblxUI::PushFont(NULL, style.FontSizeBase * scaling);
                RblxUI::Text("FontSize = %.2f (== style.FontSizeBase * %.2f * global_scale)", RblxUI::GetFontSize(), scaling);
                RblxUI::PopFont();
            }

            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Word Wrapping"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Text/Word Wrapping");
            // Using shortcut. You can use PushTextWrapPos()/PopTextWrapPos() for more flexibility.
            RblxUI::TextWrapped(
                "This text should automatically wrap on the edge of the window. The current implementation "
                "for text wrapping follows simple rules suitable for English and possibly other languages.");
            RblxUI::Spacing();

            static float wrap_width = 200.0f;
            RblxUI::SliderFloat("Wrap width", &wrap_width, -20, 600, "%.0f");

            RblxDrawList* draw_list = RblxUI::GetWindowDrawList();
            for (int n = 0; n < 2; n++)
            {
                RblxUI::Text("Test paragraph %d:", n);
                RblxVec2 pos = RblxUI::GetCursorScreenPos();
                RblxVec2 marker_min = RblxVec2(pos.x + wrap_width, pos.y);
                RblxVec2 marker_max = RblxVec2(pos.x + wrap_width + 10, pos.y + RblxUI::GetTextLineHeight());
                RblxUI::PushTextWrapPos(RblxUI::GetCursorPos().x + wrap_width);
                if (n == 0)
                    RblxUI::Text("The lazy dog is a good dog. This paragraph should fit within %.0f pixels. Testing a 1 character word. The quick brown fox jumps over the lazy dog.", wrap_width);
                else
                    RblxUI::Text("aaaaaaaa bbbbbbbb, c cccccccc,dddddddd. d eeeeeeee   ffffffff. gggggggg!hhhhhhhh");

                // Draw actual text bounding box, following by marker of our expected limit (should not overlap!)
                draw_list->AddRect(RblxUI::GetItemRectMin(), RblxUI::GetItemRectMax(), RBLXUI_COL32(255, 255, 0, 255));
                draw_list->AddRectFilled(marker_min, marker_max, RBLXUI_COL32(255, 0, 255, 255));
                RblxUI::PopTextWrapPos();
            }

            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("UTF-8 Text"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Text/UTF-8 Text");
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
            RblxUI::TextWrapped(
                "CJK text will only appear if the font was loaded with the appropriate CJK character ranges. "
                "Call io.Fonts->AddFontFromFileTTF() manually to load extra character ranges. "
                "Read docs/FONTS.md for details.");
            RblxUI::Text("Hiragana: \xe3\x81\x8b\xe3\x81\x8d\xe3\x81\x8f\xe3\x81\x91\xe3\x81\x93 (kakikukeko)");
            RblxUI::Text("Kanjis: \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e (nihongo)");
            static char buf[32] = "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e";
            //static char buf[32] = u8"NIHONGO"; // <- this is how you would write it with C++11, using real kanjis
            RblxUI::InputText("UTF-8 input", buf, RBLXUI_COUNTOF(buf));
            RblxUI::TreePop();
        }
        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTextFilter()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsTextFilter()
{
    if (RblxUI::TreeNode("Text Filter"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Text Filter");
        // Helper class to easy setup a text filter.
        // You may want to implement a more feature-full filtering scheme in your own application.
        HelpMarker("Not a widget per-se, but RblxUITextFilter is a helper to perform simple filtering on text strings.");
        static RblxUITextFilter filter;
        RblxUI::Text("Filter usage:\n"
            "  \"\"         display all lines\n"
            "  \"xxx\"      display lines containing \"xxx\"\n"
            "  \"xxx,yyy\"  display lines containing \"xxx\" or \"yyy\"\n"
            "  \"-xxx\"     hide lines containing \"xxx\"");
        filter.Draw();
        const char* lines[] = { "aaa1.c", "bbb1.c", "ccc1.c", "aaa2.cpp", "bbb2.cpp", "ccc2.cpp", "abc.h", "hello, world" };
        for (int i = 0; i < RBLXUI_COUNTOF(lines); i++)
            if (filter.PassFilter(lines[i]))
                RblxUI::BulletText("%s", lines[i]);
        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTextInput()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsTextInput()
{
    // To wire InputText() with std::string or any other custom string type,
    // see the "Text Input > Resize Callback" section of this demo, and the misc/cpp/rblxui_stdlib.h file.
    if (RblxUI::TreeNode("Text Input"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Text Input");
        if (RblxUI::TreeNode("Multi-line Text Input"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Text Input/Multi-line Text Input");
            // WE ARE USING A FIXED-SIZE BUFFER FOR SIMPLICITY HERE.
            // If you want to use InputText() with std::string or any custom dynamic string type:
            // - For std::string: use the wrapper in misc/cpp/rblxui_stdlib.h/.cpp
            // - Otherwise, see the 'Dear RblxUI Demo->Widgets->Text Input->Resize Callback' for using RblxUIInputTextFlags_CallbackResize.
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

            static RblxUIInputTextFlags flags = RblxUIInputTextFlags_AllowTabInput;
            HelpMarker("You can use the RblxUIInputTextFlags_CallbackResize facility if you need to wire InputTextMultiline() to a dynamic string type. See misc/cpp/rblxui_stdlib.h for an example. (This is not demonstrated in rblxui_demo.cpp because we don't want to include <string> in here)");
            RblxUI::CheckboxFlags("RblxUIInputTextFlags_ReadOnly", &flags, RblxUIInputTextFlags_ReadOnly);
            RblxUI::CheckboxFlags("RblxUIInputTextFlags_WordWrap", &flags, RblxUIInputTextFlags_WordWrap);
            RblxUI::SameLine(); HelpMarker("Feature is currently in Beta. Please read comments in rblxui.h");
            RblxUI::CheckboxFlags("RblxUIInputTextFlags_AllowTabInput", &flags, RblxUIInputTextFlags_AllowTabInput);
            RblxUI::SameLine(); HelpMarker("When _AllowTabInput is set, passing through the widget with Tabbing doesn't automatically activate it, in order to also cycling through subsequent widgets.");
            RblxUI::CheckboxFlags("RblxUIInputTextFlags_CtrlEnterForNewLine", &flags, RblxUIInputTextFlags_CtrlEnterForNewLine);
            RblxUI::InputTextMultiline("##source", text, RBLXUI_COUNTOF(text), RblxVec2(-FLT_MIN, RblxUI::GetTextLineHeight() * 16), flags);
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Filtered Text Input"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Text Input/Filtered Text Input");
            struct TextFilters
            {
                // Modify character input by altering 'data->Eventchar' (RblxUIInputTextFlags_CallbackCharFilter callback)
                static int FilterCasingSwap(RblxUIInputTextCallbackData* data)
                {
                    if (data->EventChar >= 'a' && data->EventChar <= 'z') { data->EventChar -= 'a' - 'A'; } // Lowercase becomes uppercase
                    else if (data->EventChar >= 'A' && data->EventChar <= 'Z') { data->EventChar += 'a' - 'A'; } // Uppercase becomes lowercase
                    return 0;
                }

                // Return 0 (pass) if the character is 'i' or 'm' or 'g' or 'u' or 'i', otherwise return 1 (filter out)
                static int FilterRblxUILetters(RblxUIInputTextCallbackData* data)
                {
                    if (data->EventChar < 256 && strchr("rblxui", (char)data->EventChar))
                        return 0;
                    return 1;
                }
            };

            static char buf1[32] = ""; RblxUI::InputText("default", buf1, RBLXUI_COUNTOF(buf1));
            static char buf2[32] = ""; RblxUI::InputText("decimal", buf2, RBLXUI_COUNTOF(buf2), RblxUIInputTextFlags_CharsDecimal);
            static char buf3[32] = ""; RblxUI::InputText("hexadecimal", buf3, RBLXUI_COUNTOF(buf3), RblxUIInputTextFlags_CharsHexadecimal | RblxUIInputTextFlags_CharsUppercase);
            static char buf4[32] = ""; RblxUI::InputText("uppercase", buf4, RBLXUI_COUNTOF(buf4), RblxUIInputTextFlags_CharsUppercase);
            static char buf5[32] = ""; RblxUI::InputText("no blank", buf5, RBLXUI_COUNTOF(buf5), RblxUIInputTextFlags_CharsNoBlank);
            static char buf6[32] = ""; RblxUI::InputText("casing swap", buf6, RBLXUI_COUNTOF(buf6), RblxUIInputTextFlags_CallbackCharFilter, TextFilters::FilterCasingSwap); // Use CharFilter callback to replace characters.
            static char buf7[32] = ""; RblxUI::InputText("\"rblxui\"", buf7, RBLXUI_COUNTOF(buf7), RblxUIInputTextFlags_CallbackCharFilter, TextFilters::FilterRblxUILetters); // Use CharFilter callback to disable some characters.
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Password Input"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Text Input/Password input");
            static char password[64] = "password123";
            RblxUI::InputText("password", password, RBLXUI_COUNTOF(password), RblxUIInputTextFlags_Password);
            RblxUI::SameLine(); HelpMarker("Display all characters as '*'.\nDisable clipboard cut and copy.\nDisable logging.\n");
            RblxUI::InputTextWithHint("password (w/ hint)", "<password>", password, RBLXUI_COUNTOF(password), RblxUIInputTextFlags_Password);
            RblxUI::InputText("password (clear)", password, RBLXUI_COUNTOF(password));
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Completion, History, Edit Callbacks"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Text Input/Completion, History, Edit Callbacks");
            struct Funcs
            {
                static int MyCallback(RblxUIInputTextCallbackData* data)
                {
                    if (data->EventFlag == RblxUIInputTextFlags_CallbackCompletion)
                    {
                        data->InsertChars(data->CursorPos, "..");
                    }
                    else if (data->EventFlag == RblxUIInputTextFlags_CallbackHistory)
                    {
                        if (data->EventKey == RblxUIKey_UpArrow)
                        {
                            data->DeleteChars(0, data->BufTextLen);
                            data->InsertChars(0, "Pressed Up!");
                            data->SelectAll();
                        }
                        else if (data->EventKey == RblxUIKey_DownArrow)
                        {
                            data->DeleteChars(0, data->BufTextLen);
                            data->InsertChars(0, "Pressed Down!");
                            data->SelectAll();
                        }
                    }
                    else if (data->EventFlag == RblxUIInputTextFlags_CallbackEdit)
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
            RblxUI::InputText("Completion", buf1, RBLXUI_COUNTOF(buf1), RblxUIInputTextFlags_CallbackCompletion, Funcs::MyCallback);
            RblxUI::SameLine(); HelpMarker(
                "Here we append \"..\" each time Tab is pressed. "
                "See 'Examples>Console' for a more meaningful demonstration of using this callback.");

            static char buf2[64];
            RblxUI::InputText("History", buf2, RBLXUI_COUNTOF(buf2), RblxUIInputTextFlags_CallbackHistory, Funcs::MyCallback);
            RblxUI::SameLine(); HelpMarker(
                "Here we replace and select text each time Up/Down are pressed. "
                "See 'Examples>Console' for a more meaningful demonstration of using this callback.");

            static char buf3[64];
            static int edit_count = 0;
            RblxUI::InputText("Edit", buf3, RBLXUI_COUNTOF(buf3), RblxUIInputTextFlags_CallbackEdit, Funcs::MyCallback, (void*)&edit_count);
            RblxUI::SameLine(); HelpMarker(
                "Here we toggle the casing of the first character on every edit + count edits.");
            RblxUI::SameLine(); RblxUI::Text("(%d)", edit_count);

            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Resize Callback"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Text Input/Resize Callback");
            // To wire InputText() with std::string or any other custom string type,
            // you can use the RblxUIInputTextFlags_CallbackResize flag + create a custom RblxUI::InputText() wrapper
            // using your preferred type. See misc/cpp/rblxui_stdlib.h for an implementation of this using std::string.
            HelpMarker(
                "Using RblxUIInputTextFlags_CallbackResize to wire your custom string type to InputText().\n\n"
                "See misc/cpp/rblxui_stdlib.h for an implementation of this for std::string.");
            struct Funcs
            {
                static int MyResizeCallback(RblxUIInputTextCallbackData* data)
                {
                    if (data->EventFlag == RblxUIInputTextFlags_CallbackResize)
                    {
                        RblxVector<char>* my_str = (RblxVector<char>*)data->UserData;
                        RBLXUI_ASSERT(my_str->begin() == data->Buf);
                        my_str->resize(data->BufSize); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
                        data->Buf = my_str->begin();
                    }
                    return 0;
                }

                // Note: Because RblxUI:: is a namespace you would typically add your own function into the namespace.
                // For example, you code may declare a function 'RblxUI::InputText(const char* label, MyString* my_str)'
                static bool MyInputTextMultiline(const char* label, RblxVector<char>* my_str, const RblxVec2& size = RblxVec2(0, 0), RblxUIInputTextFlags flags = 0)
                {
                    RBLXUI_ASSERT((flags & RblxUIInputTextFlags_CallbackResize) == 0);
                    return RblxUI::InputTextMultiline(label, my_str->begin(), (size_t)my_str->size(), size, flags | RblxUIInputTextFlags_CallbackResize, Funcs::MyResizeCallback, (void*)my_str);
                }
            };

            // For this demo we are using RblxVector as a string container.
            // Note that because we need to store a terminating zero character, our size/capacity are 1 more
            // than usually reported by a typical string class.
            static RblxUIInputTextFlags flags = RblxUIInputTextFlags_None;
            RblxUI::CheckboxFlags("RblxUIInputTextFlags_WordWrap", &flags, RblxUIInputTextFlags_WordWrap);

            static RblxVector<char> my_str;
            if (my_str.empty())
                my_str.push_back(0);
            Funcs::MyInputTextMultiline("##MyStr", &my_str, RblxVec2(-FLT_MIN, RblxUI::GetTextLineHeight() * 16), flags);
            RblxUI::Text("Data: %p\nSize: %d\nCapacity: %d", (void*)my_str.begin(), my_str.size(), my_str.capacity());
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Eliding, Alignment"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Text Input/Eliding, Alignment");
            static char buf1[128] = "/path/to/some/folder/with/long/filename.cpp";
            static RblxUIInputTextFlags flags = RblxUIInputTextFlags_ElideLeft;
            RblxUI::CheckboxFlags("RblxUIInputTextFlags_ElideLeft", &flags, RblxUIInputTextFlags_ElideLeft);
            RblxUI::InputText("Path", buf1, RBLXUI_COUNTOF(buf1), flags);
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Miscellaneous"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Text Input/Miscellaneous");
            static char buf1[16];
            static RblxUIInputTextFlags flags = RblxUIInputTextFlags_EscapeClearsAll;
            RblxUI::CheckboxFlags("RblxUIInputTextFlags_EscapeClearsAll", &flags, RblxUIInputTextFlags_EscapeClearsAll);
            RblxUI::CheckboxFlags("RblxUIInputTextFlags_ReadOnly", &flags, RblxUIInputTextFlags_ReadOnly);
            RblxUI::CheckboxFlags("RblxUIInputTextFlags_NoUndoRedo", &flags, RblxUIInputTextFlags_NoUndoRedo);
            RblxUI::InputText("Hello", buf1, RBLXUI_COUNTOF(buf1), flags);
            RblxUI::TreePop();
        }

        RblxUI::TreePop();
    }

}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTooltips()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsTooltips()
{
    if (RblxUI::TreeNode("Tooltips"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Tooltips");
        // Tooltips are windows following the mouse. They do not take focus away.
        RblxUI::SeparatorText("General");

        // Typical use cases:
        // - Short-form (text only):      SetItemTooltip("Hello");
        // - Short-form (any contents):   if (BeginItemTooltip()) { Text("Hello"); EndTooltip(); }

        // - Full-form (text only):       if (IsItemHovered(...)) { SetTooltip("Hello"); }
        // - Full-form (any contents):    if (IsItemHovered(...) && BeginTooltip()) { Text("Hello"); EndTooltip(); }

        HelpMarker(
            "Tooltip are typically created by using a IsItemHovered() + SetTooltip() sequence.\n\n"
            "We provide a helper SetItemTooltip() function to perform the two with standards flags.");

        RblxVec2 sz = RblxVec2(-FLT_MIN, 0.0f);

        RblxUI::Button("Basic", sz);
        RblxUI::SetItemTooltip("I am a tooltip");

        RblxUI::Button("Fancy", sz);
        if (RblxUI::BeginItemTooltip())
        {
            RblxUI::Text("I am a fancy tooltip");
            static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
            RblxUI::PlotLines("Curve", arr, RBLXUI_COUNTOF(arr));
            RblxUI::Text("Sin(time) = %f", sinf((float)RblxUI::GetTime()));
            RblxUI::EndTooltip();
        }

        RblxUI::SeparatorText("Always On");

        // Showcase NOT relying on a IsItemHovered() to emit a tooltip.
        // Here the tooltip is always emitted when 'always_on == true'.
        static int always_on = 0;
        RblxUI::RadioButton("Off", &always_on, 0);
        RblxUI::SameLine();
        RblxUI::RadioButton("Always On (Simple)", &always_on, 1);
        RblxUI::SameLine();
        RblxUI::RadioButton("Always On (Advanced)", &always_on, 2);
        if (always_on == 1)
            RblxUI::SetTooltip("I am following you around.");
        else if (always_on == 2 && RblxUI::BeginTooltip())
        {
            RblxUI::ProgressBar(sinf((float)RblxUI::GetTime()) * 0.5f + 0.5f, RblxVec2(RblxUI::GetFontSize() * 25, 0.0f));
            RblxUI::EndTooltip();
        }

        RblxUI::SeparatorText("Custom");

        HelpMarker(
            "Passing RblxUIHoveredFlags_ForTooltip to IsItemHovered() is the preferred way to standardize "
            "tooltip activation details across your application. You may however decide to use custom "
            "flags for a specific tooltip instance.");

        // The following examples are passed for documentation purpose but may not be useful to most users.
        // Passing RblxUIHoveredFlags_ForTooltip to IsItemHovered() will pull RblxUIHoveredFlags flags values from
        // 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav' depending on whether mouse or keyboard/gamepad is being used.
        // With default settings, RblxUIHoveredFlags_ForTooltip is equivalent to RblxUIHoveredFlags_DelayShort + RblxUIHoveredFlags_Stationary.
        RblxUI::Button("Manual", sz);
        if (RblxUI::IsItemHovered(RblxUIHoveredFlags_ForTooltip))
            RblxUI::SetTooltip("I am a manually emitted tooltip.");

        RblxUI::Button("DelayNone", sz);
        if (RblxUI::IsItemHovered(RblxUIHoveredFlags_DelayNone))
            RblxUI::SetTooltip("I am a tooltip with no delay.");

        RblxUI::Button("DelayShort", sz);
        if (RblxUI::IsItemHovered(RblxUIHoveredFlags_DelayShort | RblxUIHoveredFlags_NoSharedDelay))
            RblxUI::SetTooltip("I am a tooltip with a short delay (%0.2f sec).", RblxUI::GetStyle().HoverDelayShort);

        RblxUI::Button("DelayLong", sz);
        if (RblxUI::IsItemHovered(RblxUIHoveredFlags_DelayNormal | RblxUIHoveredFlags_NoSharedDelay))
            RblxUI::SetTooltip("I am a tooltip with a long delay (%0.2f sec).", RblxUI::GetStyle().HoverDelayNormal);

        RblxUI::Button("Stationary", sz);
        if (RblxUI::IsItemHovered(RblxUIHoveredFlags_Stationary))
            RblxUI::SetTooltip("I am a tooltip requiring mouse to be stationary before activating.");

        // Using RblxUIHoveredFlags_ForTooltip will pull flags from 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav',
        // which default value include the RblxUIHoveredFlags_AllowWhenDisabled flag.
        RblxUI::BeginDisabled();
        RblxUI::Button("Disabled item", sz);
        if (RblxUI::IsItemHovered(RblxUIHoveredFlags_ForTooltip))
            RblxUI::SetTooltip("I am a tooltip for a disabled item.");
        RblxUI::EndDisabled();

        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTreeNodes()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsTreeNodes()
{
    if (RblxUI::TreeNode("Tree Nodes"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Tree Nodes");
        // See see "Examples -> Property Editor" (ShowExampleAppPropertyEditor() function) for a fancier, data-driven tree.
        if (RblxUI::TreeNode("Basic Trees"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Tree Nodes/Basic Trees");
            for (int i = 0; i < 5; i++)
            {
                // Use SetNextItemOpen() so set the default state of a node to be open. We could
                // also use TreeNodeEx() with the RblxUITreeNodeFlags_DefaultOpen flag to achieve the same thing!
                if (i == 0)
                    RblxUI::SetNextItemOpen(true, RblxUICond_Once);

                // Here we use PushID() to generate a unique base ID, and then the "" used as TreeNode id won't conflict.
                // An alternative to using 'PushID() + TreeNode("", ...)' to generate a unique ID is to use 'TreeNode((void*)(intptr_t)i, ...)',
                // aka generate a dummy pointer-sized value to be hashed. The demo below uses that technique. Both are fine.
                RblxUI::PushID(i);
                if (RblxUI::TreeNode("", "Child %d", i))
                {
                    RblxUI::Text("blah blah");
                    RblxUI::SameLine();
                    if (RblxUI::SmallButton("button")) {}
                    RblxUI::TreePop();
                }
                RblxUI::PopID();
            }
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Hierarchy Lines"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Tree Nodes/Hierarchy Lines");
            static RblxUITreeNodeFlags base_flags = RblxUITreeNodeFlags_DrawLinesFull | RblxUITreeNodeFlags_DefaultOpen;
            HelpMarker("Default option for DrawLinesXXX is stored in style.TreeLinesFlags");
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_DrawLinesNone", &base_flags, RblxUITreeNodeFlags_DrawLinesNone);
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_DrawLinesFull", &base_flags, RblxUITreeNodeFlags_DrawLinesFull);
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_DrawLinesToNodes", &base_flags, RblxUITreeNodeFlags_DrawLinesToNodes);

            if (RblxUI::TreeNodeEx("Parent", base_flags))
            {
                if (RblxUI::TreeNodeEx("Child 1", base_flags))
                {
                    RblxUI::Button("Button for Child 1");
                    RblxUI::TreePop();
                }
                if (RblxUI::TreeNodeEx("Child 2", base_flags))
                {
                    RblxUI::Button("Button for Child 2");
                    RblxUI::TreePop();
                }
                RblxUI::Text("Remaining contents");
                RblxUI::Text("Remaining contents");
                RblxUI::TreePop();
            }

            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Clipping Large Trees"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Tree Nodes/Clipping Large Trees");
            RblxUI::TextWrapped(
                "- Using RblxUIListClipper with trees is a less easy than on arrays or grids.\n"
                "- Refer to 'Demo->Examples->Property Editor' for an example of how to do that.\n"
                "- Discuss in #3823");
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Selectable Nodes"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Tree Nodes/Selectable Nodes");
            HelpMarker(
                "Manually implemented selectable nodes.\n"
                "Click to select, Ctrl+Click to toggle, click on arrows or double-click to open.\n\n"
                "You may also use the multi-select API (see 'Demo->Widgets->Selection State & Multi-Select') for more advanced multi-selection features.");

            // Hold in 'selection_mask' a simple representation of what may be user-side selection state.
            // - You may retain selection state inside or outside your objects in whatever format you see fit.
            //   You may use RblxUISelectionBasicStorage which is conceptually close to a set<> of identifiers.
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
                RblxUITreeNodeFlags flags = RblxUITreeNodeFlags_OpenOnArrow | RblxUITreeNodeFlags_OpenOnDoubleClick | RblxUITreeNodeFlags_SpanAvailWidth;
                if (selection_mask & (1 << node_n))
                    flags |= RblxUITreeNodeFlags_Selected;

                bool is_open = RblxUI::TreeNodeEx((void*)(intptr_t)node_n, flags, "Selectable Node %d", node_n);
                if (RblxUI::IsItemClicked() && !RblxUI::IsItemToggledOpen())
                    node_clicked_idx = node_n;
                if (is_open)
                {
                    RblxUI::BulletText("<Node contents here>");
                    RblxUI::TreePop();
                }
            }
            if (node_clicked_idx != -1)
            {
                // Update selection state (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
                if (RblxUI::GetIO().KeyCtrl)
                    selection_mask ^= (1 << node_clicked_idx);          // Ctrl+Click to toggle
                else //if (!(selection_mask & (1 << node_clicked_idx))) // Depending on selection behavior you want, may want to preserve selection when clicking on item that is part of the selection
                    selection_mask = (1 << node_clicked_idx);           // Click to single-select
            }
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Advanced"))
        {
            RBLXUI_DEMO_MARKER("Widgets/Tree Nodes/Advanced");
            static RblxUITreeNodeFlags base_flags = RblxUITreeNodeFlags_OpenOnArrow | RblxUITreeNodeFlags_OpenOnDoubleClick | RblxUITreeNodeFlags_SpanAvailWidth;
            static bool align_label_with_current_x_position = false;
            static bool use_drag_and_drop = false;
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_OpenOnArrow", &base_flags, RblxUITreeNodeFlags_OpenOnArrow);
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_OpenOnDoubleClick", &base_flags, RblxUITreeNodeFlags_OpenOnDoubleClick);
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_SpanAvailWidth", &base_flags, RblxUITreeNodeFlags_SpanAvailWidth); RblxUI::SameLine(); HelpMarker("Extend hit area to all available width instead of allowing more items to be laid out after the node.");
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_SpanFullWidth", &base_flags, RblxUITreeNodeFlags_SpanFullWidth);
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_SpanLabelWidth", &base_flags, RblxUITreeNodeFlags_SpanLabelWidth); RblxUI::SameLine(); HelpMarker("Reduce hit area to the text label and a bit of margin.");
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_SpanAllColumns", &base_flags, RblxUITreeNodeFlags_SpanAllColumns); RblxUI::SameLine(); HelpMarker("For use in Tables only.");
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_AllowOverlap", &base_flags, RblxUITreeNodeFlags_AllowOverlap);
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_Framed", &base_flags, RblxUITreeNodeFlags_Framed); RblxUI::SameLine(); HelpMarker("Draw frame with background (e.g. for CollapsingHeader)");
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_FramePadding", &base_flags, RblxUITreeNodeFlags_FramePadding);
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_NavLeftJumpsToParent", &base_flags, RblxUITreeNodeFlags_NavLeftJumpsToParent);

            HelpMarker("Default option for DrawLinesXXX is stored in style.TreeLinesFlags");
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_DrawLinesNone", &base_flags, RblxUITreeNodeFlags_DrawLinesNone);
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_DrawLinesFull", &base_flags, RblxUITreeNodeFlags_DrawLinesFull);
            RblxUI::CheckboxFlags("RblxUITreeNodeFlags_DrawLinesToNodes", &base_flags, RblxUITreeNodeFlags_DrawLinesToNodes);

            RblxUI::Checkbox("Align label with current X position", &align_label_with_current_x_position);
            RblxUI::Checkbox("Make Tree Nodes as drag & drop sources", &use_drag_and_drop);
            if (align_label_with_current_x_position)
                RblxUI::Unindent(RblxUI::GetTreeNodeToLabelSpacing());

            for (int node_n = 0; node_n < 6; node_n++)
            {
                RblxUITreeNodeFlags node_flags = base_flags;
                if (node_n < 3)
                {
                    // Items 0..2 are Tree Node
                    bool is_open = RblxUI::TreeNodeEx((void*)(intptr_t)node_n, node_flags, "Selectable Node %d", node_n);
                    if (use_drag_and_drop && RblxUI::BeginDragDropSource())
                    {
                        RblxUI::SetDragDropPayload("MY_TREENODE_PAYLOAD_TYPE", NULL, 0);
                        RblxUI::Text("This is a drag and drop source");
                        RblxUI::EndDragDropSource();
                    }
                    if (node_n == 2 && (base_flags & RblxUITreeNodeFlags_SpanLabelWidth))
                    {
                        // Item 2 has an additional inline button to help demonstrate SpanLabelWidth.
                        RblxUI::SameLine();
                        if (RblxUI::SmallButton("button")) {}
                    }
                    if (is_open)
                    {
                        RblxUI::BulletText("Blah blah\nBlah Blah");
                        RblxUI::SameLine();
                        RblxUI::SmallButton("Button");
                        RblxUI::TreePop();
                    }
                }
                else
                {
                    // Items 3..5 are Tree Leaves
                    // The only reason we use TreeNode at all is to allow selection of the leaf. Otherwise we can
                    // use BulletText() or advance the cursor by GetTreeNodeToLabelSpacing() and call Text().
                    node_flags |= RblxUITreeNodeFlags_Leaf | RblxUITreeNodeFlags_NoTreePushOnOpen; // RblxUITreeNodeFlags_Bullet
                    RblxUI::TreeNodeEx((void*)(intptr_t)node_n, node_flags, "Selectable Leaf %d", node_n);
                    if (use_drag_and_drop && RblxUI::BeginDragDropSource())
                    {
                        RblxUI::SetDragDropPayload("MY_TREENODE_PAYLOAD_TYPE", NULL, 0);
                        RblxUI::Text("This is a drag and drop source");
                        RblxUI::EndDragDropSource();
                    }
                }
            }
            if (align_label_with_current_x_position)
                RblxUI::Indent(RblxUI::GetTreeNodeToLabelSpacing());
            RblxUI::TreePop();
        }
        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsVerticalSliders()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsVerticalSliders()
{
    if (RblxUI::TreeNode("Vertical Sliders"))
    {
        RBLXUI_DEMO_MARKER("Widgets/Vertical Sliders");
        const float spacing = 4;
        RblxUI::PushStyleVar(RblxUIStyleVar_ItemSpacing, RblxVec2(spacing, spacing));

        static int int_value = 0;
        RblxUI::VSliderInt("##int", RblxVec2(18, 160), &int_value, 0, 5);
        RblxUI::SameLine();

        static float values[7] = { 0.0f, 0.60f, 0.35f, 0.9f, 0.70f, 0.20f, 0.0f };
        RblxUI::PushID("set1");
        for (int i = 0; i < 7; i++)
        {
            if (i > 0) RblxUI::SameLine();
            RblxUI::PushID(i);
            RblxUI::PushStyleColor(RblxUICol_FrameBg, (RblxVec4)RblxColor::HSV(i / 7.0f, 0.5f, 0.5f));
            RblxUI::PushStyleColor(RblxUICol_FrameBgHovered, (RblxVec4)RblxColor::HSV(i / 7.0f, 0.6f, 0.5f));
            RblxUI::PushStyleColor(RblxUICol_FrameBgActive, (RblxVec4)RblxColor::HSV(i / 7.0f, 0.7f, 0.5f));
            RblxUI::PushStyleColor(RblxUICol_SliderGrab, (RblxVec4)RblxColor::HSV(i / 7.0f, 0.9f, 0.9f));
            RblxUI::VSliderFloat("##v", RblxVec2(18, 160), &values[i], 0.0f, 1.0f, "");
            if (RblxUI::IsItemActive() || RblxUI::IsItemHovered())
                RblxUI::SetTooltip("%.3f", values[i]);
            RblxUI::PopStyleColor(4);
            RblxUI::PopID();
        }
        RblxUI::PopID();

        RblxUI::SameLine();
        RblxUI::PushID("set2");
        static float values2[4] = { 0.20f, 0.80f, 0.40f, 0.25f };
        const int rows = 3;
        const RblxVec2 small_slider_size(18, (float)(int)((160.0f - (rows - 1) * spacing) / rows));
        for (int nx = 0; nx < 4; nx++)
        {
            if (nx > 0) RblxUI::SameLine();
            RblxUI::BeginGroup();
            for (int ny = 0; ny < rows; ny++)
            {
                RblxUI::PushID(nx * rows + ny);
                RblxUI::VSliderFloat("##v", small_slider_size, &values2[nx], 0.0f, 1.0f, "");
                if (RblxUI::IsItemActive() || RblxUI::IsItemHovered())
                    RblxUI::SetTooltip("%.3f", values2[nx]);
                RblxUI::PopID();
            }
            RblxUI::EndGroup();
        }
        RblxUI::PopID();

        RblxUI::SameLine();
        RblxUI::PushID("set3");
        for (int i = 0; i < 4; i++)
        {
            if (i > 0) RblxUI::SameLine();
            RblxUI::PushID(i);
            RblxUI::PushStyleVar(RblxUIStyleVar_GrabMinSize, 40);
            RblxUI::VSliderFloat("##v", RblxVec2(40, 160), &values[i], 0.0f, 1.0f, "%.2f\nsec");
            RblxUI::PopStyleVar();
            RblxUI::PopID();
        }
        RblxUI::PopID();
        RblxUI::PopStyleVar();
        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgets()
//-----------------------------------------------------------------------------

static void DemoWindowWidgets(RblxUIDemoWindowData* demo_data)
{
    //RblxUI::SetNextItemOpen(true, RblxUICond_Once);
    if (!RblxUI::CollapsingHeader("Widgets"))
        return;
    // RBLXUI_DEMO_MARKER("Widgets");

    const bool disable_all = demo_data->DisableSections; // The Checkbox for that is inside the "Disabled" section at the bottom
    if (disable_all)
        RblxUI::BeginDisabled();

    DemoWindowWidgetsBasic();
    DemoWindowWidgetsBullets();
    DemoWindowWidgetsCollapsingHeaders();
    DemoWindowWidgetsComboBoxes();
    DemoWindowWidgetsColorAndPickers();
    DemoWindowWidgetsDataTypes();

    if (disable_all)
        RblxUI::EndDisabled();
    DemoWindowWidgetsDisableBlocks(demo_data);
    if (disable_all)
        RblxUI::BeginDisabled();

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
        RblxUI::EndDisabled();
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowLayout()
//-----------------------------------------------------------------------------

static void DemoWindowLayout()
{
    if (!RblxUI::CollapsingHeader("Layout & Scrolling"))
        return;

    if (RblxUI::TreeNode("Child windows"))
    {
        RBLXUI_DEMO_MARKER("Layout/Child windows");
        RblxUI::SeparatorText("Child windows");

        HelpMarker("Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window.");
        static bool disable_mouse_wheel = false;
        static bool disable_menu = false;
        RblxUI::Checkbox("Disable Mouse Wheel", &disable_mouse_wheel);
        RblxUI::Checkbox("Disable Menu", &disable_menu);

        // Child 1: no border, enable horizontal scrollbar
        {
            RblxUIWindowFlags window_flags = RblxUIWindowFlags_HorizontalScrollbar;
            if (disable_mouse_wheel)
                window_flags |= RblxUIWindowFlags_NoScrollWithMouse;
            RblxUI::BeginChild("ChildL", RblxVec2(RblxUI::GetContentRegionAvail().x * 0.5f, 260), RblxUIChildFlags_None, window_flags);
            for (int i = 0; i < 100; i++)
                RblxUI::Text("%04d: scrollable region", i);
            RblxUI::EndChild();
        }

        RblxUI::SameLine();

        // Child 2: rounded border
        {
            RblxUIWindowFlags window_flags = RblxUIWindowFlags_None;
            if (disable_mouse_wheel)
                window_flags |= RblxUIWindowFlags_NoScrollWithMouse;
            if (!disable_menu)
                window_flags |= RblxUIWindowFlags_MenuBar;
            RblxUI::PushStyleVar(RblxUIStyleVar_ChildRounding, 5.0f);
            RblxUI::BeginChild("ChildR", RblxVec2(0, 260), RblxUIChildFlags_Borders, window_flags);
            if (!disable_menu && RblxUI::BeginMenuBar())
            {
                if (RblxUI::BeginMenu("Menu"))
                {
                    ShowExampleMenuFile();
                    RblxUI::EndMenu();
                }
                RblxUI::EndMenuBar();
            }
            if (RblxUI::BeginTable("split", 2, RblxUITableFlags_Resizable | RblxUITableFlags_NoSavedSettings))
            {
                for (int i = 0; i < 100; i++)
                {
                    char buf[32];
                    sprintf(buf, "%03d", i);
                    RblxUI::TableNextColumn();
                    RblxUI::Button(buf, RblxVec2(-FLT_MIN, 0.0f));
                }
                RblxUI::EndTable();
            }
            RblxUI::EndChild();
            RblxUI::PopStyleVar();
        }

        // Child 3: manual-resize
        RblxUI::SeparatorText("Manual-resize");
        {
            HelpMarker("Drag bottom border to resize. Double-click bottom border to auto-fit to vertical contents.");
            //if (RblxUI::Button("Set Height to 200"))
            //    RblxUI::SetNextWindowSize(RblxVec2(-FLT_MIN, 200.0f));

            RblxUI::PushStyleColor(RblxUICol_ChildBg, RblxUI::GetStyleColorVec4(RblxUICol_FrameBg));
            if (RblxUI::BeginChild("ResizableChild", RblxVec2(-FLT_MIN, RblxUI::GetTextLineHeightWithSpacing() * 8), RblxUIChildFlags_Borders | RblxUIChildFlags_ResizeY))
                for (int n = 0; n < 10; n++)
                    RblxUI::Text("Line %04d", n);
            RblxUI::PopStyleColor();
            RblxUI::EndChild();
        }

        // Child 4: auto-resizing height with a limit
        RblxUI::SeparatorText("Auto-resize with constraints");
        {
            static int draw_lines = 3;
            static int max_height_in_lines = 10;
            RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 8);
            RblxUI::DragInt("Lines Count", &draw_lines, 0.2f);
            RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 8);
            RblxUI::DragInt("Max Height (in Lines)", &max_height_in_lines, 0.2f);

            RblxUI::SetNextWindowSizeConstraints(RblxVec2(0.0f, RblxUI::GetTextLineHeightWithSpacing() * 1), RblxVec2(FLT_MAX, RblxUI::GetTextLineHeightWithSpacing() * max_height_in_lines));
            if (RblxUI::BeginChild("ConstrainedChild", RblxVec2(-FLT_MIN, 0.0f), RblxUIChildFlags_Borders | RblxUIChildFlags_AutoResizeY))
                for (int n = 0; n < draw_lines; n++)
                    RblxUI::Text("Line %04d", n);
            RblxUI::EndChild();
        }

        RblxUI::SeparatorText("Misc/Advanced");

        // Demonstrate a few extra things
        // - Changing RblxUICol_ChildBg (which is transparent black in default styles)
        // - Using SetCursorPos() to position child window (the child window is an item from the POV of parent window)
        //   You can also call SetNextWindowPos() to position the child window. The parent window will effectively
        //   layout from this position.
        // - Using RblxUI::GetItemRectMin/Max() to query the "item" state (because the child window is an item from
        //   the POV of the parent window). See 'Demo->Querying Status (Edited/Active/Hovered etc.)' for details.
        {
            static int offset_x = 0;
            static bool override_bg_color = true;
            static RblxUIChildFlags child_flags = RblxUIChildFlags_Borders | RblxUIChildFlags_ResizeX | RblxUIChildFlags_ResizeY;
            RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 8);
            RblxUI::DragInt("Offset X", &offset_x, 1.0f, -1000, 1000);
            RblxUI::Checkbox("Override ChildBg color", &override_bg_color);
            RblxUI::CheckboxFlags("RblxUIChildFlags_Borders", &child_flags, RblxUIChildFlags_Borders);
            RblxUI::CheckboxFlags("RblxUIChildFlags_AlwaysUseWindowPadding", &child_flags, RblxUIChildFlags_AlwaysUseWindowPadding);
            RblxUI::CheckboxFlags("RblxUIChildFlags_ResizeX", &child_flags, RblxUIChildFlags_ResizeX);
            RblxUI::CheckboxFlags("RblxUIChildFlags_ResizeY", &child_flags, RblxUIChildFlags_ResizeY);
            RblxUI::CheckboxFlags("RblxUIChildFlags_FrameStyle", &child_flags, RblxUIChildFlags_FrameStyle);
            RblxUI::SameLine(); HelpMarker("Style the child window like a framed item: use FrameBg, FrameRounding, FrameBorderSize, FramePadding instead of ChildBg, ChildRounding, ChildBorderSize, WindowPadding.");
            if (child_flags & RblxUIChildFlags_FrameStyle)
                override_bg_color = false;

            RblxUI::SetCursorPosX(RblxUI::GetCursorPosX() + (float)offset_x);
            if (override_bg_color)
                RblxUI::PushStyleColor(RblxUICol_ChildBg, RBLXUI_COL32(255, 0, 0, 100));
            RblxUI::BeginChild("Red", RblxVec2(200, 100), child_flags, RblxUIWindowFlags_None);
            if (override_bg_color)
                RblxUI::PopStyleColor();

            for (int n = 0; n < 50; n++)
                RblxUI::Text("Some test %d", n);
            RblxUI::EndChild();
            bool child_is_hovered = RblxUI::IsItemHovered();
            RblxVec2 child_rect_min = RblxUI::GetItemRectMin();
            RblxVec2 child_rect_max = RblxUI::GetItemRectMax();
            RblxUI::Text("Hovered: %d", child_is_hovered);
            RblxUI::Text("Rect of child window is: (%.0f,%.0f) (%.0f,%.0f)", child_rect_min.x, child_rect_min.y, child_rect_max.x, child_rect_max.y);
        }

        RblxUI::TreePop();
    }

    if (RblxUI::TreeNode("Widgets Width"))
    {
        RBLXUI_DEMO_MARKER("Layout/Widgets Width");
        static float f = 0.0f;
        static bool show_indented_items = true;
        RblxUI::Checkbox("Show indented items", &show_indented_items);

        // Use SetNextItemWidth() to set the width of a single upcoming item.
        // Use PushItemWidth()/PopItemWidth() to set the width of a group of items.
        // In real code use you'll probably want to choose width values that are proportional to your font size
        // e.g. Using '20.0f * GetFontSize()' as width instead of '200.0f', etc.

        RblxUI::Text("SetNextItemWidth/PushItemWidth(100)");
        RblxUI::SameLine(); HelpMarker("Fixed width.");
        RblxUI::PushItemWidth(100);
        RblxUI::DragFloat("float##1b", &f);
        if (show_indented_items)
        {
            RblxUI::Indent();
            RblxUI::DragFloat("float (indented)##1b", &f);
            RblxUI::Unindent();
        }
        RblxUI::PopItemWidth();

        RblxUI::Text("SetNextItemWidth/PushItemWidth(-100)");
        RblxUI::SameLine(); HelpMarker("Align to right edge minus 100");
        RblxUI::PushItemWidth(-100);
        RblxUI::DragFloat("float##2a", &f);
        if (show_indented_items)
        {
            RblxUI::Indent();
            RblxUI::DragFloat("float (indented)##2b", &f);
            RblxUI::Unindent();
        }
        RblxUI::PopItemWidth();

        RblxUI::Text("SetNextItemWidth/PushItemWidth(GetContentRegionAvail().x * 0.5f)");
        RblxUI::SameLine(); HelpMarker("Half of available width.\n(~ right-cursor_pos)\n(works within a column set)");
        RblxUI::PushItemWidth(RblxUI::GetContentRegionAvail().x * 0.5f);
        RblxUI::DragFloat("float##3a", &f);
        if (show_indented_items)
        {
            RblxUI::Indent();
            RblxUI::DragFloat("float (indented)##3b", &f);
            RblxUI::Unindent();
        }
        RblxUI::PopItemWidth();

        RblxUI::Text("SetNextItemWidth/PushItemWidth(-GetContentRegionAvail().x * 0.5f)");
        RblxUI::SameLine(); HelpMarker("Align to right edge minus half");
        RblxUI::PushItemWidth(-RblxUI::GetContentRegionAvail().x * 0.5f);
        RblxUI::DragFloat("float##4a", &f);
        if (show_indented_items)
        {
            RblxUI::Indent();
            RblxUI::DragFloat("float (indented)##4b", &f);
            RblxUI::Unindent();
        }
        RblxUI::PopItemWidth();

        RblxUI::Text("SetNextItemWidth/PushItemWidth(-Min(GetContentRegionAvail().x * 0.40f, GetFontSize() * 12))");
        RblxUI::PushItemWidth(-RBLXUI_MIN(RblxUI::GetFontSize() * 12, RblxUI::GetContentRegionAvail().x * 0.40f));
        RblxUI::DragFloat("float##5a", &f);
        if (show_indented_items)
        {
            RblxUI::Indent();
            RblxUI::DragFloat("float (indented)##5b", &f);
            RblxUI::Unindent();
        }
        RblxUI::PopItemWidth();

        // Demonstrate using PushItemWidth to surround three items.
        // Calling SetNextItemWidth() before each of them would have the same effect.
        RblxUI::Text("SetNextItemWidth/PushItemWidth(-FLT_MIN)");
        RblxUI::SameLine(); HelpMarker("Align to right edge");
        RblxUI::PushItemWidth(-FLT_MIN);
        RblxUI::DragFloat("##float6a", &f);
        if (show_indented_items)
        {
            RblxUI::Indent();
            RblxUI::DragFloat("float (indented)##6b", &f);
            RblxUI::Unindent();
        }
        RblxUI::PopItemWidth();

        RblxUI::TreePop();
    }

    if (RblxUI::TreeNode("Basic Horizontal Layout"))
    {
        RBLXUI_DEMO_MARKER("Layout/Basic Horizontal Layout");
        RblxUI::TextWrapped("(Use RblxUI::SameLine() to keep adding items to the right of the preceding item)");

        // Text
        RBLXUI_DEMO_MARKER("Layout/Basic Horizontal Layout/SameLine");
        RblxUI::Text("Two items: Hello"); RblxUI::SameLine();
        RblxUI::TextColored(RblxVec4(1, 1, 0, 1), "Sailor");

        // Adjust spacing
        RblxUI::Text("More spacing: Hello"); RblxUI::SameLine(0, 20);
        RblxUI::TextColored(RblxVec4(1, 1, 0, 1), "Sailor");

        // Button
        RblxUI::AlignTextToFramePadding();
        RblxUI::Text("Normal buttons"); RblxUI::SameLine();
        RblxUI::Button("Banana"); RblxUI::SameLine();
        RblxUI::Button("Apple"); RblxUI::SameLine();
        RblxUI::Button("Corniflower");

        // Button
        RblxUI::Text("Small buttons"); RblxUI::SameLine();
        RblxUI::SmallButton("Like this one"); RblxUI::SameLine();
        RblxUI::Text("can fit within a text block.");

        // Aligned to arbitrary position. Easy/cheap column.
        RBLXUI_DEMO_MARKER("Layout/Basic Horizontal Layout/SameLine (with offset)");
        RblxUI::Text("Aligned");
        RblxUI::SameLine(150); RblxUI::Text("x=150");
        RblxUI::SameLine(300); RblxUI::Text("x=300");
        RblxUI::Text("Aligned");
        RblxUI::SameLine(150); RblxUI::SmallButton("x=150");
        RblxUI::SameLine(300); RblxUI::SmallButton("x=300");

        // Checkbox
        RBLXUI_DEMO_MARKER("Layout/Basic Horizontal Layout/SameLine (more)");
        static bool c1 = false, c2 = false, c3 = false, c4 = false;
        RblxUI::Checkbox("My", &c1); RblxUI::SameLine();
        RblxUI::Checkbox("Tailor", &c2); RblxUI::SameLine();
        RblxUI::Checkbox("Is", &c3); RblxUI::SameLine();
        RblxUI::Checkbox("Rich", &c4);

        // Various
        static float f0 = 1.0f, f1 = 2.0f, f2 = 3.0f;
        RblxUI::PushItemWidth(RblxUI::CalcTextSize("AAAAAAA").x);
        const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD" };
        static int item = -1;
        RblxUI::Combo("Combo", &item, items, RBLXUI_COUNTOF(items)); RblxUI::SameLine();
        RblxUI::SliderFloat("X", &f0, 0.0f, 5.0f); RblxUI::SameLine();
        RblxUI::SliderFloat("Y", &f1, 0.0f, 5.0f); RblxUI::SameLine();
        RblxUI::SliderFloat("Z", &f2, 0.0f, 5.0f);

        RblxUI::Text("Lists:");
        static int selection[4] = { 0, 1, 2, 3 };
        for (int i = 0; i < 4; i++)
        {
            if (i > 0) RblxUI::SameLine();
            RblxUI::PushID(i);
            RblxUI::ListBox("", &selection[i], items, RBLXUI_COUNTOF(items));
            RblxUI::PopID();
            //RblxUI::SetItemTooltip("ListBox %d hovered", i);
        }
        RblxUI::PopItemWidth();

        // Dummy
        RBLXUI_DEMO_MARKER("Layout/Basic Horizontal Layout/Dummy");
        RblxVec2 button_sz(40, 40);
        RblxUI::Button("A", button_sz); RblxUI::SameLine();
        RblxUI::Dummy(button_sz); RblxUI::SameLine();
        RblxUI::Button("B", button_sz);

        // Manually wrapping
        // (we should eventually provide this as an automatic layout feature, but for now you can do it manually)
        RBLXUI_DEMO_MARKER("Layout/Basic Horizontal Layout/Manual wrapping");
        RblxUI::Text("Manual wrapping:");
        RblxUIStyle& style = RblxUI::GetStyle();
        int buttons_count = 20;
        float window_visible_x2 = RblxUI::GetCursorScreenPos().x + RblxUI::GetContentRegionAvail().x;
        for (int n = 0; n < buttons_count; n++)
        {
            RblxUI::PushID(n);
            RblxUI::Button("Box", button_sz);
            float last_button_x2 = RblxUI::GetItemRectMax().x;
            float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
            if (n + 1 < buttons_count && next_button_x2 < window_visible_x2)
                RblxUI::SameLine();
            RblxUI::PopID();
        }

        RblxUI::TreePop();
    }

    if (RblxUI::TreeNode("Groups"))
    {
        RBLXUI_DEMO_MARKER("Layout/Groups");
        HelpMarker(
            "BeginGroup() basically locks the horizontal position for new line. "
            "EndGroup() bundles the whole group so that you can use \"item\" functions such as "
            "IsItemHovered()/IsItemActive() or SameLine() etc. on the whole group.");
        RblxUI::BeginGroup();
        {
            RblxUI::BeginGroup();
            RblxUI::Button("AAA");
            RblxUI::SameLine();
            RblxUI::Button("BBB");
            RblxUI::SameLine();
            RblxUI::BeginGroup();
            RblxUI::Button("CCC");
            RblxUI::Button("DDD");
            RblxUI::EndGroup();
            RblxUI::SameLine();
            RblxUI::Button("EEE");
            RblxUI::EndGroup();
            RblxUI::SetItemTooltip("First group hovered");
        }
        // Capture the group size and create widgets using the same size
        RblxVec2 size = RblxUI::GetItemRectSize();
        const float values[5] = { 0.5f, 0.20f, 0.80f, 0.60f, 0.25f };
        RblxUI::PlotHistogram("##values", values, RBLXUI_COUNTOF(values), 0, NULL, 0.0f, 1.0f, size);

        RblxUI::Button("ACTION", RblxVec2((size.x - RblxUI::GetStyle().ItemSpacing.x) * 0.5f, size.y));
        RblxUI::SameLine();
        RblxUI::Button("REACTION", RblxVec2((size.x - RblxUI::GetStyle().ItemSpacing.x) * 0.5f, size.y));
        RblxUI::EndGroup();
        RblxUI::SameLine();

        RblxUI::Button("LEVERAGE\nBUZZWORD", size);
        RblxUI::SameLine();

        if (RblxUI::BeginListBox("List", size))
        {
            RblxUI::Selectable("Selected", true);
            RblxUI::Selectable("Not Selected", false);
            RblxUI::EndListBox();
        }

        RblxUI::TreePop();
    }

    if (RblxUI::TreeNode("Text Baseline Alignment"))
    {
        RBLXUI_DEMO_MARKER("Layout/Text Baseline Alignment");
        {
            RblxUI::BulletText("Text baseline:");
            RblxUI::SameLine(); HelpMarker(
                "This is testing the vertical alignment that gets applied on text to keep it aligned with widgets. "
                "Lines only composed of text or \"small\" widgets use less vertical space than lines with framed widgets.");
            RblxUI::Indent();

            RblxUI::Text("KO Blahblah"); RblxUI::SameLine();
            RblxUI::Button("Some framed item"); RblxUI::SameLine();
            HelpMarker("Baseline of button will look misaligned with text..");

            // If your line starts with text, call AlignTextToFramePadding() to align text to upcoming widgets.
            // (because we don't know what's coming after the Text() statement, we need to move the text baseline
            // down by FramePadding.y ahead of time)
            RblxUI::AlignTextToFramePadding();
            RblxUI::Text("OK Blahblah"); RblxUI::SameLine();
            RblxUI::Button("Some framed item##2"); RblxUI::SameLine();
            HelpMarker("We call AlignTextToFramePadding() to vertically align the text baseline by +FramePadding.y");

            // SmallButton() uses the same vertical padding as Text
            RblxUI::Button("TEST##1"); RblxUI::SameLine();
            RblxUI::Text("TEST"); RblxUI::SameLine();
            RblxUI::SmallButton("TEST##2");

            // If your line starts with text, call AlignTextToFramePadding() to align text to upcoming widgets.
            RblxUI::AlignTextToFramePadding();
            RblxUI::Text("Text aligned to framed item"); RblxUI::SameLine();
            RblxUI::Button("Item##1"); RblxUI::SameLine();
            RblxUI::Text("Item"); RblxUI::SameLine();
            RblxUI::SmallButton("Item##2"); RblxUI::SameLine();
            RblxUI::Button("Item##3");

            RblxUI::Unindent();
        }

        RblxUI::Spacing();

        {
            RblxUI::BulletText("Multi-line text:");
            RblxUI::Indent();
            RblxUI::Text("One\nTwo\nThree"); RblxUI::SameLine();
            RblxUI::Text("Hello\nWorld"); RblxUI::SameLine();
            RblxUI::Text("Banana");

            RblxUI::Text("Banana"); RblxUI::SameLine();
            RblxUI::Text("Hello\nWorld"); RblxUI::SameLine();
            RblxUI::Text("One\nTwo\nThree");

            RblxUI::Button("HOP##1"); RblxUI::SameLine();
            RblxUI::Text("Banana"); RblxUI::SameLine();
            RblxUI::Text("Hello\nWorld"); RblxUI::SameLine();
            RblxUI::Text("Banana");

            RblxUI::Button("HOP##2"); RblxUI::SameLine();
            RblxUI::Text("Hello\nWorld"); RblxUI::SameLine();
            RblxUI::Text("Banana");
            RblxUI::Unindent();
        }

        RblxUI::Spacing();

        {
            RblxUI::BulletText("Misc items:");
            RblxUI::Indent();

            // SmallButton() sets FramePadding to zero. Text baseline is aligned to match baseline of previous Button.
            RblxUI::Button("80x80", RblxVec2(80, 80));
            RblxUI::SameLine();
            RblxUI::Button("50x50", RblxVec2(50, 50));
            RblxUI::SameLine();
            RblxUI::Button("Button()");
            RblxUI::SameLine();
            RblxUI::SmallButton("SmallButton()");

            // Tree
            // (here the node appears after a button and has odd intent, so we use RblxUITreeNodeFlags_DrawLinesNone to disable hierarchy outline)
            const float spacing = RblxUI::GetStyle().ItemInnerSpacing.x;
            RblxUI::Button("Button##1"); // Will make line higher
            RblxUI::SameLine(0.0f, spacing);
            if (RblxUI::TreeNodeEx("Node##1", RblxUITreeNodeFlags_DrawLinesNone))
            {
                // Placeholder tree data
                for (int i = 0; i < 6; i++)
                    RblxUI::BulletText("Item %d..", i);
                RblxUI::TreePop();
            }

            const float padding = (float)(int)(RblxUI::GetFontSize() * 1.20f); // Large padding
            RblxUI::PushStyleVarY(RblxUIStyleVar_FramePadding, padding);
            RblxUI::Button("Button##2");
            RblxUI::PopStyleVar();
            RblxUI::SameLine(0.0f, spacing);
            if (RblxUI::TreeNodeEx("Node##2", RblxUITreeNodeFlags_DrawLinesNone))
                RblxUI::TreePop();

            // Vertically align text node a bit lower so it'll be vertically centered with upcoming widget.
            // Otherwise you can use SmallButton() (smaller fit).
            RblxUI::AlignTextToFramePadding();

            // Common mistake to avoid: if we want to SameLine after TreeNode we need to do it before we add
            // other contents "inside" the node.
            bool node_open = RblxUI::TreeNode("Node##3");
            RblxUI::SameLine(0.0f, spacing); RblxUI::Button("Button##3");
            if (node_open)
            {
                // Placeholder tree data
                for (int i = 0; i < 6; i++)
                    RblxUI::BulletText("Item %d..", i);
                RblxUI::TreePop();
            }

            // Bullet
            RblxUI::Button("Button##4");
            RblxUI::SameLine(0.0f, spacing);
            RblxUI::BulletText("Bullet text");

            RblxUI::AlignTextToFramePadding();
            RblxUI::BulletText("Node");
            RblxUI::SameLine(0.0f, spacing); RblxUI::Button("Button##5");
            RblxUI::Unindent();
        }

        RblxUI::TreePop();
    }

    if (RblxUI::TreeNode("Scrolling"))
    {
        RBLXUI_DEMO_MARKER("Layout/Scrolling/Vertical");
        // Vertical scroll functions
        HelpMarker("Use SetScrollHereY() or SetScrollFromPosY() to scroll to a given vertical position.");

        static int track_item = 50;
        static bool enable_track = true;
        static bool enable_extra_decorations = false;
        static float scroll_to_off_px = 0.0f;
        static float scroll_to_pos_px = 200.0f;

        RblxUI::Checkbox("Decoration", &enable_extra_decorations);

        RblxUI::PushItemWidth(RblxUI::GetFontSize() * 10);
        enable_track |= RblxUI::DragInt("##item", &track_item, 0.25f, 0, 99, "Item = %d");
        RblxUI::SameLine();
        RblxUI::Checkbox("Track", &enable_track);

        bool scroll_to_off = RblxUI::DragFloat("##off", &scroll_to_off_px, 1.00f, 0, FLT_MAX, "+%.0f px");
        RblxUI::SameLine();
        scroll_to_off |= RblxUI::Button("Scroll Offset");

        bool scroll_to_pos = RblxUI::DragFloat("##pos", &scroll_to_pos_px, 1.00f, -10, FLT_MAX, "X/Y = %.0f px");
        RblxUI::SameLine();
        scroll_to_pos |= RblxUI::Button("Scroll To Pos");
        RblxUI::PopItemWidth();

        if (scroll_to_off || scroll_to_pos)
            enable_track = false;

        RblxUIStyle& style = RblxUI::GetStyle();
        float child_w = (RblxUI::GetContentRegionAvail().x - 4 * style.ItemSpacing.x) / 5;
        if (child_w < 1.0f)
            child_w = 1.0f;
        RblxUI::PushID("##VerticalScrolling");
        for (int i = 0; i < 5; i++)
        {
            if (i > 0) RblxUI::SameLine();
            RblxUI::BeginGroup();
            const char* names[] = { "Top", "25%", "Center", "75%", "Bottom" };
            RblxUI::TextUnformatted(names[i]);

            const RblxUIWindowFlags child_flags = enable_extra_decorations ? RblxUIWindowFlags_MenuBar : 0;
            const RblxUIID child_id = RblxUI::GetID((void*)(intptr_t)i);
            const bool child_is_visible = RblxUI::BeginChild(child_id, RblxVec2(child_w, 200.0f), RblxUIChildFlags_Borders, child_flags);
            if (RblxUI::BeginMenuBar())
            {
                RblxUI::TextUnformatted("abc");
                RblxUI::EndMenuBar();
            }
            if (scroll_to_off)
                RblxUI::SetScrollY(scroll_to_off_px);
            if (scroll_to_pos)
                RblxUI::SetScrollFromPosY(RblxUI::GetCursorStartPos().y + scroll_to_pos_px, i * 0.25f);
            if (child_is_visible) // Avoid calling SetScrollHereY when running with culled items
            {
                for (int item = 0; item < 100; item++)
                {
                    if (enable_track && item == track_item)
                    {
                        RblxUI::TextColored(RblxVec4(1, 1, 0, 1), "Item %d", item);
                        RblxUI::SetScrollHereY(i * 0.25f); // 0.0f:top, 0.5f:center, 1.0f:bottom
                    }
                    else
                    {
                        RblxUI::Text("Item %d", item);
                    }
                }
            }
            float scroll_y = RblxUI::GetScrollY();
            float scroll_max_y = RblxUI::GetScrollMaxY();
            RblxUI::EndChild();
            RblxUI::Text("%.0f/%.0f", scroll_y, scroll_max_y);
            RblxUI::EndGroup();
        }
        RblxUI::PopID();

        // Horizontal scroll functions
        RBLXUI_DEMO_MARKER("Layout/Scrolling/Horizontal");
        RblxUI::Spacing();
        HelpMarker(
            "Use SetScrollHereX() or SetScrollFromPosX() to scroll to a given horizontal position.\n\n"
            "Because the clipping rectangle of most window hides half worth of WindowPadding on the "
            "left/right, using SetScrollFromPosX(+1) will usually result in clipped text whereas the "
            "equivalent SetScrollFromPosY(+1) wouldn't.");
        RblxUI::PushID("##HorizontalScrolling");
        for (int i = 0; i < 5; i++)
        {
            float child_height = RblxUI::GetTextLineHeight() + style.ScrollbarSize + style.WindowPadding.y * 2.0f;
            RblxUIWindowFlags child_flags = RblxUIWindowFlags_HorizontalScrollbar | (enable_extra_decorations ? RblxUIWindowFlags_AlwaysVerticalScrollbar : 0);
            RblxUIID child_id = RblxUI::GetID((void*)(intptr_t)i);
            bool child_is_visible = RblxUI::BeginChild(child_id, RblxVec2(-100, child_height), RblxUIChildFlags_Borders, child_flags);
            if (scroll_to_off)
                RblxUI::SetScrollX(scroll_to_off_px);
            if (scroll_to_pos)
                RblxUI::SetScrollFromPosX(RblxUI::GetCursorStartPos().x + scroll_to_pos_px, i * 0.25f);
            if (child_is_visible) // Avoid calling SetScrollHereY when running with culled items
            {
                for (int item = 0; item < 100; item++)
                {
                    if (item > 0)
                        RblxUI::SameLine();
                    if (enable_track && item == track_item)
                    {
                        RblxUI::TextColored(RblxVec4(1, 1, 0, 1), "Item %d", item);
                        RblxUI::SetScrollHereX(i * 0.25f); // 0.0f:left, 0.5f:center, 1.0f:right
                    }
                    else
                    {
                        RblxUI::Text("Item %d", item);
                    }
                }
            }
            float scroll_x = RblxUI::GetScrollX();
            float scroll_max_x = RblxUI::GetScrollMaxX();
            RblxUI::EndChild();
            RblxUI::SameLine();
            const char* names[] = { "Left", "25%", "Center", "75%", "Right" };
            RblxUI::Text("%s\n%.0f/%.0f", names[i], scroll_x, scroll_max_x);
            RblxUI::Spacing();
        }
        RblxUI::PopID();

        // Miscellaneous Horizontal Scrolling Demo
        RBLXUI_DEMO_MARKER("Layout/Scrolling/Horizontal (more)");
        HelpMarker(
            "Horizontal scrolling for a window is enabled via the RblxUIWindowFlags_HorizontalScrollbar flag.\n\n"
            "You may want to also explicitly specify content width by using SetNextWindowContentWidth() before Begin().");
        static int lines = 7;
        RblxUI::SliderInt("Lines", &lines, 1, 15);
        RblxUI::PushStyleVar(RblxUIStyleVar_FrameRounding, 3.0f);
        RblxUI::PushStyleVar(RblxUIStyleVar_FramePadding, RblxVec2(2.0f, 1.0f));
        RblxVec2 scrolling_child_size = RblxVec2(0, RblxUI::GetFrameHeightWithSpacing() * 7 + 30);
        RblxUI::BeginChild("scrolling", scrolling_child_size, RblxUIChildFlags_Borders, RblxUIWindowFlags_HorizontalScrollbar);
        for (int line = 0; line < lines; line++)
        {
            // Display random stuff. For the sake of this trivial demo we are using basic Button() + SameLine()
            // If you want to create your own time line for a real application you may be better off manipulating
            // the cursor position yourself, aka using SetCursorPos/SetCursorScreenPos to position the widgets
            // yourself. You may also want to use the lower-level RblxDrawList API.
            int num_buttons = 10 + ((line & 1) ? line * 9 : line * 3);
            for (int n = 0; n < num_buttons; n++)
            {
                if (n > 0) RblxUI::SameLine();
                RblxUI::PushID(n + line * 1000);
                char num_buf[16];
                sprintf(num_buf, "%d", n);
                const char* label = (!(n % 15)) ? "FizzBuzz" : (!(n % 3)) ? "Fizz" : (!(n % 5)) ? "Buzz" : num_buf;
                float hue = n * 0.05f;
                RblxUI::PushStyleColor(RblxUICol_Button, (RblxVec4)RblxColor::HSV(hue, 0.6f, 0.6f));
                RblxUI::PushStyleColor(RblxUICol_ButtonHovered, (RblxVec4)RblxColor::HSV(hue, 0.7f, 0.7f));
                RblxUI::PushStyleColor(RblxUICol_ButtonActive, (RblxVec4)RblxColor::HSV(hue, 0.8f, 0.8f));
                RblxUI::Button(label, RblxVec2(40.0f + sinf((float)(line + n)) * 20.0f, 0.0f));
                RblxUI::PopStyleColor(3);
                RblxUI::PopID();
            }
        }
        float scroll_x = RblxUI::GetScrollX();
        float scroll_max_x = RblxUI::GetScrollMaxX();
        RblxUI::EndChild();
        RblxUI::PopStyleVar(2);
        float scroll_x_delta = 0.0f;
        RblxUI::SmallButton("<<");
        if (RblxUI::IsItemActive())
            scroll_x_delta = -RblxUI::GetIO().DeltaTime * 1000.0f;
        RblxUI::SameLine();
        RblxUI::Text("Scroll from code"); RblxUI::SameLine();
        RblxUI::SmallButton(">>");
        if (RblxUI::IsItemActive())
            scroll_x_delta = +RblxUI::GetIO().DeltaTime * 1000.0f;
        RblxUI::SameLine();
        RblxUI::Text("%.0f/%.0f", scroll_x, scroll_max_x);
        if (scroll_x_delta != 0.0f)
        {
            // Demonstrate a trick: you can use Begin to set yourself in the context of another window
            // (here we are already out of your child window)
            RblxUI::BeginChild("scrolling");
            RblxUI::SetScrollX(RblxUI::GetScrollX() + scroll_x_delta);
            RblxUI::EndChild();
        }
        RblxUI::Spacing();

        static bool show_horizontal_contents_size_demo_window = false;
        RblxUI::Checkbox("Show Horizontal contents size demo window", &show_horizontal_contents_size_demo_window);

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
                RblxUI::SetNextWindowContentSize(RblxVec2(contents_size_x, 0.0f));
            RblxUI::Begin("Horizontal contents size demo window", &show_horizontal_contents_size_demo_window, show_h_scrollbar ? RblxUIWindowFlags_HorizontalScrollbar : 0);
            RBLXUI_DEMO_MARKER("Layout/Scrolling/Horizontal contents size demo window");
            RblxUI::PushStyleVar(RblxUIStyleVar_ItemSpacing, RblxVec2(2, 0));
            RblxUI::PushStyleVar(RblxUIStyleVar_FramePadding, RblxVec2(2, 0));
            HelpMarker(
                "Test how different widgets react and impact the work rectangle growing when horizontal scrolling is enabled.\n\n"
                "Use 'Metrics->Tools->Show windows rectangles' to visualize rectangles.");
            RblxUI::Checkbox("H-scrollbar", &show_h_scrollbar);
            RblxUI::Checkbox("Button", &show_button);            // Will grow contents size (unless explicitly overwritten)
            RblxUI::Checkbox("Tree nodes", &show_tree_nodes);    // Will grow contents size and display highlight over full width
            RblxUI::Checkbox("Text wrapped", &show_text_wrapped);// Will grow and use contents size
            RblxUI::Checkbox("Columns", &show_columns);          // Will use contents size
            RblxUI::Checkbox("Tab bar", &show_tab_bar);          // Will use contents size
            RblxUI::Checkbox("Child", &show_child);              // Will grow and use contents size
            RblxUI::Checkbox("Explicit content size", &explicit_content_size);
            RblxUI::Text("Scroll %.1f/%.1f %.1f/%.1f", RblxUI::GetScrollX(), RblxUI::GetScrollMaxX(), RblxUI::GetScrollY(), RblxUI::GetScrollMaxY());
            if (explicit_content_size)
            {
                RblxUI::SameLine();
                RblxUI::SetNextItemWidth(RblxUI::CalcTextSize("123456").x);
                RblxUI::DragFloat("##csx", &contents_size_x);
                RblxVec2 p = RblxUI::GetCursorScreenPos();
                RblxUI::GetWindowDrawList()->AddRectFilled(p, RblxVec2(p.x + 10, p.y + 10), RBLXUI_COL32_WHITE);
                RblxUI::GetWindowDrawList()->AddRectFilled(RblxVec2(p.x + contents_size_x - 10, p.y), RblxVec2(p.x + contents_size_x, p.y + 10), RBLXUI_COL32_WHITE);
                RblxUI::Dummy(RblxVec2(0, 10));
            }
            RblxUI::PopStyleVar(2);
            RblxUI::Separator();
            if (show_button)
            {
                RblxUI::Button("this is a 300-wide button", RblxVec2(300, 0));
            }
            if (show_tree_nodes)
            {
                bool open = true;
                if (RblxUI::TreeNode("this is a tree node"))
                {
                    if (RblxUI::TreeNode("another one of those tree node..."))
                    {
                        RblxUI::Text("Some tree contents");
                        RblxUI::TreePop();
                    }
                    RblxUI::TreePop();
                }
                RblxUI::CollapsingHeader("CollapsingHeader", &open);
            }
            if (show_text_wrapped)
            {
                RblxUI::TextWrapped("This text should automatically wrap on the edge of the work rectangle.");
            }
            if (show_columns)
            {
                RblxUI::Text("Tables:");
                if (RblxUI::BeginTable("table", 4, RblxUITableFlags_Borders))
                {
                    for (int n = 0; n < 4; n++)
                    {
                        RblxUI::TableNextColumn();
                        RblxUI::Text("Width %.2f", RblxUI::GetContentRegionAvail().x);
                    }
                    RblxUI::EndTable();
                }
                RblxUI::Text("Columns:");
                RblxUI::Columns(4);
                for (int n = 0; n < 4; n++)
                {
                    RblxUI::Text("Width %.2f", RblxUI::GetColumnWidth());
                    RblxUI::NextColumn();
                }
                RblxUI::Columns(1);
            }
            if (show_tab_bar && RblxUI::BeginTabBar("Hello"))
            {
                if (RblxUI::BeginTabItem("OneOneOne")) { RblxUI::EndTabItem(); }
                if (RblxUI::BeginTabItem("TwoTwoTwo")) { RblxUI::EndTabItem(); }
                if (RblxUI::BeginTabItem("ThreeThreeThree")) { RblxUI::EndTabItem(); }
                if (RblxUI::BeginTabItem("FourFourFour")) { RblxUI::EndTabItem(); }
                RblxUI::EndTabBar();
            }
            if (show_child)
            {
                RblxUI::BeginChild("child", RblxVec2(0, 0), RblxUIChildFlags_Borders);
                RblxUI::EndChild();
            }
            RblxUI::End();
        }

        RblxUI::TreePop();
    }

    if (RblxUI::TreeNode("Text Clipping"))
    {
        RBLXUI_DEMO_MARKER("Layout/Text Clipping");
        static RblxVec2 size(100.0f, 100.0f);
        static RblxVec2 offset(30.0f, 30.0f);
        RblxUI::DragFloat2("size", (float*)&size, 0.5f, 1.0f, 200.0f, "%.0f");
        RblxUI::TextWrapped("(Click and drag to scroll)");

        HelpMarker(
            "(Left) Using RblxUI::PushClipRect():\n"
            "Will alter RblxUI hit-testing logic + RblxDrawList rendering.\n"
            "(use this if you want your clipping rectangle to affect interactions)\n\n"
            "(Center) Using RblxDrawList::PushClipRect():\n"
            "Will alter RblxDrawList rendering only.\n"
            "(use this as a shortcut if you are only using RblxDrawList calls)\n\n"
            "(Right) Using RblxDrawList::AddText() with a fine ClipRect:\n"
            "Will alter only this specific RblxDrawList::AddText() rendering.\n"
            "This is often used internally to avoid altering the clipping rectangle and minimize draw calls.");

        for (int n = 0; n < 3; n++)
        {
            if (n > 0)
                RblxUI::SameLine();

            RblxUI::PushID(n);
            RblxUI::InvisibleButton("##canvas", size);
            if (RblxUI::IsItemActive() && RblxUI::IsMouseDragging(RblxUIMouseButton_Left))
            {
                offset.x += RblxUI::GetIO().MouseDelta.x;
                offset.y += RblxUI::GetIO().MouseDelta.y;
            }
            RblxUI::PopID();
            if (!RblxUI::IsItemVisible()) // Skip rendering as RblxDrawList elements are not clipped.
                continue;

            const RblxVec2 p0 = RblxUI::GetItemRectMin();
            const RblxVec2 p1 = RblxUI::GetItemRectMax();
            const char* text_str = "Line 1 hello\nLine 2 clip me!";
            const RblxVec2 text_pos = RblxVec2(p0.x + offset.x, p0.y + offset.y);
            RblxDrawList* draw_list = RblxUI::GetWindowDrawList();
            switch (n)
            {
            case 0:
                RblxUI::PushClipRect(p0, p1, true);
                draw_list->AddRectFilled(p0, p1, RBLXUI_COL32(90, 90, 120, 255));
                draw_list->AddText(text_pos, RBLXUI_COL32_WHITE, text_str);
                RblxUI::PopClipRect();
                break;
            case 1:
                draw_list->PushClipRect(p0, p1, true);
                draw_list->AddRectFilled(p0, p1, RBLXUI_COL32(90, 90, 120, 255));
                draw_list->AddText(text_pos, RBLXUI_COL32_WHITE, text_str);
                draw_list->PopClipRect();
                break;
            case 2:
                RblxVec4 clip_rect(p0.x, p0.y, p1.x, p1.y); // AddText() takes a RblxVec4* here so let's convert.
                draw_list->AddRectFilled(p0, p1, RBLXUI_COL32(90, 90, 120, 255));
                draw_list->AddText(RblxUI::GetFont(), RblxUI::GetFontSize(), text_pos, RBLXUI_COL32_WHITE, text_str, NULL, 0.0f, &clip_rect);
                break;
            }
        }

        RblxUI::TreePop();
    }

    if (RblxUI::TreeNode("Overlap Mode"))
    {
        RBLXUI_DEMO_MARKER("Layout/Overlap Mode");
        static bool enable_allow_overlap = true;

        HelpMarker(
            "Hit-testing is by default performed in item submission order, which generally is perceived as 'back-to-front'.\n\n"
            "By using SetNextItemAllowOverlap() you can notify that an item may be overlapped by another. "
            "Doing so alters the hovering logic: items using AllowOverlap mode requires an extra frame to accept hovered state.");
        RblxUI::Checkbox("Enable AllowOverlap", &enable_allow_overlap);

        RblxVec2 button1_pos = RblxUI::GetCursorScreenPos();
        RblxVec2 button2_pos = RblxVec2(button1_pos.x + 50.0f, button1_pos.y + 50.0f);
        if (enable_allow_overlap)
            RblxUI::SetNextItemAllowOverlap();
        RblxUI::Button("Button 1", RblxVec2(80, 80));
        RblxUI::SetCursorScreenPos(button2_pos);
        RblxUI::Button("Button 2", RblxVec2(80, 80));

        // This is typically used with width-spanning items.
        // (note that Selectable() has a dedicated flag RblxUISelectableFlags_AllowOverlap, which is a shortcut
        // for using SetNextItemAllowOverlap(). For demo purpose we use SetNextItemAllowOverlap() here.)
        if (enable_allow_overlap)
            RblxUI::SetNextItemAllowOverlap();
        RblxUI::Selectable("Some Selectable", false);
        RblxUI::SameLine();
        RblxUI::SmallButton("++");

        RblxUI::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowPopups()
//-----------------------------------------------------------------------------

static void DemoWindowPopups()
{
    if (!RblxUI::CollapsingHeader("Popups & Modal windows"))
        return;

    // The properties of popups windows are:
    // - They block normal mouse hovering detection outside them. (*)
    // - Unless modal, they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
    // - Their visibility state (~bool) is held internally by Dear RblxUI instead of being held by the programmer as
    //   we are used to with regular Begin() calls. User can manipulate the visibility state by calling OpenPopup().
    // (*) One can use IsItemHovered(RblxUIHoveredFlags_AllowWhenBlockedByPopup) to bypass it and detect hovering even
    //     when normally blocked by a popup.
    // Those three properties are connected. The library needs to hold their visibility state BECAUSE it can close
    // popups at any time.

    // Typical use for regular windows:
    //   bool my_tool_is_active = false; if (RblxUI::Button("Open")) my_tool_is_active = true; [...] if (my_tool_is_active) Begin("My Tool", &my_tool_is_active) { [...] } End();
    // Typical use for popups:
    //   if (RblxUI::Button("Open")) RblxUI::OpenPopup("MyPopup"); if (RblxUI::BeginPopup("MyPopup")) { [...] EndPopup(); }

    // With popups we have to go through a library call (here OpenPopup) to manipulate the visibility state.
    // This may be a bit confusing at first but it should quickly make sense. Follow on the examples below.

    if (RblxUI::TreeNode("Popups"))
    {
        RBLXUI_DEMO_MARKER("Popups/Popups");
        RblxUI::TextWrapped(
            "When a popup is active, it inhibits interacting with windows that are behind the popup. "
            "Clicking outside the popup closes it.");

        static int selected_fish = -1;
        const char* names[] = { "Bream", "Haddock", "Mackerel", "Pollock", "Tilefish" };
        static bool toggles[] = { true, false, false, false, false };

        // Simple selection popup (if you want to show the current selection inside the Button itself,
        // you may want to build a string using the "###" operator to preserve a constant ID with a variable label)
        if (RblxUI::Button("Select.."))
            RblxUI::OpenPopup("my_select_popup");
        RblxUI::SameLine();
        RblxUI::TextUnformatted(selected_fish == -1 ? "<None>" : names[selected_fish]);
        if (RblxUI::BeginPopup("my_select_popup"))
        {
            RblxUI::SeparatorText("Aquarium");
            for (int i = 0; i < RBLXUI_COUNTOF(names); i++)
                if (RblxUI::Selectable(names[i]))
                    selected_fish = i;
            RblxUI::EndPopup();
        }

        // Showing a menu with toggles
        if (RblxUI::Button("Toggle.."))
            RblxUI::OpenPopup("my_toggle_popup");
        if (RblxUI::BeginPopup("my_toggle_popup"))
        {
            for (int i = 0; i < RBLXUI_COUNTOF(names); i++)
                RblxUI::MenuItem(names[i], "", &toggles[i]);
            if (RblxUI::BeginMenu("Sub-menu"))
            {
                RblxUI::MenuItem("Click me");
                RblxUI::EndMenu();
            }

            RblxUI::Separator();
            RblxUI::Text("Tooltip here");
            RblxUI::SetItemTooltip("I am a tooltip over a popup");

            if (RblxUI::Button("Stacked Popup"))
                RblxUI::OpenPopup("another popup");
            if (RblxUI::BeginPopup("another popup"))
            {
                for (int i = 0; i < RBLXUI_COUNTOF(names); i++)
                    RblxUI::MenuItem(names[i], "", &toggles[i]);
                if (RblxUI::BeginMenu("Sub-menu"))
                {
                    RblxUI::MenuItem("Click me");
                    if (RblxUI::Button("Stacked Popup"))
                        RblxUI::OpenPopup("another popup");
                    if (RblxUI::BeginPopup("another popup"))
                    {
                        RblxUI::Text("I am the last one here.");
                        RblxUI::EndPopup();
                    }
                    RblxUI::EndMenu();
                }
                RblxUI::EndPopup();
            }
            RblxUI::EndPopup();
        }

        // Call the more complete ShowExampleMenuFile which we use in various places of this demo
        if (RblxUI::Button("With a menu.."))
            RblxUI::OpenPopup("my_file_popup");
        if (RblxUI::BeginPopup("my_file_popup", RblxUIWindowFlags_MenuBar))
        {
            if (RblxUI::BeginMenuBar())
            {
                if (RblxUI::BeginMenu("File"))
                {
                    ShowExampleMenuFile();
                    RblxUI::EndMenu();
                }
                if (RblxUI::BeginMenu("Edit"))
                {
                    RblxUI::MenuItem("Dummy");
                    RblxUI::EndMenu();
                }
                RblxUI::EndMenuBar();
            }
            RblxUI::Text("Hello from popup!");
            RblxUI::Button("This is a dummy button..");
            RblxUI::EndPopup();
        }

        RblxUI::TreePop();
    }

    if (RblxUI::TreeNode("Context menus"))
    {
        RBLXUI_DEMO_MARKER("Popups/Context menus");
        HelpMarker("\"Context\" functions are simple helpers to associate a Popup to a given Item or Window identifier.");

        // BeginPopupContextItem() is a helper to provide common/simple popup behavior of essentially doing:
        //     if (id == 0)
        //         id = GetItemID(); // Use last item id
        //     if (IsItemHovered() && IsMouseReleased(RblxUIMouseButton_Right))
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
                if (RblxUI::Selectable(names[n], selected == n))
                    selected = n;
                if (RblxUI::BeginPopupContextItem()) // <-- use last item id as popup id
                {
                    selected = n;
                    RblxUI::Text("This is a popup for \"%s\"!", names[n]);
                    if (RblxUI::Button("Close"))
                        RblxUI::CloseCurrentPopup();
                    RblxUI::EndPopup();
                }
                RblxUI::SetItemTooltip("Right-click to open popup");
            }
        }

        // Example 2
        // Popup on a Text() element which doesn't have an identifier: we need to provide an identifier to BeginPopupContextItem().
        // Using an explicit identifier is also convenient if you want to activate the popups from different locations.
        {
            HelpMarker("Text() elements don't have stable identifiers so we need to provide one.");
            static float value = 0.5f;
            RblxUI::Text("Value = %.3f <-- (1) right-click this text", value);
            if (RblxUI::BeginPopupContextItem("my popup"))
            {
                if (RblxUI::Selectable("Set to zero")) value = 0.0f;
                if (RblxUI::Selectable("Set to PI")) value = 3.1415f;
                RblxUI::SetNextItemWidth(-FLT_MIN);
                RblxUI::DragFloat("##Value", &value, 0.1f, 0.0f, 0.0f);
                RblxUI::EndPopup();
            }

            // We can also use OpenPopupOnItemClick() to toggle the visibility of a given popup.
            // Here we make it that right-clicking this other text element opens the same popup as above.
            // The popup itself will be submitted by the code above.
            RblxUI::Text("(2) Or right-click this text");
            RblxUI::OpenPopupOnItemClick("my popup", RblxUIPopupFlags_MouseButtonRight);

            // Back to square one: manually open the same popup.
            if (RblxUI::Button("(3) Or click this button"))
                RblxUI::OpenPopup("my popup");
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
            RblxUI::Button(buf);
            if (RblxUI::BeginPopupContextItem())
            {
                RblxUI::Text("Edit name:");
                RblxUI::InputText("##edit", name, RBLXUI_COUNTOF(name));
                if (RblxUI::Button("Close"))
                    RblxUI::CloseCurrentPopup();
                RblxUI::EndPopup();
            }
            RblxUI::SameLine(); RblxUI::Text("(<-- right-click here)");
        }

        RblxUI::TreePop();
    }

    if (RblxUI::TreeNode("Modals"))
    {
        RBLXUI_DEMO_MARKER("Popups/Modals");
        RblxUI::TextWrapped("Modal windows are like popups but the user cannot close them by clicking outside.");

        if (RblxUI::Button("Delete.."))
            RblxUI::OpenPopup("Delete?");

        // Always center this window when appearing
        RblxVec2 center = RblxUI::GetMainViewport()->GetCenter();
        RblxUI::SetNextWindowPos(center, RblxUICond_Appearing, RblxVec2(0.5f, 0.5f));

        if (RblxUI::BeginPopupModal("Delete?", NULL, RblxUIWindowFlags_AlwaysAutoResize))
        {
            RblxUI::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!");
            RblxUI::Separator();

            //static int unused_i = 0;
            //RblxUI::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

            static bool dont_ask_me_next_time = false;
            RblxUI::PushStyleVar(RblxUIStyleVar_FramePadding, RblxVec2(0, 0));
            RblxUI::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
            RblxUI::PopStyleVar();

            if (RblxUI::Button("OK", RblxVec2(120, 0))) { RblxUI::CloseCurrentPopup(); }
            RblxUI::SetItemDefaultFocus();
            RblxUI::SameLine();
            if (RblxUI::Button("Cancel", RblxVec2(120, 0))) { RblxUI::CloseCurrentPopup(); }
            RblxUI::EndPopup();
        }

        if (RblxUI::Button("Stacked modals.."))
            RblxUI::OpenPopup("Stacked 1");
        if (RblxUI::BeginPopupModal("Stacked 1", NULL, RblxUIWindowFlags_MenuBar))
        {
            if (RblxUI::BeginMenuBar())
            {
                if (RblxUI::BeginMenu("File"))
                {
                    if (RblxUI::MenuItem("Some menu item")) {}
                    RblxUI::EndMenu();
                }
                RblxUI::EndMenuBar();
            }
            RblxUI::Text("Hello from Stacked The First\nUsing style.Colors[RblxUICol_ModalWindowDimBg] behind it.");

            // Testing behavior of widgets stacking their own regular popups over the modal.
            static int item = 1;
            static float color[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
            RblxUI::Combo("Combo", &item, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");
            RblxUI::ColorEdit4("Color", color);

            if (RblxUI::Button("Add another modal.."))
                RblxUI::OpenPopup("Stacked 2");

            // Also demonstrate passing a bool* to BeginPopupModal(), this will create a regular close button which
            // will close the popup. Note that the visibility state of popups is owned by rblxui, so the input value
            // of the bool actually doesn't matter here.
            bool unused_open = true;
            if (RblxUI::BeginPopupModal("Stacked 2", &unused_open))
            {
                RblxUI::Text("Hello from Stacked The Second!");
                RblxUI::ColorEdit4("Color", color); // Allow opening another nested popup
                if (RblxUI::Button("Close"))
                    RblxUI::CloseCurrentPopup();
                RblxUI::EndPopup();
            }

            if (RblxUI::Button("Close"))
                RblxUI::CloseCurrentPopup();
            RblxUI::EndPopup();
        }

        RblxUI::TreePop();
    }

    if (RblxUI::TreeNode("Menus inside a regular window"))
    {
        RBLXUI_DEMO_MARKER("Popups/Menus inside a regular window");
        RblxUI::TextWrapped("Below we are testing adding menu items to a regular window. It's rather unusual but should work!");
        RblxUI::Separator();

        RblxUI::MenuItem("Menu item", "Ctrl+M");
        if (RblxUI::BeginMenu("Menu inside a regular window"))
        {
            ShowExampleMenuFile();
            RblxUI::EndMenu();
        }
        RblxUI::Separator();
        RblxUI::TreePop();
    }
}

// Dummy data structure that we use for the Table demo.
// (pre-C++11 doesn't allow us to instantiate RblxVector<MyItem> template if this structure is defined inside the demo function)
namespace
{
// We are passing our own identifier to TableSetupColumn() to facilitate identifying columns in the sorting code.
// This identifier will be passed down into RblxUITableSortSpec::ColumnUserID.
// But it is possible to omit the user id parameter of TableSetupColumn() and just use the column index instead! (RblxUITableSortSpec::ColumnIndex)
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
    // As we don't rely on std:: or other third-party library to compile dear rblxui, we only have reliable access to qsort(),
    // however qsort doesn't allow passing user data to comparing function.
    // As a workaround, we are storing the sort specs in a static/global for the comparing function to access.
    // In your own use case you would probably pass the sort specs to your sorting/comparing functions directly and not use a global.
    // We could technically call RblxUI::TableGetSortSpecs() in CompareWithSortSpecs(), but considering that this function is called
    // very often by the sorting algorithm it would be a little wasteful.
    static const RblxUITableSortSpecs* s_current_sort_specs;

    static void SortWithSortSpecs(RblxUITableSortSpecs* sort_specs, MyItem* items, int items_count)
    {
        s_current_sort_specs = sort_specs; // Store in variable accessible by the sort function.
        if (items_count > 1)
            qsort(items, (size_t)items_count, sizeof(items[0]), MyItem::CompareWithSortSpecs);
        s_current_sort_specs = NULL;
    }

    // Compare function to be used by qsort()
    static int RBLXUI_CDECL CompareWithSortSpecs(const void* lhs, const void* rhs)
    {
        const MyItem* a = (const MyItem*)lhs;
        const MyItem* b = (const MyItem*)rhs;
        for (int n = 0; n < s_current_sort_specs->SpecsCount; n++)
        {
            // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
            // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
            const RblxUITableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
            int delta = 0;
            switch (sort_spec->ColumnUserID)
            {
            case MyItemColumnID_ID:             delta = (a->ID - b->ID);                break;
            case MyItemColumnID_Name:           delta = (strcmp(a->Name, b->Name));     break;
            case MyItemColumnID_Quantity:       delta = (a->Quantity - b->Quantity);    break;
            case MyItemColumnID_Description:    delta = (strcmp(a->Name, b->Name));     break;
            default: RBLXUI_ASSERT(0); break;
            }
            if (delta > 0)
                return (sort_spec->SortDirection == RblxUISortDirection_Ascending) ? +1 : -1;
            if (delta < 0)
                return (sort_spec->SortDirection == RblxUISortDirection_Ascending) ? -1 : +1;
        }

        // qsort() is instable so always return a way to differentiate items.
        // Your own compare function may want to avoid fallback on implicit sort specs.
        // e.g. a Name compare if it wasn't already part of the sort specs.
        return a->ID - b->ID;
    }
};
const RblxUITableSortSpecs* MyItem::s_current_sort_specs = NULL;
}

// Make the UI compact because there are so many fields
static void PushStyleCompact()
{
    RblxUIStyle& style = RblxUI::GetStyle();
    RblxUI::PushStyleVarY(RblxUIStyleVar_FramePadding, (float)(int)(style.FramePadding.y * 0.60f));
    RblxUI::PushStyleVarY(RblxUIStyleVar_ItemSpacing, (float)(int)(style.ItemSpacing.y * 0.60f));
}

static void PopStyleCompact()
{
    RblxUI::PopStyleVar(2);
}

// Show a combo box with a choice of sizing policies
static void EditTableSizingFlags(RblxUITableFlags* p_flags)
{
    struct EnumDesc { RblxUITableFlags Value; const char* Name; const char* Tooltip; };
    static const EnumDesc policies[] =
    {
        { RblxUITableFlags_None,               "Default",                            "Use default sizing policy:\n- RblxUITableFlags_SizingFixedFit if ScrollX is on or if host window has RblxUIWindowFlags_AlwaysAutoResize.\n- RblxUITableFlags_SizingStretchSame otherwise." },
        { RblxUITableFlags_SizingFixedFit,     "RblxUITableFlags_SizingFixedFit",     "Columns default to _WidthFixed (if resizable) or _WidthAuto (if not resizable), matching contents width." },
        { RblxUITableFlags_SizingFixedSame,    "RblxUITableFlags_SizingFixedSame",    "Columns are all the same width, matching the maximum contents width.\nImplicitly disable RblxUITableFlags_Resizable and enable RblxUITableFlags_NoKeepColumnsVisible." },
        { RblxUITableFlags_SizingStretchProp,  "RblxUITableFlags_SizingStretchProp",  "Columns default to _WidthStretch with weights proportional to their widths." },
        { RblxUITableFlags_SizingStretchSame,  "RblxUITableFlags_SizingStretchSame",  "Columns default to _WidthStretch with same weights." }
    };
    int idx;
    for (idx = 0; idx < RBLXUI_COUNTOF(policies); idx++)
        if (policies[idx].Value == (*p_flags & RblxUITableFlags_SizingMask_))
            break;
    const char* preview_text = (idx < RBLXUI_COUNTOF(policies)) ? policies[idx].Name + (idx > 0 ? strlen("RblxUITableFlags") : 0) : "";
    if (RblxUI::BeginCombo("Sizing Policy", preview_text))
    {
        for (int n = 0; n < RBLXUI_COUNTOF(policies); n++)
            if (RblxUI::Selectable(policies[n].Name, idx == n))
                *p_flags = (*p_flags & ~RblxUITableFlags_SizingMask_) | policies[n].Value;
        RblxUI::EndCombo();
    }
    RblxUI::SameLine();
    RblxUI::TextDisabled("(?)");
    if (RblxUI::BeginItemTooltip())
    {
        RblxUI::PushTextWrapPos(RblxUI::GetFontSize() * 50.0f);
        for (int m = 0; m < RBLXUI_COUNTOF(policies); m++)
        {
            RblxUI::Separator();
            RblxUI::Text("%s:", policies[m].Name);
            RblxUI::Separator();
            RblxUI::SetCursorPosX(RblxUI::GetCursorPosX() + RblxUI::GetStyle().IndentSpacing * 0.5f);
            RblxUI::TextUnformatted(policies[m].Tooltip);
        }
        RblxUI::PopTextWrapPos();
        RblxUI::EndTooltip();
    }
}

static void EditTableColumnsFlags(RblxUITableColumnFlags* p_flags)
{
    RblxUI::CheckboxFlags("_Disabled", p_flags, RblxUITableColumnFlags_Disabled); RblxUI::SameLine(); HelpMarker("Master disable flag (also hide from context menu)");
    RblxUI::CheckboxFlags("_DefaultHide", p_flags, RblxUITableColumnFlags_DefaultHide);
    RblxUI::CheckboxFlags("_DefaultSort", p_flags, RblxUITableColumnFlags_DefaultSort);
    if (RblxUI::CheckboxFlags("_WidthStretch", p_flags, RblxUITableColumnFlags_WidthStretch))
        *p_flags &= ~(RblxUITableColumnFlags_WidthMask_ ^ RblxUITableColumnFlags_WidthStretch);
    if (RblxUI::CheckboxFlags("_WidthFixed", p_flags, RblxUITableColumnFlags_WidthFixed))
        *p_flags &= ~(RblxUITableColumnFlags_WidthMask_ ^ RblxUITableColumnFlags_WidthFixed);
    RblxUI::CheckboxFlags("_NoResize", p_flags, RblxUITableColumnFlags_NoResize);
    RblxUI::CheckboxFlags("_NoReorder", p_flags, RblxUITableColumnFlags_NoReorder);
    RblxUI::CheckboxFlags("_NoHide", p_flags, RblxUITableColumnFlags_NoHide);
    RblxUI::CheckboxFlags("_NoClip", p_flags, RblxUITableColumnFlags_NoClip);
    RblxUI::CheckboxFlags("_NoSort", p_flags, RblxUITableColumnFlags_NoSort);
    RblxUI::CheckboxFlags("_NoSortAscending", p_flags, RblxUITableColumnFlags_NoSortAscending);
    RblxUI::CheckboxFlags("_NoSortDescending", p_flags, RblxUITableColumnFlags_NoSortDescending);
    RblxUI::CheckboxFlags("_NoHeaderLabel", p_flags, RblxUITableColumnFlags_NoHeaderLabel);
    RblxUI::CheckboxFlags("_NoHeaderWidth", p_flags, RblxUITableColumnFlags_NoHeaderWidth);
    RblxUI::CheckboxFlags("_PreferSortAscending", p_flags, RblxUITableColumnFlags_PreferSortAscending);
    RblxUI::CheckboxFlags("_PreferSortDescending", p_flags, RblxUITableColumnFlags_PreferSortDescending);
    RblxUI::CheckboxFlags("_IndentEnable", p_flags, RblxUITableColumnFlags_IndentEnable); RblxUI::SameLine(); HelpMarker("Default for column 0");
    RblxUI::CheckboxFlags("_IndentDisable", p_flags, RblxUITableColumnFlags_IndentDisable); RblxUI::SameLine(); HelpMarker("Default for column >0");
    RblxUI::CheckboxFlags("_AngledHeader", p_flags, RblxUITableColumnFlags_AngledHeader);
}

static void ShowTableColumnsStatusFlags(RblxUITableColumnFlags flags)
{
    RblxUI::CheckboxFlags("_IsEnabled", &flags, RblxUITableColumnFlags_IsEnabled);
    RblxUI::CheckboxFlags("_IsVisible", &flags, RblxUITableColumnFlags_IsVisible);
    RblxUI::CheckboxFlags("_IsSorted", &flags, RblxUITableColumnFlags_IsSorted);
    RblxUI::CheckboxFlags("_IsHovered", &flags, RblxUITableColumnFlags_IsHovered);
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowTables()
//-----------------------------------------------------------------------------

static void DemoWindowTables()
{
    //RblxUI::SetNextItemOpen(true, RblxUICond_Once);
    if (!RblxUI::CollapsingHeader("Tables & Columns"))
        return;

    // Using those as a base value to create width/height that are factor of the size of our font
    const float TEXT_BASE_WIDTH = RblxUI::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = RblxUI::GetTextLineHeightWithSpacing();

    RblxUI::PushID("Tables");

    int open_action = -1;
    if (RblxUI::Button("Expand all"))
        open_action = 1;
    RblxUI::SameLine();
    if (RblxUI::Button("Collapse all"))
        open_action = 0;
    RblxUI::SameLine();

    // Options
    static bool disable_indent = false;
    RblxUI::Checkbox("Disable tree indentation", &disable_indent);
    RblxUI::SameLine();
    HelpMarker("Disable the indenting of tree nodes so demo tables can use the full window width.");
    RblxUI::Separator();
    if (disable_indent)
        RblxUI::PushStyleVar(RblxUIStyleVar_IndentSpacing, 0.0f);

    // About Styling of tables
    // Most settings are configured on a per-table basis via the flags passed to BeginTable() and TableSetupColumns APIs.
    // There are however a few settings that a shared and part of the RblxUIStyle structure:
    //   style.CellPadding                          // Padding within each cell
    //   style.Colors[RblxUICol_TableHeaderBg]       // Table header background
    //   style.Colors[RblxUICol_TableBorderStrong]   // Table outer and header borders
    //   style.Colors[RblxUICol_TableBorderLight]    // Table inner borders
    //   style.Colors[RblxUICol_TableRowBg]          // Table row background when RblxUITableFlags_RowBg is enabled (even rows)
    //   style.Colors[RblxUICol_TableRowBgAlt]       // Table row background when RblxUITableFlags_RowBg is enabled (odds rows)

    // Demos
    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Basic"))
    {
        RBLXUI_DEMO_MARKER("Tables/Basic");
        // Here we will showcase three different ways to output a table.
        // They are very simple variations of a same thing!

        // [Method 1] Using TableNextRow() to create a new row, and TableSetColumnIndex() to select the column.
        // In many situations, this is the most flexible and easy to use pattern.
        HelpMarker("Using TableNextRow() + calling TableSetColumnIndex() _before_ each cell, in a loop.");
        if (RblxUI::BeginTable("table1", 3))
        {
            for (int row = 0; row < 4; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    RblxUI::TableSetColumnIndex(column);
                    RblxUI::Text("Row %d Column %d", row, column);
                }
            }
            RblxUI::EndTable();
        }

        // [Method 2] Using TableNextColumn() called multiple times, instead of using a for loop + TableSetColumnIndex().
        // This is generally more convenient when you have code manually submitting the contents of each column.
        HelpMarker("Using TableNextRow() + calling TableNextColumn() _before_ each cell, manually.");
        if (RblxUI::BeginTable("table2", 3))
        {
            for (int row = 0; row < 4; row++)
            {
                RblxUI::TableNextRow();
                RblxUI::TableNextColumn();
                RblxUI::Text("Row %d", row);
                RblxUI::TableNextColumn();
                RblxUI::Text("Some contents");
                RblxUI::TableNextColumn();
                RblxUI::Text("123.456");
            }
            RblxUI::EndTable();
        }

        // [Method 3] We call TableNextColumn() _before_ each cell. We never call TableNextRow(),
        // as TableNextColumn() will automatically wrap around and create new rows as needed.
        // This is generally more convenient when your cells all contains the same type of data.
        HelpMarker(
            "Only using TableNextColumn(), which tends to be convenient for tables where every cell contains "
            "the same type of contents.\n This is also more similar to the old NextColumn() function of the "
            "Columns API, and provided to facilitate the Columns->Tables API transition.");
        if (RblxUI::BeginTable("table3", 3))
        {
            for (int item = 0; item < 14; item++)
            {
                RblxUI::TableNextColumn();
                RblxUI::Text("Item %d", item);
            }
            RblxUI::EndTable();
        }

        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Borders, background"))
    {
        RBLXUI_DEMO_MARKER("Tables/Borders, background");
        // Expose a few Borders related flags interactively
        enum ContentsType { CT_Text, CT_FillButton };
        static RblxUITableFlags flags = RblxUITableFlags_Borders | RblxUITableFlags_RowBg;
        static bool display_headers = false;
        static int contents_type = CT_Text;

        PushStyleCompact();
        RblxUI::CheckboxFlags("RblxUITableFlags_RowBg", &flags, RblxUITableFlags_RowBg);
        RblxUI::CheckboxFlags("RblxUITableFlags_Borders", &flags, RblxUITableFlags_Borders);
        RblxUI::SameLine(); HelpMarker("RblxUITableFlags_Borders\n = RblxUITableFlags_BordersInnerV\n | RblxUITableFlags_BordersOuterV\n | RblxUITableFlags_BordersInnerH\n | RblxUITableFlags_BordersOuterH");
        RblxUI::Indent();

        RblxUI::CheckboxFlags("RblxUITableFlags_BordersH", &flags, RblxUITableFlags_BordersH);
        RblxUI::Indent();
        RblxUI::CheckboxFlags("RblxUITableFlags_BordersOuterH", &flags, RblxUITableFlags_BordersOuterH);
        RblxUI::CheckboxFlags("RblxUITableFlags_BordersInnerH", &flags, RblxUITableFlags_BordersInnerH);
        RblxUI::Unindent();

        RblxUI::CheckboxFlags("RblxUITableFlags_BordersV", &flags, RblxUITableFlags_BordersV);
        RblxUI::Indent();
        RblxUI::CheckboxFlags("RblxUITableFlags_BordersOuterV", &flags, RblxUITableFlags_BordersOuterV);
        RblxUI::CheckboxFlags("RblxUITableFlags_BordersInnerV", &flags, RblxUITableFlags_BordersInnerV);
        RblxUI::Unindent();

        RblxUI::CheckboxFlags("RblxUITableFlags_BordersOuter", &flags, RblxUITableFlags_BordersOuter);
        RblxUI::CheckboxFlags("RblxUITableFlags_BordersInner", &flags, RblxUITableFlags_BordersInner);
        RblxUI::Unindent();

        RblxUI::AlignTextToFramePadding(); RblxUI::Text("Cell contents:");
        RblxUI::SameLine(); RblxUI::RadioButton("Text", &contents_type, CT_Text);
        RblxUI::SameLine(); RblxUI::RadioButton("FillButton", &contents_type, CT_FillButton);
        RblxUI::Checkbox("Display headers", &display_headers);
        RblxUI::CheckboxFlags("RblxUITableFlags_NoBordersInBody", &flags, RblxUITableFlags_NoBordersInBody); RblxUI::SameLine(); HelpMarker("Disable vertical borders in columns Body (borders will always appear in Headers)");
        PopStyleCompact();

        if (RblxUI::BeginTable("table1", 3, flags))
        {
            // Display headers so we can inspect their interaction with borders
            // (Headers are not the main purpose of this section of the demo, so we are not elaborating on them now. See other sections for details)
            if (display_headers)
            {
                RblxUI::TableSetupColumn("One");
                RblxUI::TableSetupColumn("Two");
                RblxUI::TableSetupColumn("Three");
                RblxUI::TableHeadersRow();
            }

            for (int row = 0; row < 5; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    RblxUI::TableSetColumnIndex(column);
                    char buf[32];
                    sprintf(buf, "Hello %d,%d", column, row);
                    if (contents_type == CT_Text)
                        RblxUI::TextUnformatted(buf);
                    else if (contents_type == CT_FillButton)
                        RblxUI::Button(buf, RblxVec2(-FLT_MIN, 0.0f));
                }
            }
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Resizable, stretch"))
    {
        RBLXUI_DEMO_MARKER("Tables/Resizable, stretch");
        // By default, if we don't enable ScrollX the sizing policy for each column is "Stretch"
        // All columns maintain a sizing weight, and they will occupy all available width.
        static RblxUITableFlags flags = RblxUITableFlags_SizingStretchSame | RblxUITableFlags_Resizable | RblxUITableFlags_BordersOuter | RblxUITableFlags_BordersV | RblxUITableFlags_ContextMenuInBody;
        PushStyleCompact();
        RblxUI::CheckboxFlags("RblxUITableFlags_Resizable", &flags, RblxUITableFlags_Resizable);
        RblxUI::CheckboxFlags("RblxUITableFlags_BordersV", &flags, RblxUITableFlags_BordersV);
        RblxUI::SameLine(); HelpMarker(
            "Using the _Resizable flag automatically enables the _BordersInnerV flag as well, "
            "this is why the resize borders are still showing when unchecking this.");
        PopStyleCompact();

        if (RblxUI::BeginTable("table1", 3, flags))
        {
            for (int row = 0; row < 5; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    RblxUI::TableSetColumnIndex(column);
                    RblxUI::Text("Hello %d,%d", column, row);
                }
            }
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Resizable, fixed"))
    {
        RBLXUI_DEMO_MARKER("Tables/Resizable, fixed");
        // Here we use RblxUITableFlags_SizingFixedFit (even though _ScrollX is not set)
        // So columns will adopt the "Fixed" policy and will maintain a fixed width regardless of the whole available width (unless table is small)
        // If there is not enough available width to fit all columns, they will however be resized down.
        // FIXME-TABLE: Providing a stretch-on-init would make sense especially for tables which don't have saved settings
        HelpMarker(
            "Using _Resizable + _SizingFixedFit flags.\n"
            "Fixed-width columns generally makes more sense if you want to use horizontal scrolling.\n\n"
            "Double-click a column border to auto-fit the column to its contents.");
        PushStyleCompact();
        static RblxUITableFlags flags = RblxUITableFlags_SizingFixedFit | RblxUITableFlags_Resizable | RblxUITableFlags_BordersOuter | RblxUITableFlags_BordersV | RblxUITableFlags_ContextMenuInBody;
        RblxUI::CheckboxFlags("RblxUITableFlags_NoHostExtendX", &flags, RblxUITableFlags_NoHostExtendX);
        PopStyleCompact();

        if (RblxUI::BeginTable("table1", 3, flags))
        {
            for (int row = 0; row < 5; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    RblxUI::TableSetColumnIndex(column);
                    RblxUI::Text("Hello %d,%d", column, row);
                }
            }
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Resizable, mixed"))
    {
        RBLXUI_DEMO_MARKER("Tables/Resizable, mixed");
        HelpMarker(
            "Using TableSetupColumn() to alter resizing policy on a per-column basis.\n\n"
            "When combining Fixed and Stretch columns, generally you only want one, maybe two trailing columns to use _WidthStretch.");
        static RblxUITableFlags flags = RblxUITableFlags_SizingFixedFit | RblxUITableFlags_RowBg | RblxUITableFlags_Borders | RblxUITableFlags_Resizable | RblxUITableFlags_Reorderable | RblxUITableFlags_Hideable;

        if (RblxUI::BeginTable("table1", 3, flags))
        {
            RblxUI::TableSetupColumn("AAA", RblxUITableColumnFlags_WidthFixed);
            RblxUI::TableSetupColumn("BBB", RblxUITableColumnFlags_WidthFixed);
            RblxUI::TableSetupColumn("CCC", RblxUITableColumnFlags_WidthStretch);
            RblxUI::TableHeadersRow();
            for (int row = 0; row < 5; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    RblxUI::TableSetColumnIndex(column);
                    RblxUI::Text("%s %d,%d", (column == 2) ? "Stretch" : "Fixed", column, row);
                }
            }
            RblxUI::EndTable();
        }
        if (RblxUI::BeginTable("table2", 6, flags))
        {
            RblxUI::TableSetupColumn("AAA", RblxUITableColumnFlags_WidthFixed);
            RblxUI::TableSetupColumn("BBB", RblxUITableColumnFlags_WidthFixed);
            RblxUI::TableSetupColumn("CCC", RblxUITableColumnFlags_WidthFixed | RblxUITableColumnFlags_DefaultHide);
            RblxUI::TableSetupColumn("DDD", RblxUITableColumnFlags_WidthStretch);
            RblxUI::TableSetupColumn("EEE", RblxUITableColumnFlags_WidthStretch);
            RblxUI::TableSetupColumn("FFF", RblxUITableColumnFlags_WidthStretch | RblxUITableColumnFlags_DefaultHide);
            RblxUI::TableHeadersRow();
            for (int row = 0; row < 5; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 6; column++)
                {
                    RblxUI::TableSetColumnIndex(column);
                    RblxUI::Text("%s %d,%d", (column >= 3) ? "Stretch" : "Fixed", column, row);
                }
            }
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Reorderable, hideable, with headers"))
    {
        RBLXUI_DEMO_MARKER("Tables/Reorderable, hideable, with headers");
        HelpMarker(
            "Click and drag column headers to reorder columns.\n\n"
            "Right-click on a header to open a context menu.");
        static RblxUITableFlags flags = RblxUITableFlags_Resizable | RblxUITableFlags_Reorderable | RblxUITableFlags_Hideable | RblxUITableFlags_BordersOuter | RblxUITableFlags_BordersV;
        PushStyleCompact();
        RblxUI::CheckboxFlags("RblxUITableFlags_Resizable", &flags, RblxUITableFlags_Resizable);
        RblxUI::CheckboxFlags("RblxUITableFlags_Reorderable", &flags, RblxUITableFlags_Reorderable);
        RblxUI::CheckboxFlags("RblxUITableFlags_Hideable", &flags, RblxUITableFlags_Hideable);
        RblxUI::CheckboxFlags("RblxUITableFlags_NoBordersInBody", &flags, RblxUITableFlags_NoBordersInBody);
        RblxUI::CheckboxFlags("RblxUITableFlags_NoBordersInBodyUntilResize", &flags, RblxUITableFlags_NoBordersInBodyUntilResize); RblxUI::SameLine(); HelpMarker("Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers)");
        RblxUI::CheckboxFlags("RblxUITableFlags_HighlightHoveredColumn", &flags, RblxUITableFlags_HighlightHoveredColumn);
        PopStyleCompact();

        if (RblxUI::BeginTable("table1", 3, flags))
        {
            // Submit columns name with TableSetupColumn() and call TableHeadersRow() to create a row with a header in each column.
            // (Later we will show how TableSetupColumn() has other uses, optional flags, sizing weight etc.)
            RblxUI::TableSetupColumn("One");
            RblxUI::TableSetupColumn("Two");
            RblxUI::TableSetupColumn("Three");
            RblxUI::TableHeadersRow();
            for (int row = 0; row < 6; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    RblxUI::TableSetColumnIndex(column);
                    RblxUI::Text("Hello %d,%d", column, row);
                }
            }
            RblxUI::EndTable();
        }

        // Use outer_size.x == 0.0f instead of default to make the table as tight as possible
        // (only valid when no scrolling and no stretch column)
        if (RblxUI::BeginTable("table2", 3, flags | RblxUITableFlags_SizingFixedFit, RblxVec2(0.0f, 0.0f)))
        {
            RblxUI::TableSetupColumn("One");
            RblxUI::TableSetupColumn("Two");
            RblxUI::TableSetupColumn("Three");
            RblxUI::TableHeadersRow();
            for (int row = 0; row < 6; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    RblxUI::TableSetColumnIndex(column);
                    RblxUI::Text("Fixed %d,%d", column, row);
                }
            }
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Padding"))
    {
        RBLXUI_DEMO_MARKER("Tables/Padding");
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

        static RblxUITableFlags flags1 = RblxUITableFlags_BordersV;
        PushStyleCompact();
        RblxUI::CheckboxFlags("RblxUITableFlags_PadOuterX", &flags1, RblxUITableFlags_PadOuterX);
        RblxUI::SameLine(); HelpMarker("Enable outer-most padding (default if RblxUITableFlags_BordersOuterV is set)");
        RblxUI::CheckboxFlags("RblxUITableFlags_NoPadOuterX", &flags1, RblxUITableFlags_NoPadOuterX);
        RblxUI::SameLine(); HelpMarker("Disable outer-most padding (default if RblxUITableFlags_BordersOuterV is not set)");
        RblxUI::CheckboxFlags("RblxUITableFlags_NoPadInnerX", &flags1, RblxUITableFlags_NoPadInnerX);
        RblxUI::SameLine(); HelpMarker("Disable inner padding between columns (double inner padding if BordersOuterV is on, single inner padding if BordersOuterV is off)");
        RblxUI::CheckboxFlags("RblxUITableFlags_BordersOuterV", &flags1, RblxUITableFlags_BordersOuterV);
        RblxUI::CheckboxFlags("RblxUITableFlags_BordersInnerV", &flags1, RblxUITableFlags_BordersInnerV);
        static bool show_headers = false;
        RblxUI::Checkbox("show_headers", &show_headers);
        PopStyleCompact();

        if (RblxUI::BeginTable("table_padding", 3, flags1))
        {
            if (show_headers)
            {
                RblxUI::TableSetupColumn("One");
                RblxUI::TableSetupColumn("Two");
                RblxUI::TableSetupColumn("Three");
                RblxUI::TableHeadersRow();
            }

            for (int row = 0; row < 5; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    RblxUI::TableSetColumnIndex(column);
                    if (row == 0)
                    {
                        RblxUI::Text("Avail %.2f", RblxUI::GetContentRegionAvail().x);
                    }
                    else
                    {
                        char buf[32];
                        sprintf(buf, "Hello %d,%d", column, row);
                        RblxUI::Button(buf, RblxVec2(-FLT_MIN, 0.0f));
                    }
                    //if (RblxUI::TableGetColumnFlags() & RblxUITableColumnFlags_IsHovered)
                    //    RblxUI::TableSetBgColor(RblxUITableBgTarget_CellBg, RBLXUI_COL32(0, 100, 0, 255));
                }
            }
            RblxUI::EndTable();
        }

        // Second example: set style.CellPadding to (0.0) or a custom value.
        // FIXME-TABLE: Vertical border effectively not displayed the same way as horizontal one...
        HelpMarker("Setting style.CellPadding to (0,0) or a custom value.");
        static RblxUITableFlags flags2 = RblxUITableFlags_Borders | RblxUITableFlags_RowBg;
        static RblxVec2 cell_padding(0.0f, 0.0f);
        static bool show_widget_frame_bg = true;

        PushStyleCompact();
        RblxUI::CheckboxFlags("RblxUITableFlags_Borders", &flags2, RblxUITableFlags_Borders);
        RblxUI::CheckboxFlags("RblxUITableFlags_BordersH", &flags2, RblxUITableFlags_BordersH);
        RblxUI::CheckboxFlags("RblxUITableFlags_BordersV", &flags2, RblxUITableFlags_BordersV);
        RblxUI::CheckboxFlags("RblxUITableFlags_BordersInner", &flags2, RblxUITableFlags_BordersInner);
        RblxUI::CheckboxFlags("RblxUITableFlags_BordersOuter", &flags2, RblxUITableFlags_BordersOuter);
        RblxUI::CheckboxFlags("RblxUITableFlags_RowBg", &flags2, RblxUITableFlags_RowBg);
        RblxUI::CheckboxFlags("RblxUITableFlags_Resizable", &flags2, RblxUITableFlags_Resizable);
        RblxUI::Checkbox("show_widget_frame_bg", &show_widget_frame_bg);
        RblxUI::SliderFloat2("CellPadding", &cell_padding.x, 0.0f, 10.0f, "%.0f");
        PopStyleCompact();

        RblxUI::PushStyleVar(RblxUIStyleVar_CellPadding, cell_padding);
        if (RblxUI::BeginTable("table_padding_2", 3, flags2))
        {
            static char text_bufs[3 * 5][16]; // Mini text storage for 3x5 cells
            static bool init = true;
            if (!show_widget_frame_bg)
                RblxUI::PushStyleColor(RblxUICol_FrameBg, 0);
            for (int cell = 0; cell < 3 * 5; cell++)
            {
                RblxUI::TableNextColumn();
                if (init)
                    strcpy(text_bufs[cell], "edit me");
                RblxUI::SetNextItemWidth(-FLT_MIN);
                RblxUI::PushID(cell);
                RblxUI::InputText("##cell", text_bufs[cell], RBLXUI_COUNTOF(text_bufs[cell]));
                RblxUI::PopID();
            }
            if (!show_widget_frame_bg)
                RblxUI::PopStyleColor();
            init = false;
            RblxUI::EndTable();
        }
        RblxUI::PopStyleVar();

        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Sizing policies"))
    {
        RBLXUI_DEMO_MARKER("Tables/Explicit widths");
        static RblxUITableFlags flags1 = RblxUITableFlags_BordersV | RblxUITableFlags_BordersOuterH | RblxUITableFlags_RowBg | RblxUITableFlags_ContextMenuInBody;
        PushStyleCompact();
        RblxUI::CheckboxFlags("RblxUITableFlags_Resizable", &flags1, RblxUITableFlags_Resizable);
        RblxUI::CheckboxFlags("RblxUITableFlags_NoHostExtendX", &flags1, RblxUITableFlags_NoHostExtendX);
        PopStyleCompact();

        static RblxUITableFlags sizing_policy_flags[4] = { RblxUITableFlags_SizingFixedFit, RblxUITableFlags_SizingFixedSame, RblxUITableFlags_SizingStretchProp, RblxUITableFlags_SizingStretchSame };
        for (int table_n = 0; table_n < 4; table_n++)
        {
            RblxUI::PushID(table_n);
            RblxUI::SetNextItemWidth(TEXT_BASE_WIDTH * 30);
            EditTableSizingFlags(&sizing_policy_flags[table_n]);

            // To make it easier to understand the different sizing policy,
            // For each policy: we display one table where the columns have equal contents width,
            // and one where the columns have different contents width.
            if (RblxUI::BeginTable("table1", 3, sizing_policy_flags[table_n] | flags1))
            {
                for (int row = 0; row < 3; row++)
                {
                    RblxUI::TableNextRow();
                    RblxUI::TableNextColumn(); RblxUI::Text("Oh dear");
                    RblxUI::TableNextColumn(); RblxUI::Text("Oh dear");
                    RblxUI::TableNextColumn(); RblxUI::Text("Oh dear");
                }
                RblxUI::EndTable();
            }
            if (RblxUI::BeginTable("table2", 3, sizing_policy_flags[table_n] | flags1))
            {
                for (int row = 0; row < 3; row++)
                {
                    RblxUI::TableNextRow();
                    RblxUI::TableNextColumn(); RblxUI::Text("AAAA");
                    RblxUI::TableNextColumn(); RblxUI::Text("BBBBBBBB");
                    RblxUI::TableNextColumn(); RblxUI::Text("CCCCCCCCCCCC");
                }
                RblxUI::EndTable();
            }
            RblxUI::PopID();
        }

        RblxUI::Spacing();
        RblxUI::TextUnformatted("Advanced");
        RblxUI::SameLine();
        HelpMarker(
            "This section allows you to interact and see the effect of various sizing policies "
            "depending on whether Scroll is enabled and the contents of your columns.");

        enum ContentsType { CT_ShowWidth, CT_ShortText, CT_LongText, CT_Button, CT_FillButton, CT_InputText };
        static RblxUITableFlags flags = RblxUITableFlags_ScrollY | RblxUITableFlags_Borders | RblxUITableFlags_RowBg | RblxUITableFlags_Resizable;
        static int contents_type = CT_ShowWidth;
        static int column_count = 3;

        PushStyleCompact();
        RblxUI::PushID("Advanced");
        RblxUI::PushItemWidth(TEXT_BASE_WIDTH * 30);
        EditTableSizingFlags(&flags);
        RblxUI::Combo("Contents", &contents_type, "Show width\0Short Text\0Long Text\0Button\0Fill Button\0InputText\0");
        if (contents_type == CT_FillButton)
        {
            RblxUI::SameLine();
            HelpMarker(
                "Be mindful that using right-alignment (e.g. size.x = -FLT_MIN) creates a feedback loop "
                "where contents width can feed into auto-column width can feed into contents width.");
        }
        RblxUI::DragInt("Columns", &column_count, 0.1f, 1, 64, "%d", RblxUISliderFlags_AlwaysClamp);
        RblxUI::CheckboxFlags("RblxUITableFlags_Resizable", &flags, RblxUITableFlags_Resizable);
        RblxUI::CheckboxFlags("RblxUITableFlags_PreciseWidths", &flags, RblxUITableFlags_PreciseWidths);
        RblxUI::SameLine(); HelpMarker("Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.");
        RblxUI::CheckboxFlags("RblxUITableFlags_ScrollX", &flags, RblxUITableFlags_ScrollX);
        RblxUI::CheckboxFlags("RblxUITableFlags_ScrollY", &flags, RblxUITableFlags_ScrollY);
        RblxUI::CheckboxFlags("RblxUITableFlags_NoClip", &flags, RblxUITableFlags_NoClip);
        RblxUI::PopItemWidth();
        RblxUI::PopID();
        PopStyleCompact();

        if (RblxUI::BeginTable("table2", column_count, flags, RblxVec2(0.0f, TEXT_BASE_HEIGHT * 7)))
        {
            for (int cell = 0; cell < 10 * column_count; cell++)
            {
                RblxUI::TableNextColumn();
                int column = RblxUI::TableGetColumnIndex();
                int row = RblxUI::TableGetRowIndex();

                RblxUI::PushID(cell);
                char label[32];
                static char text_buf[32] = "";
                sprintf(label, "Hello %d,%d", column, row);
                switch (contents_type)
                {
                case CT_ShortText:  RblxUI::TextUnformatted(label); break;
                case CT_LongText:   RblxUI::Text("Some %s text %d,%d\nOver two lines..", column == 0 ? "long" : "longeeer", column, row); break;
                case CT_ShowWidth:  RblxUI::Text("W: %.1f", RblxUI::GetContentRegionAvail().x); break;
                case CT_Button:     RblxUI::Button(label); break;
                case CT_FillButton: RblxUI::Button(label, RblxVec2(-FLT_MIN, 0.0f)); break;
                case CT_InputText:  RblxUI::SetNextItemWidth(-FLT_MIN); RblxUI::InputText("##", text_buf, RBLXUI_COUNTOF(text_buf)); break;
                }
                RblxUI::PopID();
            }
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Vertical scrolling, with clipping"))
    {
        RBLXUI_DEMO_MARKER("Tables/Vertical scrolling, with clipping");
        HelpMarker(
            "Here we activate ScrollY, which will create a child window container to allow hosting scrollable contents.\n\n"
            "We also demonstrate using RblxUIListClipper to virtualize the submission of many items.");
        static RblxUITableFlags flags = RblxUITableFlags_ScrollY | RblxUITableFlags_RowBg | RblxUITableFlags_BordersOuter | RblxUITableFlags_BordersV | RblxUITableFlags_Resizable | RblxUITableFlags_Reorderable | RblxUITableFlags_Hideable;

        PushStyleCompact();
        RblxUI::CheckboxFlags("RblxUITableFlags_ScrollY", &flags, RblxUITableFlags_ScrollY);
        PopStyleCompact();

        // When using ScrollX or ScrollY we need to specify a size for our table container!
        // Otherwise by default the table will fit all available space, like a BeginChild() call.
        RblxVec2 outer_size = RblxVec2(0.0f, TEXT_BASE_HEIGHT * 8);
        if (RblxUI::BeginTable("table_scrolly", 3, flags, outer_size))
        {
            RblxUI::TableSetupScrollFreeze(0, 1); // Make top row always visible
            RblxUI::TableSetupColumn("One", RblxUITableColumnFlags_None);
            RblxUI::TableSetupColumn("Two", RblxUITableColumnFlags_None);
            RblxUI::TableSetupColumn("Three", RblxUITableColumnFlags_None);
            RblxUI::TableHeadersRow();

            // Demonstrate using clipper for large vertical lists
            RblxUIListClipper clipper;
            clipper.Begin(1000);
            while (clipper.Step())
            {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                {
                    RblxUI::TableNextRow();
                    for (int column = 0; column < 3; column++)
                    {
                        RblxUI::TableSetColumnIndex(column);
                        RblxUI::Text("Hello %d,%d", column, row);
                    }
                }
            }
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Horizontal scrolling"))
    {
        RBLXUI_DEMO_MARKER("Tables/Horizontal scrolling");
        HelpMarker(
            "When ScrollX is enabled, the default sizing policy becomes RblxUITableFlags_SizingFixedFit, "
            "as automatically stretching columns doesn't make much sense with horizontal scrolling.\n\n"
            "Also note that as of the current version, you will almost always want to enable ScrollY along with ScrollX, "
            "because the container window won't automatically extend vertically to fix contents "
            "(this may be improved in future versions).");
        static RblxUITableFlags flags = RblxUITableFlags_ScrollX | RblxUITableFlags_ScrollY | RblxUITableFlags_RowBg | RblxUITableFlags_BordersOuter | RblxUITableFlags_BordersV | RblxUITableFlags_Resizable | RblxUITableFlags_Reorderable | RblxUITableFlags_Hideable;
        static int freeze_cols = 1;
        static int freeze_rows = 1;

        PushStyleCompact();
        RblxUI::CheckboxFlags("RblxUITableFlags_Resizable", &flags, RblxUITableFlags_Resizable);
        RblxUI::CheckboxFlags("RblxUITableFlags_ScrollX", &flags, RblxUITableFlags_ScrollX);
        RblxUI::CheckboxFlags("RblxUITableFlags_ScrollY", &flags, RblxUITableFlags_ScrollY);
        RblxUI::SetNextItemWidth(RblxUI::GetFrameHeight());
        RblxUI::DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL, RblxUISliderFlags_NoInput);
        RblxUI::SetNextItemWidth(RblxUI::GetFrameHeight());
        RblxUI::DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL, RblxUISliderFlags_NoInput);
        PopStyleCompact();

        // When using ScrollX or ScrollY we need to specify a size for our table container!
        // Otherwise by default the table will fit all available space, like a BeginChild() call.
        RblxVec2 outer_size = RblxVec2(0.0f, TEXT_BASE_HEIGHT * 8);
        if (RblxUI::BeginTable("table_scrollx", 7, flags, outer_size))
        {
            RblxUI::TableSetupScrollFreeze(freeze_cols, freeze_rows);
            RblxUI::TableSetupColumn("Line #", RblxUITableColumnFlags_NoHide); // Make the first column not hideable to match our use of TableSetupScrollFreeze()
            RblxUI::TableSetupColumn("One");
            RblxUI::TableSetupColumn("Two");
            RblxUI::TableSetupColumn("Three");
            RblxUI::TableSetupColumn("Four");
            RblxUI::TableSetupColumn("Five");
            RblxUI::TableSetupColumn("Six");
            RblxUI::TableHeadersRow();
            for (int row = 0; row < 20; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 7; column++)
                {
                    // Both TableNextColumn() and TableSetColumnIndex() return true when a column is visible or performing width measurement.
                    // Because here we know that:
                    // - A) all our columns are contributing the same to row height
                    // - B) column 0 is always visible,
                    // We only always submit this one column and can skip others.
                    // More advanced per-column clipping behaviors may benefit from polling the status flags via TableGetColumnFlags().
                    if (!RblxUI::TableSetColumnIndex(column) && column > 0)
                        continue;
                    if (column == 0)
                        RblxUI::Text("Line %d", row);
                    else
                        RblxUI::Text("Hello world %d,%d", column, row);
                }
            }
            RblxUI::EndTable();
        }

        RblxUI::Spacing();
        RblxUI::TextUnformatted("Stretch + ScrollX");
        RblxUI::SameLine();
        HelpMarker(
            "Showcase using Stretch columns + ScrollX together: "
            "this is rather unusual and only makes sense when specifying an 'inner_width' for the table!\n"
            "Without an explicit value, inner_width is == outer_size.x and therefore using Stretch columns "
            "along with ScrollX doesn't make sense.");
        static RblxUITableFlags flags2 = RblxUITableFlags_SizingStretchSame | RblxUITableFlags_ScrollX | RblxUITableFlags_ScrollY | RblxUITableFlags_BordersOuter | RblxUITableFlags_RowBg | RblxUITableFlags_ContextMenuInBody;
        static float inner_width = 1000.0f;
        PushStyleCompact();
        RblxUI::PushID("flags3");
        RblxUI::PushItemWidth(TEXT_BASE_WIDTH * 30);
        RblxUI::CheckboxFlags("RblxUITableFlags_ScrollX", &flags2, RblxUITableFlags_ScrollX);
        RblxUI::DragFloat("inner_width", &inner_width, 1.0f, 0.0f, FLT_MAX, "%.1f");
        RblxUI::PopItemWidth();
        RblxUI::PopID();
        PopStyleCompact();
        if (RblxUI::BeginTable("table2", 7, flags2, outer_size, inner_width))
        {
            for (int cell = 0; cell < 20 * 7; cell++)
            {
                RblxUI::TableNextColumn();
                RblxUI::Text("Hello world %d,%d", RblxUI::TableGetColumnIndex(), RblxUI::TableGetRowIndex());
            }
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Columns flags"))
    {
        RBLXUI_DEMO_MARKER("Tables/Columns flags");
        // Create a first table just to show all the options/flags we want to make visible in our example!
        const int column_count = 3;
        const char* column_names[column_count] = { "One", "Two", "Three" };
        static RblxUITableColumnFlags column_flags[column_count] = { RblxUITableColumnFlags_DefaultSort, RblxUITableColumnFlags_None, RblxUITableColumnFlags_DefaultHide };
        static RblxUITableColumnFlags column_flags_out[column_count] = { 0, 0, 0 }; // Output from TableGetColumnFlags()

        if (RblxUI::BeginTable("table_columns_flags_checkboxes", column_count, RblxUITableFlags_None))
        {
            PushStyleCompact();
            for (int column = 0; column < column_count; column++)
            {
                RblxUI::TableNextColumn();
                RblxUI::PushID(column);
                RblxUI::AlignTextToFramePadding(); // FIXME-TABLE: Workaround for wrong text baseline propagation across columns
                RblxUI::Text("'%s'", column_names[column]);
                RblxUI::Spacing();
                RblxUI::Text("Input flags:");
                EditTableColumnsFlags(&column_flags[column]);
                RblxUI::Spacing();
                RblxUI::Text("Output flags:");
                RblxUI::BeginDisabled();
                ShowTableColumnsStatusFlags(column_flags_out[column]);
                RblxUI::EndDisabled();
                RblxUI::PopID();
            }
            PopStyleCompact();
            RblxUI::EndTable();
        }

        // Create the real table we care about for the example!
        // We use a scrolling table to be able to showcase the difference between the _IsEnabled and _IsVisible flags above,
        // otherwise in a non-scrolling table columns are always visible (unless using RblxUITableFlags_NoKeepColumnsVisible
        // + resizing the parent window down).
        const RblxUITableFlags flags
            = RblxUITableFlags_SizingFixedFit | RblxUITableFlags_ScrollX | RblxUITableFlags_ScrollY
            | RblxUITableFlags_RowBg | RblxUITableFlags_BordersOuter | RblxUITableFlags_BordersV
            | RblxUITableFlags_Resizable | RblxUITableFlags_Reorderable | RblxUITableFlags_Hideable | RblxUITableFlags_Sortable;
        RblxVec2 outer_size = RblxVec2(0.0f, TEXT_BASE_HEIGHT * 9);
        if (RblxUI::BeginTable("table_columns_flags", column_count, flags, outer_size))
        {
            bool has_angled_header = false;
            for (int column = 0; column < column_count; column++)
            {
                has_angled_header |= (column_flags[column] & RblxUITableColumnFlags_AngledHeader) != 0;
                RblxUI::TableSetupColumn(column_names[column], column_flags[column]);
            }
            if (has_angled_header)
                RblxUI::TableAngledHeadersRow();
            RblxUI::TableHeadersRow();
            for (int column = 0; column < column_count; column++)
                column_flags_out[column] = RblxUI::TableGetColumnFlags(column);
            float indent_step = (float)((int)TEXT_BASE_WIDTH / 2);
            for (int row = 0; row < 8; row++)
            {
                // Add some indentation to demonstrate usage of per-column IndentEnable/IndentDisable flags.
                RblxUI::Indent(indent_step);
                RblxUI::TableNextRow();
                for (int column = 0; column < column_count; column++)
                {
                    RblxUI::TableSetColumnIndex(column);
                    RblxUI::Text("%s %s", (column == 0) ? "Indented" : "Hello", RblxUI::TableGetColumnName(column));
                }
            }
            RblxUI::Unindent(indent_step * 8.0f);

            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Columns widths"))
    {
        RBLXUI_DEMO_MARKER("Tables/Columns widths");
        HelpMarker("Using TableSetupColumn() to setup default width.");

        static RblxUITableFlags flags1 = RblxUITableFlags_Borders | RblxUITableFlags_NoBordersInBodyUntilResize;
        PushStyleCompact();
        RblxUI::CheckboxFlags("RblxUITableFlags_Resizable", &flags1, RblxUITableFlags_Resizable);
        RblxUI::CheckboxFlags("RblxUITableFlags_NoBordersInBodyUntilResize", &flags1, RblxUITableFlags_NoBordersInBodyUntilResize);
        PopStyleCompact();
        if (RblxUI::BeginTable("table1", 3, flags1))
        {
            // We could also set RblxUITableFlags_SizingFixedFit on the table and all columns will default to RblxUITableColumnFlags_WidthFixed.
            RblxUI::TableSetupColumn("one", RblxUITableColumnFlags_WidthFixed, 100.0f); // Default to 100.0f
            RblxUI::TableSetupColumn("two", RblxUITableColumnFlags_WidthFixed, 200.0f); // Default to 200.0f
            RblxUI::TableSetupColumn("three", RblxUITableColumnFlags_WidthFixed);       // Default to auto
            RblxUI::TableHeadersRow();
            for (int row = 0; row < 4; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    RblxUI::TableSetColumnIndex(column);
                    if (row == 0)
                        RblxUI::Text("(w: %5.1f)", RblxUI::GetContentRegionAvail().x);
                    else
                        RblxUI::Text("Hello %d,%d", column, row);
                }
            }
            RblxUI::EndTable();
        }

        HelpMarker(
            "Using TableSetupColumn() to setup explicit width.\n\nUnless _NoKeepColumnsVisible is set, "
            "fixed columns with set width may still be shrunk down if there's not enough space in the host.");

        static RblxUITableFlags flags2 = RblxUITableFlags_None;
        PushStyleCompact();
        RblxUI::CheckboxFlags("RblxUITableFlags_NoKeepColumnsVisible", &flags2, RblxUITableFlags_NoKeepColumnsVisible);
        RblxUI::CheckboxFlags("RblxUITableFlags_BordersInnerV", &flags2, RblxUITableFlags_BordersInnerV);
        RblxUI::CheckboxFlags("RblxUITableFlags_BordersOuterV", &flags2, RblxUITableFlags_BordersOuterV);
        PopStyleCompact();
        if (RblxUI::BeginTable("table2", 4, flags2))
        {
            // We could also set RblxUITableFlags_SizingFixedFit on the table and then all columns
            // will default to RblxUITableColumnFlags_WidthFixed.
            RblxUI::TableSetupColumn("", RblxUITableColumnFlags_WidthFixed, 100.0f);
            RblxUI::TableSetupColumn("", RblxUITableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 15.0f);
            RblxUI::TableSetupColumn("", RblxUITableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 30.0f);
            RblxUI::TableSetupColumn("", RblxUITableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 15.0f);
            for (int row = 0; row < 5; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 4; column++)
                {
                    RblxUI::TableSetColumnIndex(column);
                    if (row == 0)
                        RblxUI::Text("(w: %5.1f)", RblxUI::GetContentRegionAvail().x);
                    else
                        RblxUI::Text("Hello %d,%d", column, row);
                }
            }
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Nested tables"))
    {
        RBLXUI_DEMO_MARKER("Tables/Nested tables");
        HelpMarker("This demonstrates embedding a table into another table cell.");

        if (RblxUI::BeginTable("table_nested1", 2, RblxUITableFlags_Borders | RblxUITableFlags_Resizable | RblxUITableFlags_Reorderable | RblxUITableFlags_Hideable))
        {
            RblxUI::TableSetupColumn("A0");
            RblxUI::TableSetupColumn("A1");
            RblxUI::TableHeadersRow();

            RblxUI::TableNextColumn();
            RblxUI::Text("A0 Row 0");
            {
                float rows_height = (TEXT_BASE_HEIGHT * 2.0f) + (RblxUI::GetStyle().CellPadding.y * 2.0f);
                if (RblxUI::BeginTable("table_nested2", 2, RblxUITableFlags_Borders | RblxUITableFlags_Resizable | RblxUITableFlags_Reorderable | RblxUITableFlags_Hideable))
                {
                    RblxUI::TableSetupColumn("B0");
                    RblxUI::TableSetupColumn("B1");
                    RblxUI::TableHeadersRow();

                    RblxUI::TableNextRow(RblxUITableRowFlags_None, rows_height);
                    RblxUI::TableNextColumn();
                    RblxUI::Text("B0 Row 0");
                    RblxUI::TableNextColumn();
                    RblxUI::Text("B1 Row 0");
                    RblxUI::TableNextRow(RblxUITableRowFlags_None, rows_height);
                    RblxUI::TableNextColumn();
                    RblxUI::Text("B0 Row 1");
                    RblxUI::TableNextColumn();
                    RblxUI::Text("B1 Row 1");

                    RblxUI::EndTable();
                }
            }
            RblxUI::TableNextColumn(); RblxUI::Text("A1 Row 0");
            RblxUI::TableNextColumn(); RblxUI::Text("A0 Row 1");
            RblxUI::TableNextColumn(); RblxUI::Text("A1 Row 1");
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Row height"))
    {
        RBLXUI_DEMO_MARKER("Tables/Row height");
        HelpMarker(
            "You can pass a 'min_row_height' to TableNextRow().\n\nRows are padded with 'style.CellPadding.y' on top and bottom, "
            "so effectively the minimum row height will always be >= 'style.CellPadding.y * 2.0f'.\n\n"
            "We cannot honor a _maximum_ row height as that would require a unique clipping rectangle per row.");
        if (RblxUI::BeginTable("table_row_height", 1, RblxUITableFlags_Borders))
        {
            for (int row = 0; row < 8; row++)
            {
                float min_row_height = (float)(int)(TEXT_BASE_HEIGHT * 0.30f * row + RblxUI::GetStyle().CellPadding.y * 2.0f);
                RblxUI::TableNextRow(RblxUITableRowFlags_None, min_row_height);
                RblxUI::TableNextColumn();
                RblxUI::Text("min_row_height = %.2f", min_row_height);
            }
            RblxUI::EndTable();
        }

        HelpMarker(
            "Showcase using SameLine(0,0) to share Current Line Height between cells.\n\n"
            "Please note that Tables Row Height is not the same thing as Current Line Height, "
            "as a table cell may contains multiple lines.");
        if (RblxUI::BeginTable("table_share_lineheight", 2, RblxUITableFlags_Borders))
        {
            RblxUI::TableNextRow();
            RblxUI::TableNextColumn();
            RblxUI::ColorButton("##1", RblxVec4(0.13f, 0.26f, 0.40f, 1.0f), RblxUIColorEditFlags_None, RblxVec2(40, 40));
            RblxUI::TableNextColumn();
            RblxUI::Text("Line 1");
            RblxUI::Text("Line 2");

            RblxUI::TableNextRow();
            RblxUI::TableNextColumn();
            RblxUI::ColorButton("##2", RblxVec4(0.13f, 0.26f, 0.40f, 1.0f), RblxUIColorEditFlags_None, RblxVec2(40, 40));
            RblxUI::TableNextColumn();
            RblxUI::SameLine(0.0f, 0.0f); // Reuse line height from previous column
            RblxUI::Text("Line 1, with SameLine(0,0)");
            RblxUI::Text("Line 2");

            RblxUI::EndTable();
        }

        HelpMarker("Showcase altering CellPadding.y between rows. Note that CellPadding.x is locked for the entire table.");
        if (RblxUI::BeginTable("table_changing_cellpadding_y", 1, RblxUITableFlags_Borders))
        {
            RblxUIStyle& style = RblxUI::GetStyle();
            for (int row = 0; row < 8; row++)
            {
                if ((row % 3) == 2)
                    RblxUI::PushStyleVarY(RblxUIStyleVar_CellPadding, 20.0f);
                RblxUI::TableNextRow(RblxUITableRowFlags_None);
                RblxUI::TableNextColumn();
                RblxUI::Text("CellPadding.y = %.2f", style.CellPadding.y);
                if ((row % 3) == 2)
                    RblxUI::PopStyleVar();
            }
            RblxUI::EndTable();
        }

        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Outer size"))
    {
        RBLXUI_DEMO_MARKER("Tables/Outer size");
        // Showcasing use of RblxUITableFlags_NoHostExtendX and RblxUITableFlags_NoHostExtendY
        // Important to that note how the two flags have slightly different behaviors!
        RblxUI::Text("Using NoHostExtendX and NoHostExtendY:");
        PushStyleCompact();
        static RblxUITableFlags flags = RblxUITableFlags_Borders | RblxUITableFlags_Resizable | RblxUITableFlags_ContextMenuInBody | RblxUITableFlags_RowBg | RblxUITableFlags_SizingFixedFit | RblxUITableFlags_NoHostExtendX;
        RblxUI::CheckboxFlags("RblxUITableFlags_NoHostExtendX", &flags, RblxUITableFlags_NoHostExtendX);
        RblxUI::SameLine(); HelpMarker("Make outer width auto-fit to columns, overriding outer_size.x value.\n\nOnly available when ScrollX/ScrollY are disabled and Stretch columns are not used.");
        RblxUI::CheckboxFlags("RblxUITableFlags_NoHostExtendY", &flags, RblxUITableFlags_NoHostExtendY);
        RblxUI::SameLine(); HelpMarker("Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit).\n\nOnly available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.");
        PopStyleCompact();

        RblxVec2 outer_size = RblxVec2(0.0f, TEXT_BASE_HEIGHT * 5.5f);
        if (RblxUI::BeginTable("table1", 3, flags, outer_size))
        {
            for (int row = 0; row < 10; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    RblxUI::TableNextColumn();
                    RblxUI::Text("Cell %d,%d", column, row);
                }
            }
            RblxUI::EndTable();
        }
        RblxUI::SameLine();
        RblxUI::Text("Hello!");

        RblxUI::Spacing();

        RblxUI::Text("Using explicit size:");
        if (RblxUI::BeginTable("table2", 3, RblxUITableFlags_Borders | RblxUITableFlags_RowBg, RblxVec2(TEXT_BASE_WIDTH * 30, 0.0f)))
        {
            for (int row = 0; row < 5; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    RblxUI::TableNextColumn();
                    RblxUI::Text("Cell %d,%d", column, row);
                }
            }
            RblxUI::EndTable();
        }
        RblxUI::SameLine();
        if (RblxUI::BeginTable("table3", 3, RblxUITableFlags_Borders | RblxUITableFlags_RowBg, RblxVec2(TEXT_BASE_WIDTH * 30, 0.0f)))
        {
            const float rows_height = TEXT_BASE_HEIGHT * 1.5f + RblxUI::GetStyle().CellPadding.y * 2.0f;
            for (int row = 0; row < 3; row++)
            {
                RblxUI::TableNextRow(0, rows_height);
                for (int column = 0; column < 3; column++)
                {
                    RblxUI::TableNextColumn();
                    RblxUI::Text("Cell %d,%d", column, row);
                }
            }
            RblxUI::EndTable();
        }

        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Background color"))
    {
        RBLXUI_DEMO_MARKER("Tables/Background color");
        static RblxUITableFlags flags = RblxUITableFlags_RowBg;
        static int row_bg_type = 1;
        static int row_bg_target = 1;
        static int cell_bg_type = 1;

        PushStyleCompact();
        RblxUI::CheckboxFlags("RblxUITableFlags_Borders", &flags, RblxUITableFlags_Borders);
        RblxUI::CheckboxFlags("RblxUITableFlags_RowBg", &flags, RblxUITableFlags_RowBg);
        RblxUI::SameLine(); HelpMarker("RblxUITableFlags_RowBg automatically sets RowBg0 to alternative colors pulled from the Style.");
        RblxUI::Combo("row bg type", (int*)&row_bg_type, "None\0Red\0Gradient\0");
        RblxUI::Combo("row bg target", (int*)&row_bg_target, "RowBg0\0RowBg1\0"); RblxUI::SameLine(); HelpMarker("Target RowBg0 to override the alternating odd/even colors,\nTarget RowBg1 to blend with them.");
        RblxUI::Combo("cell bg type", (int*)&cell_bg_type, "None\0Blue\0"); RblxUI::SameLine(); HelpMarker("We are colorizing cells to B1->C2 here.");
        RBLXUI_ASSERT(row_bg_type >= 0 && row_bg_type <= 2);
        RBLXUI_ASSERT(row_bg_target >= 0 && row_bg_target <= 1);
        RBLXUI_ASSERT(cell_bg_type >= 0 && cell_bg_type <= 1);
        PopStyleCompact();

        if (RblxUI::BeginTable("table1", 5, flags))
        {
            for (int row = 0; row < 6; row++)
            {
                RblxUI::TableNextRow();

                // Demonstrate setting a row background color with 'RblxUI::TableSetBgColor(RblxUITableBgTarget_RowBgX, ...)'
                // We use a transparent color so we can see the one behind in case our target is RowBg1 and RowBg0 was already targeted by the RblxUITableFlags_RowBg flag.
                if (row_bg_type != 0)
                {
                    RblxU32 row_bg_color = RblxUI::GetColorU32(row_bg_type == 1 ? RblxVec4(0.7f, 0.3f, 0.3f, 0.65f) : RblxVec4(0.2f + row * 0.1f, 0.2f, 0.2f, 0.65f)); // Flat or Gradient?
                    RblxUI::TableSetBgColor(RblxUITableBgTarget_RowBg0 + row_bg_target, row_bg_color);
                }

                // Fill cells
                for (int column = 0; column < 5; column++)
                {
                    RblxUI::TableSetColumnIndex(column);
                    RblxUI::Text("%c%c", 'A' + row, '0' + column);

                    // Change background of Cells B1->C2
                    // Demonstrate setting a cell background color with 'RblxUI::TableSetBgColor(RblxUITableBgTarget_CellBg, ...)'
                    // (the CellBg color will be blended over the RowBg and ColumnBg colors)
                    // We can also pass a column number as a third parameter to TableSetBgColor() and do this outside the column loop.
                    if (row >= 1 && row <= 2 && column >= 1 && column <= 2 && cell_bg_type == 1)
                    {
                        RblxU32 cell_bg_color = RblxUI::GetColorU32(RblxVec4(0.3f, 0.3f, 0.7f, 0.65f));
                        RblxUI::TableSetBgColor(RblxUITableBgTarget_CellBg, cell_bg_color);
                    }
                }
            }
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Tree view"))
    {
        RBLXUI_DEMO_MARKER("Tables/Tree view");
        static RblxUITableFlags table_flags = RblxUITableFlags_BordersV | RblxUITableFlags_BordersOuterH | RblxUITableFlags_Resizable | RblxUITableFlags_RowBg | RblxUITableFlags_NoBordersInBody;

        static RblxUITreeNodeFlags tree_node_flags_base = RblxUITreeNodeFlags_SpanAllColumns | RblxUITreeNodeFlags_DefaultOpen | RblxUITreeNodeFlags_DrawLinesFull;
        RblxUI::CheckboxFlags("RblxUITreeNodeFlags_SpanFullWidth",  &tree_node_flags_base, RblxUITreeNodeFlags_SpanFullWidth);
        RblxUI::CheckboxFlags("RblxUITreeNodeFlags_SpanLabelWidth",  &tree_node_flags_base, RblxUITreeNodeFlags_SpanLabelWidth);
        RblxUI::CheckboxFlags("RblxUITreeNodeFlags_SpanAllColumns", &tree_node_flags_base, RblxUITreeNodeFlags_SpanAllColumns);
        RblxUI::CheckboxFlags("RblxUITreeNodeFlags_LabelSpanAllColumns", &tree_node_flags_base, RblxUITreeNodeFlags_LabelSpanAllColumns);
        RblxUI::SameLine(); HelpMarker("Useful if you know that you aren't displaying contents in other columns");

        HelpMarker("See \"Columns flags\" section to configure how indentation is applied to individual columns.");
        if (RblxUI::BeginTable("3ways", 3, table_flags))
        {
            // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
            RblxUI::TableSetupColumn("Name", RblxUITableColumnFlags_NoHide);
            RblxUI::TableSetupColumn("Size", RblxUITableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
            RblxUI::TableSetupColumn("Type", RblxUITableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
            RblxUI::TableHeadersRow();

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
                    RblxUI::TableNextRow();
                    RblxUI::TableNextColumn();
                    const bool is_folder = (node->ChildCount > 0);

                    RblxUITreeNodeFlags node_flags = tree_node_flags_base;
                    if (node != &all_nodes[0])
                        node_flags &= ~RblxUITreeNodeFlags_LabelSpanAllColumns; // Only demonstrate this on the root node.

                    if (is_folder)
                    {
                        bool open = RblxUI::TreeNodeEx(node->Name, node_flags);
                        if ((node_flags & RblxUITreeNodeFlags_LabelSpanAllColumns) == 0)
                        {
                            RblxUI::TableNextColumn();
                            RblxUI::TextDisabled("--");
                            RblxUI::TableNextColumn();
                            RblxUI::TextUnformatted(node->Type);
                        }
                        if (open)
                        {
                            for (int child_n = 0; child_n < node->ChildCount; child_n++)
                                DisplayNode(&all_nodes[node->ChildIdx + child_n], all_nodes);
                            RblxUI::TreePop();
                        }
                    }
                    else
                    {
                        RblxUI::TreeNodeEx(node->Name, node_flags | RblxUITreeNodeFlags_Leaf | RblxUITreeNodeFlags_Bullet | RblxUITreeNodeFlags_NoTreePushOnOpen);
                        RblxUI::TableNextColumn();
                        RblxUI::Text("%d", node->Size);
                        RblxUI::TableNextColumn();
                        RblxUI::TextUnformatted(node->Type);
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

            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Item width"))
    {
        RBLXUI_DEMO_MARKER("Tables/Item width");
        HelpMarker(
            "Showcase using PushItemWidth() and how it is preserved on a per-column basis.\n\n"
            "Note that on auto-resizing non-resizable fixed columns, querying the content width for "
            "e.g. right-alignment doesn't make sense.");
        if (RblxUI::BeginTable("table_item_width", 3, RblxUITableFlags_Borders))
        {
            RblxUI::TableSetupColumn("small");
            RblxUI::TableSetupColumn("half");
            RblxUI::TableSetupColumn("right-align");
            RblxUI::TableHeadersRow();

            for (int row = 0; row < 3; row++)
            {
                RblxUI::TableNextRow();
                if (row == 0)
                {
                    // Setup ItemWidth once (instead of setting up every time, which is also possible but less efficient)
                    RblxUI::TableSetColumnIndex(0);
                    RblxUI::PushItemWidth(TEXT_BASE_WIDTH * 3.0f); // Small
                    RblxUI::TableSetColumnIndex(1);
                    RblxUI::PushItemWidth(-RblxUI::GetContentRegionAvail().x * 0.5f);
                    RblxUI::TableSetColumnIndex(2);
                    RblxUI::PushItemWidth(-FLT_MIN); // Right-aligned
                }

                // Draw our contents
                static float dummy_f = 0.0f;
                RblxUI::PushID(row);
                RblxUI::TableSetColumnIndex(0);
                RblxUI::SliderFloat("float0", &dummy_f, 0.0f, 1.0f);
                RblxUI::TableSetColumnIndex(1);
                RblxUI::SliderFloat("float1", &dummy_f, 0.0f, 1.0f);
                RblxUI::TableSetColumnIndex(2);
                RblxUI::SliderFloat("##float2", &dummy_f, 0.0f, 1.0f); // No visible label since right-aligned
                RblxUI::PopID();
            }
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    // Demonstrate using TableHeader() calls instead of TableHeadersRow()
    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Custom headers"))
    {
        RBLXUI_DEMO_MARKER("Tables/Custom headers");
        const int COLUMNS_COUNT = 3;
        if (RblxUI::BeginTable("table_custom_headers", COLUMNS_COUNT, RblxUITableFlags_Borders | RblxUITableFlags_Reorderable | RblxUITableFlags_Hideable))
        {
            RblxUI::TableSetupColumn("Apricot");
            RblxUI::TableSetupColumn("Banana");
            RblxUI::TableSetupColumn("Cherry");

            // Dummy entire-column selection storage
            // FIXME: It would be nice to actually demonstrate full-featured selection using those checkbox.
            static bool column_selected[3] = {};

            // Instead of calling TableHeadersRow() we'll submit custom headers ourselves.
            // (A different approach is also possible:
            //    - Specify RblxUITableColumnFlags_NoHeaderLabel in some TableSetupColumn() call.
            //    - Call TableHeadersRow() normally. This will submit TableHeader() with no name.
            //    - Then call TableSetColumnIndex() to position yourself in the column and submit your stuff e.g. Checkbox().)
            RblxUI::TableNextRow(RblxUITableRowFlags_Headers);
            for (int column = 0; column < COLUMNS_COUNT; column++)
            {
                RblxUI::TableSetColumnIndex(column);
                const char* column_name = RblxUI::TableGetColumnName(column); // Retrieve name passed to TableSetupColumn()
                RblxUI::PushID(column);
                RblxUI::PushStyleVar(RblxUIStyleVar_FramePadding, RblxVec2(0, 0));
                RblxUI::Checkbox("##checkall", &column_selected[column]);
                RblxUI::PopStyleVar();
                RblxUI::SameLine(0.0f, RblxUI::GetStyle().ItemInnerSpacing.x);
                RblxUI::TableHeader(column_name);
                RblxUI::PopID();
            }

            // Submit table contents
            for (int row = 0; row < 5; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    char buf[32];
                    sprintf(buf, "Cell %d,%d", column, row);
                    RblxUI::TableSetColumnIndex(column);
                    RblxUI::Selectable(buf, column_selected[column]);
                }
            }
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    // Demonstrate using RblxUITableColumnFlags_AngledHeader flag to create angled headers
    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Angled headers"))
    {
        RBLXUI_DEMO_MARKER("Tables/Angled headers");
        const char* column_names[] = { "Track", "cabasa", "ride", "smash", "tom-hi", "tom-mid", "tom-low", "hihat-o", "hihat-c", "snare-s", "snare-c", "clap", "rim", "kick" };
        const int columns_count = RBLXUI_COUNTOF(column_names);
        const int rows_count = 12;

        static RblxUITableFlags table_flags = RblxUITableFlags_SizingFixedFit | RblxUITableFlags_ScrollX | RblxUITableFlags_ScrollY | RblxUITableFlags_BordersOuter | RblxUITableFlags_BordersInnerH | RblxUITableFlags_Hideable | RblxUITableFlags_Resizable | RblxUITableFlags_Reorderable | RblxUITableFlags_HighlightHoveredColumn;
        static RblxUITableColumnFlags column_flags = RblxUITableColumnFlags_AngledHeader | RblxUITableColumnFlags_WidthFixed;
        static bool bools[columns_count * rows_count] = {}; // Dummy storage selection storage
        static int frozen_cols = 1;
        static int frozen_rows = 2;
        RblxUI::CheckboxFlags("_ScrollX", &table_flags, RblxUITableFlags_ScrollX);
        RblxUI::CheckboxFlags("_ScrollY", &table_flags, RblxUITableFlags_ScrollY);
        RblxUI::CheckboxFlags("_Resizable", &table_flags, RblxUITableFlags_Resizable);
        RblxUI::CheckboxFlags("_Sortable", &table_flags, RblxUITableFlags_Sortable);
        RblxUI::CheckboxFlags("_NoBordersInBody", &table_flags, RblxUITableFlags_NoBordersInBody);
        RblxUI::CheckboxFlags("_HighlightHoveredColumn", &table_flags, RblxUITableFlags_HighlightHoveredColumn);
        RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 8);
        RblxUI::SliderInt("Frozen columns", &frozen_cols, 0, 2);
        RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 8);
        RblxUI::SliderInt("Frozen rows", &frozen_rows, 0, 2);
        RblxUI::CheckboxFlags("Disable header contributing to column width", &column_flags, RblxUITableColumnFlags_NoHeaderWidth);

        if (RblxUI::TreeNode("Style settings"))
        {
            RblxUI::SameLine();
            HelpMarker("Giving access to some RblxUIStyle value in this demo for convenience.");
            RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 8);
            RblxUI::SliderAngle("style.TableAngledHeadersAngle", &RblxUI::GetStyle().TableAngledHeadersAngle, -50.0f, +50.0f);
            RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 8);
            RblxUI::SliderFloat2("style.TableAngledHeadersTextAlign", (float*)&RblxUI::GetStyle().TableAngledHeadersTextAlign, 0.0f, 1.0f, "%.2f");
            RblxUI::TreePop();
        }

        if (RblxUI::BeginTable("table_angled_headers", columns_count, table_flags, RblxVec2(0.0f, TEXT_BASE_HEIGHT * 12)))
        {
            RblxUI::TableSetupColumn(column_names[0], RblxUITableColumnFlags_NoHide | RblxUITableColumnFlags_NoReorder);
            for (int n = 1; n < columns_count; n++)
                RblxUI::TableSetupColumn(column_names[n], column_flags);
            RblxUI::TableSetupScrollFreeze(frozen_cols, frozen_rows);

            RblxUI::TableAngledHeadersRow(); // Draw angled headers for all columns with the RblxUITableColumnFlags_AngledHeader flag.
            RblxUI::TableHeadersRow();       // Draw remaining headers and allow access to context-menu and other functions.
            for (int row = 0; row < rows_count; row++)
            {
                RblxUI::PushID(row);
                RblxUI::TableNextRow();
                RblxUI::TableSetColumnIndex(0);
                RblxUI::AlignTextToFramePadding();
                RblxUI::Text("Track %d", row);
                for (int column = 1; column < columns_count; column++)
                    if (RblxUI::TableSetColumnIndex(column))
                    {
                        RblxUI::PushID(column);
                        RblxUI::Checkbox("", &bools[row * columns_count + column]);
                        RblxUI::PopID();
                    }
                RblxUI::PopID();
            }
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    // Demonstrate creating custom context menus inside columns,
    // while playing it nice with context menus provided by TableHeadersRow()/TableHeader()
    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Context menus"))
    {
        RBLXUI_DEMO_MARKER("Tables/Context menus");
        HelpMarker(
            "By default, right-clicking over a TableHeadersRow()/TableHeader() line will open the default context-menu.\n"
            "Using RblxUITableFlags_ContextMenuInBody we also allow right-clicking over columns body.");
        static RblxUITableFlags flags1 = RblxUITableFlags_Resizable | RblxUITableFlags_Reorderable | RblxUITableFlags_Hideable | RblxUITableFlags_Borders | RblxUITableFlags_ContextMenuInBody;

        PushStyleCompact();
        RblxUI::CheckboxFlags("RblxUITableFlags_ContextMenuInBody", &flags1, RblxUITableFlags_ContextMenuInBody);
        PopStyleCompact();

        // Context Menus: first example
        // [1.1] Right-click on the TableHeadersRow() line to open the default table context menu.
        // [1.2] Right-click in columns also open the default table context menu (if RblxUITableFlags_ContextMenuInBody is set)
        const int COLUMNS_COUNT = 3;
        if (RblxUI::BeginTable("table_context_menu", COLUMNS_COUNT, flags1))
        {
            RblxUI::TableSetupColumn("One");
            RblxUI::TableSetupColumn("Two");
            RblxUI::TableSetupColumn("Three");

            // [1.1]] Right-click on the TableHeadersRow() line to open the default table context menu.
            RblxUI::TableHeadersRow();

            // Submit dummy contents
            for (int row = 0; row < 4; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < COLUMNS_COUNT; column++)
                {
                    RblxUI::TableSetColumnIndex(column);
                    RblxUI::Text("Cell %d,%d", column, row);
                }
            }
            RblxUI::EndTable();
        }

        // Context Menus: second example
        // [2.1] Right-click on the TableHeadersRow() line to open the default table context menu.
        // [2.2] Right-click on the ".." to open a custom popup
        // [2.3] Right-click in columns to open another custom popup
        HelpMarker(
            "Demonstrate mixing table context menu (over header), item context button (over button) "
            "and custom per-column context menu (over column body).");
        RblxUITableFlags flags2 = RblxUITableFlags_Resizable | RblxUITableFlags_SizingFixedFit | RblxUITableFlags_Reorderable | RblxUITableFlags_Hideable | RblxUITableFlags_Borders;
        if (RblxUI::BeginTable("table_context_menu_2", COLUMNS_COUNT, flags2))
        {
            RblxUI::TableSetupColumn("One");
            RblxUI::TableSetupColumn("Two");
            RblxUI::TableSetupColumn("Three");

            // [2.1] Right-click on the TableHeadersRow() line to open the default table context menu.
            RblxUI::TableHeadersRow();
            for (int row = 0; row < 4; row++)
            {
                RblxUI::TableNextRow();
                for (int column = 0; column < COLUMNS_COUNT; column++)
                {
                    // Submit dummy contents
                    RblxUI::TableSetColumnIndex(column);
                    RblxUI::Text("Cell %d,%d", column, row);
                    RblxUI::SameLine();

                    // [2.2] Right-click on the ".." to open a custom popup
                    RblxUI::PushID(row * COLUMNS_COUNT + column);
                    RblxUI::SmallButton("..");
                    if (RblxUI::BeginPopupContextItem())
                    {
                        RblxUI::Text("This is the popup for Button(\"..\") in Cell %d,%d", column, row);
                        if (RblxUI::Button("Close"))
                            RblxUI::CloseCurrentPopup();
                        RblxUI::EndPopup();
                    }
                    RblxUI::PopID();
                }
            }

            // [2.3] Right-click anywhere in columns to open another custom popup
            // (instead of testing for !IsAnyItemHovered() we could also call OpenPopup() with RblxUIPopupFlags_NoOpenOverExistingPopup
            // to manage popup priority as the popups triggers, here "are we hovering a column" are overlapping)
            int hovered_column = -1;
            for (int column = 0; column < COLUMNS_COUNT + 1; column++)
            {
                RblxUI::PushID(column);
                if (RblxUI::TableGetColumnFlags(column) & RblxUITableColumnFlags_IsHovered)
                    hovered_column = column;
                if (hovered_column == column && !RblxUI::IsAnyItemHovered() && RblxUI::IsMouseReleased(1))
                    RblxUI::OpenPopup("MyPopup");
                if (RblxUI::BeginPopup("MyPopup"))
                {
                    if (column == COLUMNS_COUNT)
                        RblxUI::Text("This is a custom popup for unused space after the last column.");
                    else
                        RblxUI::Text("This is a custom popup for Column %d", column);
                    if (RblxUI::Button("Close"))
                        RblxUI::CloseCurrentPopup();
                    RblxUI::EndPopup();
                }
                RblxUI::PopID();
            }

            RblxUI::EndTable();
            RblxUI::Text("Hovered column: %d", hovered_column);
        }
        RblxUI::TreePop();
    }

    // Demonstrate creating multiple tables with the same ID
    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Synced instances"))
    {
        RBLXUI_DEMO_MARKER("Tables/Synced instances");
        HelpMarker("Multiple tables with the same identifier will share their settings, width, visibility, order etc.");

        static RblxUITableFlags flags = RblxUITableFlags_Resizable | RblxUITableFlags_Reorderable | RblxUITableFlags_Hideable | RblxUITableFlags_Borders | RblxUITableFlags_SizingFixedFit | RblxUITableFlags_NoSavedSettings;
        RblxUI::CheckboxFlags("RblxUITableFlags_Resizable", &flags, RblxUITableFlags_Resizable);
        RblxUI::CheckboxFlags("RblxUITableFlags_ScrollY", &flags, RblxUITableFlags_ScrollY);
        RblxUI::CheckboxFlags("RblxUITableFlags_SizingFixedFit", &flags, RblxUITableFlags_SizingFixedFit);
        RblxUI::CheckboxFlags("RblxUITableFlags_HighlightHoveredColumn", &flags, RblxUITableFlags_HighlightHoveredColumn);
        for (int n = 0; n < 3; n++)
        {
            char buf[32];
            sprintf(buf, "Synced Table %d", n);
            bool open = RblxUI::CollapsingHeader(buf, RblxUITreeNodeFlags_DefaultOpen);
            if (open && RblxUI::BeginTable("Table", 3, flags, RblxVec2(0.0f, RblxUI::GetTextLineHeightWithSpacing() * 5)))
            {
                RblxUI::TableSetupColumn("One");
                RblxUI::TableSetupColumn("Two");
                RblxUI::TableSetupColumn("Three");
                RblxUI::TableHeadersRow();
                const int cell_count = (n == 1) ? 27 : 9; // Make second table have a scrollbar to verify that additional decoration is not affecting column positions.
                for (int cell = 0; cell < cell_count; cell++)
                {
                    RblxUI::TableNextColumn();
                    RblxUI::Text("this cell %d", cell);
                }
                RblxUI::EndTable();
            }
        }
        RblxUI::TreePop();
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
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Sorting"))
    {
        RBLXUI_DEMO_MARKER("Tables/Sorting");
        // Create item list
        static RblxVector<MyItem> items;
        if (items.Size == 0)
        {
            items.resize(50, MyItem());
            for (int n = 0; n < items.Size; n++)
            {
                const int template_n = n % RBLXUI_COUNTOF(template_items_names);
                MyItem& item = items[n];
                item.ID = n;
                item.Name = template_items_names[template_n];
                item.Quantity = (n * n - n) % 20; // Assign default quantities
            }
        }

        // Options
        static RblxUITableFlags flags =
            RblxUITableFlags_Resizable | RblxUITableFlags_Reorderable | RblxUITableFlags_Hideable | RblxUITableFlags_Sortable | RblxUITableFlags_SortMulti
            | RblxUITableFlags_RowBg | RblxUITableFlags_BordersOuter | RblxUITableFlags_BordersV | RblxUITableFlags_NoBordersInBody
            | RblxUITableFlags_ScrollY;
        PushStyleCompact();
        RblxUI::CheckboxFlags("RblxUITableFlags_SortMulti", &flags, RblxUITableFlags_SortMulti);
        RblxUI::SameLine(); HelpMarker("When sorting is enabled: hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).");
        RblxUI::CheckboxFlags("RblxUITableFlags_SortTristate", &flags, RblxUITableFlags_SortTristate);
        RblxUI::SameLine(); HelpMarker("When sorting is enabled: allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).");
        PopStyleCompact();

        if (RblxUI::BeginTable("table_sorting", 4, flags, RblxVec2(0.0f, TEXT_BASE_HEIGHT * 15), 0.0f))
        {
            // Declare columns
            // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
            // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
            // Demonstrate using a mixture of flags among available sort-related flags:
            // - RblxUITableColumnFlags_DefaultSort
            // - RblxUITableColumnFlags_NoSort / RblxUITableColumnFlags_NoSortAscending / RblxUITableColumnFlags_NoSortDescending
            // - RblxUITableColumnFlags_PreferSortAscending / RblxUITableColumnFlags_PreferSortDescending
            RblxUI::TableSetupColumn("ID",       RblxUITableColumnFlags_DefaultSort          | RblxUITableColumnFlags_WidthFixed,   0.0f, MyItemColumnID_ID);
            RblxUI::TableSetupColumn("Name",                                                  RblxUITableColumnFlags_WidthFixed,   0.0f, MyItemColumnID_Name);
            RblxUI::TableSetupColumn("Action",   RblxUITableColumnFlags_NoSort               | RblxUITableColumnFlags_WidthFixed,   0.0f, MyItemColumnID_Action);
            RblxUI::TableSetupColumn("Quantity", RblxUITableColumnFlags_PreferSortDescending | RblxUITableColumnFlags_WidthStretch, 0.0f, MyItemColumnID_Quantity);
            RblxUI::TableSetupScrollFreeze(0, 1); // Make row always visible
            RblxUI::TableHeadersRow();

            // Sort our data if sort specs have been changed!
            if (RblxUITableSortSpecs* sort_specs = RblxUI::TableGetSortSpecs())
                if (sort_specs->SpecsDirty)
                {
                    MyItem::SortWithSortSpecs(sort_specs, items.Data, items.Size);
                    sort_specs->SpecsDirty = false;
                }

            // Demonstrate using clipper for large vertical lists
            RblxUIListClipper clipper;
            clipper.Begin(items.Size);
            while (clipper.Step())
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    // Display a data item
                    MyItem* item = &items[row_n];
                    RblxUI::PushID(item->ID);
                    RblxUI::TableNextRow();
                    RblxUI::TableNextColumn();
                    RblxUI::Text("%04d", item->ID);
                    RblxUI::TableNextColumn();
                    RblxUI::TextUnformatted(item->Name);
                    RblxUI::TableNextColumn();
                    RblxUI::SmallButton("None");
                    RblxUI::TableNextColumn();
                    RblxUI::Text("%d", item->Quantity);
                    RblxUI::PopID();
                }
            RblxUI::EndTable();
        }
        RblxUI::TreePop();
    }

    // In this example we'll expose most table flags and settings.
    // For specific flags and settings refer to the corresponding section for more detailed explanation.
    // This section is mostly useful to experiment with combining certain flags or settings with each others.
    //RblxUI::SetNextItemOpen(true, RblxUICond_Once); // [DEBUG]
    if (open_action != -1)
        RblxUI::SetNextItemOpen(open_action != 0);
    if (RblxUI::TreeNode("Advanced"))
    {
        RBLXUI_DEMO_MARKER("Tables/Advanced");
        static RblxUITableFlags flags =
            RblxUITableFlags_Resizable | RblxUITableFlags_Reorderable | RblxUITableFlags_Hideable
            | RblxUITableFlags_Sortable | RblxUITableFlags_SortMulti
            | RblxUITableFlags_RowBg | RblxUITableFlags_Borders | RblxUITableFlags_NoBordersInBody
            | RblxUITableFlags_ScrollX | RblxUITableFlags_ScrollY
            | RblxUITableFlags_SizingFixedFit;
        static RblxUITableColumnFlags columns_base_flags = RblxUITableColumnFlags_None;

        enum ContentsType { CT_Text, CT_Button, CT_SmallButton, CT_FillButton, CT_Selectable, CT_SelectableSpanRow };
        static int contents_type = CT_SelectableSpanRow;
        const char* contents_type_names[] = { "Text", "Button", "SmallButton", "FillButton", "Selectable", "Selectable (span row)" };
        static int freeze_cols = 1;
        static int freeze_rows = 1;
        static int items_count = RBLXUI_COUNTOF(template_items_names) * 2;
        static RblxVec2 outer_size_value = RblxVec2(0.0f, TEXT_BASE_HEIGHT * 12);
        static float row_min_height = 0.0f; // Auto
        static float inner_width_with_scroll = 0.0f; // Auto-extend
        static bool outer_size_enabled = true;
        static bool show_headers = true;
        static bool show_wrapped_text = false;
        //static RblxUITextFilter filter;
        //RblxUI::SetNextItemOpen(true, RblxUICond_Once); // FIXME-TABLE: Enabling this results in initial clipped first pass on table which tend to affect column sizing
        if (RblxUI::TreeNode("Options"))
        {
            // Make the UI compact because there are so many fields
            PushStyleCompact();
            RblxUI::PushItemWidth(TEXT_BASE_WIDTH * 28.0f);

            if (RblxUI::TreeNodeEx("Features:", RblxUITreeNodeFlags_DefaultOpen))
            {
                RblxUI::CheckboxFlags("RblxUITableFlags_Resizable", &flags, RblxUITableFlags_Resizable);
                RblxUI::CheckboxFlags("RblxUITableFlags_Reorderable", &flags, RblxUITableFlags_Reorderable);
                RblxUI::CheckboxFlags("RblxUITableFlags_Hideable", &flags, RblxUITableFlags_Hideable);
                RblxUI::CheckboxFlags("RblxUITableFlags_Sortable", &flags, RblxUITableFlags_Sortable);
                RblxUI::CheckboxFlags("RblxUITableFlags_NoSavedSettings", &flags, RblxUITableFlags_NoSavedSettings);
                RblxUI::CheckboxFlags("RblxUITableFlags_ContextMenuInBody", &flags, RblxUITableFlags_ContextMenuInBody);
                RblxUI::TreePop();
            }

            if (RblxUI::TreeNodeEx("Decorations:", RblxUITreeNodeFlags_DefaultOpen))
            {
                RblxUI::CheckboxFlags("RblxUITableFlags_RowBg", &flags, RblxUITableFlags_RowBg);
                RblxUI::CheckboxFlags("RblxUITableFlags_BordersV", &flags, RblxUITableFlags_BordersV);
                RblxUI::CheckboxFlags("RblxUITableFlags_BordersOuterV", &flags, RblxUITableFlags_BordersOuterV);
                RblxUI::CheckboxFlags("RblxUITableFlags_BordersInnerV", &flags, RblxUITableFlags_BordersInnerV);
                RblxUI::CheckboxFlags("RblxUITableFlags_BordersH", &flags, RblxUITableFlags_BordersH);
                RblxUI::CheckboxFlags("RblxUITableFlags_BordersOuterH", &flags, RblxUITableFlags_BordersOuterH);
                RblxUI::CheckboxFlags("RblxUITableFlags_BordersInnerH", &flags, RblxUITableFlags_BordersInnerH);
                RblxUI::CheckboxFlags("RblxUITableFlags_NoBordersInBody", &flags, RblxUITableFlags_NoBordersInBody); RblxUI::SameLine(); HelpMarker("Disable vertical borders in columns Body (borders will always appear in Headers)");
                RblxUI::CheckboxFlags("RblxUITableFlags_NoBordersInBodyUntilResize", &flags, RblxUITableFlags_NoBordersInBodyUntilResize); RblxUI::SameLine(); HelpMarker("Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers)");
                RblxUI::TreePop();
            }

            if (RblxUI::TreeNodeEx("Sizing:", RblxUITreeNodeFlags_DefaultOpen))
            {
                EditTableSizingFlags(&flags);
                RblxUI::SameLine(); HelpMarker("In the Advanced demo we override the policy of each column so those table-wide settings have less effect that typical.");
                RblxUI::CheckboxFlags("RblxUITableFlags_NoHostExtendX", &flags, RblxUITableFlags_NoHostExtendX);
                RblxUI::SameLine(); HelpMarker("Make outer width auto-fit to columns, overriding outer_size.x value.\n\nOnly available when ScrollX/ScrollY are disabled and Stretch columns are not used.");
                RblxUI::CheckboxFlags("RblxUITableFlags_NoHostExtendY", &flags, RblxUITableFlags_NoHostExtendY);
                RblxUI::SameLine(); HelpMarker("Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit).\n\nOnly available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.");
                RblxUI::CheckboxFlags("RblxUITableFlags_NoKeepColumnsVisible", &flags, RblxUITableFlags_NoKeepColumnsVisible);
                RblxUI::SameLine(); HelpMarker("Only available if ScrollX is disabled.");
                RblxUI::CheckboxFlags("RblxUITableFlags_PreciseWidths", &flags, RblxUITableFlags_PreciseWidths);
                RblxUI::SameLine(); HelpMarker("Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.");
                RblxUI::CheckboxFlags("RblxUITableFlags_NoClip", &flags, RblxUITableFlags_NoClip);
                RblxUI::SameLine(); HelpMarker("Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with ScrollFreeze options.");
                RblxUI::TreePop();
            }

            if (RblxUI::TreeNodeEx("Padding:", RblxUITreeNodeFlags_DefaultOpen))
            {
                RblxUI::CheckboxFlags("RblxUITableFlags_PadOuterX", &flags, RblxUITableFlags_PadOuterX);
                RblxUI::CheckboxFlags("RblxUITableFlags_NoPadOuterX", &flags, RblxUITableFlags_NoPadOuterX);
                RblxUI::CheckboxFlags("RblxUITableFlags_NoPadInnerX", &flags, RblxUITableFlags_NoPadInnerX);
                RblxUI::TreePop();
            }

            if (RblxUI::TreeNodeEx("Scrolling:", RblxUITreeNodeFlags_DefaultOpen))
            {
                RblxUI::CheckboxFlags("RblxUITableFlags_ScrollX", &flags, RblxUITableFlags_ScrollX);
                RblxUI::SameLine();
                RblxUI::SetNextItemWidth(RblxUI::GetFrameHeight());
                RblxUI::DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL, RblxUISliderFlags_NoInput);
                RblxUI::CheckboxFlags("RblxUITableFlags_ScrollY", &flags, RblxUITableFlags_ScrollY);
                RblxUI::SameLine();
                RblxUI::SetNextItemWidth(RblxUI::GetFrameHeight());
                RblxUI::DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL, RblxUISliderFlags_NoInput);
                RblxUI::TreePop();
            }

            if (RblxUI::TreeNodeEx("Sorting:", RblxUITreeNodeFlags_DefaultOpen))
            {
                RblxUI::CheckboxFlags("RblxUITableFlags_SortMulti", &flags, RblxUITableFlags_SortMulti);
                RblxUI::SameLine(); HelpMarker("When sorting is enabled: hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).");
                RblxUI::CheckboxFlags("RblxUITableFlags_SortTristate", &flags, RblxUITableFlags_SortTristate);
                RblxUI::SameLine(); HelpMarker("When sorting is enabled: allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).");
                RblxUI::TreePop();
            }

            if (RblxUI::TreeNodeEx("Headers:", RblxUITreeNodeFlags_DefaultOpen))
            {
                RblxUI::Checkbox("show_headers", &show_headers);
                RblxUI::CheckboxFlags("RblxUITableFlags_HighlightHoveredColumn", &flags, RblxUITableFlags_HighlightHoveredColumn);
                RblxUI::CheckboxFlags("RblxUITableColumnFlags_AngledHeader", &columns_base_flags, RblxUITableColumnFlags_AngledHeader);
                RblxUI::SameLine(); HelpMarker("Enable AngledHeader on all columns. Best enabled on selected narrow columns (see \"Angled headers\" section of the demo).");
                RblxUI::TreePop();
            }

            if (RblxUI::TreeNodeEx("Other:", RblxUITreeNodeFlags_DefaultOpen))
            {
                RblxUI::Checkbox("show_wrapped_text", &show_wrapped_text);

                RblxUI::DragFloat2("##OuterSize", &outer_size_value.x);
                RblxUI::SameLine(0.0f, RblxUI::GetStyle().ItemInnerSpacing.x);
                RblxUI::Checkbox("outer_size", &outer_size_enabled);
                RblxUI::SameLine();
                HelpMarker("If scrolling is disabled (ScrollX and ScrollY not set):\n"
                    "- The table is output directly in the parent window.\n"
                    "- OuterSize.x < 0.0f will right-align the table.\n"
                    "- OuterSize.x = 0.0f will narrow fit the table unless there are any Stretch columns.\n"
                    "- OuterSize.y then becomes the minimum size for the table, which will extend vertically if there are more rows (unless NoHostExtendY is set).");

                // From a user point of view we will tend to use 'inner_width' differently depending on whether our table is embedding scrolling.
                // To facilitate toying with this demo we will actually pass 0.0f to the BeginTable() when ScrollX is disabled.
                RblxUI::DragFloat("inner_width (when ScrollX active)", &inner_width_with_scroll, 1.0f, 0.0f, FLT_MAX);

                RblxUI::DragFloat("row_min_height", &row_min_height, 1.0f, 0.0f, FLT_MAX);
                RblxUI::SameLine(); HelpMarker("Specify height of the Selectable item.");

                RblxUI::DragInt("items_count", &items_count, 0.1f, 0, 9999);
                RblxUI::Combo("items_type (first column)", &contents_type, contents_type_names, RBLXUI_COUNTOF(contents_type_names));
                //filter.Draw("filter");
                RblxUI::TreePop();
            }

            RblxUI::PopItemWidth();
            PopStyleCompact();
            RblxUI::Spacing();
            RblxUI::TreePop();
        }

        // Update item list if we changed the number of items
        static RblxVector<MyItem> items;
        static RblxVector<int> selection;
        static bool items_need_sort = false;
        if (items.Size != items_count)
        {
            items.resize(items_count, MyItem());
            for (int n = 0; n < items_count; n++)
            {
                const int template_n = n % RBLXUI_COUNTOF(template_items_names);
                MyItem& item = items[n];
                item.ID = n;
                item.Name = template_items_names[template_n];
                item.Quantity = (template_n == 3) ? 10 : (template_n == 4) ? 20 : 0; // Assign default quantities
            }
        }

        const RblxDrawList* parent_draw_list = RblxUI::GetWindowDrawList();
        const int parent_draw_list_draw_cmd_count = parent_draw_list->CmdBuffer.Size;
        RblxVec2 table_scroll_cur, table_scroll_max; // For debug display
        const RblxDrawList* table_draw_list = NULL;  // "

        // Submit table
        const float inner_width_to_use = (flags & RblxUITableFlags_ScrollX) ? inner_width_with_scroll : 0.0f;
        if (RblxUI::BeginTable("table_advanced", 6, flags, outer_size_enabled ? outer_size_value : RblxVec2(0, 0), inner_width_to_use))
        {
            // Declare columns
            // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
            // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
            RblxUI::TableSetupColumn("ID",           columns_base_flags | RblxUITableColumnFlags_DefaultSort | RblxUITableColumnFlags_WidthFixed | RblxUITableColumnFlags_NoHide, 0.0f, MyItemColumnID_ID);
            RblxUI::TableSetupColumn("Name",         columns_base_flags | RblxUITableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Name);
            RblxUI::TableSetupColumn("Action",       columns_base_flags | RblxUITableColumnFlags_NoSort | RblxUITableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Action);
            RblxUI::TableSetupColumn("Quantity",     columns_base_flags | RblxUITableColumnFlags_PreferSortDescending, 0.0f, MyItemColumnID_Quantity);
            RblxUI::TableSetupColumn("Description",  columns_base_flags | ((flags & RblxUITableFlags_NoHostExtendX) ? 0 : RblxUITableColumnFlags_WidthStretch), 0.0f, MyItemColumnID_Description);
            RblxUI::TableSetupColumn("Hidden",       columns_base_flags |  RblxUITableColumnFlags_DefaultHide | RblxUITableColumnFlags_NoSort);
            RblxUI::TableSetupScrollFreeze(freeze_cols, freeze_rows);

            // Sort our data if sort specs have been changed!
            RblxUITableSortSpecs* sort_specs = RblxUI::TableGetSortSpecs();
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
            const bool sorts_specs_using_quantity = (RblxUI::TableGetColumnFlags(3) & RblxUITableColumnFlags_IsSorted) != 0;

            // Show headers
            if (show_headers && (columns_base_flags & RblxUITableColumnFlags_AngledHeader) != 0)
                RblxUI::TableAngledHeadersRow();
            if (show_headers)
                RblxUI::TableHeadersRow();

            // Show data
            // FIXME-TABLE FIXME-NAV: How we can get decent up/down even though we have the buttons here?
#if 1
            // Demonstrate using clipper for large vertical lists
            RblxUIListClipper clipper;
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
                    RblxUI::PushID(item->ID);
                    RblxUI::TableNextRow(RblxUITableRowFlags_None, row_min_height);

                    // For the demo purpose we can select among different type of items submitted in the first column
                    RblxUI::TableSetColumnIndex(0);
                    char label[32];
                    sprintf(label, "%04d", item->ID);
                    if (contents_type == CT_Text)
                        RblxUI::TextUnformatted(label);
                    else if (contents_type == CT_Button)
                        RblxUI::Button(label);
                    else if (contents_type == CT_SmallButton)
                        RblxUI::SmallButton(label);
                    else if (contents_type == CT_FillButton)
                        RblxUI::Button(label, RblxVec2(-FLT_MIN, 0.0f));
                    else if (contents_type == CT_Selectable || contents_type == CT_SelectableSpanRow)
                    {
                        RblxUISelectableFlags selectable_flags = (contents_type == CT_SelectableSpanRow) ? RblxUISelectableFlags_SpanAllColumns | RblxUISelectableFlags_AllowOverlap : RblxUISelectableFlags_None;
                        if (RblxUI::Selectable(label, item_is_selected, selectable_flags, RblxVec2(0, row_min_height)))
                        {
                            if (RblxUI::GetIO().KeyCtrl)
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

                    if (RblxUI::TableSetColumnIndex(1))
                        RblxUI::TextUnformatted(item->Name);

                    // Here we demonstrate marking our data set as needing to be sorted again if we modified a quantity,
                    // and we are currently sorting on the column showing the Quantity.
                    // To avoid triggering a sort while holding the button, we only trigger it when the button has been released.
                    // You will probably need some extra logic if you want to automatically sort when a specific entry changes.
                    if (RblxUI::TableSetColumnIndex(2))
                    {
                        if (RblxUI::SmallButton("Chop")) { item->Quantity += 1; }
                        if (sorts_specs_using_quantity && RblxUI::IsItemDeactivated()) { items_need_sort = true; }
                        RblxUI::SameLine();
                        if (RblxUI::SmallButton("Eat")) { item->Quantity -= 1; }
                        if (sorts_specs_using_quantity && RblxUI::IsItemDeactivated()) { items_need_sort = true; }
                    }

                    if (RblxUI::TableSetColumnIndex(3))
                        RblxUI::Text("%d", item->Quantity);

                    RblxUI::TableSetColumnIndex(4);
                    if (show_wrapped_text)
                        RblxUI::TextWrapped("Lorem ipsum dolor sit amet");
                    else
                        RblxUI::Text("Lorem ipsum dolor sit amet");

                    if (RblxUI::TableSetColumnIndex(5))
                        RblxUI::Text("1234");

                    RblxUI::PopID();
                }
            }

            // Store some info to display debug details below
            table_scroll_cur = RblxVec2(RblxUI::GetScrollX(), RblxUI::GetScrollY());
            table_scroll_max = RblxVec2(RblxUI::GetScrollMaxX(), RblxUI::GetScrollMaxY());
            table_draw_list = RblxUI::GetWindowDrawList();
            RblxUI::EndTable();
        }
        static bool show_debug_details = false;
        RblxUI::Checkbox("Debug details", &show_debug_details);
        if (show_debug_details && table_draw_list)
        {
            RblxUI::SameLine(0.0f, 0.0f);
            const int table_draw_list_draw_cmd_count = table_draw_list->CmdBuffer.Size;
            if (table_draw_list == parent_draw_list)
                RblxUI::Text(": DrawCmd: +%d (in same window)",
                    table_draw_list_draw_cmd_count - parent_draw_list_draw_cmd_count);
            else
                RblxUI::Text(": DrawCmd: +%d (in child window), Scroll: (%.f/%.f) (%.f/%.f)",
                    table_draw_list_draw_cmd_count - 1, table_scroll_cur.x, table_scroll_max.x, table_scroll_cur.y, table_scroll_max.y);
        }
        RblxUI::TreePop();
    }

    RblxUI::PopID();

    DemoWindowColumns();

    if (disable_indent)
        RblxUI::PopStyleVar();
}

// Demonstrate old/legacy Columns API!
// [2020: Columns are under-featured and not maintained. Prefer using the more flexible and powerful BeginTable() API!]
static void DemoWindowColumns()
{
    bool open = RblxUI::TreeNode("Legacy Columns API");
    RblxUI::SameLine();
    HelpMarker("Columns() is an old API! Prefer using the more flexible and powerful BeginTable() API!");
    if (!open)
        return;

    // Basic columns
    if (RblxUI::TreeNode("Basic"))
    {
        RBLXUI_DEMO_MARKER("Columns (legacy API)/Basic");
        RblxUI::Text("Without border:");
        RblxUI::Columns(3, "mycolumns3", false);  // 3-ways, no border
        RblxUI::Separator();
        for (int n = 0; n < 14; n++)
        {
            char label[32];
            sprintf(label, "Item %d", n);
            if (RblxUI::Selectable(label)) {}
            //if (RblxUI::Button(label, RblxVec2(-FLT_MIN,0.0f))) {}
            RblxUI::NextColumn();
        }
        RblxUI::Columns(1);
        RblxUI::Separator();

        RblxUI::Text("With border:");
        RblxUI::Columns(4, "mycolumns"); // 4-ways, with border
        RblxUI::Separator();
        RblxUI::Text("ID"); RblxUI::NextColumn();
        RblxUI::Text("Name"); RblxUI::NextColumn();
        RblxUI::Text("Path"); RblxUI::NextColumn();
        RblxUI::Text("Hovered"); RblxUI::NextColumn();
        RblxUI::Separator();
        const char* names[3] = { "One", "Two", "Three" };
        const char* paths[3] = { "/path/one", "/path/two", "/path/three" };
        static int selected = -1;
        for (int i = 0; i < 3; i++)
        {
            char label[32];
            sprintf(label, "%04d", i);
            if (RblxUI::Selectable(label, selected == i, RblxUISelectableFlags_SpanAllColumns))
                selected = i;
            bool hovered = RblxUI::IsItemHovered();
            RblxUI::NextColumn();
            RblxUI::Text(names[i]); RblxUI::NextColumn();
            RblxUI::Text(paths[i]); RblxUI::NextColumn();
            RblxUI::Text("%d", hovered); RblxUI::NextColumn();
        }
        RblxUI::Columns(1);
        RblxUI::Separator();
        RblxUI::TreePop();
    }

    if (RblxUI::TreeNode("Borders"))
    {
        RBLXUI_DEMO_MARKER("Columns (legacy API)/Borders");
        // NB: Future columns API should allow automatic horizontal borders.
        static bool h_borders = true;
        static bool v_borders = true;
        static int columns_count = 4;
        const int lines_count = 3;
        RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 8);
        RblxUI::DragInt("##columns_count", &columns_count, 0.1f, 2, 10, "%d columns");
        if (columns_count < 2)
            columns_count = 2;
        RblxUI::SameLine();
        RblxUI::Checkbox("horizontal", &h_borders);
        RblxUI::SameLine();
        RblxUI::Checkbox("vertical", &v_borders);
        RblxUI::Columns(columns_count, NULL, v_borders);
        for (int i = 0; i < columns_count * lines_count; i++)
        {
            if (h_borders && RblxUI::GetColumnIndex() == 0)
                RblxUI::Separator();
            RblxUI::PushID(i);
            RblxUI::Text("%c%c%c", 'a' + i, 'a' + i, 'a' + i);
            RblxUI::Text("Width %.2f", RblxUI::GetColumnWidth());
            RblxUI::Text("Avail %.2f", RblxUI::GetContentRegionAvail().x);
            RblxUI::Text("Offset %.2f", RblxUI::GetColumnOffset());
            RblxUI::Text("Long text that is likely to clip");
            RblxUI::Button("Button", RblxVec2(-FLT_MIN, 0.0f));
            RblxUI::PopID();
            RblxUI::NextColumn();
        }
        RblxUI::Columns(1);
        if (h_borders)
            RblxUI::Separator();
        RblxUI::TreePop();
    }

    // Create multiple items in a same cell before switching to next column
    if (RblxUI::TreeNode("Mixed items"))
    {
        RBLXUI_DEMO_MARKER("Columns (legacy API)/Mixed items");
        RblxUI::Columns(3, "mixed");
        RblxUI::Separator();

        RblxUI::Text("Hello");
        RblxUI::Button("Banana");
        RblxUI::NextColumn();

        RblxUI::Text("RblxUI");
        RblxUI::Button("Apple");
        static float foo = 1.0f;
        RblxUI::InputFloat("red", &foo, 0.05f, 0, "%.3f");
        RblxUI::Text("An extra line here.");
        RblxUI::NextColumn();

        RblxUI::Text("Sailor");
        RblxUI::Button("Corniflower");
        static float bar = 1.0f;
        RblxUI::InputFloat("blue", &bar, 0.05f, 0, "%.3f");
        RblxUI::NextColumn();

        if (RblxUI::CollapsingHeader("Category A")) { RblxUI::Text("Blah blah blah"); } RblxUI::NextColumn();
        if (RblxUI::CollapsingHeader("Category B")) { RblxUI::Text("Blah blah blah"); } RblxUI::NextColumn();
        if (RblxUI::CollapsingHeader("Category C")) { RblxUI::Text("Blah blah blah"); } RblxUI::NextColumn();
        RblxUI::Columns(1);
        RblxUI::Separator();
        RblxUI::TreePop();
    }

    // Word wrapping
    if (RblxUI::TreeNode("Word-wrapping"))
    {
        RBLXUI_DEMO_MARKER("Columns (legacy API)/Word-wrapping");
        RblxUI::Columns(2, "word-wrapping");
        RblxUI::Separator();
        RblxUI::TextWrapped("The quick brown fox jumps over the lazy dog.");
        RblxUI::TextWrapped("Hello Left");
        RblxUI::NextColumn();
        RblxUI::TextWrapped("The quick brown fox jumps over the lazy dog.");
        RblxUI::TextWrapped("Hello Right");
        RblxUI::Columns(1);
        RblxUI::Separator();
        RblxUI::TreePop();
    }

    if (RblxUI::TreeNode("Horizontal Scrolling"))
    {
        RBLXUI_DEMO_MARKER("Columns (legacy API)/Horizontal Scrolling");
        RblxUI::SetNextWindowContentSize(RblxVec2(1500.0f, 0.0f));
        RblxVec2 child_size = RblxVec2(0, RblxUI::GetFontSize() * 20.0f);
        RblxUI::BeginChild("##ScrollingRegion", child_size, RblxUIChildFlags_None, RblxUIWindowFlags_HorizontalScrollbar);
        RblxUI::Columns(10);

        // Also demonstrate using clipper for large vertical lists
        int ITEMS_COUNT = 2000;
        RblxUIListClipper clipper;
        clipper.Begin(ITEMS_COUNT);
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                for (int j = 0; j < 10; j++)
                {
                    RblxUI::Text("Line %d Column %d...", i, j);
                    RblxUI::NextColumn();
                }
        }
        RblxUI::Columns(1);
        RblxUI::EndChild();
        RblxUI::TreePop();
    }

    if (RblxUI::TreeNode("Tree"))
    {
        RBLXUI_DEMO_MARKER("Columns (legacy API)/Tree");
        RblxUI::Columns(2, "tree", true);
        for (int x = 0; x < 3; x++)
        {
            bool open1 = RblxUI::TreeNode((void*)(intptr_t)x, "Node%d", x);
            RblxUI::NextColumn();
            RblxUI::Text("Node contents");
            RblxUI::NextColumn();
            if (open1)
            {
                for (int y = 0; y < 3; y++)
                {
                    bool open2 = RblxUI::TreeNode((void*)(intptr_t)y, "Node%d.%d", x, y);
                    RblxUI::NextColumn();
                    RblxUI::Text("Node contents");
                    if (open2)
                    {
                        RblxUI::Text("Even more contents");
                        if (RblxUI::TreeNode("Tree in column"))
                        {
                            RblxUI::Text("The quick brown fox jumps over the lazy dog");
                            RblxUI::TreePop();
                        }
                    }
                    RblxUI::NextColumn();
                    if (open2)
                        RblxUI::TreePop();
                }
                RblxUI::TreePop();
            }
        }
        RblxUI::Columns(1);
        RblxUI::TreePop();
    }

    RblxUI::TreePop();
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowInputs()
//-----------------------------------------------------------------------------

static void DemoWindowInputs()
{
    if (RblxUI::CollapsingHeader("Inputs & Focus"))
    {
        RblxUIIO& io = RblxUI::GetIO();

        // Display inputs submitted to RblxUIIO
        RblxUI::SetNextItemOpen(true, RblxUICond_Once);
        bool inputs_opened = RblxUI::TreeNode("Inputs");
        RblxUI::SameLine();
        HelpMarker(
            "This is a simplified view. See more detailed input state:\n"
            "- in 'Tools->Metrics/Debugger->Inputs'.\n"
            "- in 'Tools->Debug Log->IO'.");
        if (inputs_opened)
        {
            RBLXUI_DEMO_MARKER("Inputs & Focus/Inputs");
            if (RblxUI::IsMousePosValid())
                RblxUI::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
            else
                RblxUI::Text("Mouse pos: <INVALID>");
            RblxUI::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
            RblxUI::Text("Mouse down:");
            for (int i = 0; i < RBLXUI_COUNTOF(io.MouseDown); i++) if (RblxUI::IsMouseDown(i)) { RblxUI::SameLine(); RblxUI::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
            RblxUI::Text("Mouse wheel: %.1f", io.MouseWheel);
            RblxUI::Text("Mouse clicked count:");
            for (int i = 0; i < RBLXUI_COUNTOF(io.MouseDown); i++) if (io.MouseClickedCount[i] > 0) { RblxUI::SameLine(); RblxUI::Text("b%d: %d", i, io.MouseClickedCount[i]); }

            // We iterate both legacy native range and named RblxUIKey ranges. This is a little unusual/odd but this allows
            // displaying the data for old/new backends.
            // User code should never have to go through such hoops!
            // You can generally iterate between RblxUIKey_NamedKey_BEGIN and RblxUIKey_NamedKey_END.
            struct funcs { static bool IsLegacyNativeDupe(RblxUIKey) { return false; } };
            RblxUIKey start_key = RblxUIKey_NamedKey_BEGIN;
            RblxUI::Text("Keys down:");         for (RblxUIKey key = start_key; key < RblxUIKey_NamedKey_END; key = (RblxUIKey)(key + 1)) { if (funcs::IsLegacyNativeDupe(key) || !RblxUI::IsKeyDown(key)) continue; RblxUI::SameLine(); RblxUI::Text((key < RblxUIKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", RblxUI::GetKeyName(key), key); }
            RblxUI::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
            RblxUI::Text("Chars queue:");       for (int i = 0; i < io.InputQueueCharacters.Size; i++) { RblxWchar c = io.InputQueueCharacters[i]; RblxUI::SameLine();  RblxUI::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.

            RblxUI::TreePop();
        }

        // Display RblxUIIO output flags
        RblxUI::SetNextItemOpen(true, RblxUICond_Once);
        bool outputs_opened = RblxUI::TreeNode("Outputs");
        RblxUI::SameLine();
        HelpMarker(
            "The value of io.WantCaptureMouse and io.WantCaptureKeyboard are normally set by Dear RblxUI "
            "to instruct your application of how to route inputs. Typically, when a value is true, it means "
            "Dear RblxUI wants the corresponding inputs and we expect the underlying application to ignore them.\n\n"
            "The most typical case is: when hovering a window, Dear RblxUI set io.WantCaptureMouse to true, "
            "and underlying application should ignore mouse inputs (in practice there are many and more subtle "
            "rules leading to how those flags are set).");
        if (outputs_opened)
        {
            RBLXUI_DEMO_MARKER("Inputs & Focus/Outputs");
            RblxUI::Text("io.WantCaptureMouse: %d", io.WantCaptureMouse);
            RblxUI::Text("io.WantCaptureMouseUnlessPopupClose: %d", io.WantCaptureMouseUnlessPopupClose);
            RblxUI::Text("io.WantCaptureKeyboard: %d", io.WantCaptureKeyboard);
            RblxUI::Text("io.WantTextInput: %d", io.WantTextInput);
            RblxUI::Text("io.WantSetMousePos: %d", io.WantSetMousePos);
            RblxUI::Text("io.NavActive: %d, io.NavVisible: %d", io.NavActive, io.NavVisible);

            RBLXUI_DEMO_MARKER("Inputs & Focus/Outputs/WantCapture override");
            if (RblxUI::TreeNode("WantCapture override"))
            {
                HelpMarker(
                    "Hovering the colored canvas will override io.WantCaptureXXX fields.\n"
                    "Notice how normally (when set to none), the value of io.WantCaptureKeyboard would be false when hovering "
                    "and true when clicking.");
                static int capture_override_mouse = -1;
                static int capture_override_keyboard = -1;
                const char* capture_override_desc[] = { "None", "Set to false", "Set to true" };
                RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 15);
                RblxUI::SliderInt("SetNextFrameWantCaptureMouse() on hover", &capture_override_mouse, -1, +1, capture_override_desc[capture_override_mouse + 1], RblxUISliderFlags_AlwaysClamp);
                RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 15);
                RblxUI::SliderInt("SetNextFrameWantCaptureKeyboard() on hover", &capture_override_keyboard, -1, +1, capture_override_desc[capture_override_keyboard + 1], RblxUISliderFlags_AlwaysClamp);

                RblxUI::ColorButton("##panel", RblxVec4(0.7f, 0.1f, 0.7f, 1.0f), RblxUIColorEditFlags_NoTooltip | RblxUIColorEditFlags_NoDragDrop, RblxVec2(128.0f, 96.0f)); // Dummy item
                if (RblxUI::IsItemHovered() && capture_override_mouse != -1)
                    RblxUI::SetNextFrameWantCaptureMouse(capture_override_mouse == 1);
                if (RblxUI::IsItemHovered() && capture_override_keyboard != -1)
                    RblxUI::SetNextFrameWantCaptureKeyboard(capture_override_keyboard == 1);

                RblxUI::TreePop();
            }
            RblxUI::TreePop();
        }

        // Demonstrate using Shortcut() and Routing Policies.
        // The general flow is:
        // - Code interested in a chord (e.g. "Ctrl+A") declares their intent.
        // - Multiple locations may be interested in same chord! Routing helps find a winner.
        // - Every frame, we resolve all claims and assign one owner if the modifiers are matching.
        // - The lower-level function is 'bool SetShortcutRouting()', returns true when caller got the route.
        // - Most of the times, SetShortcutRouting() is not called directly. User mostly calls Shortcut() with routing flags.
        // - If you call Shortcut() WITHOUT any routing option, it uses RblxUIInputFlags_RouteFocused.
        // TL;DR: Most uses will simply be:
        // - Shortcut(RblxUIMod_Ctrl | RblxUIKey_A); // Use RblxUIInputFlags_RouteFocused policy.
        if (RblxUI::TreeNode("Shortcuts"))
        {
            RBLXUI_DEMO_MARKER("Inputs & Focus/Shortcuts");
            static RblxUIInputFlags route_options = RblxUIInputFlags_Repeat;
            static RblxUIInputFlags route_type = RblxUIInputFlags_RouteFocused;
            RblxUI::CheckboxFlags("RblxUIInputFlags_Repeat", &route_options, RblxUIInputFlags_Repeat);
            RblxUI::RadioButton("RblxUIInputFlags_RouteActive", &route_type, RblxUIInputFlags_RouteActive);
            RblxUI::RadioButton("RblxUIInputFlags_RouteFocused (default)", &route_type, RblxUIInputFlags_RouteFocused);
            RblxUI::Indent();
            RblxUI::BeginDisabled(route_type != RblxUIInputFlags_RouteFocused);
            RblxUI::CheckboxFlags("RblxUIInputFlags_RouteOverActive##0", &route_options, RblxUIInputFlags_RouteOverActive);
            RblxUI::EndDisabled();
            RblxUI::Unindent();
            RblxUI::RadioButton("RblxUIInputFlags_RouteGlobal", &route_type, RblxUIInputFlags_RouteGlobal);
            RblxUI::Indent();
            RblxUI::BeginDisabled(route_type != RblxUIInputFlags_RouteGlobal);
            RblxUI::CheckboxFlags("RblxUIInputFlags_RouteOverFocused", &route_options, RblxUIInputFlags_RouteOverFocused);
            RblxUI::CheckboxFlags("RblxUIInputFlags_RouteOverActive", &route_options, RblxUIInputFlags_RouteOverActive);
            RblxUI::CheckboxFlags("RblxUIInputFlags_RouteUnlessBgFocused", &route_options, RblxUIInputFlags_RouteUnlessBgFocused);
            RblxUI::EndDisabled();
            RblxUI::Unindent();
            RblxUI::RadioButton("RblxUIInputFlags_RouteAlways", &route_type, RblxUIInputFlags_RouteAlways);
            RblxUIInputFlags flags = route_type | route_options; // Merged flags
            if (route_type != RblxUIInputFlags_RouteGlobal)
                flags &= ~(RblxUIInputFlags_RouteOverFocused | RblxUIInputFlags_RouteOverActive | RblxUIInputFlags_RouteUnlessBgFocused);

            RblxUI::SeparatorText("Using SetNextItemShortcut()");
            RblxUI::Text("Ctrl+S");
            RblxUI::SetNextItemShortcut(RblxUIMod_Ctrl | RblxUIKey_S, flags | RblxUIInputFlags_Tooltip);
            RblxUI::Button("Save");
            RblxUI::Text("Alt+F");
            RblxUI::SetNextItemShortcut(RblxUIMod_Alt | RblxUIKey_F, flags | RblxUIInputFlags_Tooltip);
            static float f = 0.5f;
            RblxUI::SliderFloat("Factor", &f, 0.0f, 1.0f);

            RblxUI::SeparatorText("Using Shortcut()");
            const float line_height = RblxUI::GetTextLineHeightWithSpacing();
            const RblxUIKeyChord key_chord = RblxUIMod_Ctrl | RblxUIKey_A;

            RblxUI::Text("Ctrl+A");
            RblxUI::Text("IsWindowFocused: %d, Shortcut: %s", RblxUI::IsWindowFocused(), RblxUI::Shortcut(key_chord, flags) ? "PRESSED" : "...");

            RblxUI::PushStyleColor(RblxUICol_ChildBg, RblxVec4(1.0f, 0.0f, 1.0f, 0.1f));

            RblxUI::BeginChild("WindowA", RblxVec2(-FLT_MIN, line_height * 14), true);
            RblxUI::Text("Press Ctrl+A and see who receives it!");
            RblxUI::Separator();

            // 1: Window polling for Ctrl+A
            RblxUI::Text("(in WindowA)");
            RblxUI::Text("IsWindowFocused: %d, Shortcut: %s", RblxUI::IsWindowFocused(), RblxUI::Shortcut(key_chord, flags) ? "PRESSED" : "...");

            // 2: InputText also polling for Ctrl+A: it always uses _RouteFocused internally (gets priority when active)
            // (Commented because the owner-aware version of Shortcut() is still in rblxui_internal.h)
            //char str[16] = "Press Ctrl+A";
            //RblxUI::Spacing();
            //RblxUI::InputText("InputTextB", str, RBLXUI_COUNTOF(str), RblxUIInputTextFlags_ReadOnly);
            //RblxUIID item_id = RblxUI::GetItemID();
            //RblxUI::SameLine(); HelpMarker("Internal widgets always use _RouteFocused");
            //RblxUI::Text("IsWindowFocused: %d, Shortcut: %s", RblxUI::IsWindowFocused(), RblxUI::Shortcut(key_chord, flags, item_id) ? "PRESSED" : "...");

            // 3: Dummy child is not claiming the route: focusing them shouldn't steal route away from WindowA
            RblxUI::BeginChild("ChildD", RblxVec2(-FLT_MIN, line_height * 4), true);
            RblxUI::Text("(in ChildD: not using same Shortcut)");
            RblxUI::Text("IsWindowFocused: %d", RblxUI::IsWindowFocused());
            RblxUI::EndChild();

            // 4: Child window polling for Ctrl+A. It is deeper than WindowA and gets priority when focused.
            RblxUI::BeginChild("ChildE", RblxVec2(-FLT_MIN, line_height * 4), true);
            RblxUI::Text("(in ChildE: using same Shortcut)");
            RblxUI::Text("IsWindowFocused: %d, Shortcut: %s", RblxUI::IsWindowFocused(), RblxUI::Shortcut(key_chord, flags) ? "PRESSED" : "...");
            RblxUI::EndChild();

            // 5: In a popup
            if (RblxUI::Button("Open Popup"))
                RblxUI::OpenPopup("PopupF");
            if (RblxUI::BeginPopup("PopupF"))
            {
                RblxUI::Text("(in PopupF)");
                RblxUI::Text("IsWindowFocused: %d, Shortcut: %s", RblxUI::IsWindowFocused(), RblxUI::Shortcut(key_chord, flags) ? "PRESSED" : "...");
                // (Commented because the owner-aware version of Shortcut() is still in rblxui_internal.h)
                //RblxUI::InputText("InputTextG", str, RBLXUI_COUNTOF(str), RblxUIInputTextFlags_ReadOnly);
                //RblxUI::Text("IsWindowFocused: %d, Shortcut: %s", RblxUI::IsWindowFocused(), RblxUI::Shortcut(key_chord, flags, RblxUI::GetItemID()) ? "PRESSED" : "...");
                RblxUI::EndPopup();
            }
            RblxUI::EndChild();
            RblxUI::PopStyleColor();

            RblxUI::TreePop();
        }

        // Display mouse cursors
        if (RblxUI::TreeNode("Mouse Cursors"))
        {
            RBLXUI_DEMO_MARKER("Inputs & Focus/Mouse Cursors");
            const char* mouse_cursors_names[] = { "Arrow", "TextInput", "ResizeAll", "ResizeNS", "ResizeEW", "ResizeNESW", "ResizeNWSE", "Hand", "Wait", "Progress", "NotAllowed" };
            RBLXUI_ASSERT(RBLXUI_COUNTOF(mouse_cursors_names) == RblxUIMouseCursor_COUNT);

            RblxUIMouseCursor current = RblxUI::GetMouseCursor();
            const char* cursor_name = (current >= RblxUIMouseCursor_Arrow) && (current < RblxUIMouseCursor_COUNT) ? mouse_cursors_names[current] : "N/A";
            RblxUI::Text("Current mouse cursor = %d: %s", current, cursor_name);
            RblxUI::BeginDisabled(true);
            RblxUI::CheckboxFlags("io.BackendFlags: HasMouseCursors", &io.BackendFlags, RblxUIBackendFlags_HasMouseCursors);
            RblxUI::EndDisabled();

            RblxUI::Text("Hover to see mouse cursors:");
            RblxUI::SameLine(); HelpMarker(
                "Your application can render a different mouse cursor based on what RblxUI::GetMouseCursor() returns. "
                "If software cursor rendering (io.MouseDrawCursor) is set RblxUI will draw the right cursor for you, "
                "otherwise your backend needs to handle it.");
            for (int i = 0; i < RblxUIMouseCursor_COUNT; i++)
            {
                char label[32];
                sprintf(label, "Mouse cursor %d: %s", i, mouse_cursors_names[i]);
                RblxUI::Bullet(); RblxUI::Selectable(label, false);
                if (RblxUI::IsItemHovered())
                    RblxUI::SetMouseCursor(i);
            }
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Tabbing"))
        {
            RBLXUI_DEMO_MARKER("Inputs & Focus/Tabbing");
            RblxUI::Text("Use Tab/Shift+Tab to cycle through keyboard editable fields.");
            static char buf[32] = "hello";
            RblxUI::InputText("1", buf, RBLXUI_COUNTOF(buf));
            RblxUI::InputText("2", buf, RBLXUI_COUNTOF(buf));
            RblxUI::InputText("3", buf, RBLXUI_COUNTOF(buf));
            RblxUI::PushItemFlag(RblxUIItemFlags_NoTabStop, true);
            RblxUI::InputText("4 (tab skip)", buf, RBLXUI_COUNTOF(buf));
            RblxUI::SameLine(); HelpMarker("Item won't be cycled through when using TAB or Shift+Tab.");
            RblxUI::PopItemFlag();
            RblxUI::InputText("5", buf, RBLXUI_COUNTOF(buf));
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Focus from code"))
        {
            RBLXUI_DEMO_MARKER("Inputs & Focus/Focus from code");
            bool focus_1 = RblxUI::Button("Focus on 1"); RblxUI::SameLine();
            bool focus_2 = RblxUI::Button("Focus on 2"); RblxUI::SameLine();
            bool focus_3 = RblxUI::Button("Focus on 3");
            int has_focus = 0;
            static char buf[128] = "click on a button to set focus";

            if (focus_1) RblxUI::SetKeyboardFocusHere();
            RblxUI::InputText("1", buf, RBLXUI_COUNTOF(buf));
            if (RblxUI::IsItemActive()) has_focus = 1;

            if (focus_2) RblxUI::SetKeyboardFocusHere();
            RblxUI::InputText("2", buf, RBLXUI_COUNTOF(buf));
            if (RblxUI::IsItemActive()) has_focus = 2;

            RblxUI::PushItemFlag(RblxUIItemFlags_NoTabStop, true);
            if (focus_3) RblxUI::SetKeyboardFocusHere();
            RblxUI::InputText("3 (tab skip)", buf, RBLXUI_COUNTOF(buf));
            if (RblxUI::IsItemActive()) has_focus = 3;
            RblxUI::SameLine(); HelpMarker("Item won't be cycled through when using TAB or Shift+Tab.");
            RblxUI::PopItemFlag();

            if (has_focus)
                RblxUI::Text("Item with focus: %d", has_focus);
            else
                RblxUI::Text("Item with focus: <none>");

            // Use >= 0 parameter to SetKeyboardFocusHere() to focus an upcoming item
            static float f3[3] = { 0.0f, 0.0f, 0.0f };
            int focus_ahead = -1;
            if (RblxUI::Button("Focus on X")) { focus_ahead = 0; } RblxUI::SameLine();
            if (RblxUI::Button("Focus on Y")) { focus_ahead = 1; } RblxUI::SameLine();
            if (RblxUI::Button("Focus on Z")) { focus_ahead = 2; }
            if (focus_ahead != -1) RblxUI::SetKeyboardFocusHere(focus_ahead);
            RblxUI::SliderFloat3("Float3", &f3[0], 0.0f, 1.0f);

            RblxUI::TextWrapped("NB: Cursor & selection are preserved when refocusing last used item in code.");
            RblxUI::TreePop();
        }

        if (RblxUI::TreeNode("Dragging"))
        {
            RBLXUI_DEMO_MARKER("Inputs & Focus/Dragging");
            RblxUI::TextWrapped("You can use RblxUI::GetMouseDragDelta(0) to query for the dragged amount on any widget.");
            for (int button = 0; button < 3; button++)
            {
                RblxUI::Text("IsMouseDragging(%d):", button);
                RblxUI::Text("  w/ default threshold: %d,", RblxUI::IsMouseDragging(button));
                RblxUI::Text("  w/ zero threshold: %d,", RblxUI::IsMouseDragging(button, 0.0f));
                RblxUI::Text("  w/ large threshold: %d,", RblxUI::IsMouseDragging(button, 20.0f));
            }

            RblxUI::Button("Drag Me");
            if (RblxUI::IsItemActive())
                RblxUI::GetForegroundDrawList()->AddLine(io.MouseClickedPos[0], io.MousePos, RblxUI::GetColorU32(RblxUICol_Button), 4.0f); // Draw a line between the button and the mouse cursor

            // Drag operations gets "unlocked" when the mouse has moved past a certain threshold
            // (the default threshold is stored in io.MouseDragThreshold). You can request a lower or higher
            // threshold using the second parameter of IsMouseDragging() and GetMouseDragDelta().
            RblxVec2 value_raw = RblxUI::GetMouseDragDelta(0, 0.0f);
            RblxVec2 value_with_lock_threshold = RblxUI::GetMouseDragDelta(0);
            RblxVec2 mouse_delta = io.MouseDelta;
            RblxUI::Text("GetMouseDragDelta(0):");
            RblxUI::Text("  w/ default threshold: (%.1f, %.1f)", value_with_lock_threshold.x, value_with_lock_threshold.y);
            RblxUI::Text("  w/ zero threshold: (%.1f, %.1f)", value_raw.x, value_raw.y);
            RblxUI::Text("io.MouseDelta: (%.1f, %.1f)", mouse_delta.x, mouse_delta.y);
            RblxUI::TreePop();
        }
    }
}

//-----------------------------------------------------------------------------
// [SECTION] About Window / ShowAboutWindow()
// Access from Dear RblxUI Demo -> Tools -> About
//-----------------------------------------------------------------------------

void RblxUI::ShowAboutWindow(bool* p_open)
{
    if (!RblxUI::Begin("About Dear RblxUI", p_open, RblxUIWindowFlags_AlwaysAutoResize))
    {
        RblxUI::End();
        return;
    }
    RBLXUI_DEMO_MARKER("Tools/About Dear RblxUI");
    RblxUI::Text("Dear RblxUI %s (%d)", RBLXUI_VERSION, RBLXUI_VERSION_NUM);

    RblxUI::TextLinkOpenURL("Homepage", "https://github.com/ocornut/rblxui");
    RblxUI::SameLine();
    RblxUI::TextLinkOpenURL("FAQ", "https://github.com/ocornut/rblxui/blob/master/docs/FAQ.md");
    RblxUI::SameLine();
    RblxUI::TextLinkOpenURL("Wiki", "https://github.com/ocornut/rblxui/wiki");
    RblxUI::SameLine();
    RblxUI::TextLinkOpenURL("Extensions", "https://github.com/ocornut/rblxui/wiki/Useful-Extensions");
    RblxUI::SameLine();
    RblxUI::TextLinkOpenURL("Releases", "https://github.com/ocornut/rblxui/releases");
    RblxUI::SameLine();
    RblxUI::TextLinkOpenURL("Funding", "https://github.com/ocornut/rblxui/wiki/Funding");

    RblxUI::Separator();
    RblxUI::Text("(c) 2014-2026 Omar Cornut");
    RblxUI::Text("Developed by Omar Cornut and all Dear RblxUI contributors.");
    RblxUI::Text("Dear RblxUI is licensed under the MIT License, see LICENSE for more information.");
    RblxUI::Text("If your company uses this, please consider funding the project.");

    static bool show_config_info = false;
    RblxUI::Checkbox("Config/Build Information", &show_config_info);
    if (show_config_info)
    {
        RblxUIIO& io = RblxUI::GetIO();
        RblxUIStyle& style = RblxUI::GetStyle();

        bool copy_to_clipboard = RblxUI::Button("Copy to clipboard");
        RblxVec2 child_size = RblxVec2(0, RblxUI::GetTextLineHeightWithSpacing() * 18);
        RblxUI::BeginChild(RblxUI::GetID("cfg_infos"), child_size, RblxUIChildFlags_FrameStyle);
        if (copy_to_clipboard)
        {
            RblxUI::LogToClipboard();
            RblxUI::LogText("// (Copy from the next line. Keep the ``` markers for formatting.)\n");
            RblxUI::LogText("```cpp\n"); // Back quotes will make text appears without formatting when pasting on GitHub
        }

        RblxUI::Text("Dear RblxUI %s (%d)", RBLXUI_VERSION, RBLXUI_VERSION_NUM);
        RblxUI::Separator();
        RblxUI::Text("sizeof(size_t): %d, sizeof(RblxDrawIdx): %d, sizeof(RblxDrawVert): %d", (int)sizeof(size_t), (int)sizeof(RblxDrawIdx), (int)sizeof(RblxDrawVert));
        RblxUI::Text("define: __cplusplus=%d", (int)__cplusplus);
#ifdef RBLXUI_ENABLE_TEST_ENGINE
        RblxUI::Text("define: RBLXUI_ENABLE_TEST_ENGINE");
#endif
#ifdef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
        RblxUI::Text("define: RBLXUI_DISABLE_OBSOLETE_FUNCTIONS");
#endif
#ifdef RBLXUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
        RblxUI::Text("define: RBLXUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS");
#endif
#ifdef RBLXUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
        RblxUI::Text("define: RBLXUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS");
#endif
#ifdef RBLXUI_DISABLE_WIN32_FUNCTIONS
        RblxUI::Text("define: RBLXUI_DISABLE_WIN32_FUNCTIONS");
#endif
#ifdef RBLXUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
        RblxUI::Text("define: RBLXUI_DISABLE_DEFAULT_SHELL_FUNCTIONS");
#endif
#ifdef RBLXUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
        RblxUI::Text("define: RBLXUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS");
#endif
#ifdef RBLXUI_DISABLE_DEFAULT_MATH_FUNCTIONS
        RblxUI::Text("define: RBLXUI_DISABLE_DEFAULT_MATH_FUNCTIONS");
#endif
#ifdef RBLXUI_DISABLE_DEFAULT_FILE_FUNCTIONS
        RblxUI::Text("define: RBLXUI_DISABLE_DEFAULT_FILE_FUNCTIONS");
#endif
#ifdef RBLXUI_DISABLE_FILE_FUNCTIONS
        RblxUI::Text("define: RBLXUI_DISABLE_FILE_FUNCTIONS");
#endif
#ifdef RBLXUI_DISABLE_DEFAULT_ALLOCATORS
        RblxUI::Text("define: RBLXUI_DISABLE_DEFAULT_ALLOCATORS");
#endif
#ifdef RBLXUI_USE_BGRA_PACKED_COLOR
        RblxUI::Text("define: RBLXUI_USE_BGRA_PACKED_COLOR");
#endif
#ifdef _WIN32
        RblxUI::Text("define: _WIN32");
#endif
#ifdef _WIN64
        RblxUI::Text("define: _WIN64");
#endif
#ifdef __linux__
        RblxUI::Text("define: __linux__");
#endif
#ifdef __APPLE__
        RblxUI::Text("define: __APPLE__");
#endif
#ifdef _MSC_VER
        RblxUI::Text("define: _MSC_VER=%d", _MSC_VER);
#endif
#ifdef _MSVC_LANG
        RblxUI::Text("define: _MSVC_LANG=%d", (int)_MSVC_LANG);
#endif
#ifdef __MINGW32__
        RblxUI::Text("define: __MINGW32__");
#endif
#ifdef __MINGW64__
        RblxUI::Text("define: __MINGW64__");
#endif
#ifdef __GNUC__
        RblxUI::Text("define: __GNUC__=%d", (int)__GNUC__);
#endif
#ifdef __clang_version__
        RblxUI::Text("define: __clang_version__=%s", __clang_version__);
#endif
#ifdef __EMSCRIPTEN__
        RblxUI::Text("define: __EMSCRIPTEN__");
#ifdef __EMSCRIPTEN_MAJOR__
        RblxUI::Text("Emscripten: %d.%d.%d", __EMSCRIPTEN_MAJOR__, __EMSCRIPTEN_MINOR__, __EMSCRIPTEN_TINY__);
#else
        RblxUI::Text("Emscripten: %d.%d.%d", __EMSCRIPTEN_major__, __EMSCRIPTEN_minor__, __EMSCRIPTEN_tiny__);
#endif
#endif
#ifdef RBLXUI_HAS_VIEWPORT
        RblxUI::Text("define: RBLXUI_HAS_VIEWPORT");
#endif
#ifdef RBLXUI_HAS_DOCK
        RblxUI::Text("define: RBLXUI_HAS_DOCK");
#endif
#ifdef NDEBUG
        RblxUI::Text("define: NDEBUG");
#endif

        // Heuristic to detect no-op RBLXUI_ASSERT() macros
        // - This is designed so people opening bug reports would convey and notice that they have disabled asserts for Dear RblxUI code.
        // - 16 is > strlen("((void)(_EXPR))") which we suggested in our rblxuiconfig.h template as a possible way to disable.
        int assert_runs_expression = 0;
        RBLXUI_ASSERT(++assert_runs_expression);
        int assert_expand_len = (int)strlen(RBLXUI_STRINGIFY((RBLXUI_ASSERT(true))));
        bool assert_maybe_disabled = (!assert_runs_expression || assert_expand_len <= 16);
        RblxUI::Text("RBLXUI_ASSERT: runs expression: %s. expand size: %s%s",
            assert_runs_expression ? "OK" : "KO", (assert_expand_len > 16) ? "OK" : "KO", assert_maybe_disabled ? " (MAYBE DISABLED?!)" : "");
        if (assert_maybe_disabled)
        {
            RblxUI::SameLine();
            HelpMarker("RBLXUI_ASSERT() calls assert() by default. Compiling with NDEBUG will usually strip out assert() to nothing, which is NOT recommended because we use asserts to notify of programmer mistakes!");
        }

        RblxUI::Separator();
        RblxUI::Text("io.BackendPlatformName: %s", io.BackendPlatformName ? io.BackendPlatformName : "NULL");
        RblxUI::Text("io.BackendRendererName: %s", io.BackendRendererName ? io.BackendRendererName : "NULL");
        RblxUI::Text("io.ConfigFlags: 0x%08X", io.ConfigFlags);
        if (io.ConfigFlags & RblxUIConfigFlags_NavEnableKeyboard)        RblxUI::Text(" NavEnableKeyboard");
        if (io.ConfigFlags & RblxUIConfigFlags_NavEnableGamepad)         RblxUI::Text(" NavEnableGamepad");
        if (io.ConfigFlags & RblxUIConfigFlags_NoMouse)                  RblxUI::Text(" NoMouse");
        if (io.ConfigFlags & RblxUIConfigFlags_NoMouseCursorChange)      RblxUI::Text(" NoMouseCursorChange");
        if (io.ConfigFlags & RblxUIConfigFlags_NoKeyboard)               RblxUI::Text(" NoKeyboard");
        if (io.ConfigFlags & RblxUIConfigFlags_DockingEnable)            RblxUI::Text(" DockingEnable");
        if (io.ConfigFlags & RblxUIConfigFlags_ViewportsEnable)          RblxUI::Text(" ViewportsEnable");
        if (io.MouseDrawCursor)                                         RblxUI::Text("io.MouseDrawCursor");
        if (io.ConfigDpiScaleFonts)                                     RblxUI::Text("io.ConfigDpiScaleFonts");
        if (io.ConfigDpiScaleViewports)                                 RblxUI::Text("io.ConfigDpiScaleViewports");
        if (io.ConfigViewportsNoAutoMerge)                              RblxUI::Text("io.ConfigViewportsNoAutoMerge");
        if (io.ConfigViewportsNoTaskBarIcon)                            RblxUI::Text("io.ConfigViewportsNoTaskBarIcon");
        if (io.ConfigViewportsNoDecoration)                             RblxUI::Text("io.ConfigViewportsNoDecoration");
        if (io.ConfigViewportsNoDefaultParent)                          RblxUI::Text("io.ConfigViewportsNoDefaultParent");
        if (io.ConfigDockingNoSplit)                                    RblxUI::Text("io.ConfigDockingNoSplit");
        if (io.ConfigDockingNoDockingOver)                              RblxUI::Text("io.ConfigDockingNoDockingOver");
        if (io.ConfigDockingWithShift)                                  RblxUI::Text("io.ConfigDockingWithShift");
        if (io.ConfigDockingAlwaysTabBar)                               RblxUI::Text("io.ConfigDockingAlwaysTabBar");
        if (io.ConfigDockingTransparentPayload)                         RblxUI::Text("io.ConfigDockingTransparentPayload");
        if (io.ConfigMacOSXBehaviors)                                   RblxUI::Text("io.ConfigMacOSXBehaviors");
        if (io.ConfigNavMoveSetMousePos)                                RblxUI::Text("io.ConfigNavMoveSetMousePos");
        if (io.ConfigNavCaptureKeyboard)                                RblxUI::Text("io.ConfigNavCaptureKeyboard");
        if (io.ConfigInputTextCursorBlink)                              RblxUI::Text("io.ConfigInputTextCursorBlink");
        if (io.ConfigWindowsResizeFromEdges)                            RblxUI::Text("io.ConfigWindowsResizeFromEdges");
        if (io.ConfigWindowsMoveFromTitleBarOnly)                       RblxUI::Text("io.ConfigWindowsMoveFromTitleBarOnly");
        if (io.ConfigMemoryCompactTimer >= 0.0f)                        RblxUI::Text("io.ConfigMemoryCompactTimer = %.1f", io.ConfigMemoryCompactTimer);
        RblxUI::Text("io.BackendFlags: 0x%08X", io.BackendFlags);
        if (io.BackendFlags & RblxUIBackendFlags_HasGamepad)             RblxUI::Text(" HasGamepad");
        if (io.BackendFlags & RblxUIBackendFlags_HasMouseCursors)        RblxUI::Text(" HasMouseCursors");
        if (io.BackendFlags & RblxUIBackendFlags_HasSetMousePos)         RblxUI::Text(" HasSetMousePos");
        if (io.BackendFlags & RblxUIBackendFlags_PlatformHasViewports)   RblxUI::Text(" PlatformHasViewports");
        if (io.BackendFlags & RblxUIBackendFlags_HasMouseHoveredViewport)RblxUI::Text(" HasMouseHoveredViewport");
        if (io.BackendFlags & RblxUIBackendFlags_HasParentViewport)      RblxUI::Text(" HasParentViewport");
        if (io.BackendFlags & RblxUIBackendFlags_RendererHasVtxOffset)   RblxUI::Text(" RendererHasVtxOffset");
        if (io.BackendFlags & RblxUIBackendFlags_RendererHasTextures)    RblxUI::Text(" RendererHasTextures");
        if (io.BackendFlags & RblxUIBackendFlags_RendererHasViewports)   RblxUI::Text(" RendererHasViewports");
        RblxUI::Separator();
        RblxUI::Text("io.Fonts: %d fonts, Flags: 0x%08X, TexSize: %d,%d", io.Fonts->Fonts.Size, io.Fonts->Flags, io.Fonts->TexData->Width, io.Fonts->TexData->Height);
        RblxUI::Text("io.Fonts->FontLoaderName: %s", io.Fonts->FontLoaderName ? io.Fonts->FontLoaderName : "NULL");
        RblxUI::Text("io.DisplaySize: %.2f,%.2f", io.DisplaySize.x, io.DisplaySize.y);
        RblxUI::Text("io.DisplayFramebufferScale: %.2f,%.2f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        RblxUI::Separator();
        RblxUI::Text("style.WindowPadding: %.2f,%.2f", style.WindowPadding.x, style.WindowPadding.y);
        RblxUI::Text("style.WindowBorderSize: %.2f", style.WindowBorderSize);
        RblxUI::Text("style.FramePadding: %.2f,%.2f", style.FramePadding.x, style.FramePadding.y);
        RblxUI::Text("style.FrameRounding: %.2f", style.FrameRounding);
        RblxUI::Text("style.FrameBorderSize: %.2f", style.FrameBorderSize);
        RblxUI::Text("style.ItemSpacing: %.2f,%.2f", style.ItemSpacing.x, style.ItemSpacing.y);
        RblxUI::Text("style.ItemInnerSpacing: %.2f,%.2f", style.ItemInnerSpacing.x, style.ItemInnerSpacing.y);

        if (copy_to_clipboard)
        {
            RblxUI::LogText("\n```\n");
            RblxUI::LogFinish();
        }
        RblxUI::EndChild();
    }
    RblxUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Style Editor / ShowStyleEditor()
//-----------------------------------------------------------------------------
// - ShowStyleSelector()
// - ShowStyleEditor()
//-----------------------------------------------------------------------------

// Demo helper function to select among default colors. See ShowStyleEditor() for more advanced options.
bool RblxUI::ShowStyleSelector(const char* label)
{
    // FIXME: This is a bit tricky to get right as style are functions, they don't register a name nor the fact that one is active.
    // So we keep track of last active one among our limited selection.
    static int style_idx = -1;
    const char* style_names[] = { "Dark", "Light", "Classic" };
    bool ret = false;
    if (RblxUI::BeginCombo(label, (style_idx >= 0 && style_idx < RBLXUI_COUNTOF(style_names)) ? style_names[style_idx] : ""))
    {
        for (int n = 0; n < RBLXUI_COUNTOF(style_names); n++)
        {
            if (RblxUI::Selectable(style_names[n], style_idx == n, RblxUISelectableFlags_SelectOnNav))
            {
                style_idx = n;
                ret = true;
                switch (style_idx)
                {
                case 0: RblxUI::StyleColorsDark(); break;
                case 1: RblxUI::StyleColorsLight(); break;
                case 2: RblxUI::StyleColorsClassic(); break;
                }
            }
            else if (style_idx == n)
                RblxUI::SetItemDefaultFocus();
        }
        RblxUI::EndCombo();
    }
    return ret;
}

static const char* GetTreeLinesFlagsName(RblxUITreeNodeFlags flags)
{
    if (flags == RblxUITreeNodeFlags_DrawLinesNone) return "DrawLinesNone";
    if (flags == RblxUITreeNodeFlags_DrawLinesFull) return "DrawLinesFull";
    if (flags == RblxUITreeNodeFlags_DrawLinesToNodes) return "DrawLinesToNodes";
    return "";
}

// We omit the RblxUI:: prefix in this function, as we don't expect user to be copy and pasting this code.
void RblxUI::ShowStyleEditor(RblxUIStyle* ref)
{
    RBLXUI_DEMO_MARKER("Tools/Style Editor");
    // You can pass in a reference RblxUIStyle structure to compare to, revert to and save to
    // (without a reference style pointer, we will use one compared locally as a reference)
    RblxUIStyle& style = GetStyle();
    static RblxUIStyle ref_saved_style;

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
    const float max_border_size = RBLXUI_MAX(default_border_size, 2.0f);

    PushItemWidth(GetWindowWidth() * 0.50f);

    {
        // General
        SeparatorText("General");
        if ((GetIO().BackendFlags & RblxUIBackendFlags_RendererHasTextures) == 0)
        {
            BulletText("Warning: Font scaling will NOT be smooth, because\nRblxUIBackendFlags_RendererHasTextures is not set!");
            BulletText("For instructions, see:");
            SameLine();
            TextLinkOpenURL("docs/BACKENDS.md", "https://github.com/ocornut/rblxui/blob/master/docs/BACKENDS.md");
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
    if (BeginTabBar("##tabs", RblxUITabBarFlags_None))
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
            SliderFloat("TabBarOverlineSize", &style.TabBarOverlineSize, 0.0f, RBLXUI_MAX(3.0f, max_border_size), "%.0f");
            SameLine(); HelpMarker("Overline is only drawn over the selected tab when RblxUITabBarFlags_DrawSelectedOverline is set.");
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
            HelpMarker("[Experimental] Tree lines may not work in all situations (e.g. using a clipper) and may incurs slight traversal overhead.\n\nRblxUITreeNodeFlags_DrawLinesFull is faster than RblxUITreeNodeFlags_DrawLinesToNode.");
            if (combo_open)
            {
                const RblxUITreeNodeFlags options[] = { RblxUITreeNodeFlags_DrawLinesNone, RblxUITreeNodeFlags_DrawLinesFull, RblxUITreeNodeFlags_DrawLinesToNodes };
                for (RblxUITreeNodeFlags option : options)
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
                style.WindowMenuButtonPosition = (RblxUIDir)(window_menu_button_position - 1);

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
                    RblxUIHoveredFlags* p = (n == 0) ? &style.HoverFlagsForTooltipMouse : &style.HoverFlagsForTooltipNav;
                    CheckboxFlags("RblxUIHoveredFlags_DelayNone", p, RblxUIHoveredFlags_DelayNone);
                    CheckboxFlags("RblxUIHoveredFlags_DelayShort", p, RblxUIHoveredFlags_DelayShort);
                    CheckboxFlags("RblxUIHoveredFlags_DelayNormal", p, RblxUIHoveredFlags_DelayNormal);
                    CheckboxFlags("RblxUIHoveredFlags_Stationary", p, RblxUIHoveredFlags_Stationary);
                    CheckboxFlags("RblxUIHoveredFlags_NoSharedDelay", p, RblxUIHoveredFlags_NoSharedDelay);
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
                LogText("RblxVec4* colors = GetStyle().Colors;" RBLXUI_NEWLINE);
                for (int i = 0; i < RblxUICol_COUNT; i++)
                {
                    const RblxVec4& col = style.Colors[i];
                    const char* name = GetStyleColorName(i);
                    if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(RblxVec4)) != 0)
                        LogText("colors[RblxUICol_%s]%*s= RblxVec4(%.2ff, %.2ff, %.2ff, %.2ff);" RBLXUI_NEWLINE,
                            name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
                }
                LogFinish();
            }
            SameLine(); SetNextItemWidth(GetFontSize() * 10); Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
            SameLine(); Checkbox("Only Modified Colors", &output_only_modified);

            static RblxUITextFilter filter;
            filter.Draw("Filter colors", GetFontSize() * 16);

            static RblxUIColorEditFlags alpha_flags = 0;
            if (RadioButton("Opaque", alpha_flags == RblxUIColorEditFlags_AlphaOpaque))       { alpha_flags = RblxUIColorEditFlags_AlphaOpaque; } SameLine();
            if (RadioButton("Alpha",  alpha_flags == RblxUIColorEditFlags_None))              { alpha_flags = RblxUIColorEditFlags_None; } SameLine();
            if (RadioButton("Both",   alpha_flags == RblxUIColorEditFlags_AlphaPreviewHalf))  { alpha_flags = RblxUIColorEditFlags_AlphaPreviewHalf; } SameLine();
            HelpMarker(
                "In the color list:\n"
                "Left-click on color square to open color picker,\n"
                "Right-click to open edit options menu.");

            SetNextWindowSizeConstraints(RblxVec2(0.0f, GetTextLineHeightWithSpacing() * 10), RblxVec2(FLT_MAX, FLT_MAX));
            BeginChild("##colors", RblxVec2(0, 0), RblxUIChildFlags_Borders | RblxUIChildFlags_NavFlattened, RblxUIWindowFlags_AlwaysVerticalScrollbar | RblxUIWindowFlags_AlwaysHorizontalScrollbar);
            PushItemWidth(GetFontSize() * -12);
            for (int i = 0; i < RblxUICol_COUNT; i++)
            {
                const char* name = GetStyleColorName(i);
                if (!filter.PassFilter(name))
                    continue;
                PushID(i);
#ifndef RBLXUI_DISABLE_DEBUG_TOOLS
                if (Button("?"))
                    DebugFlashStyleColor((RblxUICol)i);
                SetItemTooltip("Flash given color to identify places where it is used.");
                SameLine();
#endif
                ColorEdit4("##color", (float*)&style.Colors[i], RblxUIColorEditFlags_AlphaBar | alpha_flags);
                if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(RblxVec4)) != 0)
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
            RblxUIIO& io = GetIO();
            RblxFontAtlas* atlas = io.Fonts;
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
                "rebuild the font atlas, and call style.ScaleAllSizes() on a reference RblxUIStyle structure.\n"
                "Using those settings here will give you poor quality results.");
            PushItemWidth(GetFontSize() * 8);
            DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", RblxUISliderFlags_AlwaysClamp); // Scale everything
            //static float window_scale = 1.0f;
            //if (DragFloat("window scale", &window_scale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", RblxUISliderFlags_AlwaysClamp)) // Scale only this window
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
            DragFloat("Circle Tessellation Max Error", &style.CircleTessellationMaxError , 0.005f, 0.10f, 5.0f, "%.2f", RblxUISliderFlags_AlwaysClamp);
            const bool show_samples = IsItemActive();
            if (show_samples)
                SetNextWindowPos(GetCursorScreenPos());
            if (show_samples && BeginTooltip())
            {
                TextUnformatted("(R = radius, N = approx number of segments)");
                Spacing();
                RblxDrawList* draw_list = GetWindowDrawList();
                const float min_widget_width = CalcTextSize("R: MMM\nN: MMM").x;
                for (int n = 0; n < 8; n++)
                {
                    const float RAD_MIN = 5.0f;
                    const float RAD_MAX = 70.0f;
                    const float rad = RAD_MIN + (RAD_MAX - RAD_MIN) * (float)n / (8.0f - 1.0f);

                    BeginGroup();

                    // N is not always exact here due to how PathArcTo() function work internally
                    Text("R: %.f\nN: %d", rad, draw_list->_CalcCircleAutoSegmentCount(rad));

                    const float canvas_width = RBLXUI_MAX(min_widget_width, rad * 2.0f);
                    const float offset_x     = floorf(canvas_width * 0.5f);
                    const float offset_y     = floorf(RAD_MAX);

                    const RblxVec2 p1 = GetCursorScreenPos();
                    draw_list->AddCircle(RblxVec2(p1.x + offset_x, p1.y + offset_y), rad, GetColorU32(RblxUICol_Text));
                    Dummy(RblxVec2(canvas_width, RAD_MAX * 2));

                    /*
                    const RblxVec2 p2 = GetCursorScreenPos();
                    draw_list->AddCircleFilled(RblxVec2(p2.x + offset_x, p2.y + offset_y), rad, GetColorU32(RblxUICol_Text));
                    Dummy(RblxVec2(canvas_width, RAD_MAX * 2));
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

// We omit the RblxUI:: prefix in this function, as we don't expect user to be copy and pasting this code.
void RblxUI::ShowUserGuide()
{
    RblxUIIO& io = GetIO();
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
// - BeginMenuBar() = menu-bar inside current window (which needs the RblxUIWindowFlags_MenuBar flag!)
// - BeginMainMenuBar() = helper to create menu-bar-sized window at the top of the main viewport + call BeginMenuBar() into it.
static void ShowExampleAppMainMenuBar()
{
    if (RblxUI::BeginMainMenuBar())
    {
        if (RblxUI::BeginMenu("File"))
        {
            RBLXUI_DEMO_MARKER("Menu/File");
            ShowExampleMenuFile();
            RblxUI::EndMenu();
        }
        if (RblxUI::BeginMenu("Edit"))
        {
            RBLXUI_DEMO_MARKER("Menu/Edit");
            if (RblxUI::MenuItem("Undo", "Ctrl+Z")) {}
            if (RblxUI::MenuItem("Redo", "Ctrl+Y", false, false)) {} // Disabled item
            RblxUI::Separator();
            if (RblxUI::MenuItem("Cut", "Ctrl+X")) {}
            if (RblxUI::MenuItem("Copy", "Ctrl+C")) {}
            if (RblxUI::MenuItem("Paste", "Ctrl+V")) {}
            RblxUI::EndMenu();
        }
        RblxUI::EndMainMenuBar();
    }
}

// Note that shortcuts are currently provided for display only
// (future version will add explicit flags to BeginMenu() to request processing shortcuts)
static void ShowExampleMenuFile()
{
    RBLXUI_DEMO_MARKER("Examples/Menu");
    RblxUI::MenuItem("(demo menu)", NULL, false, false);
    if (RblxUI::MenuItem("New")) {}
    if (RblxUI::MenuItem("Open", "Ctrl+O")) {}
    if (RblxUI::BeginMenu("Open Recent"))
    {
        RblxUI::MenuItem("fish_hat.c");
        RblxUI::MenuItem("fish_hat.inl");
        RblxUI::MenuItem("fish_hat.h");
        if (RblxUI::BeginMenu("More.."))
        {
            RblxUI::MenuItem("Hello");
            RblxUI::MenuItem("Sailor");
            if (RblxUI::BeginMenu("Recurse.."))
            {
                ShowExampleMenuFile();
                RblxUI::EndMenu();
            }
            RblxUI::EndMenu();
        }
        RblxUI::EndMenu();
    }
    if (RblxUI::MenuItem("Save", "Ctrl+S")) {}
    if (RblxUI::MenuItem("Save As..")) {}

    RblxUI::Separator();
    if (RblxUI::BeginMenu("Options"))
    {
        RBLXUI_DEMO_MARKER("Examples/Menu/Options");
        static bool enabled = true;
        RblxUI::MenuItem("Enabled", "", &enabled);
        RblxUI::BeginChild("child", RblxVec2(0, RblxUI::GetTextLineHeightWithSpacing() * 5.0f), RblxUIChildFlags_Borders);
        for (int i = 0; i < 10; i++)
            RblxUI::Text("Scrolling Text %d", i);
        RblxUI::EndChild();
        static float f = 0.5f;
        static int n = 0;
        RblxUI::SliderFloat("Value", &f, 0.0f, 1.0f);
        RblxUI::InputFloat("Input", &f, 0.1f);
        RblxUI::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        RblxUI::EndMenu();
    }

    if (RblxUI::BeginMenu("Colors"))
    {
        RBLXUI_DEMO_MARKER("Examples/Menu/Colors");
        float sz = RblxUI::GetTextLineHeight();
        for (int i = 0; i < RblxUICol_COUNT; i++)
        {
            const char* name = RblxUI::GetStyleColorName((RblxUICol)i);
            RblxVec2 p = RblxUI::GetCursorScreenPos();
            RblxUI::GetWindowDrawList()->AddRectFilled(p, RblxVec2(p.x + sz, p.y + sz), RblxUI::GetColorU32((RblxUICol)i));
            RblxUI::Dummy(RblxVec2(sz, sz));
            RblxUI::SameLine();
            RblxUI::MenuItem(name);
        }
        RblxUI::EndMenu();
    }

    // Here we demonstrate appending again to the "Options" menu (which we already created above)
    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
    // In a real code-base using it would make senses to use this feature from very different code locations.
    if (RblxUI::BeginMenu("Options")) // <-- Append!
    {
        RBLXUI_DEMO_MARKER("Examples/Menu/Append to an existing menu");
        static bool b = true;
        RblxUI::Checkbox("SomeOption", &b);
        RblxUI::EndMenu();
    }

    if (RblxUI::BeginMenu("Disabled", false)) // Disabled
    {
        RBLXUI_ASSERT(0);
    }
    if (RblxUI::MenuItem("Checked", NULL, true)) {}
    RblxUI::Separator();
    if (RblxUI::MenuItem("Quit", "Alt+F4")) {}
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple console window, with scrolling, filtering, completion and history.
// For the console example, we are using a more C++ like approach of declaring a class to hold both data and functions.
struct ExampleAppConsole
{
    char                  InputBuf[256];
    RblxVector<char*>       Items;
    RblxVector<const char*> Commands;
    RblxVector<char*>       History;
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    RblxUITextFilter       Filter;
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
        AddLog("Welcome to Dear RblxUI!");
    }
    ~ExampleAppConsole()
    {
        ClearLog();
        for (int i = 0; i < History.Size; i++)
            RblxUI::MemFree(History[i]);
    }

    // Portable helpers
    static int   Stricmp(const char* s1, const char* s2)         { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
    static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
    static char* Strdup(const char* s)                           { RBLXUI_ASSERT(s); size_t len = strlen(s) + 1; void* buf = RblxUI::MemAlloc(len); RBLXUI_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
    static void  Strtrim(char* s)                                { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

    void    ClearLog()
    {
        for (int i = 0; i < Items.Size; i++)
            RblxUI::MemFree(Items[i]);
        Items.clear();
    }

    void    AddLog(const char* fmt, ...) RBLXUI_FMTARGS(2)
    {
        // FIXME-OPT
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, RBLXUI_COUNTOF(buf), fmt, args);
        buf[RBLXUI_COUNTOF(buf)-1] = 0;
        va_end(args);
        Items.push_back(Strdup(buf));
    }

    void    Draw(const char* title, bool* p_open)
    {
        RblxUI::SetNextWindowSize(RblxVec2(520, 600), RblxUICond_FirstUseEver);
        if (!RblxUI::Begin(title, p_open))
        {
            RblxUI::End();
            return;
        }
        RBLXUI_DEMO_MARKER("Examples/Console");

        // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
        // So e.g. IsItemHovered() will return true when hovering the title bar.
        // Here we create a context menu only available from the title bar.
        if (RblxUI::BeginPopupContextItem())
        {
            if (RblxUI::MenuItem("Close Console"))
                *p_open = false;
            RblxUI::EndPopup();
        }

        RblxUI::TextWrapped(
            "This example implements a console with basic coloring, completion (TAB key) and history (Up/Down keys). A more elaborate "
            "implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
        RblxUI::TextWrapped("Enter 'HELP' for help.");

        // TODO: display items starting from the bottom

        if (RblxUI::SmallButton("Add Debug Text"))  { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); }
        RblxUI::SameLine();
        if (RblxUI::SmallButton("Add Debug Error")) { AddLog("[error] something went wrong"); }
        RblxUI::SameLine();
        if (RblxUI::SmallButton("Clear"))           { ClearLog(); }
        RblxUI::SameLine();
        bool copy_to_clipboard = RblxUI::SmallButton("Copy");
        //static float t = 0.0f; if (RblxUI::GetTime() - t > 0.02f) { t = RblxUI::GetTime(); AddLog("Spam %f", t); }

        RblxUI::Separator();

        // Options menu
        if (RblxUI::BeginPopup("Options"))
        {
            RblxUI::Checkbox("Auto-scroll", &AutoScroll);
            RblxUI::EndPopup();
        }

        // Options, Filter
        RblxUI::SetNextItemShortcut(RblxUIMod_Ctrl | RblxUIKey_O, RblxUIInputFlags_Tooltip);
        if (RblxUI::Button("Options"))
            RblxUI::OpenPopup("Options");
        RblxUI::SameLine();
        Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
        RblxUI::Separator();

        // Reserve enough left-over height for 1 separator + 1 input text
        RblxUIStyle& style = RblxUI::GetStyle();
        const float footer_height_to_reserve = style.SeparatorSize + style.ItemSpacing.y + RblxUI::GetFrameHeightWithSpacing();
        if (RblxUI::BeginChild("ScrollingRegion", RblxVec2(0, -footer_height_to_reserve), RblxUIChildFlags_NavFlattened, RblxUIWindowFlags_HorizontalScrollbar))
        {
            if (RblxUI::BeginPopupContextWindow())
            {
                if (RblxUI::Selectable("Clear")) ClearLog();
                RblxUI::EndPopup();
            }

            // Display every line as a separate entry so we can change their color or add custom widgets.
            // If you only want raw text you can use RblxUI::TextUnformatted(log.begin(), log.end());
            // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
            // to only process visible items. The clipper will automatically measure the height of your first item and then
            // "seek" to display only items in the visible area.
            // To use the clipper we can replace your standard loop:
            //      for (int i = 0; i < Items.Size; i++)
            //   With:
            //      RblxUIListClipper clipper;
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
            RblxUI::PushStyleVar(RblxUIStyleVar_ItemSpacing, RblxVec2(4, 1)); // Tighten spacing
            if (copy_to_clipboard)
                RblxUI::LogToClipboard();
            for (const char* item : Items)
            {
                if (!Filter.PassFilter(item))
                    continue;

                // Normally you would store more information in your item than just a string.
                // (e.g. make Items[] an array of structure, store color/type etc.)
                RblxVec4 color;
                bool has_color = false;
                if (strstr(item, "[error]")) { color = RblxVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
                else if (strncmp(item, "# ", 2) == 0) { color = RblxVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
                if (has_color)
                    RblxUI::PushStyleColor(RblxUICol_Text, color);
                RblxUI::TextUnformatted(item);
                if (has_color)
                    RblxUI::PopStyleColor();
            }
            if (copy_to_clipboard)
                RblxUI::LogFinish();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (ScrollToBottom || (AutoScroll && RblxUI::GetScrollY() >= RblxUI::GetScrollMaxY()))
                RblxUI::SetScrollHereY(1.0f);
            ScrollToBottom = false;

            RblxUI::PopStyleVar();
        }
        RblxUI::EndChild();
        RblxUI::Separator();

        // Command-line
        bool reclaim_focus = false;
        RblxUIInputTextFlags input_text_flags = RblxUIInputTextFlags_EnterReturnsTrue | RblxUIInputTextFlags_EscapeClearsAll | RblxUIInputTextFlags_CallbackCompletion | RblxUIInputTextFlags_CallbackHistory;
        if (RblxUI::InputText("Input", InputBuf, RBLXUI_COUNTOF(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
        {
            char* s = InputBuf;
            Strtrim(s);
            if (s[0])
                ExecCommand(s);
            strcpy(s, "");
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        RblxUI::SetItemDefaultFocus();
        if (reclaim_focus)
            RblxUI::SetKeyboardFocusHere(-1); // Auto focus previous widget

        RblxUI::End();
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
                RblxUI::MemFree(History[i]);
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
    static int TextEditCallbackStub(RblxUIInputTextCallbackData* data)
    {
        ExampleAppConsole* console = (ExampleAppConsole*)data->UserData;
        return console->TextEditCallback(data);
    }

    int     TextEditCallback(RblxUIInputTextCallbackData* data)
    {
        //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag)
        {
        case RblxUIInputTextFlags_CallbackCompletion:
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
                RblxVector<const char*> candidates;
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
        case RblxUIInputTextFlags_CallbackHistory:
            {
                // Example of HISTORY
                const int prev_history_pos = HistoryPos;
                if (data->EventKey == RblxUIKey_UpArrow)
                {
                    if (HistoryPos == -1)
                        HistoryPos = History.Size - 1;
                    else if (HistoryPos > 0)
                        HistoryPos--;
                }
                else if (data->EventKey == RblxUIKey_DownArrow)
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
    RblxFontAtlas* atlas = RblxUI::GetIO().Fonts;
    RblxTextureRef tex_ref = atlas->TexRef; // We don't have access to other textures in this demo!
    int tex_w = atlas->TexData->Width;
    int tex_h = atlas->TexData->Height;
    if (RblxUI::Begin("Example: Image Viewer", p_open))
    {
        static ExampleImageViewerData image_viewer;
        ExampleImageViewer_DrawOptions(&image_viewer);
        RblxVec2 canvas_size = RblxUI::GetContentRegionAvail();
        RblxVec2 canvas_min_size = RblxUI::IsWindowAppearing() ? RblxVec2(3.0f * tex_w, 4.0f * tex_h) : RblxVec2(1.0f, 1.0f);
        canvas_size = RblxVec2(RBLXUI_MAX(canvas_size.x, canvas_min_size.x), RBLXUI_MAX(canvas_size.y, canvas_min_size.y));
        ExampleImageViewer_DrawCanvas(&image_viewer, canvas_size, tex_ref, tex_w, tex_h);
    }
    RblxUI::End();
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
    RblxUITextBuffer     Buf;
    RblxUITextFilter     Filter;
    RblxVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
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

    void    AddLog(const char* fmt, ...) RBLXUI_FMTARGS(2)
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
        if (!RblxUI::Begin(title, p_open))
        {
            RblxUI::End();
            return;
        }

        // Options menu
        if (RblxUI::BeginPopup("Options"))
        {
            RblxUI::Checkbox("Auto-scroll", &AutoScroll);
            RblxUI::EndPopup();
        }

        // Main window
        if (RblxUI::Button("Options"))
            RblxUI::OpenPopup("Options");
        RblxUI::SameLine();
        bool clear = RblxUI::Button("Clear");
        RblxUI::SameLine();
        bool copy = RblxUI::Button("Copy");
        RblxUI::SameLine();
        Filter.Draw("Filter", -100.0f);

        RblxUI::Separator();

        if (RblxUI::BeginChild("scrolling", RblxVec2(0, 0), RblxUIChildFlags_None, RblxUIWindowFlags_HorizontalScrollbar))
        {
            if (clear)
                Clear();
            if (copy)
                RblxUI::LogToClipboard();

            RblxUI::PushStyleVar(RblxUIStyleVar_ItemSpacing, RblxVec2(0, 0));
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
                        RblxUI::TextUnformatted(line_start, line_end);
                }
            }
            else
            {
                // The simplest and easy way to display the entire buffer:
                //   RblxUI::TextUnformatted(buf_begin, buf_end);
                // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
                // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
                // within the visible area.
                // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
                // on your side is recommended. Using RblxUIListClipper requires
                // - A) random access into your data
                // - B) items all being the  same height,
                // both of which we can handle since we have an array pointing to the beginning of each line of text.
                // When using the filter (in the block of code above) we don't have random access into the data to display
                // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
                // it possible (and would be recommended if you want to search through tens of thousands of entries).
                RblxUIListClipper clipper;
                clipper.Begin(LineOffsets.Size);
                while (clipper.Step())
                {
                    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                    {
                        const char* line_start = buf + LineOffsets[line_no];
                        const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                        RblxUI::TextUnformatted(line_start, line_end);
                    }
                }
                clipper.End();
            }
            RblxUI::PopStyleVar();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (AutoScroll && RblxUI::GetScrollY() >= RblxUI::GetScrollMaxY())
                RblxUI::SetScrollHereY(1.0f);
        }
        RblxUI::EndChild();
        RblxUI::End();
    }
};

// Demonstrate creating a simple log window with basic filtering.
static void ShowExampleAppLog(bool* p_open)
{
    static ExampleAppLog log;

    // For the demo: add a debug button _BEFORE_ the normal log window contents
    // We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
    // Most of the contents of the window will be added by the log.Draw() call.
    RblxUI::SetNextWindowSize(RblxVec2(500, 400), RblxUICond_FirstUseEver);
    RblxUI::Begin("Example: Log", p_open);
    RBLXUI_DEMO_MARKER("Examples/Log");
    if (RblxUI::SmallButton("[Debug] Add 5 entries"))
    {
        static int counter = 0;
        const char* categories[3] = { "info", "warn", "error" };
        const char* words[] = { "Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", "Pauciloquent" };
        for (int n = 0; n < 5; n++)
        {
            const char* category = categories[counter % RBLXUI_COUNTOF(categories)];
            const char* word = words[counter % RBLXUI_COUNTOF(words)];
            log.AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
                RblxUI::GetFrameCount(), category, RblxUI::GetTime(), word);
            counter++;
        }
    }
    RblxUI::End();

    // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
    log.Draw("Example: Log", p_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
//-----------------------------------------------------------------------------

// Demonstrate create a window with multiple child windows.
static void ShowExampleAppLayout(bool* p_open)
{
    RblxUI::SetNextWindowSize(RblxVec2(500, 440), RblxUICond_FirstUseEver);
    if (RblxUI::Begin("Example: Simple layout", p_open, RblxUIWindowFlags_MenuBar))
    {
        RBLXUI_DEMO_MARKER("Examples/Simple layout");
        if (RblxUI::BeginMenuBar())
        {
            if (RblxUI::BeginMenu("File"))
            {
                if (RblxUI::MenuItem("Close", "Ctrl+W")) { *p_open = false; }
                RblxUI::EndMenu();
            }
            RblxUI::EndMenuBar();
        }

        // Left
        static int selected = 0;
        {
            RblxUI::BeginChild("left pane", RblxVec2(150, 0), RblxUIChildFlags_Borders | RblxUIChildFlags_ResizeX);
            for (int i = 0; i < 100; i++)
            {
                char label[128];
                sprintf(label, "MyObject %d", i);
                if (RblxUI::Selectable(label, selected == i, RblxUISelectableFlags_SelectOnNav))
                    selected = i;
            }
            RblxUI::EndChild();
        }
        RblxUI::SameLine();

        // Right
        {
            RblxUI::BeginGroup();
            RblxUI::BeginChild("item view", RblxVec2(0, -RblxUI::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
            RblxUI::Text("MyObject: %d", selected);
            RblxUI::Separator();
            if (RblxUI::BeginTabBar("##Tabs", RblxUITabBarFlags_None))
            {
                if (RblxUI::BeginTabItem("Description"))
                {
                    RblxUI::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
                    RblxUI::EndTabItem();
                }
                if (RblxUI::BeginTabItem("Details"))
                {
                    RblxUI::Text("ID: 0123456789");
                    RblxUI::EndTabItem();
                }
                RblxUI::EndTabBar();
            }
            RblxUI::EndChild();
            if (RblxUI::Button("Revert")) {}
            RblxUI::SameLine();
            if (RblxUI::Button("Save")) {}
            RblxUI::EndGroup();
        }
    }
    RblxUI::End();
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
    RblxUITextFilter     Filter;
    ExampleTreeNode*    SelectedNode = NULL;
    bool                UseClipper = false;

    void Draw(ExampleTreeNode* root_node)
    {
        RBLXUI_DEMO_MARKER("Examples/Property editor");

        // Left side: draw tree
        // - Currently using a table to benefit from RowBg feature
        // - Our tree node are all of equal height, facilitating the use of a clipper.
        if (RblxUI::BeginChild("##tree", RblxVec2(300, 0), RblxUIChildFlags_ResizeX | RblxUIChildFlags_Borders | RblxUIChildFlags_NavFlattened))
        {
            RblxUI::PushItemFlag(RblxUIItemFlags_NoNavDefaultFocus, true);
            RblxUI::Checkbox("Use Clipper", &UseClipper);
            RblxUI::SameLine();
            RblxUI::Text("(%d root nodes)", root_node->Childs.Size);
            RblxUI::SetNextItemWidth(-FLT_MIN);
            RblxUI::SetNextItemShortcut(RblxUIMod_Ctrl | RblxUIKey_F, RblxUIInputFlags_Tooltip);
            if (RblxUI::InputTextWithHint("##Filter", "incl,-excl", Filter.InputBuf, RBLXUI_COUNTOF(Filter.InputBuf), RblxUIInputTextFlags_EscapeClearsAll))
                Filter.Build();
            RblxUI::PopItemFlag();

            if (RblxUI::BeginTable("##list", 1, RblxUITableFlags_RowBg))
            {
                if (UseClipper)
                    DrawClippedTree(root_node);
                else
                    DrawTree(root_node);
                RblxUI::EndTable();
            }
        }
        RblxUI::EndChild();

        // Right side: draw properties
        RblxUI::SameLine();

        RblxUI::BeginGroup(); // Lock X position
        if (ExampleTreeNode* node = SelectedNode)
        {
            RblxUI::Text("%s", node->Name);
            RblxUI::TextDisabled("UID: 0x%08X", node->UID);
            RblxUI::Separator();
            if (RblxUI::BeginTable("##properties", 2, RblxUITableFlags_Resizable | RblxUITableFlags_ScrollY))
            {
                // Push object ID after we entered the table, so table is shared for all objects
                RblxUI::PushID((int)node->UID);
                RblxUI::TableSetupColumn("", RblxUITableColumnFlags_WidthFixed);
                RblxUI::TableSetupColumn("", RblxUITableColumnFlags_WidthStretch, 2.0f); // Default twice larger
                if (node->HasData)
                {
                    // In a typical application, the structure description would be derived from a data-driven system.
                    // - We try to mimic this with our ExampleMemberInfo structure and the ExampleTreeNodeMemberInfos[] array.
                    // - Limits and some details are hard-coded to simplify the demo.
                    for (const ExampleMemberInfo& field_desc : ExampleTreeNodeMemberInfos)
                    {
                        RblxUI::TableNextRow();
                        RblxUI::PushID(field_desc.Name);
                        RblxUI::TableNextColumn();
                        RblxUI::AlignTextToFramePadding();
                        RblxUI::TextUnformatted(field_desc.Name);
                        RblxUI::TableNextColumn();
                        void* field_ptr = (void*)(((unsigned char*)node) + field_desc.Offset);
                        switch (field_desc.DataType)
                        {
                        case RblxUIDataType_Bool:
                        {
                            RBLXUI_ASSERT(field_desc.DataCount == 1);
                            RblxUI::Checkbox("##Editor", (bool*)field_ptr);
                            break;
                        }
                        case RblxUIDataType_S32:
                        {
                            int v_min = INT_MIN, v_max = INT_MAX;
                            RblxUI::SetNextItemWidth(-FLT_MIN);
                            RblxUI::DragScalarN("##Editor", field_desc.DataType, field_ptr, field_desc.DataCount, 1.0f, &v_min, &v_max);
                            break;
                        }
                        case RblxUIDataType_Float:
                        {
                            float v_min = 0.0f, v_max = 1.0f;
                            RblxUI::SetNextItemWidth(-FLT_MIN);
                            RblxUI::SliderScalarN("##Editor", field_desc.DataType, field_ptr, field_desc.DataCount, &v_min, &v_max);
                            break;
                        }
                        case RblxUIDataType_String:
                        {
                            RblxUI::InputText("##Editor", reinterpret_cast<char*>(field_ptr), 28);
                            break;
                        }
                        }
                        RblxUI::PopID();
                    }
                }
                RblxUI::PopID();
                RblxUI::EndTable();
            }
        }
        RblxUI::EndGroup();
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
                RblxUI::TreePop();
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
    // See https://github.com/ocornut/rblxui/issues/3823 for discussions about this.
    void DrawClippedTree(ExampleTreeNode* root_node)
    {
        ExampleTreeNode* node = root_node->Childs[0]; // First node
        RblxUIListClipper clipper;
        clipper.Begin(INT_MAX);
        while (clipper.Step())
            while (clipper.UserIndex < clipper.DisplayEnd && node != NULL)
                node = DrawClippedTreeNodeAndAdvanceToNext(&clipper, node);

        // Keep going to count nodes and submit final count so we have a reliable scrollbar.
        // - One could consider caching this value and only refreshing it occasionally e.g. window is focused and an action occurs.
        // - Incorrect but cheap approximation would be to use 'clipper_current_idx = RBLXUI_MAX(clipper_current_idx, root_node->Childs.Size)' instead.
        // - If either of those is implemented, the general cost will approach zero when scrolling is at the top of the tree.
        while (node != NULL)
            node = DrawClippedTreeNodeAndAdvanceToNext(&clipper, node);
        //clipper.UserIndex = RBLXUI_MAX(clipper.UserIndex, root_node->Childs.Size); // <-- Cheap approximation instead of while() loop above.
        clipper.SeekCursorForItem(clipper.UserIndex);
    }

    ExampleTreeNode* DrawClippedTreeNodeAndAdvanceToNext(RblxUIListClipper* clipper, ExampleTreeNode* node)
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
                is_open = (node->Childs.Size > 0 && RblxUI::TreeNodeGetOpen((RblxUIID)node->UID));
                if (is_open)
                    RblxUI::TreePush(node->Name);
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
            RblxUI::TreePop();
        }
        return NULL;
    }

    // To support node with same name we incorporate node->UID into the item ID.
    // (this would more naturally be done using PushID(node->UID) + TreeNodeEx(node->Name, tree_flags),
    //   but it would require in DrawClippedTreeNodeAndAdvanceToNext() to add PushID() before TreePush(), and PopID() after TreePop(),
    //   so instead we use TreeNodeEx(node->UID, tree_flags, "%s", node->Name) here)
    bool DrawTreeNode(ExampleTreeNode* node)
    {
        RblxUI::TableNextRow();
        RblxUI::TableNextColumn();
        RblxUITreeNodeFlags tree_flags = RblxUITreeNodeFlags_None;
        tree_flags |= RblxUITreeNodeFlags_OpenOnArrow | RblxUITreeNodeFlags_OpenOnDoubleClick; // Standard opening mode as we are likely to want to add selection afterwards
        tree_flags |= RblxUITreeNodeFlags_NavLeftJumpsToParent;  // Left arrow support
        tree_flags |= RblxUITreeNodeFlags_SpanFullWidth;         // Span full width for easier mouse reach
        tree_flags |= RblxUITreeNodeFlags_DrawLinesToNodes;      // Always draw hierarchy outlines
        if (node == SelectedNode)
            tree_flags |= RblxUITreeNodeFlags_Selected;          // Draw selection highlight
        if (node->Childs.Size == 0)
            tree_flags |= RblxUITreeNodeFlags_Leaf | RblxUITreeNodeFlags_Bullet | RblxUITreeNodeFlags_NoTreePushOnOpen; // Use _NoTreePushOnOpen + set is_open=false to avoid unnecessarily push/pop on leaves.
        if (node->DataMyBool == false)
            RblxUI::PushStyleColor(RblxUICol_Text, RblxUI::GetStyle().Colors[RblxUICol_TextDisabled]);
        RblxUI::SetNextItemStorageID((RblxUIID)node->UID);        // Use node->UID as storage id
        bool is_open = RblxUI::TreeNodeEx((void*)(intptr_t)node->UID, tree_flags, "%s", node->Name);
        if (node->Childs.Size == 0)
            is_open = false;
        if (node->DataMyBool == false)
            RblxUI::PopStyleColor();
        if (RblxUI::IsItemFocused())
            SelectedNode = node;
        return is_open;
    }
};

// Demonstrate creating a simple property editor.
static void ShowExampleAppPropertyEditor(bool* p_open, RblxUIDemoWindowData* demo_data)
{
    RblxUI::SetNextWindowSize(RblxVec2(430, 450), RblxUICond_FirstUseEver);
    if (!RblxUI::Begin("Example: Property editor", p_open))
    {
        RblxUI::End();
        return;
    }

    RBLXUI_DEMO_MARKER("Examples/Property Editor");
    static ExampleAppPropertyEditor property_editor;
    if (demo_data->DemoTree == NULL)
        demo_data->DemoTree = ExampleTree_CreateDemoTree();
    property_editor.Draw(demo_data->DemoTree);

    RblxUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Long Text / ShowExampleAppLongText()
//-----------------------------------------------------------------------------

// Demonstrate/test rendering huge amount of text, and the incidence of clipping.
static void ShowExampleAppLongText(bool* p_open)
{
    RblxUI::SetNextWindowSize(RblxVec2(520, 600), RblxUICond_FirstUseEver);
    if (!RblxUI::Begin("Example: Long text display", p_open))
    {
        RblxUI::End();
        return;
    }
    RBLXUI_DEMO_MARKER("Examples/Long text display");

    static int test_type = 0;
    static RblxUITextBuffer log;
    static int lines = 0;
    RblxUI::Text("Printing unusually long amount of text.");
    RblxUI::Combo("Test type", &test_type,
        "Single call to TextUnformatted()\0"
        "Multiple calls to Text(), clipped\0"
        "Multiple calls to Text(), not clipped (slow)\0");
    RblxUI::Text("Buffer contents: %d lines, %d bytes", lines, log.size());
    if (RblxUI::Button("Clear")) { log.clear(); lines = 0; }
    RblxUI::SameLine();
    if (RblxUI::Button("Add 1000 lines"))
    {
        for (int i = 0; i < 1000; i++)
            log.appendf("%i The quick brown fox jumps over the lazy dog\n", lines + i);
        lines += 1000;
    }
    RblxUI::BeginChild("Log");
    switch (test_type)
    {
    case 0:
        // Single call to TextUnformatted() with a big buffer
        RblxUI::TextUnformatted(log.begin(), log.end());
        break;
    case 1:
        {
            // Multiple calls to Text(), manually coarsely clipped - demonstrate how to use the RblxUIListClipper helper.
            RblxUI::PushStyleVar(RblxUIStyleVar_ItemSpacing, RblxVec2(0, 0));
            RblxUIListClipper clipper;
            clipper.Begin(lines);
            while (clipper.Step())
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                    RblxUI::Text("%i The quick brown fox jumps over the lazy dog", i);
            RblxUI::PopStyleVar();
            break;
        }
    case 2:
        // Multiple calls to Text(), not clipped (slow)
        RblxUI::PushStyleVar(RblxUIStyleVar_ItemSpacing, RblxVec2(0, 0));
        for (int i = 0; i < lines; i++)
            RblxUI::Text("%i The quick brown fox jumps over the lazy dog", i);
        RblxUI::PopStyleVar();
        break;
    }
    RblxUI::EndChild();
    RblxUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
//-----------------------------------------------------------------------------

// Demonstrate creating a window which gets auto-resized according to its content.
static void ShowExampleAppAutoResize(bool* p_open)
{
    if (!RblxUI::Begin("Example: Auto-resizing window", p_open, RblxUIWindowFlags_AlwaysAutoResize))
    {
        RblxUI::End();
        return;
    }
    RBLXUI_DEMO_MARKER("Examples/Auto-resizing window");

    static int lines = 10;
    RblxUI::TextUnformatted(
        "Window will resize every-frame to the size of its content.\n"
        "Note that you probably don't want to query the window size to\n"
        "output your content because that would create a feedback loop.");
    RblxUI::SliderInt("Number of lines", &lines, 1, 20);
    for (int i = 0; i < lines; i++)
        RblxUI::Text("%*sThis is line %d", i * 4, "", i); // Pad with space to extend size horizontally
    RblxUI::End();
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
        static void AspectRatio(RblxUISizeCallbackData* data)
        {
            float aspect_ratio = *(float*)data->UserData;
            data->DesiredSize.y = (float)(int)(data->DesiredSize.x / aspect_ratio);
        }
        static void Square(RblxUISizeCallbackData* data)
        {
            data->DesiredSize.x = data->DesiredSize.y = RBLXUI_MAX(data->DesiredSize.x, data->DesiredSize.y);
        }
        static void Step(RblxUISizeCallbackData* data)
        {
            float step = *(float*)data->UserData;
            data->DesiredSize = RblxVec2((int)(data->DesiredSize.x / step + 0.5f) * step, (int)(data->DesiredSize.y / step + 0.5f) * step);
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
    if (type == 0) RblxUI::SetNextWindowSizeConstraints(RblxVec2(100, 100), RblxVec2(500, 500));         // Between 100x100 and 500x500
    if (type == 1) RblxUI::SetNextWindowSizeConstraints(RblxVec2(100, 100), RblxVec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100
    if (type == 2) RblxUI::SetNextWindowSizeConstraints(RblxVec2(-1, 0),    RblxVec2(-1, FLT_MAX));      // Resize vertical + lock current width
    if (type == 3) RblxUI::SetNextWindowSizeConstraints(RblxVec2(0, -1),    RblxVec2(FLT_MAX, -1));      // Resize horizontal + lock current height
    if (type == 4) RblxUI::SetNextWindowSizeConstraints(RblxVec2(400, -1),  RblxVec2(500, -1));          // Width Between and 400 and 500
    if (type == 5) RblxUI::SetNextWindowSizeConstraints(RblxVec2(-1, 400),  RblxVec2(-1, FLT_MAX));      // Height at least 400
    if (type == 6) RblxUI::SetNextWindowSizeConstraints(RblxVec2(0, 0),     RblxVec2(FLT_MAX, FLT_MAX), CustomConstraints::AspectRatio, (void*)&aspect_ratio);   // Aspect ratio
    if (type == 7) RblxUI::SetNextWindowSizeConstraints(RblxVec2(0, 0),     RblxVec2(FLT_MAX, FLT_MAX), CustomConstraints::Square);                              // Always Square
    if (type == 8) RblxUI::SetNextWindowSizeConstraints(RblxVec2(0, 0),     RblxVec2(FLT_MAX, FLT_MAX), CustomConstraints::Step, (void*)&fixed_step);            // Fixed Step

    // Submit window
    if (!window_padding)
        RblxUI::PushStyleVar(RblxUIStyleVar_WindowPadding, RblxVec2(0.0f, 0.0f));
    const RblxUIWindowFlags window_flags = auto_resize ? RblxUIWindowFlags_AlwaysAutoResize : 0;
    const bool window_open = RblxUI::Begin("Example: Constrained Resize", p_open, window_flags);
    if (!window_padding)
        RblxUI::PopStyleVar();
    RBLXUI_DEMO_MARKER("Examples/Constrained Resizing window");
    if (window_open)
    {
        if (RblxUI::GetIO().KeyShift)
        {
            // Display a dummy viewport (in your real app you would likely use ImageButton() to display a texture)
            RblxVec2 avail_size = RblxUI::GetContentRegionAvail();
            RblxVec2 pos = RblxUI::GetCursorScreenPos();
            RblxUI::ColorButton("viewport", RblxVec4(0.5f, 0.2f, 0.5f, 1.0f), RblxUIColorEditFlags_NoTooltip | RblxUIColorEditFlags_NoDragDrop, avail_size);
            RblxUI::SetCursorScreenPos(RblxVec2(pos.x + 10, pos.y + 10));
            RblxUI::Text("%.2f x %.2f", avail_size.x, avail_size.y);
        }
        else
        {
            RblxUI::Text("(Hold Shift to display a dummy viewport)");
            if (RblxUI::IsWindowDocked())
                RblxUI::Text("Warning: Sizing Constraints won't work if the window is docked!");
            if (RblxUI::Button("Set 200x200")) { RblxUI::SetWindowSize(RblxVec2(200, 200)); } RblxUI::SameLine();
            if (RblxUI::Button("Set 500x500")) { RblxUI::SetWindowSize(RblxVec2(500, 500)); } RblxUI::SameLine();
            if (RblxUI::Button("Set 800x200")) { RblxUI::SetWindowSize(RblxVec2(800, 200)); }
            RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 20);
            RblxUI::Combo("Constraint", &type, test_desc, RBLXUI_COUNTOF(test_desc));
            RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 20);
            RblxUI::DragInt("Lines", &display_lines, 0.2f, 1, 100);
            RblxUI::Checkbox("Auto-resize", &auto_resize);
            RblxUI::Checkbox("Window padding", &window_padding);
            for (int i = 0; i < display_lines; i++)
                RblxUI::Text("%*sHello, sailor! Making this line long enough for the example.", i * 4, "");
        }
    }
    RblxUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple overlay / ShowExampleAppSimpleOverlay()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple static window with no decoration
// + a context-menu to choose which corner of the screen to use.
static void ShowExampleAppSimpleOverlay(bool* p_open)
{
    static int location = 0;
    RblxUIIO& io = RblxUI::GetIO();
    RblxUIWindowFlags window_flags = RblxUIWindowFlags_NoDecoration | RblxUIWindowFlags_NoDocking | RblxUIWindowFlags_AlwaysAutoResize | RblxUIWindowFlags_NoSavedSettings | RblxUIWindowFlags_NoFocusOnAppearing | RblxUIWindowFlags_NoNav;
    if (location >= 0)
    {
        const float PAD = 10.0f;
        const RblxUIViewport* viewport = RblxUI::GetMainViewport();
        RblxVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        RblxVec2 work_size = viewport->WorkSize;
        RblxVec2 window_pos, window_pos_pivot;
        window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
        RblxUI::SetNextWindowPos(window_pos, RblxUICond_Always, window_pos_pivot);
        RblxUI::SetNextWindowViewport(viewport->ID);
        window_flags |= RblxUIWindowFlags_NoMove;
    }
    else if (location == -2)
    {
        // Center window
        RblxUI::SetNextWindowPos(RblxUI::GetMainViewport()->GetCenter(), RblxUICond_Always, RblxVec2(0.5f, 0.5f));
        window_flags |= RblxUIWindowFlags_NoMove;
    }
    RblxUI::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (RblxUI::Begin("Example: Simple overlay", p_open, window_flags))
    {
        RBLXUI_DEMO_MARKER("Examples/Simple overlay"); // Scroll up to the beginning of this function to see overlay flags
        RblxUI::Text("Simple overlay\n" "(right-click to change position)");
        RblxUI::Separator();
        if (RblxUI::IsMousePosValid())
            RblxUI::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        else
            RblxUI::Text("Mouse Position: <invalid>");
        if (RblxUI::BeginPopupContextWindow())
        {
            if (RblxUI::MenuItem("Custom",       NULL, location == -1)) location = -1;
            if (RblxUI::MenuItem("Center",       NULL, location == -2)) location = -2;
            if (RblxUI::MenuItem("Top-left",     NULL, location == 0)) location = 0;
            if (RblxUI::MenuItem("Top-right",    NULL, location == 1)) location = 1;
            if (RblxUI::MenuItem("Bottom-left",  NULL, location == 2)) location = 2;
            if (RblxUI::MenuItem("Bottom-right", NULL, location == 3)) location = 3;
            if (p_open && RblxUI::MenuItem("Close")) *p_open = false;
            RblxUI::EndPopup();
        }
    }
    RblxUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Fullscreen window / ShowExampleAppFullscreen()
//-----------------------------------------------------------------------------

// Demonstrate creating a window covering the entire screen/viewport
static void ShowExampleAppFullscreen(bool* p_open)
{
    static bool use_work_area = true;
    static RblxUIWindowFlags flags = RblxUIWindowFlags_NoDecoration | RblxUIWindowFlags_NoMove | RblxUIWindowFlags_NoSavedSettings;

    // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
    // Based on your use case you may want one or the other.
    const RblxUIViewport* viewport = RblxUI::GetMainViewport();
    RblxUI::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    RblxUI::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

    if (RblxUI::Begin("Example: Fullscreen window", p_open, flags))
    {
        RBLXUI_DEMO_MARKER("Examples/Fullscreen window");
        RblxUI::Checkbox("Use work area instead of main area", &use_work_area);
        RblxUI::SameLine();
        HelpMarker("Main Area = entire viewport,\nWork Area = entire viewport minus sections used by the main menu bars, task bars etc.\n\nEnable the main-menu bar in Examples menu to see the difference.");

        RblxUI::CheckboxFlags("RblxUIWindowFlags_NoBackground", &flags, RblxUIWindowFlags_NoBackground);
        RblxUI::CheckboxFlags("RblxUIWindowFlags_NoDecoration", &flags, RblxUIWindowFlags_NoDecoration);
        RblxUI::Indent();
        RblxUI::CheckboxFlags("RblxUIWindowFlags_NoTitleBar", &flags, RblxUIWindowFlags_NoTitleBar);
        RblxUI::CheckboxFlags("RblxUIWindowFlags_NoCollapse", &flags, RblxUIWindowFlags_NoCollapse);
        RblxUI::CheckboxFlags("RblxUIWindowFlags_NoScrollbar", &flags, RblxUIWindowFlags_NoScrollbar);
        RblxUI::Unindent();

        if (p_open && RblxUI::Button("Close this window"))
            *p_open = false;
    }
    RblxUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Manipulating Window Titles / ShowExampleAppWindowTitles()
//-----------------------------------------------------------------------------

// Demonstrate the use of "##" and "###" in identifiers to manipulate ID generation.
// This applies to all regular items as well.
// Read FAQ section "How can I have multiple widgets with the same label?" for details.
static void ShowExampleAppWindowTitles(bool*)
{
    const RblxUIViewport* viewport = RblxUI::GetMainViewport();
    const RblxVec2 base_pos = viewport->Pos;

    // By default, Windows are uniquely identified by their title.
    // You can use the "##" and "###" markers to manipulate the display/ID.

    // Using "##" to display same title but have unique identifier.
    RblxUI::SetNextWindowPos(RblxVec2(base_pos.x + 100, base_pos.y + 100), RblxUICond_FirstUseEver);
    RblxUI::Begin("Same title as another window##1");
    RBLXUI_DEMO_MARKER("Examples/Manipulating window titles##1");
    RblxUI::Text("This is window 1.\nMy title is the same as window 2, but my identifier is unique.");
    RblxUI::End();

    RblxUI::SetNextWindowPos(RblxVec2(base_pos.x + 100, base_pos.y + 200), RblxUICond_FirstUseEver);
    RblxUI::Begin("Same title as another window##2");
    RBLXUI_DEMO_MARKER("Examples/Manipulating window titles##2");
    RblxUI::Text("This is window 2.\nMy title is the same as window 1, but my identifier is unique.");
    RblxUI::End();

    // Using "###" to display a changing title but keep a static identifier "AnimatedTitle"
    char buf[128];
    sprintf(buf, "Animated title %c %d###AnimatedTitle", "|/-\\"[(int)(RblxUI::GetTime() / 0.25f) & 3], RblxUI::GetFrameCount());
    RblxUI::SetNextWindowPos(RblxVec2(base_pos.x + 100, base_pos.y + 300), RblxUICond_FirstUseEver);
    RblxUI::Begin(buf);
    RBLXUI_DEMO_MARKER("Examples/Manipulating window titles##3");
    RblxUI::Text("This window has a changing title.");
    RblxUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Custom Rendering using RblxDrawList API / ShowExampleAppCustomRendering()
//-----------------------------------------------------------------------------

// Add a |_| looking shape
static void PathConcaveShape(RblxDrawList* draw_list, float x, float y, float sz)
{
    const RblxVec2 pos_norms[] = { { 0.0f, 0.0f }, { 0.3f, 0.0f }, { 0.3f, 0.7f }, { 0.7f, 0.7f }, { 0.7f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
    for (const RblxVec2& p : pos_norms)
        draw_list->PathLineTo(RblxVec2(x + 0.5f + (int)(sz * p.x), y + 0.5f + (int)(sz * p.y)));
}

// Demonstrate using the low-level RblxDrawList to draw custom shapes.
static void ShowExampleAppCustomRendering(bool* p_open)
{
    if (!RblxUI::Begin("Example: Custom rendering", p_open))
    {
        RblxUI::End();
        return;
    }
    RBLXUI_DEMO_MARKER("Examples/Custom rendering");

    // Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of
    // overloaded operators, etc. Define RBLXUI_VEC2_CLASS_EXTRA in rblxuiconfig.h to create implicit conversions between your
    // types and RblxVec2/RblxVec4. Dear RblxUI defines overloaded operators but they are internal to rblxui.cpp and not
    // exposed outside (to avoid messing with your types) In this example we are not using the maths operators!

    if (RblxUI::BeginTabBar("##TabBar"))
    {
        if (RblxUI::BeginTabItem("Primitives"))
        {
            RBLXUI_DEMO_MARKER("Examples/Custom rendering/Primitives");
            RblxUI::PushItemWidth(-RblxUI::GetFontSize() * 15);
            RblxDrawList* draw_list = RblxUI::GetWindowDrawList();

            // Draw gradients
            // (note that those are currently exacerbating our sRGB/Linear issues)
            // Calling RblxUI::GetColorU32() multiplies the given colors by the current Style Alpha, but you may pass the RBLXUI_COL32() directly as well..
            RblxUI::Text("Gradients");
            RblxVec2 gradient_size = RblxVec2(RblxUI::CalcItemWidth(), RblxUI::GetFrameHeight());
            {
                RblxVec2 p0 = RblxUI::GetCursorScreenPos();
                RblxVec2 p1 = RblxVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
                RblxU32 col_a = RblxUI::GetColorU32(RBLXUI_COL32(0, 0, 0, 255));
                RblxU32 col_b = RblxUI::GetColorU32(RBLXUI_COL32(255, 255, 255, 255));
                draw_list->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
                RblxUI::InvisibleButton("##gradient1", gradient_size);
            }
            {
                RblxVec2 p0 = RblxUI::GetCursorScreenPos();
                RblxVec2 p1 = RblxVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
                RblxU32 col_a = RblxUI::GetColorU32(RBLXUI_COL32(0, 255, 0, 255));
                RblxU32 col_b = RblxUI::GetColorU32(RBLXUI_COL32(255, 0, 0, 255));
                draw_list->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
                RblxUI::InvisibleButton("##gradient2", gradient_size);
            }

            // Draw a bunch of primitives
            RblxUI::Text("All primitives");
            static float sz = 36.0f;
            static float thickness = 3.0f;
            static int ngon_sides = 6;
            static bool circle_segments_override = false;
            static int circle_segments_override_v = 12;
            static bool curve_segments_override = false;
            static int curve_segments_override_v = 8;
            static RblxVec4 colf = RblxVec4(1.0f, 1.0f, 0.4f, 1.0f);
            RblxUI::DragFloat("Size", &sz, 0.2f, 2.0f, 100.0f, "%.0f");
            RblxUI::DragFloat("Thickness", &thickness, 0.05f, 1.0f, 8.0f, "%.02f");
            RblxUI::SliderInt("N-gon sides", &ngon_sides, 3, 12);
            RblxUI::Checkbox("##circlesegmentoverride", &circle_segments_override);
            RblxUI::SameLine(0.0f, RblxUI::GetStyle().ItemInnerSpacing.x);
            circle_segments_override |= RblxUI::SliderInt("Circle segments override", &circle_segments_override_v, 3, 40);
            RblxUI::Checkbox("##curvessegmentoverride", &curve_segments_override);
            RblxUI::SameLine(0.0f, RblxUI::GetStyle().ItemInnerSpacing.x);
            curve_segments_override |= RblxUI::SliderInt("Curves segments override", &curve_segments_override_v, 3, 40);
            RblxUI::ColorEdit4("Color", &colf.x);

            const RblxVec2 p = RblxUI::GetCursorScreenPos();
            const RblxU32 col = RblxColor(colf);
            const float spacing = 10.0f;
            const RblxDrawFlags corners_tl_br = RblxDrawFlags_RoundCornersTopLeft | RblxDrawFlags_RoundCornersBottomRight;
            const float rounding = sz / 5.0f;
            const int circle_segments = circle_segments_override ? circle_segments_override_v : 0;
            const int curve_segments = curve_segments_override ? curve_segments_override_v : 0;
            const RblxVec2 cp3[3] = { RblxVec2(0.0f, sz * 0.6f), RblxVec2(sz * 0.5f, -sz * 0.4f), RblxVec2(sz, sz) }; // Control points for curves
            const RblxVec2 cp4[4] = { RblxVec2(0.0f, 0.0f), RblxVec2(sz * 1.3f, sz * 0.3f), RblxVec2(sz - sz * 1.3f, sz - sz * 0.3f), RblxVec2(sz, sz) };

            float x = p.x + 4.0f;
            float y = p.y + 4.0f;
            for (int n = 0; n < 2; n++)
            {
                // First line uses a thickness of 1.0f, second line uses the configurable thickness
                float th = (n == 0) ? 1.0f : thickness;
                draw_list->AddNgon(RblxVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, col, ngon_sides, th);                 x += sz + spacing;  // N-gon
                draw_list->AddCircle(RblxVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, col, circle_segments, th);          x += sz + spacing;  // Circle
                draw_list->AddEllipse(RblxVec2(x + sz*0.5f, y + sz*0.5f), RblxVec2(sz*0.5f, sz*0.3f), col, -0.3f, circle_segments, th); x += sz + spacing;  // Ellipse
                draw_list->AddRect(RblxVec2(x, y), RblxVec2(x + sz, y + sz), col, 0.0f, th);                            x += sz + spacing;  // Square
                draw_list->AddRect(RblxVec2(x, y), RblxVec2(x + sz, y + sz), col, rounding, th);                        x += sz + spacing;  // Square with all rounded corners
                draw_list->AddRect(RblxVec2(x, y), RblxVec2(x + sz, y + sz), col, rounding, th, corners_tl_br);         x += sz + spacing;  // Square with two rounded corners
                draw_list->AddTriangle(RblxVec2(x+sz*0.5f,y), RblxVec2(x+sz, y+sz-0.5f), RblxVec2(x, y+sz-0.5f), col, th);x += sz + spacing;  // Triangle
                //draw_list->AddTriangle(RblxVec2(x+sz*0.2f,y), RblxVec2(x, y+sz-0.5f), RblxVec2(x+sz*0.4f, y+sz-0.5f), col, th);x+= sz*0.4f + spacing; // Thin triangle
                PathConcaveShape(draw_list, x, y, sz); draw_list->PathStroke(col, th, RblxDrawFlags_Closed);          x += sz + spacing;  // Concave Shape
                //draw_list->AddPolyline(concave_shape, RBLXUI_COUNTOF(concave_shape), col, RblxDrawFlags_Closed, th);
                draw_list->AddLineH(x, x + sz, y, col, th);                                                         x += sz + spacing;  // Horizontal line (note: drawing a filled rectangle will be faster!)
                draw_list->AddLineV(x, y, y + sz, col, th);                                                         x += spacing;       // Vertical line (note: drawing a filled rectangle will be faster!)
                draw_list->AddLine(RblxVec2(x, y), RblxVec2(x + sz, y + sz), col, th);                                  x += sz + spacing;  // Diagonal line

                // Path
                draw_list->PathArcTo(RblxVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, 3.141592f, 3.141592f * -0.5f);
                draw_list->PathStroke(col, th);
                x += sz + spacing;

                // Quadratic Bezier Curve (3 control points)
                draw_list->AddBezierQuadratic(RblxVec2(x + cp3[0].x, y + cp3[0].y), RblxVec2(x + cp3[1].x, y + cp3[1].y), RblxVec2(x + cp3[2].x, y + cp3[2].y), col, th, curve_segments);
                x += sz + spacing;

                // Cubic Bezier Curve (4 control points)
                draw_list->AddBezierCubic(RblxVec2(x + cp4[0].x, y + cp4[0].y), RblxVec2(x + cp4[1].x, y + cp4[1].y), RblxVec2(x + cp4[2].x, y + cp4[2].y), RblxVec2(x + cp4[3].x, y + cp4[3].y), col, th, curve_segments);

                x = p.x + 4;
                y += sz + spacing;
            }

            // Filled shapes
            draw_list->AddNgonFilled(RblxVec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, col, ngon_sides);             x += sz + spacing;  // N-gon
            draw_list->AddCircleFilled(RblxVec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, col, circle_segments);      x += sz + spacing;  // Circle
            draw_list->AddEllipseFilled(RblxVec2(x + sz * 0.5f, y + sz * 0.5f), RblxVec2(sz * 0.5f, sz * 0.3f), col, -0.3f, circle_segments); x += sz + spacing;// Ellipse
            draw_list->AddRectFilled(RblxVec2(x, y), RblxVec2(x + sz, y + sz), col);                                    x += sz + spacing;  // Square
            draw_list->AddRectFilled(RblxVec2(x, y), RblxVec2(x + sz, y + sz), col, 10.0f);                             x += sz + spacing;  // Square with all rounded corners
            draw_list->AddRectFilled(RblxVec2(x, y), RblxVec2(x + sz, y + sz), col, 10.0f, corners_tl_br);              x += sz + spacing;  // Square with two rounded corners
            draw_list->AddTriangleFilled(RblxVec2(x+sz*0.5f,y), RblxVec2(x+sz, y+sz-0.5f), RblxVec2(x, y+sz-0.5f), col);  x += sz + spacing;  // Triangle
            //draw_list->AddTriangleFilled(RblxVec2(x+sz*0.2f,y), RblxVec2(x, y+sz-0.5f), RblxVec2(x+sz*0.4f, y+sz-0.5f), col); x += sz*0.4f + spacing; // Thin triangle
            PathConcaveShape(draw_list, x, y, sz); draw_list->PathFillConcave(col);                                 x += sz + spacing;  // Concave shape
            draw_list->AddRectFilled(RblxVec2(x, y), RblxVec2(x + sz, y + thickness), col);                             x += sz + spacing;  // Horizontal line (faster than AddLine, but only handle integer thickness)
            draw_list->AddRectFilled(RblxVec2(x, y), RblxVec2(x + thickness, y + sz), col);                             x += spacing * 2.0f;// Vertical line (faster than AddLine, but only handle integer thickness)
            draw_list->AddRectFilled(RblxVec2(x, y), RblxVec2(x + 1, y + 1), col);                                      x += sz;            // Pixel (faster than AddLine)

            // Path
            draw_list->PathArcTo(RblxVec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, 3.141592f * -0.5f, 3.141592f);
            draw_list->PathFillConvex(col);
            x += sz + spacing;

            // Quadratic Bezier Curve (3 control points)
            draw_list->PathLineTo(RblxVec2(x + cp3[0].x, y + cp3[0].y));
            draw_list->PathBezierQuadraticCurveTo(RblxVec2(x + cp3[1].x, y + cp3[1].y), RblxVec2(x + cp3[2].x, y + cp3[2].y), curve_segments);
            draw_list->PathFillConvex(col);
            x += sz + spacing;

            draw_list->AddRectFilledMultiColor(RblxVec2(x, y), RblxVec2(x + sz, y + sz), RBLXUI_COL32(0, 0, 0, 255), RBLXUI_COL32(255, 0, 0, 255), RBLXUI_COL32(255, 255, 0, 255), RBLXUI_COL32(0, 255, 0, 255));
            x += sz + spacing;

            RblxUI::Dummy(RblxVec2((sz + spacing) * 13.2f, (sz + spacing) * 3.0f));
            RblxUI::PopItemWidth();
            RblxUI::EndTabItem();
        }

        if (RblxUI::BeginTabItem("Canvas"))
        {
            RBLXUI_DEMO_MARKER("Examples/Custom rendering/Canvas");
            static RblxVector<RblxVec2> points;
            static RblxVec2 scrolling(0.0f, 0.0f);
            static bool opt_enable_grid = true;
            static bool opt_enable_context_menu = true;
            static bool adding_line = false;

            RblxUI::Checkbox("Enable grid", &opt_enable_grid);
            RblxUI::Checkbox("Enable context menu", &opt_enable_context_menu);
            RblxUI::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");

            // Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region + own scrolling.
            // Here we demonstrate that this can be replaced by simple offsetting + custom drawing + PushClipRect/PopClipRect() calls.
            // To use a child window instead we could use, e.g:
            //      RblxUI::PushStyleVar(RblxUIStyleVar_WindowPadding, RblxVec2(0, 0));      // Disable padding
            //      RblxUI::PushStyleColor(RblxUICol_ChildBg, RBLXUI_COL32(50, 50, 50, 255));  // Set a background color
            //      RblxUI::BeginChild("canvas", RblxVec2(0.0f, 0.0f), RblxUIChildFlags_Borders, RblxUIWindowFlags_NoMove);
            //      RblxUI::PopStyleColor();
            //      RblxUI::PopStyleVar();
            //      [...]
            //      RblxUI::EndChild();

            // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
            RblxVec2 canvas_p0 = RblxUI::GetCursorScreenPos();      // RblxDrawList API uses screen coordinates!
            RblxVec2 canvas_sz = RblxUI::GetContentRegionAvail();   // Resize canvas to what's available
            if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
            if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
            RblxVec2 canvas_p1 = RblxVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

            // Draw border and background color
            RblxUIIO& io = RblxUI::GetIO();
            RblxDrawList* draw_list = RblxUI::GetWindowDrawList();
            draw_list->AddRectFilled(canvas_p0, canvas_p1, RBLXUI_COL32(50, 50, 50, 255));
            draw_list->AddRect(canvas_p0, canvas_p1, RBLXUI_COL32(255, 255, 255, 255));

            // This will catch our interactions
            RblxUI::InvisibleButton("canvas", canvas_sz, RblxUIButtonFlags_MouseButtonLeft | RblxUIButtonFlags_MouseButtonRight);
            const bool is_hovered = RblxUI::IsItemHovered(); // Hovered
            const bool is_active = RblxUI::IsItemActive();   // Held
            const RblxVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
            const RblxVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

            // Add first and second point
            if (is_hovered && !adding_line && RblxUI::IsMouseClicked(RblxUIMouseButton_Left))
            {
                points.push_back(mouse_pos_in_canvas);
                points.push_back(mouse_pos_in_canvas);
                adding_line = true;
            }
            if (adding_line)
            {
                points.back() = mouse_pos_in_canvas;
                if (!RblxUI::IsMouseDown(RblxUIMouseButton_Left))
                    adding_line = false;
            }

            // Pan (we use a zero mouse threshold when there's no context menu)
            // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
            const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
            if (is_active && RblxUI::IsMouseDragging(RblxUIMouseButton_Right, mouse_threshold_for_pan))
            {
                scrolling.x += io.MouseDelta.x;
                scrolling.y += io.MouseDelta.y;
            }

            // Context menu (under default mouse threshold)
            RblxVec2 drag_delta = RblxUI::GetMouseDragDelta(RblxUIMouseButton_Right);
            if (opt_enable_context_menu && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
                RblxUI::OpenPopupOnItemClick("context", RblxUIPopupFlags_MouseButtonRight);
            if (RblxUI::BeginPopup("context"))
            {
                if (adding_line)
                    points.resize(points.size() - 2);
                adding_line = false;
                if (RblxUI::MenuItem("Remove one", NULL, false, points.Size > 0)) { points.resize(points.size() - 2); }
                if (RblxUI::MenuItem("Remove all", NULL, false, points.Size > 0)) { points.clear(); }
                RblxUI::EndPopup();
            }

            // Draw grid + all lines in the canvas
            draw_list->PushClipRect(canvas_p0, canvas_p1, true);
            if (opt_enable_grid)
            {
                const float GRID_STEP = 64.0f;
                for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
                    draw_list->AddLineV(canvas_p0.x + x, canvas_p0.y, canvas_p1.y, RBLXUI_COL32(200, 200, 200, 40));
                for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
                    draw_list->AddLineH(canvas_p0.x, canvas_p1.x, canvas_p0.y + y, RBLXUI_COL32(200, 200, 200, 40));
            }
            for (int n = 0; n < points.Size; n += 2)
                draw_list->AddLine(RblxVec2(origin.x + points[n].x, origin.y + points[n].y), RblxVec2(origin.x + points[n + 1].x, origin.y + points[n + 1].y), RBLXUI_COL32(255, 255, 0, 255), 2.0f);
            draw_list->PopClipRect();

            RblxUI::EndTabItem();
        }

        if (RblxUI::BeginTabItem("BG/FG draw lists"))
        {
            RBLXUI_DEMO_MARKER("Examples/Custom rendering/BG & FG draw lists");
            static bool draw_bg = true;
            static bool draw_fg = true;
            RblxUI::Checkbox("Draw in Background draw list", &draw_bg);
            RblxUI::SameLine(); HelpMarker("The Background draw list will be rendered below every Dear RblxUI windows.");
            RblxUI::Checkbox("Draw in Foreground draw list", &draw_fg);
            RblxUI::SameLine(); HelpMarker("The Foreground draw list will be rendered over every Dear RblxUI windows.");
            RblxVec2 window_pos = RblxUI::GetWindowPos();
            RblxVec2 window_size = RblxUI::GetWindowSize();
            RblxVec2 window_center = RblxVec2(window_pos.x + window_size.x * 0.5f, window_pos.y + window_size.y * 0.5f);
            if (draw_bg)
                RblxUI::GetBackgroundDrawList()->AddCircle(window_center, window_size.x * 0.6f, RBLXUI_COL32(255, 0, 0, 200), 0, 10 + 4);
            if (draw_fg)
                RblxUI::GetForegroundDrawList()->AddCircle(window_center, window_size.y * 0.6f, RBLXUI_COL32(0, 255, 0, 200), 0, 10);
            RblxUI::EndTabItem();
        }

        // Demonstrate out-of-order rendering via channels splitting
        // We use functions in RblxDrawList as each draw list contains a convenience splitter,
        // but you can also instantiate your own RblxDrawListSplitter if you need to nest them.
        if (RblxUI::BeginTabItem("Draw Channels"))
        {
            RBLXUI_DEMO_MARKER("Examples/Custom rendering/Draw Channels");
            RblxDrawList* draw_list = RblxUI::GetWindowDrawList();
            {
                RblxUI::Text("Blue shape is drawn first: appears in back");
                RblxUI::Text("Red shape is drawn after: appears in front");
                RblxVec2 p0 = RblxUI::GetCursorScreenPos();
                draw_list->AddRectFilled(RblxVec2(p0.x, p0.y), RblxVec2(p0.x + 50, p0.y + 50), RBLXUI_COL32(0, 0, 255, 255)); // Blue
                draw_list->AddRectFilled(RblxVec2(p0.x + 25, p0.y + 25), RblxVec2(p0.x + 75, p0.y + 75), RBLXUI_COL32(255, 0, 0, 255)); // Red
                RblxUI::Dummy(RblxVec2(75, 75));
            }
            RblxUI::Separator();
            {
                RblxUI::Text("Blue shape is drawn first, into channel 1: appears in front");
                RblxUI::Text("Red shape is drawn after, into channel 0: appears in back");
                RblxVec2 p1 = RblxUI::GetCursorScreenPos();

                // Create 2 channels and draw a Blue shape THEN a Red shape.
                // You can create any number of channels. Tables API use 1 channel per column in order to better batch draw calls.
                draw_list->ChannelsSplit(2);
                draw_list->ChannelsSetCurrent(1);
                draw_list->AddRectFilled(RblxVec2(p1.x, p1.y), RblxVec2(p1.x + 50, p1.y + 50), RBLXUI_COL32(0, 0, 255, 255)); // Blue
                draw_list->ChannelsSetCurrent(0);
                draw_list->AddRectFilled(RblxVec2(p1.x + 25, p1.y + 25), RblxVec2(p1.x + 75, p1.y + 75), RBLXUI_COL32(255, 0, 0, 255)); // Red

                // Flatten/reorder channels. Red shape is in channel 0 and it appears below the Blue shape in channel 1.
                // This works by copying draw indices only (vertices are not copied).
                draw_list->ChannelsMerge();
                RblxUI::Dummy(RblxVec2(75, 75));
                RblxUI::Text("After reordering, contents of channel 0 appears below channel 1.");
            }
            RblxUI::EndTabItem();
        }

        RblxUI::EndTabBar();
    }

    RblxUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Docking, DockSpace / ShowExampleAppDockSpace()
//-----------------------------------------------------------------------------

struct RblxUIDemoDockspaceArgs
{
    bool                IsFullscreen = true;
    bool                KeepWindowPadding = false; // Keep WindowPadding to help understand that DockSpace() is a widget inside the window.
    RblxUIDockNodeFlags  DockSpaceFlags  = RblxUIDockNodeFlags_None;
};

// THIS IS A DEMO FOR ADVANCED USAGE OF DockSpace().
// MOST REGULAR APPLICATIONS WANTING TO ALLOW DOCKING WINDOWS ON THE EDGE OF YOUR SCREEN CAN SIMPLY USE:
//    RblxUI::NewFrame(); + RblxUI::DockSpaceOverViewport();                                                   // Create a dockspace in main viewport
// OR:
//    RblxUI::NewFrame(); + RblxUI::DockSpaceOverViewport(0, nullptr, RblxUIDockNodeFlags_PassthruCentralNode); // Create a dockspace in main viewport, where central node is transparent.
// Demonstrate using DockSpace() to create an explicit docking node within an existing window, with various options.
// Read https://github.com/ocornut/rblxui/wiki/Docking for details.
// The reasons we do not use DockSpaceOverViewport() in this demo is because:
// - (1) we allow the host window to be floating/moveable instead of filling the viewport (when args->IsFullscreen == false)
//       which is mostly to showcase the idea that DockSpace() may be submitted anywhere.
//       Also see 'Demo->Examples->Documents' for a less abstract version of this.
// - (2) we allow the host window to have padding (when args->UsePadding == true)
// - (3) we expose variety of other flags.
static void ShowExampleAppDockSpaceAdvanced(RblxUIDemoDockspaceArgs* args, bool* p_open)
{
    RblxUIDockNodeFlags dockspace_flags = args->DockSpaceFlags;

    // We are using the RblxUIWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    RblxUIWindowFlags window_flags = RblxUIWindowFlags_NoDocking;
    if (args->IsFullscreen)
    {
        // Fullscreen dockspace: practically the same as calling DockSpaceOverViewport();
        const RblxUIViewport* viewport = RblxUI::GetMainViewport();
        RblxUI::SetNextWindowPos(viewport->WorkPos);
        RblxUI::SetNextWindowSize(viewport->WorkSize);
        RblxUI::SetNextWindowViewport(viewport->ID);
        RblxUI::PushStyleVar(RblxUIStyleVar_WindowRounding, 0.0f);
        RblxUI::PushStyleVar(RblxUIStyleVar_WindowBorderSize, 0.0f);
        window_flags |= RblxUIWindowFlags_NoTitleBar | RblxUIWindowFlags_NoCollapse | RblxUIWindowFlags_NoResize | RblxUIWindowFlags_NoMove;
        window_flags |= RblxUIWindowFlags_NoBringToFrontOnFocus | RblxUIWindowFlags_NoNavFocus;
        window_flags |= RblxUIWindowFlags_NoBackground;
    }
    else
    {
        // Floating dockspace
        dockspace_flags &= ~RblxUIDockNodeFlags_PassthruCentralNode;
    }

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!args->KeepWindowPadding)
        RblxUI::PushStyleVar(RblxUIStyleVar_WindowPadding, RblxVec2(0.0f, 0.0f));
    RblxUI::Begin("Window with a DockSpace", p_open, window_flags);
    if (!args->KeepWindowPadding)
        RblxUI::PopStyleVar();

    if (args->IsFullscreen)
        RblxUI::PopStyleVar(2);

    // Submit the DockSpace widget inside our window
    // - Note that the id here is different from the one used by DockSpaceOverViewport(), so docking state won't get transfered between "Basic" and "Advanced" demos.
    // - If we made the ShowExampleAppDockSpaceBasic() calculate its own ID and pass it to DockSpaceOverViewport() the ID could easily match.
    RblxUIID dockspace_id = RblxUI::GetID("MyDockSpace");
    RblxUI::DockSpace(dockspace_id, RblxVec2(0.0f, 0.0f), dockspace_flags);

    RblxUI::End();
}

static void ShowExampleAppDockSpaceBasic(RblxUIDockNodeFlags flags)
{
    // Basic version which you can use in many apps:
    // e.g:
    //   RblxUI::DockSpaceOverViewport();
    // or:
    //   RblxUI::DockSpaceOverViewport(0, nullptr, RblxUIDockNodeFlags_PassthruCentralNode); // Central node will be transparent
    // or:
    //   RblxUIViewport* viewport = RblxUI::GetMainViewport();
    //   RblxUI::DockSpaceOverViewport(0, viewport, RblxUIDockNodeFlags_None);

    RblxUI::DockSpaceOverViewport(0, nullptr, flags);
}

void ShowExampleAppDockSpace(bool* p_open)
{
    static int opt_demo_mode = 0;
    static bool opt_demo_mode_changed = false;
    static RblxUIDemoDockspaceArgs args;

    if (opt_demo_mode == 0)
        ShowExampleAppDockSpaceBasic(args.DockSpaceFlags);
    else
        ShowExampleAppDockSpaceAdvanced(&args, p_open);

    // Refocus our window to minimize perceived loss of focus when changing mode (caused by the fact that each use a different window, which would not happen in a real app)
    if (opt_demo_mode_changed)
        RblxUI::SetNextWindowFocus();
    RblxUI::Begin("Examples: Dockspace", p_open, RblxUIWindowFlags_MenuBar);
    opt_demo_mode_changed = false;
    opt_demo_mode_changed |= RblxUI::RadioButton("Basic demo mode", &opt_demo_mode, 0);
    opt_demo_mode_changed |= RblxUI::RadioButton("Advanced demo mode", &opt_demo_mode, 1);

    RblxUI::SeparatorText("Options");

    if ((RblxUI::GetIO().ConfigFlags & RblxUIConfigFlags_DockingEnable) == 0)
    {
        ShowDockingDisabledMessage();
    }
    else if (opt_demo_mode == 0)
    {
        args.DockSpaceFlags &= RblxUIDockNodeFlags_PassthruCentralNode; // Allowed flags
        RblxUI::CheckboxFlags("Flag: PassthruCentralNode", &args.DockSpaceFlags, RblxUIDockNodeFlags_PassthruCentralNode);
    }
    else if (opt_demo_mode == 1)
    {
        RblxUI::Checkbox("Fullscreen", &args.IsFullscreen);
        RblxUI::Checkbox("Keep Window Padding", &args.KeepWindowPadding);
        RblxUI::SameLine();
        HelpMarker("This is mostly exposed to facilitate understanding that a DockSpace() is _inside_ a window.");
        RblxUI::BeginDisabled(args.IsFullscreen == false);
        RblxUI::CheckboxFlags("Flag: PassthruCentralNode",      &args.DockSpaceFlags, RblxUIDockNodeFlags_PassthruCentralNode);
        RblxUI::EndDisabled();
        RblxUI::CheckboxFlags("Flag: NoDockingOverCentralNode", &args.DockSpaceFlags, RblxUIDockNodeFlags_NoDockingOverCentralNode);
        RblxUI::CheckboxFlags("Flag: NoDockingSplit",           &args.DockSpaceFlags, RblxUIDockNodeFlags_NoDockingSplit);
        RblxUI::CheckboxFlags("Flag: NoUndocking",              &args.DockSpaceFlags, RblxUIDockNodeFlags_NoUndocking);
        RblxUI::CheckboxFlags("Flag: NoResize",                 &args.DockSpaceFlags, RblxUIDockNodeFlags_NoResize);
        RblxUI::CheckboxFlags("Flag: AutoHideTabBar",           &args.DockSpaceFlags, RblxUIDockNodeFlags_AutoHideTabBar);
    }

    // Show demo options and help
    if (RblxUI::BeginMenuBar())
    {
        if (RblxUI::BeginMenu("Help"))
        {
            RblxUI::TextUnformatted(
                "This demonstrates the use of RblxUI::DockSpace() which allows you to manually\ncreate a docking node _within_ another window." "\n"
                "The \"Basic\" version uses the RblxUI::DockSpaceOverViewport() helper. Most applications can probably use this.");
            RblxUI::Separator();
            RblxUI::TextUnformatted("When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
                "- Drag from window title bar or their tab to dock/undock." "\n"
                "- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
                "- Hold SHIFT to disable docking (if io.ConfigDockingWithShift == false, default)" "\n"
                "- Hold SHIFT to enable docking (if io.ConfigDockingWithShift == true)");
            RblxUI::Separator();
            RblxUI::TextUnformatted("More details:"); RblxUI::Bullet(); RblxUI::SameLine(); RblxUI::TextLinkOpenURL("Docking Wiki page", "https://github.com/ocornut/rblxui/wiki/Docking");
            RblxUI::BulletText("Read comments in ShowExampleAppDockSpace()");
            RblxUI::EndMenu();
        }
        RblxUI::EndMenuBar();
    }

    RblxUI::End();
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
    RblxVec4      Color;      // An arbitrary variable associated to the document

    MyDocument(int uid, const char* name, bool open = true, const RblxVec4& color = RblxVec4(1.0f, 1.0f, 1.0f, 1.0f))
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
    RblxVector<MyDocument>    Documents;
    RblxVector<MyDocument*>   CloseQueue;
    MyDocument*             RenamingDoc = NULL;
    bool                    RenamingStarted = false;

    ExampleAppDocuments()
    {
        Documents.push_back(MyDocument(0, "Lettuce",             true,  RblxVec4(0.4f, 0.8f, 0.4f, 1.0f)));
        Documents.push_back(MyDocument(1, "Eggplant",            true,  RblxVec4(0.8f, 0.5f, 1.0f, 1.0f)));
        Documents.push_back(MyDocument(2, "Carrot",              true,  RblxVec4(1.0f, 0.8f, 0.5f, 1.0f)));
        Documents.push_back(MyDocument(3, "Tomato",              false, RblxVec4(1.0f, 0.3f, 0.4f, 1.0f)));
        Documents.push_back(MyDocument(4, "A Rather Long Title", false, RblxVec4(0.4f, 0.8f, 0.8f, 1.0f)));
        Documents.push_back(MyDocument(5, "Some Document",       false, RblxVec4(0.8f, 0.8f, 1.0f, 1.0f)));
    }

    // As we allow to change document name, we append a never-changing document ID so tabs are stable
    void GetTabName(MyDocument* doc, char* out_buf, size_t out_buf_size)
    {
        snprintf(out_buf, out_buf_size, "%s###doc%d", doc->Name, doc->UID);
    }

    // Display placeholder contents for the Document
    void DisplayDocContents(MyDocument* doc)
    {
        RblxUI::PushID(doc);
        RblxUI::Text("Document \"%s\"", doc->Name);
        RblxUI::PushStyleColor(RblxUICol_Text, doc->Color);
        RblxUI::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");
        RblxUI::PopStyleColor();

        RblxUI::SetNextItemShortcut(RblxUIMod_Ctrl | RblxUIKey_R, RblxUIInputFlags_Tooltip);
        if (RblxUI::Button("Rename.."))
        {
            RenamingDoc = doc;
            RenamingStarted = true;
        }
        RblxUI::SameLine();

        RblxUI::SetNextItemShortcut(RblxUIMod_Ctrl | RblxUIKey_M, RblxUIInputFlags_Tooltip);
        if (RblxUI::Button("Modify"))
            doc->Dirty = true;

        RblxUI::SameLine();
        RblxUI::SetNextItemShortcut(RblxUIMod_Ctrl | RblxUIKey_S, RblxUIInputFlags_Tooltip);
        if (RblxUI::Button("Save"))
            doc->DoSave();

        RblxUI::SameLine();
        RblxUI::SetNextItemShortcut(RblxUIMod_Ctrl | RblxUIKey_W, RblxUIInputFlags_Tooltip);
        if (RblxUI::Button("Close"))
            CloseQueue.push_back(doc);
        RblxUI::ColorEdit3("color", &doc->Color.x);  // Useful to test drag and drop and hold-dragged-to-open-tab behavior.
        RblxUI::PopID();
    }

    // Display context menu for the Document
    void DisplayDocContextMenu(MyDocument* doc)
    {
        if (!RblxUI::BeginPopupContextItem())
            return;

        char buf[256];
        sprintf(buf, "Save %s", doc->Name);
        if (RblxUI::MenuItem(buf, "Ctrl+S", false, doc->Open))
            doc->DoSave();
        if (RblxUI::MenuItem("Rename...", "Ctrl+R", false, doc->Open))
            RenamingDoc = doc;
        if (RblxUI::MenuItem("Close", "Ctrl+W", false, doc->Open))
            CloseQueue.push_back(doc);
        RblxUI::EndPopup();
    }

    // [Optional] Notify the system of Tabs/Windows closure that happened outside the regular tab interface.
    // If a tab has been closed programmatically (aka closed from another source such as the Checkbox() in the demo,
    // as opposed to clicking on the regular tab closing button) and stops being submitted, it will take a frame for
    // the tab bar to notice its absence. During this frame there will be a gap in the tab bar, and if the tab that has
    // disappeared was the selected one, the tab bar will report no selected tab during the frame. This will effectively
    // give the impression of a flicker for one frame.
    // We call SetTabItemClosed() to manually notify the Tab Bar or Docking system of removed tabs to avoid this glitch.
    // Note that this completely optional, and only affect tab bars with the RblxUITabBarFlags_Reorderable flag.
    void NotifyOfDocumentsClosedElsewhere()
    {
        for (MyDocument& doc : Documents)
        {
            if (!doc.Open && doc.OpenPrev)
                RblxUI::SetTabItemClosed(doc.Name);
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
    static RblxUITabBarFlags opt_fitting_flags = RblxUITabBarFlags_FittingPolicyDefault_;

    // When (opt_target == Target_DockSpaceAndWindow) there is the possibily that one of our child Document window (e.g. "Eggplant")
    // that we emit gets docked into the same spot as the parent window ("Example: Documents").
    // This would create a problematic feedback loop because selecting the "Eggplant" tab would make the "Example: Documents" tab
    // not visible, which in turn would stop submitting the "Eggplant" window.
    // We avoid this problem by submitting our documents window even if our parent window is not currently visible.
    // Another solution may be to make the "Example: Documents" window use the RblxUIWindowFlags_NoDocking.

    bool window_contents_visible = RblxUI::Begin("Example: Documents", p_open, RblxUIWindowFlags_MenuBar);
    if (!window_contents_visible && opt_target != Target_DockSpaceAndWindow)
    {
        RblxUI::End();
        return;
    }
    RBLXUI_DEMO_MARKER("Examples/Documents");

    // Menu
    if (RblxUI::BeginMenuBar())
    {
        if (RblxUI::BeginMenu("File"))
        {
            int open_count = 0;
            for (MyDocument& doc : app.Documents)
                open_count += doc.Open ? 1 : 0;

            if (RblxUI::BeginMenu("Open", open_count < app.Documents.Size))
            {
                for (MyDocument& doc : app.Documents)
                    if (!doc.Open && RblxUI::MenuItem(doc.Name))
                        doc.DoOpen();
                RblxUI::EndMenu();
            }
            if (RblxUI::MenuItem("Close All Documents", NULL, false, open_count > 0))
                for (MyDocument& doc : app.Documents)
                    app.CloseQueue.push_back(&doc);
            if (RblxUI::MenuItem("Exit") && p_open)
                *p_open = false;
            RblxUI::EndMenu();
        }
        RblxUI::EndMenuBar();
    }

    // [Debug] List documents with one checkbox for each
    for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
    {
        MyDocument& doc = app.Documents[doc_n];
        if (doc_n > 0)
            RblxUI::SameLine();
        RblxUI::PushID(&doc);
        if (RblxUI::Checkbox(doc.Name, &doc.Open))
            if (!doc.Open)
                doc.DoForceClose();
        RblxUI::PopID();
    }
    RblxUI::PushItemWidth(RblxUI::GetFontSize() * 12);
    RblxUI::Combo("Output", (int*)&opt_target, "None\0TabBar+Tabs\0DockSpace+Window\0");
    RblxUI::PopItemWidth();
    bool redock_all = false;
    if (opt_target == Target_Tab)                { RblxUI::SameLine(); RblxUI::Checkbox("Reorderable Tabs", &opt_reorderable); }
    if (opt_target == Target_DockSpaceAndWindow) { RblxUI::SameLine(); redock_all = RblxUI::Button("Redock all"); }

    RblxUI::Separator();

    // About the RblxUIWindowFlags_UnsavedDocument / RblxUITabItemFlags_UnsavedDocument flags.
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
        RblxUITabBarFlags tab_bar_flags = (opt_fitting_flags) | (opt_reorderable ? RblxUITabBarFlags_Reorderable : 0);
        tab_bar_flags |= RblxUITabBarFlags_DrawSelectedOverline;
        if (RblxUI::BeginTabBar("##tabs", tab_bar_flags))
        {
            if (opt_reorderable)
                app.NotifyOfDocumentsClosedElsewhere();

            // [DEBUG] Stress tests
            //if ((RblxUI::GetFrameCount() % 30) == 0) docs[1].Open ^= 1;            // [DEBUG] Automatically show/hide a tab. Test various interactions e.g. dragging with this on.
            //if (RblxUI::GetIO().KeyCtrl) RblxUI::SetTabItemSelected(docs[1].Name);  // [DEBUG] Test SetTabItemSelected(), probably not very useful as-is anyway..

            // Submit Tabs
            for (MyDocument& doc : app.Documents)
            {
                if (!doc.Open)
                    continue;

                // As we allow to change document name, we append a never-changing document id so tabs are stable
                char doc_name_buf[64];
                app.GetTabName(&doc, doc_name_buf, sizeof(doc_name_buf));
                RblxUITabItemFlags tab_flags = (doc.Dirty ? RblxUITabItemFlags_UnsavedDocument : 0);
                bool visible = RblxUI::BeginTabItem(doc_name_buf, &doc.Open, tab_flags);

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
                    RblxUI::EndTabItem();
                }
            }

            RblxUI::EndTabBar();
        }
    }
    else if (opt_target == Target_DockSpaceAndWindow)
    {
        if (RblxUI::GetIO().ConfigFlags & RblxUIConfigFlags_DockingEnable)
        {
            app.NotifyOfDocumentsClosedElsewhere();

            // Create a DockSpace node where any window can be docked
            RblxUIID dockspace_id = RblxUI::GetID("MyDockSpace");
            RblxUI::DockSpace(dockspace_id);

            // Create Windows
            for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
            {
                MyDocument* doc = &app.Documents[doc_n];
                if (!doc->Open)
                    continue;

                RblxUI::SetNextWindowDockID(dockspace_id, redock_all ? RblxUICond_Always : RblxUICond_FirstUseEver);
                RblxUIWindowFlags window_flags = (doc->Dirty ? RblxUIWindowFlags_UnsavedDocument : 0);
                bool visible = RblxUI::Begin(doc->Name, &doc->Open, window_flags);

                // Cancel attempt to close when unsaved add to save queue so we can display a popup.
                if (!doc->Open && doc->Dirty)
                {
                    doc->Open = true;
                    app.CloseQueue.push_back(doc);
                }

                app.DisplayDocContextMenu(doc);
                if (visible)
                    app.DisplayDocContents(doc);

                RblxUI::End();
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
        RblxUI::End();
        return;
    }

    // Display renaming UI
    if (app.RenamingDoc != NULL)
    {
        if (app.RenamingStarted)
            RblxUI::OpenPopup("Rename");
        if (RblxUI::BeginPopup("Rename"))
        {
            RblxUI::SetNextItemWidth(RblxUI::GetFontSize() * 30);
            if (RblxUI::InputText("###Name", app.RenamingDoc->Name, RBLXUI_COUNTOF(app.RenamingDoc->Name), RblxUIInputTextFlags_EnterReturnsTrue))
            {
                RblxUI::CloseCurrentPopup();
                app.RenamingDoc = NULL;
            }
            if (app.RenamingStarted)
                RblxUI::SetKeyboardFocusHere(-1);
            RblxUI::EndPopup();
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
            if (!RblxUI::IsPopupOpen("Save?"))
                RblxUI::OpenPopup("Save?");
            if (RblxUI::BeginPopupModal("Save?", NULL, RblxUIWindowFlags_AlwaysAutoResize))
            {
                RblxUI::Text("Save change to the following items?");
                float item_height = RblxUI::GetTextLineHeightWithSpacing();
                if (RblxUI::BeginChild(RblxUI::GetID("frame"), RblxVec2(-FLT_MIN, 6.25f * item_height), RblxUIChildFlags_FrameStyle))
                    for (MyDocument* doc : app.CloseQueue)
                        if (doc->Dirty)
                            RblxUI::Text("%s", doc->Name);
                RblxUI::EndChild();

                RblxVec2 button_size(RblxUI::GetFontSize() * 7.0f, 0.0f);
                if (RblxUI::Button("Yes", button_size))
                {
                    for (MyDocument* doc : app.CloseQueue)
                    {
                        if (doc->Dirty)
                            doc->DoSave();
                        doc->DoForceClose();
                    }
                    app.CloseQueue.clear();
                    RblxUI::CloseCurrentPopup();
                }
                RblxUI::SameLine();
                if (RblxUI::Button("No", button_size))
                {
                    for (MyDocument* doc : app.CloseQueue)
                        doc->DoForceClose();
                    app.CloseQueue.clear();
                    RblxUI::CloseCurrentPopup();
                }
                RblxUI::SameLine();
                if (RblxUI::Button("Cancel", button_size))
                {
                    app.CloseQueue.clear();
                    RblxUI::CloseCurrentPopup();
                }
                RblxUI::EndPopup();
            }
        }
    }

    RblxUI::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Assets Browser / ShowExampleAppAssetsBrowser()
//-----------------------------------------------------------------------------

//#include "rblxui_internal.h" // NavMoveRequestTryWrapping()

struct ExampleAsset
{
    RblxUIID ID;
    int     Type;

    ExampleAsset(RblxUIID id, int type) { ID = id; Type = type; }

    static const RblxUITableSortSpecs* s_current_sort_specs;

    static void SortWithSortSpecs(RblxUITableSortSpecs* sort_specs, ExampleAsset* items, int items_count)
    {
        s_current_sort_specs = sort_specs; // Store in variable accessible by the sort function.
        if (items_count > 1)
            qsort(items, (size_t)items_count, sizeof(items[0]), ExampleAsset::CompareWithSortSpecs);
        s_current_sort_specs = NULL;
    }

    // Compare function to be used by qsort()
    static int RBLXUI_CDECL CompareWithSortSpecs(const void* lhs, const void* rhs)
    {
        const ExampleAsset* a = (const ExampleAsset*)lhs;
        const ExampleAsset* b = (const ExampleAsset*)rhs;
        for (int n = 0; n < s_current_sort_specs->SpecsCount; n++)
        {
            const RblxUITableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
            int delta = 0;
            if (sort_spec->ColumnIndex == 0)
                delta = ((int)a->ID - (int)b->ID);
            else if (sort_spec->ColumnIndex == 1)
                delta = (a->Type - b->Type);
            if (delta > 0)
                return (sort_spec->SortDirection == RblxUISortDirection_Ascending) ? +1 : -1;
            if (delta < 0)
                return (sort_spec->SortDirection == RblxUISortDirection_Ascending) ? -1 : +1;
        }
        return (int)a->ID - (int)b->ID;
    }
};
const RblxUITableSortSpecs* ExampleAsset::s_current_sort_specs = NULL;

struct ExampleAssetsBrowser
{
    // Options
    bool            ShowTypeOverlay = true;
    bool            AllowSorting = true;
    bool            AllowBoxSelect = true;                  // Will set RblxUIMultiSelectFlags_BoxSelect2d
    bool            AllowBoxSelectInsideSelection = false;  // Will set RblxUIMultiSelectFlags_SelectOnClickAlways
    bool            AllowDragUnselected = false;            // Will set RblxUIMultiSelectFlags_SelectOnClickRelease
    float           IconSize = 0;
    int             IconSpacing = 10;
    int             IconHitSpacing = 4;                     // Increase hit-spacing if you want to make it possible to clear or box-select from gaps. Some spacing is required to able to amend with Shift+box-select. Value is small in Explorer.
    bool            StretchSpacing = true;
    bool            UseScrollX = false;                     // Debug: submit twice the number of items per line (overflow horizontally to exercise ScrollX + box-select)

    // State
    RblxVector<ExampleAsset> Items;               // Our items
    ExampleSelectionWithDeletion Selection;     // Our selection (RblxUISelectionBasicStorage + helper funcs to handle deletion)
    RblxUIID         NextItemId = 0;             // Unique identifier when creating new items
    bool            RequestDelete = false;      // Deferred deletion request
    bool            RequestSort = false;        // Deferred sort request
    float           ZoomWheelAccum = 0.0f;      // Mouse wheel accumulator to handle smooth wheels better

    // Calculated sizes for layout, output of UpdateLayoutSizes(). Could be locals but our code is simpler this way.
    RblxVec2          LayoutItemSize;
    RblxVec2          LayoutItemStep;             // == LayoutItemSize + LayoutItemSpacing
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
        LayoutItemSize = RblxVec2(floorf(IconSize), floorf(IconSize));
        LayoutColumnCount = RBLXUI_MAX((int)(avail_width / (LayoutItemSize.x + LayoutItemSpacing)), 1);

        // Layout: when stretching: allocate remaining space to more spacing. Round before division, so item_spacing may be non-integer.
        if (StretchSpacing && LayoutColumnCount > 1)
            LayoutItemSpacing = floorf(avail_width - LayoutItemSize.x * LayoutColumnCount) / LayoutColumnCount;

        if (UseScrollX)
            LayoutColumnCount *= 2;
        LayoutLineCount = (Items.Size + LayoutColumnCount - 1) / LayoutColumnCount;

        LayoutItemStep = RblxVec2(LayoutItemSize.x + LayoutItemSpacing, LayoutItemSize.y + LayoutItemSpacing);
        LayoutSelectableSpacing = RBLXUI_MAX(floorf(LayoutItemSpacing) - IconHitSpacing, 0.0f);
        LayoutOuterPadding = floorf(LayoutItemSpacing * 0.5f);
    }

    void Draw(const char* title, bool* p_open)
    {
        if (IconSize <= 0.0f)
            IconSize = RblxUI::CalcTextSize("99999").x;

        RblxUI::SetNextWindowSize(RblxVec2(IconSize * 25, IconSize * 15), RblxUICond_FirstUseEver);
        if (!RblxUI::Begin(title, p_open, RblxUIWindowFlags_MenuBar))
        {
            RblxUI::End();
            return;
        }
        RBLXUI_DEMO_MARKER("Examples/Assets Browser");

        // Menu bar
        if (RblxUI::BeginMenuBar())
        {
            if (RblxUI::BeginMenu("File"))
            {
                if (RblxUI::MenuItem("Add 10000 items"))
                    AddItems(10000);
                if (RblxUI::MenuItem("Clear items"))
                    ClearItems();
                RblxUI::Separator();
                if (RblxUI::MenuItem("Close", NULL, false, p_open != NULL))
                    *p_open = false;
                RblxUI::EndMenu();
            }
            if (RblxUI::BeginMenu("Edit"))
            {
                if (RblxUI::MenuItem("Delete", "Del", false, Selection.Size > 0))
                    RequestDelete = true;
                RblxUI::EndMenu();
            }
            if (RblxUI::BeginMenu("Options"))
            {
                RblxUI::PushItemWidth(RblxUI::GetFontSize() * 10);

                RblxUI::SeparatorText("Contents");
                RblxUI::Checkbox("Show Type Overlay", &ShowTypeOverlay);
                RblxUI::Checkbox("Allow Sorting", &AllowSorting);

                RblxUI::SeparatorText("Selection Behavior");
                RblxUI::Checkbox("Allow box-selection", &AllowBoxSelect);
                if (RblxUI::Checkbox("Allow box-selection from selected items", &AllowBoxSelectInsideSelection) && AllowBoxSelectInsideSelection)
                    AllowDragUnselected = false;
                if (RblxUI::Checkbox("Allow dragging unselected item", &AllowDragUnselected) && AllowDragUnselected)
                    AllowBoxSelectInsideSelection = false;

                RblxUI::SeparatorText("Layout");
                RblxUI::SliderFloat("Icon Size", &IconSize, 16.0f, 128.0f, "%.0f");
                RblxUI::SameLine(); HelpMarker("Use Ctrl+Wheel to zoom");
                RblxUI::SliderInt("Icon Spacing", &IconSpacing, 0, 32);
                RblxUI::SliderInt("Icon Hit Spacing", &IconHitSpacing, 0, 32);
                RblxUI::Checkbox("Stretch Spacing", &StretchSpacing);
                RblxUI::Checkbox("Use ScrollX", &UseScrollX);
                RblxUI::PopItemWidth();
                RblxUI::EndMenu();
            }
            RblxUI::EndMenuBar();
        }

        // Show a table with ONLY one header row to showcase the idea/possibility of using this to provide a sorting UI
        if (AllowSorting)
        {
            RblxUI::PushStyleVar(RblxUIStyleVar_ItemSpacing, RblxVec2(0, 0));
            RblxUITableFlags table_flags_for_sort_specs = RblxUITableFlags_Sortable | RblxUITableFlags_SortMulti | RblxUITableFlags_SizingFixedFit | RblxUITableFlags_Borders;
            if (RblxUI::BeginTable("for_sort_specs_only", 2, table_flags_for_sort_specs, RblxVec2(0.0f, RblxUI::GetFrameHeight())))
            {
                RblxUI::TableSetupColumn("Index");
                RblxUI::TableSetupColumn("Type");
                RblxUI::TableHeadersRow();
                if (RblxUITableSortSpecs* sort_specs = RblxUI::TableGetSortSpecs())
                    if (sort_specs->SpecsDirty || RequestSort)
                    {
                        ExampleAsset::SortWithSortSpecs(sort_specs, Items.Data, Items.Size);
                        sort_specs->SpecsDirty = RequestSort = false;
                    }
                RblxUI::EndTable();
            }
            RblxUI::PopStyleVar();
        }

        RblxUIIO& io = RblxUI::GetIO();
        RblxUI::SetNextWindowContentSize(RblxVec2(0.0f, LayoutOuterPadding + LayoutLineCount * (LayoutItemSize.y + LayoutItemSpacing)));
        if (RblxUI::BeginChild("Assets", RblxVec2(0.0f, -RblxUI::GetTextLineHeightWithSpacing()), RblxUIChildFlags_Borders, RblxUIWindowFlags_NoMove | RblxUIWindowFlags_HorizontalScrollbar))
        {
            RblxDrawList* draw_list = RblxUI::GetWindowDrawList();

            const float avail_width = RblxUI::GetContentRegionAvail().x;
            UpdateLayoutSizes(avail_width);

            // Calculate and store start position.
            RblxVec2 start_pos = RblxUI::GetCursorScreenPos();
            start_pos = RblxVec2(start_pos.x + LayoutOuterPadding, start_pos.y + LayoutOuterPadding);
            RblxUI::SetCursorScreenPos(start_pos);

            // Multi-select
            RblxUIMultiSelectFlags ms_flags = RblxUIMultiSelectFlags_ClearOnEscape | RblxUIMultiSelectFlags_ClearOnClickVoid;

            // - Enable box-select (in 2D mode, so that changing box-select rectangle X1/X2 boundaries will affect clipped items)
            if (AllowBoxSelect)
                ms_flags |= RblxUIMultiSelectFlags_BoxSelect2d;

            // - Selection mode
            if (AllowDragUnselected)
                ms_flags |= RblxUIMultiSelectFlags_SelectOnClickRelease; // Rarely used: Allows dragging an unselected item without selecting it(rarely used)
            else if (AllowBoxSelectInsideSelection)
                ms_flags |= RblxUIMultiSelectFlags_SelectOnClickAlways; // Rarely used: Prevents Drag and Drop from being used on multiple-selection, but allows e.g. BoxSelect to always reselect even when clicking inside an existing selection.

            // - Enable keyboard wrapping on X axis
            // (FIXME-MULTISELECT: We haven't designed/exposed a general nav wrapping api yet, so this flag is provided as a courtesy to avoid doing:
            //    RblxUI::NavMoveRequestTryWrapping(RblxUI::GetCurrentWindow(), RblxUINavMoveFlags_WrapX);
            // When we finish implementing a more general API for this, we will obsolete this flag in favor of the new system)
            ms_flags |= RblxUIMultiSelectFlags_NavWrapX;

            RblxUIMultiSelectIO* ms_io = RblxUI::BeginMultiSelect(ms_flags, Selection.Size, Items.Size);

            // Use custom selection adapter: store ID in selection (recommended)
            Selection.UserData = this;
            Selection.AdapterIndexToStorageId = [](RblxUISelectionBasicStorage* self_, int idx) { ExampleAssetsBrowser* self = (ExampleAssetsBrowser*)self_->UserData; return self->Items[idx].ID; };
            Selection.ApplyRequests(ms_io);

            const bool want_delete = (RblxUI::Shortcut(RblxUIKey_Delete, RblxUIInputFlags_Repeat) && (Selection.Size > 0)) || RequestDelete;
            const int item_curr_idx_to_focus = want_delete ? Selection.ApplyDeletionPreLoop(ms_io, Items.Size) : -1;
            RequestDelete = false;

            // Push LayoutSelectableSpacing (which is LayoutItemSpacing minus hit-spacing, if we decide to have hit gaps between items)
            // Altering style ItemSpacing may seem unnecessary as we position every items using SetCursorScreenPos()...
            // But it is necessary for two reasons:
            // - Selectables uses it by default to visually fill the space between two items.
            // - The vertical spacing would be measured by Clipper to calculate line height if we didn't provide it explicitly (here we do).
            RblxUI::PushStyleVar(RblxUIStyleVar_ItemSpacing, RblxVec2(LayoutSelectableSpacing, LayoutSelectableSpacing));

            // Rendering parameters
            const RblxU32 icon_type_overlay_colors[3] = { 0, RBLXUI_COL32(200, 70, 70, 255), RBLXUI_COL32(70, 170, 70, 255) };
            const RblxU32 icon_bg_color = RblxUI::GetColorU32(RBLXUI_COL32(35, 35, 35, 220));
            const RblxVec2 icon_type_overlay_size = RblxVec2(4.0f, 4.0f);
            const bool display_label = (LayoutItemSize.x >= RblxUI::CalcTextSize("999").x);

            const int column_count = LayoutColumnCount;
            RblxUIListClipper clipper;
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
                    const int item_max_idx_for_current_line = RBLXUI_MIN((line_idx + 1) * column_count, Items.Size);
                    for (int item_idx = item_min_idx_for_current_line; item_idx < item_max_idx_for_current_line; ++item_idx)
                    {
                        ExampleAsset* item_data = &Items[item_idx];
                        RblxUI::PushID((int)item_data->ID);

                        // Position item
                        RblxVec2 pos = RblxVec2(start_pos.x + (item_idx % column_count) * LayoutItemStep.x, start_pos.y + line_idx * LayoutItemStep.y);
                        RblxUI::SetCursorScreenPos(pos);

                        RblxUI::SetNextItemSelectionUserData(item_idx);
                        bool item_is_selected = Selection.Contains((RblxUIID)item_data->ID);
                        bool item_is_visible = RblxUI::IsRectVisible(LayoutItemSize);
                        RblxUI::Selectable("", item_is_selected, RblxUISelectableFlags_None, LayoutItemSize);

                        // Update our selection state immediately (without waiting for EndMultiSelect() requests)
                        // because we use this to alter the color of our text/icon.
                        if (RblxUI::IsItemToggledSelection())
                            item_is_selected = !item_is_selected;

                        // Focus (for after deletion)
                        if (item_curr_idx_to_focus == item_idx)
                            RblxUI::SetKeyboardFocusHere(-1);

                        // Drag and drop
                        if (RblxUI::BeginDragDropSource())
                        {
                            // Create payload with full selection OR single unselected item.
                            // (the later is only possible when using RblxUIMultiSelectFlags_SelectOnClickRelease)
                            if (RblxUI::GetDragDropPayload() == NULL)
                            {
                                RblxVector<RblxUIID> payload_items;
                                void* it = NULL;
                                RblxUIID id = 0;
                                if (!item_is_selected)
                                    payload_items.push_back(item_data->ID);
                                else
                                    while (Selection.GetNextSelectedItem(&it, &id))
                                        payload_items.push_back(id);
                                RblxUI::SetDragDropPayload("ASSETS_BROWSER_ITEMS", payload_items.Data, (size_t)payload_items.size_in_bytes());
                            }

                            // Display payload content in tooltip, by extracting it from the payload data
                            // (we could read from selection, but it is more correct and reusable to read from payload)
                            const RblxUIPayload* payload = RblxUI::GetDragDropPayload();
                            const int payload_count = (int)payload->DataSize / (int)sizeof(RblxUIID);
                            RblxUI::Text("%d assets", payload_count);

                            RblxUI::EndDragDropSource();
                        }

                        // Render icon (a real app would likely display an image/thumbnail here)
                        // Because we use RblxUIMultiSelectFlags_BoxSelect2d, clipping vertical may occasionally be larger, so we coarse-clip our rendering as well.
                        if (item_is_visible)
                        {
                            RblxVec2 box_min(pos.x - 1, pos.y - 1);
                            RblxVec2 box_max(box_min.x + LayoutItemSize.x + 2, box_min.y + LayoutItemSize.y + 2); // Dubious
                            draw_list->AddRectFilled(box_min, box_max, icon_bg_color); // Background color
                            if (ShowTypeOverlay && item_data->Type != 0)
                            {
                                RblxU32 type_col = icon_type_overlay_colors[item_data->Type % RBLXUI_COUNTOF(icon_type_overlay_colors)];
                                draw_list->AddRectFilled(RblxVec2(box_max.x - 2 - icon_type_overlay_size.x, box_min.y + 2), RblxVec2(box_max.x - 2, box_min.y + 2 + icon_type_overlay_size.y), type_col);
                            }
                            if (display_label)
                            {
                                RblxU32 label_col = RblxUI::GetColorU32(item_is_selected ? RblxUICol_Text : RblxUICol_TextDisabled);
                                char label[32];
                                sprintf(label, "%d", item_data->ID);
                                draw_list->AddText(RblxVec2(box_min.x, box_max.y - RblxUI::GetFontSize()), label_col, label);
                            }
                        }

                        RblxUI::PopID();
                    }
                }
            }
            clipper.End();
            if (Items.Size == 0)
                RblxUI::Dummy(RblxVec2(0, 0));
            RblxUI::PopStyleVar(); // RblxUIStyleVar_ItemSpacing

            // Context menu
            if (RblxUI::BeginPopupContextWindow())
            {
                RblxUI::Text("Selection: %d items", Selection.Size);
                RblxUI::Separator();
                if (RblxUI::MenuItem("Delete", "Del", false, Selection.Size > 0))
                    RequestDelete = true;
                RblxUI::EndPopup();
            }

            ms_io = RblxUI::EndMultiSelect();
            Selection.ApplyRequests(ms_io);
            if (want_delete)
                Selection.ApplyDeletionPostLoop(ms_io, Items, item_curr_idx_to_focus);

            // Zooming with Ctrl+Wheel
            if (RblxUI::IsWindowAppearing())
                ZoomWheelAccum = 0.0f;
            if (RblxUI::IsWindowHovered() && io.MouseWheel != 0.0f && RblxUI::IsKeyDown(RblxUIMod_Ctrl) && RblxUI::IsAnyItemActive() == false)
            {
                ZoomWheelAccum += io.MouseWheel;
                if (fabsf(ZoomWheelAccum) >= 1.0f)
                {
                    // Calculate hovered item index from mouse location
                    // FIXME: Locking aiming on 'hovered_item_idx' (with a cool-down timer) would ensure zoom keeps on it.
                    const float hovered_item_nx = (io.MousePos.x - start_pos.x + LayoutItemSpacing * 0.5f) / LayoutItemStep.x;
                    const float hovered_item_ny = (io.MousePos.y - start_pos.y + LayoutItemSpacing * 0.5f) / LayoutItemStep.y;
                    const int hovered_item_idx = ((int)hovered_item_ny * LayoutColumnCount) + (int)hovered_item_nx;
                    //RblxUI::SetTooltip("%f,%f -> item %d", hovered_item_nx, hovered_item_ny, hovered_item_idx); // Move those 4 lines in block above for easy debugging

                    // Zoom
                    IconSize *= powf(1.1f, (float)(int)ZoomWheelAccum);
                    IconSize = RBLXUI_CLAMP(IconSize, 16.0f, 128.0f);
                    ZoomWheelAccum -= (int)ZoomWheelAccum;
                    UpdateLayoutSizes(avail_width);

                    // Manipulate scroll to that we will land at the same Y location of currently hovered item.
                    // - Calculate next frame position of item under mouse
                    // - Set new scroll position to be used in next RblxUI::BeginChild() call.
                    float hovered_item_rel_pos_y = ((float)(hovered_item_idx / LayoutColumnCount) + fmodf(hovered_item_ny, 1.0f)) * LayoutItemStep.y;
                    hovered_item_rel_pos_y += RblxUI::GetStyle().WindowPadding.y;
                    float mouse_local_y = io.MousePos.y - RblxUI::GetWindowPos().y;
                    RblxUI::SetScrollY(hovered_item_rel_pos_y - mouse_local_y);
                }
            }
        }
        RblxUI::EndChild();

        RblxUI::Text("Selected: %d/%d items", Selection.Size, Items.Size);
        RblxUI::End();
    }
};

void ShowExampleAppAssetsBrowser(bool* p_open)
{
    RBLXUI_DEMO_MARKER("Examples/Assets Browser");
    static ExampleAssetsBrowser assets_browser;
    assets_browser.Draw("Example: Assets Browser", p_open);
}

// End of Demo code
#else

void RblxUI::ShowAboutWindow(bool*) {}
void RblxUI::ShowDemoWindow(bool*) {}
void RblxUI::ShowUserGuide() {}
void RblxUI::ShowStyleEditor(RblxUIStyle*) {}
bool RblxUI::ShowStyleSelector(const char*) { return false; }

#endif // #ifndef RBLXUI_DISABLE_DEMO_WINDOWS

#endif // #ifndef RBLXUI_DISABLE
