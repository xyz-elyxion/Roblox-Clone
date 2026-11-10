// dear rblxui, v1.92.9 WIP
// (internal structures/api)

// You may use this file to debug, understand or extend Dear RblxUI features but we don't provide any guarantee of forward compatibility.

/*

Index of this file:

// [SECTION] Header mess
// [SECTION] Forward declarations
// [SECTION] Context pointer
// [SECTION] RBLXSTB libraries includes
// [SECTION] Macros
// [SECTION] Generic helpers
// [SECTION] RblxDrawList support
// [SECTION] Style support
// [SECTION] Data types support
// [SECTION] Widgets support: flags, enums, data structures
// [SECTION] Popup support
// [SECTION] Inputs support
// [SECTION] Clipper support
// [SECTION] Navigation support
// [SECTION] Typing-select support
// [SECTION] Columns support
// [SECTION] Box-select support
// [SECTION] Multi-select support
// [SECTION] Docking support
// [SECTION] Viewport support
// [SECTION] Settings support
// [SECTION] Localization support
// [SECTION] Error handling, State recovery support
// [SECTION] Metrics, Debug tools
// [SECTION] Generic context hooks
// [SECTION] RblxUIContext (main rblxui context)
// [SECTION] RblxUIWindowTempData, RblxUIWindow
// [SECTION] Tab bar, Tab item support
// [SECTION] Table support
// [SECTION] RblxUI internal API
// [SECTION] RblxFontLoader
// [SECTION] RblxFontAtlas internal API
// [SECTION] Test Engine specific hooks (rblxui_test_engine)

*/

#pragma once
#ifndef RBLXUI_DISABLE

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

#ifndef RBLXUI_VERSION
#include "rblxui.h"
#endif

#include <stdio.h>      // FILE*, sscanf
#include <stdlib.h>     // NULL, malloc, free, qsort, atoi, atof
#include <math.h>       // sqrtf, fabsf, fmodf, powf, floorf, ceilf, cosf, sinf
#include <limits.h>     // INT_MIN, INT_MAX

// Enable SSE intrinsics if available
#if (defined __SSE__ || defined __x86_64__ || defined _M_X64 || (defined(_M_IX86_FP) && (_M_IX86_FP >= 1))) && !defined(RBLXUI_DISABLE_SSE) && !defined(_M_ARM64) && !defined(_M_ARM64EC)
#define RBLXUI_ENABLE_SSE
#include <immintrin.h>
#if (defined __AVX__ || defined __SSE4_2__)
#define RBLXUI_ENABLE_SSE4_2
#include <nmmintrin.h>
#endif
#endif
// Emscripten has partial SSE 4.2 support where _mm_crc32_u32 is not available. See https://emscripten.org/docs/porting/simd.html#id11 and #8213
#if defined(RBLXUI_ENABLE_SSE4_2) && !defined(RBLXUI_USE_LEGACY_CRC32_ADLER) && !defined(__EMSCRIPTEN__)
#define RBLXUI_ENABLE_SSE4_2_CRC
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4251)     // class 'xxx' needs to have dll-interface to be used by clients of struct 'xxx' // when RBLXUI_API is set to__declspec(dllexport)
#pragma warning (disable: 26495)    // [Static Analyzer] Variable 'XXX' is uninitialized. Always initialize a member variable (type.6).
#pragma warning (disable: 26812)    // [Static Analyzer] The enum type 'xxx' is unscoped. Prefer 'enum class' over 'enum' (Enum.3).
#if defined(_MSC_VER) && _MSC_VER >= 1922 // MSVC 2019 16.2 or later
#pragma warning (disable: 5054)     // operator '|': deprecated between enumerations of different types
#endif
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wfloat-equal"                    // warning: comparing floating point with == or != is unsafe // storing and comparing against same constants ok, for RblxFloor()
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant
#pragma clang diagnostic ignored "-Wdouble-promotion"               // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#pragma clang diagnostic ignored "-Wmissing-noreturn"               // warning: function 'xxx' could be declared with attribute 'noreturn'
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"// warning: bitwise operation between different enumeration types ('XXXFlags_' and 'XXXFlagsPrivate_') is deprecated
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"            // warning: 'xxx' is an unsafe pointer used for buffer access
#pragma clang diagnostic ignored "-Wnontrivial-memaccess"           // warning: first argument in call to 'memset' is a pointer to non-trivially copyable type
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"                          // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wfloat-equal"                      // warning: comparing floating-point with '==' or '!=' is unsafe
#pragma GCC diagnostic ignored "-Wclass-memaccess"                  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"  // warning: bitwise operation between different enumeration types ('XXXFlags_' and 'XXXFlagsPrivate_') is deprecated
#pragma GCC diagnostic ignored "-Wsign-conversion"                  // warning: conversion to 'xxxx' from 'xxxx' may change the sign of the result
#endif

// In 1.89.4, we moved the implementation of "courtesy maths operators" from rblxui_internal.h in rblxui.h
// As they are frequently requested, we do not want to encourage to many people using rblxui_internal.h
#if defined(RBLXUI_DEFINE_MATH_OPERATORS) && !defined(RBLXUI_DEFINE_MATH_OPERATORS_IMPLEMENTED)
#error Please '#define RBLXUI_DEFINE_MATH_OPERATORS' _BEFORE_ including rblxui.h!
#endif

// Legacy defines
#ifdef RBLXUI_DISABLE_FORMAT_STRING_FUNCTIONS            // Renamed in 1.74
#error Use RBLXUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
#endif
#ifdef RBLXUI_DISABLE_MATH_FUNCTIONS                     // Renamed in 1.74
#error Use RBLXUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#endif

// Enable rblxstb_truetype by default unless FreeType is enabled.
// You can compile with both by defining both RBLXUI_ENABLE_FREETYPE and RBLXUI_ENABLE_STB_TRUETYPE together.
#ifndef RBLXUI_ENABLE_FREETYPE
#define RBLXUI_ENABLE_STB_TRUETYPE
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations
//-----------------------------------------------------------------------------

// Utilities
// (other types which are not forwarded declared are: RblxBitArray<>, RblxSpan<>, RblxSpanAllocator<>, RblxStableVector<>, RblxPool<>, RblxChunkStream<>)
struct RblxBitVector;                 // Store 1-bit per value
struct RblxRect;                      // An axis-aligned rectangle (2 points)
struct RblxUITextIndex;              // Maintain a line index for a text buffer.

// RblxDrawList/RblxFontAtlas
struct RblxDrawDataBuilder;           // Helper to build a RblxDrawData instance
struct RblxDrawListSharedData;        // Data shared between all RblxDrawList instances
struct RblxFontAtlasBuilder;          // Internal storage for incrementally packing and building a RblxFontAtlas
struct RblxFontAtlasPostProcessData;  // Data available to potential texture post-processing functions
struct RblxFontAtlasRectEntry;        // Packed rectangle lookup entry

// RblxUI
struct RblxUIBoxSelectState;         // Box-selection state (currently used by multi-selection, could potentially be used by others)
struct RblxUIColorMod;               // Stacked color modifier, backup of modified data so we can restore it
struct RblxUIContext;                // Main Dear RblxUI context
struct RblxUIContextHook;            // Hook for extensions like RblxUITestEngine
struct RblxUIDataTypeInfo;           // Type information associated to a RblxUIDataType enum
struct RblxUIDeactivatedItemData;    // Data for IsItemDeactivated()/IsItemDeactivatedAfterEdit() function.
struct RblxUIDockContext;            // Docking system context
struct RblxUIDockRequest;            // Docking system dock/undock queued request
struct RblxUIDockNode;               // Docking system node (hold a list of Windows OR two child dock nodes)
struct RblxUIDockNodeSettings;       // Storage for a dock node in .ini file (we preserve those even if the associated dock node isn't active during the session)
struct RblxUIErrorRecoveryState;     // Storage of stack sizes for error handling and recovery
struct RblxUIGroupData;              // Stacked storage data for BeginGroup()/EndGroup()
struct RblxUIInputTextState;         // Internal state of the currently focused/edited text input box
struct RblxUIInputTextDeactivateData;// Short term storage to backup text of a deactivating InputText() while another is stealing active id
struct RblxUILastItemData;           // Status storage for last submitted items
struct RblxUILocEntry;               // A localization entry.
struct RblxUIMenuColumns;            // Simple column measurement, currently used for MenuItem() only
struct RblxUIMultiSelectState;       // Multi-selection persistent state (for focused selection).
struct RblxUIMultiSelectTempData;    // Multi-selection temporary state (while traversing).
struct RblxUINavItemData;            // Result of a keyboard/gamepad directional navigation move query result
struct RblxUIMetricsConfig;          // Storage for ShowMetricsWindow() and DebugNodeXXX() functions
struct RblxUINextWindowData;         // Storage for SetNextWindow** functions
struct RblxUINextItemData;           // Storage for SetNextItem** functions
struct RblxUIOldColumnData;          // Storage data for a single column for legacy Columns() api
struct RblxUIOldColumns;             // Storage data for a columns set for legacy Columns() api
struct RblxUIPopupData;              // Storage for current popup stack
struct RblxUISettingsHandler;        // Storage for one type registered in the .ini file
struct RblxUIStyleMod;               // Stacked style modifier, backup of modified data so we can restore it
struct RblxUIStyleVarInfo;           // Style variable information (e.g. to access style variables from an enum)
struct RblxUITabBar;                 // Storage for a tab bar
struct RblxUITabItem;                // Storage for a tab item (within a tab bar)
struct RblxUITable;                  // Storage for a table
struct RblxUITableHeaderData;        // Storage for TableAngledHeadersRow()
struct RblxUITableColumn;            // Storage for one column of a table
struct RblxUITableInstanceData;      // Storage for one instance of a same table
struct RblxUITableTempData;          // Temporary storage for one table (one per table in the stack), shared between tables.
struct RblxUITableSettings;          // Storage for a table .ini settings
struct RblxUITableColumnsSettings;   // Storage for a column .ini settings
struct RblxUITreeNodeStackData;      // Temporary storage for TreeNode().
struct RblxUITypingSelectState;      // Storage for GetTypingSelectRequest()
struct RblxUITypingSelectRequest;    // Storage for GetTypingSelectRequest() (aimed to be public)
struct RblxUIWindow;                 // Storage for one window
struct RblxUIWindowDockStyle;        // Storage for window-style data which needs to be stored for docking purpose
struct RblxUIWindowTempData;         // Temporary storage for one window (that's the data which in theory we could ditch at the end of the frame, in practice we currently keep it for each window)
struct RblxUIWindowSettings;         // Storage for a window .ini settings (we keep one of those even if the actual window wasn't instanced during this session)

// Enumerations
// Use your programming IDE "Go to definition" facility on the names of the center columns to find the actual flags/enum lists.
enum RblxUILocKey : int;                 // -> enum RblxUILocKey              // Enum: a localization entry for translation.
typedef int RblxUIDataAuthority;         // -> enum RblxUIDataAuthority_      // Enum: for storing the source authority (dock node vs window) of a field
typedef int RblxUILayoutType;            // -> enum RblxUILayoutType_         // Enum: Horizontal or vertical

// Flags
typedef int RblxDrawTextFlags;            // -> enum RblxDrawTextFlags_         // Flags: for RblxTextCalcWordWrapPositionEx()
typedef int RblxUIActivateFlags;         // -> enum RblxUIActivateFlags_      // Flags: for navigation/focus function (will be for ActivateItem() later)
typedef int RblxUIDebugLogFlags;         // -> enum RblxUIDebugLogFlags_      // Flags: for ShowDebugLogWindow(), g.DebugLogFlags
typedef int RblxUIFocusRequestFlags;     // -> enum RblxUIFocusRequestFlags_  // Flags: for FocusWindow()
typedef int RblxUIItemStatusFlags;       // -> enum RblxUIItemStatusFlags_    // Flags: for g.LastItemData.StatusFlags
typedef int RblxUIOldColumnFlags;        // -> enum RblxUIOldColumnFlags_     // Flags: for BeginColumns()
typedef int RblxUILogFlags;              // -> enum RblxUILogFlags_           // Flags: for LogBegin() text capturing function
typedef int RblxUINavRenderCursorFlags;  // -> enum RblxUINavRenderCursorFlags_//Flags: for RenderNavCursor()
typedef int RblxUINavMoveFlags;          // -> enum RblxUINavMoveFlags_       // Flags: for navigation requests
typedef int RblxUINextItemDataFlags;     // -> enum RblxUINextItemDataFlags_  // Flags: for SetNextItemXXX() functions
typedef int RblxUINextWindowDataFlags;   // -> enum RblxUINextWindowDataFlags_// Flags: for SetNextWindowXXX() functions
typedef int RblxUIScrollFlags;           // -> enum RblxUIScrollFlags_        // Flags: for ScrollToItem() and navigation requests
typedef int RblxUISeparatorFlags;        // -> enum RblxUISeparatorFlags_     // Flags: for SeparatorEx()
typedef int RblxUITextFlags;             // -> enum RblxUITextFlags_          // Flags: for TextEx()
typedef int RblxUITooltipFlags;          // -> enum RblxUITooltipFlags_       // Flags: for BeginTooltipEx()
typedef int RblxUITypingSelectFlags;     // -> enum RblxUITypingSelectFlags_  // Flags: for GetTypingSelectRequest()
typedef int RblxUIWindowBgClickFlags;    // -> enum RblxUIWindowBgClickFlags_ // Flags: for overriding behavior of clicking on window background/void.
typedef int RblxUIWindowRefreshFlags;    // -> enum RblxUIWindowRefreshFlags_ // Flags: for SetNextWindowRefreshPolicy()

// Table column indexing
typedef RblxS16 RblxUITableColumnIdx;
typedef RblxU16 RblxUITableDrawChannelIdx;

//-----------------------------------------------------------------------------
// [SECTION] Context pointer
// See implementation of this variable in rblxui.cpp for comments and details.
//-----------------------------------------------------------------------------

#ifndef GRblxUI
extern RBLXUI_API RblxUIContext* GRblxUI;  // Current implicit context pointer
#endif

//-----------------------------------------------------------------------------
// [SECTION] Macros
//-----------------------------------------------------------------------------

// Internal Drag and Drop payload types. String starting with '_' are reserved for Dear RblxUI.
#define RBLXUI_PAYLOAD_TYPE_WINDOW       "_IMWINDOW"     // Payload == RblxUIWindow*

// Debug Printing Into TTY
// (since RBLXUI_VERSION_NUM >= 18729: RBLXUI_DEBUG_LOG was reworked into RBLXUI_DEBUG_PRINTF (and removed framecount from it). If you were using a #define RBLXUI_DEBUG_LOG please rename)
#ifndef RBLXUI_DEBUG_PRINTF
#ifndef RBLXUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
#define RBLXUI_DEBUG_PRINTF(_FMT,...)    printf(_FMT, __VA_ARGS__)
#else
#define RBLXUI_DEBUG_PRINTF(_FMT,...)    ((void)0)
#endif
#endif

// Debug Logging for ShowDebugLogWindow(). This is designed for relatively rare events so please don't spam.
#define RBLXUI_DEBUG_LOG_ERROR(...)      do { if (g.DebugLogFlags & RblxUIDebugLogFlags_EventError)       RBLXUI_DEBUG_LOG(__VA_ARGS__); else g.DebugLogSkippedErrors++; } while (0)
#define RBLXUI_DEBUG_LOG_ACTIVEID(...)   do { if (g.DebugLogFlags & RblxUIDebugLogFlags_EventActiveId)    RBLXUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define RBLXUI_DEBUG_LOG_FOCUS(...)      do { if (g.DebugLogFlags & RblxUIDebugLogFlags_EventFocus)       RBLXUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define RBLXUI_DEBUG_LOG_POPUP(...)      do { if (g.DebugLogFlags & RblxUIDebugLogFlags_EventPopup)       RBLXUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define RBLXUI_DEBUG_LOG_NAV(...)        do { if (g.DebugLogFlags & RblxUIDebugLogFlags_EventNav)         RBLXUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define RBLXUI_DEBUG_LOG_SELECTION(...)  do { if (g.DebugLogFlags & RblxUIDebugLogFlags_EventSelection)   RBLXUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define RBLXUI_DEBUG_LOG_CLIPPER(...)    do { if (g.DebugLogFlags & RblxUIDebugLogFlags_EventClipper)     RBLXUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define RBLXUI_DEBUG_LOG_IO(...)         do { if (g.DebugLogFlags & RblxUIDebugLogFlags_EventIO)          RBLXUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define RBLXUI_DEBUG_LOG_FONT(...)       do { RblxUIContext* g2 = GRblxUI; if (g2 && g2->DebugLogFlags & RblxUIDebugLogFlags_EventFont) RBLXUI_DEBUG_LOG(__VA_ARGS__); } while (0) // Called from RblxFontAtlas function which may operate without a context.
#define RBLXUI_DEBUG_LOG_INPUTROUTING(...) do{if (g.DebugLogFlags & RblxUIDebugLogFlags_EventInputRouting)RBLXUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define RBLXUI_DEBUG_LOG_DOCKING(...)    do { if (g.DebugLogFlags & RblxUIDebugLogFlags_EventDocking)     RBLXUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define RBLXUI_DEBUG_LOG_VIEWPORT(...)   do { if (g.DebugLogFlags & RblxUIDebugLogFlags_EventViewport)    RBLXUI_DEBUG_LOG(__VA_ARGS__); } while (0)

// Debug options (also see ones on top of rblxui.cpp)
//#define RBLXUI_DEBUG_BOXSELECT

// Static Asserts
#define RBLXUI_STATIC_ASSERT(_COND)         static_assert(_COND, "")

// "Paranoid" Debug Asserts are meant to only be enabled during specific debugging/work, otherwise would slow down the code too much.
// We currently don't have many of those so the effect is currently negligible, but onward intent to add more aggressive ones in the code.
//#define RBLXUI_DEBUG_PARANOID
#ifdef RBLXUI_DEBUG_PARANOID
#define RBLXUI_ASSERT_PARANOID(_EXPR)       RBLXUI_ASSERT(_EXPR)
#else
#define RBLXUI_ASSERT_PARANOID(_EXPR)
#endif

// Misc Macros
#define RBLXUI_PI                           3.14159265358979323846f
#ifdef _WIN32
#define RBLXUI_NEWLINE                      "\r\n"   // Play it nice with Windows users (Update: since 2018-05, Notepad finally appears to support Unix-style carriage returns!)
#else
#define RBLXUI_NEWLINE                      "\n"
#endif
#ifndef RBLXUI_TABSIZE                      // Until we move this to runtime and/or add proper tab support, at least allow users to compile-time override
#define RBLXUI_TABSIZE                      (4)
#endif
#define RBLXUI_MEMALIGN(_OFF,_ALIGN)        (((_OFF) + ((_ALIGN) - 1)) & ~((_ALIGN) - 1))           // Memory align e.g. RBLXUI_ALIGN(0,4)=0, RBLXUI_ALIGN(1,4)=4, RBLXUI_ALIGN(4,4)=4, RBLXUI_ALIGN(5,4)=8
#define RBLXUI_F32_TO_INT8_UNBOUND(_VAL)    ((int)((_VAL) * 255.0f + ((_VAL)>=0 ? 0.5f : -0.5f)))   // Unsaturated, for display purpose
#define RBLXUI_F32_TO_INT8_SAT(_VAL)        ((int)(RblxSaturate(_VAL) * 255.0f + 0.5f))               // Saturated, always output 0..255
#define RBLXUI_TRUNC(_VAL)                  ((float)(int)(_VAL))                                    // Positive values only! RblxTrunc() is not inlined in MSVC debug builds
#define RBLXUI_ROUND(_VAL)                  ((float)(int)((_VAL) + 0.5f))                           // Positive values only! 
//#define RBLXUI_FLOOR RBLXUI_TRUNC             // [OBSOLETE] Renamed in 1.90.0 (Sept 2023)

// Hint for branch prediction
#if (defined(__cplusplus) && (__cplusplus >= 202002L)) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 202002L))
#define RBLXUI_LIKELY   [[likely]]
#define RBLXUI_UNLIKELY [[unlikely]]
#else
#define RBLXUI_LIKELY
#define RBLXUI_UNLIKELY
#endif

// Enforce cdecl calling convention for functions called by the standard library, in case compilation settings changed the default to e.g. __vectorcall
#ifdef _MSC_VER
#define RBLXUI_CDECL __cdecl
#else
#define RBLXUI_CDECL
#endif

// Warnings
#if defined(_MSC_VER) && !defined(__clang__)
#define RBLXUI_MSVC_WARNING_SUPPRESS(XXXX)  __pragma(warning(suppress: XXXX))
#else
#define RBLXUI_MSVC_WARNING_SUPPRESS(XXXX)
#endif

// Debug Tools
// Use 'Metrics/Debugger->Tools->Item Picker' to break into the call-stack of a specific item.
// This will call RBLXUI_DEBUG_BREAK() which you may redefine yourself. See https://github.com/scottt/debugbreak for more reference.
#ifndef RBLXUI_DEBUG_BREAK
#if defined (_MSC_VER)
#define RBLXUI_DEBUG_BREAK()    __debugbreak()
#elif defined(__clang__)
#define RBLXUI_DEBUG_BREAK()    __builtin_debugtrap()
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define RBLXUI_DEBUG_BREAK()    __asm__ volatile("int3;nop")
#elif defined(__GNUC__) && defined(__thumb__)
#define RBLXUI_DEBUG_BREAK()    __asm__ volatile(".inst 0xde01")
#elif defined(__GNUC__) && defined(__arm__) && !defined(__thumb__)
#define RBLXUI_DEBUG_BREAK()    __asm__ volatile(".inst 0xe7f001f0")
#else
#define RBLXUI_DEBUG_BREAK()    RBLXUI_ASSERT(0)    // It is expected that you define RBLXUI_DEBUG_BREAK() into something that will break nicely in a debugger!
#endif
#endif // #ifndef RBLXUI_DEBUG_BREAK

// Format specifiers, printing 64-bit hasn't been decently standardized...
// In a real application you should be using PRId64 and PRIu64 from <inttypes.h> (non-windows) and on Windows define them yourself.
#if defined(_MSC_VER) && !defined(__clang__)
#define RBLXUI_PRId64   "I64d"
#define RBLXUI_PRIu64   "I64u"
#define RBLXUI_PRIX64   "I64X"
#else
#define RBLXUI_PRId64   "lld"
#define RBLXUI_PRIu64   "llu"
#define RBLXUI_PRIX64   "llX"
#endif

//-----------------------------------------------------------------------------
// [SECTION] Generic helpers
// Note that the RblxXXX helpers functions are lower-level than RblxUI functions.
// RblxUI functions or the RblxUI context are never called/used from other RblxXXX functions.
//-----------------------------------------------------------------------------
// - Helpers: Hashing
// - Helpers: Sorting
// - Helpers: Bit manipulation
// - Helpers: String
// - Helpers: Formatting
// - Helpers: UTF-8 <> wchar conversions
// - Helpers: RblxVec2/RblxVec4 operators
// - Helpers: Maths
// - Helpers: Geometry
// - Helper: RblxVec1
// - Helper: RblxVec2ih
// - Helper: RblxRect
// - Helper: RblxBitArray
// - Helper: RblxBitVector
// - Helper: RblxSpan<>, RblxSpanAllocator<>
// - Helper: RblxStableVector<>
// - Helper: RblxPool<>
// - Helper: RblxChunkStream<>
// - Helper: RblxUITextIndex
// - Helper: RblxUIStorage
//-----------------------------------------------------------------------------

// Helpers: Hashing
RBLXUI_API RblxUIID       RblxHashData(const void* data, size_t data_size, RblxUIID seed = 0);
RBLXUI_API RblxUIID       RblxHashStr(const char* data, size_t data_size = 0, RblxUIID seed = 0);
RBLXUI_API const char*   RblxHashSkipUncontributingPrefix(const char* label);

// Helpers: Sorting
#ifndef RblxQsort
inline void             RblxQsort(void* base, size_t count, size_t size_of_element, int(RBLXUI_CDECL *compare_func)(void const*, void const*)) { if (count > 1) qsort(base, count, size_of_element, compare_func); }
#endif

// Helpers: Color Blending
RBLXUI_API RblxU32         RblxAlphaBlendColors(RblxU32 col_a, RblxU32 col_b);

// Helpers: Bit manipulation
inline bool             RblxIsPowerOfTwo(int v)               { return v != 0 && (v & (v - 1)) == 0; }
inline bool             RblxIsPowerOfTwo(RblxU64 v)             { return v != 0 && (v & (v - 1)) == 0; }
inline int              RblxUpperPowerOfTwo(int v)            { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; return v; }
inline unsigned int     RblxCountSetBits(unsigned int v)      { unsigned int count = 0; while (v > 0) { v = v & (v - 1); count++; } return count; }

// Helpers: String
#define RblxStrlen strlen
#define RblxMemchr memchr
RBLXUI_API int           RblxStricmp(const char* str1, const char* str2);                      // Case insensitive compare.
RBLXUI_API int           RblxStrnicmp(const char* str1, const char* str2, size_t count);       // Case insensitive compare to a certain count.
RBLXUI_API void          RblxStrncpy(char* dst, const char* src, size_t count);                // Copy to a certain count and always zero terminate (strncpy doesn't).
RBLXUI_API char*         RblxStrdup(const char* str);                                          // Duplicate a string.
RBLXUI_API void*         RblxMemdup(const void* src, size_t size);                             // Duplicate a chunk of memory.
RBLXUI_API char*         RblxStrdupcpy(char* dst, size_t* p_dst_size, const char* str);        // Copy in provided buffer, recreate buffer if needed.
RBLXUI_API const char*   RblxStrchrRange(const char* str_begin, const char* str_end, char c);  // Find first occurrence of 'c' in string range.
RBLXUI_API const char*   RblxStreolRange(const char* str, const char* str_end);                // End end-of-line
RBLXUI_API const char*   RblxStristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end);  // Find a substring in a string range.
RBLXUI_API void          RblxStrTrimBlanks(char* str);                                         // Remove leading and trailing blanks from a buffer.
RBLXUI_API const char*   RblxStrSkipBlank(const char* str);                                    // Find first non-blank character.
RBLXUI_API int           RblxStrlenW(const RblxWchar* str);                                      // Computer string length (RblxWchar string)
RBLXUI_API const char*   RblxStrbol(const char* buf_mid_line, const char* buf_begin);          // Find beginning-of-line
RBLXUI_MSVC_RUNTIME_CHECKS_OFF
inline char             RblxToUpper(char c)               { return (c >= 'a' && c <= 'z') ? c &= ~32 : c; }
inline bool             RblxCharIsBlankA(char c)          { return c == ' ' || c == '\t'; }
inline bool             RblxCharIsBlankW(unsigned int c)  { return c == ' ' || c == '\t' || c == 0x3000; }
inline bool             RblxCharIsXdigitA(char c)         { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }
RBLXUI_MSVC_RUNTIME_CHECKS_RESTORE

// Helpers: Formatting
RBLXUI_API int           RblxFormatString(char* buf, size_t buf_size, const char* fmt, ...) RBLXUI_FMTARGS(3);
RBLXUI_API int           RblxFormatStringV(char* buf, size_t buf_size, const char* fmt, va_list args) RBLXUI_FMTLIST(3);
RBLXUI_API void          RblxFormatStringToTempBuffer(const char** out_buf, const char** out_buf_end, const char* fmt, ...) RBLXUI_FMTARGS(3);
RBLXUI_API void          RblxFormatStringToTempBufferV(const char** out_buf, const char** out_buf_end, const char* fmt, va_list args) RBLXUI_FMTLIST(3);
RBLXUI_API const char*   RblxParseFormatFindStart(const char* format);
RBLXUI_API const char*   RblxParseFormatFindEnd(const char* format);
RBLXUI_API const char*   RblxParseFormatTrimDecorations(const char* format, char* buf, size_t buf_size);
RBLXUI_API void          RblxParseFormatSanitizeForPrinting(const char* fmt_in, char* fmt_out, size_t fmt_out_size);
RBLXUI_API const char*   RblxParseFormatSanitizeForScanning(const char* fmt_in, char* fmt_out, size_t fmt_out_size);
RBLXUI_API int           RblxParseFormatPrecision(const char* format, int default_value);

// Helpers: UTF-8 <> wchar conversions
RBLXUI_API int           RblxTextCharToUtf8(char out_buf[5], unsigned int c);                                                      // return output UTF-8 bytes count
RBLXUI_API int           RblxTextStrToUtf8(char* out_buf, int out_buf_size, const RblxWchar* in_text, const RblxWchar* in_text_end);   // return output UTF-8 bytes count
RBLXUI_API int           RblxTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end);               // read one character. return input UTF-8 bytes count
RBLXUI_API int           RblxTextStrFromUtf8(RblxWchar* out_buf, int out_buf_size, const char* in_text, const char* in_text_end, const char** in_remaining = NULL);   // return input UTF-8 bytes count
RBLXUI_API int           RblxTextCountCharsFromUtf8(const char* in_text, const char* in_text_end);                                 // return number of UTF-8 code-points (NOT bytes count)
RBLXUI_API int           RblxTextCountUtf8BytesFromChar(const char* in_text, const char* in_text_end);                             // return number of bytes to express one char in UTF-8
RBLXUI_API int           RblxTextCountUtf8BytesFromStr(const RblxWchar* in_text, const RblxWchar* in_text_end);                        // return number of bytes to express string in UTF-8
RBLXUI_API const char*   RblxTextFindPreviousUtf8Codepoint(const char* in_text_start, const char* in_p);                           // return previous UTF-8 code-point.
RBLXUI_API const char*   RblxTextFindValidUtf8CodepointEnd(const char* in_text_start, const char* in_text_end, const char* in_p);  // return previous UTF-8 code-point if 'in_p' is not the end of a valid one.
RBLXUI_API int           RblxTextCountLines(const char* in_text, const char* in_text_end);                                         // return number of lines taken by text. trailing carriage return doesn't count as an extra line.

// Helpers: High-level text functions (DO NOT USE!!! THIS IS A MINIMAL SUBSET OF LARGER UPCOMING CHANGES)
enum RblxDrawTextFlags_
{
    RblxDrawTextFlags_None                = 0,
    RblxDrawTextFlags_CpuFineClip         = 1 << 0,    // Must be == 1/true for legacy with 'bool cpu_fine_clip' arg to RenderText()
    RblxDrawTextFlags_WrapKeepBlanks      = 1 << 1,
    RblxDrawTextFlags_StopOnNewLine       = 1 << 2,
};
RBLXUI_API RblxVec2        RblxFontCalcTextSizeEx(RblxFont* font, float size, float max_width, float wrap_width, const char* text_begin, const char* text_end_display, const char* text_end, const char** out_remaining, RblxVec2* out_offset, RblxDrawTextFlags flags);
RBLXUI_API const char*   RblxFontCalcWordWrapPositionEx(RblxFont* font, float size, const char* text, const char* text_end, float wrap_width, RblxDrawTextFlags flags = 0);
RBLXUI_API const char*   RblxTextCalcWordWrapNextLineStart(const char* text, const char* text_end, RblxDrawTextFlags flags = 0); // trim trailing space and find beginning of next line

// Character classification for word-wrapping logic
enum RblxWcharClass
{
    RblxWcharClass_Blank, RblxWcharClass_Punct, RblxWcharClass_Other
};
RBLXUI_API void          RblxTextInitClassifiers();
RBLXUI_API void          RblxTextClassifierClear(RblxU32* bits, unsigned int codepoint_min, unsigned int codepoint_end, RblxWcharClass char_class);
RBLXUI_API void          RblxTextClassifierSetCharClass(RblxU32* bits, unsigned int codepoint_min, unsigned int codepoint_end, RblxWcharClass char_class, unsigned int c);
RBLXUI_API void          RblxTextClassifierSetCharClassFromStr(RblxU32* bits, unsigned int codepoint_min, unsigned int codepoint_end, RblxWcharClass char_class, const char* s);

// Helpers: File System
#ifdef RBLXUI_DISABLE_FILE_FUNCTIONS
#define RBLXUI_DISABLE_DEFAULT_FILE_FUNCTIONS
typedef void* RblxFileHandle;
inline RblxFileHandle         RblxFileOpen(const char*, const char*)                    { return NULL; }
inline bool                 RblxFileClose(RblxFileHandle)                               { return false; }
inline RblxU64                RblxFileGetSize(RblxFileHandle)                             { return (RblxU64)-1; }
inline RblxU64                RblxFileRead(void*, RblxU64, RblxU64, RblxFileHandle)           { return 0; }
inline RblxU64                RblxFileWrite(const void*, RblxU64, RblxU64, RblxFileHandle)    { return 0; }
#endif
#ifndef RBLXUI_DISABLE_DEFAULT_FILE_FUNCTIONS
typedef FILE* RblxFileHandle;
RBLXUI_API RblxFileHandle      RblxFileOpen(const char* filename, const char* mode);
RBLXUI_API bool              RblxFileClose(RblxFileHandle file);
RBLXUI_API RblxU64             RblxFileGetSize(RblxFileHandle file);
RBLXUI_API RblxU64             RblxFileRead(void* data, RblxU64 size, RblxU64 count, RblxFileHandle file);
RBLXUI_API RblxU64             RblxFileWrite(const void* data, RblxU64 size, RblxU64 count, RblxFileHandle file);
#else
#define RBLXUI_DISABLE_TTY_FUNCTIONS // Can't use stdout, fflush if we are not using default file functions
#endif
RBLXUI_API void*             RblxFileLoadToMemory(const char* filename, const char* mode, size_t* out_file_size = NULL, int padding_bytes = 0);

// Helpers: Maths
RBLXUI_MSVC_RUNTIME_CHECKS_OFF
// - Wrapper for standard libs functions. (Note that rblxui_demo.cpp does _not_ use them to keep the code easy to copy)
#ifndef RBLXUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#define RblxFabs(X)           fabsf(X)
#define RblxSqrt(X)           sqrtf(X)
#define RblxFmod(X, Y)        fmodf((X), (Y))
#define RblxCos(X)            cosf(X)
#define RblxSin(X)            sinf(X)
#define RblxAcos(X)           acosf(X)
#define RblxAtan2(Y, X)       atan2f((Y), (X))
#define RblxAtof(STR)         atof(STR)
#define RblxCeil(X)           ceilf(X)
inline float  RblxPow(float x, float y)    { return powf(x, y); }          // DragBehaviorT/SliderBehaviorT uses RblxPow with either float/double and need the precision
inline double RblxPow(double x, double y)  { return pow(x, y); }
inline float  RblxLog(float x)             { return logf(x); }             // DragBehaviorT/SliderBehaviorT uses RblxLog with either float/double and need the precision
inline double RblxLog(double x)            { return log(x); }
inline int    RblxAbs(int x)               { return x < 0 ? -x : x; }
inline float  RblxAbs(float x)             { return fabsf(x); }
inline double RblxAbs(double x)            { return fabs(x); }
inline float  RblxSign(float x)            { return (x < 0.0f) ? -1.0f : (x > 0.0f) ? 1.0f : 0.0f; } // Sign operator - returns -1, 0 or 1 based on sign of argument
inline double RblxSign(double x)           { return (x < 0.0) ? -1.0 : (x > 0.0) ? 1.0 : 0.0; }
#ifdef RBLXUI_ENABLE_SSE
inline float  RblxRsqrt(float x)           { return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x))); }
#else
inline float  RblxRsqrt(float x)           { return 1.0f / sqrtf(x); }
#endif
inline double RblxRsqrt(double x)          { return 1.0 / sqrt(x); }
#endif
// - RblxMin/RblxMax/RblxClamp/RblxLerp/RblxSwap are used by widgets which support variety of types: signed/unsigned int/long long float/double
// (Exceptionally using templates here but we could also redefine them for those types)
template<typename T> T RblxMin(T lhs, T rhs)                              { return lhs < rhs ? lhs : rhs; }
template<typename T> T RblxMax(T lhs, T rhs)                              { return lhs >= rhs ? lhs : rhs; }
template<typename T> T RblxClamp(T v, T mn, T mx)                         { return (v < mn) ? mn : (v > mx) ? mx : v; }
template<typename T> T RblxLerp(double a, double b, float t)              { return (T)(a + (b - a) * (double)t); }
template<typename T> T RblxLerp(T a, T b, float t)                        { return (T)((float)a + (float)(b - a) * t); }
template<typename T> void RblxSwap(T& a, T& b)                            { T tmp = a; a = b; b = tmp; }
template<typename T> T RblxAddClampOverflow(T a, T b, T mn, T mx)         { if (b < 0 && (a < mn - b)) return mn; if (b > 0 && (a > mx - b)) return mx; return a + b; }
template<typename T> T RblxSubClampOverflow(T a, T b, T mn, T mx)         { if (b > 0 && (a < mn + b)) return mn; if (b < 0 && (a > mx + b)) return mx; return a - b; }
// - Misc maths helpers
inline RblxVec2 RblxMin(const RblxVec2& lhs, const RblxVec2& rhs)               { return RblxVec2(lhs.x < rhs.x ? lhs.x : rhs.x, lhs.y < rhs.y ? lhs.y : rhs.y); }
inline RblxVec2 RblxMax(const RblxVec2& lhs, const RblxVec2& rhs)               { return RblxVec2(lhs.x >= rhs.x ? lhs.x : rhs.x, lhs.y >= rhs.y ? lhs.y : rhs.y); }
inline RblxVec2 RblxClamp(const RblxVec2& v, const RblxVec2&mn, const RblxVec2&mx){ return RblxVec2((v.x < mn.x) ? mn.x : (v.x > mx.x) ? mx.x : v.x, (v.y < mn.y) ? mn.y : (v.y > mx.y) ? mx.y : v.y); }
inline RblxVec2 RblxLerp(const RblxVec2& a, const RblxVec2& b, float t)         { return RblxVec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
inline RblxVec2 RblxLerp(const RblxVec2& a, const RblxVec2& b, const RblxVec2& t) { return RblxVec2(a.x + (b.x - a.x) * t.x, a.y + (b.y - a.y) * t.y); }
inline RblxVec4 RblxLerp(const RblxVec4& a, const RblxVec4& b, float t)         { return RblxVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t); }
inline float  RblxSaturate(float f)                                       { return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f; }
inline float  RblxLengthSqr(const RblxVec2& lhs)                            { return (lhs.x * lhs.x) + (lhs.y * lhs.y); }
inline float  RblxLengthSqr(const RblxVec4& lhs)                            { return (lhs.x * lhs.x) + (lhs.y * lhs.y) + (lhs.z * lhs.z) + (lhs.w * lhs.w); }
inline float  RblxInvLength(const RblxVec2& lhs, float fail_value)          { float d = (lhs.x * lhs.x) + (lhs.y * lhs.y); if (d > 0.0f) return RblxRsqrt(d); return fail_value; }
inline float  RblxTrunc(float f)                                          { return (float)(int)(f); }
inline RblxVec2 RblxTrunc(const RblxVec2& v)                                  { return RblxVec2((float)(int)(v.x), (float)(int)(v.y)); }
inline float  RblxFloor(float f)                                          { return (float)((f >= 0 || (float)(int)f == f) ? (int)f : (int)f - 1); } // Decent replacement for floorf()
inline RblxVec2 RblxFloor(const RblxVec2& v)                                  { return RblxVec2(RblxFloor(v.x), RblxFloor(v.y)); }
inline float  RblxTrunc64(float f)                                        { return (float)(RblxS64)(f); }
inline float  RblxRound64(float f)                                        { return (float)(RblxS64)(f + 0.5f); } // FIXME: Positive values only.
inline float  RblxCeilFast(float f)                                       { int i = (int)f; return (float)(i + (f > (float)i)); } // Consider using the the bit-hack version (search for "0x1p120f").
inline int    RblxModPositive(int a, int b)                               { return (a + b) % b; }
inline float  RblxDot(const RblxVec2& a, const RblxVec2& b)                   { return a.x * b.x + a.y * b.y; }
inline RblxVec2 RblxRotate(const RblxVec2& v, float cos_a, float sin_a)       { return RblxVec2(v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a); }
inline float  RblxLinearSweep(float current, float target, float speed)   { if (current < target) return RblxMin(current + speed, target); if (current > target) return RblxMax(current - speed, target); return current; }
inline float  RblxLinearRemapClamp(float s0, float s1, float d0, float d1, float x) { return RblxSaturate((x - s0) / (s1 - s0)) * (d1 - d0) + d0; }
inline RblxVec2 RblxMul(const RblxVec2& lhs, const RblxVec2& rhs)               { return RblxVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
inline bool   RblxIsFloatAboveGuaranteedIntegerPrecision(float f)         { return f <= -16777216 || f >= 16777216; }
inline float  RblxExponentialMovingAverage(float avg, float sample, int n){ avg -= avg / (float)n; avg += sample / (float)n; return avg; }
RBLXUI_MSVC_RUNTIME_CHECKS_RESTORE

// Helpers: Geometry
RBLXUI_API RblxVec2     RblxBezierCubicCalc(const RblxVec2& p1, const RblxVec2& p2, const RblxVec2& p3, const RblxVec2& p4, float t);
RBLXUI_API RblxVec2     RblxBezierCubicClosestPoint(const RblxVec2& p1, const RblxVec2& p2, const RblxVec2& p3, const RblxVec2& p4, const RblxVec2& p, int num_segments);       // For curves with explicit number of segments
RBLXUI_API RblxVec2     RblxBezierCubicClosestPointCasteljau(const RblxVec2& p1, const RblxVec2& p2, const RblxVec2& p3, const RblxVec2& p4, const RblxVec2& p, float tess_tol);// For auto-tessellated curves you can use tess_tol = style.CurveTessellationTol
RBLXUI_API RblxVec2     RblxBezierQuadraticCalc(const RblxVec2& p1, const RblxVec2& p2, const RblxVec2& p3, float t);
RBLXUI_API RblxVec2     RblxLineClosestPoint(const RblxVec2& a, const RblxVec2& b, const RblxVec2& p);
RBLXUI_API bool       RblxTriangleContainsPoint(const RblxVec2& a, const RblxVec2& b, const RblxVec2& c, const RblxVec2& p);
RBLXUI_API RblxVec2     RblxTriangleClosestPoint(const RblxVec2& a, const RblxVec2& b, const RblxVec2& c, const RblxVec2& p);
RBLXUI_API void       RblxTriangleBarycentricCoords(const RblxVec2& a, const RblxVec2& b, const RblxVec2& c, const RblxVec2& p, float& out_u, float& out_v, float& out_w);
inline float         RblxTriangleArea(const RblxVec2& a, const RblxVec2& b, const RblxVec2& c)          { return RblxFabs((a.x * (b.y - c.y)) + (b.x * (c.y - a.y)) + (c.x * (a.y - b.y))) * 0.5f; }
inline bool          RblxTriangleIsClockwise(const RblxVec2& a, const RblxVec2& b, const RblxVec2& c)   { return ((b.x - a.x) * (c.y - b.y)) - ((c.x - b.x) * (b.y - a.y)) > 0.0f; }

// Helper: RblxVec1 (1D vector)
// (this odd construct is used to facilitate the transition between 1D and 2D, and the maintenance of some branches/patches)
RBLXUI_MSVC_RUNTIME_CHECKS_OFF
struct RblxVec1
{
    float   x;
    constexpr RblxVec1()         : x(0.0f) { }
    constexpr RblxVec1(float _x) : x(_x) { }
};

// Helper: RblxVec2i (2D vector, integer)
struct RblxVec2i
{
    int         x, y;
    constexpr RblxVec2i()                             : x(0), y(0) {}
    constexpr RblxVec2i(int _x, int _y)               : x(_x), y(_y) {}
};

// Helper: RblxVec2ih (2D vector, half-size integer, for long-term packed storage)
struct RblxVec2ih
{
    short   x, y;
    constexpr RblxVec2ih()                           : x(0), y(0) {}
    constexpr RblxVec2ih(short _x, short _y)         : x(_x), y(_y) {}
    constexpr explicit RblxVec2ih(const RblxVec2& rhs) : x((short)rhs.x), y((short)rhs.y) {}
};

// Helper: RblxRect (2D axis aligned bounding-box)
// NB: we can't rely on RblxVec2 math operators being available here!
struct RBLXUI_API RblxRect
{
    RblxVec2      Min;    // Upper-left
    RblxVec2      Max;    // Lower-right

    constexpr RblxRect()                                        : Min(0.0f, 0.0f), Max(0.0f, 0.0f)  {}
    constexpr RblxRect(const RblxVec2& min, const RblxVec2& max)    : Min(min), Max(max)                {}
    constexpr RblxRect(const RblxVec4& v)                         : Min(v.x, v.y), Max(v.z, v.w)      {}
    constexpr RblxRect(float x1, float y1, float x2, float y2)  : Min(x1, y1), Max(x2, y2)          {}

    RblxVec2      GetCenter() const                   { return RblxVec2((Min.x + Max.x) * 0.5f, (Min.y + Max.y) * 0.5f); }
    RblxVec2      GetSize() const                     { return RblxVec2(Max.x - Min.x, Max.y - Min.y); }
    float       GetWidth() const                    { return Max.x - Min.x; }
    float       GetHeight() const                   { return Max.y - Min.y; }
    float       GetArea() const                     { return (Max.x - Min.x) * (Max.y - Min.y); }
    RblxVec2      GetTL() const                       { return Min; }                   // Top-left
    RblxVec2      GetTR() const                       { return RblxVec2(Max.x, Min.y); }  // Top-right
    RblxVec2      GetBL() const                       { return RblxVec2(Min.x, Max.y); }  // Bottom-left
    RblxVec2      GetBR() const                       { return Max; }                   // Bottom-right
    bool        Contains(const RblxVec2& p) const     { return p.x     >= Min.x && p.y     >= Min.y && p.x     <  Max.x && p.y     <  Max.y; }
    bool        Contains(const RblxRect& r) const     { return r.Min.x >= Min.x && r.Min.y >= Min.y && r.Max.x <= Max.x && r.Max.y <= Max.y; }
    bool        ContainsWithPad(const RblxVec2& p, const RblxVec2& pad) const { return p.x >= Min.x - pad.x && p.y >= Min.y - pad.y && p.x < Max.x + pad.x && p.y < Max.y + pad.y; }
    bool        Overlaps(const RblxRect& r) const     { return r.Min.y <  Max.y && r.Max.y >  Min.y && r.Min.x <  Max.x && r.Max.x >  Min.x; }
    void        Add(const RblxVec2& p)                { if (Min.x > p.x)     Min.x = p.x;     if (Min.y > p.y)     Min.y = p.y;     if (Max.x < p.x)     Max.x = p.x;     if (Max.y < p.y)     Max.y = p.y; }
    void        Add(const RblxRect& r)                { if (Min.x > r.Min.x) Min.x = r.Min.x; if (Min.y > r.Min.y) Min.y = r.Min.y; if (Max.x < r.Max.x) Max.x = r.Max.x; if (Max.y < r.Max.y) Max.y = r.Max.y; }
    void        AddX(float x)                       { if (Min.x > x)       Min.x = x;       if (Max.x < x)       Max.x = x; }
    void        AddY(float y)                       { if (Min.y > y)       Min.y = y;       if (Max.y < y)       Max.y = y; }
    void        Expand(const float amount)          { Min.x -= amount;   Min.y -= amount;   Max.x += amount;   Max.y += amount; }
    void        Expand(const RblxVec2& amount)        { Min.x -= amount.x; Min.y -= amount.y; Max.x += amount.x; Max.y += amount.y; }
    void        Translate(const RblxVec2& d)          { Min.x += d.x; Min.y += d.y; Max.x += d.x; Max.y += d.y; }
    void        TranslateX(float dx)                { Min.x += dx; Max.x += dx; }
    void        TranslateY(float dy)                { Min.y += dy; Max.y += dy; }
    void        ClipWith(const RblxRect& r)           { Min = RblxMax(Min, r.Min); Max = RblxMin(Max, r.Max); }                   // Simple version, may lead to an inverted rectangle, which is fine for Contains/Overlaps test but not for display.
    void        ClipWithFull(const RblxRect& r)       { Min = RblxClamp(Min, r.Min, r.Max); Max = RblxClamp(Max, r.Min, r.Max); } // Full version, ensure both points are fully clipped.
    bool        IsInverted() const                  { return Min.x > Max.x || Min.y > Max.y; }
    RblxVec4      ToVec4() const                      { return RblxVec4(Min.x, Min.y, Max.x, Max.y); }
    const RblxVec4& AsVec4() const                    { return *(const RblxVec4*)&Min.x; }
};

// Helper: RblxBitArray
#define         RBLXUI_BITARRAY_TESTBIT(_ARRAY, _N)                 ((_ARRAY[(_N) >> 5] & ((RblxU32)1 << ((_N) & 31))) != 0) // Macro version of RblxBitArrayTestBit(): ensure args have side-effect or are costly!
#define         RBLXUI_BITARRAY_CLEARBIT(_ARRAY, _N)                ((_ARRAY[(_N) >> 5] &= ~((RblxU32)1 << ((_N) & 31))))    // Macro version of RblxBitArrayClearBit(): ensure args have side-effect or are costly!
inline size_t   RblxBitArrayGetStorageSizeInBytes(int bitcount)   { return (size_t)((bitcount + 31) >> 5) << 2; }
inline void     RblxBitArrayClearAllBits(RblxU32* arr, int bitcount){ memset(arr, 0, RblxBitArrayGetStorageSizeInBytes(bitcount)); }
inline bool     RblxBitArrayTestBit(const RblxU32* arr, int n)      { RblxU32 mask = (RblxU32)1 << (n & 31); return (arr[n >> 5] & mask) != 0; }
inline void     RblxBitArrayClearBit(RblxU32* arr, int n)           { RblxU32 mask = (RblxU32)1 << (n & 31); arr[n >> 5] &= ~mask; }
inline void     RblxBitArraySetBit(RblxU32* arr, int n)             { RblxU32 mask = (RblxU32)1 << (n & 31); arr[n >> 5] |= mask; }
inline void     RblxBitArraySetBitRange(RblxU32* arr, int n, int n2) // Works on range [n..n2)
{
    n2--;
    while (n <= n2)
    {
        int a_mod = (n & 31);
        int b_mod = (n2 > (n | 31) ? 31 : (n2 & 31)) + 1;
        RblxU32 mask = (RblxU32)(((RblxU64)1 << b_mod) - 1) & ~(RblxU32)(((RblxU64)1 << a_mod) - 1);
        arr[n >> 5] |= mask;
        n = (n + 32) & ~31;
    }
}

typedef RblxU32* RblxBitArrayPtr; // Name for use in structs

// Helper: RblxBitArray class (wrapper over RblxBitArray functions)
// Store 1-bit per value.
template<int BITCOUNT, int OFFSET = 0>
struct RblxBitArray
{
    RblxU32           Data[(BITCOUNT + 31) >> 5];
    RblxBitArray()                                { ClearAllBits(); }
    void            ClearAllBits()              { memset(Data, 0, sizeof(Data)); }
    void            SetAllBits()                { memset(Data, 255, sizeof(Data)); }
    bool            TestBit(int n) const        { n += OFFSET; RBLXUI_ASSERT(n >= 0 && n < BITCOUNT); return RBLXUI_BITARRAY_TESTBIT(Data, n); }
    void            SetBit(int n)               { n += OFFSET; RBLXUI_ASSERT(n >= 0 && n < BITCOUNT); RblxBitArraySetBit(Data, n); }
    void            ClearBit(int n)             { n += OFFSET; RBLXUI_ASSERT(n >= 0 && n < BITCOUNT); RblxBitArrayClearBit(Data, n); }
    void            SetBitRange(int n, int n2)  { n += OFFSET; n2 += OFFSET; RBLXUI_ASSERT(n >= 0 && n < BITCOUNT && n2 > n && n2 <= BITCOUNT); RblxBitArraySetBitRange(Data, n, n2); } // Works on range [n..n2)
    bool            operator[](int n) const     { n += OFFSET; RBLXUI_ASSERT(n >= 0 && n < BITCOUNT); return RBLXUI_BITARRAY_TESTBIT(Data, n); }
};

// Helper: RblxBitVector
// Store 1-bit per value.
struct RBLXUI_API RblxBitVector
{
    RblxVector<RblxU32> Storage;
    void            Create(int sz)              { Storage.resize((sz + 31) >> 5); memset(Storage.Data, 0, (size_t)Storage.Size * sizeof(Storage.Data[0])); }
    void            Clear()                     { Storage.clear(); }
    bool            TestBit(int n) const        { RBLXUI_ASSERT(n < (Storage.Size << 5)); return RBLXUI_BITARRAY_TESTBIT(Storage.Data, n); }
    void            SetBit(int n)               { RBLXUI_ASSERT(n < (Storage.Size << 5)); RblxBitArraySetBit(Storage.Data, n); }
    void            ClearBit(int n)             { RBLXUI_ASSERT(n < (Storage.Size << 5)); RblxBitArrayClearBit(Storage.Data, n); }
};
RBLXUI_MSVC_RUNTIME_CHECKS_RESTORE

// Helper: RblxSpan<>
// Pointing to a span of data we don't own.
template<typename T>
struct RblxSpan
{
    T*                  Data;
    T*                  DataEnd;

    // Constructors, destructor
    inline RblxSpan()                                 { Data = DataEnd = NULL; }
    inline RblxSpan(T* data, int size)                { Data = data; DataEnd = data + size; }
    inline RblxSpan(T* data, T* data_end)             { Data = data; DataEnd = data_end; }

    inline void         set(T* data, int size)      { Data = data; DataEnd = data + size; }
    inline void         set(T* data, T* data_end)   { Data = data; DataEnd = data_end; }
    inline int          size() const                { return (int)(ptrdiff_t)(DataEnd - Data); }
    inline int          size_in_bytes() const       { return (int)(ptrdiff_t)(DataEnd - Data) * (int)sizeof(T); }
    inline T&           operator[](int i)           { T* p = Data + i; RBLXUI_ASSERT(p >= Data && p < DataEnd); return *p; }
    inline const T&     operator[](int i) const     { const T* p = Data + i; RBLXUI_ASSERT(p >= Data && p < DataEnd); return *p; }

    inline T*           begin()                     { return Data; }
    inline const T*     begin() const               { return Data; }
    inline T*           end()                       { return DataEnd; }
    inline const T*     end() const                 { return DataEnd; }

    // Utilities
    inline int  index_from_ptr(const T* it) const   { RBLXUI_ASSERT(it >= Data && it < DataEnd); const ptrdiff_t off = it - Data; return (int)off; }
};

// Helper: RblxSpanAllocator<>
// Facilitate storing multiple chunks into a single large block (the "arena")
// - Usage: call Reserve() N times, allocate GetArenaSizeInBytes() worth, pass it to SetArenaBasePtr(), call GetSpan() N times to retrieve the aligned ranges.
template<int CHUNKS>
struct RblxSpanAllocator
{
    char*   BasePtr;
    int     CurrOff;
    int     CurrIdx;
    int     Offsets[CHUNKS];
    int     Sizes[CHUNKS];

    RblxSpanAllocator()                               { memset((void*)this, 0, sizeof(*this)); }
    inline void  Reserve(int n, size_t sz, int a=4) { RBLXUI_ASSERT(n == CurrIdx && n < CHUNKS); CurrOff = RBLXUI_MEMALIGN(CurrOff, a); Offsets[n] = CurrOff; Sizes[n] = (int)sz; CurrIdx++; CurrOff += (int)sz; }
    inline int   GetArenaSizeInBytes()              { return CurrOff; }
    inline void  SetArenaBasePtr(void* base_ptr)    { BasePtr = (char*)base_ptr; }
    inline void* GetSpanPtrBegin(int n)             { RBLXUI_ASSERT(n >= 0 && n < CHUNKS && CurrIdx == CHUNKS); return (void*)(BasePtr + Offsets[n]); }
    inline void* GetSpanPtrEnd(int n)               { RBLXUI_ASSERT(n >= 0 && n < CHUNKS && CurrIdx == CHUNKS); return (void*)(BasePtr + Offsets[n] + Sizes[n]); }
    template<typename T>
    inline void  GetSpan(int n, RblxSpan<T>* span)    { span->set((T*)GetSpanPtrBegin(n), (T*)GetSpanPtrEnd(n)); }
};

// Helper: RblxStableVector<>
// Allocating chunks of BLOCKSIZE items. Objects pointers are never invalidated when growing, only by clear().
// Important: does not destruct anything!
// Implemented only the minimum set of functions we need for it.
template<typename T, int BLOCKSIZE>
struct RblxStableVector
{
    int                 Size = 0;
    int                 Capacity = 0;
    RblxVector<T*>        Blocks;

    // Functions
    inline ~RblxStableVector()                        { for (T* block : Blocks) RBLXUI_FREE(block); }

    inline void         clear()                     { Size = Capacity = 0; Blocks.clear_delete(); }
    inline void         resize(int new_size)        { if (new_size > Capacity) reserve(new_size); Size = new_size; }
    inline void         reserve(int new_cap)
    {
        new_cap = RBLXUI_MEMALIGN(new_cap, BLOCKSIZE);
        int old_count = Capacity / BLOCKSIZE;
        int new_count = new_cap / BLOCKSIZE;
        if (new_count <= old_count)
            return;
        Blocks.resize(new_count);
        for (int n = old_count; n < new_count; n++)
            Blocks[n] = (T*)RBLXUI_ALLOC(sizeof(T) * BLOCKSIZE);
        Capacity = new_cap;
    }
    inline T&           operator[](int i)           { RBLXUI_ASSERT(i >= 0 && i < Size); return Blocks[i / BLOCKSIZE][i % BLOCKSIZE]; }
    inline const T&     operator[](int i) const     { RBLXUI_ASSERT(i >= 0 && i < Size); return Blocks[i / BLOCKSIZE][i % BLOCKSIZE]; }
    inline T*           push_back(const T& v)       { int i = Size; RBLXUI_ASSERT(i >= 0); if (Size == Capacity) reserve(Capacity + BLOCKSIZE); void* ptr = &Blocks[i / BLOCKSIZE][i % BLOCKSIZE]; memcpy(ptr, &v, sizeof(v)); Size++; return (T*)ptr; }
};

// Helper: RblxPool<>
// Basic keyed storage for contiguous instances, slow/amortized insertion, O(1) indexable, O(Log N) queries by ID over a dense/hot buffer,
// Honor constructor/destructor. Add/remove invalidate all pointers. Indexes have the same lifetime as the associated object.
typedef int RblxPoolIdx;
template<typename T>
struct RblxPool
{
    RblxVector<T>     Buf;        // Contiguous data
    RblxUIStorage    Map;        // ID->Index
    RblxPoolIdx       FreeIdx;    // Next free idx to use
    RblxPoolIdx       AliveCount; // Number of active/alive items (for display purpose)

    RblxPool()    { FreeIdx = AliveCount = 0; }
    ~RblxPool()   { Clear(); }
    T*          GetByKey(RblxUIID key)               { int idx = Map.GetInt(key, -1); return (idx != -1) ? &Buf[idx] : NULL; }
    T*          GetByIndex(RblxPoolIdx n)             { return &Buf[n]; }
    RblxPoolIdx   GetIndex(const T* p) const          { RBLXUI_ASSERT(p >= Buf.Data && p < Buf.Data + Buf.Size); return (RblxPoolIdx)(p - Buf.Data); }
    T*          GetOrAddByKey(RblxUIID key)          { int* p_idx = Map.GetIntRef(key, -1); if (*p_idx != -1) return &Buf[*p_idx]; *p_idx = FreeIdx; return Add(); }
    bool        Contains(const T* p) const          { return (p >= Buf.Data && p < Buf.Data + Buf.Size); }
    void        Clear()                             { for (int n = 0; n < Map.Data.Size; n++) { int idx = Map.Data[n].val_i; if (idx != -1) Buf[idx].~T(); } Map.Clear(); Buf.clear(); FreeIdx = AliveCount = 0; }
    T*          Add()                               { int idx = FreeIdx; if (idx == Buf.Size) { Buf.resize(Buf.Size + 1); FreeIdx++; } else { FreeIdx = *(int*)&Buf[idx]; } RBLXUI_PLACEMENT_NEW(&Buf[idx]) T(); AliveCount++; return &Buf[idx]; }
    void        Remove(RblxUIID key, const T* p)     { Remove(key, GetIndex(p)); }
    void        Remove(RblxUIID key, RblxPoolIdx idx)  { Buf[idx].~T(); *(int*)&Buf[idx] = FreeIdx; FreeIdx = idx; Map.SetInt(key, -1); AliveCount--; }
    void        Reserve(int capacity)               { Buf.reserve(capacity); Map.Data.reserve(capacity); }

    // To iterate a RblxPool: for (int n = 0; n < pool.GetMapSize(); n++) if (T* t = pool.TryGetMapData(n)) { ... }
    // Can be avoided if you know .Remove() has never been called on the pool, or AliveCount == GetMapSize()
    int         GetAliveCount() const               { return AliveCount; }      // Number of active/alive items in the pool (for display purpose)
    int         GetBufSize() const                  { return Buf.Size; }
    int         GetMapSize() const                  { return Map.Data.Size; }   // It is the map we need iterate to find valid items, since we don't have "alive" storage anywhere
    T*          TryGetMapData(RblxPoolIdx n)          { int idx = Map.Data[n].val_i; if (idx == -1) return NULL; return GetByIndex(idx); }
};

// Helper: RblxChunkStream<>
// Build and iterate a contiguous stream of variable-sized structures.
// This is used by Settings to store persistent data while reducing allocation count.
// We store the chunk size first, and align the final size on 4 bytes boundaries.
// The tedious/zealous amount of casting is to avoid -Wcast-align warnings.
template<typename T>
struct RblxChunkStream
{
    RblxVector<char>  Buf;

    void    clear()                     { Buf.clear(); }
    bool    empty() const               { return Buf.Size == 0; }
    int     size() const                { return Buf.Size; }
    T*      alloc_chunk(size_t sz)      { size_t HDR_SZ = 4; sz = RBLXUI_MEMALIGN(HDR_SZ + sz, 4u); int off = Buf.Size; Buf.resize(off + (int)sz); ((int*)(void*)(Buf.Data + off))[0] = (int)sz; return (T*)(void*)(Buf.Data + off + (int)HDR_SZ); }
    T*      begin()                     { size_t HDR_SZ = 4; if (!Buf.Data) return NULL; return (T*)(void*)(Buf.Data + HDR_SZ); }
    T*      next_chunk(T* p)            { size_t HDR_SZ = 4; RBLXUI_ASSERT(p >= begin() && p < end()); p = (T*)(void*)((char*)(void*)p + chunk_size(p)); if (p == (T*)(void*)((char*)end() + HDR_SZ)) return (T*)0; RBLXUI_ASSERT(p < end()); return p; }
    int     chunk_size(const T* p)      { return ((const int*)p)[-1]; }
    T*      end()                       { return (T*)(void*)(Buf.Data + Buf.Size); }
    int     offset_from_ptr(const T* p) { RBLXUI_ASSERT(p >= begin() && p < end()); const ptrdiff_t off = (const char*)p - Buf.Data; return (int)off; }
    T*      ptr_from_offset(int off)    { RBLXUI_ASSERT(off >= 4 && off < Buf.Size); return (T*)(void*)(Buf.Data + off); }
    void    swap(RblxChunkStream<T>& rhs) { rhs.Buf.swap(Buf); }
};

// Helper: RblxUITextIndex
// Maintain a line index for a text buffer. This is a strong candidate to be moved into the public API.
struct RblxUITextIndex
{
    RblxVector<int>   Offsets;
    int             EndOffset = 0;                          // Because we don't own text buffer we need to maintain EndOffset (may bake in LineOffsets?)

    void            clear()                                 { Offsets.clear(); EndOffset = 0; }
    int             size()                                  { return Offsets.Size; }
    const char*     get_line_begin(const char* base, int n) { return base + (Offsets.Size != 0 ? Offsets[n] : 0); }
    const char*     get_line_end(const char* base, int n)   { return base + (n + 1 < Offsets.Size ? (Offsets[n + 1] - 1) : EndOffset); }
    void            append(const char* base, int old_size, int new_size);
};

// Helper: RblxUIStorage
RBLXUI_API RblxUIStoragePair* RblxLowerBound(RblxUIStoragePair* in_begin, RblxUIStoragePair* in_end, RblxUIID key);

//-----------------------------------------------------------------------------
// [SECTION] RblxDrawList support
//-----------------------------------------------------------------------------

// RblxDrawList: Helper function to calculate a circle's segment count given its radius and a "maximum error" value.
// Estimation of number of circle segment based on error is derived using method described in https://stackoverflow.com/a/2244088/15194693
// Number of segments (N) is calculated using equation:
//   N = ceil ( pi / acos(1 - error / r) )     where r > 0, error <= r
// Our equation is significantly simpler that one in the post thanks for choosing segment that is
// perpendicular to X axis. Follow steps in the article from this starting condition and you will
// will get this result.
//
// Rendering circles with an odd number of segments, while mathematically correct will produce
// asymmetrical results on the raster grid. Therefore we're rounding N to next even number (7->8, 8->8, 9->10 etc.)
#define RBLXUI_ROUNDUP_TO_EVEN(_V)                                  ((((_V) + 1) / 2) * 2)
#define RBLXUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN                     4
#define RBLXUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX                     512
#define RBLXUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(_RAD,_MAXERROR)    RblxClamp(RBLXUI_ROUNDUP_TO_EVEN((int)RblxCeil(RBLXUI_PI / RblxAcos(1 - RblxMin((_MAXERROR), (_RAD)) / (_RAD)))), RBLXUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN, RBLXUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX)

// Raw equation from RBLXUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC rewritten for 'r' and 'error'.
#define RBLXUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(_N,_MAXERROR)    ((_MAXERROR) / (1 - RblxCos(RBLXUI_PI / RblxMax((float)(_N), RBLXUI_PI))))
#define RBLXUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_ERROR(_N,_RAD)     ((1 - RblxCos(RBLXUI_PI / RblxMax((float)(_N), RBLXUI_PI))) / (_RAD))

// RblxDrawList: Lookup table size for adaptive arc drawing, cover full circle.
#ifndef RBLXUI_DRAWLIST_ARCFAST_TABLE_SIZE
#define RBLXUI_DRAWLIST_ARCFAST_TABLE_SIZE                          48 // Number of samples in lookup table.
#endif
#define RBLXUI_DRAWLIST_ARCFAST_SAMPLE_MAX                          RBLXUI_DRAWLIST_ARCFAST_TABLE_SIZE // Sample index _PathArcToFastEx() for 360 angle.

// Data shared between all RblxDrawList instances
// Conceptually this could have been called e.g. RblxDrawListSharedContext
// Typically one RblxUI context would create and maintain one of this.
// You may want to create your own instance of you try to RblxDrawList completely without RblxUI. In that case, watch out for future changes to this structure.
struct RBLXUI_API RblxDrawListSharedData
{
    RblxVec2          TexUvWhitePixel;            // UV of white pixel in the atlas (== FontAtlas->TexUvWhitePixel)
    const RblxVec4*   TexUvLines;                 // UV of anti-aliased lines in the atlas (== FontAtlas->TexUvLines)
    RblxFontAtlas*    FontAtlas;                  // Current font atlas
    RblxFont*         Font;                       // Current font (used for simplified AddText overload)
    float           FontSize;                   // Current font size (used for for simplified AddText overload)
    float           FontScale;                  // Current font scale (== FontSize / Font->FontSize)
    float           CurveTessellationTol;       // Tessellation tolerance when using PathBezierCurveTo()
    float           CircleTessellationMaxError; // Number of circle segments to use per pixel of radius for AddCircle() etc
    float           InitialFringeScale;         // Initial scale to apply to AA fringe
    RblxDrawListFlags InitialFlags;               // Initial flags at the beginning of the frame (it is possible to alter flags on a per-drawlist basis afterwards)
    RblxVec4          ClipRectFullscreen;         // Value for PushClipRectFullscreen()
    RblxVector<RblxVec2> TempBuffer;                // Temporary write buffer
    RblxVector<RblxDrawList*> DrawLists;            // All draw lists associated to this RblxDrawListSharedData
    RblxUIContext*   Context;                    // [OPTIONAL] Link to Dear RblxUI context. 99% of RblxDrawList/RblxFontAtlas can function without an RblxUI context, but this facilitate handling one legacy edge case.

    // Lookup tables
    RblxVec2          ArcFastVtx[RBLXUI_DRAWLIST_ARCFAST_TABLE_SIZE]; // Sample points on the quarter of the circle.
    float           ArcFastRadiusCutoff;                        // Cutoff radius after which arc drawing will fallback to slower PathArcTo()
    RblxU8            CircleSegmentCounts[64];    // Precomputed segment count for given radius before we calculate it dynamically (to avoid calculation overhead)

    RblxDrawListSharedData();
    ~RblxDrawListSharedData();
    void SetCircleTessellationMaxError(float max_error);
};

struct RblxDrawDataBuilder
{
    RblxVector<RblxDrawList*>*  Layers[2];      // Pointers to global layers for: regular, tooltip. LayersP[0] is owned by DrawData.
    RblxVector<RblxDrawList*>   LayerData1;

    RblxDrawDataBuilder()                     { memset((void*)this, 0, sizeof(*this)); }
};

struct RblxFontStackData
{
    RblxFont*     Font;
    float       FontSizeBeforeScaling;      // ~~ style.FontSizeBase
    float       FontSizeAfterScaling;       // ~~ g.FontSize
};

//-----------------------------------------------------------------------------
// [SECTION] Style support
//-----------------------------------------------------------------------------

struct RblxUIStyleVarInfo
{
    RblxU32           Count : 8;      // 1+
    RblxUIDataType   DataType : 8;
    RblxU32           Offset : 16;    // Offset in parent structure
    void* GetVarPtr(void* parent) const { return (void*)((unsigned char*)parent + Offset); }
};

// Stacked color modifier, backup of modified data so we can restore it
struct RblxUIColorMod
{
    RblxUICol        Col;
    RblxVec4          BackupValue;
};

// Stacked style modifier, backup of modified data so we can restore it. Data type inferred from the variable.
struct RblxUIStyleMod
{
    RblxUIStyleVar   VarIdx;
    union           { int BackupInt[2]; float BackupFloat[2]; };
    RblxUIStyleMod(RblxUIStyleVar idx, int v)     { VarIdx = idx; BackupInt[0] = v; }
    RblxUIStyleMod(RblxUIStyleVar idx, float v)   { VarIdx = idx; BackupFloat[0] = v; }
    RblxUIStyleMod(RblxUIStyleVar idx, RblxVec2 v)  { VarIdx = idx; BackupFloat[0] = v.x; BackupFloat[1] = v.y; }
};

//-----------------------------------------------------------------------------
// [SECTION] Data types support
//-----------------------------------------------------------------------------

struct RblxUIDataTypeStorage
{
    RblxU8        Data[8];        // Opaque storage to fit any data up to RblxUIDataType_COUNT
};

// Type information associated to one RblxUIDataType. Retrieve with DataTypeGetInfo().
struct RblxUIDataTypeInfo
{
    size_t      Size;           // Size in bytes
    const char* Name;           // Short descriptive name for the type, for debugging
    const char* PrintFmt;       // Default printf format for the type
    const char* ScanFmt;        // Default scanf format for the type
};

// Extend RblxUIDataType_
enum RblxUIDataTypePrivate_
{
    RblxUIDataType_Pointer = RblxUIDataType_COUNT,
    RblxUIDataType_ID,
};

//-----------------------------------------------------------------------------
// [SECTION] Widgets support: flags, enums, data structures
//-----------------------------------------------------------------------------

// Extend RblxUIItemFlags
// - input: PushItemFlag() manipulates g.CurrentItemFlags, g.NextItemData.ItemFlags, ItemAdd() calls may add extra flags too.
// - output: stored in g.LastItemData.ItemFlags
enum RblxUIItemFlagsPrivate_
{
    // Controlled by user
    RblxUIItemFlags_ReadOnly                 = 1 << 11, // false     // [ALPHA] Allow hovering interactions but underlying value is not changed.
    RblxUIItemFlags_MixedValue               = 1 << 12, // false     // [BETA] Represent a mixed/indeterminate value, generally multi-selection where values differ. Currently only supported by Checkbox() (later should support all sorts of widgets)
    RblxUIItemFlags_NoWindowHoverableCheck   = 1 << 13, // false     // Disable hoverable check in ItemHoverable()
    RblxUIItemFlags_AllowOverlap             = 1 << 14, // false     // Allow being overlapped by another widget. Not-hovered to Hovered transition deferred by a frame.
    RblxUIItemFlags_NoNavDisableMouseHover   = 1 << 15, // false     // Nav keyboard/gamepad mode doesn't disable hover highlight (behave as if NavHighlightItemUnderNav==false).
    RblxUIItemFlags_NoMarkEdited             = 1 << 16, // false     // Skip calling MarkItemEdited()
    RblxUIItemFlags_NoFocus                  = 1 << 17, // false     // [EXPERIMENTAL: Not very well specced] Clicking doesn't take focus. Automatically sets RblxUIButtonFlags_NoFocus + RblxUIButtonFlags_NoNavFocus in ButtonBehavior().

    // Controlled by widget code
    RblxUIItemFlags_Inputable                = 1 << 20, // false     // [WIP] Auto-activate input mode when tab focused. Currently only used and supported by a few items before it becomes a generic feature.
    RblxUIItemFlags_HasSelectionUserData     = 1 << 21, // false     // Set by SetNextItemSelectionUserData()
    RblxUIItemFlags_IsMultiSelect            = 1 << 22, // false     // Set by SetNextItemSelectionUserData()

    RblxUIItemFlags_Default_                 = RblxUIItemFlags_AutoClosePopups,    // Please don't change, use PushItemFlag() instead.

    // Obsolete
    //RblxUIItemFlags_SelectableDontClosePopup = !RblxUIItemFlags_AutoClosePopups, // Can't have a redirect as we inverted the behavior
};

// Status flags for an already submitted item
// - output: stored in g.LastItemData.StatusFlags
enum RblxUIItemStatusFlags_
{
    RblxUIItemStatusFlags_None               = 0,
    RblxUIItemStatusFlags_HoveredRect        = 1 << 0,   // Mouse position is within item rectangle (does NOT mean that the window is in correct z-order and can be hovered!, this is only one part of the most-common IsItemHovered test)
    RblxUIItemStatusFlags_HasDisplayRect     = 1 << 1,   // g.LastItemData.DisplayRect is valid
    RblxUIItemStatusFlags_Edited             = 1 << 2,   // Value exposed by item was edited in the current frame (should match the bool return value of most widgets)
    RblxUIItemStatusFlags_ToggledSelection   = 1 << 3,   // Set when Selectable(), TreeNode() reports toggling a selection. We can't report "Selected", only state changes, in order to easily handle clipping with less issues.
    RblxUIItemStatusFlags_ToggledOpen        = 1 << 4,   // Set when TreeNode() reports toggling their open state.
    RblxUIItemStatusFlags_HasDeactivated     = 1 << 5,   // Set if the widget/group is able to provide data for the RblxUIItemStatusFlags_Deactivated flag.
    RblxUIItemStatusFlags_Deactivated        = 1 << 6,   // Only valid if RblxUIItemStatusFlags_HasDeactivated is set.
    RblxUIItemStatusFlags_HoveredWindow      = 1 << 7,   // Override the HoveredWindow test to allow cross-window hover testing.
    RblxUIItemStatusFlags_Visible            = 1 << 8,   // [WIP] Set when item is overlapping the current clipping rectangle (Used internally. Please don't use yet: API/system will change as we refactor Itemadd()).
    RblxUIItemStatusFlags_HasClipRect        = 1 << 9,   // g.LastItemData.ClipRect is valid.
    RblxUIItemStatusFlags_HasShortcut        = 1 << 10,  // g.LastItemData.Shortcut valid. Set by SetNextItemShortcut() -> ItemAdd().
    //RblxUIItemStatusFlags_FocusedByTabbing = 1 << 8,   // Removed IN 1.90.1 (Dec 2023). The trigger is part of g.NavActivateId. See commit 54c1bdeceb.
    RblxUIItemStatusFlags_EditedInternal     = 1 << 11,  // Similar to RblxUIItemStatusFlags_Edited but bypassing RblxUIItemFlags_NoMarkEdited.

    // Additional status + semantic for RblxUITestEngine
#ifdef RBLXUI_ENABLE_TEST_ENGINE
    RblxUIItemStatusFlags_Openable           = 1 << 20,  // Item is an openable (e.g. TreeNode)
    RblxUIItemStatusFlags_Opened             = 1 << 21,  // Opened status
    RblxUIItemStatusFlags_Checkable          = 1 << 22,  // Item is a checkable (e.g. CheckBox, MenuItem)
    RblxUIItemStatusFlags_Checked            = 1 << 23,  // Checked status
    RblxUIItemStatusFlags_Inputable          = 1 << 24,  // Item is a text-inputable (e.g. InputText, SliderXXX, DragXXX)
#endif
};

// Extend RblxUIHoveredFlags_
enum RblxUIHoveredFlagsPrivate_
{
    RblxUIHoveredFlags_DelayMask_                    = RblxUIHoveredFlags_DelayNone | RblxUIHoveredFlags_DelayShort | RblxUIHoveredFlags_DelayNormal | RblxUIHoveredFlags_NoSharedDelay,
    RblxUIHoveredFlags_AllowedMaskForIsWindowHovered = RblxUIHoveredFlags_ChildWindows | RblxUIHoveredFlags_RootWindow | RblxUIHoveredFlags_AnyWindow | RblxUIHoveredFlags_NoPopupHierarchy | RblxUIHoveredFlags_DockHierarchy | RblxUIHoveredFlags_AllowWhenBlockedByPopup | RblxUIHoveredFlags_AllowWhenBlockedByActiveItem | RblxUIHoveredFlags_ForTooltip | RblxUIHoveredFlags_Stationary,
    RblxUIHoveredFlags_AllowedMaskForIsItemHovered   = RblxUIHoveredFlags_AllowWhenBlockedByPopup | RblxUIHoveredFlags_AllowWhenBlockedByActiveItem | RblxUIHoveredFlags_AllowWhenOverlapped | RblxUIHoveredFlags_AllowWhenDisabled | RblxUIHoveredFlags_NoNavOverride | RblxUIHoveredFlags_ForTooltip | RblxUIHoveredFlags_Stationary | RblxUIHoveredFlags_DelayMask_,
};

// Extend RblxUIInputTextFlags_
enum RblxUIInputTextFlagsPrivate_
{
    // [Internal]
    RblxUIInputTextFlags_Multiline           = 1 << 26,  // For internal use by InputTextMultiline()
    RblxUIInputTextFlags_TempInput           = 1 << 27,  // For internal use by TempInputText(), will skip calling ItemAdd(). Require bounding-box to strictly match.
    RblxUIInputTextFlags_LocalizeDecimalPoint= 1 << 28,  // For internal use by InputScalar() and TempInputScalar()
};

// Extend RblxUIButtonFlags_
enum RblxUIButtonFlagsPrivate_
{
    RblxUIButtonFlags_PressedOnClick         = 1 << 4,   // return true on click (mouse down event)
    RblxUIButtonFlags_PressedOnClickRelease  = 1 << 5,   // [Default] return true on click + release on same item <-- this is what the majority of Button are using
    RblxUIButtonFlags_PressedOnClickReleaseAnywhere = 1 << 6, // return true on click + release even if the release event is not done while hovering the item
    RblxUIButtonFlags_PressedOnRelease       = 1 << 7,   // return true on release (default requires click+release). Prior to 2026/03/20 this implied RblxUIButtonFlags_NoHoldingActiveId but they are separate now.
    RblxUIButtonFlags_PressedOnDoubleClick   = 1 << 8,   // return true on double-click (default requires click+release)
    RblxUIButtonFlags_PressedOnDragDropHold  = 1 << 9,   // return true when held into while we are drag and dropping another item (used by e.g. tree nodes, collapsing headers)
    //RblxUIButtonFlags_Repeat               = 1 << 10,  // hold to repeat -> use RblxUIItemFlags_ButtonRepeat instead.
    RblxUIButtonFlags_FlattenChildren        = 1 << 11,  // allow interactions even if a child window is overlapping
    //RblxUIButtonFlags_DontClosePopups      = 1 << 13,  // disable automatically closing parent popup on press
    //RblxUIButtonFlags_Disabled             = 1 << 14,  // disable interactions -> use BeginDisabled() or RblxUIItemFlags_Disabled
    RblxUIButtonFlags_AlignTextBaseLine      = 1 << 15,  // vertically align button to match text baseline - ButtonEx() only // FIXME: Should be removed and handled by SmallButton(), not possible currently because of DC.CursorPosPrevLine
    RblxUIButtonFlags_NoKeyModsAllowed       = 1 << 16,  // disable mouse interaction if a key modifier is held
    RblxUIButtonFlags_NoHoldingActiveId      = 1 << 17,  // don't set ActiveId while holding the mouse (RblxUIButtonFlags_PressedOnClick only)
    RblxUIButtonFlags_NoNavFocus             = 1 << 18,  // don't override navigation focus when activated (FIXME: this is essentially used every time an item uses RblxUIItemFlags_NoNav, but because legacy specs don't requires LastItemData to be set ButtonBehavior(), we can't poll g.LastItemData.ItemFlags)
    RblxUIButtonFlags_NoHoveredOnFocus       = 1 << 19,  // don't report as hovered when nav focus is on this item
    RblxUIButtonFlags_NoSetKeyOwner          = 1 << 20,  // don't set key/input owner on the initial click (note: mouse buttons are keys! often, the key in question will be RblxUIKey_MouseLeft!)
    RblxUIButtonFlags_NoTestKeyOwner         = 1 << 21,  // don't test key/input owner when polling the key (note: mouse buttons are keys! often, the key in question will be RblxUIKey_MouseLeft!)
    RblxUIButtonFlags_NoFocus                = 1 << 22,  // [EXPERIMENTAL: Not very well specced]. Don't focus parent window when clicking.
    RblxUIButtonFlags_PressedOnMask_         = RblxUIButtonFlags_PressedOnClick | RblxUIButtonFlags_PressedOnClickRelease | RblxUIButtonFlags_PressedOnClickReleaseAnywhere | RblxUIButtonFlags_PressedOnRelease | RblxUIButtonFlags_PressedOnDoubleClick | RblxUIButtonFlags_PressedOnDragDropHold,
    RblxUIButtonFlags_PressedOnDefault_      = RblxUIButtonFlags_PressedOnClickRelease,
    //RblxUIButtonFlags_NoKeyModifiers       = RblxUIButtonFlags_NoKeyModsAllowed, // Renamed in 1.91.4
};

// Extend RblxUIComboFlags_
enum RblxUIComboFlagsPrivate_
{
    RblxUIComboFlags_CustomPreview           = 1 << 20,  // enable BeginComboPreview()
};

// Extend RblxUISliderFlags_
enum RblxUISliderFlagsPrivate_
{
    RblxUISliderFlags_Vertical               = 1 << 20,  // Should this slider be orientated vertically?
    RblxUISliderFlags_ReadOnly               = 1 << 21,  // Consider using g.NextItemData.ItemFlags |= RblxUIItemFlags_ReadOnly instead.
};

// Extend RblxUISelectableFlags_
enum RblxUISelectableFlagsPrivate_
{
    // NB: need to be in sync with last value of RblxUISelectableFlags_
    RblxUISelectableFlags_NoHoldingActiveID      = 1 << 20,
    RblxUISelectableFlags_SelectOnClick          = 1 << 22,  // Override button behavior to react on Click (default is Click+Release)
    RblxUISelectableFlags_SelectOnRelease        = 1 << 23,  // Override button behavior to react on Release (default is Click+Release)
    RblxUISelectableFlags_SpanAvailWidth         = 1 << 24,  // Span all avail width even if we declared less for layout purpose. FIXME: We may be able to remove this (added in 6251d379, 2bcafc86 for menus)
    RblxUISelectableFlags_SetNavIdOnHover        = 1 << 25,  // Set Nav/Focus ID on mouse hover (used by MenuItem)
    RblxUISelectableFlags_NoPadWithHalfSpacing   = 1 << 26,  // Disable padding each side with ItemSpacing * 0.5f
    RblxUISelectableFlags_NoSetKeyOwner          = 1 << 27,  // Don't set key/input owner on the initial click (note: mouse buttons are keys! often, the key in question will be RblxUIKey_MouseLeft!)
};

// Extend RblxUITreeNodeFlags_
enum RblxUITreeNodeFlagsPrivate_
{
    RblxUITreeNodeFlags_NoNavFocus                 = 1 << 27,// Don't claim nav focus when interacting with this item (#8551)
    RblxUITreeNodeFlags_ClipLabelForTrailingButton = 1 << 28,// FIXME-WIP: Hard-coded for CollapsingHeader()
    RblxUITreeNodeFlags_UpsideDownArrow            = 1 << 29,// FIXME-WIP: Turn Down arrow into an Up arrow, for reversed trees (#6517)
    RblxUITreeNodeFlags_OpenOnMask_                = RblxUITreeNodeFlags_OpenOnDoubleClick | RblxUITreeNodeFlags_OpenOnArrow,
    RblxUITreeNodeFlags_DrawLinesMask_             = RblxUITreeNodeFlags_DrawLinesNone | RblxUITreeNodeFlags_DrawLinesFull | RblxUITreeNodeFlags_DrawLinesToNodes,
};

enum RblxUISeparatorFlags_
{
    RblxUISeparatorFlags_None                    = 0,
    RblxUISeparatorFlags_Horizontal              = 1 << 0,   // Axis default to current layout type, so generally Horizontal unless e.g. in a menu bar
    RblxUISeparatorFlags_Vertical                = 1 << 1,
    RblxUISeparatorFlags_SpanAllColumns          = 1 << 2,   // Make separator cover all columns of a legacy Columns() set.
};

// Flags for FocusWindow(). This is not called RblxUIFocusFlags to avoid confusion with public-facing RblxUIFocusedFlags.
// FIXME: Once we finishing replacing more uses of GetTopMostPopupModal()+IsWindowWithinBeginStackOf()
// and FindBlockingModal() with this, we may want to change the flag to be opt-out instead of opt-in.
enum RblxUIFocusRequestFlags_
{
    RblxUIFocusRequestFlags_None                 = 0,
    RblxUIFocusRequestFlags_RestoreFocusedChild  = 1 << 0,   // Find last focused child (if any) and focus it instead.
    RblxUIFocusRequestFlags_UnlessBelowModal     = 1 << 1,   // Do not set focus if the window is below a modal.
};

enum RblxUITextFlags_
{
    RblxUITextFlags_None                         = 0,
    RblxUITextFlags_NoWidthForLargeClippedText   = 1 << 0,
};

enum RblxUITooltipFlags_
{
    RblxUITooltipFlags_None                      = 0,
    RblxUITooltipFlags_OverridePrevious          = 1 << 1,   // Clear/ignore previously submitted tooltip (defaults to append)
};

// FIXME: this is in development, not exposed/functional as a generic feature yet.
// Horizontal/Vertical enums are fixed to 0/1 so they may be used to index RblxVec2
enum RblxUILayoutType_
{
    RblxUILayoutType_Horizontal = 0,
    RblxUILayoutType_Vertical = 1
};

// Flags for LogBegin() text capturing function
enum RblxUILogFlags_
{
    RblxUILogFlags_None = 0,

    RblxUILogFlags_OutputTTY         = 1 << 0,
    RblxUILogFlags_OutputFile        = 1 << 1,
    RblxUILogFlags_OutputBuffer      = 1 << 2,
    RblxUILogFlags_OutputClipboard   = 1 << 3,
    RblxUILogFlags_OutputMask_       = RblxUILogFlags_OutputTTY | RblxUILogFlags_OutputFile | RblxUILogFlags_OutputBuffer | RblxUILogFlags_OutputClipboard,
};

// X/Y enums are fixed to 0/1 so they may be used to index RblxVec2
enum RblxUIAxis
{
    RblxUIAxis_None = -1,
    RblxUIAxis_X = 0,
    RblxUIAxis_Y = 1
};

enum RblxUIPlotType
{
    RblxUIPlotType_Lines,
    RblxUIPlotType_Histogram,
};

// Storage data for BeginComboPreview()/EndComboPreview()
struct RBLXUI_API RblxUIComboPreviewData
{
    RblxRect          PreviewRect;
    RblxVec2          BackupCursorPos;
    RblxVec2          BackupCursorMaxPos;
    RblxVec2          BackupCursorPosPrevLine;
    float           BackupPrevLineTextBaseOffset;
    RblxUILayoutType BackupLayout;

    RblxUIComboPreviewData() { memset((void*)this, 0, sizeof(*this)); }
};

// Stacked storage data for BeginGroup()/EndGroup()
struct RBLXUI_API RblxUIGroupData
{
    RblxUIID     WindowID;
    RblxVec2      BackupCursorPos;
    RblxVec2      BackupCursorMaxPos;
    RblxVec2      BackupCursorPosPrevLine;
    RblxVec1      BackupIndent;
    RblxVec1      BackupGroupOffset;
    RblxVec2      BackupCurrLineSize;
    float       BackupCurrLineTextBaseOffset;
    RblxUIID     BackupActiveIdIsAlive;
    bool        BackupActiveIdHasBeenEditedThisFrame;
    bool        BackupDeactivatedIdIsAlive;
    bool        BackupHoveredIdIsAlive;
    bool        BackupIsSameLine;
    bool        EmitItem;
};

// Simple column measurement, currently used for MenuItem() only.. This is very short-sighted/throw-away code and NOT a generic helper.
struct RBLXUI_API RblxUIMenuColumns
{
    RblxU32       TotalWidth;
    RblxU32       NextTotalWidth;
    RblxU16       Spacing;
    RblxU16       OffsetIcon;         // Always zero for now
    RblxU16       OffsetLabel;        // Offsets are locked in Update()
    RblxU16       OffsetShortcut;
    RblxU16       OffsetMark;
    RblxU16       Widths[4];          // Width of:   Icon, Label, Shortcut, Mark  (accumulators for current frame)

    RblxUIMenuColumns() { memset((void*)this, 0, sizeof(*this)); }
    void        Update(float spacing, bool window_reappearing);
    float       DeclColumns(float w_icon, float w_label, float w_shortcut, float w_mark);
    void        CalcNextTotalWidth(bool update_offsets);
};

// Internal temporary state for deactivating InputText() instances.
// Store as part of RblxUIDeactivatedItemData?
struct RBLXUI_API RblxUIInputTextDeactivatedState
{
    RblxUIID            ID;              // widget id owning the text state (which just got deactivated)
    RblxVector<char>     TextA;           // text buffer

    RblxUIInputTextDeactivatedState()    { memset((void*)this, 0, sizeof(*this)); }
    void    ClearFreeMemory()           { ID = 0; TextA.clear(); }
};

// Forward declare rblxuistb_textedit.h structure + make its main configuration define accessible
#undef IMSTB_TEXTEDIT_STRING
#undef IMSTB_TEXTEDIT_CHARTYPE
#define IMSTB_TEXTEDIT_STRING             RblxUIInputTextState
#define IMSTB_TEXTEDIT_CHARTYPE           char
#define IMSTB_TEXTEDIT_GETWIDTH_NEWLINE   (-1.0f)
#define IMSTB_TEXTEDIT_UNDOSTATECOUNT     99
#define IMSTB_TEXTEDIT_UNDOCHARCOUNT      999
namespace RblxStb { struct RBLXSTB_TexteditState; }
typedef RblxStb::RBLXSTB_TexteditState RblxStbTexteditState;

// Internal state of the currently focused/edited text input box
// For a given item ID, access with RblxUI::GetInputTextState()
struct RBLXUI_API RblxUIInputTextState
{
    RblxUIContext*           Ctx;                    // parent UI context (needs to be set explicitly by parent).
    RblxStbTexteditState*     Stb;                    // State for rblxstb_textedit.h
    RblxUIInputTextFlags     Flags;                  // copy of InputText() flags. may be used to check if e.g. RblxUIInputTextFlags_Password is set.
    RblxUIID                 ID;                     // widget id owning the text state
    int                     TextLen;                // UTF-8 length of the string in TextA (in bytes)
    const char*             TextSrc;                // == TextA.Data unless read-only, in which case == buf passed to InputText(). For _ReadOnly fields, pointer will be null outside the InputText() call.
    RblxVector<char>          TextA;                  // main UTF8 buffer. TextA.Size is a buffer size! Should always be >= buf_size passed by user (and of course >= CurLenA + 1).
    RblxVector<char>          TextToRevertTo;         // value to revert to when pressing Escape = backup of end-user buffer at the time of focus (in UTF-8, unaltered)
    RblxVector<char>          CallbackTextBackup;     // temporary storage for callback to support automatic reconcile of undo-stack
    int                     BufCapacity;            // end-user buffer capacity (include zero terminator)
    RblxVec2                  Scroll;                 // horizontal offset (managed manually) + vertical scrolling (pulled from child window's own Scroll.y)
    int                     LineCount;              // last line count (solely for debugging)
    float                   WrapWidth;              // word-wrapping width
    float                   CursorAnim;             // timer for cursor blink, reset on every user action so the cursor reappears immediately
    bool                    CursorFollow;           // set when we want scrolling to follow the current cursor position (not always!)
    bool                    CursorCenterY;          // set when we want scrolling to be centered over the cursor position (while resizing a word-wrapping field)
    bool                    SelectedAllMouseLock;   // after a double-click to select all, we ignore further mouse drags to update selection
    bool                    EditedBefore;           // edited since activated
    bool                    EditedThisFrame;        // edited this frame
    bool                    WantReloadUserBuf;      // force a reload of user buf so it may be modified externally. may be automatic in future version.
    RblxS8                    LastMoveDirectionLR;    // RblxUIDir_Left or RblxUIDir_Right. track last movement direction so when cursor cross over a word-wrapping boundaries we can display it on either line depending on last move.s
    int                     ReloadSelectionStart;
    int                     ReloadSelectionEnd;

    RblxUIInputTextState();
    ~RblxUIInputTextState();
    void        ClearText()                 { TextLen = 0; TextA[0] = 0; CursorClamp(); }
    void        ClearFreeMemory()           { TextA.clear(); TextToRevertTo.clear(); }
    void        OnKeyPressed(int key);      // Cannot be inline because we call in code in rblxstb_textedit.h implementation
    void        OnCharPressed(unsigned int c);
    float       GetPreferredOffsetX() const;
    const char* GetText()                   { return TextA.Data ? TextA.Data : ""; }

    // Cursor & Selection
    void        CursorAnimReset();
    void        CursorClamp();
    bool        HasSelection() const;
    void        ClearSelection();
    int         GetCursorPos() const;
    int         GetSelectionStart() const;
    int         GetSelectionEnd() const;
    void        SetSelection(int start, int end);
    void        SelectAll();

    // Reload user buf (WIP #2890)
    // If you modify underlying user-passed const char* while active you need to call this (InputText V2 may lift this)
    //   strcpy(my_buf, "hello");
    //   if (RblxUIInputTextState* state = RblxUI::GetInputTextState(id)) // id may be RblxUI::GetItemID() is last item
    //       state->ReloadUserBufAndSelectAll();
    void        ReloadUserBufAndSelectAll();
    void        ReloadUserBufAndKeepSelection();
    void        ReloadUserBufAndMoveToEnd();
};

enum RblxUIWindowRefreshFlags_
{
    RblxUIWindowRefreshFlags_None                = 0,
    RblxUIWindowRefreshFlags_TryToAvoidRefresh   = 1 << 0,   // [EXPERIMENTAL] Try to keep existing contents, USER MUST NOT HONOR BEGIN() RETURNING FALSE AND NOT APPEND.
    RblxUIWindowRefreshFlags_RefreshOnHover      = 1 << 1,   // [EXPERIMENTAL] Always refresh on hover
    RblxUIWindowRefreshFlags_RefreshOnFocus      = 1 << 2,   // [EXPERIMENTAL] Always refresh on focus
    // Refresh policy/frequency, Load Balancing etc.
};

enum RblxUIWindowBgClickFlags_
{
    RblxUIWindowBgClickFlags_None                = 0,
    RblxUIWindowBgClickFlags_Move                = 1 << 0,   // Click on bg/void + drag to move window. Cleared by default when using io.ConfigWindowsMoveFromTitleBarOnly.
};

enum RblxUINextWindowDataFlags_
{
    RblxUINextWindowDataFlags_None               = 0,
    RblxUINextWindowDataFlags_HasPos             = 1 << 0,
    RblxUINextWindowDataFlags_HasSize            = 1 << 1,
    RblxUINextWindowDataFlags_HasContentSize     = 1 << 2,
    RblxUINextWindowDataFlags_HasCollapsed       = 1 << 3,
    RblxUINextWindowDataFlags_HasSizeConstraint  = 1 << 4,
    RblxUINextWindowDataFlags_HasFocus           = 1 << 5,
    RblxUINextWindowDataFlags_HasBgAlpha         = 1 << 6,
    RblxUINextWindowDataFlags_HasScroll          = 1 << 7,
    RblxUINextWindowDataFlags_HasWindowFlags     = 1 << 8,
    RblxUINextWindowDataFlags_HasChildFlags      = 1 << 9,
    RblxUINextWindowDataFlags_HasRefreshPolicy   = 1 << 10,
    RblxUINextWindowDataFlags_HasViewport        = 1 << 11,
    RblxUINextWindowDataFlags_HasDock            = 1 << 12,
    RblxUINextWindowDataFlags_HasWindowClass     = 1 << 13,
};

// Storage for SetNexWindow** functions
struct RblxUINextWindowData
{
    RblxUINextWindowDataFlags    HasFlags;

    // Members below are NOT cleared. Always rely on HasFlags.
    RblxUICond                   PosCond;
    RblxUICond                   SizeCond;
    RblxUICond                   CollapsedCond;
    RblxUICond                   DockCond;
    RblxVec2                      PosVal;
    RblxVec2                      PosPivotVal;
    RblxVec2                      SizeVal;
    RblxVec2                      ContentSizeVal;
    RblxVec2                      ScrollVal;
    RblxUIWindowFlags            WindowFlags;            // Only honored by BeginTable()
    RblxUIChildFlags             ChildFlags;
    bool                        PosUndock;
    bool                        CollapsedVal;
    RblxRect                      SizeConstraintRect;
    RblxUISizeCallback           SizeCallback;
    void*                       SizeCallbackUserData;
    float                       BgAlphaVal;             // Override background alpha
    RblxUIID                     ViewportId;
    RblxUIID                     DockId;
    RblxUIWindowClass            WindowClass;
    RblxVec2                      MenuBarOffsetMinVal;    // (Always on) This is not exposed publicly, so we don't clear it and it doesn't have a corresponding flag (could we? for consistency?)
    RblxUIWindowRefreshFlags     RefreshFlagsVal;

    RblxUINextWindowData()       { memset((void*)this, 0, sizeof(*this)); }
    inline void ClearFlags()    { HasFlags = RblxUINextWindowDataFlags_None; }
};

enum RblxUINextItemDataFlags_
{
    RblxUINextItemDataFlags_None             = 0,
    RblxUINextItemDataFlags_HasWidth         = 1 << 0,
    RblxUINextItemDataFlags_HasOpen          = 1 << 1,
    RblxUINextItemDataFlags_HasShortcut      = 1 << 2,
    RblxUINextItemDataFlags_HasRefVal        = 1 << 3,
    RblxUINextItemDataFlags_HasStorageID     = 1 << 4,
    RblxUINextItemDataFlags_HasColorMarker   = 1 << 5,
};

struct RblxUINextItemData
{
    RblxUINextItemDataFlags      HasFlags;           // Called HasFlags instead of Flags to avoid mistaking this
    RblxUIItemFlags              ItemFlags;          // Currently only tested/used for RblxUIItemFlags_AllowOverlap and RblxUIItemFlags_HasSelectionUserData.

    // Members below are NOT cleared by ItemAdd() meaning they are still valid during e.g. NavProcessItem(). Always rely on HasFlags.
    RblxUIID                     FocusScopeId;       // Set by SetNextItemSelectionUserData()
    RblxUISelectionUserData      SelectionUserData;  // Set by SetNextItemSelectionUserData() (note that NULL/0 is a valid value, we use -1 == RblxUISelectionUserData_Invalid to mark invalid values)
    float                       Width;              // Set by SetNextItemWidth()
    RblxUIKeyChord               Shortcut;           // Set by SetNextItemShortcut()
    RblxUIInputFlags             ShortcutFlags;      // Set by SetNextItemShortcut()
    bool                        OpenVal;            // Set by SetNextItemOpen()
    RblxU8                        OpenCond;           // Set by SetNextItemOpen()
    RblxUIDataTypeStorage        RefVal;             // Not exposed yet, for RblxUIInputTextFlags_ParseEmptyAsRefVal
    RblxUIID                     StorageId;          // Set by SetNextItemStorageID()
    RblxU32                       ColorMarker;        // Set by SetNextItemColorMarker(). Not exposed yet, supported by DragScalar,SliderScalar and for RblxUISliderFlags_ColorMarkers.

    RblxUINextItemData()         { memset((void*)this, 0, sizeof(*this)); SelectionUserData = -1; }
    inline void ClearFlags()    { HasFlags = RblxUINextItemDataFlags_None; ItemFlags = RblxUIItemFlags_None; } // Also cleared manually by ItemAdd()!
};

// Status storage for the last submitted item
struct RblxUILastItemData
{
    RblxUIID                 ID;
    RblxUIItemFlags          ItemFlags;          // See RblxUIItemFlags_ (called 'InFlags' before v1.91.4).
    RblxUIItemStatusFlags    StatusFlags;        // See RblxUIItemStatusFlags_
    RblxRect                  Rect;               // Full rectangle
    RblxRect                  NavRect;            // Navigation scoring rectangle (not displayed)
    // Rarely used fields are not explicitly cleared, only valid when the corresponding RblxUIItemStatusFlags are set.
    RblxRect                  DisplayRect;        // Display rectangle. ONLY VALID IF (StatusFlags & RblxUIItemStatusFlags_HasDisplayRect) is set.
    RblxRect                  ClipRect;           // Clip rectangle at the time of submitting item. ONLY VALID IF (StatusFlags & RblxUIItemStatusFlags_HasClipRect) is set..
    RblxUIKeyChord           Shortcut;           // Shortcut at the time of submitting item. ONLY VALID IF (StatusFlags & RblxUIItemStatusFlags_HasShortcut) is set..

    RblxUILastItemData()     { memset((void*)this, 0, sizeof(*this)); }
};

// Store data emitted by TreeNode() for usage by TreePop()
// - To implement RblxUITreeNodeFlags_NavLeftJumpsToParent: store the minimum amount of data
//   which we can't infer in TreePop(), to perform the equivalent of NavApplyItemToResult().
//   Only stored when the node is a potential candidate for landing on a Left arrow jump.
struct RblxUITreeNodeStackData
{
    RblxUIID                 ID;
    RblxUITreeNodeFlags      TreeFlags;
    RblxUIItemFlags          ItemFlags;      // Used for nav landing
    RblxRect                  NavRect;        // Used for nav landing
    float                   DrawLinesX1;
    float                   DrawLinesToNodesY2;
    RblxUITableColumnIdx     DrawLinesTableColumn;
};

// sizeof() = 20
struct RBLXUI_API RblxUIErrorRecoveryState
{
    short   SizeOfWindowStack;
    short   SizeOfIDStack;
    short   SizeOfTreeStack;
    short   SizeOfColorStack;
    short   SizeOfStyleVarStack;
    short   SizeOfFontStack;
    short   SizeOfFocusScopeStack;
    short   SizeOfGroupStack;
    short   SizeOfItemFlagsStack;
    short   SizeOfBeginPopupStack;
    short   SizeOfDisabledStack;

    RblxUIErrorRecoveryState() { memset((void*)this, 0, sizeof(*this)); }
};

// Data saved for each window pushed into the stack
struct RblxUIWindowStackData
{
    RblxUIWindow*            Window;
    RblxUILastItemData       ParentLastItemDataBackup;
    RblxUIErrorRecoveryState StackSizesInBegin;          // Store size of various stacks for asserting
    bool                    DisabledOverrideReenable;   // Non-child window override disabled flag
    float                   DisabledOverrideReenableAlphaBackup;
};

struct RblxUIShrinkWidthItem
{
    int         Index;
    float       Width;
    float       InitialWidth;
};

struct RblxUIPtrOrIndex
{
    void*       Ptr;            // Either field can be set, not both. e.g. Dock node tab bars are loose while BeginTabBar() ones are in a pool.
    int         Index;          // Usually index in a main pool.

    RblxUIPtrOrIndex(void* ptr)  { Ptr = ptr; Index = -1; }
    RblxUIPtrOrIndex(int index)  { Ptr = NULL; Index = index; }
};

// Data used by IsItemDeactivated()/IsItemDeactivatedAfterEdit() functions
// Also see RblxUIInputTextDeactivatedState which is an extension for this for InputText()
struct RblxUIDeactivatedItemData
{
    RblxUIID     ID;
    int         ElapseFrame;
    bool        HasBeenEditedBefore;
    bool        IsAlive;
};

//-----------------------------------------------------------------------------
// [SECTION] Popup support
//-----------------------------------------------------------------------------

enum RblxUIPopupPositionPolicy
{
    RblxUIPopupPositionPolicy_Default,
    RblxUIPopupPositionPolicy_ComboBox,
    RblxUIPopupPositionPolicy_Tooltip,
};

// Storage for popup stacks (g.OpenPopupStack and g.BeginPopupStack)
struct RblxUIPopupData
{
    RblxUIID             PopupId;        // Set on OpenPopup()
    RblxUIWindow*        Window;         // Resolved on BeginPopup() - may stay unresolved if user never calls OpenPopup()
    RblxUIWindow*        RestoreNavWindow;// Set on OpenPopup(), a NavWindow that will be restored on popup close
    int                 ParentNavLayer; // Resolved on BeginPopup(). Actually a RblxUINavLayer type (declared down below), initialized to -1 which is not part of an enum, but serves well-enough as "not any of layers" value
    int                 OpenFrameCount; // Set on OpenPopup()
    RblxUIID             OpenParentId;   // Set on OpenPopup(), we need this to differentiate multiple menu sets from each others (e.g. inside menu bar vs loose menu items)
    RblxVec2              OpenPopupPos;   // Set on OpenPopup(), preferred popup position (typically == OpenMousePos when using mouse)
    RblxVec2              OpenMousePos;   // Set on OpenPopup(), copy of mouse position at the time of opening popup

    RblxUIPopupData()    { memset((void*)this, 0, sizeof(*this)); ParentNavLayer = OpenFrameCount = -1; }
};

//-----------------------------------------------------------------------------
// [SECTION] Inputs support
//-----------------------------------------------------------------------------

// Bit array for named keys
typedef RblxBitArray<RblxUIKey_NamedKey_COUNT, -RblxUIKey_NamedKey_BEGIN>    RblxBitArrayForNamedKeys;

// [Internal] Key ranges
#define RblxUIKey_LegacyNativeKey_BEGIN  0
#define RblxUIKey_LegacyNativeKey_END    512
#define RblxUIKey_Keyboard_BEGIN         (RblxUIKey_NamedKey_BEGIN)
#define RblxUIKey_Keyboard_END           (RblxUIKey_GamepadStart)
#define RblxUIKey_Gamepad_BEGIN          (RblxUIKey_GamepadStart)
#define RblxUIKey_Gamepad_END            (RblxUIKey_GamepadRStickDown + 1)
#define RblxUIKey_Mouse_BEGIN            (RblxUIKey_MouseLeft)
#define RblxUIKey_Mouse_END              (RblxUIKey_MouseWheelY + 1)
#define RblxUIKey_Aliases_BEGIN          (RblxUIKey_Mouse_BEGIN)
#define RblxUIKey_Aliases_END            (RblxUIKey_Mouse_END)

// [Internal] Named shortcuts for Navigation
#define RblxUIKey_NavKeyboardTweakSlow   RblxUIMod_Ctrl
#define RblxUIKey_NavKeyboardTweakFast   RblxUIMod_Shift
#define RblxUIKey_NavGamepadTweakSlow    RblxUIKey_GamepadL1
#define RblxUIKey_NavGamepadTweakFast    RblxUIKey_GamepadR1
#define RblxUIKey_NavGamepadActivate     (g.IO.ConfigNavSwapGamepadButtons ? RblxUIKey_GamepadFaceRight : RblxUIKey_GamepadFaceDown)
#define RblxUIKey_NavGamepadCancel       (g.IO.ConfigNavSwapGamepadButtons ? RblxUIKey_GamepadFaceDown : RblxUIKey_GamepadFaceRight)
#define RblxUIKey_NavGamepadMenu         RblxUIKey_GamepadFaceLeft    // Toggle menu layer. Hold to enable Windowing.
#define RblxUIKey_NavGamepadContextMenu  RblxUIKey_GamepadFaceUp      // Open context menu (same as Shift+F10)

enum RblxUIInputEventType
{
    RblxUIInputEventType_None = 0,
    RblxUIInputEventType_MousePos,
    RblxUIInputEventType_MouseWheel,
    RblxUIInputEventType_MouseButton,
    RblxUIInputEventType_MouseViewport,
    RblxUIInputEventType_Key,
    RblxUIInputEventType_Text,
    RblxUIInputEventType_Focus,
    RblxUIInputEventType_COUNT
};

enum RblxUIInputSource : int
{
    RblxUIInputSource_None = 0,
    RblxUIInputSource_Mouse,         // Note: may be Mouse or TouchScreen or Pen. See io.MouseSource to distinguish them.
    RblxUIInputSource_Keyboard,
    RblxUIInputSource_Gamepad,
    RblxUIInputSource_COUNT
};

// FIXME: Structures in the union below need to be declared as anonymous unions appears to be an extension?
// Using RblxVec2() would fail on Clang 'union member 'MousePos' has a non-trivial default constructor'
struct RblxUIInputEventMousePos      { float PosX, PosY; RblxUIMouseSource MouseSource; };
struct RblxUIInputEventMouseWheel    { float WheelX, WheelY; RblxUIMouseSource MouseSource; };
struct RblxUIInputEventMouseButton   { int Button; bool Down; RblxUIMouseSource MouseSource; };
struct RblxUIInputEventMouseViewport { RblxUIID HoveredViewportID; };
struct RblxUIInputEventKey           { RblxUIKey Key; bool Down; float AnalogValue; };
struct RblxUIInputEventText          { unsigned int Char; };
struct RblxUIInputEventAppFocused    { bool Focused; };

struct RblxUIInputEvent
{
    RblxUIInputEventType             Type;
    RblxUIInputSource                Source;
    RblxU32                           EventId;        // Unique, sequential increasing integer to identify an event (if you need to correlate them to other data).
    union
    {
        RblxUIInputEventMousePos     MousePos;       // if Type == RblxUIInputEventType_MousePos
        RblxUIInputEventMouseWheel   MouseWheel;     // if Type == RblxUIInputEventType_MouseWheel
        RblxUIInputEventMouseButton  MouseButton;    // if Type == RblxUIInputEventType_MouseButton
        RblxUIInputEventMouseViewport MouseViewport; // if Type == RblxUIInputEventType_MouseViewport
        RblxUIInputEventKey          Key;            // if Type == RblxUIInputEventType_Key
        RblxUIInputEventText         Text;           // if Type == RblxUIInputEventType_Text
        RblxUIInputEventAppFocused   AppFocused;     // if Type == RblxUIInputEventType_Focus
    };
    bool                            AddedByTestEngine;

    RblxUIInputEvent() { memset((void*)this, 0, sizeof(*this)); }
};

// Input function taking an 'RblxUIID owner_id' argument defaults to (RblxUIKeyOwner_Any == 0) aka don't test ownership, which matches legacy behavior.
#define RblxUIKeyOwner_Any           ((RblxUIID)0)    // Accept key that have an owner, UNLESS a call to SetKeyOwner() explicitly used RblxUIInputFlags_LockThisFrame or RblxUIInputFlags_LockUntilRelease.
#define RblxUIKeyOwner_NoOwner       ((RblxUIID)-1)   // Require key to have no owner.
//#define RblxUIKeyOwner_None RblxUIKeyOwner_NoOwner  // We previously called this 'RblxUIKeyOwner_None' but it was inconsistent with our pattern that _None values == 0 and quite dangerous. Also using _NoOwner makes the IsKeyPressed() calls more explicit.

typedef RblxS16 RblxUIKeyRoutingIndex;

// Routing table entry (sizeof() == 16 bytes)
struct RblxUIKeyRoutingData
{
    RblxUIKeyRoutingIndex            NextEntryIndex;
    RblxU16                           Mods;               // Technically we'd only need 4-bits but for simplify we store RblxUIMod_ values which need 16-bits.
    RblxU16                           RoutingCurrScore;   // [DEBUG] For debug display
    RblxU16                           RoutingNextScore;   // Lower is better (0: perfect score)
    RblxUIID                         RoutingCurr;
    RblxUIID                         RoutingNext;

    RblxUIKeyRoutingData()           { NextEntryIndex = -1; Mods = 0; RoutingCurrScore = RoutingNextScore = 0; RoutingCurr = RoutingNext = RblxUIKeyOwner_NoOwner; }
};

// Routing table: maintain a desired owner for each possible key-chord (key + mods), and setup owner in NewFrame() when mods are matching.
// Stored in main context (1 instance)
struct RblxUIKeyRoutingTable
{
    RblxUIKeyRoutingIndex            Index[RblxUIKey_NamedKey_COUNT]; // Index of first entry in Entries[]
    RblxVector<RblxUIKeyRoutingData>   Entries;
    RblxVector<RblxUIKeyRoutingData>   EntriesNext;                    // Double-buffer to avoid reallocation (could use a shared buffer)

    RblxUIKeyRoutingTable()          { Clear(); }
    void Clear()                    { for (int n = 0; n < RBLXUI_COUNTOF(Index); n++) Index[n] = -1; Entries.clear(); EntriesNext.clear(); }
};

// This extends RblxUIKeyData but only for named keys (legacy keys don't support the new features)
// Stored in main context (1 per named key). In the future it might be merged into RblxUIKeyData.
struct RblxUIKeyOwnerData
{
    RblxUIID     OwnerCurr;
    RblxUIID     OwnerNext;
    bool        LockThisFrame;      // Reading this key requires explicit owner id (until end of frame). Set by RblxUIInputFlags_LockThisFrame.
    bool        LockUntilRelease;   // Reading this key requires explicit owner id (until key is released). Set by RblxUIInputFlags_LockUntilRelease. When this is true LockThisFrame is always true as well.

    RblxUIKeyOwnerData()             { OwnerCurr = OwnerNext = RblxUIKeyOwner_NoOwner; LockThisFrame = LockUntilRelease = false; }
};

// Extend RblxUIInputFlags_
// Flags for extended versions of IsKeyPressed(), IsMouseClicked(), Shortcut(), SetKeyOwner(), SetItemKeyOwner()
// Don't mistake with RblxUIInputTextFlags! (which is for RblxUI::InputText() function)
enum RblxUIInputFlagsPrivate_
{
    // Flags for IsKeyPressed(), IsKeyChordPressed(), IsMouseClicked(), Shortcut()
    // - Repeat mode: Repeat rate selection
    RblxUIInputFlags_RepeatRateDefault           = 1 << 1,   // Repeat rate: Regular (default)
    RblxUIInputFlags_RepeatRateNavMove           = 1 << 2,   // Repeat rate: Fast
    RblxUIInputFlags_RepeatRateNavTweak          = 1 << 3,   // Repeat rate: Faster
    // - Repeat mode: Specify when repeating key pressed can be interrupted.
    // - In theory RblxUIInputFlags_RepeatUntilOtherKeyPress may be a desirable default, but it would break too many behavior so everything is opt-in.
    RblxUIInputFlags_RepeatUntilRelease          = 1 << 4,   // Stop repeating when released (default for all functions except Shortcut). This only exists to allow overriding Shortcut() default behavior.
    RblxUIInputFlags_RepeatUntilKeyModsChange    = 1 << 5,   // Stop repeating when released OR if keyboard mods are changed (default for Shortcut)
    RblxUIInputFlags_RepeatUntilKeyModsChangeFromNone = 1 << 6,  // Stop repeating when released OR if keyboard mods are leaving the None state. Allows going from Mod+Key to Key by releasing Mod.
    RblxUIInputFlags_RepeatUntilOtherKeyPress    = 1 << 7,   // Stop repeating when released OR if any other keyboard key is pressed during the repeat

    // Flags for SetKeyOwner(), SetItemKeyOwner()
    // - Locking key away from non-input aware code. Locking is useful to make input-owner-aware code steal keys from non-input-owner-aware code. If all code is input-owner-aware locking would never be necessary.
    RblxUIInputFlags_LockThisFrame               = 1 << 20,  // Further accesses to key data will require EXPLICIT owner ID (RblxUIKeyOwner_Any/0 will NOT accepted for polling). Cleared at end of frame.
    RblxUIInputFlags_LockUntilRelease            = 1 << 21,  // Further accesses to key data will require EXPLICIT owner ID (RblxUIKeyOwner_Any/0 will NOT accepted for polling). Cleared when the key is released or at end of each frame if key is released.

    // - Condition for SetItemKeyOwner()
    RblxUIInputFlags_CondHovered                 = 1 << 22,  // Only set if item is hovered (default to both)
    RblxUIInputFlags_CondActive                  = 1 << 23,  // Only set if item is active (default to both)
    RblxUIInputFlags_CondDefault_                = RblxUIInputFlags_CondHovered | RblxUIInputFlags_CondActive,

    // [Internal] Mask of which function support which flags
    RblxUIInputFlags_RepeatRateMask_             = RblxUIInputFlags_RepeatRateDefault | RblxUIInputFlags_RepeatRateNavMove | RblxUIInputFlags_RepeatRateNavTweak,
    RblxUIInputFlags_RepeatUntilMask_            = RblxUIInputFlags_RepeatUntilRelease | RblxUIInputFlags_RepeatUntilKeyModsChange | RblxUIInputFlags_RepeatUntilKeyModsChangeFromNone | RblxUIInputFlags_RepeatUntilOtherKeyPress,
    RblxUIInputFlags_RepeatMask_                 = RblxUIInputFlags_Repeat | RblxUIInputFlags_RepeatRateMask_ | RblxUIInputFlags_RepeatUntilMask_,
    RblxUIInputFlags_CondMask_                   = RblxUIInputFlags_CondHovered | RblxUIInputFlags_CondActive,
    RblxUIInputFlags_RouteTypeMask_              = RblxUIInputFlags_RouteActive | RblxUIInputFlags_RouteFocused | RblxUIInputFlags_RouteGlobal | RblxUIInputFlags_RouteAlways,
    RblxUIInputFlags_RouteOptionsMask_           = RblxUIInputFlags_RouteOverFocused | RblxUIInputFlags_RouteOverActive | RblxUIInputFlags_RouteUnlessBgFocused | RblxUIInputFlags_RouteFromRootWindow,
    RblxUIInputFlags_SupportedByIsKeyPressed     = RblxUIInputFlags_RepeatMask_,
    RblxUIInputFlags_SupportedByIsMouseClicked   = RblxUIInputFlags_Repeat,
    RblxUIInputFlags_SupportedByShortcut         = RblxUIInputFlags_RepeatMask_ | RblxUIInputFlags_RouteTypeMask_ | RblxUIInputFlags_RouteOptionsMask_,
    RblxUIInputFlags_SupportedBySetNextItemShortcut = RblxUIInputFlags_RepeatMask_ | RblxUIInputFlags_RouteTypeMask_ | RblxUIInputFlags_RouteOptionsMask_ | RblxUIInputFlags_Tooltip,
    RblxUIInputFlags_SupportedBySetKeyOwner      = RblxUIInputFlags_LockThisFrame | RblxUIInputFlags_LockUntilRelease,
    RblxUIInputFlags_SupportedBySetItemKeyOwner  = RblxUIInputFlags_SupportedBySetKeyOwner | RblxUIInputFlags_CondMask_,
};

//-----------------------------------------------------------------------------
// [SECTION] Clipper support
//-----------------------------------------------------------------------------

// Note that Max is exclusive, so perhaps should be using a Begin/End convention.
struct RblxUIListClipperRange
{
    int     Min;
    int     Max;
    bool    PosToIndexConvert;      // Begin/End are absolute position (will be converted to indices later)
    RblxS8    PosToIndexOffsetMin;    // Add to Min after converting to indices
    RblxS8    PosToIndexOffsetMax;    // Add to Min after converting to indices

    static RblxUIListClipperRange    FromIndices(int min, int max)                               { RblxUIListClipperRange r = { min, max, false, 0, 0 }; return r; }
    static RblxUIListClipperRange    FromPositions(float y1, float y2, int off_min, int off_max) { RblxUIListClipperRange r = { (int)y1, (int)y2, true, (RblxS8)off_min, (RblxS8)off_max }; return r; }
};

// Temporary clipper data, buffers shared/reused between instances
struct RblxUIListClipperData
{
    RblxUIListClipper*               ListClipper;
    float                           LossynessOffset;
    int                             StepNo;
    int                             ItemsFrozen;
    RblxVector<RblxUIListClipperRange> Ranges;

    RblxUIListClipperData()          { memset((void*)this, 0, sizeof(*this)); }
    void                            Reset(RblxUIListClipper* clipper) { ListClipper = clipper; StepNo = ItemsFrozen = 0; Ranges.resize(0); }
};

//-----------------------------------------------------------------------------
// [SECTION] Navigation support
//-----------------------------------------------------------------------------

enum RblxUIActivateFlags_
{
    RblxUIActivateFlags_None                 = 0,
    RblxUIActivateFlags_PreferInput          = 1 << 0,       // Favor activation that requires keyboard text input (e.g. for Slider/Drag). Default for Enter key.
    RblxUIActivateFlags_PreferTweak          = 1 << 1,       // Favor activation for tweaking with arrows or gamepad (e.g. for Slider/Drag). Default for Space key and if keyboard is not used.
    RblxUIActivateFlags_TryToPreserveState   = 1 << 2,       // Request widget to preserve state if it can (e.g. InputText will try to preserve cursor/selection)
    RblxUIActivateFlags_FromTabbing          = 1 << 3,       // Activation requested by a tabbing request (RblxUINavMoveFlags_IsTabbing)
    RblxUIActivateFlags_FromShortcut         = 1 << 4,       // Activation requested by an item shortcut via SetNextItemShortcut() function.
    RblxUIActivateFlags_FromFocusApi         = 1 << 5,       // Activation requested by an api request (RblxUINavMoveFlags_FocusApi)
};

// Early work-in-progress API for ScrollToItem()
// FIXME: Missing flags to request making both edges visible when possible.
enum RblxUIScrollFlags_
{
    RblxUIScrollFlags_None                   = 0,
    RblxUIScrollFlags_KeepVisibleEdgeX       = 1 << 0,       // If item is not visible: scroll as little as possible on X axis to bring item back into view [default for X axis]
    RblxUIScrollFlags_KeepVisibleEdgeY       = 1 << 1,       // If item is not visible: scroll as little as possible on Y axis to bring item back into view [default for Y axis for windows that are already visible]
    RblxUIScrollFlags_KeepVisibleCenterX     = 1 << 2,       // If item is not visible: scroll to make the item centered on X axis [rarely used]
    RblxUIScrollFlags_KeepVisibleCenterY     = 1 << 3,       // If item is not visible: scroll to make the item centered on Y axis
    RblxUIScrollFlags_AlwaysCenterX          = 1 << 4,       // Always center the result item on X axis [rarely used]
    RblxUIScrollFlags_AlwaysCenterY          = 1 << 5,       // Always center the result item on Y axis [default for Y axis for appearing window)
    RblxUIScrollFlags_NoScrollParent         = 1 << 6,       // Disable forwarding scrolling to parent window if required to keep item/rect visible (only scroll window the function was applied to).
    RblxUIScrollFlags_MaskX_                 = RblxUIScrollFlags_KeepVisibleEdgeX | RblxUIScrollFlags_KeepVisibleCenterX | RblxUIScrollFlags_AlwaysCenterX,
    RblxUIScrollFlags_MaskY_                 = RblxUIScrollFlags_KeepVisibleEdgeY | RblxUIScrollFlags_KeepVisibleCenterY | RblxUIScrollFlags_AlwaysCenterY,
};

enum RblxUINavRenderCursorFlags_
{
    RblxUINavRenderCursorFlags_None          = 0,
    RblxUINavRenderCursorFlags_Compact       = 1 << 1,       // Compact highlight, no padding/distance from focused item
    RblxUINavRenderCursorFlags_AlwaysDraw    = 1 << 2,       // Draw rectangular highlight if (g.NavId == id) even when g.NavCursorVisible == false, aka even when using the mouse.
    RblxUINavRenderCursorFlags_NoRounding    = 1 << 3,
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    RblxUINavHighlightFlags_None             = RblxUINavRenderCursorFlags_None,       // Renamed in 1.91.4
    RblxUINavHighlightFlags_Compact          = RblxUINavRenderCursorFlags_Compact,    // Renamed in 1.91.4
    RblxUINavHighlightFlags_AlwaysDraw       = RblxUINavRenderCursorFlags_AlwaysDraw, // Renamed in 1.91.4
    RblxUINavHighlightFlags_NoRounding       = RblxUINavRenderCursorFlags_NoRounding, // Renamed in 1.91.4
    //RblxUINavHighlightFlags_TypeThin       = RblxUINavRenderCursorFlags_Compact,    // Renamed in 1.90.2
#endif
};

enum RblxUINavMoveFlags_
{
    RblxUINavMoveFlags_None                  = 0,
    RblxUINavMoveFlags_LoopX                 = 1 << 0,   // On failed request, restart from opposite side
    RblxUINavMoveFlags_LoopY                 = 1 << 1,
    RblxUINavMoveFlags_WrapX                 = 1 << 2,   // On failed request, request from opposite side one line down (when NavDir==right) or one line up (when NavDir==left)
    RblxUINavMoveFlags_WrapY                 = 1 << 3,   // This is not super useful but provided for completeness
    RblxUINavMoveFlags_WrapMask_             = RblxUINavMoveFlags_LoopX | RblxUINavMoveFlags_LoopY | RblxUINavMoveFlags_WrapX | RblxUINavMoveFlags_WrapY,
    RblxUINavMoveFlags_AllowCurrentNavId     = 1 << 4,   // Allow scoring and considering the current NavId as a move target candidate. This is used when the move source is offset (e.g. pressing PageDown actually needs to send a Up move request, if we are pressing PageDown from the bottom-most item we need to stay in place)
    RblxUINavMoveFlags_AlsoScoreVisibleSet   = 1 << 5,   // Store alternate result in NavMoveResultLocalVisible that only comprise elements that are already fully visible (used by PageUp/PageDown)
    RblxUINavMoveFlags_ScrollToEdgeY         = 1 << 6,   // Force scrolling to min/max (used by Home/End) // FIXME-NAV: Aim to remove or reword as RblxUIScrollFlags
    RblxUINavMoveFlags_Forwarded             = 1 << 7,
    RblxUINavMoveFlags_DebugNoResult         = 1 << 8,   // Dummy scoring for debug purpose, don't apply result
    RblxUINavMoveFlags_FocusApi              = 1 << 9,   // Requests from focus API can land/focus/activate items even if they are marked with _NoTabStop (see NavProcessItemForTabbingRequest() for details)
    RblxUINavMoveFlags_IsTabbing             = 1 << 10,  // == Focus + Activate if item is Inputable + DontChangeNavHighlight
    RblxUINavMoveFlags_IsPageMove            = 1 << 11,  // Identify a PageDown/PageUp request.
    RblxUINavMoveFlags_Activate              = 1 << 12,  // Activate/select target item.
    RblxUINavMoveFlags_NoSelect              = 1 << 13,  // Don't trigger selection by not setting g.NavJustMovedTo
    RblxUINavMoveFlags_NoSetNavCursorVisible = 1 << 14,  // Do not alter the nav cursor visible state
    RblxUINavMoveFlags_NoClearActiveId       = 1 << 15,  // (Experimental) Do not clear active id when applying move result
};

enum RblxUINavLayer
{
    RblxUINavLayer_Main  = 0,    // Main scrolling layer
    RblxUINavLayer_Menu  = 1,    // Menu layer (access with Alt)
    RblxUINavLayer_COUNT
};

// Storage for navigation query/results
struct RblxUINavItemData
{
    RblxUIWindow*        Window;         // Init,Move    // Best candidate window (result->ItemWindow->RootWindowForNav == request->Window)
    RblxUIID             ID;             // Init,Move    // Best candidate item ID
    RblxUIID             FocusScopeId;   // Init,Move    // Best candidate focus scope ID
    RblxRect              RectRel;        // Init,Move    // Best candidate bounding box in window relative space
    RblxUIItemFlags      ItemFlags;      // ????,Move    // Best candidate item flags
    float               DistBox;        //      Move    // Best candidate box distance to current NavId
    float               DistCenter;     //      Move    // Best candidate center distance to current NavId
    float               DistAxial;      //      Move    // Best candidate axial distance to current NavId
    RblxUISelectionUserData SelectionUserData;//I+Mov    // Best candidate SetNextItemSelectionUserData() value. Valid if (ItemFlags & RblxUIItemFlags_HasSelectionUserData)

    RblxUINavItemData()  { Clear(); }
    void Clear()        { Window = NULL; ID = FocusScopeId = 0; ItemFlags = 0; SelectionUserData = -1; DistBox = DistCenter = DistAxial = FLT_MAX; }
};

// Storage for PushFocusScope(), g.FocusScopeStack[], g.NavFocusRoute[]
struct RblxUIFocusScopeData
{
    RblxUIID             ID;
    RblxUIID             WindowID;
};

//-----------------------------------------------------------------------------
// [SECTION] Typing-select support
//-----------------------------------------------------------------------------

// Flags for GetTypingSelectRequest()
enum RblxUITypingSelectFlags_
{
    RblxUITypingSelectFlags_None                 = 0,
    RblxUITypingSelectFlags_AllowBackspace       = 1 << 0,   // Backspace to delete character inputs. If using: ensure GetTypingSelectRequest() is not called more than once per frame (filter by e.g. focus state)
    RblxUITypingSelectFlags_AllowSingleCharMode  = 1 << 1,   // Allow "single char" search mode which is activated when pressing the same character multiple times.
};

// Returned by GetTypingSelectRequest(), designed to eventually be public.
struct RBLXUI_API RblxUITypingSelectRequest
{
    RblxUITypingSelectFlags  Flags;              // Flags passed to GetTypingSelectRequest()
    int                     SearchBufferLen;
    const char*             SearchBuffer;       // Search buffer contents (use full string. unless SingleCharMode is set, in which case use SingleCharSize).
    bool                    SelectRequest;      // Set when buffer was modified this frame, requesting a selection.
    bool                    SingleCharMode;     // Notify when buffer contains same character repeated, to implement special mode. In this situation it preferred to not display any on-screen search indication.
    RblxS8                    SingleCharSize;     // Length in bytes of first letter codepoint (1 for ascii, 2-4 for UTF-8). If (SearchBufferLen==RepeatCharSize) only 1 letter has been input.
};

// Storage for GetTypingSelectRequest()
struct RBLXUI_API RblxUITypingSelectState
{
    RblxUITypingSelectRequest Request;           // User-facing data
    char            SearchBuffer[64];           // Search buffer: no need to make dynamic as this search is very transient.
    RblxUIID         FocusScope;
    int             LastRequestFrame = 0;
    float           LastRequestTime = 0.0f;
    bool            SingleCharModeLock = false; // After a certain single char repeat count we lock into SingleCharMode. Two benefits: 1) buffer never fill, 2) we can provide an immediate SingleChar mode without timer elapsing.

    RblxUITypingSelectState() { memset((void*)this, 0, sizeof(*this)); }
    void            Clear()  { SearchBuffer[0] = 0; SingleCharModeLock = false; } // We preserve remaining data for easier debugging
};

//-----------------------------------------------------------------------------
// [SECTION] Columns support
//-----------------------------------------------------------------------------

// Flags for internal's BeginColumns(). This is an obsolete API. Prefer using BeginTable() nowadays!
enum RblxUIOldColumnFlags_
{
    RblxUIOldColumnFlags_None                    = 0,
    RblxUIOldColumnFlags_NoBorder                = 1 << 0,   // Disable column dividers
    RblxUIOldColumnFlags_NoResize                = 1 << 1,   // Disable resizing columns when clicking on the dividers
    RblxUIOldColumnFlags_NoPreserveWidths        = 1 << 2,   // Disable column width preservation when adjusting columns
    RblxUIOldColumnFlags_NoForceWithinWindow     = 1 << 3,   // Disable forcing columns to fit within window
    RblxUIOldColumnFlags_GrowParentContentsSize  = 1 << 4,   // Restore pre-1.51 behavior of extending the parent window contents size but _without affecting the columns width at all_. Will eventually remove.

    // Obsolete names (will be removed)
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    //RblxUIColumnsFlags_None                    = RblxUIOldColumnFlags_None,
    //RblxUIColumnsFlags_NoBorder                = RblxUIOldColumnFlags_NoBorder,
    //RblxUIColumnsFlags_NoResize                = RblxUIOldColumnFlags_NoResize,
    //RblxUIColumnsFlags_NoPreserveWidths        = RblxUIOldColumnFlags_NoPreserveWidths,
    //RblxUIColumnsFlags_NoForceWithinWindow     = RblxUIOldColumnFlags_NoForceWithinWindow,
    //RblxUIColumnsFlags_GrowParentContentsSize  = RblxUIOldColumnFlags_GrowParentContentsSize,
#endif
};

struct RblxUIOldColumnData
{
    float               OffsetNorm;             // Column start offset, normalized 0.0 (far left) -> 1.0 (far right)
    float               OffsetNormBeforeResize;
    RblxUIOldColumnFlags Flags;                  // Not exposed
    RblxRect              ClipRect;

    RblxUIOldColumnData() { memset((void*)this, 0, sizeof(*this)); }
};

struct RblxUIOldColumns
{
    RblxUIID             ID;
    RblxUIOldColumnFlags Flags;
    bool                IsFirstFrame;
    bool                IsBeingResized;
    int                 Current;
    int                 Count;
    float               OffMinX, OffMaxX;       // Offsets from HostWorkRect.Min.x
    float               LineMinY, LineMaxY;
    float               HostCursorPosY;         // Backup of CursorPos at the time of BeginColumns()
    float               HostCursorMaxPosX;      // Backup of CursorMaxPos at the time of BeginColumns()
    RblxRect              HostInitialClipRect;    // Backup of ClipRect at the time of BeginColumns()
    RblxRect              HostBackupClipRect;     // Backup of ClipRect during PushColumnsBackground()/PopColumnsBackground()
    RblxRect              HostBackupParentWorkRect;//Backup of WorkRect at the time of BeginColumns()
    RblxVector<RblxUIOldColumnData> Columns;
    RblxDrawListSplitter  Splitter;

    RblxUIOldColumns()   { memset((void*)this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Box-select support
//-----------------------------------------------------------------------------

struct RblxUIBoxSelectState
{
    // Active box-selection data (persistent, 1 active at a time)
    RblxUIID                 ID;
    bool                    IsActive;
    bool                    IsStarting;
    bool                    IsStartedFromVoid;  // Starting click was not from an item.
    bool                    IsStartedSetNavIdOnce;
    bool                    RequestClear;
    RblxUIKeyChord           KeyMods : 16;       // Latched key-mods for box-select logic.
    RblxVec2                  StartPosRel;        // Start position in window-contents relative space (to support scrolling)
    RblxVec2                  EndPosRel;          // End position in window-contents relative space
    RblxVec2                  ScrollAccum;        // Scrolling accumulator (to behave at high-frame spaces)
    RblxUIWindow*            Window;

    // Temporary/Transient data
    bool                    UnclipMode;         // (Temp/Transient, here in hot area). Set/cleared by the BeginMultiSelect()/EndMultiSelect() owning active box-select.
    RblxRect                  UnclipRect;         // Rectangle where ItemAdd() clipping may be temporarily disabled. Need support by multi-select supporting widgets.
    RblxRect                  UnclipRects[2];     // Per-axis versions.
    RblxRect                  BoxSelectRectPrev;  // Selection rectangle in absolute coordinates (derived every frame from BoxSelectStartPosRel and MousePos)
    RblxRect                  BoxSelectRectCurr;

    RblxUIBoxSelectState()   { memset((void*)this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Multi-select support
//-----------------------------------------------------------------------------

// We always assume that -1 is an invalid value (which works for indices and pointers)
#define RblxUISelectionUserData_Invalid        ((RblxUISelectionUserData)-1)

// Temporary storage for multi-select
struct RBLXUI_API RblxUIMultiSelectTempData
{
    RblxUIMultiSelectIO      IO;                 // MUST BE FIRST FIELD. Requests are set and returned by BeginMultiSelect()/EndMultiSelect() + written to by user during the loop.
    RblxUIMultiSelectState*  Storage;
    RblxUIID                 FocusScopeId;       // Copied from g.CurrentFocusScopeId (unless another selection scope was pushed manually)
    RblxUIMultiSelectFlags   Flags;
    RblxVec2                  ScopeRectMin;
    RblxVec2                  BackupCursorMaxPos;
    //RblxUISelectionUserData CurrSubmittedItem; // Copy of last submitted item data, used to merge output ranges.
    //RblxUISelectionUserData PrevSubmittedItem; // Copy of previous submitted item data, used to merge output ranges.
    RblxUIID                 BoxSelectId;
    RblxUIKeyChord           KeyMods;
    RblxS8                    LoopRequestSetAll;  // -1: no operation, 0: clear all, 1: select all.
    bool                    IsEndIO;            // Set when switching IO from BeginMultiSelect() to EndMultiSelect() state.
    bool                    IsFocused;          // Set if currently focusing the selection scope (any item of the selection). May be used if you have custom shortcut associated to selection.
    bool                    IsKeyboardSetRange; // Set by BeginMultiSelect() when using Shift+Navigation. Because scrolling may be affected we can't afford a frame of lag with Shift+Navigation.
    bool                    NavIdPassedBy;
    bool                    RangeSrcPassedBy;   // Set by the item that matches RangeSrcItem.
    bool                    RangeDstPassedBy;   // Set by the item that matches NavJustMovedToId when IsSetRange is set.

    RblxUIMultiSelectTempData()  { Clear(); }
    void Clear()            { size_t io_sz = sizeof(IO); ClearIO(); memset((void*)(&IO + 1), 0, sizeof(*this) - io_sz); } // Zero-clear except IO as we preserve IO.Requests[] buffer allocation.
    void ClearIO()          { IO.Requests.resize(0); IO.RangeSrcItem = IO.NavIdItem = RblxUISelectionUserData_Invalid; IO.NavIdSelected = IO.RangeSrcReset = false; }
};

// Persistent storage for multi-select (as long as selection is alive)
struct RBLXUI_API RblxUIMultiSelectState
{
    RblxUIWindow*            Window;
    RblxUIID                 ID;
    int                     LastFrameActive;    // Last used frame-count, for GC.
    int                     LastSelectionSize;  // Set by BeginMultiSelect() based on optional info provided by user. May be -1 if unknown.
    RblxS8                    RangeSelected;      // -1 (don't have) or true/false
    RblxS8                    NavIdSelected;      // -1 (don't have) or true/false
    RblxUISelectionUserData  RangeSrcItem;       //
    RblxUISelectionUserData  NavIdItem;          // SetNextItemSelectionUserData() value for NavId (if part of submitted items)

    RblxUIMultiSelectState() { Window = NULL; ID = 0; LastFrameActive = LastSelectionSize = 0; RangeSelected = NavIdSelected = -1; RangeSrcItem = NavIdItem = RblxUISelectionUserData_Invalid; }
};

//-----------------------------------------------------------------------------
// [SECTION] Docking support
//-----------------------------------------------------------------------------

#define DOCKING_HOST_DRAW_CHANNEL_BG 0  // Dock host: background fill
#define DOCKING_HOST_DRAW_CHANNEL_FG 1  // Dock host: decorations and contents

#ifdef RBLXUI_HAS_DOCK

// Extend RblxUIDockNodeFlags_
enum RblxUIDockNodeFlagsPrivate_
{
    // [Internal]
    RblxUIDockNodeFlags_DockSpace                = 1 << 10,  // Saved // A dockspace is a node that occupy space within an existing user window. Otherwise the node is floating and create its own window.
    RblxUIDockNodeFlags_CentralNode              = 1 << 11,  // Saved // The central node has 2 main properties: stay visible when empty, only use "remaining" spaces from its neighbor.
    RblxUIDockNodeFlags_NoTabBar                 = 1 << 12,  // Saved // Tab bar is completely unavailable. No triangle in the corner to enable it back.
    RblxUIDockNodeFlags_HiddenTabBar             = 1 << 13,  // Saved // Tab bar is hidden, with a triangle in the corner to show it again (NB: actual tab-bar instance may be destroyed as this is only used for single-window tab bar)
    RblxUIDockNodeFlags_NoWindowMenuButton       = 1 << 14,  // Saved // Disable window/docking menu (that one that appears instead of the collapse button)
    RblxUIDockNodeFlags_NoCloseButton            = 1 << 15,  // Saved // Disable close button
    RblxUIDockNodeFlags_NoResizeX                = 1 << 16,  //       //
    RblxUIDockNodeFlags_NoResizeY                = 1 << 17,  //       //
    RblxUIDockNodeFlags_DockedWindowsInFocusRoute= 1 << 18,  //       // Any docked window will be automatically be focus-route chained (window->ParentWindowForFocusRoute set to this) so Shortcut() in this window can run when any docked window is focused.

    // Disable docking/undocking actions in this dockspace or individual node (existing docked nodes will be preserved)
    // Those are not exposed in public because the desirable sharing/inheriting/copy-flag-on-split behaviors are quite difficult to design and understand.
    // The two public flags RblxUIDockNodeFlags_NoDockingOverCentralNode/RblxUIDockNodeFlags_NoDockingSplit don't have those issues.
    RblxUIDockNodeFlags_NoDockingSplitOther      = 1 << 19,  //       // Disable this node from splitting other windows/nodes.
    RblxUIDockNodeFlags_NoDockingOverMe          = 1 << 20,  //       // Disable other windows/nodes from being docked over this node.
    RblxUIDockNodeFlags_NoDockingOverOther       = 1 << 21,  //       // Disable this node from being docked over another window or non-empty node.
    RblxUIDockNodeFlags_NoDockingOverEmpty       = 1 << 22,  //       // Disable this node from being docked over an empty node (e.g. DockSpace with no other windows)
    RblxUIDockNodeFlags_NoDocking                = RblxUIDockNodeFlags_NoDockingOverMe | RblxUIDockNodeFlags_NoDockingOverOther | RblxUIDockNodeFlags_NoDockingOverEmpty | RblxUIDockNodeFlags_NoDockingSplit | RblxUIDockNodeFlags_NoDockingSplitOther,

    // Masks
    RblxUIDockNodeFlags_SharedFlagsInheritMask_  = ~0,
    RblxUIDockNodeFlags_NoResizeFlagsMask_       = (int)RblxUIDockNodeFlags_NoResize | RblxUIDockNodeFlags_NoResizeX | RblxUIDockNodeFlags_NoResizeY,

    // When splitting, those local flags are moved to the inheriting child, never duplicated
    RblxUIDockNodeFlags_LocalFlagsTransferMask_  = (int)RblxUIDockNodeFlags_NoDockingSplit | RblxUIDockNodeFlags_NoResizeFlagsMask_ | (int)RblxUIDockNodeFlags_AutoHideTabBar | RblxUIDockNodeFlags_CentralNode | RblxUIDockNodeFlags_NoTabBar | RblxUIDockNodeFlags_HiddenTabBar | RblxUIDockNodeFlags_NoWindowMenuButton | RblxUIDockNodeFlags_NoCloseButton,
    RblxUIDockNodeFlags_SavedFlagsMask_          = RblxUIDockNodeFlags_NoResizeFlagsMask_ | RblxUIDockNodeFlags_DockSpace | RblxUIDockNodeFlags_CentralNode | RblxUIDockNodeFlags_NoTabBar | RblxUIDockNodeFlags_HiddenTabBar | RblxUIDockNodeFlags_NoWindowMenuButton | RblxUIDockNodeFlags_NoCloseButton,
};

// Store the source authority (dock node vs window) of a field
enum RblxUIDataAuthority_
{
    RblxUIDataAuthority_Auto,
    RblxUIDataAuthority_DockNode,
    RblxUIDataAuthority_Window,
};

enum RblxUIDockNodeState
{
    RblxUIDockNodeState_Unknown,
    RblxUIDockNodeState_HostWindowHiddenBecauseSingleWindow,
    RblxUIDockNodeState_HostWindowHiddenBecauseWindowsAreResizing,
    RblxUIDockNodeState_HostWindowVisible,
};

// sizeof() 176~216
struct RBLXUI_API RblxUIDockNode
{
    RblxUIID                 ID;
    RblxUIDockNodeFlags      SharedFlags;                // (Write) Flags shared by all nodes of a same dockspace hierarchy (inherited from the root node)
    RblxUIDockNodeFlags      LocalFlags;                 // (Write) Flags specific to this node
    RblxUIDockNodeFlags      LocalFlagsInWindows;        // (Write) Flags specific to this node, applied from windows
    RblxUIDockNodeFlags      MergedFlags;                // (Read)  Effective flags (== SharedFlags | LocalFlagsInNode | LocalFlagsInWindows)
    RblxUIDockNodeState      State;
    RblxUIDockNode*          ParentNode;
    RblxUIDockNode*          ChildNodes[2];              // [Split node only] Child nodes (left/right or top/bottom). Consider switching to an array.
    RblxVector<RblxUIWindow*>  Windows;                    // Note: unordered list! Iterate TabBar->Tabs for user-order.
    RblxUITabBar*            TabBar;
    RblxVec2                  Pos;                        // Current position
    RblxVec2                  Size;                       // Current size
    RblxVec2                  SizeRef;                    // [Split node only] Last explicitly written-to size (overridden when using a splitter affecting the node), used to calculate Size.
    RblxUIAxis               SplitAxis;                  // [Split node only] Split axis (X or Y)
    RblxU32                   LastBgColor;
    RblxUIWindowClass        WindowClass;                // [Root node only]

    RblxUIWindow*            HostWindow;
    RblxUIWindow*            VisibleWindow;              // Generally point to window which is ID is == SelectedTabID, but when CTRL+Tabbing this can be a different window.
    RblxUIDockNode*          CentralNode;                // [Root node only] Pointer to central node.
    RblxUIDockNode*          OnlyNodeWithWindows;        // [Root node only] Set when there is a single visible node within the hierarchy.
    int                     CountNodeWithWindows;       // [Root node only]
    int                     LastFrameAlive;             // Last frame number the node was updated or kept alive explicitly with DockSpace() + RblxUIDockNodeFlags_KeepAliveOnly
    int                     LastFrameActive;            // Last frame number the node was updated.
    int                     LastFrameFocused;           // Last frame number the node was focused.
    RblxUIID                 LastFocusedNodeId;          // [Root node only] Which of our child docking node (any ancestor in the hierarchy) was last focused.
    RblxUIID                 SelectedTabId;              // [Leaf node only] Which of our tab/window is selected.
    RblxUIID                 WantCloseTabId;             // [Leaf node only] Set when closing a specific tab/window.
    RblxUIID                 RefViewportId;              // Reference viewport ID from visible window when HostWindow == NULL.
    RblxUIDataAuthority      AuthorityForPos         :3;
    RblxUIDataAuthority      AuthorityForSize        :3;
    RblxUIDataAuthority      AuthorityForViewport    :3;
    bool                    IsVisible               :1; // Set to false when the node is hidden (usually disabled as it has no active window)
    bool                    IsFocused               :1;
    bool                    IsBgDrawnThisFrame      :1;
    bool                    HasCloseButton          :1; // Provide space for a close button (if any of the docked window has one). Note that button may be hidden on window without one.
    bool                    HasWindowMenuButton     :1;
    bool                    HasCentralNodeChild     :1;
    bool                    WantCloseAll            :1; // Set when closing all tabs at once.
    bool                    WantLockSizeOnce        :1;
    bool                    WantMouseMove           :1; // After a node extraction we need to transition toward moving the newly created host window
    bool                    WantHiddenTabBarUpdate  :1;
    bool                    WantHiddenTabBarToggle  :1;

    RblxUIDockNode(RblxUIID id);
    ~RblxUIDockNode();
    bool                    IsRootNode() const      { return ParentNode == NULL; }
    bool                    IsDockSpace() const     { return (MergedFlags & RblxUIDockNodeFlags_DockSpace) != 0; }
    bool                    IsFloatingNode() const  { return ParentNode == NULL && (MergedFlags & RblxUIDockNodeFlags_DockSpace) == 0; }
    bool                    IsCentralNode() const   { return (MergedFlags & RblxUIDockNodeFlags_CentralNode) != 0; }
    bool                    IsHiddenTabBar() const  { return (MergedFlags & RblxUIDockNodeFlags_HiddenTabBar) != 0; } // Hidden tab bar can be shown back by clicking the small triangle
    bool                    IsNoTabBar() const      { return (MergedFlags & RblxUIDockNodeFlags_NoTabBar) != 0; }     // Never show a tab bar
    bool                    IsSplitNode() const     { return ChildNodes[0] != NULL; }
    bool                    IsLeafNode() const      { return ChildNodes[0] == NULL; }
    bool                    IsEmpty() const         { return ChildNodes[0] == NULL && Windows.Size == 0; }
    RblxRect                  Rect() const            { return RblxRect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y); }

    void                    SetLocalFlags(RblxUIDockNodeFlags flags) { LocalFlags = flags; UpdateMergedFlags(); }
    void                    UpdateMergedFlags()     { MergedFlags = SharedFlags | LocalFlags | LocalFlagsInWindows; }
};

// List of colors that are stored at the time of Begin() into Docked Windows.
// We currently store the packed colors in a simple array window->DockStyle.Colors[].
// A better solution may involve appending into a log of colors in RblxUIContext + store offsets into those arrays in RblxUIWindow,
// but it would be more complex as we'd need to double-buffer both as e.g. drop target may refer to window from last frame.
enum RblxUIWindowDockStyleCol
{
    RblxUIWindowDockStyleCol_Text,
    RblxUIWindowDockStyleCol_TabHovered,
    RblxUIWindowDockStyleCol_TabFocused,
    RblxUIWindowDockStyleCol_TabSelected,
    RblxUIWindowDockStyleCol_TabSelectedOverline,
    RblxUIWindowDockStyleCol_TabDimmed,
    RblxUIWindowDockStyleCol_TabDimmedSelected,
    RblxUIWindowDockStyleCol_TabDimmedSelectedOverline,
    RblxUIWindowDockStyleCol_UnsavedMarker,
    RblxUIWindowDockStyleCol_COUNT
};

// We don't store style.Alpha: dock_node->LastBgColor embeds it and otherwise it would only affect the docking tab, which intuitively I would say we don't want to.
struct RblxUIWindowDockStyle
{
    RblxU32 Colors[RblxUIWindowDockStyleCol_COUNT];
};

struct RblxUIDockContext
{
    RblxUIStorage                    Nodes;          // Map ID -> RblxUIDockNode*: Active nodes
    RblxVector<RblxUIDockRequest>      Requests;
    RblxVector<RblxUIDockNodeSettings> NodesSettings;
    bool                            WantFullRebuild;
    RblxUIDockContext()              { memset((void*)this, 0, sizeof(*this)); }
};

#endif // #ifdef RBLXUI_HAS_DOCK

//-----------------------------------------------------------------------------
// [SECTION] Viewport support
//-----------------------------------------------------------------------------

// RblxUIViewport Private/Internals fields (cardinal sin: we are using inheritance!)
// Every instance of RblxUIViewport is in fact a RblxUIViewportP.
struct RblxUIViewportP : public RblxUIViewport
{
    RblxUIWindow*        Window;                 // Set when the viewport is owned by a window (and RblxUIViewportFlags_CanHostOtherWindows is NOT set)
    int                 Idx;
    int                 LastFrameActive;        // Last frame number this viewport was activated by a window
    int                 LastFocusedStampCount;  // Last stamp number from when a window hosted by this viewport was focused (by comparing this value between two viewport we have an implicit viewport z-order we use as fallback)
    RblxUIID             LastNameHash;
    RblxVec2              LastPos;
    RblxVec2              LastSize;
    float               Alpha;                  // Window opacity (when dragging dockable windows/viewports we make them transparent)
    float               LastAlpha;
    bool                LastFocusedHadNavWindow;// Instead of maintaining a LastFocusedWindow (which may harder to correctly maintain), we merely store weither NavWindow != NULL last time the viewport was focused.
    short               PlatformMonitor;
    float               BgFgDrawListsLastTimeActive[2]; // Last frame number the background (0) and foreground (1) draw lists were used
    RblxDrawList*         BgFgDrawLists[2];       // Convenience background (0) and foreground (1) draw lists. We use them to draw software mouser cursor when io.MouseDrawCursor is set and to draw most debug overlays.
    RblxDrawData          DrawDataP;
    RblxDrawDataBuilder   DrawDataBuilder;        // Temporary data while building final RblxDrawData
    RblxVec2              LastPlatformPos;
    RblxVec2              LastPlatformSize;
    RblxVec2              LastRendererSize;

    // Per-viewport work area
    // - Insets are >= 0.0f values, distance from viewport corners to work area.
    // - BeginMainMenuBar() and DockspaceOverViewport() tend to use work area to avoid stepping over existing contents.
    // - Generally 'safeAreaInsets' in iOS land, 'DisplayCutout' in Android land.
    RblxVec2              WorkInsetMin;           // Work Area inset locked for the frame. GetWorkRect() always fits within GetMainRect().
    RblxVec2              WorkInsetMax;           // "
    RblxVec2              BuildWorkInsetMin;      // Work Area inset accumulator for current frame, to become next frame's WorkInset
    RblxVec2              BuildWorkInsetMax;      // "

    RblxUIViewportP()                    { Window = NULL; Idx = -1; LastFrameActive = LastFocusedStampCount = -1; BgFgDrawListsLastTimeActive[0] = BgFgDrawListsLastTimeActive[1] = -1.0f; LastNameHash = 0; Alpha = LastAlpha = 1.0f; LastFocusedHadNavWindow = false; PlatformMonitor = -1; BgFgDrawLists[0] = BgFgDrawLists[1] = NULL; LastPlatformPos = LastPlatformSize = LastRendererSize = RblxVec2(FLT_MAX, FLT_MAX); }
    ~RblxUIViewportP()                   { if (BgFgDrawLists[0]) RBLXUI_DELETE(BgFgDrawLists[0]); if (BgFgDrawLists[1]) RBLXUI_DELETE(BgFgDrawLists[1]); }
    void    ClearRequestFlags()         { PlatformRequestClose = PlatformRequestMove = PlatformRequestResize = false; }

    // Calculate work rect pos/size given a set of offset (we have 1 pair of offset for rect locked from last frame data, and 1 pair for currently building rect)
    RblxVec2  CalcWorkRectPos(const RblxVec2& inset_min) const                           { return RblxVec2(Pos.x + inset_min.x, Pos.y + inset_min.y); }
    RblxVec2  CalcWorkRectSize(const RblxVec2& inset_min, const RblxVec2& inset_max) const { return RblxVec2(RblxMax(0.0f, Size.x - inset_min.x - inset_max.x), RblxMax(0.0f, Size.y - inset_min.y - inset_max.y)); }
    void    UpdateWorkRect()            { WorkPos = CalcWorkRectPos(WorkInsetMin); WorkSize = CalcWorkRectSize(WorkInsetMin, WorkInsetMax); } // Update public fields

    // Helpers to retrieve RblxRect (we don't need to store BuildWorkRect as every access tend to change it, hence the code asymmetry)
    RblxRect  GetMainRect() const         { return RblxRect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y); }
    RblxRect  GetWorkRect() const         { return RblxRect(WorkPos.x, WorkPos.y, WorkPos.x + WorkSize.x, WorkPos.y + WorkSize.y); }
    RblxRect  GetBuildWorkRect() const    { RblxVec2 pos = CalcWorkRectPos(BuildWorkInsetMin); RblxVec2 size = CalcWorkRectSize(BuildWorkInsetMin, BuildWorkInsetMax); return RblxRect(pos.x, pos.y, pos.x + size.x, pos.y + size.y); }
};

//-----------------------------------------------------------------------------
// [SECTION] Settings support
//-----------------------------------------------------------------------------

// Windows data saved in rblxui.ini file
// Because we never destroy or rename RblxUIWindowSettings, we can store the names in a separate buffer easily.
// (this is designed to be stored in a RblxChunkStream buffer, with the variable-length Name following our structure)
struct RblxUIWindowSettings
{
    RblxUIID     ID;
    RblxVec2ih    Pos;            // NB: Settings position are stored RELATIVE to the viewport! Whereas runtime ones are absolute positions.
    RblxVec2ih    Size;
    RblxVec2ih    ViewportPos;
    RblxUIID     ViewportId;
    RblxUIID     DockId;         // ID of last known DockNode (even if the DockNode is invisible because it has only 1 active window), or 0 if none.
    RblxUIID     ClassId;        // ID of window class if specified
    short       DockOrder;      // Order of the last time the window was visible within its DockNode. This is used to reorder windows that are reappearing on the same frame. Same value between windows that were active and windows that were none are possible.
    bool        Collapsed;
    bool        IsChild;
    bool        WantApply;      // Set when loaded from .ini data (to enable merging/loading .ini data into an already running context)
    bool        WantDelete;     // Set to invalidate/delete the settings entry

    RblxUIWindowSettings()       { memset((void*)this, 0, sizeof(*this)); DockOrder = -1; }
    char* GetName()             { return (char*)(this + 1); }
};

struct RblxUISettingsHandler
{
    const char* TypeName;       // Short description stored in .ini file. Disallowed characters: '[' ']'
    RblxUIID     TypeHash;       // == RblxHashStr(TypeName)
    void        (*ClearAllFn)(RblxUIContext* ctx, RblxUISettingsHandler* handler);                                // Clear all settings data
    void        (*ReadInitFn)(RblxUIContext* ctx, RblxUISettingsHandler* handler);                                // Read: Called before reading (in registration order)
    void*       (*ReadOpenFn)(RblxUIContext* ctx, RblxUISettingsHandler* handler, const char* name);              // Read: Called when entering into a new ini entry e.g. "[Window][Name]"
    void        (*ReadLineFn)(RblxUIContext* ctx, RblxUISettingsHandler* handler, void* entry, const char* line); // Read: Called for every line of text within an ini entry
    void        (*ApplyAllFn)(RblxUIContext* ctx, RblxUISettingsHandler* handler);                                // Read: Called after reading (in registration order)
    void        (*WriteAllFn)(RblxUIContext* ctx, RblxUISettingsHandler* handler, RblxUITextBuffer* out_buf);      // Write: Output every entries into 'out_buf'
    void*       UserData;

    RblxUISettingsHandler() { memset((void*)this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Localization support
//-----------------------------------------------------------------------------

// This is experimental and not officially supported, it'll probably fall short of features, if/when it does we may backtrack.
enum RblxUILocKey : int
{
    RblxUILocKey_VersionStr,
    RblxUILocKey_TableSizeOne,
    RblxUILocKey_TableSizeAllFit,
    RblxUILocKey_TableSizeAllDefault,
    RblxUILocKey_TableResetOrder,
    RblxUILocKey_WindowingMainMenuBar,
    RblxUILocKey_WindowingPopup,
    RblxUILocKey_WindowingUntitled,
    RblxUILocKey_OpenLink_s,
    RblxUILocKey_CopyLink,
    RblxUILocKey_DockingHideTabBar,
    RblxUILocKey_DockingHoldShiftToDock,
    RblxUILocKey_DockingDragToUndockOrMoveNode,
    RblxUILocKey_COUNT
};

struct RblxUILocEntry
{
    RblxUILocKey     Key;
    const char*     Text;
};

//-----------------------------------------------------------------------------
// [SECTION] Error handling, State recovery support
//-----------------------------------------------------------------------------

// Macros used by Recoverable Error handling
// - Only dispatch error if _EXPR: evaluate as assert (similar to an assert macro).
// - The message will always be a string literal, in order to increase likelihood of being display by an assert handler.
// - See 'Demo->Configuration->Error Handling' and RblxUIIO definitions for details on error handling.
// - Read https://github.com/ocornut/rblxui/wiki/Error-Handling for details on error handling.
#ifndef RBLXUI_ASSERT_USER_ERROR
#define RBLXUI_ASSERT_USER_ERROR(_EXPR,_MSG)            do { if (!(_EXPR)) { if (RblxUI::ErrorLog(_MSG)) { RBLXUI_ASSERT((_EXPR) && _MSG); } } } while (0)               // Recoverable User Error
#define RBLXUI_ASSERT_USER_ERROR_RET(_EXPR,_MSG)        do { if (!(_EXPR)) { if (RblxUI::ErrorLog(_MSG)) { RBLXUI_ASSERT((_EXPR) && _MSG); } return; } } while (0)       // Recoverable User Error
#define RBLXUI_ASSERT_USER_ERROR_RETV(_EXPR,_RETV,_MSG) do { if (!(_EXPR)) { if (RblxUI::ErrorLog(_MSG)) { RBLXUI_ASSERT((_EXPR) && _MSG); } return _RETV; } } while (0) // Recoverable User Error
#endif

// The error callback is currently not public, as it is expected that only advanced users will rely on it.
typedef void (*RblxUIErrorCallback)(RblxUIContext* ctx, void* user_data, const char* msg); // Function signature for g.ErrorCallback

//-----------------------------------------------------------------------------
// [SECTION] Metrics, Debug Tools
//-----------------------------------------------------------------------------

// See RBLXUI_DEBUG_LOG() and RBLXUI_DEBUG_LOG_XXX() macros.
enum RblxUIDebugLogFlags_
{
    // Event types
    RblxUIDebugLogFlags_None                 = 0,
    RblxUIDebugLogFlags_EventError           = 1 << 0,   // Error submitted by RBLXUI_ASSERT_USER_ERROR()
    RblxUIDebugLogFlags_EventActiveId        = 1 << 1,
    RblxUIDebugLogFlags_EventFocus           = 1 << 2,
    RblxUIDebugLogFlags_EventPopup           = 1 << 3,
    RblxUIDebugLogFlags_EventNav             = 1 << 4,
    RblxUIDebugLogFlags_EventClipper         = 1 << 5,
    RblxUIDebugLogFlags_EventSelection       = 1 << 6,
    RblxUIDebugLogFlags_EventIO              = 1 << 7,
    RblxUIDebugLogFlags_EventFont            = 1 << 8,
    RblxUIDebugLogFlags_EventInputRouting    = 1 << 9,
    RblxUIDebugLogFlags_EventDocking         = 1 << 10,
    RblxUIDebugLogFlags_EventViewport        = 1 << 11,

    RblxUIDebugLogFlags_EventMask_           = RblxUIDebugLogFlags_EventError | RblxUIDebugLogFlags_EventActiveId | RblxUIDebugLogFlags_EventFocus | RblxUIDebugLogFlags_EventPopup | RblxUIDebugLogFlags_EventNav | RblxUIDebugLogFlags_EventClipper | RblxUIDebugLogFlags_EventSelection | RblxUIDebugLogFlags_EventIO | RblxUIDebugLogFlags_EventFont | RblxUIDebugLogFlags_EventInputRouting | RblxUIDebugLogFlags_EventDocking | RblxUIDebugLogFlags_EventViewport,
    RblxUIDebugLogFlags_OutputToTTY          = 1 << 20,  // Also send output to TTY
    RblxUIDebugLogFlags_OutputToDebugger     = 1 << 21,  // Also send output to Debugger Console [Windows only]
    RblxUIDebugLogFlags_OutputToTestEngine   = 1 << 22,  // Also send output to Dear RblxUI Test Engine
};

struct RblxUIDebugAllocEntry
{
    int         FrameCount;
    RblxS16       AllocCount;
    RblxS16       FreeCount;
};

struct RblxUIDebugAllocInfo
{
    int         TotalAllocCount;            // Number of call to MemAlloc().
    int         TotalFreeCount;
    RblxS16       LastEntriesIdx;             // Current index in buffer
    RblxUIDebugAllocEntry LastEntriesBuf[6]; // Track last 6 frames that had allocations

    RblxUIDebugAllocInfo() { memset((void*)this, 0, sizeof(*this)); }
};

struct RblxUIMetricsConfig
{
    bool        ShowDebugLog = false;
    bool        ShowIDStackTool = false;
    bool        ShowWindowsRects = false;
    bool        ShowWindowsBeginOrder = false;
    bool        ShowTablesRects = false;
    bool        ShowDrawCmdMesh = true;
    bool        ShowDrawCmdBoundingBoxes = true;
    bool        ShowTextEncodingViewer = false;
    bool        ShowTextureUsedRect = false;
    bool        ShowDockingNodes = false;
    int         ShowWindowsRectsType = -1;
    int         ShowTablesRectsType = -1;
    int         HighlightMonitorIdx = -1;
    RblxUIID     HighlightViewportID = 0;
    bool        ShowFontPreview = true;
};

struct RblxUIStackLevelInfo
{
    RblxUIID                 ID;
    RblxS8                    QueryFrameCount;            // >= 1: Sub-query in progress
    bool                    QuerySuccess;               // Sub-query obtained result from DebugHookIdInfo()
    RblxS8                    DataType;                   // RblxUIDataType
    int                     DescOffset;                 // -1 or offset into parent's ResultsPathsBuf

    RblxUIStackLevelInfo()   { memset((void*)this, 0, sizeof(*this)); DataType = -1; DescOffset = -1; }
};

struct RblxUIDebugItemPathQuery
{
    RblxUIID                 MainID;                     // ID to query details for.
    bool                    Active;                     // Used to disambiguate the case when ID == 0 and e.g. some code calls PushOverrideID(0).
    bool                    Complete;                   // All sub-queries are finished (some may have failed).
    RblxS8                    Step;                       // -1: query stack + init Results, >= 0: filling individual stack level.
    RblxVector<RblxUIStackLevelInfo> Results;
    RblxUITextBuffer         ResultsDescBuf;
    RblxUITextBuffer         ResultPathBuf;

    RblxUIDebugItemPathQuery() { memset((void*)this, 0, sizeof(*this)); }
};

// State for ID Stack tool queries
struct RblxUIIDStackTool
{
    bool                    OptHexEncodeNonAsciiChars;
    bool                    OptCopyToClipboardOnCtrlC;
    int                     LastActiveFrame;
    float                   CopyToClipboardLastTime;

    RblxUIIDStackTool()      { memset((void*)this, 0, sizeof(*this)); LastActiveFrame = -1; OptHexEncodeNonAsciiChars = true; CopyToClipboardLastTime = -FLT_MAX; }
};

//-----------------------------------------------------------------------------
// [SECTION] Generic context hooks
//-----------------------------------------------------------------------------

typedef void (*RblxUIContextHookCallback)(RblxUIContext* ctx, RblxUIContextHook* hook);
enum RblxUIContextHookType { RblxUIContextHookType_NewFramePre, RblxUIContextHookType_NewFramePost, RblxUIContextHookType_EndFramePre, RblxUIContextHookType_EndFramePost, RblxUIContextHookType_RenderPre, RblxUIContextHookType_RenderPost, RblxUIContextHookType_Shutdown, RblxUIContextHookType_PendingRemoval_ };

struct RblxUIContextHook
{
    RblxUIID                     HookId;     // A unique ID assigned by AddContextHook()
    RblxUIContextHookType        Type;
    RblxUIID                     Owner;
    RblxUIContextHookCallback    Callback;
    void*                       UserData;

    RblxUIContextHook()          { memset((void*)this, 0, sizeof(*this)); }
};

typedef void (*RblxUIDemoMarkerCallback)(const char* file, int line, const char* section);

//-----------------------------------------------------------------------------
// [SECTION] RblxUIContext (main Dear RblxUI context)
//-----------------------------------------------------------------------------

struct RblxUIContext
{
    bool                    Initialized;
    bool                    WithinFrameScope;                   // Set by NewFrame(), cleared by EndFrame()
    bool                    WithinFrameScopeWithImplicitWindow; // Set by NewFrame(), cleared by EndFrame() when the implicit debug window has been pushed
    bool                    TestEngineHookItems;                // Will call test engine hooks: RblxUITestEngineHook_ItemAdd(), RblxUITestEngineHook_ItemInfo(), RblxUITestEngineHook_Log()
    int                     FrameCount;
    int                     FrameCountEnded;
    int                     FrameCountPlatformEnded;
    int                     FrameCountRendered;
    double                  Time;
    char                    ContextName[16];                    // Storage for a context name (to facilitate debugging multi-context setups)
    RblxUIIO                 IO;
    RblxUIPlatformIO         PlatformIO;
    RblxUIStyle              Style;
    RblxUIConfigFlags        ConfigFlagsCurrFrame;               // = g.IO.ConfigFlags at the time of NewFrame()
    RblxUIConfigFlags        ConfigFlagsLastFrame;
    RblxVector<RblxFontAtlas*>  FontAtlases;                        // List of font atlases used by the context (generally only contains g.IO.Fonts aka the main font atlas)
    RblxFont*                 Font;                               // Currently bound font. (== FontStack.back().Font)
    RblxFontBaked*            FontBaked;                          // Currently bound font at currently bound size. (== Font->GetFontBaked(FontSize))
    float                   FontSize;                           // Currently bound font size == line height (== FontSizeBase + externals scales applied in the UpdateCurrentFontSize() function).
    float                   FontSizeBase;                       // Font size before scaling == style.FontSizeBase == value passed to PushFont() when specified.
    float                   FontBakedScale;                     // == FontBaked->Size / FontSize. Scale factor over baked size. Rarely used nowadays, very often == 1.0f.
    float                   FontRasterizerDensity;              // Current font density. Used by all calls to GetFontBaked().
    float                   CurrentDpiScale;                    // Current window/viewport DpiScale == CurrentViewport->DpiScale
    RblxDrawListSharedData    DrawListSharedData;
    RblxUIID                 WithinEndChildID;                   // Set within EndChild()
    RblxUIID                 WithinEndPopupID;                   // Set within EndPopup()
    void*                   TestEngine;                         // Test engine user data

    // Inputs
    RblxVector<RblxUIInputEvent> InputEventsQueue;                 // Input events which will be trickled/written into IO structure.
    RblxVector<RblxUIInputEvent> InputEventsTrail;                 // Past input events processed in NewFrame(). This is to allow domain-specific application to access e.g mouse/pen trail.
    RblxUIMouseSource        InputEventsNextMouseSource;
    RblxU32                   InputEventsNextEventId;

    // Windows state
    RblxVector<RblxUIWindow*>  Windows;                            // Windows, sorted in display order, back to front
    RblxVector<RblxUIWindow*>  WindowsFocusOrder;                  // Root windows, sorted in focus order, back to front.
    RblxVector<RblxUIWindow*>  WindowsTempSortBuffer;              // Temporary buffer used in EndFrame() to reorder windows so parents are kept before their child
    RblxVector<RblxUIWindowStackData> CurrentWindowStack;
    RblxUIStorage            WindowsById;                        // Map window's RblxUIID to RblxUIWindow*
    int                     WindowsActiveCount;                 // Number of unique windows submitted by frame
    float                   WindowsBorderHoverPadding;          // Padding around resizable windows for which hovering on counts as hovering the window == RblxMax(style.TouchExtraPadding, style.WindowBorderHoverPadding). This isn't so multi-dpi friendly.
    RblxUIID                 DebugBreakInWindow;                 // Set to break in Begin() call.
    RblxUIWindow*            CurrentWindow;                      // Window being drawn into
    RblxUIWindow*            HoveredWindow;                      // Window the mouse is hovering. Will typically catch mouse inputs.
    RblxUIWindow*            HoveredWindowUnderMovingWindow;     // Hovered window ignoring MovingWindow. Only set if MovingWindow is set.
    RblxUIWindow*            HoveredWindowBeforeClear;           // Window the mouse is hovering. Filled even with _NoMouse. This is currently useful for multi-context compositors.
    RblxUIWindow*            MovingWindow;                       // Track the window we clicked on (in order to preserve focus). The actual window that is moved is generally MovingWindow->RootWindowDockTree.
    RblxUIWindow*            WheelingWindow;                     // Track the window we started mouse-wheeling on. Until a timer elapse or mouse has moved, generally keep scrolling the same window even if during the course of scrolling the mouse ends up hovering a child window.
    RblxVec2                  WheelingWindowRefMousePos;
    int                     WheelingWindowStartFrame;           // This may be set one frame before WheelingWindow is != NULL
    int                     WheelingWindowScrolledFrame;
    float                   WheelingWindowReleaseTimer;
    RblxVec2                  WheelingWindowWheelRemainder;
    RblxVec2                  WheelingAxisAvg;

    // Item/widgets state and tracking information
    RblxUIID                 DebugDrawIdConflictsId;             // Set when we detect multiple items with the same identifier
    RblxUIID                 DebugHookIdInfoId;                  // Will call core hooks: DebugHookIdInfo() from GetID functions, used by ID Stack Tool [next HoveredId/ActiveId to not pull in an extra cache-line]
    RblxUIID                 HoveredId;                          // Hovered widget, filled during the frame
    RblxUIID                 HoveredIdPreviousFrame;
    int                     HoveredIdPreviousFrameItemCount;    // Count numbers of items using the same ID as last frame's hovered id
    float                   HoveredIdTimer;                     // Measure contiguous hovering time
    float                   HoveredIdNotActiveTimer;            // Measure contiguous hovering time where the item has not been active
    bool                    HoveredIdAllowOverlap;
    bool                    HoveredIdIsDisabled;                // At least one widget passed the rect test, but has been discarded by disabled flag or popup inhibit. May be true even if HoveredId == 0.
    bool                    ItemUnclipByLog;                    // Disable ItemAdd() clipping, essentially a memory-locality friendly copy of LogEnabled
    RblxUIID                 ActiveId;                           // Active widget
    RblxUIID                 ActiveIdIsAlive;                    // Active widget has been seen this frame (we can't use a bool as the ActiveId may change within the frame)
    float                   ActiveIdTimer;
    bool                    ActiveIdIsJustActivated;            // Set at the time of activation for one frame
    bool                    ActiveIdAllowOverlap;               // Active widget allows another widget to steal active id (generally for overlapping widgets, but not always)
    bool                    ActiveIdNoClearOnFocusLoss;         // Disable losing active id if the active id window gets unfocused.
    bool                    ActiveIdHasBeenPressedBefore;       // Track whether the active id led to a press (this is to allow changing between PressOnClick and PressOnRelease without pressing twice). Used by range_select branch.
    bool                    ActiveIdHasBeenEditedBefore;        // Was the value associated to the widget Edited over the course of the Active state.
    bool                    ActiveIdHasBeenEditedThisFrame;
    bool                    ActiveIdFromShortcut;
    RblxS8                    ActiveIdMouseButton;
    RblxUIID                 ActiveIdDisabledId;                 // When clicking a disabled item we set ActiveId=window->MoveId to avoid interference with widget code. Actual item ID is stored here.
    RblxVec2                  ActiveIdClickOffset;                // Clicked offset from upper-left corner, if applicable (currently only set by ButtonBehavior)
    RblxUIInputSource        ActiveIdSource;                     // Activating source: RblxUIInputSource_Mouse OR RblxUIInputSource_Keyboard OR RblxUIInputSource_Gamepad
    RblxUIWindow*            ActiveIdWindow;
    RblxUIID                 ActiveIdPreviousFrame;
    RblxUIDeactivatedItemData DeactivatedItemData;
    RblxUIDataTypeStorage    ActiveIdValueOnActivation;          // Backup of initial value at the time of activation. ONLY SET BY SPECIFIC WIDGETS: DragXXX and SliderXXX.
    RblxUIID                 LastActiveId;                       // Store the last non-zero ActiveId, useful for animation.
    float                   LastActiveIdTimer;                  // Store the last non-zero ActiveId timer since the beginning of activation, useful for animation.

    // Key/Input Ownership + Shortcut Routing system
    // - The idea is that instead of "eating" a given key, we can link to an owner.
    // - Input query can then read input by specifying RblxUIKeyOwner_Any (== 0), RblxUIKeyOwner_NoOwner (== -1) or a custom ID.
    // - Routing is requested ahead of time for a given chord (Key + Mods) and granted in NewFrame().
    double                  LastKeyModsChangeTime;              // Record the last time key mods changed (affect repeat delay when using shortcut logic)
    double                  LastKeyModsChangeFromNoneTime;      // Record the last time key mods changed away from being 0 (affect repeat delay when using shortcut logic)
    double                  LastKeyboardKeyPressTime;           // Record the last time a keyboard key (ignore mouse/gamepad ones) was pressed.
    RblxBitArrayForNamedKeys  KeysMayBeCharInput;                 // Lookup to tell if a key can emit char input, see IsKeyChordPotentiallyCharInput(). sizeof() = 20 bytes
    RblxUIKeyOwnerData       KeysOwnerData[RblxUIKey_NamedKey_COUNT];
    RblxUIKeyRoutingTable    KeysRoutingTable;
    RblxU32                   ActiveIdUsingNavDirMask;            // Active widget will want to read those nav move requests (e.g. can activate a button and move away from it)
    bool                    ActiveIdUsingAllKeyboardKeys;       // Active widget will want to read all keyboard keys inputs. (this is a shortcut for not taking ownership of 100+ keys, frequently used by drag operations)
    RblxUIKeyChord           DebugBreakInShortcutRouting;        // Set to break in SetShortcutRouting()/Shortcut() calls.
    //RblxU32                 ActiveIdUsingNavInputMask;          // [OBSOLETE] Since (RBLXUI_VERSION_NUM >= 18804) : 'g.ActiveIdUsingNavInputMask |= (1 << RblxUINavInput_Cancel);' becomes --> 'SetKeyOwner(RblxUIKey_Escape, g.ActiveId) and/or SetKeyOwner(RblxUIKey_NavGamepadCancel, g.ActiveId);'

    // Next window/item data
    RblxUIID                 CurrentFocusScopeId;                // Value for currently appending items == g.FocusScopeStack.back(). Not to be mistaken with g.NavFocusScopeId.
    RblxUIItemFlags          CurrentItemFlags;                   // Value for currently appending items == g.ItemFlagsStack.back()
    RblxUIID                 DebugLocateId;                      // Storage for DebugLocateItemOnHover() feature: this is read by ItemAdd() so we keep it in a hot/cached location
    RblxUINextItemData       NextItemData;                       // Storage for SetNextItem** functions
    RblxUILastItemData       LastItemData;                       // Storage for last submitted item (setup by ItemAdd)
    RblxUINextWindowData     NextWindowData;                     // Storage for SetNextWindow** functions
    bool                    DebugShowGroupRects;
    bool                    GcCompactAll;                       // Request full GC

    // Shared stacks
    RblxUICol                        DebugFlashStyleColorIdx;    // (Keep close to ColorStack to share cache line)
    RblxVector<RblxUIColorMod>         ColorStack;                 // Stack for PushStyleColor()/PopStyleColor() - inherited by Begin()
    RblxVector<RblxUIStyleMod>         StyleVarStack;              // Stack for PushStyleVar()/PopStyleVar() - inherited by Begin()
    RblxVector<RblxFontStackData>       FontStack;                  // Stack for PushFont()/PopFont() - inherited by Begin()
    RblxVector<RblxUIFocusScopeData>   FocusScopeStack;            // Stack for PushFocusScope()/PopFocusScope() - inherited by BeginChild(), pushed into by Begin()
    RblxVector<RblxUIItemFlags>        ItemFlagsStack;             // Stack for PushItemFlag()/PopItemFlag() - inherited by Begin()
    RblxVector<RblxUIGroupData>        GroupStack;                 // Stack for BeginGroup()/EndGroup() - not inherited by Begin()
    RblxVector<RblxUIPopupData>        OpenPopupStack;             // Which popups are open (persistent)
    RblxVector<RblxUIPopupData>        BeginPopupStack;            // Which level of BeginPopup() we are in (reset every frame)
    RblxVector<RblxUITreeNodeStackData>TreeNodeStack;              // Stack for TreeNode()

    // Viewports
    RblxVector<RblxUIViewportP*> Viewports;                        // Active viewports (always 1+, and generally 1 unless multi-viewports are enabled). Each viewports hold their copy of RblxDrawData.
    RblxUIViewportP*         CurrentViewport;                    // We track changes of viewport (happening in Begin) so we can call Platform_OnChangedViewport()
    RblxUIViewportP*         MouseViewport;
    RblxUIViewportP*         MouseLastHoveredViewport;           // Last known viewport that was hovered by mouse (even if we are not hovering any viewport any more) + honoring the _NoInputs flag.
    RblxUIID                 PlatformLastFocusedViewportId;
    RblxUIPlatformMonitor    FallbackMonitor;                    // Virtual monitor used as fallback if backend doesn't provide monitor information.
    RblxRect                  PlatformMonitorsFullWorkRect;       // Bounding box of all platform monitors
    int                     ViewportCreatedCount;               // Unique sequential creation counter (mostly for testing/debugging)
    int                     PlatformWindowsCreatedCount;        // Unique sequential creation counter (mostly for testing/debugging)
    int                     ViewportFocusedStampCount;          // Every time the front-most window changes, we stamp its viewport with an incrementing counter

    // Keyboard/Gamepad Navigation
    bool                    NavCursorVisible;                   // Nav focus cursor/rectangle is visible? We hide it after a mouse click. We show it after a nav move.
    bool                    NavHighlightItemUnderNav;           // Disable mouse hovering highlight. Highlight navigation focused item instead of mouse hovered item.
    //bool                  NavDisableHighlight;                // Old name for !g.NavCursorVisible before 1.91.4 (2024/10/18). OPPOSITE VALUE (g.NavDisableHighlight == !g.NavCursorVisible)
    //bool                  NavDisableMouseHover;               // Old name for g.NavHighlightItemUnderNav before 1.91.1 (2024/10/18) this was called When user starts using keyboard/gamepad, we hide mouse hovering highlight until mouse is touched again.
    bool                    NavMousePosDirty;                   // When set we will update mouse position if io.ConfigNavMoveSetMousePos is set (not enabled by default)
    bool                    NavIdIsAlive;                       // Nav widget has been seen this frame ~~ NavRectRel is valid
    RblxUIID                 NavId;                              // Focused item for navigation
    RblxUIWindow*            NavWindow;                          // Focused window for navigation. Could be called 'FocusedWindow'
    RblxUIID                 NavFocusScopeId;                    // Focused focus scope (e.g. selection code often wants to "clear other items" when landing on an item of the same scope)
    RblxUINavLayer           NavLayer;                           // Focused layer (main scrolling layer, or menu/title bar layer)
    RblxUIItemFlags          NavIdItemFlags;
    RblxUIID                 NavActivateId;                      // ~~ (g.ActiveId == 0) && (IsKeyPressed(RblxUIKey_Space) || IsKeyDown(RblxUIKey_Enter) || IsKeyPressed(RblxUIKey_NavGamepadActivate)) ? NavId : 0, also set when calling ActivateItemByID()
    RblxUIID                 NavActivateDownId;                  // ~~ IsKeyDown(RblxUIKey_Space) || IsKeyDown(RblxUIKey_Enter) || IsKeyDown(RblxUIKey_NavGamepadActivate) ? NavId : 0
    RblxUIID                 NavActivatePressedId;               // ~~ IsKeyPressed(RblxUIKey_Space) || IsKeyPressed(RblxUIKey_Enter) || IsKeyPressed(RblxUIKey_NavGamepadActivate) ? NavId : 0 (no repeat)
    RblxUIActivateFlags      NavActivateFlags;
    RblxVector<RblxUIFocusScopeData> NavFocusRoute;                // Reversed copy focus scope stack for NavId (should contains NavFocusScopeId). This essentially follow the window->ParentWindowForFocusRoute chain.
    RblxUIID                 NavHighlightActivatedId;
    float                   NavHighlightActivatedTimer;
    RblxUIID                 NavOpenContextMenuItemId;
    RblxUIID                 NavOpenContextMenuWindowId;
    RblxUIID                 NavNextActivateId;                  // Set by ActivateItemByID(), queued until next frame.
    RblxUIActivateFlags      NavNextActivateFlags;
    RblxUIInputSource        NavInputSource;                     // Keyboard or Gamepad mode? THIS CAN ONLY BE RblxUIInputSource_Keyboard or RblxUIInputSource_Gamepad
    RblxUISelectionUserData  NavLastValidSelectionUserData;      // Last valid data passed to SetNextItemSelectionUser(), or -1. For current window. Not reset when focusing an item that doesn't have selection data.
    RblxS8                    NavCursorHideFrames;
    //RblxUIID               NavActivateInputId;                 // Removed in 1.89.4 (July 2023). This is now part of g.NavActivateId and sets g.NavActivateFlags |= RblxUIActivateFlags_PreferInput. See commit c9a53aa74, issue #5606.

    // Navigation: Init & Move Requests
    bool                    NavAnyRequest;                      // ~~ NavMoveRequest || NavInitRequest this is to perform early out in ItemAdd()
    bool                    NavInitRequest;                     // Init request for appearing window to select first item
    bool                    NavInitRequestFromMove;
    RblxUINavItemData        NavInitResult;                      // Init request result (first item of the window, or one for which SetItemDefaultFocus() was called)
    bool                    NavMoveSubmitted;                   // Move request submitted, will process result on next NewFrame()
    bool                    NavMoveScoringItems;                // Move request submitted, still scoring incoming items
    bool                    NavMoveForwardToNextFrame;
    RblxUINavMoveFlags       NavMoveFlags;
    RblxUIScrollFlags        NavMoveScrollFlags;
    RblxUIKeyChord           NavMoveKeyMods;
    RblxUIDir                NavMoveDir;                         // Direction of the move request (left/right/up/down)
    RblxUIDir                NavMoveDirForDebug;
    RblxUIDir                NavMoveClipDir;                     // FIXME-NAV: Describe the purpose of this better. Might want to rename?
    RblxRect                  NavScoringRect;                     // Rectangle used for scoring, in screen space. Based of window->NavRectRel[], modified for directional navigation scoring.
    RblxRect                  NavScoringNoClipRect;               // Some nav operations (such as PageUp/PageDown) enforce a region which clipper will attempt to always keep submitted. Unset/invalid if inverted.
    int                     NavScoringDebugCount;               // Metrics for debugging
    int                     NavTabbingDir;                      // Generally -1 or +1, 0 when tabbing without a nav id
    int                     NavTabbingCounter;                  // >0 when counting items for tabbing
    RblxUINavItemData        NavMoveResultLocal;                 // Best move request candidate within NavWindow
    RblxUINavItemData        NavMoveResultLocalVisible;          // Best move request candidate within NavWindow that are mostly visible (when using RblxUINavMoveFlags_AlsoScoreVisibleSet flag)
    RblxUINavItemData        NavMoveResultOther;                 // Best move request candidate within NavWindow's flattened hierarchy (when using RblxUIWindowFlags_NavFlattened flag)
    RblxUINavItemData        NavTabbingResultFirst;              // First tabbing request candidate within NavWindow and flattened hierarchy

    // Navigation: record of last move request
    RblxUIID                 NavJustMovedFromFocusScopeId;       // Just navigated from this focus scope id (result of a successfully MoveRequest).
    RblxUIID                 NavJustMovedToId;                   // Just navigated to this id (result of a successfully MoveRequest).
    RblxUIID                 NavJustMovedToFocusScopeId;         // Just navigated to this focus scope id (result of a successfully MoveRequest).
    RblxUIKeyChord           NavJustMovedToKeyMods;
    bool                    NavJustMovedToIsTabbing;            // Copy of RblxUINavMoveFlags_IsTabbing. Maybe we should store whole flags.
    bool                    NavJustMovedToHasSelectionData;     // Copy of move result's ItemFlags & RblxUIItemFlags_HasSelectionUserData). Maybe we should just store RblxUINavItemData.

    // Navigation: extra config options (will be made public eventually)
    // - Tabbing (Tab, Shift+Tab) and Windowing (Ctrl+Tab, Ctrl+Shift+Tab) are enabled REGARDLESS of RblxUIConfigFlags_NavEnableKeyboard being set.
    // - Ctrl+Tab is reconfigurable because it is the only shortcut that may be polled when no window are focused. It also doesn't work e.g. Web platforms.
    bool                    ConfigNavEnableTabbing;             // = true. Enable tabbing (Tab, Shift+Tab). PLEASE LET ME KNOW IF YOU USE THIS.
    bool                    ConfigNavWindowingWithGamepad;      // = true. Enable Ctrl+Tab by holding RblxUIKey_GamepadFaceLeft (== RblxUIKey_NavGamepadMenu). When false, the button may still be used to toggle Menu layer.
    RblxUIKeyChord           ConfigNavWindowingKeyNext;          // = RblxUIMod_Ctrl | RblxUIKey_Tab (or RblxUIMod_Super | RblxUIKey_Tab on OS X). Set to 0 to disable. For reconfiguration (see #4828)
    RblxUIKeyChord           ConfigNavWindowingKeyPrev;          // = RblxUIMod_Ctrl | RblxUIMod_Shift | RblxUIKey_Tab (or RblxUIMod_Super | RblxUIMod_Shift | RblxUIKey_Tab on OS X)

    // Navigation: Windowing (Ctrl+Tab for list, or Menu button + keys or directional pads to move/resize)
    RblxUIWindow*            NavWindowingTarget;                 // Target window when doing Ctrl+Tab (or Pad Menu + FocusPrev/Next), this window is temporarily displayed top-most!
    RblxUIWindow*            NavWindowingTargetAnim;             // Record of last valid NavWindowingTarget until DimBgRatio and NavWindowingHighlightAlpha becomes 0.0f, so the fade-out can stay on it.
    RblxUIWindow*            NavWindowingListWindow;             // Internal window actually listing the Ctrl+Tab contents
    float                   NavWindowingTimer;
    float                   NavWindowingHighlightAlpha;
    RblxUIInputSource        NavWindowingInputSource;
    bool                    NavWindowingToggleLayer;            // Set while Alt or GamepadMenu is held, may be cleared by other operations, and processed when releasing the key.
    RblxUIKey                NavWindowingToggleKey;              // Keyboard/gamepad key used when toggling to menu layer.
    RblxVec2                  NavWindowingAccumDeltaPos;
    RblxVec2                  NavWindowingAccumDeltaSize;

    // Render
    float                   DimBgRatio;                         // 0.0..1.0 animation when fading in a dimming background (for modal window and Ctrl+Tab list)

    // Drag and Drop
    bool                    DragDropActive;
    bool                    DragDropWithinSource;               // Set when within a BeginDragDropXXX/EndDragDropXXX block for a drag source.
    bool                    DragDropWithinTarget;               // Set when within a BeginDragDropXXX/EndDragDropXXX block for a drag target.
    RblxUIDragDropFlags      DragDropSourceFlags;
    int                     DragDropSourceFrameCount;
    int                     DragDropMouseButton;
    RblxUIPayload            DragDropPayload;
    RblxRect                  DragDropTargetRect;                 // Store rectangle of current target candidate (we favor small targets when overlapping)
    RblxRect                  DragDropTargetClipRect;             // Store ClipRect at the time of item's drawing
    RblxUIID                 DragDropTargetId;
    RblxUIID                 DragDropTargetFullViewport;
    RblxUIDragDropFlags      DragDropAcceptFlagsCurr;
    RblxUIDragDropFlags      DragDropAcceptFlagsPrev;
    float                   DragDropAcceptIdCurrRectSurface;    // Target item surface (we resolve overlapping targets by prioritizing the smaller surface)
    RblxUIID                 DragDropAcceptIdCurr;               // Target item id (set at the time of accepting the payload)
    RblxUIID                 DragDropAcceptIdPrev;               // Target item id from previous frame (we need to store this to allow for overlapping drag and drop targets)
    int                     DragDropAcceptFrameCount;           // Last time a target expressed a desire to accept the source
    RblxUIID                 DragDropHoldJustPressedId;          // Set when holding a payload just made ButtonBehavior() return a press.
    RblxVector<unsigned char> DragDropPayloadBufHeap;             // We don't expose the RblxVector<> directly, RblxUIPayload only holds pointer+size
    unsigned char           DragDropPayloadBufLocal[16];        // Local buffer for small payloads

    // Clipper
    int                             ClipperTempDataStacked;
    RblxVector<RblxUIListClipperData>  ClipperTempData;

    // Tables
    RblxUITable*                     CurrentTable;
    RblxUIID                         DebugBreakInTable;          // Set to break in BeginTable() call.
    int                             TablesTempDataStacked;      // Temporary table data size (because we leave previous instances undestructed, we generally don't use TablesTempData.Size)
    RblxVector<RblxUITableTempData>    TablesTempData;             // Temporary table data (buffers reused/shared across instances, support nesting)
    RblxPool<RblxUITable>              Tables;                     // Persistent table data
    RblxVector<float>                 TablesLastTimeActive;       // Last used timestamp of each tables (SOA, for efficient GC)
    RblxVector<RblxDrawChannel>         DrawChannelsTempMergeBuffer;

    // Tab bars
    RblxUITabBar*                    CurrentTabBar;
    RblxPool<RblxUITabBar>             TabBars;
    RblxVector<RblxUIPtrOrIndex>       CurrentTabBarStack;
    RblxVector<RblxUIShrinkWidthItem>  ShrinkWidthBuffer;

    // Multi-Select state
    RblxUIBoxSelectState             BoxSelectState;
    RblxUIMultiSelectTempData*       CurrentMultiSelect;
    int                             MultiSelectTempDataStacked; // Temporary multi-select data size (because we leave previous instances undestructed, we generally don't use MultiSelectTempData.Size)
    RblxVector<RblxUIMultiSelectTempData> MultiSelectTempData;
    RblxPool<RblxUIMultiSelectState>   MultiSelectStorage;

    // Hover Delay system
    RblxUIID                 HoverItemDelayId;
    RblxUIID                 HoverItemDelayIdPreviousFrame;
    float                   HoverItemDelayTimer;                // Currently used by IsItemHovered()
    float                   HoverItemDelayClearTimer;           // Currently used by IsItemHovered(): grace time before g.TooltipHoverTimer gets cleared.
    RblxUIID                 HoverItemUnlockedStationaryId;      // Mouse has once been stationary on this item. Only reset after departing the item.
    RblxUIID                 HoverWindowUnlockedStationaryId;    // Mouse has once been stationary on this window. Only reset after departing the window.

    // Mouse state
    RblxUIMouseCursor        MouseCursor;
    float                   MouseStationaryTimer;               // Time the mouse has been stationary (with some loose heuristic)
    RblxVec2                  MouseLastValidPos;

    // Widget state
    RblxUIInputTextState     InputTextState;
    RblxUITextIndex          InputTextLineIndex;                 // Temporary storage
    RblxUIInputTextDeactivatedState InputTextDeactivatedState;
    RblxFontBaked             InputTextPasswordFontBackupBaked;
    RblxFontFlags             InputTextPasswordFontBackupFlags;
    RblxUIID                 InputTextReactivateId;              // ID of InputText to reactivate on next frame (for io.ConfigInputTextEnterKeepActive behavior)
    RblxUIID                 TempInputId;                        // Temporary text input when using Ctrl+Click on a slider, etc.
    RblxUIDataTypeStorage    DataTypeZeroValue;                  // 0 for all data types
    int                     BeginMenuDepth;
    int                     BeginComboDepth;
    RblxUIColorEditFlags     ColorEditOptions;                   // Store user options for color edit widgets
    RblxUIID                 ColorEditCurrentID;                 // Set temporarily while inside of the parent-most ColorEdit4/ColorPicker4 (because they call each others).
    RblxUIID                 ColorEditSavedID;                   // ID we are saving/restoring HS for
    float                   ColorEditSavedHue;                  // Backup of last Hue associated to LastColor, so we can restore Hue in lossy RGB<>HSV round trips
    float                   ColorEditSavedSat;                  // Backup of last Saturation associated to LastColor, so we can restore Saturation in lossy RGB<>HSV round trips
    RblxU32                   ColorEditSavedColor;                // RGB value with alpha set to 0.
    RblxVec4                  ColorPickerRef;                     // Initial/reference color at the time of opening the color picker.
    RblxUIComboPreviewData   ComboPreviewData;
    RblxRect                  WindowResizeBorderExpectedRect;     // Expected border rect, switch to relative edit if moving
    bool                    WindowResizeRelativeMode;
    short                   ScrollbarSeekMode;                  // 0: scroll to clicked location, -1/+1: prev/next page.
    float                   ScrollbarClickDeltaToGrabCenter;    // When scrolling to mouse location: distance between mouse and center of grab box, normalized in parent space.
    float                   SliderGrabClickOffset;
    float                   SliderCurrentAccum;                 // Accumulated slider delta when using navigation controls.
    bool                    SliderCurrentAccumDirty;            // Has the accumulated slider delta changed since last time we tried to apply it?
    bool                    DragCurrentAccumDirty;
    float                   DragCurrentAccum;                   // Accumulator for dragging modification. Always high-precision, not rounded by end-user precision settings
    float                   DragSpeedDefaultRatio;              // If speed == 0.0f, uses (max-min) * DragSpeedDefaultRatio
    float                   DisabledAlphaBackup;                // Backup for style.Alpha for BeginDisabled()
    short                   DisabledStackSize;
    short                   TooltipOverrideCount;
    RblxUIWindow*            TooltipPreviousWindow;              // Window of last tooltip submitted during the frame
    RblxVector<char>          ClipboardHandlerData;               // If no custom clipboard handler is defined
    RblxVector<RblxUIID>       MenusIdSubmittedThisFrame;          // A list of menu IDs that were rendered at least once
    RblxUITypingSelectState  TypingSelectState;                  // State for GetTypingSelectRequest()

    // Platform support
    RblxUIPlatformImeData    PlatformImeData;                    // Data updated by current frame. Will be applied at end of the frame. For some backends, this is required to have WantVisible=true in order to receive text message.
    RblxUIPlatformImeData    PlatformImeDataPrev;                // Previous frame data. When changed we call the platform_io.Platform_SetImeDataFn() handler.

    // Extensions
    // FIXME: We could provide an API to register one slot in an array held in RblxUIContext?
    RblxVector<RblxTextureData*> UserTextures;                      // List of textures created/managed by user or third-party extension. Automatically appended into platform_io.Textures[].
    RblxUIDockContext        DockContext;
    void                    (*DockNodeWindowMenuHandler)(RblxUIContext* ctx, RblxUIDockNode* node, RblxUITabBar* tab_bar);

    // Settings
    bool                    SettingsLoaded;
    float                   SettingsDirtyTimer;                 // Save .ini Settings to memory when time reaches zero
    RblxUITextBuffer         SettingsIniData;                    // In memory .ini settings
    RblxVector<RblxUISettingsHandler>      SettingsHandlers;       // List of .ini settings handlers
    RblxChunkStream<RblxUIWindowSettings>  SettingsWindows;        // RblxUIWindow .ini settings entries
    RblxChunkStream<RblxUITableSettings>   SettingsTables;         // RblxUITable .ini settings entries

    // Hooks
    RblxVector<RblxUIContextHook>          Hooks;                  // Hooks for extensions (e.g. test engine)
    RblxUIID                             HookIdNext;             // Next available HookId
    RblxUIDemoMarkerCallback             DemoMarkerCallback;

    // Localization
    const char*             LocalizationTable[RblxUILocKey_COUNT];

    // Capture/Logging
    bool                    LogEnabled;                         // Currently capturing
    bool                    LogLineFirstItem;
    RblxUILogFlags           LogFlags;                           // Capture flags/type
    RblxUIWindow*            LogWindow;
    RblxFileHandle            LogFile;                            // If != NULL log to stdout/ file
    RblxUITextBuffer         LogBuffer;                          // Accumulation buffer when log to clipboard. This is pointer so our GRblxUI static constructor doesn't call heap allocators.
    const char*             LogNextPrefix;                      // See comment in LogSetNextTextDecoration(): doesn't copy underlying data, use carefully!
    const char*             LogNextSuffix;
    float                   LogLinePosY;
    int                     LogDepthRef;
    int                     LogDepthToExpand;
    int                     LogDepthToExpandDefault;            // Default/stored value for LogDepthMaxExpand if not specified in the LogXXX function call.

    // Error Handling
    RblxUIErrorCallback      ErrorCallback;                      // = NULL. May be exposed in public API eventually.
    void*                   ErrorCallbackUserData;              // = NULL
    RblxVec2                  ErrorTooltipLockedPos;
    bool                    ErrorFirst;
    int                     ErrorCountCurrentFrame;             // [Internal] Number of errors submitted this frame.
    RblxUIErrorRecoveryState StackSizesInNewFrame;               // [Internal]
    RblxUIErrorRecoveryState*StackSizesInBeginForCurrentWindow;  // [Internal]

    // Debug Tools
    // (some of the highly frequently used data are interleaved in other structures above: DebugBreakXXX fields, DebugHookIdInfo, DebugLocateId etc.)
    int                     DebugDrawIdConflictsCount;          // Locked count (preserved when holding Ctrl)
    RblxUIDebugLogFlags      DebugLogFlags;
    RblxUITextBuffer         DebugLogBuf;
    RblxUITextIndex          DebugLogIndex;
    int                     DebugLogSkippedErrors;
    RblxUIDebugLogFlags      DebugLogAutoDisableFlags;
    RblxU8                    DebugLogAutoDisableFrames;
    RblxU8                    DebugLocateFrames;                  // For DebugLocateItemOnHover(). This is used together with DebugLocateId which is in a hot/cached spot above.
    bool                    DebugBreakInLocateId;               // Debug break in ItemAdd() call for g.DebugLocateId.
    RblxUIKeyChord           DebugBreakKeyChord;                 // = RblxUIKey_Pause
    RblxS8                    DebugBeginReturnValueCullDepth;     // Cycle between 0..9 then wrap around.
    bool                    DebugItemPickerActive;              // Item picker is active (started with DebugStartItemPicker())
    RblxU8                    DebugItemPickerMouseButton;
    RblxUIID                 DebugItemPickerBreakId;             // Will call RBLXUI_DEBUG_BREAK() when encountering this ID
    float                   DebugFlashStyleColorTime;
    RblxVec4                  DebugFlashStyleColorBackup;
    RblxUIMetricsConfig      DebugMetricsConfig;
    RblxUIDebugItemPathQuery DebugItemPathQuery;
    RblxUIIDStackTool        DebugIDStackTool;
    RblxUIDebugAllocInfo     DebugAllocInfo;
    RblxUIDockNode*          DebugHoveredDockNode;               // Hovered dock node.
#if defined(RBLXUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS) && !defined(RBLXUI_DISABLE_DEBUG_TOOLS)
    RblxUIStorage            DebugDrawIdConflictsAliveCount;
    RblxUIStorage            DebugDrawIdConflictsHighlightSet;
#endif

    // Misc
    float                   FramerateSecPerFrame[60];           // Calculate estimate of framerate for user over the last 60 frames..
    int                     FramerateSecPerFrameIdx;
    int                     FramerateSecPerFrameCount;
    float                   FramerateSecPerFrameAccum;
    int                     WantCaptureMouseNextFrame;          // Explicit capture override via SetNextFrameWantCaptureMouse()/SetNextFrameWantCaptureKeyboard(). Default to -1.
    int                     WantCaptureKeyboardNextFrame;       // "
    int                     WantTextInputNextFrame;             // Copied in EndFrame() from g.PlatformImeData.WantTextInput. Needs to be set for some backends (SDL3) to emit character inputs.
    RblxVector<char>          TempBuffer;                         // Temporary text buffer
    char                    TempKeychordName[64];

    RblxUIContext(RblxFontAtlas* shared_font_atlas);
    ~RblxUIContext();
};

//-----------------------------------------------------------------------------
// [SECTION] RblxUIWindowTempData, RblxUIWindow
//-----------------------------------------------------------------------------

#define RBLXUI_WINDOW_HARD_MIN_SIZE 4.0f

// Transient per-window data, reset at the beginning of the frame. This used to be called RblxUIDrawContext, hence the DC variable name in RblxUIWindow.
// (That's theory, in practice the delimitation between RblxUIWindow and RblxUIWindowTempData is quite tenuous and could be reconsidered..)
// (This doesn't need a constructor because we zero-clear it as part of RblxUIWindow and all frame-temporary data are setup on Begin)
struct RBLXUI_API RblxUIWindowTempData
{
    // Layout
    RblxVec2                  CursorPos;              // Current emitting position, in absolute coordinates.
    RblxVec2                  CursorPosPrevLine;
    RblxVec2                  CursorStartPos;         // Initial position after Begin(), generally ~ window position + WindowPadding.
    RblxVec2                  CursorMaxPos;           // Used to implicitly calculate ContentSize at the beginning of next frame, for scrolling range and auto-resize. Always growing during the frame.
    RblxVec2                  IdealMaxPos;            // Used to implicitly calculate ContentSizeIdeal at the beginning of next frame, for auto-resize only. Always growing during the frame.
    RblxVec2                  CurrLineSize;
    RblxVec2                  PrevLineSize;
    float                   CurrLineTextBaseOffset; // Baseline offset (0.0f by default on a new line, generally == style.FramePadding.y when a framed item has been added).
    float                   PrevLineTextBaseOffset;
    bool                    IsSameLine;
    bool                    IsSetPos;
    RblxVec1                  Indent;                 // Indentation / start position from left of window (increased by TreePush/TreePop, etc.)
    RblxVec1                  ColumnsOffset;          // Offset to the current column (if ColumnsCurrent > 0). FIXME: This and the above should be a stack to allow use cases like Tree->Column->Tree. Need revamp columns API.
    RblxVec1                  GroupOffset;
    RblxVec2                  CursorStartPosLossyness;// Record the loss of precision of CursorStartPos due to really large scrolling amount. This is used by clipper to compensate and fix the most common use case of large scroll area.

    // Keyboard/Gamepad navigation
    RblxUINavLayer           NavLayerCurrent;        // Current layer, 0..31 (we currently only use 0..1)
    short                   NavLayersActiveMask;    // Which layers have been written to (result from previous frame)
    short                   NavLayersActiveMaskNext;// Which layers have been written to (accumulator for current frame)
    bool                    NavIsScrollPushableX;   // Set when current work location may be scrolled horizontally when moving left / right. This is generally always true UNLESS within a column.
    bool                    NavHideHighlightOneFrame;
    bool                    NavWindowHasScrollY;    // Set per window when scrolling can be used (== ScrollMax.y > 0.0f)

    // Miscellaneous
    bool                    MenuBarAppending;       // FIXME: Remove this
    RblxVec2                  MenuBarOffset;          // MenuBarOffset.x is sort of equivalent of a per-layer CursorPos.x, saved/restored as we switch to the menu bar. The only situation when MenuBarOffset.y is > 0 if when (SafeAreaPadding.y > FramePadding.y), often used on TVs.
    RblxUIMenuColumns        MenuColumns;            // Simplified columns storage for menu items measurement
    int                     TreeDepth;              // Current tree depth.
    RblxU32                   TreeHasStackDataDepthMask;      // Store whether given depth has RblxUITreeNodeStackData data. Could be turned into a RblxU64 if necessary.
    RblxU32                   TreeRecordsClippedNodesY2Mask;  // Store whether we should keep recording Y2. Cleared when passing clip max. Equivalent TreeHasStackDataDepthMask value should always be set.
    RblxVector<RblxUIWindow*>  ChildWindows;
    RblxUIStorage*           StateStorage;           // Current persistent per-window storage (store e.g. tree node open/close state)
    RblxUIOldColumns*        CurrentColumns;         // Current columns set
    int                     CurrentTableIdx;        // Current table index (into g.Tables)
    RblxUILayoutType         LayoutType;
    RblxUILayoutType         ParentLayoutType;       // Layout type of parent window at the time of Begin()
    RblxU32                   ModalDimBgColor;

    // Status flags
    RblxUIItemStatusFlags    WindowItemStatusFlags;
    RblxUIItemStatusFlags    ChildItemStatusFlags;
    RblxUIItemStatusFlags    DockTabItemStatusFlags;
    RblxRect                  DockTabItemRect;

    // Local parameters stacks
    // We store the current settings outside of the vectors to increase memory locality (reduce cache misses). The vectors are rarely modified. Also it allows us to not heap allocate for short-lived windows which are not using those settings.
    float                   ItemWidth;              // Current item width (>0.0: width in pixels, <0.0: align xx pixels to the right of window).
    float                   ItemWidthDefault;
    float                   TextWrapPos;            // Current text wrap pos.
    RblxVector<float>         ItemWidthStack;         // Store item widths to restore (attention: .back() is not == ItemWidth)
    RblxVector<float>         TextWrapPosStack;       // Store text wrap pos to restore (attention: .back() is not == TextWrapPos)
};

// Storage for one window
struct RBLXUI_API RblxUIWindow
{
    RblxUIContext*           Ctx;                                // Parent UI context (needs to be set explicitly by parent).
    char*                   Name;                               // Window name, owned by the window.
    RblxUIID                 ID;                                 // == RblxHashStr(Name)
    RblxUIWindowFlags        Flags, FlagsPreviousFrame;          // See enum RblxUIWindowFlags_
    RblxUIChildFlags         ChildFlags;                         // Set when window is a child window. See enum RblxUIChildFlags_
    RblxUIWindowClass        WindowClass;                        // Advanced users only. Set with SetNextWindowClass()
    RblxUIViewportP*         Viewport;                           // Always set in Begin(). Inactive windows may have a NULL value here if their viewport was discarded.
    RblxUIID                 ViewportId;                         // We backup the viewport id (since the viewport may disappear or never be created if the window is inactive)
    RblxVec2                  ViewportPos;                        // We backup the viewport position (since the viewport may disappear or never be created if the window is inactive)
    int                     ViewportAllowPlatformMonitorExtend; // Reset to -1 every frame (index is guaranteed to be valid between NewFrame..EndFrame), only used in the Appearing frame of a tooltip/popup to enforce clamping to a given monitor
    RblxVec2                  Pos;                                // Position (always rounded-up to nearest pixel)
    RblxVec2                  Size;                               // Current size (==SizeFull or collapsed title bar size)
    RblxVec2                  SizeFull;                           // Size when non collapsed
    RblxVec2                  ContentSize;                        // Size of contents/scrollable client area (calculated from the extents reach of the cursor) from previous frame. Does not include window decoration or window padding.
    RblxVec2                  ContentSizeIdeal;
    RblxVec2                  ContentSizeExplicit;                // Size of contents/scrollable client area explicitly request by the user via SetNextWindowContentSize().
    RblxVec2                  WindowPadding;                      // Window padding at the time of Begin().
    float                   WindowRounding;                     // Window rounding at the time of Begin(). May be clamped lower to avoid rendering artifacts with title bar, menu bar etc.
    float                   WindowBorderSize;                   // Window border size at the time of Begin().
    float                   TitleBarHeight, MenuBarHeight;      // Note that those used to be function before 2024/05/28. If you have old code calling TitleBarHeight() you can change it to TitleBarHeight.
    float                   DecoOuterSizeX1, DecoOuterSizeY1;   // Left/Up offsets. Sum of non-scrolling outer decorations (X1 generally == 0.0f. Y1 generally = TitleBarHeight + MenuBarHeight). Locked during Begin().
    float                   DecoOuterSizeX2, DecoOuterSizeY2;   // Right/Down offsets (X2 generally == ScrollbarSize.x, Y2 == ScrollbarSizes.y).
    float                   DecoInnerSizeX1, DecoInnerSizeY1;   // Applied AFTER/OVER InnerRect. Specialized for Tables as they use specialized form of clipping and frozen rows/columns are inside InnerRect (and not part of regular decoration sizes).
    int                     NameBufLen;                         // Size of buffer storing Name. May be larger than strlen(Name)!
    RblxUIID                 MoveId;                             // == window->GetID("#MOVE")
    RblxUIID                 TabId;                              // == window->GetID("#TAB")
    RblxUIID                 ChildId;                            // ID of corresponding item in parent window (for navigation to return from child window to parent window)
    RblxUIID                 PopupId;                            // ID in the popup stack when this window is used as a popup/menu (because we use generic Name/ID for recycling)
    RblxVec2                  Scroll;
    RblxVec2                  ScrollMax;
    RblxVec2                  ScrollTarget;                       // target scroll position. stored as cursor position with scrolling canceled out, so the highest point is always 0.0f. (FLT_MAX for no change)
    RblxVec2                  ScrollTargetCenterRatio;            // 0.0f = scroll so that target position is at top, 0.5f = scroll so that target position is centered
    RblxVec2                  ScrollTargetEdgeSnapDist;           // 0.0f = no snapping, >0.0f snapping threshold
    RblxVec2                  ScrollbarSizes;                     // Size taken by each scrollbars on their smaller axis. Pay attention! ScrollbarSizes.x == width of the vertical scrollbar, ScrollbarSizes.y = height of the horizontal scrollbar.
    bool                    ScrollbarX, ScrollbarY;             // Are scrollbars visible?
    bool                    ScrollbarXStabilizeEnabled;         // Was ScrollbarX previously auto-stabilized?
    RblxU8                    ScrollbarXStabilizeToggledHistory;  // Used to stabilize scrollbar visibility in case of feedback loops
    bool                    ViewportOwned;
    bool                    Active;                             // Set to true on Begin(), unless Collapsed
    bool                    WasActive;
    bool                    WriteAccessed;                      // Set to true when any widget access the current window
    bool                    Collapsed;                          // Set when collapsing window to become only title-bar
    bool                    WantCollapseToggle;
    bool                    SkipItems;                          // Set when items can safely be all clipped (e.g. window not visible or collapsed)
    bool                    SkipRefresh;                        // [EXPERIMENTAL] Reuse previous frame drawn contents, Begin() returns false.
    bool                    Appearing;                          // Set during the frame where the window is appearing (or re-appearing)
    bool                    Hidden;                             // Do not display (== HiddenFrames*** > 0)
    bool                    IsFallbackWindow;                   // Set on the "Debug##Default" window.
    bool                    IsExplicitChild;                    // Set when passed _ChildWindow, left to false by BeginDocked()
    bool                    HasCloseButton;                     // Set when the window has a close button (p_open != NULL)
    signed char             ResizeBorderHovered;                // Current border being hovered for resize (-1: none, otherwise 0-3)
    signed char             ResizeBorderHeld;                   // Current border being held for resize (-1: none, otherwise 0-3)
    short                   BeginCount;                         // Number of Begin() during the current frame (generally 0 or 1, 1+ if appending via multiple Begin/End pairs)
    short                   BeginCountPreviousFrame;            // Number of Begin() during the previous frame
    short                   BeginOrderWithinParent;             // Begin() order within immediate parent window, if we are a child window. Otherwise 0.
    short                   BeginOrderWithinContext;            // Begin() order within entire rblxui context. This is mostly used for debugging submission order related issues.
    short                   FocusOrder;                         // Order within WindowsFocusOrder[], altered when windows are focused.
    RblxUIDir                AutoPosLastDirection;
    RblxS8                    AutoFitFramesX, AutoFitFramesY;
    bool                    AutoFitOnlyGrows;
    RblxS8                    HiddenFramesCanSkipItems;           // Hide the window for N frames
    RblxS8                    HiddenFramesCannotSkipItems;        // Hide the window for N frames while allowing items to be submitted so we can measure their size
    RblxS8                    HiddenFramesForRenderOnly;          // Hide the window until frame N at Render() time only
    RblxS8                    DisableInputsFrames;                // Disable window interactions for N frames
    RblxUIWindowBgClickFlags BgClickFlags : 8;                   // Configure behavior of click+dragging on window bg/void or over items. Default sets by io.ConfigWindowsMoveFromTitleBarOnly. If you use this please report in #3379.
    RblxUICond               SetWindowPosAllowFlags : 8;         // store acceptable condition flags for SetNextWindowPos() use.
    RblxUICond               SetWindowSizeAllowFlags : 8;        // store acceptable condition flags for SetNextWindowSize() use.
    RblxUICond               SetWindowCollapsedAllowFlags : 8;   // store acceptable condition flags for SetNextWindowCollapsed() use.
    RblxUICond               SetWindowDockAllowFlags : 8;        // store acceptable condition flags for SetNextWindowDock() use.
    RblxVec2                  SetWindowPosVal;                    // store window position when using a non-zero Pivot (position set needs to be processed when we know the window size)
    RblxVec2                  SetWindowPosPivot;                  // store window pivot for positioning. RblxVec2(0, 0) when positioning from top-left corner; RblxVec2(0.5f, 0.5f) for centering; RblxVec2(1, 1) for bottom right.

    RblxVector<RblxUIID>       IDStack;                            // ID stack. ID are hashes seeded with the value at the top of the stack. (In theory this should be in the TempData structure)
    RblxUIWindowTempData     DC;                                 // Temporary per-window data, reset at the beginning of the frame. This used to be called RblxUIDrawContext, hence the "DC" variable name.

    // The best way to understand what those rectangles are is to use the 'Metrics->Tools->Show Windows Rectangles' viewer.
    // The main 'OuterRect', omitted as a field, is window->Rect().
    RblxRect                  OuterRectClipped;                   // == Window->Rect() just after setup in Begin(). == window->Rect() for root window.
    RblxRect                  InnerRect;                          // Inner rectangle (omit title bar, menu bar, scroll bar)
    RblxRect                  InnerClipRect;                      // == InnerRect shrunk by WindowPadding*0.5f on each side, clipped within viewport or parent clip rect.
    RblxRect                  WorkRect;                           // Initially covers the whole scrolling region. Reduced by containers e.g columns/tables when active. Shrunk by WindowPadding*1.0f on each side. This is meant to replace ContentRegionRect over time (from 1.71+ onward).
    RblxRect                  ParentWorkRect;                     // Backup of WorkRect before entering a container such as columns/tables. Used by e.g. SpanAllColumns functions to easily access. Stacked containers are responsible for maintaining this. // FIXME-WORKRECT: Could be a stack?
    RblxRect                  ClipRect;                           // Current clipping/scissoring rectangle, evolve as we are using PushClipRect(), etc. == DrawList->clip_rect_stack.back().
    RblxRect                  ContentRegionRect;                  // FIXME: This is currently confusing/misleading. It is essentially WorkRect but not handling of scrolling. We currently rely on it as right/bottom aligned sizing operation need some size to rely on.
    RblxVec2ih                HitTestHoleSize;                    // Define an optional rectangular hole where mouse will pass-through the window.
    RblxVec2ih                HitTestHoleOffset;

    int                     LastFrameActive;                    // Last frame number the window was Active.
    int                     LastFrameJustFocused;               // Last frame number the window was made Focused.
    float                   LastTimeActive;                     // Last timestamp the window was Active (using float as we don't need high precision there)
    RblxUIStorage            StateStorage;
    RblxVector<RblxUIOldColumns> ColumnsStorage;
    float                   FontWindowScale;                    // User scale multiplier per-window, via SetWindowFontScale()
    float                   FontWindowScaleParents;
    float                   FontRefSize;                        // This is a copy of window->CalcFontSize() at the time of Begin(), trying to phase out CalcFontSize() especially as it may be called on non-current window.
    int                     SettingsOffset;                     // Offset into SettingsWindows[] (offsets are always valid as we only grow the array from the back)

    RblxDrawList*             DrawList;                           // == &DrawListInst (for backward compatibility reason with code using rblxui_internal.h we keep this a pointer)
    RblxDrawList              DrawListInst;
    RblxUIWindow*            ParentWindow;                       // If we are a child _or_ popup _or_ docked window, this is pointing to our parent. Otherwise NULL.
    RblxUIWindow*            ParentWindowInBeginStack;
    RblxUIWindow*            RootWindow;                         // Point to ourself or first ancestor that is not a child window. Doesn't cross through popups/dock nodes.
    RblxUIWindow*            RootWindowPopupTree;                // Point to ourself or first ancestor that is not a child window. Cross through popups parent<>child.
    RblxUIWindow*            RootWindowDockTree;                 // Point to ourself or first ancestor that is not a child window. Cross through dock nodes.
    RblxUIWindow*            RootWindowForTitleBarHighlight;     // Point to ourself or first ancestor which will display TitleBgActive color when this window is active.
    RblxUIWindow*            RootWindowForNav;                   // Point to ourself or first ancestor which doesn't have the NavFlattened flag.
    RblxUIWindow*            ParentWindowForFocusRoute;          // Set to manual link a window to its logical parent so that Shortcut() chain are honored (e.g. Tool linked to Document)

    RblxUIWindow*            NavLastChildNavWindow;              // When going to the menu bar, we remember the child window we came from. (This could probably be made implicit if we kept g.Windows sorted by last focused including child window.)
    RblxUIID                 NavLastIds[RblxUINavLayer_COUNT];    // Last known NavId for this window, per layer (0/1)
    RblxRect                  NavRectRel[RblxUINavLayer_COUNT];    // Reference rectangle, in window relative space
    RblxVec2                  NavPreferredScoringPosRel[RblxUINavLayer_COUNT]; // Preferred X/Y position updated when moving on a given axis, reset to FLT_MAX.
    RblxUIID                 NavRootFocusScopeId;                // Focus Scope ID at the time of Begin()

    int                     MemoryDrawListIdxCapacity;          // Backup of last idx/vtx count, so when waking up the window we can preallocate and avoid iterative alloc/copy
    int                     MemoryDrawListVtxCapacity;
    bool                    MemoryCompacted;                    // Set when window extraneous data have been garbage collected

    // Docking
    bool                    DockIsActive        :1;             // When docking artifacts are actually visible. When this is set, DockNode is guaranteed to be != NULL. ~~ (DockNode != NULL) && (DockNode->Windows.Size > 1).
    bool                    DockNodeIsVisible   :1;
    bool                    DockTabIsVisible    :1;             // Is our window visible this frame? ~~ is the corresponding tab selected?
    bool                    DockTabWantClose    :1;
    short                   DockOrder;                          // Order of the last time the window was visible within its DockNode. This is used to reorder windows that are reappearing on the same frame. Same value between windows that were active and windows that were none are possible.
    RblxUIWindowDockStyle    DockStyle;
    RblxUIDockNode*          DockNode;                           // Which node are we docked into. Important: Prefer testing DockIsActive in many cases as this will still be set when the dock node is hidden.
    RblxUIDockNode*          DockNodeAsHost;                     // Which node are we owning (for parent windows)
    RblxUIID                 DockId;                             // Backup of last valid DockNode->ID, so single window remember their dock node id even when they are not bound any more

public:
    RblxUIWindow(RblxUIContext* context, const char* name);
    ~RblxUIWindow();

    RblxUIID     GetID(const char* str, const char* str_end = NULL);
    RblxUIID     GetID(const void* ptr);
    RblxUIID     GetID(int n);
    RblxUIID     GetIDFromPos(const RblxVec2& p_abs);
    RblxUIID     GetIDFromRectangle(const RblxRect& r_abs);

    // We don't use g.FontSize because the window may be != g.CurrentWindow.
    RblxRect      Rect() const            { return RblxRect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y); }
    RblxRect      TitleBarRect() const    { return RblxRect(Pos, RblxVec2(Pos.x + SizeFull.x, Pos.y + TitleBarHeight)); }
    RblxRect      MenuBarRect() const     { float y1 = Pos.y + TitleBarHeight; return RblxRect(Pos.x, y1, Pos.x + SizeFull.x, y1 + MenuBarHeight); }

    // [OBSOLETE] RblxUIWindow::CalcFontSize() was removed in 1.92.0 because error-prone/misleading. You can use window->FontRefSize for a copy of g.FontSize at the time of the last Begin() call for this window.
    //float     CalcFontSize() const    { RblxUIContext& g = *Ctx; return g.FontSizeBase * FontWindowScale * FontDpiScale * FontWindowScaleParents;
};

//-----------------------------------------------------------------------------
// [SECTION] Tab bar, Tab item support
//-----------------------------------------------------------------------------

// Extend RblxUITabBarFlags_
enum RblxUITabBarFlagsPrivate_
{
    RblxUITabBarFlags_DockNode                   = 1 << 20,  // Part of a dock node [we don't use this in the master branch but it facilitate branch syncing to keep this around]
    RblxUITabBarFlags_IsFocused                  = 1 << 21,
    RblxUITabBarFlags_SaveSettings               = 1 << 22,  // FIXME: Settings are handled by the docking system, this only request the tab bar to mark settings dirty when reordering tabs
};

// Extend RblxUITabItemFlags_
enum RblxUITabItemFlagsPrivate_
{
    RblxUITabItemFlags_SectionMask_              = RblxUITabItemFlags_Leading | RblxUITabItemFlags_Trailing,
    RblxUITabItemFlags_NoCloseButton             = 1 << 20,  // Track whether p_open was set or not (we'll need this info on the next frame to recompute ContentWidth during layout)
    RblxUITabItemFlags_Button                    = 1 << 21,  // Used by TabItemButton, change the tab item behavior to mimic a button
    RblxUITabItemFlags_Invisible                 = 1 << 22,  // To reserve space e.g. with RblxUITabItemFlags_Leading
    RblxUITabItemFlags_Unsorted                  = 1 << 23,  // [Docking] Trailing tabs with the _Unsorted flag will be sorted based on the DockOrder of their Window.
};

// Storage for one active tab item (sizeof() 48 bytes)
struct RblxUITabItem
{
    RblxUIID             ID;
    RblxUITabItemFlags   Flags;
    RblxUIWindow*        Window;                 // When TabItem is part of a DockNode's TabBar, we hold on to a window.
    int                 LastFrameVisible;
    int                 LastFrameSelected;      // This allows us to infer an ordered list of the last activated tabs with little maintenance
    float               Offset;                 // Position relative to beginning of tab bar
    float               Width;                  // Width currently displayed
    float               ContentWidth;           // Width of label + padding, stored during BeginTabItem() call (misnamed as "Content" would normally imply width of label only)
    float               RequestedWidth;         // Width optionally requested by caller, -1.0f is unused
    RblxS32               NameOffset;             // When Window==NULL, offset to name within parent RblxUITabBar::TabsNames
    RblxS16               BeginOrder;             // BeginTabItem() order, used to re-order tabs after toggling RblxUITabBarFlags_Reorderable
    RblxS16               IndexDuringLayout;      // Index only used during TabBarLayout(). Tabs gets reordered so 'Tabs[n].IndexDuringLayout == n' but may mismatch during additions.
    bool                WantClose;              // Marked as closed by SetTabItemClosed()

    RblxUITabItem()      { memset((void*)this, 0, sizeof(*this)); LastFrameVisible = LastFrameSelected = -1; RequestedWidth = -1.0f; NameOffset = -1; BeginOrder = IndexDuringLayout = -1; }
};

// Storage for a tab bar (sizeof() 160 bytes)
struct RBLXUI_API RblxUITabBar
{
    RblxUIWindow*        Window;
    RblxVector<RblxUITabItem> Tabs;
    RblxUITabBarFlags    Flags;
    RblxUIID             ID;                     // Zero for tab-bars used by docking
    RblxUIID             SelectedTabId;          // Selected tab/window
    RblxUIID             NextSelectedTabId;      // Next selected tab/window. Will also trigger a scrolling animation
    RblxUIID             NextScrollToTabId;
    RblxUIID             VisibleTabId;           // Can occasionally be != SelectedTabId (e.g. when previewing contents for Ctrl+Tab preview)
    int                 CurrFrameVisible;
    int                 PrevFrameVisible;
    RblxRect              BarRect;
    float               BarRectPrevWidth;       // Backup of previous width. When width change we enforce keep horizontal scroll on focused tab.
    float               CurrTabsContentsHeight;
    float               PrevTabsContentsHeight; // Record the height of contents submitted below the tab bar
    float               WidthAllTabs;           // Actual width of all tabs (locked during layout)
    float               WidthAllTabsIdeal;      // Ideal width if all tabs were visible and not clipped
    float               ScrollingAnim;
    float               ScrollingTarget;
    float               ScrollingTargetDistToVisibility;
    float               ScrollingSpeed;
    float               ScrollingRectMinX;
    float               ScrollingRectMaxX;
    float               SeparatorMinX;
    float               SeparatorMaxX;
    RblxUIID             ReorderRequestTabId;
    RblxS16               ReorderRequestOffset;
    RblxS8                BeginCount;
    bool                WantLayout;
    bool                VisibleTabWasSubmitted;
    bool                TabsAddedNew;           // Set to true when a new tab item or button has been added to the tab bar during last frame
    bool                ScrollButtonEnabled;
    RblxS16               TabsActiveCount;        // Number of tabs submitted this frame.
    RblxS16               LastTabItemIdx;         // Index of last BeginTabItem() tab for use by EndTabItem()
    float               ItemSpacingY;
    RblxVec2              FramePadding;           // style.FramePadding locked at the time of BeginTabBar()
    RblxVec2              BackupCursorPos;
    RblxUITextBuffer     TabsNames;              // For non-docking tab bar we re-append names in a contiguous buffer.

    RblxUITabBar();
};

//-----------------------------------------------------------------------------
// [SECTION] Table support
//-----------------------------------------------------------------------------

#define RBLXUI_COL32_DISABLE                RBLXUI_COL32(0,0,0,1)   // Special sentinel code which cannot be used as a regular color.
#define RBLXUI_TABLE_MAX_COLUMNS         512                 // Arbitrary "safety" maximum, may be lifted in the future if needed. Must fit in RblxUITableColumnIdx/RblxUITableDrawChannelIdx.

// [Internal] sizeof() ~ 112
// We use the terminology "Enabled" to refer to a column that is not Hidden by user/api.
// We use the terminology "Clipped" to refer to a column that is out of sight because of scrolling/clipping.
// This is in contrast with some user-facing api such as IsItemVisible() / IsRectVisible() which use "Visible" to mean "not clipped".
struct RblxUITableColumn
{
    RblxUITableColumnFlags   Flags;                          // Flags after some patching (not directly same as provided by user). See RblxUITableColumnFlags_
    float                   WidthGiven;                     // Final/actual width visible == (MaxX - MinX), locked in TableUpdateLayout(). May be > WidthRequest to honor minimum width, may be < WidthRequest to honor shrinking columns down in tight space.
    float                   MinX;                           // Absolute positions
    float                   MaxX;
    float                   WidthRequest;                   // Master width absolute value when !(Flags & _WidthStretch). When Stretch this is derived every frame from StretchWeight in TableUpdateLayout()
    float                   WidthAuto;                      // Automatic width
    float                   WidthMax;                       // Maximum width (FIXME: overwritten by each instance)
    float                   StretchWeight;                  // Master width weight when (Flags & _WidthStretch). Often around ~1.0f initially.
    float                   InitStretchWeightOrWidth;       // Value passed to TableSetupColumn(). For Width it is a content width (_without padding_).
    RblxRect                  ClipRect;                       // Clipping rectangle for the column
    RblxUIID                 UserID;                         // Optional, value passed to TableSetupColumn()
    float                   WorkMinX;                       // Contents region min ~(MinX + CellPaddingX + CellSpacingX1) == cursor start position when entering column
    float                   WorkMaxX;                       // Contents region max ~(MaxX - CellPaddingX - CellSpacingX2)
    float                   ItemWidth;                      // Current item width for the column, preserved across rows
    float                   ContentMaxXFrozen;              // Contents maximum position for frozen rows (apart from headers), from which we can infer content width.
    float                   ContentMaxXUnfrozen;
    float                   ContentMaxXHeadersUsed;         // Contents maximum position for headers rows (regardless of freezing). TableHeader() automatically softclip itself + report ideal desired size, to avoid creating extraneous draw calls
    float                   ContentMaxXHeadersIdeal;
    RblxS16                   NameOffset;                     // Offset into parent ColumnsNames[]
    RblxUITableColumnIdx     DisplayOrder;                   // Index within Table's IndexToDisplayOrder[] (column may be reordered by users)
    RblxUITableColumnIdx     IndexWithinEnabledSet;          // Index within enabled/visible set (<= IndexToDisplayOrder)
    RblxUITableColumnIdx     PrevEnabledColumn;              // Index of prev enabled/visible column within Columns[], -1 if first enabled/visible column
    RblxUITableColumnIdx     NextEnabledColumn;              // Index of next enabled/visible column within Columns[], -1 if last enabled/visible column
    RblxUITableColumnIdx     SortOrder;                      // Index of this column within sort specs, -1 if not sorting on this column, 0 for single-sort, may be >0 on multi-sort
    RblxUITableDrawChannelIdx DrawChannelCurrent;            // Index within DrawSplitter.Channels[]
    RblxUITableDrawChannelIdx DrawChannelFrozen;             // Draw channels for frozen rows (often headers)
    RblxUITableDrawChannelIdx DrawChannelUnfrozen;           // Draw channels for unfrozen rows
    bool                    IsEnabled;                      // IsUserEnabled && (Flags & RblxUITableColumnFlags_Disabled) == 0
    bool                    IsUserEnabled;                  // Is the column not marked Hidden by the user? (unrelated to being off view, e.g. clipped by scrolling).
    bool                    IsUserEnabledNextFrame;
    bool                    IsVisibleX;                     // Is actually in view (e.g. overlapping the host window clipping rectangle, not scrolled).
    bool                    IsVisibleY;
    bool                    IsRequestOutput;                // Return value for TableSetColumnIndex() / TableNextColumn(): whether we request user to output contents or not.
    bool                    IsSkipItems;                    // Do we want item submissions to this column to be completely ignored (no layout will happen).
    bool                    IsPreserveWidthAuto;
    RblxS8                    NavLayerCurrent;                // RblxUINavLayer in 1 byte
    RblxU8                    AutoFitQueue;                   // Queue of 8 values for the next 8 frames to request auto-fit
    RblxU8                    CannotSkipItemsQueue;           // Queue of 8 values for the next 8 frames to disable Clipped/SkipItem
    RblxU8                    SortDirection : 2;              // RblxUISortDirection_Ascending or RblxUISortDirection_Descending
    RblxU8                    SortDirectionsAvailCount : 2;   // Number of available sort directions (0 to 3)
    RblxU8                    SortDirectionsAvailMask : 4;    // Mask of available sort directions (1-bit each)
    RblxU8                    SortDirectionsAvailList;        // Ordered list of available sort directions (2-bits each, total 8-bits)

    RblxUITableColumn()
    {
        memset((void*)this, 0, sizeof(*this));
        StretchWeight = WidthRequest = -1.0f;
        NameOffset = -1;
        DisplayOrder = IndexWithinEnabledSet = -1;
        PrevEnabledColumn = NextEnabledColumn = -1;
        SortOrder = -1;
        SortDirection = RblxUISortDirection_None;
        DrawChannelCurrent = DrawChannelFrozen = DrawChannelUnfrozen = (RblxU8)-1;
    }
};

// Transient cell data stored per row.
// sizeof() ~ 6 bytes
struct RblxUITableCellData
{
    RblxU32                       BgColor;    // Actual color
    RblxUITableColumnIdx         Column;     // Column number
};

// Parameters for TableAngledHeadersRowEx()
// This may end up being refactored for more general purpose.
// sizeof() ~ 12 bytes
struct RblxUITableHeaderData
{
    RblxUITableColumnIdx         Index;      // Column index
    RblxU32                       TextColor;
    RblxU32                       BgColor0;
    RblxU32                       BgColor1;
};

// Per-instance data that needs preserving across frames (seemingly most others do not need to be preserved aside from debug needs. Does that means they could be moved to RblxUITableTempData?)
// sizeof() ~ 24 bytes
struct RblxUITableInstanceData
{
    RblxUIID                     TableInstanceID;
    float                       LastOuterHeight;            // Outer height from last frame
    float                       LastTopHeadersRowHeight;    // Height of first consecutive header rows from last frame (FIXME: this is used assuming consecutive headers are in same frozen set)
    float                       LastFrozenHeight;           // Height of frozen section from last frame
    int                         HoveredRowLast;             // Index of row which was hovered last frame.
    int                         HoveredRowNext;             // Index of row hovered this frame, set after encountering it.

    RblxUITableInstanceData()    { TableInstanceID = 0; LastOuterHeight = LastTopHeadersRowHeight = LastFrozenHeight = 0.0f; HoveredRowLast = HoveredRowNext = -1; }
};

// sizeof() ~ 592 bytes + heap allocs described in TableBeginInitMemory()
struct RBLXUI_API RblxUITable
{
    RblxUIID                     ID;
    RblxUITableFlags             Flags;
    void*                       RawData;                    // Single allocation to hold Columns[], DisplayOrderToIndex[], and RowCellData[]
    RblxUITableTempData*         TempData;                   // Transient data while table is active. Point within g.CurrentTableStack[]
    RblxSpan<RblxUITableColumn>    Columns;                    // Point within RawData[]
    RblxSpan<RblxUITableColumnIdx> DisplayOrderToIndex;        // Point within RawData[]. Store display order of columns (when not reordered, the values are 0...Count-1)
    RblxSpan<RblxUITableCellData>  RowCellData;                // Point within RawData[]. Store cells background requests for current row.
    RblxBitArrayPtr               EnabledMaskByDisplayOrder;  // Column DisplayOrder -> IsEnabled map
    RblxBitArrayPtr               EnabledMaskByIndex;         // Column Index -> IsEnabled map (== not hidden by user/api) in a format adequate for iterating column without touching cold data
    RblxBitArrayPtr               VisibleMaskByIndex;         // Column Index -> IsVisibleX|IsVisibleY map (== not hidden by user/api && not hidden by scrolling/cliprect)
    RblxUITableFlags             SettingsLoadedFlags;        // Which data were loaded from the .ini file (e.g. when order is not altered we won't save order)
    int                         SettingsOffset;             // Offset in g.SettingsTables
    int                         LastFrameActive;
    int                         ColumnsCount;               // Number of columns declared in BeginTable()
    int                         CurrentRow;
    int                         CurrentColumn;
    RblxS16                       InstanceCurrent;            // Count of BeginTable() calls with same ID in the same frame (generally 0). This is a little bit similar to BeginCount for a window, but multiple tables with the same ID are multiple tables, they are just synced.
    RblxS16                       InstanceInteracted;         // Mark which instance (generally 0) of the same ID is being interacted with
    float                       RowPosY1;
    float                       RowPosY2;
    float                       RowMinHeight;               // Height submitted to TableNextRow()
    float                       RowCellPaddingY;            // Top and bottom padding. Reloaded during row change.
    float                       RowTextBaseline;
    float                       RowIndentOffsetX;
    RblxUITableRowFlags          RowFlags : 16;              // Current row flags, see RblxUITableRowFlags_
    RblxUITableRowFlags          LastRowFlags : 16;
    int                         RowBgColorCounter;          // Counter for alternating background colors (can be fast-forwarded by e.g clipper), not same as CurrentRow because header rows typically don't increase this.
    RblxU32                       RowBgColor[2];              // Background color override for current row.
    RblxU32                       BorderColorStrong;
    RblxU32                       BorderColorLight;
    float                       BorderX1;
    float                       BorderX2;
    float                       HostIndentX;
    float                       MinColumnWidth;
    float                       OuterPaddingX;
    float                       CellPaddingX;               // Padding from each borders. Locked in BeginTable()/Layout.
    float                       CellSpacingX1;              // Spacing between non-bordered cells. Locked in BeginTable()/Layout.
    float                       CellSpacingX2;
    float                       InnerWidth;                 // User value passed to BeginTable(), see comments at the top of BeginTable() for details.
    float                       ColumnsGivenWidth;          // Sum of current column width
    float                       ColumnsAutoFitWidth;        // Sum of ideal column width in order nothing to be clipped, used for auto-fitting and content width submission in outer window
    float                       ColumnsStretchSumWeights;   // Sum of weight of all enabled stretching columns
    float                       ResizedColumnNextWidth;
    float                       ResizeLockMinContentsX2;    // Lock minimum contents width while resizing down in order to not create feedback loops. But we allow growing the table.
    float                       RefScale;                   // Reference scale to be able to rescale columns on font/dpi changes.
    float                       AngledHeadersHeight;        // Set by TableAngledHeadersRow(), used in TableUpdateLayout()
    float                       AngledHeadersSlope;         // Set by TableAngledHeadersRow(), used in TableUpdateLayout()
    RblxRect                      OuterRect;                  // Note: for non-scrolling table, OuterRect.Max.y is often FLT_MAX until EndTable(), unless a height has been specified in BeginTable().
    RblxRect                      InnerRect;                  // InnerRect but without decoration. As with OuterRect, for non-scrolling tables, InnerRect.Max.y is "
    RblxRect                      WorkRect;
    RblxRect                      InnerClipRect;
    RblxRect                      BgClipRect;                 // We use this to cpu-clip cell background color fill, evolve during the frame as we cross frozen rows boundaries
    RblxRect                      Bg0ClipRectForDrawCmd;      // Actual RblxDrawCmd clip rect for BG0/1 channel. This tends to be == OuterWindow->ClipRect at BeginTable() because output in BG0/BG1 is cpu-clipped
    RblxRect                      Bg2ClipRectForDrawCmd;      // Actual RblxDrawCmd clip rect for BG2 channel. This tends to be a correct, tight-fit, because output to BG2 are done by widgets relying on regular ClipRect.
    RblxRect                      HostClipRect;               // This is used to check if we can eventually merge our columns draw calls into the current draw call of the current window.
    RblxRect                      HostBackupInnerClipRect;    // Backup of InnerWindow->ClipRect during PushTableBackground()/PopTableBackground()
    RblxUIWindow*                OuterWindow;                // Parent window for the table
    RblxUIWindow*                InnerWindow;                // Window holding the table data (== OuterWindow or a child window)
    RblxUITextBuffer             ColumnsNames;               // Contiguous buffer holding columns names
    RblxDrawListSplitter*         DrawSplitter;               // Shortcut to TempData->DrawSplitter while in table. Isolate draw commands per columns to avoid switching clip rect constantly
    RblxUITableInstanceData      InstanceDataFirst;
    RblxVector<RblxUITableInstanceData>    InstanceDataExtra;  // FIXME-OPT: Using a small-vector pattern would be good.
    RblxUITableColumnSortSpecs   SortSpecsSingle;
    RblxVector<RblxUITableColumnSortSpecs> SortSpecsMulti;     // FIXME-OPT: Using a small-vector pattern would be good.
    RblxUITableSortSpecs         SortSpecs;                  // Public facing sorts specs, this is what we return in TableGetSortSpecs()
    RblxUITableColumnIdx         SortSpecsCount;
    RblxUITableColumnIdx         ColumnsEnabledCount;        // Number of enabled columns (<= ColumnsCount)
    RblxUITableColumnIdx         ColumnsEnabledFixedCount;   // Number of enabled columns using fixed width (<= ColumnsCount)
    RblxUITableColumnIdx         DeclColumnsCount;           // Count calls to TableSetupColumn()
    RblxUITableColumnIdx         AngledHeadersCount;         // Count columns with angled headers
    RblxUITableColumnIdx         HoveredColumnBody;          // Index of column whose visible region is being hovered. Important: == ColumnsCount when hovering empty region after the right-most column!
    RblxUITableColumnIdx         HoveredColumnBorder;        // Index of column whose right-border is being hovered (for resizing).
    RblxUITableColumnIdx         HighlightColumnHeader;      // Index of column which should be highlighted.
    RblxUITableColumnIdx         AutoFitSingleColumn;        // Index of single column requesting auto-fit.
    RblxUITableColumnIdx         ResizedColumn;              // Index of column being resized. Reset when InstanceCurrent==0.
    RblxUITableColumnIdx         LastResizedColumn;          // Index of column being resized from previous frame.
    RblxUITableColumnIdx         HeldHeaderColumn;           // Index of column header being held.
    RblxUITableColumnIdx         LastHeldHeaderColumn;       // Index of column header being held from previous frame.
    RblxUITableColumnIdx         ReorderColumn;              // Index of column being reordered. (not cleared)
    RblxUITableColumnIdx         ReorderColumnDstOrder;      // Requested display order of column being reordered.
    RblxUITableColumnIdx         LeftMostEnabledColumn;      // Index of left-most non-hidden column.
    RblxUITableColumnIdx         RightMostEnabledColumn;     // Index of right-most non-hidden column.
    RblxUITableColumnIdx         LeftMostStretchedColumn;    // Index of left-most stretched column.
    RblxUITableColumnIdx         RightMostStretchedColumn;   // Index of right-most stretched column.
    RblxUITableColumnIdx         ContextPopupColumn;         // Column right-clicked on, of -1 if opening context menu from a neutral/empty spot
    RblxUITableColumnIdx         FreezeRowsRequest;          // Requested frozen rows count
    RblxUITableColumnIdx         FreezeRowsCount;            // Actual frozen row count (== FreezeRowsRequest, or == 0 when no scrolling offset)
    RblxUITableColumnIdx         FreezeColumnsRequest;       // Requested frozen columns count
    RblxUITableColumnIdx         FreezeColumnsCount;         // Actual frozen columns count (== FreezeColumnsRequest, or == 0 when no scrolling offset)
    RblxUITableColumnIdx         RowCellDataCurrent;         // Index of current RowCellData[] entry in current row
    RblxUITableDrawChannelIdx    DummyDrawChannel;           // Redirect non-visible columns here.
    RblxUITableDrawChannelIdx    Bg2DrawChannelCurrent;      // For Selectable() and other widgets drawing across columns after the freezing line. Index within DrawSplitter.Channels[]
    RblxUITableDrawChannelIdx    Bg2DrawChannelUnfrozen;
    RblxS8                        NavLayer;                   // RblxUINavLayer at the time of BeginTable().
    bool                        IsLayoutLocked;             // Set by TableUpdateLayout() which is called when beginning the first row.
    bool                        IsInsideRow;                // Set when inside TableBeginRow()/TableEndRow().
    bool                        IsInitializing;
    bool                        IsSortSpecsDirty;
    bool                        IsUsingHeaders;             // Set when the first row had the RblxUITableRowFlags_Headers flag.
    bool                        IsContextPopupOpen;         // Set when default context menu is open (also see: ContextPopupColumn, InstanceInteracted).
    bool                        DisableDefaultContextMenu;  // Disable default context menu. You may submit your own using TableBeginContextMenuPopup()/EndPopup()
    bool                        IsSettingsRequestLoad;
    bool                        IsSettingsDirty;            // Set when table settings have changed and needs to be reported into RblxUITableSettings data.
    bool                        IsDefaultDisplayOrder;      // Set when display order is unchanged from default (DisplayOrder contains 0...Count-1)
    bool                        IsResetAllRequest;
    bool                        IsResetDisplayOrderRequest;
    bool                        IsUnfrozenRows;             // Set when we got past the frozen row.
    bool                        IsDefaultSizingPolicy;      // Set if user didn't explicitly set a sizing policy in BeginTable()
    bool                        IsActiveIdAliveBeforeTable;
    bool                        IsActiveIdInTable;
    bool                        HasScrollbarYCurr;          // Whether ANY instance of this table had a vertical scrollbar during the current frame.
    bool                        HasScrollbarYPrev;          // Whether ANY instance of this table had a vertical scrollbar during the previous.
    bool                        MemoryCompacted;
    bool                        HostSkipItems;              // Backup of InnerWindow->SkipItem at the end of BeginTable(), because we will overwrite InnerWindow->SkipItem on a per-column basis

    RblxUITable()                { memset((void*)this, 0, sizeof(*this)); LastFrameActive = -1; }
    ~RblxUITable()               { RBLXUI_FREE(RawData); }
};

// Transient data that are only needed between BeginTable() and EndTable(), those buffers are shared (1 per level of stacked table).
// - Accessing those requires chasing an extra pointer so for very frequently used data we leave them in the main table structure.
// - We also leave out of this structure data that tend to be particularly useful for debugging/metrics.
// FIXME-TABLE: more transient data could be stored in a stacked RblxUITableTempData: e.g. SortSpecs.
// sizeof() ~ 136 bytes.
struct RBLXUI_API RblxUITableTempData
{
    RblxUIID                     WindowID;                   // Shortcut to g.Tables[TableIndex]->OuterWindow->ID.
    int                         TableIndex;                 // Index in g.Tables.Buf[] pool
    float                       LastTimeActive;             // Last timestamp this structure was used
    float                       AngledHeadersExtraWidth;    // Used in EndTable()
    RblxVector<RblxUITableHeaderData> AngledHeadersRequests;   // Used in TableAngledHeadersRow()

    RblxVec2                      UserOuterSize;              // outer_size.x passed to BeginTable()
    RblxDrawListSplitter          DrawSplitter;

    RblxRect                      HostBackupWorkRect;         // Backup of InnerWindow->WorkRect at the end of BeginTable()
    RblxRect                      HostBackupParentWorkRect;   // Backup of InnerWindow->ParentWorkRect at the end of BeginTable()
    RblxVec2                      HostBackupPrevLineSize;     // Backup of InnerWindow->DC.PrevLineSize at the end of BeginTable()
    RblxVec2                      HostBackupCurrLineSize;     // Backup of InnerWindow->DC.CurrLineSize at the end of BeginTable()
    RblxVec2                      HostBackupCursorMaxPos;     // Backup of InnerWindow->DC.CursorMaxPos at the end of BeginTable()
    RblxVec1                      HostBackupColumnsOffset;    // Backup of OuterWindow->DC.ColumnsOffset at the end of BeginTable()
    float                       HostBackupItemWidth;        // Backup of OuterWindow->DC.ItemWidth at the end of BeginTable()
    int                         HostBackupItemWidthStackSize;//Backup of OuterWindow->DC.ItemWidthStack.Size at the end of BeginTable()

    RblxUITableTempData()        { memset((void*)this, 0, sizeof(*this)); LastTimeActive = -1.0f; }
};

// sizeof() ~ 16
struct RblxUITableColumnSettings
{
    float                   WidthOrWeight;
    RblxUIID                 UserID;
    RblxUITableColumnIdx     Index;
    RblxUITableColumnIdx     DisplayOrder;
    RblxUITableColumnIdx     SortOrder;
    RblxU8                    SortDirection : 2;
    RblxS8                    IsEnabled : 2; // "Visible" in ini file
    RblxU8                    IsStretch : 1;

    RblxUITableColumnSettings()
    {
        WidthOrWeight = 0.0f;
        UserID = 0;
        Index = -1;
        DisplayOrder = SortOrder = -1;
        SortDirection = RblxUISortDirection_None;
        IsEnabled = -1;
        IsStretch = 0;
    }
};

// This is designed to be stored in a single RblxChunkStream (1 header followed by N RblxUITableColumnSettings, etc.)
struct RblxUITableSettings
{
    RblxUIID                     ID;                     // Set to 0 to invalidate/delete the setting
    RblxUITableFlags             SaveFlags;              // Indicate data we want to save using the Resizable/Reorderable/Sortable/Hideable flags (could be using its own flags..)
    float                       RefScale;               // Reference scale to be able to rescale columns on font/dpi changes.
    RblxUITableColumnIdx         ColumnsCount;
    RblxUITableColumnIdx         ColumnsCountMax;        // Maximum number of columns this settings instance can store, we can recycle a settings instance with lower number of columns but not higher
    bool                        WantApply;              // Set when loaded from .ini data (to enable merging/loading .ini data into an already running context)

    RblxUITableSettings()        { memset((void*)this, 0, sizeof(*this)); }
    RblxUITableColumnSettings*   GetColumnSettings()     { return (RblxUITableColumnSettings*)(this + 1); }
};

//-----------------------------------------------------------------------------
// [SECTION] RblxUI internal API
// No guarantee of forward compatibility here!
//-----------------------------------------------------------------------------

namespace RblxUI
{
    // Windows
    // We should always have a CurrentWindow in the stack (there is an implicit "Debug" window)
    // If this ever crashes because g.CurrentWindow is NULL, it means that either:
    // - RblxUI::NewFrame() has never been called, which is illegal.
    // - You are calling RblxUI functions after RblxUI::EndFrame()/RblxUI::Render() and before the next RblxUI::NewFrame(), which is also illegal.
    RBLXUI_API RblxUIIO&         GetIO(RblxUIContext* ctx);
    RBLXUI_API RblxUIPlatformIO& GetPlatformIO(RblxUIContext* ctx);
    inline    float         GetScale()                  { RblxUIContext& g = *GRblxUI; return g.Style._MainScale; } // FIXME-DPI: I don't want to formalize this just yet. Because reasons. Please don't use.
    inline    RblxUIWindow*  GetCurrentWindowRead()      { RblxUIContext& g = *GRblxUI; return g.CurrentWindow; }
    inline    RblxUIWindow*  GetCurrentWindow()          { RblxUIContext& g = *GRblxUI; g.CurrentWindow->WriteAccessed = true; return g.CurrentWindow; }
    RBLXUI_API RblxUIWindow*  FindWindowByID(RblxUIID id);
    RBLXUI_API RblxUIWindow*  FindWindowByName(const char* name);
    RBLXUI_API void          UpdateWindowParentAndRootLinks(RblxUIWindow* window, RblxUIWindowFlags flags, RblxUIWindow* parent_window);
    RBLXUI_API void          UpdateWindowSkipRefresh(RblxUIWindow* window);
    RBLXUI_API RblxVec2        CalcWindowNextAutoFitSize(RblxUIWindow* window);
    RBLXUI_API bool          IsWindowChildOf(RblxUIWindow* window, RblxUIWindow* potential_parent, bool popup_hierarchy, bool dock_hierarchy);
    RBLXUI_API bool          IsWindowInBeginStack(RblxUIWindow* window);
    RBLXUI_API bool          IsWindowWithinBeginStackOf(RblxUIWindow* window, RblxUIWindow* potential_parent);
    RBLXUI_API bool          IsWindowAbove(RblxUIWindow* potential_above, RblxUIWindow* potential_below);
    RBLXUI_API bool          IsWindowNavFocusable(RblxUIWindow* window);
    RBLXUI_API void          SetWindowPos(RblxUIWindow* window, const RblxVec2& pos, RblxUICond cond = 0);
    RBLXUI_API void          SetWindowSize(RblxUIWindow* window, const RblxVec2& size, RblxUICond cond = 0);
    RBLXUI_API void          SetWindowCollapsed(RblxUIWindow* window, bool collapsed, RblxUICond cond = 0);
    RBLXUI_API void          SetWindowHitTestHole(RblxUIWindow* window, const RblxVec2& pos, const RblxVec2& size);
    RBLXUI_API void          SetWindowHiddenAndSkipItemsForCurrentFrame(RblxUIWindow* window);
    inline void             SetWindowParentWindowForFocusRoute(RblxUIWindow* window, RblxUIWindow* parent_window) { window->ParentWindowForFocusRoute = parent_window; } // You may also use SetNextWindowClass()'s FocusRouteParentWindowId field.
    inline RblxRect           WindowRectAbsToRel(RblxUIWindow* window, const RblxRect& r) { RblxVec2 off = window->DC.CursorStartPos; return RblxRect(r.Min.x - off.x, r.Min.y - off.y, r.Max.x - off.x, r.Max.y - off.y); }
    inline RblxRect           WindowRectRelToAbs(RblxUIWindow* window, const RblxRect& r) { RblxVec2 off = window->DC.CursorStartPos; return RblxRect(r.Min.x + off.x, r.Min.y + off.y, r.Max.x + off.x, r.Max.y + off.y); }
    inline RblxVec2           WindowPosAbsToRel(RblxUIWindow* window, const RblxVec2& p)  { RblxVec2 off = window->DC.CursorStartPos; return RblxVec2(p.x - off.x, p.y - off.y); }
    inline RblxVec2           WindowPosRelToAbs(RblxUIWindow* window, const RblxVec2& p)  { RblxVec2 off = window->DC.CursorStartPos; return RblxVec2(p.x + off.x, p.y + off.y); }

    // Windows: Display Order and Focus Order
    RBLXUI_API void          FocusWindow(RblxUIWindow* window, RblxUIFocusRequestFlags flags = 0);
    RBLXUI_API void          FocusTopMostWindowUnderOne(RblxUIWindow* under_this_window, RblxUIWindow* ignore_window, RblxUIViewport* filter_viewport, RblxUIFocusRequestFlags flags);
    RBLXUI_API void          BringWindowToFocusFront(RblxUIWindow* window);
    RBLXUI_API void          BringWindowToDisplayFront(RblxUIWindow* window);
    RBLXUI_API void          BringWindowToDisplayBack(RblxUIWindow* window);
    RBLXUI_API void          BringWindowToDisplayBehind(RblxUIWindow* window, RblxUIWindow* above_window);
    RBLXUI_API int           FindWindowDisplayIndex(RblxUIWindow* window);
    RBLXUI_API RblxUIWindow*  FindBottomMostVisibleWindowWithinBeginStack(RblxUIWindow* window);

    // Windows: Idle, Refresh Policies [EXPERIMENTAL]
    RBLXUI_API void          SetNextWindowRefreshPolicy(RblxUIWindowRefreshFlags flags);

    // Fonts, drawing
    RBLXUI_API void          RegisterUserTexture(RblxTextureData* tex); // Register external texture. EXPERIMENTAL.
    RBLXUI_API void          UnregisterUserTexture(RblxTextureData* tex);
    RBLXUI_API void          RegisterFontAtlas(RblxFontAtlas* atlas);
    RBLXUI_API void          UnregisterFontAtlas(RblxFontAtlas* atlas);
    RBLXUI_API void          SetCurrentFont(RblxFont* font, float font_size_before_scaling, float font_size_after_scaling);
    RBLXUI_API void          UpdateCurrentFontSize(float restore_font_size_after_scaling);
    RBLXUI_API void          SetFontRasterizerDensity(float rasterizer_density);
    inline float            GetFontRasterizerDensity() { return GRblxUI->FontRasterizerDensity; }
    inline float            GetRoundedFontSize(float size) { return RBLXUI_ROUND(size); }
    RBLXUI_API RblxFont*       GetDefaultFont();
    RBLXUI_API void          PushPasswordFont();
    RBLXUI_API void          PopPasswordFont();
    inline RblxDrawList*      GetForegroundDrawList(RblxUIWindow* window) { return GetForegroundDrawList(window->Viewport); }
    RBLXUI_API void          AddDrawListToDrawDataEx(RblxDrawData* draw_data, RblxVector<RblxDrawList*>* out_list, RblxDrawList* draw_list);

    // Init
    RBLXUI_API void          Initialize();
    RBLXUI_API void          Shutdown();    // Since 1.60 this is a _private_ function. You can call DestroyContext() to destroy the context created by CreateContext().

    // Context name & generic context hooks
    RBLXUI_API void          SetContextName(RblxUIContext* ctx, const char* name);
    RBLXUI_API RblxUIID       AddContextHook(RblxUIContext* ctx, const RblxUIContextHook* hook);
    RBLXUI_API void          RemoveContextHook(RblxUIContext* ctx, RblxUIID hook_to_remove);
    RBLXUI_API void          CallContextHooks(RblxUIContext* ctx, RblxUIContextHookType type);

    // NewFrame
    RBLXUI_API void          UpdateInputEvents(bool trickle_fast_inputs);
    RBLXUI_API void          UpdateHoveredWindowAndCaptureFlags(const RblxVec2& mouse_pos);
    RBLXUI_API void          FindHoveredWindowEx(const RblxVec2& pos, bool find_first_and_in_any_viewport, RblxUIWindow** out_hovered_window, RblxUIWindow** out_hovered_window_under_moving_window);
    RBLXUI_API void          StartMouseMovingWindow(RblxUIWindow* window);
    RBLXUI_API void          StartMouseMovingWindowOrNode(RblxUIWindow* window, RblxUIDockNode* node, bool undock);
    RBLXUI_API void          StopMouseMovingWindow();
    RBLXUI_API void          UpdateMouseMovingWindowNewFrame();
    RBLXUI_API void          UpdateMouseMovingWindowEndFrame();

    // Viewports
    RBLXUI_API void          TranslateWindowsInViewport(RblxUIViewportP* viewport, const RblxVec2& old_pos, const RblxVec2& new_pos, const RblxVec2& old_size, const RblxVec2& new_size);
    RBLXUI_API void          ScaleWindowsInViewport(RblxUIViewportP* viewport, float scale);
    RBLXUI_API void          DestroyPlatformWindow(RblxUIViewportP* viewport);
    RBLXUI_API void          SetWindowViewport(RblxUIWindow* window, RblxUIViewportP* viewport);
    RBLXUI_API void          SetCurrentViewport(RblxUIWindow* window, RblxUIViewportP* viewport);
    RBLXUI_API const RblxUIPlatformMonitor*   GetViewportPlatformMonitor(RblxUIViewport* viewport);
    RBLXUI_API RblxUIViewportP*               FindHoveredViewportFromPlatformWindowStack(const RblxVec2& mouse_platform_pos);

    // Settings
    RBLXUI_API void                  MarkIniSettingsDirty();
    RBLXUI_API void                  MarkIniSettingsDirty(RblxUIWindow* window);
    RBLXUI_API void                  ClearIniSettings();
    RBLXUI_API void                  AddSettingsHandler(const RblxUISettingsHandler* handler);
    RBLXUI_API void                  RemoveSettingsHandler(const char* type_name);
    RBLXUI_API RblxUISettingsHandler* FindSettingsHandler(const char* type_name);

    // Settings - Windows
    RBLXUI_API RblxUIWindowSettings*  CreateNewWindowSettings(const char* name);
    RBLXUI_API RblxUIWindowSettings*  FindWindowSettingsByID(RblxUIID id);
    RBLXUI_API RblxUIWindowSettings*  FindWindowSettingsByWindow(RblxUIWindow* window);
    RBLXUI_API void                  ClearWindowSettings(const char* name);

    // Localization
    RBLXUI_API void          LocalizeRegisterEntries(const RblxUILocEntry* entries, int count);
    inline const char*      LocalizeGetMsg(RblxUILocKey key) { RblxUIContext& g = *GRblxUI; const char* msg = g.LocalizationTable[key]; return msg ? msg : "*Missing Text*"; }

    // Scrolling
    RBLXUI_API void          SetScrollX(RblxUIWindow* window, float scroll_x);
    RBLXUI_API void          SetScrollY(RblxUIWindow* window, float scroll_y);
    RBLXUI_API void          SetScrollFromPosX(RblxUIWindow* window, float local_x, float center_x_ratio);
    RBLXUI_API void          SetScrollFromPosY(RblxUIWindow* window, float local_y, float center_y_ratio);

    // Early work-in-progress API (ScrollToItem() will become public)
    RBLXUI_API void          ScrollToItem(RblxUIScrollFlags flags = 0);
    RBLXUI_API void          ScrollToRect(RblxUIWindow* window, const RblxRect& rect, RblxUIScrollFlags flags = 0);
    RBLXUI_API RblxVec2        ScrollToRectEx(RblxUIWindow* window, const RblxRect& rect, RblxUIScrollFlags flags = 0);
//#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    inline void             ScrollToBringRectIntoView(RblxUIWindow* window, const RblxRect& rect) { ScrollToRect(window, rect, RblxUIScrollFlags_KeepVisibleEdgeY); }
//#endif

    // Basic Accessors
    inline RblxUIItemStatusFlags GetItemStatusFlags() { RblxUIContext& g = *GRblxUI; return g.LastItemData.StatusFlags; }
    inline RblxUIID          GetActiveID()   { RblxUIContext& g = *GRblxUI; return g.ActiveId; }
    inline RblxUIID          GetFocusID()    { RblxUIContext& g = *GRblxUI; return g.NavId; }
    RBLXUI_API void          SetActiveID(RblxUIID id, RblxUIWindow* window);
    RBLXUI_API void          SetFocusID(RblxUIID id, RblxUIWindow* window);
    RBLXUI_API void          ClearActiveID();
    RBLXUI_API RblxUIID       GetHoveredID();
    RBLXUI_API void          SetHoveredID(RblxUIID id);
    RBLXUI_API void          KeepAliveID(RblxUIID id);
    RBLXUI_API void          MarkItemEdited(RblxUIID id);     // Mark data associated to given item as "edited", used by IsItemDeactivatedAfterEdit() function.
    RBLXUI_API void          PushOverrideID(RblxUIID id);     // Push given value as-is at the top of the ID stack (whereas PushID combines old and new hashes)
    RBLXUI_API RblxUIID       GetIDWithSeed(const char* str_id_begin, const char* str_id_end, RblxUIID seed);
    RBLXUI_API RblxUIID       GetIDWithSeed(int n, RblxUIID seed);

    // Basic Helpers for widget code
    RBLXUI_API void          ItemSize(const RblxVec2& size, float text_baseline_y = -1.0f);
    inline void             ItemSize(const RblxRect& bb, float text_baseline_y = -1.0f) { ItemSize(bb.GetSize(), text_baseline_y); } // FIXME: This is a misleading API since we expect CursorPos to be bb.Min.
    RBLXUI_API bool          ItemAdd(const RblxRect& bb, RblxUIID id, const RblxRect* nav_bb = NULL, RblxUIItemFlags extra_flags = 0);
    RBLXUI_API bool          ItemHoverable(const RblxRect& bb, RblxUIID id, RblxUIItemFlags item_flags);
    RBLXUI_API bool          IsWindowContentHoverable(RblxUIWindow* window, RblxUIHoveredFlags flags = 0);
    RBLXUI_API bool          IsClippedEx(const RblxRect& bb, RblxUIID id);
    RBLXUI_API void          SetLastItemData(RblxUIID item_id, RblxUIItemFlags item_flags, RblxUIItemStatusFlags status_flags, const RblxRect& item_rect);
    RBLXUI_API RblxVec2        CalcItemSize(RblxVec2 size, float default_w, float default_h);
    RBLXUI_API float         CalcWrapWidthForPos(const RblxVec2& pos, float wrap_pos_x);
    RBLXUI_API void          PushMultiItemsWidths(int components, float width_full);
    RBLXUI_API void          ShrinkWidths(RblxUIShrinkWidthItem* items, int count, float width_excess, float width_min);
    RBLXUI_API void          CalcClipRectVisibleItemsY(const RblxRect& clip_rect, const RblxVec2& pos, float items_height, int* out_visible_start, int* out_visible_end);

    // Parameter stacks (shared)
    RBLXUI_API const RblxUIStyleVarInfo* GetStyleVarInfo(RblxUIStyleVar idx);
    RBLXUI_API void          BeginDisabledOverrideReenable();
    RBLXUI_API void          EndDisabledOverrideReenable();

    // Logging/Capture
    RBLXUI_API void          LogBegin(RblxUILogFlags flags, int auto_open_depth);         // -> BeginCapture() when we design v2 api, for now stay under the radar by using the old name.
    RBLXUI_API void          LogToBuffer(int auto_open_depth = -1);                      // Start logging/capturing to internal buffer
    RBLXUI_API void          LogRenderedText(const RblxVec2* ref_pos, const char* text, const char* text_end = NULL);
    RBLXUI_API void          LogSetNextTextDecoration(const char* prefix, const char* suffix);

    // Childs
    RBLXUI_API bool          BeginChildEx(const char* name, RblxUIID id, const RblxVec2& size_arg, RblxUIChildFlags child_flags, RblxUIWindowFlags window_flags);
    RBLXUI_API RblxUIWindow*  FindFrontMostVisibleChildWindow(RblxUIWindow* window);

    // Popups, Modals
    RBLXUI_API bool          BeginPopupEx(RblxUIID id, RblxUIWindowFlags extra_window_flags);
    RBLXUI_API bool          BeginPopupMenuEx(RblxUIID id, const char* label, RblxUIWindowFlags extra_window_flags);
    RBLXUI_API void          OpenPopupEx(RblxUIID id, RblxUIPopupFlags popup_flags = RblxUIPopupFlags_None);
    RBLXUI_API void          ClosePopupToLevel(int remaining, bool restore_focus_to_window_under_popup);
    RBLXUI_API void          ClosePopupsOverWindow(RblxUIWindow* ref_window, bool restore_focus_to_window_under_popup);
    RBLXUI_API void          ClosePopupsExceptModals();
    RBLXUI_API bool          IsPopupOpen(RblxUIID id, RblxUIPopupFlags popup_flags);
    RBLXUI_API RblxRect        GetPopupAllowedExtentRect(RblxUIWindow* window);
    RBLXUI_API RblxUIWindow*  GetTopMostPopupModal();
    RBLXUI_API RblxUIWindow*  GetTopMostAndVisiblePopupModal();
    RBLXUI_API RblxUIWindow*  FindBlockingModal(RblxUIWindow* window);
    RBLXUI_API RblxVec2        FindBestWindowPosForPopup(RblxUIWindow* window);
    RBLXUI_API RblxVec2        FindBestWindowPosForPopupEx(const RblxVec2& ref_pos, const RblxVec2& size, RblxUIDir* last_dir, const RblxRect& r_outer, const RblxRect& r_avoid, RblxUIPopupPositionPolicy policy);
    RBLXUI_API RblxUIMouseButton GetMouseButtonFromPopupFlags(RblxUIPopupFlags flags);
    RBLXUI_API bool          IsPopupOpenRequestForItem(RblxUIPopupFlags flags, RblxUIID id);
    RBLXUI_API bool          IsPopupOpenRequestForWindow(RblxUIPopupFlags flags);

    // Tooltips
    RBLXUI_API bool          BeginTooltipEx(RblxUITooltipFlags tooltip_flags, RblxUIWindowFlags extra_window_flags);
    RBLXUI_API bool          BeginTooltipHidden();

    // Menus
    RBLXUI_API bool          BeginViewportSideBar(const char* name, RblxUIViewport* viewport, RblxUIDir dir, float size, RblxUIWindowFlags window_flags);
    RBLXUI_API bool          BeginMenuEx(const char* label, const char* icon, bool enabled = true);
    RBLXUI_API bool          MenuItemEx(const char* label, const char* icon, const char* shortcut = NULL, bool selected = false, bool enabled = true);

    // Combos
    RBLXUI_API bool          BeginComboPopup(RblxUIID popup_id, const RblxRect& bb, RblxUIComboFlags flags);
    RBLXUI_API bool          BeginComboPreview();
    RBLXUI_API void          EndComboPreview();

    // Keyboard/Gamepad Navigation
    RBLXUI_API void          NavInitWindow(RblxUIWindow* window, bool force_reinit);
    RBLXUI_API void          NavInitRequestApplyResult();
    RBLXUI_API bool          NavMoveRequestButNoResultYet();
    RBLXUI_API void          NavMoveRequestSubmit(RblxUIDir move_dir, RblxUIDir clip_dir, RblxUINavMoveFlags move_flags, RblxUIScrollFlags scroll_flags);
    RBLXUI_API void          NavMoveRequestForward(RblxUIDir move_dir, RblxUIDir clip_dir, RblxUINavMoveFlags move_flags, RblxUIScrollFlags scroll_flags);
    RBLXUI_API void          NavMoveRequestResolveWithLastItem(RblxUINavItemData* result);
    RBLXUI_API void          NavMoveRequestResolveWithPastTreeNode(RblxUINavItemData* result, const RblxUITreeNodeStackData* tree_node_data);
    RBLXUI_API void          NavMoveRequestCancel();
    RBLXUI_API void          NavMoveRequestApplyResult();
    RBLXUI_API void          NavMoveRequestTryWrapping(RblxUIWindow* window, RblxUINavMoveFlags move_flags);
    RBLXUI_API void          NavHighlightActivated(RblxUIID id);
    RBLXUI_API void          NavClearPreferredPosForAxis(RblxUIAxis axis);
    RBLXUI_API void          SetNavCursorVisibleAfterMove();
    RBLXUI_API void          NavUpdateCurrentWindowIsScrollPushableX();
    RBLXUI_API void          SetNavWindow(RblxUIWindow* window);
    RBLXUI_API void          SetNavID(RblxUIID id, RblxUINavLayer nav_layer, RblxUIID focus_scope_id, const RblxRect& rect_rel);
    RBLXUI_API void          SetNavFocusScope(RblxUIID focus_scope_id);

    // Focus/Activation
    // This should be part of a larger set of API: FocusItem(offset = -1), FocusItemByID(id), ActivateItem(offset = -1), ActivateItemByID(id) etc. which are
    // much harder to design and implement than expected. I have a couple of private branches on this matter but it's not simple. For now implementing the easy ones.
    RBLXUI_API void          FocusItem();                    // Focus last item (no selection/activation).
    RBLXUI_API void          ActivateItemByID(RblxUIID id);   // Activate an item by ID (button, checkbox, tree node etc.). Activation is queued and processed on the next frame when the item is encountered again. Was called 'ActivateItem()' before 1.89.7.

    // Inputs
    // FIXME: Eventually we should aim to move e.g. IsActiveIdUsingKey() into IsKeyXXX functions.
    inline bool             IsNamedKey(RblxUIKey key)                    { return key >= RblxUIKey_NamedKey_BEGIN && key < RblxUIKey_NamedKey_END; }
    inline bool             IsNamedKeyOrMod(RblxUIKey key)               { return (key >= RblxUIKey_NamedKey_BEGIN && key < RblxUIKey_NamedKey_END) || key == RblxUIMod_Ctrl || key == RblxUIMod_Shift || key == RblxUIMod_Alt || key == RblxUIMod_Super; }
    inline bool             IsLegacyKey(RblxUIKey key)                   { return key >= RblxUIKey_LegacyNativeKey_BEGIN && key < RblxUIKey_LegacyNativeKey_END; }
    inline bool             IsKeyboardKey(RblxUIKey key)                 { return key >= RblxUIKey_Keyboard_BEGIN && key < RblxUIKey_Keyboard_END; }
    inline bool             IsGamepadKey(RblxUIKey key)                  { return key >= RblxUIKey_Gamepad_BEGIN && key < RblxUIKey_Gamepad_END; }
    inline bool             IsMouseKey(RblxUIKey key)                    { return key >= RblxUIKey_Mouse_BEGIN && key < RblxUIKey_Mouse_END; }
    inline bool             IsAliasKey(RblxUIKey key)                    { return key >= RblxUIKey_Aliases_BEGIN && key < RblxUIKey_Aliases_END; }
    inline bool             IsLRModKey(RblxUIKey key)                    { return key >= RblxUIKey_LeftCtrl && key <= RblxUIKey_RightSuper; }
    RblxUIKeyChord           FixupKeyChord(RblxUIKeyChord key_chord);
    inline RblxUIKey         ConvertSingleModFlagToKey(RblxUIKey key)
    {
        if (key == RblxUIMod_Ctrl) return RblxUIKey_ReservedForModCtrl;
        if (key == RblxUIMod_Shift) return RblxUIKey_ReservedForModShift;
        if (key == RblxUIMod_Alt) return RblxUIKey_ReservedForModAlt;
        if (key == RblxUIMod_Super) return RblxUIKey_ReservedForModSuper;
        return key;
    }

    RBLXUI_API RblxUIKeyData* GetKeyData(RblxUIContext* ctx, RblxUIKey key);
    inline RblxUIKeyData*    GetKeyData(RblxUIKey key)                                    { RblxUIContext& g = *GRblxUI; return GetKeyData(&g, key); }
    RBLXUI_API const char*   GetKeyChordName(RblxUIKeyChord key_chord);
    inline RblxUIKey         MouseButtonToKey(RblxUIMouseButton button)                   { RBLXUI_ASSERT(button >= 0 && button < RblxUIMouseButton_COUNT); return (RblxUIKey)(RblxUIKey_MouseLeft + button); }
    RBLXUI_API bool          IsMouseDragPastThreshold(RblxUIMouseButton button, float lock_threshold = -1.0f);
    RBLXUI_API RblxVec2        GetKeyMagnitude2d(RblxUIKey key_left, RblxUIKey key_right, RblxUIKey key_up, RblxUIKey key_down);
    RBLXUI_API float         GetNavTweakPressedAmount(RblxUIAxis axis);
    RBLXUI_API int           CalcTypematicRepeatAmount(float t0, float t1, float repeat_delay, float repeat_rate);
    RBLXUI_API void          GetTypematicRepeatRate(RblxUIInputFlags flags, float* repeat_delay, float* repeat_rate);
    RBLXUI_API void          TeleportMousePos(const RblxVec2& pos);
    RBLXUI_API void          SetActiveIdUsingAllKeyboardKeys();
    inline bool             IsActiveIdUsingNavDir(RblxUIDir dir)                         { RblxUIContext& g = *GRblxUI; return (g.ActiveIdUsingNavDirMask & (1 << dir)) != 0; }

    // [EXPERIMENTAL] Low-Level: Key/Input Ownership
    // - The idea is that instead of "eating" a given input, we can link to an owner id.
    // - Ownership is most often claimed as a result of reacting to a press/down event (but occasionally may be claimed ahead).
    // - Input queries can then read input by specifying RblxUIKeyOwner_Any (== 0), RblxUIKeyOwner_NoOwner (== -1) or a custom ID.
    // - Legacy input queries (without specifying an owner or _Any or _None) are equivalent to using RblxUIKeyOwner_Any (== 0).
    // - Input ownership is automatically released on the frame after a key is released. Therefore:
    //   - for ownership registration happening as a result of a down/press event, the SetKeyOwner() call may be done once (common case).
    //   - for ownership registration happening ahead of a down/press event, the SetKeyOwner() call needs to be made every frame (happens if e.g. claiming ownership on hover).
    // - SetItemKeyOwner() is a shortcut for common simple case. A custom widget will probably want to call SetKeyOwner() multiple times directly based on its interaction state.
    // - This is marked experimental because not all widgets are fully honoring the Set/Test idioms. We will need to move forward step by step.
    //   Please open a GitHub Issue to submit your usage scenario or if there's a use case you need solved.
    RBLXUI_API RblxUIID       GetKeyOwner(RblxUIKey key);
    RBLXUI_API void          SetKeyOwner(RblxUIKey key, RblxUIID owner_id, RblxUIInputFlags flags = 0);
    RBLXUI_API void          SetKeyOwnersForKeyChord(RblxUIKeyChord key, RblxUIID owner_id, RblxUIInputFlags flags = 0);
    RBLXUI_API bool          SetItemKeyOwner(RblxUIKey key, RblxUIInputFlags flags);
    RBLXUI_API bool          TestKeyOwner(RblxUIKey key, RblxUIID owner_id);               // Test that key is either not owned, either owned by 'owner_id'
    inline RblxUIKeyOwnerData* GetKeyOwnerData(RblxUIContext* ctx, RblxUIKey key)          { if (key & RblxUIMod_Mask_) key = ConvertSingleModFlagToKey(key); RBLXUI_ASSERT(IsNamedKey(key)); return &ctx->KeysOwnerData[key - RblxUIKey_NamedKey_BEGIN]; }

    // [EXPERIMENTAL] High-Level: Input Access functions w/ support for Key/Input Ownership
    // - Important: legacy IsKeyPressed(RblxUIKey, bool repeat=true) _DEFAULTS_ to repeat, new IsKeyPressed() requires _EXPLICIT_ RblxUIInputFlags_Repeat flag.
    // - Expected to be later promoted to public API, the prototypes are designed to replace existing ones (since owner_id can default to Any == 0)
    // - Specifying a value for 'RblxUIID owner' will test that EITHER the key is NOT owned (UNLESS locked), EITHER the key is owned by 'owner'.
    //   Legacy functions use RblxUIKeyOwner_Any meaning that they typically ignore ownership, unless a call to SetKeyOwner() explicitly used RblxUIInputFlags_LockThisFrame or RblxUIInputFlags_LockUntilRelease.
    // - Binding generators may want to ignore those for now, or suffix them with Ex() until we decide if this gets moved into public API.
    RBLXUI_API bool          IsKeyDown(RblxUIKey key, RblxUIID owner_id);
    RBLXUI_API bool          IsKeyPressed(RblxUIKey key, RblxUIInputFlags flags, RblxUIID owner_id = 0);    // Important: when transitioning from old to new IsKeyPressed(): old API has "bool repeat = true", so would default to repeat. New API requires explicit RblxUIInputFlags_Repeat.
    RBLXUI_API bool          IsKeyReleased(RblxUIKey key, RblxUIID owner_id);
    RBLXUI_API bool          IsKeyChordPressed(RblxUIKeyChord key_chord, RblxUIInputFlags flags, RblxUIID owner_id = 0);
    RBLXUI_API bool          IsMouseDown(RblxUIMouseButton button, RblxUIID owner_id);
    RBLXUI_API bool          IsMouseClicked(RblxUIMouseButton button, RblxUIInputFlags flags, RblxUIID owner_id = 0);
    RBLXUI_API bool          IsMouseReleased(RblxUIMouseButton button, RblxUIID owner_id);
    RBLXUI_API bool          IsMouseDoubleClicked(RblxUIMouseButton button, RblxUIID owner_id);

    // Shortcut Testing & Routing
    // - Set Shortcut() and SetNextItemShortcut() in rblxui.h
    // - When a policy (except for RblxUIInputFlags_RouteAlways *) is set, Shortcut() will register itself with SetShortcutRouting(),
    //   allowing the system to decide where to route the input among other route-aware calls.
    //   (* using RblxUIInputFlags_RouteAlways is roughly equivalent to calling IsKeyChordPressed(key) and bypassing route registration and check)
    // - When using one of the routing option:
    //   - The default route is RblxUIInputFlags_RouteFocused (accept inputs if window is in focus stack. Deep-most focused window takes inputs. ActiveId takes inputs over deep-most focused window.)
    //   - Routes are requested given a chord (key + modifiers) and a routing policy.
    //   - Routes are resolved during NewFrame(): if keyboard modifiers are matching current ones: SetKeyOwner() is called + route is granted for the frame.
    //   - Each route may be granted to a single owner. When multiple requests are made we have policies to select the winning route (e.g. deep most window).
    //   - Multiple read sites may use the same owner id can all access the granted route.
    //   - When owner_id is 0 we use the current Focus Scope ID as a owner ID in order to identify our location.
    // - You can chain two unrelated windows in the focus stack using SetWindowParentWindowForFocusRoute()
    //   e.g. if you have a tool window associated to a document, and you want document shortcuts to run when the tool is focused.
    RBLXUI_API bool          Shortcut(RblxUIKeyChord key_chord, RblxUIInputFlags flags, RblxUIID owner_id);
    RBLXUI_API bool          SetShortcutRouting(RblxUIKeyChord key_chord, RblxUIInputFlags flags, RblxUIID owner_id); // owner_id needs to be explicit and cannot be 0
    RBLXUI_API bool          TestShortcutRouting(RblxUIKeyChord key_chord, RblxUIID owner_id);
    RBLXUI_API RblxUIKeyRoutingData* GetShortcutRoutingData(RblxUIKeyChord key_chord);

    // Docking
    // (some functions are only declared in rblxui.cpp, see Docking section)
    RBLXUI_API void          DockContextInitialize(RblxUIContext* ctx);
    RBLXUI_API void          DockContextShutdown(RblxUIContext* ctx);
    RBLXUI_API void          DockContextClearNodes(RblxUIContext* ctx, RblxUIID root_id, bool clear_settings_refs); // Use root_id==0 to clear all
    RBLXUI_API void          DockContextRebuildNodes(RblxUIContext* ctx);
    RBLXUI_API void          DockContextNewFrameUpdateUndocking(RblxUIContext* ctx);
    RBLXUI_API void          DockContextNewFrameUpdateDocking(RblxUIContext* ctx);
    RBLXUI_API void          DockContextEndFrame(RblxUIContext* ctx);
    RBLXUI_API RblxUIID       DockContextGenNodeID(RblxUIContext* ctx);
    RBLXUI_API void          DockContextQueueDock(RblxUIContext* ctx, RblxUIWindow* target, RblxUIDockNode* target_node, RblxUIWindow* payload, RblxUIDir split_dir, float split_ratio, bool split_outer);
    RBLXUI_API void          DockContextQueueUndockWindow(RblxUIContext* ctx, RblxUIWindow* window);
    RBLXUI_API void          DockContextQueueUndockNode(RblxUIContext* ctx, RblxUIDockNode* node);
    RBLXUI_API void          DockContextProcessUndockWindow(RblxUIContext* ctx, RblxUIWindow* window, bool clear_persistent_docking_ref = true);
    RBLXUI_API void          DockContextProcessUndockNode(RblxUIContext* ctx, RblxUIDockNode* node);
    RBLXUI_API bool          DockContextCalcDropPosForDocking(RblxUIWindow* target, RblxUIDockNode* target_node, RblxUIWindow* payload_window, RblxUIDockNode* payload_node, RblxUIDir split_dir, bool split_outer, RblxVec2* out_pos);
    RBLXUI_API RblxUIDockNode*DockContextFindNodeByID(RblxUIContext* ctx, RblxUIID id);
    RBLXUI_API void          DockNodeWindowMenuHandler_Default(RblxUIContext* ctx, RblxUIDockNode* node, RblxUITabBar* tab_bar);
    RBLXUI_API bool          DockNodeBeginAmendTabBar(RblxUIDockNode* node);
    RBLXUI_API void          DockNodeEndAmendTabBar();
    inline RblxUIDockNode*   DockNodeGetRootNode(RblxUIDockNode* node)                 { while (node->ParentNode) node = node->ParentNode; return node; }
    inline bool             DockNodeIsInHierarchyOf(RblxUIDockNode* node, RblxUIDockNode* parent) { while (node) { if (node == parent) return true; node = node->ParentNode; } return false; }
    inline int              DockNodeGetDepth(const RblxUIDockNode* node)              { int depth = 0; while (node->ParentNode) { node = node->ParentNode; depth++; } return depth; }
    inline RblxUIID          DockNodeGetWindowMenuButtonId(const RblxUIDockNode* node) { return RblxHashStr("#COLLAPSE", 0, node->ID); }
    inline RblxUIDockNode*   GetWindowDockNode()                                      { RblxUIContext& g = *GRblxUI; return g.CurrentWindow->DockNode; }
    RBLXUI_API bool          GetWindowAlwaysWantOwnTabBar(RblxUIWindow* window);
    RBLXUI_API void          BeginDocked(RblxUIWindow* window, bool* p_open);
    RBLXUI_API void          BeginDockableDragDropSource(RblxUIWindow* window);
    RBLXUI_API void          BeginDockableDragDropTarget(RblxUIWindow* window);
    RBLXUI_API void          SetWindowDock(RblxUIWindow* window, RblxUIID dock_id, RblxUICond cond);

    // Docking - Builder function needs to be generally called before the node is used/submitted.
    // - The DockBuilderXXX functions are designed to _eventually_ become a public API, but it is too early to expose it and guarantee stability.
    // - Do not hold on RblxUIDockNode* pointers! They may be invalidated by any split/merge/remove operation and every frame.
    // - To create a DockSpace() node, make sure to set the RblxUIDockNodeFlags_DockSpace flag when calling DockBuilderAddNode().
    //   You can create dockspace nodes (attached to a window) _or_ floating nodes (carry its own window) with this API.
    // - DockBuilderSplitNode() create 2 child nodes within 1 node. The initial node becomes a parent node.
    // - If you intend to split the node immediately after creation using DockBuilderSplitNode(), make sure
    //   to call DockBuilderSetNodeSize() beforehand. If you don't, the resulting split sizes may not be reliable.
    // - Call DockBuilderFinish() after you are done.
    RBLXUI_API void          DockBuilderDockWindow(const char* window_name, RblxUIID node_id);
    RBLXUI_API RblxUIDockNode*DockBuilderGetNode(RblxUIID node_id);
    inline RblxUIDockNode*   DockBuilderGetCentralNode(RblxUIID node_id)              { RblxUIDockNode* node = DockBuilderGetNode(node_id); if (!node) return NULL; return DockNodeGetRootNode(node)->CentralNode; }
    RBLXUI_API RblxUIID       DockBuilderAddNode(RblxUIID node_id = 0, RblxUIDockNodeFlags flags = 0);
    RBLXUI_API void          DockBuilderRemoveNode(RblxUIID node_id);                 // Remove node and all its child, undock all windows
    RBLXUI_API void          DockBuilderRemoveNodeDockedWindows(RblxUIID node_id, bool clear_settings_refs = true);
    RBLXUI_API void          DockBuilderRemoveNodeChildNodes(RblxUIID node_id);       // Remove all split/hierarchy. All remaining docked windows will be re-docked to the remaining root node (node_id).
    RBLXUI_API void          DockBuilderSetNodePos(RblxUIID node_id, RblxVec2 pos);
    RBLXUI_API void          DockBuilderSetNodeSize(RblxUIID node_id, RblxVec2 size);
    RBLXUI_API RblxUIID       DockBuilderSplitNode(RblxUIID node_id, RblxUIDir split_dir, float size_ratio_for_node_at_dir, RblxUIID* out_id_at_dir, RblxUIID* out_id_at_opposite_dir); // Create 2 child nodes in this parent node.
    RBLXUI_API void          DockBuilderCopyDockSpace(RblxUIID src_dockspace_id, RblxUIID dst_dockspace_id, RblxVector<const char*>* in_window_remap_pairs);
    RBLXUI_API void          DockBuilderCopyNode(RblxUIID src_node_id, RblxUIID dst_node_id, RblxVector<RblxUIID>* out_node_remap_pairs);
    RBLXUI_API void          DockBuilderCopyWindowSettings(const char* src_name, const char* dst_name);
    RBLXUI_API void          DockBuilderFinish(RblxUIID node_id);

    // [EXPERIMENTAL] Focus Scope
    // This is generally used to identify a unique input location (for e.g. a selection set)
    // There is one per window (automatically set in Begin), but:
    // - Selection patterns generally need to react (e.g. clear a selection) when landing on one item of the set.
    //   So in order to identify a set multiple lists in same window may each need a focus scope.
    //   If you imagine an hypothetical BeginSelectionGroup()/EndSelectionGroup() api, it would likely call PushFocusScope()/EndFocusScope()
    // - Shortcut routing also use focus scope as a default location identifier if an owner is not provided.
    // We don't use the ID Stack for this as it is common to want them separate.
    RBLXUI_API void          PushFocusScope(RblxUIID id);
    RBLXUI_API void          PopFocusScope();
    RBLXUI_API bool          IsInNavFocusRoute(RblxUIID focus_scope_id);
    inline RblxUIID          GetCurrentFocusScope() { RblxUIContext& g = *GRblxUI; return g.CurrentFocusScopeId; }   // Focus scope we are outputting into, set by PushFocusScope()

    // Drag and Drop
    RBLXUI_API bool          IsDragDropActive();
    RBLXUI_API bool          BeginDragDropTargetCustom(const RblxRect& bb, RblxUIID id);
    RBLXUI_API bool          BeginDragDropTargetViewport(RblxUIViewport* viewport, const RblxRect* p_bb = NULL);
    RBLXUI_API void          ClearDragDrop();
    RBLXUI_API bool          IsDragDropPayloadBeingAccepted();
    RBLXUI_API void          RenderDragDropTargetRectForItem(const RblxRect& bb);
    RBLXUI_API void          RenderDragDropTargetRectEx(RblxDrawList* draw_list, const RblxRect& bb, float rounding);

    // Typing-Select API
    // (provide Windows Explorer style "select items by typing partial name" + "cycle through items by typing same letter" feature)
    // (this is currently not documented nor used by main library, but should work. See "widgets_typingselect" in rblxui_test_suite for usage code. Please let us know if you use this!)
    RBLXUI_API RblxUITypingSelectRequest* GetTypingSelectRequest(RblxUITypingSelectFlags flags = RblxUITypingSelectFlags_None);
    RBLXUI_API int           TypingSelectFindMatch(RblxUITypingSelectRequest* req, int items_count, const char* (*get_item_name_func)(void*, int), void* user_data, int nav_item_idx);
    RBLXUI_API int           TypingSelectFindNextSingleCharMatch(RblxUITypingSelectRequest* req, int items_count, const char* (*get_item_name_func)(void*, int), void* user_data, int nav_item_idx);
    RBLXUI_API int           TypingSelectFindBestLeadingMatch(RblxUITypingSelectRequest* req, int items_count, const char* (*get_item_name_func)(void*, int), void* user_data);

    // Box-Select API
    RBLXUI_API bool          BeginBoxSelect(const RblxRect& scope_rect, RblxUIWindow* window, RblxUIID box_select_id, RblxUIMultiSelectFlags ms_flags);
    RBLXUI_API void          EndBoxSelect(const RblxRect& scope_rect, RblxUIMultiSelectFlags ms_flags);

    // Multi-Select API
    RBLXUI_API void          MultiSelectItemHeader(RblxUIID id, bool* p_selected, RblxUIButtonFlags* p_button_flags);
    RBLXUI_API void          MultiSelectItemFooter(RblxUIID id, bool* p_selected, bool* p_pressed);
    RBLXUI_API void          MultiSelectAddSetAll(RblxUIMultiSelectTempData* ms, bool selected);
    RBLXUI_API void          MultiSelectAddSetRange(RblxUIMultiSelectTempData* ms, bool selected, int range_dir, RblxUISelectionUserData first_item, RblxUISelectionUserData last_item);
    inline RblxUIBoxSelectState*     GetBoxSelectState(RblxUIID id)   { RblxUIContext& g = *GRblxUI; return (id != 0 && g.BoxSelectState.ID == id && g.BoxSelectState.IsActive) ? &g.BoxSelectState : NULL; }
    inline RblxUIMultiSelectState*   GetMultiSelectState(RblxUIID id) { RblxUIContext& g = *GRblxUI; return g.MultiSelectStorage.GetByKey(id); }

    // Internal Columns API (this is not exposed because we will encourage transitioning to the Tables API)
    RBLXUI_API void          SetWindowClipRectBeforeSetChannel(RblxUIWindow* window, const RblxRect& clip_rect);
    RBLXUI_API void          BeginColumns(const char* str_id, int count, RblxUIOldColumnFlags flags = 0); // setup number of columns. use an identifier to distinguish multiple column sets. close with EndColumns().
    RBLXUI_API void          EndColumns();                                                               // close columns
    RBLXUI_API void          PushColumnClipRect(int column_index);
    RBLXUI_API void          PushColumnsBackground();
    RBLXUI_API void          PopColumnsBackground();
    RBLXUI_API RblxUIID       GetColumnsID(const char* str_id, int count);
    RBLXUI_API RblxUIOldColumns* FindOrCreateColumns(RblxUIWindow* window, RblxUIID id);
    RBLXUI_API float         GetColumnOffsetFromNorm(const RblxUIOldColumns* columns, float offset_norm);
    RBLXUI_API float         GetColumnNormFromOffset(const RblxUIOldColumns* columns, float offset);

    // Tables: Candidates for public API
    RBLXUI_API void          TableOpenContextMenu(int column_n = -1);
    RBLXUI_API void          TableSetColumnWidth(int column_n, float width);
    RBLXUI_API void          TableSetColumnSortDirection(int column_n, RblxUISortDirection sort_direction, bool append_to_sort_specs);
    RBLXUI_API int           TableGetHoveredRow();       // Retrieve *PREVIOUS FRAME* hovered row. This difference with TableGetHoveredColumn() is the reason why this is not public yet.
    RBLXUI_API float         TableGetHeaderRowHeight();
    RBLXUI_API float         TableGetHeaderAngledMaxLabelWidth();
    RBLXUI_API void          TablePushBackgroundChannel();
    RBLXUI_API void          TablePopBackgroundChannel();
    RBLXUI_API void          TablePushColumnChannel(int column_n);
    RBLXUI_API void          TablePopColumnChannel();
    RBLXUI_API void          TableAngledHeadersRowEx(RblxUIID row_id, float angle, float max_label_width, const RblxUITableHeaderData* data, int data_count);

    // Tables: Internals
    inline    RblxUITable*   GetCurrentTable() { RblxUIContext& g = *GRblxUI; return g.CurrentTable; }
    RBLXUI_API RblxUITable*   TableFindByID(RblxUIID id);
    RBLXUI_API bool          BeginTableEx(const char* name, RblxUIID id, int columns_count, RblxUITableFlags flags = 0, const RblxVec2& outer_size = RblxVec2(0, 0), float inner_width = 0.0f);
    RBLXUI_API void          TableBeginInitMemory(RblxUITable* table, int columns_count);
    RBLXUI_API void          TableBeginApplyRequests(RblxUITable* table);
    RBLXUI_API void          TableSetupDrawChannels(RblxUITable* table);
    RBLXUI_API void          TableUpdateLayout(RblxUITable* table);
    RBLXUI_API void          TableUpdateBorders(RblxUITable* table);
    RBLXUI_API void          TableUpdateColumnsWeightFromWidth(RblxUITable* table);
    RBLXUI_API void          TableApplyExternalUnclipRect(RblxUITable* table, RblxRect& rect);
    RBLXUI_API void          TableDrawBorders(RblxUITable* table);
    RBLXUI_API void          TableDrawDefaultContextMenu(RblxUITable* table, RblxUITableFlags flags_for_section_to_display);
    RBLXUI_API bool          TableBeginContextMenuPopup(RblxUITable* table);
    RBLXUI_API void          TableMergeDrawChannels(RblxUITable* table);
    inline RblxUITableInstanceData*  TableGetInstanceData(RblxUITable* table, int instance_no) { if (instance_no == 0) return &table->InstanceDataFirst; return &table->InstanceDataExtra[instance_no - 1]; }
    inline RblxUIID                  TableGetInstanceID(RblxUITable* table, int instance_no)   { return TableGetInstanceData(table, instance_no)->TableInstanceID; }
    RBLXUI_API void          TableFixDisplayOrder(RblxUITable* table);
    RBLXUI_API void          TableSortSpecsSanitize(RblxUITable* table);
    RBLXUI_API void          TableSortSpecsBuild(RblxUITable* table);
    RBLXUI_API RblxUISortDirection TableGetColumnNextSortDirection(RblxUITableColumn* column);
    RBLXUI_API void          TableFixColumnSortDirection(RblxUITable* table, RblxUITableColumn* column);
    RBLXUI_API float         TableGetColumnWidthAuto(RblxUITable* table, RblxUITableColumn* column);
    RBLXUI_API void          TableBeginRow(RblxUITable* table);
    RBLXUI_API void          TableEndRow(RblxUITable* table);
    RBLXUI_API void          TableBeginCell(RblxUITable* table, int column_n);
    RBLXUI_API void          TableEndCell(RblxUITable* table);
    RBLXUI_API RblxRect        TableGetCellBgRect(const RblxUITable* table, int column_n);
    RBLXUI_API const char*   TableGetColumnName(const RblxUITable* table, int column_n);
    RBLXUI_API RblxUIID       TableGetColumnResizeID(RblxUITable* table, int column_n, int instance_no = 0);
    RBLXUI_API float         TableCalcMaxColumnWidth(const RblxUITable* table, int column_n);
    RBLXUI_API void          TableSetColumnWidthAutoSingle(RblxUITable* table, int column_n);
    RBLXUI_API void          TableSetColumnWidthAutoAll(RblxUITable* table);
    RBLXUI_API void          TableSetColumnDisplayOrder(RblxUITable* table, int column_n, int dst_order);
    RBLXUI_API void          TableQueueSetColumnDisplayOrder(RblxUITable* table, int column_n, int dst_order);
    RBLXUI_API void          TableRemove(RblxUITable* table);
    RBLXUI_API void          TableGcCompactTransientBuffers(RblxUITable* table);
    RBLXUI_API void          TableGcCompactTransientBuffers(RblxUITableTempData* table);
    RBLXUI_API void          TableGcCompactSettings();

    // Tables: Settings
    RBLXUI_API void                  TableLoadSettings(RblxUITable* table);
    RBLXUI_API void                  TableSaveSettings(RblxUITable* table);
    RBLXUI_API void                  TableResetSettings(RblxUITable* table);
    RBLXUI_API RblxUITableSettings*   TableGetBoundSettings(RblxUITable* table);
    RBLXUI_API void                  TableSettingsAddSettingsHandler();
    RBLXUI_API RblxUITableSettings*   TableSettingsCreate(RblxUIID id, int columns_count);
    RBLXUI_API RblxUITableSettings*   TableSettingsFindByID(RblxUIID id);

    // Tab Bars
    inline    RblxUITabBar*  GetCurrentTabBar() { RblxUIContext& g = *GRblxUI; return g.CurrentTabBar; }
    RBLXUI_API RblxUITabBar*  TabBarFindByID(RblxUIID id);
    RBLXUI_API void          TabBarRemove(RblxUITabBar* tab_bar);
    RBLXUI_API bool          BeginTabBarEx(RblxUITabBar* tab_bar, const RblxRect& bb, RblxUITabBarFlags flags);
    RBLXUI_API RblxUITabItem* TabBarFindTabByID(RblxUITabBar* tab_bar, RblxUIID tab_id);
    RBLXUI_API RblxUITabItem* TabBarFindTabByOrder(RblxUITabBar* tab_bar, int order);
    RBLXUI_API RblxUITabItem* TabBarFindMostRecentlySelectedTabForActiveWindow(RblxUITabBar* tab_bar);
    RBLXUI_API RblxUITabItem* TabBarGetCurrentTab(RblxUITabBar* tab_bar);
    inline int              TabBarGetTabOrder(RblxUITabBar* tab_bar, RblxUITabItem* tab) { return tab_bar->Tabs.index_from_ptr(tab); }
    RBLXUI_API const char*   TabBarGetTabName(RblxUITabBar* tab_bar, RblxUITabItem* tab);
    RBLXUI_API void          TabBarAddTab(RblxUITabBar* tab_bar, RblxUITabItemFlags tab_flags, RblxUIWindow* window);
    RBLXUI_API void          TabBarRemoveTab(RblxUITabBar* tab_bar, RblxUIID tab_id);
    RBLXUI_API void          TabBarCloseTab(RblxUITabBar* tab_bar, RblxUITabItem* tab);
    RBLXUI_API void          TabBarQueueFocus(RblxUITabBar* tab_bar, RblxUITabItem* tab);
    RBLXUI_API void          TabBarQueueFocus(RblxUITabBar* tab_bar, const char* tab_name);
    RBLXUI_API void          TabBarQueueReorder(RblxUITabBar* tab_bar, RblxUITabItem* tab, int offset);
    RBLXUI_API void          TabBarQueueReorderFromMousePos(RblxUITabBar* tab_bar, RblxUITabItem* tab, RblxVec2 mouse_pos);
    RBLXUI_API bool          TabBarProcessReorder(RblxUITabBar* tab_bar);
    RBLXUI_API bool          TabItemEx(RblxUITabBar* tab_bar, const char* label, bool* p_open, RblxUITabItemFlags flags, RblxUIWindow* docked_window);
    RBLXUI_API void          TabItemSpacing(const char* str_id, RblxUITabItemFlags flags, float width);
    RBLXUI_API RblxVec2        TabItemCalcSize(const char* label, bool has_close_button_or_unsaved_marker);
    RBLXUI_API RblxVec2        TabItemCalcSize(RblxUIWindow* window);
    RBLXUI_API void          TabItemBackground(RblxDrawList* draw_list, const RblxRect& bb, RblxUITabItemFlags flags, RblxU32 col);
    RBLXUI_API void          TabItemLabelAndCloseButton(RblxDrawList* draw_list, const RblxRect& bb, RblxUITabItemFlags flags, RblxVec2 frame_padding, const char* label, RblxUIID tab_id, RblxUIID close_button_id, bool is_contents_visible, bool* out_just_closed, bool* out_text_clipped);

    // Render helpers
    // AVOID USING OUTSIDE OF RBLXUI.CPP! NOT FOR PUBLIC CONSUMPTION. THOSE FUNCTIONS ARE A MESS. THEIR SIGNATURE AND BEHAVIOR WILL CHANGE, THEY NEED TO BE REFACTORED INTO SOMETHING DECENT.
    // NB: All position are in absolute pixels coordinates (we are never using window coordinates internally)
    RBLXUI_API void          RenderText(RblxVec2 pos, const char* text, const char* text_end = NULL, bool hide_text_after_hash = true);
    RBLXUI_API void          RenderTextWrapped(RblxVec2 pos, const char* text, const char* text_end, float wrap_width);
    RBLXUI_API void          RenderTextClipped(const RblxVec2& pos_min, const RblxVec2& pos_max, const char* text, const char* text_end, const RblxVec2* text_size_if_known, const RblxVec2& align = RblxVec2(0, 0), const RblxRect* clip_rect = NULL);
    RBLXUI_API void          RenderTextClippedEx(RblxDrawList* draw_list, const RblxVec2& pos_min, const RblxVec2& pos_max, const char* text, const char* text_end, const RblxVec2* text_size_if_known, const RblxVec2& align = RblxVec2(0, 0), const RblxRect* clip_rect = NULL);
    RBLXUI_API void          RenderTextEllipsis(RblxDrawList* draw_list, const RblxVec2& pos_min, const RblxVec2& pos_max, float ellipsis_max_x, const char* text, const char* text_end, const RblxVec2* text_size_if_known);
    RBLXUI_API void          RenderFrame(RblxVec2 p_min, RblxVec2 p_max, RblxU32 fill_col, bool borders = true, float rounding = 0.0f);
    RBLXUI_API void          RenderFrameBorder(RblxVec2 p_min, RblxVec2 p_max, float rounding = 0.0f);
    RBLXUI_API void          RenderColorComponentMarker(const RblxRect& bb, RblxU32 col, float rounding);
    RBLXUI_API void          RenderColorRectWithAlphaCheckerboard(RblxDrawList* draw_list, RblxVec2 p_min, RblxVec2 p_max, RblxU32 fill_col, float grid_step, RblxVec2 grid_off, float rounding = 0.0f, RblxDrawFlags flags = 0);
    RBLXUI_API void          RenderNavCursor(const RblxRect& bb, RblxUIID id, RblxUINavRenderCursorFlags flags = RblxUINavRenderCursorFlags_None); // Navigation highlight
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    inline    void          RenderNavHighlight(const RblxRect& bb, RblxUIID id, RblxUINavRenderCursorFlags flags = RblxUINavRenderCursorFlags_None) { RenderNavCursor(bb, id, flags); } // Renamed in 1.91.4
#endif
    RBLXUI_API const char*   FindRenderedTextEnd(const char* text, const char* text_end = NULL); // Find the optional ## from which we stop displaying text.
    RBLXUI_API void          RenderMouseCursor(RblxVec2 pos, float scale, RblxUIMouseCursor mouse_cursor, RblxU32 col_fill, RblxU32 col_border, RblxU32 col_shadow);

    // Render helpers (those functions don't access any RblxUI state!)
    RBLXUI_API void          RenderArrow(RblxDrawList* draw_list, RblxVec2 pos, RblxU32 col, RblxUIDir dir, float scale = 1.0f);
    RBLXUI_API void          RenderBullet(RblxDrawList* draw_list, RblxVec2 pos, RblxU32 col);
    RBLXUI_API void          RenderCheckMark(RblxDrawList* draw_list, RblxVec2 pos, RblxU32 col, float sz);
    RBLXUI_API void          RenderArrowPointingAt(RblxDrawList* draw_list, RblxVec2 pos, RblxVec2 half_sz, RblxUIDir direction, RblxU32 col);
    RBLXUI_API void          RenderArrowDockMenu(RblxDrawList* draw_list, RblxVec2 p_min, float sz, RblxU32 col);
    RBLXUI_API void          RenderRectFilledInRangeH(RblxDrawList* draw_list, const RblxRect& rect, RblxU32 col, float fill_x0, float fill_x1, float rounding);
    RBLXUI_API void          RenderRectFilledWithHole(RblxDrawList* draw_list, const RblxRect& outer, const RblxRect& inner, RblxU32 col, float rounding);
    RBLXUI_API RblxDrawFlags   CalcRoundingFlagsForRectInRect(const RblxRect& r_in, const RblxRect& r_outer, float threshold);

    // Widgets: Text
    RBLXUI_API void          TextEx(const char* text, const char* text_end = NULL, RblxUITextFlags flags = 0);
    RBLXUI_API void          TextAligned(float align_x, float size_x, const char* fmt, ...);               // FIXME-WIP: Works but API is likely to be reworked. This is designed for 1 item on the line. (#7024)
    RBLXUI_API void          TextAlignedV(float align_x, float size_x, const char* fmt, va_list args);

    // Widgets
    RBLXUI_API bool          ButtonEx(const char* label, const RblxVec2& size_arg = RblxVec2(0, 0), RblxUIButtonFlags flags = 0);
    RBLXUI_API bool          ArrowButtonEx(const char* str_id, RblxUIDir dir, RblxVec2 size_arg, RblxUIButtonFlags flags = 0);
    RBLXUI_API bool          ImageButtonEx(RblxUIID id, RblxTextureRef tex_ref, const RblxVec2& image_size, const RblxVec2& uv0, const RblxVec2& uv1, const RblxVec4& bg_col, const RblxVec4& tint_col, RblxUIButtonFlags flags = 0);
    RBLXUI_API void          SeparatorEx(RblxUISeparatorFlags flags, float thickness = 1.0f);
    RBLXUI_API void          SeparatorTextEx(RblxUIID id, const char* label, const char* label_end, float extra_width);
    RBLXUI_API bool          CheckboxFlags(const char* label, RblxS64* flags, RblxS64 flags_value);
    RBLXUI_API bool          CheckboxFlags(const char* label, RblxU64* flags, RblxU64 flags_value);

    // Widgets: Window Decorations
    RBLXUI_API bool          CloseButton(RblxUIID id, const RblxVec2& pos);
    RBLXUI_API bool          CollapseButton(RblxUIID id, const RblxVec2& pos, RblxUIDockNode* dock_node);
    RBLXUI_API void          Scrollbar(RblxUIAxis axis);
    RBLXUI_API bool          ScrollbarEx(const RblxRect& bb, RblxUIID id, RblxUIAxis axis, RblxS64* p_scroll_v, RblxS64 avail_v, RblxS64 contents_v, RblxDrawFlags draw_rounding_flags = 0);
    RBLXUI_API RblxRect        GetWindowScrollbarRect(RblxUIWindow* window, RblxUIAxis axis);
    RBLXUI_API RblxUIID       GetWindowScrollbarID(RblxUIWindow* window, RblxUIAxis axis);
    RBLXUI_API RblxUIID       GetWindowResizeCornerID(RblxUIWindow* window, int n); // 0..3: corners
    RBLXUI_API RblxUIID       GetWindowResizeBorderID(RblxUIWindow* window, RblxUIDir dir);
    RBLXUI_API void          ExtendHitBoxWhenNearViewportEdge(RblxUIWindow* window, RblxRect* bb, float threshold, RblxUIAxis axis);

    // Widgets low-level behaviors
    RBLXUI_API bool          ButtonBehavior(const RblxRect& bb, RblxUIID id, bool* out_hovered, bool* out_held, RblxUIButtonFlags flags = 0);
    RBLXUI_API bool          DragBehavior(RblxUIID id, RblxUIDataType data_type, void* p_v, float v_speed, const void* p_min, const void* p_max, const char* format, RblxUISliderFlags flags);
    RBLXUI_API bool          SliderBehavior(const RblxRect& bb, RblxUIID id, RblxUIDataType data_type, void* p_v, const void* p_min, const void* p_max, const char* format, RblxUISliderFlags flags, RblxRect* out_grab_bb);
    RBLXUI_API bool          SplitterBehavior(const RblxRect& bb, RblxUIID id, RblxUIAxis axis, float* size1, float* size2, float min_size1, float min_size2, float hover_extend = 0.0f, float hover_visibility_delay = 0.0f, RblxU32 bg_col = 0);

    // Widgets: Tree Nodes
    RBLXUI_API bool          TreeNodeBehavior(RblxUIID id, RblxUITreeNodeFlags flags, const char* label, const char* label_end = NULL);
    RBLXUI_API void          TreeNodeDrawLineToChildNode(const RblxVec2& target_pos);
    RBLXUI_API void          TreeNodeDrawLineToTreePop(const RblxUITreeNodeStackData* data);
    RBLXUI_API void          TreePushOverrideID(RblxUIID id);
    RBLXUI_API void          TreeNodeSetOpen(RblxUIID storage_id, bool open);
    RBLXUI_API bool          TreeNodeUpdateNextOpen(RblxUIID storage_id, RblxUITreeNodeFlags flags);   // Return open state. Consume previous SetNextItemOpen() data, if any. May return true when logging.

    // Template functions are instantiated in rblxui_widgets.cpp for a finite number of types.
    // To use them externally (for custom widget) you may need an "extern template" statement in your code in order to link to existing instances and silence Clang warnings (see #2036).
    // e.g. " extern template RBLXUI_API float RoundScalarWithFormatT<float, float>(const char* format, RblxUIDataType data_type, float v); "
    template<typename T, typename SIGNED_T, typename FLOAT_T>   RBLXUI_API float ScaleRatioFromValueT(RblxUIDataType data_type, T v, T v_min, T v_max, float logarithmic_zero_epsilon, float zero_deadzone_size);
    template<typename T, typename SIGNED_T, typename FLOAT_T>   RBLXUI_API T     ScaleValueFromRatioT(RblxUIDataType data_type, float t, T v_min, T v_max, float logarithmic_zero_epsilon, float zero_deadzone_size);
    template<typename T, typename SIGNED_T, typename FLOAT_T>   RBLXUI_API bool  DragBehaviorT(RblxUIDataType data_type, T* v, float v_speed, T v_min, T v_max, const char* format, RblxUISliderFlags flags);
    template<typename T, typename SIGNED_T, typename FLOAT_T>   RBLXUI_API bool  SliderBehaviorT(const RblxRect& bb, RblxUIID id, RblxUIDataType data_type, T* v, T v_min, T v_max, const char* format, RblxUISliderFlags flags, RblxRect* out_grab_bb);
    template<typename T>                                        RBLXUI_API T     RoundScalarWithFormatT(const char* format, RblxUIDataType data_type, T v);
    template<typename T>                                        RBLXUI_API bool  CheckboxFlagsT(const char* label, T* flags, T flags_value);

    // Data type helpers
    RBLXUI_API const RblxUIDataTypeInfo*  DataTypeGetInfo(RblxUIDataType data_type);
    RBLXUI_API int           DataTypeFormatString(char* buf, int buf_size, RblxUIDataType data_type, const void* p_data, const char* format);
    RBLXUI_API void          DataTypeApplyOp(RblxUIDataType data_type, int op, void* output, const void* arg_1, const void* arg_2);
    RBLXUI_API bool          DataTypeApplyFromText(const char* buf, RblxUIDataType data_type, void* p_data, const char* format, void* p_data_when_empty = NULL);
    RBLXUI_API int           DataTypeCompare(RblxUIDataType data_type, const void* arg_1, const void* arg_2);
    RBLXUI_API bool          DataTypeClamp(RblxUIDataType data_type, void* p_data, const void* p_min, const void* p_max);
    RBLXUI_API bool          DataTypeIsZero(RblxUIDataType data_type, const void* p_data);

    // InputText
    RBLXUI_API bool          InputTextEx(const char* label, const char* hint, char* buf, int buf_size, const RblxVec2& size_arg, RblxUIInputTextFlags flags, RblxUIInputTextCallback callback = NULL, void* user_data = NULL);
    RBLXUI_API void          InputTextDeactivateHook(RblxUIID id);
    RBLXUI_API bool          TempInputText(const RblxRect& bb, RblxUIID id, const char* label, char* buf, size_t buf_size, RblxUIInputTextFlags flags = 0, RblxUIInputTextCallback callback = NULL, void* user_data = NULL);
    RBLXUI_API bool          TempInputScalar(const RblxRect& bb, RblxUIID id, const char* label, RblxUIDataType data_type, void* p_data, const char* format, const void* p_clamp_min = NULL, const void* p_clamp_max = NULL);
    inline bool             TempInputIsActive(RblxUIID id)       { RblxUIContext& g = *GRblxUI; return g.ActiveId == id && g.TempInputId == id; }
    inline RblxUIInputTextState* GetInputTextState(RblxUIID id)   { RblxUIContext& g = *GRblxUI; return (id != 0 && g.InputTextState.ID == id) ? &g.InputTextState : NULL; } // Get input text state if active
    RBLXUI_API void          SetNextItemRefVal(RblxUIDataType data_type, void* p_data);
    inline bool             IsItemActiveAsInputText() { RblxUIContext& g = *GRblxUI; return g.ActiveId != 0 && g.ActiveId == g.LastItemData.ID && g.InputTextState.ID == g.LastItemData.ID; } // This may be useful to apply workaround that a based on distinguish whenever an item is active as a text input field.

    // Color
    RBLXUI_API void          ColorTooltip(const char* text, const float* col, RblxUIColorEditFlags flags);
    RBLXUI_API void          ColorEditOptionsPopup(const float* col, RblxUIColorEditFlags flags);
    RBLXUI_API void          ColorPickerOptionsPopup(const float* ref_col, RblxUIColorEditFlags flags);
    inline void             SetNextItemColorMarker(RblxU32 col) { RblxUIContext& g = *GRblxUI; g.NextItemData.HasFlags |= RblxUINextItemDataFlags_HasColorMarker; g.NextItemData.ColorMarker = col; }

    // Plot
    RBLXUI_API int           PlotEx(RblxUIPlotType plot_type, const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, const RblxVec2& size_arg);

    // Shade functions (write over already created vertices)
    RBLXUI_API void          ShadeVertsLinearColorGradientKeepAlpha(RblxDrawList* draw_list, int vert_start_idx, int vert_end_idx, RblxVec2 gradient_p0, RblxVec2 gradient_p1, RblxU32 col0, RblxU32 col1);
    RBLXUI_API void          ShadeVertsLinearUV(RblxDrawList* draw_list, int vert_start_idx, int vert_end_idx, const RblxVec2& a, const RblxVec2& b, const RblxVec2& uv_a, const RblxVec2& uv_b, bool clamp);
    RBLXUI_API void          ShadeVertsTransformPos(RblxDrawList* draw_list, int vert_start_idx, int vert_end_idx, const RblxVec2& pivot_in, float cos_a, float sin_a, const RblxVec2& pivot_out);

    // Garbage collection
    RBLXUI_API void          GcCompactTransientMiscBuffers();
    RBLXUI_API void          GcCompactTransientWindowBuffers(RblxUIWindow* window);
    RBLXUI_API void          GcAwakeTransientWindowBuffers(RblxUIWindow* window);

    // Error handling, State Recovery
    RBLXUI_API bool          ErrorLog(const char* msg);
    RBLXUI_API void          ErrorRecoveryStoreState(RblxUIErrorRecoveryState* state_out);
    RBLXUI_API void          ErrorRecoveryTryToRecoverState(const RblxUIErrorRecoveryState* state_in);
    RBLXUI_API void          ErrorRecoveryTryToRecoverWindowState(const RblxUIErrorRecoveryState* state_in);
    RBLXUI_API void          ErrorCheckUsingSetCursorPosToExtendParentBoundaries();
    RBLXUI_API void          ErrorCheckEndFrameFinalizeErrorTooltip();
    RBLXUI_API bool          BeginErrorTooltip();
    RBLXUI_API void          EndErrorTooltip();

    // Demo Doc Marker for e.g. rblxui_explorer
    RBLXUI_API void          DemoMarker(const char* file, int line, const char* section);

    // Debug Tools
    RBLXUI_API void          DebugAllocHook(RblxUIDebugAllocInfo* info, int frame_count, void* ptr, size_t size); // size >= 0 : alloc, size = -1 : free
    RBLXUI_API void          DebugDrawCursorPos(RblxU32 col = RBLXUI_COL32(255, 0, 0, 255));
    RBLXUI_API void          DebugDrawLineExtents(RblxU32 col = RBLXUI_COL32(255, 0, 0, 255));
    RBLXUI_API void          DebugDrawItemRect(RblxU32 col = RBLXUI_COL32(255, 0, 0, 255));
    RBLXUI_API void          DebugTextUnformattedWithLocateItem(const char* line_begin, const char* line_end);
    RBLXUI_API void          DebugLocateItem(RblxUIID target_id);                     // Call sparingly: only 1 at the same time!
    RBLXUI_API void          DebugLocateItemOnHover(RblxUIID target_id);              // Only call on reaction to a mouse Hover: because only 1 at the same time!
    RBLXUI_API void          DebugLocateItemResolveWithLastItem();
    RBLXUI_API void          DebugBreakClearData();
    RBLXUI_API bool          DebugBreakButton(const char* label, const char* description_of_location);
    RBLXUI_API void          DebugBreakButtonTooltip(bool keyboard_only, const char* description_of_location);
    RBLXUI_API void          ShowFontAtlas(RblxFontAtlas* atlas);
    RBLXUI_API RblxU64         DebugTextureIDToU64(RblxTextureID tex_id);
    RBLXUI_API void          DebugHookIdInfo(RblxUIID id, RblxUIDataType data_type, const void* data_id, const void* data_id_end);
    RBLXUI_API void          DebugNodeColumns(RblxUIOldColumns* columns);
    RBLXUI_API void          DebugNodeDockNode(RblxUIDockNode* node, const char* label);
    RBLXUI_API void          DebugNodeDrawList(RblxUIWindow* window, RblxUIViewportP* viewport, const RblxDrawList* draw_list, const char* label);
    RBLXUI_API void          DebugNodeDrawCmdShowMeshAndBoundingBox(RblxDrawList* out_draw_list, const RblxDrawList* draw_list, const RblxDrawCmd* draw_cmd, bool show_mesh, bool show_aabb);
    RBLXUI_API void          DebugNodeFont(RblxFont* font);
    RBLXUI_API void          DebugNodeFontGlyphsForSrcMask(RblxFont* font, RblxFontBaked* baked, int src_mask);
    RBLXUI_API void          DebugNodeFontGlyph(RblxFont* font, const RblxFontGlyph* glyph);
    RBLXUI_API void          DebugNodeTexture(RblxTextureData* tex, int int_id, const RblxFontAtlasRect* highlight_rect = NULL); // ID used to facilitate persisting the "current" texture.
    RBLXUI_API void          DebugNodeStorage(RblxUIStorage* storage, const char* label);
    RBLXUI_API void          DebugNodeTabBar(RblxUITabBar* tab_bar, const char* label);
    RBLXUI_API void          DebugNodeTable(RblxUITable* table);
    RBLXUI_API void          DebugNodeTableSettings(RblxUITableSettings* settings);
    RBLXUI_API void          DebugNodeInputTextState(RblxUIInputTextState* state);
    RBLXUI_API void          DebugNodeTypingSelectState(RblxUITypingSelectState* state);
    RBLXUI_API void          DebugNodeMultiSelectState(RblxUIMultiSelectState* state);
    RBLXUI_API void          DebugNodeWindow(RblxUIWindow* window, const char* label);
    RBLXUI_API void          DebugNodeWindowSettings(RblxUIWindowSettings* settings);
    RBLXUI_API void          DebugNodeWindowsList(RblxVector<RblxUIWindow*>* windows, const char* label);
    RBLXUI_API void          DebugNodeWindowsListByBeginStackParent(RblxUIWindow** windows, int windows_size, RblxUIWindow* parent_in_begin_stack);
    RBLXUI_API void          DebugNodeViewport(RblxUIViewportP* viewport);
    RBLXUI_API void          DebugNodePlatformMonitor(RblxUIPlatformMonitor* monitor, const char* label, int idx);
    RBLXUI_API void          DebugRenderKeyboardPreview(RblxDrawList* draw_list);
    RBLXUI_API void          DebugRenderViewportThumbnail(RblxDrawList* draw_list, RblxUIViewportP* viewport, const RblxRect& bb);

    // Obsolete functions
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    //inline void   SetItemUsingMouseWheel()                                            { SetItemKeyOwner(RblxUIKey_MouseWheelY); }      // Changed in 1.89
    //inline bool   TreeNodeBehaviorIsOpen(RblxUIID id, RblxUITreeNodeFlags flags = 0)    { return TreeNodeUpdateNextOpen(id, flags); }   // Renamed in 1.89
    //inline bool   IsKeyPressedMap(RblxUIKey key, bool repeat = true)                   { RBLXUI_ASSERT(IsNamedKey(key)); return IsKeyPressed(key, repeat); } // Removed in 1.87: Mapping from named key is always identity!

    // Refactored focus/nav/tabbing system in 1.82 and 1.84. If you have old/custom copy-and-pasted widgets which used FocusableItemRegister():
    //  (Old) RBLXUI_VERSION_NUM  < 18209: using 'ItemAdd(....)'                              and 'bool tab_focused = FocusableItemRegister(...)'
    //  (Old) RBLXUI_VERSION_NUM >= 18209: using 'ItemAdd(..., RblxUIItemAddFlags_Focusable)'  and 'bool tab_focused = (g.LastItemData.StatusFlags & RblxUIItemStatusFlags_Focused) != 0'
    //  (New) RBLXUI_VERSION_NUM >= 18413: using 'ItemAdd(..., RblxUIItemFlags_Inputable)'     and 'bool tab_focused = (g.NavActivateId == id && (g.NavActivateFlags & RblxUIActivateFlags_PreferInput))'
    //inline bool   FocusableItemRegister(RblxUIWindow* window, RblxUIID id)              // -> pass RblxUIItemAddFlags_Inputable flag to ItemAdd()
    //inline void   FocusableItemUnregister(RblxUIWindow* window)                        // -> unnecessary: TempInputText() uses RblxUIInputTextFlags_MergedItem
#endif

} // namespace RblxUI


//-----------------------------------------------------------------------------
// [SECTION] RblxFontLoader
//-----------------------------------------------------------------------------

// Hooks and storage for a given font backend.
// This structure is likely to evolve as we add support for incremental atlas updates.
// Conceptually this could be public, but API is still going to be evolve.
struct RblxFontLoader
{
    const char*     Name;
    bool            (*LoaderInit)(RblxFontAtlas* atlas);
    void            (*LoaderShutdown)(RblxFontAtlas* atlas);
    bool            (*FontSrcInit)(RblxFontAtlas* atlas, RblxFontConfig* src);
    void            (*FontSrcDestroy)(RblxFontAtlas* atlas, RblxFontConfig* src);
    bool            (*FontSrcContainsGlyph)(RblxFontAtlas* atlas, RblxFontConfig* src, RblxWchar codepoint);
    bool            (*FontBakedInit)(RblxFontAtlas* atlas, RblxFontConfig* src, RblxFontBaked* baked, void* loader_data_for_baked_src);
    void            (*FontBakedDestroy)(RblxFontAtlas* atlas, RblxFontConfig* src, RblxFontBaked* baked, void* loader_data_for_baked_src);
    bool            (*FontBakedLoadGlyph)(RblxFontAtlas* atlas, RblxFontConfig* src, RblxFontBaked* baked, void* loader_data_for_baked_src, RblxWchar codepoint, RblxFontGlyph* out_glyph, float* out_advance_x);

    // Size of backend data, Per Baked * Per Source. Buffers are managed by core to avoid excessive allocations.
    // FIXME: At this point the two other types of buffers may be managed by core to be consistent?
    size_t          FontBakedSrcLoaderDataSize;

    RblxFontLoader()  { memset((void*)this, 0, sizeof(*this)); }
};

#ifdef RBLXUI_ENABLE_STB_TRUETYPE
RBLXUI_API const RblxFontLoader* RblxFontAtlasGetFontLoaderForStbTruetype();
#endif
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
typedef RblxFontLoader RblxFontBuilderIO; // [renamed/changed in 1.92.0] The types are not actually compatible but we provide this as a compile-time error report helper.
#endif

//-----------------------------------------------------------------------------
// [SECTION] RblxFontAtlas internal API
//-----------------------------------------------------------------------------

#define RBLXUI_FONT_SIZE_MAX                                     (512.0f)
#define RBLXUI_FONT_SIZE_THRESHOLD_FOR_LOADADVANCEXONLYMODE      (128.0f)

// Helpers: RblxTextureRef ==/!= operators provided as convenience
// (note that _TexID and _TexData are never set simultaneously)
inline bool operator==(const RblxTextureRef& lhs, const RblxTextureRef& rhs)    { return lhs._TexID == rhs._TexID && lhs._TexData == rhs._TexData; }
inline bool operator!=(const RblxTextureRef& lhs, const RblxTextureRef& rhs)    { return lhs._TexID != rhs._TexID || lhs._TexData != rhs._TexData; }

// Refer to RblxFontAtlasPackGetRect() to better understand how this works.
#define RblxFontAtlasRectId_IndexMask_        (0x0007FFFF)    // 20-bits signed: index to access builder->RectsIndex[].
#define RblxFontAtlasRectId_GenerationMask_   (0x3FF00000)    // 10-bits: entry generation, so each ID is unique and get can safely detected old identifiers.
#define RblxFontAtlasRectId_GenerationShift_  (20)
inline int               RblxFontAtlasRectId_GetIndex(RblxFontAtlasRectId id)       { return (id & RblxFontAtlasRectId_IndexMask_); }
inline unsigned int      RblxFontAtlasRectId_GetGeneration(RblxFontAtlasRectId id)  { return (unsigned int)(id & RblxFontAtlasRectId_GenerationMask_) >> RblxFontAtlasRectId_GenerationShift_; }
inline RblxFontAtlasRectId RblxFontAtlasRectId_Make(int index_idx, int gen_idx)     { RBLXUI_ASSERT(index_idx >= 0 && index_idx <= RblxFontAtlasRectId_IndexMask_ && gen_idx <= (RblxFontAtlasRectId_GenerationMask_ >> RblxFontAtlasRectId_GenerationShift_)); return (RblxFontAtlasRectId)(index_idx | (gen_idx << RblxFontAtlasRectId_GenerationShift_)); }

// Packed rectangle lookup entry (we need an indirection to allow removing/reordering rectangles)
// User are returned RblxFontAtlasRectId values which are meant to be persistent.
// We handle this with an indirection. While Rects[] may be in theory shuffled, compacted etc., RectsIndex[] cannot it is keyed by RblxFontAtlasRectId.
// RectsIndex[] is used both as an index into Rects[] and an index into itself. This is basically a free-list. See RblxFontAtlasBuildAllocRectIndexEntry() code.
// Having this also makes it easier to e.g. sort rectangles during repack.
struct RblxFontAtlasRectEntry
{
    int                 TargetIndex : 20;   // When Used: RblxFontAtlasRectId -> into Rects[]. When unused: index to next unused RectsIndex[] slot to consume free-list.
    unsigned int        Generation : 10;    // Increased each time the entry is reused for a new rectangle.
    unsigned int        IsUsed : 1;
};

// Data available to potential texture post-processing functions
struct RblxFontAtlasPostProcessData
{
    RblxFontAtlas*        FontAtlas;
    RblxFont*             Font;
    RblxFontConfig*       FontSrc;
    RblxFontBaked*        FontBaked;
    RblxFontGlyph*        Glyph;

    // Pixel data
    void*               Pixels;
    RblxTextureFormat     Format;
    int                 Pitch;
    int                 Width;
    int                 Height;
};

// We avoid dragging rblxuistb_rectpack.h into public header (partly because binding generators are having issues with it)
#ifdef RBLXUI_STB_NAMESPACE
namespace RBLXUI_STB_NAMESPACE { struct rblxstbrp_node; }
typedef RBLXUI_STB_NAMESPACE::rblxstbrp_node rblxstbrp_node_im;
#else
struct rblxstbrp_node;
typedef rblxstbrp_node rblxstbrp_node_im;
#endif
struct rblxstbrp_context_opaque { char data[80]; };

// Internal storage for incrementally packing and building a RblxFontAtlas
struct RblxFontAtlasBuilder
{
    rblxstbrp_context_opaque        PackContext;            // Actually 'rblxstbrp_context' but we don't want to define this in the header file.
    RblxVector<rblxstbrp_node_im>     PackNodes;
    RblxVector<RblxTextureRect>     Rects;
    RblxVector<RblxFontAtlasRectEntry> RectsIndex;          // RblxFontAtlasRectId -> index into Rects[]
    RblxVector<unsigned char>     TempBuffer;             // Misc scratch buffer
    int                         RectsIndexFreeListStart;// First unused entry
    int                         RectsPackedCount;       // Number of packed rectangles.
    int                         RectsPackedSurface;     // Number of packed pixels. Used when compacting to heuristically find the ideal texture size.
    int                         RectsDiscardedCount;
    int                         RectsDiscardedSurface;
    int                         FrameCount;             // Current frame count
    RblxVec2i                     MaxRectSize;            // Largest rectangle to pack (de-facto used as a "minimum texture size")
    RblxVec2i                     MaxRectBounds;          // Bottom-right most used pixels
    bool                        LockDisableResize;      // Disable resizing texture
    bool                        PreloadedAllGlyphsRanges; // Set when missing RblxUIBackendFlags_RendererHasTextures features forces atlas to preload everything.

    // Cache of all RblxFontBaked
    RblxStableVector<RblxFontBaked,32> BakedPool;
    RblxUIStorage                BakedMap;               // BakedId --> RblxFontBaked*
    int                         BakedDiscardedCount;

    // Custom rectangle identifiers
    RblxFontAtlasRectId           PackIdMouseCursors;     // White pixel + mouse cursors. Also happen to be fallback in case of packing failure.
    RblxFontAtlasRectId           PackIdLinesTexData;

    RblxFontAtlasBuilder()        { memset((void*)this, 0, sizeof(*this)); FrameCount = -1; RectsIndexFreeListStart = -1; PackIdMouseCursors = PackIdLinesTexData = -1; }
};

RBLXUI_API void              RblxFontAtlasBuildInit(RblxFontAtlas* atlas);
RBLXUI_API void              RblxFontAtlasBuildDestroy(RblxFontAtlas* atlas);
RBLXUI_API void              RblxFontAtlasBuildMain(RblxFontAtlas* atlas);
RBLXUI_API void              RblxFontAtlasBuildSetupFontLoader(RblxFontAtlas* atlas, const RblxFontLoader* font_loader);
RBLXUI_API void              RblxFontAtlasBuildNotifySetFont(RblxFontAtlas* atlas, RblxFont* old_font, RblxFont* new_font);
RBLXUI_API void              RblxFontAtlasBuildUpdatePointers(RblxFontAtlas* atlas);
RBLXUI_API void              RblxFontAtlasBuildRenderBitmapFromString(RblxFontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char);
RBLXUI_API void              RblxFontAtlasBuildClear(RblxFontAtlas* atlas); // Clear output and custom rects

RBLXUI_API RblxTextureData*    RblxFontAtlasTextureAdd(RblxFontAtlas* atlas, int w, int h);
RBLXUI_API void              RblxFontAtlasTextureMakeSpace(RblxFontAtlas* atlas);
RBLXUI_API void              RblxFontAtlasTextureRepack(RblxFontAtlas* atlas, int w, int h);
RBLXUI_API void              RblxFontAtlasTextureGrow(RblxFontAtlas* atlas, int old_w = -1, int old_h = -1);
RBLXUI_API void              RblxFontAtlasTextureCompact(RblxFontAtlas* atlas);
RBLXUI_API RblxVec2i           RblxFontAtlasTextureGetSizeEstimate(RblxFontAtlas* atlas);

RBLXUI_API void              RblxFontAtlasBuildSetupFontSpecialGlyphs(RblxFontAtlas* atlas, RblxFont* font, RblxFontConfig* src);
RBLXUI_API void              RblxFontAtlasBuildLegacyPreloadAllGlyphRanges(RblxFontAtlas* atlas); // Legacy
RBLXUI_API void              RblxFontAtlasBuildGetOversampleFactors(RblxFontConfig* src, RblxFontBaked* baked, int* out_oversample_h, int* out_oversample_v);
RBLXUI_API void              RblxFontAtlasBuildDiscardBakes(RblxFontAtlas* atlas, int unused_frames);

RBLXUI_API bool              RblxFontAtlasFontSourceInit(RblxFontAtlas* atlas, RblxFontConfig* src);
RBLXUI_API void              RblxFontAtlasFontSourceAddToFont(RblxFontAtlas* atlas, RblxFont* font, RblxFontConfig* src);
RBLXUI_API void              RblxFontAtlasFontDestroySourceData(RblxFontAtlas* atlas, RblxFontConfig* src);
RBLXUI_API bool              RblxFontAtlasFontInitOutput(RblxFontAtlas* atlas, RblxFont* font); // Using FontDestroyOutput/FontInitOutput sequence useful notably if font loader params have changed
RBLXUI_API void              RblxFontAtlasFontDestroyOutput(RblxFontAtlas* atlas, RblxFont* font);
RBLXUI_API void              RblxFontAtlasFontRebuildOutput(RblxFontAtlas* atlas, RblxFont* font);
RBLXUI_API void              RblxFontAtlasFontDiscardBakes(RblxFontAtlas* atlas, RblxFont* font, int unused_frames);

RBLXUI_API RblxUIID           RblxFontAtlasBakedGetId(RblxUIID font_id, float baked_size, float rasterizer_density);
RBLXUI_API RblxFontBaked*      RblxFontAtlasBakedGetOrAdd(RblxFontAtlas* atlas, RblxFont* font, float font_size, float font_rasterizer_density);
RBLXUI_API RblxFontBaked*      RblxFontAtlasBakedGetClosestMatch(RblxFontAtlas* atlas, RblxFont* font, float font_size, float font_rasterizer_density);
RBLXUI_API RblxFontBaked*      RblxFontAtlasBakedAdd(RblxFontAtlas* atlas, RblxFont* font, float font_size, float font_rasterizer_density, RblxUIID baked_id);
RBLXUI_API void              RblxFontAtlasBakedDiscard(RblxFontAtlas* atlas, RblxFont* font, RblxFontBaked* baked);
RBLXUI_API RblxFontGlyph*      RblxFontAtlasBakedAddFontGlyph(RblxFontAtlas* atlas, RblxFontBaked* baked, RblxFontConfig* src, const RblxFontGlyph* in_glyph);
RBLXUI_API void              RblxFontAtlasBakedAddFontGlyphAdvancedX(RblxFontAtlas* atlas, RblxFontBaked* baked, RblxFontConfig* src, RblxWchar codepoint, float advance_x);
RBLXUI_API void              RblxFontAtlasBakedDiscardFontGlyph(RblxFontAtlas* atlas, RblxFont* font, RblxFontBaked* baked, RblxFontGlyph* glyph);
RBLXUI_API void              RblxFontAtlasBakedSetFontGlyphBitmap(RblxFontAtlas* atlas, RblxFontBaked* baked, RblxFontConfig* src, RblxFontGlyph* glyph, RblxTextureRect* r, const unsigned char* src_pixels, RblxTextureFormat src_fmt, int src_pitch);

RBLXUI_API void              RblxFontAtlasPackInit(RblxFontAtlas* atlas);
RBLXUI_API RblxFontAtlasRectId RblxFontAtlasPackAddRect(RblxFontAtlas* atlas, int w, int h, RblxFontAtlasRectEntry* overwrite_entry = NULL);
RBLXUI_API RblxTextureRect*    RblxFontAtlasPackGetRect(RblxFontAtlas* atlas, RblxFontAtlasRectId id);
RBLXUI_API RblxTextureRect*    RblxFontAtlasPackGetRectSafe(RblxFontAtlas* atlas, RblxFontAtlasRectId id);
RBLXUI_API void              RblxFontAtlasPackDiscardRect(RblxFontAtlas* atlas, RblxFontAtlasRectId id);

RBLXUI_API void              RblxFontAtlasUpdateNewFrame(RblxFontAtlas* atlas, int frame_count, bool renderer_has_textures);
RBLXUI_API void              RblxFontAtlasAddDrawListSharedData(RblxFontAtlas* atlas, RblxDrawListSharedData* data);
RBLXUI_API void              RblxFontAtlasRemoveDrawListSharedData(RblxFontAtlas* atlas, RblxDrawListSharedData* data);
RBLXUI_API void              RblxFontAtlasUpdateDrawListsTextures(RblxFontAtlas* atlas, RblxTextureRef old_tex, RblxTextureRef new_tex);
RBLXUI_API void              RblxFontAtlasUpdateDrawListsSharedData(RblxFontAtlas* atlas);

RBLXUI_API void              RblxFontAtlasTextureBlockConvert(const unsigned char* src_pixels, RblxTextureFormat src_fmt, int src_pitch, unsigned char* dst_pixels, RblxTextureFormat dst_fmt, int dst_pitch, int w, int h);
RBLXUI_API void              RblxFontAtlasTextureBlockPostProcess(RblxFontAtlasPostProcessData* data);
RBLXUI_API void              RblxFontAtlasTextureBlockPostProcessMultiply(RblxFontAtlasPostProcessData* data, float multiply_factor);
RBLXUI_API void              RblxFontAtlasTextureBlockFill(RblxTextureData* dst_tex, int dst_x, int dst_y, int w, int h, RblxU32 col);
RBLXUI_API void              RblxFontAtlasTextureBlockCopy(RblxTextureData* src_tex, int src_x, int src_y, RblxTextureData* dst_tex, int dst_x, int dst_y, int w, int h);
RBLXUI_API void              RblxFontAtlasTextureBlockQueueUpload(RblxFontAtlas* atlas, RblxTextureData* tex, int x, int y, int w, int h);

RBLXUI_API bool              RblxTextureDataUpdateNewFrame(RblxTextureData* tex);
RBLXUI_API void              RblxTextureDataQueueUpload(RblxTextureData* tex, int x, int y, int w, int h);
RBLXUI_API int               RblxTextureDataGetFormatBytesPerPixel(RblxTextureFormat format);
RBLXUI_API const char*       RblxTextureDataGetStatusName(RblxTextureStatus status);
RBLXUI_API const char*       RblxTextureDataGetFormatName(RblxTextureFormat format);

#ifndef RBLXUI_DISABLE_DEBUG_TOOLS
RBLXUI_API void              RblxFontAtlasDebugLogTextureRequests(RblxFontAtlas* atlas);
#endif

RBLXUI_API bool      RblxFontAtlasGetMouseCursorTexData(RblxFontAtlas* atlas, RblxUIMouseCursor cursor_type, RblxVec2* out_offset, RblxVec2* out_size, RblxVec2 out_uv_border[2], RblxVec2 out_uv_fill[2]);

//-----------------------------------------------------------------------------
// [SECTION] Test Engine specific hooks (rblxui_test_engine)
//-----------------------------------------------------------------------------

#ifdef RBLXUI_ENABLE_TEST_ENGINE
extern void         RblxUITestEngineHook_ItemAdd(RblxUIContext* ctx, RblxUIID id, const RblxRect& bb, const RblxUILastItemData* item_data);           // item_data may be NULL
extern void         RblxUITestEngineHook_ItemInfo(RblxUIContext* ctx, RblxUIID id, const char* label, RblxUIItemStatusFlags flags);
extern void         RblxUITestEngineHook_Log(RblxUIContext* ctx, const char* fmt, ...);
extern const char*  RblxUITestEngine_FindItemDebugLabel(RblxUIContext* ctx, RblxUIID id);

// In RBLXUI_VERSION_NUM >= 18934: changed RBLXUI_TEST_ENGINE_ITEM_ADD(bb,id) to RBLXUI_TEST_ENGINE_ITEM_ADD(id,bb,item_data);
#define RBLXUI_TEST_ENGINE_ITEM_ADD(_ID,_BB,_ITEM_DATA)      if (g.TestEngineHookItems) RblxUITestEngineHook_ItemAdd(&g, _ID, _BB, _ITEM_DATA)    // Register item bounding box
#define RBLXUI_TEST_ENGINE_ITEM_INFO(_ID,_LABEL,_FLAGS)      if (g.TestEngineHookItems) RblxUITestEngineHook_ItemInfo(&g, _ID, _LABEL, _FLAGS)    // Register item label and status flags (optional)
#define RBLXUI_TEST_ENGINE_LOG(_FMT,...)                     RblxUITestEngineHook_Log(&g, _FMT, __VA_ARGS__)                                      // Custom log entry from user land into test log
#else
#define RBLXUI_TEST_ENGINE_ITEM_ADD(_ID,_BB,_ITEM_DATA)      ((void)0)
#define RBLXUI_TEST_ENGINE_ITEM_INFO(_ID,_LABEL,_FLAGS)      ((void)g)
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

#endif // #ifndef RBLXUI_DISABLE
