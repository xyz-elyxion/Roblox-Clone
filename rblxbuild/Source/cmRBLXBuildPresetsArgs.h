/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include <string>

#include <cm/optional>

#include "cmRBLXBuildPresetsGraph.h"

class cmRBLXBuildPresetsArgsBase
{
public:
  using PresetListMode = cmRBLXBuildPresetsGraph::PresetListMode;

  virtual ~cmRBLXBuildPresetsArgsBase() = default;

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
  cmRBLXBuildPresetsArgsBase() = default;
};

class cmRBLXBuildPresetsArgs : public cmRBLXBuildPresetsArgsBase
{
public:
  bool HasPresetsArg() const override
  {
    return this->cmRBLXBuildPresetsArgsBase::HasPresetsArg() ||
      this->ListPresetsMode.has_value();
  }

  void Clear() override
  {
    this->cmRBLXBuildPresetsArgsBase::Clear();
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

class cmRBLXBuildPresetsConfigureArgs : public cmRBLXBuildPresetsArgsBase
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
    return this->cmRBLXBuildPresetsArgsBase::HasPresetsArg() ||
      this->ListPresets != ListPresetsOption::None;
  }

  void Clear() override
  {
    this->cmRBLXBuildPresetsArgsBase::Clear();
    this->ListPresets = ListPresetsOption::None;
    this->ListPresetsMode = PresetListMode::Available;
  }

  ListPresetsOption ListPresets = ListPresetsOption::None;
  PresetListMode ListPresetsMode = PresetListMode::Available;
};

class cmRBLXBuildPresetsWorkflowArgs : public cmRBLXBuildPresetsArgs
{
public:
  void Clear() override
  {
    this->cmRBLXBuildPresetsArgs::Clear();
    this->Fresh = false;
  }

  bool Fresh = false;
};
