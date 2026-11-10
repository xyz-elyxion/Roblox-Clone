#pragma once

#include "setup.hpp"
#include <limits>

namespace rblxmath{
namespace detail
{
	template<typename genFIType, bool /*signed*/>
	struct compute_abs
	{};

	template<typename genFIType>
	struct compute_abs<genFIType, true>
	{
		RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR static genFIType call(genFIType x)
		{
			RBLXMATH_STATIC_ASSERT(
				std::numeric_limits<genFIType>::is_iec559 || RBLXMATH_CONFIG_UNRESTRICTED_FLOAT || std::numeric_limits<genFIType>::is_signed,
				"'abs' only accept floating-point and integer scalar or vector inputs");

			return x >= genFIType(0) ? x : -x;
			// TODO, perf comp with: *(((int *) &x) + 1) &= 0x7fffffff;
		}
	};

#if (RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA) || (RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP)
	template<>
	struct compute_abs<float, true>
	{
		RBLXMATH_FUNC_QUALIFIER static float call(float x)
		{
			return fabsf(x);
		}
	};
#endif

	template<typename genFIType>
	struct compute_abs<genFIType, false>
	{
		RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR static genFIType call(genFIType x)
		{
			RBLXMATH_STATIC_ASSERT(
				(!std::numeric_limits<genFIType>::is_signed && std::numeric_limits<genFIType>::is_integer),
				"'abs' only accept floating-point and integer scalar or vector inputs");
			return x;
		}
	};
}//namespace detail
}//namespace rblxmath
