#pragma once

#include "setup.hpp"
#include <limits>

namespace linearmath{
namespace detail
{
	template<typename genFIType, bool /*signed*/>
	struct compute_abs
	{};

	template<typename genFIType>
	struct compute_abs<genFIType, true>
	{
		LINEARMATH_FUNC_QUALIFIER LINEARMATH_CONSTEXPR static genFIType call(genFIType x)
		{
			LINEARMATH_STATIC_ASSERT(
				std::numeric_limits<genFIType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT || std::numeric_limits<genFIType>::is_signed,
				"'abs' only accept floating-point and integer scalar or vector inputs");

			return x >= genFIType(0) ? x : -x;
			// TODO, perf comp with: *(((int *) &x) + 1) &= 0x7fffffff;
		}
	};

#if (LINEARMATH_COMPILER & LINEARMATH_COMPILER_CUDA) || (LINEARMATH_COMPILER & LINEARMATH_COMPILER_HIP)
	template<>
	struct compute_abs<float, true>
	{
		LINEARMATH_FUNC_QUALIFIER static float call(float x)
		{
			return fabsf(x);
		}
	};
#endif

	template<typename genFIType>
	struct compute_abs<genFIType, false>
	{
		LINEARMATH_FUNC_QUALIFIER LINEARMATH_CONSTEXPR static genFIType call(genFIType x)
		{
			LINEARMATH_STATIC_ASSERT(
				(!std::numeric_limits<genFIType>::is_signed && std::numeric_limits<genFIType>::is_integer),
				"'abs' only accept floating-point and integer scalar or vector inputs");
			return x;
		}
	};
}//namespace detail
}//namespace linearmath
