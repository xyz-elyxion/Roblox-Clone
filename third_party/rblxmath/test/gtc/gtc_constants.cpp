#include <rblxmath/gtc/constants.hpp>

static int test_epsilon()
{
	int Error = 0;

	{
		float Test = rblxmath::epsilon<float>();
		Error += Test > 0.0f ? 0 : 1;
	}

	{
		double Test = rblxmath::epsilon<double>();
		Error += Test > 0.0 ? 0 : 1;
	}

	return Error;
}

int main()
{
	int Error(0);

	//float MinHalf = 0.0f;
	//while (rblxmath::half(MinHalf) == rblxmath::half(0.0f))
	//	MinHalf += std::numeric_limits<float>::epsilon();
	Error += test_epsilon();
	
	return Error;
}
