/// @ref gtx_optimum_pow
/// @file linearmath/gtx/optimum_pow.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_optimum_pow LINEARMATH_GTX_optimum_pow
/// @ingroup gtx
///
/// Include <linearmath/gtx/optimum_pow.hpp> to use the features of this extension.
///
/// Integer exponentiation of power functions.

#pragma once

// Dependency:
#include "../linearmath.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_optimum_pow is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_optimum_pow extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_optimum_pow
	/// @{

	/// Returns x raised to the power of 2.
	///
	/// @see gtx_optimum_pow
	template<typename genType>
	LINEARMATH_FUNC_DECL genType pow2(genType const& x);

	/// Returns x raised to the power of 3.
	///
	/// @see gtx_optimum_pow
	template<typename genType>
	LINEARMATH_FUNC_DECL genType pow3(genType const& x);

	/// Returns x raised to the power of 4.
	///
	/// @see gtx_optimum_pow
	template<typename genType>
	LINEARMATH_FUNC_DECL genType pow4(genType const& x);

	/// @}
}//namespace linearmath

#include "optimum_pow.inl"
