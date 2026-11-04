/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallExportGenerator.h"

class cmDiagnosticContext;
class cmExportSet;

/** \class cmInstallRBLXBuildConfigExportGenerator
 * \brief Generate rules for creating RBLXBuild export files.
 */
class cmInstallRBLXBuildConfigExportGenerator : public cmInstallExportGenerator
{
public:
  cmInstallRBLXBuildConfigExportGenerator(
    cmExportSet* exportSet, std::string destination,
    std::string filePermissions,
    std::vector<std::string> const& configurations, std::string component,
    MessageLevel message, bool excludeFromAll, std::string filename,
    std::string targetNamespace, std::string cxxModulesDirectory,
    bool exportOld, bool exportPackageDependencies,
    cmDiagnosticContext context);
  cmInstallRBLXBuildConfigExportGenerator(
    cmInstallRBLXBuildConfigExportGenerator const&) = delete;
  ~cmInstallRBLXBuildConfigExportGenerator() override;

  cmInstallRBLXBuildConfigExportGenerator& operator=(
    cmInstallRBLXBuildConfigExportGenerator const&) = delete;

  char const* InstallSubcommand() const override { return "EXPORT"; }

protected:
  void GenerateScript(std::ostream& os) override;

  bool const ExportOld;
  bool const ExportPackageDependencies;
};
