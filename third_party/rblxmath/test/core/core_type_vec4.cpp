#define RBLXMATH_FORCE_SWIZZLE
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/gtc/vec1.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/vector_relational.hpp>
#include <rblxmath/vec2.hpp>
#include <rblxmath/vec3.hpp>
#include <rblxmath/vec4.hpp>
#include <cstdio>
#include <ctime>
#include <vector>

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wglobal-constructors"
#	pragma clang diagnostic ignored "-Wunused-variable"
#endif

static rblxmath::vec4 g1;
static rblxmath::vec4 g2(1);
static rblxmath::vec4 g3(1, 1, 1, 1);

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

template <int Value>
struct mask
{
	enum{value = Value};
};

enum comp
{
	X,
	Y,
	Z,
	W
};

//template<comp X, comp Y, comp Z, comp W>
//__m128 swizzle(rblxmath::vec4 const& v)
//{
//	__m128 Src = _mm_set_ps(v.w, v.z, v.y, v.x);
//	return _mm_shuffle_ps(Src, Src, mask<(int(W) << 6) | (int(Z) << 4) | (int(Y) << 2) | (int(X) << 0)>::value);
//}

static int test_vec4_ctor()
{
	int Error = 0;

	{
		rblxmath::ivec4 A(1, 2, 3, 4);
		rblxmath::ivec4 B(A);
		Error += rblxmath::all(rblxmath::equal(A, B)) ? 0 : 1;
	}

#	if RBLXMATH_HAS_TRIVIAL_QUERIES
	//	Error += std::is_trivially_default_constructible<rblxmath::vec4>::value ? 0 : 1;
	//	Error += std::is_trivially_copy_assignable<rblxmath::vec4>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::vec4>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::dvec4>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::ivec4>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::uvec4>::value ? 0 : 1;

		Error += std::is_copy_constructible<rblxmath::vec4>::value ? 0 : 1;
#	endif

#if RBLXMATH_HAS_INITIALIZER_LISTS
	{
		rblxmath::vec4 a{ 0, 1, 2, 3 };
		std::vector<rblxmath::vec4> v = {
			a,
			{4, 5, 6, 7},
			{8, 9, 0, 1}};
	}

	{
		rblxmath::dvec4 a{ 0, 1, 2, 3 };
		std::vector<rblxmath::dvec4> v = {
			a,
			{4, 5, 6, 7},
			{8, 9, 0, 1}};
	}
#endif

	{
		rblxmath::ivec4 A(1);
		rblxmath::ivec4 B(1, 1, 1, 1);
		
		Error += A == B ? 0 : 1;
	}
	
	{
		std::vector<rblxmath::ivec4> Tests;
		Tests.push_back(rblxmath::ivec4(rblxmath::ivec2(1, 2), 3, 4));
		Tests.push_back(rblxmath::ivec4(1, rblxmath::ivec2(2, 3), 4));
		Tests.push_back(rblxmath::ivec4(1, 2, rblxmath::ivec2(3, 4)));
		Tests.push_back(rblxmath::ivec4(rblxmath::ivec3(1, 2, 3), 4));
		Tests.push_back(rblxmath::ivec4(1, rblxmath::ivec3(2, 3, 4)));
		Tests.push_back(rblxmath::ivec4(rblxmath::ivec2(1, 2), rblxmath::ivec2(3, 4)));
		Tests.push_back(rblxmath::ivec4(1, 2, 3, 4));
		Tests.push_back(rblxmath::ivec4(rblxmath::ivec4(1, 2, 3, 4)));
		
		for(std::size_t i = 0; i < Tests.size(); ++i)
			Error += Tests[i] == rblxmath::ivec4(1, 2, 3, 4) ? 0 : 1;
	}

	{
		rblxmath::vec1 R(1.0f);
		rblxmath::vec1 S(2.0f);
		rblxmath::vec1 T(3.0f);
		rblxmath::vec1 U(4.0f);
		rblxmath::vec4 O(1.0f, 2.0f, 3.0f, 4.0f);

		rblxmath::vec4 A(R);
		rblxmath::vec4 B(1.0f);
		Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 C(R, S, T, U);
		Error += rblxmath::all(rblxmath::equal(C, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 D(R, 2.0f, 3.0f, 4.0f);
		Error += rblxmath::all(rblxmath::equal(D, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 E(1.0f, S, 3.0f, 4.0f);
		Error += rblxmath::all(rblxmath::equal(E, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 F(R, S, 3.0f, 4.0f);
		Error += rblxmath::all(rblxmath::equal(F, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 G(1.0f, 2.0f, T, 4.0f);
		Error += rblxmath::all(rblxmath::equal(G, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 H(R, 2.0f, T, 4.0f);
		Error += rblxmath::all(rblxmath::equal(H, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 I(1.0f, S, T, 4.0f);
		Error += rblxmath::all(rblxmath::equal(I, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 J(R, S, T, 4.0f);
		Error += rblxmath::all(rblxmath::equal(J, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 K(R, 2.0f, 3.0f, U);
		Error += rblxmath::all(rblxmath::equal(K, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 L(1.0f, S, 3.0f, U);
		Error += rblxmath::all(rblxmath::equal(L, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 M(R, S, 3.0f, U);
		Error += rblxmath::all(rblxmath::equal(M, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 N(1.0f, 2.0f, T, U);
		Error += rblxmath::all(rblxmath::equal(N, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 P(R, 2.0f, T, U);
		Error += rblxmath::all(rblxmath::equal(P, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 Q(1.0f, S, T, U);
		Error += rblxmath::all(rblxmath::equal(Q, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 V(R, S, T, U);
		Error += rblxmath::all(rblxmath::equal(V, O, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::vec1 const R(1.0f);
		rblxmath::dvec1 const S(2.0);
		rblxmath::vec1 const T(3.0);
		rblxmath::dvec1 const U(4.0);
		rblxmath::vec4 const O(1.0f, 2.0, 3.0f, 4.0);

		rblxmath::vec4 const A(R);
		rblxmath::vec4 const B(1.0);
		Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const C(R, S, T, U);
		Error += rblxmath::all(rblxmath::equal(C, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const D(R, 2.0f, 3.0, 4.0f);
		Error += rblxmath::all(rblxmath::equal(D, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const E(1.0, S, 3.0f, 4.0);
		Error += rblxmath::all(rblxmath::equal(E, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const F(R, S, 3.0, 4.0f);
		Error += rblxmath::all(rblxmath::equal(F, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const G(1.0f, 2.0, T, 4.0);
		Error += rblxmath::all(rblxmath::equal(G, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const H(R, 2.0, T, 4.0);
		Error += rblxmath::all(rblxmath::equal(H, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const I(1.0, S, T, 4.0f);
		Error += rblxmath::all(rblxmath::equal(I, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const J(R, S, T, 4.0f);
		Error += rblxmath::all(rblxmath::equal(J, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const K(R, 2.0f, 3.0, U);
		Error += rblxmath::all(rblxmath::equal(K, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const L(1.0f, S, 3.0, U);
		Error += rblxmath::all(rblxmath::equal(L, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const M(R, S, 3.0, U);
		Error += rblxmath::all(rblxmath::equal(M, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const N(1.0f, 2.0, T, U);
		Error += rblxmath::all(rblxmath::equal(N, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const P(R, 2.0, T, U);
		Error += rblxmath::all(rblxmath::equal(P, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const Q(1.0f, S, T, U);
		Error += rblxmath::all(rblxmath::equal(Q, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const V(R, S, T, U);
		Error += rblxmath::all(rblxmath::equal(V, O, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::vec1 v1_0(1.0f);
		rblxmath::vec1 v1_1(2.0f);
		rblxmath::vec1 v1_2(3.0f);
		rblxmath::vec1 v1_3(4.0f);

		rblxmath::vec2 v2_0(1.0f, 2.0f);
		rblxmath::vec2 v2_1(2.0f, 3.0f);
		rblxmath::vec2 v2_2(3.0f, 4.0f);

		rblxmath::vec3 v3_0(1.0f, 2.0f, 3.0f);
		rblxmath::vec3 v3_1(2.0f, 3.0f, 4.0f);

		rblxmath::vec4 O(1.0f, 2.0, 3.0f, 4.0);

		rblxmath::vec4 A(v1_0, v1_1, v2_2);
		Error += rblxmath::all(rblxmath::equal(A, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 B(1.0f, 2.0f, v2_2);
		Error += rblxmath::all(rblxmath::equal(B, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 C(v1_0, 2.0f, v2_2);
		Error += rblxmath::all(rblxmath::equal(C, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 D(1.0f, v1_1, v2_2);
		Error += rblxmath::all(rblxmath::equal(D, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 E(v2_0, v1_2, v1_3);
		Error += rblxmath::all(rblxmath::equal(E, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 F(v2_0, 3.0, v1_3);
		Error += rblxmath::all(rblxmath::equal(F, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 G(v2_0, v1_2, 4.0);
		Error += rblxmath::all(rblxmath::equal(G, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 H(v2_0, 3.0f, 4.0);
		Error += rblxmath::all(rblxmath::equal(H, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 I(v3_0, 4.0);
		Error += rblxmath::all(rblxmath::equal(I, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 J(1.0f, v3_1);
		Error += rblxmath::all(rblxmath::equal(J, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 K(1.0f, v2_1, 4.0f);
		Error += rblxmath::all(rblxmath::equal(J, O, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	return Error;
}

static int test_bvec4_ctor()
{
	int Error = 0;

	rblxmath::bvec4 A(true);
	rblxmath::bvec4 B(true);
	rblxmath::bvec4 C(false);
	rblxmath::bvec4 D = A && B;
	rblxmath::bvec4 E = A && C;
	rblxmath::bvec4 F = A || C;

	Error += D == rblxmath::bvec4(true) ? 0 : 1;
	Error += E == rblxmath::bvec4(false) ? 0 : 1;
	Error += F == rblxmath::bvec4(true) ? 0 : 1;

	bool G = A == C;
	bool H = A != C;
	Error += !G ? 0 : 1;
	Error += H ? 0 : 1;

	return Error;
}

static int test_operators()
{
	int Error = 0;
	
	{
		rblxmath::ivec4 A(1);
		rblxmath::ivec4 B(1);
		bool R = A != B;
		bool S = A == B;

		Error += (S && !R) ? 0 : 1;
	}

	{
		rblxmath::vec4 const A(1.0f, 2.0f, 3.0f, 4.0f);
		rblxmath::vec4 const B(4.0f, 5.0f, 6.0f, 7.0f);

		rblxmath::vec4 const C = A + B;
		Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec4(5, 7, 9, 11), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const D = B - A;
		Error += rblxmath::all(rblxmath::equal(D, rblxmath::vec4(3, 3, 3, 3), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const E = A * B;
		Error += rblxmath::all(rblxmath::equal(E, rblxmath::vec4(4, 10, 18, 28), rblxmath::epsilon<float>()) )? 0 : 1;

		rblxmath::vec4 const F = B / A;
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::vec4(4, 2.5, 2, 7.0f / 4.0f), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const G = A + 1.0f;
		Error += rblxmath::all(rblxmath::equal(G, rblxmath::vec4(2, 3, 4, 5), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const H = B - 1.0f;
		Error += rblxmath::all(rblxmath::equal(H, rblxmath::vec4(3, 4, 5, 6), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const I = A * 2.0f;
		Error += rblxmath::all(rblxmath::equal(I, rblxmath::vec4(2, 4, 6, 8), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const J = B / 2.0f;
		Error += rblxmath::all(rblxmath::equal(J, rblxmath::vec4(2, 2.5, 3, 3.5), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const K = 1.0f + A;
		Error += rblxmath::all(rblxmath::equal(K, rblxmath::vec4(2, 3, 4, 5), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const L = 1.0f - B;
		Error += rblxmath::all(rblxmath::equal(L, rblxmath::vec4(-3, -4, -5, -6), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const M = 2.0f * A;
		Error += rblxmath::all(rblxmath::equal(M, rblxmath::vec4(2, 4, 6, 8), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 const N = 2.0f / B;
		Error += rblxmath::all(rblxmath::equal(N, rblxmath::vec4(0.5, 2.0 / 5.0, 2.0 / 6.0, 2.0 / 7.0), rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::ivec4 A(1.0f, 2.0f, 3.0f, 4.0f);
		rblxmath::ivec4 B(4.0f, 5.0f, 6.0f, 7.0f);

		A += B;
		Error += A == rblxmath::ivec4(5, 7, 9, 11) ? 0 : 1;

		A += 1;
		Error += A == rblxmath::ivec4(6, 8, 10, 12) ? 0 : 1;
	}
	{
		rblxmath::ivec4 A(1.0f, 2.0f, 3.0f, 4.0f);
		rblxmath::ivec4 B(4.0f, 5.0f, 6.0f, 7.0f);

		B -= A;
		Error += B == rblxmath::ivec4(3, 3, 3, 3) ? 0 : 1;

		B -= 1;
		Error += B == rblxmath::ivec4(2, 2, 2, 2) ? 0 : 1;
	}
	{
		rblxmath::ivec4 A(1.0f, 2.0f, 3.0f, 4.0f);
		rblxmath::ivec4 B(4.0f, 5.0f, 6.0f, 7.0f);

		A *= B;
		Error += A == rblxmath::ivec4(4, 10, 18, 28) ? 0 : 1;

		A *= 2;
		Error += A == rblxmath::ivec4(8, 20, 36, 56) ? 0 : 1;
	}
	{
		rblxmath::ivec4 A(1.0f, 2.0f, 2.0f, 4.0f);
		rblxmath::ivec4 B(4.0f, 4.0f, 8.0f, 8.0f);

		B /= A;
		Error += B == rblxmath::ivec4(4, 2, 4, 2) ? 0 : 1;

		B /= 2;
		Error += B == rblxmath::ivec4(2, 1, 2, 1) ? 0 : 1;
	}
	{
		rblxmath::ivec4 B(2);

		B /= B.y;
		Error += B == rblxmath::ivec4(1) ? 0 : 1;
	}

	{
		rblxmath::ivec4 A(1.0f, 2.0f, 3.0f, 4.0f);
		rblxmath::ivec4 B = -A;
		Error += B == rblxmath::ivec4(-1.0f, -2.0f, -3.0f, -4.0f) ? 0 : 1;
	}

	{
		rblxmath::ivec4 A(1.0f, 2.0f, 3.0f, 4.0f);
		rblxmath::ivec4 B = --A;
		Error += B == rblxmath::ivec4(0.0f, 1.0f, 2.0f, 3.0f) ? 0 : 1;
	}

	{
		rblxmath::ivec4 A(1.0f, 2.0f, 3.0f, 4.0f);
		rblxmath::ivec4 B = A--;
		Error += B == rblxmath::ivec4(1.0f, 2.0f, 3.0f, 4.0f) ? 0 : 1;
		Error += A == rblxmath::ivec4(0.0f, 1.0f, 2.0f, 3.0f) ? 0 : 1;
	}

	{
		rblxmath::ivec4 A(1.0f, 2.0f, 3.0f, 4.0f);
		rblxmath::ivec4 B = ++A;
		Error += B == rblxmath::ivec4(2.0f, 3.0f, 4.0f, 5.0f) ? 0 : 1;
	}

	{
		rblxmath::ivec4 A(1.0f, 2.0f, 3.0f, 4.0f);
		rblxmath::ivec4 B = A++;
		Error += B == rblxmath::ivec4(1.0f, 2.0f, 3.0f, 4.0f) ? 0 : 1;
		Error += A == rblxmath::ivec4(2.0f, 3.0f, 4.0f, 5.0f) ? 0 : 1;
	}

	return Error;
}

static int test_equal()
{
	int Error = 0;

	{
		rblxmath::uvec4 A(1, 2, 3, 4);
		rblxmath::uvec4 B(1, 2, 3, 4);
		Error += A == B ? 0 : 1;
		Error += A != B ? 1 : 0;
	}

	{
		rblxmath::ivec4 A(1, 2, 3, 4);
		rblxmath::ivec4 B(1, 2, 3, 4);
		Error += A == B ? 0 : 1;
		Error += A != B ? 1 : 0;
	}

	return Error;
}

static int test_size()
{
	int Error = 0;

	Error += sizeof(rblxmath::vec4) == sizeof(rblxmath::lowp_vec4) ? 0 : 1;
	Error += sizeof(rblxmath::vec4) == sizeof(rblxmath::mediump_vec4) ? 0 : 1;
	Error += sizeof(rblxmath::vec4) == sizeof(rblxmath::highp_vec4) ? 0 : 1;
	Error += 16 == sizeof(rblxmath::mediump_vec4) ? 0 : 1;
	Error += sizeof(rblxmath::dvec4) == sizeof(rblxmath::lowp_dvec4) ? 0 : 1;
	Error += sizeof(rblxmath::dvec4) == sizeof(rblxmath::mediump_dvec4) ? 0 : 1;
	Error += sizeof(rblxmath::dvec4) == sizeof(rblxmath::highp_dvec4) ? 0 : 1;
	Error += 32 == sizeof(rblxmath::highp_dvec4) ? 0 : 1;
	Error += rblxmath::vec4().length() == 4 ? 0 : 1;
	Error += rblxmath::dvec4().length() == 4 ? 0 : 1;
	Error += rblxmath::vec4::length() == 4 ? 0 : 1;
	Error += rblxmath::dvec4::length() == 4 ? 0 : 1;

	return Error;
}

static int test_swizzle_partial()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
	rblxmath::vec4 const A(1, 2, 3, 4);

	{
		rblxmath::vec4 B(A.xy, A.zw);
		Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;
	}
	{
		rblxmath::vec4 B(A.xy, 3.0f, 4.0f);
		Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;
	}
	{
		rblxmath::vec4 B(1.0f, A.yz, 4.0f);
		Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;
	}
	{
		rblxmath::vec4 B(1.0f, 2.0f, A.zw);
		Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::vec4 B(A.xyz, 4.0f);
		Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;
	}
	{
		rblxmath::vec4 B(1.0f, A.yzw);
		Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR || RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION

	return Error;
}

static int test_swizzle()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
	{
		rblxmath::ivec4 A = rblxmath::ivec4(1.0f, 2.0f, 3.0f, 4.0f);
		rblxmath::ivec4 B = A.xyzw;
		rblxmath::ivec4 C(A.xyzw);
		rblxmath::ivec4 D(A.xyzw());
		rblxmath::ivec4 E(A.x, A.yzw);
		rblxmath::ivec4 F(A.x, A.yzw());
		rblxmath::ivec4 G(A.xyz, A.w);
		rblxmath::ivec4 H(A.xyz(), A.w);
		rblxmath::ivec4 I(A.xy, A.zw);
		rblxmath::ivec4 J(A.xy(), A.zw());
		rblxmath::ivec4 K(A.x, A.y, A.zw);
		rblxmath::ivec4 L(A.x, A.yz, A.w);
		rblxmath::ivec4 M(A.xy, A.z, A.w);

		Error += rblxmath::all(rblxmath::equal(A, B)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, C)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, D)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, E)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, F)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, G)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, H)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, I)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, J)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, K)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, L)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, M)) ? 0 : 1;
	}
#	endif//RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR || RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION
	{
		rblxmath::vec4 A = rblxmath::vec4(1.0f, 2.0f, 3.0f, 4.0f);
		rblxmath::vec4 B = A.xyzw();
		rblxmath::vec4 C(A.xyzw());
		rblxmath::vec4 D(A.xyzw());
		rblxmath::vec4 E(A.x, A.yzw());
		rblxmath::vec4 F(A.x, A.yzw());
		rblxmath::vec4 G(A.xyz(), A.w);
		rblxmath::vec4 H(A.xyz(), A.w);
		rblxmath::vec4 I(A.xy(), A.zw());
		rblxmath::vec4 J(A.xy(), A.zw());
		rblxmath::vec4 K(A.x, A.y, A.zw());
		rblxmath::vec4 L(A.x, A.yz(), A.w);
		rblxmath::vec4 M(A.xy(), A.z, A.w);

		Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, C, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, D, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, E, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, F, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, G, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, H, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, I, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, J, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, K, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, L, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, M, rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR || RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION

	return Error;
}

static int test_operator_increment()
{
	int Error = 0;

	rblxmath::ivec4 v0(1);
	rblxmath::ivec4 v1(v0);
	rblxmath::ivec4 v2(v0);
	rblxmath::ivec4 v3 = ++v1;
	rblxmath::ivec4 v4 = v2++;

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

struct AoS
{
	rblxmath::vec4 A;
	rblxmath::vec3 B;
	rblxmath::vec3 C;
	rblxmath::vec2 D;
};

static int test_perf_AoS(std::size_t Size)
{
	int Error = 0;

	std::vector<AoS> In;
	std::vector<AoS> Out;
	In.resize(Size);
	Out.resize(Size);

	std::clock_t StartTime = std::clock();

	for(std::size_t i = 0; i < In.size(); ++i)
		Out[i] = In[i];

	std::clock_t EndTime = std::clock();

	std::printf("AoS: %d\n", static_cast<int>(EndTime - StartTime));

	return Error;
}

static int test_perf_SoA(std::size_t Size)
{
	int Error = 0;

	std::vector<rblxmath::vec4> InA;
	std::vector<rblxmath::vec3> InB;
	std::vector<rblxmath::vec3> InC;
	std::vector<rblxmath::vec2> InD;
	std::vector<rblxmath::vec4> OutA;
	std::vector<rblxmath::vec3> OutB;
	std::vector<rblxmath::vec3> OutC;
	std::vector<rblxmath::vec2> OutD;

	InA.resize(Size);
	InB.resize(Size);
	InC.resize(Size);
	InD.resize(Size);
	OutA.resize(Size);
	OutB.resize(Size);
	OutC.resize(Size);
	OutD.resize(Size);

	std::clock_t StartTime = std::clock();

	for(std::size_t i = 0; i < InA.size(); ++i)
	{
		OutA[i] = InA[i];
		OutB[i] = InB[i];
		OutC[i] = InC[i];
		OutD[i] = InD[i];
	}

	std::clock_t EndTime = std::clock();

	std::printf("SoA: %d\n", static_cast<int>(EndTime - StartTime));

	return Error;
}

namespace heap
{
	struct A
	{
		float f;
	};

	struct B : public A
	{
		float g;
		rblxmath::vec4 v;
	};

	static int test()
	{
		int Error = 0;

		A* p = new B;
		p->f = 0.0f;
		delete p;

		Error += sizeof(B) == sizeof(rblxmath::vec4) + sizeof(float) * 2 ? 0 : 1;

		return Error;
	}
}//namespace heap

static int test_simd()
{
	int Error = 0;

	rblxmath::vec4 const a(std::clock(), std::clock(), std::clock(), std::clock());
	rblxmath::vec4 const b(std::clock(), std::clock(), std::clock(), std::clock());

	rblxmath::vec4 const c(b * a);
	rblxmath::vec4 const d(a + c);

	Error += rblxmath::all(rblxmath::greaterThanEqual(d, rblxmath::vec4(0))) ? 0 : 1;

	return Error;
}

static int test_inheritance()
{
	struct my_vec4 : public rblxmath::vec4
	{
		my_vec4()
			: rblxmath::vec4(76.f, 75.f, 74.f, 73.f)
			, member(82)
		{}

		int member;
	};

	int Error = 0;

	my_vec4 v;

	Error += v.member == 82 ? 0 : 1;
	Error += rblxmath::equal(v.x, 76.f, rblxmath::epsilon<float>()) ? 0 : 1;
	Error += rblxmath::equal(v.y, 75.f, rblxmath::epsilon<float>()) ? 0 : 1;
	Error += rblxmath::equal(v.z, 74.f, rblxmath::epsilon<float>()) ? 0 : 1;
	Error += rblxmath::equal(v.w, 73.f, rblxmath::epsilon<float>()) ? 0 : 1;

	return Error;
}

static int test_constexpr()
{
#if RBLXMATH_HAS_CONSTEXPR
	static_assert(rblxmath::vec4::length() == 4, "RBLXMATH: Failed constexpr");
	static_assert(rblxmath::vec4(1.0f).x > 0.0f, "RBLXMATH: Failed constexpr");
	static_assert(rblxmath::vec4(1.0f, -1.0f, -1.0f, -1.0f).x > 0.0f, "RBLXMATH: Failed constexpr");
	static_assert(rblxmath::vec4(1.0f, -1.0f, -1.0f, -1.0f).y < 0.0f, "RBLXMATH: Failed constexpr");
#endif

	return 0;
}
/*
static int test_simd_gen()
{
	int Error = 0;

	int const C = static_cast<int>(std::clock());
	int const D = static_cast<int>(std::clock());

	rblxmath::ivec4 const A(C);
	rblxmath::ivec4 const B(D);

	Error += A != B ? 0 : 1;

	return Error;
}
*/
int main()
{
	int Error = 0;

	//Error += test_simd_gen();

/*
	{
		rblxmath::ivec4 const a1(2);
		rblxmath::ivec4 const b1 = a1 >> 1;

		__m128i const e1 = _mm_set1_epi32(2);
		__m128i const f1 = _mm_srli_epi32(e1, 1);

		rblxmath::ivec4 const g1 = *reinterpret_cast<rblxmath::ivec4 const* const>(&f1);

		rblxmath::ivec4 const a2(-2);
		rblxmath::ivec4 const b2 = a2 >> 1;

		__m128i const e2 = _mm_set1_epi32(-1);
		__m128i const f2 = _mm_srli_epi32(e2, 1);

		rblxmath::ivec4 const g2 = *reinterpret_cast<rblxmath::ivec4 const* const>(&f2);

		std::printf("GNI\n");
	}

	{
		rblxmath::uvec4 const a1(2);
		rblxmath::uvec4 const b1 = a1 >> 1u;

		__m128i const e1 = _mm_set1_epi32(2);
		__m128i const f1 = _mm_srli_epi32(e1, 1);

		rblxmath::uvec4 const g1 = *reinterpret_cast<rblxmath::uvec4 const* const>(&f1);

		rblxmath::uvec4 const a2(-1);
		rblxmath::uvec4 const b2 = a2 >> 1u;

		__m128i const e2 = _mm_set1_epi32(-1);
		__m128i const f2 = _mm_srli_epi32(e2, 1);

		rblxmath::uvec4 const g2 = *reinterpret_cast<rblxmath::uvec4 const* const>(&f2);

		std::printf("GNI\n");
	}
*/

#	ifdef NDEBUG
	std::size_t const Size(1000000);
#	else
	std::size_t const Size(1);
#	endif//NDEBUG

	Error += test_perf_AoS(Size);
	Error += test_perf_SoA(Size);

	Error += test_vec4_ctor();
	Error += test_bvec4_ctor();
	Error += test_size();
	Error += test_operators();
	Error += test_equal();
	Error += test_swizzle();
	Error += test_swizzle_partial();
	Error += test_simd();
	Error += test_operator_increment();
	Error += heap::test();
	Error += test_inheritance();
	Error += test_constexpr();

	return Error;
}

