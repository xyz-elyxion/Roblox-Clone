#include <rblxmath/ext/vector_bool1.hpp>
#include <rblxmath/ext/vector_bool1_precision.hpp>

template <typename genType>
static int test_operators()
{
	int Error = 0;

	genType const A(true);
	genType const B(true);
	{
		bool const R = A != B;
		bool const S = A == B;
		Error += (S && !R) ? 0 : 1;
	}

	return Error;
}

template <typename genType>
static int test_ctor()
{
	int Error = 0;

	rblxmath::bvec1 const A = genType(true);

	rblxmath::bvec1 const E(genType(true));
	Error += A == E ? 0 : 1;

	rblxmath::bvec1 const F(E);
	Error += A == F ? 0 : 1;

	return Error;
}

template <typename genType>
static int test_size()
{
	int Error = 0;

	Error += sizeof(rblxmath::bvec1) == sizeof(genType) ? 0 : 1;
	Error += genType().length() == 1 ? 0 : 1;
	Error += genType::length() == 1 ? 0 : 1;

	return Error;
}

template <typename genType>
static int test_relational()
{
	int Error = 0;

	genType const A(true);
	genType const B(true);
	genType const C(false);

	Error += A == B ? 0 : 1;
	Error += (A && B) == A ? 0 : 1;
	Error += (A || C) == A ? 0 : 1;

	return Error;
}

template <typename genType>
static int test_constexpr()
{
#	if RBLXMATH_HAS_CONSTEXPR
		static_assert(genType::length() == 1, "RBLXMATH: Failed constexpr");
#	endif

	return 0;
}

int main()
{
	int Error = 0;

	Error += test_operators<rblxmath::bvec1>();
	Error += test_operators<rblxmath::lowp_bvec1>();
	Error += test_operators<rblxmath::mediump_bvec1>();
	Error += test_operators<rblxmath::highp_bvec1>();

	Error += test_ctor<rblxmath::bvec1>();
	Error += test_ctor<rblxmath::lowp_bvec1>();
	Error += test_ctor<rblxmath::mediump_bvec1>();
	Error += test_ctor<rblxmath::highp_bvec1>();

	Error += test_size<rblxmath::bvec1>();
	Error += test_size<rblxmath::lowp_bvec1>();
	Error += test_size<rblxmath::mediump_bvec1>();
	Error += test_size<rblxmath::highp_bvec1>();

	Error += test_relational<rblxmath::bvec1>();
	Error += test_relational<rblxmath::lowp_bvec1>();
	Error += test_relational<rblxmath::mediump_bvec1>();
	Error += test_relational<rblxmath::highp_bvec1>();

	Error += test_constexpr<rblxmath::bvec1>();
	Error += test_constexpr<rblxmath::lowp_bvec1>();
	Error += test_constexpr<rblxmath::mediump_bvec1>();
	Error += test_constexpr<rblxmath::highp_bvec1>();

	return Error;
}
