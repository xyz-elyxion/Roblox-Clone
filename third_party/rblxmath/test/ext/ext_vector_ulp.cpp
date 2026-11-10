#include <rblxmath/ext/vector_ulp.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/vector_float4.hpp>
#include <rblxmath/ext/vector_double4.hpp>
#include <rblxmath/ext/vector_int4.hpp>

static int test_ulp_float_dist()
{
	int Error = 0;

	rblxmath::vec4 const A(1.0f);

	rblxmath::vec4 const B = rblxmath::nextFloat(A);
	Error += rblxmath::any(rblxmath::notEqual(A, B, 0)) ? 0 : 1;
	rblxmath::vec4 const C = rblxmath::prevFloat(B);
	Error += rblxmath::all(rblxmath::equal(A, C, 0)) ? 0 : 1;

	rblxmath::ivec4 const D = rblxmath::floatDistance(A, B);
	Error += D == rblxmath::ivec4(1) ? 0 : 1;
	rblxmath::ivec4 const E = rblxmath::floatDistance(A, C);
	Error += E == rblxmath::ivec4(0) ? 0 : 1;

	return Error;
}

static int test_ulp_float_step()
{
	int Error = 0;

	rblxmath::vec4 const A(1.0f);

	for(int i = 10; i < 1000; i *= 10)
	{
		rblxmath::vec4 const B = rblxmath::nextFloat(A, i);
		Error += rblxmath::any(rblxmath::notEqual(A, B, 0)) ? 0 : 1;
		rblxmath::vec4 const C = rblxmath::prevFloat(B, i);
		Error += rblxmath::all(rblxmath::equal(A, C, 0)) ? 0 : 1;

		rblxmath::ivec4 const D = rblxmath::floatDistance(A, B);
		Error += D == rblxmath::ivec4(i) ? 0 : 1;
		rblxmath::ivec4 const E = rblxmath::floatDistance(A, C);
		Error += E == rblxmath::ivec4(0) ? 0 : 1;
	}

	return Error;
}

static int test_ulp_double_dist()
{
	int Error = 0;

	rblxmath::dvec4 const A(1.0);

	rblxmath::dvec4 const B = rblxmath::nextFloat(A);
	Error += rblxmath::any(rblxmath::notEqual(A, B, 0)) ? 0 : 1;
	rblxmath::dvec4 const C = rblxmath::prevFloat(B);
	Error += rblxmath::all(rblxmath::equal(A, C, 0)) ? 0 : 1;

	rblxmath::ivec4 const D(rblxmath::floatDistance(A, B));
	Error += D == rblxmath::ivec4(1) ? 0 : 1;
	rblxmath::ivec4 const E = rblxmath::floatDistance(A, C);
	Error += E == rblxmath::ivec4(0) ? 0 : 1;

	return Error;
}

static int test_ulp_double_step()
{
	int Error = 0;

	rblxmath::dvec4 const A(1.0);

	for(int i = 10; i < 1000; i *= 10)
	{
		rblxmath::dvec4 const B = rblxmath::nextFloat(A, i);
		Error += rblxmath::any(rblxmath::notEqual(A, B, 0)) ? 0 : 1;
		rblxmath::dvec4 const C = rblxmath::prevFloat(B, i);
		Error += rblxmath::all(rblxmath::equal(A, C, 0)) ? 0 : 1;

		rblxmath::ivec4 const D(rblxmath::floatDistance(A, B));
		Error += D == rblxmath::ivec4(i) ? 0 : 1;
		rblxmath::ivec4 const E(rblxmath::floatDistance(A, C));
		Error += E == rblxmath::ivec4(0) ? 0 : 1;
	}

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_ulp_float_dist();
	Error += test_ulp_float_step();
	Error += test_ulp_double_dist();
	Error += test_ulp_double_step();

	return Error;
}
