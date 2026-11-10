/// @ref gtx_string_cast
/// @file rblxmath/gtx/string_cast.hpp
///
/// @see core (dependence)
/// @see gtx_integer (dependence)
/// @see gtx_quaternion (dependence)
///
/// @defgroup gtx_string_cast RBLXMATH_GTX_string_cast
/// @ingroup gtx
///
/// Include <rblxmath/gtx/string_cast.hpp> to use the features of this extension.
///
/// Setup strings for RBLXMATH type values

#pragma once

// Dependency:
#include "../rblxmath.hpp"
#include "../gtc/type_precision.hpp"
#include "../gtc/quaternion.hpp"
#include "../gtx/dual_quaternion.hpp"
#include <string>
#include <cmath>
#include <cstring>

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_string_cast is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_string_cast extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_string_cast
	/// @{

	/// Create a string from a RBLXMATH vector or matrix typed variable.
	/// @see gtx_string_cast extension.
	template<typename genType>
	RBLXMATH_FUNC_DECL std::string to_string(genType const& x);

	/// @}
}//namespace rblxmath

#include "string_cast.inl"
