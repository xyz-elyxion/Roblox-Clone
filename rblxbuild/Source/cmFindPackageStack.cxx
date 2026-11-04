/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#define cmFindPackageStack_cxx
#include "cmFindPackageStack.h"

#include "cmConstStack.tcc" // IWYU pragma: keep
template class cmConstStack<cmFindPackageCall, cmFindPackageStack>;
