/// @ref gtx_closest_point
/// @file linearmath/gtx/closest_point.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_closest_point LINEARMATH_GTX_closest_point
/// @ingroup gtx
///
/// Include <linearmath/gtx/closest_point.hpp> to use the features of this extension.
///
/// Find the point on a straight line which is the closet of a point.

#pragma once

// Dependency:
#include "../linearmath.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_closest_point is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_closest_point extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_closest_point
	/// @{

	/// Find the point on a straight line which is the closet of a point.
	/// @see gtx_closest_point
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL vec<3, T, Q> closestPointOnLine(
		vec<3, T, Q> const& point,
		vec<3, T, Q> const& a,
		vec<3, T, Q> const& b);

	/// 2d lines work as well
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL vec<2, T, Q> closestPointOnLine(
		vec<2, T, Q> const& point,
		vec<2, T, Q> const& a,
		vec<2, T, Q> const& b);

	/// @}
}// namespace linearmath

#include "closest_point.inl"
