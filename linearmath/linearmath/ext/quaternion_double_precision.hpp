/// @ref ext_quaternion_double_precision
/// @file linearmath/ext/quaternion_double_precision.hpp
///
/// @defgroup ext_quaternion_double_precision LINEARMATH_EXT_quaternion_double_precision
/// @ingroup ext
///
/// Exposes double-precision floating point quaternion type with various precision in term of ULPs.
///
/// Include <linearmath/ext/quaternion_double_precision.hpp> to use the features of this extension.

#pragma once

// Dependency:
#include "../detail/type_quat.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_quaternion_double_precision extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_quaternion_double_precision
	/// @{

	/// Quaternion of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	///
	/// @see ext_quaternion_double_precision
	typedef qua<double, lowp>		lowp_dquat;

	/// Quaternion of medium double-qualifier floating-point numbers using high precision arithmetic in term of ULPs.
	///
	/// @see ext_quaternion_double_precision
	typedef qua<double, mediump>	mediump_dquat;

	/// Quaternion of high double-qualifier floating-point numbers using high precision arithmetic in term of ULPs.
	///
	/// @see ext_quaternion_double_precision
	typedef qua<double, highp>		highp_dquat;

	/// @}
} //namespace linearmath

