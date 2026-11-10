#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/scalar_constants.hpp>

#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/color_space_YCoCg.hpp>

int main()
{
	int Error = 0;

	rblxmath::vec3 colorYCoCg = rblxmath::rgb2YCoCg(rblxmath::vec3(1.0f, 0.5f, 0.0f));
	rblxmath::vec3 colorRGB1 = rblxmath::YCoCg2rgb(colorYCoCg);

	Error += rblxmath::all(rblxmath::equal(colorRGB1, rblxmath::vec3(1.0f, 0.5f, 0.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}
