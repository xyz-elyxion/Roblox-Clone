/// @ref ext_quaternion_float_precision
/// @file rblxmath/ext/quaternion_float_precision.hpp
///
/// @defgroup ext_quaternion_float_precision RBLXMATH_EXT_quaternion_float_precision
/// @ingroup ext
///
/// Exposes single-precision floating point quaternion type with various precision in term of ULPs.
///
/// Include <rblxmath/ext/quaternion_float_precision.hpp> to use the features of this extension.

#pragma once

// Dependency:
#include "../detail/type_quat.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_quaternion_float_precision extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_quaternion_float_precision
	/// @{

	/// Quaternion of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	typedef qua<float, lowp>		lowp_quat;

	/// Quaternion of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	typedef qua<float, mediump>		mediump_quat;

	/// Quaternion of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	typedef qua<float, highp>		highp_quat;

	/// @}
} //namespace rblxmath

