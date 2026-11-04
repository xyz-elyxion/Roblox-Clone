/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

/* Use the nghttp2 library configured for RBLXBuild.  */
#include "cmThirdParty.h"
#ifdef RBLXBUILD_USE_SYSTEM_NGHTTP2
#  include <nghttp2/nghttp2.h> // IWYU pragma: export
#else
#  include <cmnghttp2/lib/includes/nghttp2/nghttp2.h> // IWYU pragma: export
#endif
