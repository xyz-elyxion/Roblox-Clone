/// @ref gtc_color_space
/// @file rblxmath/gtc/color_space.hpp
///
/// @see core (dependence)
/// @see gtc_color_space (dependence)
///
/// @defgroup gtc_color_space RBLXMATH_GTC_color_space
/// @ingroup gtc
///
/// Include <rblxmath/gtc/color_space.hpp> to use the features of this extension.
///
/// Allow to perform bit operations on integer values

#pragma once

// Dependencies
#include "../detail/setup.hpp"
#include "../detail/qualifier.hpp"
#include "../exponential.hpp"
#include "../vec3.hpp"
#include "../vec4.hpp"
#include <limits>

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTC_color_space extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtc_color_space
	/// @{

	/// Convert a linear color to sRGB color using a standard gamma correction.
	/// IEC 61966-2-1:1999 / Rec. 709 specification https://www.w3.org/Graphics/Color/srgb
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<L, T, Q> convertLinearToSRGB(vec<L, T, Q> const& ColorLinear);

	/// Convert a linear color to sRGB color using a custom gamma correction.
	/// IEC 61966-2-1:1999 / Rec. 709 specification https://www.w3.org/Graphics/Color/srgb
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<L, T, Q> convertLinearToSRGB(vec<L, T, Q> const& ColorLinear, T Gamma);

	/// Convert a sRGB color to linear color using a standard gamma correction.
	/// IEC 61966-2-1:1999 / Rec. 709 specification https://www.w3.org/Graphics/Color/srgb
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<L, T, Q> convertSRGBToLinear(vec<L, T, Q> const& ColorSRGB);

	/// Convert a sRGB color to linear color using a custom gamma correction.
	// IEC 61966-2-1:1999 / Rec. 709 specification https://www.w3.org/Graphics/Color/srgb
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<L, T, Q> convertSRGBToLinear(vec<L, T, Q> const& ColorSRGB, T Gamma);

	/// @}
} //namespace rblxmath

#include "color_space.inl"
