/// @ref gtx_vector_angle
/// @file rblxmath/gtx/vector_angle.hpp
///
/// @see core (dependence)
/// @see gtx_quaternion (dependence)
/// @see gtx_epsilon (dependence)
///
/// @defgroup gtx_vector_angle RBLXMATH_GTX_vector_angle
/// @ingroup gtx
///
/// Include <rblxmath/gtx/vector_angle.hpp> to use the features of this extension.
///
/// Compute angle between vectors

#pragma once

// Dependency:
#include "../rblxmath.hpp"
#include "../gtc/epsilon.hpp"
#include "../gtx/quaternion.hpp"
#include "../gtx/rotate_vector.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_vector_angle is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_vector_angle extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_vector_angle
	/// @{

	//! Returns the absolute angle between two vectors.
	//! Parameters need to be normalized.
	/// @see gtx_vector_angle extension.
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL T angle(vec<L, T, Q> const& x, vec<L, T, Q> const& y);

	//! Returns the oriented angle between two 2d vectors.
	//! Parameters need to be normalized.
	/// @see gtx_vector_angle extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL T orientedAngle(vec<2, T, Q> const& x, vec<2, T, Q> const& y);

	//! Returns the oriented angle between two 3d vectors based from a reference axis.
	//! Parameters need to be normalized.
	/// @see gtx_vector_angle extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL T orientedAngle(vec<3, T, Q> const& x, vec<3, T, Q> const& y, vec<3, T, Q> const& ref);

	/// @}
}// namespace rblxmath

#include "vector_angle.inl"
