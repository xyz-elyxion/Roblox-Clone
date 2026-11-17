/// @ref gtx_normal
/// @file linearmath/gtx/normal.hpp
///
/// @see core (dependence)
/// @see gtx_extented_min_max (dependence)
///
/// @defgroup gtx_normal LINEARMATH_GTX_normal
/// @ingroup gtx
///
/// Include <linearmath/gtx/normal.hpp> to use the features of this extension.
///
/// Compute the normal of a triangle.

#pragma once

// Dependency:
#include "../linearmath.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_normal is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_normal extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_normal
	/// @{

	/// Computes triangle normal from triangle points.
	///
	/// @see gtx_normal
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL vec<3, T, Q> triangleNormal(vec<3, T, Q> const& p1, vec<3, T, Q> const& p2, vec<3, T, Q> const& p3);

	/// @}
}//namespace linearmath

#include "normal.inl"
