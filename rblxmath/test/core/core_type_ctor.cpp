#include <rblxmath/gtc/vec1.hpp>
#include <rblxmath/gtc/quaternion.hpp>
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/rblxmath.hpp>

static int test_vec1_ctor()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_DEFAULTED_FUNCTIONS == RBLXMATH_ENABLE
	{
		union pack
		{
			rblxmath::vec1 f;
			rblxmath::ivec1 i;
		} A, B;

		A.f = rblxmath::vec1(0);
		Error += rblxmath::all(rblxmath::equal(A.i, rblxmath::ivec1(0))) ? 0 : 1;

		B.f = rblxmath::vec1(1);
		Error += rblxmath::all(rblxmath::equal(B.i, rblxmath::ivec1(1065353216))) ? 0 : 1;
	}
#	endif//RBLXMATH_CONFIG_DEFAULTED_FUNCTIONS == RBLXMATH_ENABLE

	return Error;
}

static int test_vec2_ctor()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_DEFAULTED_FUNCTIONS == RBLXMATH_ENABLE
	{
		union pack
		{
			rblxmath::vec2 f;
			rblxmath::ivec2 i;
		} A, B;

		A.f = rblxmath::vec2(0);
		Error += rblxmath::all(rblxmath::equal(A.i, rblxmath::ivec2(0))) ? 0 : 1;

		B.f = rblxmath::vec2(1);
		Error += rblxmath::all(rblxmath::equal(B.i, rblxmath::ivec2(1065353216))) ? 0 : 1;
	}
#	endif

	return Error;
}

static int test_vec3_ctor()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_DEFAULTED_FUNCTIONS == RBLXMATH_ENABLE
	{
		union pack
		{
			rblxmath::vec3 f;
			rblxmath::ivec3 i;
		} A, B;

		A.f = rblxmath::vec3(0);
		Error += rblxmath::all(rblxmath::equal(A.i, rblxmath::ivec3(0))) ? 0 : 1;

		B.f = rblxmath::vec3(1);
		Error += rblxmath::all(rblxmath::equal(B.i, rblxmath::ivec3(1065353216))) ? 0 : 1;
	}
#	endif

	return Error;
}

static int test_vec4_ctor()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_DEFAULTED_FUNCTIONS == RBLXMATH_ENABLE
	{
		union pack
		{
			rblxmath::vec4 f;
			rblxmath::ivec4 i;
		} A, B;

		A.f = rblxmath::vec4(0);
		Error += rblxmath::all(rblxmath::equal(A.i, rblxmath::ivec4(0))) ? 0 : 1;

		B.f = rblxmath::vec4(1);
		Error += rblxmath::all(rblxmath::equal(B.i, rblxmath::ivec4(1065353216))) ? 0 : 1;
	}
#	endif

	return Error;
}

static int test_mat2x2_ctor()
{
	int Error = 0;

#	if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
	{
		union pack
		{
			rblxmath::mat2x2 f;
			rblxmath::mat2x2 i;
		} A, B;

		A.f = rblxmath::mat2x2(0);
		Error += rblxmath::all(rblxmath::equal(A.i[0], rblxmath::vec2(0), rblxmath::epsilon<float>())) ? 0 : 1;

		B.f = rblxmath::mat2x2(1);
		Error += rblxmath::all(rblxmath::equal(B.i[0], rblxmath::vec2(1, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG

	return Error;
}

static int test_mat2x3_ctor()
{
	int Error = 0;

#	if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
	{
		union pack
		{
			rblxmath::mat2x3 f;
			rblxmath::mat2x3 i;
		} A, B;

		A.f = rblxmath::mat2x3(0);
		Error += rblxmath::all(rblxmath::equal(A.i[0], rblxmath::vec3(0), rblxmath::epsilon<float>())) ? 0 : 1;

		B.f = rblxmath::mat2x3(1);
		Error += rblxmath::all(rblxmath::equal(B.i[0], rblxmath::vec3(1, 0, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG

	return Error;
}

static int test_mat2x4_ctor()
{
	int Error = 0;

#	if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
	{
		union pack
		{
			rblxmath::mat2x4 f;
			rblxmath::mat2x4 i;
		} A, B;

		A.f = rblxmath::mat2x4(0);
		rblxmath::vec4 const C(0, 0, 0, 0);
		Error += rblxmath::all(rblxmath::equal(A.i[0], C, rblxmath::epsilon<float>())) ? 0 : 1;

		B.f = rblxmath::mat2x4(1);
		rblxmath::vec4 const D(1, 0, 0, 0);
		Error += rblxmath::all(rblxmath::equal(B.i[0], D, rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG

	return Error;
}

static int test_mat3x2_ctor()
{
	int Error = 0;

#	if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
	{
		union pack
		{
			rblxmath::mat3x2 f;
			rblxmath::mat3x2 i;
		} A, B;

		A.f = rblxmath::mat3x2(0);
		Error += rblxmath::all(rblxmath::equal(A.i[0], rblxmath::vec2(0), rblxmath::epsilon<float>())) ? 0 : 1;

		B.f = rblxmath::mat3x2(1);
		Error += rblxmath::all(rblxmath::equal(B.i[0], rblxmath::vec2(1, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG

	return Error;
}

static int test_mat3x3_ctor()
{
	int Error = 0;

#	if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
	{
		union pack
		{
			rblxmath::mat3x3 f;
			rblxmath::mat3x3 i;
		} A, B;

		A.f = rblxmath::mat3x3(0);
		Error += rblxmath::all(rblxmath::equal(A.i[0], rblxmath::vec3(0), rblxmath::epsilon<float>())) ? 0 : 1;

		B.f = rblxmath::mat3x3(1);
		Error += rblxmath::all(rblxmath::equal(B.i[0], rblxmath::vec3(1, 0, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG

	return Error;
}

static int test_mat3x4_ctor()
{
	int Error = 0;

#	if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
	{
		union pack
		{
			rblxmath::mat3x4 f;
			rblxmath::mat3x4 i;
		} A, B;

		A.f = rblxmath::mat3x4(0);
		Error += rblxmath::all(rblxmath::equal(A.i[0], rblxmath::vec4(0), rblxmath::epsilon<float>())) ? 0 : 1;

		B.f = rblxmath::mat3x4(1);
		Error += rblxmath::all(rblxmath::equal(B.i[0], rblxmath::vec4(1, 0, 0, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG

	return Error;
}

static int test_mat4x2_ctor()
{
	int Error = 0;

#	if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
	{
		union pack
		{
			rblxmath::mat4x2 f;
			rblxmath::mat4x2 i;
		} A, B;

		A.f = rblxmath::mat4x2(0);
		Error += rblxmath::all(rblxmath::equal(A.i[0], rblxmath::vec2(0), rblxmath::epsilon<float>())) ? 0 : 1;

		B.f = rblxmath::mat4x2(1);
		Error += rblxmath::all(rblxmath::equal(B.i[0], rblxmath::vec2(1, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG

	return Error;
}

static int test_mat4x3_ctor()
{
	int Error = 0;

#	if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
	{
		union pack
		{
			rblxmath::mat4x3 f;
			rblxmath::mat4x3 i;
		} A, B;

		A.f = rblxmath::mat4x3(0);
		Error += rblxmath::all(rblxmath::equal(A.i[0], rblxmath::vec3(0), rblxmath::epsilon<float>())) ? 0 : 1;

		B.f = rblxmath::mat4x3(1);
		Error += rblxmath::all(rblxmath::equal(B.i[0], rblxmath::vec3(1, 0, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG

	return Error;
}

static int test_mat4x4_ctor()
{
	int Error = 0;

#	if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
	{
		union pack
		{
			rblxmath::mat4 f;
			rblxmath::mat4 i;
		} A, B;

		A.f = rblxmath::mat4(0);
		Error += rblxmath::all(rblxmath::equal(A.i[0], rblxmath::vec4(0), rblxmath::epsilon<float>())) ? 0 : 1;

		B.f = rblxmath::mat4(1);
		Error += rblxmath::all(rblxmath::equal(B.i[0], rblxmath::vec4(1, 0, 0, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG

	return Error;
}

static int test_quat_ctor()
{
	int Error = 0;

#	if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
	{
		union pack
		{
			rblxmath::quat f;
			rblxmath::quat i;
		} A, B;

		A.f = rblxmath::quat(0, 0, 0, 0);
		Error += rblxmath::all(rblxmath::equal(A.i, rblxmath::quat(0, 0, 0, 0), rblxmath::epsilon<float>())) ? 0 : 1;

		B.f = rblxmath::quat(1, 1, 1, 1);
		Error += rblxmath::all(rblxmath::equal(B.i, rblxmath::quat(1, 1, 1, 1), rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_vec1_ctor();
	Error += test_vec2_ctor();
	Error += test_vec3_ctor();
	Error += test_vec4_ctor();
	Error += test_mat2x2_ctor();
	Error += test_mat2x3_ctor();
	Error += test_mat2x4_ctor();
	Error += test_mat3x2_ctor();
	Error += test_mat3x3_ctor();
	Error += test_mat3x4_ctor();
	Error += test_mat4x2_ctor();
	Error += test_mat4x3_ctor();
	Error += test_mat4x4_ctor();
	Error += test_quat_ctor();

	return Error;
}
