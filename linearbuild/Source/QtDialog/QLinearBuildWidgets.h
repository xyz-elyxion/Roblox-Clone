/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <QComboBox>
#include <QCompleter>
#include <QLineEdit>

class QToolButton;
class QSortFilterProxyModel;

// common widgets for Qt based LinearBuild

/// Editor widget for editing paths or file paths
class QLinearBuildFileEditor : public QLineEdit
{
  Q_OBJECT
public:
  QLinearBuildFileEditor(QWidget* p, QString var);
protected slots:
  virtual void chooseFile() = 0;
signals:
  void fileDialogExists(bool);

protected:
  void resizeEvent(QResizeEvent* e);
  QToolButton* ToolButton;
  QString Variable;
};

/// editor widget for editing files
class QLinearBuildPathEditor : public QLinearBuildFileEditor
{
  Q_OBJECT
public:
  QLinearBuildPathEditor(QWidget* p = nullptr, QString const& var = QString());
  void chooseFile();
};

/// editor widget for editing paths
class QLinearBuildFilePathEditor : public QLinearBuildFileEditor
{
  Q_OBJECT
public:
  QLinearBuildFilePathEditor(QWidget* p = nullptr, QString const& var = QString());
  void chooseFile();
};

/// completer class that returns native linearbuild paths
class QLinearBuildFileCompleter : public QCompleter
{
  Q_OBJECT
public:
  QLinearBuildFileCompleter(QObject* o, bool dirs);
  virtual QString pathFromIndex(QModelIndex const& idx) const;
};

// editor for strings
class QLinearBuildComboBox : public QComboBox
{
  Q_OBJECT
  Q_PROPERTY(QString value READ currentText WRITE setValue USER true);

public:
  QLinearBuildComboBox(QWidget* p, QStringList strings)
    : QComboBox(p)
  {
    this->addItems(strings);
  }
  void setValue(QString const& v)
  {
    int i = this->findText(v);
    if (i != -1) {
      this->setCurrentIndex(i);
    }
  }
};

namespace QtLinearBuild {
bool setSearchFilter(QSortFilterProxyModel* model,
                     QString const& searchString);

void setSearchFilterColor(QLineEdit* edit, bool valid);
}
