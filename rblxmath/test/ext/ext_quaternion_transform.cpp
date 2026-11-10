#include <rblxmath/ext/quaternion_transform.hpp>
#include <rblxmath/ext/quaternion_float.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/scalar_constants.hpp>

#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/quaternion.hpp>

static int test_lookAt()
{
	int Error(0);

	rblxmath::vec3 eye(0.0f);
	rblxmath::vec3 center(1.1f, -2.0f, 3.1416f);
	rblxmath::vec3 up(-0.17f, 7.23f, -1.744f);

	rblxmath::quat test_quat = rblxmath::quatLookAt(rblxmath::normalize(center - eye), up);
	rblxmath::quat test_mat = rblxmath::conjugate(rblxmath::quat_cast(rblxmath::lookAt(eye, center, up)));

	Error += static_cast<int>(rblxmath::abs(rblxmath::length(test_quat) - 1.0f) > rblxmath::epsilon<float>());
	Error += static_cast<int>(rblxmath::min(rblxmath::length(test_quat + (-test_mat)), rblxmath::length(test_quat + test_mat)) > rblxmath::epsilon<float>());

	// Test left-handed implementation
	rblxmath::quat test_quatLH = rblxmath::quatLookAtLH(rblxmath::normalize(center - eye), up);
	rblxmath::quat test_matLH = rblxmath::conjugate(rblxmath::quat_cast(rblxmath::lookAtLH(eye, center, up)));
	Error += static_cast<int>(rblxmath::abs(rblxmath::length(test_quatLH) - 1.0f) > rblxmath::epsilon<float>());
	Error += static_cast<int>(rblxmath::min(rblxmath::length(test_quatLH - test_matLH), rblxmath::length(test_quatLH + test_matLH)) > rblxmath::epsilon<float>());
 
	// Test right-handed implementation
	rblxmath::quat test_quatRH = rblxmath::quatLookAtRH(rblxmath::normalize(center - eye), up);
	rblxmath::quat test_matRH = rblxmath::conjugate(rblxmath::quat_cast(rblxmath::lookAtRH(eye, center, up)));
	Error += static_cast<int>(rblxmath::abs(rblxmath::length(test_quatRH) - 1.0f) > rblxmath::epsilon<float>());
	Error += static_cast<int>(rblxmath::min(rblxmath::length(test_quatRH - test_matRH), rblxmath::length(test_quatRH + test_matRH)) > rblxmath::epsilon<float>());

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_lookAt();

	return Error;
}
