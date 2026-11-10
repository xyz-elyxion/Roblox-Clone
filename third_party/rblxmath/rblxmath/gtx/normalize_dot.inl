/// @ref gtx_normalize_dot

namespace rblxmath
{
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER T normalizeDot(vec<L, T, Q> const& x, vec<L, T, Q> const& y)
	{
		return rblxmath::dot(x, y) * rblxmath::inversesqrt(rblxmath::dot(x, x) * rblxmath::dot(y, y));
	}

	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER T fastNormalizeDot(vec<L, T, Q> const& x, vec<L, T, Q> const& y)
	{
		return rblxmath::dot(x, y) * rblxmath::fastInverseSqrt(rblxmath::dot(x, x) * rblxmath::dot(y, y));
	}
}//namespace rblxmath
