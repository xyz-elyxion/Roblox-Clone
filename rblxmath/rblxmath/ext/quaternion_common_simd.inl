#if RBLXMATH_ARCH & RBLXMATH_ARCH_SSE2_BIT

namespace rblxmath{
namespace detail
{
	template<qualifier Q>
	struct compute_dot<qua<float, Q>, float, true>
	{
		static RBLXMATH_FUNC_QUALIFIER float call(qua<float, Q> const& x, qua<float, Q> const& y)
		{
			return _mm_cvtss_f32(rblxmath_vec1_dot(x.data, y.data));
		}
	};
}//namespace detail
}//namespace rblxmath

#endif//RBLXMATH_ARCH & RBLXMATH_ARCH_SSE2_BIT

