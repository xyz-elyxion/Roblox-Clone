/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallExportGenerator.h"

class cmDiagnosticContext;
class cmExportSet;

/** \class cmInstallLinearBuildConfigExportGenerator
 * \brief Generate rules for creating LinearBuild export files.
 */
class cmInstallLinearBuildConfigExportGenerator : public cmInstallExportGenerator
{
public:
  cmInstallLinearBuildConfigExportGenerator(
    cmExportSet* exportSet, std::string destination,
    std::string filePermissions,
    std::vector<std::string> const& configurations, std::string component,
    MessageLevel message, bool excludeFromAll, std::string filename,
    std::string targetNamespace, std::string cxxModulesDirectory,
    bool exportOld, bool exportPackageDependencies,
    cmDiagnosticContext context);
  cmInstallLinearBuildConfigExportGenerator(
    cmInstallLinearBuildConfigExportGenerator const&) = delete;
  ~cmInstallLinearBuildConfigExportGenerator() override;

  cmInstallLinearBuildConfigExportGenerator& operator=(
    cmInstallLinearBuildConfigExportGenerator const&) = delete;

  char const* InstallSubcommand() const override { return "EXPORT"; }

protected:
  void GenerateScript(std::ostream& os) override;

  bool const ExportOld;
  bool const ExportPackageDependencies;
};
