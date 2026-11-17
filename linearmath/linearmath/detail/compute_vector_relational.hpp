#pragma once

//#include "compute_common.hpp"
#include "setup.hpp"
#include <limits>

namespace linearmath{
namespace detail
{
	template <typename T, bool isFloat>
	struct compute_equal
	{
		LINEARMATH_FUNC_QUALIFIER LINEARMATH_CONSTEXPR static bool call(T a, T b)
		{
			return a == b;
		}
	};
/*
	template <typename T>
	struct compute_equal<T, true>
	{
		LINEARMATH_FUNC_QUALIFIER LINEARMATH_CONSTEXPR static bool call(T a, T b)
		{
			return detail::compute_abs<T, std::numeric_limits<T>::is_signed>::call(b - a) <= static_cast<T>(0);
			//return std::memcmp(&a, &b, sizeof(T)) == 0;
		}
	};
*/
}//namespace detail
}//namespace linearmath
