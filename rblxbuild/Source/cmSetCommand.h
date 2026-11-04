/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Set a RBLXBUILD variable
 *
 * cmSetCommand sets a variable to a value with expansion.
 */
bool cmSetCommand(std::vector<std::string> const& args,
                  cmExecutionStatus& status);
