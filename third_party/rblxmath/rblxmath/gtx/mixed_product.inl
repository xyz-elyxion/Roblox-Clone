/// @ref gtx_mixed_product

namespace rblxmath
{
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER T mixedProduct
	(
		vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2,
		vec<3, T, Q> const& v3
	)
	{
		return dot(cross(v1, v2), v3);
	}
}//namespace rblxmath
