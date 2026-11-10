// dear rblxui, v1.92.9 WIP
// (tables and columns code)

/*

Index of this file:

// [SECTION] Commentary
// [SECTION] Header mess
// [SECTION] Tables: Main code
// [SECTION] Tables: Simple accessors
// [SECTION] Tables: Row changes
// [SECTION] Tables: Columns changes
// [SECTION] Tables: Columns width management
// [SECTION] Tables: Drawing
// [SECTION] Tables: Sorting
// [SECTION] Tables: Headers
// [SECTION] Tables: Context Menu
// [SECTION] Tables: Settings (.ini data)
// [SECTION] Tables: Garbage Collection
// [SECTION] Tables: Debugging
// [SECTION] Columns, BeginColumns, EndColumns, etc.

*/

// Navigating this file:
// - In Visual Studio: Ctrl+Comma ("Edit.GoToAll") can follow symbols inside comments, whereas Ctrl+F12 ("Edit.GoToImplementation") cannot.
// - In Visual Studio w/ Visual Assist installed: Alt+G ("VAssistX.GoToImplementation") can also follow symbols inside comments.
// - In VS Code, CLion, etc.: Ctrl+Click can follow symbols inside comments.

//-----------------------------------------------------------------------------
// [SECTION] Commentary
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Typical tables call flow: (root level is generally public API):
//-----------------------------------------------------------------------------
// - BeginTable()                               user begin into a table
//    | BeginChild()                            - (if ScrollX/ScrollY is set)
//    | TableBeginInitMemory()                  - first time table is used
//    | TableResetSettings()                    - on settings reset
//    | TableLoadSettings()                     - on settings load
//    | TableBeginApplyRequests()               - apply queued resizing/reordering/hiding requests
//    | - TableSetColumnWidth()                 - apply resizing width (for mouse resize, often requested by previous frame)
//    |    - TableUpdateColumnsWeightFromWidth()- recompute columns weights (of stretch columns) from their respective width
// - TableSetupColumn()                         user submit columns details (optional)
// - TableSetupScrollFreeze()                   user submit scroll freeze information (optional)
//-----------------------------------------------------------------------------
// - TableUpdateLayout() [Internal]             followup to BeginTable(): setup everything: widths, columns positions, clipping rectangles. Automatically called by the FIRST call to TableNextRow() or TableHeadersRow().
//    | TableSetupDrawChannels()                - setup RblxDrawList channels
//    | TableUpdateBorders()                    - detect hovering columns for resize, ahead of contents submission
//    | TableBeginContextMenuPopup()
//    | - TableDrawDefaultContextMenu()         - draw right-click context menu contents
//-----------------------------------------------------------------------------
// - TableHeadersRow() or TableHeader()         user submit a headers row (optional)
//    | TableSortSpecsClickColumn()             - when left-clicked: alter sort order and sort direction
//    | TableOpenContextMenu()                  - when right-clicked: trigger opening of the default context menu
// - TableGetSortSpecs()                        user queries updated sort specs (optional, generally after submitting headers)
// - TableNextRow()                             user begin into a new row (also automatically called by TableHeadersRow())
//    | TableEndRow()                           - finish existing row
//    | TableBeginRow()                         - add a new row
// - TableSetColumnIndex() / TableNextColumn()  user begin into a cell
//    | TableEndCell()                          - close existing column/cell
//    | TableBeginCell()                        - enter into current column/cell
// - [...]                                      user emit contents
//-----------------------------------------------------------------------------
// - EndTable()                                 user ends the table
//    | TableDrawBorders()                      - draw outer borders, inner vertical borders
//    | TableMergeDrawChannels()                - merge draw channels if clipping isn't required
//    | EndChild()                              - (if ScrollX/ScrollY is set)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// TABLE SIZING
//-----------------------------------------------------------------------------
// (Read carefully because this is subtle but it does make sense!)
//-----------------------------------------------------------------------------
// About 'outer_size':
// Its meaning needs to differ slightly depending on if we are using ScrollX/ScrollY flags.
// Default value is RblxVec2(0.0f, 0.0f).
//   X
//   - outer_size.x <= 0.0f  ->  Right-align from window/work-rect right-most edge. With -FLT_MIN or 0.0f will align exactly on right-most edge.
//   - outer_size.x  > 0.0f  ->  Set Fixed width.
//   Y with ScrollX/ScrollY disabled: we output table directly in current window
//   - outer_size.y  < 0.0f  ->  Bottom-align (but will auto extend, unless _NoHostExtendY is set). Not meaningful if parent window can vertically scroll.
//   - outer_size.y  = 0.0f  ->  No minimum height (but will auto extend, unless _NoHostExtendY is set)
//   - outer_size.y  > 0.0f  ->  Set Minimum height (but will auto extend, unless _NoHostExtendY is set)
//   Y with ScrollX/ScrollY enabled: using a child window for scrolling
//   - outer_size.y  < 0.0f  ->  Bottom-align. Not meaningful if parent window can vertically scroll.
//   - outer_size.y  = 0.0f  ->  Bottom-align, consistent with BeginChild(). Not recommended unless table is last item in parent window.
//   - outer_size.y  > 0.0f  ->  Set Exact height. Recommended when using Scrolling on any axis.
//-----------------------------------------------------------------------------
// Outer size is also affected by the NoHostExtendX/NoHostExtendY flags.
// Important to note how the two flags have slightly different behaviors!
//   - RblxUITableFlags_NoHostExtendX -> Make outer width auto-fit to columns (overriding outer_size.x value). Only available when ScrollX/ScrollY are disabled and Stretch columns are not used.
//   - RblxUITableFlags_NoHostExtendY -> Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit). Only available when ScrollX/ScrollY is disabled. Data below the limit will be clipped and not visible.
// In theory RblxUITableFlags_NoHostExtendY could be the default and any non-scrolling tables with outer_size.y != 0.0f would use exact height.
// This would be consistent but perhaps less useful and more confusing (as vertically clipped items are not useful and not easily noticeable).
//-----------------------------------------------------------------------------
// About 'inner_width':
//   With ScrollX disabled:
//   - inner_width          ->  *ignored*
//   With ScrollX enabled:
//   - inner_width  < 0.0f  ->  *illegal* fit in known width (right align from outer_size.x) <-- weird
//   - inner_width  = 0.0f  ->  fit in outer_width: Fixed size columns will take space they need (if avail, otherwise shrink down), Stretch columns becomes Fixed columns.
//   - inner_width  > 0.0f  ->  override scrolling width, generally to be larger than outer_size.x. Fixed column take space they need (if avail, otherwise shrink down), Stretch columns share remaining space!
//-----------------------------------------------------------------------------
// Details:
// - If you want to use Stretch columns with ScrollX, you generally need to specify 'inner_width' otherwise the concept
//   of "available space" doesn't make sense.
// - Even if not really useful, we allow 'inner_width < outer_size.x' for consistency and to facilitate understanding
//   of what the value does.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// COLUMNS SIZING POLICIES
// (Reference: RblxUITableFlags_SizingXXX flags and RblxUITableColumnFlags_WidthXXX flags)
//-----------------------------------------------------------------------------
// About overriding column sizing policy and width/weight with TableSetupColumn():
// We use a default parameter of -1 for 'init_width'/'init_weight'.
//   - with RblxUITableColumnFlags_WidthFixed,    init_width  <= 0 (default)  --> width is automatic
//   - with RblxUITableColumnFlags_WidthFixed,    init_width  >  0 (explicit) --> width is custom
//   - with RblxUITableColumnFlags_WidthStretch,  init_weight <= 0 (default)  --> weight is 1.0f
//   - with RblxUITableColumnFlags_WidthStretch,  init_weight >  0 (explicit) --> weight is custom
// Widths are specified _without_ CellPadding. If you specify a width of 100.0f, the column will be cover (100.0f + Padding * 2.0f)
// and you can fit a 100.0f wide item in it without clipping and with padding honored.
//-----------------------------------------------------------------------------
// About default sizing policy (if you don't specify a RblxUITableColumnFlags_WidthXXXX flag)
//   - with Table policy RblxUITableFlags_SizingFixedFit      --> default Column policy is RblxUITableColumnFlags_WidthFixed, default Width is equal to contents width
//   - with Table policy RblxUITableFlags_SizingFixedSame     --> default Column policy is RblxUITableColumnFlags_WidthFixed, default Width is max of all contents width
//   - with Table policy RblxUITableFlags_SizingStretchSame   --> default Column policy is RblxUITableColumnFlags_WidthStretch, default Weight is 1.0f
//   - with Table policy RblxUITableFlags_SizingStretchWeight --> default Column policy is RblxUITableColumnFlags_WidthStretch, default Weight is proportional to contents
// Default Width and default Weight can be overridden when calling TableSetupColumn().
//-----------------------------------------------------------------------------
// About mixing Fixed/Auto and Stretch columns together:
//   - the typical use of mixing sizing policies is: any number of LEADING Fixed columns, followed by one or two TRAILING Stretch columns.
//   - using mixed policies with ScrollX does not make much sense, as using Stretch columns with ScrollX does not make much sense in the first place!
//     that is, unless 'inner_width' is passed to BeginTable() to explicitly provide a total width to layout columns in.
//   - when using RblxUITableFlags_SizingFixedSame with mixed columns, only the Fixed/Auto columns will match their widths to the width of the maximum contents.
//   - when using RblxUITableFlags_SizingStretchSame with mixed columns, only the Stretch columns will match their weights/widths.
//-----------------------------------------------------------------------------
// About using column width:
// If a column is manually resizable or has a width specified with TableSetupColumn():
//   - you may use GetContentRegionAvail().x to query the width available in a given column.
//   - right-side alignment features such as SetNextItemWidth(-x) or PushItemWidth(-x) will rely on this width.
// If the column is not resizable and has no width specified with TableSetupColumn():
//   - its width will be automatic and be set to the max of items submitted.
//   - therefore you generally cannot have ALL items of the columns use e.g. SetNextItemWidth(-FLT_MIN).
//   - but if the column has one or more items of known/fixed size, this will become the reference width used by SetNextItemWidth(-FLT_MIN).
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// TABLES CLIPPING/CULLING
//-----------------------------------------------------------------------------
// About clipping/culling of Rows in Tables:
// - For large numbers of rows, it is recommended you use RblxUIListClipper to submit only visible rows.
//   RblxUIListClipper is reliant on the fact that rows are of equal height.
//   See 'Demo->Tables->Vertical Scrolling' or 'Demo->Tables->Advanced' for a demo of using the clipper.
// - Note that auto-resizing columns don't play well with using the clipper.
//   By default a table with _ScrollX but without _Resizable will have column auto-resize.
//   So, if you want to use the clipper, make sure to either enable _Resizable, either setup columns width explicitly with _WidthFixed.
//-----------------------------------------------------------------------------
// About clipping/culling of Columns in Tables:
// - Both TableSetColumnIndex() and TableNextColumn() return true when the column is visible or performing
//   width measurements. Otherwise, you may skip submitting the contents of a cell/column, BUT ONLY if you know
//   it is not going to contribute to row height.
//   In many situations, you may skip submitting contents for every column but one (e.g. the first one).
// - Case A: column is not hidden by user, and at least partially in sight (most common case).
// - Case B: column is clipped / out of sight (because of scrolling or parent ClipRect): TableNextColumn() return false as a hint but we still allow layout output.
// - Case C: column is hidden explicitly by the user (e.g. via the context menu, or _DefaultHide column flag, etc.).
//
//                        [A]         [B]          [C]
//  TableNextColumn():    true        false        false       -> [userland] when TableNextColumn() / TableSetColumnIndex() returns false, user can skip submitting items but only if the column doesn't contribute to row height.
//          SkipItems:    false       false        true        -> [internal] when SkipItems is true, most widgets will early out if submitted, resulting is no layout output.
//           ClipRect:    normal      zero-width   zero-width  -> [internal] when ClipRect is zero, ItemAdd() will return false and most widgets will early out mid-way.
//  RblxDrawList output:    normal      dummy        dummy       -> [internal] when using the dummy channel, RblxDrawList submissions (if any) will be wasted (because cliprect is zero-width anyway).
//
// - We need to distinguish those cases because non-hidden columns that are clipped outside of scrolling bounds should still contribute their height to the row.
//   However, in the majority of cases, the contribution to row height is the same for all columns, or the tallest cells are known by the programmer.
//-----------------------------------------------------------------------------
// About clipping/culling of whole Tables:
// - Scrolling tables with a known outer size can be clipped earlier as BeginTable() will return false.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef RBLXUI_DEFINE_MATH_OPERATORS
#define RBLXUI_DEFINE_MATH_OPERATORS
#endif

#include "rblxui.h"
#ifndef RBLXUI_DISABLE
#include "rblxui_internal.h"

// System includes
#include <stdint.h>     // intptr_t

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127)     // condition expression is constant
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#if defined(_MSC_VER) && _MSC_VER >= 1922 // MSVC 2019 16.2 or later
#pragma warning (disable: 5054)     // operator '|': deprecated between enumerations of different types
#endif
#pragma warning (disable: 26451)    // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to a 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#pragma warning (disable: 26812)    // [Static Analyzer] The enum type 'xxx' is unscoped. Prefer 'enum class' over 'enum' (Enum.3).
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'                      // not all warnings are known by all Clang versions and they tend to be rename-happy.. so ignoring warnings triggers new warnings on some configuration. Great!
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast                            // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"                    // warning: comparing floating point with == or != is unsafe // storing and comparing against same constants (typically 0.0f) is ok.
#pragma clang diagnostic ignored "-Wformat"                         // warning: format specifies type 'int' but the argument has type 'unsigned int'
#pragma clang diagnostic ignored "-Wformat-nonliteral"              // warning: format string is not a string literal            // passing non-literal to vsnformat(). yes, user passing incorrect format strings can crash the code.
#pragma clang diagnostic ignored "-Wsign-conversion"                // warning: implicit conversion changes signedness
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant                    // some standard header variations use #define NULL 0
#pragma clang diagnostic ignored "-Wdouble-promotion"               // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#pragma clang diagnostic ignored "-Wenum-enum-conversion"           // warning: bitwise operation between different enumeration types ('XXXFlags_' and 'XXXFlagsPrivate_')
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"// warning: bitwise operation between different enumeration types ('XXXFlags_' and 'XXXFlagsPrivate_') is deprecated
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"            // warning: 'xxx' is an unsafe pointer used for buffer access
#pragma clang diagnostic ignored "-Wnontrivial-memaccess"           // warning: first argument in call to 'memset' is a pointer to non-trivially copyable type
#pragma clang diagnostic ignored "-Wswitch-default"                 // warning: 'switch' missing 'default' label
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                          // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wfloat-equal"                      // warning: comparing floating-point with '==' or '!=' is unsafe
#pragma GCC diagnostic ignored "-Wformat-nonliteral"                // warning: format not a string literal, format string not checked
#pragma GCC diagnostic ignored "-Wdouble-promotion"                 // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wformat"                           // warning: format '%p' expects argument of type 'int'/'void*', but argument X has type 'unsigned int'/'RblxUIWindow*'
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#pragma GCC diagnostic ignored "-Wclass-memaccess"                  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#pragma GCC diagnostic ignored "-Wconversion"                       // warning: conversion to 'xxxx' from 'xxxx' may change value
#pragma GCC diagnostic ignored "-Wsign-conversion"                  // warning: conversion to 'xxxx' from 'xxxx' may change the sign of the result
#endif

//-----------------------------------------------------------------------------
// [SECTION] Tables: Main code
//-----------------------------------------------------------------------------
// - TableFixFlags() [Internal]
// - TableFindByID() [Internal]
// - BeginTable()
// - BeginTableEx() [Internal]
// - TableBeginInitMemory() [Internal]
// - TableBeginApplyRequests() [Internal]
// - TableSetupColumnFlags() [Internal]
// - TableUpdateLayout() [Internal]
// - TableUpdateBorders() [Internal]
// - EndTable()
// - TableSetupColumn()
// - TableSetupScrollFreeze()
//-----------------------------------------------------------------------------

// Configuration
static const int TABLE_DRAW_CHANNEL_BG0 = 0;
static const int TABLE_DRAW_CHANNEL_BG2_FROZEN = 1;
static const int TABLE_DRAW_CHANNEL_NOCLIP = 2;                     // When using RblxUITableFlags_NoClip (this becomes the last visible channel)
static const float TABLE_BORDER_SIZE                     = 1.0f;    // FIXME-TABLE: Currently hard-coded because of clipping assumptions with outer borders rendering.
static const float TABLE_RESIZE_SEPARATOR_HALF_THICKNESS = 4.0f;    // Extend outside inner borders.
static const float TABLE_RESIZE_SEPARATOR_FEEDBACK_TIMER = 0.06f;   // Delay/timer before making the hover feedback (color+cursor) visible because tables/columns tends to be more cramped.

// Helper
inline RblxUITableFlags TableFixFlags(RblxUITableFlags flags, RblxUIWindow* outer_window)
{
    // Adjust flags: set default sizing policy
    if ((flags & RblxUITableFlags_SizingMask_) == 0)
        flags |= ((flags & RblxUITableFlags_ScrollX) || (outer_window->Flags & RblxUIWindowFlags_AlwaysAutoResize)) ? RblxUITableFlags_SizingFixedFit : RblxUITableFlags_SizingStretchSame;

    // Adjust flags: enable NoKeepColumnsVisible when using RblxUITableFlags_SizingFixedSame
    if ((flags & RblxUITableFlags_SizingMask_) == RblxUITableFlags_SizingFixedSame)
        flags |= RblxUITableFlags_NoKeepColumnsVisible;

    // Adjust flags: enforce borders when resizable
    if (flags & RblxUITableFlags_Resizable)
        flags |= RblxUITableFlags_BordersInnerV;

    // Adjust flags: disable NoHostExtendX/NoHostExtendY if we have any scrolling going on
    if (flags & (RblxUITableFlags_ScrollX | RblxUITableFlags_ScrollY))
        flags &= ~(RblxUITableFlags_NoHostExtendX | RblxUITableFlags_NoHostExtendY);

    // Adjust flags: NoBordersInBodyUntilResize takes priority over NoBordersInBody
    if (flags & RblxUITableFlags_NoBordersInBodyUntilResize)
        flags &= ~RblxUITableFlags_NoBordersInBody;

    // Adjust flags: disable saved settings if there's nothing to save
    if ((flags & (RblxUITableFlags_Resizable | RblxUITableFlags_Hideable | RblxUITableFlags_Reorderable | RblxUITableFlags_Sortable)) == 0)
        flags |= RblxUITableFlags_NoSavedSettings;

    // Inherit _NoSavedSettings from top-level window (child windows always have _NoSavedSettings set)
    if (outer_window->RootWindow->Flags & RblxUIWindowFlags_NoSavedSettings)
        flags |= RblxUITableFlags_NoSavedSettings;

    return flags;
}

RblxUITable* RblxUI::TableFindByID(RblxUIID id)
{
    RblxUIContext& g = *GRblxUI;
    return g.Tables.GetByKey(id);
}

// Read about "TABLE SIZING" at the top of this file.
bool    RblxUI::BeginTable(const char* str_id, int columns_count, RblxUITableFlags flags, const RblxVec2& outer_size, float inner_width)
{
    RblxUIID id = GetID(str_id);
    return BeginTableEx(str_id, id, columns_count, flags, outer_size, inner_width);
}

bool    RblxUI::BeginTableEx(const char* name, RblxUIID id, int columns_count, RblxUITableFlags flags, const RblxVec2& outer_size, float inner_width)
{
    RblxUIContext& g = *GRblxUI;
    RblxUIWindow* outer_window = GetCurrentWindow();
    if (outer_window->SkipItems) // Consistent with other tables + beneficial side effect that assert on miscalling EndTable() will be more visible.
        return false;

    // Sanity checks
    RBLXUI_ASSERT(columns_count > 0 && columns_count < RBLXUI_TABLE_MAX_COLUMNS);
    if (flags & RblxUITableFlags_ScrollX)
        RBLXUI_ASSERT(inner_width >= 0.0f);

    // If an outer size is specified ahead we will be able to early out when not visible. Exact clipping criteria may evolve.
    // FIXME: coarse clipping because access to table data causes two issues:
    // - instance numbers varying/unstable. may not be a direct problem for users, but could make outside access broken or confusing, e.g. TestEngine.
    // - can't implement support for RblxUIChildFlags_ResizeY as we need to somehow pull the height data from somewhere. this also needs stable instance numbers.
    // The side-effects of accessing table data on coarse clip would be:
    // - always reserving the pooled RblxUITable data ahead for a fully clipped table (minor IMHO). Also the 'outer_window_is_measuring_size' criteria may already be defeating this in some situations.
    // - always performing the GetOrAddByKey() O(log N) query in g.Tables.Map[].
    const bool use_child_window = (flags & (RblxUITableFlags_ScrollX | RblxUITableFlags_ScrollY)) != 0;
    const RblxVec2 avail_size = GetContentRegionAvail();
    const RblxVec2 actual_outer_size = RblxTrunc(CalcItemSize(outer_size, RblxMax(avail_size.x, RBLXUI_WINDOW_HARD_MIN_SIZE), use_child_window ? RblxMax(avail_size.y, RBLXUI_WINDOW_HARD_MIN_SIZE) : 0.0f));
    const RblxRect outer_rect(outer_window->DC.CursorPos, outer_window->DC.CursorPos + actual_outer_size);
    const bool outer_window_is_measuring_size = (outer_window->AutoFitFramesX > 0) || (outer_window->AutoFitFramesY > 0); // Doesn't apply to AlwaysAutoResize windows!
    if (use_child_window && IsClippedEx(outer_rect, 0) && !outer_window_is_measuring_size)
    {
        ItemSize(outer_rect);
        ItemAdd(outer_rect, id);
        g.NextWindowData.ClearFlags();
        return false;
    }

    // [DEBUG] Debug break requested by user
    if (g.DebugBreakInTable == id)
        RBLXUI_DEBUG_BREAK();

    // Acquire storage for the table
    RblxUITable* table = g.Tables.GetOrAddByKey(id);

    // Acquire temporary buffers
    const int table_idx = g.Tables.GetIndex(table);
    if (++g.TablesTempDataStacked > g.TablesTempData.Size)
        g.TablesTempData.resize(g.TablesTempDataStacked, RblxUITableTempData());
    RblxUITableTempData* temp_data = table->TempData = &g.TablesTempData[g.TablesTempDataStacked - 1];
    temp_data->TableIndex = table_idx;
    table->DrawSplitter = &table->TempData->DrawSplitter;
    table->DrawSplitter->Clear();

    // Fix flags
    table->IsDefaultSizingPolicy = (flags & RblxUITableFlags_SizingMask_) == 0;
    flags = TableFixFlags(flags, outer_window);

    // Initialize
    const int previous_frame_active = table->LastFrameActive;
    const int instance_no = (previous_frame_active != g.FrameCount) ? 0 : table->InstanceCurrent + 1;
    const RblxUITableFlags previous_flags = table->Flags;
    table->ID = id;
    table->Flags = flags;
    table->LastFrameActive = g.FrameCount;
    table->OuterWindow = table->InnerWindow = outer_window;
    table->ColumnsCount = columns_count;
    table->IsLayoutLocked = false;
    table->InnerWidth = inner_width;
    table->NavLayer = (RblxS8)outer_window->DC.NavLayerCurrent;
    temp_data->UserOuterSize = outer_size;

    // Instance data (for instance 0, TableID == TableInstanceID)
    RblxUIID instance_id;
    table->InstanceCurrent = (RblxS16)instance_no;
    if (instance_no > 0)
    {
        RBLXUI_ASSERT(table->ColumnsCount == columns_count && "BeginTable(): Cannot change columns count mid-frame while preserving same ID");
        if (table->InstanceDataExtra.Size < instance_no)
            table->InstanceDataExtra.push_back(RblxUITableInstanceData());
        instance_id = GetIDWithSeed(instance_no, GetIDWithSeed("##Instances", NULL, id)); // Push "##Instances" followed by (int)instance_no in ID stack.
    }
    else
    {
        instance_id = id;
    }
    RblxUITableInstanceData* table_instance = TableGetInstanceData(table, table->InstanceCurrent);
    table_instance->TableInstanceID = instance_id;

    // When not using a child window, WorkRect.Max will grow as we append contents.
    if (use_child_window)
    {
        // Ensure no vertical scrollbar appears if we only want horizontal one, to make flag consistent
        // (we have no other way to disable vertical scrollbar of a window while keeping the horizontal one showing)
        RblxVec2 override_content_size(FLT_MAX, FLT_MAX);
        if ((flags & RblxUITableFlags_ScrollX) && !(flags & RblxUITableFlags_ScrollY))
            override_content_size.y = FLT_MIN;

        // Ensure specified width (when not specified, Stretched columns will act as if the width == OuterWidth and
        // never lead to any scrolling). We don't handle inner_width < 0.0f, we could potentially use it to right-align
        // based on the right side of the child window work rect, which would require knowing ahead if we are going to
        // have decoration taking horizontal spaces (typically a vertical scrollbar).
        if ((flags & RblxUITableFlags_ScrollX) && inner_width > 0.0f)
            override_content_size.x = inner_width;

        if (override_content_size.x != FLT_MAX || override_content_size.y != FLT_MAX)
            SetNextWindowContentSize(RblxVec2(override_content_size.x != FLT_MAX ? override_content_size.x : 0.0f, override_content_size.y != FLT_MAX ? override_content_size.y : 0.0f));

        // Reset scroll if we are reactivating it
        if ((previous_flags & (RblxUITableFlags_ScrollX | RblxUITableFlags_ScrollY)) == 0)
            if ((g.NextWindowData.HasFlags & RblxUINextWindowDataFlags_HasScroll) == 0)
                SetNextWindowScroll(RblxVec2(0.0f, 0.0f));

        // Create scrolling region (without border and zero window padding)
        RblxUIChildFlags child_child_flags = (g.NextWindowData.HasFlags & RblxUINextWindowDataFlags_HasChildFlags) ? g.NextWindowData.ChildFlags : RblxUIChildFlags_None;
        RblxUIWindowFlags child_window_flags = (g.NextWindowData.HasFlags & RblxUINextWindowDataFlags_HasWindowFlags) ? g.NextWindowData.WindowFlags : RblxUIWindowFlags_None;
        if (flags & RblxUITableFlags_ScrollX)
            child_window_flags |= RblxUIWindowFlags_HorizontalScrollbar;
        BeginChildEx(name, instance_id, outer_rect.GetSize(), child_child_flags, child_window_flags);
        table->InnerWindow = g.CurrentWindow;
        table->WorkRect = table->InnerWindow->WorkRect;
        table->OuterRect = table->InnerWindow->Rect();
        table->InnerRect = table->InnerWindow->InnerRect;
        RBLXUI_ASSERT(table->InnerWindow->WindowPadding.x == 0.0f && table->InnerWindow->WindowPadding.y == 0.0f && table->InnerWindow->WindowBorderSize == 0.0f);

        // Allow submitting when host is measuring
        if (table->InnerWindow->SkipItems && outer_window_is_measuring_size)
            table->InnerWindow->SkipItems = false;

        // When using multiple instances, ensure they have the same amount of horizontal decorations (aka vertical scrollbar) so stretched columns can be aligned
        if (instance_no == 0)
        {
            table->HasScrollbarYPrev = table->HasScrollbarYCurr;
            table->HasScrollbarYCurr = false;
        }
        table->HasScrollbarYCurr |= table->InnerWindow->ScrollbarY;
    }
    else
    {
        // For non-scrolling tables, WorkRect == OuterRect == InnerRect.
        // But at this point we do NOT have a correct value for .Max.y (unless a height has been explicitly passed in). It will only be updated in EndTable().
        table->WorkRect = table->OuterRect = table->InnerRect = outer_rect;
        table->HasScrollbarYPrev = table->HasScrollbarYCurr = false;
        table->InnerWindow->DC.TreeDepth++; // This is designed to always linking RblxUITreeNodeFlags_DrawLines linking across a table
    }

    // Push a standardized ID for both child-using and not-child-using tables
    PushOverrideID(id);
    if (instance_no > 0)
        PushOverrideID(instance_id); // FIXME: Somehow this is not resolved by stack-tool, even tho GetIDWithSeed() submitted the symbol.

    // Backup a copy of host window members we will modify
    RblxUIWindow* inner_window = table->InnerWindow;
    table->HostIndentX = inner_window->DC.Indent.x;
    table->HostClipRect = inner_window->ClipRect;
    table->HostSkipItems = inner_window->SkipItems;
    temp_data->WindowID = inner_window->ID;
    temp_data->HostBackupWorkRect = inner_window->WorkRect;
    temp_data->HostBackupParentWorkRect = inner_window->ParentWorkRect;
    temp_data->HostBackupColumnsOffset = outer_window->DC.ColumnsOffset;
    temp_data->HostBackupPrevLineSize = inner_window->DC.PrevLineSize;
    temp_data->HostBackupCurrLineSize = inner_window->DC.CurrLineSize;
    temp_data->HostBackupCursorMaxPos = inner_window->DC.CursorMaxPos;
    temp_data->HostBackupItemWidth = outer_window->DC.ItemWidth;
    temp_data->HostBackupItemWidthStackSize = outer_window->DC.ItemWidthStack.Size;
    inner_window->DC.PrevLineSize = inner_window->DC.CurrLineSize = RblxVec2(0.0f, 0.0f);

    // Make borders not overlap our contents by offsetting HostClipRect (#6765, #7428, #3752)
    // (we normally shouldn't alter HostClipRect as we rely on TableMergeDrawChannels() expanding non-clipped column toward the
    // limits of that rectangle, in order for RblxDrawListSplitter::Merge() to merge the draw commands. However since the overlap
    // problem only affect scrolling tables in this case we can get away with doing it without extra cost).
    if (inner_window != outer_window)
    {
        // FIXME: Because inner_window's Scrollbar doesn't know about border size, since it's not encoded in window->WindowBorderSize,
        // it already overlaps it and doesn't need an extra offset. Ideally we should be able to pass custom border size with
        // different x/y values to BeginChild().
        if (flags & RblxUITableFlags_BordersOuterV)
        {
            table->HostClipRect.Min.x = RblxMin(table->HostClipRect.Min.x + TABLE_BORDER_SIZE, table->HostClipRect.Max.x);
            if (inner_window->DecoOuterSizeX2 == 0.0f)
                table->HostClipRect.Max.x = RblxMax(table->HostClipRect.Max.x - TABLE_BORDER_SIZE, table->HostClipRect.Min.x);
        }
        if (flags & RblxUITableFlags_BordersOuterH)
        {
            table->HostClipRect.Min.y = RblxMin(table->HostClipRect.Min.y + TABLE_BORDER_SIZE, table->HostClipRect.Max.y);
            if (inner_window->DecoOuterSizeY2 == 0.0f)
                table->HostClipRect.Max.y = RblxMax(table->HostClipRect.Max.y - TABLE_BORDER_SIZE, table->HostClipRect.Min.y);
        }
    }

    // Padding and Spacing
    // - None               ........Content..... Pad .....Content........
    // - PadOuter           | Pad ..Content..... Pad .....Content.. Pad |
    // - PadInner           ........Content.. Pad | Pad ..Content........
    // - PadOuter+PadInner  | Pad ..Content.. Pad | Pad ..Content.. Pad |
    const bool pad_outer_x = (flags & RblxUITableFlags_NoPadOuterX) ? false : (flags & RblxUITableFlags_PadOuterX) ? true : (flags & RblxUITableFlags_BordersOuterV) != 0;
    const bool pad_inner_x = (flags & RblxUITableFlags_NoPadInnerX) ? false : true;
    const float inner_spacing_for_border = (flags & RblxUITableFlags_BordersInnerV) ? TABLE_BORDER_SIZE : 0.0f;
    const float inner_spacing_explicit = (pad_inner_x && (flags & RblxUITableFlags_BordersInnerV) == 0) ? g.Style.CellPadding.x : 0.0f;
    const float inner_padding_explicit = (pad_inner_x && (flags & RblxUITableFlags_BordersInnerV) != 0) ? g.Style.CellPadding.x : 0.0f;
    table->CellSpacingX1 = inner_spacing_explicit + inner_spacing_for_border;
    table->CellSpacingX2 = inner_spacing_explicit;
    table->CellPaddingX = inner_padding_explicit;

    const float outer_padding_for_border = (flags & RblxUITableFlags_BordersOuterV) ? TABLE_BORDER_SIZE : 0.0f;
    const float outer_padding_explicit = pad_outer_x ? g.Style.CellPadding.x : 0.0f;
    table->OuterPaddingX = (outer_padding_for_border + outer_padding_explicit) - table->CellPaddingX;

    table->CurrentColumn = -1;
    table->CurrentRow = -1;
    table->RowBgColorCounter = 0;
    table->LastRowFlags = RblxUITableRowFlags_None;
    table->InnerClipRect = (inner_window == outer_window) ? table->WorkRect : inner_window->ClipRect;
    table->InnerClipRect.ClipWith(table->WorkRect);     // We need this to honor inner_width
    table->InnerClipRect.ClipWithFull(table->HostClipRect);
    table->InnerClipRect.Max.y = (flags & RblxUITableFlags_NoHostExtendY) ? RblxMin(table->InnerClipRect.Max.y, inner_window->WorkRect.Max.y) : table->HostClipRect.Max.y;

    table->RowPosY1 = table->RowPosY2 = table->WorkRect.Min.y; // This is needed somehow
    table->RowTextBaseline = 0.0f; // This will be cleared again by TableBeginRow()
    table->RowCellPaddingY = 0.0f;
    table->FreezeRowsRequest = table->FreezeRowsCount = 0; // This will be setup by TableSetupScrollFreeze(), if any
    table->FreezeColumnsRequest = table->FreezeColumnsCount = 0;
    table->IsUnfrozenRows = true;
    table->DeclColumnsCount = table->AngledHeadersCount = 0;
    if (previous_frame_active + 1 < g.FrameCount)
        table->IsActiveIdInTable = false;
    table->AngledHeadersHeight = 0.0f;
    temp_data->AngledHeadersExtraWidth = 0.0f;

    // Using opaque colors facilitate overlapping lines of the grid, otherwise we'd need to improve TableDrawBorders()
    table->BorderColorStrong = GetColorU32(RblxUICol_TableBorderStrong);
    table->BorderColorLight = GetColorU32(RblxUICol_TableBorderLight);

    // Make table current
    g.CurrentTable = table;
    inner_window->DC.NavIsScrollPushableX = false; // Shortcut for NavUpdateCurrentWindowIsScrollPushableX();
    outer_window->DC.CurrentTableIdx = table_idx;
    if (inner_window != outer_window) // So EndChild() within the inner window can restore the table properly.
        inner_window->DC.CurrentTableIdx = table_idx;

    if ((previous_flags & RblxUITableFlags_Reorderable) && (flags & RblxUITableFlags_Reorderable) == 0)
        table->IsResetDisplayOrderRequest = true;

    // Mark as used to avoid GC
    if (table_idx >= g.TablesLastTimeActive.Size)
        g.TablesLastTimeActive.resize(table_idx + 1, -1.0f);
    g.TablesLastTimeActive[table_idx] = (float)g.Time;
    temp_data->LastTimeActive = (float)g.Time;
    table->MemoryCompacted = false;

    // Setup memory buffer (clear data if columns count changed)
    RblxUITableColumn* old_columns_to_preserve = NULL;
    void* old_columns_raw_data = NULL;
    const int old_columns_count = table->Columns.size();
    if (old_columns_count != 0 && old_columns_count != columns_count)
    {
        // Attempt to preserve width and other settings on column count/specs change (#4046)
        old_columns_to_preserve = table->Columns.Data;
        old_columns_raw_data = table->RawData; // Free at end of function
        table->RawData = NULL;
    }
    if (table->RawData == NULL)
    {
        TableBeginInitMemory(table, columns_count);
        table->IsInitializing = table->IsSettingsRequestLoad = true;
    }
    if (table->IsResetAllRequest)
        TableResetSettings(table);
    if (table->IsInitializing)
    {
        // Initialize
        table->SettingsOffset = -1;
        table->IsSortSpecsDirty = true;
        table->IsSettingsDirty = true; // Records itself into .ini file even when in default state (#7934)
        table->InstanceInteracted = -1;
        table->ContextPopupColumn = -1;
        table->ReorderColumn = table->ReorderColumnDstOrder = table->ResizedColumn = table->LastResizedColumn = -1;
        table->AutoFitSingleColumn = -1;
        table->HoveredColumnBody = table->HoveredColumnBorder = -1;
        for (int n = 0; n < columns_count; n++)
        {
            RblxUITableColumn* column = &table->Columns[n];
            if (old_columns_to_preserve && n < old_columns_count)
            {
                *column = old_columns_to_preserve[n];
            }
            else
            {
                float width_auto = column->WidthAuto;
                *column = RblxUITableColumn();
                column->WidthAuto = width_auto;
                column->IsPreserveWidthAuto = true; // Preserve WidthAuto when reinitializing a live table: not technically necessary but remove a visible flicker
                column->IsEnabled = column->IsUserEnabled = column->IsUserEnabledNextFrame = true;
                column->DisplayOrder = (RblxUITableColumnIdx)n;
            }
            table->DisplayOrderToIndex[n] = column->DisplayOrder;
        }
    }
    if (old_columns_raw_data)
        RBLXUI_FREE(old_columns_raw_data);

    // Load settings
    if (table->IsSettingsRequestLoad)
        TableLoadSettings(table);

    // Handle DPI/font resize
    // This is designed to facilitate DPI changes with the assumption that e.g. style.CellPadding has been scaled as well.
    // It will also react to changing fonts with mixed results. It doesn't need to be perfect but merely provide a decent transition.
    // FIXME-DPI: Provide consistent standards for reference size. Perhaps using g.CurrentDpiScale would be more self explanatory.
    // This is will lead us to non-rounded WidthRequest in columns, which should work but is a poorly tested path.
    const float new_ref_scale_unit = g.FontSize; // g.Font->GetCharAdvance('A') ?
    if (table->RefScale != 0.0f && table->RefScale != new_ref_scale_unit)
    {
        const float scale_factor = new_ref_scale_unit / table->RefScale;
        //RBLXUI_DEBUG_PRINT("[table] %08X RefScaleUnit %.3f -> %.3f, scaling width by %.3f\n", table->ID, table->RefScaleUnit, new_ref_scale_unit, scale_factor);
        for (int n = 0; n < columns_count; n++)
            table->Columns[n].WidthRequest = table->Columns[n].WidthRequest * scale_factor;
    }
    table->RefScale = new_ref_scale_unit;

    // Disable output until user calls TableNextRow() or TableNextColumn() leading to the TableUpdateLayout() call..
    // This is not strictly necessary but will reduce cases were "out of table" output will be misleading to the user.
    // Because we cannot safely assert in EndTable() when no rows have been created, this seems like our best option.
    inner_window->SkipItems = true;

    // Clear names
    // At this point the ->NameOffset field of each column will be invalid until TableUpdateLayout() or the first call to TableSetupColumn()
    if (table->ColumnsNames.Buf.Size > 0)
        table->ColumnsNames.Buf.resize(0);

    // Apply queued resizing/reordering/hiding requests
    TableBeginApplyRequests(table);

    return true;
}

// For reference, the average total _allocation count_ for a table is:
// + 0 (for RblxUITable instance, we are pooling allocations in g.Tables[])
// + 1 (for table->RawData allocated below)
// + 1 (for table->ColumnsNames, if names are used)
// Shared allocations for the maximum number of simultaneously nested tables (generally a very small number)
// + 1 (for table->Splitter._Channels)
// + 2 * active_channels_count (for RblxDrawCmd and RblxDrawIdx buffers inside channels)
// Where active_channels_count is variable but often == columns_count or == columns_count + 1, see TableSetupDrawChannels() for details.
// Unused channels don't perform their +2 allocations.
void RblxUI::TableBeginInitMemory(RblxUITable* table, int columns_count)
{
    // Allocate single buffer for our arrays
    const int columns_bit_array_size = (int)RblxBitArrayGetStorageSizeInBytes(columns_count);
    RblxSpanAllocator<6> span_allocator;
    span_allocator.Reserve(0, columns_count * sizeof(RblxUITableColumn));
    span_allocator.Reserve(1, columns_count * sizeof(RblxUITableColumnIdx));
    span_allocator.Reserve(2, columns_count * sizeof(RblxUITableCellData), 4);
    for (int n = 3; n < 6; n++)
        span_allocator.Reserve(n, columns_bit_array_size);
    table->RawData = RBLXUI_ALLOC(span_allocator.GetArenaSizeInBytes());
    memset(table->RawData, 0, span_allocator.GetArenaSizeInBytes());
    span_allocator.SetArenaBasePtr(table->RawData);
    span_allocator.GetSpan(0, &table->Columns);
    span_allocator.GetSpan(1, &table->DisplayOrderToIndex);
    span_allocator.GetSpan(2, &table->RowCellData);
    table->EnabledMaskByDisplayOrder = (RblxU32*)span_allocator.GetSpanPtrBegin(3);
    table->EnabledMaskByIndex = (RblxU32*)span_allocator.GetSpanPtrBegin(4);
    table->VisibleMaskByIndex = (RblxU32*)span_allocator.GetSpanPtrBegin(5);
}

// Apply queued resizing/reordering/hiding requests
void RblxUI::TableBeginApplyRequests(RblxUITable* table)
{
    // Handle resizing request
    // (We process this in the TableBegin() of the first instance of each table)
    // FIXME-TABLE: Contains columns if our work area doesn't allow for scrolling?
    if (table->InstanceCurrent == 0)
    {
        if (table->ResizedColumn != -1 && table->ResizedColumnNextWidth != FLT_MAX)
            TableSetColumnWidth(table->ResizedColumn, table->ResizedColumnNextWidth);
        table->LastResizedColumn = table->ResizedColumn;
        table->ResizedColumnNextWidth = FLT_MAX;
        table->ResizedColumn = -1;

        // Process auto-fit for single column, which is a special case for stretch columns and fixed columns with FixedSame policy.
        // FIXME-TABLE: Would be nice to redistribute available stretch space accordingly to other weights, instead of giving it all to siblings.
        if (table->AutoFitSingleColumn != -1)
        {
            TableSetColumnWidth(table->AutoFitSingleColumn, table->Columns[table->AutoFitSingleColumn].WidthAuto);
            table->AutoFitSingleColumn = -1;
        }
    }

    // Handle reordering request
    //// Note: we don't clear ReorderColumn after handling the request (FIXME: clarify why or add a test).
    if (table->InstanceCurrent == 0)
    {
        table->LastHeldHeaderColumn = table->HeldHeaderColumn;
        table->HeldHeaderColumn = -1;
        if (table->ReorderColumn != -1 && table->ReorderColumnDstOrder != -1)
        {
            TableSetColumnDisplayOrder(table, table->ReorderColumn, table->ReorderColumnDstOrder);
            table->ReorderColumnDstOrder = -1;
        }

        // Release
        RblxUIContext& g = *GRblxUI;
        if (g.ActiveId == 0) // FIXME: Need to revisit. See 38f5e5a.
            table->ReorderColumn = -1;
        //if (table->HeldHeaderColumn == -1 && table->ReorderColumn != -1)
        //    table->ReorderColumn = -1;
    }

    // Handle display order reset request
    if (table->IsResetDisplayOrderRequest)
    {
        for (int n = 0; n < table->ColumnsCount; n++)
            table->DisplayOrderToIndex[n] = table->Columns[n].DisplayOrder = (RblxUITableColumnIdx)n;
        table->IsResetDisplayOrderRequest = false;
        table->IsSettingsDirty = true;
    }
}

// Apply immediately. See TableQueueSetColumnDisplayOrder() for additional checks/constraints.
void RblxUI::TableSetColumnDisplayOrder(RblxUITable* table, int column_n, int dst_order)
{
    RBLXUI_ASSERT(column_n >= 0 && column_n < table->ColumnsCount);
    RBLXUI_ASSERT(dst_order >= 0 && dst_order < table->ColumnsCount);

    RblxUITableColumn* src_column = &table->Columns[column_n];
    const int src_order = src_column->DisplayOrder;
    if (src_order == dst_order)
        return;
    const int reorder_dir = (dst_order < src_order) ? -1 : +1;

    src_column->DisplayOrder = (RblxUITableColumnIdx)dst_order;
    for (int order_n = src_order + reorder_dir; order_n != dst_order + reorder_dir; order_n += reorder_dir)
        table->Columns[table->DisplayOrderToIndex[order_n]].DisplayOrder -= (RblxUITableColumnIdx)reorder_dir;
    //RBLXUI_ASSERT(dst_column->DisplayOrder == dst_order - reorder_dir);

    // Display order is stored in both columns->IndexDisplayOrder and table->DisplayOrder[]. Rebuild later from the former.
    // FIXME-OPT: If this is called multiple times we'd effectively have a O(N^2) thing going on.
    for (int n = 0; n < table->ColumnsCount; n++)
        table->DisplayOrderToIndex[table->Columns[n].DisplayOrder] = (RblxUITableColumnIdx)n;
    table->IsSettingsDirty = true;
}

static int TableGetMaxDisplayOrderAllowed(RblxUITable* table, int src_order, int dst_order)
{
    dst_order = RblxClamp(dst_order, 0, table->ColumnsCount - 1);
    if (src_order == dst_order)
        return dst_order;

    // Cannot cross over the frozen column limit when interactively reordering.
    // TableSetupScrollFreeze() enforce a display order range for frozen columns. Reordering across the frozen column barrier is illegal and will be undone.
    if (table->FreezeColumnsRequest > 0)
        dst_order = (src_order < table->FreezeColumnsRequest) ? RblxMin(dst_order, (int)table->FreezeColumnsRequest - 1) : RblxMax(dst_order, (int)table->FreezeColumnsRequest);

    // Cannot cross over a column with the RblxUITableColumnFlags_NoReorder flag.
    int reorder_dir = (src_order < dst_order) ? +1 : -1;
    for (int order_n = src_order; (src_order < dst_order && order_n <= dst_order) || (dst_order < src_order && order_n >= dst_order); order_n += reorder_dir)
        if (table->Columns[table->DisplayOrderToIndex[order_n]].Flags & RblxUITableColumnFlags_NoReorder)
        {
            dst_order = (order_n == src_order) ? src_order : order_n - reorder_dir;
            break;
        }
    return dst_order;
}

// Reorder requested by user interaction.
void RblxUI::TableQueueSetColumnDisplayOrder(RblxUITable* table, int column_n, int dst_order)
{
    const int src_order = table->Columns[column_n].DisplayOrder;
    table->ReorderColumn = (RblxUITableColumnIdx)column_n;
    table->ReorderColumnDstOrder = (RblxUITableColumnIdx)-1;
    dst_order = TableGetMaxDisplayOrderAllowed(table, src_order, dst_order);
    if (dst_order != src_order)
        table->ReorderColumnDstOrder = (RblxUITableColumnIdx)dst_order;
}

// Adjust flags: default width mode + stretch columns are not allowed when auto extending
static void TableSetupColumnFlags(RblxUITable* table, RblxUITableColumn* column, RblxUITableColumnFlags flags_in)
{
    RblxUITableColumnFlags flags = flags_in;

    // Sizing Policy
    if ((flags & RblxUITableColumnFlags_WidthMask_) == 0)
    {
        const RblxUITableFlags table_sizing_policy = (table->Flags & RblxUITableFlags_SizingMask_);
        if (table_sizing_policy == RblxUITableFlags_SizingFixedFit || table_sizing_policy == RblxUITableFlags_SizingFixedSame)
            flags |= RblxUITableColumnFlags_WidthFixed;
        else
            flags |= RblxUITableColumnFlags_WidthStretch;
    }
    else
    {
        RBLXUI_ASSERT(RblxIsPowerOfTwo(flags & RblxUITableColumnFlags_WidthMask_)); // Check that only 1 of each set is used.
    }

    // Resize
    if ((table->Flags & RblxUITableFlags_Resizable) == 0)
        flags |= RblxUITableColumnFlags_NoResize;

    // Sorting
    if ((flags & RblxUITableColumnFlags_NoSortAscending) && (flags & RblxUITableColumnFlags_NoSortDescending))
        flags |= RblxUITableColumnFlags_NoSort;

    // Indentation
    if ((flags & RblxUITableColumnFlags_IndentMask_) == 0)
        flags |= (table->Columns.index_from_ptr(column) == 0) ? RblxUITableColumnFlags_IndentEnable : RblxUITableColumnFlags_IndentDisable;

    // Alignment
    //if ((flags & RblxUITableColumnFlags_AlignMask_) == 0)
    //    flags |= RblxUITableColumnFlags_AlignCenter;
    //RBLXUI_ASSERT(RblxIsPowerOfTwo(flags & RblxUITableColumnFlags_AlignMask_)); // Check that only 1 of each set is used.

    // Preserve status flags
    column->Flags = flags | (column->Flags & RblxUITableColumnFlags_StatusMask_);

    // Build an ordered list of available sort directions
    column->SortDirectionsAvailCount = column->SortDirectionsAvailMask = column->SortDirectionsAvailList = 0;
    if (table->Flags & RblxUITableFlags_Sortable)
    {
        int count = 0, mask = 0, list = 0;
        if ((flags & RblxUITableColumnFlags_PreferSortAscending)  != 0 && (flags & RblxUITableColumnFlags_NoSortAscending)  == 0) { mask |= 1 << RblxUISortDirection_Ascending;  list |= RblxUISortDirection_Ascending  << (count << 1); count++; }
        if ((flags & RblxUITableColumnFlags_PreferSortDescending) != 0 && (flags & RblxUITableColumnFlags_NoSortDescending) == 0) { mask |= 1 << RblxUISortDirection_Descending; list |= RblxUISortDirection_Descending << (count << 1); count++; }
        if ((flags & RblxUITableColumnFlags_PreferSortAscending)  == 0 && (flags & RblxUITableColumnFlags_NoSortAscending)  == 0) { mask |= 1 << RblxUISortDirection_Ascending;  list |= RblxUISortDirection_Ascending  << (count << 1); count++; }
        if ((flags & RblxUITableColumnFlags_PreferSortDescending) == 0 && (flags & RblxUITableColumnFlags_NoSortDescending) == 0) { mask |= 1 << RblxUISortDirection_Descending; list |= RblxUISortDirection_Descending << (count << 1); count++; }
        if ((table->Flags & RblxUITableFlags_SortTristate) || count == 0) { mask |= 1 << RblxUISortDirection_None; count++; }
        column->SortDirectionsAvailList = (RblxU8)list;
        column->SortDirectionsAvailMask = (RblxU8)mask;
        column->SortDirectionsAvailCount = (RblxU8)count;
        RblxUI::TableFixColumnSortDirection(table, column);
    }
}

// Layout columns for the frame. This is in essence the followup to BeginTable() and this is our largest function.
// Runs on the first call to TableNextRow(), to give a chance for TableSetupColumn() and other TableSetupXXXXX() functions to be called first.
// FIXME-TABLE: Our width (and therefore our WorkRect) will be minimal in the first frame for _WidthAuto columns.
// Increase feedback side-effect with widgets relying on WorkRect.Max.x... Maybe provide a default distribution for _WidthAuto columns?
void RblxUI::TableUpdateLayout(RblxUITable* table)
{
    RblxUIContext& g = *GRblxUI;
    RBLXUI_ASSERT(table->IsLayoutLocked == false);

    const RblxUITableFlags table_sizing_policy = (table->Flags & RblxUITableFlags_SizingMask_);
    table->IsDefaultDisplayOrder = true;
    table->ColumnsEnabledCount = 0;
    RblxBitArrayClearAllBits(table->EnabledMaskByIndex, table->ColumnsCount);
    RblxBitArrayClearAllBits(table->EnabledMaskByDisplayOrder, table->ColumnsCount);
    table->LeftMostEnabledColumn = -1;
    table->MinColumnWidth = RblxMax(1.0f, g.Style.FramePadding.x * 1.0f); // g.Style.ColumnsMinSpacing; // FIXME-TABLE

    // [Part 1] Apply/lock Enabled and Order states. Calculate auto/ideal width for columns. Count fixed/stretch columns.
    // Process columns in their visible orders as we are building the Prev/Next indices.
    int count_fixed = 0;                // Number of columns that have fixed sizing policies
    int count_stretch = 0;              // Number of columns that have stretch sizing policies
    int prev_visible_column_idx = -1;
    bool has_auto_fit_request = false;
    bool has_resizable = false;
    float stretch_sum_width_auto = 0.0f;
    float fixed_max_width_auto = 0.0f;
    for (int order_n = 0; order_n < table->ColumnsCount; order_n++)
    {
        const int column_n = table->DisplayOrderToIndex[order_n];
        if (column_n != order_n)
            table->IsDefaultDisplayOrder = false;
        RblxUITableColumn* column = &table->Columns[column_n];

        // Clear column setup if not submitted by user. Currently we make it mandatory to call TableSetupColumn() every frame.
        // It would easily work without but we're not ready to guarantee it since e.g. names need resubmission anyway.
        // We take a slight shortcut but in theory we could be calling TableSetupColumn() here with dummy values, it should yield the same effect.
        if (table->DeclColumnsCount <= column_n)
        {
            TableSetupColumnFlags(table, column, RblxUITableColumnFlags_None);
            column->NameOffset = -1;
            column->UserID = 0;
            column->InitStretchWeightOrWidth = -1.0f;
        }

        // Update Enabled state, mark settings and sort specs dirty
        if (!(table->Flags & RblxUITableFlags_Hideable) || (column->Flags & RblxUITableColumnFlags_NoHide))
            column->IsUserEnabledNextFrame = true;
        if (column->IsUserEnabled != column->IsUserEnabledNextFrame)
        {
            column->IsUserEnabled = column->IsUserEnabledNextFrame;
            table->IsSettingsDirty = true;
        }
        column->IsEnabled = column->IsUserEnabled && (column->Flags & RblxUITableColumnFlags_Disabled) == 0;

        if (column->SortOrder != -1 && !column->IsEnabled)
            table->IsSortSpecsDirty = true;
        if (column->SortOrder > 0 && !(table->Flags & RblxUITableFlags_SortMulti))
            table->IsSortSpecsDirty = true;

        // Auto-fit unsized columns
        const bool start_auto_fit = (column->Flags & RblxUITableColumnFlags_WidthFixed) ? (column->WidthRequest < 0.0f) : (column->StretchWeight < 0.0f);
        if (start_auto_fit)
            column->AutoFitQueue = column->CannotSkipItemsQueue = (1 << 3) - 1; // Fit for three frames

        if (!column->IsEnabled)
        {
            column->IndexWithinEnabledSet = -1;
            continue;
        }

        // Mark as enabled and link to previous/next enabled column
        column->PrevEnabledColumn = (RblxUITableColumnIdx)prev_visible_column_idx;
        column->NextEnabledColumn = -1;
        if (prev_visible_column_idx != -1)
            table->Columns[prev_visible_column_idx].NextEnabledColumn = (RblxUITableColumnIdx)column_n;
        else
            table->LeftMostEnabledColumn = (RblxUITableColumnIdx)column_n;
        column->IndexWithinEnabledSet = table->ColumnsEnabledCount++;
        RblxBitArraySetBit(table->EnabledMaskByIndex, column_n);
        RblxBitArraySetBit(table->EnabledMaskByDisplayOrder, column->DisplayOrder);
        prev_visible_column_idx = column_n;
        RBLXUI_ASSERT(column->IndexWithinEnabledSet <= column->DisplayOrder);

        // Calculate ideal/auto column width (that's the width required for all contents to be visible without clipping)
        // Combine width from regular rows + width from headers unless requested not to.
        if (!column->IsPreserveWidthAuto && table->InstanceCurrent == 0)
            column->WidthAuto = TableGetColumnWidthAuto(table, column);

        // Non-resizable columns keep their requested width (apply user value regardless of IsPreserveWidthAuto)
        const bool column_is_resizable = (column->Flags & RblxUITableColumnFlags_NoResize) == 0;
        if (column_is_resizable)
            has_resizable = true;
        if ((column->Flags & RblxUITableColumnFlags_WidthFixed) && column->InitStretchWeightOrWidth > 0.0f && !column_is_resizable)
            column->WidthAuto = column->InitStretchWeightOrWidth;

        if (column->AutoFitQueue != 0x00)
            has_auto_fit_request = true;
        if (column->Flags & RblxUITableColumnFlags_WidthStretch)
        {
            stretch_sum_width_auto += column->WidthAuto;
            count_stretch++;
        }
        else
        {
            fixed_max_width_auto = RblxMax(fixed_max_width_auto, column->WidthAuto);
            count_fixed++;
        }
    }
    if ((table->Flags & RblxUITableFlags_Sortable) && table->SortSpecsCount == 0 && !(table->Flags & RblxUITableFlags_SortTristate))
        table->IsSortSpecsDirty = true;
    table->RightMostEnabledColumn = (RblxUITableColumnIdx)prev_visible_column_idx;
    RBLXUI_ASSERT(table->LeftMostEnabledColumn >= 0 && table->RightMostEnabledColumn >= 0);

    // [Part 2] Disable child window clipping while fitting columns. This is not strictly necessary but makes it possible to avoid
    // the column fitting having to wait until the first visible frame of the child container (may or not be a good thing). Also see #6510.
    // FIXME-TABLE: for always auto-resizing columns may not want to do that all the time.
    if (has_auto_fit_request && table->OuterWindow != table->InnerWindow)
        table->InnerWindow->SkipItems = false;
    if (has_auto_fit_request)
        table->IsSettingsDirty = true;

    // [Part 3] Fix column flags and record a few extra information.
    float sum_width_requests = 0.0f;    // Sum of all width for fixed and auto-resize columns, excluding width contributed by Stretch columns but including spacing/padding.
    float stretch_sum_weights = 0.0f;   // Sum of all weights for stretch columns.
    table->LeftMostStretchedColumn = table->RightMostStretchedColumn = -1;
    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
    {
        if (!RBLXUI_BITARRAY_TESTBIT(table->EnabledMaskByIndex, column_n))
            continue;
        RblxUITableColumn* column = &table->Columns[column_n];

        const bool column_is_resizable = (column->Flags & RblxUITableColumnFlags_NoResize) == 0;
        if (column->Flags & RblxUITableColumnFlags_WidthFixed)
        {
            // Apply same widths policy
            float width_auto = column->WidthAuto;
            if (table_sizing_policy == RblxUITableFlags_SizingFixedSame && (column->AutoFitQueue != 0x00 || !column_is_resizable))
                width_auto = fixed_max_width_auto;

            // Apply automatic width
            // Latch initial size for fixed columns and update it constantly for auto-resizing column (unless clipped!)
            if (column->AutoFitQueue != 0x00)
                column->WidthRequest = width_auto;
            else if ((column->Flags & RblxUITableColumnFlags_WidthFixed) && !column_is_resizable && column->IsRequestOutput)
                column->WidthRequest = width_auto;

            // FIXME-TABLE: Increase minimum size during init frame to avoid biasing auto-fitting widgets
            // (e.g. TextWrapped) too much. Otherwise what tends to happen is that TextWrapped would output a very
            // large height (= first frame scrollbar display very off + clipper would skip lots of items).
            // This is merely making the side-effect less extreme, but doesn't properly fixes it.
            // FIXME: Move this to ->WidthGiven to avoid temporary lossyness?
            // FIXME: This break IsPreserveWidthAuto from not flickering if the stored WidthAuto was smaller.
            if (column->AutoFitQueue > 0x01 && table->IsInitializing && !column->IsPreserveWidthAuto)
                column->WidthRequest = RblxMax(column->WidthRequest, table->MinColumnWidth * 4.0f); // FIXME-TABLE: Another constant/scale?
            sum_width_requests += column->WidthRequest;
        }
        else
        {
            // Initialize stretch weight
            if (column->AutoFitQueue != 0x00 || column->StretchWeight < 0.0f || !column_is_resizable)
            {
                if (column->InitStretchWeightOrWidth > 0.0f)
                    column->StretchWeight = column->InitStretchWeightOrWidth;
                else if (table_sizing_policy == RblxUITableFlags_SizingStretchProp)
                    column->StretchWeight = (column->WidthAuto / stretch_sum_width_auto) * count_stretch;
                else
                    column->StretchWeight = 1.0f;
            }

            stretch_sum_weights += column->StretchWeight;
            if (table->LeftMostStretchedColumn == -1 || table->Columns[table->LeftMostStretchedColumn].DisplayOrder > column->DisplayOrder)
                table->LeftMostStretchedColumn = (RblxUITableColumnIdx)column_n;
            if (table->RightMostStretchedColumn == -1 || table->Columns[table->RightMostStretchedColumn].DisplayOrder < column->DisplayOrder)
                table->RightMostStretchedColumn = (RblxUITableColumnIdx)column_n;
        }
        column->IsPreserveWidthAuto = false;
        sum_width_requests += table->CellPaddingX * 2.0f;
    }
    table->ColumnsEnabledFixedCount = (RblxUITableColumnIdx)count_fixed;
    table->ColumnsStretchSumWeights = stretch_sum_weights;

    // [Part 4] Apply final widths based on requested widths
    const RblxRect work_rect = table->WorkRect;
    const float width_spacings = (table->OuterPaddingX * 2.0f) + (table->CellSpacingX1 + table->CellSpacingX2) * (table->ColumnsEnabledCount - 1);
    const float width_removed = (table->HasScrollbarYPrev && !table->InnerWindow->ScrollbarY) ? g.Style.ScrollbarSize : 0.0f; // To synchronize decoration width of synced tables with mismatching scrollbar state (#5920)
    const float width_avail = RblxMax(1.0f, (((table->Flags & RblxUITableFlags_ScrollX) && table->InnerWidth == 0.0f) ? table->InnerClipRect.GetWidth() : work_rect.GetWidth()) - width_removed);
    const float width_avail_for_stretched_columns = width_avail - width_spacings - sum_width_requests;
    float width_remaining_for_stretched_columns = width_avail_for_stretched_columns;
    table->ColumnsGivenWidth = width_spacings + (table->CellPaddingX * 2.0f) * table->ColumnsEnabledCount;
    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
    {
        if (!RBLXUI_BITARRAY_TESTBIT(table->EnabledMaskByIndex, column_n))
            continue;
        RblxUITableColumn* column = &table->Columns[column_n];

        // Allocate width for stretched/weighted columns (StretchWeight gets converted into WidthRequest)
        if (column->Flags & RblxUITableColumnFlags_WidthStretch)
        {
            float weight_ratio = column->StretchWeight / stretch_sum_weights;
            column->WidthRequest = RBLXUI_TRUNC(RblxMax(width_avail_for_stretched_columns * weight_ratio, table->MinColumnWidth) + 0.01f);
            width_remaining_for_stretched_columns -= column->WidthRequest;
        }

        // [Resize Rule 1] The right-most Visible column is not resizable if there is at least one Stretch column
        // See additional comments in TableSetColumnWidth().
        if (column->NextEnabledColumn == -1 && table->LeftMostStretchedColumn != -1)
            column->Flags |= RblxUITableColumnFlags_NoDirectResize_;

        // Assign final width, record width in case we will need to shrink
        column->WidthGiven = RblxTrunc(RblxMax(column->WidthRequest, table->MinColumnWidth));
        table->ColumnsGivenWidth += column->WidthGiven;
    }

    // [Part 5] Redistribute stretch remainder width due to rounding (remainder width is < 1.0f * number of Stretch column).
    // Using right-to-left distribution (more likely to match resizing cursor).
    if (width_remaining_for_stretched_columns >= 1.0f && !(table->Flags & RblxUITableFlags_PreciseWidths))
        for (int order_n = table->ColumnsCount - 1; stretch_sum_weights > 0.0f && width_remaining_for_stretched_columns >= 1.0f && order_n >= 0; order_n--)
        {
            if (!RBLXUI_BITARRAY_TESTBIT(table->EnabledMaskByDisplayOrder, order_n))
                continue;
            RblxUITableColumn* column = &table->Columns[table->DisplayOrderToIndex[order_n]];
            if (!(column->Flags & RblxUITableColumnFlags_WidthStretch))
                continue;
            column->WidthRequest += 1.0f;
            column->WidthGiven += 1.0f;
            width_remaining_for_stretched_columns -= 1.0f;
        }

    // Determine if table is hovered which will be used to flag columns as hovered.
    // - In principle we'd like to use the equivalent of IsItemHovered(RblxUIHoveredFlags_AllowWhenBlockedByActiveItem),
    //   but because our item is partially submitted at this point we use ItemHoverable() and a workaround (temporarily
    //   clear ActiveId, which is equivalent to the change provided by _AllowWhenBLockedByActiveItem).
    // - This allows columns to be marked as hovered when e.g. clicking a button inside the column, or using drag and drop.
    RblxUITableInstanceData* table_instance = TableGetInstanceData(table, table->InstanceCurrent);
    table_instance->HoveredRowLast = table_instance->HoveredRowNext;
    table_instance->HoveredRowNext = -1;
    table->HoveredColumnBody = table->HoveredColumnBorder = -1;
    const RblxRect mouse_hit_rect(table->OuterRect.Min.x, table->OuterRect.Min.y, table->OuterRect.Max.x, RblxMax(table->OuterRect.Max.y, table->OuterRect.Min.y + table_instance->LastOuterHeight));
    const RblxUIID backup_active_id = g.ActiveId;
    g.ActiveId = 0;
    const bool is_hovering_table = ItemHoverable(mouse_hit_rect, 0, RblxUIItemFlags_None);
    g.ActiveId = backup_active_id;

    // Determine skewed MousePos.x to support angled headers.
    float mouse_skewed_x = g.IO.MousePos.x;
    if (table->AngledHeadersHeight > 0.0f)
        if (g.IO.MousePos.y >= table->OuterRect.Min.y && g.IO.MousePos.y <= table->OuterRect.Min.y + table->AngledHeadersHeight)
            mouse_skewed_x += RblxTrunc((table->OuterRect.Min.y + table->AngledHeadersHeight - g.IO.MousePos.y) * table->AngledHeadersSlope);

    // [Part 6] Setup final position, offset, skip/clip states and clipping rectangles, detect hovered column
    // Process columns in their visible orders as we are comparing the visible order and adjusting host_clip_rect while looping.
    bool has_at_least_one_column_requesting_output = false;
    bool offset_x_frozen = (table->FreezeColumnsCount > 0);
    float offset_x = ((table->FreezeColumnsCount > 0) ? table->OuterRect.Min.x : work_rect.Min.x) + table->OuterPaddingX - table->CellSpacingX1;
    RblxRect host_clip_rect = table->InnerClipRect;
    //host_clip_rect.Max.x += table->CellPaddingX + table->CellSpacingX2;
    RblxBitArrayClearAllBits(table->VisibleMaskByIndex, table->ColumnsCount);
    for (int order_n = 0; order_n < table->ColumnsCount; order_n++)
    {
        const int column_n = table->DisplayOrderToIndex[order_n];
        RblxUITableColumn* column = &table->Columns[column_n];

        // Initial nav layer: using FreezeRowsCount, NOT FreezeRowsRequest, so Header line changes layer when frozen
        column->NavLayerCurrent = (RblxS8)(table->FreezeRowsCount > 0 ? RblxUINavLayer_Menu : (RblxUINavLayer)table->NavLayer);

        if (offset_x_frozen && table->FreezeColumnsCount == order_n)
        {
            offset_x += work_rect.Min.x - table->OuterRect.Min.x;
            offset_x_frozen = false;
        }

        // Clear status flags
        column->Flags &= ~RblxUITableColumnFlags_StatusMask_;

        if (!RBLXUI_BITARRAY_TESTBIT(table->EnabledMaskByDisplayOrder, order_n))
        {
            // Hidden column: clear a few fields and we are done with it for the remainder of the function.
            // We set a zero-width clip rect but set Min.y/Max.y properly to not interfere with the clipper.
            column->MinX = column->MaxX = column->WorkMinX = column->ClipRect.Min.x = column->ClipRect.Max.x = offset_x;
            column->WidthGiven = 0.0f;
            column->ClipRect.Min.y = work_rect.Min.y;
            column->ClipRect.Max.y = FLT_MAX;
            column->ClipRect.ClipWithFull(host_clip_rect);
            column->IsVisibleX = column->IsVisibleY = column->IsRequestOutput = false;
            column->IsSkipItems = true;
            column->ItemWidth = 1.0f;
            continue;
        }

        // Lock start position
        column->MinX = offset_x;

        // Lock width based on start position and minimum/maximum width for this position
        column->WidthMax = TableCalcMaxColumnWidth(table, column_n);
        column->WidthGiven = RblxMin(column->WidthGiven, column->WidthMax);
        column->WidthGiven = RblxMax(column->WidthGiven, RblxMin(column->WidthRequest, table->MinColumnWidth));
        column->MaxX = offset_x + column->WidthGiven + table->CellSpacingX1 + table->CellSpacingX2 + table->CellPaddingX * 2.0f;

        // Lock other positions
        // - ClipRect.Min.x: Because merging draw commands doesn't compare min boundaries, we make ClipRect.Min.x match left bounds to be consistent regardless of merging.
        // - ClipRect.Max.x: using WorkMaxX instead of MaxX (aka including padding) makes things more consistent when resizing down, tho slightly detrimental to visibility in very-small column.
        // - ClipRect.Max.x: using MaxX makes it easier for header to receive hover highlight with no discontinuity and display sorting arrow.
        // - FIXME-TABLE: We want equal width columns to have equal (ClipRect.Max.x - WorkMinX) width, which means ClipRect.max.x cannot stray off host_clip_rect.Max.x else right-most column may appear shorter.
        const float previous_instance_work_min_x = column->WorkMinX;
        column->WorkMinX = column->MinX + table->CellPaddingX + table->CellSpacingX1;
        column->WorkMaxX = column->MaxX - table->CellPaddingX - table->CellSpacingX2; // Expected max
        column->ItemWidth = RblxTrunc(column->WidthGiven * 0.65f);
        column->ClipRect.Min.x = column->MinX;
        column->ClipRect.Min.y = work_rect.Min.y;
        column->ClipRect.Max.x = column->MaxX; //column->WorkMaxX;
        column->ClipRect.Max.y = FLT_MAX;
        column->ClipRect.ClipWithFull(host_clip_rect);

        // Mark column as Clipped (not in sight)
        // Note that scrolling tables (where inner_window != outer_window) handle Y clipped earlier in BeginTable() so IsVisibleY really only applies to non-scrolling tables.
        // FIXME-TABLE: Because InnerClipRect.Max.y is conservatively ==outer_window->ClipRect.Max.y, we never can mark columns _Above_ the scroll line as not IsVisibleY.
        // Taking advantage of LastOuterHeight would yield good results there...
        // FIXME-TABLE: Y clipping is disabled because it effectively means not submitting will reduce contents width which is fed to outer_window->DC.CursorMaxPos.x,
        // and this may be used (e.g. typically by outer_window using AlwaysAutoResize or outer_window's horizontal scrollbar, but could be something else).
        // Possible solution to preserve last known content width for clipped column. Test 'table_reported_size' fails when enabling Y clipping and window is resized small.
        column->IsVisibleX = (column->ClipRect.Max.x > column->ClipRect.Min.x);
        column->IsVisibleY = true; // (column->ClipRect.Max.y > column->ClipRect.Min.y);
        const bool is_visible = column->IsVisibleX; //&& column->IsVisibleY;
        if (is_visible)
            RblxBitArraySetBit(table->VisibleMaskByIndex, column_n);

        // Mark column as requesting output from user. Note that fixed + non-resizable sets are auto-fitting at all times and therefore always request output.
        column->IsRequestOutput = is_visible || column->AutoFitQueue != 0 || column->CannotSkipItemsQueue != 0;

        // Mark column as SkipItems (ignoring all items/layout)
        // (table->HostSkipItems is a copy of inner_window->SkipItems before we cleared it above in Part 2)
        column->IsSkipItems = !column->IsEnabled || table->HostSkipItems;
        if (column->IsSkipItems)
            RBLXUI_ASSERT(!is_visible);
        if (column->IsRequestOutput && !column->IsSkipItems)
            has_at_least_one_column_requesting_output = true;

        // Update status flags
        column->Flags |= RblxUITableColumnFlags_IsEnabled;
        if (is_visible)
            column->Flags |= RblxUITableColumnFlags_IsVisible;
        if (column->SortOrder != -1)
            column->Flags |= RblxUITableColumnFlags_IsSorted;

        // Detect hovered column
        if (is_hovering_table && mouse_skewed_x >= column->ClipRect.Min.x && mouse_skewed_x < column->ClipRect.Max.x)
        {
            column->Flags |= RblxUITableColumnFlags_IsHovered;
            table->HoveredColumnBody = (RblxUITableColumnIdx)column_n;
        }

        // Alignment
        // FIXME-TABLE: This align based on the whole column width, not per-cell, and therefore isn't useful in
        // many cases (to be able to honor this we might be able to store a log of cells width, per row, for
        // visible rows, but nav/programmatic scroll would have visible artifacts.)
        //if (column->Flags & RblxUITableColumnFlags_AlignRight)
        //    column->WorkMinX = RblxMax(column->WorkMinX, column->MaxX - column->ContentWidthRowsUnfrozen);
        //else if (column->Flags & RblxUITableColumnFlags_AlignCenter)
        //    column->WorkMinX = RblxLerp(column->WorkMinX, RblxMax(column->StartX, column->MaxX - column->ContentWidthRowsUnfrozen), 0.5f);

        // Reset content width variables
        if (table->InstanceCurrent == 0)
        {
            column->ContentMaxXFrozen = column->WorkMinX;
            column->ContentMaxXUnfrozen = column->WorkMinX;
            column->ContentMaxXHeadersUsed = column->WorkMinX;
            column->ContentMaxXHeadersIdeal = column->WorkMinX;
        }
        else
        {
            // As we store an absolute value to make per-cell updates faster, we need to offset values used for width computation.
            const float offset_from_previous_instance = column->WorkMinX - previous_instance_work_min_x;
            column->ContentMaxXFrozen += offset_from_previous_instance;
            column->ContentMaxXUnfrozen += offset_from_previous_instance;
            column->ContentMaxXHeadersUsed += offset_from_previous_instance;
            column->ContentMaxXHeadersIdeal += offset_from_previous_instance;
        }

        // Don't decrement auto-fit counters until container window got a chance to submit its items
        if (table->HostSkipItems == false && table->InstanceCurrent == 0)
        {
            column->AutoFitQueue >>= 1;
            column->CannotSkipItemsQueue >>= 1;
        }

        if (order_n < table->FreezeColumnsCount)
            host_clip_rect.Min.x = RblxClamp(column->MaxX + TABLE_BORDER_SIZE, host_clip_rect.Min.x, host_clip_rect.Max.x);

        offset_x += column->WidthGiven + table->CellSpacingX1 + table->CellSpacingX2 + table->CellPaddingX * 2.0f;
    }

    // In case the table is visible (e.g. decorations) but all columns clipped, we keep a column visible.
    // Else if give no chance to a clipper-savvy user to submit rows and therefore total contents height used by scrollbar.
    if (has_at_least_one_column_requesting_output == false)
    {
        table->Columns[table->LeftMostEnabledColumn].IsRequestOutput = true;
        table->Columns[table->LeftMostEnabledColumn].IsSkipItems = false;
    }

    // [Part 7] Detect/store when we are hovering the unused space after the right-most column (so e.g. context menus can react on it)
    // Clear Resizable flag if none of our column are actually resizable (either via an explicit _NoResize flag, either
    // because of using _WidthAuto/_WidthStretch). This will hide the resizing option from the context menu.
    const float unused_x1 = RblxMax(table->WorkRect.Min.x, table->Columns[table->RightMostEnabledColumn].ClipRect.Max.x);
    if (is_hovering_table && table->HoveredColumnBody == -1)
        if (mouse_skewed_x >= unused_x1)
            table->HoveredColumnBody = (RblxUITableColumnIdx)table->ColumnsCount;
    if (has_resizable == false && (table->Flags & RblxUITableFlags_Resizable))
        table->Flags &= ~RblxUITableFlags_Resizable;

    table->IsActiveIdAliveBeforeTable = (g.ActiveIdIsAlive != 0);

    // [Part 8] Lock actual OuterRect/WorkRect right-most position.
    // This is done late to handle the case of fixed-columns tables not claiming more widths that they need.
    // Because of this we are careful with uses of WorkRect and InnerClipRect before this point.
    if (table->RightMostStretchedColumn != -1)
        table->Flags &= ~RblxUITableFlags_NoHostExtendX;
    if (table->Flags & RblxUITableFlags_NoHostExtendX)
    {
        table->OuterRect.Max.x = table->WorkRect.Max.x = unused_x1;
        table->InnerClipRect.Max.x = RblxMin(table->InnerClipRect.Max.x, unused_x1);
    }
    table->InnerWindow->ParentWorkRect = table->WorkRect;
    table->BorderX1 = table->InnerClipRect.Min.x;
    table->BorderX2 = table->InnerClipRect.Max.x;

    // Setup window's WorkRect.Max.y for GetContentRegionAvail(). Other values will be updated in each TableBeginCell() call.
    float window_content_max_y;
    if (table->Flags & RblxUITableFlags_NoHostExtendY)
        window_content_max_y = table->OuterRect.Max.y;
    else
        window_content_max_y = RblxMax(table->InnerWindow->ContentRegionRect.Max.y, (table->Flags & RblxUITableFlags_ScrollY) ? 0.0f : table->OuterRect.Max.y);
    table->InnerWindow->WorkRect.Max.y = RblxClamp(window_content_max_y - g.Style.CellPadding.y, table->InnerWindow->WorkRect.Min.y, table->InnerWindow->WorkRect.Max.y);

    // [Part 9] Allocate draw channels and setup background cliprect
    TableSetupDrawChannels(table);

    // [Part 10] Hit testing on borders
    if (table->Flags & RblxUITableFlags_Resizable)
        TableUpdateBorders(table);
    table_instance->LastTopHeadersRowHeight = 0.0f;
    table->IsLayoutLocked = true;
    table->IsUsingHeaders = false;

    // Highlight header
    table->HighlightColumnHeader = -1;
    if (table->IsContextPopupOpen && table->ContextPopupColumn != -1 && table->InstanceInteracted == table->InstanceCurrent)
        table->HighlightColumnHeader = table->ContextPopupColumn;
    else if ((table->Flags & RblxUITableFlags_HighlightHoveredColumn) && table->HoveredColumnBody != -1 && table->HoveredColumnBody != table->ColumnsCount && table->HoveredColumnBorder == -1)
        if (g.ActiveId == 0 || (table->IsActiveIdInTable || g.DragDropActive))
            table->HighlightColumnHeader = table->HoveredColumnBody;

    // [Part 11] Default context menu
    // - To append to this menu: you can call TableBeginContextMenuPopup()/.../EndPopup().
    // - To modify or replace this: set table->DisableDefaultContextMenu = true, then call TableBeginContextMenuPopup()/.../EndPopup().
    // - You may call TableDrawDefaultContextMenu() with selected flags to display specific sections of the default menu,
    //   e.g. TableDrawDefaultContextMenu(table, table->Flags & ~RblxUITableFlags_Hideable) will display everything EXCEPT columns visibility options.
    if (table->DisableDefaultContextMenu == false && TableBeginContextMenuPopup(table))
    {
        TableDrawDefaultContextMenu(table, table->Flags);
        EndPopup();
    }

    // [Part 12] Sanitize and build sort specs before we have a chance to use them for display.
    // This path will only be exercised when sort specs are modified before header rows (e.g. init or visibility change)
    if (table->IsSortSpecsDirty && (table->Flags & RblxUITableFlags_Sortable))
        TableSortSpecsBuild(table);

    // [Part 13] Setup inner window decoration size (for scrolling / nav tracking to properly take account of frozen rows/columns)
    if (table->FreezeColumnsRequest > 0)
        table->InnerWindow->DecoInnerSizeX1 = table->Columns[table->DisplayOrderToIndex[table->FreezeColumnsRequest - 1]].MaxX - table->OuterRect.Min.x; // FIXME-FROZEN
    if (table->FreezeRowsRequest > 0)
        table->InnerWindow->DecoInnerSizeY1 = table_instance->LastFrozenHeight;
    table_instance->LastFrozenHeight = 0.0f;

    RblxUIWindow* inner_window = table->InnerWindow;
    RblxUIBoxSelectState* bs = &g.BoxSelectState;
    if (bs->Window == inner_window && bs->UnclipMode)
        TableApplyExternalUnclipRect(table, bs->UnclipRect);

    // Initial state
    if (table->Flags & RblxUITableFlags_NoClip)
        table->DrawSplitter->SetCurrentChannel(inner_window->DrawList, TABLE_DRAW_CHANNEL_NOCLIP);
    else
        inner_window->DrawList->PushClipRect(inner_window->InnerClipRect.Min, inner_window->InnerClipRect.Max, false); // FIXME: use table->InnerClipRect?
}

// When starting a BeginMultiSelect() after table has been layout we update IsRequestOutput fields.
void RblxUI::TableApplyExternalUnclipRect(RblxUITable* table, RblxRect& rect)
{
    if (rect.IsInverted())
        return;
    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
    {
        RblxUITableColumn* column = &table->Columns[column_n];
        if (!column->IsRequestOutput)
            if (rect.Overlaps(RblxRect(column->MinX, table->WorkRect.Min.y, column->MaxX, FLT_MAX)))
                column->IsRequestOutput = true;
    }
}

// Process hit-testing on resizing borders. Actual size change will be applied in EndTable()
// - Set table->HoveredColumnBorder with a short delay/timer to reduce visual feedback noise.
void RblxUI::TableUpdateBorders(RblxUITable* table)
{
    RblxUIContext& g = *GRblxUI;
    RBLXUI_ASSERT(table->Flags & RblxUITableFlags_Resizable);

    // At this point OuterRect height may be zero or under actual final height, so we rely on temporal coherency and
    // use the final height from last frame. Because this is only affecting _interaction_ with columns, it is not
    // really problematic (whereas the actual visual will be displayed in EndTable() and using the current frame height).
    // Actual columns highlight/render will be performed in EndTable() and not be affected.
    RblxUITableInstanceData* table_instance = TableGetInstanceData(table, table->InstanceCurrent);
    const float hit_half_width = RblxTrunc(TABLE_RESIZE_SEPARATOR_HALF_THICKNESS * g.CurrentDpiScale);
    const float hit_y1 = (table->FreezeRowsCount >= 1 ? table->OuterRect.Min.y : table->WorkRect.Min.y) + table->AngledHeadersHeight;
    const float hit_y2_body = RblxMax(table->OuterRect.Max.y, hit_y1 + table_instance->LastOuterHeight - table->AngledHeadersHeight);
    const float hit_y2_head = hit_y1 + table_instance->LastTopHeadersRowHeight;

    for (int order_n = 0; order_n < table->ColumnsCount; order_n++)
    {
        if (!RBLXUI_BITARRAY_TESTBIT(table->EnabledMaskByDisplayOrder, order_n))
            continue;

        const int column_n = table->DisplayOrderToIndex[order_n];
        RblxUITableColumn* column = &table->Columns[column_n];
        if (column->Flags & (RblxUITableColumnFlags_NoResize | RblxUITableColumnFlags_NoDirectResize_))
            continue;

        // RblxUITableFlags_NoBordersInBodyUntilResize will be honored in TableDrawBorders()
        const float border_y2_hit = (table->Flags & RblxUITableFlags_NoBordersInBody) ? hit_y2_head : hit_y2_body;
        if ((table->Flags & RblxUITableFlags_NoBordersInBody) && table->IsUsingHeaders == false)
            continue;

        if (!column->IsVisibleX && table->LastResizedColumn != column_n)
            continue;

        RblxUIID column_id = TableGetColumnResizeID(table, column_n, table->InstanceCurrent);
        RblxRect hit_rect(column->MaxX - hit_half_width, hit_y1, column->MaxX + hit_half_width, border_y2_hit);
        ItemAdd(hit_rect, column_id, NULL, RblxUIItemFlags_NoNav);
        //GetForegroundDrawList()->AddRect(hit_rect.Min, hit_rect.Max, RBLXUI_COL32(255, 0, 0, 100));

        bool hovered = false, held = false;
        bool pressed = ButtonBehavior(hit_rect, column_id, &hovered, &held, RblxUIButtonFlags_FlattenChildren | RblxUIButtonFlags_PressedOnClick | RblxUIButtonFlags_PressedOnDoubleClick | RblxUIButtonFlags_NoNavFocus);
        if (pressed && IsMouseDoubleClicked(0))
        {
            TableSetColumnWidthAutoSingle(table, column_n);
            ClearActiveID();
            held = false;
        }
        if (held)
        {
            if (table->LastResizedColumn == -1)
                table->ResizeLockMinContentsX2 = table->RightMostEnabledColumn != -1 ? table->Columns[table->RightMostEnabledColumn].MaxX : -FLT_MAX;
            table->ResizedColumn = (RblxUITableColumnIdx)column_n;
            table->InstanceInteracted = table->InstanceCurrent;
        }
        if ((hovered && g.HoveredIdTimer > TABLE_RESIZE_SEPARATOR_FEEDBACK_TIMER) || held)
        {
            table->HoveredColumnBorder = (RblxUITableColumnIdx)column_n;
            SetMouseCursor(RblxUIMouseCursor_ResizeEW);
        }
    }
}

void    RblxUI::EndTable()
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    RBLXUI_ASSERT_USER_ERROR_RET(table != NULL, "EndTable() call should only be done while in BeginTable() scope!");

    // This assert would be very useful to catch a common error... unfortunately it would probably trigger in some
    // cases, and for consistency user may sometimes output empty tables (and still benefit from e.g. outer border)
    //RBLXUI_ASSERT(table->IsLayoutLocked && "Table unused: never called TableNextRow(), is that the intent?");

    // If the user never got to call TableNextRow() or TableNextColumn(), we call layout ourselves to ensure all our
    // code paths are consistent (instead of just hoping that TableBegin/TableEnd will work), get borders drawn, etc.
    if (!table->IsLayoutLocked)
        TableUpdateLayout(table);

    const RblxUITableFlags flags = table->Flags;
    RblxUIWindow* inner_window = table->InnerWindow;
    RblxUIWindow* outer_window = table->OuterWindow;
    RblxUITableTempData* temp_data = table->TempData;
    RBLXUI_ASSERT(inner_window == g.CurrentWindow && inner_window->ID == temp_data->WindowID);
    RBLXUI_ASSERT(outer_window == inner_window || outer_window == inner_window->ParentWindow);

    if (table->IsInsideRow)
        TableEndRow(table);

    // Context menu in columns body
    if (flags & RblxUITableFlags_ContextMenuInBody)
        if (table->HoveredColumnBody != -1 && !IsAnyItemHovered() && IsMouseReleased(RblxUIMouseButton_Right))
            TableOpenContextMenu((int)table->HoveredColumnBody);

    // Finalize table height
    RblxUITableInstanceData* table_instance = TableGetInstanceData(table, table->InstanceCurrent);
    inner_window->DC.PrevLineSize = temp_data->HostBackupPrevLineSize;
    inner_window->DC.CurrLineSize = temp_data->HostBackupCurrLineSize;
    inner_window->DC.CursorMaxPos = temp_data->HostBackupCursorMaxPos;
    const float inner_content_max_y = RblxCeil(table->RowPosY2); // Rounding final position is important as we currently don't round row height ('Demo->Tables->Outer Size' demo uses non-integer heights)
    RBLXUI_ASSERT(table->RowPosY2 == inner_window->DC.CursorPos.y);
    if (inner_window != outer_window)
        inner_window->DC.CursorMaxPos.y = inner_content_max_y;
    else if (!(flags & RblxUITableFlags_NoHostExtendY))
        table->OuterRect.Max.y = table->InnerRect.Max.y = RblxMax(table->OuterRect.Max.y, inner_content_max_y); // Patch OuterRect/InnerRect height
    table->WorkRect.Max.y = RblxMax(table->WorkRect.Max.y, table->OuterRect.Max.y);
    table_instance->LastOuterHeight = table->OuterRect.GetHeight();

    // Setup inner scrolling range
    // FIXME: This ideally should be done earlier, in BeginTable() SetNextWindowContentSize call, just like writing to inner_window->DC.CursorMaxPos.y,
    // but since the later is likely to be impossible to do we'd rather update both axes together.
    if (table->Flags & RblxUITableFlags_ScrollX)
    {
        const float outer_padding_for_border = (table->Flags & RblxUITableFlags_BordersOuterV) ? TABLE_BORDER_SIZE : 0.0f;
        float max_pos_x = inner_window->DC.CursorMaxPos.x;
        if (table->RightMostEnabledColumn != -1)
            max_pos_x = RblxMax(max_pos_x, table->Columns[table->RightMostEnabledColumn].WorkMaxX + table->CellPaddingX + table->OuterPaddingX - outer_padding_for_border);
        if (table->ResizedColumn != -1)
            max_pos_x = RblxMax(max_pos_x, table->ResizeLockMinContentsX2);
        inner_window->DC.CursorMaxPos.x = max_pos_x + table->TempData->AngledHeadersExtraWidth;
    }

    // Pop clipping rect
    if (!(flags & RblxUITableFlags_NoClip))
        inner_window->DrawList->PopClipRect();
    inner_window->ClipRect = inner_window->DrawList->_ClipRectStack.back();

    // Draw borders
    if ((flags & RblxUITableFlags_Borders) != 0)
        TableDrawBorders(table);

#if 0
    // Strip out dummy channel draw calls
    // We have no way to prevent user submitting direct RblxDrawList calls into a hidden column (but RblxUI:: calls will be clipped out)
    // Pros: remove draw calls which will have no effect. since they'll have zero-size cliprect they may be early out anyway.
    // Cons: making it harder for users watching metrics/debugger to spot the wasted vertices.
    if (table->DummyDrawChannel != (RblxUITableColumnIdx)-1)
    {
        RblxDrawChannel* dummy_channel = &table->DrawSplitter._Channels[table->DummyDrawChannel];
        dummy_channel->_CmdBuffer.resize(0);
        dummy_channel->_IdxBuffer.resize(0);
    }
#endif

    // Flatten channels and merge draw calls
    RblxDrawListSplitter* splitter = table->DrawSplitter;
    splitter->SetCurrentChannel(inner_window->DrawList, 0);
    if ((table->Flags & RblxUITableFlags_NoClip) == 0)
        TableMergeDrawChannels(table);
    splitter->Merge(inner_window->DrawList);

    // Update ColumnsAutoFitWidth to get us ahead for host using our size to auto-resize without waiting for next BeginTable()
    float auto_fit_width_for_fixed = 0.0f;
    float auto_fit_width_for_stretched = 0.0f;
    float auto_fit_width_for_stretched_min = 0.0f;
    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
        if (RBLXUI_BITARRAY_TESTBIT(table->EnabledMaskByIndex, column_n))
        {
            RblxUITableColumn* column = &table->Columns[column_n];
            float column_width_request = ((column->Flags & RblxUITableColumnFlags_WidthFixed) && !(column->Flags & RblxUITableColumnFlags_NoResize)) ? column->WidthRequest : TableGetColumnWidthAuto(table, column);
            if (column->Flags & RblxUITableColumnFlags_WidthFixed)
                auto_fit_width_for_fixed += column_width_request;
            else
                auto_fit_width_for_stretched += column_width_request;
            if ((column->Flags & RblxUITableColumnFlags_WidthStretch) && (column->Flags & RblxUITableColumnFlags_NoResize) != 0)
                auto_fit_width_for_stretched_min = RblxMax(auto_fit_width_for_stretched_min, column_width_request / (column->StretchWeight / table->ColumnsStretchSumWeights));
        }
    const float width_spacings = (table->OuterPaddingX * 2.0f) + (table->CellSpacingX1 + table->CellSpacingX2) * (table->ColumnsEnabledCount - 1);
    table->ColumnsAutoFitWidth = width_spacings + (table->CellPaddingX * 2.0f) * table->ColumnsEnabledCount + auto_fit_width_for_fixed + RblxMax(auto_fit_width_for_stretched, auto_fit_width_for_stretched_min);

    // Update scroll
    if ((table->Flags & RblxUITableFlags_ScrollX) == 0 && inner_window != outer_window)
    {
        inner_window->Scroll.x = 0.0f;
    }
    else if (table->LastResizedColumn != -1 && table->ResizedColumn == -1 && inner_window->ScrollbarX && table->InstanceInteracted == table->InstanceCurrent)
    {
        // When releasing a column being resized, scroll to keep the resulting column in sight
        const float neighbor_width_to_keep_visible = table->MinColumnWidth + table->CellPaddingX * 2.0f;
        RblxUITableColumn* column = &table->Columns[table->LastResizedColumn];
        if (column->MaxX < table->InnerClipRect.Min.x)
            SetScrollFromPosX(inner_window, column->MaxX - inner_window->Pos.x - neighbor_width_to_keep_visible, 1.0f);
        else if (column->MaxX > table->InnerClipRect.Max.x)
            SetScrollFromPosX(inner_window, column->MaxX - inner_window->Pos.x + neighbor_width_to_keep_visible, 1.0f);
    }

    // Apply resizing/dragging at the end of the frame
    if (table->ResizedColumn != -1 && table->InstanceCurrent == table->InstanceInteracted)
    {
        RblxUITableColumn* column = &table->Columns[table->ResizedColumn];
        const float new_x2 = (g.IO.MousePos.x - g.ActiveIdClickOffset.x + RblxTrunc(TABLE_RESIZE_SEPARATOR_HALF_THICKNESS * g.CurrentDpiScale));
        const float new_width = RblxTrunc(new_x2 - column->MinX - table->CellSpacingX1 - table->CellPaddingX * 2.0f);
        table->ResizedColumnNextWidth = new_width;
    }

    table->IsActiveIdInTable = (g.ActiveIdIsAlive != 0 && table->IsActiveIdAliveBeforeTable == false);

    // Pop from id stack
    RBLXUI_ASSERT_USER_ERROR(inner_window->IDStack.back() == table_instance->TableInstanceID, "Mismatching PushID/PopID!");
    RBLXUI_ASSERT_USER_ERROR(outer_window->DC.ItemWidthStack.Size >= temp_data->HostBackupItemWidthStackSize, "Too many PopItemWidth!");
    if (table->InstanceCurrent > 0)
        PopID();
    PopID();

    // Restore window data that we modified
    const RblxVec2 backup_outer_max_pos = outer_window->DC.CursorMaxPos;
    inner_window->WorkRect = temp_data->HostBackupWorkRect;
    inner_window->ParentWorkRect = temp_data->HostBackupParentWorkRect;
    inner_window->SkipItems = table->HostSkipItems;
    outer_window->DC.CursorPos = table->OuterRect.Min;
    outer_window->DC.ItemWidth = temp_data->HostBackupItemWidth;
    outer_window->DC.ItemWidthStack.Size = temp_data->HostBackupItemWidthStackSize;
    outer_window->DC.ColumnsOffset = temp_data->HostBackupColumnsOffset;

    // Layout in outer window
    // (FIXME: To allow auto-fit and allow desirable effect of SameLine() we dissociate 'used' vs 'ideal' size by overriding
    // CursorPosPrevLine and CursorMaxPos manually. That should be a more general layout feature, see same problem e.g. #3414)
    if (inner_window != outer_window)
    {
        short backup_nav_layers_active_mask = inner_window->DC.NavLayersActiveMask;
        inner_window->DC.NavLayersActiveMask |= 1 << table->NavLayer; // So empty table don't appear to navigate differently.
        g.CurrentTable = NULL; // To avoid error recovery recursing
        EndChild();
        g.CurrentTable = table;
        inner_window->DC.NavLayersActiveMask = backup_nav_layers_active_mask;
    }
    else
    {
        inner_window->DC.TreeDepth--;
        ItemSize(table->OuterRect.GetSize());
        ItemAdd(table->OuterRect, 0);
    }

    // Override declared contents width/height to enable auto-resize while not needlessly adding a scrollbar
    if (table->Flags & RblxUITableFlags_NoHostExtendX)
    {
        // FIXME-TABLE: Could we remove this section?
        // ColumnsAutoFitWidth may be one frame ahead here since for Fixed+NoResize is calculated from latest contents
        RBLXUI_ASSERT((table->Flags & RblxUITableFlags_ScrollX) == 0);
        outer_window->DC.CursorMaxPos.x = RblxMax(backup_outer_max_pos.x, table->OuterRect.Min.x + table->ColumnsAutoFitWidth);
    }
    else if (temp_data->UserOuterSize.x <= 0.0f)
    {
        // Some references for this: #7651 + tests "table_reported_size", "table_reported_size_outer" equivalent Y block, #9352
        // - FIXME-TABLE: Would make sense to pre-compute expected scrollbar visibility/sizes to generally save a frame of feedback? See broken test in 'table_reported_size_outer'
        const float outer_content_max_x = table->OuterRect.Min.x + table->ColumnsAutoFitWidth;
        const float decoration_size = table->TempData->AngledHeadersExtraWidth + ((inner_window != outer_window) ? inner_window->ScrollbarSizes.x : 0.0f);
        outer_window->DC.IdealMaxPos.x = RblxMax(outer_window->DC.IdealMaxPos.x, outer_content_max_x + decoration_size - temp_data->UserOuterSize.x);
        outer_window->DC.CursorMaxPos.x = RblxMax(backup_outer_max_pos.x, RblxMin(table->OuterRect.Max.x, outer_content_max_x + decoration_size));
    }
    else
    {
        outer_window->DC.CursorMaxPos.x = RblxMax(backup_outer_max_pos.x, table->OuterRect.Max.x);
    }
    if (temp_data->UserOuterSize.y <= 0.0f)
    {
        // (same comment as above)
        const float outer_content_size_y = (inner_window == outer_window) ? (inner_content_max_y - table->InnerRect.Min.y) : (inner_content_max_y - inner_window->DC.CursorStartPos.y);
        const float outer_content_max_y = table->OuterRect.Min.y + outer_content_size_y;
        const float decoration_size = (inner_window != outer_window ? inner_window->ScrollbarSizes.y : 0.0f);
        outer_window->DC.IdealMaxPos.y = RblxMax(outer_window->DC.IdealMaxPos.y, outer_content_max_y + decoration_size - temp_data->UserOuterSize.y);
        outer_window->DC.CursorMaxPos.y = RblxMax(backup_outer_max_pos.y, RblxMin(table->OuterRect.Max.y, outer_content_max_y + decoration_size));
    }
    else
    {
        // OuterRect.Max.y may already have been pushed downward from the initial value (unless RblxUITableFlags_NoHostExtendY is set)
        outer_window->DC.CursorMaxPos.y = RblxMax(backup_outer_max_pos.y, table->OuterRect.Max.y);
    }

    // Save settings
    if (table->IsSettingsDirty)
        TableSaveSettings(table);
    table->IsInitializing = false;

    // Clear or restore current table, if any
    RBLXUI_ASSERT(g.CurrentWindow == outer_window && g.CurrentTable == table);
    RBLXUI_ASSERT(g.TablesTempDataStacked > 0);
    temp_data = (--g.TablesTempDataStacked > 0) ? &g.TablesTempData[g.TablesTempDataStacked - 1] : NULL;
    g.CurrentTable = temp_data && (temp_data->WindowID == outer_window->ID) ? g.Tables.GetByIndex(temp_data->TableIndex) : NULL;
    if (g.CurrentTable)
    {
        g.CurrentTable->TempData = temp_data;
        g.CurrentTable->DrawSplitter = &temp_data->DrawSplitter;
    }
    outer_window->DC.CurrentTableIdx = g.CurrentTable ? g.Tables.GetIndex(g.CurrentTable) : -1;
    NavUpdateCurrentWindowIsScrollPushableX();
}

// Called in TableSetupColumn() when initializing and in TableLoadSettings() for defaults before applying stored settings.
// 'init_mask' specify which fields to initialize.
static void TableInitColumnDefaults(RblxUITable* table, RblxUITableColumn* column, RblxUITableColumnFlags init_mask)
{
    RblxUITableColumnFlags flags = column->Flags;
    if (init_mask & RblxUITableFlags_Resizable)
    {
        float init_width_or_weight = column->InitStretchWeightOrWidth;
        column->WidthRequest = ((flags & RblxUITableColumnFlags_WidthFixed) && init_width_or_weight > 0.0f) ? init_width_or_weight : -1.0f;
        column->StretchWeight = (init_width_or_weight > 0.0f && (flags & RblxUITableColumnFlags_WidthStretch)) ? init_width_or_weight : -1.0f;
        if (init_width_or_weight > 0.0f) // Disable auto-fit if an explicit width/weight has been specified
            column->AutoFitQueue = 0x00;
    }
    if (init_mask & RblxUITableFlags_Reorderable)
        column->DisplayOrder = (RblxUITableColumnIdx)table->Columns.index_from_ptr(column);
    if (init_mask & RblxUITableFlags_Hideable)
        column->IsUserEnabled = column->IsUserEnabledNextFrame = (flags & RblxUITableColumnFlags_DefaultHide) ? 0 : 1;
    if (init_mask & RblxUITableFlags_Sortable)
    {
        // Multiple columns using _DefaultSort will be reassigned unique SortOrder values when building the sort specs.
        column->SortOrder = (flags & RblxUITableColumnFlags_DefaultSort) ? 0 : -1;
        column->SortDirection = (flags & RblxUITableColumnFlags_DefaultSort) ? ((flags & RblxUITableColumnFlags_PreferSortDescending) ? (RblxS8)RblxUISortDirection_Descending : (RblxU8)(RblxUISortDirection_Ascending)) : (RblxS8)RblxUISortDirection_None;
    }
}

// See "COLUMNS SIZING POLICIES" comments at the top of this file
// If (init_width_or_weight <= 0.0f) it is ignored
void RblxUI::TableSetupColumn(const char* label, RblxUITableColumnFlags flags, float init_width_or_weight, RblxUIID user_id)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    RBLXUI_ASSERT_USER_ERROR_RET(table != NULL, "Call should only be done while in BeginTable() scope!");
    RBLXUI_ASSERT_USER_ERROR_RET(table->DeclColumnsCount < table->ColumnsCount, "TableSetupColumn(): called too many times!");
    RBLXUI_ASSERT_USER_ERROR_RET(table->IsLayoutLocked == false, "TableSetupColumn(): need to call before first row!"); // Table layout is locked when submitting a row or when calling BeginMultiSelect() with box-select.
    RBLXUI_ASSERT((flags & RblxUITableColumnFlags_StatusMask_) == 0 && "Illegal to pass StatusMask values to TableSetupColumn()");

    RblxUITableColumn* column = &table->Columns[table->DeclColumnsCount];
    table->DeclColumnsCount++;

    // Assert when passing a width or weight if policy is entirely left to default, to avoid storing width into weight and vice-versa.
    // Give a grace to users of RblxUITableFlags_ScrollX.
    if (table->IsDefaultSizingPolicy && (flags & RblxUITableColumnFlags_WidthMask_) == 0 && (flags & RblxUITableFlags_ScrollX) == 0)
        RBLXUI_ASSERT_USER_ERROR_RET(init_width_or_weight <= 0.0f, "TableSetupColumn(): can only specify width/weight if sizing policy is set explicitly in either Table or Column.");

    // When passing a width automatically enforce WidthFixed policy
    // (whereas TableSetupColumnFlags would default to WidthAuto if table is not resizable)
    if ((flags & RblxUITableColumnFlags_WidthMask_) == 0 && init_width_or_weight > 0.0f)
        if ((table->Flags & RblxUITableFlags_SizingMask_) == RblxUITableFlags_SizingFixedFit || (table->Flags & RblxUITableFlags_SizingMask_) == RblxUITableFlags_SizingFixedSame)
            flags |= RblxUITableColumnFlags_WidthFixed;
    if (flags & RblxUITableColumnFlags_AngledHeader)
    {
        flags |= RblxUITableColumnFlags_NoHeaderLabel;
        table->AngledHeadersCount++;
    }

    TableSetupColumnFlags(table, column, flags);
    column->UserID = user_id;
    flags = column->Flags;

    // Initialize defaults
    column->InitStretchWeightOrWidth = init_width_or_weight;
    if (table->IsInitializing)
    {
        RblxUITableFlags init_flags = ~table->SettingsLoadedFlags;
        if (column->WidthRequest < 0.0f && column->StretchWeight < 0.0f)
            init_flags |= RblxUITableFlags_Resizable;
        TableInitColumnDefaults(table, column, init_flags);
    }

    // Store name (append with zero-terminator in contiguous buffer)
    // FIXME: If we recorded the number of \n in names we could compute header row height
    column->NameOffset = -1;
    if (label != NULL && label[0] != 0)
    {
        column->NameOffset = (RblxS16)table->ColumnsNames.size();
        table->ColumnsNames.append(label, label + RblxStrlen(label) + 1);
    }
}

// [Public]
void RblxUI::TableSetupScrollFreeze(int columns, int rows)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    RBLXUI_ASSERT_USER_ERROR_RET(table != NULL, "Call should only be done while in BeginTable() scope!");
    RBLXUI_ASSERT(table->IsLayoutLocked == false && "TableSetupColumn(): need to call before first row!");
    RBLXUI_ASSERT(columns >= 0 && columns < RBLXUI_TABLE_MAX_COLUMNS);
    RBLXUI_ASSERT(rows >= 0 && rows < 128); // Arbitrary limit

    table->FreezeColumnsRequest = (table->Flags & RblxUITableFlags_ScrollX) ? (RblxUITableColumnIdx)RblxMin(columns, table->ColumnsCount) : 0;
    table->FreezeColumnsCount = (table->InnerWindow->Scroll.x != 0.0f) ? table->FreezeColumnsRequest : 0;
    table->FreezeRowsRequest = (table->Flags & RblxUITableFlags_ScrollY) ? (RblxUITableColumnIdx)rows : 0;
    table->FreezeRowsCount = (table->InnerWindow->Scroll.y != 0.0f) ? table->FreezeRowsRequest : 0;
    table->IsUnfrozenRows = (table->FreezeRowsCount == 0); // Make sure this is set before TableUpdateLayout() so RblxUIListClipper can benefit from it.b
}

//-----------------------------------------------------------------------------
// [SECTION] Tables: Simple accessors
//-----------------------------------------------------------------------------
// - TableGetColumnCount()
// - TableGetColumnName()
// - TableGetColumnName() [Internal]
// - TableSetColumnEnabled()
// - TableGetColumnFlags()
// - TableGetCellBgRect() [Internal]
// - TableGetColumnResizeID() [Internal]
// - TableGetHoveredColumn() [Internal]
// - TableGetHoveredRow() [Internal]
// - TableSetBgColor()
//-----------------------------------------------------------------------------

int RblxUI::TableGetColumnCount()
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    return table ? table->ColumnsCount : 0;
}

const char* RblxUI::TableGetColumnName(int column_n)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    if (!table)
        return NULL;
    if (column_n < 0)
        column_n = table->CurrentColumn;
    return TableGetColumnName(table, column_n);
}

const char* RblxUI::TableGetColumnName(const RblxUITable* table, int column_n)
{
    if (table->IsLayoutLocked == false && column_n >= table->DeclColumnsCount)
        return ""; // NameOffset is invalid at this point
    const RblxUITableColumn* column = &table->Columns[column_n];
    if (column->NameOffset == -1)
        return "";
    return &table->ColumnsNames.Buf[column->NameOffset];
}

// Change user accessible enabled/disabled state of a column (often perceived as "showing/hiding" from users point of view)
// Note that end-user can use the context menu to change this themselves (right-click in headers, or right-click in columns body with RblxUITableFlags_ContextMenuInBody)
// - Require table to have the RblxUITableFlags_Hideable flag because we are manipulating user accessible state.
// - Request will be applied during next layout, which happens on the first call to TableNextRow() after BeginTable().
// - For the getter you can test (TableGetColumnFlags() & RblxUITableColumnFlags_IsEnabled) != 0.
// - Alternative: the RblxUITableColumnFlags_Disabled is an overriding/master disable flag which will also hide the column from context menu.
void RblxUI::TableSetColumnEnabled(int column_n, bool enabled)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    RBLXUI_ASSERT_USER_ERROR_RET(table != NULL, "Call should only be done while in BeginTable() scope!");
    RBLXUI_ASSERT(table->Flags & RblxUITableFlags_Hideable); // See comments above
    if (column_n < 0)
        column_n = table->CurrentColumn;
    RBLXUI_ASSERT(column_n >= 0 && column_n < table->ColumnsCount);
    RblxUITableColumn* column = &table->Columns[column_n];
    column->IsUserEnabledNextFrame = enabled;
}

// We allow querying for an extra column in order to poll the IsHovered state of the right-most section
RblxUITableColumnFlags RblxUI::TableGetColumnFlags(int column_n)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    if (!table)
        return RblxUITableColumnFlags_None;
    if (column_n < 0)
        column_n = table->CurrentColumn;
    if (column_n == table->ColumnsCount)
        return (table->HoveredColumnBody == column_n) ? RblxUITableColumnFlags_IsHovered : RblxUITableColumnFlags_None;
    return table->Columns[column_n].Flags;
}

// Return the cell rectangle based on currently known height.
// - Important: we generally don't know our row height until the end of the row, so Max.y will be incorrect in many situations.
//   The only case where this is correct is if we provided a min_row_height to TableNextRow() and don't go below it, or in TableEndRow() when we locked that height.
// - Important: if RblxUITableFlags_PadOuterX is set but RblxUITableFlags_PadInnerX is not set, the outer-most left and right
//   columns report a small offset so their CellBgRect can extend up to the outer border.
//   FIXME: But the rendering code in TableEndRow() nullifies that with clamping required for scrolling.
RblxRect RblxUI::TableGetCellBgRect(const RblxUITable* table, int column_n)
{
    const RblxUITableColumn* column = &table->Columns[column_n];
    float x1 = column->MinX;
    float x2 = column->MaxX;
    //if (column->PrevEnabledColumn == -1)
    //    x1 -= table->OuterPaddingX;
    //if (column->NextEnabledColumn == -1)
    //    x2 += table->OuterPaddingX;
    x1 = RblxMax(x1, table->WorkRect.Min.x);
    x2 = RblxMin(x2, table->WorkRect.Max.x);
    return RblxRect(x1, table->RowPosY1, x2, table->RowPosY2);
}

// Return the resizing ID for the right-side of the given column.
RblxUIID RblxUI::TableGetColumnResizeID(RblxUITable* table, int column_n, int instance_no)
{
    RBLXUI_ASSERT(column_n >= 0 && column_n < table->ColumnsCount);
    RblxUIID instance_id = TableGetInstanceID(table, instance_no);
    return instance_id + 1 + column_n; // FIXME: #6140: still not ideal
}

// Return -1 when table is not hovered. return columns_count if hovering the unused space at the right of the right-most visible column.
int RblxUI::TableGetHoveredColumn()
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    if (!table)
        return -1;
    return (int)table->HoveredColumnBody;
}

// Return -1 when table is not hovered. Return maxrow+1 if in table but below last submitted row.
// *IMPORTANT* Unlike TableGetHoveredColumn(), this has a one frame latency in updating the value.
// This difference with is the reason why this is not public yet.
int RblxUI::TableGetHoveredRow()
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    if (!table)
        return -1;
    RblxUITableInstanceData* table_instance = TableGetInstanceData(table, table->InstanceCurrent);
    return (int)table_instance->HoveredRowLast;
}

void RblxUI::TableSetBgColor(RblxUITableBgTarget target, RblxU32 color, int column_n)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    RBLXUI_ASSERT_USER_ERROR_RET(table != NULL, "Call should only be done while in BeginTable() scope!");
    RBLXUI_ASSERT(target != RblxUITableBgTarget_None);

    if (color == RBLXUI_COL32_DISABLE)
        color = 0;

    // We cannot draw neither the cell or row background immediately as we don't know the row height at this point in time.
    switch (target)
    {
    case RblxUITableBgTarget_CellBg:
    {
        if (table->RowPosY1 > table->InnerClipRect.Max.y) // Discard
            return;
        if (column_n == -1)
            column_n = table->CurrentColumn;
        if (!RBLXUI_BITARRAY_TESTBIT(table->VisibleMaskByIndex, column_n))
            return;
        if (table->RowCellDataCurrent < 0 || table->RowCellData[table->RowCellDataCurrent].Column != column_n)
            table->RowCellDataCurrent++;
        RblxUITableCellData* cell_data = &table->RowCellData[table->RowCellDataCurrent];
        cell_data->BgColor = color;
        cell_data->Column = (RblxUITableColumnIdx)column_n;
        break;
    }
    case RblxUITableBgTarget_RowBg0:
    case RblxUITableBgTarget_RowBg1:
    {
        if (table->RowPosY1 > table->InnerClipRect.Max.y) // Discard
            return;
        RBLXUI_ASSERT(column_n == -1);
        int bg_idx = (target == RblxUITableBgTarget_RowBg1) ? 1 : 0;
        table->RowBgColor[bg_idx] = color;
        break;
    }
    default:
        RBLXUI_ASSERT(0);
    }
}

//-------------------------------------------------------------------------
// [SECTION] Tables: Row changes
//-------------------------------------------------------------------------
// - TableGetRowIndex()
// - TableNextRow()
// - TableBeginRow() [Internal]
// - TableEndRow() [Internal]
//-------------------------------------------------------------------------

// [Public] Note: for row coloring we use ->RowBgColorCounter which is the same value without counting header rows
int RblxUI::TableGetRowIndex()
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    if (!table)
        return 0;
    return table->CurrentRow;
}

// [Public] Starts into the first cell of a new row
void RblxUI::TableNextRow(RblxUITableRowFlags row_flags, float row_min_height)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;

    if (!table->IsLayoutLocked)
        TableUpdateLayout(table);
    if (table->IsInsideRow)
        TableEndRow(table);

    table->LastRowFlags = table->RowFlags;
    table->RowFlags = row_flags;
    table->RowCellPaddingY = g.Style.CellPadding.y;
    table->RowMinHeight = row_min_height;
    TableBeginRow(table);

    // We honor min_row_height requested by user, but cannot guarantee per-row maximum height,
    // because that would essentially require a unique clipping rectangle per-cell.
    table->RowPosY2 += table->RowCellPaddingY * 2.0f;
    table->RowPosY2 = RblxMax(table->RowPosY2, table->RowPosY1 + row_min_height);

    // Disable output until user calls TableNextColumn()
    table->InnerWindow->SkipItems = true;
}

// [Internal] Only called by TableNextRow()
void RblxUI::TableBeginRow(RblxUITable* table)
{
    RblxUIWindow* window = table->InnerWindow;
    RBLXUI_ASSERT(!table->IsInsideRow);

    // New row
    table->CurrentRow++;
    table->CurrentColumn = -1;
    table->RowBgColor[0] = table->RowBgColor[1] = RBLXUI_COL32_DISABLE;
    table->RowCellDataCurrent = -1;
    table->IsInsideRow = true;

    // Begin frozen rows
    float next_y1 = table->RowPosY2;
    if (table->CurrentRow == 0 && table->FreezeRowsCount > 0)
        next_y1 = window->DC.CursorPos.y = table->OuterRect.Min.y;

    table->RowPosY1 = table->RowPosY2 = next_y1;
    table->RowTextBaseline = 0.0f;
    table->RowIndentOffsetX = window->DC.Indent.x - table->HostIndentX; // Lock indent

    window->DC.PrevLineTextBaseOffset = 0.0f;
    window->DC.CursorPosPrevLine = RblxVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + table->RowCellPaddingY); // This allows users to call SameLine() to share LineSize between columns.
    window->DC.PrevLineSize = window->DC.CurrLineSize = RblxVec2(0.0f, 0.0f); // This allows users to call SameLine() to share LineSize between columns, and to call it from first column too.
    window->DC.IsSameLine = window->DC.IsSetPos = false;
    window->DC.CursorMaxPos.y = next_y1;

    // Making the header BG color non-transparent will allow us to overlay it multiple times when handling smooth dragging.
    if (table->RowFlags & RblxUITableRowFlags_Headers)
    {
        TableSetBgColor(RblxUITableBgTarget_RowBg0, GetColorU32(RblxUICol_TableHeaderBg));
        if (table->CurrentRow == 0)
            table->IsUsingHeaders = true;
    }
}

// [Internal] Called by TableNextRow()
void RblxUI::TableEndRow(RblxUITable* table)
{
    RblxUIContext& g = *GRblxUI;
    RblxUIWindow* window = g.CurrentWindow;
    RBLXUI_ASSERT(window == table->InnerWindow);
    RBLXUI_ASSERT(table->IsInsideRow);

    if (table->CurrentColumn != -1)
    {
        TableEndCell(table);
        table->CurrentColumn = -1;
    }

    // Logging
    if (g.LogEnabled)
        LogRenderedText(NULL, "|");

    // Position cursor at the bottom of our row so it can be used for e.g. clipping calculation. However it is
    // likely that the next call to TableBeginCell() will reposition the cursor to take account of vertical padding.
    window->DC.CursorPos.y = table->RowPosY2;

    // Row background fill
    const float bg_y1 = table->RowPosY1;
    const float bg_y2 = table->RowPosY2;
    const bool unfreeze_rows_actual = (table->CurrentRow + 1 == table->FreezeRowsCount);
    const bool unfreeze_rows_request = (table->CurrentRow + 1 == table->FreezeRowsRequest);
    RblxUITableInstanceData* table_instance = TableGetInstanceData(table, table->InstanceCurrent);
    if ((table->RowFlags & RblxUITableRowFlags_Headers) && (table->CurrentRow == 0 || (table->LastRowFlags & RblxUITableRowFlags_Headers)))
        table_instance->LastTopHeadersRowHeight += bg_y2 - bg_y1;

    const bool is_visible = (bg_y2 >= table->InnerClipRect.Min.y && bg_y1 <= table->InnerClipRect.Max.y);
    if (is_visible)
    {
        // Update data for TableGetHoveredRow()
        if (table->HoveredColumnBody != -1 && g.IO.MousePos.y >= bg_y1 && g.IO.MousePos.y < bg_y2 && table_instance->HoveredRowNext < 0)
            table_instance->HoveredRowNext = table->CurrentRow;

        // Decide of background color for the row
        RblxU32 bg_col0 = 0;
        RblxU32 bg_col1 = 0;
        if (table->RowBgColor[0] != RBLXUI_COL32_DISABLE)
            bg_col0 = table->RowBgColor[0];
        else if (table->Flags & RblxUITableFlags_RowBg)
            bg_col0 = GetColorU32((table->RowBgColorCounter & 1) ? RblxUICol_TableRowBgAlt : RblxUICol_TableRowBg);
        if (table->RowBgColor[1] != RBLXUI_COL32_DISABLE)
            bg_col1 = table->RowBgColor[1];

        // Decide of top border color
        RblxU32 top_border_col = 0;
        const float border_size = TABLE_BORDER_SIZE;
        if (table->CurrentRow > 0 && (table->Flags & RblxUITableFlags_BordersInnerH))
            top_border_col = (table->LastRowFlags & RblxUITableRowFlags_Headers) ? table->BorderColorStrong : table->BorderColorLight;

        const bool draw_cell_bg_color = table->RowCellDataCurrent >= 0;
        const bool draw_strong_bottom_border = unfreeze_rows_actual;
        if ((bg_col0 | bg_col1 | top_border_col) != 0 || draw_strong_bottom_border || draw_cell_bg_color)
        {
            // In theory we could call SetWindowClipRectBeforeSetChannel() but since we know TableEndRow() is
            // always followed by a change of clipping rectangle we perform the smallest overwrite possible here.
            if ((table->Flags & RblxUITableFlags_NoClip) == 0)
                window->DrawList->_CmdHeader.ClipRect = table->Bg0ClipRectForDrawCmd.ToVec4();
            table->DrawSplitter->SetCurrentChannel(window->DrawList, TABLE_DRAW_CHANNEL_BG0);
        }

        // Draw row background
        // We soft/cpu clip this so all backgrounds and borders can share the same clipping rectangle
        if (bg_col0 || bg_col1)
        {
            RblxRect row_rect(table->WorkRect.Min.x, bg_y1, table->WorkRect.Max.x, bg_y2);
            row_rect.ClipWith(table->BgClipRect);
            if (bg_col0 != 0 && row_rect.Min.y < row_rect.Max.y)
                window->DrawList->AddRectFilled(row_rect.Min, row_rect.Max, bg_col0);
            if (bg_col1 != 0 && row_rect.Min.y < row_rect.Max.y)
                window->DrawList->AddRectFilled(row_rect.Min, row_rect.Max, bg_col1);
        }

        // Draw cell background color
        if (draw_cell_bg_color)
        {
            RblxUITableCellData* cell_data_end = &table->RowCellData[table->RowCellDataCurrent];
            for (RblxUITableCellData* cell_data = &table->RowCellData[0]; cell_data <= cell_data_end; cell_data++)
            {
                // As we render the BG here we need to clip things (for layout we would not)
                // FIXME: This cancels the OuterPadding addition done by TableGetCellBgRect(), need to keep it while rendering correctly while scrolling.
                const RblxUITableColumn* column = &table->Columns[cell_data->Column];
                RblxRect cell_bg_rect = TableGetCellBgRect(table, cell_data->Column);
                cell_bg_rect.ClipWith(table->BgClipRect);
                cell_bg_rect.Min.x = RblxMax(cell_bg_rect.Min.x, column->ClipRect.Min.x);     // So that first column after frozen one gets clipped when scrolling
                cell_bg_rect.Max.x = RblxMin(cell_bg_rect.Max.x, column->MaxX);
                if (cell_bg_rect.Min.y < cell_bg_rect.Max.y)
                    window->DrawList->AddRectFilled(cell_bg_rect.Min, cell_bg_rect.Max, cell_data->BgColor);
            }
        }

        // Draw top border
        if (top_border_col && bg_y1 >= table->BgClipRect.Min.y && bg_y1 < table->BgClipRect.Max.y)
            window->DrawList->AddLineH(table->BorderX1, table->BorderX2, bg_y1, top_border_col, border_size);

        // Draw bottom border at the row unfreezing mark (always strong)
        if (draw_strong_bottom_border && bg_y2 >= table->BgClipRect.Min.y && bg_y2 < table->BgClipRect.Max.y)
            window->DrawList->AddLineH(table->BorderX1, table->BorderX2, bg_y2, table->BorderColorStrong, border_size);
    }

    // End frozen rows (when we are past the last frozen row line, teleport cursor and alter clipping rectangle)
    // - We need to do that in TableEndRow() instead of TableBeginRow() so the list clipper can mark
    //   end of row and get the new cursor position.
    if (unfreeze_rows_request)
    {
        RBLXUI_ASSERT(table->FreezeRowsRequest > 0);
        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
            table->Columns[column_n].NavLayerCurrent = table->NavLayer;
        const float y0 = RblxMax(table->RowPosY2 + 1, table->InnerClipRect.Min.y);
        table_instance->LastFrozenHeight = y0 - table->OuterRect.Min.y;

        if (unfreeze_rows_actual)
        {
            RBLXUI_ASSERT(table->IsUnfrozenRows == false);
            table->IsUnfrozenRows = true;

            // BgClipRect starts as table->InnerClipRect, reduce it now and make BgClipRectForDrawCmd == BgClipRect
            table->BgClipRect.Min.y = table->Bg2ClipRectForDrawCmd.Min.y = RblxMin(y0, table->InnerClipRect.Max.y);
            table->BgClipRect.Max.y = table->Bg2ClipRectForDrawCmd.Max.y = table->InnerClipRect.Max.y;
            table->Bg2DrawChannelCurrent = table->Bg2DrawChannelUnfrozen;
            RBLXUI_ASSERT(table->Bg2ClipRectForDrawCmd.Min.y <= table->Bg2ClipRectForDrawCmd.Max.y);

            float row_height = table->RowPosY2 - table->RowPosY1;
            table->RowPosY2 = window->DC.CursorPos.y = table->WorkRect.Min.y + table->RowPosY2 - table->OuterRect.Min.y;
            table->RowPosY1 = table->RowPosY2 - row_height;
            for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
            {
                RblxUITableColumn* column = &table->Columns[column_n];
                column->DrawChannelCurrent = column->DrawChannelUnfrozen;
                column->ClipRect.Min.y = table->Bg2ClipRectForDrawCmd.Min.y;
            }

            // Update cliprect ahead of TableBeginCell() so clipper can access to new ClipRect->Min.y
            SetWindowClipRectBeforeSetChannel(window, table->Columns[0].ClipRect);
            table->DrawSplitter->SetCurrentChannel(window->DrawList, table->Columns[0].DrawChannelCurrent);
        }
    }

    if (!(table->RowFlags & RblxUITableRowFlags_Headers))
        table->RowBgColorCounter++;
    table->IsInsideRow = false;
}

//-------------------------------------------------------------------------
// [SECTION] Tables: Columns changes
//-------------------------------------------------------------------------
// - TableGetColumnIndex()
// - TableSetColumnIndex()
// - TableNextColumn()
// - TableBeginCell() [Internal]
// - TableEndCell() [Internal]
//-------------------------------------------------------------------------

int RblxUI::TableGetColumnIndex()
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    if (!table)
        return 0;
    return table->CurrentColumn;
}

// [Public] Append into a specific column
bool RblxUI::TableSetColumnIndex(int column_n)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    if (!table)
        return false;

    if (table->CurrentColumn != column_n)
    {
        if (table->CurrentColumn != -1)
            TableEndCell(table);
        RBLXUI_ASSERT_USER_ERROR_RETV(column_n >= 0 && column_n < table->ColumnsCount, false, "TableSetColumnIndex() invalid column index!");
        TableBeginCell(table, column_n);
    }

    // Return whether the column is visible. User may choose to skip submitting items based on this return value,
    // however they shouldn't skip submitting for columns that may have the tallest contribution to row height.
    return table->Columns[column_n].IsRequestOutput;
}

// [Public] Append into the next column, wrap and create a new row when already on last column
bool RblxUI::TableNextColumn()
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    if (!table)
        return false;

    if (table->IsInsideRow && table->CurrentColumn + 1 < table->ColumnsCount)
    {
        if (table->CurrentColumn != -1)
            TableEndCell(table);
        TableBeginCell(table, table->CurrentColumn + 1);
    }
    else
    {
        TableNextRow();
        TableBeginCell(table, 0);
    }

    // Return whether the column is visible. User may choose to skip submitting items based on this return value,
    // however they shouldn't skip submitting for columns that may have the tallest contribution to row height.
    return table->Columns[table->CurrentColumn].IsRequestOutput;
}


// [Internal] Called by TableSetColumnIndex()/TableNextColumn()
// This is called very frequently, so we need to be mindful of unnecessary overhead.
// FIXME-TABLE FIXME-OPT: Could probably shortcut some things for non-active or clipped columns.
void RblxUI::TableBeginCell(RblxUITable* table, int column_n)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITableColumn* column = &table->Columns[column_n];
    RblxUIWindow* window = table->InnerWindow;
    table->CurrentColumn = column_n;

    // Start position is roughly ~~ CellRect.Min + CellPadding + Indent
    float start_x = column->WorkMinX;
    if (column->Flags & RblxUITableColumnFlags_IndentEnable)
        start_x += table->RowIndentOffsetX; // ~~ += window.DC.Indent.x - table->HostIndentX, except we locked it for the row.

    window->DC.CursorPos.x = start_x;
    window->DC.CursorPos.y = table->RowPosY1 + table->RowCellPaddingY;
    window->DC.CursorMaxPos.x = window->DC.CursorPos.x;
    window->DC.ColumnsOffset.x = start_x - window->Pos.x - window->DC.Indent.x; // FIXME-WORKRECT
    window->DC.CursorPosPrevLine.x = window->DC.CursorPos.x; // PrevLine.y is preserved. This allows users to call SameLine() to share LineSize between columns.
    window->DC.CurrLineTextBaseOffset = table->RowTextBaseline;
    window->DC.NavLayerCurrent = (RblxUINavLayer)column->NavLayerCurrent;

    // Note how WorkRect.Max.y is only set once during layout
    window->WorkRect.Min.y = window->DC.CursorPos.y;
    window->WorkRect.Min.x = column->WorkMinX;
    window->WorkRect.Max.x = column->WorkMaxX;
    window->DC.ItemWidth = column->ItemWidth;

    window->SkipItems = column->IsSkipItems;
    if (column->IsSkipItems)
    {
        g.LastItemData.ID = 0;
        g.LastItemData.StatusFlags = 0;
    }

    // Also see TablePushColumnChannel()
    if (table->Flags & RblxUITableFlags_NoClip)
    {
        // FIXME: if we end up drawing all borders/bg in EndTable, could remove this and just assert that channel hasn't changed.
        table->DrawSplitter->SetCurrentChannel(window->DrawList, TABLE_DRAW_CHANNEL_NOCLIP);
        //RBLXUI_ASSERT(table->DrawSplitter._Current == TABLE_DRAW_CHANNEL_NOCLIP);
    }
    else
    {
        // FIXME-TABLE: Could avoid this if draw channel is dummy channel?
        SetWindowClipRectBeforeSetChannel(window, column->ClipRect);
        table->DrawSplitter->SetCurrentChannel(window->DrawList, column->DrawChannelCurrent);
    }

    // Logging
    if (g.LogEnabled && !column->IsSkipItems)
    {
        LogRenderedText(&window->DC.CursorPos, "|");
        g.LogLinePosY = FLT_MAX;
    }
}

// [Internal] Called by TableNextRow()/TableSetColumnIndex()/TableNextColumn()
void RblxUI::TableEndCell(RblxUITable* table)
{
    RblxUITableColumn* column = &table->Columns[table->CurrentColumn];
    RblxUIWindow* window = table->InnerWindow;

    if (window->DC.IsSetPos)
        ErrorCheckUsingSetCursorPosToExtendParentBoundaries();

    // Report maximum position so we can infer content size per column.
    float* p_max_pos_x;
    if (table->RowFlags & RblxUITableRowFlags_Headers)
        p_max_pos_x = &column->ContentMaxXHeadersUsed;  // Useful in case user submit contents in header row that is not a TableHeader() call
    else
        p_max_pos_x = table->IsUnfrozenRows ? &column->ContentMaxXUnfrozen : &column->ContentMaxXFrozen;
    *p_max_pos_x = RblxMax(*p_max_pos_x, window->DC.CursorMaxPos.x);
    if (column->IsEnabled)
        table->RowPosY2 = RblxMax(table->RowPosY2, window->DC.CursorMaxPos.y + table->RowCellPaddingY);
    column->ItemWidth = window->DC.ItemWidth;

    // Propagate text baseline for the entire row
    // FIXME-TABLE: Here we propagate text baseline from the last line of the cell.. instead of the first one.
    table->RowTextBaseline = RblxMax(table->RowTextBaseline, window->DC.PrevLineTextBaseOffset);
}

//-------------------------------------------------------------------------
// [SECTION] Tables: Columns width management
//-------------------------------------------------------------------------
// - TableGetMaxColumnWidth() [Internal]
// - TableGetColumnWidthAuto() [Internal]
// - TableSetColumnWidth()
// - TableSetColumnWidthAutoSingle() [Internal]
// - TableSetColumnWidthAutoAll() [Internal]
// - TableUpdateColumnsWeightFromWidth() [Internal]
//-------------------------------------------------------------------------
// Note that actual columns widths are computed in TableUpdateLayout().
//-------------------------------------------------------------------------

// Maximum column content width given current layout. Use column->MinX so this value differs on a per-column basis.
float RblxUI::TableCalcMaxColumnWidth(const RblxUITable* table, int column_n)
{
    const RblxUITableColumn* column = &table->Columns[column_n];
    float max_width = FLT_MAX;
    const float min_column_distance = table->MinColumnWidth + table->CellPaddingX * 2.0f + table->CellSpacingX1 + table->CellSpacingX2;
    if (table->Flags & RblxUITableFlags_ScrollX)
    {
        // Frozen columns can't reach beyond visible width else scrolling will naturally break.
        // (we use DisplayOrder as within a set of multiple frozen column reordering is possible)
        if (column->DisplayOrder < table->FreezeColumnsRequest)
        {
            max_width = (table->InnerClipRect.Max.x - (table->FreezeColumnsRequest - column->DisplayOrder) * min_column_distance) - column->MinX;
            max_width = max_width - table->OuterPaddingX - table->CellPaddingX - table->CellSpacingX2;
        }
    }
    else if ((table->Flags & RblxUITableFlags_NoKeepColumnsVisible) == 0)
    {
        // If horizontal scrolling if disabled, we apply a final lossless shrinking of columns in order to make
        // sure they are all visible. Because of this we also know that all of the columns will always fit in
        // table->WorkRect and therefore in table->InnerRect (because ScrollX is off)
        // FIXME-TABLE: This is solved incorrectly but also quite a difficult problem to fix as we also want ClipRect width to match.
        // See "table_width_distrib" and "table_width_keep_visible" tests
        max_width = table->WorkRect.Max.x - (table->ColumnsEnabledCount - column->IndexWithinEnabledSet - 1) * min_column_distance - column->MinX;
        //max_width -= table->CellSpacingX1;
        max_width -= table->CellSpacingX2;
        max_width -= table->CellPaddingX * 2.0f;
        max_width -= table->OuterPaddingX;
    }
    return max_width;
}

// Note this is meant to be stored in column->WidthAuto, please generally use the WidthAuto field
float RblxUI::TableGetColumnWidthAuto(RblxUITable* table, RblxUITableColumn* column)
{
    const float content_width_body = RblxMax(column->ContentMaxXFrozen, column->ContentMaxXUnfrozen) - column->WorkMinX;
    const float content_width_headers = column->ContentMaxXHeadersIdeal - column->WorkMinX;
    float width_auto = content_width_body;
    if (!(column->Flags & RblxUITableColumnFlags_NoHeaderWidth))
        width_auto = RblxMax(width_auto, content_width_headers);

    // Non-resizable fixed columns preserve their requested width
    if ((column->Flags & RblxUITableColumnFlags_WidthFixed) && column->InitStretchWeightOrWidth > 0.0f)
        if (!(table->Flags & RblxUITableFlags_Resizable) || (column->Flags & RblxUITableColumnFlags_NoResize))
            width_auto = column->InitStretchWeightOrWidth;

    return RblxMax(width_auto, table->MinColumnWidth);
}

// 'width' = inner column width, without padding
void RblxUI::TableSetColumnWidth(int column_n, float width)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    RBLXUI_ASSERT(table != NULL && table->IsLayoutLocked == false);
    RBLXUI_ASSERT(column_n >= 0 && column_n < table->ColumnsCount);
    RblxUITableColumn* column_0 = &table->Columns[column_n];
    float column_0_width = width;

    // Apply constraints early
    // Compare both requested and actual given width to avoid overwriting requested width when column is stuck (minimum size, bounded)
    RBLXUI_ASSERT(table->MinColumnWidth > 0.0f);
    const float min_width = table->MinColumnWidth;
    const float max_width = RblxMax(min_width, column_0->WidthMax); // Don't use TableCalcMaxColumnWidth() here as it would rely on MinX from last instance (#7933)
    column_0_width = RblxClamp(column_0_width, min_width, max_width);
    if (column_0->WidthGiven == column_0_width || column_0->WidthRequest == column_0_width)
        return;

    //RBLXUI_DEBUG_PRINT("TableSetColumnWidth(%d, %.1f->%.1f)\n", column_0_idx, column_0->WidthGiven, column_0_width);
    RblxUITableColumn* column_1 = (column_0->NextEnabledColumn != -1) ? &table->Columns[column_0->NextEnabledColumn] : NULL;

    // In this surprisingly not simple because of how we support mixing Fixed and multiple Stretch columns.
    // - All fixed: easy.
    // - All stretch: easy.
    // - One or more fixed + one stretch: easy.
    // - One or more fixed + more than one stretch: tricky.
    // Qt when manual resize is enabled only supports a single _trailing_ stretch column, we support more cases here.

    // When forwarding resize from Wn| to Fn+1| we need to be considerate of the _NoResize flag on Fn+1.
    // FIXME-TABLE: Find a way to rewrite all of this so interactions feel more consistent for the user.
    // Scenarios:
    // - F1 F2 F3  resize from F1| or F2|   --> ok: alter ->WidthRequested of Fixed column. Subsequent columns will be offset.
    // - F1 F2 F3  resize from F3|          --> ok: alter ->WidthRequested of Fixed column. If active, ScrollX extent can be altered.
    // - F1 F2 W3  resize from F1| or F2|   --> ok: alter ->WidthRequested of Fixed column. If active, ScrollX extent can be altered, but it doesn't make much sense as the Stretch column will always be minimal size.
    // - F1 F2 W3  resize from W3|          --> ok: no-op (disabled by Resize Rule 1)
    // - W1 W2 W3  resize from W1| or W2|   --> ok
    // - W1 W2 W3  resize from W3|          --> ok: no-op (disabled by Resize Rule 1)
    // - W1 F2 F3  resize from F3|          --> ok: no-op (disabled by Resize Rule 1)
    // - W1 F2     resize from F2|          --> ok: no-op (disabled by Resize Rule 1)
    // - W1 W2 F3  resize from W1| or W2|   --> ok
    // - W1 F2 W3  resize from W1| or F2|   --> ok
    // - F1 W2 F3  resize from W2|          --> ok
    // - F1 W3 F2  resize from W3|          --> ok
    // - W1 F2 F3  resize from W1|          --> ok: equivalent to resizing |F2. F3 will not move.
    // - W1 F2 F3  resize from F2|          --> ok
    // All resizes from a Wx columns are locking other columns.

    // Possible improvements:
    // - W1 W2 W3  resize W1|               --> to not be stuck, both W2 and W3 would stretch down. Seems possible to fix. Would be most beneficial to simplify resize of all-weighted columns.
    // - W3 F1 F2  resize W3|               --> to not be stuck past F1|, both F1 and F2 would need to stretch down, which would be lossy or ambiguous. Seems hard to fix.

    // [Resize Rule 1] Can't resize from right of right-most visible column if there is any Stretch column. Implemented in TableUpdateLayout().

    // If we have all Fixed columns OR resizing a Fixed column that doesn't come after a Stretch one, we can do an offsetting resize.
    // This is the preferred resize path
    if (column_0->Flags & RblxUITableColumnFlags_WidthFixed)
        if (!column_1 || table->LeftMostStretchedColumn == -1 || table->Columns[table->LeftMostStretchedColumn].DisplayOrder >= column_0->DisplayOrder)
        {
            column_0->WidthRequest = column_0_width;
            table->IsSettingsDirty = true;
            return;
        }

    // We can also use previous column if there's no next one (this is used when doing an auto-fit on the right-most stretch column)
    if (column_1 == NULL)
        column_1 = (column_0->PrevEnabledColumn != -1) ? &table->Columns[column_0->PrevEnabledColumn] : NULL;
    if (column_1 == NULL)
        return;

    // Resizing from right-side of a Stretch column before a Fixed column forward sizing to left-side of fixed column.
    // (old_a + old_b == new_a + new_b) --> (new_a == old_a + old_b - new_b)
    float column_1_width = RblxMax(column_1->WidthRequest - (column_0_width - column_0->WidthRequest), min_width);
    column_0_width = column_0->WidthRequest + column_1->WidthRequest - column_1_width;
    RBLXUI_ASSERT(column_0_width > 0.0f && column_1_width > 0.0f);
    column_0->WidthRequest = column_0_width;
    column_1->WidthRequest = column_1_width;
    if ((column_0->Flags | column_1->Flags) & RblxUITableColumnFlags_WidthStretch)
        TableUpdateColumnsWeightFromWidth(table);
    table->IsSettingsDirty = true;
}

// Disable clipping then auto-fit, will take 2 frames
// (we don't take a shortcut for unclipped columns to reduce inconsistencies when e.g. resizing multiple columns)
void RblxUI::TableSetColumnWidthAutoSingle(RblxUITable* table, int column_n)
{
    // Single auto width uses auto-fit
    RblxUITableColumn* column = &table->Columns[column_n];
    if (!column->IsEnabled)
        return;
    column->CannotSkipItemsQueue = (1 << 0);
    table->AutoFitSingleColumn = (RblxUITableColumnIdx)column_n;
}

void RblxUI::TableSetColumnWidthAutoAll(RblxUITable* table)
{
    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
    {
        RblxUITableColumn* column = &table->Columns[column_n];
        if (!column->IsEnabled && !(column->Flags & RblxUITableColumnFlags_WidthStretch)) // Cannot reset weight of hidden stretch column
            continue;
        column->CannotSkipItemsQueue = (1 << 0);
        column->AutoFitQueue = (1 << 1);
    }
}

void RblxUI::TableUpdateColumnsWeightFromWidth(RblxUITable* table)
{
    RBLXUI_ASSERT(table->LeftMostStretchedColumn != -1 && table->RightMostStretchedColumn != -1);

    // Measure existing quantities
    float visible_weight = 0.0f;
    float visible_width = 0.0f;
    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
    {
        RblxUITableColumn* column = &table->Columns[column_n];
        if (!column->IsEnabled || !(column->Flags & RblxUITableColumnFlags_WidthStretch))
            continue;
        RBLXUI_ASSERT(column->StretchWeight > 0.0f);
        visible_weight += column->StretchWeight;
        visible_width += column->WidthRequest;
    }
    RBLXUI_ASSERT(visible_weight > 0.0f && visible_width > 0.0f);

    // Apply new weights
    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
    {
        RblxUITableColumn* column = &table->Columns[column_n];
        if (!column->IsEnabled || !(column->Flags & RblxUITableColumnFlags_WidthStretch))
            continue;
        column->StretchWeight = (column->WidthRequest / visible_width) * visible_weight;
        RBLXUI_ASSERT(column->StretchWeight > 0.0f);
    }
}

//-------------------------------------------------------------------------
// [SECTION] Tables: Drawing
//-------------------------------------------------------------------------
// - TablePushBackgroundChannel() [Internal]
// - TablePopBackgroundChannel() [Internal]
// - TableSetupDrawChannels() [Internal]
// - TableMergeDrawChannels() [Internal]
// - TableGetColumnBorderCol() [Internal]
// - TableDrawBorders() [Internal]
//-------------------------------------------------------------------------


// FIXME: This could be abstracted and merged with PushColumnsBackground(), by creating a generic struct
// with storage for backup cliprect + backup channel + storage for splitter pointer, new clip rect.
// This would slightly simplify caller code.

// Bg2 is used by Selectable (and possibly other widgets) to render to the background.
// Unlike our Bg0/1 channel which we uses for RowBg/CellBg/Borders and where we guarantee all shapes to be CPU-clipped, the Bg2 channel being widgets-facing will rely on regular ClipRect.
void RblxUI::TablePushBackgroundChannel()
{
    RblxUIContext& g = *GRblxUI;
    RblxUIWindow* window = g.CurrentWindow;
    RblxUITable* table = g.CurrentTable;

    // Optimization: avoid SetCurrentChannel() + PushClipRect()
    table->HostBackupInnerClipRect = window->ClipRect;
    SetWindowClipRectBeforeSetChannel(window, table->Bg2ClipRectForDrawCmd);
    table->DrawSplitter->SetCurrentChannel(window->DrawList, table->Bg2DrawChannelCurrent);
}

void RblxUI::TablePopBackgroundChannel()
{
    RblxUIContext& g = *GRblxUI;
    RblxUIWindow* window = g.CurrentWindow;
    RblxUITable* table = g.CurrentTable;

    // Optimization: avoid PopClipRect() + SetCurrentChannel()
    SetWindowClipRectBeforeSetChannel(window, table->HostBackupInnerClipRect);
    table->DrawSplitter->SetCurrentChannel(window->DrawList, table->Columns[table->CurrentColumn].DrawChannelCurrent);
}

// Also see TableBeginCell()
void RblxUI::TablePushColumnChannel(int column_n)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;

    // Optimization: avoid SetCurrentChannel() + PushClipRect()
    if (table->Flags & RblxUITableFlags_NoClip)
        return;
    RblxUIWindow* window = g.CurrentWindow;
    const RblxUITableColumn* column = &table->Columns[column_n];
    SetWindowClipRectBeforeSetChannel(window, column->ClipRect);
    table->DrawSplitter->SetCurrentChannel(window->DrawList, column->DrawChannelCurrent);
}

void RblxUI::TablePopColumnChannel()
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;

    // Optimization: avoid PopClipRect() + SetCurrentChannel()
    if ((table->Flags & RblxUITableFlags_NoClip) || (table->CurrentColumn == -1)) // Calling TreePop() after TableNextRow() is supported.
        return;
    RblxUIWindow* window = g.CurrentWindow;
    const RblxUITableColumn* column = &table->Columns[table->CurrentColumn];
    SetWindowClipRectBeforeSetChannel(window, column->ClipRect);
    table->DrawSplitter->SetCurrentChannel(window->DrawList, column->DrawChannelCurrent);
}

// Allocate draw channels. Called by TableUpdateLayout()
// - We allocate them following storage order instead of display order so reordering columns won't needlessly
//   increase overall dormant memory cost.
// - We isolate headers draw commands in their own channels instead of just altering clip rects.
//   This is in order to facilitate merging of draw commands.
// - After crossing FreezeRowsCount, all columns see their current draw channel changed to a second set of channels.
// - We only use the dummy draw channel so we can push a null clipping rectangle into it without affecting other
//   channels, while simplifying per-row/per-cell overhead. It will be empty and discarded when merged.
// - We allocate 1 or 2 background draw channels. This is because we know TablePushBackgroundChannel() is only used for
//   horizontal spanning. If we allowed vertical spanning we'd need one background draw channel per merge group (1-4).
// Draw channel allocation (before merging):
// - NoClip                       --> 2+D+1 channels: bg0/1 + bg2 + foreground (same clip rect == always 1 draw call)
// - Clip                         --> 2+D+N channels
// - FreezeRows                   --> 2+D+N*2 (unless scrolling value is zero)
// - FreezeRows || FreezeColumns  --> 3+D+N*2 (unless scrolling value is zero)
// Where D is 1 if any column is clipped or hidden (dummy channel) otherwise 0.
void RblxUI::TableSetupDrawChannels(RblxUITable* table)
{
    const int freeze_row_multiplier = (table->FreezeRowsCount > 0) ? 2 : 1;
    const int channels_for_row = (table->Flags & RblxUITableFlags_NoClip) ? 1 : table->ColumnsEnabledCount;
    const int channels_for_bg = 1 + 1 * freeze_row_multiplier;
    const int channels_for_dummy = (table->ColumnsEnabledCount < table->ColumnsCount || (memcmp(table->VisibleMaskByIndex, table->EnabledMaskByIndex, RblxBitArrayGetStorageSizeInBytes(table->ColumnsCount)) != 0)) ? +1 : 0;
    const int channels_total = channels_for_bg + (channels_for_row * freeze_row_multiplier) + channels_for_dummy;
    table->DrawSplitter->Split(table->InnerWindow->DrawList, channels_total);
    table->DummyDrawChannel = (RblxUITableDrawChannelIdx)((channels_for_dummy > 0) ? channels_total - 1 : -1);
    table->Bg2DrawChannelCurrent = TABLE_DRAW_CHANNEL_BG2_FROZEN;
    table->Bg2DrawChannelUnfrozen = (RblxUITableDrawChannelIdx)((table->FreezeRowsCount > 0) ? 2 + channels_for_row : TABLE_DRAW_CHANNEL_BG2_FROZEN);

    int draw_channel_current = 2;
    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
    {
        RblxUITableColumn* column = &table->Columns[column_n];
        if (column->IsVisibleX && column->IsVisibleY)
        {
            column->DrawChannelFrozen = (RblxUITableDrawChannelIdx)(draw_channel_current);
            column->DrawChannelUnfrozen = (RblxUITableDrawChannelIdx)(draw_channel_current + (table->FreezeRowsCount > 0 ? channels_for_row + 1 : 0));
            if (!(table->Flags & RblxUITableFlags_NoClip))
                draw_channel_current++;
        }
        else
        {
            column->DrawChannelFrozen = column->DrawChannelUnfrozen = table->DummyDrawChannel;
        }
        column->DrawChannelCurrent = column->DrawChannelFrozen;
    }

    // Initial draw cmd starts with a BgClipRect that matches the one of its host, to facilitate merge draw commands by default.
    // All our cell highlight are manually clipped with BgClipRect. When unfreezing it will be made smaller to fit scrolling rect.
    // (This technically isn't part of setting up draw channels, but is reasonably related to be done here)
    table->BgClipRect = table->InnerClipRect;
    table->Bg0ClipRectForDrawCmd = table->OuterWindow->ClipRect;
    table->Bg2ClipRectForDrawCmd = table->HostClipRect;
    RBLXUI_ASSERT(table->BgClipRect.Min.y <= table->BgClipRect.Max.y);
}

// This function reorder draw channels based on matching clip rectangle, to facilitate merging them. Called by EndTable().
// For simplicity we call it TableMergeDrawChannels() but in fact it only reorder channels + overwrite ClipRect,
// actual merging is done by table->DrawSplitter.Merge() which is called right after TableMergeDrawChannels().
//
// Columns where the contents didn't stray off their local clip rectangle can be merged. To achieve
// this we merge their clip rect and make them contiguous in the channel list, so they can be merged
// by the call to DrawSplitter.Merge() following to the call to this function.
// We reorder draw commands by arranging them into a maximum of 4 distinct groups:
//
//   1 group:               2 groups:              2 groups:              4 groups:
//   [ 0. ] no freeze       [ 0. ] row freeze      [ 01 ] col freeze      [ 01 ] row+col freeze
//   [ .. ]  or no scroll   [ 2. ]  and v-scroll   [ .. ]  and h-scroll   [ 23 ]  and v+h-scroll
//
// Each column itself can use 1 channel (row freeze disabled) or 2 channels (row freeze enabled).
// When the contents of a column didn't stray off its limit, we move its channels into the corresponding group
// based on its position (within frozen rows/columns groups or not).
// At the end of the operation our 1-4 groups will each have a RblxDrawCmd using the same ClipRect.
// This function assume that each column are pointing to a distinct draw channel,
// otherwise merge_group->ChannelsCount will not match set bit count of merge_group->ChannelsMask.
//
// Column channels will not be merged into one of the 1-4 groups in the following cases:
// - The contents stray off its clipping rectangle (we only compare the MaxX value, not the MinX value).
//   Direct RblxDrawList calls won't be taken into account by default, if you use them make sure the RblxUI:: bounds
//   matches, by e.g. calling SetCursorScreenPos().
// - The channel uses more than one draw command itself. We drop all our attempt at merging stuff here..
//   we could do better but it's going to be rare and probably not worth the hassle.
// Columns for which the draw channel(s) haven't been merged with other will use their own RblxDrawCmd.
//
// This function is particularly tricky to understand.. take a breath.
void RblxUI::TableMergeDrawChannels(RblxUITable* table)
{
    RblxUIContext& g = *GRblxUI;
    RblxDrawListSplitter* splitter = table->DrawSplitter;
    const bool has_freeze_v = (table->FreezeRowsCount > 0);
    const bool has_freeze_h = (table->FreezeColumnsCount > 0);
    RBLXUI_ASSERT(splitter->_Current == 0);

    // Track which groups we are going to attempt to merge, and which channels goes into each group.
    struct MergeGroup
    {
        RblxRect          ClipRect;
        int             ChannelsCount = 0;
        RblxBitArrayPtr   ChannelsMask = NULL;
    };
    int merge_group_mask = 0x00;
    MergeGroup merge_groups[4];

    // Use a reusable temp buffer for the merge masks as they are dynamically sized.
    const int max_draw_channels = (4 + table->ColumnsCount * 2);
    const int size_for_masks_bitarrays_one = (int)RblxBitArrayGetStorageSizeInBytes(max_draw_channels);
    g.TempBuffer.reserve(size_for_masks_bitarrays_one * 5);
    memset(g.TempBuffer.Data, 0, size_for_masks_bitarrays_one * 5);
    for (int n = 0; n < RBLXUI_COUNTOF(merge_groups); n++)
        merge_groups[n].ChannelsMask = (RblxBitArrayPtr)(void*)(g.TempBuffer.Data + (size_for_masks_bitarrays_one * n));
    RblxBitArrayPtr remaining_mask = (RblxBitArrayPtr)(void*)(g.TempBuffer.Data + (size_for_masks_bitarrays_one * 4));

    // 1. Scan channels and take note of those which can be merged
    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
    {
        if (!RBLXUI_BITARRAY_TESTBIT(table->VisibleMaskByIndex, column_n))
            continue;
        RblxUITableColumn* column = &table->Columns[column_n];

        const int merge_group_sub_count = has_freeze_v ? 2 : 1;
        for (int merge_group_sub_n = 0; merge_group_sub_n < merge_group_sub_count; merge_group_sub_n++)
        {
            const int channel_no = (merge_group_sub_n == 0) ? column->DrawChannelFrozen : column->DrawChannelUnfrozen;

            // Don't attempt to merge if there are multiple draw calls within the column
            RblxDrawChannel* src_channel = &splitter->_Channels[channel_no];
            if (src_channel->_CmdBuffer.Size > 0 && src_channel->_CmdBuffer.back().ElemCount == 0 && src_channel->_CmdBuffer.back().UserCallback == NULL) // Equivalent of PopUnusedDrawCmd()
                src_channel->_CmdBuffer.pop_back();
            if (src_channel->_CmdBuffer.Size != 1)
                continue;

            // Find out the width of this merge group and check if it will fit in our column
            // (note that we assume that rendering didn't stray on the left direction. we should need a CursorMinPos to detect it)
            if (!(column->Flags & RblxUITableColumnFlags_NoClip))
            {
                float content_max_x;
                if (!has_freeze_v)
                    content_max_x = RblxMax(column->ContentMaxXUnfrozen, column->ContentMaxXHeadersUsed); // No row freeze
                else if (merge_group_sub_n == 0)
                    content_max_x = RblxMax(column->ContentMaxXFrozen, column->ContentMaxXHeadersUsed);   // Row freeze: use width before freeze
                else
                    content_max_x = column->ContentMaxXUnfrozen;                                        // Row freeze: use width after freeze
                if (content_max_x > column->ClipRect.Max.x)
                    continue;
            }

            const int merge_group_n = (has_freeze_h && column_n < table->FreezeColumnsCount ? 0 : 1) + (has_freeze_v && merge_group_sub_n == 0 ? 0 : 2);
            RBLXUI_ASSERT(channel_no < max_draw_channels);
            MergeGroup* merge_group = &merge_groups[merge_group_n];
            if (merge_group->ChannelsCount == 0)
                merge_group->ClipRect = RblxRect(+FLT_MAX, +FLT_MAX, -FLT_MAX, -FLT_MAX);
            RblxBitArraySetBit(merge_group->ChannelsMask, channel_no);
            merge_group->ChannelsCount++;
            merge_group->ClipRect.Add(src_channel->_CmdBuffer[0].ClipRect);
            merge_group_mask |= (1 << merge_group_n);
        }

        // Invalidate current draw channel
        // (we don't clear DrawChannelFrozen/DrawChannelUnfrozen solely to facilitate debugging/later inspection of data)
        column->DrawChannelCurrent = (RblxUITableDrawChannelIdx)-1;
    }

    // [DEBUG] Display merge groups
#if 0
    if (g.IO.KeyShift)
        for (int merge_group_n = 0; merge_group_n < RBLXUI_COUNTOF(merge_groups); merge_group_n++)
        {
            MergeGroup* merge_group = &merge_groups[merge_group_n];
            if (merge_group->ChannelsCount == 0)
                continue;
            char buf[32];
            RblxFormatString(buf, 32, "MG%d:%d", merge_group_n, merge_group->ChannelsCount);
            RblxVec2 text_pos = merge_group->ClipRect.Min + RblxVec2(4, 4);
            RblxVec2 text_size = CalcTextSize(buf, NULL);
            GetForegroundDrawList()->AddRectFilled(text_pos, text_pos + text_size, RBLXUI_COL32(0, 0, 0, 255));
            GetForegroundDrawList()->AddText(text_pos, RBLXUI_COL32(255, 255, 0, 255), buf, NULL);
            GetForegroundDrawList()->AddRect(merge_group->ClipRect.Min, merge_group->ClipRect.Max, RBLXUI_COL32(255, 255, 0, 255));
        }
#endif

    // 2. Rewrite channel list in our preferred order
    if (merge_group_mask != 0)
    {
        // We skip channel 0 (Bg0/Bg1) and 1 (Bg2 frozen) from the shuffling since they won't move - see channels allocation in TableSetupDrawChannels().
        const int LEADING_DRAW_CHANNELS = 2;
        g.DrawChannelsTempMergeBuffer.resize(splitter->_Count - LEADING_DRAW_CHANNELS); // Use shared temporary storage so the allocation gets amortized
        RblxDrawChannel* dst_tmp = g.DrawChannelsTempMergeBuffer.Data;
        RblxBitArraySetBitRange(remaining_mask, LEADING_DRAW_CHANNELS, splitter->_Count);
        RblxBitArrayClearBit(remaining_mask, table->Bg2DrawChannelUnfrozen);
        RBLXUI_ASSERT(has_freeze_v == false || table->Bg2DrawChannelUnfrozen != TABLE_DRAW_CHANNEL_BG2_FROZEN);
        int remaining_count = splitter->_Count - (has_freeze_v ? LEADING_DRAW_CHANNELS + 1 : LEADING_DRAW_CHANNELS);
        //RblxRect host_rect = (table->InnerWindow == table->OuterWindow) ? table->InnerClipRect : table->HostClipRect;
        RblxRect host_rect = table->HostClipRect;
        for (int merge_group_n = 0; merge_group_n < RBLXUI_COUNTOF(merge_groups); merge_group_n++)
        {
            if (int merge_channels_count = merge_groups[merge_group_n].ChannelsCount)
            {
                MergeGroup* merge_group = &merge_groups[merge_group_n];
                RblxRect merge_clip_rect = merge_group->ClipRect;

                // Extend outer-most clip limits to match those of host, so draw calls can be merged even if
                // outer-most columns have some outer padding offsetting them from their parent ClipRect.
                // The principal cases this is dealing with are:
                // - On a same-window table (not scrolling = single group), all fitting columns ClipRect -> will extend and match host ClipRect -> will merge
                // - Columns can use padding and have left-most ClipRect.Min.x and right-most ClipRect.Max.x != from host ClipRect -> will extend and match host ClipRect -> will merge
                // FIXME-TABLE FIXME-WORKRECT: We are wasting a merge opportunity on tables without scrolling if column doesn't fit
                // within host clip rect, solely because of the half-padding difference between window->WorkRect and window->InnerClipRect.
                if ((merge_group_n & 1) == 0 || !has_freeze_h)
                    merge_clip_rect.Min.x = RblxMin(merge_clip_rect.Min.x, host_rect.Min.x);
                if ((merge_group_n & 2) == 0 || !has_freeze_v)
                    merge_clip_rect.Min.y = RblxMin(merge_clip_rect.Min.y, host_rect.Min.y);
                if ((merge_group_n & 1) != 0)
                    merge_clip_rect.Max.x = RblxMax(merge_clip_rect.Max.x, host_rect.Max.x);
                if ((merge_group_n & 2) != 0 && (table->Flags & RblxUITableFlags_NoHostExtendY) == 0)
                    merge_clip_rect.Max.y = RblxMax(merge_clip_rect.Max.y, host_rect.Max.y);
                //GetForegroundDrawList()->AddRect(merge_group->ClipRect.Min, merge_group->ClipRect.Max, RBLXUI_COL32(255, 0, 0, 200), 0.0f, 0, 1.0f); // [DEBUG]
                //GetForegroundDrawList()->AddLine(merge_group->ClipRect.Min, merge_clip_rect.Min, RBLXUI_COL32(255, 100, 0, 200));
                //GetForegroundDrawList()->AddLine(merge_group->ClipRect.Max, merge_clip_rect.Max, RBLXUI_COL32(255, 100, 0, 200));
                remaining_count -= merge_group->ChannelsCount;
                for (int n = 0; n < (size_for_masks_bitarrays_one >> 2); n++)
                    remaining_mask[n] &= ~merge_group->ChannelsMask[n];
                for (int n = 0; n < splitter->_Count && merge_channels_count != 0; n++)
                {
                    // Copy + overwrite new clip rect
                    if (!RBLXUI_BITARRAY_TESTBIT(merge_group->ChannelsMask, n))
                        continue;
                    RBLXUI_BITARRAY_CLEARBIT(merge_group->ChannelsMask, n);
                    merge_channels_count--;

                    RblxDrawChannel* channel = &splitter->_Channels[n];
                    RBLXUI_ASSERT(channel->_CmdBuffer.Size == 1 && merge_clip_rect.Contains(RblxRect(channel->_CmdBuffer[0].ClipRect)));
                    channel->_CmdBuffer[0].ClipRect = merge_clip_rect.ToVec4();
                    memcpy(dst_tmp++, channel, sizeof(RblxDrawChannel));
                }
            }

            // Make sure Bg2DrawChannelUnfrozen appears in the middle of our groups (whereas Bg0/Bg1 and Bg2 frozen are fixed to 0 and 1)
            if (merge_group_n == 1 && has_freeze_v)
                memcpy(dst_tmp++, &splitter->_Channels[table->Bg2DrawChannelUnfrozen], sizeof(RblxDrawChannel));
        }

        // Append unmergeable channels that we didn't reorder at the end of the list
        for (int n = 0; n < splitter->_Count && remaining_count != 0; n++)
        {
            if (!RBLXUI_BITARRAY_TESTBIT(remaining_mask, n))
                continue;
            RblxDrawChannel* channel = &splitter->_Channels[n];
            memcpy(dst_tmp++, channel, sizeof(RblxDrawChannel));
            remaining_count--;
        }
        RBLXUI_ASSERT(dst_tmp == g.DrawChannelsTempMergeBuffer.Data + g.DrawChannelsTempMergeBuffer.Size);
        memcpy(splitter->_Channels.Data + LEADING_DRAW_CHANNELS, g.DrawChannelsTempMergeBuffer.Data, (splitter->_Count - LEADING_DRAW_CHANNELS) * sizeof(RblxDrawChannel));
    }
}

static RblxU32 TableGetColumnBorderCol(RblxUITable* table, int order_n, int column_n)
{
    const bool is_hovered = (table->HoveredColumnBorder == column_n);
    const bool is_resized = (table->ResizedColumn == column_n) && (table->InstanceInteracted == table->InstanceCurrent);
    const bool is_frozen_separator = (table->FreezeColumnsCount == order_n + 1);
    if (is_resized || is_hovered)
        return RblxUI::GetColorU32(is_resized ? RblxUICol_SeparatorActive : RblxUICol_SeparatorHovered);
    if (is_frozen_separator || (table->Flags & (RblxUITableFlags_NoBordersInBody | RblxUITableFlags_NoBordersInBodyUntilResize)))
        return table->BorderColorStrong;
    return table->BorderColorLight;
}

// FIXME-TABLE: This is a mess, need to redesign how we render borders (as some are also done in TableEndRow)
void RblxUI::TableDrawBorders(RblxUITable* table)
{
    RblxUIWindow* inner_window = table->InnerWindow;
    if (!table->OuterWindow->ClipRect.Overlaps(table->OuterRect))
        return;

    RblxDrawList* inner_drawlist = inner_window->DrawList;
    table->DrawSplitter->SetCurrentChannel(inner_drawlist, TABLE_DRAW_CHANNEL_BG0);
    inner_drawlist->PushClipRect(table->Bg0ClipRectForDrawCmd.Min, table->Bg0ClipRectForDrawCmd.Max, false);

    // Draw inner border and resizing feedback
    RblxUITableInstanceData* table_instance = TableGetInstanceData(table, table->InstanceCurrent);
    const float border_size = TABLE_BORDER_SIZE;
    const float draw_y1 = RblxMax(table->InnerRect.Min.y, (table->FreezeRowsCount >= 1 ? table->InnerRect.Min.y : table->WorkRect.Min.y) + table->AngledHeadersHeight) + ((table->Flags & RblxUITableFlags_BordersOuterH) ? 1.0f : 0.0f);
    const float draw_y2_body = table->InnerRect.Max.y;
    const float draw_y2_head = table->IsUsingHeaders ? RblxMin(table->InnerRect.Max.y, (table->FreezeRowsCount >= 1 ? table->InnerRect.Min.y : table->WorkRect.Min.y) + table_instance->LastTopHeadersRowHeight) : draw_y1;
    if (table->Flags & RblxUITableFlags_BordersInnerV)
    {
        for (int order_n = 0; order_n < table->ColumnsCount; order_n++)
        {
            if (!RBLXUI_BITARRAY_TESTBIT(table->EnabledMaskByDisplayOrder, order_n))
                continue;

            const int column_n = table->DisplayOrderToIndex[order_n];
            RblxUITableColumn* column = &table->Columns[column_n];
            const bool is_hovered = (table->HoveredColumnBorder == column_n);
            const bool is_resized = (table->ResizedColumn == column_n) && (table->InstanceInteracted == table->InstanceCurrent);
            const bool is_resizable = (column->Flags & (RblxUITableColumnFlags_NoResize | RblxUITableColumnFlags_NoDirectResize_)) == 0;
            const bool is_frozen_separator = (table->FreezeColumnsCount == order_n + 1);
            if (column->MaxX > table->InnerClipRect.Max.x && !is_resized)
                continue;

            // Decide whether right-most column is visible
            if (column->NextEnabledColumn == -1 && !is_resizable)
                if ((table->Flags & RblxUITableFlags_SizingMask_) != RblxUITableFlags_SizingFixedSame || (table->Flags & RblxUITableFlags_NoHostExtendX))
                    continue;
            if (column->MaxX <= column->ClipRect.Min.x) // FIXME-TABLE FIXME-STYLE: Assume BorderSize==1, this is problematic if we want to increase the border size..
                continue;

            // Draw in outer window so right-most column won't be clipped
            float draw_y2 = draw_y2_head;
            if (is_frozen_separator)
                draw_y2 = draw_y2_body;
            else if ((table->Flags & RblxUITableFlags_NoBordersInBodyUntilResize) != 0 && (is_hovered || is_resized))
                draw_y2 = draw_y2_body;
            else if ((table->Flags & (RblxUITableFlags_NoBordersInBodyUntilResize | RblxUITableFlags_NoBordersInBody)) == 0)
                draw_y2 = draw_y2_body;
            if (draw_y2 > draw_y1)
                inner_drawlist->AddLineV(column->MaxX, draw_y1, draw_y2, TableGetColumnBorderCol(table, order_n, column_n), border_size);
        }
    }

    // Draw outer border
    // FIXME: could use AddRect or explicit VLine/HLine helper?
    if (table->Flags & RblxUITableFlags_BordersOuter)
    {
        // Display outer border offset by 1 which is a simple way to display it without adding an extra draw call
        // (Without the offset, in outer_window it would be rendered behind cells, because child windows are above their
        // parent. In inner_window, it won't reach out over scrollbars. Another weird solution would be to display part
        // of it in inner window, and the part that's over scrollbars in the outer window..)
        // Either solution currently won't allow us to use a larger border size: the border would clipped.
        const RblxRect outer_border = table->OuterRect;
        const RblxU32 outer_col = table->BorderColorStrong;
        if ((table->Flags & RblxUITableFlags_BordersOuter) == RblxUITableFlags_BordersOuter)
        {
            inner_drawlist->AddRect(outer_border.Min, outer_border.Max, outer_col, 0.0f, border_size);
        }
        else if (table->Flags & RblxUITableFlags_BordersOuterV)
        {
            inner_drawlist->AddLineV(outer_border.Min.x, outer_border.Min.y, outer_border.Max.y, outer_col, border_size);
            inner_drawlist->AddLineV(outer_border.Max.x, outer_border.Min.y, outer_border.Max.y, outer_col, border_size);
        }
        else if (table->Flags & RblxUITableFlags_BordersOuterH)
        {
            inner_drawlist->AddLineH(outer_border.Min.x, outer_border.Max.x, outer_border.Min.y, outer_col, border_size);
            inner_drawlist->AddLineH(outer_border.Min.x, outer_border.Max.x, outer_border.Max.y, outer_col, border_size);
        }
    }
    if ((table->Flags & RblxUITableFlags_BordersInnerH) && table->RowPosY2 < table->OuterRect.Max.y)
    {
        // Draw bottom-most row border between it is above outer border.
        const float border_y = table->RowPosY2;
        if (border_y >= table->BgClipRect.Min.y && border_y < table->BgClipRect.Max.y)
            inner_drawlist->AddLineH(table->BorderX1, table->BorderX2, border_y, table->BorderColorLight, border_size);
    }

    inner_drawlist->PopClipRect();
}

//-------------------------------------------------------------------------
// [SECTION] Tables: Sorting
//-------------------------------------------------------------------------
// - TableGetSortSpecs()
// - TableFixColumnSortDirection() [Internal]
// - TableGetColumnNextSortDirection() [Internal]
// - TableSetColumnSortDirection() [Internal]
// - TableSortSpecsSanitize() [Internal]
// - TableSortSpecsBuild() [Internal]
//-------------------------------------------------------------------------

// Return NULL if no sort specs (most often when RblxUITableFlags_Sortable is not set)
// When 'sort_specs->SpecsDirty == true' you should sort your data. It will be true when sorting specs have
// changed since last call, or the first time. Make sure to set 'SpecsDirty = false' after sorting,
// else you may wastefully sort your data every frame!
// Lifetime: don't hold on this pointer over multiple frames or past any subsequent call to BeginTable()!
RblxUITableSortSpecs* RblxUI::TableGetSortSpecs()
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    if (table == NULL || !(table->Flags & RblxUITableFlags_Sortable))
        return NULL;

    // Require layout (in case TableHeadersRow() hasn't been called) as it may alter IsSortSpecsDirty in some paths.
    if (!table->IsLayoutLocked)
        TableUpdateLayout(table);

    TableSortSpecsBuild(table);
    return &table->SortSpecs;
}

static inline RblxUISortDirection TableGetColumnAvailSortDirection(RblxUITableColumn* column, int n)
{
    RBLXUI_ASSERT(n < column->SortDirectionsAvailCount);
    return (RblxUISortDirection)((column->SortDirectionsAvailList >> (n << 1)) & 0x03);
}

// Fix sort direction if currently set on a value which is unavailable (e.g. activating NoSortAscending/NoSortDescending)
void RblxUI::TableFixColumnSortDirection(RblxUITable* table, RblxUITableColumn* column)
{
    if (column->SortOrder == -1 || (column->SortDirectionsAvailMask & (1 << column->SortDirection)) != 0)
        return;
    column->SortDirection = (RblxU8)TableGetColumnAvailSortDirection(column, 0);
    table->IsSortSpecsDirty = true;
}

// Calculate next sort direction that would be set after clicking the column
// - If the PreferSortDescending flag is set, we will default to a Descending direction on the first click.
// - Note that the PreferSortAscending flag is never checked, it is essentially the default and therefore a no-op.
RBLXUI_STATIC_ASSERT(RblxUISortDirection_None == 0 && RblxUISortDirection_Ascending == 1 && RblxUISortDirection_Descending == 2);
RblxUISortDirection RblxUI::TableGetColumnNextSortDirection(RblxUITableColumn* column)
{
    RBLXUI_ASSERT(column->SortDirectionsAvailCount > 0);
    if (column->SortOrder == -1)
        return TableGetColumnAvailSortDirection(column, 0);
    for (int n = 0; n < 3; n++)
        if (column->SortDirection == TableGetColumnAvailSortDirection(column, n))
            return TableGetColumnAvailSortDirection(column, (n + 1) % column->SortDirectionsAvailCount);
    RBLXUI_ASSERT(0);
    return RblxUISortDirection_None;
}

// Note that the NoSortAscending/NoSortDescending flags are processed in TableSortSpecsSanitize(), and they may change/revert
// the value of SortDirection. We could technically also do it here but it would be unnecessary and duplicate code.
void RblxUI::TableSetColumnSortDirection(int column_n, RblxUISortDirection sort_direction, bool append_to_sort_specs)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;

    if (!(table->Flags & RblxUITableFlags_SortMulti))
        append_to_sort_specs = false;
    if (!(table->Flags & RblxUITableFlags_SortTristate))
        RBLXUI_ASSERT(sort_direction != RblxUISortDirection_None);

    RblxUITableColumnIdx sort_order_max = 0;
    if (append_to_sort_specs)
        for (int other_column_n = 0; other_column_n < table->ColumnsCount; other_column_n++)
            sort_order_max = RblxMax(sort_order_max, table->Columns[other_column_n].SortOrder);

    RblxUITableColumn* column = &table->Columns[column_n];
    column->SortDirection = (RblxU8)sort_direction;
    if (column->SortDirection == RblxUISortDirection_None)
        column->SortOrder = -1;
    else if (column->SortOrder == -1 || !append_to_sort_specs)
        column->SortOrder = append_to_sort_specs ? sort_order_max + 1 : 0;

    for (int other_column_n = 0; other_column_n < table->ColumnsCount; other_column_n++)
    {
        RblxUITableColumn* other_column = &table->Columns[other_column_n];
        if (other_column != column && !append_to_sort_specs)
            other_column->SortOrder = -1;
        TableFixColumnSortDirection(table, other_column);
    }
    table->IsSettingsDirty = true;
    table->IsSortSpecsDirty = true;
}

void RblxUI::TableSortSpecsSanitize(RblxUITable* table)
{
    RBLXUI_ASSERT(table->Flags & RblxUITableFlags_Sortable);

    // Clear SortOrder from hidden column and verify that there's no gap or duplicate.
    int sort_order_count = 0;
    RblxU64 sort_order_mask = 0x00;
    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
    {
        RblxUITableColumn* column = &table->Columns[column_n];
        if (column->SortOrder != -1 && !column->IsEnabled)
            column->SortOrder = -1;
        if (column->SortOrder == -1)
            continue;
        sort_order_count++;
        sort_order_mask |= ((RblxU64)1 << column->SortOrder);
        RBLXUI_ASSERT(sort_order_count < (int)sizeof(sort_order_mask) * 8);
    }

    const bool need_fix_linearize = ((RblxU64)1 << sort_order_count) != (sort_order_mask + 1);
    const bool need_fix_single_sort_order = (sort_order_count > 1) && !(table->Flags & RblxUITableFlags_SortMulti);
    if (need_fix_linearize || need_fix_single_sort_order)
    {
        RblxU64 fixed_mask = 0x00;
        for (int sort_n = 0; sort_n < sort_order_count; sort_n++)
        {
            // Fix: Rewrite sort order fields if needed so they have no gap or duplicate.
            // (e.g. SortOrder 0 disappeared, SortOrder 1..2 exists --> rewrite then as SortOrder 0..1)
            int column_with_smallest_sort_order = -1;
            for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
                if ((fixed_mask & ((RblxU64)1 << (RblxU64)column_n)) == 0 && table->Columns[column_n].SortOrder != -1)
                    if (column_with_smallest_sort_order == -1 || table->Columns[column_n].SortOrder < table->Columns[column_with_smallest_sort_order].SortOrder)
                        column_with_smallest_sort_order = column_n;
            RBLXUI_ASSERT(column_with_smallest_sort_order != -1);
            fixed_mask |= ((RblxU64)1 << column_with_smallest_sort_order);
            table->Columns[column_with_smallest_sort_order].SortOrder = (RblxUITableColumnIdx)sort_n;

            // Fix: Make sure only one column has a SortOrder if RblxUITableFlags_MultiSortable is not set.
            if (need_fix_single_sort_order)
            {
                sort_order_count = 1;
                for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
                    if (column_n != column_with_smallest_sort_order)
                        table->Columns[column_n].SortOrder = -1;
                break;
            }
        }
    }

    // Fallback default sort order (if no column with the RblxUITableColumnFlags_DefaultSort flag)
    if (sort_order_count == 0 && !(table->Flags & RblxUITableFlags_SortTristate))
        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
        {
            RblxUITableColumn* column = &table->Columns[column_n];
            if (column->IsEnabled && !(column->Flags & RblxUITableColumnFlags_NoSort))
            {
                sort_order_count = 1;
                column->SortOrder = 0;
                column->SortDirection = (RblxU8)TableGetColumnAvailSortDirection(column, 0);
                break;
            }
        }

    table->SortSpecsCount = (RblxUITableColumnIdx)sort_order_count;
}

void RblxUI::TableSortSpecsBuild(RblxUITable* table)
{
    bool dirty = table->IsSortSpecsDirty;
    if (dirty)
    {
        TableSortSpecsSanitize(table);
        table->SortSpecsMulti.resize(table->SortSpecsCount <= 1 ? 0 : table->SortSpecsCount);
        table->SortSpecs.SpecsDirty = true; // Mark as dirty for user
        table->IsSortSpecsDirty = false; // Mark as not dirty for us
    }

    // Write output
    // May be able to move all SortSpecs data from table (48 bytes) to RblxUITableTempData if we decide to write it back on every BeginTable()
    RblxUITableColumnSortSpecs* sort_specs = (table->SortSpecsCount == 0) ? NULL : (table->SortSpecsCount == 1) ? &table->SortSpecsSingle : table->SortSpecsMulti.Data;
    if (dirty && sort_specs != NULL)
        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
        {
            RblxUITableColumn* column = &table->Columns[column_n];
            if (column->SortOrder == -1)
                continue;
            RBLXUI_ASSERT(column->SortOrder < table->SortSpecsCount);
            RblxUITableColumnSortSpecs* sort_spec = &sort_specs[column->SortOrder];
            sort_spec->ColumnUserID = column->UserID;
            sort_spec->ColumnIndex = (RblxUITableColumnIdx)column_n;
            sort_spec->SortOrder = (RblxUITableColumnIdx)column->SortOrder;
            sort_spec->SortDirection = (RblxUISortDirection)column->SortDirection;
        }

    table->SortSpecs.Specs = sort_specs;
    table->SortSpecs.SpecsCount = table->SortSpecsCount;
}

//-------------------------------------------------------------------------
// [SECTION] Tables: Headers
//-------------------------------------------------------------------------
// - TableGetHeaderRowHeight() [Internal]
// - TableGetHeaderAngledMaxLabelWidth() [Internal]
// - TableHeadersRow()
// - TableHeader()
// - TableAngledHeadersRow()
// - TableAngledHeadersRowEx() [Internal]
//-------------------------------------------------------------------------

float RblxUI::TableGetHeaderRowHeight()
{
    // Caring for a minor edge case:
    // Calculate row height, for the unlikely case that some labels may be taller than others.
    // If we didn't do that, uneven header height would highlight but smaller one before the tallest wouldn't catch input for all height.
    // In your custom header row you may omit this all together and just call TableNextRow() without a height...
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    float row_height = g.FontSize;
    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
        if (RBLXUI_BITARRAY_TESTBIT(table->EnabledMaskByIndex, column_n))
            if ((table->Columns[column_n].Flags & RblxUITableColumnFlags_NoHeaderLabel) == 0)
                row_height = RblxMax(row_height, CalcTextSize(TableGetColumnName(table, column_n)).y);
    return row_height + g.Style.CellPadding.y * 2.0f;
}

float RblxUI::TableGetHeaderAngledMaxLabelWidth()
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    float width = 0.0f;
    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
        if (RBLXUI_BITARRAY_TESTBIT(table->EnabledMaskByIndex, column_n))
            if (table->Columns[column_n].Flags & RblxUITableColumnFlags_AngledHeader)
                width = RblxMax(width, CalcTextSize(TableGetColumnName(table, column_n), NULL, true).x);
    return width + g.Style.CellPadding.y * 2.0f; // Swap padding
}

// [Public] This is a helper to output TableHeader() calls based on the column names declared in TableSetupColumn().
// The intent is that advanced users willing to create customized headers would not need to use this helper
// and can create their own! For example: TableHeader() may be preceded by Checkbox() or other custom widgets.
// See 'Demo->Tables->Custom headers' for a demonstration of implementing a custom version of this.
// This code is intentionally written to not make much use of internal functions, to give you better direction
// if you need to write your own.
// FIXME-TABLE: TableOpenContextMenu() and TableGetHeaderRowHeight() are not public.
void RblxUI::TableHeadersRow()
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    RBLXUI_ASSERT_USER_ERROR_RET(table != NULL, "Call should only be done while in BeginTable() scope!");

    // Call layout if not already done. This is automatically done by TableNextRow: we do it here _only_ to make
    // it easier to debug-step in TableUpdateLayout(). Your own version of this function doesn't need this.
    if (!table->IsLayoutLocked)
        TableUpdateLayout(table);

    // Open row
    const float row_height = TableGetHeaderRowHeight();
    TableNextRow(RblxUITableRowFlags_Headers, row_height);
    const float row_y1 = GetCursorScreenPos().y;
    if (table->HostSkipItems) // Merely an optimization, you may skip in your own code.
        return;

    const int columns_count = TableGetColumnCount();
    for (int column_n = 0; column_n < columns_count; column_n++)
    {
        if (!TableSetColumnIndex(column_n) && table->LastHeldHeaderColumn != column_n)
            continue;

        // Push an id to allow empty/unnamed headers. This is also idiomatic as it ensure there is a consistent ID path to access columns (for e.g. automation)
        const char* name = (TableGetColumnFlags(column_n) & RblxUITableColumnFlags_NoHeaderLabel) ? "" : TableGetColumnName(column_n);
        PushID(column_n);
        TableHeader(name);
        PopID();
    }

    // Allow opening popup from the right-most section after the last column.
    RblxVec2 mouse_pos = RblxUI::GetMousePos();
    if (IsMouseReleased(1) && TableGetHoveredColumn() == columns_count)
        if (mouse_pos.y >= row_y1 && mouse_pos.y < row_y1 + row_height)
            TableOpenContextMenu(columns_count); // Will open a non-column-specific popup.
}

// Emit a column header (text + optional sort order)
// We cpu-clip text here so that all columns headers can be merged into a same draw call.
// Note that because of how we cpu-clip and display sorting indicators, you _cannot_ use SameLine() after a TableHeader()
void RblxUI::TableHeader(const char* label)
{
    RblxUIContext& g = *GRblxUI;
    RblxUIWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return;

    RblxUITable* table = g.CurrentTable;
    RBLXUI_ASSERT_USER_ERROR_RET(table != NULL, "Call should only be done while in BeginTable() scope!");
    RBLXUI_ASSERT(table->CurrentColumn != -1);
    const int column_n = table->CurrentColumn;
    RblxUITableColumn* column = &table->Columns[column_n];

    // Label
    if (label == NULL)
        label = "";
    const char* label_end = FindRenderedTextEnd(label);
    RblxVec2 label_size = CalcTextSize(label, label_end, false);
    RblxVec2 label_pos = window->DC.CursorPos;

    // If we already got a row height, there's use that.
    // FIXME-TABLE: Padding problem if the correct outer-padding CellBgRect strays off our ClipRect?
    RblxRect cell_r = TableGetCellBgRect(table, column_n);
    float label_height = RblxMax(label_size.y, table->RowMinHeight - table->RowCellPaddingY * 2.0f);

    // Calculate ideal size for sort order arrow
    float w_arrow = 0.0f;
    float w_sort_text = 0.0f;
    bool sort_arrow = false;
    char sort_order_suf[4] = "";
    const float ARROW_SCALE = 0.65f;
    if ((table->Flags & RblxUITableFlags_Sortable) && !(column->Flags & RblxUITableColumnFlags_NoSort))
    {
        w_arrow = RblxTrunc(g.FontSize * ARROW_SCALE + g.Style.FramePadding.x);
        if (column->SortOrder != -1)
            sort_arrow = true;
        if (column->SortOrder > 0)
        {
            RblxFormatString(sort_order_suf, RBLXUI_COUNTOF(sort_order_suf), "%d", column->SortOrder + 1);
            w_sort_text = g.Style.ItemInnerSpacing.x + CalcTextSize(sort_order_suf).x;
        }
    }

    // We feed our unclipped width to the column without writing on CursorMaxPos, so that column is still considered for merging.
    float max_pos_x = label_pos.x + label_size.x + w_sort_text + w_arrow;
    column->ContentMaxXHeadersUsed = RblxMax(column->ContentMaxXHeadersUsed, sort_arrow ? cell_r.Max.x : RblxMin(max_pos_x, cell_r.Max.x));
    column->ContentMaxXHeadersIdeal = RblxMax(column->ContentMaxXHeadersIdeal, max_pos_x);

    // Keep header highlighted when context menu is open.
    RblxUIID id = window->GetID(label);
    RblxRect bb(cell_r.Min.x, cell_r.Min.y, cell_r.Max.x, RblxMax(cell_r.Max.y, cell_r.Min.y + label_height + g.Style.CellPadding.y * 2.0f));
    ItemSize(RblxVec2(0.0f, label_height)); // Don't declare unclipped width, it'll be fed ContentMaxPosHeadersIdeal
    if (!ItemAdd(bb, id))
        return;

    //GetForegroundDrawList()->AddRect(cell_r.Min, cell_r.Max, RBLXUI_COL32(255, 0, 0, 255)); // [DEBUG]
    //GetForegroundDrawList()->AddRect(bb.Min, bb.Max, RBLXUI_COL32(255, 0, 0, 255)); // [DEBUG]

    // Using AllowOverlap mode because we cover the whole cell, and we want user to be able to submit subsequent items.
    const bool highlight = (table->HighlightColumnHeader == column_n);
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, RblxUIButtonFlags_AllowOverlap);
    if (held || hovered || highlight)
    {
        const RblxU32 col = GetColorU32(held ? RblxUICol_HeaderActive : hovered ? RblxUICol_HeaderHovered : RblxUICol_Header);
        //RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
        TableSetBgColor(RblxUITableBgTarget_CellBg, col, table->CurrentColumn);
    }
    else
    {
        // Submit single cell bg color in the case we didn't submit a full header row
        if ((table->RowFlags & RblxUITableRowFlags_Headers) == 0)
            TableSetBgColor(RblxUITableBgTarget_CellBg, GetColorU32(RblxUICol_TableHeaderBg), table->CurrentColumn);
    }
    RenderNavCursor(bb, id, RblxUINavRenderCursorFlags_Compact | RblxUINavRenderCursorFlags_NoRounding);
    if (held)
        table->HeldHeaderColumn = (RblxUITableColumnIdx)column_n;
    window->DC.CursorPos.y -= g.Style.ItemSpacing.y * 0.5f;

    // Drag and drop to re-order columns.
    // FIXME-TABLE: Scroll request while reordering a column and it lands out of the scrolling zone.
    if (held && (table->Flags & RblxUITableFlags_Reorderable) && IsMouseDragging(0) && !g.DragDropActive)
    {
        // - While moving a column it will jump on the other side of the mouse, so we also test for MouseDelta.x
        // - We need to handle reordering across hidden columns.
        //   In the configuration below, moving C to the right of E will lead to:
        //      ... C [D] E  --->  ... [D] E  C   (Column name/index)
        //      ... 2  3  4        ...  2  3  4   (Display order)
        // - The other constraints are enforced by TableQueueSetColumnDisplayOrder() which might early out.
        table->InstanceInteracted = table->InstanceCurrent;
        if (g.IO.MouseDelta.x < 0.0f && g.IO.MousePos.x < cell_r.Min.x)
            if (RblxUITableColumn* prev_column = (column->PrevEnabledColumn != -1) ? &table->Columns[column->PrevEnabledColumn] : NULL)
                TableQueueSetColumnDisplayOrder(table, column_n, prev_column->DisplayOrder);
        if (g.IO.MouseDelta.x > 0.0f && g.IO.MousePos.x > cell_r.Max.x)
            if (RblxUITableColumn* next_column = (column->NextEnabledColumn != -1) ? &table->Columns[column->NextEnabledColumn] : NULL)
                TableQueueSetColumnDisplayOrder(table, column_n, next_column->DisplayOrder);
    }

    // Sort order arrow
    const float ellipsis_max = RblxMax(cell_r.Max.x - w_arrow - w_sort_text, label_pos.x);
    if ((table->Flags & RblxUITableFlags_Sortable) && !(column->Flags & RblxUITableColumnFlags_NoSort))
    {
        if (column->SortOrder != -1)
        {
            float x = RblxMax(cell_r.Min.x, cell_r.Max.x - w_arrow - w_sort_text);
            float y = label_pos.y;
            if (column->SortOrder > 0)
            {
                PushStyleColor(RblxUICol_Text, GetColorU32(RblxUICol_Text, 0.70f));
                RenderText(RblxVec2(x + g.Style.ItemInnerSpacing.x, y), sort_order_suf);
                PopStyleColor();
                x += w_sort_text;
            }
            RenderArrow(window->DrawList, RblxVec2(x, y), GetColorU32(RblxUICol_Text), column->SortDirection == RblxUISortDirection_Ascending ? RblxUIDir_Up : RblxUIDir_Down, ARROW_SCALE);
        }

        // Handle clicking on column header to adjust Sort Order
        if (pressed && table->ReorderColumn != column_n)
        {
            RblxUISortDirection sort_direction = TableGetColumnNextSortDirection(column);
            TableSetColumnSortDirection(column_n, sort_direction, g.IO.KeyShift);
        }
    }

    // Render clipped label. Clipping here ensure that in the majority of situations, all our header cells will
    // be merged into a single draw call.
    //window->DrawList->AddCircleFilled(RblxVec2(ellipsis_max, label_pos.y), 40, RBLXUI_COL32_WHITE);
    RenderTextEllipsis(window->DrawList, label_pos, RblxVec2(ellipsis_max, bb.Max.y), ellipsis_max, label, label_end, &label_size);

    const bool text_clipped = label_size.x > (ellipsis_max - label_pos.x);
    if (text_clipped && hovered && g.ActiveId == 0)
        SetItemTooltip("%.*s", (int)(label_end - label), label);

    // We don't use BeginPopupContextItem() because we want the popup to stay up even after the column is hidden
    if (IsPopupOpenRequestForItem(RblxUIPopupFlags_None, id))
        TableOpenContextMenu(column_n);

    RBLXUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
}

// Unlike TableHeadersRow() it is not expected that you can reimplement or customize this with custom widgets.
// FIXME: No hit-testing/button on the angled header.
void RblxUI::TableAngledHeadersRow()
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    RblxUITableTempData* temp_data = table->TempData;
    temp_data->AngledHeadersRequests.resize(0);
    temp_data->AngledHeadersRequests.reserve(table->ColumnsEnabledCount);

    // Which column needs highlight?
    const RblxUIID row_id = GetID("##AngledHeaders");
    RblxUITableInstanceData* table_instance = TableGetInstanceData(table, table->InstanceCurrent);
    int highlight_column_n = (table->LastHeldHeaderColumn != -1) ? table->LastHeldHeaderColumn : table->HighlightColumnHeader;
    if (highlight_column_n == -1 && table->HoveredColumnBody != -1)
        if (table_instance->HoveredRowLast == 0 && table->HoveredColumnBorder == -1 && (g.ActiveId == 0 || g.ActiveId == row_id || (table->IsActiveIdInTable || g.DragDropActive)))
            highlight_column_n = table->HoveredColumnBody;

    // Build up request
    RblxU32 col_header_bg = GetColorU32(RblxUICol_TableHeaderBg);
    RblxU32 col_text = GetColorU32(RblxUICol_Text);
    for (int order_n = 0; order_n < table->ColumnsCount; order_n++)
        if (RBLXUI_BITARRAY_TESTBIT(table->EnabledMaskByDisplayOrder, order_n))
        {
            const int column_n = table->DisplayOrderToIndex[order_n];
            RblxUITableColumn* column = &table->Columns[column_n];
            if ((column->Flags & RblxUITableColumnFlags_AngledHeader) == 0) // Note: can't rely on RblxUITableColumnFlags_IsVisible test here.
                continue;
            RblxUITableHeaderData request = { (RblxUITableColumnIdx)column_n, col_text, col_header_bg, (column_n == highlight_column_n) ? GetColorU32(RblxUICol_Header) : 0 };
            temp_data->AngledHeadersRequests.push_back(request);
        }

    // Render row
    TableAngledHeadersRowEx(row_id, g.Style.TableAngledHeadersAngle, 0.0f, temp_data->AngledHeadersRequests.Data, temp_data->AngledHeadersRequests.Size);
}

// Important: data must be fed left to right
void RblxUI::TableAngledHeadersRowEx(RblxUIID row_id, float angle, float max_label_width, const RblxUITableHeaderData* data, int data_count)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    RblxUIWindow* window = g.CurrentWindow;
    RblxDrawList* draw_list = window->DrawList;
    RBLXUI_ASSERT_USER_ERROR_RET(table != NULL, "Call should only be done while in BeginTable() scope!");
    RBLXUI_ASSERT(table->CurrentRow == -1 && "Must be first row");

    if (max_label_width == 0.0f)
        max_label_width = TableGetHeaderAngledMaxLabelWidth();

    // Angle argument expressed in (-RBLXUI_PI/2 .. +RBLXUI_PI/2) as it is easier to think about for user.
    const bool flip_label = (angle < 0.0f);
    angle -= RBLXUI_PI * 0.5f;
    const float cos_a = RblxCos(angle);
    const float sin_a = RblxSin(angle);
    const float label_cos_a = flip_label ? RblxCos(angle + RBLXUI_PI) : cos_a;
    const float label_sin_a = flip_label ? RblxSin(angle + RBLXUI_PI) : sin_a;
    const RblxVec2 unit_right = RblxVec2(cos_a, sin_a);

    // Calculate our base metrics and set angled headers data _before_ the first call to TableNextRow()
    // FIXME-STYLE: Would it be better for user to submit 'max_label_width' or 'row_height' ? One can be derived from the other.
    const float header_height = g.FontSize + g.Style.CellPadding.x * 2.0f;
    const float row_height = RblxTrunc(RblxFabs(RblxRotate(RblxVec2(max_label_width, flip_label ? +header_height : -header_height), cos_a, sin_a).y));
    table->AngledHeadersHeight = row_height;
    table->AngledHeadersSlope = (sin_a != 0.0f) ? (cos_a / sin_a) : 0.0f;
    const RblxVec2 header_angled_vector = unit_right * (row_height / -sin_a); // vector from bottom-left to top-left, and from bottom-right to top-right

    // Declare row, override and draw our own background
    // FIXME-TABLE: Generally broken when overlapping frozen columns limit.
    TableNextRow(RblxUITableRowFlags_Headers, row_height);
    TableNextColumn();
    const RblxRect row_r(table->WorkRect.Min.x, table->BgClipRect.Min.y, table->WorkRect.Max.x, table->RowPosY2);
    table->DrawSplitter->SetCurrentChannel(draw_list, TABLE_DRAW_CHANNEL_BG0);
    float clip_rect_min_x = table->BgClipRect.Min.x;
    if (table->FreezeColumnsCount > 0)
        clip_rect_min_x = RblxMax(clip_rect_min_x, table->Columns[table->DisplayOrderToIndex[table->FreezeColumnsCount - 1]].MaxX);
    TableSetBgColor(RblxUITableBgTarget_RowBg0, 0); // Cancel
    PushClipRect(table->BgClipRect.Min, table->BgClipRect.Max, false); // Span all columns
    draw_list->AddRectFilled(RblxVec2(table->BgClipRect.Min.x, row_r.Min.y), RblxVec2(table->BgClipRect.Max.x, row_r.Max.y), GetColorU32(RblxUICol_TableHeaderBg, 0.25f)); // FIXME-STYLE: Change row background with an arbitrary color.
    PushClipRect(RblxVec2(clip_rect_min_x, table->BgClipRect.Min.y), table->BgClipRect.Max, true); // Span all columns

    ButtonBehavior(row_r, row_id, NULL, NULL);
    KeepAliveID(row_id);

    const float ascent_scaled = g.FontBaked->Ascent * g.FontBakedScale; // FIXME: Standardize those scaling factors better
    const float line_off_for_ascent_x = (RblxMax((g.FontSize - ascent_scaled) * 0.5f, 0.0f) / -sin_a) * (flip_label ? -1.0f : 1.0f);
    const RblxVec2 padding = g.Style.CellPadding; // We will always use swapped component
    const RblxVec2 align = g.Style.TableAngledHeadersTextAlign;

    // Draw background and labels in first pass, then all borders.
    float max_x = -FLT_MAX;
    for (int pass = 0; pass < 2; pass++)
        for (int order_n = 0; order_n < data_count; order_n++)
        {
            const RblxUITableHeaderData* request = &data[order_n];
            const int column_n = request->Index;
            RblxUITableColumn* column = &table->Columns[column_n];

            RblxVec2 bg_shape[4];
            bg_shape[0] = RblxVec2(column->MaxX, row_r.Max.y);
            bg_shape[1] = RblxVec2(column->MinX, row_r.Max.y);
            bg_shape[2] = bg_shape[1] + header_angled_vector;
            bg_shape[3] = bg_shape[0] + header_angled_vector;
            if (pass == 0)
            {
                // Draw shape
                draw_list->AddQuadFilled(bg_shape[0], bg_shape[1], bg_shape[2], bg_shape[3], request->BgColor0);
                draw_list->AddQuadFilled(bg_shape[0], bg_shape[1], bg_shape[2], bg_shape[3], request->BgColor1); // Optional highlight
                max_x = RblxMax(max_x, bg_shape[3].x);

                // Draw label
                // - First draw at an offset where RenderTextXXX() function won't meddle with applying current ClipRect, then transform to final offset.
                // - Handle multiple lines manually, as we want each lines to follow on the horizontal border, rather than see a whole block rotated.
                const char* label_name = TableGetColumnName(table, column_n);
                const char* label_name_end = FindRenderedTextEnd(label_name);
                const float line_off_step_x = (g.FontSize / -sin_a);
                const int label_lines = RblxTextCountLines(label_name, label_name_end);

                // Left<>Right alignment
                float line_off_curr_x = flip_label ? (label_lines - 1) * line_off_step_x : 0.0f;
                float line_off_for_align_x = RblxFloor(RblxMax((((column->MaxX - column->MinX) - padding.x * 2.0f) - (label_lines * line_off_step_x)), 0.0f) * align.x);
                line_off_curr_x += line_off_for_align_x - line_off_for_ascent_x;

                // Register header width
                column->ContentMaxXHeadersUsed = column->ContentMaxXHeadersIdeal = column->WorkMinX + RblxCeil(label_lines * line_off_step_x - line_off_for_align_x);

                while (label_name < label_name_end)
                {
                    const char* label_name_eol = strchr(label_name, '\n');
                    if (label_name_eol == NULL)
                        label_name_eol = label_name_end;

                    // FIXME: Individual line clipping for right-most column is broken for negative angles.
                    RblxVec2 label_size = CalcTextSize(label_name, label_name_eol);
                    float clip_width = max_label_width - padding.y; // Using padding.y*2.0f would be symmetrical but hide more text.
                    float clip_height = RblxMin(label_size.y, column->ClipRect.Max.x - column->WorkMinX - line_off_curr_x);
                    RblxRect clip_r(window->ClipRect.Min, window->ClipRect.Min + RblxVec2(clip_width, clip_height));
                    int vtx_idx_begin = draw_list->_VtxCurrentIdx;
                    PushStyleColor(RblxUICol_Text, request->TextColor);
                    RenderTextEllipsis(draw_list, clip_r.Min, clip_r.Max, clip_r.Max.x, label_name, label_name_eol, &label_size);
                    PopStyleColor();
                    int vtx_idx_end = draw_list->_VtxCurrentIdx;

                    // Up<>Down alignment
                    const float available_space = RblxMax(clip_width - label_size.x + RblxAbs(padding.x * cos_a) * 2.0f - RblxAbs(padding.y * sin_a) * 2.0f, 0.0f);
                    const float vertical_offset = available_space * align.y * (flip_label ? -1.0f : 1.0f);

                    // Rotate and offset label
                    RblxVec2 pivot_in = RblxVec2(window->ClipRect.Min.x - vertical_offset, window->ClipRect.Min.y + label_size.y);
                    RblxVec2 pivot_out = RblxVec2(column->WorkMinX, row_r.Max.y);
                    line_off_curr_x += flip_label ? -line_off_step_x : line_off_step_x;
                    pivot_out += unit_right * padding.y;
                    if (flip_label)
                        pivot_out += unit_right * (clip_width - RblxMax(0.0f, clip_width - label_size.x));
                    pivot_out.x += flip_label ? line_off_curr_x + line_off_step_x : line_off_curr_x;
                    ShadeVertsTransformPos(draw_list, vtx_idx_begin, vtx_idx_end, pivot_in, label_cos_a, label_sin_a, pivot_out); // Rotate and offset
                    //if (g.IO.KeyShift) { RblxDrawList* fg_dl = GetForegroundDrawList(); vtx_idx_begin = fg_dl->_VtxCurrentIdx; fg_dl->AddRect(clip_r.Min, clip_r.Max, RBLXUI_COL32(0, 255, 0, 255), 0.0f, 0, 1.0f); ShadeVertsTransformPos(fg_dl, vtx_idx_begin, fg_dl->_VtxCurrentIdx, pivot_in, label_cos_a, label_sin_a, pivot_out); }

                    label_name = label_name_eol + 1;
                }
            }
            if (pass == 1)
            {
                // Draw border
                draw_list->AddLine(bg_shape[0], bg_shape[3], TableGetColumnBorderCol(table, order_n, column_n));
            }
        }
    PopClipRect();
    PopClipRect();
    table->TempData->AngledHeadersExtraWidth = RblxMax(0.0f, max_x - table->Columns[table->RightMostEnabledColumn].MaxX);
}

//-------------------------------------------------------------------------
// [SECTION] Tables: Context Menu
//-------------------------------------------------------------------------
// - TableOpenContextMenu() [Internal]
// - TableBeginContextMenuPopup() [Internal]
// - TableDrawDefaultContextMenu() [Internal]
//-------------------------------------------------------------------------

// Use -1 to open menu not specific to a given column.
void RblxUI::TableOpenContextMenu(int column_n)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITable* table = g.CurrentTable;
    if (column_n == -1 && table->CurrentColumn != -1)   // When called within a column automatically use this one (for consistency)
        column_n = table->CurrentColumn;
    if (column_n == table->ColumnsCount)                // To facilitate using with TableGetHoveredColumn()
        column_n = -1;
    RBLXUI_ASSERT(column_n >= -1 && column_n < table->ColumnsCount);
    if (table->Flags & (RblxUITableFlags_Resizable | RblxUITableFlags_Reorderable | RblxUITableFlags_Hideable))
    {
        table->IsContextPopupOpen = true;
        table->ContextPopupColumn = (RblxUITableColumnIdx)column_n;
        table->InstanceInteracted = table->InstanceCurrent;
        const RblxUIID context_menu_id = RblxHashStr("##ContextMenu", 0, table->ID);
        OpenPopupEx(context_menu_id, RblxUIPopupFlags_None);
    }
}

bool RblxUI::TableBeginContextMenuPopup(RblxUITable* table)
{
    if (!table->IsContextPopupOpen || table->InstanceCurrent != table->InstanceInteracted)
        return false;
    const RblxUIID context_menu_id = RblxHashStr("##ContextMenu", 0, table->ID);
    if (BeginPopupEx(context_menu_id, RblxUIWindowFlags_AlwaysAutoResize | RblxUIWindowFlags_NoTitleBar | RblxUIWindowFlags_NoSavedSettings))
        return true;
    table->IsContextPopupOpen = false;
    return false;
}

// FIXME: Copied from MenuItem() for the purpose of being able to pass _SelectOnRelease (#9312)
static bool MenuItemForColumnReorder(const char* label, bool selected, bool enabled)
{
    using namespace RblxUI;
    RblxUIContext& g = *GRblxUI;
    RblxUIWindow* window = g.CurrentWindow;

    RblxVec2 label_size = CalcTextSize(label, NULL, true);
    RblxUIMenuColumns* offsets = &window->DC.MenuColumns;
    float checkmark_w = RBLXUI_TRUNC(g.FontSize * 1.20f);
    float min_w = offsets->DeclColumns(0.0f, label_size.x, 0.0f, checkmark_w); // Feedback for next frame
    float stretch_w = RblxMax(0.0f, GetContentRegionAvail().x - min_w);
    RblxVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);

    RblxUIID id = GetID(label);
    RblxUISelectableFlags selectable_flags = RblxUISelectableFlags_SelectOnRelease | RblxUISelectableFlags_SpanAvailWidth;
    if (g.ActiveId == id)
        selectable_flags |= RblxUISelectableFlags_Highlight; // Stays highlighted while dragging.
    const bool has_been_moved = (g.ActiveId == id) && g.ActiveIdHasBeenEditedBefore; // But disable toggling once moved.

    BeginDisabled(!enabled); // Don't use RblxUISelectableFlags_Disabled so that Check mark is also affected.
    bool ret = Selectable(label, false, selectable_flags, RblxVec2(min_w, label_size.y)) && !has_been_moved; // Can't use IsMouseDragging(0) as button is released already.
    if ((g.LastItemData.StatusFlags & RblxUIItemStatusFlags_Visible) && selected)
        RenderCheckMark(window->DrawList, text_pos + RblxVec2(offsets->OffsetMark + stretch_w + g.FontSize * 0.40f, g.FontSize * 0.134f * 0.5f), GetColorU32(RblxUICol_Text), g.FontSize * 0.866f);
    EndDisabled();

    RBLXUI_TEST_ENGINE_ITEM_INFO(g.LastItemData.ID, label, g.LastItemData.StatusFlags | RblxUIItemStatusFlags_Checkable | (selected ? RblxUIItemStatusFlags_Checked : 0));
    return ret;
}

// Output context menu into current window (generally a popup)
// FIXME-TABLE: Ideally this should be writable by the user. Full programmatic access to that data?
// Sections to display are pulled from 'flags_for_section_to_display', which is typically == table->Flags.
// - RblxUITableFlags_Resizable   -> display Sizing menu items
// - RblxUITableFlags_Reorderable -> display "Reset Order"
////- RblxUITableFlags_Sortable   -> display sorting options (disabled)
// - RblxUITableFlags_Hideable    -> display columns visibility menu items
// It means if you have a custom context menus you can call this section and omit some sections, and add your own.
void RblxUI::TableDrawDefaultContextMenu(RblxUITable* table, RblxUITableFlags flags_for_section_to_display)
{
    RblxUIContext& g = *GRblxUI;
    RblxUIWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return;

    bool want_separator = false;
    const int context_column_n = (table->ContextPopupColumn >= 0 && table->ContextPopupColumn < table->ColumnsCount) ? table->ContextPopupColumn : -1;
    RblxUITableColumn* context_column = (context_column_n != -1) ? &table->Columns[context_column_n] : NULL;

    // Sizing
    if (flags_for_section_to_display & RblxUITableFlags_Resizable)
    {
        if (context_column != NULL)
        {
            const bool can_resize = !(context_column->Flags & RblxUITableColumnFlags_NoResize) && context_column->IsEnabled;
            if (MenuItem(LocalizeGetMsg(RblxUILocKey_TableSizeOne), NULL, false, can_resize)) // "###SizeOne"
                TableSetColumnWidthAutoSingle(table, context_column_n);
        }

        const char* size_all_desc;
        if (table->ColumnsEnabledFixedCount == table->ColumnsEnabledCount && (table->Flags & RblxUITableFlags_SizingMask_) != RblxUITableFlags_SizingFixedSame)
            size_all_desc = LocalizeGetMsg(RblxUILocKey_TableSizeAllFit);        // "###SizeAll" All fixed
        else
            size_all_desc = LocalizeGetMsg(RblxUILocKey_TableSizeAllDefault);    // "###SizeAll" All stretch or mixed
        if (MenuItem(size_all_desc, NULL))
            TableSetColumnWidthAutoAll(table);
        want_separator = true;
    }

    // Ordering
    if (flags_for_section_to_display & RblxUITableFlags_Reorderable)
    {
        if (MenuItem(LocalizeGetMsg(RblxUILocKey_TableResetOrder), NULL, false, !table->IsDefaultDisplayOrder))
            table->IsResetDisplayOrderRequest = true;
        want_separator = true;
    }

    // Reset all (should work but seems unnecessary/noisy to expose?)
    //if (MenuItem("Reset all"))
    //    table->IsResetAllRequest = true;

    // Sorting
    // (modify TableOpenContextMenu() to add _Sortable flag if enabling this)
#if 0
    if ((flags_for_section_to_display & RblxUITableFlags_Sortable) && column != NULL && (column->Flags & RblxUITableColumnFlags_NoSort) == 0)
    {
        if (want_separator)
            Separator();
        want_separator = true;

        bool append_to_sort_specs = g.IO.KeyShift;
        if (MenuItem("Sort in Ascending Order", NULL, column->SortOrder != -1 && column->SortDirection == RblxUISortDirection_Ascending, (column->Flags & RblxUITableColumnFlags_NoSortAscending) == 0))
            TableSetColumnSortDirection(table, column_n, RblxUISortDirection_Ascending, append_to_sort_specs);
        if (MenuItem("Sort in Descending Order", NULL, column->SortOrder != -1 && column->SortDirection == RblxUISortDirection_Descending, (column->Flags & RblxUITableColumnFlags_NoSortDescending) == 0))
            TableSetColumnSortDirection(table, column_n, RblxUISortDirection_Descending, append_to_sort_specs);
    }
#endif

    // Hiding / Visibility
    if (flags_for_section_to_display & RblxUITableFlags_Hideable)
    {
        if (want_separator)
            Separator();
        want_separator = true;

        // While reordering: we calculate min/max allowed range once here so we can avoid a O(N log N) in the loop (because the query itself does a sweep scan).
        // This assume that reordering constraints output a single range, otherwise would need to either call TableGetMaxDisplayOrderAllowed() for each item below, or cache this once per frame into columns.
        const bool is_reordering = (g.ActiveId != 0 && g.ActiveIdWindow == g.CurrentWindow && table->ReorderColumn != -1 && g.ActiveIdHasBeenEditedBefore); // FIXME: This is a bit of a hack.
        const int reorder_src_order = is_reordering ? table->Columns[table->ReorderColumn].DisplayOrder : -1;
        const int reorder_min_order = is_reordering ? TableGetMaxDisplayOrderAllowed(table, reorder_src_order, 0) : 0;
        const int reorder_max_order = is_reordering ? TableGetMaxDisplayOrderAllowed(table, reorder_src_order, table->ColumnsCount - 1) : table->ColumnsCount - 1;
        PushItemFlag(RblxUIItemFlags_AutoClosePopups, false);
        for (int order_n = 0; order_n < table->ColumnsCount; order_n++)
        {
            const int column_n = table->DisplayOrderToIndex[order_n];
            RblxUITableColumn* column = &table->Columns[column_n];
            if (column->Flags & RblxUITableColumnFlags_Disabled)
                continue;

            const char* name = TableGetColumnName(table, column_n);
            if (name == NULL || name[0] == 0)
                name = "<Unknown>";

            // Make sure we can't hide the last active column
            bool menu_item_enabled = (column->Flags & RblxUITableColumnFlags_NoHide) ? false : true;
            if (column->IsUserEnabled && table->ColumnsEnabledCount <= 1)
                menu_item_enabled = false;
            if (is_reordering && (column->DisplayOrder < reorder_min_order || column->DisplayOrder > reorder_max_order))
                menu_item_enabled = false;
            if (MenuItemForColumnReorder(name, column->IsUserEnabled, menu_item_enabled))
                column->IsUserEnabledNextFrame = !column->IsUserEnabled;

            // Drag to reorder
            // FIXME: It is currently not possible to reorder columns marked with RblxUITableColumnFlags_NoHide.
            if (IsItemActive() && IsMouseDragging(0) && g.ActiveIdSource == RblxUIInputSource_Mouse && (table->Flags & RblxUITableFlags_Reorderable))
            {
                g.ActiveIdHasBeenEditedBefore = true; // Disable toggle in MenuItemForColumnReorder() + start dimming to display allowed reorder targets.
                table->ReorderColumn = (RblxUITableColumnIdx)column_n;
                if (!IsItemHovered())
                {
                    int reorder_dir = (g.IO.MousePos.y < (g.LastItemData.Rect.Min.y + g.LastItemData.Rect.Max.y) * 0.5f) ? -1 : +1;
                    float reorder_amount = (reorder_dir < 0 ? g.LastItemData.Rect.Min.y - g.IO.MousePos.y : g.IO.MousePos.y - g.LastItemData.Rect.Max.y) / g.LastItemData.Rect.GetHeight();
                    int dst_order = column->DisplayOrder + (int)RblxCeil(reorder_amount) * reorder_dir; // Estimated target order, will be validated and clamped.
                    TableQueueSetColumnDisplayOrder(table, column_n, dst_order);
                }
            }
        }
        PopItemFlag();
    }
}

//-------------------------------------------------------------------------
// [SECTION] Tables: Settings (.ini data)
//-------------------------------------------------------------------------
// FIXME: The binding/finding/creating flow are too confusing.
//-------------------------------------------------------------------------
// - TableSettingsInit() [Internal]
// - TableSettingsCalcChunkSize() [Internal]
// - TableSettingsCreate() [Internal]
// - TableSettingsFindByID() [Internal]
// - TableGetBoundSettings() [Internal]
// - TableResetSettings()
// - TableSaveSettings() [Internal]
// - TableLoadSettings() [Internal]
// - TableSettingsHandler_ClearAll() [Internal]
// - TableSettingsHandler_ApplyAll() [Internal]
// - TableSettingsHandler_ReadOpen() [Internal]
// - TableSettingsHandler_ReadLine() [Internal]
// - TableSettingsHandler_WriteAll() [Internal]
// - TableSettingsInstallHandler() [Internal]
//-------------------------------------------------------------------------
// [Init] 1: TableSettingsHandler_ReadXXXX()   Load and parse .ini file into TableSettings.
// [Main] 2: TableLoadSettings()               When table is created, bind Table to TableSettings, serialize TableSettings data into Table.
// [Main] 3: TableSaveSettings()               When table properties are modified, serialize Table data into bound or new TableSettings, mark .ini as dirty.
// [Main] 4: TableSettingsHandler_WriteAll()   When .ini file is dirty (which can come from other source), save TableSettings into .ini file.
//-------------------------------------------------------------------------

// Clear and initialize empty settings instance
static void TableSettingsInit(RblxUITableSettings* settings, RblxUIID id, int columns_count, int columns_count_max)
{
    RBLXUI_PLACEMENT_NEW(settings) RblxUITableSettings();
    RblxUITableColumnSettings* settings_column = settings->GetColumnSettings();
    for (int n = 0; n < columns_count_max; n++, settings_column++)
        RBLXUI_PLACEMENT_NEW(settings_column) RblxUITableColumnSettings();
    settings->ID = id;
    settings->ColumnsCount = (RblxUITableColumnIdx)columns_count;
    settings->ColumnsCountMax = (RblxUITableColumnIdx)columns_count_max;
    settings->WantApply = true;
}

static size_t TableSettingsCalcChunkSize(int columns_count)
{
    return sizeof(RblxUITableSettings) + (size_t)columns_count * sizeof(RblxUITableColumnSettings);
}

RblxUITableSettings* RblxUI::TableSettingsCreate(RblxUIID id, int columns_count)
{
    RblxUIContext& g = *GRblxUI;
    RblxUITableSettings* settings = g.SettingsTables.alloc_chunk(TableSettingsCalcChunkSize(columns_count));
    TableSettingsInit(settings, id, columns_count, columns_count);
    return settings;
}

// Find existing settings
RblxUITableSettings* RblxUI::TableSettingsFindByID(RblxUIID id)
{
    // FIXME-OPT: Might want to store a lookup map for this?
    RblxUIContext& g = *GRblxUI;
    for (RblxUITableSettings* settings = g.SettingsTables.begin(); settings != NULL; settings = g.SettingsTables.next_chunk(settings))
        if (settings->ID == id)
            return settings;
    return NULL;
}

// Get settings for a given table, NULL if none
RblxUITableSettings* RblxUI::TableGetBoundSettings(RblxUITable* table)
{
    if (table->SettingsOffset != -1)
    {
        RblxUIContext& g = *GRblxUI;
        RblxUITableSettings* settings = g.SettingsTables.ptr_from_offset(table->SettingsOffset);
        RBLXUI_ASSERT(settings->ID == table->ID);
        if (settings->ColumnsCountMax >= table->ColumnsCount)
            return settings; // OK
        settings->ID = 0; // Invalidate storage, we won't fit because of a count change
    }
    return NULL;
}

// Restore initial state of table (with or without saved settings)
void RblxUI::TableResetSettings(RblxUITable* table)
{
    table->IsInitializing = table->IsSettingsDirty = true;
    table->IsResetAllRequest = false;
    table->IsSettingsRequestLoad = false;                   // Don't reload from ini
    table->SettingsLoadedFlags = RblxUITableFlags_None;      // Mark as nothing loaded so our initialized data becomes authoritative
}

void RblxUI::TableSaveSettings(RblxUITable* table)
{
    table->IsSettingsDirty = false;
    if (table->Flags & RblxUITableFlags_NoSavedSettings)
        return;

    // Bind or create settings data
    RblxUIContext& g = *GRblxUI;
    RblxUITableSettings* settings = TableGetBoundSettings(table);
    if (settings == NULL)
    {
        settings = TableSettingsCreate(table->ID, table->ColumnsCount);
        table->SettingsOffset = g.SettingsTables.offset_from_ptr(settings);
    }
    settings->ColumnsCount = (RblxUITableColumnIdx)table->ColumnsCount;

    // Serialize RblxUITable/RblxUITableColumn into RblxUITableSettings/RblxUITableColumnSettings
    RBLXUI_ASSERT(settings->ID == table->ID);
    RBLXUI_ASSERT(settings->ColumnsCount == table->ColumnsCount && settings->ColumnsCountMax >= settings->ColumnsCount);
    RblxUITableColumn* column = table->Columns.Data;
    RblxUITableColumnSettings* column_settings = settings->GetColumnSettings();

    bool save_ref_scale = false;
    settings->SaveFlags = RblxUITableFlags_None;
    for (int n = 0; n < table->ColumnsCount; n++, column++, column_settings++)
    {
        const float width_or_weight = (column->Flags & RblxUITableColumnFlags_WidthStretch) ? column->StretchWeight : column->WidthRequest;
        column_settings->WidthOrWeight = width_or_weight;
        column_settings->Index = (RblxUITableColumnIdx)n;
        column_settings->DisplayOrder = column->DisplayOrder;
        column_settings->SortOrder = column->SortOrder;
        column_settings->SortDirection = column->SortDirection;
        column_settings->IsEnabled = column->IsUserEnabled;
        column_settings->IsStretch = (column->Flags & RblxUITableColumnFlags_WidthStretch) ? 1 : 0;
        if ((column->Flags & RblxUITableColumnFlags_WidthStretch) == 0)
            save_ref_scale = true;

        // We skip saving some data in the .ini file when they are unnecessary to restore our state.
        // Note that fixed width where initial width was derived from auto-fit will always be saved as InitStretchWeightOrWidth will be 0.0f.
        // FIXME-TABLE: We don't have logic to easily compare SortOrder to DefaultSortOrder yet so it's always saved when present.
        if (width_or_weight != column->InitStretchWeightOrWidth)
            settings->SaveFlags |= RblxUITableFlags_Resizable;
        if (column->DisplayOrder != n)
            settings->SaveFlags |= RblxUITableFlags_Reorderable;
        if (column->SortOrder != -1)
            settings->SaveFlags |= RblxUITableFlags_Sortable;
        if (column->IsUserEnabled != ((column->Flags & RblxUITableColumnFlags_DefaultHide) == 0))
            settings->SaveFlags |= RblxUITableFlags_Hideable;
    }
    settings->SaveFlags &= table->Flags;
    settings->RefScale = save_ref_scale ? table->RefScale : 0.0f;

    MarkIniSettingsDirty();
}

void RblxUI::TableLoadSettings(RblxUITable* table)
{
    RblxUIContext& g = *GRblxUI;
    table->IsSettingsRequestLoad = false;
    if (table->Flags & RblxUITableFlags_NoSavedSettings)
        return;

    // Bind settings
    RblxUITableSettings* settings;
    if (table->SettingsOffset == -1)
    {
        settings = TableSettingsFindByID(table->ID);
        if (settings == NULL)
            return;
        if (settings->ColumnsCount != table->ColumnsCount) // Allow settings if columns count changed. We could otherwise decide to return...
            table->IsSettingsDirty = true;
        table->SettingsOffset = g.SettingsTables.offset_from_ptr(settings);
    }
    else
    {
        settings = TableGetBoundSettings(table);
    }

    table->SettingsLoadedFlags = settings->SaveFlags;
    table->RefScale = settings->RefScale;

    // Initialize default columns settings
    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
    {
        RblxUITableColumn* column = &table->Columns[column_n];
        TableInitColumnDefaults(table, column, ~0);
        column->AutoFitQueue = 0x00;
    }

    // Serialize RblxUITableSettings/RblxUITableColumnSettings into RblxUITable/RblxUITableColumn
    RblxUITableColumnSettings* column_settings = settings->GetColumnSettings();
    for (int data_n = 0; data_n < settings->ColumnsCount; data_n++, column_settings++)
    {
        int column_n = column_settings->Index;
        if (column_n < 0 || column_n >= table->ColumnsCount)
            continue;

        RblxUITableColumn* column = &table->Columns[column_n];
        if (settings->SaveFlags & RblxUITableFlags_Resizable)
        {
            if (column_settings->IsStretch)
                column->StretchWeight = column_settings->WidthOrWeight;
            else
                column->WidthRequest = column_settings->WidthOrWeight;
        }
        if (settings->SaveFlags & RblxUITableFlags_Reorderable)
            column->DisplayOrder = column_settings->DisplayOrder;
        if ((settings->SaveFlags & RblxUITableFlags_Hideable) && column_settings->IsEnabled != -1)
            column->IsUserEnabled = column->IsUserEnabledNextFrame = column_settings->IsEnabled == 1;
        column->SortOrder = column_settings->SortOrder;
        column->SortDirection = column_settings->SortDirection;
    }

    // Fix display order and build index
    if (settings->SaveFlags & RblxUITableFlags_Reorderable)
        TableFixDisplayOrder(table);
    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
        table->DisplayOrderToIndex[table->Columns[column_n].DisplayOrder] = (RblxUITableColumnIdx)column_n;
}

struct RblxUITableFixDisplayOrderColumnData
{
    RblxUITableColumnIdx     Idx;
    RblxUITable*             Table;  // This is unfortunate but we don't have userdata in qsort api.
};

// Sort by DisplayOrder and then Index
static int RBLXUI_CDECL TableFixDisplayOrderComparer(const void* lhs, const void* rhs)
{
    const RblxUITable* table = ((const RblxUITableFixDisplayOrderColumnData*)lhs)->Table;
    const RblxUITableColumnIdx lhs_idx = ((const RblxUITableFixDisplayOrderColumnData*)lhs)->Idx;
    const RblxUITableColumnIdx rhs_idx = ((const RblxUITableFixDisplayOrderColumnData*)rhs)->Idx;
    const int order_delta = (table->Columns[lhs_idx].DisplayOrder - table->Columns[rhs_idx].DisplayOrder);
    return (order_delta > 0) ? +1 : (order_delta < 0) ? -1 : (lhs_idx > rhs_idx) ? +1 : -1;
}

// Fix invalid display order data: compact values (0,1,3 -> 0,1,2); preserve relative order (0,3,1 -> 0,2,1); deduplicate (0,4,1,1 -> 0,3,1,2)
void RblxUI::TableFixDisplayOrder(RblxUITable* table)
{
    RblxUIContext& g = *GRblxUI;
    g.TempBuffer.reserve((int)(sizeof(RblxUITableFixDisplayOrderColumnData) * table->ColumnsCount)); // FIXME: Maybe wrap those two lines as a helper.
    RblxUITableFixDisplayOrderColumnData* fdo_columns = (RblxUITableFixDisplayOrderColumnData*)(void*)g.TempBuffer.Data;
    for (int n = 0; n < table->ColumnsCount; n++)
    {
        fdo_columns[n].Idx = (RblxUITableColumnIdx)n;
        fdo_columns[n].Table = table;
    }
    RblxQsort(fdo_columns, (size_t)table->ColumnsCount, sizeof(RblxUITableFixDisplayOrderColumnData), TableFixDisplayOrderComparer);
    for (int n = 0; n < table->ColumnsCount; n++)
        table->Columns[fdo_columns[n].Idx].DisplayOrder = (RblxUITableColumnIdx)n;
}

static void TableSettingsHandler_ClearAll(RblxUIContext* ctx, RblxUISettingsHandler*)
{
    RblxUIContext& g = *ctx;
    for (int i = 0; i != g.Tables.GetMapSize(); i++)
        if (RblxUITable* table = g.Tables.TryGetMapData(i))
            table->SettingsOffset = -1;
    g.SettingsTables.clear();
}

// Apply to existing windows (if any)
static void TableSettingsHandler_ApplyAll(RblxUIContext* ctx, RblxUISettingsHandler*)
{
    RblxUIContext& g = *ctx;
    for (int i = 0; i != g.Tables.GetMapSize(); i++)
        if (RblxUITable* table = g.Tables.TryGetMapData(i))
        {
            table->IsSettingsRequestLoad = true;
            table->SettingsOffset = -1;
        }
}

static void* TableSettingsHandler_ReadOpen(RblxUIContext*, RblxUISettingsHandler*, const char* name)
{
    RblxUIID id = 0;
    int columns_count = 0;
    if (sscanf(name, "0x%08X,%d", &id, &columns_count) < 2)
        return NULL;

    if (RblxUITableSettings* settings = RblxUI::TableSettingsFindByID(id))
    {
        if (settings->ColumnsCountMax >= columns_count)
        {
            TableSettingsInit(settings, id, columns_count, settings->ColumnsCountMax); // Recycle
            return settings;
        }
        settings->ID = 0; // Invalidate storage, we won't fit because of a count change
    }
    return RblxUI::TableSettingsCreate(id, columns_count);
}

static void TableSettingsHandler_ReadLine(RblxUIContext*, RblxUISettingsHandler*, void* entry, const char* line)
{
    // "Column 0  UserID=0x42AD2D21 Width=100 Visible=1 Order=0 Sort=0v"
    RblxUITableSettings* settings = (RblxUITableSettings*)entry;
    float f = 0.0f;
    int column_n = 0, r = 0, n = 0;

    if (sscanf(line, "RefScale=%f", &f) == 1) { settings->RefScale = f; return; }

    if (sscanf(line, "Column %d%n", &column_n, &r) == 1)
    {
        if (column_n < 0 || column_n >= settings->ColumnsCount)
            return;
        line = RblxStrSkipBlank(line + r);
        char c = 0;
        RblxUITableColumnSettings* column = settings->GetColumnSettings() + column_n;
        column->Index = (RblxUITableColumnIdx)column_n;
        if (sscanf(line, "UserID=0x%08X%n", (RblxU32*)&n, &r)==1) { line = RblxStrSkipBlank(line + r); column->UserID = (RblxUIID)n; }
        if (sscanf(line, "Width=%d%n", &n, &r) == 1)            { line = RblxStrSkipBlank(line + r); column->WidthOrWeight = (float)n; column->IsStretch = 0; settings->SaveFlags |= RblxUITableFlags_Resizable; }
        if (sscanf(line, "Weight=%f%n", &f, &r) == 1)           { line = RblxStrSkipBlank(line + r); column->WidthOrWeight = f; column->IsStretch = 1; settings->SaveFlags |= RblxUITableFlags_Resizable; }
        if (sscanf(line, "Visible=%d%n", &n, &r) == 1)          { line = RblxStrSkipBlank(line + r); column->IsEnabled = (RblxU8)n; settings->SaveFlags |= RblxUITableFlags_Hideable; }
        if (sscanf(line, "Order=%d%n", &n, &r) == 1)            { line = RblxStrSkipBlank(line + r); column->DisplayOrder = (RblxUITableColumnIdx)n; settings->SaveFlags |= RblxUITableFlags_Reorderable; }
        if (sscanf(line, "Sort=%d%c%n", &n, &c, &r) == 2)       { line = RblxStrSkipBlank(line + r); column->SortOrder = (RblxUITableColumnIdx)n; column->SortDirection = (c == '^') ? RblxUISortDirection_Descending : RblxUISortDirection_Ascending; settings->SaveFlags |= RblxUITableFlags_Sortable; }
    }
}

static void TableSettingsHandler_WriteAll(RblxUIContext* ctx, RblxUISettingsHandler* handler, RblxUITextBuffer* buf)
{
    RblxUIContext& g = *ctx;
    for (RblxUITableSettings* settings = g.SettingsTables.begin(); settings != NULL; settings = g.SettingsTables.next_chunk(settings))
    {
        if (settings->ID == 0) // Skip ditched settings
            continue;

        // TableSaveSettings() may clear some of those flags when we establish that the data can be stripped
        // (e.g. Order was unchanged)
        const bool save_size    = (settings->SaveFlags & RblxUITableFlags_Resizable) != 0;
        const bool save_visible = (settings->SaveFlags & RblxUITableFlags_Hideable) != 0;
        const bool save_order   = (settings->SaveFlags & RblxUITableFlags_Reorderable) != 0;
        const bool save_sort    = (settings->SaveFlags & RblxUITableFlags_Sortable) != 0;
        // We need to save the [Table] entry even if all the bools are false, since this records a table with "default settings".

        buf->reserve(buf->size() + 30 + settings->ColumnsCount * 50); // ballpark reserve
        buf->appendf("[%s][0x%08X,%d]\n", handler->TypeName, settings->ID, settings->ColumnsCount);
        if (settings->RefScale != 0.0f)
            buf->appendf("RefScale=%g\n", settings->RefScale);
        RblxUITableColumnSettings* column = settings->GetColumnSettings();
        for (int column_n = 0; column_n < settings->ColumnsCount; column_n++, column++)
        {
            // "Column 0  UserID=0x42AD2D21 Width=100 Visible=1 Order=0 Sort=0v"
            bool save_column = column->UserID != 0 || save_size || save_visible || save_order || (save_sort && column->SortOrder != -1);
            if (!save_column)
                continue;
            buf->appendf("Column %-2d", column_n);
            if (column->UserID != 0)                    { buf->appendf(" UserID=%08X", column->UserID); }
            if (save_size && column->IsStretch)         { buf->appendf(" Weight=%.4f", column->WidthOrWeight); }
            if (save_size && !column->IsStretch)        { buf->appendf(" Width=%d", (int)column->WidthOrWeight); }
            if (save_visible)                           { buf->appendf(" Visible=%d", column->IsEnabled); }
            if (save_order)                             { buf->appendf(" Order=%d", column->DisplayOrder); }
            if (save_sort && column->SortOrder != -1)   { buf->appendf(" Sort=%d%c", column->SortOrder, (column->SortDirection == RblxUISortDirection_Ascending) ? 'v' : '^'); }
            buf->append("\n");
        }
        buf->append("\n");
    }
}

void RblxUI::TableSettingsAddSettingsHandler()
{
    RblxUISettingsHandler ini_handler;
    ini_handler.TypeName = "Table";
    ini_handler.TypeHash = RblxHashStr("Table");
    ini_handler.ClearAllFn = TableSettingsHandler_ClearAll;
    ini_handler.ReadOpenFn = TableSettingsHandler_ReadOpen;
    ini_handler.ReadLineFn = TableSettingsHandler_ReadLine;
    ini_handler.ApplyAllFn = TableSettingsHandler_ApplyAll;
    ini_handler.WriteAllFn = TableSettingsHandler_WriteAll;
    AddSettingsHandler(&ini_handler);
}

//-------------------------------------------------------------------------
// [SECTION] Tables: Garbage Collection
//-------------------------------------------------------------------------
// - TableRemove() [Internal]
// - TableGcCompactTransientBuffers() [Internal]
// - TableGcCompactSettings() [Internal]
//-------------------------------------------------------------------------

// Remove Table data (currently only used by TestEngine)
void RblxUI::TableRemove(RblxUITable* table)
{
    //RBLXUI_DEBUG_PRINT("TableRemove() id=0x%08X\n", table->ID);
    RblxUIContext& g = *GRblxUI;
    int table_idx = g.Tables.GetIndex(table);
    //memset(table->RawData.Data, 0, table->RawData.size_in_bytes());
    //memset(table, 0, sizeof(RblxUITable));
    g.Tables.Remove(table->ID, table);
    g.TablesLastTimeActive[table_idx] = -1.0f;
}

// Free up/compact internal Table buffers for when it gets unused
void RblxUI::TableGcCompactTransientBuffers(RblxUITable* table)
{
    //RBLXUI_DEBUG_PRINT("TableGcCompactTransientBuffers() id=0x%08X\n", table->ID);
    RblxUIContext& g = *GRblxUI;
    RBLXUI_ASSERT(table->MemoryCompacted == false);
    table->SortSpecs.Specs = NULL;
    table->SortSpecsMulti.clear();
    table->IsSortSpecsDirty = true; // FIXME: In theory shouldn't have to leak into user performing a sort on resume.
    table->ColumnsNames.clear();
    table->MemoryCompacted = true;
    for (int n = 0; n < table->ColumnsCount; n++)
        table->Columns[n].NameOffset = -1;
    g.TablesLastTimeActive[g.Tables.GetIndex(table)] = -1.0f;
}

void RblxUI::TableGcCompactTransientBuffers(RblxUITableTempData* temp_data)
{
    temp_data->DrawSplitter.ClearFreeMemory();
    temp_data->LastTimeActive = -1.0f;
}

// Compact and remove unused settings data (currently only used by TestEngine)
void RblxUI::TableGcCompactSettings()
{
    RblxUIContext& g = *GRblxUI;
    int required_memory = 0;
    for (RblxUITableSettings* settings = g.SettingsTables.begin(); settings != NULL; settings = g.SettingsTables.next_chunk(settings))
        if (settings->ID != 0)
            required_memory += (int)TableSettingsCalcChunkSize(settings->ColumnsCount);
    if (required_memory == g.SettingsTables.Buf.Size)
        return;
    RblxChunkStream<RblxUITableSettings> new_chunk_stream;
    new_chunk_stream.Buf.reserve(required_memory);
    for (RblxUITableSettings* settings = g.SettingsTables.begin(); settings != NULL; settings = g.SettingsTables.next_chunk(settings))
        if (settings->ID != 0)
            memcpy(new_chunk_stream.alloc_chunk(TableSettingsCalcChunkSize(settings->ColumnsCount)), settings, TableSettingsCalcChunkSize(settings->ColumnsCount));
    g.SettingsTables.swap(new_chunk_stream);
}


//-------------------------------------------------------------------------
// [SECTION] Tables: Debugging
//-------------------------------------------------------------------------
// - DebugNodeTable() [Internal]
//-------------------------------------------------------------------------

#ifndef RBLXUI_DISABLE_DEBUG_TOOLS

static const char* DebugNodeTableGetSizingPolicyDesc(RblxUITableFlags sizing_policy)
{
    sizing_policy &= RblxUITableFlags_SizingMask_;
    if (sizing_policy == RblxUITableFlags_SizingFixedFit)    { return "FixedFit"; }
    if (sizing_policy == RblxUITableFlags_SizingFixedSame)   { return "FixedSame"; }
    if (sizing_policy == RblxUITableFlags_SizingStretchProp) { return "StretchProp"; }
    if (sizing_policy == RblxUITableFlags_SizingStretchSame) { return "StretchSame"; }
    return "N/A";
}

void RblxUI::DebugNodeTable(RblxUITable* table)
{
    RblxUIContext& g = *GRblxUI;
    const bool is_active = (table->LastFrameActive >= g.FrameCount - 2); // Note that fully clipped early out scrolling tables will appear as inactive here.
    if (!is_active) { PushStyleColor(RblxUICol_Text, GetStyleColorVec4(RblxUICol_TextDisabled)); }
    bool open = TreeNode(table, "Table 0x%08X (%d columns, in '%s')%s", table->ID, table->ColumnsCount, table->OuterWindow->Name, is_active ? "" : " *Inactive*");
    if (!is_active) { PopStyleColor(); }
    if (IsItemHovered())
        GetForegroundDrawList(table->OuterWindow)->AddRect(table->OuterRect.Min, table->OuterRect.Max, RBLXUI_COL32(255, 255, 0, 255));
    if (IsItemVisible() && table->HoveredColumnBody != -1)
        GetForegroundDrawList(table->OuterWindow)->AddRect(GetItemRectMin(), GetItemRectMax(), RBLXUI_COL32(255, 255, 0, 255));
    if (!open)
        return;
    if (table->InstanceCurrent > 0)
        Text("** %d instances of same table! Some data below will refer to last instance.", table->InstanceCurrent + 1);
    if (g.IO.ConfigDebugIsDebuggerPresent)
    {
        if (DebugBreakButton("**DebugBreak**", "in BeginTable()"))
            g.DebugBreakInTable = table->ID;
        SameLine();
    }

    bool clear_settings = SmallButton("Clear settings");
    BulletText("OuterRect: Pos: (%.1f,%.1f) Size: (%.1f,%.1f) Sizing: '%s'", table->OuterRect.Min.x, table->OuterRect.Min.y, table->OuterRect.GetWidth(), table->OuterRect.GetHeight(), DebugNodeTableGetSizingPolicyDesc(table->Flags));
    BulletText("ColumnsGivenWidth: %.1f, ColumnsAutoFitWidth: %.1f, InnerWidth: %.1f%s", table->ColumnsGivenWidth, table->ColumnsAutoFitWidth, table->InnerWidth, table->InnerWidth == 0.0f ? " (auto)" : "");
    BulletText("CellPaddingX: %.1f, CellSpacingX: %.1f/%.1f, OuterPaddingX: %.1f", table->CellPaddingX, table->CellSpacingX1, table->CellSpacingX2, table->OuterPaddingX);
    BulletText("HoveredColumnBody: %d, HoveredColumnBorder: %d", table->HoveredColumnBody, table->HoveredColumnBorder);
    BulletText("ResizedColumn: %d, HeldHeaderColumn: %d, ReorderColumn: %d", table->LastResizedColumn, table->LastHeldHeaderColumn, table->ReorderColumn);
    for (int n = 0; n < table->InstanceCurrent + 1; n++)
    {
        RblxUITableInstanceData* table_instance = TableGetInstanceData(table, n);
        BulletText("Instance %d: HoveredRow: %d, LastOuterHeight: %.2f", n, table_instance->HoveredRowLast, table_instance->LastOuterHeight);
    }
    //BulletText("BgDrawChannels: %d/%d", 0, table->BgDrawChannelUnfrozen);
    float sum_weights = 0.0f;
    for (int n = 0; n < table->ColumnsCount; n++)
        if (table->Columns[n].Flags & RblxUITableColumnFlags_WidthStretch)
            sum_weights += table->Columns[n].StretchWeight;
    for (int n = 0; n < table->ColumnsCount; n++)
    {
        RblxUITableColumn* column = &table->Columns[n];
        const char* name = TableGetColumnName(table, n);
        char buf[512];
        RblxFormatString(buf, RBLXUI_COUNTOF(buf),
            "Column %d order %d '%s': offset %+.2f to %+.2f%s\n"
            "Enabled: %d, VisibleX/Y: %d/%d, RequestOutput: %d, SkipItems: %d, DrawChannels: %d,%d\n"
            "WidthGiven: %.1f, Request/Auto: %.1f/%.1f, StretchWeight: %.3f (%.1f%%)\n"
            "MinX: %.1f, MaxX: %.1f (%+.1f), ClipRect: %.1f to %.1f (+%.1f)\n"
            "ContentWidth: %.1f,%.1f, HeadersUsed/Ideal %.1f/%.1f\n"
            "Sort: %d%s, UserID: 0x%08X, Flags: 0x%04X: %s%s%s..",
            n, column->DisplayOrder, name, column->MinX - table->WorkRect.Min.x, column->MaxX - table->WorkRect.Min.x, (n < table->FreezeColumnsRequest) ? " (Frozen)" : "",
            column->IsEnabled, column->IsVisibleX, column->IsVisibleY, column->IsRequestOutput, column->IsSkipItems, column->DrawChannelFrozen, column->DrawChannelUnfrozen,
            column->WidthGiven, column->WidthRequest, column->WidthAuto, column->StretchWeight, column->StretchWeight > 0.0f ? (column->StretchWeight / sum_weights) * 100.0f : 0.0f,
            column->MinX, column->MaxX, column->MaxX - column->MinX, column->ClipRect.Min.x, column->ClipRect.Max.x, column->ClipRect.Max.x - column->ClipRect.Min.x,
            column->ContentMaxXFrozen - column->WorkMinX, column->ContentMaxXUnfrozen - column->WorkMinX, column->ContentMaxXHeadersUsed - column->WorkMinX, column->ContentMaxXHeadersIdeal - column->WorkMinX,
            column->SortOrder, (column->SortDirection == RblxUISortDirection_Ascending) ? " (Asc)" : (column->SortDirection == RblxUISortDirection_Descending) ? " (Des)" : "", column->UserID, column->Flags,
            (column->Flags & RblxUITableColumnFlags_WidthStretch) ? "WidthStretch " : "",
            (column->Flags & RblxUITableColumnFlags_WidthFixed) ? "WidthFixed " : "",
            (column->Flags & RblxUITableColumnFlags_NoResize) ? "NoResize " : "");
        Bullet();
        Selectable(buf);
        if (IsItemHovered())
        {
            RblxRect r(column->MinX, table->OuterRect.Min.y, column->MaxX, table->OuterRect.Max.y);
            GetForegroundDrawList(table->OuterWindow)->AddRect(r.Min, r.Max, RBLXUI_COL32(255, 255, 0, 255));
        }
    }
    if (RblxUITableSettings* settings = TableGetBoundSettings(table))
        DebugNodeTableSettings(settings);
    if (clear_settings)
        table->IsResetAllRequest = true;
    TreePop();
}

void RblxUI::DebugNodeTableSettings(RblxUITableSettings* settings)
{
    if (!TreeNode((void*)(intptr_t)settings->ID, "Settings 0x%08X (%d columns)", settings->ID, settings->ColumnsCount))
        return;
    BulletText("SaveFlags: 0x%08X", settings->SaveFlags);
    BulletText("ColumnsCount: %d (max %d)", settings->ColumnsCount, settings->ColumnsCountMax);
    for (int n = 0; n < settings->ColumnsCount; n++)
    {
        RblxUITableColumnSettings* column_settings = &settings->GetColumnSettings()[n];
        RblxUISortDirection sort_dir = (column_settings->SortOrder != -1) ? (RblxUISortDirection)column_settings->SortDirection : RblxUISortDirection_None;
        BulletText("Column %d Order %d SortOrder %d %s Vis %d %s %7.3f UserID 0x%08X",
            n, column_settings->DisplayOrder, column_settings->SortOrder,
            (sort_dir == RblxUISortDirection_Ascending) ? "Asc" : (sort_dir == RblxUISortDirection_Descending) ? "Des" : "---",
            column_settings->IsEnabled, column_settings->IsStretch ? "Weight" : "Width ", column_settings->WidthOrWeight, column_settings->UserID);
    }
    TreePop();
}

#else // #ifndef RBLXUI_DISABLE_DEBUG_TOOLS

void RblxUI::DebugNodeTable(RblxUITable*) {}
void RblxUI::DebugNodeTableSettings(RblxUITableSettings*) {}

#endif


//-------------------------------------------------------------------------
// [SECTION] Columns, BeginColumns, EndColumns, etc.
// (This is a legacy API, prefer using BeginTable/EndTable!)
//-------------------------------------------------------------------------
// FIXME: sizing is lossy when columns width is very small (default width may turn negative etc.)
//-------------------------------------------------------------------------
// - SetWindowClipRectBeforeSetChannel() [Internal]
// - GetColumnIndex()
// - GetColumnsCount()
// - GetColumnOffset()
// - GetColumnWidth()
// - SetColumnOffset()
// - SetColumnWidth()
// - PushColumnClipRect() [Internal]
// - PushColumnsBackground() [Internal]
// - PopColumnsBackground() [Internal]
// - FindOrCreateColumns() [Internal]
// - GetColumnsID() [Internal]
// - BeginColumns()
// - NextColumn()
// - EndColumns()
// - Columns()
//-------------------------------------------------------------------------

// [Internal] Small optimization to avoid calls to PopClipRect/SetCurrentChannel/PushClipRect in sequences,
// they would meddle many times with the underlying RblxDrawCmd.
// Instead, we do a preemptive overwrite of clipping rectangle _without_ altering the command-buffer and let
// the subsequent single call to SetCurrentChannel() does it things once.
void RblxUI::SetWindowClipRectBeforeSetChannel(RblxUIWindow* window, const RblxRect& clip_rect)
{
    RblxVec4 clip_rect_vec4 = clip_rect.ToVec4();
    window->ClipRect = clip_rect;
    window->DrawList->_CmdHeader.ClipRect = clip_rect_vec4;
    window->DrawList->_ClipRectStack.Data[window->DrawList->_ClipRectStack.Size - 1] = clip_rect_vec4;
}

int RblxUI::GetColumnIndex()
{
    RblxUIWindow* window = GetCurrentWindowRead();
    return window->DC.CurrentColumns ? window->DC.CurrentColumns->Current : 0;
}

int RblxUI::GetColumnsCount()
{
    RblxUIWindow* window = GetCurrentWindowRead();
    return window->DC.CurrentColumns ? window->DC.CurrentColumns->Count : 1;
}

float RblxUI::GetColumnOffsetFromNorm(const RblxUIOldColumns* columns, float offset_norm)
{
    return offset_norm * (columns->OffMaxX - columns->OffMinX);
}

float RblxUI::GetColumnNormFromOffset(const RblxUIOldColumns* columns, float offset)
{
    return offset / (columns->OffMaxX - columns->OffMinX);
}

static const float COLUMNS_HIT_RECT_HALF_THICKNESS = 4.0f;

static float GetDraggedColumnOffset(RblxUIOldColumns* columns, int column_index)
{
    // Active (dragged) column always follow mouse. The reason we need this is that dragging a column to the right edge of an auto-resizing
    // window creates a feedback loop because we store normalized positions. So while dragging we enforce absolute positioning.
    RblxUIContext& g = *GRblxUI;
    RblxUIWindow* window = g.CurrentWindow;
    RBLXUI_ASSERT(column_index > 0); // We are not supposed to drag column 0.
    RBLXUI_ASSERT(g.ActiveId == columns->ID + RblxUIID(column_index));

    float x = g.IO.MousePos.x - g.ActiveIdClickOffset.x + RblxTrunc(COLUMNS_HIT_RECT_HALF_THICKNESS * g.CurrentDpiScale) - window->Pos.x;
    x = RblxMax(x, RblxUI::GetColumnOffset(column_index - 1) + g.Style.ColumnsMinSpacing);
    if ((columns->Flags & RblxUIOldColumnFlags_NoPreserveWidths))
        x = RblxMin(x, RblxUI::GetColumnOffset(column_index + 1) - g.Style.ColumnsMinSpacing);

    return x;
}

float RblxUI::GetColumnOffset(int column_index)
{
    RblxUIWindow* window = GetCurrentWindowRead();
    RblxUIOldColumns* columns = window->DC.CurrentColumns;
    if (columns == NULL)
        return 0.0f;

    if (column_index < 0)
        column_index = columns->Current;
    RBLXUI_ASSERT(column_index < columns->Columns.Size);

    const float t = columns->Columns[column_index].OffsetNorm;
    const float x_offset = RblxLerp(columns->OffMinX, columns->OffMaxX, t);
    return x_offset;
}

static float GetColumnWidthEx(RblxUIOldColumns* columns, int column_index, bool before_resize = false)
{
    if (column_index < 0)
        column_index = columns->Current;

    float offset_norm;
    if (before_resize)
        offset_norm = columns->Columns[column_index + 1].OffsetNormBeforeResize - columns->Columns[column_index].OffsetNormBeforeResize;
    else
        offset_norm = columns->Columns[column_index + 1].OffsetNorm - columns->Columns[column_index].OffsetNorm;
    return RblxUI::GetColumnOffsetFromNorm(columns, offset_norm);
}

float RblxUI::GetColumnWidth(int column_index)
{
    RblxUIContext& g = *GRblxUI;
    RblxUIWindow* window = g.CurrentWindow;
    RblxUIOldColumns* columns = window->DC.CurrentColumns;
    if (columns == NULL)
        return GetContentRegionAvail().x;

    if (column_index < 0)
        column_index = columns->Current;
    return GetColumnOffsetFromNorm(columns, columns->Columns[column_index + 1].OffsetNorm - columns->Columns[column_index].OffsetNorm);
}

void RblxUI::SetColumnOffset(int column_index, float offset)
{
    RblxUIContext& g = *GRblxUI;
    RblxUIWindow* window = g.CurrentWindow;
    RblxUIOldColumns* columns = window->DC.CurrentColumns;
    RBLXUI_ASSERT(columns != NULL);

    if (column_index < 0)
        column_index = columns->Current;
    RBLXUI_ASSERT(column_index < columns->Columns.Size);

    const bool preserve_width = !(columns->Flags & RblxUIOldColumnFlags_NoPreserveWidths) && (column_index < columns->Count - 1);
    const float width = preserve_width ? GetColumnWidthEx(columns, column_index, columns->IsBeingResized) : 0.0f;

    if (!(columns->Flags & RblxUIOldColumnFlags_NoForceWithinWindow))
        offset = RblxMin(offset, columns->OffMaxX - g.Style.ColumnsMinSpacing * (columns->Count - column_index));
    columns->Columns[column_index].OffsetNorm = GetColumnNormFromOffset(columns, offset - columns->OffMinX);

    if (preserve_width)
        SetColumnOffset(column_index + 1, offset + RblxMax(g.Style.ColumnsMinSpacing, width));
}

void RblxUI::SetColumnWidth(int column_index, float width)
{
    RblxUIWindow* window = GetCurrentWindowRead();
    RblxUIOldColumns* columns = window->DC.CurrentColumns;
    RBLXUI_ASSERT(columns != NULL);

    if (column_index < 0)
        column_index = columns->Current;
    SetColumnOffset(column_index + 1, GetColumnOffset(column_index) + width);
}

void RblxUI::PushColumnClipRect(int column_index)
{
    RblxUIWindow* window = GetCurrentWindowRead();
    RblxUIOldColumns* columns = window->DC.CurrentColumns;
    if (column_index < 0)
        column_index = columns->Current;

    RblxUIOldColumnData* column = &columns->Columns[column_index];
    PushClipRect(column->ClipRect.Min, column->ClipRect.Max, false);
}

// Get into the columns background draw command (which is generally the same draw command as before we called BeginColumns)
void RblxUI::PushColumnsBackground()
{
    RblxUIWindow* window = GetCurrentWindowRead();
    RblxUIOldColumns* columns = window->DC.CurrentColumns;
    if (columns->Count == 1)
        return;

    // Optimization: avoid SetCurrentChannel() + PushClipRect()
    columns->HostBackupClipRect = window->ClipRect;
    SetWindowClipRectBeforeSetChannel(window, columns->HostInitialClipRect);
    columns->Splitter.SetCurrentChannel(window->DrawList, 0);
}

void RblxUI::PopColumnsBackground()
{
    RblxUIWindow* window = GetCurrentWindowRead();
    RblxUIOldColumns* columns = window->DC.CurrentColumns;
    if (columns->Count == 1)
        return;

    // Optimization: avoid PopClipRect() + SetCurrentChannel()
    SetWindowClipRectBeforeSetChannel(window, columns->HostBackupClipRect);
    columns->Splitter.SetCurrentChannel(window->DrawList, columns->Current + 1);
}

RblxUIOldColumns* RblxUI::FindOrCreateColumns(RblxUIWindow* window, RblxUIID id)
{
    // We have few columns per window so for now we don't need bother much with turning this into a faster lookup.
    for (int n = 0; n < window->ColumnsStorage.Size; n++)
        if (window->ColumnsStorage[n].ID == id)
            return &window->ColumnsStorage[n];

    window->ColumnsStorage.push_back(RblxUIOldColumns());
    RblxUIOldColumns* columns = &window->ColumnsStorage.back();
    columns->ID = id;
    return columns;
}

RblxUIID RblxUI::GetColumnsID(const char* str_id, int columns_count)
{
    RblxUIWindow* window = GetCurrentWindow();

    // Differentiate column ID with an arbitrary prefix for cases where users name their columns set the same as another widget.
    // In addition, when an identifier isn't explicitly provided we include the number of columns in the hash to make it uniquer.
    PushID(0x11223347 + (str_id ? 0 : columns_count));
    RblxUIID id = window->GetID(str_id ? str_id : "columns");
    PopID();

    return id;
}

void RblxUI::BeginColumns(const char* str_id, int columns_count, RblxUIOldColumnFlags flags)
{
    RblxUIContext& g = *GRblxUI;
    RblxUIWindow* window = GetCurrentWindow();

    RBLXUI_ASSERT(columns_count >= 1);
    RBLXUI_ASSERT(window->DC.CurrentColumns == NULL);   // Nested columns are currently not supported

    // Acquire storage for the columns set
    RblxUIID id = GetColumnsID(str_id, columns_count);
    RblxUIOldColumns* columns = FindOrCreateColumns(window, id);
    RBLXUI_ASSERT(columns->ID == id);
    columns->Current = 0;
    columns->Count = columns_count;
    columns->Flags = flags;
    window->DC.CurrentColumns = columns;
    window->DC.NavIsScrollPushableX = false; // Shortcut for NavUpdateCurrentWindowIsScrollPushableX();

    columns->HostCursorPosY = window->DC.CursorPos.y;
    columns->HostCursorMaxPosX = window->DC.CursorMaxPos.x;
    columns->HostInitialClipRect = window->ClipRect;
    columns->HostBackupParentWorkRect = window->ParentWorkRect;
    window->ParentWorkRect = window->WorkRect;

    // Set state for first column
    // We aim so that the right-most column will have the same clipping width as other after being clipped by parent ClipRect
    const float column_padding = g.Style.ItemSpacing.x;
    const float half_clip_extend_x = RblxTrunc(RblxMax(window->WindowPadding.x * 0.5f, window->WindowBorderSize));
    const float max_1 = window->WorkRect.Max.x + column_padding - RblxMax(column_padding - window->WindowPadding.x, 0.0f);
    const float max_2 = window->WorkRect.Max.x + half_clip_extend_x;
    columns->OffMinX = window->DC.Indent.x - column_padding + RblxMax(column_padding - window->WindowPadding.x, 0.0f);
    columns->OffMaxX = RblxMax(RblxMin(max_1, max_2) - window->Pos.x, columns->OffMinX + 1.0f);
    columns->LineMinY = columns->LineMaxY = window->DC.CursorPos.y;

    // Clear data if columns count changed
    if (columns->Columns.Size != 0 && columns->Columns.Size != columns_count + 1)
        columns->Columns.resize(0);

    // Initialize default widths
    columns->IsFirstFrame = (columns->Columns.Size == 0);
    if (columns->Columns.Size == 0)
    {
        columns->Columns.reserve(columns_count + 1);
        for (int n = 0; n < columns_count + 1; n++)
        {
            RblxUIOldColumnData column;
            column.OffsetNorm = n / (float)columns_count;
            columns->Columns.push_back(column);
        }
    }

    for (int n = 0; n < columns_count; n++)
    {
        // Compute clipping rectangle
        RblxUIOldColumnData* column = &columns->Columns[n];
        float clip_x1 = RBLXUI_ROUND(window->Pos.x + GetColumnOffset(n));
        float clip_x2 = RBLXUI_ROUND(window->Pos.x + GetColumnOffset(n + 1) - 1.0f);
        column->ClipRect = RblxRect(clip_x1, -FLT_MAX, clip_x2, +FLT_MAX);
        column->ClipRect.ClipWithFull(window->ClipRect);
    }

    if (columns->Count > 1)
    {
        columns->Splitter.Split(window->DrawList, 1 + columns->Count);
        columns->Splitter.SetCurrentChannel(window->DrawList, 1);
        PushColumnClipRect(0);
    }

    // We don't generally store Indent.x inside ColumnsOffset because it may be manipulated by the user.
    float offset_0 = GetColumnOffset(columns->Current);
    float offset_1 = GetColumnOffset(columns->Current + 1);
    float width = offset_1 - offset_0;
    PushItemWidth(width * 0.65f);
    window->DC.ColumnsOffset.x = RblxMax(column_padding - window->WindowPadding.x, 0.0f);
    window->DC.CursorPos.x = RBLXUI_TRUNC(window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x);
    window->WorkRect.Max.x = window->Pos.x + offset_1 - column_padding;
    window->WorkRect.Max.y = window->ContentRegionRect.Max.y;
}

void RblxUI::NextColumn()
{
    RblxUIWindow* window = GetCurrentWindow();
    if (window->SkipItems || window->DC.CurrentColumns == NULL)
        return;

    RblxUIContext& g = *GRblxUI;
    RblxUIOldColumns* columns = window->DC.CurrentColumns;

    if (columns->Count == 1)
    {
        window->DC.CursorPos.x = RBLXUI_TRUNC(window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x);
        RBLXUI_ASSERT(columns->Current == 0);
        return;
    }

    // Next column
    if (++columns->Current == columns->Count)
        columns->Current = 0;

    PopItemWidth();

    // Optimization: avoid PopClipRect() + SetCurrentChannel() + PushClipRect()
    // (which would needlessly attempt to update commands in the wrong channel, then pop or overwrite them),
    RblxUIOldColumnData* column = &columns->Columns[columns->Current];
    SetWindowClipRectBeforeSetChannel(window, column->ClipRect);
    columns->Splitter.SetCurrentChannel(window->DrawList, columns->Current + 1);

    const float column_padding = g.Style.ItemSpacing.x;
    columns->LineMaxY = RblxMax(columns->LineMaxY, window->DC.CursorPos.y);
    if (columns->Current > 0)
    {
        // Columns 1+ ignore IndentX (by canceling it out)
        // FIXME-COLUMNS: Unnecessary, could be locked?
        window->DC.ColumnsOffset.x = GetColumnOffset(columns->Current) - window->DC.Indent.x + column_padding;
    }
    else
    {
        // New row/line: column 0 honor IndentX.
        window->DC.ColumnsOffset.x = RblxMax(column_padding - window->WindowPadding.x, 0.0f);
        window->DC.IsSameLine = false;
        columns->LineMinY = columns->LineMaxY;
    }
    window->DC.CursorPos.x = RBLXUI_TRUNC(window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x);
    window->DC.CursorPos.y = columns->LineMinY;
    window->DC.CurrLineSize = RblxVec2(0.0f, 0.0f);
    window->DC.CurrLineTextBaseOffset = 0.0f;

    // FIXME-COLUMNS: Share code with BeginColumns() - move code on columns setup.
    float offset_0 = GetColumnOffset(columns->Current);
    float offset_1 = GetColumnOffset(columns->Current + 1);
    float width = offset_1 - offset_0;
    PushItemWidth(width * 0.65f);
    window->WorkRect.Max.x = window->Pos.x + offset_1 - column_padding;
}

void RblxUI::EndColumns()
{
    RblxUIContext& g = *GRblxUI;
    RblxUIWindow* window = GetCurrentWindow();
    RblxUIOldColumns* columns = window->DC.CurrentColumns;
    RBLXUI_ASSERT(columns != NULL);

    PopItemWidth();
    if (columns->Count > 1)
    {
        PopClipRect();
        columns->Splitter.Merge(window->DrawList);
    }

    const RblxUIOldColumnFlags flags = columns->Flags;
    columns->LineMaxY = RblxMax(columns->LineMaxY, window->DC.CursorPos.y);
    window->DC.CursorPos.y = columns->LineMaxY;
    if (!(flags & RblxUIOldColumnFlags_GrowParentContentsSize))
        window->DC.CursorMaxPos.x = columns->HostCursorMaxPosX;  // Restore cursor max pos, as columns don't grow parent

    // Draw columns borders and handle resize
    // The IsBeingResized flag ensure we preserve pre-resize columns width so back-and-forth are not lossy
    bool is_being_resized = false;
    if (!(flags & RblxUIOldColumnFlags_NoBorder) && !window->SkipItems)
    {
        // We clip Y boundaries CPU side because very long triangles are mishandled by some GPU drivers.
        const float y1 = RblxMax(columns->HostCursorPosY, window->ClipRect.Min.y);
        const float y2 = RblxMin(window->DC.CursorPos.y, window->ClipRect.Max.y);
        int dragging_column = -1;
        for (int n = 1; n < columns->Count; n++)
        {
            RblxUIOldColumnData* column = &columns->Columns[n];
            float x = window->Pos.x + GetColumnOffset(n);
            const RblxUIID column_id = columns->ID + RblxUIID(n);
            const float column_hit_hw = RblxTrunc(COLUMNS_HIT_RECT_HALF_THICKNESS * g.CurrentDpiScale);
            const RblxRect column_hit_rect(RblxVec2(x - column_hit_hw, y1), RblxVec2(x + column_hit_hw, y2));
            if (!ItemAdd(column_hit_rect, column_id, NULL, RblxUIItemFlags_NoNav))
                continue;

            bool hovered = false, held = false;
            if (!(flags & RblxUIOldColumnFlags_NoResize))
            {
                ButtonBehavior(column_hit_rect, column_id, &hovered, &held);
                if (hovered || held)
                    SetMouseCursor(RblxUIMouseCursor_ResizeEW);
                if (held && !(column->Flags & RblxUIOldColumnFlags_NoResize))
                    dragging_column = n;
            }

            // Draw column
            const RblxU32 col = GetColorU32(held ? RblxUICol_SeparatorActive : hovered ? RblxUICol_SeparatorHovered : RblxUICol_Separator);
            const float xi = RBLXUI_TRUNC(x);
            window->DrawList->AddLineV(xi, y1 + 1.0f, y2, col);
        }

        // Apply dragging after drawing the column lines, so our rendered lines are in sync with how items were displayed during the frame.
        if (dragging_column != -1)
        {
            if (!columns->IsBeingResized)
                for (int n = 0; n < columns->Count + 1; n++)
                    columns->Columns[n].OffsetNormBeforeResize = columns->Columns[n].OffsetNorm;
            columns->IsBeingResized = is_being_resized = true;
            float x = GetDraggedColumnOffset(columns, dragging_column);
            SetColumnOffset(dragging_column, x);
        }
    }
    columns->IsBeingResized = is_being_resized;

    window->WorkRect = window->ParentWorkRect;
    window->ParentWorkRect = columns->HostBackupParentWorkRect;
    window->DC.CurrentColumns = NULL;
    window->DC.ColumnsOffset.x = 0.0f;
    window->DC.CursorPos.x = RBLXUI_TRUNC(window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x);
    NavUpdateCurrentWindowIsScrollPushableX();
}

void RblxUI::Columns(int columns_count, const char* id, bool borders)
{
    RblxUIWindow* window = GetCurrentWindow();
    RBLXUI_ASSERT(columns_count >= 1);

    RblxUIOldColumnFlags flags = (borders ? 0 : RblxUIOldColumnFlags_NoBorder);
    //flags |= RblxUIOldColumnFlags_NoPreserveWidths; // NB: Legacy behavior
    RblxUIOldColumns* columns = window->DC.CurrentColumns;
    if (columns != NULL && columns->Count == columns_count && columns->Flags == flags)
        return;

    if (columns != NULL)
        EndColumns();

    if (columns_count != 1)
        BeginColumns(id, columns_count, flags);
}

//-------------------------------------------------------------------------

#endif // #ifndef RBLXUI_DISABLE
