#define RBLXMATH_ENABLE_EXPERIMENTAL

#include <rblxmath/gtx/extended_min_max.hpp>
#include <rblxmath/gtc/vec1.hpp>
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>

// This file has divisions by zero to test isnan
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#	pragma warning(disable : 4723)
#endif

namespace fmin_
{
	static int test()
	{
		int Error = 0;

		float Zero_f = 0.0f;
		rblxmath::vec1 A0 = rblxmath::fmin(rblxmath::vec1(1), rblxmath::vec1(Zero_f / 0.0f));
		Error += rblxmath::equal(A0.x, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;

		rblxmath::vec1 A1 = rblxmath::fmin(rblxmath::vec1(Zero_f / 0.0f), rblxmath::vec1(1));
		Error += rblxmath::equal(A1.x, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;

		rblxmath::vec2 B0 = rblxmath::fmin(rblxmath::vec2(1), rblxmath::vec2(1));
		rblxmath::vec2 B1 = rblxmath::fmin(rblxmath::vec2(1), 1.0f);
		bool B2 = rblxmath::all(rblxmath::equal(B0, B1, rblxmath::epsilon<float>()));
		Error += B2 ? 0 : 1;

		rblxmath::vec3 C0 = rblxmath::fmin(rblxmath::vec3(1), rblxmath::vec3(1));
		rblxmath::vec3 C1 = rblxmath::fmin(rblxmath::vec3(1), 1.0f);
		bool C2 = rblxmath::all(rblxmath::equal(C0, C1, rblxmath::epsilon<float>()));
		Error += C2 ? 0 : 1;

		rblxmath::vec4 D0 = rblxmath::fmin(rblxmath::vec4(1), rblxmath::vec4(1));
		rblxmath::vec4 D1 = rblxmath::fmin(rblxmath::vec4(1), 1.0f);
		bool D2 = rblxmath::all(rblxmath::equal(D0, D1, rblxmath::epsilon<float>()));
		Error += D2 ? 0 : 1;

		return Error;
	}
}//namespace fmin_

namespace fmax_
{
	static int test()
	{
		int Error = 0;

		float Zero_f = 0.0f;
		rblxmath::vec1 A0 = rblxmath::fmax(rblxmath::vec1(1), rblxmath::vec1(Zero_f / 0.0f));
		Error += rblxmath::equal(A0.x, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;

		rblxmath::vec1 A1 = rblxmath::fmax(rblxmath::vec1(Zero_f / 0.0f), rblxmath::vec1(1));
		Error += rblxmath::equal(A1.x, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;

		rblxmath::vec2 B0 = rblxmath::fmax(rblxmath::vec2(1), rblxmath::vec2(1));
		rblxmath::vec2 B1 = rblxmath::fmax(rblxmath::vec2(1), 1.0f);
		bool B2 = rblxmath::all(rblxmath::equal(B0, B1, rblxmath::epsilon<float>()));
		Error += B2 ? 0 : 1;

		rblxmath::vec3 C0 = rblxmath::fmax(rblxmath::vec3(1), rblxmath::vec3(1));
		rblxmath::vec3 C1 = rblxmath::fmax(rblxmath::vec3(1), 1.0f);
		bool C2 = rblxmath::all(rblxmath::equal(C0, C1, rblxmath::epsilon<float>()));
		Error += C2 ? 0 : 1;

		rblxmath::vec4 D0 = rblxmath::fmax(rblxmath::vec4(1), rblxmath::vec4(1));
		rblxmath::vec4 D1 = rblxmath::fmax(rblxmath::vec4(1), 1.0f);
		bool D2 = rblxmath::all(rblxmath::equal(D0, D1, rblxmath::epsilon<float>()));
		Error += D2 ? 0 : 1;

		return Error;
	}
}//namespace fmax_

namespace fclamp_
{
	static int test()
	{
		int Error = 0;

		float Zero_f = 0.0f;
		rblxmath::vec1 A0 = rblxmath::fclamp(rblxmath::vec1(1), rblxmath::vec1(Zero_f / 0.0f), rblxmath::vec1(2.0f));
		Error += rblxmath::equal(A0.x, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;

		return Error;
	}
}//namespace fclamp_

int main()
{
	int Error = 0;

	Error += fmin_::test();
	Error += fmax_::test();
	Error += fclamp_::test();

	return Error;
}
