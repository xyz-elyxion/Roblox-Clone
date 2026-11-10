#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/ext/quaternion_relational.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/gtc/type_ptr.hpp>
#include <rblxmath/gtc/matrix_transform.hpp>
#include <rblxmath/gtx/transform.hpp>
#include <rblxmath/gtx/quaternion.hpp>
#include <rblxmath/gtx/compatibility.hpp>

static int test_quat_fastMix()
{
	int Error = 0;

	rblxmath::quat A = rblxmath::angleAxis(0.0f, rblxmath::vec3(0, 0, 1));
	rblxmath::quat B = rblxmath::angleAxis(rblxmath::pi<float>() * 0.5f, rblxmath::vec3(0, 0, 1));
	rblxmath::quat C = rblxmath::fastMix(A, B, 0.5f);
	rblxmath::quat D = rblxmath::angleAxis(rblxmath::pi<float>() * 0.25f, rblxmath::vec3(0, 0, 1));

	Error += rblxmath::epsilonEqual(C.x, D.x, 0.01f) ? 0 : 1;
	Error += rblxmath::epsilonEqual(C.y, D.y, 0.01f) ? 0 : 1;
	Error += rblxmath::epsilonEqual(C.z, D.z, 0.01f) ? 0 : 1;
	Error += rblxmath::epsilonEqual(C.w, D.w, 0.01f) ? 0 : 1;

	return Error;
}

static int test_quat_shortMix()
{
	int Error = 0;

	rblxmath::quat A = rblxmath::angleAxis(0.0f, rblxmath::vec3(0, 0, 1));
	rblxmath::quat B = rblxmath::angleAxis(rblxmath::pi<float>() * 0.5f, rblxmath::vec3(0, 0, 1));
	rblxmath::quat C = rblxmath::shortMix(A, B, 0.5f);
	rblxmath::quat D = rblxmath::angleAxis(rblxmath::pi<float>() * 0.25f, rblxmath::vec3(0, 0, 1));

	Error += rblxmath::epsilonEqual(C.x, D.x, 0.01f) ? 0 : 1;
	Error += rblxmath::epsilonEqual(C.y, D.y, 0.01f) ? 0 : 1;
	Error += rblxmath::epsilonEqual(C.z, D.z, 0.01f) ? 0 : 1;
	Error += rblxmath::epsilonEqual(C.w, D.w, 0.01f) ? 0 : 1;

	return Error;
}

static int test_orientation()
{
	int Error = 0;

	{
		rblxmath::quat q(1.0f, 0.0f, 0.0f, 1.0f);
		float p = rblxmath::roll(q);
		Error += rblxmath::epsilonEqual(p, rblxmath::pi<float>() * 0.5f, 0.0001f) ? 0 : 1;
	}

	{
		rblxmath::quat q(1.0f, 0.0f, 0.0f, 1.0f);
		float p = rblxmath::pitch(q);
		Error += rblxmath::epsilonEqual(p, 0.f, 0.0001f) ? 0 : 1;
	}

	{
		rblxmath::quat q(1.0f, 0.0f, 0.0f, 1.0f);
		float p = rblxmath::yaw(q);
		Error += rblxmath::epsilonEqual(p, 0.f, 0.0001f) ? 0 : 1;
	}

	return Error;
}

static int test_rotation()
{
	int Error = 0;

	rblxmath::vec3 v(1, 0, 0);
	rblxmath::vec3 u(0, 1, 0);

	rblxmath::quat Rotation = rblxmath::rotation(v, u);

	float Angle = rblxmath::angle(Rotation);

	Error += rblxmath::abs(Angle - rblxmath::pi<float>() * 0.5f) < rblxmath::epsilon<float>() ? 0 : 1;

	return Error;
}

static int test_log()
{
	int Error = 0;
	
	rblxmath::vec3 v(1, 0, 0);
	rblxmath::vec3 u(0, 1, 0);

	rblxmath::quat q = rblxmath::rotation(v, u);
	rblxmath::quat p = rblxmath::log(q);
	rblxmath::quat r = rblxmath::exp(p);

	Error += rblxmath::all(rblxmath::equal(q, r, 0.0001f)) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_log();
	Error += test_rotation();
	Error += test_orientation();
	Error += test_quat_fastMix();
	Error += test_quat_shortMix();

	return Error;
}
