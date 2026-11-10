#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/functions.hpp>
#include <rblxmath/ext/vector_float2.hpp>
#include <vector>

static int test_gauss_1d()
{
	int Error = 0;

	std::vector<float> Result(20);
	for(std::size_t i = 0, n = Result.size(); i < n; ++i)
		Result[i] = rblxmath::gauss(static_cast<float>(i) * 0.1f, 0.0f, 1.0f);

	return Error;
}

static int test_gauss_2d()
{
	int Error = 0;

	std::vector<float> Result(20);
	for(std::size_t i = 0, n = Result.size(); i < n; ++i)
		Result[i] = rblxmath::gauss(rblxmath::vec2(static_cast<float>(i)) * 0.1f, rblxmath::vec2(0.0f), rblxmath::vec2(1.0f));

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_gauss_1d();
	Error += test_gauss_2d();

	return Error;
}

