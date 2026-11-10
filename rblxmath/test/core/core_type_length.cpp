#include <rblxmath/rblxmath.hpp>

static int test_length_mat_non_squared()
{
	int Error = 0;

	Error += rblxmath::mat2x3().length() == 2 ? 0 : 1;
	Error += rblxmath::mat2x4().length() == 2 ? 0 : 1;
	Error += rblxmath::mat3x2().length() == 3 ? 0 : 1;
	Error += rblxmath::mat3x4().length() == 3 ? 0 : 1;
	Error += rblxmath::mat4x2().length() == 4 ? 0 : 1;
	Error += rblxmath::mat4x3().length() == 4 ? 0 : 1;
	
	Error += rblxmath::dmat2x3().length() == 2 ? 0 : 1;
	Error += rblxmath::dmat2x4().length() == 2 ? 0 : 1;
	Error += rblxmath::dmat3x2().length() == 3 ? 0 : 1;
	Error += rblxmath::dmat3x4().length() == 3 ? 0 : 1;
	Error += rblxmath::dmat4x2().length() == 4 ? 0 : 1;
	Error += rblxmath::dmat4x3().length() == 4 ? 0 : 1;
	
	return Error;
}

static int test_length_mat()
{
	int Error = 0;
	
	Error += rblxmath::mat2().length() == 2 ? 0 : 1;
	Error += rblxmath::mat3().length() == 3 ? 0 : 1;
	Error += rblxmath::mat4().length() == 4 ? 0 : 1;
	Error += rblxmath::mat2x2().length() == 2 ? 0 : 1;
	Error += rblxmath::mat3x3().length() == 3 ? 0 : 1;
	Error += rblxmath::mat4x4().length() == 4 ? 0 : 1;
	
	Error += rblxmath::dmat2().length() == 2 ? 0 : 1;
	Error += rblxmath::dmat3().length() == 3 ? 0 : 1;
	Error += rblxmath::dmat4().length() == 4 ? 0 : 1;
	Error += rblxmath::dmat2x2().length() == 2 ? 0 : 1;
	Error += rblxmath::dmat3x3().length() == 3 ? 0 : 1;
	Error += rblxmath::dmat4x4().length() == 4 ? 0 : 1;
	
	return Error;
}

static int test_length_vec()
{
	int Error = 0;
	
	Error += rblxmath::vec2().length() == 2 ? 0 : 1;
	Error += rblxmath::vec3().length() == 3 ? 0 : 1;
	Error += rblxmath::vec4().length() == 4 ? 0 : 1;

	Error += rblxmath::ivec2().length() == 2 ? 0 : 1;
	Error += rblxmath::ivec3().length() == 3 ? 0 : 1;
	Error += rblxmath::ivec4().length() == 4 ? 0 : 1;

	Error += rblxmath::uvec2().length() == 2 ? 0 : 1;
	Error += rblxmath::uvec3().length() == 3 ? 0 : 1;
	Error += rblxmath::uvec4().length() == 4 ? 0 : 1;	
	
	Error += rblxmath::dvec2().length() == 2 ? 0 : 1;
	Error += rblxmath::dvec3().length() == 3 ? 0 : 1;
	Error += rblxmath::dvec4().length() == 4 ? 0 : 1;
	
	return Error;
}

int main()
{
	int Error = 0;
	
	Error += test_length_vec();
	Error += test_length_mat();
	Error += test_length_mat_non_squared();
	
	return Error;
}

