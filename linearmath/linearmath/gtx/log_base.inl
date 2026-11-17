/// @ref gtx_log_base

namespace linearmath
{
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType log(genType const& x, genType const& base)
	{
		return linearmath::log(x) / linearmath::log(base);
	}

	template<length_t L, typename T, qualifier Q>
	LINEARMATH_FUNC_QUALIFIER vec<L, T, Q> log(vec<L, T, Q> const& x, vec<L, T, Q> const& base)
	{
		return linearmath::log(x) / linearmath::log(base);
	}
}//namespace linearmath
