/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include <cm/optional>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QtGlobal>

#include "QLinearBuildPreset.h"

class QObject;

class QLinearBuildPresetItemModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  QLinearBuildPresetItemModel(QObject* parent = nullptr);

  QVariant data(QModelIndex const& index, int role) const override;
  Qt::ItemFlags flags(QModelIndex const& index) const override;

  int rowCount(QModelIndex const& parent = QModelIndex{}) const override;
  int columnCount(QModelIndex const& parent = QModelIndex{}) const override;

  QModelIndex index(int row, int column,
                    QModelIndex const& parent = QModelIndex{}) const override;
  QModelIndex parent(QModelIndex const& index) const override;

  QVector<QLinearBuildPreset> const& presets() const;

  int presetNameToRow(QString const& name) const;

public slots:
  void setPresets(QVector<QLinearBuildPreset> const& presets);

private:
  QVector<QLinearBuildPreset> m_presets;

  static constexpr quintptr SEPARATOR_INDEX = static_cast<quintptr>(-2);
  static constexpr quintptr CUSTOM_INDEX = static_cast<quintptr>(-1);
};
