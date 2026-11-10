#define RBLXMATH_FORCE_SWIZZLE
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/gtc/vec1.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/vec2.hpp>
#include <vector>

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wglobal-constructors"
#	pragma clang diagnostic ignored "-Wunused-variable"
#endif

static rblxmath::vec1 g1;
static rblxmath::vec1 g2(1);

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

static int test_operators()
{
	int Error = 0;

	rblxmath::ivec1 A(1);
	rblxmath::ivec1 B(1);
	{
		bool R = A != B;
		bool S = A == B;

		Error += (S && !R) ? 0 : 1;
	}

	{
		A *= 1;
		B *= 1;
		A += 1;
		B += 1;

		bool R = A != B;
		bool S = A == B;

		Error += (S && !R) ? 0 : 1;
	}

	return Error;
}

static int test_ctor()
{
	int Error = 0;

#	if RBLXMATH_HAS_TRIVIAL_QUERIES
	//	Error += std::is_trivially_default_constructible<rblxmath::vec1>::value ? 0 : 1;
	//	Error += std::is_trivially_copy_assignable<rblxmath::vec1>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::vec1>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::dvec1>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::ivec1>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::uvec1>::value ? 0 : 1;

		Error += std::is_copy_constructible<rblxmath::vec1>::value ? 0 : 1;
#	endif

/*
#if RBLXMATH_HAS_INITIALIZER_LISTS
	{
		rblxmath::vec1 a{ 0 };
		std::vector<rblxmath::vec1> v = {
			{0.f},
			{4.f},
			{8.f}};
	}

	{
		rblxmath::dvec2 a{ 0 };
		std::vector<rblxmath::dvec1> v = {
			{0.0},
			{4.0},
			{8.0}};
	}
#endif
*/

	{
		rblxmath::vec1 A = rblxmath::vec2(2.0f);
		Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec1(2.0f), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec1 B = rblxmath::vec2(2.0f, 3.0f);
		Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec1(2.0f), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec1 C = rblxmath::vec2(2.0f, 3.0);
		Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec1(2.0f), rblxmath::epsilon<float>())) ? 0 : 1;

		//rblxmath::vec1 D = rblxmath::dvec1(2.0); // Build error TODO: What does the specification says?

		rblxmath::vec1 E(rblxmath::dvec2(2.0));
		Error += rblxmath::all(rblxmath::equal(E, rblxmath::vec1(2.0f), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec1 F(rblxmath::ivec2(2));
		Error += rblxmath::all(rblxmath::equal(F, rblxmath::vec1(2.0f), rblxmath::epsilon<float>())) ? 0 : 1;
	}

	return Error;
}

static int test_size()
{
	int Error = 0;

	Error += sizeof(rblxmath::vec1) == sizeof(rblxmath::mediump_vec1) ? 0 : 1;
	Error += 4 == sizeof(rblxmath::mediump_vec1) ? 0 : 1;
	Error += sizeof(rblxmath::dvec1) == sizeof(rblxmath::highp_dvec1) ? 0 : 1;
	Error += 8 == sizeof(rblxmath::highp_dvec1) ? 0 : 1;
	Error += rblxmath::vec1().length() == 1 ? 0 : 1;
	Error += rblxmath::dvec1().length() == 1 ? 0 : 1;
	Error += rblxmath::vec1::length() == 1 ? 0 : 1;
	Error += rblxmath::dvec1::length() == 1 ? 0 : 1;

	return Error;
}

static int test_operator_increment()
{
	int Error(0);

	rblxmath::ivec1 v0(1);
	rblxmath::ivec1 v1(v0);
	rblxmath::ivec1 v2(v0);
	rblxmath::ivec1 v3 = ++v1;
	rblxmath::ivec1 v4 = v2++;

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
		rblxmath::vec1 A = rblxmath::vec1(1.0f);
		//rblxmath::vec1 B = A.x;
		rblxmath::vec1 C(A.x);

		//Error += rblxmath::all(rblxmath::equal(A, B)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(A, C, rblxmath::epsilon<float>())) ? 0 : 1;
	}
#	endif//RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR

	return Error;
}

static int test_constexpr()
{
#if RBLXMATH_HAS_CONSTEXPR
	static_assert(rblxmath::vec1::length() == 1, "RBLXMATH: Failed constexpr");
	static_assert(rblxmath::vec1(1.0f).x > 0.0f, "RBLXMATH: Failed constexpr");
#endif

	return 0;
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
