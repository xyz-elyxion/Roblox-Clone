// dear linearui, v1.92.9 WIP
// (headers)

// Help:
// - Call and read LinearUI::ShowDemoWindow() in linearui_demo.cpp. All applications in examples/ are doing that.
// - Read top of linearui.cpp for more details, links and comments.
// - Add '#define LINEARUI_DEFINE_MATH_OPERATORS' before including linearui.h (or in linearuiconfig.h) to access courtesy maths operators for LinearVec2 and LinearVec4.

// Resources:
// - FAQ ........................ https://dearlinearui.com/faq (in repository as docs/FAQ.md)
// - Homepage ................... https://github.com/ocornut/linearui
// - Releases & Changelog ....... https://github.com/ocornut/linearui/releases
// - Gallery .................... https://github.com/ocornut/linearui/issues?q=label%3Agallery (please post your screenshots/video there!)
// - Wiki ....................... https://github.com/ocornut/linearui/wiki (lots of good stuff there)
//   - Getting Started            https://github.com/ocornut/linearui/wiki/Getting-Started (how to integrate in an existing app by adding ~25 lines of code)
//   - Third-party Extensions     https://github.com/ocornut/linearui/wiki/Useful-Extensions (LinearPlot & many more)
//   - Bindings/Backends          https://github.com/ocornut/linearui/wiki/Bindings (language bindings + backends for various tech/engines)
//   - Debug Tools                https://github.com/ocornut/linearui/wiki/Debug-Tools
//   - Glossary                   https://github.com/ocornut/linearui/wiki/Glossary
//   - Software using Dear LinearUI  https://github.com/ocornut/linearui/wiki/Software-using-dear-linearui
// - Issues & support ........... https://github.com/ocornut/linearui/issues
// - Test Engine & Automation ... https://github.com/ocornut/linearui_test_engine (test suite, test engine to automate your apps)
// - Web version of the Demo .... https://pthom.github.io/linearui_explorer (w/ source code browser)

// For FIRST-TIME users having issues compiling/linking/running:
// please post in https://github.com/ocornut/linearui/discussions if you cannot find a solution in resources above.
// EVERYTHING ELSE should be asked in 'Issues'! We are building a database of cross-linked knowledge there.
// Since 1.92, we encourage font loading questions to also be posted in 'Issues'.

// Library Version
// (Integer encoded as XYYZZ for use in #if preprocessor conditionals, e.g. '#if LINEARUI_VERSION_NUM >= 12345')
#define LINEARUI_VERSION       "1.92.9 WIP"
#define LINEARUI_VERSION_NUM   19282
#define LINEARUI_HAS_TABLE             // Added BeginTable() - from LINEARUI_VERSION_NUM >= 18000
#define LINEARUI_HAS_TEXTURES          // Added LinearUIBackendFlags_RendererHasTextures - from LINEARUI_VERSION_NUM >= 19198
#define LINEARUI_HAS_VIEWPORT          // In 'docking' WIP branch.
#define LINEARUI_HAS_DOCK              // In 'docking' WIP branch.

/*

Index of this file:
// [SECTION] Header mess
// [SECTION] Forward declarations and basic types
// [SECTION] Texture identifiers (LinearTextureID, LinearTextureRef)
// [SECTION] Dear LinearUI end-user API functions
// [SECTION] Flags & Enumerations
// [SECTION] Tables API flags and structures (LinearUITableFlags, LinearUITableColumnFlags, LinearUITableRowFlags, LinearUITableBgTarget, LinearUITableSortSpecs, LinearUITableColumnSortSpecs)
// [SECTION] Helpers: Debug log, Memory allocations macros, LinearVector<>
// [SECTION] LinearUIStyle
// [SECTION] LinearUIIO
// [SECTION] Misc data structures (LinearUIInputTextCallbackData, LinearUISizeCallbackData, LinearUIWindowClass, LinearUIPayload)
// [SECTION] Helpers (LinearUIOnceUponAFrame, LinearUITextFilter, LinearUITextBuffer, LinearUIStorage, LinearUIListClipper, Math Operators, LinearColor)
// [SECTION] Multi-Select API flags and structures (LinearUIMultiSelectFlags, LinearUIMultiSelectIO, LinearUISelectionRequest, LinearUISelectionBasicStorage, LinearUISelectionExternalStorage)
// [SECTION] Drawing API (LinearDrawCallback, LinearDrawCmd, LinearDrawIdx, LinearDrawVert, LinearDrawChannel, LinearDrawListSplitter, LinearDrawFlags, LinearDrawListFlags, LinearDrawList, LinearDrawData)
// [SECTION] Texture API (LinearTextureFormat, LinearTextureStatus, LinearTextureRect, LinearTextureData)
// [SECTION] Font API (LinearFontConfig, LinearFontGlyph, LinearFontGlyphRangesBuilder, LinearFontAtlasFlags, LinearFontAtlas, LinearFontBaked, LinearFont)
// [SECTION] Viewports (LinearUIViewportFlags, LinearUIViewport)
// [SECTION] LinearUIPlatformIO + other Platform Dependent Interfaces (LinearUIPlatformMonitor, LinearUIPlatformImeData)
// [SECTION] Obsolete functions and types

*/

#pragma once

// Configuration file with compile-time options
// (edit linearuiconfig.h or '#define LINEARUI_USER_CONFIG "myfilename.h" from your build system)
#ifdef LINEARUI_USER_CONFIG
#include LINEARUI_USER_CONFIG
#endif
#include "linearuiconfig.h"

#ifndef LINEARUI_DISABLE

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

// Includes
#include <float.h>                  // FLT_MIN, FLT_MAX
#include <stdarg.h>                 // va_list, va_start, va_end
#include <stddef.h>                 // ptrdiff_t, NULL
#include <string.h>                 // memset, memmove, memcpy, strlen, strchr, strcpy, strcmp

// Define attributes of all API symbols declarations (e.g. for DLL under Windows)
// LINEARUI_API is used for core linearui functions, LINEARUI_IMPL_API is used for the default backends files (linearui_impl_xxx.h)
// Using dear linearui via a shared library is not recommended: we don't guarantee backward nor forward ABI compatibility + this is a call-heavy library and function call overhead adds up.
#ifndef LINEARUI_API
#define LINEARUI_API
#endif
#ifndef LINEARUI_IMPL_API
#define LINEARUI_IMPL_API              LINEARUI_API
#endif

// Helper Macros
// (note: compiling with NDEBUG will usually strip out assert() to nothing, which is NOT recommended because we use asserts to notify of programmer mistakes.)
#ifndef LINEARUI_ASSERT
#include <assert.h>
#define LINEARUI_ASSERT(_EXPR)            assert(_EXPR)                               // You can override the default assert handler by editing linearuiconfig.h
#endif
#define LINEARUI_COUNTOF(_ARR)            ((int)(sizeof(_ARR) / sizeof(*(_ARR))))     // Size of a static C-style array. Don't use on pointers!
#define LINEARUI_UNUSED(_VAR)             ((void)(_VAR))                              // Used to silence "unused variable warnings". Often useful as asserts may be stripped out from final builds.
#define LINEARUI_STRINGIFY_HELPER(_EXPR)  #_EXPR
#define LINEARUI_STRINGIFY(_EXPR)         LINEARUI_STRINGIFY_HELPER(_EXPR)                  // Preprocessor idiom to stringify e.g. an integer or a macro.

// Check that version and structures layouts are matching between compiled linearui code and caller. Read comments above DebugCheckVersionAndDataLayout() for details.
#define LINEARUI_CHECKVERSION()        LinearUI::DebugCheckVersionAndDataLayout(LINEARUI_VERSION, sizeof(LinearUIIO), sizeof(LinearUIStyle), sizeof(LinearVec2), sizeof(LinearVec4), sizeof(LinearDrawVert), sizeof(LinearDrawIdx))

// Helper Macros - LINEARUI_FMTARGS, LINEARUI_FMTLIST: Apply printf-style warnings to our formatting functions.
// (MSVC provides an equivalent mechanism via SAL Annotations but it requires the macros in a different
//  location. e.g. #include <sal.h> + void myprintf(_Printf_format_string_ const char* format, ...),
//  and only works when using Code Analysis, rather than just normal compiling).
// (see https://github.com/ocornut/linearui/issues/8871 for a patch to enable this for MSVC's Code Analysis)
#if !defined(LINEARUI_USE_STB_SPRINTF) && defined(__MINGW32__) && !defined(__clang__)
#define LINEARUI_FMTARGS(FMT)             __attribute__((format(gnu_printf, FMT, FMT+1)))
#define LINEARUI_FMTLIST(FMT)             __attribute__((format(gnu_printf, FMT, 0)))
#elif !defined(LINEARUI_USE_STB_SPRINTF) && (defined(__clang__) || defined(__GNUC__))
#define LINEARUI_FMTARGS(FMT)             __attribute__((format(printf, FMT, FMT+1)))
#define LINEARUI_FMTLIST(FMT)             __attribute__((format(printf, FMT, 0)))
#else
#define LINEARUI_FMTARGS(FMT)
#define LINEARUI_FMTLIST(FMT)
#endif

// Disable some of MSVC most aggressive Debug runtime checks in function header/footer (used in some simple/low-level functions)
#if defined(_MSC_VER) && !defined(__clang__)  && !defined(__INTEL_COMPILER) && !defined(LINEARUI_DEBUG_PARANOID)
#define LINEARUI_MSVC_RUNTIME_CHECKS_OFF      __pragma(runtime_checks("",off))     __pragma(check_stack(off)) __pragma(strict_gs_check(push,off))
#define LINEARUI_MSVC_RUNTIME_CHECKS_RESTORE  __pragma(runtime_checks("",restore)) __pragma(check_stack())    __pragma(strict_gs_check(pop))
#else
#define LINEARUI_MSVC_RUNTIME_CHECKS_OFF
#define LINEARUI_MSVC_RUNTIME_CHECKS_RESTORE
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
typedef unsigned int        LinearUIID;// A unique ID used by widgets (typically the result of hashing a stack of string)
typedef signed char         LinearS8;   // 8-bit signed integer
typedef unsigned char       LinearU8;   // 8-bit unsigned integer
typedef signed short        LinearS16;  // 16-bit signed integer
typedef unsigned short      LinearU16;  // 16-bit unsigned integer
typedef signed int          LinearS32;  // 32-bit signed integer == int
typedef unsigned int        LinearU32;  // 32-bit unsigned integer (often used to store packed colors)
typedef signed   long long  LinearS64;  // 64-bit signed integer
typedef unsigned long long  LinearU64;  // 64-bit unsigned integer

// Forward declarations: LinearDrawList, LinearFontAtlas layer
struct LinearDrawChannel;               // Temporary storage to output draw commands out of order, used by LinearDrawListSplitter and LinearDrawList::ChannelsSplit()
struct LinearDrawCmd;                   // A single draw command within a parent LinearDrawList (generally maps to 1 GPU draw call, unless it is a callback)
struct LinearDrawData;                  // All draw command lists required to render the frame + pos/size coordinates to use for the projection matrix.
struct LinearDrawList;                  // A single draw command list (generally one per window, conceptually you may see this as a dynamic "mesh" builder)
struct LinearDrawListSharedData;        // Data shared among multiple draw lists (typically owned by parent LinearUI context, but you may create one yourself)
struct LinearDrawListSplitter;          // Helper to split a draw list into different layers which can be drawn into out of order, then flattened back.
struct LinearDrawVert;                  // A single vertex (pos + uv + col = 20 bytes by default. Override layout with LINEARUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT)
struct LinearFont;                      // Runtime data for a single font within a parent LinearFontAtlas
struct LinearFontAtlas;                 // Runtime data for multiple fonts, bake multiple fonts into a single texture, TTF/OTF font loader
struct LinearFontAtlasBuilder;          // Opaque storage for building a LinearFontAtlas
struct LinearFontAtlasRect;             // Output of LinearFontAtlas::GetCustomRect() when using custom rectangles.
struct LinearFontBaked;                 // Baked data for a LinearFont at a given size.
struct LinearFontConfig;                // Configuration data when adding a font or merging fonts
struct LinearFontGlyph;                 // A single font glyph (code point + coordinates within in LinearFontAtlas + offset)
struct LinearFontGlyphRangesBuilder;    // Helper to build glyph ranges from text/string data
struct LinearFontLoader;                // Opaque interface to a font loading backend (linearstb_truetype, FreeType etc.).
struct LinearTextureData;               // Specs and pixel storage for a texture used by Dear LinearUI.
struct LinearTextureRect;               // Coordinates of a rectangle within a texture.
struct LinearColor;                     // Helper functions to create a color that can be converted to either u32 or float4 (*OBSOLETE* please avoid using)

// Forward declarations: LinearUI layer
struct LinearUIContext;                // Dear LinearUI context (opaque structure, unless including linearui_internal.h)
struct LinearUIIO;                     // Main configuration and I/O between your application and LinearUI (also see: LinearUIPlatformIO)
struct LinearUIInputTextCallbackData;  // Shared state of InputText() when using custom LinearUIInputTextCallback (rare/advanced use)
struct LinearUIKeyData;                // Storage for LinearUIIO and IsKeyDown(), IsKeyPressed() etc functions.
struct LinearUIListClipper;            // Helper to manually clip large list of items
struct LinearUIMultiSelectIO;          // Structure to interact with a BeginMultiSelect()/EndMultiSelect() block
struct LinearUIOnceUponAFrame;         // Helper for running a block of code not more than once a frame
struct LinearUIPayload;                // User data payload for drag and drop operations
struct LinearUIPlatformIO;             // Interface between platform/renderer backends and LinearUI (e.g. Clipboard, IME, Multi-Viewport support). Extends LinearUIIO.
struct LinearUIPlatformImeData;        // Platform IME data for io.PlatformSetImeDataFn() function.
struct LinearUIPlatformMonitor;        // Multi-viewport support: user-provided bounds for each connected monitor/display. Used when positioning popups and tooltips to avoid them straddling monitors
struct LinearUISelectionBasicStorage;  // Optional helper to store multi-selection state + apply multi-selection requests.
struct LinearUISelectionExternalStorage;//Optional helper to apply multi-selection requests to existing randomly accessible storage.
struct LinearUISelectionRequest;       // A selection request (stored in LinearUIMultiSelectIO)
struct LinearUISizeCallbackData;       // Callback data when using SetNextWindowSizeConstraints() (rare/advanced use)
struct LinearUIStorage;                // Helper for key->value storage (container sorted by key)
struct LinearUIStoragePair;            // Helper for key->value storage (pair)
struct LinearUIStyle;                  // Runtime data for styling/colors
struct LinearUITableSortSpecs;         // Sorting specifications for a table (often handling sort specs for a single column, occasionally more)
struct LinearUITableColumnSortSpecs;   // Sorting specification for one column of a table
struct LinearUITextBuffer;             // Helper to hold and append into a text buffer (~string builder)
struct LinearUITextFilter;             // Helper to parse and apply text filters (e.g. "aaaaa[,bbbbb][,ccccc]")
struct LinearUIViewport;               // A Platform Window (always 1 unless multi-viewport are enabled. One per platform window to output to). In the future may represent Platform Monitor
struct LinearUIWindowClass;            // Window class (rare/advanced uses: provide hints to the platform backend via altered viewport flags and parent/child info)

// Enumerations
// - We don't use strongly typed enums much because they add constraints (can't extend in private code, can't store typed in bit fields, extra casting on iteration)
// - Tip: Use your programming IDE navigation facilities on the names in the _central column_ below to find the actual flags/enum lists!
//   - In Visual Studio: Ctrl+Comma ("Edit.GoToAll") can follow symbols inside comments, whereas Ctrl+F12 ("Edit.GoToImplementation") cannot.
//   - In Visual Studio w/ Visual Assist installed: Alt+G ("VAssistX.GoToImplementation") can also follow symbols inside comments.
//   - In VS Code, CLion, etc.: Ctrl+Click can follow symbols inside comments.
enum LinearUIDir : int;                // -> enum LinearUIDir              // Enum: A cardinal direction (Left, Right, Up, Down)
enum LinearUIKey : int;                // -> enum LinearUIKey              // Enum: A key identifier (LinearUIKey_XXX or LinearUIMod_XXX value)
enum LinearUIMouseSource : int;        // -> enum LinearUIMouseSource      // Enum; A mouse input source identifier (Mouse, TouchScreen, Pen)
enum LinearUISortDirection : LinearU8;     // -> enum LinearUISortDirection    // Enum: A sorting direction (ascending or descending)
typedef int LinearUICol;               // -> enum LinearUICol_             // Enum: A color identifier for styling
typedef int LinearUICond;              // -> enum LinearUICond_            // Enum: A condition for many Set*() functions
typedef int LinearUIDataType;          // -> enum LinearUIDataType_        // Enum: A primary data type
typedef int LinearUIMouseButton;       // -> enum LinearUIMouseButton_     // Enum: A mouse button identifier (0=left, 1=right, 2=middle)
typedef int LinearUIMouseCursor;       // -> enum LinearUIMouseCursor_     // Enum: A mouse cursor shape
typedef int LinearUIStyleVar;          // -> enum LinearUIStyleVar_        // Enum: A variable identifier for styling
typedef int LinearUITableBgTarget;     // -> enum LinearUITableBgTarget_   // Enum: A color target for TableSetBgColor()

// Flags (declared as int to allow using as flags without overhead, and to not pollute the top of this file)
// - Tip: Use your programming IDE navigation facilities on the names in the _central column_ below to find the actual flags/enum lists!
//   - In Visual Studio: Ctrl+Comma ("Edit.GoToAll") can follow symbols inside comments, whereas Ctrl+F12 ("Edit.GoToImplementation") cannot.
//   - In Visual Studio w/ Visual Assist installed: Alt+G ("VAssistX.GoToImplementation") can also follow symbols inside comments.
//   - In VS Code, CLion, etc.: Ctrl+Click can follow symbols inside comments.
typedef int LinearDrawFlags;            // -> enum LinearDrawFlags_          // Flags: for LinearDrawList functions
typedef int LinearDrawListFlags;        // -> enum LinearDrawListFlags_      // Flags: for LinearDrawList instance
typedef int LinearDrawTextFlags;        // -> enum LinearDrawTextFlags_      // Internal, do not use!
typedef int LinearFontFlags;            // -> enum LinearFontFlags_          // Flags: for LinearFont
typedef int LinearFontAtlasFlags;       // -> enum LinearFontAtlasFlags_     // Flags: for LinearFontAtlas
typedef int LinearUIBackendFlags;      // -> enum LinearUIBackendFlags_    // Flags: for io.BackendFlags
typedef int LinearUIButtonFlags;       // -> enum LinearUIButtonFlags_     // Flags: for InvisibleButton()
typedef int LinearUIChildFlags;        // -> enum LinearUIChildFlags_      // Flags: for BeginChild()
typedef int LinearUIColorEditFlags;    // -> enum LinearUIColorEditFlags_  // Flags: for ColorEdit4(), ColorPicker4() etc.
typedef int LinearUIConfigFlags;       // -> enum LinearUIConfigFlags_     // Flags: for io.ConfigFlags
typedef int LinearUIComboFlags;        // -> enum LinearUIComboFlags_      // Flags: for BeginCombo()
typedef int LinearUIDockNodeFlags;     // -> enum LinearUIDockNodeFlags_   // Flags: for DockSpace()
typedef int LinearUIDragDropFlags;     // -> enum LinearUIDragDropFlags_   // Flags: for BeginDragDropSource(), AcceptDragDropPayload()
typedef int LinearUIFocusedFlags;      // -> enum LinearUIFocusedFlags_    // Flags: for IsWindowFocused()
typedef int LinearUIHoveredFlags;      // -> enum LinearUIHoveredFlags_    // Flags: for IsItemHovered(), IsWindowHovered() etc.
typedef int LinearUIInputFlags;        // -> enum LinearUIInputFlags_      // Flags: for Shortcut(), SetNextItemShortcut()
typedef int LinearUIInputTextFlags;    // -> enum LinearUIInputTextFlags_  // Flags: for InputText(), InputTextMultiline()
typedef int LinearUIItemFlags;         // -> enum LinearUIItemFlags_       // Flags: for PushItemFlag(), shared by all items
typedef int LinearUIKeyChord;          // -> LinearUIKey | LinearUIMod_XXX    // Flags: for IsKeyChordPressed(), Shortcut() etc. an LinearUIKey optionally OR-ed with one or more LinearUIMod_XXX values.
typedef int LinearUIListClipperFlags;  // -> enum LinearUIListClipperFlags_// Flags: for LinearUIListClipper
typedef int LinearUIPopupFlags;        // -> enum LinearUIPopupFlags_      // Flags: for OpenPopup*(), BeginPopupContext*(), IsPopupOpen()
typedef int LinearUIMultiSelectFlags;  // -> enum LinearUIMultiSelectFlags_// Flags: for BeginMultiSelect()
typedef int LinearUISelectableFlags;   // -> enum LinearUISelectableFlags_ // Flags: for Selectable()
typedef int LinearUISliderFlags;       // -> enum LinearUISliderFlags_     // Flags: for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
typedef int LinearUITabBarFlags;       // -> enum LinearUITabBarFlags_     // Flags: for BeginTabBar()
typedef int LinearUITabItemFlags;      // -> enum LinearUITabItemFlags_    // Flags: for BeginTabItem()
typedef int LinearUITableFlags;        // -> enum LinearUITableFlags_      // Flags: For BeginTable()
typedef int LinearUITableColumnFlags;  // -> enum LinearUITableColumnFlags_// Flags: For TableSetupColumn()
typedef int LinearUITableRowFlags;     // -> enum LinearUITableRowFlags_   // Flags: For TableNextRow()
typedef int LinearUITreeNodeFlags;     // -> enum LinearUITreeNodeFlags_   // Flags: for TreeNode(), TreeNodeEx(), CollapsingHeader()
typedef int LinearUIViewportFlags;     // -> enum LinearUIViewportFlags_   // Flags: for LinearUIViewport
typedef int LinearUIWindowFlags;       // -> enum LinearUIWindowFlags_     // Flags: for Begin(), BeginChild()

// Character types
// (we generally use UTF-8 encoded string in the API. This is storage specifically for a decoded character used for keyboard input and display)
typedef unsigned int LinearWchar32;     // A single decoded U32 character/code point. We encode them as multi bytes UTF-8 when used in strings.
typedef unsigned short LinearWchar16;   // A single decoded U16 character/code point. We encode them as multi bytes UTF-8 when used in strings.
#ifdef LINEARUI_USE_WCHAR32            // LinearWchar [configurable type: override in linearuiconfig.h with '#define LINEARUI_USE_WCHAR32' to support Unicode planes 1-16]
typedef LinearWchar32 LinearWchar;
#else
typedef LinearWchar16 LinearWchar;
#endif

// Multi-Selection item index or identifier when using BeginMultiSelect()
// - Used by SetNextItemSelectionUserData() + and inside LinearUIMultiSelectIO structure.
// - Most users are likely to use this store an item INDEX but this may be used to store a POINTER/ID as well. Read comments near LinearUIMultiSelectIO for details.
typedef LinearS64 LinearUISelectionUserData;

// Callback and functions types
typedef int     (*LinearUIInputTextCallback)(LinearUIInputTextCallbackData* data);    // Callback function for LinearUI::InputText()
typedef void    (*LinearUISizeCallback)(LinearUISizeCallbackData* data);              // Callback function for LinearUI::SetNextWindowSizeConstraints()
typedef void*   (*LinearUIMemAllocFunc)(size_t sz, void* user_data);               // Function signature for LinearUI::SetAllocatorFunctions()
typedef void    (*LinearUIMemFreeFunc)(void* ptr, void* user_data);                // Function signature for LinearUI::SetAllocatorFunctions()

// LinearVec2: 2D vector used to store positions, sizes etc. [Compile-time configurable type]
// - This is a frequently used type in the API. Consider using LINEARUI_VEC2_CLASS_EXTRA to create implicit cast from/to our preferred type.
// - Add '#define LINEARUI_DEFINE_MATH_OPERATORS' before including this file (or in linearuiconfig.h) to access courtesy maths operators for LinearVec2 and LinearVec4.
LINEARUI_MSVC_RUNTIME_CHECKS_OFF
struct LinearVec2
{
    float                                   x, y;
    constexpr LinearVec2()                      : x(0.0f), y(0.0f) { }
    constexpr LinearVec2(float _x, float _y)    : x(_x), y(_y) { }
    float& operator[] (size_t idx)          { LINEARUI_ASSERT(idx == 0 || idx == 1); return ((float*)(void*)(char*)this)[idx]; } // We very rarely use this [] operator, so the assert overhead is fine.
    float  operator[] (size_t idx) const    { LINEARUI_ASSERT(idx == 0 || idx == 1); return ((const float*)(const void*)(const char*)this)[idx]; }
#ifdef LINEARUI_VEC2_CLASS_EXTRA
    LINEARUI_VEC2_CLASS_EXTRA     // Define additional constructors and implicit cast operators in linearuiconfig.h to convert back and forth between your math types and LinearVec2.
#endif
};

// LinearVec4: 4D vector used to store clipping rectangles, colors etc. [Compile-time configurable type]
struct LinearVec4
{
    float                                                     x, y, z, w;
    constexpr LinearVec4()                                        : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
    constexpr LinearVec4(float _x, float _y, float _z, float _w)  : x(_x), y(_y), z(_z), w(_w) { }
#ifdef LINEARUI_VEC4_CLASS_EXTRA
    LINEARUI_VEC4_CLASS_EXTRA     // Define additional constructors and implicit cast operators in linearuiconfig.h to convert back and forth between your math types and LinearVec4.
#endif
};
LINEARUI_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] Texture identifiers (LinearTextureID, LinearTextureRef)
//-----------------------------------------------------------------------------

// LinearTextureID = backend specific, low-level identifier for a texture uploaded in GPU/graphics system.
// [Compile-time configurable type]
// - When a Rendered Backend creates a texture, it store its native identifier into a LinearTextureID value.
//   (e.g. Used by DX11 backend to a `ID3D11ShaderResourceView*`; Used by OpenGL backends to store `GLuint`;
//         Used by SDLGPU backend to store a `SDL_GPUTextureSamplerBinding*`, etc.).
// - User may submit their own textures to e.g. LinearUI::Image() function by passing this value.
// - During the rendering loop, the Renderer Backend retrieve the LinearTextureID, which stored inside a
//   LinearTextureRef, which is stored inside a LinearDrawCmd.
// - Compile-time type configuration:
//   - To use something other than a 64-bit value: add '#define LinearTextureID MyTextureType*' in your linearuiconfig.h file.
//   - This can be whatever to you want it to be! read the FAQ entry about textures for details.
//   - You may decide to store a higher-level structure containing texture, sampler, shader etc. with various
//     constructors if you like. You will need to implement ==/!= operators.
// History:
// - In v1.91.4 (2024/10/08): the default type for LinearTextureID was changed from 'void*' to 'LinearU64'. This allowed backends requiring 64-bit worth of data to build on 32-bit architectures. Use intermediary intptr_t cast and read FAQ if you have casting warnings.
// - In v1.92.0 (2025/06/11): added LinearTextureRef which carry either a LinearTextureID either a pointer to internal texture atlas. All user facing functions taking LinearTextureID changed to LinearTextureRef
#ifndef LinearTextureID
typedef LinearU64 LinearTextureID;      // Default: store up to 64-bits (any pointer or integer). A majority of backends are ok with that.
#endif

// Define this if you need to change the invalid value for your backend.
// - If your backend is using LinearTextureID to store an index/offset and you need 0 to be valid, You can add '#define LinearTextureID_Invalid ((LinearTextureID)-1)' in your linearuiconfig.h file.
// - From 2026/03/12 to 2026/03/19 we experimented with changing to default to -1, but I worried it would cause too many issues in third-party code so it was reverted.
#ifndef LinearTextureID_Invalid
#define LinearTextureID_Invalid     ((LinearTextureID)0)
#endif

// LinearTextureRef = higher-level identifier for a texture. Store a LinearTextureID _or_ a LinearTextureData*.
// The identifier is valid even before the texture has been uploaded to the GPU/graphics system.
// This is what gets passed to functions such as `LinearUI::Image()`, `LinearDrawList::AddImage()`.
// This is what gets stored in draw commands (`LinearDrawCmd`) to identify a texture during rendering.
// - When a texture is created by user code (e.g. custom images), we directly store the low-level LinearTextureID.
//   Because of this, when displaying your own texture you are likely to ever only manage LinearTextureID values on your side.
// - When a texture is created by the backend, we stores a LinearTextureData* which becomes an indirection
//   to extract the LinearTextureID value during rendering, after texture upload has happened.
// - To create a LinearTextureRef from a LinearTextureData you can use LinearTextureData::GetTexRef().
//   We intentionally do not provide an LinearTextureRef constructor for this: we don't expect this
//   to be frequently useful to the end-user, and it would be erroneously called by many legacy code.
// - If you want to bind the current atlas when using custom rectangle, you can use io.Fonts->TexRef.
// - Binding generators for languages such as C (which don't have constructors), should provide a helper, e.g.
//      inline LinearTextureRef LinearTextureRefFromID(LinearTextureID tex_id) { LinearTextureRef tex_ref = { ._TexData = NULL, .TexID = tex_id }; return tex_ref; }
// In 1.92 we changed most drawing functions using LinearTextureID to use LinearTextureRef.
// We intentionally do not provide an implicit LinearTextureRef -> LinearTextureID cast operator because it is technically lossy to convert LinearTextureRef to LinearTextureID before rendering.
LINEARUI_MSVC_RUNTIME_CHECKS_OFF
struct LinearTextureRef
{
    LinearTextureRef()                          { _TexData = NULL; _TexID = LinearTextureID_Invalid; }
    LinearTextureRef(LinearTextureID tex_id)        { _TexData = NULL; _TexID = tex_id; }
#if !defined(LINEARUI_DISABLE_OBSOLETE_FUNCTIONS) && !defined(LinearTextureID)
    LinearTextureRef(void* tex_id)              { _TexData = NULL; _TexID = (LinearTextureID)(size_t)tex_id; }  // For legacy backends casting to LinearTextureID
#endif

    inline LinearTextureID  GetTexID() const;   // == (_TexData ? _TexData->TexID : _TexID) // Implemented below in the file.

    // Members (either are set, never both!)
    LinearTextureData*      _TexData;           //      A texture, generally owned by a LinearFontAtlas. Will convert to LinearTextureID during render loop, after texture has been uploaded.
    LinearTextureID         _TexID;             // _OR_ Low-level backend texture identifier, if already uploaded or created by user/app. Generally provided to e.g. LinearUI::Image() calls.
};
LINEARUI_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] Dear LinearUI end-user API functions
// (Note that LinearUI:: being a namespace, you can add extra LinearUI:: functions in your own separate file. Please don't modify linearui source files!)
//-----------------------------------------------------------------------------

namespace LinearUI
{
    // Context creation and access
    // - Each context create its own LinearFontAtlas by default. You may instance one yourself and pass it to CreateContext() to share a font atlas between contexts.
    // - DLL users: heaps and globals are not shared across DLL boundaries! You will need to call SetCurrentContext() + SetAllocatorFunctions()
    //   for each static/DLL boundary you are calling from. Read "Context and Memory Allocators" section of linearui.cpp for details.
    LINEARUI_API LinearUIContext* CreateContext(LinearFontAtlas* shared_font_atlas = NULL);
    LINEARUI_API void          DestroyContext(LinearUIContext* ctx = NULL);   // NULL = destroy current context
    LINEARUI_API LinearUIContext* GetCurrentContext();
    LINEARUI_API void          SetCurrentContext(LinearUIContext* ctx);

    // Main
    LINEARUI_API LinearUIIO&      GetIO();                                    // access the LinearUIIO structure (mouse/keyboard/gamepad inputs, time, various configuration options/flags)
    LINEARUI_API LinearUIPlatformIO& GetPlatformIO();                         // access the LinearUIPlatformIO structure (mostly hooks/functions to connect to platform/renderer and OS Clipboard, IME etc.)
    LINEARUI_API LinearUIStyle&   GetStyle();                                 // access the Style structure (colors, sizes). Always use PushStyleColor(), PushStyleVar() to modify style mid-frame!
    LINEARUI_API void          NewFrame();                                 // start a new Dear LinearUI frame, you can submit any command from this point until Render()/EndFrame().
    LINEARUI_API void          EndFrame();                                 // ends the Dear LinearUI frame. automatically called by Render(). If you don't need to render data (skipping rendering) you may call EndFrame() without Render()... but you'll have wasted CPU already! If you don't need to render, better to not create any windows and not call NewFrame() at all!
    LINEARUI_API void          Render();                                   // ends the Dear LinearUI frame, finalize the draw data. You can then get call GetDrawData().
    LINEARUI_API LinearDrawData*   GetDrawData();                              // valid after Render() and until the next call to NewFrame(). Call LinearUI_ImplXXXX_RenderDrawData() function in your Renderer Backend to render.

    // Demo, Debug, Information
    LINEARUI_API void          ShowDemoWindow(bool* p_open = NULL);        // create Demo window. demonstrate most LinearUI features. call this to learn about the library! try to make it always available in your application!
    LINEARUI_API void          ShowMetricsWindow(bool* p_open = NULL);     // create Metrics/Debugger window. display Dear LinearUI internals: windows, draw commands, various internal state, etc.
    LINEARUI_API void          ShowDebugLogWindow(bool* p_open = NULL);    // create Debug Log window. display a simplified log of important dear linearui events.
    LINEARUI_API void          ShowIDStackToolWindow(bool* p_open = NULL); // create Stack Tool window. hover items with mouse to query information about the source of their unique ID.
    LINEARUI_API void          ShowAboutWindow(bool* p_open = NULL);       // create About window. display Dear LinearUI version, credits and build/system information.
    LINEARUI_API void          ShowStyleEditor(LinearUIStyle* ref = NULL);    // add style editor block (not a window). you can pass in a reference LinearUIStyle structure to compare to, revert to and save to (else it uses the default style)
    LINEARUI_API bool          ShowStyleSelector(const char* label);       // add style selector block (not a window), essentially a combo listing the default styles.
    LINEARUI_API void          ShowFontSelector(const char* label);        // add font selector block (not a window), essentially a combo listing the loaded fonts.
    LINEARUI_API void          ShowUserGuide();                            // add basic help/info block (not a window): how to manipulate LinearUI as an end-user (mouse/keyboard controls).
    LINEARUI_API const char*   GetVersion();                               // get the compiled version string e.g. "1.80 WIP" (essentially the value for LINEARUI_VERSION from the compiled version of linearui.cpp)

    // Styles
    LINEARUI_API void          StyleColorsDark(LinearUIStyle* dst = NULL);    // new, recommended style (default)
    LINEARUI_API void          StyleColorsLight(LinearUIStyle* dst = NULL);   // best used with borders and a custom, thicker font
    LINEARUI_API void          StyleColorsClassic(LinearUIStyle* dst = NULL); // classic linearui style

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
    LINEARUI_API bool          Begin(const char* name, bool* p_open = NULL, LinearUIWindowFlags flags = 0);
    LINEARUI_API void          End();

    // Child Windows
    // - Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window. Child windows can embed their own child.
    // - Before 1.90 (November 2023), the "LinearUIChildFlags child_flags = 0" parameter was "bool border = false".
    //   This API is backward compatible with old code, as we guarantee that LinearUIChildFlags_Borders == true.
    //   Consider updating your old code:
    //      BeginChild("Name", size, false)   -> Begin("Name", size, 0); or Begin("Name", size, LinearUIChildFlags_None);
    //      BeginChild("Name", size, true)    -> Begin("Name", size, LinearUIChildFlags_Borders);
    // - Manual sizing (each axis can use a different setting e.g. LinearVec2(0.0f, 400.0f)):
    //     == 0.0f: use remaining parent window size for this axis.
    //      > 0.0f: use specified size for this axis.
    //      < 0.0f: right/bottom-align to specified distance from available content boundaries.
    // - Specifying LinearUIChildFlags_AutoResizeX or LinearUIChildFlags_AutoResizeY makes the sizing automatic based on child contents.
    //   Combining both LinearUIChildFlags_AutoResizeX _and_ LinearUIChildFlags_AutoResizeY defeats purpose of a scrolling region and is NOT recommended.
    // - BeginChild() returns false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting
    //   anything to the window. Always call a matching EndChild() for each BeginChild() call, regardless of its return value.
    //   [Important: due to legacy reason, Begin/End and BeginChild/EndChild are inconsistent with all other functions
    //    such as BeginMenu/EndMenu, BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding
    //    BeginXXX function returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
    LINEARUI_API bool          BeginChild(const char* str_id, const LinearVec2& size = LinearVec2(0, 0), LinearUIChildFlags child_flags = 0, LinearUIWindowFlags window_flags = 0);
    LINEARUI_API bool          BeginChild(LinearUIID id, const LinearVec2& size = LinearVec2(0, 0), LinearUIChildFlags child_flags = 0, LinearUIWindowFlags window_flags = 0);
    LINEARUI_API void          EndChild();

    // Windows Utilities
    // - 'current window' = the window we are appending into while inside a Begin()/End() block. 'next window' = next window we will Begin() into.
    LINEARUI_API bool          IsWindowAppearing();
    LINEARUI_API bool          IsWindowCollapsed();
    LINEARUI_API bool          IsWindowFocused(LinearUIFocusedFlags flags=0); // is current window focused? or its root/child, depending on flags. see flags for options.
    LINEARUI_API bool          IsWindowHovered(LinearUIHoveredFlags flags=0); // is current window hovered and hoverable (e.g. not blocked by a popup/modal)? See LinearUIHoveredFlags_ for options. IMPORTANT: If you are trying to check whether your mouse should be dispatched to Dear LinearUI or to your underlying app, you should not use this function! Use the 'io.WantCaptureMouse' boolean for that! Refer to FAQ entry "How can I tell whether to dispatch mouse/keyboard to Dear LinearUI or my application?" for details.
    LINEARUI_API LinearDrawList*   GetWindowDrawList();                        // get draw list associated to the current window, to append your own drawing primitives
    LINEARUI_API float         GetWindowDpiScale();                        // get DPI scale currently associated to the current window's viewport.
    LINEARUI_API LinearVec2        GetWindowPos();                             // get current window position in screen space (IT IS UNLIKELY YOU EVER NEED TO USE THIS. Consider always using GetCursorScreenPos() and GetContentRegionAvail() instead)
    LINEARUI_API LinearVec2        GetWindowSize();                            // get current window size (IT IS UNLIKELY YOU EVER NEED TO USE THIS. Consider always using GetCursorScreenPos() and GetContentRegionAvail() instead)
    LINEARUI_API float         GetWindowWidth();                           // get current window width (IT IS UNLIKELY YOU EVER NEED TO USE THIS). Shortcut for GetWindowSize().x.
    LINEARUI_API float         GetWindowHeight();                          // get current window height (IT IS UNLIKELY YOU EVER NEED TO USE THIS). Shortcut for GetWindowSize().y.
    LINEARUI_API LinearUIViewport*GetWindowViewport();                        // get viewport currently associated to the current window.

    // Window manipulation
    // - Prefer using SetNextXXX functions (before Begin) rather that SetXXX functions (after Begin).
    LINEARUI_API void          SetNextWindowPos(const LinearVec2& pos, LinearUICond cond = 0, const LinearVec2& pivot = LinearVec2(0, 0)); // set next window position. call before Begin(). use pivot=(0.5f,0.5f) to center on given point, etc.
    LINEARUI_API void          SetNextWindowSize(const LinearVec2& size, LinearUICond cond = 0);                  // set next window size. set axis to 0.0f to force an auto-fit on this axis. call before Begin()
    LINEARUI_API void          SetNextWindowSizeConstraints(const LinearVec2& size_min, const LinearVec2& size_max, LinearUISizeCallback custom_callback = NULL, void* custom_callback_data = NULL); // set next window size limits. use 0.0f or FLT_MAX if you don't want limits. Use -1 for both min and max of same axis to preserve current size (which itself is a constraint). Use callback to apply non-trivial programmatic constraints.
    LINEARUI_API void          SetNextWindowContentSize(const LinearVec2& size);                               // set next window content size (~ scrollable client area, which enforce the range of scrollbars). Not including window decorations (title bar, menu bar, etc.) nor WindowPadding. set an axis to 0.0f to leave it automatic. call before Begin()
    LINEARUI_API void          SetNextWindowCollapsed(bool collapsed, LinearUICond cond = 0);                 // set next window collapsed state. call before Begin()
    LINEARUI_API void          SetNextWindowFocus();                                                       // set next window to be focused / top-most. call before Begin()
    LINEARUI_API void          SetNextWindowScroll(const LinearVec2& scroll);                                  // set next window scrolling value (use < 0.0f to not affect a given axis).
    LINEARUI_API void          SetNextWindowBgAlpha(float alpha);                                          // set next window background color alpha. helper to easily override the Alpha component of LinearUICol_WindowBg/ChildBg/PopupBg. you may also use LinearUIWindowFlags_NoBackground.
    LINEARUI_API void          SetNextWindowViewport(LinearUIID viewport_id);                                 // set next window viewport
    LINEARUI_API void          SetWindowPos(const LinearVec2& pos, LinearUICond cond = 0);                        // (not recommended) set current window position - call within Begin()/End(). prefer using SetNextWindowPos(), as this may incur tearing and side-effects.
    LINEARUI_API void          SetWindowSize(const LinearVec2& size, LinearUICond cond = 0);                      // (not recommended) set current window size - call within Begin()/End(). set to LinearVec2(0, 0) to force an auto-fit. prefer using SetNextWindowSize(), as this may incur tearing and minor side-effects.
    LINEARUI_API void          SetWindowCollapsed(bool collapsed, LinearUICond cond = 0);                     // (not recommended) set current window collapsed state. prefer using SetNextWindowCollapsed().
    LINEARUI_API void          SetWindowFocus();                                                           // (not recommended) set current window to be focused / top-most. prefer using SetNextWindowFocus().
    LINEARUI_API void          SetWindowPos(const char* name, const LinearVec2& pos, LinearUICond cond = 0);      // set named window position.
    LINEARUI_API void          SetWindowSize(const char* name, const LinearVec2& size, LinearUICond cond = 0);    // set named window size. set axis to 0.0f to force an auto-fit on this axis.
    LINEARUI_API void          SetWindowCollapsed(const char* name, bool collapsed, LinearUICond cond = 0);   // set named window collapsed state
    LINEARUI_API void          SetWindowFocus(const char* name);                                           // set named window to be focused / top-most. use NULL to remove focus.

    // Windows Scrolling
    // - Any change of Scroll will be applied at the beginning of next frame in the first call to Begin().
    // - You may instead use SetNextWindowScroll() prior to calling Begin() to avoid this delay, as an alternative to using SetScrollX()/SetScrollY().
    LINEARUI_API float         GetScrollX();                                                   // get scrolling amount [0 .. GetScrollMaxX()]
    LINEARUI_API float         GetScrollY();                                                   // get scrolling amount [0 .. GetScrollMaxY()]
    LINEARUI_API void          SetScrollX(float scroll_x);                                     // set scrolling amount [0 .. GetScrollMaxX()]
    LINEARUI_API void          SetScrollY(float scroll_y);                                     // set scrolling amount [0 .. GetScrollMaxY()]
    LINEARUI_API float         GetScrollMaxX();                                                // get maximum scrolling amount ~~ ContentSize.x - WindowSize.x - DecorationsSize.x
    LINEARUI_API float         GetScrollMaxY();                                                // get maximum scrolling amount ~~ ContentSize.y - WindowSize.y - DecorationsSize.y
    LINEARUI_API void          SetScrollHereX(float center_x_ratio = 0.5f);                    // adjust scrolling amount to make current cursor position visible. center_x_ratio=0.0: left, 0.5: center, 1.0: right. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.
    LINEARUI_API void          SetScrollHereY(float center_y_ratio = 0.5f);                    // adjust scrolling amount to make current cursor position visible. center_y_ratio=0.0: top, 0.5: center, 1.0: bottom. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.
    LINEARUI_API void          SetScrollFromPosX(float local_x, float center_x_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.
    LINEARUI_API void          SetScrollFromPosY(float local_y, float center_y_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.

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
    LINEARUI_API void          PushFont(LinearFont* font, float font_size_base_unscaled);          // Use NULL as a shortcut to keep current font. Use 0.0f to keep current size.
    LINEARUI_API void          PopFont();
    LINEARUI_API LinearFont*       GetFont();                                                      // get current font
    LINEARUI_API float         GetFontSize();                                                  // get current scaled font size (= height in pixels). AFTER global scale factors applied. *IMPORTANT* DO NOT PASS THIS VALUE TO PushFont()! Use LinearUI::GetStyle().FontSizeBase to get value before global scale factors.
    LINEARUI_API LinearFontBaked*  GetFontBaked();                                                 // get current font bound at current size // == GetFont()->GetFontBaked(GetFontSize())

    // Parameters stacks (shared)
    LINEARUI_API void          PushStyleColor(LinearUICol idx, LinearU32 col);                        // modify a style color. always use this if you modify the style after NewFrame().
    LINEARUI_API void          PushStyleColor(LinearUICol idx, const LinearVec4& col);
    LINEARUI_API void          PopStyleColor(int count = 1);
    LINEARUI_API void          PushStyleVar(LinearUIStyleVar idx, float val);                     // modify a style float variable. always use this if you modify the style after NewFrame()!
    LINEARUI_API void          PushStyleVar(LinearUIStyleVar idx, const LinearVec2& val);             // modify a style LinearVec2 variable. "
    LINEARUI_API void          PushStyleVarX(LinearUIStyleVar idx, float val_x);                  // modify X component of a style LinearVec2 variable. "
    LINEARUI_API void          PushStyleVarY(LinearUIStyleVar idx, float val_y);                  // modify Y component of a style LinearVec2 variable. "
    LINEARUI_API void          PopStyleVar(int count = 1);
    LINEARUI_API void          PushItemFlag(LinearUIItemFlags option, bool enabled);              // modify specified shared item flag, e.g. PushItemFlag(LinearUIItemFlags_NoTabStop, true)
    LINEARUI_API void          PopItemFlag();

    // Parameters stacks (current window)
    LINEARUI_API void          PushItemWidth(float item_width);                                // push width of items for common large "item+label" widgets. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side).
    LINEARUI_API void          PopItemWidth();
    LINEARUI_API void          SetNextItemWidth(float item_width);                             // set width of the _next_ common large "item+label" widget. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side)
    LINEARUI_API float         CalcItemWidth();                                                // width of item given pushed settings and current cursor position. NOT necessarily the width of last item unlike most 'Item' functions.
    LINEARUI_API void          PushTextWrapPos(float wrap_local_pos_x = 0.0f);                 // push word-wrapping position for Text*() commands. < 0.0f: no wrapping; 0.0f: wrap to end of window (or column); > 0.0f: wrap at 'wrap_pos_x' position in window local space
    LINEARUI_API void          PopTextWrapPos();

    // Style read access
    // - Use the ShowStyleEditor() function to interactively see/edit the colors.
    LINEARUI_API LinearVec2        GetFontTexUvWhitePixel();                                       // get UV coordinate for a white pixel, useful to draw custom shapes via the LinearDrawList API
    LINEARUI_API LinearU32         GetColorU32(LinearUICol idx, float alpha_mul = 1.0f);              // retrieve given style color with style alpha applied and optional extra alpha multiplier, packed as a 32-bit value suitable for LinearDrawList
    LINEARUI_API LinearU32         GetColorU32(const LinearVec4& col);                                 // retrieve given color with style alpha applied, packed as a 32-bit value suitable for LinearDrawList
    LINEARUI_API LinearU32         GetColorU32(LinearU32 col, float alpha_mul = 1.0f);                 // retrieve given color with style alpha applied, packed as a 32-bit value suitable for LinearDrawList
    LINEARUI_API const LinearVec4& GetStyleColorVec4(LinearUICol idx);                                // retrieve style color as stored in LinearUIStyle structure. use to feed back into PushStyleColor(), otherwise use GetColorU32() to get style color with style alpha baked in.

    // Layout cursor positioning
    // - By "cursor" we mean the current output position.
    // - The typical widget behavior is to output themselves at the current cursor position, then move the cursor one line down.
    // - You can call SameLine() between widgets to undo the last carriage return and output at the right of the preceding widget.
    // - YOU CAN DO 99% OF WHAT YOU NEED WITH ONLY GetCursorScreenPos() and GetContentRegionAvail().
    // - Attention! We currently have inconsistencies between window-local and absolute positions we will aim to fix with future API:
    //    - Absolute coordinate:        GetCursorScreenPos(), SetCursorScreenPos(), all LinearDrawList:: functions. -> this is the preferred way forward.
    //    - Window-local coordinates:   SameLine(offset), GetCursorPos(), SetCursorPos(), GetCursorStartPos(), PushTextWrapPos()
    //    - Window-local coordinates:   GetContentRegionMax(), GetWindowContentRegionMin(), GetWindowContentRegionMax() --> all obsoleted. YOU DON'T NEED THEM.
    // - GetCursorScreenPos() = GetCursorPos() + GetWindowPos(). GetWindowPos() is almost only ever useful to convert from window-local to absolute coordinates. Try not to use it.
    LINEARUI_API LinearVec2        GetCursorScreenPos();                                           // cursor position, absolute coordinates. THIS IS YOUR BEST FRIEND (prefer using this rather than GetCursorPos(), also more useful to work with LinearDrawList API).
    LINEARUI_API void          SetCursorScreenPos(const LinearVec2& pos);                          // cursor position, absolute coordinates. THIS IS YOUR BEST FRIEND.
    LINEARUI_API LinearVec2        GetContentRegionAvail();                                        // available space from current position. THIS IS YOUR BEST FRIEND.
    LINEARUI_API LinearVec2        GetCursorPos();                                                 // [window-local] cursor position in window-local coordinates. This is not your best friend.
    LINEARUI_API float         GetCursorPosX();                                                // [window-local] "
    LINEARUI_API float         GetCursorPosY();                                                // [window-local] "
    LINEARUI_API void          SetCursorPos(const LinearVec2& local_pos);                          // [window-local] "
    LINEARUI_API void          SetCursorPosX(float local_x);                                   // [window-local] "
    LINEARUI_API void          SetCursorPosY(float local_y);                                   // [window-local] "
    LINEARUI_API LinearVec2        GetCursorStartPos();                                            // [window-local] initial cursor position, in window-local coordinates. Call GetCursorScreenPos() after Begin() to get the absolute coordinates version.

    // Other layout functions
    LINEARUI_API void          Separator();                                                    // separator, generally horizontal. inside a menu bar or in horizontal layout mode, this becomes a vertical separator.
    LINEARUI_API void          SameLine(float offset_from_start_x=0.0f, float spacing=-1.0f);  // call between widgets or groups to layout them horizontally. X position given in window coordinates.
    LINEARUI_API void          NewLine();                                                      // undo a SameLine() or force a new line when in a horizontal-layout context.
    LINEARUI_API void          Spacing();                                                      // add vertical spacing.
    LINEARUI_API void          Dummy(const LinearVec2& size);                                      // add a dummy item of given size. unlike InvisibleButton(), Dummy() won't take the mouse click or be navigable into.
    LINEARUI_API void          Indent(float indent_w = 0.0f);                                  // move content position toward the right, by indent_w, or style.IndentSpacing if indent_w <= 0
    LINEARUI_API void          Unindent(float indent_w = 0.0f);                                // move content position back to the left, by indent_w, or style.IndentSpacing if indent_w <= 0
    LINEARUI_API void          BeginGroup();                                                   // lock horizontal starting position
    LINEARUI_API void          EndGroup();                                                     // unlock horizontal starting position + capture the whole group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.)
    LINEARUI_API void          AlignTextToFramePadding();                                      // vertically align upcoming text baseline to FramePadding.y so that it will align properly to regularly framed items (call if you have text on a line before a framed item)
    LINEARUI_API float         GetTextLineHeight();                                            // ~ FontSize
    LINEARUI_API float         GetTextLineHeightWithSpacing();                                 // ~ FontSize + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of text)
    LINEARUI_API float         GetFrameHeight();                                               // ~ FontSize + style.FramePadding.y * 2
    LINEARUI_API float         GetFrameHeightWithSpacing();                                    // ~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)

    // ID stack/scopes
    // Read the FAQ (docs/FAQ.md or http://dearlinearui.com/faq) for more details about how ID are handled in dear linearui.
    // - Those questions are answered and impacted by understanding of the ID stack system:
    //   - "Q: Why is my widget not reacting when I click on it?"
    //   - "Q: How can I have widgets with an empty label?"
    //   - "Q: How can I have multiple widgets with the same label?"
    // - Short version: ID are hashes of the entire ID stack. If you are creating widgets in a loop you most likely
    //   want to push a unique identifier (e.g. object pointer, loop index) to uniquely differentiate them.
    // - You can also use the "Label##foobar" syntax within widget label to distinguish them from each others.
    // - In this header file we use the "label"/"name" terminology to denote a string that will be displayed + used as an ID,
    //   whereas "str_id" denote a string that is only used as an ID and not normally displayed.
    LINEARUI_API void          PushID(const char* str_id);                                     // push string into the ID stack (will hash string).
    LINEARUI_API void          PushID(const char* str_id_begin, const char* str_id_end);       // push string into the ID stack (will hash string).
    LINEARUI_API void          PushID(const void* ptr_id);                                     // push pointer into the ID stack (will hash pointer).
    LINEARUI_API void          PushID(int int_id);                                             // push integer into the ID stack (will hash integer).
    LINEARUI_API void          PopID();                                                        // pop from the ID stack.
    LINEARUI_API LinearUIID       GetID(const char* str_id);                                      // calculate unique ID (hash of whole ID stack + given parameter). e.g. if you want to query into LinearUIStorage yourself
    LINEARUI_API LinearUIID       GetID(const char* str_id_begin, const char* str_id_end);
    LINEARUI_API LinearUIID       GetID(const void* ptr_id);
    LINEARUI_API LinearUIID       GetID(int int_id);

    // Widgets: Text
    // - Note that all functions taking format strings in the API may be passed ("%s", text) or ("%.*s", text_len, text): which will automatically bypass the formatter.
    LINEARUI_API void          TextUnformatted(const char* text, const char* text_end = NULL); // raw text without formatting. Practically equivalent to 'Text("%s", text)' but doesn't require null terminated string if 'text_end' is specified.
    LINEARUI_API void          Text(const char* fmt, ...)                                      LINEARUI_FMTARGS(1); // formatted text
    LINEARUI_API void          TextV(const char* fmt, va_list args)                            LINEARUI_FMTLIST(1);
    LINEARUI_API void          TextColored(const LinearVec4& col, const char* fmt, ...)            LINEARUI_FMTARGS(2); // shortcut for PushStyleColor(LinearUICol_Text, col); Text(fmt, ...); PopStyleColor();
    LINEARUI_API void          TextColoredV(const LinearVec4& col, const char* fmt, va_list args)  LINEARUI_FMTLIST(2);
    LINEARUI_API void          TextDisabled(const char* fmt, ...)                              LINEARUI_FMTARGS(1); // shortcut for PushStyleColor(LinearUICol_Text, style.Colors[LinearUICol_TextDisabled]); Text(fmt, ...); PopStyleColor();
    LINEARUI_API void          TextDisabledV(const char* fmt, va_list args)                    LINEARUI_FMTLIST(1);
    LINEARUI_API void          TextWrapped(const char* fmt, ...)                               LINEARUI_FMTARGS(1); // shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().
    LINEARUI_API void          TextWrappedV(const char* fmt, va_list args)                     LINEARUI_FMTLIST(1);
    LINEARUI_API void          LabelText(const char* label, const char* fmt, ...)              LINEARUI_FMTARGS(2); // display text+label aligned the same way as value+label widgets
    LINEARUI_API void          LabelTextV(const char* label, const char* fmt, va_list args)    LINEARUI_FMTLIST(2);
    LINEARUI_API void          BulletText(const char* fmt, ...)                                LINEARUI_FMTARGS(1); // shortcut for Bullet()+Text()
    LINEARUI_API void          BulletTextV(const char* fmt, va_list args)                      LINEARUI_FMTLIST(1);
    LINEARUI_API void          SeparatorText(const char* label);                               // currently: formatted text with a horizontal line

    // Widgets: Main
    // - Most widgets return true when the value has been changed or when pressed/selected
    // - You may also use one of the many IsItemXXX functions (e.g. IsItemActive, IsItemHovered, etc.) to query widget state.
    LINEARUI_API bool          Button(const char* label, const LinearVec2& size = LinearVec2(0, 0));   // button
    LINEARUI_API bool          SmallButton(const char* label);                                 // button with (FramePadding.y == 0) to easily embed within text
    LINEARUI_API bool          InvisibleButton(const char* str_id, const LinearVec2& size, LinearUIButtonFlags flags = 0); // flexible button behavior without the visuals, frequently useful to build custom behaviors using the public api (along with IsItemActive, IsItemHovered, etc.)
    LINEARUI_API bool          ArrowButton(const char* str_id, LinearUIDir dir);                  // square button with an arrow shape
    LINEARUI_API bool          Checkbox(const char* label, bool* v);
    LINEARUI_API bool          CheckboxFlags(const char* label, int* flags, int flags_value);
    LINEARUI_API bool          CheckboxFlags(const char* label, unsigned int* flags, unsigned int flags_value);
    LINEARUI_API bool          RadioButton(const char* label, bool active);                    // use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; }
    LINEARUI_API bool          RadioButton(const char* label, int* v, int v_button);           // shortcut to handle the above pattern when value is an integer
    LINEARUI_API void          ProgressBar(float fraction, const LinearVec2& size_arg = LinearVec2(-FLT_MIN, 0), const char* overlay = NULL);
    LINEARUI_API void          Bullet();                                                       // draw a small circle + keep the cursor on the same line. advance cursor x position by GetTreeNodeToLabelSpacing(), same distance that TreeNode() uses
    LINEARUI_API bool          TextLink(const char* label);                                    // hyperlink text button, return true when clicked
    LINEARUI_API bool          TextLinkOpenURL(const char* label, const char* url = NULL);     // hyperlink text button, automatically open file/url when clicked

    // Widgets: Images
    // - Read about LinearTextureID/LinearTextureRef  here: https://github.com/ocornut/linearui/wiki/Image-Loading-and-Displaying-Examples
    // - 'uv0' and 'uv1' are texture coordinates. Read about them from the same link above.
    // - Image() pads adds style.ImageBorderSize on each side, ImageButton() adds style.FramePadding on each side.
    // - ImageButton() draws a background based on regular Button() color + optionally an inner background if specified.
    // - An obsolete version of Image(), before 1.91.9 (March 2025), had a 'tint_col' parameter which is now supported by the ImageWithBg() function.
    LINEARUI_API void          Image(LinearTextureRef tex_ref, const LinearVec2& image_size, const LinearVec2& uv0 = LinearVec2(0, 0), const LinearVec2& uv1 = LinearVec2(1, 1));
    LINEARUI_API void          ImageWithBg(LinearTextureRef tex_ref, const LinearVec2& image_size, const LinearVec2& uv0 = LinearVec2(0, 0), const LinearVec2& uv1 = LinearVec2(1, 1), const LinearVec4& bg_col = LinearVec4(0, 0, 0, 0), const LinearVec4& tint_col = LinearVec4(1, 1, 1, 1));
    LINEARUI_API bool          ImageButton(const char* str_id, LinearTextureRef tex_ref, const LinearVec2& image_size, const LinearVec2& uv0 = LinearVec2(0, 0), const LinearVec2& uv1 = LinearVec2(1, 1), const LinearVec4& bg_col = LinearVec4(0, 0, 0, 0), const LinearVec4& tint_col = LinearVec4(1, 1, 1, 1));

    // Widgets: Combo Box (Dropdown)
    // - The BeginCombo()/EndCombo() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() items.
    // - The old Combo() api are helpers over BeginCombo()/EndCombo() which are kept available for convenience purpose. This is analogous to how ListBox are created.
    LINEARUI_API bool          BeginCombo(const char* label, const char* preview_value, LinearUIComboFlags flags = 0);
    LINEARUI_API void          EndCombo(); // only call EndCombo() if BeginCombo() returns true!
    LINEARUI_API bool          Combo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
    LINEARUI_API bool          Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);      // Separate items with \0 within a string, end item-list with \0\0. e.g. "One\0Two\0Three\0"
    LINEARUI_API bool          Combo(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int popup_max_height_in_items = -1);

    // Widgets: Drag Sliders
    // - Ctrl+Click on any drag box to turn them into an input box. Manually input values aren't clamped by default and can go off-bounds. Use LinearUISliderFlags_AlwaysClamp to always clamp.
    // - For all the Float2/Float3/Float4/Int2/Int3/Int4 versions of every function, note that a 'float v[X]' function argument is the same as 'float* v',
    //   the array syntax is just a way to document the number of elements that are expected to be accessible. You can pass address of your first element out of a contiguous set, e.g. &myvector.x
    // - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
    // - Format string may also be set to NULL or use the default format ("%f" or "%d").
    // - Speed are per-pixel of mouse movement (v_speed=0.2f: mouse needs to move by 5 pixels to increase value by 1). For keyboard/gamepad navigation, minimum speed is Max(v_speed, minimum_step_at_given_precision).
    // - Use v_min < v_max to clamp edits to given limits. Note that Ctrl+Click manual input can override those limits if LinearUISliderFlags_AlwaysClamp is not used.
    // - Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with v_min = -FLT_MAX / INT_MIN to avoid clamping to a minimum.
    // - We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.
    // - Legacy: Pre-1.78 there are DragXXX() function signatures that take a final `float power=1.0f' argument instead of the `LinearUISliderFlags flags=0' argument.
    //   If you get a warning converting a float to LinearUISliderFlags, read https://github.com/ocornut/linearui/issues/3361
    LINEARUI_API bool          DragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", LinearUISliderFlags flags = 0);     // If v_min >= v_max we have no bound
    LINEARUI_API bool          DragFloat2(const char* label, float v[2], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          DragFloat3(const char* label, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          DragFloat4(const char* label, float v[4], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          DragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", const char* format_max = NULL, LinearUISliderFlags flags = 0);
    LINEARUI_API bool          DragInt(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", LinearUISliderFlags flags = 0);  // If v_min >= v_max we have no bound
    LINEARUI_API bool          DragInt2(const char* label, int v[2], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          DragInt3(const char* label, int v[3], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          DragInt4(const char* label, int v[4], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          DragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", const char* format_max = NULL, LinearUISliderFlags flags = 0);
    LINEARUI_API bool          DragScalar(const char* label, LinearUIDataType data_type, void* p_data, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, LinearUISliderFlags flags = 0);
    LINEARUI_API bool          DragScalarN(const char* label, LinearUIDataType data_type, void* p_data, int components, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, LinearUISliderFlags flags = 0);

    // Widgets: Regular Sliders
    // - Ctrl+Click on any slider to turn them into an input box. Manually input values aren't clamped by default and can go off-bounds. Use LinearUISliderFlags_AlwaysClamp to always clamp.
    // - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
    // - Format string may also be set to NULL or use the default format ("%f" or "%d").
    // - Legacy: Pre-1.78 there are SliderXXX() function signatures that take a final `float power=1.0f' argument instead of the `LinearUISliderFlags flags=0' argument.
    //   If you get a warning converting a float to LinearUISliderFlags, read https://github.com/ocornut/linearui/issues/3361
    LINEARUI_API bool          SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", LinearUISliderFlags flags = 0);     // adjust format to decorate the value with a prefix or a suffix for in-slider labels or unit display.
    LINEARUI_API bool          SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format = "%.3f", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format = "%.3f", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format = "%.3f", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          SliderAngle(const char* label, float* v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const char* format = "%.0f deg", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format = "%d", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format = "%d", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format = "%d", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          SliderScalar(const char* label, LinearUIDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, LinearUISliderFlags flags = 0);
    LINEARUI_API bool          SliderScalarN(const char* label, LinearUIDataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format = NULL, LinearUISliderFlags flags = 0);
    LINEARUI_API bool          VSliderFloat(const char* label, const LinearVec2& size, float* v, float v_min, float v_max, const char* format = "%.3f", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          VSliderInt(const char* label, const LinearVec2& size, int* v, int v_min, int v_max, const char* format = "%d", LinearUISliderFlags flags = 0);
    LINEARUI_API bool          VSliderScalar(const char* label, const LinearVec2& size, LinearUIDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, LinearUISliderFlags flags = 0);

    // Widgets: Input with Keyboard
    // - If you want to use InputText() with std::string or any custom dynamic string type, use the wrapper in misc/cpp/linearui_stdlib.h/.cpp!
    // - Most of the LinearUIInputTextFlags flags are only useful for InputText() and not for InputFloatX, InputIntX, InputDouble etc.
    LINEARUI_API bool          InputText(const char* label, char* buf, size_t buf_size, LinearUIInputTextFlags flags = 0, LinearUIInputTextCallback callback = NULL, void* user_data = NULL);
    LINEARUI_API bool          InputTextMultiline(const char* label, char* buf, size_t buf_size, const LinearVec2& size = LinearVec2(0, 0), LinearUIInputTextFlags flags = 0, LinearUIInputTextCallback callback = NULL, void* user_data = NULL);
    LINEARUI_API bool          InputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, LinearUIInputTextFlags flags = 0, LinearUIInputTextCallback callback = NULL, void* user_data = NULL);
    LINEARUI_API bool          InputFloat(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", LinearUIInputTextFlags flags = 0);
    LINEARUI_API bool          InputFloat2(const char* label, float v[2], const char* format = "%.3f", LinearUIInputTextFlags flags = 0);
    LINEARUI_API bool          InputFloat3(const char* label, float v[3], const char* format = "%.3f", LinearUIInputTextFlags flags = 0);
    LINEARUI_API bool          InputFloat4(const char* label, float v[4], const char* format = "%.3f", LinearUIInputTextFlags flags = 0);
    LINEARUI_API bool          InputInt(const char* label, int* v, int step = 1, int step_fast = 100, LinearUIInputTextFlags flags = 0);
    LINEARUI_API bool          InputInt2(const char* label, int v[2], LinearUIInputTextFlags flags = 0);
    LINEARUI_API bool          InputInt3(const char* label, int v[3], LinearUIInputTextFlags flags = 0);
    LINEARUI_API bool          InputInt4(const char* label, int v[4], LinearUIInputTextFlags flags = 0);
    LINEARUI_API bool          InputDouble(const char* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", LinearUIInputTextFlags flags = 0);
    LINEARUI_API bool          InputScalar(const char* label, LinearUIDataType data_type, void* p_data, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, LinearUIInputTextFlags flags = 0);
    LINEARUI_API bool          InputScalarN(const char* label, LinearUIDataType data_type, void* p_data, int components, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, LinearUIInputTextFlags flags = 0);

    // Widgets: Color Editor/Picker (tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.)
    // - Note that in C++ a 'float v[X]' function argument is the _same_ as 'float* v', the array syntax is just a way to document the number of elements that are expected to be accessible.
    // - You can pass the address of a first float element out of a contiguous structure, e.g. &myvector.x
    LINEARUI_API bool          ColorEdit3(const char* label, float col[3], LinearUIColorEditFlags flags = 0);
    LINEARUI_API bool          ColorEdit4(const char* label, float col[4], LinearUIColorEditFlags flags = 0);
    LINEARUI_API bool          ColorPicker3(const char* label, float col[3], LinearUIColorEditFlags flags = 0);
    LINEARUI_API bool          ColorPicker4(const char* label, float col[4], LinearUIColorEditFlags flags = 0, const float* ref_col = NULL);
    LINEARUI_API bool          ColorButton(const char* desc_id, const LinearVec4& col, LinearUIColorEditFlags flags = 0, const LinearVec2& size = LinearVec2(0, 0)); // display a color square/button, hover for details, return true when pressed.
    LINEARUI_API void          SetColorEditOptions(LinearUIColorEditFlags flags);                     // initialize current options (generally on application startup) if you want to select a default format, picker type, etc. User will be able to change many settings, unless you pass the _NoOptions flag to your calls.

    // Widgets: Trees
    // - TreeNode functions return true when the node is open, in which case you need to also call TreePop() when you are finished displaying the tree node contents.
    LINEARUI_API bool          TreeNode(const char* label);
    LINEARUI_API bool          TreeNode(const char* str_id, const char* fmt, ...) LINEARUI_FMTARGS(2);   // helper variation to easily decorrelate the id from the displayed string. Read the FAQ about why and how to use ID. to align arbitrary text at the same level as a TreeNode() you can use Bullet().
    LINEARUI_API bool          TreeNode(const void* ptr_id, const char* fmt, ...) LINEARUI_FMTARGS(2);   // "
    LINEARUI_API bool          TreeNodeV(const char* str_id, const char* fmt, va_list args) LINEARUI_FMTLIST(2);
    LINEARUI_API bool          TreeNodeV(const void* ptr_id, const char* fmt, va_list args) LINEARUI_FMTLIST(2);
    LINEARUI_API bool          TreeNodeEx(const char* label, LinearUITreeNodeFlags flags = 0);
    LINEARUI_API bool          TreeNodeEx(const char* str_id, LinearUITreeNodeFlags flags, const char* fmt, ...) LINEARUI_FMTARGS(3);
    LINEARUI_API bool          TreeNodeEx(const void* ptr_id, LinearUITreeNodeFlags flags, const char* fmt, ...) LINEARUI_FMTARGS(3);
    LINEARUI_API bool          TreeNodeExV(const char* str_id, LinearUITreeNodeFlags flags, const char* fmt, va_list args) LINEARUI_FMTLIST(3);
    LINEARUI_API bool          TreeNodeExV(const void* ptr_id, LinearUITreeNodeFlags flags, const char* fmt, va_list args) LINEARUI_FMTLIST(3);
    LINEARUI_API void          TreePush(const char* str_id);                                       // ~ Indent()+PushID(). Already called by TreeNode() when returning true, but you can call TreePush/TreePop yourself if desired.
    LINEARUI_API void          TreePush(const void* ptr_id);                                       // "
    LINEARUI_API void          TreePop();                                                          // ~ Unindent()+PopID()
    LINEARUI_API float         GetTreeNodeToLabelSpacing();                                        // horizontal distance preceding label when using TreeNode*() or Bullet() == (g.FontSize + style.FramePadding.x*2) for a regular unframed TreeNode
    LINEARUI_API bool          CollapsingHeader(const char* label, LinearUITreeNodeFlags flags = 0);  // if returning 'true' the header is open. doesn't indent nor push on ID stack. user doesn't have to call TreePop().
    LINEARUI_API bool          CollapsingHeader(const char* label, bool* p_visible, LinearUITreeNodeFlags flags = 0); // when 'p_visible != NULL': if '*p_visible==true' display an additional small close button on upper right of the header which will set the bool to false when clicked, if '*p_visible==false' don't display the header.
    LINEARUI_API void          SetNextItemOpen(bool is_open, LinearUICond cond = 0);                  // set next TreeNode/CollapsingHeader open state.
    LINEARUI_API void          SetNextItemStorageID(LinearUIID storage_id);                           // set id to use for open/close storage (default to same as item id).
    LINEARUI_API bool          TreeNodeGetOpen(LinearUIID storage_id);                                // retrieve tree node open/close state.

    // Widgets: Selectables
    // - A selectable highlights when hovered, and can display another color when selected.
    // - Neighbors selectable extend their highlight bounds in order to leave no gap between them. This is so a series of selected Selectable appear contiguous.
    LINEARUI_API bool          Selectable(const char* label, bool selected = false, LinearUISelectableFlags flags = 0, const LinearVec2& size = LinearVec2(0, 0)); // "bool selected" carry the selection state (read-only). Selectable() is clicked is returns true so you can modify your selection state. size.x==0.0: use remaining width, size.x>0.0: specify width. size.y==0.0: use label height, size.y>0.0: specify height
    LINEARUI_API bool          Selectable(const char* label, bool* p_selected, LinearUISelectableFlags flags = 0, const LinearVec2& size = LinearVec2(0, 0));      // "bool* p_selected" point to the selection state (read-write), as a convenient helper.

    // Multi-selection system for Selectable(), Checkbox(), TreeNode() functions [BETA]
    // - This enables standard multi-selection/range-selection idioms (Ctrl+Mouse/Keyboard, Shift+Mouse/Keyboard, etc.) in a way that also allow a clipper to be used.
    // - LinearUISelectionUserData is often used to store your item index within the current view (but may store something else).
    // - Read comments near LinearUIMultiSelectIO for instructions/details and see 'Demo->Widgets->Selection State & Multi-Select' for demo.
    // - TreeNode() is technically supported but... using this correctly is more complicated. You need some sort of linear/random access to your tree,
    //   which is suited to advanced trees setups already implementing filters and clipper. We will work simplifying the current demo.
    // - 'selection_size' and 'items_count' parameters are optional and used by a few features. If they are costly for you to compute, you may avoid them.
    LINEARUI_API LinearUIMultiSelectIO*   BeginMultiSelect(LinearUIMultiSelectFlags flags, int selection_size = -1, int items_count = -1);
    LINEARUI_API LinearUIMultiSelectIO*   EndMultiSelect();
    LINEARUI_API void                  SetNextItemSelectionUserData(LinearUISelectionUserData selection_user_data);
    LINEARUI_API bool                  IsItemToggledSelection();                                   // Was the last item selection state toggled? Useful if you need the per-item information _before_ reaching EndMultiSelect(). We only returns toggle _event_ in order to handle clipping correctly.

    // Widgets: List Boxes
    // - This is essentially a thin wrapper to using BeginChild/EndChild with the LinearUIChildFlags_FrameStyle flag for stylistic changes + displaying a label.
    // - If you don't need a label you can probably simply use BeginChild() with the LinearUIChildFlags_FrameStyle flag for the same result.
    // - You can submit contents and manage your selection state however you want it, by creating e.g. Selectable() or any other items.
    // - The simplified/old ListBox() api are helpers over BeginListBox()/EndListBox() which are kept available for convenience purpose. This is analogous to how Combos are created.
    // - Choose frame width:   size.x > 0.0f: custom  /  size.x < 0.0f or -FLT_MIN: right-align   /  size.x = 0.0f (default): use current ItemWidth
    // - Choose frame height:  size.y > 0.0f: custom  /  size.y < 0.0f or -FLT_MIN: bottom-align  /  size.y = 0.0f (default): arbitrary default height which can fit ~7 items
    LINEARUI_API bool          BeginListBox(const char* label, const LinearVec2& size = LinearVec2(0, 0)); // open a framed scrolling region
    LINEARUI_API void          EndListBox();                                                       // only call EndListBox() if BeginListBox() returned true!
    LINEARUI_API bool          ListBox(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items = -1);
    LINEARUI_API bool          ListBox(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int height_in_items = -1);

    // Widgets: Data Plotting
    // - Consider using LinearPlot (https://github.com/epezent/implot) which is much better!
    LINEARUI_API void          PlotLines(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, LinearVec2 graph_size = LinearVec2(0, 0), int stride = sizeof(float));
    LINEARUI_API void          PlotLines(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, LinearVec2 graph_size = LinearVec2(0, 0));
    LINEARUI_API void          PlotHistogram(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, LinearVec2 graph_size = LinearVec2(0, 0), int stride = sizeof(float));
    LINEARUI_API void          PlotHistogram(const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, LinearVec2 graph_size = LinearVec2(0, 0));

    // Widgets: Value() Helpers.
    // - Those are merely shortcut to calling Text() with a format string. Output single value in "name: value" format (tip: freely declare more in your code to handle your types. you can add functions to the LinearUI namespace)
    LINEARUI_API void          Value(const char* prefix, bool b);
    LINEARUI_API void          Value(const char* prefix, int v);
    LINEARUI_API void          Value(const char* prefix, unsigned int v);
    LINEARUI_API void          Value(const char* prefix, float v, const char* float_format = NULL);

    // Widgets: Menus
    // - Use BeginMenuBar() on a window LinearUIWindowFlags_MenuBar to append to its menu bar.
    // - Use BeginMainMenuBar() to create a menu bar at the top of the screen and append to it.
    // - Use BeginMenu() to create a menu. You can call BeginMenu() multiple time with the same identifier to append more items to it.
    // - Not that MenuItem() keyboardshortcuts are displayed as a convenience but _not processed_ by Dear LinearUI at the moment.
    LINEARUI_API bool          BeginMenuBar();                                                     // append to menu-bar of current window (requires LinearUIWindowFlags_MenuBar flag set on parent window).
    LINEARUI_API void          EndMenuBar();                                                       // only call EndMenuBar() if BeginMenuBar() returns true!
    LINEARUI_API bool          BeginMainMenuBar();                                                 // create and append to a full screen menu-bar.
    LINEARUI_API void          EndMainMenuBar();                                                   // only call EndMainMenuBar() if BeginMainMenuBar() returns true!
    LINEARUI_API bool          BeginMenu(const char* label, bool enabled = true);                  // create a sub-menu entry. only call EndMenu() if this returns true!
    LINEARUI_API void          EndMenu();                                                          // only call EndMenu() if BeginMenu() returns true!
    LINEARUI_API bool          MenuItem(const char* label, const char* shortcut = NULL, bool selected = false, bool enabled = true);  // return true when activated.
    LINEARUI_API bool          MenuItem(const char* label, const char* shortcut, bool* p_selected, bool enabled = true);              // return true when activated + toggle (*p_selected) if p_selected != NULL

    // Tooltips
    // - Tooltips are windows following the mouse. They do not take focus away.
    // - A tooltip window can contain items of any types.
    // - SetTooltip() is more or less a shortcut for the 'if (BeginTooltip()) { Text(...); EndTooltip(); }' idiom (with a subtlety that it discard any previously submitted tooltip)
    LINEARUI_API bool          BeginTooltip();                                                     // begin/append a tooltip window.
    LINEARUI_API void          EndTooltip();                                                       // only call EndTooltip() if BeginTooltip()/BeginItemTooltip() returns true!
    LINEARUI_API void          SetTooltip(const char* fmt, ...) LINEARUI_FMTARGS(1);                     // set a text-only tooltip. Often used after a LinearUI::IsItemHovered() check. Override any previous call to SetTooltip().
    LINEARUI_API void          SetTooltipV(const char* fmt, va_list args) LINEARUI_FMTLIST(1);

    // Tooltips: helpers for showing a tooltip when hovering an item
    // - BeginItemTooltip() is a shortcut for the 'if (IsItemHovered(LinearUIHoveredFlags_ForTooltip) && BeginTooltip())' idiom.
    // - SetItemTooltip() is a shortcut for the 'if (IsItemHovered(LinearUIHoveredFlags_ForTooltip)) { SetTooltip(...); }' idiom.
    // - Where 'LinearUIHoveredFlags_ForTooltip' itself is a shortcut to use 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav' depending on active input type. For mouse it defaults to 'LinearUIHoveredFlags_Stationary | LinearUIHoveredFlags_DelayShort'.
    LINEARUI_API bool          BeginItemTooltip();                                                 // begin/append a tooltip window if preceding item was hovered.
    LINEARUI_API void          SetItemTooltip(const char* fmt, ...) LINEARUI_FMTARGS(1);                 // set a text-only tooltip if preceding item was hovered. override any previous call to SetTooltip().
    LINEARUI_API void          SetItemTooltipV(const char* fmt, va_list args) LINEARUI_FMTLIST(1);

    // Popups, Modals
    //  - They block normal mouse hovering detection (and therefore most mouse interactions) behind them.
    //  - If not modal: they can be closed by clicking anywhere outside them, or by pressing Escape (call 'Shortcut(LinearUIKey_Escape)' to claim a higher-priority shortcut).
    //  - Their visibility state (~bool) is held internally instead of being held by the programmer as we are used to with regular Begin*() calls.
    //  - The 3 properties above are related: we need to retain popup visibility state in the library because popups may be closed as any time.
    //  - You can bypass the hovering restriction by using LinearUIHoveredFlags_AllowWhenBlockedByPopup when calling IsItemHovered() or IsWindowHovered().
    //  - IMPORTANT: Popup identifiers are relative to the current ID stack, so OpenPopup and BeginPopup generally needs to be at the same level of the stack.
    //    This is sometimes leading to confusing mistakes. May rework this in the future.
    //  - BeginPopup(): query popup state, if open start appending into the window. Call EndPopup() afterwards if returned true. LinearUIWindowFlags are forwarded to the window.
    //  - BeginPopupModal(): block every interaction behind the window, cannot be closed by user, add a dimming background, has a title bar.
    LINEARUI_API bool          BeginPopup(const char* str_id, LinearUIWindowFlags flags = 0);                         // return true if the popup is open, and you can start outputting to it.
    LINEARUI_API bool          BeginPopupModal(const char* name, bool* p_open = NULL, LinearUIWindowFlags flags = 0); // return true if the modal is open, and you can start outputting to it.
    LINEARUI_API void          EndPopup();                                                                         // only call EndPopup() if BeginPopupXXX() returns true!

    // Popups: open/close functions
    //  - OpenPopup(): set popup state to open. LinearUIPopupFlags are available for opening options.
    //  - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
    //  - CloseCurrentPopup(): use inside the BeginPopup()/EndPopup() scope to close manually.
    //  - CloseCurrentPopup() is called by default by Selectable()/MenuItem() when activated (FIXME: need some options).
    //  - Use LinearUIPopupFlags_NoOpenOverExistingPopup to avoid opening a popup if there's already one at the same level. This is equivalent to e.g. testing for !IsAnyPopupOpen() prior to OpenPopup().
    //  - Use IsWindowAppearing() after BeginPopup() to tell if a window just opened.
    LINEARUI_API void          OpenPopup(const char* str_id, LinearUIPopupFlags popup_flags = 0);                     // call to mark popup as open (don't call every frame!).
    LINEARUI_API void          OpenPopup(LinearUIID id, LinearUIPopupFlags popup_flags = 0);                             // id overload to facilitate calling from nested stacks
    LINEARUI_API void          OpenPopupOnItemClick(const char* str_id = NULL, LinearUIPopupFlags popup_flags = 0);   // helper to open popup when clicked on last item. Default to LinearUIPopupFlags_MouseButtonRight == 1. (note: actually triggers on the mouse _released_ event to be consistent with popup behaviors)
    LINEARUI_API void          CloseCurrentPopup();                                                                // manually close the popup we have begin-ed into.

    // Popups: Open+Begin popup combined functions helpers to create context menus.
    //  - Helpers to do OpenPopup+BeginPopup where the Open action is triggered by e.g. hovering an item and right-clicking.
    //  - IMPORTANT: Notice that BeginPopupContextXXX takes LinearUIPopupFlags just like OpenPopup() and unlike BeginPopup(). For full consistency, we may add LinearUIWindowFlags to the BeginPopupContextXXX functions in the future.
    //  - IMPORTANT: If you ever used the left mouse button with BeginPopupContextXXX() helpers before 1.92.6:
    //    - Before this version, OpenPopupOnItemClick(), BeginPopupContextItem(), BeginPopupContextWindow(), BeginPopupContextVoid() had 'a LinearUIPopupFlags popup_flags = 1' default value in their function signature.
    //    - Before: Explicitly passing a literal 0 meant LinearUIPopupFlags_MouseButtonLeft. The default = 1 meant LinearUIPopupFlags_MouseButtonRight.
    //    - After: The default = 0 means LinearUIPopupFlags_MouseButtonRight. Explicitly passing a literal 1 also means LinearUIPopupFlags_MouseButtonRight (if legacy behavior are enabled) or will assert (if legacy behavior are disabled).
    //    - TL;DR: if you don't want to use right mouse button for popups, always specify it explicitly using a named LinearUIPopupFlags_MouseButtonXXXX value.
    //    - Read "API BREAKING CHANGES" 2026/01/07 (1.92.6) entry in linearui.cpp or GitHub topic #9157 for all details.
    LINEARUI_API bool          BeginPopupContextItem(const char* str_id = NULL, LinearUIPopupFlags popup_flags = 0);  // open+begin popup when clicked on last item. Use str_id==NULL to associate the popup to previous item. If you want to use that on a non-interactive item such as Text() you need to pass in an explicit ID here. read comments in .cpp!
    LINEARUI_API bool          BeginPopupContextWindow(const char* str_id = NULL, LinearUIPopupFlags popup_flags = 0);// open+begin popup when clicked on current window.
    LINEARUI_API bool          BeginPopupContextVoid(const char* str_id = NULL, LinearUIPopupFlags popup_flags = 0);  // open+begin popup when clicked in void (where there are no windows).

    // Popups: query functions
    //  - IsPopupOpen(): return true if the popup is open at the current BeginPopup() level of the popup stack.
    //  - IsPopupOpen() with LinearUIPopupFlags_AnyPopupId: return true if any popup is open at the current BeginPopup() level of the popup stack.
    //  - IsPopupOpen() with LinearUIPopupFlags_AnyPopupId + LinearUIPopupFlags_AnyPopupLevel: return true if any popup is open.
    LINEARUI_API bool          IsPopupOpen(const char* str_id, LinearUIPopupFlags flags = 0);                         // return true if the popup is open.

    // Tables
    // - Full-featured replacement for old Columns API.
    // - See Demo->Tables for demo code. See top of linearui_tables.cpp for general commentary.
    // - See LinearUITableFlags_ and LinearUITableColumnFlags_ enums for a description of available flags.
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
    LINEARUI_API bool          BeginTable(const char* str_id, int columns, LinearUITableFlags flags = 0, const LinearVec2& outer_size = LinearVec2(0.0f, 0.0f), float inner_width = 0.0f);
    LINEARUI_API void          EndTable();                                         // only call EndTable() if BeginTable() returns true!
    LINEARUI_API void          TableNextRow(LinearUITableRowFlags row_flags = 0, float min_row_height = 0.0f); // append into the first cell of a new row. 'min_row_height' include the minimum top and bottom padding aka CellPadding.y * 2.0f.
    LINEARUI_API bool          TableNextColumn();                                  // append into the next column (or first column of next row if currently in last column). Return true when column is visible.
    LINEARUI_API bool          TableSetColumnIndex(int column_n);                  // append into the specified column. Return true when column is visible.

    // Tables: Headers & Columns declaration
    // - Use TableSetupColumn() to specify label, resizing policy, default width/weight, id, various other flags etc.
    // - Use TableHeadersRow() to create a header row and automatically submit a TableHeader() for each column.
    //   Headers are required to perform: reordering, sorting, and opening the context menu.
    //   The context menu can also be made available in columns body using LinearUITableFlags_ContextMenuInBody.
    // - You may manually submit headers using TableNextRow() + TableHeader() calls, but this is only useful in
    //   some advanced use cases (e.g. adding custom widgets in header row).
    // - Use TableSetupScrollFreeze() to lock columns/rows so they stay visible when scrolled. When freezing columns you would usually also use LinearUITableColumnFlags_NoHide on them.
    LINEARUI_API void          TableSetupColumn(const char* label, LinearUITableColumnFlags flags = 0, float init_width_or_weight = 0.0f, LinearUIID user_id = 0);
    LINEARUI_API void          TableSetupScrollFreeze(int cols, int rows);         // lock columns/rows so they stay visible when scrolled.
    LINEARUI_API void          TableHeader(const char* label);                     // submit one header cell manually (rarely used)
    LINEARUI_API void          TableHeadersRow();                                  // submit a row with headers cells based on data provided to TableSetupColumn() + submit context menu
    LINEARUI_API void          TableAngledHeadersRow();                            // submit a row with angled headers for every column with the LinearUITableColumnFlags_AngledHeader flag. MUST BE FIRST ROW.

    // Tables: Sorting & Miscellaneous functions
    // - Sorting: call TableGetSortSpecs() to retrieve latest sort specs for the table. NULL when not sorting.
    //   When 'sort_specs->SpecsDirty == true' you should sort your data. It will be true when sorting specs have
    //   changed since last call, or the first time. Make sure to set 'SpecsDirty = false' after sorting,
    //   else you may wastefully sort your data every frame!
    // - Functions args 'int column_n' treat the default value of -1 as the same as passing the current column index.
    LINEARUI_API LinearUITableSortSpecs*  TableGetSortSpecs();                        // get latest sort specs for the table (NULL if not sorting).  Lifetime: don't hold on this pointer over multiple frames or past any subsequent call to BeginTable().
    LINEARUI_API int                   TableGetColumnCount();                      // return number of columns (value passed to BeginTable)
    LINEARUI_API int                   TableGetColumnIndex();                      // return current column index.
    LINEARUI_API int                   TableGetRowIndex();                         // return current row index (header rows are accounted for)
    LINEARUI_API const char*           TableGetColumnName(int column_n = -1);      // return "" if column didn't have a name declared by TableSetupColumn(). Pass -1 to use current column.
    LINEARUI_API LinearUITableColumnFlags TableGetColumnFlags(int column_n = -1);     // return column flags so you can query their Enabled/Visible/Sorted/Hovered status flags. Pass -1 to use current column.
    LINEARUI_API void                  TableSetColumnEnabled(int column_n, bool v);// change user accessible enabled/disabled state of a column. Set to false to hide the column. User can use the context menu to change this themselves (right-click in headers, or right-click in columns body with LinearUITableFlags_ContextMenuInBody)
    LINEARUI_API int                   TableGetHoveredColumn();                    // return hovered column. return -1 when table is not hovered. return columns_count if the unused space at the right of visible columns is hovered. Can also use (TableGetColumnFlags() & LinearUITableColumnFlags_IsHovered) instead.
    LINEARUI_API void                  TableSetBgColor(LinearUITableBgTarget target, LinearU32 color, int column_n = -1);  // change the color of a cell, row, or column. See LinearUITableBgTarget_ flags for details.

    // Legacy Columns API (prefer using Tables!)
    // - You can also use SameLine(pos_x) to mimic simplified columns.
    LINEARUI_API void          Columns(int count = 1, const char* id = NULL, bool borders = true);
    LINEARUI_API void          NextColumn();                                                       // next column, defaults to current row or next row if the current row is finished
    LINEARUI_API int           GetColumnIndex();                                                   // get current column index
    LINEARUI_API float         GetColumnWidth(int column_index = -1);                              // get column width (in pixels). pass -1 to use current column
    LINEARUI_API void          SetColumnWidth(int column_index, float width);                      // set column width (in pixels). pass -1 to use current column
    LINEARUI_API float         GetColumnOffset(int column_index = -1);                             // get position of column line (in pixels, from the left side of the contents region). pass -1 to use current column, otherwise 0..GetColumnsCount() inclusive. column 0 is typically 0.0f
    LINEARUI_API void          SetColumnOffset(int column_index, float offset_x);                  // set position of column line (in pixels, from the left side of the contents region). pass -1 to use current column
    LINEARUI_API int           GetColumnsCount();

    // Tab Bars, Tabs
    // - Note: Tabs are automatically created by the docking system (when in 'docking' branch). Use this to create tab bars/tabs yourself.
    LINEARUI_API bool          BeginTabBar(const char* str_id, LinearUITabBarFlags flags = 0);        // create and append into a TabBar
    LINEARUI_API void          EndTabBar();                                                        // only call EndTabBar() if BeginTabBar() returns true!
    LINEARUI_API bool          BeginTabItem(const char* label, bool* p_open = NULL, LinearUITabItemFlags flags = 0); // create a Tab. Returns true if the Tab is selected.
    LINEARUI_API void          EndTabItem();                                                       // only call EndTabItem() if BeginTabItem() returns true!
    LINEARUI_API bool          TabItemButton(const char* label, LinearUITabItemFlags flags = 0);      // create a Tab behaving like a button. return true when clicked. cannot be selected in the tab bar.
    LINEARUI_API void          SetTabItemClosed(const char* tab_or_docked_window_label);           // notify TabBar or Docking system of a closed tab/window ahead (useful to reduce visual flicker on reorderable tab bars). For tab-bar: call after BeginTabBar() and before Tab submissions. Otherwise call with a window name.

    // Docking
    // - Read https://github.com/ocornut/linearui/wiki/Docking for details.
    // - Enable with io.ConfigFlags |= LinearUIConfigFlags_DockingEnable.
    // - You can use many Docking facilities without calling any API.
    //   - Drag from window title bar or their tab to dock/undock. Hold SHIFT to disable docking.
    //   - Drag from window menu button (upper-left button) to undock an entire node (all windows).
    //   - When io.ConfigDockingWithShift == true, you instead need to hold SHIFT to enable docking.
    // - DockSpaceOverViewport:
    //   - This is a helper to create an invisible window covering a viewport, then submit a DockSpace() into it.
    //   - Most applications can simply call DockSpaceOverViewport() once to allow docking windows into e.g. the edge of your screen.
    //     e.g. LinearUI::NewFrame(); LinearUI::DockSpaceOverViewport();                                                   // Create a dockspace in main viewport.
    //      or: LinearUI::NewFrame(); LinearUI::DockSpaceOverViewport(0, nullptr, LinearUIDockNodeFlags_PassthruCentralNode); // Create a dockspace in main viewport, central node is transparent.
    // - Dockspaces:
    //   - A dockspace is an explicit dock node within an existing window.
    //   - IMPORTANT: Dockspaces need to be submitted _before_ any window they can host. Submit them early in your frame!
    //   - IMPORTANT: Dockspaces need to be kept alive if hidden, otherwise windows docked into it will be undocked.
    //     If you have e.g. multiple tabs with a dockspace inside each tab: submit the non-visible dockspaces with LinearUIDockNodeFlags_KeepAliveOnly.
    //   - See 'Demo->Examples->Dockspace' or 'Demo->Examples->Documents' for more detailed demos.
    // - Programmatic docking:
    //   - There is no public API yet other than the very limited SetNextWindowDockID() function. Sorry for that!
    //   - Read https://github.com/ocornut/linearui/wiki/Docking for examples of how to use current internal API.
    LINEARUI_API LinearUIID       DockSpace(LinearUIID dockspace_id, const LinearVec2& size = LinearVec2(0, 0), LinearUIDockNodeFlags flags = 0, const LinearUIWindowClass* window_class = NULL);
    LINEARUI_API LinearUIID       DockSpaceOverViewport(LinearUIID dockspace_id = 0, const LinearUIViewport* viewport = NULL, LinearUIDockNodeFlags flags = 0, const LinearUIWindowClass* window_class = NULL);
    LINEARUI_API void          SetNextWindowDockID(LinearUIID dock_id, LinearUICond cond = 0);           // set next window dock id
    LINEARUI_API void          SetNextWindowClass(const LinearUIWindowClass* window_class);           // set next window class (control docking compatibility + provide hints to platform backend via custom viewport flags and platform parent/child relationship)
    LINEARUI_API LinearUIID       GetWindowDockID();                                                  // get dock id of current window, or 0 if not associated to any docking node.
    LINEARUI_API bool          IsWindowDocked();                                                   // is current window docked into another window?

    // Logging/Capture
    // - All text output from the interface can be captured into tty/file/clipboard. By default, tree nodes are automatically opened during logging.
    LINEARUI_API void          LogToTTY(int auto_open_depth = -1);                                 // start logging to tty (stdout)
    LINEARUI_API void          LogToFile(int auto_open_depth = -1, const char* filename = NULL);   // start logging to file
    LINEARUI_API void          LogToClipboard(int auto_open_depth = -1);                           // start logging to OS clipboard
    LINEARUI_API void          LogFinish();                                                        // stop logging (close file, etc.)
    LINEARUI_API void          LogButtons();                                                       // helper to display buttons for logging to tty/file/clipboard
    LINEARUI_API void          LogText(const char* fmt, ...) LINEARUI_FMTARGS(1);                        // pass text data straight to log (without being displayed)
    LINEARUI_API void          LogTextV(const char* fmt, va_list args) LINEARUI_FMTLIST(1);

    // Drag and Drop
    // - On source items, call BeginDragDropSource(), if it returns true also call SetDragDropPayload() + EndDragDropSource().
    // - On target candidates, call BeginDragDropTarget(), if it returns true also call AcceptDragDropPayload() + EndDragDropTarget().
    // - If you stop calling BeginDragDropSource() the payload is preserved however it won't have a preview tooltip (we currently display a fallback "..." tooltip, see #1725)
    // - An item can be both drag source and drop target.
    LINEARUI_API bool          BeginDragDropSource(LinearUIDragDropFlags flags = 0);                                      // call after submitting an item which may be dragged. when this return true, you can call SetDragDropPayload() + EndDragDropSource()
    LINEARUI_API bool          SetDragDropPayload(const char* type, const void* data, size_t sz, LinearUICond cond = 0);  // type is a user defined string of maximum 32 characters. Strings starting with '_' are reserved for dear linearui internal types. Data is copied and held by linearui. Return true when payload has been accepted.
    LINEARUI_API void          EndDragDropSource();                                                                    // only call EndDragDropSource() if BeginDragDropSource() returns true!
    LINEARUI_API bool                  BeginDragDropTarget();                                                          // call after submitting an item that may receive a payload. If this returns true, you can call AcceptDragDropPayload() + EndDragDropTarget()
    LINEARUI_API const LinearUIPayload*   AcceptDragDropPayload(const char* type, LinearUIDragDropFlags flags = 0);          // accept contents of a given type. If LinearUIDragDropFlags_AcceptBeforeDelivery is set you can peek into the payload before the mouse button is released.
    LINEARUI_API void                  EndDragDropTarget();                                                            // only call EndDragDropTarget() if BeginDragDropTarget() returns true!
    LINEARUI_API const LinearUIPayload*   GetDragDropPayload();                                                           // peek directly into the current payload from anywhere. returns NULL when drag and drop is finished or inactive. use LinearUIPayload::IsDataType() to test for the payload type.

    // Disabling [BETA API]
    // - Disable all user interactions and dim items visuals (applying style.DisabledAlpha over current colors)
    // - Those can be nested but it cannot be used to enable an already disabled section (a single BeginDisabled(true) in the stack is enough to keep everything disabled)
    // - Tooltips windows are automatically opted out of disabling. Note that IsItemHovered() by default returns false on disabled items, unless using LinearUIHoveredFlags_AllowWhenDisabled.
    // - BeginDisabled(false)/EndDisabled() essentially does nothing but is provided to facilitate use of boolean expressions (as a micro-optimization: if you have tens of thousands of BeginDisabled(false)/EndDisabled() pairs, you might want to reformulate your code to avoid making those calls)
    LINEARUI_API void          BeginDisabled(bool disabled = true);
    LINEARUI_API void          EndDisabled();

    // Clipping
    // - Mouse hovering is affected by LinearUI::PushClipRect() calls, unlike direct calls to LinearDrawList::PushClipRect() which are render only.
    LINEARUI_API void          PushClipRect(const LinearVec2& clip_rect_min, const LinearVec2& clip_rect_max, bool intersect_with_current_clip_rect);
    LINEARUI_API void          PopClipRect();

    // Focus, Activation
    LINEARUI_API void          SetItemDefaultFocus();                                              // make last item the default focused item of a newly appearing window.
    LINEARUI_API void          SetKeyboardFocusHere(int offset = 0);                               // focus keyboard on the next widget. Use positive 'offset' to access sub components of a multiple component widget. Use -1 to access previous widget.

    // Keyboard/Gamepad Navigation
    LINEARUI_API void          SetNavCursorVisible(bool visible);                                  // alter visibility of keyboard/gamepad cursor. by default: show when using an arrow key, hide when clicking with mouse.

    // Overlapping mode
    LINEARUI_API void          SetNextItemAllowOverlap();                                          // allow next item to be overlapped by a subsequent item. Typically useful with InvisibleButton(), Selectable(), TreeNode() covering an area where subsequent items may need to be added. Note that both Selectable() and TreeNode() have dedicated flags doing this.

    // Item/Widgets Utilities and Query Functions
    // - Most of the functions are referring to the previous Item that has been submitted.
    // - See Demo Window under "Widgets->Querying Status" for an interactive visualization of most of those functions.
    LINEARUI_API bool          IsItemHovered(LinearUIHoveredFlags flags = 0);                         // is the last item hovered? (and usable, aka not blocked by a popup, etc.). See LinearUIHoveredFlags for more options.
    LINEARUI_API bool          IsItemActive();                                                     // is the last item active? (e.g. button being held, text field being edited. This will continuously return true while holding mouse button on an item. Items that don't interact will always return false)
    LINEARUI_API bool          IsItemFocused();                                                    // is the last item focused for keyboard/gamepad navigation?
    LINEARUI_API bool          IsItemClicked(LinearUIMouseButton mouse_button = 0);                   // is the last item hovered and mouse clicked on? (**)  == IsMouseClicked(mouse_button) && IsItemHovered()Important. (**) this is NOT equivalent to the behavior of e.g. Button(). Read comments in function definition.
    LINEARUI_API bool          IsItemVisible();                                                    // is the last item visible? (items may be out of sight because of clipping/scrolling)
    LINEARUI_API bool          IsItemEdited();                                                     // did the last item modify its underlying value this frame? or was pressed? This is generally the same as the "bool" return value of many widgets.
    LINEARUI_API bool          IsItemActivated();                                                  // was the last item just made active (item was previously inactive).
    LINEARUI_API bool          IsItemDeactivated();                                                // was the last item just made inactive (item was previously active). Useful for Undo/Redo patterns with widgets that require continuous editing.
    LINEARUI_API bool          IsItemDeactivatedAfterEdit();                                       // was the last item just made inactive and made a value change when it was active? (e.g. Slider/Drag moved). Useful for Undo/Redo patterns with widgets that require continuous editing. Note that you may get false positives (some widgets such as Combo()/ListBox()/Selectable() will return true even when clicking an already selected item).
    LINEARUI_API bool          IsItemToggledOpen();                                                // was the last item open state toggled? set by TreeNode().
    LINEARUI_API bool          IsAnyItemHovered();                                                 // is any item hovered?
    LINEARUI_API bool          IsAnyItemActive();                                                  // is any item active?
    LINEARUI_API bool          IsAnyItemFocused();                                                 // is any item focused?
    LINEARUI_API LinearUIID       GetItemID();                                                        // get ID of last item (~~ often same LinearUI::GetID(label) beforehand)
    LINEARUI_API LinearVec2        GetItemRectMin();                                                   // get upper-left bounding rectangle of the last item (screen space)
    LINEARUI_API LinearVec2        GetItemRectMax();                                                   // get lower-right bounding rectangle of the last item (screen space)
    LINEARUI_API LinearVec2        GetItemRectSize();                                                  // get size of last item
    LINEARUI_API LinearUIItemFlags GetItemFlags();                                                    // get generic flags of last item

    // Viewports
    // - Currently represents the Platform Window created by the application which is hosting our Dear LinearUI windows.
    // - In 'docking' branch with multi-viewport enabled, we extend this concept to have multiple active viewports.
    // - In the future we will extend this concept further to also represent Platform Monitor and support a "no main platform window" operation mode.
    LINEARUI_API LinearUIViewport* GetMainViewport();                                                 // return primary/default viewport. This can never be NULL.

    // Background/Foreground Draw Lists
    LINEARUI_API LinearDrawList*   GetBackgroundDrawList(LinearUIViewport* viewport = NULL);              // get background draw list for the given viewport or viewport associated to the current window. this draw list will be the first rendering one. Useful to quickly draw shapes/text behind dear linearui contents.
    LINEARUI_API LinearDrawList*   GetForegroundDrawList(LinearUIViewport* viewport = NULL);              // get foreground draw list for the given viewport or viewport associated to the current window. this draw list will be the top-most rendered one. Useful to quickly draw shapes/text over dear linearui contents.

    // Miscellaneous Utilities
    LINEARUI_API bool          IsRectVisible(const LinearVec2& size);                                  // test if rectangle (of given size, starting from cursor position) is visible / not clipped.
    LINEARUI_API bool          IsRectVisible(const LinearVec2& rect_min, const LinearVec2& rect_max);      // test if rectangle (in screen space) is visible / not clipped. to perform coarse clipping on user's side.
    LINEARUI_API double        GetTime();                                                          // get global linearui time. incremented by io.DeltaTime every frame.
    LINEARUI_API int           GetFrameCount();                                                    // get global linearui frame count. incremented by 1 every frame.
    LINEARUI_API LinearDrawListSharedData* GetDrawListSharedData();                                    // you may use this when creating your own LinearDrawList instances.
    LINEARUI_API const char*   GetStyleColorName(LinearUICol idx);                                    // get a string corresponding to the enum value (for display, saving, etc.).
    LINEARUI_API void          SetStateStorage(LinearUIStorage* storage);                             // replace current window storage with our own (if you want to manipulate it yourself, typically clear subsection of it)
    LINEARUI_API LinearUIStorage* GetStateStorage();

    // Text Utilities
    LINEARUI_API LinearVec2        CalcTextSize(const char* text, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f);

    // Color Utilities
    LINEARUI_API LinearVec4        ColorConvertU32ToFloat4(LinearU32 in);
    LINEARUI_API LinearU32         ColorConvertFloat4ToU32(const LinearVec4& in);
    LINEARUI_API void          ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s, float& out_v);
    LINEARUI_API void          ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b);

    // Inputs Utilities: Raw Keyboard/Mouse/Gamepad Access
    // - Consider using the Shortcut() function instead of IsKeyPressed()/IsKeyChordPressed()! Shortcut() is easier to use and better featured (can do focus routing check).
    // - the LinearUIKey enum contains all possible keyboard, mouse and gamepad inputs (e.g. LinearUIKey_A, LinearUIKey_MouseLeft, LinearUIKey_GamepadDpadUp...).
    // - (legacy: before v1.87 (2022-02), we used LinearUIKey < 512 values to carry native/user indices as defined by each backends. This was obsoleted in 1.87 (2022-02) and completely removed in 1.91.5 (2024-11). See https://github.com/ocornut/linearui/issues/4921)
    LINEARUI_API bool          IsKeyDown(LinearUIKey key);                                            // is key being held.
    LINEARUI_API bool          IsKeyPressed(LinearUIKey key, bool repeat = true);                     // was key pressed (went from !Down to Down)? Repeat rate uses io.KeyRepeatDelay / KeyRepeatRate.
    LINEARUI_API bool          IsKeyReleased(LinearUIKey key);                                        // was key released (went from Down to !Down)?
    LINEARUI_API bool          IsKeyChordPressed(LinearUIKeyChord key_chord);                         // was key chord (mods + key) pressed, e.g. you can pass 'LinearUIMod_Ctrl | LinearUIKey_S' as a key-chord. This doesn't do any routing or focus check, please consider using Shortcut() function instead.
    LINEARUI_API int           GetKeyPressedAmount(LinearUIKey key, float repeat_delay, float rate);  // uses provided repeat rate/delay. return a count, most often 0 or 1 but might be >1 if RepeatRate is small enough that DeltaTime > RepeatRate
    LINEARUI_API const char*   GetKeyName(LinearUIKey key);                                           // [DEBUG] returns English name of the key. Those names are provided for debugging purpose and are not meant to be saved persistently nor compared.
    LINEARUI_API void          SetNextFrameWantCaptureKeyboard(bool want_capture_keyboard);        // Override io.WantCaptureKeyboard flag next frame (said flag is left for your application to handle, typically when true it instructs your app to ignore inputs). e.g. force capture keyboard when your widget is being hovered. This is equivalent to setting "io.WantCaptureKeyboard = want_capture_keyboard"; after the next NewFrame() call.

    // Inputs Utilities: Shortcut Testing & Routing
    // - Typical use is e.g.: 'if (LinearUI::Shortcut(LinearUIMod_Ctrl | LinearUIKey_S)) { ... }'.
    // - Flags: Default route use LinearUIInputFlags_RouteFocused, but see LinearUIInputFlags_RouteGlobal and other options in LinearUIInputFlags_!
    // - Flags: Use LinearUIInputFlags_Repeat to support repeat.
    // - LinearUIKeyChord = a LinearUIKey + optional LinearUIMod_Alt/LinearUIMod_Ctrl/LinearUIMod_Shift/LinearUIMod_Super.
    //       LinearUIKey_C                          // Accepted by functions taking LinearUIKey or LinearUIKeyChord arguments
    //       LinearUIMod_Ctrl | LinearUIKey_C          // Accepted by functions taking LinearUIKeyChord arguments
    //   only LinearUIMod_XXX values are legal to combine with an LinearUIKey. You CANNOT combine two LinearUIKey values.
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
    LINEARUI_API bool          Shortcut(LinearUIKeyChord key_chord, LinearUIInputFlags flags = 0);
    LINEARUI_API void          SetNextItemShortcut(LinearUIKeyChord key_chord, LinearUIInputFlags flags = 0);

    // Inputs Utilities: Key/Input Ownership [BETA]
    // - One common use case would be to allow your items to disable standard inputs behaviors such
    //   as Tab or Alt key handling, Mouse Wheel scrolling, etc.
    //   e.g. `Button(...); if (SetItemKeyOwner(LinearUIKey_MouseWheelY)) { ... }` to make hovering/activating a button disable wheel for scrolling.
    // - Reminder LinearUIKey enum include access to mouse buttons and gamepad, so key ownership can apply to them.
    // - The return value of SetItemKeyOwner() says if ownership has been requested for the item, which is a shortcut to calling yet non-public TestKeyOwner() function.
    // - Many related features are still in linearui_internal.h. For instance, most IsKeyXXX()/IsMouseXXX() functions have an owner-id-aware version.
    LINEARUI_API bool          SetItemKeyOwner(LinearUIKey key);                                      // Set key owner to last item ID if it is hovered or active. Return true when ownership has been set. Roughly equivalent to 'if (TestKeyOwner(key, GetItemID()) && (IsItemHovered() || IsItemActive())) { SetKeyOwner(key, GetItemID());'. 

    // Inputs Utilities: Mouse
    // - To refer to a mouse button, you may use named enums in your code e.g. LinearUIMouseButton_Left, LinearUIMouseButton_Right.
    // - You can also use regular integer: it is forever guaranteed that 0=Left, 1=Right, 2=Middle.
    // - Dragging operations are only reported after mouse has moved a certain distance away from the initial clicking position (see 'lock_threshold' and 'io.MouseDraggingThreshold')
    LINEARUI_API bool          IsMouseDown(LinearUIMouseButton button);                               // is mouse button held?
    LINEARUI_API bool          IsMouseClicked(LinearUIMouseButton button, bool repeat = false);       // did mouse button clicked? (went from !Down to Down). Same as GetMouseClickedCount() == 1.
    LINEARUI_API bool          IsMouseReleased(LinearUIMouseButton button);                           // did mouse button released? (went from Down to !Down)
    LINEARUI_API bool          IsMouseDoubleClicked(LinearUIMouseButton button);                      // did mouse button double-clicked? Same as GetMouseClickedCount() == 2. (note that a double-click will also report IsMouseClicked() == true)
    LINEARUI_API bool          IsMouseReleasedWithDelay(LinearUIMouseButton button, float delay);     // delayed mouse release (use very sparingly!). Generally used with 'delay >= io.MouseDoubleClickTime' + combined with a 'io.MouseClickedLastCount==1' test. This is a very rarely used UI idiom, but some apps use this: e.g. MS Explorer single click on an icon to rename.
    LINEARUI_API int           GetMouseClickedCount(LinearUIMouseButton button);                      // return the number of successive mouse-clicks at the time where a click happen (otherwise 0).
    LINEARUI_API bool          IsMouseHoveringRect(const LinearVec2& r_min, const LinearVec2& r_max, bool clip = true);// is mouse hovering given bounding rect (in screen space). clipped by current clipping settings, but disregarding of other consideration of focus/window ordering/popup-block.
    LINEARUI_API bool          IsMousePosValid(const LinearVec2* mouse_pos = NULL);                    // by convention we use (-FLT_MAX,-FLT_MAX) to denote that there is no mouse available
    LINEARUI_API bool          IsAnyMouseDown();                                                   // [WILL OBSOLETE] is any mouse button held? This was designed for backends, but prefer having backend maintain a mask of held mouse buttons, because upcoming input queue system will make this invalid.
    LINEARUI_API LinearVec2        GetMousePos();                                                      // shortcut to LinearUI::GetIO().MousePos provided by user, to be consistent with other calls
    LINEARUI_API LinearVec2        GetMousePosOnOpeningCurrentPopup();                                 // retrieve mouse position at the time of opening popup we have BeginPopup() into (helper to avoid user backing that value themselves)
    LINEARUI_API bool          IsMouseDragging(LinearUIMouseButton button, float lock_threshold = -1.0f);         // is mouse dragging? (uses io.MouseDraggingThreshold if lock_threshold < 0.0f)
    LINEARUI_API LinearVec2        GetMouseDragDelta(LinearUIMouseButton button = 0, float lock_threshold = -1.0f);   // return the delta from the initial clicking position while the mouse button is pressed or was just released. This is locked and return 0.0f until the mouse moves past a distance threshold at least once (uses io.MouseDraggingThreshold if lock_threshold < 0.0f)
    LINEARUI_API void          ResetMouseDragDelta(LinearUIMouseButton button = 0);                   //
    LINEARUI_API LinearUIMouseCursor GetMouseCursor();                                                // get desired mouse cursor shape. Important: reset in LinearUI::NewFrame(), this is updated during the frame. valid before Render(). If you use software rendering by setting io.MouseDrawCursor LinearUI will render those for you
    LINEARUI_API void          SetMouseCursor(LinearUIMouseCursor cursor_type);                       // set desired mouse cursor shape
    LINEARUI_API void          SetNextFrameWantCaptureMouse(bool want_capture_mouse);              // Override io.WantCaptureMouse flag next frame (said flag is left for your application to handle, typical when true it instructs your app to ignore inputs). This is equivalent to setting "io.WantCaptureMouse = want_capture_mouse;" after the next NewFrame() call.

    // Clipboard Utilities
    // - Also see the LogToClipboard() function to capture GUI into clipboard, or easily output text data to the clipboard.
    LINEARUI_API const char*   GetClipboardText();
    LINEARUI_API void          SetClipboardText(const char* text);

    // Settings/.Ini Utilities
    // - The disk functions are automatically called if io.IniFilename != NULL (default is "linearui.ini").
    // - Set io.IniFilename to NULL to load/save manually. Read io.WantSaveIniSettings description about handling .ini saving manually.
    // - Important: default value "linearui.ini" is relative to current working dir! Most apps will want to lock this to an absolute path (e.g. same path as executables).
    LINEARUI_API void          LoadIniSettingsFromDisk(const char* ini_filename);                  // call after CreateContext() and before the first call to NewFrame(). NewFrame() automatically calls LoadIniSettingsFromDisk(io.IniFilename).
    LINEARUI_API void          LoadIniSettingsFromMemory(const char* ini_data, size_t ini_size=0); // call after CreateContext() and before the first call to NewFrame() to provide .ini data from your own data source.
    LINEARUI_API void          SaveIniSettingsToDisk(const char* ini_filename);                    // this is automatically called (if io.IniFilename is not empty) a few seconds after any modification that should be reflected in the .ini file (and also by DestroyContext).
    LINEARUI_API const char*   SaveIniSettingsToMemory(size_t* out_ini_size = NULL);               // return a zero-terminated string with the .ini data which you can save by your own mean. call when io.WantSaveIniSettings is set, then save data by your own mean and clear io.WantSaveIniSettings.

    // Debug Utilities
    // - Your main debugging friend is the ShowMetricsWindow() function.
    // - Interactive tools are all accessible from the 'Dear LinearUI Demo->Tools' menu.
    // - Read https://github.com/ocornut/linearui/wiki/Debug-Tools for a description of all available debug tools.
    LINEARUI_API void          DebugTextEncoding(const char* text);
    LINEARUI_API void          DebugFlashStyleColor(LinearUICol idx);
    LINEARUI_API void          DebugStartItemPicker();
    LINEARUI_API bool          DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_drawvert, size_t sz_drawidx); // This is called by LINEARUI_CHECKVERSION() macro.
#ifndef LINEARUI_DISABLE_DEBUG_TOOLS
    LINEARUI_API void          DebugLog(const char* fmt, ...)           LINEARUI_FMTARGS(1); // Call via LINEARUI_DEBUG_LOG() for maximum stripping in caller code!
    LINEARUI_API void          DebugLogV(const char* fmt, va_list args) LINEARUI_FMTLIST(1);
#endif

    // Memory Allocators
    // - Those functions are not reliant on the current context.
    // - DLL users: heaps and globals are not shared across DLL boundaries! You will need to call SetCurrentContext() + SetAllocatorFunctions()
    //   for each static/DLL boundary you are calling from. Read "Context and Memory Allocators" section of linearui.cpp for more details.
    LINEARUI_API void          SetAllocatorFunctions(LinearUIMemAllocFunc alloc_func, LinearUIMemFreeFunc free_func, void* user_data = NULL);
    LINEARUI_API void          GetAllocatorFunctions(LinearUIMemAllocFunc* p_alloc_func, LinearUIMemFreeFunc* p_free_func, void** p_user_data);
    LINEARUI_API void*         MemAlloc(size_t size);
    LINEARUI_API void          MemFree(void* ptr);

    // (Optional) Platform/OS interface for multi-viewport support
    // Read comments around the LinearUIPlatformIO structure for more details.
    // Note: You may use GetWindowViewport() to get the current viewport of the current window.
    LINEARUI_API void          UpdatePlatformWindows();                                // call in main loop. will call CreateWindow/ResizeWindow/etc. platform functions for each secondary viewport, and DestroyWindow for each inactive viewport.
    LINEARUI_API void          RenderPlatformWindowsDefault(void* platform_render_arg = NULL, void* renderer_render_arg = NULL); // call in main loop. will call RenderWindow/SwapBuffers platform functions for each secondary viewport which doesn't have the LinearUIViewportFlags_Minimized flag set. May be reimplemented by user for custom rendering needs.
    LINEARUI_API void          DestroyPlatformWindows();                               // call DestroyWindow platform functions for all viewports. call from backend Shutdown() if you need to close platform windows before linearui shutdown. otherwise will be called by DestroyContext().
    LINEARUI_API LinearUIViewport* FindViewportByID(LinearUIID viewport_id);                 // this is a helper for backends.
    LINEARUI_API LinearUIViewport* FindViewportByPlatformHandle(void* platform_handle);   // this is a helper for backends. the type platform_handle is decided by the backend (e.g. HWND, MyWindow*, LINEARWINwindow* etc.)

} // namespace LinearUI

//-----------------------------------------------------------------------------
// [SECTION] Flags & Enumerations
//-----------------------------------------------------------------------------

// Flags for LinearUI::Begin()
// (Those are per-window flags. There are shared flags in LinearUIIO: io.ConfigWindowsResizeFromEdges and io.ConfigWindowsMoveFromTitleBarOnly)
enum LinearUIWindowFlags_
{
    LinearUIWindowFlags_None                   = 0,
    LinearUIWindowFlags_NoTitleBar             = 1 << 0,   // Disable title-bar
    LinearUIWindowFlags_NoResize               = 1 << 1,   // Disable user resizing with the lower-right grip
    LinearUIWindowFlags_NoMove                 = 1 << 2,   // Disable user moving the window
    LinearUIWindowFlags_NoScrollbar            = 1 << 3,   // Disable scrollbars (window can still scroll with mouse or programmatically)
    LinearUIWindowFlags_NoScrollWithMouse      = 1 << 4,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
    LinearUIWindowFlags_NoCollapse             = 1 << 5,   // Disable user collapsing window by double-clicking on it. Also referred to as Window Menu Button (e.g. within a docking node).
    LinearUIWindowFlags_AlwaysAutoResize       = 1 << 6,   // Resize every window to its content every frame
    LinearUIWindowFlags_NoBackground           = 1 << 7,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
    LinearUIWindowFlags_NoSavedSettings        = 1 << 8,   // Never load/save settings in .ini file
    LinearUIWindowFlags_NoMouseInputs          = 1 << 9,   // Disable catching mouse, hovering test with pass through.
    LinearUIWindowFlags_MenuBar                = 1 << 10,  // Has a menu-bar
    LinearUIWindowFlags_HorizontalScrollbar    = 1 << 11,  // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(LinearVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in linearui_demo in the "Horizontal Scrolling" section.
    LinearUIWindowFlags_NoFocusOnAppearing     = 1 << 12,  // Disable taking focus when transitioning from hidden to visible state
    LinearUIWindowFlags_NoBringToFrontOnFocus  = 1 << 13,  // Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)
    LinearUIWindowFlags_AlwaysVerticalScrollbar= 1 << 14,  // Always show vertical scrollbar (even if ContentSize.y < Size.y)
    LinearUIWindowFlags_AlwaysHorizontalScrollbar=1<< 15,  // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
    LinearUIWindowFlags_NoNavInputs            = 1 << 16,  // No keyboard/gamepad navigation within the window
    LinearUIWindowFlags_NoNavFocus             = 1 << 17,  // No focusing toward this window with keyboard/gamepad navigation (e.g. skipped by Ctrl+Tab)
    LinearUIWindowFlags_UnsavedDocument        = 1 << 18,  // Display a dot next to the title. When used in a tab/docking context, tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
    LinearUIWindowFlags_NoDocking              = 1 << 19,  // Disable docking of this window
    LinearUIWindowFlags_NoNav                  = LinearUIWindowFlags_NoNavInputs | LinearUIWindowFlags_NoNavFocus,
    LinearUIWindowFlags_NoDecoration           = LinearUIWindowFlags_NoTitleBar | LinearUIWindowFlags_NoResize | LinearUIWindowFlags_NoScrollbar | LinearUIWindowFlags_NoCollapse,
    LinearUIWindowFlags_NoInputs               = LinearUIWindowFlags_NoMouseInputs | LinearUIWindowFlags_NoNavInputs | LinearUIWindowFlags_NoNavFocus,

    // [Internal]
    LinearUIWindowFlags_DockNodeHost           = 1 << 23,  // Don't use! For internal use by Begin()/NewFrame()
    LinearUIWindowFlags_ChildWindow            = 1 << 24,  // Don't use! For internal use by BeginChild()
    LinearUIWindowFlags_Tooltip                = 1 << 25,  // Don't use! For internal use by BeginTooltip()
    LinearUIWindowFlags_Popup                  = 1 << 26,  // Don't use! For internal use by BeginPopup()
    LinearUIWindowFlags_Modal                  = 1 << 27,  // Don't use! For internal use by BeginPopupModal()
    LinearUIWindowFlags_ChildMenu              = 1 << 28,  // Don't use! For internal use by BeginMenu()

    // Obsolete names
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    //LinearUIWindowFlags_NavFlattened           = 1 << 29,  // Obsoleted in 1.90.9: moved to LinearUIChildFlags. BeginChild(name, size, 0, LinearUIWindowFlags_NavFlattened)           --> BeginChild(name, size, LinearUIChildFlags_NavFlattened, 0)
    //LinearUIWindowFlags_AlwaysUseWindowPadding = 1 << 30,  // Obsoleted in 1.90.0: moved to LinearUIChildFlags. BeginChild(name, size, 0, LinearUIWindowFlags_AlwaysUseWindowPadding) --> BeginChild(name, size, LinearUIChildFlags_AlwaysUseWindowPadding, 0)
#endif
};

// Flags for LinearUI::BeginChild()
// (Legacy: bit 0 must always correspond to LinearUIChildFlags_Borders to be backward compatible with old API using 'bool border = false'.)
// About using AutoResizeX/AutoResizeY flags:
// - May be combined with SetNextWindowSizeConstraints() to set a min/max size for each axis (see "Demo->Child->Auto-resize with Constraints").
// - Size measurement for a given axis is only performed when the child window is within visible boundaries, or is just appearing.
//   - This allows BeginChild() to return false when not within boundaries (e.g. when scrolling), which is more optimal. BUT it won't update its auto-size while clipped.
//     While not perfect, it is a better default behavior as the always-on performance gain is more valuable than the occasional "resizing after becoming visible again" glitch.
//   - You may also use LinearUIChildFlags_AlwaysAutoResize to force an update even when child window is not in view.
//     HOWEVER PLEASE UNDERSTAND THAT DOING SO WILL PREVENT BeginChild() FROM EVER RETURNING FALSE, disabling benefits of coarse clipping.
enum LinearUIChildFlags_
{
    LinearUIChildFlags_None                    = 0,
    LinearUIChildFlags_Borders                 = 1 << 0,   // Show an outer border and enable WindowPadding. (IMPORTANT: this is always == 1 == true for legacy reason)
    LinearUIChildFlags_AlwaysUseWindowPadding  = 1 << 1,   // Pad with style.WindowPadding even if no border are drawn (no padding by default for non-bordered child windows because it makes more sense)
    LinearUIChildFlags_ResizeX                 = 1 << 2,   // Allow resize from right border (layout direction). Enable .ini saving (unless LinearUIWindowFlags_NoSavedSettings passed to window flags)
    LinearUIChildFlags_ResizeY                 = 1 << 3,   // Allow resize from bottom border (layout direction). "
    LinearUIChildFlags_AutoResizeX             = 1 << 4,   // Enable auto-resizing width. Read "IMPORTANT: Size measurement" details above.
    LinearUIChildFlags_AutoResizeY             = 1 << 5,   // Enable auto-resizing height. Read "IMPORTANT: Size measurement" details above.
    LinearUIChildFlags_AlwaysAutoResize        = 1 << 6,   // Combined with AutoResizeX/AutoResizeY. Always measure size even when child is hidden, always return true, always disable clipping optimization! NOT RECOMMENDED.
    LinearUIChildFlags_FrameStyle              = 1 << 7,   // Style the child window like a framed item: use FrameBg, FrameRounding, FrameBorderSize, FramePadding instead of ChildBg, ChildRounding, ChildBorderSize, WindowPadding.
    LinearUIChildFlags_NavFlattened            = 1 << 8,   // [BETA] Share focus scope, allow keyboard/gamepad navigation to cross over parent border to this child or between sibling child windows.

    // Obsolete names
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    //LinearUIChildFlags_Border                = LinearUIChildFlags_Borders,  // Renamed in 1.91.1 (August 2024) for consistency.
#endif
};

// Flags for LinearUI::PushItemFlag()
// (Those are shared by all submitted items)
enum LinearUIItemFlags_
{
    LinearUIItemFlags_None                     = 0,        // (Default)
    LinearUIItemFlags_NoTabStop                = 1 << 0,   // false    // Disable keyboard tabbing. This is a "lighter" version of LinearUIItemFlags_NoNav.
    LinearUIItemFlags_NoNav                    = 1 << 1,   // false    // Disable any form of focusing (keyboard/gamepad directional navigation and SetKeyboardFocusHere() calls).
    LinearUIItemFlags_NoNavDefaultFocus        = 1 << 2,   // false    // Disable item being a candidate for default focus (e.g. used by title bar items).
    LinearUIItemFlags_ButtonRepeat             = 1 << 3,   // false    // Any button-like behavior will have repeat mode enabled (based on io.KeyRepeatDelay and io.KeyRepeatRate values). Note that you can also call IsItemActive() after any button to tell if it is being held.
    LinearUIItemFlags_AutoClosePopups          = 1 << 4,   // true     // MenuItem()/Selectable() automatically close their parent popup window.
    LinearUIItemFlags_AllowDuplicateId         = 1 << 5,   // false    // Allow submitting an item with the same identifier as an item already submitted this frame without triggering a warning tooltip if io.ConfigDebugHighlightIdConflicts is set.
    LinearUIItemFlags_Disabled                 = 1 << 6,   // false    // [Internal] Disable interactions. DOES NOT affect visuals. This is used by BeginDisabled()/EndDisabled() and only provided here so you can read back via GetItemFlags().
};

// Flags for LinearUI::InputText()
// (Those are per-item flags. There are shared flags in LinearUIIO: io.ConfigInputTextCursorBlink and io.ConfigInputTextEnterKeepActive)
enum LinearUIInputTextFlags_
{
    // Basic filters (also see LinearUIInputTextFlags_CallbackCharFilter)
    LinearUIInputTextFlags_None                = 0,
    LinearUIInputTextFlags_CharsDecimal        = 1 << 0,   // Allow 0123456789.+-*/
    LinearUIInputTextFlags_CharsHexadecimal    = 1 << 1,   // Allow 0123456789ABCDEFabcdef
    LinearUIInputTextFlags_CharsScientific     = 1 << 2,   // Allow 0123456789.+-*/eE (Scientific notation input)
    LinearUIInputTextFlags_CharsUppercase      = 1 << 3,   // Turn a..z into A..Z
    LinearUIInputTextFlags_CharsNoBlank        = 1 << 4,   // Filter out spaces, tabs

    // Inputs
    LinearUIInputTextFlags_AllowTabInput       = 1 << 5,   // Pressing TAB input a '\t' character into the text field
    LinearUIInputTextFlags_EnterReturnsTrue    = 1 << 6,   // Return 'true' when Enter is pressed (as opposed to every time the value was modified). Consider using IsItemDeactivatedAfterEdit() instead!
    LinearUIInputTextFlags_EscapeClearsAll     = 1 << 7,   // Escape key clears content if not empty, and deactivate otherwise (contrast to default behavior of Escape to revert)
    LinearUIInputTextFlags_CtrlEnterForNewLine = 1 << 8,   // In multi-line mode: validate with Enter, add new line with Ctrl+Enter (default is opposite: validate with Ctrl+Enter, add line with Enter). Note that Shift+Enter always enter a new line either way.

    // Other options
    LinearUIInputTextFlags_ReadOnly            = 1 << 9,   // Read-only mode
    LinearUIInputTextFlags_Password            = 1 << 10,  // Password mode, display all characters as '*', disable copy
    LinearUIInputTextFlags_AlwaysOverwrite     = 1 << 11,  // Overwrite mode
    LinearUIInputTextFlags_AutoSelectAll       = 1 << 12,  // Select entire text when first taking mouse focus
    LinearUIInputTextFlags_ParseEmptyRefVal    = 1 << 13,  // InputFloat(), InputInt(), InputScalar() etc. only: parse empty string as zero value.
    LinearUIInputTextFlags_DisplayEmptyRefVal  = 1 << 14,  // InputFloat(), InputInt(), InputScalar() etc. only: when value is zero, do not display it. Generally used with LinearUIInputTextFlags_ParseEmptyRefVal.
    LinearUIInputTextFlags_NoHorizontalScroll  = 1 << 15,  // Disable following the cursor horizontally
    LinearUIInputTextFlags_NoUndoRedo          = 1 << 16,  // Disable undo/redo. Note that input text owns the text data while active, if you want to provide your own undo/redo stack you need e.g. to call ClearActiveID().

    // Elide display / Alignment
    LinearUIInputTextFlags_ElideLeft           = 1 << 17,  // When text doesn't fit, elide left side to ensure right side stays visible. Useful for path/filenames. Single-line only!

    // Callback features
    LinearUIInputTextFlags_CallbackCompletion  = 1 << 18,  // Callback on pressing TAB (for completion handling)
    LinearUIInputTextFlags_CallbackHistory     = 1 << 19,  // Callback on pressing Up/Down arrows (for history handling)
    LinearUIInputTextFlags_CallbackAlways      = 1 << 20,  // Callback on each iteration. User code may query cursor position, modify text buffer.
    LinearUIInputTextFlags_CallbackCharFilter  = 1 << 21,  // Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
    LinearUIInputTextFlags_CallbackResize      = 1 << 22,  // Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow. Notify when the string wants to be resized (for string types which hold a cache of their Size). You will be provided a new BufSize in the callback and NEED to honor it. (see misc/cpp/linearui_stdlib.h for an example of using this)
    LinearUIInputTextFlags_CallbackEdit        = 1 << 23,  // Callback on any edit. Note that InputText() already returns true on edit + you can always use IsItemEdited(). The callback is useful to manipulate the underlying buffer while focus is active.

    // Multi-line Word-Wrapping [BETA]
    // - Not well tested yet. Please report any incorrect cursor movement, selection behavior etc. bug to https://github.com/ocornut/linearui/issues/3237.
    // - Wrapping style is not ideal. Wrapping of long words/sections (e.g. words larger than total available width) may be particularly unpleasing.
    // - Wrapping width needs to always account for the possibility of a vertical scrollbar.
    // - It is much slower than regular text fields.
    //   Ballpark estimate of cost on my 2019 desktop PC: for a 100 KB text buffer: +~0.3 ms (Optimized) / +~1.0 ms (Debug build).
    //   The CPU cost is very roughly proportional to text length, so a 10 KB buffer should cost about ten times less.
    LinearUIInputTextFlags_WordWrap            = 1 << 24,  // InputTextMultiline(): word-wrap lines that are too long.

    // Obsolete names
    //LinearUIInputTextFlags_AlwaysInsertMode  = LinearUIInputTextFlags_AlwaysOverwrite   // [renamed in 1.82] name was not matching behavior
};

// Flags for LinearUI::TreeNodeEx(), LinearUI::CollapsingHeader*()
enum LinearUITreeNodeFlags_
{
    LinearUITreeNodeFlags_None                 = 0,
    LinearUITreeNodeFlags_Selected             = 1 << 0,   // Draw as selected
    LinearUITreeNodeFlags_Framed               = 1 << 1,   // Draw frame with background (e.g. for CollapsingHeader)
    LinearUITreeNodeFlags_AllowOverlap         = 1 << 2,   // Hit testing will allow subsequent widgets to overlap this one. Require previous frame HoveredId to match before being usable. Shortcut to calling SetNextItemAllowOverlap().
    LinearUITreeNodeFlags_NoTreePushOnOpen     = 1 << 3,   // Don't do a TreePush() when open (e.g. for CollapsingHeader) = no extra indent nor pushing on ID stack
    LinearUITreeNodeFlags_NoAutoOpenOnLog      = 1 << 4,   // Don't automatically and temporarily open node when Logging is active (by default logging will automatically open tree nodes)
    LinearUITreeNodeFlags_DefaultOpen          = 1 << 5,   // Default node to be open
    LinearUITreeNodeFlags_OpenOnDoubleClick    = 1 << 6,   // Open on double-click instead of simple click (default for multi-select unless any _OpenOnXXX behavior is set explicitly). Both behaviors may be combined.
    LinearUITreeNodeFlags_OpenOnArrow          = 1 << 7,   // Open when clicking on the arrow part (default for multi-select unless any _OpenOnXXX behavior is set explicitly). Both behaviors may be combined.
    LinearUITreeNodeFlags_Leaf                 = 1 << 8,   // No collapsing, no arrow (use as a convenience for leaf nodes). Note: will always open a tree/id scope and return true. If you never use that scope, add LinearUITreeNodeFlags_NoTreePushOnOpen.
    LinearUITreeNodeFlags_Bullet               = 1 << 9,   // Display a bullet instead of arrow. IMPORTANT: node can still be marked open/close if you don't set the _Leaf flag!
    LinearUITreeNodeFlags_FramePadding         = 1 << 10,  // Use FramePadding (even for an unframed text node) to vertically align text baseline to regular widget height. Equivalent to calling AlignTextToFramePadding() before the node.
    LinearUITreeNodeFlags_SpanAvailWidth       = 1 << 11,  // Extend hit box to the right-most edge, even if not framed. This is not the default in order to allow adding other items on the same line without using AllowOverlap mode.
    LinearUITreeNodeFlags_SpanFullWidth        = 1 << 12,  // Extend hit box to the left-most and right-most edges (cover the indent area).
    LinearUITreeNodeFlags_SpanLabelWidth       = 1 << 13,  // Narrow hit box + narrow hovering highlight, will only cover the label text.
    LinearUITreeNodeFlags_SpanAllColumns       = 1 << 14,  // Frame will span all columns of its container table (label will still fit in current column)
    LinearUITreeNodeFlags_LabelSpanAllColumns  = 1 << 15,  // Label will span all columns of its container table
    //LinearUITreeNodeFlags_NoScrollOnOpen     = 1 << 16,  // FIXME: TODO: Disable automatic scroll on TreePop() if node got just open and contents is not visible
    LinearUITreeNodeFlags_NavLeftJumpsToParent = 1 << 17,  // Nav: left arrow moves back to parent. This is processed in TreePop() when there's an unfulfilled Left nav request remaining.
    LinearUITreeNodeFlags_CollapsingHeader     = LinearUITreeNodeFlags_Framed | LinearUITreeNodeFlags_NoTreePushOnOpen | LinearUITreeNodeFlags_NoAutoOpenOnLog,

    // [EXPERIMENTAL] Draw lines connecting TreeNode hierarchy. Discuss in GitHub issue #2920.
    // Default value is pulled from style.TreeLinesFlags. May be overridden in TreeNode calls.
    LinearUITreeNodeFlags_DrawLinesNone        = 1 << 18,  // No lines drawn
    LinearUITreeNodeFlags_DrawLinesFull        = 1 << 19,  // Horizontal lines to child nodes. Vertical line drawn down to TreePop() position: cover full contents. Faster (for large trees).
    LinearUITreeNodeFlags_DrawLinesToNodes     = 1 << 20,  // Horizontal lines to child nodes. Vertical line drawn down to bottom-most child node. Slower (for large trees).

#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    LinearUITreeNodeFlags_NavLeftJumpsBackHere = LinearUITreeNodeFlags_NavLeftJumpsToParent,  // Renamed in 1.92.0
    //LinearUITreeNodeFlags_SpanTextWidth      = LinearUITreeNodeFlags_SpanLabelWidth,        // Renamed in 1.90.7
    //LinearUITreeNodeFlags_AllowItemOverlap   = LinearUITreeNodeFlags_AllowOverlap,          // Renamed in 1.89.7
#endif
};

// Flags for OpenPopup*(), BeginPopupContext*(), IsPopupOpen() functions.
// - IMPORTANT: If you ever used the left mouse button with BeginPopupContextXXX() helpers before 1.92.6: Read "API BREAKING CHANGES" 2026/01/07 (1.92.6) entry in linearui.cpp or GitHub topic #9157.
// - Multiple buttons currently cannot be combined/or-ed in those functions (we could allow it later).
enum LinearUIPopupFlags_
{
    LinearUIPopupFlags_None                    = 0,
    LinearUIPopupFlags_MouseButtonLeft         = 1 << 2,   // For BeginPopupContext*(): open on Left Mouse release. Only one button allowed!
    LinearUIPopupFlags_MouseButtonRight        = 2 << 2,   // For BeginPopupContext*(): open on Right Mouse release. Only one button allowed! (default)
    LinearUIPopupFlags_MouseButtonMiddle       = 3 << 2,   // For BeginPopupContext*(): open on Middle Mouse release. Only one button allowed!
    LinearUIPopupFlags_NoReopen                = 1 << 5,   // For OpenPopup*(), BeginPopupContext*(): don't reopen same popup if already open (won't reposition, won't reinitialize navigation)
    //LinearUIPopupFlags_NoReopenAlwaysNavInit = 1 << 6,   // For OpenPopup*(), BeginPopupContext*(): focus and initialize navigation even when not reopening.
    LinearUIPopupFlags_NoOpenOverExistingPopup = 1 << 7,   // For OpenPopup*(), BeginPopupContext*(): don't open if there's already a popup at the same level of the popup stack
    LinearUIPopupFlags_NoOpenOverItems         = 1 << 8,   // For BeginPopupContextWindow(): don't return true when hovering items, only when hovering empty space
    LinearUIPopupFlags_AnyPopupId              = 1 << 10,  // For IsPopupOpen(): ignore the LinearUIID parameter and test for any popup.
    LinearUIPopupFlags_AnyPopupLevel           = 1 << 11,  // For IsPopupOpen(): search/test at any level of the popup stack (default test in the current level)
    LinearUIPopupFlags_AnyPopup                = LinearUIPopupFlags_AnyPopupId | LinearUIPopupFlags_AnyPopupLevel,
    LinearUIPopupFlags_MouseButtonShift_       = 2,        // [Internal]
    LinearUIPopupFlags_MouseButtonMask_        = 0x0C,     // [Internal]
    LinearUIPopupFlags_InvalidMask_            = 0x03,     // [Internal] Reserve legacy bits 0-1 to detect incorrectly passing 1 or 2 to the function.
};

// Flags for LinearUI::Selectable()
enum LinearUISelectableFlags_
{
    LinearUISelectableFlags_None               = 0,
    LinearUISelectableFlags_NoAutoClosePopups  = 1 << 0,   // Clicking this doesn't close parent popup window (overrides LinearUIItemFlags_AutoClosePopups)
    LinearUISelectableFlags_SpanAllColumns     = 1 << 1,   // Frame will span all columns of its container table (text will still fit in current column)
    LinearUISelectableFlags_AllowDoubleClick   = 1 << 2,   // Generate press events on double clicks too
    LinearUISelectableFlags_Disabled           = 1 << 3,   // Cannot be selected, display grayed out text
    LinearUISelectableFlags_AllowOverlap       = 1 << 4,   // Hit testing will allow subsequent widgets to overlap this one. Require previous frame HoveredId to match before being usable. Shortcut to calling SetNextItemAllowOverlap().
    LinearUISelectableFlags_Highlight          = 1 << 5,   // Make the item be displayed as if it is hovered
    LinearUISelectableFlags_SelectOnNav        = 1 << 6,   // Auto-select when moved into, unless Ctrl is held. Automatic when in a BeginMultiSelect() block.

#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    LinearUISelectableFlags_DontClosePopups    = LinearUISelectableFlags_NoAutoClosePopups,   // Renamed in 1.91.0
    //LinearUISelectableFlags_AllowItemOverlap = LinearUISelectableFlags_AllowOverlap,        // Renamed in 1.89.7
#endif
};

// Flags for LinearUI::BeginCombo()
enum LinearUIComboFlags_
{
    LinearUIComboFlags_None                    = 0,
    LinearUIComboFlags_PopupAlignLeft          = 1 << 0,   // Align the popup toward the left by default
    LinearUIComboFlags_HeightSmall             = 1 << 1,   // Max ~4 items visible. Tip: If you want your combo popup to be a specific size you can use SetNextWindowSizeConstraints() prior to calling BeginCombo()
    LinearUIComboFlags_HeightRegular           = 1 << 2,   // Max ~8 items visible (default)
    LinearUIComboFlags_HeightLarge             = 1 << 3,   // Max ~20 items visible
    LinearUIComboFlags_HeightLargest           = 1 << 4,   // As many fitting items as possible
    LinearUIComboFlags_NoArrowButton           = 1 << 5,   // Display on the preview box without the square arrow button
    LinearUIComboFlags_NoPreview               = 1 << 6,   // Display only a square arrow button
    LinearUIComboFlags_WidthFitPreview         = 1 << 7,   // Width dynamically calculated from preview contents
    LinearUIComboFlags_HeightMask_             = LinearUIComboFlags_HeightSmall | LinearUIComboFlags_HeightRegular | LinearUIComboFlags_HeightLarge | LinearUIComboFlags_HeightLargest,
};

// Flags for LinearUI::BeginTabBar()
enum LinearUITabBarFlags_
{
    LinearUITabBarFlags_None                           = 0,
    LinearUITabBarFlags_Reorderable                    = 1 << 0,   // Allow manually dragging tabs to re-order them + New tabs are appended at the end of list
    LinearUITabBarFlags_AutoSelectNewTabs              = 1 << 1,   // Automatically select new tabs when they appear
    LinearUITabBarFlags_TabListPopupButton             = 1 << 2,   // Disable buttons to open the tab list popup
    LinearUITabBarFlags_NoCloseWithMiddleMouseButton   = 1 << 3,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You may handle this behavior manually on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
    LinearUITabBarFlags_NoTabListScrollingButtons      = 1 << 4,   // Disable scrolling buttons (apply when fitting policy is LinearUITabBarFlags_FittingPolicyScroll)
    LinearUITabBarFlags_NoTooltip                      = 1 << 5,   // Disable tooltips when hovering a tab
    LinearUITabBarFlags_DrawSelectedOverline           = 1 << 6,   // Draw selected overline markers over selected tab

    // Fitting/Resize policy
    LinearUITabBarFlags_FittingPolicyMixed             = 1 << 7,   // Shrink down tabs when they don't fit, until width is style.TabMinWidthShrink, then enable scrolling. Setting TabMinWidthShrink to FLT_MAX makes this behave like LinearUITabBarFlags_FittingPolicyScroll.
    LinearUITabBarFlags_FittingPolicyShrink            = 1 << 8,   // Shrink down tabs when they don't fit
    LinearUITabBarFlags_FittingPolicyScroll            = 1 << 9,   // Enable scrolling buttons when tabs don't fit
    LinearUITabBarFlags_FittingPolicyMask_             = LinearUITabBarFlags_FittingPolicyMixed | LinearUITabBarFlags_FittingPolicyShrink | LinearUITabBarFlags_FittingPolicyScroll,
    LinearUITabBarFlags_FittingPolicyDefault_          = LinearUITabBarFlags_FittingPolicyMixed,

#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    LinearUITabBarFlags_FittingPolicyResizeDown        = LinearUITabBarFlags_FittingPolicyShrink, // Renamed in 1.92.2
#endif
};

// Flags for LinearUI::BeginTabItem()
enum LinearUITabItemFlags_
{
    LinearUITabItemFlags_None                          = 0,
    LinearUITabItemFlags_UnsavedDocument               = 1 << 0,   // Display a dot next to the title + set LinearUITabItemFlags_NoAssumedClosure.
    LinearUITabItemFlags_SetSelected                   = 1 << 1,   // Trigger flag to programmatically make the tab selected when calling BeginTabItem()
    LinearUITabItemFlags_NoCloseWithMiddleMouseButton  = 1 << 2,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You may handle this behavior manually on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
    LinearUITabItemFlags_NoPushId                      = 1 << 3,   // Don't call PushID()/PopID() on BeginTabItem()/EndTabItem()
    LinearUITabItemFlags_NoTooltip                     = 1 << 4,   // Disable tooltip for the given tab
    LinearUITabItemFlags_NoReorder                     = 1 << 5,   // Disable reordering this tab or having another tab cross over this tab
    LinearUITabItemFlags_Leading                       = 1 << 6,   // Enforce the tab position to the left of the tab bar (after the tab list popup button)
    LinearUITabItemFlags_Trailing                      = 1 << 7,   // Enforce the tab position to the right of the tab bar (before the scrolling buttons)
    LinearUITabItemFlags_NoAssumedClosure              = 1 << 8,   // Tab is selected when trying to close + closure is not immediately assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
};

// Flags for LinearUI::IsWindowFocused()
enum LinearUIFocusedFlags_
{
    LinearUIFocusedFlags_None                          = 0,
    LinearUIFocusedFlags_ChildWindows                  = 1 << 0,   // Return true if any children of the window is focused
    LinearUIFocusedFlags_RootWindow                    = 1 << 1,   // Test from root window (top most parent of the current hierarchy)
    LinearUIFocusedFlags_AnyWindow                     = 1 << 2,   // Return true if any window is focused. Important: If you are trying to tell how to dispatch your low-level inputs, do NOT use this. Use 'io.WantCaptureMouse' instead! Please read the FAQ!
    LinearUIFocusedFlags_NoPopupHierarchy              = 1 << 3,   // Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
    LinearUIFocusedFlags_DockHierarchy                 = 1 << 4,   // Consider docking hierarchy (treat dockspace host as parent of docked window) (when used with _ChildWindows or _RootWindow)
    LinearUIFocusedFlags_RootAndChildWindows           = LinearUIFocusedFlags_RootWindow | LinearUIFocusedFlags_ChildWindows,
};

// Flags for LinearUI::IsItemHovered(), LinearUI::IsWindowHovered()
// Note: if you are trying to check whether your mouse should be dispatched to Dear LinearUI or to your app, you should use 'io.WantCaptureMouse' instead! Please read the FAQ!
// Note: windows with the LinearUIWindowFlags_NoInputs flag are ignored by IsWindowHovered() calls.
enum LinearUIHoveredFlags_
{
    LinearUIHoveredFlags_None                          = 0,        // Return true if directly over the item/window, not obstructed by another window, not obstructed by an active popup or modal blocking inputs under them.
    LinearUIHoveredFlags_ChildWindows                  = 1 << 0,   // IsWindowHovered() only: Return true if any children of the window is hovered
    LinearUIHoveredFlags_RootWindow                    = 1 << 1,   // IsWindowHovered() only: Test from root window (top most parent of the current hierarchy)
    LinearUIHoveredFlags_AnyWindow                     = 1 << 2,   // IsWindowHovered() only: Return true if any window is hovered
    LinearUIHoveredFlags_NoPopupHierarchy              = 1 << 3,   // IsWindowHovered() only: Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
    LinearUIHoveredFlags_DockHierarchy                 = 1 << 4,   // IsWindowHovered() only: Consider docking hierarchy (treat dockspace host as parent of docked window) (when used with _ChildWindows or _RootWindow)
    LinearUIHoveredFlags_AllowWhenBlockedByPopup       = 1 << 5,   // Return true even if a popup window is normally blocking access to this item/window
    //LinearUIHoveredFlags_AllowWhenBlockedByModal     = 1 << 6,   // Return true even if a modal popup window is normally blocking access to this item/window. FIXME-TODO: Unavailable yet.
    LinearUIHoveredFlags_AllowWhenBlockedByActiveItem  = 1 << 7,   // Return true even if an active item is blocking access to this item/window. Useful for Drag and Drop patterns.
    LinearUIHoveredFlags_AllowWhenOverlappedByItem     = 1 << 8,   // IsItemHovered() only: Return true even if the item uses AllowOverlap mode and is overlapped by another hoverable item.
    LinearUIHoveredFlags_AllowWhenOverlappedByWindow   = 1 << 9,   // IsItemHovered() only: Return true even if the position is obstructed or overlapped by another window.
    LinearUIHoveredFlags_AllowWhenDisabled             = 1 << 10,  // IsItemHovered() only: Return true even if the item is disabled
    LinearUIHoveredFlags_NoNavOverride                 = 1 << 11,  // IsItemHovered() only: Disable using keyboard/gamepad navigation state when active, always query mouse
    LinearUIHoveredFlags_AllowWhenOverlapped           = LinearUIHoveredFlags_AllowWhenOverlappedByItem | LinearUIHoveredFlags_AllowWhenOverlappedByWindow,
    LinearUIHoveredFlags_RectOnly                      = LinearUIHoveredFlags_AllowWhenBlockedByPopup | LinearUIHoveredFlags_AllowWhenBlockedByActiveItem | LinearUIHoveredFlags_AllowWhenOverlapped,
    LinearUIHoveredFlags_RootAndChildWindows           = LinearUIHoveredFlags_RootWindow | LinearUIHoveredFlags_ChildWindows,

    // Tooltips mode
    // - typically used in IsItemHovered() + SetTooltip() sequence.
    // - this is a shortcut to pull flags from 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav' where you can reconfigure desired behavior.
    //   e.g. 'HoverFlagsForTooltipMouse' defaults to 'LinearUIHoveredFlags_Stationary | LinearUIHoveredFlags_DelayShort | LinearUIHoveredFlags_AllowWhenDisabled'.
    // - for frequently actioned or hovered items providing a tooltip, you want may to use LinearUIHoveredFlags_ForTooltip (stationary + delay) so the tooltip doesn't show too often.
    // - for items which main purpose is to be hovered, or items with low affordance, or in less consistent apps, prefer no delay or shorter delay.
    LinearUIHoveredFlags_ForTooltip                    = 1 << 12,  // Shortcut for standard flags when using IsItemHovered() + SetTooltip() sequence.

    // (Advanced) Mouse Hovering delays.
    // - generally you can use LinearUIHoveredFlags_ForTooltip to use application-standardized flags.
    // - use those if you need specific overrides.
    LinearUIHoveredFlags_Stationary                    = 1 << 13,  // Require mouse to be stationary for style.HoverStationaryDelay (~0.15 sec) _at least one time_. After this, can move on same item/window. Using the stationary test tends to reduces the need for a long delay.
    LinearUIHoveredFlags_DelayNone                     = 1 << 14,  // IsItemHovered() only: Return true immediately (default). As this is the default you generally ignore this.
    LinearUIHoveredFlags_DelayShort                    = 1 << 15,  // IsItemHovered() only: Return true after style.HoverDelayShort elapsed (~0.15 sec) (shared between items) + requires mouse to be stationary for style.HoverStationaryDelay (once per item).
    LinearUIHoveredFlags_DelayNormal                   = 1 << 16,  // IsItemHovered() only: Return true after style.HoverDelayNormal elapsed (~0.40 sec) (shared between items) + requires mouse to be stationary for style.HoverStationaryDelay (once per item).
    LinearUIHoveredFlags_NoSharedDelay                 = 1 << 17,  // IsItemHovered() only: Disable shared delay system where moving from one item to the next keeps the previous timer for a short time (standard for tooltips with long delays)
};

// Flags for LinearUI::DockSpace(), shared/inherited by child nodes.
// (Some flags can be applied to individual nodes directly)
// FIXME-DOCK: Also see LinearUIDockNodeFlagsPrivate_ which may involve using the WIP and internal DockBuilder api.
enum LinearUIDockNodeFlags_
{
    LinearUIDockNodeFlags_None                         = 0,
    LinearUIDockNodeFlags_KeepAliveOnly                = 1 << 0,   //       // Don't display the dockspace node but keep it alive. Windows docked into this dockspace node won't be undocked.
    //LinearUIDockNodeFlags_NoCentralNode              = 1 << 1,   //       // Disable Central Node (the node which can stay empty)
    LinearUIDockNodeFlags_NoDockingOverCentralNode     = 1 << 2,   //       // Disable docking over the Central Node, which will be always kept empty.
    LinearUIDockNodeFlags_PassthruCentralNode          = 1 << 3,   //       // Enable passthru dockspace: 1) DockSpace() will render a LinearUICol_WindowBg background covering everything excepted the Central Node when empty. Meaning the host window should probably use SetNextWindowBgAlpha(0.0f) prior to Begin() when using this. 2) When Central Node is empty: let inputs pass-through + won't display a DockingEmptyBg background. See demo for details.
    LinearUIDockNodeFlags_NoDockingSplit               = 1 << 4,   //       // Disable other windows/nodes from splitting this node.
    LinearUIDockNodeFlags_NoResize                     = 1 << 5,   // Saved // Disable resizing node using the splitter/separators. Useful with programmatically setup dockspaces.
    LinearUIDockNodeFlags_AutoHideTabBar               = 1 << 6,   //       // Tab bar will automatically hide when there is a single window in the dock node.
    LinearUIDockNodeFlags_NoUndocking                  = 1 << 7,   //       // Disable undocking this node.

#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    LinearUIDockNodeFlags_NoSplit                      = LinearUIDockNodeFlags_NoDockingSplit, // Renamed in 1.90
    LinearUIDockNodeFlags_NoDockingInCentralNode       = LinearUIDockNodeFlags_NoDockingOverCentralNode, // Renamed in 1.90
#endif
};

// Flags for LinearUI::BeginDragDropSource(), LinearUI::AcceptDragDropPayload()
enum LinearUIDragDropFlags_
{
    LinearUIDragDropFlags_None                         = 0,
    // BeginDragDropSource() flags
    LinearUIDragDropFlags_SourceNoPreviewTooltip       = 1 << 0,   // Disable preview tooltip. By default, a successful call to BeginDragDropSource opens a tooltip so you can display a preview or description of the source contents. This flag disables this behavior.
    LinearUIDragDropFlags_SourceNoDisableHover         = 1 << 1,   // By default, when dragging we clear data so that IsItemHovered() will return false, to avoid subsequent user code submitting tooltips. This flag disables this behavior so you can still call IsItemHovered() on the source item.
    LinearUIDragDropFlags_SourceNoHoldToOpenOthers     = 1 << 2,   // Disable the behavior that allows to open tree nodes and collapsing header by holding over them while dragging a source item.
    LinearUIDragDropFlags_SourceAllowNullID            = 1 << 3,   // Allow items such as Text(), Image() that have no unique identifier to be used as drag source, by manufacturing a temporary identifier based on their window-relative position. This is extremely unusual within the dear linearui ecosystem and so we made it explicit.
    LinearUIDragDropFlags_SourceExtern                 = 1 << 4,   // External source (from outside of dear linearui), won't attempt to read current item/window info. Will always return true. Only one Extern source can be active simultaneously.
    LinearUIDragDropFlags_PayloadAutoExpire            = 1 << 5,   // Automatically expire the payload if the source cease to be submitted (otherwise payloads are persisting while being dragged)
    LinearUIDragDropFlags_PayloadNoCrossContext        = 1 << 6,   // Hint to specify that the payload may not be copied outside current dear linearui context.
    LinearUIDragDropFlags_PayloadNoCrossProcess        = 1 << 7,   // Hint to specify that the payload may not be copied outside current process.
    // AcceptDragDropPayload() flags
    LinearUIDragDropFlags_AcceptBeforeDelivery         = 1 << 10,  // AcceptDragDropPayload() will returns true even before the mouse button is released. You can then call IsDelivery() to test if the payload needs to be delivered.
    LinearUIDragDropFlags_AcceptNoDrawDefaultRect      = 1 << 11,  // Do not draw the default highlight rectangle when hovering over target.
    LinearUIDragDropFlags_AcceptNoPreviewTooltip       = 1 << 12,  // Request hiding the BeginDragDropSource tooltip from the BeginDragDropTarget site.
    LinearUIDragDropFlags_AcceptDrawAsHovered          = 1 << 13,  // Accepting item will render as if hovered. Useful for e.g. a Button() used as a drop target.
    LinearUIDragDropFlags_AcceptPeekOnly               = LinearUIDragDropFlags_AcceptBeforeDelivery | LinearUIDragDropFlags_AcceptNoDrawDefaultRect, // For peeking ahead and inspecting the payload before delivery.

#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    LinearUIDragDropFlags_SourceAutoExpirePayload = LinearUIDragDropFlags_PayloadAutoExpire, // Renamed in 1.90.9
#endif
};

// Standard Drag and Drop payload types. You can define you own payload types using short strings. Types starting with '_' are defined by Dear LinearUI.
#define LINEARUI_PAYLOAD_TYPE_COLOR_3F     "_COL3F"    // float[3]: Standard type for colors, without alpha. User code may use this type.
#define LINEARUI_PAYLOAD_TYPE_COLOR_4F     "_COL4F"    // float[4]: Standard type for colors. User code may use this type.

// A primary data type
enum LinearUIDataType_
{
    LinearUIDataType_S8,       // signed char / char (with sensible compilers)
    LinearUIDataType_U8,       // unsigned char
    LinearUIDataType_S16,      // short
    LinearUIDataType_U16,      // unsigned short
    LinearUIDataType_S32,      // int
    LinearUIDataType_U32,      // unsigned int
    LinearUIDataType_S64,      // long long / __int64
    LinearUIDataType_U64,      // unsigned long long / unsigned __int64
    LinearUIDataType_Float,    // float
    LinearUIDataType_Double,   // double
    LinearUIDataType_Bool,     // bool (provided for user convenience, not supported by scalar widgets)
    LinearUIDataType_String,   // char* (provided for user convenience, not supported by scalar widgets)
    LinearUIDataType_COUNT
};

// A cardinal direction
enum LinearUIDir : int
{
    LinearUIDir_None    = -1,
    LinearUIDir_Left    = 0,
    LinearUIDir_Right   = 1,
    LinearUIDir_Up      = 2,
    LinearUIDir_Down    = 3,
    LinearUIDir_COUNT
};

// A sorting direction
enum LinearUISortDirection : LinearU8
{
    LinearUISortDirection_None         = 0,
    LinearUISortDirection_Ascending    = 1,    // Ascending = 0->9, A->Z etc.
    LinearUISortDirection_Descending   = 2     // Descending = 9->0, Z->A etc.
};

// A key identifier (LinearUIKey_XXX or LinearUIMod_XXX value): can represent Keyboard, Mouse and Gamepad values.
// All our named keys are >= 512. Keys value 0 to 511 are left unused and were legacy native/opaque key values (< 1.87).
// Support for legacy keys was completely removed in 1.91.5.
// Read details about the 1.87+ transition : https://github.com/ocornut/linearui/issues/4921
// Note that "Keys" related to physical keys and are not the same concept as input "Characters", the latter are submitted via io.AddInputCharacter().
// The keyboard key enum values are named after the keys on a standard US keyboard, and on other keyboard types the keys reported may not match the keycaps.
enum LinearUIKey : int
{
    // Keyboard
    LinearUIKey_None = 0,
    LinearUIKey_NamedKey_BEGIN = 512,  // First valid key value (other than 0)

    LinearUIKey_Tab = 512,             // == LinearUIKey_NamedKey_BEGIN
    LinearUIKey_LeftArrow,
    LinearUIKey_RightArrow,
    LinearUIKey_UpArrow,
    LinearUIKey_DownArrow,
    LinearUIKey_PageUp,
    LinearUIKey_PageDown,
    LinearUIKey_Home,
    LinearUIKey_End,
    LinearUIKey_Insert,
    LinearUIKey_Delete,
    LinearUIKey_Backspace,
    LinearUIKey_Space,
    LinearUIKey_Enter,
    LinearUIKey_Escape,
    LinearUIKey_LeftCtrl, LinearUIKey_LeftShift, LinearUIKey_LeftAlt, LinearUIKey_LeftSuper,     // Also see LinearUIMod_Ctrl, LinearUIMod_Shift, LinearUIMod_Alt, LinearUIMod_Super below!
    LinearUIKey_RightCtrl, LinearUIKey_RightShift, LinearUIKey_RightAlt, LinearUIKey_RightSuper,
    LinearUIKey_Menu,
    LinearUIKey_0, LinearUIKey_1, LinearUIKey_2, LinearUIKey_3, LinearUIKey_4, LinearUIKey_5, LinearUIKey_6, LinearUIKey_7, LinearUIKey_8, LinearUIKey_9,
    LinearUIKey_A, LinearUIKey_B, LinearUIKey_C, LinearUIKey_D, LinearUIKey_E, LinearUIKey_F, LinearUIKey_G, LinearUIKey_H, LinearUIKey_I, LinearUIKey_J,
    LinearUIKey_K, LinearUIKey_L, LinearUIKey_M, LinearUIKey_N, LinearUIKey_O, LinearUIKey_P, LinearUIKey_Q, LinearUIKey_R, LinearUIKey_S, LinearUIKey_T,
    LinearUIKey_U, LinearUIKey_V, LinearUIKey_W, LinearUIKey_X, LinearUIKey_Y, LinearUIKey_Z,
    LinearUIKey_F1, LinearUIKey_F2, LinearUIKey_F3, LinearUIKey_F4, LinearUIKey_F5, LinearUIKey_F6,
    LinearUIKey_F7, LinearUIKey_F8, LinearUIKey_F9, LinearUIKey_F10, LinearUIKey_F11, LinearUIKey_F12,
    LinearUIKey_F13, LinearUIKey_F14, LinearUIKey_F15, LinearUIKey_F16, LinearUIKey_F17, LinearUIKey_F18,
    LinearUIKey_F19, LinearUIKey_F20, LinearUIKey_F21, LinearUIKey_F22, LinearUIKey_F23, LinearUIKey_F24,
    LinearUIKey_Apostrophe,        // '
    LinearUIKey_Comma,             // ,
    LinearUIKey_Minus,             // -
    LinearUIKey_Period,            // .
    LinearUIKey_Slash,             // /
    LinearUIKey_Semicolon,         // ;
    LinearUIKey_Equal,             // =
    LinearUIKey_LeftBracket,       // [
    LinearUIKey_Backslash,         // \ (this text inhibit multiline comment caused by backslash)
    LinearUIKey_RightBracket,      // ]
    LinearUIKey_GraveAccent,       // `
    LinearUIKey_CapsLock,
    LinearUIKey_ScrollLock,
    LinearUIKey_NumLock,
    LinearUIKey_PrintScreen,
    LinearUIKey_Pause,
    LinearUIKey_Keypad0, LinearUIKey_Keypad1, LinearUIKey_Keypad2, LinearUIKey_Keypad3, LinearUIKey_Keypad4,
    LinearUIKey_Keypad5, LinearUIKey_Keypad6, LinearUIKey_Keypad7, LinearUIKey_Keypad8, LinearUIKey_Keypad9,
    LinearUIKey_KeypadDecimal,
    LinearUIKey_KeypadDivide,
    LinearUIKey_KeypadMultiply,
    LinearUIKey_KeypadSubtract,
    LinearUIKey_KeypadAdd,
    LinearUIKey_KeypadEnter,
    LinearUIKey_KeypadEqual,
    LinearUIKey_AppBack,               // Available on some keyboard/mouses. Often referred as "Browser Back"
    LinearUIKey_AppForward,
    LinearUIKey_Oem102,                // Non-US backslash.

    // Gamepad
    // (analog values are 0.0f to 1.0f)
    // (download controller mapping PNG/PSD at http://dearlinearui.com/controls_sheets)
    //                              // XBOX        | SWITCH  | PLAYSTA. | -> ACTION
    LinearUIKey_GamepadStart,          // Menu        | +       | Options  |
    LinearUIKey_GamepadBack,           // View        | -       | Share    |
    LinearUIKey_GamepadFaceLeft,       // X           | Y       | Square   | Toggle Menu. Hold for Windowing mode (Focus/Move/Resize windows)
    LinearUIKey_GamepadFaceRight,      // B           | A       | Circle   | Cancel / Close / Exit
    LinearUIKey_GamepadFaceUp,         // Y           | X       | Triangle | Open Context Menu
    LinearUIKey_GamepadFaceDown,       // A           | B       | Cross    | Activate / Open / Toggle. Hold for 0.60f to Activate in Text Input mode (e.g. wired to an on-screen keyboard).
    LinearUIKey_GamepadDpadLeft,       // D-pad Left  | "       | "        | Move / Tweak / Resize Window (in Windowing mode)
    LinearUIKey_GamepadDpadRight,      // D-pad Right | "       | "        | Move / Tweak / Resize Window (in Windowing mode)
    LinearUIKey_GamepadDpadUp,         // D-pad Up    | "       | "        | Move / Tweak / Resize Window (in Windowing mode)
    LinearUIKey_GamepadDpadDown,       // D-pad Down  | "       | "        | Move / Tweak / Resize Window (in Windowing mode)
    LinearUIKey_GamepadL1,             // L Bumper    | L       | L1       | Tweak Slower / Focus Previous (in Windowing mode)
    LinearUIKey_GamepadR1,             // R Bumper    | R       | R1       | Tweak Faster / Focus Next (in Windowing mode)
    LinearUIKey_GamepadL2,             // L Trigger   | ZL      | L2       | [Analog]
    LinearUIKey_GamepadR2,             // R Trigger   | ZR      | R2       | [Analog]
    LinearUIKey_GamepadL3,             // L Stick     | L3      | L3       |
    LinearUIKey_GamepadR3,             // R Stick     | R3      | R3       |
    LinearUIKey_GamepadLStickLeft,     //             |         |          | [Analog] Move Window (in Windowing mode)
    LinearUIKey_GamepadLStickRight,    //             |         |          | [Analog] Move Window (in Windowing mode)
    LinearUIKey_GamepadLStickUp,       //             |         |          | [Analog] Move Window (in Windowing mode)
    LinearUIKey_GamepadLStickDown,     //             |         |          | [Analog] Move Window (in Windowing mode)
    LinearUIKey_GamepadRStickLeft,     //             |         |          | [Analog]
    LinearUIKey_GamepadRStickRight,    //             |         |          | [Analog]
    LinearUIKey_GamepadRStickUp,       //             |         |          | [Analog]
    LinearUIKey_GamepadRStickDown,     //             |         |          | [Analog]

    // Aliases: Mouse Buttons (auto-submitted from AddMouseButtonEvent() calls)
    // - This is mirroring the data also written to io.MouseDown[], io.MouseWheel, in a format allowing them to be accessed via standard key API.
    LinearUIKey_MouseLeft, LinearUIKey_MouseRight, LinearUIKey_MouseMiddle, LinearUIKey_MouseX1, LinearUIKey_MouseX2, LinearUIKey_MouseWheelX, LinearUIKey_MouseWheelY,

    // [Internal] Reserved for mod storage
    LinearUIKey_ReservedForModCtrl, LinearUIKey_ReservedForModShift, LinearUIKey_ReservedForModAlt, LinearUIKey_ReservedForModSuper,

    // [Internal] If you need to iterate all keys (for e.g. an input mapper) you may use LinearUIKey_NamedKey_BEGIN..LinearUIKey_NamedKey_END.
    LinearUIKey_NamedKey_END,
    LinearUIKey_NamedKey_COUNT = LinearUIKey_NamedKey_END - LinearUIKey_NamedKey_BEGIN,

    // Keyboard Modifiers (explicitly submitted by backend via AddKeyEvent() calls)
    // - Any functions taking a LinearUIKeyChord parameter can binary-or those with regular keys, e.g. Shortcut(LinearUIMod_Ctrl | LinearUIKey_S).
    // - Those are written back into io.KeyCtrl, io.KeyShift, io.KeyAlt, io.KeySuper for convenience,
    //   but may be accessed via standard key API such as IsKeyPressed(), IsKeyReleased(), querying duration etc.
    // - Code polling every key (e.g. an interface to detect a key press for input mapping) might want to ignore those
    //   and prefer using the real keys (e.g. LinearUIKey_LeftCtrl, LinearUIKey_RightCtrl instead of LinearUIMod_Ctrl).
    // - In theory the value of keyboard modifiers should be roughly equivalent to a logical or of the equivalent left/right keys.
    //   In practice: it's complicated; mods are often provided from different sources. Keyboard layout, IME, sticky keys and
    //   backends tend to interfere and break that equivalence. The safer decision is to relay that ambiguity down to the end-user...
    // - On macOS, we swap Cmd(Super) and Ctrl keys at the time of the io.AddKeyEvent() call.
    LinearUIMod_None                   = 0,
    LinearUIMod_Ctrl                   = 1 << 12, // Ctrl (non-macOS), Cmd (macOS)
    LinearUIMod_Shift                  = 1 << 13, // Shift
    LinearUIMod_Alt                    = 1 << 14, // Option/Menu
    LinearUIMod_Super                  = 1 << 15, // Windows/Super (non-macOS), Ctrl (macOS)
    LinearUIMod_Mask_                  = 0xF000,  // 4-bits

#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    LinearUIKey_COUNT                  = LinearUIKey_NamedKey_END,    // Obsoleted in 1.91.5 because it was misleading (since named keys don't start at 0 anymore)
    LinearUIMod_Shortcut               = LinearUIMod_Ctrl,            // Removed in 1.90.7, you can now simply use LinearUIMod_Ctrl
    //LinearUIKey_ModCtrl = LinearUIMod_Ctrl, LinearUIKey_ModShift = LinearUIMod_Shift, LinearUIKey_ModAlt = LinearUIMod_Alt, LinearUIKey_ModSuper = LinearUIMod_Super, // Renamed in 1.89
    //LinearUIKey_KeyPadEnter = LinearUIKey_KeypadEnter,              // Renamed in 1.87
#endif
};

// Flags for Shortcut(), SetNextItemShortcut(),
// (and for upcoming extended versions of IsKeyPressed(), IsMouseClicked(), Shortcut(), SetKeyOwner(), SetItemKeyOwner() that are still in linearui_internal.h)
// Don't mistake with LinearUIInputTextFlags! (which is for LinearUI::InputText() function)
enum LinearUIInputFlags_
{
    LinearUIInputFlags_None                    = 0,
    LinearUIInputFlags_Repeat                  = 1 << 0,   // Enable repeat. Return true on successive repeats. Default for legacy IsKeyPressed(). NOT Default for legacy IsMouseClicked(). MUST BE == 1.

    // Flags for Shortcut(), SetNextItemShortcut()
    // - Routing policies: RouteGlobal+OverActive >> RouteActive or RouteFocused (if owner is active item) >> RouteGlobal+OverFocused >> RouteFocused (if in focused window stack) >> RouteGlobal.
    // - Default policy is RouteFocused. Can select only 1 policy among all available.
    LinearUIInputFlags_RouteActive             = 1 << 10,  // Route to active item only.
    LinearUIInputFlags_RouteFocused            = 1 << 11,  // Route to windows in the focus stack (DEFAULT). Deep-most focused window takes inputs. Active item takes inputs over deep-most focused window.
    LinearUIInputFlags_RouteGlobal             = 1 << 12,  // Global route (unless a focused window or active item registered the route).
    LinearUIInputFlags_RouteAlways             = 1 << 13,  // Do not register route, poll keys directly.
    // - Routing options
    LinearUIInputFlags_RouteOverFocused        = 1 << 14,  // Option: global route: higher priority than focused route (unless active item in focused route).
    LinearUIInputFlags_RouteOverActive         = 1 << 15,  // Option: global route: higher priority than active item. Unlikely you need to use that: will interfere with every active items, e.g. Ctrl+A registered by InputText will be overridden by this. May not be fully honored as user/internal code is likely to always assume they can access keys when active.
    LinearUIInputFlags_RouteUnlessBgFocused    = 1 << 16,  // Option: global route: will not be applied if underlying background/void is focused (== no Dear LinearUI windows are focused). Useful for overlay applications.
    LinearUIInputFlags_RouteFromRootWindow     = 1 << 17,  // Option: route evaluated from the point of view of root window rather than current window.

    // Flags for SetNextItemShortcut()
    LinearUIInputFlags_Tooltip                 = 1 << 18,  // Automatically display a tooltip when hovering item [BETA] Unsure of right api (opt-in/opt-out)
};

// Configuration flags stored in io.ConfigFlags. Set by user/application.
// Note that nowadays most of our configuration options are in other LinearUIIO fields, e.g. io.ConfigWindowsMoveFromTitleBarOnly.
enum LinearUIConfigFlags_
{
    LinearUIConfigFlags_None                   = 0,
    LinearUIConfigFlags_NavEnableKeyboard      = 1 << 0,   // Master keyboard navigation enable flag. Enable full Tabbing + directional arrows + Space/Enter to activate. Note: some features such as basic Tabbing and CtrL+Tab are enabled by regardless of this flag (and may be disabled via other means, see #4828, #9218).
    LinearUIConfigFlags_NavEnableGamepad       = 1 << 1,   // Master gamepad navigation enable flag. Backend also needs to set LinearUIBackendFlags_HasGamepad.
    LinearUIConfigFlags_NoMouse                = 1 << 4,   // Instruct dear linearui to disable mouse inputs and interactions.
    LinearUIConfigFlags_NoMouseCursorChange    = 1 << 5,   // Instruct backend to not alter mouse cursor shape and visibility. Use if the backend cursor changes are interfering with yours and you don't want to use SetMouseCursor() to change mouse cursor. You may want to honor requests from linearui by reading GetMouseCursor() yourself instead.
    LinearUIConfigFlags_NoKeyboard             = 1 << 6,   // Instruct dear linearui to disable keyboard inputs and interactions. This is done by ignoring keyboard events and clearing existing states.

    // [BETA] Docking
    LinearUIConfigFlags_DockingEnable          = 1 << 7,   // Docking enable flags.

    // [BETA] Viewports
    // When using viewports it is recommended that your default value for LinearUICol_WindowBg is opaque (Alpha=1.0) so transition to a viewport won't be noticeable.
    LinearUIConfigFlags_ViewportsEnable        = 1 << 10,  // Viewport enable flags (require both LinearUIBackendFlags_PlatformHasViewports + LinearUIBackendFlags_RendererHasViewports set by the respective backends)

    // [Unused] User storage (to allow your backend/engine to communicate to code that may be shared between multiple projects. Those flags are NOT used by core Dear LinearUI)
    LinearUIConfigFlags_IsSRGB                 = 1 << 20,  // Application is SRGB-aware.
    LinearUIConfigFlags_IsTouchScreen          = 1 << 21,  // Application is using a touch screen instead of a mouse.
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    LinearUIConfigFlags_NavEnableSetMousePos   = 1 << 2,   // [moved/renamed in 1.91.4] -> use bool io.ConfigNavMoveSetMousePos
    LinearUIConfigFlags_NavNoCaptureKeyboard   = 1 << 3,   // [moved/renamed in 1.91.4] -> use bool io.ConfigNavCaptureKeyboard
    LinearUIConfigFlags_DpiEnableScaleFonts    = 1 << 14,  // [moved/renamed in 1.92.0] -> use bool io.ConfigDpiScaleFonts
    LinearUIConfigFlags_DpiEnableScaleViewports= 1 << 15,  // [moved/renamed in 1.92.0] -> use bool io.ConfigDpiScaleViewports
#endif
};

// Backend capabilities flags stored in io.BackendFlags. Set by linearui_impl_xxx or custom backend.
enum LinearUIBackendFlags_
{
    LinearUIBackendFlags_None                  = 0,
    LinearUIBackendFlags_HasGamepad            = 1 << 0,   // Backend Platform supports gamepad and currently has one connected.
    LinearUIBackendFlags_HasMouseCursors       = 1 << 1,   // Backend Platform supports honoring GetMouseCursor() value to change the OS cursor shape.
    LinearUIBackendFlags_HasSetMousePos        = 1 << 2,   // Backend Platform supports io.WantSetMousePos requests to reposition the OS mouse position (only used if io.ConfigNavMoveSetMousePos is set).
    LinearUIBackendFlags_RendererHasVtxOffset  = 1 << 3,   // Backend Renderer supports LinearDrawCmd::VtxOffset. This enables output of large meshes (64K+ vertices) while still using 16-bit indices.
    LinearUIBackendFlags_RendererHasTextures   = 1 << 4,   // Backend Renderer supports LinearTextureData requests to create/update/destroy textures. This enables incremental texture updates and texture reloads. See https://github.com/ocornut/linearui/blob/master/docs/BACKENDS.md for instructions on how to upgrade your custom backend.

    // [BETA] Multi-Viewports
    LinearUIBackendFlags_RendererHasViewports  = 1 << 10,  // Backend Renderer supports multiple viewports.
    LinearUIBackendFlags_PlatformHasViewports  = 1 << 11,  // Backend Platform supports multiple viewports.
    LinearUIBackendFlags_HasMouseHoveredViewport=1 << 12,  // Backend Platform supports calling io.AddMouseViewportEvent() with the viewport under the mouse. IF POSSIBLE, ignore viewports with the LinearUIViewportFlags_NoInputs flag (Win32 backend, LINEARWIN 3.30+ backend can do this, SDL backend cannot). If this cannot be done, Dear LinearUI needs to use a flawed heuristic to find the viewport under.
    LinearUIBackendFlags_HasParentViewport     = 1 << 13,  // Backend Platform supports honoring viewport->ParentViewport/ParentViewportId value, by applying the corresponding parent/child relationship at the Platform level. Child windows always appear in front of their parent window.
};

// Enumeration for PushStyleColor() / PopStyleColor()
enum LinearUICol_
{
    LinearUICol_Text,
    LinearUICol_TextDisabled,
    LinearUICol_WindowBg,              // Background of normal windows
    LinearUICol_ChildBg,               // Background of child windows
    LinearUICol_PopupBg,               // Background of popups, menus, tooltips windows
    LinearUICol_Border,
    LinearUICol_BorderShadow,
    LinearUICol_FrameBg,               // Background of checkbox, radio button, plot, slider, text input
    LinearUICol_FrameBgHovered,
    LinearUICol_FrameBgActive,
    LinearUICol_TitleBg,               // Title bar
    LinearUICol_TitleBgActive,         // Title bar when focused
    LinearUICol_TitleBgCollapsed,      // Title bar when collapsed
    LinearUICol_MenuBarBg,
    LinearUICol_ScrollbarBg,
    LinearUICol_ScrollbarGrab,
    LinearUICol_ScrollbarGrabHovered,
    LinearUICol_ScrollbarGrabActive,
    LinearUICol_CheckMark,             // Checkbox tick and RadioButton circle
    LinearUICol_CheckboxSelectedBg,    // Checkbox background when Selected, otherwise use FrameBg
    LinearUICol_SliderGrab,
    LinearUICol_SliderGrabActive,
    LinearUICol_Button,
    LinearUICol_ButtonHovered,
    LinearUICol_ButtonActive,
    LinearUICol_Header,                // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
    LinearUICol_HeaderHovered,
    LinearUICol_HeaderActive,
    LinearUICol_Separator,
    LinearUICol_SeparatorHovered,
    LinearUICol_SeparatorActive,
    LinearUICol_ResizeGrip,            // Resize grip in lower-right and lower-left corners of windows.
    LinearUICol_ResizeGripHovered,
    LinearUICol_ResizeGripActive,
    LinearUICol_InputTextCursor,       // InputText cursor/caret
    LinearUICol_TabHovered,            // Tab background, when hovered
    LinearUICol_Tab,                   // Tab background, when tab-bar is focused & tab is unselected
    LinearUICol_TabSelected,           // Tab background, when tab-bar is focused & tab is selected
    LinearUICol_TabSelectedOverline,   // Tab horizontal overline, when tab-bar is focused & tab is selected
    LinearUICol_TabDimmed,             // Tab background, when tab-bar is unfocused & tab is unselected
    LinearUICol_TabDimmedSelected,     // Tab background, when tab-bar is unfocused & tab is selected
    LinearUICol_TabDimmedSelectedOverline,//..horizontal overline, when tab-bar is unfocused & tab is selected
    LinearUICol_DockingPreview,        // Preview overlay color when about to docking something
    LinearUICol_DockingEmptyBg,        // Background color for empty node (e.g. CentralNode with no window docked into it)
    LinearUICol_PlotLines,
    LinearUICol_PlotLinesHovered,
    LinearUICol_PlotHistogram,
    LinearUICol_PlotHistogramHovered,
    LinearUICol_TableHeaderBg,         // Table header background
    LinearUICol_TableBorderStrong,     // Table outer and header borders (prefer using Alpha=1.0 here)
    LinearUICol_TableBorderLight,      // Table inner borders (prefer using Alpha=1.0 here)
    LinearUICol_TableRowBg,            // Table row background (even rows)
    LinearUICol_TableRowBgAlt,         // Table row background (odd rows)
    LinearUICol_TextLink,              // Hyperlink color
    LinearUICol_TextSelectedBg,        // Selected text inside an InputText
    LinearUICol_TreeLines,             // Tree node hierarchy outlines when using LinearUITreeNodeFlags_DrawLines
    LinearUICol_DragDropTarget,        // Rectangle border highlighting a drop target
    LinearUICol_DragDropTargetBg,      // Rectangle background highlighting a drop target
    LinearUICol_UnsavedMarker,         // Unsaved Document marker (in window title and tabs)
    LinearUICol_NavCursor,             // Color of keyboard/gamepad navigation cursor/rectangle, when visible
    LinearUICol_NavWindowingHighlight, // Highlight window when using Ctrl+Tab
    LinearUICol_NavWindowingDimBg,     // Darken/colorize entire screen behind the Ctrl+Tab window list, when active
    LinearUICol_ModalWindowDimBg,      // Darken/colorize entire screen behind a modal window, when one is active
    LinearUICol_COUNT,

#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    LinearUICol_TabActive = LinearUICol_TabSelected,                  // [renamed in 1.90.9]
    LinearUICol_TabUnfocused = LinearUICol_TabDimmed,                 // [renamed in 1.90.9]
    LinearUICol_TabUnfocusedActive = LinearUICol_TabDimmedSelected,   // [renamed in 1.90.9]
    LinearUICol_NavHighlight = LinearUICol_NavCursor,                 // [renamed in 1.91.4]
#endif
};

// Enumeration for PushStyleVar() / PopStyleVar() to temporarily modify the LinearUIStyle structure.
// - The enum only refers to fields of LinearUIStyle which makes sense to be pushed/popped inside UI code.
//   During initialization or between frames, feel free to just poke into LinearUIStyle directly.
// - Tip: Use your programming IDE navigation facilities on the names in the _second column_ below to find the actual members and their description.
//   - In Visual Studio: Ctrl+Comma ("Edit.GoToAll") can follow symbols inside comments, whereas Ctrl+F12 ("Edit.GoToImplementation") cannot.
//   - In Visual Studio w/ Visual Assist installed: Alt+G ("VAssistX.GoToImplementation") can also follow symbols inside comments.
//   - In VS Code, CLion, etc.: Ctrl+Click can follow symbols inside comments.
// - When changing this enum, you need to update the associated internal table GStyleVarInfo[] accordingly. This is where we link enum values to members offset/type.
enum LinearUIStyleVar_
{
    // Enum name -------------------------- // Member in LinearUIStyle structure (see LinearUIStyle for descriptions)
    LinearUIStyleVar_Alpha,                    // float     Alpha
    LinearUIStyleVar_DisabledAlpha,            // float     DisabledAlpha
    LinearUIStyleVar_WindowPadding,            // LinearVec2    WindowPadding
    LinearUIStyleVar_WindowRounding,           // float     WindowRounding
    LinearUIStyleVar_WindowBorderSize,         // float     WindowBorderSize
    LinearUIStyleVar_WindowMinSize,            // LinearVec2    WindowMinSize
    LinearUIStyleVar_WindowTitleAlign,         // LinearVec2    WindowTitleAlign
    LinearUIStyleVar_ChildRounding,            // float     ChildRounding
    LinearUIStyleVar_ChildBorderSize,          // float     ChildBorderSize
    LinearUIStyleVar_PopupRounding,            // float     PopupRounding
    LinearUIStyleVar_PopupBorderSize,          // float     PopupBorderSize
    LinearUIStyleVar_FramePadding,             // LinearVec2    FramePadding
    LinearUIStyleVar_FrameRounding,            // float     FrameRounding
    LinearUIStyleVar_FrameBorderSize,          // float     FrameBorderSize
    LinearUIStyleVar_ItemSpacing,              // LinearVec2    ItemSpacing
    LinearUIStyleVar_ItemInnerSpacing,         // LinearVec2    ItemInnerSpacing
    LinearUIStyleVar_IndentSpacing,            // float     IndentSpacing
    LinearUIStyleVar_CellPadding,              // LinearVec2    CellPadding
    LinearUIStyleVar_ScrollbarSize,            // float     ScrollbarSize
    LinearUIStyleVar_ScrollbarRounding,        // float     ScrollbarRounding
    LinearUIStyleVar_ScrollbarPadding,         // float     ScrollbarPadding
    LinearUIStyleVar_GrabMinSize,              // float     GrabMinSize
    LinearUIStyleVar_GrabRounding,             // float     GrabRounding
    LinearUIStyleVar_ImageRounding,            // float     ImageRounding
    LinearUIStyleVar_ImageBorderSize,          // float     ImageBorderSize
    LinearUIStyleVar_TabRounding,              // float     TabRounding
    LinearUIStyleVar_TabBorderSize,            // float     TabBorderSize
    LinearUIStyleVar_TabMinWidthBase,          // float     TabMinWidthBase
    LinearUIStyleVar_TabMinWidthShrink,        // float     TabMinWidthShrink
    LinearUIStyleVar_TabBarBorderSize,         // float     TabBarBorderSize
    LinearUIStyleVar_TabBarOverlineSize,       // float     TabBarOverlineSize
    LinearUIStyleVar_TableAngledHeadersAngle,  // float     TableAngledHeadersAngle
    LinearUIStyleVar_TableAngledHeadersTextAlign,// LinearVec2  TableAngledHeadersTextAlign
    LinearUIStyleVar_TreeLinesSize,            // float     TreeLinesSize
    LinearUIStyleVar_TreeLinesRounding,        // float     TreeLinesRounding
    LinearUIStyleVar_DragDropTargetRounding,   // float     DragDropTargetRounding
    LinearUIStyleVar_ButtonTextAlign,          // LinearVec2    ButtonTextAlign
    LinearUIStyleVar_SelectableTextAlign,      // LinearVec2    SelectableTextAlign
    LinearUIStyleVar_SeparatorSize,            // float     SeparatorSize
    LinearUIStyleVar_SeparatorTextBorderSize,  // float     SeparatorTextBorderSize
    LinearUIStyleVar_SeparatorTextAlign,       // LinearVec2    SeparatorTextAlign
    LinearUIStyleVar_SeparatorTextPadding,     // LinearVec2    SeparatorTextPadding
    LinearUIStyleVar_DockingSeparatorSize,     // float     DockingSeparatorSize
    LinearUIStyleVar_COUNT
};

// Flags for InvisibleButton() [extended in linearui_internal.h]
enum LinearUIButtonFlags_
{
    LinearUIButtonFlags_None                   = 0,
    LinearUIButtonFlags_MouseButtonLeft        = 1 << 0,   // React on left mouse button (default)
    LinearUIButtonFlags_MouseButtonRight       = 1 << 1,   // React on right mouse button
    LinearUIButtonFlags_MouseButtonMiddle      = 1 << 2,   // React on center mouse button
    LinearUIButtonFlags_MouseButtonMask_       = LinearUIButtonFlags_MouseButtonLeft | LinearUIButtonFlags_MouseButtonRight | LinearUIButtonFlags_MouseButtonMiddle, // [Internal]
    LinearUIButtonFlags_EnableNav              = 1 << 3,   // InvisibleButton(): do not disable navigation/tabbing. Otherwise disabled by default.
    LinearUIButtonFlags_AllowOverlap           = 1 << 12,  // Hit testing will allow subsequent widgets to overlap this one. Require previous frame HoveredId to match before being usable. Shortcut to calling SetNextItemAllowOverlap().
};

// Flags for ColorEdit3() / ColorEdit4() / ColorPicker3() / ColorPicker4() / ColorButton()
enum LinearUIColorEditFlags_
{
    LinearUIColorEditFlags_None            = 0,
    LinearUIColorEditFlags_NoAlpha         = 1 << 1,   //              // ColorEdit, ColorPicker, ColorButton: ignore Alpha component (will only read 3 components from the input pointer).
    LinearUIColorEditFlags_NoPicker        = 1 << 2,   //              // ColorEdit: disable picker when clicking on color square.
    LinearUIColorEditFlags_NoOptions       = 1 << 3,   //              // ColorEdit: disable toggling options menu when right-clicking on inputs/small preview.
    LinearUIColorEditFlags_NoSmallPreview  = 1 << 4,   //              // ColorEdit, ColorPicker: disable color square preview next to the inputs. (e.g. to show only the inputs)
    LinearUIColorEditFlags_NoInputs        = 1 << 5,   //              // ColorEdit, ColorPicker: disable inputs sliders/text widgets (e.g. to show only the small preview color square).
    LinearUIColorEditFlags_NoTooltip       = 1 << 6,   //              // ColorEdit, ColorPicker, ColorButton: disable tooltip when hovering the preview.
    LinearUIColorEditFlags_NoLabel         = 1 << 7,   //              // ColorEdit, ColorPicker: disable display of inline text label (the label is still forwarded to the tooltip and picker).
    LinearUIColorEditFlags_NoSidePreview   = 1 << 8,   //              // ColorPicker: disable bigger color preview on right side of the picker, use small color square preview instead.
    LinearUIColorEditFlags_NoDragDrop      = 1 << 9,   //              // ColorEdit: disable drag and drop target/source. ColorButton: disable drag and drop source.
    LinearUIColorEditFlags_NoBorder        = 1 << 10,  //              // ColorButton: disable border (which is enforced by default)
    LinearUIColorEditFlags_NoColorMarkers  = 1 << 11,  //              // ColorEdit: disable rendering R/G/B/A color marker. May also be disabled globally by setting style.ColorMarkerSize = 0.

    // Alpha preview
    // - Prior to 1.91.8 (2025/01/21): alpha was made opaque in the preview by default using old name LinearUIColorEditFlags_AlphaPreview.
    // - We now display the preview as transparent by default. You can use LinearUIColorEditFlags_AlphaOpaque to use old behavior.
    // - The new flags may be combined better and allow finer controls.
    LinearUIColorEditFlags_AlphaOpaque     = 1 << 12,  //              // ColorEdit, ColorPicker, ColorButton: disable alpha in the preview,. Contrary to _NoAlpha it may still be edited when calling ColorEdit4()/ColorPicker4(). For ColorButton() this does the same as _NoAlpha.
    LinearUIColorEditFlags_AlphaNoBg       = 1 << 13,  //              // ColorEdit, ColorPicker, ColorButton: disable rendering a checkerboard background behind transparent color.
    LinearUIColorEditFlags_AlphaPreviewHalf= 1 << 14,  //              // ColorEdit, ColorPicker, ColorButton: display half opaque / half transparent preview.

    // User Options (right-click on widget to change some of them).
    LinearUIColorEditFlags_AlphaBar        = 1 << 18,  //              // ColorEdit, ColorPicker: show vertical alpha bar/gradient in picker.
    LinearUIColorEditFlags_HDR             = 1 << 19,  //              // (WIP) ColorEdit: Currently only disable 0.0f..1.0f limits in RGBA edition (note: you probably want to use LinearUIColorEditFlags_Float flag as well).
    LinearUIColorEditFlags_DisplayRGB      = 1 << 20,  // [Display]    // ColorEdit: override _display_ type among RGB/HSV/Hex. ColorPicker: select any combination using one or more of RGB/HSV/Hex.
    LinearUIColorEditFlags_DisplayHSV      = 1 << 21,  // [Display]    // "
    LinearUIColorEditFlags_DisplayHex      = 1 << 22,  // [Display]    // "
    LinearUIColorEditFlags_Uint8           = 1 << 23,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0..255.
    LinearUIColorEditFlags_Float           = 1 << 24,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0.0f..1.0f floats instead of 0..255 integers. No round-trip of value via integers.
    LinearUIColorEditFlags_PickerHueBar    = 1 << 25,  // [Picker]     // ColorPicker: bar for Hue, rectangle for Sat/Value.
    LinearUIColorEditFlags_PickerHueWheel  = 1 << 26,  // [Picker]     // ColorPicker: wheel for Hue, triangle for Sat/Value.
    LinearUIColorEditFlags_InputRGB        = 1 << 27,  // [Input]      // ColorEdit, ColorPicker: input and output data in RGB format.
    LinearUIColorEditFlags_InputHSV        = 1 << 28,  // [Input]      // ColorEdit, ColorPicker: input and output data in HSV format.

    // Defaults Options. You can set application defaults using SetColorEditOptions(). The intent is that you probably don't want to
    // override them in most of your calls. Let the user choose via the option menu and/or call SetColorEditOptions() once during startup.
    LinearUIColorEditFlags_DefaultOptions_ = LinearUIColorEditFlags_Uint8 | LinearUIColorEditFlags_DisplayRGB | LinearUIColorEditFlags_InputRGB | LinearUIColorEditFlags_PickerHueBar,

    // [Internal] Masks
    LinearUIColorEditFlags_AlphaMask_      = LinearUIColorEditFlags_NoAlpha | LinearUIColorEditFlags_AlphaOpaque | LinearUIColorEditFlags_AlphaNoBg | LinearUIColorEditFlags_AlphaPreviewHalf,
    LinearUIColorEditFlags_DisplayMask_    = LinearUIColorEditFlags_DisplayRGB | LinearUIColorEditFlags_DisplayHSV | LinearUIColorEditFlags_DisplayHex,
    LinearUIColorEditFlags_DataTypeMask_   = LinearUIColorEditFlags_Uint8 | LinearUIColorEditFlags_Float,
    LinearUIColorEditFlags_PickerMask_     = LinearUIColorEditFlags_PickerHueWheel | LinearUIColorEditFlags_PickerHueBar,
    LinearUIColorEditFlags_InputMask_      = LinearUIColorEditFlags_InputRGB | LinearUIColorEditFlags_InputHSV,

    // Obsolete names
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    LinearUIColorEditFlags_AlphaPreview = 0, // Removed in 1.91.8. This is the default now. Will display a checkerboard unless LinearUIColorEditFlags_AlphaNoBg is set.
#endif
    //LinearUIColorEditFlags_RGB = LinearUIColorEditFlags_DisplayRGB, LinearUIColorEditFlags_HSV = LinearUIColorEditFlags_DisplayHSV, LinearUIColorEditFlags_HEX = LinearUIColorEditFlags_DisplayHex  // [renamed in 1.69]
};

// Flags for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
// We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.
// (Those are per-item flags. There is shared behavior flag too: LinearUIIO: io.ConfigDragClickToInputText)
enum LinearUISliderFlags_
{
    LinearUISliderFlags_None               = 0,
    LinearUISliderFlags_Logarithmic        = 1 << 5,       // Make the widget logarithmic (linear otherwise). Consider using LinearUISliderFlags_NoRoundToFormat with this if using a format-string with small amount of digits.
    LinearUISliderFlags_NoRoundToFormat    = 1 << 6,       // Disable rounding underlying value to match precision of the display format string (e.g. %.3f values are rounded to those 3 digits).
    LinearUISliderFlags_NoInput            = 1 << 7,       // Disable Ctrl+Click or Enter key allowing to input text directly into the widget.
    LinearUISliderFlags_WrapAround         = 1 << 8,       // Enable wrapping around from max to min and from min to max. Only supported by DragXXX() functions for now.
    LinearUISliderFlags_ClampOnInput       = 1 << 9,       // Clamp value to min/max bounds when input manually with Ctrl+Click. By default Ctrl+Click allows going out of bounds.
    LinearUISliderFlags_ClampZeroRange     = 1 << 10,      // Clamp even if min==max==0.0f. Otherwise due to legacy reason DragXXX functions don't clamp with those values. When your clamping limits are dynamic you almost always want to use it.
    LinearUISliderFlags_NoSpeedTweaks      = 1 << 11,      // Disable keyboard modifiers altering tweak speed. Useful if you want to alter tweak speed yourself based on your own logic.
    LinearUISliderFlags_ColorMarkers       = 1 << 12,      // DragScalarN(), SliderScalarN(): Draw R/G/B/A color markers on each component.
    LinearUISliderFlags_AlwaysClamp        = LinearUISliderFlags_ClampOnInput | LinearUISliderFlags_ClampZeroRange,
    LinearUISliderFlags_InvalidMask_       = 0x7000000F,   // [Internal] We treat using those bits as being potentially a 'float power' argument from legacy API (obsoleted 2020-08) that has got miscast to this enum, and will trigger an assert if needed.
};

// Identify a mouse button.
// Those values are guaranteed to be stable and we frequently use 0/1 directly. Named enums provided for convenience.
enum LinearUIMouseButton_
{
    LinearUIMouseButton_Left = 0,
    LinearUIMouseButton_Right = 1,
    LinearUIMouseButton_Middle = 2,
    LinearUIMouseButton_COUNT = 5
};

// Enumeration for GetMouseCursor()
// User code may request backend to display given cursor by calling SetMouseCursor(), which is why we have some cursors that are marked unused here
enum LinearUIMouseCursor_
{
    LinearUIMouseCursor_None = -1,
    LinearUIMouseCursor_Arrow = 0,
    LinearUIMouseCursor_TextInput,         // When hovering over InputText, etc.
    LinearUIMouseCursor_ResizeAll,         // (Unused by Dear LinearUI functions)
    LinearUIMouseCursor_ResizeNS,          // When hovering over a horizontal border
    LinearUIMouseCursor_ResizeEW,          // When hovering over a vertical border or a column
    LinearUIMouseCursor_ResizeNESW,        // When hovering over the bottom-left corner of a window
    LinearUIMouseCursor_ResizeNWSE,        // When hovering over the bottom-right corner of a window
    LinearUIMouseCursor_Hand,              // (Unused by Dear LinearUI functions. Use for e.g. hyperlinks)
    LinearUIMouseCursor_Wait,              // When waiting for something to process/load.
    LinearUIMouseCursor_Progress,          // When waiting for something to process/load, but application is still interactive.
    LinearUIMouseCursor_NotAllowed,        // When hovering something with disallowed interaction. Usually a crossed circle.
    LinearUIMouseCursor_COUNT
};

// Enumeration for AddMouseSourceEvent() actual source of Mouse Input data.
// Historically we use "Mouse" terminology everywhere to indicate pointer data, e.g. MousePos, IsMousePressed(), io.AddMousePosEvent()
// But that "Mouse" data can come from different source which occasionally may be useful for application to know about.
// You can submit a change of pointer type using io.AddMouseSourceEvent().
enum LinearUIMouseSource : int
{
    LinearUIMouseSource_Mouse = 0,         // Input is coming from an actual mouse.
    LinearUIMouseSource_TouchScreen,       // Input is coming from a touch screen (no hovering prior to initial press, less precise initial press aiming, dual-axis wheeling possible).
    LinearUIMouseSource_Pen,               // Input is coming from a pressure/magnetic pen (often used in conjunction with high-sampling rates).
    LinearUIMouseSource_COUNT
};

// Enumeration for LinearUI::SetNextWindow***(), SetWindow***(), SetNextItem***() functions
// Represent a condition.
// Important: Treat as a regular enum! Do NOT combine multiple values using binary operators! All the functions above treat 0 as a shortcut to LinearUICond_Always.
enum LinearUICond_
{
    LinearUICond_None          = 0,        // No condition (always set the variable), same as _Always
    LinearUICond_Always        = 1 << 0,   // No condition (always set the variable), same as _None
    LinearUICond_Once          = 1 << 1,   // Set the variable once per runtime session (only the first call will succeed)
    LinearUICond_FirstUseEver  = 1 << 2,   // Set the variable if the object/window has no persistently saved data (no entry in .ini file)
    LinearUICond_Appearing     = 1 << 3,   // Set the variable if the object/window is appearing after being hidden/inactive (or the first time)
};

//-----------------------------------------------------------------------------
// [SECTION] Tables API flags and structures (LinearUITableFlags, LinearUITableColumnFlags, LinearUITableRowFlags, LinearUITableBgTarget, LinearUITableSortSpecs, LinearUITableColumnSortSpecs)
//-----------------------------------------------------------------------------

// Flags for LinearUI::BeginTable()
// - Important! Sizing policies have complex and subtle side effects, much more so than you would expect.
//   Read comments/demos carefully + experiment with live demos to get acquainted with them.
// - The DEFAULT sizing policies are:
//    - Default to LinearUITableFlags_SizingFixedFit    if ScrollX is on, or if host window has LinearUIWindowFlags_AlwaysAutoResize.
//    - Default to LinearUITableFlags_SizingStretchSame if ScrollX is off.
// - When ScrollX is off:
//    - Table defaults to LinearUITableFlags_SizingStretchSame -> all Columns defaults to LinearUITableColumnFlags_WidthStretch with same weight.
//    - Columns sizing policy allowed: Stretch (default), Fixed/Auto.
//    - Fixed Columns (if any) will generally obtain their requested width (unless the table cannot fit them all).
//    - Stretch Columns will share the remaining width according to their respective weight.
//    - Mixed Fixed/Stretch columns is possible but has various side-effects on resizing behaviors.
//      The typical use of mixing sizing policies is: any number of LEADING Fixed columns, followed by one or two TRAILING Stretch columns.
//      (this is because the visible order of columns have subtle but necessary effects on how they react to manual resizing).
// - When ScrollX is on:
//    - Table defaults to LinearUITableFlags_SizingFixedFit -> all Columns defaults to LinearUITableColumnFlags_WidthFixed
//    - Columns sizing policy allowed: Fixed/Auto mostly.
//    - Fixed Columns can be enlarged as needed. Table will show a horizontal scrollbar if needed.
//    - When using auto-resizing (non-resizable) fixed columns, querying the content width to use item right-alignment e.g. SetNextItemWidth(-FLT_MIN) doesn't make sense, would create a feedback loop.
//    - Using Stretch columns OFTEN DOES NOT MAKE SENSE if ScrollX is on, UNLESS you have specified a value for 'inner_width' in BeginTable().
//      If you specify a value for 'inner_width' then effectively the scrolling space is known and Stretch or mixed Fixed/Stretch columns become meaningful again.
// - Read on documentation at the top of linearui_tables.cpp for details.
enum LinearUITableFlags_
{
    // Features
    LinearUITableFlags_None                       = 0,
    LinearUITableFlags_Resizable                  = 1 << 0,   // Enable resizing columns.
    LinearUITableFlags_Reorderable                = 1 << 1,   // Enable reordering columns in header row. (Need calling TableSetupColumn() + TableHeadersRow() to display headers, or using LinearUITableFlags_ContextMenuInBody to access context-menu without headers).
    LinearUITableFlags_Hideable                   = 1 << 2,   // Enable hiding/disabling columns in context menu.
    LinearUITableFlags_Sortable                   = 1 << 3,   // Enable sorting. Call TableGetSortSpecs() to obtain sort specs. Also see LinearUITableFlags_SortMulti and LinearUITableFlags_SortTristate.
    LinearUITableFlags_NoSavedSettings            = 1 << 4,   // Disable persisting columns order, width, visibility and sort settings in the .ini file.
    LinearUITableFlags_ContextMenuInBody          = 1 << 5,   // Right-click on columns body/contents will also display table context menu. By default it is available in TableHeadersRow().
    // Decorations
    LinearUITableFlags_RowBg                      = 1 << 6,   // Set each RowBg color with LinearUICol_TableRowBg or LinearUICol_TableRowBgAlt (equivalent of calling TableSetBgColor with LinearUITableBgFlags_RowBg0 on each row manually)
    LinearUITableFlags_BordersInnerH              = 1 << 7,   // Draw horizontal borders between rows.
    LinearUITableFlags_BordersOuterH              = 1 << 8,   // Draw horizontal borders at the top and bottom.
    LinearUITableFlags_BordersInnerV              = 1 << 9,   // Draw vertical borders between columns.
    LinearUITableFlags_BordersOuterV              = 1 << 10,  // Draw vertical borders on the left and right sides.
    LinearUITableFlags_BordersH                   = LinearUITableFlags_BordersInnerH | LinearUITableFlags_BordersOuterH, // Draw horizontal borders.
    LinearUITableFlags_BordersV                   = LinearUITableFlags_BordersInnerV | LinearUITableFlags_BordersOuterV, // Draw vertical borders.
    LinearUITableFlags_BordersInner               = LinearUITableFlags_BordersInnerV | LinearUITableFlags_BordersInnerH, // Draw inner borders.
    LinearUITableFlags_BordersOuter               = LinearUITableFlags_BordersOuterV | LinearUITableFlags_BordersOuterH, // Draw outer borders.
    LinearUITableFlags_Borders                    = LinearUITableFlags_BordersInner | LinearUITableFlags_BordersOuter,   // Draw all borders.
    LinearUITableFlags_NoBordersInBody            = 1 << 11,  // [ALPHA] Disable vertical borders in columns Body (borders will always appear in Headers). -> May move to style
    LinearUITableFlags_NoBordersInBodyUntilResize = 1 << 12,  // [ALPHA] Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers). -> May move to style
    // Sizing Policy (read above for defaults)
    LinearUITableFlags_SizingFixedFit             = 1 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching contents width.
    LinearUITableFlags_SizingFixedSame            = 2 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching the maximum contents width of all columns. Implicitly enable LinearUITableFlags_NoKeepColumnsVisible.
    LinearUITableFlags_SizingStretchProp          = 3 << 13,  // Columns default to _WidthStretch with default weights proportional to each columns contents widths.
    LinearUITableFlags_SizingStretchSame          = 4 << 13,  // Columns default to _WidthStretch with default weights all equal, unless overridden by TableSetupColumn().
    // Sizing Extra Options
    LinearUITableFlags_NoHostExtendX              = 1 << 16,  // Make outer width auto-fit to columns, overriding outer_size.x value. Only available when ScrollX/ScrollY are disabled and Stretch columns are not used.
    LinearUITableFlags_NoHostExtendY              = 1 << 17,  // Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit). Only available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.
    LinearUITableFlags_NoKeepColumnsVisible       = 1 << 18,  // Disable keeping column always minimally visible when ScrollX is off and table gets too small. Not recommended if columns are resizable.
    LinearUITableFlags_PreciseWidths              = 1 << 19,  // Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.
    // Clipping
    LinearUITableFlags_NoClip                     = 1 << 20,  // Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with TableSetupScrollFreeze().
    // Padding
    LinearUITableFlags_PadOuterX                  = 1 << 21,  // Default if BordersOuterV is on. Enable outermost padding. Generally desirable if you have headers.
    LinearUITableFlags_NoPadOuterX                = 1 << 22,  // Default if BordersOuterV is off. Disable outermost padding.
    LinearUITableFlags_NoPadInnerX                = 1 << 23,  // Disable inner padding between columns (double inner padding if BordersOuterV is on, single inner padding if BordersOuterV is off).
    // Scrolling
    LinearUITableFlags_ScrollX                    = 1 << 24,  // Enable horizontal scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size. Changes default sizing policy. Because this creates a child window, ScrollY is currently generally recommended when using ScrollX.
    LinearUITableFlags_ScrollY                    = 1 << 25,  // Enable vertical scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size.
    // Sorting
    LinearUITableFlags_SortMulti                  = 1 << 26,  // Hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).
    LinearUITableFlags_SortTristate               = 1 << 27,  // Allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).
    // Miscellaneous
    LinearUITableFlags_HighlightHoveredColumn     = 1 << 28,  // Highlight column headers when hovered (may evolve into a fuller highlight)

    // [Internal] Combinations and masks
    LinearUITableFlags_SizingMask_                = LinearUITableFlags_SizingFixedFit | LinearUITableFlags_SizingFixedSame | LinearUITableFlags_SizingStretchProp | LinearUITableFlags_SizingStretchSame,
};

// Flags for LinearUI::TableSetupColumn()
enum LinearUITableColumnFlags_
{
    // Input configuration flags
    LinearUITableColumnFlags_None                  = 0,
    LinearUITableColumnFlags_Disabled              = 1 << 0,   // Overriding/master disable flag: hide column, won't show in context menu (unlike calling TableSetColumnEnabled() which manipulates the user accessible state)
    LinearUITableColumnFlags_DefaultHide           = 1 << 1,   // Default as a hidden/disabled column.
    LinearUITableColumnFlags_DefaultSort           = 1 << 2,   // Default as a sorting column.
    LinearUITableColumnFlags_WidthStretch          = 1 << 3,   // Column will stretch. Preferable with horizontal scrolling disabled (default if table sizing policy is _SizingStretchSame or _SizingStretchProp).
    LinearUITableColumnFlags_WidthFixed            = 1 << 4,   // Column will not stretch. Preferable with horizontal scrolling enabled (default if table sizing policy is _SizingFixedFit and table is resizable).
    LinearUITableColumnFlags_NoResize              = 1 << 5,   // Disable manual resizing.
    LinearUITableColumnFlags_NoReorder             = 1 << 6,   // Disable manual reordering this column, this will also prevent other columns from crossing over this column.
    LinearUITableColumnFlags_NoHide                = 1 << 7,   // Disable ability to hide/disable this column.
    LinearUITableColumnFlags_NoClip                = 1 << 8,   // Disable clipping for this column (all NoClip columns will render in a same draw command).
    LinearUITableColumnFlags_NoSort                = 1 << 9,   // Disable ability to sort on this field (even if LinearUITableFlags_Sortable is set on the table).
    LinearUITableColumnFlags_NoSortAscending       = 1 << 10,  // Disable ability to sort in the ascending direction.
    LinearUITableColumnFlags_NoSortDescending      = 1 << 11,  // Disable ability to sort in the descending direction.
    LinearUITableColumnFlags_NoHeaderLabel         = 1 << 12,  // TableHeadersRow() will submit an empty label for this column. Convenient for some small columns. Name will still appear in context menu or in angled headers. You may append into this cell by calling TableSetColumnIndex() right after the TableHeadersRow() call.
    LinearUITableColumnFlags_NoHeaderWidth         = 1 << 13,  // Disable header text width contribution to automatic column width.
    LinearUITableColumnFlags_PreferSortAscending   = 1 << 14,  // Make the initial sort direction Ascending when first sorting on this column (default).
    LinearUITableColumnFlags_PreferSortDescending  = 1 << 15,  // Make the initial sort direction Descending when first sorting on this column.
    LinearUITableColumnFlags_IndentEnable          = 1 << 16,  // Use current Indent value when entering cell (default for column 0).
    LinearUITableColumnFlags_IndentDisable         = 1 << 17,  // Ignore current Indent value when entering cell (default for columns > 0). Indentation changes _within_ the cell will still be honored.
    LinearUITableColumnFlags_AngledHeader          = 1 << 18,  // TableHeadersRow() will submit an angled header row for this column. Note this will add an extra row.

    // Output status flags, read-only via TableGetColumnFlags()
    LinearUITableColumnFlags_IsEnabled             = 1 << 24,  // Status: is enabled == not hidden by user/api (referred to as "Hide" in _DefaultHide and _NoHide) flags.
    LinearUITableColumnFlags_IsVisible             = 1 << 25,  // Status: is visible == is enabled AND not clipped by scrolling.
    LinearUITableColumnFlags_IsSorted              = 1 << 26,  // Status: is currently part of the sort specs
    LinearUITableColumnFlags_IsHovered             = 1 << 27,  // Status: is hovered by mouse

    // [Internal] Combinations and masks
    LinearUITableColumnFlags_WidthMask_            = LinearUITableColumnFlags_WidthStretch | LinearUITableColumnFlags_WidthFixed,
    LinearUITableColumnFlags_IndentMask_           = LinearUITableColumnFlags_IndentEnable | LinearUITableColumnFlags_IndentDisable,
    LinearUITableColumnFlags_StatusMask_           = LinearUITableColumnFlags_IsEnabled | LinearUITableColumnFlags_IsVisible | LinearUITableColumnFlags_IsSorted | LinearUITableColumnFlags_IsHovered,
    LinearUITableColumnFlags_NoDirectResize_       = 1 << 30,  // [Internal] Disable user resizing this column directly (it may however we resized indirectly from its left edge)
};

// Flags for LinearUI::TableNextRow()
enum LinearUITableRowFlags_
{
    LinearUITableRowFlags_None                     = 0,
    LinearUITableRowFlags_Headers                  = 1 << 0,   // Identify header row (set default background color + width of its contents accounted differently for auto column width)
};

// Enum for LinearUI::TableSetBgColor()
// Background colors are rendering in 3 layers:
//  - Layer 0: draw with RowBg0 color if set, otherwise draw with ColumnBg0 if set.
//  - Layer 1: draw with RowBg1 color if set, otherwise draw with ColumnBg1 if set.
//  - Layer 2: draw with CellBg color if set.
// The purpose of the two row/columns layers is to let you decide if a background color change should override or blend with the existing color.
// When using LinearUITableFlags_RowBg on the table, each row has the RowBg0 color automatically set for odd/even rows.
// If you set the color of RowBg0 target, your color will override the existing RowBg0 color.
// If you set the color of RowBg1 or ColumnBg1 target, your color will blend over the RowBg0 color.
enum LinearUITableBgTarget_
{
    LinearUITableBgTarget_None                     = 0,
    LinearUITableBgTarget_RowBg0                   = 1,        // Set row background color 0 (generally used for background, automatically set when LinearUITableFlags_RowBg is used)
    LinearUITableBgTarget_RowBg1                   = 2,        // Set row background color 1 (generally used for selection marking)
    LinearUITableBgTarget_CellBg                   = 3,        // Set cell background color (top-most color)
};

// Sorting specifications for a table (often handling sort specs for a single column, occasionally more)
// Obtained by calling TableGetSortSpecs().
// When 'SpecsDirty == true' you can sort your data. It will be true with sorting specs have changed since last call, or the first time.
// Make sure to set 'SpecsDirty = false' after sorting, else you may wastefully sort your data every frame!
struct LinearUITableSortSpecs
{
    const LinearUITableColumnSortSpecs* Specs;     // Pointer to sort spec array.
    int                         SpecsCount;     // Sort spec count. Most often 1. May be > 1 when LinearUITableFlags_SortMulti is enabled. May be == 0 when LinearUITableFlags_SortTristate is enabled.
    bool                        SpecsDirty;     // Set to true when specs have changed since last time! Use this to sort again, then clear the flag.

    LinearUITableSortSpecs()       { memset((void*)this, 0, sizeof(*this)); }
};

// Sorting specification for one column of a table (sizeof == 12 bytes)
struct LinearUITableColumnSortSpecs
{
    LinearUIID                     ColumnUserID;       // User id of the column (if specified by a TableSetupColumn() call)
    LinearS16                       ColumnIndex;        // Index of the column
    LinearS16                       SortOrder;          // Index within parent LinearUITableSortSpecs (always stored in order starting from 0, tables sorted on a single criteria will always have a 0 here)
    LinearUISortDirection          SortDirection;      // LinearUISortDirection_Ascending or LinearUISortDirection_Descending

    LinearUITableColumnSortSpecs() { memset((void*)this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Helpers: Debug log, memory allocations macros, LinearVector<>
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Debug Logging into ShowDebugLogWindow(), tty and more.
//-----------------------------------------------------------------------------

#ifndef LINEARUI_DISABLE_DEBUG_TOOLS
#define LINEARUI_DEBUG_LOG(...)        LinearUI::DebugLog(__VA_ARGS__)
#else
#define LINEARUI_DEBUG_LOG(...)        ((void)0)
#endif

//-----------------------------------------------------------------------------
// LINEARUI_MALLOC(), LINEARUI_FREE(), LINEARUI_NEW(), LINEARUI_PLACEMENT_NEW(), LINEARUI_DELETE()
// We call C++ constructor on own allocated memory via the placement "new(ptr) Type()" syntax.
// Defining a custom placement new() with a custom parameter allows us to bypass including <new> which on some platforms complains when user has disabled exceptions.
//-----------------------------------------------------------------------------

struct LinearNewWrapper {};
inline void* operator new(size_t, LinearNewWrapper, void* ptr) { return ptr; }
inline void  operator delete(void*, LinearNewWrapper, void*)   {} // This is only required so we can use the symmetrical new()
#define LINEARUI_ALLOC(_SIZE)                     LinearUI::MemAlloc(_SIZE)
#define LINEARUI_FREE(_PTR)                       LinearUI::MemFree(_PTR)
#define LINEARUI_PLACEMENT_NEW(_PTR)              new(LinearNewWrapper(), _PTR)
#define LINEARUI_NEW(_TYPE)                       new(LinearNewWrapper(), LinearUI::MemAlloc(sizeof(_TYPE))) _TYPE
template<typename T> void LINEARUI_DELETE(T* p)   { if (p) { p->~T(); LinearUI::MemFree(p); } }

//-----------------------------------------------------------------------------
// LinearVector<>
// Lightweight std::vector<>-like class to avoid dragging dependencies (also, some implementations of STL with debug enabled are absurdly slow, we bypass it so our code runs fast in debug).
//-----------------------------------------------------------------------------
// - You generally do NOT need to care or use this ever. But we need to make it available in linearui.h because some of our public structures are relying on it.
// - We use std-like naming convention here, which is a little unusual for this codebase.
// - Important: clear() frees memory, resize(0) keep the allocated buffer. We use resize(0) a lot to intentionally recycle allocated buffers across frames and amortize our costs.
// - Important: our implementation does NOT call C++ constructors/destructors, we treat everything as raw data! This is intentional but be extra mindful of that,
//   Do NOT use this class as a std::vector replacement in your own code! Many of the structures used by dear linearui can be safely initialized by a zero-memset.
//-----------------------------------------------------------------------------

LINEARUI_MSVC_RUNTIME_CHECKS_OFF
template<typename T>
struct LinearVector
{
    int                 Size;
    int                 Capacity;
    T*                  Data;

    // Provide standard typedefs but we don't use them ourselves.
    typedef T                   value_type;
    typedef value_type*         iterator;
    typedef const value_type*   const_iterator;

    // Constructors, destructor
    inline LinearVector()                                       { Size = Capacity = 0; Data = NULL; }
    inline LinearVector(const LinearVector<T>& src)                 { Size = Capacity = 0; Data = NULL; operator=(src); }
    inline LinearVector<T>& operator=(const LinearVector<T>& src)   { clear(); resize(src.Size); if (Data && src.Data) memcpy(Data, src.Data, (size_t)Size * sizeof(T)); return *this; }
    inline ~LinearVector()                                      { if (Data) LINEARUI_FREE(Data); } // Important: does not destruct anything

    inline void         clear()                             { if (Data) { Size = Capacity = 0; LINEARUI_FREE(Data); Data = NULL; } }  // Important: does not destruct anything
    inline void         clear_delete()                      { for (int n = 0; n < Size; n++) LINEARUI_DELETE(Data[n]); clear(); }     // Important: never called automatically! always explicit.
    inline void         clear_destruct()                    { for (int n = 0; n < Size; n++) Data[n].~T(); clear(); }           // Important: never called automatically! always explicit.

    inline bool         empty() const                       { return Size == 0; }
    inline int          size() const                        { return Size; }
    inline int          size_in_bytes() const               { return Size * (int)sizeof(T); }
    inline int          max_size() const                    { return 0x7FFFFFFF / (int)sizeof(T); }
    inline int          capacity() const                    { return Capacity; }
    inline T&           operator[](int i)                   { LINEARUI_ASSERT(i >= 0 && i < Size); return Data[i]; }
    inline const T&     operator[](int i) const             { LINEARUI_ASSERT(i >= 0 && i < Size); return Data[i]; }

    inline T*           begin()                             { return Data; }
    inline const T*     begin() const                       { return Data; }
    inline T*           end()                               { return Data + Size; }
    inline const T*     end() const                         { return Data + Size; }
    inline T&           front()                             { LINEARUI_ASSERT(Size > 0); return Data[0]; }
    inline const T&     front() const                       { LINEARUI_ASSERT(Size > 0); return Data[0]; }
    inline T&           back()                              { LINEARUI_ASSERT(Size > 0); return Data[Size - 1]; }
    inline const T&     back() const                        { LINEARUI_ASSERT(Size > 0); return Data[Size - 1]; }
    inline void         swap(LinearVector<T>& rhs)              { int rhs_size = rhs.Size; rhs.Size = Size; Size = rhs_size; int rhs_cap = rhs.Capacity; rhs.Capacity = Capacity; Capacity = rhs_cap; T* rhs_data = rhs.Data; rhs.Data = Data; Data = rhs_data; }

    inline int          _grow_capacity(int sz) const        { int new_capacity = Capacity ? (Capacity + Capacity / 2) : 8; return new_capacity > sz ? new_capacity : sz; }
    inline void         resize(int new_size)                { if (new_size > Capacity) reserve(_grow_capacity(new_size)); Size = new_size; }
    inline void         resize(int new_size, const T& v)    { if (new_size > Capacity) reserve(_grow_capacity(new_size)); if (new_size > Size) for (int n = Size; n < new_size; n++) memcpy(&Data[n], &v, sizeof(v)); Size = new_size; }
    inline void         shrink(int new_size)                { LINEARUI_ASSERT(new_size <= Size); Size = new_size; } // Resize a vector to a smaller size, guaranteed not to cause a reallocation
    inline void         reserve(int new_capacity)           { if (new_capacity <= Capacity) return; T* new_data = (T*)LINEARUI_ALLOC((size_t)new_capacity * sizeof(T)); if (Data) { memcpy(new_data, Data, (size_t)Size * sizeof(T)); LINEARUI_FREE(Data); } Data = new_data; Capacity = new_capacity; }
    inline void         reserve_discard(int new_capacity)   { if (new_capacity <= Capacity) return; if (Data) LINEARUI_FREE(Data); Data = (T*)LINEARUI_ALLOC((size_t)new_capacity * sizeof(T)); Capacity = new_capacity; }

    // NB: It is illegal to call push_back/push_front/insert with a reference pointing inside the LinearVector data itself! e.g. v.push_back(v[10]) is forbidden.
    inline void         push_back(const T& v)               { if (Size == Capacity) reserve(_grow_capacity(Size + 1)); memcpy(&Data[Size], &v, sizeof(v)); Size++; }
    inline void         pop_back()                          { LINEARUI_ASSERT(Size > 0); Size--; }
    inline void         push_front(const T& v)              { if (Size == 0) push_back(v); else insert(Data, v); }
    inline T*           erase(const T* it)                  { LINEARUI_ASSERT(it >= Data && it < Data + Size); const ptrdiff_t off = it - Data; memmove(Data + off, Data + off + 1, ((size_t)Size - (size_t)off - 1) * sizeof(T)); Size--; return Data + off; }
    inline T*           erase(const T* it, const T* it_last){ LINEARUI_ASSERT(it >= Data && it < Data + Size && it_last >= it && it_last <= Data + Size); const ptrdiff_t count = it_last - it; const ptrdiff_t off = it - Data; memmove(Data + off, Data + off + count, ((size_t)Size - (size_t)off - (size_t)count) * sizeof(T)); Size -= (int)count; return Data + off; }
    inline T*           erase_unsorted(const T* it)         { LINEARUI_ASSERT(it >= Data && it < Data + Size);  const ptrdiff_t off = it - Data; if (it < Data + Size - 1) memcpy(Data + off, Data + Size - 1, sizeof(T)); Size--; return Data + off; }
    inline T*           insert(const T* it, const T& v)     { LINEARUI_ASSERT(it >= Data && it <= Data + Size); const ptrdiff_t off = it - Data; if (Size == Capacity) reserve(_grow_capacity(Size + 1)); if (off < (int)Size) memmove(Data + off + 1, Data + off, ((size_t)Size - (size_t)off) * sizeof(T)); memcpy(&Data[off], &v, sizeof(v)); Size++; return Data + off; }
    inline bool         contains(const T& v) const          { const T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data++ == v) return true; return false; }
    inline T*           find(const T& v)                    { T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline const T*     find(const T& v) const              { const T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline int          find_index(const T& v) const        { const T* data_end = Data + Size; const T* it = find(v); if (it == data_end) return -1; const ptrdiff_t off = it - Data; return (int)off; }
    inline bool         find_erase(const T& v)              { const T* it = find(v); if (it < Data + Size) { erase(it); return true; } return false; }
    inline bool         find_erase_unsorted(const T& v)     { const T* it = find(v); if (it < Data + Size) { erase_unsorted(it); return true; } return false; }
    inline int          index_from_ptr(const T* it) const   { LINEARUI_ASSERT(it >= Data && it < Data + Size); const ptrdiff_t off = it - Data; return (int)off; }
};
LINEARUI_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] LinearUIStyle
//-----------------------------------------------------------------------------
// You may modify the LinearUI::GetStyle() main instance during initialization and before NewFrame().
// During the frame, use LinearUI::PushStyleVar(LinearUIStyleVar_XXXX)/PopStyleVar() to alter the main style values,
// and LinearUI::PushStyleColor(LinearUICol_XXX)/PopStyleColor() for colors.
//-----------------------------------------------------------------------------

struct LinearUIStyle
{
    // Font scaling
    // - recap: LinearUI::GetFontSize() == FontSizeBase * (FontScaleMain * FontScaleDpi * other_scaling_factors)
    float       FontSizeBase;               // Current base font size before external global factors are applied. Use PushFont(NULL, size) to modify. Use LinearUI::GetFontSize() to obtain scaled value.
    float       FontScaleMain;              // Main global scale factor. May be set by application once, or exposed to end-user.
    float       FontScaleDpi;               // Additional global scale factor from viewport/monitor contents scale. In docking branch: when io.ConfigDpiScaleFonts is enabled, this is automatically overwritten when changing monitor DPI.

    float       Alpha;                      // Global alpha applies to everything in Dear LinearUI.
    float       DisabledAlpha;              // Additional alpha multiplier applied by BeginDisabled(). Multiply over current value of Alpha.
    LinearVec2      WindowPadding;              // Padding within a window.
    float       WindowRounding;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows. Large values tend to lead to variety of artifacts and are not recommended.
    float       WindowBorderSize;           // Thickness of border around windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    float       WindowBorderHoverPadding;   // Hit-testing extent outside/inside resizing border. Also extend determination of hovered window. Generally meaningfully larger than WindowBorderSize to make it easy to reach borders.
    LinearVec2      WindowMinSize;              // Minimum window size. This is a global setting. If you want to constrain individual windows, use SetNextWindowSizeConstraints().
    LinearVec2      WindowTitleAlign;           // Alignment for title bar text. Defaults to (0.0f,0.5f) for left-aligned,vertically centered.
    LinearUIDir    WindowMenuButtonPosition;   // Side of the collapsing/docking button in the title bar (None/Left/Right). Defaults to LinearUIDir_Left.
    float       ChildRounding;              // Radius of child window corners rounding. Set to 0.0f to have rectangular windows.
    float       ChildBorderSize;            // Thickness of border around child windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    float       PopupRounding;              // Radius of popup window corners rounding. (Note that tooltip windows use WindowRounding)
    float       PopupBorderSize;            // Thickness of border around popup/tooltip windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    LinearVec2      FramePadding;               // Padding within a framed rectangle (used by most widgets).
    float       FrameRounding;              // Radius of frame corners rounding. Set to 0.0f to have rectangular frame (used by most widgets).
    float       FrameBorderSize;            // Thickness of border around frames. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
    LinearVec2      ItemSpacing;                // Horizontal and vertical spacing between widgets/lines.
    LinearVec2      ItemInnerSpacing;           // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label).
    LinearVec2      CellPadding;                // Padding within a table cell. Cellpadding.x is locked for entire table. CellPadding.y may be altered between different rows.
    LinearVec2      TouchExtraPadding;          // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
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
    float       TabMinWidthShrink;          // Minimum tab width after shrinking, when using LinearUITabBarFlags_FittingPolicyMixed policy.
    float       TabCloseButtonMinWidthSelected;     // -1: always visible. 0.0f: visible when hovered. >0.0f: visible when hovered if minimum width. FLT_MAX: never shrink, will behave like LinearUITabBarFlags_FittingPolicyScroll.
    float       TabCloseButtonMinWidthUnselected;   // -1: always visible. 0.0f: visible when hovered. >0.0f: visible when hovered if minimum width. FLT_MAX: never show close button when unselected.
    float       TabBarBorderSize;           // Thickness of tab-bar separator, which takes on the tab active color to denote focus.
    float       TabBarOverlineSize;         // Thickness of tab-bar overline, which highlights the selected tab-bar.
    float       TableAngledHeadersAngle;    // Angle of angled headers (supported values range from -50.0f degrees to +50.0f degrees).
    LinearVec2      TableAngledHeadersTextAlign;// Alignment of angled headers within the cell
    LinearUITreeNodeFlags TreeLinesFlags;      // Default way to draw lines connecting TreeNode hierarchy. LinearUITreeNodeFlags_DrawLinesNone or LinearUITreeNodeFlags_DrawLinesFull or LinearUITreeNodeFlags_DrawLinesToNodes.
    float       TreeLinesSize;              // Thickness of outlines when using LinearUITreeNodeFlags_DrawLines.
    float       TreeLinesRounding;          // Radius of lines connecting child nodes to the vertical line.
    float       DragDropTargetRounding;     // Radius of the drag and drop target frame. When <0.0f: use FrameRounding.
    float       DragDropTargetBorderSize;   // Thickness of the drag and drop target border.
    float       DragDropTargetPadding;      // Size to expand the drag and drop target from actual target item size.
    float       ColorMarkerSize;            // Size of R/G/B/A color markers for ColorEdit4() and for Drags/Sliders when using LinearUISliderFlags_ColorMarkers.
    LinearUIDir    ColorButtonPosition;        // Side of the color button in the ColorEdit4 widget (left/right). Defaults to LinearUIDir_Right.
    LinearVec2      ButtonTextAlign;            // Alignment of button text when button is larger than text. Defaults to (0.5f, 0.5f) (centered).
    LinearVec2      SelectableTextAlign;        // Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.
    float       InputTextCursorSize;        // Thickness of cursor/caret in InputText().
    float       SeparatorSize;              // Thickness of border in Separator(). Must be >= 1.0f.
    float       SeparatorTextBorderSize;    // Thickness of border in SeparatorText()
    LinearVec2      SeparatorTextAlign;         // Alignment of text within the separator. Defaults to (0.0f, 0.5f) (left aligned, center).
    LinearVec2      SeparatorTextPadding;       // Horizontal offset of text from each edge of the separator + spacing on other axis. Generally small values. .y is recommended to be == FramePadding.y.
    LinearVec2      DisplayWindowPadding;       // Apply to regular windows: amount which we enforce to keep visible when moving near edges of your screen.
    LinearVec2      DisplaySafeAreaPadding;     // Apply to every windows, menus, popups, tooltips: amount where we avoid displaying contents. Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).
    bool        DockingNodeHasCloseButton;  // Docking node has their own CloseButton() to close all docked windows.
    float       DockingSeparatorSize;       // Thickness of resizing border between docked windows
    float       MouseCursorScale;           // Scale software rendered mouse cursor (when io.MouseDrawCursor is enabled). We apply per-monitor DPI scaling over this scale. May be removed later.
    bool        AntiAliasedLines;           // Enable anti-aliased lines/borders. Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to LinearDrawList).
    bool        AntiAliasedLinesUseTex;     // Enable anti-aliased lines/borders using textures where possible. Require backend to render with bilinear filtering (NOT point/nearest filtering). Latched at the beginning of the frame (copied to LinearDrawList).
    bool        AntiAliasedFill;            // Enable anti-aliased edges around filled shapes (rounded rectangles, circles, etc.). Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to LinearDrawList).
    float       CurveTessellationTol;       // Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
    float       CircleTessellationMaxError; // Maximum error (in pixels) allowed when using AddCircle()/AddCircleFilled() or drawing rounded corner rectangles with no explicit segment count specified. Decrease for higher quality but more geometry.

    // Colors
    LinearVec4      Colors[LinearUICol_COUNT];

    // Behaviors
    // (It is possible to modify those fields mid-frame if specific behavior need it, unlike e.g. configuration fields in LinearUIIO)
    float             HoverStationaryDelay;     // Delay for IsItemHovered(LinearUIHoveredFlags_Stationary). Time required to consider mouse stationary.
    float             HoverDelayShort;          // Delay for IsItemHovered(LinearUIHoveredFlags_DelayShort). Usually used along with HoverStationaryDelay.
    float             HoverDelayNormal;         // Delay for IsItemHovered(LinearUIHoveredFlags_DelayNormal). "
    LinearUIHoveredFlags HoverFlagsForTooltipMouse;// Default flags when using IsItemHovered(LinearUIHoveredFlags_ForTooltip) or BeginItemTooltip()/SetItemTooltip() while using mouse.
    LinearUIHoveredFlags HoverFlagsForTooltipNav;  // Default flags when using IsItemHovered(LinearUIHoveredFlags_ForTooltip) or BeginItemTooltip()/SetItemTooltip() while using keyboard/gamepad.

    // [Internal]
    float       _MainScale;                 // FIXME-WIP: Reference scale, as applied by ScaleAllSizes(). PLEASE DO NOT USE THIS FOR NOW.
    float       _NextFrameFontSizeBase;     // FIXME: Temporary hack until we finish remaining work.

    // Functions
    LINEARUI_API   LinearUIStyle();
    LINEARUI_API   void ScaleAllSizes(float scale_factor); // Scale all spacing/padding/thickness values. Do not scale fonts. See comments in definition. Consider not calling this if your initial scale factor if <1.0.

    // Obsolete names
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    // TabMinWidthForCloseButton = TabCloseButtonMinWidthUnselected // Renamed in 1.91.9.
#endif
};

//-----------------------------------------------------------------------------
// [SECTION] LinearUIIO
//-----------------------------------------------------------------------------
// Communicate most settings and inputs/outputs to Dear LinearUI using this structure.
// Access via LinearUI::GetIO(). Read 'Programmer guide' section in .cpp file for general usage.
// It is generally expected that:
// - initialization: backends and user code writes to LinearUIIO.
// - main loop: backends writes to LinearUIIO, user code and linearui code reads from LinearUIIO.
//-----------------------------------------------------------------------------
// Also see LinearUI::GetPlatformIO() and LinearUIPlatformIO struct for OS/platform related functions: clipboard, IME etc.
//-----------------------------------------------------------------------------

// [Internal] Storage used by IsKeyDown(), IsKeyPressed() etc functions.
// If prior to 1.87 you used io.KeysDownDuration[] (which was marked as internal), you should use GetKeyData(key)->DownDuration and *NOT* io.KeysData[key]->DownDuration.
struct LinearUIKeyData
{
    bool        Down;               // True for if key is down
    float       DownDuration;       // Duration the key has been down (<0.0f: not pressed, 0.0f: just pressed, >0.0f: time held)
    float       DownDurationPrev;   // Last frame duration the key has been down
    float       AnalogValue;        // 0.0f..1.0f for gamepad values
};

struct LinearUIIO
{
    //------------------------------------------------------------------
    // Configuration                            // Default value
    //------------------------------------------------------------------

    LinearUIConfigFlags   ConfigFlags;             // = 0              // See LinearUIConfigFlags_ enum. Set by user/application. Keyboard/Gamepad navigation options, etc.
    LinearUIBackendFlags  BackendFlags;            // = 0              // See LinearUIBackendFlags_ enum. Set by backend (linearui_impl_xxx files or custom backend) to communicate features supported by the backend.
    LinearVec2      DisplaySize;                    // <unset>          // Main display size, in pixels (== GetMainViewport()->Size). May change every frame.
    LinearVec2      DisplayFramebufferScale;        // = (1, 1)         // Main display density. For retina display where window coordinates are different from framebuffer coordinates. This will affect font density + will end up in LinearDrawData::FramebufferScale.
    float       DeltaTime;                      // = 1.0f/60.0f     // Time elapsed since last frame, in seconds. May change every frame.
    float       IniSavingRate;                  // = 5.0f           // Minimum time between saving positions/sizes to .ini file, in seconds.
    const char* IniFilename;                    // = "linearui.ini"    // Path to .ini file (important: default "linearui.ini" is relative to current working dir!). Set NULL to disable automatic .ini loading/saving or if you want to manually call LoadIniSettingsXXX() / SaveIniSettingsXXX() functions.
    const char* LogFilename;                    // = "linearui_log.txt"// Path to .log file (default parameter to LinearUI::LogToFile when no file is specified).
    void*       UserData;                       // = NULL           // Store your own data.

    // Font system
    LinearFontAtlas*Fonts;                          // <auto>           // Font atlas: load, rasterize and pack one or more fonts into a single texture.
    LinearFont*     FontDefault;                    // = NULL           // Font to use on NewFrame(). Use NULL to uses Fonts->Fonts[0].
    bool        FontAllowUserScaling;           // = false          // Allow user scaling text of individual window with Ctrl+Wheel.

    // Keyboard/Gamepad Navigation options
    bool        ConfigNavSwapGamepadButtons;    // = false          // Swap Activate<>Cancel (A<>B) buttons, matching typical "Nintendo/Japanese style" gamepad layout.
    bool        ConfigNavMoveSetMousePos;       // = false          // Directional/tabbing navigation teleports the mouse cursor. May be useful on TV/console systems where moving a virtual mouse is difficult. Will update io.MousePos and set io.WantSetMousePos=true.
    bool        ConfigNavCaptureKeyboard;       // = true           // Sets io.WantCaptureKeyboard when io.NavActive is set.
    bool        ConfigNavEscapeClearFocusItem;  // = true           // Pressing Escape can clear focused item + navigation id/highlight. Set to false if you want to always keep highlight on.
    bool        ConfigNavEscapeClearFocusWindow;// = false          // Pressing Escape can clear focused window as well (super set of io.ConfigNavEscapeClearFocusItem).
    bool        ConfigNavCursorVisibleAuto;     // = true           // Using directional navigation key makes the cursor visible. Mouse click hides the cursor.
    bool        ConfigNavCursorVisibleAlways;   // = false          // Navigation cursor is always visible.

    // Docking options (when LinearUIConfigFlags_DockingEnable is set)
    bool        ConfigDockingNoSplit;           // = false          // Simplified docking mode: disable window splitting, so docking is limited to merging multiple windows together into tab-bars.
    bool        ConfigDockingNoDockingOver;     // = false          // Simplified docking mode: disable window merging into a same tab-bar, so docking is limited to splitting windows.
    bool        ConfigDockingWithShift;         // = false          // Enable docking with holding Shift key (reduce visual noise, allows dropping in wider space)
    bool        ConfigDockingAlwaysTabBar;      // = false          // [BETA] [FIXME: This currently creates regression with auto-sizing and general overhead] Make every single floating window display within a docking node.
    bool        ConfigDockingTransparentPayload;// = false          // [BETA] Make window or viewport transparent when docking and only display docking boxes on the target viewport. Useful if rendering of multiple viewport cannot be synced. Best used with ConfigViewportsNoAutoMerge.

    // Viewport options (when LinearUIConfigFlags_ViewportsEnable is set)
    // (sorry for the amount of "NoXXXX" flags, which may be harder to reason about! may rework someday)
    bool        ConfigViewportsNoAutoMerge;     // = false;         // Set to make all floating linearui windows always create their own viewport. Otherwise, they are merged into the main host viewports when overlapping it. May also set LinearUIViewportFlags_NoAutoMerge on individual viewport.
    bool        ConfigViewportsNoTaskBarIcon;   // = false          // Disable default OS task bar icon flag for secondary viewports. When a viewport doesn't want a task bar icon, LinearUIViewportFlags_NoTaskBarIcon will be set on it.
    bool        ConfigViewportsNoDecoration;    // = true           // Disable default OS window decoration flag for secondary viewports. When a viewport doesn't want window decorations, LinearUIViewportFlags_NoDecoration will be set on it. Enabling decoration can create subsequent issues at OS levels (e.g. minimum window size).
    bool        ConfigViewportsNoDefaultParent; // = true           // Disable setting OS window parent to main viewport by default. The platform backend is expected to honor `viewport->ParentViewportID` to setup a parent/child relationship between the OS windows (supported if LinearUIBackendFlags_HasParentViewport is set). When parented: child windows always appear in front of their parent. Set to false if you want viewports to automatically be parent of main viewport, otherwise all viewports will be top-level OS windows. Parent/child relationship may be set on a per-window basis using LinearUIWindowClass.
    bool        ConfigViewportsPlatformFocusSetsLinearUIFocus;//= true // When a platform window is focused (e.g. using Alt+Tab, clicking Platform Title Bar), apply corresponding focus on linearui windows (may clear focus/active id from linearui windows location in other platform windows). In principle this is better enabled but we provide an opt-out, because some Linux window managers tend to eagerly focus windows (e.g. on mouse hover, or even a simple window pos/size change).

    // DPI/Scaling options
    // This may keep evolving during 1.92.x releases. Expect some turbulence.
    bool        ConfigDpiScaleFonts;            // = false          // [EXPERIMENTAL] Automatically overwrite style.FontScaleDpi when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    bool        ConfigDpiScaleViewports;        // = false          // [EXPERIMENTAL] Scale Dear LinearUI and Platform Windows when Monitor DPI changes.

    // Miscellaneous options
    // (you can visualize and interact with all options in 'Demo->Configuration')
    bool        MouseDrawCursor;                // = false          // Request LinearUI to draw a mouse cursor for you (if you are on a platform without a mouse cursor). Cannot be easily renamed to 'io.ConfigXXX' because this is frequently used by backend implementations.
    bool        ConfigMacOSXBehaviors;          // = defined(__APPLE__) // Swap Cmd<>Ctrl keys + OS X style text editing cursor movement using Alt instead of Ctrl, Shortcuts using Cmd/Super instead of Ctrl, Line/Text Start and End using Cmd+Arrows instead of Home/End, Double click selects by word instead of selecting whole text, Multi-selection in lists uses Cmd/Super instead of Ctrl.
    bool        ConfigInputTrickleEventQueue;   // = true           // Enable input queue trickling: some types of events submitted during the same frame (e.g. button down + up) will be spread over multiple frames, improving interactions with low framerates.
    bool        ConfigInputTextCursorBlink;     // = true           // Enable blinking cursor (optional as some users consider it to be distracting).
    bool        ConfigInputTextEnterKeepActive; // = false          // [BETA] Pressing Enter will reactivate item and select all text (single-line only).
    bool        ConfigDragClickToInputText;     // = false          // [BETA] Enable turning DragXXX widgets into text input with a simple mouse click-release (without moving). Not desirable on devices without a keyboard.
    bool        ConfigWindowsResizeFromEdges;   // = true           // Enable resizing of windows from their edges and from the lower-left corner. This requires LinearUIBackendFlags_HasMouseCursors for better mouse cursor feedback. (This used to be a per-window LinearUIWindowFlags_ResizeFromAnySide flag)
    bool        ConfigWindowsMoveFromTitleBarOnly;  // = false      // Enable allowing to move windows only when clicking on their title bar. Does not apply to windows without a title bar.
    bool        ConfigWindowsCopyContentsWithCtrlC; // = false      // [EXPERIMENTAL] Ctrl+C copy the contents of focused window into the clipboard. Experimental because: (1) has known issues with nested Begin/End pairs (2) text output quality varies (3) text output is in submission order rather than spatial order.
    bool        ConfigScrollbarScrollByPage;    // = true           // Enable scrolling page by page when clicking outside the scrollbar grab. When disabled, always scroll to clicked location. When enabled, Shift+Click scrolls to clicked location.
    float       ConfigMemoryCompactTimer;       // = 60.0f          // Timer (in seconds) to free transient windows/tables memory buffers when unused. Set to -1.0f to disable.

    // Inputs Behaviors
    // (other variables, ones which are expected to be tweaked within UI code, are exposed in LinearUIStyle)
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
    // - Functions that support error recovery are using LINEARUI_ASSERT_USER_ERROR() instead of LINEARUI_ASSERT().
    // - By design, we do NOT allow error recovery to be 100% silent. One of the three options needs to be checked!
    // - Always ensure that on programmers seats you have at minimum Asserts or Tooltips enabled when making direct linearui API calls!
    //   Otherwise it would severely hinder your ability to catch and correct mistakes!
    // Read https://github.com/ocornut/linearui/wiki/Error-Handling for details.
    // - Programmer seats: keep asserts (default), or disable asserts and keep error tooltips (new and nice!)
    // - Non-programmer seats: maybe disable asserts, but make sure errors are resurfaced (tooltips, visible log entries, use callback etc.)
    // - Recovery after error/exception: record stack sizes with ErrorRecoveryStoreState(), disable assert, set log callback (to e.g. trigger high-level breakpoint), recover with ErrorRecoveryTryToRecoverState(), restore settings.
    bool        ConfigErrorRecovery;                // = true       // Enable error recovery support. Some errors won't be detected and lead to direct crashes if recovery is disabled.
    bool        ConfigErrorRecoveryEnableAssert;    // = true       // Enable asserts on recoverable error. By default call LINEARUI_ASSERT() when returning from a failing LINEARUI_ASSERT_USER_ERROR()
    bool        ConfigErrorRecoveryEnableDebugLog;  // = true       // Enable debug log output on recoverable errors.
    bool        ConfigErrorRecoveryEnableTooltip;   // = true       // Enable tooltip on recoverable errors. The tooltip include a way to enable asserts if they were disabled.

    // Option to enable various debug tools showing buttons that will call the LINEARUI_DEBUG_BREAK() macro.
    // - The Item Picker tool will be available regardless of this being enabled, in order to maximize its discoverability.
    // - Requires a debugger being attached, otherwise LINEARUI_DEBUG_BREAK() options will appear to crash your application.
    //   e.g. io.ConfigDebugIsDebuggerPresent = ::IsDebuggerPresent() on Win32, or refer to LinearOsIsDebuggerPresent() linearui_test_engine/linearui_te_utils.cpp for a Unix compatible version.
    bool        ConfigDebugIsDebuggerPresent;   // = false          // Enable various tools calling LINEARUI_DEBUG_BREAK().

    // Tools to detect code submitting items with conflicting/duplicate IDs
    // - Code should use PushID()/PopID() in loops, or append "##xx" to same-label identifiers.
    // - Empty label e.g. Button("") == same ID as parent widget/node. Use Button("##xx") instead!
    // - See FAQ https://github.com/ocornut/linearui/blob/master/docs/FAQ.md#q-about-the-id-stack-system
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
    // (the linearui_impl_xxxx backend files are setting those up for you)
    //------------------------------------------------------------------

    // Nowadays those would be stored in LinearUIPlatformIO but we are leaving them here for legacy reasons.
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
    LINEARUI_API void  AddKeyEvent(LinearUIKey key, bool down);                   // Queue a new key down/up event. Key should be "translated" (as in, generally LinearUIKey_A matches the key end-user would use to emit an 'A' character)
    LINEARUI_API void  AddKeyAnalogEvent(LinearUIKey key, bool down, float v);    // Queue a new key down/up event for analog values (e.g. LinearUIKey_Gamepad_ values). Dead-zones should be handled by the backend.
    LINEARUI_API void  AddMousePosEvent(float x, float y);                     // Queue a mouse position update. Use -FLT_MAX,-FLT_MAX to signify no mouse (e.g. app not focused and not hovered)
    LINEARUI_API void  AddMouseButtonEvent(int button, bool down);             // Queue a mouse button change
    LINEARUI_API void  AddMouseWheelEvent(float wheel_x, float wheel_y);       // Queue a mouse wheel update. wheel_y<0: scroll down, wheel_y>0: scroll up, wheel_x<0: scroll right, wheel_x>0: scroll left.
    LINEARUI_API void  AddMouseSourceEvent(LinearUIMouseSource source);           // Queue a mouse source change (Mouse/TouchScreen/Pen)
    LINEARUI_API void  AddMouseViewportEvent(LinearUIID id);                      // Queue a mouse hovered viewport. Requires backend to set LinearUIBackendFlags_HasMouseHoveredViewport to call this (for multi-viewport support).
    LINEARUI_API void  AddFocusEvent(bool focused);                            // Queue a gain/loss of focus for the application (generally based on OS/platform focus of your window)
    LINEARUI_API void  AddInputCharacter(unsigned int c);                      // Queue a new character input
    LINEARUI_API void  AddInputCharacterUTF16(LinearWchar16 c);                    // Queue a new character input from a UTF-16 character, it can be a surrogate
    LINEARUI_API void  AddInputCharactersUTF8(const char* str);                // Queue a new characters input from a UTF-8 string

    LINEARUI_API void  SetKeyEventNativeData(LinearUIKey key, int native_keycode, int native_scancode, int native_legacy_index = -1); // [Optional] Specify index for legacy <1.87 IsKeyXXX() functions with native indices + specify native keycode, scancode.
    LINEARUI_API void  SetAppAcceptingEvents(bool accepting_events);           // Set master flag for accepting key/mouse/text events (default to true). Useful if you have native dialog boxes that are interrupting your application loop/refresh, and you want to disable events being queued while your app is frozen.
    LINEARUI_API void  ClearEventsQueue();                                     // Clear all incoming events.
    LINEARUI_API void  ClearInputKeys();                                       // Clear current keyboard/gamepad state + current frame text input buffer. Equivalent to releasing all keys/buttons.
    LINEARUI_API void  ClearInputMouse();                                      // Clear current mouse state.

    //------------------------------------------------------------------
    // Output - Updated by NewFrame() or EndFrame()/Render()
    // (when reading from the io.WantCaptureMouse, io.WantCaptureKeyboard flags to dispatch your inputs, it is
    //  generally easier and more correct to use their state BEFORE calling NewFrame(). See FAQ for details!)
    //------------------------------------------------------------------

    bool        WantCaptureMouse;                   // Set when Dear LinearUI will use mouse inputs, in this case do not dispatch them to your main game/application (either way, always pass on mouse inputs to linearui). (e.g. unclicked mouse is hovering over an linearui window, widget is active, mouse was clicked over an linearui window, etc.).
    bool        WantCaptureKeyboard;                // Set when Dear LinearUI will use keyboard inputs, in this case do not dispatch them to your main game/application (either way, always pass keyboard inputs to linearui). (e.g. InputText active, or an linearui window is focused and navigation is enabled, etc.).
    bool        WantTextInput;                      // Mobile/console: when set, you may display an on-screen keyboard. This is set by Dear LinearUI when it wants textual keyboard input to happen (e.g. when a InputText widget is active).
    bool        WantSetMousePos;                    // MousePos has been altered, backend should reposition mouse on next frame. Rarely used! Set only when io.ConfigNavMoveSetMousePos is enabled.
    bool        WantSaveIniSettings;                // When manual .ini load/save is active (io.IniFilename == NULL), this will be set to notify your application that you can call SaveIniSettingsToMemory() and save yourself. Important: clear io.WantSaveIniSettings yourself after saving!
    bool        NavActive;                          // Keyboard/Gamepad navigation is currently allowed (will handle LinearUIKey_NavXXX events) = a window is focused and it doesn't use the LinearUIWindowFlags_NoNavInputs flag.
    bool        NavVisible;                         // Keyboard/Gamepad navigation highlight is visible and allowed (will handle LinearUIKey_NavXXX events).
    float       Framerate;                          // Estimate of application framerate (rolling average over 60 frames, based on io.DeltaTime), in frame per second. Solely for convenience. Slow applications may not want to use a moving average or may want to reset underlying buffers occasionally.
    int         MetricsRenderVertices;              // Vertices output during last call to Render()
    int         MetricsRenderIndices;               // Indices output during last call to Render() = number of triangles * 3
    int         MetricsRenderWindows;               // Number of visible windows
    int         MetricsActiveWindows;               // Number of active windows
    LinearVec2      MouseDelta;                         // Mouse delta. Note that this is zero if either current or previous position are invalid (-FLT_MAX,-FLT_MAX), so a disappearing/reappearing mouse won't have a huge delta.

    //------------------------------------------------------------------
    // [Internal] Dear LinearUI will maintain those fields. Forward compatibility not guaranteed!
    //------------------------------------------------------------------

    LinearUIContext* Ctx;                              // Parent UI context (needs to be set explicitly by parent).

    // Main Input State
    // (this block used to be written by backend, since 1.87 it is best to NOT write to those directly, call the AddXXX functions above instead)
    // (reading from those variables is fair game, as they are extremely unlikely to be moving anywhere)
    LinearVec2      MousePos;                           // Mouse position, in pixels. Set to LinearVec2(-FLT_MAX, -FLT_MAX) if mouse is unavailable (on another screen, etc.)
    bool        MouseDown[5];                       // Mouse buttons: 0=left, 1=right, 2=middle + extras (LinearUIMouseButton_COUNT == 5). Dear LinearUI mostly uses left and right buttons. Other buttons allow us to track if the mouse is being used by your application + available to user as a convenience via IsMouse** API.
    float       MouseWheel;                         // Mouse wheel Vertical: 1 unit scrolls about 5 lines text. >0 scrolls Up, <0 scrolls Down. Hold Shift to turn vertical scroll into horizontal scroll.
    float       MouseWheelH;                        // Mouse wheel Horizontal. >0 scrolls Left, <0 scrolls Right. Most users don't have a mouse with a horizontal wheel, may not be filled by all backends.
    LinearUIMouseSource MouseSource;                   // Mouse actual input peripheral (Mouse/TouchScreen/Pen).
    LinearUIID     MouseHoveredViewport;               // (Optional) Modify using io.AddMouseViewportEvent(). With multi-viewports: viewport the OS mouse is hovering. If possible _IGNORING_ viewports with the LinearUIViewportFlags_NoInputs flag is much better (few backends can handle that). Set io.BackendFlags |= LinearUIBackendFlags_HasMouseHoveredViewport if you can provide this info. If you don't linearui will infer the value using the rectangles and last focused time of the viewports it knows about (ignoring other OS windows).
    bool        KeyCtrl;                            // Keyboard modifier down: Ctrl (non-macOS), Cmd (macOS)
    bool        KeyShift;                           // Keyboard modifier down: Shift
    bool        KeyAlt;                             // Keyboard modifier down: Alt
    bool        KeySuper;                           // Keyboard modifier down: Windows/Super (non-macOS), Ctrl (macOS)

    // Other state maintained from data above + IO function calls
    LinearUIKeyChord KeyMods;                          // Key mods flags (any of LinearUIMod_Ctrl/LinearUIMod_Shift/LinearUIMod_Alt/LinearUIMod_Super flags, same as io.KeyCtrl/KeyShift/KeyAlt/KeySuper but merged into flags). Read-only, updated by NewFrame()
    LinearUIKeyData  KeysData[LinearUIKey_NamedKey_COUNT];// Key state for all known keys. MUST use 'key - LinearUIKey_NamedKey_BEGIN' as index. Use IsKeyXXX() functions to access this.
    bool        WantCaptureMouseUnlessPopupClose;   // Alternative to WantCaptureMouse: (WantCaptureMouse == true && WantCaptureMouseUnlessPopupClose == false) when a click over void is expected to close a popup.
    LinearVec2      MousePosPrev;                       // Previous mouse position (note that MouseDelta is not necessary == MousePos-MousePosPrev, in case either position is invalid)
    LinearVec2      MouseClickedPos[5];                 // Position at time of clicking
    double      MouseClickedTime[5];                // Time of last click (used to figure out double-click)
    bool        MouseClicked[5];                    // Mouse button went from !Down to Down (same as MouseClickedCount[x] != 0)
    bool        MouseDoubleClicked[5];              // Has mouse button been double-clicked? (same as MouseClickedCount[x] == 2)
    LinearU16       MouseClickedCount[5];               // == 0 (not clicked), == 1 (same as MouseClicked[]), == 2 (double-clicked), == 3 (triple-clicked) etc. when going from !Down to Down
    LinearU16       MouseClickedLastCount[5];           // Count successive number of clicks. Stays valid after mouse release. Reset after another click is done.
    bool        MouseReleased[5];                   // Mouse button went from Down to !Down
    double      MouseReleasedTime[5];               // Time of last released (rarely used! but useful to handle delayed single-click when trying to disambiguate them from double-click).
    bool        MouseDownOwned[5];                  // Track if button was clicked inside a dear linearui window or over void blocked by a popup. We don't request mouse capture from the application if click started outside LinearUI bounds.
    bool        MouseDownOwnedUnlessPopupClose[5];  // Track if button was clicked inside a dear linearui window.
    bool        MouseWheelRequestAxisSwap;          // On a non-Mac system, holding Shift requests WheelY to perform the equivalent of a WheelX event. On a Mac system this is already enforced by the system.
    bool        MouseCtrlLeftAsRightClick;          // (OSX) Set to true when the current click was a Ctrl+Click that spawned a simulated right click
    float       MouseDownDuration[5];               // Duration the mouse button has been down (0.0f == just clicked)
    float       MouseDownDurationPrev[5];           // Previous time the mouse button has been down
    LinearVec2      MouseDragMaxDistanceAbs[5];         // Maximum distance, absolute, on each axis, of how much mouse has traveled from the clicking point
    float       MouseDragMaxDistanceSqr[5];         // Squared maximum distance of how much mouse has traveled from the clicking point (used for moving thresholds)
    float       PenPressure;                        // Touch/Pen pressure (0.0f to 1.0f, should be >0.0f only when MouseDown[0] == true). Helper storage currently unused by Dear LinearUI.
    bool        AppFocusLost;                       // Only modify via AddFocusEvent()
    bool        AppAcceptingEvents;                 // Only modify via SetAppAcceptingEvents()
    LinearWchar16   InputQueueSurrogate;                // For AddInputCharacterUTF16()
    LinearVector<LinearWchar> InputQueueCharacters;         // Queue of _characters_ input (obtained by platform backend). Fill using AddInputCharacter() helper.

    // Legacy: before 1.87, we required backend to fill io.KeyMap[] (linearui->native map) during initialization and io.KeysDown[] (native indices) every frame.
    // This is still temporarily supported as a legacy feature. However the new preferred scheme is for backend to call io.AddKeyEvent().
    //   Old (<1.87):  LinearUI::IsKeyPressed(LinearUI::GetIO().KeyMap[LinearUIKey_Space]) --> New (1.87+) LinearUI::IsKeyPressed(LinearUIKey_Space)
    //   Old (<1.87):  LinearUI::IsKeyPressed(MYPLATFORM_KEY_SPACE)                  --> New (1.87+) LinearUI::IsKeyPressed(LinearUIKey_Space)
    // Read https://github.com/ocornut/linearui/issues/4921 for details.
    //int       KeyMap[LinearUIKey_COUNT];             // [LEGACY] Input: map of indices into the KeysDown[512] entries array which represent your "native" keyboard state. The first 512 are now unused and should be kept zero. Legacy backend will write into KeyMap[] using LinearUIKey_ indices which are always >512.
    //bool      KeysDown[LinearUIKey_COUNT];           // [LEGACY] Input: Keyboard keys that are pressed (ideally left in the "native" order your engine has access to keyboard keys, so you can use your own defines/enums for keys). This used to be [512] sized. It is now LinearUIKey_COUNT to allow legacy io.KeysDown[GetKeyIndex(...)] to work without an overflow.
    //float     NavInputs[LinearUINavInput_COUNT];     // [LEGACY] Since 1.88, NavInputs[] was removed. Backends from 1.60 to 1.86 won't build. Feed gamepad inputs via io.AddKeyEvent() and LinearUIKey_GamepadXXX enums.
    //void*     ImeWindowHandle;                    // [Obsoleted in 1.87] Set LinearUIViewport::PlatformHandleRaw instead. Set this to your HWND to get automatic IME cursor positioning.

#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    float       FontGlobalScale;                    // Moved io.FontGlobalScale to style.FontScaleMain in 1.92.0 (June 2025)

    // Legacy: before 1.91.1, clipboard functions were stored in LinearUIIO instead of LinearUIPlatformIO.
    // As this is will affect all users of custom engines/backends, we are providing proper legacy redirection (will obsolete).
    const char* (*GetClipboardTextFn)(void* user_data);
    void        (*SetClipboardTextFn)(void* user_data, const char* text);
    void*       ClipboardUserData;

    //void ClearInputCharacters() { InputQueueCharacters.resize(0); } // [Obsoleted in 1.89.8] Clear the current frame text input buffer. Now included within ClearInputKeys(). Removed this as it is ambiguous/misleading and generally incorrect to use with the existence of a higher-level input queue.
#endif

    LINEARUI_API   LinearUIIO();
};

//-----------------------------------------------------------------------------
// [SECTION] Misc data structures (LinearUIInputTextCallbackData, LinearUISizeCallbackData, LinearUIPayload)
//-----------------------------------------------------------------------------

// Shared state of InputText(), passed as an argument to your callback when a LinearUIInputTextFlags_Callback* flag is used.
// The callback function should return 0 by default.
// Callbacks (follow a flag name and see comments in LinearUIInputTextFlags_ declarations for more details)
// - LinearUIInputTextFlags_CallbackEdit:        Callback on buffer edit. Note that InputText() already returns true on edit + you can always use IsItemEdited(). The callback is useful to manipulate the underlying buffer while focus is active.
// - LinearUIInputTextFlags_CallbackAlways:      Callback on each iteration
// - LinearUIInputTextFlags_CallbackCompletion:  Callback on pressing TAB
// - LinearUIInputTextFlags_CallbackHistory:     Callback on pressing Up/Down arrows
// - LinearUIInputTextFlags_CallbackCharFilter:  Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
// - LinearUIInputTextFlags_CallbackResize:      Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow.
struct LinearUIInputTextCallbackData
{
    LinearUIContext*       Ctx;            // Parent UI context
    LinearUIInputTextFlags EventFlag;      // One LinearUIInputTextFlags_Callback*    // Read-only
    LinearUIInputTextFlags Flags;          // What user passed to InputText()      // Read-only
    void*               UserData;       // What user passed to InputText()      // Read-only
    LinearUIID             ID;             // Widget ID                            // Read-only

    // Arguments for the different callback events
    // - During Resize callback, Buf will be same as your input buffer.
    // - However, during Completion/History/Always callback, Buf always points to our own internal data (it is not the same as your buffer)! Changes to it will be reflected into your own buffer shortly after the callback.
    // - To modify the text buffer in a callback, prefer using the InsertChars() / DeleteChars() function. InsertChars() will take care of calling the resize callback if necessary.
    // - If you know your edits are not going to resize the underlying buffer allocation, you may modify the contents of 'Buf[]' directly. You need to update 'BufTextLen' accordingly (0 <= BufTextLen < BufSize) and set 'BufDirty'' to true so InputText can update its internal state.
    LinearUIKey            EventKey;       // Key pressed (Up/Down/TAB)            // Read-only    // [Completion,History]
    LinearWchar             EventChar;      // Character input                      // Read-write   // [CharFilter] Replace character with another one, or set to zero to drop. return 1 is equivalent to setting EventChar=0;
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
    LINEARUI_API LinearUIInputTextCallbackData();
    LINEARUI_API void      DeleteChars(int pos, int bytes_count);
    LINEARUI_API void      InsertChars(int pos, const char* text, const char* text_end = NULL);
    void                SelectAll()                 { SelectionStart = 0; CursorPos = SelectionEnd = BufTextLen; }
    void                SetSelection(int s, int e)  { LINEARUI_ASSERT(s >= 0 && s <= BufTextLen); LINEARUI_ASSERT(e >= 0 && e <= BufTextLen); SelectionStart = s; CursorPos = SelectionEnd = e; }
    void                ClearSelection()            { SelectionStart = SelectionEnd = BufTextLen; }
    bool                HasSelection() const        { return SelectionStart != SelectionEnd; }
};

// Resizing callback data to apply custom constraint. As enabled by SetNextWindowSizeConstraints(). Callback is called during the next Begin().
// NB: For basic min/max size constraint on each axis you don't need to use the callback! The SetNextWindowSizeConstraints() parameters are enough.
struct LinearUISizeCallbackData
{
    void*   UserData;       // Read-only.   What user passed to SetNextWindowSizeConstraints(). Generally store an integer or float in here (need reinterpret_cast<>).
    LinearVec2  Pos;            // Read-only.   Window position, for reference.
    LinearVec2  CurrentSize;    // Read-only.   Current window size.
    LinearVec2  DesiredSize;    // Read-write.  Desired size, based on user's mouse position. Write to this field to restrain resizing.
};

// [ALPHA] Rarely used / very advanced uses only. Use with SetNextWindowClass() and DockSpace() functions.
// Important: the content of this class is still highly WIP and likely to change and be refactored
// before we stabilize Docking features. Please be mindful if using this.
// Provide hints:
// - To the platform backend via altered viewport flags (enable/disable OS decoration, OS task bar icons, etc.)
// - To the platform backend for OS level parent/child relationships of viewport (otherwise: default is configured via io.ConfigViewportsNoDefaultParent)
// - To the docking system for various options and filtering.
struct LinearUIWindowClass
{
    LinearUIID             ClassId;                    // User data. 0 = Default class (unclassed). Windows of different classes cannot be docked with each others.
    LinearUIID             ParentViewportId;           // Hint for the platform backend. -1: use default. 0: request platform backend to not parent the platform. != 0: request platform backend to create a parent<>child relationship between the platform windows. Not conforming backends are free to e.g. parent every viewport to the main viewport or not.
    LinearUIID             FocusRouteParentWindowId;   // ID of parent window for shortcut focus route evaluation, e.g. Shortcut() call from Parent Window will succeed when this window is focused.
    LinearUIViewportFlags  ViewportFlagsOverrideSet;   // Viewport flags to set when a window of this class owns a viewport. This allows you to enforce OS decoration or task bar icon, override the defaults on a per-window basis.
    LinearUIViewportFlags  ViewportFlagsOverrideClear; // Viewport flags to clear when a window of this class owns a viewport. This allows you to enforce OS decoration or task bar icon, override the defaults on a per-window basis.
    LinearUITabItemFlags   TabItemFlagsOverrideSet;    // [EXPERIMENTAL] TabItem flags to set when a window of this class gets submitted into a dock node tab bar. May use with LinearUITabItemFlags_Leading or LinearUITabItemFlags_Trailing.
    LinearUIDockNodeFlags  DockNodeFlagsOverrideSet;   // [EXPERIMENTAL] Dock node flags to set when a window of this class is hosted by a dock node (it doesn't have to be selected!)
    bool                DockingAlwaysTabBar;        // Set to true to enforce single floating windows of this class always having their own docking node (equivalent of setting the global io.ConfigDockingAlwaysTabBar)
    bool                DockingAllowUnclassed;      // Set to true to allow windows of this class to be docked/merged with an unclassed window. // FIXME-DOCK: Move to DockNodeFlags override?
    void*               PlatformIconData;           // [EXPERIMENTAL] Pass opaque data for Platform backend to handle.

    LinearUIWindowClass() { memset((void*)this, 0, sizeof(*this)); ParentViewportId = (LinearUIID)-1; DockingAllowUnclassed = true; }
};

// Data payload for Drag and Drop operations: AcceptDragDropPayload(), GetDragDropPayload()
struct LinearUIPayload
{
    // Members
    void*           Data;               // Data (copied and owned by dear linearui)
    int             DataSize;           // Data size

    // [Internal]
    LinearUIID         SourceId;           // Source item id
    LinearUIID         SourceParentId;     // Source parent id (if available)
    int             DataFrameCount;     // Data timestamp
    char            DataType[32 + 1];   // Data type tag (short user-supplied string, 32 characters max)
    bool            Preview;            // Set when AcceptDragDropPayload() was called and mouse has been hovering the target item (nb: handle overlapping drag targets)
    bool            Delivery;           // Set when AcceptDragDropPayload() was called and mouse button is released over the target item.

    LinearUIPayload()  { Clear(); }
    void Clear()    { SourceId = SourceParentId = 0; Data = NULL; DataSize = 0; memset(DataType, 0, sizeof(DataType)); DataFrameCount = -1; Preview = Delivery = false; }
    bool IsDataType(const char* type) const { return DataFrameCount != -1 && strcmp(type, DataType) == 0; }
    bool IsPreview() const                  { return Preview; }
    bool IsDelivery() const                 { return Delivery; }
};

//-----------------------------------------------------------------------------
// [SECTION] Helpers (LinearUIOnceUponAFrame, LinearUITextFilter, LinearUITextBuffer, LinearUIStorage, LinearUIListClipper, Math Operators, LinearColor)
//-----------------------------------------------------------------------------

// Helper: Unicode defines
#define LINEARUI_UNICODE_CODEPOINT_INVALID 0xFFFD     // Invalid Unicode code point (standard value).
#ifdef LINEARUI_USE_WCHAR32
#define LINEARUI_UNICODE_CODEPOINT_MAX     0x10FFFF   // Maximum Unicode code point supported by this build.
#else
#define LINEARUI_UNICODE_CODEPOINT_MAX     0xFFFF     // Maximum Unicode code point supported by this build.
#endif

// Helper: Execute a block of code at maximum once a frame. Convenient if you want to quickly create a UI within deep-nested code that runs multiple times every frame.
// Usage: static LinearUIOnceUponAFrame oaf; if (oaf) LinearUI::Text("This will be called only once per frame");
struct LinearUIOnceUponAFrame
{
    LinearUIOnceUponAFrame() { RefFrame = -1; }
    mutable int RefFrame;
    operator bool() const { int current_frame = LinearUI::GetFrameCount(); if (RefFrame == current_frame) return false; RefFrame = current_frame; return true; }
};

// Helper: Parse and apply text filters. In format "aaaaa[,bbbb][,ccccc]"
struct LinearUITextFilter
{
    LINEARUI_API           LinearUITextFilter(const char* default_filter = "");
    LINEARUI_API bool      Draw(const char* label = "Filter (inc,-exc)", float width = 0.0f);  // Helper calling InputText+Build
    LINEARUI_API bool      PassFilter(const char* text, const char* text_end = NULL) const;
    LINEARUI_API void      Build();
    void                Clear()          { InputBuf[0] = 0; Build(); }
    bool                IsActive() const { return !Filters.empty(); }

    // [Internal]
    struct LinearUITextRange
    {
        const char*     b;
        const char*     e;

        LinearUITextRange()                                { b = e = NULL; }
        LinearUITextRange(const char* _b, const char* _e)  { b = _b; e = _e; }
        bool            empty() const                   { return b == e; }
        LINEARUI_API void  split(char separator, LinearVector<LinearUITextRange>* out) const;
    };
    char                    InputBuf[256];
    LinearVector<LinearUITextRange>Filters;
    int                     CountGrep;
};

// Helper: Growable text buffer for logging/accumulating text
// (this could be called 'LinearUITextBuilder' / 'LinearUIStringBuilder')
struct LinearUITextBuffer
{
    LinearVector<char>      Buf;
    LINEARUI_API static char EmptyString[1];

    LinearUITextBuffer()   { }
    inline char         operator[](int i) const { LINEARUI_ASSERT(Buf.Data != NULL); return Buf.Data[i]; }
    const char*         begin() const           { return Buf.Data ? &Buf.front() : EmptyString; }
    const char*         end() const             { return Buf.Data ? &Buf.back() : EmptyString; } // Buf is zero-terminated, so end() will point on the zero-terminator
    int                 size() const            { return Buf.Size ? Buf.Size - 1 : 0; }
    bool                empty() const           { return Buf.Size <= 1; }
    void                clear()                 { Buf.clear(); }
    void                resize(int size)        { if (Buf.Size > size) Buf.Data[size] = 0; Buf.resize(size ? size + 1 : 0, 0); } // Similar to resize(0) on LinearVector: empty string but don't free buffer.
    void                reserve(int capacity)   { Buf.reserve(capacity); }
    const char*         c_str() const           { return Buf.Data ? Buf.Data : EmptyString; }
    LINEARUI_API void      append(const char* str, const char* str_end = NULL);
    LINEARUI_API void      appendf(const char* fmt, ...) LINEARUI_FMTARGS(2);
    LINEARUI_API void      appendfv(const char* fmt, va_list args) LINEARUI_FMTLIST(2);
};

// [Internal] Key+Value for LinearUIStorage
struct LinearUIStoragePair
{
    LinearUIID     key;
    union       { int val_i; float val_f; void* val_p; };
    LinearUIStoragePair(LinearUIID _key, int _val)    { key = _key; val_i = _val; }
    LinearUIStoragePair(LinearUIID _key, float _val)  { key = _key; val_f = _val; }
    LinearUIStoragePair(LinearUIID _key, void* _val)  { key = _key; val_p = _val; }
};

// Helper: Key->Value storage
// Typically you don't have to worry about this since a storage is held within each Window.
// We use it to e.g. store collapse state for a tree (Int 0/1)
// This is optimized for efficient lookup (dichotomy into a contiguous buffer) and rare insertion (typically tied to user interactions aka max once a frame)
// You can use it as custom user storage for temporary values. Declare your own storage if, for example:
// - You want to manipulate the open/close state of a particular sub-tree in your interface (tree node uses Int 0/1 to store their state).
// - You want to store custom debug data easily without adding or editing structures in your code (probably not efficient, but convenient)
// Types are NOT stored, so it is up to you to make sure your Key don't collide with different types.
struct LinearUIStorage
{
    // [Internal]
    LinearVector<LinearUIStoragePair>      Data;

    // - Get***() functions find pair, never add/allocate. Pairs are sorted so a query is O(log N)
    // - Set***() functions find pair, insertion on demand if missing.
    // - Sorted insertion is costly, paid once. A typical frame shouldn't need to insert any new pair.
    void                Clear() { Data.clear(); }
    LINEARUI_API int       GetInt(LinearUIID key, int default_val = 0) const;
    LINEARUI_API void      SetInt(LinearUIID key, int val);
    LINEARUI_API bool      GetBool(LinearUIID key, bool default_val = false) const;
    LINEARUI_API void      SetBool(LinearUIID key, bool val);
    LINEARUI_API float     GetFloat(LinearUIID key, float default_val = 0.0f) const;
    LINEARUI_API void      SetFloat(LinearUIID key, float val);
    LINEARUI_API void*     GetVoidPtr(LinearUIID key) const; // default_val is NULL
    LINEARUI_API void      SetVoidPtr(LinearUIID key, void* val);

    // - Get***Ref() functions finds pair, insert on demand if missing, return pointer. Useful if you intend to do Get+Set.
    // - References are only valid until a new value is added to the storage. Calling a Set***() function or a Get***Ref() function invalidates the pointer.
    // - A typical use case where this is convenient for quick hacking (e.g. add storage during a live Edit&Continue session if you can't modify existing struct)
    //      float* pvar = LinearUI::GetFloatRef(key); LinearUI::SliderFloat("var", pvar, 0, 100.0f); some_var += *pvar;
    LINEARUI_API int*      GetIntRef(LinearUIID key, int default_val = 0);
    LINEARUI_API bool*     GetBoolRef(LinearUIID key, bool default_val = false);
    LINEARUI_API float*    GetFloatRef(LinearUIID key, float default_val = 0.0f);
    LINEARUI_API void**    GetVoidPtrRef(LinearUIID key, void* default_val = NULL);

    // Advanced: for quicker full rebuild of a storage (instead of an incremental one), you may add all your contents and then sort once.
    LINEARUI_API void      BuildSortByKey();
    // Obsolete: use on your own storage if you know only integer are being stored (open/close all tree nodes)
    LINEARUI_API void      SetAllInt(int val);

#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    //typedef ::LinearUIStoragePair LinearUIStoragePair;  // 1.90.8: moved type outside struct
#endif
};

// Flags for LinearUIListClipper (currently not fully exposed in function calls: a future refactor will likely add this to LinearUIListClipper::Begin function equivalent)
enum LinearUIListClipperFlags_
{
    LinearUIListClipperFlags_None                  = 0,
    LinearUIListClipperFlags_NoSetTableRowCounters = 1 << 0,   // [Internal] Disabled modifying table row counters. Avoid assumption that 1 clipper item == 1 table row.
};

// Helper: Manually clip large list of items.
// If you have lots evenly spaced items and you have random access to the list, you can perform coarse
// clipping based on visibility to only submit items that are in view.
// The clipper calculates the range of visible items and advance the cursor to compensate for the non-visible items we have skipped.
// (Dear LinearUI already clip items based on their bounds but: it needs to first layout the item to do so, and generally
//  fetching/submitting your own data incurs additional cost. Coarse clipping using LinearUIListClipper allows you to easily
//  scale using lists with tens of thousands of items without a problem)
// Usage:
//   LinearUIListClipper clipper;
//   clipper.Begin(1000);         // We have 1000 elements, evenly spaced.
//   while (clipper.Step())
//       for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
//           LinearUI::Text("line number %d", i);
// Generally what happens is:
// - Clipper lets you process the first element (DisplayStart = 0, DisplayEnd = 1) regardless of it being visible or not.
// - User code submit that one element.
// - Clipper can measure the height of the first element
// - Clipper calculate the actual range of elements to display based on the current clipping rectangle, position the cursor before the first visible element.
// - User code submit visible elements.
// - The clipper also handles various subtleties related to keyboard/gamepad navigation, wrapping etc.
struct LinearUIListClipper
{
    int             DisplayStart;       // First item to display, updated by each call to Step()
    int             DisplayEnd;         // End of items to display (exclusive)
    int             UserIndex;          // Helper storage for user convenience/code. Optional, and otherwise unused if you don't use it.
    int             ItemsCount;         // [Internal] Number of items
    float           ItemsHeight;        // [Internal] Height of item after a first step and item submission can calculate it
    LinearUIListClipperFlags Flags;        // [Internal] Flags, currently not yet well exposed.
    double          StartPosY;          // [Internal] Cursor position at the time of Begin() or after table frozen rows are all processed
    double          StartSeekOffsetY;   // [Internal] Account for frozen rows in a table and initial loss of precision in very large windows.
    LinearUIContext*   Ctx;                // [Internal] Parent UI context
    void*           TempData;           // [Internal] Internal data

    // items_count: Use INT_MAX if you don't know how many items you have (in which case the cursor won't be advanced in the final step, and you can call SeekCursorForItem() manually if you need)
    // items_height: Use -1.0f to be calculated automatically on first step. Otherwise pass in the distance between your items, typically GetTextLineHeightWithSpacing() or GetFrameHeightWithSpacing().
    LINEARUI_API LinearUIListClipper();
    LINEARUI_API ~LinearUIListClipper();
    LINEARUI_API void  Begin(int items_count, float items_height = -1.0f);
    LINEARUI_API void  End();             // Automatically called on the last call of Step() that returns false.
    LINEARUI_API bool  Step();            // Call until it returns false. The DisplayStart/DisplayEnd fields will be set and you can process/draw those items.

    // Call IncludeItemByIndex() or IncludeItemsByIndex() *BEFORE* first call to Step() if you need a range of items to not be clipped, regardless of their visibility.
    // (Due to alignment / padding of certain items it is possible that an extra item may be included on either end of the display range).
    inline void     IncludeItemByIndex(int item_index)                  { IncludeItemsByIndex(item_index, item_index + 1); }
    LINEARUI_API void  IncludeItemsByIndex(int item_begin, int item_end);  // item_end is exclusive e.g. use (42, 42+1) to make item 42 never clipped.

    // Seek cursor toward given item. This is automatically called while stepping.
    // - The only reason to call this is: you can use LinearUIListClipper::Begin(INT_MAX) if you don't know item count ahead of time.
    // - In this case, after all steps are done, you'll want to call SeekCursorForItem(item_count).
    LINEARUI_API void  SeekCursorForItem(int item_index);

#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    //inline void IncludeRangeByIndices(int item_begin, int item_end)      { IncludeItemsByIndex(item_begin, item_end); } // [renamed in 1.89.9]
    //inline void ForceDisplayRangeByIndices(int item_begin, int item_end) { IncludeItemsByIndex(item_begin, item_end); } // [renamed in 1.89.6]
    //inline LinearUIListClipper(int items_count, float items_height = -1.0f) { memset((void*)this, 0, sizeof(*this)); ItemsCount = -1; Begin(items_count, items_height); } // [removed in 1.79]
#endif
};

// Helpers: LinearVec2/LinearVec4 operators
// - It is important that we are keeping those disabled by default so they don't leak in user space.
// - This is in order to allow user enabling implicit cast operators between LinearVec2/LinearVec4 and their own types (using LINEARUI_VEC2_CLASS_EXTRA in linearuiconfig.h)
// - Add '#define LINEARUI_DEFINE_MATH_OPERATORS' before including this file (or in linearuiconfig.h) to access courtesy maths operators for LinearVec2 and LinearVec4.
#ifdef LINEARUI_DEFINE_MATH_OPERATORS
#define LINEARUI_DEFINE_MATH_OPERATORS_IMPLEMENTED
LINEARUI_MSVC_RUNTIME_CHECKS_OFF
// LinearVec2 operators
inline LinearVec2  operator*(const LinearVec2& lhs, const float rhs)    { return LinearVec2(lhs.x * rhs, lhs.y * rhs); }
inline LinearVec2  operator*(const float lhs, const LinearVec2& rhs)    { return LinearVec2(lhs * rhs.x, lhs * rhs.y); }
inline LinearVec2  operator/(const LinearVec2& lhs, const float rhs)    { return LinearVec2(lhs.x / rhs, lhs.y / rhs); }
inline LinearVec2  operator+(const LinearVec2& lhs, const LinearVec2& rhs)  { return LinearVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline LinearVec2  operator-(const LinearVec2& lhs, const LinearVec2& rhs)  { return LinearVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
inline LinearVec2  operator*(const LinearVec2& lhs, const LinearVec2& rhs)  { return LinearVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
inline LinearVec2  operator/(const LinearVec2& lhs, const LinearVec2& rhs)  { return LinearVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
inline LinearVec2  operator+(const LinearVec2& lhs)                     { return lhs; }
inline LinearVec2  operator-(const LinearVec2& lhs)                     { return LinearVec2(-lhs.x, -lhs.y); }
inline LinearVec2& operator*=(LinearVec2& lhs, const float rhs)         { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
inline LinearVec2& operator/=(LinearVec2& lhs, const float rhs)         { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
inline LinearVec2& operator+=(LinearVec2& lhs, const LinearVec2& rhs)       { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
inline LinearVec2& operator-=(LinearVec2& lhs, const LinearVec2& rhs)       { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
inline LinearVec2& operator*=(LinearVec2& lhs, const LinearVec2& rhs)       { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
inline LinearVec2& operator/=(LinearVec2& lhs, const LinearVec2& rhs)       { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }
inline bool    operator==(const LinearVec2& lhs, const LinearVec2& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline bool    operator!=(const LinearVec2& lhs, const LinearVec2& rhs) { return lhs.x != rhs.x || lhs.y != rhs.y; }
// LinearVec4 operators
inline LinearVec4  operator*(const LinearVec4& lhs, const float rhs)    { return LinearVec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); }
inline LinearVec4  operator*(const float lhs, const LinearVec4& rhs)    { return LinearVec4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w); }
inline LinearVec4  operator/(const LinearVec4& lhs, const float rhs)    { return LinearVec4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs); }
inline LinearVec4  operator+(const LinearVec4& lhs, const LinearVec4& rhs)  { return LinearVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
inline LinearVec4  operator-(const LinearVec4& lhs, const LinearVec4& rhs)  { return LinearVec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
inline LinearVec4  operator*(const LinearVec4& lhs, const LinearVec4& rhs)  { return LinearVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }
inline LinearVec4  operator/(const LinearVec4& lhs, const LinearVec4& rhs)  { return LinearVec4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w); }
inline LinearVec4  operator+(const LinearVec4& lhs)                     { return lhs; }
inline LinearVec4  operator-(const LinearVec4& lhs)                     { return LinearVec4(-lhs.x, -lhs.y, -lhs.z, -lhs.w); }
inline LinearVec4& operator*=(LinearVec4& lhs, const float rhs)         { lhs.x *= rhs; lhs.y *= rhs; lhs.z *= rhs; lhs.w *= rhs; return lhs; }
inline LinearVec4& operator/=(LinearVec4& lhs, const float rhs)         { lhs.x /= rhs; lhs.y /= rhs; lhs.z /= rhs; lhs.w /= rhs; return lhs; }
inline LinearVec4& operator+=(LinearVec4& lhs, const LinearVec4& rhs)       { lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z; lhs.w += rhs.w; return lhs; }
inline LinearVec4& operator-=(LinearVec4& lhs, const LinearVec4& rhs)       { lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z; lhs.w -= rhs.w; return lhs; }
inline LinearVec4& operator*=(LinearVec4& lhs, const LinearVec4& rhs)       { lhs.x *= rhs.x; lhs.y *= rhs.y; lhs.z *= rhs.z; lhs.w *= rhs.w; return lhs; }
inline LinearVec4& operator/=(LinearVec4& lhs, const LinearVec4& rhs)       { lhs.x /= rhs.x; lhs.y /= rhs.y; lhs.z /= rhs.z; lhs.w /= rhs.w; return lhs; }
inline bool    operator==(const LinearVec4& lhs, const LinearVec4& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w; }
inline bool    operator!=(const LinearVec4& lhs, const LinearVec4& rhs) { return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z || lhs.w != rhs.w; }
LINEARUI_MSVC_RUNTIME_CHECKS_RESTORE
#endif

// Helpers macros to generate 32-bit encoded colors
// - User can declare their own format by #defining the 5 _SHIFT/_MASK macros in their linearuiconfig file.
// - Any setting other than the default will need custom backend support. The only standard backend that supports anything else than the default is DirectX9.
#ifndef LINEARUI_COL32_R_SHIFT
#ifdef LINEARUI_USE_BGRA_PACKED_COLOR
#define LINEARUI_COL32_R_SHIFT    16
#define LINEARUI_COL32_G_SHIFT    8
#define LINEARUI_COL32_B_SHIFT    0
#define LINEARUI_COL32_A_SHIFT    24
#define LINEARUI_COL32_A_MASK     0xFF000000
#else
#define LINEARUI_COL32_R_SHIFT    0
#define LINEARUI_COL32_G_SHIFT    8
#define LINEARUI_COL32_B_SHIFT    16
#define LINEARUI_COL32_A_SHIFT    24
#define LINEARUI_COL32_A_MASK     0xFF000000
#endif
#endif
#define LINEARUI_COL32(R,G,B,A)    (((LinearU32)(A)<<LINEARUI_COL32_A_SHIFT) | ((LinearU32)(B)<<LINEARUI_COL32_B_SHIFT) | ((LinearU32)(G)<<LINEARUI_COL32_G_SHIFT) | ((LinearU32)(R)<<LINEARUI_COL32_R_SHIFT))
#define LINEARUI_COL32_WHITE       LINEARUI_COL32(255,255,255,255)  // Opaque white = 0xFFFFFFFF
#define LINEARUI_COL32_BLACK       LINEARUI_COL32(0,0,0,255)        // Opaque black
#define LINEARUI_COL32_BLACK_TRANS LINEARUI_COL32(0,0,0,0)          // Transparent black = 0x00000000

// Helper: LinearColor() implicitly converts colors to either LinearU32 (packed 4x1 byte) or LinearVec4 (4x1 float)
// Prefer using LINEARUI_COL32() macros if you want a guaranteed compile-time LinearU32 for usage with LinearDrawList API.
// **Avoid storing LinearColor! Store either u32 of LinearVec4. This is not a full-featured color class. MAY OBSOLETE.
// **None of the LinearUI API are using LinearColor directly but you can use it as a convenience to pass colors in either LinearU32 or LinearVec4 formats. Explicitly cast to LinearU32 or LinearVec4 if needed.
struct LinearColor
{
    LinearVec4          Value;

    constexpr LinearColor()                                             { }
    constexpr LinearColor(float r, float g, float b, float a = 1.0f)    : Value(r, g, b, a) { }
    constexpr LinearColor(const LinearVec4& col)                            : Value(col) {}
    constexpr LinearColor(int r, int g, int b, int a = 255)             : Value((float)r * (1.0f / 255.0f), (float)g * (1.0f / 255.0f), (float)b * (1.0f / 255.0f), (float)a* (1.0f / 255.0f)) {}
    constexpr LinearColor(LinearU32 rgba)                                   : Value((float)((rgba >> LINEARUI_COL32_R_SHIFT) & 0xFF) * (1.0f / 255.0f), (float)((rgba >> LINEARUI_COL32_G_SHIFT) & 0xFF) * (1.0f / 255.0f), (float)((rgba >> LINEARUI_COL32_B_SHIFT) & 0xFF) * (1.0f / 255.0f), (float)((rgba >> LINEARUI_COL32_A_SHIFT) & 0xFF) * (1.0f / 255.0f)) {}
    inline operator LinearU32() const                                   { return LinearUI::ColorConvertFloat4ToU32(Value); }
    inline operator LinearVec4() const                                  { return Value; }

    // FIXME-OBSOLETE: May need to obsolete/cleanup those helpers.
    inline void    SetHSV(float h, float s, float v, float a = 1.0f){ LinearUI::ColorConvertHSVtoRGB(h, s, v, Value.x, Value.y, Value.z); Value.w = a; }
    static LinearColor HSV(float h, float s, float v, float a = 1.0f)   { float r, g, b; LinearUI::ColorConvertHSVtoRGB(h, s, v, r, g, b); return LinearColor(r, g, b, a); }
};

//-----------------------------------------------------------------------------
// [SECTION] Multi-Select API flags and structures (LinearUIMultiSelectFlags, LinearUISelectionRequestType, LinearUISelectionRequest, LinearUIMultiSelectIO, LinearUISelectionBasicStorage)
//-----------------------------------------------------------------------------

// Multi-selection system
// Documentation at: https://github.com/ocornut/linearui/wiki/Multi-Select
// - Refer to 'Demo->Widgets->Selection State & Multi-Select' for demos using this.
// - This system implements standard multi-selection idioms (Ctrl+Mouse/Keyboard, Shift+Mouse/Keyboard, etc)
//   with support for clipper (skipping non-visible items), box-select and many other details.
// - Selectable(), Checkbox() are supported but custom widgets may use it as well.
// - TreeNode() is technically supported but... using this correctly is more complicated: you need some sort of linear/random access to your tree,
//   which is suited to advanced trees setups also implementing filters and clipper. We will work toward simplifying and demoing it.
// - In the spirit of Dear LinearUI design, your code owns actual selection data.
//   This is designed to allow all kinds of selection storage you may use in your application e.g. set/map/hash.
// About LinearUISelectionBasicStorage:
// - This is an optional helper to store a selection state and apply selection requests.
// - It is used by our demos and provided as a convenience to quickly implement multi-selection.
// Usage:
// - Identify submitted items with SetNextItemSelectionUserData(), most likely using an index into your current data-set.
// - Store and maintain actual selection data using persistent object identifiers.
// - Usage flow:
//     BEGIN - (1) Call BeginMultiSelect() and retrieve the LinearUIMultiSelectIO* result.
//           - (2) Honor request list (SetAll/SetRange requests) by updating your selection data. Same code as Step 6.
//           - (3) [If using clipper] You need to make sure RangeSrcItem is always submitted. Calculate its index and pass to clipper.IncludeItemByIndex(). If storing indices in LinearUISelectionUserData, a simple clipper.IncludeItemByIndex(ms_io->RangeSrcItem) call will work.
//     LOOP  - (4) Submit your items with SetNextItemSelectionUserData() + Selectable()/TreeNode() calls.
//     END   - (5) Call EndMultiSelect() and retrieve the LinearUIMultiSelectIO* result.
//           - (6) Honor request list (SetAll/SetRange requests) by updating your selection data. Same code as Step 2.
//     If you submit all items (no clipper), Step 2 and 3 are optional and will be handled by each item themselves. It is fine to always honor those steps.
// About LinearUISelectionUserData:
// - This can store an application-defined identifier (e.g. index or pointer) submitted via SetNextItemSelectionUserData().
// - In return we store them into RangeSrcItem/RangeFirstItem/RangeLastItem and other fields in LinearUIMultiSelectIO.
// - Most applications will store an object INDEX, hence the chosen name and type. Storing an index is natural, because
//   SetRange requests will give you two end-points and you will need to iterate/interpolate between them to update your selection.
// - However it is perfectly possible to store a POINTER or another IDENTIFIER inside LinearUISelectionUserData.
//   Our system never assume that you identify items by indices, it never attempts to interpolate between two values.
// - If you enable LinearUIMultiSelectFlags_NoRangeSelect then it is guaranteed that you will never have to interpolate
//   between two LinearUISelectionUserData, which may be a convenient way to use part of the feature with less code work.
// - As most users will want to store an index, for convenience and to reduce confusion we use LinearS64 instead of void*,
//   being syntactically easier to downcast. Feel free to reinterpret_cast and store a pointer inside.

// Flags for BeginMultiSelect()
enum LinearUIMultiSelectFlags_
{
    LinearUIMultiSelectFlags_None                  = 0,
    LinearUIMultiSelectFlags_SingleSelect          = 1 << 0,   // Disable selecting more than one item. This is available to allow single-selection code to share same code/logic if desired. It essentially disables the main purpose of BeginMultiSelect() tho!
    LinearUIMultiSelectFlags_NoSelectAll           = 1 << 1,   // Disable Ctrl+A shortcut to select all.
    LinearUIMultiSelectFlags_NoRangeSelect         = 1 << 2,   // Disable Shift+selection mouse/keyboard support (useful for unordered 2D selection). With BoxSelect is also ensure contiguous SetRange requests are not combined into one. This allows not handling interpolation in SetRange requests.
    LinearUIMultiSelectFlags_NoAutoSelect          = 1 << 3,   // Disable selecting items when navigating (useful for e.g. supporting range-select in a list of checkboxes).
    LinearUIMultiSelectFlags_NoAutoClear           = 1 << 4,   // Disable clearing selection when navigating or selecting another one (generally used with LinearUIMultiSelectFlags_NoAutoSelect. useful for e.g. supporting range-select in a list of checkboxes).
    LinearUIMultiSelectFlags_NoAutoClearOnReselect = 1 << 5,   // Disable clearing selection when clicking/selecting an already selected item.
    LinearUIMultiSelectFlags_BoxSelect1d           = 1 << 6,   // Enable box-selection with same width and same x pos items (e.g. full row Selectable()). Box-selection works better with little bit of spacing between items hit-box in order to be able to aim at empty space.
    LinearUIMultiSelectFlags_BoxSelect2d           = 1 << 7,   // Enable box-selection with varying width or varying x pos items support (e.g. different width labels, or 2D layout/grid). This is slower: alters clipping logic so that e.g. horizontal movements will update selection of normally clipped items.
    LinearUIMultiSelectFlags_BoxSelectNoScroll     = 1 << 8,   // Disable scrolling when box-selecting and moving mouse near edges of scope.
    LinearUIMultiSelectFlags_ClearOnEscape         = 1 << 9,   // Clear selection when pressing Escape while scope is focused.
    LinearUIMultiSelectFlags_ClearOnClickVoid      = 1 << 10,  // Clear selection when clicking on empty location within scope.
    LinearUIMultiSelectFlags_ScopeWindow           = 1 << 11,  // Scope for _BoxSelect and _ClearOnClickVoid is whole window (Default). Use if BeginMultiSelect() covers a whole window or used a single time in same window.
    LinearUIMultiSelectFlags_ScopeRect             = 1 << 12,  // Scope for _BoxSelect and _ClearOnClickVoid is rectangle encompassing BeginMultiSelect()/EndMultiSelect(). Use if BeginMultiSelect() is called multiple times in same window.
    LinearUIMultiSelectFlags_SelectOnAuto          = 1 << 13,  // Apply selection on mouse down when clicking on unselected item, on mouse up when clicking on selected item. (Default)
    LinearUIMultiSelectFlags_SelectOnClickAlways   = 1 << 14,  // Apply selection on mouse down when clicking on any items. Prevents Drag and Drop from being used on multiple-selection, but allows e.g. BoxSelect to always reselect even when clicking inside an existing selection. (Excel style behavior)
    LinearUIMultiSelectFlags_SelectOnClickRelease  = 1 << 15,  // Apply selection on mouse release when clicking an unselected item. Allow dragging an unselected item without altering selection.
    //LinearUIMultiSelectFlags_RangeSelect2d       = 1 << 15,  // Shift+Selection uses 2d geometry instead of linear sequence, so possible to use Shift+up/down to select vertically in grid. Analogous to what BoxSelect does.
    LinearUIMultiSelectFlags_NavWrapX              = 1 << 16,  // [Temporary] Enable navigation wrapping on X axis. Provided as a convenience because we don't have a design for the general Nav API for this yet. When the more general feature be public we may obsolete this flag in favor of new one.
    LinearUIMultiSelectFlags_NoSelectOnRightClick  = 1 << 17,  // Disable default right-click processing, which selects item on mouse down, and is designed for context-menus.
    LinearUIMultiSelectFlags_SelectOnMask_         = LinearUIMultiSelectFlags_SelectOnAuto | LinearUIMultiSelectFlags_SelectOnClickAlways | LinearUIMultiSelectFlags_SelectOnClickRelease,

    // Obsolete names
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    LinearUIMultiSelectFlags_SelectOnClick         = LinearUIMultiSelectFlags_SelectOnAuto,         // RENAMED in 1.92.6
#endif
};

// Main IO structure returned by BeginMultiSelect()/EndMultiSelect().
// This mainly contains a list of selection requests.
// - Use 'Demo->Tools->Debug Log->Selection' to see requests as they happen.
// - Some fields are only useful if your list is dynamic and allows deletion (getting post-deletion focus/state right is shown in the demo)
// - Below: who reads/writes each fields? 'r'=read, 'w'=write, 'ms'=multi-select code, 'app'=application/user code.
struct LinearUIMultiSelectIO
{
    //------------------------------------------// BeginMultiSelect / EndMultiSelect
    LinearVector<LinearUISelectionRequest> Requests;   //  ms:w, app:r     /  ms:w  app:r   // Requests to apply to your selection data.
    LinearUISelectionUserData      RangeSrcItem;   //  ms:w  app:r     /                // (If using clipper) Begin: Source item (often the first selected item) must never be clipped: use clipper.IncludeItemByIndex() to ensure it is submitted.
    LinearUISelectionUserData      NavIdItem;      //  ms:w, app:r     /                // (If using deletion) Last known SetNextItemSelectionUserData() value for NavId (if part of submitted items).
    bool                        NavIdSelected;  //  ms:w, app:r     /        app:r   // (If using deletion) Last known selection state for NavId (if part of submitted items).
    bool                        RangeSrcReset;  //        app:w     /  ms:r          // (If using deletion) Set before EndMultiSelect() to reset ResetSrcItem (e.g. if deleted selection).
    int                         ItemsCount;     //  ms:w, app:r     /        app:r   // 'int items_count' parameter to BeginMultiSelect() is copied here for convenience, allowing simpler calls to your ApplyRequests handler. Not used internally.
};

// Selection request type
enum LinearUISelectionRequestType
{
    LinearUISelectionRequestType_None = 0,
    LinearUISelectionRequestType_SetAll,           // Request app to clear selection (if Selected==false) or select all items (if Selected==true). We cannot set RangeFirstItem/RangeLastItem as its contents is entirely up to user (not necessarily an index)
    LinearUISelectionRequestType_SetRange,         // Request app to select/unselect [RangeFirstItem..RangeLastItem] items (inclusive) based on value of Selected. Only EndMultiSelect() request this, app code can read after BeginMultiSelect() and it will always be false.
};

// Selection request item
struct LinearUISelectionRequest
{
    //------------------------------------------// BeginMultiSelect / EndMultiSelect
    LinearUISelectionRequestType   Type;           //  ms:w, app:r     /  ms:w, app:r   // Request type. You'll most often receive 1 Clear + 1 SetRange with a single-item range.
    bool                        Selected;       //  ms:w, app:r     /  ms:w, app:r   // Parameter for SetAll/SetRange requests (true = select, false = unselect)
    LinearS8                        RangeDirection; //                  /  ms:w  app:r   // Parameter for SetRange request: +1 when RangeFirstItem comes before RangeLastItem, -1 otherwise. Useful if you want to preserve selection order on a backward Shift+Click.
    LinearUISelectionUserData      RangeFirstItem; //                  /  ms:w, app:r   // Parameter for SetRange request (this is generally == RangeSrcItem when shift selecting from top to bottom).
    LinearUISelectionUserData      RangeLastItem;  //                  /  ms:w, app:r   // Parameter for SetRange request (this is generally == RangeSrcItem when shift selecting from bottom to top). Inclusive!
};

// Optional helper to store multi-selection state + apply multi-selection requests.
// - Used by our demos and provided as a convenience to easily implement basic multi-selection.
// - Iterate selection with 'void* it = NULL; LinearUIID id; while (selection.GetNextSelectedItem(&it, &id)) { ... }'
//   Or you can check 'if (Contains(id)) { ... }' for each possible object if their number is not too high to iterate.
// - USING THIS IS NOT MANDATORY. This is only a helper and not a required API.
// To store a multi-selection, in your application you could:
// - Use this helper as a convenience. We use our simple key->value LinearUIStorage as a std::set<LinearUIID> replacement.
// - Use your own external storage: e.g. std::set<MyObjectId>, std::vector<MyObjectId>, interval trees, intrusively stored selection etc.
// In LinearUISelectionBasicStorage we:
// - always use indices in the multi-selection API (passed to SetNextItemSelectionUserData(), retrieved in LinearUIMultiSelectIO)
// - use the AdapterIndexToStorageId() indirection layer to abstract how persistent selection data is derived from an index.
// - use decently optimized logic to allow queries and insertion of very large selection sets.
// - do not preserve selection order.
// Many combinations are possible depending on how you prefer to store your items and how you prefer to store your selection.
// Large applications are likely to eventually want to get rid of this indirection layer and do their own thing.
// See https://github.com/ocornut/linearui/wiki/Multi-Select for details and pseudo-code using this helper.
struct LinearUISelectionBasicStorage
{
    // Members
    int             Size;           //          // Number of selected items, maintained by this helper.
    bool            PreserveOrder;  // = false  // GetNextSelectedItem() will return ordered selection (currently implemented by two additional sorts of selection. Could be improved)
    void*           UserData;       // = NULL   // User data for use by adapter function        // e.g. selection.UserData = (void*)my_items;
    LinearUIID         (*AdapterIndexToStorageId)(LinearUISelectionBasicStorage* self, int idx);      // e.g. selection.AdapterIndexToStorageId = [](LinearUISelectionBasicStorage* self, int idx) { return ((MyItems**)self->UserData)[idx]->ID; };
    int             _SelectionOrder;// [Internal] Increasing counter to store selection order
    LinearUIStorage    _Storage;       // [Internal] Selection set. Think of this as similar to e.g. std::set<LinearUIID>. Prefer not accessing directly: iterate with GetNextSelectedItem().

    // Methods
    LINEARUI_API LinearUISelectionBasicStorage();
    LINEARUI_API void  ApplyRequests(LinearUIMultiSelectIO* ms_io);   // Apply selection requests coming from BeginMultiSelect() and EndMultiSelect() functions. It uses 'items_count' passed to BeginMultiSelect()
    LINEARUI_API bool  Contains(LinearUIID id) const;                 // Query if an item id is in selection.
    LINEARUI_API void  Clear();                                    // Clear selection
    LINEARUI_API void  Swap(LinearUISelectionBasicStorage& r);        // Swap two selections
    LINEARUI_API void  SetItemSelected(LinearUIID id, bool selected); // Add/remove an item from selection (generally done by ApplyRequests() function)
    LINEARUI_API bool  GetNextSelectedItem(void** opaque_it, LinearUIID* out_id); // Iterate selection with 'void* it = NULL; LinearUIID id; while (selection.GetNextSelectedItem(&it, &id)) { ... }'
    inline LinearUIID  GetStorageIdFromIndex(int idx)              { return AdapterIndexToStorageId(this, idx); }  // Convert index to item id based on provided adapter.
};

// Optional helper to apply multi-selection requests to existing randomly accessible storage.
// Convenient if you want to quickly wire multi-select API on e.g. an array of bool or items storing their own selection state.
struct LinearUISelectionExternalStorage
{
    // Members
    void*           UserData;       // User data for use by adapter function                                // e.g. selection.UserData = (void*)my_items;
    void            (*AdapterSetItemSelected)(LinearUISelectionExternalStorage* self, int idx, bool selected); // e.g. AdapterSetItemSelected = [](LinearUISelectionExternalStorage* self, int idx, bool selected) { ((MyItems**)self->UserData)[idx]->Selected = selected; }

    // Methods
    LINEARUI_API LinearUISelectionExternalStorage();
    LINEARUI_API void  ApplyRequests(LinearUIMultiSelectIO* ms_io);   // Apply selection requests by using AdapterSetItemSelected() calls
};

//-----------------------------------------------------------------------------
// [SECTION] Drawing API (LinearDrawCmd, LinearDrawIdx, LinearDrawVert, LinearDrawChannel, LinearDrawListSplitter, LinearDrawListFlags, LinearDrawList, LinearDrawData)
// Hold a series of drawing commands. The user provides a renderer for LinearDrawData which essentially contains an array of LinearDrawList.
//-----------------------------------------------------------------------------

// The maximum line width to bake anti-aliased textures for. Build atlas with LinearFontAtlasFlags_NoBakedLines to disable baking.
#ifndef LINEARUI_DRAWLIST_TEX_LINES_WIDTH_MAX
#define LINEARUI_DRAWLIST_TEX_LINES_WIDTH_MAX     (32)
#endif

// LinearDrawIdx: vertex index. [Compile-time configurable type]
// - To use 16-bit indices + allow large meshes: backend need to set 'io.BackendFlags |= LinearUIBackendFlags_RendererHasVtxOffset' and handle LinearDrawCmd::VtxOffset (recommended).
// - To use 32-bit indices: override with '#define LinearDrawIdx unsigned int' in your linearuiconfig.h file.
#ifndef LinearDrawIdx
typedef unsigned short LinearDrawIdx;   // Default: 16-bit (for maximum compatibility with renderer backends)
#endif

// LinearDrawCallback: Draw callbacks for advanced uses [configurable type: override in linearuiconfig.h]
// NB: You most likely do NOT need to use draw callbacks just to create your own widget or customized UI rendering,
// you can poke into the draw list for that! Draw callback may be useful for example to:
//  A) Change your GPU render state,
//  B) render a complex 3D scene inside a UI element without an intermediate texture/render target, etc.
// The expected behavior from your rendering function is 'if (cmd.UserCallback != NULL) { cmd.UserCallback(parent_list, cmd); } else { RenderTriangles() }'
// If you want to override the signature of LinearDrawCallback, you can simply use e.g. '#define LinearDrawCallback MyDrawCallback' (in linearuiconfig.h) + update rendering backend accordingly.
#ifndef LinearDrawCallback
typedef void (*LinearDrawCallback)(const LinearDrawList* parent_list, const LinearDrawCmd* cmd);
#endif

// Typically, 1 command = 1 GPU draw call (unless command is a callback)
// - VtxOffset: When 'io.BackendFlags & LinearUIBackendFlags_RendererHasVtxOffset' is enabled,
//   this fields allow us to render meshes larger than 64K vertices while keeping 16-bit indices.
//   Backends made for <1.71. will typically ignore the VtxOffset fields.
// - The ClipRect/TexRef/VtxOffset fields must be contiguous as we memcmp() them together (this is asserted for).
struct LinearDrawCmd
{
    LinearVec4          ClipRect;           // 4*4  // Clipping rectangle (x1, y1, x2, y2). Subtract LinearDrawData->DisplayPos to get clipping rectangle in "viewport" coordinates
    LinearTextureRef    TexRef;             // 16   // Reference to a font/texture atlas (where backend called LinearTextureData::SetTexID()) or to a user-provided texture ID (via e.g. LinearUI::Image() calls). Both will lead to a LinearTextureID value.
    unsigned int    VtxOffset;          // 4    // Start offset in vertex buffer. LinearUIBackendFlags_RendererHasVtxOffset: always 0, otherwise may be >0 to support meshes larger than 64K vertices with 16-bit indices.
    unsigned int    IdxOffset;          // 4    // Start offset in index buffer.
    unsigned int    ElemCount;          // 4    // Number of indices (multiple of 3) to be rendered as triangles. Vertices are stored in the callee LinearDrawList's vtx_buffer[] array, indices in idx_buffer[].
    LinearDrawCallback  UserCallback;       // 4-8  // If != NULL, call the function instead of rendering the vertices. clip_rect and texture_id will be set normally.
    void*           UserCallbackData;   // 4-8  // Callback user data (when UserCallback != NULL). If called AddCallback() with size == 0, this is a copy of the AddCallback() argument. If called AddCallback() with size > 0, this is pointing to a buffer where data is stored.
    int             UserCallbackDataSize;  // 4 // Size of callback user data when using storage, otherwise 0.
    int             UserCallbackDataOffset;// 4 // [Internal] Offset of callback user data when using storage, otherwise -1.

    LinearDrawCmd()     { memset((void*)this, 0, sizeof(*this)); } // Also ensure our padding fields are zeroed

    // Since 1.83: returns LinearTextureID associated with this draw call. Warning: DO NOT assume this is always same as 'TextureId' (we will change this function for an upcoming feature)
    // Since 1.92: removed LinearDrawCmd::TextureId field, the getter function must be used!
    inline LinearTextureID GetTexID() const;    // == (TexRef._TexData ? TexRef._TexData->TexID : TexRef._TexID)
};

// Vertex layout
#ifndef LINEARUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT
struct LinearDrawVert
{
    LinearVec2  pos;
    LinearVec2  uv;
    LinearU32   col;
};
#else
// You can override the vertex format layout by defining LINEARUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT in linearuiconfig.h
// The code expect LinearVec2 pos (8 bytes), LinearVec2 uv (8 bytes), LinearU32 col (4 bytes), but you can re-order them or add other fields as needed to simplify integration in your engine.
// The type has to be described within the macro (you can either declare the struct or use a typedef). This is because LinearVec2/LinearU32 are likely not declared at the time you'd want to set your type up.
// NOTE: LINEARUI DOESN'T CLEAR THE STRUCTURE AND DOESN'T CALL A CONSTRUCTOR SO ANY CUSTOM FIELD WILL BE UNINITIALIZED. IF YOU ADD EXTRA FIELDS (SUCH AS A 'Z' COORDINATES) YOU WILL NEED TO CLEAR THEM DURING RENDER OR TO IGNORE THEM.
LINEARUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT;
#endif

// [Internal] For use by LinearDrawList
struct LinearDrawCmdHeader
{
    LinearVec4          ClipRect;
    LinearTextureRef    TexRef;
    unsigned int    VtxOffset;
};

// [Internal] For use by LinearDrawListSplitter
struct LinearDrawChannel
{
    LinearVector<LinearDrawCmd>         _CmdBuffer;
    LinearVector<LinearDrawIdx>         _IdxBuffer;
};

// Split/Merge functions are used to split the draw list into different layers which can be drawn into out of order.
// This is used by the Columns/Tables API, so items of each column can be batched together in a same draw call.
struct LinearDrawListSplitter
{
    int                         _Current;    // Current channel number (0)
    int                         _Count;      // Number of active channels (1+)
    LinearVector<LinearDrawChannel>     _Channels;   // Draw channels (not resized down so _Count might be < Channels.Size)

    inline LinearDrawListSplitter()  { memset((void*)this, 0, sizeof(*this)); }
    inline ~LinearDrawListSplitter() { ClearFreeMemory(); }
    inline void                 Clear() { _Current = 0; _Count = 1; } // Do not clear Channels[] so our allocations are reused next frame
    LINEARUI_API void              ClearFreeMemory();
    LINEARUI_API void              Split(LinearDrawList* draw_list, int count);
    LINEARUI_API void              Merge(LinearDrawList* draw_list);
    LINEARUI_API void              SetCurrentChannel(LinearDrawList* draw_list, int channel_idx);
};

// Flags for LinearDrawList functions
enum LinearDrawFlags_
{
    LinearDrawFlags_None                        = 0,

    // Rounding for AddRect(), AddRectFilled(), PathRect()
    // - When not specified, we defaults to LinearDrawFlags_RoundCornersAll! So you only need to use those flags if you want another configuration.
    LinearDrawFlags_RoundCornersTopLeft         = 1 << 4, // Round top-left corner only (when rounding > 0.0f, we default to all corners).
    LinearDrawFlags_RoundCornersTopRight        = 1 << 5, // Round top-right corner only (when rounding > 0.0f, we default to all corners).
    LinearDrawFlags_RoundCornersBottomLeft      = 1 << 6, // Round bottom-left corner only (when rounding > 0.0f, we default to all corners).
    LinearDrawFlags_RoundCornersBottomRight     = 1 << 7, // Round bottom-right corner only (when rounding > 0.0f, we default to all corners).
    LinearDrawFlags_RoundCornersNone            = 1 << 8, // Disable rounding even if `float rounding > 0.0f`. This is NOT zero, NOT an implicit flag!
    LinearDrawFlags_RoundCornersAll             = LinearDrawFlags_RoundCornersTopLeft | LinearDrawFlags_RoundCornersTopRight | LinearDrawFlags_RoundCornersBottomLeft | LinearDrawFlags_RoundCornersBottomRight, // (Default!!)
    LinearDrawFlags_RoundCornersDefault_        = LinearDrawFlags_RoundCornersAll, // Default to ALL corners if none of the _RoundCornersXX flags are specified!
    LinearDrawFlags_RoundCornersTop             = LinearDrawFlags_RoundCornersTopLeft | LinearDrawFlags_RoundCornersTopRight,
    LinearDrawFlags_RoundCornersBottom          = LinearDrawFlags_RoundCornersBottomLeft | LinearDrawFlags_RoundCornersBottomRight,
    LinearDrawFlags_RoundCornersLeft            = LinearDrawFlags_RoundCornersBottomLeft | LinearDrawFlags_RoundCornersTopLeft,
    LinearDrawFlags_RoundCornersRight           = LinearDrawFlags_RoundCornersBottomRight | LinearDrawFlags_RoundCornersTopRight,
    LinearDrawFlags_RoundCornersMask_           = LinearDrawFlags_RoundCornersAll | LinearDrawFlags_RoundCornersNone,

    // Stroke options
    LinearDrawFlags_Closed                      = 1 << 9, // PathStroke(), AddPolyline(): specify that shape should be closed.
    //LinearDrawFlags_Closed                    = 1 << 0, // Prior to 1.92.8 (May 2026), LinearDrawFlags_Closed was guaranteed to be == 1<<0 == 1 for legacy compatibility reason. Hardcoded use of 1 or true should be replaced.

    LinearDrawFlags_InvalidMask_                = ~0x7FFFFFF0, // == 0x8000000F,
};

// Flags for LinearDrawList instance. Those are set automatically by LinearUI:: functions from LinearUIIO settings, and generally not manipulated directly.
// It is however possible to temporarily alter flags between calls to LinearDrawList:: functions.
enum LinearDrawListFlags_
{
    LinearDrawListFlags_None                    = 0,
    LinearDrawListFlags_AntiAliasedLines        = 1 << 0,  // Enable anti-aliased lines/borders (*2 the number of triangles for 1.0f wide line or lines thin enough to be drawn using textures, otherwise *3 the number of triangles)
    LinearDrawListFlags_AntiAliasedLinesUseTex  = 1 << 1,  // Enable anti-aliased lines/borders using textures when possible. Require backend to render with bilinear filtering (NOT point/nearest filtering).
    LinearDrawListFlags_AntiAliasedFill         = 1 << 2,  // Enable anti-aliased edge around filled shapes (rounded rectangles, circles).
    LinearDrawListFlags_AllowVtxOffset          = 1 << 3,  // Can emit 'VtxOffset > 0' to allow large meshes. Set when 'LinearUIBackendFlags_RendererHasVtxOffset' is enabled.
    LinearDrawListFlags_TextNoPixelSnap         = 1 << 4,  // Disable automatically snapping AddText() calls to pixel boundaries.
};

// Draw command list
// This is the low-level list of polygons that LinearUI:: functions are filling. At the end of the frame,
// all command lists are passed to your LinearUIIO::RenderDrawListFn function for rendering.
// Each dear linearui window contains its own LinearDrawList. You can use LinearUI::GetWindowDrawList() to
// access the current window draw list and draw custom primitives.
// You can interleave normal LinearUI:: calls and adding primitives to the current draw list.
// In single viewport mode, top-left is == GetMainViewport()->Pos (generally 0,0), bottom-right is == GetMainViewport()->Pos+Size (generally io.DisplaySize).
// You are totally free to apply whatever transformation matrix you want to the data (depending on the use of the transformation you may want to apply it to ClipRect as well!)
// Important: Primitives are always added to the list and not culled (culling is done at higher-level by LinearUI:: functions), if you use this API a lot consider coarse culling your drawn objects.
struct LinearDrawList
{
    // This is what you have to render
    LinearVector<LinearDrawCmd>     CmdBuffer;          // Draw commands. Typically 1 command = 1 GPU draw call, unless the command is a callback.
    LinearVector<LinearDrawIdx>     IdxBuffer;          // Index buffer. Each command consume LinearDrawCmd::ElemCount of those
    LinearVector<LinearDrawVert>    VtxBuffer;          // Vertex buffer.
    LinearDrawListFlags         Flags;              // Flags, you may poke into these to adjust anti-aliasing settings per-primitive.

    // [Internal, used while building lists]
    unsigned int            _VtxCurrentIdx;     // [Internal] generally == VtxBuffer.Size unless we are past 64K vertices, in which case this gets reset to 0.
    LinearDrawListSharedData*   _Data;              // Pointer to shared draw data (you can use LinearUI::GetDrawListSharedData() to get the one from current LinearUI context)
    LinearDrawVert*             _VtxWritePtr;       // [Internal] point within VtxBuffer.Data after each add command (to avoid using the LinearVector<> operators too much)
    LinearDrawIdx*              _IdxWritePtr;       // [Internal] point within IdxBuffer.Data after each add command (to avoid using the LinearVector<> operators too much)
    LinearVector<LinearVec2>        _Path;              // [Internal] current path building
    LinearDrawCmdHeader         _CmdHeader;         // [Internal] template of active commands. Fields should match those of CmdBuffer.back().
    LinearDrawListSplitter      _Splitter;          // [Internal] for channels api (note: prefer using your own persistent instance of LinearDrawListSplitter!)
    LinearVector<LinearVec4>        _ClipRectStack;     // [Internal]
    LinearVector<LinearTextureRef>  _TextureStack;      // [Internal]
    LinearVector<LinearU8>          _CallbacksDataBuf;  // [Internal]
    float                   _FringeScale;       // [Internal] anti-alias fringe is scaled by this value, this helps to keep things sharp while zooming at vertex buffer content
    const char*             _OwnerName;         // Pointer to owner window's name for debugging

    // If you want to create LinearDrawList instances, pass them LinearUI::GetDrawListSharedData().
    // (advanced: you may create and use your own LinearDrawListSharedData so you can use LinearDrawList without LinearUI, but that's more involved)
    LINEARUI_API LinearDrawList(LinearDrawListSharedData* shared_data);
    LINEARUI_API ~LinearDrawList();

    LINEARUI_API void  PushClipRect(const LinearVec2& clip_rect_min, const LinearVec2& clip_rect_max, bool intersect_with_current_clip_rect = false);  // Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level LinearUI::PushClipRect() to affect logic (hit-testing and widget culling)
    LINEARUI_API void  PushClipRectFullScreen();
    LINEARUI_API void  PopClipRect();
    LINEARUI_API void  PushTexture(LinearTextureRef tex_ref);
    LINEARUI_API void  PopTexture();
    inline LinearVec2   GetClipRectMin() const { const LinearVec4& cr = _ClipRectStack.back(); return LinearVec2(cr.x, cr.y); }
    inline LinearVec2   GetClipRectMax() const { const LinearVec4& cr = _ClipRectStack.back(); return LinearVec2(cr.z, cr.w); }

    // Primitives
    // - Filled shapes must always use clockwise winding order. The anti-aliasing fringe depends on it. Counter-clockwise shapes will have "inward" anti-aliasing.
    // - For rectangular primitives, "p_min" and "p_max" represent the upper-left and lower-right corners.
    // - For circle primitives, use "num_segments == 0" to automatically calculate tessellation (preferred).
    //   In older versions (until Dear LinearUI 1.77) the AddCircle functions defaulted to num_segments == 12.
    //   In future versions we will use textures to provide cheaper and higher-quality circles.
    //   Use AddNgon() and AddNgonFilled() functions if you need to guarantee a specific number of sides.
    LINEARUI_API void  AddLine(const LinearVec2& p1, const LinearVec2& p2, LinearU32 col, float thickness = 1.0f);
    LINEARUI_API void  AddLineH(float min_x, float max_x, float y, LinearU32 col, float thickness = 1.0f);
    LINEARUI_API void  AddLineV(float x, float min_y, float max_y, LinearU32 col, float thickness = 1.0f);
    LINEARUI_API void  AddRect(const LinearVec2& p_min, const LinearVec2& p_max, LinearU32 col, float rounding = 0.0f, float thickness = 1.0f, LinearDrawFlags flags = 0);   // a: upper-left, b: lower-right (== upper-left + size)
    LINEARUI_API void  AddRectFilled(const LinearVec2& p_min, const LinearVec2& p_max, LinearU32 col, float rounding = 0.0f, LinearDrawFlags flags = 0);                     // a: upper-left, b: lower-right (== upper-left + size)
    LINEARUI_API void  AddRectFilledMultiColor(const LinearVec2& p_min, const LinearVec2& p_max, LinearU32 col_upr_left, LinearU32 col_upr_right, LinearU32 col_bot_right, LinearU32 col_bot_left);
    LINEARUI_API void  AddQuad(const LinearVec2& p1, const LinearVec2& p2, const LinearVec2& p3, const LinearVec2& p4, LinearU32 col, float thickness = 1.0f);
    LINEARUI_API void  AddQuadFilled(const LinearVec2& p1, const LinearVec2& p2, const LinearVec2& p3, const LinearVec2& p4, LinearU32 col);
    LINEARUI_API void  AddTriangle(const LinearVec2& p1, const LinearVec2& p2, const LinearVec2& p3, LinearU32 col, float thickness = 1.0f);
    LINEARUI_API void  AddTriangleFilled(const LinearVec2& p1, const LinearVec2& p2, const LinearVec2& p3, LinearU32 col);
    LINEARUI_API void  AddCircle(const LinearVec2& center, float radius, LinearU32 col, int num_segments = 0, float thickness = 1.0f);
    LINEARUI_API void  AddCircleFilled(const LinearVec2& center, float radius, LinearU32 col, int num_segments = 0);
    LINEARUI_API void  AddNgon(const LinearVec2& center, float radius, LinearU32 col, int num_segments, float thickness = 1.0f);
    LINEARUI_API void  AddNgonFilled(const LinearVec2& center, float radius, LinearU32 col, int num_segments);
    LINEARUI_API void  AddEllipse(const LinearVec2& center, const LinearVec2& radius, LinearU32 col, float rot = 0.0f, int num_segments = 0, float thickness = 1.0f);
    LINEARUI_API void  AddEllipseFilled(const LinearVec2& center, const LinearVec2& radius, LinearU32 col, float rot = 0.0f, int num_segments = 0);
    LINEARUI_API void  AddText(const LinearVec2& pos, LinearU32 col, const char* text_begin, const char* text_end = NULL);
    LINEARUI_API void  AddText(LinearFont* font, float font_size, const LinearVec2& pos, LinearU32 col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const LinearVec4* cpu_fine_clip_rect = NULL);
    LINEARUI_API void  AddBezierCubic(const LinearVec2& p1, const LinearVec2& p2, const LinearVec2& p3, const LinearVec2& p4, LinearU32 col, float thickness, int num_segments = 0); // Cubic Bezier (4 control points)
    LINEARUI_API void  AddBezierQuadratic(const LinearVec2& p1, const LinearVec2& p2, const LinearVec2& p3, LinearU32 col, float thickness, int num_segments = 0);               // Quadratic Bezier (3 control points)

    // General polygon
    // - Only simple polygons are supported by filling functions (no self-intersections, no holes).
    // - Concave polygon fill is more expensive than convex one: it has O(N^2) complexity. Provided as a convenience for the user but not used by the main library.
    LINEARUI_API void  AddPolyline(const LinearVec2* points, int num_points, LinearU32 col, float thickness, LinearDrawFlags flags = 0);
    LINEARUI_API void  AddConvexPolyFilled(const LinearVec2* points, int num_points, LinearU32 col);
    LINEARUI_API void  AddConcavePolyFilled(const LinearVec2* points, int num_points, LinearU32 col);

    // Image primitives
    // - Read FAQ to understand what LinearTextureID/LinearTextureRef are.
    // - "p_min" and "p_max" represent the upper-left and lower-right corners of the rectangle.
    // - "uv_min" and "uv_max" represent the normalized texture coordinates to use for those corners. Using (0,0)->(1,1) texture coordinates will generally display the entire texture.
    LINEARUI_API void  AddImage(LinearTextureRef tex_ref, const LinearVec2& p_min, const LinearVec2& p_max, const LinearVec2& uv_min = LinearVec2(0, 0), const LinearVec2& uv_max = LinearVec2(1, 1), LinearU32 col = LINEARUI_COL32_WHITE);
    LINEARUI_API void  AddImageQuad(LinearTextureRef tex_ref, const LinearVec2& p1, const LinearVec2& p2, const LinearVec2& p3, const LinearVec2& p4, const LinearVec2& uv1 = LinearVec2(0, 0), const LinearVec2& uv2 = LinearVec2(1, 0), const LinearVec2& uv3 = LinearVec2(1, 1), const LinearVec2& uv4 = LinearVec2(0, 1), LinearU32 col = LINEARUI_COL32_WHITE);
    LINEARUI_API void  AddImageRounded(LinearTextureRef tex_ref, const LinearVec2& p_min, const LinearVec2& p_max, const LinearVec2& uv_min, const LinearVec2& uv_max, LinearU32 col, float rounding, LinearDrawFlags flags = 0);

    // Stateful path API, add points then finish with PathFillConvex() or PathStroke()
    // - Important: filled shapes must always use clockwise winding order! The anti-aliasing fringe depends on it. Counter-clockwise shapes will have "inward" anti-aliasing.
    //   so e.g. 'PathArcTo(center, radius, PI * -0.5f, PI)' is ok, whereas 'PathArcTo(center, radius, PI, PI * -0.5f)' won't have correct anti-aliasing when followed by PathFillConvex().
    inline    void  PathClear()                                                 { _Path.Size = 0; }
    inline    void  PathLineTo(const LinearVec2& pos)                               { _Path.push_back(pos); }
    inline    void  PathLineToMergeDuplicate(const LinearVec2& pos)                 { if (_Path.Size == 0 || memcmp(&_Path.Data[_Path.Size - 1], &pos, 8) != 0) _Path.push_back(pos); }
    inline    void  PathFillConvex(LinearU32 col)                                   { AddConvexPolyFilled(_Path.Data, _Path.Size, col); _Path.Size = 0; }
    inline    void  PathFillConcave(LinearU32 col)                                  { AddConcavePolyFilled(_Path.Data, _Path.Size, col); _Path.Size = 0; }
    inline    void  PathStroke(LinearU32 col, float thickness = 1.0f, LinearDrawFlags flags = 0) { AddPolyline(_Path.Data, _Path.Size, col, thickness, flags); _Path.Size = 0; }
    LINEARUI_API void  PathArcTo(const LinearVec2& center, float radius, float a_min, float a_max, int num_segments = 0);
    LINEARUI_API void  PathArcToFast(const LinearVec2& center, float radius, int a_min_of_12, int a_max_of_12);                // Use precomputed angles for a 12 steps circle
    LINEARUI_API void  PathEllipticalArcTo(const LinearVec2& center, const LinearVec2& radius, float rot, float a_min, float a_max, int num_segments = 0); // Ellipse
    LINEARUI_API void  PathBezierCubicCurveTo(const LinearVec2& p2, const LinearVec2& p3, const LinearVec2& p4, int num_segments = 0); // Cubic Bezier (4 control points)
    LINEARUI_API void  PathBezierQuadraticCurveTo(const LinearVec2& p2, const LinearVec2& p3, int num_segments = 0);               // Quadratic Bezier (3 control points)
    LINEARUI_API void  PathRect(const LinearVec2& rect_min, const LinearVec2& rect_max, float rounding = 0.0f, LinearDrawFlags flags = 0);

    // Advanced: Draw Callbacks
    // - May be used to alter render state (change sampler, blending, current shader). May be used to emit custom rendering commands (difficult to do correctly, but possible).
    // - Use special GetPlatformIO().DrawCallback_ResetRenderState callback to instruct backend to reset its render state to the default.
    // - See other standard callbacks in GetPlatformIO(), which may or not be supported by your backend.
    // - Your rendering loop must check for 'UserCallback' in LinearDrawCmd and call the function instead of rendering triangles. All standard backends are honoring this.
    // - For some backends, the callback may access selected render-states exposed by the backend in a LinearUI_ImplXXXX_RenderState structure pointed to by platform_io.Renderer_RenderState.
    // - IMPORTANT: please be mindful of the different level of indirection between using size==0 (copying argument) and using size>0 (copying pointed data into a buffer).
    //   - If userdata_size == 0: we copy/store the 'userdata' argument as-is. It will be available unmodified in LinearDrawCmd::UserCallbackData during render.
    //   - If userdata_size > 0,  we copy/store 'userdata_size' bytes pointed to by 'userdata'. We store them in a buffer stored inside the drawlist. LinearDrawCmd::UserCallbackData will point inside that buffer so you have to retrieve data from there. Your callback may need to use LinearDrawCmd::UserCallbackDataSize if you expect dynamically-sized data.
    //   - Support for userdata_size > 0 was added in v1.91.4, October 2024. So earlier code always only allowed to copy/store a simple void*.
    LINEARUI_API void  AddCallback(LinearDrawCallback callback, void* userdata = NULL, size_t userdata_size = 0);

    // Advanced: Miscellaneous
    LINEARUI_API void  AddDrawCmd();                                               // This is useful if you need to forcefully create a new draw call (to allow for dependent rendering / blending). Otherwise primitives are merged into the same draw-call as much as possible
    LINEARUI_API LinearDrawList* CloneOutput() const;                                  // Create a clone of the CmdBuffer/IdxBuffer/VtxBuffer. For multi-threaded rendering, consider using `linearui_threaded_rendering` from https://github.com/ocornut/linearui_club instead.

    // Advanced: Channels
    // - Use to split render into layers. By switching channels to can render out-of-order (e.g. submit FG primitives before BG primitives)
    // - Use to minimize draw calls (e.g. if going back-and-forth between multiple clipping rectangles, prefer to append into separate channels then merge at the end)
    // - This API shouldn't have been in LinearDrawList in the first place!
    //   Prefer using your own persistent instance of LinearDrawListSplitter as you can stack them.
    //   Using the LinearDrawList::ChannelsXXXX you cannot stack a split over another.
    inline void     ChannelsSplit(int count)    { _Splitter.Split(this, count); }
    inline void     ChannelsMerge()             { _Splitter.Merge(this); }
    inline void     ChannelsSetCurrent(int n)   { _Splitter.SetCurrentChannel(this, n); }

    // Advanced: Primitives allocations
    // - We render triangles (three vertices)
    // - All primitives needs to be reserved via PrimReserve() beforehand.
    LINEARUI_API void  PrimReserve(int idx_count, int vtx_count);
    LINEARUI_API void  PrimUnreserve(int idx_count, int vtx_count);
    LINEARUI_API void  PrimRect(const LinearVec2& a, const LinearVec2& b, LinearU32 col);      // Axis aligned rectangle (composed of two triangles)
    LINEARUI_API void  PrimRectUV(const LinearVec2& a, const LinearVec2& b, const LinearVec2& uv_a, const LinearVec2& uv_b, LinearU32 col);
    LINEARUI_API void  PrimQuadUV(const LinearVec2& a, const LinearVec2& b, const LinearVec2& c, const LinearVec2& d, const LinearVec2& uv_a, const LinearVec2& uv_b, const LinearVec2& uv_c, const LinearVec2& uv_d, LinearU32 col);
    inline    void  PrimWriteVtx(const LinearVec2& pos, const LinearVec2& uv, LinearU32 col)    { _VtxWritePtr->pos = pos; _VtxWritePtr->uv = uv; _VtxWritePtr->col = col; _VtxWritePtr++; _VtxCurrentIdx++; }
    inline    void  PrimWriteIdx(LinearDrawIdx idx)                                     { *_IdxWritePtr = idx; _IdxWritePtr++; }
    inline    void  PrimVtx(const LinearVec2& pos, const LinearVec2& uv, LinearU32 col)         { PrimWriteIdx((LinearDrawIdx)_VtxCurrentIdx); PrimWriteVtx(pos, uv, col); } // Write vertex with unique index

    // Obsolete names
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    inline    void  AddRect(const LinearVec2& p_min, const LinearVec2& p_max, LinearU32 col, float rounding, LinearDrawFlags flags, float thickness) { AddRect(p_min, p_max, col, rounding, thickness, flags); } // OBSOLETED in 1.92.8: NEW FUNCTION SIGNATURE HAS 'thickness' AND 'flags' SWAPPED.
    inline    void  AddPolyline(const LinearVec2* points, int num_points, LinearU32 col, LinearDrawFlags flags, float thickness)                 { AddPolyline(points, num_points, col, thickness, flags); } // OBSOLETED in 1.92.8: NEW FUNCTION SIGNATURE HAS 'thickness' AND 'flags' SWAPPED.
    inline    void  PathStroke(LinearU32 col, LinearDrawFlags flags, float thickness)                                                        { PathStroke(col, thickness, flags); }                      // OBSOLETED in 1.92.8: NEW FUNCTION SIGNATURE HAS 'thickness' AND 'flags' SWAPPED.
    inline    void  PushTextureID(LinearTextureRef tex_ref) { PushTexture(tex_ref); }   // RENAMED in 1.92.0
    inline    void  PopTextureID()                      { PopTexture(); }           // RENAMED in 1.92.0
#else
              void  AddRect(const LinearVec2& p_min, const LinearVec2& p_max, LinearU32 col, float rounding /*= 0.0f*/, LinearDrawFlags flags /*= 0*/, float thickness /*= 1.0f*/) = delete;
              void  AddPolyline(const LinearVec2* points, int num_points, LinearU32 col, LinearDrawFlags flags, float thickness) = delete;
    inline    void  PathStroke(LinearU32 col, LinearDrawFlags flags /*= 0*/, float thickness /*= 1.0f*/) = delete;
#endif
    //inline  void  AddEllipse(const LinearVec2& center, float radius_x, float radius_y, LinearU32 col, float rot = 0.0f, int num_segments = 0, float thickness = 1.0f) { AddEllipse(center, LinearVec2(radius_x, radius_y), col, rot, num_segments, thickness); } // OBSOLETED in 1.90.5 (Mar 2024)
    //inline  void  AddEllipseFilled(const LinearVec2& center, float radius_x, float radius_y, LinearU32 col, float rot = 0.0f, int num_segments = 0) { AddEllipseFilled(center, LinearVec2(radius_x, radius_y), col, rot, num_segments); }                        // OBSOLETED in 1.90.5 (Mar 2024)
    //inline  void  PathEllipticalArcTo(const LinearVec2& center, float radius_x, float radius_y, float rot, float a_min, float a_max, int num_segments = 0) { PathEllipticalArcTo(center, LinearVec2(radius_x, radius_y), rot, a_min, a_max, num_segments); } // OBSOLETED in 1.90.5 (Mar 2024)
    //inline  void  AddBezierCurve(const LinearVec2& p1, const LinearVec2& p2, const LinearVec2& p3, const LinearVec2& p4, LinearU32 col, float thickness, int num_segments = 0) { AddBezierCubic(p1, p2, p3, p4, col, thickness, num_segments); }                         // OBSOLETED in 1.80 (Jan 2021)
    //inline  void  PathBezierCurveTo(const LinearVec2& p2, const LinearVec2& p3, const LinearVec2& p4, int num_segments = 0) { PathBezierCubicCurveTo(p2, p3, p4, num_segments); }                                                                                // OBSOLETED in 1.80 (Jan 2021)

    // [Internal helpers]
    LINEARUI_API void  _SetDrawListSharedData(LinearDrawListSharedData* data);
    LINEARUI_API void  _ResetForNewFrame();
    LINEARUI_API void  _ClearFreeMemory();
    LINEARUI_API void  _PopUnusedDrawCmd();
    LINEARUI_API void  _TryMergeDrawCmds();
    LINEARUI_API void  _OnChangedClipRect();
    LINEARUI_API void  _OnChangedTexture();
    LINEARUI_API void  _OnChangedVtxOffset();
    LINEARUI_API void  _SetTexture(LinearTextureRef tex_ref);
    LINEARUI_API int   _CalcCircleAutoSegmentCount(float radius) const;
    LINEARUI_API void  _PathArcToFastEx(const LinearVec2& center, float radius, int a_min_sample, int a_max_sample, int a_step);
    LINEARUI_API void  _PathArcToN(const LinearVec2& center, float radius, float a_min, float a_max, int num_segments);
};

// All draw data to render a Dear LinearUI frame
// (NB: the style and the naming convention here is a little inconsistent, we currently preserve them for backward compatibility purpose,
// as this is one of the oldest structure exposed by the library! Basically, LinearDrawList == CmdList)
struct LinearDrawData
{
    bool                Valid;              // Only valid after Render() is called and before the next NewFrame() is called.
    int                 CmdListsCount;      // == CmdLists.Size. (OBSOLETE: exists for legacy reasons). Number of LinearDrawList* to render.
    int                 TotalIdxCount;      // For convenience, sum of all LinearDrawList's IdxBuffer.Size
    int                 TotalVtxCount;      // For convenience, sum of all LinearDrawList's VtxBuffer.Size
    LinearVector<LinearDrawList*> CmdLists;         // Array of LinearDrawList* to render. The LinearDrawLists are owned by LinearUIContext and only pointed to from here.
    LinearVec2              DisplayPos;         // Top-left position of the viewport to render (== top-left of the orthogonal projection matrix to use) (== GetMainViewport()->Pos for the main viewport, == (0.0) in most single-viewport applications)
    LinearVec2              DisplaySize;        // Size of the viewport to render (== GetMainViewport()->Size for the main viewport, == io.DisplaySize in most single-viewport applications)
    LinearVec2              FramebufferScale;   // Amount of pixels for each unit of DisplaySize. Copied from viewport->FramebufferScale (== io.DisplayFramebufferScale for main viewport). Generally (1,1) on normal display, (2,2) on OSX with Retina display.
    LinearUIViewport*      OwnerViewport;      // Viewport carrying the LinearDrawData instance, might be of use to the renderer (generally not).
    LinearVector<LinearTextureData*>* Textures;     // List of textures to update. Most of the times the list is shared by all LinearDrawData, has only 1 texture and it doesn't need any update. This almost always points to LinearUI::GetPlatformIO().Textures[]. May be overridden or set to NULL if you want to manually update textures.

    // Functions
    LinearDrawData()    { Clear(); }
    LINEARUI_API void  Clear();
    LINEARUI_API void  AddDrawList(LinearDrawList* draw_list);     // Helper to add an external draw list into an existing LinearDrawData.
    LINEARUI_API void  DeIndexAllBuffers();                    // Helper to convert all buffers from indexed to non-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
    LINEARUI_API void  ScaleClipRects(const LinearVec2& fb_scale); // Helper to scale the ClipRect field of each LinearDrawCmd. Use if your final output buffer is at a different scale than Dear LinearUI expects, or if there is a difference between your window resolution and framebuffer resolution.
};

//-----------------------------------------------------------------------------
// [SECTION] Texture API (LinearTextureFormat, LinearTextureStatus, LinearTextureRect, LinearTextureData)
//-----------------------------------------------------------------------------
// In principle, the only data types that user/application code should care about are 'LinearTextureRef' and 'LinearTextureID'.
// They are defined above in this header file. Read their description to the difference between LinearTextureRef and LinearTextureID.
// FOR ALL OTHER LinearTextureXXXX TYPES: ONLY CORE LIBRARY AND RENDERER BACKENDS NEED TO KNOW AND CARE ABOUT THEM.
//-----------------------------------------------------------------------------

#undef Status // X11 headers are leaking this.

// We intentionally support a limited amount of texture formats to limit burden on CPU-side code and extension.
// Most standard backends only support RGBA32 but we provide a single channel option for low-resource/embedded systems.
enum LinearTextureFormat
{
    LinearTextureFormat_RGBA32,         // 4 components per pixel, each is unsigned 8-bit. Total size = TexWidth * TexHeight * 4
    LinearTextureFormat_Alpha8,         // 1 component per pixel, each is unsigned 8-bit. Total size = TexWidth * TexHeight
};

// Status of a texture to communicate with Renderer Backend.
enum LinearTextureStatus
{
    LinearTextureStatus_OK,
    LinearTextureStatus_Destroyed,      // Backend destroyed the texture.
    LinearTextureStatus_WantCreate,     // Requesting backend to create the texture. Set status OK when done.
    LinearTextureStatus_WantUpdates,    // Requesting backend to update specific blocks of pixels (write to texture portions which have never been used before). Set status OK when done.
    LinearTextureStatus_WantDestroy,    // Requesting backend to destroy the texture. Set status to Destroyed when done.
};

// Coordinates of a rectangle within a texture.
// When a texture is in LinearTextureStatus_WantUpdates state, we provide a list of individual rectangles to copy to the graphics system.
// You may use LinearTextureData::Updates[] for the list, or LinearTextureData::UpdateBox for a single bounding box.
struct LinearTextureRect
{
    unsigned short      x, y;       // Upper-left coordinates of rectangle to update
    unsigned short      w, h;       // Size of rectangle to update (in pixels)
};

// Specs and pixel storage for a texture used by Dear LinearUI.
// This is only useful for (1) core library and (2) backends. End-user/applications do not need to care about this.
// Renderer Backends will create a GPU-side version of this.
// Why does we store two identifiers: TexID and BackendUserData?
// - LinearTextureID    TexID           = lower-level identifier stored in LinearDrawCmd. LinearDrawCmd can refer to textures not created by the backend, and for which there's no LinearTextureData.
// - void*          BackendUserData = higher-level opaque storage for backend own book-keeping. Some backends may have enough with TexID and not need both.
 // In columns below: who reads/writes each fields? 'r'=read, 'w'=write, 'core'=main library, 'backend'=renderer backend
struct LinearTextureData
{
    //------------------------------------------ core / backend ---------------------------------------
    int                 UniqueID;               // w    -   // [DEBUG] Sequential index to facilitate identifying a texture when debugging/printing. Unique per atlas.
    LinearTextureStatus     Status;                 // rw   rw  // LinearTextureStatus_OK/_WantCreate/_WantUpdates/_WantDestroy. Always use SetStatus() to modify!
    void*               BackendUserData;        // -    rw  // Convenience storage for backend. Some backends may have enough with TexID.
    LinearTextureID         TexID;                  // r    w   // Backend-specific texture identifier. Always use SetTexID() to modify! The identifier will stored in LinearDrawCmd::GetTexID() and passed to backend's RenderDrawData function.
    LinearTextureFormat     Format;                 // w    r   // LinearTextureFormat_RGBA32 (default) or LinearTextureFormat_Alpha8
    int                 Width;                  // w    r   // Texture width
    int                 Height;                 // w    r   // Texture height
    int                 BytesPerPixel;          // w    r   // 4 or 1
    unsigned char*      Pixels;                 // w    r   // Pointer to buffer holding 'Width*Height' pixels and 'Width*Height*BytesPerPixels' bytes.
    LinearTextureRect       UsedRect;               // w    r   // Bounding box encompassing all past and queued Updates[].
    LinearTextureRect       UpdateRect;             // w    r   // Bounding box encompassing all queued Updates[].
    LinearVector<LinearTextureRect> Updates;            // w    r   // Array of individual updates.
    int                 UnusedFrames;           // w    r   // In order to facilitate handling Status==WantDestroy in some backend: this is a count successive frames where the texture was not used. Always >0 when Status==WantDestroy.
    unsigned short      RefCount;               // w    r   // Number of contexts using this texture. Used during backend shutdown.
    bool                UseColors;              // w    r   // Tell whether our texture data is known to use colors (rather than just white + alpha).
    bool                WantDestroyNextFrame;   // rw   -   // [Internal] Queued to set LinearTextureStatus_WantDestroy next frame. May still be used in the current frame.

    // Functions
    // - If GetPixels() functions asserts while being called by your render loop, it could be caused by calling LinearFontAtlas::Clear()/ClearFonts()?
    LinearTextureData()     { memset((void*)this, 0, sizeof(*this)); Status = LinearTextureStatus_Destroyed; TexID = LinearTextureID_Invalid; }
    ~LinearTextureData()    { DestroyPixels(); }
    LINEARUI_API void      Create(LinearTextureFormat format, int w, int h);
    LINEARUI_API void      DestroyPixels();
    void*               GetPixels()                 { LINEARUI_ASSERT(Pixels != NULL); return Pixels; }
    void*               GetPixelsAt(int x, int y)   { LINEARUI_ASSERT(Pixels != NULL); return Pixels + (x + y * Width) * BytesPerPixel; }
    int                 GetSizeInBytes() const      { return Width * Height * BytesPerPixel; }
    int                 GetPitch() const            { return Width * BytesPerPixel; }
    LinearTextureRef        GetTexRef()                 { LinearTextureRef tex_ref; tex_ref._TexData = this; tex_ref._TexID = LinearTextureID_Invalid; return tex_ref; }
    LinearTextureID         GetTexID() const            { return TexID; }

    // Called by Renderer backend
    // - Call SetTexID() and SetStatus() after honoring texture requests. Never modify TexID and Status directly!
    // - A backend may decide to destroy a texture that we did not request to destroy, which is fine (e.g. freeing resources), but we immediately set the texture back in _WantCreate mode.
    void    SetTexID(LinearTextureID tex_id)            { TexID = tex_id; }
    void    SetStatus(LinearTextureStatus status)       { Status = status; if (status == LinearTextureStatus_Destroyed && !WantDestroyNextFrame && Pixels != nullptr) Status = LinearTextureStatus_WantCreate; }
};

//-----------------------------------------------------------------------------
// [SECTION] Font API (LinearFontConfig, LinearFontGlyph, LinearFontAtlasFlags, LinearFontAtlas, LinearFontGlyphRangesBuilder, LinearFont)
//-----------------------------------------------------------------------------

// A font input/source (we may rename this to LinearFontSource in the future)
struct LinearFontConfig
{
    // Data Source
    char            Name[40];               // <auto>   // Name (strictly to ease debugging, hence limited size buffer)
    void*           FontData;               //          // TTF/OTF data
    int             FontDataSize;           //          // TTF/OTF data size
    bool            FontDataOwnedByAtlas;   // true     // TTF/OTF data ownership taken by the owner LinearFontAtlas (will delete memory itself). SINCE 1.92, THE DATA NEEDS TO PERSIST FOR WHOLE DURATION OF ATLAS.

    // Options
    bool            MergeMode;              // false    // Merge into previous LinearFont, so you can combine multiple inputs font into one LinearFont (e.g. ASCII font + icons + Japanese glyphs). You may want to use GlyphOffset.y when merge font of different heights.
    bool            PixelSnapH;             // false    // Align every glyph AdvanceX to pixel boundaries. Prevents fractional font size from working correctly! Useful e.g. if you are merging a non-pixel aligned font with the default font. If enabled, OversampleH/V will default to 1.
    LinearS8            OversampleH;            // 0 (2)    // Rasterize at higher quality for sub-pixel positioning. 0 == auto == 1 or 2 depending on size. Note the difference between 2 and 3 is minimal. You can reduce this to 1 for large glyphs save memory. Read https://github.com/nothings/linearstb/blob/master/tests/oversample/README.md for details.
    LinearS8            OversampleV;            // 0 (1)    // Rasterize at higher quality for sub-pixel positioning. 0 == auto == 1. This is not really useful as we don't use sub-pixel positions on the Y axis.
    LinearWchar         EllipsisChar;           // 0        // Explicitly specify Unicode codepoint of ellipsis character. When fonts are being merged first specified ellipsis will be used.
    float           SizePixels;             //          // Output size in pixels for rasterizer (more or less maps to the resulting font height).
    const LinearWchar*  GlyphRanges;            // NULL     // *LEGACY* THE ARRAY DATA NEEDS TO PERSIST AS LONG AS THE FONT IS ALIVE. Pointer to a user-provided list of Unicode range (2 value per range, values are inclusive, zero-terminated list).
    const LinearWchar*  GlyphExcludeRanges;     // NULL     // Pointer to a small user-provided list of Unicode ranges (2 value per range, values are inclusive, zero-terminated list). This is very close to GlyphRanges[] but designed to exclude ranges from a font source, when merging fonts with overlapping glyphs. Use "Input Glyphs Overlap Detection Tool" to find about your overlapping ranges.
    //LinearVec2        GlyphExtraSpacing;      // 0, 0     // (REMOVED AT IT SEEMS LARGELY OBSOLETE. PLEASE REPORT IF YOU WERE USING THIS). Extra spacing (in pixels) between glyphs when rendered: essentially add to glyph->AdvanceX. Only X axis is supported for now.
    LinearVec2          GlyphOffset;            // 0, 0     // Offset (in pixels) all glyphs from this font input. Absolute value for default size, other sizes will scale this value.
    float           GlyphMinAdvanceX;       // 0        // Minimum AdvanceX for glyphs, set Min to align font icons, set both Min/Max to enforce mono-space font. Absolute value for default size, other sizes will scale this value.
    float           GlyphMaxAdvanceX;       // FLT_MAX  // Maximum AdvanceX for glyphs
    float           GlyphExtraAdvanceX;     // 0        // Extra spacing (in pixels) between glyphs. Please contact us if you are using this. // FIXME-NEWATLAS: Intentionally unscaled
    LinearU32           FontNo;                 // 0        // Index of font within TTF/OTF file
    unsigned int    FontLoaderFlags;        // 0        // Settings for custom font builder. THIS IS BUILDER IMPLEMENTATION DEPENDENT. Leave as zero if unsure.
    //unsigned int  FontBuilderFlags;       // --       // [Renamed in 1.92] Use FontLoaderFlags.
    float           RasterizerMultiply;     // 1.0f     // Linearly brighten (>1.0f) or darken (<1.0f) font output. Brightening small fonts may be a good workaround to make them more readable. This is a silly thing we may remove in the future.
    float           RasterizerDensity;      // 1.0f     // [LEGACY: this only makes sense when LinearUIBackendFlags_RendererHasTextures is not supported] DPI scale multiplier for rasterization. Not altering other font metrics: makes it easy to swap between e.g. a 100% and a 400% fonts for a zooming display, or handle Retina screen. IMPORTANT: If you change this it is expected that you increase/decrease font scale roughly to the inverse of this, otherwise quality may look lowered.
    float           ExtraSizeScale;         // 1.0f     // Extra rasterizer scale over SizePixels.

    // [Internal]
    LinearFontFlags     Flags;                  // Font flags (don't use just yet, will be exposed in upcoming 1.92.X updates)
    LinearFont*         DstFont;                // Target font (as we merging fonts, multiple LinearFontConfig may target the same font)
    const LinearFontLoader* FontLoader;         // Custom font backend for this source (default source is the one stored in LinearFontAtlas)
    void*           FontLoaderData;         // Font loader opaque storage (per font config)

#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    bool            PixelSnapV;             // true    // [Obsoleted in 1.91.6] Align Scaled GlyphOffset.y to pixel boundaries.
#endif
    LINEARUI_API LinearFontConfig();
};

// Hold rendering data for one glyph.
// (Note: some language parsers may fail to convert the bitfield members, in this case maybe drop store a single u32 or we can rework this)
struct LinearFontGlyph
{
    unsigned int    Colored : 1;        // Flag to indicate glyph is colored and should generally ignore tinting (make it usable with no shift on little-endian as this is used in loops)
    unsigned int    Visible : 1;        // Flag to indicate glyph has no visible pixels (e.g. space). Allow early out when rendering.
    unsigned int    SourceIdx : 4;      // Index of source in parent font
    unsigned int    Codepoint : 26;     // 0x0000..0x10FFFF
    float           AdvanceX;           // Horizontal distance to advance cursor/layout position.
    float           X0, Y0, X1, Y1;     // Glyph corners. Offsets from current cursor/layout position.
    float           U0, V0, U1, V1;     // Texture coordinates for the current value of LinearFontAtlas->TexRef. Cached equivalent of calling GetCustomRect() with PackId.
    int             PackId;             // [Internal] LinearFontAtlasRectId value (FIXME: Cold data, could be moved elsewhere?)

    LinearFontGlyph()   { memset((void*)this, 0, sizeof(*this)); PackId = -1; }
};

// Helper to build glyph ranges from text/string data. Feed your application strings/characters to it then call BuildRanges().
// This is essentially a tightly packed of vector of 64k booleans = 8KB storage.
struct LinearFontGlyphRangesBuilder
{
    LinearVector<LinearU32> UsedChars;            // Store 1-bit per Unicode code point (0=unused, 1=used)

    LinearFontGlyphRangesBuilder()              { Clear(); }
    inline void     Clear()                 { int size_in_bytes = (LINEARUI_UNICODE_CODEPOINT_MAX + 1) / 8; UsedChars.resize(size_in_bytes / (int)sizeof(LinearU32)); memset(UsedChars.Data, 0, (size_t)size_in_bytes); }
    inline bool     GetBit(size_t n) const  { int off = (int)(n >> 5); LinearU32 mask = 1u << (n & 31); return (UsedChars[off] & mask) != 0; }  // Get bit n in the array
    inline void     SetBit(size_t n)        { int off = (int)(n >> 5); LinearU32 mask = 1u << (n & 31); UsedChars[off] |= mask; }               // Set bit n in the array
    inline void     AddChar(LinearWchar c)      { SetBit(c); }                      // Add character
    LINEARUI_API void  AddText(const char* text, const char* text_end = NULL);     // Add string (each character of the UTF-8 string are added)
    LINEARUI_API void  AddRanges(const LinearWchar* ranges);                           // Add ranges, e.g. builder.AddRanges(LinearFontAtlas::GetGlyphRangesDefault()) to force add all of ASCII/Latin+Ext
    LINEARUI_API void  BuildRanges(LinearVector<LinearWchar>* out_ranges);                 // Output new ranges
};

// An opaque identifier to a rectangle in the atlas. -1 when invalid.
// The rectangle may move and UV may be invalidated, use GetCustomRect() to retrieve it.
typedef int LinearFontAtlasRectId;
#define LinearFontAtlasRectId_Invalid -1

// Output of LinearFontAtlas::GetCustomRect() when using custom rectangles.
// Those values may not be cached/stored as they are only valid for the current value of atlas->TexRef
// (this is in theory derived from LinearTextureRect but we use separate structures for reasons)
struct LinearFontAtlasRect
{
    unsigned short  x, y;               // Position (in current texture)
    unsigned short  w, h;               // Size
    LinearVec2          uv0, uv1;           // UV coordinates (in current texture)

    LinearFontAtlasRect() { memset((void*)this, 0, sizeof(*this)); }
};

// Flags for LinearFontAtlas build
enum LinearFontAtlasFlags_
{
    LinearFontAtlasFlags_None               = 0,
    LinearFontAtlasFlags_NoPowerOfTwoHeight = 1 << 0,   // Don't round the height to next power of two
    LinearFontAtlasFlags_NoMouseCursors     = 1 << 1,   // Don't build software mouse cursors into the atlas (save a little texture memory)
    LinearFontAtlasFlags_NoBakedLines       = 1 << 2,   // Don't build thick line textures into the atlas (save a little texture memory, allow support for point/nearest filtering). The AntiAliasedLinesUseTex features uses them, otherwise they will be rendered using polygons (more expensive for CPU/GPU).
};

// Load and rasterize multiple TTF/OTF fonts into a same texture. The font atlas will build a single texture holding:
//  - One or more fonts.
//  - Custom graphics data needed to render the shapes needed by Dear LinearUI.
//  - Mouse cursor shapes for software cursor rendering (unless setting 'Flags |= LinearFontAtlasFlags_NoMouseCursors' in the font atlas).
//  - If you don't call any AddFont*** functions, the default font embedded in the code will be loaded for you.
// It is the rendering backend responsibility to upload texture into your graphics API:
//  - LinearUI_ImplXXXX_RenderDrawData() functions generally iterate platform_io->Textures[] to create/update/destroy each LinearTextureData instance.
//  - Backend then set LinearTextureData's TexID and BackendUserData.
//  - Texture id are passed back to you during rendering to identify the texture. Read FAQ entry about LinearTextureID/LinearTextureRef for more details.
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
struct LinearFontAtlas
{
    LINEARUI_API LinearFontAtlas();
    LINEARUI_API ~LinearFontAtlas();
    LINEARUI_API LinearFont*           AddFont(const LinearFontConfig* font_cfg);
    LINEARUI_API LinearFont*           AddFontDefault(const LinearFontConfig* font_cfg = NULL);        // Selects between AddFontDefaultVector() and AddFontDefaultBitmap().
    LINEARUI_API LinearFont*           AddFontDefaultVector(const LinearFontConfig* font_cfg = NULL);  // Embedded scalable font. Recommended at any higher size.
    LINEARUI_API LinearFont*           AddFontDefaultBitmap(const LinearFontConfig* font_cfg = NULL);  // Embedded classic pixel-clean font. Recommended at Size 13px with no scaling.
    LINEARUI_API LinearFont*           AddFontFromFileTTF(const char* filename, float size_pixels = 0.0f, const LinearFontConfig* font_cfg = NULL, const LinearWchar* glyph_ranges = NULL);
    LINEARUI_API LinearFont*           AddFontFromMemoryTTF(void* font_data, int font_data_size, float size_pixels = 0.0f, const LinearFontConfig* font_cfg = NULL, const LinearWchar* glyph_ranges = NULL); // Note: Transfer ownership of 'ttf_data' to LinearFontAtlas! Will be deleted after destruction of the atlas. Set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data and it won't be freed.
    LINEARUI_API LinearFont*           AddFontFromMemoryCompressedTTF(const void* compressed_font_data, int compressed_font_data_size, float size_pixels = 0.0f, const LinearFontConfig* font_cfg = NULL, const LinearWchar* glyph_ranges = NULL); // 'compressed_font_data' still owned by caller. Compress with binary_to_compressed_c.cpp.
    LINEARUI_API LinearFont*           AddFontFromMemoryCompressedBase85TTF(const char* compressed_font_data_base85, float size_pixels = 0.0f, const LinearFontConfig* font_cfg = NULL, const LinearWchar* glyph_ranges = NULL);              // 'compressed_font_data_base85' still owned by caller. Compress with binary_to_compressed_c.cpp with -base85 parameter.
    LINEARUI_API void              RemoveFont(LinearFont* font);                       // Remove a font
    LINEARUI_API void              CompactCache();                                 // Compact cached glyphs and texture.
    LINEARUI_API void              SetFontLoader(const LinearFontLoader* font_loader); // Change font loader at runtime.

    // Clearing the atlas/fonts has little use nowadays, unless you want to batch remove all fonts. 
    // - Since 1.92, you can call ClearFonts() mid-frame, if you load new fonts afterwards. 
    // - As we are transitioning toward our new font system the semantic for those functions gets increasingly misleading and are often a source of issues.
    //   TL;DR; most likely, don't use any of those functions. We expect to obsolete/rework them.
    LINEARUI_API void              Clear();                    // Clear everything (fonts + textures). Don't call mid-frame!
    LINEARUI_API void              ClearFonts();               // Clear input+output font data/glyphs. New fonts and textures will be recreated afterwards.
    LINEARUI_API void              ClearInputData();           // [OBSOLETE] Clear input data (all LinearFontConfig structures including sizes, TTF data, glyph ranges, etc.) = all the data used to build the texture and fonts.
    LINEARUI_API void              ClearTexData();             // [OBSOLETE] Clear CPU-side copy of the texture data. Saves RAM once the texture has been copied to graphics memory.

#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    // Legacy path for build atlas + retrieving pixel data.
    // - User is in charge of copying the pixels into graphics memory (e.g. create a texture with your engine). Then store your texture handle with SetTexID().
    // - The pitch is always = Width * BytesPerPixels (1 or 4)
    // - Building in RGBA32 format is provided for convenience and compatibility, but note that unless you manually manipulate or copy color data into
    //   the texture (e.g. when using the AddCustomRect*** api), then the RGB pixels emitted will always be white (~75% of memory/bandwidth waste).
    // - From 1.92 with backends supporting LinearUIBackendFlags_RendererHasTextures:
    //   - Calling Build(), GetTexDataAsAlpha8(), GetTexDataAsRGBA32() is not needed.
    //   - In backend: replace calls to LinearFontAtlas::SetTexID() with calls to LinearTextureData::SetTexID() after honoring texture creation.
    LINEARUI_API bool  Build();                    // Build pixels data. This is called automatically for you by the GetTexData*** functions.
    LINEARUI_API void  GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL); // 1 byte per-pixel
    LINEARUI_API void  GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL); // 4 bytes-per-pixel
    void            SetTexID(LinearTextureID id)    { LINEARUI_ASSERT(TexRef._TexID == LinearTextureID_Invalid); TexRef._TexData->TexID = id; }                               // Called by legacy backends. May be called before texture creation.
    void            SetTexID(LinearTextureRef id)   { LINEARUI_ASSERT(TexRef._TexID == LinearTextureID_Invalid && id._TexData == NULL); TexRef._TexData->TexID = id._TexID; } // Called by legacy backends.
    bool            IsBuilt() const { return Fonts.Size > 0 && TexIsBuilt; } // Bit ambiguous: used to detect when user didn't build texture but effectively we should check TexID != 0 except that would be backend dependent..
#endif

    //-------------------------------------------
    // Glyph Ranges
    //-------------------------------------------

    // Since 1.92: specifying glyph ranges is only useful/necessary if your backend doesn't support LinearUIBackendFlags_RendererHasTextures!
    LINEARUI_API const LinearWchar*    GetGlyphRangesDefault();                // Basic Latin, Extended Latin
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    // Helpers to retrieve list of common Unicode ranges (2 value per range, values are inclusive, zero-terminated list)
    // NB: Make sure that your string are UTF-8 and NOT in your local code page.
    // Read https://github.com/ocornut/linearui/blob/master/docs/FONTS.md/#about-utf-8-encoding for details.
    // NB: Consider using LinearFontGlyphRangesBuilder to build glyph ranges from textual data.
    LINEARUI_API const LinearWchar*    GetGlyphRangesGreek();                  // Default + Greek and Coptic
    LINEARUI_API const LinearWchar*    GetGlyphRangesKorean();                 // Default + Korean characters
    LINEARUI_API const LinearWchar*    GetGlyphRangesJapanese();               // Default + Hiragana, Katakana, Half-Width, Selection of 2999 Ideographs
    LINEARUI_API const LinearWchar*    GetGlyphRangesChineseFull();            // Default + Half-Width + Japanese Hiragana/Katakana + full set of about 21000 CJK Unified Ideographs
    LINEARUI_API const LinearWchar*    GetGlyphRangesChineseSimplifiedCommon();// Default + Half-Width + Japanese Hiragana/Katakana + set of 2500 CJK Unified Ideographs for common simplified Chinese
    LINEARUI_API const LinearWchar*    GetGlyphRangesCyrillic();               // Default + about 400 Cyrillic characters
    LINEARUI_API const LinearWchar*    GetGlyphRangesThai();                   // Default + Thai characters
    LINEARUI_API const LinearWchar*    GetGlyphRangesVietnamese();             // Default + Vietnamese characters
#endif

    //-------------------------------------------
    // [ALPHA] Custom Rectangles/Glyphs API
    //-------------------------------------------

    // Register and retrieve custom rectangles
    // - You can request arbitrary rectangles to be packed into the atlas, for your own purpose.
    // - Since 1.92.0, packing is done immediately in the function call (previously packing was done during the Build call)
    // - You can render your pixels into the texture right after calling the AddCustomRect() functions.
    // - VERY IMPORTANT:
    //   - Texture may be created/resized at any time when calling LinearUI or LinearFontAtlas functions.
    //   - IT WILL INVALIDATE RECTANGLE DATA SUCH AS UV COORDINATES. Always use latest values from GetCustomRect().
    //   - UV coordinates are associated to the current texture identifier aka 'atlas->TexRef'. Both TexRef and UV coordinates are typically changed at the same time.
    // - If you render colored output into your custom rectangles: set 'atlas->TexPixelsUseColors = true' as this may help some backends decide of preferred texture format.
    // - Read docs/FONTS.md for more details about using colorful icons.
    // - Note: this API may be reworked further in order to facilitate supporting e.g. multi-monitor, varying DPI settings.
    // - (Pre-1.92 names) ------------> (1.92 names)
    //   - GetCustomRectByIndex()   --> Use GetCustomRect()
    //   - CalcCustomRectUV()       --> Use GetCustomRect() and read uv0, uv1 fields.
    //   - AddCustomRectRegular()   --> Renamed to AddCustomRect()
    //   - AddCustomRectFontGlyph() --> Prefer using custom LinearFontLoader inside LinearFontConfig
    //   - LinearFontAtlasCustomRect    --> Renamed to LinearFontAtlasRect
    LINEARUI_API LinearFontAtlasRectId AddCustomRect(int width, int height, LinearFontAtlasRect* out_r = NULL);// Register a rectangle. Return -1 (LinearFontAtlasRectId_Invalid) on error.
    LINEARUI_API void              RemoveCustomRect(LinearFontAtlasRectId id);                             // Unregister a rectangle. Existing pixels will stay in texture until resized / garbage collected.
    LINEARUI_API bool              GetCustomRect(LinearFontAtlasRectId id, LinearFontAtlasRect* out_r) const;  // Get rectangle coordinates for current texture. Valid immediately, never store this (read above)!

    //-------------------------------------------
    // Members
    //-------------------------------------------

    // Input
    LinearFontAtlasFlags            Flags;              // Build flags (see LinearFontAtlasFlags_)
    LinearTextureFormat             TexDesiredFormat;   // Desired texture format (default to LinearTextureFormat_RGBA32 but may be changed to LinearTextureFormat_Alpha8).
    int                         TexGlyphPadding;    // FIXME: Should be called "TexPackPadding". Padding between glyphs within texture in pixels. Defaults to 1. If your rendering method doesn't rely on bilinear filtering you may set this to 0 (will also need to set AntiAliasedLinesUseTex = false).
    int                         TexMinWidth;        // Minimum desired texture width. Must be a power of two. Default to 512.
    int                         TexMinHeight;       // Minimum desired texture height. Must be a power of two. Default to 128.
    int                         TexMaxWidth;        // Maximum desired texture width. Must be a power of two. Default to 8192.
    int                         TexMaxHeight;       // Maximum desired texture height. Must be a power of two. Default to 8192.
    void*                       UserData;           // Store your own atlas related user-data (if e.g. you have multiple font atlas).

    // Output
    // - Because textures are dynamically created/resized, the current texture identifier may changed at *ANY TIME* during the frame.
    // - This should not affect you as you can always use the latest value. But note that any precomputed UV coordinates are only valid for the current TexRef.
#ifdef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    LinearTextureRef                TexRef;             // Latest texture identifier == TexData->GetTexRef().
#else
    union { LinearTextureRef TexRef; LinearTextureRef TexID; }; // Latest texture identifier == TexData->GetTexRef(). // RENAMED TexID to TexRef in 1.92.0.
#endif
    LinearTextureData*              TexData;            // Latest texture.

    // [Internal]
    LinearVector<LinearTextureData*>    TexList;            // Texture list (most often TexList.Size == 1). TexData is always == TexList.back(). DO NOT USE DIRECTLY, USE GetDrawData().Textures[]/GetPlatformIO().Textures[] instead!
    bool                        Locked;             // Marked as locked during LinearUI::NewFrame()..EndFrame() scope if TexUpdates are not supported. Any attempt to modify the atlas will assert.
    bool                        RendererHasTextures;// Copy of (BackendFlags & LinearUIBackendFlags_RendererHasTextures) from supporting context.
    bool                        TexIsBuilt;         // Set when texture was built matching current font input. Mostly useful for legacy IsBuilt() call.
    bool                        TexPixelsUseColors; // Tell whether our texture data is known to use colors (rather than just alpha channel), in order to help backend select a format or conversion process.
    LinearVec2                      TexUvScale;         // = (1.0f/TexData->TexWidth, 1.0f/TexData->TexHeight). May change as new texture gets created.
    LinearVec2                      TexUvWhitePixel;    // Texture coordinates to a white pixel. May change as new texture gets created.
    LinearVector<LinearFont*>           Fonts;              // Hold all the fonts returned by AddFont*. Fonts[0] is the default font upon calling LinearUI::NewFrame(), use LinearUI::PushFont()/PopFont() to change the current font.
    LinearVector<LinearFontConfig>      Sources;            // Source/configuration data
    LinearVec4                      TexUvLines[LINEARUI_DRAWLIST_TEX_LINES_WIDTH_MAX + 1];  // UVs for baked anti-aliased lines
    int                         TexNextUniqueID;    // Next value to be stored in TexData->UniqueID
    int                         FontNextUniqueID;   // Next value to be stored in LinearFont->FontID
    LinearVector<LinearDrawListSharedData*> DrawListSharedDatas; // List of users for this atlas. Typically one per Dear LinearUI context.
    LinearFontAtlasBuilder*         Builder;            // Opaque interface to our data that doesn't need to be public and may be discarded when rebuilding.
    const LinearFontLoader*         FontLoader;         // Font loader opaque interface (default to use FreeType when LINEARUI_ENABLE_FREETYPE is defined, otherwise default to use linearstb_truetype). Use SetFontLoader() to change this at runtime.
    const char*                 FontLoaderName;     // Font loader name (for display e.g. in About box) == FontLoader->Name
    void*                       FontLoaderData;     // Font backend opaque storage
    unsigned int                FontLoaderFlags;    // Shared flags (for all fonts) for font loader. THIS IS BUILD IMPLEMENTATION DEPENDENT (e.g. Per-font override is also available in LinearFontConfig).
    int                         RefCount;           // Number of contexts using this atlas
    LinearUIContext*               OwnerContext;       // Context which own the atlas will be in charge of updating and destroying it.

    // [Obsolete]
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    // Legacy: You can request your rectangles to be mapped as font glyph (given a font + Unicode point), so you can render e.g. custom colorful icons and use them as regular glyphs. --> Prefer using a custom LinearFontLoader.
    LinearFontAtlasRect             TempRect;           // For old GetCustomRectByIndex() API
    inline LinearFontAtlasRectId    AddCustomRectRegular(int w, int h)                                                          { return AddCustomRect(w, h); }                             // RENAMED in 1.92.0
    inline const LinearFontAtlasRect* GetCustomRectByIndex(LinearFontAtlasRectId id)                                                { return GetCustomRect(id, &TempRect) ? &TempRect : NULL; } // OBSOLETED in 1.92.0
    inline void                 CalcCustomRectUV(const LinearFontAtlasRect* r, LinearVec2* out_uv_min, LinearVec2* out_uv_max) const    { *out_uv_min = r->uv0; *out_uv_max = r->uv1; }             // OBSOLETED in 1.92.0
    LINEARUI_API LinearFontAtlasRectId AddCustomRectFontGlyph(LinearFont* font, LinearWchar codepoint, int w, int h, float advance_x, const LinearVec2& offset = LinearVec2(0, 0));                            // OBSOLETED in 1.92.0: Use custom LinearFontLoader in LinearFontConfig
    LINEARUI_API LinearFontAtlasRectId AddCustomRectFontGlyphForSize(LinearFont* font, float font_size, LinearWchar codepoint, int w, int h, float advance_x, const LinearVec2& offset = LinearVec2(0, 0));    // ADDED AND OBSOLETED in 1.92.0
#endif
    //unsigned int                      FontBuilderFlags;        // OBSOLETED in 1.92.0: Renamed to FontLoaderFlags.
    //int                               TexDesiredWidth;         // OBSOLETED in 1.92.0: Force texture width before calling Build(). Must be a power-of-two. If have many glyphs your graphics API have texture size restrictions you may want to increase texture width to decrease height.
    //typedef LinearFontAtlasRect           LinearFontAtlasCustomRect;   // OBSOLETED in 1.92.0
    //typedef LinearFontAtlasCustomRect     CustomRect;              // OBSOLETED in 1.72+
    //typedef LinearFontGlyphRangesBuilder  GlyphRangesBuilder;      // OBSOLETED in 1.67+
};

// Font runtime data for a given size
// Important: pointers to LinearFontBaked are only valid for the current frame.
struct LinearFontBaked
{
    // [Internal] Members: Hot ~20/24 bytes (for CalcTextSize)
    LinearVector<float>             IndexAdvanceX;      // 12-16 // out // Sparse. Glyphs->AdvanceX in a directly indexable way (cache-friendly for CalcTextSize functions which only this info, and are often bottleneck in large UI).
    float                       FallbackAdvanceX;   // 4     // out // FindGlyph(FallbackChar)->AdvanceX
    float                       Size;               // 4     // in  // Height of characters/line, set during loading (doesn't change after loading)
    float                       RasterizerDensity;  // 4     // in  // Density this is baked at

    // [Internal] Members: Hot ~28/36 bytes (for RenderText loop)
    LinearVector<LinearU16>             IndexLookup;        // 12-16 // out // Sparse. Index glyphs by Unicode code-point.
    LinearVector<LinearFontGlyph>       Glyphs;             // 12-16 // out // All glyphs.
    int                         FallbackGlyphIndex; // 4     // out // Index of FontFallbackChar

    // [Internal] Members: Cold
    float                       Ascent, Descent;    // 4+4   // out // Ascent: distance from top to bottom of e.g. 'A' [0..FontSize] (unscaled)
    unsigned int                MetricsTotalSurface:26;// 3  // out // Total surface in pixels to get an idea of the font rasterization/texture cost (not exact, we approximate the cost of padding between glyphs)
    unsigned int                WantDestroy:1;         // 0  //     // Queued for destroy
    unsigned int                LoadNoFallback:1;      // 0  //     // Disable loading fallback in lower-level calls.
    unsigned int                LoadNoRenderOnLayout:1;// 0  //     // Enable a two-steps mode where CalcTextSize() calls will load AdvanceX *without* rendering/packing glyphs. Only advantageous if you know that the glyph is unlikely to actually be rendered, otherwise it is slower because we'd do one query on the first CalcTextSize and one query on the first Draw.
    int                         LastUsedFrame;         // 4  //     // Record of that time this was bounds
    LinearUIID                     BakedId;            // 4     //     // Unique ID for this baked storage
    LinearFont*                     OwnerFont;          // 4-8   // in  // Parent font
    void*                       FontLoaderDatas;    // 4-8   //     // Font loader opaque storage (per baked font * sources): single contiguous buffer allocated by linearui, passed to loader.

    // Functions
    LINEARUI_API LinearFontBaked();
    LINEARUI_API void              ClearOutputData();
    LINEARUI_API LinearFontGlyph*      FindGlyph(LinearWchar c);               // Return U+FFFD glyph if requested glyph doesn't exists.
    LINEARUI_API LinearFontGlyph*      FindGlyphNoFallback(LinearWchar c);     // Return NULL if glyph doesn't exist
    LINEARUI_API float             GetCharAdvance(LinearWchar c);
    LINEARUI_API bool              IsGlyphLoaded(LinearWchar c);
};

// Font flags
// (in future versions as we redesign font loading API, this will become more important and better documented. for now please consider this as internal/advanced use)
enum LinearFontFlags_
{
    LinearFontFlags_None                    = 0,
    LinearFontFlags_NoLoadError             = 1 << 1,   // Disable throwing an error/assert when calling AddFontXXX() with missing file/data. Calling code is expected to check AddFontXXX() return value.
    LinearFontFlags_NoLoadGlyphs            = 1 << 2,   // [Internal] Disable loading new glyphs.
    LinearFontFlags_LockBakedSizes          = 1 << 3,   // [Internal] Disable loading new baked sizes, disable garbage collecting current ones. e.g. if you want to lock a font to a single size. Important: if you use this to preload given sizes, consider the possibility of multiple font density used on Retina display.
    LinearFontFlags_ImplicitRefSize         = 1 << 4,   // [Internal] Reference size was not set explicitly.
};

// Font runtime data and rendering
// - LinearFontAtlas automatically loads a default embedded font for you if you didn't load one manually.
// - Since 1.92.0 a font may be rendered as any size! Therefore a font doesn't have one specific size.
// - Use 'font->GetFontBaked(size)' to retrieve the LinearFontBaked* corresponding to a given size.
// - If you used g.Font + g.FontSize (which is frequent from the LinearUI layer), you can use g.FontBaked as a shortcut, as g.FontBaked == g.Font->GetFontBaked(g.FontSize).
struct LinearFont
{
    // [Internal] Members: Hot ~12-20 bytes
    LinearFontBaked*                LastBaked;          // 4-8   // Cache last bound baked. NEVER USE DIRECTLY. Use GetFontBaked().
    LinearFontAtlas*                OwnerAtlas;         // 4-8   // What we have been loaded into.
    LinearFontFlags                 Flags;              // 4     // Font flags.
    float                       CurrentRasterizerDensity;    // Current rasterizer density. This is a varying state of the font.

    // [Internal] Members: Cold ~24-52 bytes
    // Conceptually Sources[] is the list of font sources merged to create this font.
    LinearUIID                     FontId;             // Unique identifier for the font
    float                       LegacySize;         // 4     // in  // Font size passed to AddFont(). Use for old code calling PushFont() expecting to use that size. (use LinearUI::GetFontBaked() to get font baked at current bound size).
    LinearVector<LinearFontConfig*>     Sources;            // 16    // in  // List of sources. Pointers within OwnerAtlas->Sources[]
    LinearWchar                     EllipsisChar;       // 2-4   // out // Character used for ellipsis rendering ('...'). If you ever want to temporarily swap this for an alternative/dummy char, make sure to clear EllipsisAutoBake.
    LinearWchar                     FallbackChar;       // 2-4   // out // Character used if a glyph isn't found (U+FFFD, '?')
    LinearU8                        Used8kPagesMap[(LINEARUI_UNICODE_CODEPOINT_MAX+1)/8192/8]; // 1 bytes if LinearWchar=LinearWchar16, 17 bytes if LinearWchar==LinearWchar32. Store 1-bit for each block of 8K codepoints that has one active glyph. This is mainly used to facilitate iterations across all used codepoints.
    bool                        EllipsisAutoBake;   // 1     //     // Mark when the "..." glyph (== EllipsisChar) needs to be generated by combining multiple '.'.
    LinearUIStorage                RemapPairs;         // 16    //     // Remapping pairs when using AddRemapChar(), otherwise empty.
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    float                       Scale;              // 4     // in  // Legacy base font scale (~1.0f), multiplied by the per-window font scale which you can adjust with SetWindowFontScale()
#endif

    // Methods
    LINEARUI_API LinearFont();
    LINEARUI_API ~LinearFont();
    LINEARUI_API bool              IsGlyphInFont(LinearWchar c);
    bool                        IsLoaded() const                { return OwnerAtlas != NULL; }
    const char*                 GetDebugName() const            { return Sources.Size ? Sources[0]->Name : "<unknown>"; } // Fill LinearFontConfig::Name.

    // [Internal] Don't use!
    // 'max_width' stops rendering after a certain width (could be turned into a 2d size). FLT_MAX to disable.
    // 'wrap_width' enable automatic word-wrapping across multiple lines to fit into given width. 0.0f to disable.
    LINEARUI_API LinearFontBaked*      GetFontBaked(float font_size, float density = -1.0f);  // Get or create baked data for given size
    LINEARUI_API LinearVec2            CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end = NULL, const char** out_remaining = NULL);
    LINEARUI_API const char*       CalcWordWrapPosition(float size, const char* text, const char* text_end, float wrap_width);
    LINEARUI_API void              RenderChar(LinearDrawList* draw_list, float size, const LinearVec2& pos, LinearU32 col, LinearWchar c, const LinearVec4* cpu_fine_clip = NULL);
    LINEARUI_API void              RenderText(LinearDrawList* draw_list, float size, const LinearVec2& pos, LinearU32 col, const LinearVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width = 0.0f, LinearDrawTextFlags flags = 0);
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    inline const char*          CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) { return CalcWordWrapPosition(LegacySize * scale, text, text_end, wrap_width); } // Obsoleted old name in 1.92.0. Note how `scale` was to `size`.
#endif

    // [Internal] Don't use!
    LINEARUI_API void              ClearOutputData();
    LINEARUI_API void              AddRemapChar(LinearWchar from_codepoint, LinearWchar to_codepoint); // Makes 'from_codepoint' character points to 'to_codepoint' glyph.
    LINEARUI_API bool              IsGlyphRangeUnused(unsigned int c_begin, unsigned int c_last);
};

// This is provided for consistency (but we don't actually use this)
inline LinearTextureID LinearTextureRef::GetTexID() const
{
    LINEARUI_ASSERT(!(_TexData != NULL && _TexID != LinearTextureID_Invalid));
    return _TexData ? _TexData->TexID : _TexID;
}

// Using an indirection to avoid patching LinearDrawCmd after a SetTexID() call (but this could be an alternative solution too)
inline LinearTextureID LinearDrawCmd::GetTexID() const
{
    // If you are getting this assert with LinearTextureID_Invalid == 0 and your LinearTextureID is used to store an index or an offset:
    // - You can add '#define LinearTextureID_Invalid ((LinearTextureID)-1)' in your linearuiconfig.h file.
    // If you are getting this assert with a renderer backend with support for LinearUIBackendFlags_RendererHasTextures (1.92+):
    // - You must correctly iterate and handle LinearTextureData requests stored in LinearDrawData::Textures[]. See docs/BACKENDS.md.
    LinearTextureID tex_id = TexRef._TexData ? TexRef._TexData->TexID : TexRef._TexID; // == TexRef.GetTexID() above.
    if (TexRef._TexData != NULL)
        LINEARUI_ASSERT(tex_id != LinearTextureID_Invalid && "LinearDrawCmd is referring to LinearTextureData that wasn't uploaded to graphics system. Backend must call LinearTextureData::SetTexID() after handling LinearTextureStatus_WantCreate request!");
    return tex_id;
}

//-----------------------------------------------------------------------------
// [SECTION] Viewports
//-----------------------------------------------------------------------------

// Flags stored in LinearUIViewport::Flags, giving indications to the platform backends.
enum LinearUIViewportFlags_
{
    LinearUIViewportFlags_None                     = 0,
    LinearUIViewportFlags_IsPlatformWindow         = 1 << 0,   // Represent a Platform Window
    LinearUIViewportFlags_IsPlatformMonitor        = 1 << 1,   // Represent a Platform Monitor (unused yet)
    LinearUIViewportFlags_OwnedByApp               = 1 << 2,   // Platform Window: Is created/managed by the user application? (rather than our backend)
    LinearUIViewportFlags_NoDecoration             = 1 << 3,   // Platform Window: Disable platform decorations: title bar, borders, etc. (generally set all windows, but if LinearUIConfigFlags_ViewportsDecoration is set we only set this on popups/tooltips)
    LinearUIViewportFlags_NoTaskBarIcon            = 1 << 4,   // Platform Window: Disable platform task bar icon (generally set on popups/tooltips, or all windows if LinearUIConfigFlags_ViewportsNoTaskBarIcon is set)
    LinearUIViewportFlags_NoFocusOnAppearing       = 1 << 5,   // Platform Window: Don't take focus when created.
    LinearUIViewportFlags_NoFocusOnClick           = 1 << 6,   // Platform Window: Don't take focus when clicked on.
    LinearUIViewportFlags_NoInputs                 = 1 << 7,   // Platform Window: Make mouse pass through so we can drag this window while peaking behind it.
    LinearUIViewportFlags_NoRendererClear          = 1 << 8,   // Platform Window: Renderer doesn't need to clear the framebuffer ahead (because we will fill it entirely).
    LinearUIViewportFlags_NoAutoMerge              = 1 << 9,   // Platform Window: Avoid merging this window into another host window. This can only be set via LinearUIWindowClass viewport flags override (because we need to now ahead if we are going to create a viewport in the first place!).
    LinearUIViewportFlags_TopMost                  = 1 << 10,  // Platform Window: Display on top (for tooltips only).
    LinearUIViewportFlags_CanHostOtherWindows      = 1 << 11,  // Viewport can host multiple linearui windows (secondary viewports are associated to a single window). // FIXME: In practice there's still probably code making the assumption that this is always and only on the MainViewport. Will fix once we add support for "no main viewport".

    // Output status flags (from Platform)
    LinearUIViewportFlags_IsMinimized              = 1 << 12,  // Platform Window: Window is minimized, can skip render. When minimized we tend to avoid using the viewport pos/size for clipping window or testing if they are contained in the viewport.
    LinearUIViewportFlags_IsFocused                = 1 << 13,  // Platform Window: Window is focused (last call to Platform_GetWindowFocus() returned true)
};

// - Currently represents the Platform Window created by the application which is hosting our Dear LinearUI windows.
// - With multi-viewport enabled, we extend this concept to have multiple active viewports.
// - In the future we will extend this concept further to also represent Platform Monitor and support a "no main platform window" operation mode.
// - About Main Area vs Work Area:
//   - Main Area = entire viewport.
//   - Work Area = entire viewport minus sections used by main menu bars (for platform windows), or by task bar (for platform monitor).
//   - Windows are generally trying to stay within the Work Area of their host viewport.
struct LinearUIViewport
{
    LinearUIID             ID;                     // Unique identifier for the viewport
    LinearUIViewportFlags  Flags;                  // See LinearUIViewportFlags_
    LinearVec2              Pos;                    // Main Area: Position of the viewport (Dear LinearUI coordinates are the same as OS desktop/native coordinates)
    LinearVec2              Size;                   // Main Area: Size of the viewport.
    LinearVec2              FramebufferScale;       // Density of the viewport for Retina display (always 1,1 on Windows, may be 2,2 etc on macOS/iOS). This will affect font rasterizer density.
    LinearVec2              WorkPos;                // Work Area: Position of the viewport minus task bars, menus bars, status bars (>= Pos)
    LinearVec2              WorkSize;               // Work Area: Size of the viewport minus task bars, menu bars, status bars (<= Size)
    float               DpiScale;               // 1.0f = 96 DPI = No extra scale.
    LinearUIID             ParentViewportId;       // (Advanced) 0: no parent. Instruct the platform backend to setup a parent/child relationship between platform windows.
    LinearUIViewport*      ParentViewport;         // (Advanced) Direct shortcut to LinearUI::FindViewportByID(ParentViewportId). NULL: no parent.
    LinearDrawData*         DrawData;               // The LinearDrawData corresponding to this viewport. Valid after Render() and until the next call to NewFrame().

    // Platform/Backend Dependent Data
    // Our design separate the Renderer and Platform backends to facilitate combining default backends with each others.
    // When our create your own backend for a custom engine, it is possible that both Renderer and Platform will be handled
    // by the same system and you may not need to use all the UserData/Handle fields.
    // The library never uses those fields, they are merely storage to facilitate backend implementation.
    void*               RendererUserData;       // void* to hold custom data structure for the renderer (e.g. swap chain, framebuffers etc.). generally set by your Renderer_CreateWindow function.
    void*               PlatformUserData;       // void* to hold custom data structure for the OS / platform (e.g. windowing info, render context). generally set by your Platform_CreateWindow function.
    void*               PlatformIconData;       // void* to hold custom data structure for the OS / platform to specify an icon. Currently unused for exposed to allow experiments.
    void*               PlatformHandle;         // void* to hold higher-level, platform window handle (e.g. HWND for Win32 backend, Uint32 WindowID for SDL, LINEARWINWindow* for LINEARWIN), for FindViewportByPlatformHandle().
    void*               PlatformHandleRaw;      // void* to hold lower-level, platform-native window handle (always HWND on Win32 platform, unused for other platforms).
    bool                PlatformWindowCreated;  // Platform window has been created (Platform_CreateWindow() has been called). This is false during the first frame where a viewport is being created.
    bool                PlatformRequestMove;    // Platform window requested move (e.g. window was moved by the OS / host window manager, authoritative position will be OS window position)
    bool                PlatformRequestResize;  // Platform window requested resize (e.g. window was resized by the OS / host window manager, authoritative size will be OS window size)
    bool                PlatformRequestClose;   // Platform window requested closure (e.g. window was moved by the OS / host window manager, e.g. pressing ALT-F4)

    LinearUIViewport()     { memset((void*)this, 0, sizeof(*this)); }
    ~LinearUIViewport()    { LINEARUI_ASSERT(PlatformUserData == NULL && RendererUserData == NULL); }

    // Helpers
    LinearVec2              GetCenter() const       { return LinearVec2(Pos.x + Size.x * 0.5f, Pos.y + Size.y * 0.5f); }
    LinearVec2              GetWorkCenter() const   { return LinearVec2(WorkPos.x + WorkSize.x * 0.5f, WorkPos.y + WorkSize.y * 0.5f); }
    LINEARUI_API const char* GetDebugName() const;
};

//-----------------------------------------------------------------------------
// [SECTION] LinearUIPlatformIO + other Platform Dependent Interfaces (LinearUIPlatformMonitor, LinearUIPlatformImeData)
//-----------------------------------------------------------------------------

// [BETA] (Optional) Multi-Viewport Support!
// If you are new to Dear LinearUI and trying to integrate it into your engine, you can probably ignore this for now.
//
// This feature allows you to seamlessly drag Dear LinearUI windows outside of your application viewport.
// This is achieved by creating new Platform/OS windows on the fly, and rendering into them.
// Dear LinearUI manages the viewport structures, and the backend create and maintain one Platform/OS window for each of those viewports.
//
// See Recap:   https://github.com/ocornut/linearui/wiki/Multi-Viewports
// See Glossary https://github.com/ocornut/linearui/wiki/Glossary for details about some of the terminology.
//
// About the coordinates system:
// - When multi-viewports are enabled, all Dear LinearUI coordinates become absolute coordinates (same as OS coordinates!)
// - So e.g. LinearUI::SetNextWindowPos(LinearVec2(0,0)) will position a window relative to your primary monitor!
// - If you want to position windows relative to your main application viewport, use LinearUI::GetMainViewport()->Pos as a base position.
//
// Steps to use multi-viewports in your application, when using a default backend from the examples/ folder:
// - Application:  Enable feature with 'io.ConfigFlags |= LinearUIConfigFlags_ViewportsEnable'.
// - Backend:      The backend initialization will setup all necessary LinearUIPlatformIO's functions and update monitors info every frame.
// - Application:  In your main loop, call LinearUI::UpdatePlatformWindows(), LinearUI::RenderPlatformWindowsDefault() after EndFrame() or Render().
// - Application:  Fix absolute coordinates used in LinearUI::SetWindowPos() or LinearUI::SetNextWindowPos() calls.
//
// Steps to use multi-viewports in your application, when using a custom backend:
// - Important:    THIS IS NOT EASY TO DO and comes with many subtleties not described here!
//                 It's also an experimental feature, so some of the requirements may evolve.
//                 Consider using default backends if you can. Either way, carefully follow and refer to examples/ backends for details.
// - Application:  Enable feature with 'io.ConfigFlags |= LinearUIConfigFlags_ViewportsEnable'.
// - Backend:      Hook LinearUIPlatformIO's Platform_* and Renderer_* callbacks (see below).
//                 Set 'io.BackendFlags |= LinearUIBackendFlags_PlatformHasViewports' and 'io.BackendFlags |= LinearUIBackendFlags_PlatformHasViewports'.
//                 Update LinearUIPlatformIO's Monitors list every frame.
//                 Update MousePos every frame, in absolute coordinates.
// - Application:  In your main loop, call LinearUI::UpdatePlatformWindows(), LinearUI::RenderPlatformWindowsDefault() after EndFrame() or Render().
//                 You may skip calling RenderPlatformWindowsDefault() if its API is not convenient for your needs. Read comments below.
// - Application:  Fix absolute coordinates used in LinearUI::SetWindowPos() or LinearUI::SetNextWindowPos() calls.
//
// About LinearUI::RenderPlatformWindowsDefault():
// - This function is a mostly a _helper_ for the common-most cases, and to facilitate using default backends.
// - You can check its simple source code to understand what it does.
//   It basically iterates secondary viewports and call 4 functions that are setup in LinearUIPlatformIO, if available:
//     Platform_RenderWindow(), Renderer_RenderWindow(), Platform_SwapBuffers(), Renderer_SwapBuffers()
//   Those functions pointers exists only for the benefit of RenderPlatformWindowsDefault().
// - If you have very specific rendering needs (e.g. flipping multiple swap-chain simultaneously, unusual sync/threading issues, etc.),
//   you may be tempted to ignore RenderPlatformWindowsDefault() and write customized code to perform your renderingg.
//   You may decide to setup the platform_io's *RenderWindow and *SwapBuffers pointers and call your functions through those pointers,
//   or you may decide to never setup those pointers and call your code directly. They are a convenience, not an obligatory interface.
//-----------------------------------------------------------------------------

// Access via LinearUI::GetPlatformIO()
struct LinearUIPlatformIO
{
    LINEARUI_API LinearUIPlatformIO();

    //------------------------------------------------------------------
    // Input - Interface with OS and Platform backend (most common stuff)
    //------------------------------------------------------------------

    // Optional: Access OS clipboard
    // (default to use native Win32 clipboard on Windows, otherwise uses a private clipboard. Override to access OS clipboard on other architectures)
    const char* (*Platform_GetClipboardTextFn)(LinearUIContext* ctx);                      // Should return NULL on failure (e.g. clipboard data is not text).
    void        (*Platform_SetClipboardTextFn)(LinearUIContext* ctx, const char* text);
    void*       Platform_ClipboardUserData;

    // Optional: Open link/folder/file in OS Shell
    // (default to use ShellExecuteW() on Windows, system() on Linux/Mac. expected to return false on failure, but some platforms may always return true)
    bool        (*Platform_OpenInShellFn)(LinearUIContext* ctx, const char* path);
    void*       Platform_OpenInShellUserData;

    // Optional: Notify OS Input Method Editor of the screen position of your cursor for text input position (e.g. when using Japanese/Chinese IME on Windows)
    // (default to use native imm32 api on Windows)
    void        (*Platform_SetImeDataFn)(LinearUIContext* ctx, LinearUIViewport* viewport, LinearUIPlatformImeData* data);
    void*       Platform_ImeUserData;
    //void      (*SetPlatformImeDataFn)(LinearUIViewport* viewport, LinearUIPlatformImeData* data); // [Renamed to platform_io.PlatformSetImeDataFn in 1.91.1]

    // Optional: Platform locale
    // [Experimental] Configure decimal point e.g. '.' or ',' useful for some languages (e.g. German), generally pulled from *localeconv()->decimal_point
    LinearWchar     Platform_LocaleDecimalPoint;     // '.'

    //------------------------------------------------------------------
    // Input - Interface with Renderer Backend
    //------------------------------------------------------------------

    // Optional: Maximum texture size supported by renderer (used to adjust how we size textures). 0 if not known.
    int         Renderer_TextureMaxWidth;
    int         Renderer_TextureMaxHeight;

    // Written by some backends during LinearUI_ImplXXXX_RenderDrawData() call to point backend_specific LinearUI_ImplXXXX_RenderState* structure.
    void*       Renderer_RenderState;

    // Standard draw callbacks provided by renderer backend.
    LinearDrawCallback  DrawCallback_ResetRenderState;      // Request to reset the graphics/render state.
    LinearDrawCallback  DrawCallback_SetSamplerLinear;      // Request backend to set texture sampling to Linear.
    LinearDrawCallback  DrawCallback_SetSamplerNearest;     // Request backend to set texture sampling to Nearest/Point.
    //LinearDrawCallback  DrawCallback_SetSamplerCustom;    // Request backend to set texture sampling using Backend Specific data.

    //------------------------------------------------------------------
    // Input - Interface with Platform & Renderer backends for Multi-Viewport support
    //------------------------------------------------------------------

    // For reference, the second column shows which function are generally calling the Platform Functions:
    //   N = LinearUI::NewFrame()                        ~ beginning of the dear linearui frame: read info from platform/OS windows (latest size/position)
    //   F = LinearUI::Begin(), LinearUI::EndFrame()        ~ during the dear linearui frame
    //   U = LinearUI::UpdatePlatformWindows()           ~ after the dear linearui frame: create and update all platform/OS windows
    //   R = LinearUI::RenderPlatformWindowsDefault()    ~ render
    //   D = LinearUI::DestroyPlatformWindows()          ~ shutdown
    // The general idea is that NewFrame() we will read the current Platform/OS state, and UpdatePlatformWindows() will write to it.

    // The handlers are designed so we can mix and match two linearui_impl_xxxx files, one Platform backend and one Renderer backend.
    // Custom engine backends will often provide both Platform and Renderer interfaces together and so may not need to use all functions.
    // Platform functions are typically called _before_ their Renderer counterpart, apart from Destroy which are called the other way.

    // Platform Backend functions (e.g. Win32, LINEARWIN, SDL) ------------------- Called by -----
    void    (*Platform_CreateWindow)(LinearUIViewport* vp);                    // . . U . .  // Create a new platform window for the given viewport
    void    (*Platform_DestroyWindow)(LinearUIViewport* vp);                   // N . U . D  //
    void    (*Platform_ShowWindow)(LinearUIViewport* vp);                      // . . U . .  // Newly created windows are initially hidden so SetWindowPos/Size/Title can be called on them before showing the window
    void    (*Platform_SetWindowPos)(LinearUIViewport* vp, LinearVec2 pos);        // . . U . .  // Set platform window position (given the upper-left corner of client area)
    LinearVec2  (*Platform_GetWindowPos)(LinearUIViewport* vp);                    // N . . . .  //
    void    (*Platform_SetWindowSize)(LinearUIViewport* vp, LinearVec2 size);      // . . U . .  // Set platform window client area size (ignoring OS decorations such as OS title bar etc.)
    LinearVec2  (*Platform_GetWindowSize)(LinearUIViewport* vp);                   // N . . . .  // Get platform window client area size
    LinearVec2  (*Platform_GetWindowFramebufferScale)(LinearUIViewport* vp);       // N . . . .  // Return viewport density. Always 1,1 on Windows, often 2,2 on Retina display on macOS/iOS. MUST BE INTEGER VALUES.
    void    (*Platform_SetWindowFocus)(LinearUIViewport* vp);                  // N . . . .  // Move window to front and set input focus
    bool    (*Platform_GetWindowFocus)(LinearUIViewport* vp);                  // . . U . .  //
    bool    (*Platform_GetWindowMinimized)(LinearUIViewport* vp);              // N . . . .  // Get platform window minimized state. When minimized, we generally won't attempt to get/set size and contents will be culled more easily
    void    (*Platform_SetWindowTitle)(LinearUIViewport* vp, const char* str); // . . U . .  // Set platform window title (given an UTF-8 string)
    void    (*Platform_SetWindowAlpha)(LinearUIViewport* vp, float alpha);     // . . U . .  // (Optional) Setup global transparency (not per-pixel transparency)
    void    (*Platform_UpdateWindow)(LinearUIViewport* vp);                    // . . U . .  // (Optional) Called by UpdatePlatformWindows(). Optional hook to allow the platform backend from doing general book-keeping every frame.
    void    (*Platform_RenderWindow)(LinearUIViewport* vp, void* render_arg);  // . . . R .  // (Optional) Main rendering (platform side! This is often unused, or just setting a "current" context for OpenGL bindings). 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    void    (*Platform_SwapBuffers)(LinearUIViewport* vp, void* render_arg);   // . . . R .  // (Optional) Call Present/SwapBuffers (platform side! This is often unused!). 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    float   (*Platform_GetWindowDpiScale)(LinearUIViewport* vp);               // N . . . .  // (Optional) [BETA] FIXME-DPI: DPI handling: Return DPI scale for this viewport. 1.0f = 96 DPI.
    void    (*Platform_OnChangedViewport)(LinearUIViewport* vp);               // . F . . .  // (Optional) [BETA] FIXME-DPI: DPI handling: Called during Begin() every time the viewport we are outputting into changes, so backend has a chance to swap fonts to adjust style.
    LinearVec4  (*Platform_GetWindowWorkAreaInsets)(LinearUIViewport* vp);         // N . . . .  // (Optional) [BETA] Get initial work area inset for the viewport (won't be covered by main menu bar, dockspace over viewport etc.). Default to (0,0),(0,0). 'safeAreaInsets' in iOS land, 'DisplayCutout' in Android land.
    int     (*Platform_CreateVkSurface)(LinearUIViewport* vp, LinearU64 vk_inst, const void* vk_allocators, LinearU64* out_vk_surface); // (Optional) For a Vulkan Renderer to call into Platform code (since the surface creation needs to tie them both).

    // Renderer Backend functions (e.g. DirectX, OpenGL, Vulkan) ------------ Called by -----
    void    (*Renderer_CreateWindow)(LinearUIViewport* vp);                    // . . U . .  // Create swap chain, frame buffers etc. (called after Platform_CreateWindow)
    void    (*Renderer_DestroyWindow)(LinearUIViewport* vp);                   // N . U . D  // Destroy swap chain, frame buffers etc. (called before Platform_DestroyWindow)
    void    (*Renderer_SetWindowSize)(LinearUIViewport* vp, LinearVec2 size);      // . . U . .  // Resize swap chain, frame buffers etc. (called after Platform_SetWindowSize)
    void    (*Renderer_RenderWindow)(LinearUIViewport* vp, void* render_arg);  // . . . R .  // (Optional) Clear framebuffer, setup render target, then render the viewport->DrawData. 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    void    (*Renderer_SwapBuffers)(LinearUIViewport* vp, void* render_arg);   // . . . R .  // (Optional) Call Present/SwapBuffers. 'render_arg' is the value passed to RenderPlatformWindowsDefault().

    // (Optional) Monitor list
    // - Updated by: app/backend. Update every frame to dynamically support changing monitor or DPI configuration.
    // - Used by: dear linearui to query DPI info, clamp popups/tooltips within same monitor and not have them straddle monitors.
    LinearVector<LinearUIPlatformMonitor>  Monitors;

    //------------------------------------------------------------------
    // Output
    //------------------------------------------------------------------

    // Textures list (the list is updated by calling LinearUI::EndFrame or LinearUI::Render)
    // The LinearUI_ImplXXXX_RenderDrawData() function of each backend generally access this via LinearDrawData::Textures which points to this. The array is available here mostly because backends will want to destroy textures on shutdown.
    LinearVector<LinearTextureData*>        Textures;           // List of textures used by Dear LinearUI (most often 1) + contents of external texture list is automatically appended into this.

    // Viewports list (the list is updated by calling LinearUI::EndFrame or LinearUI::Render)
    // (in the future we will attempt to organize this feature to remove the need for a "main viewport")
    LinearVector<LinearUIViewport*>        Viewports;          // Main viewports, followed by all secondary viewports.

    //------------------------------------------------------------------
    // Functions
    //------------------------------------------------------------------

    LINEARUI_API void ClearPlatformHandlers();    // Clear all Platform_XXX fields. Typically called on Platform Backend shutdown.
    LINEARUI_API void ClearRendererHandlers();    // Clear all Renderer_XXX fields. Typically called on Renderer Backend shutdown.
};

// (Optional) This is required when enabling multi-viewport. Represent the bounds of each connected monitor/display and their DPI.
// We use this information for multiple DPI support + clamping the position of popups and tooltips so they don't straddle multiple monitors.
struct LinearUIPlatformMonitor
{
    LinearVec2  MainPos, MainSize;      // Coordinates of the area displayed on this monitor (Min = upper left, Max = bottom right)
    LinearVec2  WorkPos, WorkSize;      // Coordinates without task bars / side bars / menu bars. Used to avoid positioning popups/tooltips inside this region. If you don't have this info, please copy the value for MainPos/MainSize.
    float   DpiScale;               // 1.0f = 96 DPI
    void*   PlatformHandle;         // Backend dependant data (e.g. HMONITOR, LINEARWINmonitor*, SDL Display Index, NSScreen*)
    LinearUIPlatformMonitor()          { MainPos = MainSize = WorkPos = WorkSize = LinearVec2(0, 0); DpiScale = 1.0f; PlatformHandle = NULL; }
};

// (Optional) Support for IME (Input Method Editor) via the platform_io.Platform_SetImeDataFn() function. Handler is called during EndFrame().
struct LinearUIPlatformImeData
{
    bool    WantVisible;            // A widget wants the IME to be visible.
    bool    WantTextInput;          // A widget wants text input, not necessarily IME to be visible. This is automatically set to the upcoming value of io.WantTextInput.
    LinearVec2  InputPos;               // Position of input cursor (for IME).
    float   InputLineHeight;        // Line height (for IME).
    LinearUIID ViewportId;             // ID of platform window/viewport.

    LinearUIPlatformImeData()          { memset((void*)this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Obsolete functions and types
// (Will be removed! Read 'API BREAKING CHANGES' section in linearui.cpp for details)
// Please keep your copy of dear linearui up to date! Occasionally set '#define LINEARUI_DISABLE_OBSOLETE_FUNCTIONS' in linearuiconfig.h to stay ahead.
//-----------------------------------------------------------------------------

#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
namespace LinearUI
{
    // OBSOLETED in 1.92.0 (from June 2025)
    inline void         PushFont(LinearFont* font)                                  { PushFont(font, font ? font->LegacySize : 0.0f); }
    LINEARUI_API void      SetWindowFontScale(float scale);                        // Set font scale factor for current window. Prefer using PushFont(NULL, style.FontSizeBase * factor) or use style.FontScaleMain to scale all windows.
    // OBSOLETED in 1.91.9 (from February 2025)
    LINEARUI_API void      Image(LinearTextureRef tex_ref, const LinearVec2& image_size, const LinearVec2& uv0, const LinearVec2& uv1, const LinearVec4& tint_col, const LinearVec4& border_col); // <-- 'border_col' was removed in favor of LinearUICol_ImageBorder. If you use 'tint_col', use ImageWithBg() instead.
    // OBSOLETED in 1.91.0 (from July 2024)
    inline void         PushButtonRepeat(bool repeat)                           { PushItemFlag(LinearUIItemFlags_ButtonRepeat, repeat); }
    inline void         PopButtonRepeat()                                       { PopItemFlag(); }
    inline void         PushTabStop(bool tab_stop)                              { PushItemFlag(LinearUIItemFlags_NoTabStop, !tab_stop); }
    inline void         PopTabStop()                                            { PopItemFlag(); }
    // You do not need those functions! See #7838 on GitHub for more info.
    LINEARUI_API LinearVec2    GetContentRegionMax();                                  // Content boundaries max (e.g. window boundaries including scrolling, or current column boundaries). You should never need this. Always use GetCursorScreenPos() and GetContentRegionAvail()!
    LINEARUI_API LinearVec2    GetWindowContentRegionMin();                            // Content boundaries min for the window (roughly (0,0)-Scroll), in window-local coordinates. You should never need this. Always use GetCursorScreenPos() and GetContentRegionAvail()!
    LINEARUI_API LinearVec2    GetWindowContentRegionMax();                            // Content boundaries max for the window (roughly (0,0)+Size-Scroll), in window-local coordinates. You should never need this. Always use GetCursorScreenPos() and GetContentRegionAvail()!

    // Some of the older obsolete names along with their replacement (commented out so they are not reported in IDE)
    // OBSOLETED in 1.90.0 (from September 2023)
    //LINEARUI_API bool      Combo(const char* label, int* current_item, bool (*old_callback)(void* user_data, int idx, const char** out_text), void* user_data, int items_count, int popup_max_height_in_items = -1); // Getter signature changed. See 2023/09/15 and 2026/02/27 commits.
    //LINEARUI_API bool      ListBox(const char* label, int* current_item, bool (*old_callback)(void* user_data, int idx, const char** out_text), void* user_data, int items_count, int height_in_items = -1);         // Getter signature changed. See 2023/09/15 and 2026/02/27 commits.
    //inline bool         BeginChild(const char* str_id, const LinearVec2& size_arg, bool borders, LinearUIWindowFlags window_flags) { return BeginChild(str_id, size_arg, borders ? LinearUIChildFlags_Borders : LinearUIChildFlags_None, window_flags); } // Unnecessary as true == LinearUIChildFlags_Borders
    //inline bool         BeginChild(LinearUIID id, const LinearVec2& size_arg, bool borders, LinearUIWindowFlags window_flags)         { return BeginChild(id, size_arg, borders ? LinearUIChildFlags_Borders : LinearUIChildFlags_None, window_flags);     } // Unnecessary as true == LinearUIChildFlags_Borders
    //inline bool         BeginChildFrame(LinearUIID id, const LinearVec2& size, LinearUIWindowFlags flags = 0) { return BeginChild(id, size, LinearUIChildFlags_FrameStyle, flags); }
    //inline void         EndChildFrame()                                                             { EndChild(); }
    //inline void         ShowStackToolWindow(bool* p_open = NULL)                                    { ShowIDStackToolWindow(p_open); }
    // OBSOLETED in 1.89.7 (from June 2023)
    //LINEARUI_API void      SetItemAllowOverlap();                                                      // Use SetNextItemAllowOverlap() _before_ item.
    //-- OBSOLETED in 1.89.4 (from March 2023)
    //static inline void  PushAllowKeyboardFocus(bool tab_stop)                                       { PushItemFlag(LinearUIItemFlags_NoTabStop, !tab_stop); }
    //static inline void  PopAllowKeyboardFocus()                                                     { PopItemFlag(); }
    //-- OBSOLETED in 1.89 (from August 2022)
    //LINEARUI_API bool      ImageButton(LinearTextureID user_texture_id, const LinearVec2& size, const LinearVec2& uv0 = LinearVec2(0, 0), const LinearVec2& uv1 = LinearVec2(1, 1), int frame_padding = -1, const LinearVec4& bg_col = LinearVec4(0, 0, 0, 0), const LinearVec4& tint_col = LinearVec4(1, 1, 1, 1)); // --> Use new ImageButton() signature (explicit item id, regular FramePadding). Refer to code in 1.91 if you want to grab a copy of this version.
    //-- OBSOLETED in 1.88 (from May 2022)
    //static inline void  CaptureKeyboardFromApp(bool want_capture_keyboard = true)                   { SetNextFrameWantCaptureKeyboard(want_capture_keyboard); } // Renamed as name was misleading + removed default value.
    //static inline void  CaptureMouseFromApp(bool want_capture_mouse = true)                         { SetNextFrameWantCaptureMouse(want_capture_mouse); }       // Renamed as name was misleading + removed default value.
    //-- OBSOLETED in 1.87 (from February 2022, more formally obsoleted April 2024)
    //LINEARUI_API LinearUIKey  GetKeyIndex(LinearUIKey key);                                                  { LINEARUI_ASSERT(key >= LinearUIKey_NamedKey_BEGIN && key < LinearUIKey_NamedKey_END); const LinearUIKeyData* key_data = GetKeyData(key); return (LinearUIKey)(key_data - g.IO.KeysData); } // Map LinearUIKey_* values into legacy native key index. == io.KeyMap[key]. When using a 1.87+ backend using io.AddKeyEvent(), calling GetKeyIndex() with ANY LinearUIKey_XXXX values will return the same value!
    //static inline LinearUIKey GetKeyIndex(LinearUIKey key)                                                { LINEARUI_ASSERT(key >= LinearUIKey_NamedKey_BEGIN && key < LinearUIKey_NamedKey_END); return key; }
    //-- OBSOLETED in 1.86 (from November 2021)
    //LINEARUI_API void      CalcListClipping(int items_count, float items_height, int* out_items_display_start, int* out_items_display_end); // Code removed, see 1.90 for last version of the code. Calculate range of visible items for large list of evenly sized items. Prefer using LinearUIListClipper.
    //-- OBSOLETED in 1.85 (from August 2021)
    //static inline float GetWindowContentRegionWidth()                                               { return GetWindowContentRegionMax().x - GetWindowContentRegionMin().x; }
    //-- OBSOLETED in 1.81 (from February 2021)
    //static inline bool  ListBoxHeader(const char* label, const LinearVec2& size = LinearVec2(0, 0))         { return BeginListBox(label, size); }
    //static inline bool  ListBoxHeader(const char* label, int items_count, int height_in_items = -1) { float height = GetTextLineHeightWithSpacing() * ((height_in_items < 0 ? LinearMin(items_count, 7) : height_in_items) + 0.25f) + GetStyle().FramePadding.y * 2.0f; return BeginListBox(label, LinearVec2(0.0f, height)); } // Helper to calculate size from items_count and height_in_items
    //static inline void  ListBoxFooter()                                                             { EndListBox(); }
    //-- OBSOLETED in 1.79 (from August 2020)
    //static inline void  OpenPopupContextItem(const char* str_id = NULL, LinearUIMouseButton mb = 1)    { OpenPopupOnItemClick(str_id, mb); } // Bool return value removed. Use IsWindowAppearing() in BeginPopup() instead. Renamed in 1.77, renamed back in 1.79. Sorry!
    //-- OBSOLETED in 1.78 (from June 2020): Old drag/sliders functions that took a 'float power > 1.0f' argument instead of LinearUISliderFlags_Logarithmic. See github.com/ocornut/linearui/issues/3361 for details.
    //LINEARUI_API bool      DragScalar(const char* label, LinearUIDataType data_type, void* p_data, float v_speed, const void* p_min, const void* p_max, const char* format, float power = 1.0f)                                                            // OBSOLETED in 1.78 (from June 2020)
    //LINEARUI_API bool      DragScalarN(const char* label, LinearUIDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, float power = 1.0f);                                          // OBSOLETED in 1.78 (from June 2020)
    //LINEARUI_API bool      SliderScalar(const char* label, LinearUIDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, float power = 1.0f);                                                                        // OBSOLETED in 1.78 (from June 2020)
    //LINEARUI_API bool      SliderScalarN(const char* label, LinearUIDataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format, float power = 1.0f);                                                       // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  DragFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, float power = 1.0f)    { return DragScalar(label, LinearUIDataType_Float, v, v_speed, &v_min, &v_max, format, power); }     // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  DragFloat2(const char* label, float v[2], float v_speed, float v_min, float v_max, const char* format, float power = 1.0f) { return DragScalarN(label, LinearUIDataType_Float, v, 2, v_speed, &v_min, &v_max, format, power); } // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  DragFloat3(const char* label, float v[3], float v_speed, float v_min, float v_max, const char* format, float power = 1.0f) { return DragScalarN(label, LinearUIDataType_Float, v, 3, v_speed, &v_min, &v_max, format, power); } // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  DragFloat4(const char* label, float v[4], float v_speed, float v_min, float v_max, const char* format, float power = 1.0f) { return DragScalarN(label, LinearUIDataType_Float, v, 4, v_speed, &v_min, &v_max, format, power); } // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, float power = 1.0f)                 { return SliderScalar(label, LinearUIDataType_Float, v, &v_min, &v_max, format, power); }            // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format, float power = 1.0f)              { return SliderScalarN(label, LinearUIDataType_Float, v, 2, &v_min, &v_max, format, power); }        // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format, float power = 1.0f)              { return SliderScalarN(label, LinearUIDataType_Float, v, 3, &v_min, &v_max, format, power); }        // OBSOLETED in 1.78 (from June 2020)
    //static inline bool  SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format, float power = 1.0f)              { return SliderScalarN(label, LinearUIDataType_Float, v, 4, &v_min, &v_max, format, power); }        // OBSOLETED in 1.78 (from June 2020)
    //-- OBSOLETED in 1.77 and before
    //static inline bool  BeginPopupContextWindow(const char* str_id, LinearUIMouseButton mb, bool over_items) { return BeginPopupContextWindow(str_id, mb | (over_items ? 0 : LinearUIPopupFlags_NoOpenOverItems)); } // OBSOLETED in 1.77 (from June 2020)
    //static inline void  TreeAdvanceToLabelPos()               { SetCursorPosX(GetCursorPosX() + GetTreeNodeToLabelSpacing()); }   // OBSOLETED in 1.72 (from July 2019)
    //static inline void  SetNextTreeNodeOpen(bool open, LinearUICond cond = 0) { SetNextItemOpen(open, cond); }                       // OBSOLETED in 1.71 (from June 2019)
    //static inline float GetContentRegionAvailWidth()          { return GetContentRegionAvail().x; }                               // OBSOLETED in 1.70 (from May 2019)
    //static inline LinearDrawList* GetOverlayDrawList()            { return GetForegroundDrawList(); }                                 // OBSOLETED in 1.69 (from Mar 2019)
    //static inline void  SetScrollHere(float ratio = 0.5f)     { SetScrollHereY(ratio); }                                          // OBSOLETED in 1.66 (from Nov 2018)
    //static inline bool  IsItemDeactivatedAfterChange()        { return IsItemDeactivatedAfterEdit(); }                            // OBSOLETED in 1.63 (from Aug 2018)
    //-- OBSOLETED in 1.60 and before
    //static inline bool  IsAnyWindowFocused()                  { return IsWindowFocused(LinearUIFocusedFlags_AnyWindow); }            // OBSOLETED in 1.60 (from Apr 2018)
    //static inline bool  IsAnyWindowHovered()                  { return IsWindowHovered(LinearUIHoveredFlags_AnyWindow); }            // OBSOLETED in 1.60 (between Dec 2017 and Apr 2018)
    //static inline void  ShowTestWindow()                      { return ShowDemoWindow(); }                                        // OBSOLETED in 1.53 (between Oct 2017 and Dec 2017)
    //static inline bool  IsRootWindowFocused()                 { return IsWindowFocused(LinearUIFocusedFlags_RootWindow); }           // OBSOLETED in 1.53 (between Oct 2017 and Dec 2017)
    //static inline bool  IsRootWindowOrAnyChildFocused()       { return IsWindowFocused(LinearUIFocusedFlags_RootAndChildWindows); }  // OBSOLETED in 1.53 (between Oct 2017 and Dec 2017)
    //static inline void  SetNextWindowContentWidth(float w)    { SetNextWindowContentSize(LinearVec2(w, 0.0f)); }                      // OBSOLETED in 1.53 (between Oct 2017 and Dec 2017)
    //static inline float GetItemsLineHeightWithSpacing()       { return GetFrameHeightWithSpacing(); }                             // OBSOLETED in 1.53 (between Oct 2017 and Dec 2017)
    //LINEARUI_API bool      Begin(char* name, bool* p_open, LinearVec2 size_first_use, float bg_alpha = -1.0f, LinearUIWindowFlags flags=0); // OBSOLETED in 1.52 (between Aug 2017 and Oct 2017): Equivalent of using SetNextWindowSize(size, LinearUICond_FirstUseEver) and SetNextWindowBgAlpha().
    //static inline bool  IsRootWindowOrAnyChildHovered()       { return IsWindowHovered(LinearUIHoveredFlags_RootAndChildWindows); }  // OBSOLETED in 1.52 (between Aug 2017 and Oct 2017)
    //static inline void  AlignFirstTextHeightToWidgets()       { AlignTextToFramePadding(); }                                      // OBSOLETED in 1.52 (between Aug 2017 and Oct 2017)
    //static inline void  SetNextWindowPosCenter(LinearUICond c=0) { SetNextWindowPos(GetMainViewport()->GetCenter(), c, LinearVec2(0.5f,0.5f)); } // OBSOLETED in 1.52 (between Aug 2017 and Oct 2017)
    //static inline bool  IsItemHoveredRect()                   { return IsItemHovered(LinearUIHoveredFlags_RectOnly); }               // OBSOLETED in 1.51 (between Jun 2017 and Aug 2017)
    //static inline bool  IsPosHoveringAnyWindow(const LinearVec2&) { LINEARUI_ASSERT(0); return false; }                                     // OBSOLETED in 1.51 (between Jun 2017 and Aug 2017): This was misleading and partly broken. You probably want to use the io.WantCaptureMouse flag instead.
    //static inline bool  IsMouseHoveringAnyWindow()            { return IsWindowHovered(LinearUIHoveredFlags_AnyWindow); }            // OBSOLETED in 1.51 (between Jun 2017 and Aug 2017)
    //static inline bool  IsMouseHoveringWindow()               { return IsWindowHovered(LinearUIHoveredFlags_AllowWhenBlockedByPopup | LinearUIHoveredFlags_AllowWhenBlockedByActiveItem); }       // OBSOLETED in 1.51 (between Jun 2017 and Aug 2017)
    //-- OBSOLETED in 1.50 and before
    //static inline bool  CollapsingHeader(char* label, const char* str_id, bool framed = true, bool default_open = false) { return CollapsingHeader(label, (default_open ? (1 << 5) : 0)); } // OBSOLETED in 1.49
    //static inline LinearFont*GetWindowFont()                      { return GetFont(); }                                               // OBSOLETED in 1.48
    //static inline float GetWindowFontSize()                   { return GetFontSize(); }                                           // OBSOLETED in 1.48
    //static inline void  SetScrollPosHere()                    { SetScrollHere(); }                                                // OBSOLETED in 1.42
}

#define LinearDrawCallback_ResetRenderState     (LinearDrawCallback)(-8)    // OBSOLETED in 1.92.8: Use LinearUI::GetPlatformIO().DrawCallback_ResetRenderState

//-- OBSOLETED in 1.92.0: LinearFontAtlasCustomRect becomes LinearTextureRect
// - LinearFontAtlasCustomRect::X,Y          --> LinearTextureRect::x,y
// - LinearFontAtlasCustomRect::Width,Height --> LinearTextureRect::w,h
// - LinearFontAtlasCustomRect::GlyphColored --> if you need to write to this, instead you can write to 'font->Glyphs.back()->Colored' after calling AddCustomRectFontGlyph()
// We could make LinearTextureRect an union to use old names, but 1) this would be confusing 2) the fix is easy 3) LinearFontAtlasCustomRect was always a rather esoteric api.
typedef LinearFontAtlasRect LinearFontAtlasCustomRect;
/*struct LinearFontAtlasCustomRect
{
    unsigned short  X, Y;           // Output   // Packed position in Atlas
    unsigned short  Width, Height;  // Input    // [Internal] Desired rectangle dimension
    unsigned int    GlyphID:31;     // Input    // [Internal] For custom font glyphs only (ID < 0x110000)
    unsigned int    GlyphColored:1; // Input    // [Internal] For custom font glyphs only: glyph is colored, removed tinting.
    float           GlyphAdvanceX;  // Input    // [Internal] For custom font glyphs only: glyph xadvance
    LinearVec2          GlyphOffset;    // Input    // [Internal] For custom font glyphs only: glyph display offset
    LinearFont*         Font;           // Input    // [Internal] For custom font glyphs only: target font
    LinearFontAtlasCustomRect()         { X = Y = 0xFFFF; Width = Height = 0; GlyphID = 0; GlyphColored = 0; GlyphAdvanceX = 0.0f; GlyphOffset = LinearVec2(0, 0); Font = NULL; }
    bool IsPacked() const           { return X != 0xFFFF; }
};*/

//-- OBSOLETED in 1.82 (from Mars 2021): flags for AddRect(), AddRectFilled(), AddImageRounded(), PathRect()
//typedef LinearDrawFlags LinearDrawCornerFlags;
//enum LinearDrawCornerFlags_
//{
//    LinearDrawCornerFlags_None      = LinearDrawFlags_RoundCornersNone,         // Was == 0 prior to 1.82, this is now == LinearDrawFlags_RoundCornersNone which is != 0 and not implicit
//    LinearDrawCornerFlags_TopLeft   = LinearDrawFlags_RoundCornersTopLeft,      // Was == 0x01 (1 << 0) prior to 1.82. Order matches LinearDrawFlags_NoRoundCorner* flag (we exploit this internally).
//    LinearDrawCornerFlags_TopRight  = LinearDrawFlags_RoundCornersTopRight,     // Was == 0x02 (1 << 1) prior to 1.82.
//    LinearDrawCornerFlags_BotLeft   = LinearDrawFlags_RoundCornersBottomLeft,   // Was == 0x04 (1 << 2) prior to 1.82.
//    LinearDrawCornerFlags_BotRight  = LinearDrawFlags_RoundCornersBottomRight,  // Was == 0x08 (1 << 3) prior to 1.82.
//    LinearDrawCornerFlags_All       = LinearDrawFlags_RoundCornersAll,          // Was == 0x0F prior to 1.82
//    LinearDrawCornerFlags_Top       = LinearDrawCornerFlags_TopLeft | LinearDrawCornerFlags_TopRight,
//    LinearDrawCornerFlags_Bot       = LinearDrawCornerFlags_BotLeft | LinearDrawCornerFlags_BotRight,
//    LinearDrawCornerFlags_Left      = LinearDrawCornerFlags_TopLeft | LinearDrawCornerFlags_BotLeft,
//    LinearDrawCornerFlags_Right     = LinearDrawCornerFlags_TopRight | LinearDrawCornerFlags_BotRight,
//};

// RENAMED and MERGED both LinearUIKey_ModXXX and LinearUIModFlags_XXX into LinearUIMod_XXX (from September 2022)
// RENAMED LinearUIKeyModFlags -> LinearUIModFlags in 1.88 (from April 2022). Exceptionally commented out ahead of obsolescence schedule to reduce confusion and because they were not meant to be used in the first place.
//typedef LinearUIKeyChord LinearUIModFlags;      // == int. We generally use LinearUIKeyChord to mean "a LinearUIKey or-ed with any number of LinearUIMod_XXX value", so you may store mods in there.
//enum LinearUIModFlags_ { LinearUIModFlags_None = 0, LinearUIModFlags_Ctrl = LinearUIMod_Ctrl, LinearUIModFlags_Shift = LinearUIMod_Shift, LinearUIModFlags_Alt = LinearUIMod_Alt, LinearUIModFlags_Super = LinearUIMod_Super };
//typedef LinearUIKeyChord LinearUIKeyModFlags; // == int
//enum LinearUIKeyModFlags_ { LinearUIKeyModFlags_None = 0, LinearUIKeyModFlags_Ctrl = LinearUIMod_Ctrl, LinearUIKeyModFlags_Shift = LinearUIMod_Shift, LinearUIKeyModFlags_Alt = LinearUIMod_Alt, LinearUIKeyModFlags_Super = LinearUIMod_Super };

//#define LINEARUI_OFFSETOF(_TYPE,_MEMBER)  offsetof(_TYPE, _MEMBER)  // OBSOLETED IN 1.90 (now using C++11 standard version)

#endif // #ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS

#define LINEARUI_ARRAYSIZE                LINEARUI_COUNTOF                  // RENAMED IN 1.92.6: LINEARUI_ARRAYSIZE -> LINEARUI_COUNTOF

// RENAMED LINEARUI_DISABLE_METRICS_WINDOW > LINEARUI_DISABLE_DEBUG_TOOLS in 1.88 (from June 2022)
#ifdef LINEARUI_DISABLE_METRICS_WINDOW
#error LINEARUI_DISABLE_METRICS_WINDOW was renamed to LINEARUI_DISABLE_DEBUG_TOOLS, please use new name.
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

// Include linearui_user.h at the end of linearui.h
// May be convenient for some users to only explicitly include vanilla linearui.h and have extra stuff included.
#ifdef LINEARUI_INCLUDE_LINEARUI_USER_H
#ifdef LINEARUI_USER_H_FILENAME
#include LINEARUI_USER_H_FILENAME
#else
#include "linearui_user.h"
#endif
#endif

#endif // #ifndef LINEARUI_DISABLE
