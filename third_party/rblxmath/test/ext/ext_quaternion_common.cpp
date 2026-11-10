#include <rblxmath/ext/vector_float3.hpp>
#include <rblxmath/ext/quaternion_common.hpp>
#include <rblxmath/ext/quaternion_float.hpp>
#include <rblxmath/ext/quaternion_relational.hpp>
#include <rblxmath/ext/quaternion_trigonometric.hpp>
#include <rblxmath/ext/scalar_constants.hpp>
#include <rblxmath/ext/scalar_relational.hpp>

static int test_conjugate()
{
	int Error = 0;

	rblxmath::quat const A(rblxmath::vec3(1, 0, 0), rblxmath::vec3(0, 1, 0));
	rblxmath::quat const C = rblxmath::conjugate(A);
	Error += rblxmath::any(rblxmath::notEqual(A, C, rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::quat const B = rblxmath::conjugate(C);
	Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mix()
{
	int Error = 0;

	rblxmath::quat const Q1(rblxmath::vec3(1, 0, 0), rblxmath::vec3(1, 0, 0));
	rblxmath::quat const Q2(rblxmath::vec3(1, 0, 0), rblxmath::vec3(0, 1, 0));

	{
		rblxmath::quat const Q3 = rblxmath::mix(Q1, Q2, 0.5f);
		float const F3 = rblxmath::degrees(rblxmath::angle(Q3));
		Error += rblxmath::equal(F3, 45.0f, 0.001f) ? 0 : 1;

		rblxmath::quat const Q4 = rblxmath::mix(Q2, Q1, 0.5f);
		float const F4 = rblxmath::degrees(rblxmath::angle(Q4));
		Error += rblxmath::equal(F4, 45.0f, 0.001f) ? 0 : 1;
	}

	{
		rblxmath::quat const Q3 = rblxmath::slerp(Q1, Q2, 0.5f);
		float const F3 = rblxmath::degrees(rblxmath::angle(Q3));
		Error += rblxmath::equal(F3, 45.0f, 0.001f) ? 0 : 1;

		rblxmath::quat const Q4 = rblxmath::slerp(Q2, Q1, 0.5f);
		float const F4 = rblxmath::degrees(rblxmath::angle(Q4));
		Error += rblxmath::equal(F4, 45.0f, 0.001f) ? 0 : 1;
	}

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_conjugate();
	Error += test_mix();

	return Error;
}
