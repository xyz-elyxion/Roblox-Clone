#include <rblxmath/ext/matrix_relational.hpp>
#include <rblxmath/ext/matrix_transform.hpp>
#include <rblxmath/ext/matrix_float4x4.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/vector_float4.hpp>
#include <rblxmath/ext/vector_float3.hpp>

static int test_translate()
{
	int Error = 0;

	rblxmath::mat4 const M(1.0f);
	rblxmath::vec3 const V(1.0f);

	rblxmath::mat4 const T = rblxmath::translate(M, V);
	Error += rblxmath::all(rblxmath::equal(T[3], rblxmath::vec4(1.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_scale()
{
	int Error = 0;

	rblxmath::mat4 const M(1.0f);
	rblxmath::vec3 const V(2.0f);

	rblxmath::mat4 const S = rblxmath::scale(M, V);
	rblxmath::mat4 const R = rblxmath::mat4(
		rblxmath::vec4(2, 0, 0, 0),
		rblxmath::vec4(0, 2, 0, 0),
		rblxmath::vec4(0, 0, 2, 0),
		rblxmath::vec4(0, 0, 0, 1));
	Error += rblxmath::all(rblxmath::equal(S, R, rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_rotate()
{
	int Error = 0;

	rblxmath::vec4 const A(1.0f, 0.0f, 0.0f, 1.0f);

	rblxmath::mat4 const R = rblxmath::rotate(rblxmath::mat4(1.0f), rblxmath::radians(90.f), rblxmath::vec3(0, 0, 1));
	rblxmath::vec4 const B = R * A;
	Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.0001f)) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_translate();
	Error += test_scale();
	Error += test_rotate();

	return Error;
}
