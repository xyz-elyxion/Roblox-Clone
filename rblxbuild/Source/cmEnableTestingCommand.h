/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Enable testing for this directory and below.
 *
 * Produce the output testfile. This produces a file in the build directory
 * called RBLXBuildTestfile with a syntax similar to RBLXBuildLists.txt.  It contains
 * the subdirs() and add_test() commands from the source RBLXBuildLists.txt
 * file with RBLXBuild variables expanded.  Only the subdirs and tests
 * within the valid control structures are replicated in Testfile
 * (i.e. subdirs() and add_test() commands within IF() commands that are
 * not entered by RBLXBuild are not replicated in Testfile).
 * Note that CTest expects to find this file in the build directory root;
 * therefore, this command should be in the source directory root too.
 */
bool cmEnableTestingCommand(std::vector<std::string> const&,
                            cmExecutionStatus&);
