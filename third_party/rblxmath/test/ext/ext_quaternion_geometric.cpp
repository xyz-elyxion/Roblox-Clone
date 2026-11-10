#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/quaternion_geometric.hpp>
#include <rblxmath/ext/quaternion_float.hpp>
#include <rblxmath/ext/quaternion_trigonometric.hpp>
#include <rblxmath/ext/quaternion_float_precision.hpp>
#include <rblxmath/ext/quaternion_double.hpp>
#include <rblxmath/ext/quaternion_double_precision.hpp>
#include <rblxmath/ext/vector_float3.hpp>
#include <rblxmath/ext/vector_float3_precision.hpp>
#include <rblxmath/ext/vector_double3.hpp>
#include <rblxmath/ext/vector_double3_precision.hpp>
#include <rblxmath/ext/scalar_relational.hpp>

float const Epsilon = 0.001f;

static int test_length()
{
	int Error = 0;

	{
		float const A = rblxmath::length(rblxmath::quat(1, 0, 0, 0));
		Error += rblxmath::equal(A, 1.0f, Epsilon) ? 0 : 1;
	}

	{
		float const A = rblxmath::length(rblxmath::quat(1, rblxmath::vec3(0)));
		Error += rblxmath::equal(A, 1.0f, Epsilon) ? 0 : 1;
	}

	{
		float const A = rblxmath::length(rblxmath::quat(rblxmath::vec3(1, 0, 0), rblxmath::vec3(0, 1, 0)));
		Error += rblxmath::equal(A, 1.0f, Epsilon) ? 0 : 1;
	}

	return Error;
}

static int test_normalize()
{
	int Error = 0;

	{
		rblxmath::quat const A = rblxmath::quat(1, 0, 0, 0);
		rblxmath::quat const N = rblxmath::normalize(A);
		Error += rblxmath::all(rblxmath::equal(A, N, Epsilon)) ? 0 : 1;
	}

	{
		rblxmath::quat const A = rblxmath::quat(1, rblxmath::vec3(0));
		rblxmath::quat const N = rblxmath::normalize(A);
		Error += rblxmath::all(rblxmath::equal(A, N, Epsilon)) ? 0 : 1;
	}

	return Error;
}

static int test_dot()
{
	int Error = 0;

	{
		rblxmath::quat const A = rblxmath::quat(1, 0, 0, 0);
		rblxmath::quat const B = rblxmath::quat(1, 0, 0, 0);
		float const C = rblxmath::dot(A, B);
		Error += rblxmath::equal(C, 1.0f, Epsilon) ? 0 : 1;
	}

	return Error;
}

static int test_cross()
{
	int Error = 0;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_length();
	Error += test_normalize();
	Error += test_dot();
	Error += test_cross();

	return Error;
}
