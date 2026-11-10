/// @ref gtx_log_base
/// @file rblxmath/gtx/log_base.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_log_base RBLXMATH_GTX_log_base
/// @ingroup gtx
///
/// Include <rblxmath/gtx/log_base.hpp> to use the features of this extension.
///
/// Logarithm for any base. base can be a vector or a scalar.

#pragma once

// Dependency:
#include "../rblxmath.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_log_base is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_log_base extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_log_base
	/// @{

	/// Logarithm for any base.
	/// From RBLXMATH_GTX_log_base.
	template<typename genType>
	RBLXMATH_FUNC_DECL genType log(
		genType const& x,
		genType const& base);

	/// Logarithm for any base.
	/// From RBLXMATH_GTX_log_base.
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<L, T, Q> sign(
		vec<L, T, Q> const& x,
		vec<L, T, Q> const& base);

	/// @}
}//namespace rblxmath

#include "log_base.inl"
