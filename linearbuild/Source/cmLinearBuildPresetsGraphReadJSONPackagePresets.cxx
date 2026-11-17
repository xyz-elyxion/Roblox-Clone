/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <cm/optional>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmLinearBuildPresetsErrors.h"
#include "cmLinearBuildPresetsGraph.h"
#include "cmLinearBuildPresetsGraphInternal.h"
#include "cmJSONHelpers.h"
#include "cmJSONState.h"

namespace {
using PackagePreset = cmLinearBuildPresetsGraph::PackagePreset;

auto const OutputHelper =
  cmJSONHelperBuilder::Object<PackagePreset>(
    JsonErrors::INVALID_NAMED_OBJECT_KEY, false)
    .Bind("debug"_s, &PackagePreset::DebugOutput,
          cmLinearBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("verbose"_s, &PackagePreset::VerboseOutput,
          cmLinearBuildPresetsGraphInternal::PresetOptionalBoolHelper, false);

auto const VariableHelper =
  cmJSONHelperBuilder::String(cmLinearBuildPresetsErrors::INVALID_VARIABLE);

auto const VariablesHelper = cmJSONHelperBuilder::Map<std::string>(
  cmLinearBuildPresetsErrors::INVALID_VARIABLE, VariableHelper);

auto const PackagePresetHelper =
  cmLinearBuildPresetsGraphInternal::BindDependentPresetFields(
    cmLinearBuildPresetsGraphInternal::BindPresetIdentityFields(
      cmJSONHelperBuilder::Object<PackagePreset>(
        cmLinearBuildPresetsErrors::INVALID_PRESET_OBJECT, false)))
    .Bind("generators"_s, &PackagePreset::Generators,
          cmLinearBuildPresetsGraphInternal::PresetVectorStringHelper, false)
    .Bind("configurations"_s, &PackagePreset::Configurations,
          cmLinearBuildPresetsGraphInternal::PresetVectorStringHelper, false)
    .Bind("variables"_s, &PackagePreset::Variables, VariablesHelper, false)
    .Bind("configFile"_s, &PackagePreset::ConfigFile,
          cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("output"_s, OutputHelper, false)
    .Bind("packageName"_s, &PackagePreset::PackageName,
          cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("packageVersion"_s, &PackagePreset::PackageVersion,
          cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("packageDirectory"_s, &PackagePreset::PackageDirectory,
          cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("vendorName"_s, &PackagePreset::VendorName,
          cmLinearBuildPresetsGraphInternal::PresetStringHelper, false);
}

namespace cmLinearBuildPresetsGraphInternal {
bool PackagePresetsHelper(std::vector<cmLinearBuildPresetsGraph::PackagePreset>& out,
                          Json::Value const* value, cmJSONState* state)
{
  static auto const helper = cmJSONHelperBuilder::Vector<PackagePreset>(
    cmLinearBuildPresetsErrors::INVALID_PRESETS, PackagePresetHelper);

  return helper(out, value, state);
}
}
