#include <rblxmath/gtc/ulp.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <limits>

static int test_ulp_float_dist()
{
	int Error = 0;

	float A = 1.0f;

	float B = rblxmath::next_float(A);
	Error += rblxmath::notEqual(A, B, 0) ? 0 : 1;
	float C = rblxmath::prev_float(B);
	Error += rblxmath::equal(A, C, 0) ? 0 : 1;

	int D = rblxmath::float_distance(A, B);
	Error += D == 1 ? 0 : 1;
	int E = rblxmath::float_distance(A, C);
	Error += E == 0 ? 0 : 1;

	return Error;
}

static int test_ulp_float_step()
{
	int Error = 0;

	float A = 1.0f;

	for(int i = 10; i < 1000; i *= 10)
	{
		float B = rblxmath::next_float(A, i);
		Error += rblxmath::notEqual(A, B, 0) ? 0 : 1;
		float C = rblxmath::prev_float(B, i);
		Error += rblxmath::equal(A, C, 0) ? 0 : 1;

		int D = rblxmath::float_distance(A, B);
		Error += D == i ? 0 : 1;
		int E = rblxmath::float_distance(A, C);
		Error += E == 0 ? 0 : 1;
	}

	return Error;
}

static int test_ulp_double_dist()
{
	int Error = 0;

	double A = 1.0;

	double B = rblxmath::next_float(A);
	Error += rblxmath::notEqual(A, B, 0) ? 0 : 1;
	double C = rblxmath::prev_float(B);
	Error += rblxmath::equal(A, C, 0) ? 0 : 1;

	rblxmath::int64 const D = rblxmath::float_distance(A, B);
	Error += D == 1 ? 0 : 1;
	rblxmath::int64 const E = rblxmath::float_distance(A, C);
	Error += E == 0 ? 0 : 1;

	return Error;
}

static int test_ulp_double_step()
{
	int Error = 0;

	double A = 1.0;

	for(int i = 10; i < 1000; i *= 10)
	{
		double B = rblxmath::next_float(A, i);
		Error += rblxmath::notEqual(A, B, 0) ? 0 : 1;
		double C = rblxmath::prev_float(B, i);
		Error += rblxmath::equal(A, C, 0) ? 0 : 1;

		rblxmath::int64 const D = rblxmath::float_distance(A, B);
		Error += D == i ? 0 : 1;
		rblxmath::int64 const E = rblxmath::float_distance(A, C);
		Error += E == 0 ? 0 : 1;
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


