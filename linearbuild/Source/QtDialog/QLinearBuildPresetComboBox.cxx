/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#include "QLinearBuildPresetComboBox.h"

#include "QLinearBuildPresetItemModel.h"

QLinearBuildPresetComboBox::QLinearBuildPresetComboBox(QWidget* parent)
  : QComboBox(parent)
{
  this->m_model = new QLinearBuildPresetItemModel(this);
  this->setModel(this->m_model);

  QObject::connect(this->m_model, &QLinearBuildPresetItemModel::modelAboutToBeReset,
                   this, [this]() { this->m_resetting = true; });
  QObject::connect(this->m_model, &QLinearBuildPresetItemModel::modelReset, this,
                   [this]() {
                     this->setPresetName(this->m_lastPreset);
                     this->m_resetting = false;
                     this->emitPresetChanged();
                   });
  QObject::connect(
    this,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    this, [this](int /*row*/) {
      if (!this->m_resetting) {
        this->emitPresetChanged();
      }
    });
}

QVector<QLinearBuildPreset> const& QLinearBuildPresetComboBox::presets() const
{
  return this->m_model->presets();
}

QString QLinearBuildPresetComboBox::presetName() const
{
  auto preset = this->currentData();
  if (preset.canConvert<QLinearBuildPreset>()) {
    return preset.value<QLinearBuildPreset>().name;
  }
  return QString{};
}

void QLinearBuildPresetComboBox::setPresets(QVector<QLinearBuildPreset> const& presets)
{
  this->m_model->setPresets(presets);
}

void QLinearBuildPresetComboBox::setPresetName(QString const& name)
{
  this->setCurrentIndex(this->m_model->presetNameToRow(name));
  if (this->signalsBlocked()) {
    this->m_lastPreset = this->presetName();
  }
}

void QLinearBuildPresetComboBox::emitPresetChanged()
{
  if (this->presetName() != this->m_lastPreset) {
    emit this->presetChanged(this->presetName());
    this->m_lastPreset = this->presetName();
  }
}
