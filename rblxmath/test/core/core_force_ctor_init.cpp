#define RBLXMATH_FORCE_CTOR_INIT

#include <rblxmath/rblxmath.hpp>
#include <rblxmath/ext.hpp>

static int test_vec()
{
	int Error = 0;

	rblxmath::vec1 V1;
	Error += rblxmath::all(rblxmath::equal(V1, rblxmath::vec1(0), rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::dvec1 U1;
	Error += rblxmath::all(rblxmath::equal(U1, rblxmath::dvec1(0), rblxmath::epsilon<double>())) ? 0 : 1;

	rblxmath::vec2 V2;
	Error += rblxmath::all(rblxmath::equal(V2, rblxmath::vec2(0, 0), rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::dvec2 U2;
	Error += rblxmath::all(rblxmath::equal(U2, rblxmath::dvec2(0, 0), rblxmath::epsilon<double>())) ? 0 : 1;

	rblxmath::vec3 V3;
	Error += rblxmath::all(rblxmath::equal(V3, rblxmath::vec3(0, 0, 0), rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::dvec3 U3;
	Error += rblxmath::all(rblxmath::equal(U3, rblxmath::dvec3(0, 0, 0), rblxmath::epsilon<double>())) ? 0 : 1;

	rblxmath::vec4 V4;
	Error += rblxmath::all(rblxmath::equal(V4, rblxmath::vec4(0, 0, 0, 0), rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::dvec4 U4;
	Error += rblxmath::all(rblxmath::equal(U4, rblxmath::dvec4(0, 0, 0, 0), rblxmath::epsilon<double>())) ? 0 : 1;

	return Error;
}

static int test_mat()
{
	int Error = 0;

	{
		rblxmath::mat2x2 F;
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::mat2x2(1), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::dmat2x2 D;
		Error += rblxmath::all(rblxmath::equal(D, rblxmath::dmat2x2(1), rblxmath::epsilon<double>())) ? 0 : 1;
	}

	{
		rblxmath::mat2x3 F;
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::mat2x3(1), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::dmat2x3 D;
		Error += rblxmath::all(rblxmath::equal(D, rblxmath::dmat2x3(1), rblxmath::epsilon<double>())) ? 0 : 1;
	}

	{
		rblxmath::mat2x4 F;
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::mat2x4(1), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::dmat2x4 D;
		Error += rblxmath::all(rblxmath::equal(D, rblxmath::dmat2x4(1), rblxmath::epsilon<double>())) ? 0 : 1;
	}

	{
		rblxmath::mat3x2 F;
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::mat3x2(1), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::dmat3x2 D;
		Error += rblxmath::all(rblxmath::equal(D, rblxmath::dmat3x2(1), rblxmath::epsilon<double>())) ? 0 : 1;
	}

	{
		rblxmath::mat3x3 F;
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::mat3x3(1), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::dmat3x3 D;
		Error += rblxmath::all(rblxmath::equal(D, rblxmath::dmat3x3(1), rblxmath::epsilon<double>())) ? 0 : 1;
	}

	{
		rblxmath::mat3x4 F;
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::mat3x4(1), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::dmat3x4 D;
		Error += rblxmath::all(rblxmath::equal(D, rblxmath::dmat3x4(1), rblxmath::epsilon<double>())) ? 0 : 1;
	}

	{
		rblxmath::mat4x2 F;
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::mat4x2(1), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::dmat4x2 D;
		Error += rblxmath::all(rblxmath::equal(D, rblxmath::dmat4x2(1), rblxmath::epsilon<double>())) ? 0 : 1;
	}

	{
		rblxmath::mat4x3 F;
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::mat4x3(1), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::dmat4x3 D;
		Error += rblxmath::all(rblxmath::equal(D, rblxmath::dmat4x3(1), rblxmath::epsilon<double>())) ? 0 : 1;
	}

	{
		rblxmath::mat4x4 F;
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::mat4x4(1), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::dmat4x4 D;
		Error += rblxmath::all(rblxmath::equal(D, rblxmath::dmat4x4(1), rblxmath::epsilon<double>())) ? 0 : 1;
	}

	return Error;
}

static int test_qua()
{
	int Error = 0;

	rblxmath::quat F;
	Error += rblxmath::all(rblxmath::equal(F, rblxmath::quat(1, 0, 0, 0), rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::dquat D;
	Error += rblxmath::all(rblxmath::equal(D, rblxmath::dquat(1, 0, 0, 0), rblxmath::epsilon<double>())) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_vec();
	Error += test_mat();
	Error += test_qua();

	return Error;
}

