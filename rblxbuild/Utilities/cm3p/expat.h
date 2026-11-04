/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

/* Use the expat library configured for RBLXBuild.  */
#include "cmThirdParty.h"
#ifdef RBLXBUILD_USE_SYSTEM_EXPAT
#  include <expat.h> // IWYU pragma: export
#else
#  include <cmexpat/lib/expat.h> // IWYU pragma: export
#endif
