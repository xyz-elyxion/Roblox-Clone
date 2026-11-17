/// @ref ext_matrix_common
/// @file linearmath/ext/matrix_common.hpp
///
/// @defgroup ext_matrix_common LINEARMATH_EXT_matrix_common
/// @ingroup ext
///
/// Defines functions for common matrix operations.
///
/// Include <linearmath/ext/matrix_common.hpp> to use the features of this extension.
///
/// @see ext_matrix_common

#pragma once

#include "../detail/qualifier.hpp"
#include "../detail/_fixes.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_matrix_common extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_matrix_common
	/// @{

	template<length_t C, length_t R, typename T, typename U, qualifier Q>
	LINEARMATH_FUNC_DECL mat<C, R, T, Q> mix(mat<C, R, T, Q> const& x, mat<C, R, T, Q> const& y, mat<C, R, U, Q> const& a);

	template<length_t C, length_t R, typename T, typename U, qualifier Q>
	LINEARMATH_FUNC_DECL mat<C, R, T, Q> mix(mat<C, R, T, Q> const& x, mat<C, R, T, Q> const& y, U a);

	template <length_t C, length_t R, typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<C, R, T, Q> abs(mat<C, R, T, Q> const& x);

	/// @}
}//namespace linearmath

#include "matrix_common.inl"
