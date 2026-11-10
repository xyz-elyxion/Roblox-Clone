/// @ref gtx_projection
/// @file rblxmath/gtx/projection.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_projection RBLXMATH_GTX_projection
/// @ingroup gtx
///
/// Include <rblxmath/gtx/projection.hpp> to use the features of this extension.
///
/// Projection of a vector to other one

#pragma once

// Dependency:
#include "../geometric.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_projection is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_projection extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_projection
	/// @{

	/// Projects x on Normal.
	///
	/// @param[in] x A vector to project
	/// @param[in] Normal A normal that doesn't need to be of unit length.
	///
	/// @see gtx_projection
	template<typename genType>
	RBLXMATH_FUNC_DECL genType proj(genType const& x, genType const& Normal);

	/// @}
}//namespace rblxmath

#include "projection.inl"
