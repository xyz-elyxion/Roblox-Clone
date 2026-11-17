/// @ref gtx_projection

namespace linearmath
{
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType proj(genType const& x, genType const& Normal)
	{
		return linearmath::dot(x, Normal) / linearmath::dot(Normal, Normal) * Normal;
	}
}//namespace linearmath
