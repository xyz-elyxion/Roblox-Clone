#include <rblxmath/gtc/type_ptr.hpp>
#include <rblxmath/gtc/vec1.hpp>
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/matrix_relational.hpp>

static int test_value_ptr_vec()
{
	int Error = 0;

	{
		rblxmath::vec2 v(1.0);
		float * p = rblxmath::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		rblxmath::vec3 v(1.0);
		float * p = rblxmath::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		rblxmath::vec4 v(1.0);
		float * p = rblxmath::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}

	{
		rblxmath::dvec2 v(1.0);
		double * p = rblxmath::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		rblxmath::dvec3 v(1.0);
		double * p = rblxmath::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		rblxmath::dvec4 v(1.0);
		double * p = rblxmath::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}

	return Error;
}

static int test_value_ptr_vec_const()
{
	int Error = 0;

	{
		rblxmath::vec2 const v(1.0);
		float const * p = rblxmath::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		rblxmath::vec3 const v(1.0);
		float const * p = rblxmath::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		rblxmath::vec4 const v(1.0);
		float const * p = rblxmath::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}

	{
		rblxmath::dvec2 const v(1.0);
		double const * p = rblxmath::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		rblxmath::dvec3 const v(1.0);
		double const * p = rblxmath::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}
	{
		rblxmath::dvec4 const v(1.0);
		double const * p = rblxmath::value_ptr(v);
		Error += p == &v[0] ? 0 : 1;
	}

	return Error;
}

static int test_value_ptr_mat()
{
	int Error = 0;

	{
		rblxmath::mat2x2 m(1.0);
		float * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat2x3 m(1.0);
		float * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat2x4 m(1.0);
		float * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat3x2 m(1.0);
		float * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat3x3 m(1.0);
		float * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat3x4 m(1.0);
		float * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat4x2 m(1.0);
		float * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat4x3 m(1.0);
		float * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat4x4 m(1.0);
		float * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}

	return Error;
}

static int test_value_ptr_mat_const()
{
	int Error = 0;

	{
		rblxmath::mat2x2 const m(1.0);
		float const * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat2x3 const m(1.0);
		float const * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat2x4 const m(1.0);
		float const * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat3x2 const m(1.0);
		float const * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat3x3 const m(1.0);
		float const * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat3x4 const m(1.0);
		float const * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat4x2 const m(1.0);
		float const * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat4x3 const m(1.0);
		float const * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}
	{
		rblxmath::mat4x4 const m(1.0);
		float const * p = rblxmath::value_ptr(m);
		Error += p == &m[0][0] ? 0 : 1;
	}

	return Error;
}

static int test_make_pointer_mat()
{
	int Error = 0;

	float ArrayA[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	double ArrayB[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

	rblxmath::mat2x2 Mat2x2A = rblxmath::make_mat2x2(ArrayA);
	Error += rblxmath::all(rblxmath::equal(Mat2x2A, rblxmath::mat2x2(0, 1, 2, 3), 0.001f)) ? 0 : 1;

	rblxmath::mat2x3 Mat2x3A = rblxmath::make_mat2x3(ArrayA);
	Error += rblxmath::all(rblxmath::equal(Mat2x3A, rblxmath::mat2x3(0, 1, 2, 3, 4, 5), 0.001f)) ? 0 : 1;

	rblxmath::mat2x4 Mat2x4A = rblxmath::make_mat2x4(ArrayA);
	Error += rblxmath::all(rblxmath::equal(Mat2x4A, rblxmath::mat2x4(0, 1, 2, 3, 4, 5, 6, 7), 0.001f)) ? 0 : 1;

	rblxmath::mat3x2 Mat3x2A = rblxmath::make_mat3x2(ArrayA);
	Error += rblxmath::all(rblxmath::equal(Mat3x2A, rblxmath::mat3x2(0, 1, 2, 3, 4, 5), 0.001f)) ? 0 : 1;

	rblxmath::mat3x3 Mat3x3A = rblxmath::make_mat3x3(ArrayA);
	Error += rblxmath::all(rblxmath::equal(Mat3x3A, rblxmath::mat3x3(0, 1, 2, 3, 4, 5, 6, 7, 8), 0.001f)) ? 0 : 1;

	rblxmath::mat3x4 Mat3x4A = rblxmath::make_mat3x4(ArrayA);
	Error += rblxmath::all(rblxmath::equal(Mat3x4A, rblxmath::mat3x4(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11), 0.001f)) ? 0 : 1;

	rblxmath::mat4x2 Mat4x2A = rblxmath::make_mat4x2(ArrayA);
	Error += rblxmath::all(rblxmath::equal(Mat4x2A, rblxmath::mat4x2(0, 1, 2, 3, 4, 5, 6, 7), 0.001f)) ? 0 : 1;

	rblxmath::mat4x3 Mat4x3A = rblxmath::make_mat4x3(ArrayA);
	Error += rblxmath::all(rblxmath::equal(Mat4x3A, rblxmath::mat4x3(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11), 0.001f)) ? 0 : 1;

	rblxmath::mat4x4 Mat4x4A = rblxmath::make_mat4x4(ArrayA);
	Error += rblxmath::all(rblxmath::equal(Mat4x4A, rblxmath::mat4x4(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15), 0.001f)) ? 0 : 1;



	rblxmath::dmat2x2 Mat2x2B = rblxmath::make_mat2x2(ArrayB);
	Error += rblxmath::all(rblxmath::equal(Mat2x2B, rblxmath::dmat2x2(0, 1, 2, 3), 0.001)) ? 0 : 1;

	rblxmath::dmat2x3 Mat2x3B = rblxmath::make_mat2x3(ArrayB);
	Error += rblxmath::all(rblxmath::equal(Mat2x3B, rblxmath::dmat2x3(0, 1, 2, 3, 4, 5), 0.001)) ? 0 : 1;

	rblxmath::dmat2x4 Mat2x4B = rblxmath::make_mat2x4(ArrayB);
	Error += rblxmath::all(rblxmath::equal(Mat2x4B, rblxmath::dmat2x4(0, 1, 2, 3, 4, 5, 6, 7), 0.001)) ? 0 : 1;

	rblxmath::dmat3x2 Mat3x2B = rblxmath::make_mat3x2(ArrayB);
	Error += rblxmath::all(rblxmath::equal(Mat3x2B, rblxmath::dmat3x2(0, 1, 2, 3, 4, 5), 0.001)) ? 0 : 1;

	rblxmath::dmat3x3 Mat3x3B = rblxmath::make_mat3x3(ArrayB);
	Error += rblxmath::all(rblxmath::equal(Mat3x3B, rblxmath::dmat3x3(0, 1, 2, 3, 4, 5, 6, 7, 8), 0.001)) ? 0 : 1;

	rblxmath::dmat3x4 Mat3x4B = rblxmath::make_mat3x4(ArrayB);
	Error += rblxmath::all(rblxmath::equal(Mat3x4B, rblxmath::dmat3x4(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11), 0.001)) ? 0 : 1;

	rblxmath::dmat4x2 Mat4x2B = rblxmath::make_mat4x2(ArrayB);
	Error += rblxmath::all(rblxmath::equal(Mat4x2B, rblxmath::dmat4x2(0, 1, 2, 3, 4, 5, 6, 7), 0.001)) ? 0 : 1;

	rblxmath::dmat4x3 Mat4x3B = rblxmath::make_mat4x3(ArrayB);
	Error += rblxmath::all(rblxmath::equal(Mat4x3B, rblxmath::dmat4x3(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11), 0.001)) ? 0 : 1;

	rblxmath::dmat4x4 Mat4x4B = rblxmath::make_mat4x4(ArrayB);
	Error += rblxmath::all(rblxmath::equal(Mat4x4B, rblxmath::dmat4x4(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15), 0.001)) ? 0 : 1;

	return Error;
}

static int test_make_pointer_vec()
{
	int Error = 0;

	float ArrayA[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	int ArrayB[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	bool ArrayC[] = {true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false};

	rblxmath::vec2 Vec2A = rblxmath::make_vec2(ArrayA);
	Error += rblxmath::all(rblxmath::equal(Vec2A, rblxmath::vec2(0, 1), 0.001f)) ? 0 : 1;

	rblxmath::vec3 Vec3A = rblxmath::make_vec3(ArrayA);
	Error += rblxmath::all(rblxmath::equal(Vec3A, rblxmath::vec3(0, 1, 2), 0.001f)) ? 0 : 1;

	rblxmath::vec4 Vec4A = rblxmath::make_vec4(ArrayA);
	Error += rblxmath::all(rblxmath::equal(Vec4A, rblxmath::vec4(0, 1, 2, 3), 0.001f)) ? 0 : 1;

	rblxmath::ivec2 Vec2B = rblxmath::make_vec2(ArrayB);
	Error += rblxmath::all(rblxmath::equal(Vec2B, rblxmath::ivec2(0, 1))) ? 0 : 1;

	rblxmath::ivec3 Vec3B = rblxmath::make_vec3(ArrayB);
	Error += rblxmath::all(rblxmath::equal(Vec3B, rblxmath::ivec3(0, 1, 2))) ? 0 : 1;

	rblxmath::ivec4 Vec4B = rblxmath::make_vec4(ArrayB);
	Error += rblxmath::all(rblxmath::equal(Vec4B, rblxmath::ivec4(0, 1, 2, 3))) ? 0 : 1;

	rblxmath::bvec2 Vec2C = rblxmath::make_vec2(ArrayC);
	Error += rblxmath::all(rblxmath::equal(Vec2C, rblxmath::bvec2(true, false))) ? 0 : 1;

	rblxmath::bvec3 Vec3C = rblxmath::make_vec3(ArrayC);
	Error += rblxmath::all(rblxmath::equal(Vec3C, rblxmath::bvec3(true, false, true))) ? 0 : 1;

	rblxmath::bvec4 Vec4C = rblxmath::make_vec4(ArrayC);
	Error += rblxmath::all(rblxmath::equal(Vec4C, rblxmath::bvec4(true, false, true, false))) ? 0 : 1;

	return Error;
}

static int test_make_vec1()
{
	int Error = 0;

	rblxmath::ivec1 const v1 = rblxmath::make_vec1(rblxmath::ivec1(2));
	Error += v1 == rblxmath::ivec1(2) ? 0 : 1;

	rblxmath::ivec1 const v2 = rblxmath::make_vec1(rblxmath::ivec2(2));
	Error += v2 == rblxmath::ivec1(2) ? 0 : 1;

	rblxmath::ivec1 const v3 = rblxmath::make_vec1(rblxmath::ivec3(2));
	Error += v3 == rblxmath::ivec1(2) ? 0 : 1;

	rblxmath::ivec1 const v4 = rblxmath::make_vec1(rblxmath::ivec4(2));
	Error += v4 == rblxmath::ivec1(2) ? 0 : 1;

	return Error;
}

static int test_make_vec2()
{
	int Error = 0;

	rblxmath::ivec2 const v1 = rblxmath::make_vec2(rblxmath::ivec1(2));
	Error += v1 == rblxmath::ivec2(2, 0) ? 0 : 1;

	rblxmath::ivec2 const v2 = rblxmath::make_vec2(rblxmath::ivec2(2));
	Error += v2 == rblxmath::ivec2(2, 2) ? 0 : 1;

	rblxmath::ivec2 const v3 = rblxmath::make_vec2(rblxmath::ivec3(2));
	Error += v3 == rblxmath::ivec2(2, 2) ? 0 : 1;

	rblxmath::ivec2 const v4 = rblxmath::make_vec2(rblxmath::ivec4(2));
	Error += v4 == rblxmath::ivec2(2, 2) ? 0 : 1;

	return Error;
}

static int test_make_vec3()
{
	int Error = 0;

	rblxmath::ivec3 const v1 = rblxmath::make_vec3(rblxmath::ivec1(2));
	Error += v1 == rblxmath::ivec3(2, 0, 0) ? 0 : 1;

	rblxmath::ivec3 const v2 = rblxmath::make_vec3(rblxmath::ivec2(2));
	Error += v2 == rblxmath::ivec3(2, 2, 0) ? 0 : 1;

	rblxmath::ivec3 const v3 = rblxmath::make_vec3(rblxmath::ivec3(2));
	Error += v3 == rblxmath::ivec3(2, 2, 2) ? 0 : 1;

	rblxmath::ivec3 const v4 = rblxmath::make_vec3(rblxmath::ivec4(2));
	Error += v4 == rblxmath::ivec3(2, 2, 2) ? 0 : 1;

	return Error;
}

static int test_make_vec4()
{
	int Error = 0;

	rblxmath::ivec4 const v1 = rblxmath::make_vec4(rblxmath::ivec1(2));
	Error += v1 == rblxmath::ivec4(2, 0, 0, 1) ? 0 : 1;

	rblxmath::ivec4 const v2 = rblxmath::make_vec4(rblxmath::ivec2(2));
	Error += v2 == rblxmath::ivec4(2, 2, 0, 1) ? 0 : 1;

	rblxmath::ivec4 const v3 = rblxmath::make_vec4(rblxmath::ivec3(2));
	Error += v3 == rblxmath::ivec4(2, 2, 2, 1) ? 0 : 1;

	rblxmath::ivec4 const v4 = rblxmath::make_vec4(rblxmath::ivec4(2));
	Error += v4 == rblxmath::ivec4(2, 2, 2, 2) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_make_vec1();
	Error += test_make_vec2();
	Error += test_make_vec3();
	Error += test_make_vec4();
	Error += test_make_pointer_vec();
	Error += test_make_pointer_mat();
	Error += test_value_ptr_vec();
	Error += test_value_ptr_vec_const();
	Error += test_value_ptr_mat();
	Error += test_value_ptr_mat_const();

	return Error;
}
