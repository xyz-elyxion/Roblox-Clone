/// @ref gtx_scalar_multiplication
/// @file rblxmath/gtx/scalar_multiplication.hpp
/// @author Joshua Moerman
///
/// @defgroup gtx_scalar_multiplication RBLXMATH_GTX_scalar_multiplication
/// @ingroup gtx
///
/// Include <rblxmath/gtx/scalar_multiplication.hpp> to use the features of this extension.
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

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_scalar_multiplication is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_scalar_multiplication extension included")
#endif

#include "../vec2.hpp"
#include "../vec3.hpp"
#include "../vec4.hpp"
#include "../mat2x2.hpp"
#include <type_traits>

namespace rblxmath
{
	/// @addtogroup gtx_scalar_multiplication
	/// @{

	template<typename T, typename Vec>
	using return_type_scalar_multiplication = typename std::enable_if<
		!std::is_same<T, float>::value       // T may not be a float
		&& std::is_arithmetic<T>::value, Vec // But it may be an int or double (no vec3 or mat3, ...)
	>::type;

#define RBLXMATH_IMPLEMENT_SCAL_MULT(Vec) \
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

RBLXMATH_IMPLEMENT_SCAL_MULT(vec2)
RBLXMATH_IMPLEMENT_SCAL_MULT(vec3)
RBLXMATH_IMPLEMENT_SCAL_MULT(vec4)

RBLXMATH_IMPLEMENT_SCAL_MULT(mat2)
RBLXMATH_IMPLEMENT_SCAL_MULT(mat2x3)
RBLXMATH_IMPLEMENT_SCAL_MULT(mat2x4)
RBLXMATH_IMPLEMENT_SCAL_MULT(mat3x2)
RBLXMATH_IMPLEMENT_SCAL_MULT(mat3)
RBLXMATH_IMPLEMENT_SCAL_MULT(mat3x4)
RBLXMATH_IMPLEMENT_SCAL_MULT(mat4x2)
RBLXMATH_IMPLEMENT_SCAL_MULT(mat4x3)
RBLXMATH_IMPLEMENT_SCAL_MULT(mat4)

#undef RBLXMATH_IMPLEMENT_SCAL_MULT
	/// @}
} // namespace rblxmath
