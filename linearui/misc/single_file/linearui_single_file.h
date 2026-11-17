// dear linearui: single-file wrapper include
// We use this to validate compiling all *.cpp files in a same compilation unit.
// Users of that technique (also called "Unity builds") can generally provide this themselves,
// so we don't really recommend you use this in your projects.

// Do this:
//    #define LINEARUI_IMPLEMENTATION
// Before you include this file in *one* C++ file to create the implementation.
// Using this in your project will leak the contents of linearui_internal.h and LinearVec2 operators in this compilation unit.

#ifdef LINEARUI_IMPLEMENTATION
#define LINEARUI_DEFINE_MATH_OPERATORS
#endif

#include "../../linearui.h"
#ifdef LINEARUI_ENABLE_FREETYPE
#include "../../misc/freetype/linearui_freetype.h"
#endif

#ifdef LINEARUI_IMPLEMENTATION
#include "../../linearui.cpp"
#include "../../linearui_demo.cpp"
#include "../../linearui_draw.cpp"
#include "../../linearui_tables.cpp"
#include "../../linearui_widgets.cpp"
#ifdef LINEARUI_ENABLE_FREETYPE
#include "../../misc/freetype/linearui_freetype.cpp"
#endif
#endif
