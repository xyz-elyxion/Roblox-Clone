/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */

#ifndef cmLinearBuildCompat_h
#define cmLinearBuildCompat_h

#include <string>

// LinearBuild was forked from a build tool whose commands, variables and modules
// were spelled with a different prefix. Our own project and, more importantly,
// the third-party dependencies it fetches with FetchContent still use that
// spelling. Mapping the two into each other here keeps those build files usable
// unmodified, instead of requiring every dependency to be rewritten.
//
// The mapping is single step in both directions, so callers may apply it to
// either spelling and get the other one back.
inline std::string cmLinearBuildLegacyAlias(std::string const& name)
{
  struct Mapping
  {
    char const* from;
    char const* to;
  };
  static Mapping const mappings[] = {
    { "CMAKE_", "LINEARBUILD_" },
    { "LINEARBUILD_", "CMAKE_" },
    { "CMake_", "LinearBuild_" },
    { "LinearBuild_", "CMake_" },
    { "cmake_", "linearbuild_" },
    { "linearbuild_", "cmake_" },
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
