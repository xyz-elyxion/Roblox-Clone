/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */

#include <QApplication>

class RBLXBuildSetupDialog;

void SetupDefaultQSettings()
{
}

int RBLXBuildGUIExec(RBLXBuildSetupDialog* /*window*/)
{
  return QApplication::exec();
}
