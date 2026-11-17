/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include <cm3p/kwiml/int.h>

#include "cmVersionConfig.h"

#define LinearBuild_VERSION_PATCH_IS_RELEASE(patch) ((patch) < 20000000)
#if LinearBuild_VERSION_PATCH_IS_RELEASE(LinearBuild_VERSION_PATCH)
#  define LinearBuild_VERSION_IS_RELEASE 1
#endif

/* Encode with room for up to 1000 minor releases between major releases
   and to encode dates until the year 10000 in the patch level.  */
#define LinearBuild_VERSION_ENCODE_BASE KWIML_INT_UINT64_C(100000000)
#define LinearBuild_VERSION_ENCODE(major, minor, patch)                             \
  ((((major) * 1000u) * LinearBuild_VERSION_ENCODE_BASE) +                          \
   (((minor) % 1000u) * LinearBuild_VERSION_ENCODE_BASE) +                          \
   (((patch) % LinearBuild_VERSION_ENCODE_BASE)))

#define CMV_STRINGIFY(X) CMV_STRINGIFY_DELAY(X)
#define CMV_STRINGIFY_DELAY(X) #X

#define LinearBuild_VERSION_DEVEL(major, minor)                                     \
  (LinearBuild_VERSION_ENCODE(major, minor, 0) >                                    \
       LinearBuild_VERSION_ENCODE(LinearBuild_VERSION_MAJOR, LinearBuild_VERSION_MINOR, 0)      \
     ? CMV_STRINGIFY(LinearBuild_VERSION_MAJOR) "." CMV_STRINGIFY(                  \
         LinearBuild_VERSION_MINOR) "." CMV_STRINGIFY(LinearBuild_VERSION_PATCH)          \
     : #major "." #minor ".0")
