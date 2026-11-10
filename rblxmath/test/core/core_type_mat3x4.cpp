#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/matrix_relational.hpp>
#include <rblxmath/mat2x2.hpp>
#include <rblxmath/mat2x3.hpp>
#include <rblxmath/mat2x4.hpp>
#include <rblxmath/mat3x2.hpp>
#include <rblxmath/mat3x3.hpp>
#include <rblxmath/mat3x4.hpp>
#include <rblxmath/mat4x2.hpp>
#include <rblxmath/mat4x3.hpp>
#include <rblxmath/mat4x4.hpp>
#include <vector>

static int test_operators()
{
	int Error = 0;

	rblxmath::mat3x4 l(1.0f);
	rblxmath::mat3x4 m(1.0f);
	rblxmath::vec3 u(1.0f);
	rblxmath::vec4 v(1.0f);

	float x = 1.0f;
	rblxmath::vec4 a = m * u;
	Error += rblxmath::all(rblxmath::equal(a, rblxmath::vec4(u, 0.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::vec3 b = v * m;
	Error += rblxmath::all(rblxmath::equal(b, rblxmath::vec3(1.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::mat3x4 n0(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	rblxmath::mat3x4 n = x / n0;
	Error += rblxmath::all(rblxmath::equal(n, n0, rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::mat3x4 o = m / x;
	Error += rblxmath::all(rblxmath::equal(o, m, rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::mat3x4 p = x * m;
	Error += rblxmath::all(rblxmath::equal(p, m, rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::mat3x4 q = m * x;
	bool R = rblxmath::any(rblxmath::notEqual(m, q, rblxmath::epsilon<float>()));
	bool S = rblxmath::all(rblxmath::equal(m, l, rblxmath::epsilon<float>()));

	Error += (S && !R) ? 0 : 1;

	return Error;
}

static int test_ctr()
{
	int Error(0);

#if(RBLXMATH_HAS_INITIALIZER_LISTS)
	rblxmath::mat3x4 m0(
		rblxmath::vec4(0, 1, 2, 3),
		rblxmath::vec4(4, 5, 6, 7),
		rblxmath::vec4(8, 9, 10, 11));

	rblxmath::mat3x4 m1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

	rblxmath::mat3x4 m2{
		{0, 1, 2, 3},
		{4, 5, 6, 7},
		{8, 9, 10, 11}};

	Error += rblxmath::all(rblxmath::equal(m0, m2, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(m1, m2, rblxmath::epsilon<float>())) ? 0 : 1;

	std::vector<rblxmath::mat3x4> v1{
		{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
		{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}
	};
	
	std::vector<rblxmath::mat3x4> v2{
		{
			{ 0, 1, 2, 3},
			{ 4, 5, 6, 7},
			{ 8, 9, 10, 11}
		},
		{
			{ 0, 1, 2, 3},
			{ 4, 5, 6, 7},
			{ 8, 9, 10, 11}
		}
	};
	
#endif//RBLXMATH_HAS_INITIALIZER_LISTS
	
	return Error;
}

namespace cast
{
	template<typename genType>
	static int entry()
	{
		int Error = 0;

		genType A(1.0f);
		rblxmath::mat3x4 B(A);
		rblxmath::mat3x4 Identity(1.0f);

		for(rblxmath::length_t i = 0, length = B.length(); i < length; ++i)
			Error += rblxmath::all(rblxmath::epsilonEqual(B[i], Identity[i], rblxmath::epsilon<float>())) ? 0 : 1;

		return Error;
	}

	static int test()
	{
		int Error = 0;
		
		Error += entry<rblxmath::mat2x2>();
		Error += entry<rblxmath::mat2x3>();
		Error += entry<rblxmath::mat2x4>();
		Error += entry<rblxmath::mat3x2>();
		Error += entry<rblxmath::mat3x3>();
		Error += entry<rblxmath::mat3x4>();
		Error += entry<rblxmath::mat4x2>();
		Error += entry<rblxmath::mat4x3>();
		Error += entry<rblxmath::mat4x4>();

		return Error;
	}
}//namespace cast

static int test_size()
{
	int Error = 0;

	Error += 48 == sizeof(rblxmath::mat3x4) ? 0 : 1;
	Error += 96 == sizeof(rblxmath::dmat3x4) ? 0 : 1;
	Error += rblxmath::mat3x4().length() == 3 ? 0 : 1;
	Error += rblxmath::dmat3x4().length() == 3 ? 0 : 1;
	Error += rblxmath::mat3x4::length() == 3 ? 0 : 1;
	Error += rblxmath::dmat3x4::length() == 3 ? 0 : 1;

	return Error;
}

static int test_constexpr()
{
	int Error = 0;

#if RBLXMATH_HAS_CONSTEXPR
	static_assert(rblxmath::mat3x4::length() == 3, "RBLXMATH: Failed constexpr");

	constexpr rblxmath::mat3x4 const Z(1.0f);
	Error += rblxmath::all(rblxmath::equal(Z, rblxmath::mat3x4(1.0f), rblxmath::epsilon<float>())) ? 0 : 1;
#endif

	return Error;
}

int main()
{
	int Error = 0;

	Error += cast::test();
	Error += test_ctr();
	Error += test_operators();
	Error += test_size();
	Error += test_constexpr();

	return Error;
}

