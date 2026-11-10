/// @ref ext_matrix_common
/// @file rblxmath/ext/matrix_common.hpp
///
/// @defgroup ext_matrix_common RBLXMATH_EXT_matrix_common
/// @ingroup ext
///
/// Defines functions for common matrix operations.
///
/// Include <rblxmath/ext/matrix_common.hpp> to use the features of this extension.
///
/// @see ext_matrix_common

#pragma once

#include "../detail/qualifier.hpp"
#include "../detail/_fixes.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_matrix_common extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_matrix_common
	/// @{

	template<length_t C, length_t R, typename T, typename U, qualifier Q>
	RBLXMATH_FUNC_DECL mat<C, R, T, Q> mix(mat<C, R, T, Q> const& x, mat<C, R, T, Q> const& y, mat<C, R, U, Q> const& a);

	template<length_t C, length_t R, typename T, typename U, qualifier Q>
	RBLXMATH_FUNC_DECL mat<C, R, T, Q> mix(mat<C, R, T, Q> const& x, mat<C, R, T, Q> const& y, U a);

	template <length_t C, length_t R, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR mat<C, R, T, Q> abs(mat<C, R, T, Q> const& x);

	/// @}
}//namespace rblxmath

#include "matrix_common.inl"
