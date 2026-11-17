/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cmext/string_view>

#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>

#include "cmLinearBuildPresetsErrors.h"
#include "cmLinearBuildPresetsGraph.h"
#include "cmLinearBuildPresetsGraphInternal.h"
#include "cmJSONHelpers.h"
#include "cmJSONState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmVersion.h"

namespace {
using CacheVariable = cmLinearBuildPresetsGraph::CacheVariable;
using ConfigurePreset = cmLinearBuildPresetsGraph::ConfigurePreset;
using BuildPreset = cmLinearBuildPresetsGraph::BuildPreset;
using TestPreset = cmLinearBuildPresetsGraph::TestPreset;
using PackagePreset = cmLinearBuildPresetsGraph::PackagePreset;
using WorkflowPreset = cmLinearBuildPresetsGraph::WorkflowPreset;
template <typename T>
using PresetPair = cmLinearBuildPresetsGraph::PresetPair<T>;
using ArchToolsetStrategy = cmLinearBuildPresetsGraph::ArchToolsetStrategy;
using JSONHelperBuilder = cmJSONHelperBuilder;
using ExpandMacroResult = cmLinearBuildPresetsGraphInternal::ExpandMacroResult;
using MacroExpander = cmLinearBuildPresetsGraphInternal::MacroExpander;
using MacroExpanderVector = cmLinearBuildPresetsGraphInternal::MacroExpanderVector;
using cmLinearBuildPresetsGraphInternal::BaseMacroExpander;
using cmLinearBuildPresetsGraphInternal::ExpandMacros;
using cmLinearBuildPresetsGraphInternal::ExpandImmediateMacros;

constexpr int MIN_VERSION = 1;
constexpr int MAX_VERSION = 13;

struct LinearBuildVersion
{
  unsigned int Major = 0;
  unsigned int Minor = 0;
  unsigned int Patch = 0;
};

struct RootPresets
{
  LinearBuildVersion LinearBuildMinimumRequired;
  std::vector<ConfigurePreset> ConfigurePresets;
  std::vector<BuildPreset> BuildPresets;
  std::vector<TestPreset> TestPresets;
  std::vector<PackagePreset> PackagePresets;
  std::vector<WorkflowPreset> WorkflowPresets;
  std::vector<std::string> Include;
};

std::unique_ptr<cmLinearBuildPresetsGraphInternal::NotCondition> InvertCondition(
  std::unique_ptr<cmLinearBuildPresetsGraph::Condition> condition)
{
  auto retval = cm::make_unique<cmLinearBuildPresetsGraphInternal::NotCondition>();
  retval->SubCondition = std::move(condition);
  return retval;
}

auto const ConditionStringHelper = JSONHelperBuilder::String();

auto const ConditionBoolHelper = JSONHelperBuilder::Bool();

auto const ConditionStringListHelper = JSONHelperBuilder::Vector<std::string>(
  cmLinearBuildPresetsErrors::INVALID_CONDITION, ConditionStringHelper);

auto const ConstConditionHelper =
  JSONHelperBuilder::Object<cmLinearBuildPresetsGraphInternal::ConstCondition>(
    cmLinearBuildPresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("value"_s, &cmLinearBuildPresetsGraphInternal::ConstCondition::Value,
          ConditionBoolHelper, true);

auto const EqualsConditionHelper =
  JSONHelperBuilder::Object<cmLinearBuildPresetsGraphInternal::EqualsCondition>(
    cmLinearBuildPresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("lhs"_s, &cmLinearBuildPresetsGraphInternal::EqualsCondition::Lhs,
          ConditionStringHelper, true)
    .Bind("rhs"_s, &cmLinearBuildPresetsGraphInternal::EqualsCondition::Rhs,
          ConditionStringHelper, true);

auto const InListConditionHelper =
  JSONHelperBuilder::Object<cmLinearBuildPresetsGraphInternal::InListCondition>(
    cmLinearBuildPresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("string"_s, &cmLinearBuildPresetsGraphInternal::InListCondition::String,
          ConditionStringHelper, true)
    .Bind("list"_s, &cmLinearBuildPresetsGraphInternal::InListCondition::List,
          ConditionStringListHelper, true);

auto const MatchesConditionHelper =
  JSONHelperBuilder::Object<cmLinearBuildPresetsGraphInternal::MatchesCondition>(
    cmLinearBuildPresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("string"_s, &cmLinearBuildPresetsGraphInternal::MatchesCondition::String,
          ConditionStringHelper, true)
    .Bind("regex"_s, &cmLinearBuildPresetsGraphInternal::MatchesCondition::Regex,
          ConditionStringHelper, true);

bool SubConditionHelper(std::unique_ptr<cmLinearBuildPresetsGraph::Condition>& out,
                        Json::Value const* value, cmJSONState* state);

auto const ListConditionVectorHelper =
  JSONHelperBuilder::Vector<std::unique_ptr<cmLinearBuildPresetsGraph::Condition>>(
    cmLinearBuildPresetsErrors::INVALID_CONDITION, SubConditionHelper);
auto const AnyAllOfConditionHelper =
  JSONHelperBuilder::Object<cmLinearBuildPresetsGraphInternal::AnyAllOfCondition>(
    cmLinearBuildPresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("conditions"_s,
          &cmLinearBuildPresetsGraphInternal::AnyAllOfCondition::Conditions,
          ListConditionVectorHelper);

auto const NotConditionHelper =
  JSONHelperBuilder::Object<cmLinearBuildPresetsGraphInternal::NotCondition>(
    cmLinearBuildPresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("condition"_s,
          &cmLinearBuildPresetsGraphInternal::NotCondition::SubCondition,
          SubConditionHelper);

bool ConditionHelper(std::unique_ptr<cmLinearBuildPresetsGraph::Condition>& out,
                     Json::Value const* value, cmJSONState* state)
{
  if (!value) {
    out.reset();
    return true;
  }

  if (value->isBool()) {
    auto c = cm::make_unique<cmLinearBuildPresetsGraphInternal::ConstCondition>();
    c->Value = value->asBool();
    out = std::move(c);
    return true;
  }

  if (value->isNull()) {
    out = cm::make_unique<cmLinearBuildPresetsGraphInternal::NullCondition>();
    return true;
  }

  if (value->isObject()) {
    if (!value->isMember("type")) {
      cmLinearBuildPresetsErrors::INVALID_CONDITION(value, state);
      return false;
    }

    if (!(*value)["type"].isString()) {
      cmLinearBuildPresetsErrors::INVALID_CONDITION(value, state);
      return false;
    }
    auto type = (*value)["type"].asString();

    if (type == "const") {
      auto c = cm::make_unique<cmLinearBuildPresetsGraphInternal::ConstCondition>();
      CHECK_OK(ConstConditionHelper(*c, value, state));
      out = std::move(c);
      return true;
    }

    if (type == "equals" || type == "notEquals") {
      auto c = cm::make_unique<cmLinearBuildPresetsGraphInternal::EqualsCondition>();
      CHECK_OK(EqualsConditionHelper(*c, value, state));
      out = std::move(c);
      if (type == "notEquals") {
        out = InvertCondition(std::move(out));
      }
      return true;
    }

    if (type == "inList" || type == "notInList") {
      auto c = cm::make_unique<cmLinearBuildPresetsGraphInternal::InListCondition>();
      CHECK_OK(InListConditionHelper(*c, value, state));
      out = std::move(c);
      if (type == "notInList") {
        out = InvertCondition(std::move(out));
      }
      return true;
    }

    if (type == "matches" || type == "notMatches") {
      auto c =
        cm::make_unique<cmLinearBuildPresetsGraphInternal::MatchesCondition>();
      CHECK_OK(MatchesConditionHelper(*c, value, state));
      out = std::move(c);
      if (type == "notMatches") {
        out = InvertCondition(std::move(out));
      }
      return true;
    }

    if (type == "anyOf" || type == "allOf") {
      auto c =
        cm::make_unique<cmLinearBuildPresetsGraphInternal::AnyAllOfCondition>();
      c->StopValue = (type == "anyOf");
      CHECK_OK(AnyAllOfConditionHelper(*c, value, state));
      out = std::move(c);
      return true;
    }

    if (type == "not") {
      auto c = cm::make_unique<cmLinearBuildPresetsGraphInternal::NotCondition>();
      CHECK_OK(NotConditionHelper(*c, value, state));
      out = std::move(c);
      return true;
    }
  }

  cmLinearBuildPresetsErrors::INVALID_CONDITION(value, state);
  return false;
}

bool SubConditionHelper(std::unique_ptr<cmLinearBuildPresetsGraph::Condition>& out,
                        Json::Value const* value, cmJSONState* state)
{
  std::unique_ptr<cmLinearBuildPresetsGraph::Condition> ptr;
  auto result = ConditionHelper(ptr, value, state);
  if (ptr && ptr->IsNull()) {
    cmLinearBuildPresetsErrors::INVALID_CONDITION(value, state);
    return false;
  }
  out = std::move(ptr);
  return result;
}

bool EnvironmentHelper(cm::optional<std::string>& out,
                       Json::Value const* value, cmJSONState* state)
{
  if (!value || value->isNull()) {
    out = cm::nullopt;
    return true;
  }
  if (value->isString()) {
    out = value->asString();
    return true;
  }
  cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const VersionIntHelper =
  JSONHelperBuilder::Int(cmLinearBuildPresetsErrors::INVALID_VERSION);

auto const VersionHelper = JSONHelperBuilder::Required<int>(
  cmLinearBuildPresetsErrors::NO_VERSION, VersionIntHelper);

auto const VersionRangeHelper = JSONHelperBuilder::Checked<int>(
  cmLinearBuildPresetsErrors::UNRECOGNIZED_VERSION_RANGE(MIN_VERSION, MAX_VERSION),
  VersionHelper,
  [](int const v) -> bool { return v >= MIN_VERSION && v <= MAX_VERSION; });

auto const RootVersionHelper =
  JSONHelperBuilder::Object<int>(cmLinearBuildPresetsErrors::INVALID_ROOT_OBJECT)
    .Bind("version"_s, VersionRangeHelper, false);

auto const LinearBuildVersionUIntHelper =
  JSONHelperBuilder::UInt(cmLinearBuildPresetsErrors::INVALID_VERSION);

auto const LinearBuildVersionHelper =
  JSONHelperBuilder::Object<LinearBuildVersion>(JsonErrors::INVALID_NAMED_OBJECT_KEY,
                                          false)
    .Bind("major"_s, &LinearBuildVersion::Major, LinearBuildVersionUIntHelper, false)
    .Bind("minor"_s, &LinearBuildVersion::Minor, LinearBuildVersionUIntHelper, false)
    .Bind("patch"_s, &LinearBuildVersion::Patch, LinearBuildVersionUIntHelper, false);

auto const IncludeHelper =
  JSONHelperBuilder::String(cmLinearBuildPresetsErrors::INVALID_INCLUDE);

auto const IncludeVectorHelper = JSONHelperBuilder::Vector<std::string>(
  cmLinearBuildPresetsErrors::INVALID_INCLUDE, IncludeHelper);

auto const RootPresetsHelper =
  JSONHelperBuilder::Object<RootPresets>(
    cmLinearBuildPresetsErrors::INVALID_ROOT_OBJECT, false)
    .Bind<int>("version"_s, nullptr, VersionHelper)
    .Bind("configurePresets"_s, &RootPresets::ConfigurePresets,
          cmLinearBuildPresetsGraphInternal::ConfigurePresetsHelper, false)
    .Bind("buildPresets"_s, &RootPresets::BuildPresets,
          cmLinearBuildPresetsGraphInternal::BuildPresetsHelper, false)
    .Bind("testPresets"_s, &RootPresets::TestPresets,
          cmLinearBuildPresetsGraphInternal::TestPresetsHelper, false)
    .Bind("packagePresets"_s, &RootPresets::PackagePresets,
          cmLinearBuildPresetsGraphInternal::PackagePresetsHelper, false)
    .Bind("workflowPresets"_s, &RootPresets::WorkflowPresets,
          cmLinearBuildPresetsGraphInternal::WorkflowPresetsHelper, false)
    .Bind("linearbuildMinimumRequired"_s, &RootPresets::LinearBuildMinimumRequired,
          LinearBuildVersionHelper, false)
    .Bind("include"_s, &RootPresets::Include, IncludeVectorHelper, false)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          cmLinearBuildPresetsGraphInternal::VendorHelper(
                            cmLinearBuildPresetsErrors::INVALID_ROOT),
                          false)
    .Bind<std::nullptr_t>("$schema"_s, nullptr,
                          cmLinearBuildPresetsGraphInternal::SchemaHelper(), false);

class EnvironmentMacroExpander : public MacroExpander
{
public:
  ExpandMacroResult operator()(std::string const& macroNamespace,
                               std::string const& macroName,
                               std::string& macroOut,
                               int /*version*/) const override
  {
    if (macroNamespace == "penv") {
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

template <typename T>
bool RegisterPresets(std::vector<T>& presets, cmLinearBuildPresetsGraph::File* file,
                     std::map<std::string, PresetPair<T>>& out,
                     std::vector<std::string>& order, cmJSONState* state,
                     std::function<bool(T&)> const& checkVersionSupport = {})
{
  for (auto& preset : presets) {
    preset.OriginFile = file;
    if (preset.Name.empty()) {
      // No error, already handled by PresetNameHelper
      return false;
    }

    if (!ExpandImmediateMacros<T>(preset)) {
      cmLinearBuildPresetsErrors::INVALID_MACRO_EXPANSION(preset.Name, state);
      return false;
    }

    if (checkVersionSupport && !checkVersionSupport(preset)) {
      return false;
    }

    PresetPair<T> presetPair;
    presetPair.Unexpanded = preset;
    presetPair.Expanded = cm::nullopt;
    if (!out.emplace(preset.Name, presetPair).second) {
      cmLinearBuildPresetsErrors::DUPLICATE_PRESETS(preset.Name, state);
      return false;
    }

    order.push_back(preset.Name);
  }
  return true;
}
}

namespace cmLinearBuildPresetsGraphInternal {
bool PresetStringHelper(std::string& out, Json::Value const* value,
                        cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::String();
  return helper(out, value, state);
}

bool PresetNameHelper(std::string& out, Json::Value const* value,
                      cmJSONState* state)
{
  if (!value || !value->isString() || value->asString().empty()) {
    cmLinearBuildPresetsErrors::INVALID_PRESET_NAME(value, state);
    return false;
  }
  out = value->asString();
  return true;
}

bool PresetVectorStringHelper(std::vector<std::string>& out,
                              Json::Value const* value, cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Vector<std::string>(
    cmLinearBuildPresetsErrors::INVALID_PRESET,
    cmLinearBuildPresetsGraphInternal::PresetStringHelper);
  return helper(out, value, state);
}

bool PresetBoolHelper(bool& out, Json::Value const* value, cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Bool();
  return helper(out, value, state);
}

bool PresetOptionalBoolHelper(cm::optional<bool>& out,
                              Json::Value const* value, cmJSONState* state)
{
  static auto const helper =
    JSONHelperBuilder::Optional<bool>(PresetBoolHelper);
  return helper(out, value, state);
}

bool PresetIntHelper(int& out, Json::Value const* value, cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Int();
  return helper(out, value, state);
}

bool PresetOptionalIntHelper(cm::optional<int>& out, Json::Value const* value,
                             cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Optional<int>(PresetIntHelper);
  return helper(out, value, state);
}

bool PresetUIntHelper(unsigned int& out, Json::Value const* value,
                      cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::UInt();
  return helper(out, value, state);
}

bool PresetOptionalUIntHelper(cm::optional<unsigned int>& out,
                              Json::Value const* value, cmJSONState* state)
{
  static auto const helper =
    JSONHelperBuilder::Optional<unsigned int>(PresetUIntHelper);
  return helper(out, value, state);
}

bool PresetVectorIntHelper(std::vector<int>& out, Json::Value const* value,
                           cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Vector<int>(
    cmLinearBuildPresetsErrors::INVALID_PRESET, PresetIntHelper);
  return helper(out, value, state);
}

cmJSONHelper<std::nullptr_t> VendorHelper(ErrorGenerator const& error)
{
  return [error](std::nullptr_t& /*out*/, Json::Value const* value,
                 cmJSONState* state) -> bool {
    if (!value) {
      return true;
    }

    if (!value->isObject()) {
      error(value, state);
      return false;
    }

    return true;
  };
}

bool PresetConditionHelper(
  std::shared_ptr<cmLinearBuildPresetsGraph::Condition>& out,
  Json::Value const* value, cmJSONState* state)
{
  std::unique_ptr<cmLinearBuildPresetsGraph::Condition> ptr;
  auto result = ConditionHelper(ptr, value, state);
  if (result && ptr) {
    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";
    ptr->ConditionJson = Json::writeString(writer, *value);
  }
  out = std::move(ptr);
  return result;
}

bool PresetVectorOneOrMoreStringHelper(std::vector<std::string>& out,
                                       Json::Value const* value,
                                       cmJSONState* state)
{
  out.clear();
  if (!value) {
    return true;
  }

  if (value->isString()) {
    out.push_back(value->asString());
    return true;
  }

  return PresetVectorStringHelper(out, value, state);
}

bool EnvironmentMapHelper(
  std::map<std::string, cm::optional<std::string>>& out,
  Json::Value const* value, cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Map<cm::optional<std::string>>(
    cmLinearBuildPresetsErrors::INVALID_PRESET, EnvironmentHelper);

  return helper(out, value, state);
}

cmJSONHelper<std::nullptr_t> SchemaHelper()
{
  return [](std::nullptr_t&, Json::Value const*, cmJSONState*) -> bool {
    return true;
  };
}
}

bool cmLinearBuildPresetsGraph::ReadJSONFile(std::string const& filename,
                                       RootType rootType,
                                       ReadReason readReason,
                                       std::vector<File*>& inProgressFiles,
                                       File*& file, std::string& errMsg)
{
  bool result;

  for (auto const& f : this->Files) {
    if (cmSystemTools::SameFile(filename, f->Filename)) {
      file = f.get();
      auto fileIt =
        std::find(inProgressFiles.begin(), inProgressFiles.end(), file);
      if (fileIt != inProgressFiles.end()) {
        cmLinearBuildPresetsErrors::CYCLIC_INCLUDE(filename, &this->parseState);
        return false;
      }

      return true;
    }
  }

  Json::Value root;
  this->parseState = cmJSONState(filename, &root);
  if (!this->parseState.errors.empty()) {
    return false;
  }

  int v = 0;
  if ((result = RootVersionHelper(v, &root, &parseState)) != true) {
    return result;
  }

  // Support for build and test presets added in version 2.
  if (v < 2) {
    if (root.isMember("buildPresets")) {
      cmLinearBuildPresetsErrors::BUILD_TEST_PRESETS_UNSUPPORTED(
        &root["buildPresets"], &this->parseState);
      return false;
    }
    if (root.isMember("testPresets")) {
      cmLinearBuildPresetsErrors::BUILD_TEST_PRESETS_UNSUPPORTED(
        &root["testPresets"], &this->parseState);
      return false;
    }
  }

  // Support for package presets added in version 6.
  if (v < 6 && root.isMember("packagePresets")) {
    cmLinearBuildPresetsErrors::PACKAGE_PRESETS_UNSUPPORTED(&root["packagePresets"],
                                                      &this->parseState);
    return false;
  }

  // Support for workflow presets added in version 6.
  if (v < 6 && root.isMember("workflowPresets")) {
    cmLinearBuildPresetsErrors::WORKFLOW_PRESETS_UNSUPPORTED(
      &root["workflowPresets"], &this->parseState);
    return false;
  }

  // Support for include added in version 4.
  if (v < 4 && root.isMember("include")) {
    cmLinearBuildPresetsErrors::INCLUDE_UNSUPPORTED(&root["include"],
                                              &this->parseState);
    return false;
  }

  // Support for $schema added in version 8.
  if (v < 8 && root.isMember("$schema")) {
    cmLinearBuildPresetsErrors::SCHEMA_UNSUPPORTED(&this->parseState);
    return false;
  }

  // Support for $comment added in version 10.
  this->parseState.allowComments = (v >= 10);

  RootPresets presets;
  if ((result = RootPresetsHelper(presets, &root, &parseState)) != true) {
    return result;
  }

  unsigned int currentMajor = cmVersion::GetMajorVersion();
  unsigned int currentMinor = cmVersion::GetMinorVersion();
  unsigned int currentPatch = cmVersion::GetPatchVersion();
  auto const& required = presets.LinearBuildMinimumRequired;
  if (required.Major > currentMajor) {
    ErrorGenerator error = cmLinearBuildPresetsErrors::UNRECOGNIZED_LINEARBUILD_VERSION(
      "major", currentMajor, required.Major);
    error(&root["linearbuildMinimumRequired"]["major"], &this->parseState);
    return false;
  }
  if (required.Major == currentMajor) {
    if (required.Minor > currentMinor) {
      ErrorGenerator error = cmLinearBuildPresetsErrors::UNRECOGNIZED_LINEARBUILD_VERSION(
        "minor", currentMinor, required.Minor);
      error(&root["linearbuildMinimumRequired"]["minor"], &this->parseState);
      return false;
    }
    if (required.Minor == currentMinor && required.Patch > currentPatch) {
      ErrorGenerator error = cmLinearBuildPresetsErrors::UNRECOGNIZED_LINEARBUILD_VERSION(
        "patch", currentPatch, required.Patch);
      error(&root["linearbuildMinimumRequired"]["patch"], &this->parseState);
      return false;
    }
  }

  auto filePtr = cm::make_unique<File>();
  file = filePtr.get();
  this->Files.emplace_back(std::move(filePtr));
  inProgressFiles.emplace_back(file);
  file->Filename = filename;
  file->Version = v;
  file->ReachableFiles.insert(file);

  auto const configureVersionCheck = [&](ConfigurePreset& preset) -> bool {
    // Support for installDir presets added in version 3.
    if (v < 3 && !preset.InstallDir.empty()) {
      cmLinearBuildPresetsErrors::INSTALL_PREFIX_UNSUPPORTED(&root["installDir"],
                                                       &this->parseState);
      return false;
    }

    // Support for conditions added in version 3.
    if (v < 3 && preset.ConditionEvaluator) {
      cmLinearBuildPresetsErrors::CONDITION_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for toolchainFile presets added in version 3.
    if (v < 3 && !preset.ToolchainFile.empty()) {
      cmLinearBuildPresetsErrors::TOOLCHAIN_FILE_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for trace presets added in version 7.
    if (v < 7 &&
        (preset.TraceMode.has_value() || preset.TraceFormat.has_value() ||
         !preset.TraceRedirect.empty() || !preset.TraceSource.empty())) {
      cmLinearBuildPresetsErrors::TRACE_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for graphviz argument added in version 10.
    if (v < 10 && !preset.GraphVizFile.empty()) {
      cmLinearBuildPresetsErrors::GRAPHVIZ_FILE_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for diagnostics.
    return cmLinearBuildPresetsGraphInternal::CheckDiagnostics(&this->parseState, v,
                                                         preset);
  };

  if (!RegisterPresets<ConfigurePreset>(
        presets.ConfigurePresets, file, this->ConfigurePresets,
        this->ConfigurePresetOrder, &this->parseState,
        configureVersionCheck)) {
    return false;
  }

  auto const buildVersionCheck = [&](BuildPreset& preset) -> bool {
    // Support for conditions added in version 3.
    if (v < 3 && preset.ConditionEvaluator) {
      cmLinearBuildPresetsErrors::CONDITION_UNSUPPORTED(&this->parseState);
      return false;
    }
    return true;
  };

  if (!RegisterPresets<BuildPreset>(presets.BuildPresets, file,
                                    this->BuildPresets, this->BuildPresetOrder,
                                    &this->parseState, buildVersionCheck)) {
    return false;
  }

  auto const testVersionCheck = [&](TestPreset& preset) -> bool {
    // Support for conditions added in version 3.
    if (v < 3 && preset.ConditionEvaluator) {
      cmLinearBuildPresetsErrors::CONDITION_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for TestOutputTruncation added in version 5.
    if (v < 5 && preset.Output && preset.Output->TestOutputTruncation) {
      cmLinearBuildPresetsErrors::TEST_OUTPUT_TRUNCATION_UNSUPPORTED(
        &this->parseState);
      return false;
    }

    // Support for outputJUnitFile added in version 6.
    if (v < 6 && preset.Output && !preset.Output->OutputJUnitFile.empty()) {
      cmLinearBuildPresetsErrors::CTEST_JUNIT_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for processor-count-based jobs added in version 11.
    if (v < 11 && preset.Execution && preset.Execution->Jobs.has_value() &&
        !preset.Execution->Jobs->has_value()) {
      cmLinearBuildPresetsErrors::JOBS_PROC_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for testPassthroughArguments added in version 12.
    if (v < 12 && preset.Execution &&
        !preset.Execution->TestPassthroughArguments.empty()) {
      cmLinearBuildPresetsErrors::PASSTHROUGH_ARGS_UNSUPPORTED(&this->parseState);
      return false;
    }

    return true;
  };

  if (!RegisterPresets<TestPreset>(presets.TestPresets, file,
                                   this->TestPresets, this->TestPresetOrder,
                                   &this->parseState, testVersionCheck)) {
    return false;
  }

  // Support for conditions added in version 3, but this requires version 5
  // already, so no action needed.
  if (!RegisterPresets<PackagePreset>(
        presets.PackagePresets, file, this->PackagePresets,
        this->PackagePresetOrder, &this->parseState)) {
    return false;
  }

  // Support for conditions added in version 3, but this requires version 6
  // already, so no action needed.
  if (!RegisterPresets<WorkflowPreset>(
        presets.WorkflowPresets, file, this->WorkflowPresets,
        this->WorkflowPresetOrder, &this->parseState)) {
    return false;
  }

  auto const includeFile = [this, &inProgressFiles,
                            file](std::string const& include,
                                  RootType rootType2, ReadReason readReason2,
                                  std::string& FailureMessage) -> bool {
    bool r;
    File* includedFile;
    if ((r =
           this->ReadJSONFile(include, rootType2, readReason2, inProgressFiles,
                              includedFile, FailureMessage)) != true) {
      return r;
    }

    file->ReachableFiles.insert(includedFile->ReachableFiles.begin(),
                                includedFile->ReachableFiles.end());
    return true;
  };

  MacroExpanderVector macroExpanders{};

  if (v >= 9) {
    macroExpanders.push_back(
      cm::make_unique<BaseMacroExpander>(*this, filename));
  }
  macroExpanders.push_back(cm::make_unique<EnvironmentMacroExpander>());

  for (Json::ArrayIndex i = 0; i < presets.Include.size(); ++i) {
    auto include = presets.Include[i];

    // Support for macro expansion in includes added in version 7
    if (v >= 7) {
      if (ExpandMacros(include, macroExpanders, v) != ExpandMacroResult::Ok) {
        cmLinearBuildPresetsErrors::INVALID_INCLUDE(&root["include"][i],
                                              &this->parseState);
        return false;
      }
    }

    if (!cmSystemTools::FileIsFullPath(include)) {
      auto directory = cmSystemTools::GetFilenamePath(filename);
      include = cmStrCat(directory, '/', std::move(include));
    }

    if ((result = includeFile(include, rootType, ReadReason::Included,
                              errMsg)) != true) {
      return result;
    }
  }

  if (rootType == RootType::User && readReason == ReadReason::Root) {
    auto linearbuildPresetsFilename = GetFilename(this->SourceDir);
    if (cmSystemTools::FileExists(linearbuildPresetsFilename)) {
      if ((result = includeFile(linearbuildPresetsFilename, RootType::Project,
                                ReadReason::Root, errMsg)) != true) {
        return result;
      }
    }
  }

  inProgressFiles.pop_back();
  return true;
}
