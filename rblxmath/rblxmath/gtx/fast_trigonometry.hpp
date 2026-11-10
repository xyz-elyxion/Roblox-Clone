/// @ref gtx_fast_trigonometry
/// @file rblxmath/gtx/fast_trigonometry.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_fast_trigonometry RBLXMATH_GTX_fast_trigonometry
/// @ingroup gtx
///
/// Include <rblxmath/gtx/fast_trigonometry.hpp> to use the features of this extension.
///
/// Fast but less accurate implementations of trigonometric functions.

#pragma once

// Dependency:
#include "../gtc/constants.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_fast_trigonometry is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_fast_trigonometry extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_fast_trigonometry
	/// @{

	/// Wrap an angle to [0 2pi[
	/// From RBLXMATH_GTX_fast_trigonometry extension.
	template<typename T>
	RBLXMATH_FUNC_DECL T wrapAngle(T angle);

	/// Faster than the common sin function but less accurate.
	/// From RBLXMATH_GTX_fast_trigonometry extension.
	template<typename T>
	RBLXMATH_FUNC_DECL T fastSin(T angle);

	/// Faster than the common cos function but less accurate.
	/// From RBLXMATH_GTX_fast_trigonometry extension.
	template<typename T>
	RBLXMATH_FUNC_DECL T fastCos(T angle);

	/// Faster than the common tan function but less accurate.
	/// Defined between -2pi and 2pi.
	/// From RBLXMATH_GTX_fast_trigonometry extension.
	template<typename T>
	RBLXMATH_FUNC_DECL T fastTan(T angle);

	/// Faster than the common asin function but less accurate.
	/// Defined between -2pi and 2pi.
	/// From RBLXMATH_GTX_fast_trigonometry extension.
	template<typename T>
	RBLXMATH_FUNC_DECL T fastAsin(T angle);

	/// Faster than the common acos function but less accurate.
	/// Defined between -2pi and 2pi.
	/// From RBLXMATH_GTX_fast_trigonometry extension.
	template<typename T>
	RBLXMATH_FUNC_DECL T fastAcos(T angle);

	/// Faster than the common atan function but less accurate.
	/// Defined between -2pi and 2pi.
	/// From RBLXMATH_GTX_fast_trigonometry extension.
	template<typename T>
	RBLXMATH_FUNC_DECL T fastAtan(T y, T x);

	/// Faster than the common atan function but less accurate.
	/// Defined between -2pi and 2pi.
	/// From RBLXMATH_GTX_fast_trigonometry extension.
	template<typename T>
	RBLXMATH_FUNC_DECL T fastAtan(T angle);

	/// @}
}//namespace rblxmath

#include "fast_trigonometry.inl"
