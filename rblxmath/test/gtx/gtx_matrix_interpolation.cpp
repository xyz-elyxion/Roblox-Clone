#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtc/quaternion.hpp>
#include <rblxmath/gtx/component_wise.hpp>
#include <rblxmath/gtx/matrix_interpolation.hpp>

#include <iostream>
#include <limits>
#include <math.h>


static int test_axisAngle()
{
	int Error = 0;

	rblxmath::mat4 m1(-0.9946f, 0.0f, -0.104531f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.104531f, 0.0f, -0.9946f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	rblxmath::mat4 m2(-0.992624f, 0.0f, -0.121874f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.121874f, 0.0f, -0.992624f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	rblxmath::mat4 const m1rot = rblxmath::extractMatrixRotation(m1);
	rblxmath::mat4 const dltRotation = m2 * rblxmath::transpose(m1rot);

	rblxmath::vec3 dltAxis(0.0f);
	float dltAngle = 0.0f;
	rblxmath::axisAngle(dltRotation, dltAxis, dltAngle);

	std::cout << "dltAxis: (" << dltAxis.x << ", " << dltAxis.y << ", " << dltAxis.z << "), dltAngle: " << dltAngle << std::endl;

	rblxmath::quat q = rblxmath::quat_cast(dltRotation);
	std::cout << "q: (" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")" << std::endl;
	float yaw = rblxmath::yaw(q);
	std::cout << "Yaw: " << yaw << std::endl;

	return Error;
}

template <class T>
static int testForAxisAngle(rblxmath::vec<3, T, rblxmath::defaultp> const axisTrue, T const angleTrue)
{
    T const eps = std::sqrt(std::numeric_limits<T>::epsilon());

    rblxmath::mat<4, 4, T, rblxmath::defaultp> const matTrue = rblxmath::axisAngleMatrix(axisTrue, angleTrue);

    rblxmath::vec<3, T, rblxmath::defaultp> axis;
    T angle;
    rblxmath::axisAngle(matTrue, axis, angle);
    rblxmath::mat<4, 4, T, rblxmath::defaultp> const matRebuilt = rblxmath::axisAngleMatrix(axis, angle);

    rblxmath::mat<4, 4, T, rblxmath::defaultp> const errMat = matTrue - matRebuilt;
    T const maxErr = rblxmath::compMax(rblxmath::vec<4, T, rblxmath::defaultp>(
            rblxmath::compMax(rblxmath::abs(errMat[0])),
            rblxmath::compMax(rblxmath::abs(errMat[1])),
            rblxmath::compMax(rblxmath::abs(errMat[2])),
            rblxmath::compMax(rblxmath::abs(errMat[3]))
        ));
    
    return maxErr < eps ? 0 : 1;
}

static int test_axisAngle2()
{
	int Error = 0;
    
    Error += testForAxisAngle(rblxmath::vec3(0.0f, 1.0f, 0.0f), 0.0f);
    Error += testForAxisAngle(rblxmath::vec3(0.358f, 0.0716f, 0.9309f), 0.00001f);
    Error += testForAxisAngle(rblxmath::vec3(1.0f, 0.0f, 0.0f), 0.0001f);
    Error += testForAxisAngle(rblxmath::vec3(0.0f, 0.0f, 1.0f), 0.001f);
    Error += testForAxisAngle(rblxmath::vec3(0.0f, 0.0f, 1.0f), 0.001f);
    Error += testForAxisAngle(rblxmath::vec3(0.0f, 1.0f, 0.0f), 0.005f);
    Error += testForAxisAngle(rblxmath::vec3(0.0f, 0.0f, 1.0f), 0.005f);
    Error += testForAxisAngle(rblxmath::vec3(0.358f, 0.0716f, 0.9309f), 0.03f);
    Error += testForAxisAngle(rblxmath::vec3(0.358f, 0.0716f, 0.9309f), 0.0003f);
    Error += testForAxisAngle(rblxmath::vec3(0.0f, 0.0f, 1.0f), 0.01f);
    Error += testForAxisAngle(rblxmath::dvec3(0.0f, 1.0f, 0.0f), 0.00005);
    Error += testForAxisAngle(rblxmath::dvec3(-1.0f, 0.0f, 0.0f), 0.000001);
    Error += testForAxisAngle(rblxmath::dvec3(0.7071f, 0.7071f, 0.0f), 0.5);
    Error += testForAxisAngle(rblxmath::dvec3(0.7071f, 0.0f, 0.7071f), 0.0002);
    Error += testForAxisAngle(rblxmath::dvec3(0.7071f, 0.0f, 0.7071f), 0.00002);
    Error += testForAxisAngle(rblxmath::dvec3(0.7071f, 0.0f, 0.7071f), 0.000002);
    Error += testForAxisAngle(rblxmath::dvec3(0.7071f, 0.0f, 0.7071f), 0.0000002);
    Error += testForAxisAngle(rblxmath::vec3(0.0f, 0.7071f, 0.7071f), 1.3f);
    Error += testForAxisAngle(rblxmath::vec3(0.0f, 0.7071f, 0.7071f), 6.3f);
    Error += testForAxisAngle(rblxmath::vec3(1.0f, 0.0f, 0.0f), -0.23456f);
    Error += testForAxisAngle(rblxmath::vec3(1.0f, 0.0f, 0.0f), rblxmath::pi<float>());
    Error += testForAxisAngle(rblxmath::vec3(0.0f, 1.0f, 0.0f), -rblxmath::pi<float>());
    Error += testForAxisAngle(rblxmath::vec3(0.358f, 0.0716f, 0.9309f), -rblxmath::pi<float>());
    Error += testForAxisAngle(rblxmath::vec3(1.0f, 0.0f, 0.0f), rblxmath::pi<float>() + 2e-6f);
    Error += testForAxisAngle(rblxmath::vec3(1.0f, 0.0f, 0.0f), rblxmath::pi<float>() + 1e-4f);
    Error += testForAxisAngle(rblxmath::vec3(0.0f, 1.0f, 0.0f), -rblxmath::pi<float>() + 1e-3f);
    Error += testForAxisAngle(rblxmath::vec3(0.358f, 0.0716f, 0.9309f), -rblxmath::pi<float>() + 5e-3f);

	return Error;
}

static int test_rotate()
{
	rblxmath::mat4 m2(1.0);
	float myAngle = 1.0f;
	m2 = rblxmath::rotate(m2, myAngle, rblxmath::vec3(1.0f, 0.0f, 0.0f));
	rblxmath::vec3 m2Axis;
	float m2Angle;
	rblxmath::axisAngle(m2, m2Axis, m2Angle);

	return 0;
}

int main()
{
	int Error = 0;

	Error += test_axisAngle();
	Error += test_axisAngle2();
	Error += test_rotate();

	return Error;
}


