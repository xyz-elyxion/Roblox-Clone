/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

/* Use the zlib library configured for RBLXBuild.  */
#include "cmThirdParty.h"
#ifdef RBLXBUILD_USE_SYSTEM_ZLIB
#  include <zlib.h> // IWYU pragma: export
#else
#  include <cmzlib/zlib.h> // IWYU pragma: export
#endif
