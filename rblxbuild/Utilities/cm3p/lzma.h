/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

/* Use the liblzma configured for RBLXBuild.  */
#include "cmThirdParty.h"
#ifdef RBLXBUILD_USE_SYSTEM_LIBLZMA
#  include <lzma.h> // IWYU pragma: export
#else
#  include <cmliblzma/liblzma/api/lzma.h> // IWYU pragma: export
#endif
