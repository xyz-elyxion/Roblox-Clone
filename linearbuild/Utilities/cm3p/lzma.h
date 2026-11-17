/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

/* Use the liblzma configured for LinearBuild.  */
#include "cmThirdParty.h"
#ifdef LINEARBUILD_USE_SYSTEM_LIBLZMA
#  include <lzma.h> // IWYU pragma: export
#else
#  include <cmliblzma/liblzma/api/lzma.h> // IWYU pragma: export
#endif
