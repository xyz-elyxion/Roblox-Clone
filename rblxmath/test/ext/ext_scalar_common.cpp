#include <rblxmath/ext/scalar_common.hpp>
#include <rblxmath/ext/scalar_constants.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/common.hpp>

template <typename T>
static int test_min()
{
	int Error = 0;

	T const N = static_cast<T>(0);
	T const B = static_cast<T>(1);
	Error += rblxmath::equal(rblxmath::min(N, B), N, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::min(B, N), N, rblxmath::epsilon<T>()) ? 0 : 1;

	T const C = static_cast<T>(2);
	Error += rblxmath::equal(rblxmath::min(N, B, C), N, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::min(B, N, C), N, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::min(C, N, B), N, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::min(C, B, N), N, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::min(B, C, N), N, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::min(N, C, B), N, rblxmath::epsilon<T>()) ? 0 : 1;

	T const D = static_cast<T>(3);
	Error += rblxmath::equal(rblxmath::min(D, N, B, C), N, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::min(B, D, N, C), N, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::min(C, N, D, B), N, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::min(C, B, D, N), N, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::min(B, C, N, D), N, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::min(N, C, B, D), N, rblxmath::epsilon<T>()) ? 0 : 1;

	return Error;
}

template <typename T>
static int test_max()
{
	int Error = 0;

	T const N = static_cast<T>(0);
	T const B = static_cast<T>(1);
	Error += rblxmath::equal(rblxmath::max(N, B), B, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::max(B, N), B, rblxmath::epsilon<T>()) ? 0 : 1;

	T const C = static_cast<T>(2);
	Error += rblxmath::equal(rblxmath::max(N, B, C), C, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::max(B, N, C), C, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::max(C, N, B), C, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::max(C, B, N), C, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::max(B, C, N), C, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::max(N, C, B), C, rblxmath::epsilon<T>()) ? 0 : 1;

	T const D = static_cast<T>(3);
	Error += rblxmath::equal(rblxmath::max(D, N, B, C), D, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::max(B, D, N, C), D, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::max(C, N, D, B), D, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::max(C, B, D, N), D, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::max(B, C, N, D), D, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::max(N, C, B, D), D, rblxmath::epsilon<T>()) ? 0 : 1;

	return Error;
}

#if ((RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG) || (RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC))
template <typename T>
static int test_min_nan()
{
	int Error = 0;

	T const B = static_cast<T>(1);
	T const N = static_cast<T>(NAN);
	Error += rblxmath::isnan(rblxmath::min(N, B)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::min(B, N)) ? 0 : 1;

	T const C = static_cast<T>(2);
	Error += rblxmath::isnan(rblxmath::min(N, B, C)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::min(B, N, C)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::min(C, N, B)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::min(C, B, N)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::min(B, C, N)) ? 0 : 1;
	Error += rblxmath::isnan(rblxmath::min(N, C, B)) ? 0 : 1;

	T const D = static_cast<T>(3);
	Error += !rblxmath::isnan(rblxmath::min(D, N, B, C)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::min(B, D, N, C)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::min(C, N, D, B)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::min(C, B, D, N)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::min(B, C, N, D)) ? 0 : 1;
	Error += rblxmath::isnan(rblxmath::min(N, C, B, D)) ? 0 : 1;

	return Error;
}

template <typename T>
static int test_max_nan()
{
	int Error = 0;

	T const B = static_cast<T>(1);
	T const N = static_cast<T>(NAN);
	Error += rblxmath::isnan(rblxmath::max(N, B)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::max(B, N)) ? 0 : 1;

	T const C = static_cast<T>(2);
	Error += rblxmath::isnan(rblxmath::max(N, B, C)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::max(B, N, C)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::max(C, N, B)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::max(C, B, N)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::max(B, C, N)) ? 0 : 1;
	Error += rblxmath::isnan(rblxmath::max(N, C, B)) ? 0 : 1;

	T const D = static_cast<T>(3);
	Error += !rblxmath::isnan(rblxmath::max(D, N, B, C)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::max(B, D, N, C)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::max(C, N, D, B)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::max(C, B, D, N)) ? 0 : 1;
	Error += !rblxmath::isnan(rblxmath::max(B, C, N, D)) ? 0 : 1;
	Error += rblxmath::isnan(rblxmath::max(N, C, B, D)) ? 0 : 1;

	return Error;
}

template <typename T>
static int test_fmin()
{
	int Error = 0;

	T const B = static_cast<T>(1);
	T const N = static_cast<T>(NAN);
	Error += rblxmath::equal(rblxmath::fmin(N, B), B, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmin(B, N), B, rblxmath::epsilon<T>()) ? 0 : 1;

	T const C = static_cast<T>(2);
	Error += rblxmath::equal(rblxmath::fmin(N, B, C), B, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmin(B, N, C), B, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmin(C, N, B), B, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmin(C, B, N), B, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmin(B, C, N), B, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmin(N, C, B), B, rblxmath::epsilon<T>()) ? 0 : 1;

	T const D = static_cast<T>(3);
	Error += rblxmath::equal(rblxmath::fmin(D, N, B, C), B, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmin(B, D, N, C), B, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmin(C, N, D, B), B, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmin(C, B, D, N), B, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmin(B, C, N, D), B, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmin(N, C, B, D), B, rblxmath::epsilon<T>()) ? 0 : 1;

	return Error;
}

template <typename T>
static int test_fmax()
{
	int Error = 0;

	T const B = static_cast<T>(1);
	T const N = static_cast<T>(NAN);
	Error += rblxmath::equal(rblxmath::fmax(N, B), B, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmax(B, N), B, rblxmath::epsilon<T>()) ? 0 : 1;

	T const C = static_cast<T>(2);
	Error += rblxmath::equal(rblxmath::fmax(N, B, C), C, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmax(B, N, C), C, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmax(C, N, B), C, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmax(C, B, N), C, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmax(B, C, N), C, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmax(N, C, B), C, rblxmath::epsilon<T>()) ? 0 : 1;

	T const D = static_cast<T>(3);
	Error += rblxmath::equal(rblxmath::fmax(D, N, B, C), D, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmax(B, D, N, C), D, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmax(C, N, D, B), D, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmax(C, B, D, N), D, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmax(B, C, N, D), D, rblxmath::epsilon<T>()) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::fmax(N, C, B, D), D, rblxmath::epsilon<T>()) ? 0 : 1;

	return Error;
}
#endif//((RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG) || (RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC))

static int test_clamp()
{
	int Error = 0;

	float A = rblxmath::clamp(0.5f);
	Error += rblxmath::equal(A, 0.5f, 0.00001f) ? 0 : 1;

	float B = rblxmath::clamp(0.0f);
	Error += rblxmath::equal(B, 0.0f, 0.00001f) ? 0 : 1;

	float C = rblxmath::clamp(1.0f);
	Error += rblxmath::equal(C, 1.0f, 0.00001f) ? 0 : 1;

	float D = rblxmath::clamp(-0.5f);
	Error += rblxmath::equal(D, 0.0f, 0.00001f) ? 0 : 1;

	float E = rblxmath::clamp(1.5f);
	Error += rblxmath::equal(E, 1.0f, 0.00001f) ? 0 : 1;

	return Error;
}

static int test_repeat()
{
	int Error = 0;

	float A = rblxmath::repeat(0.5f);
	Error += rblxmath::equal(A, 0.5f, 0.00001f) ? 0 : 1;

	float B = rblxmath::repeat(0.0f);
	Error += rblxmath::equal(B, 0.0f, 0.00001f) ? 0 : 1;

	float C = rblxmath::repeat(1.0f);
	Error += rblxmath::equal(C, 0.0f, 0.00001f) ? 0 : 1;

	float D = rblxmath::repeat(-0.5f);
	Error += rblxmath::equal(D, 0.5f, 0.00001f) ? 0 : 1;

	float E = rblxmath::repeat(1.5f);
	Error += rblxmath::equal(E, 0.5f, 0.00001f) ? 0 : 1;

	float F = rblxmath::repeat(0.9f);
	Error += rblxmath::equal(F, 0.9f, 0.00001f) ? 0 : 1;

	return Error;
}

static int test_mirrorClamp()
{
	int Error = 0;

	float A = rblxmath::mirrorClamp(0.5f);
	Error += rblxmath::equal(A, 0.5f, 0.00001f) ? 0 : 1;

	float B = rblxmath::mirrorClamp(0.0f);
	Error += rblxmath::equal(B, 0.0f, 0.00001f) ? 0 : 1;

	float C = rblxmath::mirrorClamp(1.1f);
	Error += rblxmath::equal(C, 0.1f, 0.00001f) ? 0 : 1;

	float D = rblxmath::mirrorClamp(-0.5f);
	Error += rblxmath::equal(D, 0.5f, 0.00001f) ? 0 : 1;

	float E = rblxmath::mirrorClamp(1.5f);
	Error += rblxmath::equal(E, 0.5f, 0.00001f) ? 0 : 1;

	float F = rblxmath::mirrorClamp(0.9f);
	Error += rblxmath::equal(F, 0.9f, 0.00001f) ? 0 : 1;

	float G = rblxmath::mirrorClamp(3.1f);
	Error += rblxmath::equal(G, 0.1f, 0.00001f) ? 0 : 1;

	float H = rblxmath::mirrorClamp(-3.1f);
	Error += rblxmath::equal(H, 0.1f, 0.00001f) ? 0 : 1;

	float I = rblxmath::mirrorClamp(-0.9f);
	Error += rblxmath::equal(I, 0.9f, 0.00001f) ? 0 : 1;

	return Error;
}

static int test_mirrorRepeat()
{
	int Error = 0;

	float A = rblxmath::mirrorRepeat(0.5f);
	Error += rblxmath::equal(A, 0.5f, 0.00001f) ? 0 : 1;

	float B = rblxmath::mirrorRepeat(0.0f);
	Error += rblxmath::equal(B, 0.0f, 0.00001f) ? 0 : 1;

	float C = rblxmath::mirrorRepeat(1.0f);
	Error += rblxmath::equal(C, 1.0f, 0.00001f) ? 0 : 1;

	float D = rblxmath::mirrorRepeat(-0.5f);
	Error += rblxmath::equal(D, 0.5f, 0.00001f) ? 0 : 1;

	float E = rblxmath::mirrorRepeat(1.5f);
	Error += rblxmath::equal(E, 0.5f, 0.00001f) ? 0 : 1;

	float F = rblxmath::mirrorRepeat(0.9f);
	Error += rblxmath::equal(F, 0.9f, 0.00001f) ? 0 : 1;

	float G = rblxmath::mirrorRepeat(3.0f);
	Error += rblxmath::equal(G, 1.0f, 0.00001f) ? 0 : 1;

	float H = rblxmath::mirrorRepeat(-3.0f);
	Error += rblxmath::equal(H, 1.0f, 0.00001f) ? 0 : 1;

	float I = rblxmath::mirrorRepeat(-1.0f);
	Error += rblxmath::equal(I, 1.0f, 0.00001f) ? 0 : 1;

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

	Error += test_min<float>();
	Error += test_min<double>();


	Error += test_max<float>();
	Error += test_max<double>();

#if ((RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG) || (RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC))
	Error += test_min_nan<float>();
	Error += test_min_nan<double>();

	Error += test_max_nan<float>();
	Error += test_max_nan<double>();

	Error += test_fmin<float>();
	Error += test_fmin<double>();

	Error += test_fmax<float>();
	Error += test_fmax<double>();
#endif//

	Error += test_clamp();
	Error += test_repeat();
	Error += test_mirrorClamp();
	Error += test_mirrorRepeat();

	Error += test_iround();
	Error += test_uround();

	return Error;
}
