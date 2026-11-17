/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

/* Use the bzip2 library configured for LinearBuild.  */
#include "cmThirdParty.h"
#ifdef LINEARBUILD_USE_SYSTEM_BZIP2
#  include <bzlib.h> // IWYU pragma: export
#else
#  include <cmbzip2/bzlib.h> // IWYU pragma: export
#endif
