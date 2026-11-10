#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/common.hpp>
#include <rblxmath/gtc/integer.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/vector_relational.hpp>
#include <rblxmath/common.hpp>

namespace fmod_
{
	template<typename genType>
	RBLXMATH_FUNC_QUALIFIER genType modTrunc(genType a, genType b)
	{
		return a - b * rblxmath::trunc(a / b);
	}

	static int test()
	{
		int Error(0);

		{
			float A0(3.0);
			float B0(2.0f);
			float C0 = rblxmath::fmod(A0, B0);

			Error += rblxmath::abs(C0 - 1.0f) < 0.00001f ? 0 : 1;

			rblxmath::vec4 A1(3.0);
			float B1(2.0f);
			rblxmath::vec4 C1 = rblxmath::fmod(A1, B1);

			Error += rblxmath::all(rblxmath::epsilonEqual(C1, rblxmath::vec4(1.0f), 0.00001f)) ? 0 : 1;

			rblxmath::vec4 A2(3.0);
			rblxmath::vec4 B2(2.0f);
			rblxmath::vec4 C2 = rblxmath::fmod(A2, B2);

			Error += rblxmath::all(rblxmath::epsilonEqual(C2, rblxmath::vec4(1.0f), 0.00001f)) ? 0 : 1;

			rblxmath::ivec4 A3(3);
			int B3(2);
			rblxmath::ivec4 C3 = rblxmath::fmod(A3, B3);

			Error += rblxmath::all(rblxmath::equal(C3, rblxmath::ivec4(1))) ? 0 : 1;

			rblxmath::ivec4 A4(3);
			rblxmath::ivec4 B4(2);
			rblxmath::ivec4 C4 = rblxmath::fmod(A4, B4);

			Error += rblxmath::all(rblxmath::equal(C4, rblxmath::ivec4(1))) ? 0 : 1;
		}

		{
			float A0(22.0);
			float B0(-10.0f);
			float C0 = rblxmath::fmod(A0, B0);

			Error += rblxmath::abs(C0 - 2.0f) < 0.00001f ? 0 : 1;

			rblxmath::vec4 A1(22.0);
			float B1(-10.0f);
			rblxmath::vec4 C1 = rblxmath::fmod(A1, B1);

			Error += rblxmath::all(rblxmath::epsilonEqual(C1, rblxmath::vec4(2.0f), 0.00001f)) ? 0 : 1;

			rblxmath::vec4 A2(22.0);
			rblxmath::vec4 B2(-10.0f);
			rblxmath::vec4 C2 = rblxmath::fmod(A2, B2);

			Error += rblxmath::all(rblxmath::epsilonEqual(C2, rblxmath::vec4(2.0f), 0.00001f)) ? 0 : 1;

			rblxmath::ivec4 A3(22);
			int B3(-10);
			rblxmath::ivec4 C3 = rblxmath::fmod(A3, B3);

			Error += rblxmath::all(rblxmath::equal(C3, rblxmath::ivec4(2))) ? 0 : 1;

			rblxmath::ivec4 A4(22);
			rblxmath::ivec4 B4(-10);
			rblxmath::ivec4 C4 = rblxmath::fmod(A4, B4);

			Error += rblxmath::all(rblxmath::equal(C4, rblxmath::ivec4(2))) ? 0 : 1;
		}

		// http://stackoverflow.com/questions/7610631/glsl-mod-vs-hlsl-fmod
		{
			for (float y = -10.0f; y < 10.0f; y += 0.1f)
			for (float x = -10.0f; x < 10.0f; x += 0.1f)
			{
				float const A(std::fmod(x, y));
				//float const B(std::remainder(x, y));
				float const C(rblxmath::fmod(x, y));
				float const D(modTrunc(x, y));

				//Error += rblxmath::epsilonEqual(A, B, 0.0001f) ? 0 : 1;
				//assert(!Error);
				Error += rblxmath::epsilonEqual(A, C, 0.0001f) ? 0 : 1;
				assert(!Error);
				Error += rblxmath::epsilonEqual(A, D, 0.00001f) ? 0 : 1;
				assert(!Error);
			}
		}

		return Error;
	}
}//namespace fmod_

static int test_isdenormal()
{
	int Error = 0;

	bool A = rblxmath::isdenormal(1.0f);
	Error += !A ? 0 : 1;

	rblxmath::bvec1 B = rblxmath::isdenormal(rblxmath::vec1(1.0f));
	Error += !rblxmath::any(B) ? 0 : 1;

	rblxmath::bvec2 C = rblxmath::isdenormal(rblxmath::vec2(1.0f));
	Error += !rblxmath::any(C) ? 0 : 1;

	rblxmath::bvec3 D = rblxmath::isdenormal(rblxmath::vec3(1.0f));
	Error += !rblxmath::any(D) ? 0 : 1;

	rblxmath::bvec4 E = rblxmath::isdenormal(rblxmath::vec4(1.0f));
	Error += !rblxmath::any(E) ? 0 : 1;

	return Error;
}

static int test_openBounded()
{
	int Error = 0;

	Error += rblxmath::all(rblxmath::openBounded(rblxmath::ivec2(2), rblxmath::ivec2(1), rblxmath::ivec2(3))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::openBounded(rblxmath::ivec2(1), rblxmath::ivec2(1), rblxmath::ivec2(3))) ? 0 : 1;
	Error += !rblxmath::all(rblxmath::openBounded(rblxmath::ivec2(3), rblxmath::ivec2(1), rblxmath::ivec2(3))) ? 0 : 1;

	return Error;
}

static int test_closeBounded()
{
	int Error = 0;

	Error += rblxmath::all(rblxmath::closeBounded(rblxmath::ivec2(2), rblxmath::ivec2(1), rblxmath::ivec2(3))) ? 0 : 1;
	Error += rblxmath::all(rblxmath::closeBounded(rblxmath::ivec2(1), rblxmath::ivec2(1), rblxmath::ivec2(3))) ? 0 : 1;
	Error += rblxmath::all(rblxmath::closeBounded(rblxmath::ivec2(3), rblxmath::ivec2(1), rblxmath::ivec2(3))) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_isdenormal();
	Error += ::fmod_::test();
	Error += test_openBounded();
	Error += test_closeBounded();

	return Error;
}
