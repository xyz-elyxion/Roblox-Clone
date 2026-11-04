/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <cm/optional>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmRBLXBuildPresetsErrors.h"
#include "cmRBLXBuildPresetsGraph.h"
#include "cmRBLXBuildPresetsGraphInternal.h"
#include "cmJSONHelpers.h"
#include "cmJSONState.h"

namespace {
using PackagePreset = cmRBLXBuildPresetsGraph::PackagePreset;

auto const OutputHelper =
  cmJSONHelperBuilder::Object<PackagePreset>(
    JsonErrors::INVALID_NAMED_OBJECT_KEY, false)
    .Bind("debug"_s, &PackagePreset::DebugOutput,
          cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("verbose"_s, &PackagePreset::VerboseOutput,
          cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false);

auto const VariableHelper =
  cmJSONHelperBuilder::String(cmRBLXBuildPresetsErrors::INVALID_VARIABLE);

auto const VariablesHelper = cmJSONHelperBuilder::Map<std::string>(
  cmRBLXBuildPresetsErrors::INVALID_VARIABLE, VariableHelper);

auto const PackagePresetHelper =
  cmRBLXBuildPresetsGraphInternal::BindDependentPresetFields(
    cmRBLXBuildPresetsGraphInternal::BindPresetIdentityFields(
      cmJSONHelperBuilder::Object<PackagePreset>(
        cmRBLXBuildPresetsErrors::INVALID_PRESET_OBJECT, false)))
    .Bind("generators"_s, &PackagePreset::Generators,
          cmRBLXBuildPresetsGraphInternal::PresetVectorStringHelper, false)
    .Bind("configurations"_s, &PackagePreset::Configurations,
          cmRBLXBuildPresetsGraphInternal::PresetVectorStringHelper, false)
    .Bind("variables"_s, &PackagePreset::Variables, VariablesHelper, false)
    .Bind("configFile"_s, &PackagePreset::ConfigFile,
          cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("output"_s, OutputHelper, false)
    .Bind("packageName"_s, &PackagePreset::PackageName,
          cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("packageVersion"_s, &PackagePreset::PackageVersion,
          cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("packageDirectory"_s, &PackagePreset::PackageDirectory,
          cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("vendorName"_s, &PackagePreset::VendorName,
          cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false);
}

namespace cmRBLXBuildPresetsGraphInternal {
bool PackagePresetsHelper(std::vector<cmRBLXBuildPresetsGraph::PackagePreset>& out,
                          Json::Value const* value, cmJSONState* state)
{
  static auto const helper = cmJSONHelperBuilder::Vector<PackagePreset>(
    cmRBLXBuildPresetsErrors::INVALID_PRESETS, PackagePresetHelper);

  return helper(out, value, state);
}
}
