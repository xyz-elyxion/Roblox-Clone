/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include <cmext/string_view>

#include "cmRBLXBuildPresetsErrors.h"
#include "cmRBLXBuildPresetsGraph.h"
#include "cmRBLXBuildPresetsGraphInternal.h"
#include "cmJSONHelpers.h"

class cmJSONState;

namespace Json {
class Value;
}

namespace {
using WorkflowPreset = cmRBLXBuildPresetsGraph::WorkflowPreset;

bool WorkflowStepTypeHelper(WorkflowPreset::WorkflowStep::Type& out,
                            Json::Value const* value, cmJSONState* state)
{
  if (!value) {
    cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
    return false;
  }

  std::string workflowType;
  if (!cmRBLXBuildPresetsGraphInternal::PresetStringHelper(workflowType, value,
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

  cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const WorkflowStepHelper =
  cmJSONHelperBuilder::Object<WorkflowPreset::WorkflowStep>(
    JsonErrors::INVALID_OBJECT, false)
    .Bind("type"_s, &WorkflowPreset::WorkflowStep::PresetType,
          WorkflowStepTypeHelper)
    .Bind("name"_s, &WorkflowPreset::WorkflowStep::PresetName,
          cmRBLXBuildPresetsGraphInternal::PresetStringHelper);

auto const WorkflowStepsHelper =
  cmJSONHelperBuilder::Vector<WorkflowPreset::WorkflowStep>(
    cmRBLXBuildPresetsErrors::INVALID_PRESET, WorkflowStepHelper);

auto const WorkflowPresetHelper =
  cmJSONHelperBuilder::Object<WorkflowPreset>(
    cmRBLXBuildPresetsErrors::INVALID_PRESET_OBJECT, false)
    .Bind("name"_s, &WorkflowPreset::Name,
          cmRBLXBuildPresetsGraphInternal::PresetNameHelper)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          cmRBLXBuildPresetsGraphInternal::VendorHelper(
                            cmRBLXBuildPresetsErrors::INVALID_PRESET),
                          false)
    .Bind("displayName"_s, &WorkflowPreset::DisplayName,
          cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("description"_s, &WorkflowPreset::Description,
          cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("steps"_s, &WorkflowPreset::Steps, WorkflowStepsHelper);
}

namespace cmRBLXBuildPresetsGraphInternal {
bool WorkflowPresetsHelper(
  std::vector<cmRBLXBuildPresetsGraph::WorkflowPreset>& out,
  Json::Value const* value, cmJSONState* state)
{
  static auto const helper = cmJSONHelperBuilder::Vector<WorkflowPreset>(
    cmRBLXBuildPresetsErrors::INVALID_PRESETS, WorkflowPresetHelper);

  return helper(out, value, state);
}
}
