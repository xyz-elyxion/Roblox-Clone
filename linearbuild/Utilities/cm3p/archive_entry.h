/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

/* Use the libarchive configured for LinearBuild.  */
#include "cmThirdParty.h"
#ifdef LINEARBUILD_USE_SYSTEM_LIBARCHIVE
#  include <archive_entry.h> // IWYU pragma: export
#else
#  include <cmlibarchive/libarchive/archive_entry.h> // IWYU pragma: export
#endif
