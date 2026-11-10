#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/gtx/vector_angle.hpp>
#include <limits>

static int test_angle()
{
	int Error = 0;
	
	float AngleA = rblxmath::angle(rblxmath::vec2(1, 0), rblxmath::normalize(rblxmath::vec2(1, 1)));
	Error += rblxmath::epsilonEqual(AngleA, rblxmath::pi<float>() * 0.25f, 0.01f) ? 0 : 1;
	float AngleB = rblxmath::angle(rblxmath::vec3(1, 0, 0), rblxmath::normalize(rblxmath::vec3(1, 1, 0)));
	Error += rblxmath::epsilonEqual(AngleB, rblxmath::pi<float>() * 0.25f, 0.01f) ? 0 : 1;
	float AngleC = rblxmath::angle(rblxmath::vec4(1, 0, 0, 0), rblxmath::normalize(rblxmath::vec4(1, 1, 0, 0)));
	Error += rblxmath::epsilonEqual(AngleC, rblxmath::pi<float>() * 0.25f, 0.01f) ? 0 : 1;

	return Error;
}

static int test_orientedAngle_vec2()
{
	int Error = 0;
	
	float AngleA = rblxmath::orientedAngle(rblxmath::vec2(1, 0), rblxmath::normalize(rblxmath::vec2(1, 1)));
	Error += rblxmath::epsilonEqual(AngleA, rblxmath::pi<float>() * 0.25f, 0.01f) ? 0 : 1;
	float AngleB = rblxmath::orientedAngle(rblxmath::vec2(0, 1), rblxmath::normalize(rblxmath::vec2(1, 1)));
	Error += rblxmath::epsilonEqual(AngleB, -rblxmath::pi<float>() * 0.25f, 0.01f) ? 0 : 1;
	float AngleC = rblxmath::orientedAngle(rblxmath::normalize(rblxmath::vec2(1, 1)), rblxmath::vec2(0, 1));
	Error += rblxmath::epsilonEqual(AngleC, rblxmath::pi<float>() * 0.25f, 0.01f) ? 0 : 1;

	return Error;
}

static int test_orientedAngle_vec3()
{
	int Error = 0;
	
	float AngleA = rblxmath::orientedAngle(rblxmath::vec3(1, 0, 0), rblxmath::normalize(rblxmath::vec3(1, 1, 0)), rblxmath::vec3(0, 0, 1));
	Error += rblxmath::epsilonEqual(AngleA, rblxmath::pi<float>() * 0.25f, 0.01f) ? 0 : 1;
	float AngleB = rblxmath::orientedAngle(rblxmath::vec3(0, 1, 0), rblxmath::normalize(rblxmath::vec3(1, 1, 0)), rblxmath::vec3(0, 0, 1));
	Error += rblxmath::epsilonEqual(AngleB, -rblxmath::pi<float>() * 0.25f, 0.01f) ? 0 : 1;
	float AngleC = rblxmath::orientedAngle(rblxmath::normalize(rblxmath::vec3(1, 1, 0)), rblxmath::vec3(0, 1, 0), rblxmath::vec3(0, 0, 1));
	Error += rblxmath::epsilonEqual(AngleC, rblxmath::pi<float>() * 0.25f, 0.01f) ? 0 : 1;

	return Error;
}

int main()
{
	int Error(0);
	
	Error += test_angle();
	Error += test_orientedAngle_vec2();
	Error += test_orientedAngle_vec3();

	return Error;
}


