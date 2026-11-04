/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include <functional>
#include <string>
#include <vector>

#include <cm/optional>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmRBLXBuildPresetsErrors.h"
#include "cmRBLXBuildPresetsGraph.h"
#include "cmRBLXBuildPresetsGraphInternal.h"
#include "cmJSONHelpers.h"

#include "CTest/cmCTestTypes.h"

class cmJSONState;

namespace {
using TestPreset = cmRBLXBuildPresetsGraph::TestPreset;
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
    cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
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

  cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
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
    cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
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

  cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const TestPresetOptionalOutputHelper =
  JSONHelperBuilder::Optional<TestPreset::OutputOptions>(
    JSONHelperBuilder::Object<TestPreset::OutputOptions>(
      JsonErrors::INVALID_OBJECT, false)
      .Bind("shortProgress"_s, &TestPreset::OutputOptions::ShortProgress,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("verbosity"_s, &TestPreset::OutputOptions::Verbosity,
            TestPresetOptionalOutputVerbosityHelper, false)
      .Bind("debug"_s, &TestPreset::OutputOptions::Debug,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("outputOnFailure"_s, &TestPreset::OutputOptions::OutputOnFailure,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("quiet"_s, &TestPreset::OutputOptions::Quiet,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("outputLogFile"_s, &TestPreset::OutputOptions::OutputLogFile,
            cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("outputJUnitFile"_s, &TestPreset::OutputOptions::OutputJUnitFile,
            cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("labelSummary"_s, &TestPreset::OutputOptions::LabelSummary,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("subprojectSummary"_s,
            &TestPreset::OutputOptions::SubprojectSummary,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("maxPassedTestOutputSize"_s,
            &TestPreset::OutputOptions::MaxPassedTestOutputSize,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("maxFailedTestOutputSize"_s,
            &TestPreset::OutputOptions::MaxFailedTestOutputSize,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("testOutputTruncation"_s,
            &TestPreset::OutputOptions::TestOutputTruncation,
            TestPresetOutputTruncationHelper, false)
      .Bind("maxTestNameWidth"_s, &TestPreset::OutputOptions::MaxTestNameWidth,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalIntHelper, false));

auto const TestPresetOptionalFilterIncludeIndexObjectHelper =
  JSONHelperBuilder::Optional<TestPreset::IncludeOptions::IndexOptions>(
    JSONHelperBuilder::Object<TestPreset::IncludeOptions::IndexOptions>()
      .Bind("start"_s, &TestPreset::IncludeOptions::IndexOptions::Start,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("end"_s, &TestPreset::IncludeOptions::IndexOptions::End,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("stride"_s, &TestPreset::IncludeOptions::IndexOptions::Stride,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("specificTests"_s,
            &TestPreset::IncludeOptions::IndexOptions::SpecificTests,
            cmRBLXBuildPresetsGraphInternal::PresetVectorIntHelper, false));

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

  cmRBLXBuildPresetsErrors::INVALID_TEST_FILTER_INCLUDE_INDEX(value, state);
  return false;
}

auto const TestPresetOptionalFilterIncludeHelper =
  JSONHelperBuilder::Optional<TestPreset::IncludeOptions>(
    JSONHelperBuilder::Object<TestPreset::IncludeOptions>()
      .Bind("name"_s, &TestPreset::IncludeOptions::Name,
            cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("label"_s, &TestPreset::IncludeOptions::Label,
            cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("index"_s, &TestPreset::IncludeOptions::Index,
            TestPresetOptionalFilterIncludeIndexHelper, false)
      .Bind("useUnion"_s, &TestPreset::IncludeOptions::UseUnion,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false));

auto const TestPresetOptionalFilterExcludeFixturesHelper =
  JSONHelperBuilder::Optional<TestPreset::ExcludeOptions::FixturesOptions>(
    JSONHelperBuilder::Object<TestPreset::ExcludeOptions::FixturesOptions>()
      .Bind("any"_s, &TestPreset::ExcludeOptions::FixturesOptions::Any,
            cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("setup"_s, &TestPreset::ExcludeOptions::FixturesOptions::Setup,
            cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("cleanup"_s, &TestPreset::ExcludeOptions::FixturesOptions::Cleanup,
            cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false));

auto const TestPresetOptionalFilterExcludeHelper =
  JSONHelperBuilder::Optional<TestPreset::ExcludeOptions>(
    JSONHelperBuilder::Object<TestPreset::ExcludeOptions>()
      .Bind("name"_s, &TestPreset::ExcludeOptions::Name,
            cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("label"_s, &TestPreset::ExcludeOptions::Label,
            cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("fixtures"_s, &TestPreset::ExcludeOptions::Fixtures,
            TestPresetOptionalFilterExcludeFixturesHelper, false));

bool TestPresetExecutionShowOnlyHelper(
  TestPreset::ExecutionOptions::ShowOnlyEnum& out, Json::Value const* value,
  cmJSONState* state)
{
  if (!value || !value->isString()) {
    cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
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

  cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
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
    cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
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

  cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const TestPresetOptionalExecutionRepeatHelper =
  JSONHelperBuilder::Optional<TestPreset::ExecutionOptions::RepeatOptions>(
    JSONHelperBuilder::Object<TestPreset::ExecutionOptions::RepeatOptions>()
      .Bind("mode"_s, &TestPreset::ExecutionOptions::RepeatOptions::Mode,
            TestPresetExecutionModeHelper, true)
      .Bind("count"_s, &TestPreset::ExecutionOptions::RepeatOptions::Count,
            cmRBLXBuildPresetsGraphInternal::PresetIntHelper, true));

bool TestPresetExecutionNoTestsActionHelper(
  TestPreset::ExecutionOptions::NoTestsActionEnum& out,
  Json::Value const* value, cmJSONState* state)
{
  if (!value) {
    out = TestPreset::ExecutionOptions::NoTestsActionEnum::Default;
    return true;
  }

  if (!value->isString()) {
    cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
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

  cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
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
      cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
      return false;
    }
    out.reset();
    return true;
  }

  if (value->isUInt()) {
    out.emplace(value->asUInt());
    return true;
  }

  cmRBLXBuildPresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const TestPresetOptionalExecutionJobsHelper =
  JSONHelperBuilder::Optional<cm::optional<unsigned int>>(
    TestPresetExecutionJobsHelper);

auto const TestPresetExecutionHelper =
  JSONHelperBuilder::Optional<TestPreset::ExecutionOptions>(
    JSONHelperBuilder::Object<TestPreset::ExecutionOptions>()
      .Bind("stopOnFailure"_s, &TestPreset::ExecutionOptions::StopOnFailure,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("enableFailover"_s, &TestPreset::ExecutionOptions::EnableFailover,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("jobs"_s, &TestPreset::ExecutionOptions::Jobs,
            TestPresetOptionalExecutionJobsHelper, false)
      .Bind("resourceSpecFile"_s,
            &TestPreset::ExecutionOptions::ResourceSpecFile,
            cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
      .Bind("testLoad"_s, &TestPreset::ExecutionOptions::TestLoad,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("showOnly"_s, &TestPreset::ExecutionOptions::ShowOnly,
            TestPresetOptionalExecutionShowOnlyHelper, false)
      .Bind("repeat"_s, &TestPreset::ExecutionOptions::Repeat,
            TestPresetOptionalExecutionRepeatHelper, false)
      .Bind("interactiveDebugging"_s,
            &TestPreset::ExecutionOptions::InteractiveDebugging,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("scheduleRandom"_s, &TestPreset::ExecutionOptions::ScheduleRandom,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("timeout"_s, &TestPreset::ExecutionOptions::Timeout,
            cmRBLXBuildPresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("noTestsAction"_s, &TestPreset::ExecutionOptions::NoTestsAction,
            TestPresetOptionalExecutionNoTestsActionHelper, false)
      .Bind("testPassthroughArguments"_s,
            &TestPreset::ExecutionOptions::TestPassthroughArguments,
            cmRBLXBuildPresetsGraphInternal::PresetVectorStringHelper, false));

auto const TestPresetFilterHelper =
  JSONHelperBuilder::Optional<TestPreset::FilterOptions>(
    JSONHelperBuilder::Object<TestPreset::FilterOptions>()
      .Bind("include"_s, &TestPreset::FilterOptions::Include,
            TestPresetOptionalFilterIncludeHelper, false)
      .Bind("exclude"_s, &TestPreset::FilterOptions::Exclude,
            TestPresetOptionalFilterExcludeHelper, false));

auto const TestPresetHelper =
  cmRBLXBuildPresetsGraphInternal::BindDependentPresetFields(
    cmRBLXBuildPresetsGraphInternal::BindPresetIdentityFields(
      JSONHelperBuilder::Object<TestPreset>(
        cmRBLXBuildPresetsErrors::INVALID_PRESET_OBJECT, false)))
    .Bind("configuration"_s, &TestPreset::Configuration,
          cmRBLXBuildPresetsGraphInternal::PresetStringHelper, false)
    .Bind("overwriteConfigurationFile"_s,
          &TestPreset::OverwriteConfigurationFile,
          cmRBLXBuildPresetsGraphInternal::PresetVectorStringHelper, false)
    .Bind("output"_s, &TestPreset::Output, TestPresetOptionalOutputHelper,
          false)
    .Bind("filter"_s, &TestPreset::Filter, TestPresetFilterHelper, false)
    .Bind("execution"_s, &TestPreset::Execution, TestPresetExecutionHelper,
          false);
}

namespace cmRBLXBuildPresetsGraphInternal {
bool TestPresetsHelper(std::vector<cmRBLXBuildPresetsGraph::TestPreset>& out,
                       Json::Value const* value, cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Vector<TestPreset>(
    cmRBLXBuildPresetsErrors::INVALID_PRESETS, TestPresetHelper);

  return helper(out, value, state);
}
}
