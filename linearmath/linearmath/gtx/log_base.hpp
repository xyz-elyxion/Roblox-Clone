/// @ref gtx_log_base
/// @file linearmath/gtx/log_base.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_log_base LINEARMATH_GTX_log_base
/// @ingroup gtx
///
/// Include <linearmath/gtx/log_base.hpp> to use the features of this extension.
///
/// Logarithm for any base. base can be a vector or a scalar.

#pragma once

// Dependency:
#include "../linearmath.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_log_base is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_log_base extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_log_base
	/// @{

	/// Logarithm for any base.
	/// From LINEARMATH_GTX_log_base.
	template<typename genType>
	LINEARMATH_FUNC_DECL genType log(
		genType const& x,
		genType const& base);

	/// Logarithm for any base.
	/// From LINEARMATH_GTX_log_base.
	template<length_t L, typename T, qualifier Q>
	LINEARMATH_FUNC_DECL vec<L, T, Q> sign(
		vec<L, T, Q> const& x,
		vec<L, T, Q> const& base);

	/// @}
}//namespace linearmath

#include "log_base.inl"
