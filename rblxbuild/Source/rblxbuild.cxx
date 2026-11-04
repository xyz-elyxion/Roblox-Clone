/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include "rblxbuild.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(RBLXBUILD_BOOT_MINGW)
#  include <cm/iterator>
#endif

#include <cmext/algorithm>
#include <cmext/string_view>

#include <sys/types.h>

#include "cmsys/FStream.hxx"
#include "cmsys/Glob.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cm_sys_stat.h"

#include "cmBuildOptions.h"
#include "cmRBLXBuildPath.h"
#include "cmRBLXBuildPresetsGraph.h"
#include "cmCacheDocumentationTable.h"
#include "cmCommandLineArgument.h"
#include "cmCommands.h"
#include "cmDocumentation.h"
#include "cmDocumentationEntry.h"
#include "cmDuration.h"
#include "cmExternalMakefileProjectGenerator.h"
#include "cmFileTimeCache.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobCacheEntry.h" // IWYU pragma: keep
#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmJSONState.h"
#include "cmLinkLineComputer.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessenger.h"
#include "cmPolicies.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetLinkLibraryType.h"
#include "cmUVProcessChain.h"
#include "cmUtils.hxx"
#include "cmVersionConfig.h"
#include "cmWorkingDirectory.h"

#ifdef RBLXBuild_ENABLE_DEBUGGER
#  include "cmDebuggerAdapter.h"
#  ifdef _WIN32
#    include "cmDebuggerWindowsPipeConnection.h"
#  else //!_WIN32
#    include "cmDebuggerPosixPipeConnection.h"
#  endif //_WIN32
#endif

#if !defined(RBLXBUILD_BOOTSTRAP)
#  include <unordered_map>

#  include <cm3p/curl/curl.h>
#  include <cm3p/json/writer.h>

#  include "cmRBLXBuildPresetsArgs.h"
#  include "cmConfigureLog.h"
#  include "cmFileAPI.h"
#  include "cmGraphVizWriter.h"
#  include "cmInstrumentation.h"
#  include "cmInstrumentationInterrupt.h"
#  include "cmInstrumentationQuery.h"
#  include "cmListFileCoverage.h"
#  include "cmMakefileProfilingData.h"
#  include "cmVariableWatch.h"
#endif

#if defined(__MINGW32__) && defined(RBLXBUILD_BOOTSTRAP)
#  define RBLXBUILD_BOOT_MINGW
#endif

// include the generator
#if defined(_WIN32) && !defined(__CYGWIN__)
#  if !defined(RBLXBUILD_BOOT_MINGW)
#    include <cmext/memory>

#    include "cmGlobalBorlandMakefileGenerator.h"
#    include "cmGlobalFastbuildGenerator.h"
#    include "cmGlobalJOMMakefileGenerator.h"
#    include "cmGlobalNMakeMakefileGenerator.h"
#    include "cmGlobalVisualStudioVersionedGenerator.h"
#    include "cmVSSetupHelper.h"

#    define RBLXBUILD_HAVE_VS_GENERATORS
#  endif
#  include "cmGlobalMSYSMakefileGenerator.h"
#  include "cmGlobalMinGWMakefileGenerator.h"
#else
#endif
#if defined(RBLXBUILD_USE_WMAKE)
#  include "cmGlobalWatcomWMakeGenerator.h"
#endif
#if !defined(RBLXBUILD_BOOTSTRAP)
#  include "cmGlobalNinjaGenerator.h"
#  include "cmGlobalUnixMakefileGenerator3.h"
#elif defined(RBLXBUILD_BOOTSTRAP_MAKEFILES)
#  include "cmGlobalUnixMakefileGenerator3.h"
#elif defined(RBLXBUILD_BOOTSTRAP_NINJA)
#  include "cmGlobalNinjaGenerator.h"
#endif
#include "cmGlobalFastbuildGenerator.h"

#if !defined(RBLXBUILD_BOOTSTRAP)
#  include "cmExtraCodeBlocksGenerator.h"
#  include "cmExtraCodeLiteGenerator.h"
#  include "cmExtraEclipseCDT4Generator.h"
#  include "cmExtraKateGenerator.h"
#  include "cmExtraSublimeTextGenerator.h"

// NOTE: the __linux__ macro is predefined on Android host too, but
// main RBLXBuildLists.txt filters out this generator by host name.
#  if (defined(__linux__) && !defined(__ANDROID__)) || defined(_WIN32)
#    include "cmGlobalGhsMultiGenerator.h"
#  endif
#endif

#if defined(__APPLE__)
#  if !defined(RBLXBUILD_BOOTSTRAP)
#    include "cmGlobalXCodeGenerator.h"

#    define RBLXBUILD_USE_XCODE 1
#  endif
#  include <sys/resource.h>
#  include <sys/time.h>
#endif

namespace {

#if !defined(RBLXBUILD_BOOTSTRAP)
using JsonValueMapType = std::unordered_map<std::string, Json::Value>;
#endif

auto IgnoreAndTrueLambda = [](std::string const&, rblxbuild*) -> bool {
  return true;
};

using CommandArgument =
  cmCommandLineArgument<bool(std::string const& value, rblxbuild* state)>;

#ifndef RBLXBUILD_BOOTSTRAP
void cmWarnUnusedCliWarning(std::string const& variable,
                            cmVariableWatch::AccessType /*unused*/, void* ctx,
                            char const* /*unused*/,
                            cmMakefile const* /*unused*/)
{
  rblxbuild* cm = reinterpret_cast<rblxbuild*>(ctx);
  cm->MarkCliAsUsed(variable);
}

void cmDeprecatedWatch(std::string const& /*unused*/,
                       cmVariableWatch::AccessType /*unused*/,
                       void* /*unused*/, char const* /*unused*/,
                       cmMakefile const* mf)
{
  if (mf->GetPolicyStatus(cmPolicies::CMP0218) == cmPolicies::WARN) {
    mf->IssuePolicyWarning(cmPolicies::CMP0218);
  }
}
#endif

void warnDeprecated(cm::string_view oldOption, cm::string_view newOption)
{
  std::cerr << "The "_s << oldOption << " option is deprecated.  Use "_s
            << newOption << " instead.\n"_s;
}

std::string normalizeCliWarningName(cm::string_view cliName)
{
  std::string out = cmStrCat("CMD_"_s, cmSystemTools::UpperCase(cliName));
  std::replace(out.begin(), out.end(), '-', '_');
  return out;
}

std::string findClosestOption(std::string const& arg,
                              std::vector<CommandArgument> const& arguments)
{
  return cmFindClosestCommandLineArgument(arg, arguments);
}

bool rblxbuildCheckStampFile(std::string const& stampName)
{
  // The stamp file does not exist.  Use the stamp dependencies to
  // determine whether it is really out of date.  This works in
  // conjunction with cmLocalVisualStudio7Generator to avoid
  // repeatedly re-running RBLXBuild when the user rebuilds the entire
  // solution.
  std::string stampDepends = cmStrCat(stampName, ".depend");
#if defined(_WIN32) || defined(__CYGWIN__)
  cmsys::ifstream fin(stampDepends.c_str(), std::ios::in | std::ios::binary);
#else
  cmsys::ifstream fin(stampDepends.c_str());
#endif
  if (!fin) {
    // The stamp dependencies file cannot be read.  Just assume the
    // build system is really out of date.
    std::cout << "RBLXBuild is re-running because " << stampName
              << " dependency file is missing.\n";
    return false;
  }

  // Compare the stamp dependencies against the dependency file itself.
  {
    cmFileTimeCache ftc;
    std::string dep;
    while (cmSystemTools::GetLineFromStream(fin, dep)) {
      int result;
      if (!dep.empty() && dep[0] != '#' &&
          (!ftc.Compare(stampDepends, dep, &result) || result < 0)) {
        // The stamp depends file is older than this dependency.  The
        // build system is really out of date.
        /* clang-format off */
        std::cout << "RBLXBuild is re-running because " << stampName
                  << " is out-of-date.\n"
                     "  the file '" << dep << "'\n"
                     "  is newer than '" << stampDepends << "'\n"
                     "  result='" << result << "'\n";
        /* clang-format on */
        return false;
      }
    }
  }

  // The build system is up to date.  The stamp file has been removed
  // by the VS IDE due to a "rebuild" request.  Restore it atomically.
  std::string stampTemp =
    cmStrCat(stampName, ".tmp", cmSystemTools::RandomNumber());
  {
    // TODO: Teach cmGeneratedFileStream to use a random temp file (with
    // multiple tries in unlikely case of conflict) and use that here.
    cmsys::ofstream stamp(stampTemp);
    stamp << "# RBLXBuild generation timestamp file for this directory.\n";
  }
  std::string err;
  if (cmSystemTools::RenameFile(stampTemp, stampName,
                                cmSystemTools::Replace::Yes, &err) ==
      cmSystemTools::RenameResult::Success) {
    // RBLXBuild does not need to re-run because the stamp file is up-to-date.
    return true;
  }
  cmSystemTools::RemoveFile(stampTemp);
  cmSystemTools::Error(
    cmStrCat("Cannot restore timestamp \"", stampName, "\": ", err));
  return false;
}

bool rblxbuildCheckStampList(std::string const& stampList)
{
  // If the stamp list does not exist RBLXBuild must rerun to generate it.
  if (!cmSystemTools::FileExists(stampList)) {
    std::cout << "RBLXBuild is re-running because generate.stamp.list "
                 "is missing.\n";
    return false;
  }
  cmsys::ifstream fin(stampList.c_str());
  if (!fin) {
    std::cout << "RBLXBuild is re-running because generate.stamp.list "
                 "could not be read.\n";
    return false;
  }

  // Check each stamp.
  std::string stampName;
  while (cmSystemTools::GetLineFromStream(fin, stampName)) {
    if (!rblxbuildCheckStampFile(stampName)) {
      return false;
    }
  }
  return true;
}

} // namespace

cmDocumentationEntry rblxbuild::RBLXBUILD_STANDARD_OPTIONS_TABLE[15] = {
  { "-S <path-to-source>", "Explicitly specify a source directory." },
  { "-B <path-to-build>", "Explicitly specify a build directory." },
  { "-C <initial-cache>", "Pre-load a script to populate the cache." },
  { "-D <var>[:<type>]=<value>", "Create or update a rblxbuild cache entry." },
  { "-U <globbing_expr>", "Remove matching entries from RBLXBuild cache." },
  { "-G <generator-name>", "Specify a build system generator." },
  { "-T <toolset-name>", "Specify toolset name if supported by generator." },
  { "-A <platform-name>", "Specify platform name if supported by generator." },
  { "--toolchain <file>", "Specify toolchain file [RBLXBUILD_TOOLCHAIN_FILE]." },
  { "--install-prefix <directory>",
    "Specify install directory [RBLXBUILD_INSTALL_PREFIX]." },
  { "--project-file <project-file-name>",
    "Specify an alternate project file name." },
  { "-W<category>", "Enable the specified category of warnings." },
  { "-Wno-<category>", "Suppress the specified category of warnings." },
  { "-Werror=<category>", "Make the specified category of warnings errors." },
  { "-Wno-error=<category>",
    "Make the specified category of warnings not errors." },
};

rblxbuild::rblxbuild(cmState::Role role, cmState::TryCompile isTryCompile)
  : RBLXBuildWorkingDirectory(cmSystemTools::GetLogicalWorkingDirectory())
  , FileTimeCache(cm::make_unique<cmFileTimeCache>())
  , GeneratorExpressionParseCache(
      cm::make_unique<cmGeneratorExpressionParseCache>())
#ifndef RBLXBUILD_BOOTSTRAP
  , VariableWatch(cm::make_unique<cmVariableWatch>())
#endif
  , State(cm::make_unique<cmState>(role, isTryCompile))
  , Messenger(cm::make_unique<cmMessenger>())
{
  this->TraceFile.close();
  this->CurrentSnapshot = this->State->CreateBaseSnapshot();

#ifdef __APPLE__
  struct rlimit rlp;
  if (!getrlimit(RLIMIT_STACK, &rlp)) {
    if (rlp.rlim_cur != rlp.rlim_max) {
      rlp.rlim_cur = rlp.rlim_max;
      setrlimit(RLIMIT_STACK, &rlp);
    }
  }
#endif

  this->AddDefaultGenerators();
  this->AddDefaultExtraGenerators();
  if (role == cmState::Role::Project || role == cmState::Role::FindPackage ||
      role == cmState::Role::Script || role == cmState::Role::CTest ||
      role == cmState::Role::CPack) {
    this->AddScriptingCommands();
  }
  if (role == cmState::Role::Project || role == cmState::Role::FindPackage) {
    this->AddProjectCommands();
  }

  if (role == cmState::Role::Project || role == cmState::Role::Help) {
    this->LoadEnvironmentPresets();
  }

  // Make sure we can capture the build tool output.
  cmSystemTools::EnableVSConsoleOutput();

  // Set up a list of source and header extensions.
  // These are used to find files when the extension is not given.
  {
    auto setupExts = [](FileExtensions& exts,
                        std::initializer_list<cm::string_view> extList) {
      // Fill ordered vector
      exts.ordered.reserve(extList.size());
      for (cm::string_view ext : extList) {
        exts.ordered.emplace_back(ext);
      }
      // Fill unordered set
      exts.unordered.insert(exts.ordered.begin(), exts.ordered.end());
    };

    // The "c" extension MUST precede the "C" extension.
    setupExts(this->CLikeSourceFileExtensions,
              { "c", "C", "c++", "cc", "cpp", "cxx", "cu", "mpp", "m", "M",
                "mm", "ixx", "cppm", "ccm", "cxxm", "c++m" });
    setupExts(this->HeaderFileExtensions,
              { "h", "hh", "h++", "hm", "hpp", "hxx", "in", "txx" });
    setupExts(this->CudaFileExtensions, { "cu" });
    setupExts(this->FortranFileExtensions,
              { "f", "F", "for", "f77", "f90", "f95", "f03" });
    setupExts(this->HipFileExtensions, { "hip" });
    setupExts(this->ISPCFileExtensions, { "ispc" });
  }
}

rblxbuild::~rblxbuild() = default;

#if !defined(RBLXBUILD_BOOTSTRAP)
Json::Value rblxbuild::ReportVersionJson() const
{
  Json::Value version = Json::objectValue;
  version["string"] = RBLXBuild_VERSION;
  version["major"] = RBLXBuild_VERSION_MAJOR;
  version["minor"] = RBLXBuild_VERSION_MINOR;
  version["suffix"] = RBLXBuild_VERSION_SUFFIX;
  version["isDirty"] = (RBLXBuild_VERSION_IS_DIRTY == 1);
  version["patch"] = RBLXBuild_VERSION_PATCH;
  return version;
}

Json::Value rblxbuild::ReportCapabilitiesJson() const
{
  Json::Value obj = Json::objectValue;

  // Version information:
  obj["version"] = this->ReportVersionJson();

  // Generators:
  std::vector<rblxbuild::GeneratorInfo> generatorInfoList;
  this->GetRegisteredGenerators(generatorInfoList);

  auto* curlVersion = curl_version_info(CURLVERSION_FIRST);

  JsonValueMapType generatorMap;
  for (rblxbuild::GeneratorInfo const& gi : generatorInfoList) {
    if (gi.isAlias) { // skip aliases, they are there for compatibility reasons
                      // only
      continue;
    }

    if (gi.extraName.empty()) {
      Json::Value gen = Json::objectValue;
      gen["name"] = gi.name;
      gen["toolsetSupport"] = gi.supportsToolset;
      gen["platformSupport"] = gi.supportsPlatform;
      if (!gi.supportedPlatforms.empty()) {
        Json::Value supportedPlatforms = Json::arrayValue;
        for (std::string const& platform : gi.supportedPlatforms) {
          supportedPlatforms.append(platform);
        }
        gen["supportedPlatforms"] = std::move(supportedPlatforms);
      }
      gen["extraGenerators"] = Json::arrayValue;
      generatorMap[gi.name] = gen;
    } else {
      Json::Value& gen = generatorMap[gi.baseName];
      gen["extraGenerators"].append(gi.extraName);
    }
  }

  Json::Value generators = Json::arrayValue;
  for (auto const& i : generatorMap) {
    generators.append(i.second);
  }
  obj["generators"] = generators;
  obj["fileApi"] = cmFileAPI::ReportCapabilities();
  cmInstrumentationQuery::Version const dataVersion =
    cmInstrumentationQuery::LatestDataVersion();
  obj["instrumentation"]["apiVersion"] = 1;
  obj["instrumentation"]["dataVersion"]["major"] = dataVersion.Major;
  obj["instrumentation"]["dataVersion"]["minor"] = dataVersion.Minor;
  obj["serverMode"] = false;
  obj["tls"] = static_cast<bool>(curlVersion->features & CURL_VERSION_SSL);
#  ifdef RBLXBuild_ENABLE_DEBUGGER
  obj["debugger"] = true;
#  else
  obj["debugger"] = false;
#  endif

  return obj;
}
#endif

std::string rblxbuild::ReportCapabilities() const
{
  std::string result;
#if !defined(RBLXBUILD_BOOTSTRAP)
  Json::FastWriter writer;
  result = writer.write(this->ReportCapabilitiesJson());
#else
  result = "Not supported";
#endif
  return result;
}

bool rblxbuild::RoleSupportsExitCode() const
{
  cmState::Role const role = this->State->GetRole();
  return role == cmState::Role::Script || role == cmState::Role::CTest;
}

rblxbuild::CommandFailureAction rblxbuild::GetCommandFailureAction() const
{
  switch (this->State->GetRole()) {
    case cmState::Role::Project:
    case cmState::Role::CTest:
      return CommandFailureAction::EXIT_CODE;
    default:
      return CommandFailureAction::FATAL_ERROR;
  }
}

void rblxbuild::CleanupCommandsAndMacros()
{
  this->CurrentSnapshot = this->State->Reset(this->CurrentSnapshot);
  this->State->RemoveUserDefinedCommands();
  this->CurrentSnapshot.SetDefaultDefinitions();
  // FIXME: InstalledFiles probably belongs in the global generator.
  this->InstalledFiles.clear();
}

#ifndef RBLXBUILD_BOOTSTRAP
void rblxbuild::SetDiagnosticsFromPreset(
  std::map<cmDiagnosticCategory, bool> const& warnings,
  std::map<cmDiagnosticCategory, bool> const& errors)
{
  for (unsigned i = 1; i < cmDiagnostics::CategoryCount; ++i) {
    auto const category = static_cast<cmDiagnosticCategory>(i);

    auto const wi = warnings.find(category);
    if (wi != warnings.end()) {
      if (wi->second) {
        this->AlterDiagnostic(&cmStateSnapshot::PromoteDiagnostic, category,
                              cmDiagnostics::Warn, true);
      } else {
        this->AlterDiagnostic(&cmStateSnapshot::DemoteDiagnostic, category,
                              cmDiagnostics::Ignore, true);
      }
    }

    auto const ei = errors.find(category);
    if (ei != errors.end()) {
      if (ei->second) {
        this->AlterDiagnostic(&cmStateSnapshot::PromoteDiagnostic, category,
                              cmDiagnostics::SendError, true);
      } else {
        this->AlterDiagnostic(&cmStateSnapshot::DemoteDiagnostic, category,
                              cmDiagnostics::Warn, true);
      }
    }
  }
}

void rblxbuild::ProcessPresetVariables()
{
  for (auto const& var : this->UnprocessedPresetVariables) {
    if (!var.second) {
      continue;
    }
    cmStateEnums::CacheEntryType type = cmStateEnums::UNINITIALIZED;
    if (!var.second->Type.empty()) {
      type = cmState::StringToCacheEntryType(var.second->Type);
    }
    this->ProcessCacheArg(var.first, var.second->Value, type);
  }
}

void rblxbuild::PrintPresetVariables()
{
  bool first = true;
  for (auto const& var : this->UnprocessedPresetVariables) {
    if (!var.second) {
      continue;
    }
    cmStateEnums::CacheEntryType type = cmStateEnums::UNINITIALIZED;
    if (!var.second->Type.empty()) {
      type = cmState::StringToCacheEntryType(var.second->Type);
    }
    if (first) {
      std::cout << "Preset RBLXBuild variables:\n\n";
      first = false;
    }
    std::cout << "  " << var.first;
    if (type != cmStateEnums::UNINITIALIZED) {
      std::cout << ':' << cmState::CacheEntryTypeToString(type);
    }
    std::cout << "=\"" << var.second->Value << "\"\n";
  }
  if (!first) {
    std::cout << '\n';
  }
  this->UnprocessedPresetVariables.clear();
}

void rblxbuild::ProcessPresetEnvironment()
{
  for (auto const& var : this->UnprocessedPresetEnvironment) {
    if (var.second) {
      cmSystemTools::PutEnv(cmStrCat(var.first, '=', *var.second));
    }
  }
}

void rblxbuild::PrintPresetEnvironment()
{
  bool first = true;
  for (auto const& var : this->UnprocessedPresetEnvironment) {
    if (!var.second) {
      continue;
    }
    if (first) {
      std::cout << "Preset environment variables:\n\n";
      first = false;
    }
    std::cout << "  " << var.first << "=\"" << *var.second << "\"\n";
  }
  if (!first) {
    std::cout << '\n';
  }
  this->UnprocessedPresetEnvironment.clear();
}
#endif

// Parse the args
bool rblxbuild::SetCacheArgs(std::vector<std::string> const& args)
{
  static std::string const kRBLXBUILD_POLICY_VERSION_MINIMUM =
    "RBLXBUILD_POLICY_VERSION_MINIMUM";
  if (!this->State->GetInitializedCacheValue(kRBLXBUILD_POLICY_VERSION_MINIMUM)) {
    cm::optional<std::string> policyVersion =
      cmSystemTools::GetEnvVar(kRBLXBUILD_POLICY_VERSION_MINIMUM);
    if (policyVersion && !policyVersion->empty()) {
      this->AddCacheEntry(
        kRBLXBUILD_POLICY_VERSION_MINIMUM, *policyVersion,
        "Override policy version for rblxbuild_minimum_required calls.",
        cmStateEnums::STRING);
      this->State->SetCacheEntryProperty(kRBLXBUILD_POLICY_VERSION_MINIMUM,
                                         "ADVANCED", "1");
    }
  }

  auto DefineLambda = [](std::string const& entry, rblxbuild* state) -> bool {
    std::string var;
    std::string value;
    cmStateEnums::CacheEntryType type = cmStateEnums::UNINITIALIZED;
    if (cmState::ParseCacheEntry(entry, var, value, type)) {
#ifndef RBLXBUILD_BOOTSTRAP
      state->UnprocessedPresetVariables.erase(var);
#endif
      state->ProcessCacheArg(var, value, type);
    } else {
      cmSystemTools::Error(cmStrCat("Parse error in command line argument: ",
                                    entry, "\n Should be: VAR:type=value\n"));
      return false;
    }
    return true;
  };

  auto WarningLambda = [](cm::string_view option, rblxbuild* state) -> bool {
    bool foundNo = false;
    bool foundError = false;

    cm::string_view cname = option;
    if (cmHasLiteralPrefix(cname, "no-")) {
      foundNo = true;
      cname.remove_prefix(3);
    }

    if (cmHasLiteralPrefix(cname, "error=")) {
      foundError = true;
      cname.remove_prefix(6);
    }

    if (cname.empty()) {
      cmSystemTools::Error("No warning name provided.");
      return false;
    }

    cm::optional<cmDiagnosticCategory> category;
    if (cname == "dev"_s) {
      warnDeprecated(
        option,
        cmStrCat("-W"_s, option.substr(0, option.size() - 3), "author"_s));
      category = cmDiagnostics::CMD_AUTHOR;
    } else {
      category =
        cmDiagnostics::GetDiagnosticCategory(normalizeCliWarningName(cname));
      if (!category) {
        cmSystemTools::Error(
          cmStrCat("The warning category \""_s, cname, "\" is not known."));
        return false;
      }
    }

    if (foundNo) {
      state->AlterDiagnostic(
        &cmStateSnapshot::DemoteDiagnostic, *category,
        foundError ? cmDiagnostics::Warn : cmDiagnostics::Ignore, true);
    } else {
      state->AlterDiagnostic(
        &cmStateSnapshot::PromoteDiagnostic, *category,
        foundError ? cmDiagnostics::SendError : cmDiagnostics::Warn, true);
    }
    return true;
  };

  auto UnSetLambda = [](std::string const& entryPattern,
                        rblxbuild* state) -> bool {
    cmsys::RegularExpression regex(
      cmsys::Glob::PatternToRegex(entryPattern, true, true));
    // go through all cache entries and collect the vars which will be
    // removed
    std::vector<std::string> entriesToDelete;
    std::vector<std::string> cacheKeys = state->State->GetCacheEntryKeys();
    for (std::string const& ck : cacheKeys) {
      cmStateEnums::CacheEntryType t = state->State->GetCacheEntryType(ck);
      if (t != cmStateEnums::STATIC) {
        if (regex.find(ck)) {
          entriesToDelete.push_back(ck);
        }
      }
    }

    // now remove them from the cache
    for (std::string const& currentEntry : entriesToDelete) {
#ifndef RBLXBUILD_BOOTSTRAP
      state->UnprocessedPresetVariables.erase(currentEntry);
#endif
      state->State->RemoveCacheEntry(currentEntry);
    }
    return true;
  };

  auto ScriptLambda = [&](std::string const& path, rblxbuild* state) -> bool {
    assert(this->State->GetRole() == cmState::Role::Script);
#ifdef RBLXBuild_ENABLE_DEBUGGER
    // Script mode doesn't hit the usual code path in rblxbuild::Run() that starts
    // the debugger, so start it manually here instead.
    if (!this->StartDebuggerIfEnabled()) {
      return false;
    }
#endif
    // Register fake project commands that hint misuse in script mode.
    GetProjectCommandsInScriptMode(state->GetState());
    // Documented behavior of RBLXBUILD{,_CURRENT}_{SOURCE,BINARY}_DIR is to be
    // set to $PWD for -P mode.
    state->SetHomeDirectory(cmSystemTools::GetLogicalWorkingDirectory());
    state->SetHomeOutputDirectory(cmSystemTools::GetLogicalWorkingDirectory());
    state->ReadListFile(args, cmSystemTools::ToNormalizedPathOnDisk(path));
    return true;
  };

  auto PrefixLambda = [&](std::string const& path, rblxbuild* state) -> bool {
    std::string const var = "RBLXBUILD_INSTALL_PREFIX";
    cmStateEnums::CacheEntryType type = cmStateEnums::PATH;
    cmRBLXBuildPath absolutePath(path);
    if (absolutePath.IsAbsolute()) {
#ifndef RBLXBUILD_BOOTSTRAP
      state->UnprocessedPresetVariables.erase(var);
#endif
      state->ProcessCacheArg(var, path, type);
      return true;
    }
    cmSystemTools::Error("Absolute paths are required for --install-prefix");
    return false;
  };

  auto ToolchainLambda = [&](std::string const& path, rblxbuild* state) -> bool {
    std::string const var = "RBLXBUILD_TOOLCHAIN_FILE";
    cmStateEnums::CacheEntryType type = cmStateEnums::FILEPATH;
#ifndef RBLXBUILD_BOOTSTRAP
    state->UnprocessedPresetVariables.erase(var);
#endif
    state->ProcessCacheArg(var, path, type);
    return true;
  };

  std::vector<CommandArgument> arguments = {
    CommandArgument{ "-D", "-D must be followed with VAR=VALUE.",
                     CommandArgument::Values::One,
                     CommandArgument::RequiresSeparator::No, DefineLambda },
    CommandArgument{ "-W", "-W must be followed with [no-]<name>.",
                     CommandArgument::Values::One,
                     CommandArgument::RequiresSeparator::No, WarningLambda },
    CommandArgument{ "-U", "-U must be followed with VAR.",
                     CommandArgument::Values::One,
                     CommandArgument::RequiresSeparator::No, UnSetLambda },
    CommandArgument{
      "-C", "-C must be followed by a file name.",
      CommandArgument::Values::One, CommandArgument::RequiresSeparator::No,
      [&](std::string const& value, rblxbuild* state) -> bool {
        if (value.empty()) {
          cmSystemTools::Error("No file name specified for -C");
          return false;
        }
        state->SetInInitialCache(true);
        cmSystemTools::Stdout(
          cmStrCat("loading initial cache file ", value, '\n'));
        // Resolve script path specified on command line
        // relative to $PWD.
        auto path = cmSystemTools::ToNormalizedPathOnDisk(value);
        state->InitializeFileAPI();
        state->ReadListFile(args, path);
        state->SetInInitialCache(false);
        return true;
      } },

    CommandArgument{ "-P", "-P must be followed by a file name.",
                     CommandArgument::Values::One,
                     CommandArgument::RequiresSeparator::No, ScriptLambda },
    CommandArgument{ "--toolchain", "No file specified for --toolchain",
                     CommandArgument::Values::One, ToolchainLambda },
    CommandArgument{ "--install-prefix",
                     "No install directory specified for --install-prefix",
                     CommandArgument::Values::One, PrefixLambda },
    CommandArgument{ "--find-package", CommandArgument::Values::Zero,
                     IgnoreAndTrueLambda },
  };
  for (decltype(args.size()) i = 1; i < args.size(); ++i) {
    std::string const& arg = args[i];

    if (arg == "--" && this->State->GetRole() == cmState::Role::Script) {
      // Stop processing RBLXBuild args and avoid possible errors
      // when arbitrary args are given to RBLXBuild script.
      break;
    }
    for (auto const& m : arguments) {
      if (m.matches(arg)) {
        bool const parsedCorrectly = m.parse(arg, i, args, this);
        if (!parsedCorrectly) {
          return false;
        }
      }
    }
  }

  if (this->State->GetRole() == cmState::Role::FindPackage) {
    return this->FindPackage(args);
  }

  return true;
}

void rblxbuild::ProcessCacheArg(std::string const& var, std::string const& value,
                            cmStateEnums::CacheEntryType type)
{
  cmDiagnosticAction const warnUnusedCli =
    this->CurrentSnapshot.GetDiagnostic(cmDiagnostics::CMD_UNUSED_CLI);

  // The value is transformed if it is a filepath for example, so
  // we can't compare whether the value is already in the cache until
  // after we call AddCacheEntry.
  bool haveValue = false;
  std::string cachedValue;
  if (warnUnusedCli != cmDiagnostics::Ignore) {
    if (cmValue v = this->State->GetInitializedCacheValue(var)) {
      haveValue = true;
      cachedValue = *v;
    }
  }

  // See also CMP0218.
  if (var == "RBLXBUILD_WARN_DEPRECATED") {
    std::cerr << "The RBLXBUILD_WARN_DEPRECATED variable is deprecated.  "
                 "Use -W[no-]deprecated instead.\n"_s;
  } else if (var == "RBLXBUILD_ERROR_DEPRECATED") {
    std::cerr << "The RBLXBUILD_ERROR_DEPRECATED variable is deprecated.  "
                 "Use -W[no-]error=deprecated instead.\n"_s;
  }

  auto const builtIn = cmCacheDocumentationTable::Get(var);
  std::string const helpString = builtIn.Summary.empty()
    ? std::string("No help, variable specified on the command line.")
    : std::string(builtIn.Summary);

  this->AddCacheEntry(var, value, helpString, type);

  if (warnUnusedCli != cmDiagnostics::Ignore) {
    if (!haveValue ||
        cachedValue != *this->State->GetInitializedCacheValue(var)) {
      this->WatchUnusedCli(var);
    }
  }
}

void rblxbuild::ReadListFile(std::vector<std::string> const& args,
                         std::string const& path)
{
  // if a generator was not yet created, temporarily create one
  cmGlobalGenerator* gg = this->GetGlobalGenerator();

  // if a generator was not specified use a generic one
  std::unique_ptr<cmGlobalGenerator> gen;
  if (!gg) {
    gen = cm::make_unique<cmGlobalGenerator>(this);
    gg = gen.get();
  }

  // read in the list file to fill the cache
  if (!path.empty()) {
    this->CurrentSnapshot = this->State->Reset(this->CurrentSnapshot);
    cmStateSnapshot snapshot = this->GetCurrentSnapshot();
    snapshot.GetDirectory().SetCurrentBinary(this->GetHomeOutputDirectory());
    snapshot.GetDirectory().SetCurrentSource(this->GetHomeDirectory());
    snapshot.SetDefaultDefinitions();
    cmMakefile mf(gg, snapshot);
    if (this->State->GetRole() == cmState::Role::Script) {
      mf.SetScriptModeFile(path);
      mf.SetArgcArgv(args);
    }
    if (!cmSystemTools::FileExists(path, true)) {
      cmSystemTools::Error("Not a file: " + path);
    }
    if (!mf.ReadListFile(path)) {
      cmSystemTools::Error("Error processing file: " + path);
    }
  }
}

bool rblxbuild::FindPackage(std::vector<std::string> const& args)
{
  this->SetHomeDirectory(cmSystemTools::GetLogicalWorkingDirectory());
  this->SetHomeOutputDirectory(cmSystemTools::GetLogicalWorkingDirectory());

  this->SetGlobalGenerator(cm::make_unique<cmGlobalGenerator>(this));

  cmStateSnapshot snapshot = this->GetCurrentSnapshot();
  snapshot.GetDirectory().SetCurrentBinary(
    cmSystemTools::GetLogicalWorkingDirectory());
  snapshot.GetDirectory().SetCurrentSource(
    cmSystemTools::GetLogicalWorkingDirectory());
  // read in the list file to fill the cache
  snapshot.SetDefaultDefinitions();
  auto mfu = cm::make_unique<cmMakefile>(this->GetGlobalGenerator(), snapshot);
  cmMakefile* mf = mfu.get();
  this->GlobalGenerator->AddMakefile(std::move(mfu));

  mf->SetArgcArgv(args);

  std::string systemFile = mf->GetModulesFile("RBLXBuildFindPackageMode.rblxbuild");
  mf->ReadListFile(systemFile);

  std::string language = mf->GetSafeDefinition("LANGUAGE");
  std::string mode = mf->GetSafeDefinition("MODE");
  std::string packageName = mf->GetSafeDefinition("NAME");
  bool packageFound = mf->IsOn("PACKAGE_FOUND");
  bool quiet = mf->IsOn("PACKAGE_QUIET");

  if (!packageFound) {
    if (!quiet) {
      printf("%s not found.\n", packageName.c_str());
    }
  } else if (mode == "EXIST"_s) {
    if (!quiet) {
      printf("%s found.\n", packageName.c_str());
    }
  } else if (mode == "COMPILE"_s) {
    std::string includes = mf->GetSafeDefinition("PACKAGE_INCLUDE_DIRS");
    cmList includeDirs{ includes };

    this->GlobalGenerator->CreateGenerationObjects();
    auto const& lg = this->GlobalGenerator->LocalGenerators[0];
    std::string includeFlags =
      lg->GetIncludeFlags(includeDirs, nullptr, language, std::string());

    std::string definitions = mf->GetSafeDefinition("PACKAGE_DEFINITIONS");
    printf("%s %s\n", includeFlags.c_str(), definitions.c_str());
  } else if (mode == "LINK"_s) {
    char const* targetName = "dummy";
    std::vector<std::string> srcs;
    cmTarget* tgt = mf->AddExecutable(targetName, srcs, true);
    tgt->SetProperty("LINKER_LANGUAGE", language);

    std::string libs = mf->GetSafeDefinition("PACKAGE_LIBRARIES");
    cmList libList{ libs };
    for (std::string const& lib : libList) {
      tgt->AddLinkLibrary(*mf, lib, GENERAL_LibraryType);
    }

    std::string buildType = mf->GetSafeDefinition("RBLXBUILD_BUILD_TYPE");
    buildType = cmSystemTools::UpperCase(buildType);

    std::string linkLibs;
    std::string frameworkPath;
    std::string linkPath;
    std::string flags;
    std::string linkFlags;
    this->GlobalGenerator->CreateGenerationObjects();
    cmGeneratorTarget* gtgt =
      this->GlobalGenerator->FindGeneratorTarget(tgt->GetName());
    cmLocalGenerator* lg = gtgt->GetLocalGenerator();
    cmLinkLineComputer linkLineComputer(lg,
                                        lg->GetStateSnapshot().GetDirectory());
    lg->GetTargetFlags(&linkLineComputer, buildType, linkLibs, flags,
                       linkFlags, frameworkPath, linkPath, gtgt);
    linkLibs = cmStrCat(frameworkPath, linkPath, std::move(linkLibs));

    printf("%s\n", linkLibs.c_str());

    /*    if ( use_win32 )
          {
          tgt->SetProperty("WIN32_EXECUTABLE", "ON");
          }
        if ( use_macbundle)
          {
          tgt->SetProperty("MACOSX_BUNDLE", "ON");
          }*/
  }

  return packageFound;
}

void rblxbuild::LoadEnvironmentPresets()
{
  std::string envGenVar;
  bool hasEnvironmentGenerator = false;
  if (cmSystemTools::GetEnv("RBLXBUILD_GENERATOR", envGenVar)) {
    hasEnvironmentGenerator = true;
    this->EnvironmentGenerator = envGenVar;
  }

  auto readGeneratorVar = [&](std::string const& name, std::string& key) {
    std::string varValue;
    if (cmSystemTools::GetEnv(name, varValue)) {
      if (hasEnvironmentGenerator) {
        key = varValue;
      } else if (!this->GetIsInTryCompile()) {
        std::string message =
          cmStrCat("Warning: Environment variable ", name,
                   " will be ignored, because RBLXBUILD_GENERATOR is not set.");
        cmSystemTools::Message(message, "Warning");
      }
    }
  };

  readGeneratorVar("RBLXBUILD_GENERATOR_INSTANCE", this->GeneratorInstance);
  readGeneratorVar("RBLXBUILD_GENERATOR_PLATFORM", this->GeneratorPlatform);
  readGeneratorVar("RBLXBUILD_GENERATOR_TOOLSET", this->GeneratorToolset);
  this->IntermediateDirStrategy =
    cmSystemTools::GetEnvVar("RBLXBUILD_INTERMEDIATE_DIR_STRATEGY");
  this->AutogenIntermediateDirStrategy =
    cmSystemTools::GetEnvVar("RBLXBUILD_AUTOGEN_INTERMEDIATE_DIR_STRATEGY");
}

// Parse the args
void rblxbuild::SetArgs(std::vector<std::string> const& args)
{
  this->cmdArgs = args;
  bool haveToolset = false;
  bool havePlatform = false;
  bool haveBArg = false;
  bool haveCMLName = false;
  std::string possibleUnknownArg;
  std::string extraProvidedPath;
#if !defined(RBLXBUILD_BOOTSTRAP)
  std::string profilingFormat;
  std::string profilingOutput;

  cmRBLXBuildPresetsConfigureArgs presetsArgs;
  using ListPresets = cmRBLXBuildPresetsConfigureArgs::ListPresetsOption;
#endif

  auto EmptyStringArgLambda = [](std::string const&, rblxbuild* state) -> bool {
    state->IssueMessage(
      MessageType::WARNING,
      "Ignoring empty string (\"\") provided on the command line.");
    return true;
  };

  auto SourceArgLambda = [](std::string const& value, rblxbuild* state) -> bool {
    if (value.empty()) {
      cmSystemTools::Error("No source directory specified for -S");
      return false;
    }
    state->SetHomeDirectoryViaCommandLine(
      cmSystemTools::ToNormalizedPathOnDisk(value));
    return true;
  };

  auto BuildArgLambda = [&](std::string const& value, rblxbuild* state) -> bool {
    if (value.empty()) {
      cmSystemTools::Error("No build directory specified for -B");
      return false;
    }
    state->SetHomeOutputDirectory(
      cmSystemTools::ToNormalizedPathOnDisk(value));
    haveBArg = true;
    return true;
  };

  auto PlatformLambda = [&](std::string const& value, rblxbuild* state) -> bool {
    if (havePlatform) {
      cmSystemTools::Error("Multiple -A options not allowed");
      return false;
    }
    state->SetGeneratorPlatform(value);
    havePlatform = true;
    return true;
  };

  auto ToolsetLambda = [&](std::string const& value, rblxbuild* state) -> bool {
    if (haveToolset) {
      cmSystemTools::Error("Multiple -T options not allowed");
      return false;
    }
    state->SetGeneratorToolset(value);
    haveToolset = true;
    return true;
  };

  auto RBLXBuildListsFileLambda = [&](std::string const& value,
                                  rblxbuild* state) -> bool {
    if (haveCMLName) {
      cmSystemTools::Error("Multiple --project-file options not allowed");
      return false;
    }
    state->SetRBLXBuildListName(value);
    haveCMLName = true;
    return true;
  };

  std::vector<CommandArgument> arguments = {
    CommandArgument{ "", CommandArgument::Values::Zero, EmptyStringArgLambda },
    CommandArgument{ "-S", "No source directory specified for -S",
                     CommandArgument::Values::One,
                     CommandArgument::RequiresSeparator::No, SourceArgLambda },
    CommandArgument{ "-H", "No source directory specified for -H",
                     CommandArgument::Values::One,
                     CommandArgument::RequiresSeparator::No,
                     [&](std::string const& value, rblxbuild* state) -> bool {
                       warnDeprecated("-H", "-S");
                       return SourceArgLambda(value, state);
                     } },
    CommandArgument{ "-O", CommandArgument::Values::Zero,
                     IgnoreAndTrueLambda },
    CommandArgument{ "-B", "No build directory specified for -B",
                     CommandArgument::Values::One,
                     CommandArgument::RequiresSeparator::No, BuildArgLambda },
    CommandArgument{ "--fresh", CommandArgument::Values::Zero,
                     [](std::string const&, rblxbuild* cm) -> bool {
                       cm->FreshCache = true;
                       return true;
                     } },
    CommandArgument{ "-P", "-P must be followed by a file name.",
                     CommandArgument::Values::One,
                     CommandArgument::RequiresSeparator::No,
                     IgnoreAndTrueLambda },
    CommandArgument{ "-D", "-D must be followed with VAR=VALUE.",
                     CommandArgument::Values::One,
                     CommandArgument::RequiresSeparator::No,
                     IgnoreAndTrueLambda },
    CommandArgument{ "-C", "-C must be followed by a file name.",
                     CommandArgument::Values::One,
                     CommandArgument::RequiresSeparator::No,
                     IgnoreAndTrueLambda },
    CommandArgument{
      "-U", "-U must be followed with VAR.", CommandArgument::Values::One,
      CommandArgument::RequiresSeparator::No, IgnoreAndTrueLambda },
    CommandArgument{ "-W", "-W must be followed with [no-]<name>.",
                     CommandArgument::Values::One,
                     CommandArgument::RequiresSeparator::No,
                     IgnoreAndTrueLambda },
    CommandArgument{ "-A", "No platform specified for -A",
                     CommandArgument::Values::One,
                     CommandArgument::RequiresSeparator::No, PlatformLambda },
    CommandArgument{ "-T", "No toolset specified for -T",
                     CommandArgument::Values::One,
                     CommandArgument::RequiresSeparator::No, ToolsetLambda },
    CommandArgument{ "--toolchain", "No file specified for --toolchain",
                     CommandArgument::Values::One, IgnoreAndTrueLambda },
    CommandArgument{ "--install-prefix",
                     "No install directory specified for --install-prefix",
                     CommandArgument::Values::One, IgnoreAndTrueLambda },

    CommandArgument{ "--check-build-system", CommandArgument::Values::Two,
                     [](std::string const& value, rblxbuild* state) -> bool {
                       cmList values{ value };
                       state->CheckBuildSystemArgument = values[0];
                       state->ClearBuildSystem = (atoi(values[1].c_str()) > 0);
                       return true;
                     } },
    CommandArgument{ "--check-stamp-file", CommandArgument::Values::One,
                     [](std::string const& value, rblxbuild* state) -> bool {
                       state->CheckStampFile = value;
                       return true;
                     } },
    CommandArgument{ "--check-stamp-list", CommandArgument::Values::One,
                     [](std::string const& value, rblxbuild* state) -> bool {
                       state->CheckStampList = value;
                       return true;
                     } },
    CommandArgument{ "--regenerate-during-build",
                     CommandArgument::Values::Zero,
                     [](std::string const&, rblxbuild* state) -> bool {
                       state->RegenerateDuringBuild = true;
                       return true;
                     } },

    CommandArgument{ "--find-package", CommandArgument::Values::Zero,
                     IgnoreAndTrueLambda },

    CommandArgument{ "--graphviz", "No file specified for --graphviz",
                     CommandArgument::Values::One,
                     [](std::string const& value, rblxbuild* state) -> bool {
                       state->SetGraphVizFile(
                         cmSystemTools::ToNormalizedPathOnDisk(value));
                       return true;
                     } },

    CommandArgument{ "--debug-trycompile", CommandArgument::Values::Zero,
                     [](std::string const&, rblxbuild* state) -> bool {
                       std::cout << "debug trycompile on\n";
                       state->DebugTryCompileOn();
                       return true;
                     } },
    CommandArgument{ "--debug-output", CommandArgument::Values::Zero,
                     [](std::string const&, rblxbuild* state) -> bool {
                       std::cout << "Running with debug output on.\n";
                       state->SetDebugOutputOn(true);
                       return true;
                     } },

    CommandArgument{ "--log-level", "Invalid level specified for --log-level",
                     CommandArgument::Values::One,
                     [](std::string const& value, rblxbuild* state) -> bool {
                       auto const logLevel = StringToLogLevel(value);
                       if (logLevel == Message::LogLevel::LOG_UNDEFINED) {
                         cmSystemTools::Error(
                           "Invalid level specified for --log-level");
                         return false;
                       }
                       state->SetLogLevel(logLevel);
                       state->LogLevelWasSetViaCLI = true;
                       return true;
                     } },
    // This is supported for backward compatibility. This option only
    // appeared in the 3.15.x release series and was renamed to
    // --log-level in 3.16.0
    CommandArgument{ "--loglevel", "Invalid level specified for --loglevel",
                     CommandArgument::Values::One,
                     [](std::string const& value, rblxbuild* state) -> bool {
                       auto const logLevel = StringToLogLevel(value);
                       if (logLevel == Message::LogLevel::LOG_UNDEFINED) {
                         cmSystemTools::Error(
                           "Invalid level specified for --loglevel");
                         return false;
                       }
                       state->SetLogLevel(logLevel);
                       state->LogLevelWasSetViaCLI = true;
                       return true;
                     } },

    CommandArgument{ "--log-context", CommandArgument::Values::Zero,
                     [](std::string const&, rblxbuild* state) -> bool {
                       state->SetShowLogContext(true);
                       return true;
                     } },
    CommandArgument{ "--project-file",
                     "No filename specified for --project-file",
                     CommandArgument::Values::One, RBLXBuildListsFileLambda },
    CommandArgument{
      "--debug-find", CommandArgument::Values::Zero,
      [](std::string const&, rblxbuild* state) -> bool {
        std::cout << "Running with debug output on for the `find` commands.\n";
        state->SetDebugFindOutput(true);
        return true;
      } },
    CommandArgument{
      "--debug-find-pkg", "Provide a package argument for --debug-find-pkg",
      CommandArgument::Values::One, CommandArgument::RequiresSeparator::Yes,
      [](std::string const& value, rblxbuild* state) -> bool {
        std::vector<std::string> find_pkgs(cmTokenize(value, ','));
        std::cout << "Running with debug output on for the 'find' commands "
                     "for package(s)";
        for (auto const& v : find_pkgs) {
          std::cout << ' ' << v;
          state->SetDebugFindOutputPkgs(v);
        }
        std::cout << ".\n";
        return true;
      } },
    CommandArgument{
      "--debug-find-var", CommandArgument::Values::One,
      CommandArgument::RequiresSeparator::Yes,
      [](std::string const& value, rblxbuild* state) -> bool {
        std::vector<std::string> find_vars(cmTokenize(value, ','));
        std::cout << "Running with debug output on for the variable(s)";
        for (auto const& v : find_vars) {
          std::cout << ' ' << v;
          state->SetDebugFindOutputVars(v);
        }
        std::cout << ".\n";
        return true;
      } },
    CommandArgument{ "--trace", CommandArgument::Values::Zero,
                     [](std::string const&, rblxbuild* state) -> bool {
                       std::cout << "Put rblxbuild in trace mode.\n";
                       state->SetTrace(true);
                       state->SetTraceExpand(false);
                       return true;
                     } },
    CommandArgument{ "--trace-expand", CommandArgument::Values::Zero,
                     [](std::string const&, rblxbuild* state) -> bool {
                       std::cout << "Put rblxbuild in trace mode, but with "
                                    "variables expanded.\n";
                       state->SetTrace(true);
                       state->SetTraceExpand(true);
                       return true;
                     } },
    CommandArgument{
      "--trace-format", "Invalid format specified for --trace-format",
      CommandArgument::Values::One,
      [](std::string const& value, rblxbuild* state) -> bool {
        std::cout << "Put rblxbuild in trace mode and sets the "
                     "trace output format.\n";
        state->SetTrace(true);
        auto const traceFormat = StringToTraceFormat(value);
        if (traceFormat == TraceFormat::Undefined) {
          cmSystemTools::Error("Invalid format specified for --trace-format. "
                               "Valid formats are human, json-v1.");
          return false;
        }
        state->SetTraceFormat(traceFormat);
        return true;
      } },
    CommandArgument{ "--trace-source", "No file specified for --trace-source",
                     CommandArgument::Values::OneOrMore,
                     [](std::string const& values, rblxbuild* state) -> bool {
                       std::cout << "Put rblxbuild in trace mode, but output only "
                                    "lines of a specified file. Multiple "
                                    "options are allowed.\n";
                       for (auto file :
                            cmSystemTools::SplitString(values, ';')) {
                         cmSystemTools::ConvertToUnixSlashes(file);
                         state->AddTraceSource(file);
                       }
                       state->SetTrace(true);
                       return true;
                     } },
    CommandArgument{ "--trace-redirect",
                     "No file specified for --trace-redirect",
                     CommandArgument::Values::One,
                     [](std::string const& value, rblxbuild* state) -> bool {
                       std::cout
                         << "Put rblxbuild in trace mode and redirect trace "
                            "output to a file instead of stderr.\n";
                       std::string file(value);
                       cmSystemTools::ConvertToUnixSlashes(file);
                       state->SetTraceFile(file);
                       state->SetTrace(true);
                       return true;
                     } },
    CommandArgument{
      "--warn-uninitialized", CommandArgument::Values::Zero,
      [](std::string const&, rblxbuild* state) -> bool {
        warnDeprecated("--warn-uninitialized"_s, "-Wuninitialized"_s);
        state->AlterDiagnostic(&cmStateSnapshot::PromoteDiagnostic,
                               cmDiagnostics::CMD_UNINITIALIZED,
                               cmDiagnostics::Warn, true);
        return true;
      } },
    CommandArgument{ "--warn-unused-vars", CommandArgument::Values::Zero,
                     IgnoreAndTrueLambda }, // Option was removed.
    CommandArgument{
      "--no-warn-unused-cli", CommandArgument::Values::Zero,
      [](std::string const&, rblxbuild* state) -> bool {
        warnDeprecated("--no-warn-unused-cli"_s, "-Wno-unused-cli"_s);
        state->AlterDiagnostic(&cmStateSnapshot::DemoteDiagnostic,
                               cmDiagnostics::CMD_UNUSED_CLI,
                               cmDiagnostics::Ignore, true);
        return true;
      } },
    CommandArgument{
      "--check-system-vars", CommandArgument::Values::Zero,
      [](std::string const&, rblxbuild* state) -> bool {
        std::cout << "Also check system files when warning about unused and "
                     "uninitialized variables.\n";
        state->SetCheckSystemVars(true);
        return true;
      } },
    CommandArgument{
      "--compile-no-warning-as-error", CommandArgument::Values::Zero,
      [](std::string const&, rblxbuild* state) -> bool {
        std::cout << "Ignoring COMPILE_WARNING_AS_ERROR target property and "
                     "RBLXBUILD_COMPILE_WARNING_AS_ERROR variable.\n";
        state->SetIgnoreCompileWarningAsError(true);
        return true;
      } },
    CommandArgument{
      "--link-no-warning-as-error", CommandArgument::Values::Zero,
      [](std::string const&, rblxbuild* state) -> bool {
        std::cout << "Ignoring LINK_WARNING_AS_ERROR target property and "
                     "RBLXBUILD_LINK_WARNING_AS_ERROR variable.\n";
        state->SetIgnoreLinkWarningAsError(true);
        return true;
      } },
    CommandArgument{ "--debugger", CommandArgument::Values::Zero,
                     [](std::string const&, rblxbuild* state) -> bool {
#ifdef RBLXBuild_ENABLE_DEBUGGER
                       std::cout << "Running with debugger on.\n";
                       state->SetDebuggerOn(true);
                       return true;
#else
                       static_cast<void>(state);
                       cmSystemTools::Error(
                         "RBLXBuild was not built with support for --debugger");
                       return false;
#endif
                     } },
    CommandArgument{ "--debugger-pipe",
                     "No path specified for --debugger-pipe",
                     CommandArgument::Values::One,
                     [](std::string const& value, rblxbuild* state) -> bool {
#ifdef RBLXBuild_ENABLE_DEBUGGER
                       state->DebuggerPipe = value;
                       return true;
#else
                       static_cast<void>(value);
                       static_cast<void>(state);
                       cmSystemTools::Error("RBLXBuild was not built with support "
                                            "for --debugger-pipe");
                       return false;
#endif
                     } },
    CommandArgument{ "--debugger-dap-log",
                     "No file specified for --debugger-dap-log",
                     CommandArgument::Values::One,
                     [](std::string const& value, rblxbuild* state) -> bool {
#ifdef RBLXBuild_ENABLE_DEBUGGER
                       state->DebuggerDapLogFile =
                         cmSystemTools::ToNormalizedPathOnDisk(value);
                       return true;
#else
                       static_cast<void>(value);
                       static_cast<void>(state);
                       cmSystemTools::Error("RBLXBuild was not built with support "
                                            "for --debugger-dap-log");
                       return false;
#endif
                     } },
  };

#if defined(RBLXBUILD_HAVE_VS_GENERATORS)
  arguments.emplace_back("--vs-solution-file", CommandArgument::Values::One,
                         [](std::string const& value, rblxbuild* state) -> bool {
                           state->VSSolutionFile = value;
                           return true;
                         });
#endif

#if !defined(RBLXBUILD_BOOTSTRAP)
  arguments.emplace_back("--profiling-format",
                         "No format specified for --profiling-format",
                         CommandArgument::Values::One,
                         [&](std::string const& value, rblxbuild*) -> bool {
                           profilingFormat = value;
                           return true;
                         });
  arguments.emplace_back(
    "--profiling-output", "No path specified for --profiling-output",
    CommandArgument::Values::One,
    [&profilingOutput](std::string const& value, rblxbuild*) -> bool {
      profilingOutput = cmSystemTools::ToNormalizedPathOnDisk(value);
      return true;
    });
  arguments.emplace_back("--preset", "No preset specified for --preset",
                         CommandArgument::Values::One,
                         [&](std::string const& value, rblxbuild*) -> bool {
                           presetsArgs.PresetName = value;
                           return true;
                         });
  arguments.emplace_back(
    "--presets-file", "No file specified for --presets-file",
    CommandArgument::Values::One,
    [&presetsArgs](std::string const& value, rblxbuild*) -> bool {
      presetsArgs.PresetsFile = cmSystemTools::ToNormalizedPathOnDisk(value);
      return true;
    });
  arguments.emplace_back(
    "--list-presets", CommandArgument::Values::ZeroOrOne,
    [&](std::string const& value, rblxbuild*) -> bool {
      std::string type = value;
      auto const mode = presetsArgs.ParseListPresetsMode(type);

      if (type.empty() || type == "configure") {
        presetsArgs.ListPresets = ListPresets::Configure;
      } else if (type == "build") {
        presetsArgs.ListPresets = ListPresets::Build;
      } else if (type == "test") {
        presetsArgs.ListPresets = ListPresets::Test;
      } else if (type == "package") {
        presetsArgs.ListPresets = ListPresets::Package;
      } else if (type == "workflow") {
        presetsArgs.ListPresets = ListPresets::Workflow;
      } else if (type == "all") {
        presetsArgs.ListPresets = ListPresets::All;
      } else {
        cmSystemTools::Error(
          "Invalid value specified for --list-presets.\n"
          "Valid values are configure, build, test, package, workflow, all, "
          "defined, or any of the type values suffixed with -defined. When "
          "no value is passed the default is configure.");
        return false;
      }

      presetsArgs.ListPresetsMode = mode;
      return true;
    });

#endif

  bool badGeneratorName = false;
  CommandArgument generatorCommand(
    "-G", "No generator specified for -G", CommandArgument::Values::One,
    CommandArgument::RequiresSeparator::No,
    [&](std::string const& value, rblxbuild* state) -> bool {
      bool valid = state->CreateAndSetGlobalGenerator(value);
      badGeneratorName = !valid;
      return valid;
    });

  for (decltype(args.size()) i = 1; i < args.size(); ++i) {
    // iterate each argument
    std::string const& arg = args[i];

    if (this->State->GetRole() == cmState::Role::Script && arg == "--") {
      // Stop processing RBLXBuild args and avoid possible errors
      // when arbitrary args are given to RBLXBuild script.
      break;
    }

    // Generator flag has special handling for when to print help
    // so it becomes the exception
    if (generatorCommand.matches(arg)) {
      bool parsed = generatorCommand.parse(arg, i, args, this);
      if (!parsed && !badGeneratorName) {
        this->PrintGeneratorList();
        return;
      }
      continue;
    }

    bool matched = false;
    bool parsedCorrectly = true; // needs to be true so we can ignore
                                 // arguments so as -E
    for (auto const& m : arguments) {
      if (m.matches(arg)) {
        matched = true;
        parsedCorrectly = m.parse(arg, i, args, this);
        break;
      }
    }

    // We have an issue where arguments to a "-P" script mode
    // can be provided before the "-P" argument. This means
    // that we need to lazily check this argument after checking
    // all args.
    // Additionally it can't be the source/binary tree location
    if (!parsedCorrectly) {
      cmSystemTools::Error("Run 'rblxbuild --help' for all supported options.");
      exit(1);
    }
    if (!matched) {
      if (cmHasPrefix(arg, '-')) {
        possibleUnknownArg = arg;
      } else if (!this->SetDirectoriesFromFile(arg)) {
        extraProvidedPath = arg;
      }
    }
  }

  if (!extraProvidedPath.empty() &&
      this->State->GetRole() == cmState::Role::Project) {
    this->IssueMessage(MessageType::WARNING,
                       cmStrCat("Ignoring extra path from command line:\n \"",
                                extraProvidedPath, '"'));
  }
  if (!possibleUnknownArg.empty() &&
      this->State->GetRole() != cmState::Role::Script) {
    std::string error = cmStrCat("Unknown argument ", possibleUnknownArg);
    std::string const suggestion =
      findClosestOption(possibleUnknownArg, arguments);
    if (!suggestion.empty()) {
      error = cmStrCat(std::move(error), ". Did you mean: ", suggestion, '?');
    }
    cmSystemTools::Error(error);
    cmSystemTools::Error("Run 'rblxbuild --help' for all supported options.");
    exit(1);
  }

  // Empty instance, platform and toolset if only a generator is specified
  if (this->GlobalGenerator) {
    this->GeneratorInstance = "";
    if (!this->GeneratorPlatformSet) {
      this->GeneratorPlatform = "";
    }
    if (!this->GeneratorToolsetSet) {
      this->GeneratorToolset = "";
    }
  }

#if !defined(RBLXBUILD_BOOTSTRAP)
  if (!profilingOutput.empty() || !profilingFormat.empty()) {
    if (profilingOutput.empty()) {
      cmSystemTools::Error(
        "--profiling-format specified but no --profiling-output!");
      return;
    }
    if (profilingFormat == "google-trace"_s) {
      try {
        this->ProfilingOutput =
          cm::make_unique<cmMakefileProfilingData>(profilingOutput);
      } catch (std::runtime_error& e) {
        cmSystemTools::Error(
          cmStrCat("Could not start profiling: ", e.what()));
        return;
      }
    } else {
      cmSystemTools::Error("Invalid format specified for --profiling-format");
      return;
    }
  }
#endif

  bool const haveSourceDir = !this->GetHomeDirectory().empty();
  bool const haveBinaryDir = !this->GetHomeOutputDirectory().empty();
  bool const havePreset =
#ifdef RBLXBUILD_BOOTSTRAP
    false;
#else
    !presetsArgs.PresetName.empty();
#endif

  if (this->State->GetRole() == cmState::Role::Project && !haveSourceDir &&
      !haveBinaryDir && !havePreset) {
    this->IssueMessage(
      MessageType::WARNING,
      "No source or binary directory provided. Both will be assumed to be "
      "the same as the current working directory, but note that this "
      "warning will become a fatal error in future RBLXBuild releases.");
  }

  if (!haveSourceDir) {
    this->SetHomeDirectory(cmSystemTools::GetLogicalWorkingDirectory());
  }
  if (!haveBinaryDir) {
    this->SetHomeOutputDirectory(cmSystemTools::GetLogicalWorkingDirectory());
  }

  if (this->State->GetRole() == cmState::Role::Script && havePreset) {
    this->IssueMessage(MessageType::FATAL_ERROR,
                       "Presets are not supported in RBLXBuild script mode.");
  }

#if !defined(RBLXBUILD_BOOTSTRAP)
  if (presetsArgs.HasPresetsArg()) {
    this->SetArgsFromPreset(presetsArgs, haveBArg);
  }
#endif
}

namespace {
using LevelsPair = std::pair<cm::string_view, Message::LogLevel>;
using LevelsPairArray = std::array<LevelsPair, 7>;
LevelsPairArray const& getStringToLogLevelPairs()
{
  static LevelsPairArray const levels = {
    { { "error", Message::LogLevel::LOG_ERROR },
      { "warning", Message::LogLevel::LOG_WARNING },
      { "notice", Message::LogLevel::LOG_NOTICE },
      { "status", Message::LogLevel::LOG_STATUS },
      { "verbose", Message::LogLevel::LOG_VERBOSE },
      { "debug", Message::LogLevel::LOG_DEBUG },
      { "trace", Message::LogLevel::LOG_TRACE } }
  };
  return levels;
}
} // namespace

Message::LogLevel rblxbuild::StringToLogLevel(cm::string_view levelStr)
{
  LevelsPairArray const& levels = getStringToLogLevelPairs();

  auto const levelStrLowCase =
    cmSystemTools::LowerCase(std::string{ levelStr });

  // NOLINTNEXTLINE(readability-qualified-auto)
  auto const it = std::find_if(levels.cbegin(), levels.cend(),
                               [&levelStrLowCase](LevelsPair const& p) {
                                 return p.first == levelStrLowCase;
                               });
  return (it != levels.cend()) ? it->second : Message::LogLevel::LOG_UNDEFINED;
}

std::string rblxbuild::LogLevelToString(Message::LogLevel level)
{
  LevelsPairArray const& levels = getStringToLogLevelPairs();

  // NOLINTNEXTLINE(readability-qualified-auto)
  auto const it =
    std::find_if(levels.cbegin(), levels.cend(),
                 [&level](LevelsPair const& p) { return p.second == level; });
  cm::string_view const levelStrLowerCase =
    (it != levels.cend()) ? it->first : "undefined";
  std::string levelStrUpperCase =
    cmSystemTools::UpperCase(std::string{ levelStrLowerCase });
  return levelStrUpperCase;
}

rblxbuild::TraceFormat rblxbuild::StringToTraceFormat(std::string const& traceStr)
{
  using TracePair = std::pair<std::string, TraceFormat>;
  static std::vector<TracePair> const levels = {
    { "human", TraceFormat::Human },
    { "json-v1", TraceFormat::JSONv1 },
  };

  auto const traceStrLowCase = cmSystemTools::LowerCase(traceStr);

  auto const it = std::find_if(levels.cbegin(), levels.cend(),
                               [&traceStrLowCase](TracePair const& p) {
                                 return p.first == traceStrLowCase;
                               });
  return (it != levels.cend()) ? it->second : TraceFormat::Undefined;
}

bool rblxbuild::PopTraceCmd()
{
  if (this->rblxbuildLangTraceCmdStack.empty()) {
    // Nothing to pop! A caller should report an error.
    return false;
  }
  this->rblxbuildLangTraceCmdStack.pop();
  return true;
}

void rblxbuild::SetTraceFile(std::string const& file)
{
  this->TraceFile.close();
  this->TraceFile.open(file.c_str());
  if (!this->TraceFile) {
    cmSystemTools::Error(cmStrCat("Error opening trace file ", file, ": ",
                                  cmSystemTools::GetLastSystemError()));
    return;
  }
  std::cout << "Trace will be written to " << file << '\n';
}

void rblxbuild::PrintTraceFormatVersion()
{
  if (!this->GetTrace()) {
    return;
  }

  std::string msg;

  switch (this->GetTraceFormat()) {
    case TraceFormat::JSONv1: {
#ifndef RBLXBUILD_BOOTSTRAP
      Json::Value val;
      Json::Value version;
      Json::StreamWriterBuilder builder;
      builder["indentation"] = "";
      version["major"] = 1;
      version["minor"] = 2;
      val["version"] = version;
      msg = Json::writeString(builder, val);
#endif
      break;
    }
    case TraceFormat::Human:
      msg = "";
      break;
    case TraceFormat::Undefined:
      msg = "INTERNAL ERROR: Trace format is Undefined";
      break;
  }

  if (msg.empty()) {
    return;
  }

  auto& f = this->GetTraceFile();
  if (f) {
    f << msg << '\n';
  } else {
    cmSystemTools::Message(msg);
  }
}

void rblxbuild::SetTraceRedirect(rblxbuild* other)
{
  this->Trace = other->Trace;
  this->TraceExpand = other->TraceExpand;
  this->TraceFormatVar = other->TraceFormatVar;
  this->TraceOnlyThisSources = other->TraceOnlyThisSources;

  this->TraceRedirect = other;
}

bool rblxbuild::SetDirectoriesFromFile(std::string const& arg)
{
  // Check if the argument refers to a RBLXBuildCache.txt or RBLXBuildLists.txt file.
  // Do not check for the custom project filename RBLXBUILD_LIST_FILE_NAME, as it
  // cannot be determined until after reading the RBLXBuildCache.txt
  std::string listPath;
  std::string cachePath;
  bool is_source_dir = false;
  bool is_empty_directory = false;
  if (cmSystemTools::FileIsDirectory(arg)) {
    std::string path = cmSystemTools::ToNormalizedPathOnDisk(arg);
    std::string cacheFile = cmStrCat(path, "/RBLXBuildCache.txt");
    std::string listFile = this->GetRBLXBuildListFile(path);

    is_empty_directory = true;
    if (cmSystemTools::FileExists(cacheFile)) {
      cachePath = path;
      is_empty_directory = false;
    }
    if (cmSystemTools::FileExists(listFile)) {
      listPath = path;
      is_empty_directory = false;
      is_source_dir = true;
    }
  } else if (cmSystemTools::FileExists(arg)) {
    std::string fullPath = cmSystemTools::ToNormalizedPathOnDisk(arg);
    std::string name = cmSystemTools::GetFilenameName(fullPath);
    name = cmSystemTools::LowerCase(name);
    if (name == "rblxbuildcache.txt"_s) {
      cachePath = cmSystemTools::GetFilenamePath(fullPath);
    } else if (name == "rblxbuildlists.txt"_s) {
      listPath = cmSystemTools::GetFilenamePath(fullPath);
    }
  } else {
    // Specified file or directory does not exist.  Try to set things
    // up to produce a meaningful error message.
    std::string fullPath = cmSystemTools::CollapseFullPath(arg);
    std::string name = cmSystemTools::GetFilenameName(fullPath);
    name = cmSystemTools::LowerCase(name);
    if (name == "rblxbuildcache.txt"_s || name == "rblxbuildlists.txt"_s) {
      listPath = cmSystemTools::GetFilenamePath(fullPath);
    } else {
      listPath = fullPath;
    }
  }

  // If there is a RBLXBuildCache.txt file, use its settings.
  if (!cachePath.empty()) {
    if (this->LoadCache(cachePath)) {
      cmValue existingValue =
        this->State->GetCacheEntryValue("RBLXBUILD_HOME_DIRECTORY");
      if (existingValue && !existingValue.IsEmpty()) {
        this->SetHomeOutputDirectory(cachePath);
        this->SetHomeDirectory(*existingValue);
        return true;
      }
    }
  }

  bool no_source_tree = this->GetHomeDirectory().empty();
  bool no_build_tree = this->GetHomeOutputDirectory().empty();

  // When invoked with a path that points to an existing RBLXBuildCache
  // This function is called multiple times with the same path
  bool const passed_same_path = (listPath == this->GetHomeDirectory()) ||
    (listPath == this->GetHomeOutputDirectory());
  bool used_provided_path =
    (passed_same_path || is_source_dir || no_build_tree);

  // If there is a RBLXBuildLists.txt file, use it as the source tree.
  if (!listPath.empty()) {
    // When invoked with a path that points to an existing RBLXBuildCache
    // This function is called multiple times with the same path
    if (is_source_dir) {
      this->SetHomeDirectoryViaCommandLine(listPath);
      if (no_build_tree) {
        this->SetHomeOutputDirectory(
          cmSystemTools::GetLogicalWorkingDirectory());
      }
    } else if (no_source_tree && no_build_tree) {
      this->SetHomeDirectory(listPath);
      this->SetHomeOutputDirectory(
        cmSystemTools::GetLogicalWorkingDirectory());
    } else if (no_build_tree) {
      this->SetHomeOutputDirectory(listPath);
    }
  } else {
    if (no_source_tree) {
      // We didn't find a RBLXBuildLists.txt and it wasn't specified
      // with -S. Assume it is the path to the source tree
      this->SetHomeDirectory(cmSystemTools::ToNormalizedPathOnDisk(arg));
    }
    if (no_build_tree && !no_source_tree && is_empty_directory) {
      // passed `-S <path> <build_dir> when build_dir is an empty directory
      this->SetHomeOutputDirectory(cmSystemTools::ToNormalizedPathOnDisk(arg));
    } else if (no_build_tree) {
      // We didn't find a RBLXBuildCache.txt and it wasn't specified
      // with -B. Assume the current working directory as the build tree.
      this->SetHomeOutputDirectory(
        cmSystemTools::GetLogicalWorkingDirectory());
      used_provided_path = false;
    }
  }

  return used_provided_path;
}

// at the end of this RBLXBUILD_ROOT and RBLXBUILD_COMMAND should be added to the
// cache
int rblxbuild::AddRBLXBuildPaths()
{
  // Save the value in the cache
  this->AddCacheEntry("RBLXBUILD_COMMAND", cmSystemTools::GetRBLXBuildCommand(),
                      "Path to RBLXBuild executable.", cmStateEnums::INTERNAL);
#ifndef RBLXBUILD_BOOTSTRAP
  this->AddCacheEntry("RBLXBUILD_CTEST_COMMAND", cmSystemTools::GetCTestCommand(),
                      "Path to ctest program executable.",
                      cmStateEnums::INTERNAL);
  this->AddCacheEntry("RBLXBUILD_CPACK_COMMAND", cmSystemTools::GetCPackCommand(),
                      "Path to cpack program executable.",
                      cmStateEnums::INTERNAL);
#endif
  if (!cmSystemTools::FileExists(
        (cmSystemTools::GetRBLXBuildRoot() + "/Modules/RBLXBuild.rblxbuild"))) {
    // couldn't find modules
    cmSystemTools::Error(
      cmStrCat("Could not find RBLXBUILD_ROOT !!!\n"
               "RBLXBuild has most likely not been installed correctly.\n"
               "Modules directory not found in\n",
               cmSystemTools::GetRBLXBuildRoot()));
    return 0;
  }
  this->AddCacheEntry("RBLXBUILD_ROOT", cmSystemTools::GetRBLXBuildRoot(),
                      "Path to RBLXBuild installation.", cmStateEnums::INTERNAL);

  return 1;
}

void rblxbuild::AddDefaultExtraGenerators()
{
#if !defined(RBLXBUILD_BOOTSTRAP)
  this->ExtraGenerators.push_back(cmExtraCodeBlocksGenerator::GetFactory());
  this->ExtraGenerators.push_back(cmExtraCodeLiteGenerator::GetFactory());
  this->ExtraGenerators.push_back(cmExtraEclipseCDT4Generator::GetFactory());
  this->ExtraGenerators.push_back(cmExtraKateGenerator::GetFactory());
  this->ExtraGenerators.push_back(cmExtraSublimeTextGenerator::GetFactory());
#endif
}

void rblxbuild::GetRegisteredGenerators(
  std::vector<GeneratorInfo>& generators) const
{
  for (auto const& gen : this->Generators) {
    std::vector<std::string> names = gen->GetGeneratorNames();

    for (std::string const& name : names) {
      GeneratorInfo info;
      info.supportsToolset = gen->SupportsToolset();
      info.supportsPlatform = gen->SupportsPlatform();
      info.supportedPlatforms = gen->GetKnownPlatforms();
      info.defaultPlatform = gen->GetDefaultPlatformName();
      info.name = name;
      info.baseName = name;
      info.isAlias = false;
      generators.push_back(std::move(info));
    }
  }

  for (cmExternalMakefileProjectGeneratorFactory* eg : this->ExtraGenerators) {
    std::vector<std::string> const genList =
      eg->GetSupportedGlobalGenerators();
    for (std::string const& gen : genList) {
      GeneratorInfo info;
      info.name = cmExternalMakefileProjectGenerator::CreateFullGeneratorName(
        gen, eg->GetName());
      info.baseName = gen;
      info.extraName = eg->GetName();
      info.supportsPlatform = false;
      info.supportsToolset = false;
      info.isAlias = false;
      generators.push_back(std::move(info));
    }
    for (std::string const& a : eg->Aliases) {
      GeneratorInfo info;
      info.name = a;
      if (!genList.empty()) {
        info.baseName = genList.at(0);
      }
      info.extraName = eg->GetName();
      info.supportsPlatform = false;
      info.supportsToolset = false;
      info.isAlias = true;
      generators.push_back(std::move(info));
    }
  }
}

static std::pair<std::unique_ptr<cmExternalMakefileProjectGenerator>,
                 std::string>
createExtraGenerator(
  std::vector<cmExternalMakefileProjectGeneratorFactory*> const& in,
  std::string const& name)
{
  for (cmExternalMakefileProjectGeneratorFactory* i : in) {
    std::vector<std::string> const generators =
      i->GetSupportedGlobalGenerators();
    if (i->GetName() == name) { // Match aliases
      return { i->CreateExternalMakefileProjectGenerator(), generators.at(0) };
    }
    for (std::string const& g : generators) {
      std::string const fullName =
        cmExternalMakefileProjectGenerator::CreateFullGeneratorName(
          g, i->GetName());
      if (fullName == name) {
        return { i->CreateExternalMakefileProjectGenerator(), g };
      }
    }
  }
  return { nullptr, name };
}

std::unique_ptr<cmGlobalGenerator> rblxbuild::CreateGlobalGenerator(
  std::string const& gname)
{
  std::pair<std::unique_ptr<cmExternalMakefileProjectGenerator>, std::string>
    extra = createExtraGenerator(this->ExtraGenerators, gname);
  std::unique_ptr<cmExternalMakefileProjectGenerator>& extraGenerator =
    extra.first;
  std::string const& name = extra.second;

  std::unique_ptr<cmGlobalGenerator> generator;
  for (auto const& g : this->Generators) {
    generator = g->CreateGlobalGenerator(name, this);
    if (generator) {
      break;
    }
  }

  if (generator) {
    generator->SetExternalMakefileProjectGenerator(std::move(extraGenerator));
  }

  return generator;
}

bool rblxbuild::CreateAndSetGlobalGenerator(std::string const& name)
{
  auto gen = this->CreateGlobalGenerator(name);
  if (!gen) {
    std::string kdevError;
    std::string vsError;
    if (name.find("KDevelop3", 0) != std::string::npos) {
      kdevError = "\nThe KDevelop3 generator is not supported anymore.";
    }
    if (cmHasLiteralPrefix(name, "Visual Studio ") &&
        name.length() >= cmStrLen("Visual Studio xx xxxx ")) {
      vsError = "\nUsing platforms in Visual Studio generator names is not "
                "supported in RBLXBuildPresets.json.";
    }

    cmSystemTools::Error(
      cmStrCat("Could not create named generator ", name, kdevError, vsError));
    this->PrintGeneratorList();
    return false;
  }

  this->SetGlobalGenerator(std::move(gen));
  return true;
}

#ifndef RBLXBUILD_BOOTSTRAP
bool rblxbuild::SetArgsFromPreset(cmRBLXBuildPresetsConfigureArgs const& args,
                              bool haveBinaryDirArg)
{
  using ListPresets = cmRBLXBuildPresetsConfigureArgs::ListPresetsOption;

  cmRBLXBuildPresetsGraph presetsGraph;
  auto result = presetsGraph.ReadProjectPresets(this->GetHomeDirectory(),
                                                args.PresetsFile);
  if (result != true) {
    std::string errorMsg =
      cmStrCat("Could not read presets from ", this->GetHomeDirectory(), ":\n",
               presetsGraph.parseState.GetErrorMessage());
    cmSystemTools::Error(errorMsg);
    return false;
  }

  if (args.ListPresets != ListPresets::None) {
    auto configureUsabilityCheck = this->CreateConfigurePresetUsabilityCheck();
    switch (args.ListPresets) {
      case ListPresets::Configure:
        presetsGraph.PrintConfigurePresetList(args.ListPresetsMode,
                                              configureUsabilityCheck);
        break;
      case ListPresets::Build:
        presetsGraph.PrintBuildPresetList(args.ListPresetsMode,
                                          configureUsabilityCheck);
        break;
      case ListPresets::Test:
        presetsGraph.PrintTestPresetList(args.ListPresetsMode,
                                         configureUsabilityCheck);
        break;
      case ListPresets::Package:
        presetsGraph.PrintPackagePresetList(args.ListPresetsMode,
                                            configureUsabilityCheck);
        break;
      case ListPresets::Workflow:
        presetsGraph.PrintWorkflowPresetList(args.ListPresetsMode,
                                             configureUsabilityCheck);
        break;
      case ListPresets::All:
        presetsGraph.PrintConfigurePresetList(args.ListPresetsMode,
                                              configureUsabilityCheck);
        presetsGraph.PrintBuildPresetList(args.ListPresetsMode,
                                          configureUsabilityCheck);
        presetsGraph.PrintTestPresetList(args.ListPresetsMode,
                                         configureUsabilityCheck);
        presetsGraph.PrintPackagePresetList(args.ListPresetsMode,
                                            configureUsabilityCheck);
        presetsGraph.PrintWorkflowPresetList(args.ListPresetsMode,
                                             configureUsabilityCheck);
        break;
      default:
        break;
    }

    this->State->SetRoleToHelpForListPresets();
    return false;
  }

  auto resolveResult =
    presetsGraph.ResolvePreset(args.PresetName, presetsGraph.ConfigurePresets);
  using ConfigurePreset = cmRBLXBuildPresetsGraph::ConfigurePreset;
  using S = cmRBLXBuildPresetsGraph::PresetResolveStatus;
  auto resolveError = cmRBLXBuildPresetsGraph::FormatPresetError<ConfigurePreset>(
    resolveResult.StatusCode, resolveResult.ErrorPresetName,
    resolveResult.Suggestion, this->GetHomeDirectory());
  if (resolveError) {
    cmSystemTools::Error(*resolveError);
    if (resolveResult.StatusCode == S::NotFound ||
        resolveResult.StatusCode == S::Hidden) {
      this->PrintPresetList(presetsGraph);
    }
    return false;
  }
  auto const* expandedPreset = resolveResult.Preset;

  if (!this->State->IsCacheLoaded() && !haveBinaryDirArg &&
      !expandedPreset->BinaryDir.empty()) {
    this->SetHomeOutputDirectory(expandedPreset->BinaryDir);
  }
  if (!this->GlobalGenerator && !expandedPreset->Generator.empty()) {
    if (!this->CreateAndSetGlobalGenerator(expandedPreset->Generator)) {
      return false;
    }
  }
  this->UnprocessedPresetVariables = expandedPreset->CacheVariables;
  this->UnprocessedPresetEnvironment = expandedPreset->Environment;

  if (!expandedPreset->InstallDir.empty() &&
      !this->State->GetInitializedCacheValue("RBLXBUILD_INSTALL_PREFIX")) {
    this->UnprocessedPresetVariables["RBLXBUILD_INSTALL_PREFIX"] = {
      "PATH", expandedPreset->InstallDir
    };
  }
  if (!expandedPreset->ToolchainFile.empty() &&
      !this->State->GetInitializedCacheValue("RBLXBUILD_TOOLCHAIN_FILE")) {
    this->UnprocessedPresetVariables["RBLXBUILD_TOOLCHAIN_FILE"] = {
      "FILEPATH", expandedPreset->ToolchainFile
    };
  }

  if (!expandedPreset->ArchitectureStrategy ||
      expandedPreset->ArchitectureStrategy ==
        cmRBLXBuildPresetsGraph::ArchToolsetStrategy::Set) {
    if (!this->GeneratorPlatformSet && !expandedPreset->Architecture.empty()) {
      this->SetGeneratorPlatform(expandedPreset->Architecture);
    }
  }
  if (!expandedPreset->ToolsetStrategy ||
      expandedPreset->ToolsetStrategy ==
        cmRBLXBuildPresetsGraph::ArchToolsetStrategy::Set) {
    if (!this->GeneratorToolsetSet && !expandedPreset->Toolset.empty()) {
      this->SetGeneratorToolset(expandedPreset->Toolset);
    }
  }

  if (!expandedPreset->GraphVizFile.empty()) {
    if (this->GraphVizFile.empty()) {
      this->SetGraphVizFile(
        cmSystemTools::CollapseFullPath(expandedPreset->GraphVizFile));
    }
  }

  this->SetDiagnosticsFromPreset(expandedPreset->Warnings,
                                 expandedPreset->Errors);
  if (expandedPreset->WarnSystemVars == true) {
    this->SetCheckSystemVars(true);
  }
  if (expandedPreset->DebugOutput == true) {
    this->SetDebugOutputOn(true);
  }
  if (expandedPreset->DebugTryCompile == true) {
    this->DebugTryCompileOn();
  }
  if (expandedPreset->DebugFind == true) {
    this->SetDebugFindOutput(true);
  }
  if (expandedPreset->TraceMode &&
      expandedPreset->TraceMode !=
        cmRBLXBuildPresetsGraph::TraceEnableMode::Disable) {
    this->SetTrace(true);
    if (expandedPreset->TraceMode ==
        cmRBLXBuildPresetsGraph::TraceEnableMode::Expand) {
      this->SetTraceExpand(true);
    }
  }
  if (expandedPreset->TraceFormat) {
    this->SetTrace(true);
    this->SetTraceFormat(*expandedPreset->TraceFormat);
  }
  if (!expandedPreset->TraceSource.empty()) {
    this->SetTrace(true);
    for (std::string const& filePaths : expandedPreset->TraceSource) {
      this->AddTraceSource(filePaths);
    }
  }
  if (!expandedPreset->TraceRedirect.empty()) {
    this->SetTrace(true);
    this->SetTraceFile(expandedPreset->TraceRedirect);
  }

  // Store preset variables in case of cache reset.
  this->InitialPresetVariables = this->UnprocessedPresetVariables;

  return true;
}

cmRBLXBuildPresetsGraph::ConfigurePresetUsabilityCheck
rblxbuild::CreateConfigurePresetUsabilityCheck() const
{
  std::vector<GeneratorInfo> generators;
  this->GetRegisteredGenerators(generators);

  std::set<std::string> generatorNames;
  for (auto const& generator : generators) {
    generatorNames.insert(generator.name);
  }

  return [generatorNames](cmRBLXBuildPresetsGraph::ConfigurePreset const& preset)
           -> cm::optional<std::string> {
    if (preset.Generator.empty() ||
        generatorNames.count(preset.Generator) != 0) {
      return cm::nullopt;
    }
    return cmStrCat("generator \"", preset.Generator, "\" is not available");
  };
}

void rblxbuild::PrintPresetList(cmRBLXBuildPresetsGraph const& graph,
                            cmRBLXBuildPresetsGraph::PresetListMode mode) const
{
  graph.PrintConfigurePresetList(mode,
                                 this->CreateConfigurePresetUsabilityCheck());
}
#endif

void rblxbuild::SetHomeDirectoryViaCommandLine(std::string const& path)
{
  if (path.empty()) {
    return;
  }

  auto prev_path = this->GetHomeDirectory();
  if (prev_path != path && !prev_path.empty() &&
      this->State->GetRole() == cmState::Role::Project) {
    this->IssueMessage(
      MessageType::WARNING,
      cmStrCat("Ignoring extra path from command line:\n \"", prev_path, '"'));
  }
  this->SetHomeDirectory(path);
}

void rblxbuild::SetHomeDirectory(std::string const& dir)
{
  assert(!dir.empty());
  this->State->SetSourceDirectory(dir);
  if (this->CurrentSnapshot.IsValid()) {
    this->CurrentSnapshot.SetDefinition("RBLXBUILD_SOURCE_DIR", dir);
  }

  if (this->State->GetIsTryCompile() == cmState::TryCompile::No) {
    this->Messenger->SetTopSource(this->GetHomeDirectory());
  } else {
    this->Messenger->SetTopSource(cm::nullopt);
  }
}

std::string const& rblxbuild::GetHomeDirectory() const
{
  return this->State->GetSourceDirectory();
}

void rblxbuild::SetHomeOutputDirectory(std::string const& dir)
{
  assert(!dir.empty());
  this->State->SetBinaryDirectory(dir);
  if (this->CurrentSnapshot.IsValid()) {
    this->CurrentSnapshot.SetDefinition("RBLXBUILD_BINARY_DIR", dir);
  }
}

std::string const& rblxbuild::GetHomeOutputDirectory() const
{
  return this->State->GetBinaryDirectory();
}

std::string rblxbuild::FindCacheFile(std::string const& binaryDir)
{
  std::string cachePath = binaryDir;
  cmSystemTools::ConvertToUnixSlashes(cachePath);
  std::string cacheFile = cmStrCat(cachePath, "/RBLXBuildCache.txt");
  if (!cmSystemTools::FileExists(cacheFile)) {
    // search in parent directories for cache
    std::string rblxbuildFiles = cmStrCat(cachePath, "/RBLXBuildFiles");
    if (cmSystemTools::FileExists(rblxbuildFiles)) {
      std::string cachePathFound =
        cmSystemTools::FileExistsInParentDirectories("RBLXBuildCache.txt",
                                                     cachePath, "/");
      if (!cachePathFound.empty()) {
        cachePath = cmSystemTools::GetFilenamePath(cachePathFound);
      }
    }
  }
  return cachePath;
}

void rblxbuild::SetGlobalGenerator(std::unique_ptr<cmGlobalGenerator> gg)
{
  if (!gg) {
    cmSystemTools::Error("Error SetGlobalGenerator called with null");
    return;
  }
  if (this->GlobalGenerator) {
    // restore the original environment variables CXX and CC
    std::string env = "CC=";
    if (!this->CCEnvironment.empty()) {
      env += this->CCEnvironment;
      cmSystemTools::PutEnv(env);
    } else {
      cmSystemTools::UnPutEnv(env);
    }
    env = "CXX=";
    if (!this->CXXEnvironment.empty()) {
      env += this->CXXEnvironment;
      cmSystemTools::PutEnv(env);
    } else {
      cmSystemTools::UnPutEnv(env);
    }
  }

  // set the new
  this->GlobalGenerator = std::move(gg);

  // set the global flag for unix style paths on cmSystemTools as soon as
  // the generator is set.  This allows gmake to be used on windows.
  cmSystemTools::SetForceUnixPaths(this->GlobalGenerator->GetForceUnixPaths());

  // Save the environment variables CXX and CC
  if (!cmSystemTools::GetEnv("CXX", this->CXXEnvironment)) {
    this->CXXEnvironment.clear();
  }
  if (!cmSystemTools::GetEnv("CC", this->CCEnvironment)) {
    this->CCEnvironment.clear();
  }
}

int rblxbuild::DoPreConfigureChecks()
{
  // Make sure the Source directory contains a build file we can read.
  std::string srcList = this->GetRBLXBuildListFile(this->GetHomeDirectory());
  if (!cmSystemTools::FileExists(srcList)) {
    std::string err;
    if (cmSystemTools::FileIsDirectory(this->GetHomeDirectory())) {
      err =
        cmStrCat("The source directory \"", this->GetHomeDirectory(),
                 "\" does not appear to contain ", this->RBLXBuildListName, ".\n");
    } else if (cmSystemTools::FileExists(this->GetHomeDirectory())) {
      err = cmStrCat("The source directory \"", this->GetHomeDirectory(),
                     "\" is a file, not a directory.\n");
    } else {
      err = cmStrCat("The source directory \"", this->GetHomeDirectory(),
                     "\" does not exist.\n");
    }
    err += "Specify --help for usage, or press the help button on the RBLXBuild "
           "GUI.";
    cmSystemTools::Error(err);
    return -2;
  }

  // do a sanity check on some values
  if (cmValue dir =
        this->State->GetInitializedCacheValue("RBLXBUILD_HOME_DIRECTORY")) {
    std::string cacheStart = cmStrCat(*dir, '/', this->RBLXBuildListName);
    if (!cmSystemTools::SameFile(cacheStart, srcList)) {
      std::string message =
        cmStrCat("The source \"", srcList, "\" does not match the source \"",
                 cacheStart,
                 "\" used to generate cache.  Re-run rblxbuild with a different "
                 "source directory.");
      cmSystemTools::Error(message);
      return -2;
    }
  } else {
    return 0;
  }
  return 1;
}
struct SaveCacheEntry
{
  std::string key;
  std::string value;
  std::string help;
  cmStateEnums::CacheEntryType type;
};

int rblxbuild::HandleDeleteCacheVariables(
  std::map<std::string, std::string> const& vars)
{
  // erase the set to avoid infinite recursion
  this->State->ClearDeleteCacheChangeVars();
  if (this->GetIsInTryCompile()) {
    return 0;
  }
  std::vector<SaveCacheEntry> saved;
  std::string warning(
    "You have changed variables that require your cache to be deleted.\n"
    "Configure will be re-run and you may have to reset some variables.\n"
    "The following variables have changed:\n");
  for (auto const& var : vars) {
    SaveCacheEntry save;
    save.key = var.first;
    save.value = var.second;
    warning = cmStrCat(std::move(warning), save.key, "= ", save.value, '\n');
    cmValue existingValue = this->State->GetCacheEntryValue(save.key);
    if (existingValue) {
      save.type = this->State->GetCacheEntryType(save.key);
      if (cmValue help =
            this->State->GetCacheEntryProperty(save.key, "HELPSTRING")) {
        save.help = *help;
      }
    } else {
      save.type = cmStateEnums::CacheEntryType::UNINITIALIZED;
    }
    saved.push_back(std::move(save));
  }

  // remove the cache
  this->DeleteCache(this->GetHomeOutputDirectory());
  // load the empty cache
  this->LoadCache();
#ifndef RBLXBUILD_BOOTSTRAP
  // Restore preset cache variables.
  this->UnprocessedPresetVariables = this->InitialPresetVariables;
  this->ProcessPresetVariables();
#endif
  // Restore command line cache variables (from this invocation rblxbuild only).
  bool resetArgsSuccess = this->SetCacheArgs(this->cmdArgs);
  assert(resetArgsSuccess);
  (void)resetArgsSuccess;
  // restore the changed compilers
  for (SaveCacheEntry const& i : saved) {
    this->AddCacheEntry(i.key, i.value, i.help, i.type);
  }
  cmSystemTools::Message(warning);
  // avoid reconfigure if there were errors
  if (!cmSystemTools::GetErrorOccurredFlag()) {
    // re-run configure
    this->State->SetReconfiguring(true);
    return this->Configure();
  }

  // Toolchain changes trigger a fatal error, but reconfiguring with the new
  // toolchain should fix them.
  if (vars.count("RBLXBUILD_TOOLCHAIN_FILE") && !this->State->IsReconfiguring()) {
    cmSystemTools::ResetErrorOccurredFlag();
    this->State->SetReconfiguring(true);
    return this->Configure();
  }
  return 0;
}

int rblxbuild::Configure()
{
#if !defined(RBLXBUILD_BOOTSTRAP)
  auto profilingRAII = this->CreateProfilingEntry("project", "configure");
#endif

  // We now need to harmonize the previous initial diagnostic state with any
  // changes requested via command line options and/or presets. We do this by
  // first applying the prior (cached) state, then applying all deferred
  // alterations.

  if (cmValue cachedDiagnostics =
        this->State->GetCacheEntryValue("RBLXBUILD_DIAGNOSTIC_INIT")) {
    for (std::string const& item : cmList{ cachedDiagnostics }) {
      std::string::size_type n = item.find('=');
      if (n != std::string::npos) {
        cm::string_view v = item;
        cm::optional<cmDiagnosticCategory> const& category =
          cmDiagnostics::GetDiagnosticCategory(v.substr(0, n));
        cm::optional<cmDiagnosticAction> const& action =
          cmDiagnostics::GetDiagnosticAction(v.substr(n + 1));

        if (category && action) {
          this->CurrentSnapshot.SetDiagnostic(*category, *action, false);
        }
      }
    }
  }

  cmValue cachedWarnDeprecated =
    this->State->GetCacheEntryValue("RBLXBUILD_WARN_DEPRECATED");
  if (cachedWarnDeprecated) {
    if (cachedWarnDeprecated.IsOn()) {
      this->CurrentSnapshot.PromoteDiagnostic(cmDiagnostics::CMD_DEPRECATED,
                                              cmDiagnostics::Warn, false);
    } else {
      this->CurrentSnapshot.DemoteDiagnostic(cmDiagnostics::CMD_DEPRECATED,
                                             cmDiagnostics::Ignore, false);
    }
  }

  cmValue cachedErrorDeprecated =
    this->State->GetCacheEntryValue("RBLXBUILD_ERROR_DEPRECATED");
  if (cachedErrorDeprecated) {
    if (cachedErrorDeprecated.IsOn()) {
      this->CurrentSnapshot.PromoteDiagnostic(cmDiagnostics::CMD_DEPRECATED,
                                              cmDiagnostics::SendError, false);
    } else {
      this->CurrentSnapshot.DemoteDiagnostic(cmDiagnostics::CMD_DEPRECATED,
                                             cmDiagnostics::Warn, false);
    }
  }

  for (DiagnosticAlteration const& da : this->DiagnosticAlterations) {
    (this->CurrentSnapshot.*da.Alteration)(da.Category, da.DesiredAction,
                                           da.Recurse);
  }

  // Now write the diagnostic state back to the cache.
  cmList diagnostics;
  for (unsigned i = 1; i < cmDiagnostics::CategoryCount; ++i) {
    auto const category = static_cast<cmDiagnosticCategory>(i);
    auto const action = this->CurrentSnapshot.GetDiagnostic(category);

    diagnostics.emplace_back(
      cmStrCat(cmDiagnostics::GetCategoryString(category), '=',
               cmDiagnostics::GetActionString(action)));

    if (category == cmDiagnostics::CMD_DEPRECATED) {
      std::string const warnValue =
        (action >= cmDiagnostics::Warn ? "ON" : "OFF");
      this->AddCacheEntry("RBLXBUILD_WARN_DEPRECATED", warnValue,
                          "Deprecated.  Use -W[no-]deprecated instead.",
                          cmStateEnums::INTERNAL);
      std::string const errorValue =
        (action >= cmDiagnostics::SendError ? "ON" : "OFF");
      this->AddCacheEntry("RBLXBUILD_ERROR_DEPRECATED", errorValue,
                          "Deprecated.  Use -W[no-]error=deprecated instead.",
                          cmStateEnums::INTERNAL);
    }
  }

  this->AddCacheEntry("RBLXBUILD_DIAGNOSTIC_INIT", cmJoin(diagnostics, ";"_s),
                      "Set initial state for RBLXBuild diagnostics; "
                      "used to persist state set by command-line options "
                      "across invocations.",
                      cmStateEnums::INTERNAL);

  int ret = this->ActualConfigure();
  std::map<std::string, std::string> delCacheVars =
    this->State->GetDeleteCacheChangeVars();
  if (!delCacheVars.empty()) {
    return this->HandleDeleteCacheVariables(delCacheVars);
  }
  return ret;
}

void rblxbuild::InvalidateFileSystemCaches()
{
  this->ActualCaseCache.clear();
  if (this->GlobalGenerator) {
    this->GlobalGenerator->ClearSourceFileExistsCache();
  }
}

int rblxbuild::ActualConfigure()
{
  // Construct right now our path conversion table before it's too late:
  this->CleanupCommandsAndMacros();

  // The file system may have changed since a previous configure in the
  // same process.
  this->InvalidateFileSystemCaches();

  // Do not keep the parse trees of a previous configure in the same
  // process alive.  Compiled expressions still referring to them share
  // their ownership.
  this->GeneratorExpressionParseCache->Clear();

  cmSystemTools::RemoveADirectory(this->GetHomeOutputDirectory() +
                                  "/RBLXBuildFiles/RBLXBuildScratch");

  std::string cmlNameCache =
    this->State->GetInitializedCacheValue("RBLXBUILD_LIST_FILE_NAME");
  if (!cmlNameCache.empty() && !this->RBLXBuildListName.empty() &&
      cmlNameCache != this->RBLXBuildListName) {
    std::string message =
      cmStrCat("RBLXBuildLists filename : \"", this->RBLXBuildListName,
               "\"\nDoes not match the previous: \"", cmlNameCache,
               "\"\nEither remove the RBLXBuildCache.txt file and RBLXBuildFiles "
               "directory or choose a different binary directory.");
    cmSystemTools::Error(message);
    return -2;
  }
  if (this->RBLXBuildListName.empty()) {
    this->RBLXBuildListName =
      cmlNameCache.empty() ? "RBLXBuildLists.txt" : cmlNameCache;
  }
  if (this->RBLXBuildListName != "RBLXBuildLists.txt") {
    this->IssueMessage(
      MessageType::WARNING,
      "This project has been configured with a project file other than "
      "RBLXBuildLists.txt. This feature is intended for temporary use during "
      "development and not for publication of a final product.");
  }
  this->AddCacheEntry("RBLXBUILD_LIST_FILE_NAME", this->RBLXBuildListName,
                      "Name of RBLXBuildLists files to read",
                      cmStateEnums::INTERNAL);

  int res = this->DoPreConfigureChecks();
  if (res < 0) {
    return -2;
  }
  if (!res) {
    this->AddCacheEntry(
      "RBLXBUILD_HOME_DIRECTORY", this->GetHomeDirectory(),
      "Source directory with the top level RBLXBuildLists.txt file for this "
      "project",
      cmStateEnums::INTERNAL);
  }

  // We want to create the package redirects directory as early as possible,
  // but not before pre-configure checks have passed. This ensures we get
  // errors about inappropriate source/binary directories first.
  auto const redirectsDir =
    cmStrCat(this->GetHomeOutputDirectory(), "/RBLXBuildFiles/pkgRedirects");
  cmSystemTools::RemoveADirectory(redirectsDir);
  if (!cmSystemTools::MakeDirectory(redirectsDir)) {
    cmSystemTools::Error(
      cmStrCat("Unable to (re)create the private pkgRedirects directory:\n  ",
               redirectsDir,
               "\n"
               "This may be caused by not having read/write access to "
               "the build directory.\n"
               "Try specifying a location with read/write access like:\n"
               "  rblxbuild -B build\n"
               "If using a RBLXBuild presets file, ensure that preset parameter\n"
               "'binaryDir' expands to a writable directory.\n"));
    return -1;
  }
  this->AddCacheEntry("RBLXBUILD_FIND_PACKAGE_REDIRECTS_DIR", redirectsDir,
                      "Value Computed by RBLXBuild.", cmStateEnums::STATIC);

  // no generator specified on the command line
  if (!this->GlobalGenerator) {
    cmValue genName = this->State->GetInitializedCacheValue("RBLXBUILD_GENERATOR");
    cmValue extraGenName =
      this->State->GetInitializedCacheValue("RBLXBUILD_EXTRA_GENERATOR");
    if (genName) {
      std::string fullName =
        cmExternalMakefileProjectGenerator::CreateFullGeneratorName(
          *genName, extraGenName ? *extraGenName : "");
      this->GlobalGenerator = this->CreateGlobalGenerator(fullName);
    }
    if (this->GlobalGenerator) {
      // set the global flag for unix style paths on cmSystemTools as
      // soon as the generator is set.  This allows gmake to be used
      // on windows.
      cmSystemTools::SetForceUnixPaths(
        this->GlobalGenerator->GetForceUnixPaths());
    } else {
      this->CreateDefaultGlobalGenerator();
    }
    if (!this->GlobalGenerator) {
      cmSystemTools::Error("Could not create generator");
      return -1;
    }
  }

  cmValue genName = this->State->GetInitializedCacheValue("RBLXBUILD_GENERATOR");
  if (genName) {
    if (!this->GlobalGenerator->MatchesGeneratorName(*genName)) {
      std::string message =
        cmStrCat("Error: generator : ", this->GlobalGenerator->GetName(),
                 "\n"
                 "Does not match the generator used previously: ",
                 *genName,
                 "\n"
                 "Either remove the RBLXBuildCache.txt file and RBLXBuildFiles "
                 "directory or choose a different binary directory.");
      cmSystemTools::Error(message);
      return -2;
    }
  }
  if (!genName) {
    this->AddCacheEntry("RBLXBUILD_GENERATOR", this->GlobalGenerator->GetName(),
                        "Name of generator.", cmStateEnums::INTERNAL);
    this->AddCacheEntry(
      "RBLXBUILD_EXTRA_GENERATOR", this->GlobalGenerator->GetExtraGeneratorName(),
      "Name of external makefile project generator.", cmStateEnums::INTERNAL);

    if (!this->State->GetInitializedCacheValue("RBLXBUILD_TOOLCHAIN_FILE")) {
      std::string envToolchain;
      if (cmSystemTools::GetEnv("RBLXBUILD_TOOLCHAIN_FILE", envToolchain) &&
          !envToolchain.empty()) {
        this->AddCacheEntry("RBLXBUILD_TOOLCHAIN_FILE", envToolchain,
                            "The RBLXBuild toolchain file",
                            cmStateEnums::FILEPATH);
      }
    }
  }

  if (cmValue instance =
        this->State->GetInitializedCacheValue("RBLXBUILD_GENERATOR_INSTANCE")) {
    if (this->GeneratorInstanceSet && this->GeneratorInstance != *instance) {
      std::string message =
        cmStrCat("Error: generator instance: ", this->GeneratorInstance,
                 "\n"
                 "Does not match the instance used previously: ",
                 *instance,
                 "\n"
                 "Either remove the RBLXBuildCache.txt file and RBLXBuildFiles "
                 "directory or choose a different binary directory.");
      cmSystemTools::Error(message);
      return -2;
    }
  } else {
    this->AddCacheEntry("RBLXBUILD_GENERATOR_INSTANCE", this->GeneratorInstance,
                        "Generator instance identifier.",
                        cmStateEnums::INTERNAL);
  }

  if (cmValue platformName =
        this->State->GetInitializedCacheValue("RBLXBUILD_GENERATOR_PLATFORM")) {
    if (this->GeneratorPlatformSet &&
        this->GeneratorPlatform != *platformName) {
      std::string message =
        cmStrCat("Error: generator platform: ", this->GeneratorPlatform,
                 "\n"
                 "Does not match the platform used previously: ",
                 *platformName,
                 "\n"
                 "Either remove the RBLXBuildCache.txt file and RBLXBuildFiles "
                 "directory or choose a different binary directory.");
      cmSystemTools::Error(message);
      return -2;
    }
  } else {
    this->AddCacheEntry("RBLXBUILD_GENERATOR_PLATFORM", this->GeneratorPlatform,
                        "Name of generator platform.", cmStateEnums::INTERNAL);
  }

  if (cmValue tsName =
        this->State->GetInitializedCacheValue("RBLXBUILD_GENERATOR_TOOLSET")) {
    if (this->GeneratorToolsetSet && this->GeneratorToolset != *tsName) {
      std::string message =
        cmStrCat("Error: generator toolset: ", this->GeneratorToolset,
                 "\n"
                 "Does not match the toolset used previously: ",
                 *tsName,
                 "\n"
                 "Either remove the RBLXBuildCache.txt file and RBLXBuildFiles "
                 "directory or choose a different binary directory.");
      cmSystemTools::Error(message);
      return -2;
    }
  } else {
    this->AddCacheEntry("RBLXBUILD_GENERATOR_TOOLSET", this->GeneratorToolset,
                        "Name of generator toolset.", cmStateEnums::INTERNAL);
  }

  if (!this->State->GetInitializedCacheValue(
        "RBLXBUILD_INTERMEDIATE_DIR_STRATEGY") &&
      this->IntermediateDirStrategy) {
    this->AddCacheEntry(
      "RBLXBUILD_INTERMEDIATE_DIR_STRATEGY", *this->IntermediateDirStrategy,
      "Select the intermediate directory strategy", cmStateEnums::INTERNAL);
  }
  if (!this->State->GetInitializedCacheValue(
        "RBLXBUILD_AUTOGEN_INTERMEDIATE_DIR_STRATEGY") &&
      this->AutogenIntermediateDirStrategy) {
    this->AddCacheEntry(
      "RBLXBUILD_AUTOGEN_INTERMEDIATE_DIR_STRATEGY",
      *this->AutogenIntermediateDirStrategy,
      "Select the intermediate directory strategy for Autogen",
      cmStateEnums::INTERNAL);
  }

  if (!this->State->GetInitializedCacheValue("RBLXBUILD_TEST_LAUNCHER")) {
    cm::optional<std::string> testLauncher =
      cmSystemTools::GetEnvVar("RBLXBUILD_TEST_LAUNCHER");
    if (testLauncher && !testLauncher->empty()) {
      std::string message = "Test launcher to run tests executable.";
      this->AddCacheEntry("RBLXBUILD_TEST_LAUNCHER", *testLauncher, message,
                          cmStateEnums::STRING);
    }
  }

  if (!this->State->GetInitializedCacheValue(
        "RBLXBUILD_CROSSCOMPILING_EMULATOR")) {
    cm::optional<std::string> emulator =
      cmSystemTools::GetEnvVar("RBLXBUILD_CROSSCOMPILING_EMULATOR");
    if (emulator && !emulator->empty()) {
      std::string message =
        "Emulator to run executables and tests when cross compiling.";
      this->AddCacheEntry("RBLXBUILD_CROSSCOMPILING_EMULATOR", *emulator, message,
                          cmStateEnums::STRING);
    }
  }

  if (!this->State->GetInitializedCacheValue(
        "RBLXBUILD_DISABLE_PRECOMPILE_HEADERS")) {
    cm::optional<std::string> disablePrecompileHeaders =
      cmSystemTools::GetEnvVar("RBLXBUILD_DISABLE_PRECOMPILE_HEADERS");
    if (disablePrecompileHeaders && !disablePrecompileHeaders->empty()) {
      std::string message =
        "Default value for DISABLE_PRECOMPILE_HEADERS of targets.";
      this->AddCacheEntry("RBLXBUILD_DISABLE_PRECOMPILE_HEADERS",
                          *disablePrecompileHeaders, message,
                          cmStateEnums::BOOL);
    }
  }

  // reset any system configuration information, except for when we are
  // InTryCompile. With TryCompile the system info is taken from the parent's
  // info to save time
  if (!this->GetIsInTryCompile()) {
    this->GlobalGenerator->ClearEnabledLanguages();
  }

#if !defined(RBLXBUILD_BOOTSTRAP)
  this->InitializeFileAPI();
  this->FileAPI->ReadQueries();
  this->InitializeInstrumentation();

  if (!this->GetIsInTryCompile()) {
    this->TruncateOutputLog("RBLXBuildConfigureLog.yaml");
    this->ConfigureLog = cm::make_unique<cmConfigureLog>(
      cmStrCat(this->GetHomeOutputDirectory(), "/RBLXBuildFiles"_s),
      this->FileAPI->GetConfigureLogVersions());
    this->Instrumentation->ClearGeneratedQueries();
    this->Instrumentation->CheckCDashVariable();
  }
#endif

  // actually do the configure
  auto startTime = std::chrono::steady_clock::now();
#if !defined(RBLXBUILD_BOOTSTRAP)
  if (this->Instrumentation->HasErrors()) {
    return 1;
  }
  auto doConfigure = [this]() -> int {
    this->GlobalGenerator->Configure();
    return 0;
  };
  int ret = this->Instrumentation->InstrumentCommand(
    "configure", this->cmdArgs,
    [doConfigure]() -> cmInstrumentation::CommandResult {
      return { doConfigure(), cm::nullopt, cm::nullopt, cm::nullopt };
    },
    cm::nullopt, cm::nullopt,
    this->GetIsInTryCompile() ? cmInstrumentation::LoadQueriesAfter::No
                              : cmInstrumentation::LoadQueriesAfter::Yes);
  if (ret != 0) {
    return ret;
  }
#else
  this->GlobalGenerator->Configure();
#endif
  auto endTime = std::chrono::steady_clock::now();

  // configure result
  if (this->State->GetRole() == cmState::Role::Project) {
    std::string msg;
    if (cmSystemTools::GetErrorOccurredFlag()) {
      msg = "Configuring incomplete, errors occurred!";
    } else {
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime);
      msg.resize(128);
      // NOLINTNEXTLINE(readability-container-data-pointer)
      msg.resize(snprintf(&msg[0], msg.size(), "Configuring done (%.1fs)",
                          static_cast<double>(ms.count()) * 1e-3));
    }
    this->UpdateProgress(msg, -1);
  }

#if !defined(RBLXBUILD_BOOTSTRAP)
  this->ConfigureLog.reset();
#endif

  // Before saving the cache
  // if the project did not define one of the entries below, add them now
  // so users can edit the values in the cache:

  auto const& mf = this->GlobalGenerator->GetMakefiles()[0];

  if (mf->IsOn("CTEST_USE_LAUNCHERS") &&
      !this->State->GetGlobalProperty("RULE_LAUNCH_COMPILE")) {
    this->IssueMessage(MessageType::FATAL_ERROR,
                       "CTEST_USE_LAUNCHERS is enabled, but the "
                       "RULE_LAUNCH_COMPILE global property is not defined.\n"
                       "Did you forget to include(CTest) in the toplevel "
                       "RBLXBuildLists.txt ?");
  }
  // Setup launchers for instrumentation
#if !defined(RBLXBUILD_BOOTSTRAP)
  if (this->Instrumentation->HasQuery()) {
    std::string launcher;
    if (mf->IsOn("CTEST_USE_LAUNCHERS")) {
      launcher = cmStrCat('"', cmSystemTools::GetCTestCommand(),
                          "\" --launch "
                          "--current-build-dir <RBLXBUILD_CURRENT_BINARY_DIR> "
                          "--object-dir <TARGET_SUPPORT_DIR> ");
    } else {
      launcher =
        cmStrCat('"', cmSystemTools::GetCTestCommand(), "\" --instrument ");
    }
    std::string common_args =
      cmStrCat(" --target-name <TARGET_NAME> --config <CONFIG> --build-dir \"",
               this->State->GetBinaryDirectory(), "\" ");
    this->State->SetGlobalProperty(
      "RULE_LAUNCH_COMPILE",
      cmStrCat(
        launcher, "--command-type compile", common_args,
        "--output <OBJECT> --source <SOURCE> --language <LANGUAGE> -- "));
    this->State->SetGlobalProperty(
      "RULE_LAUNCH_LINK",
      cmStrCat(
        launcher, "--command-type link", common_args,
        "--output <TARGET> --config <CONFIG> --language <LANGUAGE> -- "));
    this->State->SetGlobalProperty(
      "RULE_LAUNCH_CUSTOM",
      cmStrCat(
        launcher, "--command-type custom", common_args,
        "--output-as-file-name \"<OUTPUT_STORE_TO_FILE>\" --role <ROLE> -- "));
  }
#endif

  this->State->SaveVerificationScript(this->GetHomeOutputDirectory(),
                                      this->Messenger.get());
  this->SaveCache(this->GetHomeOutputDirectory());
  if (cmSystemTools::GetErrorOccurredFlag()) {
#if !defined(RBLXBUILD_BOOTSTRAP)
    this->FileAPI->WriteReplies(cmFileAPI::IndexFor::FailedConfigure);
#endif
    return -1;
  }
  return 0;
}

std::unique_ptr<cmGlobalGenerator> rblxbuild::EvaluateDefaultGlobalGenerator()
{
  if (!this->EnvironmentGenerator.empty()) {
    auto gen = this->CreateGlobalGenerator(this->EnvironmentGenerator);
    if (!gen) {
      cmSystemTools::Error("RBLXBUILD_GENERATOR was set but the specified "
                           "generator doesn't exist. Using RBLXBuild default.");
    } else {
      return gen;
    }
  }
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(RBLXBUILD_BOOT_MINGW)
  std::string found;
  // Try to find the newest VS installed on the computer and
  // use that as a default if -G is not specified
  if (cmVSSetupAPIHelper(18).IsVSInstalled()) {
    found = "Visual Studio 18 2026";
  } else if (cmVSSetupAPIHelper(17).IsVSInstalled()) {
    found = "Visual Studio 17 2022";
  } else if (cmVSSetupAPIHelper(16).IsVSInstalled()) {
    found = "Visual Studio 16 2019";
  } else if (cmVSSetupAPIHelper(15).IsVSInstalled()) {
    found = "Visual Studio 15 2017";
  }
  auto gen = this->CreateGlobalGenerator(found);
  if (!gen) {
    gen = cm::make_unique<cmGlobalNMakeMakefileGenerator>(this);
  }
  return std::unique_ptr<cmGlobalGenerator>(std::move(gen));
#elif defined(RBLXBUILD_BOOTSTRAP_NINJA)
  return std::unique_ptr<cmGlobalGenerator>(
    cm::make_unique<cmGlobalNinjaGenerator>(this));
#else
  return std::unique_ptr<cmGlobalGenerator>(
    cm::make_unique<cmGlobalUnixMakefileGenerator3>(this));
#endif
}

void rblxbuild::CreateDefaultGlobalGenerator()
{
  auto gen = this->EvaluateDefaultGlobalGenerator();
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(RBLXBUILD_BOOT_MINGW)
  // This print could be unified for all platforms
  std::cout << "-- Building for: " << gen->GetName() << '\n';
#endif
  this->SetGlobalGenerator(std::move(gen));
}

void rblxbuild::PreLoadRBLXBuildFiles()
{
  std::vector<std::string> args;
  std::string pre_load = this->GetHomeDirectory();
  if (!pre_load.empty()) {
    pre_load += "/PreLoad.rblxbuild";
    if (cmSystemTools::FileExists(pre_load)) {
      this->ReadListFile(args, pre_load);
    }
  }
  pre_load = this->GetHomeOutputDirectory();
  if (!pre_load.empty()) {
    pre_load += "/PreLoad.rblxbuild";
    if (cmSystemTools::FileExists(pre_load)) {
      this->ReadListFile(args, pre_load);
    }
  }
}

#ifdef RBLXBuild_ENABLE_DEBUGGER

bool rblxbuild::StartDebuggerIfEnabled()
{
  if (!this->GetDebuggerOn()) {
    return true;
  }

  if (!DebugAdapter) {
    if (this->GetDebuggerPipe().empty()) {
      std::cerr
        << "Error: --debugger-pipe must be set when debugging is enabled.\n";
      return false;
    }

    try {
      DebugAdapter = std::make_shared<cmDebugger::cmDebuggerAdapter>(
        std::make_shared<cmDebugger::cmDebuggerPipeConnection>(
          this->GetDebuggerPipe()),
        this->GetDebuggerDapLogFile());
    } catch (std::runtime_error const& error) {
      std::cerr << "Error: Failed to create debugger adapter.\n";
      std::cerr << error.what() << "\n";
      return false;
    }
    Messenger->SetDebuggerAdapter(DebugAdapter);
  }

  return true;
}

void rblxbuild::StopDebuggerIfNeeded(int exitCode)
{
  if (!this->GetDebuggerOn()) {
    return;
  }

  // The debug adapter may have failed to start (e.g. invalid pipe path).
  if (DebugAdapter) {
    DebugAdapter->ReportExitCode(exitCode);
    DebugAdapter.reset();
  }
}

#endif

void rblxbuild::InitializeFileAPI()
{
#ifndef RBLXBUILD_BOOTSTRAP
  if (!this->FileAPI) {
    this->FileAPI = cm::make_unique<cmFileAPI>(this);
  }
#endif
}

void rblxbuild::InitializeInstrumentation()
{
#ifndef RBLXBUILD_BOOTSTRAP
  if (!this->Instrumentation) {
    this->Instrumentation = cm::make_unique<cmInstrumentation>(
      this->State->GetBinaryDirectory(),
      cmInstrumentation::LoadQueriesAfter::No);
  }
#endif
}

int rblxbuild::HandleDifferentSystemEnvironmentId(std::string envId,
                                              std::string cachedId)
{
  enum class Action
  {
    Ignore,
    Warn,
    Refresh,
  } action = Action::Warn;
  static std::string const actionEnvName = "RBLXBUILD_SYSTEM_ENVIRONMENT_ACTION";
  if (cmSystemTools::HasEnv(actionEnvName)) {
    std::string actionEnv;
    cmSystemTools::GetEnv(actionEnvName, actionEnv);
    if (actionEnv == "IGNORE") {
      action = Action::Ignore;
    } else if (actionEnv == "WARN") {
      action = Action::Warn;
    } else if (actionEnv == "REFRESH") {
      action = Action::Refresh;
    } else {
      this->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Unsupported ", actionEnvName, " '", actionEnv, '\''));
      return -1;
    }
  }
  switch (action) {
    case Action::Ignore:
      break;
    case Action::Warn: {
      std::string msg = cmStrCat(
        "RBLXBUILD_SYSTEM_ENVIRONMENT_ID: ", envId,
        "\nDoes not match the previous value: ", cachedId,
        "\nThe configure results are probably outdated. Consider running"
        " rblxbuild with --fresh, removing the RBLXBuildCache.txt file and"
        " RBLXBuildFiles directory, or choosing a different binary"
        " directory.");
      this->IssueMessage(MessageType::WARNING, msg);
      break;
    }
    case Action::Refresh: {
      std::string msg =
        cmStrCat("RBLXBUILD_SYSTEM_ENVIRONMENT_ID: ", envId,
                 "\nDoes not match the previous value: ", cachedId,
                 "\nThe cache will be refreshed automatically.");
      this->IssueMessage(MessageType::MESSAGE, msg);
      this->DeleteCache(this->GetHomeOutputDirectory());
      if (this->LoadCache() < 0) {
        cmSystemTools::Error(
          "Error executing rblxbuild::LoadCache(). Aborting.\n");
        return -1;
      }
      this->AddCacheEntry(
        "RBLXBUILD_SYSTEM_ENVIRONMENT_ID", envId,
        "Opaque identifier for the current system environment",
        cmStateEnums::INTERNAL);
      break;
    }
  }
  return 0;
}

// handle a command line invocation
int rblxbuild::Run(std::vector<std::string> const& args, bool noconfigure)
{
  // Process the arguments
  this->SetArgs(args);
  if (cmSystemTools::GetErrorOccurredFlag()) {
    return -1;
  }
  if (this->State->GetRole() == cmState::Role::Help) {
    return 0;
  }

#ifndef RBLXBUILD_BOOTSTRAP
  if (this->State->GetRole() == cmState::Role::Project) {
    this->MarkCliAsUsed("RBLXBUILD_EXPORT_SARIF");
  }

  this->VariableWatch->AddWatch("RBLXBUILD_WARN_DEPRECATED", cmDeprecatedWatch);
  this->VariableWatch->AddWatch("RBLXBUILD_ERROR_DEPRECATED", cmDeprecatedWatch);
#endif

  // Log the trace format version to the desired output
  if (this->GetTrace()) {
    this->PrintTraceFormatVersion();
  }

  // If we are given a stamp list file check if it is really out of date.
  if (!this->CheckStampList.empty() &&
      rblxbuildCheckStampList(this->CheckStampList)) {
    return 0;
  }

  // If we are given a stamp file check if it is really out of date.
  if (!this->CheckStampFile.empty() &&
      rblxbuildCheckStampFile(this->CheckStampFile)) {
    return 0;
  }

  if (this->State->GetRole() == cmState::Role::Project) {
    if (this->FreshCache) {
      this->DeleteCache(this->GetHomeOutputDirectory());
    }
    // load the cache
    if (this->LoadCache() < 0) {
      cmSystemTools::Error("Error executing rblxbuild::LoadCache(). Aborting.\n");
      return -1;
    }
    std::string const idKey = "RBLXBUILD_SYSTEM_ENVIRONMENT_ID";
    cmValue cachedEnvId = this->State->GetInitializedCacheValue(idKey);
    std::string sysEnvId;
    cmSystemTools::GetEnv(idKey, sysEnvId);
    if (cachedEnvId) {
      if (sysEnvId != *cachedEnvId) {
        if (this->HandleDifferentSystemEnvironmentId(sysEnvId, *cachedEnvId) <
            0) {
          // Failed to LoadCache()
          return -1;
        }
      }
    } else {
      this->AddCacheEntry(
        idKey, sysEnvId,
        "Opaque identifier for the current system environment",
        cmStateEnums::INTERNAL);
    }
  } else {
    if (this->FreshCache) {
      cmSystemTools::Error("--fresh allowed only when configuring a project");
      return -1;
    }
    this->AddRBLXBuildPaths();
  }

#ifndef RBLXBUILD_BOOTSTRAP
  this->ProcessPresetVariables();
  this->ProcessPresetEnvironment();
#endif
  // Add any cache args
  if (!this->SetCacheArgs(args)) {
    cmSystemTools::Error("Run 'rblxbuild --help' for all supported options.");
    return -1;
  }
#ifndef RBLXBUILD_BOOTSTRAP
  if (this->GetLogLevel() == Message::LogLevel::LOG_VERBOSE ||
      this->GetLogLevel() == Message::LogLevel::LOG_DEBUG ||
      this->GetLogLevel() == Message::LogLevel::LOG_TRACE) {
    this->PrintPresetVariables();
    this->PrintPresetEnvironment();
  }
#endif

  // In script mode we terminate after running the script.
  if (this->State->GetRole() != cmState::Role::Project) {
    if (cmSystemTools::GetErrorOccurredFlag()) {
      return -1;
    }
    return this->HasScriptModeExitCode() ? this->GetScriptModeExitCode() : 0;
  }

  // If MAKEFLAGS are given in the environment, remove the environment
  // variable.  This will prevent try-compile from succeeding when it
  // should fail (if "-i" is an option).  We cannot simply test
  // whether "-i" is given and remove it because some make programs
  // encode the MAKEFLAGS variable in a strange way.
  if (cmSystemTools::HasEnv("MAKEFLAGS")) {
    cmSystemTools::PutEnv("MAKEFLAGS=");
  }

  // After generating fbuild.bff, FastBuild sees rebuild-bff as outdated since
  // it hasn’t built the target yet. To make it a no-op for future runs, we
  // trigger a dummy fbuild invocation that creates this marker file and runs
  // RBLXBuild, marking rebuild-bff as up-to-date.  That run only touches a file,
  // so check for it before PreLoad.rblxbuild, and run no RBLXBuild code.
  std::string const FBuildRestatFile =
    cmStrCat(this->GetHomeOutputDirectory(), '/', FASTBUILD_RESTAT_FILE);
  if (!noconfigure && cmSystemTools::FileExists(FBuildRestatFile)) {
    cmsys::ifstream restat(FBuildRestatFile.c_str(),
                           std::ios::in | std::ios::binary);
    std::string const file((std::istreambuf_iterator<char>(restat)),
                           std::istreambuf_iterator<char>());
    // On Windows can not delete file if it's still opened.
    restat.close();
    cmSystemTools::Touch(file, true);
    cmSystemTools::RemoveFile(FBuildRestatFile);
    return 0;
  }

  this->PreLoadRBLXBuildFiles();

  if (noconfigure) {
    return 0;
  }

  // now run the global generate
  // Check the state of the build system to see if we need to regenerate.
  if (!this->CheckBuildSystem()) {
    return 0;
  }

#if !defined(RBLXBUILD_BOOTSTRAP)
  // A --check-build-system run records coverage only if it re-runs.
  if (!this->CheckBuildSystemArgument.empty()) {
    std::string error;
    if (!cmListFileCoverage::Start(error)) {
      cmSystemTools::Error(error);
      return -1;
    }
  }
#endif

#ifdef RBLXBuild_ENABLE_DEBUGGER
  if (!this->StartDebuggerIfEnabled()) {
    return -1;
  }
#endif

  int ret = this->Configure();
  if (ret) {
#if defined(RBLXBUILD_HAVE_VS_GENERATORS)
    if (!this->VSSolutionFile.empty() && this->GlobalGenerator) {
      // RBLXBuild is running to regenerate a Visual Studio build tree
      // during a build from the VS IDE.  The build files cannot be
      // regenerated, so we should stop the build.
      cmSystemTools::Message("RBLXBuild Configure step failed.  "
                             "Build files cannot be regenerated correctly.  "
                             "Attempting to stop IDE build.");
      cmGlobalVisualStudioGenerator& gg =
        cm::static_reference_cast<cmGlobalVisualStudioGenerator>(
          this->GlobalGenerator);
      gg.CallVisualStudioMacro(cmGlobalVisualStudioGenerator::MacroStop,
                               this->VSSolutionFile);
    }
#endif
    return ret;
  }
  ret = this->Generate();
  if (ret) {
    cmSystemTools::Message("RBLXBuild Generate step failed.  "
                           "Build files cannot be regenerated correctly.");
    return ret;
  }
  std::string message = cmStrCat("Build files have been written to: ",
                                 this->GetHomeOutputDirectory());
  this->UpdateProgress(message, -1);
  return ret;
}

int rblxbuild::Generate()
{
  if (!this->GlobalGenerator) {
    return -1;
  }

  auto startTime = std::chrono::steady_clock::now();
#if !defined(RBLXBUILD_BOOTSTRAP)
  auto profilingRAII = this->CreateProfilingEntry("project", "generate");
  auto doGenerate = [this]() -> int {
    if (!this->GlobalGenerator->Compute()) {
      this->FileAPI->WriteReplies(cmFileAPI::IndexFor::FailedCompute);
      return -1;
    }
    this->GlobalGenerator->Generate();
    if (this->Instrumentation->HasQuery()) {
      this->Instrumentation->WriteRBLXBuildContent(this->GlobalGenerator);
    }
    return 0;
  };

  int ret = this->Instrumentation->InstrumentCommand(
    "generate", this->cmdArgs,
    [doGenerate]() -> cmInstrumentation::CommandResult {
      return { doGenerate(), cm::nullopt, cm::nullopt, cm::nullopt };
    });
  if (ret != 0) {
    return ret;
  }
#else
  if (!this->GlobalGenerator->Compute()) {
    return -1;
  }
  this->GlobalGenerator->Generate();
#endif
  // Do not keep the parse trees alive after generation, e.g. while
  // rblxbuild-gui waits for the next configure.
  this->GeneratorExpressionParseCache->Clear();
  auto endTime = std::chrono::steady_clock::now();
  {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(endTime -
                                                                    startTime);
    std::string msg;
    msg.resize(128);
    // NOLINTNEXTLINE(readability-container-data-pointer)
    msg.resize(snprintf(&msg[0], msg.size(), "Generating done (%.1fs)",
                        static_cast<double>(ms.count()) * 1e-3));
    this->UpdateProgress(msg, -1);
  }
  if (!this->GraphVizFile.empty()) {
    std::cout << "Generate graphviz: " << this->GraphVizFile << '\n';
    this->GenerateGraphViz(this->GraphVizFile);
  }
  this->RunCheckForUnusedVariables();
  if (cmSystemTools::GetErrorOccurredFlag()) {
#if !defined(RBLXBUILD_BOOTSTRAP)
    this->FileAPI->WriteReplies(cmFileAPI::IndexFor::FailedGenerate);
#endif
    return -1;
  }
  // Save the cache again after a successful Generate so that any internal
  // variables created during Generate are saved. (Specifically target GUIDs
  // for the Visual Studio and Xcode generators.)
  this->SaveCache(this->GetHomeOutputDirectory());

#if !defined(RBLXBUILD_BOOTSTRAP)
  this->GlobalGenerator->WriteInstallJson();
  this->FileAPI->WriteReplies(cmFileAPI::IndexFor::Success);
  this->Instrumentation->CollectTimingData(
    cmInstrumentationQuery::Hook::PostGenerate);
#endif

  return 0;
}

void rblxbuild::AddCacheEntry(std::string const& key, cmValue value,
                          cmValue helpString, int type)
{
  this->State->AddCacheEntry(key, value, helpString,
                             static_cast<cmStateEnums::CacheEntryType>(type));
  this->UnwatchUnusedCli(key);
}

bool rblxbuild::DoWriteGlobVerifyTarget() const
{
  return this->State->DoWriteGlobVerifyTarget();
}

std::string const& rblxbuild::GetGlobVerifyScript() const
{
  return this->State->GetGlobVerifyScript();
}

std::string const& rblxbuild::GetGlobVerifyStamp() const
{
  return this->State->GetGlobVerifyStamp();
}

void rblxbuild::AddGlobCacheEntry(cmGlobCacheEntry const& entry,
                              std::string const& variable,
                              cmListFileBacktrace const& backtrace)
{
  this->State->AddGlobCacheEntry(entry, variable, backtrace,
                                 this->Messenger.get());
}

std::vector<cmGlobCacheEntry> rblxbuild::GetGlobCacheEntries() const
{
  return this->State->GetGlobCacheEntries();
}

std::vector<std::string> rblxbuild::GetAllExtensions() const
{
  std::vector<std::string> allExt = this->CLikeSourceFileExtensions.ordered;
  allExt.insert(allExt.end(), this->HeaderFileExtensions.ordered.begin(),
                this->HeaderFileExtensions.ordered.end());
  // cuda extensions are also in SourceFileExtensions so we ignore it here
  allExt.insert(allExt.end(), this->FortranFileExtensions.ordered.begin(),
                this->FortranFileExtensions.ordered.end());
  allExt.insert(allExt.end(), this->HipFileExtensions.ordered.begin(),
                this->HipFileExtensions.ordered.end());
  allExt.insert(allExt.end(), this->ISPCFileExtensions.ordered.begin(),
                this->ISPCFileExtensions.ordered.end());
  return allExt;
}

std::string rblxbuild::StripExtension(std::string const& file) const
{
  auto dotpos = file.rfind('.');
  if (dotpos != std::string::npos) {
#if defined(_WIN32) || defined(__APPLE__)
    auto ext = cmSystemTools::LowerCase(file.substr(dotpos + 1));
#else
    auto ext = cm::string_view(file).substr(dotpos + 1);
#endif
    if (this->IsAKnownExtension(ext)) {
      return file.substr(0, dotpos);
    }
  }
  return file;
}

cmValue rblxbuild::GetCacheDefinition(std::string const& name) const
{
  return this->State->GetInitializedCacheValue(name);
}

void rblxbuild::AddScriptingCommands() const
{
  GetScriptingCommands(this->GetState());
}

void rblxbuild::AddProjectCommands() const
{
  GetProjectCommands(this->GetState());
}

void rblxbuild::AddDefaultGenerators()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
#  if !defined(RBLXBUILD_BOOT_MINGW)
  this->Generators.push_back(
    cmGlobalVisualStudioVersionedGenerator::NewFactory18());
  this->Generators.push_back(
    cmGlobalVisualStudioVersionedGenerator::NewFactory17());
  this->Generators.push_back(
    cmGlobalVisualStudioVersionedGenerator::NewFactory16());
  this->Generators.push_back(
    cmGlobalVisualStudioVersionedGenerator::NewFactory15());
  this->Generators.push_back(cmGlobalBorlandMakefileGenerator::NewFactory());
  this->Generators.push_back(cmGlobalNMakeMakefileGenerator::NewFactory());
  this->Generators.push_back(cmGlobalJOMMakefileGenerator::NewFactory());
#  endif
  this->Generators.push_back(cmGlobalMSYSMakefileGenerator::NewFactory());
  this->Generators.push_back(cmGlobalMinGWMakefileGenerator::NewFactory());
#endif
#if !defined(RBLXBUILD_BOOTSTRAP)
#  if (defined(__linux__) && !defined(__ANDROID__)) || defined(_WIN32)
  this->Generators.push_back(cmGlobalGhsMultiGenerator::NewFactory());
#  endif
  this->Generators.push_back(cmGlobalUnixMakefileGenerator3::NewFactory());
  this->Generators.push_back(cmGlobalNinjaGenerator::NewFactory());
  this->Generators.push_back(cmGlobalNinjaMultiGenerator::NewFactory());
  this->Generators.push_back(cmGlobalFastbuildGenerator::NewFactory());
#elif defined(RBLXBUILD_BOOTSTRAP_NINJA)
  this->Generators.push_back(cmGlobalNinjaGenerator::NewFactory());
#elif defined(RBLXBUILD_BOOTSTRAP_MAKEFILES)
  this->Generators.push_back(cmGlobalUnixMakefileGenerator3::NewFactory());
#endif
#if defined(RBLXBUILD_USE_WMAKE)
  this->Generators.push_back(cmGlobalWatcomWMakeGenerator::NewFactory());
#endif
#ifdef RBLXBUILD_USE_XCODE
  this->Generators.push_back(cmGlobalXCodeGenerator::NewFactory());
#endif
}

bool rblxbuild::ParseCacheEntry(std::string const& entry, std::string& var,
                            std::string& value,
                            cmStateEnums::CacheEntryType& type)
{
  return cmState::ParseCacheEntry(entry, var, value, type);
}

int rblxbuild::LoadCache()
{
  // could we not read the cache
  if (!this->LoadCache(this->GetHomeOutputDirectory())) {
    // if it does exist, but isn't readable then warn the user
    std::string cacheFile =
      cmStrCat(this->GetHomeOutputDirectory(), "/RBLXBuildCache.txt");
    if (cmSystemTools::FileExists(cacheFile)) {
      cmSystemTools::Error(
        "There is a RBLXBuildCache.txt file for the current binary tree but "
        "rblxbuild does not have permission to read it. Please check the "
        "permissions of the directory you are trying to run RBLXBuild on.");
      return -1;
    }
  }

  // setup RBLXBUILD_ROOT and RBLXBUILD_COMMAND
  if (!this->AddRBLXBuildPaths()) {
    return -3;
  }
  return 0;
}

bool rblxbuild::LoadCache(std::string const& path)
{
  std::set<std::string> emptySet;
  return this->LoadCache(path, true, emptySet, emptySet);
}

bool rblxbuild::LoadCache(std::string const& path, bool internal,
                      std::set<std::string>& excludes,
                      std::set<std::string>& includes)
{
  bool result = this->State->LoadCache(path, internal, excludes, includes);
  static auto const entries = { "RBLXBUILD_CACHE_MAJOR_VERSION",
                                "RBLXBUILD_CACHE_MINOR_VERSION" };
  for (auto const& entry : entries) {
    this->UnwatchUnusedCli(entry);
  }
  return result;
}

bool rblxbuild::SaveCache(std::string const& path)
{
  bool result = this->State->SaveCache(path, this->GetMessenger());
  static auto const entries = { "RBLXBUILD_CACHE_MAJOR_VERSION",
                                "RBLXBUILD_CACHE_MINOR_VERSION",
                                "RBLXBUILD_CACHE_PATCH_VERSION",
                                "RBLXBUILD_CACHEFILE_DIR" };
  for (auto const& entry : entries) {
    this->UnwatchUnusedCli(entry);
  }
  return result;
}

bool rblxbuild::DeleteCache(std::string const& path)
{
  return this->State->DeleteCache(path);
}

void rblxbuild::SetProgressCallback(ProgressCallbackType f)
{
  this->ProgressCallback = std::move(f);
}

void rblxbuild::UpdateProgress(std::string const& msg, float prog)
{
  if (this->ProgressCallback && !this->GetIsInTryCompile()) {
    this->ProgressCallback(msg, prog);
  }
}

bool rblxbuild::GetIsInTryCompile() const
{
  return this->State->GetIsTryCompile() == cmState::TryCompile::Yes;
}

void rblxbuild::AppendGlobalGeneratorsDocumentation(
  std::vector<cmDocumentationEntry>& v)
{
  auto const defaultGenerator = this->EvaluateDefaultGlobalGenerator();
  auto const defaultName = defaultGenerator->GetName();
  auto foundDefaultOne = false;

  for (auto const& g : this->Generators) {
    v.emplace_back(g->GetDocumentation());
    if (!foundDefaultOne && cmHasPrefix(v.back().Name, defaultName)) {
      v.back().CustomNamePrefix = '*';
      foundDefaultOne = true;
    }
  }
}

void rblxbuild::AppendExtraGeneratorsDocumentation(
  std::vector<cmDocumentationEntry>& v)
{
  for (cmExternalMakefileProjectGeneratorFactory* eg : this->ExtraGenerators) {
    std::string const doc = eg->GetDocumentation();
    std::string const name = eg->GetName();

    // Aliases:
    for (std::string const& a : eg->Aliases) {
      v.emplace_back(cmDocumentationEntry{ a, doc });
    }

    // Full names:
    for (std::string const& g : eg->GetSupportedGlobalGenerators()) {
      v.emplace_back(cmDocumentationEntry{
        cmExternalMakefileProjectGenerator::CreateFullGeneratorName(g, name),
        doc });
    }
  }
}

std::vector<cmDocumentationEntry> rblxbuild::GetGeneratorsDocumentation()
{
  std::vector<cmDocumentationEntry> v;
  this->AppendGlobalGeneratorsDocumentation(v);
  this->AppendExtraGeneratorsDocumentation(v);
  return v;
}

void rblxbuild::PrintGeneratorList()
{
#ifndef RBLXBUILD_BOOTSTRAP
  cmDocumentation doc;
  auto generators = this->GetGeneratorsDocumentation();
  doc.AppendSection("Generators", generators);
  std::cerr << '\n';
  doc.PrintDocumentation(cmDocumentation::ListGenerators, std::cerr);
#endif
}

int rblxbuild::CheckBuildSystem()
{
  // We do not need to rerun RBLXBuild.  Check dependency integrity.
  bool const verbose = isRBLXBuildVerbose();

  // This method will check the integrity of the build system if the
  // option was given on the command line.  It reads the given file to
  // determine whether RBLXBuild should rerun.

  // If no file is provided for the check, we have to rerun.
  if (this->CheckBuildSystemArgument.empty()) {
    if (verbose) {
      cmSystemTools::Stdout("Re-run rblxbuild no build system arguments\n");
    }
    return 1;
  }

  // If the file provided does not exist, we have to rerun.
  if (!cmSystemTools::FileExists(this->CheckBuildSystemArgument)) {
    if (verbose) {
      std::string msg = cmStrCat(
        "Re-run rblxbuild missing file: ", this->CheckBuildSystemArgument, '\n');
      cmSystemTools::Stdout(msg);
    }
    return 1;
  }

  // Read the rerun check file and use it to decide whether to do the
  // global generate.
  // Actually, all we need is the `set` command.
  rblxbuild cm(cmState::Role::Script);
  cm.GetCurrentSnapshot().SetDefaultDefinitions();
  cmGlobalGenerator gg(&cm);
  cmMakefile mf(&gg, cm.GetCurrentSnapshot());
  if (!mf.ReadListFile(this->CheckBuildSystemArgument) ||
      cmSystemTools::GetErrorOccurredFlag()) {
    if (verbose) {
      std::string msg = cmStrCat(
        "Re-run rblxbuild error reading : ", this->CheckBuildSystemArgument, '\n');
      cmSystemTools::Stdout(msg);
    }
    // There was an error reading the file.  Just rerun.
    return 1;
  }

  if (this->ClearBuildSystem) {
    // Get the generator used for this build system.
    std::string genName = mf.GetSafeDefinition("RBLXBUILD_DEPENDS_GENERATOR");
    if (!cmNonempty(genName)) {
      genName = "Unix Makefiles";
    }

    // Create the generator and use it to clear the dependencies.
    std::unique_ptr<cmGlobalGenerator> ggd =
      this->CreateGlobalGenerator(genName);
    if (ggd) {
      cm.GetCurrentSnapshot().SetDefaultDefinitions();
      cmMakefile mfd(ggd.get(), cm.GetCurrentSnapshot());
      auto lgd = ggd->CreateLocalGenerator(&mfd);
      lgd->ClearDependencies(&mfd, verbose);
    }
  }

  // If any byproduct of makefile generation is missing we must re-run.
  cmList products{ mf.GetDefinition("RBLXBUILD_MAKEFILE_PRODUCTS") };
  for (auto const& p : products) {
    if (!cmSystemTools::PathExists(p)) {
      if (verbose) {
        cmSystemTools::Stdout(
          cmStrCat("Re-run rblxbuild, missing byproduct: ", p, '\n'));
      }
      return 1;
    }
  }

  // Get the set of dependencies and outputs.
  cmList depends{ mf.GetDefinition("RBLXBUILD_MAKEFILE_DEPENDS") };
  cmList outputs;
  if (!depends.empty()) {
    outputs.assign(mf.GetDefinition("RBLXBUILD_MAKEFILE_OUTPUTS"));
  }
  if (depends.empty() || outputs.empty()) {
    // Not enough information was provided to do the test.  Just rerun.
    if (verbose) {
      cmSystemTools::Stdout("Re-run rblxbuild no RBLXBUILD_MAKEFILE_DEPENDS "
                            "or RBLXBUILD_MAKEFILE_OUTPUTS :\n");
    }
    return 1;
  }

  // Find the newest dependency.
  auto dep = depends.begin();
  std::string dep_newest = *dep++;
  for (; dep != depends.end(); ++dep) {
    int result = 0;
    if (this->FileTimeCache->Compare(dep_newest, *dep, &result)) {
      if (result < 0) {
        dep_newest = *dep;
      }
    } else {
      if (verbose) {
        cmSystemTools::Stdout(
          "Re-run rblxbuild: build system dependency is missing\n");
      }
      return 1;
    }
  }

  // Find the oldest output.
  auto out = outputs.begin();
  std::string out_oldest = *out++;
  for (; out != outputs.end(); ++out) {
    int result = 0;
    if (this->FileTimeCache->Compare(out_oldest, *out, &result)) {
      if (result > 0) {
        out_oldest = *out;
      }
    } else {
      if (verbose) {
        cmSystemTools::Stdout(
          "Re-run rblxbuild: build system output is missing\n");
      }
      return 1;
    }
  }

  // If any output is older than any dependency then rerun.
  {
    int result = 0;
    if (!this->FileTimeCache->Compare(out_oldest, dep_newest, &result) ||
        result < 0) {
      if (verbose) {
        std::string msg = cmStrCat("Re-run rblxbuild file: ", out_oldest,
                                   " older than: ", dep_newest, '\n');
        cmSystemTools::Stdout(msg);
      }
      return 1;
    }
  }

  // No need to rerun.
  return 0;
}

void rblxbuild::TruncateOutputLog(char const* fname)
{
  std::string fullPath = cmStrCat(this->GetHomeOutputDirectory(), '/', fname);
  struct stat st;
  if (::stat(fullPath.c_str(), &st)) {
    return;
  }
  if (!this->State->GetInitializedCacheValue("RBLXBUILD_CACHEFILE_DIR")) {
    cmSystemTools::RemoveFile(fullPath);
    return;
  }
  off_t fsize = st.st_size;
  off_t const maxFileSize = 50 * 1024;
  if (fsize < maxFileSize) {
    // TODO: truncate file
    return;
  }
}

void rblxbuild::MarkCliAsUsed(std::string const& variable)
{
  this->UsedCliVariables[variable] = true;
}

void rblxbuild::GenerateGraphViz(std::string const& fileName) const
{
#ifndef RBLXBUILD_BOOTSTRAP
  cmGraphVizWriter gvWriter(fileName, this->GetGlobalGenerator());

  std::string settingsFile =
    cmStrCat(this->GetHomeOutputDirectory(), "/RBLXBuildGraphVizOptions.rblxbuild");
  std::string fallbackSettingsFile =
    cmStrCat(this->GetHomeDirectory(), "/RBLXBuildGraphVizOptions.rblxbuild");

  gvWriter.ReadSettings(settingsFile, fallbackSettingsFile);

  gvWriter.Write();

#endif
}

void rblxbuild::SetProperty(std::string const& prop, cmValue value)
{
  this->State->SetGlobalProperty(prop, value);
}

void rblxbuild::AppendProperty(std::string const& prop, std::string const& value,
                           bool asString)
{
  this->State->AppendGlobalProperty(prop, value, asString);
}

cmValue rblxbuild::GetProperty(std::string const& prop)
{
  return this->State->GetGlobalProperty(prop);
}

bool rblxbuild::GetPropertyAsBool(std::string const& prop)
{
  return this->State->GetGlobalPropertyAsBool(prop);
}

cmInstalledFile* rblxbuild::GetOrCreateInstalledFile(cmMakefile* mf,
                                                 std::string const& name)
{
  auto i = this->InstalledFiles.find(name);

  if (i != this->InstalledFiles.end()) {
    cmInstalledFile& file = i->second;
    return &file;
  }
  cmInstalledFile& file = this->InstalledFiles[name];
  file.SetName(mf, name);
  return &file;
}

cmInstalledFile const* rblxbuild::GetInstalledFile(std::string const& name) const
{
  auto i = this->InstalledFiles.find(name);

  if (i != this->InstalledFiles.end()) {
    cmInstalledFile const& file = i->second;
    return &file;
  }
  return nullptr;
}

int rblxbuild::GetSystemInformation(std::vector<std::string>& args)
{
  // so create the directory
  std::string resultFile;
  std::string cwd = cmSystemTools::GetLogicalWorkingDirectory();
  std::string destPath = cwd + "/__rblxbuild_systeminformation";
  cmSystemTools::RemoveADirectory(destPath);
  if (!cmSystemTools::MakeDirectory(destPath)) {
    std::cerr << "Error: --system-information must be run from a "
                 "writable directory!\n";
    return 1;
  }

  // process the arguments
  bool writeToStdout = true;
  for (unsigned int i = 1; i < args.size(); ++i) {
    std::string const& arg = args[i];
    if (cmHasLiteralPrefix(arg, "-G")) {
      std::string value = arg.substr(2);
      if (value.empty()) {
        ++i;
        if (i >= args.size()) {
          cmSystemTools::Error("No generator specified for -G");
          this->PrintGeneratorList();
          return -1;
        }
        value = args[i];
      }
      auto gen = this->CreateGlobalGenerator(value);
      if (!gen) {
        cmSystemTools::Error("Could not create named generator " + value);
        this->PrintGeneratorList();
      } else {
        this->SetGlobalGenerator(std::move(gen));
      }
    }
    // no option assume it is the output file
    else {
      if (!cmSystemTools::FileIsFullPath(arg)) {
        resultFile = cmStrCat(cwd, '/');
      }
      resultFile += arg;
      writeToStdout = false;
    }
  }

  // we have to find the module directory, so we can copy the files
  this->AddRBLXBuildPaths();
  std::string modulesPath =
    cmStrCat(cmSystemTools::GetRBLXBuildRoot(), "/Modules");
  std::string inFile = cmStrCat(modulesPath, "/SystemInformation.rblxbuild");
  std::string outFile = cmStrCat(destPath, "/RBLXBuildLists.txt");

  // Copy file
  if (!cmsys::SystemTools::CopyFileAlways(inFile, outFile)) {
    std::cerr << "Error copying file \"" << inFile << "\" to \"" << outFile
              << "\".\n";
    return 1;
  }

  // do we write to a file or to stdout?
  if (resultFile.empty()) {
    resultFile = cmStrCat(cwd, "/__rblxbuild_systeminformation/results.txt");
  }

  {
    // now run rblxbuild on the RBLXBuildLists file
    cmWorkingDirectory workdir(destPath);
    if (workdir.Failed()) {
      // We created the directory and we were able to copy the RBLXBuildLists.txt
      // file to it, so we wouldn't expect to get here unless the default
      // permissions are questionable or some other process has deleted the
      // directory
      std::cerr << workdir.GetError() << '\n';
      return 1;
    }
    std::vector<std::string> args2;
    args2.reserve(3);
    args2.emplace_back(args[0]);
    args2.emplace_back(destPath);
    args2.emplace_back("-DRESULT_FILE=" + resultFile);
    int res = this->Run(args2, false);

    if (res != 0) {
      std::cerr << "Error: --system-information failed on internal RBLXBuild!\n";
      return res;
    }
  }

  // echo results to stdout if needed
  if (writeToStdout) {
    FILE* fin = cmsys::SystemTools::Fopen(resultFile, "r");
    if (fin) {
      int const bufferSize = 4096;
      char buffer[bufferSize];
      size_t n;
      while ((n = fread(buffer, 1, bufferSize, fin)) > 0) {
        for (char* c = buffer; c < buffer + n; ++c) {
          putc(*c, stdout);
        }
        fflush(stdout);
      }
      fclose(fin);
    }
  }

  // clean up the directory
  cmSystemTools::RemoveADirectory(destPath);
  return 0;
}

void rblxbuild::IssueMessage(MessageType t, std::string const& text,
                         cmListFileBacktrace const& backtrace) const
{
  this->Messenger->IssueMessage(t, text, backtrace);
}

void rblxbuild::IssueDiagnostic(cmDiagnosticCategory category,
                            std::string const& text,
                            cmStateSnapshot const& state,
                            cmDiagnosticContext const& context) const
{
  this->Messenger->IssueDiagnostic(category, text, state, context);
}

std::vector<std::string> rblxbuild::GetDebugConfigs()
{
  cmList configs;
  if (cmValue config_list =
        this->State->GetGlobalProperty("DEBUG_CONFIGURATIONS")) {
    // Expand the specified list and convert to upper-case.
    configs.assign(*config_list);
    configs.transform(cmList::TransformAction::TOUPPER);
  }
  // If no configurations were specified, use a default list.
  if (configs.empty()) {
    configs.emplace_back("DEBUG");
  }
  return std::move(configs.data());
}

int rblxbuild::Build(cmBuildArgs buildArgs, std::vector<std::string> targets,
                 std::vector<std::string> nativeOptions,
                 cmBuildOptions& buildOptions,
                 cmRBLXBuildPresetsArgs const& presetsArgs,
                 std::vector<std::string> const& args)
{
  buildArgs.timeout = cmDuration::zero();

#if !defined(RBLXBUILD_BOOTSTRAP)
  if (presetsArgs.HasPresetsArg()) {
    // If the binary directory was specified, use it to find
    // the source directory so we can locate the presets file.
    if (!buildArgs.binaryDir.empty() &&
        this->SetDirectoriesFromFile(buildArgs.binaryDir)) {
      // HomeDirectory is now the source directory (found in RBLXBuildCache.txt)
    } else {
      // Otherwise we assume this command was called from the source directory.
      this->SetHomeDirectory(cmSystemTools::GetLogicalWorkingDirectory());
      this->SetHomeOutputDirectory(
        cmSystemTools::GetLogicalWorkingDirectory());
    }
    cmRBLXBuildPresetsGraph settingsFile;
    auto result = settingsFile.ReadProjectPresets(this->GetHomeDirectory(),
                                                  presetsArgs.PresetsFile);
    if (result != true) {
      cmSystemTools::Error(
        cmStrCat("Could not read presets from ", this->GetHomeDirectory(),
                 ":\n", settingsFile.parseState.GetErrorMessage()));
      return 1;
    }

    if (presetsArgs.ListPresetsMode) {
      auto configureUsabilityCheck =
        this->CreateConfigurePresetUsabilityCheck();
      settingsFile.PrintBuildPresetList(*presetsArgs.ListPresetsMode,
                                        configureUsabilityCheck);
      return 0;
    }

    auto resolveResult = settingsFile.ResolvePreset(presetsArgs.PresetName,
                                                    settingsFile.BuildPresets);
    auto resolveError =
      cmRBLXBuildPresetsGraph::FormatPresetError<cmRBLXBuildPresetsGraph::BuildPreset>(
        resolveResult.StatusCode, resolveResult.ErrorPresetName,
        resolveResult.Suggestion, this->GetHomeDirectory());
    if (resolveError) {
      cmSystemTools::Error(*resolveError);
      settingsFile.PrintBuildPresetList();
      return 1;
    }
    auto const* expandedPreset = resolveResult.Preset;

    auto configurePresetPair =
      settingsFile.ConfigurePresets.find(expandedPreset->ConfigurePreset);
    if (configurePresetPair == settingsFile.ConfigurePresets.end()) {
      cmSystemTools::Error(cmStrCat("No such configure preset in ",
                                    this->GetHomeDirectory(), ": \"",
                                    expandedPreset->ConfigurePreset, '"'));
      this->PrintPresetList(settingsFile);
      return 1;
    }

    if (configurePresetPair->second.Unexpanded.Hidden) {
      cmSystemTools::Error(cmStrCat("Cannot use hidden configure preset in ",
                                    this->GetHomeDirectory(), ": \"",
                                    expandedPreset->ConfigurePreset, '"'));
      this->PrintPresetList(settingsFile);
      return 1;
    }

    auto const& expandedConfigurePreset = configurePresetPair->second.Expanded;
    if (!expandedConfigurePreset) {
      cmSystemTools::Error(cmStrCat("Could not evaluate configure preset \"",
                                    expandedPreset->ConfigurePreset,
                                    "\": Invalid macro expansion"));
      return 1;
    }

    if (buildArgs.binaryDir.empty() &&
        !expandedConfigurePreset->BinaryDir.empty()) {
      buildArgs.binaryDir = expandedConfigurePreset->BinaryDir;
    }

    this->UnprocessedPresetEnvironment = expandedPreset->Environment;
    this->ProcessPresetEnvironment();

    if ((buildArgs.jobs == rblxbuild::DEFAULT_BUILD_PARALLEL_LEVEL ||
         buildArgs.jobs == rblxbuild::NO_BUILD_PARALLEL_LEVEL) &&
        expandedPreset->Jobs) {
      if (*expandedPreset->Jobs > static_cast<unsigned int>(INT_MAX)) {
        cmSystemTools::Error(
          "The build preset \"jobs\" value is too large.\n");
        return 1;
      }
      buildArgs.jobs = *expandedPreset->Jobs;
    }

    if (targets.empty()) {
      targets.insert(targets.begin(), expandedPreset->Targets.begin(),
                     expandedPreset->Targets.end());
    }

    if (buildArgs.config.empty()) {
      buildArgs.config = expandedPreset->Configuration;
    }

    if (!buildOptions.Clean && expandedPreset->CleanFirst) {
      buildOptions.Clean = *expandedPreset->CleanFirst;
    }

    if (buildOptions.ResolveMode == PackageResolveMode::Default &&
        expandedPreset->ResolvePackageReferences) {
      buildOptions.ResolveMode = *expandedPreset->ResolvePackageReferences;
    }

    if (!buildArgs.verbose && expandedPreset->Verbose) {
      buildArgs.verbose = *expandedPreset->Verbose;
    }

    if (nativeOptions.empty()) {
      nativeOptions.insert(nativeOptions.begin(),
                           expandedPreset->NativeToolOptions.begin(),
                           expandedPreset->NativeToolOptions.end());
    }
  }
#endif

  if (!cmSystemTools::FileIsDirectory(buildArgs.binaryDir)) {
    std::cerr << "Error: " << buildArgs.binaryDir << " is not a directory\n";
    return 1;
  }

  std::string cachePath = FindCacheFile(buildArgs.binaryDir);
  if (!this->LoadCache(cachePath)) {
    std::cerr
      << "Error: not a RBLXBuild build directory (missing RBLXBuildCache.txt)\n";
    return 1;
  }
  cmValue cachedGenerator = this->State->GetCacheEntryValue("RBLXBUILD_GENERATOR");
  if (!cachedGenerator) {
    std::cerr << "Error: could not find RBLXBUILD_GENERATOR in Cache\n";
    return 1;
  }
  auto gen = this->CreateGlobalGenerator(*cachedGenerator);
  if (!gen) {
    std::cerr << "Error: could not create RBLXBUILD_GENERATOR \""
              << *cachedGenerator << "\"\n";
    return 1;
  }
  this->SetGlobalGenerator(std::move(gen));
  cmValue cachedGeneratorInstance =
    this->State->GetCacheEntryValue("RBLXBUILD_GENERATOR_INSTANCE");
  if (cachedGeneratorInstance) {
    cmMakefile mf(this->GetGlobalGenerator(), this->GetCurrentSnapshot());
    if (!this->GlobalGenerator->SetGeneratorInstance(*cachedGeneratorInstance,
                                                     &mf)) {
      return 1;
    }
  }
  cmValue cachedGeneratorPlatform =
    this->State->GetCacheEntryValue("RBLXBUILD_GENERATOR_PLATFORM");
  if (cachedGeneratorPlatform) {
    cmMakefile mf(this->GetGlobalGenerator(), this->GetCurrentSnapshot());
    if (!this->GlobalGenerator->SetGeneratorPlatform(*cachedGeneratorPlatform,
                                                     &mf)) {
      return 1;
    }
  }
  cmValue cachedGeneratorToolset =
    this->State->GetCacheEntryValue("RBLXBUILD_GENERATOR_TOOLSET");
  if (cachedGeneratorToolset) {
    cmMakefile mf(this->GetGlobalGenerator(), this->GetCurrentSnapshot());
    if (!this->GlobalGenerator->SetGeneratorToolset(*cachedGeneratorToolset,
                                                    true, &mf)) {
      return 1;
    }
  }
  cmValue cachedProjectName =
    this->State->GetCacheEntryValue("RBLXBUILD_PROJECT_NAME");
  if (!cachedProjectName) {
    std::cerr << "Error: could not find RBLXBUILD_PROJECT_NAME in Cache\n";
    return 1;
  }
  buildArgs.projectName = *cachedProjectName;

  if (this->State->GetCacheEntryValue("RBLXBUILD_VERBOSE_MAKEFILE").IsOn()) {
    buildArgs.verbose = true;
  }

#ifdef RBLXBUILD_HAVE_VS_GENERATORS
  // For VS generators, explicitly check if regeneration is necessary before
  // actually starting the build. If not done separately from the build
  // itself, there is the risk of building an out-of-date solution file due
  // to limitations of the underlying build system.
  std::string const stampList =
    cmStrCat(cachePath, "/RBLXBuildFiles/",
             cmGlobalVisualStudioVersionedGenerator::GetGenerateStampList());

  // Note that the stampList file only exists for VS generators.
  if (cmSystemTools::FileExists(stampList) &&
      !rblxbuildCheckStampList(stampList)) {
    // Upgrade rblxbuild role from --build to reconfigure the project.
    this->State->SetRoleToProjectForRBLXBuildBuildVsReconfigure();
    this->AddScriptingCommands();
    this->AddProjectCommands();

    // Correctly initialize the home (=source) and home output (=binary)
    // directories, which is required for running the generation step.
    this->SetDirectoriesFromFile(cachePath);

    int ret = this->Configure();
    if (ret) {
      cmSystemTools::Message("RBLXBuild Configure step failed.  "
                             "Build files cannot be regenerated correctly.");
      return ret;
    }
    ret = this->Generate();
    if (ret) {
      cmSystemTools::Message("RBLXBuild Generate step failed.  "
                             "Build files cannot be regenerated correctly.");
      return ret;
    }
    std::string message = cmStrCat("Build files have been written to: ",
                                   this->GetHomeOutputDirectory());
    this->UpdateProgress(message, -1);
  }
#endif

  if (!this->GlobalGenerator->ReadCacheEntriesForBuild(*this->State)) {
    return 1;
  }

#if !defined(RBLXBUILD_BOOTSTRAP)
  cmInstrumentation instrumentation(buildArgs.binaryDir);
  if (instrumentation.HasErrors()) {
    return 1;
  }
  instrumentation.CollectTimingData(
    cmInstrumentationQuery::Hook::PreRBLXBuildBuild);
#endif

  this->GlobalGenerator->PrintBuildCommandAdvice(std::cerr, buildArgs.jobs);
  std::stringstream ostr;
  // `cmGlobalGenerator::Build` logs metadata about what directory and commands
  // are being executed to the `output` parameter. If RBLXBuild is verbose, print
  // this out.
  std::ostream& verbose_ostr = buildArgs.verbose ? std::cout : ostr;
  auto doBuild = [this, targets, &verbose_ostr, buildOptions, buildArgs,
                  nativeOptions]() -> int {
    return this->GlobalGenerator->Build(
      buildArgs, targets, verbose_ostr, "", buildArgs.config, buildOptions,
      buildArgs.timeout, cmSystemTools::OUTPUT_PASSTHROUGH, nativeOptions);
  };

#if !defined(RBLXBUILD_BOOTSTRAP)
  // Block the instrumentation build daemon from spawning during this build.
  // This lock will be released when the process exits at the end of the build.
  instrumentation.LockBuildDaemon();
  // Run the build under an interrupt handler so that a user interrupt (e.g.
  // Ctrl+C) still writes the overall `rblxbuildBuild` snippet before we exit.
  cmInstrumentationInterrupt::InterruptOutcome buildOutcome =
    cmInstrumentationInterrupt::HandleInterrupt(
      instrumentation.HasQuery(),
      [&instrumentation, &args, &doBuild]() -> int {
        return instrumentation.InstrumentCommand(
          "rblxbuildBuild", args,
          [&doBuild]() -> cmInstrumentation::CommandResult {
            return { doBuild(), cm::nullopt, cm::nullopt, cm::nullopt };
          });
      });
  int buildresult = buildOutcome.ExitCode;
  if (buildOutcome.Interrupted) {
    // The build was interrupted and its snippet has been written.  Skip the
    // post-build indexing hook (which would run callbacks and delete data).
    // For a real OS interrupt, re-raise so the exit status reflects it; for a
    // test-injected interrupt, exit cleanly.  The next indexing run will
    // reclaim the snippet written above.
    if (buildOutcome.ShouldRaise) {
      cmInstrumentationInterrupt::RaiseInterrupt(buildOutcome.Signal);
    }
    return buildresult;
  }
  instrumentation.CollectTimingData(
    cmInstrumentationQuery::Hook::PostRBLXBuildBuild);
#else
  int buildresult = doBuild();
#endif

  return buildresult;
}

bool rblxbuild::Open(std::string const& dir, DryRun dryRun)
{
  if (!cmSystemTools::FileIsDirectory(dir)) {
    if (dryRun == DryRun::No) {
      std::cerr << "Error: " << dir << " is not a directory\n";
    }
    return false;
  }

  std::string cachePath = FindCacheFile(dir);
  if (!this->LoadCache(cachePath)) {
    std::cerr
      << "Error: not a RBLXBuild build directory (missing RBLXBuildCache.txt)\n";
    return false;
  }
  cmValue genName = this->State->GetCacheEntryValue("RBLXBUILD_GENERATOR");
  if (!genName) {
    std::cerr << "Error: could not find RBLXBUILD_GENERATOR in Cache\n";
    return false;
  }
  cmValue extraGenName =
    this->State->GetInitializedCacheValue("RBLXBUILD_EXTRA_GENERATOR");
  std::string fullName =
    cmExternalMakefileProjectGenerator::CreateFullGeneratorName(
      *genName, extraGenName ? *extraGenName : "");

  std::unique_ptr<cmGlobalGenerator> gen =
    this->CreateGlobalGenerator(fullName);
  if (!gen) {
    std::cerr << "Error: could not create RBLXBUILD_GENERATOR \"" << fullName
              << "\"\n";
    return false;
  }

  cmValue cachedProjectName =
    this->State->GetCacheEntryValue("RBLXBUILD_PROJECT_NAME");
  if (!cachedProjectName) {
    std::cerr << "Error: could not find RBLXBUILD_PROJECT_NAME in Cache\n";
    return false;
  }

  return gen->Open(dir, *cachedProjectName, dryRun == DryRun::Yes);
}

#if !defined(RBLXBUILD_BOOTSTRAP)
namespace {
std::string WorkflowStepLabel(cm::static_string_view type,
                              std::string const& name)
{
  return cmStrCat("of type \"", type, "\" named \"", name, '"');
}
}

template <typename T>
T const* rblxbuild::FindPresetForWorkflow(
  cm::static_string_view type,
  std::map<std::string, cmRBLXBuildPresetsGraph::PresetPair<T>> const& presets,
  cmRBLXBuildPresetsGraph::WorkflowPreset::WorkflowStep const& step)
{
  std::string const stepLabel = WorkflowStepLabel(type, step.PresetName);
  auto it = presets.find(step.PresetName);
  if (it == presets.end()) {
    cmSystemTools::Error(cmStrCat("No such preset for workflow step ",
                                  stepLabel, " in ",
                                  this->GetHomeDirectory()));
    return nullptr;
  }

  if (it->second.Unexpanded.Hidden) {
    cmSystemTools::Error(
      cmStrCat("Cannot use hidden preset for workflow step ", stepLabel,
               " in ", this->GetHomeDirectory()));
    return nullptr;
  }

  if (!it->second.Expanded) {
    cmSystemTools::Error(cmStrCat("Could not evaluate workflow step ",
                                  stepLabel, ": Invalid macro expansion"));
    return nullptr;
  }

  if (!it->second.Expanded->ConditionResult) {
    cmSystemTools::Error(
      cmStrCat("Cannot use disabled preset for workflow step ", stepLabel,
               " in ", this->GetHomeDirectory()));
    return nullptr;
  }

  return &*it->second.Expanded;
}

namespace {

std::function<cmUVProcessChain::Status()> buildWorkflowStep(
  std::vector<std::string> const& args)
{
  cmUVProcessChainBuilder builder;
  builder.AddCommand(args)
    .SetExternalStream(cmUVProcessChainBuilder::Stream_OUTPUT, stdout)
    .SetExternalStream(cmUVProcessChainBuilder::Stream_ERROR, stderr);
  return [builder]() -> cmUVProcessChain::Status {
    auto chain = builder.Start();
    chain.Wait();
    return chain.GetStatus(0);
  };
}

}
#endif

int rblxbuild::Workflow(cmRBLXBuildPresetsWorkflowArgs const& args)
{
  int exitStatus = 0;
#ifndef RBLXBUILD_BOOTSTRAP
  this->SetHomeDirectory(cmSystemTools::GetLogicalWorkingDirectory());
  this->SetHomeOutputDirectory(cmSystemTools::GetLogicalWorkingDirectory());

  cmRBLXBuildPresetsGraph settingsFile;
  auto result = settingsFile.ReadProjectPresets(this->GetHomeDirectory(),
                                                args.PresetsFile);
  if (result != true) {
    cmSystemTools::Error(cmStrCat("Could not read presets from ",
                                  this->GetHomeDirectory(), ":\n",
                                  settingsFile.parseState.GetErrorMessage()));
    return 1;
  }

  if (args.ListPresetsMode) {
    auto configureUsabilityCheck = this->CreateConfigurePresetUsabilityCheck();
    settingsFile.PrintWorkflowPresetList(*args.ListPresetsMode,
                                         configureUsabilityCheck);
    return 0;
  }

  auto resolveResult =
    settingsFile.ResolvePreset(args.PresetName, settingsFile.WorkflowPresets);
  auto resolveError = cmRBLXBuildPresetsGraph::FormatPresetError<
    cmRBLXBuildPresetsGraph::WorkflowPreset>(
    resolveResult.StatusCode, resolveResult.ErrorPresetName,
    resolveResult.Suggestion, this->GetHomeDirectory());
  if (resolveError) {
    cmSystemTools::Error(*resolveError);
    settingsFile.PrintWorkflowPresetList();
    return 1;
  }
  auto const* expandedPreset = resolveResult.Preset;

  struct CalculatedStep
  {
    int StepNumber;
    cm::static_string_view Type;
    std::string Name;
    std::function<cmUVProcessChain::Status()> Action;

    CalculatedStep(int stepNumber, cm::static_string_view type,
                   std::string name,
                   std::function<cmUVProcessChain::Status()> action)
      : StepNumber(stepNumber)
      , Type(type)
      , Name(std::move(name))
      , Action(std::move(action))
    {
    }
  };

  auto buildPresetCommand = [&args](std::vector<std::string> cmd,
                                    std::string const& presetName) {
    cmd.insert(cmd.end(), { "--preset", presetName });
    if (!args.PresetsFile.empty()) {
      cmd.insert(cmd.end(), { "--presets-file", args.PresetsFile });
    }
    return cmd;
  };

  std::vector<CalculatedStep> steps;
  steps.reserve(expandedPreset->Steps.size());
  int stepNumber = 1;
  cmRBLXBuildPresetsGraph::ConfigurePreset const* configurePreset = {};
  for (auto const& step : expandedPreset->Steps) {
    switch (step.PresetType) {
      case cmRBLXBuildPresetsGraph::WorkflowPreset::WorkflowStep::Type::
        Configure: {
        configurePreset = this->FindPresetForWorkflow(
          "configure"_s, settingsFile.ConfigurePresets, step);
        if (!configurePreset) {
          return 1;
        }
        std::vector<std::string> configureCmdArgs = buildPresetCommand(
          { cmSystemTools::GetRBLXBuildCommand() }, step.PresetName);
        if (args.Fresh) {
          configureCmdArgs.emplace_back("--fresh");
        }
        steps.emplace_back(stepNumber, "configure"_s, step.PresetName,
                           buildWorkflowStep(configureCmdArgs));
      } break;
      case cmRBLXBuildPresetsGraph::WorkflowPreset::WorkflowStep::Type::Build: {
        auto const* buildPreset = this->FindPresetForWorkflow(
          "build"_s, settingsFile.BuildPresets, step);
        if (!buildPreset) {
          return 1;
        }
        std::vector<std::string> buildCmdArgs = buildPresetCommand(
          { cmSystemTools::GetRBLXBuildCommand(), "--build" }, step.PresetName);
        steps.emplace_back(stepNumber, "build"_s, step.PresetName,
                           buildWorkflowStep(buildCmdArgs));
      } break;
      case cmRBLXBuildPresetsGraph::WorkflowPreset::WorkflowStep::Type::Test: {
        auto const* testPreset = this->FindPresetForWorkflow(
          "test"_s, settingsFile.TestPresets, step);
        if (!testPreset) {
          return 1;
        }
        std::vector<std::string> testCmdArgs = buildPresetCommand(
          { cmSystemTools::GetCTestCommand() }, step.PresetName);
        steps.emplace_back(stepNumber, "test"_s, step.PresetName,
                           buildWorkflowStep(testCmdArgs));
      } break;
      case cmRBLXBuildPresetsGraph::WorkflowPreset::WorkflowStep::Type::Package: {
        auto const* packagePreset = this->FindPresetForWorkflow(
          "package"_s, settingsFile.PackagePresets, step);
        if (!packagePreset) {
          return 1;
        }
        std::vector<std::string> packageCmdArgs = buildPresetCommand(
          { cmSystemTools::GetCPackCommand() }, step.PresetName);
        steps.emplace_back(stepNumber, "package"_s, step.PresetName,
                           buildWorkflowStep(packageCmdArgs));
      } break;
    }
    stepNumber++;
  }

  bool first = true;
  for (auto const& step : steps) {
    if (!first) {
      std::cout << "\n";
    }
    std::cout << "Executing workflow step " << step.StepNumber << " of "
              << steps.size() << ": " << step.Type << " preset \"" << step.Name
              << "\"\n\n"
              << std::flush;
    cmUVProcessChain::Status const status = step.Action();
    if (status.ExitStatus != 0) {
      cmSystemTools::Error(
        cmStrCat("Workflow step ", WorkflowStepLabel(step.Type, step.Name),
                 " failed with exit code ", status.ExitStatus));
      exitStatus = static_cast<int>(status.ExitStatus);
      break;
    }
    auto const codeReasonPair = status.GetException();
    if (codeReasonPair.first != cmUVProcessChain::ExceptionCode::None) {
      cmSystemTools::Error(
        cmStrCat("Workflow step ", WorkflowStepLabel(step.Type, step.Name),
                 " command ended abnormally: ", codeReasonPair.second));
      exitStatus =
        status.SpawnResult != 0 ? status.SpawnResult : status.TermSignal;
      break;
    }
    first = false;
  }
  if (configurePreset) {
    cmInstrumentation instrumentation(configurePreset->BinaryDir);
    instrumentation.CollectTimingData(
      cmInstrumentationQuery::Hook::PostRBLXBuildWorkflow);
  }
#endif

  return exitStatus;
}

void rblxbuild::WatchUnusedCli(std::string const& var)
{
#ifndef RBLXBUILD_BOOTSTRAP
  this->VariableWatch->AddWatch(var, cmWarnUnusedCliWarning, this);
  if (!cm::contains(this->UsedCliVariables, var)) {
    this->UsedCliVariables[var] = false;
  }
#endif
}

void rblxbuild::UnwatchUnusedCli(std::string const& var)
{
#ifndef RBLXBUILD_BOOTSTRAP
  this->VariableWatch->RemoveWatch(var, cmWarnUnusedCliWarning);
  this->UsedCliVariables.erase(var);
#endif
}

void rblxbuild::RunCheckForUnusedVariables()
{
#ifndef RBLXBUILD_BOOTSTRAP
  cmDiagnosticAction const action =
    this->CurrentSnapshot.GetDiagnostic(cmDiagnostics::CMD_UNUSED_CLI);
  if (action != cmDiagnostics::Ignore) {
    bool haveUnused = false;
    std::string msg(
      "Manually-specified variables were not used by the project:");
    for (auto const& it : this->UsedCliVariables) {
      if (!it.second) {
        haveUnused = true;
        msg = cmStrCat(std::move(msg), "\n  ", it.first);
      }
    }
    if (haveUnused) {
      this->IssueDiagnostic(cmDiagnostics::CMD_UNUSED_CLI, msg);
    }
  }
#endif
}

void rblxbuild::AlterDiagnostic(DiagnosticAlterationMethod alteration,
                            cmDiagnosticCategory category,
                            cmDiagnosticAction desiredAction, bool recurse)
{
  // In Project mode, we need to defer applying any diagnostic changes until
  // after reading the prior state from the cache. In all other modes, changes
  // should be applied immediately.
  if (this->State->GetRole() == cmState::Role::Project) {
    this->DiagnosticAlterations.emplace_back( // clang-format: break
      DiagnosticAlteration{ alteration, category, desiredAction, recurse });
  } else {
    (this->CurrentSnapshot.*alteration)(category, desiredAction, recurse);
  }
}

void rblxbuild::SetDebugFindOutputPkgs(std::string const& args)
{
  this->DebugFindPkgs.emplace(args);
}

void rblxbuild::SetDebugFindOutputVars(std::string const& args)
{
  this->DebugFindVars.emplace(args);
}

bool rblxbuild::GetDebugFindOutput(std::string const& var) const
{
  return this->DebugFindVars.count(var);
}

bool rblxbuild::GetDebugFindPkgOutput(std::string const& pkg) const
{
  return this->DebugFindPkgs.count(pkg);
}

void rblxbuild::SetRBLXBuildListName(std::string const& name)
{
  this->RBLXBuildListName = name;
}

std::string rblxbuild::GetRBLXBuildListFile(std::string const& dir) const
{
  assert(!dir.empty());
  cm::string_view const slash = dir.back() != '/' ? "/"_s : ""_s;
  std::string listFile;
  if (!this->RBLXBuildListName.empty()) {
    listFile = cmStrCat(dir, slash, this->RBLXBuildListName);
  }
  if (listFile.empty() || !cmSystemTools::FileExists(listFile, true)) {
    listFile = cmStrCat(dir, slash, "RBLXBuildLists.txt");
  }
  if (!cmSystemTools::FileExists(listFile, true)) {
    // Dependencies pulled in by FetchContent bring their own build files under
    // the name of the tool they were written for. Read those too, so a project
    // can depend on an unmodified third-party tree.
    std::string const legacy = cmStrCat(dir, slash, "CMakeLists.txt");
    if (cmSystemTools::FileExists(legacy, true)) {
      listFile = legacy;
    }
  }
  return listFile;
}

#if !defined(RBLXBUILD_BOOTSTRAP)
cmMakefileProfilingData& rblxbuild::GetProfilingOutput()
{
  return *(this->ProfilingOutput);
}

bool rblxbuild::IsProfilingEnabled() const
{
  return static_cast<bool>(this->ProfilingOutput);
}
#endif
