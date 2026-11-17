/// @ref gtc_noise
/// @file linearmath/gtc/noise.hpp
///
/// @see core (dependence)
///
/// @defgroup gtc_noise LINEARMATH_GTC_noise
/// @ingroup gtc
///
/// Include <linearmath/gtc/noise.hpp> to use the features of this extension.
///
/// Defines 2D, 3D and 4D procedural noise functions
/// Based on the work of Stefan Gustavson and Ashima Arts on "webgl-noise":
/// https://github.com/ashima/webgl-noise
/// Following Stefan Gustavson's paper "Simplex noise demystified":
/// http://www.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf

#pragma once

// Dependencies
#include "../detail/setup.hpp"
#include "../detail/qualifier.hpp"
#include "../detail/_noise.hpp"
#include "../geometric.hpp"
#include "../common.hpp"
#include "../vector_relational.hpp"
#include "../vec2.hpp"
#include "../vec3.hpp"
#include "../vec4.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTC_noise extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtc_noise
	/// @{

	/// Classic perlin noise.
	/// @see gtc_noise
	template<length_t L, typename T, qualifier Q>
	LINEARMATH_FUNC_DECL T perlin(
		vec<L, T, Q> const& p);

	/// Periodic perlin noise.
	/// @see gtc_noise
	template<length_t L, typename T, qualifier Q>
	LINEARMATH_FUNC_DECL T perlin(
		vec<L, T, Q> const& p,
		vec<L, T, Q> const& rep);

	/// Simplex noise.
	/// @see gtc_noise
	template<length_t L, typename T, qualifier Q>
	LINEARMATH_FUNC_DECL T simplex(
		vec<L, T, Q> const& p);

	/// @}
}//namespace linearmath

#include "noise.inl"
