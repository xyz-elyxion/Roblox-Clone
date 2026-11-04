/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include "cmFileAPIRBLXBuildFiles.h"

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
#include "rblxbuild.h"

namespace {

class RBLXBuildFiles
{
  cmFileAPI& FileAPI;
  unsigned int Version;
  std::string RBLXBuildModules;
  std::string const& TopSource;
  std::string const& TopBuild;
  bool OutOfSource;

  Json::Value DumpPaths();
  Json::Value DumpInputs();
  Json::Value DumpInput(std::string const& file);
  Json::Value DumpGlobsDependent();
  Json::Value DumpGlobDependent(cmGlobCacheEntry const& entry);

public:
  RBLXBuildFiles(cmFileAPI& fileAPI, unsigned int version);
  Json::Value Dump();
};

RBLXBuildFiles::RBLXBuildFiles(cmFileAPI& fileAPI, unsigned int version)
  : FileAPI(fileAPI)
  , Version(version)
  , RBLXBuildModules(cmSystemTools::GetRBLXBuildRoot() + "/Modules")
  , TopSource(this->FileAPI.GetRBLXBuildInstance()->GetHomeDirectory())
  , TopBuild(this->FileAPI.GetRBLXBuildInstance()->GetHomeOutputDirectory())
  , OutOfSource(this->TopBuild != this->TopSource)
{
  static_cast<void>(this->Version);
}

Json::Value RBLXBuildFiles::Dump()
{
  Json::Value rblxbuildFiles = Json::objectValue;
  rblxbuildFiles["paths"] = this->DumpPaths();
  rblxbuildFiles["inputs"] = this->DumpInputs();
  Json::Value globsDependent = this->DumpGlobsDependent();
  if (!globsDependent.empty()) {
    rblxbuildFiles["globsDependent"] = std::move(globsDependent);
  }
  return rblxbuildFiles;
}

Json::Value RBLXBuildFiles::DumpPaths()
{
  Json::Value paths = Json::objectValue;
  paths["source"] = this->TopSource;
  paths["build"] = this->TopBuild;
  return paths;
}

Json::Value RBLXBuildFiles::DumpInputs()
{
  Json::Value inputs = Json::arrayValue;

  cmGlobalGenerator* gg =
    this->FileAPI.GetRBLXBuildInstance()->GetGlobalGenerator();
  for (auto const& lg : gg->GetLocalGenerators()) {
    cmMakefile const* mf = lg->GetMakefile();
    for (std::string const& file : mf->GetListFiles()) {
      inputs.append(this->DumpInput(file));
    }
  }

  return inputs;
}

Json::Value RBLXBuildFiles::DumpInput(std::string const& file)
{
  Json::Value input = Json::objectValue;

  bool const isRBLXBuild = cmSystemTools::IsSubDirectory(file, this->RBLXBuildModules);
  if (isRBLXBuild) {
    input["isRBLXBuild"] = true;
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
  if (!isRBLXBuild && cmSystemTools::IsSubDirectory(path, this->TopSource)) {
    // Use a relative path within the source directory.
    path = cmSystemTools::RelativePath(this->TopSource, path);
  }
  input["path"] = path;

  return input;
}

Json::Value RBLXBuildFiles::DumpGlobsDependent()
{
  Json::Value globsDependent = Json::arrayValue;
  for (cmGlobCacheEntry const& entry :
       this->FileAPI.GetRBLXBuildInstance()->GetGlobCacheEntries()) {
    globsDependent.append(this->DumpGlobDependent(entry));
  }
  return globsDependent;
}

Json::Value RBLXBuildFiles::DumpGlobDependent(cmGlobCacheEntry const& entry)
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

Json::Value cmFileAPIRBLXBuildFilesDump(cmFileAPI& fileAPI, unsigned int version)
{
  RBLXBuildFiles rblxbuildFiles(fileAPI, version);
  return rblxbuildFiles.Dump();
}
