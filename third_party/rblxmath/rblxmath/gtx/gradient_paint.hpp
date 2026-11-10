/// @ref gtx_gradient_paint
/// @file rblxmath/gtx/gradient_paint.hpp
///
/// @see core (dependence)
/// @see gtx_optimum_pow (dependence)
///
/// @defgroup gtx_gradient_paint RBLXMATH_GTX_gradient_paint
/// @ingroup gtx
///
/// Include <rblxmath/gtx/gradient_paint.hpp> to use the features of this extension.
///
/// Functions that return the color of procedural gradient for specific coordinates.

#pragma once

// Dependency:
#include "../rblxmath.hpp"
#include "../gtx/optimum_pow.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_gradient_paint is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_gradient_paint extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_gradient_paint
	/// @{

	/// Return a color from a radial gradient.
	/// @see - gtx_gradient_paint
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL T radialGradient(
		vec<2, T, Q> const& Center,
		T const& Radius,
		vec<2, T, Q> const& Focal,
		vec<2, T, Q> const& Position);

	/// Return a color from a linear gradient.
	/// @see - gtx_gradient_paint
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL T linearGradient(
		vec<2, T, Q> const& Point0,
		vec<2, T, Q> const& Point1,
		vec<2, T, Q> const& Position);

	/// @}
}// namespace rblxmath

#include "gradient_paint.inl"
