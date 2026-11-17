/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

/* Use the expat library configured for LinearBuild.  */
#include "cmThirdParty.h"
#ifdef LINEARBUILD_USE_SYSTEM_EXPAT
#  include <expat.h> // IWYU pragma: export
#else
#  include <cmexpat/lib/expat.h> // IWYU pragma: export
#endif
