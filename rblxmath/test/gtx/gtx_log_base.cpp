#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/log_base.hpp>
#include <rblxmath/gtc/vec1.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/exponential.hpp>

namespace test_log
{
	static int run()
	{
		int Error = 0;

		{
			float A = rblxmath::log(10.f, 2.0f);
			float B = rblxmath::log2(10.f);
			Error += rblxmath::epsilonEqual(A, B, 0.00001f) ? 0 : 1;
		}

		{
			rblxmath::vec1 A = rblxmath::log(rblxmath::vec1(10.f), rblxmath::vec1(2.0f));
			rblxmath::vec1 B = rblxmath::log2(rblxmath::vec1(10.f));
			Error += rblxmath::all(rblxmath::epsilonEqual(A, B, rblxmath::vec1(0.00001f))) ? 0 : 1;
		}

		{
			rblxmath::vec2 A = rblxmath::log(rblxmath::vec2(10.f), rblxmath::vec2(2.0f));
			rblxmath::vec2 B = rblxmath::log2(rblxmath::vec2(10.f));
			Error += rblxmath::all(rblxmath::epsilonEqual(A, B, rblxmath::vec2(0.00001f))) ? 0 : 1;
		}

		{
			rblxmath::vec3 A = rblxmath::log(rblxmath::vec3(10.f), rblxmath::vec3(2.0f));
			rblxmath::vec3 B = rblxmath::log2(rblxmath::vec3(10.f));
			Error += rblxmath::all(rblxmath::epsilonEqual(A, B, rblxmath::vec3(0.00001f))) ? 0 : 1;
		}

		{
			rblxmath::vec4 A = rblxmath::log(rblxmath::vec4(10.f), rblxmath::vec4(2.0f));
			rblxmath::vec4 B = rblxmath::log2(rblxmath::vec4(10.f));
			Error += rblxmath::all(rblxmath::epsilonEqual(A, B, rblxmath::vec4(0.00001f))) ? 0 : 1;
		}

		return Error;
	}
}//namespace test_log

int main()
{
	int Error(0);

	Error += test_log::run();

	return Error;
}
