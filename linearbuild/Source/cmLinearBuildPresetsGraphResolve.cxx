/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>

#include "cmLinearBuildPresetsGraph.h"
#include "cmStringAlgorithms.h"
#include "cmUnreachable.h"

template <class T>
cmLinearBuildPresetsGraph::PresetResolveResult<T> cmLinearBuildPresetsGraph::ResolvePreset(
  std::string const& presetName,
  std::map<std::string, PresetPair<T>> const& presets) const
{
  using Result = PresetResolveResult<T>;
  Result result;

  auto presetPair = presets.find(presetName);
  if (presetPair == presets.end()) {
    result.StatusCode = Result::Status::NotFound;
    result.ErrorPresetName = presetName;
    std::vector<std::string> candidates;
    for (auto const& pair : presets) {
      if (!pair.second.Unexpanded.Hidden) {
        candidates.push_back(pair.first);
      }
    }
    result.Suggestion = cmFindClosestString(presetName, candidates);
    return result;
  }
  if (presetPair->second.Unexpanded.Hidden) {
    result.StatusCode = Result::Status::Hidden;
    result.ErrorPresetName = presetName;
    return result;
  }
  if (!presetPair->second.Expanded) {
    result.StatusCode = Result::Status::InvalidMacroExpansion;
    result.ErrorPresetName = presetName;
    return result;
  }
  if (!presetPair->second.Expanded->ConditionResult) {
    result.StatusCode = Result::Status::Disabled;
    result.ErrorPresetName = presetName;
    return result;
  }

  result.Preset = &*presetPair->second.Expanded;
  return result;
}

// Explicit template instantiations
template cmLinearBuildPresetsGraph::PresetResolveResult<
  cmLinearBuildPresetsGraph::ConfigurePreset>
cmLinearBuildPresetsGraph::ResolvePreset(
  std::string const&,
  std::map<std::string,
           PresetPair<cmLinearBuildPresetsGraph::ConfigurePreset>> const&) const;

template cmLinearBuildPresetsGraph::PresetResolveResult<
  cmLinearBuildPresetsGraph::BuildPreset>
cmLinearBuildPresetsGraph::ResolvePreset(
  std::string const&,
  std::map<std::string, PresetPair<cmLinearBuildPresetsGraph::BuildPreset>> const&)
  const;

template cmLinearBuildPresetsGraph::PresetResolveResult<
  cmLinearBuildPresetsGraph::TestPreset>
cmLinearBuildPresetsGraph::ResolvePreset(
  std::string const&,
  std::map<std::string, PresetPair<cmLinearBuildPresetsGraph::TestPreset>> const&)
  const;

template cmLinearBuildPresetsGraph::PresetResolveResult<
  cmLinearBuildPresetsGraph::PackagePreset>
cmLinearBuildPresetsGraph::ResolvePreset(
  std::string const&,
  std::map<std::string, PresetPair<cmLinearBuildPresetsGraph::PackagePreset>> const&)
  const;

template cmLinearBuildPresetsGraph::PresetResolveResult<
  cmLinearBuildPresetsGraph::WorkflowPreset>
cmLinearBuildPresetsGraph::ResolvePreset(
  std::string const&,
  std::map<std::string,
           PresetPair<cmLinearBuildPresetsGraph::WorkflowPreset>> const&) const;

template <class T>
cm::optional<std::string> cmLinearBuildPresetsGraph::FormatPresetError(
  PresetResolveStatus status, std::string const& errorPresetName,
  std::string const& suggestion, std::string const& directory)
{
  using Status = PresetResolveStatus;
  switch (status) {
    case Status::NotFound: {
      std::string error = cmStrCat("No such ", T::kind(), " preset in ",
                                   directory, ": \"", errorPresetName, '"');
      if (!suggestion.empty()) {
        error =
          cmStrCat(std::move(error), ". Did you mean: \"", suggestion, "\"?");
      }
      return error;
    }
    case Status::Hidden:
      return cmStrCat("Cannot use hidden ", T::kind(), " preset in ",
                      directory, ": \"", errorPresetName, '"');
    case Status::InvalidMacroExpansion:
      return cmStrCat("Could not evaluate ", T::kind(), " preset \"",
                      errorPresetName, "\": Invalid macro expansion");
    case Status::Disabled:
      return cmStrCat("Cannot use disabled ", T::kind(), " preset in ",
                      directory, ": \"", errorPresetName, '"');
    case Status::Success:
      return cm::nullopt;
  }
  CM_UNREACHABLE;
  return cm::nullopt;
}

// Explicit template instantiations for FormatPresetError
template cm::optional<std::string>
cmLinearBuildPresetsGraph::FormatPresetError<cmLinearBuildPresetsGraph::ConfigurePreset>(
  PresetResolveStatus, std::string const&, std::string const&,
  std::string const&);

template cm::optional<std::string> cmLinearBuildPresetsGraph::FormatPresetError<
  cmLinearBuildPresetsGraph::BuildPreset>(PresetResolveStatus, std::string const&,
                                    std::string const&, std::string const&);

template cm::optional<std::string> cmLinearBuildPresetsGraph::FormatPresetError<
  cmLinearBuildPresetsGraph::TestPreset>(PresetResolveStatus, std::string const&,
                                   std::string const&, std::string const&);

template cm::optional<std::string> cmLinearBuildPresetsGraph::FormatPresetError<
  cmLinearBuildPresetsGraph::PackagePreset>(PresetResolveStatus, std::string const&,
                                      std::string const&, std::string const&);

template cm::optional<std::string> cmLinearBuildPresetsGraph::FormatPresetError<
  cmLinearBuildPresetsGraph::WorkflowPreset>(PresetResolveStatus, std::string const&,
                                       std::string const&, std::string const&);
