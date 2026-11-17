/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://linearbuild.org/licensing for details.  */
#pragma once

#include <iostream>

#include "cmSystemTools.h"

#define CM_DBG(expr) cm::dbg_impl(__FILE__, __LINE__, #expr, expr)

namespace cm {

namespace {

template <typename T>
T dbg_impl(char const* fname, int line, char const* expr, T value)
{
  if (!cmSystemTools::GetEnvVar("LINEARBUILD_NO_DBG")) {
    std::cerr << fname << ':' << line << ": " << expr << " = " << value
              << std::endl;
  }
  return value;
}

} // namespace

} // namespace cm
