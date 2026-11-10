#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/rblxmath.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/gtx/matrix_operation.hpp>
#include <limits>

static int test_adjugate()
{
	int Error = 0;

	const float epsilon = std::numeric_limits<float>::epsilon();

	// mat2
	const rblxmath::mat2 m2(
		2, 3,
		1, 5
	);

	const rblxmath::mat2 eam2(
		5, -3,
		-1, 2
	);

	const rblxmath::mat2 am2 = rblxmath::adjugate(m2);

	Error += rblxmath::all(rblxmath::bvec2(
		rblxmath::all(rblxmath::epsilonEqual(am2[0], eam2[0], epsilon)),
		rblxmath::all(rblxmath::epsilonEqual(am2[1], eam2[1], epsilon))
	)) ? 0 : 1;

	// mat3
	const rblxmath::mat3 m3(
		2, 3, 3,
		1, 5, 4,
		4, 6, 8
	);

	const rblxmath::mat3 eam3(
		16, -6, -3,
		8, 4, -5,
		-14, 0, 7
	);

	const rblxmath::mat3 am3 = rblxmath::adjugate(m3);

	Error += rblxmath::all(rblxmath::bvec3(
		rblxmath::all(rblxmath::epsilonEqual(am3[0], eam3[0], epsilon)),
		rblxmath::all(rblxmath::epsilonEqual(am3[1], eam3[1], epsilon)),
		rblxmath::all(rblxmath::epsilonEqual(am3[2], eam3[2], epsilon))
	)) ? 0 : 1;

	// mat4
	const rblxmath::mat4 m4(
		2, 3, 3, 1,
		1, 5, 4, 3,
		4, 6, 8, 5,
		-2, -3, -3, 4
	);

	const rblxmath::mat4 eam4(
		97, -30, -15, 17,
		45, 20, -25, 5,
		-91, 0, 35, -21,
		14, 0, 0, 14
	);

	const rblxmath::mat4 am4 = rblxmath::adjugate(m4);

	Error += rblxmath::all(rblxmath::bvec4(
		rblxmath::all(rblxmath::epsilonEqual(am4[0], eam4[0], epsilon)),
		rblxmath::all(rblxmath::epsilonEqual(am4[1], eam4[1], epsilon)),
		rblxmath::all(rblxmath::epsilonEqual(am4[2], eam4[2], epsilon)),
		rblxmath::all(rblxmath::epsilonEqual(am4[3], eam4[3], epsilon))
	)) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_adjugate();

	return Error;
}
