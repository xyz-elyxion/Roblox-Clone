/// @ref simd
/// @file rblxmath/simd/common.h

#pragma once

#include "platform.h"

#if RBLXMATH_ARCH & RBLXMATH_ARCH_SSE2_BIT

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec4_add(rblxmath_f32vec4 a, rblxmath_f32vec4 b)
{
	return _mm_add_ps(a, b);
}

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec1_add(rblxmath_f32vec4 a, rblxmath_f32vec4 b)
{
	return _mm_add_ss(a, b);
}

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec4_sub(rblxmath_f32vec4 a, rblxmath_f32vec4 b)
{
	return _mm_sub_ps(a, b);
}

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec1_sub(rblxmath_f32vec4 a, rblxmath_f32vec4 b)
{
	return _mm_sub_ss(a, b);
}

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec4_mul(rblxmath_f32vec4 a, rblxmath_f32vec4 b)
{
	return _mm_mul_ps(a, b);
}

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec1_mul(rblxmath_f32vec4 a, rblxmath_f32vec4 b)
{
	return _mm_mul_ss(a, b);
}

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec4_div(rblxmath_f32vec4 a, rblxmath_f32vec4 b)
{
	return _mm_div_ps(a, b);
}

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec1_div(rblxmath_f32vec4 a, rblxmath_f32vec4 b)
{
	return _mm_div_ss(a, b);
}

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec4_div_lowp(rblxmath_f32vec4 a, rblxmath_f32vec4 b)
{
	return rblxmath_vec4_mul(a, _mm_rcp_ps(b));
}

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec4_swizzle_xyzw(rblxmath_f32vec4 a)
{
#	if RBLXMATH_ARCH & RBLXMATH_ARCH_AVX2_BIT
		return _mm_permute_ps(a, _MM_SHUFFLE(3, 2, 1, 0));
#	else
		return _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 2, 1, 0));
#	endif
}

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec1_fma(rblxmath_f32vec4 a, rblxmath_f32vec4 b, rblxmath_f32vec4 c)
{
#	if (RBLXMATH_ARCH & RBLXMATH_ARCH_AVX2_BIT) && !(RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG)
		return _mm_fmadd_ss(a, b, c);
#	else
		return _mm_add_ss(_mm_mul_ss(a, b), c);
#	endif
}

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec4_fma(rblxmath_f32vec4 a, rblxmath_f32vec4 b, rblxmath_f32vec4 c)
{
#	if (RBLXMATH_ARCH & RBLXMATH_ARCH_AVX2_BIT) && !(RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG)
		return _mm_fmadd_ps(a, b, c);
#	else
		return rblxmath_vec4_add(rblxmath_vec4_mul(a, b), c);
#	endif
}

RBLXMATH_FUNC_QUALIFIER rblxmath_f32vec4 rblxmath_vec4_abs(rblxmath_f32vec4 x)
{
	return _mm_and_ps(x, _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF)));
}

RBLXMATH_FUNC_QUALIFIER rblxmath_ivec4 rblxmath_ivec4_abs(rblxmath_ivec4 x)
{
#	if RBLXMATH_ARCH & RBLXMATH_ARCH_SSSE3_BIT
		return _mm_sign_epi32(x, x);
#	else
		rblxmath_ivec4 const sgn0 = _mm_srai_epi32(x, 31);
		rblxmath_ivec4 const inv0 = _mm_xor_si128(x, sgn0);
		rblxmath_ivec4 const sub0 = _mm_sub_epi32(inv0, sgn0);
		return sub0;
#	endif
}

RBLXMATH_FUNC_QUALIFIER rblxmath_vec4 rblxmath_vec4_sign(rblxmath_vec4 x)
{
	rblxmath_vec4 const zro0 = _mm_setzero_ps();
	rblxmath_vec4 const cmp0 = _mm_cmplt_ps(x, zro0);
	rblxmath_vec4 const cmp1 = _mm_cmpgt_ps(x, zro0);
	rblxmath_vec4 const and0 = _mm_and_ps(cmp0, _mm_set1_ps(-1.0f));
	rblxmath_vec4 const and1 = _mm_and_ps(cmp1, _mm_set1_ps(1.0f));
	rblxmath_vec4 const or0 = _mm_or_ps(and0, and1);
	return or0;
}

RBLXMATH_FUNC_QUALIFIER rblxmath_vec4 rblxmath_vec4_round(rblxmath_vec4 x)
{
#	if RBLXMATH_ARCH & RBLXMATH_ARCH_SSE41_BIT
		return _mm_round_ps(x, _MM_FROUND_TO_NEAREST_INT);
#	else
		rblxmath_vec4 const sgn0 = _mm_castsi128_ps(_mm_set1_epi32(int(0x80000000)));
		rblxmath_vec4 const and0 = _mm_and_ps(sgn0, x);
		rblxmath_vec4 const or0 = _mm_or_ps(and0, _mm_set_ps1(8388608.0f));
		rblxmath_vec4 const add0 = rblxmath_vec4_add(x, or0);
		rblxmath_vec4 const sub0 = rblxmath_vec4_sub(add0, or0);
		return sub0;
#	endif
}

RBLXMATH_FUNC_QUALIFIER rblxmath_vec4 rblxmath_vec4_floor(rblxmath_vec4 x)
{
#	if RBLXMATH_ARCH & RBLXMATH_ARCH_SSE41_BIT
		return _mm_floor_ps(x);
#	else
		rblxmath_vec4 const rnd0 = rblxmath_vec4_round(x);
		rblxmath_vec4 const cmp0 = _mm_cmplt_ps(x, rnd0);
		rblxmath_vec4 const and0 = _mm_and_ps(cmp0, _mm_set1_ps(1.0f));
		rblxmath_vec4 const sub0 = rblxmath_vec4_sub(rnd0, and0);
		return sub0;
#	endif
}

/* trunc TODO
RBLXMATH_FUNC_QUALIFIER rblxmath_vec4 rblxmath_vec4_trunc(rblxmath_vec4 x)
{
	return rblxmath_vec4();
}
*/

//roundEven
RBLXMATH_FUNC_QUALIFIER rblxmath_vec4 rblxmath_vec4_roundEven(rblxmath_vec4 x)
{
	rblxmath_vec4 const sgn0 = _mm_castsi128_ps(_mm_set1_epi32(int(0x80000000)));
	rblxmath_vec4 const and0 = _mm_and_ps(sgn0, x);
	rblxmath_vec4 const or0 = _mm_or_ps(and0, _mm_set_ps1(8388608.0f));
	rblxmath_vec4 const add0 = rblxmath_vec4_add(x, or0);
	rblxmath_vec4 const sub0 = rblxmath_vec4_sub(add0, or0);
	return sub0;
}

RBLXMATH_FUNC_QUALIFIER rblxmath_vec4 rblxmath_vec4_ceil(rblxmath_vec4 x)
{
#	if RBLXMATH_ARCH & RBLXMATH_ARCH_SSE41_BIT
		return _mm_ceil_ps(x);
#	else
		rblxmath_vec4 const rnd0 = rblxmath_vec4_round(x);
		rblxmath_vec4 const cmp0 = _mm_cmpgt_ps(x, rnd0);
		rblxmath_vec4 const and0 = _mm_and_ps(cmp0, _mm_set1_ps(1.0f));
		rblxmath_vec4 const add0 = rblxmath_vec4_add(rnd0, and0);
		return add0;
#	endif
}

RBLXMATH_FUNC_QUALIFIER rblxmath_vec4 rblxmath_vec4_fract(rblxmath_vec4 x)
{
	rblxmath_vec4 const flr0 = rblxmath_vec4_floor(x);
	rblxmath_vec4 const sub0 = rblxmath_vec4_sub(x, flr0);
	return sub0;
}

RBLXMATH_FUNC_QUALIFIER rblxmath_vec4 rblxmath_vec4_mod(rblxmath_vec4 x, rblxmath_vec4 y)
{
	rblxmath_vec4 const div0 = rblxmath_vec4_div(x, y);
	rblxmath_vec4 const flr0 = rblxmath_vec4_floor(div0);
	rblxmath_vec4 const mul0 = rblxmath_vec4_mul(y, flr0);
	rblxmath_vec4 const sub0 = rblxmath_vec4_sub(x, mul0);
	return sub0;
}

RBLXMATH_FUNC_QUALIFIER rblxmath_vec4 rblxmath_vec4_clamp(rblxmath_vec4 v, rblxmath_vec4 minVal, rblxmath_vec4 maxVal)
{
	rblxmath_vec4 const min0 = _mm_min_ps(v, maxVal);
	rblxmath_vec4 const max0 = _mm_max_ps(min0, minVal);
	return max0;
}

RBLXMATH_FUNC_QUALIFIER rblxmath_vec4 rblxmath_vec4_mix(rblxmath_vec4 v1, rblxmath_vec4 v2, rblxmath_vec4 a)
{
	rblxmath_vec4 const sub0 = rblxmath_vec4_sub(_mm_set1_ps(1.0f), a);
	rblxmath_vec4 const mul0 = rblxmath_vec4_mul(v1, sub0);
	rblxmath_vec4 const mad0 = rblxmath_vec4_fma(v2, a, mul0);
	return mad0;
}

RBLXMATH_FUNC_QUALIFIER rblxmath_vec4 rblxmath_vec4_step(rblxmath_vec4 edge, rblxmath_vec4 x)
{
	rblxmath_vec4 const cmp = _mm_cmple_ps(x, edge);
	return _mm_movemask_ps(cmp) == 0 ? _mm_set1_ps(1.0f) : _mm_setzero_ps();
}

RBLXMATH_FUNC_QUALIFIER rblxmath_vec4 rblxmath_vec4_smoothstep(rblxmath_vec4 edge0, rblxmath_vec4 edge1, rblxmath_vec4 x)
{
	rblxmath_vec4 const sub0 = rblxmath_vec4_sub(x, edge0);
	rblxmath_vec4 const sub1 = rblxmath_vec4_sub(edge1, edge0);
	rblxmath_vec4 const div0 = rblxmath_vec4_div(sub0, sub1);
	rblxmath_vec4 const clp0 = rblxmath_vec4_clamp(div0, _mm_setzero_ps(), _mm_set1_ps(1.0f));
	rblxmath_vec4 const mul0 = rblxmath_vec4_mul(_mm_set1_ps(2.0f), clp0);
	rblxmath_vec4 const sub2 = rblxmath_vec4_sub(_mm_set1_ps(3.0f), mul0);
	rblxmath_vec4 const mul1 = rblxmath_vec4_mul(clp0, clp0);
	rblxmath_vec4 const mul2 = rblxmath_vec4_mul(mul1, sub2);
	return mul2;
}

// Agner Fog method
RBLXMATH_FUNC_QUALIFIER rblxmath_vec4 rblxmath_vec4_nan(rblxmath_vec4 x)
{
	rblxmath_ivec4 const t1 = _mm_castps_si128(x);						// reinterpret as 32-bit integer
	rblxmath_ivec4 const t2 = _mm_sll_epi32(t1, _mm_cvtsi32_si128(1));	// shift out sign bit
	rblxmath_ivec4 const t3 = _mm_set1_epi32(int(0xFF000000));				// exponent mask
	rblxmath_ivec4 const t4 = _mm_and_si128(t2, t3);						// exponent
	rblxmath_ivec4 const t5 = _mm_andnot_si128(t3, t2);					// fraction
	rblxmath_ivec4 const Equal = _mm_cmpeq_epi32(t3, t4);
	rblxmath_ivec4 const Nequal = _mm_cmpeq_epi32(t5, _mm_setzero_si128());
	rblxmath_ivec4 const And = _mm_and_si128(Equal, Nequal);
	return _mm_castsi128_ps(And);									// exponent = all 1s and fraction != 0
}

// Agner Fog method
RBLXMATH_FUNC_QUALIFIER rblxmath_vec4 rblxmath_vec4_inf(rblxmath_vec4 x)
{
	rblxmath_ivec4 const t1 = _mm_castps_si128(x);										// reinterpret as 32-bit integer
	rblxmath_ivec4 const t2 = _mm_sll_epi32(t1, _mm_cvtsi32_si128(1));					// shift out sign bit
	return _mm_castsi128_ps(_mm_cmpeq_epi32(t2, _mm_set1_epi32(int(0xFF000000))));		// exponent is all 1s, fraction is 0
}

#endif//RBLXMATH_ARCH & RBLXMATH_ARCH_SSE2_BIT
