/// @ref gtx_functions
/// @file rblxmath/gtx/functions.hpp
///
/// @see core (dependence)
/// @see gtc_quaternion (dependence)
///
/// @defgroup gtx_functions RBLXMATH_GTX_functions
/// @ingroup gtx
///
/// Include <rblxmath/gtx/functions.hpp> to use the features of this extension.
///
/// List of useful common functions.

#pragma once

// Dependencies
#include "../detail/setup.hpp"
#include "../detail/qualifier.hpp"
#include "../detail/type_vec2.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_functions is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_functions extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_functions
	/// @{

	/// 1D gauss function
	///
	/// @see gtc_epsilon
	template<typename T>
	RBLXMATH_FUNC_DECL T gauss(
		T x,
		T ExpectedValue,
		T StandardDeviation);

	/// 2D gauss function
	///
	/// @see gtc_epsilon
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL T gauss(
		vec<2, T, Q> const& Coord,
		vec<2, T, Q> const& ExpectedValue,
		vec<2, T, Q> const& StandardDeviation);

	/// @}
}//namespace rblxmath

#include "functions.inl"

