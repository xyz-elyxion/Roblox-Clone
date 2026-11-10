/// @ref simd
/// @file rblxmath/simd/experimental.h

#pragma once

#include "platform.h"

#if RBLXMATH_ARCH & RBLXMATH_ARCH_SSE2_BIT

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec1_sqrt_lowp(rblxmath_f32vec4 x)
{
	return _mm_mul_ss(_mm_rsqrt_ss(x), x);
}

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec4_sqrt_lowp(rblxmath_f32vec4 x)
{
	return _mm_mul_ps(_mm_rsqrt_ps(x), x);
}

#endif//RBLXMATH_ARCH & RBLXMATH_ARCH_SSE2_BIT
