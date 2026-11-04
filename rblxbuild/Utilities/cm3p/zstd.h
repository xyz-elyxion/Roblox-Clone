/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

/* Use the libzstd configured for RBLXBuild.  */
#include "cmThirdParty.h"
#ifdef RBLXBUILD_USE_SYSTEM_ZSTD
#  include <zstd.h> // IWYU pragma: export
#else
#  include <cmzstd/lib/zstd.h> // IWYU pragma: export
#endif
