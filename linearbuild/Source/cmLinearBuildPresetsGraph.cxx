/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#include "cmLinearBuildPresetsGraph.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <utility>

#include <cm/memory>
#include <cm/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmLinearBuildPresetsErrors.h"
#include "cmLinearBuildPresetsGraphInternal.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

#define CHECK_EXPAND(out, field, expanders, version)                          \
  do {                                                                        \
    switch (ExpandMacros(field, expanders, version)) {                        \
      case ExpandMacroResult::Error:                                          \
        return false;                                                         \
      case ExpandMacroResult::Ignore:                                         \
        out.reset();                                                          \
        return true;                                                          \
      case ExpandMacroResult::Defer:                                          \
        CM_FALLTHROUGH;                                                       \
      case ExpandMacroResult::Ok:                                             \
        break;                                                                \
    }                                                                         \
  } while (false)

namespace {
enum class CycleStatus
{
  Unvisited,
  InProgress,
  Verified,
};

using ConfigurePreset = cmLinearBuildPresetsGraph::ConfigurePreset;
using BuildPreset = cmLinearBuildPresetsGraph::BuildPreset;
using TestPreset = cmLinearBuildPresetsGraph::TestPreset;
using PackagePreset = cmLinearBuildPresetsGraph::PackagePreset;
using WorkflowPreset = cmLinearBuildPresetsGraph::WorkflowPreset;
template <typename T>
using PresetPair = cmLinearBuildPresetsGraph::PresetPair<T>;
using ExpandMacroResult = cmLinearBuildPresetsGraphInternal::ExpandMacroResult;
using MacroExpander = cmLinearBuildPresetsGraphInternal::MacroExpander;
using MacroExpanderVector = cmLinearBuildPresetsGraphInternal::MacroExpanderVector;
using BaseMacroExpander = cmLinearBuildPresetsGraphInternal::BaseMacroExpander;
template <typename T>
using PresetMacroExpander =
  cmLinearBuildPresetsGraphInternal::PresetMacroExpander<T>;
template <typename T>
using ImmediateMacroExpander =
  cmLinearBuildPresetsGraphInternal::ImmediateMacroExpander<T>;
using cmLinearBuildPresetsGraphInternal::ExpandMacros;

bool gSkipNewLine = true;

char const* WorkflowStepTypeToString(WorkflowPreset::WorkflowStep::Type type)
{
  switch (type) {
    case WorkflowPreset::WorkflowStep::Type::Configure:
      return "configure";
    case WorkflowPreset::WorkflowStep::Type::Build:
      return "build";
    case WorkflowPreset::WorkflowStep::Type::Test:
      return "test";
    case WorkflowPreset::WorkflowStep::Type::Package:
      return "package";
  }
  return "";
}

void InheritString(std::string& child, std::string const& parent)
{
  if (child.empty()) {
    child = parent;
  }
}

template <typename T>
void InheritOptionalValue(cm::optional<T>& child,
                          cm::optional<T> const& parent)
{
  if (!child) {
    child = parent;
  }
}

template <typename T>
void InheritVector(std::vector<T>& child, std::vector<T> const& parent)
{
  if (child.empty()) {
    child = parent;
  }
}

template <typename K, typename V>
void InheritMap(std::map<K, V>& child, std::map<K, V> const& parent)
{
  for (auto const& item : parent) {
    child.insert(item);
  }
}

/**
 * Check preset inheritance for cycles (using a DAG check algorithm) while
 * also bubbling up fields through the inheritance hierarchy, then verify
 * that each preset has the required fields, either directly or through
 * inheritance.
 */
template <typename T>
bool VisitPreset(
  T& preset,
  std::map<std::string, cmLinearBuildPresetsGraph::PresetPair<T>>& presets,
  std::map<std::string, CycleStatus> cycleStatus, cmLinearBuildPresetsGraph& graph)
{
  switch (cycleStatus[preset.Name]) {
    case CycleStatus::InProgress:
      cmLinearBuildPresetsErrors::CYCLIC_PRESET_INHERITANCE(
        preset.Name, preset.kind(), &graph.parseState);
      return false;
    case CycleStatus::Verified:
      return true;
    default:
      break;
  }

  cycleStatus[preset.Name] = CycleStatus::InProgress;

  if (preset.Environment.count("") != 0) {
    cmLinearBuildPresetsErrors::INVALID_PRESET_NAMED(
      preset.Name, preset.kind(), &graph.parseState,
      "Empty environment variable names are not allowed");
    return false;
  }

  bool result = preset.VisitPresetBeforeInherit();
  if (!result) {
    cmLinearBuildPresetsErrors::INVALID_PRESET_NAMED(
      preset.Name, preset.kind(), &graph.parseState, preset.ErrorDetail);
    return false;
  }

  for (auto const& i : preset.Inherits) {
    auto parent = presets.find(i);
    if (parent == presets.end()) {
      cmLinearBuildPresetsErrors::INVALID_PRESET_NAMED(
        preset.Name, preset.kind(), &graph.parseState,
        cmStrCat("Could not find inherited preset \"", i, '"'));
      return false;
    }

    auto& parentPreset = parent->second.Unexpanded;
    if (!preset.OriginFile->ReachableFiles.count(parentPreset.OriginFile)) {
      cmLinearBuildPresetsErrors::INHERITED_PRESET_UNREACHABLE_FROM_FILE(
        preset.Name, i, preset.kind(), &graph.parseState);
      return false;
    }

    if (!VisitPreset(parentPreset, presets, cycleStatus, graph)) {
      return false;
    }

    result = preset.VisitPresetInherit(parentPreset);
    if (!result) {
      cmLinearBuildPresetsErrors::INVALID_PRESET_NAMED(
        preset.Name, preset.kind(), &graph.parseState, preset.ErrorDetail);
      return false;
    }

    InheritMap(preset.Environment, parentPreset.Environment);

    if (!preset.ConditionEvaluator) {
      preset.ConditionEvaluator = parentPreset.ConditionEvaluator;
    }
  }

  if (preset.ConditionEvaluator && preset.ConditionEvaluator->IsNull()) {
    preset.ConditionEvaluator.reset();
  }

  result = preset.VisitPresetAfterInherit(graph.GetVersion(preset),
                                          &graph.parseState);
  if (!result) {
    cmLinearBuildPresetsErrors::INVALID_PRESET_NAMED(
      preset.Name, preset.kind(), &graph.parseState, preset.ErrorDetail);
    return false;
  }

  cycleStatus[preset.Name] = CycleStatus::Verified;
  return true;
}

template <typename T>
bool ComputePresetInheritance(
  std::map<std::string, cmLinearBuildPresetsGraph::PresetPair<T>>& presets,
  cmLinearBuildPresetsGraph& graph)
{
  std::map<std::string, CycleStatus> cycleStatus;
  for (auto const& it : presets) {
    cycleStatus[it.first] = CycleStatus::Unvisited;
  }

  for (auto& it : presets) {
    auto& preset = it.second.Unexpanded;
    if (!VisitPreset<T>(preset, presets, cycleStatus, graph)) {
      return false;
    }
  }

  return true;
}

constexpr char const* ValidPrefixes[] = {
  "",
  "env",
  "penv",
  "vendor",
};

bool PrefixesValidMacroNamespace(std::string const& str)
{
  return std::any_of(
    std::begin(ValidPrefixes), std::end(ValidPrefixes),
    [&str](char const* prefix) -> bool { return cmHasPrefix(prefix, str); });
}

bool IsValidMacroNamespace(std::string const& str)
{
  return std::any_of(
    std::begin(ValidPrefixes), std::end(ValidPrefixes),
    [&str](char const* prefix) -> bool { return str == prefix; });
}

ExpandMacroResult VisitEnv(std::string& value, CycleStatus& status,
                           MacroExpanderVector const& macroExpanders,
                           int version);
template <typename T>
class EnvironmentMacroExpander : public MacroExpander
{
  std::map<std::string, CycleStatus>& EnvCycles;
  cm::optional<T>& Out;
  MacroExpanderVector& MacroExpanders;

public:
  EnvironmentMacroExpander(MacroExpanderVector& macroExpanders,
                           cm::optional<T>& out,
                           std::map<std::string, CycleStatus>& envCycles)
    : EnvCycles(envCycles)
    , Out(out)
    , MacroExpanders(macroExpanders)
  {
  }
  ExpandMacroResult operator()(std::string const& macroNamespace,
                               std::string const& macroName,
                               std::string& macroOut,
                               int version) const override
  {
    if (macroNamespace == "env" && !macroName.empty() && Out) {
      auto v = Out->Environment.find(macroName);
      if (v != Out->Environment.end() && v->second) {
        auto e =
          VisitEnv(*v->second, EnvCycles[macroName], MacroExpanders, version);
        if (e != ExpandMacroResult::Ok) {
          return e;
        }
        macroOut += *v->second;
        return ExpandMacroResult::Ok;
      }
    }

    if (macroNamespace == "env" || macroNamespace == "penv") {
      if (macroName.empty()) {
        return ExpandMacroResult::Error;
      }
      if (cm::optional<std::string> value =
            cmSystemTools::GetEnvVar(macroName)) {
        macroOut += *value;
      }
      return ExpandMacroResult::Ok;
    }

    return ExpandMacroResult::Ignore;
  }
};

bool CheckExpandMacros(ConfigurePreset const& preset,
                       cm::optional<ConfigurePreset>& out,
                       MacroExpanderVector const& macroExpanders,
                       cmLinearBuildPresetsGraph const* graph = nullptr)
{
  int version = graph ? graph->GetVersion(preset) : preset.OriginFile->Version;

  std::string binaryDir = preset.BinaryDir;
  CHECK_EXPAND(out, binaryDir, macroExpanders, version);

  if (!binaryDir.empty()) {
    if (graph) {
      if (!cmSystemTools::FileIsFullPath(binaryDir)) {
        binaryDir = cmStrCat(graph->SourceDir, '/', std::move(binaryDir));
      }
      out->BinaryDir = cmSystemTools::CollapseFullPath(binaryDir);
      cmSystemTools::ConvertToUnixSlashes(out->BinaryDir);
    } else {
      // Defer path normalization until the graph is merged, but store the
      // intermediate result.
      out->BinaryDir = binaryDir;
    }
  }

  if (!preset.InstallDir.empty()) {
    std::string installDir = preset.InstallDir;
    CHECK_EXPAND(out, installDir, macroExpanders, version);

    if (graph) {
      if (!cmSystemTools::FileIsFullPath(installDir)) {
        installDir = cmStrCat(graph->SourceDir, '/', std::move(installDir));
      }
      out->InstallDir = cmSystemTools::CollapseFullPath(installDir);
      cmSystemTools::ConvertToUnixSlashes(out->InstallDir);
    } else {
      // Defer path normalization until the graph is merged, but store the
      // intermediate result.
      out->InstallDir = installDir;
    }
  }

  if (!preset.ToolchainFile.empty()) {
    std::string toolchain = preset.ToolchainFile;
    CHECK_EXPAND(out, toolchain, macroExpanders, version);
    out->ToolchainFile = toolchain;
  }

  if (!preset.GraphVizFile.empty()) {
    std::string graphVizFile = preset.GraphVizFile;
    CHECK_EXPAND(out, graphVizFile, macroExpanders, version);
    out->GraphVizFile = graphVizFile;
  }

  for (auto& variable : out->CacheVariables) {
    if (variable.second) {
      CHECK_EXPAND(out, variable.second->Value, macroExpanders, version);
    }
  }

  return true;
}

bool CheckExpandMacros(BuildPreset const& preset,
                       cm::optional<BuildPreset>& out,
                       MacroExpanderVector const& macroExpanders,
                       cmLinearBuildPresetsGraph const* graph = nullptr)
{
  int version = graph ? graph->GetVersion(preset) : preset.OriginFile->Version;

  for (auto& target : out->Targets) {
    CHECK_EXPAND(out, target, macroExpanders, version);
  }

  for (auto& nativeToolOption : out->NativeToolOptions) {
    CHECK_EXPAND(out, nativeToolOption, macroExpanders, version);
  }

  return true;
}

bool CheckExpandMacros(TestPreset const& preset, cm::optional<TestPreset>& out,
                       MacroExpanderVector const& macroExpanders,
                       cmLinearBuildPresetsGraph const* graph = nullptr)
{
  int version = graph ? graph->GetVersion(preset) : preset.OriginFile->Version;

  for (auto& overwrite : out->OverwriteConfigurationFile) {
    CHECK_EXPAND(out, overwrite, macroExpanders, version);
  }

  if (out->Output) {
    CHECK_EXPAND(out, out->Output->OutputLogFile, macroExpanders, version);
    CHECK_EXPAND(out, out->Output->OutputJUnitFile, macroExpanders, version);
  }

  if (out->Filter) {
    if (out->Filter->Include) {
      CHECK_EXPAND(out, out->Filter->Include->Name, macroExpanders, version);
      CHECK_EXPAND(out, out->Filter->Include->Label, macroExpanders, version);

      if (out->Filter->Include->Index) {
        CHECK_EXPAND(out, out->Filter->Include->Index->IndexFile,
                     macroExpanders, version);
      }
    }

    if (out->Filter->Exclude) {
      CHECK_EXPAND(out, out->Filter->Exclude->Name, macroExpanders, version);
      CHECK_EXPAND(out, out->Filter->Exclude->Label, macroExpanders, version);

      if (out->Filter->Exclude->Fixtures) {
        CHECK_EXPAND(out, out->Filter->Exclude->Fixtures->Any, macroExpanders,
                     version);
        CHECK_EXPAND(out, out->Filter->Exclude->Fixtures->Setup,
                     macroExpanders, version);
        CHECK_EXPAND(out, out->Filter->Exclude->Fixtures->Cleanup,
                     macroExpanders, version);
      }
    }
  }

  if (out->Execution) {
    CHECK_EXPAND(out, out->Execution->ResourceSpecFile, macroExpanders,
                 version);
  }

  return true;
}

bool CheckExpandMacros(PackagePreset const& preset,
                       cm::optional<PackagePreset>& out,
                       MacroExpanderVector const& macroExpanders,
                       cmLinearBuildPresetsGraph const* graph = nullptr)
{
  int version = graph ? graph->GetVersion(preset) : preset.OriginFile->Version;

  for (auto& variable : out->Variables) {
    CHECK_EXPAND(out, variable.second, macroExpanders, version);
  }

  CHECK_EXPAND(out, out->ConfigFile, macroExpanders, version);
  CHECK_EXPAND(out, out->PackageName, macroExpanders, version);
  CHECK_EXPAND(out, out->PackageVersion, macroExpanders, version);
  CHECK_EXPAND(out, out->PackageDirectory, macroExpanders, version);
  CHECK_EXPAND(out, out->VendorName, macroExpanders, version);

  return true;
}

bool CheckExpandMacros(WorkflowPreset const& /*preset*/,
                       cm::optional<WorkflowPreset>& /*out*/,
                       MacroExpanderVector const& /*macroExpanders*/,
                       cmLinearBuildPresetsGraph const* /*graph*/ = nullptr)
{
  return true;
}

template <typename T>
bool ExpandMacros(cmLinearBuildPresetsGraph* graph, T const& preset,
                  cm::optional<T>& out)
{
  out.emplace(preset);

  std::map<std::string, CycleStatus> envCycles;
  for (auto const& v : out->Environment) {
    envCycles[v.first] = CycleStatus::Unvisited;
  }

  MacroExpanderVector macroExpanders{};

  macroExpanders.push_back(cm::make_unique<BaseMacroExpander>(*graph));
  macroExpanders.push_back(
    cm::make_unique<PresetMacroExpander<T>>(*graph, preset));
  macroExpanders.push_back(cm::make_unique<EnvironmentMacroExpander<T>>(
    macroExpanders, out, envCycles));

  for (auto& v : out->Environment) {
    if (v.second) {
      switch (VisitEnv(*v.second, envCycles[v.first], macroExpanders,
                       graph->GetVersion(preset))) {
        case ExpandMacroResult::Error:
          cmLinearBuildPresetsErrors::INVALID_PRESET_NAMED(
            preset.Name, preset.kind(), &graph->parseState,
            "Invalid macro expansion");
          return false;
        case ExpandMacroResult::Ignore:
          out.reset();
          return true;
        case ExpandMacroResult::Defer:
          CM_FALLTHROUGH;
        case ExpandMacroResult::Ok:
          break;
      }
    }
  }

  if (preset.ConditionEvaluator) {
    cm::optional<bool> result;
    if (!preset.ConditionEvaluator->Evaluate(
          macroExpanders, graph->GetVersion(preset), result)) {
      cmLinearBuildPresetsErrors::INVALID_PRESET_NAMED(
        preset.Name, preset.kind(), &graph->parseState, "Invalid condition");
      return false;
    }
    if (!result) {
      out.reset();
      return true;
    }
    out->ConditionResult = *result;
  }

  return CheckExpandMacros(preset, out, macroExpanders, graph);
}

ExpandMacroResult VisitEnv(std::string& value, CycleStatus& status,
                           MacroExpanderVector const& macroExpanders,
                           int version)
{
  if (status == CycleStatus::Verified) {
    return ExpandMacroResult::Ok;
  }
  if (status == CycleStatus::InProgress) {
    return ExpandMacroResult::Error;
  }

  status = CycleStatus::InProgress;
  auto e = ExpandMacros(value, macroExpanders, version);
  if (e != ExpandMacroResult::Ok) {
    return e;
  }
  status = CycleStatus::Verified;
  return ExpandMacroResult::Ok;
}

struct PresetListEntry
{
  cmLinearBuildPresetsGraph::Preset const* Preset;
  cm::optional<std::string> UnavailableReason;
};

void PrintPresets(std::vector<PresetListEntry> const& presets)
{
  if (presets.empty()) {
    return;
  }

  auto presetWithLongestName = std::max_element(
    presets.begin(), presets.end(),
    [](PresetListEntry const& a, PresetListEntry const& b) {
      return a.Preset->Name.length() < b.Preset->Name.length();
    });
  auto longestLength = presetWithLongestName->Preset->Name.length();

  for (auto entryIt = presets.begin(); entryIt != presets.end(); ++entryIt) {
    auto const& entry = *entryIt;
    auto const* preset = entry.Preset;
    std::string name = cmStrCat("  \"", preset->Name, '"');
    if (!preset->DisplayName.empty()) {
      int const width = static_cast<int>(longestLength + name.length() -
                                         preset->Name.length());
      std::cout << std::left << std::setw(width) << name << " - "
                << preset->DisplayName << '\n';
    } else {
      std::cout << name << '\n';
    }
    if (entry.UnavailableReason) {
      std::cout << "    Unavailable: " << *entry.UnavailableReason << '\n';
      if (entryIt + 1 != presets.end()) {
        std::cout << '\n';
      }
    }
  }
}

template <typename PresetType>
cm::optional<std::string> GetUnavailableReason(
  cmLinearBuildPresetsGraph::PresetPair<PresetType> const& preset)
{
  if (preset.Unexpanded.Hidden) {
    return "hidden";
  }
  if (!preset.Expanded) {
    // Expansion errors abort preset loading.  A missing expanded preset means
    // expansion was ignored, such as for an unsupported vendor macro.
    return "unsupported macro expansion";
  }
  if (!preset.Expanded->ConditionResult) {
    auto const& condition = preset.Expanded->ConditionEvaluator;
    return cmStrCat("condition evaluated to false",
                    !condition || condition->ConditionJson.empty()
                      ? ""
                      : cmStrCat(": ", condition->ConditionJson));
  }
  return cm::nullopt;
}

template <typename PresetType>
cm::optional<std::string> GetUnavailableReason(
  std::string const& name,
  std::map<std::string, PresetPair<PresetType>> const& presets)
{
  auto const preset = presets.find(name);
  if (preset == presets.end()) {
    return cmStrCat(PresetType::kind(), " preset \"", name,
                    "\" does not exist");
  }
  auto reason = GetUnavailableReason(preset->second);
  if (!reason) {
    return cm::nullopt;
  }
  return cmStrCat(PresetType::kind(), " preset \"", name, "\": ", *reason);
}

cm::optional<std::string> GetUnavailableReason(
  std::string const& name,
  std::map<std::string, PresetPair<ConfigurePreset>> const& presets,
  cmLinearBuildPresetsGraph::ConfigurePresetUsabilityCheck const&
    additionalUsabilityCheck)
{
  auto const preset = presets.find(name);
  if (preset == presets.end()) {
    return cmStrCat("configure preset \"", name, "\" does not exist");
  }
  auto reason = GetUnavailableReason(preset->second);
  if (!reason && additionalUsabilityCheck) {
    assert(preset->second.Expanded);
    reason = additionalUsabilityCheck(*preset->second.Expanded);
  }
  if (!reason) {
    return cm::nullopt;
  }
  return cmStrCat("configure preset \"", name, "\": ", *reason);
}

template <typename PresetType, typename UsabilityCheck>
void PrintPresetList(
  cmLinearBuildPresetsGraph const* const graph,
  std::map<std::string, cmLinearBuildPresetsGraph::PresetPair<PresetType>>
    cmLinearBuildPresetsGraph::*data,
  std::vector<std::string> cmLinearBuildPresetsGraph::*index,
  cmLinearBuildPresetsGraph::PresetListMode mode,
  UsabilityCheck const& additionalUsabilityCheck)
{
  std::vector<PresetListEntry> presets;
  presets.reserve((graph->*index).size());
  for (auto const& name : graph->*index) {
    auto const& preset = (graph->*data).at(name);
    if (preset.Unexpanded.Hidden) {
      continue;
    }
    auto reason = GetUnavailableReason(preset);
    if (!reason) {
      assert(preset.Expanded);
      reason = additionalUsabilityCheck(*preset.Expanded);
    }
    if (mode == cmLinearBuildPresetsGraph::PresetListMode::Defined || !reason) {
      presets.push_back({ &preset.Unexpanded, std::move(reason) });
    }
  }

  if (!presets.empty()) {
    std::cout << (gSkipNewLine ? "" : "\n")
              << (mode == cmLinearBuildPresetsGraph::PresetListMode::Defined
                    ? "Defined "
                    : "Available ")
              << PresetType::kind() << " presets:\n\n";
    gSkipNewLine = false;
    PrintPresets(presets);
  }
}
}

template <typename T>
bool cmLinearBuildPresetsGraphInternal::ExpandImmediateMacros(T& preset)
{
  MacroExpanderVector macroExpanders{};
  macroExpanders.push_back(cm::make_unique<ImmediateMacroExpander<T>>(preset));
  cm::optional<T> out = preset;
  bool result = CheckExpandMacros(preset, out, macroExpanders);
  if (out.has_value()) {
    preset = out.value();
  };
  return result;
}

template bool cmLinearBuildPresetsGraphInternal::ExpandImmediateMacros<
  ConfigurePreset>(ConfigurePreset&);
template bool cmLinearBuildPresetsGraphInternal::ExpandImmediateMacros<BuildPreset>(
  BuildPreset&);
template bool cmLinearBuildPresetsGraphInternal::ExpandImmediateMacros<TestPreset>(
  TestPreset&);
template bool cmLinearBuildPresetsGraphInternal::ExpandImmediateMacros<
  PackagePreset>(PackagePreset&);
template bool cmLinearBuildPresetsGraphInternal::ExpandImmediateMacros<
  WorkflowPreset>(WorkflowPreset&);

ExpandMacroResult cmLinearBuildPresetsGraphInternal::ExpandMacros(
  std::string& out, MacroExpanderVector const& macroExpanders, int version)
{
  std::string result;
  std::string macroNamespace;
  std::string macroName;

  enum class State
  {
    Default,
    MacroNamespace,
    MacroName,
  } state = State::Default;

  for (auto c : out) {
    switch (state) {
      case State::Default:
        if (c == '$') {
          state = State::MacroNamespace;
        } else {
          result += c;
        }
        break;

      case State::MacroNamespace:
        if (c == '{') {
          if (IsValidMacroNamespace(macroNamespace)) {
            state = State::MacroName;
          } else {
            result += '$';
            result += macroNamespace;
            result += '{';
            macroNamespace.clear();
            state = State::Default;
          }
        } else {
          macroNamespace += c;
          if (!PrefixesValidMacroNamespace(macroNamespace)) {
            result += '$';
            result += macroNamespace;
            macroNamespace.clear();
            state = State::Default;
          }
        }
        break;

      case State::MacroName:
        if (c == '}') {
          auto e = ExpandMacro(result, macroNamespace, macroName,
                               macroExpanders, version);
          if (e != ExpandMacroResult::Ok) {
            return e;
          }
          macroNamespace.clear();
          macroName.clear();
          state = State::Default;
        } else {
          macroName += c;
        }
        break;
    }
  }

  switch (state) {
    case State::Default:
      break;
    case State::MacroNamespace:
      result += '$';
      result += macroNamespace;
      break;
    case State::MacroName:
      return ExpandMacroResult::Error;
  }

  out = std::move(result);
  return ExpandMacroResult::Ok;
}

ExpandMacroResult cmLinearBuildPresetsGraphInternal::ExpandMacro(
  std::string& out, std::string const& macroNamespace,
  std::string const& macroName, MacroExpanderVector const& macroExpanders,
  int version)
{
  for (auto const& macroExpander : macroExpanders) {
    auto result = (*macroExpander)(macroNamespace, macroName, out, version);
    if (result != ExpandMacroResult::Ignore) {
      return result;
    }
  }

  if (macroNamespace == "vendor") {
    return ExpandMacroResult::Ignore;
  }

  return ExpandMacroResult::Error;
}

namespace {
template <typename T>
bool SetupWorkflowConfigurePreset(T const& preset,
                                  ConfigurePreset const*& configurePreset,
                                  char const*, std::string const&,
                                  cmJSONState* state)
{
  if (preset.ConfigurePreset != configurePreset->Name) {
    cmLinearBuildPresetsErrors::WORKFLOW_STEP_CONFIGURE_PRESET_MISMATCH(
      preset.kind(), preset.Name, preset.ConfigurePreset,
      configurePreset->Name, state);
    return false;
  }
  return true;
}

template <>
bool SetupWorkflowConfigurePreset<ConfigurePreset>(
  ConfigurePreset const& preset, ConfigurePreset const*& configurePreset,
  char const*, std::string const&, cmJSONState*)
{
  configurePreset = &preset;
  return true;
}

template <typename T>
bool TryReachPresetFromWorkflow(
  WorkflowPreset const& origin,
  std::map<std::string, PresetPair<T>> const& presets, std::string const& name,
  char const* workflowStepType, ConfigurePreset const*& configurePreset,
  cmJSONState* state)
{
  auto it = presets.find(name);
  if (it == presets.end()) {
    cmLinearBuildPresetsErrors::INVALID_WORKFLOW_STEPS(workflowStepType, name,
                                                 state);
    return false;
  }
  if (!origin.OriginFile->ReachableFiles.count(
        it->second.Unexpanded.OriginFile)) {
    cmLinearBuildPresetsErrors::WORKFLOW_STEP_UNREACHABLE_FROM_FILE(workflowStepType,
                                                              name, state);
    return false;
  }
  return SetupWorkflowConfigurePreset<T>(
    it->second.Unexpanded, configurePreset, workflowStepType, name, state);
}
}

ExpandMacroResult BaseMacroExpander::operator()(
  std::string const& macroNamespace, std::string const& macroName,
  std::string& macroOut, int version) const
{
  if (macroNamespace.empty()) {
    if (macroName == "sourceDir") {
      macroOut += Graph.SourceDir;
      return ExpandMacroResult::Ok;
    }
    if (macroName == "sourceParentDir") {
      macroOut += cmSystemTools::GetParentDirectory(Graph.SourceDir);
      return ExpandMacroResult::Ok;
    }
    if (macroName == "sourceDirName") {
      macroOut += cmSystemTools::GetFilenameName(Graph.SourceDir);
      return ExpandMacroResult::Ok;
    }
    if (macroName == "dollar") {
      macroOut += '$';
      return ExpandMacroResult::Ok;
    }
    if (macroName == "hostSystemName") {
      if (version < 3) {
        return ExpandMacroResult::Error;
      }
      macroOut += cmSystemTools::GetSystemName();
      return ExpandMacroResult::Ok;
    }
    // Enable fileDir macro expansion for non-preset expanders
    if (macroName == "fileDir" && File) {
      if (version < 4) {
        return ExpandMacroResult::Error;
      }
      macroOut += cmSystemTools::GetParentDirectory(File.value());
      return ExpandMacroResult::Ok;
    }
    if (macroName == "pathListSep") {
      if (version < 5) {
        return ExpandMacroResult::Error;
      }
      macroOut += cmSystemTools::GetSystemPathlistSeparator();
      return ExpandMacroResult::Ok;
    }
  }

  return ExpandMacroResult::Ignore;
}

bool cmLinearBuildPresetsGraphInternal::EqualsCondition::Evaluate(
  MacroExpanderVector const& expanders, int version,
  cm::optional<bool>& out) const
{
  std::string lhs = this->Lhs;
  CHECK_EXPAND(out, lhs, expanders, version);

  std::string rhs = this->Rhs;
  CHECK_EXPAND(out, rhs, expanders, version);

  out = (lhs == rhs);
  return true;
}

bool cmLinearBuildPresetsGraphInternal::InListCondition::Evaluate(
  MacroExpanderVector const& expanders, int version,
  cm::optional<bool>& out) const
{
  std::string str = this->String;
  CHECK_EXPAND(out, str, expanders, version);

  for (auto item : this->List) {
    CHECK_EXPAND(out, item, expanders, version);
    if (str == item) {
      out = true;
      return true;
    }
  }

  out = false;
  return true;
}

bool cmLinearBuildPresetsGraphInternal::MatchesCondition::Evaluate(
  MacroExpanderVector const& expanders, int version,
  cm::optional<bool>& out) const
{
  std::string str = this->String;
  CHECK_EXPAND(out, str, expanders, version);
  std::string regexStr = this->Regex;
  CHECK_EXPAND(out, regexStr, expanders, version);

  cmsys::RegularExpression regex;
  if (!regex.compile(regexStr)) {
    return false;
  }

  out = regex.find(str);
  return true;
}

bool cmLinearBuildPresetsGraphInternal::AnyAllOfCondition::Evaluate(
  MacroExpanderVector const& expanders, int version,
  cm::optional<bool>& out) const
{
  for (auto const& condition : this->Conditions) {
    cm::optional<bool> result;
    if (!condition->Evaluate(expanders, version, result)) {
      out.reset();
      return false;
    }

    if (!result) {
      out.reset();
      return true;
    }

    if (result == this->StopValue) {
      out = result;
      return true;
    }
  }

  out = !this->StopValue;
  return true;
}

bool cmLinearBuildPresetsGraphInternal::NotCondition::Evaluate(
  MacroExpanderVector const& expanders, int version,
  cm::optional<bool>& out) const
{
  out.reset();
  if (!this->SubCondition->Evaluate(expanders, version, out)) {
    out.reset();
    return false;
  }
  if (out) {
    *out = !*out;
  }
  return true;
}

bool cmLinearBuildPresetsGraph::ConfigurePreset::VisitPresetInherit(
  cmLinearBuildPresetsGraph::Preset const& parentPreset)
{
  auto& preset = *this;
  ConfigurePreset const& parent =
    static_cast<ConfigurePreset const&>(parentPreset);
  InheritString(preset.Generator, parent.Generator);
  InheritString(preset.Architecture, parent.Architecture);
  InheritString(preset.Toolset, parent.Toolset);
  if (!preset.ArchitectureStrategy) {
    preset.ArchitectureStrategy = parent.ArchitectureStrategy;
  }
  if (!preset.ToolsetStrategy) {
    preset.ToolsetStrategy = parent.ToolsetStrategy;
  }
  InheritString(preset.BinaryDir, parent.BinaryDir);
  InheritString(preset.InstallDir, parent.InstallDir);
  InheritString(preset.ToolchainFile, parent.ToolchainFile);
  InheritString(preset.GraphVizFile, parent.GraphVizFile);
  InheritMap(preset.Warnings, parent.Warnings);
  InheritMap(preset.Errors, parent.Errors);
  InheritOptionalValue(preset.WarnSystemVars, parent.WarnSystemVars);
  InheritMap(preset.CacheVariables, parent.CacheVariables);
  InheritOptionalValue(preset.DebugOutput, parent.DebugOutput);
  InheritOptionalValue(preset.DebugTryCompile, parent.DebugTryCompile);
  InheritOptionalValue(preset.DebugFind, parent.DebugFind);
  InheritOptionalValue(preset.TraceMode, parent.TraceMode);
  InheritOptionalValue(preset.TraceFormat, parent.TraceFormat);
  InheritVector(preset.TraceSource, parent.TraceSource);
  InheritString(preset.TraceRedirect, parent.TraceRedirect);

  return true;
}

bool cmLinearBuildPresetsGraph::ConfigurePreset::VisitPresetBeforeInherit()
{
  auto& preset = *this;
  if (preset.Environment.count("") != 0) {
    this->ErrorDetail =
      "Empty environment variable names are not allowed in configure presets";
    return false;
  }

  return true;
}

bool cmLinearBuildPresetsGraph::ConfigurePreset::VisitPresetAfterInherit(
  int version, cmJSONState* state)
{
  auto& preset = *this;
  if (!preset.Hidden) {
    if (version < 3) {
      if (preset.Generator.empty()) {
        cmLinearBuildPresetsErrors::PRESET_MISSING_FIELD(preset.Name, "generator",
                                                   state);
        return false;
      }
      if (preset.BinaryDir.empty()) {
        cmLinearBuildPresetsErrors::PRESET_MISSING_FIELD(preset.Name, "binaryDir",
                                                   state);
        return false;
      }
    }

    for (auto const& w : preset.Warnings) {
      auto const ei = preset.Errors.find(w.first);
      if (ei != preset.Errors.end()) {
        if (w.second == false && ei->second == true) {
          cm::string_view const diagnostic =
            version < 12 && w.first == cmDiagnostics::CMD_AUTHOR
            ? cm::string_view{ "dev" }
            : cmLinearBuildPresetsGraphInternal::GetDiagnosticJSONName(w.first);
          this->ErrorDetail = cmStrCat("\"errors.", diagnostic,
                                       "\" is enabled while \"warnings.",
                                       diagnostic, "\" is disabled");
          return false;
        }
      }
    }

    if (preset.CacheVariables.count("") != 0) {
      this->ErrorDetail = "Empty cache variable names are not allowed";
      return false;
    }
  }

  return true;
}

bool cmLinearBuildPresetsGraph::BuildPreset::VisitPresetInherit(
  cmLinearBuildPresetsGraph::Preset const& parentPreset)
{
  auto& preset = *this;
  BuildPreset const& parent = static_cast<BuildPreset const&>(parentPreset);

  InheritString(preset.ConfigurePreset, parent.ConfigurePreset);
  InheritOptionalValue(preset.InheritConfigureEnvironment,
                       parent.InheritConfigureEnvironment);
  InheritOptionalValue(preset.Jobs, parent.Jobs);
  InheritVector(preset.Targets, parent.Targets);
  InheritString(preset.Configuration, parent.Configuration);
  InheritOptionalValue(preset.CleanFirst, parent.CleanFirst);
  InheritOptionalValue(preset.Verbose, parent.Verbose);
  InheritVector(preset.NativeToolOptions, parent.NativeToolOptions);
  if (!preset.ResolvePackageReferences) {
    preset.ResolvePackageReferences = parent.ResolvePackageReferences;
  }

  return true;
}

bool cmLinearBuildPresetsGraph::BuildPreset::VisitPresetAfterInherit(
  int /* version */, cmJSONState* /*stat*/)
{
  if (!this->Hidden && this->ConfigurePreset.empty()) {
    this->ErrorDetail = "Build presets must either be hidden or have an "
                        "associated configure preset";
    return false;
  }
  return true;
}

bool cmLinearBuildPresetsGraph::TestPreset::VisitPresetInherit(
  cmLinearBuildPresetsGraph::Preset const& parentPreset)
{
  auto& preset = *this;
  TestPreset const& parent = static_cast<TestPreset const&>(parentPreset);

  InheritString(preset.ConfigurePreset, parent.ConfigurePreset);
  InheritOptionalValue(preset.InheritConfigureEnvironment,
                       parent.InheritConfigureEnvironment);
  InheritString(preset.Configuration, parent.Configuration);
  InheritVector(preset.OverwriteConfigurationFile,
                parent.OverwriteConfigurationFile);

  if (parent.Output) {
    if (preset.Output) {
      auto& output = preset.Output.value();
      auto const& parentOutput = parent.Output.value();
      InheritOptionalValue(output.ShortProgress, parentOutput.ShortProgress);
      InheritOptionalValue(output.Verbosity, parentOutput.Verbosity);
      InheritOptionalValue(output.Debug, parentOutput.Debug);
      InheritOptionalValue(output.OutputOnFailure,
                           parentOutput.OutputOnFailure);
      InheritOptionalValue(output.Quiet, parentOutput.Quiet);
      InheritString(output.OutputLogFile, parentOutput.OutputLogFile);
      InheritString(output.OutputJUnitFile, parentOutput.OutputJUnitFile);
      InheritOptionalValue(output.LabelSummary, parentOutput.LabelSummary);
      InheritOptionalValue(output.SubprojectSummary,
                           parentOutput.SubprojectSummary);
      InheritOptionalValue(output.MaxPassedTestOutputSize,
                           parentOutput.MaxPassedTestOutputSize);
      InheritOptionalValue(output.MaxFailedTestOutputSize,
                           parentOutput.MaxFailedTestOutputSize);
      InheritOptionalValue(output.TestOutputTruncation,
                           parentOutput.TestOutputTruncation);
      InheritOptionalValue(output.MaxTestNameWidth,
                           parentOutput.MaxTestNameWidth);
    } else {
      preset.Output = parent.Output;
    }
  }

  if (parent.Filter) {
    if (parent.Filter->Include) {
      if (preset.Filter && preset.Filter->Include) {
        auto& include = *preset.Filter->Include;
        auto const& parentInclude = *parent.Filter->Include;
        InheritString(include.Name, parentInclude.Name);
        InheritString(include.Label, parentInclude.Label);
        InheritOptionalValue(include.Index, parentInclude.Index);
      } else {
        if (!preset.Filter) {
          preset.Filter.emplace();
        }
        preset.Filter->Include = parent.Filter->Include;
      }
    }

    if (parent.Filter->Exclude) {
      if (preset.Filter && preset.Filter->Exclude) {
        auto& exclude = *preset.Filter->Exclude;
        auto const& parentExclude = *parent.Filter->Exclude;
        InheritString(exclude.Name, parentExclude.Name);
        InheritString(exclude.Label, parentExclude.Label);
        InheritOptionalValue(exclude.Fixtures, parentExclude.Fixtures);
      } else {
        if (!preset.Filter) {
          preset.Filter.emplace();
        }
        preset.Filter->Exclude = parent.Filter->Exclude;
      }
    }
  }

  if (parent.Execution) {
    if (preset.Execution) {
      auto& execution = *preset.Execution;
      auto const& parentExecution = *parent.Execution;
      InheritOptionalValue(execution.StopOnFailure,
                           parentExecution.StopOnFailure);
      InheritOptionalValue(execution.EnableFailover,
                           parentExecution.EnableFailover);
      InheritOptionalValue(execution.Jobs, parentExecution.Jobs);
      InheritString(execution.ResourceSpecFile,
                    parentExecution.ResourceSpecFile);
      InheritOptionalValue(execution.TestLoad, parentExecution.TestLoad);
      InheritOptionalValue(execution.ShowOnly, parentExecution.ShowOnly);
      InheritOptionalValue(execution.Repeat, parentExecution.Repeat);
      InheritOptionalValue(execution.InteractiveDebugging,
                           parentExecution.InteractiveDebugging);
      InheritOptionalValue(execution.ScheduleRandom,
                           parentExecution.ScheduleRandom);
      InheritOptionalValue(execution.Timeout, parentExecution.Timeout);
      InheritOptionalValue(execution.NoTestsAction,
                           parentExecution.NoTestsAction);
      InheritVector(execution.TestPassthroughArguments,
                    parentExecution.TestPassthroughArguments);
    } else {
      preset.Execution = parent.Execution;
    }
  }

  return true;
}

bool cmLinearBuildPresetsGraph::TestPreset::VisitPresetAfterInherit(
  int /* version */, cmJSONState* /*state*/)
{
  if (!this->Hidden && this->ConfigurePreset.empty()) {
    this->ErrorDetail = "Test presets must either be hidden or have an "
                        "associated configure preset";
    return false;
  }
  return true;
}

bool cmLinearBuildPresetsGraph::PackagePreset::VisitPresetInherit(
  cmLinearBuildPresetsGraph::Preset const& parentPreset)
{
  auto& preset = *this;
  PackagePreset const& parent =
    static_cast<PackagePreset const&>(parentPreset);

  InheritString(preset.ConfigurePreset, parent.ConfigurePreset);
  InheritOptionalValue(preset.InheritConfigureEnvironment,
                       parent.InheritConfigureEnvironment);
  InheritVector(preset.Generators, parent.Generators);
  InheritVector(preset.Configurations, parent.Configurations);
  InheritMap(preset.Variables, parent.Variables);
  InheritOptionalValue(preset.DebugOutput, parent.DebugOutput);
  InheritOptionalValue(preset.VerboseOutput, parent.VerboseOutput);
  InheritString(preset.PackageName, parent.PackageName);
  InheritString(preset.PackageVersion, parent.PackageVersion);
  InheritString(preset.PackageDirectory, parent.PackageDirectory);
  InheritString(preset.VendorName, parent.VendorName);

  return true;
}

bool cmLinearBuildPresetsGraph::PackagePreset::VisitPresetAfterInherit(
  int /* version */, cmJSONState* /*state*/)
{
  if (!this->Hidden && this->ConfigurePreset.empty()) {
    this->ErrorDetail = "Package presets must either be hidden or have an "
                        "associated configure preset";
    return false;
  }
  return true;
}

bool cmLinearBuildPresetsGraph::WorkflowPreset::VisitPresetInherit(
  cmLinearBuildPresetsGraph::Preset const& /*parentPreset*/)
{
  return true;
}

bool cmLinearBuildPresetsGraph::WorkflowPreset::VisitPresetAfterInherit(
  int /* version */, cmJSONState* /*state*/)
{
  return true;
}

std::string cmLinearBuildPresetsGraph::GetFilename(std::string const& sourceDir)
{
  return cmStrCat(sourceDir, "/LinearBuildPresets.json");
}

std::string cmLinearBuildPresetsGraph::GetUserFilename(std::string const& sourceDir)
{
  return cmStrCat(sourceDir, "/LinearBuildUserPresets.json");
}

bool cmLinearBuildPresetsGraph::ReadProjectPresets(std::string const& sourceDir,
                                             std::string const& presetsFile,
                                             ReadOption readFilesOption)
{
  this->SourceDir = cmSystemTools::CollapseFullPath(sourceDir);
  this->ClearPresets();

  if (!this->ReadProjectPresetsInternal(presetsFile, readFilesOption)) {
    this->ClearPresets();
    return false;
  }

  return true;
}

std::string cmLinearBuildPresetsGraph::GetGeneratorForPreset(
  std::string const& presetName) const
{
  auto configurePresetName = presetName;

  auto buildPresetIterator = this->BuildPresets.find(presetName);
  if (buildPresetIterator != this->BuildPresets.end()) {
    configurePresetName =
      buildPresetIterator->second.Unexpanded.ConfigurePreset;
  } else {
    auto testPresetIterator = this->TestPresets.find(presetName);
    if (testPresetIterator != this->TestPresets.end()) {
      configurePresetName =
        testPresetIterator->second.Unexpanded.ConfigurePreset;
    }
  }

  auto configurePresetIterator =
    this->ConfigurePresets.find(configurePresetName);
  if (configurePresetIterator != this->ConfigurePresets.end()) {
    return configurePresetIterator->second.Unexpanded.Generator;
  }

  // This should only happen if the preset is hidden
  // or (for build or test presets) if ConfigurePreset is invalid.
  return {};
}

template <typename T>
bool cmLinearBuildPresetsGraph::ResolveDependentPresets(
  std::map<std::string, PresetPair<T>>& presets)
{
  for (auto& it : presets) {
    if (!it.second.Unexpanded.Hidden) {
      auto const configurePreset =
        this->ConfigurePresets.find(it.second.Unexpanded.ConfigurePreset);
      if (configurePreset == this->ConfigurePresets.end()) {
        cmLinearBuildPresetsErrors::CONFIGURE_PRESET_NOT_FOUND(
          it.first, T::kind(), it.second.Unexpanded.ConfigurePreset,
          &this->parseState);
        return false;
      }
      if (!it.second.Unexpanded.OriginFile->ReachableFiles.count(
            configurePreset->second.Unexpanded.OriginFile)) {
        cmLinearBuildPresetsErrors::CONFIGURE_PRESET_UNREACHABLE_FROM_FILE(
          it.first, T::kind(), it.second.Unexpanded.ConfigurePreset,
          &this->parseState);
        return false;
      }
      if (it.second.Unexpanded.InheritConfigureEnvironment.value_or(true)) {
        it.second.Unexpanded.Environment.insert(
          configurePreset->second.Unexpanded.Environment.begin(),
          configurePreset->second.Unexpanded.Environment.end());
      }
    }
    if (!ExpandMacros(this, it.second.Unexpanded, it.second.Expanded)) {
      cmLinearBuildPresetsErrors::INVALID_MACRO_EXPANSION(it.first,
                                                    &this->parseState);
      return false;
    }
  }
  return true;
}

template bool
cmLinearBuildPresetsGraph::ResolveDependentPresets<cmLinearBuildPresetsGraph::BuildPreset>(
  std::map<std::string, PresetPair<cmLinearBuildPresetsGraph::BuildPreset>>&);

template bool
cmLinearBuildPresetsGraph::ResolveDependentPresets<cmLinearBuildPresetsGraph::TestPreset>(
  std::map<std::string, PresetPair<cmLinearBuildPresetsGraph::TestPreset>>&);

template bool cmLinearBuildPresetsGraph::ResolveDependentPresets<
  cmLinearBuildPresetsGraph::PackagePreset>(
  std::map<std::string, PresetPair<cmLinearBuildPresetsGraph::PackagePreset>>&);

bool cmLinearBuildPresetsGraph::ReadProjectPresetsInternal(
  std::string const& presetsFile, ReadOption readFilesOption)
{
  bool haveOneFile = false;

  File* file;
  std::string filename;
  std::vector<File*> inProgressFiles;
  if (!presetsFile.empty()) {
    filename = presetsFile;
    if (cmSystemTools::FileExists(filename)) {
      if (!this->ReadJSONFile(filename, RootType::Any, ReadReason::Root,
                              inProgressFiles, file, this->errors)) {
        return false;
      }
      haveOneFile = true;
    }
  } else {
    filename = GetUserFilename(this->SourceDir);
    if (cmSystemTools::FileExists(filename)) {
      if (!this->ReadJSONFile(filename, RootType::User, ReadReason::Root,
                              inProgressFiles, file, this->errors)) {
        return false;
      }
      haveOneFile = true;
    } else {
      filename = GetFilename(this->SourceDir);
      if (cmSystemTools::FileExists(filename)) {
        if (!this->ReadJSONFile(filename, RootType::Project, ReadReason::Root,
                                inProgressFiles, file, this->errors)) {
          return false;
        }
        haveOneFile = true;
      }
    }
  }
  assert(inProgressFiles.empty());

  if (!haveOneFile) {
    if (readFilesOption == ReadOption::AllowNoFiles) {
      return true;
    }
    cmLinearBuildPresetsErrors::FILE_NOT_FOUND(filename, &this->parseState);
    return false;
  }

  bool result = ComputePresetInheritance(this->ConfigurePresets, *this) &&
    ComputePresetInheritance(this->BuildPresets, *this) &&
    ComputePresetInheritance(this->TestPresets, *this) &&
    ComputePresetInheritance(this->PackagePresets, *this) &&
    ComputePresetInheritance(this->WorkflowPresets, *this);
  if (!result) {
    return false;
  }

  for (auto& it : this->ConfigurePresets) {
    if (!ExpandMacros(this, it.second.Unexpanded, it.second.Expanded)) {
      cmLinearBuildPresetsErrors::INVALID_MACRO_EXPANSION(it.first,
                                                    &this->parseState);
      return false;
    }
  }

  if (!ResolveDependentPresets(this->BuildPresets) ||
      !ResolveDependentPresets(this->TestPresets) ||
      !ResolveDependentPresets(this->PackagePresets)) {
    return false;
  }

  for (auto& it : this->WorkflowPresets) {
    using Type = WorkflowPreset::WorkflowStep::Type;

    ConfigurePreset const* configurePreset = nullptr;
    for (auto const& step : it.second.Unexpanded.Steps) {
      char const* const stepType = WorkflowStepTypeToString(step.PresetType);
      if (!configurePreset && step.PresetType != Type::Configure) {
        cmLinearBuildPresetsErrors::FIRST_WORKFLOW_STEP_NOT_CONFIGURE(
          stepType, step.PresetName, &this->parseState);
        return false;
      }
      if (configurePreset && step.PresetType == Type::Configure) {
        cmLinearBuildPresetsErrors::CONFIGURE_WORKFLOW_STEP_NOT_FIRST(
          stepType, step.PresetName, &this->parseState);
        return false;
      }

      switch (step.PresetType) {
        case Type::Configure:
          result = TryReachPresetFromWorkflow(
            it.second.Unexpanded, this->ConfigurePresets, step.PresetName,
            stepType, configurePreset, &this->parseState);
          break;
        case Type::Build:
          result = TryReachPresetFromWorkflow(
            it.second.Unexpanded, this->BuildPresets, step.PresetName,
            stepType, configurePreset, &this->parseState);
          break;
        case Type::Test:
          result = TryReachPresetFromWorkflow(
            it.second.Unexpanded, this->TestPresets, step.PresetName, stepType,
            configurePreset, &this->parseState);
          break;
        case Type::Package:
          result = TryReachPresetFromWorkflow(
            it.second.Unexpanded, this->PackagePresets, step.PresetName,
            stepType, configurePreset, &this->parseState);
          break;
      }
      if (!result) {
        return false;
      }
    }

    if (!configurePreset) {
      cmLinearBuildPresetsErrors::NO_WORKFLOW_STEPS(it.first, &this->parseState);
      return false;
    }

    if (!ExpandMacros(this, it.second.Unexpanded, it.second.Expanded)) {
      cmLinearBuildPresetsErrors::INVALID_MACRO_EXPANSION(it.first,
                                                    &this->parseState);
      return false;
    }
  }

  return true;
}

void cmLinearBuildPresetsGraph::ClearPresets()
{
  this->ConfigurePresets.clear();
  this->BuildPresets.clear();
  this->TestPresets.clear();
  this->PackagePresets.clear();
  this->WorkflowPresets.clear();

  this->ConfigurePresetOrder.clear();
  this->BuildPresetOrder.clear();
  this->TestPresetOrder.clear();
  this->PackagePresetOrder.clear();
  this->WorkflowPresetOrder.clear();

  this->Files.clear();
}

void cmLinearBuildPresetsGraph::PrintConfigurePresetList(
  PresetListMode mode,
  ConfigurePresetUsabilityCheck const& usabilityCheck) const
{
  auto check = [&usabilityCheck](
                 ConfigurePreset const& preset) -> cm::optional<std::string> {
    if (!usabilityCheck) {
      return cm::nullopt;
    }
    return usabilityCheck(preset);
  };
  PrintPresetList<ConfigurePreset>(
    this, &cmLinearBuildPresetsGraph::ConfigurePresets,
    &cmLinearBuildPresetsGraph::ConfigurePresetOrder, mode, check);
}

void cmLinearBuildPresetsGraph::PrintBuildPresetList(
  PresetListMode mode,
  ConfigurePresetUsabilityCheck const& configureUsabilityCheck) const
{
  auto usabilityCheck = [this,
                         &configureUsabilityCheck](BuildPreset const& preset) {
    return GetUnavailableReason(preset.ConfigurePreset, this->ConfigurePresets,
                                configureUsabilityCheck);
  };
  PrintPresetList<BuildPreset>(this, &cmLinearBuildPresetsGraph::BuildPresets,
                               &cmLinearBuildPresetsGraph::BuildPresetOrder, mode,
                               usabilityCheck);
}

void cmLinearBuildPresetsGraph::PrintTestPresetList(
  PresetListMode mode,
  ConfigurePresetUsabilityCheck const& configureUsabilityCheck) const
{
  auto usabilityCheck = [this,
                         &configureUsabilityCheck](TestPreset const& preset) {
    return GetUnavailableReason(preset.ConfigurePreset, this->ConfigurePresets,
                                configureUsabilityCheck);
  };
  PrintPresetList<TestPreset>(this, &cmLinearBuildPresetsGraph::TestPresets,
                              &cmLinearBuildPresetsGraph::TestPresetOrder, mode,
                              usabilityCheck);
}

void cmLinearBuildPresetsGraph::PrintPackagePresetList(
  PresetListMode mode,
  ConfigurePresetUsabilityCheck const& configureUsabilityCheck) const
{
  auto usabilityCheck = [this, &configureUsabilityCheck](
                          PackagePreset const& preset) {
    return GetUnavailableReason(preset.ConfigurePreset, this->ConfigurePresets,
                                configureUsabilityCheck);
  };
  PrintPresetList<PackagePreset>(this, &cmLinearBuildPresetsGraph::PackagePresets,
                                 &cmLinearBuildPresetsGraph::PackagePresetOrder,
                                 mode, usabilityCheck);
}

void cmLinearBuildPresetsGraph::PrintPackagePresetList(
  std::function<bool(PackagePreset const&)> const& packageGeneratorsPresent,
  PresetListMode mode,
  ConfigurePresetUsabilityCheck const& configureUsabilityCheck) const
{
  auto usabilityCheck = [this, &packageGeneratorsPresent,
                         &configureUsabilityCheck](
                          PackagePreset const& preset) {
    auto configureReason = GetUnavailableReason(
      preset.ConfigurePreset, this->ConfigurePresets, configureUsabilityCheck);
    if (configureReason) {
      return configureReason;
    }
    cm::optional<std::string> generatorReason;
    if (!packageGeneratorsPresent(preset)) {
      generatorReason = "one or more package generators are not available";
    }
    return generatorReason;
  };
  PrintPresetList<PackagePreset>(this, &cmLinearBuildPresetsGraph::PackagePresets,
                                 &cmLinearBuildPresetsGraph::PackagePresetOrder,
                                 mode, usabilityCheck);
}

void cmLinearBuildPresetsGraph::PrintWorkflowPresetList(
  PresetListMode mode,
  ConfigurePresetUsabilityCheck const& configureUsabilityCheck) const
{
  auto usabilityCheck = [this, &configureUsabilityCheck](
                          WorkflowPreset const& preset) {
    return this->GetWorkflowUnavailableReason(preset, configureUsabilityCheck);
  };
  PrintPresetList<WorkflowPreset>(this, &cmLinearBuildPresetsGraph::WorkflowPresets,
                                  &cmLinearBuildPresetsGraph::WorkflowPresetOrder,
                                  mode, usabilityCheck);
}

cm::optional<std::string> cmLinearBuildPresetsGraph::GetWorkflowUnavailableReason(
  WorkflowPreset const& preset,
  ConfigurePresetUsabilityCheck const& configureUsabilityCheck) const
{
  using Type = WorkflowPreset::WorkflowStep::Type;

  std::size_t stepNumber = 0;
  for (auto const& step : preset.Steps) {
    ++stepNumber;
    cm::optional<std::string> reason;
    switch (step.PresetType) {
      case Type::Configure:
        reason = GetUnavailableReason(step.PresetName, this->ConfigurePresets,
                                      configureUsabilityCheck);
        break;
      case Type::Build:
        reason = GetUnavailableReason(step.PresetName, this->BuildPresets);
        break;
      case Type::Test:
        reason = GetUnavailableReason(step.PresetName, this->TestPresets);
        break;
      case Type::Package:
        reason = GetUnavailableReason(step.PresetName, this->PackagePresets);
        break;
    }
    if (reason) {
      return cmStrCat("step ", stepNumber, ": ", *reason);
    }
  }
  return cm::nullopt;
}
