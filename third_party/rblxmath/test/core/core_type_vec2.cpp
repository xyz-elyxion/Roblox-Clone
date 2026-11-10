#define RBLXMATH_FORCE_SWIZZLE
#include <rblxmath/gtc/vec1.hpp>
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/vector_float1.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/vector_relational.hpp>
#include <rblxmath/vec2.hpp>
#include <vector>
#if RBLXMATH_HAS_TRIVIAL_QUERIES
#	include <type_traits>
#endif

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wglobal-constructors"
#	pragma clang diagnostic ignored "-Wunused-variable"
#endif

static rblxmath::ivec2 g1;
static rblxmath::ivec2 g2(1);
static rblxmath::ivec2 g3(1, 1);

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

static int test_operators()
{
	int Error = 0;

	{
		rblxmath::ivec2 A(1);
		rblxmath::ivec2 B(1);
		Error += A != B ? 1 : 0;
		Error += A == B ? 0 : 1;
	}

	{
		rblxmath::vec2 A(1.0f);
		rblxmath::vec2 C = A + 1.0f;
		A += 1.0f;
		Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec2(2.0f), rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, C, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::vec2 A(1.0f);
		rblxmath::vec2 B(2.0f,-1.0f);
		rblxmath::vec2 C = A + B;
		A += B;
		Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec2(3.0f, 0.0f), rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, C, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::vec2 A(1.0f);
		rblxmath::vec2 C = A - 1.0f;
		A -= 1.0f;
		Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec2(0.0f), rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, C, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::vec2 A(1.0f);
		rblxmath::vec2 B(2.0f,-1.0f);
		rblxmath::vec2 C = A - B;
		A -= B;
		Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec2(-1.0f, 2.0f), rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, C, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::vec2 A(1.0f);
		rblxmath::vec2 C = A * 2.0f;
		A *= 2.0f;
		Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec2(2.0f), rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, C, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::vec2 A(2.0f);
		rblxmath::vec2 B(2.0f);
		rblxmath::vec2 C = A / B;
		A /= B;
		Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec2(1.0f), rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, C, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::vec2 A(1.0f, 2.0f);
		rblxmath::vec2 B(4.0f, 5.0f);

		rblxmath::vec2 C = A + B;
		Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec2(5, 7), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec2 D = B - A;
		Error += rblxmath::all(rblxmath::equal(D, rblxmath::vec2(3, 3), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec2 E = A * B;
		Error += rblxmath::all(rblxmath::equal(E, rblxmath::vec2(4, 10), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec2 F = B / A;
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::vec2(4, 2.5), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec2 G = A + 1.0f;
		Error += rblxmath::all(rblxmath::equal(G, rblxmath::vec2(2, 3), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec2 H = B - 1.0f;
		Error += rblxmath::all(rblxmath::equal(H, rblxmath::vec2(3, 4), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec2 I = A * 2.0f;
		Error += rblxmath::all(rblxmath::equal(I, rblxmath::vec2(2, 4), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec2 J = B / 2.0f;
		Error += rblxmath::all(rblxmath::equal(J, rblxmath::vec2(2, 2.5), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec2 K = 1.0f + A;
		Error += rblxmath::all(rblxmath::equal(K, rblxmath::vec2(2, 3), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec2 L = 1.0f - B;
		Error += rblxmath::all(rblxmath::equal(L, rblxmath::vec2(-3, -4), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec2 M = 2.0f * A;
		Error += rblxmath::all(rblxmath::equal(M, rblxmath::vec2(2, 4), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec2 N = 2.0f / B;
		Error += rblxmath::all(rblxmath::equal(N, rblxmath::vec2(0.5, 2.0 / 5.0), rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::vec2 A(1.0f, 2.0f);
		rblxmath::vec2 B(4.0f, 5.0f);

		A += B;
		Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec2(5, 7), rblxmath::epsilon<float>())) ? 0 : 1;

		A += 1.0f;
		Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec2(6, 8), rblxmath::epsilon<float>())) ? 0 : 1;
	}
	{
		rblxmath::ivec2 A(1.0f, 2.0f);
		rblxmath::ivec2 B(4.0f, 5.0f);

		B -= A;
		Error += B == rblxmath::ivec2(3, 3) ? 0 : 1;

		B -= 1.0f;
		Error += B == rblxmath::ivec2(2, 2) ? 0 : 1;
	}
	{
		rblxmath::ivec2 A(1.0f, 2.0f);
		rblxmath::ivec2 B(4.0f, 5.0f);

		A *= B;
		Error += A == rblxmath::ivec2(4, 10) ? 0 : 1;

		A *= 2;
		Error += A == rblxmath::ivec2(8, 20) ? 0 : 1;
	}
	{
		rblxmath::ivec2 A(1.0f, 2.0f);
		rblxmath::ivec2 B(4.0f, 16.0f);

		B /= A;
		Error += B == rblxmath::ivec2(4, 8) ? 0 : 1;

		B /= 2.0f;
		Error += B == rblxmath::ivec2(2, 4) ? 0 : 1;
	}
	{
		rblxmath::ivec2 B(2);

		B /= B.y;
		Error += B == rblxmath::ivec2(1) ? 0 : 1;
	}

	{
		rblxmath::ivec2 A(1.0f, 2.0f);
		rblxmath::ivec2 B = -A;
		Error += B == rblxmath::ivec2(-1.0f, -2.0f) ? 0 : 1;
	}

	{
		rblxmath::ivec2 A(1.0f, 2.0f);
		rblxmath::ivec2 B = --A;
		Error += B == rblxmath::ivec2(0.0f, 1.0f) ? 0 : 1;
	}

	{
		rblxmath::ivec2 A(1.0f, 2.0f);
		rblxmath::ivec2 B = A--;
		Error += B == rblxmath::ivec2(1.0f, 2.0f) ? 0 : 1;
		Error += A == rblxmath::ivec2(0.0f, 1.0f) ? 0 : 1;
	}

	{
		rblxmath::ivec2 A(1.0f, 2.0f);
		rblxmath::ivec2 B = ++A;
		Error += B == rblxmath::ivec2(2.0f, 3.0f) ? 0 : 1;
	}

	{
		rblxmath::ivec2 A(1.0f, 2.0f);
		rblxmath::ivec2 B = A++;
		Error += B == rblxmath::ivec2(1.0f, 2.0f) ? 0 : 1;
		Error += A == rblxmath::ivec2(2.0f, 3.0f) ? 0 : 1;
	}

	return Error;
}

static int test_ctor()
{
	int Error = 0;

	{
		rblxmath::ivec2 A(1);
		rblxmath::ivec2 B(A);
		Error += A == B ? 0 : 1;
	}

#	if RBLXMATH_HAS_TRIVIAL_QUERIES
	//	Error += std::is_trivially_default_constructible<rblxmath::vec2>::value ? 0 : 1;
	//	Error += std::is_trivially_copy_assignable<rblxmath::vec2>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::vec2>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::dvec2>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::ivec2>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::uvec2>::value ? 0 : 1;

		Error += std::is_copy_constructible<rblxmath::vec2>::value ? 0 : 1;
#	endif

#if RBLXMATH_HAS_INITIALIZER_LISTS
	{
		rblxmath::vec2 a{ 0, 1 };
		std::vector<rblxmath::vec2> v = {
			a,
			{4, 5},
			{8, 9}};
	}

	{
		rblxmath::dvec2 a{ 0, 1 };
		std::vector<rblxmath::dvec2> v = {
			a,
			{4, 5},
			{8, 9}};
	}
#endif

	{
		rblxmath::vec2 A = rblxmath::vec2(2.0f);
		Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec2(2.0f), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec2 B = rblxmath::vec2(2.0f, 3.0f);
		Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec2(2.0f, 3.0f), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec2 C = rblxmath::vec2(2.0f, 3.0);
		Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec2(2.0f, 3.0f), rblxmath::epsilon<float>())) ? 0 : 1;

		//rblxmath::vec2 D = rblxmath::dvec2(2.0); // Build error TODO: What does the specification says?


		rblxmath::vec2 E(rblxmath::dvec2(2.0));
		Error += rblxmath::all(rblxmath::equal(E, rblxmath::vec2(2.0f), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec2 F(rblxmath::ivec2(2));
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::vec2(2.0f), rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::vec1 const R(1.0f);
		rblxmath::vec1 const S(2.0f);
		rblxmath::vec2 const O(1.0f, 2.0f);

		rblxmath::vec2 const A(R);
		rblxmath::vec2 const B(1.0f);
		Error += rblxmath::all(rblxmath::equal(A, B, 0.0001f)) ? 0 : 1;

		rblxmath::vec2 const C(R, S);
		Error += rblxmath::all(rblxmath::equal(C, O, 0.0001f)) ? 0 : 1;

		rblxmath::vec2 const D(R, 2.0f);
		Error += rblxmath::all(rblxmath::equal(D, O, 0.0001f)) ? 0 : 1;

		rblxmath::vec2 const E(1.0f, S);
		Error += rblxmath::all(rblxmath::equal(E, O, 0.0001f)) ? 0 : 1;
	}

	{
		rblxmath::vec1 const R(1.0f);
		rblxmath::dvec1 const S(2.0);
		rblxmath::vec2 const O(1.0, 2.0);

		rblxmath::vec2 const A(R);
		rblxmath::vec2 const B(1.0);
		Error += rblxmath::all(rblxmath::equal(A, B, 0.0001f)) ? 0 : 1;

		rblxmath::vec2 const C(R, S);
		Error += rblxmath::all(rblxmath::equal(C, O, 0.0001f)) ? 0 : 1;

		rblxmath::vec2 const D(R, 2.0);
		Error += rblxmath::all(rblxmath::equal(D, O, 0.0001f)) ? 0 : 1;

		rblxmath::vec2 const E(1.0, S);
		Error += rblxmath::all(rblxmath::equal(E, O, 0.0001f)) ? 0 : 1;
	}

	return Error;
}

static int test_size()
{
	int Error = 0;

	Error += sizeof(rblxmath::vec2) == sizeof(rblxmath::mediump_vec2) ? 0 : 1;
	Error += 8 == sizeof(rblxmath::mediump_vec2) ? 0 : 1;
	Error += sizeof(rblxmath::dvec2) == sizeof(rblxmath::highp_dvec2) ? 0 : 1;
	Error += 16 == sizeof(rblxmath::highp_dvec2) ? 0 : 1;
	Error += rblxmath::vec2().length() == 2 ? 0 : 1;
	Error += rblxmath::dvec2().length() == 2 ? 0 : 1;
	Error += rblxmath::vec2::length() == 2 ? 0 : 1;
	Error += rblxmath::dvec2::length() == 2 ? 0 : 1;

	RBLXMATH_CONSTEXPR rblxmath::length_t Length = rblxmath::vec2::length();
	Error += Length == 2 ? 0 : 1;

	return Error;
}

static int test_operator_increment()
{
	int Error = 0;

	rblxmath::ivec2 v0(1);
	rblxmath::ivec2 v1(v0);
	rblxmath::ivec2 v2(v0);
	rblxmath::ivec2 v3 = ++v1;
	rblxmath::ivec2 v4 = v2++;

	Error += rblxmath::all(rblxmath::equal(v0, v4)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(v1, v2)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(v1, v3)) ? 0 : 1;

	int i0(1);
	int i1(i0);
	int i2(i0);
	int i3 = ++i1;
	int i4 = i2++;

	Error += i0 == i4 ? 0 : 1;
	Error += i1 == i2 ? 0 : 1;
	Error += i1 == i3 ? 0 : 1;

	return Error;
}

static int test_constexpr()
{
#if RBLXMATH_HAS_CONSTEXPR
	static_assert(rblxmath::vec2::length() == 2, "RBLXMATH: Failed constexpr");
	static_assert(rblxmath::vec2(1.0f).x > 0.0f, "RBLXMATH: Failed constexpr");
	static_assert(rblxmath::vec2(1.0f, -1.0f).x > 0.0f, "RBLXMATH: Failed constexpr");
	static_assert(rblxmath::vec2(1.0f, -1.0f).y < 0.0f, "RBLXMATH: Failed constexpr");
#endif

	return 0;
}

static int test_swizzle()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
	{
		rblxmath::vec2 A = rblxmath::vec2(1.0f, 2.0f);
		rblxmath::vec2 B = A.xy;
		rblxmath::vec2 C(A.xy);
		rblxmath::vec2 D(A.xy());

		Error += rblxmath::all(rblxmath::equal(A, B, 0.0001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, C, 0.0001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, D, 0.0001f)) ? 0 : 1;
	}
#	endif//RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR || RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION
	{
		rblxmath::vec2 A = rblxmath::vec2(1.0f, 2.0f);
		rblxmath::vec2 B = A.xy();
		rblxmath::vec2 C(A.xy());

		Error += rblxmath::all(rblxmath::equal(A, B, 0.0001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, C, 0.0001f)) ? 0 : 1;
	}
#	endif//RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR || RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_size();
	Error += test_ctor();
	Error += test_operators();
	Error += test_operator_increment();
	Error += test_swizzle();
	Error += test_constexpr();

	return Error;
}
