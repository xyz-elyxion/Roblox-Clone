/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include <string>

#include <cm/optional>

#include "cmLinearBuildPresetsGraph.h"

class cmLinearBuildPresetsArgsBase
{
public:
  using PresetListMode = cmLinearBuildPresetsGraph::PresetListMode;

  virtual ~cmLinearBuildPresetsArgsBase() = default;

  virtual bool HasPresetsArg() const { return !this->PresetName.empty(); };
  virtual void Clear() { this->PresetName.clear(); }

  static PresetListMode ParseListPresetsMode(std::string& type)
  {
    struct DefinedPresetType
    {
      char const* Type;
      char const* BaseType;
    };
    static constexpr DefinedPresetType definedPresetTypes[] = {
      { "defined", "" },
      { "configure-defined", "configure" },
      { "build-defined", "build" },
      { "test-defined", "test" },
      { "package-defined", "package" },
      { "workflow-defined", "workflow" },
      { "all-defined", "all" },
    };

    for (auto const& definedPresetType : definedPresetTypes) {
      if (type == definedPresetType.Type) {
        type = definedPresetType.BaseType;
        return PresetListMode::Defined;
      }
    }
    return PresetListMode::Available;
  }

  std::string PresetName;
  std::string PresetsFile;

protected:
  cmLinearBuildPresetsArgsBase() = default;
};

class cmLinearBuildPresetsArgs : public cmLinearBuildPresetsArgsBase
{
public:
  bool HasPresetsArg() const override
  {
    return this->cmLinearBuildPresetsArgsBase::HasPresetsArg() ||
      this->ListPresetsMode.has_value();
  }

  void Clear() override
  {
    this->cmLinearBuildPresetsArgsBase::Clear();
    this->ListPresetsMode.reset();
  }

  bool SetListPresets(std::string const& value)
  {
    std::string type = value;
    auto const mode = ParseListPresetsMode(type);
    if (!type.empty()) {
      this->Clear();
      return false;
    }
    this->ListPresetsMode = mode;
    return true;
  }

  cm::optional<PresetListMode> ListPresetsMode;
};

class cmLinearBuildPresetsConfigureArgs : public cmLinearBuildPresetsArgsBase
{
public:
  enum class ListPresetsOption
  {
    None,
    Configure,
    Build,
    Test,
    Package,
    Workflow,
    All,
  };

  bool HasPresetsArg() const override
  {
    return this->cmLinearBuildPresetsArgsBase::HasPresetsArg() ||
      this->ListPresets != ListPresetsOption::None;
  }

  void Clear() override
  {
    this->cmLinearBuildPresetsArgsBase::Clear();
    this->ListPresets = ListPresetsOption::None;
    this->ListPresetsMode = PresetListMode::Available;
  }

  ListPresetsOption ListPresets = ListPresetsOption::None;
  PresetListMode ListPresetsMode = PresetListMode::Available;
};

class cmLinearBuildPresetsWorkflowArgs : public cmLinearBuildPresetsArgs
{
public:
  void Clear() override
  {
    this->cmLinearBuildPresetsArgs::Clear();
    this->Fresh = false;
  }

  bool Fresh = false;
};
