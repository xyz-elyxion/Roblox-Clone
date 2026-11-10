/// @ref gtx_perpendicular
/// @file rblxmath/gtx/perpendicular.hpp
///
/// @see core (dependence)
/// @see gtx_projection (dependence)
///
/// @defgroup gtx_perpendicular RBLXMATH_GTX_perpendicular
/// @ingroup gtx
///
/// Include <rblxmath/gtx/perpendicular.hpp> to use the features of this extension.
///
/// Perpendicular of a vector from other one

#pragma once

// Dependency:
#include "../rblxmath.hpp"
#include "../gtx/projection.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_perpendicular is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_perpendicular extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_perpendicular
	/// @{

	//! Projects x a perpendicular axis of Normal.
	//! From RBLXMATH_GTX_perpendicular extension.
	template<typename genType>
	RBLXMATH_FUNC_DECL genType perp(genType const& x, genType const& Normal);

	/// @}
}//namespace rblxmath

#include "perpendicular.inl"
