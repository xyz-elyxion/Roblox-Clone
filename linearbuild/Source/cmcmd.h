/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include <cm/optional>

namespace cm {
namespace StdIo {
class Console;
}
}

namespace cmcmd {
/**
 * Execute commands during the build process. Supports options such
 * as echo, remove file etc.
 */
int ExecuteLinearBuildCommand(std::vector<std::string> args,
                        cm::optional<cm::StdIo::Console> console);
}
