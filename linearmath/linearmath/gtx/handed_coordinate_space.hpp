/// @ref gtx_handed_coordinate_space
/// @file linearmath/gtx/handed_coordinate_space.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_handed_coordinate_space LINEARMATH_GTX_handed_coordinate_space
/// @ingroup gtx
///
/// Include <linearmath/gtx/handed_coordinate_space.hpp> to use the features of this extension.
///
/// To know if a set of three basis vectors defines a right or left-handed coordinate system.

#pragma once

// Dependency:
#include "../linearmath.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_handed_coordinate_space is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_handed_coordinate_space extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_handed_coordinate_space
	/// @{

	//! Return if a trihedron right handed or not.
	//! From LINEARMATH_GTX_handed_coordinate_space extension.
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL bool rightHanded(
		vec<3, T, Q> const& tangent,
		vec<3, T, Q> const& binormal,
		vec<3, T, Q> const& normal);

	//! Return if a trihedron left handed or not.
	//! From LINEARMATH_GTX_handed_coordinate_space extension.
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL bool leftHanded(
		vec<3, T, Q> const& tangent,
		vec<3, T, Q> const& binormal,
		vec<3, T, Q> const& normal);

	/// @}
}// namespace linearmath

#include "handed_coordinate_space.inl"
