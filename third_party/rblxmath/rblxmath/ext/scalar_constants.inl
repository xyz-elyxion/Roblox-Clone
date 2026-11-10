#include <limits>

namespace rblxmath
{
	template<typename genType>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR genType epsilon()
	{
		RBLXMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || RBLXMATH_CONFIG_UNRESTRICTED_FLOAT, "'epsilon' only accepts floating-point inputs");
		return std::numeric_limits<genType>::epsilon();
	}

	template<typename genType>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR genType pi()
	{
		RBLXMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || RBLXMATH_CONFIG_UNRESTRICTED_FLOAT, "'pi' only accepts floating-point inputs");
		return static_cast<genType>(3.14159265358979323846264338327950288);
	}

	template<typename genType>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR genType cos_one_over_two()
	{
		return genType(0.877582561890372716130286068203503191);
	}
} //namespace rblxmath
