#include <rblxmath/ext/scalar_reciprocal.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/scalar_constants.hpp>

static int test_sec()
{
	int Error = 0;
	
	Error += rblxmath::equal(rblxmath::sec(0.0), 1.0, 0.01) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::sec(rblxmath::pi<double>() * 2.0), 1.0, 0.01) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::sec(rblxmath::pi<double>() * -2.0), 1.0, 0.01) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::sec(rblxmath::pi<double>() * 1.0), -1.0, 0.01) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::sec(rblxmath::pi<double>() * -1.0), -1.0, 0.01) ? 0 : 1;

	return Error;
}

static int test_csc()
{
	int Error = 0;
	
	double const a = rblxmath::csc(rblxmath::pi<double>() * 0.5);
	Error += rblxmath::equal(a, 1.0, 0.01) ? 0 : 1;
	double const b = rblxmath::csc(rblxmath::pi<double>() * -0.5);
	Error += rblxmath::equal(b, -1.0, 0.01) ? 0 : 1;

	return Error;
}

static int test_cot()
{
	int Error = 0;
	
	double const a = rblxmath::cot(rblxmath::pi<double>() * 0.5);
	Error += rblxmath::equal(a, 0.0, 0.01) ? 0 : 1;
	double const b = rblxmath::cot(rblxmath::pi<double>() * -0.5);
	Error += rblxmath::equal(b, 0.0, 0.01) ? 0 : 1;

	return Error;
}

static int test_asec()
{
	int Error = 0;
	
	Error += rblxmath::equal(rblxmath::asec(100000.0), rblxmath::pi<double>() * 0.5, 0.01) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::asec(-100000.0), rblxmath::pi<double>() * 0.5, 0.01) ? 0 : 1;

	return Error;
}

static int test_acsc()
{
	int Error = 0;
	
	Error += rblxmath::equal(rblxmath::acsc(100000.0), 0.0, 0.01) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::acsc(-100000.0), 0.0, 0.01) ? 0 : 1;

	return Error;
}

static int test_acot()
{
	int Error = 0;
	
	Error += rblxmath::equal(rblxmath::acot(100000.0), 0.0, 0.01) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::acot(-100000.0), rblxmath::pi<double>(), 0.01) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::acot(0.0), rblxmath::pi<double>() * 0.5, 0.01) ? 0 : 1;

	return Error;
}

static int test_sech()
{
	int Error = 0;
	
	Error += rblxmath::equal(rblxmath::sech(100000.0), 0.0, 0.01) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::sech(-100000.0), 0.0, 0.01) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::sech(0.0), 1.0, 0.01) ? 0 : 1;

	return Error;
}

static int test_csch()
{
	int Error = 0;
	
	Error += rblxmath::equal(rblxmath::csch(100000.0), 0.0, 0.01) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::csch(-100000.0), 0.0, 0.01) ? 0 : 1;

	return Error;
}

static int test_coth()
{
	int Error = 0;
	
	double const a = rblxmath::coth(100.0);
	Error += rblxmath::equal(a, 1.0, 0.01) ? 0 : 1;
	
	double const b = rblxmath::coth(-100.0);
	Error += rblxmath::equal(b, -1.0, 0.01) ? 0 : 1;

	return Error;
}

static int test_asech()
{
	int Error = 0;
	
	double const a = rblxmath::asech(1.0);
	Error += rblxmath::equal(a, 0.0, 0.01) ? 0 : 1;

	return Error;
}

static int test_acsch()
{
	int Error = 0;
	
	Error += rblxmath::acsch(0.01) > 1.0 ? 0 : 1;
	Error += rblxmath::acsch(-0.01) < -1.0 ? 0 : 1;

	Error += rblxmath::equal(rblxmath::acsch(100.0), 0.0, 0.01) ? 0 : 1;
	Error += rblxmath::equal(rblxmath::acsch(-100.0), 0.0, 0.01) ? 0 : 1;

	return Error;
}

static int test_acoth()
{
	int Error = 0;
	
	double const a = rblxmath::acoth(1.00001);
	Error += a > 6.0 ? 0 : 1;
	
	double const b = rblxmath::acoth(-1.00001);
	Error += b < -6.0 ? 0 : 1;

	double const c = rblxmath::acoth(10000.0);
	Error += rblxmath::equal(c, 0.0, 0.01) ? 0 : 1;
	
	double const d = rblxmath::acoth(-10000.0);
	Error += rblxmath::equal(d, 0.0, 0.01) ? 0 : 1;

	return Error;
}


int main()
{
	int Error = 0;
	
	Error += test_sec();
	Error += test_csc();
	Error += test_cot();

	Error += test_asec();
	Error += test_acsc();
	Error += test_acot();

	Error += test_sech();
	Error += test_csch();
	Error += test_coth();

	Error += test_asech();
	Error += test_acsch();
	Error += test_acoth();

	return Error;
}
