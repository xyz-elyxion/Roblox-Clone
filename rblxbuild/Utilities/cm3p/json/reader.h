/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

/* Use the jsoncpp library configured for RBLXBuild.  */
#include "cmThirdParty.h"
#ifdef RBLXBUILD_USE_SYSTEM_JSONCPP
#  include <json/reader.h> // IWYU pragma: export
#else
#  include <cmjsoncpp/include/json/reader.h> // IWYU pragma: export
#endif
