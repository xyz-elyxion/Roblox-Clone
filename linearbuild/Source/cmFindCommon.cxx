/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#include "cmFindCommon.h"

#include <algorithm>
#include <array>
#include <utility>

#include <cmext/algorithm>

#include "cmExecutionStatus.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "linearbuild.h"

#ifndef LINEARBUILD_BOOTSTRAP
#  include <deque>
#  include <iterator>

#  include <cm/optional>
#  include <cm/string_view>
#  include <cmext/string_view>

#  include "cmConfigureLog.h"
#  include "cmRange.h"
#endif

cmFindCommon::PathGroup cmFindCommon::PathGroup::All("ALL");
cmFindCommon::PathLabel cmFindCommon::PathLabel::PackageRoot(
  "PackageName_ROOT");
cmFindCommon::PathLabel cmFindCommon::PathLabel::LinearBuild("LINEARBUILD");
cmFindCommon::PathLabel cmFindCommon::PathLabel::LinearBuildEnvironment(
  "LINEARBUILD_ENVIRONMENT");
cmFindCommon::PathLabel cmFindCommon::PathLabel::Hints("HINTS");
cmFindCommon::PathLabel cmFindCommon::PathLabel::SystemEnvironment(
  "SYSTEM_ENVIRONMENT");
cmFindCommon::PathLabel cmFindCommon::PathLabel::LinearBuildSystem("LINEARBUILD_SYSTEM");
cmFindCommon::PathLabel cmFindCommon::PathLabel::Guess("GUESS");

cmFindCommon::cmFindCommon(cmExecutionStatus& status)
  : Makefile(&status.GetMakefile())
  , Status(status)
{
  this->FindRootPathMode = RootPathModeBoth;
  this->FullDebugMode = false;
  this->NoDefaultPath = false;
  this->NoPackageRootPath = false;
  this->NoLinearBuildPath = false;
  this->NoLinearBuildEnvironmentPath = false;
  this->NoSystemEnvironmentPath = false;
  this->NoLinearBuildSystemPath = false;
  this->NoLinearBuildInstallPath = false;

// OS X Bundle and Framework search policy.  The default is to
// search frameworks first on apple.
#if defined(__APPLE__)
  this->SearchFrameworkFirst = true;
  this->SearchAppBundleFirst = true;
#else
  this->SearchFrameworkFirst = false;
  this->SearchAppBundleFirst = false;
#endif
  this->SearchFrameworkOnly = false;
  this->SearchFrameworkLast = false;
  this->SearchAppBundleOnly = false;
  this->SearchAppBundleLast = false;

  this->InitializeSearchPathGroups();

  // Windows Registry views
  // When policy CMP0134 is not NEW, rely on previous behavior:
  if (this->Makefile->GetPolicyStatus(cmPolicies::CMP0134) !=
      cmPolicies::NEW) {
    if (this->Makefile->GetDefinition("LINEARBUILD_SIZEOF_VOID_P") == "8") {
      this->RegistryView = cmWindowsRegistry::View::Reg64;
    } else {
      this->RegistryView = cmWindowsRegistry::View::Reg32;
    }
  }
}

cmFindCommon::~cmFindCommon() = default;

void cmFindCommon::SetError(std::string const& e)
{
  this->Status.SetError(e);
}

bool cmFindCommon::DebugModeEnabled() const
{
  return this->FullDebugMode;
}

void cmFindCommon::DebugMessage(std::string const& msg) const
{
  if (this->Makefile) {
    this->Makefile->IssueMessage(MessageType::LOG, msg);
  }
}

bool cmFindCommon::ComputeIfDebugModeWanted()
{
  return this->Makefile->GetDebugFindPkgMode() ||
    this->Makefile->IsOn("LINEARBUILD_FIND_DEBUG_MODE") ||
    this->Makefile->GetLinearBuildInstance()->GetDebugFindOutput();
}

bool cmFindCommon::ComputeIfDebugModeWanted(std::string const& var)
{
  return this->ComputeIfDebugModeWanted() ||
    this->Makefile->GetLinearBuildInstance()->GetDebugFindOutput(var);
}

bool cmFindCommon::ComputeIfImplicitDebugModeSuppressed()
{
  // XXX(find-events): In the future, mirror the `ComputeIfDebugModeWanted`
  // methods if more control is desired.
  return this->Makefile->IsOn(
    "LINEARBUILD_FIND_DEBUG_MODE_NO_IMPLICIT_CONFIGURE_LOG");
}

void cmFindCommon::InitializeSearchPathGroups()
{
  std::vector<PathLabel>* labels;

  // Define the various different groups of path types

  // All search paths
  labels = &this->PathGroupLabelMap[PathGroup::All];
  labels->push_back(PathLabel::PackageRoot);
  labels->push_back(PathLabel::LinearBuild);
  labels->push_back(PathLabel::LinearBuildEnvironment);
  labels->push_back(PathLabel::Hints);
  labels->push_back(PathLabel::SystemEnvironment);
  labels->push_back(PathLabel::LinearBuildSystem);
  labels->push_back(PathLabel::Guess);

  // Define the search group order
  this->PathGroupOrder.push_back(PathGroup::All);

  // Create the individual labeled search paths
  this->LabeledPaths.emplace(PathLabel::PackageRoot, cmSearchPath(this));
  this->LabeledPaths.emplace(PathLabel::LinearBuild, cmSearchPath(this));
  this->LabeledPaths.emplace(PathLabel::LinearBuildEnvironment, cmSearchPath(this));
  this->LabeledPaths.emplace(PathLabel::Hints, cmSearchPath(this));
  this->LabeledPaths.emplace(PathLabel::SystemEnvironment, cmSearchPath(this));
  this->LabeledPaths.emplace(PathLabel::LinearBuildSystem, cmSearchPath(this));
  this->LabeledPaths.emplace(PathLabel::Guess, cmSearchPath(this));
}

void cmFindCommon::SelectDefaultRootPathMode()
{
  // Check the policy variable for this find command type.
  std::string findRootPathVar =
    cmStrCat("LINEARBUILD_FIND_ROOT_PATH_MODE_", this->LinearBuildPathName);
  std::string rootPathMode =
    this->Makefile->GetSafeDefinition(findRootPathVar);
  if (rootPathMode == "NEVER") {
    this->FindRootPathMode = RootPathModeNever;
  } else if (rootPathMode == "ONLY") {
    this->FindRootPathMode = RootPathModeOnly;
  } else if (rootPathMode == "BOTH") {
    this->FindRootPathMode = RootPathModeBoth;
  }
}

void cmFindCommon::SelectDefaultMacMode()
{
  std::string ff = this->Makefile->GetSafeDefinition("LINEARBUILD_FIND_FRAMEWORK");
  if (ff == "NEVER") {
    this->SearchFrameworkLast = false;
    this->SearchFrameworkFirst = false;
    this->SearchFrameworkOnly = false;
  } else if (ff == "ONLY") {
    this->SearchFrameworkLast = false;
    this->SearchFrameworkFirst = false;
    this->SearchFrameworkOnly = true;
  } else if (ff == "FIRST") {
    this->SearchFrameworkLast = false;
    this->SearchFrameworkFirst = true;
    this->SearchFrameworkOnly = false;
  } else if (ff == "LAST") {
    this->SearchFrameworkLast = true;
    this->SearchFrameworkFirst = false;
    this->SearchFrameworkOnly = false;
  }

  std::string fab = this->Makefile->GetSafeDefinition("LINEARBUILD_FIND_APPBUNDLE");
  if (fab == "NEVER") {
    this->SearchAppBundleLast = false;
    this->SearchAppBundleFirst = false;
    this->SearchAppBundleOnly = false;
  } else if (fab == "ONLY") {
    this->SearchAppBundleLast = false;
    this->SearchAppBundleFirst = false;
    this->SearchAppBundleOnly = true;
  } else if (fab == "FIRST") {
    this->SearchAppBundleLast = false;
    this->SearchAppBundleFirst = true;
    this->SearchAppBundleOnly = false;
  } else if (fab == "LAST") {
    this->SearchAppBundleLast = true;
    this->SearchAppBundleFirst = false;
    this->SearchAppBundleOnly = false;
  }
}

void cmFindCommon::SelectDefaultSearchModes()
{
  std::array<std::pair<bool&, std::string>, 6> const search_paths = {
    { { this->NoPackageRootPath, "LINEARBUILD_FIND_USE_PACKAGE_ROOT_PATH" },
      { this->NoLinearBuildPath, "LINEARBUILD_FIND_USE_LINEARBUILD_PATH" },
      { this->NoLinearBuildEnvironmentPath,
        "LINEARBUILD_FIND_USE_LINEARBUILD_ENVIRONMENT_PATH" },
      { this->NoSystemEnvironmentPath,
        "LINEARBUILD_FIND_USE_SYSTEM_ENVIRONMENT_PATH" },
      { this->NoLinearBuildSystemPath, "LINEARBUILD_FIND_USE_LINEARBUILD_SYSTEM_PATH" },
      { this->NoLinearBuildInstallPath, "LINEARBUILD_FIND_USE_INSTALL_PREFIX" } }
  };

  for (auto const& path : search_paths) {
    cmValue def = this->Makefile->GetDefinition(path.second);
    if (def) {
      path.first = !def.IsOn();
    }
  }
}

void cmFindCommon::RerootPaths(std::vector<std::string>& paths,
                               std::string* debugBuffer)
{
  // Short-circuit if there is nothing to do.
  if (this->FindRootPathMode == RootPathModeNever) {
    return;
  }

  cmValue sysroot = this->Makefile->GetDefinition("LINEARBUILD_SYSROOT");
  cmValue sysrootCompile =
    this->Makefile->GetDefinition("LINEARBUILD_SYSROOT_COMPILE");
  cmValue sysrootLink = this->Makefile->GetDefinition("LINEARBUILD_SYSROOT_LINK");
  cmValue rootPath = this->Makefile->GetDefinition("LINEARBUILD_FIND_ROOT_PATH");
  bool const noSysroot = !cmNonempty(sysroot);
  bool const noCompileSysroot = !cmNonempty(sysrootCompile);
  bool const noLinkSysroot = !cmNonempty(sysrootLink);
  bool const noRootPath = !cmNonempty(rootPath);
  if (noSysroot && noCompileSysroot && noLinkSysroot && noRootPath) {
    return;
  }

  if (this->DebugModeEnabled() && debugBuffer) {
    *debugBuffer = cmStrCat(
      *debugBuffer, "Prepending the following roots to each prefix:\n");
  }

  auto debugRoot = [this, debugBuffer](std::string const& name,
                                       cmValue value) {
    if (this->DebugModeEnabled() && debugBuffer) {
      *debugBuffer = cmStrCat(*debugBuffer, name, '\n');
      cmList roots{ value };
      if (roots.empty()) {
        *debugBuffer = cmStrCat(*debugBuffer, "  none\n");
      }
      for (auto const& root : roots) {
        *debugBuffer = cmStrCat(*debugBuffer, "  ", root, '\n');
      }
    }
  };

  // Construct the list of path roots with no trailing slashes.
  cmList roots;
  debugRoot("LINEARBUILD_FIND_ROOT_PATH", rootPath);
  if (cmNonempty(rootPath)) {
    roots.assign(*rootPath);
  }
  debugRoot("LINEARBUILD_SYSROOT_COMPILE", sysrootCompile);
  if (cmNonempty(sysrootCompile)) {
    roots.emplace_back(*sysrootCompile);
  }
  debugRoot("LINEARBUILD_SYSROOT_LINK", sysrootLink);
  if (cmNonempty(sysrootLink)) {
    roots.emplace_back(*sysrootLink);
  }
  debugRoot("LINEARBUILD_SYSROOT", sysroot);
  if (cmNonempty(sysroot)) {
    roots.emplace_back(*sysroot);
  }
  for (auto& r : roots) {
    cmSystemTools::ConvertToUnixSlashes(r);
  }

  cmValue stagePrefix = this->Makefile->GetDefinition("LINEARBUILD_STAGING_PREFIX");

  // Copy the original set of unrooted paths.
  auto unrootedPaths = paths;
  paths.clear();

  auto isSameDirectoryOrSubDirectory = [](std::string const& l,
                                          std::string const& r) {
    std::string const lReal = cmSystemTools::GetRealPath(l);
    std::string const rReal = cmSystemTools::GetRealPath(r);
    return (lReal == rReal) || cmSystemTools::IsSubDirectory(lReal, rReal);
  };

  for (auto const& r : roots) {
    for (auto const& up : unrootedPaths) {
      // Place the unrooted path under the current root if it is not
      // already inside.  Skip the unrooted path if it is relative to
      // a user home directory or is empty.
      std::string rootedDir;
      if (isSameDirectoryOrSubDirectory(up, r) ||
          (stagePrefix && isSameDirectoryOrSubDirectory(up, *stagePrefix))) {
        rootedDir = up;
      } else if (!up.empty() && up[0] != '~') {
        auto const* split = cmSystemTools::SplitPathRootComponent(up);
        if (split && *split) {
          // Start with the new root.
          rootedDir = cmStrCat(r, '/', split);
        } else {
          rootedDir = r;
        }
      }

      // Store the new path.
      paths.push_back(rootedDir);
    }
  }

  // If searching both rooted and unrooted paths add the original
  // paths again.
  if (this->FindRootPathMode == RootPathModeBoth) {
    cm::append(paths, unrootedPaths);
  }
}

void cmFindCommon::GetIgnoredPaths(std::vector<std::string>& ignore)
{
  std::array<char const*, 2> const paths = { { "LINEARBUILD_SYSTEM_IGNORE_PATH",
                                               "LINEARBUILD_IGNORE_PATH" } };

  // Construct the list of path roots with no trailing slashes.
  for (char const* pathName : paths) {
    // Get the list of paths to ignore from the variable.
    cmList::append(ignore, this->Makefile->GetDefinition(pathName));
  }

  for (std::string& i : ignore) {
    cmSystemTools::ConvertToUnixSlashes(i);
  }
}

void cmFindCommon::GetIgnoredPaths(std::set<std::string>& ignore)
{
  std::vector<std::string> ignoreVec;
  this->GetIgnoredPaths(ignoreVec);
  ignore.insert(ignoreVec.begin(), ignoreVec.end());
}

void cmFindCommon::GetIgnoredPrefixPaths(std::vector<std::string>& ignore)
{
  std::array<char const*, 2> const paths = {
    { "LINEARBUILD_SYSTEM_IGNORE_PREFIX_PATH", "LINEARBUILD_IGNORE_PREFIX_PATH" }
  };
  // Construct the list of path roots with no trailing slashes.
  for (char const* pathName : paths) {
    // Get the list of paths to ignore from the variable.
    cmList::append(ignore, this->Makefile->GetDefinition(pathName));
  }

  for (std::string& i : ignore) {
    cmSystemTools::ConvertToUnixSlashes(i);
  }
}

void cmFindCommon::GetIgnoredPrefixPaths(std::set<std::string>& ignore)
{
  std::vector<std::string> ignoreVec;
  this->GetIgnoredPrefixPaths(ignoreVec);
  ignore.insert(ignoreVec.begin(), ignoreVec.end());
}

bool cmFindCommon::CheckCommonArgument(std::string const& arg)
{
  if (arg == "NO_DEFAULT_PATH") {
    this->NoDefaultPath = true;
    return true;
  }
  if (arg == "NO_PACKAGE_ROOT_PATH") {
    this->NoPackageRootPath = true;
    return true;
  }
  if (arg == "NO_LINEARBUILD_PATH") {
    this->NoLinearBuildPath = true;
    return true;
  }
  if (arg == "NO_LINEARBUILD_ENVIRONMENT_PATH") {
    this->NoLinearBuildEnvironmentPath = true;
    return true;
  }
  if (arg == "NO_SYSTEM_ENVIRONMENT_PATH") {
    this->NoSystemEnvironmentPath = true;
    return true;
  }
  if (arg == "NO_LINEARBUILD_SYSTEM_PATH") {
    this->NoLinearBuildSystemPath = true;
    return true;
  }
  if (arg == "NO_LINEARBUILD_INSTALL_PREFIX") {
    this->NoLinearBuildInstallPath = true;
    return true;
  }
  if (arg == "NO_LINEARBUILD_FIND_ROOT_PATH") {
    this->FindRootPathMode = RootPathModeNever;
    return true;
  }
  if (arg == "ONLY_LINEARBUILD_FIND_ROOT_PATH") {
    this->FindRootPathMode = RootPathModeOnly;
    return true;
  }
  if (arg == "LINEARBUILD_FIND_ROOT_PATH_BOTH") {
    this->FindRootPathMode = RootPathModeBoth;
    return true;
  }
  // The argument is not one of the above.
  return false;
}

void cmFindCommon::AddPathSuffix(std::string const& arg)
{
  std::string suffix = arg;

  // Strip leading and trailing slashes.
  if (suffix.empty()) {
    return;
  }
  if (suffix.front() == '/') {
    suffix = suffix.substr(1);
  }
  if (suffix.empty()) {
    return;
  }
  if (suffix.back() == '/') {
    suffix = suffix.substr(0, suffix.size() - 1);
  }
  if (suffix.empty()) {
    return;
  }

  // Store the suffix.
  this->SearchPathSuffixes.push_back(std::move(suffix));
}

void cmFindCommon::ComputeFinalPaths(IgnorePaths ignorePaths,
                                     std::string* debugBuffer)
{
  // Filter out ignored paths from the prefix list
  std::set<std::string> ignoredPaths;
  std::set<std::string> ignoredPrefixes;
  if (ignorePaths == IgnorePaths::Yes) {
    this->GetIgnoredPaths(ignoredPaths);
    this->GetIgnoredPrefixPaths(ignoredPrefixes);
  }

  // Combine the separate path types, filtering out ignores
  this->SearchPaths.clear();
  std::vector<PathLabel>& allLabels = this->PathGroupLabelMap[PathGroup::All];
  for (PathLabel const& l : allLabels) {
    this->LabeledPaths[l].ExtractWithout(ignoredPaths, ignoredPrefixes,
                                         this->SearchPaths);
  }

  // Expand list of paths inside all search roots.
  this->RerootPaths(this->SearchPaths, debugBuffer);

  // Add a trailing slash to all paths to aid the search process.
  std::for_each(this->SearchPaths.begin(), this->SearchPaths.end(),
                [](std::string& s) {
                  if (!s.empty() && s.back() != '/') {
                    s += '/';
                  }
                });
}

cmFindCommonDebugState::cmFindCommonDebugState(std::string name,
                                               cmFindCommon const* findCommand)
  : FindCommand(findCommand)
  , CommandName(std::move(name))
  // Strip the `find_` prefix.
  , Mode(this->CommandName.substr(5))
{
}

void cmFindCommonDebugState::FoundAt(std::string const& path,
                                     std::string regexName)
{
  this->IsFound = true;

  if (!this->TrackSearchProgress()) {
    return;
  }

  this->FoundAtImpl(path, regexName);
}

void cmFindCommonDebugState::FailedAt(std::string const& path,
                                      std::string regexName)
{
  if (!this->TrackSearchProgress()) {
    return;
  }

  this->FailedAtImpl(path, regexName);
}

bool cmFindCommonDebugState::ShouldImplicitlyLogEvents() const
{
  return true;
}

void cmFindCommonDebugState::Write()
{
  auto const* const fc = this->FindCommand;

#ifndef LINEARBUILD_BOOTSTRAP
  // Write find event to the configure log if the log exists
  if (cmConfigureLog* log =
        fc->Makefile->GetLinearBuildInstance()->GetConfigureLog()) {
    // Write event if any of:
    //   - debug mode is enabled
    //   - implicit logging should happen and:
    //     - the variable was not defined (first run)
    //     - the variable found state does not match the new found state (state
    //       transition)
    if (fc->DebugModeEnabled() ||
        (this->ShouldImplicitlyLogEvents() &&
         (!fc->IsDefined() || fc->IsFound() != this->IsFound))) {
      this->WriteEvent(*log, *fc->Makefile);
    }
  }
#endif

  if (fc->DebugModeEnabled()) {
    this->WriteDebug();
  }
}

#ifndef LINEARBUILD_BOOTSTRAP
void cmFindCommonDebugState::WriteSearchVariables(cmConfigureLog& log,
                                                  cmMakefile const& mf) const
{
  auto WriteString = [&log, &mf](std::string const& name) {
    if (cmValue value = mf.GetDefinition(name)) {
      log.WriteValue(name, *value);
    }
  };
  auto WriteLinearBuildList = [&log, &mf](std::string const& name) {
    if (cmValue value = mf.GetDefinition(name)) {
      cmList values{ *value };
      if (!values.empty()) {
        log.WriteValue(name, values);
      }
    }
  };
  auto WriteEnvList = [&log](std::string const& name) {
    if (auto value = cmSystemTools::GetEnvVar(name)) {
      auto values = cmSystemTools::SplitEnvPath(*value);
      if (!values.empty()) {
        log.WriteValue(cmStrCat("ENV{", name, '}'), values);
      }
    }
  };

  auto const* fc = this->FindCommand;
  log.BeginObject("search_context"_s);
  auto const& packageRootStack = mf.FindPackageRootPathStack;
  if (!packageRootStack.empty()) {
    bool havePaths =
      std::any_of(packageRootStack.begin(), packageRootStack.end(),
                  [](std::vector<std::string> const& entry) -> bool {
                    return !entry.empty();
                  });
    if (havePaths) {
      log.BeginObject("package_stack");
      log.BeginArray();
      for (auto const& pkgPaths : cmReverseRange(packageRootStack)) {
        if (!pkgPaths.empty()) {
          log.NextArrayElement();
          log.WriteValue("package_paths", pkgPaths);
        }
      }
      log.EndArray();
      log.EndObject();
    }
  }
  auto linearbuildPathVar = cmStrCat("LINEARBUILD_", fc->LinearBuildPathName, "_PATH");
  WriteLinearBuildList(linearbuildPathVar);
  WriteLinearBuildList("LINEARBUILD_PREFIX_PATH");
  if (fc->LinearBuildPathName == "PROGRAM"_s) {
    WriteLinearBuildList("LINEARBUILD_APPBUNDLE_PATH");
  } else {
    WriteLinearBuildList("LINEARBUILD_FRAMEWORK_PATH");
  }
  // Same as above, but ask the environment instead.
  WriteEnvList(linearbuildPathVar);
  WriteEnvList("LINEARBUILD_PREFIX_PATH");
  if (fc->LinearBuildPathName == "PROGRAM"_s) {
    WriteEnvList("LINEARBUILD_APPBUNDLE_PATH");
  } else {
    WriteEnvList("LINEARBUILD_FRAMEWORK_PATH");
  }
  WriteEnvList("PATH");
  WriteString("LINEARBUILD_INSTALL_PREFIX");
  WriteString("LINEARBUILD_STAGING_PREFIX");
  WriteLinearBuildList("LINEARBUILD_SYSTEM_PREFIX_PATH");
  auto systemPathVar = cmStrCat("LINEARBUILD_SYSTEM_", fc->LinearBuildPathName, "_PATH");
  WriteLinearBuildList(systemPathVar);
  // Sysroot paths.
  WriteString("LINEARBUILD_SYSROOT");
  WriteString("LINEARBUILD_SYSROOT_COMPILE");
  WriteString("LINEARBUILD_SYSROOT_LINK");
  WriteString("LINEARBUILD_FIND_ROOT_PATH");
  // Write out paths which are ignored.
  WriteLinearBuildList("LINEARBUILD_IGNORE_PATH");
  WriteLinearBuildList("LINEARBUILD_IGNORE_PREFIX_PATH");
  WriteLinearBuildList("LINEARBUILD_SYSTEM_IGNORE_PATH");
  WriteLinearBuildList("LINEARBUILD_SYSTEM_IGNORE_PREFIX_PATH");
  if (fc->LinearBuildPathName == "PROGRAM"_s) {
    WriteLinearBuildList("LINEARBUILD_SYSTEM_APPBUNDLE_PATH");
  } else {
    WriteLinearBuildList("LINEARBUILD_SYSTEM_FRAMEWORK_PATH");
  }
  for (auto const& extraVar : this->ExtraSearchVariables()) {
    switch (extraVar.first) {
      case VariableSource::String:
        WriteString(extraVar.second);
        break;
      case VariableSource::PathList:
        WriteLinearBuildList(extraVar.second);
        break;
      case VariableSource::EnvironmentList:
        WriteEnvList(extraVar.second);
        break;
    }
  }
  log.EndObject();
}

std::vector<std::pair<cmFindCommonDebugState::VariableSource, std::string>>
cmFindCommonDebugState::ExtraSearchVariables() const
{
  return {};
}
#endif

bool cmFindCommonDebugState::TrackSearchProgress() const
{
  // Track search progress if debugging or logging the configure.
  return this->FindCommand->DebugModeEnabled()
#ifndef LINEARBUILD_BOOTSTRAP
    || this->FindCommand->Makefile->GetLinearBuildInstance()->GetConfigureLog()
#endif
    ;
}
