#include <rblxmath/vec2.hpp>
#include <rblxmath/vec3.hpp>
#include <rblxmath/vec4.hpp>
#include <rblxmath/vector_relational.hpp>
#include <rblxmath/gtc/vec1.hpp>

static int test_not()
{
	int Error = 0;

	{
		rblxmath::bvec1 v(false);
		Error += rblxmath::all(rblxmath::not_(v)) ? 0 : 1;
	}

	{
		rblxmath::bvec2 v(false);
		Error += rblxmath::all(rblxmath::not_(v)) ? 0 : 1;
	}

	{
		rblxmath::bvec3 v(false);
		Error += rblxmath::all(rblxmath::not_(v)) ? 0 : 1;
	}
	
	{
		rblxmath::bvec4 v(false);
		Error += rblxmath::all(rblxmath::not_(v)) ? 0 : 1;
	}

	return Error;
}

static int test_less()
{
	int Error = 0;

	{
		rblxmath::vec2 const A(1, 2);
		rblxmath::vec2 const B(2, 3);
		Error += rblxmath::all(rblxmath::lessThan(A, B)) ? 0: 1;
		Error += rblxmath::all(rblxmath::lessThanEqual(A, B)) ? 0: 1;
	}

	{
		rblxmath::vec3 const A(1, 2, 3);
		rblxmath::vec3 const B(2, 3, 4);
		Error += rblxmath::all(rblxmath::lessThan(A, B)) ? 0: 1;
		Error += rblxmath::all(rblxmath::lessThanEqual(A, B)) ? 0: 1;
	}

	{
		rblxmath::vec4 const A(1, 2, 3, 4);
		rblxmath::vec4 const B(2, 3, 4, 5);
		Error += rblxmath::all(rblxmath::lessThan(A, B)) ? 0: 1;
		Error += rblxmath::all(rblxmath::lessThanEqual(A, B)) ? 0: 1;
	}

	{
		rblxmath::ivec2 const A(1, 2);
		rblxmath::ivec2 const B(2, 3);
		Error += rblxmath::all(rblxmath::lessThan(A, B)) ? 0: 1;

		rblxmath::ivec2 const C(1, 3);
		Error += rblxmath::all(rblxmath::lessThanEqual(A, C)) ? 0: 1;
	}

	{
		rblxmath::ivec3 const A(1, 2, 3);
		rblxmath::ivec3 const B(2, 3, 4);
		Error += rblxmath::all(rblxmath::lessThan(A, B)) ? 0: 1;

		rblxmath::ivec3 const C(1, 3, 4);
		Error += rblxmath::all(rblxmath::lessThanEqual(A, C)) ? 0: 1;
	}

	{
		rblxmath::ivec4 const A(1, 2, 3, 4);
		rblxmath::ivec4 const B(2, 3, 4, 5);
		Error += rblxmath::all(rblxmath::lessThan(A, B)) ? 0: 1;

		rblxmath::ivec4 const C(1, 3, 4, 5);
		Error += rblxmath::all(rblxmath::lessThanEqual(A, C)) ? 0: 1;
	}

	return Error;
}

static int test_greater()
{
	int Error = 0;

	{
		rblxmath::vec2 const A(1, 2);
		rblxmath::vec2 const B(2, 3);
		Error += rblxmath::all(rblxmath::greaterThan(B, A)) ? 0: 1;
		Error += rblxmath::all(rblxmath::greaterThanEqual(B, A)) ? 0: 1;
	}

	{
		rblxmath::vec3 const A(1, 2, 3);
		rblxmath::vec3 const B(2, 3, 4);
		Error += rblxmath::all(rblxmath::greaterThan(B, A)) ? 0: 1;
		Error += rblxmath::all(rblxmath::greaterThanEqual(B, A)) ? 0: 1;
	}

	{
		rblxmath::vec4 const A(1, 2, 3, 4);
		rblxmath::vec4 const B(2, 3, 4, 5);
		Error += rblxmath::all(rblxmath::greaterThan(B, A)) ? 0: 1;
		Error += rblxmath::all(rblxmath::greaterThanEqual(B, A)) ? 0: 1;
	}

	{
		rblxmath::ivec2 const A(1, 2);
		rblxmath::ivec2 const B(2, 3);
		Error += rblxmath::all(rblxmath::greaterThan(B, A)) ? 0: 1;

		rblxmath::ivec2 const C(1, 3);
		Error += rblxmath::all(rblxmath::greaterThanEqual(C, A)) ? 0: 1;
	}

	{
		rblxmath::ivec3 const A(1, 2, 3);
		rblxmath::ivec3 const B(2, 3, 4);
		Error += rblxmath::all(rblxmath::greaterThan(B, A)) ? 0: 1;

		rblxmath::ivec3 const C(1, 3, 4);
		Error += rblxmath::all(rblxmath::greaterThanEqual(C, A)) ? 0: 1;
	}

	{
		rblxmath::ivec4 const A(1, 2, 3, 4);
		rblxmath::ivec4 const B(2, 3, 4, 5);
		Error += rblxmath::all(rblxmath::greaterThan(B, A)) ? 0: 1;

		rblxmath::ivec4 const C(1, 3, 4, 5);
		Error += rblxmath::all(rblxmath::greaterThanEqual(C, A)) ? 0: 1;
	}

	return Error;
}

static int test_equal()
{
	int Error = 0;

	{
		rblxmath::ivec2 const A(1, 2);
		rblxmath::ivec2 const B(1, 2);
		Error += rblxmath::all(rblxmath::equal(B, A)) ? 0: 1;
	}

	{
		rblxmath::ivec3 const A(1, 2, 3);
		rblxmath::ivec3 const B(1, 2, 3);
		Error += rblxmath::all(rblxmath::equal(B, A)) ? 0: 1;
	}

	{
		rblxmath::ivec4 const A(1, 2, 3, 4);
		rblxmath::ivec4 const B(1, 2, 3, 4);
		Error += rblxmath::all(rblxmath::equal(B, A)) ? 0: 1;
	}

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_not();
	Error += test_less();
	Error += test_greater();
	Error += test_equal();

	return Error;
}

