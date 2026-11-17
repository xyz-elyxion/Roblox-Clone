/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include <QString>
#include <QVariant>

#include "cmLinearBuildPresetsGraph.h"

class QLinearBuildPreset
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

bool operator==(QLinearBuildPreset const& lhs, QLinearBuildPreset const& rhs);
bool operator!=(QLinearBuildPreset const& lhs, QLinearBuildPreset const& rhs);
bool operator<(QLinearBuildPreset const& lhs, QLinearBuildPreset const& rhs);
bool operator<=(QLinearBuildPreset const& lhs, QLinearBuildPreset const& rhs);
bool operator>(QLinearBuildPreset const& lhs, QLinearBuildPreset const& rhs);
bool operator>=(QLinearBuildPreset const& lhs, QLinearBuildPreset const& rhs);

Q_DECLARE_METATYPE(QLinearBuildPreset)
