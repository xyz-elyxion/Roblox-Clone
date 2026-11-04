/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#pragma once

#include <clang-tidy/ClangTidyCheck.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace clang {
namespace tidy {
namespace rblxbuild {
class UseBespokeEnumClassCheck : public ClangTidyCheck
{
public:
  UseBespokeEnumClassCheck(StringRef Name, ClangTidyContext* Context);
  void registerMatchers(ast_matchers::MatchFinder* Finder) override;

  void check(ast_matchers::MatchFinder::MatchResult const& Result) override;
};
}
}
}
