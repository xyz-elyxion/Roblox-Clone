#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/rotate_vector.hpp>
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/vector_relational.hpp>

static int test_rotate()
{
	int Error = 0;

	rblxmath::vec2 A = rblxmath::rotate(rblxmath::vec2(1, 0), rblxmath::pi<float>() * 0.5f);
	Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec2(0.0f, 1.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::vec3 B = rblxmath::rotate(rblxmath::vec3(1, 0, 0), rblxmath::pi<float>() * 0.5f, rblxmath::vec3(0, 0, 1));
	Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec3(0.0f, 1.0f, 0.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::vec4 C = rblxmath::rotate(rblxmath::vec4(1, 0, 0, 1), rblxmath::pi<float>() * 0.5f, rblxmath::vec3(0, 0, 1));
	Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec4(0.0f, 1.0f, 0.0f, 1.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_rotateX()
{
	int Error = 0;

	rblxmath::vec3 D = rblxmath::rotateX(rblxmath::vec3(1, 0, 0), rblxmath::pi<float>() * 0.5f);
	Error += rblxmath::all(rblxmath::equal(D, rblxmath::vec3(1.0f, 0.0f, 0.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::vec4 E = rblxmath::rotateX(rblxmath::vec4(1, 0, 0, 1), rblxmath::pi<float>() * 0.5f);
	Error += rblxmath::all(rblxmath::equal(E, rblxmath::vec4(1.0f, 0.0f, 0.0f, 1.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_rotateY()
{
	int Error = 0;

	rblxmath::vec3 F = rblxmath::rotateY(rblxmath::vec3(1, 0, 0), rblxmath::pi<float>() * 0.5f);
	Error += rblxmath::all(rblxmath::equal(F, rblxmath::vec3(0.0f, 0.0f, -1.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::vec4 G = rblxmath::rotateY(rblxmath::vec4(1, 0, 0, 1), rblxmath::pi<float>() * 0.5f);
	Error += rblxmath::all(rblxmath::equal(G, rblxmath::vec4(0.0f, 0.0f, -1.0f, 1.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}


static int test_rotateZ()
{
	int Error = 0;

	rblxmath::vec3 H = rblxmath::rotateZ(rblxmath::vec3(1, 0, 0), rblxmath::pi<float>() * 0.5f);
	Error += rblxmath::all(rblxmath::equal(H, rblxmath::vec3(0.0f, 1.0f, 0.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::vec4 I = rblxmath::rotateZ(rblxmath::vec4(1, 0, 0, 1), rblxmath::pi<float>() * 0.5f);
	Error += rblxmath::all(rblxmath::equal(I, rblxmath::vec4(0.0f, 1.0f, 0.0f, 1.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_orientation()
{
	int Error = 0;

	rblxmath::mat4 O = rblxmath::orientation(rblxmath::normalize(rblxmath::vec3(1)), rblxmath::vec3(0, 0, 1));
	Error += rblxmath::all(rblxmath::equal(O[0], rblxmath::vec4(0.79f, -0.21f,-0.58f, 0.0f), 0.1f)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(O[1], rblxmath::vec4(-0.21f, 0.79f,-0.58f, 0.0f), 0.1f)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(O[2], rblxmath::vec4(0.58f, 0.58f, 0.58f, 0.0f), 0.1f)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(O[3], rblxmath::vec4(0.0f, 0.0f, 0.0f, 1.0f), 0.1f)) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_rotate();
	Error += test_rotateX();
	Error += test_rotateY();
	Error += test_rotateZ();
	Error += test_orientation();

	return Error;
}


