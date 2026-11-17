/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief linearbuild_minimum_required command
 *
 * cmLinearBuildMinimumRequired implements the linearbuild_minimum_required LinearBuild command
 */
bool cmLinearBuildMinimumRequired(std::vector<std::string> const& args,
                            cmExecutionStatus& status);
