#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/gradient_paint.hpp>

static int test_radialGradient()
{
	int Error = 0;
	
	float Gradient = rblxmath::radialGradient(rblxmath::vec2(0), 1.0f, rblxmath::vec2(1), rblxmath::vec2(0.5));
	Error += Gradient != 0.0f ? 0 : 1;
	
	return Error;
}

static int test_linearGradient()
{
	int Error = 0;

	float Gradient = rblxmath::linearGradient(rblxmath::vec2(0), rblxmath::vec2(1), rblxmath::vec2(0.5));
	Error += Gradient != 0.0f ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

    Error += test_radialGradient();
    Error += test_linearGradient();
    
	return Error;
}


