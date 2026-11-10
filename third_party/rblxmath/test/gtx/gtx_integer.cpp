#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/exponential.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/gtx/integer.hpp>
#include <cstdio>
/*
static int test_floor_log2()
{
	int Error = 0;

	for(std::size_t i = 1; i < 1000000; ++i)
	{
		rblxmath::uint A = rblxmath::floor_log2(rblxmath::uint(i));
		rblxmath::uint B = rblxmath::uint(rblxmath::floor(rblxmath::log2(double(i)))); // Will fail with float, lack of accuracy

		Error += A == B ? 0 : 1;
		assert(!Error);
	}

	return Error;
}
*/
static int test_log2()
{
	int Error = 0;

	for(std::size_t i = 1; i < 24; ++i)
	{
		rblxmath::uint A = rblxmath::log2(rblxmath::uint(1 << i));
		rblxmath::uint B = rblxmath::uint(rblxmath::log2(double(1 << i)));

		//Error += rblxmath::equalEpsilon(double(A), B, 1.0) ? 0 : 1;
		Error += rblxmath::abs(double(A) - B) <= 24 ? 0 : 1;
		assert(!Error);

		std::printf("Log2(%d) error A=%d, B=%d\n", 1 << i, A, B);
	}

	std::printf("log2 error=%d\n", Error);

	return Error;
}

static int test_nlz()
{
	int Error = 0;

	for(rblxmath::uint i = 1; i < rblxmath::uint(33); ++i)
		Error += rblxmath::nlz(i) == 31u - static_cast<rblxmath::uint>(rblxmath::findMSB(i)) ? 0 : 1;
		//printf("%d, %d\n", rblxmath::nlz(i), 31u - rblxmath::findMSB(i));

	return Error;
}

static int test_pow_uint()
{
	int Error = 0;

	rblxmath::uint const p0 = rblxmath::pow(2u, 0u);
	Error += p0 == 1u ? 0 : 1;

	rblxmath::uint const p1 = rblxmath::pow(2u, 1u);
	Error += p1 == 2u ? 0 : 1;

	rblxmath::uint const p2 = rblxmath::pow(2u, 2u);
	Error += p2 == 4u ? 0 : 1;

	return Error;
}

static int test_pow_int()
{
	int Error = 0;

	int const p0 = rblxmath::pow(2, 0u);
	Error += p0 == 1 ? 0 : 1;

	int const p1 = rblxmath::pow(2, 1u);
	Error += p1 == 2 ? 0 : 1;

	int const p2 = rblxmath::pow(2, 2u);
	Error += p2 == 4 ? 0 : 1;

	int const p0n = rblxmath::pow(-2, 0u);
	Error += p0n == -1 ? 0 : 1;

	int const p1n = rblxmath::pow(-2, 1u);
	Error += p1n == -2 ? 0 : 1;

	int const p2n = rblxmath::pow(-2, 2u);
	Error += p2n == 4 ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_nlz();
//	Error += test_floor_log2();
	Error += test_log2();
	Error += test_pow_uint();
	Error += test_pow_int();

	return Error;
}

