/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include <clang-tidy/ClangTidyModule.h>

#include "OstringstreamUseCmstrcatCheck.h"
#include "StringConcatenationUseCmstrcatCheck.h"
#include "UseBespokeEnumClassCheck.h"
#include "UseCmstrlenCheck.h"
#include "UseCmsysFstreamCheck.h"
#include "UsePragmaOnceCheck.h"

namespace clang {
namespace tidy {
namespace rblxbuild {
class RBLXBuildClangTidyModule : public ClangTidyModule
{
public:
  void addCheckFactories(ClangTidyCheckFactories& CheckFactories) override
  {
    CheckFactories.registerCheck<UseCmstrlenCheck>("rblxbuild-use-cmstrlen");
    CheckFactories.registerCheck<UseCmsysFstreamCheck>(
      "rblxbuild-use-cmsys-fstream");
    CheckFactories.registerCheck<UseBespokeEnumClassCheck>(
      "rblxbuild-use-bespoke-enum-class");
    CheckFactories.registerCheck<OstringstreamUseCmstrcatCheck>(
      "rblxbuild-ostringstream-use-cmstrcat");
    CheckFactories.registerCheck<UsePragmaOnceCheck>("rblxbuild-use-pragma-once");
    CheckFactories.registerCheck<StringConcatenationUseCmstrcatCheck>(
      "rblxbuild-string-concatenation-use-cmstrcat");
  }
};

static ClangTidyModuleRegistry::Add<RBLXBuildClangTidyModule> X(
  "rblxbuild-clang-tidy", "Adds lint checks for the RBLXBuild code base.");
}
}
}
