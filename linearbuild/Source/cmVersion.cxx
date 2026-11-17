/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#include "cmVersion.h"

#include "cmVersionConfig.h"

unsigned int cmVersion::GetMajorVersion()
{
  return LinearBuild_VERSION_MAJOR;
}
unsigned int cmVersion::GetMinorVersion()
{
  return LinearBuild_VERSION_MINOR;
}
unsigned int cmVersion::GetPatchVersion()
{
  return LinearBuild_VERSION_PATCH;
}
unsigned int cmVersion::GetTweakVersion()
{
  return 0;
}

char const* cmVersion::GetLinearBuildVersion()
{
  return LinearBuild_VERSION;
}
