/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include <QComboBox>
#include <QObject>
#include <QString>
#include <QVector>

#include "QRBLXBuildPreset.h"

class QRBLXBuildPresetItemModel;

class QRBLXBuildPresetComboBox : public QComboBox
{
  Q_OBJECT
public:
  QRBLXBuildPresetComboBox(QWidget* parent = nullptr);

  QVector<QRBLXBuildPreset> const& presets() const;
  QString presetName() const;

public slots:
  void setPresets(QVector<QRBLXBuildPreset> const& presets);
  void setPresetName(QString const& name);

signals:
  void presetChanged(QString const& name);

private:
  QRBLXBuildPresetItemModel* m_model;
  bool m_resetting = false;
  QString m_lastPreset;

  void emitPresetChanged();
};
