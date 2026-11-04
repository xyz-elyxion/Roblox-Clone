/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include "cmVersion.h"

#include "cmVersionConfig.h"

unsigned int cmVersion::GetMajorVersion()
{
  return RBLXBuild_VERSION_MAJOR;
}
unsigned int cmVersion::GetMinorVersion()
{
  return RBLXBuild_VERSION_MINOR;
}
unsigned int cmVersion::GetPatchVersion()
{
  return RBLXBuild_VERSION_PATCH;
}
unsigned int cmVersion::GetTweakVersion()
{
  return 0;
}

char const* cmVersion::GetRBLXBuildVersion()
{
  return RBLXBuild_VERSION;
}
