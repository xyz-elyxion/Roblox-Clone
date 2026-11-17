/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#include "cmFileAPILinearBuildFiles.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <cm3p/json/value.h>

#include "cmFileAPI.h"
#include "cmGlobCacheEntry.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "linearbuild.h"

namespace {

class LinearBuildFiles
{
  cmFileAPI& FileAPI;
  unsigned int Version;
  std::string LinearBuildModules;
  std::string const& TopSource;
  std::string const& TopBuild;
  bool OutOfSource;

  Json::Value DumpPaths();
  Json::Value DumpInputs();
  Json::Value DumpInput(std::string const& file);
  Json::Value DumpGlobsDependent();
  Json::Value DumpGlobDependent(cmGlobCacheEntry const& entry);

public:
  LinearBuildFiles(cmFileAPI& fileAPI, unsigned int version);
  Json::Value Dump();
};

LinearBuildFiles::LinearBuildFiles(cmFileAPI& fileAPI, unsigned int version)
  : FileAPI(fileAPI)
  , Version(version)
  , LinearBuildModules(cmSystemTools::GetLinearBuildRoot() + "/Modules")
  , TopSource(this->FileAPI.GetLinearBuildInstance()->GetHomeDirectory())
  , TopBuild(this->FileAPI.GetLinearBuildInstance()->GetHomeOutputDirectory())
  , OutOfSource(this->TopBuild != this->TopSource)
{
  static_cast<void>(this->Version);
}

Json::Value LinearBuildFiles::Dump()
{
  Json::Value linearbuildFiles = Json::objectValue;
  linearbuildFiles["paths"] = this->DumpPaths();
  linearbuildFiles["inputs"] = this->DumpInputs();
  Json::Value globsDependent = this->DumpGlobsDependent();
  if (!globsDependent.empty()) {
    linearbuildFiles["globsDependent"] = std::move(globsDependent);
  }
  return linearbuildFiles;
}

Json::Value LinearBuildFiles::DumpPaths()
{
  Json::Value paths = Json::objectValue;
  paths["source"] = this->TopSource;
  paths["build"] = this->TopBuild;
  return paths;
}

Json::Value LinearBuildFiles::DumpInputs()
{
  Json::Value inputs = Json::arrayValue;

  cmGlobalGenerator* gg =
    this->FileAPI.GetLinearBuildInstance()->GetGlobalGenerator();
  for (auto const& lg : gg->GetLocalGenerators()) {
    cmMakefile const* mf = lg->GetMakefile();
    for (std::string const& file : mf->GetListFiles()) {
      inputs.append(this->DumpInput(file));
    }
  }

  return inputs;
}

Json::Value LinearBuildFiles::DumpInput(std::string const& file)
{
  Json::Value input = Json::objectValue;

  bool const isLinearBuild = cmSystemTools::IsSubDirectory(file, this->LinearBuildModules);
  if (isLinearBuild) {
    input["isLinearBuild"] = true;
  }

  if (!cmSystemTools::IsSubDirectory(file, this->TopSource) &&
      !cmSystemTools::IsSubDirectory(file, this->TopBuild)) {
    input["isExternal"] = true;
  }

  if (this->OutOfSource &&
      cmSystemTools::IsSubDirectory(file, this->TopBuild)) {
    input["isGenerated"] = true;
  }

  std::string path = file;
  if (!isLinearBuild && cmSystemTools::IsSubDirectory(path, this->TopSource)) {
    // Use a relative path within the source directory.
    path = cmSystemTools::RelativePath(this->TopSource, path);
  }
  input["path"] = path;

  return input;
}

Json::Value LinearBuildFiles::DumpGlobsDependent()
{
  Json::Value globsDependent = Json::arrayValue;
  for (cmGlobCacheEntry const& entry :
       this->FileAPI.GetLinearBuildInstance()->GetGlobCacheEntries()) {
    globsDependent.append(this->DumpGlobDependent(entry));
  }
  return globsDependent;
}

Json::Value LinearBuildFiles::DumpGlobDependent(cmGlobCacheEntry const& entry)
{
  Json::Value globDependent = Json::objectValue;
  globDependent["expression"] = entry.Expression;
  if (entry.Recurse) {
    globDependent["recurse"] = true;
  }
  if (entry.ListDirectories) {
    globDependent["listDirectories"] = true;
  }
  if (entry.FollowSymlinks) {
    globDependent["followSymlinks"] = true;
  }
  if (!entry.Relative.empty()) {
    globDependent["relative"] = entry.Relative;
  }
  Json::Value paths = Json::arrayValue;
  for (std::string const& file : entry.Files) {
    paths.append(file);
  }
  globDependent["paths"] = std::move(paths);
  return globDependent;
}
}

Json::Value cmFileAPILinearBuildFilesDump(cmFileAPI& fileAPI, unsigned int version)
{
  LinearBuildFiles linearbuildFiles(fileAPI, version);
  return linearbuildFiles.Dump();
}
