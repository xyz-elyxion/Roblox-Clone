/// @ref gtx_handed_coordinate_space
/// @file rblxmath/gtx/handed_coordinate_space.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_handed_coordinate_space RBLXMATH_GTX_handed_coordinate_space
/// @ingroup gtx
///
/// Include <rblxmath/gtx/handed_coordinate_space.hpp> to use the features of this extension.
///
/// To know if a set of three basis vectors defines a right or left-handed coordinate system.

#pragma once

// Dependency:
#include "../rblxmath.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_handed_coordinate_space is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_handed_coordinate_space extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_handed_coordinate_space
	/// @{

	//! Return if a trihedron right handed or not.
	//! From RBLXMATH_GTX_handed_coordinate_space extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL bool rightHanded(
		vec<3, T, Q> const& tangent,
		vec<3, T, Q> const& binormal,
		vec<3, T, Q> const& normal);

	//! Return if a trihedron left handed or not.
	//! From RBLXMATH_GTX_handed_coordinate_space extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL bool leftHanded(
		vec<3, T, Q> const& tangent,
		vec<3, T, Q> const& binormal,
		vec<3, T, Q> const& normal);

	/// @}
}// namespace rblxmath

#include "handed_coordinate_space.inl"
