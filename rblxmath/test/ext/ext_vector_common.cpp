#include <rblxmath/ext/vector_common.hpp>

#include <rblxmath/ext/vector_bool1.hpp>
#include <rblxmath/ext/vector_bool1_precision.hpp>
#include <rblxmath/ext/vector_bool2.hpp>
#include <rblxmath/ext/vector_bool2_precision.hpp>
#include <rblxmath/ext/vector_bool3.hpp>
#include <rblxmath/ext/vector_bool3_precision.hpp>
#include <rblxmath/ext/vector_bool4.hpp>
#include <rblxmath/ext/vector_bool4_precision.hpp>

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
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/scalar_constants.hpp>
#include <rblxmath/vector_relational.hpp>
#include <rblxmath/common.hpp>

template <typename vecType>
static int test_min()
{
	typedef typename vecType::value_type T;

	int Error = 0;

	vecType const N(static_cast<T>(0));
	vecType const B(static_cast<T>(1));

	Error += rblxmath::all(rblxmath::equal(rblxmath::min(N, B), N, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::min(B, N), N, rblxmath::epsilon<T>())) ? 0 : 1;

	vecType const C(static_cast<T>(2));
	Error += rblxmath::all(rblxmath::equal(rblxmath::min(N, B, C), N, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::min(B, N, C), N, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::min(C, N, B), N, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::min(C, B, N), N, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::min(B, C, N), N, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::min(N, C, B), N, rblxmath::epsilon<T>())) ? 0 : 1;

	vecType const D(static_cast<T>(3));
	Error += rblxmath::all(rblxmath::equal(rblxmath::min(D, N, B, C), N, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::min(B, D, N, C), N, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::min(C, N, D, B), N, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::min(C, B, D, N), N, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::min(B, C, N, D), N, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::min(N, C, B, D), N, rblxmath::epsilon<T>())) ? 0 : 1;

	return Error;
}

template <typename vecType>
static int test_max()
{
	typedef typename vecType::value_type T;

	int Error = 0;

	vecType const N(static_cast<T>(0));
	vecType const B(static_cast<T>(1));
	Error += rblxmath::all(rblxmath::equal(rblxmath::max(N, B), B, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::max(B, N), B, rblxmath::epsilon<T>())) ? 0 : 1;

	vecType const C(static_cast<T>(2));
	Error += rblxmath::all(rblxmath::equal(rblxmath::max(N, B, C), C, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::max(B, N, C), C, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::max(C, N, B), C, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::max(C, B, N), C, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::max(B, C, N), C, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::max(N, C, B), C, rblxmath::epsilon<T>())) ? 0 : 1;

	vecType const D(static_cast<T>(3));
	Error += rblxmath::all(rblxmath::equal(rblxmath::max(D, N, B, C), D, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::max(B, D, N, C), D, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::max(C, N, D, B), D, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::max(C, B, D, N), D, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::max(B, C, N, D), D, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::max(N, C, B, D), D, rblxmath::epsilon<T>())) ? 0 : 1;

	return Error;
}

#if ((RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG) || (RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC))
template <typename vecType>
static int test_min_nan()
{
	typedef typename vecType::value_type T;

	int Error = 0;

	vecType const B(static_cast<T>(1));
	vecType const N(NAN);

	Error += rblxmath::all(rblxmath::isnan(rblxmath::min(N, B))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::min(B, N))) ? 0 : 1;

	vecType const C(static_cast<T>(2));
	Error += rblxmath::all(rblxmath::isnan(rblxmath::min(N, B, C))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::min(B, N, C))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::min(C, N, B))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::min(C, B, N))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::min(B, C, N))) ? 0 : 1;
	Error += rblxmath::all(rblxmath::isnan(rblxmath::min(N, C, B))) ? 0 : 1;

	vecType const D(static_cast<T>(3));
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::min(D, N, B, C))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::min(B, D, N, C))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::min(C, N, D, B))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::min(C, B, D, N))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::min(B, C, N, D))) ? 0 : 1;
	Error += rblxmath::all(rblxmath::isnan(rblxmath::min(N, C, B, D))) ? 0 : 1;

	return Error;
}

template <typename vecType>
static int test_max_nan()
{
	typedef typename vecType::value_type T;

	int Error = 0;

	vecType const B(static_cast<T>(1));
	vecType const N(NAN);

	Error += rblxmath::all(rblxmath::isnan(rblxmath::max(N, B))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::max(B, N))) ? 0 : 1;

	vecType const C(static_cast<T>(2));
	Error += rblxmath::all(rblxmath::isnan(rblxmath::max(N, B, C))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::max(B, N, C))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::max(C, N, B))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::max(C, B, N))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::max(B, C, N))) ? 0 : 1;
	Error += rblxmath::all(rblxmath::isnan(rblxmath::max(N, C, B))) ? 0 : 1;

	vecType const D(static_cast<T>(3));
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::max(D, N, B, C))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::max(B, D, N, C))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::max(C, N, D, B))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::max(C, B, D, N))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::isnan(rblxmath::max(B, C, N, D))) ? 0 : 1;
	Error += rblxmath::all(rblxmath::isnan(rblxmath::max(N, C, B, D))) ? 0 : 1;

	return Error;
}

template <typename vecType>
static int test_fmin()
{
	typedef typename vecType::value_type T;

	int Error = 0;

	vecType const B(static_cast<T>(1));
	vecType const N(NAN);

	Error += rblxmath::all(rblxmath::equal(rblxmath::fmin(N, B), B, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmin(B, N), B, rblxmath::epsilon<T>())) ? 0 : 1;

	vecType const C(static_cast<T>(2));
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmin(N, B, C), B, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmin(B, N, C), B, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmin(C, N, B), B, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmin(C, B, N), B, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmin(B, C, N), B, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmin(N, C, B), B, rblxmath::epsilon<T>())) ? 0 : 1;

	vecType const D(static_cast<T>(3));
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmin(D, N, B, C), B, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmin(B, D, N, C), B, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmin(C, N, D, B), B, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmin(C, B, D, N), B, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmin(B, C, N, D), B, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmin(N, C, B, D), B, rblxmath::epsilon<T>())) ? 0 : 1;

	return Error;
}

template <typename vecType>
static int test_fmax()
{
	typedef typename vecType::value_type T;

	int Error = 0;

	vecType const B(static_cast<T>(1));
	vecType const N(NAN);

	Error += rblxmath::all(rblxmath::equal(rblxmath::fmax(N, B), B, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmax(B, N), B, rblxmath::epsilon<T>())) ? 0 : 1;

	vecType const C(static_cast<T>(2));
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmax(N, B, C), C, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmax(B, N, C), C, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmax(C, N, B), C, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmax(C, B, N), C, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmax(B, C, N), C, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmax(N, C, B), C, rblxmath::epsilon<T>())) ? 0 : 1;

	vecType const D(static_cast<T>(3));
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmax(D, N, B, C), D, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmax(B, D, N, C), D, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmax(C, N, D, B), D, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmax(C, B, D, N), D, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmax(B, C, N, D), D, rblxmath::epsilon<T>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::fmax(N, C, B, D), D, rblxmath::epsilon<T>())) ? 0 : 1;

	return Error;
}
#endif//((RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG) || (RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC))

static int test_clamp()
{
	int Error = 0;

	rblxmath::vec2 K = rblxmath::clamp(rblxmath::vec2(0.5f));
	Error += rblxmath::all(rblxmath::equal(K, rblxmath::vec2(0.5f), rblxmath::vec2(0.00001f))) ? 0 : 1;

	rblxmath::vec3 L = rblxmath::clamp(rblxmath::vec3(0.5f));
	Error += rblxmath::all(rblxmath::equal(L, rblxmath::vec3(0.5f), rblxmath::vec3(0.00001f))) ? 0 : 1;

	rblxmath::vec4 M = rblxmath::clamp(rblxmath::vec4(0.5f));
	Error += rblxmath::all(rblxmath::equal(M, rblxmath::vec4(0.5f), rblxmath::vec4(0.00001f))) ? 0 : 1;

	rblxmath::vec1 N = rblxmath::clamp(rblxmath::vec1(0.5f));
	Error += rblxmath::all(rblxmath::equal(N, rblxmath::vec1(0.5f), rblxmath::vec1(0.00001f))) ? 0 : 1;

	return Error;
}

static int test_repeat()
{
	int Error = 0;

	rblxmath::vec2 K = rblxmath::repeat(rblxmath::vec2(0.5f));
	Error += rblxmath::all(rblxmath::equal(K, rblxmath::vec2(0.5f), rblxmath::vec2(0.00001f))) ? 0 : 1;

	rblxmath::vec3 L = rblxmath::repeat(rblxmath::vec3(0.5f));
	Error += rblxmath::all(rblxmath::equal(L, rblxmath::vec3(0.5f), rblxmath::vec3(0.00001f))) ? 0 : 1;

	rblxmath::vec4 M = rblxmath::repeat(rblxmath::vec4(0.5f));
	Error += rblxmath::all(rblxmath::equal(M, rblxmath::vec4(0.5f), rblxmath::vec4(0.00001f))) ? 0 : 1;

	rblxmath::vec1 N = rblxmath::repeat(rblxmath::vec1(0.5f));
	Error += rblxmath::all(rblxmath::equal(N, rblxmath::vec1(0.5f), rblxmath::vec1(0.00001f))) ? 0 : 1;

	return Error;
}

static int test_mirrorClamp()
{
	int Error = 0;

	rblxmath::vec2 K = rblxmath::mirrorClamp(rblxmath::vec2(0.5f));
	Error += rblxmath::all(rblxmath::equal(K, rblxmath::vec2(0.5f), rblxmath::vec2(0.00001f))) ? 0 : 1;

	rblxmath::vec3 L = rblxmath::mirrorClamp(rblxmath::vec3(0.5f));
	Error += rblxmath::all(rblxmath::equal(L, rblxmath::vec3(0.5f), rblxmath::vec3(0.00001f))) ? 0 : 1;

	rblxmath::vec4 M = rblxmath::mirrorClamp(rblxmath::vec4(0.5f));
	Error += rblxmath::all(rblxmath::equal(M, rblxmath::vec4(0.5f), rblxmath::vec4(0.00001f))) ? 0 : 1;

	rblxmath::vec1 N = rblxmath::mirrorClamp(rblxmath::vec1(0.5f));
	Error += rblxmath::all(rblxmath::equal(N, rblxmath::vec1(0.5f), rblxmath::vec1(0.00001f))) ? 0 : 1;

	return Error;
}

static int test_mirrorRepeat()
{
	int Error = 0;

	rblxmath::vec2 K = rblxmath::mirrorRepeat(rblxmath::vec2(0.5f));
	Error += rblxmath::all(rblxmath::equal(K, rblxmath::vec2(0.5f), rblxmath::vec2(0.00001f))) ? 0 : 1;

	rblxmath::vec3 L = rblxmath::mirrorRepeat(rblxmath::vec3(0.5f));
	Error += rblxmath::all(rblxmath::equal(L, rblxmath::vec3(0.5f), rblxmath::vec3(0.00001f))) ? 0 : 1;

	rblxmath::vec4 M = rblxmath::mirrorRepeat(rblxmath::vec4(0.5f));
	Error += rblxmath::all(rblxmath::equal(M, rblxmath::vec4(0.5f), rblxmath::vec4(0.00001f))) ? 0 : 1;

	rblxmath::vec1 N = rblxmath::mirrorRepeat(rblxmath::vec1(0.5f));
	Error += rblxmath::all(rblxmath::equal(N, rblxmath::vec1(0.5f), rblxmath::vec1(0.00001f))) ? 0 : 1;

	return Error;
}

static int test_iround()
{
	int Error = 0;

	for(float f = 0.0f; f < 3.1f; f += 0.05f)
	{
		int RoundFast = static_cast<int>(rblxmath::iround(f));
		int RoundSTD = static_cast<int>(rblxmath::round(f));
		Error += RoundFast == RoundSTD ? 0 : 1;
		assert(!Error);
	}

	return Error;
}

static int test_uround()
{
	int Error = 0;

	for(float f = 0.0f; f < 3.1f; f += 0.05f)
	{
		int RoundFast = static_cast<int>(rblxmath::uround(f));
		int RoundSTD = static_cast<int>(rblxmath::round(f));
		Error += RoundFast == RoundSTD ? 0 : 1;
		assert(!Error);
	}

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_min<rblxmath::vec3>();
	Error += test_min<rblxmath::vec2>();
	Error += test_max<rblxmath::vec3>();
	Error += test_max<rblxmath::vec2>();

#if ((RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG) || (RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC))
	Error += test_min_nan<rblxmath::vec3>();
	Error += test_min_nan<rblxmath::vec2>();
	Error += test_max_nan<rblxmath::vec3>();
	Error += test_max_nan<rblxmath::vec2>();

	Error += test_fmin<rblxmath::vec3>();
	Error += test_fmin<rblxmath::vec2>();
	Error += test_fmax<rblxmath::vec3>();
	Error += test_fmax<rblxmath::vec2>();
#endif//

	Error += test_clamp();
	Error += test_repeat();
	Error += test_mirrorClamp();
	Error += test_mirrorRepeat();

	Error += test_iround();
	Error += test_uround();

	return Error;
}
