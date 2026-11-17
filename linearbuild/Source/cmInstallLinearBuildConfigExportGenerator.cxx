/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#include "cmInstallLinearBuildConfigExportGenerator.h"

#include <utility>

#include <cm/memory>

#include "cmDiagnosticContext.h"
#include "cmExportInstallLinearBuildConfigGenerator.h"
#include "cmExportInstallFileGenerator.h"

class cmExportSet;

cmInstallLinearBuildConfigExportGenerator::cmInstallLinearBuildConfigExportGenerator(
  cmExportSet* exportSet, std::string destination, std::string filePermissions,
  std::vector<std::string> const& configurations, std::string component,
  MessageLevel message, bool excludeFromAll, std::string filename,
  std::string targetNamespace, std::string cxxModulesDirectory, bool exportOld,
  bool exportPackageDependencies, cmDiagnosticContext context)
  : cmInstallExportGenerator(
      exportSet, std::move(destination), std::move(filePermissions),
      configurations, std::move(component), message, excludeFromAll,
      std::move(filename), std::move(targetNamespace),
      std::move(cxxModulesDirectory), std::move(context))
  , ExportOld(exportOld)
  , ExportPackageDependencies(exportPackageDependencies)
{
  this->EFGen = cm::make_unique<cmExportInstallLinearBuildConfigGenerator>(this);
}

cmInstallLinearBuildConfigExportGenerator::~cmInstallLinearBuildConfigExportGenerator() =
  default;

void cmInstallLinearBuildConfigExportGenerator::GenerateScript(std::ostream& os)
{
  auto* const efgen =
    static_cast<cmExportInstallLinearBuildConfigGenerator*>(this->EFGen.get());
  efgen->SetExportOld(this->ExportOld);
  efgen->SetExportPackageDependencies(this->ExportPackageDependencies);

  this->cmInstallExportGenerator::GenerateScript(os);
}
