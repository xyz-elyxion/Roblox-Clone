#include <rblxmath/ext/vector_relational.hpp>

#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/color_space.hpp>

static int test_hsv()
{
	int Error = 0;

	rblxmath::vec3 colorHSV = rblxmath::hsvColor(rblxmath::vec3(1.0f, 0.5f, 0.0f));
	rblxmath::vec3 colorRGB = rblxmath::rgbColor(colorHSV);

	Error += rblxmath::all(rblxmath::equal(colorRGB, rblxmath::vec3(1.0f, 0.5f, 0.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_saturation()
{
	int Error = 0;
	
	rblxmath::vec4 Color = rblxmath::saturation(1.0f, rblxmath::vec4(1.0, 0.5, 0.0, 1.0));
	Error += rblxmath::all(rblxmath::equal(Color, rblxmath::vec4(1.0, 0.5, 0.0, 1.0), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

int main()
{
	int Error(0);

	Error += test_hsv();
	Error += test_saturation();

	return Error;
}
