/// @ref core
/// @file linearmath/detail/func_exponential_simd.inl

#include "../simd/exponential.h"

#if LINEARMATH_ARCH & LINEARMATH_ARCH_SSE2_BIT

namespace linearmath{
namespace detail
{
	template<qualifier Q>
	struct compute_sqrt<4, float, Q, true>
	{
		LINEARMATH_FUNC_QUALIFIER static vec<4, float, Q> call(vec<4, float, Q> const& v)
		{
			vec<4, float, Q> Result;
			Result.data = _mm_sqrt_ps(v.data);
			return Result;
		}
	};

#	if LINEARMATH_CONFIG_ALIGNED_GENTYPES == LINEARMATH_ENABLE
	template<>
	struct compute_sqrt<4, float, aligned_lowp, true>
	{
		LINEARMATH_FUNC_QUALIFIER static vec<4, float, aligned_lowp> call(vec<4, float, aligned_lowp> const& v)
		{
			vec<4, float, aligned_lowp> Result;
			Result.data = linearmath_vec4_sqrt_lowp(v.data);
			return Result;
		}
	};
#	endif
}//namespace detail
}//namespace linearmath

#endif//LINEARMATH_ARCH & LINEARMATH_ARCH_SSE2_BIT
