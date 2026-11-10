#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/scalar_integer.hpp>
#include <rblxmath/ext/scalar_ulp.hpp>
#include <cmath>

static int test_equal_epsilon()
{
#	if RBLXMATH_CONFIG_CONSTEXP == RBLXMATH_ENABLE
		static_assert(rblxmath::equal(1.01f, 1.02f, 0.1f), "RBLXMATH: Failed constexpr");
		static_assert(!rblxmath::equal(1.01f, 1.02f, 0.001f), "RBLXMATH: Failed constexpr");
#	endif
	
	int Error = 0;

	Error += rblxmath::equal(1.01f, 1.02f, 0.1f) ? 0 : 1;
	Error += !rblxmath::equal(1.01f, 1.02f, 0.001f) ? 0 : 1;

	return Error;
}

static int test_notEqual_epsilon()
{
#	if RBLXMATH_CONFIG_CONSTEXP == RBLXMATH_ENABLE
		static_assert(rblxmath::notEqual(1.01f, 1.02f, 0.001f), "RBLXMATH: Failed constexpr");
		static_assert(!rblxmath::notEqual(1.01f, 1.02f, 0.1f), "RBLXMATH: Failed constexpr");
#	endif
	
	int Error = 0;

	Error += rblxmath::notEqual(1.01f, 1.02f, 0.001f) ? 0 : 1;
	Error += !rblxmath::notEqual(1.01f, 1.02f, 0.1f) ? 0 : 1;

	return Error;
}

static int test_equal_ulps()
{
	int Error = 0;
	
	float const ULP1Plus = rblxmath::nextFloat(1.0f);
	Error += rblxmath::equal(1.0f, ULP1Plus, 1) ? 0 : 1;

	float const ULP2Plus = rblxmath::nextFloat(ULP1Plus);
	Error += !rblxmath::equal(1.0f, ULP2Plus, 1) ? 0 : 1;
	
	float const ULP1Minus = rblxmath::prevFloat(1.0f);
	Error += rblxmath::equal(1.0f, ULP1Minus, 1) ? 0 : 1;

	float const ULP2Minus = rblxmath::prevFloat(ULP1Minus);
	Error += !rblxmath::equal(1.0f, ULP2Minus, 1) ? 0 : 1;
	
	return Error;
}

static int test_notEqual_ulps()
{
	int Error = 0;
	
	float const ULP1Plus = rblxmath::nextFloat(1.0f);
	Error += !rblxmath::notEqual(1.0f, ULP1Plus, 1) ? 0 : 1;
	
	float const ULP2Plus = rblxmath::nextFloat(ULP1Plus);
	Error += rblxmath::notEqual(1.0f, ULP2Plus, 1) ? 0 : 1;
	
	float const ULP1Minus = rblxmath::prevFloat(1.0f);
	Error += !rblxmath::notEqual(1.0f, ULP1Minus, 1) ? 0 : 1;
	
	float const ULP2Minus = rblxmath::prevFloat(ULP1Minus);
	Error += rblxmath::notEqual(1.0f, ULP2Minus, 1) ? 0 : 1;
	
	return Error;
}

static int test_equal_sign()
{
	int Error = 0;

	Error += !rblxmath::equal(-0.0f, 0.0f, 2) ? 0 : 1;
	Error += !rblxmath::equal(-0.0, 0.0, 2) ? 0 : 1;

	Error += !rblxmath::equal(-1.0f, 2.0f, 2) ? 0 : 1;
	Error += !rblxmath::equal(-1.0, 2.0, 2) ? 0 : 1;

	Error += !rblxmath::equal(-0.00001f, 1.00000f, 2) ? 0 : 1;
	Error += !rblxmath::equal(-0.00001, 1.00000, 2) ? 0 : 1;

	Error += !rblxmath::equal(-1.0f, 1.0f, 2) ? 0 : 1;
	Error += !rblxmath::equal(-1.0, 1.0, 2) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_equal_epsilon();
	Error += test_notEqual_epsilon();

	Error += test_equal_ulps();
	Error += test_notEqual_ulps();

	Error += test_equal_sign();

	return Error;
}
