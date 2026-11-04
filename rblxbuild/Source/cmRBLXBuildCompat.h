/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */

#ifndef cmRBLXBuildCompat_h
#define cmRBLXBuildCompat_h

#include <string>

// RBLXBuild was forked from a build tool whose commands, variables and modules
// were spelled with a different prefix. Our own project and, more importantly,
// the third-party dependencies it fetches with FetchContent still use that
// spelling. Mapping the two into each other here keeps those build files usable
// unmodified, instead of requiring every dependency to be rewritten.
//
// The mapping is single step in both directions, so callers may apply it to
// either spelling and get the other one back.
inline std::string cmRBLXBuildLegacyAlias(std::string const& name)
{
  struct Mapping
  {
    char const* from;
    char const* to;
  };
  static Mapping const mappings[] = {
    { "CMAKE_", "RBLXBUILD_" },
    { "RBLXBUILD_", "CMAKE_" },
    { "CMake_", "RBLXBuild_" },
    { "RBLXBuild_", "CMake_" },
    { "cmake_", "rblxbuild_" },
    { "rblxbuild_", "cmake_" },
  };
  for (Mapping const& m : mappings) {
    std::string const from = m.from;
    if (name.size() > from.size() && name.compare(0, from.size(), from) == 0) {
      return std::string(m.to) + name.substr(from.size());
    }
  }
  return std::string();
}

#endif
