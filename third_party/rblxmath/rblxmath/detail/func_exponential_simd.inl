/// @ref core
/// @file rblxmath/detail/func_exponential_simd.inl

#include "../simd/exponential.h"

#if RBLXMATH_ARCH & RBLXMATH_ARCH_SSE2_BIT

namespace rblxmath{
namespace detail
{
	template<qualifier Q>
	struct compute_sqrt<4, float, Q, true>
	{
		RBLXMATH_FUNC_QUALIFIER static vec<4, float, Q> call(vec<4, float, Q> const& v)
		{
			vec<4, float, Q> Result;
			Result.data = _mm_sqrt_ps(v.data);
			return Result;
		}
	};

#	if RBLXMATH_CONFIG_ALIGNED_GENTYPES == RBLXMATH_ENABLE
	template<>
	struct compute_sqrt<4, float, aligned_lowp, true>
	{
		RBLXMATH_FUNC_QUALIFIER static vec<4, float, aligned_lowp> call(vec<4, float, aligned_lowp> const& v)
		{
			vec<4, float, aligned_lowp> Result;
			Result.data = rblxmath_vec4_sqrt_lowp(v.data);
			return Result;
		}
	};
#	endif
}//namespace detail
}//namespace rblxmath

#endif//RBLXMATH_ARCH & RBLXMATH_ARCH_SSE2_BIT
