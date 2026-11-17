/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include <QCheckBox>
#include <QStringList>
#include <QWidget>

#include "QLinearBuild.h"
#include "ui_AddCacheEntry.h"

class AddCacheEntry
  : public QWidget
  , public Ui::AddCacheEntry
{
  Q_OBJECT
public:
  AddCacheEntry(QWidget* p, QStringList const& varNames,
                QStringList const& varTypes);

  QString name() const;
  QVariant value() const;
  QString description() const;
  QLinearBuildProperty::PropertyType type() const;
  QString typeString() const;

private slots:
  void onCompletionActivated(QString const& text);

private:
  QStringList const& VarNames;
  QStringList const& VarTypes;
};
