/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include <cmext/string_view>

#include "cmLinearBuildPresetsErrors.h"
#include "cmLinearBuildPresetsGraph.h"
#include "cmLinearBuildPresetsGraphInternal.h"
#include "cmJSONHelpers.h"

class cmJSONState;

namespace Json {
class Value;
}

namespace {
using WorkflowPreset = cmLinearBuildPresetsGraph::WorkflowPreset;

bool WorkflowStepTypeHelper(WorkflowPreset::WorkflowStep::Type& out,
                            Json::Value const* value, cmJSONState* state)
{
  if (!value) {
    cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
    return false;
  }

  std::string workflowType;
  if (!cmLinearBuildPresetsGraphInternal::PresetStringHelper(workflowType, value,
                                                       state)) {
    return false;
  }

  if (workflowType == "configure") {
    out = WorkflowPreset::WorkflowStep::Type::Configure;
    return true;
  }

  if (workflowType == "build") {
    out = WorkflowPreset::WorkflowStep::Type::Build;
    return true;
  }

  if (workflowType == "test") {
    out = WorkflowPreset::WorkflowStep::Type::Test;
    return true;
  }

  if (workflowType == "package") {
    out = WorkflowPreset::WorkflowStep::Type::Package;
    return true;
  }

  cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const WorkflowStepHelper =
  cmJSONHelperBuilder::Object<WorkflowPreset::WorkflowStep>(
    JsonErrors::INVALID_OBJECT, false)
    .Bind("type"_s, &WorkflowPreset::WorkflowStep::PresetType,
          WorkflowStepTypeHelper)
    .Bind("name"_s, &WorkflowPreset::WorkflowStep::PresetName,
          cmLinearBuildPresetsGraphInternal::PresetStringHelper);

auto const WorkflowStepsHelper =
  cmJSONHelperBuilder::Vector<WorkflowPreset::WorkflowStep>(
    cmLinearBuildPresetsErrors::INVALID_PRESET, WorkflowStepHelper);

auto const WorkflowPresetHelper =
  cmJSONHelperBuilder::Object<WorkflowPreset>(
    cmLinearBuildPresetsErrors::INVALID_PRESET_OBJECT, false)
    .Bind("name"_s, &WorkflowPreset::Name,
          cmLinearBuildPresetsGraphInternal::PresetNameHelper)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          cmLinearBuildPresetsGraphInternal::VendorHelper(
                            cmLinearBuildPresetsErrors::INVALID_PRESET),
                          false)
    .Bind("displayName"_s, &WorkflowPreset::DisplayName,
          cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("description"_s, &WorkflowPreset::Description,
          cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("steps"_s, &WorkflowPreset::Steps, WorkflowStepsHelper);
}

namespace cmLinearBuildPresetsGraphInternal {
bool WorkflowPresetsHelper(
  std::vector<cmLinearBuildPresetsGraph::WorkflowPreset>& out,
  Json::Value const* value, cmJSONState* state)
{
  static auto const helper = cmJSONHelperBuilder::Vector<WorkflowPreset>(
    cmLinearBuildPresetsErrors::INVALID_PRESETS, WorkflowPresetHelper);

  return helper(out, value, state);
}
}
