/// @ref gtx_optimum_pow
/// @file rblxmath/gtx/optimum_pow.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_optimum_pow RBLXMATH_GTX_optimum_pow
/// @ingroup gtx
///
/// Include <rblxmath/gtx/optimum_pow.hpp> to use the features of this extension.
///
/// Integer exponentiation of power functions.

#pragma once

// Dependency:
#include "../rblxmath.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_optimum_pow is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_optimum_pow extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_optimum_pow
	/// @{

	/// Returns x raised to the power of 2.
	///
	/// @see gtx_optimum_pow
	template<typename genType>
	RBLXMATH_FUNC_DECL genType pow2(genType const& x);

	/// Returns x raised to the power of 3.
	///
	/// @see gtx_optimum_pow
	template<typename genType>
	RBLXMATH_FUNC_DECL genType pow3(genType const& x);

	/// Returns x raised to the power of 4.
	///
	/// @see gtx_optimum_pow
	template<typename genType>
	RBLXMATH_FUNC_DECL genType pow4(genType const& x);

	/// @}
}//namespace rblxmath

#include "optimum_pow.inl"
