/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

/* Use the libuv library configured for LinearBuild.  */
#include "cmThirdParty.h"
#ifdef LINEARBUILD_USE_SYSTEM_LIBUV
#  include <uv.h> // IWYU pragma: export
#else
#  include <cmlibuv/include/uv.h> // IWYU pragma: export
#endif
