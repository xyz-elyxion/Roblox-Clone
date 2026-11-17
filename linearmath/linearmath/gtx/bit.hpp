/// @ref gtx_bit
/// @file linearmath/gtx/bit.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_bit LINEARMATH_GTX_bit
/// @ingroup gtx
///
/// Include <linearmath/gtx/bit.hpp> to use the features of this extension.
///
/// Allow to perform bit operations on integer values

#pragma once

// Dependencies
#include "../gtc/bitfield.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_bit is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_bit extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_bit
	/// @{

	/// @see gtx_bit
	template<typename genIUType>
	LINEARMATH_FUNC_DECL genIUType highestBitValue(genIUType Value);

	/// @see gtx_bit
	template<typename genIUType>
	LINEARMATH_FUNC_DECL genIUType lowestBitValue(genIUType Value);

	/// Find the highest bit set to 1 in a integer variable and return its value.
	///
	/// @see gtx_bit
	template<length_t L, typename T, qualifier Q>
	LINEARMATH_FUNC_DECL vec<L, T, Q> highestBitValue(vec<L, T, Q> const& value);

	/// Return the power of two number which value is just higher the input value.
	/// Deprecated, use ceilPowerOfTwo from GTC_round instead
	///
	/// @see gtc_round
	/// @see gtx_bit
	template<typename genIUType>
	LINEARMATH_DEPRECATED LINEARMATH_FUNC_DECL genIUType powerOfTwoAbove(genIUType Value);

	/// Return the power of two number which value is just higher the input value.
	/// Deprecated, use ceilPowerOfTwo from GTC_round instead
	///
	/// @see gtc_round
	/// @see gtx_bit
	template<length_t L, typename T, qualifier Q>
	LINEARMATH_DEPRECATED LINEARMATH_FUNC_DECL vec<L, T, Q> powerOfTwoAbove(vec<L, T, Q> const& value);

	/// Return the power of two number which value is just lower the input value.
	/// Deprecated, use floorPowerOfTwo from GTC_round instead
	///
	/// @see gtc_round
	/// @see gtx_bit
	template<typename genIUType>
	LINEARMATH_DEPRECATED LINEARMATH_FUNC_DECL genIUType powerOfTwoBelow(genIUType Value);

	/// Return the power of two number which value is just lower the input value.
	/// Deprecated, use floorPowerOfTwo from GTC_round instead
	///
	/// @see gtc_round
	/// @see gtx_bit
	template<length_t L, typename T, qualifier Q>
	LINEARMATH_DEPRECATED LINEARMATH_FUNC_DECL vec<L, T, Q> powerOfTwoBelow(vec<L, T, Q> const& value);

	/// Return the power of two number which value is the closet to the input value.
	/// Deprecated, use roundPowerOfTwo from GTC_round instead
	///
	/// @see gtc_round
	/// @see gtx_bit
	template<typename genIUType>
	LINEARMATH_DEPRECATED LINEARMATH_FUNC_DECL genIUType powerOfTwoNearest(genIUType Value);

	/// Return the power of two number which value is the closet to the input value.
	/// Deprecated, use roundPowerOfTwo from GTC_round instead
	///
	/// @see gtc_round
	/// @see gtx_bit
	template<length_t L, typename T, qualifier Q>
	LINEARMATH_DEPRECATED LINEARMATH_FUNC_DECL vec<L, T, Q> powerOfTwoNearest(vec<L, T, Q> const& value);

	/// @}
} //namespace linearmath


#include "bit.inl"

