/// @ref gtc_integer
/// @file rblxmath/gtc/integer.hpp
///
/// @see core (dependence)
/// @see gtc_integer (dependence)
///
/// @defgroup gtc_integer RBLXMATH_GTC_integer
/// @ingroup gtc
///
/// Include <rblxmath/gtc/integer.hpp> to use the features of this extension.
///
/// @brief Allow to perform bit operations on integer values

#pragma once

// Dependencies
#include "../detail/setup.hpp"
#include "../detail/qualifier.hpp"
#include "../common.hpp"
#include "../integer.hpp"
#include "../exponential.hpp"
#include "../ext/scalar_common.hpp"
#include "../ext/vector_common.hpp"
#include <limits>

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTC_integer extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtc_integer
	/// @{

	/// Returns the log2 of x for integer values. Useful to compute mipmap count from the texture size.
	/// @see gtc_integer
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<L, T, Q> log2(vec<L, T, Q> const& v);

	/// @}
} //namespace rblxmath

#include "integer.inl"
