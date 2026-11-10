#define RBLXMATH_FORCE_SWIZZLE
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/rblxmath.hpp>

static int test_ivec2_swizzle()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR || RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION
	{
		rblxmath::ivec2 A(1, 2);
		rblxmath::ivec2 B = A.yx();
		rblxmath::ivec2 C = B.yx();

		Error += A != B ? 0 : 1;
		Error += A == C ? 0 : 1;
	}
#	endif//RBLXMATH_CONFIG_SWIZZLE

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
	{
		rblxmath::ivec2 A(1, 2);
		rblxmath::ivec2 B = A.yx;
		rblxmath::ivec2 C = A.yx;

		Error += A != B ? 0 : 1;
		Error += B == C ? 0 : 1;

		B.xy = B.yx;
		C.xy = C.yx;

		Error += B == C ? 0 : 1;

		rblxmath::ivec2 D(0, 0);
		D.yx = A.xy;
		Error += A.yx() == D ? 0 : 1;

		rblxmath::ivec2 E = A.yx;
		Error += E == D ? 0 : 1;
	}
#	endif//RBLXMATH_CONFIG_SWIZZLE

	return Error;
}

static int test_ivec3_swizzle()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR || RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION
	{
		rblxmath::ivec3 A(1, 2, 3);
		rblxmath::ivec3 B = A.zyx();
		rblxmath::ivec3 C = B.zyx();

		Error += A != B ? 0 : 1;
		Error += A == C ? 0 : 1;
	}
#	endif

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
	{
		rblxmath::ivec3 const A(1, 2, 3);
		rblxmath::ivec2 B = A.yx;
		rblxmath::ivec2 C = A.yx;

		Error += A.yx() == B ? 0 : 1;
		Error += B == C ? 0 : 1;

		B.xy = B.yx;
		C.xy = C.yx;

		Error += B == C ? 0 : 1;

		rblxmath::ivec2 D(0, 0);
		D.yx = A.xy;

		Error += A.yx() == D ? 0 : 1;

		rblxmath::ivec2 E(0, 0);
		E.xy = A.xy();

		Error += E == A.xy() ? 0 : 1;
		Error += E.xy() == A.xy() ? 0 : 1;

		rblxmath::ivec3 const F = A.xxx + A.xxx;
		Error += F == rblxmath::ivec3(2) ? 0 : 1;

		rblxmath::ivec3 const G = A.xxx - A.xxx;
		Error += G == rblxmath::ivec3(0) ? 0 : 1;

		rblxmath::ivec3 const H = A.xxx * A.xxx;
		Error += H == rblxmath::ivec3(1) ? 0 : 1;

		rblxmath::ivec3 const I = A.xxx / A.xxx;
		Error += I == rblxmath::ivec3(1) ? 0 : 1;

		rblxmath::ivec3 J(1, 2, 3);
		J.xyz += rblxmath::ivec3(1);
		Error += J == rblxmath::ivec3(2, 3, 4) ? 0 : 1;

		rblxmath::ivec3 K(1, 2, 3);
		K.xyz += A.xyz;
		Error += K == rblxmath::ivec3(2, 4, 6) ? 0 : 1;
	}
#	endif

	return Error;
}

static int test_ivec4_swizzle()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR || RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION
	{
		rblxmath::ivec4 A(1, 2, 3, 4);
		rblxmath::ivec4 B = A.wzyx();
		rblxmath::ivec4 C = B.wzyx();

		Error += A != B ? 0 : 1;
		Error += A == C ? 0 : 1;
	}
#	endif

	return Error;
}

static int test_vec4_swizzle()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR || RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION
	{
		rblxmath::vec4 A(1, 2, 3, 4);
		rblxmath::vec4 B = A.wzyx();
		rblxmath::vec4 C = B.wzyx();

		Error += rblxmath::any(rblxmath::notEqual(A, B, 0.0001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, C, 0.0001f)) ? 0 : 1;

		float D = rblxmath::dot(C.wzyx(), C.xyzw());
		Error += rblxmath::equal(D, 20.f, 0.001f) ? 0 : 1;
	}
#	endif

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_ivec2_swizzle();
	Error += test_ivec3_swizzle();
	Error += test_ivec4_swizzle();
	Error += test_vec4_swizzle();

	return Error;
}



