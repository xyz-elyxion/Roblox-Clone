#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/quaternion_relational.hpp>
#include <rblxmath/ext/quaternion_float.hpp>
#include <rblxmath/ext/quaternion_float_precision.hpp>
#include <rblxmath/ext/quaternion_double.hpp>
#include <rblxmath/ext/quaternion_double_precision.hpp>
#include <rblxmath/ext/vector_float3.hpp>
#include <vector>

static int test_ctr()
{
	int Error(0);

#	if RBLXMATH_HAS_TRIVIAL_QUERIES
	//	Error += std::is_trivially_default_constructible<rblxmath::quat>::value ? 0 : 1;
	//	Error += std::is_trivially_default_constructible<rblxmath::dquat>::value ? 0 : 1;
	//	Error += std::is_trivially_copy_assignable<rblxmath::quat>::value ? 0 : 1;
	//	Error += std::is_trivially_copy_assignable<rblxmath::dquat>::value ? 0 : 1;
	Error += std::is_trivially_copyable<rblxmath::quat>::value ? 0 : 1;
	Error += std::is_trivially_copyable<rblxmath::dquat>::value ? 0 : 1;

	Error += std::is_copy_constructible<rblxmath::quat>::value ? 0 : 1;
	Error += std::is_copy_constructible<rblxmath::dquat>::value ? 0 : 1;
#	endif

#	if RBLXMATH_HAS_INITIALIZER_LISTS
	{
		rblxmath::quat A{0, 1, 2, 3};

		std::vector<rblxmath::quat> B{
			A,
			{0, 1, 2, 3}};
	}
#	endif//RBLXMATH_HAS_INITIALIZER_LISTS

	return Error;
}

static int test_two_axis_ctr()
{
	int Error = 0;

	rblxmath::quat const q1(rblxmath::vec3(1, 0, 0), rblxmath::vec3(0, 1, 0));
	rblxmath::vec3 const v1 = q1 * rblxmath::vec3(1, 0, 0);
	Error += rblxmath::all(rblxmath::equal(v1, rblxmath::vec3(0, 1, 0), 0.0001f)) ? 0 : 1;

	rblxmath::quat const q2 = q1 * q1;
	rblxmath::vec3 const v2 = q2 * rblxmath::vec3(1, 0, 0);
	Error += rblxmath::all(rblxmath::equal(v2, rblxmath::vec3(-1, 0, 0), 0.0001f)) ? 0 : 1;

	rblxmath::quat const q3(rblxmath::vec3(1, 0, 0), rblxmath::vec3(-1, 0, 0));
	rblxmath::vec3 const v3 = q3 * rblxmath::vec3(1, 0, 0);
	Error += rblxmath::all(rblxmath::equal(v3, rblxmath::vec3(-1, 0, 0), 0.0001f)) ? 0 : 1;

	rblxmath::quat const q4(rblxmath::vec3(0, 1, 0), rblxmath::vec3(0, -1, 0));
	rblxmath::vec3 const v4 = q4 * rblxmath::vec3(0, 1, 0);
	Error += rblxmath::all(rblxmath::equal(v4, rblxmath::vec3(0, -1, 0), 0.0001f)) ? 0 : 1;

	rblxmath::quat const q5(rblxmath::vec3(0, 0, 1), rblxmath::vec3(0, 0, -1));
	rblxmath::vec3 const v5 = q5 * rblxmath::vec3(0, 0, 1);
	Error += rblxmath::all(rblxmath::equal(v5, rblxmath::vec3(0, 0, -1), 0.0001f)) ? 0 : 1;

	return Error;
}

static int test_size()
{
	int Error = 0;

	std::size_t const A = sizeof(rblxmath::quat);
	Error += 16 == A ? 0 : 1;
	std::size_t const B = sizeof(rblxmath::dquat);
	Error += 32 == B ? 0 : 1;
	Error += rblxmath::quat().length() == 4 ? 0 : 1;
	Error += rblxmath::dquat().length() == 4 ? 0 : 1;
	Error += rblxmath::quat::length() == 4 ? 0 : 1;
	Error += rblxmath::dquat::length() == 4 ? 0 : 1;

	return Error;
}

static int test_precision()
{
	int Error = 0;

	Error += sizeof(rblxmath::lowp_quat) <= sizeof(rblxmath::mediump_quat) ? 0 : 1;
	Error += sizeof(rblxmath::mediump_quat) <= sizeof(rblxmath::highp_quat) ? 0 : 1;

	return Error;
}

static int test_constexpr()
{
#if RBLXMATH_HAS_CONSTEXPR
	static_assert(rblxmath::quat::length() == 4, "RBLXMATH: Failed constexpr");
	static_assert(rblxmath::quat(1.0f, rblxmath::vec3(0.0f)).w > 0.0f, "RBLXMATH: Failed constexpr");
#endif

	return 0;
}

int main()
{
	int Error = 0;

	Error += test_ctr();
	Error += test_two_axis_ctr();
	Error += test_size();
	Error += test_precision();
	Error += test_constexpr();

	return Error;
}
