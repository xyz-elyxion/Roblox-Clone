/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>

#include "cmRBLXBuildPresetsGraph.h"
#include "cmStringAlgorithms.h"
#include "cmUnreachable.h"

template <class T>
cmRBLXBuildPresetsGraph::PresetResolveResult<T> cmRBLXBuildPresetsGraph::ResolvePreset(
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
template cmRBLXBuildPresetsGraph::PresetResolveResult<
  cmRBLXBuildPresetsGraph::ConfigurePreset>
cmRBLXBuildPresetsGraph::ResolvePreset(
  std::string const&,
  std::map<std::string,
           PresetPair<cmRBLXBuildPresetsGraph::ConfigurePreset>> const&) const;

template cmRBLXBuildPresetsGraph::PresetResolveResult<
  cmRBLXBuildPresetsGraph::BuildPreset>
cmRBLXBuildPresetsGraph::ResolvePreset(
  std::string const&,
  std::map<std::string, PresetPair<cmRBLXBuildPresetsGraph::BuildPreset>> const&)
  const;

template cmRBLXBuildPresetsGraph::PresetResolveResult<
  cmRBLXBuildPresetsGraph::TestPreset>
cmRBLXBuildPresetsGraph::ResolvePreset(
  std::string const&,
  std::map<std::string, PresetPair<cmRBLXBuildPresetsGraph::TestPreset>> const&)
  const;

template cmRBLXBuildPresetsGraph::PresetResolveResult<
  cmRBLXBuildPresetsGraph::PackagePreset>
cmRBLXBuildPresetsGraph::ResolvePreset(
  std::string const&,
  std::map<std::string, PresetPair<cmRBLXBuildPresetsGraph::PackagePreset>> const&)
  const;

template cmRBLXBuildPresetsGraph::PresetResolveResult<
  cmRBLXBuildPresetsGraph::WorkflowPreset>
cmRBLXBuildPresetsGraph::ResolvePreset(
  std::string const&,
  std::map<std::string,
           PresetPair<cmRBLXBuildPresetsGraph::WorkflowPreset>> const&) const;

template <class T>
cm::optional<std::string> cmRBLXBuildPresetsGraph::FormatPresetError(
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
cmRBLXBuildPresetsGraph::FormatPresetError<cmRBLXBuildPresetsGraph::ConfigurePreset>(
  PresetResolveStatus, std::string const&, std::string const&,
  std::string const&);

template cm::optional<std::string> cmRBLXBuildPresetsGraph::FormatPresetError<
  cmRBLXBuildPresetsGraph::BuildPreset>(PresetResolveStatus, std::string const&,
                                    std::string const&, std::string const&);

template cm::optional<std::string> cmRBLXBuildPresetsGraph::FormatPresetError<
  cmRBLXBuildPresetsGraph::TestPreset>(PresetResolveStatus, std::string const&,
                                   std::string const&, std::string const&);

template cm::optional<std::string> cmRBLXBuildPresetsGraph::FormatPresetError<
  cmRBLXBuildPresetsGraph::PackagePreset>(PresetResolveStatus, std::string const&,
                                      std::string const&, std::string const&);

template cm::optional<std::string> cmRBLXBuildPresetsGraph::FormatPresetError<
  cmRBLXBuildPresetsGraph::WorkflowPreset>(PresetResolveStatus, std::string const&,
                                       std::string const&, std::string const&);
