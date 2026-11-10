#define RBLXMATH_FORCE_SWIZZLE
#include <rblxmath/vector_relational.hpp>
#include <rblxmath/gtc/vec1.hpp>
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

static int test_vec1_operators()
{
	int Error(0);

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

static int test_vec1_ctor()
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


	{
		rblxmath::ivec1 A = rblxmath::vec1(2.0f);

		rblxmath::ivec1 E(rblxmath::dvec1(2.0));
		Error += A == E ? 0 : 1;

		rblxmath::ivec1 F(rblxmath::ivec1(2));
		Error += A == F ? 0 : 1;
	}

	return Error;
}

static int test_vec1_size()
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

	RBLXMATH_CONSTEXPR rblxmath::length_t Length = rblxmath::vec1::length();
	Error += Length == 1 ? 0 : 1;

	return Error;
}

static int test_vec1_operator_increment()
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

static int test_bvec1_ctor()
{
	int Error = 0;

	rblxmath::bvec1 A(true);
	rblxmath::bvec1 B(true);
	rblxmath::bvec1 C(false);
	rblxmath::bvec1 D = A && B;
	rblxmath::bvec1 E = A && C;
	rblxmath::bvec1 F = A || C;

	Error += D == rblxmath::bvec1(true) ? 0 : 1;
	Error += E == rblxmath::bvec1(false) ? 0 : 1;
	Error += F == rblxmath::bvec1(true) ? 0 : 1;

	bool G = A == C;
	bool H = A != C;
	Error += !G ? 0 : 1;
	Error += H ? 0 : 1;

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

	Error += test_vec1_size();
	Error += test_vec1_ctor();
	Error += test_bvec1_ctor();
	Error += test_vec1_operators();
	Error += test_vec1_operator_increment();
	Error += test_constexpr();
	
	return Error;
}
