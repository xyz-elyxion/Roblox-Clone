/// @ref gtx_string_cast
/// @file linearmath/gtx/string_cast.hpp
///
/// @see core (dependence)
/// @see gtx_integer (dependence)
/// @see gtx_quaternion (dependence)
///
/// @defgroup gtx_string_cast LINEARMATH_GTX_string_cast
/// @ingroup gtx
///
/// Include <linearmath/gtx/string_cast.hpp> to use the features of this extension.
///
/// Setup strings for LINEARMATH type values

#pragma once

// Dependency:
#include "../linearmath.hpp"
#include "../gtc/type_precision.hpp"
#include "../gtc/quaternion.hpp"
#include "../gtx/dual_quaternion.hpp"
#include <string>
#include <cmath>
#include <cstring>

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_string_cast is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_string_cast extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_string_cast
	/// @{

	/// Create a string from a LINEARMATH vector or matrix typed variable.
	/// @see gtx_string_cast extension.
	template<typename genType>
	LINEARMATH_FUNC_DECL std::string to_string(genType const& x);

	/// @}
}//namespace linearmath

#include "string_cast.inl"
