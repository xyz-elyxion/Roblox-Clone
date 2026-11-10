#include <rblxmath/vec2.hpp>

#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/texture.hpp>

static int test_levels()
{
	int Error = 0;

	int const Levels = rblxmath::levels(rblxmath::ivec2(3, 2));
	Error += Levels == 2 ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_levels();

	return Error;
}
