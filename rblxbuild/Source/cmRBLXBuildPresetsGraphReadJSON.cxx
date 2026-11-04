/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
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

#include "cmRBLXBuildPresetsErrors.h"
#include "cmRBLXBuildPresetsGraph.h"
#include "cmRBLXBuildPresetsGraphInternal.h"
#include "cmJSONHelpers.h"
#include "cmJSONState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmVersion.h"

namespace {
using CacheVariable = cmRBLXBuildPresetsGraph::CacheVariable;
using ConfigurePreset = cmRBLXBuildPresetsGraph::ConfigurePreset;
using BuildPreset = cmRBLXBuildPresetsGraph::BuildPreset;
using TestPreset = cmRBLXBuildPresetsGraph::TestPreset;
using PackagePreset = cmRBLXBuildPresetsGraph::PackagePreset;
using WorkflowPreset = cmRBLXBuildPresetsGraph::WorkflowPreset;
template <typename T>
using PresetPair = cmRBLXBuildPresetsGraph::PresetPair<T>;
using ArchToolsetStrategy = cmRBLXBuildPresetsGraph::ArchToolsetStrategy;
using JSONHelperBuilder = cmJSONHelperBuilder;
using ExpandMacroResult = cmRBLXBuildPresetsGraphInternal::ExpandMacroResult;
using MacroExpander = cmRBLXBuildPresetsGraphInternal::MacroExpander;
using MacroExpanderVector = cmRBLXBuildPresetsGraphInternal::MacroExpanderVector;
using cmRBLXBuildPresetsGraphInternal::BaseMacroExpander;
using cmRBLXBuildPresetsGraphInternal::ExpandMacros;
using cmRBLXBuildPresetsGraphInternal::ExpandImmediateMacros;

constexpr int MIN_VERSION = 1;
constexpr int MAX_VERSION = 13;

struct RBLXBuildVersion
{
  unsigned int Major = 0;
  unsigned int Minor = 0;
  unsigned int Patch = 0;
};

struct RootPresets
{
  RBLXBuildVersion RBLXBuildMinimumRequired;
  std::vector<ConfigurePreset> ConfigurePresets;
  std::vector<BuildPreset> BuildPresets;
  std::vector<TestPreset> TestPresets;
  std::vector<PackagePreset> PackagePresets;
  std::vector<WorkflowPreset> WorkflowPresets;
  std::vector<std::string> Include;
};

std::unique_ptr<cmRBLXBuildPresetsGraphInternal::NotCondition> InvertCondition(
  std::unique_ptr<cmRBLXBuildPresetsGraph::Condition> condition)
{
  auto retval = cm::make_unique<cmRBLXBuildPresetsGraphInternal::NotCondition>();
  retval->SubCondition = std::move(condition);
  return retval;
}

auto const ConditionStringHelper = JSONHelperBuilder::String();

auto const ConditionBoolHelper = JSONHelperBuilder::Bool();

auto const ConditionStringListHelper = JSONHelperBuilder::Vector<std::string>(
  cmRBLXBuildPresetsErrors::INVALID_CONDITION, ConditionStringHelper);

auto const ConstConditionHelper =
  JSONHelperBuilder::Object<cmRBLXBuildPresetsGraphInternal::ConstCondition>(
    cmRBLXBuildPresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("value"_s, &cmRBLXBuildPresetsGraphInternal::ConstCondition::Value,
          ConditionBoolHelper, true);

auto const EqualsConditionHelper =
  JSONHelperBuilder::Object<cmRBLXBuildPresetsGraphInternal::EqualsCondition>(
    cmRBLXBuildPresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("lhs"_s, &cmRBLXBuildPresetsGraphInternal::EqualsCondition::Lhs,
          ConditionStringHelper, true)
    .Bind("rhs"_s, &cmRBLXBuildPresetsGraphInternal::EqualsCondition::Rhs,
          ConditionStringHelper, true);

auto const InListConditionHelper =
  JSONHelperBuilder::Object<cmRBLXBuildPresetsGraphInternal::InListCondition>(
    cmRBLXBuildPresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("string"_s, &cmRBLXBuildPresetsGraphInternal::InListCondition::String,
          ConditionStringHelper, true)
    .Bind("list"_s, &cmRBLXBuildPresetsGraphInternal::InListCondition::List,
          ConditionStringListHelper, true);

auto const MatchesConditionHelper =
  JSONHelperBuilder::Object<cmRBLXBuildPresetsGraphInternal::MatchesCondition>(
    cmRBLXBuildPresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("string"_s, &cmRBLXBuildPresetsGraphInternal::MatchesCondition::String,
          ConditionStringHelper, true)
    .Bind("regex"_s, &cmRBLXBuildPresetsGraphInternal::MatchesCondition::Regex,
          ConditionStringHelper, true);

bool SubConditionHelper(std::unique_ptr<cmRBLXBuildPresetsGraph::Condition>& out,
                        Json::Value const* value, cmJSONState* state);

auto const ListConditionVectorHelper =
  JSONHelperBuilder::Vector<std::unique_ptr<cmRBLXBuildPresetsGraph::Condition>>(
    cmRBLXBuildPresetsErrors::INVALID_CONDITION, SubConditionHelper);
auto const AnyAllOfConditionHelper =
  JSONHelperBuilder::Object<cmRBLXBuildPresetsGraphInternal::AnyAllOfCondition>(
    cmRBLXBuildPresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("conditions"_s,
          &cmRBLXBuildPresetsGraphInternal::AnyAllOfCondition::Conditions,
          ListConditionVectorHelper);

auto const NotConditionHelper =
  JSONHelperBuilder::Object<cmRBLXBuildPresetsGraphInternal::NotCondition>(
    cmRBLXBuildPresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("condition"_s,
          &cmRBLXBuildPresetsGraphInternal::NotCondition::SubCondition,
          SubConditionHelper);

bool ConditionHelper(std::unique_ptr<cmRBLXBuildPresetsGraph::Condition>& out,
                     Json::Value const* value, cmJSONState* state)
{
  if (!value) {
    out.reset();
    return true;
  }

  if (value->isBool()) {
    auto c = cm::make_unique<cmRBLXBuildPresetsGraphInternal::ConstCondition>();
    c->Value = value->asBool();
    out = std::move(c);
    return true;
  }

  if (value->isNull()) {
    out = cm::make_unique<cmRBLXBuildPresetsGraphInternal::NullCondition>();
    return true;
  }

  if (value->isObject()) {
    if (!value->isMember("type")) {
      cmRBLXBuildPresetsErrors::INVALID_CONDITION(value, state);
      return false;
    }

    if (!(*value)["type"].isString()) {
      cmRBLXBuildPresetsErrors::INVALID_CONDITION(value, state);
      return false;
    }
    auto type = (*value)["type"].asString();

    if (type == "const") {
      auto c = cm::make_unique<cmRBLXBuildPresetsGraphInternal::ConstCondition>();
      CHECK_OK(ConstConditionHelper(*c, value, state));
      out = std::move(c);
      return true;
    }

    if (type == "equals" || type == "notEquals") {
      auto c = cm::make_unique<cmRBLXBuildPresetsGraphInternal::EqualsCondition>();
      CHECK_OK(EqualsConditionHelper(*c, value, state));
      out = std::move(c);
      if (type == "notEquals") {
        out = InvertCondition(std::move(out));
      }
      return true;
    }

    if (type == "inList" || type == "notInList") {
      auto c = cm::make_unique<cmRBLXBuildPresetsGraphInternal::InListCondition>();
      CHECK_OK(InListConditionHelper(*c, value, state));
      out = std::move(c);
      if (type == "notInList") {
        out = InvertCondition(std::move(out));
      }
      return true;
    }

    if (type == "matches" || type == "notMatches") {
      auto c =
        cm::make_unique<cmRBLXBuildPresetsGraphInternal::MatchesCondition>();
      CHECK_OK(MatchesConditionHelper(*c, value, state));
      out = std::move(c);
      if (type == "notMatches") {
        out = InvertCondition(std::move(out));
      }
      return true;
    }

    if (type == "anyOf" || type == "allOf") {
      auto c =
        cm::make_unique<cmRBLXBuildPresetsGraphInternal::AnyAllOfCondition>();
      c->StopValue = (type == "anyOf");
      CHECK_OK(AnyAllOfConditionHelper(*c, value, state));
      out = std::move(c);
      return true;
    }

    if (type == "not") {
      auto c = cm::make_unique<cmRBLXBuildPresetsGraphInternal::NotCondition>();
      CHECK_OK(NotConditionHelper(*c, value, state));
      out = std::move(c);
      return true;
    }
  }

  cmRBLXBuildPresetsErrors::INVALID_CONDITION(value, state);
  return false;
}

bool SubConditionHelper(std::unique_ptr<cmRBLXBuildPresetsGraph::Condition>& out,
                        Json::Value const* value, cmJSONState* state)
{
  std::unique_ptr<cmRBLXBuildPresetsGraph::Condition> ptr;
  auto result = ConditionHelper(ptr, value, state);
  if (ptr && ptr->IsNull()) {
    cmRBLXBuildPresetsErrors::INVALID_CONDITION(value, state);
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
  cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const VersionIntHelper =
  JSONHelperBuilder::Int(cmRBLXBuildPresetsErrors::INVALID_VERSION);

auto const VersionHelper = JSONHelperBuilder::Required<int>(
  cmRBLXBuildPresetsErrors::NO_VERSION, VersionIntHelper);

auto const VersionRangeHelper = JSONHelperBuilder::Checked<int>(
  cmRBLXBuildPresetsErrors::UNRECOGNIZED_VERSION_RANGE(MIN_VERSION, MAX_VERSION),
  VersionHelper,
  [](int const v) -> bool { return v >= MIN_VERSION && v <= MAX_VERSION; });

auto const RootVersionHelper =
  JSONHelperBuilder::Object<int>(cmRBLXBuildPresetsErrors::INVALID_ROOT_OBJECT)
    .Bind("version"_s, VersionRangeHelper, false);

auto const RBLXBuildVersionUIntHelper =
  JSONHelperBuilder::UInt(cmRBLXBuildPresetsErrors::INVALID_VERSION);

auto const RBLXBuildVersionHelper =
  JSONHelperBuilder::Object<RBLXBuildVersion>(JsonErrors::INVALID_NAMED_OBJECT_KEY,
                                          false)
    .Bind("major"_s, &RBLXBuildVersion::Major, RBLXBuildVersionUIntHelper, false)
    .Bind("minor"_s, &RBLXBuildVersion::Minor, RBLXBuildVersionUIntHelper, false)
    .Bind("patch"_s, &RBLXBuildVersion::Patch, RBLXBuildVersionUIntHelper, false);

auto const IncludeHelper =
  JSONHelperBuilder::String(cmRBLXBuildPresetsErrors::INVALID_INCLUDE);

auto const IncludeVectorHelper = JSONHelperBuilder::Vector<std::string>(
  cmRBLXBuildPresetsErrors::INVALID_INCLUDE, IncludeHelper);

auto const RootPresetsHelper =
  JSONHelperBuilder::Object<RootPresets>(
    cmRBLXBuildPresetsErrors::INVALID_ROOT_OBJECT, false)
    .Bind<int>("version"_s, nullptr, VersionHelper)
    .Bind("configurePresets"_s, &RootPresets::ConfigurePresets,
          cmRBLXBuildPresetsGraphInternal::ConfigurePresetsHelper, false)
    .Bind("buildPresets"_s, &RootPresets::BuildPresets,
          cmRBLXBuildPresetsGraphInternal::BuildPresetsHelper, false)
    .Bind("testPresets"_s, &RootPresets::TestPresets,
          cmRBLXBuildPresetsGraphInternal::TestPresetsHelper, false)
    .Bind("packagePresets"_s, &RootPresets::PackagePresets,
          cmRBLXBuildPresetsGraphInternal::PackagePresetsHelper, false)
    .Bind("workflowPresets"_s, &RootPresets::WorkflowPresets,
          cmRBLXBuildPresetsGraphInternal::WorkflowPresetsHelper, false)
    .Bind("rblxbuildMinimumRequired"_s, &RootPresets::RBLXBuildMinimumRequired,
          RBLXBuildVersionHelper, false)
    .Bind("include"_s, &RootPresets::Include, IncludeVectorHelper, false)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          cmRBLXBuildPresetsGraphInternal::VendorHelper(
                            cmRBLXBuildPresetsErrors::INVALID_ROOT),
                          false)
    .Bind<std::nullptr_t>("$schema"_s, nullptr,
                          cmRBLXBuildPresetsGraphInternal::SchemaHelper(), false);

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
bool RegisterPresets(std::vector<T>& presets, cmRBLXBuildPresetsGraph::File* file,
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
      cmRBLXBuildPresetsErrors::INVALID_MACRO_EXPANSION(preset.Name, state);
      return false;
    }

    if (checkVersionSupport && !checkVersionSupport(preset)) {
      return false;
    }

    PresetPair<T> presetPair;
    presetPair.Unexpanded = preset;
    presetPair.Expanded = cm::nullopt;
    if (!out.emplace(preset.Name, presetPair).second) {
      cmRBLXBuildPresetsErrors::DUPLICATE_PRESETS(preset.Name, state);
      return false;
    }

    order.push_back(preset.Name);
  }
  return true;
}
}

namespace cmRBLXBuildPresetsGraphInternal {
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
    cmRBLXBuildPresetsErrors::INVALID_PRESET_NAME(value, state);
    return false;
  }
  out = value->asString();
  return true;
}

bool PresetVectorStringHelper(std::vector<std::string>& out,
                              Json::Value const* value, cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Vector<std::string>(
    cmRBLXBuildPresetsErrors::INVALID_PRESET,
    cmRBLXBuildPresetsGraphInternal::PresetStringHelper);
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
    cmRBLXBuildPresetsErrors::INVALID_PRESET, PresetIntHelper);
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
  std::shared_ptr<cmRBLXBuildPresetsGraph::Condition>& out,
  Json::Value const* value, cmJSONState* state)
{
  std::unique_ptr<cmRBLXBuildPresetsGraph::Condition> ptr;
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
    cmRBLXBuildPresetsErrors::INVALID_PRESET, EnvironmentHelper);

  return helper(out, value, state);
}

cmJSONHelper<std::nullptr_t> SchemaHelper()
{
  return [](std::nullptr_t&, Json::Value const*, cmJSONState*) -> bool {
    return true;
  };
}
}

bool cmRBLXBuildPresetsGraph::ReadJSONFile(std::string const& filename,
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
        cmRBLXBuildPresetsErrors::CYCLIC_INCLUDE(filename, &this->parseState);
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
      cmRBLXBuildPresetsErrors::BUILD_TEST_PRESETS_UNSUPPORTED(
        &root["buildPresets"], &this->parseState);
      return false;
    }
    if (root.isMember("testPresets")) {
      cmRBLXBuildPresetsErrors::BUILD_TEST_PRESETS_UNSUPPORTED(
        &root["testPresets"], &this->parseState);
      return false;
    }
  }

  // Support for package presets added in version 6.
  if (v < 6 && root.isMember("packagePresets")) {
    cmRBLXBuildPresetsErrors::PACKAGE_PRESETS_UNSUPPORTED(&root["packagePresets"],
                                                      &this->parseState);
    return false;
  }

  // Support for workflow presets added in version 6.
  if (v < 6 && root.isMember("workflowPresets")) {
    cmRBLXBuildPresetsErrors::WORKFLOW_PRESETS_UNSUPPORTED(
      &root["workflowPresets"], &this->parseState);
    return false;
  }

  // Support for include added in version 4.
  if (v < 4 && root.isMember("include")) {
    cmRBLXBuildPresetsErrors::INCLUDE_UNSUPPORTED(&root["include"],
                                              &this->parseState);
    return false;
  }

  // Support for $schema added in version 8.
  if (v < 8 && root.isMember("$schema")) {
    cmRBLXBuildPresetsErrors::SCHEMA_UNSUPPORTED(&this->parseState);
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
  auto const& required = presets.RBLXBuildMinimumRequired;
  if (required.Major > currentMajor) {
    ErrorGenerator error = cmRBLXBuildPresetsErrors::UNRECOGNIZED_RBLXBUILD_VERSION(
      "major", currentMajor, required.Major);
    error(&root["rblxbuildMinimumRequired"]["major"], &this->parseState);
    return false;
  }
  if (required.Major == currentMajor) {
    if (required.Minor > currentMinor) {
      ErrorGenerator error = cmRBLXBuildPresetsErrors::UNRECOGNIZED_RBLXBUILD_VERSION(
        "minor", currentMinor, required.Minor);
      error(&root["rblxbuildMinimumRequired"]["minor"], &this->parseState);
      return false;
    }
    if (required.Minor == currentMinor && required.Patch > currentPatch) {
      ErrorGenerator error = cmRBLXBuildPresetsErrors::UNRECOGNIZED_RBLXBUILD_VERSION(
        "patch", currentPatch, required.Patch);
      error(&root["rblxbuildMinimumRequired"]["patch"], &this->parseState);
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
      cmRBLXBuildPresetsErrors::INSTALL_PREFIX_UNSUPPORTED(&root["installDir"],
                                                       &this->parseState);
      return false;
    }

    // Support for conditions added in version 3.
    if (v < 3 && preset.ConditionEvaluator) {
      cmRBLXBuildPresetsErrors::CONDITION_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for toolchainFile presets added in version 3.
    if (v < 3 && !preset.ToolchainFile.empty()) {
      cmRBLXBuildPresetsErrors::TOOLCHAIN_FILE_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for trace presets added in version 7.
    if (v < 7 &&
        (preset.TraceMode.has_value() || preset.TraceFormat.has_value() ||
         !preset.TraceRedirect.empty() || !preset.TraceSource.empty())) {
      cmRBLXBuildPresetsErrors::TRACE_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for graphviz argument added in version 10.
    if (v < 10 && !preset.GraphVizFile.empty()) {
      cmRBLXBuildPresetsErrors::GRAPHVIZ_FILE_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for diagnostics.
    return cmRBLXBuildPresetsGraphInternal::CheckDiagnostics(&this->parseState, v,
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
      cmRBLXBuildPresetsErrors::CONDITION_UNSUPPORTED(&this->parseState);
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
      cmRBLXBuildPresetsErrors::CONDITION_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for TestOutputTruncation added in version 5.
    if (v < 5 && preset.Output && preset.Output->TestOutputTruncation) {
      cmRBLXBuildPresetsErrors::TEST_OUTPUT_TRUNCATION_UNSUPPORTED(
        &this->parseState);
      return false;
    }

    // Support for outputJUnitFile added in version 6.
    if (v < 6 && preset.Output && !preset.Output->OutputJUnitFile.empty()) {
      cmRBLXBuildPresetsErrors::CTEST_JUNIT_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for processor-count-based jobs added in version 11.
    if (v < 11 && preset.Execution && preset.Execution->Jobs.has_value() &&
        !preset.Execution->Jobs->has_value()) {
      cmRBLXBuildPresetsErrors::JOBS_PROC_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for testPassthroughArguments added in version 12.
    if (v < 12 && preset.Execution &&
        !preset.Execution->TestPassthroughArguments.empty()) {
      cmRBLXBuildPresetsErrors::PASSTHROUGH_ARGS_UNSUPPORTED(&this->parseState);
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
        cmRBLXBuildPresetsErrors::INVALID_INCLUDE(&root["include"][i],
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
    auto rblxbuildPresetsFilename = GetFilename(this->SourceDir);
    if (cmSystemTools::FileExists(rblxbuildPresetsFilename)) {
      if ((result = includeFile(rblxbuildPresetsFilename, RootType::Project,
                                ReadReason::Root, errMsg)) != true) {
        return result;
      }
    }
  }

  inProgressFiles.pop_back();
  return true;
}
