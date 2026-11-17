/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#include <clang-tidy/ClangTidyModule.h>

#include "OstringstreamUseCmstrcatCheck.h"
#include "StringConcatenationUseCmstrcatCheck.h"
#include "UseBespokeEnumClassCheck.h"
#include "UseCmstrlenCheck.h"
#include "UseCmsysFstreamCheck.h"
#include "UsePragmaOnceCheck.h"

namespace clang {
namespace tidy {
namespace linearbuild {
class LinearBuildClangTidyModule : public ClangTidyModule
{
public:
  void addCheckFactories(ClangTidyCheckFactories& CheckFactories) override
  {
    CheckFactories.registerCheck<UseCmstrlenCheck>("linearbuild-use-cmstrlen");
    CheckFactories.registerCheck<UseCmsysFstreamCheck>(
      "linearbuild-use-cmsys-fstream");
    CheckFactories.registerCheck<UseBespokeEnumClassCheck>(
      "linearbuild-use-bespoke-enum-class");
    CheckFactories.registerCheck<OstringstreamUseCmstrcatCheck>(
      "linearbuild-ostringstream-use-cmstrcat");
    CheckFactories.registerCheck<UsePragmaOnceCheck>("linearbuild-use-pragma-once");
    CheckFactories.registerCheck<StringConcatenationUseCmstrcatCheck>(
      "linearbuild-string-concatenation-use-cmstrcat");
  }
};

static ClangTidyModuleRegistry::Add<LinearBuildClangTidyModule> X(
  "linearbuild-clang-tidy", "Adds lint checks for the LinearBuild code base.");
}
}
}
