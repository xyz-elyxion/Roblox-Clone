/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Command that adds a target to the build system.
 *
 * cmExecuteProcessCommand is a RBLXBuild language interface to the KWSys
 * Process Execution implementation.
 */
bool cmExecuteProcessCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status);
