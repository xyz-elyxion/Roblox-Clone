/// @ref gtx_closest_point
/// @file rblxmath/gtx/closest_point.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_closest_point RBLXMATH_GTX_closest_point
/// @ingroup gtx
///
/// Include <rblxmath/gtx/closest_point.hpp> to use the features of this extension.
///
/// Find the point on a straight line which is the closet of a point.

#pragma once

// Dependency:
#include "../rblxmath.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_closest_point is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_closest_point extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_closest_point
	/// @{

	/// Find the point on a straight line which is the closet of a point.
	/// @see gtx_closest_point
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<3, T, Q> closestPointOnLine(
		vec<3, T, Q> const& point,
		vec<3, T, Q> const& a,
		vec<3, T, Q> const& b);

	/// 2d lines work as well
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<2, T, Q> closestPointOnLine(
		vec<2, T, Q> const& point,
		vec<2, T, Q> const& a,
		vec<2, T, Q> const& b);

	/// @}
}// namespace rblxmath

#include "closest_point.inl"
