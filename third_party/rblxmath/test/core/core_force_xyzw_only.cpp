#define RBLXMATH_FORCE_XYZW_ONLY

#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/gtc/vec1.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/vec2.hpp>
#include <rblxmath/vec3.hpp>
#include <rblxmath/vec4.hpp>

static int test_comp()
{
	int Error = 0;

	{
		rblxmath::ivec1 A(1);
		Error += A.x == 1 ? 0 : 1;
	}

	{
		rblxmath::ivec2 A(1, 2);
		Error += A.x == 1 ? 0 : 1;
		Error += A.y == 2 ? 0 : 1;
	}

	{
		rblxmath::ivec3 A(1, 2, 3);
		Error += A.x == 1 ? 0 : 1;
		Error += A.y == 2 ? 0 : 1;
		Error += A.z == 3 ? 0 : 1;
	}

	{
		rblxmath::ivec4 A(1, 2, 3, 4);
		Error += A.x == 1 ? 0 : 1;
		Error += A.y == 2 ? 0 : 1;
		Error += A.z == 3 ? 0 : 1;
		Error += A.w == 4 ? 0 : 1;
	}

	return Error;
}

static int test_constexpr()
{
	int Error = 0;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_comp();
	Error += test_constexpr();

	return Error;
}
