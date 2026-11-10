/// @ref gtx_fast_square_root
/// @file rblxmath/gtx/fast_square_root.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_fast_square_root RBLXMATH_GTX_fast_square_root
/// @ingroup gtx
///
/// Include <rblxmath/gtx/fast_square_root.hpp> to use the features of this extension.
///
/// Fast but less accurate implementations of square root based functions.
/// - Sqrt optimisation based on Newton's method,
/// www.gamedev.net/community/forums/topic.asp?topic id=139956

#pragma once

// Dependency:
#include "../common.hpp"
#include "../exponential.hpp"
#include "../geometric.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_fast_square_root is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_fast_square_root extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_fast_square_root
	/// @{

	/// Faster than the common sqrt function but less accurate.
	///
	/// @see gtx_fast_square_root extension.
	template<typename genType>
	RBLXMATH_FUNC_DECL genType fastSqrt(genType x);

	/// Faster than the common sqrt function but less accurate.
	///
	/// @see gtx_fast_square_root extension.
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<L, T, Q> fastSqrt(vec<L, T, Q> const& x);

	/// Faster than the common inversesqrt function but less accurate.
	///
	/// @see gtx_fast_square_root extension.
	template<typename genType>
	RBLXMATH_FUNC_DECL genType fastInverseSqrt(genType x);

	/// Faster than the common inversesqrt function but less accurate.
	///
	/// @see gtx_fast_square_root extension.
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<L, T, Q> fastInverseSqrt(vec<L, T, Q> const& x);

	/// Faster than the common length function but less accurate.
	///
	/// @see gtx_fast_square_root extension.
	template<typename genType>
	RBLXMATH_FUNC_DECL genType fastLength(genType x);

	/// Faster than the common length function but less accurate.
	///
	/// @see gtx_fast_square_root extension.
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL T fastLength(vec<L, T, Q> const& x);

	/// Faster than the common distance function but less accurate.
	///
	/// @see gtx_fast_square_root extension.
	template<typename genType>
	RBLXMATH_FUNC_DECL genType fastDistance(genType x, genType y);

	/// Faster than the common distance function but less accurate.
	///
	/// @see gtx_fast_square_root extension.
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL T fastDistance(vec<L, T, Q> const& x, vec<L, T, Q> const& y);

	/// Faster than the common normalize function but less accurate.
	///
	/// @see gtx_fast_square_root extension.
	template<typename genType>
	RBLXMATH_FUNC_DECL genType fastNormalize(genType x);

	/// Faster than the common normalize function but less accurate.
	///
	/// @see gtx_fast_square_root extension.
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<L, T, Q> fastNormalize(vec<L, T, Q> const& x);

	/// @}
}// namespace rblxmath

#include "fast_square_root.inl"
