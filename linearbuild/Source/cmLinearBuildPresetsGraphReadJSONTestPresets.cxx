/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#include <functional>
#include <string>
#include <vector>

#include <cm/optional>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmLinearBuildPresetsErrors.h"
#include "cmLinearBuildPresetsGraph.h"
#include "cmLinearBuildPresetsGraphInternal.h"
#include "cmJSONHelpers.h"

#include "CTest/cmCTestTypes.h"

class cmJSONState;

namespace {
using TestPreset = cmLinearBuildPresetsGraph::TestPreset;
using JSONHelperBuilder = cmJSONHelperBuilder;

bool TestPresetOutputVerbosityHelper(
  TestPreset::OutputOptions::VerbosityEnum& out, Json::Value const* value,
  cmJSONState* state)
{
  if (!value) {
    out = TestPreset::OutputOptions::VerbosityEnum::Default;
    return true;
  }

  if (!value->isString()) {
    cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
    return false;
  }

  if (value->asString() == "default") {
    out = TestPreset::OutputOptions::VerbosityEnum::Default;
    return true;
  }

  if (value->asString() == "verbose") {
    out = TestPreset::OutputOptions::VerbosityEnum::Verbose;
    return true;
  }

  if (value->asString() == "extra") {
    out = TestPreset::OutputOptions::VerbosityEnum::Extra;
    return true;
  }

  cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const TestPresetOptionalOutputVerbosityHelper =
  JSONHelperBuilder::Optional<TestPreset::OutputOptions::VerbosityEnum>(
    TestPresetOutputVerbosityHelper);

bool TestPresetOutputTruncationHelper(
  cm::optional<cmCTestTypes::TruncationMode>& out, Json::Value const* value,
  cmJSONState* state)
{
  if (!value) {
    out = cm::nullopt;
    return true;
  }

  if (!value->isString()) {
    cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
    return false;
  }

  if (value->asString() == "tail") {
    out = cmCTestTypes::TruncationMode::Tail;
    return true;
  }

  if (value->asString() == "middle") {
    out = cmCTestTypes::TruncationMode::Middle;
    return true;
  }

  if (value->asString() == "head") {
    out = cmCTestTypes::TruncationMode::Head;
    return true;
  }

  cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const TestPresetOptionalOutputHelper =
  JSONHelperBuilder::Optional<TestPreset::OutputOptions>(
    JSONHelperBuilder::Object<TestPreset::OutputOptions>(
      JsonErrors::INVALID_OBJECT, false)
      .Bind("shortProgress"_s, &TestPreset::OutputOptions::ShortProgress,
            cmLinearBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("verbosity"_s, &TestPreset::OutputOptions::Verbosity,
            TestPresetOptionalOutputVerbosityHelper, false)
      .Bind("debug"_s, &TestPreset::OutputOptions::Debug,
            cmLinearBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("outputOnFailure"_s, &TestPreset::OutputOptions::OutputOnFailure,
            cmLinearBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("quiet"_s, &TestPreset::OutputOptions::Quiet,
            cmLinearBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("outputLogFile"_s, &TestPreset::OutputOptions::OutputLogFile,
            cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("outputJUnitFile"_s, &TestPreset::OutputOptions::OutputJUnitFile,
            cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("labelSummary"_s, &TestPreset::OutputOptions::LabelSummary,
            cmLinearBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("subprojectSummary"_s,
            &TestPreset::OutputOptions::SubprojectSummary,
            cmLinearBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("maxPassedTestOutputSize"_s,
            &TestPreset::OutputOptions::MaxPassedTestOutputSize,
            cmLinearBuildPresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("maxFailedTestOutputSize"_s,
            &TestPreset::OutputOptions::MaxFailedTestOutputSize,
            cmLinearBuildPresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("testOutputTruncation"_s,
            &TestPreset::OutputOptions::TestOutputTruncation,
            TestPresetOutputTruncationHelper, false)
      .Bind("maxTestNameWidth"_s, &TestPreset::OutputOptions::MaxTestNameWidth,
            cmLinearBuildPresetsGraphInternal::PresetOptionalIntHelper, false));

auto const TestPresetOptionalFilterIncludeIndexObjectHelper =
  JSONHelperBuilder::Optional<TestPreset::IncludeOptions::IndexOptions>(
    JSONHelperBuilder::Object<TestPreset::IncludeOptions::IndexOptions>()
      .Bind("start"_s, &TestPreset::IncludeOptions::IndexOptions::Start,
            cmLinearBuildPresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("end"_s, &TestPreset::IncludeOptions::IndexOptions::End,
            cmLinearBuildPresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("stride"_s, &TestPreset::IncludeOptions::IndexOptions::Stride,
            cmLinearBuildPresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("specificTests"_s,
            &TestPreset::IncludeOptions::IndexOptions::SpecificTests,
            cmLinearBuildPresetsGraphInternal::PresetVectorIntHelper, false));

bool TestPresetOptionalFilterIncludeIndexHelper(
  cm::optional<TestPreset::IncludeOptions::IndexOptions>& out,
  Json::Value const* value, cmJSONState* state)
{
  if (!value) {
    out = cm::nullopt;
    return true;
  }

  if (value->isString()) {
    out.emplace();
    out->IndexFile = value->asString();
    return true;
  }

  if (value->isObject()) {
    return TestPresetOptionalFilterIncludeIndexObjectHelper(out, value, state);
  }

  cmLinearBuildPresetsErrors::INVALID_TEST_FILTER_INCLUDE_INDEX(value, state);
  return false;
}

auto const TestPresetOptionalFilterIncludeHelper =
  JSONHelperBuilder::Optional<TestPreset::IncludeOptions>(
    JSONHelperBuilder::Object<TestPreset::IncludeOptions>()
      .Bind("name"_s, &TestPreset::IncludeOptions::Name,
            cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("label"_s, &TestPreset::IncludeOptions::Label,
            cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("index"_s, &TestPreset::IncludeOptions::Index,
            TestPresetOptionalFilterIncludeIndexHelper, false)
      .Bind("useUnion"_s, &TestPreset::IncludeOptions::UseUnion,
            cmLinearBuildPresetsGraphInternal::PresetOptionalBoolHelper, false));

auto const TestPresetOptionalFilterExcludeFixturesHelper =
  JSONHelperBuilder::Optional<TestPreset::ExcludeOptions::FixturesOptions>(
    JSONHelperBuilder::Object<TestPreset::ExcludeOptions::FixturesOptions>()
      .Bind("any"_s, &TestPreset::ExcludeOptions::FixturesOptions::Any,
            cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("setup"_s, &TestPreset::ExcludeOptions::FixturesOptions::Setup,
            cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("cleanup"_s, &TestPreset::ExcludeOptions::FixturesOptions::Cleanup,
            cmLinearBuildPresetsGraphInternal::PresetStringHelper, false));

auto const TestPresetOptionalFilterExcludeHelper =
  JSONHelperBuilder::Optional<TestPreset::ExcludeOptions>(
    JSONHelperBuilder::Object<TestPreset::ExcludeOptions>()
      .Bind("name"_s, &TestPreset::ExcludeOptions::Name,
            cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("label"_s, &TestPreset::ExcludeOptions::Label,
            cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("fixtures"_s, &TestPreset::ExcludeOptions::Fixtures,
            TestPresetOptionalFilterExcludeFixturesHelper, false));

bool TestPresetExecutionShowOnlyHelper(
  TestPreset::ExecutionOptions::ShowOnlyEnum& out, Json::Value const* value,
  cmJSONState* state)
{
  if (!value || !value->isString()) {
    cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
    return false;
  }

  if (value->asString() == "human") {
    out = TestPreset::ExecutionOptions::ShowOnlyEnum::Human;
    return true;
  }

  if (value->asString() == "json-v1") {
    out = TestPreset::ExecutionOptions::ShowOnlyEnum::JsonV1;
    return true;
  }

  cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const TestPresetOptionalExecutionShowOnlyHelper =
  JSONHelperBuilder::Optional<TestPreset::ExecutionOptions::ShowOnlyEnum>(
    TestPresetExecutionShowOnlyHelper);

bool TestPresetExecutionModeHelper(
  TestPreset::ExecutionOptions::RepeatOptions::ModeEnum& out,
  Json::Value const* value, cmJSONState* state)
{
  if (!value) {
    return true;
  }

  if (!value->isString()) {
    cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
    return false;
  }

  if (value->asString() == "until-fail") {
    out = TestPreset::ExecutionOptions::RepeatOptions::ModeEnum::UntilFail;
    return true;
  }

  if (value->asString() == "until-pass") {
    out = TestPreset::ExecutionOptions::RepeatOptions::ModeEnum::UntilPass;
    return true;
  }

  if (value->asString() == "after-timeout") {
    out = TestPreset::ExecutionOptions::RepeatOptions::ModeEnum::AfterTimeout;
    return true;
  }

  cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const TestPresetOptionalExecutionRepeatHelper =
  JSONHelperBuilder::Optional<TestPreset::ExecutionOptions::RepeatOptions>(
    JSONHelperBuilder::Object<TestPreset::ExecutionOptions::RepeatOptions>()
      .Bind("mode"_s, &TestPreset::ExecutionOptions::RepeatOptions::Mode,
            TestPresetExecutionModeHelper, true)
      .Bind("count"_s, &TestPreset::ExecutionOptions::RepeatOptions::Count,
            cmLinearBuildPresetsGraphInternal::PresetIntHelper, true));

bool TestPresetExecutionNoTestsActionHelper(
  TestPreset::ExecutionOptions::NoTestsActionEnum& out,
  Json::Value const* value, cmJSONState* state)
{
  if (!value) {
    out = TestPreset::ExecutionOptions::NoTestsActionEnum::Default;
    return true;
  }

  if (!value->isString()) {
    cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
    return false;
  }

  if (value->asString() == "default") {
    out = TestPreset::ExecutionOptions::NoTestsActionEnum::Default;
    return true;
  }

  if (value->asString() == "error") {
    out = TestPreset::ExecutionOptions::NoTestsActionEnum::Error;
    return true;
  }

  if (value->asString() == "ignore") {
    out = TestPreset::ExecutionOptions::NoTestsActionEnum::Ignore;
    return true;
  }

  cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const TestPresetOptionalExecutionNoTestsActionHelper =
  JSONHelperBuilder::Optional<TestPreset::ExecutionOptions::NoTestsActionEnum>(
    TestPresetExecutionNoTestsActionHelper);

bool TestPresetExecutionJobsHelper(cm::optional<unsigned int>& out,
                                   Json::Value const* value,
                                   cmJSONState* state)
{
  if (value->isString()) {
    if (!value->asString().empty()) {
      cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
      return false;
    }
    out.reset();
    return true;
  }

  if (value->isUInt()) {
    out.emplace(value->asUInt());
    return true;
  }

  cmLinearBuildPresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const TestPresetOptionalExecutionJobsHelper =
  JSONHelperBuilder::Optional<cm::optional<unsigned int>>(
    TestPresetExecutionJobsHelper);

auto const TestPresetExecutionHelper =
  JSONHelperBuilder::Optional<TestPreset::ExecutionOptions>(
    JSONHelperBuilder::Object<TestPreset::ExecutionOptions>()
      .Bind("stopOnFailure"_s, &TestPreset::ExecutionOptions::StopOnFailure,
            cmLinearBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("enableFailover"_s, &TestPreset::ExecutionOptions::EnableFailover,
            cmLinearBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("jobs"_s, &TestPreset::ExecutionOptions::Jobs,
            TestPresetOptionalExecutionJobsHelper, false)
      .Bind("resourceSpecFile"_s,
            &TestPreset::ExecutionOptions::ResourceSpecFile,
            cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("testLoad"_s, &TestPreset::ExecutionOptions::TestLoad,
            cmLinearBuildPresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("showOnly"_s, &TestPreset::ExecutionOptions::ShowOnly,
            TestPresetOptionalExecutionShowOnlyHelper, false)
      .Bind("repeat"_s, &TestPreset::ExecutionOptions::Repeat,
            TestPresetOptionalExecutionRepeatHelper, false)
      .Bind("interactiveDebugging"_s,
            &TestPreset::ExecutionOptions::InteractiveDebugging,
            cmLinearBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("scheduleRandom"_s, &TestPreset::ExecutionOptions::ScheduleRandom,
            cmLinearBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("timeout"_s, &TestPreset::ExecutionOptions::Timeout,
            cmLinearBuildPresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("noTestsAction"_s, &TestPreset::ExecutionOptions::NoTestsAction,
            TestPresetOptionalExecutionNoTestsActionHelper, false)
      .Bind("testPassthroughArguments"_s,
            &TestPreset::ExecutionOptions::TestPassthroughArguments,
            cmLinearBuildPresetsGraphInternal::PresetVectorStringHelper, false));

auto const TestPresetFilterHelper =
  JSONHelperBuilder::Optional<TestPreset::FilterOptions>(
    JSONHelperBuilder::Object<TestPreset::FilterOptions>()
      .Bind("include"_s, &TestPreset::FilterOptions::Include,
            TestPresetOptionalFilterIncludeHelper, false)
      .Bind("exclude"_s, &TestPreset::FilterOptions::Exclude,
            TestPresetOptionalFilterExcludeHelper, false));

auto const TestPresetHelper =
  cmLinearBuildPresetsGraphInternal::BindDependentPresetFields(
    cmLinearBuildPresetsGraphInternal::BindPresetIdentityFields(
      JSONHelperBuilder::Object<TestPreset>(
        cmLinearBuildPresetsErrors::INVALID_PRESET_OBJECT, false)))
    .Bind("configuration"_s, &TestPreset::Configuration,
          cmLinearBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("overwriteConfigurationFile"_s,
          &TestPreset::OverwriteConfigurationFile,
          cmLinearBuildPresetsGraphInternal::PresetVectorStringHelper, false)
    .Bind("output"_s, &TestPreset::Output, TestPresetOptionalOutputHelper,
          false)
    .Bind("filter"_s, &TestPreset::Filter, TestPresetFilterHelper, false)
    .Bind("execution"_s, &TestPreset::Execution, TestPresetExecutionHelper,
          false);
}

namespace cmLinearBuildPresetsGraphInternal {
bool TestPresetsHelper(std::vector<cmLinearBuildPresetsGraph::TestPreset>& out,
                       Json::Value const* value, cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Vector<TestPreset>(
    cmLinearBuildPresetsErrors::INVALID_PRESETS, TestPresetHelper);

  return helper(out, value, state);
}
}
