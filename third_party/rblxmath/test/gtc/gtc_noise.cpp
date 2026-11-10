#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtc/noise.hpp>
#include <rblxmath/gtc/type_precision.hpp>
#include <rblxmath/gtx/raw_data.hpp>

static int test_simplex_float()
{
	int Error = 0;

	rblxmath::u8vec4 const PixelSimplex2D(rblxmath::byte(rblxmath::abs(rblxmath::simplex(rblxmath::vec2(0.f, 0.f))) * 255.f));
	rblxmath::u8vec4 const PixelSimplex3D(rblxmath::byte(rblxmath::abs(rblxmath::simplex(rblxmath::vec3(0.f, 0.f, 0.f))) * 255.f));
	rblxmath::u8vec4 const PixelSimplex4D(rblxmath::byte(rblxmath::abs(rblxmath::simplex(rblxmath::vec4(0.f, 0.f, 0.f, 0.f))) * 255.f));

	return Error;
}

static int test_simplex_double()
{
	int Error = 0;

	rblxmath::u8vec4 const PixelSimplex2D(rblxmath::byte(rblxmath::abs(rblxmath::simplex(rblxmath::dvec2(0.f, 0.f))) * 255.));
	rblxmath::u8vec4 const PixelSimplex3D(rblxmath::byte(rblxmath::abs(rblxmath::simplex(rblxmath::dvec3(0.f, 0.f, 0.f))) * 255.));
	rblxmath::u8vec4 const PixelSimplex4D(rblxmath::byte(rblxmath::abs(rblxmath::simplex(rblxmath::dvec4(0.f, 0.f, 0.f, 0.f))) * 255.));

	return Error;
}

static int test_perlin_float()
{
	int Error = 0;

	rblxmath::u8vec4 const PixelPerlin2D(rblxmath::byte(rblxmath::abs(rblxmath::perlin(rblxmath::vec2(0.f, 0.f))) * 255.f));
	rblxmath::u8vec4 const PixelPerlin3D(rblxmath::byte(rblxmath::abs(rblxmath::perlin(rblxmath::vec3(0.f, 0.f, 0.f))) * 255.f));
	rblxmath::u8vec4 const PixelPerlin4D(rblxmath::byte(rblxmath::abs(rblxmath::perlin(rblxmath::vec4(0.f, 0.f, 0.f, 0.f))) * 255.f));

	return Error;
}

static int test_perlin_double()
{
	int Error = 0;

	rblxmath::u8vec4 const PixelPerlin2D(rblxmath::byte(rblxmath::abs(rblxmath::perlin(rblxmath::dvec2(0.f, 0.f))) * 255.));
	rblxmath::u8vec4 const PixelPerlin3D(rblxmath::byte(rblxmath::abs(rblxmath::perlin(rblxmath::dvec3(0.f, 0.f, 0.f))) * 255.));
	rblxmath::u8vec4 const PixelPerlin4D(rblxmath::byte(rblxmath::abs(rblxmath::perlin(rblxmath::dvec4(0.f, 0.f, 0.f, 0.f))) * 255.));

	return Error;
}

static int test_perlin_pedioric_float()
{
	int Error = 0;

	rblxmath::u8vec4 const PixelPeriodic2D(rblxmath::byte(rblxmath::abs(rblxmath::perlin(rblxmath::vec2(0.f, 0.f), rblxmath::vec2(2.0f))) * 255.f));
	rblxmath::u8vec4 const PixelPeriodic3D(rblxmath::byte(rblxmath::abs(rblxmath::perlin(rblxmath::vec3(0.f, 0.f, 0.f), rblxmath::vec3(2.0f))) * 255.f));
	rblxmath::u8vec4 const PixelPeriodic4D(rblxmath::byte(rblxmath::abs(rblxmath::perlin(rblxmath::vec4(0.f, 0.f, 0.f, 0.f), rblxmath::vec4(2.0f))) * 255.f));

	return Error;
}

static int test_perlin_pedioric_double()
{
	int Error = 0;

	rblxmath::u8vec4 const PixelPeriodic2D(rblxmath::byte(rblxmath::abs(rblxmath::perlin(rblxmath::dvec2(0.f, 0.f), rblxmath::dvec2(2.0))) * 255.));
	rblxmath::u8vec4 const PixelPeriodic3D(rblxmath::byte(rblxmath::abs(rblxmath::perlin(rblxmath::dvec3(0.f, 0.f, 0.f), rblxmath::dvec3(2.0))) * 255.));
	rblxmath::u8vec4 const PixelPeriodic4D(rblxmath::byte(rblxmath::abs(rblxmath::perlin(rblxmath::dvec4(0.f, 0.f, 0.f, 0.f), rblxmath::dvec4(2.0))) * 255.));

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_simplex_float();
	Error += test_simplex_double();

	Error += test_perlin_float();
	Error += test_perlin_double();

	Error += test_perlin_pedioric_float();
	Error += test_perlin_pedioric_double();

	return Error;
}
