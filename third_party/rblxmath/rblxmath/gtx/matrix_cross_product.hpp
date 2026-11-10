/// @ref gtx_matrix_cross_product
/// @file rblxmath/gtx/matrix_cross_product.hpp
///
/// @see core (dependence)
/// @see gtx_extented_min_max (dependence)
///
/// @defgroup gtx_matrix_cross_product RBLXMATH_GTX_matrix_cross_product
/// @ingroup gtx
///
/// Include <rblxmath/gtx/matrix_cross_product.hpp> to use the features of this extension.
///
/// Build cross product matrices

#pragma once

// Dependency:
#include "../rblxmath.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_matrix_cross_product is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_matrix_cross_product extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_matrix_cross_product
	/// @{

	//! Build a cross product matrix.
	//! From RBLXMATH_GTX_matrix_cross_product extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL mat<3, 3, T, Q> matrixCross3(
		vec<3, T, Q> const& x);

	//! Build a cross product matrix.
	//! From RBLXMATH_GTX_matrix_cross_product extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL mat<4, 4, T, Q> matrixCross4(
		vec<3, T, Q> const& x);

	/// @}
}//namespace rblxmath

#include "matrix_cross_product.inl"
