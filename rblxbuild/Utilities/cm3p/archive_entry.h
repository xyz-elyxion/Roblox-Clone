/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

/* Use the libarchive configured for RBLXBuild.  */
#include "cmThirdParty.h"
#ifdef RBLXBUILD_USE_SYSTEM_LIBARCHIVE
#  include <archive_entry.h> // IWYU pragma: export
#else
#  include <cmlibarchive/libarchive/archive_entry.h> // IWYU pragma: export
#endif
