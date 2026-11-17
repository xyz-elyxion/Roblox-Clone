/// @ref gtx_perpendicular
/// @file linearmath/gtx/perpendicular.hpp
///
/// @see core (dependence)
/// @see gtx_projection (dependence)
///
/// @defgroup gtx_perpendicular LINEARMATH_GTX_perpendicular
/// @ingroup gtx
///
/// Include <linearmath/gtx/perpendicular.hpp> to use the features of this extension.
///
/// Perpendicular of a vector from other one

#pragma once

// Dependency:
#include "../linearmath.hpp"
#include "../gtx/projection.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_perpendicular is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_perpendicular extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_perpendicular
	/// @{

	//! Projects x a perpendicular axis of Normal.
	//! From LINEARMATH_GTX_perpendicular extension.
	template<typename genType>
	LINEARMATH_FUNC_DECL genType perp(genType const& x, genType const& Normal);

	/// @}
}//namespace linearmath

#include "perpendicular.inl"
