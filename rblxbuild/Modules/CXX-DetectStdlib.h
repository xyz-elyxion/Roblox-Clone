#include <version>
// clang-format off
#if defined(_LIBCPP_VERSION)
RBLXBUILD-STDLIB-DETECT: libc++
#elif defined(__GLIBCXX__)
RBLXBUILD-STDLIB-DETECT: libstdc++
#elif defined(_MSVC_STL_VERSION)
RBLXBUILD-STDLIB-DETECT: msvc
#else
RBLXBUILD-STDLIB-DETECT: UNKNOWN
#endif
   // clang-format on
