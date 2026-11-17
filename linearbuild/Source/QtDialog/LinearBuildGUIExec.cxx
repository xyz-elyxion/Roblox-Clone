/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */

#include <QApplication>

class LinearBuildSetupDialog;

void SetupDefaultQSettings()
{
}

int LinearBuildGUIExec(LinearBuildSetupDialog* /*window*/)
{
  return QApplication::exec();
}
