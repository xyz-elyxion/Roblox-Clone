#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/matrix_query.hpp>

static int test_isNull()
{
	int Error(0);
	
	bool TestA = rblxmath::isNull(rblxmath::mat4(0), 0.00001f);
	Error += TestA ? 0 : 1;

	return Error;
}

static int test_isIdentity()
{
	int Error(0);
	
	{
		bool TestA = rblxmath::isIdentity(rblxmath::mat2(1), 0.00001f);
		Error += TestA ? 0 : 1;
	}
	{
		bool TestA = rblxmath::isIdentity(rblxmath::mat3(1), 0.00001f);
		Error += TestA ? 0 : 1;
	}
	{
		bool TestA = rblxmath::isIdentity(rblxmath::mat4(1), 0.00001f);
		Error += TestA ? 0 : 1;
	}

	return Error;
}

static int test_isNormalized()
{
	int Error(0);

	bool TestA = rblxmath::isNormalized(rblxmath::mat4(1), 0.00001f);
	Error += TestA ? 0 : 1;

	return Error;
}

static int test_isOrthogonal()
{
	int Error(0);

	{
		bool TestA = rblxmath::isOrthogonal(rblxmath::mat4(1), 0.00001f);
		Error += TestA ? 0 : 1;
	}
	{
		bool TestA = rblxmath::isOrthogonal(rblxmath::mat4(0), 0.00001f);
		Error += TestA ? 1 : 0;
	}

	return Error;
}

int main()
{
	int Error(0);

	Error += test_isNull();
	Error += test_isIdentity();
	Error += test_isNormalized();
	Error += test_isOrthogonal();

	return Error;
}


