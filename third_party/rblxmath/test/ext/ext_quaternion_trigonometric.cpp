#include <rblxmath/ext/quaternion_trigonometric.hpp>
#include <rblxmath/ext/quaternion_float.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/scalar_relational.hpp>

float const Epsilon = 0.001f;

static int test_angle()
{
	int Error = 0;

	{
		rblxmath::quat const Q = rblxmath::quat(rblxmath::vec3(1, 0, 0), rblxmath::vec3(0, 1, 0));
		float const A = rblxmath::degrees(rblxmath::angle(Q));
		Error += rblxmath::equal(A, 90.0f, Epsilon) ? 0 : 1;
	}

	{
		rblxmath::quat const Q = rblxmath::quat(rblxmath::vec3(0, 1, 0), rblxmath::vec3(1, 0, 0));
		float const A = rblxmath::degrees(rblxmath::angle(Q));
		Error += rblxmath::equal(A, 90.0f, Epsilon) ? 0 : 1;
	}

	{
		rblxmath::quat const Q = rblxmath::angleAxis(rblxmath::two_pi<float>() - 1.0f, rblxmath::vec3(1, 0, 0));
		float const A = rblxmath::angle(Q);
		Error += rblxmath::equal(A, 1.0f, Epsilon) ? 1 : 0;
	}

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_angle();

	return Error;
}
