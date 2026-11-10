#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/gtc/matrix_access.hpp>
#include <rblxmath/mat2x2.hpp>
#include <rblxmath/mat2x3.hpp>
#include <rblxmath/mat2x4.hpp>
#include <rblxmath/mat3x2.hpp>
#include <rblxmath/mat3x3.hpp>
#include <rblxmath/mat3x4.hpp>
#include <rblxmath/mat4x2.hpp>
#include <rblxmath/mat4x3.hpp>
#include <rblxmath/mat4x4.hpp>

static int test_mat2x2_row_set()
{
	int Error = 0;

	rblxmath::mat2x2 m(1);

	m = rblxmath::row(m, 0, rblxmath::vec2( 0,  1));
	m = rblxmath::row(m, 1, rblxmath::vec2( 4,  5));

	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 0), rblxmath::vec2( 0,  1), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 1), rblxmath::vec2( 4,  5), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat2x2_col_set()
{
	int Error = 0;

	rblxmath::mat2x2 m(1);

	m = rblxmath::column(m, 0, rblxmath::vec2( 0,  1));
	m = rblxmath::column(m, 1, rblxmath::vec2( 4,  5));

	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 0), rblxmath::vec2( 0,  1), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 1), rblxmath::vec2( 4,  5), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat2x3_row_set()
{
	int Error = 0;

	rblxmath::mat2x3 m(1);

	m = rblxmath::row(m, 0, rblxmath::vec2( 0,  1));
	m = rblxmath::row(m, 1, rblxmath::vec2( 4,  5));
	m = rblxmath::row(m, 2, rblxmath::vec2( 8,  9));

	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 0), rblxmath::vec2( 0,  1), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 1), rblxmath::vec2( 4,  5), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 2), rblxmath::vec2( 8,  9), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat2x3_col_set()
{
	int Error = 0;

	rblxmath::mat2x3 m(1);

	m = rblxmath::column(m, 0, rblxmath::vec3( 0,  1,  2));
	m = rblxmath::column(m, 1, rblxmath::vec3( 4,  5,  6));

	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 0), rblxmath::vec3( 0,  1,  2), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 1), rblxmath::vec3( 4,  5,  6), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat2x4_row_set()
{
	int Error = 0;

	rblxmath::mat2x4 m(1);

	m = rblxmath::row(m, 0, rblxmath::vec2( 0,  1));
	m = rblxmath::row(m, 1, rblxmath::vec2( 4,  5));
	m = rblxmath::row(m, 2, rblxmath::vec2( 8,  9));
	m = rblxmath::row(m, 3, rblxmath::vec2(12, 13));

	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 0), rblxmath::vec2( 0,  1), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 1), rblxmath::vec2( 4,  5), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 2), rblxmath::vec2( 8,  9), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 3), rblxmath::vec2(12, 13), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat2x4_col_set()
{
	int Error = 0;

	rblxmath::mat2x4 m(1);

	m = rblxmath::column(m, 0, rblxmath::vec4( 0,  1,  2, 3));
	m = rblxmath::column(m, 1, rblxmath::vec4( 4,  5,  6, 7));

	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 0), rblxmath::vec4( 0,  1,  2, 3), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 1), rblxmath::vec4( 4,  5,  6, 7), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat3x2_row_set()
{
	int Error = 0;

	rblxmath::mat3x2 m(1);

	m = rblxmath::row(m, 0, rblxmath::vec3( 0,  1,  2));
	m = rblxmath::row(m, 1, rblxmath::vec3( 4,  5,  6));

	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 0), rblxmath::vec3( 0,  1,  2), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 1), rblxmath::vec3( 4,  5,  6), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat3x2_col_set()
{
	int Error = 0;

	rblxmath::mat3x2 m(1);

	m = rblxmath::column(m, 0, rblxmath::vec2( 0,  1));
	m = rblxmath::column(m, 1, rblxmath::vec2( 4,  5));
	m = rblxmath::column(m, 2, rblxmath::vec2( 8,  9));

	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 0), rblxmath::vec2( 0,  1), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 1), rblxmath::vec2( 4,  5), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 2), rblxmath::vec2( 8,  9), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat3x3_row_set()
{
	int Error = 0;

	rblxmath::mat3x3 m(1);

	m = rblxmath::row(m, 0, rblxmath::vec3( 0,  1,  2));
	m = rblxmath::row(m, 1, rblxmath::vec3( 4,  5,  6));
	m = rblxmath::row(m, 2, rblxmath::vec3( 8,  9, 10));

	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 0), rblxmath::vec3( 0,  1,  2), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 1), rblxmath::vec3( 4,  5,  6), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 2), rblxmath::vec3( 8,  9, 10), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat3x3_col_set()
{
	int Error = 0;

	rblxmath::mat3x3 m(1);

	m = rblxmath::column(m, 0, rblxmath::vec3( 0,  1,  2));
	m = rblxmath::column(m, 1, rblxmath::vec3( 4,  5,  6));
	m = rblxmath::column(m, 2, rblxmath::vec3( 8,  9, 10));

	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 0), rblxmath::vec3( 0,  1,  2), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 1), rblxmath::vec3( 4,  5,  6), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 2), rblxmath::vec3( 8,  9, 10), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat3x4_row_set()
{
	int Error = 0;

	rblxmath::mat3x4 m(1);

	m = rblxmath::row(m, 0, rblxmath::vec3( 0,  1,  2));
	m = rblxmath::row(m, 1, rblxmath::vec3( 4,  5,  6));
	m = rblxmath::row(m, 2, rblxmath::vec3( 8,  9, 10));
	m = rblxmath::row(m, 3, rblxmath::vec3(12, 13, 14));

	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 0), rblxmath::vec3( 0,  1,  2), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 1), rblxmath::vec3( 4,  5,  6), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 2), rblxmath::vec3( 8,  9, 10), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 3), rblxmath::vec3(12, 13, 14), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat3x4_col_set()
{
	int Error = 0;

	rblxmath::mat3x4 m(1);

	m = rblxmath::column(m, 0, rblxmath::vec4( 0,  1,  2, 3));
	m = rblxmath::column(m, 1, rblxmath::vec4( 4,  5,  6, 7));
	m = rblxmath::column(m, 2, rblxmath::vec4( 8,  9, 10, 11));

	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 0), rblxmath::vec4( 0,  1,  2, 3), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 1), rblxmath::vec4( 4,  5,  6, 7), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 2), rblxmath::vec4( 8,  9, 10, 11), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat4x2_row_set()
{
	int Error = 0;

	rblxmath::mat4x2 m(1);

	m = rblxmath::row(m, 0, rblxmath::vec4( 0,  1,  2,  3));
	m = rblxmath::row(m, 1, rblxmath::vec4( 4,  5,  6,  7));

	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 0), rblxmath::vec4( 0,  1,  2,  3), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 1), rblxmath::vec4( 4,  5,  6,  7), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat4x2_col_set()
{
	int Error = 0;

	rblxmath::mat4x2 m(1);

	m = rblxmath::column(m, 0, rblxmath::vec2( 0,  1));
	m = rblxmath::column(m, 1, rblxmath::vec2( 4,  5));
	m = rblxmath::column(m, 2, rblxmath::vec2( 8,  9));
	m = rblxmath::column(m, 3, rblxmath::vec2(12, 13));

	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 0), rblxmath::vec2( 0,  1), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 1), rblxmath::vec2( 4,  5), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 2), rblxmath::vec2( 8,  9), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 3), rblxmath::vec2(12, 13), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat4x3_row_set()
{
	int Error = 0;

	rblxmath::mat4x3 m(1);

	m = rblxmath::row(m, 0, rblxmath::vec4( 0,  1,  2,  3));
	m = rblxmath::row(m, 1, rblxmath::vec4( 4,  5,  6,  7));
	m = rblxmath::row(m, 2, rblxmath::vec4( 8,  9, 10, 11));

	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 0), rblxmath::vec4( 0,  1,  2,  3), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 1), rblxmath::vec4( 4,  5,  6,  7), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 2), rblxmath::vec4( 8,  9, 10, 11), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat4x3_col_set()
{
	int Error = 0;

	rblxmath::mat4x3 m(1);

	m = rblxmath::column(m, 0, rblxmath::vec3( 0,  1,  2));
	m = rblxmath::column(m, 1, rblxmath::vec3( 4,  5,  6));
	m = rblxmath::column(m, 2, rblxmath::vec3( 8,  9, 10));
	m = rblxmath::column(m, 3, rblxmath::vec3(12, 13, 14));

	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 0), rblxmath::vec3( 0,  1,  2), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 1), rblxmath::vec3( 4,  5,  6), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 2), rblxmath::vec3( 8,  9, 10), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 3), rblxmath::vec3(12, 13, 14), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat4x4_row_set()
{
	int Error = 0;

	rblxmath::mat4 m(1);

	m = rblxmath::row(m, 0, rblxmath::vec4( 0,  1,  2,  3));
	m = rblxmath::row(m, 1, rblxmath::vec4( 4,  5,  6,  7));
	m = rblxmath::row(m, 2, rblxmath::vec4( 8,  9, 10, 11));
	m = rblxmath::row(m, 3, rblxmath::vec4(12, 13, 14, 15));

	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 0), rblxmath::vec4( 0,  1,  2,  3), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 1), rblxmath::vec4( 4,  5,  6,  7), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 2), rblxmath::vec4( 8,  9, 10, 11), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::row(m, 3), rblxmath::vec4(12, 13, 14, 15), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat4x4_col_set()
{
	int Error = 0;

	rblxmath::mat4 m(1);

	m = rblxmath::column(m, 0, rblxmath::vec4( 0,  1,  2,  3));
	m = rblxmath::column(m, 1, rblxmath::vec4( 4,  5,  6,  7));
	m = rblxmath::column(m, 2, rblxmath::vec4( 8,  9, 10, 11));
	m = rblxmath::column(m, 3, rblxmath::vec4(12, 13, 14, 15));

	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 0), rblxmath::vec4( 0,  1,  2,  3), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 1), rblxmath::vec4( 4,  5,  6,  7), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 2), rblxmath::vec4( 8,  9, 10, 11), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::column(m, 3), rblxmath::vec4(12, 13, 14, 15), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat4x4_row_get()
{
	int Error = 0;

	rblxmath::mat4 m(1);

	rblxmath::vec4 A = rblxmath::row(m, 0);
	Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec4(1, 0, 0, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	rblxmath::vec4 B = rblxmath::row(m, 1);
	Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec4(0, 1, 0, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	rblxmath::vec4 C = rblxmath::row(m, 2);
	Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec4(0, 0, 1, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	rblxmath::vec4 D = rblxmath::row(m, 3);
	Error += rblxmath::all(rblxmath::equal(D, rblxmath::vec4(0, 0, 0, 1), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_mat4x4_col_get()
{
	int Error = 0;

	rblxmath::mat4 m(1);

	rblxmath::vec4 A = rblxmath::column(m, 0);
	Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec4(1, 0, 0, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	rblxmath::vec4 B = rblxmath::column(m, 1);
	Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec4(0, 1, 0, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	rblxmath::vec4 C = rblxmath::column(m, 2);
	Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec4(0, 0, 1, 0), rblxmath::epsilon<float>())) ? 0 : 1;
	rblxmath::vec4 D = rblxmath::column(m, 3);
	Error += rblxmath::all(rblxmath::equal(D, rblxmath::vec4(0, 0, 0, 1), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_mat2x2_row_set();
	Error += test_mat2x2_col_set();
	Error += test_mat2x3_row_set();
	Error += test_mat2x3_col_set();
	Error += test_mat2x4_row_set();
	Error += test_mat2x4_col_set();
	Error += test_mat3x2_row_set();
	Error += test_mat3x2_col_set();
	Error += test_mat3x3_row_set();
	Error += test_mat3x3_col_set();
	Error += test_mat3x4_row_set();
	Error += test_mat3x4_col_set();
	Error += test_mat4x2_row_set();
	Error += test_mat4x2_col_set();
	Error += test_mat4x3_row_set();
	Error += test_mat4x3_col_set();
	Error += test_mat4x4_row_set();
	Error += test_mat4x4_col_set();

	Error += test_mat4x4_row_get();
	Error += test_mat4x4_col_get();

	return Error;
}
