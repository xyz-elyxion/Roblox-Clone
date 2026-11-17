/// @ref gtx_scalar_relational
/// @file linearmath/gtx/scalar_relational.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_scalar_relational LINEARMATH_GTX_scalar_relational
/// @ingroup gtx
///
/// Include <linearmath/gtx/scalar_relational.hpp> to use the features of this extension.
///
/// Extend a position from a source to a position at a defined length.

#pragma once

// Dependency:
#include "../linearmath.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_scalar_relational is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_scalar_relational extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_scalar_relational
	/// @{



	/// @}
}//namespace linearmath

#include "scalar_relational.inl"
