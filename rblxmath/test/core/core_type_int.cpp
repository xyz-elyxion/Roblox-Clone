#include <rblxmath/rblxmath.hpp>
#include <rblxmath/ext/scalar_int_sized.hpp>

static int test_bit_operator()
{
	int Error = 0;

	rblxmath::ivec4 const a(1);
	rblxmath::ivec4 const b = ~a;
	Error += rblxmath::all(rblxmath::equal(b, rblxmath::ivec4(-2))) ? 0 : 1;

	rblxmath::int32 const c(1);
	rblxmath::int32 const d = ~c;
	Error += d == -2 ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_bit_operator();

	return Error;
}
