/// @ref gtx_perpendicular

namespace linearmath
{
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType perp(genType const& x, genType const& Normal)
	{
		return x - proj(x, Normal);
	}
}//namespace linearmath
