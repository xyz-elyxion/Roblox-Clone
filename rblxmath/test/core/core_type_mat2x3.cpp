#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/matrix_relational.hpp>
#include <rblxmath/gtc/constants.hpp>
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

	rblxmath::mat2x3 l(1.0f);
	rblxmath::mat2x3 m(1.0f);
	rblxmath::vec2 u(1.0f);
	rblxmath::vec3 v(1.0f);

	float x = 1.0f;
	rblxmath::vec3 a = m * u;
	Error += rblxmath::all(rblxmath::equal(a, rblxmath::vec3(u, 0.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::vec2 b = v * m;
	Error += rblxmath::all(rblxmath::equal(b, rblxmath::vec2(1.0f), rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::mat2x3 n0(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	rblxmath::mat2x3 n = x / n0;
	Error += rblxmath::all(rblxmath::equal(n, n0, rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::mat2x3 o = m / x;
	Error += rblxmath::all(rblxmath::equal(o, m, rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::mat2x3 p = x * m;
	Error += rblxmath::all(rblxmath::equal(p, m, rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::mat2x3 q = m * x;
	bool R = rblxmath::any(rblxmath::notEqual(m, q, rblxmath::epsilon<float>()));
	bool S = rblxmath::all(rblxmath::equal(m, l, rblxmath::epsilon<float>()));

	Error += (S && !R) ? 0 : 1;

	return Error;
}

static int test_ctr()
{
	int Error(0);

#if RBLXMATH_HAS_INITIALIZER_LISTS
	rblxmath::mat2x3 m0(
		rblxmath::vec3(0, 1, 2),
		rblxmath::vec3(3, 4, 5));
	
	rblxmath::mat2x3 m1{0, 1, 2, 3, 4, 5};
	
	rblxmath::mat2x3 m2{
		{0, 1, 2},
		{3, 4, 5}};
	
	Error += rblxmath::all(rblxmath::equal(m0, m2, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(m1, m2, rblxmath::epsilon<float>())) ? 0 : 1;
	
	std::vector<rblxmath::mat2x3> v1{
		{0, 1, 2, 3, 4, 5},
		{0, 1, 2, 3, 4, 5}
	};
	
	std::vector<rblxmath::mat2x3> v2{
		{
			{ 0, 1, 2},
			{ 4, 5, 6}
		},
		{
			{ 0, 1, 2},
			{ 4, 5, 6}
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
		rblxmath::mat2x3 B(A);
		rblxmath::mat2x3 Identity(1.0f);

		Error += rblxmath::all(rblxmath::equal(B, Identity, rblxmath::epsilon<float>())) ? 0 : 1;

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

	Error += 24 == sizeof(rblxmath::mat2x3) ? 0 : 1;
	Error += 48 == sizeof(rblxmath::dmat2x3) ? 0 : 1;
	Error += rblxmath::mat2x3().length() == 2 ? 0 : 1;
	Error += rblxmath::dmat2x3().length() == 2 ? 0 : 1;
	Error += rblxmath::mat2x3::length() == 2 ? 0 : 1;
	Error += rblxmath::dmat2x3::length() == 2 ? 0 : 1;

	return Error;
}

static int test_constexpr()
{
	int Error = 0;

#if RBLXMATH_HAS_CONSTEXPR
	static_assert(rblxmath::mat2x3::length() == 2, "RBLXMATH: Failed constexpr");

	constexpr rblxmath::mat2x3 const Z(1.0f);
	Error += rblxmath::all(rblxmath::equal(Z, rblxmath::mat2x3(1.0f), rblxmath::epsilon<float>())) ? 0 : 1;
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
