#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/rblxmath.hpp>
#include <rblxmath/gtx/quaternion.hpp>
#include <rblxmath/gtx/easing.hpp>

namespace
{
	template<typename T>
	static void _test_easing()
	{
		T a = static_cast<T>(0.5);
		T r;

		r = rblxmath::linearInterpolation(a);

		r = rblxmath::quadraticEaseIn(a);
		r = rblxmath::quadraticEaseOut(a);
		r = rblxmath::quadraticEaseInOut(a);

		r = rblxmath::cubicEaseIn(a);
		r = rblxmath::cubicEaseOut(a);
		r = rblxmath::cubicEaseInOut(a);

		r = rblxmath::quarticEaseIn(a);
		r = rblxmath::quarticEaseOut(a);
		r = rblxmath::quinticEaseInOut(a);

		r = rblxmath::sineEaseIn(a);
		r = rblxmath::sineEaseOut(a);
		r = rblxmath::sineEaseInOut(a);

		r = rblxmath::circularEaseIn(a);
		r = rblxmath::circularEaseOut(a);
		r = rblxmath::circularEaseInOut(a);

		r = rblxmath::exponentialEaseIn(a);
		r = rblxmath::exponentialEaseOut(a);
		r = rblxmath::exponentialEaseInOut(a);

		r = rblxmath::elasticEaseIn(a);
		r = rblxmath::elasticEaseOut(a);
		r = rblxmath::elasticEaseInOut(a);

		r = rblxmath::backEaseIn(a);
		r = rblxmath::backEaseOut(a);
		r = rblxmath::backEaseInOut(a);

		r = rblxmath::bounceEaseIn(a);
		r = rblxmath::bounceEaseOut(a);
		r = rblxmath::bounceEaseInOut(a);
	}
}

int main()
{
	int Error = 0;

	_test_easing<float>();
	_test_easing<double>();

	return Error;
}

