/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <vector>

class cmExecutionStatus;
struct cmListFileArgument;

/**
 * \brief Calls a scripted or built-in command
 *
 */
bool cmLinearBuildLanguageCommand(std::vector<cmListFileArgument> const& args,
                            cmExecutionStatus& status);
