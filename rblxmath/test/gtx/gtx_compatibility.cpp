#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/compatibility.hpp>

int main()
{
	int Error(0);

	Error += rblxmath::isfinite(1.0f) ? 0 : 1;
	Error += rblxmath::isfinite(1.0) ? 0 : 1;
	Error += rblxmath::isfinite(-1.0f) ? 0 : 1;
	Error += rblxmath::isfinite(-1.0) ? 0 : 1;

	Error += rblxmath::all(rblxmath::isfinite(rblxmath::vec4(1.0f))) ? 0 : 1;
	Error += rblxmath::all(rblxmath::isfinite(rblxmath::dvec4(1.0))) ? 0 : 1;
	Error += rblxmath::all(rblxmath::isfinite(rblxmath::vec4(-1.0f))) ? 0 : 1;
	Error += rblxmath::all(rblxmath::isfinite(rblxmath::dvec4(-1.0))) ? 0 : 1;

	return Error;
}
