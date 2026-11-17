/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

/* Use the libzstd configured for LinearBuild.  */
#include "cmThirdParty.h"
#ifdef LINEARBUILD_USE_SYSTEM_ZSTD
#  include <zstd.h> // IWYU pragma: export
#else
#  include <cmzstd/lib/zstd.h> // IWYU pragma: export
#endif
