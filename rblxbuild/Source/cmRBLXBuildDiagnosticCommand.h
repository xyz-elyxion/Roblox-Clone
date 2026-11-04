/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Set how RBLXBuild should handle diagnostics
 *
 * cmRBLXBuildDiagnosticCommand sets how RBLXBuild should deal with diagnostics.
 */
bool cmRBLXBuildDiagnosticCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status);
