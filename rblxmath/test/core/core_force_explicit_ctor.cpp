#define RBLXMATH_FORCE_EXPLICIT_CTOR

#include <rblxmath/rblxmath.hpp>
#include <rblxmath/ext.hpp>

int main()
{
	int Error = 0;

	rblxmath::ivec4 B(1);
	Error += B == rblxmath::ivec4(1) ? 0 : 1;

	//rblxmath::vec4 A = B;

	return Error;
}

