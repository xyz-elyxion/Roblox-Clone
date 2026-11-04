/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include <functional>
#include <string>
#include <vector>

#include <cm/optional>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmBuildOptions.h"
#include "cmRBLXBuildPresetsErrors.h"
#include "cmRBLXBuildPresetsGraph.h"
#include "cmRBLXBuildPresetsGraphInternal.h"
#include "cmJSONHelpers.h"

class cmJSONState;
namespace {
using BuildPreset = cmRBLXBuildPresetsGraph::BuildPreset;
using JSONHelperBuilder = cmJSONHelperBuilder;

bool PackageResolveModeHelper(cm::optional<PackageResolveMode>& out,
                              Json::Value const* value, cmJSONState* state)
{
  if (!value) {
    out = cm::nullopt;
    return true;
  }

  if (!value->isString()) {
    cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
    return false;
  }

  if (value->asString() == "on") {
    out = PackageResolveMode::Force;
  } else if (value->asString() == "off") {
    out = PackageResolveMode::Disable;
  } else if (value->asString() == "only") {
    out = PackageResolveMode::OnlyResolve;
  } else {
    cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
    return false;
  }

  return true;
}

std::function<bool(BuildPreset&, Json::Value const*, cmJSONState*)> const
  ResolvePackageReferencesHelper = [](BuildPreset& out,
                                      Json::Value const* value,
                                      cmJSONState* state) -> bool {
  return PackageResolveModeHelper(out.ResolvePackageReferences, value, state);
};

auto const BuildPresetHelper =
  cmRBLXBuildPresetsGraphInternal::BindDependentPresetFields(
    cmRBLXBuildPresetsGraphInternal::BindPresetIdentityFields(
      JSONHelperBuilder::Object<BuildPreset>(
        cmRBLXBuildPresetsErrors::INVALID_PRESET_OBJECT, false)))
    .Bind("jobs"_s, &BuildPreset::Jobs,
          cmRBLXBuildPresetsGraphInternal::PresetOptionalUIntHelper, false)
    .Bind("targets"_s, &BuildPreset::Targets,
          cmRBLXBuildPresetsGraphInternal::PresetVectorOneOrMoreStringHelper,
          false)
    .Bind("configuration"_s, &BuildPreset::Configuration,
          cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("cleanFirst"_s, &BuildPreset::CleanFirst,
          cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("verbose"_s, &BuildPreset::Verbose,
          cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("nativeToolOptions"_s, &BuildPreset::NativeToolOptions,
          cmRBLXBuildPresetsGraphInternal::PresetVectorStringHelper, false)
    .Bind("resolvePackageReferences"_s, ResolvePackageReferencesHelper, false);
}

namespace cmRBLXBuildPresetsGraphInternal {
bool BuildPresetsHelper(std::vector<BuildPreset>& out,
                        Json::Value const* value, cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Vector<BuildPreset>(
    cmRBLXBuildPresetsErrors::INVALID_PRESETS, BuildPresetHelper);

  return helper(out, value, state);
}
}
