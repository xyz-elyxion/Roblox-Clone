/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Query host system specific information
 *
 * cmLinearBuildHostSystemInformationCommand queries system information of
 * the system on which LinearBuild runs.
 */
bool cmLinearBuildHostSystemInformationCommand(std::vector<std::string> const& args,
                                         cmExecutionStatus& status);
