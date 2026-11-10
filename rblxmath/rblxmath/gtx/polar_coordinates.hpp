/// @ref gtx_polar_coordinates
/// @file rblxmath/gtx/polar_coordinates.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_polar_coordinates RBLXMATH_GTX_polar_coordinates
/// @ingroup gtx
///
/// Include <rblxmath/gtx/polar_coordinates.hpp> to use the features of this extension.
///
/// Conversion from Euclidean space to polar space and revert.

#pragma once

// Dependency:
#include "../rblxmath.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_polar_coordinates is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_polar_coordinates extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_polar_coordinates
	/// @{

	/// Convert Euclidean to Polar coordinates, x is the latitude, y the longitude and z the xz distance.
	///
	/// @see gtx_polar_coordinates
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<3, T, Q> polar(
		vec<3, T, Q> const& euclidean);

	/// Convert Polar to Euclidean coordinates.
	///
	/// @see gtx_polar_coordinates
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<3, T, Q> euclidean(
		vec<2, T, Q> const& polar);

	/// @}
}//namespace rblxmath

#include "polar_coordinates.inl"
