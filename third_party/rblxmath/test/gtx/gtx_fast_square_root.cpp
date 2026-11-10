#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/fast_square_root.hpp>
#include <rblxmath/gtc/type_precision.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/vector_relational.hpp>

static int test_fastInverseSqrt()
{
	int Error = 0;

	Error += rblxmath::epsilonEqual(rblxmath::fastInverseSqrt(1.0f), 1.0f, 0.01f) ? 0 : 1;
	Error += rblxmath::epsilonEqual(rblxmath::fastInverseSqrt(1.0), 1.0, 0.01) ? 0 : 1;
	Error += rblxmath::all(rblxmath::epsilonEqual(rblxmath::fastInverseSqrt(rblxmath::vec2(1.0f)), rblxmath::vec2(1.0f), 0.01f)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::epsilonEqual(rblxmath::fastInverseSqrt(rblxmath::dvec3(1.0)), rblxmath::dvec3(1.0), 0.01)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::epsilonEqual(rblxmath::fastInverseSqrt(rblxmath::dvec4(1.0)), rblxmath::dvec4(1.0), 0.01)) ? 0 : 1;

	return Error;
}

static int test_fastDistance()
{
	int Error = 0;

	float const A = rblxmath::fastDistance(0.0f, 1.0f);
	float const B = rblxmath::fastDistance(rblxmath::vec2(0.0f), rblxmath::vec2(1.0f, 0.0f));
	float const C = rblxmath::fastDistance(rblxmath::vec3(0.0f), rblxmath::vec3(1.0f, 0.0f, 0.0f));
	float const D = rblxmath::fastDistance(rblxmath::vec4(0.0f), rblxmath::vec4(1.0f, 0.0f, 0.0f, 0.0f));

	Error += rblxmath::epsilonEqual(A, 1.0f, 0.01f) ? 0 : 1;
	Error += rblxmath::epsilonEqual(B, 1.0f, 0.01f) ? 0 : 1;
	Error += rblxmath::epsilonEqual(C, 1.0f, 0.01f) ? 0 : 1;
	Error += rblxmath::epsilonEqual(D, 1.0f, 0.01f) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_fastInverseSqrt();
	Error += test_fastDistance();

	return Error;
}
