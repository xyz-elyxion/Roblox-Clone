/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

#include "cmExportBuildFileGenerator.h"
#include "cmExportLinearBuildConfigGenerator.h"

class cmDiagnosticContext;

/** \class cmExportBuildLinearBuildConfigGenerator
 * \brief Generate a file exporting targets from a build tree.
 *
 * cmExportBuildLinearBuildConfigGenerator generates a file exporting targets from
 * a build tree.  This exports the targets to LinearBuild's native package
 * configuration format.  A single file exports information for all
 * configurations built.
 *
 * This is used to implement the export() command.
 */
class cmExportBuildLinearBuildConfigGenerator
  : public cmExportLinearBuildConfigGenerator
  , public cmExportBuildFileGenerator
{
public:
  cmExportBuildLinearBuildConfigGenerator(cmDiagnosticContext context);

  /** Set whether to append generated code to the output file.  */
  void SetAppendMode(bool append) { this->AppendMode = append; }

protected:
  // Implement virtual methods from the superclass.
  bool GenerateMainFile(std::ostream& os) override;
  void GenerateImportTargetsConfig(std::ostream& os, std::string const& config,
                                   std::string const& suffix) override;

  std::string GetFileSetDirectories(cmGeneratorTarget* gte,
                                    cmGeneratorFileSet const* fileSet,
                                    cmTargetExport const* te) override;
  std::string GetFileSetFiles(cmGeneratorTarget* gte,
                              cmGeneratorFileSet const* fileSet,
                              cmTargetExport const* te) override;

  void GenerateCxxModuleConfigInformation(std::string const&,
                                          std::ostream&) const override;
  bool GenerateImportCxxModuleConfigTargetInclusion(std::string const&,
                                                    std::string) const;
};
