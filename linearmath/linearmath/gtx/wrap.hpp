/// @ref gtx_wrap
/// @file linearmath/gtx/wrap.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_wrap LINEARMATH_GTX_wrap
/// @ingroup gtx
///
/// Include <linearmath/gtx/wrap.hpp> to use the features of this extension.
///
/// Wrapping mode of texture coordinates.

#pragma once

// Dependency:
#include "../linearmath.hpp"
#include "../ext/scalar_common.hpp"
#include "../ext/vector_common.hpp"
#include "../gtc/vec1.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_wrap is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_wrap extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_wrap
	/// @{

	/// @}
}// namespace linearmath

#include "wrap.inl"
