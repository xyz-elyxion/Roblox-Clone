/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include <QComboBox>
#include <QObject>
#include <QString>
#include <QVector>

#include "QLinearBuildPreset.h"

class QLinearBuildPresetItemModel;

class QLinearBuildPresetComboBox : public QComboBox
{
  Q_OBJECT
public:
  QLinearBuildPresetComboBox(QWidget* parent = nullptr);

  QVector<QLinearBuildPreset> const& presets() const;
  QString presetName() const;

public slots:
  void setPresets(QVector<QLinearBuildPreset> const& presets);
  void setPresetName(QString const& name);

signals:
  void presetChanged(QString const& name);

private:
  QLinearBuildPresetItemModel* m_model;
  bool m_resetting = false;
  QString m_lastPreset;

  void emitPresetChanged();
};
