#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/gtc/quaternion.hpp>
#include <rblxmath/vector_relational.hpp>

static int test_defined()
{
	int Error = 0;

	Error += rblxmath::all(rblxmath::epsilonEqual(rblxmath::vec2(0.0f), rblxmath::vec2(0.0f), rblxmath::vec2(rblxmath::epsilon<float>()))) ? 0 : 1;
	Error += rblxmath::all(rblxmath::epsilonEqual(rblxmath::vec3(0.0f), rblxmath::vec3(0.0f), rblxmath::vec3(rblxmath::epsilon<float>()))) ? 0 : 1;
	Error += rblxmath::all(rblxmath::epsilonEqual(rblxmath::vec4(0.0f), rblxmath::vec4(0.0f), rblxmath::vec4(rblxmath::epsilon<float>()))) ? 0 : 1;

	Error += rblxmath::all(rblxmath::epsilonNotEqual(rblxmath::vec2(1.0f), rblxmath::vec2(2.0f), rblxmath::vec2(rblxmath::epsilon<float>()))) ? 0 : 1;
	Error += rblxmath::all(rblxmath::epsilonNotEqual(rblxmath::vec3(1.0f), rblxmath::vec3(2.0f), rblxmath::vec3(rblxmath::epsilon<float>()))) ? 0 : 1;
	Error += rblxmath::all(rblxmath::epsilonNotEqual(rblxmath::vec4(1.0f), rblxmath::vec4(2.0f), rblxmath::vec4(rblxmath::epsilon<float>()))) ? 0 : 1;
	
	Error += rblxmath::all(rblxmath::epsilonEqual(rblxmath::vec2(0.0f), rblxmath::vec2(0.0f), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::epsilonEqual(rblxmath::vec3(0.0f), rblxmath::vec3(0.0f), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::epsilonEqual(rblxmath::vec4(0.0f), rblxmath::vec4(0.0f), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::epsilonEqual(rblxmath::quat(rblxmath::vec3(0.0f)), rblxmath::quat(rblxmath::vec3(0.0f)), rblxmath::epsilon<float>())) ? 0 : 1;

	Error += rblxmath::all(rblxmath::epsilonNotEqual(rblxmath::vec2(1.0f), rblxmath::vec2(2.0f), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::epsilonNotEqual(rblxmath::vec3(1.0f), rblxmath::vec3(2.0f), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::epsilonNotEqual(rblxmath::vec4(1.0f), rblxmath::vec4(2.0f), rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::epsilonNotEqual(rblxmath::quat(rblxmath::vec3(0.0f)), rblxmath::quat(rblxmath::vec3(1.0f)), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

template<typename T>
static int test_equal()
{
	int Error = 0;
	
	{
		T A = rblxmath::epsilon<T>();
		T B = rblxmath::epsilon<T>();
		Error += rblxmath::epsilonEqual(A, B, rblxmath::epsilon<T>() * T(2)) ? 0 : 1;
	}

	{
		T A(0);
		T B = static_cast<T>(0) + rblxmath::epsilon<T>();
		Error += rblxmath::epsilonEqual(A, B, rblxmath::epsilon<T>() * T(2)) ? 0 : 1;
	}

	{
		T A(0);
		T B = static_cast<T>(0) - rblxmath::epsilon<T>();
		Error += rblxmath::epsilonEqual(A, B, rblxmath::epsilon<T>() * T(2)) ? 0 : 1;
	}

	{
		T A = static_cast<T>(0) + rblxmath::epsilon<T>();
		T B = static_cast<T>(0);
		Error += rblxmath::epsilonEqual(A, B, rblxmath::epsilon<T>() * T(2)) ? 0 : 1;
	}

	{
		T A = static_cast<T>(0) - rblxmath::epsilon<T>();
		T B = static_cast<T>(0);
		Error += rblxmath::epsilonEqual(A, B, rblxmath::epsilon<T>() * T(2)) ? 0 : 1;
	}

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_defined();
	Error += test_equal<float>();
	Error += test_equal<double>();

	return Error;
}


