// dear linearui, v1.92.9 WIP
// (internal structures/api)

// You may use this file to debug, understand or extend Dear LinearUI features but we don't provide any guarantee of forward compatibility.

/*

Index of this file:

// [SECTION] Header mess
// [SECTION] Forward declarations
// [SECTION] Context pointer
// [SECTION] LINEARSTB libraries includes
// [SECTION] Macros
// [SECTION] Generic helpers
// [SECTION] LinearDrawList support
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
// [SECTION] LinearUIContext (main linearui context)
// [SECTION] LinearUIWindowTempData, LinearUIWindow
// [SECTION] Tab bar, Tab item support
// [SECTION] Table support
// [SECTION] LinearUI internal API
// [SECTION] LinearFontLoader
// [SECTION] LinearFontAtlas internal API
// [SECTION] Test Engine specific hooks (linearui_test_engine)

*/

#pragma once
#ifndef LINEARUI_DISABLE

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

#ifndef LINEARUI_VERSION
#include "linearui.h"
#endif

#include <stdio.h>      // FILE*, sscanf
#include <stdlib.h>     // NULL, malloc, free, qsort, atoi, atof
#include <math.h>       // sqrtf, fabsf, fmodf, powf, floorf, ceilf, cosf, sinf
#include <limits.h>     // INT_MIN, INT_MAX

// Enable SSE intrinsics if available
#if (defined __SSE__ || defined __x86_64__ || defined _M_X64 || (defined(_M_IX86_FP) && (_M_IX86_FP >= 1))) && !defined(LINEARUI_DISABLE_SSE) && !defined(_M_ARM64) && !defined(_M_ARM64EC)
#define LINEARUI_ENABLE_SSE
#include <immintrin.h>
#if (defined __AVX__ || defined __SSE4_2__)
#define LINEARUI_ENABLE_SSE4_2
#include <nmmintrin.h>
#endif
#endif
// Emscripten has partial SSE 4.2 support where _mm_crc32_u32 is not available. See https://emscripten.org/docs/porting/simd.html#id11 and #8213
#if defined(LINEARUI_ENABLE_SSE4_2) && !defined(LINEARUI_USE_LEGACY_CRC32_ADLER) && !defined(__EMSCRIPTEN__)
#define LINEARUI_ENABLE_SSE4_2_CRC
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4251)     // class 'xxx' needs to have dll-interface to be used by clients of struct 'xxx' // when LINEARUI_API is set to__declspec(dllexport)
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
#pragma clang diagnostic ignored "-Wfloat-equal"                    // warning: comparing floating point with == or != is unsafe // storing and comparing against same constants ok, for LinearFloor()
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

// In 1.89.4, we moved the implementation of "courtesy maths operators" from linearui_internal.h in linearui.h
// As they are frequently requested, we do not want to encourage to many people using linearui_internal.h
#if defined(LINEARUI_DEFINE_MATH_OPERATORS) && !defined(LINEARUI_DEFINE_MATH_OPERATORS_IMPLEMENTED)
#error Please '#define LINEARUI_DEFINE_MATH_OPERATORS' _BEFORE_ including linearui.h!
#endif

// Legacy defines
#ifdef LINEARUI_DISABLE_FORMAT_STRING_FUNCTIONS            // Renamed in 1.74
#error Use LINEARUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
#endif
#ifdef LINEARUI_DISABLE_MATH_FUNCTIONS                     // Renamed in 1.74
#error Use LINEARUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#endif

// Enable linearstb_truetype by default unless FreeType is enabled.
// You can compile with both by defining both LINEARUI_ENABLE_FREETYPE and LINEARUI_ENABLE_STB_TRUETYPE together.
#ifndef LINEARUI_ENABLE_FREETYPE
#define LINEARUI_ENABLE_STB_TRUETYPE
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations
//-----------------------------------------------------------------------------

// Utilities
// (other types which are not forwarded declared are: LinearBitArray<>, LinearSpan<>, LinearSpanAllocator<>, LinearStableVector<>, LinearPool<>, LinearChunkStream<>)
struct LinearBitVector;                 // Store 1-bit per value
struct LinearRect;                      // An axis-aligned rectangle (2 points)
struct LinearUITextIndex;              // Maintain a line index for a text buffer.

// LinearDrawList/LinearFontAtlas
struct LinearDrawDataBuilder;           // Helper to build a LinearDrawData instance
struct LinearDrawListSharedData;        // Data shared between all LinearDrawList instances
struct LinearFontAtlasBuilder;          // Internal storage for incrementally packing and building a LinearFontAtlas
struct LinearFontAtlasPostProcessData;  // Data available to potential texture post-processing functions
struct LinearFontAtlasRectEntry;        // Packed rectangle lookup entry

// LinearUI
struct LinearUIBoxSelectState;         // Box-selection state (currently used by multi-selection, could potentially be used by others)
struct LinearUIColorMod;               // Stacked color modifier, backup of modified data so we can restore it
struct LinearUIContext;                // Main Dear LinearUI context
struct LinearUIContextHook;            // Hook for extensions like LinearUITestEngine
struct LinearUIDataTypeInfo;           // Type information associated to a LinearUIDataType enum
struct LinearUIDeactivatedItemData;    // Data for IsItemDeactivated()/IsItemDeactivatedAfterEdit() function.
struct LinearUIDockContext;            // Docking system context
struct LinearUIDockRequest;            // Docking system dock/undock queued request
struct LinearUIDockNode;               // Docking system node (hold a list of Windows OR two child dock nodes)
struct LinearUIDockNodeSettings;       // Storage for a dock node in .ini file (we preserve those even if the associated dock node isn't active during the session)
struct LinearUIErrorRecoveryState;     // Storage of stack sizes for error handling and recovery
struct LinearUIGroupData;              // Stacked storage data for BeginGroup()/EndGroup()
struct LinearUIInputTextState;         // Internal state of the currently focused/edited text input box
struct LinearUIInputTextDeactivateData;// Short term storage to backup text of a deactivating InputText() while another is stealing active id
struct LinearUILastItemData;           // Status storage for last submitted items
struct LinearUILocEntry;               // A localization entry.
struct LinearUIMenuColumns;            // Simple column measurement, currently used for MenuItem() only
struct LinearUIMultiSelectState;       // Multi-selection persistent state (for focused selection).
struct LinearUIMultiSelectTempData;    // Multi-selection temporary state (while traversing).
struct LinearUINavItemData;            // Result of a keyboard/gamepad directional navigation move query result
struct LinearUIMetricsConfig;          // Storage for ShowMetricsWindow() and DebugNodeXXX() functions
struct LinearUINextWindowData;         // Storage for SetNextWindow** functions
struct LinearUINextItemData;           // Storage for SetNextItem** functions
struct LinearUIOldColumnData;          // Storage data for a single column for legacy Columns() api
struct LinearUIOldColumns;             // Storage data for a columns set for legacy Columns() api
struct LinearUIPopupData;              // Storage for current popup stack
struct LinearUISettingsHandler;        // Storage for one type registered in the .ini file
struct LinearUIStyleMod;               // Stacked style modifier, backup of modified data so we can restore it
struct LinearUIStyleVarInfo;           // Style variable information (e.g. to access style variables from an enum)
struct LinearUITabBar;                 // Storage for a tab bar
struct LinearUITabItem;                // Storage for a tab item (within a tab bar)
struct LinearUITable;                  // Storage for a table
struct LinearUITableHeaderData;        // Storage for TableAngledHeadersRow()
struct LinearUITableColumn;            // Storage for one column of a table
struct LinearUITableInstanceData;      // Storage for one instance of a same table
struct LinearUITableTempData;          // Temporary storage for one table (one per table in the stack), shared between tables.
struct LinearUITableSettings;          // Storage for a table .ini settings
struct LinearUITableColumnsSettings;   // Storage for a column .ini settings
struct LinearUITreeNodeStackData;      // Temporary storage for TreeNode().
struct LinearUITypingSelectState;      // Storage for GetTypingSelectRequest()
struct LinearUITypingSelectRequest;    // Storage for GetTypingSelectRequest() (aimed to be public)
struct LinearUIWindow;                 // Storage for one window
struct LinearUIWindowDockStyle;        // Storage for window-style data which needs to be stored for docking purpose
struct LinearUIWindowTempData;         // Temporary storage for one window (that's the data which in theory we could ditch at the end of the frame, in practice we currently keep it for each window)
struct LinearUIWindowSettings;         // Storage for a window .ini settings (we keep one of those even if the actual window wasn't instanced during this session)

// Enumerations
// Use your programming IDE "Go to definition" facility on the names of the center columns to find the actual flags/enum lists.
enum LinearUILocKey : int;                 // -> enum LinearUILocKey              // Enum: a localization entry for translation.
typedef int LinearUIDataAuthority;         // -> enum LinearUIDataAuthority_      // Enum: for storing the source authority (dock node vs window) of a field
typedef int LinearUILayoutType;            // -> enum LinearUILayoutType_         // Enum: Horizontal or vertical

// Flags
typedef int LinearDrawTextFlags;            // -> enum LinearDrawTextFlags_         // Flags: for LinearTextCalcWordWrapPositionEx()
typedef int LinearUIActivateFlags;         // -> enum LinearUIActivateFlags_      // Flags: for navigation/focus function (will be for ActivateItem() later)
typedef int LinearUIDebugLogFlags;         // -> enum LinearUIDebugLogFlags_      // Flags: for ShowDebugLogWindow(), g.DebugLogFlags
typedef int LinearUIFocusRequestFlags;     // -> enum LinearUIFocusRequestFlags_  // Flags: for FocusWindow()
typedef int LinearUIItemStatusFlags;       // -> enum LinearUIItemStatusFlags_    // Flags: for g.LastItemData.StatusFlags
typedef int LinearUIOldColumnFlags;        // -> enum LinearUIOldColumnFlags_     // Flags: for BeginColumns()
typedef int LinearUILogFlags;              // -> enum LinearUILogFlags_           // Flags: for LogBegin() text capturing function
typedef int LinearUINavRenderCursorFlags;  // -> enum LinearUINavRenderCursorFlags_//Flags: for RenderNavCursor()
typedef int LinearUINavMoveFlags;          // -> enum LinearUINavMoveFlags_       // Flags: for navigation requests
typedef int LinearUINextItemDataFlags;     // -> enum LinearUINextItemDataFlags_  // Flags: for SetNextItemXXX() functions
typedef int LinearUINextWindowDataFlags;   // -> enum LinearUINextWindowDataFlags_// Flags: for SetNextWindowXXX() functions
typedef int LinearUIScrollFlags;           // -> enum LinearUIScrollFlags_        // Flags: for ScrollToItem() and navigation requests
typedef int LinearUISeparatorFlags;        // -> enum LinearUISeparatorFlags_     // Flags: for SeparatorEx()
typedef int LinearUITextFlags;             // -> enum LinearUITextFlags_          // Flags: for TextEx()
typedef int LinearUITooltipFlags;          // -> enum LinearUITooltipFlags_       // Flags: for BeginTooltipEx()
typedef int LinearUITypingSelectFlags;     // -> enum LinearUITypingSelectFlags_  // Flags: for GetTypingSelectRequest()
typedef int LinearUIWindowBgClickFlags;    // -> enum LinearUIWindowBgClickFlags_ // Flags: for overriding behavior of clicking on window background/void.
typedef int LinearUIWindowRefreshFlags;    // -> enum LinearUIWindowRefreshFlags_ // Flags: for SetNextWindowRefreshPolicy()

// Table column indexing
typedef LinearS16 LinearUITableColumnIdx;
typedef LinearU16 LinearUITableDrawChannelIdx;

//-----------------------------------------------------------------------------
// [SECTION] Context pointer
// See implementation of this variable in linearui.cpp for comments and details.
//-----------------------------------------------------------------------------

#ifndef GLinearUI
extern LINEARUI_API LinearUIContext* GLinearUI;  // Current implicit context pointer
#endif

//-----------------------------------------------------------------------------
// [SECTION] Macros
//-----------------------------------------------------------------------------

// Internal Drag and Drop payload types. String starting with '_' are reserved for Dear LinearUI.
#define LINEARUI_PAYLOAD_TYPE_WINDOW       "_IMWINDOW"     // Payload == LinearUIWindow*

// Debug Printing Into TTY
// (since LINEARUI_VERSION_NUM >= 18729: LINEARUI_DEBUG_LOG was reworked into LINEARUI_DEBUG_PRINTF (and removed framecount from it). If you were using a #define LINEARUI_DEBUG_LOG please rename)
#ifndef LINEARUI_DEBUG_PRINTF
#ifndef LINEARUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
#define LINEARUI_DEBUG_PRINTF(_FMT,...)    printf(_FMT, __VA_ARGS__)
#else
#define LINEARUI_DEBUG_PRINTF(_FMT,...)    ((void)0)
#endif
#endif

// Debug Logging for ShowDebugLogWindow(). This is designed for relatively rare events so please don't spam.
#define LINEARUI_DEBUG_LOG_ERROR(...)      do { if (g.DebugLogFlags & LinearUIDebugLogFlags_EventError)       LINEARUI_DEBUG_LOG(__VA_ARGS__); else g.DebugLogSkippedErrors++; } while (0)
#define LINEARUI_DEBUG_LOG_ACTIVEID(...)   do { if (g.DebugLogFlags & LinearUIDebugLogFlags_EventActiveId)    LINEARUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define LINEARUI_DEBUG_LOG_FOCUS(...)      do { if (g.DebugLogFlags & LinearUIDebugLogFlags_EventFocus)       LINEARUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define LINEARUI_DEBUG_LOG_POPUP(...)      do { if (g.DebugLogFlags & LinearUIDebugLogFlags_EventPopup)       LINEARUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define LINEARUI_DEBUG_LOG_NAV(...)        do { if (g.DebugLogFlags & LinearUIDebugLogFlags_EventNav)         LINEARUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define LINEARUI_DEBUG_LOG_SELECTION(...)  do { if (g.DebugLogFlags & LinearUIDebugLogFlags_EventSelection)   LINEARUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define LINEARUI_DEBUG_LOG_CLIPPER(...)    do { if (g.DebugLogFlags & LinearUIDebugLogFlags_EventClipper)     LINEARUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define LINEARUI_DEBUG_LOG_IO(...)         do { if (g.DebugLogFlags & LinearUIDebugLogFlags_EventIO)          LINEARUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define LINEARUI_DEBUG_LOG_FONT(...)       do { LinearUIContext* g2 = GLinearUI; if (g2 && g2->DebugLogFlags & LinearUIDebugLogFlags_EventFont) LINEARUI_DEBUG_LOG(__VA_ARGS__); } while (0) // Called from LinearFontAtlas function which may operate without a context.
#define LINEARUI_DEBUG_LOG_INPUTROUTING(...) do{if (g.DebugLogFlags & LinearUIDebugLogFlags_EventInputRouting)LINEARUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define LINEARUI_DEBUG_LOG_DOCKING(...)    do { if (g.DebugLogFlags & LinearUIDebugLogFlags_EventDocking)     LINEARUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define LINEARUI_DEBUG_LOG_VIEWPORT(...)   do { if (g.DebugLogFlags & LinearUIDebugLogFlags_EventViewport)    LINEARUI_DEBUG_LOG(__VA_ARGS__); } while (0)

// Debug options (also see ones on top of linearui.cpp)
//#define LINEARUI_DEBUG_BOXSELECT

// Static Asserts
#define LINEARUI_STATIC_ASSERT(_COND)         static_assert(_COND, "")

// "Paranoid" Debug Asserts are meant to only be enabled during specific debugging/work, otherwise would slow down the code too much.
// We currently don't have many of those so the effect is currently negligible, but onward intent to add more aggressive ones in the code.
//#define LINEARUI_DEBUG_PARANOID
#ifdef LINEARUI_DEBUG_PARANOID
#define LINEARUI_ASSERT_PARANOID(_EXPR)       LINEARUI_ASSERT(_EXPR)
#else
#define LINEARUI_ASSERT_PARANOID(_EXPR)
#endif

// Misc Macros
#define LINEARUI_PI                           3.14159265358979323846f
#ifdef _WIN32
#define LINEARUI_NEWLINE                      "\r\n"   // Play it nice with Windows users (Update: since 2018-05, Notepad finally appears to support Unix-style carriage returns!)
#else
#define LINEARUI_NEWLINE                      "\n"
#endif
#ifndef LINEARUI_TABSIZE                      // Until we move this to runtime and/or add proper tab support, at least allow users to compile-time override
#define LINEARUI_TABSIZE                      (4)
#endif
#define LINEARUI_MEMALIGN(_OFF,_ALIGN)        (((_OFF) + ((_ALIGN) - 1)) & ~((_ALIGN) - 1))           // Memory align e.g. LINEARUI_ALIGN(0,4)=0, LINEARUI_ALIGN(1,4)=4, LINEARUI_ALIGN(4,4)=4, LINEARUI_ALIGN(5,4)=8
#define LINEARUI_F32_TO_INT8_UNBOUND(_VAL)    ((int)((_VAL) * 255.0f + ((_VAL)>=0 ? 0.5f : -0.5f)))   // Unsaturated, for display purpose
#define LINEARUI_F32_TO_INT8_SAT(_VAL)        ((int)(LinearSaturate(_VAL) * 255.0f + 0.5f))               // Saturated, always output 0..255
#define LINEARUI_TRUNC(_VAL)                  ((float)(int)(_VAL))                                    // Positive values only! LinearTrunc() is not inlined in MSVC debug builds
#define LINEARUI_ROUND(_VAL)                  ((float)(int)((_VAL) + 0.5f))                           // Positive values only! 
//#define LINEARUI_FLOOR LINEARUI_TRUNC             // [OBSOLETE] Renamed in 1.90.0 (Sept 2023)

// Hint for branch prediction
#if (defined(__cplusplus) && (__cplusplus >= 202002L)) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 202002L))
#define LINEARUI_LIKELY   [[likely]]
#define LINEARUI_UNLIKELY [[unlikely]]
#else
#define LINEARUI_LIKELY
#define LINEARUI_UNLIKELY
#endif

// Enforce cdecl calling convention for functions called by the standard library, in case compilation settings changed the default to e.g. __vectorcall
#ifdef _MSC_VER
#define LINEARUI_CDECL __cdecl
#else
#define LINEARUI_CDECL
#endif

// Warnings
#if defined(_MSC_VER) && !defined(__clang__)
#define LINEARUI_MSVC_WARNING_SUPPRESS(XXXX)  __pragma(warning(suppress: XXXX))
#else
#define LINEARUI_MSVC_WARNING_SUPPRESS(XXXX)
#endif

// Debug Tools
// Use 'Metrics/Debugger->Tools->Item Picker' to break into the call-stack of a specific item.
// This will call LINEARUI_DEBUG_BREAK() which you may redefine yourself. See https://github.com/scottt/debugbreak for more reference.
#ifndef LINEARUI_DEBUG_BREAK
#if defined (_MSC_VER)
#define LINEARUI_DEBUG_BREAK()    __debugbreak()
#elif defined(__clang__)
#define LINEARUI_DEBUG_BREAK()    __builtin_debugtrap()
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define LINEARUI_DEBUG_BREAK()    __asm__ volatile("int3;nop")
#elif defined(__GNUC__) && defined(__thumb__)
#define LINEARUI_DEBUG_BREAK()    __asm__ volatile(".inst 0xde01")
#elif defined(__GNUC__) && defined(__arm__) && !defined(__thumb__)
#define LINEARUI_DEBUG_BREAK()    __asm__ volatile(".inst 0xe7f001f0")
#else
#define LINEARUI_DEBUG_BREAK()    LINEARUI_ASSERT(0)    // It is expected that you define LINEARUI_DEBUG_BREAK() into something that will break nicely in a debugger!
#endif
#endif // #ifndef LINEARUI_DEBUG_BREAK

// Format specifiers, printing 64-bit hasn't been decently standardized...
// In a real application you should be using PRId64 and PRIu64 from <inttypes.h> (non-windows) and on Windows define them yourself.
#if defined(_MSC_VER) && !defined(__clang__)
#define LINEARUI_PRId64   "I64d"
#define LINEARUI_PRIu64   "I64u"
#define LINEARUI_PRIX64   "I64X"
#else
#define LINEARUI_PRId64   "lld"
#define LINEARUI_PRIu64   "llu"
#define LINEARUI_PRIX64   "llX"
#endif

//-----------------------------------------------------------------------------
// [SECTION] Generic helpers
// Note that the LinearXXX helpers functions are lower-level than LinearUI functions.
// LinearUI functions or the LinearUI context are never called/used from other LinearXXX functions.
//-----------------------------------------------------------------------------
// - Helpers: Hashing
// - Helpers: Sorting
// - Helpers: Bit manipulation
// - Helpers: String
// - Helpers: Formatting
// - Helpers: UTF-8 <> wchar conversions
// - Helpers: LinearVec2/LinearVec4 operators
// - Helpers: Maths
// - Helpers: Geometry
// - Helper: LinearVec1
// - Helper: LinearVec2ih
// - Helper: LinearRect
// - Helper: LinearBitArray
// - Helper: LinearBitVector
// - Helper: LinearSpan<>, LinearSpanAllocator<>
// - Helper: LinearStableVector<>
// - Helper: LinearPool<>
// - Helper: LinearChunkStream<>
// - Helper: LinearUITextIndex
// - Helper: LinearUIStorage
//-----------------------------------------------------------------------------

// Helpers: Hashing
LINEARUI_API LinearUIID       LinearHashData(const void* data, size_t data_size, LinearUIID seed = 0);
LINEARUI_API LinearUIID       LinearHashStr(const char* data, size_t data_size = 0, LinearUIID seed = 0);
LINEARUI_API const char*   LinearHashSkipUncontributingPrefix(const char* label);

// Helpers: Sorting
#ifndef LinearQsort
inline void             LinearQsort(void* base, size_t count, size_t size_of_element, int(LINEARUI_CDECL *compare_func)(void const*, void const*)) { if (count > 1) qsort(base, count, size_of_element, compare_func); }
#endif

// Helpers: Color Blending
LINEARUI_API LinearU32         LinearAlphaBlendColors(LinearU32 col_a, LinearU32 col_b);

// Helpers: Bit manipulation
inline bool             LinearIsPowerOfTwo(int v)               { return v != 0 && (v & (v - 1)) == 0; }
inline bool             LinearIsPowerOfTwo(LinearU64 v)             { return v != 0 && (v & (v - 1)) == 0; }
inline int              LinearUpperPowerOfTwo(int v)            { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; return v; }
inline unsigned int     LinearCountSetBits(unsigned int v)      { unsigned int count = 0; while (v > 0) { v = v & (v - 1); count++; } return count; }

// Helpers: String
#define LinearStrlen strlen
#define LinearMemchr memchr
LINEARUI_API int           LinearStricmp(const char* str1, const char* str2);                      // Case insensitive compare.
LINEARUI_API int           LinearStrnicmp(const char* str1, const char* str2, size_t count);       // Case insensitive compare to a certain count.
LINEARUI_API void          LinearStrncpy(char* dst, const char* src, size_t count);                // Copy to a certain count and always zero terminate (strncpy doesn't).
LINEARUI_API char*         LinearStrdup(const char* str);                                          // Duplicate a string.
LINEARUI_API void*         LinearMemdup(const void* src, size_t size);                             // Duplicate a chunk of memory.
LINEARUI_API char*         LinearStrdupcpy(char* dst, size_t* p_dst_size, const char* str);        // Copy in provided buffer, recreate buffer if needed.
LINEARUI_API const char*   LinearStrchrRange(const char* str_begin, const char* str_end, char c);  // Find first occurrence of 'c' in string range.
LINEARUI_API const char*   LinearStreolRange(const char* str, const char* str_end);                // End end-of-line
LINEARUI_API const char*   LinearStristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end);  // Find a substring in a string range.
LINEARUI_API void          LinearStrTrimBlanks(char* str);                                         // Remove leading and trailing blanks from a buffer.
LINEARUI_API const char*   LinearStrSkipBlank(const char* str);                                    // Find first non-blank character.
LINEARUI_API int           LinearStrlenW(const LinearWchar* str);                                      // Computer string length (LinearWchar string)
LINEARUI_API const char*   LinearStrbol(const char* buf_mid_line, const char* buf_begin);          // Find beginning-of-line
LINEARUI_MSVC_RUNTIME_CHECKS_OFF
inline char             LinearToUpper(char c)               { return (c >= 'a' && c <= 'z') ? c &= ~32 : c; }
inline bool             LinearCharIsBlankA(char c)          { return c == ' ' || c == '\t'; }
inline bool             LinearCharIsBlankW(unsigned int c)  { return c == ' ' || c == '\t' || c == 0x3000; }
inline bool             LinearCharIsXdigitA(char c)         { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }
LINEARUI_MSVC_RUNTIME_CHECKS_RESTORE

// Helpers: Formatting
LINEARUI_API int           LinearFormatString(char* buf, size_t buf_size, const char* fmt, ...) LINEARUI_FMTARGS(3);
LINEARUI_API int           LinearFormatStringV(char* buf, size_t buf_size, const char* fmt, va_list args) LINEARUI_FMTLIST(3);
LINEARUI_API void          LinearFormatStringToTempBuffer(const char** out_buf, const char** out_buf_end, const char* fmt, ...) LINEARUI_FMTARGS(3);
LINEARUI_API void          LinearFormatStringToTempBufferV(const char** out_buf, const char** out_buf_end, const char* fmt, va_list args) LINEARUI_FMTLIST(3);
LINEARUI_API const char*   LinearParseFormatFindStart(const char* format);
LINEARUI_API const char*   LinearParseFormatFindEnd(const char* format);
LINEARUI_API const char*   LinearParseFormatTrimDecorations(const char* format, char* buf, size_t buf_size);
LINEARUI_API void          LinearParseFormatSanitizeForPrinting(const char* fmt_in, char* fmt_out, size_t fmt_out_size);
LINEARUI_API const char*   LinearParseFormatSanitizeForScanning(const char* fmt_in, char* fmt_out, size_t fmt_out_size);
LINEARUI_API int           LinearParseFormatPrecision(const char* format, int default_value);

// Helpers: UTF-8 <> wchar conversions
LINEARUI_API int           LinearTextCharToUtf8(char out_buf[5], unsigned int c);                                                      // return output UTF-8 bytes count
LINEARUI_API int           LinearTextStrToUtf8(char* out_buf, int out_buf_size, const LinearWchar* in_text, const LinearWchar* in_text_end);   // return output UTF-8 bytes count
LINEARUI_API int           LinearTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end);               // read one character. return input UTF-8 bytes count
LINEARUI_API int           LinearTextStrFromUtf8(LinearWchar* out_buf, int out_buf_size, const char* in_text, const char* in_text_end, const char** in_remaining = NULL);   // return input UTF-8 bytes count
LINEARUI_API int           LinearTextCountCharsFromUtf8(const char* in_text, const char* in_text_end);                                 // return number of UTF-8 code-points (NOT bytes count)
LINEARUI_API int           LinearTextCountUtf8BytesFromChar(const char* in_text, const char* in_text_end);                             // return number of bytes to express one char in UTF-8
LINEARUI_API int           LinearTextCountUtf8BytesFromStr(const LinearWchar* in_text, const LinearWchar* in_text_end);                        // return number of bytes to express string in UTF-8
LINEARUI_API const char*   LinearTextFindPreviousUtf8Codepoint(const char* in_text_start, const char* in_p);                           // return previous UTF-8 code-point.
LINEARUI_API const char*   LinearTextFindValidUtf8CodepointEnd(const char* in_text_start, const char* in_text_end, const char* in_p);  // return previous UTF-8 code-point if 'in_p' is not the end of a valid one.
LINEARUI_API int           LinearTextCountLines(const char* in_text, const char* in_text_end);                                         // return number of lines taken by text. trailing carriage return doesn't count as an extra line.

// Helpers: High-level text functions (DO NOT USE!!! THIS IS A MINIMAL SUBSET OF LARGER UPCOMING CHANGES)
enum LinearDrawTextFlags_
{
    LinearDrawTextFlags_None                = 0,
    LinearDrawTextFlags_CpuFineClip         = 1 << 0,    // Must be == 1/true for legacy with 'bool cpu_fine_clip' arg to RenderText()
    LinearDrawTextFlags_WrapKeepBlanks      = 1 << 1,
    LinearDrawTextFlags_StopOnNewLine       = 1 << 2,
};
LINEARUI_API LinearVec2        LinearFontCalcTextSizeEx(LinearFont* font, float size, float max_width, float wrap_width, const char* text_begin, const char* text_end_display, const char* text_end, const char** out_remaining, LinearVec2* out_offset, LinearDrawTextFlags flags);
LINEARUI_API const char*   LinearFontCalcWordWrapPositionEx(LinearFont* font, float size, const char* text, const char* text_end, float wrap_width, LinearDrawTextFlags flags = 0);
LINEARUI_API const char*   LinearTextCalcWordWrapNextLineStart(const char* text, const char* text_end, LinearDrawTextFlags flags = 0); // trim trailing space and find beginning of next line

// Character classification for word-wrapping logic
enum LinearWcharClass
{
    LinearWcharClass_Blank, LinearWcharClass_Punct, LinearWcharClass_Other
};
LINEARUI_API void          LinearTextInitClassifiers();
LINEARUI_API void          LinearTextClassifierClear(LinearU32* bits, unsigned int codepoint_min, unsigned int codepoint_end, LinearWcharClass char_class);
LINEARUI_API void          LinearTextClassifierSetCharClass(LinearU32* bits, unsigned int codepoint_min, unsigned int codepoint_end, LinearWcharClass char_class, unsigned int c);
LINEARUI_API void          LinearTextClassifierSetCharClassFromStr(LinearU32* bits, unsigned int codepoint_min, unsigned int codepoint_end, LinearWcharClass char_class, const char* s);

// Helpers: File System
#ifdef LINEARUI_DISABLE_FILE_FUNCTIONS
#define LINEARUI_DISABLE_DEFAULT_FILE_FUNCTIONS
typedef void* LinearFileHandle;
inline LinearFileHandle         LinearFileOpen(const char*, const char*)                    { return NULL; }
inline bool                 LinearFileClose(LinearFileHandle)                               { return false; }
inline LinearU64                LinearFileGetSize(LinearFileHandle)                             { return (LinearU64)-1; }
inline LinearU64                LinearFileRead(void*, LinearU64, LinearU64, LinearFileHandle)           { return 0; }
inline LinearU64                LinearFileWrite(const void*, LinearU64, LinearU64, LinearFileHandle)    { return 0; }
#endif
#ifndef LINEARUI_DISABLE_DEFAULT_FILE_FUNCTIONS
typedef FILE* LinearFileHandle;
LINEARUI_API LinearFileHandle      LinearFileOpen(const char* filename, const char* mode);
LINEARUI_API bool              LinearFileClose(LinearFileHandle file);
LINEARUI_API LinearU64             LinearFileGetSize(LinearFileHandle file);
LINEARUI_API LinearU64             LinearFileRead(void* data, LinearU64 size, LinearU64 count, LinearFileHandle file);
LINEARUI_API LinearU64             LinearFileWrite(const void* data, LinearU64 size, LinearU64 count, LinearFileHandle file);
#else
#define LINEARUI_DISABLE_TTY_FUNCTIONS // Can't use stdout, fflush if we are not using default file functions
#endif
LINEARUI_API void*             LinearFileLoadToMemory(const char* filename, const char* mode, size_t* out_file_size = NULL, int padding_bytes = 0);

// Helpers: Maths
LINEARUI_MSVC_RUNTIME_CHECKS_OFF
// - Wrapper for standard libs functions. (Note that linearui_demo.cpp does _not_ use them to keep the code easy to copy)
#ifndef LINEARUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#define LinearFabs(X)           fabsf(X)
#define LinearSqrt(X)           sqrtf(X)
#define LinearFmod(X, Y)        fmodf((X), (Y))
#define LinearCos(X)            cosf(X)
#define LinearSin(X)            sinf(X)
#define LinearAcos(X)           acosf(X)
#define LinearAtan2(Y, X)       atan2f((Y), (X))
#define LinearAtof(STR)         atof(STR)
#define LinearCeil(X)           ceilf(X)
inline float  LinearPow(float x, float y)    { return powf(x, y); }          // DragBehaviorT/SliderBehaviorT uses LinearPow with either float/double and need the precision
inline double LinearPow(double x, double y)  { return pow(x, y); }
inline float  LinearLog(float x)             { return logf(x); }             // DragBehaviorT/SliderBehaviorT uses LinearLog with either float/double and need the precision
inline double LinearLog(double x)            { return log(x); }
inline int    LinearAbs(int x)               { return x < 0 ? -x : x; }
inline float  LinearAbs(float x)             { return fabsf(x); }
inline double LinearAbs(double x)            { return fabs(x); }
inline float  LinearSign(float x)            { return (x < 0.0f) ? -1.0f : (x > 0.0f) ? 1.0f : 0.0f; } // Sign operator - returns -1, 0 or 1 based on sign of argument
inline double LinearSign(double x)           { return (x < 0.0) ? -1.0 : (x > 0.0) ? 1.0 : 0.0; }
#ifdef LINEARUI_ENABLE_SSE
inline float  LinearRsqrt(float x)           { return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x))); }
#else
inline float  LinearRsqrt(float x)           { return 1.0f / sqrtf(x); }
#endif
inline double LinearRsqrt(double x)          { return 1.0 / sqrt(x); }
#endif
// - LinearMin/LinearMax/LinearClamp/LinearLerp/LinearSwap are used by widgets which support variety of types: signed/unsigned int/long long float/double
// (Exceptionally using templates here but we could also redefine them for those types)
template<typename T> T LinearMin(T lhs, T rhs)                              { return lhs < rhs ? lhs : rhs; }
template<typename T> T LinearMax(T lhs, T rhs)                              { return lhs >= rhs ? lhs : rhs; }
template<typename T> T LinearClamp(T v, T mn, T mx)                         { return (v < mn) ? mn : (v > mx) ? mx : v; }
template<typename T> T LinearLerp(double a, double b, float t)              { return (T)(a + (b - a) * (double)t); }
template<typename T> T LinearLerp(T a, T b, float t)                        { return (T)((float)a + (float)(b - a) * t); }
template<typename T> void LinearSwap(T& a, T& b)                            { T tmp = a; a = b; b = tmp; }
template<typename T> T LinearAddClampOverflow(T a, T b, T mn, T mx)         { if (b < 0 && (a < mn - b)) return mn; if (b > 0 && (a > mx - b)) return mx; return a + b; }
template<typename T> T LinearSubClampOverflow(T a, T b, T mn, T mx)         { if (b > 0 && (a < mn + b)) return mn; if (b < 0 && (a > mx + b)) return mx; return a - b; }
// - Misc maths helpers
inline LinearVec2 LinearMin(const LinearVec2& lhs, const LinearVec2& rhs)               { return LinearVec2(lhs.x < rhs.x ? lhs.x : rhs.x, lhs.y < rhs.y ? lhs.y : rhs.y); }
inline LinearVec2 LinearMax(const LinearVec2& lhs, const LinearVec2& rhs)               { return LinearVec2(lhs.x >= rhs.x ? lhs.x : rhs.x, lhs.y >= rhs.y ? lhs.y : rhs.y); }
inline LinearVec2 LinearClamp(const LinearVec2& v, const LinearVec2&mn, const LinearVec2&mx){ return LinearVec2((v.x < mn.x) ? mn.x : (v.x > mx.x) ? mx.x : v.x, (v.y < mn.y) ? mn.y : (v.y > mx.y) ? mx.y : v.y); }
inline LinearVec2 LinearLerp(const LinearVec2& a, const LinearVec2& b, float t)         { return LinearVec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
inline LinearVec2 LinearLerp(const LinearVec2& a, const LinearVec2& b, const LinearVec2& t) { return LinearVec2(a.x + (b.x - a.x) * t.x, a.y + (b.y - a.y) * t.y); }
inline LinearVec4 LinearLerp(const LinearVec4& a, const LinearVec4& b, float t)         { return LinearVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t); }
inline float  LinearSaturate(float f)                                       { return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f; }
inline float  LinearLengthSqr(const LinearVec2& lhs)                            { return (lhs.x * lhs.x) + (lhs.y * lhs.y); }
inline float  LinearLengthSqr(const LinearVec4& lhs)                            { return (lhs.x * lhs.x) + (lhs.y * lhs.y) + (lhs.z * lhs.z) + (lhs.w * lhs.w); }
inline float  LinearInvLength(const LinearVec2& lhs, float fail_value)          { float d = (lhs.x * lhs.x) + (lhs.y * lhs.y); if (d > 0.0f) return LinearRsqrt(d); return fail_value; }
inline float  LinearTrunc(float f)                                          { return (float)(int)(f); }
inline LinearVec2 LinearTrunc(const LinearVec2& v)                                  { return LinearVec2((float)(int)(v.x), (float)(int)(v.y)); }
inline float  LinearFloor(float f)                                          { return (float)((f >= 0 || (float)(int)f == f) ? (int)f : (int)f - 1); } // Decent replacement for floorf()
inline LinearVec2 LinearFloor(const LinearVec2& v)                                  { return LinearVec2(LinearFloor(v.x), LinearFloor(v.y)); }
inline float  LinearTrunc64(float f)                                        { return (float)(LinearS64)(f); }
inline float  LinearRound64(float f)                                        { return (float)(LinearS64)(f + 0.5f); } // FIXME: Positive values only.
inline float  LinearCeilFast(float f)                                       { int i = (int)f; return (float)(i + (f > (float)i)); } // Consider using the the bit-hack version (search for "0x1p120f").
inline int    LinearModPositive(int a, int b)                               { return (a + b) % b; }
inline float  LinearDot(const LinearVec2& a, const LinearVec2& b)                   { return a.x * b.x + a.y * b.y; }
inline LinearVec2 LinearRotate(const LinearVec2& v, float cos_a, float sin_a)       { return LinearVec2(v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a); }
inline float  LinearLinearSweep(float current, float target, float speed)   { if (current < target) return LinearMin(current + speed, target); if (current > target) return LinearMax(current - speed, target); return current; }
inline float  LinearLinearRemapClamp(float s0, float s1, float d0, float d1, float x) { return LinearSaturate((x - s0) / (s1 - s0)) * (d1 - d0) + d0; }
inline LinearVec2 LinearMul(const LinearVec2& lhs, const LinearVec2& rhs)               { return LinearVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
inline bool   LinearIsFloatAboveGuaranteedIntegerPrecision(float f)         { return f <= -16777216 || f >= 16777216; }
inline float  LinearExponentialMovingAverage(float avg, float sample, int n){ avg -= avg / (float)n; avg += sample / (float)n; return avg; }
LINEARUI_MSVC_RUNTIME_CHECKS_RESTORE

// Helpers: Geometry
LINEARUI_API LinearVec2     LinearBezierCubicCalc(const LinearVec2& p1, const LinearVec2& p2, const LinearVec2& p3, const LinearVec2& p4, float t);
LINEARUI_API LinearVec2     LinearBezierCubicClosestPoint(const LinearVec2& p1, const LinearVec2& p2, const LinearVec2& p3, const LinearVec2& p4, const LinearVec2& p, int num_segments);       // For curves with explicit number of segments
LINEARUI_API LinearVec2     LinearBezierCubicClosestPointCasteljau(const LinearVec2& p1, const LinearVec2& p2, const LinearVec2& p3, const LinearVec2& p4, const LinearVec2& p, float tess_tol);// For auto-tessellated curves you can use tess_tol = style.CurveTessellationTol
LINEARUI_API LinearVec2     LinearBezierQuadraticCalc(const LinearVec2& p1, const LinearVec2& p2, const LinearVec2& p3, float t);
LINEARUI_API LinearVec2     LinearLineClosestPoint(const LinearVec2& a, const LinearVec2& b, const LinearVec2& p);
LINEARUI_API bool       LinearTriangleContainsPoint(const LinearVec2& a, const LinearVec2& b, const LinearVec2& c, const LinearVec2& p);
LINEARUI_API LinearVec2     LinearTriangleClosestPoint(const LinearVec2& a, const LinearVec2& b, const LinearVec2& c, const LinearVec2& p);
LINEARUI_API void       LinearTriangleBarycentricCoords(const LinearVec2& a, const LinearVec2& b, const LinearVec2& c, const LinearVec2& p, float& out_u, float& out_v, float& out_w);
inline float         LinearTriangleArea(const LinearVec2& a, const LinearVec2& b, const LinearVec2& c)          { return LinearFabs((a.x * (b.y - c.y)) + (b.x * (c.y - a.y)) + (c.x * (a.y - b.y))) * 0.5f; }
inline bool          LinearTriangleIsClockwise(const LinearVec2& a, const LinearVec2& b, const LinearVec2& c)   { return ((b.x - a.x) * (c.y - b.y)) - ((c.x - b.x) * (b.y - a.y)) > 0.0f; }

// Helper: LinearVec1 (1D vector)
// (this odd construct is used to facilitate the transition between 1D and 2D, and the maintenance of some branches/patches)
LINEARUI_MSVC_RUNTIME_CHECKS_OFF
struct LinearVec1
{
    float   x;
    constexpr LinearVec1()         : x(0.0f) { }
    constexpr LinearVec1(float _x) : x(_x) { }
};

// Helper: LinearVec2i (2D vector, integer)
struct LinearVec2i
{
    int         x, y;
    constexpr LinearVec2i()                             : x(0), y(0) {}
    constexpr LinearVec2i(int _x, int _y)               : x(_x), y(_y) {}
};

// Helper: LinearVec2ih (2D vector, half-size integer, for long-term packed storage)
struct LinearVec2ih
{
    short   x, y;
    constexpr LinearVec2ih()                           : x(0), y(0) {}
    constexpr LinearVec2ih(short _x, short _y)         : x(_x), y(_y) {}
    constexpr explicit LinearVec2ih(const LinearVec2& rhs) : x((short)rhs.x), y((short)rhs.y) {}
};

// Helper: LinearRect (2D axis aligned bounding-box)
// NB: we can't rely on LinearVec2 math operators being available here!
struct LINEARUI_API LinearRect
{
    LinearVec2      Min;    // Upper-left
    LinearVec2      Max;    // Lower-right

    constexpr LinearRect()                                        : Min(0.0f, 0.0f), Max(0.0f, 0.0f)  {}
    constexpr LinearRect(const LinearVec2& min, const LinearVec2& max)    : Min(min), Max(max)                {}
    constexpr LinearRect(const LinearVec4& v)                         : Min(v.x, v.y), Max(v.z, v.w)      {}
    constexpr LinearRect(float x1, float y1, float x2, float y2)  : Min(x1, y1), Max(x2, y2)          {}

    LinearVec2      GetCenter() const                   { return LinearVec2((Min.x + Max.x) * 0.5f, (Min.y + Max.y) * 0.5f); }
    LinearVec2      GetSize() const                     { return LinearVec2(Max.x - Min.x, Max.y - Min.y); }
    float       GetWidth() const                    { return Max.x - Min.x; }
    float       GetHeight() const                   { return Max.y - Min.y; }
    float       GetArea() const                     { return (Max.x - Min.x) * (Max.y - Min.y); }
    LinearVec2      GetTL() const                       { return Min; }                   // Top-left
    LinearVec2      GetTR() const                       { return LinearVec2(Max.x, Min.y); }  // Top-right
    LinearVec2      GetBL() const                       { return LinearVec2(Min.x, Max.y); }  // Bottom-left
    LinearVec2      GetBR() const                       { return Max; }                   // Bottom-right
    bool        Contains(const LinearVec2& p) const     { return p.x     >= Min.x && p.y     >= Min.y && p.x     <  Max.x && p.y     <  Max.y; }
    bool        Contains(const LinearRect& r) const     { return r.Min.x >= Min.x && r.Min.y >= Min.y && r.Max.x <= Max.x && r.Max.y <= Max.y; }
    bool        ContainsWithPad(const LinearVec2& p, const LinearVec2& pad) const { return p.x >= Min.x - pad.x && p.y >= Min.y - pad.y && p.x < Max.x + pad.x && p.y < Max.y + pad.y; }
    bool        Overlaps(const LinearRect& r) const     { return r.Min.y <  Max.y && r.Max.y >  Min.y && r.Min.x <  Max.x && r.Max.x >  Min.x; }
    void        Add(const LinearVec2& p)                { if (Min.x > p.x)     Min.x = p.x;     if (Min.y > p.y)     Min.y = p.y;     if (Max.x < p.x)     Max.x = p.x;     if (Max.y < p.y)     Max.y = p.y; }
    void        Add(const LinearRect& r)                { if (Min.x > r.Min.x) Min.x = r.Min.x; if (Min.y > r.Min.y) Min.y = r.Min.y; if (Max.x < r.Max.x) Max.x = r.Max.x; if (Max.y < r.Max.y) Max.y = r.Max.y; }
    void        AddX(float x)                       { if (Min.x > x)       Min.x = x;       if (Max.x < x)       Max.x = x; }
    void        AddY(float y)                       { if (Min.y > y)       Min.y = y;       if (Max.y < y)       Max.y = y; }
    void        Expand(const float amount)          { Min.x -= amount;   Min.y -= amount;   Max.x += amount;   Max.y += amount; }
    void        Expand(const LinearVec2& amount)        { Min.x -= amount.x; Min.y -= amount.y; Max.x += amount.x; Max.y += amount.y; }
    void        Translate(const LinearVec2& d)          { Min.x += d.x; Min.y += d.y; Max.x += d.x; Max.y += d.y; }
    void        TranslateX(float dx)                { Min.x += dx; Max.x += dx; }
    void        TranslateY(float dy)                { Min.y += dy; Max.y += dy; }
    void        ClipWith(const LinearRect& r)           { Min = LinearMax(Min, r.Min); Max = LinearMin(Max, r.Max); }                   // Simple version, may lead to an inverted rectangle, which is fine for Contains/Overlaps test but not for display.
    void        ClipWithFull(const LinearRect& r)       { Min = LinearClamp(Min, r.Min, r.Max); Max = LinearClamp(Max, r.Min, r.Max); } // Full version, ensure both points are fully clipped.
    bool        IsInverted() const                  { return Min.x > Max.x || Min.y > Max.y; }
    LinearVec4      ToVec4() const                      { return LinearVec4(Min.x, Min.y, Max.x, Max.y); }
    const LinearVec4& AsVec4() const                    { return *(const LinearVec4*)&Min.x; }
};

// Helper: LinearBitArray
#define         LINEARUI_BITARRAY_TESTBIT(_ARRAY, _N)                 ((_ARRAY[(_N) >> 5] & ((LinearU32)1 << ((_N) & 31))) != 0) // Macro version of LinearBitArrayTestBit(): ensure args have side-effect or are costly!
#define         LINEARUI_BITARRAY_CLEARBIT(_ARRAY, _N)                ((_ARRAY[(_N) >> 5] &= ~((LinearU32)1 << ((_N) & 31))))    // Macro version of LinearBitArrayClearBit(): ensure args have side-effect or are costly!
inline size_t   LinearBitArrayGetStorageSizeInBytes(int bitcount)   { return (size_t)((bitcount + 31) >> 5) << 2; }
inline void     LinearBitArrayClearAllBits(LinearU32* arr, int bitcount){ memset(arr, 0, LinearBitArrayGetStorageSizeInBytes(bitcount)); }
inline bool     LinearBitArrayTestBit(const LinearU32* arr, int n)      { LinearU32 mask = (LinearU32)1 << (n & 31); return (arr[n >> 5] & mask) != 0; }
inline void     LinearBitArrayClearBit(LinearU32* arr, int n)           { LinearU32 mask = (LinearU32)1 << (n & 31); arr[n >> 5] &= ~mask; }
inline void     LinearBitArraySetBit(LinearU32* arr, int n)             { LinearU32 mask = (LinearU32)1 << (n & 31); arr[n >> 5] |= mask; }
inline void     LinearBitArraySetBitRange(LinearU32* arr, int n, int n2) // Works on range [n..n2)
{
    n2--;
    while (n <= n2)
    {
        int a_mod = (n & 31);
        int b_mod = (n2 > (n | 31) ? 31 : (n2 & 31)) + 1;
        LinearU32 mask = (LinearU32)(((LinearU64)1 << b_mod) - 1) & ~(LinearU32)(((LinearU64)1 << a_mod) - 1);
        arr[n >> 5] |= mask;
        n = (n + 32) & ~31;
    }
}

typedef LinearU32* LinearBitArrayPtr; // Name for use in structs

// Helper: LinearBitArray class (wrapper over LinearBitArray functions)
// Store 1-bit per value.
template<int BITCOUNT, int OFFSET = 0>
struct LinearBitArray
{
    LinearU32           Data[(BITCOUNT + 31) >> 5];
    LinearBitArray()                                { ClearAllBits(); }
    void            ClearAllBits()              { memset(Data, 0, sizeof(Data)); }
    void            SetAllBits()                { memset(Data, 255, sizeof(Data)); }
    bool            TestBit(int n) const        { n += OFFSET; LINEARUI_ASSERT(n >= 0 && n < BITCOUNT); return LINEARUI_BITARRAY_TESTBIT(Data, n); }
    void            SetBit(int n)               { n += OFFSET; LINEARUI_ASSERT(n >= 0 && n < BITCOUNT); LinearBitArraySetBit(Data, n); }
    void            ClearBit(int n)             { n += OFFSET; LINEARUI_ASSERT(n >= 0 && n < BITCOUNT); LinearBitArrayClearBit(Data, n); }
    void            SetBitRange(int n, int n2)  { n += OFFSET; n2 += OFFSET; LINEARUI_ASSERT(n >= 0 && n < BITCOUNT && n2 > n && n2 <= BITCOUNT); LinearBitArraySetBitRange(Data, n, n2); } // Works on range [n..n2)
    bool            operator[](int n) const     { n += OFFSET; LINEARUI_ASSERT(n >= 0 && n < BITCOUNT); return LINEARUI_BITARRAY_TESTBIT(Data, n); }
};

// Helper: LinearBitVector
// Store 1-bit per value.
struct LINEARUI_API LinearBitVector
{
    LinearVector<LinearU32> Storage;
    void            Create(int sz)              { Storage.resize((sz + 31) >> 5); memset(Storage.Data, 0, (size_t)Storage.Size * sizeof(Storage.Data[0])); }
    void            Clear()                     { Storage.clear(); }
    bool            TestBit(int n) const        { LINEARUI_ASSERT(n < (Storage.Size << 5)); return LINEARUI_BITARRAY_TESTBIT(Storage.Data, n); }
    void            SetBit(int n)               { LINEARUI_ASSERT(n < (Storage.Size << 5)); LinearBitArraySetBit(Storage.Data, n); }
    void            ClearBit(int n)             { LINEARUI_ASSERT(n < (Storage.Size << 5)); LinearBitArrayClearBit(Storage.Data, n); }
};
LINEARUI_MSVC_RUNTIME_CHECKS_RESTORE

// Helper: LinearSpan<>
// Pointing to a span of data we don't own.
template<typename T>
struct LinearSpan
{
    T*                  Data;
    T*                  DataEnd;

    // Constructors, destructor
    inline LinearSpan()                                 { Data = DataEnd = NULL; }
    inline LinearSpan(T* data, int size)                { Data = data; DataEnd = data + size; }
    inline LinearSpan(T* data, T* data_end)             { Data = data; DataEnd = data_end; }

    inline void         set(T* data, int size)      { Data = data; DataEnd = data + size; }
    inline void         set(T* data, T* data_end)   { Data = data; DataEnd = data_end; }
    inline int          size() const                { return (int)(ptrdiff_t)(DataEnd - Data); }
    inline int          size_in_bytes() const       { return (int)(ptrdiff_t)(DataEnd - Data) * (int)sizeof(T); }
    inline T&           operator[](int i)           { T* p = Data + i; LINEARUI_ASSERT(p >= Data && p < DataEnd); return *p; }
    inline const T&     operator[](int i) const     { const T* p = Data + i; LINEARUI_ASSERT(p >= Data && p < DataEnd); return *p; }

    inline T*           begin()                     { return Data; }
    inline const T*     begin() const               { return Data; }
    inline T*           end()                       { return DataEnd; }
    inline const T*     end() const                 { return DataEnd; }

    // Utilities
    inline int  index_from_ptr(const T* it) const   { LINEARUI_ASSERT(it >= Data && it < DataEnd); const ptrdiff_t off = it - Data; return (int)off; }
};

// Helper: LinearSpanAllocator<>
// Facilitate storing multiple chunks into a single large block (the "arena")
// - Usage: call Reserve() N times, allocate GetArenaSizeInBytes() worth, pass it to SetArenaBasePtr(), call GetSpan() N times to retrieve the aligned ranges.
template<int CHUNKS>
struct LinearSpanAllocator
{
    char*   BasePtr;
    int     CurrOff;
    int     CurrIdx;
    int     Offsets[CHUNKS];
    int     Sizes[CHUNKS];

    LinearSpanAllocator()                               { memset((void*)this, 0, sizeof(*this)); }
    inline void  Reserve(int n, size_t sz, int a=4) { LINEARUI_ASSERT(n == CurrIdx && n < CHUNKS); CurrOff = LINEARUI_MEMALIGN(CurrOff, a); Offsets[n] = CurrOff; Sizes[n] = (int)sz; CurrIdx++; CurrOff += (int)sz; }
    inline int   GetArenaSizeInBytes()              { return CurrOff; }
    inline void  SetArenaBasePtr(void* base_ptr)    { BasePtr = (char*)base_ptr; }
    inline void* GetSpanPtrBegin(int n)             { LINEARUI_ASSERT(n >= 0 && n < CHUNKS && CurrIdx == CHUNKS); return (void*)(BasePtr + Offsets[n]); }
    inline void* GetSpanPtrEnd(int n)               { LINEARUI_ASSERT(n >= 0 && n < CHUNKS && CurrIdx == CHUNKS); return (void*)(BasePtr + Offsets[n] + Sizes[n]); }
    template<typename T>
    inline void  GetSpan(int n, LinearSpan<T>* span)    { span->set((T*)GetSpanPtrBegin(n), (T*)GetSpanPtrEnd(n)); }
};

// Helper: LinearStableVector<>
// Allocating chunks of BLOCKSIZE items. Objects pointers are never invalidated when growing, only by clear().
// Important: does not destruct anything!
// Implemented only the minimum set of functions we need for it.
template<typename T, int BLOCKSIZE>
struct LinearStableVector
{
    int                 Size = 0;
    int                 Capacity = 0;
    LinearVector<T*>        Blocks;

    // Functions
    inline ~LinearStableVector()                        { for (T* block : Blocks) LINEARUI_FREE(block); }

    inline void         clear()                     { Size = Capacity = 0; Blocks.clear_delete(); }
    inline void         resize(int new_size)        { if (new_size > Capacity) reserve(new_size); Size = new_size; }
    inline void         reserve(int new_cap)
    {
        new_cap = LINEARUI_MEMALIGN(new_cap, BLOCKSIZE);
        int old_count = Capacity / BLOCKSIZE;
        int new_count = new_cap / BLOCKSIZE;
        if (new_count <= old_count)
            return;
        Blocks.resize(new_count);
        for (int n = old_count; n < new_count; n++)
            Blocks[n] = (T*)LINEARUI_ALLOC(sizeof(T) * BLOCKSIZE);
        Capacity = new_cap;
    }
    inline T&           operator[](int i)           { LINEARUI_ASSERT(i >= 0 && i < Size); return Blocks[i / BLOCKSIZE][i % BLOCKSIZE]; }
    inline const T&     operator[](int i) const     { LINEARUI_ASSERT(i >= 0 && i < Size); return Blocks[i / BLOCKSIZE][i % BLOCKSIZE]; }
    inline T*           push_back(const T& v)       { int i = Size; LINEARUI_ASSERT(i >= 0); if (Size == Capacity) reserve(Capacity + BLOCKSIZE); void* ptr = &Blocks[i / BLOCKSIZE][i % BLOCKSIZE]; memcpy(ptr, &v, sizeof(v)); Size++; return (T*)ptr; }
};

// Helper: LinearPool<>
// Basic keyed storage for contiguous instances, slow/amortized insertion, O(1) indexable, O(Log N) queries by ID over a dense/hot buffer,
// Honor constructor/destructor. Add/remove invalidate all pointers. Indexes have the same lifetime as the associated object.
typedef int LinearPoolIdx;
template<typename T>
struct LinearPool
{
    LinearVector<T>     Buf;        // Contiguous data
    LinearUIStorage    Map;        // ID->Index
    LinearPoolIdx       FreeIdx;    // Next free idx to use
    LinearPoolIdx       AliveCount; // Number of active/alive items (for display purpose)

    LinearPool()    { FreeIdx = AliveCount = 0; }
    ~LinearPool()   { Clear(); }
    T*          GetByKey(LinearUIID key)               { int idx = Map.GetInt(key, -1); return (idx != -1) ? &Buf[idx] : NULL; }
    T*          GetByIndex(LinearPoolIdx n)             { return &Buf[n]; }
    LinearPoolIdx   GetIndex(const T* p) const          { LINEARUI_ASSERT(p >= Buf.Data && p < Buf.Data + Buf.Size); return (LinearPoolIdx)(p - Buf.Data); }
    T*          GetOrAddByKey(LinearUIID key)          { int* p_idx = Map.GetIntRef(key, -1); if (*p_idx != -1) return &Buf[*p_idx]; *p_idx = FreeIdx; return Add(); }
    bool        Contains(const T* p) const          { return (p >= Buf.Data && p < Buf.Data + Buf.Size); }
    void        Clear()                             { for (int n = 0; n < Map.Data.Size; n++) { int idx = Map.Data[n].val_i; if (idx != -1) Buf[idx].~T(); } Map.Clear(); Buf.clear(); FreeIdx = AliveCount = 0; }
    T*          Add()                               { int idx = FreeIdx; if (idx == Buf.Size) { Buf.resize(Buf.Size + 1); FreeIdx++; } else { FreeIdx = *(int*)&Buf[idx]; } LINEARUI_PLACEMENT_NEW(&Buf[idx]) T(); AliveCount++; return &Buf[idx]; }
    void        Remove(LinearUIID key, const T* p)     { Remove(key, GetIndex(p)); }
    void        Remove(LinearUIID key, LinearPoolIdx idx)  { Buf[idx].~T(); *(int*)&Buf[idx] = FreeIdx; FreeIdx = idx; Map.SetInt(key, -1); AliveCount--; }
    void        Reserve(int capacity)               { Buf.reserve(capacity); Map.Data.reserve(capacity); }

    // To iterate a LinearPool: for (int n = 0; n < pool.GetMapSize(); n++) if (T* t = pool.TryGetMapData(n)) { ... }
    // Can be avoided if you know .Remove() has never been called on the pool, or AliveCount == GetMapSize()
    int         GetAliveCount() const               { return AliveCount; }      // Number of active/alive items in the pool (for display purpose)
    int         GetBufSize() const                  { return Buf.Size; }
    int         GetMapSize() const                  { return Map.Data.Size; }   // It is the map we need iterate to find valid items, since we don't have "alive" storage anywhere
    T*          TryGetMapData(LinearPoolIdx n)          { int idx = Map.Data[n].val_i; if (idx == -1) return NULL; return GetByIndex(idx); }
};

// Helper: LinearChunkStream<>
// Build and iterate a contiguous stream of variable-sized structures.
// This is used by Settings to store persistent data while reducing allocation count.
// We store the chunk size first, and align the final size on 4 bytes boundaries.
// The tedious/zealous amount of casting is to avoid -Wcast-align warnings.
template<typename T>
struct LinearChunkStream
{
    LinearVector<char>  Buf;

    void    clear()                     { Buf.clear(); }
    bool    empty() const               { return Buf.Size == 0; }
    int     size() const                { return Buf.Size; }
    T*      alloc_chunk(size_t sz)      { size_t HDR_SZ = 4; sz = LINEARUI_MEMALIGN(HDR_SZ + sz, 4u); int off = Buf.Size; Buf.resize(off + (int)sz); ((int*)(void*)(Buf.Data + off))[0] = (int)sz; return (T*)(void*)(Buf.Data + off + (int)HDR_SZ); }
    T*      begin()                     { size_t HDR_SZ = 4; if (!Buf.Data) return NULL; return (T*)(void*)(Buf.Data + HDR_SZ); }
    T*      next_chunk(T* p)            { size_t HDR_SZ = 4; LINEARUI_ASSERT(p >= begin() && p < end()); p = (T*)(void*)((char*)(void*)p + chunk_size(p)); if (p == (T*)(void*)((char*)end() + HDR_SZ)) return (T*)0; LINEARUI_ASSERT(p < end()); return p; }
    int     chunk_size(const T* p)      { return ((const int*)p)[-1]; }
    T*      end()                       { return (T*)(void*)(Buf.Data + Buf.Size); }
    int     offset_from_ptr(const T* p) { LINEARUI_ASSERT(p >= begin() && p < end()); const ptrdiff_t off = (const char*)p - Buf.Data; return (int)off; }
    T*      ptr_from_offset(int off)    { LINEARUI_ASSERT(off >= 4 && off < Buf.Size); return (T*)(void*)(Buf.Data + off); }
    void    swap(LinearChunkStream<T>& rhs) { rhs.Buf.swap(Buf); }
};

// Helper: LinearUITextIndex
// Maintain a line index for a text buffer. This is a strong candidate to be moved into the public API.
struct LinearUITextIndex
{
    LinearVector<int>   Offsets;
    int             EndOffset = 0;                          // Because we don't own text buffer we need to maintain EndOffset (may bake in LineOffsets?)

    void            clear()                                 { Offsets.clear(); EndOffset = 0; }
    int             size()                                  { return Offsets.Size; }
    const char*     get_line_begin(const char* base, int n) { return base + (Offsets.Size != 0 ? Offsets[n] : 0); }
    const char*     get_line_end(const char* base, int n)   { return base + (n + 1 < Offsets.Size ? (Offsets[n + 1] - 1) : EndOffset); }
    void            append(const char* base, int old_size, int new_size);
};

// Helper: LinearUIStorage
LINEARUI_API LinearUIStoragePair* LinearLowerBound(LinearUIStoragePair* in_begin, LinearUIStoragePair* in_end, LinearUIID key);

//-----------------------------------------------------------------------------
// [SECTION] LinearDrawList support
//-----------------------------------------------------------------------------

// LinearDrawList: Helper function to calculate a circle's segment count given its radius and a "maximum error" value.
// Estimation of number of circle segment based on error is derived using method described in https://stackoverflow.com/a/2244088/15194693
// Number of segments (N) is calculated using equation:
//   N = ceil ( pi / acos(1 - error / r) )     where r > 0, error <= r
// Our equation is significantly simpler that one in the post thanks for choosing segment that is
// perpendicular to X axis. Follow steps in the article from this starting condition and you will
// will get this result.
//
// Rendering circles with an odd number of segments, while mathematically correct will produce
// asymmetrical results on the raster grid. Therefore we're rounding N to next even number (7->8, 8->8, 9->10 etc.)
#define LINEARUI_ROUNDUP_TO_EVEN(_V)                                  ((((_V) + 1) / 2) * 2)
#define LINEARUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN                     4
#define LINEARUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX                     512
#define LINEARUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(_RAD,_MAXERROR)    LinearClamp(LINEARUI_ROUNDUP_TO_EVEN((int)LinearCeil(LINEARUI_PI / LinearAcos(1 - LinearMin((_MAXERROR), (_RAD)) / (_RAD)))), LINEARUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN, LINEARUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX)

// Raw equation from LINEARUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC rewritten for 'r' and 'error'.
#define LINEARUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(_N,_MAXERROR)    ((_MAXERROR) / (1 - LinearCos(LINEARUI_PI / LinearMax((float)(_N), LINEARUI_PI))))
#define LINEARUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_ERROR(_N,_RAD)     ((1 - LinearCos(LINEARUI_PI / LinearMax((float)(_N), LINEARUI_PI))) / (_RAD))

// LinearDrawList: Lookup table size for adaptive arc drawing, cover full circle.
#ifndef LINEARUI_DRAWLIST_ARCFAST_TABLE_SIZE
#define LINEARUI_DRAWLIST_ARCFAST_TABLE_SIZE                          48 // Number of samples in lookup table.
#endif
#define LINEARUI_DRAWLIST_ARCFAST_SAMPLE_MAX                          LINEARUI_DRAWLIST_ARCFAST_TABLE_SIZE // Sample index _PathArcToFastEx() for 360 angle.

// Data shared between all LinearDrawList instances
// Conceptually this could have been called e.g. LinearDrawListSharedContext
// Typically one LinearUI context would create and maintain one of this.
// You may want to create your own instance of you try to LinearDrawList completely without LinearUI. In that case, watch out for future changes to this structure.
struct LINEARUI_API LinearDrawListSharedData
{
    LinearVec2          TexUvWhitePixel;            // UV of white pixel in the atlas (== FontAtlas->TexUvWhitePixel)
    const LinearVec4*   TexUvLines;                 // UV of anti-aliased lines in the atlas (== FontAtlas->TexUvLines)
    LinearFontAtlas*    FontAtlas;                  // Current font atlas
    LinearFont*         Font;                       // Current font (used for simplified AddText overload)
    float           FontSize;                   // Current font size (used for for simplified AddText overload)
    float           FontScale;                  // Current font scale (== FontSize / Font->FontSize)
    float           CurveTessellationTol;       // Tessellation tolerance when using PathBezierCurveTo()
    float           CircleTessellationMaxError; // Number of circle segments to use per pixel of radius for AddCircle() etc
    float           InitialFringeScale;         // Initial scale to apply to AA fringe
    LinearDrawListFlags InitialFlags;               // Initial flags at the beginning of the frame (it is possible to alter flags on a per-drawlist basis afterwards)
    LinearVec4          ClipRectFullscreen;         // Value for PushClipRectFullscreen()
    LinearVector<LinearVec2> TempBuffer;                // Temporary write buffer
    LinearVector<LinearDrawList*> DrawLists;            // All draw lists associated to this LinearDrawListSharedData
    LinearUIContext*   Context;                    // [OPTIONAL] Link to Dear LinearUI context. 99% of LinearDrawList/LinearFontAtlas can function without an LinearUI context, but this facilitate handling one legacy edge case.

    // Lookup tables
    LinearVec2          ArcFastVtx[LINEARUI_DRAWLIST_ARCFAST_TABLE_SIZE]; // Sample points on the quarter of the circle.
    float           ArcFastRadiusCutoff;                        // Cutoff radius after which arc drawing will fallback to slower PathArcTo()
    LinearU8            CircleSegmentCounts[64];    // Precomputed segment count for given radius before we calculate it dynamically (to avoid calculation overhead)

    LinearDrawListSharedData();
    ~LinearDrawListSharedData();
    void SetCircleTessellationMaxError(float max_error);
};

struct LinearDrawDataBuilder
{
    LinearVector<LinearDrawList*>*  Layers[2];      // Pointers to global layers for: regular, tooltip. LayersP[0] is owned by DrawData.
    LinearVector<LinearDrawList*>   LayerData1;

    LinearDrawDataBuilder()                     { memset((void*)this, 0, sizeof(*this)); }
};

struct LinearFontStackData
{
    LinearFont*     Font;
    float       FontSizeBeforeScaling;      // ~~ style.FontSizeBase
    float       FontSizeAfterScaling;       // ~~ g.FontSize
};

//-----------------------------------------------------------------------------
// [SECTION] Style support
//-----------------------------------------------------------------------------

struct LinearUIStyleVarInfo
{
    LinearU32           Count : 8;      // 1+
    LinearUIDataType   DataType : 8;
    LinearU32           Offset : 16;    // Offset in parent structure
    void* GetVarPtr(void* parent) const { return (void*)((unsigned char*)parent + Offset); }
};

// Stacked color modifier, backup of modified data so we can restore it
struct LinearUIColorMod
{
    LinearUICol        Col;
    LinearVec4          BackupValue;
};

// Stacked style modifier, backup of modified data so we can restore it. Data type inferred from the variable.
struct LinearUIStyleMod
{
    LinearUIStyleVar   VarIdx;
    union           { int BackupInt[2]; float BackupFloat[2]; };
    LinearUIStyleMod(LinearUIStyleVar idx, int v)     { VarIdx = idx; BackupInt[0] = v; }
    LinearUIStyleMod(LinearUIStyleVar idx, float v)   { VarIdx = idx; BackupFloat[0] = v; }
    LinearUIStyleMod(LinearUIStyleVar idx, LinearVec2 v)  { VarIdx = idx; BackupFloat[0] = v.x; BackupFloat[1] = v.y; }
};

//-----------------------------------------------------------------------------
// [SECTION] Data types support
//-----------------------------------------------------------------------------

struct LinearUIDataTypeStorage
{
    LinearU8        Data[8];        // Opaque storage to fit any data up to LinearUIDataType_COUNT
};

// Type information associated to one LinearUIDataType. Retrieve with DataTypeGetInfo().
struct LinearUIDataTypeInfo
{
    size_t      Size;           // Size in bytes
    const char* Name;           // Short descriptive name for the type, for debugging
    const char* PrintFmt;       // Default printf format for the type
    const char* ScanFmt;        // Default scanf format for the type
};

// Extend LinearUIDataType_
enum LinearUIDataTypePrivate_
{
    LinearUIDataType_Pointer = LinearUIDataType_COUNT,
    LinearUIDataType_ID,
};

//-----------------------------------------------------------------------------
// [SECTION] Widgets support: flags, enums, data structures
//-----------------------------------------------------------------------------

// Extend LinearUIItemFlags
// - input: PushItemFlag() manipulates g.CurrentItemFlags, g.NextItemData.ItemFlags, ItemAdd() calls may add extra flags too.
// - output: stored in g.LastItemData.ItemFlags
enum LinearUIItemFlagsPrivate_
{
    // Controlled by user
    LinearUIItemFlags_ReadOnly                 = 1 << 11, // false     // [ALPHA] Allow hovering interactions but underlying value is not changed.
    LinearUIItemFlags_MixedValue               = 1 << 12, // false     // [BETA] Represent a mixed/indeterminate value, generally multi-selection where values differ. Currently only supported by Checkbox() (later should support all sorts of widgets)
    LinearUIItemFlags_NoWindowHoverableCheck   = 1 << 13, // false     // Disable hoverable check in ItemHoverable()
    LinearUIItemFlags_AllowOverlap             = 1 << 14, // false     // Allow being overlapped by another widget. Not-hovered to Hovered transition deferred by a frame.
    LinearUIItemFlags_NoNavDisableMouseHover   = 1 << 15, // false     // Nav keyboard/gamepad mode doesn't disable hover highlight (behave as if NavHighlightItemUnderNav==false).
    LinearUIItemFlags_NoMarkEdited             = 1 << 16, // false     // Skip calling MarkItemEdited()
    LinearUIItemFlags_NoFocus                  = 1 << 17, // false     // [EXPERIMENTAL: Not very well specced] Clicking doesn't take focus. Automatically sets LinearUIButtonFlags_NoFocus + LinearUIButtonFlags_NoNavFocus in ButtonBehavior().

    // Controlled by widget code
    LinearUIItemFlags_Inputable                = 1 << 20, // false     // [WIP] Auto-activate input mode when tab focused. Currently only used and supported by a few items before it becomes a generic feature.
    LinearUIItemFlags_HasSelectionUserData     = 1 << 21, // false     // Set by SetNextItemSelectionUserData()
    LinearUIItemFlags_IsMultiSelect            = 1 << 22, // false     // Set by SetNextItemSelectionUserData()

    LinearUIItemFlags_Default_                 = LinearUIItemFlags_AutoClosePopups,    // Please don't change, use PushItemFlag() instead.

    // Obsolete
    //LinearUIItemFlags_SelectableDontClosePopup = !LinearUIItemFlags_AutoClosePopups, // Can't have a redirect as we inverted the behavior
};

// Status flags for an already submitted item
// - output: stored in g.LastItemData.StatusFlags
enum LinearUIItemStatusFlags_
{
    LinearUIItemStatusFlags_None               = 0,
    LinearUIItemStatusFlags_HoveredRect        = 1 << 0,   // Mouse position is within item rectangle (does NOT mean that the window is in correct z-order and can be hovered!, this is only one part of the most-common IsItemHovered test)
    LinearUIItemStatusFlags_HasDisplayRect     = 1 << 1,   // g.LastItemData.DisplayRect is valid
    LinearUIItemStatusFlags_Edited             = 1 << 2,   // Value exposed by item was edited in the current frame (should match the bool return value of most widgets)
    LinearUIItemStatusFlags_ToggledSelection   = 1 << 3,   // Set when Selectable(), TreeNode() reports toggling a selection. We can't report "Selected", only state changes, in order to easily handle clipping with less issues.
    LinearUIItemStatusFlags_ToggledOpen        = 1 << 4,   // Set when TreeNode() reports toggling their open state.
    LinearUIItemStatusFlags_HasDeactivated     = 1 << 5,   // Set if the widget/group is able to provide data for the LinearUIItemStatusFlags_Deactivated flag.
    LinearUIItemStatusFlags_Deactivated        = 1 << 6,   // Only valid if LinearUIItemStatusFlags_HasDeactivated is set.
    LinearUIItemStatusFlags_HoveredWindow      = 1 << 7,   // Override the HoveredWindow test to allow cross-window hover testing.
    LinearUIItemStatusFlags_Visible            = 1 << 8,   // [WIP] Set when item is overlapping the current clipping rectangle (Used internally. Please don't use yet: API/system will change as we refactor Itemadd()).
    LinearUIItemStatusFlags_HasClipRect        = 1 << 9,   // g.LastItemData.ClipRect is valid.
    LinearUIItemStatusFlags_HasShortcut        = 1 << 10,  // g.LastItemData.Shortcut valid. Set by SetNextItemShortcut() -> ItemAdd().
    //LinearUIItemStatusFlags_FocusedByTabbing = 1 << 8,   // Removed IN 1.90.1 (Dec 2023). The trigger is part of g.NavActivateId. See commit 54c1bdeceb.
    LinearUIItemStatusFlags_EditedInternal     = 1 << 11,  // Similar to LinearUIItemStatusFlags_Edited but bypassing LinearUIItemFlags_NoMarkEdited.

    // Additional status + semantic for LinearUITestEngine
#ifdef LINEARUI_ENABLE_TEST_ENGINE
    LinearUIItemStatusFlags_Openable           = 1 << 20,  // Item is an openable (e.g. TreeNode)
    LinearUIItemStatusFlags_Opened             = 1 << 21,  // Opened status
    LinearUIItemStatusFlags_Checkable          = 1 << 22,  // Item is a checkable (e.g. CheckBox, MenuItem)
    LinearUIItemStatusFlags_Checked            = 1 << 23,  // Checked status
    LinearUIItemStatusFlags_Inputable          = 1 << 24,  // Item is a text-inputable (e.g. InputText, SliderXXX, DragXXX)
#endif
};

// Extend LinearUIHoveredFlags_
enum LinearUIHoveredFlagsPrivate_
{
    LinearUIHoveredFlags_DelayMask_                    = LinearUIHoveredFlags_DelayNone | LinearUIHoveredFlags_DelayShort | LinearUIHoveredFlags_DelayNormal | LinearUIHoveredFlags_NoSharedDelay,
    LinearUIHoveredFlags_AllowedMaskForIsWindowHovered = LinearUIHoveredFlags_ChildWindows | LinearUIHoveredFlags_RootWindow | LinearUIHoveredFlags_AnyWindow | LinearUIHoveredFlags_NoPopupHierarchy | LinearUIHoveredFlags_DockHierarchy | LinearUIHoveredFlags_AllowWhenBlockedByPopup | LinearUIHoveredFlags_AllowWhenBlockedByActiveItem | LinearUIHoveredFlags_ForTooltip | LinearUIHoveredFlags_Stationary,
    LinearUIHoveredFlags_AllowedMaskForIsItemHovered   = LinearUIHoveredFlags_AllowWhenBlockedByPopup | LinearUIHoveredFlags_AllowWhenBlockedByActiveItem | LinearUIHoveredFlags_AllowWhenOverlapped | LinearUIHoveredFlags_AllowWhenDisabled | LinearUIHoveredFlags_NoNavOverride | LinearUIHoveredFlags_ForTooltip | LinearUIHoveredFlags_Stationary | LinearUIHoveredFlags_DelayMask_,
};

// Extend LinearUIInputTextFlags_
enum LinearUIInputTextFlagsPrivate_
{
    // [Internal]
    LinearUIInputTextFlags_Multiline           = 1 << 26,  // For internal use by InputTextMultiline()
    LinearUIInputTextFlags_TempInput           = 1 << 27,  // For internal use by TempInputText(), will skip calling ItemAdd(). Require bounding-box to strictly match.
    LinearUIInputTextFlags_LocalizeDecimalPoint= 1 << 28,  // For internal use by InputScalar() and TempInputScalar()
};

// Extend LinearUIButtonFlags_
enum LinearUIButtonFlagsPrivate_
{
    LinearUIButtonFlags_PressedOnClick         = 1 << 4,   // return true on click (mouse down event)
    LinearUIButtonFlags_PressedOnClickRelease  = 1 << 5,   // [Default] return true on click + release on same item <-- this is what the majority of Button are using
    LinearUIButtonFlags_PressedOnClickReleaseAnywhere = 1 << 6, // return true on click + release even if the release event is not done while hovering the item
    LinearUIButtonFlags_PressedOnRelease       = 1 << 7,   // return true on release (default requires click+release). Prior to 2026/03/20 this implied LinearUIButtonFlags_NoHoldingActiveId but they are separate now.
    LinearUIButtonFlags_PressedOnDoubleClick   = 1 << 8,   // return true on double-click (default requires click+release)
    LinearUIButtonFlags_PressedOnDragDropHold  = 1 << 9,   // return true when held into while we are drag and dropping another item (used by e.g. tree nodes, collapsing headers)
    //LinearUIButtonFlags_Repeat               = 1 << 10,  // hold to repeat -> use LinearUIItemFlags_ButtonRepeat instead.
    LinearUIButtonFlags_FlattenChildren        = 1 << 11,  // allow interactions even if a child window is overlapping
    //LinearUIButtonFlags_DontClosePopups      = 1 << 13,  // disable automatically closing parent popup on press
    //LinearUIButtonFlags_Disabled             = 1 << 14,  // disable interactions -> use BeginDisabled() or LinearUIItemFlags_Disabled
    LinearUIButtonFlags_AlignTextBaseLine      = 1 << 15,  // vertically align button to match text baseline - ButtonEx() only // FIXME: Should be removed and handled by SmallButton(), not possible currently because of DC.CursorPosPrevLine
    LinearUIButtonFlags_NoKeyModsAllowed       = 1 << 16,  // disable mouse interaction if a key modifier is held
    LinearUIButtonFlags_NoHoldingActiveId      = 1 << 17,  // don't set ActiveId while holding the mouse (LinearUIButtonFlags_PressedOnClick only)
    LinearUIButtonFlags_NoNavFocus             = 1 << 18,  // don't override navigation focus when activated (FIXME: this is essentially used every time an item uses LinearUIItemFlags_NoNav, but because legacy specs don't requires LastItemData to be set ButtonBehavior(), we can't poll g.LastItemData.ItemFlags)
    LinearUIButtonFlags_NoHoveredOnFocus       = 1 << 19,  // don't report as hovered when nav focus is on this item
    LinearUIButtonFlags_NoSetKeyOwner          = 1 << 20,  // don't set key/input owner on the initial click (note: mouse buttons are keys! often, the key in question will be LinearUIKey_MouseLeft!)
    LinearUIButtonFlags_NoTestKeyOwner         = 1 << 21,  // don't test key/input owner when polling the key (note: mouse buttons are keys! often, the key in question will be LinearUIKey_MouseLeft!)
    LinearUIButtonFlags_NoFocus                = 1 << 22,  // [EXPERIMENTAL: Not very well specced]. Don't focus parent window when clicking.
    LinearUIButtonFlags_PressedOnMask_         = LinearUIButtonFlags_PressedOnClick | LinearUIButtonFlags_PressedOnClickRelease | LinearUIButtonFlags_PressedOnClickReleaseAnywhere | LinearUIButtonFlags_PressedOnRelease | LinearUIButtonFlags_PressedOnDoubleClick | LinearUIButtonFlags_PressedOnDragDropHold,
    LinearUIButtonFlags_PressedOnDefault_      = LinearUIButtonFlags_PressedOnClickRelease,
    //LinearUIButtonFlags_NoKeyModifiers       = LinearUIButtonFlags_NoKeyModsAllowed, // Renamed in 1.91.4
};

// Extend LinearUIComboFlags_
enum LinearUIComboFlagsPrivate_
{
    LinearUIComboFlags_CustomPreview           = 1 << 20,  // enable BeginComboPreview()
};

// Extend LinearUISliderFlags_
enum LinearUISliderFlagsPrivate_
{
    LinearUISliderFlags_Vertical               = 1 << 20,  // Should this slider be orientated vertically?
    LinearUISliderFlags_ReadOnly               = 1 << 21,  // Consider using g.NextItemData.ItemFlags |= LinearUIItemFlags_ReadOnly instead.
};

// Extend LinearUISelectableFlags_
enum LinearUISelectableFlagsPrivate_
{
    // NB: need to be in sync with last value of LinearUISelectableFlags_
    LinearUISelectableFlags_NoHoldingActiveID      = 1 << 20,
    LinearUISelectableFlags_SelectOnClick          = 1 << 22,  // Override button behavior to react on Click (default is Click+Release)
    LinearUISelectableFlags_SelectOnRelease        = 1 << 23,  // Override button behavior to react on Release (default is Click+Release)
    LinearUISelectableFlags_SpanAvailWidth         = 1 << 24,  // Span all avail width even if we declared less for layout purpose. FIXME: We may be able to remove this (added in 6251d379, 2bcafc86 for menus)
    LinearUISelectableFlags_SetNavIdOnHover        = 1 << 25,  // Set Nav/Focus ID on mouse hover (used by MenuItem)
    LinearUISelectableFlags_NoPadWithHalfSpacing   = 1 << 26,  // Disable padding each side with ItemSpacing * 0.5f
    LinearUISelectableFlags_NoSetKeyOwner          = 1 << 27,  // Don't set key/input owner on the initial click (note: mouse buttons are keys! often, the key in question will be LinearUIKey_MouseLeft!)
};

// Extend LinearUITreeNodeFlags_
enum LinearUITreeNodeFlagsPrivate_
{
    LinearUITreeNodeFlags_NoNavFocus                 = 1 << 27,// Don't claim nav focus when interacting with this item (#8551)
    LinearUITreeNodeFlags_ClipLabelForTrailingButton = 1 << 28,// FIXME-WIP: Hard-coded for CollapsingHeader()
    LinearUITreeNodeFlags_UpsideDownArrow            = 1 << 29,// FIXME-WIP: Turn Down arrow into an Up arrow, for reversed trees (#6517)
    LinearUITreeNodeFlags_OpenOnMask_                = LinearUITreeNodeFlags_OpenOnDoubleClick | LinearUITreeNodeFlags_OpenOnArrow,
    LinearUITreeNodeFlags_DrawLinesMask_             = LinearUITreeNodeFlags_DrawLinesNone | LinearUITreeNodeFlags_DrawLinesFull | LinearUITreeNodeFlags_DrawLinesToNodes,
};

enum LinearUISeparatorFlags_
{
    LinearUISeparatorFlags_None                    = 0,
    LinearUISeparatorFlags_Horizontal              = 1 << 0,   // Axis default to current layout type, so generally Horizontal unless e.g. in a menu bar
    LinearUISeparatorFlags_Vertical                = 1 << 1,
    LinearUISeparatorFlags_SpanAllColumns          = 1 << 2,   // Make separator cover all columns of a legacy Columns() set.
};

// Flags for FocusWindow(). This is not called LinearUIFocusFlags to avoid confusion with public-facing LinearUIFocusedFlags.
// FIXME: Once we finishing replacing more uses of GetTopMostPopupModal()+IsWindowWithinBeginStackOf()
// and FindBlockingModal() with this, we may want to change the flag to be opt-out instead of opt-in.
enum LinearUIFocusRequestFlags_
{
    LinearUIFocusRequestFlags_None                 = 0,
    LinearUIFocusRequestFlags_RestoreFocusedChild  = 1 << 0,   // Find last focused child (if any) and focus it instead.
    LinearUIFocusRequestFlags_UnlessBelowModal     = 1 << 1,   // Do not set focus if the window is below a modal.
};

enum LinearUITextFlags_
{
    LinearUITextFlags_None                         = 0,
    LinearUITextFlags_NoWidthForLargeClippedText   = 1 << 0,
};

enum LinearUITooltipFlags_
{
    LinearUITooltipFlags_None                      = 0,
    LinearUITooltipFlags_OverridePrevious          = 1 << 1,   // Clear/ignore previously submitted tooltip (defaults to append)
};

// FIXME: this is in development, not exposed/functional as a generic feature yet.
// Horizontal/Vertical enums are fixed to 0/1 so they may be used to index LinearVec2
enum LinearUILayoutType_
{
    LinearUILayoutType_Horizontal = 0,
    LinearUILayoutType_Vertical = 1
};

// Flags for LogBegin() text capturing function
enum LinearUILogFlags_
{
    LinearUILogFlags_None = 0,

    LinearUILogFlags_OutputTTY         = 1 << 0,
    LinearUILogFlags_OutputFile        = 1 << 1,
    LinearUILogFlags_OutputBuffer      = 1 << 2,
    LinearUILogFlags_OutputClipboard   = 1 << 3,
    LinearUILogFlags_OutputMask_       = LinearUILogFlags_OutputTTY | LinearUILogFlags_OutputFile | LinearUILogFlags_OutputBuffer | LinearUILogFlags_OutputClipboard,
};

// X/Y enums are fixed to 0/1 so they may be used to index LinearVec2
enum LinearUIAxis
{
    LinearUIAxis_None = -1,
    LinearUIAxis_X = 0,
    LinearUIAxis_Y = 1
};

enum LinearUIPlotType
{
    LinearUIPlotType_Lines,
    LinearUIPlotType_Histogram,
};

// Storage data for BeginComboPreview()/EndComboPreview()
struct LINEARUI_API LinearUIComboPreviewData
{
    LinearRect          PreviewRect;
    LinearVec2          BackupCursorPos;
    LinearVec2          BackupCursorMaxPos;
    LinearVec2          BackupCursorPosPrevLine;
    float           BackupPrevLineTextBaseOffset;
    LinearUILayoutType BackupLayout;

    LinearUIComboPreviewData() { memset((void*)this, 0, sizeof(*this)); }
};

// Stacked storage data for BeginGroup()/EndGroup()
struct LINEARUI_API LinearUIGroupData
{
    LinearUIID     WindowID;
    LinearVec2      BackupCursorPos;
    LinearVec2      BackupCursorMaxPos;
    LinearVec2      BackupCursorPosPrevLine;
    LinearVec1      BackupIndent;
    LinearVec1      BackupGroupOffset;
    LinearVec2      BackupCurrLineSize;
    float       BackupCurrLineTextBaseOffset;
    LinearUIID     BackupActiveIdIsAlive;
    bool        BackupActiveIdHasBeenEditedThisFrame;
    bool        BackupDeactivatedIdIsAlive;
    bool        BackupHoveredIdIsAlive;
    bool        BackupIsSameLine;
    bool        EmitItem;
};

// Simple column measurement, currently used for MenuItem() only.. This is very short-sighted/throw-away code and NOT a generic helper.
struct LINEARUI_API LinearUIMenuColumns
{
    LinearU32       TotalWidth;
    LinearU32       NextTotalWidth;
    LinearU16       Spacing;
    LinearU16       OffsetIcon;         // Always zero for now
    LinearU16       OffsetLabel;        // Offsets are locked in Update()
    LinearU16       OffsetShortcut;
    LinearU16       OffsetMark;
    LinearU16       Widths[4];          // Width of:   Icon, Label, Shortcut, Mark  (accumulators for current frame)

    LinearUIMenuColumns() { memset((void*)this, 0, sizeof(*this)); }
    void        Update(float spacing, bool window_reappearing);
    float       DeclColumns(float w_icon, float w_label, float w_shortcut, float w_mark);
    void        CalcNextTotalWidth(bool update_offsets);
};

// Internal temporary state for deactivating InputText() instances.
// Store as part of LinearUIDeactivatedItemData?
struct LINEARUI_API LinearUIInputTextDeactivatedState
{
    LinearUIID            ID;              // widget id owning the text state (which just got deactivated)
    LinearVector<char>     TextA;           // text buffer

    LinearUIInputTextDeactivatedState()    { memset((void*)this, 0, sizeof(*this)); }
    void    ClearFreeMemory()           { ID = 0; TextA.clear(); }
};

// Forward declare linearuistb_textedit.h structure + make its main configuration define accessible
#undef IMSTB_TEXTEDIT_STRING
#undef IMSTB_TEXTEDIT_CHARTYPE
#define IMSTB_TEXTEDIT_STRING             LinearUIInputTextState
#define IMSTB_TEXTEDIT_CHARTYPE           char
#define IMSTB_TEXTEDIT_GETWIDTH_NEWLINE   (-1.0f)
#define IMSTB_TEXTEDIT_UNDOSTATECOUNT     99
#define IMSTB_TEXTEDIT_UNDOCHARCOUNT      999
namespace LinearStb { struct LINEARSTB_TexteditState; }
typedef LinearStb::LINEARSTB_TexteditState LinearStbTexteditState;

// Internal state of the currently focused/edited text input box
// For a given item ID, access with LinearUI::GetInputTextState()
struct LINEARUI_API LinearUIInputTextState
{
    LinearUIContext*           Ctx;                    // parent UI context (needs to be set explicitly by parent).
    LinearStbTexteditState*     Stb;                    // State for linearstb_textedit.h
    LinearUIInputTextFlags     Flags;                  // copy of InputText() flags. may be used to check if e.g. LinearUIInputTextFlags_Password is set.
    LinearUIID                 ID;                     // widget id owning the text state
    int                     TextLen;                // UTF-8 length of the string in TextA (in bytes)
    const char*             TextSrc;                // == TextA.Data unless read-only, in which case == buf passed to InputText(). For _ReadOnly fields, pointer will be null outside the InputText() call.
    LinearVector<char>          TextA;                  // main UTF8 buffer. TextA.Size is a buffer size! Should always be >= buf_size passed by user (and of course >= CurLenA + 1).
    LinearVector<char>          TextToRevertTo;         // value to revert to when pressing Escape = backup of end-user buffer at the time of focus (in UTF-8, unaltered)
    LinearVector<char>          CallbackTextBackup;     // temporary storage for callback to support automatic reconcile of undo-stack
    int                     BufCapacity;            // end-user buffer capacity (include zero terminator)
    LinearVec2                  Scroll;                 // horizontal offset (managed manually) + vertical scrolling (pulled from child window's own Scroll.y)
    int                     LineCount;              // last line count (solely for debugging)
    float                   WrapWidth;              // word-wrapping width
    float                   CursorAnim;             // timer for cursor blink, reset on every user action so the cursor reappears immediately
    bool                    CursorFollow;           // set when we want scrolling to follow the current cursor position (not always!)
    bool                    CursorCenterY;          // set when we want scrolling to be centered over the cursor position (while resizing a word-wrapping field)
    bool                    SelectedAllMouseLock;   // after a double-click to select all, we ignore further mouse drags to update selection
    bool                    EditedBefore;           // edited since activated
    bool                    EditedThisFrame;        // edited this frame
    bool                    WantReloadUserBuf;      // force a reload of user buf so it may be modified externally. may be automatic in future version.
    LinearS8                    LastMoveDirectionLR;    // LinearUIDir_Left or LinearUIDir_Right. track last movement direction so when cursor cross over a word-wrapping boundaries we can display it on either line depending on last move.s
    int                     ReloadSelectionStart;
    int                     ReloadSelectionEnd;

    LinearUIInputTextState();
    ~LinearUIInputTextState();
    void        ClearText()                 { TextLen = 0; TextA[0] = 0; CursorClamp(); }
    void        ClearFreeMemory()           { TextA.clear(); TextToRevertTo.clear(); }
    void        OnKeyPressed(int key);      // Cannot be inline because we call in code in linearstb_textedit.h implementation
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
    //   if (LinearUIInputTextState* state = LinearUI::GetInputTextState(id)) // id may be LinearUI::GetItemID() is last item
    //       state->ReloadUserBufAndSelectAll();
    void        ReloadUserBufAndSelectAll();
    void        ReloadUserBufAndKeepSelection();
    void        ReloadUserBufAndMoveToEnd();
};

enum LinearUIWindowRefreshFlags_
{
    LinearUIWindowRefreshFlags_None                = 0,
    LinearUIWindowRefreshFlags_TryToAvoidRefresh   = 1 << 0,   // [EXPERIMENTAL] Try to keep existing contents, USER MUST NOT HONOR BEGIN() RETURNING FALSE AND NOT APPEND.
    LinearUIWindowRefreshFlags_RefreshOnHover      = 1 << 1,   // [EXPERIMENTAL] Always refresh on hover
    LinearUIWindowRefreshFlags_RefreshOnFocus      = 1 << 2,   // [EXPERIMENTAL] Always refresh on focus
    // Refresh policy/frequency, Load Balancing etc.
};

enum LinearUIWindowBgClickFlags_
{
    LinearUIWindowBgClickFlags_None                = 0,
    LinearUIWindowBgClickFlags_Move                = 1 << 0,   // Click on bg/void + drag to move window. Cleared by default when using io.ConfigWindowsMoveFromTitleBarOnly.
};

enum LinearUINextWindowDataFlags_
{
    LinearUINextWindowDataFlags_None               = 0,
    LinearUINextWindowDataFlags_HasPos             = 1 << 0,
    LinearUINextWindowDataFlags_HasSize            = 1 << 1,
    LinearUINextWindowDataFlags_HasContentSize     = 1 << 2,
    LinearUINextWindowDataFlags_HasCollapsed       = 1 << 3,
    LinearUINextWindowDataFlags_HasSizeConstraint  = 1 << 4,
    LinearUINextWindowDataFlags_HasFocus           = 1 << 5,
    LinearUINextWindowDataFlags_HasBgAlpha         = 1 << 6,
    LinearUINextWindowDataFlags_HasScroll          = 1 << 7,
    LinearUINextWindowDataFlags_HasWindowFlags     = 1 << 8,
    LinearUINextWindowDataFlags_HasChildFlags      = 1 << 9,
    LinearUINextWindowDataFlags_HasRefreshPolicy   = 1 << 10,
    LinearUINextWindowDataFlags_HasViewport        = 1 << 11,
    LinearUINextWindowDataFlags_HasDock            = 1 << 12,
    LinearUINextWindowDataFlags_HasWindowClass     = 1 << 13,
};

// Storage for SetNexWindow** functions
struct LinearUINextWindowData
{
    LinearUINextWindowDataFlags    HasFlags;

    // Members below are NOT cleared. Always rely on HasFlags.
    LinearUICond                   PosCond;
    LinearUICond                   SizeCond;
    LinearUICond                   CollapsedCond;
    LinearUICond                   DockCond;
    LinearVec2                      PosVal;
    LinearVec2                      PosPivotVal;
    LinearVec2                      SizeVal;
    LinearVec2                      ContentSizeVal;
    LinearVec2                      ScrollVal;
    LinearUIWindowFlags            WindowFlags;            // Only honored by BeginTable()
    LinearUIChildFlags             ChildFlags;
    bool                        PosUndock;
    bool                        CollapsedVal;
    LinearRect                      SizeConstraintRect;
    LinearUISizeCallback           SizeCallback;
    void*                       SizeCallbackUserData;
    float                       BgAlphaVal;             // Override background alpha
    LinearUIID                     ViewportId;
    LinearUIID                     DockId;
    LinearUIWindowClass            WindowClass;
    LinearVec2                      MenuBarOffsetMinVal;    // (Always on) This is not exposed publicly, so we don't clear it and it doesn't have a corresponding flag (could we? for consistency?)
    LinearUIWindowRefreshFlags     RefreshFlagsVal;

    LinearUINextWindowData()       { memset((void*)this, 0, sizeof(*this)); }
    inline void ClearFlags()    { HasFlags = LinearUINextWindowDataFlags_None; }
};

enum LinearUINextItemDataFlags_
{
    LinearUINextItemDataFlags_None             = 0,
    LinearUINextItemDataFlags_HasWidth         = 1 << 0,
    LinearUINextItemDataFlags_HasOpen          = 1 << 1,
    LinearUINextItemDataFlags_HasShortcut      = 1 << 2,
    LinearUINextItemDataFlags_HasRefVal        = 1 << 3,
    LinearUINextItemDataFlags_HasStorageID     = 1 << 4,
    LinearUINextItemDataFlags_HasColorMarker   = 1 << 5,
};

struct LinearUINextItemData
{
    LinearUINextItemDataFlags      HasFlags;           // Called HasFlags instead of Flags to avoid mistaking this
    LinearUIItemFlags              ItemFlags;          // Currently only tested/used for LinearUIItemFlags_AllowOverlap and LinearUIItemFlags_HasSelectionUserData.

    // Members below are NOT cleared by ItemAdd() meaning they are still valid during e.g. NavProcessItem(). Always rely on HasFlags.
    LinearUIID                     FocusScopeId;       // Set by SetNextItemSelectionUserData()
    LinearUISelectionUserData      SelectionUserData;  // Set by SetNextItemSelectionUserData() (note that NULL/0 is a valid value, we use -1 == LinearUISelectionUserData_Invalid to mark invalid values)
    float                       Width;              // Set by SetNextItemWidth()
    LinearUIKeyChord               Shortcut;           // Set by SetNextItemShortcut()
    LinearUIInputFlags             ShortcutFlags;      // Set by SetNextItemShortcut()
    bool                        OpenVal;            // Set by SetNextItemOpen()
    LinearU8                        OpenCond;           // Set by SetNextItemOpen()
    LinearUIDataTypeStorage        RefVal;             // Not exposed yet, for LinearUIInputTextFlags_ParseEmptyAsRefVal
    LinearUIID                     StorageId;          // Set by SetNextItemStorageID()
    LinearU32                       ColorMarker;        // Set by SetNextItemColorMarker(). Not exposed yet, supported by DragScalar,SliderScalar and for LinearUISliderFlags_ColorMarkers.

    LinearUINextItemData()         { memset((void*)this, 0, sizeof(*this)); SelectionUserData = -1; }
    inline void ClearFlags()    { HasFlags = LinearUINextItemDataFlags_None; ItemFlags = LinearUIItemFlags_None; } // Also cleared manually by ItemAdd()!
};

// Status storage for the last submitted item
struct LinearUILastItemData
{
    LinearUIID                 ID;
    LinearUIItemFlags          ItemFlags;          // See LinearUIItemFlags_ (called 'InFlags' before v1.91.4).
    LinearUIItemStatusFlags    StatusFlags;        // See LinearUIItemStatusFlags_
    LinearRect                  Rect;               // Full rectangle
    LinearRect                  NavRect;            // Navigation scoring rectangle (not displayed)
    // Rarely used fields are not explicitly cleared, only valid when the corresponding LinearUIItemStatusFlags are set.
    LinearRect                  DisplayRect;        // Display rectangle. ONLY VALID IF (StatusFlags & LinearUIItemStatusFlags_HasDisplayRect) is set.
    LinearRect                  ClipRect;           // Clip rectangle at the time of submitting item. ONLY VALID IF (StatusFlags & LinearUIItemStatusFlags_HasClipRect) is set..
    LinearUIKeyChord           Shortcut;           // Shortcut at the time of submitting item. ONLY VALID IF (StatusFlags & LinearUIItemStatusFlags_HasShortcut) is set..

    LinearUILastItemData()     { memset((void*)this, 0, sizeof(*this)); }
};

// Store data emitted by TreeNode() for usage by TreePop()
// - To implement LinearUITreeNodeFlags_NavLeftJumpsToParent: store the minimum amount of data
//   which we can't infer in TreePop(), to perform the equivalent of NavApplyItemToResult().
//   Only stored when the node is a potential candidate for landing on a Left arrow jump.
struct LinearUITreeNodeStackData
{
    LinearUIID                 ID;
    LinearUITreeNodeFlags      TreeFlags;
    LinearUIItemFlags          ItemFlags;      // Used for nav landing
    LinearRect                  NavRect;        // Used for nav landing
    float                   DrawLinesX1;
    float                   DrawLinesToNodesY2;
    LinearUITableColumnIdx     DrawLinesTableColumn;
};

// sizeof() = 20
struct LINEARUI_API LinearUIErrorRecoveryState
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

    LinearUIErrorRecoveryState() { memset((void*)this, 0, sizeof(*this)); }
};

// Data saved for each window pushed into the stack
struct LinearUIWindowStackData
{
    LinearUIWindow*            Window;
    LinearUILastItemData       ParentLastItemDataBackup;
    LinearUIErrorRecoveryState StackSizesInBegin;          // Store size of various stacks for asserting
    bool                    DisabledOverrideReenable;   // Non-child window override disabled flag
    float                   DisabledOverrideReenableAlphaBackup;
};

struct LinearUIShrinkWidthItem
{
    int         Index;
    float       Width;
    float       InitialWidth;
};

struct LinearUIPtrOrIndex
{
    void*       Ptr;            // Either field can be set, not both. e.g. Dock node tab bars are loose while BeginTabBar() ones are in a pool.
    int         Index;          // Usually index in a main pool.

    LinearUIPtrOrIndex(void* ptr)  { Ptr = ptr; Index = -1; }
    LinearUIPtrOrIndex(int index)  { Ptr = NULL; Index = index; }
};

// Data used by IsItemDeactivated()/IsItemDeactivatedAfterEdit() functions
// Also see LinearUIInputTextDeactivatedState which is an extension for this for InputText()
struct LinearUIDeactivatedItemData
{
    LinearUIID     ID;
    int         ElapseFrame;
    bool        HasBeenEditedBefore;
    bool        IsAlive;
};

//-----------------------------------------------------------------------------
// [SECTION] Popup support
//-----------------------------------------------------------------------------

enum LinearUIPopupPositionPolicy
{
    LinearUIPopupPositionPolicy_Default,
    LinearUIPopupPositionPolicy_ComboBox,
    LinearUIPopupPositionPolicy_Tooltip,
};

// Storage for popup stacks (g.OpenPopupStack and g.BeginPopupStack)
struct LinearUIPopupData
{
    LinearUIID             PopupId;        // Set on OpenPopup()
    LinearUIWindow*        Window;         // Resolved on BeginPopup() - may stay unresolved if user never calls OpenPopup()
    LinearUIWindow*        RestoreNavWindow;// Set on OpenPopup(), a NavWindow that will be restored on popup close
    int                 ParentNavLayer; // Resolved on BeginPopup(). Actually a LinearUINavLayer type (declared down below), initialized to -1 which is not part of an enum, but serves well-enough as "not any of layers" value
    int                 OpenFrameCount; // Set on OpenPopup()
    LinearUIID             OpenParentId;   // Set on OpenPopup(), we need this to differentiate multiple menu sets from each others (e.g. inside menu bar vs loose menu items)
    LinearVec2              OpenPopupPos;   // Set on OpenPopup(), preferred popup position (typically == OpenMousePos when using mouse)
    LinearVec2              OpenMousePos;   // Set on OpenPopup(), copy of mouse position at the time of opening popup

    LinearUIPopupData()    { memset((void*)this, 0, sizeof(*this)); ParentNavLayer = OpenFrameCount = -1; }
};

//-----------------------------------------------------------------------------
// [SECTION] Inputs support
//-----------------------------------------------------------------------------

// Bit array for named keys
typedef LinearBitArray<LinearUIKey_NamedKey_COUNT, -LinearUIKey_NamedKey_BEGIN>    LinearBitArrayForNamedKeys;

// [Internal] Key ranges
#define LinearUIKey_LegacyNativeKey_BEGIN  0
#define LinearUIKey_LegacyNativeKey_END    512
#define LinearUIKey_Keyboard_BEGIN         (LinearUIKey_NamedKey_BEGIN)
#define LinearUIKey_Keyboard_END           (LinearUIKey_GamepadStart)
#define LinearUIKey_Gamepad_BEGIN          (LinearUIKey_GamepadStart)
#define LinearUIKey_Gamepad_END            (LinearUIKey_GamepadRStickDown + 1)
#define LinearUIKey_Mouse_BEGIN            (LinearUIKey_MouseLeft)
#define LinearUIKey_Mouse_END              (LinearUIKey_MouseWheelY + 1)
#define LinearUIKey_Aliases_BEGIN          (LinearUIKey_Mouse_BEGIN)
#define LinearUIKey_Aliases_END            (LinearUIKey_Mouse_END)

// [Internal] Named shortcuts for Navigation
#define LinearUIKey_NavKeyboardTweakSlow   LinearUIMod_Ctrl
#define LinearUIKey_NavKeyboardTweakFast   LinearUIMod_Shift
#define LinearUIKey_NavGamepadTweakSlow    LinearUIKey_GamepadL1
#define LinearUIKey_NavGamepadTweakFast    LinearUIKey_GamepadR1
#define LinearUIKey_NavGamepadActivate     (g.IO.ConfigNavSwapGamepadButtons ? LinearUIKey_GamepadFaceRight : LinearUIKey_GamepadFaceDown)
#define LinearUIKey_NavGamepadCancel       (g.IO.ConfigNavSwapGamepadButtons ? LinearUIKey_GamepadFaceDown : LinearUIKey_GamepadFaceRight)
#define LinearUIKey_NavGamepadMenu         LinearUIKey_GamepadFaceLeft    // Toggle menu layer. Hold to enable Windowing.
#define LinearUIKey_NavGamepadContextMenu  LinearUIKey_GamepadFaceUp      // Open context menu (same as Shift+F10)

enum LinearUIInputEventType
{
    LinearUIInputEventType_None = 0,
    LinearUIInputEventType_MousePos,
    LinearUIInputEventType_MouseWheel,
    LinearUIInputEventType_MouseButton,
    LinearUIInputEventType_MouseViewport,
    LinearUIInputEventType_Key,
    LinearUIInputEventType_Text,
    LinearUIInputEventType_Focus,
    LinearUIInputEventType_COUNT
};

enum LinearUIInputSource : int
{
    LinearUIInputSource_None = 0,
    LinearUIInputSource_Mouse,         // Note: may be Mouse or TouchScreen or Pen. See io.MouseSource to distinguish them.
    LinearUIInputSource_Keyboard,
    LinearUIInputSource_Gamepad,
    LinearUIInputSource_COUNT
};

// FIXME: Structures in the union below need to be declared as anonymous unions appears to be an extension?
// Using LinearVec2() would fail on Clang 'union member 'MousePos' has a non-trivial default constructor'
struct LinearUIInputEventMousePos      { float PosX, PosY; LinearUIMouseSource MouseSource; };
struct LinearUIInputEventMouseWheel    { float WheelX, WheelY; LinearUIMouseSource MouseSource; };
struct LinearUIInputEventMouseButton   { int Button; bool Down; LinearUIMouseSource MouseSource; };
struct LinearUIInputEventMouseViewport { LinearUIID HoveredViewportID; };
struct LinearUIInputEventKey           { LinearUIKey Key; bool Down; float AnalogValue; };
struct LinearUIInputEventText          { unsigned int Char; };
struct LinearUIInputEventAppFocused    { bool Focused; };

struct LinearUIInputEvent
{
    LinearUIInputEventType             Type;
    LinearUIInputSource                Source;
    LinearU32                           EventId;        // Unique, sequential increasing integer to identify an event (if you need to correlate them to other data).
    union
    {
        LinearUIInputEventMousePos     MousePos;       // if Type == LinearUIInputEventType_MousePos
        LinearUIInputEventMouseWheel   MouseWheel;     // if Type == LinearUIInputEventType_MouseWheel
        LinearUIInputEventMouseButton  MouseButton;    // if Type == LinearUIInputEventType_MouseButton
        LinearUIInputEventMouseViewport MouseViewport; // if Type == LinearUIInputEventType_MouseViewport
        LinearUIInputEventKey          Key;            // if Type == LinearUIInputEventType_Key
        LinearUIInputEventText         Text;           // if Type == LinearUIInputEventType_Text
        LinearUIInputEventAppFocused   AppFocused;     // if Type == LinearUIInputEventType_Focus
    };
    bool                            AddedByTestEngine;

    LinearUIInputEvent() { memset((void*)this, 0, sizeof(*this)); }
};

// Input function taking an 'LinearUIID owner_id' argument defaults to (LinearUIKeyOwner_Any == 0) aka don't test ownership, which matches legacy behavior.
#define LinearUIKeyOwner_Any           ((LinearUIID)0)    // Accept key that have an owner, UNLESS a call to SetKeyOwner() explicitly used LinearUIInputFlags_LockThisFrame or LinearUIInputFlags_LockUntilRelease.
#define LinearUIKeyOwner_NoOwner       ((LinearUIID)-1)   // Require key to have no owner.
//#define LinearUIKeyOwner_None LinearUIKeyOwner_NoOwner  // We previously called this 'LinearUIKeyOwner_None' but it was inconsistent with our pattern that _None values == 0 and quite dangerous. Also using _NoOwner makes the IsKeyPressed() calls more explicit.

typedef LinearS16 LinearUIKeyRoutingIndex;

// Routing table entry (sizeof() == 16 bytes)
struct LinearUIKeyRoutingData
{
    LinearUIKeyRoutingIndex            NextEntryIndex;
    LinearU16                           Mods;               // Technically we'd only need 4-bits but for simplify we store LinearUIMod_ values which need 16-bits.
    LinearU16                           RoutingCurrScore;   // [DEBUG] For debug display
    LinearU16                           RoutingNextScore;   // Lower is better (0: perfect score)
    LinearUIID                         RoutingCurr;
    LinearUIID                         RoutingNext;

    LinearUIKeyRoutingData()           { NextEntryIndex = -1; Mods = 0; RoutingCurrScore = RoutingNextScore = 0; RoutingCurr = RoutingNext = LinearUIKeyOwner_NoOwner; }
};

// Routing table: maintain a desired owner for each possible key-chord (key + mods), and setup owner in NewFrame() when mods are matching.
// Stored in main context (1 instance)
struct LinearUIKeyRoutingTable
{
    LinearUIKeyRoutingIndex            Index[LinearUIKey_NamedKey_COUNT]; // Index of first entry in Entries[]
    LinearVector<LinearUIKeyRoutingData>   Entries;
    LinearVector<LinearUIKeyRoutingData>   EntriesNext;                    // Double-buffer to avoid reallocation (could use a shared buffer)

    LinearUIKeyRoutingTable()          { Clear(); }
    void Clear()                    { for (int n = 0; n < LINEARUI_COUNTOF(Index); n++) Index[n] = -1; Entries.clear(); EntriesNext.clear(); }
};

// This extends LinearUIKeyData but only for named keys (legacy keys don't support the new features)
// Stored in main context (1 per named key). In the future it might be merged into LinearUIKeyData.
struct LinearUIKeyOwnerData
{
    LinearUIID     OwnerCurr;
    LinearUIID     OwnerNext;
    bool        LockThisFrame;      // Reading this key requires explicit owner id (until end of frame). Set by LinearUIInputFlags_LockThisFrame.
    bool        LockUntilRelease;   // Reading this key requires explicit owner id (until key is released). Set by LinearUIInputFlags_LockUntilRelease. When this is true LockThisFrame is always true as well.

    LinearUIKeyOwnerData()             { OwnerCurr = OwnerNext = LinearUIKeyOwner_NoOwner; LockThisFrame = LockUntilRelease = false; }
};

// Extend LinearUIInputFlags_
// Flags for extended versions of IsKeyPressed(), IsMouseClicked(), Shortcut(), SetKeyOwner(), SetItemKeyOwner()
// Don't mistake with LinearUIInputTextFlags! (which is for LinearUI::InputText() function)
enum LinearUIInputFlagsPrivate_
{
    // Flags for IsKeyPressed(), IsKeyChordPressed(), IsMouseClicked(), Shortcut()
    // - Repeat mode: Repeat rate selection
    LinearUIInputFlags_RepeatRateDefault           = 1 << 1,   // Repeat rate: Regular (default)
    LinearUIInputFlags_RepeatRateNavMove           = 1 << 2,   // Repeat rate: Fast
    LinearUIInputFlags_RepeatRateNavTweak          = 1 << 3,   // Repeat rate: Faster
    // - Repeat mode: Specify when repeating key pressed can be interrupted.
    // - In theory LinearUIInputFlags_RepeatUntilOtherKeyPress may be a desirable default, but it would break too many behavior so everything is opt-in.
    LinearUIInputFlags_RepeatUntilRelease          = 1 << 4,   // Stop repeating when released (default for all functions except Shortcut). This only exists to allow overriding Shortcut() default behavior.
    LinearUIInputFlags_RepeatUntilKeyModsChange    = 1 << 5,   // Stop repeating when released OR if keyboard mods are changed (default for Shortcut)
    LinearUIInputFlags_RepeatUntilKeyModsChangeFromNone = 1 << 6,  // Stop repeating when released OR if keyboard mods are leaving the None state. Allows going from Mod+Key to Key by releasing Mod.
    LinearUIInputFlags_RepeatUntilOtherKeyPress    = 1 << 7,   // Stop repeating when released OR if any other keyboard key is pressed during the repeat

    // Flags for SetKeyOwner(), SetItemKeyOwner()
    // - Locking key away from non-input aware code. Locking is useful to make input-owner-aware code steal keys from non-input-owner-aware code. If all code is input-owner-aware locking would never be necessary.
    LinearUIInputFlags_LockThisFrame               = 1 << 20,  // Further accesses to key data will require EXPLICIT owner ID (LinearUIKeyOwner_Any/0 will NOT accepted for polling). Cleared at end of frame.
    LinearUIInputFlags_LockUntilRelease            = 1 << 21,  // Further accesses to key data will require EXPLICIT owner ID (LinearUIKeyOwner_Any/0 will NOT accepted for polling). Cleared when the key is released or at end of each frame if key is released.

    // - Condition for SetItemKeyOwner()
    LinearUIInputFlags_CondHovered                 = 1 << 22,  // Only set if item is hovered (default to both)
    LinearUIInputFlags_CondActive                  = 1 << 23,  // Only set if item is active (default to both)
    LinearUIInputFlags_CondDefault_                = LinearUIInputFlags_CondHovered | LinearUIInputFlags_CondActive,

    // [Internal] Mask of which function support which flags
    LinearUIInputFlags_RepeatRateMask_             = LinearUIInputFlags_RepeatRateDefault | LinearUIInputFlags_RepeatRateNavMove | LinearUIInputFlags_RepeatRateNavTweak,
    LinearUIInputFlags_RepeatUntilMask_            = LinearUIInputFlags_RepeatUntilRelease | LinearUIInputFlags_RepeatUntilKeyModsChange | LinearUIInputFlags_RepeatUntilKeyModsChangeFromNone | LinearUIInputFlags_RepeatUntilOtherKeyPress,
    LinearUIInputFlags_RepeatMask_                 = LinearUIInputFlags_Repeat | LinearUIInputFlags_RepeatRateMask_ | LinearUIInputFlags_RepeatUntilMask_,
    LinearUIInputFlags_CondMask_                   = LinearUIInputFlags_CondHovered | LinearUIInputFlags_CondActive,
    LinearUIInputFlags_RouteTypeMask_              = LinearUIInputFlags_RouteActive | LinearUIInputFlags_RouteFocused | LinearUIInputFlags_RouteGlobal | LinearUIInputFlags_RouteAlways,
    LinearUIInputFlags_RouteOptionsMask_           = LinearUIInputFlags_RouteOverFocused | LinearUIInputFlags_RouteOverActive | LinearUIInputFlags_RouteUnlessBgFocused | LinearUIInputFlags_RouteFromRootWindow,
    LinearUIInputFlags_SupportedByIsKeyPressed     = LinearUIInputFlags_RepeatMask_,
    LinearUIInputFlags_SupportedByIsMouseClicked   = LinearUIInputFlags_Repeat,
    LinearUIInputFlags_SupportedByShortcut         = LinearUIInputFlags_RepeatMask_ | LinearUIInputFlags_RouteTypeMask_ | LinearUIInputFlags_RouteOptionsMask_,
    LinearUIInputFlags_SupportedBySetNextItemShortcut = LinearUIInputFlags_RepeatMask_ | LinearUIInputFlags_RouteTypeMask_ | LinearUIInputFlags_RouteOptionsMask_ | LinearUIInputFlags_Tooltip,
    LinearUIInputFlags_SupportedBySetKeyOwner      = LinearUIInputFlags_LockThisFrame | LinearUIInputFlags_LockUntilRelease,
    LinearUIInputFlags_SupportedBySetItemKeyOwner  = LinearUIInputFlags_SupportedBySetKeyOwner | LinearUIInputFlags_CondMask_,
};

//-----------------------------------------------------------------------------
// [SECTION] Clipper support
//-----------------------------------------------------------------------------

// Note that Max is exclusive, so perhaps should be using a Begin/End convention.
struct LinearUIListClipperRange
{
    int     Min;
    int     Max;
    bool    PosToIndexConvert;      // Begin/End are absolute position (will be converted to indices later)
    LinearS8    PosToIndexOffsetMin;    // Add to Min after converting to indices
    LinearS8    PosToIndexOffsetMax;    // Add to Min after converting to indices

    static LinearUIListClipperRange    FromIndices(int min, int max)                               { LinearUIListClipperRange r = { min, max, false, 0, 0 }; return r; }
    static LinearUIListClipperRange    FromPositions(float y1, float y2, int off_min, int off_max) { LinearUIListClipperRange r = { (int)y1, (int)y2, true, (LinearS8)off_min, (LinearS8)off_max }; return r; }
};

// Temporary clipper data, buffers shared/reused between instances
struct LinearUIListClipperData
{
    LinearUIListClipper*               ListClipper;
    float                           LossynessOffset;
    int                             StepNo;
    int                             ItemsFrozen;
    LinearVector<LinearUIListClipperRange> Ranges;

    LinearUIListClipperData()          { memset((void*)this, 0, sizeof(*this)); }
    void                            Reset(LinearUIListClipper* clipper) { ListClipper = clipper; StepNo = ItemsFrozen = 0; Ranges.resize(0); }
};

//-----------------------------------------------------------------------------
// [SECTION] Navigation support
//-----------------------------------------------------------------------------

enum LinearUIActivateFlags_
{
    LinearUIActivateFlags_None                 = 0,
    LinearUIActivateFlags_PreferInput          = 1 << 0,       // Favor activation that requires keyboard text input (e.g. for Slider/Drag). Default for Enter key.
    LinearUIActivateFlags_PreferTweak          = 1 << 1,       // Favor activation for tweaking with arrows or gamepad (e.g. for Slider/Drag). Default for Space key and if keyboard is not used.
    LinearUIActivateFlags_TryToPreserveState   = 1 << 2,       // Request widget to preserve state if it can (e.g. InputText will try to preserve cursor/selection)
    LinearUIActivateFlags_FromTabbing          = 1 << 3,       // Activation requested by a tabbing request (LinearUINavMoveFlags_IsTabbing)
    LinearUIActivateFlags_FromShortcut         = 1 << 4,       // Activation requested by an item shortcut via SetNextItemShortcut() function.
    LinearUIActivateFlags_FromFocusApi         = 1 << 5,       // Activation requested by an api request (LinearUINavMoveFlags_FocusApi)
};

// Early work-in-progress API for ScrollToItem()
// FIXME: Missing flags to request making both edges visible when possible.
enum LinearUIScrollFlags_
{
    LinearUIScrollFlags_None                   = 0,
    LinearUIScrollFlags_KeepVisibleEdgeX       = 1 << 0,       // If item is not visible: scroll as little as possible on X axis to bring item back into view [default for X axis]
    LinearUIScrollFlags_KeepVisibleEdgeY       = 1 << 1,       // If item is not visible: scroll as little as possible on Y axis to bring item back into view [default for Y axis for windows that are already visible]
    LinearUIScrollFlags_KeepVisibleCenterX     = 1 << 2,       // If item is not visible: scroll to make the item centered on X axis [rarely used]
    LinearUIScrollFlags_KeepVisibleCenterY     = 1 << 3,       // If item is not visible: scroll to make the item centered on Y axis
    LinearUIScrollFlags_AlwaysCenterX          = 1 << 4,       // Always center the result item on X axis [rarely used]
    LinearUIScrollFlags_AlwaysCenterY          = 1 << 5,       // Always center the result item on Y axis [default for Y axis for appearing window)
    LinearUIScrollFlags_NoScrollParent         = 1 << 6,       // Disable forwarding scrolling to parent window if required to keep item/rect visible (only scroll window the function was applied to).
    LinearUIScrollFlags_MaskX_                 = LinearUIScrollFlags_KeepVisibleEdgeX | LinearUIScrollFlags_KeepVisibleCenterX | LinearUIScrollFlags_AlwaysCenterX,
    LinearUIScrollFlags_MaskY_                 = LinearUIScrollFlags_KeepVisibleEdgeY | LinearUIScrollFlags_KeepVisibleCenterY | LinearUIScrollFlags_AlwaysCenterY,
};

enum LinearUINavRenderCursorFlags_
{
    LinearUINavRenderCursorFlags_None          = 0,
    LinearUINavRenderCursorFlags_Compact       = 1 << 1,       // Compact highlight, no padding/distance from focused item
    LinearUINavRenderCursorFlags_AlwaysDraw    = 1 << 2,       // Draw rectangular highlight if (g.NavId == id) even when g.NavCursorVisible == false, aka even when using the mouse.
    LinearUINavRenderCursorFlags_NoRounding    = 1 << 3,
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    LinearUINavHighlightFlags_None             = LinearUINavRenderCursorFlags_None,       // Renamed in 1.91.4
    LinearUINavHighlightFlags_Compact          = LinearUINavRenderCursorFlags_Compact,    // Renamed in 1.91.4
    LinearUINavHighlightFlags_AlwaysDraw       = LinearUINavRenderCursorFlags_AlwaysDraw, // Renamed in 1.91.4
    LinearUINavHighlightFlags_NoRounding       = LinearUINavRenderCursorFlags_NoRounding, // Renamed in 1.91.4
    //LinearUINavHighlightFlags_TypeThin       = LinearUINavRenderCursorFlags_Compact,    // Renamed in 1.90.2
#endif
};

enum LinearUINavMoveFlags_
{
    LinearUINavMoveFlags_None                  = 0,
    LinearUINavMoveFlags_LoopX                 = 1 << 0,   // On failed request, restart from opposite side
    LinearUINavMoveFlags_LoopY                 = 1 << 1,
    LinearUINavMoveFlags_WrapX                 = 1 << 2,   // On failed request, request from opposite side one line down (when NavDir==right) or one line up (when NavDir==left)
    LinearUINavMoveFlags_WrapY                 = 1 << 3,   // This is not super useful but provided for completeness
    LinearUINavMoveFlags_WrapMask_             = LinearUINavMoveFlags_LoopX | LinearUINavMoveFlags_LoopY | LinearUINavMoveFlags_WrapX | LinearUINavMoveFlags_WrapY,
    LinearUINavMoveFlags_AllowCurrentNavId     = 1 << 4,   // Allow scoring and considering the current NavId as a move target candidate. This is used when the move source is offset (e.g. pressing PageDown actually needs to send a Up move request, if we are pressing PageDown from the bottom-most item we need to stay in place)
    LinearUINavMoveFlags_AlsoScoreVisibleSet   = 1 << 5,   // Store alternate result in NavMoveResultLocalVisible that only comprise elements that are already fully visible (used by PageUp/PageDown)
    LinearUINavMoveFlags_ScrollToEdgeY         = 1 << 6,   // Force scrolling to min/max (used by Home/End) // FIXME-NAV: Aim to remove or reword as LinearUIScrollFlags
    LinearUINavMoveFlags_Forwarded             = 1 << 7,
    LinearUINavMoveFlags_DebugNoResult         = 1 << 8,   // Dummy scoring for debug purpose, don't apply result
    LinearUINavMoveFlags_FocusApi              = 1 << 9,   // Requests from focus API can land/focus/activate items even if they are marked with _NoTabStop (see NavProcessItemForTabbingRequest() for details)
    LinearUINavMoveFlags_IsTabbing             = 1 << 10,  // == Focus + Activate if item is Inputable + DontChangeNavHighlight
    LinearUINavMoveFlags_IsPageMove            = 1 << 11,  // Identify a PageDown/PageUp request.
    LinearUINavMoveFlags_Activate              = 1 << 12,  // Activate/select target item.
    LinearUINavMoveFlags_NoSelect              = 1 << 13,  // Don't trigger selection by not setting g.NavJustMovedTo
    LinearUINavMoveFlags_NoSetNavCursorVisible = 1 << 14,  // Do not alter the nav cursor visible state
    LinearUINavMoveFlags_NoClearActiveId       = 1 << 15,  // (Experimental) Do not clear active id when applying move result
};

enum LinearUINavLayer
{
    LinearUINavLayer_Main  = 0,    // Main scrolling layer
    LinearUINavLayer_Menu  = 1,    // Menu layer (access with Alt)
    LinearUINavLayer_COUNT
};

// Storage for navigation query/results
struct LinearUINavItemData
{
    LinearUIWindow*        Window;         // Init,Move    // Best candidate window (result->ItemWindow->RootWindowForNav == request->Window)
    LinearUIID             ID;             // Init,Move    // Best candidate item ID
    LinearUIID             FocusScopeId;   // Init,Move    // Best candidate focus scope ID
    LinearRect              RectRel;        // Init,Move    // Best candidate bounding box in window relative space
    LinearUIItemFlags      ItemFlags;      // ????,Move    // Best candidate item flags
    float               DistBox;        //      Move    // Best candidate box distance to current NavId
    float               DistCenter;     //      Move    // Best candidate center distance to current NavId
    float               DistAxial;      //      Move    // Best candidate axial distance to current NavId
    LinearUISelectionUserData SelectionUserData;//I+Mov    // Best candidate SetNextItemSelectionUserData() value. Valid if (ItemFlags & LinearUIItemFlags_HasSelectionUserData)

    LinearUINavItemData()  { Clear(); }
    void Clear()        { Window = NULL; ID = FocusScopeId = 0; ItemFlags = 0; SelectionUserData = -1; DistBox = DistCenter = DistAxial = FLT_MAX; }
};

// Storage for PushFocusScope(), g.FocusScopeStack[], g.NavFocusRoute[]
struct LinearUIFocusScopeData
{
    LinearUIID             ID;
    LinearUIID             WindowID;
};

//-----------------------------------------------------------------------------
// [SECTION] Typing-select support
//-----------------------------------------------------------------------------

// Flags for GetTypingSelectRequest()
enum LinearUITypingSelectFlags_
{
    LinearUITypingSelectFlags_None                 = 0,
    LinearUITypingSelectFlags_AllowBackspace       = 1 << 0,   // Backspace to delete character inputs. If using: ensure GetTypingSelectRequest() is not called more than once per frame (filter by e.g. focus state)
    LinearUITypingSelectFlags_AllowSingleCharMode  = 1 << 1,   // Allow "single char" search mode which is activated when pressing the same character multiple times.
};

// Returned by GetTypingSelectRequest(), designed to eventually be public.
struct LINEARUI_API LinearUITypingSelectRequest
{
    LinearUITypingSelectFlags  Flags;              // Flags passed to GetTypingSelectRequest()
    int                     SearchBufferLen;
    const char*             SearchBuffer;       // Search buffer contents (use full string. unless SingleCharMode is set, in which case use SingleCharSize).
    bool                    SelectRequest;      // Set when buffer was modified this frame, requesting a selection.
    bool                    SingleCharMode;     // Notify when buffer contains same character repeated, to implement special mode. In this situation it preferred to not display any on-screen search indication.
    LinearS8                    SingleCharSize;     // Length in bytes of first letter codepoint (1 for ascii, 2-4 for UTF-8). If (SearchBufferLen==RepeatCharSize) only 1 letter has been input.
};

// Storage for GetTypingSelectRequest()
struct LINEARUI_API LinearUITypingSelectState
{
    LinearUITypingSelectRequest Request;           // User-facing data
    char            SearchBuffer[64];           // Search buffer: no need to make dynamic as this search is very transient.
    LinearUIID         FocusScope;
    int             LastRequestFrame = 0;
    float           LastRequestTime = 0.0f;
    bool            SingleCharModeLock = false; // After a certain single char repeat count we lock into SingleCharMode. Two benefits: 1) buffer never fill, 2) we can provide an immediate SingleChar mode without timer elapsing.

    LinearUITypingSelectState() { memset((void*)this, 0, sizeof(*this)); }
    void            Clear()  { SearchBuffer[0] = 0; SingleCharModeLock = false; } // We preserve remaining data for easier debugging
};

//-----------------------------------------------------------------------------
// [SECTION] Columns support
//-----------------------------------------------------------------------------

// Flags for internal's BeginColumns(). This is an obsolete API. Prefer using BeginTable() nowadays!
enum LinearUIOldColumnFlags_
{
    LinearUIOldColumnFlags_None                    = 0,
    LinearUIOldColumnFlags_NoBorder                = 1 << 0,   // Disable column dividers
    LinearUIOldColumnFlags_NoResize                = 1 << 1,   // Disable resizing columns when clicking on the dividers
    LinearUIOldColumnFlags_NoPreserveWidths        = 1 << 2,   // Disable column width preservation when adjusting columns
    LinearUIOldColumnFlags_NoForceWithinWindow     = 1 << 3,   // Disable forcing columns to fit within window
    LinearUIOldColumnFlags_GrowParentContentsSize  = 1 << 4,   // Restore pre-1.51 behavior of extending the parent window contents size but _without affecting the columns width at all_. Will eventually remove.

    // Obsolete names (will be removed)
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    //LinearUIColumnsFlags_None                    = LinearUIOldColumnFlags_None,
    //LinearUIColumnsFlags_NoBorder                = LinearUIOldColumnFlags_NoBorder,
    //LinearUIColumnsFlags_NoResize                = LinearUIOldColumnFlags_NoResize,
    //LinearUIColumnsFlags_NoPreserveWidths        = LinearUIOldColumnFlags_NoPreserveWidths,
    //LinearUIColumnsFlags_NoForceWithinWindow     = LinearUIOldColumnFlags_NoForceWithinWindow,
    //LinearUIColumnsFlags_GrowParentContentsSize  = LinearUIOldColumnFlags_GrowParentContentsSize,
#endif
};

struct LinearUIOldColumnData
{
    float               OffsetNorm;             // Column start offset, normalized 0.0 (far left) -> 1.0 (far right)
    float               OffsetNormBeforeResize;
    LinearUIOldColumnFlags Flags;                  // Not exposed
    LinearRect              ClipRect;

    LinearUIOldColumnData() { memset((void*)this, 0, sizeof(*this)); }
};

struct LinearUIOldColumns
{
    LinearUIID             ID;
    LinearUIOldColumnFlags Flags;
    bool                IsFirstFrame;
    bool                IsBeingResized;
    int                 Current;
    int                 Count;
    float               OffMinX, OffMaxX;       // Offsets from HostWorkRect.Min.x
    float               LineMinY, LineMaxY;
    float               HostCursorPosY;         // Backup of CursorPos at the time of BeginColumns()
    float               HostCursorMaxPosX;      // Backup of CursorMaxPos at the time of BeginColumns()
    LinearRect              HostInitialClipRect;    // Backup of ClipRect at the time of BeginColumns()
    LinearRect              HostBackupClipRect;     // Backup of ClipRect during PushColumnsBackground()/PopColumnsBackground()
    LinearRect              HostBackupParentWorkRect;//Backup of WorkRect at the time of BeginColumns()
    LinearVector<LinearUIOldColumnData> Columns;
    LinearDrawListSplitter  Splitter;

    LinearUIOldColumns()   { memset((void*)this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Box-select support
//-----------------------------------------------------------------------------

struct LinearUIBoxSelectState
{
    // Active box-selection data (persistent, 1 active at a time)
    LinearUIID                 ID;
    bool                    IsActive;
    bool                    IsStarting;
    bool                    IsStartedFromVoid;  // Starting click was not from an item.
    bool                    IsStartedSetNavIdOnce;
    bool                    RequestClear;
    LinearUIKeyChord           KeyMods : 16;       // Latched key-mods for box-select logic.
    LinearVec2                  StartPosRel;        // Start position in window-contents relative space (to support scrolling)
    LinearVec2                  EndPosRel;          // End position in window-contents relative space
    LinearVec2                  ScrollAccum;        // Scrolling accumulator (to behave at high-frame spaces)
    LinearUIWindow*            Window;

    // Temporary/Transient data
    bool                    UnclipMode;         // (Temp/Transient, here in hot area). Set/cleared by the BeginMultiSelect()/EndMultiSelect() owning active box-select.
    LinearRect                  UnclipRect;         // Rectangle where ItemAdd() clipping may be temporarily disabled. Need support by multi-select supporting widgets.
    LinearRect                  UnclipRects[2];     // Per-axis versions.
    LinearRect                  BoxSelectRectPrev;  // Selection rectangle in absolute coordinates (derived every frame from BoxSelectStartPosRel and MousePos)
    LinearRect                  BoxSelectRectCurr;

    LinearUIBoxSelectState()   { memset((void*)this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Multi-select support
//-----------------------------------------------------------------------------

// We always assume that -1 is an invalid value (which works for indices and pointers)
#define LinearUISelectionUserData_Invalid        ((LinearUISelectionUserData)-1)

// Temporary storage for multi-select
struct LINEARUI_API LinearUIMultiSelectTempData
{
    LinearUIMultiSelectIO      IO;                 // MUST BE FIRST FIELD. Requests are set and returned by BeginMultiSelect()/EndMultiSelect() + written to by user during the loop.
    LinearUIMultiSelectState*  Storage;
    LinearUIID                 FocusScopeId;       // Copied from g.CurrentFocusScopeId (unless another selection scope was pushed manually)
    LinearUIMultiSelectFlags   Flags;
    LinearVec2                  ScopeRectMin;
    LinearVec2                  BackupCursorMaxPos;
    //LinearUISelectionUserData CurrSubmittedItem; // Copy of last submitted item data, used to merge output ranges.
    //LinearUISelectionUserData PrevSubmittedItem; // Copy of previous submitted item data, used to merge output ranges.
    LinearUIID                 BoxSelectId;
    LinearUIKeyChord           KeyMods;
    LinearS8                    LoopRequestSetAll;  // -1: no operation, 0: clear all, 1: select all.
    bool                    IsEndIO;            // Set when switching IO from BeginMultiSelect() to EndMultiSelect() state.
    bool                    IsFocused;          // Set if currently focusing the selection scope (any item of the selection). May be used if you have custom shortcut associated to selection.
    bool                    IsKeyboardSetRange; // Set by BeginMultiSelect() when using Shift+Navigation. Because scrolling may be affected we can't afford a frame of lag with Shift+Navigation.
    bool                    NavIdPassedBy;
    bool                    RangeSrcPassedBy;   // Set by the item that matches RangeSrcItem.
    bool                    RangeDstPassedBy;   // Set by the item that matches NavJustMovedToId when IsSetRange is set.

    LinearUIMultiSelectTempData()  { Clear(); }
    void Clear()            { size_t io_sz = sizeof(IO); ClearIO(); memset((void*)(&IO + 1), 0, sizeof(*this) - io_sz); } // Zero-clear except IO as we preserve IO.Requests[] buffer allocation.
    void ClearIO()          { IO.Requests.resize(0); IO.RangeSrcItem = IO.NavIdItem = LinearUISelectionUserData_Invalid; IO.NavIdSelected = IO.RangeSrcReset = false; }
};

// Persistent storage for multi-select (as long as selection is alive)
struct LINEARUI_API LinearUIMultiSelectState
{
    LinearUIWindow*            Window;
    LinearUIID                 ID;
    int                     LastFrameActive;    // Last used frame-count, for GC.
    int                     LastSelectionSize;  // Set by BeginMultiSelect() based on optional info provided by user. May be -1 if unknown.
    LinearS8                    RangeSelected;      // -1 (don't have) or true/false
    LinearS8                    NavIdSelected;      // -1 (don't have) or true/false
    LinearUISelectionUserData  RangeSrcItem;       //
    LinearUISelectionUserData  NavIdItem;          // SetNextItemSelectionUserData() value for NavId (if part of submitted items)

    LinearUIMultiSelectState() { Window = NULL; ID = 0; LastFrameActive = LastSelectionSize = 0; RangeSelected = NavIdSelected = -1; RangeSrcItem = NavIdItem = LinearUISelectionUserData_Invalid; }
};

//-----------------------------------------------------------------------------
// [SECTION] Docking support
//-----------------------------------------------------------------------------

#define DOCKING_HOST_DRAW_CHANNEL_BG 0  // Dock host: background fill
#define DOCKING_HOST_DRAW_CHANNEL_FG 1  // Dock host: decorations and contents

#ifdef LINEARUI_HAS_DOCK

// Extend LinearUIDockNodeFlags_
enum LinearUIDockNodeFlagsPrivate_
{
    // [Internal]
    LinearUIDockNodeFlags_DockSpace                = 1 << 10,  // Saved // A dockspace is a node that occupy space within an existing user window. Otherwise the node is floating and create its own window.
    LinearUIDockNodeFlags_CentralNode              = 1 << 11,  // Saved // The central node has 2 main properties: stay visible when empty, only use "remaining" spaces from its neighbor.
    LinearUIDockNodeFlags_NoTabBar                 = 1 << 12,  // Saved // Tab bar is completely unavailable. No triangle in the corner to enable it back.
    LinearUIDockNodeFlags_HiddenTabBar             = 1 << 13,  // Saved // Tab bar is hidden, with a triangle in the corner to show it again (NB: actual tab-bar instance may be destroyed as this is only used for single-window tab bar)
    LinearUIDockNodeFlags_NoWindowMenuButton       = 1 << 14,  // Saved // Disable window/docking menu (that one that appears instead of the collapse button)
    LinearUIDockNodeFlags_NoCloseButton            = 1 << 15,  // Saved // Disable close button
    LinearUIDockNodeFlags_NoResizeX                = 1 << 16,  //       //
    LinearUIDockNodeFlags_NoResizeY                = 1 << 17,  //       //
    LinearUIDockNodeFlags_DockedWindowsInFocusRoute= 1 << 18,  //       // Any docked window will be automatically be focus-route chained (window->ParentWindowForFocusRoute set to this) so Shortcut() in this window can run when any docked window is focused.

    // Disable docking/undocking actions in this dockspace or individual node (existing docked nodes will be preserved)
    // Those are not exposed in public because the desirable sharing/inheriting/copy-flag-on-split behaviors are quite difficult to design and understand.
    // The two public flags LinearUIDockNodeFlags_NoDockingOverCentralNode/LinearUIDockNodeFlags_NoDockingSplit don't have those issues.
    LinearUIDockNodeFlags_NoDockingSplitOther      = 1 << 19,  //       // Disable this node from splitting other windows/nodes.
    LinearUIDockNodeFlags_NoDockingOverMe          = 1 << 20,  //       // Disable other windows/nodes from being docked over this node.
    LinearUIDockNodeFlags_NoDockingOverOther       = 1 << 21,  //       // Disable this node from being docked over another window or non-empty node.
    LinearUIDockNodeFlags_NoDockingOverEmpty       = 1 << 22,  //       // Disable this node from being docked over an empty node (e.g. DockSpace with no other windows)
    LinearUIDockNodeFlags_NoDocking                = LinearUIDockNodeFlags_NoDockingOverMe | LinearUIDockNodeFlags_NoDockingOverOther | LinearUIDockNodeFlags_NoDockingOverEmpty | LinearUIDockNodeFlags_NoDockingSplit | LinearUIDockNodeFlags_NoDockingSplitOther,

    // Masks
    LinearUIDockNodeFlags_SharedFlagsInheritMask_  = ~0,
    LinearUIDockNodeFlags_NoResizeFlagsMask_       = (int)LinearUIDockNodeFlags_NoResize | LinearUIDockNodeFlags_NoResizeX | LinearUIDockNodeFlags_NoResizeY,

    // When splitting, those local flags are moved to the inheriting child, never duplicated
    LinearUIDockNodeFlags_LocalFlagsTransferMask_  = (int)LinearUIDockNodeFlags_NoDockingSplit | LinearUIDockNodeFlags_NoResizeFlagsMask_ | (int)LinearUIDockNodeFlags_AutoHideTabBar | LinearUIDockNodeFlags_CentralNode | LinearUIDockNodeFlags_NoTabBar | LinearUIDockNodeFlags_HiddenTabBar | LinearUIDockNodeFlags_NoWindowMenuButton | LinearUIDockNodeFlags_NoCloseButton,
    LinearUIDockNodeFlags_SavedFlagsMask_          = LinearUIDockNodeFlags_NoResizeFlagsMask_ | LinearUIDockNodeFlags_DockSpace | LinearUIDockNodeFlags_CentralNode | LinearUIDockNodeFlags_NoTabBar | LinearUIDockNodeFlags_HiddenTabBar | LinearUIDockNodeFlags_NoWindowMenuButton | LinearUIDockNodeFlags_NoCloseButton,
};

// Store the source authority (dock node vs window) of a field
enum LinearUIDataAuthority_
{
    LinearUIDataAuthority_Auto,
    LinearUIDataAuthority_DockNode,
    LinearUIDataAuthority_Window,
};

enum LinearUIDockNodeState
{
    LinearUIDockNodeState_Unknown,
    LinearUIDockNodeState_HostWindowHiddenBecauseSingleWindow,
    LinearUIDockNodeState_HostWindowHiddenBecauseWindowsAreResizing,
    LinearUIDockNodeState_HostWindowVisible,
};

// sizeof() 176~216
struct LINEARUI_API LinearUIDockNode
{
    LinearUIID                 ID;
    LinearUIDockNodeFlags      SharedFlags;                // (Write) Flags shared by all nodes of a same dockspace hierarchy (inherited from the root node)
    LinearUIDockNodeFlags      LocalFlags;                 // (Write) Flags specific to this node
    LinearUIDockNodeFlags      LocalFlagsInWindows;        // (Write) Flags specific to this node, applied from windows
    LinearUIDockNodeFlags      MergedFlags;                // (Read)  Effective flags (== SharedFlags | LocalFlagsInNode | LocalFlagsInWindows)
    LinearUIDockNodeState      State;
    LinearUIDockNode*          ParentNode;
    LinearUIDockNode*          ChildNodes[2];              // [Split node only] Child nodes (left/right or top/bottom). Consider switching to an array.
    LinearVector<LinearUIWindow*>  Windows;                    // Note: unordered list! Iterate TabBar->Tabs for user-order.
    LinearUITabBar*            TabBar;
    LinearVec2                  Pos;                        // Current position
    LinearVec2                  Size;                       // Current size
    LinearVec2                  SizeRef;                    // [Split node only] Last explicitly written-to size (overridden when using a splitter affecting the node), used to calculate Size.
    LinearUIAxis               SplitAxis;                  // [Split node only] Split axis (X or Y)
    LinearU32                   LastBgColor;
    LinearUIWindowClass        WindowClass;                // [Root node only]

    LinearUIWindow*            HostWindow;
    LinearUIWindow*            VisibleWindow;              // Generally point to window which is ID is == SelectedTabID, but when CTRL+Tabbing this can be a different window.
    LinearUIDockNode*          CentralNode;                // [Root node only] Pointer to central node.
    LinearUIDockNode*          OnlyNodeWithWindows;        // [Root node only] Set when there is a single visible node within the hierarchy.
    int                     CountNodeWithWindows;       // [Root node only]
    int                     LastFrameAlive;             // Last frame number the node was updated or kept alive explicitly with DockSpace() + LinearUIDockNodeFlags_KeepAliveOnly
    int                     LastFrameActive;            // Last frame number the node was updated.
    int                     LastFrameFocused;           // Last frame number the node was focused.
    LinearUIID                 LastFocusedNodeId;          // [Root node only] Which of our child docking node (any ancestor in the hierarchy) was last focused.
    LinearUIID                 SelectedTabId;              // [Leaf node only] Which of our tab/window is selected.
    LinearUIID                 WantCloseTabId;             // [Leaf node only] Set when closing a specific tab/window.
    LinearUIID                 RefViewportId;              // Reference viewport ID from visible window when HostWindow == NULL.
    LinearUIDataAuthority      AuthorityForPos         :3;
    LinearUIDataAuthority      AuthorityForSize        :3;
    LinearUIDataAuthority      AuthorityForViewport    :3;
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

    LinearUIDockNode(LinearUIID id);
    ~LinearUIDockNode();
    bool                    IsRootNode() const      { return ParentNode == NULL; }
    bool                    IsDockSpace() const     { return (MergedFlags & LinearUIDockNodeFlags_DockSpace) != 0; }
    bool                    IsFloatingNode() const  { return ParentNode == NULL && (MergedFlags & LinearUIDockNodeFlags_DockSpace) == 0; }
    bool                    IsCentralNode() const   { return (MergedFlags & LinearUIDockNodeFlags_CentralNode) != 0; }
    bool                    IsHiddenTabBar() const  { return (MergedFlags & LinearUIDockNodeFlags_HiddenTabBar) != 0; } // Hidden tab bar can be shown back by clicking the small triangle
    bool                    IsNoTabBar() const      { return (MergedFlags & LinearUIDockNodeFlags_NoTabBar) != 0; }     // Never show a tab bar
    bool                    IsSplitNode() const     { return ChildNodes[0] != NULL; }
    bool                    IsLeafNode() const      { return ChildNodes[0] == NULL; }
    bool                    IsEmpty() const         { return ChildNodes[0] == NULL && Windows.Size == 0; }
    LinearRect                  Rect() const            { return LinearRect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y); }

    void                    SetLocalFlags(LinearUIDockNodeFlags flags) { LocalFlags = flags; UpdateMergedFlags(); }
    void                    UpdateMergedFlags()     { MergedFlags = SharedFlags | LocalFlags | LocalFlagsInWindows; }
};

// List of colors that are stored at the time of Begin() into Docked Windows.
// We currently store the packed colors in a simple array window->DockStyle.Colors[].
// A better solution may involve appending into a log of colors in LinearUIContext + store offsets into those arrays in LinearUIWindow,
// but it would be more complex as we'd need to double-buffer both as e.g. drop target may refer to window from last frame.
enum LinearUIWindowDockStyleCol
{
    LinearUIWindowDockStyleCol_Text,
    LinearUIWindowDockStyleCol_TabHovered,
    LinearUIWindowDockStyleCol_TabFocused,
    LinearUIWindowDockStyleCol_TabSelected,
    LinearUIWindowDockStyleCol_TabSelectedOverline,
    LinearUIWindowDockStyleCol_TabDimmed,
    LinearUIWindowDockStyleCol_TabDimmedSelected,
    LinearUIWindowDockStyleCol_TabDimmedSelectedOverline,
    LinearUIWindowDockStyleCol_UnsavedMarker,
    LinearUIWindowDockStyleCol_COUNT
};

// We don't store style.Alpha: dock_node->LastBgColor embeds it and otherwise it would only affect the docking tab, which intuitively I would say we don't want to.
struct LinearUIWindowDockStyle
{
    LinearU32 Colors[LinearUIWindowDockStyleCol_COUNT];
};

struct LinearUIDockContext
{
    LinearUIStorage                    Nodes;          // Map ID -> LinearUIDockNode*: Active nodes
    LinearVector<LinearUIDockRequest>      Requests;
    LinearVector<LinearUIDockNodeSettings> NodesSettings;
    bool                            WantFullRebuild;
    LinearUIDockContext()              { memset((void*)this, 0, sizeof(*this)); }
};

#endif // #ifdef LINEARUI_HAS_DOCK

//-----------------------------------------------------------------------------
// [SECTION] Viewport support
//-----------------------------------------------------------------------------

// LinearUIViewport Private/Internals fields (cardinal sin: we are using inheritance!)
// Every instance of LinearUIViewport is in fact a LinearUIViewportP.
struct LinearUIViewportP : public LinearUIViewport
{
    LinearUIWindow*        Window;                 // Set when the viewport is owned by a window (and LinearUIViewportFlags_CanHostOtherWindows is NOT set)
    int                 Idx;
    int                 LastFrameActive;        // Last frame number this viewport was activated by a window
    int                 LastFocusedStampCount;  // Last stamp number from when a window hosted by this viewport was focused (by comparing this value between two viewport we have an implicit viewport z-order we use as fallback)
    LinearUIID             LastNameHash;
    LinearVec2              LastPos;
    LinearVec2              LastSize;
    float               Alpha;                  // Window opacity (when dragging dockable windows/viewports we make them transparent)
    float               LastAlpha;
    bool                LastFocusedHadNavWindow;// Instead of maintaining a LastFocusedWindow (which may harder to correctly maintain), we merely store weither NavWindow != NULL last time the viewport was focused.
    short               PlatformMonitor;
    float               BgFgDrawListsLastTimeActive[2]; // Last frame number the background (0) and foreground (1) draw lists were used
    LinearDrawList*         BgFgDrawLists[2];       // Convenience background (0) and foreground (1) draw lists. We use them to draw software mouser cursor when io.MouseDrawCursor is set and to draw most debug overlays.
    LinearDrawData          DrawDataP;
    LinearDrawDataBuilder   DrawDataBuilder;        // Temporary data while building final LinearDrawData
    LinearVec2              LastPlatformPos;
    LinearVec2              LastPlatformSize;
    LinearVec2              LastRendererSize;

    // Per-viewport work area
    // - Insets are >= 0.0f values, distance from viewport corners to work area.
    // - BeginMainMenuBar() and DockspaceOverViewport() tend to use work area to avoid stepping over existing contents.
    // - Generally 'safeAreaInsets' in iOS land, 'DisplayCutout' in Android land.
    LinearVec2              WorkInsetMin;           // Work Area inset locked for the frame. GetWorkRect() always fits within GetMainRect().
    LinearVec2              WorkInsetMax;           // "
    LinearVec2              BuildWorkInsetMin;      // Work Area inset accumulator for current frame, to become next frame's WorkInset
    LinearVec2              BuildWorkInsetMax;      // "

    LinearUIViewportP()                    { Window = NULL; Idx = -1; LastFrameActive = LastFocusedStampCount = -1; BgFgDrawListsLastTimeActive[0] = BgFgDrawListsLastTimeActive[1] = -1.0f; LastNameHash = 0; Alpha = LastAlpha = 1.0f; LastFocusedHadNavWindow = false; PlatformMonitor = -1; BgFgDrawLists[0] = BgFgDrawLists[1] = NULL; LastPlatformPos = LastPlatformSize = LastRendererSize = LinearVec2(FLT_MAX, FLT_MAX); }
    ~LinearUIViewportP()                   { if (BgFgDrawLists[0]) LINEARUI_DELETE(BgFgDrawLists[0]); if (BgFgDrawLists[1]) LINEARUI_DELETE(BgFgDrawLists[1]); }
    void    ClearRequestFlags()         { PlatformRequestClose = PlatformRequestMove = PlatformRequestResize = false; }

    // Calculate work rect pos/size given a set of offset (we have 1 pair of offset for rect locked from last frame data, and 1 pair for currently building rect)
    LinearVec2  CalcWorkRectPos(const LinearVec2& inset_min) const                           { return LinearVec2(Pos.x + inset_min.x, Pos.y + inset_min.y); }
    LinearVec2  CalcWorkRectSize(const LinearVec2& inset_min, const LinearVec2& inset_max) const { return LinearVec2(LinearMax(0.0f, Size.x - inset_min.x - inset_max.x), LinearMax(0.0f, Size.y - inset_min.y - inset_max.y)); }
    void    UpdateWorkRect()            { WorkPos = CalcWorkRectPos(WorkInsetMin); WorkSize = CalcWorkRectSize(WorkInsetMin, WorkInsetMax); } // Update public fields

    // Helpers to retrieve LinearRect (we don't need to store BuildWorkRect as every access tend to change it, hence the code asymmetry)
    LinearRect  GetMainRect() const         { return LinearRect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y); }
    LinearRect  GetWorkRect() const         { return LinearRect(WorkPos.x, WorkPos.y, WorkPos.x + WorkSize.x, WorkPos.y + WorkSize.y); }
    LinearRect  GetBuildWorkRect() const    { LinearVec2 pos = CalcWorkRectPos(BuildWorkInsetMin); LinearVec2 size = CalcWorkRectSize(BuildWorkInsetMin, BuildWorkInsetMax); return LinearRect(pos.x, pos.y, pos.x + size.x, pos.y + size.y); }
};

//-----------------------------------------------------------------------------
// [SECTION] Settings support
//-----------------------------------------------------------------------------

// Windows data saved in linearui.ini file
// Because we never destroy or rename LinearUIWindowSettings, we can store the names in a separate buffer easily.
// (this is designed to be stored in a LinearChunkStream buffer, with the variable-length Name following our structure)
struct LinearUIWindowSettings
{
    LinearUIID     ID;
    LinearVec2ih    Pos;            // NB: Settings position are stored RELATIVE to the viewport! Whereas runtime ones are absolute positions.
    LinearVec2ih    Size;
    LinearVec2ih    ViewportPos;
    LinearUIID     ViewportId;
    LinearUIID     DockId;         // ID of last known DockNode (even if the DockNode is invisible because it has only 1 active window), or 0 if none.
    LinearUIID     ClassId;        // ID of window class if specified
    short       DockOrder;      // Order of the last time the window was visible within its DockNode. This is used to reorder windows that are reappearing on the same frame. Same value between windows that were active and windows that were none are possible.
    bool        Collapsed;
    bool        IsChild;
    bool        WantApply;      // Set when loaded from .ini data (to enable merging/loading .ini data into an already running context)
    bool        WantDelete;     // Set to invalidate/delete the settings entry

    LinearUIWindowSettings()       { memset((void*)this, 0, sizeof(*this)); DockOrder = -1; }
    char* GetName()             { return (char*)(this + 1); }
};

struct LinearUISettingsHandler
{
    const char* TypeName;       // Short description stored in .ini file. Disallowed characters: '[' ']'
    LinearUIID     TypeHash;       // == LinearHashStr(TypeName)
    void        (*ClearAllFn)(LinearUIContext* ctx, LinearUISettingsHandler* handler);                                // Clear all settings data
    void        (*ReadInitFn)(LinearUIContext* ctx, LinearUISettingsHandler* handler);                                // Read: Called before reading (in registration order)
    void*       (*ReadOpenFn)(LinearUIContext* ctx, LinearUISettingsHandler* handler, const char* name);              // Read: Called when entering into a new ini entry e.g. "[Window][Name]"
    void        (*ReadLineFn)(LinearUIContext* ctx, LinearUISettingsHandler* handler, void* entry, const char* line); // Read: Called for every line of text within an ini entry
    void        (*ApplyAllFn)(LinearUIContext* ctx, LinearUISettingsHandler* handler);                                // Read: Called after reading (in registration order)
    void        (*WriteAllFn)(LinearUIContext* ctx, LinearUISettingsHandler* handler, LinearUITextBuffer* out_buf);      // Write: Output every entries into 'out_buf'
    void*       UserData;

    LinearUISettingsHandler() { memset((void*)this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Localization support
//-----------------------------------------------------------------------------

// This is experimental and not officially supported, it'll probably fall short of features, if/when it does we may backtrack.
enum LinearUILocKey : int
{
    LinearUILocKey_VersionStr,
    LinearUILocKey_TableSizeOne,
    LinearUILocKey_TableSizeAllFit,
    LinearUILocKey_TableSizeAllDefault,
    LinearUILocKey_TableResetOrder,
    LinearUILocKey_WindowingMainMenuBar,
    LinearUILocKey_WindowingPopup,
    LinearUILocKey_WindowingUntitled,
    LinearUILocKey_OpenLink_s,
    LinearUILocKey_CopyLink,
    LinearUILocKey_DockingHideTabBar,
    LinearUILocKey_DockingHoldShiftToDock,
    LinearUILocKey_DockingDragToUndockOrMoveNode,
    LinearUILocKey_COUNT
};

struct LinearUILocEntry
{
    LinearUILocKey     Key;
    const char*     Text;
};

//-----------------------------------------------------------------------------
// [SECTION] Error handling, State recovery support
//-----------------------------------------------------------------------------

// Macros used by Recoverable Error handling
// - Only dispatch error if _EXPR: evaluate as assert (similar to an assert macro).
// - The message will always be a string literal, in order to increase likelihood of being display by an assert handler.
// - See 'Demo->Configuration->Error Handling' and LinearUIIO definitions for details on error handling.
// - Read https://github.com/ocornut/linearui/wiki/Error-Handling for details on error handling.
#ifndef LINEARUI_ASSERT_USER_ERROR
#define LINEARUI_ASSERT_USER_ERROR(_EXPR,_MSG)            do { if (!(_EXPR)) { if (LinearUI::ErrorLog(_MSG)) { LINEARUI_ASSERT((_EXPR) && _MSG); } } } while (0)               // Recoverable User Error
#define LINEARUI_ASSERT_USER_ERROR_RET(_EXPR,_MSG)        do { if (!(_EXPR)) { if (LinearUI::ErrorLog(_MSG)) { LINEARUI_ASSERT((_EXPR) && _MSG); } return; } } while (0)       // Recoverable User Error
#define LINEARUI_ASSERT_USER_ERROR_RETV(_EXPR,_RETV,_MSG) do { if (!(_EXPR)) { if (LinearUI::ErrorLog(_MSG)) { LINEARUI_ASSERT((_EXPR) && _MSG); } return _RETV; } } while (0) // Recoverable User Error
#endif

// The error callback is currently not public, as it is expected that only advanced users will rely on it.
typedef void (*LinearUIErrorCallback)(LinearUIContext* ctx, void* user_data, const char* msg); // Function signature for g.ErrorCallback

//-----------------------------------------------------------------------------
// [SECTION] Metrics, Debug Tools
//-----------------------------------------------------------------------------

// See LINEARUI_DEBUG_LOG() and LINEARUI_DEBUG_LOG_XXX() macros.
enum LinearUIDebugLogFlags_
{
    // Event types
    LinearUIDebugLogFlags_None                 = 0,
    LinearUIDebugLogFlags_EventError           = 1 << 0,   // Error submitted by LINEARUI_ASSERT_USER_ERROR()
    LinearUIDebugLogFlags_EventActiveId        = 1 << 1,
    LinearUIDebugLogFlags_EventFocus           = 1 << 2,
    LinearUIDebugLogFlags_EventPopup           = 1 << 3,
    LinearUIDebugLogFlags_EventNav             = 1 << 4,
    LinearUIDebugLogFlags_EventClipper         = 1 << 5,
    LinearUIDebugLogFlags_EventSelection       = 1 << 6,
    LinearUIDebugLogFlags_EventIO              = 1 << 7,
    LinearUIDebugLogFlags_EventFont            = 1 << 8,
    LinearUIDebugLogFlags_EventInputRouting    = 1 << 9,
    LinearUIDebugLogFlags_EventDocking         = 1 << 10,
    LinearUIDebugLogFlags_EventViewport        = 1 << 11,

    LinearUIDebugLogFlags_EventMask_           = LinearUIDebugLogFlags_EventError | LinearUIDebugLogFlags_EventActiveId | LinearUIDebugLogFlags_EventFocus | LinearUIDebugLogFlags_EventPopup | LinearUIDebugLogFlags_EventNav | LinearUIDebugLogFlags_EventClipper | LinearUIDebugLogFlags_EventSelection | LinearUIDebugLogFlags_EventIO | LinearUIDebugLogFlags_EventFont | LinearUIDebugLogFlags_EventInputRouting | LinearUIDebugLogFlags_EventDocking | LinearUIDebugLogFlags_EventViewport,
    LinearUIDebugLogFlags_OutputToTTY          = 1 << 20,  // Also send output to TTY
    LinearUIDebugLogFlags_OutputToDebugger     = 1 << 21,  // Also send output to Debugger Console [Windows only]
    LinearUIDebugLogFlags_OutputToTestEngine   = 1 << 22,  // Also send output to Dear LinearUI Test Engine
};

struct LinearUIDebugAllocEntry
{
    int         FrameCount;
    LinearS16       AllocCount;
    LinearS16       FreeCount;
};

struct LinearUIDebugAllocInfo
{
    int         TotalAllocCount;            // Number of call to MemAlloc().
    int         TotalFreeCount;
    LinearS16       LastEntriesIdx;             // Current index in buffer
    LinearUIDebugAllocEntry LastEntriesBuf[6]; // Track last 6 frames that had allocations

    LinearUIDebugAllocInfo() { memset((void*)this, 0, sizeof(*this)); }
};

struct LinearUIMetricsConfig
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
    LinearUIID     HighlightViewportID = 0;
    bool        ShowFontPreview = true;
};

struct LinearUIStackLevelInfo
{
    LinearUIID                 ID;
    LinearS8                    QueryFrameCount;            // >= 1: Sub-query in progress
    bool                    QuerySuccess;               // Sub-query obtained result from DebugHookIdInfo()
    LinearS8                    DataType;                   // LinearUIDataType
    int                     DescOffset;                 // -1 or offset into parent's ResultsPathsBuf

    LinearUIStackLevelInfo()   { memset((void*)this, 0, sizeof(*this)); DataType = -1; DescOffset = -1; }
};

struct LinearUIDebugItemPathQuery
{
    LinearUIID                 MainID;                     // ID to query details for.
    bool                    Active;                     // Used to disambiguate the case when ID == 0 and e.g. some code calls PushOverrideID(0).
    bool                    Complete;                   // All sub-queries are finished (some may have failed).
    LinearS8                    Step;                       // -1: query stack + init Results, >= 0: filling individual stack level.
    LinearVector<LinearUIStackLevelInfo> Results;
    LinearUITextBuffer         ResultsDescBuf;
    LinearUITextBuffer         ResultPathBuf;

    LinearUIDebugItemPathQuery() { memset((void*)this, 0, sizeof(*this)); }
};

// State for ID Stack tool queries
struct LinearUIIDStackTool
{
    bool                    OptHexEncodeNonAsciiChars;
    bool                    OptCopyToClipboardOnCtrlC;
    int                     LastActiveFrame;
    float                   CopyToClipboardLastTime;

    LinearUIIDStackTool()      { memset((void*)this, 0, sizeof(*this)); LastActiveFrame = -1; OptHexEncodeNonAsciiChars = true; CopyToClipboardLastTime = -FLT_MAX; }
};

//-----------------------------------------------------------------------------
// [SECTION] Generic context hooks
//-----------------------------------------------------------------------------

typedef void (*LinearUIContextHookCallback)(LinearUIContext* ctx, LinearUIContextHook* hook);
enum LinearUIContextHookType { LinearUIContextHookType_NewFramePre, LinearUIContextHookType_NewFramePost, LinearUIContextHookType_EndFramePre, LinearUIContextHookType_EndFramePost, LinearUIContextHookType_RenderPre, LinearUIContextHookType_RenderPost, LinearUIContextHookType_Shutdown, LinearUIContextHookType_PendingRemoval_ };

struct LinearUIContextHook
{
    LinearUIID                     HookId;     // A unique ID assigned by AddContextHook()
    LinearUIContextHookType        Type;
    LinearUIID                     Owner;
    LinearUIContextHookCallback    Callback;
    void*                       UserData;

    LinearUIContextHook()          { memset((void*)this, 0, sizeof(*this)); }
};

typedef void (*LinearUIDemoMarkerCallback)(const char* file, int line, const char* section);

//-----------------------------------------------------------------------------
// [SECTION] LinearUIContext (main Dear LinearUI context)
//-----------------------------------------------------------------------------

struct LinearUIContext
{
    bool                    Initialized;
    bool                    WithinFrameScope;                   // Set by NewFrame(), cleared by EndFrame()
    bool                    WithinFrameScopeWithImplicitWindow; // Set by NewFrame(), cleared by EndFrame() when the implicit debug window has been pushed
    bool                    TestEngineHookItems;                // Will call test engine hooks: LinearUITestEngineHook_ItemAdd(), LinearUITestEngineHook_ItemInfo(), LinearUITestEngineHook_Log()
    int                     FrameCount;
    int                     FrameCountEnded;
    int                     FrameCountPlatformEnded;
    int                     FrameCountRendered;
    double                  Time;
    char                    ContextName[16];                    // Storage for a context name (to facilitate debugging multi-context setups)
    LinearUIIO                 IO;
    LinearUIPlatformIO         PlatformIO;
    LinearUIStyle              Style;
    LinearUIConfigFlags        ConfigFlagsCurrFrame;               // = g.IO.ConfigFlags at the time of NewFrame()
    LinearUIConfigFlags        ConfigFlagsLastFrame;
    LinearVector<LinearFontAtlas*>  FontAtlases;                        // List of font atlases used by the context (generally only contains g.IO.Fonts aka the main font atlas)
    LinearFont*                 Font;                               // Currently bound font. (== FontStack.back().Font)
    LinearFontBaked*            FontBaked;                          // Currently bound font at currently bound size. (== Font->GetFontBaked(FontSize))
    float                   FontSize;                           // Currently bound font size == line height (== FontSizeBase + externals scales applied in the UpdateCurrentFontSize() function).
    float                   FontSizeBase;                       // Font size before scaling == style.FontSizeBase == value passed to PushFont() when specified.
    float                   FontBakedScale;                     // == FontBaked->Size / FontSize. Scale factor over baked size. Rarely used nowadays, very often == 1.0f.
    float                   FontRasterizerDensity;              // Current font density. Used by all calls to GetFontBaked().
    float                   CurrentDpiScale;                    // Current window/viewport DpiScale == CurrentViewport->DpiScale
    LinearDrawListSharedData    DrawListSharedData;
    LinearUIID                 WithinEndChildID;                   // Set within EndChild()
    LinearUIID                 WithinEndPopupID;                   // Set within EndPopup()
    void*                   TestEngine;                         // Test engine user data

    // Inputs
    LinearVector<LinearUIInputEvent> InputEventsQueue;                 // Input events which will be trickled/written into IO structure.
    LinearVector<LinearUIInputEvent> InputEventsTrail;                 // Past input events processed in NewFrame(). This is to allow domain-specific application to access e.g mouse/pen trail.
    LinearUIMouseSource        InputEventsNextMouseSource;
    LinearU32                   InputEventsNextEventId;

    // Windows state
    LinearVector<LinearUIWindow*>  Windows;                            // Windows, sorted in display order, back to front
    LinearVector<LinearUIWindow*>  WindowsFocusOrder;                  // Root windows, sorted in focus order, back to front.
    LinearVector<LinearUIWindow*>  WindowsTempSortBuffer;              // Temporary buffer used in EndFrame() to reorder windows so parents are kept before their child
    LinearVector<LinearUIWindowStackData> CurrentWindowStack;
    LinearUIStorage            WindowsById;                        // Map window's LinearUIID to LinearUIWindow*
    int                     WindowsActiveCount;                 // Number of unique windows submitted by frame
    float                   WindowsBorderHoverPadding;          // Padding around resizable windows for which hovering on counts as hovering the window == LinearMax(style.TouchExtraPadding, style.WindowBorderHoverPadding). This isn't so multi-dpi friendly.
    LinearUIID                 DebugBreakInWindow;                 // Set to break in Begin() call.
    LinearUIWindow*            CurrentWindow;                      // Window being drawn into
    LinearUIWindow*            HoveredWindow;                      // Window the mouse is hovering. Will typically catch mouse inputs.
    LinearUIWindow*            HoveredWindowUnderMovingWindow;     // Hovered window ignoring MovingWindow. Only set if MovingWindow is set.
    LinearUIWindow*            HoveredWindowBeforeClear;           // Window the mouse is hovering. Filled even with _NoMouse. This is currently useful for multi-context compositors.
    LinearUIWindow*            MovingWindow;                       // Track the window we clicked on (in order to preserve focus). The actual window that is moved is generally MovingWindow->RootWindowDockTree.
    LinearUIWindow*            WheelingWindow;                     // Track the window we started mouse-wheeling on. Until a timer elapse or mouse has moved, generally keep scrolling the same window even if during the course of scrolling the mouse ends up hovering a child window.
    LinearVec2                  WheelingWindowRefMousePos;
    int                     WheelingWindowStartFrame;           // This may be set one frame before WheelingWindow is != NULL
    int                     WheelingWindowScrolledFrame;
    float                   WheelingWindowReleaseTimer;
    LinearVec2                  WheelingWindowWheelRemainder;
    LinearVec2                  WheelingAxisAvg;

    // Item/widgets state and tracking information
    LinearUIID                 DebugDrawIdConflictsId;             // Set when we detect multiple items with the same identifier
    LinearUIID                 DebugHookIdInfoId;                  // Will call core hooks: DebugHookIdInfo() from GetID functions, used by ID Stack Tool [next HoveredId/ActiveId to not pull in an extra cache-line]
    LinearUIID                 HoveredId;                          // Hovered widget, filled during the frame
    LinearUIID                 HoveredIdPreviousFrame;
    int                     HoveredIdPreviousFrameItemCount;    // Count numbers of items using the same ID as last frame's hovered id
    float                   HoveredIdTimer;                     // Measure contiguous hovering time
    float                   HoveredIdNotActiveTimer;            // Measure contiguous hovering time where the item has not been active
    bool                    HoveredIdAllowOverlap;
    bool                    HoveredIdIsDisabled;                // At least one widget passed the rect test, but has been discarded by disabled flag or popup inhibit. May be true even if HoveredId == 0.
    bool                    ItemUnclipByLog;                    // Disable ItemAdd() clipping, essentially a memory-locality friendly copy of LogEnabled
    LinearUIID                 ActiveId;                           // Active widget
    LinearUIID                 ActiveIdIsAlive;                    // Active widget has been seen this frame (we can't use a bool as the ActiveId may change within the frame)
    float                   ActiveIdTimer;
    bool                    ActiveIdIsJustActivated;            // Set at the time of activation for one frame
    bool                    ActiveIdAllowOverlap;               // Active widget allows another widget to steal active id (generally for overlapping widgets, but not always)
    bool                    ActiveIdNoClearOnFocusLoss;         // Disable losing active id if the active id window gets unfocused.
    bool                    ActiveIdHasBeenPressedBefore;       // Track whether the active id led to a press (this is to allow changing between PressOnClick and PressOnRelease without pressing twice). Used by range_select branch.
    bool                    ActiveIdHasBeenEditedBefore;        // Was the value associated to the widget Edited over the course of the Active state.
    bool                    ActiveIdHasBeenEditedThisFrame;
    bool                    ActiveIdFromShortcut;
    LinearS8                    ActiveIdMouseButton;
    LinearUIID                 ActiveIdDisabledId;                 // When clicking a disabled item we set ActiveId=window->MoveId to avoid interference with widget code. Actual item ID is stored here.
    LinearVec2                  ActiveIdClickOffset;                // Clicked offset from upper-left corner, if applicable (currently only set by ButtonBehavior)
    LinearUIInputSource        ActiveIdSource;                     // Activating source: LinearUIInputSource_Mouse OR LinearUIInputSource_Keyboard OR LinearUIInputSource_Gamepad
    LinearUIWindow*            ActiveIdWindow;
    LinearUIID                 ActiveIdPreviousFrame;
    LinearUIDeactivatedItemData DeactivatedItemData;
    LinearUIDataTypeStorage    ActiveIdValueOnActivation;          // Backup of initial value at the time of activation. ONLY SET BY SPECIFIC WIDGETS: DragXXX and SliderXXX.
    LinearUIID                 LastActiveId;                       // Store the last non-zero ActiveId, useful for animation.
    float                   LastActiveIdTimer;                  // Store the last non-zero ActiveId timer since the beginning of activation, useful for animation.

    // Key/Input Ownership + Shortcut Routing system
    // - The idea is that instead of "eating" a given key, we can link to an owner.
    // - Input query can then read input by specifying LinearUIKeyOwner_Any (== 0), LinearUIKeyOwner_NoOwner (== -1) or a custom ID.
    // - Routing is requested ahead of time for a given chord (Key + Mods) and granted in NewFrame().
    double                  LastKeyModsChangeTime;              // Record the last time key mods changed (affect repeat delay when using shortcut logic)
    double                  LastKeyModsChangeFromNoneTime;      // Record the last time key mods changed away from being 0 (affect repeat delay when using shortcut logic)
    double                  LastKeyboardKeyPressTime;           // Record the last time a keyboard key (ignore mouse/gamepad ones) was pressed.
    LinearBitArrayForNamedKeys  KeysMayBeCharInput;                 // Lookup to tell if a key can emit char input, see IsKeyChordPotentiallyCharInput(). sizeof() = 20 bytes
    LinearUIKeyOwnerData       KeysOwnerData[LinearUIKey_NamedKey_COUNT];
    LinearUIKeyRoutingTable    KeysRoutingTable;
    LinearU32                   ActiveIdUsingNavDirMask;            // Active widget will want to read those nav move requests (e.g. can activate a button and move away from it)
    bool                    ActiveIdUsingAllKeyboardKeys;       // Active widget will want to read all keyboard keys inputs. (this is a shortcut for not taking ownership of 100+ keys, frequently used by drag operations)
    LinearUIKeyChord           DebugBreakInShortcutRouting;        // Set to break in SetShortcutRouting()/Shortcut() calls.
    //LinearU32                 ActiveIdUsingNavInputMask;          // [OBSOLETE] Since (LINEARUI_VERSION_NUM >= 18804) : 'g.ActiveIdUsingNavInputMask |= (1 << LinearUINavInput_Cancel);' becomes --> 'SetKeyOwner(LinearUIKey_Escape, g.ActiveId) and/or SetKeyOwner(LinearUIKey_NavGamepadCancel, g.ActiveId);'

    // Next window/item data
    LinearUIID                 CurrentFocusScopeId;                // Value for currently appending items == g.FocusScopeStack.back(). Not to be mistaken with g.NavFocusScopeId.
    LinearUIItemFlags          CurrentItemFlags;                   // Value for currently appending items == g.ItemFlagsStack.back()
    LinearUIID                 DebugLocateId;                      // Storage for DebugLocateItemOnHover() feature: this is read by ItemAdd() so we keep it in a hot/cached location
    LinearUINextItemData       NextItemData;                       // Storage for SetNextItem** functions
    LinearUILastItemData       LastItemData;                       // Storage for last submitted item (setup by ItemAdd)
    LinearUINextWindowData     NextWindowData;                     // Storage for SetNextWindow** functions
    bool                    DebugShowGroupRects;
    bool                    GcCompactAll;                       // Request full GC

    // Shared stacks
    LinearUICol                        DebugFlashStyleColorIdx;    // (Keep close to ColorStack to share cache line)
    LinearVector<LinearUIColorMod>         ColorStack;                 // Stack for PushStyleColor()/PopStyleColor() - inherited by Begin()
    LinearVector<LinearUIStyleMod>         StyleVarStack;              // Stack for PushStyleVar()/PopStyleVar() - inherited by Begin()
    LinearVector<LinearFontStackData>       FontStack;                  // Stack for PushFont()/PopFont() - inherited by Begin()
    LinearVector<LinearUIFocusScopeData>   FocusScopeStack;            // Stack for PushFocusScope()/PopFocusScope() - inherited by BeginChild(), pushed into by Begin()
    LinearVector<LinearUIItemFlags>        ItemFlagsStack;             // Stack for PushItemFlag()/PopItemFlag() - inherited by Begin()
    LinearVector<LinearUIGroupData>        GroupStack;                 // Stack for BeginGroup()/EndGroup() - not inherited by Begin()
    LinearVector<LinearUIPopupData>        OpenPopupStack;             // Which popups are open (persistent)
    LinearVector<LinearUIPopupData>        BeginPopupStack;            // Which level of BeginPopup() we are in (reset every frame)
    LinearVector<LinearUITreeNodeStackData>TreeNodeStack;              // Stack for TreeNode()

    // Viewports
    LinearVector<LinearUIViewportP*> Viewports;                        // Active viewports (always 1+, and generally 1 unless multi-viewports are enabled). Each viewports hold their copy of LinearDrawData.
    LinearUIViewportP*         CurrentViewport;                    // We track changes of viewport (happening in Begin) so we can call Platform_OnChangedViewport()
    LinearUIViewportP*         MouseViewport;
    LinearUIViewportP*         MouseLastHoveredViewport;           // Last known viewport that was hovered by mouse (even if we are not hovering any viewport any more) + honoring the _NoInputs flag.
    LinearUIID                 PlatformLastFocusedViewportId;
    LinearUIPlatformMonitor    FallbackMonitor;                    // Virtual monitor used as fallback if backend doesn't provide monitor information.
    LinearRect                  PlatformMonitorsFullWorkRect;       // Bounding box of all platform monitors
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
    LinearUIID                 NavId;                              // Focused item for navigation
    LinearUIWindow*            NavWindow;                          // Focused window for navigation. Could be called 'FocusedWindow'
    LinearUIID                 NavFocusScopeId;                    // Focused focus scope (e.g. selection code often wants to "clear other items" when landing on an item of the same scope)
    LinearUINavLayer           NavLayer;                           // Focused layer (main scrolling layer, or menu/title bar layer)
    LinearUIItemFlags          NavIdItemFlags;
    LinearUIID                 NavActivateId;                      // ~~ (g.ActiveId == 0) && (IsKeyPressed(LinearUIKey_Space) || IsKeyDown(LinearUIKey_Enter) || IsKeyPressed(LinearUIKey_NavGamepadActivate)) ? NavId : 0, also set when calling ActivateItemByID()
    LinearUIID                 NavActivateDownId;                  // ~~ IsKeyDown(LinearUIKey_Space) || IsKeyDown(LinearUIKey_Enter) || IsKeyDown(LinearUIKey_NavGamepadActivate) ? NavId : 0
    LinearUIID                 NavActivatePressedId;               // ~~ IsKeyPressed(LinearUIKey_Space) || IsKeyPressed(LinearUIKey_Enter) || IsKeyPressed(LinearUIKey_NavGamepadActivate) ? NavId : 0 (no repeat)
    LinearUIActivateFlags      NavActivateFlags;
    LinearVector<LinearUIFocusScopeData> NavFocusRoute;                // Reversed copy focus scope stack for NavId (should contains NavFocusScopeId). This essentially follow the window->ParentWindowForFocusRoute chain.
    LinearUIID                 NavHighlightActivatedId;
    float                   NavHighlightActivatedTimer;
    LinearUIID                 NavOpenContextMenuItemId;
    LinearUIID                 NavOpenContextMenuWindowId;
    LinearUIID                 NavNextActivateId;                  // Set by ActivateItemByID(), queued until next frame.
    LinearUIActivateFlags      NavNextActivateFlags;
    LinearUIInputSource        NavInputSource;                     // Keyboard or Gamepad mode? THIS CAN ONLY BE LinearUIInputSource_Keyboard or LinearUIInputSource_Gamepad
    LinearUISelectionUserData  NavLastValidSelectionUserData;      // Last valid data passed to SetNextItemSelectionUser(), or -1. For current window. Not reset when focusing an item that doesn't have selection data.
    LinearS8                    NavCursorHideFrames;
    //LinearUIID               NavActivateInputId;                 // Removed in 1.89.4 (July 2023). This is now part of g.NavActivateId and sets g.NavActivateFlags |= LinearUIActivateFlags_PreferInput. See commit c9a53aa74, issue #5606.

    // Navigation: Init & Move Requests
    bool                    NavAnyRequest;                      // ~~ NavMoveRequest || NavInitRequest this is to perform early out in ItemAdd()
    bool                    NavInitRequest;                     // Init request for appearing window to select first item
    bool                    NavInitRequestFromMove;
    LinearUINavItemData        NavInitResult;                      // Init request result (first item of the window, or one for which SetItemDefaultFocus() was called)
    bool                    NavMoveSubmitted;                   // Move request submitted, will process result on next NewFrame()
    bool                    NavMoveScoringItems;                // Move request submitted, still scoring incoming items
    bool                    NavMoveForwardToNextFrame;
    LinearUINavMoveFlags       NavMoveFlags;
    LinearUIScrollFlags        NavMoveScrollFlags;
    LinearUIKeyChord           NavMoveKeyMods;
    LinearUIDir                NavMoveDir;                         // Direction of the move request (left/right/up/down)
    LinearUIDir                NavMoveDirForDebug;
    LinearUIDir                NavMoveClipDir;                     // FIXME-NAV: Describe the purpose of this better. Might want to rename?
    LinearRect                  NavScoringRect;                     // Rectangle used for scoring, in screen space. Based of window->NavRectRel[], modified for directional navigation scoring.
    LinearRect                  NavScoringNoClipRect;               // Some nav operations (such as PageUp/PageDown) enforce a region which clipper will attempt to always keep submitted. Unset/invalid if inverted.
    int                     NavScoringDebugCount;               // Metrics for debugging
    int                     NavTabbingDir;                      // Generally -1 or +1, 0 when tabbing without a nav id
    int                     NavTabbingCounter;                  // >0 when counting items for tabbing
    LinearUINavItemData        NavMoveResultLocal;                 // Best move request candidate within NavWindow
    LinearUINavItemData        NavMoveResultLocalVisible;          // Best move request candidate within NavWindow that are mostly visible (when using LinearUINavMoveFlags_AlsoScoreVisibleSet flag)
    LinearUINavItemData        NavMoveResultOther;                 // Best move request candidate within NavWindow's flattened hierarchy (when using LinearUIWindowFlags_NavFlattened flag)
    LinearUINavItemData        NavTabbingResultFirst;              // First tabbing request candidate within NavWindow and flattened hierarchy

    // Navigation: record of last move request
    LinearUIID                 NavJustMovedFromFocusScopeId;       // Just navigated from this focus scope id (result of a successfully MoveRequest).
    LinearUIID                 NavJustMovedToId;                   // Just navigated to this id (result of a successfully MoveRequest).
    LinearUIID                 NavJustMovedToFocusScopeId;         // Just navigated to this focus scope id (result of a successfully MoveRequest).
    LinearUIKeyChord           NavJustMovedToKeyMods;
    bool                    NavJustMovedToIsTabbing;            // Copy of LinearUINavMoveFlags_IsTabbing. Maybe we should store whole flags.
    bool                    NavJustMovedToHasSelectionData;     // Copy of move result's ItemFlags & LinearUIItemFlags_HasSelectionUserData). Maybe we should just store LinearUINavItemData.

    // Navigation: extra config options (will be made public eventually)
    // - Tabbing (Tab, Shift+Tab) and Windowing (Ctrl+Tab, Ctrl+Shift+Tab) are enabled REGARDLESS of LinearUIConfigFlags_NavEnableKeyboard being set.
    // - Ctrl+Tab is reconfigurable because it is the only shortcut that may be polled when no window are focused. It also doesn't work e.g. Web platforms.
    bool                    ConfigNavEnableTabbing;             // = true. Enable tabbing (Tab, Shift+Tab). PLEASE LET ME KNOW IF YOU USE THIS.
    bool                    ConfigNavWindowingWithGamepad;      // = true. Enable Ctrl+Tab by holding LinearUIKey_GamepadFaceLeft (== LinearUIKey_NavGamepadMenu). When false, the button may still be used to toggle Menu layer.
    LinearUIKeyChord           ConfigNavWindowingKeyNext;          // = LinearUIMod_Ctrl | LinearUIKey_Tab (or LinearUIMod_Super | LinearUIKey_Tab on OS X). Set to 0 to disable. For reconfiguration (see #4828)
    LinearUIKeyChord           ConfigNavWindowingKeyPrev;          // = LinearUIMod_Ctrl | LinearUIMod_Shift | LinearUIKey_Tab (or LinearUIMod_Super | LinearUIMod_Shift | LinearUIKey_Tab on OS X)

    // Navigation: Windowing (Ctrl+Tab for list, or Menu button + keys or directional pads to move/resize)
    LinearUIWindow*            NavWindowingTarget;                 // Target window when doing Ctrl+Tab (or Pad Menu + FocusPrev/Next), this window is temporarily displayed top-most!
    LinearUIWindow*            NavWindowingTargetAnim;             // Record of last valid NavWindowingTarget until DimBgRatio and NavWindowingHighlightAlpha becomes 0.0f, so the fade-out can stay on it.
    LinearUIWindow*            NavWindowingListWindow;             // Internal window actually listing the Ctrl+Tab contents
    float                   NavWindowingTimer;
    float                   NavWindowingHighlightAlpha;
    LinearUIInputSource        NavWindowingInputSource;
    bool                    NavWindowingToggleLayer;            // Set while Alt or GamepadMenu is held, may be cleared by other operations, and processed when releasing the key.
    LinearUIKey                NavWindowingToggleKey;              // Keyboard/gamepad key used when toggling to menu layer.
    LinearVec2                  NavWindowingAccumDeltaPos;
    LinearVec2                  NavWindowingAccumDeltaSize;

    // Render
    float                   DimBgRatio;                         // 0.0..1.0 animation when fading in a dimming background (for modal window and Ctrl+Tab list)

    // Drag and Drop
    bool                    DragDropActive;
    bool                    DragDropWithinSource;               // Set when within a BeginDragDropXXX/EndDragDropXXX block for a drag source.
    bool                    DragDropWithinTarget;               // Set when within a BeginDragDropXXX/EndDragDropXXX block for a drag target.
    LinearUIDragDropFlags      DragDropSourceFlags;
    int                     DragDropSourceFrameCount;
    int                     DragDropMouseButton;
    LinearUIPayload            DragDropPayload;
    LinearRect                  DragDropTargetRect;                 // Store rectangle of current target candidate (we favor small targets when overlapping)
    LinearRect                  DragDropTargetClipRect;             // Store ClipRect at the time of item's drawing
    LinearUIID                 DragDropTargetId;
    LinearUIID                 DragDropTargetFullViewport;
    LinearUIDragDropFlags      DragDropAcceptFlagsCurr;
    LinearUIDragDropFlags      DragDropAcceptFlagsPrev;
    float                   DragDropAcceptIdCurrRectSurface;    // Target item surface (we resolve overlapping targets by prioritizing the smaller surface)
    LinearUIID                 DragDropAcceptIdCurr;               // Target item id (set at the time of accepting the payload)
    LinearUIID                 DragDropAcceptIdPrev;               // Target item id from previous frame (we need to store this to allow for overlapping drag and drop targets)
    int                     DragDropAcceptFrameCount;           // Last time a target expressed a desire to accept the source
    LinearUIID                 DragDropHoldJustPressedId;          // Set when holding a payload just made ButtonBehavior() return a press.
    LinearVector<unsigned char> DragDropPayloadBufHeap;             // We don't expose the LinearVector<> directly, LinearUIPayload only holds pointer+size
    unsigned char           DragDropPayloadBufLocal[16];        // Local buffer for small payloads

    // Clipper
    int                             ClipperTempDataStacked;
    LinearVector<LinearUIListClipperData>  ClipperTempData;

    // Tables
    LinearUITable*                     CurrentTable;
    LinearUIID                         DebugBreakInTable;          // Set to break in BeginTable() call.
    int                             TablesTempDataStacked;      // Temporary table data size (because we leave previous instances undestructed, we generally don't use TablesTempData.Size)
    LinearVector<LinearUITableTempData>    TablesTempData;             // Temporary table data (buffers reused/shared across instances, support nesting)
    LinearPool<LinearUITable>              Tables;                     // Persistent table data
    LinearVector<float>                 TablesLastTimeActive;       // Last used timestamp of each tables (SOA, for efficient GC)
    LinearVector<LinearDrawChannel>         DrawChannelsTempMergeBuffer;

    // Tab bars
    LinearUITabBar*                    CurrentTabBar;
    LinearPool<LinearUITabBar>             TabBars;
    LinearVector<LinearUIPtrOrIndex>       CurrentTabBarStack;
    LinearVector<LinearUIShrinkWidthItem>  ShrinkWidthBuffer;

    // Multi-Select state
    LinearUIBoxSelectState             BoxSelectState;
    LinearUIMultiSelectTempData*       CurrentMultiSelect;
    int                             MultiSelectTempDataStacked; // Temporary multi-select data size (because we leave previous instances undestructed, we generally don't use MultiSelectTempData.Size)
    LinearVector<LinearUIMultiSelectTempData> MultiSelectTempData;
    LinearPool<LinearUIMultiSelectState>   MultiSelectStorage;

    // Hover Delay system
    LinearUIID                 HoverItemDelayId;
    LinearUIID                 HoverItemDelayIdPreviousFrame;
    float                   HoverItemDelayTimer;                // Currently used by IsItemHovered()
    float                   HoverItemDelayClearTimer;           // Currently used by IsItemHovered(): grace time before g.TooltipHoverTimer gets cleared.
    LinearUIID                 HoverItemUnlockedStationaryId;      // Mouse has once been stationary on this item. Only reset after departing the item.
    LinearUIID                 HoverWindowUnlockedStationaryId;    // Mouse has once been stationary on this window. Only reset after departing the window.

    // Mouse state
    LinearUIMouseCursor        MouseCursor;
    float                   MouseStationaryTimer;               // Time the mouse has been stationary (with some loose heuristic)
    LinearVec2                  MouseLastValidPos;

    // Widget state
    LinearUIInputTextState     InputTextState;
    LinearUITextIndex          InputTextLineIndex;                 // Temporary storage
    LinearUIInputTextDeactivatedState InputTextDeactivatedState;
    LinearFontBaked             InputTextPasswordFontBackupBaked;
    LinearFontFlags             InputTextPasswordFontBackupFlags;
    LinearUIID                 InputTextReactivateId;              // ID of InputText to reactivate on next frame (for io.ConfigInputTextEnterKeepActive behavior)
    LinearUIID                 TempInputId;                        // Temporary text input when using Ctrl+Click on a slider, etc.
    LinearUIDataTypeStorage    DataTypeZeroValue;                  // 0 for all data types
    int                     BeginMenuDepth;
    int                     BeginComboDepth;
    LinearUIColorEditFlags     ColorEditOptions;                   // Store user options for color edit widgets
    LinearUIID                 ColorEditCurrentID;                 // Set temporarily while inside of the parent-most ColorEdit4/ColorPicker4 (because they call each others).
    LinearUIID                 ColorEditSavedID;                   // ID we are saving/restoring HS for
    float                   ColorEditSavedHue;                  // Backup of last Hue associated to LastColor, so we can restore Hue in lossy RGB<>HSV round trips
    float                   ColorEditSavedSat;                  // Backup of last Saturation associated to LastColor, so we can restore Saturation in lossy RGB<>HSV round trips
    LinearU32                   ColorEditSavedColor;                // RGB value with alpha set to 0.
    LinearVec4                  ColorPickerRef;                     // Initial/reference color at the time of opening the color picker.
    LinearUIComboPreviewData   ComboPreviewData;
    LinearRect                  WindowResizeBorderExpectedRect;     // Expected border rect, switch to relative edit if moving
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
    LinearUIWindow*            TooltipPreviousWindow;              // Window of last tooltip submitted during the frame
    LinearVector<char>          ClipboardHandlerData;               // If no custom clipboard handler is defined
    LinearVector<LinearUIID>       MenusIdSubmittedThisFrame;          // A list of menu IDs that were rendered at least once
    LinearUITypingSelectState  TypingSelectState;                  // State for GetTypingSelectRequest()

    // Platform support
    LinearUIPlatformImeData    PlatformImeData;                    // Data updated by current frame. Will be applied at end of the frame. For some backends, this is required to have WantVisible=true in order to receive text message.
    LinearUIPlatformImeData    PlatformImeDataPrev;                // Previous frame data. When changed we call the platform_io.Platform_SetImeDataFn() handler.

    // Extensions
    // FIXME: We could provide an API to register one slot in an array held in LinearUIContext?
    LinearVector<LinearTextureData*> UserTextures;                      // List of textures created/managed by user or third-party extension. Automatically appended into platform_io.Textures[].
    LinearUIDockContext        DockContext;
    void                    (*DockNodeWindowMenuHandler)(LinearUIContext* ctx, LinearUIDockNode* node, LinearUITabBar* tab_bar);

    // Settings
    bool                    SettingsLoaded;
    float                   SettingsDirtyTimer;                 // Save .ini Settings to memory when time reaches zero
    LinearUITextBuffer         SettingsIniData;                    // In memory .ini settings
    LinearVector<LinearUISettingsHandler>      SettingsHandlers;       // List of .ini settings handlers
    LinearChunkStream<LinearUIWindowSettings>  SettingsWindows;        // LinearUIWindow .ini settings entries
    LinearChunkStream<LinearUITableSettings>   SettingsTables;         // LinearUITable .ini settings entries

    // Hooks
    LinearVector<LinearUIContextHook>          Hooks;                  // Hooks for extensions (e.g. test engine)
    LinearUIID                             HookIdNext;             // Next available HookId
    LinearUIDemoMarkerCallback             DemoMarkerCallback;

    // Localization
    const char*             LocalizationTable[LinearUILocKey_COUNT];

    // Capture/Logging
    bool                    LogEnabled;                         // Currently capturing
    bool                    LogLineFirstItem;
    LinearUILogFlags           LogFlags;                           // Capture flags/type
    LinearUIWindow*            LogWindow;
    LinearFileHandle            LogFile;                            // If != NULL log to stdout/ file
    LinearUITextBuffer         LogBuffer;                          // Accumulation buffer when log to clipboard. This is pointer so our GLinearUI static constructor doesn't call heap allocators.
    const char*             LogNextPrefix;                      // See comment in LogSetNextTextDecoration(): doesn't copy underlying data, use carefully!
    const char*             LogNextSuffix;
    float                   LogLinePosY;
    int                     LogDepthRef;
    int                     LogDepthToExpand;
    int                     LogDepthToExpandDefault;            // Default/stored value for LogDepthMaxExpand if not specified in the LogXXX function call.

    // Error Handling
    LinearUIErrorCallback      ErrorCallback;                      // = NULL. May be exposed in public API eventually.
    void*                   ErrorCallbackUserData;              // = NULL
    LinearVec2                  ErrorTooltipLockedPos;
    bool                    ErrorFirst;
    int                     ErrorCountCurrentFrame;             // [Internal] Number of errors submitted this frame.
    LinearUIErrorRecoveryState StackSizesInNewFrame;               // [Internal]
    LinearUIErrorRecoveryState*StackSizesInBeginForCurrentWindow;  // [Internal]

    // Debug Tools
    // (some of the highly frequently used data are interleaved in other structures above: DebugBreakXXX fields, DebugHookIdInfo, DebugLocateId etc.)
    int                     DebugDrawIdConflictsCount;          // Locked count (preserved when holding Ctrl)
    LinearUIDebugLogFlags      DebugLogFlags;
    LinearUITextBuffer         DebugLogBuf;
    LinearUITextIndex          DebugLogIndex;
    int                     DebugLogSkippedErrors;
    LinearUIDebugLogFlags      DebugLogAutoDisableFlags;
    LinearU8                    DebugLogAutoDisableFrames;
    LinearU8                    DebugLocateFrames;                  // For DebugLocateItemOnHover(). This is used together with DebugLocateId which is in a hot/cached spot above.
    bool                    DebugBreakInLocateId;               // Debug break in ItemAdd() call for g.DebugLocateId.
    LinearUIKeyChord           DebugBreakKeyChord;                 // = LinearUIKey_Pause
    LinearS8                    DebugBeginReturnValueCullDepth;     // Cycle between 0..9 then wrap around.
    bool                    DebugItemPickerActive;              // Item picker is active (started with DebugStartItemPicker())
    LinearU8                    DebugItemPickerMouseButton;
    LinearUIID                 DebugItemPickerBreakId;             // Will call LINEARUI_DEBUG_BREAK() when encountering this ID
    float                   DebugFlashStyleColorTime;
    LinearVec4                  DebugFlashStyleColorBackup;
    LinearUIMetricsConfig      DebugMetricsConfig;
    LinearUIDebugItemPathQuery DebugItemPathQuery;
    LinearUIIDStackTool        DebugIDStackTool;
    LinearUIDebugAllocInfo     DebugAllocInfo;
    LinearUIDockNode*          DebugHoveredDockNode;               // Hovered dock node.
#if defined(LINEARUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS) && !defined(LINEARUI_DISABLE_DEBUG_TOOLS)
    LinearUIStorage            DebugDrawIdConflictsAliveCount;
    LinearUIStorage            DebugDrawIdConflictsHighlightSet;
#endif

    // Misc
    float                   FramerateSecPerFrame[60];           // Calculate estimate of framerate for user over the last 60 frames..
    int                     FramerateSecPerFrameIdx;
    int                     FramerateSecPerFrameCount;
    float                   FramerateSecPerFrameAccum;
    int                     WantCaptureMouseNextFrame;          // Explicit capture override via SetNextFrameWantCaptureMouse()/SetNextFrameWantCaptureKeyboard(). Default to -1.
    int                     WantCaptureKeyboardNextFrame;       // "
    int                     WantTextInputNextFrame;             // Copied in EndFrame() from g.PlatformImeData.WantTextInput. Needs to be set for some backends (SDL3) to emit character inputs.
    LinearVector<char>          TempBuffer;                         // Temporary text buffer
    char                    TempKeychordName[64];

    LinearUIContext(LinearFontAtlas* shared_font_atlas);
    ~LinearUIContext();
};

//-----------------------------------------------------------------------------
// [SECTION] LinearUIWindowTempData, LinearUIWindow
//-----------------------------------------------------------------------------

#define LINEARUI_WINDOW_HARD_MIN_SIZE 4.0f

// Transient per-window data, reset at the beginning of the frame. This used to be called LinearUIDrawContext, hence the DC variable name in LinearUIWindow.
// (That's theory, in practice the delimitation between LinearUIWindow and LinearUIWindowTempData is quite tenuous and could be reconsidered..)
// (This doesn't need a constructor because we zero-clear it as part of LinearUIWindow and all frame-temporary data are setup on Begin)
struct LINEARUI_API LinearUIWindowTempData
{
    // Layout
    LinearVec2                  CursorPos;              // Current emitting position, in absolute coordinates.
    LinearVec2                  CursorPosPrevLine;
    LinearVec2                  CursorStartPos;         // Initial position after Begin(), generally ~ window position + WindowPadding.
    LinearVec2                  CursorMaxPos;           // Used to implicitly calculate ContentSize at the beginning of next frame, for scrolling range and auto-resize. Always growing during the frame.
    LinearVec2                  IdealMaxPos;            // Used to implicitly calculate ContentSizeIdeal at the beginning of next frame, for auto-resize only. Always growing during the frame.
    LinearVec2                  CurrLineSize;
    LinearVec2                  PrevLineSize;
    float                   CurrLineTextBaseOffset; // Baseline offset (0.0f by default on a new line, generally == style.FramePadding.y when a framed item has been added).
    float                   PrevLineTextBaseOffset;
    bool                    IsSameLine;
    bool                    IsSetPos;
    LinearVec1                  Indent;                 // Indentation / start position from left of window (increased by TreePush/TreePop, etc.)
    LinearVec1                  ColumnsOffset;          // Offset to the current column (if ColumnsCurrent > 0). FIXME: This and the above should be a stack to allow use cases like Tree->Column->Tree. Need revamp columns API.
    LinearVec1                  GroupOffset;
    LinearVec2                  CursorStartPosLossyness;// Record the loss of precision of CursorStartPos due to really large scrolling amount. This is used by clipper to compensate and fix the most common use case of large scroll area.

    // Keyboard/Gamepad navigation
    LinearUINavLayer           NavLayerCurrent;        // Current layer, 0..31 (we currently only use 0..1)
    short                   NavLayersActiveMask;    // Which layers have been written to (result from previous frame)
    short                   NavLayersActiveMaskNext;// Which layers have been written to (accumulator for current frame)
    bool                    NavIsScrollPushableX;   // Set when current work location may be scrolled horizontally when moving left / right. This is generally always true UNLESS within a column.
    bool                    NavHideHighlightOneFrame;
    bool                    NavWindowHasScrollY;    // Set per window when scrolling can be used (== ScrollMax.y > 0.0f)

    // Miscellaneous
    bool                    MenuBarAppending;       // FIXME: Remove this
    LinearVec2                  MenuBarOffset;          // MenuBarOffset.x is sort of equivalent of a per-layer CursorPos.x, saved/restored as we switch to the menu bar. The only situation when MenuBarOffset.y is > 0 if when (SafeAreaPadding.y > FramePadding.y), often used on TVs.
    LinearUIMenuColumns        MenuColumns;            // Simplified columns storage for menu items measurement
    int                     TreeDepth;              // Current tree depth.
    LinearU32                   TreeHasStackDataDepthMask;      // Store whether given depth has LinearUITreeNodeStackData data. Could be turned into a LinearU64 if necessary.
    LinearU32                   TreeRecordsClippedNodesY2Mask;  // Store whether we should keep recording Y2. Cleared when passing clip max. Equivalent TreeHasStackDataDepthMask value should always be set.
    LinearVector<LinearUIWindow*>  ChildWindows;
    LinearUIStorage*           StateStorage;           // Current persistent per-window storage (store e.g. tree node open/close state)
    LinearUIOldColumns*        CurrentColumns;         // Current columns set
    int                     CurrentTableIdx;        // Current table index (into g.Tables)
    LinearUILayoutType         LayoutType;
    LinearUILayoutType         ParentLayoutType;       // Layout type of parent window at the time of Begin()
    LinearU32                   ModalDimBgColor;

    // Status flags
    LinearUIItemStatusFlags    WindowItemStatusFlags;
    LinearUIItemStatusFlags    ChildItemStatusFlags;
    LinearUIItemStatusFlags    DockTabItemStatusFlags;
    LinearRect                  DockTabItemRect;

    // Local parameters stacks
    // We store the current settings outside of the vectors to increase memory locality (reduce cache misses). The vectors are rarely modified. Also it allows us to not heap allocate for short-lived windows which are not using those settings.
    float                   ItemWidth;              // Current item width (>0.0: width in pixels, <0.0: align xx pixels to the right of window).
    float                   ItemWidthDefault;
    float                   TextWrapPos;            // Current text wrap pos.
    LinearVector<float>         ItemWidthStack;         // Store item widths to restore (attention: .back() is not == ItemWidth)
    LinearVector<float>         TextWrapPosStack;       // Store text wrap pos to restore (attention: .back() is not == TextWrapPos)
};

// Storage for one window
struct LINEARUI_API LinearUIWindow
{
    LinearUIContext*           Ctx;                                // Parent UI context (needs to be set explicitly by parent).
    char*                   Name;                               // Window name, owned by the window.
    LinearUIID                 ID;                                 // == LinearHashStr(Name)
    LinearUIWindowFlags        Flags, FlagsPreviousFrame;          // See enum LinearUIWindowFlags_
    LinearUIChildFlags         ChildFlags;                         // Set when window is a child window. See enum LinearUIChildFlags_
    LinearUIWindowClass        WindowClass;                        // Advanced users only. Set with SetNextWindowClass()
    LinearUIViewportP*         Viewport;                           // Always set in Begin(). Inactive windows may have a NULL value here if their viewport was discarded.
    LinearUIID                 ViewportId;                         // We backup the viewport id (since the viewport may disappear or never be created if the window is inactive)
    LinearVec2                  ViewportPos;                        // We backup the viewport position (since the viewport may disappear or never be created if the window is inactive)
    int                     ViewportAllowPlatformMonitorExtend; // Reset to -1 every frame (index is guaranteed to be valid between NewFrame..EndFrame), only used in the Appearing frame of a tooltip/popup to enforce clamping to a given monitor
    LinearVec2                  Pos;                                // Position (always rounded-up to nearest pixel)
    LinearVec2                  Size;                               // Current size (==SizeFull or collapsed title bar size)
    LinearVec2                  SizeFull;                           // Size when non collapsed
    LinearVec2                  ContentSize;                        // Size of contents/scrollable client area (calculated from the extents reach of the cursor) from previous frame. Does not include window decoration or window padding.
    LinearVec2                  ContentSizeIdeal;
    LinearVec2                  ContentSizeExplicit;                // Size of contents/scrollable client area explicitly request by the user via SetNextWindowContentSize().
    LinearVec2                  WindowPadding;                      // Window padding at the time of Begin().
    float                   WindowRounding;                     // Window rounding at the time of Begin(). May be clamped lower to avoid rendering artifacts with title bar, menu bar etc.
    float                   WindowBorderSize;                   // Window border size at the time of Begin().
    float                   TitleBarHeight, MenuBarHeight;      // Note that those used to be function before 2024/05/28. If you have old code calling TitleBarHeight() you can change it to TitleBarHeight.
    float                   DecoOuterSizeX1, DecoOuterSizeY1;   // Left/Up offsets. Sum of non-scrolling outer decorations (X1 generally == 0.0f. Y1 generally = TitleBarHeight + MenuBarHeight). Locked during Begin().
    float                   DecoOuterSizeX2, DecoOuterSizeY2;   // Right/Down offsets (X2 generally == ScrollbarSize.x, Y2 == ScrollbarSizes.y).
    float                   DecoInnerSizeX1, DecoInnerSizeY1;   // Applied AFTER/OVER InnerRect. Specialized for Tables as they use specialized form of clipping and frozen rows/columns are inside InnerRect (and not part of regular decoration sizes).
    int                     NameBufLen;                         // Size of buffer storing Name. May be larger than strlen(Name)!
    LinearUIID                 MoveId;                             // == window->GetID("#MOVE")
    LinearUIID                 TabId;                              // == window->GetID("#TAB")
    LinearUIID                 ChildId;                            // ID of corresponding item in parent window (for navigation to return from child window to parent window)
    LinearUIID                 PopupId;                            // ID in the popup stack when this window is used as a popup/menu (because we use generic Name/ID for recycling)
    LinearVec2                  Scroll;
    LinearVec2                  ScrollMax;
    LinearVec2                  ScrollTarget;                       // target scroll position. stored as cursor position with scrolling canceled out, so the highest point is always 0.0f. (FLT_MAX for no change)
    LinearVec2                  ScrollTargetCenterRatio;            // 0.0f = scroll so that target position is at top, 0.5f = scroll so that target position is centered
    LinearVec2                  ScrollTargetEdgeSnapDist;           // 0.0f = no snapping, >0.0f snapping threshold
    LinearVec2                  ScrollbarSizes;                     // Size taken by each scrollbars on their smaller axis. Pay attention! ScrollbarSizes.x == width of the vertical scrollbar, ScrollbarSizes.y = height of the horizontal scrollbar.
    bool                    ScrollbarX, ScrollbarY;             // Are scrollbars visible?
    bool                    ScrollbarXStabilizeEnabled;         // Was ScrollbarX previously auto-stabilized?
    LinearU8                    ScrollbarXStabilizeToggledHistory;  // Used to stabilize scrollbar visibility in case of feedback loops
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
    short                   BeginOrderWithinContext;            // Begin() order within entire linearui context. This is mostly used for debugging submission order related issues.
    short                   FocusOrder;                         // Order within WindowsFocusOrder[], altered when windows are focused.
    LinearUIDir                AutoPosLastDirection;
    LinearS8                    AutoFitFramesX, AutoFitFramesY;
    bool                    AutoFitOnlyGrows;
    LinearS8                    HiddenFramesCanSkipItems;           // Hide the window for N frames
    LinearS8                    HiddenFramesCannotSkipItems;        // Hide the window for N frames while allowing items to be submitted so we can measure their size
    LinearS8                    HiddenFramesForRenderOnly;          // Hide the window until frame N at Render() time only
    LinearS8                    DisableInputsFrames;                // Disable window interactions for N frames
    LinearUIWindowBgClickFlags BgClickFlags : 8;                   // Configure behavior of click+dragging on window bg/void or over items. Default sets by io.ConfigWindowsMoveFromTitleBarOnly. If you use this please report in #3379.
    LinearUICond               SetWindowPosAllowFlags : 8;         // store acceptable condition flags for SetNextWindowPos() use.
    LinearUICond               SetWindowSizeAllowFlags : 8;        // store acceptable condition flags for SetNextWindowSize() use.
    LinearUICond               SetWindowCollapsedAllowFlags : 8;   // store acceptable condition flags for SetNextWindowCollapsed() use.
    LinearUICond               SetWindowDockAllowFlags : 8;        // store acceptable condition flags for SetNextWindowDock() use.
    LinearVec2                  SetWindowPosVal;                    // store window position when using a non-zero Pivot (position set needs to be processed when we know the window size)
    LinearVec2                  SetWindowPosPivot;                  // store window pivot for positioning. LinearVec2(0, 0) when positioning from top-left corner; LinearVec2(0.5f, 0.5f) for centering; LinearVec2(1, 1) for bottom right.

    LinearVector<LinearUIID>       IDStack;                            // ID stack. ID are hashes seeded with the value at the top of the stack. (In theory this should be in the TempData structure)
    LinearUIWindowTempData     DC;                                 // Temporary per-window data, reset at the beginning of the frame. This used to be called LinearUIDrawContext, hence the "DC" variable name.

    // The best way to understand what those rectangles are is to use the 'Metrics->Tools->Show Windows Rectangles' viewer.
    // The main 'OuterRect', omitted as a field, is window->Rect().
    LinearRect                  OuterRectClipped;                   // == Window->Rect() just after setup in Begin(). == window->Rect() for root window.
    LinearRect                  InnerRect;                          // Inner rectangle (omit title bar, menu bar, scroll bar)
    LinearRect                  InnerClipRect;                      // == InnerRect shrunk by WindowPadding*0.5f on each side, clipped within viewport or parent clip rect.
    LinearRect                  WorkRect;                           // Initially covers the whole scrolling region. Reduced by containers e.g columns/tables when active. Shrunk by WindowPadding*1.0f on each side. This is meant to replace ContentRegionRect over time (from 1.71+ onward).
    LinearRect                  ParentWorkRect;                     // Backup of WorkRect before entering a container such as columns/tables. Used by e.g. SpanAllColumns functions to easily access. Stacked containers are responsible for maintaining this. // FIXME-WORKRECT: Could be a stack?
    LinearRect                  ClipRect;                           // Current clipping/scissoring rectangle, evolve as we are using PushClipRect(), etc. == DrawList->clip_rect_stack.back().
    LinearRect                  ContentRegionRect;                  // FIXME: This is currently confusing/misleading. It is essentially WorkRect but not handling of scrolling. We currently rely on it as right/bottom aligned sizing operation need some size to rely on.
    LinearVec2ih                HitTestHoleSize;                    // Define an optional rectangular hole where mouse will pass-through the window.
    LinearVec2ih                HitTestHoleOffset;

    int                     LastFrameActive;                    // Last frame number the window was Active.
    int                     LastFrameJustFocused;               // Last frame number the window was made Focused.
    float                   LastTimeActive;                     // Last timestamp the window was Active (using float as we don't need high precision there)
    LinearUIStorage            StateStorage;
    LinearVector<LinearUIOldColumns> ColumnsStorage;
    float                   FontWindowScale;                    // User scale multiplier per-window, via SetWindowFontScale()
    float                   FontWindowScaleParents;
    float                   FontRefSize;                        // This is a copy of window->CalcFontSize() at the time of Begin(), trying to phase out CalcFontSize() especially as it may be called on non-current window.
    int                     SettingsOffset;                     // Offset into SettingsWindows[] (offsets are always valid as we only grow the array from the back)

    LinearDrawList*             DrawList;                           // == &DrawListInst (for backward compatibility reason with code using linearui_internal.h we keep this a pointer)
    LinearDrawList              DrawListInst;
    LinearUIWindow*            ParentWindow;                       // If we are a child _or_ popup _or_ docked window, this is pointing to our parent. Otherwise NULL.
    LinearUIWindow*            ParentWindowInBeginStack;
    LinearUIWindow*            RootWindow;                         // Point to ourself or first ancestor that is not a child window. Doesn't cross through popups/dock nodes.
    LinearUIWindow*            RootWindowPopupTree;                // Point to ourself or first ancestor that is not a child window. Cross through popups parent<>child.
    LinearUIWindow*            RootWindowDockTree;                 // Point to ourself or first ancestor that is not a child window. Cross through dock nodes.
    LinearUIWindow*            RootWindowForTitleBarHighlight;     // Point to ourself or first ancestor which will display TitleBgActive color when this window is active.
    LinearUIWindow*            RootWindowForNav;                   // Point to ourself or first ancestor which doesn't have the NavFlattened flag.
    LinearUIWindow*            ParentWindowForFocusRoute;          // Set to manual link a window to its logical parent so that Shortcut() chain are honored (e.g. Tool linked to Document)

    LinearUIWindow*            NavLastChildNavWindow;              // When going to the menu bar, we remember the child window we came from. (This could probably be made implicit if we kept g.Windows sorted by last focused including child window.)
    LinearUIID                 NavLastIds[LinearUINavLayer_COUNT];    // Last known NavId for this window, per layer (0/1)
    LinearRect                  NavRectRel[LinearUINavLayer_COUNT];    // Reference rectangle, in window relative space
    LinearVec2                  NavPreferredScoringPosRel[LinearUINavLayer_COUNT]; // Preferred X/Y position updated when moving on a given axis, reset to FLT_MAX.
    LinearUIID                 NavRootFocusScopeId;                // Focus Scope ID at the time of Begin()

    int                     MemoryDrawListIdxCapacity;          // Backup of last idx/vtx count, so when waking up the window we can preallocate and avoid iterative alloc/copy
    int                     MemoryDrawListVtxCapacity;
    bool                    MemoryCompacted;                    // Set when window extraneous data have been garbage collected

    // Docking
    bool                    DockIsActive        :1;             // When docking artifacts are actually visible. When this is set, DockNode is guaranteed to be != NULL. ~~ (DockNode != NULL) && (DockNode->Windows.Size > 1).
    bool                    DockNodeIsVisible   :1;
    bool                    DockTabIsVisible    :1;             // Is our window visible this frame? ~~ is the corresponding tab selected?
    bool                    DockTabWantClose    :1;
    short                   DockOrder;                          // Order of the last time the window was visible within its DockNode. This is used to reorder windows that are reappearing on the same frame. Same value between windows that were active and windows that were none are possible.
    LinearUIWindowDockStyle    DockStyle;
    LinearUIDockNode*          DockNode;                           // Which node are we docked into. Important: Prefer testing DockIsActive in many cases as this will still be set when the dock node is hidden.
    LinearUIDockNode*          DockNodeAsHost;                     // Which node are we owning (for parent windows)
    LinearUIID                 DockId;                             // Backup of last valid DockNode->ID, so single window remember their dock node id even when they are not bound any more

public:
    LinearUIWindow(LinearUIContext* context, const char* name);
    ~LinearUIWindow();

    LinearUIID     GetID(const char* str, const char* str_end = NULL);
    LinearUIID     GetID(const void* ptr);
    LinearUIID     GetID(int n);
    LinearUIID     GetIDFromPos(const LinearVec2& p_abs);
    LinearUIID     GetIDFromRectangle(const LinearRect& r_abs);

    // We don't use g.FontSize because the window may be != g.CurrentWindow.
    LinearRect      Rect() const            { return LinearRect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y); }
    LinearRect      TitleBarRect() const    { return LinearRect(Pos, LinearVec2(Pos.x + SizeFull.x, Pos.y + TitleBarHeight)); }
    LinearRect      MenuBarRect() const     { float y1 = Pos.y + TitleBarHeight; return LinearRect(Pos.x, y1, Pos.x + SizeFull.x, y1 + MenuBarHeight); }

    // [OBSOLETE] LinearUIWindow::CalcFontSize() was removed in 1.92.0 because error-prone/misleading. You can use window->FontRefSize for a copy of g.FontSize at the time of the last Begin() call for this window.
    //float     CalcFontSize() const    { LinearUIContext& g = *Ctx; return g.FontSizeBase * FontWindowScale * FontDpiScale * FontWindowScaleParents;
};

//-----------------------------------------------------------------------------
// [SECTION] Tab bar, Tab item support
//-----------------------------------------------------------------------------

// Extend LinearUITabBarFlags_
enum LinearUITabBarFlagsPrivate_
{
    LinearUITabBarFlags_DockNode                   = 1 << 20,  // Part of a dock node [we don't use this in the master branch but it facilitate branch syncing to keep this around]
    LinearUITabBarFlags_IsFocused                  = 1 << 21,
    LinearUITabBarFlags_SaveSettings               = 1 << 22,  // FIXME: Settings are handled by the docking system, this only request the tab bar to mark settings dirty when reordering tabs
};

// Extend LinearUITabItemFlags_
enum LinearUITabItemFlagsPrivate_
{
    LinearUITabItemFlags_SectionMask_              = LinearUITabItemFlags_Leading | LinearUITabItemFlags_Trailing,
    LinearUITabItemFlags_NoCloseButton             = 1 << 20,  // Track whether p_open was set or not (we'll need this info on the next frame to recompute ContentWidth during layout)
    LinearUITabItemFlags_Button                    = 1 << 21,  // Used by TabItemButton, change the tab item behavior to mimic a button
    LinearUITabItemFlags_Invisible                 = 1 << 22,  // To reserve space e.g. with LinearUITabItemFlags_Leading
    LinearUITabItemFlags_Unsorted                  = 1 << 23,  // [Docking] Trailing tabs with the _Unsorted flag will be sorted based on the DockOrder of their Window.
};

// Storage for one active tab item (sizeof() 48 bytes)
struct LinearUITabItem
{
    LinearUIID             ID;
    LinearUITabItemFlags   Flags;
    LinearUIWindow*        Window;                 // When TabItem is part of a DockNode's TabBar, we hold on to a window.
    int                 LastFrameVisible;
    int                 LastFrameSelected;      // This allows us to infer an ordered list of the last activated tabs with little maintenance
    float               Offset;                 // Position relative to beginning of tab bar
    float               Width;                  // Width currently displayed
    float               ContentWidth;           // Width of label + padding, stored during BeginTabItem() call (misnamed as "Content" would normally imply width of label only)
    float               RequestedWidth;         // Width optionally requested by caller, -1.0f is unused
    LinearS32               NameOffset;             // When Window==NULL, offset to name within parent LinearUITabBar::TabsNames
    LinearS16               BeginOrder;             // BeginTabItem() order, used to re-order tabs after toggling LinearUITabBarFlags_Reorderable
    LinearS16               IndexDuringLayout;      // Index only used during TabBarLayout(). Tabs gets reordered so 'Tabs[n].IndexDuringLayout == n' but may mismatch during additions.
    bool                WantClose;              // Marked as closed by SetTabItemClosed()

    LinearUITabItem()      { memset((void*)this, 0, sizeof(*this)); LastFrameVisible = LastFrameSelected = -1; RequestedWidth = -1.0f; NameOffset = -1; BeginOrder = IndexDuringLayout = -1; }
};

// Storage for a tab bar (sizeof() 160 bytes)
struct LINEARUI_API LinearUITabBar
{
    LinearUIWindow*        Window;
    LinearVector<LinearUITabItem> Tabs;
    LinearUITabBarFlags    Flags;
    LinearUIID             ID;                     // Zero for tab-bars used by docking
    LinearUIID             SelectedTabId;          // Selected tab/window
    LinearUIID             NextSelectedTabId;      // Next selected tab/window. Will also trigger a scrolling animation
    LinearUIID             NextScrollToTabId;
    LinearUIID             VisibleTabId;           // Can occasionally be != SelectedTabId (e.g. when previewing contents for Ctrl+Tab preview)
    int                 CurrFrameVisible;
    int                 PrevFrameVisible;
    LinearRect              BarRect;
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
    LinearUIID             ReorderRequestTabId;
    LinearS16               ReorderRequestOffset;
    LinearS8                BeginCount;
    bool                WantLayout;
    bool                VisibleTabWasSubmitted;
    bool                TabsAddedNew;           // Set to true when a new tab item or button has been added to the tab bar during last frame
    bool                ScrollButtonEnabled;
    LinearS16               TabsActiveCount;        // Number of tabs submitted this frame.
    LinearS16               LastTabItemIdx;         // Index of last BeginTabItem() tab for use by EndTabItem()
    float               ItemSpacingY;
    LinearVec2              FramePadding;           // style.FramePadding locked at the time of BeginTabBar()
    LinearVec2              BackupCursorPos;
    LinearUITextBuffer     TabsNames;              // For non-docking tab bar we re-append names in a contiguous buffer.

    LinearUITabBar();
};

//-----------------------------------------------------------------------------
// [SECTION] Table support
//-----------------------------------------------------------------------------

#define LINEARUI_COL32_DISABLE                LINEARUI_COL32(0,0,0,1)   // Special sentinel code which cannot be used as a regular color.
#define LINEARUI_TABLE_MAX_COLUMNS         512                 // Arbitrary "safety" maximum, may be lifted in the future if needed. Must fit in LinearUITableColumnIdx/LinearUITableDrawChannelIdx.

// [Internal] sizeof() ~ 112
// We use the terminology "Enabled" to refer to a column that is not Hidden by user/api.
// We use the terminology "Clipped" to refer to a column that is out of sight because of scrolling/clipping.
// This is in contrast with some user-facing api such as IsItemVisible() / IsRectVisible() which use "Visible" to mean "not clipped".
struct LinearUITableColumn
{
    LinearUITableColumnFlags   Flags;                          // Flags after some patching (not directly same as provided by user). See LinearUITableColumnFlags_
    float                   WidthGiven;                     // Final/actual width visible == (MaxX - MinX), locked in TableUpdateLayout(). May be > WidthRequest to honor minimum width, may be < WidthRequest to honor shrinking columns down in tight space.
    float                   MinX;                           // Absolute positions
    float                   MaxX;
    float                   WidthRequest;                   // Master width absolute value when !(Flags & _WidthStretch). When Stretch this is derived every frame from StretchWeight in TableUpdateLayout()
    float                   WidthAuto;                      // Automatic width
    float                   WidthMax;                       // Maximum width (FIXME: overwritten by each instance)
    float                   StretchWeight;                  // Master width weight when (Flags & _WidthStretch). Often around ~1.0f initially.
    float                   InitStretchWeightOrWidth;       // Value passed to TableSetupColumn(). For Width it is a content width (_without padding_).
    LinearRect                  ClipRect;                       // Clipping rectangle for the column
    LinearUIID                 UserID;                         // Optional, value passed to TableSetupColumn()
    float                   WorkMinX;                       // Contents region min ~(MinX + CellPaddingX + CellSpacingX1) == cursor start position when entering column
    float                   WorkMaxX;                       // Contents region max ~(MaxX - CellPaddingX - CellSpacingX2)
    float                   ItemWidth;                      // Current item width for the column, preserved across rows
    float                   ContentMaxXFrozen;              // Contents maximum position for frozen rows (apart from headers), from which we can infer content width.
    float                   ContentMaxXUnfrozen;
    float                   ContentMaxXHeadersUsed;         // Contents maximum position for headers rows (regardless of freezing). TableHeader() automatically softclip itself + report ideal desired size, to avoid creating extraneous draw calls
    float                   ContentMaxXHeadersIdeal;
    LinearS16                   NameOffset;                     // Offset into parent ColumnsNames[]
    LinearUITableColumnIdx     DisplayOrder;                   // Index within Table's IndexToDisplayOrder[] (column may be reordered by users)
    LinearUITableColumnIdx     IndexWithinEnabledSet;          // Index within enabled/visible set (<= IndexToDisplayOrder)
    LinearUITableColumnIdx     PrevEnabledColumn;              // Index of prev enabled/visible column within Columns[], -1 if first enabled/visible column
    LinearUITableColumnIdx     NextEnabledColumn;              // Index of next enabled/visible column within Columns[], -1 if last enabled/visible column
    LinearUITableColumnIdx     SortOrder;                      // Index of this column within sort specs, -1 if not sorting on this column, 0 for single-sort, may be >0 on multi-sort
    LinearUITableDrawChannelIdx DrawChannelCurrent;            // Index within DrawSplitter.Channels[]
    LinearUITableDrawChannelIdx DrawChannelFrozen;             // Draw channels for frozen rows (often headers)
    LinearUITableDrawChannelIdx DrawChannelUnfrozen;           // Draw channels for unfrozen rows
    bool                    IsEnabled;                      // IsUserEnabled && (Flags & LinearUITableColumnFlags_Disabled) == 0
    bool                    IsUserEnabled;                  // Is the column not marked Hidden by the user? (unrelated to being off view, e.g. clipped by scrolling).
    bool                    IsUserEnabledNextFrame;
    bool                    IsVisibleX;                     // Is actually in view (e.g. overlapping the host window clipping rectangle, not scrolled).
    bool                    IsVisibleY;
    bool                    IsRequestOutput;                // Return value for TableSetColumnIndex() / TableNextColumn(): whether we request user to output contents or not.
    bool                    IsSkipItems;                    // Do we want item submissions to this column to be completely ignored (no layout will happen).
    bool                    IsPreserveWidthAuto;
    LinearS8                    NavLayerCurrent;                // LinearUINavLayer in 1 byte
    LinearU8                    AutoFitQueue;                   // Queue of 8 values for the next 8 frames to request auto-fit
    LinearU8                    CannotSkipItemsQueue;           // Queue of 8 values for the next 8 frames to disable Clipped/SkipItem
    LinearU8                    SortDirection : 2;              // LinearUISortDirection_Ascending or LinearUISortDirection_Descending
    LinearU8                    SortDirectionsAvailCount : 2;   // Number of available sort directions (0 to 3)
    LinearU8                    SortDirectionsAvailMask : 4;    // Mask of available sort directions (1-bit each)
    LinearU8                    SortDirectionsAvailList;        // Ordered list of available sort directions (2-bits each, total 8-bits)

    LinearUITableColumn()
    {
        memset((void*)this, 0, sizeof(*this));
        StretchWeight = WidthRequest = -1.0f;
        NameOffset = -1;
        DisplayOrder = IndexWithinEnabledSet = -1;
        PrevEnabledColumn = NextEnabledColumn = -1;
        SortOrder = -1;
        SortDirection = LinearUISortDirection_None;
        DrawChannelCurrent = DrawChannelFrozen = DrawChannelUnfrozen = (LinearU8)-1;
    }
};

// Transient cell data stored per row.
// sizeof() ~ 6 bytes
struct LinearUITableCellData
{
    LinearU32                       BgColor;    // Actual color
    LinearUITableColumnIdx         Column;     // Column number
};

// Parameters for TableAngledHeadersRowEx()
// This may end up being refactored for more general purpose.
// sizeof() ~ 12 bytes
struct LinearUITableHeaderData
{
    LinearUITableColumnIdx         Index;      // Column index
    LinearU32                       TextColor;
    LinearU32                       BgColor0;
    LinearU32                       BgColor1;
};

// Per-instance data that needs preserving across frames (seemingly most others do not need to be preserved aside from debug needs. Does that means they could be moved to LinearUITableTempData?)
// sizeof() ~ 24 bytes
struct LinearUITableInstanceData
{
    LinearUIID                     TableInstanceID;
    float                       LastOuterHeight;            // Outer height from last frame
    float                       LastTopHeadersRowHeight;    // Height of first consecutive header rows from last frame (FIXME: this is used assuming consecutive headers are in same frozen set)
    float                       LastFrozenHeight;           // Height of frozen section from last frame
    int                         HoveredRowLast;             // Index of row which was hovered last frame.
    int                         HoveredRowNext;             // Index of row hovered this frame, set after encountering it.

    LinearUITableInstanceData()    { TableInstanceID = 0; LastOuterHeight = LastTopHeadersRowHeight = LastFrozenHeight = 0.0f; HoveredRowLast = HoveredRowNext = -1; }
};

// sizeof() ~ 592 bytes + heap allocs described in TableBeginInitMemory()
struct LINEARUI_API LinearUITable
{
    LinearUIID                     ID;
    LinearUITableFlags             Flags;
    void*                       RawData;                    // Single allocation to hold Columns[], DisplayOrderToIndex[], and RowCellData[]
    LinearUITableTempData*         TempData;                   // Transient data while table is active. Point within g.CurrentTableStack[]
    LinearSpan<LinearUITableColumn>    Columns;                    // Point within RawData[]
    LinearSpan<LinearUITableColumnIdx> DisplayOrderToIndex;        // Point within RawData[]. Store display order of columns (when not reordered, the values are 0...Count-1)
    LinearSpan<LinearUITableCellData>  RowCellData;                // Point within RawData[]. Store cells background requests for current row.
    LinearBitArrayPtr               EnabledMaskByDisplayOrder;  // Column DisplayOrder -> IsEnabled map
    LinearBitArrayPtr               EnabledMaskByIndex;         // Column Index -> IsEnabled map (== not hidden by user/api) in a format adequate for iterating column without touching cold data
    LinearBitArrayPtr               VisibleMaskByIndex;         // Column Index -> IsVisibleX|IsVisibleY map (== not hidden by user/api && not hidden by scrolling/cliprect)
    LinearUITableFlags             SettingsLoadedFlags;        // Which data were loaded from the .ini file (e.g. when order is not altered we won't save order)
    int                         SettingsOffset;             // Offset in g.SettingsTables
    int                         LastFrameActive;
    int                         ColumnsCount;               // Number of columns declared in BeginTable()
    int                         CurrentRow;
    int                         CurrentColumn;
    LinearS16                       InstanceCurrent;            // Count of BeginTable() calls with same ID in the same frame (generally 0). This is a little bit similar to BeginCount for a window, but multiple tables with the same ID are multiple tables, they are just synced.
    LinearS16                       InstanceInteracted;         // Mark which instance (generally 0) of the same ID is being interacted with
    float                       RowPosY1;
    float                       RowPosY2;
    float                       RowMinHeight;               // Height submitted to TableNextRow()
    float                       RowCellPaddingY;            // Top and bottom padding. Reloaded during row change.
    float                       RowTextBaseline;
    float                       RowIndentOffsetX;
    LinearUITableRowFlags          RowFlags : 16;              // Current row flags, see LinearUITableRowFlags_
    LinearUITableRowFlags          LastRowFlags : 16;
    int                         RowBgColorCounter;          // Counter for alternating background colors (can be fast-forwarded by e.g clipper), not same as CurrentRow because header rows typically don't increase this.
    LinearU32                       RowBgColor[2];              // Background color override for current row.
    LinearU32                       BorderColorStrong;
    LinearU32                       BorderColorLight;
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
    LinearRect                      OuterRect;                  // Note: for non-scrolling table, OuterRect.Max.y is often FLT_MAX until EndTable(), unless a height has been specified in BeginTable().
    LinearRect                      InnerRect;                  // InnerRect but without decoration. As with OuterRect, for non-scrolling tables, InnerRect.Max.y is "
    LinearRect                      WorkRect;
    LinearRect                      InnerClipRect;
    LinearRect                      BgClipRect;                 // We use this to cpu-clip cell background color fill, evolve during the frame as we cross frozen rows boundaries
    LinearRect                      Bg0ClipRectForDrawCmd;      // Actual LinearDrawCmd clip rect for BG0/1 channel. This tends to be == OuterWindow->ClipRect at BeginTable() because output in BG0/BG1 is cpu-clipped
    LinearRect                      Bg2ClipRectForDrawCmd;      // Actual LinearDrawCmd clip rect for BG2 channel. This tends to be a correct, tight-fit, because output to BG2 are done by widgets relying on regular ClipRect.
    LinearRect                      HostClipRect;               // This is used to check if we can eventually merge our columns draw calls into the current draw call of the current window.
    LinearRect                      HostBackupInnerClipRect;    // Backup of InnerWindow->ClipRect during PushTableBackground()/PopTableBackground()
    LinearUIWindow*                OuterWindow;                // Parent window for the table
    LinearUIWindow*                InnerWindow;                // Window holding the table data (== OuterWindow or a child window)
    LinearUITextBuffer             ColumnsNames;               // Contiguous buffer holding columns names
    LinearDrawListSplitter*         DrawSplitter;               // Shortcut to TempData->DrawSplitter while in table. Isolate draw commands per columns to avoid switching clip rect constantly
    LinearUITableInstanceData      InstanceDataFirst;
    LinearVector<LinearUITableInstanceData>    InstanceDataExtra;  // FIXME-OPT: Using a small-vector pattern would be good.
    LinearUITableColumnSortSpecs   SortSpecsSingle;
    LinearVector<LinearUITableColumnSortSpecs> SortSpecsMulti;     // FIXME-OPT: Using a small-vector pattern would be good.
    LinearUITableSortSpecs         SortSpecs;                  // Public facing sorts specs, this is what we return in TableGetSortSpecs()
    LinearUITableColumnIdx         SortSpecsCount;
    LinearUITableColumnIdx         ColumnsEnabledCount;        // Number of enabled columns (<= ColumnsCount)
    LinearUITableColumnIdx         ColumnsEnabledFixedCount;   // Number of enabled columns using fixed width (<= ColumnsCount)
    LinearUITableColumnIdx         DeclColumnsCount;           // Count calls to TableSetupColumn()
    LinearUITableColumnIdx         AngledHeadersCount;         // Count columns with angled headers
    LinearUITableColumnIdx         HoveredColumnBody;          // Index of column whose visible region is being hovered. Important: == ColumnsCount when hovering empty region after the right-most column!
    LinearUITableColumnIdx         HoveredColumnBorder;        // Index of column whose right-border is being hovered (for resizing).
    LinearUITableColumnIdx         HighlightColumnHeader;      // Index of column which should be highlighted.
    LinearUITableColumnIdx         AutoFitSingleColumn;        // Index of single column requesting auto-fit.
    LinearUITableColumnIdx         ResizedColumn;              // Index of column being resized. Reset when InstanceCurrent==0.
    LinearUITableColumnIdx         LastResizedColumn;          // Index of column being resized from previous frame.
    LinearUITableColumnIdx         HeldHeaderColumn;           // Index of column header being held.
    LinearUITableColumnIdx         LastHeldHeaderColumn;       // Index of column header being held from previous frame.
    LinearUITableColumnIdx         ReorderColumn;              // Index of column being reordered. (not cleared)
    LinearUITableColumnIdx         ReorderColumnDstOrder;      // Requested display order of column being reordered.
    LinearUITableColumnIdx         LeftMostEnabledColumn;      // Index of left-most non-hidden column.
    LinearUITableColumnIdx         RightMostEnabledColumn;     // Index of right-most non-hidden column.
    LinearUITableColumnIdx         LeftMostStretchedColumn;    // Index of left-most stretched column.
    LinearUITableColumnIdx         RightMostStretchedColumn;   // Index of right-most stretched column.
    LinearUITableColumnIdx         ContextPopupColumn;         // Column right-clicked on, of -1 if opening context menu from a neutral/empty spot
    LinearUITableColumnIdx         FreezeRowsRequest;          // Requested frozen rows count
    LinearUITableColumnIdx         FreezeRowsCount;            // Actual frozen row count (== FreezeRowsRequest, or == 0 when no scrolling offset)
    LinearUITableColumnIdx         FreezeColumnsRequest;       // Requested frozen columns count
    LinearUITableColumnIdx         FreezeColumnsCount;         // Actual frozen columns count (== FreezeColumnsRequest, or == 0 when no scrolling offset)
    LinearUITableColumnIdx         RowCellDataCurrent;         // Index of current RowCellData[] entry in current row
    LinearUITableDrawChannelIdx    DummyDrawChannel;           // Redirect non-visible columns here.
    LinearUITableDrawChannelIdx    Bg2DrawChannelCurrent;      // For Selectable() and other widgets drawing across columns after the freezing line. Index within DrawSplitter.Channels[]
    LinearUITableDrawChannelIdx    Bg2DrawChannelUnfrozen;
    LinearS8                        NavLayer;                   // LinearUINavLayer at the time of BeginTable().
    bool                        IsLayoutLocked;             // Set by TableUpdateLayout() which is called when beginning the first row.
    bool                        IsInsideRow;                // Set when inside TableBeginRow()/TableEndRow().
    bool                        IsInitializing;
    bool                        IsSortSpecsDirty;
    bool                        IsUsingHeaders;             // Set when the first row had the LinearUITableRowFlags_Headers flag.
    bool                        IsContextPopupOpen;         // Set when default context menu is open (also see: ContextPopupColumn, InstanceInteracted).
    bool                        DisableDefaultContextMenu;  // Disable default context menu. You may submit your own using TableBeginContextMenuPopup()/EndPopup()
    bool                        IsSettingsRequestLoad;
    bool                        IsSettingsDirty;            // Set when table settings have changed and needs to be reported into LinearUITableSettings data.
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

    LinearUITable()                { memset((void*)this, 0, sizeof(*this)); LastFrameActive = -1; }
    ~LinearUITable()               { LINEARUI_FREE(RawData); }
};

// Transient data that are only needed between BeginTable() and EndTable(), those buffers are shared (1 per level of stacked table).
// - Accessing those requires chasing an extra pointer so for very frequently used data we leave them in the main table structure.
// - We also leave out of this structure data that tend to be particularly useful for debugging/metrics.
// FIXME-TABLE: more transient data could be stored in a stacked LinearUITableTempData: e.g. SortSpecs.
// sizeof() ~ 136 bytes.
struct LINEARUI_API LinearUITableTempData
{
    LinearUIID                     WindowID;                   // Shortcut to g.Tables[TableIndex]->OuterWindow->ID.
    int                         TableIndex;                 // Index in g.Tables.Buf[] pool
    float                       LastTimeActive;             // Last timestamp this structure was used
    float                       AngledHeadersExtraWidth;    // Used in EndTable()
    LinearVector<LinearUITableHeaderData> AngledHeadersRequests;   // Used in TableAngledHeadersRow()

    LinearVec2                      UserOuterSize;              // outer_size.x passed to BeginTable()
    LinearDrawListSplitter          DrawSplitter;

    LinearRect                      HostBackupWorkRect;         // Backup of InnerWindow->WorkRect at the end of BeginTable()
    LinearRect                      HostBackupParentWorkRect;   // Backup of InnerWindow->ParentWorkRect at the end of BeginTable()
    LinearVec2                      HostBackupPrevLineSize;     // Backup of InnerWindow->DC.PrevLineSize at the end of BeginTable()
    LinearVec2                      HostBackupCurrLineSize;     // Backup of InnerWindow->DC.CurrLineSize at the end of BeginTable()
    LinearVec2                      HostBackupCursorMaxPos;     // Backup of InnerWindow->DC.CursorMaxPos at the end of BeginTable()
    LinearVec1                      HostBackupColumnsOffset;    // Backup of OuterWindow->DC.ColumnsOffset at the end of BeginTable()
    float                       HostBackupItemWidth;        // Backup of OuterWindow->DC.ItemWidth at the end of BeginTable()
    int                         HostBackupItemWidthStackSize;//Backup of OuterWindow->DC.ItemWidthStack.Size at the end of BeginTable()

    LinearUITableTempData()        { memset((void*)this, 0, sizeof(*this)); LastTimeActive = -1.0f; }
};

// sizeof() ~ 16
struct LinearUITableColumnSettings
{
    float                   WidthOrWeight;
    LinearUIID                 UserID;
    LinearUITableColumnIdx     Index;
    LinearUITableColumnIdx     DisplayOrder;
    LinearUITableColumnIdx     SortOrder;
    LinearU8                    SortDirection : 2;
    LinearS8                    IsEnabled : 2; // "Visible" in ini file
    LinearU8                    IsStretch : 1;

    LinearUITableColumnSettings()
    {
        WidthOrWeight = 0.0f;
        UserID = 0;
        Index = -1;
        DisplayOrder = SortOrder = -1;
        SortDirection = LinearUISortDirection_None;
        IsEnabled = -1;
        IsStretch = 0;
    }
};

// This is designed to be stored in a single LinearChunkStream (1 header followed by N LinearUITableColumnSettings, etc.)
struct LinearUITableSettings
{
    LinearUIID                     ID;                     // Set to 0 to invalidate/delete the setting
    LinearUITableFlags             SaveFlags;              // Indicate data we want to save using the Resizable/Reorderable/Sortable/Hideable flags (could be using its own flags..)
    float                       RefScale;               // Reference scale to be able to rescale columns on font/dpi changes.
    LinearUITableColumnIdx         ColumnsCount;
    LinearUITableColumnIdx         ColumnsCountMax;        // Maximum number of columns this settings instance can store, we can recycle a settings instance with lower number of columns but not higher
    bool                        WantApply;              // Set when loaded from .ini data (to enable merging/loading .ini data into an already running context)

    LinearUITableSettings()        { memset((void*)this, 0, sizeof(*this)); }
    LinearUITableColumnSettings*   GetColumnSettings()     { return (LinearUITableColumnSettings*)(this + 1); }
};

//-----------------------------------------------------------------------------
// [SECTION] LinearUI internal API
// No guarantee of forward compatibility here!
//-----------------------------------------------------------------------------

namespace LinearUI
{
    // Windows
    // We should always have a CurrentWindow in the stack (there is an implicit "Debug" window)
    // If this ever crashes because g.CurrentWindow is NULL, it means that either:
    // - LinearUI::NewFrame() has never been called, which is illegal.
    // - You are calling LinearUI functions after LinearUI::EndFrame()/LinearUI::Render() and before the next LinearUI::NewFrame(), which is also illegal.
    LINEARUI_API LinearUIIO&         GetIO(LinearUIContext* ctx);
    LINEARUI_API LinearUIPlatformIO& GetPlatformIO(LinearUIContext* ctx);
    inline    float         GetScale()                  { LinearUIContext& g = *GLinearUI; return g.Style._MainScale; } // FIXME-DPI: I don't want to formalize this just yet. Because reasons. Please don't use.
    inline    LinearUIWindow*  GetCurrentWindowRead()      { LinearUIContext& g = *GLinearUI; return g.CurrentWindow; }
    inline    LinearUIWindow*  GetCurrentWindow()          { LinearUIContext& g = *GLinearUI; g.CurrentWindow->WriteAccessed = true; return g.CurrentWindow; }
    LINEARUI_API LinearUIWindow*  FindWindowByID(LinearUIID id);
    LINEARUI_API LinearUIWindow*  FindWindowByName(const char* name);
    LINEARUI_API void          UpdateWindowParentAndRootLinks(LinearUIWindow* window, LinearUIWindowFlags flags, LinearUIWindow* parent_window);
    LINEARUI_API void          UpdateWindowSkipRefresh(LinearUIWindow* window);
    LINEARUI_API LinearVec2        CalcWindowNextAutoFitSize(LinearUIWindow* window);
    LINEARUI_API bool          IsWindowChildOf(LinearUIWindow* window, LinearUIWindow* potential_parent, bool popup_hierarchy, bool dock_hierarchy);
    LINEARUI_API bool          IsWindowInBeginStack(LinearUIWindow* window);
    LINEARUI_API bool          IsWindowWithinBeginStackOf(LinearUIWindow* window, LinearUIWindow* potential_parent);
    LINEARUI_API bool          IsWindowAbove(LinearUIWindow* potential_above, LinearUIWindow* potential_below);
    LINEARUI_API bool          IsWindowNavFocusable(LinearUIWindow* window);
    LINEARUI_API void          SetWindowPos(LinearUIWindow* window, const LinearVec2& pos, LinearUICond cond = 0);
    LINEARUI_API void          SetWindowSize(LinearUIWindow* window, const LinearVec2& size, LinearUICond cond = 0);
    LINEARUI_API void          SetWindowCollapsed(LinearUIWindow* window, bool collapsed, LinearUICond cond = 0);
    LINEARUI_API void          SetWindowHitTestHole(LinearUIWindow* window, const LinearVec2& pos, const LinearVec2& size);
    LINEARUI_API void          SetWindowHiddenAndSkipItemsForCurrentFrame(LinearUIWindow* window);
    inline void             SetWindowParentWindowForFocusRoute(LinearUIWindow* window, LinearUIWindow* parent_window) { window->ParentWindowForFocusRoute = parent_window; } // You may also use SetNextWindowClass()'s FocusRouteParentWindowId field.
    inline LinearRect           WindowRectAbsToRel(LinearUIWindow* window, const LinearRect& r) { LinearVec2 off = window->DC.CursorStartPos; return LinearRect(r.Min.x - off.x, r.Min.y - off.y, r.Max.x - off.x, r.Max.y - off.y); }
    inline LinearRect           WindowRectRelToAbs(LinearUIWindow* window, const LinearRect& r) { LinearVec2 off = window->DC.CursorStartPos; return LinearRect(r.Min.x + off.x, r.Min.y + off.y, r.Max.x + off.x, r.Max.y + off.y); }
    inline LinearVec2           WindowPosAbsToRel(LinearUIWindow* window, const LinearVec2& p)  { LinearVec2 off = window->DC.CursorStartPos; return LinearVec2(p.x - off.x, p.y - off.y); }
    inline LinearVec2           WindowPosRelToAbs(LinearUIWindow* window, const LinearVec2& p)  { LinearVec2 off = window->DC.CursorStartPos; return LinearVec2(p.x + off.x, p.y + off.y); }

    // Windows: Display Order and Focus Order
    LINEARUI_API void          FocusWindow(LinearUIWindow* window, LinearUIFocusRequestFlags flags = 0);
    LINEARUI_API void          FocusTopMostWindowUnderOne(LinearUIWindow* under_this_window, LinearUIWindow* ignore_window, LinearUIViewport* filter_viewport, LinearUIFocusRequestFlags flags);
    LINEARUI_API void          BringWindowToFocusFront(LinearUIWindow* window);
    LINEARUI_API void          BringWindowToDisplayFront(LinearUIWindow* window);
    LINEARUI_API void          BringWindowToDisplayBack(LinearUIWindow* window);
    LINEARUI_API void          BringWindowToDisplayBehind(LinearUIWindow* window, LinearUIWindow* above_window);
    LINEARUI_API int           FindWindowDisplayIndex(LinearUIWindow* window);
    LINEARUI_API LinearUIWindow*  FindBottomMostVisibleWindowWithinBeginStack(LinearUIWindow* window);

    // Windows: Idle, Refresh Policies [EXPERIMENTAL]
    LINEARUI_API void          SetNextWindowRefreshPolicy(LinearUIWindowRefreshFlags flags);

    // Fonts, drawing
    LINEARUI_API void          RegisterUserTexture(LinearTextureData* tex); // Register external texture. EXPERIMENTAL.
    LINEARUI_API void          UnregisterUserTexture(LinearTextureData* tex);
    LINEARUI_API void          RegisterFontAtlas(LinearFontAtlas* atlas);
    LINEARUI_API void          UnregisterFontAtlas(LinearFontAtlas* atlas);
    LINEARUI_API void          SetCurrentFont(LinearFont* font, float font_size_before_scaling, float font_size_after_scaling);
    LINEARUI_API void          UpdateCurrentFontSize(float restore_font_size_after_scaling);
    LINEARUI_API void          SetFontRasterizerDensity(float rasterizer_density);
    inline float            GetFontRasterizerDensity() { return GLinearUI->FontRasterizerDensity; }
    inline float            GetRoundedFontSize(float size) { return LINEARUI_ROUND(size); }
    LINEARUI_API LinearFont*       GetDefaultFont();
    LINEARUI_API void          PushPasswordFont();
    LINEARUI_API void          PopPasswordFont();
    inline LinearDrawList*      GetForegroundDrawList(LinearUIWindow* window) { return GetForegroundDrawList(window->Viewport); }
    LINEARUI_API void          AddDrawListToDrawDataEx(LinearDrawData* draw_data, LinearVector<LinearDrawList*>* out_list, LinearDrawList* draw_list);

    // Init
    LINEARUI_API void          Initialize();
    LINEARUI_API void          Shutdown();    // Since 1.60 this is a _private_ function. You can call DestroyContext() to destroy the context created by CreateContext().

    // Context name & generic context hooks
    LINEARUI_API void          SetContextName(LinearUIContext* ctx, const char* name);
    LINEARUI_API LinearUIID       AddContextHook(LinearUIContext* ctx, const LinearUIContextHook* hook);
    LINEARUI_API void          RemoveContextHook(LinearUIContext* ctx, LinearUIID hook_to_remove);
    LINEARUI_API void          CallContextHooks(LinearUIContext* ctx, LinearUIContextHookType type);

    // NewFrame
    LINEARUI_API void          UpdateInputEvents(bool trickle_fast_inputs);
    LINEARUI_API void          UpdateHoveredWindowAndCaptureFlags(const LinearVec2& mouse_pos);
    LINEARUI_API void          FindHoveredWindowEx(const LinearVec2& pos, bool find_first_and_in_any_viewport, LinearUIWindow** out_hovered_window, LinearUIWindow** out_hovered_window_under_moving_window);
    LINEARUI_API void          StartMouseMovingWindow(LinearUIWindow* window);
    LINEARUI_API void          StartMouseMovingWindowOrNode(LinearUIWindow* window, LinearUIDockNode* node, bool undock);
    LINEARUI_API void          StopMouseMovingWindow();
    LINEARUI_API void          UpdateMouseMovingWindowNewFrame();
    LINEARUI_API void          UpdateMouseMovingWindowEndFrame();

    // Viewports
    LINEARUI_API void          TranslateWindowsInViewport(LinearUIViewportP* viewport, const LinearVec2& old_pos, const LinearVec2& new_pos, const LinearVec2& old_size, const LinearVec2& new_size);
    LINEARUI_API void          ScaleWindowsInViewport(LinearUIViewportP* viewport, float scale);
    LINEARUI_API void          DestroyPlatformWindow(LinearUIViewportP* viewport);
    LINEARUI_API void          SetWindowViewport(LinearUIWindow* window, LinearUIViewportP* viewport);
    LINEARUI_API void          SetCurrentViewport(LinearUIWindow* window, LinearUIViewportP* viewport);
    LINEARUI_API const LinearUIPlatformMonitor*   GetViewportPlatformMonitor(LinearUIViewport* viewport);
    LINEARUI_API LinearUIViewportP*               FindHoveredViewportFromPlatformWindowStack(const LinearVec2& mouse_platform_pos);

    // Settings
    LINEARUI_API void                  MarkIniSettingsDirty();
    LINEARUI_API void                  MarkIniSettingsDirty(LinearUIWindow* window);
    LINEARUI_API void                  ClearIniSettings();
    LINEARUI_API void                  AddSettingsHandler(const LinearUISettingsHandler* handler);
    LINEARUI_API void                  RemoveSettingsHandler(const char* type_name);
    LINEARUI_API LinearUISettingsHandler* FindSettingsHandler(const char* type_name);

    // Settings - Windows
    LINEARUI_API LinearUIWindowSettings*  CreateNewWindowSettings(const char* name);
    LINEARUI_API LinearUIWindowSettings*  FindWindowSettingsByID(LinearUIID id);
    LINEARUI_API LinearUIWindowSettings*  FindWindowSettingsByWindow(LinearUIWindow* window);
    LINEARUI_API void                  ClearWindowSettings(const char* name);

    // Localization
    LINEARUI_API void          LocalizeRegisterEntries(const LinearUILocEntry* entries, int count);
    inline const char*      LocalizeGetMsg(LinearUILocKey key) { LinearUIContext& g = *GLinearUI; const char* msg = g.LocalizationTable[key]; return msg ? msg : "*Missing Text*"; }

    // Scrolling
    LINEARUI_API void          SetScrollX(LinearUIWindow* window, float scroll_x);
    LINEARUI_API void          SetScrollY(LinearUIWindow* window, float scroll_y);
    LINEARUI_API void          SetScrollFromPosX(LinearUIWindow* window, float local_x, float center_x_ratio);
    LINEARUI_API void          SetScrollFromPosY(LinearUIWindow* window, float local_y, float center_y_ratio);

    // Early work-in-progress API (ScrollToItem() will become public)
    LINEARUI_API void          ScrollToItem(LinearUIScrollFlags flags = 0);
    LINEARUI_API void          ScrollToRect(LinearUIWindow* window, const LinearRect& rect, LinearUIScrollFlags flags = 0);
    LINEARUI_API LinearVec2        ScrollToRectEx(LinearUIWindow* window, const LinearRect& rect, LinearUIScrollFlags flags = 0);
//#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    inline void             ScrollToBringRectIntoView(LinearUIWindow* window, const LinearRect& rect) { ScrollToRect(window, rect, LinearUIScrollFlags_KeepVisibleEdgeY); }
//#endif

    // Basic Accessors
    inline LinearUIItemStatusFlags GetItemStatusFlags() { LinearUIContext& g = *GLinearUI; return g.LastItemData.StatusFlags; }
    inline LinearUIID          GetActiveID()   { LinearUIContext& g = *GLinearUI; return g.ActiveId; }
    inline LinearUIID          GetFocusID()    { LinearUIContext& g = *GLinearUI; return g.NavId; }
    LINEARUI_API void          SetActiveID(LinearUIID id, LinearUIWindow* window);
    LINEARUI_API void          SetFocusID(LinearUIID id, LinearUIWindow* window);
    LINEARUI_API void          ClearActiveID();
    LINEARUI_API LinearUIID       GetHoveredID();
    LINEARUI_API void          SetHoveredID(LinearUIID id);
    LINEARUI_API void          KeepAliveID(LinearUIID id);
    LINEARUI_API void          MarkItemEdited(LinearUIID id);     // Mark data associated to given item as "edited", used by IsItemDeactivatedAfterEdit() function.
    LINEARUI_API void          PushOverrideID(LinearUIID id);     // Push given value as-is at the top of the ID stack (whereas PushID combines old and new hashes)
    LINEARUI_API LinearUIID       GetIDWithSeed(const char* str_id_begin, const char* str_id_end, LinearUIID seed);
    LINEARUI_API LinearUIID       GetIDWithSeed(int n, LinearUIID seed);

    // Basic Helpers for widget code
    LINEARUI_API void          ItemSize(const LinearVec2& size, float text_baseline_y = -1.0f);
    inline void             ItemSize(const LinearRect& bb, float text_baseline_y = -1.0f) { ItemSize(bb.GetSize(), text_baseline_y); } // FIXME: This is a misleading API since we expect CursorPos to be bb.Min.
    LINEARUI_API bool          ItemAdd(const LinearRect& bb, LinearUIID id, const LinearRect* nav_bb = NULL, LinearUIItemFlags extra_flags = 0);
    LINEARUI_API bool          ItemHoverable(const LinearRect& bb, LinearUIID id, LinearUIItemFlags item_flags);
    LINEARUI_API bool          IsWindowContentHoverable(LinearUIWindow* window, LinearUIHoveredFlags flags = 0);
    LINEARUI_API bool          IsClippedEx(const LinearRect& bb, LinearUIID id);
    LINEARUI_API void          SetLastItemData(LinearUIID item_id, LinearUIItemFlags item_flags, LinearUIItemStatusFlags status_flags, const LinearRect& item_rect);
    LINEARUI_API LinearVec2        CalcItemSize(LinearVec2 size, float default_w, float default_h);
    LINEARUI_API float         CalcWrapWidthForPos(const LinearVec2& pos, float wrap_pos_x);
    LINEARUI_API void          PushMultiItemsWidths(int components, float width_full);
    LINEARUI_API void          ShrinkWidths(LinearUIShrinkWidthItem* items, int count, float width_excess, float width_min);
    LINEARUI_API void          CalcClipRectVisibleItemsY(const LinearRect& clip_rect, const LinearVec2& pos, float items_height, int* out_visible_start, int* out_visible_end);

    // Parameter stacks (shared)
    LINEARUI_API const LinearUIStyleVarInfo* GetStyleVarInfo(LinearUIStyleVar idx);
    LINEARUI_API void          BeginDisabledOverrideReenable();
    LINEARUI_API void          EndDisabledOverrideReenable();

    // Logging/Capture
    LINEARUI_API void          LogBegin(LinearUILogFlags flags, int auto_open_depth);         // -> BeginCapture() when we design v2 api, for now stay under the radar by using the old name.
    LINEARUI_API void          LogToBuffer(int auto_open_depth = -1);                      // Start logging/capturing to internal buffer
    LINEARUI_API void          LogRenderedText(const LinearVec2* ref_pos, const char* text, const char* text_end = NULL);
    LINEARUI_API void          LogSetNextTextDecoration(const char* prefix, const char* suffix);

    // Childs
    LINEARUI_API bool          BeginChildEx(const char* name, LinearUIID id, const LinearVec2& size_arg, LinearUIChildFlags child_flags, LinearUIWindowFlags window_flags);
    LINEARUI_API LinearUIWindow*  FindFrontMostVisibleChildWindow(LinearUIWindow* window);

    // Popups, Modals
    LINEARUI_API bool          BeginPopupEx(LinearUIID id, LinearUIWindowFlags extra_window_flags);
    LINEARUI_API bool          BeginPopupMenuEx(LinearUIID id, const char* label, LinearUIWindowFlags extra_window_flags);
    LINEARUI_API void          OpenPopupEx(LinearUIID id, LinearUIPopupFlags popup_flags = LinearUIPopupFlags_None);
    LINEARUI_API void          ClosePopupToLevel(int remaining, bool restore_focus_to_window_under_popup);
    LINEARUI_API void          ClosePopupsOverWindow(LinearUIWindow* ref_window, bool restore_focus_to_window_under_popup);
    LINEARUI_API void          ClosePopupsExceptModals();
    LINEARUI_API bool          IsPopupOpen(LinearUIID id, LinearUIPopupFlags popup_flags);
    LINEARUI_API LinearRect        GetPopupAllowedExtentRect(LinearUIWindow* window);
    LINEARUI_API LinearUIWindow*  GetTopMostPopupModal();
    LINEARUI_API LinearUIWindow*  GetTopMostAndVisiblePopupModal();
    LINEARUI_API LinearUIWindow*  FindBlockingModal(LinearUIWindow* window);
    LINEARUI_API LinearVec2        FindBestWindowPosForPopup(LinearUIWindow* window);
    LINEARUI_API LinearVec2        FindBestWindowPosForPopupEx(const LinearVec2& ref_pos, const LinearVec2& size, LinearUIDir* last_dir, const LinearRect& r_outer, const LinearRect& r_avoid, LinearUIPopupPositionPolicy policy);
    LINEARUI_API LinearUIMouseButton GetMouseButtonFromPopupFlags(LinearUIPopupFlags flags);
    LINEARUI_API bool          IsPopupOpenRequestForItem(LinearUIPopupFlags flags, LinearUIID id);
    LINEARUI_API bool          IsPopupOpenRequestForWindow(LinearUIPopupFlags flags);

    // Tooltips
    LINEARUI_API bool          BeginTooltipEx(LinearUITooltipFlags tooltip_flags, LinearUIWindowFlags extra_window_flags);
    LINEARUI_API bool          BeginTooltipHidden();

    // Menus
    LINEARUI_API bool          BeginViewportSideBar(const char* name, LinearUIViewport* viewport, LinearUIDir dir, float size, LinearUIWindowFlags window_flags);
    LINEARUI_API bool          BeginMenuEx(const char* label, const char* icon, bool enabled = true);
    LINEARUI_API bool          MenuItemEx(const char* label, const char* icon, const char* shortcut = NULL, bool selected = false, bool enabled = true);

    // Combos
    LINEARUI_API bool          BeginComboPopup(LinearUIID popup_id, const LinearRect& bb, LinearUIComboFlags flags);
    LINEARUI_API bool          BeginComboPreview();
    LINEARUI_API void          EndComboPreview();

    // Keyboard/Gamepad Navigation
    LINEARUI_API void          NavInitWindow(LinearUIWindow* window, bool force_reinit);
    LINEARUI_API void          NavInitRequestApplyResult();
    LINEARUI_API bool          NavMoveRequestButNoResultYet();
    LINEARUI_API void          NavMoveRequestSubmit(LinearUIDir move_dir, LinearUIDir clip_dir, LinearUINavMoveFlags move_flags, LinearUIScrollFlags scroll_flags);
    LINEARUI_API void          NavMoveRequestForward(LinearUIDir move_dir, LinearUIDir clip_dir, LinearUINavMoveFlags move_flags, LinearUIScrollFlags scroll_flags);
    LINEARUI_API void          NavMoveRequestResolveWithLastItem(LinearUINavItemData* result);
    LINEARUI_API void          NavMoveRequestResolveWithPastTreeNode(LinearUINavItemData* result, const LinearUITreeNodeStackData* tree_node_data);
    LINEARUI_API void          NavMoveRequestCancel();
    LINEARUI_API void          NavMoveRequestApplyResult();
    LINEARUI_API void          NavMoveRequestTryWrapping(LinearUIWindow* window, LinearUINavMoveFlags move_flags);
    LINEARUI_API void          NavHighlightActivated(LinearUIID id);
    LINEARUI_API void          NavClearPreferredPosForAxis(LinearUIAxis axis);
    LINEARUI_API void          SetNavCursorVisibleAfterMove();
    LINEARUI_API void          NavUpdateCurrentWindowIsScrollPushableX();
    LINEARUI_API void          SetNavWindow(LinearUIWindow* window);
    LINEARUI_API void          SetNavID(LinearUIID id, LinearUINavLayer nav_layer, LinearUIID focus_scope_id, const LinearRect& rect_rel);
    LINEARUI_API void          SetNavFocusScope(LinearUIID focus_scope_id);

    // Focus/Activation
    // This should be part of a larger set of API: FocusItem(offset = -1), FocusItemByID(id), ActivateItem(offset = -1), ActivateItemByID(id) etc. which are
    // much harder to design and implement than expected. I have a couple of private branches on this matter but it's not simple. For now implementing the easy ones.
    LINEARUI_API void          FocusItem();                    // Focus last item (no selection/activation).
    LINEARUI_API void          ActivateItemByID(LinearUIID id);   // Activate an item by ID (button, checkbox, tree node etc.). Activation is queued and processed on the next frame when the item is encountered again. Was called 'ActivateItem()' before 1.89.7.

    // Inputs
    // FIXME: Eventually we should aim to move e.g. IsActiveIdUsingKey() into IsKeyXXX functions.
    inline bool             IsNamedKey(LinearUIKey key)                    { return key >= LinearUIKey_NamedKey_BEGIN && key < LinearUIKey_NamedKey_END; }
    inline bool             IsNamedKeyOrMod(LinearUIKey key)               { return (key >= LinearUIKey_NamedKey_BEGIN && key < LinearUIKey_NamedKey_END) || key == LinearUIMod_Ctrl || key == LinearUIMod_Shift || key == LinearUIMod_Alt || key == LinearUIMod_Super; }
    inline bool             IsLegacyKey(LinearUIKey key)                   { return key >= LinearUIKey_LegacyNativeKey_BEGIN && key < LinearUIKey_LegacyNativeKey_END; }
    inline bool             IsKeyboardKey(LinearUIKey key)                 { return key >= LinearUIKey_Keyboard_BEGIN && key < LinearUIKey_Keyboard_END; }
    inline bool             IsGamepadKey(LinearUIKey key)                  { return key >= LinearUIKey_Gamepad_BEGIN && key < LinearUIKey_Gamepad_END; }
    inline bool             IsMouseKey(LinearUIKey key)                    { return key >= LinearUIKey_Mouse_BEGIN && key < LinearUIKey_Mouse_END; }
    inline bool             IsAliasKey(LinearUIKey key)                    { return key >= LinearUIKey_Aliases_BEGIN && key < LinearUIKey_Aliases_END; }
    inline bool             IsLRModKey(LinearUIKey key)                    { return key >= LinearUIKey_LeftCtrl && key <= LinearUIKey_RightSuper; }
    LinearUIKeyChord           FixupKeyChord(LinearUIKeyChord key_chord);
    inline LinearUIKey         ConvertSingleModFlagToKey(LinearUIKey key)
    {
        if (key == LinearUIMod_Ctrl) return LinearUIKey_ReservedForModCtrl;
        if (key == LinearUIMod_Shift) return LinearUIKey_ReservedForModShift;
        if (key == LinearUIMod_Alt) return LinearUIKey_ReservedForModAlt;
        if (key == LinearUIMod_Super) return LinearUIKey_ReservedForModSuper;
        return key;
    }

    LINEARUI_API LinearUIKeyData* GetKeyData(LinearUIContext* ctx, LinearUIKey key);
    inline LinearUIKeyData*    GetKeyData(LinearUIKey key)                                    { LinearUIContext& g = *GLinearUI; return GetKeyData(&g, key); }
    LINEARUI_API const char*   GetKeyChordName(LinearUIKeyChord key_chord);
    inline LinearUIKey         MouseButtonToKey(LinearUIMouseButton button)                   { LINEARUI_ASSERT(button >= 0 && button < LinearUIMouseButton_COUNT); return (LinearUIKey)(LinearUIKey_MouseLeft + button); }
    LINEARUI_API bool          IsMouseDragPastThreshold(LinearUIMouseButton button, float lock_threshold = -1.0f);
    LINEARUI_API LinearVec2        GetKeyMagnitude2d(LinearUIKey key_left, LinearUIKey key_right, LinearUIKey key_up, LinearUIKey key_down);
    LINEARUI_API float         GetNavTweakPressedAmount(LinearUIAxis axis);
    LINEARUI_API int           CalcTypematicRepeatAmount(float t0, float t1, float repeat_delay, float repeat_rate);
    LINEARUI_API void          GetTypematicRepeatRate(LinearUIInputFlags flags, float* repeat_delay, float* repeat_rate);
    LINEARUI_API void          TeleportMousePos(const LinearVec2& pos);
    LINEARUI_API void          SetActiveIdUsingAllKeyboardKeys();
    inline bool             IsActiveIdUsingNavDir(LinearUIDir dir)                         { LinearUIContext& g = *GLinearUI; return (g.ActiveIdUsingNavDirMask & (1 << dir)) != 0; }

    // [EXPERIMENTAL] Low-Level: Key/Input Ownership
    // - The idea is that instead of "eating" a given input, we can link to an owner id.
    // - Ownership is most often claimed as a result of reacting to a press/down event (but occasionally may be claimed ahead).
    // - Input queries can then read input by specifying LinearUIKeyOwner_Any (== 0), LinearUIKeyOwner_NoOwner (== -1) or a custom ID.
    // - Legacy input queries (without specifying an owner or _Any or _None) are equivalent to using LinearUIKeyOwner_Any (== 0).
    // - Input ownership is automatically released on the frame after a key is released. Therefore:
    //   - for ownership registration happening as a result of a down/press event, the SetKeyOwner() call may be done once (common case).
    //   - for ownership registration happening ahead of a down/press event, the SetKeyOwner() call needs to be made every frame (happens if e.g. claiming ownership on hover).
    // - SetItemKeyOwner() is a shortcut for common simple case. A custom widget will probably want to call SetKeyOwner() multiple times directly based on its interaction state.
    // - This is marked experimental because not all widgets are fully honoring the Set/Test idioms. We will need to move forward step by step.
    //   Please open a GitHub Issue to submit your usage scenario or if there's a use case you need solved.
    LINEARUI_API LinearUIID       GetKeyOwner(LinearUIKey key);
    LINEARUI_API void          SetKeyOwner(LinearUIKey key, LinearUIID owner_id, LinearUIInputFlags flags = 0);
    LINEARUI_API void          SetKeyOwnersForKeyChord(LinearUIKeyChord key, LinearUIID owner_id, LinearUIInputFlags flags = 0);
    LINEARUI_API bool          SetItemKeyOwner(LinearUIKey key, LinearUIInputFlags flags);
    LINEARUI_API bool          TestKeyOwner(LinearUIKey key, LinearUIID owner_id);               // Test that key is either not owned, either owned by 'owner_id'
    inline LinearUIKeyOwnerData* GetKeyOwnerData(LinearUIContext* ctx, LinearUIKey key)          { if (key & LinearUIMod_Mask_) key = ConvertSingleModFlagToKey(key); LINEARUI_ASSERT(IsNamedKey(key)); return &ctx->KeysOwnerData[key - LinearUIKey_NamedKey_BEGIN]; }

    // [EXPERIMENTAL] High-Level: Input Access functions w/ support for Key/Input Ownership
    // - Important: legacy IsKeyPressed(LinearUIKey, bool repeat=true) _DEFAULTS_ to repeat, new IsKeyPressed() requires _EXPLICIT_ LinearUIInputFlags_Repeat flag.
    // - Expected to be later promoted to public API, the prototypes are designed to replace existing ones (since owner_id can default to Any == 0)
    // - Specifying a value for 'LinearUIID owner' will test that EITHER the key is NOT owned (UNLESS locked), EITHER the key is owned by 'owner'.
    //   Legacy functions use LinearUIKeyOwner_Any meaning that they typically ignore ownership, unless a call to SetKeyOwner() explicitly used LinearUIInputFlags_LockThisFrame or LinearUIInputFlags_LockUntilRelease.
    // - Binding generators may want to ignore those for now, or suffix them with Ex() until we decide if this gets moved into public API.
    LINEARUI_API bool          IsKeyDown(LinearUIKey key, LinearUIID owner_id);
    LINEARUI_API bool          IsKeyPressed(LinearUIKey key, LinearUIInputFlags flags, LinearUIID owner_id = 0);    // Important: when transitioning from old to new IsKeyPressed(): old API has "bool repeat = true", so would default to repeat. New API requires explicit LinearUIInputFlags_Repeat.
    LINEARUI_API bool          IsKeyReleased(LinearUIKey key, LinearUIID owner_id);
    LINEARUI_API bool          IsKeyChordPressed(LinearUIKeyChord key_chord, LinearUIInputFlags flags, LinearUIID owner_id = 0);
    LINEARUI_API bool          IsMouseDown(LinearUIMouseButton button, LinearUIID owner_id);
    LINEARUI_API bool          IsMouseClicked(LinearUIMouseButton button, LinearUIInputFlags flags, LinearUIID owner_id = 0);
    LINEARUI_API bool          IsMouseReleased(LinearUIMouseButton button, LinearUIID owner_id);
    LINEARUI_API bool          IsMouseDoubleClicked(LinearUIMouseButton button, LinearUIID owner_id);

    // Shortcut Testing & Routing
    // - Set Shortcut() and SetNextItemShortcut() in linearui.h
    // - When a policy (except for LinearUIInputFlags_RouteAlways *) is set, Shortcut() will register itself with SetShortcutRouting(),
    //   allowing the system to decide where to route the input among other route-aware calls.
    //   (* using LinearUIInputFlags_RouteAlways is roughly equivalent to calling IsKeyChordPressed(key) and bypassing route registration and check)
    // - When using one of the routing option:
    //   - The default route is LinearUIInputFlags_RouteFocused (accept inputs if window is in focus stack. Deep-most focused window takes inputs. ActiveId takes inputs over deep-most focused window.)
    //   - Routes are requested given a chord (key + modifiers) and a routing policy.
    //   - Routes are resolved during NewFrame(): if keyboard modifiers are matching current ones: SetKeyOwner() is called + route is granted for the frame.
    //   - Each route may be granted to a single owner. When multiple requests are made we have policies to select the winning route (e.g. deep most window).
    //   - Multiple read sites may use the same owner id can all access the granted route.
    //   - When owner_id is 0 we use the current Focus Scope ID as a owner ID in order to identify our location.
    // - You can chain two unrelated windows in the focus stack using SetWindowParentWindowForFocusRoute()
    //   e.g. if you have a tool window associated to a document, and you want document shortcuts to run when the tool is focused.
    LINEARUI_API bool          Shortcut(LinearUIKeyChord key_chord, LinearUIInputFlags flags, LinearUIID owner_id);
    LINEARUI_API bool          SetShortcutRouting(LinearUIKeyChord key_chord, LinearUIInputFlags flags, LinearUIID owner_id); // owner_id needs to be explicit and cannot be 0
    LINEARUI_API bool          TestShortcutRouting(LinearUIKeyChord key_chord, LinearUIID owner_id);
    LINEARUI_API LinearUIKeyRoutingData* GetShortcutRoutingData(LinearUIKeyChord key_chord);

    // Docking
    // (some functions are only declared in linearui.cpp, see Docking section)
    LINEARUI_API void          DockContextInitialize(LinearUIContext* ctx);
    LINEARUI_API void          DockContextShutdown(LinearUIContext* ctx);
    LINEARUI_API void          DockContextClearNodes(LinearUIContext* ctx, LinearUIID root_id, bool clear_settings_refs); // Use root_id==0 to clear all
    LINEARUI_API void          DockContextRebuildNodes(LinearUIContext* ctx);
    LINEARUI_API void          DockContextNewFrameUpdateUndocking(LinearUIContext* ctx);
    LINEARUI_API void          DockContextNewFrameUpdateDocking(LinearUIContext* ctx);
    LINEARUI_API void          DockContextEndFrame(LinearUIContext* ctx);
    LINEARUI_API LinearUIID       DockContextGenNodeID(LinearUIContext* ctx);
    LINEARUI_API void          DockContextQueueDock(LinearUIContext* ctx, LinearUIWindow* target, LinearUIDockNode* target_node, LinearUIWindow* payload, LinearUIDir split_dir, float split_ratio, bool split_outer);
    LINEARUI_API void          DockContextQueueUndockWindow(LinearUIContext* ctx, LinearUIWindow* window);
    LINEARUI_API void          DockContextQueueUndockNode(LinearUIContext* ctx, LinearUIDockNode* node);
    LINEARUI_API void          DockContextProcessUndockWindow(LinearUIContext* ctx, LinearUIWindow* window, bool clear_persistent_docking_ref = true);
    LINEARUI_API void          DockContextProcessUndockNode(LinearUIContext* ctx, LinearUIDockNode* node);
    LINEARUI_API bool          DockContextCalcDropPosForDocking(LinearUIWindow* target, LinearUIDockNode* target_node, LinearUIWindow* payload_window, LinearUIDockNode* payload_node, LinearUIDir split_dir, bool split_outer, LinearVec2* out_pos);
    LINEARUI_API LinearUIDockNode*DockContextFindNodeByID(LinearUIContext* ctx, LinearUIID id);
    LINEARUI_API void          DockNodeWindowMenuHandler_Default(LinearUIContext* ctx, LinearUIDockNode* node, LinearUITabBar* tab_bar);
    LINEARUI_API bool          DockNodeBeginAmendTabBar(LinearUIDockNode* node);
    LINEARUI_API void          DockNodeEndAmendTabBar();
    inline LinearUIDockNode*   DockNodeGetRootNode(LinearUIDockNode* node)                 { while (node->ParentNode) node = node->ParentNode; return node; }
    inline bool             DockNodeIsInHierarchyOf(LinearUIDockNode* node, LinearUIDockNode* parent) { while (node) { if (node == parent) return true; node = node->ParentNode; } return false; }
    inline int              DockNodeGetDepth(const LinearUIDockNode* node)              { int depth = 0; while (node->ParentNode) { node = node->ParentNode; depth++; } return depth; }
    inline LinearUIID          DockNodeGetWindowMenuButtonId(const LinearUIDockNode* node) { return LinearHashStr("#COLLAPSE", 0, node->ID); }
    inline LinearUIDockNode*   GetWindowDockNode()                                      { LinearUIContext& g = *GLinearUI; return g.CurrentWindow->DockNode; }
    LINEARUI_API bool          GetWindowAlwaysWantOwnTabBar(LinearUIWindow* window);
    LINEARUI_API void          BeginDocked(LinearUIWindow* window, bool* p_open);
    LINEARUI_API void          BeginDockableDragDropSource(LinearUIWindow* window);
    LINEARUI_API void          BeginDockableDragDropTarget(LinearUIWindow* window);
    LINEARUI_API void          SetWindowDock(LinearUIWindow* window, LinearUIID dock_id, LinearUICond cond);

    // Docking - Builder function needs to be generally called before the node is used/submitted.
    // - The DockBuilderXXX functions are designed to _eventually_ become a public API, but it is too early to expose it and guarantee stability.
    // - Do not hold on LinearUIDockNode* pointers! They may be invalidated by any split/merge/remove operation and every frame.
    // - To create a DockSpace() node, make sure to set the LinearUIDockNodeFlags_DockSpace flag when calling DockBuilderAddNode().
    //   You can create dockspace nodes (attached to a window) _or_ floating nodes (carry its own window) with this API.
    // - DockBuilderSplitNode() create 2 child nodes within 1 node. The initial node becomes a parent node.
    // - If you intend to split the node immediately after creation using DockBuilderSplitNode(), make sure
    //   to call DockBuilderSetNodeSize() beforehand. If you don't, the resulting split sizes may not be reliable.
    // - Call DockBuilderFinish() after you are done.
    LINEARUI_API void          DockBuilderDockWindow(const char* window_name, LinearUIID node_id);
    LINEARUI_API LinearUIDockNode*DockBuilderGetNode(LinearUIID node_id);
    inline LinearUIDockNode*   DockBuilderGetCentralNode(LinearUIID node_id)              { LinearUIDockNode* node = DockBuilderGetNode(node_id); if (!node) return NULL; return DockNodeGetRootNode(node)->CentralNode; }
    LINEARUI_API LinearUIID       DockBuilderAddNode(LinearUIID node_id = 0, LinearUIDockNodeFlags flags = 0);
    LINEARUI_API void          DockBuilderRemoveNode(LinearUIID node_id);                 // Remove node and all its child, undock all windows
    LINEARUI_API void          DockBuilderRemoveNodeDockedWindows(LinearUIID node_id, bool clear_settings_refs = true);
    LINEARUI_API void          DockBuilderRemoveNodeChildNodes(LinearUIID node_id);       // Remove all split/hierarchy. All remaining docked windows will be re-docked to the remaining root node (node_id).
    LINEARUI_API void          DockBuilderSetNodePos(LinearUIID node_id, LinearVec2 pos);
    LINEARUI_API void          DockBuilderSetNodeSize(LinearUIID node_id, LinearVec2 size);
    LINEARUI_API LinearUIID       DockBuilderSplitNode(LinearUIID node_id, LinearUIDir split_dir, float size_ratio_for_node_at_dir, LinearUIID* out_id_at_dir, LinearUIID* out_id_at_opposite_dir); // Create 2 child nodes in this parent node.
    LINEARUI_API void          DockBuilderCopyDockSpace(LinearUIID src_dockspace_id, LinearUIID dst_dockspace_id, LinearVector<const char*>* in_window_remap_pairs);
    LINEARUI_API void          DockBuilderCopyNode(LinearUIID src_node_id, LinearUIID dst_node_id, LinearVector<LinearUIID>* out_node_remap_pairs);
    LINEARUI_API void          DockBuilderCopyWindowSettings(const char* src_name, const char* dst_name);
    LINEARUI_API void          DockBuilderFinish(LinearUIID node_id);

    // [EXPERIMENTAL] Focus Scope
    // This is generally used to identify a unique input location (for e.g. a selection set)
    // There is one per window (automatically set in Begin), but:
    // - Selection patterns generally need to react (e.g. clear a selection) when landing on one item of the set.
    //   So in order to identify a set multiple lists in same window may each need a focus scope.
    //   If you imagine an hypothetical BeginSelectionGroup()/EndSelectionGroup() api, it would likely call PushFocusScope()/EndFocusScope()
    // - Shortcut routing also use focus scope as a default location identifier if an owner is not provided.
    // We don't use the ID Stack for this as it is common to want them separate.
    LINEARUI_API void          PushFocusScope(LinearUIID id);
    LINEARUI_API void          PopFocusScope();
    LINEARUI_API bool          IsInNavFocusRoute(LinearUIID focus_scope_id);
    inline LinearUIID          GetCurrentFocusScope() { LinearUIContext& g = *GLinearUI; return g.CurrentFocusScopeId; }   // Focus scope we are outputting into, set by PushFocusScope()

    // Drag and Drop
    LINEARUI_API bool          IsDragDropActive();
    LINEARUI_API bool          BeginDragDropTargetCustom(const LinearRect& bb, LinearUIID id);
    LINEARUI_API bool          BeginDragDropTargetViewport(LinearUIViewport* viewport, const LinearRect* p_bb = NULL);
    LINEARUI_API void          ClearDragDrop();
    LINEARUI_API bool          IsDragDropPayloadBeingAccepted();
    LINEARUI_API void          RenderDragDropTargetRectForItem(const LinearRect& bb);
    LINEARUI_API void          RenderDragDropTargetRectEx(LinearDrawList* draw_list, const LinearRect& bb, float rounding);

    // Typing-Select API
    // (provide Windows Explorer style "select items by typing partial name" + "cycle through items by typing same letter" feature)
    // (this is currently not documented nor used by main library, but should work. See "widgets_typingselect" in linearui_test_suite for usage code. Please let us know if you use this!)
    LINEARUI_API LinearUITypingSelectRequest* GetTypingSelectRequest(LinearUITypingSelectFlags flags = LinearUITypingSelectFlags_None);
    LINEARUI_API int           TypingSelectFindMatch(LinearUITypingSelectRequest* req, int items_count, const char* (*get_item_name_func)(void*, int), void* user_data, int nav_item_idx);
    LINEARUI_API int           TypingSelectFindNextSingleCharMatch(LinearUITypingSelectRequest* req, int items_count, const char* (*get_item_name_func)(void*, int), void* user_data, int nav_item_idx);
    LINEARUI_API int           TypingSelectFindBestLeadingMatch(LinearUITypingSelectRequest* req, int items_count, const char* (*get_item_name_func)(void*, int), void* user_data);

    // Box-Select API
    LINEARUI_API bool          BeginBoxSelect(const LinearRect& scope_rect, LinearUIWindow* window, LinearUIID box_select_id, LinearUIMultiSelectFlags ms_flags);
    LINEARUI_API void          EndBoxSelect(const LinearRect& scope_rect, LinearUIMultiSelectFlags ms_flags);

    // Multi-Select API
    LINEARUI_API void          MultiSelectItemHeader(LinearUIID id, bool* p_selected, LinearUIButtonFlags* p_button_flags);
    LINEARUI_API void          MultiSelectItemFooter(LinearUIID id, bool* p_selected, bool* p_pressed);
    LINEARUI_API void          MultiSelectAddSetAll(LinearUIMultiSelectTempData* ms, bool selected);
    LINEARUI_API void          MultiSelectAddSetRange(LinearUIMultiSelectTempData* ms, bool selected, int range_dir, LinearUISelectionUserData first_item, LinearUISelectionUserData last_item);
    inline LinearUIBoxSelectState*     GetBoxSelectState(LinearUIID id)   { LinearUIContext& g = *GLinearUI; return (id != 0 && g.BoxSelectState.ID == id && g.BoxSelectState.IsActive) ? &g.BoxSelectState : NULL; }
    inline LinearUIMultiSelectState*   GetMultiSelectState(LinearUIID id) { LinearUIContext& g = *GLinearUI; return g.MultiSelectStorage.GetByKey(id); }

    // Internal Columns API (this is not exposed because we will encourage transitioning to the Tables API)
    LINEARUI_API void          SetWindowClipRectBeforeSetChannel(LinearUIWindow* window, const LinearRect& clip_rect);
    LINEARUI_API void          BeginColumns(const char* str_id, int count, LinearUIOldColumnFlags flags = 0); // setup number of columns. use an identifier to distinguish multiple column sets. close with EndColumns().
    LINEARUI_API void          EndColumns();                                                               // close columns
    LINEARUI_API void          PushColumnClipRect(int column_index);
    LINEARUI_API void          PushColumnsBackground();
    LINEARUI_API void          PopColumnsBackground();
    LINEARUI_API LinearUIID       GetColumnsID(const char* str_id, int count);
    LINEARUI_API LinearUIOldColumns* FindOrCreateColumns(LinearUIWindow* window, LinearUIID id);
    LINEARUI_API float         GetColumnOffsetFromNorm(const LinearUIOldColumns* columns, float offset_norm);
    LINEARUI_API float         GetColumnNormFromOffset(const LinearUIOldColumns* columns, float offset);

    // Tables: Candidates for public API
    LINEARUI_API void          TableOpenContextMenu(int column_n = -1);
    LINEARUI_API void          TableSetColumnWidth(int column_n, float width);
    LINEARUI_API void          TableSetColumnSortDirection(int column_n, LinearUISortDirection sort_direction, bool append_to_sort_specs);
    LINEARUI_API int           TableGetHoveredRow();       // Retrieve *PREVIOUS FRAME* hovered row. This difference with TableGetHoveredColumn() is the reason why this is not public yet.
    LINEARUI_API float         TableGetHeaderRowHeight();
    LINEARUI_API float         TableGetHeaderAngledMaxLabelWidth();
    LINEARUI_API void          TablePushBackgroundChannel();
    LINEARUI_API void          TablePopBackgroundChannel();
    LINEARUI_API void          TablePushColumnChannel(int column_n);
    LINEARUI_API void          TablePopColumnChannel();
    LINEARUI_API void          TableAngledHeadersRowEx(LinearUIID row_id, float angle, float max_label_width, const LinearUITableHeaderData* data, int data_count);

    // Tables: Internals
    inline    LinearUITable*   GetCurrentTable() { LinearUIContext& g = *GLinearUI; return g.CurrentTable; }
    LINEARUI_API LinearUITable*   TableFindByID(LinearUIID id);
    LINEARUI_API bool          BeginTableEx(const char* name, LinearUIID id, int columns_count, LinearUITableFlags flags = 0, const LinearVec2& outer_size = LinearVec2(0, 0), float inner_width = 0.0f);
    LINEARUI_API void          TableBeginInitMemory(LinearUITable* table, int columns_count);
    LINEARUI_API void          TableBeginApplyRequests(LinearUITable* table);
    LINEARUI_API void          TableSetupDrawChannels(LinearUITable* table);
    LINEARUI_API void          TableUpdateLayout(LinearUITable* table);
    LINEARUI_API void          TableUpdateBorders(LinearUITable* table);
    LINEARUI_API void          TableUpdateColumnsWeightFromWidth(LinearUITable* table);
    LINEARUI_API void          TableApplyExternalUnclipRect(LinearUITable* table, LinearRect& rect);
    LINEARUI_API void          TableDrawBorders(LinearUITable* table);
    LINEARUI_API void          TableDrawDefaultContextMenu(LinearUITable* table, LinearUITableFlags flags_for_section_to_display);
    LINEARUI_API bool          TableBeginContextMenuPopup(LinearUITable* table);
    LINEARUI_API void          TableMergeDrawChannels(LinearUITable* table);
    inline LinearUITableInstanceData*  TableGetInstanceData(LinearUITable* table, int instance_no) { if (instance_no == 0) return &table->InstanceDataFirst; return &table->InstanceDataExtra[instance_no - 1]; }
    inline LinearUIID                  TableGetInstanceID(LinearUITable* table, int instance_no)   { return TableGetInstanceData(table, instance_no)->TableInstanceID; }
    LINEARUI_API void          TableFixDisplayOrder(LinearUITable* table);
    LINEARUI_API void          TableSortSpecsSanitize(LinearUITable* table);
    LINEARUI_API void          TableSortSpecsBuild(LinearUITable* table);
    LINEARUI_API LinearUISortDirection TableGetColumnNextSortDirection(LinearUITableColumn* column);
    LINEARUI_API void          TableFixColumnSortDirection(LinearUITable* table, LinearUITableColumn* column);
    LINEARUI_API float         TableGetColumnWidthAuto(LinearUITable* table, LinearUITableColumn* column);
    LINEARUI_API void          TableBeginRow(LinearUITable* table);
    LINEARUI_API void          TableEndRow(LinearUITable* table);
    LINEARUI_API void          TableBeginCell(LinearUITable* table, int column_n);
    LINEARUI_API void          TableEndCell(LinearUITable* table);
    LINEARUI_API LinearRect        TableGetCellBgRect(const LinearUITable* table, int column_n);
    LINEARUI_API const char*   TableGetColumnName(const LinearUITable* table, int column_n);
    LINEARUI_API LinearUIID       TableGetColumnResizeID(LinearUITable* table, int column_n, int instance_no = 0);
    LINEARUI_API float         TableCalcMaxColumnWidth(const LinearUITable* table, int column_n);
    LINEARUI_API void          TableSetColumnWidthAutoSingle(LinearUITable* table, int column_n);
    LINEARUI_API void          TableSetColumnWidthAutoAll(LinearUITable* table);
    LINEARUI_API void          TableSetColumnDisplayOrder(LinearUITable* table, int column_n, int dst_order);
    LINEARUI_API void          TableQueueSetColumnDisplayOrder(LinearUITable* table, int column_n, int dst_order);
    LINEARUI_API void          TableRemove(LinearUITable* table);
    LINEARUI_API void          TableGcCompactTransientBuffers(LinearUITable* table);
    LINEARUI_API void          TableGcCompactTransientBuffers(LinearUITableTempData* table);
    LINEARUI_API void          TableGcCompactSettings();

    // Tables: Settings
    LINEARUI_API void                  TableLoadSettings(LinearUITable* table);
    LINEARUI_API void                  TableSaveSettings(LinearUITable* table);
    LINEARUI_API void                  TableResetSettings(LinearUITable* table);
    LINEARUI_API LinearUITableSettings*   TableGetBoundSettings(LinearUITable* table);
    LINEARUI_API void                  TableSettingsAddSettingsHandler();
    LINEARUI_API LinearUITableSettings*   TableSettingsCreate(LinearUIID id, int columns_count);
    LINEARUI_API LinearUITableSettings*   TableSettingsFindByID(LinearUIID id);

    // Tab Bars
    inline    LinearUITabBar*  GetCurrentTabBar() { LinearUIContext& g = *GLinearUI; return g.CurrentTabBar; }
    LINEARUI_API LinearUITabBar*  TabBarFindByID(LinearUIID id);
    LINEARUI_API void          TabBarRemove(LinearUITabBar* tab_bar);
    LINEARUI_API bool          BeginTabBarEx(LinearUITabBar* tab_bar, const LinearRect& bb, LinearUITabBarFlags flags);
    LINEARUI_API LinearUITabItem* TabBarFindTabByID(LinearUITabBar* tab_bar, LinearUIID tab_id);
    LINEARUI_API LinearUITabItem* TabBarFindTabByOrder(LinearUITabBar* tab_bar, int order);
    LINEARUI_API LinearUITabItem* TabBarFindMostRecentlySelectedTabForActiveWindow(LinearUITabBar* tab_bar);
    LINEARUI_API LinearUITabItem* TabBarGetCurrentTab(LinearUITabBar* tab_bar);
    inline int              TabBarGetTabOrder(LinearUITabBar* tab_bar, LinearUITabItem* tab) { return tab_bar->Tabs.index_from_ptr(tab); }
    LINEARUI_API const char*   TabBarGetTabName(LinearUITabBar* tab_bar, LinearUITabItem* tab);
    LINEARUI_API void          TabBarAddTab(LinearUITabBar* tab_bar, LinearUITabItemFlags tab_flags, LinearUIWindow* window);
    LINEARUI_API void          TabBarRemoveTab(LinearUITabBar* tab_bar, LinearUIID tab_id);
    LINEARUI_API void          TabBarCloseTab(LinearUITabBar* tab_bar, LinearUITabItem* tab);
    LINEARUI_API void          TabBarQueueFocus(LinearUITabBar* tab_bar, LinearUITabItem* tab);
    LINEARUI_API void          TabBarQueueFocus(LinearUITabBar* tab_bar, const char* tab_name);
    LINEARUI_API void          TabBarQueueReorder(LinearUITabBar* tab_bar, LinearUITabItem* tab, int offset);
    LINEARUI_API void          TabBarQueueReorderFromMousePos(LinearUITabBar* tab_bar, LinearUITabItem* tab, LinearVec2 mouse_pos);
    LINEARUI_API bool          TabBarProcessReorder(LinearUITabBar* tab_bar);
    LINEARUI_API bool          TabItemEx(LinearUITabBar* tab_bar, const char* label, bool* p_open, LinearUITabItemFlags flags, LinearUIWindow* docked_window);
    LINEARUI_API void          TabItemSpacing(const char* str_id, LinearUITabItemFlags flags, float width);
    LINEARUI_API LinearVec2        TabItemCalcSize(const char* label, bool has_close_button_or_unsaved_marker);
    LINEARUI_API LinearVec2        TabItemCalcSize(LinearUIWindow* window);
    LINEARUI_API void          TabItemBackground(LinearDrawList* draw_list, const LinearRect& bb, LinearUITabItemFlags flags, LinearU32 col);
    LINEARUI_API void          TabItemLabelAndCloseButton(LinearDrawList* draw_list, const LinearRect& bb, LinearUITabItemFlags flags, LinearVec2 frame_padding, const char* label, LinearUIID tab_id, LinearUIID close_button_id, bool is_contents_visible, bool* out_just_closed, bool* out_text_clipped);

    // Render helpers
    // AVOID USING OUTSIDE OF LINEARUI.CPP! NOT FOR PUBLIC CONSUMPTION. THOSE FUNCTIONS ARE A MESS. THEIR SIGNATURE AND BEHAVIOR WILL CHANGE, THEY NEED TO BE REFACTORED INTO SOMETHING DECENT.
    // NB: All position are in absolute pixels coordinates (we are never using window coordinates internally)
    LINEARUI_API void          RenderText(LinearVec2 pos, const char* text, const char* text_end = NULL, bool hide_text_after_hash = true);
    LINEARUI_API void          RenderTextWrapped(LinearVec2 pos, const char* text, const char* text_end, float wrap_width);
    LINEARUI_API void          RenderTextClipped(const LinearVec2& pos_min, const LinearVec2& pos_max, const char* text, const char* text_end, const LinearVec2* text_size_if_known, const LinearVec2& align = LinearVec2(0, 0), const LinearRect* clip_rect = NULL);
    LINEARUI_API void          RenderTextClippedEx(LinearDrawList* draw_list, const LinearVec2& pos_min, const LinearVec2& pos_max, const char* text, const char* text_end, const LinearVec2* text_size_if_known, const LinearVec2& align = LinearVec2(0, 0), const LinearRect* clip_rect = NULL);
    LINEARUI_API void          RenderTextEllipsis(LinearDrawList* draw_list, const LinearVec2& pos_min, const LinearVec2& pos_max, float ellipsis_max_x, const char* text, const char* text_end, const LinearVec2* text_size_if_known);
    LINEARUI_API void          RenderFrame(LinearVec2 p_min, LinearVec2 p_max, LinearU32 fill_col, bool borders = true, float rounding = 0.0f);
    LINEARUI_API void          RenderFrameBorder(LinearVec2 p_min, LinearVec2 p_max, float rounding = 0.0f);
    LINEARUI_API void          RenderColorComponentMarker(const LinearRect& bb, LinearU32 col, float rounding);
    LINEARUI_API void          RenderColorRectWithAlphaCheckerboard(LinearDrawList* draw_list, LinearVec2 p_min, LinearVec2 p_max, LinearU32 fill_col, float grid_step, LinearVec2 grid_off, float rounding = 0.0f, LinearDrawFlags flags = 0);
    LINEARUI_API void          RenderNavCursor(const LinearRect& bb, LinearUIID id, LinearUINavRenderCursorFlags flags = LinearUINavRenderCursorFlags_None); // Navigation highlight
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    inline    void          RenderNavHighlight(const LinearRect& bb, LinearUIID id, LinearUINavRenderCursorFlags flags = LinearUINavRenderCursorFlags_None) { RenderNavCursor(bb, id, flags); } // Renamed in 1.91.4
#endif
    LINEARUI_API const char*   FindRenderedTextEnd(const char* text, const char* text_end = NULL); // Find the optional ## from which we stop displaying text.
    LINEARUI_API void          RenderMouseCursor(LinearVec2 pos, float scale, LinearUIMouseCursor mouse_cursor, LinearU32 col_fill, LinearU32 col_border, LinearU32 col_shadow);

    // Render helpers (those functions don't access any LinearUI state!)
    LINEARUI_API void          RenderArrow(LinearDrawList* draw_list, LinearVec2 pos, LinearU32 col, LinearUIDir dir, float scale = 1.0f);
    LINEARUI_API void          RenderBullet(LinearDrawList* draw_list, LinearVec2 pos, LinearU32 col);
    LINEARUI_API void          RenderCheckMark(LinearDrawList* draw_list, LinearVec2 pos, LinearU32 col, float sz);
    LINEARUI_API void          RenderArrowPointingAt(LinearDrawList* draw_list, LinearVec2 pos, LinearVec2 half_sz, LinearUIDir direction, LinearU32 col);
    LINEARUI_API void          RenderArrowDockMenu(LinearDrawList* draw_list, LinearVec2 p_min, float sz, LinearU32 col);
    LINEARUI_API void          RenderRectFilledInRangeH(LinearDrawList* draw_list, const LinearRect& rect, LinearU32 col, float fill_x0, float fill_x1, float rounding);
    LINEARUI_API void          RenderRectFilledWithHole(LinearDrawList* draw_list, const LinearRect& outer, const LinearRect& inner, LinearU32 col, float rounding);
    LINEARUI_API LinearDrawFlags   CalcRoundingFlagsForRectInRect(const LinearRect& r_in, const LinearRect& r_outer, float threshold);

    // Widgets: Text
    LINEARUI_API void          TextEx(const char* text, const char* text_end = NULL, LinearUITextFlags flags = 0);
    LINEARUI_API void          TextAligned(float align_x, float size_x, const char* fmt, ...);               // FIXME-WIP: Works but API is likely to be reworked. This is designed for 1 item on the line. (#7024)
    LINEARUI_API void          TextAlignedV(float align_x, float size_x, const char* fmt, va_list args);

    // Widgets
    LINEARUI_API bool          ButtonEx(const char* label, const LinearVec2& size_arg = LinearVec2(0, 0), LinearUIButtonFlags flags = 0);
    LINEARUI_API bool          ArrowButtonEx(const char* str_id, LinearUIDir dir, LinearVec2 size_arg, LinearUIButtonFlags flags = 0);
    LINEARUI_API bool          ImageButtonEx(LinearUIID id, LinearTextureRef tex_ref, const LinearVec2& image_size, const LinearVec2& uv0, const LinearVec2& uv1, const LinearVec4& bg_col, const LinearVec4& tint_col, LinearUIButtonFlags flags = 0);
    LINEARUI_API void          SeparatorEx(LinearUISeparatorFlags flags, float thickness = 1.0f);
    LINEARUI_API void          SeparatorTextEx(LinearUIID id, const char* label, const char* label_end, float extra_width);
    LINEARUI_API bool          CheckboxFlags(const char* label, LinearS64* flags, LinearS64 flags_value);
    LINEARUI_API bool          CheckboxFlags(const char* label, LinearU64* flags, LinearU64 flags_value);

    // Widgets: Window Decorations
    LINEARUI_API bool          CloseButton(LinearUIID id, const LinearVec2& pos);
    LINEARUI_API bool          CollapseButton(LinearUIID id, const LinearVec2& pos, LinearUIDockNode* dock_node);
    LINEARUI_API void          Scrollbar(LinearUIAxis axis);
    LINEARUI_API bool          ScrollbarEx(const LinearRect& bb, LinearUIID id, LinearUIAxis axis, LinearS64* p_scroll_v, LinearS64 avail_v, LinearS64 contents_v, LinearDrawFlags draw_rounding_flags = 0);
    LINEARUI_API LinearRect        GetWindowScrollbarRect(LinearUIWindow* window, LinearUIAxis axis);
    LINEARUI_API LinearUIID       GetWindowScrollbarID(LinearUIWindow* window, LinearUIAxis axis);
    LINEARUI_API LinearUIID       GetWindowResizeCornerID(LinearUIWindow* window, int n); // 0..3: corners
    LINEARUI_API LinearUIID       GetWindowResizeBorderID(LinearUIWindow* window, LinearUIDir dir);
    LINEARUI_API void          ExtendHitBoxWhenNearViewportEdge(LinearUIWindow* window, LinearRect* bb, float threshold, LinearUIAxis axis);

    // Widgets low-level behaviors
    LINEARUI_API bool          ButtonBehavior(const LinearRect& bb, LinearUIID id, bool* out_hovered, bool* out_held, LinearUIButtonFlags flags = 0);
    LINEARUI_API bool          DragBehavior(LinearUIID id, LinearUIDataType data_type, void* p_v, float v_speed, const void* p_min, const void* p_max, const char* format, LinearUISliderFlags flags);
    LINEARUI_API bool          SliderBehavior(const LinearRect& bb, LinearUIID id, LinearUIDataType data_type, void* p_v, const void* p_min, const void* p_max, const char* format, LinearUISliderFlags flags, LinearRect* out_grab_bb);
    LINEARUI_API bool          SplitterBehavior(const LinearRect& bb, LinearUIID id, LinearUIAxis axis, float* size1, float* size2, float min_size1, float min_size2, float hover_extend = 0.0f, float hover_visibility_delay = 0.0f, LinearU32 bg_col = 0);

    // Widgets: Tree Nodes
    LINEARUI_API bool          TreeNodeBehavior(LinearUIID id, LinearUITreeNodeFlags flags, const char* label, const char* label_end = NULL);
    LINEARUI_API void          TreeNodeDrawLineToChildNode(const LinearVec2& target_pos);
    LINEARUI_API void          TreeNodeDrawLineToTreePop(const LinearUITreeNodeStackData* data);
    LINEARUI_API void          TreePushOverrideID(LinearUIID id);
    LINEARUI_API void          TreeNodeSetOpen(LinearUIID storage_id, bool open);
    LINEARUI_API bool          TreeNodeUpdateNextOpen(LinearUIID storage_id, LinearUITreeNodeFlags flags);   // Return open state. Consume previous SetNextItemOpen() data, if any. May return true when logging.

    // Template functions are instantiated in linearui_widgets.cpp for a finite number of types.
    // To use them externally (for custom widget) you may need an "extern template" statement in your code in order to link to existing instances and silence Clang warnings (see #2036).
    // e.g. " extern template LINEARUI_API float RoundScalarWithFormatT<float, float>(const char* format, LinearUIDataType data_type, float v); "
    template<typename T, typename SIGNED_T, typename FLOAT_T>   LINEARUI_API float ScaleRatioFromValueT(LinearUIDataType data_type, T v, T v_min, T v_max, float logarithmic_zero_epsilon, float zero_deadzone_size);
    template<typename T, typename SIGNED_T, typename FLOAT_T>   LINEARUI_API T     ScaleValueFromRatioT(LinearUIDataType data_type, float t, T v_min, T v_max, float logarithmic_zero_epsilon, float zero_deadzone_size);
    template<typename T, typename SIGNED_T, typename FLOAT_T>   LINEARUI_API bool  DragBehaviorT(LinearUIDataType data_type, T* v, float v_speed, T v_min, T v_max, const char* format, LinearUISliderFlags flags);
    template<typename T, typename SIGNED_T, typename FLOAT_T>   LINEARUI_API bool  SliderBehaviorT(const LinearRect& bb, LinearUIID id, LinearUIDataType data_type, T* v, T v_min, T v_max, const char* format, LinearUISliderFlags flags, LinearRect* out_grab_bb);
    template<typename T>                                        LINEARUI_API T     RoundScalarWithFormatT(const char* format, LinearUIDataType data_type, T v);
    template<typename T>                                        LINEARUI_API bool  CheckboxFlagsT(const char* label, T* flags, T flags_value);

    // Data type helpers
    LINEARUI_API const LinearUIDataTypeInfo*  DataTypeGetInfo(LinearUIDataType data_type);
    LINEARUI_API int           DataTypeFormatString(char* buf, int buf_size, LinearUIDataType data_type, const void* p_data, const char* format);
    LINEARUI_API void          DataTypeApplyOp(LinearUIDataType data_type, int op, void* output, const void* arg_1, const void* arg_2);
    LINEARUI_API bool          DataTypeApplyFromText(const char* buf, LinearUIDataType data_type, void* p_data, const char* format, void* p_data_when_empty = NULL);
    LINEARUI_API int           DataTypeCompare(LinearUIDataType data_type, const void* arg_1, const void* arg_2);
    LINEARUI_API bool          DataTypeClamp(LinearUIDataType data_type, void* p_data, const void* p_min, const void* p_max);
    LINEARUI_API bool          DataTypeIsZero(LinearUIDataType data_type, const void* p_data);

    // InputText
    LINEARUI_API bool          InputTextEx(const char* label, const char* hint, char* buf, int buf_size, const LinearVec2& size_arg, LinearUIInputTextFlags flags, LinearUIInputTextCallback callback = NULL, void* user_data = NULL);
    LINEARUI_API void          InputTextDeactivateHook(LinearUIID id);
    LINEARUI_API bool          TempInputText(const LinearRect& bb, LinearUIID id, const char* label, char* buf, size_t buf_size, LinearUIInputTextFlags flags = 0, LinearUIInputTextCallback callback = NULL, void* user_data = NULL);
    LINEARUI_API bool          TempInputScalar(const LinearRect& bb, LinearUIID id, const char* label, LinearUIDataType data_type, void* p_data, const char* format, const void* p_clamp_min = NULL, const void* p_clamp_max = NULL);
    inline bool             TempInputIsActive(LinearUIID id)       { LinearUIContext& g = *GLinearUI; return g.ActiveId == id && g.TempInputId == id; }
    inline LinearUIInputTextState* GetInputTextState(LinearUIID id)   { LinearUIContext& g = *GLinearUI; return (id != 0 && g.InputTextState.ID == id) ? &g.InputTextState : NULL; } // Get input text state if active
    LINEARUI_API void          SetNextItemRefVal(LinearUIDataType data_type, void* p_data);
    inline bool             IsItemActiveAsInputText() { LinearUIContext& g = *GLinearUI; return g.ActiveId != 0 && g.ActiveId == g.LastItemData.ID && g.InputTextState.ID == g.LastItemData.ID; } // This may be useful to apply workaround that a based on distinguish whenever an item is active as a text input field.

    // Color
    LINEARUI_API void          ColorTooltip(const char* text, const float* col, LinearUIColorEditFlags flags);
    LINEARUI_API void          ColorEditOptionsPopup(const float* col, LinearUIColorEditFlags flags);
    LINEARUI_API void          ColorPickerOptionsPopup(const float* ref_col, LinearUIColorEditFlags flags);
    inline void             SetNextItemColorMarker(LinearU32 col) { LinearUIContext& g = *GLinearUI; g.NextItemData.HasFlags |= LinearUINextItemDataFlags_HasColorMarker; g.NextItemData.ColorMarker = col; }

    // Plot
    LINEARUI_API int           PlotEx(LinearUIPlotType plot_type, const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, const LinearVec2& size_arg);

    // Shade functions (write over already created vertices)
    LINEARUI_API void          ShadeVertsLinearColorGradientKeepAlpha(LinearDrawList* draw_list, int vert_start_idx, int vert_end_idx, LinearVec2 gradient_p0, LinearVec2 gradient_p1, LinearU32 col0, LinearU32 col1);
    LINEARUI_API void          ShadeVertsLinearUV(LinearDrawList* draw_list, int vert_start_idx, int vert_end_idx, const LinearVec2& a, const LinearVec2& b, const LinearVec2& uv_a, const LinearVec2& uv_b, bool clamp);
    LINEARUI_API void          ShadeVertsTransformPos(LinearDrawList* draw_list, int vert_start_idx, int vert_end_idx, const LinearVec2& pivot_in, float cos_a, float sin_a, const LinearVec2& pivot_out);

    // Garbage collection
    LINEARUI_API void          GcCompactTransientMiscBuffers();
    LINEARUI_API void          GcCompactTransientWindowBuffers(LinearUIWindow* window);
    LINEARUI_API void          GcAwakeTransientWindowBuffers(LinearUIWindow* window);

    // Error handling, State Recovery
    LINEARUI_API bool          ErrorLog(const char* msg);
    LINEARUI_API void          ErrorRecoveryStoreState(LinearUIErrorRecoveryState* state_out);
    LINEARUI_API void          ErrorRecoveryTryToRecoverState(const LinearUIErrorRecoveryState* state_in);
    LINEARUI_API void          ErrorRecoveryTryToRecoverWindowState(const LinearUIErrorRecoveryState* state_in);
    LINEARUI_API void          ErrorCheckUsingSetCursorPosToExtendParentBoundaries();
    LINEARUI_API void          ErrorCheckEndFrameFinalizeErrorTooltip();
    LINEARUI_API bool          BeginErrorTooltip();
    LINEARUI_API void          EndErrorTooltip();

    // Demo Doc Marker for e.g. linearui_explorer
    LINEARUI_API void          DemoMarker(const char* file, int line, const char* section);

    // Debug Tools
    LINEARUI_API void          DebugAllocHook(LinearUIDebugAllocInfo* info, int frame_count, void* ptr, size_t size); // size >= 0 : alloc, size = -1 : free
    LINEARUI_API void          DebugDrawCursorPos(LinearU32 col = LINEARUI_COL32(255, 0, 0, 255));
    LINEARUI_API void          DebugDrawLineExtents(LinearU32 col = LINEARUI_COL32(255, 0, 0, 255));
    LINEARUI_API void          DebugDrawItemRect(LinearU32 col = LINEARUI_COL32(255, 0, 0, 255));
    LINEARUI_API void          DebugTextUnformattedWithLocateItem(const char* line_begin, const char* line_end);
    LINEARUI_API void          DebugLocateItem(LinearUIID target_id);                     // Call sparingly: only 1 at the same time!
    LINEARUI_API void          DebugLocateItemOnHover(LinearUIID target_id);              // Only call on reaction to a mouse Hover: because only 1 at the same time!
    LINEARUI_API void          DebugLocateItemResolveWithLastItem();
    LINEARUI_API void          DebugBreakClearData();
    LINEARUI_API bool          DebugBreakButton(const char* label, const char* description_of_location);
    LINEARUI_API void          DebugBreakButtonTooltip(bool keyboard_only, const char* description_of_location);
    LINEARUI_API void          ShowFontAtlas(LinearFontAtlas* atlas);
    LINEARUI_API LinearU64         DebugTextureIDToU64(LinearTextureID tex_id);
    LINEARUI_API void          DebugHookIdInfo(LinearUIID id, LinearUIDataType data_type, const void* data_id, const void* data_id_end);
    LINEARUI_API void          DebugNodeColumns(LinearUIOldColumns* columns);
    LINEARUI_API void          DebugNodeDockNode(LinearUIDockNode* node, const char* label);
    LINEARUI_API void          DebugNodeDrawList(LinearUIWindow* window, LinearUIViewportP* viewport, const LinearDrawList* draw_list, const char* label);
    LINEARUI_API void          DebugNodeDrawCmdShowMeshAndBoundingBox(LinearDrawList* out_draw_list, const LinearDrawList* draw_list, const LinearDrawCmd* draw_cmd, bool show_mesh, bool show_aabb);
    LINEARUI_API void          DebugNodeFont(LinearFont* font);
    LINEARUI_API void          DebugNodeFontGlyphsForSrcMask(LinearFont* font, LinearFontBaked* baked, int src_mask);
    LINEARUI_API void          DebugNodeFontGlyph(LinearFont* font, const LinearFontGlyph* glyph);
    LINEARUI_API void          DebugNodeTexture(LinearTextureData* tex, int int_id, const LinearFontAtlasRect* highlight_rect = NULL); // ID used to facilitate persisting the "current" texture.
    LINEARUI_API void          DebugNodeStorage(LinearUIStorage* storage, const char* label);
    LINEARUI_API void          DebugNodeTabBar(LinearUITabBar* tab_bar, const char* label);
    LINEARUI_API void          DebugNodeTable(LinearUITable* table);
    LINEARUI_API void          DebugNodeTableSettings(LinearUITableSettings* settings);
    LINEARUI_API void          DebugNodeInputTextState(LinearUIInputTextState* state);
    LINEARUI_API void          DebugNodeTypingSelectState(LinearUITypingSelectState* state);
    LINEARUI_API void          DebugNodeMultiSelectState(LinearUIMultiSelectState* state);
    LINEARUI_API void          DebugNodeWindow(LinearUIWindow* window, const char* label);
    LINEARUI_API void          DebugNodeWindowSettings(LinearUIWindowSettings* settings);
    LINEARUI_API void          DebugNodeWindowsList(LinearVector<LinearUIWindow*>* windows, const char* label);
    LINEARUI_API void          DebugNodeWindowsListByBeginStackParent(LinearUIWindow** windows, int windows_size, LinearUIWindow* parent_in_begin_stack);
    LINEARUI_API void          DebugNodeViewport(LinearUIViewportP* viewport);
    LINEARUI_API void          DebugNodePlatformMonitor(LinearUIPlatformMonitor* monitor, const char* label, int idx);
    LINEARUI_API void          DebugRenderKeyboardPreview(LinearDrawList* draw_list);
    LINEARUI_API void          DebugRenderViewportThumbnail(LinearDrawList* draw_list, LinearUIViewportP* viewport, const LinearRect& bb);

    // Obsolete functions
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
    //inline void   SetItemUsingMouseWheel()                                            { SetItemKeyOwner(LinearUIKey_MouseWheelY); }      // Changed in 1.89
    //inline bool   TreeNodeBehaviorIsOpen(LinearUIID id, LinearUITreeNodeFlags flags = 0)    { return TreeNodeUpdateNextOpen(id, flags); }   // Renamed in 1.89
    //inline bool   IsKeyPressedMap(LinearUIKey key, bool repeat = true)                   { LINEARUI_ASSERT(IsNamedKey(key)); return IsKeyPressed(key, repeat); } // Removed in 1.87: Mapping from named key is always identity!

    // Refactored focus/nav/tabbing system in 1.82 and 1.84. If you have old/custom copy-and-pasted widgets which used FocusableItemRegister():
    //  (Old) LINEARUI_VERSION_NUM  < 18209: using 'ItemAdd(....)'                              and 'bool tab_focused = FocusableItemRegister(...)'
    //  (Old) LINEARUI_VERSION_NUM >= 18209: using 'ItemAdd(..., LinearUIItemAddFlags_Focusable)'  and 'bool tab_focused = (g.LastItemData.StatusFlags & LinearUIItemStatusFlags_Focused) != 0'
    //  (New) LINEARUI_VERSION_NUM >= 18413: using 'ItemAdd(..., LinearUIItemFlags_Inputable)'     and 'bool tab_focused = (g.NavActivateId == id && (g.NavActivateFlags & LinearUIActivateFlags_PreferInput))'
    //inline bool   FocusableItemRegister(LinearUIWindow* window, LinearUIID id)              // -> pass LinearUIItemAddFlags_Inputable flag to ItemAdd()
    //inline void   FocusableItemUnregister(LinearUIWindow* window)                        // -> unnecessary: TempInputText() uses LinearUIInputTextFlags_MergedItem
#endif

} // namespace LinearUI


//-----------------------------------------------------------------------------
// [SECTION] LinearFontLoader
//-----------------------------------------------------------------------------

// Hooks and storage for a given font backend.
// This structure is likely to evolve as we add support for incremental atlas updates.
// Conceptually this could be public, but API is still going to be evolve.
struct LinearFontLoader
{
    const char*     Name;
    bool            (*LoaderInit)(LinearFontAtlas* atlas);
    void            (*LoaderShutdown)(LinearFontAtlas* atlas);
    bool            (*FontSrcInit)(LinearFontAtlas* atlas, LinearFontConfig* src);
    void            (*FontSrcDestroy)(LinearFontAtlas* atlas, LinearFontConfig* src);
    bool            (*FontSrcContainsGlyph)(LinearFontAtlas* atlas, LinearFontConfig* src, LinearWchar codepoint);
    bool            (*FontBakedInit)(LinearFontAtlas* atlas, LinearFontConfig* src, LinearFontBaked* baked, void* loader_data_for_baked_src);
    void            (*FontBakedDestroy)(LinearFontAtlas* atlas, LinearFontConfig* src, LinearFontBaked* baked, void* loader_data_for_baked_src);
    bool            (*FontBakedLoadGlyph)(LinearFontAtlas* atlas, LinearFontConfig* src, LinearFontBaked* baked, void* loader_data_for_baked_src, LinearWchar codepoint, LinearFontGlyph* out_glyph, float* out_advance_x);

    // Size of backend data, Per Baked * Per Source. Buffers are managed by core to avoid excessive allocations.
    // FIXME: At this point the two other types of buffers may be managed by core to be consistent?
    size_t          FontBakedSrcLoaderDataSize;

    LinearFontLoader()  { memset((void*)this, 0, sizeof(*this)); }
};

#ifdef LINEARUI_ENABLE_STB_TRUETYPE
LINEARUI_API const LinearFontLoader* LinearFontAtlasGetFontLoaderForStbTruetype();
#endif
#ifndef LINEARUI_DISABLE_OBSOLETE_FUNCTIONS
typedef LinearFontLoader LinearFontBuilderIO; // [renamed/changed in 1.92.0] The types are not actually compatible but we provide this as a compile-time error report helper.
#endif

//-----------------------------------------------------------------------------
// [SECTION] LinearFontAtlas internal API
//-----------------------------------------------------------------------------

#define LINEARUI_FONT_SIZE_MAX                                     (512.0f)
#define LINEARUI_FONT_SIZE_THRESHOLD_FOR_LOADADVANCEXONLYMODE      (128.0f)

// Helpers: LinearTextureRef ==/!= operators provided as convenience
// (note that _TexID and _TexData are never set simultaneously)
inline bool operator==(const LinearTextureRef& lhs, const LinearTextureRef& rhs)    { return lhs._TexID == rhs._TexID && lhs._TexData == rhs._TexData; }
inline bool operator!=(const LinearTextureRef& lhs, const LinearTextureRef& rhs)    { return lhs._TexID != rhs._TexID || lhs._TexData != rhs._TexData; }

// Refer to LinearFontAtlasPackGetRect() to better understand how this works.
#define LinearFontAtlasRectId_IndexMask_        (0x0007FFFF)    // 20-bits signed: index to access builder->RectsIndex[].
#define LinearFontAtlasRectId_GenerationMask_   (0x3FF00000)    // 10-bits: entry generation, so each ID is unique and get can safely detected old identifiers.
#define LinearFontAtlasRectId_GenerationShift_  (20)
inline int               LinearFontAtlasRectId_GetIndex(LinearFontAtlasRectId id)       { return (id & LinearFontAtlasRectId_IndexMask_); }
inline unsigned int      LinearFontAtlasRectId_GetGeneration(LinearFontAtlasRectId id)  { return (unsigned int)(id & LinearFontAtlasRectId_GenerationMask_) >> LinearFontAtlasRectId_GenerationShift_; }
inline LinearFontAtlasRectId LinearFontAtlasRectId_Make(int index_idx, int gen_idx)     { LINEARUI_ASSERT(index_idx >= 0 && index_idx <= LinearFontAtlasRectId_IndexMask_ && gen_idx <= (LinearFontAtlasRectId_GenerationMask_ >> LinearFontAtlasRectId_GenerationShift_)); return (LinearFontAtlasRectId)(index_idx | (gen_idx << LinearFontAtlasRectId_GenerationShift_)); }

// Packed rectangle lookup entry (we need an indirection to allow removing/reordering rectangles)
// User are returned LinearFontAtlasRectId values which are meant to be persistent.
// We handle this with an indirection. While Rects[] may be in theory shuffled, compacted etc., RectsIndex[] cannot it is keyed by LinearFontAtlasRectId.
// RectsIndex[] is used both as an index into Rects[] and an index into itself. This is basically a free-list. See LinearFontAtlasBuildAllocRectIndexEntry() code.
// Having this also makes it easier to e.g. sort rectangles during repack.
struct LinearFontAtlasRectEntry
{
    int                 TargetIndex : 20;   // When Used: LinearFontAtlasRectId -> into Rects[]. When unused: index to next unused RectsIndex[] slot to consume free-list.
    unsigned int        Generation : 10;    // Increased each time the entry is reused for a new rectangle.
    unsigned int        IsUsed : 1;
};

// Data available to potential texture post-processing functions
struct LinearFontAtlasPostProcessData
{
    LinearFontAtlas*        FontAtlas;
    LinearFont*             Font;
    LinearFontConfig*       FontSrc;
    LinearFontBaked*        FontBaked;
    LinearFontGlyph*        Glyph;

    // Pixel data
    void*               Pixels;
    LinearTextureFormat     Format;
    int                 Pitch;
    int                 Width;
    int                 Height;
};

// We avoid dragging linearuistb_rectpack.h into public header (partly because binding generators are having issues with it)
#ifdef LINEARUI_STB_NAMESPACE
namespace LINEARUI_STB_NAMESPACE { struct linearstbrp_node; }
typedef LINEARUI_STB_NAMESPACE::linearstbrp_node linearstbrp_node_im;
#else
struct linearstbrp_node;
typedef linearstbrp_node linearstbrp_node_im;
#endif
struct linearstbrp_context_opaque { char data[80]; };

// Internal storage for incrementally packing and building a LinearFontAtlas
struct LinearFontAtlasBuilder
{
    linearstbrp_context_opaque        PackContext;            // Actually 'linearstbrp_context' but we don't want to define this in the header file.
    LinearVector<linearstbrp_node_im>     PackNodes;
    LinearVector<LinearTextureRect>     Rects;
    LinearVector<LinearFontAtlasRectEntry> RectsIndex;          // LinearFontAtlasRectId -> index into Rects[]
    LinearVector<unsigned char>     TempBuffer;             // Misc scratch buffer
    int                         RectsIndexFreeListStart;// First unused entry
    int                         RectsPackedCount;       // Number of packed rectangles.
    int                         RectsPackedSurface;     // Number of packed pixels. Used when compacting to heuristically find the ideal texture size.
    int                         RectsDiscardedCount;
    int                         RectsDiscardedSurface;
    int                         FrameCount;             // Current frame count
    LinearVec2i                     MaxRectSize;            // Largest rectangle to pack (de-facto used as a "minimum texture size")
    LinearVec2i                     MaxRectBounds;          // Bottom-right most used pixels
    bool                        LockDisableResize;      // Disable resizing texture
    bool                        PreloadedAllGlyphsRanges; // Set when missing LinearUIBackendFlags_RendererHasTextures features forces atlas to preload everything.

    // Cache of all LinearFontBaked
    LinearStableVector<LinearFontBaked,32> BakedPool;
    LinearUIStorage                BakedMap;               // BakedId --> LinearFontBaked*
    int                         BakedDiscardedCount;

    // Custom rectangle identifiers
    LinearFontAtlasRectId           PackIdMouseCursors;     // White pixel + mouse cursors. Also happen to be fallback in case of packing failure.
    LinearFontAtlasRectId           PackIdLinesTexData;

    LinearFontAtlasBuilder()        { memset((void*)this, 0, sizeof(*this)); FrameCount = -1; RectsIndexFreeListStart = -1; PackIdMouseCursors = PackIdLinesTexData = -1; }
};

LINEARUI_API void              LinearFontAtlasBuildInit(LinearFontAtlas* atlas);
LINEARUI_API void              LinearFontAtlasBuildDestroy(LinearFontAtlas* atlas);
LINEARUI_API void              LinearFontAtlasBuildMain(LinearFontAtlas* atlas);
LINEARUI_API void              LinearFontAtlasBuildSetupFontLoader(LinearFontAtlas* atlas, const LinearFontLoader* font_loader);
LINEARUI_API void              LinearFontAtlasBuildNotifySetFont(LinearFontAtlas* atlas, LinearFont* old_font, LinearFont* new_font);
LINEARUI_API void              LinearFontAtlasBuildUpdatePointers(LinearFontAtlas* atlas);
LINEARUI_API void              LinearFontAtlasBuildRenderBitmapFromString(LinearFontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char);
LINEARUI_API void              LinearFontAtlasBuildClear(LinearFontAtlas* atlas); // Clear output and custom rects

LINEARUI_API LinearTextureData*    LinearFontAtlasTextureAdd(LinearFontAtlas* atlas, int w, int h);
LINEARUI_API void              LinearFontAtlasTextureMakeSpace(LinearFontAtlas* atlas);
LINEARUI_API void              LinearFontAtlasTextureRepack(LinearFontAtlas* atlas, int w, int h);
LINEARUI_API void              LinearFontAtlasTextureGrow(LinearFontAtlas* atlas, int old_w = -1, int old_h = -1);
LINEARUI_API void              LinearFontAtlasTextureCompact(LinearFontAtlas* atlas);
LINEARUI_API LinearVec2i           LinearFontAtlasTextureGetSizeEstimate(LinearFontAtlas* atlas);

LINEARUI_API void              LinearFontAtlasBuildSetupFontSpecialGlyphs(LinearFontAtlas* atlas, LinearFont* font, LinearFontConfig* src);
LINEARUI_API void              LinearFontAtlasBuildLegacyPreloadAllGlyphRanges(LinearFontAtlas* atlas); // Legacy
LINEARUI_API void              LinearFontAtlasBuildGetOversampleFactors(LinearFontConfig* src, LinearFontBaked* baked, int* out_oversample_h, int* out_oversample_v);
LINEARUI_API void              LinearFontAtlasBuildDiscardBakes(LinearFontAtlas* atlas, int unused_frames);

LINEARUI_API bool              LinearFontAtlasFontSourceInit(LinearFontAtlas* atlas, LinearFontConfig* src);
LINEARUI_API void              LinearFontAtlasFontSourceAddToFont(LinearFontAtlas* atlas, LinearFont* font, LinearFontConfig* src);
LINEARUI_API void              LinearFontAtlasFontDestroySourceData(LinearFontAtlas* atlas, LinearFontConfig* src);
LINEARUI_API bool              LinearFontAtlasFontInitOutput(LinearFontAtlas* atlas, LinearFont* font); // Using FontDestroyOutput/FontInitOutput sequence useful notably if font loader params have changed
LINEARUI_API void              LinearFontAtlasFontDestroyOutput(LinearFontAtlas* atlas, LinearFont* font);
LINEARUI_API void              LinearFontAtlasFontRebuildOutput(LinearFontAtlas* atlas, LinearFont* font);
LINEARUI_API void              LinearFontAtlasFontDiscardBakes(LinearFontAtlas* atlas, LinearFont* font, int unused_frames);

LINEARUI_API LinearUIID           LinearFontAtlasBakedGetId(LinearUIID font_id, float baked_size, float rasterizer_density);
LINEARUI_API LinearFontBaked*      LinearFontAtlasBakedGetOrAdd(LinearFontAtlas* atlas, LinearFont* font, float font_size, float font_rasterizer_density);
LINEARUI_API LinearFontBaked*      LinearFontAtlasBakedGetClosestMatch(LinearFontAtlas* atlas, LinearFont* font, float font_size, float font_rasterizer_density);
LINEARUI_API LinearFontBaked*      LinearFontAtlasBakedAdd(LinearFontAtlas* atlas, LinearFont* font, float font_size, float font_rasterizer_density, LinearUIID baked_id);
LINEARUI_API void              LinearFontAtlasBakedDiscard(LinearFontAtlas* atlas, LinearFont* font, LinearFontBaked* baked);
LINEARUI_API LinearFontGlyph*      LinearFontAtlasBakedAddFontGlyph(LinearFontAtlas* atlas, LinearFontBaked* baked, LinearFontConfig* src, const LinearFontGlyph* in_glyph);
LINEARUI_API void              LinearFontAtlasBakedAddFontGlyphAdvancedX(LinearFontAtlas* atlas, LinearFontBaked* baked, LinearFontConfig* src, LinearWchar codepoint, float advance_x);
LINEARUI_API void              LinearFontAtlasBakedDiscardFontGlyph(LinearFontAtlas* atlas, LinearFont* font, LinearFontBaked* baked, LinearFontGlyph* glyph);
LINEARUI_API void              LinearFontAtlasBakedSetFontGlyphBitmap(LinearFontAtlas* atlas, LinearFontBaked* baked, LinearFontConfig* src, LinearFontGlyph* glyph, LinearTextureRect* r, const unsigned char* src_pixels, LinearTextureFormat src_fmt, int src_pitch);

LINEARUI_API void              LinearFontAtlasPackInit(LinearFontAtlas* atlas);
LINEARUI_API LinearFontAtlasRectId LinearFontAtlasPackAddRect(LinearFontAtlas* atlas, int w, int h, LinearFontAtlasRectEntry* overwrite_entry = NULL);
LINEARUI_API LinearTextureRect*    LinearFontAtlasPackGetRect(LinearFontAtlas* atlas, LinearFontAtlasRectId id);
LINEARUI_API LinearTextureRect*    LinearFontAtlasPackGetRectSafe(LinearFontAtlas* atlas, LinearFontAtlasRectId id);
LINEARUI_API void              LinearFontAtlasPackDiscardRect(LinearFontAtlas* atlas, LinearFontAtlasRectId id);

LINEARUI_API void              LinearFontAtlasUpdateNewFrame(LinearFontAtlas* atlas, int frame_count, bool renderer_has_textures);
LINEARUI_API void              LinearFontAtlasAddDrawListSharedData(LinearFontAtlas* atlas, LinearDrawListSharedData* data);
LINEARUI_API void              LinearFontAtlasRemoveDrawListSharedData(LinearFontAtlas* atlas, LinearDrawListSharedData* data);
LINEARUI_API void              LinearFontAtlasUpdateDrawListsTextures(LinearFontAtlas* atlas, LinearTextureRef old_tex, LinearTextureRef new_tex);
LINEARUI_API void              LinearFontAtlasUpdateDrawListsSharedData(LinearFontAtlas* atlas);

LINEARUI_API void              LinearFontAtlasTextureBlockConvert(const unsigned char* src_pixels, LinearTextureFormat src_fmt, int src_pitch, unsigned char* dst_pixels, LinearTextureFormat dst_fmt, int dst_pitch, int w, int h);
LINEARUI_API void              LinearFontAtlasTextureBlockPostProcess(LinearFontAtlasPostProcessData* data);
LINEARUI_API void              LinearFontAtlasTextureBlockPostProcessMultiply(LinearFontAtlasPostProcessData* data, float multiply_factor);
LINEARUI_API void              LinearFontAtlasTextureBlockFill(LinearTextureData* dst_tex, int dst_x, int dst_y, int w, int h, LinearU32 col);
LINEARUI_API void              LinearFontAtlasTextureBlockCopy(LinearTextureData* src_tex, int src_x, int src_y, LinearTextureData* dst_tex, int dst_x, int dst_y, int w, int h);
LINEARUI_API void              LinearFontAtlasTextureBlockQueueUpload(LinearFontAtlas* atlas, LinearTextureData* tex, int x, int y, int w, int h);

LINEARUI_API bool              LinearTextureDataUpdateNewFrame(LinearTextureData* tex);
LINEARUI_API void              LinearTextureDataQueueUpload(LinearTextureData* tex, int x, int y, int w, int h);
LINEARUI_API int               LinearTextureDataGetFormatBytesPerPixel(LinearTextureFormat format);
LINEARUI_API const char*       LinearTextureDataGetStatusName(LinearTextureStatus status);
LINEARUI_API const char*       LinearTextureDataGetFormatName(LinearTextureFormat format);

#ifndef LINEARUI_DISABLE_DEBUG_TOOLS
LINEARUI_API void              LinearFontAtlasDebugLogTextureRequests(LinearFontAtlas* atlas);
#endif

LINEARUI_API bool      LinearFontAtlasGetMouseCursorTexData(LinearFontAtlas* atlas, LinearUIMouseCursor cursor_type, LinearVec2* out_offset, LinearVec2* out_size, LinearVec2 out_uv_border[2], LinearVec2 out_uv_fill[2]);

//-----------------------------------------------------------------------------
// [SECTION] Test Engine specific hooks (linearui_test_engine)
//-----------------------------------------------------------------------------

#ifdef LINEARUI_ENABLE_TEST_ENGINE
extern void         LinearUITestEngineHook_ItemAdd(LinearUIContext* ctx, LinearUIID id, const LinearRect& bb, const LinearUILastItemData* item_data);           // item_data may be NULL
extern void         LinearUITestEngineHook_ItemInfo(LinearUIContext* ctx, LinearUIID id, const char* label, LinearUIItemStatusFlags flags);
extern void         LinearUITestEngineHook_Log(LinearUIContext* ctx, const char* fmt, ...);
extern const char*  LinearUITestEngine_FindItemDebugLabel(LinearUIContext* ctx, LinearUIID id);

// In LINEARUI_VERSION_NUM >= 18934: changed LINEARUI_TEST_ENGINE_ITEM_ADD(bb,id) to LINEARUI_TEST_ENGINE_ITEM_ADD(id,bb,item_data);
#define LINEARUI_TEST_ENGINE_ITEM_ADD(_ID,_BB,_ITEM_DATA)      if (g.TestEngineHookItems) LinearUITestEngineHook_ItemAdd(&g, _ID, _BB, _ITEM_DATA)    // Register item bounding box
#define LINEARUI_TEST_ENGINE_ITEM_INFO(_ID,_LABEL,_FLAGS)      if (g.TestEngineHookItems) LinearUITestEngineHook_ItemInfo(&g, _ID, _LABEL, _FLAGS)    // Register item label and status flags (optional)
#define LINEARUI_TEST_ENGINE_LOG(_FMT,...)                     LinearUITestEngineHook_Log(&g, _FMT, __VA_ARGS__)                                      // Custom log entry from user land into test log
#else
#define LINEARUI_TEST_ENGINE_ITEM_ADD(_ID,_BB,_ITEM_DATA)      ((void)0)
#define LINEARUI_TEST_ENGINE_ITEM_INFO(_ID,_LABEL,_FLAGS)      ((void)g)
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

#endif // #ifndef LINEARUI_DISABLE
