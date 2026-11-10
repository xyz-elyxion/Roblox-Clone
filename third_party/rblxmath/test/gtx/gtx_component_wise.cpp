#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/component_wise.hpp>
#include <rblxmath/gtc/type_precision.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/scalar_constants.hpp>
#include <limits>

namespace compNormalize
{
	static int run()
	{
		int Error(0);

		{
			rblxmath::vec4 const A = rblxmath::compNormalize<float>(rblxmath::u8vec4(0, 127, 128, 255));

			Error += rblxmath::epsilonEqual(A.x, 0.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			Error += A.y < 0.5f ? 0 : 1;
			Error += A.z > 0.5f ? 0 : 1;
			Error += rblxmath::epsilonEqual(A.w, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
		}

		{
			rblxmath::vec4 const A = rblxmath::compNormalize<float>(rblxmath::i8vec4(-128, -1, 0, 127));

			Error += rblxmath::epsilonEqual(A.x,-1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			Error += A.y < 0.0f ? 0 : 1;
			Error += A.z > 0.0f ? 0 : 1;
			Error += rblxmath::epsilonEqual(A.w, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
		}

		{
			rblxmath::vec4 const A = rblxmath::compNormalize<float>(rblxmath::u16vec4(
				std::numeric_limits<rblxmath::u16>::min(),
				(std::numeric_limits<rblxmath::u16>::max() >> 1) + 0,
				(std::numeric_limits<rblxmath::u16>::max() >> 1) + 1,
				std::numeric_limits<rblxmath::u16>::max()));

			Error += rblxmath::epsilonEqual(A.x, 0.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			Error += A.y < 0.5f ? 0 : 1;
			Error += A.z > 0.5f ? 0 : 1;
			Error += rblxmath::epsilonEqual(A.w, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
		}

		{
			rblxmath::vec4 const A = rblxmath::compNormalize<float>(rblxmath::i16vec4(
				std::numeric_limits<rblxmath::i16>::min(),
				static_cast<rblxmath::i16>(-1),
				static_cast<rblxmath::i16>(0),
				std::numeric_limits<rblxmath::i16>::max()));

			Error += rblxmath::epsilonEqual(A.x,-1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			Error += A.y < 0.0f ? 0 : 1;
			Error += A.z > 0.0f ? 0 : 1;
			Error += rblxmath::epsilonEqual(A.w, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
		}

		return Error;
	}
}//namespace compNormalize

namespace compScale
{
	static int run()
	{
		int Error(0);

		{
			rblxmath::u8vec4 const A = rblxmath::compScale<rblxmath::u8>(rblxmath::vec4(0.0f, 0.2f, 0.5f, 1.0f));

			Error += A.x == std::numeric_limits<rblxmath::u8>::min() ? 0 : 1;
			Error += A.y < (std::numeric_limits<rblxmath::u8>::max() >> 2) ? 0 : 1;
			Error += A.z == 127 ? 0 : 1;
			Error += A.w == 255 ? 0 : 1;
		}

		{
			rblxmath::i8vec4 const A = rblxmath::compScale<rblxmath::i8>(rblxmath::vec4(0.0f,-1.0f, 0.5f, 1.0f));

			Error += A.x == 0 ? 0 : 1;
			Error += A.y == -128 ? 0 : 1;
			Error += A.z == 63 ? 0 : 1;
			Error += A.w == 127 ? 0 : 1;
		}

		{
			rblxmath::u16vec4 const A = rblxmath::compScale<rblxmath::u16>(rblxmath::vec4(0.0f, 0.2f, 0.5f, 1.0f));

			Error += A.x == std::numeric_limits<rblxmath::u16>::min() ? 0 : 1;
			Error += A.y < (std::numeric_limits<rblxmath::u16>::max() >> 2) ? 0 : 1;
			Error += A.z == 32767 ? 0 : 1;
			Error += A.w == 65535 ? 0 : 1;
		}

		{
			rblxmath::i16vec4 const A = rblxmath::compScale<rblxmath::i16>(rblxmath::vec4(0.0f,-1.0f, 0.5f, 1.0f));

			Error += A.x == 0 ? 0 : 1;
			Error += A.y == -32768 ? 0 : 1;
			Error += A.z == 16383 ? 0 : 1;
			Error += A.w == 32767 ? 0 : 1;
		}

		return Error;
	}
}// compScale

#if ((RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG) || (RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC))
namespace fcompMax
{
	static int run()
	{
		int Error(0);

		{
            float const A = rblxmath::fcompMax(rblxmath::vec4(NAN, 0.2f, 0.5f, 1.0f));

			Error += rblxmath::equal(A, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
		}

        {
            float const A = rblxmath::fcompMax(rblxmath::vec4(2.0f, NAN, 0.3f, 0.7f));

			Error += rblxmath::equal(A, 2.0f, rblxmath::epsilon<float>()) ? 0 : 1;
		}

        {
            float const A = rblxmath::fcompMax(rblxmath::vec4(NAN, NAN, NAN, NAN));

			Error += std::isnan(A) ? 0 : 1;
		}

		return Error;
	}
}// fcompMax

namespace fcompMin
{
	static int run()
	{
		int Error(0);

		{
            float const A = rblxmath::fcompMin(rblxmath::vec4(NAN, 0.2f, 0.5f, 1.0f));

			Error += rblxmath::equal(A, 0.2f, rblxmath::epsilon<float>()) ? 0 : 1;
		}

        {
            float const A = rblxmath::fcompMin(rblxmath::vec4(2.0f, NAN, 0.3f, 0.7f));

			Error += rblxmath::equal(A, 0.3f, rblxmath::epsilon<float>()) ? 0 : 1;
		}

        {
            float const A = rblxmath::fcompMin(rblxmath::vec4(NAN, NAN, NAN, NAN));

			Error += std::isnan(A) ? 0 : 1;
		}


		return Error;
	}
}// fcompMin
#endif//((RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG) || (RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC))

int main()
{
	int Error(0);

	Error += compNormalize::run();
	Error += compScale::run();

#if ((RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG) || (RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC))
	Error += fcompMax::run();
	Error += fcompMin::run();
#endif//((RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG) || (RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC))

	return Error;
}
