/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include <memory>
#include <string>

#include "cmListFileCache.h"

class cmLinkItem;
class rblxbuild;
class cmGeneratorFileSet;
class cmGeneratorTarget;
struct cmGeneratorExpressionDAGChecker;

namespace cm {
namespace GenEx {
struct Context;
}

class TargetPropertyEntry
{
protected:
  static cmLinkItem NoLinkItem;

public:
  TargetPropertyEntry(cmLinkItem const& item);
  virtual ~TargetPropertyEntry() = default;

  static std::unique_ptr<TargetPropertyEntry> Create(
    rblxbuild& rblxbuildInstance, const BT<std::string>& propertyValue,
    bool evaluateForBuildsystem = false);

  virtual std::string const& Evaluate(
    cm::GenEx::Context const& context, cmGeneratorTarget const* headTarget,
    cmGeneratorExpressionDAGChecker* dagChecker,
    cmGeneratorFileSet const* fileSet) const = 0;

  virtual cmListFileBacktrace GetBacktrace() const = 0;
  virtual std::string const& GetInput() const = 0;
  virtual bool GetHadContextSensitiveCondition() const;

  cmLinkItem const& LinkItem;
};
}
