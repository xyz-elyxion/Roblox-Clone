#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/rblxmath.hpp>

#if RBLXMATH_HAS_RANGE_FOR

#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/range.hpp>

static int test_vec()
{
	int Error = 0;

	{
		rblxmath::ivec3 const v(1, 2, 3);

		int count = 0;
		rblxmath::ivec3 Result(0);
		for(int x : v)
		{
			Result[count] = x;
			count++;
		}
		Error += count == 3 ? 0 : 1;
		Error += v == Result ? 0 : 1;
	}

	{
		rblxmath::ivec3 v(1, 2, 3);
		for(int& x : v)
			x = 0;
		Error += rblxmath::all(rblxmath::equal(v, rblxmath::ivec3(0))) ? 0 : 1;
	}

	return Error;
}

static int test_mat()
{
	int Error = 0;

	{
		rblxmath::mat4x3 m(1.0f);

		int count = 0;
		float Sum = 0.0f;
		for(float x : m)
		{
			count++;
			Sum += x;
		}
		Error += count == 12 ? 0 : 1;
		Error += rblxmath::equal(Sum, 3.0f, 0.001f) ? 0 : 1;
	}

	{
		rblxmath::mat4x3 m(1.0f);

		for (float& x : m) { x = 0; }
		rblxmath::vec4 v(1, 1, 1, 1);
		Error += rblxmath::all(rblxmath::equal(m*v, rblxmath::vec3(0, 0, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	}

	return Error;
}

int main()
{
	int Error = 0;
	Error += test_vec();
	Error += test_mat();
	return Error;
}

#else

int main()
{
	return 0;
}

#endif//RBLXMATH_HAS_RANGE_FOR
