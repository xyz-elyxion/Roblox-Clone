#ifndef RBLXMATH_FORCE_PURE
#	define RBLXMATH_FORCE_PURE
#endif//RBLXMATH_FORCE_PURE
#define RBLXMATH_FORCE_DEFAULT_ALIGNED_GENTYPES
#define RBLXMATH_FORCE_SWIZZLE
#include <rblxmath/ext/scalar_constants.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/vector_relational.hpp>
#include <rblxmath/vec2.hpp>
#include <rblxmath/vec3.hpp>
#include <rblxmath/vec4.hpp>
#include <ctime>
#include <vector>

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
		Error += rblxmath::all(rblxmath::equal(a, rblxmath::vec4(0, 1, 2, 3), rblxmath::epsilon<float>())) ? 0 : 1;

		std::vector<rblxmath::vec4> v = {
			{0, 1, 2, 3},
			{4, 5, 6, 7},
			{8, 9, 0, 1}};
		Error += rblxmath::all(rblxmath::equal(v[0], rblxmath::vec4(0, 1, 2, 3), rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v[1], rblxmath::vec4(4, 5, 6, 7), rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v[2], rblxmath::vec4(8, 9, 0, 1), rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::dvec4 a{ 0, 1, 2, 3 };
		Error += rblxmath::all(rblxmath::equal(a, rblxmath::dvec4(0, 1, 2, 3), rblxmath::epsilon<double>())) ? 0 : 1;

		std::vector<rblxmath::dvec4> v = {
			{0, 1, 2, 3},
			{4, 5, 6, 7},
			{8, 9, 0, 1}};
		Error += rblxmath::all(rblxmath::equal(v[0], rblxmath::dvec4(0, 1, 2, 3), rblxmath::epsilon<double>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v[1], rblxmath::dvec4(4, 5, 6, 7), rblxmath::epsilon<double>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v[2], rblxmath::dvec4(8, 9, 0, 1), rblxmath::epsilon<double>())) ? 0 : 1;
	}
#endif

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
	{
		rblxmath::ivec4 A = rblxmath::vec4(1.0f, 2.0f, 3.0f, 4.0f);
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
#	endif

#	if RBLXMATH_CONFIG_SWIZZLE
	{
		rblxmath::ivec4 A = rblxmath::vec4(1.0f, 2.0f, 3.0f, 4.0f);
		rblxmath::ivec4 B = A.xyzw();
		rblxmath::ivec4 C(A.xyzw());
		rblxmath::ivec4 D(A.xyzw());
		rblxmath::ivec4 E(A.x, A.yzw());
		rblxmath::ivec4 F(A.x, A.yzw());
		rblxmath::ivec4 G(A.xyz(), A.w);
		rblxmath::ivec4 H(A.xyz(), A.w);
		rblxmath::ivec4 I(A.xy(), A.zw());
		rblxmath::ivec4 J(A.xy(), A.zw());
		rblxmath::ivec4 K(A.x, A.y, A.zw());
		rblxmath::ivec4 L(A.x, A.yz(), A.w);
		rblxmath::ivec4 M(A.xy(), A.z, A.w);

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
#	endif//RBLXMATH_CONFIG_SWIZZLE

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

	Error += (D == A) ? 0 : 1;
	Error += (E == C) ? 0 : 1;
	Error += (F == A) ? 0 : 1;

	bool G = A == C;
	bool H = A != C;

	Error += !G ? 0 : 1;
	Error += H ? 0 : 1;

	return Error;
}

static int test_vec4_operators()
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
		Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec4(5, 7, 9, 11), 0.001f)) ? 0 : 1;

		rblxmath::vec4 D = B - A;
		Error += rblxmath::all(rblxmath::equal(D, rblxmath::vec4(3, 3, 3, 3), 0.001f)) ? 0 : 1;

		rblxmath::vec4 E = A * B;
		Error += rblxmath::all(rblxmath::equal(E, rblxmath::vec4(4, 10, 18, 28), 0.001f)) ? 0 : 1;

		rblxmath::vec4 F = B / A;
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::vec4(4, 2.5, 2, 7.0f / 4.0f), 0.001f)) ? 0 : 1;

		rblxmath::vec4 G = A + 1.0f;
		Error += rblxmath::all(rblxmath::equal(G, rblxmath::vec4(2, 3, 4, 5), 0.001f)) ? 0 : 1;

		rblxmath::vec4 H = B - 1.0f;
		Error += rblxmath::all(rblxmath::equal(H, rblxmath::vec4(3, 4, 5, 6), 0.001f)) ? 0 : 1;

		rblxmath::vec4 I = A * 2.0f;
		Error += rblxmath::all(rblxmath::equal(I, rblxmath::vec4(2, 4, 6, 8), 0.001f)) ? 0 : 1;

		rblxmath::vec4 J = B / 2.0f;
		Error += rblxmath::all(rblxmath::equal(J, rblxmath::vec4(2, 2.5, 3, 3.5), 0.001f)) ? 0 : 1;

		rblxmath::vec4 K = 1.0f + A;
		Error += rblxmath::all(rblxmath::equal(K, rblxmath::vec4(2, 3, 4, 5), 0.001f)) ? 0 : 1;

		rblxmath::vec4 L = 1.0f - B;
		Error += rblxmath::all(rblxmath::equal(L, rblxmath::vec4(-3, -4, -5, -6), 0.001f)) ? 0 : 1;

		rblxmath::vec4 M = 2.0f * A;
		Error += rblxmath::all(rblxmath::equal(M, rblxmath::vec4(2, 4, 6, 8), 0.001f)) ? 0 : 1;

		rblxmath::vec4 const N = 2.0f / B;
		Error += rblxmath::all(rblxmath::equal(N, rblxmath::vec4(0.5, 2.0 / 5.0, 2.0 / 6.0, 2.0 / 7.0), 0.0001f)) ? 0 : 1;
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
		rblxmath::ivec4 A(1.0f, 2.0f, 3.0f, 4.0f);
		rblxmath::ivec4 B(4.0f, 4.0f, 6.0f, 8.0f);

		B /= A;
		Error += B == rblxmath::ivec4(4, 2, 2, 2) ? 0 : 1;

		B /= 2;
		Error += B == rblxmath::ivec4(2, 1, 1, 1) ? 0 : 1;
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

static int test_vec4_equal()
{
	int Error = 0;

	{
		rblxmath::uvec4 A(1, 2, 3, 4);
		Error += (A == rblxmath::uvec4(1, 2, 3, 4)) ? 0 : 1;
		Error += (A != rblxmath::uvec4(1, 2, 3, 4)) ? 1 : 0;
	}

	{
		rblxmath::ivec4 A(1, 2, 3, 4);
		Error += (A == rblxmath::ivec4(1, 2, 3, 4)) ? 0 : 1;
		Error += (A != rblxmath::ivec4(1, 2, 3, 4)) ? 1 : 0;
	}

	return Error;
}

static int test_vec4_size()
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

	return Error;
}

static int test_vec4_swizzle_partial()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR

	rblxmath::ivec4 A(1, 2, 3, 4);

	{
		rblxmath::ivec4 B(A.xy, A.zw);
		Error += A == B ? 0 : 1;
	}
	{
		rblxmath::ivec4 B(A.xy, 3, 4);
		Error += A == B ? 0 : 1;
	}
	{
		rblxmath::ivec4 B(1, A.yz, 4);
		Error += A == B ? 0 : 1;
	}
	{
		rblxmath::ivec4 B(1, 2, A.zw);
		Error += A == B ? 0 : 1;
	}

	{
		rblxmath::ivec4 B(A.xyz, 4);
		Error += A == B ? 0 : 1;
	}
	{
		rblxmath::ivec4 B(1, A.yzw);
		Error += A == B ? 0 : 1;
	}
#	endif

	return Error;
}

static int test_operator_increment()
{
	int Error(0);

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

static int test_vec4_simd()
{
	int Error = 0;

	rblxmath::vec4 const a(std::clock(), std::clock(), std::clock(), std::clock());
	rblxmath::vec4 const b(std::clock(), std::clock(), std::clock(), std::clock());

	rblxmath::vec4 const c(b * a);
	rblxmath::vec4 const d(a + c);

	Error += rblxmath::all(rblxmath::greaterThanEqual(d, rblxmath::vec4(0))) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_vec4_ctor();
	Error += test_bvec4_ctor();
	Error += test_vec4_size();
	Error += test_vec4_operators();
	Error += test_vec4_equal();
	Error += test_vec4_swizzle_partial();
	Error += test_vec4_simd();
	Error += test_operator_increment();

	return Error;
}

