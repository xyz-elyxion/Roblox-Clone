/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief rblxbuild_minimum_required command
 *
 * cmRBLXBuildMinimumRequired implements the rblxbuild_minimum_required RBLXBuild command
 */
bool cmRBLXBuildMinimumRequired(std::vector<std::string> const& args,
                            cmExecutionStatus& status);
