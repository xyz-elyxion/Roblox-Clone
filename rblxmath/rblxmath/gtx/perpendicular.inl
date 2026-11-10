/// @ref gtx_perpendicular

namespace rblxmath
{
	template<typename genType>
	RBLXMATH_FUNC_QUALIFIER genType perp(genType const& x, genType const& Normal)
	{
		return x - proj(x, Normal);
	}
}//namespace rblxmath
