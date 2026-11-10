#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/vector_float1.hpp>
#include <rblxmath/ext/vector_float2.hpp>
#include <rblxmath/ext/vector_float3.hpp>
#include <rblxmath/ext/vector_float4.hpp>
#include <rblxmath/common.hpp>
#include <rblxmath/exponential.hpp>

static int test_pow()
{
	int Error(0);

	float A = rblxmath::pow(2.f, 2.f);
	Error += rblxmath::equal(A, 4.f, 0.01f) ? 0 : 1;

	rblxmath::vec1 B = rblxmath::pow(rblxmath::vec1(2.f), rblxmath::vec1(2.f));
	Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec1(4.f), 0.01f)) ? 0 : 1;

	rblxmath::vec2 C = rblxmath::pow(rblxmath::vec2(2.f), rblxmath::vec2(2.f));
	Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec2(4.f), 0.01f)) ? 0 : 1;

	rblxmath::vec3 D = rblxmath::pow(rblxmath::vec3(2.f), rblxmath::vec3(2.f));
	Error += rblxmath::all(rblxmath::equal(D, rblxmath::vec3(4.f), 0.01f)) ? 0 : 1;

	rblxmath::vec4 E = rblxmath::pow(rblxmath::vec4(2.f), rblxmath::vec4(2.f));
	Error += rblxmath::all(rblxmath::equal(E, rblxmath::vec4(4.f), 0.01f)) ? 0 : 1;

	return Error;
}

static int test_sqrt()
{
	int Error = 0;

	float A = rblxmath::sqrt(4.f);
	Error += rblxmath::equal(A, 2.f, 0.01f) ? 0 : 1;

	rblxmath::vec1 B = rblxmath::sqrt(rblxmath::vec1(4.f));
	Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec1(2.f), 0.01f)) ? 0 : 1;

	rblxmath::vec2 C = rblxmath::sqrt(rblxmath::vec2(4.f));
	Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec2(2.f), 0.01f)) ? 0 : 1;

	rblxmath::vec3 D = rblxmath::sqrt(rblxmath::vec3(4.f));
	Error += rblxmath::all(rblxmath::equal(D, rblxmath::vec3(2.f), 0.01f)) ? 0 : 1;

	rblxmath::vec4 E = rblxmath::sqrt(rblxmath::vec4(4.f));
	Error += rblxmath::all(rblxmath::equal(E, rblxmath::vec4(2.f), 0.01f)) ? 0 : 1;

	return Error;
}

static int test_exp()
{
	int Error = 0;

	float A = rblxmath::exp(1.f);
	Error += rblxmath::equal(A, rblxmath::e<float>(), 0.01f) ? 0 : 1;

	rblxmath::vec1 B = rblxmath::exp(rblxmath::vec1(1.f));
	Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec1(rblxmath::e<float>()), 0.01f)) ? 0 : 1;

	rblxmath::vec2 C = rblxmath::exp(rblxmath::vec2(1.f));
	Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec2(rblxmath::e<float>()), 0.01f)) ? 0 : 1;

	rblxmath::vec3 D = rblxmath::exp(rblxmath::vec3(1.f));
	Error += rblxmath::all(rblxmath::equal(D, rblxmath::vec3(rblxmath::e<float>()), 0.01f)) ? 0 : 1;

	rblxmath::vec4 E = rblxmath::exp(rblxmath::vec4(1.f));
	Error += rblxmath::all(rblxmath::equal(E, rblxmath::vec4(rblxmath::e<float>()), 0.01f)) ? 0 : 1;

	return Error;
}

static int test_log()
{
	int Error = 0;

	float const A = rblxmath::log(rblxmath::e<float>());
	Error += rblxmath::equal(A, 1.f, 0.01f) ? 0 : 1;

	rblxmath::vec1 const B = rblxmath::log(rblxmath::vec1(rblxmath::e<float>()));
	Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec1(1.f), 0.01f)) ? 0 : 1;

	rblxmath::vec2 const C = rblxmath::log(rblxmath::vec2(rblxmath::e<float>()));
	Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec2(1.f), 0.01f)) ? 0 : 1;

	rblxmath::vec3 const D = rblxmath::log(rblxmath::vec3(rblxmath::e<float>()));
	Error += rblxmath::all(rblxmath::equal(D, rblxmath::vec3(1.f), 0.01f)) ? 0 : 1;

	rblxmath::vec4 const E = rblxmath::log(rblxmath::vec4(rblxmath::e<float>()));
	Error += rblxmath::all(rblxmath::equal(E, rblxmath::vec4(1.f), 0.01f)) ? 0 : 1;

	return Error;
}

static int test_exp2()
{
	int Error = 0;

	float A = rblxmath::exp2(4.f);
	Error += rblxmath::equal(A, 16.f, 0.01f) ? 0 : 1;

	rblxmath::vec1 B = rblxmath::exp2(rblxmath::vec1(4.f));
	Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec1(16.f), 0.01f)) ? 0 : 1;

	rblxmath::vec2 C = rblxmath::exp2(rblxmath::vec2(4.f, 3.f));
	Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec2(16.f, 8.f), 0.01f)) ? 0 : 1;

	rblxmath::vec3 D = rblxmath::exp2(rblxmath::vec3(4.f, 3.f, 2.f));
	Error += rblxmath::all(rblxmath::equal(D, rblxmath::vec3(16.f, 8.f, 4.f), 0.01f)) ? 0 : 1;

	rblxmath::vec4 E = rblxmath::exp2(rblxmath::vec4(4.f, 3.f, 2.f, 1.f));
	Error += rblxmath::all(rblxmath::equal(E, rblxmath::vec4(16.f, 8.f, 4.f, 2.f), 0.01f)) ? 0 : 1;

#   if RBLXMATH_HAS_CXX11_STL
	//large exponent
	float F = rblxmath::exp2(23.f);
	Error += rblxmath::equal(F, 8388608.f, 0.01f) ? 0 : 1;
#   endif

	return Error;
}

static int test_log2()
{
	int Error = 0;

	float A = rblxmath::log2(16.f);
	Error += rblxmath::equal(A, 4.f, 0.01f) ? 0 : 1;

	rblxmath::vec1 B = rblxmath::log2(rblxmath::vec1(16.f));
	Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec1(4.f), 0.01f)) ? 0 : 1;

	rblxmath::vec2 C = rblxmath::log2(rblxmath::vec2(16.f, 8.f));
	Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec2(4.f, 3.f), 0.01f)) ? 0 : 1;

	rblxmath::vec3 D = rblxmath::log2(rblxmath::vec3(16.f, 8.f, 4.f));
	Error += rblxmath::all(rblxmath::equal(D, rblxmath::vec3(4.f, 3.f, 2.f), 0.01f)) ? 0 : 1;

	rblxmath::vec4 E = rblxmath::log2(rblxmath::vec4(16.f, 8.f, 4.f, 2.f));
	Error += rblxmath::all(rblxmath::equal(E, rblxmath::vec4(4.f, 3.f, 2.f, 1.f), 0.01f)) ? 0 : 1;

	return Error;
}

static int test_inversesqrt()
{
	int Error = 0;

	float A = rblxmath::inversesqrt(16.f) * rblxmath::sqrt(16.f);
	Error += rblxmath::equal(A, 1.f, 0.01f) ? 0 : 1;

	rblxmath::vec1 B = rblxmath::inversesqrt(rblxmath::vec1(16.f)) * rblxmath::sqrt(16.f);
	Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec1(1.f), 0.01f)) ? 0 : 1;

	rblxmath::vec2 C = rblxmath::inversesqrt(rblxmath::vec2(16.f)) * rblxmath::sqrt(16.f);
	Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec2(1.f), 0.01f)) ? 0 : 1;

	rblxmath::vec3 D = rblxmath::inversesqrt(rblxmath::vec3(16.f)) * rblxmath::sqrt(16.f);
	Error += rblxmath::all(rblxmath::equal(D, rblxmath::vec3(1.f), 0.01f)) ? 0 : 1;

	rblxmath::vec4 E = rblxmath::inversesqrt(rblxmath::vec4(16.f)) * rblxmath::sqrt(16.f);
	Error += rblxmath::all(rblxmath::equal(E, rblxmath::vec4(1.f), 0.01f)) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_pow();
	Error += test_sqrt();
	Error += test_exp();
	Error += test_log();
	Error += test_exp2();
	Error += test_log2();
	Error += test_inversesqrt();

	return Error;
}

