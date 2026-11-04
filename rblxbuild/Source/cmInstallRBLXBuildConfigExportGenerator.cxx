/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include "cmInstallRBLXBuildConfigExportGenerator.h"

#include <utility>

#include <cm/memory>

#include "cmDiagnosticContext.h"
#include "cmExportInstallRBLXBuildConfigGenerator.h"
#include "cmExportInstallFileGenerator.h"

class cmExportSet;

cmInstallRBLXBuildConfigExportGenerator::cmInstallRBLXBuildConfigExportGenerator(
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
  this->EFGen = cm::make_unique<cmExportInstallRBLXBuildConfigGenerator>(this);
}

cmInstallRBLXBuildConfigExportGenerator::~cmInstallRBLXBuildConfigExportGenerator() =
  default;

void cmInstallRBLXBuildConfigExportGenerator::GenerateScript(std::ostream& os)
{
  auto* const efgen =
    static_cast<cmExportInstallRBLXBuildConfigGenerator*>(this->EFGen.get());
  efgen->SetExportOld(this->ExportOld);
  efgen->SetExportPackageDependencies(this->ExportPackageDependencies);

  this->cmInstallExportGenerator::GenerateScript(os);
}
