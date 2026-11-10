// dear rblxui: single-file wrapper include
// We use this to validate compiling all *.cpp files in a same compilation unit.
// Users of that technique (also called "Unity builds") can generally provide this themselves,
// so we don't really recommend you use this in your projects.

// Do this:
//    #define RBLXUI_IMPLEMENTATION
// Before you include this file in *one* C++ file to create the implementation.
// Using this in your project will leak the contents of rblxui_internal.h and RblxVec2 operators in this compilation unit.

#ifdef RBLXUI_IMPLEMENTATION
#define RBLXUI_DEFINE_MATH_OPERATORS
#endif

#include "../../rblxui.h"
#ifdef RBLXUI_ENABLE_FREETYPE
#include "../../misc/freetype/rblxui_freetype.h"
#endif

#ifdef RBLXUI_IMPLEMENTATION
#include "../../rblxui.cpp"
#include "../../rblxui_demo.cpp"
#include "../../rblxui_draw.cpp"
#include "../../rblxui_tables.cpp"
#include "../../rblxui_widgets.cpp"
#ifdef RBLXUI_ENABLE_FREETYPE
#include "../../misc/freetype/rblxui_freetype.cpp"
#endif
#endif
