/// @ref gtx_exterior_product

#include <limits>

namespace rblxmath {
namespace detail
{
	template<typename T, qualifier Q, bool Aligned>
	struct compute_cross_vec2
	{
		RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR static T call(vec<2, T, Q> const& v, vec<2, T, Q> const& u)
		{
			RBLXMATH_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || RBLXMATH_CONFIG_UNRESTRICTED_FLOAT, "'cross' accepts only floating-point inputs");

			return v.x * u.y - u.x * v.y;
		}
	};
}//namespace detail

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR T cross(vec<2, T, Q> const& x, vec<2, T, Q> const& y)
	{
		return detail::compute_cross_vec2<T, Q, detail::is_aligned<Q>::value>::call(x, y);
	}
}//namespace rblxmath

