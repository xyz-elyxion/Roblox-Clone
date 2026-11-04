/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include "cmsys/SystemTools.hxx"

// Use the make system's VERBOSE environment variable to enable
// verbose output. This can be skipped by also setting RBLXBUILD_NO_VERBOSE
// (which is set by the Eclipse generator).
inline bool isRBLXBuildVerbose()
{
  return (cmSystemTools::HasEnv("VERBOSE") &&
          !cmSystemTools::HasEnv("RBLXBUILD_NO_VERBOSE"));
}
