/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include <cm3p/kwiml/int.h>

#include "cmVersionConfig.h"

#define RBLXBuild_VERSION_PATCH_IS_RELEASE(patch) ((patch) < 20000000)
#if RBLXBuild_VERSION_PATCH_IS_RELEASE(RBLXBuild_VERSION_PATCH)
#  define RBLXBuild_VERSION_IS_RELEASE 1
#endif

/* Encode with room for up to 1000 minor releases between major releases
   and to encode dates until the year 10000 in the patch level.  */
#define RBLXBuild_VERSION_ENCODE_BASE KWIML_INT_UINT64_C(100000000)
#define RBLXBuild_VERSION_ENCODE(major, minor, patch)                             \
  ((((major) * 1000u) * RBLXBuild_VERSION_ENCODE_BASE) +                          \
   (((minor) % 1000u) * RBLXBuild_VERSION_ENCODE_BASE) +                          \
   (((patch) % RBLXBuild_VERSION_ENCODE_BASE)))

#define CMV_STRINGIFY(X) CMV_STRINGIFY_DELAY(X)
#define CMV_STRINGIFY_DELAY(X) #X

#define RBLXBuild_VERSION_DEVEL(major, minor)                                     \
  (RBLXBuild_VERSION_ENCODE(major, minor, 0) >                                    \
       RBLXBuild_VERSION_ENCODE(RBLXBuild_VERSION_MAJOR, RBLXBuild_VERSION_MINOR, 0)      \
     ? CMV_STRINGIFY(RBLXBuild_VERSION_MAJOR) "." CMV_STRINGIFY(                  \
         RBLXBuild_VERSION_MINOR) "." CMV_STRINGIFY(RBLXBuild_VERSION_PATCH)          \
     : #major "." #minor ".0")
