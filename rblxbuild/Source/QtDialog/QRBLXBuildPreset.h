/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include <QString>
#include <QVariant>

#include "cmRBLXBuildPresetsGraph.h"

class QRBLXBuildPreset
{
public:
  QString name;
  QString displayName;
  QString description;
  QString generator;
  QString architecture;
  bool setArchitecture;
  QString toolset;
  bool setToolset;
  bool enabled;
};

bool operator==(QRBLXBuildPreset const& lhs, QRBLXBuildPreset const& rhs);
bool operator!=(QRBLXBuildPreset const& lhs, QRBLXBuildPreset const& rhs);
bool operator<(QRBLXBuildPreset const& lhs, QRBLXBuildPreset const& rhs);
bool operator<=(QRBLXBuildPreset const& lhs, QRBLXBuildPreset const& rhs);
bool operator>(QRBLXBuildPreset const& lhs, QRBLXBuildPreset const& rhs);
bool operator>=(QRBLXBuildPreset const& lhs, QRBLXBuildPreset const& rhs);

Q_DECLARE_METATYPE(QRBLXBuildPreset)
