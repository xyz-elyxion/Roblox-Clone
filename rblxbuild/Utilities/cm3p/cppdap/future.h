/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

/* Use the cppdap library configured for RBLXBuild.  */
#include "cmThirdParty.h"
#ifdef RBLXBUILD_USE_SYSTEM_CPPDAP
#  include <dap/future.h> // IWYU pragma: export
#else
#  include <cmcppdap/include/dap/future.h> // IWYU pragma: export
#endif
