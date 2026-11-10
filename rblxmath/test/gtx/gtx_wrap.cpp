#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/wrap.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>

namespace clamp
{
	static int test()
	{
		int Error(0);

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
}//namespace clamp

namespace repeat
{
	static int test()
	{
		int Error(0);

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
}//namespace repeat

namespace mirrorClamp
{
	static int test()
	{
		int Error(0);

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
}//namespace mirrorClamp

namespace mirrorRepeat
{
	static int test()
	{
		int Error(0);

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
}//namespace mirrorRepeat

int main()
{
	int Error(0);

	Error += clamp::test();
	Error += repeat::test();
	Error += mirrorClamp::test();
	Error += mirrorRepeat::test();

	return Error;
}
