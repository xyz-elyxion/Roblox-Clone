/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include <cm/optional>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QtGlobal>

#include "QRBLXBuildPreset.h"

class QObject;

class QRBLXBuildPresetItemModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  QRBLXBuildPresetItemModel(QObject* parent = nullptr);

  QVariant data(QModelIndex const& index, int role) const override;
  Qt::ItemFlags flags(QModelIndex const& index) const override;

  int rowCount(QModelIndex const& parent = QModelIndex{}) const override;
  int columnCount(QModelIndex const& parent = QModelIndex{}) const override;

  QModelIndex index(int row, int column,
                    QModelIndex const& parent = QModelIndex{}) const override;
  QModelIndex parent(QModelIndex const& index) const override;

  QVector<QRBLXBuildPreset> const& presets() const;

  int presetNameToRow(QString const& name) const;

public slots:
  void setPresets(QVector<QRBLXBuildPreset> const& presets);

private:
  QVector<QRBLXBuildPreset> m_presets;

  static constexpr quintptr SEPARATOR_INDEX = static_cast<quintptr>(-2);
  static constexpr quintptr CUSTOM_INDEX = static_cast<quintptr>(-1);
};
