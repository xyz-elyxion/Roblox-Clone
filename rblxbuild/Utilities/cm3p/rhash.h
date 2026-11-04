/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

/* Use the LibRHash library configured for RBLXBuild.  */
#include "cmThirdParty.h"
#ifdef RBLXBUILD_USE_SYSTEM_LIBRHASH
#  include <rhash.h> // IWYU pragma: export
#else
#  include <cmlibrhash/librhash/rhash.h> // IWYU pragma: export
#endif
