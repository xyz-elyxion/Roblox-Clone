/// @ref ext_scalar_reciprocal

#include "../trigonometric.hpp"
#include <limits>

namespace linearmath
{
	// sec
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType sec(genType angle)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'sec' only accept floating-point values");
		return genType(1) / linearmath::cos(angle);
	}

	// csc
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType csc(genType angle)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'csc' only accept floating-point values");
		return genType(1) / linearmath::sin(angle);
	}

	// cot
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType cot(genType angle)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'cot' only accept floating-point values");

		genType const pi_over_2 = genType(3.1415926535897932384626433832795 / 2.0);
		return linearmath::tan(pi_over_2 - angle);
	}

	// asec
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType asec(genType x)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'asec' only accept floating-point values");
		return acos(genType(1) / x);
	}

	// acsc
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType acsc(genType x)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'acsc' only accept floating-point values");
		return asin(genType(1) / x);
	}

	// acot
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType acot(genType x)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'acot' only accept floating-point values");

		genType const pi_over_2 = genType(3.1415926535897932384626433832795 / 2.0);
		return pi_over_2 - atan(x);
	}

	// sech
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType sech(genType angle)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'sech' only accept floating-point values");
		return genType(1) / linearmath::cosh(angle);
	}

	// csch
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType csch(genType angle)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'csch' only accept floating-point values");
		return genType(1) / linearmath::sinh(angle);
	}

	// coth
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType coth(genType angle)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'coth' only accept floating-point values");
		return linearmath::cosh(angle) / linearmath::sinh(angle);
	}

	// asech
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType asech(genType x)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'asech' only accept floating-point values");
		return acosh(genType(1) / x);
	}

	// acsch
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType acsch(genType x)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'acsch' only accept floating-point values");
		return asinh(genType(1) / x);
	}

	// acoth
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType acoth(genType x)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'acoth' only accept floating-point values");
		return atanh(genType(1) / x);
	}
}//namespace linearmath
