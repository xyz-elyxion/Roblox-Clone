#if LINEARMATH_ARCH & LINEARMATH_ARCH_SSE2_BIT

namespace linearmath{
namespace detail
{
	template<qualifier Q>
	struct compute_dot<qua<float, Q>, float, true>
	{
		static LINEARMATH_FUNC_QUALIFIER float call(qua<float, Q> const& x, qua<float, Q> const& y)
		{
			return _mm_cvtss_f32(linearmath_vec1_dot(x.data, y.data));
		}
	};
}//namespace detail
}//namespace linearmath

#endif//LINEARMATH_ARCH & LINEARMATH_ARCH_SSE2_BIT

