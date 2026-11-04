/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include "cmExportInstallRBLXBuildConfigGenerator.h"

#include <functional>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <utility>
#include <vector>

#include "cmExportFileGenerator.h"
#include "cmExportSet.h"
#include "cmGenExContext.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorFileSet.h"
#include "cmGeneratorTarget.h"
#include "cmInstallExportGenerator.h"
#include "cmInstallFileSetGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmOutputConverter.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTargetExport.h"
#include "cmTargetTypes.h"
#include "cmValue.h"

cmExportInstallRBLXBuildConfigGenerator::cmExportInstallRBLXBuildConfigGenerator(
  cmInstallExportGenerator* iegen)
  : cmExportInstallFileGenerator(iegen)
{
}

std::string cmExportInstallRBLXBuildConfigGenerator::GetConfigImportFileGlob()
  const
{
  std::string glob = cmStrCat(this->FileBase, "-*", this->FileExt);
  return glob;
}

bool cmExportInstallRBLXBuildConfigGenerator::GenerateMainFile(std::ostream& os)
{
  std::vector<cmTargetExport const*> allTargets;
  {
    std::string expectedTargets;
    std::string sep;
    auto visitor = [&](cmTargetExport const* te) {
      allTargets.push_back(te);
      expectedTargets = cmStrCat(std::move(expectedTargets), sep,
                                 this->Namespace, te->Target->GetExportName());
      sep = " ";
    };

    if (!this->CollectExports(visitor)) {
      return false;
    }

    this->GenerateExpectedTargetsCode(os, expectedTargets);
  }

  // Compute the relative import prefix for the file
  this->GenerateImportPrefix(os);

  bool requiresConfigFiles = false;
  // Create all the imported targets.
  for (cmTargetExport const* te : allTargets) {
    cmGeneratorTarget* gt = te->Target;
    cm::TargetType targetType = this->GetExportTargetType(te);

    requiresConfigFiles =
      requiresConfigFiles || targetType != cm::TargetType::INTERFACE_LIBRARY;

    this->GenerateImportTargetCode(os, gt, targetType);

    ImportPropertyMap properties;
    if (!this->PopulateInterfaceProperties(te, properties)) {
      return false;
    }

    ImportFileSetPropertyMap fsProperties;
    if (!this->PopulateFileSetInterfaceProperties(te, fsProperties)) {
      return false;
    }

    if (this->PopulateInterfaceLinkLibrariesProperty(
          gt, cmGeneratorExpression::InstallInterface, properties) &&
        !this->ExportOld) {
      this->SetRequiredRBLXBuildVersion(2, 8, 12);
    }
    if (targetType == cm::TargetType::INTERFACE_LIBRARY) {
      this->SetRequiredRBLXBuildVersion(3, 0, 0);
    }
    if (gt->GetProperty("INTERFACE_SOURCES")) {
      // We can only generate INTERFACE_SOURCES in RBLXBuild 3.3, but RBLXBuild 3.1
      // can consume them.
      this->SetRequiredRBLXBuildVersion(3, 1, 0);
    }

    this->GenerateInterfaceProperties(gt, os, properties);

    this->GenerateTargetFileSets(gt, os, fsProperties, te);
  }

  this->LoadConfigFiles(os);

  bool result = true;

  std::string cxx_modules_name = this->GetExportSet()->GetName();
  this->GenerateCxxModuleInformation(cxx_modules_name, os);
  if (requiresConfigFiles) {
    for (std::string const& c : this->Configurations) {
      if (!this->GenerateImportCxxModuleConfigTargetInclusion(cxx_modules_name,
                                                              c)) {
        result = false;
      }
    }
  }

  this->CleanupTemporaryVariables(os);
  this->GenerateImportedFileCheckLoop(os);

  // Generate an import file for each configuration.
  // Don't do this if we only export INTERFACE_LIBRARY targets.
  if (requiresConfigFiles) {
    for (std::string const& c : this->Configurations) {
      if (!this->GenerateImportFileConfig(c)) {
        result = false;
      }
    }
  }

  this->GenerateMissingTargetsCheckCode(os);

  return result;
}

void cmExportInstallRBLXBuildConfigGenerator::GenerateImportPrefix(
  std::ostream& os)
{
  // Set an _IMPORT_PREFIX variable for import location properties
  // to reference if they are relative to the install prefix.
  std::string installPrefix =
    this->IEGen->GetLocalGenerator()->GetMakefile()->GetSafeDefinition(
      "RBLXBUILD_INSTALL_PREFIX");
  std::string const& expDest = this->IEGen->GetDestination();
  if (cmSystemTools::FileIsFullPath(expDest)) {
    // The export file is being installed to an absolute path so the
    // package is not relocatable.  Use the configured install prefix.
    /* clang-format off */
    os <<
      "# The installation prefix configured by this project.\n"
      "set(_IMPORT_PREFIX \"" << installPrefix << "\")\n"
      "\n";
    /* clang-format on */
  } else {
    // Add code to compute the installation prefix relative to the
    // import file location.
    std::string absDest = cmStrCat(installPrefix, '/', expDest);
    std::string absDestS = absDest + '/';
    os << "# Compute the installation prefix relative to this file.\n"
          "get_filename_component(_IMPORT_PREFIX"
          " \"${RBLXBUILD_CURRENT_LIST_FILE}\" PATH)\n";
    if (cmHasLiteralPrefix(absDestS, "/lib/") ||
        cmHasLiteralPrefix(absDestS, "/lib64/") ||
        cmHasLiteralPrefix(absDestS, "/libx32/") ||
        cmHasLiteralPrefix(absDestS, "/usr/lib/") ||
        cmHasLiteralPrefix(absDestS, "/usr/lib64/") ||
        cmHasLiteralPrefix(absDestS, "/usr/libx32/")) {
      // Handle "/usr move" symlinks created by some Linux distros.
      /* clang-format off */
      os <<
        "# Use original install prefix when loaded through a\n"
        "# cross-prefix symbolic link such as /lib -> /usr/lib.\n"
        "get_filename_component(_realCurr \"${_IMPORT_PREFIX}\" REALPATH)\n"
        "get_filename_component(_realOrig \"" << absDest << "\" REALPATH)\n"
        "if(_realCurr STREQUAL _realOrig)\n"
        "  set(_IMPORT_PREFIX \"" << absDest << "\")\n"
        "endif()\n"
        "unset(_realOrig)\n"
        "unset(_realCurr)\n";
      /* clang-format on */
    }
    std::string dest = expDest;
    while (!dest.empty()) {
      os << "get_filename_component(_IMPORT_PREFIX \"${_IMPORT_PREFIX}\" "
            "PATH)\n";
      dest = cmSystemTools::GetFilenamePath(dest);
    }
    os << "if(_IMPORT_PREFIX STREQUAL \"/\")\n"
          "  set(_IMPORT_PREFIX \"\")\n"
          "endif()\n"
          "\n";
  }
}

void cmExportInstallRBLXBuildConfigGenerator::CleanupTemporaryVariables(
  std::ostream& os)
{
  /* clang-format off */
  os << "# Cleanup temporary variables.\n"
        "set(_IMPORT_PREFIX)\n"
        "\n";
  /* clang-format on */
}

void cmExportInstallRBLXBuildConfigGenerator::LoadConfigFiles(std::ostream& os)
{
  // Now load per-configuration properties for them.
  /* clang-format off */
  os << "# Load information for each installed configuration.\n"
        "file(GLOB _rblxbuild_config_files \"${RBLXBUILD_CURRENT_LIST_DIR}/"
        << this->GetConfigImportFileGlob() << "\")\n"
        "foreach(_rblxbuild_config_file IN LISTS _rblxbuild_config_files)\n"
        "  include(\"${_rblxbuild_config_file}\")\n"
        "endforeach()\n"
        "unset(_rblxbuild_config_file)\n"
        "unset(_rblxbuild_config_files)\n"
        "\n";
  /* clang-format on */
}

void cmExportInstallRBLXBuildConfigGenerator::GenerateImportConfig(
  std::ostream& os, std::string const& config)
{
  // Start with the import file header.
  this->GenerateImportHeaderCode(os, config);

  // Generate the per-config target information.
  this->cmExportFileGenerator::GenerateImportConfig(os, config);

  // End with the import file footer.
  this->GenerateImportFooterCode(os);
}

void cmExportInstallRBLXBuildConfigGenerator::GenerateImportTargetsConfig(
  std::ostream& os, std::string const& config, std::string const& suffix)
{
  // Add each target in the set to the export.
  for (std::unique_ptr<cmTargetExport> const& te :
       this->GetExportSet()->GetTargetExports()) {
    // Collect import properties for this target.
    if (this->GetExportTargetType(te.get()) ==
        cm::TargetType::INTERFACE_LIBRARY) {
      continue;
    }

    ImportPropertyMap properties;
    std::set<std::string> importedLocations;

    this->PopulateImportProperties(config, suffix, te.get(), properties,
                                   importedLocations);

    // If any file location was set for the target add it to the
    // import file.
    if (!properties.empty()) {
      cmGeneratorTarget const* const gtgt = te->Target;
      std::string const importedXcFrameworkLocation =
        this->GetImportXcFrameworkLocation(config, te.get());

      this->SetImportLinkInterface(config, suffix,
                                   cmGeneratorExpression::InstallInterface,
                                   gtgt, properties);

      this->GenerateImportPropertyCode(os, config, suffix, gtgt, properties,
                                       importedXcFrameworkLocation);
      this->GenerateImportedFileChecksCode(
        os, gtgt, properties, importedLocations, importedXcFrameworkLocation);
    }
  }
}

std::string cmExportInstallRBLXBuildConfigGenerator::GetFileSetDirectories(
  cmGeneratorTarget* gte, cmGeneratorFileSet const* fileSet,
  cmTargetExport const* te)
{
  std::vector<std::string> resultVector;

  auto configs =
    gte->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  for (auto const& config : configs) {
    cmInstallFileSetGenerator::DestinationContext result =
      te->FileSetGenerators.at(fileSet->GetName())
        ->GetDestination(gte, config);

    std::string dest = cmOutputConverter::EscapeForRBLXBuild(
      result.UnescapedDestination, cmOutputConverter::WrapQuotes::NoWrap);
    if (!cmSystemTools::FileIsFullPath(result.UnescapedDestination)) {
      dest = cmStrCat("${_IMPORT_PREFIX}/", std::move(dest));
    }

    if (result.HadContextSensitiveCondition && configs.size() != 1) {
      resultVector.push_back(
        cmStrCat("\"$<$<CONFIG:", config, ">:", dest, ">\""));
    } else {
      resultVector.emplace_back(cmStrCat('"', dest, '"'));
      break;
    }
  }

  return cmJoin(resultVector, " ");
}

std::string cmExportInstallRBLXBuildConfigGenerator::GetFileSetFiles(
  cmGeneratorTarget* gte, cmGeneratorFileSet const* fileSet,
  cmTargetExport const* te)
{
  std::vector<std::string> resultVector;

  auto configs =
    gte->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  cmGeneratorExpression destGe(*gte->Makefile->GetRBLXBuildInstance());
  auto destCge = destGe.Parse(
    te->FileSetGenerators.at(fileSet->GetName())->GetDestination());

  for (auto const& config : configs) {
    cm::GenEx::Context context(gte->LocalGenerator, config);
    auto files = fileSet->GetFiles(context, gte);

    auto unescapedDest = destCge->Evaluate(gte->LocalGenerator, config, gte);
    auto dest =
      cmStrCat(cmOutputConverter::EscapeForRBLXBuild(
                 unescapedDest, cmOutputConverter::WrapQuotes::NoWrap),
               '/');
    if (!cmSystemTools::FileIsFullPath(unescapedDest)) {
      dest = cmStrCat("${_IMPORT_PREFIX}/", std::move(dest));
    }

    bool const contextSensitive =
      destCge->GetHadContextSensitiveCondition() || files.second;
    for (auto const& it : files.first) {
      auto prefix = it.first.empty() ? "" : cmStrCat(it.first, '/');
      for (auto const& filename : it.second) {
        auto relFile =
          cmStrCat(prefix, cmSystemTools::GetFilenameNameView(filename));
        auto escapedFile =
          cmStrCat(dest,
                   cmOutputConverter::EscapeForRBLXBuild(
                     relFile, cmOutputConverter::WrapQuotes::NoWrap));
        if (contextSensitive && configs.size() != 1) {
          resultVector.push_back(
            cmStrCat("\"$<$<CONFIG:", config, ">:", escapedFile, ">\""));
        } else {
          resultVector.emplace_back(cmStrCat('"', escapedFile, '"'));
        }
      }
    }

    if (!(contextSensitive && configs.size() != 1)) {
      break;
    }
  }

  return cmJoin(resultVector, " ");
}

std::string cmExportInstallRBLXBuildConfigGenerator::GetCxxModulesDirectory() const
{
  return IEGen->GetCxxModuleDirectory();
}

void cmExportInstallRBLXBuildConfigGenerator::GenerateCxxModuleConfigInformation(
  std::string const& name, std::ostream& os) const
{
  // Now load per-configuration properties for them.
  /* clang-format off */
  os << "# Load information for each installed configuration.\n"
        "file(GLOB _rblxbuild_cxx_module_includes \"${RBLXBUILD_CURRENT_LIST_DIR}/cxx-modules-" << name << "-*.rblxbuild\")\n"
        "foreach(_rblxbuild_cxx_module_include IN LISTS _rblxbuild_cxx_module_includes)\n"
        "  include(\"${_rblxbuild_cxx_module_include}\")\n"
        "endforeach()\n"
        "unset(_rblxbuild_cxx_module_include)\n"
        "unset(_rblxbuild_cxx_module_includes)\n";
  /* clang-format on */
}

bool cmExportInstallRBLXBuildConfigGenerator::
  GenerateImportCxxModuleConfigTargetInclusion(std::string const& name,
                                               std::string const& config)
{
  auto cxx_modules_dirname = this->GetCxxModulesDirectory();
  if (cxx_modules_dirname.empty()) {
    return true;
  }

  std::string filename_config = config;
  if (filename_config.empty()) {
    filename_config = "noconfig";
  }

  std::string const dest =
    cmStrCat(this->FileDir, '/', cxx_modules_dirname, '/');
  std::string fileName =
    cmStrCat(dest, "cxx-modules-", name, '-', filename_config, ".rblxbuild");

  cmGeneratedFileStream os(fileName, true);
  if (!os) {
    std::string se = cmSystemTools::GetLastSystemError();
    cmSystemTools::Error(
      cmStrCat("cannot write to file \"", fileName, "\": ", se));
    return false;
  }
  os.SetCopyIfDifferent(true);

  // Record this per-config import file.
  this->ConfigCxxModuleFiles[config] = fileName;

  auto& prop_files = this->ConfigCxxModuleTargetFiles[config];
  for (auto const* tgt : this->ExportedTargets) {
    // Only targets with C++ module sources will have a
    // collator-generated install script.
    if (!tgt->HaveCxx20ModuleSources()) {
      continue;
    }

    auto prop_filename = cmStrCat("target-", tgt->GetFilesystemExportName(),
                                  '-', filename_config, ".rblxbuild");
    prop_files.emplace_back(cmStrCat(dest, prop_filename));
    os << "include(\"${RBLXBUILD_CURRENT_LIST_DIR}/" << prop_filename << "\")\n";
  }

  return true;
}
