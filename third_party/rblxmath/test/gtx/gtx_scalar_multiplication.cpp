#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/rblxmath.hpp>

#if RBLXMATH_HAS_TEMPLATE_ALIASES && !(RBLXMATH_COMPILER & RBLXMATH_COMPILER_GCC)

#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/scalar_multiplication.hpp>

int main()
{
	int Error(0);
	rblxmath::vec3 v(0.5, 3.1, -9.1);

	Error += rblxmath::all(rblxmath::equal(v, 1.0 * v, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(v, 1 * v, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(v, 1u * v, rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::mat3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
	rblxmath::vec3 w = 0.5f * m * v;

	Error += rblxmath::all(rblxmath::equal((m*v)/2, w, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(m*(v/2), w, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal((m/2)*v, w, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal((0.5*m)*v, w, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(0.5*(m*v), w, rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

#else

int main()
{
	return 0;
}

#endif
