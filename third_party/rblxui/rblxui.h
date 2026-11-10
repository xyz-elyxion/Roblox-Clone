// dear rblxui, v1.92.9 WIP
// (headers)

// Help:
// - Call and read RblxUI::ShowDemoWindow() in rblxui_demo.cpp. All applications in examples/ are doing that.
// - Read top of rblxui.cpp for more details, links and comments.
// - Add '#define RBLXUI_DEFINE_MATH_OPERATORS' before including rblxui.h (or in rblxuiconfig.h) to access courtesy maths operators for RblxVec2 and RblxVec4.

// Resources:
// - FAQ ........................ https://dearrblxui.com/faq (in repository as docs/FAQ.md)
// - Homepage ................... https://github.com/ocornut/rblxui
// - Releases & Changelog ....... https://github.com/ocornut/rblxui/releases
// - Gallery .................... https://github.com/ocornut/rblxui/issues?q=label%3Agallery (please post your screenshots/video there!)
// - Wiki ....................... https://github.com/ocornut/rblxui/wiki (lots of good stuff there)
//   - Getting Started            https://github.com/ocornut/rblxui/wiki/Getting-Started (how to integrate in an existing app by adding ~25 lines of code)
//   - Third-party Extensions     https://github.com/ocornut/rblxui/wiki/Useful-Extensions (RblxPlot & many more)
//   - Bindings/Backends          https://github.com/ocornut/rblxui/wiki/Bindings (language bindings + backends for various tech/engines)
//   - Debug Tools                https://github.com/ocornut/rblxui/wiki/Debug-Tools
//   - Glossary                   https://github.com/ocornut/rblxui/wiki/Glossary
//   - Software using Dear RblxUI  https://github.com/ocornut/rblxui/wiki/Software-using-dear-rblxui
// - Issues & support ........... https://github.com/ocornut/rblxui/issues
// - Test Engine & Automation ... https://github.com/ocornut/rblxui_test_engine (test suite, test engine to automate your apps)
// - Web version of the Demo .... https://pthom.github.io/rblxui_explorer (w/ source code browser)

// For FIRST-TIME users having issues compiling/linking/running:
// please post in https://github.com/ocornut/rblxui/discussions if you cannot find a solution in resources above.
// EVERYTHING ELSE should be asked in 'Issues'! We are building a database of cross-linked knowledge there.
// Since 1.92, we encourage font loading questions to also be posted in 'Issues'.

// Library Version
// (Integer encoded as XYYZZ for use in #if preprocessor conditionals, e.g. '#if RBLXUI_VERSION_NUM >= 12345')
#define RBLXUI_VERSION       "1.92.9 WIP"
#define RBLXUI_VERSION_NUM   19282
#define RBLXUI_HAS_TABLE             // Added BeginTable() - from RBLXUI_VERSION_NUM >= 18000
#define RBLXUI_HAS_TEXTURES          // Added RblxUIBackendFlags_RendererHasTextures - from RBLXUI_VERSION_NUM >= 19198
#define RBLXUI_HAS_VIEWPORT          // In 'docking' WIP branch.
#define RBLXUI_HAS_DOCK              // In 'docking' WIP branch.

/*

Index of this file:
// [SECTION] Header mess
// [SECTION] Forward declarations and basic types
// [SECTION] Texture identifiers (RblxTextureID, RblxTextureRef)
// [SECTION] Dear RblxUI end-user API functions
// [SECTION] Flags & Enumerations
// [SECTION] Tables API flags and structures (RblxUITableFlags, RblxUITableColumnFlags, RblxUITableRowFlags, RblxUITableBgTarget, RblxUITableSortSpecs, RblxUITableColumnSortSpecs)
// [SECTION] Helpers: Debug log, Memory allocations macros, RblxVector<>
// [SECTION] RblxUIStyle
// [SECTION] RblxUIIO
// [SECTION] Misc data structures (RblxUIInputTextCallbackData, RblxUISizeCallbackData, RblxUIWindowClass, RblxUIPayload)
// [SECTION] Helpers (RblxUIOnceUponAFrame, RblxUITextFilter, RblxUITextBuffer, RblxUIStorage, RblxUIListClipper, Math Operators, RblxColor)
// [SECTION] Multi-Select API flags and structures (RblxUIMultiSelectFlags, RblxUIMultiSelectIO, RblxUISelectionRequest, RblxUISelectionBasicStorage, RblxUISelectionExternalStorage)
// [SECTION] Drawing API (RblxDrawCallback, RblxDrawCmd, RblxDrawIdx, RblxDrawVert, RblxDrawChannel, RblxDrawListSplitter, RblxDrawFlags, RblxDrawListFlags, RblxDrawList, RblxDrawData)
// [SECTION] Texture API (RblxTextureFormat, RblxTextureStatus, RblxTextureRect, RblxTextureData)
// [SECTION] Font API (RblxFontConfig, RblxFontGlyph, RblxFontGlyphRangesBuilder, RblxFontAtlasFlags, RblxFontAtlas, RblxFontBaked, RblxFont)
// [SECTION] Viewports (RblxUIViewportFlags, RblxUIViewport)
// [SECTION] RblxUIPlatformIO + other Platform Dependent Interfaces (RblxUIPlatformMonitor, RblxUIPlatformImeData)
// [SECTION] Obsolete functions and types

*/

#pragma once

// Configuration file with compile-time options
// (edit rblxuiconfig.h or '#define RBLXUI_USER_CONFIG "myfilename.h" from your build system)
#ifdef RBLXUI_USER_CONFIG
#include RBLXUI_USER_CONFIG
#endif
#include "rblxuiconfig.h"

#ifndef RBLXUI_DISABLE

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

// Includes
#include <float.h>                  // FLT_MIN, FLT_MAX
#include <stdarg.h>                 // va_list, va_start, va_end
#include <stddef.h>                 // ptrdiff_t, NULL
#include <string.h>                 // memset, memmove, memcpy, strlen, strchr, strcpy, strcmp

// Define attributes of all API symbols declarations (e.g. for DLL under Windows)
// RBLXUI_API is used for core rblxui functions, RBLXUI_IMPL_API is used for the default backends files (rblxui_impl_xxx.h)
// Using dear rblxui via a shared library is not recommended: we don't guarantee backward nor forward ABI compatibility + this is a call-heavy library and function call overhead adds up.
#ifndef RBLXUI_API
#define RBLXUI_API
#endif
#ifndef RBLXUI_IMPL_API
#define RBLXUI_IMPL_API              RBLXUI_API
#endif

// Helper Macros
// (note: compiling with NDEBUG will usually strip out assert() to nothing, which is NOT recommended because we use asserts to notify of programmer mistakes.)
#ifndef RBLXUI_ASSERT
#include <assert.h>
#define RBLXUI_ASSERT(_EXPR)            assert(_EXPR)                               // You can override the default assert handler by editing rblxuiconfig.h
#endif
#define RBLXUI_COUNTOF(_ARR)            ((int)(sizeof(_ARR) / sizeof(*(_ARR))))     // Size of a static C-style array. Don't use on pointers!
#define RBLXUI_UNUSED(_VAR)             ((void)(_VAR))                              // Used to silence "unused variable warnings". Often useful as asserts may be stripped out from final builds.
#define RBLXUI_STRINGIFY_HELPER(_EXPR)  #_EXPR
#define RBLXUI_STRINGIFY(_EXPR)         RBLXUI_STRINGIFY_HELPER(_EXPR)                  // Preprocessor idiom to stringify e.g. an integer or a macro.

// Check that version and structures layouts are matching between compiled rblxui code and caller. Read comments above DebugCheckVersionAndDataLayout() for details.
#define RBLXUI_CHECKVERSION()        RblxUI::DebugCheckVersionAndDataLayout(RBLXUI_VERSION, sizeof(RblxUIIO), sizeof(RblxUIStyle), sizeof(RblxVec2), sizeof(RblxVec4), sizeof(RblxDrawVert), sizeof(RblxDrawIdx))

// Helper Macros - RBLXUI_FMTARGS, RBLXUI_FMTLIST: Apply printf-style warnings to our formatting functions.
// (MSVC provides an equivalent mechanism via SAL Annotations but it requires the macros in a different
//  location. e.g. #include <sal.h> + void myprintf(_Printf_format_string_ const char* format, ...),
//  and only works when using Code Analysis, rather than just normal compiling).
// (see https://github.com/ocornut/rblxui/issues/8871 for a patch to enable this for MSVC's Code Analysis)
#if !defined(RBLXUI_USE_STB_SPRINTF) && defined(__MINGW32__) && !defined(__clang__)
#define RBLXUI_FMTARGS(FMT)             __attribute__((format(gnu_printf, FMT, FMT+1)))
#define RBLXUI_FMTLIST(FMT)             __attribute__((format(gnu_printf, FMT, 0)))
#elif !defined(RBLXUI_USE_STB_SPRINTF) && (defined(__clang__) || defined(__GNUC__))
#define RBLXUI_FMTARGS(FMT)             __attribute__((format(printf, FMT, FMT+1)))
#define RBLXUI_FMTLIST(FMT)             __attribute__((format(printf, FMT, 0)))
#else
#define RBLXUI_FMTARGS(FMT)
#define RBLXUI_FMTLIST(FMT)
#endif

// Disable some of MSVC most aggressive Debug runtime checks in function header/footer (used in some simple/low-level functions)
#if defined(_MSC_VER) && !defined(__clang__)  && !defined(__INTEL_COMPILER) && !defined(RBLXUI_DEBUG_PARANOID)
#define RBLXUI_MSVC_RUNTIME_CHECKS_OFF      __pragma(runtime_checks("",off))     __pragma(check_stack(off)) __pragma(strict_gs_check(push,off))
#define RBLXUI_MSVC_RUNTIME_CHECKS_RESTORE  __pragma(runtime_checks("",restore)) __pragma(check_stack())    __pragma(strict_gs_check(pop))
#else
#define RBLXUI_MSVC_RUNTIME_CHECKS_OFF
#define RBLXUI_MSVC_RUNTIME_CHECKS_RESTORE
#endif

// Warnings
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 26495)    // [Static Analyzer] Variable 'XXX' is uninitialized. Always initialize a member variable (type.6).
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast
#pragma clang diagnostic ignored "-Wfloat-equal"                    // warning: comparing floating point with == or != is unsafe
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant
#pragma clang diagnostic ignored "-Wreserved-identifier"            // warning: identifier '_Xxx' is reserved because it starts with '_' followed by a capital letter
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"            // warning: 'xxx' is an unsafe pointer used for buffer access
#pragma clang diagnostic ignored "-Wnontrivial-memaccess"           // warning: first argument in call to 'memset' is a pointer to non-trivially copyable type
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"                          // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wfloat-equal"                      // warning: comparing floating-point with '==' or '!=' is unsafe
#pragma GCC diagnostic ignored "-Wclass-memaccess"                  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------

// Scalar data types
typedef unsigned int        RblxUIID;// A unique ID used by widgets (typically the result of hashing a stack of string)
typedef signed char         RblxS8;   // 8-bit signed integer
typedef unsigned char       RblxU8;   // 8-bit unsigned integer
typedef signed short        RblxS16;  // 16-bit signed integer
typedef unsigned short      RblxU16;  // 16-bit unsigned integer
typedef signed int          RblxS32;  // 32-bit signed integer == int
typedef unsigned int        RblxU32;  // 32-bit unsigned integer (often used to store packed colors)
typedef signed   long long  RblxS64;  // 64-bit signed integer
typedef unsigned long long  RblxU64;  // 64-bit unsigned integer

// Forward declarations: RblxDrawList, RblxFontAtlas layer
struct RblxDrawChannel;               // Temporary storage to output draw commands out of order, used by RblxDrawListSplitter and RblxDrawList::ChannelsSplit()
struct RblxDrawCmd;                   // A single draw command within a parent RblxDrawList (generally maps to 1 GPU draw call, unless it is a callback)
struct RblxDrawData;                  // All draw command lists required to render the frame + pos/size coordinates to use for the projection matrix.
struct RblxDrawList;                  // A single draw command list (generally one per window, conceptually you may see this as a dynamic "mesh" builder)
struct RblxDrawListSharedData;        // Data shared among multiple draw lists (typically owned by parent RblxUI context, but you may create one yourself)
struct RblxDrawListSplitter;          // Helper to split a draw list into different layers which can be drawn into out of order, then flattened back.
struct RblxDrawVert;                  // A single vertex (pos + uv + col = 20 bytes by default. Override layout with RBLXUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT)
struct RblxFont;                      // Runtime data for a single font within a parent RblxFontAtlas
struct RblxFontAtlas;                 // Runtime data for multiple fonts, bake multiple fonts into a single texture, TTF/OTF font loader
struct RblxFontAtlasBuilder;          // Opaque storage for building a RblxFontAtlas
struct RblxFontAtlasRect;             // Output of RblxFontAtlas::GetCustomRect() when using custom rectangles.
struct RblxFontBaked;                 // Baked data for a RblxFont at a given size.
struct RblxFontConfig;                // Configuration data when adding a font or merging fonts
struct RblxFontGlyph;                 // A single font glyph (code point + coordinates within in RblxFontAtlas + offset)
struct RblxFontGlyphRangesBuilder;    // Helper to build glyph ranges from text/string data
struct RblxFontLoader;                // Opaque interface to a font loading backend (rblxstb_truetype, FreeType etc.).
struct RblxTextureData;               // Specs and pixel storage for a texture used by Dear RblxUI.
struct RblxTextureRect;               // Coordinates of a rectangle within a texture.
struct RblxColor;                     // Helper functions to create a color that can be converted to either u32 or float4 (*OBSOLETE* please avoid using)

// Forward declarations: RblxUI layer
struct RblxUIContext;                // Dear RblxUI context (opaque structure, unless including rblxui_internal.h)
struct RblxUIIO;                     // Main configuration and I/O between your application and RblxUI (also see: RblxUIPlatformIO)
struct RblxUIInputTextCallbackData;  // Shared state of InputText() when using custom RblxUIInputTextCallback (rare/advanced use)
struct RblxUIKeyData;                // Storage for RblxUIIO and IsKeyDown(), IsKeyPressed() etc functions.
struct RblxUIListClipper;            // Helper to manually clip large list of items
struct RblxUIMultiSelectIO;          // Structure to interact with a BeginMultiSelect()/EndMultiSelect() block
struct RblxUIOnceUponAFrame;         // Helper for running a block of code not more than once a frame
struct RblxUIPayload;                // User data payload for drag and drop operations
struct RblxUIPlatformIO;             // Interface between platform/renderer backends and RblxUI (e.g. Clipboard, IME, Multi-Viewport support). Extends RblxUIIO.
struct RblxUIPlatformImeData;        // Platform IME data for io.PlatformSetImeDataFn() function.
struct RblxUIPlatformMonitor;        // Multi-viewport support: user-provided bounds for each connected monitor/display. Used when positioning popups and tooltips to avoid them straddling monitors
struct RblxUISelectionBasicStorage;  // Optional helper to store multi-selection state + apply multi-selection requests.
struct RblxUISelectionExternalStorage;//Optional helper to apply multi-selection requests to existing randomly accessible storage.
struct RblxUISelectionRequest;       // A selection request (stored in RblxUIMultiSelectIO)
struct RblxUISizeCallbackData;       // Callback data when using SetNextWindowSizeConstraints() (rare/advanced use)
struct RblxUIStorage;                // Helper for key->value storage (container sorted by key)
struct RblxUIStoragePair;            // Helper for key->value storage (pair)
struct RblxUIStyle;                  // Runtime data for styling/colors
struct RblxUITableSortSpecs;         // Sorting specifications for a table (often handling sort specs for a single column, occasionally more)
struct RblxUITableColumnSortSpecs;   // Sorting specification for one column of a table
struct RblxUITextBuffer;             // Helper to hold and append into a text buffer (~string builder)
struct RblxUITextFilter;             // Helper to parse and apply text filters (e.g. "aaaaa[,bbbbb][,ccccc]")
struct RblxUIViewport;               // A Platform Window (always 1 unless multi-viewport are enabled. One per platform window to output to). In the future may represent Platform Monitor
struct RblxUIWindowClass;            // Window class (rare/advanced uses: provide hints to the platform backend via altered viewport flags and parent/child info)

// Enumerations
// - We don't use strongly typed enums much because they add constraints (can't extend in private code, can't store typed in bit fields, extra casting on iteration)
// - Tip: Use your programming IDE navigation facilities on the names in the _central column_ below to find the actual flags/enum lists!
//   - In Visual Studio: Ctrl+Comma ("Edit.GoToAll") can follow symbols inside comments, whereas Ctrl+F12 ("Edit.GoToImplementation") cannot.
//   - In Visual Studio w/ Visual Assist installed: Alt+G ("VAssistX.GoToImplementation") can also follow symbols inside comments.
//   - In VS Code, CLion, etc.: Ctrl+Click can follow symbols inside comments.
enum RblxUIDir : int;                // -> enum RblxUIDir              // Enum: A cardinal direction (Left, Right, Up, Down)
enum RblxUIKey : int;                // -> enum RblxUIKey              // Enum: A key identifier (RblxUIKey_XXX or RblxUIMod_XXX value)
enum RblxUIMouseSource : int;        // -> enum RblxUIMouseSource      // Enum; A mouse input source identifier (Mouse, TouchScreen, Pen)
enum RblxUISortDirection : RblxU8;     // -> enum RblxUISortDirection    // Enum: A sorting direction (ascending or descending)
typedef int RblxUICol;               // -> enum RblxUICol_             // Enum: A color identifier for styling
typedef int RblxUICond;              // -> enum RblxUICond_            // Enum: A condition for many Set*() functions
typedef int RblxUIDataType;          // -> enum RblxUIDataType_        // Enum: A primary data type
typedef int RblxUIMouseButton;       // -> enum RblxUIMouseButton_     // Enum: A mouse button identifier (0=left, 1=right, 2=middle)
typedef int RblxUIMouseCursor;       // -> enum RblxUIMouseCursor_     // Enum: A mouse cursor shape
typedef int RblxUIStyleVar;          // -> enum RblxUIStyleVar_        // Enum: A variable identifier for styling
typedef int RblxUITableBgTarget;     // -> enum RblxUITableBgTarget_   // Enum: A color target for TableSetBgColor()

// Flags (declared as int to allow using as flags without overhead, and to not pollute the top of this file)
// - Tip: Use your programming IDE navigation facilities on the names in the _central column_ below to find the actual flags/enum lists!
//   - In Visual Studio: Ctrl+Comma ("Edit.GoToAll") can follow symbols inside comments, whereas Ctrl+F12 ("Edit.GoToImplementation") cannot.
//   - In Visual Studio w/ Visual Assist installed: Alt+G ("VAssistX.GoToImplementation") can also follow symbols inside comments.
//   - In VS Code, CLion, etc.: Ctrl+Click can follow symbols inside comments.
typedef int RblxDrawFlags;            // -> enum RblxDrawFlags_          // Flags: for RblxDrawList functions
typedef int RblxDrawListFlags;        // -> enum RblxDrawListFlags_      // Flags: for RblxDrawList instance
typedef int RblxDrawTextFlags;        // -> enum RblxDrawTextFlags_      // Internal, do not use!
typedef int RblxFontFlags;            // -> enum RblxFontFlags_          // Flags: for RblxFont
typedef int RblxFontAtlasFlags;       // -> enum RblxFontAtlasFlags_     // Flags: for RblxFontAtlas
typedef int RblxUIBackendFlags;      // -> enum RblxUIBackendFlags_    // Flags: for io.BackendFlags
typedef int RblxUIButtonFlags;       // -> enum RblxUIButtonFlags_     // Flags: for InvisibleButton()
typedef int RblxUIChildFlags;        // -> enum RblxUIChildFlags_      // Flags: for BeginChild()
typedef int RblxUIColorEditFlags;    // -> enum RblxUIColorEditFlags_  // Flags: for ColorEdit4(), ColorPicker4() etc.
typedef int RblxUIConfigFlags;       // -> enum RblxUIConfigFlags_     // Flags: for io.ConfigFlags
typedef int RblxUIComboFlags;        // -> enum RblxUIComboFlags_      // Flags: for BeginCombo()
typedef int RblxUIDockNodeFlags;     // -> enum RblxUIDockNodeFlags_   // Flags: for DockSpace()
typedef int RblxUIDragDropFlags;     // -> enum RblxUIDragDropFlags_   // Flags: for BeginDragDropSource(), AcceptDragDropPayload()
typedef int RblxUIFocusedFlags;      // -> enum RblxUIFocusedFlags_    // Flags: for IsWindowFocused()
typedef int RblxUIHoveredFlags;      // -> enum RblxUIHoveredFlags_    // Flags: for IsItemHovered(), IsWindowHovered() etc.
typedef int RblxUIInputFlags;        // -> enum RblxUIInputFlags_      // Flags: for Shortcut(), SetNextItemShortcut()
typedef int RblxUIInputTextFlags;    // -> enum RblxUIInputTextFlags_  // Flags: for InputText(), InputTextMultiline()
typedef int RblxUIItemFlags;         // -> enum RblxUIItemFlags_       // Flags: for PushItemFlag(), shared by all items
typedef int RblxUIKeyChord;          // -> RblxUIKey | RblxUIMod_XXX    // Flags: for IsKeyChordPressed(), Shortcut() etc. an RblxUIKey optionally OR-ed with one or more RblxUIMod_XXX values.
typedef int RblxUIListClipperFlags;  // -> enum RblxUIListClipperFlags_// Flags: for RblxUIListClipper
typedef int RblxUIPopupFlags;        // -> enum RblxUIPopupFlags_      // Flags: for OpenPopup*(), BeginPopupContext*(), IsPopupOpen()
typedef int RblxUIMultiSelectFlags;  // -> enum RblxUIMultiSelectFlags_// Flags: for BeginMultiSelect()
typedef int RblxUISelectableFlags;   // -> enum RblxUISelectableFlags_ // Flags: for Selectable()
typedef int RblxUISliderFlags;       // -> enum RblxUISliderFlags_     // Flags: for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
typedef int RblxUITabBarFlags;       // -> enum RblxUITabBarFlags_     // Flags: for BeginTabBar()
typedef int RblxUITabItemFlags;      // -> enum RblxUITabItemFlags_    // Flags: for BeginTabItem()
typedef int RblxUITableFlags;        // -> enum RblxUITableFlags_      // Flags: For BeginTable()
typedef int RblxUITableColumnFlags;  // -> enum RblxUITableColumnFlags_// Flags: For TableSetupColumn()
typedef int RblxUITableRowFlags;     // -> enum RblxUITableRowFlags_   // Flags: For TableNextRow()
typedef int RblxUITreeNodeFlags;     // -> enum RblxUITreeNodeFlags_   // Flags: for TreeNode(), TreeNodeEx(), CollapsingHeader()
typedef int RblxUIViewportFlags;     // -> enum RblxUIViewportFlags_   // Flags: for RblxUIViewport
typedef int RblxUIWindowFlags;       // -> enum RblxUIWindowFlags_     // Flags: for Begin(), BeginChild()

// Character types
// (we generally use UTF-8 encoded string in the API. This is storage specifically for a decoded character used for keyboard input and display)
typedef unsigned int RblxWchar32;     // A single decoded U32 character/code point. We encode them as multi bytes UTF-8 when used in strings.
typedef unsigned short RblxWchar16;   // A single decoded U16 character/code point. We encode them as multi bytes UTF-8 when used in strings.
#ifdef RBLXUI_USE_WCHAR32            // RblxWchar [configurable type: override in rblxuiconfig.h with '#define RBLXUI_USE_WCHAR32' to support Unicode planes 1-16]
typedef RblxWchar32 RblxWchar;
#else
typedef RblxWchar16 RblxWchar;
#endif

// Multi-Selection item index or identifier when using BeginMultiSelect()
// - Used by SetNextItemSelectionUserData() + and inside RblxUIMultiSelectIO structure.
// - Most users are likely to use this store an item INDEX but this may be used to store a POINTER/ID as well. Read comments near RblxUIMultiSelectIO for details.
typedef RblxS64 RblxUISelectionUserData;

// Callback and functions types
typedef int     (*RblxUIInputTextCallback)(RblxUIInputTextCallbackData* data);    // Callback function for RblxUI::InputText()
typedef void    (*RblxUISizeCallback)(RblxUISizeCallbackData* data);              // Callback function for RblxUI::SetNextWindowSizeConstraints()
typedef void*   (*RblxUIMemAllocFunc)(size_t sz, void* user_data);               // Function signature for RblxUI::SetAllocatorFunctions()
typedef void    (*RblxUIMemFreeFunc)(void* ptr, void* user_data);                // Function signature for RblxUI::SetAllocatorFunctions()

// RblxVec2: 2D vector used to store positions, sizes etc. [Compile-time configurable type]
// - This is a frequently used type in the API. Consider using RBLXUI_VEC2_CLASS_EXTRA to create implicit cast from/to our preferred type.
// - Add '#define RBLXUI_DEFINE_MATH_OPERATORS' before including this file (or in rblxuiconfig.h) to access courtesy maths operators for RblxVec2 and RblxVec4.
RBLXUI_MSVC_RUNTIME_CHECKS_OFF
struct RblxVec2
{
    float                                   x, y;
    constexpr RblxVec2()                      : x(0.0f), y(0.0f) { }
    constexpr RblxVec2(float _x, float _y)    : x(_x), y(_y) { }
    float& operator[] (size_t idx)          { RBLXUI_ASSERT(idx == 0 || idx == 1); return ((float*)(void*)(char*)this)[idx]; } // We very rarely use this [] operator, so the assert overhead is fine.
    float  operator[] (size_t idx) const    { RBLXUI_ASSERT(idx == 0 || idx == 1); return ((const float*)(const void*)(const char*)this)[idx]; }
#ifdef RBLXUI_VEC2_CLASS_EXTRA
    RBLXUI_VEC2_CLASS_EXTRA     // Define additional constructors and implicit cast operators in rblxuiconfig.h to convert back and forth between your math types and RblxVec2.
#endif
};

// RblxVec4: 4D vector used to store clipping rectangles, colors etc. [Compile-time configurable type]
struct RblxVec4
{
    float                                                     x, y, z, w;
    constexpr RblxVec4()                                        : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
    constexpr RblxVec4(float _x, float _y, float _z, float _w)  : x(_x), y(_y), z(_z), w(_w) { }
#ifdef RBLXUI_VEC4_CLASS_EXTRA
    RBLXUI_VEC4_CLASS_EXTRA     // Define additional constructors and implicit cast operators in rblxuiconfig.h to convert back and forth between your math types and RblxVec4.
#endif
};
RBLXUI_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] Texture identifiers (RblxTextureID, RblxTextureRef)
//-----------------------------------------------------------------------------

// RblxTextureID = backend specific, low-level identifier for a texture uploaded in GPU/graphics system.
// [Compile-time configurable type]
// - When a Rendered Backend creates a texture, it store its native identifier into a RblxTextureID value.
//   (e.g. Used by DX11 backend to a `ID3D11ShaderResourceView*`; Used by OpenGL backends to store `GLuint`;
//         Used by SDLGPU backend to store a `SDL_GPUTextureSamplerBinding*`, etc.).
// - User may submit their own textures to e.g. RblxUI::Image() function by passing this value.
// - During the rendering loop, the Renderer Backend retrieve the RblxTextureID, which stored inside a
//   RblxTextureRef, which is stored inside a RblxDrawCmd.
// - Compile-time type configuration:
//   - To use something other than a 64-bit value: add '#define RblxTextureID MyTextureType*' in your rblxuiconfig.h file.
//   - This can be whatever to you want it to be! read the FAQ entry about textures for details.
//   - You may decide to store a higher-level structure containing texture, sampler, shader etc. with various
//     constructors if you like. You will need to implement ==/!= operators.
// History:
// - In v1.91.4 (2024/10/08): the default type for RblxTextureID was changed from 'void*' to 'RblxU64'. This allowed backends requiring 64-bit worth of data to build on 32-bit architectures. Use intermediary intptr_t cast and read FAQ if you have casting warnings.
// - In v1.92.0 (2025/06/11): added RblxTextureRef which carry either a RblxTextureID either a pointer to internal texture atlas. All user facing functions taking RblxTextureID changed to RblxTextureRef
#ifndef RblxTextureID
typedef RblxU64 RblxTextureID;      // Default: store up to 64-bits (any pointer or integer). A majority of backends are ok with that.
#endif

// Define this if you need to change the invalid value for your backend.
// - If your backend is using RblxTextureID to store an index/offset and you need 0 to be valid, You can add '#define RblxTextureID_Invalid ((RblxTextureID)-1)' in your rblxuiconfig.h file.
// - From 2026/03/12 to 2026/03/19 we experimented with changing to default to -1, but I worried it would cause too many issues in third-party code so it was reverted.
#ifndef RblxTextureID_Invalid
#define RblxTextureID_Invalid     ((RblxTextureID)0)
#endif

// RblxTextureRef = higher-level identifier for a texture. Store a RblxTextureID _or_ a RblxTextureData*.
// The identifier is valid even before the texture has been uploaded to the GPU/graphics system.
// This is what gets passed to functions such as `RblxUI::Image()`, `RblxDrawList::AddImage()`.
// This is what gets stored in draw commands (`RblxDrawCmd`) to identify a texture during rendering.
// - When a texture is created by user code (e.g. custom images), we directly store the low-level RblxTextureID.
//   Because of this, when displaying your own texture you are likely to ever only manage RblxTextureID values on your side.
// - When a texture is created by the backend, we stores a RblxTextureData* which becomes an indirection
//   to extract the RblxTextureID value during rendering, after texture upload has happened.
// - To create a RblxTextureRef from a RblxTextureData you can use RblxTextureData::GetTexRef().
//   We intentionally do not provide an RblxTextureRef constructor for this: we don't expect this
//   to be frequently useful to the end-user, and it would be erroneously called by many legacy code.
// - If you want to bind the current atlas when using custom rectangle, you can use io.Fonts->TexRef.
// - Binding generators for languages such as C (which don't have constructors), should provide a helper, e.g.
//      inline RblxTextureRef RblxTextureRefFromID(RblxTextureID tex_id) { RblxTextureRef tex_ref = { ._TexData = NULL, .TexID = tex_id }; return tex_ref; }
// In 1.92 we changed most drawing functions using RblxTextureID to use RblxTextureRef.
// We intentionally do not provide an implicit RblxTextureRef -> RblxTextureID cast operator because it is technically lossy to convert RblxTextureRef to RblxTextureID before rendering.
RBLXUI_MSVC_RUNTIME_CHECKS_OFF
struct RblxTextureRef
{
    RblxTextureRef()                          { _TexData = NULL; _TexID = RblxTextureID_Invalid; }
    RblxTextureRef(RblxTextureID tex_id)        { _TexData = NULL; _TexID = tex_id; }
#if !defined(RBLXUI_DISABLE_OBSOLETE_FUNCTIONS) && !defined(RblxTextureID)
    RblxTextureRef(void* tex_id)              { _TexData = NULL; _TexID = (RblxTextureID)(size_t)tex_id; }  // For legacy backends casting to RblxTextureID
#endif

    inline RblxTextureID  GetTexID() const;   // == (_TexData ? _TexData->TexID : _TexID) // Implemented below in the file.

    // Members (either are set, never both!)
    RblxTextureData*      _TexData;           //      A texture, generally owned by a RblxFontAtlas. Will convert to RblxTextureID during render loop, after texture has been uploaded.
    RblxTextureID         _TexID;             // _OR_ Low-level backend texture identifier, if already uploaded or created by user/app. Generally provided to e.g. RblxUI::Image() calls.
};
RBLXUI_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] Dear RblxUI end-user API functions
// (Note that RblxUI:: being a namespace, you can add extra RblxUI:: functions in your own separate file. Please don't modify rblxui source files!)
//-----------------------------------------------------------------------------

namespace RblxUI
{
    // Context creation and access
    // - Each context create its own RblxFontAtlas by default. You may instance one yourself and pass it to CreateContext() to share a font atlas between contexts.
    // - DLL users: heaps and globals are not shared across DLL boundaries! You will need to call SetCurrentContext() + SetAllocatorFunctions()
    //   for each static/DLL boundary you are calling from. Read "Context and Memory Allocators" section of rblxui.cpp for details.
    RBLXUI_API RblxUIContext* CreateContext(RblxFontAtlas* shared_font_atlas = NULL);
    RBLXUI_API void          DestroyContext(RblxUIContext* ctx = NULL);   // NULL = destroy current context
    RBLXUI_API RblxUIContext* GetCurrentContext();
    RBLXUI_API void          SetCurrentContext(RblxUIContext* ctx);

    // Main
    RBLXUI_API RblxUIIO&      GetIO();                                    // access the RblxUIIO structure (mouse/keyboard/gamepad inputs, time, various configuration options/flags)
    RBLXUI_API RblxUIPlatformIO& GetPlatformIO();                         // access the RblxUIPlatformIO structure (mostly hooks/functions to connect to platform/renderer and OS Clipboard, IME etc.)
    RBLXUI_API RblxUIStyle&   GetStyle();                                 // access the Style structure (colors, sizes). Always use PushStyleColor(), PushStyleVar() to modify style mid-frame!
    RBLXUI_API void          NewFrame();                                 // start a new Dear RblxUI frame, you can submit any command from this point until Render()/EndFrame().
    RBLXUI_API void          EndFrame();                                 // ends the Dear RblxUI frame. automatically called by Render(). If you don't need to render data (skipping rendering) you may call EndFrame() without Render()... but you'll have wasted CPU already! If you don't need to render, better to not create any windows and not call NewFrame() at all!
    RBLXUI_API void          Render();                                   // ends the Dear RblxUI frame, finalize the draw data. You can then get call GetDrawData().
    RBLXUI_API RblxDrawData*   GetDrawData();                              // valid after Render() and until the next call to NewFrame(). Call RblxUI_ImplXXXX_RenderDrawData() function in your Renderer Backend to render.

    // Demo, Debug, Information
    RBLXUI_API void          ShowDemoWindow(bool* p_open = NULL);        // create Demo window. demonstrate most RblxUI features. call this to learn about the library! try to make it always available in your application!
    RBLXUI_API void          ShowMetricsWindow(bool* p_open = NULL);     // create Metrics/Debugger window. display Dear RblxUI internals: windows, draw commands, various internal state, etc.
    RBLXUI_API void          ShowDebugLogWindow(bool* p_open = NULL);    // create Debug Log window. display a simplified log of important dear rblxui events.
    RBLXUI_API void          ShowIDStackToolWindow(bool* p_open = NULL); // create Stack Tool window. hover items with mouse to query information about the source of their unique ID.
    RBLXUI_API void          ShowAboutWindow(bool* p_open = NULL);       // create About window. display Dear RblxUI version, credits and build/system information.
    RBLXUI_API void          ShowStyleEditor(RblxUIStyle* ref = NULL);    // add style editor block (not a window). you can pass in a reference RblxUIStyle structure to compare to, revert to and save to (else it uses the default style)
    RBLXUI_API bool          ShowStyleSelector(const char* label);       // add style selector block (not a window), essentially a combo listing the default styles.
    RBLXUI_API void          ShowFontSelector(const char* label);        // add font selector block (not a window), essentially a combo listing the loaded fonts.
    RBLXUI_API void          ShowUserGuide();                            // add basic help/info block (not a window): how to manipulate RblxUI as an end-user (mouse/keyboard controls).
    RBLXUI_API const char*   GetVersion();                               // get the compiled version string e.g. "1.80 WIP" (essentially the value for RBLXUI_VERSION from the compiled version of rblxui.cpp)

    // Styles
    RBLXUI_API void          StyleColorsDark(RblxUIStyle* dst = NULL);    // new, recommended style (default)
    RBLXUI_API void          StyleColorsLight(RblxUIStyle* dst = NULL);   // best used with borders and a custom, thicker font
    RBLXUI_API void          StyleColorsClassic(RblxUIStyle* dst = NULL); // classic rblxui style

    // Windows
    // - Begin() = push window to the stack and start appending to it. End() = pop window from the stack.
    // - Passing 'bool* p_open != NULL' shows a window-closing widget in the upper-right corner of the window,
    //   which clicking will set the boolean to false when clicked.
    // - You may append multiple times to the same window during the same frame by calling Begin()/End() pairs multiple times.
    //   Some information such as 'flags' or 'p_open' will only be considered by the first call to Begin().
    // - Begin() return false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting
    //   anything to the window. Always call a matching End() for each Begin() call, regardless of its return value!
    //   [Important: due to legacy reason, Begin/End and BeginChild/EndChild are inconsistent with all other functions
    //    such as BeginMenu/EndMenu, BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding
    //    BeginXXX function returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
    // - Note that the bottom of window stack always contains a window called "Debug".
    RBLXUI_API bool          Begin(const char* name, bool* p_open = NULL, RblxUIWindowFlags flags = 0);
    RBLXUI_API void          End();

    // Child Windows
    // - Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window. Child windows can embed their own child.
    // - Before 1.90 (November 2023), the "RblxUIChildFlags child_flags = 0" parameter was "bool border = false".
    //   This API is backward compatible with old code, as we guarantee that RblxUIChildFlags_Borders == true.
    //   Consider updating your old code:
    //      BeginChild("Name", size, false)   -> Begin("Name", size, 0); or Begin("Name", size, RblxUIChildFlags_None);
    //      BeginChild("Name", size, true)    -> Begin("Name", size, RblxUIChildFlags_Borders);
    // - Manual sizing (each axis can use a different setting e.g. RblxVec2(0.0f, 400.0f)):
    //     == 0.0f: use remaining parent window size for this axis.
    //      > 0.0f: use specified size for this axis.
    //      < 0.0f: right/bottom-align to specified distance from available content boundaries.
    // - Specifying RblxUIChildFlags_AutoResizeX or RblxUIChildFlags_AutoResizeY makes the sizing automatic based on child contents.
    //   Combining both RblxUIChildFlags_AutoResizeX _and_ RblxUIChildFlags_AutoResizeY defeats purpose of a scrolling region and is NOT recommended.
    // - BeginChild() returns false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting
    //   anything to the window. Always call a matching EndChild() for each BeginChild() call, regardless of its return value.
    //   [Important: due to legacy reason, Begin/End and BeginChild/EndChild are inconsistent with all other functions
    //    such as BeginMenu/EndMenu, BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding
    //    BeginXXX function returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
    RBLXUI_API bool          BeginChild(const char* str_id, const RblxVec2& size = RblxVec2(0, 0), RblxUIChildFlags child_flags = 0, RblxUIWindowFlags window_flags = 0);
    RBLXUI_API bool          BeginChild(RblxUIID id, const RblxVec2& size = RblxVec2(0, 0), RblxUIChildFlags child_flags = 0, RblxUIWindowFlags window_flags = 0);
    RBLXUI_API void          EndChild();

    // Windows Utilities
    // - 'current window' = the window we are appending into while inside a Begin()/End() block. 'next window' = next window we will Begin() into.
    RBLXUI_API bool          IsWindowAppearing();
    RBLXUI_API bool          IsWindowCollapsed();
    RBLXUI_API bool          IsWindowFocused(RblxUIFocusedFlags flags=0); // is current window focused? or its root/child, depending on flags. see flags for options.
    RBLXUI_API bool          IsWindowHovered(RblxUIHoveredFlags flags=0); // is current window hovered and hoverable (e.g. not blocked by a popup/modal)? See RblxUIHoveredFlags_ for options. IMPORTANT: If you are trying to check whether your mouse should be dispatched to Dear RblxUI or to your underlying app, you should not use this function! Use the 'io.WantCaptureMouse' boolean for that! Refer to FAQ entry "How can I tell whether to dispatch mouse/keyboard to Dear RblxUI or my application?" for details.
    RBLXUI_API RblxDrawList*   GetWindowDrawList();                        // get draw list associated to the current window, to append your own drawing primitives
    RBLXUI_API float         GetWindowDpiScale();                        // get DPI scale currently associated to the current window's viewport.
    RBLXUI_API RblxVec2        GetWindowPos();                             // get current window position in screen space (IT IS UNLIKELY YOU EVER NEED TO USE THIS. Consider always using GetCursorScreenPos() and GetContentRegionAvail() instead)
    RBLXUI_API RblxVec2        GetWindowSize();                            // get current window size (IT IS UNLIKELY YOU EVER NEED TO USE THIS. Consider always using GetCursorScreenPos() and GetContentRegionAvail() instead)
    RBLXUI_API float         GetWindowWidth();                           // get current window width (IT IS UNLIKELY YOU EVER NEED TO USE THIS). Shortcut for GetWindowSize().x.
    RBLXUI_API float         GetWindowHeight();                          // get current window height (IT IS UNLIKELY YOU EVER NEED TO USE THIS). Shortcut for GetWindowSize().y.
    RBLXUI_API RblxUIViewport*GetWindowViewport();                        // get viewport currently associated to the current window.

    // Window manipulation
    // - Prefer using SetNextXXX functions (before Begin) rather that SetXXX functions (after Begin).
    RBLXUI_API void          SetNextWindowPos(const RblxVec2& pos, RblxUICond cond = 0, const RblxVec2& pivot = RblxVec2(0, 0)); // set next window position. call before Begin(). use pivot=(0.5f,0.5f) to center on given point, etc.
    RBLXUI_API void          SetNextWindowSize(const RblxVec2& size, RblxUICond cond = 0);                  // set next window size. set axis to 0.0f to force an auto-fit on this axis. call before Begin()
    RBLXUI_API void          SetNextWindowSizeConstraints(const RblxVec2& size_min, const RblxVec2& size_max, RblxUISizeCallback custom_callback = NULL, void* custom_callback_data = NULL); // set next window size limits. use 0.0f or FLT_MAX if you don't want limits. Use -1 for both min and max of same axis to preserve current size (which itself is a constraint). Use callback to apply non-trivial programmatic constraints.
    RBLXUI_API void          SetNextWindowContentSize(const RblxVec2& size);                               // set next window content size (~ scrollable client area, which enforce the range of scrollbars). Not including window decorations (title bar, menu bar, etc.) nor WindowPadding. set an axis to 0.0f to leave it automatic. call before Begin()
    RBLXUI_API void          SetNextWindowCollapsed(bool collapsed, RblxUICond cond = 0);                 // set next window collapsed state. call before Begin()
    RBLXUI_API void          SetNextWindowFocus();                                                       // set next window to be focused / top-most. call before Begin()
    RBLXUI_API void          SetNextWindowScroll(const RblxVec2& scroll);                                  // set next window scrolling value (use < 0.0f to not affect a given axis).
    RBLXUI_API void          SetNextWindowBgAlpha(float alpha);                                          // set next window background color alpha. helper to easily override the Alpha component of RblxUICol_WindowBg/ChildBg/PopupBg. you may also use RblxUIWindowFlags_NoBackground.
    RBLXUI_API void          SetNextWindowViewport(RblxUIID viewport_id);                                 // set next window viewport
    RBLXUI_API void          SetWindowPos(const RblxVec2& pos, RblxUICond cond = 0);                        // (not recommended) set current window position - call within Begin()/End(). prefer using SetNextWindowPos(), as this may incur tearing and side-effects.
    RBLXUI_API void          SetWindowSize(const RblxVec2& size, RblxUICond cond = 0);                      // (not recommended) set current window size - call within Begin()/End(). set to RblxVec2(0, 0) to force an auto-fit. prefer using SetNextWindowSize(), as this may incur tearing and minor side-effects.
    RBLXUI_API void          SetWindowCollapsed(bool collapsed, RblxUICond cond = 0);                     // (not recommended) set current window collapsed state. prefer using SetNextWindowCollapsed().
    RBLXUI_API void          SetWindowFocus();                                                           // (not recommended) set current window to be focused / top-most. prefer using SetNextWindowFocus().
    RBLXUI_API void          SetWindowPos(const char* name, const RblxVec2& pos, RblxUICond cond = 0);      // set named window position.
    RBLXUI_API void          SetWindowSize(const char* name, const RblxVec2& size, RblxUICond cond = 0);    // set named window size. set axis to 0.0f to force an auto-fit on this axis.
    RBLXUI_API void          SetWindowCollapsed(const char* name, bool collapsed, RblxUICond cond = 0);   // set named window collapsed state
    RBLXUI_API void          SetWindowFocus(const char* name);                                           // set named window to be focused / top-most. use NULL to remove focus.

    // Windows Scrolling
    // - Any change of Scroll will be applied at the beginning of next frame in the first call to Begin().
    // - You may instead use SetNextWindowScroll() prior to calling Begin() to avoid this delay, as an alternative to using SetScrollX()/SetScrollY().
    RBLXUI_API float         GetScrollX();                                                   // get scrolling amount [0 .. GetScrollMaxX()]
    RBLXUI_API float         GetScrollY();                                                   // get scrolling amount [0 .. GetScrollMaxY()]
    RBLXUI_API void          SetScrollX(float scroll_x);                                     // set scrolling amount [0 .. GetScrollMaxX()]
    RBLXUI_API void          SetScrollY(float scroll_y);                                     // set scrolling amount [0 .. GetScrollMaxY()]
    RBLXUI_API float         GetScrollMaxX();                                                // get maximum scrolling amount ~~ ContentSize.x - WindowSize.x - DecorationsSize.x
    RBLXUI_API float         GetScrollMaxY();                                                // get maximum scrolling amount ~~ ContentSize.y - WindowSize.y - DecorationsSize.y
    RBLXUI_API void          SetScrollHereX(float center_x_ratio = 0.5f);                    // adjust scrolling amount to make current cursor position visible. center_x_ratio=0.0: left, 0.5: center, 1.0: right. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.
    RBLXUI_API void          SetScrollHereY(float center_y_ratio = 0.5f);                    // adjust scrolling amount to make current cursor position visible. center_y_ratio=0.0: top, 0.5: center, 1.0: bottom. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.
    RBLXUI_API void          SetScrollFromPosX(float local_x, float center_x_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.
    RBLXUI_API void          SetScrollFromPosY(float local_y, float center_y_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.

    // Parameters stacks (font)
    //  - PushFont(font, 0.0f)                       // Change font and keep current size
    //  - PushFont(NULL, 20.0f)                      // Keep font and change current size
    //  - PushFont(font, 20.0f)                      // Change font and set size to 20.0f
    //  - PushFont(font, style.FontSizeBase * 2.0f)  // Change font and set size to be twice bigger than current size.
    //  - PushFont(font, font->LegacySize)           // Change font and set size to size passed to AddFontXXX() function. Same as pre-1.92 behavior.
    // *IMPORTANT* before 1.92, fonts had a single size. They can now be dynamically be adjusted.
    //  - In 1.92 we have REMOVED the single parameter version of PushFont() because it seems like the easiest way to provide an error-proof transition.
    //  - PushFont(font) before 1.92 = PushFont(font, font->LegacySize) after 1.92          // Use default font size as passed to AddFontXXX() function.
    // *IMPORTANT* global scale factors are applied over the provided size.
    //  - Global scale factors are: 'style.FontScaleMain', 'style.FontScaleDpi' and maybe more.
    // -  If you want to apply a factor to the _current_ font size:
    //  - CORRECT:   PushFont(NULL, style.FontSizeBase)         // use current unscaled size    == does nothing
    //  - CORRECT:   PushFont(NULL, style.FontSizeBase * 2.0f)  // use current unscaled size x2 == make text twice bigger
    //  - INCORRECT: PushFont(NULL, GetFontSize())              // INCORRECT! using size after global factors already applied == GLOBAL SCALING FACTORS WILL APPLY TWICE!
    //  - INCORRECT: PushFont(NULL, GetFontSize() * 2.0f)       // INCORRECT! using size after global factors already applied == GLOBAL SCALING FACTORS WILL APPLY TWICE!
    RBLXUI_API void          PushFont(RblxFont* font, float font_size_base_unscaled);          // Use NULL as a shortcut to keep current font. Use 0.0f to keep current size.
    RBLXUI_API void          PopFont();
    RBLXUI_API RblxFont*       GetFont();                                                      // get current font
    RBLXUI_API float         GetFontSize();                                                  // get current scaled font size (= height in pixels). AFTER global scale factors applied. *IMPORTANT* DO NOT PASS THIS VALUE TO PushFont()! Use RblxUI::GetStyle().FontSizeBase to get value before global scale factors.
    RBLXUI_API RblxFontBaked*  GetFontBaked();                                                 // get current font bound at current size // == GetFont()->GetFontBaked(GetFontSize())

    // Parameters stacks (shared)
    RBLXUI_API void          PushStyleColor(RblxUICol idx, RblxU32 col);                        // modify a style color. always use this if you modify the style after NewFrame().
    RBLXUI_API void          PushStyleColor(RblxUICol idx, const RblxVec4& col);
    RBLXUI_API void          PopStyleColor(int count = 1);
    RBLXUI_API void          PushStyleVar(RblxUIStyleVar idx, float val);                     // modify a style float variable. always use this if you modify the style after NewFrame()!
    RBLXUI_API void          PushStyleVar(RblxUIStyleVar idx, const RblxVec2& val);             // modify a style RblxVec2 variable. "
    RBLXUI_API void          PushStyleVarX(RblxUIStyleVar idx, float val_x);                  // modify X component of a style RblxVec2 variable. "
    RBLXUI_API void          PushStyleVarY(RblxUIStyleVar idx, float val_y);                  // modify Y component of a style RblxVec2 variable. "
    RBLXUI_API void          PopStyleVar(int count = 1);
    RBLXUI_API void          PushItemFlag(RblxUIItemFlags option, bool enabled);              // modify specified shared item flag, e.g. PushItemFlag(RblxUIItemFlags_NoTabStop, true)
    RBLXUI_API void          PopItemFlag();

    // Parameters stacks (current window)
    RBLXUI_API void          PushItemWidth(float item_width);                                // push width of items for common large "item+label" widgets. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side).
    RBLXUI_API void          PopItemWidth();
    RBLXUI_API void          SetNextItemWidth(float item_width);                             // set width of the _next_ common large "item+label" widget. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side)
    RBLXUI_API float         CalcItemWidth();                                                // width of item given pushed settings and current cursor position. NOT necessarily the width of last item unlike most 'Item' functions.
    RBLXUI_API void          PushTextWrapPos(float wrap_local_pos_x = 0.0f);                 // push word-wrapping position for Text*() commands. < 0.0f: no wrapping; 0.0f: wrap to end of window (or column); > 0.0f: wrap at 'wrap_pos_x' position in window local space
    RBLXUI_API void          PopTextWrapPos();

    // Style read access
    // - Use the ShowStyleEditor() function to interactively see/edit the colors.
    RBLXUI_API RblxVec2        GetFontTexUvWhitePixel();                                       // get UV coordinate for a white pixel, useful to draw custom shapes via the RblxDrawList API
    RBLXUI_API RblxU32         GetColorU32(RblxUICol idx, float alpha_mul = 1.0f);              // retrieve given style color with style alpha applied and optional extra alpha multiplier, packed as a 32-bit value suitable for RblxDrawList
    RBLXUI_API RblxU32         GetColorU32(const RblxVec4& col);                                 // retrieve given color with style alpha applied, packed as a 32-bit value suitable for RblxDrawList
    RBLXUI_API RblxU32         GetColorU32(RblxU32 col, float alpha_mul = 1.0f);                 // retrieve given color with style alpha applied, packed as a 32-bit value suitable for RblxDrawList
    RBLXUI_API const RblxVec4& GetStyleColorVec4(RblxUICol idx);                                // retrieve style color as stored in RblxUIStyle structure. use to feed back into PushStyleColor(), otherwise use GetColorU32() to get style color with style alpha baked in.

    // Layout cursor positioning
    // - By "cursor" we mean the current output position.
    // - The typical widget behavior is to output themselves at the current cursor position, then move the cursor one line down.
    // - You can call SameLine() between widgets to undo the last carriage return and output at the right of the preceding widget.
    // - YOU CAN DO 99% OF WHAT YOU NEED WITH ONLY GetCursorScreenPos() and GetContentRegionAvail().
    // - Attention! We currently have inconsistencies between window-local and absolute positions we will aim to fix with future API:
    //    - Absolute coordinate:        GetCursorScreenPos(), SetCursorScreenPos(), all RblxDrawList:: functions. -> this is the preferred way forward.
    //    - Window-local coordinates:   SameLine(offset), GetCursorPos(), SetCursorPos(), GetCursorStartPos(), PushTextWrapPos()
    //    - Window-local coordinates:   GetContentRegionMax(), GetWindowContentRegionMin(), GetWindowContentRegionMax() --> all obsoleted. YOU DON'T NEED THEM.
    // - GetCursorScreenPos() = GetCursorPos() + GetWindowPos(). GetWindowPos() is almost only ever useful to convert from window-local to absolute coordinates. Try not to use it.
    RBLXUI_API RblxVec2        GetCursorScreenPos();                                           // cursor position, absolute coordinates. THIS IS YOUR BEST FRIEND (prefer using this rather than GetCursorPos(), also more useful to work with RblxDrawList API).
    RBLXUI_API void          SetCursorScreenPos(const RblxVec2& pos);                          // cursor position, absolute coordinates. THIS IS YOUR BEST FRIEND.
    RBLXUI_API RblxVec2        GetContentRegionAvail();                                        // available space from current position. THIS IS YOUR BEST FRIEND.
    RBLXUI_API RblxVec2        GetCursorPos();                                                 // [window-local] cursor position in window-local coordinates. This is not your best friend.
    RBLXUI_API float         GetCursorPosX();                                                // [window-local] "
    RBLXUI_API float         GetCursorPosY();                                                // [window-local] "
    RBLXUI_API void          SetCursorPos(const RblxVec2& local_pos);                          // [window-local] "
    RBLXUI_API void          SetCursorPosX(float local_x);                                   // [window-local] "
    RBLXUI_API void          SetCursorPosY(float local_y);                                   // [window-local] "
    RBLXUI_API RblxVec2        GetCursorStartPos();                                            // [window-local] initial cursor position, in window-local coordinates. Call GetCursorScreenPos() after Begin() to get the absolute coordinates version.

    // Other layout functions
    RBLXUI_API void          Separator();                                                    // separator, generally horizontal. inside a menu bar or in horizontal layout mode, this becomes a vertical separator.
    RBLXUI_API void          SameLine(float offset_from_start_x=0.0f, float spacing=-1.0f);  // call between widgets or groups to layout them horizontally. X position given in window coordinates.
    RBLXUI_API void          NewLine();                                                      // undo a SameLine() or force a new line when in a horizontal-layout context.
    RBLXUI_API void          Spacing();                                                      // add vertical spacing.
    RBLXUI_API void          Dummy(const RblxVec2& size);                                      // add a dummy item of given size. unlike InvisibleButton(), Dummy() won't take the mouse click or be navigable into.
    RBLXUI_API void          Indent(float indent_w = 0.0f);                                  // move content position toward the right, by indent_w, or style.IndentSpacing if indent_w <= 0
    RBLXUI_API void          Unindent(float indent_w = 0.0f);                                // move content position back to the left, by indent_w, or style.IndentSpacing if indent_w <= 0
    RBLXUI_API void          BeginGroup();                                                   // lock horizontal starting position
    RBLXUI_API void          EndGroup();                                                     // unlock horizontal starting position + capture the whole group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.)
    RBLXUI_API void          AlignTextToFramePadding();                                      // vertically align upcoming text baseline to FramePadding.y so that it will align properly to regularly framed items (call if you have text on a line before a framed item)
    RBLXUI_API float         GetTextLineHeight();                                            // ~ FontSize
    RBLXUI_API float         GetTextLineHeightWithSpacing();                                 // ~ FontSize + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of text)
    RBLXUI_API float         GetFrameHeight();                                               // ~ FontSize + style.FramePadding.y * 2
    RBLXUI_API float         GetFrameHeightWithSpacing();                                    // ~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)

    // ID stack/scopes
    // Read the FAQ (docs/FAQ.md or http://dearrblxui.com/faq) for more details about how ID are handled in dear rblxui.
    // - Those questions are answered and impacted by understanding of the ID stack system:
    //   - "Q: Why is my widget not reacting when I click on it?"
    //   - "Q: How can I have widgets with an empty label?"
    //   - "Q: How can I have multiple widgets with the same label?"
    // - Short version: ID are hashes of the entire ID stack. If you are creating widgets in a loop you most likely
    //   want to push a unique identifier (e.g. object pointer, loop index) to uniquely differentiate them.
    // - You can also use the "Label##foobar" syntax within widget label to distinguish them from each others.
    // - In this header file we use the "label"/"name" terminology to denote a string that will be displayed + used as an ID,
    //   whereas "str_id" denote a string that is only used as an ID and not normally displayed.
    RBLXUI_API void          PushID(const char* str_id);                                     // push string into the ID stack (will hash string).
    RBLXUI_API void          PushID(const char* str_id_begin, const char* str_id_end);       // push string into the ID stack (will hash string).
    RBLXUI_API void          PushID(const void* ptr_id);                                     // push pointer into the ID stack (will hash pointer).
    RBLXUI_API void          PushID(int int_id);                                             // push integer into the ID stack (will hash integer).
    RBLXUI_API void          PopID();                                                        // pop from the ID stack.
    RBLXUI_API RblxUIID       GetID(const char* str_id);                                      // calculate unique ID (hash of whole ID stack + given parameter). e.g. if you want to query into RblxUIStorage yourself
    RBLXUI_API RblxUIID       GetID(const char* str_id_begin, const char* str_id_end);
    RBLXUI_API RblxUIID       GetID(const void* ptr_id);
    RBLXUI_API RblxUIID       GetID(int int_id);

    // Widgets: Text
    // - Note that all functions taking format strings in the API may be passed ("%s", text) or ("%.*s", text_len, text): which will automatically bypass the formatter.
    RBLXUI_API void          TextUnformatted(const char* text, const char* text_end = NULL); // raw text without formatting. Practically equivalent to 'Text("%s", text)' but doesn't require null terminated string if 'text_end' is specified.
    RBLXUI_API void          Text(const char* fmt, ...)                                      RBLXUI_FMTARGS(1); // formatted text
    RBLXUI_API void          TextV(const char* fmt, va_list args)                            RBLXUI_FMTLIST(1);
    RBLXUI_API void          TextColored(const RblxVec4& col, const char* fmt, ...)            RBLXUI_FMTARGS(2); // shortcut for PushStyleColor(RblxUICol_Text, col); Text(fmt, ...); PopStyleColor();
    RBLXUI_API void          TextColoredV(const RblxVec4& col, const char* fmt, va_list args)  RBLXUI_FMTLIST(2);
    RBLXUI_API void          TextDisabled(const char* fmt, ...)                              RBLXUI_FMTARGS(1); // shortcut for PushStyleColor(RblxUICol_Text, style.Colors[RblxUICol_TextDisabled]); Text(fmt, ...); PopStyleColor();
    RBLXUI_API void          TextDisabledV(const char* fmt, va_list args)                    RBLXUI_FMTLIST(1);
    RBLXUI_API void          TextWrapped(const char* fmt, ...)                               RBLXUI_FMTARGS(1); // shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().
    RBLXUI_API void          TextWrappedV(const char* fmt, va_list args)                     RBLXUI_FMTLIST(1);
    RBLXUI_API void          LabelText(const char* label, const char* fmt, ...)              RBLXUI_FMTARGS(2); // display text+label aligned the same way as value+label widgets
    RBLXUI_API void          LabelTextV(const char* label, const char* fmt, va_list args)    RBLXUI_FMTLIST(2);
    RBLXUI_API void          BulletText(const char* fmt, ...)                                RBLXUI_FMTARGS(1); // shortcut for Bullet()+Text()
    RBLXUI_API void          BulletTextV(const char* fmt, va_list args)                      RBLXUI_FMTLIST(1);
    RBLXUI_API void          SeparatorText(const char* label);                               // currently: formatted text with a horizontal line

    // Widgets: Main
    // - Most widgets return true when the value has been changed or when pressed/selected
    // - You may also use one of the many IsItemXXX functions (e.g. IsItemActive, IsItemHovered, etc.) to query widget state.
    RBLXUI_API bool          Button(const char* label, const RblxVec2& size = RblxVec2(0, 0));   // button
    RBLXUI_API bool          SmallButton(const char* label);                                 // button with (FramePadding.y == 0) to easily embed within text
    RBLXUI_API bool          InvisibleButton(const char* str_id, const RblxVec2& size, RblxUIButtonFlags flags = 0); // flexible button behavior without the visuals, frequently useful to build custom behaviors using the public api (along with IsItemActive, IsItemHovered, etc.)
    RBLXUI_API bool          ArrowButton(const char* str_id, RblxUIDir dir);                  // square button with an arrow shape
    RBLXUI_API bool          Checkbox(const char* label, bool* v);
    RBLXUI_API bool          CheckboxFlags(const char* label, int* flags, int flags_value);
    RBLXUI_API bool          CheckboxFlags(const char* label, unsigned int* flags, unsigned int flags_value);
    RBLXUI_API bool          RadioButton(const char* label, bool active);                    // use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; }
    RBLXUI_API bool          RadioButton(const char* label, int* v, int v_button);           // shortcut to handle the above pattern when value is an integer
    RBLXUI_API void          ProgressBar(float fraction, const RblxVec2& size_arg = RblxVec2(-FLT_MIN, 0), const char* overlay = NULL);
    RBLXUI_API void          Bullet();                                                       // draw a small circle + keep the cursor on the same line. advance cursor x position by GetTreeNodeToLabelSpacing(), same distance that TreeNode() uses
    RBLXUI_API bool          TextLink(const char* label);                                    // hyperlink text button, return true when clicked
    RBLXUI_API bool          TextLinkOpenURL(const char* label, const char* url = NULL);     // hyperlink text button, automatically open file/url when clicked

    // Widgets: Images
    // - Read about RblxTextureID/RblxTextureRef  here: https://github.com/ocornut/rblxui/wiki/Image-Loading-and-Displaying-Examples
    // - 'uv0' and 'uv1' are texture coordinates. Read about them from the same link above.
    // - Image() pads adds style.ImageBorderSize on each side, ImageButton() adds style.FramePadding on each side.
    // - ImageButton() draws a background based on regular Button() color + optionally an inner background if specified.
    // - An obsolete version of Image(), before 1.91.9 (March 2025), had a 'tint_col' parameter which is now supported by the ImageWithBg() function.
    RBLXUI_API void          Image(RblxTextureRef tex_ref, const RblxVec2& image_size, const RblxVec2& uv0 = RblxVec2(0, 0), const RblxVec2& uv1 = RblxVec2(1, 1));
    RBLXUI_API void          ImageWithBg(RblxTextureRef tex_ref, const RblxVec2& image_size, const RblxVec2& uv0 = RblxVec2(0, 0), const RblxVec2& uv1 = RblxVec2(1, 1), const RblxVec4& bg_col = RblxVec4(0, 0, 0, 0), const RblxVec4& tint_col = RblxVec4(1, 1, 1, 1));
    RBLXUI_API bool          ImageButton(const char* str_id, RblxTextureRef tex_ref, const RblxVec2& image_size, const RblxVec2& uv0 = RblxVec2(0, 0), const RblxVec2& uv1 = RblxVec2(1, 1), const RblxVec4& bg_col = RblxVec4(0, 0, 0, 0), const RblxVec4& tint_col = RblxVec4(1, 1, 1, 1));

    // Widgets: Combo Box (Dropdown)
    // - The BeginCombo()/EndCombo() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() items.
    // - The old Combo() api are helpers over BeginCombo()/EndCombo() which are kept available for convenience purpose. This is analogous to how ListBox are created.
    RBLXUI_API bool          BeginCombo(const char* label, const char* preview_value, RblxUIComboFlags flags = 0);
    RBLXUI_API void          EndCombo(); // only call EndCombo() if BeginCombo() returns true!
    RBLXUI_API bool          Combo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
    RBLXUI_API bool          Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);      // Separate items with \0 within a string, end item-list with \0\0. e.g. "One\0Two\0Three\0"
    RBLXUI_API bool          Combo(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int popup_max_height_in_items = -1);

    // Widgets: Drag Sliders
    // - Ctrl+Click on any drag box to turn them into an input box. Manually input values aren't clamped by default and can go off-bounds. Use RblxUISliderFlags_AlwaysClamp to always clamp.
    // - For all the Float2/Float3/Float4/Int2/Int3/Int4 versions of every function, note that a 'float v[X]' function argument is the same as 'float* v',
    //   the array syntax is just a way to document the number of elements that are expected to be accessible. You can pass address of your first element out of a contiguous set, e.g. &myvector.x
    // - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
    // - Format string may also be set to NULL or use the default format ("%f" or "%d").
    // - Speed are per-pixel of mouse movement (v_speed=0.2f: mouse needs to move by 5 pixels to increase value by 1). For keyboard/gamepad navigation, minimum speed is Max(v_speed, minimum_step_at_given_precision).
    // - Use v_min < v_max to clamp edits to given limits. Note that Ctrl+Click manual input can override those limits if RblxUISliderFlags_AlwaysClamp is not used.
    // - Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with v_min = -FLT_MAX / INT_MIN to avoid clamping to a minimum.
    // - We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.
    // - Legacy: Pre-1.78 there are DragXXX() function signatures that take a final `float power=1.0f' argument instead of the `RblxUISliderFlags flags=0' argument.
    //   If you get a warning converting a float to RblxUISliderFlags, read https://github.com/ocornut/rblxui/issues/3361
    RBLXUI_API bool          DragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", RblxUISliderFlags flags = 0);     // If v_min >= v_max we have no bound
    RBLXUI_API bool          DragFloat2(const char* label, float v[2], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          DragFloat3(const char* label, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          DragFloat4(const char* label, float v[4], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          DragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", const char* format_max = NULL, RblxUISliderFlags flags = 0);
    RBLXUI_API bool          DragInt(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", RblxUISliderFlags flags = 0);  // If v_min >= v_max we have no bound
    RBLXUI_API bool          DragInt2(const char* label, int v[2], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          DragInt3(const char* label, int v[3], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          DragInt4(const char* label, int v[4], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          DragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", const char* format_max = NULL, RblxUISliderFlags flags = 0);
    RBLXUI_API bool          DragScalar(const char* label, RblxUIDataType data_type, void* p_data, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, RblxUISliderFlags flags = 0);
    RBLXUI_API bool          DragScalarN(const char* label, RblxUIDataType data_type, void* p_data, int components, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, RblxUISliderFlags flags = 0);

    // Widgets: Regular Sliders
    // - Ctrl+Click on any slider to turn them into an input box. Manually input values aren't clamped by default and can go off-bounds. Use RblxUISliderFlags_AlwaysClamp to always clamp.
    // - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
    // - Format string may also be set to NULL or use the default format ("%f" or "%d").
    // - Legacy: Pre-1.78 there are SliderXXX() function signatures that take a final `float power=1.0f' argument instead of the `RblxUISliderFlags flags=0' argument.
    //   If you get a warning converting a float to RblxUISliderFlags, read https://github.com/ocornut/rblxui/issues/3361
    RBLXUI_API bool          SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", RblxUISliderFlags flags = 0);     // adjust format to decorate the value with a prefix or a suffix for in-slider labels or unit display.
    RBLXUI_API bool          SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format = "%.3f", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format = "%.3f", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format = "%.3f", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          SliderAngle(const char* label, float* v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const char* format = "%.0f deg", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format = "%d", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format = "%d", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format = "%d", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          SliderScalar(const char* label, RblxUIDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, RblxUISliderFlags flags = 0);
    RBLXUI_API bool          SliderScalarN(const char* label, RblxUIDataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format = NULL, RblxUISliderFlags flags = 0);
    RBLXUI_API bool          VSliderFloat(const char* label, const RblxVec2& size, float* v, float v_min, float v_max, const char* format = "%.3f", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          VSliderInt(const char* label, const RblxVec2& size, int* v, int v_min, int v_max, const char* format = "%d", RblxUISliderFlags flags = 0);
    RBLXUI_API bool          VSliderScalar(const char* label, const RblxVec2& size, RblxUIDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, RblxUISliderFlags flags = 0);

    // Widgets: Input with Keyboard
    // - If you want to use InputText() with std::string or any custom dynamic string type, use the wrapper in misc/cpp/rblxui_stdlib.h/.cpp!
    // - Most of the RblxUIInputTextFlags flags are only useful for InputText() and not for InputFloatX, InputIntX, InputDouble etc.
    RBLXUI_API bool          InputText(const char* label, char* buf, size_t buf_size, RblxUIInputTextFlags flags = 0, RblxUIInputTextCallback callback = NULL, void* user_data = NULL);
    RBLXUI_API bool          InputTextMultiline(const char* label, char* buf, size_t buf_size, const RblxVec2& size = RblxVec2(0, 0), RblxUIInputTextFlags flags = 0, RblxUIInputTextCallback callback = NULL, void* user_data = NULL);
    RBLXUI_API bool          InputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, RblxUIInputTextFlags flags = 0, RblxUIInputTextCallback callback = NULL, void* user_data = NULL);
    RBLXUI_API bool          InputFloat(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", RblxUIInputTextFlags flags = 0);
    RBLXUI_API bool          InputFloat2(const char* label, float v[2], const char* format = "%.3f", RblxUIInputTextFlags flags = 0);
    RBLXUI_API bool          InputFloat3(const char* label, float v[3], const char* format = "%.3f", RblxUIInputTextFlags flags = 0);
    RBLXUI_API bool          InputFloat4(const char* label, float v[4], const char* format = "%.3f", RblxUIInputTextFlags flags = 0);
    RBLXUI_API bool          InputInt(const char* label, int* v, int step = 1, int step_fast = 100, RblxUIInputTextFlags flags = 0);
    RBLXUI_API bool          InputInt2(const char* label, int v[2], RblxUIInputTextFlags flags = 0);
    RBLXUI_API bool          InputInt3(const char* label, int v[3], RblxUIInputTextFlags flags = 0);
    RBLXUI_API bool          InputInt4(const char* label, int v[4], RblxUIInputTextFlags flags = 0);
    RBLXUI_API bool          InputDouble(const char* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", RblxUIInputTextFlags flags = 0);
    RBLXUI_API bool          InputScalar(const char* label, RblxUIDataType data_type, void* p_data, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, RblxUIInputTextFlags flags = 0);
    RBLXUI_API bool          InputScalarN(const char* label, RblxUIDataType data_type, void* p_data, int components, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, RblxUIInputTextFlags flags = 0);

    // Widgets: Color Editor/Picker (tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.)
    // - Note that in C++ a 'float v[X]' function argument is the _same_ as 'float* v', the array syntax is just a way to document the number of elements that are expected to be accessible.
    // - You can pass the address of a first float element out of a contiguous structure, e.g. &myvector.x
    RBLXUI_API bool          ColorEdit3(const char* label, float col[3], RblxUIColorEditFlags flags = 0);
    RBLXUI_API bool          ColorEdit4(const char* label, float col[4], RblxUIColorEditFlags flags = 0);
    RBLXUI_API bool          ColorPicker3(const char* label, float col[3], RblxUIColorEditFlags flags = 0);
    RBLXUI_API bool          ColorPicker4(const char* label, float col[4], RblxUIColorEditFlags flags = 0, const float* ref_col = NULL);
    RBLXUI_API bool          ColorButton(const char* desc_id, const RblxVec4& col, RblxUIColorEditFlags flags = 0, const RblxVec2& size = RblxVec2(0, 0)); // display a color square/button, hover for details, return true when pressed.
    RBLXUI_API void          SetColorEditOptions(RblxUIColorEditFlags flags);                     // initialize current options (generally on application startup) if you want to select a default format, picker type, etc. User will be able to change many settings, unless you pass the _NoOptions flag to your calls.

    // Widgets: Trees
    // - TreeNode functions return true when the node is open, in which case you need to also call TreePop() when you are finished displaying the tree node contents.
    RBLXUI_API bool          TreeNode(const char* label);
    RBLXUI_API bool          TreeNode(const char* str_id, const char* fmt, ...) RBLXUI_FMTARGS(2);   // helper variation to easily decorrelate the id from the displayed string. Read the FAQ about why and how to use ID. to align arbitrary text at the same level as a TreeNode() you can use Bullet().
    RBLXUI_API bool          TreeNode(const void* ptr_id, const char* fmt, ...) RBLXUI_FMTARGS(2);   // "
    RBLXUI_API bool          TreeNodeV(const char* str_id, const char* fmt, va_list args) RBLXUI_FMTLIST(2);
    RBLXUI_API bool          TreeNodeV(const void* ptr_id, const char* fmt, va_list args) RBLXUI_FMTLIST(2);
    RBLXUI_API bool          TreeNodeEx(const char* label, RblxUITreeNodeFlags flags = 0);
    RBLXUI_API bool          TreeNodeEx(const char* str_id, RblxUITreeNodeFlags flags, const char* fmt, ...) RBLXUI_FMTARGS(3);
    RBLXUI_API bool          TreeNodeEx(const void* ptr_id, RblxUITreeNodeFlags flags, const char* fmt, ...) RBLXUI_FMTARGS(3);
    RBLXUI_API bool          TreeNodeExV(const char* str_id, RblxUITreeNodeFlags flags, const char* fmt, va_list args) RBLXUI_FMTLIST(3);
    RBLXUI_API bool          TreeNodeExV(const void* ptr_id, RblxUITreeNodeFlags flags, const char* fmt, va_list args) RBLXUI_FMTLIST(3);
    RBLXUI_API void          TreePush(const char* str_id);                                       // ~ Indent()+PushID(). Already called by TreeNode() when returning true, but you can call TreePush/TreePop yourself if desired.
    RBLXUI_API void          TreePush(const void* ptr_id);                                       // "
    RBLXUI_API void          TreePop();                                                          // ~ Unindent()+PopID()
    RBLXUI_API float         GetTreeNodeToLabelSpacing();                                        // horizontal distance preceding label when using TreeNode*() or Bullet() == (g.FontSize + style.FramePadding.x*2) for a regular unframed TreeNode
    RBLXUI_API bool          CollapsingHeader(const char* label, RblxUITreeNodeFlags flags = 0);  // if returning 'true' the header is open. doesn't indent nor push on ID stack. user doesn't have to call TreePop().
    RBLXUI_API bool          CollapsingHeader(const char* label, bool* p_visible, RblxUITreeNodeFlags flags = 0); // when 'p_visible != NULL': if '*p_visible==true' display an additional small close button on upper right of the header which will set the bool to false when clicked, if '*p_visible==false' don't display the header.
    RBLXUI_API void          SetNextItemOpen(bool is_open, RblxUICond cond = 0);                  // set next TreeNode/CollapsingHeader open state.
    RBLXUI_API void          SetNextItemStorageID(RblxUIID storage_id);                           // set id to use for open/close storage (default to same as item id).
    RBLXUI_API bool          TreeNodeGetOpen(RblxUIID storage_id);                                // retrieve tree node open/close state.

    // Widgets: Selectables
    // - A selectable highlights when hovered, and can display another color when selected.
    // - Neighbors selectable extend their highlight bounds in order to leave no gap between them. This is so a series of selected Selectable appear contiguous.
    RBLXUI_API bool          Selectable(const char* label, bool selected = false, RblxUISelectableFlags flags = 0, const RblxVec2& size = RblxVec2(0, 0)); // "bool selected" carry the selection state (read-only). Selectable() is clicked is returns true so you can modify your selection state. size.x==0.0: use remaining width, size.x>0.0: specify width. size.y==0.0: use label height, size.y>0.0: specify height
    RBLXUI_API bool          Selectable(const char* label, bool* p_selected, RblxUISelectableFlags flags = 0, const RblxVec2& size = RblxVec2(0, 0));      // "bool* p_selected" point to the selection state (read-write), as a convenient helper.

    // Multi-selection system for Selectable(), Checkbox(), TreeNode() functions [BETA]
    // - This enables standard multi-selection/range-selection idioms (Ctrl+Mouse/Keyboard, Shift+Mouse/Keyboard, etc.) in a way that also allow a clipper to be used.
    // - RblxUISelectionUserData is often used to store your item index within the current view (but may store something else).
    // - Read comments near RblxUIMultiSelectIO for instructions/details and see 'Demo->Widgets->Selection State & Multi-Select' for demo.
    // - TreeNode() is technically supported but... using this correctly is more complicated. You need some sort of linear/random access to your tree,
    //   which is suited to advanced trees setups already implementing filters and clipper. We will work simplifying the current demo.
    // - 'selection_size' and 'items_count' parameters are optional and used by a few features. If they are costly for you to compute, you may avoid them.
    RBLXUI_API RblxUIMultiSelectIO*   BeginMultiSelect(RblxUIMultiSelectFlags flags, int selection_size = -1, int items_count = -1);
    RBLXUI_API RblxUIMultiSelectIO*   EndMultiSelect();
    RBLXUI_API void                  SetNextItemSelectionUserData(RblxUISelectionUserData selection_user_data);
    RBLXUI_API bool                  IsItemToggledSelection();                                   // Was the last item selection state toggled? Useful if you need the per-item information _before_ reaching EndMultiSelect(). We only returns toggle _event_ in order to handle clipping correctly.

    // Widgets: List Boxes
    // - This is essentially a thin wrapper to using BeginChild/EndChild with the RblxUIChildFlags_FrameStyle flag for stylistic changes + displaying a label.
    // - If you don't need a label you can probably simply use BeginChild() with the RblxUIChildFlags_FrameStyle flag for the same result.
    // - You can submit contents and manage your selection state however you want it, by creating e.g. Selectable() or any other items.
    // - The simplified/old ListBox() api are helpers over BeginListBox()/EndListBox() which are kept available for convenience purpose. This is analogous to how Combos are created.
    // - Choose frame width:   size.x > 0.0f: custom  /  size.x < 0.0f or -FLT_MIN: right-align   /  size.x = 0.0f (default): use current ItemWidth
    // - Choose frame height:  size.y > 0.0f: custom  /  size.y < 0.0f or -FLT_MIN: bottom-align  /  size.y = 0.0f (default): arbitrary default height which can fit ~7 items
    RBLXUI_API bool          BeginListBox(const char* label, const RblxVec2& size = RblxVec2(0, 0)); // open a framed scrolling region
    RBLXUI_API void          EndListBox();                                                       // only call EndListBox() if BeginListBox() returned true!
    RBLXUI_API bool          ListBox(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items = -1);
    RBLXUI_API bool          ListBox(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int height_in_items = -1);

    // Widgets: Data Plotting
    // - Consider using RblxPlot (https://github.com/epezent/implot) which is much better!
    RBLXUI_API void          PlotLines(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, RblxVec2 graph_size = RblxVec2(0, 0), int stride = sizeof(float));
    RBLXUI_API void          PlotLines(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, RblxVec2 graph_size = RblxVec2(0, 0));
    RBLXUI_API void          PlotHistogram(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, RblxVec2 graph_size = RblxVec2(0, 0), int stride = sizeof(float));
    RBLXUI_API void          PlotHistogram(const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, RblxVec2 graph_size = RblxVec2(0, 0));

    // Widgets: Value() Helpers.
    // - Those are merely shortcut to calling Text() with a format string. Output single value in "name: value" format (tip: freely declare more in your code to handle your types. you can add functions to the RblxUI namespace)
    RBLXUI_API void          Value(const char* prefix, bool b);
    RBLXUI_API void          Value(const char* prefix, int v);
    RBLXUI_API void          Value(const char* prefix, unsigned int v);
    RBLXUI_API void          Value(const char* prefix, float v, const char* float_format = NULL);

    // Widgets: Menus
    // - Use BeginMenuBar() on a window RblxUIWindowFlags_MenuBar to append to its menu bar.
    // - Use BeginMainMenuBar() to create a menu bar at the top of the screen and append to it.
    // - Use BeginMenu() to create a menu. You can call BeginMenu() multiple time with the same identifier to append more items to it.
    // - Not that MenuItem() keyboardshortcuts are displayed as a convenience but _not processed_ by Dear RblxUI at the moment.
    RBLXUI_API bool          BeginMenuBar();                                                     // append to menu-bar of current window (requires RblxUIWindowFlags_MenuBar flag set on parent window).
    RBLXUI_API void          EndMenuBar();                                                       // only call EndMenuBar() if BeginMenuBar() returns true!
    RBLXUI_API bool          BeginMainMenuBar();                                                 // create and append to a full screen menu-bar.
    RBLXUI_API void          EndMainMenuBar();                                                   // only call EndMainMenuBar() if BeginMainMenuBar() returns true!
    RBLXUI_API bool          BeginMenu(const char* label, bool enabled = true);                  // create a sub-menu entry. only call EndMenu() if this returns true!
    RBLXUI_API void          EndMenu();                                                          // only call EndMenu() if BeginMenu() returns true!
    RBLXUI_API bool          MenuItem(const char* label, const char* shortcut = NULL, bool selected = false, bool enabled = true);  // return true when activated.
    RBLXUI_API bool          MenuItem(const char* label, const char* shortcut, bool* p_selected, bool enabled = true);              // return true when activated + toggle (*p_selected) if p_selected != NULL

    // Tooltips
    // - Tooltips are windows following the mouse. They do not take focus away.
    // - A tooltip window can contain items of any types.
    // - SetTooltip() is more or less a shortcut for the 'if (BeginTooltip()) { Text(...); EndTooltip(); }' idiom (with a subtlety that it discard any previously submitted tooltip)
    RBLXUI_API bool          BeginTooltip();                                                     // begin/append a tooltip window.
    RBLXUI_API void          EndTooltip();                                                       // only call EndTooltip() if BeginTooltip()/BeginItemTooltip() returns true!
    RBLXUI_API void          SetTooltip(const char* fmt, ...) RBLXUI_FMTARGS(1);                     // set a text-only tooltip. Often used after a RblxUI::IsItemHovered() check. Override any previous call to SetTooltip().
    RBLXUI_API void          SetTooltipV(const char* fmt, va_list args) RBLXUI_FMTLIST(1);

    // Tooltips: helpers for showing a tooltip when hovering an item
    // - BeginItemTooltip() is a shortcut for the 'if (IsItemHovered(RblxUIHoveredFlags_ForTooltip) && BeginTooltip())' idiom.
    // - SetItemTooltip() is a shortcut for the 'if (IsItemHovered(RblxUIHoveredFlags_ForTooltip)) { SetTooltip(...); }' idiom.
    // - Where 'RblxUIHoveredFlags_ForTooltip' itself is a shortcut to use 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav' depending on active input type. For mouse it defaults to 'RblxUIHoveredFlags_Stationary | RblxUIHoveredFlags_DelayShort'.
    RBLXUI_API bool          BeginItemTooltip();                                                 // begin/append a tooltip window if preceding item was hovered.
    RBLXUI_API void          SetItemTooltip(const char* fmt, ...) RBLXUI_FMTARGS(1);                 // set a text-only tooltip if preceding item was hovered. override any previous call to SetTooltip().
    RBLXUI_API void          SetItemTooltipV(const char* fmt, va_list args) RBLXUI_FMTLIST(1);

    // Popups, Modals
    //  - They block normal mouse hovering detection (and therefore most mouse interactions) behind them.
    //  - If not modal: they can be closed by clicking anywhere outside them, or by pressing Escape (call 'Shortcut(RblxUIKey_Escape)' to claim a higher-priority shortcut).
    //  - Their visibility state (~bool) is held internally instead of being held by the programmer as we are used to with regular Begin*() calls.
    //  - The 3 properties above are related: we need to retain popup visibility state in the library because popups may be closed as any time.
    //  - You can bypass the hovering restriction by using RblxUIHoveredFlags_AllowWhenBlockedByPopup when calling IsItemHovered() or IsWindowHovered().
    //  - IMPORTANT: Popup identifiers are relative to the current ID stack, so OpenPopup and BeginPopup generally needs to be at the same level of the stack.
    //    This is sometimes leading to confusing mistakes. May rework this in the future.
    //  - BeginPopup(): query popup state, if open start appending into the window. Call EndPopup() afterwards if returned true. RblxUIWindowFlags are forwarded to the window.
    //  - BeginPopupModal(): block every interaction behind the window, cannot be closed by user, add a dimming background, has a title bar.
    RBLXUI_API bool          BeginPopup(const char* str_id, RblxUIWindowFlags flags = 0);                         // return true if the popup is open, and you can start outputting to it.
    RBLXUI_API bool          BeginPopupModal(const char* name, bool* p_open = NULL, RblxUIWindowFlags flags = 0); // return true if the modal is open, and you can start outputting to it.
    RBLXUI_API void          EndPopup();                                                                         // only call EndPopup() if BeginPopupXXX() returns true!

    // Popups: open/close functions
    //  - OpenPopup(): set popup state to open. RblxUIPopupFlags are available for opening options.
    //  - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
    //  - CloseCurrentPopup(): use inside the BeginPopup()/EndPopup() scope to close manually.
    //  - CloseCurrentPopup() is called by default by Selectable()/MenuItem() when activated (FIXME: need some options).
    //  - Use RblxUIPopupFlags_NoOpenOverExistingPopup to avoid opening a popup if there's already one at the same level. This is equivalent to e.g. testing for !IsAnyPopupOpen() prior to OpenPopup().
    //  - Use IsWindowAppearing() after BeginPopup() to tell if a window just opened.
    RBLXUI_API void          OpenPopup(const char* str_id, RblxUIPopupFlags popup_flags = 0);                     // call to mark popup as open (don't call every frame!).
    RBLXUI_API void          OpenPopup(RblxUIID id, RblxUIPopupFlags popup_flags = 0);                             // id overload to facilitate calling from nested stacks
    RBLXUI_API void          OpenPopupOnItemClick(const char* str_id = NULL, RblxUIPopupFlags popup_flags = 0);   // helper to open popup when clicked on last item. Default to RblxUIPopupFlags_MouseButtonRight == 1. (note: actually triggers on the mouse _released_ event to be consistent with popup behaviors)
    RBLXUI_API void          CloseCurrentPopup();                                                                // manually close the popup we have begin-ed into.

    // Popups: Open+Begin popup combined functions helpers to create context menus.
    //  - Helpers to do OpenPopup+BeginPopup where the Open action is triggered by e.g. hovering an item and right-clicking.
    //  - IMPORTANT: Notice that BeginPopupContextXXX takes RblxUIPopupFlags just like OpenPopup() and unlike BeginPopup(). For full consistency, we may add RblxUIWindowFlags to the BeginPopupContextXXX functions in the future.
    //  - IMPORTANT: If you ever used the left mouse button with BeginPopupContextXXX() helpers before 1.92.6:
    //    - Before this version, OpenPopupOnItemClick(), BeginPopupContextItem(), BeginPopupContextWindow(), BeginPopupContextVoid() had 'a RblxUIPopupFlags popup_flags = 1' default value in their function signature.
    //    - Before: Explicitly passing a literal 0 meant RblxUIPopupFlags_MouseButtonLeft. The default = 1 meant RblxUIPopupFlags_MouseButtonRight.
    //    - After: The default = 0 means RblxUIPopupFlags_MouseButtonRight. Explicitly passing a literal 1 also means RblxUIPopupFlags_MouseButtonRight (if legacy behavior are enabled) or will assert (if legacy behavior are disabled).
    //    - TL;DR: if you don't want to use right mouse button for popups, always specify it explicitly using a named RblxUIPopupFlags_MouseButtonXXXX value.
    //    - Read "API BREAKING CHANGES" 2026/01/07 (1.92.6) entry in rblxui.cpp or GitHub topic #9157 for all details.
    RBLXUI_API bool          BeginPopupContextItem(const char* str_id = NULL, RblxUIPopupFlags popup_flags = 0);  // open+begin popup when clicked on last item. Use str_id==NULL to associate the popup to previous item. If you want to use that on a non-interactive item such as Text() you need to pass in an explicit ID here. read comments in .cpp!
    RBLXUI_API bool          BeginPopupContextWindow(const char* str_id = NULL, RblxUIPopupFlags popup_flags = 0);// open+begin popup when clicked on current window.
    RBLXUI_API bool          BeginPopupContextVoid(const char* str_id = NULL, RblxUIPopupFlags popup_flags = 0);  // open+begin popup when clicked in void (where there are no windows).

    // Popups: query functions
    //  - IsPopupOpen(): return true if the popup is open at the current BeginPopup() level of the popup stack.
    //  - IsPopupOpen() with RblxUIPopupFlags_AnyPopupId: return true if any popup is open at the current BeginPopup() level of the popup stack.
    //  - IsPopupOpen() with RblxUIPopupFlags_AnyPopupId + RblxUIPopupFlags_AnyPopupLevel: return true if any popup is open.
    RBLXUI_API bool          IsPopupOpen(const char* str_id, RblxUIPopupFlags flags = 0);                         // return true if the popup is open.

    // Tables
    // - Full-featured replacement for old Columns API.
    // - See Demo->Tables for demo code. See top of rblxui_tables.cpp for general commentary.
    // - See RblxUITableFlags_ and RblxUITableColumnFlags_ enums for a description of available flags.
    // The typical call flow is:
    // - 1. Call BeginTable(), early out if returning false.
    // - 2. Optionally call TableSetupColumn() to submit column name/flags/defaults.
    // - 3. Optionally call TableSetupScrollFreeze() to request scroll freezing of columns/rows.
    // - 4. Optionally call TableHeadersRow() to submit a header row. Names are pulled from TableSetupColumn() data.
    // - 5. Populate contents:
    //    - In most situations you can use TableNextRow() + TableSetColumnIndex(N) to start appending into a column.
    //    - If you are using tables as a sort of grid, where every column is holding the same type of contents,
    //      you may prefer using TableNextColumn() instead of TableNextRow() + TableSetColumnIndex().
    //      TableNextColumn() will automatically wrap-around into the next row if needed.
    //    - IMPORTANT: Comparatively to the old Columns() API, we need to call TableNextColumn() for the first column!
    //    - Summary of possible call flow:
    //        - TableNextRow() -> TableSetColumnIndex(0) -> Text("Hello 0") -> TableSetColumnIndex(1) -> Text("Hello 1")  // OK
    //        - TableNextRow() -> TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK
    //        -                   TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK: TableNextColumn() automatically gets to next row!
    //        - TableNextRow()                           -> Text("Hello 0")                                               // Not OK! Missing TableSetColumnIndex() or TableNextColumn()! Text will not appear!
    // - 5. Call EndTable()
    RBLXUI_API bool          BeginTable(const char* str_id, int columns, RblxUITableFlags flags = 0, const RblxVec2& outer_size = RblxVec2(0.0f, 0.0f), float inner_width = 0.0f);
    RBLXUI_API void          EndTable();                                         // only call EndTable() if BeginTable() returns true!
    RBLXUI_API void          TableNextRow(RblxUITableRowFlags row_flags = 0, float min_row_height = 0.0f); // append into the first cell of a new row. 'min_row_height' include the minimum top and bottom padding aka CellPadding.y * 2.0f.
    RBLXUI_API bool          TableNextColumn();                                  // append into the next column (or first column of next row if currently in last column). Return true when column is visible.
    RBLXUI_API bool          TableSetColumnIndex(int column_n);                  // append into the specified column. Return true when column is visible.

    // Tables: Headers & Columns declaration
    // - Use TableSetupColumn() to specify label, resizing policy, default width/weight, id, various other flags etc.
    // - Use TableHeadersRow() to create a header row and automatically submit a TableHeader() for each column.
    //   Headers are required to perform: reordering, sorting, and opening the context menu.
    //   The context menu can also be made available in columns body using RblxUITableFlags_ContextMenuInBody.
    // - You may manually submit headers using TableNextRow() + TableHeader() calls, but this is only useful in
    //   some advanced use cases (e.g. adding custom widgets in header row).
    // - Use TableSetupScrollFreeze() to lock columns/rows so they stay visible when scrolled. When freezing columns you would usually also use RblxUITableColumnFlags_NoHide on them.
    RBLXUI_API void          TableSetupColumn(const char* label, RblxUITableColumnFlags flags = 0, float init_width_or_weight = 0.0f, RblxUIID user_id = 0);
    RBLXUI_API void          TableSetupScrollFreeze(int cols, int rows);         // lock columns/rows so they stay visible when scrolled.
    RBLXUI_API void          TableHeader(const char* label);                     // submit one header cell manually (rarely used)
    RBLXUI_API void          TableHeadersRow();                                  // submit a row with headers cells based on data provided to TableSetupColumn() + submit context menu
    RBLXUI_API void          TableAngledHeadersRow();                            // submit a row with angled headers for every column with the RblxUITableColumnFlags_AngledHeader flag. MUST BE FIRST ROW.

    // Tables: Sorting & Miscellaneous functions
    // - Sorting: call TableGetSortSpecs() to retrieve latest sort specs for the table. NULL when not sorting.
    //   When 'sort_specs->SpecsDirty == true' you should sort your data. It will be true when sorting specs have
    //   changed since last call, or the first time. Make sure to set 'SpecsDirty = false' after sorting,
    //   else you may wastefully sort your data every frame!
    // - Functions args 'int column_n' treat the default value of -1 as the same as passing the current column index.
    RBLXUI_API RblxUITableSortSpecs*  TableGetSortSpecs();                        // get latest sort specs for the table (NULL if not sorting).  Lifetime: don't hold on this pointer over multiple frames or past any subsequent call to BeginTable().
    RBLXUI_API int                   TableGetColumnCount();                      // return number of columns (value passed to BeginTable)
    RBLXUI_API int                   TableGetColumnIndex();                      // return current column index.
    RBLXUI_API int                   TableGetRowIndex();                         // return current row index (header rows are accounted for)
    RBLXUI_API const char*           TableGetColumnName(int column_n = -1);      // return "" if column didn't have a name declared by TableSetupColumn(). Pass -1 to use current column.
    RBLXUI_API RblxUITableColumnFlags TableGetColumnFlags(int column_n = -1);     // return column flags so you can query their Enabled/Visible/Sorted/Hovered status flags. Pass -1 to use current column.
    RBLXUI_API void                  TableSetColumnEnabled(int column_n, bool v);// change user accessible enabled/disabled state of a column. Set to false to hide the column. User can use the context menu to change this themselves (right-click in headers, or right-click in columns body with RblxUITableFlags_ContextMenuInBody)
    RBLXUI_API int                   TableGetHoveredColumn();                    // return hovered column. return -1 when table is not hovered. return columns_count if the unused space at the right of visible columns is hovered. Can also use (TableGetColumnFlags() & RblxUITableColumnFlags_IsHovered) instead.
    RBLXUI_API void                  TableSetBgColor(RblxUITableBgTarget target, RblxU32 color, int column_n = -1);  // change the color of a cell, row, or column. See RblxUITableBgTarget_ flags for details.

    // Legacy Columns API (prefer using Tables!)
    // - You can also use SameLine(pos_x) to mimic simplified columns.
    RBLXUI_API void          Columns(int count = 1, const char* id = NULL, bool borders = true);
    RBLXUI_API void          NextColumn();                                                       // next column, defaults to current row or next row if the current row is finished
    RBLXUI_API int           GetColumnIndex();                                                   // get current column index
    RBLXUI_API float         GetColumnWidth(int column_index = -1);                              // get column width (in pixels). pass -1 to use current column
    RBLXUI_API void          SetColumnWidth(int column_index, float width);                      // set column width (in pixels). pass -1 to use current column
    RBLXUI_API float         GetColumnOffset(int column_index = -1);                             // get position of column line (in pixels, from the left side of the contents region). pass -1 to use current column, otherwise 0..GetColumnsCount() inclusive. column 0 is typically 0.0f
    RBLXUI_API void          SetColumnOffset(int column_index, float offset_x);                  // set position of column line (in pixels, from the left side of the contents region). pass -1 to use current column
    RBLXUI_API int           GetColumnsCount();

    // Tab Bars, Tabs
    // - Note: Tabs are automatically created by the docking system (when in 'docking' branch). Use this to create tab bars/tabs yourself.
    RBLXUI_API bool          BeginTabBar(const char* str_id, RblxUITabBarFlags flags = 0);        // create and append into a TabBar
    RBLXUI_API void          EndTabBar();                                                        // only call EndTabBar() if BeginTabBar() returns true!
    RBLXUI_API bool          BeginTabItem(const char* label, bool* p_open = NULL, RblxUITabItemFlags flags = 0); // create a Tab. Returns true if the Tab is selected.
    RBLXUI_API void          EndTabItem();                                                       // only call EndTabItem() if BeginTabItem() returns true!
    RBLXUI_API bool          TabItemButton(const char* label, RblxUITabItemFlags flags = 0);      // create a Tab behaving like a button. return true when clicked. cannot be selected in the tab bar.
    RBLXUI_API void          SetTabItemClosed(const char* tab_or_docked_window_label);           // notify TabBar or Docking system of a closed tab/window ahead (useful to reduce visual flicker on reorderable tab bars). For tab-bar: call after BeginTabBar() and before Tab submissions. Otherwise call with a window name.

    // Docking
    // - Read https://github.com/ocornut/rblxui/wiki/Docking for details.
    // - Enable with io.ConfigFlags |= RblxUIConfigFlags_DockingEnable.
    // - You can use many Docking facilities without calling any API.
    //   - Drag from window title bar or their tab to dock/undock. Hold SHIFT to disable docking.
    //   - Drag from window menu button (upper-left button) to undock an entire node (all windows).
    //   - When io.ConfigDockingWithShift == true, you instead need to hold SHIFT to enable docking.
    // - DockSpaceOverViewport:
    //   - This is a helper to create an invisible window covering a viewport, then submit a DockSpace() into it.
    //   - Most applications can simply call DockSpaceOverViewport() once to allow docking windows into e.g. the edge of your screen.
    //     e.g. RblxUI::NewFrame(); RblxUI::DockSpaceOverViewport();                                                   // Create a dockspace in main viewport.
    //      or: RblxUI::NewFrame(); RblxUI::DockSpaceOverViewport(0, nullptr, RblxUIDockNodeFlags_PassthruCentralNode); // Create a dockspace in main viewport, central node is transparent.
    // - Dockspaces:
    //   - A dockspace is an explicit dock node within an existing window.
    //   - IMPORTANT: Dockspaces need to be submitted _before_ any window they can host. Submit them early in your frame!
    //   - IMPORTANT: Dockspaces need to be kept alive if hidden, otherwise windows docked into it will be undocked.
    //     If you have e.g. multiple tabs with a dockspace inside each tab: submit the non-visible dockspaces with RblxUIDockNodeFlags_KeepAliveOnly.
    //   - See 'Demo->Examples->Dockspace' or 'Demo->Examples->Documents' for more detailed demos.
    // - Programmatic docking:
    //   - There is no public API yet other than the very limited SetNextWindowDockID() function. Sorry for that!
    //   - Read https://github.com/ocornut/rblxui/wiki/Docking for examples of how to use current internal API.
    RBLXUI_API RblxUIID       DockSpace(RblxUIID dockspace_id, const RblxVec2& size = RblxVec2(0, 0), RblxUIDockNodeFlags flags = 0, const RblxUIWindowClass* window_class = NULL);
    RBLXUI_API RblxUIID       DockSpaceOverViewport(RblxUIID dockspace_id = 0, const RblxUIViewport* viewport = NULL, RblxUIDockNodeFlags flags = 0, const RblxUIWindowClass* window_class = NULL);
    RBLXUI_API void          SetNextWindowDockID(RblxUIID dock_id, RblxUICond cond = 0);           // set next window dock id
    RBLXUI_API void          SetNextWindowClass(const RblxUIWindowClass* window_class);           // set next window class (control docking compatibility + provide hints to platform backend via custom viewport flags and platform parent/child relationship)
    RBLXUI_API RblxUIID       GetWindowDockID();                                                  // get dock id of current window, or 0 if not associated to any docking node.
    RBLXUI_API bool          IsWindowDocked();                                                   // is current window docked into another window?

    // Logging/Capture
    // - All text output from the interface can be captured into tty/file/clipboard. By default, tree nodes are automatically opened during logging.
    RBLXUI_API void          LogToTTY(int auto_open_depth = -1);                                 // start logging to tty (stdout)
    RBLXUI_API void          LogToFile(int auto_open_depth = -1, const char* filename = NULL);   // start logging to file
    RBLXUI_API void          LogToClipboard(int auto_open_depth = -1);                           // start logging to OS clipboard
    RBLXUI_API void          LogFinish();                                                        // stop logging (close file, etc.)
    RBLXUI_API void          LogButtons();                                                       // helper to display buttons for logging to tty/file/clipboard
    RBLXUI_API void          LogText(const char* fmt, ...) RBLXUI_FMTARGS(1);                        // pass text data straight to log (without being displayed)
    RBLXUI_API void          LogTextV(const char* fmt, va_list args) RBLXUI_FMTLIST(1);

    // Drag and Drop
    // - On source items, call BeginDragDropSource(), if it returns true also call SetDragDropPayload() + EndDragDropSource().
    // - On target candidates, call BeginDragDropTarget(), if it returns true also call AcceptDragDropPayload() + EndDragDropTarget().
    // - If you stop calling BeginDragDropSource() the payload is preserved however it won't have a preview tooltip (we currently display a fallback "..." tooltip, see #1725)
    // - An item can be both drag source and drop target.
    RBLXUI_API bool          BeginDragDropSource(RblxUIDragDropFlags flags = 0);                                      // call after submitting an item which may be dragged. when this return true, you can call SetDragDropPayload() + EndDragDropSource()
    RBLXUI_API bool          SetDragDropPayload(const char* type, const void* data, size_t sz, RblxUICond cond = 0);  // type is a user defined string of maximum 32 characters. Strings starting with '_' are reserved for dear rblxui internal types. Data is copied and held by rblxui. Return true when payload has been accepted.
    RBLXUI_API void          EndDragDropSource();                                                                    // only call EndDragDropSource() if BeginDragDropSource() returns true!
    RBLXUI_API bool                  BeginDragDropTarget();                                                          // call after submitting an item that may receive a payload. If this returns true, you can call AcceptDragDropPayload() + EndDragDropTarget()
    RBLXUI_API const RblxUIPayload*   AcceptDragDropPayload(const char* type, RblxUIDragDropFlags flags = 0);          // accept contents of a given type. If RblxUIDragDropFlags_AcceptBeforeDelivery is set you can peek into the payload before the mouse button is released.
    RBLXUI_API void                  EndDragDropTarget();                                                            // only call EndDragDropTarget() if BeginDragDropTarget() returns true!
    RBLXUI_API const RblxUIPayload*   GetDragDropPayload();                                                           // peek directly into the current payload from anywhere. returns NULL when drag and drop is finished or inactive. use RblxUIPayload::IsDataType() to test for the payload type.

    // Disabling [BETA API]
    // - Disable all user interactions and dim items visuals (applying style.DisabledAlpha over current colors)
    // - Those can be nested but it cannot be used to enable an already disabled section (a single BeginDisabled(true) in the stack is enough to keep everything disabled)
    // - Tooltips windows are automatically opted out of disabling. Note that IsItemHovered() by default returns false on disabled items, unless using RblxUIHoveredFlags_AllowWhenDisabled.
    // - BeginDisabled(false)/EndDisabled() essentially does nothing but is provided to facilitate use of boolean expressions (as a micro-optimization: if you have tens of thousands of BeginDisabled(false)/EndDisabled() pairs, you might want to reformulate your code to avoid making those calls)
    RBLXUI_API void          BeginDisabled(bool disabled = true);
    RBLXUI_API void          EndDisabled();

    // Clipping
    // - Mouse hovering is affected by RblxUI::PushClipRect() calls, unlike direct calls to RblxDrawList::PushClipRect() which are render only.
    RBLXUI_API void          PushClipRect(const RblxVec2& clip_rect_min, const RblxVec2& clip_rect_max, bool intersect_with_current_clip_rect);
    RBLXUI_API void          PopClipRect();

    // Focus, Activation
    RBLXUI_API void          SetItemDefaultFocus();                                              // make last item the default focused item of a newly appearing window.
    RBLXUI_API void          SetKeyboardFocusHere(int offset = 0);                               // focus keyboard on the next widget. Use positive 'offset' to access sub components of a multiple component widget. Use -1 to access previous widget.

    // Keyboard/Gamepad Navigation
    RBLXUI_API void          SetNavCursorVisible(bool visible);                                  // alter visibility of keyboard/gamepad cursor. by default: show when using an arrow key, hide when clicking with mouse.

    // Overlapping mode
    RBLXUI_API void          SetNextItemAllowOverlap();                                          // allow next item to be overlapped by a subsequent item. Typically useful with InvisibleButton(), Selectable(), TreeNode() covering an area where subsequent items may need to be added. Note that both Selectable() and TreeNode() have dedicated flags doing this.

    // Item/Widgets Utilities and Query Functions
    // - Most of the functions are referring to the previous Item that has been submitted.
    // - See Demo Window under "Widgets->Querying Status" for an interactive visualization of most of those functions.
    RBLXUI_API bool          IsItemHovered(RblxUIHoveredFlags flags = 0);                         // is the last item hovered? (and usable, aka not blocked by a popup, etc.). See RblxUIHoveredFlags for more options.
    RBLXUI_API bool          IsItemActive();                                                     // is the last item active? (e.g. button being held, text field being edited. This will continuously return true while holding mouse button on an item. Items that don't interact will always return false)
    RBLXUI_API bool          IsItemFocused();                                                    // is the last item focused for keyboard/gamepad navigation?
    RBLXUI_API bool          IsItemClicked(RblxUIMouseButton mouse_button = 0);                   // is the last item hovered and mouse clicked on? (**)  == IsMouseClicked(mouse_button) && IsItemHovered()Important. (**) this is NOT equivalent to the behavior of e.g. Button(). Read comments in function definition.
    RBLXUI_API bool          IsItemVisible();                                                    // is the last item visible? (items may be out of sight because of clipping/scrolling)
    RBLXUI_API bool          IsItemEdited();                                                     // did the last item modify its underlying value this frame? or was pressed? This is generally the same as the "bool" return value of many widgets.
    RBLXUI_API bool          IsItemActivated();                                                  // was the last item just made active (item was previously inactive).
    RBLXUI_API bool          IsItemDeactivated();                                                // was the last item just made inactive (item was previously active). Useful for Undo/Redo patterns with widgets that require continuous editing.
    RBLXUI_API bool          IsItemDeactivatedAfterEdit();                                       // was the last item just made inactive and made a value change when it was active? (e.g. Slider/Drag moved). Useful for Undo/Redo patterns with widgets that require continuous editing. Note that you may get false positives (some widgets such as Combo()/ListBox()/Selectable() will return true even when clicking an already selected item).
    RBLXUI_API bool          IsItemToggledOpen();                                                // was the last item open state toggled? set by TreeNode().
    RBLXUI_API bool          IsAnyItemHovered();                                                 // is any item hovered?
    RBLXUI_API bool          IsAnyItemActive();                                                  // is any item active?
    RBLXUI_API bool          IsAnyItemFocused();                                                 // is any item focused?
    RBLXUI_API RblxUIID       GetItemID();                                                        // get ID of last item (~~ often same RblxUI::GetID(label) beforehand)
    RBLXUI_API RblxVec2        GetItemRectMin();                                                   // get upper-left bounding rectangle of the last item (screen space)
    RBLXUI_API RblxVec2        GetItemRectMax();                                                   // get lower-right bounding rectangle of the last item (screen space)
    RBLXUI_API RblxVec2        GetItemRectSize();                                                  // get size of last item
    RBLXUI_API RblxUIItemFlags GetItemFlags();                                                    // get generic flags of last item

    // Viewports
    // - Currently represents the Platform Window created by the application which is hosting our Dear RblxUI windows.
    // - In 'docking' branch with multi-viewport enabled, we extend this concept to have multiple active viewports.
    // - In the future we will extend this concept further to also represent Platform Monitor and support a "no main platform window" operation mode.
    RBLXUI_API RblxUIViewport* GetMainViewport();                                                 // return primary/default viewport. This can never be NULL.

    // Background/Foreground Draw Lists
    RBLXUI_API RblxDrawList*   GetBackgroundDrawList(RblxUIViewport* viewport = NULL);              // get background draw list for the given viewport or viewport associated to the current window. this draw list will be the first rendering one. Useful to quickly draw shapes/text behind dear rblxui contents.
    RBLXUI_API RblxDrawList*   GetForegroundDrawList(RblxUIViewport* viewport = NULL);              // get foreground draw list for the given viewport or viewport associated to the current window. this draw list will be the top-most rendered one. Useful to quickly draw shapes/text over dear rblxui contents.

    // Miscellaneous Utilities
    RBLXUI_API bool          IsRectVisible(const RblxVec2& size);                                  // test if rectangle (of given size, starting from cursor position) is visible / not clipped.
    RBLXUI_API bool          IsRectVisible(const RblxVec2& rect_min, const RblxVec2& rect_max);      // test if rectangle (in screen space) is visible / not clipped. to perform coarse clipping on user's side.
    RBLXUI_API double        GetTime();                                                          // get global rblxui time. incremented by io.DeltaTime every frame.
    RBLXUI_API int           GetFrameCount();                                                    // get global rblxui frame count. incremented by 1 every frame.
    RBLXUI_API RblxDrawListSharedData* GetDrawListSharedData();                                    // you may use this when creating your own RblxDrawList instances.
    RBLXUI_API const char*   GetStyleColorName(RblxUICol idx);                                    // get a string corresponding to the enum value (for display, saving, etc.).
    RBLXUI_API void          SetStateStorage(RblxUIStorage* storage);                             // replace current window storage with our own (if you want to manipulate it yourself, typically clear subsection of it)
    RBLXUI_API RblxUIStorage* GetStateStorage();

    // Text Utilities
    RBLXUI_API RblxVec2        CalcTextSize(const char* text, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f);

    // Color Utilities
    RBLXUI_API RblxVec4        ColorConvertU32ToFloat4(RblxU32 in);
    RBLXUI_API RblxU32         ColorConvertFloat4ToU32(const RblxVec4& in);
    RBLXUI_API void          ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s, float& out_v);
    RBLXUI_API void          ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b);

    // Inputs Utilities: Raw Keyboard/Mouse/Gamepad Access
    // - Consider using the Shortcut() function instead of IsKeyPressed()/IsKeyChordPressed()! Shortcut() is easier to use and better featured (can do focus routing check).
    // - the RblxUIKey enum contains all possible keyboard, mouse and gamepad inputs (e.g. RblxUIKey_A, RblxUIKey_MouseLeft, RblxUIKey_GamepadDpadUp...).
    // - (legacy: before v1.87 (2022-02), we used RblxUIKey < 512 values to carry native/user indices as defined by each backends. This was obsoleted in 1.87 (2022-02) and completely removed in 1.91.5 (2024-11). See https://github.com/ocornut/rblxui/issues/4921)
    RBLXUI_API bool          IsKeyDown(RblxUIKey key);                                            // is key being held.
    RBLXUI_API bool          IsKeyPressed(RblxUIKey key, bool repeat = true);                     // was key pressed (went from !Down to Down)? Repeat rate uses io.KeyRepeatDelay / KeyRepeatRate.
    RBLXUI_API bool          IsKeyReleased(RblxUIKey key);                                        // was key released (went from Down to !Down)?
    RBLXUI_API bool          IsKeyChordPressed(RblxUIKeyChord key_chord);                         // was key chord (mods + key) pressed, e.g. you can pass 'RblxUIMod_Ctrl | RblxUIKey_S' as a key-chord. This doesn't do any routing or focus check, please consider using Shortcut() function instead.
    RBLXUI_API int           GetKeyPressedAmount(RblxUIKey key, float repeat_delay, float rate);  // uses provided repeat rate/delay. return a count, most often 0 or 1 but might be >1 if RepeatRate is small enough that DeltaTime > RepeatRate
    RBLXUI_API const char*   GetKeyName(RblxUIKey key);                                           // [DEBUG] returns English name of the key. Those names are provided for debugging purpose and are not meant to be saved persistently nor compared.
    RBLXUI_API void          SetNextFrameWantCaptureKeyboard(bool want_capture_keyboard);        // Override io.WantCaptureKeyboard flag next frame (said flag is left for your application to handle, typically when true it instructs your app to ignore inputs). e.g. force capture keyboard when your widget is being hovered. This is equivalent to setting "io.WantCaptureKeyboard = want_capture_keyboard"; after the next NewFrame() call.

    // Inputs Utilities: Shortcut Testing & Routing
    // - Typical use is e.g.: 'if (RblxUI::Shortcut(RblxUIMod_Ctrl | RblxUIKey_S)) { ... }'.
    // - Flags: Default route use RblxUIInputFlags_RouteFocused, but see RblxUIInputFlags_RouteGlobal and other options in RblxUIInputFlags_!
    // - Flags: Use RblxUIInputFlags_Repeat to support repeat.
    // - RblxUIKeyChord = a RblxUIKey + optional RblxUIMod_Alt/RblxUIMod_Ctrl/RblxUIMod_Shift/RblxUIMod_Super.
    //       RblxUIKey_C                          // Accepted by functions taking RblxUIKey or RblxUIKeyChord arguments
    //       RblxUIMod_Ctrl | RblxUIKey_C          // Accepted by functions taking RblxUIKeyChord arguments
    //   only RblxUIMod_XXX values are legal to combine with an RblxUIKey. You CANNOT combine two RblxUIKey values.
    // - The general idea is that several callers may register interest in a shortcut, and only one owner gets it.
    //      Parent   -> call Shortcut(Ctrl+S)    // When Parent is focused, Parent gets the shortcut.
    //        Child1 -> call Shortcut(Ctrl+S)    // When Child1 is focused, Child1 gets the shortcut (Child1 overrides Parent shortcuts)
    //        Child2 -> no call                  // When Child2 is focused, Parent gets the shortcut.
    //   The whole system is order independent, so if Child1 makes its calls before Parent, results will be identical.
    //   This is an important property as it facilitate working with foreign code or larger codebase.
    // - To understand the difference:
    //   - IsKeyChordPressed() compares mods and call IsKeyPressed()
    //     -> the function has no side-effect.
    //   - Shortcut() submits a route, routes are resolved, if it currently can be routed it calls IsKeyChordPressed()
    //     -> the function has (desirable) side-effects as it can prevents another call from getting the route.
    // - Visualize registered routes in 'Metrics/Debugger->Inputs'.
    RBLXUI_API bool          Shortcut(RblxUIKeyChord key_chord, RblxUIInputFlags flags = 0);
    RBLXUI_API void          SetNextItemShortcut(RblxUIKeyChord key_chord, RblxUIInputFlags flags = 0);

    // Inputs Utilities: Key/Input Ownership [BETA]
    // - One common use case would be to allow your items to disable standard inputs behaviors such
    //   as Tab or Alt key handling, Mouse Wheel scrolling, etc.
    //   e.g. `Button(...); if (SetItemKeyOwner(RblxUIKey_MouseWheelY)) { ... }` to make hovering/activating a button disable wheel for scrolling.
    // - Reminder RblxUIKey enum include access to mouse buttons and gamepad, so key ownership can apply to them.
    // - The return value of SetItemKeyOwner() says if ownership has been requested for the item, which is a shortcut to calling yet non-public TestKeyOwner() function.
    // - Many related features are still in rblxui_internal.h. For instance, most IsKeyXXX()/IsMouseXXX() functions have an owner-id-aware version.
    RBLXUI_API bool          SetItemKeyOwner(RblxUIKey key);                                      // Set key owner to last item ID if it is hovered or active. Return true when ownership has been set. Roughly equivalent to 'if (TestKeyOwner(key, GetItemID()) && (IsItemHovered() || IsItemActive())) { SetKeyOwner(key, GetItemID());'. 

    // Inputs Utilities: Mouse
    // - To refer to a mouse button, you may use named enums in your code e.g. RblxUIMouseButton_Left, RblxUIMouseButton_Right.
    // - You can also use regular integer: it is forever guaranteed that 0=Left, 1=Right, 2=Middle.
    // - Dragging operations are only reported after mouse has moved a certain distance away from the initial clicking position (see 'lock_threshold' and 'io.MouseDraggingThreshold')
    RBLXUI_API bool          IsMouseDown(RblxUIMouseButton button);                               // is mouse button held?
    RBLXUI_API bool          IsMouseClicked(RblxUIMouseButton button, bool repeat = false);       // did mouse button clicked? (went from !Down to Down). Same as GetMouseClickedCount() == 1.
    RBLXUI_API bool          IsMouseReleased(RblxUIMouseButton button);                           // did mouse button released? (went from Down to !Down)
    RBLXUI_API bool          IsMouseDoubleClicked(RblxUIMouseButton button);                      // did mouse button double-clicked? Same as GetMouseClickedCount() == 2. (note that a double-click will also report IsMouseClicked() == true)
    RBLXUI_API bool          IsMouseReleasedWithDelay(RblxUIMouseButton button, float delay);     // delayed mouse release (use very sparingly!). Generally used with 'delay >= io.MouseDoubleClickTime' + combined with a 'io.MouseClickedLastCount==1' test. This is a very rarely used UI idiom, but some apps use this: e.g. MS Explorer single click on an icon to rename.
    RBLXUI_API int           GetMouseClickedCount(RblxUIMouseButton button);                      // return the number of successive mouse-clicks at the time where a click happen (otherwise 0).
    RBLXUI_API bool          IsMouseHoveringRect(const RblxVec2& r_min, const RblxVec2& r_max, bool clip = true);// is mouse hovering given bounding rect (in screen space). clipped by current clipping settings, but disregarding of other consideration of focus/window ordering/popup-block.
    RBLXUI_API bool          IsMousePosValid(const RblxVec2* mouse_pos = NULL);                    // by convention we use (-FLT_MAX,-FLT_MAX) to denote that there is no mouse available
    RBLXUI_API bool          IsAnyMouseDown();                                                   // [WILL OBSOLETE] is any mouse button held? This was designed for backends, but prefer having backend maintain a mask of held mouse buttons, because upcoming input queue system will make this invalid.
    RBLXUI_API RblxVec2        GetMousePos();                                                      // shortcut to RblxUI::GetIO().MousePos provided by user, to be consistent with other calls
    RBLXUI_API RblxVec2        GetMousePosOnOpeningCurrentPopup();                                 // retrieve mouse position at the time of opening popup we have BeginPopup() into (helper to avoid user backing that value themselves)
    RBLXUI_API bool          IsMouseDragging(RblxUIMouseButton button, float lock_threshold = -1.0f);         // is mouse dragging? (uses io.MouseDraggingThreshold if lock_threshold < 0.0f)
    RBLXUI_API RblxVec2        GetMouseDragDelta(RblxUIMouseButton button = 0, float lock_threshold = -1.0f);   // return the delta from the initial clicking position while the mouse button is pressed or was just released. This is locked and return 0.0f until the mouse moves past a distance threshold at least once (uses io.MouseDraggingThreshold if lock_threshold < 0.0f)
    RBLXUI_API void          ResetMouseDragDelta(RblxUIMouseButton button = 0);                   //
    RBLXUI_API RblxUIMouseCursor GetMouseCursor();                                                // get desired mouse cursor shape. Important: reset in RblxUI::NewFrame(), this is updated during the frame. valid before Render(). If you use software rendering by setting io.MouseDrawCursor RblxUI will render those for you
    RBLXUI_API void          SetMouseCursor(RblxUIMouseCursor cursor_type);                       // set desired mouse cursor shape
    RBLXUI_API void          SetNextFrameWantCaptureMouse(bool want_capture_mouse);              // Override io.WantCaptureMouse flag next frame (said flag is left for your application to handle, typical when true it instructs your app to ignore inputs). This is equivalent to setting "io.WantCaptureMouse = want_capture_mouse;" after the next NewFrame() call.

    // Clipboard Utilities
    // - Also see the LogToClipboard() function to capture GUI into clipboard, or easily output text data to the clipboard.
    RBLXUI_API const char*   GetClipboardText();
    RBLXUI_API void          SetClipboardText(const char* text);

    // Settings/.Ini Utilities
    // - The disk functions are automatically called if io.IniFilename != NULL (default is "rblxui.ini").
    // - Set io.IniFilename to NULL to load/save manually. Read io.WantSaveIniSettings description about handling .ini saving manually.
    // - Important: default value "rblxui.ini" is relative to current working dir! Most apps will want to lock this to an absolute path (e.g. same path as executables).
    RBLXUI_API void          LoadIniSettingsFromDisk(const char* ini_filename);                  // call after CreateContext() and before the first call to NewFrame(). NewFrame() automatically calls LoadIniSettingsFromDisk(io.IniFilename).
    RBLXUI_API void          LoadIniSettingsFromMemory(const char* ini_data, size_t ini_size=0); // call after CreateContext() and before the first call to NewFrame() to provide .ini data from your own data source.
    RBLXUI_API void          SaveIniSettingsToDisk(const char* ini_filename);                    // this is automatically called (if io.IniFilename is not empty) a few seconds after any modification that should be reflected in the .ini file (and also by DestroyContext).
    RBLXUI_API const char*   SaveIniSettingsToMemory(size_t* out_ini_size = NULL);               // return a zero-terminated string with the .ini data which you can save by your own mean. call when io.WantSaveIniSettings is set, then save data by your own mean and clear io.WantSaveIniSettings.

    // Debug Utilities
    // - Your main debugging friend is the ShowMetricsWindow() function.
    // - Interactive tools are all accessible from the 'Dear RblxUI Demo->Tools' menu.
    // - Read https://github.com/ocornut/rblxui/wiki/Debug-Tools for a description of all available debug tools.
    RBLXUI_API void          DebugTextEncoding(const char* text);
    RBLXUI_API void          DebugFlashStyleColor(RblxUICol idx);
    RBLXUI_API void          DebugStartItemPicker();
    RBLXUI_API bool          DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_drawvert, size_t sz_drawidx); // This is called by RBLXUI_CHECKVERSION() macro.
#ifndef RBLXUI_DISABLE_DEBUG_TOOLS
    RBLXUI_API void          DebugLog(const char* fmt, ...)           RBLXUI_FMTARGS(1); // Call via RBLXUI_DEBUG_LOG() for maximum stripping in caller code!
    RBLXUI_API void          DebugLogV(const char* fmt, va_list args) RBLXUI_FMTLIST(1);
#endif

    // Memory Allocators
    // - Those functions are not reliant on the current context.
    // - DLL users: heaps and globals are not shared across DLL boundaries! You will need to call SetCurrentContext() + SetAllocatorFunctions()
    //   for each static/DLL boundary you are calling from. Read "Context and Memory Allocators" section of rblxui.cpp for more details.
    RBLXUI_API void          SetAllocatorFunctions(RblxUIMemAllocFunc alloc_func, RblxUIMemFreeFunc free_func, void* user_data = NULL);
    RBLXUI_API void          GetAllocatorFunctions(RblxUIMemAllocFunc* p_alloc_func, RblxUIMemFreeFunc* p_free_func, void** p_user_data);
    RBLXUI_API void*         MemAlloc(size_t size);
    RBLXUI_API void          MemFree(void* ptr);

    // (Optional) Platform/OS interface for multi-viewport support
    // Read comments around the RblxUIPlatformIO structure for more details.
    // Note: You may use GetWindowViewport() to get the current viewport of the current window.
    RBLXUI_API void          UpdatePlatformWindows();                                // call in main loop. will call CreateWindow/ResizeWindow/etc. platform functions for each secondary viewport, and DestroyWindow for each inactive viewport.
    RBLXUI_API void          RenderPlatformWindowsDefault(void* platform_render_arg = NULL, void* renderer_render_arg = NULL); // call in main loop. will call RenderWindow/SwapBuffers platform functions for each secondary viewport which doesn't have the RblxUIViewportFlags_Minimized flag set. May be reimplemented by user for custom rendering needs.
    RBLXUI_API void          DestroyPlatformWindows();                               // call DestroyWindow platform functions for all viewports. call from backend Shutdown() if you need to close platform windows before rblxui shutdown. otherwise will be called by DestroyContext().
    RBLXUI_API RblxUIViewport* FindViewportByID(RblxUIID viewport_id);                 // this is a helper for backends.
    RBLXUI_API RblxUIViewport* FindViewportByPlatformHandle(void* platform_handle);   // this is a helper for backends. the type platform_handle is decided by the backend (e.g. HWND, MyWindow*, RBLXWINwindow* etc.)

} // namespace RblxUI

//-----------------------------------------------------------------------------
// [SECTION] Flags & Enumerations
//-----------------------------------------------------------------------------

// Flags for RblxUI::Begin()
// (Those are per-window flags. There are shared flags in RblxUIIO: io.ConfigWindowsResizeFromEdges and io.ConfigWindowsMoveFromTitleBarOnly)
enum RblxUIWindowFlags_
{
    RblxUIWindowFlags_None                   = 0,
    RblxUIWindowFlags_NoTitleBar             = 1 << 0,   // Disable title-bar
    RblxUIWindowFlags_NoResize               = 1 << 1,   // Disable user resizing with the lower-right grip
    RblxUIWindowFlags_NoMove                 = 1 << 2,   // Disable user moving the window
    RblxUIWindowFlags_NoScrollbar            = 1 << 3,   // Disable scrollbars (window can still scroll with mouse or programmatically)
    RblxUIWindowFlags_NoScrollWithMouse      = 1 << 4,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
    RblxUIWindowFlags_NoCollapse             = 1 << 5,   // Disable user collapsing window by double-clicking on it. Also referred to as Window Menu Button (e.g. within a docking node).
    RblxUIWindowFlags_AlwaysAutoResize       = 1 << 6,   // Resize every window to its content every frame
    RblxUIWindowFlags_NoBackground           = 1 << 7,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
    RblxUIWindowFlags_NoSavedSettings        = 1 << 8,   // Never load/save settings in .ini file
    RblxUIWindowFlags_NoMouseInputs          = 1 << 9,   // Disable catching mouse, hovering test with pass through.
    RblxUIWindowFlags_MenuBar                = 1 << 10,  // Has a menu-bar
    RblxUIWindowFlags_HorizontalScrollbar    = 1 << 11,  // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(RblxVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in rblxui_demo in the "Horizontal Scrolling" section.
    RblxUIWindowFlags_NoFocusOnAppearing     = 1 << 12,  // Disable taking focus when transitioning from hidden to visible state
    RblxUIWindowFlags_NoBringToFrontOnFocus  = 1 << 13,  // Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)
    RblxUIWindowFlags_AlwaysVerticalScrollbar= 1 << 14,  // Always show vertical scrollbar (even if ContentSize.y < Size.y)
    RblxUIWindowFlags_AlwaysHorizontalScrollbar=1<< 15,  // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
    RblxUIWindowFlags_NoNavInputs            = 1 << 16,  // No keyboard/gamepad navigation within the window
    RblxUIWindowFlags_NoNavFocus             = 1 << 17,  // No focusing toward this window with keyboard/gamepad navigation (e.g. skipped by Ctrl+Tab)
    RblxUIWindowFlags_UnsavedDocument        = 1 << 18,  // Display a dot next to the title. When used in a tab/docking context, tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
    RblxUIWindowFlags_NoDocking              = 1 << 19,  // Disable docking of this window
    RblxUIWindowFlags_NoNav                  = RblxUIWindowFlags_NoNavInputs | RblxUIWindowFlags_NoNavFocus,
    RblxUIWindowFlags_NoDecoration           = RblxUIWindowFlags_NoTitleBar | RblxUIWindowFlags_NoResize | RblxUIWindowFlags_NoScrollbar | RblxUIWindowFlags_NoCollapse,
    RblxUIWindowFlags_NoInputs               = RblxUIWindowFlags_NoMouseInputs | RblxUIWindowFlags_NoNavInputs | RblxUIWindowFlags_NoNavFocus,

    // [Internal]
    RblxUIWindowFlags_DockNodeHost           = 1 << 23,  // Don't use! For internal use by Begin()/NewFrame()
    RblxUIWindowFlags_ChildWindow            = 1 << 24,  // Don't use! For internal use by BeginChild()
    RblxUIWindowFlags_Tooltip                = 1 << 25,  // Don't use! For internal use by BeginTooltip()
    RblxUIWindowFlags_Popup                  = 1 << 26,  // Don't use! For internal use by BeginPopup()
    RblxUIWindowFlags_Modal                  = 1 << 27,  // Don't use! For internal use by BeginPopupModal()
    RblxUIWindowFlags_ChildMenu              = 1 << 28,  // Don't use! For internal use by BeginMenu()

    // Obsolete names
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    //RblxUIWindowFlags_NavFlattened           = 1 << 29,  // Obsoleted in 1.90.9: moved to RblxUIChildFlags. BeginChild(name, size, 0, RblxUIWindowFlags_NavFlattened)           --> BeginChild(name, size, RblxUIChildFlags_NavFlattened, 0)
    //RblxUIWindowFlags_AlwaysUseWindowPadding = 1 << 30,  // Obsoleted in 1.90.0: moved to RblxUIChildFlags. BeginChild(name, size, 0, RblxUIWindowFlags_AlwaysUseWindowPadding) --> BeginChild(name, size, RblxUIChildFlags_AlwaysUseWindowPadding, 0)
#endif
};

// Flags for RblxUI::BeginChild()
// (Legacy: bit 0 must always correspond to RblxUIChildFlags_Borders to be backward compatible with old API using 'bool border = false'.)
// About using AutoResizeX/AutoResizeY flags:
// - May be combined with SetNextWindowSizeConstraints() to set a min/max size for each axis (see "Demo->Child->Auto-resize with Constraints").
// - Size measurement for a given axis is only performed when the child window is within visible boundaries, or is just appearing.
//   - This allows BeginChild() to return false when not within boundaries (e.g. when scrolling), which is more optimal. BUT it won't update its auto-size while clipped.
//     While not perfect, it is a better default behavior as the always-on performance gain is more valuable than the occasional "resizing after becoming visible again" glitch.
//   - You may also use RblxUIChildFlags_AlwaysAutoResize to force an update even when child window is not in view.
//     HOWEVER PLEASE UNDERSTAND THAT DOING SO WILL PREVENT BeginChild() FROM EVER RETURNING FALSE, disabling benefits of coarse clipping.
enum RblxUIChildFlags_
{
    RblxUIChildFlags_None                    = 0,
    RblxUIChildFlags_Borders                 = 1 << 0,   // Show an outer border and enable WindowPadding. (IMPORTANT: this is always == 1 == true for legacy reason)
    RblxUIChildFlags_AlwaysUseWindowPadding  = 1 << 1,   // Pad with style.WindowPadding even if no border are drawn (no padding by default for non-bordered child windows because it makes more sense)
    RblxUIChildFlags_ResizeX                 = 1 << 2,   // Allow resize from right border (layout direction). Enable .ini saving (unless RblxUIWindowFlags_NoSavedSettings passed to window flags)
    RblxUIChildFlags_ResizeY                 = 1 << 3,   // Allow resize from bottom border (layout direction). "
    RblxUIChildFlags_AutoResizeX             = 1 << 4,   // Enable auto-resizing width. Read "IMPORTANT: Size measurement" details above.
    RblxUIChildFlags_AutoResizeY             = 1 << 5,   // Enable auto-resizing height. Read "IMPORTANT: Size measurement" details above.
    RblxUIChildFlags_AlwaysAutoResize        = 1 << 6,   // Combined with AutoResizeX/AutoResizeY. Always measure size even when child is hidden, always return true, always disable clipping optimization! NOT RECOMMENDED.
    RblxUIChildFlags_FrameStyle              = 1 << 7,   // Style the child window like a framed item: use FrameBg, FrameRounding, FrameBorderSize, FramePadding instead of ChildBg, ChildRounding, ChildBorderSize, WindowPadding.
    RblxUIChildFlags_NavFlattened            = 1 << 8,   // [BETA] Share focus scope, allow keyboard/gamepad navigation to cross over parent border to this child or between sibling child windows.

    // Obsolete names
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    //RblxUIChildFlags_Border                = RblxUIChildFlags_Borders,  // Renamed in 1.91.1 (August 2024) for consistency.
#endif
};

// Flags for RblxUI::PushItemFlag()
// (Those are shared by all submitted items)
enum RblxUIItemFlags_
{
    RblxUIItemFlags_None                     = 0,        // (Default)
    RblxUIItemFlags_NoTabStop                = 1 << 0,   // false    // Disable keyboard tabbing. This is a "lighter" version of RblxUIItemFlags_NoNav.
    RblxUIItemFlags_NoNav                    = 1 << 1,   // false    // Disable any form of focusing (keyboard/gamepad directional navigation and SetKeyboardFocusHere() calls).
    RblxUIItemFlags_NoNavDefaultFocus        = 1 << 2,   // false    // Disable item being a candidate for default focus (e.g. used by title bar items).
    RblxUIItemFlags_ButtonRepeat             = 1 << 3,   // false    // Any button-like behavior will have repeat mode enabled (based on io.KeyRepeatDelay and io.KeyRepeatRate values). Note that you can also call IsItemActive() after any button to tell if it is being held.
    RblxUIItemFlags_AutoClosePopups          = 1 << 4,   // true     // MenuItem()/Selectable() automatically close their parent popup window.
    RblxUIItemFlags_AllowDuplicateId         = 1 << 5,   // false    // Allow submitting an item with the same identifier as an item already submitted this frame without triggering a warning tooltip if io.ConfigDebugHighlightIdConflicts is set.
    RblxUIItemFlags_Disabled                 = 1 << 6,   // false    // [Internal] Disable interactions. DOES NOT affect visuals. This is used by BeginDisabled()/EndDisabled() and only provided here so you can read back via GetItemFlags().
};

// Flags for RblxUI::InputText()
// (Those are per-item flags. There are shared flags in RblxUIIO: io.ConfigInputTextCursorBlink and io.ConfigInputTextEnterKeepActive)
enum RblxUIInputTextFlags_
{
    // Basic filters (also see RblxUIInputTextFlags_CallbackCharFilter)
    RblxUIInputTextFlags_None                = 0,
    RblxUIInputTextFlags_CharsDecimal        = 1 << 0,   // Allow 0123456789.+-*/
    RblxUIInputTextFlags_CharsHexadecimal    = 1 << 1,   // Allow 0123456789ABCDEFabcdef
    RblxUIInputTextFlags_CharsScientific     = 1 << 2,   // Allow 0123456789.+-*/eE (Scientific notation input)
    RblxUIInputTextFlags_CharsUppercase      = 1 << 3,   // Turn a..z into A..Z
    RblxUIInputTextFlags_CharsNoBlank        = 1 << 4,   // Filter out spaces, tabs

    // Inputs
    RblxUIInputTextFlags_AllowTabInput       = 1 << 5,   // Pressing TAB input a '\t' character into the text field
    RblxUIInputTextFlags_EnterReturnsTrue    = 1 << 6,   // Return 'true' when Enter is pressed (as opposed to every time the value was modified). Consider using IsItemDeactivatedAfterEdit() instead!
    RblxUIInputTextFlags_EscapeClearsAll     = 1 << 7,   // Escape key clears content if not empty, and deactivate otherwise (contrast to default behavior of Escape to revert)
    RblxUIInputTextFlags_CtrlEnterForNewLine = 1 << 8,   // In multi-line mode: validate with Enter, add new line with Ctrl+Enter (default is opposite: validate with Ctrl+Enter, add line with Enter). Note that Shift+Enter always enter a new line either way.

    // Other options
    RblxUIInputTextFlags_ReadOnly            = 1 << 9,   // Read-only mode
    RblxUIInputTextFlags_Password            = 1 << 10,  // Password mode, display all characters as '*', disable copy
    RblxUIInputTextFlags_AlwaysOverwrite     = 1 << 11,  // Overwrite mode
    RblxUIInputTextFlags_AutoSelectAll       = 1 << 12,  // Select entire text when first taking mouse focus
    RblxUIInputTextFlags_ParseEmptyRefVal    = 1 << 13,  // InputFloat(), InputInt(), InputScalar() etc. only: parse empty string as zero value.
    RblxUIInputTextFlags_DisplayEmptyRefVal  = 1 << 14,  // InputFloat(), InputInt(), InputScalar() etc. only: when value is zero, do not display it. Generally used with RblxUIInputTextFlags_ParseEmptyRefVal.
    RblxUIInputTextFlags_NoHorizontalScroll  = 1 << 15,  // Disable following the cursor horizontally
    RblxUIInputTextFlags_NoUndoRedo          = 1 << 16,  // Disable undo/redo. Note that input text owns the text data while active, if you want to provide your own undo/redo stack you need e.g. to call ClearActiveID().

    // Elide display / Alignment
    RblxUIInputTextFlags_ElideLeft           = 1 << 17,  // When text doesn't fit, elide left side to ensure right side stays visible. Useful for path/filenames. Single-line only!

    // Callback features
    RblxUIInputTextFlags_CallbackCompletion  = 1 << 18,  // Callback on pressing TAB (for completion handling)
    RblxUIInputTextFlags_CallbackHistory     = 1 << 19,  // Callback on pressing Up/Down arrows (for history handling)
    RblxUIInputTextFlags_CallbackAlways      = 1 << 20,  // Callback on each iteration. User code may query cursor position, modify text buffer.
    RblxUIInputTextFlags_CallbackCharFilter  = 1 << 21,  // Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
    RblxUIInputTextFlags_CallbackResize      = 1 << 22,  // Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow. Notify when the string wants to be resized (for string types which hold a cache of their Size). You will be provided a new BufSize in the callback and NEED to honor it. (see misc/cpp/rblxui_stdlib.h for an example of using this)
    RblxUIInputTextFlags_CallbackEdit        = 1 << 23,  // Callback on any edit. Note that InputText() already returns true on edit + you can always use IsItemEdited(). The callback is useful to manipulate the underlying buffer while focus is active.

    // Multi-line Word-Wrapping [BETA]
    // - Not well tested yet. Please report any incorrect cursor movement, selection behavior etc. bug to https://github.com/ocornut/rblxui/issues/3237.
    // - Wrapping style is not ideal. Wrapping of long words/sections (e.g. words larger than total available width) may be particularly unpleasing.
    // - Wrapping width needs to always account for the possibility of a vertical scrollbar.
    // - It is much slower than regular text fields.
    //   Ballpark estimate of cost on my 2019 desktop PC: for a 100 KB text buffer: +~0.3 ms (Optimized) / +~1.0 ms (Debug build).
    //   The CPU cost is very roughly proportional to text length, so a 10 KB buffer should cost about ten times less.
    RblxUIInputTextFlags_WordWrap            = 1 << 24,  // InputTextMultiline(): word-wrap lines that are too long.

    // Obsolete names
    //RblxUIInputTextFlags_AlwaysInsertMode  = RblxUIInputTextFlags_AlwaysOverwrite   // [renamed in 1.82] name was not matching behavior
};

// Flags for RblxUI::TreeNodeEx(), RblxUI::CollapsingHeader*()
enum RblxUITreeNodeFlags_
{
    RblxUITreeNodeFlags_None                 = 0,
    RblxUITreeNodeFlags_Selected             = 1 << 0,   // Draw as selected
    RblxUITreeNodeFlags_Framed               = 1 << 1,   // Draw frame with background (e.g. for CollapsingHeader)
    RblxUITreeNodeFlags_AllowOverlap         = 1 << 2,   // Hit testing will allow subsequent widgets to overlap this one. Require previous frame HoveredId to match before being usable. Shortcut to calling SetNextItemAllowOverlap().
    RblxUITreeNodeFlags_NoTreePushOnOpen     = 1 << 3,   // Don't do a TreePush() when open (e.g. for CollapsingHeader) = no extra indent nor pushing on ID stack
    RblxUITreeNodeFlags_NoAutoOpenOnLog      = 1 << 4,   // Don't automatically and temporarily open node when Logging is active (by default logging will automatically open tree nodes)
    RblxUITreeNodeFlags_DefaultOpen          = 1 << 5,   // Default node to be open
    RblxUITreeNodeFlags_OpenOnDoubleClick    = 1 << 6,   // Open on double-click instead of simple click (default for multi-select unless any _OpenOnXXX behavior is set explicitly). Both behaviors may be combined.
    RblxUITreeNodeFlags_OpenOnArrow          = 1 << 7,   // Open when clicking on the arrow part (default for multi-select unless any _OpenOnXXX behavior is set explicitly). Both behaviors may be combined.
    RblxUITreeNodeFlags_Leaf                 = 1 << 8,   // No collapsing, no arrow (use as a convenience for leaf nodes). Note: will always open a tree/id scope and return true. If you never use that scope, add RblxUITreeNodeFlags_NoTreePushOnOpen.
    RblxUITreeNodeFlags_Bullet               = 1 << 9,   // Display a bullet instead of arrow. IMPORTANT: node can still be marked open/close if you don't set the _Leaf flag!
    RblxUITreeNodeFlags_FramePadding         = 1 << 10,  // Use FramePadding (even for an unframed text node) to vertically align text baseline to regular widget height. Equivalent to calling AlignTextToFramePadding() before the node.
    RblxUITreeNodeFlags_SpanAvailWidth       = 1 << 11,  // Extend hit box to the right-most edge, even if not framed. This is not the default in order to allow adding other items on the same line without using AllowOverlap mode.
    RblxUITreeNodeFlags_SpanFullWidth        = 1 << 12,  // Extend hit box to the left-most and right-most edges (cover the indent area).
    RblxUITreeNodeFlags_SpanLabelWidth       = 1 << 13,  // Narrow hit box + narrow hovering highlight, will only cover the label text.
    RblxUITreeNodeFlags_SpanAllColumns       = 1 << 14,  // Frame will span all columns of its container table (label will still fit in current column)
    RblxUITreeNodeFlags_LabelSpanAllColumns  = 1 << 15,  // Label will span all columns of its container table
    //RblxUITreeNodeFlags_NoScrollOnOpen     = 1 << 16,  // FIXME: TODO: Disable automatic scroll on TreePop() if node got just open and contents is not visible
    RblxUITreeNodeFlags_NavLeftJumpsToParent = 1 << 17,  // Nav: left arrow moves back to parent. This is processed in TreePop() when there's an unfulfilled Left nav request remaining.
    RblxUITreeNodeFlags_CollapsingHeader     = RblxUITreeNodeFlags_Framed | RblxUITreeNodeFlags_NoTreePushOnOpen | RblxUITreeNodeFlags_NoAutoOpenOnLog,

    // [EXPERIMENTAL] Draw lines connecting TreeNode hierarchy. Discuss in GitHub issue #2920.
    // Default value is pulled from style.TreeLinesFlags. May be overridden in TreeNode calls.
    RblxUITreeNodeFlags_DrawLinesNone        = 1 << 18,  // No lines drawn
    RblxUITreeNodeFlags_DrawLinesFull        = 1 << 19,  // Horizontal lines to child nodes. Vertical line drawn down to TreePop() position: cover full contents. Faster (for large trees).
    RblxUITreeNodeFlags_DrawLinesToNodes     = 1 << 20,  // Horizontal lines to child nodes. Vertical line drawn down to bottom-most child node. Slower (for large trees).

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    RblxUITreeNodeFlags_NavLeftJumpsBackHere = RblxUITreeNodeFlags_NavLeftJumpsToParent,  // Renamed in 1.92.0
    //RblxUITreeNodeFlags_SpanTextWidth      = RblxUITreeNodeFlags_SpanLabelWidth,        // Renamed in 1.90.7
    //RblxUITreeNodeFlags_AllowItemOverlap   = RblxUITreeNodeFlags_AllowOverlap,          // Renamed in 1.89.7
#endif
};

// Flags for OpenPopup*(), BeginPopupContext*(), IsPopupOpen() functions.
// - IMPORTANT: If you ever used the left mouse button with BeginPopupContextXXX() helpers before 1.92.6: Read "API BREAKING CHANGES" 2026/01/07 (1.92.6) entry in rblxui.cpp or GitHub topic #9157.
// - Multiple buttons currently cannot be combined/or-ed in those functions (we could allow it later).
enum RblxUIPopupFlags_
{
    RblxUIPopupFlags_None                    = 0,
    RblxUIPopupFlags_MouseButtonLeft         = 1 << 2,   // For BeginPopupContext*(): open on Left Mouse release. Only one button allowed!
    RblxUIPopupFlags_MouseButtonRight        = 2 << 2,   // For BeginPopupContext*(): open on Right Mouse release. Only one button allowed! (default)
    RblxUIPopupFlags_MouseButtonMiddle       = 3 << 2,   // For BeginPopupContext*(): open on Middle Mouse release. Only one button allowed!
    RblxUIPopupFlags_NoReopen                = 1 << 5,   // For OpenPopup*(), BeginPopupContext*(): don't reopen same popup if already open (won't reposition, won't reinitialize navigation)
    //RblxUIPopupFlags_NoReopenAlwaysNavInit = 1 << 6,   // For OpenPopup*(), BeginPopupContext*(): focus and initialize navigation even when not reopening.
    RblxUIPopupFlags_NoOpenOverExistingPopup = 1 << 7,   // For OpenPopup*(), BeginPopupContext*(): don't open if there's already a popup at the same level of the popup stack
    RblxUIPopupFlags_NoOpenOverItems         = 1 << 8,   // For BeginPopupContextWindow(): don't return true when hovering items, only when hovering empty space
    RblxUIPopupFlags_AnyPopupId              = 1 << 10,  // For IsPopupOpen(): ignore the RblxUIID parameter and test for any popup.
    RblxUIPopupFlags_AnyPopupLevel           = 1 << 11,  // For IsPopupOpen(): search/test at any level of the popup stack (default test in the current level)
    RblxUIPopupFlags_AnyPopup                = RblxUIPopupFlags_AnyPopupId | RblxUIPopupFlags_AnyPopupLevel,
    RblxUIPopupFlags_MouseButtonShift_       = 2,        // [Internal]
    RblxUIPopupFlags_MouseButtonMask_        = 0x0C,     // [Internal]
    RblxUIPopupFlags_InvalidMask_            = 0x03,     // [Internal] Reserve legacy bits 0-1 to detect incorrectly passing 1 or 2 to the function.
};

// Flags for RblxUI::Selectable()
enum RblxUISelectableFlags_
{
    RblxUISelectableFlags_None               = 0,
    RblxUISelectableFlags_NoAutoClosePopups  = 1 << 0,   // Clicking this doesn't close parent popup window (overrides RblxUIItemFlags_AutoClosePopups)
    RblxUISelectableFlags_SpanAllColumns     = 1 << 1,   // Frame will span all columns of its container table (text will still fit in current column)
    RblxUISelectableFlags_AllowDoubleClick   = 1 << 2,   // Generate press events on double clicks too
    RblxUISelectableFlags_Disabled           = 1 << 3,   // Cannot be selected, display grayed out text
    RblxUISelectableFlags_AllowOverlap       = 1 << 4,   // Hit testing will allow subsequent widgets to overlap this one. Require previous frame HoveredId to match before being usable. Shortcut to calling SetNextItemAllowOverlap().
    RblxUISelectableFlags_Highlight          = 1 << 5,   // Make the item be displayed as if it is hovered
    RblxUISelectableFlags_SelectOnNav        = 1 << 6,   // Auto-select when moved into, unless Ctrl is held. Automatic when in a BeginMultiSelect() block.

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    RblxUISelectableFlags_DontClosePopups    = RblxUISelectableFlags_NoAutoClosePopups,   // Renamed in 1.91.0
    //RblxUISelectableFlags_AllowItemOverlap = RblxUISelectableFlags_AllowOverlap,        // Renamed in 1.89.7
#endif
};

// Flags for RblxUI::BeginCombo()
enum RblxUIComboFlags_
{
    RblxUIComboFlags_None                    = 0,
    RblxUIComboFlags_PopupAlignLeft          = 1 << 0,   // Align the popup toward the left by default
    RblxUIComboFlags_HeightSmall             = 1 << 1,   // Max ~4 items visible. Tip: If you want your combo popup to be a specific size you can use SetNextWindowSizeConstraints() prior to calling BeginCombo()
    RblxUIComboFlags_HeightRegular           = 1 << 2,   // Max ~8 items visible (default)
    RblxUIComboFlags_HeightLarge             = 1 << 3,   // Max ~20 items visible
    RblxUIComboFlags_HeightLargest           = 1 << 4,   // As many fitting items as possible
    RblxUIComboFlags_NoArrowButton           = 1 << 5,   // Display on the preview box without the square arrow button
    RblxUIComboFlags_NoPreview               = 1 << 6,   // Display only a square arrow button
    RblxUIComboFlags_WidthFitPreview         = 1 << 7,   // Width dynamically calculated from preview contents
    RblxUIComboFlags_HeightMask_             = RblxUIComboFlags_HeightSmall | RblxUIComboFlags_HeightRegular | RblxUIComboFlags_HeightLarge | RblxUIComboFlags_HeightLargest,
};

// Flags for RblxUI::BeginTabBar()
enum RblxUITabBarFlags_
{
    RblxUITabBarFlags_None                           = 0,
    RblxUITabBarFlags_Reorderable                    = 1 << 0,   // Allow manually dragging tabs to re-order them + New tabs are appended at the end of list
    RblxUITabBarFlags_AutoSelectNewTabs              = 1 << 1,   // Automatically select new tabs when they appear
    RblxUITabBarFlags_TabListPopupButton             = 1 << 2,   // Disable buttons to open the tab list popup
    RblxUITabBarFlags_NoCloseWithMiddleMouseButton   = 1 << 3,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You may handle this behavior manually on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
    RblxUITabBarFlags_NoTabListScrollingButtons      = 1 << 4,   // Disable scrolling buttons (apply when fitting policy is RblxUITabBarFlags_FittingPolicyScroll)
    RblxUITabBarFlags_NoTooltip                      = 1 << 5,   // Disable tooltips when hovering a tab
    RblxUITabBarFlags_DrawSelectedOverline           = 1 << 6,   // Draw selected overline markers over selected tab

    // Fitting/Resize policy
    RblxUITabBarFlags_FittingPolicyMixed             = 1 << 7,   // Shrink down tabs when they don't fit, until width is style.TabMinWidthShrink, then enable scrolling. Setting TabMinWidthShrink to FLT_MAX makes this behave like RblxUITabBarFlags_FittingPolicyScroll.
    RblxUITabBarFlags_FittingPolicyShrink            = 1 << 8,   // Shrink down tabs when they don't fit
    RblxUITabBarFlags_FittingPolicyScroll            = 1 << 9,   // Enable scrolling buttons when tabs don't fit
    RblxUITabBarFlags_FittingPolicyMask_             = RblxUITabBarFlags_FittingPolicyMixed | RblxUITabBarFlags_FittingPolicyShrink | RblxUITabBarFlags_FittingPolicyScroll,
    RblxUITabBarFlags_FittingPolicyDefault_          = RblxUITabBarFlags_FittingPolicyMixed,

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    RblxUITabBarFlags_FittingPolicyResizeDown        = RblxUITabBarFlags_FittingPolicyShrink, // Renamed in 1.92.2
#endif
};

// Flags for RblxUI::BeginTabItem()
enum RblxUITabItemFlags_
{
    RblxUITabItemFlags_None                          = 0,
    RblxUITabItemFlags_UnsavedDocument               = 1 << 0,   // Display a dot next to the title + set RblxUITabItemFlags_NoAssumedClosure.
    RblxUITabItemFlags_SetSelected                   = 1 << 1,   // Trigger flag to programmatically make the tab selected when calling BeginTabItem()
    RblxUITabItemFlags_NoCloseWithMiddleMouseButton  = 1 << 2,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You may handle this behavior manually on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
    RblxUITabItemFlags_NoPushId                      = 1 << 3,   // Don't call PushID()/PopID() on BeginTabItem()/EndTabItem()
    RblxUITabItemFlags_NoTooltip                     = 1 << 4,   // Disable tooltip for the given tab
    RblxUITabItemFlags_NoReorder                     = 1 << 5,   // Disable reordering this tab or having another tab cross over this tab
    RblxUITabItemFlags_Leading                       = 1 << 6,   // Enforce the tab position to the left of the tab bar (after the tab list popup button)
    RblxUITabItemFlags_Trailing                      = 1 << 7,   // Enforce the tab position to the right of the tab bar (before the scrolling buttons)
    RblxUITabItemFlags_NoAssumedClosure              = 1 << 8,   // Tab is selected when trying to close + closure is not immediately assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
};

// Flags for RblxUI::IsWindowFocused()
enum RblxUIFocusedFlags_
{
    RblxUIFocusedFlags_None                          = 0,
    RblxUIFocusedFlags_ChildWindows                  = 1 << 0,   // Return true if any children of the window is focused
    RblxUIFocusedFlags_RootWindow                    = 1 << 1,   // Test from root window (top most parent of the current hierarchy)
    RblxUIFocusedFlags_AnyWindow                     = 1 << 2,   // Return true if any window is focused. Important: If you are trying to tell how to dispatch your low-level inputs, do NOT use this. Use 'io.WantCaptureMouse' instead! Please read the FAQ!
    RblxUIFocusedFlags_NoPopupHierarchy              = 1 << 3,   // Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
    RblxUIFocusedFlags_DockHierarchy                 = 1 << 4,   // Consider docking hierarchy (treat dockspace host as parent of docked window) (when used with _ChildWindows or _RootWindow)
    RblxUIFocusedFlags_RootAndChildWindows           = RblxUIFocusedFlags_RootWindow | RblxUIFocusedFlags_ChildWindows,
};

// Flags for RblxUI::IsItemHovered(), RblxUI::IsWindowHovered()
// Note: if you are trying to check whether your mouse should be dispatched to Dear RblxUI or to your app, you should use 'io.WantCaptureMouse' instead! Please read the FAQ!
// Note: windows with the RblxUIWindowFlags_NoInputs flag are ignored by IsWindowHovered() calls.
enum RblxUIHoveredFlags_
{
    RblxUIHoveredFlags_None                          = 0,        // Return true if directly over the item/window, not obstructed by another window, not obstructed by an active popup or modal blocking inputs under them.
    RblxUIHoveredFlags_ChildWindows                  = 1 << 0,   // IsWindowHovered() only: Return true if any children of the window is hovered
    RblxUIHoveredFlags_RootWindow                    = 1 << 1,   // IsWindowHovered() only: Test from root window (top most parent of the current hierarchy)
    RblxUIHoveredFlags_AnyWindow                     = 1 << 2,   // IsWindowHovered() only: Return true if any window is hovered
    RblxUIHoveredFlags_NoPopupHierarchy              = 1 << 3,   // IsWindowHovered() only: Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
    RblxUIHoveredFlags_DockHierarchy                 = 1 << 4,   // IsWindowHovered() only: Consider docking hierarchy (treat dockspace host as parent of docked window) (when used with _ChildWindows or _RootWindow)
    RblxUIHoveredFlags_AllowWhenBlockedByPopup       = 1 << 5,   // Return true even if a popup window is normally blocking access to this item/window
    //RblxUIHoveredFlags_AllowWhenBlockedByModal     = 1 << 6,   // Return true even if a modal popup window is normally blocking access to this item/window. FIXME-TODO: Unavailable yet.
    RblxUIHoveredFlags_AllowWhenBlockedByActiveItem  = 1 << 7,   // Return true even if an active item is blocking access to this item/window. Useful for Drag and Drop patterns.
    RblxUIHoveredFlags_AllowWhenOverlappedByItem     = 1 << 8,   // IsItemHovered() only: Return true even if the item uses AllowOverlap mode and is overlapped by another hoverable item.
    RblxUIHoveredFlags_AllowWhenOverlappedByWindow   = 1 << 9,   // IsItemHovered() only: Return true even if the position is obstructed or overlapped by another window.
    RblxUIHoveredFlags_AllowWhenDisabled             = 1 << 10,  // IsItemHovered() only: Return true even if the item is disabled
    RblxUIHoveredFlags_NoNavOverride                 = 1 << 11,  // IsItemHovered() only: Disable using keyboard/gamepad navigation state when active, always query mouse
    RblxUIHoveredFlags_AllowWhenOverlapped           = RblxUIHoveredFlags_AllowWhenOverlappedByItem | RblxUIHoveredFlags_AllowWhenOverlappedByWindow,
    RblxUIHoveredFlags_RectOnly                      = RblxUIHoveredFlags_AllowWhenBlockedByPopup | RblxUIHoveredFlags_AllowWhenBlockedByActiveItem | RblxUIHoveredFlags_AllowWhenOverlapped,
    RblxUIHoveredFlags_RootAndChildWindows           = RblxUIHoveredFlags_RootWindow | RblxUIHoveredFlags_ChildWindows,

    // Tooltips mode
    // - typically used in IsItemHovered() + SetTooltip() sequence.
    // - this is a shortcut to pull flags from 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav' where you can reconfigure desired behavior.
    //   e.g. 'HoverFlagsForTooltipMouse' defaults to 'RblxUIHoveredFlags_Stationary | RblxUIHoveredFlags_DelayShort | RblxUIHoveredFlags_AllowWhenDisabled'.
    // - for frequently actioned or hovered items providing a tooltip, you want may to use RblxUIHoveredFlags_ForTooltip (stationary + delay) so the tooltip doesn't show too often.
    // - for items which main purpose is to be hovered, or items with low affordance, or in less consistent apps, prefer no delay or shorter delay.
    RblxUIHoveredFlags_ForTooltip                    = 1 << 12,  // Shortcut for standard flags when using IsItemHovered() + SetTooltip() sequence.

    // (Advanced) Mouse Hovering delays.
    // - generally you can use RblxUIHoveredFlags_ForTooltip to use application-standardized flags.
    // - use those if you need specific overrides.
    RblxUIHoveredFlags_Stationary                    = 1 << 13,  // Require mouse to be stationary for style.HoverStationaryDelay (~0.15 sec) _at least one time_. After this, can move on same item/window. Using the stationary test tends to reduces the need for a long delay.
    RblxUIHoveredFlags_DelayNone                     = 1 << 14,  // IsItemHovered() only: Return true immediately (default). As this is the default you generally ignore this.
    RblxUIHoveredFlags_DelayShort                    = 1 << 15,  // IsItemHovered() only: Return true after style.HoverDelayShort elapsed (~0.15 sec) (shared between items) + requires mouse to be stationary for style.HoverStationaryDelay (once per item).
    RblxUIHoveredFlags_DelayNormal                   = 1 << 16,  // IsItemHovered() only: Return true after style.HoverDelayNormal elapsed (~0.40 sec) (shared between items) + requires mouse to be stationary for style.HoverStationaryDelay (once per item).
    RblxUIHoveredFlags_NoSharedDelay                 = 1 << 17,  // IsItemHovered() only: Disable shared delay system where moving from one item to the next keeps the previous timer for a short time (standard for tooltips with long delays)
};

// Flags for RblxUI::DockSpace(), shared/inherited by child nodes.
// (Some flags can be applied to individual nodes directly)
// FIXME-DOCK: Also see RblxUIDockNodeFlagsPrivate_ which may involve using the WIP and internal DockBuilder api.
enum RblxUIDockNodeFlags_
{
    RblxUIDockNodeFlags_None                         = 0,
    RblxUIDockNodeFlags_KeepAliveOnly                = 1 << 0,   //       // Don't display the dockspace node but keep it alive. Windows docked into this dockspace node won't be undocked.
    //RblxUIDockNodeFlags_NoCentralNode              = 1 << 1,   //       // Disable Central Node (the node which can stay empty)
    RblxUIDockNodeFlags_NoDockingOverCentralNode     = 1 << 2,   //       // Disable docking over the Central Node, which will be always kept empty.
    RblxUIDockNodeFlags_PassthruCentralNode          = 1 << 3,   //       // Enable passthru dockspace: 1) DockSpace() will render a RblxUICol_WindowBg background covering everything excepted the Central Node when empty. Meaning the host window should probably use SetNextWindowBgAlpha(0.0f) prior to Begin() when using this. 2) When Central Node is empty: let inputs pass-through + won't display a DockingEmptyBg background. See demo for details.
    RblxUIDockNodeFlags_NoDockingSplit               = 1 << 4,   //       // Disable other windows/nodes from splitting this node.
    RblxUIDockNodeFlags_NoResize                     = 1 << 5,   // Saved // Disable resizing node using the splitter/separators. Useful with programmatically setup dockspaces.
    RblxUIDockNodeFlags_AutoHideTabBar               = 1 << 6,   //       // Tab bar will automatically hide when there is a single window in the dock node.
    RblxUIDockNodeFlags_NoUndocking                  = 1 << 7,   //       // Disable undocking this node.

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    RblxUIDockNodeFlags_NoSplit                      = RblxUIDockNodeFlags_NoDockingSplit, // Renamed in 1.90
    RblxUIDockNodeFlags_NoDockingInCentralNode       = RblxUIDockNodeFlags_NoDockingOverCentralNode, // Renamed in 1.90
#endif
};

// Flags for RblxUI::BeginDragDropSource(), RblxUI::AcceptDragDropPayload()
enum RblxUIDragDropFlags_
{
    RblxUIDragDropFlags_None                         = 0,
    // BeginDragDropSource() flags
    RblxUIDragDropFlags_SourceNoPreviewTooltip       = 1 << 0,   // Disable preview tooltip. By default, a successful call to BeginDragDropSource opens a tooltip so you can display a preview or description of the source contents. This flag disables this behavior.
    RblxUIDragDropFlags_SourceNoDisableHover         = 1 << 1,   // By default, when dragging we clear data so that IsItemHovered() will return false, to avoid subsequent user code submitting tooltips. This flag disables this behavior so you can still call IsItemHovered() on the source item.
    RblxUIDragDropFlags_SourceNoHoldToOpenOthers     = 1 << 2,   // Disable the behavior that allows to open tree nodes and collapsing header by holding over them while dragging a source item.
    RblxUIDragDropFlags_SourceAllowNullID            = 1 << 3,   // Allow items such as Text(), Image() that have no unique identifier to be used as drag source, by manufacturing a temporary identifier based on their window-relative position. This is extremely unusual within the dear rblxui ecosystem and so we made it explicit.
    RblxUIDragDropFlags_SourceExtern                 = 1 << 4,   // External source (from outside of dear rblxui), won't attempt to read current item/window info. Will always return true. Only one Extern source can be active simultaneously.
    RblxUIDragDropFlags_PayloadAutoExpire            = 1 << 5,   // Automatically expire the payload if the source cease to be submitted (otherwise payloads are persisting while being dragged)
    RblxUIDragDropFlags_PayloadNoCrossContext        = 1 << 6,   // Hint to specify that the payload may not be copied outside current dear rblxui context.
    RblxUIDragDropFlags_PayloadNoCrossProcess        = 1 << 7,   // Hint to specify that the payload may not be copied outside current process.
    // AcceptDragDropPayload() flags
    RblxUIDragDropFlags_AcceptBeforeDelivery         = 1 << 10,  // AcceptDragDropPayload() will returns true even before the mouse button is released. You can then call IsDelivery() to test if the payload needs to be delivered.
    RblxUIDragDropFlags_AcceptNoDrawDefaultRect      = 1 << 11,  // Do not draw the default highlight rectangle when hovering over target.
    RblxUIDragDropFlags_AcceptNoPreviewTooltip       = 1 << 12,  // Request hiding the BeginDragDropSource tooltip from the BeginDragDropTarget site.
    RblxUIDragDropFlags_AcceptDrawAsHovered          = 1 << 13,  // Accepting item will render as if hovered. Useful for e.g. a Button() used as a drop target.
    RblxUIDragDropFlags_AcceptPeekOnly               = RblxUIDragDropFlags_AcceptBeforeDelivery | RblxUIDragDropFlags_AcceptNoDrawDefaultRect, // For peeking ahead and inspecting the payload before delivery.

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    RblxUIDragDropFlags_SourceAutoExpirePayload = RblxUIDragDropFlags_PayloadAutoExpire, // Renamed in 1.90.9
#endif
};

// Standard Drag and Drop payload types. You can define you own payload types using short strings. Types starting with '_' are defined by Dear RblxUI.
#define RBLXUI_PAYLOAD_TYPE_COLOR_3F     "_COL3F"    // float[3]: Standard type for colors, without alpha. User code may use this type.
#define RBLXUI_PAYLOAD_TYPE_COLOR_4F     "_COL4F"    // float[4]: Standard type for colors. User code may use this type.

// A primary data type
enum RblxUIDataType_
{
    RblxUIDataType_S8,       // signed char / char (with sensible compilers)
    RblxUIDataType_U8,       // unsigned char
    RblxUIDataType_S16,      // short
    RblxUIDataType_U16,      // unsigned short
    RblxUIDataType_S32,      // int
    RblxUIDataType_U32,      // unsigned int
    RblxUIDataType_S64,      // long long / __int64
    RblxUIDataType_U64,      // unsigned long long / unsigned __int64
    RblxUIDataType_Float,    // float
    RblxUIDataType_Double,   // double
    RblxUIDataType_Bool,     // bool (provided for user convenience, not supported by scalar widgets)
    RblxUIDataType_String,   // char* (provided for user convenience, not supported by scalar widgets)
    RblxUIDataType_COUNT
};

// A cardinal direction
enum RblxUIDir : int
{
    RblxUIDir_None    = -1,
    RblxUIDir_Left    = 0,
    RblxUIDir_Right   = 1,
    RblxUIDir_Up      = 2,
    RblxUIDir_Down    = 3,
    RblxUIDir_COUNT
};

// A sorting direction
enum RblxUISortDirection : RblxU8
{
    RblxUISortDirection_None         = 0,
    RblxUISortDirection_Ascending    = 1,    // Ascending = 0->9, A->Z etc.
    RblxUISortDirection_Descending   = 2     // Descending = 9->0, Z->A etc.
};

// A key identifier (RblxUIKey_XXX or RblxUIMod_XXX value): can represent Keyboard, Mouse and Gamepad values.
// All our named keys are >= 512. Keys value 0 to 511 are left unused and were legacy native/opaque key values (< 1.87).
// Support for legacy keys was completely removed in 1.91.5.
// Read details about the 1.87+ transition : https://github.com/ocornut/rblxui/issues/4921
// Note that "Keys" related to physical keys and are not the same concept as input "Characters", the latter are submitted via io.AddInputCharacter().
// The keyboard key enum values are named after the keys on a standard US keyboard, and on other keyboard types the keys reported may not match the keycaps.
enum RblxUIKey : int
{
    // Keyboard
    RblxUIKey_None = 0,
    RblxUIKey_NamedKey_BEGIN = 512,  // First valid key value (other than 0)

    RblxUIKey_Tab = 512,             // == RblxUIKey_NamedKey_BEGIN
    RblxUIKey_LeftArrow,
    RblxUIKey_RightArrow,
    RblxUIKey_UpArrow,
    RblxUIKey_DownArrow,
    RblxUIKey_PageUp,
    RblxUIKey_PageDown,
    RblxUIKey_Home,
    RblxUIKey_End,
    RblxUIKey_Insert,
    RblxUIKey_Delete,
    RblxUIKey_Backspace,
    RblxUIKey_Space,
    RblxUIKey_Enter,
    RblxUIKey_Escape,
    RblxUIKey_LeftCtrl, RblxUIKey_LeftShift, RblxUIKey_LeftAlt, RblxUIKey_LeftSuper,     // Also see RblxUIMod_Ctrl, RblxUIMod_Shift, RblxUIMod_Alt, RblxUIMod_Super below!
    RblxUIKey_RightCtrl, RblxUIKey_RightShift, RblxUIKey_RightAlt, RblxUIKey_RightSuper,
    RblxUIKey_Menu,
    RblxUIKey_0, RblxUIKey_1, RblxUIKey_2, RblxUIKey_3, RblxUIKey_4, RblxUIKey_5, RblxUIKey_6, RblxUIKey_7, RblxUIKey_8, RblxUIKey_9,
    RblxUIKey_A, RblxUIKey_B, RblxUIKey_C, RblxUIKey_D, RblxUIKey_E, RblxUIKey_F, RblxUIKey_G, RblxUIKey_H, RblxUIKey_I, RblxUIKey_J,
    RblxUIKey_K, RblxUIKey_L, RblxUIKey_M, RblxUIKey_N, RblxUIKey_O, RblxUIKey_P, RblxUIKey_Q, RblxUIKey_R, RblxUIKey_S, RblxUIKey_T,
    RblxUIKey_U, RblxUIKey_V, RblxUIKey_W, RblxUIKey_X, RblxUIKey_Y, RblxUIKey_Z,
    RblxUIKey_F1, RblxUIKey_F2, RblxUIKey_F3, RblxUIKey_F4, RblxUIKey_F5, RblxUIKey_F6,
    RblxUIKey_F7, RblxUIKey_F8, RblxUIKey_F9, RblxUIKey_F10, RblxUIKey_F11, RblxUIKey_F12,
    RblxUIKey_F13, RblxUIKey_F14, RblxUIKey_F15, RblxUIKey_F16, RblxUIKey_F17, RblxUIKey_F18,
    RblxUIKey_F19, RblxUIKey_F20, RblxUIKey_F21, RblxUIKey_F22, RblxUIKey_F23, RblxUIKey_F24,
    RblxUIKey_Apostrophe,        // '
    RblxUIKey_Comma,             // ,
    RblxUIKey_Minus,             // -
    RblxUIKey_Period,            // .
    RblxUIKey_Slash,             // /
    RblxUIKey_Semicolon,         // ;
    RblxUIKey_Equal,             // =
    RblxUIKey_LeftBracket,       // [
    RblxUIKey_Backslash,         // \ (this text inhibit multiline comment caused by backslash)
    RblxUIKey_RightBracket,      // ]
    RblxUIKey_GraveAccent,       // `
    RblxUIKey_CapsLock,
    RblxUIKey_ScrollLock,
    RblxUIKey_NumLock,
    RblxUIKey_PrintScreen,
    RblxUIKey_Pause,
    RblxUIKey_Keypad0, RblxUIKey_Keypad1, RblxUIKey_Keypad2, RblxUIKey_Keypad3, RblxUIKey_Keypad4,
    RblxUIKey_Keypad5, RblxUIKey_Keypad6, RblxUIKey_Keypad7, RblxUIKey_Keypad8, RblxUIKey_Keypad9,
    RblxUIKey_KeypadDecimal,
    RblxUIKey_KeypadDivide,
    RblxUIKey_KeypadMultiply,
    RblxUIKey_KeypadSubtract,
    RblxUIKey_KeypadAdd,
    RblxUIKey_KeypadEnter,
    RblxUIKey_KeypadEqual,
    RblxUIKey_AppBack,               // Available on some keyboard/mouses. Often referred as "Browser Back"
    RblxUIKey_AppForward,
    RblxUIKey_Oem102,                // Non-US backslash.

    // Gamepad
    // (analog values are 0.0f to 1.0f)
    // (download controller mapping PNG/PSD at http://dearrblxui.com/controls_sheets)
    //                              // XBOX        | SWITCH  | PLAYSTA. | -> ACTION
    RblxUIKey_GamepadStart,          // Menu        | +       | Options  |
    RblxUIKey_GamepadBack,           // View        | -       | Share    |
    RblxUIKey_GamepadFaceLeft,       // X           | Y       | Square   | Toggle Menu. Hold for Windowing mode (Focus/Move/Resize windows)
    RblxUIKey_GamepadFaceRight,      // B           | A       | Circle   | Cancel / Close / Exit
    RblxUIKey_GamepadFaceUp,         // Y           | X       | Triangle | Open Context Menu
    RblxUIKey_GamepadFaceDown,       // A           | B       | Cross    | Activate / Open / Toggle. Hold for 0.60f to Activate in Text Input mode (e.g. wired to an on-screen keyboard).
    RblxUIKey_GamepadDpadLeft,       // D-pad Left  | "       | "        | Move / Tweak / Resize Window (in Windowing mode)
    RblxUIKey_GamepadDpadRight,      // D-pad Right | "       | "        | Move / Tweak / Resize Window (in Windowing mode)
    RblxUIKey_GamepadDpadUp,         // D-pad Up    | "       | "        | Move / Tweak / Resize Window (in Windowing mode)
    RblxUIKey_GamepadDpadDown,       // D-pad Down  | "       | "        | Move / Tweak / Resize Window (in Windowing mode)
    RblxUIKey_GamepadL1,             // L Bumper    | L       | L1       | Tweak Slower / Focus Previous (in Windowing mode)
    RblxUIKey_GamepadR1,             // R Bumper    | R       | R1       | Tweak Faster / Focus Next (in Windowing mode)
    RblxUIKey_GamepadL2,             // L Trigger   | ZL      | L2       | [Analog]
    RblxUIKey_GamepadR2,             // R Trigger   | ZR      | R2       | [Analog]
    RblxUIKey_GamepadL3,             // L Stick     | L3      | L3       |
    RblxUIKey_GamepadR3,             // R Stick     | R3      | R3       |
    RblxUIKey_GamepadLStickLeft,     //             |         |          | [Analog] Move Window (in Windowing mode)
    RblxUIKey_GamepadLStickRight,    //             |         |          | [Analog] Move Window (in Windowing mode)
    RblxUIKey_GamepadLStickUp,       //             |         |          | [Analog] Move Window (in Windowing mode)
    RblxUIKey_GamepadLStickDown,     //             |         |          | [Analog] Move Window (in Windowing mode)
    RblxUIKey_GamepadRStickLeft,     //             |         |          | [Analog]
    RblxUIKey_GamepadRStickRight,    //             |         |          | [Analog]
    RblxUIKey_GamepadRStickUp,       //             |         |          | [Analog]
    RblxUIKey_GamepadRStickDown,     //             |         |          | [Analog]

    // Aliases: Mouse Buttons (auto-submitted from AddMouseButtonEvent() calls)
    // - This is mirroring the data also written to io.MouseDown[], io.MouseWheel, in a format allowing them to be accessed via standard key API.
    RblxUIKey_MouseLeft, RblxUIKey_MouseRight, RblxUIKey_MouseMiddle, RblxUIKey_MouseX1, RblxUIKey_MouseX2, RblxUIKey_MouseWheelX, RblxUIKey_MouseWheelY,

    // [Internal] Reserved for mod storage
    RblxUIKey_ReservedForModCtrl, RblxUIKey_ReservedForModShift, RblxUIKey_ReservedForModAlt, RblxUIKey_ReservedForModSuper,

    // [Internal] If you need to iterate all keys (for e.g. an input mapper) you may use RblxUIKey_NamedKey_BEGIN..RblxUIKey_NamedKey_END.
    RblxUIKey_NamedKey_END,
    RblxUIKey_NamedKey_COUNT = RblxUIKey_NamedKey_END - RblxUIKey_NamedKey_BEGIN,

    // Keyboard Modifiers (explicitly submitted by backend via AddKeyEvent() calls)
    // - Any functions taking a RblxUIKeyChord parameter can binary-or those with regular keys, e.g. Shortcut(RblxUIMod_Ctrl | RblxUIKey_S).
    // - Those are written back into io.KeyCtrl, io.KeyShift, io.KeyAlt, io.KeySuper for convenience,
    //   but may be accessed via standard key API such as IsKeyPressed(), IsKeyReleased(), querying duration etc.
    // - Code polling every key (e.g. an interface to detect a key press for input mapping) might want to ignore those
    //   and prefer using the real keys (e.g. RblxUIKey_LeftCtrl, RblxUIKey_RightCtrl instead of RblxUIMod_Ctrl).
    // - In theory the value of keyboard modifiers should be roughly equivalent to a logical or of the equivalent left/right keys.
    //   In practice: it's complicated; mods are often provided from different sources. Keyboard layout, IME, sticky keys and
    //   backends tend to interfere and break that equivalence. The safer decision is to relay that ambiguity down to the end-user...
    // - On macOS, we swap Cmd(Super) and Ctrl keys at the time of the io.AddKeyEvent() call.
    RblxUIMod_None                   = 0,
    RblxUIMod_Ctrl                   = 1 << 12, // Ctrl (non-macOS), Cmd (macOS)
    RblxUIMod_Shift                  = 1 << 13, // Shift
    RblxUIMod_Alt                    = 1 << 14, // Option/Menu
    RblxUIMod_Super                  = 1 << 15, // Windows/Super (non-macOS), Ctrl (macOS)
    RblxUIMod_Mask_                  = 0xF000,  // 4-bits

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    RblxUIKey_COUNT                  = RblxUIKey_NamedKey_END,    // Obsoleted in 1.91.5 because it was misleading (since named keys don't start at 0 anymore)
    RblxUIMod_Shortcut               = RblxUIMod_Ctrl,            // Removed in 1.90.7, you can now simply use RblxUIMod_Ctrl
    //RblxUIKey_ModCtrl = RblxUIMod_Ctrl, RblxUIKey_ModShift = RblxUIMod_Shift, RblxUIKey_ModAlt = RblxUIMod_Alt, RblxUIKey_ModSuper = RblxUIMod_Super, // Renamed in 1.89
    //RblxUIKey_KeyPadEnter = RblxUIKey_KeypadEnter,              // Renamed in 1.87
#endif
};

// Flags for Shortcut(), SetNextItemShortcut(),
// (and for upcoming extended versions of IsKeyPressed(), IsMouseClicked(), Shortcut(), SetKeyOwner(), SetItemKeyOwner() that are still in rblxui_internal.h)
// Don't mistake with RblxUIInputTextFlags! (which is for RblxUI::InputText() function)
enum RblxUIInputFlags_
{
    RblxUIInputFlags_None                    = 0,
    RblxUIInputFlags_Repeat                  = 1 << 0,   // Enable repeat. Return true on successive repeats. Default for legacy IsKeyPressed(). NOT Default for legacy IsMouseClicked(). MUST BE == 1.

    // Flags for Shortcut(), SetNextItemShortcut()
    // - Routing policies: RouteGlobal+OverActive >> RouteActive or RouteFocused (if owner is active item) >> RouteGlobal+OverFocused >> RouteFocused (if in focused window stack) >> RouteGlobal.
    // - Default policy is RouteFocused. Can select only 1 policy among all available.
    RblxUIInputFlags_RouteActive             = 1 << 10,  // Route to active item only.
    RblxUIInputFlags_RouteFocused            = 1 << 11,  // Route to windows in the focus stack (DEFAULT). Deep-most focused window takes inputs. Active item takes inputs over deep-most focused window.
    RblxUIInputFlags_RouteGlobal             = 1 << 12,  // Global route (unless a focused window or active item registered the route).
    RblxUIInputFlags_RouteAlways             = 1 << 13,  // Do not register route, poll keys directly.
    // - Routing options
    RblxUIInputFlags_RouteOverFocused        = 1 << 14,  // Option: global route: higher priority than focused route (unless active item in focused route).
    RblxUIInputFlags_RouteOverActive         = 1 << 15,  // Option: global route: higher priority than active item. Unlikely you need to use that: will interfere with every active items, e.g. Ctrl+A registered by InputText will be overridden by this. May not be fully honored as user/internal code is likely to always assume they can access keys when active.
    RblxUIInputFlags_RouteUnlessBgFocused    = 1 << 16,  // Option: global route: will not be applied if underlying background/void is focused (== no Dear RblxUI windows are focused). Useful for overlay applications.
    RblxUIInputFlags_RouteFromRootWindow     = 1 << 17,  // Option: route evaluated from the point of view of root window rather than current window.

    // Flags for SetNextItemShortcut()
    RblxUIInputFlags_Tooltip                 = 1 << 18,  // Automatically display a tooltip when hovering item [BETA] Unsure of right api (opt-in/opt-out)
};

// Configuration flags stored in io.ConfigFlags. Set by user/application.
// Note that nowadays most of our configuration options are in other RblxUIIO fields, e.g. io.ConfigWindowsMoveFromTitleBarOnly.
enum RblxUIConfigFlags_
{
    RblxUIConfigFlags_None                   = 0,
    RblxUIConfigFlags_NavEnableKeyboard      = 1 << 0,   // Master keyboard navigation enable flag. Enable full Tabbing + directional arrows + Space/Enter to activate. Note: some features such as basic Tabbing and CtrL+Tab are enabled by regardless of this flag (and may be disabled via other means, see #4828, #9218).
    RblxUIConfigFlags_NavEnableGamepad       = 1 << 1,   // Master gamepad navigation enable flag. Backend also needs to set RblxUIBackendFlags_HasGamepad.
    RblxUIConfigFlags_NoMouse                = 1 << 4,   // Instruct dear rblxui to disable mouse inputs and interactions.
    RblxUIConfigFlags_NoMouseCursorChange    = 1 << 5,   // Instruct backend to not alter mouse cursor shape and visibility. Use if the backend cursor changes are interfering with yours and you don't want to use SetMouseCursor() to change mouse cursor. You may want to honor requests from rblxui by reading GetMouseCursor() yourself instead.
    RblxUIConfigFlags_NoKeyboard             = 1 << 6,   // Instruct dear rblxui to disable keyboard inputs and interactions. This is done by ignoring keyboard events and clearing existing states.

    // [BETA] Docking
    RblxUIConfigFlags_DockingEnable          = 1 << 7,   // Docking enable flags.

    // [BETA] Viewports
    // When using viewports it is recommended that your default value for RblxUICol_WindowBg is opaque (Alpha=1.0) so transition to a viewport won't be noticeable.
    RblxUIConfigFlags_ViewportsEnable        = 1 << 10,  // Viewport enable flags (require both RblxUIBackendFlags_PlatformHasViewports + RblxUIBackendFlags_RendererHasViewports set by the respective backends)

    // [Unused] User storage (to allow your backend/engine to communicate to code that may be shared between multiple projects. Those flags are NOT used by core Dear RblxUI)
    RblxUIConfigFlags_IsSRGB                 = 1 << 20,  // Application is SRGB-aware.
    RblxUIConfigFlags_IsTouchScreen          = 1 << 21,  // Application is using a touch screen instead of a mouse.
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    RblxUIConfigFlags_NavEnableSetMousePos   = 1 << 2,   // [moved/renamed in 1.91.4] -> use bool io.ConfigNavMoveSetMousePos
    RblxUIConfigFlags_NavNoCaptureKeyboard   = 1 << 3,   // [moved/renamed in 1.91.4] -> use bool io.ConfigNavCaptureKeyboard
    RblxUIConfigFlags_DpiEnableScaleFonts    = 1 << 14,  // [moved/renamed in 1.92.0] -> use bool io.ConfigDpiScaleFonts
    RblxUIConfigFlags_DpiEnableScaleViewports= 1 << 15,  // [moved/renamed in 1.92.0] -> use bool io.ConfigDpiScaleViewports
#endif
};

// Backend capabilities flags stored in io.BackendFlags. Set by rblxui_impl_xxx or custom backend.
enum RblxUIBackendFlags_
{
    RblxUIBackendFlags_None                  = 0,
    RblxUIBackendFlags_HasGamepad            = 1 << 0,   // Backend Platform supports gamepad and currently has one connected.
    RblxUIBackendFlags_HasMouseCursors       = 1 << 1,   // Backend Platform supports honoring GetMouseCursor() value to change the OS cursor shape.
    RblxUIBackendFlags_HasSetMousePos        = 1 << 2,   // Backend Platform supports io.WantSetMousePos requests to reposition the OS mouse position (only used if io.ConfigNavMoveSetMousePos is set).
    RblxUIBackendFlags_RendererHasVtxOffset  = 1 << 3,   // Backend Renderer supports RblxDrawCmd::VtxOffset. This enables output of large meshes (64K+ vertices) while still using 16-bit indices.
    RblxUIBackendFlags_RendererHasTextures   = 1 << 4,   // Backend Renderer supports RblxTextureData requests to create/update/destroy textures. This enables incremental texture updates and texture reloads. See https://github.com/ocornut/rblxui/blob/master/docs/BACKENDS.md for instructions on how to upgrade your custom backend.

    // [BETA] Multi-Viewports
    RblxUIBackendFlags_RendererHasViewports  = 1 << 10,  // Backend Renderer supports multiple viewports.
    RblxUIBackendFlags_PlatformHasViewports  = 1 << 11,  // Backend Platform supports multiple viewports.
    RblxUIBackendFlags_HasMouseHoveredViewport=1 << 12,  // Backend Platform supports calling io.AddMouseViewportEvent() with the viewport under the mouse. IF POSSIBLE, ignore viewports with the RblxUIViewportFlags_NoInputs flag (Win32 backend, RBLXWIN 3.30+ backend can do this, SDL backend cannot). If this cannot be done, Dear RblxUI needs to use a flawed heuristic to find the viewport under.
    RblxUIBackendFlags_HasParentViewport     = 1 << 13,  // Backend Platform supports honoring viewport->ParentViewport/ParentViewportId value, by applying the corresponding parent/child relationship at the Platform level. Child windows always appear in front of their parent window.
};

// Enumeration for PushStyleColor() / PopStyleColor()
enum RblxUICol_
{
    RblxUICol_Text,
    RblxUICol_TextDisabled,
    RblxUICol_WindowBg,              // Background of normal windows
    RblxUICol_ChildBg,               // Background of child windows
    RblxUICol_PopupBg,               // Background of popups, menus, tooltips windows
    RblxUICol_Border,
    RblxUICol_BorderShadow,
    RblxUICol_FrameBg,               // Background of checkbox, radio button, plot, slider, text input
    RblxUICol_FrameBgHovered,
    RblxUICol_FrameBgActive,
    RblxUICol_TitleBg,               // Title bar
    RblxUICol_TitleBgActive,         // Title bar when focused
    RblxUICol_TitleBgCollapsed,      // Title bar when collapsed
    RblxUICol_MenuBarBg,
    RblxUICol_ScrollbarBg,
    RblxUICol_ScrollbarGrab,
    RblxUICol_ScrollbarGrabHovered,
    RblxUICol_ScrollbarGrabActive,
    RblxUICol_CheckMark,             // Checkbox tick and RadioButton circle
    RblxUICol_CheckboxSelectedBg,    // Checkbox background when Selected, otherwise use FrameBg
    RblxUICol_SliderGrab,
    RblxUICol_SliderGrabActive,
    RblxUICol_Button,
    RblxUICol_ButtonHovered,
    RblxUICol_ButtonActive,
    RblxUICol_Header,                // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
    RblxUICol_HeaderHovered,
    RblxUICol_HeaderActive,
    RblxUICol_Separator,
    RblxUICol_SeparatorHovered,
    RblxUICol_SeparatorActive,
    RblxUICol_ResizeGrip,            // Resize grip in lower-right and lower-left corners of windows.
    RblxUICol_ResizeGripHovered,
    RblxUICol_ResizeGripActive,
    RblxUICol_InputTextCursor,       // InputText cursor/caret
    RblxUICol_TabHovered,            // Tab background, when hovered
    RblxUICol_Tab,                   // Tab background, when tab-bar is focused & tab is unselected
    RblxUICol_TabSelected,           // Tab background, when tab-bar is focused & tab is selected
    RblxUICol_TabSelectedOverline,   // Tab horizontal overline, when tab-bar is focused & tab is selected
    RblxUICol_TabDimmed,             // Tab background, when tab-bar is unfocused & tab is unselected
    RblxUICol_TabDimmedSelected,     // Tab background, when tab-bar is unfocused & tab is selected
    RblxUICol_TabDimmedSelectedOverline,//..horizontal overline, when tab-bar is unfocused & tab is selected
    RblxUICol_DockingPreview,        // Preview overlay color when about to docking something
    RblxUICol_DockingEmptyBg,        // Background color for empty node (e.g. CentralNode with no window docked into it)
    RblxUICol_PlotLines,
    RblxUICol_PlotLinesHovered,
    RblxUICol_PlotHistogram,
    RblxUICol_PlotHistogramHovered,
    RblxUICol_TableHeaderBg,         // Table header background
    RblxUICol_TableBorderStrong,     // Table outer and header borders (prefer using Alpha=1.0 here)
    RblxUICol_TableBorderLight,      // Table inner borders (prefer using Alpha=1.0 here)
    RblxUICol_TableRowBg,            // Table row background (even rows)
    RblxUICol_TableRowBgAlt,         // Table row background (odd rows)
    RblxUICol_TextLink,              // Hyperlink color
    RblxUICol_TextSelectedBg,        // Selected text inside an InputText
    RblxUICol_TreeLines,             // Tree node hierarchy outlines when using RblxUITreeNodeFlags_DrawLines
    RblxUICol_DragDropTarget,        // Rectangle border highlighting a drop target
    RblxUICol_DragDropTargetBg,      // Rectangle background highlighting a drop target
    RblxUICol_UnsavedMarker,         // Unsaved Document marker (in window title and tabs)
    RblxUICol_NavCursor,             // Color of keyboard/gamepad navigation cursor/rectangle, when visible
    RblxUICol_NavWindowingHighlight, // Highlight window when using Ctrl+Tab
    RblxUICol_NavWindowingDimBg,     // Darken/colorize entire screen behind the Ctrl+Tab window list, when active
    RblxUICol_ModalWindowDimBg,      // Darken/colorize entire screen behind a modal window, when one is active
    RblxUICol_COUNT,

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    RblxUICol_TabActive = RblxUICol_TabSelected,                  // [renamed in 1.90.9]
    RblxUICol_TabUnfocused = RblxUICol_TabDimmed,                 // [renamed in 1.90.9]
    RblxUICol_TabUnfocusedActive = RblxUICol_TabDimmedSelected,   // [renamed in 1.90.9]
    RblxUICol_NavHighlight = RblxUICol_NavCursor,                 // [renamed in 1.91.4]
#endif
};

// Enumeration for PushStyleVar() / PopStyleVar() to temporarily modify the RblxUIStyle structure.
// - The enum only refers to fields of RblxUIStyle which makes sense to be pushed/popped inside UI code.
//   During initialization or between frames, feel free to just poke into RblxUIStyle directly.
// - Tip: Use your programming IDE navigation facilities on the names in the _second column_ below to find the actual members and their description.
//   - In Visual Studio: Ctrl+Comma ("Edit.GoToAll") can follow symbols inside comments, whereas Ctrl+F12 ("Edit.GoToImplementation") cannot.
//   - In Visual Studio w/ Visual Assist installed: Alt+G ("VAssistX.GoToImplementation") can also follow symbols inside comments.
//   - In VS Code, CLion, etc.: Ctrl+Click can follow symbols inside comments.
// - When changing this enum, you need to update the associated internal table GStyleVarInfo[] accordingly. This is where we link enum values to members offset/type.
enum RblxUIStyleVar_
{
    // Enum name -------------------------- // Member in RblxUIStyle structure (see RblxUIStyle for descriptions)
    RblxUIStyleVar_Alpha,                    // float     Alpha
    RblxUIStyleVar_DisabledAlpha,            // float     DisabledAlpha
    RblxUIStyleVar_WindowPadding,            // RblxVec2    WindowPadding
    RblxUIStyleVar_WindowRounding,           // float     WindowRounding
    RblxUIStyleVar_WindowBorderSize,         // float     WindowBorderSize
    RblxUIStyleVar_WindowMinSize,            // RblxVec2    WindowMinSize
    RblxUIStyleVar_WindowTitleAlign,         // RblxVec2    WindowTitleAlign
    RblxUIStyleVar_ChildRounding,            // float     ChildRounding
    RblxUIStyleVar_ChildBorderSize,          // float     ChildBorderSize
    RblxUIStyleVar_PopupRounding,            // float     PopupRounding
    RblxUIStyleVar_PopupBorderSize,          // float     PopupBorderSize
    RblxUIStyleVar_FramePadding,             // RblxVec2    FramePadding
    RblxUIStyleVar_FrameRounding,            // float     FrameRounding
    RblxUIStyleVar_FrameBorderSize,          // float     FrameBorderSize
    RblxUIStyleVar_ItemSpacing,              // RblxVec2    ItemSpacing
    RblxUIStyleVar_ItemInnerSpacing,         // RblxVec2    ItemInnerSpacing
    RblxUIStyleVar_IndentSpacing,            // float     IndentSpacing
    RblxUIStyleVar_CellPadding,              // RblxVec2    CellPadding
    RblxUIStyleVar_ScrollbarSize,            // float     ScrollbarSize
    RblxUIStyleVar_ScrollbarRounding,        // float     ScrollbarRounding
    RblxUIStyleVar_ScrollbarPadding,         // float     ScrollbarPadding
    RblxUIStyleVar_GrabMinSize,              // float     GrabMinSize
    RblxUIStyleVar_GrabRounding,             // float     GrabRounding
    RblxUIStyleVar_ImageRounding,            // float     ImageRounding
    RblxUIStyleVar_ImageBorderSize,          // float     ImageBorderSize
    RblxUIStyleVar_TabRounding,              // float     TabRounding
    RblxUIStyleVar_TabBorderSize,            // float     TabBorderSize
    RblxUIStyleVar_TabMinWidthBase,          // float     TabMinWidthBase
    RblxUIStyleVar_TabMinWidthShrink,        // float     TabMinWidthShrink
    RblxUIStyleVar_TabBarBorderSize,         // float     TabBarBorderSize
    RblxUIStyleVar_TabBarOverlineSize,       // float     TabBarOverlineSize
    RblxUIStyleVar_TableAngledHeadersAngle,  // float     TableAngledHeadersAngle
    RblxUIStyleVar_TableAngledHeadersTextAlign,// RblxVec2  TableAngledHeadersTextAlign
    RblxUIStyleVar_TreeLinesSize,            // float     TreeLinesSize
    RblxUIStyleVar_TreeLinesRounding,        // float     TreeLinesRounding
    RblxUIStyleVar_DragDropTargetRounding,   // float     DragDropTargetRounding
    RblxUIStyleVar_ButtonTextAlign,          // RblxVec2    ButtonTextAlign
    RblxUIStyleVar_SelectableTextAlign,      // RblxVec2    SelectableTextAlign
    RblxUIStyleVar_SeparatorSize,            // float     SeparatorSize
    RblxUIStyleVar_SeparatorTextBorderSize,  // float     SeparatorTextBorderSize
    RblxUIStyleVar_SeparatorTextAlign,       // RblxVec2    SeparatorTextAlign
    RblxUIStyleVar_SeparatorTextPadding,     // RblxVec2    SeparatorTextPadding
    RblxUIStyleVar_DockingSeparatorSize,     // float     DockingSeparatorSize
    RblxUIStyleVar_COUNT
};

// Flags for InvisibleButton() [extended in rblxui_internal.h]
enum RblxUIButtonFlags_
{
    RblxUIButtonFlags_None                   = 0,
    RblxUIButtonFlags_MouseButtonLeft        = 1 << 0,   // React on left mouse button (default)
    RblxUIButtonFlags_MouseButtonRight       = 1 << 1,   // React on right mouse button
    RblxUIButtonFlags_MouseButtonMiddle      = 1 << 2,   // React on center mouse button
    RblxUIButtonFlags_MouseButtonMask_       = RblxUIButtonFlags_MouseButtonLeft | RblxUIButtonFlags_MouseButtonRight | RblxUIButtonFlags_MouseButtonMiddle, // [Internal]
    RblxUIButtonFlags_EnableNav              = 1 << 3,   // InvisibleButton(): do not disable navigation/tabbing. Otherwise disabled by default.
    RblxUIButtonFlags_AllowOverlap           = 1 << 12,  // Hit testing will allow subsequent widgets to overlap this one. Require previous frame HoveredId to match before being usable. Shortcut to calling SetNextItemAllowOverlap().
};

// Flags for ColorEdit3() / ColorEdit4() / ColorPicker3() / ColorPicker4() / ColorButton()
enum RblxUIColorEditFlags_
{
    RblxUIColorEditFlags_None            = 0,
    RblxUIColorEditFlags_NoAlpha         = 1 << 1,   //              // ColorEdit, ColorPicker, ColorButton: ignore Alpha component (will only read 3 components from the input pointer).
    RblxUIColorEditFlags_NoPicker        = 1 << 2,   //              // ColorEdit: disable picker when clicking on color square.
    RblxUIColorEditFlags_NoOptions       = 1 << 3,   //              // ColorEdit: disable toggling options menu when right-clicking on inputs/small preview.
    RblxUIColorEditFlags_NoSmallPreview  = 1 << 4,   //              // ColorEdit, ColorPicker: disable color square preview next to the inputs. (e.g. to show only the inputs)
    RblxUIColorEditFlags_NoInputs        = 1 << 5,   //              // ColorEdit, ColorPicker: disable inputs sliders/text widgets (e.g. to show only the small preview color square).
    RblxUIColorEditFlags_NoTooltip       = 1 << 6,   //              // ColorEdit, ColorPicker, ColorButton: disable tooltip when hovering the preview.
    RblxUIColorEditFlags_NoLabel         = 1 << 7,   //              // ColorEdit, ColorPicker: disable display of inline text label (the label is still forwarded to the tooltip and picker).
    RblxUIColorEditFlags_NoSidePreview   = 1 << 8,   //              // ColorPicker: disable bigger color preview on right side of the picker, use small color square preview instead.
    RblxUIColorEditFlags_NoDragDrop      = 1 << 9,   //              // ColorEdit: disable drag and drop target/source. ColorButton: disable drag and drop source.
    RblxUIColorEditFlags_NoBorder        = 1 << 10,  //              // ColorButton: disable border (which is enforced by default)
    RblxUIColorEditFlags_NoColorMarkers  = 1 << 11,  //              // ColorEdit: disable rendering R/G/B/A color marker. May also be disabled globally by setting style.ColorMarkerSize = 0.

    // Alpha preview
    // - Prior to 1.91.8 (2025/01/21): alpha was made opaque in the preview by default using old name RblxUIColorEditFlags_AlphaPreview.
    // - We now display the preview as transparent by default. You can use RblxUIColorEditFlags_AlphaOpaque to use old behavior.
    // - The new flags may be combined better and allow finer controls.
    RblxUIColorEditFlags_AlphaOpaque     = 1 << 12,  //              // ColorEdit, ColorPicker, ColorButton: disable alpha in the preview,. Contrary to _NoAlpha it may still be edited when calling ColorEdit4()/ColorPicker4(). For ColorButton() this does the same as _NoAlpha.
    RblxUIColorEditFlags_AlphaNoBg       = 1 << 13,  //              // ColorEdit, ColorPicker, ColorButton: disable rendering a checkerboard background behind transparent color.
    RblxUIColorEditFlags_AlphaPreviewHalf= 1 << 14,  //              // ColorEdit, ColorPicker, ColorButton: display half opaque / half transparent preview.

    // User Options (right-click on widget to change some of them).
    RblxUIColorEditFlags_AlphaBar        = 1 << 18,  //              // ColorEdit, ColorPicker: show vertical alpha bar/gradient in picker.
    RblxUIColorEditFlags_HDR             = 1 << 19,  //              // (WIP) ColorEdit: Currently only disable 0.0f..1.0f limits in RGBA edition (note: you probably want to use RblxUIColorEditFlags_Float flag as well).
    RblxUIColorEditFlags_DisplayRGB      = 1 << 20,  // [Display]    // ColorEdit: override _display_ type among RGB/HSV/Hex. ColorPicker: select any combination using one or more of RGB/HSV/Hex.
    RblxUIColorEditFlags_DisplayHSV      = 1 << 21,  // [Display]    // "
    RblxUIColorEditFlags_DisplayHex      = 1 << 22,  // [Display]    // "
    RblxUIColorEditFlags_Uint8           = 1 << 23,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0..255.
    RblxUIColorEditFlags_Float           = 1 << 24,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0.0f..1.0f floats instead of 0..255 integers. No round-trip of value via integers.
    RblxUIColorEditFlags_PickerHueBar    = 1 << 25,  // [Picker]     // ColorPicker: bar for Hue, rectangle for Sat/Value.
    RblxUIColorEditFlags_PickerHueWheel  = 1 << 26,  // [Picker]     // ColorPicker: wheel for Hue, triangle for Sat/Value.
    RblxUIColorEditFlags_InputRGB        = 1 << 27,  // [Input]      // ColorEdit, ColorPicker: input and output data in RGB format.
    RblxUIColorEditFlags_InputHSV        = 1 << 28,  // [Input]      // ColorEdit, ColorPicker: input and output data in HSV format.

    // Defaults Options. You can set application defaults using SetColorEditOptions(). The intent is that you probably don't want to
    // override them in most of your calls. Let the user choose via the option menu and/or call SetColorEditOptions() once during startup.
    RblxUIColorEditFlags_DefaultOptions_ = RblxUIColorEditFlags_Uint8 | RblxUIColorEditFlags_DisplayRGB | RblxUIColorEditFlags_InputRGB | RblxUIColorEditFlags_PickerHueBar,

    // [Internal] Masks
    RblxUIColorEditFlags_AlphaMask_      = RblxUIColorEditFlags_NoAlpha | RblxUIColorEditFlags_AlphaOpaque | RblxUIColorEditFlags_AlphaNoBg | RblxUIColorEditFlags_AlphaPreviewHalf,
    RblxUIColorEditFlags_DisplayMask_    = RblxUIColorEditFlags_DisplayRGB | RblxUIColorEditFlags_DisplayHSV | RblxUIColorEditFlags_DisplayHex,
    RblxUIColorEditFlags_DataTypeMask_   = RblxUIColorEditFlags_Uint8 | RblxUIColorEditFlags_Float,
    RblxUIColorEditFlags_PickerMask_     = RblxUIColorEditFlags_PickerHueWheel | RblxUIColorEditFlags_PickerHueBar,
    RblxUIColorEditFlags_InputMask_      = RblxUIColorEditFlags_InputRGB | RblxUIColorEditFlags_InputHSV,

    // Obsolete names
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    RblxUIColorEditFlags_AlphaPreview = 0, // Removed in 1.91.8. This is the default now. Will display a checkerboard unless RblxUIColorEditFlags_AlphaNoBg is set.
#endif
    //RblxUIColorEditFlags_RGB = RblxUIColorEditFlags_DisplayRGB, RblxUIColorEditFlags_HSV = RblxUIColorEditFlags_DisplayHSV, RblxUIColorEditFlags_HEX = RblxUIColorEditFlags_DisplayHex  // [renamed in 1.69]
};

// Flags for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
// We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.
// (Those are per-item flags. There is shared behavior flag too: RblxUIIO: io.ConfigDragClickToInputText)
enum RblxUISliderFlags_
{
    RblxUISliderFlags_None               = 0,
    RblxUISliderFlags_Logarithmic        = 1 << 5,       // Make the widget logarithmic (linear otherwise). Consider using RblxUISliderFlags_NoRoundToFormat with this if using a format-string with small amount of digits.
    RblxUISliderFlags_NoRoundToFormat    = 1 << 6,       // Disable rounding underlying value to match precision of the display format string (e.g. %.3f values are rounded to those 3 digits).
    RblxUISliderFlags_NoInput            = 1 << 7,       // Disable Ctrl+Click or Enter key allowing to input text directly into the widget.
    RblxUISliderFlags_WrapAround         = 1 << 8,       // Enable wrapping around from max to min and from min to max. Only supported by DragXXX() functions for now.
    RblxUISliderFlags_ClampOnInput       = 1 << 9,       // Clamp value to min/max bounds when input manually with Ctrl+Click. By default Ctrl+Click allows going out of bounds.
    RblxUISliderFlags_ClampZeroRange     = 1 << 10,      // Clamp even if min==max==0.0f. Otherwise due to legacy reason DragXXX functions don't clamp with those values. When your clamping limits are dynamic you almost always want to use it.
    RblxUISliderFlags_NoSpeedTweaks      = 1 << 11,      // Disable keyboard modifiers altering tweak speed. Useful if you want to alter tweak speed yourself based on your own logic.
    RblxUISliderFlags_ColorMarkers       = 1 << 12,      // DragScalarN(), SliderScalarN(): Draw R/G/B/A color markers on each component.
    RblxUISliderFlags_AlwaysClamp        = RblxUISliderFlags_ClampOnInput | RblxUISliderFlags_ClampZeroRange,
    RblxUISliderFlags_InvalidMask_       = 0x7000000F,   // [Internal] We treat using those bits as being potentially a 'float power' argument from legacy API (obsoleted 2020-08) that has got miscast to this enum, and will trigger an assert if needed.
};

// Identify a mouse button.
// Those values are guaranteed to be stable and we frequently use 0/1 directly. Named enums provided for convenience.
enum RblxUIMouseButton_
{
    RblxUIMouseButton_Left = 0,
    RblxUIMouseButton_Right = 1,
    RblxUIMouseButton_Middle = 2,
    RblxUIMouseButton_COUNT = 5
};

// Enumeration for GetMouseCursor()
// User code may request backend to display given cursor by calling SetMouseCursor(), which is why we have some cursors that are marked unused here
enum RblxUIMouseCursor_
{
    RblxUIMouseCursor_None = -1,
    RblxUIMouseCursor_Arrow = 0,
    RblxUIMouseCursor_TextInput,         // When hovering over InputText, etc.
    RblxUIMouseCursor_ResizeAll,         // (Unused by Dear RblxUI functions)
    RblxUIMouseCursor_ResizeNS,          // When hovering over a horizontal border
    RblxUIMouseCursor_ResizeEW,          // When hovering over a vertical border or a column
    RblxUIMouseCursor_ResizeNESW,        // When hovering over the bottom-left corner of a window
    RblxUIMouseCursor_ResizeNWSE,        // When hovering over the bottom-right corner of a window
    RblxUIMouseCursor_Hand,              // (Unused by Dear RblxUI functions. Use for e.g. hyperlinks)
    RblxUIMouseCursor_Wait,              // When waiting for something to process/load.
    RblxUIMouseCursor_Progress,          // When waiting for something to process/load, but application is still interactive.
    RblxUIMouseCursor_NotAllowed,        // When hovering something with disallowed interaction. Usually a crossed circle.
    RblxUIMouseCursor_COUNT
};

// Enumeration for AddMouseSourceEvent() actual source of Mouse Input data.
// Historically we use "Mouse" terminology everywhere to indicate pointer data, e.g. MousePos, IsMousePressed(), io.AddMousePosEvent()
// But that "Mouse" data can come from different source which occasionally may be useful for application to know about.
// You can submit a change of pointer type using io.AddMouseSourceEvent().
enum RblxUIMouseSource : int
{
    RblxUIMouseSource_Mouse = 0,         // Input is coming from an actual mouse.
    RblxUIMouseSource_TouchScreen,       // Input is coming from a touch screen (no hovering prior to initial press, less precise initial press aiming, dual-axis wheeling possible).
    RblxUIMouseSource_Pen,               // Input is coming from a pressure/magnetic pen (often used in conjunction with high-sampling rates).
    RblxUIMouseSource_COUNT
};

// Enumeration for RblxUI::SetNextWindow***(), SetWindow***(), SetNextItem***() functions
// Represent a condition.
// Important: Treat as a regular enum! Do NOT combine multiple values using binary operators! All the functions above treat 0 as a shortcut to RblxUICond_Always.
enum RblxUICond_
{
    RblxUICond_None          = 0,        // No condition (always set the variable), same as _Always
    RblxUICond_Always        = 1 << 0,   // No condition (always set the variable), same as _None
    RblxUICond_Once          = 1 << 1,   // Set the variable once per runtime session (only the first call will succeed)
    RblxUICond_FirstUseEver  = 1 << 2,   // Set the variable if the object/window has no persistently saved data (no entry in .ini file)
    RblxUICond_Appearing     = 1 << 3,   // Set the variable if the object/window is appearing after being hidden/inactive (or the first time)
};

//-----------------------------------------------------------------------------
// [SECTION] Tables API flags and structures (RblxUITableFlags, RblxUITableColumnFlags, RblxUITableRowFlags, RblxUITableBgTarget, RblxUITableSortSpecs, RblxUITableColumnSortSpecs)
//-----------------------------------------------------------------------------

// Flags for RblxUI::BeginTable()
// - Important! Sizing policies have complex and subtle side effects, much more so than you would expect.
//   Read comments/demos carefully + experiment with live demos to get acquainted with them.
// - The DEFAULT sizing policies are:
//    - Default to RblxUITableFlags_SizingFixedFit    if ScrollX is on, or if host window has RblxUIWindowFlags_AlwaysAutoResize.
//    - Default to RblxUITableFlags_SizingStretchSame if ScrollX is off.
// - When ScrollX is off:
//    - Table defaults to RblxUITableFlags_SizingStretchSame -> all Columns defaults to RblxUITableColumnFlags_WidthStretch with same weight.
//    - Columns sizing policy allowed: Stretch (default), Fixed/Auto.
//    - Fixed Columns (if any) will generally obtain their requested width (unless the table cannot fit them all).
//    - Stretch Columns will share the remaining width according to their respective weight.
//    - Mixed Fixed/Stretch columns is possible but has various side-effects on resizing behaviors.
//      The typical use of mixing sizing policies is: any number of LEADING Fixed columns, followed by one or two TRAILING Stretch columns.
//      (this is because the visible order of columns have subtle but necessary effects on how they react to manual resizing).
// - When ScrollX is on:
//    - Table defaults to RblxUITableFlags_SizingFixedFit -> all Columns defaults to RblxUITableColumnFlags_WidthFixed
//    - Columns sizing policy allowed: Fixed/Auto mostly.
//    - Fixed Columns can be enlarged as needed. Table will show a horizontal scrollbar if needed.
//    - When using auto-resizing (non-resizable) fixed columns, querying the content width to use item right-alignment e.g. SetNextItemWidth(-FLT_MIN) doesn't make sense, would create a feedback loop.
//    - Using Stretch columns OFTEN DOES NOT MAKE SENSE if ScrollX is on, UNLESS you have specified a value for 'inner_width' in BeginTable().
//      If you specify a value for 'inner_width' then effectively the scrolling space is known and Stretch or mixed Fixed/Stretch columns become meaningful again.
// - Read on documentation at the top of rblxui_tables.cpp for details.
enum RblxUITableFlags_
{
    // Features
    RblxUITableFlags_None                       = 0,
    RblxUITableFlags_Resizable                  = 1 << 0,   // Enable resizing columns.
    RblxUITableFlags_Reorderable                = 1 << 1,   // Enable reordering columns in header row. (Need calling TableSetupColumn() + TableHeadersRow() to display headers, or using RblxUITableFlags_ContextMenuInBody to access context-menu without headers).
    RblxUITableFlags_Hideable                   = 1 << 2,   // Enable hiding/disabling columns in context menu.
    RblxUITableFlags_Sortable                   = 1 << 3,   // Enable sorting. Call TableGetSortSpecs() to obtain sort specs. Also see RblxUITableFlags_SortMulti and RblxUITableFlags_SortTristate.
    RblxUITableFlags_NoSavedSettings            = 1 << 4,   // Disable persisting columns order, width, visibility and sort settings in the .ini file.
    RblxUITableFlags_ContextMenuInBody          = 1 << 5,   // Right-click on columns body/contents will also display table context menu. By default it is available in TableHeadersRow().
    // Decorations
    RblxUITableFlags_RowBg                      = 1 << 6,   // Set each RowBg color with RblxUICol_TableRowBg or RblxUICol_TableRowBgAlt (equivalent of calling TableSetBgColor with RblxUITableBgFlags_RowBg0 on each row manually)
    RblxUITableFlags_BordersInnerH              = 1 << 7,   // Draw horizontal borders between rows.
    RblxUITableFlags_BordersOuterH              = 1 << 8,   // Draw horizontal borders at the top and bottom.
    RblxUITableFlags_BordersInnerV              = 1 << 9,   // Draw vertical borders between columns.
    RblxUITableFlags_BordersOuterV              = 1 << 10,  // Draw vertical borders on the left and right sides.
    RblxUITableFlags_BordersH                   = RblxUITableFlags_BordersInnerH | RblxUITableFlags_BordersOuterH, // Draw horizontal borders.
    RblxUITableFlags_BordersV                   = RblxUITableFlags_BordersInnerV | RblxUITableFlags_BordersOuterV, // Draw vertical borders.
    RblxUITableFlags_BordersInner               = RblxUITableFlags_BordersInnerV | RblxUITableFlags_BordersInnerH, // Draw inner borders.
    RblxUITableFlags_BordersOuter               = RblxUITableFlags_BordersOuterV | RblxUITableFlags_BordersOuterH, // Draw outer borders.
    RblxUITableFlags_Borders                    = RblxUITableFlags_BordersInner | RblxUITableFlags_BordersOuter,   // Draw all borders.
    RblxUITableFlags_NoBordersInBody            = 1 << 11,  // [ALPHA] Disable vertical borders in columns Body (borders will always appear in Headers). -> May move to style
    RblxUITableFlags_NoBordersInBodyUntilResize = 1 << 12,  // [ALPHA] Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers). -> May move to style
    // Sizing Policy (read above for defaults)
    RblxUITableFlags_SizingFixedFit             = 1 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching contents width.
    RblxUITableFlags_SizingFixedSame            = 2 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching the maximum contents width of all columns. Implicitly enable RblxUITableFlags_NoKeepColumnsVisible.
    RblxUITableFlags_SizingStretchProp          = 3 << 13,  // Columns default to _WidthStretch with default weights proportional to each columns contents widths.
    RblxUITableFlags_SizingStretchSame          = 4 << 13,  // Columns default to _WidthStretch with default weights all equal, unless overridden by TableSetupColumn().
    // Sizing Extra Options
    RblxUITableFlags_NoHostExtendX              = 1 << 16,  // Make outer width auto-fit to columns, overriding outer_size.x value. Only available when ScrollX/ScrollY are disabled and Stretch columns are not used.
    RblxUITableFlags_NoHostExtendY              = 1 << 17,  // Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit). Only available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.
    RblxUITableFlags_NoKeepColumnsVisible       = 1 << 18,  // Disable keeping column always minimally visible when ScrollX is off and table gets too small. Not recommended if columns are resizable.
    RblxUITableFlags_PreciseWidths              = 1 << 19,  // Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.
    // Clipping
    RblxUITableFlags_NoClip                     = 1 << 20,  // Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with TableSetupScrollFreeze().
    // Padding
    RblxUITableFlags_PadOuterX                  = 1 << 21,  // Default if BordersOuterV is on. Enable outermost padding. Generally desirable if you have headers.
    RblxUITableFlags_NoPadOuterX                = 1 << 22,  // Default if BordersOuterV is off. Disable outermost padding.
    RblxUITableFlags_NoPadInnerX                = 1 << 23,  // Disable inner padding between columns (double inner padding if BordersOuterV is on, single inner padding if BordersOuterV is off).
    // Scrolling
    RblxUITableFlags_ScrollX                    = 1 << 24,  // Enable horizontal scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size. Changes default sizing policy. Because this creates a child window, ScrollY is currently generally recommended when using ScrollX.
    RblxUITableFlags_ScrollY                    = 1 << 25,  // Enable vertical scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size.
    // Sorting
    RblxUITableFlags_SortMulti                  = 1 << 26,  // Hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).
    RblxUITableFlags_SortTristate               = 1 << 27,  // Allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).
    // Miscellaneous
    RblxUITableFlags_HighlightHoveredColumn     = 1 << 28,  // Highlight column headers when hovered (may evolve into a fuller highlight)

    // [Internal] Combinations and masks
    RblxUITableFlags_SizingMask_                = RblxUITableFlags_SizingFixedFit | RblxUITableFlags_SizingFixedSame | RblxUITableFlags_SizingStretchProp | RblxUITableFlags_SizingStretchSame,
};

// Flags for RblxUI::TableSetupColumn()
enum RblxUITableColumnFlags_
{
    // Input configuration flags
    RblxUITableColumnFlags_None                  = 0,
    RblxUITableColumnFlags_Disabled              = 1 << 0,   // Overriding/master disable flag: hide column, won't show in context menu (unlike calling TableSetColumnEnabled() which manipulates the user accessible state)
    RblxUITableColumnFlags_DefaultHide           = 1 << 1,   // Default as a hidden/disabled column.
    RblxUITableColumnFlags_DefaultSort           = 1 << 2,   // Default as a sorting column.
    RblxUITableColumnFlags_WidthStretch          = 1 << 3,   // Column will stretch. Preferable with horizontal scrolling disabled (default if table sizing policy is _SizingStretchSame or _SizingStretchProp).
    RblxUITableColumnFlags_WidthFixed            = 1 << 4,   // Column will not stretch. Preferable with horizontal scrolling enabled (default if table sizing policy is _SizingFixedFit and table is resizable).
    RblxUITableColumnFlags_NoResize              = 1 << 5,   // Disable manual resizing.
    RblxUITableColumnFlags_NoReorder             = 1 << 6,   // Disable manual reordering this column, this will also prevent other columns from crossing over this column.
    RblxUITableColumnFlags_NoHide                = 1 << 7,   // Disable ability to hide/disable this column.
    RblxUITableColumnFlags_NoClip                = 1 << 8,   // Disable clipping for this column (all NoClip columns will render in a same draw command).
    RblxUITableColumnFlags_NoSort                = 1 << 9,   // Disable ability to sort on this field (even if RblxUITableFlags_Sortable is set on the table).
    RblxUITableColumnFlags_NoSortAscending       = 1 << 10,  // Disable ability to sort in the ascending direction.
    RblxUITableColumnFlags_NoSortDescending      = 1 << 11,  // Disable ability to sort in the descending direction.
    RblxUITableColumnFlags_NoHeaderLabel         = 1 << 12,  // TableHeadersRow() will submit an empty label for this column. Convenient for some small columns. Name will still appear in context menu or in angled headers. You may append into this cell by calling TableSetColumnIndex() right after the TableHeadersRow() call.
    RblxUITableColumnFlags_NoHeaderWidth         = 1 << 13,  // Disable header text width contribution to automatic column width.
    RblxUITableColumnFlags_PreferSortAscending   = 1 << 14,  // Make the initial sort direction Ascending when first sorting on this column (default).
    RblxUITableColumnFlags_PreferSortDescending  = 1 << 15,  // Make the initial sort direction Descending when first sorting on this column.
    RblxUITableColumnFlags_IndentEnable          = 1 << 16,  // Use current Indent value when entering cell (default for column 0).
    RblxUITableColumnFlags_IndentDisable         = 1 << 17,  // Ignore current Indent value when entering cell (default for columns > 0). Indentation changes _within_ the cell will still be honored.
    RblxUITableColumnFlags_AngledHeader          = 1 << 18,  // TableHeadersRow() will submit an angled header row for this column. Note this will add an extra row.

    // Output status flags, read-only via TableGetColumnFlags()
    RblxUITableColumnFlags_IsEnabled             = 1 << 24,  // Status: is enabled == not hidden by user/api (referred to as "Hide" in _DefaultHide and _NoHide) flags.
    RblxUITableColumnFlags_IsVisible             = 1 << 25,  // Status: is visible == is enabled AND not clipped by scrolling.
    RblxUITableColumnFlags_IsSorted              = 1 << 26,  // Status: is currently part of the sort specs
    RblxUITableColumnFlags_IsHovered             = 1 << 27,  // Status: is hovered by mouse

    // [Internal] Combinations and masks
    RblxUITableColumnFlags_WidthMask_            = RblxUITableColumnFlags_WidthStretch | RblxUITableColumnFlags_WidthFixed,
    RblxUITableColumnFlags_IndentMask_           = RblxUITableColumnFlags_IndentEnable | RblxUITableColumnFlags_IndentDisable,
    RblxUITableColumnFlags_StatusMask_           = RblxUITableColumnFlags_IsEnabled | RblxUITableColumnFlags_IsVisible | RblxUITableColumnFlags_IsSorted | RblxUITableColumnFlags_IsHovered,
    RblxUITableColumnFlags_NoDirectResize_       = 1 << 30,  // [Internal] Disable user resizing this column directly (it may however we resized indirectly from its left edge)
};

// Flags for RblxUI::TableNextRow()
enum RblxUITableRowFlags_
{
    RblxUITableRowFlags_None                     = 0,
    RblxUITableRowFlags_Headers                  = 1 << 0,   // Identify header row (set default background color + width of its contents accounted differently for auto column width)
};

// Enum for RblxUI::TableSetBgColor()
// Background colors are rendering in 3 layers:
//  - Layer 0: draw with RowBg0 color if set, otherwise draw with ColumnBg0 if set.
//  - Layer 1: draw with RowBg1 color if set, otherwise draw with ColumnBg1 if set.
//  - Layer 2: draw with CellBg color if set.
// The purpose of the two row/columns layers is to let you decide if a background color change should override or blend with the existing color.
// When using RblxUITableFlags_RowBg on the table, each row has the RowBg0 color automatically set for odd/even rows.
// If you set the color of RowBg0 target, your color will override the existing RowBg0 color.
// If you set the color of RowBg1 or ColumnBg1 target, your color will blend over the RowBg0 color.
enum RblxUITableBgTarget_
{
    RblxUITableBgTarget_None                     = 0,
    RblxUITableBgTarget_RowBg0                   = 1,        // Set row background color 0 (generally used for background, automatically set when RblxUITableFlags_RowBg is used)
    RblxUITableBgTarget_RowBg1                   = 2,        // Set row background color 1 (generally used for selection marking)
    RblxUITableBgTarget_CellBg                   = 3,        // Set cell background color (top-most color)
};

// Sorting specifications for a table (often handling sort specs for a single column, occasionally more)
// Obtained by calling TableGetSortSpecs().
// When 'SpecsDirty == true' you can sort your data. It will be true with sorting specs have changed since last call, or the first time.
// Make sure to set 'SpecsDirty = false' after sorting, else you may wastefully sort your data every frame!
struct RblxUITableSortSpecs
{
    const RblxUITableColumnSortSpecs* Specs;     // Pointer to sort spec array.
    int                         SpecsCount;     // Sort spec count. Most often 1. May be > 1 when RblxUITableFlags_SortMulti is enabled. May be == 0 when RblxUITableFlags_SortTristate is enabled.
    bool                        SpecsDirty;     // Set to true when specs have changed since last time! Use this to sort again, then clear the flag.

    RblxUITableSortSpecs()       { memset((void*)this, 0, sizeof(*this)); }
};

// Sorting specification for one column of a table (sizeof == 12 bytes)
struct RblxUITableColumnSortSpecs
{
    RblxUIID                     ColumnUserID;       // User id of the column (if specified by a TableSetupColumn() call)
    RblxS16                       ColumnIndex;        // Index of the column
    RblxS16                       SortOrder;          // Index within parent RblxUITableSortSpecs (always stored in order starting from 0, tables sorted on a single criteria will always have a 0 here)
    RblxUISortDirection          SortDirection;      // RblxUISortDirection_Ascending or RblxUISortDirection_Descending

    RblxUITableColumnSortSpecs() { memset((void*)this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Helpers: Debug log, memory allocations macros, RblxVector<>
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Debug Logging into ShowDebugLogWindow(), tty and more.
//-----------------------------------------------------------------------------

#ifndef RBLXUI_DISABLE_DEBUG_TOOLS
#define RBLXUI_DEBUG_LOG(...)        RblxUI::DebugLog(__VA_ARGS__)
#else
#define RBLXUI_DEBUG_LOG(...)        ((void)0)
#endif

//-----------------------------------------------------------------------------
// RBLXUI_MALLOC(), RBLXUI_FREE(), RBLXUI_NEW(), RBLXUI_PLACEMENT_NEW(), RBLXUI_DELETE()
// We call C++ constructor on own allocated memory via the placement "new(ptr) Type()" syntax.
// Defining a custom placement new() with a custom parameter allows us to bypass including <new> which on some platforms complains when user has disabled exceptions.
//-----------------------------------------------------------------------------

struct RblxNewWrapper {};
inline void* operator new(size_t, RblxNewWrapper, void* ptr) { return ptr; }
inline void  operator delete(void*, RblxNewWrapper, void*)   {} // This is only required so we can use the symmetrical new()
#define RBLXUI_ALLOC(_SIZE)                     RblxUI::MemAlloc(_SIZE)
#define RBLXUI_FREE(_PTR)                       RblxUI::MemFree(_PTR)
#define RBLXUI_PLACEMENT_NEW(_PTR)              new(RblxNewWrapper(), _PTR)
#define RBLXUI_NEW(_TYPE)                       new(RblxNewWrapper(), RblxUI::MemAlloc(sizeof(_TYPE))) _TYPE
template<typename T> void RBLXUI_DELETE(T* p)   { if (p) { p->~T(); RblxUI::MemFree(p); } }

//-----------------------------------------------------------------------------
// RblxVector<>
// Lightweight std::vector<>-like class to avoid dragging dependencies (also, some implementations of STL with debug enabled are absurdly slow, we bypass it so our code runs fast in debug).
//-----------------------------------------------------------------------------
// - You generally do NOT need to care or use this ever. But we need to make it available in rblxui.h because some of our public structures are relying on it.
// - We use std-like naming convention here, which is a little unusual for this codebase.
// - Important: clear() frees memory, resize(0) keep the allocated buffer. We use resize(0) a lot to intentionally recycle allocated buffers across frames and amortize our costs.
// - Important: our implementation does NOT call C++ constructors/destructors, we treat everything as raw data! This is intentional but be extra mindful of that,
//   Do NOT use this class as a std::vector replacement in your own code! Many of the structures used by dear rblxui can be safely initialized by a zero-memset.
//-----------------------------------------------------------------------------

RBLXUI_MSVC_RUNTIME_CHECKS_OFF
template<typename T>
struct RblxVector
{
    int                 Size;
    int                 Capacity;
    T*                  Data;

    // Provide standard typedefs but we don't use them ourselves.
    typedef T                   value_type;
    typedef value_type*         iterator;
    typedef const value_type*   const_iterator;

    // Constructors, destructor
    inline RblxVector()                                       { Size = Capacity = 0; Data = NULL; }
    inline RblxVector(const RblxVector<T>& src)                 { Size = Capacity = 0; Data = NULL; operator=(src); }
    inline RblxVector<T>& operator=(const RblxVector<T>& src)   { clear(); resize(src.Size); if (Data && src.Data) memcpy(Data, src.Data, (size_t)Size * sizeof(T)); return *this; }
    inline ~RblxVector()                                      { if (Data) RBLXUI_FREE(Data); } // Important: does not destruct anything

    inline void         clear()                             { if (Data) { Size = Capacity = 0; RBLXUI_FREE(Data); Data = NULL; } }  // Important: does not destruct anything
    inline void         clear_delete()                      { for (int n = 0; n < Size; n++) RBLXUI_DELETE(Data[n]); clear(); }     // Important: never called automatically! always explicit.
    inline void         clear_destruct()                    { for (int n = 0; n < Size; n++) Data[n].~T(); clear(); }           // Important: never called automatically! always explicit.

    inline bool         empty() const                       { return Size == 0; }
    inline int          size() const                        { return Size; }
    inline int          size_in_bytes() const               { return Size * (int)sizeof(T); }
    inline int          max_size() const                    { return 0x7FFFFFFF / (int)sizeof(T); }
    inline int          capacity() const                    { return Capacity; }
    inline T&           operator[](int i)                   { RBLXUI_ASSERT(i >= 0 && i < Size); return Data[i]; }
    inline const T&     operator[](int i) const             { RBLXUI_ASSERT(i >= 0 && i < Size); return Data[i]; }

    inline T*           begin()                             { return Data; }
    inline const T*     begin() const                       { return Data; }
    inline T*           end()                               { return Data + Size; }
    inline const T*     end() const                         { return Data + Size; }
    inline T&           front()                             { RBLXUI_ASSERT(Size > 0); return Data[0]; }
    inline const T&     front() const                       { RBLXUI_ASSERT(Size > 0); return Data[0]; }
    inline T&           back()                              { RBLXUI_ASSERT(Size > 0); return Data[Size - 1]; }
    inline const T&     back() const                        { RBLXUI_ASSERT(Size > 0); return Data[Size - 1]; }
    inline void         swap(RblxVector<T>& rhs)              { int rhs_size = rhs.Size; rhs.Size = Size; Size = rhs_size; int rhs_cap = rhs.Capacity; rhs.Capacity = Capacity; Capacity = rhs_cap; T* rhs_data = rhs.Data; rhs.Data = Data; Data = rhs_data; }

    inline int          _grow_capacity(int sz) const        { int new_capacity = Capacity ? (Capacity + Capacity / 2) : 8; return new_capacity > sz ? new_capacity : sz; }
    inline void         resize(int new_size)                { if (new_size > Capacity) reserve(_grow_capacity(new_size)); Size = new_size; }
    inline void         resize(int new_size, const T& v)    { if (new_size > Capacity) reserve(_grow_capacity(new_size)); if (new_size > Size) for (int n = Size; n < new_size; n++) memcpy(&Data[n], &v, sizeof(v)); Size = new_size; }
    inline void         shrink(int new_size)                { RBLXUI_ASSERT(new_size <= Size); Size = new_size; } // Resize a vector to a smaller size, guaranteed not to cause a reallocation
    inline void         reserve(int new_capacity)           { if (new_capacity <= Capacity) return; T* new_data = (T*)RBLXUI_ALLOC((size_t)new_capacity * sizeof(T)); if (Data) { memcpy(new_data, Data, (size_t)Size * sizeof(T)); RBLXUI_FREE(Data); } Data = new_data; Capacity = new_capacity; }
    inline void         reserve_discard(int new_capacity)   { if (new_capacity <= Capacity) return; if (Data) RBLXUI_FREE(Data); Data = (T*)RBLXUI_ALLOC((size_t)new_capacity * sizeof(T)); Capacity = new_capacity; }

    // NB: It is illegal to call push_back/push_front/insert with a reference pointing inside the RblxVector data itself! e.g. v.push_back(v[10]) is forbidden.
    inline void         push_back(const T& v)               { if (Size == Capacity) reserve(_grow_capacity(Size + 1)); memcpy(&Data[Size], &v, sizeof(v)); Size++; }
    inline void         pop_back()                          { RBLXUI_ASSERT(Size > 0); Size--; }
    inline void         push_front(const T& v)              { if (Size == 0) push_back(v); else insert(Data, v); }
    inline T*           erase(const T* it)                  { RBLXUI_ASSERT(it >= Data && it < Data + Size); const ptrdiff_t off = it - Data; memmove(Data + off, Data + off + 1, ((size_t)Size - (size_t)off - 1) * sizeof(T)); Size--; return Data + off; }
    inline T*           erase(const T* it, const T* it_last){ RBLXUI_ASSERT(it >= Data && it < Data + Size && it_last >= it && it_last <= Data + Size); const ptrdiff_t count = it_last - it; const ptrdiff_t off = it - Data; memmove(Data + off, Data + off + count, ((size_t)Size - (size_t)off - (size_t)count) * sizeof(T)); Size -= (int)count; return Data + off; }
    inline T*           erase_unsorted(const T* it)         { RBLXUI_ASSERT(it >= Data && it < Data + Size);  const ptrdiff_t off = it - Data; if (it < Data + Size - 1) memcpy(Data + off, Data + Size - 1, sizeof(T)); Size--; return Data + off; }
    inline T*           insert(const T* it, const T& v)     { RBLXUI_ASSERT(it >= Data && it <= Data + Size); const ptrdiff_t off = it - Data; if (Size == Capacity) reserve(_grow_capacity(Size + 1)); if (off < (int)Size) memmove(Data + off + 1, Data + off, ((size_t)Size - (size_t)off) * sizeof(T)); memcpy(&Data[off], &v, sizeof(v)); Size++; return Data + off; }
    inline bool         contains(const T& v) const          { const T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data++ == v) return true; return false; }
    inline T*           find(const T& v)                    { T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline const T*     find(const T& v) const              { const T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline int          find_index(const T& v) const        { const T* data_end = Data + Size; const T* it = find(v); if (it == data_end) return -1; const ptrdiff_t off = it - Data; return (int)off; }
    inline bool         find_erase(const T& v)              { const T* it = find(v); if (it < Data + Size) { erase(it); return true; } return false; }
    inline bool         find_erase_unsorted(const T& v)     { const T* it = find(v); if (it < Data + Size) { erase_unsorted(it); return true; } return false; }
    inline int          index_from_ptr(const T* it) const   { RBLXUI_ASSERT(it >= Data && it < Data + Size); const ptrdiff_t off = it - Data; return (int)off; }
};
RBLXUI_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] RblxUIStyle
//-----------------------------------------------------------------------------
// You may modify the RblxUI::GetStyle() main instance during initialization and before NewFrame().
// During the frame, use RblxUI::PushStyleVar(RblxUIStyleVar_XXXX)/PopStyleVar() to alter the main style values,
// and RblxUI::PushStyleColor(RblxUICol_XXX)/PopStyleColor() for colors.
//-----------------------------------------------------------------------------

struct RblxUIStyle
{
    // Font scaling
    // - recap: RblxUI::GetFontSize() == FontSizeBase * (FontScaleMain * FontScaleDpi * other_scaling_factors)
    float       FontSizeBase;               // Current base font size before external global factors are applied. Use PushFont(NULL, size) to modify. Use RblxUI::GetFontSize() to obtain scaled value.
    float       FontScaleMain;              // Main global scale factor. May be set by application once, or exposed to end-user.
    float       FontScaleDpi;               // Additional global scale factor from viewport/monitor contents scale. In docking branch: when io.ConfigDpiScaleFonts is enabled, this is automatically overwritten when changing monitor DPI.

    float       Alpha;                      // Global alpha applies to everything in Dear RblxUI.
    float       DisabledAlpha;              // Additional alpha multiplier applied by BeginDisabled(). Multiply over current value of Alpha.
    RblxVec2      WindowPadding;              // Padding within a window.
    float       WindowRounding;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows. Large values tend to lead to variety of artifacts and are not recommended.
    float       WindowBorderSize;           // Thickness of border around windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    float       WindowBorderHoverPadding;   // Hit-testing extent outside/inside resizing border. Also extend determination of hovered window. Generally meaningfully larger than WindowBorderSize to make it easy to reach borders.
    RblxVec2      WindowMinSize;              // Minimum window size. This is a global setting. If you want to constrain individual windows, use SetNextWindowSizeConstraints().
    RblxVec2      WindowTitleAlign;           // Alignment for title bar text. Defaults to (0.0f,0.5f) for left-aligned,vertically centered.
    RblxUIDir    WindowMenuButtonPosition;   // Side of the collapsing/docking button in the title bar (None/Left/Right). Defaults to RblxUIDir_Left.
    float       ChildRounding;              // Radius of child window corners rounding. Set to 0.0f to have rectangular windows.
    float       ChildBorderSize;            // Thickness of border around child windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    float       PopupRounding;              // Radius of popup window corners rounding. (Note that tooltip windows use WindowRounding)
    float       PopupBorderSize;            // Thickness of border around popup/tooltip windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    RblxVec2      FramePadding;               // Padding within a framed rectangle (used by most widgets).
    float       FrameRounding;              // Radius of frame corners rounding. Set to 0.0f to have rectangular frame (used by most widgets).
    float       FrameBorderSize;            // Thickness of border around frames. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    RblxVec2      ItemSpacing;                // Horizontal and vertical spacing between widgets/lines.
    RblxVec2      ItemInnerSpacing;           // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label).
    RblxVec2      CellPadding;                // Padding within a table cell. Cellpadding.x is locked for entire table. CellPadding.y may be altered between different rows.
    RblxVec2      TouchExtraPadding;          // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
    float       IndentSpacing;              // Horizontal indentation when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
    float       ColumnsMinSpacing;          // Minimum horizontal spacing between two columns. Preferably > (FramePadding.x + 1).
    float       ScrollbarSize;              // Width of the vertical scrollbar, Height of the horizontal scrollbar.
    float       ScrollbarRounding;          // Radius of grab corners for scrollbar.
    float       ScrollbarPadding;           // Padding of scrollbar grab within its frame (same for both axes).
    float       GrabMinSize;                // Minimum width/height of a grab box for slider/scrollbar.
    float       GrabRounding;               // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
    float       LogSliderDeadzone;          // The size in pixels of the dead-zone around zero on logarithmic sliders that cross zero.
    float       ImageRounding;              // Rounding of Image() calls.
    float       ImageBorderSize;            // Thickness of border around Image() calls.
    float       TabRounding;                // Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.
    float       TabBorderSize;              // Thickness of border around tabs.
    float       TabMinWidthBase;            // Minimum tab width, to make tabs larger than their contents. TabBar buttons are not affected.
    float       TabMinWidthShrink;          // Minimum tab width after shrinking, when using RblxUITabBarFlags_FittingPolicyMixed policy.
    float       TabCloseButtonMinWidthSelected;     // -1: always visible. 0.0f: visible when hovered. >0.0f: visible when hovered if minimum width. FLT_MAX: never shrink, will behave like RblxUITabBarFlags_FittingPolicyScroll.
    float       TabCloseButtonMinWidthUnselected;   // -1: always visible. 0.0f: visible when hovered. >0.0f: visible when hovered if minimum width. FLT_MAX: never show close button when unselected.
    float       TabBarBorderSize;           // Thickness of tab-bar separator, which takes on the tab active color to denote focus.
    float       TabBarOverlineSize;         // Thickness of tab-bar overline, which highlights the selected tab-bar.
    float       TableAngledHeadersAngle;    // Angle of angled headers (supported values range from -50.0f degrees to +50.0f degrees).
    RblxVec2      TableAngledHeadersTextAlign;// Alignment of angled headers within the cell
    RblxUITreeNodeFlags TreeLinesFlags;      // Default way to draw lines connecting TreeNode hierarchy. RblxUITreeNodeFlags_DrawLinesNone or RblxUITreeNodeFlags_DrawLinesFull or RblxUITreeNodeFlags_DrawLinesToNodes.
    float       TreeLinesSize;              // Thickness of outlines when using RblxUITreeNodeFlags_DrawLines.
    float       TreeLinesRounding;          // Radius of lines connecting child nodes to the vertical line.
    float       DragDropTargetRounding;     // Radius of the drag and drop target frame. When <0.0f: use FrameRounding.
    float       DragDropTargetBorderSize;   // Thickness of the drag and drop target border.
    float       DragDropTargetPadding;      // Size to expand the drag and drop target from actual target item size.
    float       ColorMarkerSize;            // Size of R/G/B/A color markers for ColorEdit4() and for Drags/Sliders when using RblxUISliderFlags_ColorMarkers.
    RblxUIDir    ColorButtonPosition;        // Side of the color button in the ColorEdit4 widget (left/right). Defaults to RblxUIDir_Right.
    RblxVec2      ButtonTextAlign;            // Alignment of button text when button is larger than text. Defaults to (0.5f, 0.5f) (centered).
    RblxVec2      SelectableTextAlign;        // Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.
    float       InputTextCursorSize;        // Thickness of cursor/caret in InputText().
    float       SeparatorSize;              // Thickness of border in Separator(). Must be >= 1.0f.
    float       SeparatorTextBorderSize;    // Thickness of border in SeparatorText()
    RblxVec2      SeparatorTextAlign;         // Alignment of text within the separator. Defaults to (0.0f, 0.5f) (left aligned, center).
    RblxVec2      SeparatorTextPadding;       // Horizontal offset of text from each edge of the separator + spacing on other axis. Generally small values. .y is recommended to be == FramePadding.y.
    RblxVec2      DisplayWindowPadding;       // Apply to regular windows: amount which we enforce to keep visible when moving near edges of your screen.
    RblxVec2      DisplaySafeAreaPadding;     // Apply to every windows, menus, popups, tooltips: amount where we avoid displaying contents. Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).
    bool        DockingNodeHasCloseButton;  // Docking node has their own CloseButton() to close all docked windows.
    float       DockingSeparatorSize;       // Thickness of resizing border between docked windows
    float       MouseCursorScale;           // Scale software rendered mouse cursor (when io.MouseDrawCursor is enabled). We apply per-monitor DPI scaling over this scale. May be removed later.
    bool        AntiAliasedLines;           // Enable anti-aliased lines/borders. Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to RblxDrawList).
    bool        AntiAliasedLinesUseTex;     // Enable anti-aliased lines/borders using textures where possible. Require backend to render with bilinear filtering (NOT point/nearest filtering). Latched at the beginning of the frame (copied to RblxDrawList).
    bool        AntiAliasedFill;            // Enable anti-aliased edges around filled shapes (rounded rectangles, circles, etc.). Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to RblxDrawList).
    float       CurveTessellationTol;       // Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
    float       CircleTessellationMaxError; // Maximum error (in pixels) allowed when using AddCircle()/AddCircleFilled() or drawing rounded corner rectangles with no explicit segment count specified. Decrease for higher quality but more geometry.

    // Colors
    RblxVec4      Colors[RblxUICol_COUNT];

    // Behaviors
    // (It is possible to modify those fields mid-frame if specific behavior need it, unlike e.g. configuration fields in RblxUIIO)
    float             HoverStationaryDelay;     // Delay for IsItemHovered(RblxUIHoveredFlags_Stationary). Time required to consider mouse stationary.
    float             HoverDelayShort;          // Delay for IsItemHovered(RblxUIHoveredFlags_DelayShort). Usually used along with HoverStationaryDelay.
    float             HoverDelayNormal;         // Delay for IsItemHovered(RblxUIHoveredFlags_DelayNormal). "
    RblxUIHoveredFlags HoverFlagsForTooltipMouse;// Default flags when using IsItemHovered(RblxUIHoveredFlags_ForTooltip) or BeginItemTooltip()/SetItemTooltip() while using mouse.
    RblxUIHoveredFlags HoverFlagsForTooltipNav;  // Default flags when using IsItemHovered(RblxUIHoveredFlags_ForTooltip) or BeginItemTooltip()/SetItemTooltip() while using keyboard/gamepad.

    // [Internal]
    float       _MainScale;                 // FIXME-WIP: Reference scale, as applied by ScaleAllSizes(). PLEASE DO NOT USE THIS FOR NOW.
    float       _NextFrameFontSizeBase;     // FIXME: Temporary hack until we finish remaining work.

    // Functions
    RBLXUI_API   RblxUIStyle();
    RBLXUI_API   void ScaleAllSizes(float scale_factor); // Scale all spacing/padding/thickness values. Do not scale fonts. See comments in definition. Consider not calling this if your initial scale factor if <1.0.

    // Obsolete names
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    // TabMinWidthForCloseButton = TabCloseButtonMinWidthUnselected // Renamed in 1.91.9.
#endif
};

//-----------------------------------------------------------------------------
// [SECTION] RblxUIIO
//-----------------------------------------------------------------------------
// Communicate most settings and inputs/outputs to Dear RblxUI using this structure.
// Access via RblxUI::GetIO(). Read 'Programmer guide' section in .cpp file for general usage.
// It is generally expected that:
// - initialization: backends and user code writes to RblxUIIO.
// - main loop: backends writes to RblxUIIO, user code and rblxui code reads from RblxUIIO.
//-----------------------------------------------------------------------------
// Also see RblxUI::GetPlatformIO() and RblxUIPlatformIO struct for OS/platform related functions: clipboard, IME etc.
//-----------------------------------------------------------------------------

// [Internal] Storage used by IsKeyDown(), IsKeyPressed() etc functions.
// If prior to 1.87 you used io.KeysDownDuration[] (which was marked as internal), you should use GetKeyData(key)->DownDuration and *NOT* io.KeysData[key]->DownDuration.
struct RblxUIKeyData
{
    bool        Down;               // True for if key is down
    float       DownDuration;       // Duration the key has been down (<0.0f: not pressed, 0.0f: just pressed, >0.0f: time held)
    float       DownDurationPrev;   // Last frame duration the key has been down
    float       AnalogValue;        // 0.0f..1.0f for gamepad values
};

struct RblxUIIO
{
    //------------------------------------------------------------------
    // Configuration                            // Default value
    //------------------------------------------------------------------

    RblxUIConfigFlags   ConfigFlags;             // = 0              // See RblxUIConfigFlags_ enum. Set by user/application. Keyboard/Gamepad navigation options, etc.
    RblxUIBackendFlags  BackendFlags;            // = 0              // See RblxUIBackendFlags_ enum. Set by backend (rblxui_impl_xxx files or custom backend) to communicate features supported by the backend.
    RblxVec2      DisplaySize;                    // <unset>          // Main display size, in pixels (== GetMainViewport()->Size). May change every frame.
    RblxVec2      DisplayFramebufferScale;        // = (1, 1)         // Main display density. For retina display where window coordinates are different from framebuffer coordinates. This will affect font density + will end up in RblxDrawData::FramebufferScale.
    float       DeltaTime;                      // = 1.0f/60.0f     // Time elapsed since last frame, in seconds. May change every frame.
    float       IniSavingRate;                  // = 5.0f           // Minimum time between saving positions/sizes to .ini file, in seconds.
    const char* IniFilename;                    // = "rblxui.ini"    // Path to .ini file (important: default "rblxui.ini" is relative to current working dir!). Set NULL to disable automatic .ini loading/saving or if you want to manually call LoadIniSettingsXXX() / SaveIniSettingsXXX() functions.
    const char* LogFilename;                    // = "rblxui_log.txt"// Path to .log file (default parameter to RblxUI::LogToFile when no file is specified).
    void*       UserData;                       // = NULL           // Store your own data.

    // Font system
    RblxFontAtlas*Fonts;                          // <auto>           // Font atlas: load, rasterize and pack one or more fonts into a single texture.
    RblxFont*     FontDefault;                    // = NULL           // Font to use on NewFrame(). Use NULL to uses Fonts->Fonts[0].
    bool        FontAllowUserScaling;           // = false          // Allow user scaling text of individual window with Ctrl+Wheel.

    // Keyboard/Gamepad Navigation options
    bool        ConfigNavSwapGamepadButtons;    // = false          // Swap Activate<>Cancel (A<>B) buttons, matching typical "Nintendo/Japanese style" gamepad layout.
    bool        ConfigNavMoveSetMousePos;       // = false          // Directional/tabbing navigation teleports the mouse cursor. May be useful on TV/console systems where moving a virtual mouse is difficult. Will update io.MousePos and set io.WantSetMousePos=true.
    bool        ConfigNavCaptureKeyboard;       // = true           // Sets io.WantCaptureKeyboard when io.NavActive is set.
    bool        ConfigNavEscapeClearFocusItem;  // = true           // Pressing Escape can clear focused item + navigation id/highlight. Set to false if you want to always keep highlight on.
    bool        ConfigNavEscapeClearFocusWindow;// = false          // Pressing Escape can clear focused window as well (super set of io.ConfigNavEscapeClearFocusItem).
    bool        ConfigNavCursorVisibleAuto;     // = true           // Using directional navigation key makes the cursor visible. Mouse click hides the cursor.
    bool        ConfigNavCursorVisibleAlways;   // = false          // Navigation cursor is always visible.

    // Docking options (when RblxUIConfigFlags_DockingEnable is set)
    bool        ConfigDockingNoSplit;           // = false          // Simplified docking mode: disable window splitting, so docking is limited to merging multiple windows together into tab-bars.
    bool        ConfigDockingNoDockingOver;     // = false          // Simplified docking mode: disable window merging into a same tab-bar, so docking is limited to splitting windows.
    bool        ConfigDockingWithShift;         // = false          // Enable docking with holding Shift key (reduce visual noise, allows dropping in wider space)
    bool        ConfigDockingAlwaysTabBar;      // = false          // [BETA] [FIXME: This currently creates regression with auto-sizing and general overhead] Make every single floating window display within a docking node.
    bool        ConfigDockingTransparentPayload;// = false          // [BETA] Make window or viewport transparent when docking and only display docking boxes on the target viewport. Useful if rendering of multiple viewport cannot be synced. Best used with ConfigViewportsNoAutoMerge.

    // Viewport options (when RblxUIConfigFlags_ViewportsEnable is set)
    // (sorry for the amount of "NoXXXX" flags, which may be harder to reason about! may rework someday)
    bool        ConfigViewportsNoAutoMerge;     // = false;         // Set to make all floating rblxui windows always create their own viewport. Otherwise, they are merged into the main host viewports when overlapping it. May also set RblxUIViewportFlags_NoAutoMerge on individual viewport.
    bool        ConfigViewportsNoTaskBarIcon;   // = false          // Disable default OS task bar icon flag for secondary viewports. When a viewport doesn't want a task bar icon, RblxUIViewportFlags_NoTaskBarIcon will be set on it.
    bool        ConfigViewportsNoDecoration;    // = true           // Disable default OS window decoration flag for secondary viewports. When a viewport doesn't want window decorations, RblxUIViewportFlags_NoDecoration will be set on it. Enabling decoration can create subsequent issues at OS levels (e.g. minimum window size).
    bool        ConfigViewportsNoDefaultParent; // = true           // Disable setting OS window parent to main viewport by default. The platform backend is expected to honor `viewport->ParentViewportID` to setup a parent/child relationship between the OS windows (supported if RblxUIBackendFlags_HasParentViewport is set). When parented: child windows always appear in front of their parent. Set to false if you want viewports to automatically be parent of main viewport, otherwise all viewports will be top-level OS windows. Parent/child relationship may be set on a per-window basis using RblxUIWindowClass.
    bool        ConfigViewportsPlatformFocusSetsRblxUIFocus;//= true // When a platform window is focused (e.g. using Alt+Tab, clicking Platform Title Bar), apply corresponding focus on rblxui windows (may clear focus/active id from rblxui windows location in other platform windows). In principle this is better enabled but we provide an opt-out, because some Linux window managers tend to eagerly focus windows (e.g. on mouse hover, or even a simple window pos/size change).

    // DPI/Scaling options
    // This may keep evolving during 1.92.x releases. Expect some turbulence.
    bool        ConfigDpiScaleFonts;            // = false          // [EXPERIMENTAL] Automatically overwrite style.FontScaleDpi when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    bool        ConfigDpiScaleViewports;        // = false          // [EXPERIMENTAL] Scale Dear RblxUI and Platform Windows when Monitor DPI changes.

    // Miscellaneous options
    // (you can visualize and interact with all options in 'Demo->Configuration')
    bool        MouseDrawCursor;                // = false          // Request RblxUI to draw a mouse cursor for you (if you are on a platform without a mouse cursor). Cannot be easily renamed to 'io.ConfigXXX' because this is frequently used by backend implementations.
    bool        ConfigMacOSXBehaviors;          // = defined(__APPLE__) // Swap Cmd<>Ctrl keys + OS X style text editing cursor movement using Alt instead of Ctrl, Shortcuts using Cmd/Super instead of Ctrl, Line/Text Start and End using Cmd+Arrows instead of Home/End, Double click selects by word instead of selecting whole text, Multi-selection in lists uses Cmd/Super instead of Ctrl.
    bool        ConfigInputTrickleEventQueue;   // = true           // Enable input queue trickling: some types of events submitted during the same frame (e.g. button down + up) will be spread over multiple frames, improving interactions with low framerates.
    bool        ConfigInputTextCursorBlink;     // = true           // Enable blinking cursor (optional as some users consider it to be distracting).
    bool        ConfigInputTextEnterKeepActive; // = false          // [BETA] Pressing Enter will reactivate item and select all text (single-line only).
    bool        ConfigDragClickToInputText;     // = false          // [BETA] Enable turning DragXXX widgets into text input with a simple mouse click-release (without moving). Not desirable on devices without a keyboard.
    bool        ConfigWindowsResizeFromEdges;   // = true           // Enable resizing of windows from their edges and from the lower-left corner. This requires RblxUIBackendFlags_HasMouseCursors for better mouse cursor feedback. (This used to be a per-window RblxUIWindowFlags_ResizeFromAnySide flag)
    bool        ConfigWindowsMoveFromTitleBarOnly;  // = false      // Enable allowing to move windows only when clicking on their title bar. Does not apply to windows without a title bar.
    bool        ConfigWindowsCopyContentsWithCtrlC; // = false      // [EXPERIMENTAL] Ctrl+C copy the contents of focused window into the clipboard. Experimental because: (1) has known issues with nested Begin/End pairs (2) text output quality varies (3) text output is in submission order rather than spatial order.
    bool        ConfigScrollbarScrollByPage;    // = true           // Enable scrolling page by page when clicking outside the scrollbar grab. When disabled, always scroll to clicked location. When enabled, Shift+Click scrolls to clicked location.
    float       ConfigMemoryCompactTimer;       // = 60.0f          // Timer (in seconds) to free transient windows/tables memory buffers when unused. Set to -1.0f to disable.

    // Inputs Behaviors
    // (other variables, ones which are expected to be tweaked within UI code, are exposed in RblxUIStyle)
    float       MouseDoubleClickTime;           // = 0.30f          // Time for a double-click, in seconds.
    float       MouseDoubleClickMaxDist;        // = 6.0f           // Distance threshold to stay in to validate a double-click, in pixels.
    float       MouseDragThreshold;             // = 6.0f           // Distance threshold before considering we are dragging.
    float       KeyRepeatDelay;                 // = 0.275f         // When holding a key/button, time before it starts repeating, in seconds (for buttons in Repeat mode, etc.).
    float       KeyRepeatRate;                  // = 0.050f         // When holding a key/button, rate at which it repeats, in seconds.

    //------------------------------------------------------------------
    // Debug options
    //------------------------------------------------------------------

    // Options to configure Error Handling and how we handle recoverable errors [EXPERIMENTAL]
    // - Error recovery is provided as a way to facilitate:
    //    - Recovery after a programming error (native code or scripting language - the latter tends to facilitate iterating on code while running).
    //    - Recovery after running an exception handler or any error processing which may skip code after an error has been detected.
    // - Error recovery is not perfect nor guaranteed! It is a feature to ease development.
    //   You not are not supposed to rely on it in the course of a normal application run.
    // - Functions that support error recovery are using RBLXUI_ASSERT_USER_ERROR() instead of RBLXUI_ASSERT().
    // - By design, we do NOT allow error recovery to be 100% silent. One of the three options needs to be checked!
    // - Always ensure that on programmers seats you have at minimum Asserts or Tooltips enabled when making direct rblxui API calls!
    //   Otherwise it would severely hinder your ability to catch and correct mistakes!
    // Read https://github.com/ocornut/rblxui/wiki/Error-Handling for details.
    // - Programmer seats: keep asserts (default), or disable asserts and keep error tooltips (new and nice!)
    // - Non-programmer seats: maybe disable asserts, but make sure errors are resurfaced (tooltips, visible log entries, use callback etc.)
    // - Recovery after error/exception: record stack sizes with ErrorRecoveryStoreState(), disable assert, set log callback (to e.g. trigger high-level breakpoint), recover with ErrorRecoveryTryToRecoverState(), restore settings.
    bool        ConfigErrorRecovery;                // = true       // Enable error recovery support. Some errors won't be detected and lead to direct crashes if recovery is disabled.
    bool        ConfigErrorRecoveryEnableAssert;    // = true       // Enable asserts on recoverable error. By default call RBLXUI_ASSERT() when returning from a failing RBLXUI_ASSERT_USER_ERROR()
    bool        ConfigErrorRecoveryEnableDebugLog;  // = true       // Enable debug log output on recoverable errors.
    bool        ConfigErrorRecoveryEnableTooltip;   // = true       // Enable tooltip on recoverable errors. The tooltip include a way to enable asserts if they were disabled.

    // Option to enable various debug tools showing buttons that will call the RBLXUI_DEBUG_BREAK() macro.
    // - The Item Picker tool will be available regardless of this being enabled, in order to maximize its discoverability.
    // - Requires a debugger being attached, otherwise RBLXUI_DEBUG_BREAK() options will appear to crash your application.
    //   e.g. io.ConfigDebugIsDebuggerPresent = ::IsDebuggerPresent() on Win32, or refer to RblxOsIsDebuggerPresent() rblxui_test_engine/rblxui_te_utils.cpp for a Unix compatible version.
    bool        ConfigDebugIsDebuggerPresent;   // = false          // Enable various tools calling RBLXUI_DEBUG_BREAK().

    // Tools to detect code submitting items with conflicting/duplicate IDs
    // - Code should use PushID()/PopID() in loops, or append "##xx" to same-label identifiers.
    // - Empty label e.g. Button("") == same ID as parent widget/node. Use Button("##xx") instead!
    // - See FAQ https://github.com/ocornut/rblxui/blob/master/docs/FAQ.md#q-about-the-id-stack-system
    bool        ConfigDebugHighlightIdConflicts;// = true           // Highlight and show an error message popup when multiple items have conflicting identifiers.
    bool        ConfigDebugHighlightIdConflictsShowItemPicker;//=true // Show "Item Picker" button in aforementioned popup.

    // Tools to test correct Begin/End and BeginChild/EndChild behaviors.
    // - Presently Begin()/End() and BeginChild()/EndChild() needs to ALWAYS be called in tandem, regardless of return value of BeginXXX()
    // - This is inconsistent with other BeginXXX functions and create confusion for many users.
    // - We expect to update the API eventually. In the meanwhile we provide tools to facilitate checking user-code behavior.
    bool        ConfigDebugBeginReturnValueOnce;// = false          // First-time calls to Begin()/BeginChild() will return false. NEEDS TO BE SET AT APPLICATION BOOT TIME if you don't want to miss windows.
    bool        ConfigDebugBeginReturnValueLoop;// = false          // Some calls to Begin()/BeginChild() will return false. Will cycle through window depths then repeat. Suggested use: add "io.ConfigDebugBeginReturnValue = io.KeyShift" in your main loop then occasionally press SHIFT. Windows should be flickering while running.

    // Option to deactivate io.AddFocusEvent(false) handling.
    // - May facilitate interactions with a debugger when focus loss leads to clearing inputs data.
    // - Backends may have other side-effects on focus loss, so this will reduce side-effects but not necessary remove all of them.
    bool        ConfigDebugIgnoreFocusLoss;     // = false          // Ignore io.AddFocusEvent(false), consequently not calling io.ClearInputKeys()/io.ClearInputMouse() in input processing.

    // Option to audit .ini data
    bool        ConfigDebugIniSettings;         // = false          // Save .ini data with extra comments (particularly helpful for Docking, but makes saving slower)

    //------------------------------------------------------------------
    // Platform Identifiers
    // (the rblxui_impl_xxxx backend files are setting those up for you)
    //------------------------------------------------------------------

    // Nowadays those would be stored in RblxUIPlatformIO but we are leaving them here for legacy reasons.
    // Optional: Platform/Renderer backend name (informational only! will be displayed in About Window) + User data for backend/wrappers to store their own stuff.
    const char* BackendPlatformName;            // = NULL
    const char* BackendRendererName;            // = NULL
    void*       BackendPlatformUserData;        // = NULL           // User data for platform backend
    void*       BackendRendererUserData;        // = NULL           // User data for renderer backend
    void*       BackendLanguageUserData;        // = NULL           // User data for non C++ programming language backend

    //------------------------------------------------------------------
    // Input - Call before calling NewFrame()
    //------------------------------------------------------------------

    // Input Functions
    RBLXUI_API void  AddKeyEvent(RblxUIKey key, bool down);                   // Queue a new key down/up event. Key should be "translated" (as in, generally RblxUIKey_A matches the key end-user would use to emit an 'A' character)
    RBLXUI_API void  AddKeyAnalogEvent(RblxUIKey key, bool down, float v);    // Queue a new key down/up event for analog values (e.g. RblxUIKey_Gamepad_ values). Dead-zones should be handled by the backend.
    RBLXUI_API void  AddMousePosEvent(float x, float y);                     // Queue a mouse position update. Use -FLT_MAX,-FLT_MAX to signify no mouse (e.g. app not focused and not hovered)
    RBLXUI_API void  AddMouseButtonEvent(int button, bool down);             // Queue a mouse button change
    RBLXUI_API void  AddMouseWheelEvent(float wheel_x, float wheel_y);       // Queue a mouse wheel update. wheel_y<0: scroll down, wheel_y>0: scroll up, wheel_x<0: scroll right, wheel_x>0: scroll left.
    RBLXUI_API void  AddMouseSourceEvent(RblxUIMouseSource source);           // Queue a mouse source change (Mouse/TouchScreen/Pen)
    RBLXUI_API void  AddMouseViewportEvent(RblxUIID id);                      // Queue a mouse hovered viewport. Requires backend to set RblxUIBackendFlags_HasMouseHoveredViewport to call this (for multi-viewport support).
    RBLXUI_API void  AddFocusEvent(bool focused);                            // Queue a gain/loss of focus for the application (generally based on OS/platform focus of your window)
    RBLXUI_API void  AddInputCharacter(unsigned int c);                      // Queue a new character input
    RBLXUI_API void  AddInputCharacterUTF16(RblxWchar16 c);                    // Queue a new character input from a UTF-16 character, it can be a surrogate
    RBLXUI_API void  AddInputCharactersUTF8(const char* str);                // Queue a new characters input from a UTF-8 string

    RBLXUI_API void  SetKeyEventNativeData(RblxUIKey key, int native_keycode, int native_scancode, int native_legacy_index = -1); // [Optional] Specify index for legacy <1.87 IsKeyXXX() functions with native indices + specify native keycode, scancode.
    RBLXUI_API void  SetAppAcceptingEvents(bool accepting_events);           // Set master flag for accepting key/mouse/text events (default to true). Useful if you have native dialog boxes that are interrupting your application loop/refresh, and you want to disable events being queued while your app is frozen.
    RBLXUI_API void  ClearEventsQueue();                                     // Clear all incoming events.
    RBLXUI_API void  ClearInputKeys();                                       // Clear current keyboard/gamepad state + current frame text input buffer. Equivalent to releasing all keys/buttons.
    RBLXUI_API void  ClearInputMouse();                                      // Clear current mouse state.

    //------------------------------------------------------------------
    // Output - Updated by NewFrame() or EndFrame()/Render()
    // (when reading from the io.WantCaptureMouse, io.WantCaptureKeyboard flags to dispatch your inputs, it is
    //  generally easier and more correct to use their state BEFORE calling NewFrame(). See FAQ for details!)
    //------------------------------------------------------------------

    bool        WantCaptureMouse;                   // Set when Dear RblxUI will use mouse inputs, in this case do not dispatch them to your main game/application (either way, always pass on mouse inputs to rblxui). (e.g. unclicked mouse is hovering over an rblxui window, widget is active, mouse was clicked over an rblxui window, etc.).
    bool        WantCaptureKeyboard;                // Set when Dear RblxUI will use keyboard inputs, in this case do not dispatch them to your main game/application (either way, always pass keyboard inputs to rblxui). (e.g. InputText active, or an rblxui window is focused and navigation is enabled, etc.).
    bool        WantTextInput;                      // Mobile/console: when set, you may display an on-screen keyboard. This is set by Dear RblxUI when it wants textual keyboard input to happen (e.g. when a InputText widget is active).
    bool        WantSetMousePos;                    // MousePos has been altered, backend should reposition mouse on next frame. Rarely used! Set only when io.ConfigNavMoveSetMousePos is enabled.
    bool        WantSaveIniSettings;                // When manual .ini load/save is active (io.IniFilename == NULL), this will be set to notify your application that you can call SaveIniSettingsToMemory() and save yourself. Important: clear io.WantSaveIniSettings yourself after saving!
    bool        NavActive;                          // Keyboard/Gamepad navigation is currently allowed (will handle RblxUIKey_NavXXX events) = a window is focused and it doesn't use the RblxUIWindowFlags_NoNavInputs flag.
    bool        NavVisible;                         // Keyboard/Gamepad navigation highlight is visible and allowed (will handle RblxUIKey_NavXXX events).
    float       Framerate;                          // Estimate of application framerate (rolling average over 60 frames, based on io.DeltaTime), in frame per second. Solely for convenience. Slow applications may not want to use a moving average or may want to reset underlying buffers occasionally.
    int         MetricsRenderVertices;              // Vertices output during last call to Render()
    int         MetricsRenderIndices;               // Indices output during last call to Render() = number of triangles * 3
    int         MetricsRenderWindows;               // Number of visible windows
    int         MetricsActiveWindows;               // Number of active windows
    RblxVec2      MouseDelta;                         // Mouse delta. Note that this is zero if either current or previous position are invalid (-FLT_MAX,-FLT_MAX), so a disappearing/reappearing mouse won't have a huge delta.

    //------------------------------------------------------------------
    // [Internal] Dear RblxUI will maintain those fields. Forward compatibility not guaranteed!
    //------------------------------------------------------------------

    RblxUIContext* Ctx;                              // Parent UI context (needs to be set explicitly by parent).

    // Main Input State
    // (this block used to be written by backend, since 1.87 it is best to NOT write to those directly, call the AddXXX functions above instead)
    // (reading from those variables is fair game, as they are extremely unlikely to be moving anywhere)
    RblxVec2      MousePos;                           // Mouse position, in pixels. Set to RblxVec2(-FLT_MAX, -FLT_MAX) if mouse is unavailable (on another screen, etc.)
    bool        MouseDown[5];                       // Mouse buttons: 0=left, 1=right, 2=middle + extras (RblxUIMouseButton_COUNT == 5). Dear RblxUI mostly uses left and right buttons. Other buttons allow us to track if the mouse is being used by your application + available to user as a convenience via IsMouse** API.
    float       MouseWheel;                         // Mouse wheel Vertical: 1 unit scrolls about 5 lines text. >0 scrolls Up, <0 scrolls Down. Hold Shift to turn vertical scroll into horizontal scroll.
    float       MouseWheelH;                        // Mouse wheel Horizontal. >0 scrolls Left, <0 scrolls Right. Most users don't have a mouse with a horizontal wheel, may not be filled by all backends.
    RblxUIMouseSource MouseSource;                   // Mouse actual input peripheral (Mouse/TouchScreen/Pen).
    RblxUIID     MouseHoveredViewport;               // (Optional) Modify using io.AddMouseViewportEvent(). With multi-viewports: viewport the OS mouse is hovering. If possible _IGNORING_ viewports with the RblxUIViewportFlags_NoInputs flag is much better (few backends can handle that). Set io.BackendFlags |= RblxUIBackendFlags_HasMouseHoveredViewport if you can provide this info. If you don't rblxui will infer the value using the rectangles and last focused time of the viewports it knows about (ignoring other OS windows).
    bool        KeyCtrl;                            // Keyboard modifier down: Ctrl (non-macOS), Cmd (macOS)
    bool        KeyShift;                           // Keyboard modifier down: Shift
    bool        KeyAlt;                             // Keyboard modifier down: Alt
    bool        KeySuper;                           // Keyboard modifier down: Windows/Super (non-macOS), Ctrl (macOS)

    // Other state maintained from data above + IO function calls
    RblxUIKeyChord KeyMods;                          // Key mods flags (any of RblxUIMod_Ctrl/RblxUIMod_Shift/RblxUIMod_Alt/RblxUIMod_Super flags, same as io.KeyCtrl/KeyShift/KeyAlt/KeySuper but merged into flags). Read-only, updated by NewFrame()
    RblxUIKeyData  KeysData[RblxUIKey_NamedKey_COUNT];// Key state for all known keys. MUST use 'key - RblxUIKey_NamedKey_BEGIN' as index. Use IsKeyXXX() functions to access this.
    bool        WantCaptureMouseUnlessPopupClose;   // Alternative to WantCaptureMouse: (WantCaptureMouse == true && WantCaptureMouseUnlessPopupClose == false) when a click over void is expected to close a popup.
    RblxVec2      MousePosPrev;                       // Previous mouse position (note that MouseDelta is not necessary == MousePos-MousePosPrev, in case either position is invalid)
    RblxVec2      MouseClickedPos[5];                 // Position at time of clicking
    double      MouseClickedTime[5];                // Time of last click (used to figure out double-click)
    bool        MouseClicked[5];                    // Mouse button went from !Down to Down (same as MouseClickedCount[x] != 0)
    bool        MouseDoubleClicked[5];              // Has mouse button been double-clicked? (same as MouseClickedCount[x] == 2)
    RblxU16       MouseClickedCount[5];               // == 0 (not clicked), == 1 (same as MouseClicked[]), == 2 (double-clicked), == 3 (triple-clicked) etc. when going from !Down to Down
    RblxU16       MouseClickedLastCount[5];           // Count successive number of clicks. Stays valid after mouse release. Reset after another click is done.
    bool        MouseReleased[5];                   // Mouse button went from Down to !Down
    double      MouseReleasedTime[5];               // Time of last released (rarely used! but useful to handle delayed single-click when trying to disambiguate them from double-click).
    bool        MouseDownOwned[5];                  // Track if button was clicked inside a dear rblxui window or over void blocked by a popup. We don't request mouse capture from the application if click started outside RblxUI bounds.
    bool        MouseDownOwnedUnlessPopupClose[5];  // Track if button was clicked inside a dear rblxui window.
    bool        MouseWheelRequestAxisSwap;          // On a non-Mac system, holding Shift requests WheelY to perform the equivalent of a WheelX event. On a Mac system this is already enforced by the system.
    bool        MouseCtrlLeftAsRightClick;          // (OSX) Set to true when the current click was a Ctrl+Click that spawned a simulated right click
    float       MouseDownDuration[5];               // Duration the mouse button has been down (0.0f == just clicked)
    float       MouseDownDurationPrev[5];           // Previous time the mouse button has been down
    RblxVec2      MouseDragMaxDistanceAbs[5];         // Maximum distance, absolute, on each axis, of how much mouse has traveled from the clicking point
    float       MouseDragMaxDistanceSqr[5];         // Squared maximum distance of how much mouse has traveled from the clicking point (used for moving thresholds)
    float       PenPressure;                        // Touch/Pen pressure (0.0f to 1.0f, should be >0.0f only when MouseDown[0] == true). Helper storage currently unused by Dear RblxUI.
    bool        AppFocusLost;                       // Only modify via AddFocusEvent()
    bool        AppAcceptingEvents;                 // Only modify via SetAppAcceptingEvents()
    RblxWchar16   InputQueueSurrogate;                // For AddInputCharacterUTF16()
    RblxVector<RblxWchar> InputQueueCharacters;         // Queue of _characters_ input (obtained by platform backend). Fill using AddInputCharacter() helper.

    // Legacy: before 1.87, we required backend to fill io.KeyMap[] (rblxui->native map) during initialization and io.KeysDown[] (native indices) every frame.
    // This is still temporarily supported as a legacy feature. However the new preferred scheme is for backend to call io.AddKeyEvent().
    //   Old (<1.87):  RblxUI::IsKeyPressed(RblxUI::GetIO().KeyMap[RblxUIKey_Space]) --> New (1.87+) RblxUI::IsKeyPressed(RblxUIKey_Space)
    //   Old (<1.87):  RblxUI::IsKeyPressed(MYPLATFORM_KEY_SPACE)                  --> New (1.87+) RblxUI::IsKeyPressed(RblxUIKey_Space)
    // Read https://github.com/ocornut/rblxui/issues/4921 for details.
    //int       KeyMap[RblxUIKey_COUNT];             // [LEGACY] Input: map of indices into the KeysDown[512] entries array which represent your "native" keyboard state. The first 512 are now unused and should be kept zero. Legacy backend will write into KeyMap[] using RblxUIKey_ indices which are always >512.
    //bool      KeysDown[RblxUIKey_COUNT];           // [LEGACY] Input: Keyboard keys that are pressed (ideally left in the "native" order your engine has access to keyboard keys, so you can use your own defines/enums for keys). This used to be [512] sized. It is now RblxUIKey_COUNT to allow legacy io.KeysDown[GetKeyIndex(...)] to work without an overflow.
    //float     NavInputs[RblxUINavInput_COUNT];     // [LEGACY] Since 1.88, NavInputs[] was removed. Backends from 1.60 to 1.86 won't build. Feed gamepad inputs via io.AddKeyEvent() and RblxUIKey_GamepadXXX enums.
    //void*     ImeWindowHandle;                    // [Obsoleted in 1.87] Set RblxUIViewport::PlatformHandleRaw instead. Set this to your HWND to get automatic IME cursor positioning.

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    float       FontGlobalScale;                    // Moved io.FontGlobalScale to style.FontScaleMain in 1.92.0 (June 2025)

    // Legacy: before 1.91.1, clipboard functions were stored in RblxUIIO instead of RblxUIPlatformIO.
    // As this is will affect all users of custom engines/backends, we are providing proper legacy redirection (will obsolete).
    const char* (*GetClipboardTextFn)(void* user_data);
    void        (*SetClipboardTextFn)(void* user_data, const char* text);
    void*       ClipboardUserData;

    //void ClearInputCharacters() { InputQueueCharacters.resize(0); } // [Obsoleted in 1.89.8] Clear the current frame text input buffer. Now included within ClearInputKeys(). Removed this as it is ambiguous/misleading and generally incorrect to use with the existence of a higher-level input queue.
#endif

    RBLXUI_API   RblxUIIO();
};

//-----------------------------------------------------------------------------
// [SECTION] Misc data structures (RblxUIInputTextCallbackData, RblxUISizeCallbackData, RblxUIPayload)
//-----------------------------------------------------------------------------

// Shared state of InputText(), passed as an argument to your callback when a RblxUIInputTextFlags_Callback* flag is used.
// The callback function should return 0 by default.
// Callbacks (follow a flag name and see comments in RblxUIInputTextFlags_ declarations for more details)
// - RblxUIInputTextFlags_CallbackEdit:        Callback on buffer edit. Note that InputText() already returns true on edit + you can always use IsItemEdited(). The callback is useful to manipulate the underlying buffer while focus is active.
// - RblxUIInputTextFlags_CallbackAlways:      Callback on each iteration
// - RblxUIInputTextFlags_CallbackCompletion:  Callback on pressing TAB
// - RblxUIInputTextFlags_CallbackHistory:     Callback on pressing Up/Down arrows
// - RblxUIInputTextFlags_CallbackCharFilter:  Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
// - RblxUIInputTextFlags_CallbackResize:      Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow.
struct RblxUIInputTextCallbackData
{
    RblxUIContext*       Ctx;            // Parent UI context
    RblxUIInputTextFlags EventFlag;      // One RblxUIInputTextFlags_Callback*    // Read-only
    RblxUIInputTextFlags Flags;          // What user passed to InputText()      // Read-only
    void*               UserData;       // What user passed to InputText()      // Read-only
    RblxUIID             ID;             // Widget ID                            // Read-only

    // Arguments for the different callback events
    // - During Resize callback, Buf will be same as your input buffer.
    // - However, during Completion/History/Always callback, Buf always points to our own internal data (it is not the same as your buffer)! Changes to it will be reflected into your own buffer shortly after the callback.
    // - To modify the text buffer in a callback, prefer using the InsertChars() / DeleteChars() function. InsertChars() will take care of calling the resize callback if necessary.
    // - If you know your edits are not going to resize the underlying buffer allocation, you may modify the contents of 'Buf[]' directly. You need to update 'BufTextLen' accordingly (0 <= BufTextLen < BufSize) and set 'BufDirty'' to true so InputText can update its internal state.
    RblxUIKey            EventKey;       // Key pressed (Up/Down/TAB)            // Read-only    // [Completion,History]
    RblxWchar             EventChar;      // Character input                      // Read-write   // [CharFilter] Replace character with another one, or set to zero to drop. return 1 is equivalent to setting EventChar=0;
    bool                EventActivated; // Input field just got activated       // Read-only    // [Always]
    bool                BufDirty;       // Set if you modify Buf/BufTextLen!    // Write        // [Completion,History,Always]
    char*               Buf;            // Text buffer                          // Read-write   // [Resize] Can replace pointer / [Completion,History,Always] Only write to pointed data, don't replace the actual pointer!
    int                 BufTextLen;     // Text length (in bytes)               // Read-write   // [Resize,Completion,History,Always] Exclude zero-terminator storage. In C land: == strlen(some_text), in C++ land: string.length()
    int                 BufSize;        // Buffer size (in bytes) = capacity+1  // Read-only    // [Resize,Completion,History,Always] Include zero-terminator storage. In C land: == ARRAYSIZE(my_char_array), in C++ land: string.capacity()+1
    int                 CursorPos;      //                                      // Read-write   // [Completion,History,Always,CharFilter]
    int                 SelectionStart; //                                      // Read-write   // [Completion,History,Always,CharFilter] == to SelectionEnd when no selection
    int                 SelectionEnd;   //                                      // Read-write   // [Completion,History,Always,CharFilter]

    // Helper functions for text manipulation.
    // Use those function to benefit from the CallbackResize behaviors. Calling those function reset the selection.
    RBLXUI_API RblxUIInputTextCallbackData();
    RBLXUI_API void      DeleteChars(int pos, int bytes_count);
    RBLXUI_API void      InsertChars(int pos, const char* text, const char* text_end = NULL);
    void                SelectAll()                 { SelectionStart = 0; CursorPos = SelectionEnd = BufTextLen; }
    void                SetSelection(int s, int e)  { RBLXUI_ASSERT(s >= 0 && s <= BufTextLen); RBLXUI_ASSERT(e >= 0 && e <= BufTextLen); SelectionStart = s; CursorPos = SelectionEnd = e; }
    void                ClearSelection()            { SelectionStart = SelectionEnd = BufTextLen; }
    bool                HasSelection() const        { return SelectionStart != SelectionEnd; }
};

// Resizing callback data to apply custom constraint. As enabled by SetNextWindowSizeConstraints(). Callback is called during the next Begin().
// NB: For basic min/max size constraint on each axis you don't need to use the callback! The SetNextWindowSizeConstraints() parameters are enough.
struct RblxUISizeCallbackData
{
    void*   UserData;       // Read-only.   What user passed to SetNextWindowSizeConstraints(). Generally store an integer or float in here (need reinterpret_cast<>).
    RblxVec2  Pos;            // Read-only.   Window position, for reference.
    RblxVec2  CurrentSize;    // Read-only.   Current window size.
    RblxVec2  DesiredSize;    // Read-write.  Desired size, based on user's mouse position. Write to this field to restrain resizing.
};

// [ALPHA] Rarely used / very advanced uses only. Use with SetNextWindowClass() and DockSpace() functions.
// Important: the content of this class is still highly WIP and likely to change and be refactored
// before we stabilize Docking features. Please be mindful if using this.
// Provide hints:
// - To the platform backend via altered viewport flags (enable/disable OS decoration, OS task bar icons, etc.)
// - To the platform backend for OS level parent/child relationships of viewport (otherwise: default is configured via io.ConfigViewportsNoDefaultParent)
// - To the docking system for various options and filtering.
struct RblxUIWindowClass
{
    RblxUIID             ClassId;                    // User data. 0 = Default class (unclassed). Windows of different classes cannot be docked with each others.
    RblxUIID             ParentViewportId;           // Hint for the platform backend. -1: use default. 0: request platform backend to not parent the platform. != 0: request platform backend to create a parent<>child relationship between the platform windows. Not conforming backends are free to e.g. parent every viewport to the main viewport or not.
    RblxUIID             FocusRouteParentWindowId;   // ID of parent window for shortcut focus route evaluation, e.g. Shortcut() call from Parent Window will succeed when this window is focused.
    RblxUIViewportFlags  ViewportFlagsOverrideSet;   // Viewport flags to set when a window of this class owns a viewport. This allows you to enforce OS decoration or task bar icon, override the defaults on a per-window basis.
    RblxUIViewportFlags  ViewportFlagsOverrideClear; // Viewport flags to clear when a window of this class owns a viewport. This allows you to enforce OS decoration or task bar icon, override the defaults on a per-window basis.
    RblxUITabItemFlags   TabItemFlagsOverrideSet;    // [EXPERIMENTAL] TabItem flags to set when a window of this class gets submitted into a dock node tab bar. May use with RblxUITabItemFlags_Leading or RblxUITabItemFlags_Trailing.
    RblxUIDockNodeFlags  DockNodeFlagsOverrideSet;   // [EXPERIMENTAL] Dock node flags to set when a window of this class is hosted by a dock node (it doesn't have to be selected!)
    bool                DockingAlwaysTabBar;        // Set to true to enforce single floating windows of this class always having their own docking node (equivalent of setting the global io.ConfigDockingAlwaysTabBar)
    bool                DockingAllowUnclassed;      // Set to true to allow windows of this class to be docked/merged with an unclassed window. // FIXME-DOCK: Move to DockNodeFlags override?
    void*               PlatformIconData;           // [EXPERIMENTAL] Pass opaque data for Platform backend to handle.

    RblxUIWindowClass() { memset((void*)this, 0, sizeof(*this)); ParentViewportId = (RblxUIID)-1; DockingAllowUnclassed = true; }
};

// Data payload for Drag and Drop operations: AcceptDragDropPayload(), GetDragDropPayload()
struct RblxUIPayload
{
    // Members
    void*           Data;               // Data (copied and owned by dear rblxui)
    int             DataSize;           // Data size

    // [Internal]
    RblxUIID         SourceId;           // Source item id
    RblxUIID         SourceParentId;     // Source parent id (if available)
    int             DataFrameCount;     // Data timestamp
    char            DataType[32 + 1];   // Data type tag (short user-supplied string, 32 characters max)
    bool            Preview;            // Set when AcceptDragDropPayload() was called and mouse has been hovering the target item (nb: handle overlapping drag targets)
    bool            Delivery;           // Set when AcceptDragDropPayload() was called and mouse button is released over the target item.

    RblxUIPayload()  { Clear(); }
    void Clear()    { SourceId = SourceParentId = 0; Data = NULL; DataSize = 0; memset(DataType, 0, sizeof(DataType)); DataFrameCount = -1; Preview = Delivery = false; }
    bool IsDataType(const char* type) const { return DataFrameCount != -1 && strcmp(type, DataType) == 0; }
    bool IsPreview() const                  { return Preview; }
    bool IsDelivery() const                 { return Delivery; }
};

//-----------------------------------------------------------------------------
// [SECTION] Helpers (RblxUIOnceUponAFrame, RblxUITextFilter, RblxUITextBuffer, RblxUIStorage, RblxUIListClipper, Math Operators, RblxColor)
//-----------------------------------------------------------------------------

// Helper: Unicode defines
#define RBLXUI_UNICODE_CODEPOINT_INVALID 0xFFFD     // Invalid Unicode code point (standard value).
#ifdef RBLXUI_USE_WCHAR32
#define RBLXUI_UNICODE_CODEPOINT_MAX     0x10FFFF   // Maximum Unicode code point supported by this build.
#else
#define RBLXUI_UNICODE_CODEPOINT_MAX     0xFFFF     // Maximum Unicode code point supported by this build.
#endif

// Helper: Execute a block of code at maximum once a frame. Convenient if you want to quickly create a UI within deep-nested code that runs multiple times every frame.
// Usage: static RblxUIOnceUponAFrame oaf; if (oaf) RblxUI::Text("This will be called only once per frame");
struct RblxUIOnceUponAFrame
{
    RblxUIOnceUponAFrame() { RefFrame = -1; }
    mutable int RefFrame;
    operator bool() const { int current_frame = RblxUI::GetFrameCount(); if (RefFrame == current_frame) return false; RefFrame = current_frame; return true; }
};

// Helper: Parse and apply text filters. In format "aaaaa[,bbbb][,ccccc]"
struct RblxUITextFilter
{
    RBLXUI_API           RblxUITextFilter(const char* default_filter = "");
    RBLXUI_API bool      Draw(const char* label = "Filter (inc,-exc)", float width = 0.0f);  // Helper calling InputText+Build
    RBLXUI_API bool      PassFilter(const char* text, const char* text_end = NULL) const;
    RBLXUI_API void      Build();
    void                Clear()          { InputBuf[0] = 0; Build(); }
    bool                IsActive() const { return !Filters.empty(); }

    // [Internal]
    struct RblxUITextRange
    {
        const char*     b;
        const char*     e;

        RblxUITextRange()                                { b = e = NULL; }
        RblxUITextRange(const char* _b, const char* _e)  { b = _b; e = _e; }
        bool            empty() const                   { return b == e; }
        RBLXUI_API void  split(char separator, RblxVector<RblxUITextRange>* out) const;
    };
    char                    InputBuf[256];
    RblxVector<RblxUITextRange>Filters;
    int                     CountGrep;
};

// Helper: Growable text buffer for logging/accumulating text
// (this could be called 'RblxUITextBuilder' / 'RblxUIStringBuilder')
struct RblxUITextBuffer
{
    RblxVector<char>      Buf;
    RBLXUI_API static char EmptyString[1];

    RblxUITextBuffer()   { }
    inline char         operator[](int i) const { RBLXUI_ASSERT(Buf.Data != NULL); return Buf.Data[i]; }
    const char*         begin() const           { return Buf.Data ? &Buf.front() : EmptyString; }
    const char*         end() const             { return Buf.Data ? &Buf.back() : EmptyString; } // Buf is zero-terminated, so end() will point on the zero-terminator
    int                 size() const            { return Buf.Size ? Buf.Size - 1 : 0; }
    bool                empty() const           { return Buf.Size <= 1; }
    void                clear()                 { Buf.clear(); }
    void                resize(int size)        { if (Buf.Size > size) Buf.Data[size] = 0; Buf.resize(size ? size + 1 : 0, 0); } // Similar to resize(0) on RblxVector: empty string but don't free buffer.
    void                reserve(int capacity)   { Buf.reserve(capacity); }
    const char*         c_str() const           { return Buf.Data ? Buf.Data : EmptyString; }
    RBLXUI_API void      append(const char* str, const char* str_end = NULL);
    RBLXUI_API void      appendf(const char* fmt, ...) RBLXUI_FMTARGS(2);
    RBLXUI_API void      appendfv(const char* fmt, va_list args) RBLXUI_FMTLIST(2);
};

// [Internal] Key+Value for RblxUIStorage
struct RblxUIStoragePair
{
    RblxUIID     key;
    union       { int val_i; float val_f; void* val_p; };
    RblxUIStoragePair(RblxUIID _key, int _val)    { key = _key; val_i = _val; }
    RblxUIStoragePair(RblxUIID _key, float _val)  { key = _key; val_f = _val; }
    RblxUIStoragePair(RblxUIID _key, void* _val)  { key = _key; val_p = _val; }
};

// Helper: Key->Value storage
// Typically you don't have to worry about this since a storage is held within each Window.
// We use it to e.g. store collapse state for a tree (Int 0/1)
// This is optimized for efficient lookup (dichotomy into a contiguous buffer) and rare insertion (typically tied to user interactions aka max once a frame)
// You can use it as custom user storage for temporary values. Declare your own storage if, for example:
// - You want to manipulate the open/close state of a particular sub-tree in your interface (tree node uses Int 0/1 to store their state).
// - You want to store custom debug data easily without adding or editing structures in your code (probably not efficient, but convenient)
// Types are NOT stored, so it is up to you to make sure your Key don't collide with different types.
struct RblxUIStorage
{
    // [Internal]
    RblxVector<RblxUIStoragePair>      Data;

    // - Get***() functions find pair, never add/allocate. Pairs are sorted so a query is O(log N)
    // - Set***() functions find pair, insertion on demand if missing.
    // - Sorted insertion is costly, paid once. A typical frame shouldn't need to insert any new pair.
    void                Clear() { Data.clear(); }
    RBLXUI_API int       GetInt(RblxUIID key, int default_val = 0) const;
    RBLXUI_API void      SetInt(RblxUIID key, int val);
    RBLXUI_API bool      GetBool(RblxUIID key, bool default_val = false) const;
    RBLXUI_API void      SetBool(RblxUIID key, bool val);
    RBLXUI_API float     GetFloat(RblxUIID key, float default_val = 0.0f) const;
    RBLXUI_API void      SetFloat(RblxUIID key, float val);
    RBLXUI_API void*     GetVoidPtr(RblxUIID key) const; // default_val is NULL
    RBLXUI_API void      SetVoidPtr(RblxUIID key, void* val);

    // - Get***Ref() functions finds pair, insert on demand if missing, return pointer. Useful if you intend to do Get+Set.
    // - References are only valid until a new value is added to the storage. Calling a Set***() function or a Get***Ref() function invalidates the pointer.
    // - A typical use case where this is convenient for quick hacking (e.g. add storage during a live Edit&Continue session if you can't modify existing struct)
    //      float* pvar = RblxUI::GetFloatRef(key); RblxUI::SliderFloat("var", pvar, 0, 100.0f); some_var += *pvar;
    RBLXUI_API int*      GetIntRef(RblxUIID key, int default_val = 0);
    RBLXUI_API bool*     GetBoolRef(RblxUIID key, bool default_val = false);
    RBLXUI_API float*    GetFloatRef(RblxUIID key, float default_val = 0.0f);
    RBLXUI_API void**    GetVoidPtrRef(RblxUIID key, void* default_val = NULL);

    // Advanced: for quicker full rebuild of a storage (instead of an incremental one), you may add all your contents and then sort once.
    RBLXUI_API void      BuildSortByKey();
    // Obsolete: use on your own storage if you know only integer are being stored (open/close all tree nodes)
    RBLXUI_API void      SetAllInt(int val);

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    //typedef ::RblxUIStoragePair RblxUIStoragePair;  // 1.90.8: moved type outside struct
#endif
};

// Flags for RblxUIListClipper (currently not fully exposed in function calls: a future refactor will likely add this to RblxUIListClipper::Begin function equivalent)
enum RblxUIListClipperFlags_
{
    RblxUIListClipperFlags_None                  = 0,
    RblxUIListClipperFlags_NoSetTableRowCounters = 1 << 0,   // [Internal] Disabled modifying table row counters. Avoid assumption that 1 clipper item == 1 table row.
};

// Helper: Manually clip large list of items.
// If you have lots evenly spaced items and you have random access to the list, you can perform coarse
// clipping based on visibility to only submit items that are in view.
// The clipper calculates the range of visible items and advance the cursor to compensate for the non-visible items we have skipped.
// (Dear RblxUI already clip items based on their bounds but: it needs to first layout the item to do so, and generally
//  fetching/submitting your own data incurs additional cost. Coarse clipping using RblxUIListClipper allows you to easily
//  scale using lists with tens of thousands of items without a problem)
// Usage:
//   RblxUIListClipper clipper;
//   clipper.Begin(1000);         // We have 1000 elements, evenly spaced.
//   while (clipper.Step())
//       for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
//           RblxUI::Text("line number %d", i);
// Generally what happens is:
// - Clipper lets you process the first element (DisplayStart = 0, DisplayEnd = 1) regardless of it being visible or not.
// - User code submit that one element.
// - Clipper can measure the height of the first element
// - Clipper calculate the actual range of elements to display based on the current clipping rectangle, position the cursor before the first visible element.
// - User code submit visible elements.
// - The clipper also handles various subtleties related to keyboard/gamepad navigation, wrapping etc.
struct RblxUIListClipper
{
    int             DisplayStart;       // First item to display, updated by each call to Step()
    int             DisplayEnd;         // End of items to display (exclusive)
    int             UserIndex;          // Helper storage for user convenience/code. Optional, and otherwise unused if you don't use it.
    int             ItemsCount;         // [Internal] Number of items
    float           ItemsHeight;        // [Internal] Height of item after a first step and item submission can calculate it
    RblxUIListClipperFlags Flags;        // [Internal] Flags, currently not yet well exposed.
    double          StartPosY;          // [Internal] Cursor position at the time of Begin() or after table frozen rows are all processed
    double          StartSeekOffsetY;   // [Internal] Account for frozen rows in a table and initial loss of precision in very large windows.
    RblxUIContext*   Ctx;                // [Internal] Parent UI context
    void*           TempData;           // [Internal] Internal data

    // items_count: Use INT_MAX if you don't know how many items you have (in which case the cursor won't be advanced in the final step, and you can call SeekCursorForItem() manually if you need)
    // items_height: Use -1.0f to be calculated automatically on first step. Otherwise pass in the distance between your items, typically GetTextLineHeightWithSpacing() or GetFrameHeightWithSpacing().
    RBLXUI_API RblxUIListClipper();
    RBLXUI_API ~RblxUIListClipper();
    RBLXUI_API void  Begin(int items_count, float items_height = -1.0f);
    RBLXUI_API void  End();             // Automatically called on the last call of Step() that returns false.
    RBLXUI_API bool  Step();            // Call until it returns false. The DisplayStart/DisplayEnd fields will be set and you can process/draw those items.

    // Call IncludeItemByIndex() or IncludeItemsByIndex() *BEFORE* first call to Step() if you need a range of items to not be clipped, regardless of their visibility.
    // (Due to alignment / padding of certain items it is possible that an extra item may be included on either end of the display range).
    inline void     IncludeItemByIndex(int item_index)                  { IncludeItemsByIndex(item_index, item_index + 1); }
    RBLXUI_API void  IncludeItemsByIndex(int item_begin, int item_end);  // item_end is exclusive e.g. use (42, 42+1) to make item 42 never clipped.

    // Seek cursor toward given item. This is automatically called while stepping.
    // - The only reason to call this is: you can use RblxUIListClipper::Begin(INT_MAX) if you don't know item count ahead of time.
    // - In this case, after all steps are done, you'll want to call SeekCursorForItem(item_count).
    RBLXUI_API void  SeekCursorForItem(int item_index);

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    //inline void IncludeRangeByIndices(int item_begin, int item_end)      { IncludeItemsByIndex(item_begin, item_end); } // [renamed in 1.89.9]
    //inline void ForceDisplayRangeByIndices(int item_begin, int item_end) { IncludeItemsByIndex(item_begin, item_end); } // [renamed in 1.89.6]
    //inline RblxUIListClipper(int items_count, float items_height = -1.0f) { memset((void*)this, 0, sizeof(*this)); ItemsCount = -1; Begin(items_count, items_height); } // [removed in 1.79]
#endif
};

// Helpers: RblxVec2/RblxVec4 operators
// - It is important that we are keeping those disabled by default so they don't leak in user space.
// - This is in order to allow user enabling implicit cast operators between RblxVec2/RblxVec4 and their own types (using RBLXUI_VEC2_CLASS_EXTRA in rblxuiconfig.h)
// - Add '#define RBLXUI_DEFINE_MATH_OPERATORS' before including this file (or in rblxuiconfig.h) to access courtesy maths operators for RblxVec2 and RblxVec4.
#ifdef RBLXUI_DEFINE_MATH_OPERATORS
#define RBLXUI_DEFINE_MATH_OPERATORS_IMPLEMENTED
RBLXUI_MSVC_RUNTIME_CHECKS_OFF
// RblxVec2 operators
inline RblxVec2  operator*(const RblxVec2& lhs, const float rhs)    { return RblxVec2(lhs.x * rhs, lhs.y * rhs); }
inline RblxVec2  operator*(const float lhs, const RblxVec2& rhs)    { return RblxVec2(lhs * rhs.x, lhs * rhs.y); }
inline RblxVec2  operator/(const RblxVec2& lhs, const float rhs)    { return RblxVec2(lhs.x / rhs, lhs.y / rhs); }
inline RblxVec2  operator+(const RblxVec2& lhs, const RblxVec2& rhs)  { return RblxVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline RblxVec2  operator-(const RblxVec2& lhs, const RblxVec2& rhs)  { return RblxVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
inline RblxVec2  operator*(const RblxVec2& lhs, const RblxVec2& rhs)  { return RblxVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
inline RblxVec2  operator/(const RblxVec2& lhs, const RblxVec2& rhs)  { return RblxVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
inline RblxVec2  operator+(const RblxVec2& lhs)                     { return lhs; }
inline RblxVec2  operator-(const RblxVec2& lhs)                     { return RblxVec2(-lhs.x, -lhs.y); }
inline RblxVec2& operator*=(RblxVec2& lhs, const float rhs)         { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
inline RblxVec2& operator/=(RblxVec2& lhs, const float rhs)         { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
inline RblxVec2& operator+=(RblxVec2& lhs, const RblxVec2& rhs)       { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
inline RblxVec2& operator-=(RblxVec2& lhs, const RblxVec2& rhs)       { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
inline RblxVec2& operator*=(RblxVec2& lhs, const RblxVec2& rhs)       { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
inline RblxVec2& operator/=(RblxVec2& lhs, const RblxVec2& rhs)       { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }
inline bool    operator==(const RblxVec2& lhs, const RblxVec2& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline bool    operator!=(const RblxVec2& lhs, const RblxVec2& rhs) { return lhs.x != rhs.x || lhs.y != rhs.y; }
// RblxVec4 operators
inline RblxVec4  operator*(const RblxVec4& lhs, const float rhs)    { return RblxVec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); }
inline RblxVec4  operator*(const float lhs, const RblxVec4& rhs)    { return RblxVec4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w); }
inline RblxVec4  operator/(const RblxVec4& lhs, const float rhs)    { return RblxVec4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs); }
inline RblxVec4  operator+(const RblxVec4& lhs, const RblxVec4& rhs)  { return RblxVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
inline RblxVec4  operator-(const RblxVec4& lhs, const RblxVec4& rhs)  { return RblxVec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
inline RblxVec4  operator*(const RblxVec4& lhs, const RblxVec4& rhs)  { return RblxVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }
inline RblxVec4  operator/(const RblxVec4& lhs, const RblxVec4& rhs)  { return RblxVec4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w); }
inline RblxVec4  operator+(const RblxVec4& lhs)                     { return lhs; }
inline RblxVec4  operator-(const RblxVec4& lhs)                     { return RblxVec4(-lhs.x, -lhs.y, -lhs.z, -lhs.w); }
inline RblxVec4& operator*=(RblxVec4& lhs, const float rhs)         { lhs.x *= rhs; lhs.y *= rhs; lhs.z *= rhs; lhs.w *= rhs; return lhs; }
inline RblxVec4& operator/=(RblxVec4& lhs, const float rhs)         { lhs.x /= rhs; lhs.y /= rhs; lhs.z /= rhs; lhs.w /= rhs; return lhs; }
inline RblxVec4& operator+=(RblxVec4& lhs, const RblxVec4& rhs)       { lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z; lhs.w += rhs.w; return lhs; }
inline RblxVec4& operator-=(RblxVec4& lhs, const RblxVec4& rhs)       { lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z; lhs.w -= rhs.w; return lhs; }
inline RblxVec4& operator*=(RblxVec4& lhs, const RblxVec4& rhs)       { lhs.x *= rhs.x; lhs.y *= rhs.y; lhs.z *= rhs.z; lhs.w *= rhs.w; return lhs; }
inline RblxVec4& operator/=(RblxVec4& lhs, const RblxVec4& rhs)       { lhs.x /= rhs.x; lhs.y /= rhs.y; lhs.z /= rhs.z; lhs.w /= rhs.w; return lhs; }
inline bool    operator==(const RblxVec4& lhs, const RblxVec4& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w; }
inline bool    operator!=(const RblxVec4& lhs, const RblxVec4& rhs) { return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z || lhs.w != rhs.w; }
RBLXUI_MSVC_RUNTIME_CHECKS_RESTORE
#endif

// Helpers macros to generate 32-bit encoded colors
// - User can declare their own format by #defining the 5 _SHIFT/_MASK macros in their rblxuiconfig file.
// - Any setting other than the default will need custom backend support. The only standard backend that supports anything else than the default is DirectX9.
#ifndef RBLXUI_COL32_R_SHIFT
#ifdef RBLXUI_USE_BGRA_PACKED_COLOR
#define RBLXUI_COL32_R_SHIFT    16
#define RBLXUI_COL32_G_SHIFT    8
#define RBLXUI_COL32_B_SHIFT    0
#define RBLXUI_COL32_A_SHIFT    24
#define RBLXUI_COL32_A_MASK     0xFF000000
#else
#define RBLXUI_COL32_R_SHIFT    0
#define RBLXUI_COL32_G_SHIFT    8
#define RBLXUI_COL32_B_SHIFT    16
#define RBLXUI_COL32_A_SHIFT    24
#define RBLXUI_COL32_A_MASK     0xFF000000
#endif
#endif
#define RBLXUI_COL32(R,G,B,A)    (((RblxU32)(A)<<RBLXUI_COL32_A_SHIFT) | ((RblxU32)(B)<<RBLXUI_COL32_B_SHIFT) | ((RblxU32)(G)<<RBLXUI_COL32_G_SHIFT) | ((RblxU32)(R)<<RBLXUI_COL32_R_SHIFT))
#define RBLXUI_COL32_WHITE       RBLXUI_COL32(255,255,255,255)  // Opaque white = 0xFFFFFFFF
#define RBLXUI_COL32_BLACK       RBLXUI_COL32(0,0,0,255)        // Opaque black
#define RBLXUI_COL32_BLACK_TRANS RBLXUI_COL32(0,0,0,0)          // Transparent black = 0x00000000

// Helper: RblxColor() implicitly converts colors to either RblxU32 (packed 4x1 byte) or RblxVec4 (4x1 float)
// Prefer using RBLXUI_COL32() macros if you want a guaranteed compile-time RblxU32 for usage with RblxDrawList API.
// **Avoid storing RblxColor! Store either u32 of RblxVec4. This is not a full-featured color class. MAY OBSOLETE.
// **None of the RblxUI API are using RblxColor directly but you can use it as a convenience to pass colors in either RblxU32 or RblxVec4 formats. Explicitly cast to RblxU32 or RblxVec4 if needed.
struct RblxColor
{
    RblxVec4          Value;

    constexpr RblxColor()                                             { }
    constexpr RblxColor(float r, float g, float b, float a = 1.0f)    : Value(r, g, b, a) { }
    constexpr RblxColor(const RblxVec4& col)                            : Value(col) {}
    constexpr RblxColor(int r, int g, int b, int a = 255)             : Value((float)r * (1.0f / 255.0f), (float)g * (1.0f / 255.0f), (float)b * (1.0f / 255.0f), (float)a* (1.0f / 255.0f)) {}
    constexpr RblxColor(RblxU32 rgba)                                   : Value((float)((rgba >> RBLXUI_COL32_R_SHIFT) & 0xFF) * (1.0f / 255.0f), (float)((rgba >> RBLXUI_COL32_G_SHIFT) & 0xFF) * (1.0f / 255.0f), (float)((rgba >> RBLXUI_COL32_B_SHIFT) & 0xFF) * (1.0f / 255.0f), (float)((rgba >> RBLXUI_COL32_A_SHIFT) & 0xFF) * (1.0f / 255.0f)) {}
    inline operator RblxU32() const                                   { return RblxUI::ColorConvertFloat4ToU32(Value); }
    inline operator RblxVec4() const                                  { return Value; }

    // FIXME-OBSOLETE: May need to obsolete/cleanup those helpers.
    inline void    SetHSV(float h, float s, float v, float a = 1.0f){ RblxUI::ColorConvertHSVtoRGB(h, s, v, Value.x, Value.y, Value.z); Value.w = a; }
    static RblxColor HSV(float h, float s, float v, float a = 1.0f)   { float r, g, b; RblxUI::ColorConvertHSVtoRGB(h, s, v, r, g, b); return RblxColor(r, g, b, a); }
};

//-----------------------------------------------------------------------------
// [SECTION] Multi-Select API flags and structures (RblxUIMultiSelectFlags, RblxUISelectionRequestType, RblxUISelectionRequest, RblxUIMultiSelectIO, RblxUISelectionBasicStorage)
//-----------------------------------------------------------------------------

// Multi-selection system
// Documentation at: https://github.com/ocornut/rblxui/wiki/Multi-Select
// - Refer to 'Demo->Widgets->Selection State & Multi-Select' for demos using this.
// - This system implements standard multi-selection idioms (Ctrl+Mouse/Keyboard, Shift+Mouse/Keyboard, etc)
//   with support for clipper (skipping non-visible items), box-select and many other details.
// - Selectable(), Checkbox() are supported but custom widgets may use it as well.
// - TreeNode() is technically supported but... using this correctly is more complicated: you need some sort of linear/random access to your tree,
//   which is suited to advanced trees setups also implementing filters and clipper. We will work toward simplifying and demoing it.
// - In the spirit of Dear RblxUI design, your code owns actual selection data.
//   This is designed to allow all kinds of selection storage you may use in your application e.g. set/map/hash.
// About RblxUISelectionBasicStorage:
// - This is an optional helper to store a selection state and apply selection requests.
// - It is used by our demos and provided as a convenience to quickly implement multi-selection.
// Usage:
// - Identify submitted items with SetNextItemSelectionUserData(), most likely using an index into your current data-set.
// - Store and maintain actual selection data using persistent object identifiers.
// - Usage flow:
//     BEGIN - (1) Call BeginMultiSelect() and retrieve the RblxUIMultiSelectIO* result.
//           - (2) Honor request list (SetAll/SetRange requests) by updating your selection data. Same code as Step 6.
//           - (3) [If using clipper] You need to make sure RangeSrcItem is always submitted. Calculate its index and pass to clipper.IncludeItemByIndex(). If storing indices in RblxUISelectionUserData, a simple clipper.IncludeItemByIndex(ms_io->RangeSrcItem) call will work.
//     LOOP  - (4) Submit your items with SetNextItemSelectionUserData() + Selectable()/TreeNode() calls.
//     END   - (5) Call EndMultiSelect() and retrieve the RblxUIMultiSelectIO* result.
//           - (6) Honor request list (SetAll/SetRange requests) by updating your selection data. Same code as Step 2.
//     If you submit all items (no clipper), Step 2 and 3 are optional and will be handled by each item themselves. It is fine to always honor those steps.
// About RblxUISelectionUserData:
// - This can store an application-defined identifier (e.g. index or pointer) submitted via SetNextItemSelectionUserData().
// - In return we store them into RangeSrcItem/RangeFirstItem/RangeLastItem and other fields in RblxUIMultiSelectIO.
// - Most applications will store an object INDEX, hence the chosen name and type. Storing an index is natural, because
//   SetRange requests will give you two end-points and you will need to iterate/interpolate between them to update your selection.
// - However it is perfectly possible to store a POINTER or another IDENTIFIER inside RblxUISelectionUserData.
//   Our system never assume that you identify items by indices, it never attempts to interpolate between two values.
// - If you enable RblxUIMultiSelectFlags_NoRangeSelect then it is guaranteed that you will never have to interpolate
//   between two RblxUISelectionUserData, which may be a convenient way to use part of the feature with less code work.
// - As most users will want to store an index, for convenience and to reduce confusion we use RblxS64 instead of void*,
//   being syntactically easier to downcast. Feel free to reinterpret_cast and store a pointer inside.

// Flags for BeginMultiSelect()
enum RblxUIMultiSelectFlags_
{
    RblxUIMultiSelectFlags_None                  = 0,
    RblxUIMultiSelectFlags_SingleSelect          = 1 << 0,   // Disable selecting more than one item. This is available to allow single-selection code to share same code/logic if desired. It essentially disables the main purpose of BeginMultiSelect() tho!
    RblxUIMultiSelectFlags_NoSelectAll           = 1 << 1,   // Disable Ctrl+A shortcut to select all.
    RblxUIMultiSelectFlags_NoRangeSelect         = 1 << 2,   // Disable Shift+selection mouse/keyboard support (useful for unordered 2D selection). With BoxSelect is also ensure contiguous SetRange requests are not combined into one. This allows not handling interpolation in SetRange requests.
    RblxUIMultiSelectFlags_NoAutoSelect          = 1 << 3,   // Disable selecting items when navigating (useful for e.g. supporting range-select in a list of checkboxes).
    RblxUIMultiSelectFlags_NoAutoClear           = 1 << 4,   // Disable clearing selection when navigating or selecting another one (generally used with RblxUIMultiSelectFlags_NoAutoSelect. useful for e.g. supporting range-select in a list of checkboxes).
    RblxUIMultiSelectFlags_NoAutoClearOnReselect = 1 << 5,   // Disable clearing selection when clicking/selecting an already selected item.
    RblxUIMultiSelectFlags_BoxSelect1d           = 1 << 6,   // Enable box-selection with same width and same x pos items (e.g. full row Selectable()). Box-selection works better with little bit of spacing between items hit-box in order to be able to aim at empty space.
    RblxUIMultiSelectFlags_BoxSelect2d           = 1 << 7,   // Enable box-selection with varying width or varying x pos items support (e.g. different width labels, or 2D layout/grid). This is slower: alters clipping logic so that e.g. horizontal movements will update selection of normally clipped items.
    RblxUIMultiSelectFlags_BoxSelectNoScroll     = 1 << 8,   // Disable scrolling when box-selecting and moving mouse near edges of scope.
    RblxUIMultiSelectFlags_ClearOnEscape         = 1 << 9,   // Clear selection when pressing Escape while scope is focused.
    RblxUIMultiSelectFlags_ClearOnClickVoid      = 1 << 10,  // Clear selection when clicking on empty location within scope.
    RblxUIMultiSelectFlags_ScopeWindow           = 1 << 11,  // Scope for _BoxSelect and _ClearOnClickVoid is whole window (Default). Use if BeginMultiSelect() covers a whole window or used a single time in same window.
    RblxUIMultiSelectFlags_ScopeRect             = 1 << 12,  // Scope for _BoxSelect and _ClearOnClickVoid is rectangle encompassing BeginMultiSelect()/EndMultiSelect(). Use if BeginMultiSelect() is called multiple times in same window.
    RblxUIMultiSelectFlags_SelectOnAuto          = 1 << 13,  // Apply selection on mouse down when clicking on unselected item, on mouse up when clicking on selected item. (Default)
    RblxUIMultiSelectFlags_SelectOnClickAlways   = 1 << 14,  // Apply selection on mouse down when clicking on any items. Prevents Drag and Drop from being used on multiple-selection, but allows e.g. BoxSelect to always reselect even when clicking inside an existing selection. (Excel style behavior)
    RblxUIMultiSelectFlags_SelectOnClickRelease  = 1 << 15,  // Apply selection on mouse release when clicking an unselected item. Allow dragging an unselected item without altering selection.
    //RblxUIMultiSelectFlags_RangeSelect2d       = 1 << 15,  // Shift+Selection uses 2d geometry instead of linear sequence, so possible to use Shift+up/down to select vertically in grid. Analogous to what BoxSelect does.
    RblxUIMultiSelectFlags_NavWrapX              = 1 << 16,  // [Temporary] Enable navigation wrapping on X axis. Provided as a convenience because we don't have a design for the general Nav API for this yet. When the more general feature be public we may obsolete this flag in favor of new one.
    RblxUIMultiSelectFlags_NoSelectOnRightClick  = 1 << 17,  // Disable default right-click processing, which selects item on mouse down, and is designed for context-menus.
    RblxUIMultiSelectFlags_SelectOnMask_         = RblxUIMultiSelectFlags_SelectOnAuto | RblxUIMultiSelectFlags_SelectOnClickAlways | RblxUIMultiSelectFlags_SelectOnClickRelease,

    // Obsolete names
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    RblxUIMultiSelectFlags_SelectOnClick         = RblxUIMultiSelectFlags_SelectOnAuto,         // RENAMED in 1.92.6
#endif
};

// Main IO structure returned by BeginMultiSelect()/EndMultiSelect().
// This mainly contains a list of selection requests.
// - Use 'Demo->Tools->Debug Log->Selection' to see requests as they happen.
// - Some fields are only useful if your list is dynamic and allows deletion (getting post-deletion focus/state right is shown in the demo)
// - Below: who reads/writes each fields? 'r'=read, 'w'=write, 'ms'=multi-select code, 'app'=application/user code.
struct RblxUIMultiSelectIO
{
    //------------------------------------------// BeginMultiSelect / EndMultiSelect
    RblxVector<RblxUISelectionRequest> Requests;   //  ms:w, app:r     /  ms:w  app:r   // Requests to apply to your selection data.
    RblxUISelectionUserData      RangeSrcItem;   //  ms:w  app:r     /                // (If using clipper) Begin: Source item (often the first selected item) must never be clipped: use clipper.IncludeItemByIndex() to ensure it is submitted.
    RblxUISelectionUserData      NavIdItem;      //  ms:w, app:r     /                // (If using deletion) Last known SetNextItemSelectionUserData() value for NavId (if part of submitted items).
    bool                        NavIdSelected;  //  ms:w, app:r     /        app:r   // (If using deletion) Last known selection state for NavId (if part of submitted items).
    bool                        RangeSrcReset;  //        app:w     /  ms:r          // (If using deletion) Set before EndMultiSelect() to reset ResetSrcItem (e.g. if deleted selection).
    int                         ItemsCount;     //  ms:w, app:r     /        app:r   // 'int items_count' parameter to BeginMultiSelect() is copied here for convenience, allowing simpler calls to your ApplyRequests handler. Not used internally.
};

// Selection request type
enum RblxUISelectionRequestType
{
    RblxUISelectionRequestType_None = 0,
    RblxUISelectionRequestType_SetAll,           // Request app to clear selection (if Selected==false) or select all items (if Selected==true). We cannot set RangeFirstItem/RangeLastItem as its contents is entirely up to user (not necessarily an index)
    RblxUISelectionRequestType_SetRange,         // Request app to select/unselect [RangeFirstItem..RangeLastItem] items (inclusive) based on value of Selected. Only EndMultiSelect() request this, app code can read after BeginMultiSelect() and it will always be false.
};

// Selection request item
struct RblxUISelectionRequest
{
    //------------------------------------------// BeginMultiSelect / EndMultiSelect
    RblxUISelectionRequestType   Type;           //  ms:w, app:r     /  ms:w, app:r   // Request type. You'll most often receive 1 Clear + 1 SetRange with a single-item range.
    bool                        Selected;       //  ms:w, app:r     /  ms:w, app:r   // Parameter for SetAll/SetRange requests (true = select, false = unselect)
    RblxS8                        RangeDirection; //                  /  ms:w  app:r   // Parameter for SetRange request: +1 when RangeFirstItem comes before RangeLastItem, -1 otherwise. Useful if you want to preserve selection order on a backward Shift+Click.
    RblxUISelectionUserData      RangeFirstItem; //                  /  ms:w, app:r   // Parameter for SetRange request (this is generally == RangeSrcItem when shift selecting from top to bottom).
    RblxUISelectionUserData      RangeLastItem;  //                  /  ms:w, app:r   // Parameter for SetRange request (this is generally == RangeSrcItem when shift selecting from bottom to top). Inclusive!
};

// Optional helper to store multi-selection state + apply multi-selection requests.
// - Used by our demos and provided as a convenience to easily implement basic multi-selection.
// - Iterate selection with 'void* it = NULL; RblxUIID id; while (selection.GetNextSelectedItem(&it, &id)) { ... }'
//   Or you can check 'if (Contains(id)) { ... }' for each possible object if their number is not too high to iterate.
// - USING THIS IS NOT MANDATORY. This is only a helper and not a required API.
// To store a multi-selection, in your application you could:
// - Use this helper as a convenience. We use our simple key->value RblxUIStorage as a std::set<RblxUIID> replacement.
// - Use your own external storage: e.g. std::set<MyObjectId>, std::vector<MyObjectId>, interval trees, intrusively stored selection etc.
// In RblxUISelectionBasicStorage we:
// - always use indices in the multi-selection API (passed to SetNextItemSelectionUserData(), retrieved in RblxUIMultiSelectIO)
// - use the AdapterIndexToStorageId() indirection layer to abstract how persistent selection data is derived from an index.
// - use decently optimized logic to allow queries and insertion of very large selection sets.
// - do not preserve selection order.
// Many combinations are possible depending on how you prefer to store your items and how you prefer to store your selection.
// Large applications are likely to eventually want to get rid of this indirection layer and do their own thing.
// See https://github.com/ocornut/rblxui/wiki/Multi-Select for details and pseudo-code using this helper.
struct RblxUISelectionBasicStorage
{
    // Members
    int             Size;           //          // Number of selected items, maintained by this helper.
    bool            PreserveOrder;  // = false  // GetNextSelectedItem() will return ordered selection (currently implemented by two additional sorts of selection. Could be improved)
    void*           UserData;       // = NULL   // User data for use by adapter function        // e.g. selection.UserData = (void*)my_items;
    RblxUIID         (*AdapterIndexToStorageId)(RblxUISelectionBasicStorage* self, int idx);      // e.g. selection.AdapterIndexToStorageId = [](RblxUISelectionBasicStorage* self, int idx) { return ((MyItems**)self->UserData)[idx]->ID; };
    int             _SelectionOrder;// [Internal] Increasing counter to store selection order
    RblxUIStorage    _Storage;       // [Internal] Selection set. Think of this as similar to e.g. std::set<RblxUIID>. Prefer not accessing directly: iterate with GetNextSelectedItem().

    // Methods
    RBLXUI_API RblxUISelectionBasicStorage();
    RBLXUI_API void  ApplyRequests(RblxUIMultiSelectIO* ms_io);   // Apply selection requests coming from BeginMultiSelect() and EndMultiSelect() functions. It uses 'items_count' passed to BeginMultiSelect()
    RBLXUI_API bool  Contains(RblxUIID id) const;                 // Query if an item id is in selection.
    RBLXUI_API void  Clear();                                    // Clear selection
    RBLXUI_API void  Swap(RblxUISelectionBasicStorage& r);        // Swap two selections
    RBLXUI_API void  SetItemSelected(RblxUIID id, bool selected); // Add/remove an item from selection (generally done by ApplyRequests() function)
    RBLXUI_API bool  GetNextSelectedItem(void** opaque_it, RblxUIID* out_id); // Iterate selection with 'void* it = NULL; RblxUIID id; while (selection.GetNextSelectedItem(&it, &id)) { ... }'
    inline RblxUIID  GetStorageIdFromIndex(int idx)              { return AdapterIndexToStorageId(this, idx); }  // Convert index to item id based on provided adapter.
};

// Optional helper to apply multi-selection requests to existing randomly accessible storage.
// Convenient if you want to quickly wire multi-select API on e.g. an array of bool or items storing their own selection state.
struct RblxUISelectionExternalStorage
{
    // Members
    void*           UserData;       // User data for use by adapter function                                // e.g. selection.UserData = (void*)my_items;
    void            (*AdapterSetItemSelected)(RblxUISelectionExternalStorage* self, int idx, bool selected); // e.g. AdapterSetItemSelected = [](RblxUISelectionExternalStorage* self, int idx, bool selected) { ((MyItems**)self->UserData)[idx]->Selected = selected; }

    // Methods
    RBLXUI_API RblxUISelectionExternalStorage();
    RBLXUI_API void  ApplyRequests(RblxUIMultiSelectIO* ms_io);   // Apply selection requests by using AdapterSetItemSelected() calls
};

//-----------------------------------------------------------------------------
// [SECTION] Drawing API (RblxDrawCmd, RblxDrawIdx, RblxDrawVert, RblxDrawChannel, RblxDrawListSplitter, RblxDrawListFlags, RblxDrawList, RblxDrawData)
// Hold a series of drawing commands. The user provides a renderer for RblxDrawData which essentially contains an array of RblxDrawList.
//-----------------------------------------------------------------------------

// The maximum line width to bake anti-aliased textures for. Build atlas with RblxFontAtlasFlags_NoBakedLines to disable baking.
#ifndef RBLXUI_DRAWLIST_TEX_LINES_WIDTH_MAX
#define RBLXUI_DRAWLIST_TEX_LINES_WIDTH_MAX     (32)
#endif

// RblxDrawIdx: vertex index. [Compile-time configurable type]
// - To use 16-bit indices + allow large meshes: backend need to set 'io.BackendFlags |= RblxUIBackendFlags_RendererHasVtxOffset' and handle RblxDrawCmd::VtxOffset (recommended).
// - To use 32-bit indices: override with '#define RblxDrawIdx unsigned int' in your rblxuiconfig.h file.
#ifndef RblxDrawIdx
typedef unsigned short RblxDrawIdx;   // Default: 16-bit (for maximum compatibility with renderer backends)
#endif

// RblxDrawCallback: Draw callbacks for advanced uses [configurable type: override in rblxuiconfig.h]
// NB: You most likely do NOT need to use draw callbacks just to create your own widget or customized UI rendering,
// you can poke into the draw list for that! Draw callback may be useful for example to:
//  A) Change your GPU render state,
//  B) render a complex 3D scene inside a UI element without an intermediate texture/render target, etc.
// The expected behavior from your rendering function is 'if (cmd.UserCallback != NULL) { cmd.UserCallback(parent_list, cmd); } else { RenderTriangles() }'
// If you want to override the signature of RblxDrawCallback, you can simply use e.g. '#define RblxDrawCallback MyDrawCallback' (in rblxuiconfig.h) + update rendering backend accordingly.
#ifndef RblxDrawCallback
typedef void (*RblxDrawCallback)(const RblxDrawList* parent_list, const RblxDrawCmd* cmd);
#endif

// Typically, 1 command = 1 GPU draw call (unless command is a callback)
// - VtxOffset: When 'io.BackendFlags & RblxUIBackendFlags_RendererHasVtxOffset' is enabled,
//   this fields allow us to render meshes larger than 64K vertices while keeping 16-bit indices.
//   Backends made for <1.71. will typically ignore the VtxOffset fields.
// - The ClipRect/TexRef/VtxOffset fields must be contiguous as we memcmp() them together (this is asserted for).
struct RblxDrawCmd
{
    RblxVec4          ClipRect;           // 4*4  // Clipping rectangle (x1, y1, x2, y2). Subtract RblxDrawData->DisplayPos to get clipping rectangle in "viewport" coordinates
    RblxTextureRef    TexRef;             // 16   // Reference to a font/texture atlas (where backend called RblxTextureData::SetTexID()) or to a user-provided texture ID (via e.g. RblxUI::Image() calls). Both will lead to a RblxTextureID value.
    unsigned int    VtxOffset;          // 4    // Start offset in vertex buffer. RblxUIBackendFlags_RendererHasVtxOffset: always 0, otherwise may be >0 to support meshes larger than 64K vertices with 16-bit indices.
    unsigned int    IdxOffset;          // 4    // Start offset in index buffer.
    unsigned int    ElemCount;          // 4    // Number of indices (multiple of 3) to be rendered as triangles. Vertices are stored in the callee RblxDrawList's vtx_buffer[] array, indices in idx_buffer[].
    RblxDrawCallback  UserCallback;       // 4-8  // If != NULL, call the function instead of rendering the vertices. clip_rect and texture_id will be set normally.
    void*           UserCallbackData;   // 4-8  // Callback user data (when UserCallback != NULL). If called AddCallback() with size == 0, this is a copy of the AddCallback() argument. If called AddCallback() with size > 0, this is pointing to a buffer where data is stored.
    int             UserCallbackDataSize;  // 4 // Size of callback user data when using storage, otherwise 0.
    int             UserCallbackDataOffset;// 4 // [Internal] Offset of callback user data when using storage, otherwise -1.

    RblxDrawCmd()     { memset((void*)this, 0, sizeof(*this)); } // Also ensure our padding fields are zeroed

    // Since 1.83: returns RblxTextureID associated with this draw call. Warning: DO NOT assume this is always same as 'TextureId' (we will change this function for an upcoming feature)
    // Since 1.92: removed RblxDrawCmd::TextureId field, the getter function must be used!
    inline RblxTextureID GetTexID() const;    // == (TexRef._TexData ? TexRef._TexData->TexID : TexRef._TexID)
};

// Vertex layout
#ifndef RBLXUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT
struct RblxDrawVert
{
    RblxVec2  pos;
    RblxVec2  uv;
    RblxU32   col;
};
#else
// You can override the vertex format layout by defining RBLXUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT in rblxuiconfig.h
// The code expect RblxVec2 pos (8 bytes), RblxVec2 uv (8 bytes), RblxU32 col (4 bytes), but you can re-order them or add other fields as needed to simplify integration in your engine.
// The type has to be described within the macro (you can either declare the struct or use a typedef). This is because RblxVec2/RblxU32 are likely not declared at the time you'd want to set your type up.
// NOTE: RBLXUI DOESN'T CLEAR THE STRUCTURE AND DOESN'T CALL A CONSTRUCTOR SO ANY CUSTOM FIELD WILL BE UNINITIALIZED. IF YOU ADD EXTRA FIELDS (SUCH AS A 'Z' COORDINATES) YOU WILL NEED TO CLEAR THEM DURING RENDER OR TO IGNORE THEM.
RBLXUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT;
#endif

// [Internal] For use by RblxDrawList
struct RblxDrawCmdHeader
{
    RblxVec4          ClipRect;
    RblxTextureRef    TexRef;
    unsigned int    VtxOffset;
};

// [Internal] For use by RblxDrawListSplitter
struct RblxDrawChannel
{
    RblxVector<RblxDrawCmd>         _CmdBuffer;
    RblxVector<RblxDrawIdx>         _IdxBuffer;
};

// Split/Merge functions are used to split the draw list into different layers which can be drawn into out of order.
// This is used by the Columns/Tables API, so items of each column can be batched together in a same draw call.
struct RblxDrawListSplitter
{
    int                         _Current;    // Current channel number (0)
    int                         _Count;      // Number of active channels (1+)
    RblxVector<RblxDrawChannel>     _Channels;   // Draw channels (not resized down so _Count might be < Channels.Size)

    inline RblxDrawListSplitter()  { memset((void*)this, 0, sizeof(*this)); }
    inline ~RblxDrawListSplitter() { ClearFreeMemory(); }
    inline void                 Clear() { _Current = 0; _Count = 1; } // Do not clear Channels[] so our allocations are reused next frame
    RBLXUI_API void              ClearFreeMemory();
    RBLXUI_API void              Split(RblxDrawList* draw_list, int count);
    RBLXUI_API void              Merge(RblxDrawList* draw_list);
    RBLXUI_API void              SetCurrentChannel(RblxDrawList* draw_list, int channel_idx);
};

// Flags for RblxDrawList functions
enum RblxDrawFlags_
{
    RblxDrawFlags_None                        = 0,

    // Rounding for AddRect(), AddRectFilled(), PathRect()
    // - When not specified, we defaults to RblxDrawFlags_RoundCornersAll! So you only need to use those flags if you want another configuration.
    RblxDrawFlags_RoundCornersTopLeft         = 1 << 4, // Round top-left corner only (when rounding > 0.0f, we default to all corners).
    RblxDrawFlags_RoundCornersTopRight        = 1 << 5, // Round top-right corner only (when rounding > 0.0f, we default to all corners).
    RblxDrawFlags_RoundCornersBottomLeft      = 1 << 6, // Round bottom-left corner only (when rounding > 0.0f, we default to all corners).
    RblxDrawFlags_RoundCornersBottomRight     = 1 << 7, // Round bottom-right corner only (when rounding > 0.0f, we default to all corners).
    RblxDrawFlags_RoundCornersNone            = 1 << 8, // Disable rounding even if `float rounding > 0.0f`. This is NOT zero, NOT an implicit flag!
    RblxDrawFlags_RoundCornersAll             = RblxDrawFlags_RoundCornersTopLeft | RblxDrawFlags_RoundCornersTopRight | RblxDrawFlags_RoundCornersBottomLeft | RblxDrawFlags_RoundCornersBottomRight, // (Default!!)
    RblxDrawFlags_RoundCornersDefault_        = RblxDrawFlags_RoundCornersAll, // Default to ALL corners if none of the _RoundCornersXX flags are specified!
    RblxDrawFlags_RoundCornersTop             = RblxDrawFlags_RoundCornersTopLeft | RblxDrawFlags_RoundCornersTopRight,
    RblxDrawFlags_RoundCornersBottom          = RblxDrawFlags_RoundCornersBottomLeft | RblxDrawFlags_RoundCornersBottomRight,
    RblxDrawFlags_RoundCornersLeft            = RblxDrawFlags_RoundCornersBottomLeft | RblxDrawFlags_RoundCornersTopLeft,
    RblxDrawFlags_RoundCornersRight           = RblxDrawFlags_RoundCornersBottomRight | RblxDrawFlags_RoundCornersTopRight,
    RblxDrawFlags_RoundCornersMask_           = RblxDrawFlags_RoundCornersAll | RblxDrawFlags_RoundCornersNone,

    // Stroke options
    RblxDrawFlags_Closed                      = 1 << 9, // PathStroke(), AddPolyline(): specify that shape should be closed.
    //RblxDrawFlags_Closed                    = 1 << 0, // Prior to 1.92.8 (May 2026), RblxDrawFlags_Closed was guaranteed to be == 1<<0 == 1 for legacy compatibility reason. Hardcoded use of 1 or true should be replaced.

    RblxDrawFlags_InvalidMask_                = ~0x7FFFFFF0, // == 0x8000000F,
};

// Flags for RblxDrawList instance. Those are set automatically by RblxUI:: functions from RblxUIIO settings, and generally not manipulated directly.
// It is however possible to temporarily alter flags between calls to RblxDrawList:: functions.
enum RblxDrawListFlags_
{
    RblxDrawListFlags_None                    = 0,
    RblxDrawListFlags_AntiAliasedLines        = 1 << 0,  // Enable anti-aliased lines/borders (*2 the number of triangles for 1.0f wide line or lines thin enough to be drawn using textures, otherwise *3 the number of triangles)
    RblxDrawListFlags_AntiAliasedLinesUseTex  = 1 << 1,  // Enable anti-aliased lines/borders using textures when possible. Require backend to render with bilinear filtering (NOT point/nearest filtering).
    RblxDrawListFlags_AntiAliasedFill         = 1 << 2,  // Enable anti-aliased edge around filled shapes (rounded rectangles, circles).
    RblxDrawListFlags_AllowVtxOffset          = 1 << 3,  // Can emit 'VtxOffset > 0' to allow large meshes. Set when 'RblxUIBackendFlags_RendererHasVtxOffset' is enabled.
    RblxDrawListFlags_TextNoPixelSnap         = 1 << 4,  // Disable automatically snapping AddText() calls to pixel boundaries.
};

// Draw command list
// This is the low-level list of polygons that RblxUI:: functions are filling. At the end of the frame,
// all command lists are passed to your RblxUIIO::RenderDrawListFn function for rendering.
// Each dear rblxui window contains its own RblxDrawList. You can use RblxUI::GetWindowDrawList() to
// access the current window draw list and draw custom primitives.
// You can interleave normal RblxUI:: calls and adding primitives to the current draw list.
// In single viewport mode, top-left is == GetMainViewport()->Pos (generally 0,0), bottom-right is == GetMainViewport()->Pos+Size (generally io.DisplaySize).
// You are totally free to apply whatever transformation matrix you want to the data (depending on the use of the transformation you may want to apply it to ClipRect as well!)
// Important: Primitives are always added to the list and not culled (culling is done at higher-level by RblxUI:: functions), if you use this API a lot consider coarse culling your drawn objects.
struct RblxDrawList
{
    // This is what you have to render
    RblxVector<RblxDrawCmd>     CmdBuffer;          // Draw commands. Typically 1 command = 1 GPU draw call, unless the command is a callback.
    RblxVector<RblxDrawIdx>     IdxBuffer;          // Index buffer. Each command consume RblxDrawCmd::ElemCount of those
    RblxVector<RblxDrawVert>    VtxBuffer;          // Vertex buffer.
    RblxDrawListFlags         Flags;              // Flags, you may poke into these to adjust anti-aliasing settings per-primitive.

    // [Internal, used while building lists]
    unsigned int            _VtxCurrentIdx;     // [Internal] generally == VtxBuffer.Size unless we are past 64K vertices, in which case this gets reset to 0.
    RblxDrawListSharedData*   _Data;              // Pointer to shared draw data (you can use RblxUI::GetDrawListSharedData() to get the one from current RblxUI context)
    RblxDrawVert*             _VtxWritePtr;       // [Internal] point within VtxBuffer.Data after each add command (to avoid using the RblxVector<> operators too much)
    RblxDrawIdx*              _IdxWritePtr;       // [Internal] point within IdxBuffer.Data after each add command (to avoid using the RblxVector<> operators too much)
    RblxVector<RblxVec2>        _Path;              // [Internal] current path building
    RblxDrawCmdHeader         _CmdHeader;         // [Internal] template of active commands. Fields should match those of CmdBuffer.back().
    RblxDrawListSplitter      _Splitter;          // [Internal] for channels api (note: prefer using your own persistent instance of RblxDrawListSplitter!)
    RblxVector<RblxVec4>        _ClipRectStack;     // [Internal]
    RblxVector<RblxTextureRef>  _TextureStack;      // [Internal]
    RblxVector<RblxU8>          _CallbacksDataBuf;  // [Internal]
    float                   _FringeScale;       // [Internal] anti-alias fringe is scaled by this value, this helps to keep things sharp while zooming at vertex buffer content
    const char*             _OwnerName;         // Pointer to owner window's name for debugging

    // If you want to create RblxDrawList instances, pass them RblxUI::GetDrawListSharedData().
    // (advanced: you may create and use your own RblxDrawListSharedData so you can use RblxDrawList without RblxUI, but that's more involved)
    RBLXUI_API RblxDrawList(RblxDrawListSharedData* shared_data);
    RBLXUI_API ~RblxDrawList();

    RBLXUI_API void  PushClipRect(const RblxVec2& clip_rect_min, const RblxVec2& clip_rect_max, bool intersect_with_current_clip_rect = false);  // Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level RblxUI::PushClipRect() to affect logic (hit-testing and widget culling)
    RBLXUI_API void  PushClipRectFullScreen();
    RBLXUI_API void  PopClipRect();
    RBLXUI_API void  PushTexture(RblxTextureRef tex_ref);
    RBLXUI_API void  PopTexture();
    inline RblxVec2   GetClipRectMin() const { const RblxVec4& cr = _ClipRectStack.back(); return RblxVec2(cr.x, cr.y); }
    inline RblxVec2   GetClipRectMax() const { const RblxVec4& cr = _ClipRectStack.back(); return RblxVec2(cr.z, cr.w); }

    // Primitives
    // - Filled shapes must always use clockwise winding order. The anti-aliasing fringe depends on it. Counter-clockwise shapes will have "inward" anti-aliasing.
    // - For rectangular primitives, "p_min" and "p_max" represent the upper-left and lower-right corners.
    // - For circle primitives, use "num_segments == 0" to automatically calculate tessellation (preferred).
    //   In older versions (until Dear RblxUI 1.77) the AddCircle functions defaulted to num_segments == 12.
    //   In future versions we will use textures to provide cheaper and higher-quality circles.
    //   Use AddNgon() and AddNgonFilled() functions if you need to guarantee a specific number of sides.
    RBLXUI_API void  AddLine(const RblxVec2& p1, const RblxVec2& p2, RblxU32 col, float thickness = 1.0f);
    RBLXUI_API void  AddLineH(float min_x, float max_x, float y, RblxU32 col, float thickness = 1.0f);
    RBLXUI_API void  AddLineV(float x, float min_y, float max_y, RblxU32 col, float thickness = 1.0f);
    RBLXUI_API void  AddRect(const RblxVec2& p_min, const RblxVec2& p_max, RblxU32 col, float rounding = 0.0f, float thickness = 1.0f, RblxDrawFlags flags = 0);   // a: upper-left, b: lower-right (== upper-left + size)
    RBLXUI_API void  AddRectFilled(const RblxVec2& p_min, const RblxVec2& p_max, RblxU32 col, float rounding = 0.0f, RblxDrawFlags flags = 0);                     // a: upper-left, b: lower-right (== upper-left + size)
    RBLXUI_API void  AddRectFilledMultiColor(const RblxVec2& p_min, const RblxVec2& p_max, RblxU32 col_upr_left, RblxU32 col_upr_right, RblxU32 col_bot_right, RblxU32 col_bot_left);
    RBLXUI_API void  AddQuad(const RblxVec2& p1, const RblxVec2& p2, const RblxVec2& p3, const RblxVec2& p4, RblxU32 col, float thickness = 1.0f);
    RBLXUI_API void  AddQuadFilled(const RblxVec2& p1, const RblxVec2& p2, const RblxVec2& p3, const RblxVec2& p4, RblxU32 col);
    RBLXUI_API void  AddTriangle(const RblxVec2& p1, const RblxVec2& p2, const RblxVec2& p3, RblxU32 col, float thickness = 1.0f);
    RBLXUI_API void  AddTriangleFilled(const RblxVec2& p1, const RblxVec2& p2, const RblxVec2& p3, RblxU32 col);
    RBLXUI_API void  AddCircle(const RblxVec2& center, float radius, RblxU32 col, int num_segments = 0, float thickness = 1.0f);
    RBLXUI_API void  AddCircleFilled(const RblxVec2& center, float radius, RblxU32 col, int num_segments = 0);
    RBLXUI_API void  AddNgon(const RblxVec2& center, float radius, RblxU32 col, int num_segments, float thickness = 1.0f);
    RBLXUI_API void  AddNgonFilled(const RblxVec2& center, float radius, RblxU32 col, int num_segments);
    RBLXUI_API void  AddEllipse(const RblxVec2& center, const RblxVec2& radius, RblxU32 col, float rot = 0.0f, int num_segments = 0, float thickness = 1.0f);
    RBLXUI_API void  AddEllipseFilled(const RblxVec2& center, const RblxVec2& radius, RblxU32 col, float rot = 0.0f, int num_segments = 0);
    RBLXUI_API void  AddText(const RblxVec2& pos, RblxU32 col, const char* text_begin, const char* text_end = NULL);
    RBLXUI_API void  AddText(RblxFont* font, float font_size, const RblxVec2& pos, RblxU32 col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const RblxVec4* cpu_fine_clip_rect = NULL);
    RBLXUI_API void  AddBezierCubic(const RblxVec2& p1, const RblxVec2& p2, const RblxVec2& p3, const RblxVec2& p4, RblxU32 col, float thickness, int num_segments = 0); // Cubic Bezier (4 control points)
    RBLXUI_API void  AddBezierQuadratic(const RblxVec2& p1, const RblxVec2& p2, const RblxVec2& p3, RblxU32 col, float thickness, int num_segments = 0);               // Quadratic Bezier (3 control points)

    // General polygon
    // - Only simple polygons are supported by filling functions (no self-intersections, no holes).
    // - Concave polygon fill is more expensive than convex one: it has O(N^2) complexity. Provided as a convenience for the user but not used by the main library.
    RBLXUI_API void  AddPolyline(const RblxVec2* points, int num_points, RblxU32 col, float thickness, RblxDrawFlags flags = 0);
    RBLXUI_API void  AddConvexPolyFilled(const RblxVec2* points, int num_points, RblxU32 col);
    RBLXUI_API void  AddConcavePolyFilled(const RblxVec2* points, int num_points, RblxU32 col);

    // Image primitives
    // - Read FAQ to understand what RblxTextureID/RblxTextureRef are.
    // - "p_min" and "p_max" represent the upper-left and lower-right corners of the rectangle.
    // - "uv_min" and "uv_max" represent the normalized texture coordinates to use for those corners. Using (0,0)->(1,1) texture coordinates will generally display the entire texture.
    RBLXUI_API void  AddImage(RblxTextureRef tex_ref, const RblxVec2& p_min, const RblxVec2& p_max, const RblxVec2& uv_min = RblxVec2(0, 0), const RblxVec2& uv_max = RblxVec2(1, 1), RblxU32 col = RBLXUI_COL32_WHITE);
    RBLXUI_API void  AddImageQuad(RblxTextureRef tex_ref, const RblxVec2& p1, const RblxVec2& p2, const RblxVec2& p3, const RblxVec2& p4, const RblxVec2& uv1 = RblxVec2(0, 0), const RblxVec2& uv2 = RblxVec2(1, 0), const RblxVec2& uv3 = RblxVec2(1, 1), const RblxVec2& uv4 = RblxVec2(0, 1), RblxU32 col = RBLXUI_COL32_WHITE);
    RBLXUI_API void  AddImageRounded(RblxTextureRef tex_ref, const RblxVec2& p_min, const RblxVec2& p_max, const RblxVec2& uv_min, const RblxVec2& uv_max, RblxU32 col, float rounding, RblxDrawFlags flags = 0);

    // Stateful path API, add points then finish with PathFillConvex() or PathStroke()
    // - Important: filled shapes must always use clockwise winding order! The anti-aliasing fringe depends on it. Counter-clockwise shapes will have "inward" anti-aliasing.
    //   so e.g. 'PathArcTo(center, radius, PI * -0.5f, PI)' is ok, whereas 'PathArcTo(center, radius, PI, PI * -0.5f)' won't have correct anti-aliasing when followed by PathFillConvex().
    inline    void  PathClear()                                                 { _Path.Size = 0; }
    inline    void  PathLineTo(const RblxVec2& pos)                               { _Path.push_back(pos); }
    inline    void  PathLineToMergeDuplicate(const RblxVec2& pos)                 { if (_Path.Size == 0 || memcmp(&_Path.Data[_Path.Size - 1], &pos, 8) != 0) _Path.push_back(pos); }
    inline    void  PathFillConvex(RblxU32 col)                                   { AddConvexPolyFilled(_Path.Data, _Path.Size, col); _Path.Size = 0; }
    inline    void  PathFillConcave(RblxU32 col)                                  { AddConcavePolyFilled(_Path.Data, _Path.Size, col); _Path.Size = 0; }
    inline    void  PathStroke(RblxU32 col, float thickness = 1.0f, RblxDrawFlags flags = 0) { AddPolyline(_Path.Data, _Path.Size, col, thickness, flags); _Path.Size = 0; }
    RBLXUI_API void  PathArcTo(const RblxVec2& center, float radius, float a_min, float a_max, int num_segments = 0);
    RBLXUI_API void  PathArcToFast(const RblxVec2& center, float radius, int a_min_of_12, int a_max_of_12);                // Use precomputed angles for a 12 steps circle
    RBLXUI_API void  PathEllipticalArcTo(const RblxVec2& center, const RblxVec2& radius, float rot, float a_min, float a_max, int num_segments = 0); // Ellipse
    RBLXUI_API void  PathBezierCubicCurveTo(const RblxVec2& p2, const RblxVec2& p3, const RblxVec2& p4, int num_segments = 0); // Cubic Bezier (4 control points)
    RBLXUI_API void  PathBezierQuadraticCurveTo(const RblxVec2& p2, const RblxVec2& p3, int num_segments = 0);               // Quadratic Bezier (3 control points)
    RBLXUI_API void  PathRect(const RblxVec2& rect_min, const RblxVec2& rect_max, float rounding = 0.0f, RblxDrawFlags flags = 0);

    // Advanced: Draw Callbacks
    // - May be used to alter render state (change sampler, blending, current shader). May be used to emit custom rendering commands (difficult to do correctly, but possible).
    // - Use special GetPlatformIO().DrawCallback_ResetRenderState callback to instruct backend to reset its render state to the default.
    // - See other standard callbacks in GetPlatformIO(), which may or not be supported by your backend.
    // - Your rendering loop must check for 'UserCallback' in RblxDrawCmd and call the function instead of rendering triangles. All standard backends are honoring this.
    // - For some backends, the callback may access selected render-states exposed by the backend in a RblxUI_ImplXXXX_RenderState structure pointed to by platform_io.Renderer_RenderState.
    // - IMPORTANT: please be mindful of the different level of indirection between using size==0 (copying argument) and using size>0 (copying pointed data into a buffer).
    //   - If userdata_size == 0: we copy/store the 'userdata' argument as-is. It will be available unmodified in RblxDrawCmd::UserCallbackData during render.
    //   - If userdata_size > 0,  we copy/store 'userdata_size' bytes pointed to by 'userdata'. We store them in a buffer stored inside the drawlist. RblxDrawCmd::UserCallbackData will point inside that buffer so you have to retrieve data from there. Your callback may need to use RblxDrawCmd::UserCallbackDataSize if you expect dynamically-sized data.
    //   - Support for userdata_size > 0 was added in v1.91.4, October 2024. So earlier code always only allowed to copy/store a simple void*.
    RBLXUI_API void  AddCallback(RblxDrawCallback callback, void* userdata = NULL, size_t userdata_size = 0);

    // Advanced: Miscellaneous
    RBLXUI_API void  AddDrawCmd();                                               // This is useful if you need to forcefully create a new draw call (to allow for dependent rendering / blending). Otherwise primitives are merged into the same draw-call as much as possible
    RBLXUI_API RblxDrawList* CloneOutput() const;                                  // Create a clone of the CmdBuffer/IdxBuffer/VtxBuffer. For multi-threaded rendering, consider using `rblxui_threaded_rendering` from https://github.com/ocornut/rblxui_club instead.

    // Advanced: Channels
    // - Use to split render into layers. By switching channels to can render out-of-order (e.g. submit FG primitives before BG primitives)
    // - Use to minimize draw calls (e.g. if going back-and-forth between multiple clipping rectangles, prefer to append into separate channels then merge at the end)
    // - This API shouldn't have been in RblxDrawList in the first place!
    //   Prefer using your own persistent instance of RblxDrawListSplitter as you can stack them.
    //   Using the RblxDrawList::ChannelsXXXX you cannot stack a split over another.
    inline void     ChannelsSplit(int count)    { _Splitter.Split(this, count); }
    inline void     ChannelsMerge()             { _Splitter.Merge(this); }
    inline void     ChannelsSetCurrent(int n)   { _Splitter.SetCurrentChannel(this, n); }

    // Advanced: Primitives allocations
    // - We render triangles (three vertices)
    // - All primitives needs to be reserved via PrimReserve() beforehand.
    RBLXUI_API void  PrimReserve(int idx_count, int vtx_count);
    RBLXUI_API void  PrimUnreserve(int idx_count, int vtx_count);
    RBLXUI_API void  PrimRect(const RblxVec2& a, const RblxVec2& b, RblxU32 col);      // Axis aligned rectangle (composed of two triangles)
    RBLXUI_API void  PrimRectUV(const RblxVec2& a, const RblxVec2& b, const RblxVec2& uv_a, const RblxVec2& uv_b, RblxU32 col);
    RBLXUI_API void  PrimQuadUV(const RblxVec2& a, const RblxVec2& b, const RblxVec2& c, const RblxVec2& d, const RblxVec2& uv_a, const RblxVec2& uv_b, const RblxVec2& uv_c, const RblxVec2& uv_d, RblxU32 col);
    inline    void  PrimWriteVtx(const RblxVec2& pos, const RblxVec2& uv, RblxU32 col)    { _VtxWritePtr->pos = pos; _VtxWritePtr->uv = uv; _VtxWritePtr->col = col; _VtxWritePtr++; _VtxCurrentIdx++; }
    inline    void  PrimWriteIdx(RblxDrawIdx idx)                                     { *_IdxWritePtr = idx; _IdxWritePtr++; }
    inline    void  PrimVtx(const RblxVec2& pos, const RblxVec2& uv, RblxU32 col)         { PrimWriteIdx((RblxDrawIdx)_VtxCurrentIdx); PrimWriteVtx(pos, uv, col); } // Write vertex with unique index

    // Obsolete names
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    inline    void  AddRect(const RblxVec2& p_min, const RblxVec2& p_max, RblxU32 col, float rounding, RblxDrawFlags flags, float thickness) { AddRect(p_min, p_max, col, rounding, thickness, flags); } // OBSOLETED in 1.92.8: NEW FUNCTION SIGNATURE HAS 'thickness' AND 'flags' SWAPPED.
    inline    void  AddPolyline(const RblxVec2* points, int num_points, RblxU32 col, RblxDrawFlags flags, float thickness)                 { AddPolyline(points, num_points, col, thickness, flags); } // OBSOLETED in 1.92.8: NEW FUNCTION SIGNATURE HAS 'thickness' AND 'flags' SWAPPED.
    inline    void  PathStroke(RblxU32 col, RblxDrawFlags flags, float thickness)                                                        { PathStroke(col, thickness, flags); }                      // OBSOLETED in 1.92.8: NEW FUNCTION SIGNATURE HAS 'thickness' AND 'flags' SWAPPED.
    inline    void  PushTextureID(RblxTextureRef tex_ref) { PushTexture(tex_ref); }   // RENAMED in 1.92.0
    inline    void  PopTextureID()                      { PopTexture(); }           // RENAMED in 1.92.0
#else
              void  AddRect(const RblxVec2& p_min, const RblxVec2& p_max, RblxU32 col, float rounding /*= 0.0f*/, RblxDrawFlags flags /*= 0*/, float thickness /*= 1.0f*/) = delete;
              void  AddPolyline(const RblxVec2* points, int num_points, RblxU32 col, RblxDrawFlags flags, float thickness) = delete;
    inline    void  PathStroke(RblxU32 col, RblxDrawFlags flags /*= 0*/, float thickness /*= 1.0f*/) = delete;
#endif
    //inline  void  AddEllipse(const RblxVec2& center, float radius_x, float radius_y, RblxU32 col, float rot = 0.0f, int num_segments = 0, float thickness = 1.0f) { AddEllipse(center, RblxVec2(radius_x, radius_y), col, rot, num_segments, thickness); } // OBSOLETED in 1.90.5 (Mar 2024)
    //inline  void  AddEllipseFilled(const RblxVec2& center, float radius_x, float radius_y, RblxU32 col, float rot = 0.0f, int num_segments = 0) { AddEllipseFilled(center, RblxVec2(radius_x, radius_y), col, rot, num_segments); }                        // OBSOLETED in 1.90.5 (Mar 2024)
    //inline  void  PathEllipticalArcTo(const RblxVec2& center, float radius_x, float radius_y, float rot, float a_min, float a_max, int num_segments = 0) { PathEllipticalArcTo(center, RblxVec2(radius_x, radius_y), rot, a_min, a_max, num_segments); } // OBSOLETED in 1.90.5 (Mar 2024)
    //inline  void  AddBezierCurve(const RblxVec2& p1, const RblxVec2& p2, const RblxVec2& p3, const RblxVec2& p4, RblxU32 col, float thickness, int num_segments = 0) { AddBezierCubic(p1, p2, p3, p4, col, thickness, num_segments); }                         // OBSOLETED in 1.80 (Jan 2021)
    //inline  void  PathBezierCurveTo(const RblxVec2& p2, const RblxVec2& p3, const RblxVec2& p4, int num_segments = 0) { PathBezierCubicCurveTo(p2, p3, p4, num_segments); }                                                                                // OBSOLETED in 1.80 (Jan 2021)

    // [Internal helpers]
    RBLXUI_API void  _SetDrawListSharedData(RblxDrawListSharedData* data);
    RBLXUI_API void  _ResetForNewFrame();
    RBLXUI_API void  _ClearFreeMemory();
    RBLXUI_API void  _PopUnusedDrawCmd();
    RBLXUI_API void  _TryMergeDrawCmds();
    RBLXUI_API void  _OnChangedClipRect();
    RBLXUI_API void  _OnChangedTexture();
    RBLXUI_API void  _OnChangedVtxOffset();
    RBLXUI_API void  _SetTexture(RblxTextureRef tex_ref);
    RBLXUI_API int   _CalcCircleAutoSegmentCount(float radius) const;
    RBLXUI_API void  _PathArcToFastEx(const RblxVec2& center, float radius, int a_min_sample, int a_max_sample, int a_step);
    RBLXUI_API void  _PathArcToN(const RblxVec2& center, float radius, float a_min, float a_max, int num_segments);
};

// All draw data to render a Dear RblxUI frame
// (NB: the style and the naming convention here is a little inconsistent, we currently preserve them for backward compatibility purpose,
// as this is one of the oldest structure exposed by the library! Basically, RblxDrawList == CmdList)
struct RblxDrawData
{
    bool                Valid;              // Only valid after Render() is called and before the next NewFrame() is called.
    int                 CmdListsCount;      // == CmdLists.Size. (OBSOLETE: exists for legacy reasons). Number of RblxDrawList* to render.
    int                 TotalIdxCount;      // For convenience, sum of all RblxDrawList's IdxBuffer.Size
    int                 TotalVtxCount;      // For convenience, sum of all RblxDrawList's VtxBuffer.Size
    RblxVector<RblxDrawList*> CmdLists;         // Array of RblxDrawList* to render. The RblxDrawLists are owned by RblxUIContext and only pointed to from here.
    RblxVec2              DisplayPos;         // Top-left position of the viewport to render (== top-left of the orthogonal projection matrix to use) (== GetMainViewport()->Pos for the main viewport, == (0.0) in most single-viewport applications)
    RblxVec2              DisplaySize;        // Size of the viewport to render (== GetMainViewport()->Size for the main viewport, == io.DisplaySize in most single-viewport applications)
    RblxVec2              FramebufferScale;   // Amount of pixels for each unit of DisplaySize. Copied from viewport->FramebufferScale (== io.DisplayFramebufferScale for main viewport). Generally (1,1) on normal display, (2,2) on OSX with Retina display.
    RblxUIViewport*      OwnerViewport;      // Viewport carrying the RblxDrawData instance, might be of use to the renderer (generally not).
    RblxVector<RblxTextureData*>* Textures;     // List of textures to update. Most of the times the list is shared by all RblxDrawData, has only 1 texture and it doesn't need any update. This almost always points to RblxUI::GetPlatformIO().Textures[]. May be overridden or set to NULL if you want to manually update textures.

    // Functions
    RblxDrawData()    { Clear(); }
    RBLXUI_API void  Clear();
    RBLXUI_API void  AddDrawList(RblxDrawList* draw_list);     // Helper to add an external draw list into an existing RblxDrawData.
    RBLXUI_API void  DeIndexAllBuffers();                    // Helper to convert all buffers from indexed to non-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
    RBLXUI_API void  ScaleClipRects(const RblxVec2& fb_scale); // Helper to scale the ClipRect field of each RblxDrawCmd. Use if your final output buffer is at a different scale than Dear RblxUI expects, or if there is a difference between your window resolution and framebuffer resolution.
};

//-----------------------------------------------------------------------------
// [SECTION] Texture API (RblxTextureFormat, RblxTextureStatus, RblxTextureRect, RblxTextureData)
//-----------------------------------------------------------------------------
// In principle, the only data types that user/application code should care about are 'RblxTextureRef' and 'RblxTextureID'.
// They are defined above in this header file. Read their description to the difference between RblxTextureRef and RblxTextureID.
// FOR ALL OTHER RblxTextureXXXX TYPES: ONLY CORE LIBRARY AND RENDERER BACKENDS NEED TO KNOW AND CARE ABOUT THEM.
//-----------------------------------------------------------------------------

#undef Status // X11 headers are leaking this.

// We intentionally support a limited amount of texture formats to limit burden on CPU-side code and extension.
// Most standard backends only support RGBA32 but we provide a single channel option for low-resource/embedded systems.
enum RblxTextureFormat
{
    RblxTextureFormat_RGBA32,         // 4 components per pixel, each is unsigned 8-bit. Total size = TexWidth * TexHeight * 4
    RblxTextureFormat_Alpha8,         // 1 component per pixel, each is unsigned 8-bit. Total size = TexWidth * TexHeight
};

// Status of a texture to communicate with Renderer Backend.
enum RblxTextureStatus
{
    RblxTextureStatus_OK,
    RblxTextureStatus_Destroyed,      // Backend destroyed the texture.
    RblxTextureStatus_WantCreate,     // Requesting backend to create the texture. Set status OK when done.
    RblxTextureStatus_WantUpdates,    // Requesting backend to update specific blocks of pixels (write to texture portions which have never been used before). Set status OK when done.
    RblxTextureStatus_WantDestroy,    // Requesting backend to destroy the texture. Set status to Destroyed when done.
};

// Coordinates of a rectangle within a texture.
// When a texture is in RblxTextureStatus_WantUpdates state, we provide a list of individual rectangles to copy to the graphics system.
// You may use RblxTextureData::Updates[] for the list, or RblxTextureData::UpdateBox for a single bounding box.
struct RblxTextureRect
{
    unsigned short      x, y;       // Upper-left coordinates of rectangle to update
    unsigned short      w, h;       // Size of rectangle to update (in pixels)
};

// Specs and pixel storage for a texture used by Dear RblxUI.
// This is only useful for (1) core library and (2) backends. End-user/applications do not need to care about this.
// Renderer Backends will create a GPU-side version of this.
// Why does we store two identifiers: TexID and BackendUserData?
// - RblxTextureID    TexID           = lower-level identifier stored in RblxDrawCmd. RblxDrawCmd can refer to textures not created by the backend, and for which there's no RblxTextureData.
// - void*          BackendUserData = higher-level opaque storage for backend own book-keeping. Some backends may have enough with TexID and not need both.
 // In columns below: who reads/writes each fields? 'r'=read, 'w'=write, 'core'=main library, 'backend'=renderer backend
struct RblxTextureData
{
    //------------------------------------------ core / backend ---------------------------------------
    int                 UniqueID;               // w    -   // [DEBUG] Sequential index to facilitate identifying a texture when debugging/printing. Unique per atlas.
    RblxTextureStatus     Status;                 // rw   rw  // RblxTextureStatus_OK/_WantCreate/_WantUpdates/_WantDestroy. Always use SetStatus() to modify!
    void*               BackendUserData;        // -    rw  // Convenience storage for backend. Some backends may have enough with TexID.
    RblxTextureID         TexID;                  // r    w   // Backend-specific texture identifier. Always use SetTexID() to modify! The identifier will stored in RblxDrawCmd::GetTexID() and passed to backend's RenderDrawData function.
    RblxTextureFormat     Format;                 // w    r   // RblxTextureFormat_RGBA32 (default) or RblxTextureFormat_Alpha8
    int                 Width;                  // w    r   // Texture width
    int                 Height;                 // w    r   // Texture height
    int                 BytesPerPixel;          // w    r   // 4 or 1
    unsigned char*      Pixels;                 // w    r   // Pointer to buffer holding 'Width*Height' pixels and 'Width*Height*BytesPerPixels' bytes.
    RblxTextureRect       UsedRect;               // w    r   // Bounding box encompassing all past and queued Updates[].
    RblxTextureRect       UpdateRect;             // w    r   // Bounding box encompassing all queued Updates[].
    RblxVector<RblxTextureRect> Updates;            // w    r   // Array of individual updates.
    int                 UnusedFrames;           // w    r   // In order to facilitate handling Status==WantDestroy in some backend: this is a count successive frames where the texture was not used. Always >0 when Status==WantDestroy.
    unsigned short      RefCount;               // w    r   // Number of contexts using this texture. Used during backend shutdown.
    bool                UseColors;              // w    r   // Tell whether our texture data is known to use colors (rather than just white + alpha).
    bool                WantDestroyNextFrame;   // rw   -   // [Internal] Queued to set RblxTextureStatus_WantDestroy next frame. May still be used in the current frame.

    // Functions
    // - If GetPixels() functions asserts while being called by your render loop, it could be caused by calling RblxFontAtlas::Clear()/ClearFonts()?
    RblxTextureData()     { memset((void*)this, 0, sizeof(*this)); Status = RblxTextureStatus_Destroyed; TexID = RblxTextureID_Invalid; }
    ~RblxTextureData()    { DestroyPixels(); }
    RBLXUI_API void      Create(RblxTextureFormat format, int w, int h);
    RBLXUI_API void      DestroyPixels();
    void*               GetPixels()                 { RBLXUI_ASSERT(Pixels != NULL); return Pixels; }
    void*               GetPixelsAt(int x, int y)   { RBLXUI_ASSERT(Pixels != NULL); return Pixels + (x + y * Width) * BytesPerPixel; }
    int                 GetSizeInBytes() const      { return Width * Height * BytesPerPixel; }
    int                 GetPitch() const            { return Width * BytesPerPixel; }
    RblxTextureRef        GetTexRef()                 { RblxTextureRef tex_ref; tex_ref._TexData = this; tex_ref._TexID = RblxTextureID_Invalid; return tex_ref; }
    RblxTextureID         GetTexID() const            { return TexID; }

    // Called by Renderer backend
    // - Call SetTexID() and SetStatus() after honoring texture requests. Never modify TexID and Status directly!
    // - A backend may decide to destroy a texture that we did not request to destroy, which is fine (e.g. freeing resources), but we immediately set the texture back in _WantCreate mode.
    void    SetTexID(RblxTextureID tex_id)            { TexID = tex_id; }
    void    SetStatus(RblxTextureStatus status)       { Status = status; if (status == RblxTextureStatus_Destroyed && !WantDestroyNextFrame && Pixels != nullptr) Status = RblxTextureStatus_WantCreate; }
};

//-----------------------------------------------------------------------------
// [SECTION] Font API (RblxFontConfig, RblxFontGlyph, RblxFontAtlasFlags, RblxFontAtlas, RblxFontGlyphRangesBuilder, RblxFont)
//-----------------------------------------------------------------------------

// A font input/source (we may rename this to RblxFontSource in the future)
struct RblxFontConfig
{
    // Data Source
    char            Name[40];               // <auto>   // Name (strictly to ease debugging, hence limited size buffer)
    void*           FontData;               //          // TTF/OTF data
    int             FontDataSize;           //          // TTF/OTF data size
    bool            FontDataOwnedByAtlas;   // true     // TTF/OTF data ownership taken by the owner RblxFontAtlas (will delete memory itself). SINCE 1.92, THE DATA NEEDS TO PERSIST FOR WHOLE DURATION OF ATLAS.

    // Options
    bool            MergeMode;              // false    // Merge into previous RblxFont, so you can combine multiple inputs font into one RblxFont (e.g. ASCII font + icons + Japanese glyphs). You may want to use GlyphOffset.y when merge font of different heights.
    bool            PixelSnapH;             // false    // Align every glyph AdvanceX to pixel boundaries. Prevents fractional font size from working correctly! Useful e.g. if you are merging a non-pixel aligned font with the default font. If enabled, OversampleH/V will default to 1.
    RblxS8            OversampleH;            // 0 (2)    // Rasterize at higher quality for sub-pixel positioning. 0 == auto == 1 or 2 depending on size. Note the difference between 2 and 3 is minimal. You can reduce this to 1 for large glyphs save memory. Read https://github.com/nothings/rblxstb/blob/master/tests/oversample/README.md for details.
    RblxS8            OversampleV;            // 0 (1)    // Rasterize at higher quality for sub-pixel positioning. 0 == auto == 1. This is not really useful as we don't use sub-pixel positions on the Y axis.
    RblxWchar         EllipsisChar;           // 0        // Explicitly specify Unicode codepoint of ellipsis character. When fonts are being merged first specified ellipsis will be used.
    float           SizePixels;             //          // Output size in pixels for rasterizer (more or less maps to the resulting font height).
    const RblxWchar*  GlyphRanges;            // NULL     // *LEGACY* THE ARRAY DATA NEEDS TO PERSIST AS LONG AS THE FONT IS ALIVE. Pointer to a user-provided list of Unicode range (2 value per range, values are inclusive, zero-terminated list).
    const RblxWchar*  GlyphExcludeRanges;     // NULL     // Pointer to a small user-provided list of Unicode ranges (2 value per range, values are inclusive, zero-terminated list). This is very close to GlyphRanges[] but designed to exclude ranges from a font source, when merging fonts with overlapping glyphs. Use "Input Glyphs Overlap Detection Tool" to find about your overlapping ranges.
    //RblxVec2        GlyphExtraSpacing;      // 0, 0     // (REMOVED AT IT SEEMS LARGELY OBSOLETE. PLEASE REPORT IF YOU WERE USING THIS). Extra spacing (in pixels) between glyphs when rendered: essentially add to glyph->AdvanceX. Only X axis is supported for now.
    RblxVec2          GlyphOffset;            // 0, 0     // Offset (in pixels) all glyphs from this font input. Absolute value for default size, other sizes will scale this value.
    float           GlyphMinAdvanceX;       // 0        // Minimum AdvanceX for glyphs, set Min to align font icons, set both Min/Max to enforce mono-space font. Absolute value for default size, other sizes will scale this value.
    float           GlyphMaxAdvanceX;       // FLT_MAX  // Maximum AdvanceX for glyphs
    float           GlyphExtraAdvanceX;     // 0        // Extra spacing (in pixels) between glyphs. Please contact us if you are using this. // FIXME-NEWATLAS: Intentionally unscaled
    RblxU32           FontNo;                 // 0        // Index of font within TTF/OTF file
    unsigned int    FontLoaderFlags;        // 0        // Settings for custom font builder. THIS IS BUILDER IMPLEMENTATION DEPENDENT. Leave as zero if unsure.
    //unsigned int  FontBuilderFlags;       // --       // [Renamed in 1.92] Use FontLoaderFlags.
    float           RasterizerMultiply;     // 1.0f     // Linearly brighten (>1.0f) or darken (<1.0f) font output. Brightening small fonts may be a good workaround to make them more readable. This is a silly thing we may remove in the future.
    float           RasterizerDensity;      // 1.0f     // [LEGACY: this only makes sense when RblxUIBackendFlags_RendererHasTextures is not supported] DPI scale multiplier for rasterization. Not altering other font metrics: makes it easy to swap between e.g. a 100% and a 400% fonts for a zooming display, or handle Retina screen. IMPORTANT: If you change this it is expected that you increase/decrease font scale roughly to the inverse of this, otherwise quality may look lowered.
    float           ExtraSizeScale;         // 1.0f     // Extra rasterizer scale over SizePixels.

    // [Internal]
    RblxFontFlags     Flags;                  // Font flags (don't use just yet, will be exposed in upcoming 1.92.X updates)
    RblxFont*         DstFont;                // Target font (as we merging fonts, multiple RblxFontConfig may target the same font)
    const RblxFontLoader* FontLoader;         // Custom font backend for this source (default source is the one stored in RblxFontAtlas)
    void*           FontLoaderData;         // Font loader opaque storage (per font config)

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    bool            PixelSnapV;             // true    // [Obsoleted in 1.91.6] Align Scaled GlyphOffset.y to pixel boundaries.
#endif
    RBLXUI_API RblxFontConfig();
};

// Hold rendering data for one glyph.
// (Note: some language parsers may fail to convert the bitfield members, in this case maybe drop store a single u32 or we can rework this)
struct RblxFontGlyph
{
    unsigned int    Colored : 1;        // Flag to indicate glyph is colored and should generally ignore tinting (make it usable with no shift on little-endian as this is used in loops)
    unsigned int    Visible : 1;        // Flag to indicate glyph has no visible pixels (e.g. space). Allow early out when rendering.
    unsigned int    SourceIdx : 4;      // Index of source in parent font
    unsigned int    Codepoint : 26;     // 0x0000..0x10FFFF
    float           AdvanceX;           // Horizontal distance to advance cursor/layout position.
    float           X0, Y0, X1, Y1;     // Glyph corners. Offsets from current cursor/layout position.
    float           U0, V0, U1, V1;     // Texture coordinates for the current value of RblxFontAtlas->TexRef. Cached equivalent of calling GetCustomRect() with PackId.
    int             PackId;             // [Internal] RblxFontAtlasRectId value (FIXME: Cold data, could be moved elsewhere?)

    RblxFontGlyph()   { memset((void*)this, 0, sizeof(*this)); PackId = -1; }
};

// Helper to build glyph ranges from text/string data. Feed your application strings/characters to it then call BuildRanges().
// This is essentially a tightly packed of vector of 64k booleans = 8KB storage.
struct RblxFontGlyphRangesBuilder
{
    RblxVector<RblxU32> UsedChars;            // Store 1-bit per Unicode code point (0=unused, 1=used)

    RblxFontGlyphRangesBuilder()              { Clear(); }
    inline void     Clear()                 { int size_in_bytes = (RBLXUI_UNICODE_CODEPOINT_MAX + 1) / 8; UsedChars.resize(size_in_bytes / (int)sizeof(RblxU32)); memset(UsedChars.Data, 0, (size_t)size_in_bytes); }
    inline bool     GetBit(size_t n) const  { int off = (int)(n >> 5); RblxU32 mask = 1u << (n & 31); return (UsedChars[off] & mask) != 0; }  // Get bit n in the array
    inline void     SetBit(size_t n)        { int off = (int)(n >> 5); RblxU32 mask = 1u << (n & 31); UsedChars[off] |= mask; }               // Set bit n in the array
    inline void     AddChar(RblxWchar c)      { SetBit(c); }                      // Add character
    RBLXUI_API void  AddText(const char* text, const char* text_end = NULL);     // Add string (each character of the UTF-8 string are added)
    RBLXUI_API void  AddRanges(const RblxWchar* ranges);                           // Add ranges, e.g. builder.AddRanges(RblxFontAtlas::GetGlyphRangesDefault()) to force add all of ASCII/Latin+Ext
    RBLXUI_API void  BuildRanges(RblxVector<RblxWchar>* out_ranges);                 // Output new ranges
};

// An opaque identifier to a rectangle in the atlas. -1 when invalid.
// The rectangle may move and UV may be invalidated, use GetCustomRect() to retrieve it.
typedef int RblxFontAtlasRectId;
#define RblxFontAtlasRectId_Invalid -1

// Output of RblxFontAtlas::GetCustomRect() when using custom rectangles.
// Those values may not be cached/stored as they are only valid for the current value of atlas->TexRef
// (this is in theory derived from RblxTextureRect but we use separate structures for reasons)
struct RblxFontAtlasRect
{
    unsigned short  x, y;               // Position (in current texture)
    unsigned short  w, h;               // Size
    RblxVec2          uv0, uv1;           // UV coordinates (in current texture)

    RblxFontAtlasRect() { memset((void*)this, 0, sizeof(*this)); }
};

// Flags for RblxFontAtlas build
enum RblxFontAtlasFlags_
{
    RblxFontAtlasFlags_None               = 0,
    RblxFontAtlasFlags_NoPowerOfTwoHeight = 1 << 0,   // Don't round the height to next power of two
    RblxFontAtlasFlags_NoMouseCursors     = 1 << 1,   // Don't build software mouse cursors into the atlas (save a little texture memory)
    RblxFontAtlasFlags_NoBakedLines       = 1 << 2,   // Don't build thick line textures into the atlas (save a little texture memory, allow support for point/nearest filtering). The AntiAliasedLinesUseTex features uses them, otherwise they will be rendered using polygons (more expensive for CPU/GPU).
};

// Load and rasterize multiple TTF/OTF fonts into a same texture. The font atlas will build a single texture holding:
//  - One or more fonts.
//  - Custom graphics data needed to render the shapes needed by Dear RblxUI.
//  - Mouse cursor shapes for software cursor rendering (unless setting 'Flags |= RblxFontAtlasFlags_NoMouseCursors' in the font atlas).
//  - If you don't call any AddFont*** functions, the default font embedded in the code will be loaded for you.
// It is the rendering backend responsibility to upload texture into your graphics API:
//  - RblxUI_ImplXXXX_RenderDrawData() functions generally iterate platform_io->Textures[] to create/update/destroy each RblxTextureData instance.
//  - Backend then set RblxTextureData's TexID and BackendUserData.
//  - Texture id are passed back to you during rendering to identify the texture. Read FAQ entry about RblxTextureID/RblxTextureRef for more details.
// Legacy path:
//  - Call Build() + GetTexDataAsAlpha8() or GetTexDataAsRGBA32() to build and retrieve pixels data.
//  - Call SetTexID(my_tex_id); and pass the pointer/identifier to your texture in a format natural to your graphics API.
// Common pitfalls:
// - If you pass a 'glyph_ranges' array to AddFont*** functions, you need to make sure that your array persists up until the
//   atlas is build (when calling GetTexData*** or Build()). We only copy the pointer, not the data.
// - Important: By default, AddFontFromMemoryTTF() takes ownership of the data. Even though we are not writing to it, we will free the pointer on destruction.
//   You can set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data and it won't be freed,
// - Even though many functions are suffixed with "TTF", OTF data is supported just as well.
// - This is an old API and it is currently awkward for those and various other reasons! We will address them in the future!
struct RblxFontAtlas
{
    RBLXUI_API RblxFontAtlas();
    RBLXUI_API ~RblxFontAtlas();
    RBLXUI_API RblxFont*           AddFont(const RblxFontConfig* font_cfg);
    RBLXUI_API RblxFont*           AddFontDefault(const RblxFontConfig* font_cfg = NULL);        // Selects between AddFontDefaultVector() and AddFontDefaultBitmap().
    RBLXUI_API RblxFont*           AddFontDefaultVector(const RblxFontConfig* font_cfg = NULL);  // Embedded scalable font. Recommended at any higher size.
    RBLXUI_API RblxFont*           AddFontDefaultBitmap(const RblxFontConfig* font_cfg = NULL);  // Embedded classic pixel-clean font. Recommended at Size 13px with no scaling.
    RBLXUI_API RblxFont*           AddFontFromFileTTF(const char* filename, float size_pixels = 0.0f, const RblxFontConfig* font_cfg = NULL, const RblxWchar* glyph_ranges = NULL);
    RBLXUI_API RblxFont*           AddFontFromMemoryTTF(void* font_data, int font_data_size, float size_pixels = 0.0f, const RblxFontConfig* font_cfg = NULL, const RblxWchar* glyph_ranges = NULL); // Note: Transfer ownership of 'ttf_data' to RblxFontAtlas! Will be deleted after destruction of the atlas. Set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data and it won't be freed.
    RBLXUI_API RblxFont*           AddFontFromMemoryCompressedTTF(const void* compressed_font_data, int compressed_font_data_size, float size_pixels = 0.0f, const RblxFontConfig* font_cfg = NULL, const RblxWchar* glyph_ranges = NULL); // 'compressed_font_data' still owned by caller. Compress with binary_to_compressed_c.cpp.
    RBLXUI_API RblxFont*           AddFontFromMemoryCompressedBase85TTF(const char* compressed_font_data_base85, float size_pixels = 0.0f, const RblxFontConfig* font_cfg = NULL, const RblxWchar* glyph_ranges = NULL);              // 'compressed_font_data_base85' still owned by caller. Compress with binary_to_compressed_c.cpp with -base85 parameter.
    RBLXUI_API void              RemoveFont(RblxFont* font);                       // Remove a font
    RBLXUI_API void              CompactCache();                                 // Compact cached glyphs and texture.
    RBLXUI_API void              SetFontLoader(const RblxFontLoader* font_loader); // Change font loader at runtime.

    // Clearing the atlas/fonts has little use nowadays, unless you want to batch remove all fonts. 
    // - Since 1.92, you can call ClearFonts() mid-frame, if you load new fonts afterwards. 
    // - As we are transitioning toward our new font system the semantic for those functions gets increasingly misleading and are often a source of issues.
    //   TL;DR; most likely, don't use any of those functions. We expect to obsolete/rework them.
    RBLXUI_API void              Clear();                    // Clear everything (fonts + textures). Don't call mid-frame!
    RBLXUI_API void              ClearFonts();               // Clear input+output font data/glyphs. New fonts and textures will be recreated afterwards.
    RBLXUI_API void              ClearInputData();           // [OBSOLETE] Clear input data (all RblxFontConfig structures including sizes, TTF data, glyph ranges, etc.) = all the data used to build the texture and fonts.
    RBLXUI_API void              ClearTexData();             // [OBSOLETE] Clear CPU-side copy of the texture data. Saves RAM once the texture has been copied to graphics memory.

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    // Legacy path for build atlas + retrieving pixel data.
    // - User is in charge of copying the pixels into graphics memory (e.g. create a texture with your engine). Then store your texture handle with SetTexID().
    // - The pitch is always = Width * BytesPerPixels (1 or 4)
    // - Building in RGBA32 format is provided for convenience and compatibility, but note that unless you manually manipulate or copy color data into
    //   the texture (e.g. when using the AddCustomRect*** api), then the RGB pixels emitted will always be white (~75% of memory/bandwidth waste).
    // - From 1.92 with backends supporting RblxUIBackendFlags_RendererHasTextures:
    //   - Calling Build(), GetTexDataAsAlpha8(), GetTexDataAsRGBA32() is not needed.
    //   - In backend: replace calls to RblxFontAtlas::SetTexID() with calls to RblxTextureData::SetTexID() after honoring texture creation.
    RBLXUI_API bool  Build();                    // Build pixels data. This is called automatically for you by the GetTexData*** functions.
    RBLXUI_API void  GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL); // 1 byte per-pixel
    RBLXUI_API void  GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL); // 4 bytes-per-pixel
    void            SetTexID(RblxTextureID id)    { RBLXUI_ASSERT(TexRef._TexID == RblxTextureID_Invalid); TexRef._TexData->TexID = id; }                               // Called by legacy backends. May be called before texture creation.
    void            SetTexID(RblxTextureRef id)   { RBLXUI_ASSERT(TexRef._TexID == RblxTextureID_Invalid && id._TexData == NULL); TexRef._TexData->TexID = id._TexID; } // Called by legacy backends.
    bool            IsBuilt() const { return Fonts.Size > 0 && TexIsBuilt; } // Bit ambiguous: used to detect when user didn't build texture but effectively we should check TexID != 0 except that would be backend dependent..
#endif

    //-------------------------------------------
    // Glyph Ranges
    //-------------------------------------------

    // Since 1.92: specifying glyph ranges is only useful/necessary if your backend doesn't support RblxUIBackendFlags_RendererHasTextures!
    RBLXUI_API const RblxWchar*    GetGlyphRangesDefault();                // Basic Latin, Extended Latin
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    // Helpers to retrieve list of common Unicode ranges (2 value per range, values are inclusive, zero-terminated list)
    // NB: Make sure that your string are UTF-8 and NOT in your local code page.
    // Read https://github.com/ocornut/rblxui/blob/master/docs/FONTS.md/#about-utf-8-encoding for details.
    // NB: Consider using RblxFontGlyphRangesBuilder to build glyph ranges from textual data.
    RBLXUI_API const RblxWchar*    GetGlyphRangesGreek();                  // Default + Greek and Coptic
    RBLXUI_API const RblxWchar*    GetGlyphRangesKorean();                 // Default + Korean characters
    RBLXUI_API const RblxWchar*    GetGlyphRangesJapanese();               // Default + Hiragana, Katakana, Half-Width, Selection of 2999 Ideographs
    RBLXUI_API const RblxWchar*    GetGlyphRangesChineseFull();            // Default + Half-Width + Japanese Hiragana/Katakana + full set of about 21000 CJK Unified Ideographs
    RBLXUI_API const RblxWchar*    GetGlyphRangesChineseSimplifiedCommon();// Default + Half-Width + Japanese Hiragana/Katakana + set of 2500 CJK Unified Ideographs for common simplified Chinese
    RBLXUI_API const RblxWchar*    GetGlyphRangesCyrillic();               // Default + about 400 Cyrillic characters
    RBLXUI_API const RblxWchar*    GetGlyphRangesThai();                   // Default + Thai characters
    RBLXUI_API const RblxWchar*    GetGlyphRangesVietnamese();             // Default + Vietnamese characters
#endif

    //-------------------------------------------
    // [ALPHA] Custom Rectangles/Glyphs API
    //-------------------------------------------

    // Register and retrieve custom rectangles
    // - You can request arbitrary rectangles to be packed into the atlas, for your own purpose.
    // - Since 1.92.0, packing is done immediately in the function call (previously packing was done during the Build call)
    // - You can render your pixels into the texture right after calling the AddCustomRect() functions.
    // - VERY IMPORTANT:
    //   - Texture may be created/resized at any time when calling RblxUI or RblxFontAtlas functions.
    //   - IT WILL INVALIDATE RECTANGLE DATA SUCH AS UV COORDINATES. Always use latest values from GetCustomRect().
    //   - UV coordinates are associated to the current texture identifier aka 'atlas->TexRef'. Both TexRef and UV coordinates are typically changed at the same time.
    // - If you render colored output into your custom rectangles: set 'atlas->TexPixelsUseColors = true' as this may help some backends decide of preferred texture format.
    // - Read docs/FONTS.md for more details about using colorful icons.
    // - Note: this API may be reworked further in order to facilitate supporting e.g. multi-monitor, varying DPI settings.
    // - (Pre-1.92 names) ------------> (1.92 names)
    //   - GetCustomRectByIndex()   --> Use GetCustomRect()
    //   - CalcCustomRectUV()       --> Use GetCustomRect() and read uv0, uv1 fields.
    //   - AddCustomRectRegular()   --> Renamed to AddCustomRect()
    //   - AddCustomRectFontGlyph() --> Prefer using custom RblxFontLoader inside RblxFontConfig
    //   - RblxFontAtlasCustomRect    --> Renamed to RblxFontAtlasRect
    RBLXUI_API RblxFontAtlasRectId AddCustomRect(int width, int height, RblxFontAtlasRect* out_r = NULL);// Register a rectangle. Return -1 (RblxFontAtlasRectId_Invalid) on error.
    RBLXUI_API void              RemoveCustomRect(RblxFontAtlasRectId id);                             // Unregister a rectangle. Existing pixels will stay in texture until resized / garbage collected.
    RBLXUI_API bool              GetCustomRect(RblxFontAtlasRectId id, RblxFontAtlasRect* out_r) const;  // Get rectangle coordinates for current texture. Valid immediately, never store this (read above)!

    //-------------------------------------------
    // Members
    //-------------------------------------------

    // Input
    RblxFontAtlasFlags            Flags;              // Build flags (see RblxFontAtlasFlags_)
    RblxTextureFormat             TexDesiredFormat;   // Desired texture format (default to RblxTextureFormat_RGBA32 but may be changed to RblxTextureFormat_Alpha8).
    int                         TexGlyphPadding;    // FIXME: Should be called "TexPackPadding". Padding between glyphs within texture in pixels. Defaults to 1. If your rendering method doesn't rely on bilinear filtering you may set this to 0 (will also need to set AntiAliasedLinesUseTex = false).
    int                         TexMinWidth;        // Minimum desired texture width. Must be a power of two. Default to 512.
    int                         TexMinHeight;       // Minimum desired texture height. Must be a power of two. Default to 128.
    int                         TexMaxWidth;        // Maximum desired texture width. Must be a power of two. Default to 8192.
    int                         TexMaxHeight;       // Maximum desired texture height. Must be a power of two. Default to 8192.
    void*                       UserData;           // Store your own atlas related user-data (if e.g. you have multiple font atlas).

    // Output
    // - Because textures are dynamically created/resized, the current texture identifier may changed at *ANY TIME* during the frame.
    // - This should not affect you as you can always use the latest value. But note that any precomputed UV coordinates are only valid for the current TexRef.
#ifdef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    RblxTextureRef                TexRef;             // Latest texture identifier == TexData->GetTexRef().
#else
    union { RblxTextureRef TexRef; RblxTextureRef TexID; }; // Latest texture identifier == TexData->GetTexRef(). // RENAMED TexID to TexRef in 1.92.0.
#endif
    RblxTextureData*              TexData;            // Latest texture.

    // [Internal]
    RblxVector<RblxTextureData*>    TexList;            // Texture list (most often TexList.Size == 1). TexData is always == TexList.back(). DO NOT USE DIRECTLY, USE GetDrawData().Textures[]/GetPlatformIO().Textures[] instead!
    bool                        Locked;             // Marked as locked during RblxUI::NewFrame()..EndFrame() scope if TexUpdates are not supported. Any attempt to modify the atlas will assert.
    bool                        RendererHasTextures;// Copy of (BackendFlags & RblxUIBackendFlags_RendererHasTextures) from supporting context.
    bool                        TexIsBuilt;         // Set when texture was built matching current font input. Mostly useful for legacy IsBuilt() call.
    bool                        TexPixelsUseColors; // Tell whether our texture data is known to use colors (rather than just alpha channel), in order to help backend select a format or conversion process.
    RblxVec2                      TexUvScale;         // = (1.0f/TexData->TexWidth, 1.0f/TexData->TexHeight). May change as new texture gets created.
    RblxVec2                      TexUvWhitePixel;    // Texture coordinates to a white pixel. May change as new texture gets created.
    RblxVector<RblxFont*>           Fonts;              // Hold all the fonts returned by AddFont*. Fonts[0] is the default font upon calling RblxUI::NewFrame(), use RblxUI::PushFont()/PopFont() to change the current font.
    RblxVector<RblxFontConfig>      Sources;            // Source/configuration data
    RblxVec4                      TexUvLines[RBLXUI_DRAWLIST_TEX_LINES_WIDTH_MAX + 1];  // UVs for baked anti-aliased lines
    int                         TexNextUniqueID;    // Next value to be stored in TexData->UniqueID
    int                         FontNextUniqueID;   // Next value to be stored in RblxFont->FontID
    RblxVector<RblxDrawListSharedData*> DrawListSharedDatas; // List of users for this atlas. Typically one per Dear RblxUI context.
    RblxFontAtlasBuilder*         Builder;            // Opaque interface to our data that doesn't need to be public and may be discarded when rebuilding.
    const RblxFontLoader*         FontLoader;         // Font loader opaque interface (default to use FreeType when RBLXUI_ENABLE_FREETYPE is defined, otherwise default to use rblxstb_truetype). Use SetFontLoader() to change this at runtime.
    const char*                 FontLoaderName;     // Font loader name (for display e.g. in About box) == FontLoader->Name
    void*                       FontLoaderData;     // Font backend opaque storage
    unsigned int                FontLoaderFlags;    // Shared flags (for all fonts) for font loader. THIS IS BUILD IMPLEMENTATION DEPENDENT (e.g. Per-font override is also available in RblxFontConfig).
    int                         RefCount;           // Number of contexts using this atlas
    RblxUIContext*               OwnerContext;       // Context which own the atlas will be in charge of updating and destroying it.

    // [Obsolete]
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    // Legacy: You can request your rectangles to be mapped as font glyph (given a font + Unicode point), so you can render e.g. custom colorful icons and use them as regular glyphs. --> Prefer using a custom RblxFontLoader.
    RblxFontAtlasRect             TempRect;           // For old GetCustomRectByIndex() API
    inline RblxFontAtlasRectId    AddCustomRectRegular(int w, int h)                                                          { return AddCustomRect(w, h); }                             // RENAMED in 1.92.0
    inline const RblxFontAtlasRect* GetCustomRectByIndex(RblxFontAtlasRectId id)                                                { return GetCustomRect(id, &TempRect) ? &TempRect : NULL; } // OBSOLETED in 1.92.0
    inline void                 CalcCustomRectUV(const RblxFontAtlasRect* r, RblxVec2* out_uv_min, RblxVec2* out_uv_max) const    { *out_uv_min = r->uv0; *out_uv_max = r->uv1; }             // OBSOLETED in 1.92.0
    RBLXUI_API RblxFontAtlasRectId AddCustomRectFontGlyph(RblxFont* font, RblxWchar codepoint, int w, int h, float advance_x, const RblxVec2& offset = RblxVec2(0, 0));                            // OBSOLETED in 1.92.0: Use custom RblxFontLoader in RblxFontConfig
    RBLXUI_API RblxFontAtlasRectId AddCustomRectFontGlyphForSize(RblxFont* font, float font_size, RblxWchar codepoint, int w, int h, float advance_x, const RblxVec2& offset = RblxVec2(0, 0));    // ADDED AND OBSOLETED in 1.92.0
#endif
    //unsigned int                      FontBuilderFlags;        // OBSOLETED in 1.92.0: Renamed to FontLoaderFlags.
    //int                               TexDesiredWidth;         // OBSOLETED in 1.92.0: Force texture width before calling Build(). Must be a power-of-two. If have many glyphs your graphics API have texture size restrictions you may want to increase texture width to decrease height.
    //typedef RblxFontAtlasRect           RblxFontAtlasCustomRect;   // OBSOLETED in 1.92.0
    //typedef RblxFontAtlasCustomRect     CustomRect;              // OBSOLETED in 1.72+
    //typedef RblxFontGlyphRangesBuilder  GlyphRangesBuilder;      // OBSOLETED in 1.67+
};

// Font runtime data for a given size
// Important: pointers to RblxFontBaked are only valid for the current frame.
struct RblxFontBaked
{
    // [Internal] Members: Hot ~20/24 bytes (for CalcTextSize)
    RblxVector<float>             IndexAdvanceX;      // 12-16 // out // Sparse. Glyphs->AdvanceX in a directly indexable way (cache-friendly for CalcTextSize functions which only this info, and are often bottleneck in large UI).
    float                       FallbackAdvanceX;   // 4     // out // FindGlyph(FallbackChar)->AdvanceX
    float                       Size;               // 4     // in  // Height of characters/line, set during loading (doesn't change after loading)
    float                       RasterizerDensity;  // 4     // in  // Density this is baked at

    // [Internal] Members: Hot ~28/36 bytes (for RenderText loop)
    RblxVector<RblxU16>             IndexLookup;        // 12-16 // out // Sparse. Index glyphs by Unicode code-point.
    RblxVector<RblxFontGlyph>       Glyphs;             // 12-16 // out // All glyphs.
    int                         FallbackGlyphIndex; // 4     // out // Index of FontFallbackChar

    // [Internal] Members: Cold
    float                       Ascent, Descent;    // 4+4   // out // Ascent: distance from top to bottom of e.g. 'A' [0..FontSize] (unscaled)
    unsigned int                MetricsTotalSurface:26;// 3  // out // Total surface in pixels to get an idea of the font rasterization/texture cost (not exact, we approximate the cost of padding between glyphs)
    unsigned int                WantDestroy:1;         // 0  //     // Queued for destroy
    unsigned int                LoadNoFallback:1;      // 0  //     // Disable loading fallback in lower-level calls.
    unsigned int                LoadNoRenderOnLayout:1;// 0  //     // Enable a two-steps mode where CalcTextSize() calls will load AdvanceX *without* rendering/packing glyphs. Only advantageous if you know that the glyph is unlikely to actually be rendered, otherwise it is slower because we'd do one query on the first CalcTextSize and one query on the first Draw.
    int                         LastUsedFrame;         // 4  //     // Record of that time this was bounds
    RblxUIID                     BakedId;            // 4     //     // Unique ID for this baked storage
    RblxFont*                     OwnerFont;          // 4-8   // in  // Parent font
    void*                       FontLoaderDatas;    // 4-8   //     // Font loader opaque storage (per baked font * sources): single contiguous buffer allocated by rblxui, passed to loader.

    // Functions
    RBLXUI_API RblxFontBaked();
    RBLXUI_API void              ClearOutputData();
    RBLXUI_API RblxFontGlyph*      FindGlyph(RblxWchar c);               // Return U+FFFD glyph if requested glyph doesn't exists.
    RBLXUI_API RblxFontGlyph*      FindGlyphNoFallback(RblxWchar c);     // Return NULL if glyph doesn't exist
    RBLXUI_API float             GetCharAdvance(RblxWchar c);
    RBLXUI_API bool              IsGlyphLoaded(RblxWchar c);
};

// Font flags
// (in future versions as we redesign font loading API, this will become more important and better documented. for now please consider this as internal/advanced use)
enum RblxFontFlags_
{
    RblxFontFlags_None                    = 0,
    RblxFontFlags_NoLoadError             = 1 << 1,   // Disable throwing an error/assert when calling AddFontXXX() with missing file/data. Calling code is expected to check AddFontXXX() return value.
    RblxFontFlags_NoLoadGlyphs            = 1 << 2,   // [Internal] Disable loading new glyphs.
    RblxFontFlags_LockBakedSizes          = 1 << 3,   // [Internal] Disable loading new baked sizes, disable garbage collecting current ones. e.g. if you want to lock a font to a single size. Important: if you use this to preload given sizes, consider the possibility of multiple font density used on Retina display.
    RblxFontFlags_ImplicitRefSize         = 1 << 4,   // [Internal] Reference size was not set explicitly.
};

// Font runtime data and rendering
// - RblxFontAtlas automatically loads a default embedded font for you if you didn't load one manually.
// - Since 1.92.0 a font may be rendered as any size! Therefore a font doesn't have one specific size.
// - Use 'font->GetFontBaked(size)' to retrieve the RblxFontBaked* corresponding to a given size.
// - If you used g.Font + g.FontSize (which is frequent from the RblxUI layer), you can use g.FontBaked as a shortcut, as g.FontBaked == g.Font->GetFontBaked(g.FontSize).
struct RblxFont
{
    // [Internal] Members: Hot ~12-20 bytes
    RblxFontBaked*                LastBaked;          // 4-8   // Cache last bound baked. NEVER USE DIRECTLY. Use GetFontBaked().
    RblxFontAtlas*                OwnerAtlas;         // 4-8   // What we have been loaded into.
    RblxFontFlags                 Flags;              // 4     // Font flags.
    float                       CurrentRasterizerDensity;    // Current rasterizer density. This is a varying state of the font.

    // [Internal] Members: Cold ~24-52 bytes
    // Conceptually Sources[] is the list of font sources merged to create this font.
    RblxUIID                     FontId;             // Unique identifier for the font
    float                       LegacySize;         // 4     // in  // Font size passed to AddFont(). Use for old code calling PushFont() expecting to use that size. (use RblxUI::GetFontBaked() to get font baked at current bound size).
    RblxVector<RblxFontConfig*>     Sources;            // 16    // in  // List of sources. Pointers within OwnerAtlas->Sources[]
    RblxWchar                     EllipsisChar;       // 2-4   // out // Character used for ellipsis rendering ('...'). If you ever want to temporarily swap this for an alternative/dummy char, make sure to clear EllipsisAutoBake.
    RblxWchar                     FallbackChar;       // 2-4   // out // Character used if a glyph isn't found (U+FFFD, '?')
    RblxU8                        Used8kPagesMap[(RBLXUI_UNICODE_CODEPOINT_MAX+1)/8192/8]; // 1 bytes if RblxWchar=RblxWchar16, 17 bytes if RblxWchar==RblxWchar32. Store 1-bit for each block of 8K codepoints that has one active glyph. This is mainly used to facilitate iterations across all used codepoints.
    bool                        EllipsisAutoBake;   // 1     //     // Mark when the "..." glyph (== EllipsisChar) needs to be generated by combining multiple '.'.
    RblxUIStorage                RemapPairs;         // 16    //     // Remapping pairs when using AddRemapChar(), otherwise empty.
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    float                       Scale;              // 4     // in  // Legacy base font scale (~1.0f), multiplied by the per-window font scale which you can adjust with SetWindowFontScale()
#endif

    // Methods
    RBLXUI_API RblxFont();
    RBLXUI_API ~RblxFont();
    RBLXUI_API bool              IsGlyphInFont(RblxWchar c);
    bool                        IsLoaded() const                { return OwnerAtlas != NULL; }
    const char*                 GetDebugName() const            { return Sources.Size ? Sources[0]->Name : "<unknown>"; } // Fill RblxFontConfig::Name.

    // [Internal] Don't use!
    // 'max_width' stops rendering after a certain width (could be turned into a 2d size). FLT_MAX to disable.
    // 'wrap_width' enable automatic word-wrapping across multiple lines to fit into given width. 0.0f to disable.
    RBLXUI_API RblxFontBaked*      GetFontBaked(float font_size, float density = -1.0f);  // Get or create baked data for given size
    RBLXUI_API RblxVec2            CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end = NULL, const char** out_remaining = NULL);
    RBLXUI_API const char*       CalcWordWrapPosition(float size, const char* text, const char* text_end, float wrap_width);
    RBLXUI_API void              RenderChar(RblxDrawList* draw_list, float size, const RblxVec2& pos, RblxU32 col, RblxWchar c, const RblxVec4* cpu_fine_clip = NULL);
    RBLXUI_API void              RenderText(RblxDrawList* draw_list, float size, const RblxVec2& pos, RblxU32 col, const RblxVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width = 0.0f, RblxDrawTextFlags flags = 0);
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    inline const char*          CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) { return CalcWordWrapPosition(LegacySize * scale, text, text_end, wrap_width); } // Obsoleted old name in 1.92.0. Note how `scale` was to `size`.
#endif

    // [Internal] Don't use!
    RBLXUI_API void              ClearOutputData();
    RBLXUI_API void              AddRemapChar(RblxWchar from_codepoint, RblxWchar to_codepoint); // Makes 'from_codepoint' character points to 'to_codepoint' glyph.
    RBLXUI_API bool              IsGlyphRangeUnused(unsigned int c_begin, unsigned int c_last);
};

// This is provided for consistency (but we don't actually use this)
inline RblxTextureID RblxTextureRef::GetTexID() const
{
    RBLXUI_ASSERT(!(_TexData != NULL && _TexID != RblxTextureID_Invalid));
    return _TexData ? _TexData->TexID : _TexID;
}

// Using an indirection to avoid patching RblxDrawCmd after a SetTexID() call (but this could be an alternative solution too)
inline RblxTextureID RblxDrawCmd::GetTexID() const
{
    // If you are getting this assert with RblxTextureID_Invalid == 0 and your RblxTextureID is used to store an index or an offset:
    // - You can add '#define RblxTextureID_Invalid ((RblxTextureID)-1)' in your rblxuiconfig.h file.
    // If you are getting this assert with a renderer backend with support for RblxUIBackendFlags_RendererHasTextures (1.92+):
    // - You must correctly iterate and handle RblxTextureData requests stored in RblxDrawData::Textures[]. See docs/BACKENDS.md.
    RblxTextureID tex_id = TexRef._TexData ? TexRef._TexData->TexID : TexRef._TexID; // == TexRef.GetTexID() above.
    if (TexRef._TexData != NULL)
        RBLXUI_ASSERT(tex_id != RblxTextureID_Invalid && "RblxDrawCmd is referring to RblxTextureData that wasn't uploaded to graphics system. Backend must call RblxTextureData::SetTexID() after handling RblxTextureStatus_WantCreate request!");
    return tex_id;
}

//-----------------------------------------------------------------------------
// [SECTION] Viewports
//-----------------------------------------------------------------------------

// Flags stored in RblxUIViewport::Flags, giving indications to the platform backends.
enum RblxUIViewportFlags_
{
    RblxUIViewportFlags_None                     = 0,
    RblxUIViewportFlags_IsPlatformWindow         = 1 << 0,   // Represent a Platform Window
    RblxUIViewportFlags_IsPlatformMonitor        = 1 << 1,   // Represent a Platform Monitor (unused yet)
    RblxUIViewportFlags_OwnedByApp               = 1 << 2,   // Platform Window: Is created/managed by the user application? (rather than our backend)
    RblxUIViewportFlags_NoDecoration             = 1 << 3,   // Platform Window: Disable platform decorations: title bar, borders, etc. (generally set all windows, but if RblxUIConfigFlags_ViewportsDecoration is set we only set this on popups/tooltips)
    RblxUIViewportFlags_NoTaskBarIcon            = 1 << 4,   // Platform Window: Disable platform task bar icon (generally set on popups/tooltips, or all windows if RblxUIConfigFlags_ViewportsNoTaskBarIcon is set)
    RblxUIViewportFlags_NoFocusOnAppearing       = 1 << 5,   // Platform Window: Don't take focus when created.
    RblxUIViewportFlags_NoFocusOnClick           = 1 << 6,   // Platform Window: Don't take focus when clicked on.
    RblxUIViewportFlags_NoInputs                 = 1 << 7,   // Platform Window: Make mouse pass through so we can drag this window while peaking behind it.
    RblxUIViewportFlags_NoRendererClear          = 1 << 8,   // Platform Window: Renderer doesn't need to clear the framebuffer ahead (because we will fill it entirely).
    RblxUIViewportFlags_NoAutoMerge              = 1 << 9,   // Platform Window: Avoid merging this window into another host window. This can only be set via RblxUIWindowClass viewport flags override (because we need to now ahead if we are going to create a viewport in the first place!).
    RblxUIViewportFlags_TopMost                  = 1 << 10,  // Platform Window: Display on top (for tooltips only).
    RblxUIViewportFlags_CanHostOtherWindows      = 1 << 11,  // Viewport can host multiple rblxui windows (secondary viewports are associated to a single window). // FIXME: In practice there's still probably code making the assumption that this is always and only on the MainViewport. Will fix once we add support for "no main viewport".

    // Output status flags (from Platform)
    RblxUIViewportFlags_IsMinimized              = 1 << 12,  // Platform Window: Window is minimized, can skip render. When minimized we tend to avoid using the viewport pos/size for clipping window or testing if they are contained in the viewport.
    RblxUIViewportFlags_IsFocused                = 1 << 13,  // Platform Window: Window is focused (last call to Platform_GetWindowFocus() returned true)
};

// - Currently represents the Platform Window created by the application which is hosting our Dear RblxUI windows.
// - With multi-viewport enabled, we extend this concept to have multiple active viewports.
// - In the future we will extend this concept further to also represent Platform Monitor and support a "no main platform window" operation mode.
// - About Main Area vs Work Area:
//   - Main Area = entire viewport.
//   - Work Area = entire viewport minus sections used by main menu bars (for platform windows), or by task bar (for platform monitor).
//   - Windows are generally trying to stay within the Work Area of their host viewport.
struct RblxUIViewport
{
    RblxUIID             ID;                     // Unique identifier for the viewport
    RblxUIViewportFlags  Flags;                  // See RblxUIViewportFlags_
    RblxVec2              Pos;                    // Main Area: Position of the viewport (Dear RblxUI coordinates are the same as OS desktop/native coordinates)
    RblxVec2              Size;                   // Main Area: Size of the viewport.
    RblxVec2              FramebufferScale;       // Density of the viewport for Retina display (always 1,1 on Windows, may be 2,2 etc on macOS/iOS). This will affect font rasterizer density.
    RblxVec2              WorkPos;                // Work Area: Position of the viewport minus task bars, menus bars, status bars (>= Pos)
    RblxVec2              WorkSize;               // Work Area: Size of the viewport minus task bars, menu bars, status bars (<= Size)
    float               DpiScale;               // 1.0f = 96 DPI = No extra scale.
    RblxUIID             ParentViewportId;       // (Advanced) 0: no parent. Instruct the platform backend to setup a parent/child relationship between platform windows.
    RblxUIViewport*      ParentViewport;         // (Advanced) Direct shortcut to RblxUI::FindViewportByID(ParentViewportId). NULL: no parent.
    RblxDrawData*         DrawData;               // The RblxDrawData corresponding to this viewport. Valid after Render() and until the next call to NewFrame().

    // Platform/Backend Dependent Data
    // Our design separate the Renderer and Platform backends to facilitate combining default backends with each others.
    // When our create your own backend for a custom engine, it is possible that both Renderer and Platform will be handled
    // by the same system and you may not need to use all the UserData/Handle fields.
    // The library never uses those fields, they are merely storage to facilitate backend implementation.
    void*               RendererUserData;       // void* to hold custom data structure for the renderer (e.g. swap chain, framebuffers etc.). generally set by your Renderer_CreateWindow function.
    void*               PlatformUserData;       // void* to hold custom data structure for the OS / platform (e.g. windowing info, render context). generally set by your Platform_CreateWindow function.
    void*               PlatformIconData;       // void* to hold custom data structure for the OS / platform to specify an icon. Currently unused for exposed to allow experiments.
    void*               PlatformHandle;         // void* to hold higher-level, platform window handle (e.g. HWND for Win32 backend, Uint32 WindowID for SDL, RBLXWINWindow* for RBLXWIN), for FindViewportByPlatformHandle().
    void*               PlatformHandleRaw;      // void* to hold lower-level, platform-native window handle (always HWND on Win32 platform, unused for other platforms).
    bool                PlatformWindowCreated;  // Platform window has been created (Platform_CreateWindow() has been called). This is false during the first frame where a viewport is being created.
    bool                PlatformRequestMove;    // Platform window requested move (e.g. window was moved by the OS / host window manager, authoritative position will be OS window position)
    bool                PlatformRequestResize;  // Platform window requested resize (e.g. window was resized by the OS / host window manager, authoritative size will be OS window size)
    bool                PlatformRequestClose;   // Platform window requested closure (e.g. window was moved by the OS / host window manager, e.g. pressing ALT-F4)

    RblxUIViewport()     { memset((void*)this, 0, sizeof(*this)); }
    ~RblxUIViewport()    { RBLXUI_ASSERT(PlatformUserData == NULL && RendererUserData == NULL); }

    // Helpers
    RblxVec2              GetCenter() const       { return RblxVec2(Pos.x + Size.x * 0.5f, Pos.y + Size.y * 0.5f); }
    RblxVec2              GetWorkCenter() const   { return RblxVec2(WorkPos.x + WorkSize.x * 0.5f, WorkPos.y + WorkSize.y * 0.5f); }
    RBLXUI_API const char* GetDebugName() const;
};

//-----------------------------------------------------------------------------
// [SECTION] RblxUIPlatformIO + other Platform Dependent Interfaces (RblxUIPlatformMonitor, RblxUIPlatformImeData)
//-----------------------------------------------------------------------------

// [BETA] (Optional) Multi-Viewport Support!
// If you are new to Dear RblxUI and trying to integrate it into your engine, you can probably ignore this for now.
//
// This feature allows you to seamlessly drag Dear RblxUI windows outside of your application viewport.
// This is achieved by creating new Platform/OS windows on the fly, and rendering into them.
// Dear RblxUI manages the viewport structures, and the backend create and maintain one Platform/OS window for each of those viewports.
//
// See Recap:   https://github.com/ocornut/rblxui/wiki/Multi-Viewports
// See Glossary https://github.com/ocornut/rblxui/wiki/Glossary for details about some of the terminology.
//
// About the coordinates system:
// - When multi-viewports are enabled, all Dear RblxUI coordinates become absolute coordinates (same as OS coordinates!)
// - So e.g. RblxUI::SetNextWindowPos(RblxVec2(0,0)) will position a window relative to your primary monitor!
// - If you want to position windows relative to your main application viewport, use RblxUI::GetMainViewport()->Pos as a base position.
//
// Steps to use multi-viewports in your application, when using a default backend from the examples/ folder:
// - Application:  Enable feature with 'io.ConfigFlags |= RblxUIConfigFlags_ViewportsEnable'.
// - Backend:      The backend initialization will setup all necessary RblxUIPlatformIO's functions and update monitors info every frame.
// - Application:  In your main loop, call RblxUI::UpdatePlatformWindows(), RblxUI::RenderPlatformWindowsDefault() after EndFrame() or Render().
// - Application:  Fix absolute coordinates used in RblxUI::SetWindowPos() or RblxUI::SetNextWindowPos() calls.
//
// Steps to use multi-viewports in your application, when using a custom backend:
// - Important:    THIS IS NOT EASY TO DO and comes with many subtleties not described here!
//                 It's also an experimental feature, so some of the requirements may evolve.
//                 Consider using default backends if you can. Either way, carefully follow and refer to examples/ backends for details.
// - Application:  Enable feature with 'io.ConfigFlags |= RblxUIConfigFlags_ViewportsEnable'.
// - Backend:      Hook RblxUIPlatformIO's Platform_* and Renderer_* callbacks (see below).
//                 Set 'io.BackendFlags |= RblxUIBackendFlags_PlatformHasViewports' and 'io.BackendFlags |= RblxUIBackendFlags_PlatformHasViewports'.
//                 Update RblxUIPlatformIO's Monitors list every frame.
//                 Update MousePos every frame, in absolute coordinates.
// - Application:  In your main loop, call RblxUI::UpdatePlatformWindows(), RblxUI::RenderPlatformWindowsDefault() after EndFrame() or Render().
//                 You may skip calling RenderPlatformWindowsDefault() if its API is not convenient for your needs. Read comments below.
// - Application:  Fix absolute coordinates used in RblxUI::SetWindowPos() or RblxUI::SetNextWindowPos() calls.
//
// About RblxUI::RenderPlatformWindowsDefault():
// - This function is a mostly a _helper_ for the common-most cases, and to facilitate using default backends.
// - You can check its simple source code to understand what it does.
//   It basically iterates secondary viewports and call 4 functions that are setup in RblxUIPlatformIO, if available:
//     Platform_RenderWindow(), Renderer_RenderWindow(), Platform_SwapBuffers(), Renderer_SwapBuffers()
//   Those functions pointers exists only for the benefit of RenderPlatformWindowsDefault().
// - If you have very specific rendering needs (e.g. flipping multiple swap-chain simultaneously, unusual sync/threading issues, etc.),
//   you may be tempted to ignore RenderPlatformWindowsDefault() and write customized code to perform your renderingg.
//   You may decide to setup the platform_io's *RenderWindow and *SwapBuffers pointers and call your functions through those pointers,
//   or you may decide to never setup those pointers and call your code directly. They are a convenience, not an obligatory interface.
//-----------------------------------------------------------------------------

// Access via RblxUI::GetPlatformIO()
struct RblxUIPlatformIO
{
    RBLXUI_API RblxUIPlatformIO();

    //------------------------------------------------------------------
    // Input - Interface with OS and Platform backend (most common stuff)
    //------------------------------------------------------------------

    // Optional: Access OS clipboard
    // (default to use native Win32 clipboard on Windows, otherwise uses a private clipboard. Override to access OS clipboard on other architectures)
    const char* (*Platform_GetClipboardTextFn)(RblxUIContext* ctx);                      // Should return NULL on failure (e.g. clipboard data is not text).
    void        (*Platform_SetClipboardTextFn)(RblxUIContext* ctx, const char* text);
    void*       Platform_ClipboardUserData;

    // Optional: Open link/folder/file in OS Shell
    // (default to use ShellExecuteW() on Windows, system() on Linux/Mac. expected to return false on failure, but some platforms may always return true)
    bool        (*Platform_OpenInShellFn)(RblxUIContext* ctx, const char* path);
    void*       Platform_OpenInShellUserData;

    // Optional: Notify OS Input Method Editor of the screen position of your cursor for text input position (e.g. when using Japanese/Chinese IME on Windows)
    // (default to use native imm32 api on Windows)
    void        (*Platform_SetImeDataFn)(RblxUIContext* ctx, RblxUIViewport* viewport, RblxUIPlatformImeData* data);
    void*       Platform_ImeUserData;
    //void      (*SetPlatformImeDataFn)(RblxUIViewport* viewport, RblxUIPlatformImeData* data); // [Renamed to platform_io.PlatformSetImeDataFn in 1.91.1]

    // Optional: Platform locale
    // [Experimental] Configure decimal point e.g. '.' or ',' useful for some languages (e.g. German), generally pulled from *localeconv()->decimal_point
    RblxWchar     Platform_LocaleDecimalPoint;     // '.'

    //------------------------------------------------------------------
    // Input - Interface with Renderer Backend
    //------------------------------------------------------------------

    // Optional: Maximum texture size supported by renderer (used to adjust how we size textures). 0 if not known.
    int         Renderer_TextureMaxWidth;
    int         Renderer_TextureMaxHeight;

    // Written by some backends during RblxUI_ImplXXXX_RenderDrawData() call to point backend_specific RblxUI_ImplXXXX_RenderState* structure.
    void*       Renderer_RenderState;

    // Standard draw callbacks provided by renderer backend.
    RblxDrawCallback  DrawCallback_ResetRenderState;      // Request to reset the graphics/render state.
    RblxDrawCallback  DrawCallback_SetSamplerLinear;      // Request backend to set texture sampling to Linear.
    RblxDrawCallback  DrawCallback_SetSamplerNearest;     // Request backend to set texture sampling to Nearest/Point.
    //RblxDrawCallback  DrawCallback_SetSamplerCustom;    // Request backend to set texture sampling using Backend Specific data.

    //------------------------------------------------------------------
    // Input - Interface with Platform & Renderer backends for Multi-Viewport support
    //------------------------------------------------------------------

    // For reference, the second column shows which function are generally calling the Platform Functions:
    //   N = RblxUI::NewFrame()                        ~ beginning of the dear rblxui frame: read info from platform/OS windows (latest size/position)
    //   F = RblxUI::Begin(), RblxUI::EndFrame()        ~ during the dear rblxui frame
    //   U = RblxUI::UpdatePlatformWindows()           ~ after the dear rblxui frame: create and update all platform/OS windows
    //   R = RblxUI::RenderPlatformWindowsDefault()    ~ render
    //   D = RblxUI::DestroyPlatformWindows()          ~ shutdown
    // The general idea is that NewFrame() we will read the current Platform/OS state, and UpdatePlatformWindows() will write to it.

    // The handlers are designed so we can mix and match two rblxui_impl_xxxx files, one Platform backend and one Renderer backend.
    // Custom engine backends will often provide both Platform and Renderer interfaces together and so may not need to use all functions.
    // Platform functions are typically called _before_ their Renderer counterpart, apart from Destroy which are called the other way.

    // Platform Backend functions (e.g. Win32, RBLXWIN, SDL) ------------------- Called by -----
    void    (*Platform_CreateWindow)(RblxUIViewport* vp);                    // . . U . .  // Create a new platform window for the given viewport
    void    (*Platform_DestroyWindow)(RblxUIViewport* vp);                   // N . U . D  //
    void    (*Platform_ShowWindow)(RblxUIViewport* vp);                      // . . U . .  // Newly created windows are initially hidden so SetWindowPos/Size/Title can be called on them before showing the window
    void    (*Platform_SetWindowPos)(RblxUIViewport* vp, RblxVec2 pos);        // . . U . .  // Set platform window position (given the upper-left corner of client area)
    RblxVec2  (*Platform_GetWindowPos)(RblxUIViewport* vp);                    // N . . . .  //
    void    (*Platform_SetWindowSize)(RblxUIViewport* vp, RblxVec2 size);      // . . U . .  // Set platform window client area size (ignoring OS decorations such as OS title bar etc.)
    RblxVec2  (*Platform_GetWindowSize)(RblxUIViewport* vp);                   // N . . . .  // Get platform window client area size
    RblxVec2  (*Platform_GetWindowFramebufferScale)(RblxUIViewport* vp);       // N . . . .  // Return viewport density. Always 1,1 on Windows, often 2,2 on Retina display on macOS/iOS. MUST BE INTEGER VALUES.
    void    (*Platform_SetWindowFocus)(RblxUIViewport* vp);                  // N . . . .  // Move window to front and set input focus
    bool    (*Platform_GetWindowFocus)(RblxUIViewport* vp);                  // . . U . .  //
    bool    (*Platform_GetWindowMinimized)(RblxUIViewport* vp);              // N . . . .  // Get platform window minimized state. When minimized, we generally won't attempt to get/set size and contents will be culled more easily
    void    (*Platform_SetWindowTitle)(RblxUIViewport* vp, const char* str); // . . U . .  // Set platform window title (given an UTF-8 string)
    void    (*Platform_SetWindowAlpha)(RblxUIViewport* vp, float alpha);     // . . U . .  // (Optional) Setup global transparency (not per-pixel transparency)
    void    (*Platform_UpdateWindow)(RblxUIViewport* vp);                    // . . U . .  // (Optional) Called by UpdatePlatformWindows(). Optional hook to allow the platform backend from doing general book-keeping every frame.
    void    (*Platform_RenderWindow)(RblxUIViewport* vp, void* render_arg);  // . . . R .  // (Optional) Main rendering (platform side! This is often unused, or just setting a "current" context for OpenGL bindings). 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    void    (*Platform_SwapBuffers)(RblxUIViewport* vp, void* render_arg);   // . . . R .  // (Optional) Call Present/SwapBuffers (platform side! This is often unused!). 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    float   (*Platform_GetWindowDpiScale)(RblxUIViewport* vp);               // N . . . .  // (Optional) [BETA] FIXME-DPI: DPI handling: Return DPI scale for this viewport. 1.0f = 96 DPI.
    void    (*Platform_OnChangedViewport)(RblxUIViewport* vp);               // . F . . .  // (Optional) [BETA] FIXME-DPI: DPI handling: Called during Begin() every time the viewport we are outputting into changes, so backend has a chance to swap fonts to adjust style.
    RblxVec4  (*Platform_GetWindowWorkAreaInsets)(RblxUIViewport* vp);         // N . . . .  // (Optional) [BETA] Get initial work area inset for the viewport (won't be covered by main menu bar, dockspace over viewport etc.). Default to (0,0),(0,0). 'safeAreaInsets' in iOS land, 'DisplayCutout' in Android land.
    int     (*Platform_CreateVkSurface)(RblxUIViewport* vp, RblxU64 vk_inst, const void* vk_allocators, RblxU64* out_vk_surface); // (Optional) For a Vulkan Renderer to call into Platform code (since the surface creation needs to tie them both).

    // Renderer Backend functions (e.g. DirectX, OpenGL, Vulkan) ------------ Called by -----
    void    (*Renderer_CreateWindow)(RblxUIViewport* vp);                    // . . U . .  // Create swap chain, frame buffers etc. (called after Platform_CreateWindow)
    void    (*Renderer_DestroyWindow)(RblxUIViewport* vp);                   // N . U . D  // Destroy swap chain, frame buffers etc. (called before Platform_DestroyWindow)
    void    (*Renderer_SetWindowSize)(RblxUIViewport* vp, RblxVec2 size);      // . . U . .  // Resize swap chain, frame buffers etc. (called after Platform_SetWindowSize)
    void    (*Renderer_RenderWindow)(RblxUIViewport* vp, void* render_arg);  // . . . R .  // (Optional) Clear framebuffer, setup render target, then render the viewport->DrawData. 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    void    (*Renderer_SwapBuffers)(RblxUIViewport* vp, void* render_arg);   // . . . R .  // (Optional) Call Present/SwapBuffers. 'render_arg' is the value passed to RenderPlatformWindowsDefault().

    // (Optional) Monitor list
    // - Updated by: app/backend. Update every frame to dynamically support changing monitor or DPI configuration.
    // - Used by: dear rblxui to query DPI info, clamp popups/tooltips within same monitor and not have them straddle monitors.
    RblxVector<RblxUIPlatformMonitor>  Monitors;

    //------------------------------------------------------------------
    // Output
    //------------------------------------------------------------------

    // Textures list (the list is updated by calling RblxUI::EndFrame or RblxUI::Render)
    // The RblxUI_ImplXXXX_RenderDrawData() function of each backend generally access this via RblxDrawData::Textures which points to this. The array is available here mostly because backends will want to destroy textures on shutdown.
    RblxVector<RblxTextureData*>        Textures;           // List of textures used by Dear RblxUI (most often 1) + contents of external texture list is automatically appended into this.

    // Viewports list (the list is updated by calling RblxUI::EndFrame or RblxUI::Render)
    // (in the future we will attempt to organize this feature to remove the need for a "main viewport")
    RblxVector<RblxUIViewport*>        Viewports;          // Main viewports, followed by all secondary viewports.

    //------------------------------------------------------------------
    // Functions
    //------------------------------------------------------------------

    RBLXUI_API void ClearPlatformHandlers();    // Clear all Platform_XXX fields. Typically called on Platform Backend shutdown.
    RBLXUI_API void ClearRendererHandlers();    // Clear all Renderer_XXX fields. Typically called on Renderer Backend shutdown.
};

// (Optional) This is required when enabling multi-viewport. Represent the bounds of each connected monitor/display and their DPI.
// We use this information for multiple DPI support + clamping the position of popups and tooltips so they don't straddle multiple monitors.
struct RblxUIPlatformMonitor
{
    RblxVec2  MainPos, MainSize;      // Coordinates of the area displayed on this monitor (Min = upper left, Max = bottom right)
    RblxVec2  WorkPos, WorkSize;      // Coordinates without task bars / side bars / menu bars. Used to avoid positioning popups/tooltips inside this region. If you don't have this info, please copy the value for MainPos/MainSize.
    float   DpiScale;               // 1.0f = 96 DPI
    void*   PlatformHandle;         // Backend dependant data (e.g. HMONITOR, RBLXWINmonitor*, SDL Display Index, NSScreen*)
    RblxUIPlatformMonitor()          { MainPos = MainSize = WorkPos = WorkSize = RblxVec2(0, 0); DpiScale = 1.0f; PlatformHandle = NULL; }
};

// (Optional) Support for IME (Input Method Editor) via the platform_io.Platform_SetImeDataFn() function. Handler is called during EndFrame().
struct RblxUIPlatformImeData
{
    bool    WantVisible;            // A widget wants the IME to be visible.
    bool    WantTextInput;          // A widget wants text input, not necessarily IME to be visible. This is automatically set to the upcoming value of io.WantTextInput.
    RblxVec2  InputPos;               // Position of input cursor (for IME).
    float   InputLineHeight;        // Line height (for IME).
    RblxUIID ViewportId;             // ID of platform window/viewport.

    RblxUIPlatformImeData()          { memset((void*)this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Obsolete functions and types
// (Will be removed! Read 'API BREAKING CHANGES' section in rblxui.cpp for details)
// Please keep your copy of dear rblxui up to date! Occasionally set '#define RBLXUI_DISABLE_OBSOLETE_FUNCTIONS' in rblxuiconfig.h to stay ahead.
//-----------------------------------------------------------------------------

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
namespace RblxUI
{
    // OBSOLETED in 1.92.0 (from June 2025)
    inline void         PushFont(RblxFont* font)                                  { PushFont(font, font ? font->LegacySize : 0.0f); }
    RBLXUI_API void      SetWindowFontScale(float scale);                        // Set font scale factor for current window. Prefer using PushFont(NULL, style.FontSizeBase * factor) or use style.FontScaleMain to scale all windows.
    // OBSOLETED in 1.91.9 (from February 2025)
    RBLXUI_API void      Image(RblxTextureRef tex_ref, const RblxVec2& image_size, const RblxVec2& uv0, const RblxVec2& uv1, const RblxVec4& tint_col, const RblxVec4& border_col); // <-- 'border_col' was removed in favor of RblxUICol_ImageBorder. If you use 'tint_col', use ImageWithBg() instead.
    // OBSOLETED in 1.91.0 (from July 2024)
    inline void         PushButtonRepeat(bool repeat)                           { PushItemFlag(RblxUIItemFlags_ButtonRepeat, repeat); }
    inline void         PopButtonRepeat()                                       { PopItemFlag(); }
    inline void         PushTabStop(bool tab_stop)                              { PushItemFlag(RblxUIItemFlags_NoTabStop, !tab_stop); }
    inline void         PopTabStop()                                            { PopItemFlag(); }
    // You do not need those functions! See #7838 on GitHub for more info.
    RBLXUI_API RblxVec2    GetContentRegionMax();                                  // Content boundaries max (e.g. window boundaries including scrolling, or current column boundaries). You should never need this. Always use GetCursorScreenPos() and GetContentRegionAvail()!
    RBLXUI_API RblxVec2    GetWindowContentRegionMin();                            // Content boundaries min for the window (roughly (0,0)-Scroll), in window-local coordinates. You should never need this. Always use GetCursorScreenPos() and GetContentRegionAvail()!
    RBLXUI_API RblxVec2    GetWindowContentRegionMax();                            // Content boundaries max for the window (roughly (0,0)+Size-Scroll), in window-local coordinates. You should never need this. Always use GetCursorScreenPos() and GetContentRegionAvail()!

    // Some of the older obsolete names along with their replacement (commented out so they are not reported in IDE)
    // OBSOLETED in 1.90.0 (from September 2023)
    //RBLXUI_API bool      Combo(const char* label, int* current_item, bool (*old_callback)(void* user_data, int idx, const char** out_text), void* user_data, int items_count, int popup_max_height_in_items = -1); // Getter signature changed. See 2023/09/15 and 2026/02/27 commits.
    //RBLXUI_API bool      ListBox(const char* label, int* current_item, bool (*old_callback)(void* user_data, int idx, const char** out_text), void* user_data, int items_count, int height_in_items = -1);         // Getter signature changed. See 2023/09/15 and 2026/02/27 commits.
    //inline bool         BeginChild(const char* str_id, const RblxVec2& size_arg, bool borders, RblxUIWindowFlags window_flags) { return BeginChild(str_id, size_arg, borders ? RblxUIChildFlags_Borders : RblxUIChildFlags_None, window_flags); } // Unnecessary as true == RblxUIChildFlags_Borders
    //inline bool         BeginChild(RblxUIID id, const RblxVec2& size_arg, bool borders, RblxUIWindowFlags window_flags)         { return BeginChild(id, size_arg, borders ? RblxUIChildFlags_Borders : RblxUIChildFlags_None, window_flags);     } // Unnecessary as true == RblxUIChildFlags_Borders
    //inline bool         BeginChildFrame(RblxUIID id, const RblxVec2& size, RblxUIWindowFlags flags = 0) { return BeginChild(id, size, RblxUIChildFlags_FrameStyle, flags); }
    //inline void         EndChildFrame()                                                             { EndChild(); }
    //inline void         ShowStackToolWindow(bool* p_open = NULL)                                    { ShowIDStackToolWindow(p_open); }
    // OBSOLETED in 1.89.7 (from June 2023)
    //RBLXUI_API void      SetItemAllowOverlap();                                                      // Use SetNextItemAllowOverlap() _before_ item.
    //-- OBSOLETED in 1.89.4 (from March 2023)
    //static inline void  PushAllowKeyboardFocus(bool tab_stop)                                       { PushItemFlag(RblxUIItemFlags_NoTabStop, !tab_stop); }
    //static inline void  PopAllowKeyboardFocus()                                                     { PopItemFlag(); }
    //-- OBSOLETED in 1.89 (from August 2022)
    //RBLXUI_API bool      ImageButton(RblxTextureID user_texture_id, const RblxVec2& size, const RblxVec2& uv0 = RblxVec2(0, 0), const RblxVec2& uv1 = RblxVec2(1, 1), int frame_padding = -1, const RblxVec4& bg_col = RblxVec4(0, 0, 0, 0), const RblxVec4& tint_col = RblxVec4(1, 1, 1, 1)); // --> Use new ImageButton() signature (explicit item id, regular FramePadding). Refer to code in 1.91 if you want to grab a copy of this version.
    //-- OBSOLETED in 1.88 (from May 2022)
    //static inline void  CaptureKeyboardFromApp(bool want_capture_keyboard = true)                   { SetNextFrameWantCaptureKeyboard(want_capture_keyboard); } // Renamed as name was misleading + removed default value.
    //static inline void  CaptureMouseFromApp(bool want_capture_mouse = true)                         { SetNextFrameWantCaptureMouse(want_capture_mouse); }       // Renamed as name was misleading + removed default value.
    //-- OBSOLETED in 1.87 (from February 2022, more formally obsoleted April 2024)
    //RBLXUI_API RblxUIKey  GetKeyIndex(RblxUIKey key);                                                  { RBLXUI_ASSERT(key >= RblxUIKey_NamedKey_BEGIN && key < RblxUIKey_NamedKey_END); const RblxUIKeyData* key_data = GetKeyData(key); return (RblxUIKey)(key_data - g.IO.KeysData); } // Map RblxUIKey_* values into legacy native key index. == io.KeyMap[key]. When using a 1.87+ backend using io.AddKeyEvent(), calling GetKeyIndex() with ANY RblxUIKey_XXXX values will return the same value!
    //static inline RblxUIKey GetKeyIndex(RblxUIKey key)                                                { RBLXUI_ASSERT(key >= RblxUIKey_NamedKey_BEGIN && key < RblxUIKey_NamedKey_END); return key; }
    //-- OBSOLETED in 1.86 (from November 2021)
    //RBLXUI_API void      CalcListClipping(int items_count, float items_height, int* out_items_display_start, int* out_items_display_end); // Code removed, see 1.90 for last version of the code. Calculate range of visible items for large list of evenly sized items. Prefer using RblxUIListClipper.
    //-- OBSOLETED in 1.85 (from August 2021)
    //static inline float GetWindowContentRegionWidth()                                               { return GetWindowContentRegionMax().x - GetWindowContentRegionMin().x; }
    //-- OBSOLETED in 1.81 (from February 2021)
    //static inline bool  ListBoxHeader(const char* label, const RblxVec2& size = RblxVec2(0, 0))         { return BeginListBox(label, size); }
    //static inline bool  ListBoxHeader(const char* label, int items_count, int height_in_items = -1) { float height = GetTextLineHeightWithSpacing() * ((height_in_items < 0 ? RblxMin(items_count, 7) : height_in_items) + 0.25f) + GetStyle().FramePadding.y * 2.0f; return BeginListBox(label, RblxVec2(0.0f, height)); } // Helper to calculate size from items_count and height_in_items
    //static inline void  ListBoxFooter()                                                             { EndListBox(); }
    //-- OBSOLETED in 1.79 (from August 2020)
    //static inline void  OpenPopupContextItem(const char* str_id = NULL, RblxUIMouseButton mb = 1)    { OpenPopupOnItemClick(str_id, mb); } // Bool return value removed. Use IsWindowAppearing() in BeginPopup() instead. Renamed in 1.77, renamed back in 1.79. Sorry!
    //-- OBSOLETED in 1.78 (from June 2020): Old drag/sliders functions that took a 'float power > 1.0f' argument instead of RblxUISliderFlags_Logarithmic. See github.com/ocornut/rblxui/issues/3361 for details.
    //RBLXUI_API bool      DragScalar(const char* label, RblxUIDataType data_type, void* p_data, float v_speed, const void* p_min, const void* p_max, const char* format, float power = 1.0f)                                                            // OBSOLETED in 1.78 (from June 2020)
    //RBLXUI_API bool      DragScalarN(const char* label, RblxUIDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, float power = 1.0f);                                          // OBSOLETED in 1.78 (from June 2020)
    //RBLXUI_API bool      SliderScalar(const char* label, RblxUIDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, float power = 1.0f);                                                                        // OBSOLETED in 1.78 (from June 2020)
    //RBLXUI_API bool      SliderScalarN(const char* label, RblxUIDataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format, float power = 1.0f);                                                       // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  DragFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, float power = 1.0f)    { return DragScalar(label, RblxUIDataType_Float, v, v_speed, &v_min, &v_max, format, power); }     // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  DragFloat2(const char* label, float v[2], float v_speed, float v_min, float v_max, const char* format, float power = 1.0f) { return DragScalarN(label, RblxUIDataType_Float, v, 2, v_speed, &v_min, &v_max, format, power); } // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  DragFloat3(const char* label, float v[3], float v_speed, float v_min, float v_max, const char* format, float power = 1.0f) { return DragScalarN(label, RblxUIDataType_Float, v, 3, v_speed, &v_min, &v_max, format, power); } // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  DragFloat4(const char* label, float v[4], float v_speed, float v_min, float v_max, const char* format, float power = 1.0f) { return DragScalarN(label, RblxUIDataType_Float, v, 4, v_speed, &v_min, &v_max, format, power); } // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, float power = 1.0f)                 { return SliderScalar(label, RblxUIDataType_Float, v, &v_min, &v_max, format, power); }            // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format, float power = 1.0f)              { return SliderScalarN(label, RblxUIDataType_Float, v, 2, &v_min, &v_max, format, power); }        // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format, float power = 1.0f)              { return SliderScalarN(label, RblxUIDataType_Float, v, 3, &v_min, &v_max, format, power); }        // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format, float power = 1.0f)              { return SliderScalarN(label, RblxUIDataType_Float, v, 4, &v_min, &v_max, format, power); }        // OBSOLETED in 1.78 (from June 2020)
    //-- OBSOLETED in 1.77 and before
    //static inline bool  BeginPopupContextWindow(const char* str_id, RblxUIMouseButton mb, bool over_items) { return BeginPopupContextWindow(str_id, mb | (over_items ? 0 : RblxUIPopupFlags_NoOpenOverItems)); } // OBSOLETED in 1.77 (from June 2020)
    //static inline void  TreeAdvanceToLabelPos()               { SetCursorPosX(GetCursorPosX() + GetTreeNodeToLabelSpacing()); }   // OBSOLETED in 1.72 (from July 2019)
    //static inline void  SetNextTreeNodeOpen(bool open, RblxUICond cond = 0) { SetNextItemOpen(open, cond); }                       // OBSOLETED in 1.71 (from June 2019)
    //static inline float GetContentRegionAvailWidth()          { return GetContentRegionAvail().x; }                               // OBSOLETED in 1.70 (from May 2019)
    //static inline RblxDrawList* GetOverlayDrawList()            { return GetForegroundDrawList(); }                                 // OBSOLETED in 1.69 (from Mar 2019)
    //static inline void  SetScrollHere(float ratio = 0.5f)     { SetScrollHereY(ratio); }                                          // OBSOLETED in 1.66 (from Nov 2018)
    //static inline bool  IsItemDeactivatedAfterChange()        { return IsItemDeactivatedAfterEdit(); }                            // OBSOLETED in 1.63 (from Aug 2018)
    //-- OBSOLETED in 1.60 and before
    //static inline bool  IsAnyWindowFocused()                  { return IsWindowFocused(RblxUIFocusedFlags_AnyWindow); }            // OBSOLETED in 1.60 (from Apr 2018)
    //static inline bool  IsAnyWindowHovered()                  { return IsWindowHovered(RblxUIHoveredFlags_AnyWindow); }            // OBSOLETED in 1.60 (between Dec 2017 and Apr 2018)
    //static inline void  ShowTestWindow()                      { return ShowDemoWindow(); }                                        // OBSOLETED in 1.53 (between Oct 2017 and Dec 2017)
    //static inline bool  IsRootWindowFocused()                 { return IsWindowFocused(RblxUIFocusedFlags_RootWindow); }           // OBSOLETED in 1.53 (between Oct 2017 and Dec 2017)
    //static inline bool  IsRootWindowOrAnyChildFocused()       { return IsWindowFocused(RblxUIFocusedFlags_RootAndChildWindows); }  // OBSOLETED in 1.53 (between Oct 2017 and Dec 2017)
    //static inline void  SetNextWindowContentWidth(float w)    { SetNextWindowContentSize(RblxVec2(w, 0.0f)); }                      // OBSOLETED in 1.53 (between Oct 2017 and Dec 2017)
    //static inline float GetItemsLineHeightWithSpacing()       { return GetFrameHeightWithSpacing(); }                             // OBSOLETED in 1.53 (between Oct 2017 and Dec 2017)
    //RBLXUI_API bool      Begin(char* name, bool* p_open, RblxVec2 size_first_use, float bg_alpha = -1.0f, RblxUIWindowFlags flags=0); // OBSOLETED in 1.52 (between Aug 2017 and Oct 2017): Equivalent of using SetNextWindowSize(size, RblxUICond_FirstUseEver) and SetNextWindowBgAlpha().
    //static inline bool  IsRootWindowOrAnyChildHovered()       { return IsWindowHovered(RblxUIHoveredFlags_RootAndChildWindows); }  // OBSOLETED in 1.52 (between Aug 2017 and Oct 2017)
    //static inline void  AlignFirstTextHeightToWidgets()       { AlignTextToFramePadding(); }                                      // OBSOLETED in 1.52 (between Aug 2017 and Oct 2017)
    //static inline void  SetNextWindowPosCenter(RblxUICond c=0) { SetNextWindowPos(GetMainViewport()->GetCenter(), c, RblxVec2(0.5f,0.5f)); } // OBSOLETED in 1.52 (between Aug 2017 and Oct 2017)
    //static inline bool  IsItemHoveredRect()                   { return IsItemHovered(RblxUIHoveredFlags_RectOnly); }               // OBSOLETED in 1.51 (between Jun 2017 and Aug 2017)
    //static inline bool  IsPosHoveringAnyWindow(const RblxVec2&) { RBLXUI_ASSERT(0); return false; }                                     // OBSOLETED in 1.51 (between Jun 2017 and Aug 2017): This was misleading and partly broken. You probably want to use the io.WantCaptureMouse flag instead.
    //static inline bool  IsMouseHoveringAnyWindow()            { return IsWindowHovered(RblxUIHoveredFlags_AnyWindow); }            // OBSOLETED in 1.51 (between Jun 2017 and Aug 2017)
    //static inline bool  IsMouseHoveringWindow()               { return IsWindowHovered(RblxUIHoveredFlags_AllowWhenBlockedByPopup | RblxUIHoveredFlags_AllowWhenBlockedByActiveItem); }       // OBSOLETED in 1.51 (between Jun 2017 and Aug 2017)
    //-- OBSOLETED in 1.50 and before
    //static inline bool  CollapsingHeader(char* label, const char* str_id, bool framed = true, bool default_open = false) { return CollapsingHeader(label, (default_open ? (1 << 5) : 0)); } // OBSOLETED in 1.49
    //static inline RblxFont*GetWindowFont()                      { return GetFont(); }                                               // OBSOLETED in 1.48
    //static inline float GetWindowFontSize()                   { return GetFontSize(); }                                           // OBSOLETED in 1.48
    //static inline void  SetScrollPosHere()                    { SetScrollHere(); }                                                // OBSOLETED in 1.42
}

#define RblxDrawCallback_ResetRenderState     (RblxDrawCallback)(-8)    // OBSOLETED in 1.92.8: Use RblxUI::GetPlatformIO().DrawCallback_ResetRenderState

//-- OBSOLETED in 1.92.0: RblxFontAtlasCustomRect becomes RblxTextureRect
// - RblxFontAtlasCustomRect::X,Y          --> RblxTextureRect::x,y
// - RblxFontAtlasCustomRect::Width,Height --> RblxTextureRect::w,h
// - RblxFontAtlasCustomRect::GlyphColored --> if you need to write to this, instead you can write to 'font->Glyphs.back()->Colored' after calling AddCustomRectFontGlyph()
// We could make RblxTextureRect an union to use old names, but 1) this would be confusing 2) the fix is easy 3) RblxFontAtlasCustomRect was always a rather esoteric api.
typedef RblxFontAtlasRect RblxFontAtlasCustomRect;
/*struct RblxFontAtlasCustomRect
{
    unsigned short  X, Y;           // Output   // Packed position in Atlas
    unsigned short  Width, Height;  // Input    // [Internal] Desired rectangle dimension
    unsigned int    GlyphID:31;     // Input    // [Internal] For custom font glyphs only (ID < 0x110000)
    unsigned int    GlyphColored:1; // Input    // [Internal] For custom font glyphs only: glyph is colored, removed tinting.
    float           GlyphAdvanceX;  // Input    // [Internal] For custom font glyphs only: glyph xadvance
    RblxVec2          GlyphOffset;    // Input    // [Internal] For custom font glyphs only: glyph display offset
    RblxFont*         Font;           // Input    // [Internal] For custom font glyphs only: target font
    RblxFontAtlasCustomRect()         { X = Y = 0xFFFF; Width = Height = 0; GlyphID = 0; GlyphColored = 0; GlyphAdvanceX = 0.0f; GlyphOffset = RblxVec2(0, 0); Font = NULL; }
    bool IsPacked() const           { return X != 0xFFFF; }
};*/

//-- OBSOLETED in 1.82 (from Mars 2021): flags for AddRect(), AddRectFilled(), AddImageRounded(), PathRect()
//typedef RblxDrawFlags RblxDrawCornerFlags;
//enum RblxDrawCornerFlags_
//{
//    RblxDrawCornerFlags_None      = RblxDrawFlags_RoundCornersNone,         // Was == 0 prior to 1.82, this is now == RblxDrawFlags_RoundCornersNone which is != 0 and not implicit
//    RblxDrawCornerFlags_TopLeft   = RblxDrawFlags_RoundCornersTopLeft,      // Was == 0x01 (1 << 0) prior to 1.82. Order matches RblxDrawFlags_NoRoundCorner* flag (we exploit this internally).
//    RblxDrawCornerFlags_TopRight  = RblxDrawFlags_RoundCornersTopRight,     // Was == 0x02 (1 << 1) prior to 1.82.
//    RblxDrawCornerFlags_BotLeft   = RblxDrawFlags_RoundCornersBottomLeft,   // Was == 0x04 (1 << 2) prior to 1.82.
//    RblxDrawCornerFlags_BotRight  = RblxDrawFlags_RoundCornersBottomRight,  // Was == 0x08 (1 << 3) prior to 1.82.
//    RblxDrawCornerFlags_All       = RblxDrawFlags_RoundCornersAll,          // Was == 0x0F prior to 1.82
//    RblxDrawCornerFlags_Top       = RblxDrawCornerFlags_TopLeft | RblxDrawCornerFlags_TopRight,
//    RblxDrawCornerFlags_Bot       = RblxDrawCornerFlags_BotLeft | RblxDrawCornerFlags_BotRight,
//    RblxDrawCornerFlags_Left      = RblxDrawCornerFlags_TopLeft | RblxDrawCornerFlags_BotLeft,
//    RblxDrawCornerFlags_Right     = RblxDrawCornerFlags_TopRight | RblxDrawCornerFlags_BotRight,
//};

// RENAMED and MERGED both RblxUIKey_ModXXX and RblxUIModFlags_XXX into RblxUIMod_XXX (from September 2022)
// RENAMED RblxUIKeyModFlags -> RblxUIModFlags in 1.88 (from April 2022). Exceptionally commented out ahead of obsolescence schedule to reduce confusion and because they were not meant to be used in the first place.
//typedef RblxUIKeyChord RblxUIModFlags;      // == int. We generally use RblxUIKeyChord to mean "a RblxUIKey or-ed with any number of RblxUIMod_XXX value", so you may store mods in there.
//enum RblxUIModFlags_ { RblxUIModFlags_None = 0, RblxUIModFlags_Ctrl = RblxUIMod_Ctrl, RblxUIModFlags_Shift = RblxUIMod_Shift, RblxUIModFlags_Alt = RblxUIMod_Alt, RblxUIModFlags_Super = RblxUIMod_Super };
//typedef RblxUIKeyChord RblxUIKeyModFlags; // == int
//enum RblxUIKeyModFlags_ { RblxUIKeyModFlags_None = 0, RblxUIKeyModFlags_Ctrl = RblxUIMod_Ctrl, RblxUIKeyModFlags_Shift = RblxUIMod_Shift, RblxUIKeyModFlags_Alt = RblxUIMod_Alt, RblxUIKeyModFlags_Super = RblxUIMod_Super };

//#define RBLXUI_OFFSETOF(_TYPE,_MEMBER)  offsetof(_TYPE, _MEMBER)  // OBSOLETED IN 1.90 (now using C++11 standard version)

#endif // #ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS

#define RBLXUI_ARRAYSIZE                RBLXUI_COUNTOF                  // RENAMED IN 1.92.6: RBLXUI_ARRAYSIZE -> RBLXUI_COUNTOF

// RENAMED RBLXUI_DISABLE_METRICS_WINDOW > RBLXUI_DISABLE_DEBUG_TOOLS in 1.88 (from June 2022)
#ifdef RBLXUI_DISABLE_METRICS_WINDOW
#error RBLXUI_DISABLE_METRICS_WINDOW was renamed to RBLXUI_DISABLE_DEBUG_TOOLS, please use new name.
#endif

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning (pop)
#endif

// Include rblxui_user.h at the end of rblxui.h
// May be convenient for some users to only explicitly include vanilla rblxui.h and have extra stuff included.
#ifdef RBLXUI_INCLUDE_RBLXUI_USER_H
#ifdef RBLXUI_USER_H_FILENAME
#include RBLXUI_USER_H_FILENAME
#else
#include "rblxui_user.h"
#endif
#endif

#endif // #ifndef RBLXUI_DISABLE
