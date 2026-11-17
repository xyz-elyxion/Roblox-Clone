/// @ref gtx_scalar_multiplication
/// @file linearmath/gtx/scalar_multiplication.hpp
/// @author Joshua Moerman
///
/// @defgroup gtx_scalar_multiplication LINEARMATH_GTX_scalar_multiplication
/// @ingroup gtx
///
/// Include <linearmath/gtx/scalar_multiplication.hpp> to use the features of this extension.
///
/// Enables scalar multiplication for all types
///
/// Since GLSL is very strict about types, the following (often used) combinations do not work:
///    double * vec4
///    int * vec4
///    vec4 / int
/// So we'll fix that! Of course "float * vec4" should remain the same (hence the enable_if magic)

#pragma once

#include "../detail/setup.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_scalar_multiplication is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_scalar_multiplication extension included")
#endif

#include "../vec2.hpp"
#include "../vec3.hpp"
#include "../vec4.hpp"
#include "../mat2x2.hpp"
#include <type_traits>

namespace linearmath
{
	/// @addtogroup gtx_scalar_multiplication
	/// @{

	template<typename T, typename Vec>
	using return_type_scalar_multiplication = typename std::enable_if<
		!std::is_same<T, float>::value       // T may not be a float
		&& std::is_arithmetic<T>::value, Vec // But it may be an int or double (no vec3 or mat3, ...)
	>::type;

#define LINEARMATH_IMPLEMENT_SCAL_MULT(Vec) \
	template<typename T> \
	return_type_scalar_multiplication<T, Vec> \
	operator*(T const& s, Vec rh){ \
		return rh *= static_cast<float>(s); \
	} \
	 \
	template<typename T> \
	return_type_scalar_multiplication<T, Vec> \
	operator*(Vec lh, T const& s){ \
		return lh *= static_cast<float>(s); \
	} \
	 \
	template<typename T> \
	return_type_scalar_multiplication<T, Vec> \
	operator/(Vec lh, T const& s){ \
		return lh *= 1.0f / static_cast<float>(s); \
	}

LINEARMATH_IMPLEMENT_SCAL_MULT(vec2)
LINEARMATH_IMPLEMENT_SCAL_MULT(vec3)
LINEARMATH_IMPLEMENT_SCAL_MULT(vec4)

LINEARMATH_IMPLEMENT_SCAL_MULT(mat2)
LINEARMATH_IMPLEMENT_SCAL_MULT(mat2x3)
LINEARMATH_IMPLEMENT_SCAL_MULT(mat2x4)
LINEARMATH_IMPLEMENT_SCAL_MULT(mat3x2)
LINEARMATH_IMPLEMENT_SCAL_MULT(mat3)
LINEARMATH_IMPLEMENT_SCAL_MULT(mat3x4)
LINEARMATH_IMPLEMENT_SCAL_MULT(mat4x2)
LINEARMATH_IMPLEMENT_SCAL_MULT(mat4x3)
LINEARMATH_IMPLEMENT_SCAL_MULT(mat4)

#undef LINEARMATH_IMPLEMENT_SCAL_MULT
	/// @}
} // namespace linearmath
