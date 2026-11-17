/// @ref gtx_mixed_product
/// @file linearmath/gtx/mixed_product.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_mixed_product LINEARMATH_GTX_mixed_producte
/// @ingroup gtx
///
/// Include <linearmath/gtx/mixed_product.hpp> to use the features of this extension.
///
/// Mixed product of 3 vectors.

#pragma once

// Dependency:
#include "../linearmath.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_mixed_product is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_mixed_product extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_mixed_product
	/// @{

	/// @brief Mixed product of 3 vectors (from LINEARMATH_GTX_mixed_product extension)
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL T mixedProduct(
		vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2,
		vec<3, T, Q> const& v3);

	/// @}
}// namespace linearmath

#include "mixed_product.inl"
