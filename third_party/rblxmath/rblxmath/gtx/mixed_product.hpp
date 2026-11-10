/// @ref gtx_mixed_product
/// @file rblxmath/gtx/mixed_product.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_mixed_product RBLXMATH_GTX_mixed_producte
/// @ingroup gtx
///
/// Include <rblxmath/gtx/mixed_product.hpp> to use the features of this extension.
///
/// Mixed product of 3 vectors.

#pragma once

// Dependency:
#include "../rblxmath.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_mixed_product is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_mixed_product extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_mixed_product
	/// @{

	/// @brief Mixed product of 3 vectors (from RBLXMATH_GTX_mixed_product extension)
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL T mixedProduct(
		vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2,
		vec<3, T, Q> const& v3);

	/// @}
}// namespace rblxmath

#include "mixed_product.inl"
