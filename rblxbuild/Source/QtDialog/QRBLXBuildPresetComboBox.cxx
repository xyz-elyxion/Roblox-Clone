/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include "QRBLXBuildPresetComboBox.h"

#include "QRBLXBuildPresetItemModel.h"

QRBLXBuildPresetComboBox::QRBLXBuildPresetComboBox(QWidget* parent)
  : QComboBox(parent)
{
  this->m_model = new QRBLXBuildPresetItemModel(this);
  this->setModel(this->m_model);

  QObject::connect(this->m_model, &QRBLXBuildPresetItemModel::modelAboutToBeReset,
                   this, [this]() { this->m_resetting = true; });
  QObject::connect(this->m_model, &QRBLXBuildPresetItemModel::modelReset, this,
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

QVector<QRBLXBuildPreset> const& QRBLXBuildPresetComboBox::presets() const
{
  return this->m_model->presets();
}

QString QRBLXBuildPresetComboBox::presetName() const
{
  auto preset = this->currentData();
  if (preset.canConvert<QRBLXBuildPreset>()) {
    return preset.value<QRBLXBuildPreset>().name;
  }
  return QString{};
}

void QRBLXBuildPresetComboBox::setPresets(QVector<QRBLXBuildPreset> const& presets)
{
  this->m_model->setPresets(presets);
}

void QRBLXBuildPresetComboBox::setPresetName(QString const& name)
{
  this->setCurrentIndex(this->m_model->presetNameToRow(name));
  if (this->signalsBlocked()) {
    this->m_lastPreset = this->presetName();
  }
}

void QRBLXBuildPresetComboBox::emitPresetChanged()
{
  if (this->presetName() != this->m_lastPreset) {
    emit this->presetChanged(this->presetName());
    this->m_lastPreset = this->presetName();
  }
}
