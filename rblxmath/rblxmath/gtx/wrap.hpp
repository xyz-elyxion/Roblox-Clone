/// @ref gtx_wrap
/// @file rblxmath/gtx/wrap.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_wrap RBLXMATH_GTX_wrap
/// @ingroup gtx
///
/// Include <rblxmath/gtx/wrap.hpp> to use the features of this extension.
///
/// Wrapping mode of texture coordinates.

#pragma once

// Dependency:
#include "../rblxmath.hpp"
#include "../ext/scalar_common.hpp"
#include "../ext/vector_common.hpp"
#include "../gtc/vec1.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_wrap is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_wrap extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_wrap
	/// @{

	/// @}
}// namespace rblxmath

#include "wrap.inl"
