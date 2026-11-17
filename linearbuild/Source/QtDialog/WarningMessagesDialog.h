/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include <map>

#include <QDialog>
#include <QWidget>

#include "ui_WarningMessagesDialog.h"

class QButtonGroup;

class QLinearBuild;

/**
 * Dialog window for setting the warning message related options.
 */
class WarningMessagesDialog
  : public QDialog
  , public Ui_MessagesDialog
{
  Q_OBJECT

public:
  WarningMessagesDialog(QWidget* prnt, QLinearBuild* instance);

private slots:
  /**
   * Handler for the accept event of the ok/cancel button box.
   */
  void doAccept();

private:
  QLinearBuild* linearbuildInstance;
  std::map<unsigned, QButtonGroup*> buttons;

  /**
   * Set the initial values of the widgets on this dialog window, using the
   * current state of the cache.
   */
  void setInitialValues();

  /**
   * Setup the signals for the widgets on this dialog window.
   */
  void setupSignals();
};
