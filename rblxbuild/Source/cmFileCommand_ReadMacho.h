/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include <string>
#include <vector>

class cmExecutionStatus;

bool HandleReadMachoCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status);
