/// @ref gtx_normal
/// @file rblxmath/gtx/normal.hpp
///
/// @see core (dependence)
/// @see gtx_extented_min_max (dependence)
///
/// @defgroup gtx_normal RBLXMATH_GTX_normal
/// @ingroup gtx
///
/// Include <rblxmath/gtx/normal.hpp> to use the features of this extension.
///
/// Compute the normal of a triangle.

#pragma once

// Dependency:
#include "../rblxmath.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_normal is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_normal extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_normal
	/// @{

	/// Computes triangle normal from triangle points.
	///
	/// @see gtx_normal
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<3, T, Q> triangleNormal(vec<3, T, Q> const& p1, vec<3, T, Q> const& p2, vec<3, T, Q> const& p3);

	/// @}
}//namespace rblxmath

#include "normal.inl"
