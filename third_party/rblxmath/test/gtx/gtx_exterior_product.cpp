#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/exterior_product.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/vec2.hpp>

int main()
{
	int Error = 0;

	float const f = rblxmath::cross(rblxmath::vec2(1.0f, 1.0f), rblxmath::vec2(1.0f, 1.0f));
	Error += rblxmath::epsilonEqual(f, 0.0f, 0.001f) ? 0 : 1;

	return Error;
}

