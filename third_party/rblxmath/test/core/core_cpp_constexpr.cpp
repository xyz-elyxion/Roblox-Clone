#include <rblxmath/rblxmath.hpp>

#if RBLXMATH_CONFIG_CONSTEXP == RBLXMATH_ENABLE

#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/gtc/quaternion.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/vector_int1.hpp>
#include <rblxmath/ext/vector_bool1.hpp>
#include <rblxmath/ext/vector_bool4.hpp>
#include <rblxmath/ext/vector_float1.hpp>
#include <rblxmath/vector_relational.hpp>

static int test_vec1()
{
	int Error = 0;

	{
		constexpr rblxmath::bvec1 B(true);
		constexpr bool A = rblxmath::all(B);
		static_assert(A, "RBLXMATH: Failed constexpr");
		
		constexpr rblxmath::bvec1 D(true);
		constexpr bool C = rblxmath::any(D);
		static_assert(C, "RBLXMATH: Failed constexpr");
	}
	
	{
		constexpr rblxmath::bvec2 C(true);
		constexpr rblxmath::bvec2 B(true);
		static_assert(rblxmath::any(rblxmath::equal(C, B)), "RBLXMATH: Failed constexpr");
	}
	
	{
		constexpr rblxmath::ivec1 O(rblxmath::ivec1(1));
		static_assert(rblxmath::ivec1(1) == O, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 P(1);
		static_assert(rblxmath::ivec1(1) == P, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec1 L(rblxmath::ivec2(1, 2));
		static_assert(rblxmath::ivec1(1) == L, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 M(rblxmath::ivec3(1, 2, 3));
		static_assert(rblxmath::ivec1(1) == M, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 N(rblxmath::ivec4(1, 2, 3, 4));
		static_assert(rblxmath::ivec1(1) == N, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec1 A(1);
		static_assert(A[0] == 1, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::vec1(1.0f).x > 0.0f, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::vec1::length() == 1, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::bvec1 A1(true);
		constexpr rblxmath::bvec1 A2(true);
		constexpr rblxmath::bvec1 B1(false);
		constexpr rblxmath::bvec1 B2(false);
		static_assert(A1 == A2 && B1 == B2, "RBLXMATH: Failed constexpr");
		static_assert(A1 == A2 || B1 == B2, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec1 A(1);
		constexpr rblxmath::ivec1 B = A + 1;
		constexpr rblxmath::ivec1 C(3);
		static_assert(A + B == C, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 D = +A;
		static_assert(D == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec1 A(3);
		constexpr rblxmath::ivec1 B = A - 1;
		constexpr rblxmath::ivec1 C(1);
		static_assert(A - B == C, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 D = -A;
		static_assert(-D == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec1 A(3);
		constexpr rblxmath::ivec1 B = A * 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(B * C == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec1 A(3);
		constexpr rblxmath::ivec1 B = A / 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(B / C == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec1 A(3);
		constexpr rblxmath::ivec1 B = A % 2;
		constexpr rblxmath::ivec1 C(1);
		static_assert(B == C, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 D(2);
		static_assert(A % D == C, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec1 A(1);
		constexpr rblxmath::ivec1 B = A & 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(A == (A & C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec1 A(1);
		constexpr rblxmath::ivec1 B = A | 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(A == (A | C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec1 A(1);
		constexpr rblxmath::ivec1 B = A ^ 0;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(0);
		static_assert(A == (A ^ C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec1 A(1);
		constexpr rblxmath::ivec1 B = A << 1;
		static_assert(B == rblxmath::ivec1(2), "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(B == (A << C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec1 A(2);
		constexpr rblxmath::ivec1 B = A >> 1;
		static_assert(B == rblxmath::ivec1(1), "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(B == A >> C, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec1 A(~0);
		constexpr rblxmath::ivec1 B = ~A;
		static_assert(A == ~B, "RBLXMATH: Failed constexpr");
	}

	return Error;
}

static int test_vec2()
{
	int Error = 0;

	{
		constexpr rblxmath::bvec2 B(true);
		constexpr bool A = rblxmath::all(B);
		static_assert(A, "RBLXMATH: Failed constexpr");
		
		constexpr rblxmath::bvec2 D(true, false);
		constexpr bool C = rblxmath::any(D);
		static_assert(C, "RBLXMATH: Failed constexpr");
	}
	
	{
		constexpr rblxmath::bvec2 C(true);
		constexpr rblxmath::bvec2 B(true, false);
		static_assert(rblxmath::any(rblxmath::equal(C, B)), "RBLXMATH: Failed constexpr");
	}
	
	{
		constexpr rblxmath::ivec2 O(rblxmath::ivec1(1));
		static_assert(rblxmath::ivec2(1) == O, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec2 A(1);
		static_assert(rblxmath::ivec2(1) == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec2 F(rblxmath::ivec1(1), rblxmath::ivec1(2));
		static_assert(rblxmath::ivec2(1, 2) == F, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec2 G(1, rblxmath::ivec1(2));
		static_assert(rblxmath::ivec2(1, 2) == G, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec2 H(rblxmath::ivec1(1), 2);
		static_assert(rblxmath::ivec2(1, 2) == H, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec2 I(1, 2);
		static_assert(rblxmath::ivec2(1, 2) == I, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec2 L(rblxmath::ivec2(1, 2));
		static_assert(rblxmath::ivec2(1, 2) == L, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec2 M(rblxmath::ivec3(1, 2, 3));
		static_assert(rblxmath::ivec2(1, 2) == M, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec2 N(rblxmath::ivec4(1, 2, 3, 4));
		static_assert(rblxmath::ivec2(1, 2) == N, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec2 A(1);
		static_assert(A[0] == 1, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::vec2(1.0f).x > 0.0f, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::vec2(1.0f, -1.0f).x > 0.0f, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::vec2(1.0f, -1.0f).y < 0.0f, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::vec2::length() == 2, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::bvec2 A1(true);
		constexpr rblxmath::bvec2 A2(true);
		constexpr rblxmath::bvec2 B1(false);
		constexpr rblxmath::bvec2 B2(false);
		static_assert(A1 == A2 && B1 == B2, "RBLXMATH: Failed constexpr");
		static_assert(A1 == A2 || B1 == B2, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec2 A(1);
		constexpr rblxmath::ivec2 B = A + 1;
		constexpr rblxmath::ivec2 C(3);
		static_assert(A + B == C, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec2 D = +A;
		static_assert(D == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec2 A(3);
		constexpr rblxmath::ivec2 B = A - 1;
		constexpr rblxmath::ivec2 C(1);
		static_assert(A - B == C, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec2 D = -A;
		static_assert(-D == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec2 A(3);
		constexpr rblxmath::ivec2 B = A * 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec2 C(1);
		static_assert(B * C == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec2 A(3);
		constexpr rblxmath::ivec2 B = A / 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec2 C(1);
		static_assert(B / C == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec2 A(3);
		constexpr rblxmath::ivec2 B = A % 2;
		constexpr rblxmath::ivec2 C(1);
		static_assert(B == C, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 D(2);
		static_assert(A % D == C, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec2 A(1);
		constexpr rblxmath::ivec2 B = A & 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(A == (A & C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec2 A(1);
		constexpr rblxmath::ivec2 B = A | 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(A == (A | C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec2 A(1);
		constexpr rblxmath::ivec2 B = A ^ 0;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(0);
		static_assert(A == (A ^ C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec2 A(1);
		constexpr rblxmath::ivec2 B = A << 1;
		static_assert(B == rblxmath::ivec2(2), "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(B == (A << C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec2 A(2);
		constexpr rblxmath::ivec2 B = A >> 1;
		static_assert(B == rblxmath::ivec2(1), "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(B == A >> C, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec2 A(~0);
		constexpr rblxmath::ivec2 B = ~A;
		static_assert(A == ~B, "RBLXMATH: Failed constexpr");
	}

	return Error;
}

static int test_vec3()
{
	int Error = 0;

	{
		constexpr rblxmath::bvec3 B(true);
		constexpr bool A = rblxmath::all(B);
		static_assert(A, "RBLXMATH: Failed constexpr");
		
		constexpr rblxmath::bvec3 D(true, false, true);
		constexpr bool C = rblxmath::any(D);
		static_assert(C, "RBLXMATH: Failed constexpr");
	}
	
	{
		constexpr rblxmath::bvec3 C(true);
		constexpr rblxmath::bvec3 B(true, false, true);
		static_assert(rblxmath::any(rblxmath::equal(C, B)), "RBLXMATH: Failed constexpr");
	}
	
	{
		constexpr rblxmath::ivec3 O(rblxmath::ivec1(1));
		static_assert(rblxmath::ivec3(1) == O, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 A(1);
		static_assert(rblxmath::ivec3(1) == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 B(rblxmath::ivec2(1, 2), 3);
		static_assert(rblxmath::ivec3(1, 2, 3) == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 C(1, rblxmath::ivec2(2, 3));
		static_assert(rblxmath::ivec3(1, 2, 3) == C, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 D(rblxmath::ivec1(1), rblxmath::ivec2(2, 3));
		static_assert(rblxmath::ivec3(1, 2, 3) == D, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 E(rblxmath::ivec2(1, 2), rblxmath::ivec1(3));
		static_assert(rblxmath::ivec3(1, 2, 3) == E, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 F(rblxmath::ivec1(1), rblxmath::ivec1(2), rblxmath::ivec1(3));
		static_assert(rblxmath::ivec3(1, 2, 3) == F, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 G(1, rblxmath::ivec1(2), rblxmath::ivec1(3));
		static_assert(rblxmath::ivec3(1, 2, 3) == G, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 H(rblxmath::ivec1(1), 2, rblxmath::ivec1(3));
		static_assert(rblxmath::ivec3(1, 2, 3) == H, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 I(1, 2, rblxmath::ivec1(3));
		static_assert(rblxmath::ivec3(1, 2, 3) == I, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 J(rblxmath::ivec1(1), rblxmath::ivec1(2), 3);
		static_assert(rblxmath::ivec3(1, 2, 3) == J, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 K(1, rblxmath::ivec1(2), 3);
		static_assert(rblxmath::ivec3(1, 2, 3) == K, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 L(rblxmath::ivec1(1), 2, 3);
		static_assert(rblxmath::ivec3(1, 2, 3) == L, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 M(1, 2, 3);
		static_assert(rblxmath::ivec3(1, 2, 3) == M, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 N(rblxmath::ivec4(1, 2, 3, 4));
		static_assert(rblxmath::ivec3(1, 2, 3) == N, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 const A(1);
		static_assert(A[0] == 1, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::vec3(1.0f).x > 0.0f, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::vec3(1.0f, -1.0f, -1.0f).x > 0.0f, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::vec3(1.0f, -1.0f, -1.0f).y < 0.0f, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::vec3::length() == 3, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::bvec3 A1(true);
		constexpr rblxmath::bvec3 A2(true);
		constexpr rblxmath::bvec3 B1(false);
		constexpr rblxmath::bvec3 B2(false);
		static_assert(A1 == A2 && B1 == B2, "RBLXMATH: Failed constexpr");
		static_assert(A1 == A2 || B1 == B2, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 A(1);
		constexpr rblxmath::ivec3 B = A + 1;
		constexpr rblxmath::ivec3 C(3);
		static_assert(A + B == C, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 D = +A;
		static_assert(D == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 A(3);
		constexpr rblxmath::ivec3 B = A - 1;
		constexpr rblxmath::ivec3 C(1);
		static_assert(A - B == C, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 D = -A;
		static_assert(-D == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 A(3);
		constexpr rblxmath::ivec3 B = A * 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 C(1);
		static_assert(B * C == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 A(3);
		constexpr rblxmath::ivec3 B = A / 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec3 C(1);
		static_assert(B / C == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 A(3);
		constexpr rblxmath::ivec3 B = A % 2;
		constexpr rblxmath::ivec3 C(1);
		static_assert(B == C, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 D(2);
		static_assert(A % D == C, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 A(1);
		constexpr rblxmath::ivec3 B = A & 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(A == (A & C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 A(1);
		constexpr rblxmath::ivec3 B = A | 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(A == (A | C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 A(1);
		constexpr rblxmath::ivec3 B = A ^ 0;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(0);
		static_assert(A == (A ^ C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 A(1);
		constexpr rblxmath::ivec3 B = A << 1;
		static_assert(B == rblxmath::ivec3(2), "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(B == (A << C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 A(2);
		constexpr rblxmath::ivec3 B = A >> 1;
		static_assert(B == rblxmath::ivec3(1), "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(B == A >> C, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec3 A(~0);
		constexpr rblxmath::ivec3 B = ~A;
		static_assert(A == ~B, "RBLXMATH: Failed constexpr");
	}

	return Error;
}

static int test_vec4()
{
	int Error = 0;
	
	{
		constexpr rblxmath::bvec4 B(true);
		constexpr bool A = rblxmath::all(B);
		static_assert(A, "RBLXMATH: Failed constexpr");
		
		constexpr rblxmath::bvec4 D(true, false, true, false);
		constexpr bool C = rblxmath::any(D);
		static_assert(C, "RBLXMATH: Failed constexpr");
	}
	
	{
		constexpr rblxmath::bvec4 C(true);
		constexpr rblxmath::bvec4 B(true, false, true, false);
		static_assert(rblxmath::any(rblxmath::equal(C, B)), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec4 O(rblxmath::ivec4(1));
		static_assert(rblxmath::ivec4(1) == O, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec4 A(1);
		static_assert(rblxmath::ivec4(1) == A, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec4 N(rblxmath::ivec4(1, 2, 3, 4));
		static_assert(rblxmath::ivec4(1, 2, 3, 4) == N, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec4 A(rblxmath::ivec3(1, 2, 3), 4);
		static_assert(rblxmath::ivec4(1, 2, 3, 4) == A, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec4 B(rblxmath::ivec2(1, 2), rblxmath::ivec2(3, 4));
		static_assert(rblxmath::ivec4(1, 2, 3, 4) == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec4 C(1, rblxmath::ivec3(2, 3, 4));
		static_assert(rblxmath::ivec4(1, 2, 3, 4) == C, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec4 D(rblxmath::ivec1(1), rblxmath::ivec2(2, 3), rblxmath::ivec1(4));
		static_assert(rblxmath::ivec4(1, 2, 3, 4) == D, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec4 E(rblxmath::ivec2(1, 2), rblxmath::ivec1(3), rblxmath::ivec1(4));
		static_assert(rblxmath::ivec4(1, 2, 3, 4) == E, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec4 F(rblxmath::ivec1(1), rblxmath::ivec1(2), rblxmath::ivec2(3, 4));
		static_assert(rblxmath::ivec4(1, 2, 3, 4) == F, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec4 A(1);
		static_assert(A[0] == 1, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::ivec4(1).x > 0, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::ivec4(1.0f, -1.0f, -1.0f, 1.0f).x > 0, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::ivec4(1.0f, -1.0f, -1.0f, 1.0f).y < 0, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::ivec4::length() == 4, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::bvec4 A1(true);
		constexpr rblxmath::bvec4 A2(true);
		constexpr rblxmath::bvec4 B1(false);
		constexpr rblxmath::bvec4 B2(false);
		static_assert(A1 == A2 && B1 == B2, "RBLXMATH: Failed constexpr");
		static_assert(A1 == A2 || B1 == B2, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec4 A(1);
		constexpr rblxmath::ivec4 B = A + 1;
		constexpr rblxmath::ivec4 C(3);
		static_assert(A + B == C, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec4 D = +A;
		static_assert(D == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec4 A(3);
		constexpr rblxmath::ivec4 B = A - 1;
		constexpr rblxmath::ivec4 C(1);
		static_assert(A - B == C, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec4 D = -A;
		static_assert(-D == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec4 A(3);
		constexpr rblxmath::ivec4 B = A * 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec4 C(1);
		static_assert(B * C == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec4 A(3);
		constexpr rblxmath::ivec4 B = A / 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec4 C(1);
		static_assert(B / C == A, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec4 A(3);
		constexpr rblxmath::ivec4 B = A % 2;
		constexpr rblxmath::ivec4 C(1);
		static_assert(B == C, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 D(2);
		static_assert(A % D == C, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec4 A(1);
		constexpr rblxmath::ivec4 B = A & 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(A == (A & C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec4 A(1);
		constexpr rblxmath::ivec4 B = A | 1;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(A == (A | C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec4 A(1);
		constexpr rblxmath::ivec4 B = A ^ 0;
		static_assert(A == B, "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(0);
		static_assert(A == (A ^ C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec4 A(1);
		constexpr rblxmath::ivec4 B = A << 1;
		static_assert(B == rblxmath::ivec4(2), "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(B == (A << C), "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec4 A(2);
		constexpr rblxmath::ivec4 B = A >> 1;
		static_assert(B == rblxmath::ivec4(1), "RBLXMATH: Failed constexpr");

		constexpr rblxmath::ivec1 C(1);
		static_assert(B == A >> C, "RBLXMATH: Failed constexpr");
	}

	{
		constexpr rblxmath::ivec4 A(~0);
		constexpr rblxmath::ivec4 B = ~A;
		static_assert(A == ~B, "RBLXMATH: Failed constexpr");
	}

	return Error;
}

static int test_quat()
{
	int Error = 0;

	{
		static_assert(rblxmath::quat::length() == 4, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::quat(1.0f, rblxmath::vec3(0.0f)).w > 0.0f, "RBLXMATH: Failed constexpr");
		static_assert(rblxmath::quat(1.0f, 0.0f, 0.0f, 0.0f).w > 0.0f, "RBLXMATH: Failed constexpr");

		rblxmath::quat constexpr Q = rblxmath::identity<rblxmath::quat>();
		static_assert(Q.x - rblxmath::quat(1.0f, rblxmath::vec3(0.0f)).x <= rblxmath::epsilon<float>(), "RBLXMATH: Failed constexpr");
	}

	return Error;
}

static int test_mat2x2()
{
	int Error = 0;

	static_assert(rblxmath::mat2x2::length() == 2, "RBLXMATH: Failed constexpr");

	return Error;
}

#endif//RBLXMATH_CONFIG_CONSTEXP == RBLXMATH_ENABLE

int main()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_CONSTEXP == RBLXMATH_ENABLE
		Error += test_vec1();
		Error += test_vec2();
		Error += test_vec3();
		Error += test_vec4();
		Error += test_quat();
		Error += test_mat2x2();
#	endif//RBLXMATH_CONFIG_CONSTEXP == RBLXMATH_ENABLE

	return Error;
}

