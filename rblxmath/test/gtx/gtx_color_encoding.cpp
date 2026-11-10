#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/color_encoding.hpp>
#include <rblxmath/gtc/color_space.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/gtc/constants.hpp>

namespace srgb
{
	static int test()
	{
		int Error(0);

		rblxmath::vec3 const ColorSourceRGB(1.0, 0.5, 0.0);
/*
		{
			rblxmath::vec3 const ColorSRGB = rblxmath::convertLinearSRGBToD65XYZ(ColorSourceRGB);
			rblxmath::vec3 const ColorRGB = rblxmath::convertD65XYZToLinearSRGB(ColorSRGB);
			Error += rblxmath::all(rblxmath::epsilonEqual(ColorSourceRGB, ColorRGB, 0.00001f)) ? 0 : 1;
		}
*/
		{
			rblxmath::vec3 const ColorSRGB = rblxmath::convertLinearToSRGB(ColorSourceRGB, 2.8f);
			rblxmath::vec3 const ColorRGB = rblxmath::convertSRGBToLinear(ColorSRGB, 2.8f);
			Error += rblxmath::all(rblxmath::epsilonEqual(ColorSourceRGB, ColorRGB, 0.00001f)) ? 0 : 1;
		}

		rblxmath::vec4 const ColorSourceRGBA(1.0, 0.5, 0.0, 1.0);

		{
			rblxmath::vec4 const ColorSRGB = rblxmath::convertLinearToSRGB(ColorSourceRGBA);
			rblxmath::vec4 const ColorRGB = rblxmath::convertSRGBToLinear(ColorSRGB);
			Error += rblxmath::all(rblxmath::epsilonEqual(ColorSourceRGBA, ColorRGB, 0.00001f)) ? 0 : 1;
		}

		{
			rblxmath::vec4 const ColorSRGB = rblxmath::convertLinearToSRGB(ColorSourceRGBA, 2.8f);
			rblxmath::vec4 const ColorRGB = rblxmath::convertSRGBToLinear(ColorSRGB, 2.8f);
			Error += rblxmath::all(rblxmath::epsilonEqual(ColorSourceRGBA, ColorRGB, 0.00001f)) ? 0 : 1;
		}

		return Error;
	}
}//namespace srgb

int main()
{
	int Error(0);

	Error += srgb::test();

	return Error;
}
