/// @ref gtx_projection

namespace rblxmath
{
	template<typename genType>
	RBLXMATH_FUNC_QUALIFIER genType proj(genType const& x, genType const& Normal)
	{
		return rblxmath::dot(x, Normal) / rblxmath::dot(Normal, Normal) * Normal;
	}
}//namespace rblxmath
