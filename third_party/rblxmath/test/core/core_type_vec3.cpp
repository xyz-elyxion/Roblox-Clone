#define RBLXMATH_FORCE_SWIZZLE
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/gtc/vec1.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/vector_relational.hpp>
#include <rblxmath/geometric.hpp>
#include <rblxmath/vec2.hpp>
#include <rblxmath/vec3.hpp>
#include <rblxmath/vec4.hpp>
#include <vector>

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wglobal-constructors"
#	pragma clang diagnostic ignored "-Wunused-variable"
#endif

static rblxmath::vec3 g1;
static rblxmath::vec3 g2(1);
static rblxmath::vec3 g3(1, 1, 1);

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

static int test_vec3_ctor()
{
	int Error = 0;

#	if RBLXMATH_HAS_TRIVIAL_QUERIES
	//	Error += std::is_trivially_default_constructible<rblxmath::vec3>::value ? 0 : 1;
	//	Error += std::is_trivially_copy_assignable<rblxmath::vec3>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::vec3>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::dvec3>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::ivec3>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::uvec3>::value ? 0 : 1;

		Error += std::is_copy_constructible<rblxmath::vec3>::value ? 0 : 1;
#	endif

#	if RBLXMATH_HAS_INITIALIZER_LISTS
	{
		rblxmath::vec3 a{ 0, 1, 2 };
		std::vector<rblxmath::vec3> v = {
			a,
			{4, 5, 6},
			{8, 9, 0}};
	}

	{
		rblxmath::dvec3 a{ 0, 1, 2 };
		std::vector<rblxmath::dvec3> v = {
			a,
			{4, 5, 6},
			{8, 9, 0}};
	}
#	endif

	{
		rblxmath::ivec3 A(1);
		rblxmath::ivec3 B(1, 1, 1);
		
		Error += A == B ? 0 : 1;
	}

	{
		std::vector<rblxmath::ivec3> Tests;
		Tests.push_back(rblxmath::ivec3(rblxmath::ivec2(1, 2), 3));
		Tests.push_back(rblxmath::ivec3(1, rblxmath::ivec2(2, 3)));
		Tests.push_back(rblxmath::ivec3(1, 2, 3));
		Tests.push_back(rblxmath::ivec3(rblxmath::ivec4(1, 2, 3, 4)));

		for (std::size_t i = 0; i < Tests.size(); ++i) {
			Error += Tests[i] == rblxmath::ivec3(1, 2, 3) ? 0 : 1;
		}
	}

	{
		rblxmath::vec1 const R(1.0f);
		rblxmath::vec1 const S(2.0f);
		rblxmath::vec1 const T(3.0f);
		rblxmath::vec3 const O(1.0f, 2.0f, 3.0f);

		rblxmath::vec3 const A(R);
		rblxmath::vec3 const B(1.0f);
		Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const C(R, S, T);
		Error += rblxmath::all(rblxmath::equal(C, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const D(R, 2.0f, 3.0f);
		Error += rblxmath::all(rblxmath::equal(D, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const E(1.0f, S, 3.0f);
		Error += rblxmath::all(rblxmath::equal(E, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const F(1.0f, S, T);
		Error += rblxmath::all(rblxmath::equal(F, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const G(R, 2.0f, T);
		Error += rblxmath::all(rblxmath::equal(G, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const H(R, S, 3.0f);
		Error += rblxmath::all(rblxmath::equal(H, O, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::vec1 const R(1.0);
		rblxmath::dvec1 const S(2.0);
		rblxmath::vec1 const T(3.0);
		rblxmath::vec3 const O(1.0f, 2.0f, 3.0f);

		rblxmath::vec3 const A(R);
		rblxmath::vec3 const B(1.0);
		Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const C(R, S, T);
		Error += rblxmath::all(rblxmath::equal(C, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const D(R, 2.0, 3.0);
		Error += rblxmath::all(rblxmath::equal(D, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const E(1.0f, S, 3.0);
		Error += rblxmath::all(rblxmath::equal(E, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const F(1.0, S, T);
		Error += rblxmath::all(rblxmath::equal(F, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const G(R, 2.0, T);
		Error += rblxmath::all(rblxmath::equal(G, O, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const H(R, S, 3.0);
		Error += rblxmath::all(rblxmath::equal(H, O, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	return Error;
}

static int test_bvec3_ctor()
{
	int Error = 0;

	rblxmath::bvec3 A(true);
	rblxmath::bvec3 B(true);
	rblxmath::bvec3 C(false);
	rblxmath::bvec3 D = A && B;
	rblxmath::bvec3 E = A && C;
	rblxmath::bvec3 F = A || C;

	Error += D == rblxmath::bvec3(true) ? 0 : 1;
	Error += E == rblxmath::bvec3(false) ? 0 : 1;
	Error += F == rblxmath::bvec3(true) ? 0 : 1;

	bool G = A == C;
	bool H = A != C;
	Error += !G ? 0 : 1;
	Error += H ? 0 : 1;

	return Error;
}

static int test_vec3_operators()
{
	int Error = 0;
	
	{
		rblxmath::ivec3 A(1);
		rblxmath::ivec3 B(1);
		bool R = A != B;
		bool S = A == B;

		Error += (S && !R) ? 0 : 1;
	}

	{
		rblxmath::vec3 const A(1.0f, 2.0f, 3.0f);
		rblxmath::vec3 const B(4.0f, 5.0f, 6.0f);

		rblxmath::vec3 const C = A + B;
		Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec3(5, 7, 9), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const D = B - A;
		Error += rblxmath::all(rblxmath::equal(D, rblxmath::vec3(3, 3, 3), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const E = A * B;
		Error += rblxmath::all(rblxmath::equal(E, rblxmath::vec3(4, 10, 18), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const F = B / A;
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::vec3(4, 2.5, 2), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const G = A + 1.0f;
		Error += rblxmath::all(rblxmath::equal(G, rblxmath::vec3(2, 3, 4), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const H = B - 1.0f;
		Error += rblxmath::all(rblxmath::equal(H, rblxmath::vec3(3, 4, 5), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const I = A * 2.0f;
		Error += rblxmath::all(rblxmath::equal(I, rblxmath::vec3(2, 4, 6), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const J = B / 2.0f;
		Error += rblxmath::all(rblxmath::equal(J, rblxmath::vec3(2, 2.5, 3), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const K = 1.0f + A;
		Error += rblxmath::all(rblxmath::equal(K, rblxmath::vec3(2, 3, 4), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const L = 1.0f - B;
		Error += rblxmath::all(rblxmath::equal(L, rblxmath::vec3(-3, -4, -5), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const M = 2.0f * A;
		Error += rblxmath::all(rblxmath::equal(M, rblxmath::vec3(2, 4, 6), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 const N = 2.0f / B;
		Error += rblxmath::all(rblxmath::equal(N, rblxmath::vec3(0.5, 2.0 / 5.0, 2.0 / 6.0), rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::ivec3 A(1.0f, 2.0f, 3.0f);
		rblxmath::ivec3 B(4.0f, 5.0f, 6.0f);

		A += B;
		Error += A == rblxmath::ivec3(5, 7, 9) ? 0 : 1;

		A += 1;
		Error += A == rblxmath::ivec3(6, 8, 10) ? 0 : 1;
	}
	{
		rblxmath::ivec3 A(1.0f, 2.0f, 3.0f);
		rblxmath::ivec3 B(4.0f, 5.0f, 6.0f);

		B -= A;
		Error += B == rblxmath::ivec3(3, 3, 3) ? 0 : 1;

		B -= 1;
		Error += B == rblxmath::ivec3(2, 2, 2) ? 0 : 1;
	}
	{
		rblxmath::ivec3 A(1.0f, 2.0f, 3.0f);
		rblxmath::ivec3 B(4.0f, 5.0f, 6.0f);

		A *= B;
		Error += A == rblxmath::ivec3(4, 10, 18) ? 0 : 1;

		A *= 2;
		Error += A == rblxmath::ivec3(8, 20, 36) ? 0 : 1;
	}
	{
		rblxmath::ivec3 A(1.0f, 2.0f, 3.0f);
		rblxmath::ivec3 B(4.0f, 4.0f, 6.0f);

		B /= A;
		Error += B == rblxmath::ivec3(4, 2, 2) ? 0 : 1;

		B /= 2;
		Error += B == rblxmath::ivec3(2, 1, 1) ? 0 : 1;
	}
	{
		rblxmath::ivec3 B(2);

		B /= B.y;
		Error += B == rblxmath::ivec3(1) ? 0 : 1;
	}

	{
		rblxmath::ivec3 A(1.0f, 2.0f, 3.0f);
		rblxmath::ivec3 B = -A;
		Error += B == rblxmath::ivec3(-1.0f, -2.0f, -3.0f) ? 0 : 1;
	}

	{
		rblxmath::ivec3 A(1.0f, 2.0f, 3.0f);
		rblxmath::ivec3 B = --A;
		Error += B == rblxmath::ivec3(0.0f, 1.0f, 2.0f) ? 0 : 1;
	}

	{
		rblxmath::ivec3 A(1.0f, 2.0f, 3.0f);
		rblxmath::ivec3 B = A--;
		Error += B == rblxmath::ivec3(1.0f, 2.0f, 3.0f) ? 0 : 1;
		Error += A == rblxmath::ivec3(0.0f, 1.0f, 2.0f) ? 0 : 1;
	}

	{
		rblxmath::ivec3 A(1.0f, 2.0f, 3.0f);
		rblxmath::ivec3 B = ++A;
		Error += B == rblxmath::ivec3(2.0f, 3.0f, 4.0f) ? 0 : 1;
	}

	{
		rblxmath::ivec3 A(1.0f, 2.0f, 3.0f);
		rblxmath::ivec3 B = A++;
		Error += B == rblxmath::ivec3(1.0f, 2.0f, 3.0f) ? 0 : 1;
		Error += A == rblxmath::ivec3(2.0f, 3.0f, 4.0f) ? 0 : 1;
	}

	return Error;
}

static int test_vec3_size()
{
	int Error = 0;
	
	Error += sizeof(rblxmath::vec3) == sizeof(rblxmath::lowp_vec3) ? 0 : 1;
	Error += sizeof(rblxmath::vec3) == sizeof(rblxmath::mediump_vec3) ? 0 : 1;
	Error += sizeof(rblxmath::vec3) == sizeof(rblxmath::highp_vec3) ? 0 : 1;
	Error += 12 == sizeof(rblxmath::mediump_vec3) ? 0 : 1;
	Error += sizeof(rblxmath::dvec3) == sizeof(rblxmath::lowp_dvec3) ? 0 : 1;
	Error += sizeof(rblxmath::dvec3) == sizeof(rblxmath::mediump_dvec3) ? 0 : 1;
	Error += sizeof(rblxmath::dvec3) == sizeof(rblxmath::highp_dvec3) ? 0 : 1;
	Error += 24 == sizeof(rblxmath::highp_dvec3) ? 0 : 1;
	Error += rblxmath::vec3().length() == 3 ? 0 : 1;
	Error += rblxmath::dvec3().length() == 3 ? 0 : 1;
	Error += rblxmath::vec3::length() == 3 ? 0 : 1;
	Error += rblxmath::dvec3::length() == 3 ? 0 : 1;

	RBLXMATH_CONSTEXPR rblxmath::length_t Length = rblxmath::vec3::length();
	Error += Length == 3 ? 0 : 1;

	return Error;
}

static int test_vec3_swizzle3_2()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
	{
		rblxmath::ivec3 v(1, 2, 3);
		rblxmath::ivec2 u;

		// Can not assign a vec3 swizzle to a vec2
		//u = v.xyz;    //Illegal
		//u = v.rgb;    //Illegal
		//u = v.stp;    //Illegal

		u = v.xx;       Error += (u.x == 1 && u.y == 1) ? 0 : 1;
		u = v.xy;       Error += (u.x == 1 && u.y == 2) ? 0 : 1;
		u = v.xz;       Error += (u.x == 1 && u.y == 3) ? 0 : 1;
		u = v.yx;       Error += (u.x == 2 && u.y == 1) ? 0 : 1;
		u = v.yy;       Error += (u.x == 2 && u.y == 2) ? 0 : 1;
		u = v.yz;       Error += (u.x == 2 && u.y == 3) ? 0 : 1;
		u = v.zx;       Error += (u.x == 3 && u.y == 1) ? 0 : 1;
		u = v.zy;       Error += (u.x == 3 && u.y == 2) ? 0 : 1;
		u = v.zz;       Error += (u.x == 3 && u.y == 3) ? 0 : 1;

		u = v.rr;       Error += (u.r == 1 && u.g == 1) ? 0 : 1;
		u = v.rg;       Error += (u.r == 1 && u.g == 2) ? 0 : 1;
		u = v.rb;       Error += (u.r == 1 && u.g == 3) ? 0 : 1;
		u = v.gr;       Error += (u.r == 2 && u.g == 1) ? 0 : 1;
		u = v.gg;       Error += (u.r == 2 && u.g == 2) ? 0 : 1;
		u = v.gb;       Error += (u.r == 2 && u.g == 3) ? 0 : 1;
		u = v.br;       Error += (u.r == 3 && u.g == 1) ? 0 : 1;
		u = v.bg;       Error += (u.r == 3 && u.g == 2) ? 0 : 1;
		u = v.bb;       Error += (u.r == 3 && u.g == 3) ? 0 : 1;

		u = v.ss;       Error += (u.s == 1 && u.t == 1) ? 0 : 1;
		u = v.st;       Error += (u.s == 1 && u.t == 2) ? 0 : 1;
		u = v.sp;       Error += (u.s == 1 && u.t == 3) ? 0 : 1;
		u = v.ts;       Error += (u.s == 2 && u.t == 1) ? 0 : 1;
		u = v.tt;       Error += (u.s == 2 && u.t == 2) ? 0 : 1;
		u = v.tp;       Error += (u.s == 2 && u.t == 3) ? 0 : 1;
		u = v.ps;       Error += (u.s == 3 && u.t == 1) ? 0 : 1;
		u = v.pt;       Error += (u.s == 3 && u.t == 2) ? 0 : 1;
		u = v.pp;       Error += (u.s == 3 && u.t == 3) ? 0 : 1;
		// Mixed member aliases are not valid
		//u = v.rx;     //Illegal
		//u = v.sy;     //Illegal

		u = rblxmath::ivec2(1, 2);
		v = rblxmath::ivec3(1, 2, 3);
		//v.xx = u;     //Illegal
		v.xy = u;       Error += (v.x == 1 && v.y == 2 && v.z == 3) ? 0 : 1;
		v.xz = u;       Error += (v.x == 1 && v.y == 2 && v.z == 2) ? 0 : 1;
		v.yx = u;       Error += (v.x == 2 && v.y == 1 && v.z == 2) ? 0 : 1;
		//v.yy = u;     //Illegal
		v.yz = u;       Error += (v.x == 2 && v.y == 1 && v.z == 2) ? 0 : 1;
		v.zx = u;       Error += (v.x == 2 && v.y == 1 && v.z == 1) ? 0 : 1;
		v.zy = u;       Error += (v.x == 2 && v.y == 2 && v.z == 1) ? 0 : 1;
		//v.zz = u;     //Illegal
	}
#	endif//RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR

	return Error;
}

static int test_vec3_swizzle3_3()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
	{
		rblxmath::ivec3 v(1, 2, 3);
		rblxmath::ivec3 u;

		u = v;          Error += (u.x == 1 && u.y == 2 && u.z == 3) ? 0 : 1;

		u = v.xyz;      Error += (u.x == 1 && u.y == 2 && u.z == 3) ? 0 : 1;
		u = v.zyx;      Error += (u.x == 3 && u.y == 2 && u.z == 1) ? 0 : 1;
		u.zyx = v;      Error += (u.x == 3 && u.y == 2 && u.z == 1) ? 0 : 1;

		u = v.rgb;      Error += (u.x == 1 && u.y == 2 && u.z == 3) ? 0 : 1;
		u = v.bgr;      Error += (u.x == 3 && u.y == 2 && u.z == 1) ? 0 : 1;
		u.bgr = v;      Error += (u.x == 3 && u.y == 2 && u.z == 1) ? 0 : 1;

		u = v.stp;      Error += (u.x == 1 && u.y == 2 && u.z == 3) ? 0 : 1;
		u = v.pts;      Error += (u.x == 3 && u.y == 2 && u.z == 1) ? 0 : 1;
		u.pts = v;      Error += (u.x == 3 && u.y == 2 && u.z == 1) ? 0 : 1;
	}
#	endif//RBLXMATH_LANG

	return Error;
}

static int test_vec3_swizzle_operators()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
	{
		rblxmath::ivec3 const u = rblxmath::ivec3(1, 2, 3);
		rblxmath::ivec3 const v = rblxmath::ivec3(10, 20, 30);

		rblxmath::ivec3 q;

		// Swizzle, swizzle binary operators
		q = u.xyz + v.xyz;          Error += (q == (u + v)) ? 0 : 1;
		q = (u.zyx + v.zyx).zyx;    Error += (q == (u + v)) ? 0 : 1;
		q = (u.xyz - v.xyz);        Error += (q == (u - v)) ? 0 : 1;
		q = (u.xyz * v.xyz);        Error += (q == (u * v)) ? 0 : 1;
		q = (u.xxx * v.xxx);        Error += (q == rblxmath::ivec3(u.x * v.x)) ? 0 : 1;
		q = (u.xyz / v.xyz);        Error += (q == (u / v)) ? 0 : 1;

		// vec, swizzle binary operators
		q = u + v.xyz;              Error += (q == (u + v)) ? 0 : 1;
		q = (u - v.xyz);            Error += (q == (u - v)) ? 0 : 1;
		q = (u * v.xyz);            Error += (q == (u * v)) ? 0 : 1;
		q = (u * v.xxx);            Error += (q == v.x * u) ? 0 : 1;
		q = (u / v.xyz);            Error += (q == (u / v)) ? 0 : 1;

		// swizzle,vec binary operators
		q = u.xyz + v;              Error += (q == (u + v)) ? 0 : 1;
		q = (u.xyz - v);            Error += (q == (u - v)) ? 0 : 1;
		q = (u.xyz * v);            Error += (q == (u * v)) ? 0 : 1;
		q = (u.xxx * v);            Error += (q == u.x * v) ? 0 : 1;
		q = (u.xyz / v);            Error += (q == (u / v)) ? 0 : 1;
	}
#	endif//RBLXMATH_LANG

	// Compile errors
	//q = (u.yz * v.xyz);
	//q = (u * v.xy);

	return Error;
}

static int test_vec3_swizzle_functions()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR || RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION
	{
		// NOTE: template functions cannot pick up the implicit conversion from
		// a swizzle to the unswizzled type, therefore the operator() must be 
		// used.  E.g.:
		//
		// rblxmath::dot(u.xy, v.xy);        <--- Compile error
		// rblxmath::dot(u.xy(), v.xy());    <--- Compiles correctly

		float r;

		// vec2
		rblxmath::vec2 a(1, 2);
		rblxmath::vec2 b(10, 20);
		r = rblxmath::dot(a, b);                 Error += (int(r) == 50) ? 0 : 1;
		r = rblxmath::dot(rblxmath::vec2(a.xy()), rblxmath::vec2(b.xy()));       Error += (int(r) == 50) ? 0 : 1;
		r = rblxmath::dot(rblxmath::vec2(a.xy()), rblxmath::vec2(b.yy()));       Error += (int(r) == 60) ? 0 : 1;

		// vec3
		rblxmath::vec3 u = rblxmath::vec3(1, 2, 3);
		rblxmath::vec3 v = rblxmath::vec3(10, 20, 30);
		r = rblxmath::dot(u, v);                 Error += (int(r) == 140) ? 0 : 1;
		r = rblxmath::dot(u.xyz(), v.zyz());     Error += (int(r) == 160) ? 0 : 1;
		r = rblxmath::dot(u, v.zyx());           Error += (int(r) == 100) ? 0 : 1;
		r = rblxmath::dot(u.xyz(), v);           Error += (int(r) == 140) ? 0 : 1;
		r = rblxmath::dot(u.xy(), v.xy());       Error += (int(r) == 50) ? 0 : 1;

		// vec4
		rblxmath::vec4 s = rblxmath::vec4(1, 2, 3, 4);
		rblxmath::vec4 t = rblxmath::vec4(10, 20, 30, 40);
		r = rblxmath::dot(s, t);                 Error += (int(r) == 300) ? 0 : 1;
		r = rblxmath::dot(s.xyzw(), t.xyzw());   Error += (int(r) == 300) ? 0 : 1;
		r = rblxmath::dot(s.xyz(), t.xyz());     Error += (int(r) == 140) ? 0 : 1;
	}
#	endif//RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR || RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION

	return Error;
}

static int test_vec3_swizzle_partial()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
	{
		rblxmath::vec3 const A(1, 2, 3);
		rblxmath::vec3 B(A.xy, 3);
		Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::ivec3 const A(1, 2, 3);
		rblxmath::ivec3 const B(1, A.yz);
		Error += A == B ? 0 : 1;
	}

	{
		rblxmath::ivec3 const A(1, 2, 3);
		rblxmath::ivec3 const B(A.xyz);
		Error += A == B ? 0 : 1;
	}
#	endif//RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR

	return Error;
}

static int test_operator_increment()
{
	int Error = 0;

	rblxmath::ivec3 v0(1);
	rblxmath::ivec3 v1(v0);
	rblxmath::ivec3 v2(v0);
	rblxmath::ivec3 v3 = ++v1;
	rblxmath::ivec3 v4 = v2++;

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

static int test_swizzle()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
	{
		rblxmath::vec3 A = rblxmath::vec3(1.0f, 2.0f, 3.0f);
		rblxmath::vec3 B = A.xyz;
		rblxmath::vec3 C(A.xyz);
		rblxmath::vec3 D(A.xyz());
		rblxmath::vec3 E(A.x, A.yz);
		rblxmath::vec3 F(A.x, A.yz());
		rblxmath::vec3 G(A.xy, A.z);
		rblxmath::vec3 H(A.xy(), A.z);

		Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, C, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, D, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, E, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, F, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, G, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, H, rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR

#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR || RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION
	{
		rblxmath::vec3 A = rblxmath::vec3(1.0f, 2.0f, 3.0f);
		rblxmath::vec3 B = A.xyz();
		rblxmath::vec3 C(A.xyz());
		rblxmath::vec3 D(A.xyz());
		rblxmath::vec3 E(A.x, A.yz());
		rblxmath::vec3 F(A.x, A.yz());
		rblxmath::vec3 G(A.xy(), A.z);
		rblxmath::vec3 H(A.xy(), A.z);

		Error += rblxmath::all(rblxmath::equal(A, B, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, C, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, D, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, E, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, F, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, G, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, H, rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR || RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION

	return Error;
}

static int test_constexpr()
{
#if RBLXMATH_HAS_CONSTEXPR
	static_assert(rblxmath::vec3::length() == 3, "RBLXMATH: Failed constexpr");
	static_assert(rblxmath::vec3(1.0f).x > 0.0f, "RBLXMATH: Failed constexpr");
	static_assert(rblxmath::vec3(1.0f, -1.0f, -1.0f).x > 0.0f, "RBLXMATH: Failed constexpr");
	static_assert(rblxmath::vec3(1.0f, -1.0f, -1.0f).y < 0.0f, "RBLXMATH: Failed constexpr");
#endif

	return 0;
}

int main()
{
	int Error = 0;

	Error += test_vec3_ctor();
	Error += test_bvec3_ctor();
	Error += test_vec3_operators();
	Error += test_vec3_size();
	Error += test_operator_increment();
	Error += test_constexpr();

	Error += test_swizzle();
	Error += test_vec3_swizzle3_2();
	Error += test_vec3_swizzle3_3();
	Error += test_vec3_swizzle_partial();
	Error += test_vec3_swizzle_operators();
	Error += test_vec3_swizzle_functions();

	return Error;
}
