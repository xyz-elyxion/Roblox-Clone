#include <rblxmath/ext/matrix_relational.hpp>
#include <rblxmath/ext/matrix_double2x2.hpp>
#include <rblxmath/ext/matrix_double2x3.hpp>
#include <rblxmath/ext/matrix_double2x4.hpp>
#include <rblxmath/ext/matrix_double3x2.hpp>
#include <rblxmath/ext/matrix_double3x3.hpp>
#include <rblxmath/ext/matrix_double3x4.hpp>
#include <rblxmath/ext/matrix_double4x2.hpp>
#include <rblxmath/ext/matrix_double4x3.hpp>
#include <rblxmath/ext/matrix_double4x4.hpp>
#include <rblxmath/ext/vector_double2.hpp>
#include <rblxmath/ext/vector_double3.hpp>
#include <rblxmath/ext/vector_double4.hpp>
#include <rblxmath/ext/matrix_float2x2.hpp>
#include <rblxmath/ext/matrix_float2x3.hpp>
#include <rblxmath/ext/matrix_float2x4.hpp>
#include <rblxmath/ext/matrix_float3x2.hpp>
#include <rblxmath/ext/matrix_float3x3.hpp>
#include <rblxmath/ext/matrix_float3x4.hpp>
#include <rblxmath/ext/matrix_float4x2.hpp>
#include <rblxmath/ext/matrix_float4x3.hpp>
#include <rblxmath/ext/matrix_float4x4.hpp>
#include <rblxmath/ext/vector_float2.hpp>
#include <rblxmath/ext/vector_float3.hpp>
#include <rblxmath/ext/vector_float4.hpp>
#include <rblxmath/ext/scalar_ulp.hpp>

template <typename matType, typename vecType>
static int test_equal()
{
	typedef typename matType::value_type valType;

	valType const Epsilon = static_cast<valType>(0.001f);
	valType const One = static_cast<valType>(1);
	valType const Two = static_cast<valType>(2);

	int Error = 0;

	Error += rblxmath::all(rblxmath::equal(matType(One), matType(One), Epsilon)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(matType(One), matType(Two), vecType(Epsilon))) ? 1 : 0;

	return Error;
}

template <typename matType, typename vecType>
static int test_notEqual()
{
	typedef typename matType::value_type valType;

	valType const Epsilon = static_cast<valType>(0.001f);
	valType const One = static_cast<valType>(1);
	valType const Two = static_cast<valType>(2);

	int Error = 0;

	Error += !rblxmath::any(rblxmath::notEqual(matType(One), matType(One), Epsilon)) ? 0 : 1;
	Error += !rblxmath::any(rblxmath::notEqual(matType(One), matType(Two), vecType(Epsilon))) ? 1 : 0;

	return Error;
}


template <typename T>
static int test_equal_ulps()
{
	typedef rblxmath::mat<4, 4, T, rblxmath::defaultp> mat4;

	T const One(1);
	mat4 const Ones(1);

	int Error = 0;

	T const ULP1Plus = rblxmath::nextFloat(One);
	Error += rblxmath::all(rblxmath::equal(Ones, mat4(ULP1Plus), 1)) ? 0 : 1;

	T const ULP2Plus = rblxmath::nextFloat(ULP1Plus);
	Error += !rblxmath::all(rblxmath::equal(Ones, mat4(ULP2Plus), 1)) ? 0 : 1;

	T const ULP1Minus = rblxmath::prevFloat(One);
	Error += rblxmath::all(rblxmath::equal(Ones, mat4(ULP1Minus), 1)) ? 0 : 1;

	T const ULP2Minus = rblxmath::prevFloat(ULP1Minus);
	Error += !rblxmath::all(rblxmath::equal(Ones, mat4(ULP2Minus), 1)) ? 0 : 1;

	return Error;
}

template <typename T>
static int test_notEqual_ulps()
{
	typedef rblxmath::mat<4, 4, T, rblxmath::defaultp> mat4;

	T const One(1);
	mat4 const Ones(1);

	int Error = 0;

	T const ULP1Plus = rblxmath::nextFloat(One);
	Error += !rblxmath::all(rblxmath::notEqual(Ones, mat4(ULP1Plus), 1)) ? 0 : 1;

	T const ULP2Plus = rblxmath::nextFloat(ULP1Plus);
	Error += rblxmath::all(rblxmath::notEqual(Ones, mat4(ULP2Plus), 1)) ? 0 : 1;

	T const ULP1Minus = rblxmath::prevFloat(One);
	Error += !rblxmath::all(rblxmath::notEqual(Ones, mat4(ULP1Minus), 1)) ? 0 : 1;

	T const ULP2Minus = rblxmath::prevFloat(ULP1Minus);
	Error += rblxmath::all(rblxmath::notEqual(Ones, mat4(ULP2Minus), 1)) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_equal_ulps<float>();
	Error += test_equal_ulps<double>();
	Error += test_notEqual_ulps<float>();
	Error += test_notEqual_ulps<double>();

	Error += test_equal<rblxmath::mat2x2, rblxmath::vec2>();
	Error += test_equal<rblxmath::mat2x3, rblxmath::vec2>();
	Error += test_equal<rblxmath::mat2x4, rblxmath::vec2>();
	Error += test_equal<rblxmath::mat3x2, rblxmath::vec3>();
	Error += test_equal<rblxmath::mat3x3, rblxmath::vec3>();
	Error += test_equal<rblxmath::mat3x4, rblxmath::vec3>();
	Error += test_equal<rblxmath::mat4x2, rblxmath::vec4>();
	Error += test_equal<rblxmath::mat4x3, rblxmath::vec4>();
	Error += test_equal<rblxmath::mat4x4, rblxmath::vec4>();

	Error += test_equal<rblxmath::dmat2x2, rblxmath::dvec2>();
	Error += test_equal<rblxmath::dmat2x3, rblxmath::dvec2>();
	Error += test_equal<rblxmath::dmat2x4, rblxmath::dvec2>();
	Error += test_equal<rblxmath::dmat3x2, rblxmath::dvec3>();
	Error += test_equal<rblxmath::dmat3x3, rblxmath::dvec3>();
	Error += test_equal<rblxmath::dmat3x4, rblxmath::dvec3>();
	Error += test_equal<rblxmath::dmat4x2, rblxmath::dvec4>();
	Error += test_equal<rblxmath::dmat4x3, rblxmath::dvec4>();
	Error += test_equal<rblxmath::dmat4x4, rblxmath::dvec4>();

	Error += test_notEqual<rblxmath::mat2x2, rblxmath::vec2>();
	Error += test_notEqual<rblxmath::mat2x3, rblxmath::vec2>();
	Error += test_notEqual<rblxmath::mat2x4, rblxmath::vec2>();
	Error += test_notEqual<rblxmath::mat3x2, rblxmath::vec3>();
	Error += test_notEqual<rblxmath::mat3x3, rblxmath::vec3>();
	Error += test_notEqual<rblxmath::mat3x4, rblxmath::vec3>();
	Error += test_notEqual<rblxmath::mat4x2, rblxmath::vec4>();
	Error += test_notEqual<rblxmath::mat4x3, rblxmath::vec4>();
	Error += test_notEqual<rblxmath::mat4x4, rblxmath::vec4>();

	Error += test_notEqual<rblxmath::dmat2x2, rblxmath::dvec2>();
	Error += test_notEqual<rblxmath::dmat2x3, rblxmath::dvec2>();
	Error += test_notEqual<rblxmath::dmat2x4, rblxmath::dvec2>();
	Error += test_notEqual<rblxmath::dmat3x2, rblxmath::dvec3>();
	Error += test_notEqual<rblxmath::dmat3x3, rblxmath::dvec3>();
	Error += test_notEqual<rblxmath::dmat3x4, rblxmath::dvec3>();
	Error += test_notEqual<rblxmath::dmat4x2, rblxmath::dvec4>();
	Error += test_notEqual<rblxmath::dmat4x3, rblxmath::dvec4>();
	Error += test_notEqual<rblxmath::dmat4x4, rblxmath::dvec4>();

	return Error;
}
