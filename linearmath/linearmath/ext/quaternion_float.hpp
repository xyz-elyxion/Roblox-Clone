/// @ref ext_quaternion_float
/// @file linearmath/ext/quaternion_float.hpp
///
/// @defgroup ext_quaternion_float LINEARMATH_EXT_quaternion_float
/// @ingroup ext
///
/// Exposes single-precision floating point quaternion type.
///
/// Include <linearmath/ext/quaternion_float.hpp> to use the features of this extension.
///
/// @see ext_quaternion_double
/// @see ext_quaternion_float_precision
/// @see ext_quaternion_common
/// @see ext_quaternion_exponential
/// @see ext_quaternion_geometric
/// @see ext_quaternion_relational
/// @see ext_quaternion_transform
/// @see ext_quaternion_trigonometric

#pragma once

// Dependency:
#include "../detail/type_quat.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_quaternion_float extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_quaternion_float
	/// @{

	/// Quaternion of single-precision floating-point numbers.
	typedef qua<float, defaultp>		quat;

	/// @}
} //namespace linearmath

