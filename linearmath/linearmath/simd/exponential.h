/// @ref simd
/// @file linearmath/simd/experimental.h

#pragma once

#include "platform.h"

#if LINEARMATH_ARCH & LINEARMATH_ARCH_SSE2_BIT

LINEARMATH_FUNC_QUALIFIER linearmath_f32vec4 linearmath_vec1_sqrt_lowp(linearmath_f32vec4 x)
{
	return _mm_mul_ss(_mm_rsqrt_ss(x), x);
}

LINEARMATH_FUNC_QUALIFIER linearmath_f32vec4 linearmath_vec4_sqrt_lowp(linearmath_f32vec4 x)
{
	return _mm_mul_ps(_mm_rsqrt_ps(x), x);
}

#endif//LINEARMATH_ARCH & LINEARMATH_ARCH_SSE2_BIT
