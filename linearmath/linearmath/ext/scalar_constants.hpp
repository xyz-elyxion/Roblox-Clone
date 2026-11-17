/// @ref ext_scalar_constants
/// @file linearmath/ext/scalar_constants.hpp
///
/// @defgroup ext_scalar_constants LINEARMATH_EXT_scalar_constants
/// @ingroup ext
///
/// Provides a list of constants and precomputed useful values.
///
/// Include <linearmath/ext/scalar_constants.hpp> to use the features of this extension.

#pragma once

// Dependencies
#include "../detail/setup.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_scalar_constants extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_scalar_constants
	/// @{

	/// Return the epsilon constant for floating point types.
	template<typename genType>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR genType epsilon();

	/// Return the pi constant for floating point types.
	template<typename genType>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR genType pi();

	/// Return the value of cos(1 / 2) for floating point types.
	template<typename genType>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR genType cos_one_over_two();

	/// @}
} //namespace linearmath

#include "scalar_constants.inl"
