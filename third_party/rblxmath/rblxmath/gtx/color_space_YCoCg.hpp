/// @ref gtx_color_space_YCoCg
/// @file rblxmath/gtx/color_space_YCoCg.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_color_space_YCoCg RBLXMATH_GTX_color_space_YCoCg
/// @ingroup gtx
///
/// Include <rblxmath/gtx/color_space_YCoCg.hpp> to use the features of this extension.
///
/// RGB to YCoCg conversions and operations

#pragma once

// Dependency:
#include "../rblxmath.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_color_space_YCoCg is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_color_space_YCoCg extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_color_space_YCoCg
	/// @{

	/// Convert a color from RGB color space to YCoCg color space.
	/// @see gtx_color_space_YCoCg
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<3, T, Q> rgb2YCoCg(
		vec<3, T, Q> const& rgbColor);

	/// Convert a color from YCoCg color space to RGB color space.
	/// @see gtx_color_space_YCoCg
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<3, T, Q> YCoCg2rgb(
		vec<3, T, Q> const& YCoCgColor);

	/// Convert a color from RGB color space to YCoCgR color space.
	/// @see "YCoCg-R: A Color Space with RGB Reversibility and Low Dynamic Range"
	/// @see gtx_color_space_YCoCg
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<3, T, Q> rgb2YCoCgR(
		vec<3, T, Q> const& rgbColor);

	/// Convert a color from YCoCgR color space to RGB color space.
	/// @see "YCoCg-R: A Color Space with RGB Reversibility and Low Dynamic Range"
	/// @see gtx_color_space_YCoCg
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<3, T, Q> YCoCgR2rgb(
		vec<3, T, Q> const& YCoCgColor);

	/// @}
}//namespace rblxmath

#include "color_space_YCoCg.inl"
