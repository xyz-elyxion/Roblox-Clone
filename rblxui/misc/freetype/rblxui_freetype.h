// dear rblxui: FreeType font builder (used as a replacement for the rblxstb_truetype builder)
// (headers)

#pragma once
#include "rblxui.h"      // RBLXUI_API
#ifndef RBLXUI_DISABLE

// Usage:
// - Add '#define RBLXUI_ENABLE_FREETYPE' in your rblxuiconfig to automatically enable support
//   for rblxui_freetype in rblxui. It is equivalent to selecting the default loader with:
//      io.Fonts->SetFontLoader(RblxUIFreeType::GetFontLoader())

// Optional support for OpenType SVG fonts:
// - Add '#define RBLXUI_ENABLE_FREETYPE_PLUTOSVG' to use plutosvg (not provided). See #7927.
// - Add '#define RBLXUI_ENABLE_FREETYPE_LUNASVG' to use lunasvg (not provided). See #6591.

// Forward declarations
struct RblxFontAtlas;
struct RblxFontLoader;

// Hinting greatly impacts visuals (and glyph sizes).
// - By default, hinting is enabled and the font's native hinter is preferred over the auto-hinter.
// - When disabled, FreeType generates blurrier glyphs, more or less matches the rblxstb_truetype.h
// - The Default hinting mode usually looks good, but may distort glyphs in an unusual way.
// - The Light hinting mode generates fuzzier glyphs but better matches Microsoft's rasterizer.
// You can set those flags globally in RblxFontAtlas::FontLoaderFlags
// You can set those flags on a per font basis in RblxFontConfig::FontLoaderFlags
typedef unsigned int RblxUIFreeTypeLoaderFlags;
enum RblxUIFreeTypeLoaderFlags_
{
    RblxUIFreeTypeLoaderFlags_NoHinting     = 1 << 0,   // Disable hinting. This generally generates 'blurrier' bitmap glyphs when the glyph are rendered in any of the anti-aliased modes.
    RblxUIFreeTypeLoaderFlags_NoAutoHint    = 1 << 1,   // Disable auto-hinter.
    RblxUIFreeTypeLoaderFlags_ForceAutoHint = 1 << 2,   // Indicates that the auto-hinter is preferred over the font's native hinter.
    RblxUIFreeTypeLoaderFlags_LightHinting  = 1 << 3,   // A lighter hinting algorithm for gray-level modes. Many generated glyphs are fuzzier but better resemble their original shape. This is achieved by snapping glyphs to the pixel grid only vertically (Y-axis), as is done by Microsoft's ClearType and Adobe's proprietary font renderer. This preserves inter-glyph spacing in horizontal text.
    RblxUIFreeTypeLoaderFlags_MonoHinting   = 1 << 4,   // Strong hinting algorithm that should only be used for monochrome output.
    RblxUIFreeTypeLoaderFlags_Bold          = 1 << 5,   // Styling: Should we artificially embolden the font?
    RblxUIFreeTypeLoaderFlags_Oblique       = 1 << 6,   // Styling: Should we slant the font, emulating italic style?
    RblxUIFreeTypeLoaderFlags_Monochrome    = 1 << 7,   // Disable anti-aliasing. Combine this with MonoHinting for best results!
    RblxUIFreeTypeLoaderFlags_LoadColor     = 1 << 8,   // Enable FreeType color-layered glyphs
    RblxUIFreeTypeLoaderFlags_Bitmap        = 1 << 9,   // Enable FreeType bitmap glyphs

#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    RblxUIFreeTypeBuilderFlags_NoHinting     = RblxUIFreeTypeLoaderFlags_NoHinting,
    RblxUIFreeTypeBuilderFlags_NoAutoHint    = RblxUIFreeTypeLoaderFlags_NoAutoHint,
    RblxUIFreeTypeBuilderFlags_ForceAutoHint = RblxUIFreeTypeLoaderFlags_ForceAutoHint,
    RblxUIFreeTypeBuilderFlags_LightHinting  = RblxUIFreeTypeLoaderFlags_LightHinting,
    RblxUIFreeTypeBuilderFlags_MonoHinting   = RblxUIFreeTypeLoaderFlags_MonoHinting,
    RblxUIFreeTypeBuilderFlags_Bold          = RblxUIFreeTypeLoaderFlags_Bold,
    RblxUIFreeTypeBuilderFlags_Oblique       = RblxUIFreeTypeLoaderFlags_Oblique,
    RblxUIFreeTypeBuilderFlags_Monochrome    = RblxUIFreeTypeLoaderFlags_Monochrome,
    RblxUIFreeTypeBuilderFlags_LoadColor     = RblxUIFreeTypeLoaderFlags_LoadColor,
    RblxUIFreeTypeBuilderFlags_Bitmap        = RblxUIFreeTypeLoaderFlags_Bitmap,
#endif
};

// Obsolete names (will be removed)
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
typedef RblxUIFreeTypeLoaderFlags_ RblxUIFreeTypeBuilderFlags_;
#endif

namespace RblxUIFreeType
{
    // This is automatically assigned when using '#define RBLXUI_ENABLE_FREETYPE'.
    // If you need to dynamically select between multiple builders:
    // - you can manually assign this builder with 'atlas->SetFontLoader(RblxUIFreeType::GetFontLoader())'
    // - prefer deep-copying this into your own RblxFontLoader instance if you use hot-reloading that messes up static data.
    RBLXUI_API const RblxFontLoader*       GetFontLoader();

    // Override allocators. By default RblxUIFreeType will use RBLXUI_ALLOC()/RBLXUI_FREE()
    // However, as FreeType does lots of allocations we provide a way for the user to redirect it to a separate memory heap if desired.
    RBLXUI_API void                      SetAllocatorFunctions(void* (*alloc_func)(size_t sz, void* user_data), void (*free_func)(void* ptr, void* user_data), void* user_data = nullptr);

    // Display UI to edit RblxFontAtlas::FontLoaderFlags (shared) or RblxFontConfig::FontLoaderFlags (single source)
    RBLXUI_API bool                      DebugEditFontLoaderFlags(RblxUIFreeTypeLoaderFlags* p_font_loader_flags);

    // Obsolete names (will be removed)
#ifndef RBLXUI_DISABLE_OBSOLETE_FUNCTIONS
    //RBLXUI_API const RblxFontBuilderIO* GetBuilderForFreeType(); // Renamed/changed in 1.92. Change 'io.Fonts->FontBuilderIO = RblxUIFreeType::GetBuilderForFreeType()' to 'io.Fonts->SetFontLoader(RblxUIFreeType::GetFontLoader())' if you need runtime selection.
    //static inline bool BuildFontAtlas(RblxFontAtlas* atlas, unsigned int flags = 0) { atlas->FontBuilderIO = GetBuilderForFreeType(); atlas->FontLoaderFlags = flags; return atlas->Build(); } // Prefer using '#define RBLXUI_ENABLE_FREETYPE'
#endif
}

#endif // #ifndef RBLXUI_DISABLE
