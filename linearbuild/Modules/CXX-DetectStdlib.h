#include <version>
// clang-format off
#if defined(_LIBCPP_VERSION)
LINEARBUILD-STDLIB-DETECT: libc++
#elif defined(__GLIBCXX__)
LINEARBUILD-STDLIB-DETECT: libstdc++
#elif defined(_MSVC_STL_VERSION)
LINEARBUILD-STDLIB-DETECT: msvc
#else
LINEARBUILD-STDLIB-DETECT: UNKNOWN
#endif
   // clang-format on
