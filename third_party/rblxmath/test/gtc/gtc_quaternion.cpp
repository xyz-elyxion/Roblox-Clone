#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/gtc/quaternion.hpp>
#include <rblxmath/gtc/matrix_transform.hpp>
#include <rblxmath/ext/matrix_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/rblxmath.hpp>
#include <vector>

static int test_quat_angle()
{
	int Error = 0;

	{
		rblxmath::quat Q = rblxmath::angleAxis(rblxmath::pi<float>() * 0.25f, rblxmath::vec3(0, 0, 1));
		rblxmath::quat N = rblxmath::normalize(Q);
		float L = rblxmath::length(N);
		Error += rblxmath::equal(L, 1.0f, 0.01f) ? 0 : 1;
		float A = rblxmath::angle(N);
		Error += rblxmath::equal(A, rblxmath::pi<float>() * 0.25f, 0.01f) ? 0 : 1;
	}
	{
		rblxmath::quat Q = rblxmath::angleAxis(rblxmath::pi<float>() * 0.25f, rblxmath::normalize(rblxmath::vec3(0, 1, 1)));
		rblxmath::quat N = rblxmath::normalize(Q);
		float L = rblxmath::length(N);
		Error += rblxmath::equal(L, 1.0f, 0.01f) ? 0 : 1;
		float A = rblxmath::angle(N);
		Error += rblxmath::equal(A, rblxmath::pi<float>() * 0.25f, 0.01f) ? 0 : 1;
	}
	{
		rblxmath::quat Q = rblxmath::angleAxis(rblxmath::pi<float>() * 0.25f, rblxmath::normalize(rblxmath::vec3(1, 2, 3)));
		rblxmath::quat N = rblxmath::normalize(Q);
		float L = rblxmath::length(N);
		Error += rblxmath::equal(L, 1.0f, 0.01f) ? 0 : 1;
		float A = rblxmath::angle(N);
		Error += rblxmath::equal(A, rblxmath::pi<float>() * 0.25f, 0.01f) ? 0 : 1;
	}

	return Error;
}

static int test_quat_angleAxis()
{
	int Error = 0;

	rblxmath::quat A = rblxmath::angleAxis(0.f, rblxmath::vec3(0.f, 0.f, 1.f));
	rblxmath::quat B = rblxmath::angleAxis(rblxmath::pi<float>() * 0.5f, rblxmath::vec3(0, 0, 1));
	rblxmath::quat C = rblxmath::mix(A, B, 0.5f);
	rblxmath::quat D = rblxmath::angleAxis(rblxmath::pi<float>() * 0.25f, rblxmath::vec3(0, 0, 1));

	Error += rblxmath::equal(C.x, D.x, 0.01f) ? 0 : 1;
	Error += rblxmath::equal(C.y, D.y, 0.01f) ? 0 : 1;
	Error += rblxmath::equal(C.z, D.z, 0.01f) ? 0 : 1;
	Error += rblxmath::equal(C.w, D.w, 0.01f) ? 0 : 1;

	return Error;
}

static int test_quat_mix()
{
	int Error = 0;

	rblxmath::quat A = rblxmath::angleAxis(0.f, rblxmath::vec3(0.f, 0.f, 1.f));
	rblxmath::quat B = rblxmath::angleAxis(rblxmath::pi<float>() * 0.5f, rblxmath::vec3(0, 0, 1));
	rblxmath::quat C = rblxmath::mix(A, B, 0.5f);
	rblxmath::quat D = rblxmath::angleAxis(rblxmath::pi<float>() * 0.25f, rblxmath::vec3(0, 0, 1));

	Error += rblxmath::equal(C.x, D.x, 0.01f) ? 0 : 1;
	Error += rblxmath::equal(C.y, D.y, 0.01f) ? 0 : 1;
	Error += rblxmath::equal(C.z, D.z, 0.01f) ? 0 : 1;
	Error += rblxmath::equal(C.w, D.w, 0.01f) ? 0 : 1;

	return Error;
}

static int test_quat_normalize()
{
	int Error(0);

	{
		rblxmath::quat Q = rblxmath::angleAxis(rblxmath::pi<float>() * 0.25f, rblxmath::vec3(0, 0, 1));
		rblxmath::quat N = rblxmath::normalize(Q);
		float L = rblxmath::length(N);
		Error += rblxmath::equal(L, 1.0f, 0.000001f) ? 0 : 1;
	}
	{
		rblxmath::quat Q = rblxmath::angleAxis(rblxmath::pi<float>() * 0.25f, rblxmath::vec3(0, 0, 2));
		rblxmath::quat N = rblxmath::normalize(Q);
		float L = rblxmath::length(N);
		Error += rblxmath::equal(L, 1.0f, 0.000001f) ? 0 : 1;
	}
	{
		rblxmath::quat Q = rblxmath::angleAxis(rblxmath::pi<float>() * 0.25f, rblxmath::vec3(1, 2, 3));
		rblxmath::quat N = rblxmath::normalize(Q);
		float L = rblxmath::length(N);
		Error += rblxmath::equal(L, 1.0f, 0.000001f) ? 0 : 1;
	}

	return Error;
}

static int test_quat_euler()
{
	int Error = 0;

	{
		rblxmath::quat q(1.0f, 0.0f, 0.0f, 1.0f);
		float Roll = rblxmath::roll(q);
		float Pitch = rblxmath::pitch(q);
		float Yaw = rblxmath::yaw(q);
		rblxmath::vec3 Angles = rblxmath::eulerAngles(q);
		Error += rblxmath::all(rblxmath::equal(Angles, rblxmath::vec3(Pitch, Yaw, Roll), 0.000001f)) ? 0 : 1;
	}

	{
		rblxmath::dquat q(1.0, 0.0, 0.0, 1.0);
		double Roll = rblxmath::roll(q);
		double Pitch = rblxmath::pitch(q);
		double Yaw = rblxmath::yaw(q);
		rblxmath::dvec3 Angles = rblxmath::eulerAngles(q);
		Error += rblxmath::all(rblxmath::equal(Angles, rblxmath::dvec3(Pitch, Yaw, Roll), 0.000001)) ? 0 : 1;
	}

	return Error;
}

static int test_quat_slerp()
{
	int Error = 0;

	float const Epsilon = 0.0001f;//rblxmath::epsilon<float>();

	float sqrt2 = std::sqrt(2.0f)/2.0f;
	rblxmath::quat id(static_cast<float>(1), static_cast<float>(0), static_cast<float>(0), static_cast<float>(0));
	rblxmath::quat Y90rot(sqrt2, 0.0f, sqrt2, 0.0f);

	// Testing a == 0
	// Must be id
	rblxmath::quat id2 = rblxmath::slerp(id, Y90rot, 0.0f);
	Error += rblxmath::all(rblxmath::equal(id, id2, Epsilon)) ? 0 : 1;

	// Testing a == 1
	// Must be 90 degrees rotation on Y : 0 0.7 0 0.7
	rblxmath::quat Y90rot2 = rblxmath::slerp(id, Y90rot, 1.0f);
	Error += rblxmath::all(rblxmath::equal(Y90rot, Y90rot2, Epsilon)) ? 0 : 1;

	// Testing standard, easy case
	// Must be 45 degrees rotation on Y : 0 0.38 0 0.92
	rblxmath::quat Y45rot1 = rblxmath::slerp(id, Y90rot, 0.5f);
	Error += rblxmath::all(rblxmath::equal(Y45rot1, rblxmath::quat(0.924f, 0.0f, 0.383f, 0.0f), 0.01f)) ? 0 : 1;

	// Testing reverse case
	// Must be 45 degrees rotation on Y : 0 0.38 0 0.92
	rblxmath::quat Ym45rot2 = rblxmath::slerp(Y90rot, id, 0.5f);

	// Testing against full circle around the sphere instead of shortest path
	// Must be 45 degrees rotation on Y
	// certainly not a 135 degrees rotation
	rblxmath::quat Y45rot3 = rblxmath::slerp(id , -Y90rot, 0.5f);
	float Y45angle3 = rblxmath::angle(Y45rot3);
	Error += rblxmath::equal(Y45angle3, rblxmath::pi<float>() * 0.25f, Epsilon) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(Ym45rot2, Y45rot3, Epsilon)) ? 0 : 1;

	// Same, but inverted
	// Must also be 45 degrees rotation on Y :  0 0.38 0 0.92
	// -0 -0.38 -0 -0.92 is ok too
	rblxmath::quat Y45rot4 = rblxmath::slerp(-Y90rot, id, 0.5f);
	Error += rblxmath::all(rblxmath::equal(Ym45rot2, -Y45rot4, Epsilon)) ? 0 : 1;

	// Testing q1 = q2
	// Must be 90 degrees rotation on Y : 0 0.7 0 0.7
	rblxmath::quat Y90rot3 = rblxmath::slerp(Y90rot, Y90rot, 0.5f);
	Error += rblxmath::all(rblxmath::equal(Y90rot, Y90rot3, Epsilon)) ? 0 : 1;

	// Testing 180 degrees rotation
	// Must be 90 degrees rotation on almost any axis that is on the XZ plane
	rblxmath::quat XZ90rot = rblxmath::slerp(id, -Y90rot, 0.5f);
	float XZ90angle = rblxmath::angle(XZ90rot); // Must be PI/4 = 0.78;
	Error += rblxmath::equal(XZ90angle, rblxmath::pi<float>() * 0.25f, Epsilon) ? 0 : 1;

	// Testing almost equal quaternions (this test should pass through the linear interpolation)
	// Must be 0 0.00X 0 0.99999
	rblxmath::quat almostid = rblxmath::slerp(id, rblxmath::angleAxis(0.1f, rblxmath::vec3(0.0f, 1.0f, 0.0f)), 0.5f);
	Error += rblxmath::all(rblxmath::equal(almostid, rblxmath::quat(1.0f, 0.0f, 0.0f, 0.0f), 0.1f)) ? 0 : 1;

	// Testing quaternions with opposite sign
	{
		rblxmath::quat a(-1, 0, 0, 0);

		rblxmath::quat result = rblxmath::slerp(a, id, 0.5f);

		Error += rblxmath::equal(rblxmath::pow(rblxmath::dot(id, result), 2.f), 1.f, 0.01f) ? 0 : 1;
	}

	return Error;
}

static int test_quat_slerp_spins()
{
    int Error = 0;

    float const Epsilon = 0.0001f;//rblxmath::epsilon<float>();

    float sqrt2 = std::sqrt(2.0f) / 2.0f;
    rblxmath::quat id(static_cast<float>(1), static_cast<float>(0), static_cast<float>(0), static_cast<float>(0));
    rblxmath::quat Y90rot(sqrt2, 0.0f, sqrt2, 0.0f);
    rblxmath::quat Y180rot(0.0f, 0.0f, 1.0f, 0.0f);

    // Testing a == 0, k == 1
    // Must be id
    rblxmath::quat id2 = rblxmath::slerp(id, id, 1.0f, 1);
    Error += rblxmath::all(rblxmath::equal(id, id2, Epsilon)) ? 0 : 1;

    // Testing a == 1, k == 2
    // Must be id
    rblxmath::quat id3 = rblxmath::slerp(id, id, 1.0f, 2);
    Error += rblxmath::all(rblxmath::equal(id, id3, Epsilon)) ? 0 : 1;

    // Testing a == 1, k == 1
    // Must be 90 degrees rotation on Y : 0 0.7 0 0.7
    // Negative quaternion is representing same orientation
    rblxmath::quat Y90rot2 = rblxmath::slerp(id, Y90rot, 1.0f, 1);
    Error += rblxmath::all(rblxmath::equal(Y90rot, -Y90rot2, Epsilon)) ? 0 : 1;

    // Testing a == 1, k == 2
    // Must be id
    rblxmath::quat Y90rot3 = rblxmath::slerp(id, Y90rot, 8.0f / 9.0f, 2);
    Error += rblxmath::all(rblxmath::equal(id, Y90rot3, Epsilon)) ? 0 : 1;

    // Testing a == 1, k == 1
    // Must be 90 degrees rotation on Y : 0 0.7 0 0.7
    rblxmath::quat Y90rot4 = rblxmath::slerp(id, Y90rot, 0.2f, 1);
    Error += rblxmath::all(rblxmath::equal(Y90rot, Y90rot4, Epsilon)) ? 0 : 1;

    // Testing reverse case
    // Must be 45 degrees rotation on Y : 0 0.38 0 0.92
    // Negative quaternion is representing same orientation
    rblxmath::quat Ym45rot2 = rblxmath::slerp(Y90rot, id, 0.9f, 1);
    rblxmath::quat Ym45rot3 = rblxmath::slerp(Y90rot, id, 0.5f);
    Error += rblxmath::all(rblxmath::equal(-Ym45rot2, Ym45rot3, Epsilon)) ? 0 : 1;

    // Testing against full circle around the sphere instead of shortest path
    // Must be 45 degrees rotation on Y
    // certainly not a 135 degrees rotation
    rblxmath::quat Y45rot3 = rblxmath::slerp(id, -Y90rot, 0.5f, 0);
    float Y45angle3 = rblxmath::angle(Y45rot3);
    Error += rblxmath::equal(Y45angle3, rblxmath::pi<float>() * 0.25f, Epsilon) ? 0 : 1;
    Error += rblxmath::all(rblxmath::equal(Ym45rot3, Y45rot3, Epsilon)) ? 0 : 1;

    // Same, but inverted
    // Must also be 45 degrees rotation on Y :  0 0.38 0 0.92
    // -0 -0.38 -0 -0.92 is ok too
    rblxmath::quat Y45rot4 = rblxmath::slerp(-Y90rot, id, 0.5f, 0);
    Error += rblxmath::all(rblxmath::equal(Ym45rot2, Y45rot4, Epsilon)) ? 0 : 1;

    // Testing q1 = q2 k == 2
    // Must be 90 degrees rotation on Y : 0 0.7 0 0.7
    rblxmath::quat Y90rot5 = rblxmath::slerp(Y90rot, Y90rot, 0.5f, 2);
    Error += rblxmath::all(rblxmath::equal(Y90rot, Y90rot5, Epsilon)) ? 0 : 1;

    // Testing 180 degrees rotation
    // Must be 90 degrees rotation on almost any axis that is on the XZ plane
    rblxmath::quat XZ90rot = rblxmath::slerp(id, -Y90rot, 0.5f, 1);
    float XZ90angle = rblxmath::angle(XZ90rot); // Must be PI/4 = 0.78;
    Error += rblxmath::equal(XZ90angle, rblxmath::pi<float>() * 1.25f, Epsilon) ? 0 : 1;

    // Testing rotation over long arc
    // Distance from id to 90 degrees is 270 degrees, so 2/3 of it should be 180 degrees
    // Negative quaternion is representing same orientation
    rblxmath::quat Neg90rot = rblxmath::slerp(id, Y90rot, 2.0f / 3.0f, -1);
    Error += rblxmath::all(rblxmath::equal(Y180rot, -Neg90rot, Epsilon)) ? 0 : 1;

    return Error;
}

static int test_quat_mul_vec()
{
	int Error(0);

	rblxmath::quat q = rblxmath::angleAxis(rblxmath::pi<float>() * 0.5f, rblxmath::vec3(0, 0, 1));
	rblxmath::vec3 v(1, 0, 0);
	rblxmath::vec3 u(q * v);
	rblxmath::vec3 w(u * q);

	Error += rblxmath::all(rblxmath::equal(v, w, 0.01f)) ? 0 : 1;

	return Error;
}

static int test_mul()
{
	int Error = 0;

	rblxmath::quat temp1 = rblxmath::normalize(rblxmath::quat(1.0f, rblxmath::vec3(0.0, 1.0, 0.0)));
	rblxmath::quat temp2 = rblxmath::normalize(rblxmath::quat(0.5f, rblxmath::vec3(1.0, 0.0, 0.0)));

	rblxmath::vec3 transformed0 = (temp1 * rblxmath::vec3(0.0, 1.0, 0.0) * rblxmath::inverse(temp1));
	rblxmath::vec3 temp4 = temp2 * transformed0 * rblxmath::inverse(temp2);
	Error += rblxmath::all(rblxmath::equal(temp4, rblxmath::vec3(0.0f, -0.28f, -0.96f), 0.01f)) ? 0 : 1;

	rblxmath::quat temp5 = rblxmath::normalize(temp1 * temp2);
	rblxmath::vec3 temp6 = temp5 * rblxmath::vec3(0.0, 1.0, 0.0) * rblxmath::inverse(temp5);
	Error += rblxmath::all(rblxmath::equal(temp6, rblxmath::vec3(-0.48f, 0.36f, -0.8f), 0.01f)) ? 0 : 1;

	rblxmath::quat temp7(1.0f, rblxmath::vec3(0.0, 1.0, 0.0));

	temp7 *= temp5;
	temp7 *= rblxmath::inverse(temp5);

	Error += rblxmath::any(rblxmath::notEqual(temp7, rblxmath::quat(1.0f, rblxmath::vec3(0.0, 1.0, 0.0)), rblxmath::epsilon<float>())) ? 1 : 0;

	return Error;
}

static int test_identity()
{
	int Error = 0;

	rblxmath::quat const Q = rblxmath::identity<rblxmath::quat>();

	Error += rblxmath::all(rblxmath::equal(Q, rblxmath::quat(1, 0, 0, 0), 0.0001f)) ? 0 : 1;
	Error += rblxmath::any(rblxmath::notEqual(Q, rblxmath::quat(1, 0, 0, 0), 0.0001f)) ? 1 : 0;

	rblxmath::mat4 const M = rblxmath::identity<rblxmath::mat4x4>();
	rblxmath::mat4 const N(1.0f);

	Error += rblxmath::all(rblxmath::equal(M, N, 0.0001f)) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_mul();
	Error += test_quat_mul_vec();
	Error += test_quat_angle();
	Error += test_quat_angleAxis();
	Error += test_quat_mix();
	Error += test_quat_normalize();
	Error += test_quat_euler();
	Error += test_quat_slerp();
    Error += test_quat_slerp_spins();
	Error += test_identity();

	return Error;
}
