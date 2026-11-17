/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmLinearBuildPresetsErrors.h"
#include "cmLinearBuildPresetsGraph.h"
#include "cmJSONHelpers.h"
#include "cmSystemTools.h"

#define CHECK_OK(expr)                                                        \
  do {                                                                        \
    auto _result = expr;                                                      \
    if (_result != true)                                                      \
      return _result;                                                         \
  } while (false)

namespace cmLinearBuildPresetsGraphInternal {
enum class ExpandMacroResult
{
  Ok,
  Ignore,
  Defer,
  Error,
};

class MacroExpander
{
public:
  virtual ExpandMacroResult operator()(std::string const& macroNamespace,
                                       std::string const& macroName,
                                       std::string& macroOut,
                                       int version) const = 0;
  virtual ~MacroExpander() = default;
};
using MacroExpanderVector = std::vector<std::unique_ptr<MacroExpander>>;

ExpandMacroResult ExpandMacros(std::string& out,
                               MacroExpanderVector const& macroExpanders,
                               int version);

ExpandMacroResult ExpandMacro(std::string& out,
                              std::string const& macroNamespace,
                              std::string const& macroName,
                              MacroExpanderVector const& macroExpanders,
                              int version);
}

class cmLinearBuildPresetsGraph::Condition
{
public:
  virtual ~Condition() = default;

  std::string ConditionJson;

  virtual bool Evaluate(
    cmLinearBuildPresetsGraphInternal::MacroExpanderVector const& expanders,
    int version, cm::optional<bool>& out) const = 0;
  virtual bool IsNull() const { return false; }
};

namespace cmLinearBuildPresetsGraphInternal {
class BaseMacroExpander : public MacroExpander
{
  cmLinearBuildPresetsGraph const& Graph;
  cm::optional<std::string> File;

public:
  BaseMacroExpander(cmLinearBuildPresetsGraph const& graph)
    : Graph(graph)
  {
  }
  BaseMacroExpander(cmLinearBuildPresetsGraph const& graph, std::string const& file)
    : Graph(graph)
    , File(file)
  {
  }
  ExpandMacroResult operator()(std::string const& macroNamespace,
                               std::string const& macroName,
                               std::string& macroOut,
                               int version) const override;
};

template <typename T>
std::string const* GetConfigurePresetName(T const& /*preset*/)
{
  return nullptr;
}

inline std::string const* GetConfigurePresetName(
  cmLinearBuildPresetsGraph::ConfigurePreset const& preset)
{
  return &preset.Name;
}

inline std::string const* GetConfigurePresetName(
  cmLinearBuildPresetsGraph::BuildPreset const& preset)
{
  return &preset.ConfigurePreset;
}

inline std::string const* GetConfigurePresetName(
  cmLinearBuildPresetsGraph::TestPreset const& preset)
{
  return &preset.ConfigurePreset;
}

inline std::string const* GetConfigurePresetName(
  cmLinearBuildPresetsGraph::PackagePreset const& preset)
{
  return &preset.ConfigurePreset;
}

template <typename T>
class PresetMacroExpander : public MacroExpander
{
  cmLinearBuildPresetsGraph const& Graph;
  T const& Preset;

public:
  PresetMacroExpander(cmLinearBuildPresetsGraph const& graph, T const& preset)
    : Graph(graph)
    , Preset(preset)
  {
  }
  ExpandMacroResult operator()(std::string const& macroNamespace,
                               std::string const& macroName,
                               std::string& macroOut,
                               int version) const override
  {
    if (macroNamespace.empty()) {
      if (macroName == "presetName") {
        macroOut += Preset.Name;
        return ExpandMacroResult::Ok;
      }
      if (macroName == "configurePresetName") {
        if (version < 13) {
          return ExpandMacroResult::Error;
        }
        if (auto const* configurePresetName = GetConfigurePresetName(Preset)) {
          macroOut += *configurePresetName;
          return ExpandMacroResult::Ok;
        }
        return ExpandMacroResult::Error;
      }
      if (macroName == "generator") {
        // Generator only makes sense if preset is not hidden.
        if (!Preset.Hidden) {
          macroOut += Graph.GetGeneratorForPreset(Preset.Name);
        }
        return ExpandMacroResult::Ok;
      }
      if (macroName == "fileDir") {
        if (version < 4) {
          return ExpandMacroResult::Error;
        }
        macroOut +=
          cmSystemTools::GetParentDirectory(Preset.OriginFile->Filename);
        return ExpandMacroResult::Ok;
      }
    }
    return ExpandMacroResult::Ignore;
  }
};

template <class T>
class ImmediateMacroExpander : public MacroExpander
{
  T const& Preset;

public:
  ImmediateMacroExpander(T const& preset)
    : Preset(preset)
  {
  }
  ExpandMacroResult operator()(std::string const& macroNamespace,
                               std::string const& macroName,
                               std::string& macroOut,
                               int version) const override
  {
    if (macroNamespace.empty()) {
      if (macroName == "fileDir") {
        if (version < 12) {
          return ExpandMacroResult::Defer;
        }
        macroOut +=
          cmSystemTools::GetParentDirectory(Preset.OriginFile->Filename);
        return ExpandMacroResult::Ok;
      }
    }
    return ExpandMacroResult::Defer;
  }
};

template <typename T>
bool ExpandImmediateMacros(T& preset);

extern template bool ExpandImmediateMacros<
  cmLinearBuildPresetsGraph::ConfigurePreset>(cmLinearBuildPresetsGraph::ConfigurePreset&);
extern template bool ExpandImmediateMacros<cmLinearBuildPresetsGraph::BuildPreset>(
  cmLinearBuildPresetsGraph::BuildPreset&);
extern template bool ExpandImmediateMacros<cmLinearBuildPresetsGraph::TestPreset>(
  cmLinearBuildPresetsGraph::TestPreset&);
extern template bool ExpandImmediateMacros<cmLinearBuildPresetsGraph::PackagePreset>(
  cmLinearBuildPresetsGraph::PackagePreset&);
extern template bool ExpandImmediateMacros<
  cmLinearBuildPresetsGraph::WorkflowPreset>(cmLinearBuildPresetsGraph::WorkflowPreset&);

class NullCondition : public cmLinearBuildPresetsGraph::Condition
{
  bool Evaluate(MacroExpanderVector const& /*expanders*/, int /*version*/,
                cm::optional<bool>& out) const override
  {
    out = true;
    return true;
  }

  bool IsNull() const override { return true; }
};

class ConstCondition : public cmLinearBuildPresetsGraph::Condition
{
public:
  bool Evaluate(MacroExpanderVector const& /*expanders*/, int /*version*/,
                cm::optional<bool>& out) const override
  {
    out = this->Value;
    return true;
  }

  bool Value;
};

class EqualsCondition : public cmLinearBuildPresetsGraph::Condition
{
public:
  bool Evaluate(MacroExpanderVector const& expanders, int version,
                cm::optional<bool>& out) const override;

  std::string Lhs;
  std::string Rhs;
};

class InListCondition : public cmLinearBuildPresetsGraph::Condition
{
public:
  bool Evaluate(MacroExpanderVector const& expanders, int version,
                cm::optional<bool>& out) const override;

  std::string String;
  std::vector<std::string> List;
};

class MatchesCondition : public cmLinearBuildPresetsGraph::Condition
{
public:
  bool Evaluate(MacroExpanderVector const& expanders, int version,
                cm::optional<bool>& out) const override;

  std::string String;
  std::string Regex;
};

class AnyAllOfCondition : public cmLinearBuildPresetsGraph::Condition
{
public:
  bool Evaluate(MacroExpanderVector const& expanders, int version,
                cm::optional<bool>& out) const override;

  std::vector<std::unique_ptr<Condition>> Conditions;
  bool StopValue;
};

class NotCondition : public cmLinearBuildPresetsGraph::Condition
{
public:
  bool Evaluate(MacroExpanderVector const& expanders, int version,
                cm::optional<bool>& out) const override;

  std::unique_ptr<Condition> SubCondition;
};

bool PresetStringHelper(std::string& out, Json::Value const* value,
                        cmJSONState* state);

bool PresetNameHelper(std::string& out, Json::Value const* value,
                      cmJSONState* state);

bool PresetVectorStringHelper(std::vector<std::string>& out,
                              Json::Value const* value, cmJSONState* state);

bool PresetBoolHelper(bool& out, Json::Value const* value, cmJSONState* state);

bool PresetOptionalBoolHelper(cm::optional<bool>& out,
                              Json::Value const* value, cmJSONState* state);

template <typename K>
bool PresetMapToBoolHelper(std::map<K, bool>& out, Json::Value const* value,
                           K key, cmJSONState* state)
{
  cm::optional<bool> temp;
  if (!PresetOptionalBoolHelper(temp, value, state)) {
    return false;
  }

  if (temp) {
    out[key] = *temp;
  }

  return true;
}

bool PresetIntHelper(int& out, Json::Value const* value, cmJSONState* state);

bool PresetOptionalIntHelper(cm::optional<int>& out, Json::Value const* value,
                             cmJSONState* state);

bool PresetUIntHelper(unsigned int& out, Json::Value const* value,
                      cmJSONState* state);

bool PresetOptionalUIntHelper(cm::optional<unsigned int>& out,
                              Json::Value const* value, cmJSONState* state);

bool PresetVectorIntHelper(std::vector<int>& out, Json::Value const* value,
                           cmJSONState* state);

bool ConfigurePresetsHelper(
  std::vector<cmLinearBuildPresetsGraph::ConfigurePreset>& out,
  Json::Value const* value, cmJSONState* state);

bool BuildPresetsHelper(std::vector<cmLinearBuildPresetsGraph::BuildPreset>& out,
                        Json::Value const* value, cmJSONState* state);

bool TestPresetsHelper(std::vector<cmLinearBuildPresetsGraph::TestPreset>& out,
                       Json::Value const* value, cmJSONState* state);

bool PackagePresetsHelper(std::vector<cmLinearBuildPresetsGraph::PackagePreset>& out,
                          Json::Value const* value, cmJSONState* state);

bool WorkflowPresetsHelper(
  std::vector<cmLinearBuildPresetsGraph::WorkflowPreset>& out,
  Json::Value const* value, cmJSONState* state);

cm::string_view GetDiagnosticJSONName(cmDiagnosticCategory category);

cmJSONHelper<std::nullptr_t> VendorHelper(ErrorGenerator const& error);

bool PresetConditionHelper(
  std::shared_ptr<cmLinearBuildPresetsGraph::Condition>& out,
  Json::Value const* value, cmJSONState* state);

bool PresetVectorOneOrMoreStringHelper(std::vector<std::string>& out,
                                       Json::Value const* value,
                                       cmJSONState* state);

bool EnvironmentMapHelper(
  std::map<std::string, cm::optional<std::string>>& out,
  Json::Value const* value, cmJSONState* state);

cmJSONHelper<std::nullptr_t> SchemaHelper();

bool CheckDiagnostics(cmJSONState* state, int version,
                      cmLinearBuildPresetsGraph::ConfigurePreset& preset);

// Binds the fields common to every preset kind.
template <typename T>
cmJSONHelperBuilder::Object<T> BindPresetIdentityFields(
  cmJSONHelperBuilder::Object<T> obj)
{
  obj.Bind("name"_s, &cmLinearBuildPresetsGraph::Preset::Name, PresetNameHelper)
    .Bind("inherits"_s, &cmLinearBuildPresetsGraph::Preset::Inherits,
          PresetVectorOneOrMoreStringHelper, false)
    .Bind("hidden"_s, &cmLinearBuildPresetsGraph::Preset::Hidden, PresetBoolHelper,
          false)
    .template Bind<std::nullptr_t>(
      "vendor"_s, nullptr, VendorHelper(cmLinearBuildPresetsErrors::INVALID_PRESET),
      false)
    .Bind("displayName"_s, &cmLinearBuildPresetsGraph::Preset::DisplayName,
          PresetStringHelper, false)
    .Bind("description"_s, &cmLinearBuildPresetsGraph::Preset::Description,
          PresetStringHelper, false)
    .Bind("condition"_s, &cmLinearBuildPresetsGraph::Preset::ConditionEvaluator,
          PresetConditionHelper, false);
  return obj;
}

// Binds the fields shared by build, test, and package presets, which resolve
// against a configure preset.
template <typename T>
cmJSONHelperBuilder::Object<T> BindDependentPresetFields(
  cmJSONHelperBuilder::Object<T> obj)
{
  obj
    .Bind("environment"_s, &cmLinearBuildPresetsGraph::Preset::Environment,
          EnvironmentMapHelper, false)
    .Bind("configurePreset"_s, &T::ConfigurePreset, PresetStringHelper, false)
    .Bind("inheritConfigureEnvironment"_s, &T::InheritConfigureEnvironment,
          PresetOptionalBoolHelper, false);
  return obj;
}
}
