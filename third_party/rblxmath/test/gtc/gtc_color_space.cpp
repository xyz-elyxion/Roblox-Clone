#include <rblxmath/gtc/color_space.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/gtc/constants.hpp>

namespace srgb
{
	static int test()
	{
		int Error(0);

		rblxmath::vec3 const ColorSourceRGB(1.0, 0.5, 0.0);

		{
			rblxmath::vec3 const ColorSRGB = rblxmath::convertLinearToSRGB(ColorSourceRGB);
			rblxmath::vec3 const ColorRGB = rblxmath::convertSRGBToLinear(ColorSRGB);
			Error += rblxmath::all(rblxmath::epsilonEqual(ColorSourceRGB, ColorRGB, 0.00001f)) ? 0 : 1;
		}

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

		rblxmath::vec4 const ColorSourceGNI = rblxmath::vec4(107, 107, 104, 131) / rblxmath::vec4(255);

		{
			rblxmath::vec4 const ColorSRGB = rblxmath::convertLinearToSRGB(ColorSourceGNI);
			rblxmath::vec4 const ColorRGB = rblxmath::convertSRGBToLinear(ColorSRGB);
			Error += rblxmath::all(rblxmath::epsilonEqual(ColorSourceGNI, ColorRGB, 0.00001f)) ? 0 : 1;
		}

		return Error;
	}
}//namespace srgb

namespace srgb_lowp
{
	static int test()
	{
		int Error(0);

		for(float Color = 0.0f; Color < 1.0f; Color += 0.01f)
		{
			rblxmath::highp_vec3 const HighpSRGB = rblxmath::convertLinearToSRGB(rblxmath::highp_vec3(Color));
			rblxmath::lowp_vec3 const LowpSRGB = rblxmath::convertLinearToSRGB(rblxmath::lowp_vec3(Color));
			Error += rblxmath::all(rblxmath::epsilonEqual(rblxmath::abs(HighpSRGB - rblxmath::highp_vec3(LowpSRGB)), rblxmath::highp_vec3(0), 0.1f)) ? 0 : 1;
		}

		return Error;
	}
}//namespace srgb_lowp

int main()
{
	int Error(0);

	Error += srgb::test();
	Error += srgb_lowp::test();

	return Error;
}
