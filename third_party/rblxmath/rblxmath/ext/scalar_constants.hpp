/// @ref ext_scalar_constants
/// @file rblxmath/ext/scalar_constants.hpp
///
/// @defgroup ext_scalar_constants RBLXMATH_EXT_scalar_constants
/// @ingroup ext
///
/// Provides a list of constants and precomputed useful values.
///
/// Include <rblxmath/ext/scalar_constants.hpp> to use the features of this extension.

#pragma once

// Dependencies
#include "../detail/setup.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_scalar_constants extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_scalar_constants
	/// @{

	/// Return the epsilon constant for floating point types.
	template<typename genType>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR genType epsilon();

	/// Return the pi constant for floating point types.
	template<typename genType>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR genType pi();

	/// Return the value of cos(1 / 2) for floating point types.
	template<typename genType>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR genType cos_one_over_two();

	/// @}
} //namespace rblxmath

#include "scalar_constants.inl"
