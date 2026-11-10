/// @ref gtx_log_base

namespace rblxmath
{
	template<typename genType>
	RBLXMATH_FUNC_QUALIFIER genType log(genType const& x, genType const& base)
	{
		return rblxmath::log(x) / rblxmath::log(base);
	}

	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER vec<L, T, Q> log(vec<L, T, Q> const& x, vec<L, T, Q> const& base)
	{
		return rblxmath::log(x) / rblxmath::log(base);
	}
}//namespace rblxmath
