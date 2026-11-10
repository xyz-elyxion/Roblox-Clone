#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/norm.hpp>

static int test_lMaxNorm()
{
	int Error(0);
	
	{
		float norm = rblxmath::lMaxNorm(rblxmath::vec3(-1, -2, -3));
		Error += rblxmath::epsilonEqual(norm, 3.f, 0.00001f) ? 0 : 1;
	}

	{
		float norm = rblxmath::lMaxNorm(rblxmath::vec3(2, 3, 1));
		Error += rblxmath::epsilonEqual(norm, 3.f, 0.00001f) ? 0 : 1;
	}
  
	return Error;
}

static int test_lxNorm()
{
	int Error(0);

	{
		unsigned int depth_1 = 1;
		float normA = rblxmath::lxNorm(rblxmath::vec3(2, 3, 1), depth_1);
		float normB = rblxmath::l1Norm(rblxmath::vec3(2, 3, 1));
		Error += rblxmath::epsilonEqual(normA, normB, 0.00001f) ? 0 : 1;
		Error += rblxmath::epsilonEqual(normA, 6.f, 0.00001f) ? 0 : 1;
	}

	{
		unsigned int depth_1 = 1;
		float normA = rblxmath::lxNorm(rblxmath::vec3(-1, -2, -3), depth_1);
		float normB = rblxmath::l1Norm(rblxmath::vec3(-1, -2, -3));
		Error += rblxmath::epsilonEqual(normA, normB, 0.00001f) ? 0 : 1;
		Error += rblxmath::epsilonEqual(normA, 6.f, 0.00001f) ? 0 : 1;
	}

	{
		unsigned int depth_2 = 2;
		float normA = rblxmath::lxNorm(rblxmath::vec3(2, 3, 1), depth_2);
		float normB = rblxmath::l2Norm(rblxmath::vec3(2, 3, 1));
		Error += rblxmath::epsilonEqual(normA, normB, 0.00001f) ? 0 : 1;
		Error += rblxmath::epsilonEqual(normA, 3.741657387f, 0.00001f) ? 0 : 1;
	}

	{
		unsigned int depth_2 = 2;
		float normA = rblxmath::lxNorm(rblxmath::vec3(-1, -2, -3), depth_2);
		float normB = rblxmath::l2Norm(rblxmath::vec3(-1, -2, -3));
		Error += rblxmath::epsilonEqual(normA, normB, 0.00001f) ? 0 : 1;
		Error += rblxmath::epsilonEqual(normA, 3.741657387f, 0.00001f) ? 0 : 1;
	}

	{
		unsigned int oddDepth = 3;
		float norm = rblxmath::lxNorm(rblxmath::vec3(2, 3, 1), oddDepth);
		Error += rblxmath::epsilonEqual(norm, 3.301927249f, 0.00001f) ? 0 : 1;
	}

	{
		unsigned int oddDepth = 3;
		float norm = rblxmath::lxNorm(rblxmath::vec3(-1, -2, -3), oddDepth);
		Error += rblxmath::epsilonEqual(norm, 3.301927249f, 0.00001f) ? 0 : 1;
	}

	return Error;
}

int main()
{
	int Error(0);

	Error += test_lMaxNorm();
	Error += test_lxNorm();

	return Error;
}
