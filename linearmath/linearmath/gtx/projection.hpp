/// @ref gtx_projection
/// @file linearmath/gtx/projection.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_projection LINEARMATH_GTX_projection
/// @ingroup gtx
///
/// Include <linearmath/gtx/projection.hpp> to use the features of this extension.
///
/// Projection of a vector to other one

#pragma once

// Dependency:
#include "../geometric.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_projection is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_projection extension included")
#endif

namespace linearmath
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
	LINEARMATH_FUNC_DECL genType proj(genType const& x, genType const& Normal);

	/// @}
}//namespace linearmath

#include "projection.inl"
