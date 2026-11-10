#include <rblxmath/ext/vector_reciprocal.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/scalar_constants.hpp>
#include <rblxmath/ext/vector_double1.hpp>

static int test_sec()
{
	int Error = 0;
	
	rblxmath::dvec1 const a = rblxmath::sec(rblxmath::dvec1(0.0));
	Error += rblxmath::all(rblxmath::equal(a, rblxmath::dvec1(1.0), 0.01)) ? 0 : 1;

	rblxmath::dvec1 const b = rblxmath::sec(rblxmath::dvec1(rblxmath::pi<double>() * 2.0));
	Error += rblxmath::all(rblxmath::equal(b, rblxmath::dvec1(1.0), 0.01)) ? 0 : 1;

	rblxmath::dvec1 const c = rblxmath::sec(rblxmath::dvec1(rblxmath::pi<double>() * -2.0));
	Error += rblxmath::all(rblxmath::equal(c, rblxmath::dvec1(1.0), 0.01)) ? 0 : 1;

	rblxmath::dvec1 const d = rblxmath::sec(rblxmath::dvec1(rblxmath::pi<double>() * 1.0));
	Error += rblxmath::all(rblxmath::equal(d, -rblxmath::dvec1(1.0), 0.01)) ? 0 : 1;

	rblxmath::dvec1 const e = rblxmath::sec(rblxmath::dvec1(rblxmath::pi<double>() * -1.0));
	Error += rblxmath::all(rblxmath::equal(e, -rblxmath::dvec1(1.0), 0.01)) ? 0 : 1;

	return Error;
}

static int test_csc()
{
	int Error = 0;
	
	rblxmath::dvec1 const a = rblxmath::csc(rblxmath::dvec1(rblxmath::pi<double>() * 0.5));
	Error += rblxmath::all(rblxmath::equal(a, rblxmath::dvec1(1.0), 0.01)) ? 0 : 1;

	rblxmath::dvec1 const b = rblxmath::csc(rblxmath::dvec1(rblxmath::pi<double>() * -0.5));
	Error += rblxmath::all(rblxmath::equal(b, rblxmath::dvec1(-1.0), 0.01)) ? 0 : 1;

	return Error;
}

static int test_cot()
{
	int Error = 0;
	
	rblxmath::dvec1 const a = rblxmath::cot(rblxmath::dvec1(rblxmath::pi<double>() * 0.5));
	Error += rblxmath::all(rblxmath::equal(a, rblxmath::dvec1(0.0), 0.01)) ? 0 : 1;

	rblxmath::dvec1 const b = rblxmath::cot(rblxmath::dvec1(rblxmath::pi<double>() * -0.5));
	Error += rblxmath::all(rblxmath::equal(b, rblxmath::dvec1(0.0), 0.01)) ? 0 : 1;

	return Error;
}

static int test_asec()
{
	int Error = 0;
	
	Error += rblxmath::all(rblxmath::equal(rblxmath::asec(rblxmath::dvec1(100000.0)), rblxmath::dvec1(rblxmath::pi<double>() * 0.5), 0.01)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::asec(rblxmath::dvec1(-100000.0)), rblxmath::dvec1(rblxmath::pi<double>() * 0.5), 0.01)) ? 0 : 1;

	return Error;
}

static int test_acsc()
{
	int Error = 0;
	
	Error += rblxmath::all(rblxmath::equal(rblxmath::acsc(rblxmath::dvec1(100000.0)), rblxmath::dvec1(0.0), 0.01)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::acsc(rblxmath::dvec1(-100000.0)), rblxmath::dvec1(0.0), 0.01)) ? 0 : 1;

	return Error;
}

static int test_acot()
{
	int Error = 0;
	
	Error += rblxmath::all(rblxmath::equal(rblxmath::acot(rblxmath::dvec1(100000.0)), rblxmath::dvec1(0.0), 0.01)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::acot(rblxmath::dvec1(-100000.0)), rblxmath::dvec1(rblxmath::pi<double>()), 0.01)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::acot(rblxmath::dvec1(0.0)), rblxmath::dvec1(rblxmath::pi<double>() * 0.5), 0.01)) ? 0 : 1;

	return Error;
}

static int test_sech()
{
	int Error = 0;
	
	Error += rblxmath::all(rblxmath::equal(rblxmath::sech(rblxmath::dvec1(100000.0)), rblxmath::dvec1(0.0), 0.01)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::sech(rblxmath::dvec1(-100000.0)), rblxmath::dvec1(0.0), 0.01)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::sech(rblxmath::dvec1(0.0)), rblxmath::dvec1(1.0), 0.01)) ? 0 : 1;

	return Error;
}

static int test_csch()
{
	int Error = 0;
	
	Error += rblxmath::all(rblxmath::equal(rblxmath::csch(rblxmath::dvec1(100000.0)), rblxmath::dvec1(0.0), 0.01)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::csch(rblxmath::dvec1(-100000.0)), rblxmath::dvec1(0.0), 0.01)) ? 0 : 1;

	return Error;
}

static int test_coth()
{
	int Error = 0;
	
	rblxmath::dvec1 const a = rblxmath::coth(rblxmath::dvec1(100.0));
	Error += rblxmath::all(rblxmath::equal(a, rblxmath::dvec1(1.0), 0.01)) ? 0 : 1;
	
	rblxmath::dvec1 const b = rblxmath::coth(rblxmath::dvec1(-100.0));
	Error += rblxmath::all(rblxmath::equal(b, rblxmath::dvec1(-1.0), 0.01)) ? 0 : 1;

	return Error;
}

static int test_asech()
{
	int Error = 0;
	
	rblxmath::dvec1 const a = rblxmath::asech(rblxmath::dvec1(1.0));
	Error += rblxmath::all(rblxmath::equal(a, rblxmath::dvec1(0.0), 0.01)) ? 0 : 1;

	return Error;
}

static int test_acsch()
{
	int Error = 0;
	
	rblxmath::dvec1 const a(rblxmath::acsch(rblxmath::dvec1(0.01)));
	Error += a.x > 1.0 ? 0 : 1;

	rblxmath::dvec1 const b(rblxmath::acsch(rblxmath::dvec1(-0.01)));
	Error += b.x < -1.0 ? 0 : 1;

	Error += rblxmath::all(rblxmath::equal(rblxmath::acsch(rblxmath::dvec1(100.0)), rblxmath::dvec1(0.0), 0.01)) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(rblxmath::acsch(rblxmath::dvec1(-100.0)), rblxmath::dvec1(0.0), 0.01)) ? 0 : 1;

	return Error;
}

static int test_acoth()
{
	int Error = 0;
	
	rblxmath::dvec1 const a = rblxmath::acoth(rblxmath::dvec1(1.00001));
	Error += a.x > 6.0 ? 0 : 1;
	
	rblxmath::dvec1 const b = rblxmath::acoth(rblxmath::dvec1(-1.00001));
	Error += b.x < -6.0 ? 0 : 1;

	rblxmath::dvec1 const c = rblxmath::acoth(rblxmath::dvec1(10000.0));
	Error += rblxmath::all(rblxmath::equal(c, rblxmath::dvec1(0.0), 0.01)) ? 0 : 1;
	
	rblxmath::dvec1 const d = rblxmath::acoth(rblxmath::dvec1(-10000.0));
	Error += rblxmath::all(rblxmath::equal(d, rblxmath::dvec1(0.0), 0.01)) ? 0 : 1;

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
