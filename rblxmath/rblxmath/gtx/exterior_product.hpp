/// @ref gtx_exterior_product
/// @file rblxmath/gtx/exterior_product.hpp
///
/// @see core (dependence)
/// @see gtx_exterior_product (dependence)
///
/// @defgroup gtx_exterior_product RBLXMATH_GTX_exterior_product
/// @ingroup gtx
///
/// Include <rblxmath/gtx/exterior_product.hpp> to use the features of this extension.
///
/// @brief Allow to perform bit operations on integer values

#pragma once

// Dependencies
#include "../detail/setup.hpp"
#include "../detail/qualifier.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_exterior_product is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_exterior_product extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_exterior_product
	/// @{

	/// Returns the cross product of x and y.
	///
	/// @tparam T Floating-point scalar types
	/// @tparam Q Value from qualifier enum
	///
	/// @see <a href="https://en.wikipedia.org/wiki/Exterior_algebra#Cross_and_triple_products">Exterior product</a>
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR T cross(vec<2, T, Q> const& v, vec<2, T, Q> const& u);

	/// @}
} //namespace rblxmath

#include "exterior_product.inl"
