#include <rblxmath/rblxmath.hpp>

#if RBLXMATH_CONFIG_DEFAULTED_FUNCTIONS == RBLXMATH_ENABLE

#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/gtc/quaternion.hpp>
#include <rblxmath/gtc/vec1.hpp>
#include <rblxmath/ext/matrix_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <cstring>

static int test_vec_memcpy()
{
	int Error = 0;

	{
		rblxmath::ivec1 const A = rblxmath::ivec1(76);
		rblxmath::ivec1 B;
		std::memcpy(&B, &A, sizeof(rblxmath::ivec1));
		Error += B == A ? 0 : 1;
	}

	{
		rblxmath::ivec2 const A = rblxmath::ivec2(76);
		rblxmath::ivec2 B;
		std::memcpy(&B, &A, sizeof(rblxmath::ivec2));
		Error += B == A ? 0 : 1;
	}

	{
		rblxmath::ivec3 const A = rblxmath::ivec3(76);
		rblxmath::ivec3 B;
		std::memcpy(&B, &A, sizeof(rblxmath::ivec3));
		Error += B == A ? 0 : 1;
	}

	{
		rblxmath::ivec4 const A = rblxmath::ivec4(76);
		rblxmath::ivec4 B;
		std::memcpy(&B, &A, sizeof(rblxmath::ivec4));
		Error += B == A ? 0 : 1;
	}

	return Error;
}

static int test_mat_memcpy()
{
	int Error = 0;

	{
		rblxmath::mat2x2 const A = rblxmath::mat2x2(76);
		rblxmath::mat2x2 B;
		std::memcpy(&B, &A, sizeof(rblxmath::mat2x2));
		Error += rblxmath::all(rblxmath::equal(B, A, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::mat2x3 const A = rblxmath::mat2x3(76);
		rblxmath::mat2x3 B;
		std::memcpy(&B, &A, sizeof(rblxmath::mat2x3));
		Error += rblxmath::all(rblxmath::equal(B, A, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::mat2x4 const A = rblxmath::mat2x4(76);
		rblxmath::mat2x4 B;
		std::memcpy(&B, &A, sizeof(rblxmath::mat2x4));
		Error += rblxmath::all(rblxmath::equal(B, A, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::mat3x2 const A = rblxmath::mat3x2(76);
		rblxmath::mat3x2 B;
		std::memcpy(&B, &A, sizeof(rblxmath::mat3x2));
		Error += rblxmath::all(rblxmath::equal(B, A, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::mat3x3 const A = rblxmath::mat3x3(76);
		rblxmath::mat3x3 B;
		std::memcpy(&B, &A, sizeof(rblxmath::mat3x3));
		Error += rblxmath::all(rblxmath::equal(B, A, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::mat3x4 const A = rblxmath::mat3x4(76);
		rblxmath::mat3x4 B;
		std::memcpy(&B, &A, sizeof(rblxmath::mat3x4));
		Error += rblxmath::all(rblxmath::equal(B, A, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::mat4x2 const A = rblxmath::mat4x2(76);
		rblxmath::mat4x2 B;
		std::memcpy(&B, &A, sizeof(rblxmath::mat4x2));
		Error += rblxmath::all(rblxmath::equal(B, A, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::mat4x3 const A = rblxmath::mat4x3(76);
		rblxmath::mat4x3 B;
		std::memcpy(&B, &A, sizeof(rblxmath::mat4x3));
		Error += rblxmath::all(rblxmath::equal(B, A, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::mat4x4 const A = rblxmath::mat4x4(76);
		rblxmath::mat4x4 B;
		std::memcpy(&B, &A, sizeof(rblxmath::mat4x4));
		Error += rblxmath::all(rblxmath::equal(B, A, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	return Error;
}

static int test_quat_memcpy()
{
	int Error = 0;

	{
		rblxmath::quat const A = rblxmath::quat(1, 0, 0, 0);
		rblxmath::quat B;
		std::memcpy(&B, &A, sizeof(rblxmath::quat));
		Error += rblxmath::all(rblxmath::equal(B, A, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	return Error;
}

#endif//RBLXMATH_CONFIG_DEFAULTED_FUNCTIONS == RBLXMATH_ENABLE

int main()
{
	int Error = 0;

#	if RBLXMATH_CONFIG_DEFAULTED_FUNCTIONS == RBLXMATH_ENABLE
		Error += test_vec_memcpy();
		Error += test_mat_memcpy();
		Error += test_quat_memcpy();
#	endif//RBLXMATH_CONFIG_DEFAULTED_FUNCTIONS == RBLXMATH_ENABLE

	return Error;
}

