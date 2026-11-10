#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/vector_float1.hpp>
#include <rblxmath/ext/vector_float1_precision.hpp>
#include <rblxmath/ext/vector_float2.hpp>
#include <rblxmath/ext/vector_float2_precision.hpp>
#include <rblxmath/ext/vector_float3.hpp>
#include <rblxmath/ext/vector_float3_precision.hpp>
#include <rblxmath/ext/vector_float4.hpp>
#include <rblxmath/ext/vector_float4_precision.hpp>
#include <rblxmath/ext/vector_double1.hpp>
#include <rblxmath/ext/vector_double1_precision.hpp>
#include <rblxmath/ext/vector_double2.hpp>
#include <rblxmath/ext/vector_double2_precision.hpp>
#include <rblxmath/ext/vector_double3.hpp>
#include <rblxmath/ext/vector_double3_precision.hpp>
#include <rblxmath/ext/vector_double4.hpp>
#include <rblxmath/ext/vector_double4_precision.hpp>
#include <rblxmath/ext/vector_ulp.hpp>

template <typename vecType>
static int test_equal()
{
	typedef typename vecType::value_type valType;

	valType const A = static_cast<valType>(1.01f);
	valType const B = static_cast<valType>(1.02f);
	valType const Epsilon1 = static_cast<valType>(0.1f);
	valType const Epsilon2 = static_cast<valType>(0.001f);

	int Error = 0;

	Error += rblxmath::all(rblxmath::equal(vecType(A), vecType(B), Epsilon1)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(vecType(A), vecType(B), vecType(Epsilon1))) ? 0 : 1;

	Error += !rblxmath::any(rblxmath::equal(vecType(A), vecType(B), Epsilon2)) ? 0 : 1;
	Error += !rblxmath::any(rblxmath::equal(vecType(A), vecType(B), vecType(Epsilon2))) ? 0 : 1;

	return Error;
}

template <typename vecType>
static int test_notEqual()
{
	typedef typename vecType::value_type valType;

	valType const A = static_cast<valType>(1.01f);
	valType const B = static_cast<valType>(1.02f);
	valType const Epsilon1 = static_cast<valType>(0.1f);
	valType const Epsilon2 = static_cast<valType>(0.001f);

	int Error = 0;

	Error += rblxmath::all(rblxmath::notEqual(vecType(A), vecType(B), Epsilon2)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::notEqual(vecType(A), vecType(B), vecType(Epsilon2))) ? 0 : 1;

	Error += !rblxmath::any(rblxmath::notEqual(vecType(A), vecType(B), Epsilon1)) ? 0 : 1;
	Error += !rblxmath::any(rblxmath::notEqual(vecType(A), vecType(B), vecType(Epsilon1))) ? 0 : 1;

	return Error;
}

template <typename genType, typename valType>
static int test_constexpr()
{
#	if RBLXMATH_CONFIG_CONSTEXP == RBLXMATH_ENABLE
		static_assert(rblxmath::all(rblxmath::equal(genType(static_cast<valType>(1.01f)), genType(static_cast<valType>(1.02f)), static_cast<valType>(0.1f))), "RBLXMATH: Failed constexpr");
#	endif

	return 0;
}

template <typename T>
static int test_equal_ulps()
{
	typedef rblxmath::vec<4, T, rblxmath::defaultp> vec4;

	T const One(1);
	vec4 const Ones(1);

	int Error = 0;

	T const ULP1Plus = rblxmath::nextFloat(One);
	Error += rblxmath::all(rblxmath::equal(Ones, vec4(ULP1Plus), 1)) ? 0 : 1;

	T const ULP2Plus = rblxmath::nextFloat(ULP1Plus);
	Error += !rblxmath::all(rblxmath::equal(Ones, vec4(ULP2Plus), 1)) ? 0 : 1;

	T const ULP1Minus = rblxmath::prevFloat(One);
	Error += rblxmath::all(rblxmath::equal(Ones, vec4(ULP1Minus), 1)) ? 0 : 1;

	T const ULP2Minus = rblxmath::prevFloat(ULP1Minus);
	Error += !rblxmath::all(rblxmath::equal(Ones, vec4(ULP2Minus), 1)) ? 0 : 1;

	return Error;
}

template <typename T>
static int test_notEqual_ulps()
{
	typedef rblxmath::vec<4, T, rblxmath::defaultp> vec4;

	T const One(1);
	vec4 const Ones(1);

	int Error = 0;

	T const ULP1Plus = rblxmath::nextFloat(One);
	Error += !rblxmath::all(rblxmath::notEqual(Ones, vec4(ULP1Plus), 1)) ? 0 : 1;

	T const ULP2Plus = rblxmath::nextFloat(ULP1Plus);
	Error += rblxmath::all(rblxmath::notEqual(Ones, vec4(ULP2Plus), 1)) ? 0 : 1;

	T const ULP1Minus = rblxmath::prevFloat(One);
	Error += !rblxmath::all(rblxmath::notEqual(Ones, vec4(ULP1Minus), 1)) ? 0 : 1;

	T const ULP2Minus = rblxmath::prevFloat(ULP1Minus);
	Error += rblxmath::all(rblxmath::notEqual(Ones, vec4(ULP2Minus), 1)) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_equal_ulps<float>();
	Error += test_equal_ulps<double>();
	Error += test_notEqual_ulps<float>();
	Error += test_notEqual_ulps<double>();

	Error += test_equal<rblxmath::vec1>();
	Error += test_equal<rblxmath::lowp_vec1>();
	Error += test_equal<rblxmath::mediump_vec1>();
	Error += test_equal<rblxmath::highp_vec1>();
	Error += test_equal<rblxmath::vec2>();
	Error += test_equal<rblxmath::lowp_vec2>();
	Error += test_equal<rblxmath::mediump_vec2>();
	Error += test_equal<rblxmath::highp_vec2>();
	Error += test_equal<rblxmath::vec3>();
	Error += test_equal<rblxmath::lowp_vec3>();
	Error += test_equal<rblxmath::mediump_vec3>();
	Error += test_equal<rblxmath::highp_vec3>();
	Error += test_equal<rblxmath::vec4>();
	Error += test_equal<rblxmath::lowp_vec4>();
	Error += test_equal<rblxmath::mediump_vec4>();
	Error += test_equal<rblxmath::highp_vec4>();

	Error += test_equal<rblxmath::dvec1>();
	Error += test_equal<rblxmath::lowp_dvec1>();
	Error += test_equal<rblxmath::mediump_dvec1>();
	Error += test_equal<rblxmath::highp_dvec1>();
	Error += test_equal<rblxmath::dvec2>();
	Error += test_equal<rblxmath::lowp_dvec2>();
	Error += test_equal<rblxmath::mediump_dvec2>();
	Error += test_equal<rblxmath::highp_dvec2>();
	Error += test_equal<rblxmath::dvec3>();
	Error += test_equal<rblxmath::lowp_dvec3>();
	Error += test_equal<rblxmath::mediump_dvec3>();
	Error += test_equal<rblxmath::highp_dvec3>();
	Error += test_equal<rblxmath::dvec4>();
	Error += test_equal<rblxmath::lowp_dvec4>();
	Error += test_equal<rblxmath::mediump_dvec4>();
	Error += test_equal<rblxmath::highp_dvec4>();

	Error += test_notEqual<rblxmath::vec1>();
	Error += test_notEqual<rblxmath::lowp_vec1>();
	Error += test_notEqual<rblxmath::mediump_vec1>();
	Error += test_notEqual<rblxmath::highp_vec1>();
	Error += test_notEqual<rblxmath::vec2>();
	Error += test_notEqual<rblxmath::lowp_vec2>();
	Error += test_notEqual<rblxmath::mediump_vec2>();
	Error += test_notEqual<rblxmath::highp_vec2>();
	Error += test_notEqual<rblxmath::vec3>();
	Error += test_notEqual<rblxmath::lowp_vec3>();
	Error += test_notEqual<rblxmath::mediump_vec3>();
	Error += test_notEqual<rblxmath::highp_vec3>();
	Error += test_notEqual<rblxmath::vec4>();
	Error += test_notEqual<rblxmath::lowp_vec4>();
	Error += test_notEqual<rblxmath::mediump_vec4>();
	Error += test_notEqual<rblxmath::highp_vec4>();

	Error += test_notEqual<rblxmath::dvec1>();
	Error += test_notEqual<rblxmath::lowp_dvec1>();
	Error += test_notEqual<rblxmath::mediump_dvec1>();
	Error += test_notEqual<rblxmath::highp_dvec1>();
	Error += test_notEqual<rblxmath::dvec2>();
	Error += test_notEqual<rblxmath::lowp_dvec2>();
	Error += test_notEqual<rblxmath::mediump_dvec2>();
	Error += test_notEqual<rblxmath::highp_dvec2>();
	Error += test_notEqual<rblxmath::dvec3>();
	Error += test_notEqual<rblxmath::lowp_dvec3>();
	Error += test_notEqual<rblxmath::mediump_dvec3>();
	Error += test_notEqual<rblxmath::highp_dvec3>();
	Error += test_notEqual<rblxmath::dvec4>();
	Error += test_notEqual<rblxmath::lowp_dvec4>();
	Error += test_notEqual<rblxmath::mediump_dvec4>();
	Error += test_notEqual<rblxmath::highp_dvec4>();

	Error += test_constexpr<rblxmath::vec1, float>();
	Error += test_constexpr<rblxmath::vec2, float>();
	Error += test_constexpr<rblxmath::vec3, float>();
	Error += test_constexpr<rblxmath::vec4, float>();

	return Error;
}
