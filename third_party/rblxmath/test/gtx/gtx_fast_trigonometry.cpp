#include <rblxmath/ext/scalar_ulp.hpp>

#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtc/type_precision.hpp>
#include <rblxmath/gtx/fast_trigonometry.hpp>
#include <rblxmath/gtx/integer.hpp>
#include <rblxmath/gtx/common.hpp>
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/gtc/vec1.hpp>
#include <rblxmath/trigonometric.hpp>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <vector>

namespace fastCos
{
	static int perf(bool NextFloat)
	{
		const float begin = -rblxmath::pi<float>();
		const float end = rblxmath::pi<float>();
		float result = 0.f;

		const std::clock_t timestamp1 = std::clock();
		for(float i = begin; i < end; i = NextFloat ? rblxmath::nextFloat(i) : i += 0.1f)
			result = rblxmath::fastCos(i);

		const std::clock_t timestamp2 = std::clock();
		for(float i = begin; i < end; i = NextFloat ? rblxmath::nextFloat(i) : i += 0.1f)
			result = rblxmath::cos(i);

		const std::clock_t timestamp3 = std::clock();
		const std::clock_t time_fast = timestamp2 - timestamp1;
		const std::clock_t time_default = timestamp3 - timestamp2;
		std::printf("fastCos Time %d clocks\n", static_cast<int>(time_fast));
		std::printf("cos Time %d clocks\n", static_cast<int>(time_default));

		(void) result; // Silence set but not used warning

		return time_fast <= time_default ? 0 : 1;
	}
}//namespace fastCos

namespace fastSin
{
	/*
	float sin(float x) {
	float temp;
	temp = (x + M_PI) / ((2 * M_PI) - M_PI);
	return limited_sin((x + M_PI) - ((2 * M_PI) - M_PI) * temp));
	}
	*/

	static int perf(bool NextFloat)
	{
		const float begin = -rblxmath::pi<float>();
		const float end = rblxmath::pi<float>();
		float result = 0.f;

		const std::clock_t timestamp1 = std::clock();
		for(float i = begin; i < end; i = NextFloat ? rblxmath::nextFloat(i) : i += 0.1f)
			result = rblxmath::fastSin(i);

		const std::clock_t timestamp2 = std::clock();
		for(float i = begin; i < end; i = NextFloat ? rblxmath::nextFloat(i) : i += 0.1f)
			result = rblxmath::sin(i);

		const std::clock_t timestamp3 = std::clock();
		const std::clock_t time_fast = timestamp2 - timestamp1;
		const std::clock_t time_default = timestamp3 - timestamp2;
		std::printf("fastSin Time %d clocks\n", static_cast<int>(time_fast));
		std::printf("sin Time %d clocks\n", static_cast<int>(time_default));

		(void) result; // Silence set but not used warning

		return time_fast <= time_default ? 0 : 1;
	}
}//namespace fastSin

namespace fastTan
{
	static int perf(bool NextFloat)
	{
		const float begin = -rblxmath::pi<float>();
		const float end = rblxmath::pi<float>();
		float result = 0.f;

		const std::clock_t timestamp1 = std::clock();
		for(float i = begin; i < end; i = NextFloat ? rblxmath::nextFloat(i) : i += 0.1f)
			result = rblxmath::fastTan(i);

		const std::clock_t timestamp2 = std::clock();
		for (float i = begin; i < end; i = NextFloat ? rblxmath::nextFloat(i) : i += 0.1f)
			result = rblxmath::tan(i);

		const std::clock_t timestamp3 = std::clock();
		const std::clock_t time_fast = timestamp2 - timestamp1;
		const std::clock_t time_default = timestamp3 - timestamp2;
		std::printf("fastTan Time %d clocks\n", static_cast<int>(time_fast));
		std::printf("tan Time %d clocks\n", static_cast<int>(time_default));

		(void) result; // Silence set but not used warning

		return time_fast <= time_default ? 0 : 1;
	}
}//namespace fastTan

namespace fastAcos
{
	static int perf(bool NextFloat)
	{
		const float begin = -rblxmath::pi<float>();
		const float end = rblxmath::pi<float>();
		float result = 0.f;

		const std::clock_t timestamp1 = std::clock();
		for(float i = begin; i < end; i = NextFloat ? rblxmath::nextFloat(i) : i += 0.1f)
			result = rblxmath::fastAcos(i);

		const std::clock_t timestamp2 = std::clock();
		for(float i = begin; i < end; i = NextFloat ? rblxmath::nextFloat(i) : i += 0.1f)
			result = rblxmath::acos(i);

		const std::clock_t timestamp3 = std::clock();
		const std::clock_t time_fast = timestamp2 - timestamp1;
		const std::clock_t time_default = timestamp3 - timestamp2;

		std::printf("fastAcos Time %d clocks\n", static_cast<int>(time_fast));
		std::printf("acos Time %d clocks\n", static_cast<int>(time_default));

		(void) result; // Silence set but not used warning

		return time_fast <= time_default ? 0 : 1;
	}
}//namespace fastAcos

namespace fastAsin
{
	static int perf(bool NextFloat)
	{
		const float begin = -rblxmath::pi<float>();
		const float end = rblxmath::pi<float>();
		float result = 0.f;
		const std::clock_t timestamp1 = std::clock();
		for(float i = begin; i < end; i = NextFloat ? rblxmath::nextFloat(i) : i += 0.1f)
			result = rblxmath::fastAsin(i);
		const std::clock_t timestamp2 = std::clock();
		for(float i = begin; i < end; i = NextFloat ? rblxmath::nextFloat(i) : i += 0.1f)
			result = rblxmath::asin(i);
		const std::clock_t timestamp3 = std::clock();
		const std::clock_t time_fast = timestamp2 - timestamp1;
		const std::clock_t time_default = timestamp3 - timestamp2;
		std::printf("fastAsin Time %d clocks\n", static_cast<int>(time_fast));
		std::printf("asin Time %d clocks\n", static_cast<int>(time_default));

		(void) result; // Silence set but not used warning

		return time_fast <= time_default ? 0 : 1;
	}
}//namespace fastAsin

namespace fastAtan
{
	static int perf(bool NextFloat)
	{
		const float begin = -rblxmath::pi<float>();
		const float end = rblxmath::pi<float>();
		float result = 0.f;
		const std::clock_t timestamp1 = std::clock();
		for(float i = begin; i < end; i = NextFloat ? rblxmath::nextFloat(i) : i += 0.1f)
			result = rblxmath::fastAtan(i);
		const std::clock_t timestamp2 = std::clock();
		for(float i = begin; i < end; i = NextFloat ? rblxmath::nextFloat(i) : i += 0.1f)
			result = rblxmath::atan(i);
		const std::clock_t timestamp3 = std::clock();
		const std::clock_t time_fast = timestamp2 - timestamp1;
		const std::clock_t time_default = timestamp3 - timestamp2;
		std::printf("fastAtan Time %d clocks\n", static_cast<int>(time_fast));
		std::printf("atan Time %d clocks\n", static_cast<int>(time_default));

		(void) result; // Silence set but not used warning

		return time_fast <= time_default ? 0 : 1;
	}
}//namespace fastAtan

namespace taylorCos
{
	using rblxmath::qualifier;
	using rblxmath::length_t;

#	if (RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG)
#		pragma clang diagnostic push
#		pragma clang diagnostic ignored "-Wglobal-constructors"
#	endif

	rblxmath::vec4 const AngleShift(0.0f, rblxmath::half_pi<float>(), rblxmath::pi<float>(), rblxmath::three_over_two_pi<float>());

#	if (RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG)
#		pragma clang diagnostic pop
#	endif

	template<length_t L, typename T, qualifier Q>
	static RBLXMATH_FUNC_QUALIFIER rblxmath::vec<L, T, Q> taylorSeriesNewCos(rblxmath::vec<L, T, Q> const& x)
	{
		rblxmath::vec<L, T, Q> const Powed2(x * x);
		rblxmath::vec<L, T, Q> const Powed4(Powed2 * Powed2);
		rblxmath::vec<L, T, Q> const Powed6(Powed4 * Powed2);
		rblxmath::vec<L, T, Q> const Powed8(Powed4 * Powed4);

		return static_cast<T>(1)
			- Powed2 * static_cast<T>(0.5)
			+ Powed4 * static_cast<T>(0.04166666666666666666666666666667)
			- Powed6 * static_cast<T>(0.00138888888888888888888888888889)
			+ Powed8 * static_cast<T>(2.4801587301587301587301587301587e-5);
	}
/*
	template<length_t L, typename T, qualifier Q>
	static RBLXMATH_FUNC_QUALIFIER rblxmath::vec<L, T, Q> taylorSeriesNewCos6(rblxmath::vec<L, T, Q> const& x)
	{
		rblxmath::vec<L, T, Q> const Powed2(x * x);
		rblxmath::vec<L, T, Q> const Powed4(Powed2 * Powed2);
		rblxmath::vec<L, T, Q> const Powed6(Powed4 * Powed2);

		return static_cast<T>(1)
			- Powed2 * static_cast<T>(0.5)
			+ Powed4 * static_cast<T>(0.04166666666666666666666666666667)
			- Powed6 * static_cast<T>(0.00138888888888888888888888888889);
	}
*/
/*
	template<rblxmath::length_t L, qualifier Q>
	static RBLXMATH_FUNC_QUALIFIER rblxmath::vec<L, float, Q> fastAbs(rblxmath::vec<L, float, Q> x)
	{
		int* Pointer = reinterpret_cast<int*>(&x[0]);
		Pointer[0] &= 0x7fffffff;
		Pointer[1] &= 0x7fffffff;
		Pointer[2] &= 0x7fffffff;
		Pointer[3] &= 0x7fffffff;
		return x;
	}

	template<rblxmath::length_t L, typename T, qualifier Q>
	static RBLXMATH_FUNC_QUALIFIER rblxmath::vec<L, T, Q> fastCosNew(rblxmath::vec<L, T, Q> const& x)
	{
		rblxmath::vec<L, T, Q> const Angle0_PI(fastAbs(fmod(x + rblxmath::pi<T>(), rblxmath::two_pi<T>()) - rblxmath::pi<T>()));
		return taylorSeriesNewCos6(x);
//		vec<L, bool, Q> const FirstQuarterPi(lessThanEqual(Angle0_PI, vec<L, T, Q>(rblxmath::half_pi<T>())));

//		vec<L, T, Q> const RevertAngle(mix(vec<L, T, Q>(rblxmath::pi<T>()), vec<L, T, Q>(0), FirstQuarterPi));
//		vec<L, T, Q> const ReturnSign(mix(vec<L, T, Q>(-1), vec<L, T, Q>(1), FirstQuarterPi));
//		vec<L, T, Q> const SectionAngle(RevertAngle - Angle0_PI);

//		return ReturnSign * taylorSeriesNewCos(SectionAngle);
	}
*/
/*
	static int perf_fastCosNew(float Begin, float End, std::size_t Samples)
	{
		std::vector<rblxmath::vec4> Results;
		Results.resize(Samples);

		float const Steps = (End - Begin) / static_cast<float>(Samples);

		std::clock_t const TimeStampBegin = std::clock();

		for(std::size_t i = 0; i < Samples; ++i)
			Results[i] = fastCosNew(AngleShift + rblxmath::vec4(Begin + Steps * static_cast<float>(i)));

		std::clock_t const TimeStampEnd = std::clock();

		std::printf("fastCosNew %d clocks\n", static_cast<int>(TimeStampEnd - TimeStampBegin));

		int Error = 0;
		for(std::size_t i = 0; i < Samples; ++i)
			Error += Results[i].x >= -1.0f && Results[i].x <= 1.0f ? 0 : 1;
		return Error;
	}
*/
	template<length_t L, typename T, qualifier Q>
	static RBLXMATH_FUNC_QUALIFIER rblxmath::vec<L, T, Q> deterministic_fmod(rblxmath::vec<L, T, Q> const& x, T y)
	{
		return x - y * trunc(x / y);
	}

	template<length_t L, typename T, qualifier Q>
	static RBLXMATH_FUNC_QUALIFIER rblxmath::vec<L, T, Q> fastCosDeterminisctic(rblxmath::vec<L, T, Q> const& x)
	{
		rblxmath::vec<L, T, Q> const Angle0_PI(abs(deterministic_fmod(x + rblxmath::pi<T>(), rblxmath::two_pi<T>()) - rblxmath::pi<T>()));
		rblxmath::vec<L, bool, Q> const FirstQuarterPi(lessThanEqual(Angle0_PI, rblxmath::vec<L, T, Q>(rblxmath::half_pi<T>())));

		rblxmath::vec<L, T, Q> const RevertAngle(mix(rblxmath::vec<L, T, Q>(rblxmath::pi<T>()), rblxmath::vec<L, T, Q>(0), FirstQuarterPi));
		rblxmath::vec<L, T, Q> const ReturnSign(mix(rblxmath::vec<L, T, Q>(-1), rblxmath::vec<L, T, Q>(1), FirstQuarterPi));
		rblxmath::vec<L, T, Q> const SectionAngle(RevertAngle - Angle0_PI);

		return ReturnSign * taylorSeriesNewCos(SectionAngle);
	}

	static int perf_fastCosDeterminisctic(float Begin, float End, std::size_t Samples)
	{
		std::vector<rblxmath::vec4> Results;
		Results.resize(Samples);

		float const Steps = (End - Begin) / static_cast<float>(Samples);

		std::clock_t const TimeStampBegin = std::clock();

		for(std::size_t i = 0; i < Samples; ++i)
			Results[i] = taylorCos::fastCosDeterminisctic(AngleShift + rblxmath::vec4(Begin + Steps * static_cast<float>(i)));

		std::clock_t const TimeStampEnd = std::clock();

		std::printf("fastCosDeterminisctic %d clocks\n", static_cast<int>(TimeStampEnd - TimeStampBegin));

		int Error = 0;
		for(std::size_t i = 0; i < Samples; ++i)
			Error += Results[i].x >= -1.0f && Results[i].x <= 1.0f ? 0 : 1;
		return Error;
	}

	template<length_t L, typename T, qualifier Q>
	static RBLXMATH_FUNC_QUALIFIER rblxmath::vec<L, T, Q> taylorSeriesRefCos(rblxmath::vec<L, T, Q> const& x)
	{
		return static_cast<T>(1)
			- (x * x) / rblxmath::factorial(static_cast<T>(2))
			+ (x * x * x * x) / rblxmath::factorial(static_cast<T>(4))
			- (x * x * x * x * x * x) / rblxmath::factorial(static_cast<T>(6))
			+ (x * x * x * x * x * x * x * x) / rblxmath::factorial(static_cast<T>(8));
	}

	template<length_t L, typename T, qualifier Q>
	static RBLXMATH_FUNC_QUALIFIER rblxmath::vec<L, T, Q> fastRefCos(rblxmath::vec<L, T, Q> const& x)
	{
		rblxmath::vec<L, T, Q> const Angle0_PI(rblxmath::abs(fmod(x + rblxmath::pi<T>(), rblxmath::two_pi<T>()) - rblxmath::pi<T>()));
//		return taylorSeriesRefCos(Angle0_PI);

		rblxmath::vec<L, bool, Q> const FirstQuarterPi(lessThanEqual(Angle0_PI, rblxmath::vec<L, T, Q>(rblxmath::half_pi<T>())));

		rblxmath::vec<L, T, Q> const RevertAngle(mix(rblxmath::vec<L, T, Q>(rblxmath::pi<T>()), rblxmath::vec<L, T, Q>(0), FirstQuarterPi));
		rblxmath::vec<L, T, Q> const ReturnSign(mix(rblxmath::vec<L, T, Q>(-1), rblxmath::vec<L, T, Q>(1), FirstQuarterPi));
		rblxmath::vec<L, T, Q> const SectionAngle(RevertAngle - Angle0_PI);

		return ReturnSign * taylorSeriesRefCos(SectionAngle);
	}

	static int perf_fastCosRef(float Begin, float End, std::size_t Samples)
	{
		std::vector<rblxmath::vec4> Results;
		Results.resize(Samples);

		float const Steps = (End - Begin) / static_cast<float>(Samples);

		std::clock_t const TimeStampBegin = std::clock();

		for(std::size_t i = 0; i < Samples; ++i)
			Results[i] = taylorCos::fastRefCos(AngleShift + rblxmath::vec4(Begin + Steps * static_cast<float>(i)));

		std::clock_t const TimeStampEnd = std::clock();

		std::printf("fastCosRef %d clocks\n", static_cast<int>(TimeStampEnd - TimeStampBegin));

		int Error = 0;
		for(std::size_t i = 0; i < Samples; ++i)
			Error += Results[i].x >= -1.0f && Results[i].x <= 1.0f ? 0 : 1;
		return Error;
	}

	static int perf_fastCosOld(float Begin, float End, std::size_t Samples)
	{
		std::vector<rblxmath::vec4> Results;
		Results.resize(Samples);

		float const Steps = (End - Begin) / static_cast<float>(Samples);

		std::clock_t const TimeStampBegin = std::clock();

		for(std::size_t i = 0; i < Samples; ++i)
			Results[i] = rblxmath::fastCos(AngleShift + rblxmath::vec4(Begin + Steps * static_cast<float>(i)));

		std::clock_t const TimeStampEnd = std::clock();

		std::printf("fastCosOld %d clocks\n", static_cast<int>(TimeStampEnd - TimeStampBegin));

		int Error = 0;
		for(std::size_t i = 0; i < Samples; ++i)
			Error += Results[i].x >= -1.0f && Results[i].x <= 1.0f ? 0 : 1;
		return Error;
	}

	static int perf_cos(float Begin, float End, std::size_t Samples)
	{
		std::vector<rblxmath::vec4> Results;
		Results.resize(Samples);

		float const Steps = (End - Begin) / static_cast<float>(Samples);

		std::clock_t const TimeStampBegin = std::clock();

		for(std::size_t i = 0; i < Samples; ++i)
			Results[i] = rblxmath::cos(AngleShift + rblxmath::vec4(Begin + Steps * static_cast<float>(i)));

		std::clock_t const TimeStampEnd = std::clock();

		std::printf("cos %d clocks\n", static_cast<int>(TimeStampEnd - TimeStampBegin));

		int Error = 0;
		for(std::size_t i = 0; i < Samples; ++i)
			Error += Results[i].x >= -1.0f && Results[i].x <= 1.0f ? 0 : 1;
		return Error;
	}

	static int perf(std::size_t const Samples)
	{
		int Error = 0;

		float const Begin = -rblxmath::pi<float>();
		float const End = rblxmath::pi<float>();

		Error += perf_cos(Begin, End, Samples);
		Error += perf_fastCosOld(Begin, End, Samples);
		Error += perf_fastCosRef(Begin, End, Samples);
		//Error += perf_fastCosNew(Begin, End, Samples);
		Error += perf_fastCosDeterminisctic(Begin, End, Samples);

		return Error;
	}

	static int test()
	{
		int Error = 0;

		//for(float Angle = -4.0f * rblxmath::pi<float>(); Angle < 4.0f * rblxmath::pi<float>(); Angle += 0.1f)
		//for(float Angle = -720.0f; Angle < 720.0f; Angle += 0.1f)
		for(float Angle = 0.0f; Angle < 180.0f; Angle += 0.1f)
		{
			float const modAngle = std::fmod(rblxmath::abs(Angle), 360.f);
			assert(modAngle >= 0.0f && modAngle <= 360.f);
			float const radAngle = rblxmath::radians(modAngle);
			float const Cos0 = std::cos(radAngle);

			float const Cos1 = taylorCos::fastRefCos(rblxmath::fvec1(radAngle)).x;
			Error += rblxmath::abs(Cos1 - Cos0) < 0.1f ? 0 : 1;

			//float const Cos2 = taylorCos::fastCosNew(rblxmath::fvec1(radAngle)).x;
			//Error += rblxmath::abs(Cos2 - Cos0) < 0.1f ? 0 : 1;

			assert(!Error);
		}

		return Error;
	}
}//namespace taylorCos

namespace taylor2
{
#	if (RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG)
#		pragma clang diagnostic push
#		pragma clang diagnostic ignored "-Wglobal-constructors"
#	endif

	rblxmath::vec4 const AngleShift(0.0f, rblxmath::pi<float>() * 0.5f, rblxmath::pi<float>() * 1.0f, rblxmath::pi<float>() * 1.5f);

#	if (RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG)
#		pragma clang diagnostic pop
#	endif

	static float taylorCosA(float x)
	{
		return 1.f
			- (x * x) * (1.f / 2.f)
			+ (x * x * x * x) * (1.f / 24.f)
			- (x * x * x * x * x * x) * (1.f / 720.f)
			+ (x * x * x * x * x * x * x * x) * (1.f / 40320.f);
	}

	static float taylorCosB(float x)
	{
		return 1.f
			- (x * x) * (1.f / 2.f)
			+ (x * x * x * x) * (1.f / 24.f)
			- (x * x * x * x * x * x) * (1.f / 720.f)
			+ (x * x * x * x * x * x * x * x) * (1.f / 40320.f);
	}

	static float taylorCosC(float x)
	{
		return 1.f
			- (x * x) * (1.f / 2.f)
			+ ((x * x) * (x * x)) * (1.f / 24.f)
			- (((x * x) * (x * x)) * (x * x)) * (1.f / 720.f)
			+ (((x * x) * (x * x)) * ((x * x) * (x * x))) * (1.f / 40320.f);
	}

	static int perf_taylorCosA(float Begin, float End, std::size_t Samples)
	{
		std::vector<float> Results;
		Results.resize(Samples);

		float const Steps = (End - Begin) / static_cast<float>(Samples);

		std::clock_t const TimeStampBegin = std::clock();

		for(std::size_t i = 0; i < Samples; ++i)
			Results[i] = taylorCosA(AngleShift.x + Begin + Steps * static_cast<float>(i));

		std::clock_t const TimeStampEnd = std::clock();

		std::printf("taylorCosA %d clocks\n", static_cast<int>(TimeStampEnd - TimeStampBegin));

		int Error = 0;
		for(std::size_t i = 0; i < Samples; ++i)
			Error += Results[i] >= -1.0f && Results[i] <= 1.0f ? 0 : 1;
		return Error;
	}

	static int perf_taylorCosB(float Begin, float End, std::size_t Samples)
	{
		std::vector<float> Results;
		Results.resize(Samples);

		float const Steps = (End - Begin) / static_cast<float>(Samples);

		std::clock_t const TimeStampBegin = std::clock();

		for(std::size_t i = 0; i < Samples; ++i)
			Results[i] = taylorCosB(AngleShift.x + Begin + Steps * static_cast<float>(i));

		std::clock_t const TimeStampEnd = std::clock();

		std::printf("taylorCosB %d clocks\n", static_cast<int>(TimeStampEnd - TimeStampBegin));

		int Error = 0;
		for(std::size_t i = 0; i < Samples; ++i)
			Error += Results[i] >= -1.0f && Results[i] <= 1.0f ? 0 : 1;
		return Error;
	}

	static int perf_taylorCosC(float Begin, float End, std::size_t Samples)
	{
		std::vector<float> Results;
		Results.resize(Samples);

		float const Steps = (End - Begin) / static_cast<float>(Samples);

		std::clock_t const TimeStampBegin = std::clock();

		for(std::size_t i = 0; i < Samples; ++i)
			Results[i] = taylorCosC(AngleShift.x + Begin + Steps * static_cast<float>(i));

		std::clock_t const TimeStampEnd = std::clock();

		std::printf("taylorCosC %d clocks\n", static_cast<int>(TimeStampEnd - TimeStampBegin));

		int Error = 0;
		for(std::size_t i = 0; i < Samples; ++i)
			Error += Results[i] >= -1.0f && Results[i] <= 1.0f ? 0 : 1;
		return Error;
	}

	static int perf(std::size_t Samples)
	{
		int Error = 0;

		float const Begin = -rblxmath::pi<float>();
		float const End = rblxmath::pi<float>();

		Error += perf_taylorCosA(Begin, End, Samples);
		Error += perf_taylorCosB(Begin, End, Samples);
		Error += perf_taylorCosC(Begin, End, Samples);

		return Error;
	}

}//namespace taylor2

int main()
{
	int Error(0);

	Error += ::taylor2::perf(1000);
	Error += ::taylorCos::test();
	Error += ::taylorCos::perf(1000);

	::fastCos::perf(false);
	::fastSin::perf(false);
	::fastTan::perf(false);
	::fastAcos::perf(false);
	::fastAsin::perf(false);
	::fastAtan::perf(false);

	return Error;
}
