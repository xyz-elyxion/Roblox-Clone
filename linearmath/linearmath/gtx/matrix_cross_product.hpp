/// @ref gtx_matrix_cross_product
/// @file linearmath/gtx/matrix_cross_product.hpp
///
/// @see core (dependence)
/// @see gtx_extented_min_max (dependence)
///
/// @defgroup gtx_matrix_cross_product LINEARMATH_GTX_matrix_cross_product
/// @ingroup gtx
///
/// Include <linearmath/gtx/matrix_cross_product.hpp> to use the features of this extension.
///
/// Build cross product matrices

#pragma once

// Dependency:
#include "../linearmath.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_matrix_cross_product is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_matrix_cross_product extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_matrix_cross_product
	/// @{

	//! Build a cross product matrix.
	//! From LINEARMATH_GTX_matrix_cross_product extension.
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL mat<3, 3, T, Q> matrixCross3(
		vec<3, T, Q> const& x);

	//! Build a cross product matrix.
	//! From LINEARMATH_GTX_matrix_cross_product extension.
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL mat<4, 4, T, Q> matrixCross4(
		vec<3, T, Q> const& x);

	/// @}
}//namespace linearmath

#include "matrix_cross_product.inl"
