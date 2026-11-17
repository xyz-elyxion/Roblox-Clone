/// @ref gtx_normalize_dot

namespace linearmath
{
	template<length_t L, typename T, qualifier Q>
	LINEARMATH_FUNC_QUALIFIER T normalizeDot(vec<L, T, Q> const& x, vec<L, T, Q> const& y)
	{
		return linearmath::dot(x, y) * linearmath::inversesqrt(linearmath::dot(x, x) * linearmath::dot(y, y));
	}

	template<length_t L, typename T, qualifier Q>
	LINEARMATH_FUNC_QUALIFIER T fastNormalizeDot(vec<L, T, Q> const& x, vec<L, T, Q> const& y)
	{
		return linearmath::dot(x, y) * linearmath::fastInverseSqrt(linearmath::dot(x, x) * linearmath::dot(y, y));
	}
}//namespace linearmath
