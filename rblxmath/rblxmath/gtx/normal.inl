/// @ref gtx_normal

namespace rblxmath
{
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER vec<3, T, Q> triangleNormal
	(
		vec<3, T, Q> const& p1,
		vec<3, T, Q> const& p2,
		vec<3, T, Q> const& p3
	)
	{
		return normalize(cross(p1 - p2, p1 - p3));
	}
}//namespace rblxmath
