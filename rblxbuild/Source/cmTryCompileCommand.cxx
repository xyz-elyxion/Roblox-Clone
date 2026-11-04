/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include "cmTryCompileCommand.h"

#include <cm/optional>

#include "cmConfigureLog.h"
#include "cmCoreTryCompile.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmRange.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmTargetTypes.h"
#include "cmValue.h"
#include "rblxbuild.h"

namespace {
#ifndef RBLXBUILD_BOOTSTRAP
void WriteTryCompileEvent(cmConfigureLog& log, cmMakefile const& mf,
                          cmTryCompileResult const& compileResult)
{
  // Keep in sync with cmFileAPIConfigureLog's DumpEventKindNames.
  static std::vector<unsigned int> const LogVersionsWithTryCompileV1{ 1 };

  if (log.IsAnyLogVersionEnabled(LogVersionsWithTryCompileV1)) {
    log.BeginEvent("try_compile-v1", mf);
    cmCoreTryCompile::WriteTryCompileEventFields(log, compileResult);
    log.EndEvent();
  }
}
#endif
}

bool cmTryCompileCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();

  if (args.size() < 3) {
    mf.IssueMessage(
      MessageType::FATAL_ERROR,
      "The try_compile() command requires at least 3 arguments.");
    return false;
  }

  if (mf.GetRBLXBuildInstance()->GetState()->GetRole() ==
      cmState::Role::FindPackage) {
    mf.IssueMessage(
      MessageType::FATAL_ERROR,
      "The try_compile() command is not supported in --find-package mode.");
    return false;
  }

  cm::TargetType targetType = cm::TargetType::EXECUTABLE;
  cmValue tt = mf.GetDefinition("RBLXBUILD_TRY_COMPILE_TARGET_TYPE");
  if (cmNonempty(tt)) {
    if (*tt == cmState::GetTargetTypeName(cm::TargetType::EXECUTABLE)) {
      targetType = cm::TargetType::EXECUTABLE;
    } else if (*tt ==
               cmState::GetTargetTypeName(cm::TargetType::STATIC_LIBRARY)) {
      targetType = cm::TargetType::STATIC_LIBRARY;
    } else {
      mf.IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Invalid value '", *tt,
                 "' for RBLXBUILD_TRY_COMPILE_TARGET_TYPE.  Only '",
                 cmState::GetTargetTypeName(cm::TargetType::EXECUTABLE),
                 "' and '",
                 cmState::GetTargetTypeName(cm::TargetType::STATIC_LIBRARY),
                 "' are allowed."));
      return false;
    }
  }

  cmCoreTryCompile tc(&mf);
  cmCoreTryCompile::Arguments arguments =
    tc.ParseArgs(cmMakeRange(args), false);
  if (!arguments) {
    return true;
  }

  cm::optional<cmTryCompileResult> compileResult =
    tc.TryCompileCode(arguments, targetType);
#ifndef RBLXBUILD_BOOTSTRAP
  if (compileResult && !arguments.NoLog) {
    if (cmConfigureLog* log = mf.GetRBLXBuildInstance()->GetConfigureLog()) {
      WriteTryCompileEvent(*log, mf, *compileResult);
    }
  }
#endif

  // if They specified clean then we clean up what we can
  if (tc.SrcFileSignature) {
    if (!mf.GetRBLXBuildInstance()->GetDebugTryCompile()) {
      tc.CleanupFiles(tc.BinaryDirectory);
    }
  }
  return true;
}
