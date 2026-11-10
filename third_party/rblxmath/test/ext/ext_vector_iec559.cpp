#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/vector_double1.hpp>
#include <rblxmath/ext/vector_double1_precision.hpp>
#include <rblxmath/ext/vector_double2.hpp>
#include <rblxmath/ext/vector_double3.hpp>
#include <rblxmath/ext/vector_double4.hpp>
#include <rblxmath/ext/vector_float1.hpp>
#include <rblxmath/ext/vector_float1_precision.hpp>
#include <rblxmath/ext/vector_float2.hpp>
#include <rblxmath/ext/vector_float3.hpp>
#include <rblxmath/ext/vector_float4.hpp>

template <typename genType>
static int test_operators()
{
	typedef typename genType::value_type valType;
	
	int Error = 0;

	{
		genType const A(1);
		genType const B(1);

		genType const C = A + B;
		Error += rblxmath::all(rblxmath::equal(C, genType(2), rblxmath::epsilon<valType>())) ? 0 : 1;

		genType const D = A - B;
		Error += rblxmath::all(rblxmath::equal(D, genType(0), rblxmath::epsilon<valType>())) ? 0 : 1;

		genType const E = A * B;
		Error += rblxmath::all(rblxmath::equal(E, genType(1), rblxmath::epsilon<valType>())) ? 0 : 1;

		genType const F = A / B;
		Error += rblxmath::all(rblxmath::equal(F, genType(1), rblxmath::epsilon<valType>())) ? 0 : 1;
	}

	return Error;
}

template <typename genType>
static int test_ctor()
{
	typedef typename genType::value_type T;
	
	int Error = 0;

	rblxmath::vec<1, T> const A = genType(1);

	rblxmath::vec<1, T> const E(genType(1));
	Error += rblxmath::all(rblxmath::equal(A, E, rblxmath::epsilon<T>())) ? 0 : 1;

	rblxmath::vec<1, T> const F(E);
	Error += rblxmath::all(rblxmath::equal(A, F, rblxmath::epsilon<T>())) ? 0 : 1;

	genType const B = genType(1);
	genType const G(rblxmath::vec<2, T>(1));
	Error += rblxmath::all(rblxmath::equal(B, G, rblxmath::epsilon<T>())) ? 0 : 1;

	genType const H(rblxmath::vec<3, T>(1));
	Error += rblxmath::all(rblxmath::equal(B, H, rblxmath::epsilon<T>())) ? 0 : 1;

	genType const I(rblxmath::vec<4, T>(1));
	Error += rblxmath::all(rblxmath::equal(B, I, rblxmath::epsilon<T>())) ? 0 : 1;

	return Error;
}

template <typename genType>
static int test_size()
{
	typedef typename genType::value_type T;
	
	int Error = 0;

	Error += sizeof(rblxmath::vec<1, T>) == sizeof(genType) ? 0 : 1;
	Error += genType().length() == 1 ? 0 : 1;
	Error += genType::length() == 1 ? 0 : 1;

	return Error;
}

template <typename genType>
static int test_relational()
{
	typedef typename genType::value_type valType;
	
	int Error = 0;

	genType const A(1);
	genType const B(1);
	genType const C(0);

	Error += all(equal(A, B, rblxmath::epsilon<valType>())) ? 0 : 1;
	Error += any(notEqual(A, C, rblxmath::epsilon<valType>())) ? 0 : 1;

	return Error;
}

template <typename genType>
static int test_constexpr()
{
#	if RBLXMATH_CONFIG_CONSTEXP == RBLXMATH_ENABLE
		static_assert(genType::length() == 1, "RBLXMATH: Failed constexpr");
#	endif

	return 0;
}

int main()
{
	int Error = 0;

	Error += test_operators<rblxmath::dvec1>();
	Error += test_operators<rblxmath::lowp_dvec1>();
	Error += test_operators<rblxmath::mediump_dvec1>();
	Error += test_operators<rblxmath::highp_dvec1>();

	Error += test_ctor<rblxmath::dvec1>();
	Error += test_ctor<rblxmath::lowp_dvec1>();
	Error += test_ctor<rblxmath::mediump_dvec1>();
	Error += test_ctor<rblxmath::highp_dvec1>();

	Error += test_size<rblxmath::dvec1>();
	Error += test_size<rblxmath::lowp_dvec1>();
	Error += test_size<rblxmath::mediump_dvec1>();
	Error += test_size<rblxmath::highp_dvec1>();

	Error += test_relational<rblxmath::dvec1>();
	Error += test_relational<rblxmath::lowp_dvec1>();
	Error += test_relational<rblxmath::mediump_dvec1>();
	Error += test_relational<rblxmath::highp_dvec1>();

	Error += test_constexpr<rblxmath::dvec1>();
	Error += test_constexpr<rblxmath::lowp_dvec1>();
	Error += test_constexpr<rblxmath::mediump_dvec1>();
	Error += test_constexpr<rblxmath::highp_dvec1>();

	Error += test_operators<rblxmath::vec1>();
	Error += test_operators<rblxmath::lowp_vec1>();
	Error += test_operators<rblxmath::mediump_vec1>();
	Error += test_operators<rblxmath::highp_vec1>();
	
	Error += test_ctor<rblxmath::vec1>();
	Error += test_ctor<rblxmath::lowp_vec1>();
	Error += test_ctor<rblxmath::mediump_vec1>();
	Error += test_ctor<rblxmath::highp_vec1>();
	
	Error += test_size<rblxmath::vec1>();
	Error += test_size<rblxmath::lowp_vec1>();
	Error += test_size<rblxmath::mediump_vec1>();
	Error += test_size<rblxmath::highp_vec1>();
	
	Error += test_relational<rblxmath::vec1>();
	Error += test_relational<rblxmath::lowp_vec1>();
	Error += test_relational<rblxmath::mediump_vec1>();
	Error += test_relational<rblxmath::highp_vec1>();
	
	Error += test_constexpr<rblxmath::vec1>();
	Error += test_constexpr<rblxmath::lowp_vec1>();
	Error += test_constexpr<rblxmath::mediump_vec1>();
	Error += test_constexpr<rblxmath::highp_vec1>();
	
	return Error;
}
