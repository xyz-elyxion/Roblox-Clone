#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/quaternion_relational.hpp>
#include <rblxmath/ext/quaternion_float.hpp>
#include <rblxmath/ext/quaternion_float_precision.hpp>
#include <rblxmath/ext/quaternion_double.hpp>
#include <rblxmath/ext/quaternion_double_precision.hpp>
#include <rblxmath/ext/vector_float3.hpp>
#include <rblxmath/ext/vector_float3_precision.hpp>
#include <rblxmath/ext/vector_double3.hpp>
#include <rblxmath/ext/vector_double3_precision.hpp>

template <typename quaType>
static int test_equal()
{
	int Error = 0;

	quaType const Q(1, 0, 0, 0);
	quaType const P(1, 0, 0, 0);
	Error += rblxmath::all(rblxmath::equal(Q, P, rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

template <typename quaType>
static int test_notEqual()
{
	int Error = 0;

	quaType const Q(1, 0, 0, 0);
	quaType const P(1, 0, 0, 0);
	Error += rblxmath::any(rblxmath::notEqual(Q, P, rblxmath::epsilon<float>())) ? 1 : 0;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_equal<rblxmath::quat>();
	Error += test_equal<rblxmath::lowp_quat>();
	Error += test_equal<rblxmath::mediump_quat>();
	Error += test_equal<rblxmath::highp_quat>();

	Error += test_notEqual<rblxmath::quat>();
	Error += test_notEqual<rblxmath::lowp_quat>();
	Error += test_notEqual<rblxmath::mediump_quat>();
	Error += test_notEqual<rblxmath::highp_quat>();

	return Error;
}
