#define RBLXMATH_FORCE_INLINE
#define RBLXMATH_PRECISION_HIGHP_FLOAT
#include <rblxmath/rblxmath.hpp>
#include <rblxmath/ext.hpp>

static int test_mat()
{
	int Error = 0;

	Error += sizeof(rblxmath::mat2) == sizeof(rblxmath::highp_mat2) ? 0 : 1;
	Error += sizeof(rblxmath::mat3) == sizeof(rblxmath::highp_mat3) ? 0 : 1;
	Error += sizeof(rblxmath::mat4) == sizeof(rblxmath::highp_mat4) ? 0 : 1;

	Error += sizeof(rblxmath::mat2x2) == sizeof(rblxmath::highp_mat2x2) ? 0 : 1;
	Error += sizeof(rblxmath::mat2x3) == sizeof(rblxmath::highp_mat2x3) ? 0 : 1;
	Error += sizeof(rblxmath::mat2x4) == sizeof(rblxmath::highp_mat2x4) ? 0 : 1;
	Error += sizeof(rblxmath::mat3x2) == sizeof(rblxmath::highp_mat3x2) ? 0 : 1;
	Error += sizeof(rblxmath::mat3x3) == sizeof(rblxmath::highp_mat3x3) ? 0 : 1;
	Error += sizeof(rblxmath::mat3x4) == sizeof(rblxmath::highp_mat3x4) ? 0 : 1;
	Error += sizeof(rblxmath::mat4x2) == sizeof(rblxmath::highp_mat4x2) ? 0 : 1;
	Error += sizeof(rblxmath::mat4x3) == sizeof(rblxmath::highp_mat4x3) ? 0 : 1;
	Error += sizeof(rblxmath::mat4x4) == sizeof(rblxmath::highp_mat4x4) ? 0 : 1;

	return Error;
}

static int test_vec()
{
	int Error = 0;

	Error += sizeof(rblxmath::vec2) == sizeof(rblxmath::highp_vec2) ? 0 : 1;
	Error += sizeof(rblxmath::vec3) == sizeof(rblxmath::highp_vec3) ? 0 : 1;
	Error += sizeof(rblxmath::vec4) == sizeof(rblxmath::highp_vec4) ? 0 : 1;

	return Error;
}

static int test_dvec()
{
	int Error = 0;
	
	Error += sizeof(rblxmath::dvec2) == sizeof(rblxmath::highp_dvec2) ? 0 : 1;
	Error += sizeof(rblxmath::dvec3) == sizeof(rblxmath::highp_dvec3) ? 0 : 1;
	Error += sizeof(rblxmath::dvec4) == sizeof(rblxmath::highp_dvec4) ? 0 : 1;
	
	return Error;
}

int main()
{
	int Error = 0;

	Error += test_mat();
	Error += test_vec();
	Error += test_dvec();
	
	return Error;
}
