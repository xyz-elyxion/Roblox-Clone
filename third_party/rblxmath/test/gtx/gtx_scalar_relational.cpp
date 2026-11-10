#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/scalar_relational.hpp>
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/matrix_relational.hpp>
#include <rblxmath/rblxmath.hpp>

static int test_lessThan()
{
	int Error = 0;

	Error += rblxmath::lessThan(0, 1) ? 0 : 1;
	Error += rblxmath::lessThan(1, 0) ? 1 : 0;
	Error += rblxmath::lessThan(0, 0) ? 1 : 0;
	Error += rblxmath::lessThan(1, 1) ? 1 : 0;
	Error += rblxmath::lessThan(0.0f, 1.0f) ? 0 : 1;
	Error += rblxmath::lessThan(1.0f, 0.0f) ? 1 : 0;
	Error += rblxmath::lessThan(0.0f, 0.0f) ? 1 : 0;
	Error += rblxmath::lessThan(1.0f, 1.0f) ? 1 : 0;
	Error += rblxmath::lessThan(0.0, 1.0) ? 0 : 1;
	Error += rblxmath::lessThan(1.0, 0.0) ? 1 : 0;
	Error += rblxmath::lessThan(0.0, 0.0) ? 1 : 0;
	Error += rblxmath::lessThan(1.0, 1.0) ? 1 : 0;

	return Error;
}

static int test_lessThanEqual()
{
	int Error = 0;

	Error += rblxmath::lessThanEqual(0, 1) ? 0 : 1;
	Error += rblxmath::lessThanEqual(1, 0) ? 1 : 0;
	Error += rblxmath::lessThanEqual(0, 0) ? 0 : 1;
	Error += rblxmath::lessThanEqual(1, 1) ? 0 : 1;
	Error += rblxmath::lessThanEqual(0.0f, 1.0f) ? 0 : 1;
	Error += rblxmath::lessThanEqual(1.0f, 0.0f) ? 1 : 0;
	Error += rblxmath::lessThanEqual(0.0f, 0.0f) ? 0 : 1;
	Error += rblxmath::lessThanEqual(1.0f, 1.0f) ? 0 : 1;
	Error += rblxmath::lessThanEqual(0.0, 1.0) ? 0 : 1;
	Error += rblxmath::lessThanEqual(1.0, 0.0) ? 1 : 0;
	Error += rblxmath::lessThanEqual(0.0, 0.0) ? 0 : 1;
	Error += rblxmath::lessThanEqual(1.0, 1.0) ? 0 : 1;

	return Error;
}

static int test_greaterThan()
{
	int Error = 0;

	Error += rblxmath::greaterThan(0, 1) ? 1 : 0;
	Error += rblxmath::greaterThan(1, 0) ? 0 : 1;
	Error += rblxmath::greaterThan(0, 0) ? 1 : 0;
	Error += rblxmath::greaterThan(1, 1) ? 1 : 0;
	Error += rblxmath::greaterThan(0.0f, 1.0f) ? 1 : 0;
	Error += rblxmath::greaterThan(1.0f, 0.0f) ? 0 : 1;
	Error += rblxmath::greaterThan(0.0f, 0.0f) ? 1 : 0;
	Error += rblxmath::greaterThan(1.0f, 1.0f) ? 1 : 0;
	Error += rblxmath::greaterThan(0.0, 1.0) ? 1 : 0;
	Error += rblxmath::greaterThan(1.0, 0.0) ? 0 : 1;
	Error += rblxmath::greaterThan(0.0, 0.0) ? 1 : 0;
	Error += rblxmath::greaterThan(1.0, 1.0) ? 1 : 0;

	return Error;
}

static int test_greaterThanEqual()
{
	int Error = 0;

	Error += rblxmath::greaterThanEqual(0, 1) ? 1 : 0;
	Error += rblxmath::greaterThanEqual(1, 0) ? 0 : 1;
	Error += rblxmath::greaterThanEqual(0, 0) ? 0 : 1;
	Error += rblxmath::greaterThanEqual(1, 1) ? 0 : 1;
	Error += rblxmath::greaterThanEqual(0.0f, 1.0f) ? 1 : 0;
	Error += rblxmath::greaterThanEqual(1.0f, 0.0f) ? 0 : 1;
	Error += rblxmath::greaterThanEqual(0.0f, 0.0f) ? 0 : 1;
	Error += rblxmath::greaterThanEqual(1.0f, 1.0f) ? 0 : 1;
	Error += rblxmath::greaterThanEqual(0.0, 1.0) ? 1 : 0;
	Error += rblxmath::greaterThanEqual(1.0, 0.0) ? 0 : 1;
	Error += rblxmath::greaterThanEqual(0.0, 0.0) ? 0 : 1;
	Error += rblxmath::greaterThanEqual(1.0, 1.0) ? 0 : 1;

	return Error;
}

static int test_equal()
{
	int Error = 0;

	Error += rblxmath::equal(0, 1) ? 1 : 0;
	Error += rblxmath::equal(1, 0) ? 1 : 0;
	Error += rblxmath::equal(0, 0) ? 0 : 1;
	Error += rblxmath::equal(1, 1) ? 0 : 1;
	Error += rblxmath::equal(0.0f, 1.0f, rblxmath::epsilon<float>()) ? 1 : 0;
	Error += rblxmath::equal(1.0f, 0.0f, rblxmath::epsilon<float>()) ? 1 : 0;
	Error += rblxmath::equal(0.0f, 0.0f, rblxmath::epsilon<float>()) ? 0 : 1;
	Error += rblxmath::equal(1.0f, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
	Error += rblxmath::equal(0.0, 1.0, rblxmath::epsilon<double>()) ? 1 : 0;
	Error += rblxmath::equal(1.0, 0.0, rblxmath::epsilon<double>()) ? 1 : 0;
	Error += rblxmath::equal(0.0, 0.0, rblxmath::epsilon<double>()) ? 0 : 1;
	Error += rblxmath::equal(1.0, 1.0, rblxmath::epsilon<double>()) ? 0 : 1;

	return Error;
}

static int test_notEqual()
{
	int Error = 0;

	Error += rblxmath::notEqual(0, 1) ? 0 : 1;
	Error += rblxmath::notEqual(1, 0) ? 0 : 1;
	Error += rblxmath::notEqual(0, 0) ? 1 : 0;
	Error += rblxmath::notEqual(1, 1) ? 1 : 0;
	Error += rblxmath::notEqual(0.0f, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
	Error += rblxmath::notEqual(1.0f, 0.0f, rblxmath::epsilon<float>()) ? 0 : 1;
	Error += rblxmath::notEqual(0.0f, 0.0f, rblxmath::epsilon<float>()) ? 1 : 0;
	Error += rblxmath::notEqual(1.0f, 1.0f, rblxmath::epsilon<float>()) ? 1 : 0;
	Error += rblxmath::notEqual(0.0, 1.0, rblxmath::epsilon<double>()) ? 0 : 1;
	Error += rblxmath::notEqual(1.0, 0.0, rblxmath::epsilon<double>()) ? 0 : 1;
	Error += rblxmath::notEqual(0.0, 0.0, rblxmath::epsilon<double>()) ? 1 : 0;
	Error += rblxmath::notEqual(1.0, 1.0, rblxmath::epsilon<double>()) ? 1 : 0;

	return Error;
}

static int test_any()
{
	int Error = 0;

	Error += rblxmath::any(true) ? 0 : 1;
	Error += rblxmath::any(false) ? 1 : 0;

	return Error;
}

static int test_all()
{
	int Error = 0;

	Error += rblxmath::all(true) ? 0 : 1;
	Error += rblxmath::all(false) ? 1 : 0;

	return Error;
}

static int test_not()
{
	int Error = 0;

	Error += rblxmath::not_(true) ? 1 : 0;
	Error += rblxmath::not_(false) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_lessThan();
	Error += test_lessThanEqual();
	Error += test_greaterThan();
	Error += test_greaterThanEqual();
	Error += test_equal();
	Error += test_notEqual();
	Error += test_any();
	Error += test_all();
	Error += test_not();

	return Error;
}
