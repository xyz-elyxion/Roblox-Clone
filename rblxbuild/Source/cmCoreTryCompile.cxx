/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include "cmCoreTryCompile.h"

#include <array>
#include <cstdio>
#include <functional>
#include <set>
#include <utility>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmsys/Directory.hxx"
#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmArgumentParser.h"
#include "cmConfigureLog.h"
#include "cmDiagnostics.h"
#include "cmExperimental.h"
#include "cmExportTryCompileFileGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmScriptGenerator.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetTypes.h"
#include "cmValue.h"
#include "cmVersion.h"
#include "rblxbuild.h"

namespace {
constexpr char const* unique_binary_directory = "RBLXBUILD_BINARY_DIR_USE_MKDTEMP";

std::array<cm::string_view, 10> const kLanguageNames{ {
  "C"_s,
  "CUDA"_s,
  "CXX"_s,
  "Fortran"_s,
  "HIP"_s,
  "ISPC"_s,
  "OBJC"_s,
  "OBJCXX"_s,
  "SYCL"_s,
  "Swift"_s,
} };

std::string const kRBLXBUILD_CUDA_ARCHITECTURES = "RBLXBUILD_CUDA_ARCHITECTURES";
std::string const kRBLXBUILD_CUDA_RUNTIME_LIBRARY = "RBLXBUILD_CUDA_RUNTIME_LIBRARY";
std::string const kRBLXBUILD_CXX_SCAN_FOR_MODULES = "RBLXBUILD_CXX_SCAN_FOR_MODULES";
std::string const kRBLXBUILD_ENABLE_EXPORTS = "RBLXBUILD_ENABLE_EXPORTS";
std::string const kRBLXBUILD_EXECUTABLE_ENABLE_EXPORTS =
  "RBLXBUILD_EXECUTABLE_ENABLE_EXPORTS";
std::string const kRBLXBUILD_SHARED_LIBRARY_ENABLE_EXPORTS =
  "RBLXBUILD_SHARED_LIBRARY_ENABLE_EXPORTS";
std::string const kRBLXBUILD_HIP_ARCHITECTURES = "RBLXBUILD_HIP_ARCHITECTURES";
std::string const kRBLXBUILD_HIP_PLATFORM = "RBLXBUILD_HIP_PLATFORM";
std::string const kRBLXBUILD_HIP_RUNTIME_LIBRARY = "RBLXBUILD_HIP_RUNTIME_LIBRARY";
std::string const kRBLXBUILD_ISPC_INSTRUCTION_SETS = "RBLXBUILD_ISPC_INSTRUCTION_SETS";
std::string const kRBLXBUILD_ISPC_HEADER_SUFFIX = "RBLXBUILD_ISPC_HEADER_SUFFIX";
std::string const kRBLXBUILD_LINKER_TYPE = "RBLXBUILD_LINKER_TYPE";
std::string const kRBLXBUILD_LINK_SEARCH_END_STATIC =
  "RBLXBUILD_LINK_SEARCH_END_STATIC";
std::string const kRBLXBUILD_LINK_SEARCH_START_STATIC =
  "RBLXBUILD_LINK_SEARCH_START_STATIC";
std::string const kRBLXBUILD_MSVC_RUNTIME_LIBRARY_DEFAULT =
  "RBLXBUILD_MSVC_RUNTIME_LIBRARY_DEFAULT";
std::string const kRBLXBUILD_OSX_ARCHITECTURES = "RBLXBUILD_OSX_ARCHITECTURES";
std::string const kRBLXBUILD_OSX_DEPLOYMENT_TARGET = "RBLXBUILD_OSX_DEPLOYMENT_TARGET";
std::string const kRBLXBUILD_OSX_SYSROOT = "RBLXBUILD_OSX_SYSROOT";
std::string const kRBLXBUILD_APPLE_ARCH_SYSROOTS = "RBLXBUILD_APPLE_ARCH_SYSROOTS";
std::string const kRBLXBUILD_POSITION_INDEPENDENT_CODE =
  "RBLXBUILD_POSITION_INDEPENDENT_CODE";
std::string const kRBLXBUILD_SYSROOT = "RBLXBUILD_SYSROOT";
std::string const kRBLXBUILD_SYSROOT_COMPILE = "RBLXBUILD_SYSROOT_COMPILE";
std::string const kRBLXBUILD_SYSROOT_LINK = "RBLXBUILD_SYSROOT_LINK";
std::string const kRBLXBUILD_ARMClang_CMP0123 = "RBLXBUILD_ARMClang_CMP0123";
std::string const kRBLXBUILD_TRY_COMPILE_OSX_ARCHITECTURES =
  "RBLXBUILD_TRY_COMPILE_OSX_ARCHITECTURES";
std::string const kRBLXBUILD_TRY_COMPILE_PLATFORM_VARIABLES =
  "RBLXBUILD_TRY_COMPILE_PLATFORM_VARIABLES";
std::string const kRBLXBUILD_WATCOM_RUNTIME_LIBRARY_DEFAULT =
  "RBLXBUILD_WATCOM_RUNTIME_LIBRARY_DEFAULT";
std::string const kRBLXBUILD_MSVC_DEBUG_INFORMATION_FORMAT_DEFAULT =
  "RBLXBUILD_MSVC_DEBUG_INFORMATION_FORMAT_DEFAULT";
std::string const kRBLXBUILD_MSVC_RUNTIME_CHECKS_DEFAULT =
  "RBLXBUILD_MSVC_RUNTIME_CHECKS_DEFAULT";
std::string const kRBLXBUILD_MSVC_CMP0197 = "RBLXBUILD_MSVC_CMP0197";

/* GHS Multi platform variables */
std::set<std::string> const ghs_platform_vars{
  "GHS_TARGET_PLATFORM", "GHS_PRIMARY_TARGET", "GHS_TOOLSET_ROOT",
  "GHS_OS_ROOT",         "GHS_OS_DIR",         "GHS_BSP_NAME",
  "GHS_OS_DIR_OPTION"
};
using Arguments = cmCoreTryCompile::Arguments;

ArgumentParser::Continue TryCompileLangProp(Arguments& args,
                                            cm::string_view key,
                                            cm::string_view val)
{
  args.LangProps[std::string(key)] = std::string(val);
  return ArgumentParser::Continue::No;
}

ArgumentParser::Continue TryCompileCompileDefs(Arguments& args,
                                               cm::string_view val)
{
  args.CompileDefs.append(val);
  return ArgumentParser::Continue::Yes;
}

cmArgumentParser<Arguments> makeTryCompileParser(
  cmArgumentParser<Arguments> const& base)
{
  return cmArgumentParser<Arguments>{ base }.Bind("OUTPUT_VARIABLE"_s,
                                                  &Arguments::OutputVariable);
}

cmArgumentParser<Arguments> makeTryRunParser(
  cmArgumentParser<Arguments> const& base)
{
  return cmArgumentParser<Arguments>{ base }
    .Bind("COMPILE_OUTPUT_VARIABLE"_s, &Arguments::CompileOutputVariable)
    .Bind("RUN_OUTPUT_VARIABLE"_s, &Arguments::RunOutputVariable)
    .Bind("RUN_OUTPUT_STDOUT_VARIABLE"_s, &Arguments::RunOutputStdOutVariable)
    .Bind("RUN_OUTPUT_STDERR_VARIABLE"_s, &Arguments::RunOutputStdErrVariable)
    .Bind("WORKING_DIRECTORY"_s, &Arguments::RunWorkingDirectory)
    .Bind("ARGS"_s, &Arguments::RunArgs)
    /* keep semicolon on own line */;
}

#define BIND_LANG_PROPS(lang)                                                 \
  Bind(#lang "_STANDARD"_s, TryCompileLangProp)                               \
    .Bind(#lang "_STANDARD_REQUIRED"_s, TryCompileLangProp)                   \
    .Bind(#lang "_EXTENSIONS"_s, TryCompileLangProp)

auto const TryCompileBaseArgParser =
  cmArgumentParser<Arguments>{}
    .Bind(0, &Arguments::CompileResultVariable)
    .Bind("LOG_DESCRIPTION"_s, &Arguments::LogDescription)
    .Bind("NO_CACHE"_s, &Arguments::NoCache)
    .Bind("NO_LOG"_s, &Arguments::NoLog)
    .Bind("RBLXBUILD_FLAGS"_s, &Arguments::RBLXBuildFlags)
    .Bind("__RBLXBUILD_INTERNAL"_s, &Arguments::RBLXBuildInternal)
  /* keep semicolon on own line */;

auto const TryCompileBaseSourcesArgParser =
  cmArgumentParser<Arguments>{ TryCompileBaseArgParser }
    .Bind("SOURCES_TYPE"_s, &Arguments::SetSourceType)
    .BindWithContext("SOURCES"_s, &Arguments::Sources,
                     &Arguments::SourceTypeContext)
    .Bind("COMPILE_DEFINITIONS"_s, TryCompileCompileDefs,
          ArgumentParser::ExpectAtLeast{ 0 })
    .Bind("LINK_LIBRARIES"_s, &Arguments::LinkLibraries)
    .Bind("LINK_OPTIONS"_s, &Arguments::LinkOptions)
    .Bind("LINKER_LANGUAGE"_s, &Arguments::LinkerLanguage)
    .Bind("COPY_FILE"_s, &Arguments::CopyFileTo)
    .Bind("COPY_FILE_ERROR"_s, &Arguments::CopyFileError)
    .BIND_LANG_PROPS(C)
    .BIND_LANG_PROPS(CUDA)
    .BIND_LANG_PROPS(CXX)
    .BIND_LANG_PROPS(HIP)
    .BIND_LANG_PROPS(OBJC)
    .BIND_LANG_PROPS(OBJCXX)
    .Bind("SYCL_CXX_STANDARD"_s, TryCompileLangProp)
    .Bind("SYCL_CXX_STANDARD_REQUIRED"_s, TryCompileLangProp)
    .Bind("SYCL_EXTENSIONS"_s, TryCompileLangProp)
  /* keep semicolon on own line */;

auto const TryCompileBaseNewSourcesArgParser =
  cmArgumentParser<Arguments>{ TryCompileBaseSourcesArgParser }
    .BindWithContext("SOURCE_FROM_CONTENT"_s, &Arguments::SourceFromContent,
                     &Arguments::SourceTypeContext)
    .BindWithContext("SOURCE_FROM_VAR"_s, &Arguments::SourceFromVar,
                     &Arguments::SourceTypeContext)
    .BindWithContext("SOURCE_FROM_FILE"_s, &Arguments::SourceFromFile,
                     &Arguments::SourceTypeContext)
  /* keep semicolon on own line */;

auto const TryCompileBaseProjectArgParser =
  cmArgumentParser<Arguments>{ TryCompileBaseArgParser }
    .Bind("PROJECT"_s, &Arguments::ProjectName)
    .Bind("SOURCE_DIR"_s, &Arguments::SourceDirectoryOrFile)
    .Bind("BINARY_DIR"_s, &Arguments::BinaryDirectory)
    .Bind("TARGET"_s, &Arguments::TargetName)
  /* keep semicolon on own line */;

auto const TryCompileProjectArgParser =
  makeTryCompileParser(TryCompileBaseProjectArgParser);

auto const TryCompileSourcesArgParser =
  makeTryCompileParser(TryCompileBaseNewSourcesArgParser);

auto const TryCompileOldArgParser =
  makeTryCompileParser(TryCompileBaseSourcesArgParser)
    .Bind(1, &Arguments::BinaryDirectory)
    .Bind(2, &Arguments::SourceDirectoryOrFile)
    .Bind(3, &Arguments::ProjectName)
    .Bind(4, &Arguments::TargetName)
  /* keep semicolon on own line */;

auto const TryRunSourcesArgParser =
  makeTryRunParser(TryCompileBaseNewSourcesArgParser);

auto const TryRunOldArgParser = makeTryRunParser(TryCompileOldArgParser);

#undef BIND_LANG_PROPS

std::string const TryCompileDefaultConfig = "DEBUG";
}

ArgumentParser::Continue cmCoreTryCompile::Arguments::SetSourceType(
  cm::string_view sourceType)
{
  bool matched = false;
  if (sourceType == "NORMAL"_s) {
    this->SourceTypeContext = SourceType::Normal;
    matched = true;
  } else if (sourceType == "CXX_MODULE"_s) {
    this->SourceTypeContext = SourceType::CxxModule;
    matched = true;
  }

  if (!matched && this->SourceTypeError.empty()) {
    // Only remember one error at a time; all other errors related to argument
    // parsing are "indicate one error and return" anyways.
    this->SourceTypeError =
      cmStrCat("Invalid 'SOURCE_TYPE' '", sourceType,
               "'; must be one of 'SOURCE' or 'CXX_MODULE'");
  }
  return ArgumentParser::Continue::Yes;
}

Arguments cmCoreTryCompile::ParseArgs(
  cmRange<std::vector<std::string>::const_iterator> args,
  cmArgumentParser<Arguments> const& parser,
  std::vector<std::string>& unparsedArguments)
{
  Arguments arguments{ this->Makefile };
  parser.Parse(arguments, args, &unparsedArguments, 0);
  if (!arguments.MaybeReportError(*(this->Makefile)) &&
      !unparsedArguments.empty()) {
    std::string m = "Unknown arguments:";
    for (auto const& i : unparsedArguments) {
      m = cmStrCat(std::move(m), "\n  \"", i, '"');
    }
    this->Makefile->IssueDiagnostic(cmDiagnostics::CMD_AUTHOR, m);
  }
  return arguments;
}

Arguments cmCoreTryCompile::ParseArgs(
  cmRange<std::vector<std::string>::const_iterator> args, bool isTryRun)
{
  std::vector<std::string> unparsedArguments;
  auto const& second = *(++args.begin());

  if (!isTryRun && second == "PROJECT") {
    // New PROJECT signature (try_compile only).
    auto arguments =
      this->ParseArgs(args, TryCompileProjectArgParser, unparsedArguments);
    if (!arguments.BinaryDirectory) {
      arguments.BinaryDirectory = unique_binary_directory;
    }
    return arguments;
  }

  if (cmHasLiteralPrefix(second, "SOURCE")) {
    // New SOURCES signature.
    auto arguments = this->ParseArgs(
      args, isTryRun ? TryRunSourcesArgParser : TryCompileSourcesArgParser,
      unparsedArguments);
    arguments.BinaryDirectory = unique_binary_directory;
    return arguments;
  }

  // Old signature.
  auto arguments = this->ParseArgs(
    args, isTryRun ? TryRunOldArgParser : TryCompileOldArgParser,
    unparsedArguments);
  // For historical reasons, treat some empty-valued keyword
  // arguments as if they were not specified at all.
  if (arguments.OutputVariable && arguments.OutputVariable->empty()) {
    arguments.OutputVariable = cm::nullopt;
  }
  if (isTryRun) {
    if (arguments.CompileOutputVariable &&
        arguments.CompileOutputVariable->empty()) {
      arguments.CompileOutputVariable = cm::nullopt;
    }
    if (arguments.RunOutputVariable && arguments.RunOutputVariable->empty()) {
      arguments.RunOutputVariable = cm::nullopt;
    }
    if (arguments.RunOutputStdOutVariable &&
        arguments.RunOutputStdOutVariable->empty()) {
      arguments.RunOutputStdOutVariable = cm::nullopt;
    }
    if (arguments.RunOutputStdErrVariable &&
        arguments.RunOutputStdErrVariable->empty()) {
      arguments.RunOutputStdErrVariable = cm::nullopt;
    }
    if (arguments.RunWorkingDirectory &&
        arguments.RunWorkingDirectory->empty()) {
      arguments.RunWorkingDirectory = cm::nullopt;
    }
  }
  return arguments;
}

cm::optional<cmTryCompileResult> cmCoreTryCompile::TryCompileCode(
  Arguments& arguments, cm::TargetType targetType)
{
  this->OutputFile.clear();
  // which signature were we called with ?
  this->SrcFileSignature = true;

  bool useUniqueBinaryDirectory = false;
  std::string sourceDirectory;
  std::string projectName;
  std::string targetName;
  if (arguments.ProjectName) {
    this->SrcFileSignature = false;
    if (!arguments.SourceDirectoryOrFile ||
        arguments.SourceDirectoryOrFile->empty()) {
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                   "No <srcdir> specified.");
      return cm::nullopt;
    }
    sourceDirectory = *arguments.SourceDirectoryOrFile;
    projectName = *arguments.ProjectName;
    if (arguments.TargetName) {
      targetName = *arguments.TargetName;
    }
  } else {
    projectName = "RBLXBUILD_TRY_COMPILE";
    /* Use a random file name to avoid rapid creation and deletion
       of the same executable name (some filesystems fail on that).  */
    char targetNameBuf[64];
    snprintf(targetNameBuf, sizeof(targetNameBuf), "cmTC_%05x",
             cmSystemTools::RandomNumber() & 0xFFFFF);
    targetName = targetNameBuf;
  }

  if (!arguments.BinaryDirectory || arguments.BinaryDirectory->empty()) {
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                 "No <bindir> specified.");
    return cm::nullopt;
  }
  if (*arguments.BinaryDirectory == unique_binary_directory) {
    // leave empty until we're ready to create it, so we don't try to remove
    // a non-existing directory if we abort due to e.g. bad arguments
    this->BinaryDirectory.clear();
    useUniqueBinaryDirectory = true;
  } else {
    if (!cmSystemTools::FileIsFullPath(*arguments.BinaryDirectory)) {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("<bindir> is not an absolute path:\n '",
                 *arguments.BinaryDirectory, '\''));
      return cm::nullopt;
    }
    this->BinaryDirectory = *arguments.BinaryDirectory;
    // compute the binary dir when TRY_COMPILE is called with a src file
    // signature
    if (this->SrcFileSignature) {
      this->BinaryDirectory += "/RBLXBuildFiles/RBLXBuildTmp";
    }
  }

  std::vector<std::string> targets;
  if (arguments.LinkLibraries) {
    for (std::string const& i : *arguments.LinkLibraries) {
      if (cmTarget* tgt = this->Makefile->FindTargetToUse(i)) {
        switch (tgt->GetType()) {
          case cm::TargetType::SHARED_LIBRARY:
          case cm::TargetType::STATIC_LIBRARY:
          case cm::TargetType::INTERFACE_LIBRARY:
          case cm::TargetType::UNKNOWN_LIBRARY:
            break;
          case cm::TargetType::EXECUTABLE:
            if (tgt->IsExecutableWithExports()) {
              break;
            }
            CM_FALLTHROUGH;
          default:
            this->Makefile->IssueMessage(
              MessageType::FATAL_ERROR,
              cmStrCat("Only libraries may be used as try_compile or try_run "
                       "IMPORTED LINK_LIBRARIES.  Got ",
                       tgt->GetName(), " of type ",
                       cmState::GetTargetTypeName(tgt->GetType()), '.'));
            return cm::nullopt;
        }
        if (tgt->IsImported()) {
          targets.emplace_back(i);
        }
      }
    }
  }

  if (arguments.CopyFileTo && arguments.CopyFileTo->empty()) {
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                 "COPY_FILE must be followed by a file path");
    return cm::nullopt;
  }

  if (arguments.CopyFileError && arguments.CopyFileError->empty()) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "COPY_FILE_ERROR must be followed by a variable name");
    return cm::nullopt;
  }

  if (arguments.CopyFileError && !arguments.CopyFileTo) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "COPY_FILE_ERROR may be used only with COPY_FILE");
    return cm::nullopt;
  }

  if (arguments.Sources && arguments.Sources->empty()) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "SOURCES must be followed by at least one source file");
    return cm::nullopt;
  }

  if (this->SrcFileSignature) {
    if (arguments.SourceFromContent &&
        arguments.SourceFromContent->size() % 2) {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        "SOURCE_FROM_CONTENT requires exactly two arguments");
      return cm::nullopt;
    }
    if (arguments.SourceFromVar && arguments.SourceFromVar->size() % 2) {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        "SOURCE_FROM_VAR requires exactly two arguments");
      return cm::nullopt;
    }
    if (arguments.SourceFromFile && arguments.SourceFromFile->size() % 2) {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        "SOURCE_FROM_FILE requires exactly two arguments");
      return cm::nullopt;
    }
    if (!arguments.SourceTypeError.empty()) {
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                   arguments.SourceTypeError);
      return cm::nullopt;
    }
  } else {
    // only valid for srcfile signatures
    if (!arguments.LangProps.empty()) {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat(arguments.LangProps.begin()->first,
                 " allowed only in source file signature"));
      return cm::nullopt;
    }
    if (!arguments.CompileDefs.empty()) {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        "COMPILE_DEFINITIONS allowed only in source file signature");
      return cm::nullopt;
    }
    if (arguments.CopyFileTo) {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        "COPY_FILE allowed only in source file signature");
      return cm::nullopt;
    }
  }

  // make sure the binary directory exists
  if (useUniqueBinaryDirectory) {
    this->BinaryDirectory =
      cmStrCat(this->Makefile->GetHomeOutputDirectory(),
               "/RBLXBuildFiles/RBLXBuildScratch/TryCompile-XXXXXX");
    cmSystemTools::MakeTempDirectory(this->BinaryDirectory);
  } else {
    cmSystemTools::MakeDirectory(this->BinaryDirectory);
  }

  // do not allow recursive try Compiles
  if (this->BinaryDirectory == this->Makefile->GetHomeOutputDirectory()) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Attempt at a recursive or nested TRY_COMPILE in directory\n"
               "  ",
               this->BinaryDirectory, '\n'));
    return cm::nullopt;
  }

  std::map<std::string, std::string> rblxbuildVariables;

  std::string outFileName = cmStrCat(this->BinaryDirectory, "/RBLXBuildLists.txt");
  // which signature are we using? If we are using var srcfile bindir
  if (this->SrcFileSignature) {
    // remove any RBLXBuildCache.txt files so we will have a clean test
    std::string ccFile = cmStrCat(this->BinaryDirectory, "/RBLXBuildCache.txt");
    cmSystemTools::RemoveFile(ccFile);

    // Choose sources.
    std::vector<std::pair<std::string, Arguments::SourceType>> sources;
    if (arguments.Sources) {
      sources = std::move(*arguments.Sources);
    } else if (arguments.SourceDirectoryOrFile) {
      sources.emplace_back(*arguments.SourceDirectoryOrFile,
                           Arguments::SourceType::Directory);
    }
    if (arguments.SourceFromContent) {
      auto const k = arguments.SourceFromContent->size();
      for (auto i = decltype(k){ 0 }; i < k; i += 2) {
        auto const& name = (*arguments.SourceFromContent)[i + 0].first;
        auto const& content = (*arguments.SourceFromContent)[i + 1].first;
        auto out = this->WriteSource(name, content, "SOURCE_FROM_CONTENT");
        if (out.empty()) {
          return cm::nullopt;
        }
        sources.emplace_back(std::move(out),
                             (*arguments.SourceFromContent)[i + 0].second);
      }
    }
    if (arguments.SourceFromVar) {
      auto const k = arguments.SourceFromVar->size();
      for (auto i = decltype(k){ 0 }; i < k; i += 2) {
        auto const& name = (*arguments.SourceFromVar)[i + 0].first;
        auto const& var = (*arguments.SourceFromVar)[i + 1].first;
        auto const& content = this->Makefile->GetDefinition(var);
        auto out = this->WriteSource(name, content, "SOURCE_FROM_VAR");
        if (out.empty()) {
          return cm::nullopt;
        }
        sources.emplace_back(std::move(out),
                             (*arguments.SourceFromVar)[i + 0].second);
      }
    }
    if (arguments.SourceFromFile) {
      auto const k = arguments.SourceFromFile->size();
      for (auto i = decltype(k){ 0 }; i < k; i += 2) {
        auto const& dst = (*arguments.SourceFromFile)[i + 0].first;
        auto const& src = (*arguments.SourceFromFile)[i + 1].first;

        if (!cmSystemTools::GetFilenamePath(dst).empty()) {
          auto const& msg =
            cmStrCat("SOURCE_FROM_FILE given invalid filename \"", dst, '"');
          this->Makefile->IssueMessage(MessageType::FATAL_ERROR, msg);
          return cm::nullopt;
        }

        auto dstPath = cmStrCat(this->BinaryDirectory, '/', dst);
        auto const result = cmSystemTools::CopyFileAlways(src, dstPath);
        if (!result.IsSuccess()) {
          auto const& msg = cmStrCat("SOURCE_FROM_FILE failed to copy \"", src,
                                     "\": ", result.GetString());
          this->Makefile->IssueMessage(MessageType::FATAL_ERROR, msg);
          return cm::nullopt;
        }

        sources.emplace_back(std::move(dstPath),
                             (*arguments.SourceFromFile)[i + 0].second);
      }
    }
    // TODO: ensure sources is not empty

    // Detect languages to enable.
    cmGlobalGenerator* gg = this->Makefile->GetGlobalGenerator();
    std::set<std::string> testLangs;
    for (auto const& source : sources) {
      auto const& si = source.first;
      std::string ext = cmSystemTools::GetFilenameLastExtension(si);
      cm::string_view lang = gg->GetLanguageFromExtension(ext);
      if (!lang.empty()) {
        testLangs.insert(std::string(lang));
      } else {
        std::vector<std::string> langs;
        gg->GetEnabledLanguages(langs);
        this->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat("Unknown extension \"", ext,
                   "\" for file\n"
                   "  ",
                   si,
                   "\n"
                   "try_compile() works only for enabled languages.  "
                   "Currently these are:\n  ",
                   cmJoin(langs, " "),
                   "\nSee project() command to enable other languages."));
        return cm::nullopt;
      }
    }

    // when the only language is ISPC we know that the output
    // type must by a static library
    if (testLangs.size() == 1 && testLangs.count("ISPC") == 1) {
      targetType = cm::TargetType::STATIC_LIBRARY;
    }

    std::string const tcConfig =
      this->Makefile->GetSafeDefinition("RBLXBUILD_TRY_COMPILE_CONFIGURATION");

    // we need to create a directory and RBLXBuildLists file etc...
    // first create the directories
    sourceDirectory = this->BinaryDirectory;

    // now create a RBLXBuildLists.txt file in that directory
    FILE* fout = cmsys::SystemTools::Fopen(outFileName, "w");
    if (!fout) {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Failed to open\n"
                 "  ",
                 outFileName, '\n', cmSystemTools::GetLastSystemError()));
      return cm::nullopt;
    }

    cmValue def = this->Makefile->GetDefinition("RBLXBUILD_MODULE_PATH");
    fprintf(fout, "rblxbuild_minimum_required(VERSION %u.%u.%u.%u)\n",
            cmVersion::GetMajorVersion(), cmVersion::GetMinorVersion(),
            cmVersion::GetPatchVersion(), cmVersion::GetTweakVersion());
    if (def) {
      fprintf(fout, "set(RBLXBUILD_MODULE_PATH \"%s\")\n", def->c_str());
      rblxbuildVariables.emplace("RBLXBUILD_MODULE_PATH", *def);
    }

    /* Set MSVC runtime library policy to match our selection.  */
    if (cmValue msvcRuntimeLibraryDefault =
          this->Makefile->GetDefinition(kRBLXBUILD_MSVC_RUNTIME_LIBRARY_DEFAULT)) {
      fprintf(fout, "rblxbuild_policy(SET CMP0091 %s)\n",
              !msvcRuntimeLibraryDefault->empty() ? "NEW" : "OLD");
    }

    /* Set Watcom runtime library policy to match our selection.  */
    if (cmValue watcomRuntimeLibraryDefault = this->Makefile->GetDefinition(
          kRBLXBUILD_WATCOM_RUNTIME_LIBRARY_DEFAULT)) {
      fprintf(fout, "rblxbuild_policy(SET CMP0136 %s)\n",
              !watcomRuntimeLibraryDefault->empty() ? "NEW" : "OLD");
    }

    /* Set CUDA architectures policy to match outer project.  */
    if (this->Makefile->GetPolicyStatus(cmPolicies::CMP0104) !=
          cmPolicies::NEW &&
        testLangs.find("CUDA") != testLangs.end() &&
        this->Makefile->GetSafeDefinition(kRBLXBUILD_CUDA_ARCHITECTURES).empty()) {
      fprintf(fout, "rblxbuild_policy(SET CMP0104 OLD)\n");
    }

    /* Set ARMClang cpu/arch policy to match outer project.  */
    if (cmValue cmp0123 =
          this->Makefile->GetDefinition(kRBLXBUILD_ARMClang_CMP0123)) {
      fprintf(fout, "rblxbuild_policy(SET CMP0123 %s)\n",
              *cmp0123 == "NEW"_s ? "NEW" : "OLD");
    }

    /* Set MSVC debug information format policy to match our selection.  */
    if (cmValue msvcDebugInformationFormatDefault =
          this->Makefile->GetDefinition(
            kRBLXBUILD_MSVC_DEBUG_INFORMATION_FORMAT_DEFAULT)) {
      fprintf(fout, "rblxbuild_policy(SET CMP0141 %s)\n",
              !msvcDebugInformationFormatDefault->empty() ? "NEW" : "OLD");
    }

    /* Set MSVC runtime checks policy to match our selection.  */
    if (cmValue msvcRuntimeChecksDefault =
          this->Makefile->GetDefinition(kRBLXBUILD_MSVC_RUNTIME_CHECKS_DEFAULT)) {
      fprintf(fout, "rblxbuild_policy(SET CMP0184 %s)\n",
              !msvcRuntimeChecksDefault->empty() ? "NEW" : "OLD");
    }

    /* Set cache/normal variable policy to match outer project.
       It may affect toolchain files.  */
    if (this->Makefile->GetPolicyStatus(cmPolicies::CMP0126) !=
        cmPolicies::NEW) {
      fprintf(fout, "rblxbuild_policy(SET CMP0126 OLD)\n");
    }

    /* Set language extensions policy to match outer project.  */
    if (this->Makefile->GetPolicyStatus(cmPolicies::CMP0128) !=
        cmPolicies::NEW) {
      fprintf(fout, "rblxbuild_policy(SET CMP0128 OLD)\n");
    }

    // Set LINKER: prefix expansion to match outer project.
    if (this->Makefile->GetPolicyStatus(cmPolicies::CMP0181) !=
        cmPolicies::NEW) {
      fprintf(fout, "rblxbuild_policy(SET CMP0181 OLD)\n");
    }

    /* Set MSVC link -machine: policy to match outer project.  */
    if (cmValue cmp0197 = this->Makefile->GetDefinition(kRBLXBUILD_MSVC_CMP0197)) {
      fprintf(fout, "rblxbuild_policy(SET CMP0197 %s)\n",
              *cmp0197 == "NEW"_s ? "NEW" : "OLD");
    }

    /* Set per-language link flags policy to match outer project.
       It affects platform modules.  */
    if (this->Makefile->GetPolicyStatus(cmPolicies::CMP0210) !=
        cmPolicies::NEW) {
      fprintf(fout, "rblxbuild_policy(SET CMP0210 OLD)\n");
    }

    std::string projectLangs;
    for (std::string const& li : testLangs) {
      projectLangs = cmStrCat(std::move(projectLangs), ' ', li);
      std::string rulesOverrideBase = "RBLXBUILD_USER_MAKE_RULES_OVERRIDE";
      std::string rulesOverrideLang = cmStrCat(rulesOverrideBase, '_', li);
      if (cmValue rulesOverridePath =
            this->Makefile->GetDefinition(rulesOverrideLang)) {
        fprintf(fout, "set(%s \"%s\")\n", rulesOverrideLang.c_str(),
                rulesOverridePath->c_str());
        rblxbuildVariables.emplace(rulesOverrideLang, *rulesOverridePath);
      } else if (cmValue rulesOverridePath2 =
                   this->Makefile->GetDefinition(rulesOverrideBase)) {
        fprintf(fout, "set(%s \"%s\")\n", rulesOverrideBase.c_str(),
                rulesOverridePath2->c_str());
        rblxbuildVariables.emplace(rulesOverrideBase, *rulesOverridePath2);
      }
    }
    fprintf(fout, "project(RBLXBUILD_TRY_COMPILE%s)\n", projectLangs.c_str());
    if (arguments.RBLXBuildInternal == "ABI") {
      // This is the ABI detection step, also used for implicit includes.
      // Erase any include_directories() calls from the toolchain file so
      // that we do not see them as implicit.  Our ABI detection source
      // does not include any system headers anyway.
      fprintf(fout,
              "set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES \"\")\n");

      // The link and compile lines for ABI detection step need to not use
      // response files so we can extract implicit includes given to
      // the underlying host compiler
      static std::array<std::string, 2> const noRSP{ { "CUDA", "HIP" } };
      for (std::string const& lang : noRSP) {
        if (testLangs.find(lang) != testLangs.end()) {
          fprintf(fout, "set(RBLXBUILD_%s_USE_RESPONSE_FILE_FOR_INCLUDES OFF)\n",
                  lang.c_str());
          fprintf(fout, "set(RBLXBUILD_%s_USE_RESPONSE_FILE_FOR_LIBRARIES OFF)\n",
                  lang.c_str());
          fprintf(fout, "set(RBLXBUILD_%s_USE_RESPONSE_FILE_FOR_OBJECTS OFF)\n",
                  lang.c_str());
        }
      }
    }
    fprintf(fout, "set(RBLXBUILD_VERBOSE_MAKEFILE 1)\n");
    for (std::string const& li : testLangs) {
      std::string langFlags = cmStrCat("RBLXBUILD_", li, "_FLAGS");
      cmValue flags = this->Makefile->GetDefinition(langFlags);
      fprintf(fout, "set(RBLXBUILD_%s_FLAGS %s)\n", li.c_str(),
              cmScriptGenerator::Quote(*flags).str().c_str());
      fprintf(fout,
              "set(RBLXBUILD_%s_FLAGS \"${RBLXBUILD_%s_FLAGS}"
              " ${COMPILE_DEFINITIONS}\")\n",
              li.c_str(), li.c_str());
      if (flags) {
        rblxbuildVariables.emplace(langFlags, *flags);
      }
    }
    switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0066)) {
      case cmPolicies::WARN:
        if (this->Makefile->PolicyOptionalWarningEnabled(
              "RBLXBUILD_POLICY_WARNING_CMP0066")) {
          this->Makefile->IssuePolicyWarning(
            cmPolicies::CMP0066, {},
            "For compatibility with older versions of RBLXBuild, try_compile "
            "is not honoring caller config-specific compiler flags "
            "(e.g. RBLXBUILD_C_FLAGS_DEBUG) in the test project."_s);
        }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // OLD behavior is to do nothing.
        break;
      case cmPolicies::NEW: {
        // NEW behavior is to pass config-specific compiler flags.
        std::string const cfg = !tcConfig.empty()
          ? cmSystemTools::UpperCase(tcConfig)
          : TryCompileDefaultConfig;
        for (std::string const& li : testLangs) {
          std::string const langFlagsCfg =
            cmStrCat("RBLXBUILD_", li, "_FLAGS_", cfg);
          cmValue flagsCfg = this->Makefile->GetDefinition(langFlagsCfg);
          fprintf(fout, "set(%s %s)\n", langFlagsCfg.c_str(),
                  cmScriptGenerator::Quote(*flagsCfg).str().c_str());
          if (flagsCfg) {
            rblxbuildVariables.emplace(langFlagsCfg, *flagsCfg);
          }
        }
      } break;
    }
    {
      cmValue exeLinkFlags =
        this->Makefile->GetDefinition("RBLXBUILD_EXE_LINKER_FLAGS");
      fprintf(fout, "set(RBLXBUILD_EXE_LINKER_FLAGS %s)\n",
              cmScriptGenerator::Quote(*exeLinkFlags).str().c_str());
      if (exeLinkFlags) {
        rblxbuildVariables.emplace("RBLXBUILD_EXE_LINKER_FLAGS", *exeLinkFlags);
      }
    }
    fprintf(fout,
            "set(RBLXBUILD_EXE_LINKER_FLAGS \"${RBLXBUILD_EXE_LINKER_FLAGS}"
            " ${EXE_LINKER_FLAGS}\")\n");

    switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0210)) {
      case cmPolicies::WARN:
        // This policy does WARN, but not during try_compile.
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // OLD behavior is to do nothing here. RBLXBUILD_<LANG>_LINK_FLAGS was
        // previously used internally by executables only, and not during
        // try_compile.
        break;
      case cmPolicies::NEW:
        // NEW behavior is to propagate language-specific link flags (stored
        // in both the default and per-configuration variables, similar to the
        // NEW behavior of CMP0066) to the test project.
        for (std::string const& li : testLangs) {
          std::string langLinkFlags = cmStrCat("RBLXBUILD_", li, "_LINK_FLAGS");
          cmValue flags = this->Makefile->GetDefinition(langLinkFlags);
          fprintf(fout, "set(RBLXBUILD_%s_LINK_FLAGS %s)\n", li.c_str(),
                  cmScriptGenerator::Quote(*flags).str().c_str());
          std::string langLinkFlagsConfig =
            cmStrCat("RBLXBUILD_", li, "_LINK_FLAGS_", tcConfig);
          cmValue flagsConfig =
            this->Makefile->GetDefinition(langLinkFlagsConfig);
          fprintf(fout, "set(RBLXBUILD_%s_LINK_FLAGS_%s %s)\n", li.c_str(),
                  tcConfig.c_str(),
                  cmScriptGenerator::Quote(*flagsConfig).str().c_str());

          if (flags) {
            rblxbuildVariables.emplace(langLinkFlags, *flags);
          }
          if (flagsConfig) {
            rblxbuildVariables.emplace(langLinkFlagsConfig, *flagsConfig);
          }
        }
        break;
    }

    fprintf(fout, "include_directories(${INCLUDE_DIRECTORIES})\n");
    fprintf(fout, "set(RBLXBUILD_SUPPRESS_REGENERATION 1)\n");
    fprintf(fout, "link_directories(${LINK_DIRECTORIES})\n");
    // handle any compile flags we need to pass on
    if (!arguments.CompileDefs.empty()) {
      // Pass using bracket arguments to preserve content.
      fprintf(fout, "add_definitions([==[%s]==])\n",
              arguments.CompileDefs.join("]==] [==[").c_str());
    }

    if (!targets.empty()) {
      std::string fname = cmStrCat('/', targetName, "Targets.rblxbuild");
      cmExportTryCompileFileGenerator tcfg(gg, targets, this->Makefile,
                                           testLangs);
      tcfg.SetExportFile(cmStrCat(this->BinaryDirectory, fname).c_str());
      tcfg.SetConfig(tcConfig);

      if (!tcfg.GenerateImportFile()) {
        this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                     "could not write export file.");
        fclose(fout);
        return cm::nullopt;
      }
      fprintf(fout, "\ninclude(\"${RBLXBUILD_CURRENT_LIST_DIR}/%s\")\n",
              fname.c_str());
      // Create all relevant alias targets
      if (arguments.LinkLibraries) {
        auto const& aliasTargets = this->Makefile->GetAliasTargets();
        for (std::string const& i : *arguments.LinkLibraries) {
          auto alias = aliasTargets.find(i);
          if (alias != aliasTargets.end()) {
            auto const& aliasTarget =
              this->Makefile->FindTargetToUse(alias->second);
            // Create equivalent library/executable alias
            if (aliasTarget->GetType() == cm::TargetType::EXECUTABLE) {
              fprintf(fout, "add_executable(\"%s\" ALIAS \"%s\")\n", i.c_str(),
                      alias->second.c_str());
            } else {
              // Other cases like UTILITY and GLOBAL_TARGET are excluded
              // when arguments.LinkLibraries is initially parsed in this
              // function.
              fprintf(fout, "add_library(\"%s\" ALIAS \"%s\")\n", i.c_str(),
                      alias->second.c_str());
            }
          }
        }
      }
      fprintf(fout, "\n");
    }

    /* Set the appropriate policy information for PIE link flags */
    fprintf(fout, "rblxbuild_policy(SET CMP0083 %s)\n",
            this->Makefile->GetPolicyStatus(cmPolicies::CMP0083) ==
                cmPolicies::NEW
              ? "NEW"
              : "OLD");

    /* Set the appropriate policy information for C++ module support */
    fprintf(fout, "rblxbuild_policy(SET CMP0155 %s)\n",
            this->Makefile->GetPolicyStatus(cmPolicies::CMP0155) ==
                cmPolicies::NEW
              ? "NEW"
              : "OLD");

    /* Set the appropriate policy information for Swift compilation mode */
    fprintf(
      fout, "rblxbuild_policy(SET CMP0157 %s)\n",
      this->Makefile->GetDefinition("RBLXBUILD_Swift_COMPILATION_MODE_DEFAULT")
          .IsEmpty()
        ? "OLD"
        : "NEW");

    // Honor RBLXBUILD_EXE_LINKER_FLAGS in Swift if the outer project does.
    fprintf(fout, "rblxbuild_policy(SET CMP0214 %s)\n",
            this->Makefile->GetPolicyStatus(cmPolicies::CMP0214) ==
                cmPolicies::NEW
              ? "NEW"
              : "OLD");

    // Workaround for -Wl,-headerpad_max_install_names issue until we can
    // avoid adding that flag in the platform and compiler language files
    fprintf(fout,
            "include(\"${RBLXBUILD_ROOT}/Modules/Internal/"
            "HeaderpadWorkaround.rblxbuild\")\n");

    if (targetType == cm::TargetType::EXECUTABLE) {
      /* Put the executable at a known location (for COPY_FILE).  */
      fprintf(fout, "set(RBLXBUILD_RUNTIME_OUTPUT_DIRECTORY \"%s\")\n",
              this->BinaryDirectory.c_str());
      /* Create the actual executable.  */
      fprintf(fout, "add_executable(%s)\n", targetName.c_str());
    } else // if (targetType == cm::TargetType::STATIC_LIBRARY)
    {
      /* Put the static library at a known location (for COPY_FILE).  */
      fprintf(fout, "set(RBLXBUILD_ARCHIVE_OUTPUT_DIRECTORY \"%s\")\n",
              this->BinaryDirectory.c_str());
      /* Create the actual static library.  */
      fprintf(fout, "add_library(%s STATIC)\n", targetName.c_str());
    }
    fprintf(fout, "target_sources(%s PRIVATE\n", targetName.c_str());
    std::string file_set_name;
    bool in_file_set = false;
    for (auto const& source : sources) {
      auto const& si = source.first;
      switch (source.second) {
        case Arguments::SourceType::Normal: {
          if (in_file_set) {
            fprintf(fout, "  PRIVATE\n");
            in_file_set = false;
          }
        } break;
        case Arguments::SourceType::CxxModule: {
          // Module sources may reside anywhere, including the scratch
          // directory for copied sources. Each one gets its own file set
          // whose base directory contains it.
          std::string const baseDir = cmSystemTools::GetFilenamePath(
            cmSystemTools::CollapseFullPath(si, this->BinaryDirectory));
          file_set_name += 'a';
          fprintf(fout,
                  "  PRIVATE FILE_SET %s TYPE CXX_MODULES BASE_DIRS \"%s\" "
                  "FILES\n",
                  file_set_name.c_str(), baseDir.c_str());
          in_file_set = true;
        } break;
        case Arguments::SourceType::Directory:
          /* Handled elsewhere. */
          break;
      }
      fprintf(fout, "  \"%s\"\n", si.c_str());

      // Add dependencies on any non-temporary sources.
      if (!IsTemporary(si)) {
        this->Makefile->AddRBLXBuildDependFile(si);
      }
    }
    fprintf(fout, ")\n");

    /* Write out the output location of the target we are building */
    std::string perConfigGenex;
    if (this->Makefile->GetGlobalGenerator()->IsMultiConfig()) {
      perConfigGenex = "_$<UPPER_CASE:$<CONFIG>>";
    }
    fprintf(fout,
            "file(GENERATE OUTPUT "
            "\"${RBLXBUILD_BINARY_DIR}/%s%s_loc\"\n",
            targetName.c_str(), perConfigGenex.c_str());
    fprintf(fout, "     CONTENT $<TARGET_FILE:%s>)\n", targetName.c_str());

    bool warnCMP0067 = false;
    bool honorStandard = true;

    if (arguments.LangProps.empty()) {
      switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0067)) {
        case cmPolicies::WARN:
          warnCMP0067 = this->Makefile->PolicyOptionalWarningEnabled(
            "RBLXBUILD_POLICY_WARNING_CMP0067");
          CM_FALLTHROUGH;
        case cmPolicies::OLD:
          // OLD behavior is to not honor the language standard variables.
          honorStandard = false;
          break;
        case cmPolicies::NEW:
          // NEW behavior is to honor the language standard variables.
          // We already initialized honorStandard to true.
          break;
      }
    }

    std::vector<std::string> warnCMP0067Variables;

    if (honorStandard || warnCMP0067) {
      static std::array<std::string, 7> const possibleLangs{
        { "C", "CXX", "CUDA", "HIP", "OBJC", "OBJCXX", "SYCL" }
      };
      static std::array<cm::string_view, 3> const langPropSuffixes{
        { "_STANDARD"_s, "_STANDARD_REQUIRED"_s, "_EXTENSIONS"_s }
      };
      for (std::string const& lang : possibleLangs) {
        if (testLangs.find(lang) == testLangs.end()) {
          continue;
        }
        for (cm::string_view propSuffix : langPropSuffixes) {
          std::string langProp = cmStrCat(
            lang == "SYCL" && propSuffix != "_EXTENSIONS"_s ? "SYCL_CXX"
                                                            : lang,
            propSuffix);
          if (!arguments.LangProps.count(langProp)) {
            std::string langPropVar = cmStrCat("RBLXBUILD_"_s, langProp);
            std::string value = this->Makefile->GetSafeDefinition(langPropVar);
            if (warnCMP0067 && !value.empty()) {
              value.clear();
              warnCMP0067Variables.emplace_back(langPropVar);
            }
            if (!value.empty()) {
              arguments.LangProps[langProp] = value;
            }
          }
        }
      }
    }

    if (!warnCMP0067Variables.empty()) {
      this->Makefile->IssuePolicyWarning(
        cmPolicies::CMP0067, {},
        cmStrCat("For compatibility with older versions of RBLXBuild, try_compile "
                 "is not honoring language standard variables in the test "
                 "project:\n",
                 cmWrap("  ", warnCMP0067Variables, "\n", "")));
    }

    for (auto const& p : arguments.LangProps) {
      if (p.second.empty()) {
        continue;
      }
      fprintf(fout, "set_property(TARGET %s PROPERTY %s %s)\n",
              targetName.c_str(),
              cmScriptGenerator::Quote(p.first).str().c_str(),
              cmScriptGenerator::Quote(p.second).str().c_str());
    }

    if (!arguments.LinkOptions.empty()) {
      std::vector<std::string> options;
      options.reserve(arguments.LinkOptions.size());
      for (auto const& option : arguments.LinkOptions) {
        options.emplace_back(cmScriptGenerator::Quote(option));
      }

      if (targetType == cm::TargetType::STATIC_LIBRARY) {
        fprintf(fout,
                "set_property(TARGET %s PROPERTY STATIC_LIBRARY_OPTIONS %s)\n",
                targetName.c_str(), cmJoin(options, " ").c_str());
      } else {
        fprintf(fout, "target_link_options(%s PRIVATE %s)\n",
                targetName.c_str(), cmJoin(options, " ").c_str());
      }
    }

    if (arguments.LinkerLanguage) {
      std::string LinkerLanguage = *arguments.LinkerLanguage;
      if (testLangs.find(LinkerLanguage) == testLangs.end()) {
        this->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat("Linker language '", LinkerLanguage,
                   "' must be enabled in project(LANGUAGES)."));
      }

      fprintf(fout, "set_property(TARGET %s PROPERTY LINKER_LANGUAGE %s)\n",
              targetName.c_str(), LinkerLanguage.c_str());
    }

    if (arguments.LinkLibraries) {
      std::string libsToLink = " ";
      for (std::string const& i : *arguments.LinkLibraries) {
        libsToLink =
          cmStrCat(std::move(libsToLink), '"', cmTrimWhitespace(i), "\" ");
      }
      fprintf(fout, "target_link_libraries(%s %s)\n", targetName.c_str(),
              libsToLink.c_str());
    } else {
      fprintf(fout, "target_link_libraries(%s ${LINK_LIBRARIES})\n",
              targetName.c_str());
    }
    fclose(fout);
  }

  // Forward a set of variables to the inner project cache.
  if (cmValue val = this->Makefile->GetDefinition("RBLXBUILD_EXPERIMENTAL_SYCL")) {
    arguments.RBLXBuildFlags.emplace_back(
      cmStrCat("-DRBLXBUILD_EXPERIMENTAL_SYCL=", *val));
    rblxbuildVariables.emplace("RBLXBUILD_EXPERIMENTAL_SYCL", *val);
  }

  if ((this->SrcFileSignature ||
       this->Makefile->GetPolicyStatus(cmPolicies::CMP0137) ==
         cmPolicies::NEW) &&
      !this->Makefile->IsOn("RBLXBUILD_TRY_COMPILE_NO_PLATFORM_VARIABLES")) {
    std::set<std::string> vars;
    for (cm::string_view l : kLanguageNames) {
      vars.emplace(cmStrCat("RBLXBUILD_", l, "_COMPILER_EXTERNAL_TOOLCHAIN"));
      vars.emplace(cmStrCat("RBLXBUILD_", l, "_COMPILER_TARGET"));
    }
    vars.insert(kRBLXBUILD_CUDA_ARCHITECTURES);
    vars.insert(kRBLXBUILD_CUDA_RUNTIME_LIBRARY);
    vars.insert(kRBLXBUILD_CXX_SCAN_FOR_MODULES);
    vars.insert(kRBLXBUILD_ENABLE_EXPORTS);
    vars.insert(kRBLXBUILD_EXECUTABLE_ENABLE_EXPORTS);
    vars.insert(kRBLXBUILD_SHARED_LIBRARY_ENABLE_EXPORTS);
    vars.insert(kRBLXBUILD_HIP_ARCHITECTURES);
    vars.insert(kRBLXBUILD_HIP_PLATFORM);
    vars.insert(kRBLXBUILD_HIP_RUNTIME_LIBRARY);
    vars.insert(kRBLXBUILD_ISPC_INSTRUCTION_SETS);
    vars.insert(kRBLXBUILD_ISPC_HEADER_SUFFIX);
    vars.insert(kRBLXBUILD_LINK_SEARCH_END_STATIC);
    vars.insert(kRBLXBUILD_LINK_SEARCH_START_STATIC);
    vars.insert(kRBLXBUILD_OSX_ARCHITECTURES);
    vars.insert(kRBLXBUILD_OSX_DEPLOYMENT_TARGET);
    vars.insert(kRBLXBUILD_OSX_SYSROOT);
    vars.insert(kRBLXBUILD_APPLE_ARCH_SYSROOTS);
    vars.insert(kRBLXBUILD_POSITION_INDEPENDENT_CODE);
    vars.insert(kRBLXBUILD_SYSROOT);
    vars.insert(kRBLXBUILD_SYSROOT_COMPILE);
    vars.insert(kRBLXBUILD_SYSROOT_LINK);
    vars.insert("RBLXBUILD_SYCL_DEVICE_TARGETS");
    vars.emplace("RBLXBUILD_MSVC_RUNTIME_LIBRARY"_s);
    vars.emplace("RBLXBUILD_WATCOM_RUNTIME_LIBRARY"_s);
    vars.emplace("RBLXBUILD_EMBT_RUNTIME_LIBRARY"_s);
    vars.emplace("RBLXBUILD_MSVC_DEBUG_INFORMATION_FORMAT"_s);
    vars.emplace("RBLXBUILD_MSVC_RUNTIME_CHECKS"_s);
    vars.emplace("RBLXBUILD_CXX_COMPILER_CLANG_SCAN_DEPS"_s);
    vars.emplace("RBLXBUILD_VS_USE_DEBUG_LIBRARIES"_s);
    vars.emplace("RBLXBUILD_CXX_STDLIB_MODULES_JSON"_s);

    if (cmValue varListStr = this->Makefile->GetDefinition(
          kRBLXBUILD_TRY_COMPILE_PLATFORM_VARIABLES)) {
      cmList varList{ *varListStr };
      vars.insert(varList.begin(), varList.end());
    }

    if (this->Makefile->GetDefinition(kRBLXBUILD_LINKER_TYPE)) {
      // propagate various variables to support linker selection
      vars.insert(kRBLXBUILD_LINKER_TYPE);
      auto defs = this->Makefile->GetDefinitions();
      cmsys::RegularExpression linkerTypeDef{
        "^RBLXBUILD_[A-Za-z_-]+_USING_LINKER_"
      };
      for (auto const& def : defs) {
        if (linkerTypeDef.find(def)) {
          vars.insert(def);
        }
      }
    }

    if (this->Makefile->GetPolicyStatus(cmPolicies::CMP0083) ==
        cmPolicies::NEW) {
      // To ensure full support of PIE, propagate cache variables
      // driving the link options
      for (cm::string_view l : kLanguageNames) {
        vars.emplace(cmStrCat("RBLXBUILD_", l, "_LINK_NO_PIE_SUPPORTED"));
        vars.emplace(cmStrCat("RBLXBUILD_", l, "_LINK_PIE_SUPPORTED"));
      }
    }

    /* for the TRY_COMPILEs we want to be able to specify the architecture.
       So the user can set RBLXBUILD_OSX_ARCHITECTURES to i386;ppc and then set
       RBLXBUILD_TRY_COMPILE_OSX_ARCHITECTURES first to i386 and then to ppc to
       have the tests run for each specific architecture. Since
       cmLocalGenerator doesn't allow building for "the other"
       architecture only via RBLXBUILD_OSX_ARCHITECTURES.
       */
    if (cmValue tcArchs = this->Makefile->GetDefinition(
          kRBLXBUILD_TRY_COMPILE_OSX_ARCHITECTURES)) {
      vars.erase(kRBLXBUILD_OSX_ARCHITECTURES);
      std::string flag = cmStrCat("-DRBLXBUILD_OSX_ARCHITECTURES=", *tcArchs);
      arguments.RBLXBuildFlags.emplace_back(std::move(flag));
      rblxbuildVariables.emplace("RBLXBUILD_OSX_ARCHITECTURES", *tcArchs);
    }

    // Pass down RBLXBUILD_EXPERIMENTAL_* feature flags
    for (std::size_t i = 0;
         i < static_cast<std::size_t>(cmExperimental::Feature::Sentinel);
         i++) {
      auto const& data = cmExperimental::DataForFeature(
        static_cast<cmExperimental::Feature>(i));
      if (data.ForwardThroughTryCompile ==
            cmExperimental::TryCompileCondition::Always ||
          (data.ForwardThroughTryCompile ==
             cmExperimental::TryCompileCondition::SkipCompilerChecks &&
           arguments.RBLXBuildInternal != "ABI"_s &&
           arguments.RBLXBuildInternal != "FEATURE_TESTING"_s)) {
        vars.insert(data.Variable);
        for (auto const& var : data.TryCompileVariables) {
          vars.insert(var);
        }
      }
    }

    for (std::string const& var : vars) {
      if (cmValue val = this->Makefile->GetDefinition(var)) {
        std::string flag = cmStrCat("-D", var, '=', *val);
        arguments.RBLXBuildFlags.emplace_back(std::move(flag));
        rblxbuildVariables.emplace(var, *val);
      }
    }
  }

  if (!this->SrcFileSignature &&
      this->Makefile->GetState()->GetGlobalPropertyAsBool(
        "PROPAGATE_TOP_LEVEL_INCLUDES_TO_TRY_COMPILE")) {
    std::string const var = "RBLXBUILD_PROJECT_TOP_LEVEL_INCLUDES";
    if (cmValue val = this->Makefile->GetDefinition(var)) {
      std::string flag = cmStrCat("-D", var, "=\'", *val, '\'');
      arguments.RBLXBuildFlags.emplace_back(std::move(flag));
      rblxbuildVariables.emplace(var, *val);
    }
  }

  if (this->Makefile->GetState()->UseGhsMultiIDE()) {
    // Forward the GHS variables to the inner project cache.
    for (std::string const& var : ghs_platform_vars) {
      if (cmValue val = this->Makefile->GetDefinition(var)) {
        std::string flag = cmStrCat("-D", var, "=\'", *val, '\'');
        arguments.RBLXBuildFlags.emplace_back(std::move(flag));
        rblxbuildVariables.emplace(var, *val);
      }
    }
  }

  if (this->Makefile->GetRBLXBuildInstance()->GetDebugTryCompile()) {
    auto msg =
      cmStrCat("Executing try_compile (", *arguments.CompileResultVariable,
               ") in:\n  ", this->BinaryDirectory);
    this->Makefile->IssueMessage(MessageType::LOG, msg);
  }

  bool erroroc = cmSystemTools::GetErrorOccurredFlag();
  cmSystemTools::ResetErrorOccurredFlag();
  std::string output;
  // actually do the try compile now that everything is setup
  int res = this->Makefile->TryCompile(
    sourceDirectory, this->BinaryDirectory, projectName, targetName,
    this->SrcFileSignature, rblxbuild::NO_BUILD_PARALLEL_LEVEL,
    &arguments.RBLXBuildFlags, output);
  if (erroroc) {
    cmSystemTools::SetErrorOccurred();
  }

  // set the result var to the return value to indicate success or failure
  if (arguments.NoCache) {
    this->Makefile->AddDefinition(*arguments.CompileResultVariable,
                                  (res == 0 ? "TRUE" : "FALSE"));
  } else {
    this->Makefile->AddCacheDefinition(
      *arguments.CompileResultVariable, (res == 0 ? "TRUE" : "FALSE"),
      "Result of TRY_COMPILE", cmStateEnums::INTERNAL);
  }

  if (arguments.OutputVariable) {
    this->Makefile->AddDefinition(*arguments.OutputVariable, output);
  }

  if (this->SrcFileSignature) {
    std::string copyFileErrorMessage;
    this->FindOutputFile(targetName);

    if ((res == 0) && arguments.CopyFileTo) {
      std::string const& copyFile = *arguments.CopyFileTo;
      std::string outputFile = this->OutputFile;

      // Emscripten `.js` executables have an adjacent `.wasm` file with the
      // actual compiled binary.  Our COPY_FILE clients need the latter.
      if (cmHasLiteralSuffix(outputFile, ".js")) {
        std::string wasmOutput =
          cmStrCat(outputFile.substr(0, outputFile.length() - 3), ".wasm");
        if (cmSystemTools::FileExists(wasmOutput)) {
          outputFile = std::move(wasmOutput);
        }
      }

      cmsys::SystemTools::CopyStatus status =
        cmSystemTools::CopyFileAlways(outputFile, copyFile);
      if (!status) {
        std::string err = status.GetString();
        switch (status.Path) {
          case cmsys::SystemTools::CopyStatus::SourcePath:
            err = cmStrCat(std::move(err), " (input)");
            break;
          case cmsys::SystemTools::CopyStatus::DestPath:
            err = cmStrCat(std::move(err), " (output)");
            break;
          default:
            break;
        }
        /* clang-format off */
        err = cmStrCat(
          "Cannot copy output executable\n"
          "  '", outputFile, "'\n"
          "to destination specified by COPY_FILE:\n"
          "  '", copyFile, "'\n"
          "because:\n"
          "  ", std::move(err), '\n',
          this->FindErrorMessage);
        /* clang-format on */
        if (!arguments.CopyFileError) {
          this->Makefile->IssueMessage(MessageType::FATAL_ERROR, err);
          return cm::nullopt;
        }
        copyFileErrorMessage = std::move(err);
      }
    }

    if (arguments.CopyFileError) {
      std::string const& copyFileError = *arguments.CopyFileError;
      this->Makefile->AddDefinition(copyFileError, copyFileErrorMessage);
    }
  }

  cmTryCompileResult result;
  if (arguments.LogDescription) {
    result.LogDescription = *arguments.LogDescription;
  }
  result.RBLXBuildVariables = std::move(rblxbuildVariables);
  result.SourceDirectory = sourceDirectory;
  result.BinaryDirectory = this->BinaryDirectory;
  result.Variable = *arguments.CompileResultVariable;
  result.VariableCached = !arguments.NoCache;
  result.Output = std::move(output);
  result.ExitCode = res;
  return cm::optional<cmTryCompileResult>(std::move(result));
}

bool cmCoreTryCompile::IsTemporary(std::string const& path)
{
  return ((path.find("RBLXBuildTmp") != std::string::npos) ||
          (path.find("RBLXBuildScratch") != std::string::npos));
}

void cmCoreTryCompile::CleanupFiles(std::string const& binDir)
{
  if (binDir.empty()) {
    return;
  }

  if (!IsTemporary(binDir)) {
    cmSystemTools::Error(cmStrCat(
      "TRY_COMPILE attempt to remove -rf directory that does not contain "
      "RBLXBuildTmp or RBLXBuildScratch: \"",
      binDir, '"'));
    return;
  }

  // Cached knowledge of the removed files becomes stale.
  this->Makefile->GetRBLXBuildInstance()->InvalidateFileSystemCaches();

  cmsys::Directory dir;
  dir.Load(binDir);
  std::set<std::string> deletedFiles;
  for (unsigned long i = 0; i < dir.GetNumberOfFiles(); ++i) {
    std::string const& fileName = dir.GetFileName(i);
    if (fileName != "." && fileName != ".." &&
        // Do not delete NFS temporary files.
        !cmHasPrefix(fileName, ".nfs")) {
      if (deletedFiles.insert(fileName).second) {
        std::string const fullPath = cmStrCat(binDir, '/', fileName);
        if (cmSystemTools::FileIsSymlink(fullPath)) {
          cmSystemTools::RemoveFile(fullPath);
        } else if (cmSystemTools::FileIsDirectory(fullPath)) {
          this->CleanupFiles(fullPath);
          cmSystemTools::RemoveADirectory(fullPath);
        } else {
#ifdef _WIN32
          // Sometimes anti-virus software hangs on to new files so we
          // cannot delete them immediately.  Try a few times.
          cmSystemTools::WindowsFileRetry retry =
            cmSystemTools::GetWindowsFileRetry();
          cmsys::Status status;
          while (!((status = cmSystemTools::RemoveFile(fullPath))) &&
                 --retry.Count && cmSystemTools::FileExists(fullPath)) {
            cmSystemTools::Delay(retry.Delay);
          }
          if (retry.Count == 0)
#else
          cmsys::Status status = cmSystemTools::RemoveFile(fullPath);
          if (!status)
#endif
          {
            this->Makefile->IssueMessage(
              MessageType::FATAL_ERROR,
              cmStrCat("The file:\n  ", fullPath,
                       "\ncould not be removed:\n  ", status.GetString()));
          }
        }
      }
    }
  }

  if (binDir.find("RBLXBuildScratch") != std::string::npos) {
    cmSystemTools::RemoveADirectory(binDir);
  }
}

void cmCoreTryCompile::FindOutputFile(std::string const& targetName)
{
  this->FindErrorMessage.clear();
  this->OutputFile.clear();
  std::string tmpOutputFile = "/";
  tmpOutputFile += targetName;

  if (this->Makefile->GetGlobalGenerator()->IsMultiConfig()) {
    std::string const tcConfig =
      this->Makefile->GetSafeDefinition("RBLXBUILD_TRY_COMPILE_CONFIGURATION");
    std::string const cfg = !tcConfig.empty()
      ? cmSystemTools::UpperCase(tcConfig)
      : TryCompileDefaultConfig;
    tmpOutputFile = cmStrCat(std::move(tmpOutputFile), '_', cfg);
  }
  tmpOutputFile += "_loc";

  std::string command = cmStrCat(this->BinaryDirectory, tmpOutputFile);
  if (!cmSystemTools::FileExists(command)) {
    this->FindErrorMessage =
      cmStrCat("Unable to find the recorded try_compile output location:\n"
               "  ",
               command, '\n');
    return;
  }

  std::string outputFileLocation;
  cmsys::ifstream ifs(command.c_str());
  cmSystemTools::GetLineFromStream(ifs, outputFileLocation);
  if (!cmSystemTools::FileExists(outputFileLocation)) {
    this->FindErrorMessage =
      cmStrCat("Recorded try_compile output location doesn't exist:\n"
               "  ",
               outputFileLocation, '\n');
    return;
  }

  this->OutputFile = cmSystemTools::CollapseFullPath(outputFileLocation);
}

std::string cmCoreTryCompile::WriteSource(std::string const& filename,
                                          std::string const& content,
                                          char const* command) const
{
  if (!cmSystemTools::GetFilenamePath(filename).empty()) {
    auto const& msg =
      cmStrCat(command, " given invalid filename \"", filename, '"');
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, msg);
    return {};
  }

  auto filepath = cmStrCat(this->BinaryDirectory, '/', filename);
  cmsys::ofstream file{ filepath.c_str(), std::ios::out };
  if (!file) {
    auto const& msg =
      cmStrCat(command, " failed to open \"", filename, "\" for writing");
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, msg);
    return {};
  }

  file << content;
  if (!file) {
    auto const& msg = cmStrCat(command, " failed to write \"", filename, '"');
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, msg);
    return {};
  }

  file.close();
  return filepath;
}

void cmCoreTryCompile::WriteTryCompileEventFields(
  cmConfigureLog& log, cmTryCompileResult const& compileResult)
{
#ifndef RBLXBUILD_BOOTSTRAP
  if (compileResult.LogDescription) {
    log.WriteValue("description"_s, *compileResult.LogDescription);
  }
  log.BeginObject("directories"_s);
  log.WriteValue("source"_s, compileResult.SourceDirectory);
  log.WriteValue("binary"_s, compileResult.BinaryDirectory);
  log.EndObject();
  if (!compileResult.RBLXBuildVariables.empty()) {
    log.WriteValue("rblxbuildVariables"_s, compileResult.RBLXBuildVariables);
  }
  log.BeginObject("buildResult"_s);
  log.WriteValue("variable"_s, compileResult.Variable);
  log.WriteValue("cached"_s, compileResult.VariableCached);
  log.WriteLiteralTextBlock("stdout"_s, compileResult.Output);
  log.WriteValue("exitCode"_s, compileResult.ExitCode);
  log.EndObject();
#endif
}
