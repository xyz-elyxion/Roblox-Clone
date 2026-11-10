#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/vec2.hpp>
#include <rblxmath/vec3.hpp>
#include <rblxmath/vec4.hpp>
#include <rblxmath/gtx/vector_query.hpp>

static int test_areCollinear()
{
	int Error(0);

	{
		bool TestA = rblxmath::areCollinear(rblxmath::vec2(-1), rblxmath::vec2(1), 0.00001f);
		Error += TestA ? 0 : 1;
	}

	{
		bool TestA = rblxmath::areCollinear(rblxmath::vec3(-1), rblxmath::vec3(1), 0.00001f);
		Error += TestA ? 0 : 1;
	}

	{
		bool TestA = rblxmath::areCollinear(rblxmath::vec4(-1), rblxmath::vec4(1), 0.00001f);
		Error += TestA ? 0 : 1;
	}

	return Error;
}

static int test_areOrthogonal()
{
	int Error(0);
	
	bool TestA = rblxmath::areOrthogonal(rblxmath::vec2(1, 0), rblxmath::vec2(0, 1), 0.00001f);
	Error += TestA ? 0 : 1;

	return Error;
}

static int test_isNormalized()
{
	int Error(0);
	
	bool TestA = rblxmath::isNormalized(rblxmath::vec4(1, 0, 0, 0), 0.00001f);
	Error += TestA ? 0 : 1;

	return Error;
}

static int test_isNull()
{
	int Error(0);
	
	bool TestA = rblxmath::isNull(rblxmath::vec4(0), 0.00001f);
	Error += TestA ? 0 : 1;

	return Error;
}

static int test_areOrthonormal()
{
	int Error(0);

	bool TestA = rblxmath::areOrthonormal(rblxmath::vec2(1, 0), rblxmath::vec2(0, 1), 0.00001f);
	Error += TestA ? 0 : 1;

	return Error;
}

int main()
{
	int Error(0);

	Error += test_areCollinear();
	Error += test_areOrthogonal();
	Error += test_isNormalized();
	Error += test_isNull();
	Error += test_areOrthonormal();

	return Error;
}


