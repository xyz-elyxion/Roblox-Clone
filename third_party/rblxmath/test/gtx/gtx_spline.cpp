#include <rblxmath/vec2.hpp>
#include <rblxmath/vec3.hpp>
#include <rblxmath/vec4.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/scalar_constants.hpp>

#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/spline.hpp>

namespace catmullRom
{
	static int test()
	{
		int Error = 0;

		rblxmath::vec2 Result2 = rblxmath::catmullRom(
			rblxmath::vec2(0.0f, 0.0f),
			rblxmath::vec2(1.0f, 0.0f),
			rblxmath::vec2(1.0f, 1.0f),
			rblxmath::vec2(0.0f, 1.0f), 0.5f);

		Error += rblxmath::all(rblxmath::equal(Result2, rblxmath::vec2(1.125f, 0.5f), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 Result3 = rblxmath::catmullRom(
			rblxmath::vec3(0.0f, 0.0f, 0.0f), 
			rblxmath::vec3(1.0f, 0.0f, 0.0f), 
			rblxmath::vec3(1.0f, 1.0f, 0.0f), 
			rblxmath::vec3(0.0f, 1.0f, 0.0f), 0.5f);

		Error += rblxmath::all(rblxmath::equal(Result3, rblxmath::vec3(1.125f, 0.5f, 0.0f), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 Result4 = rblxmath::catmullRom(
			rblxmath::vec4(0.0f, 0.0f, 0.0f, 1.0f), 
			rblxmath::vec4(1.0f, 0.0f, 0.0f, 1.0f), 
			rblxmath::vec4(1.0f, 1.0f, 0.0f, 1.0f), 
			rblxmath::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.5f);

		Error += rblxmath::all(rblxmath::equal(Result4, rblxmath::vec4(1.125f, 0.5f, 0.0f, 1.0f), rblxmath::epsilon<float>())) ? 0 : 1;

		return Error;
	}
}//catmullRom

namespace hermite
{
	static int test()
	{
		int Error = 0;

		rblxmath::vec2 Result2 = rblxmath::hermite(
			rblxmath::vec2(0.0f, 0.0f),
			rblxmath::vec2(1.0f, 0.0f),
			rblxmath::vec2(1.0f, 1.0f),
			rblxmath::vec2(0.0f, 1.0f), 0.5f);

		Error += rblxmath::all(rblxmath::equal(Result2, rblxmath::vec2(0.625f, 0.375f), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 Result3 = rblxmath::hermite(
			rblxmath::vec3(0.0f, 0.0f, 0.0f), 
			rblxmath::vec3(1.0f, 0.0f, 0.0f), 
			rblxmath::vec3(1.0f, 1.0f, 0.0f), 
			rblxmath::vec3(0.0f, 1.0f, 0.0f), 0.5f);

		Error += rblxmath::all(rblxmath::equal(Result3, rblxmath::vec3(0.625f, 0.375f, 0.0f), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 Result4 = rblxmath::hermite(
			rblxmath::vec4(0.0f, 0.0f, 0.0f, 1.0f), 
			rblxmath::vec4(1.0f, 0.0f, 0.0f, 1.0f), 
			rblxmath::vec4(1.0f, 1.0f, 0.0f, 1.0f), 
			rblxmath::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.5f);

		Error += rblxmath::all(rblxmath::equal(Result4, rblxmath::vec4(0.625f, 0.375f, 0.0f, 1.0f), rblxmath::epsilon<float>())) ? 0 : 1;

		return Error;
	}
}//catmullRom

namespace cubic
{
	static int test()
	{
		int Error = 0;

		rblxmath::vec2 Result2 = rblxmath::cubic(
			rblxmath::vec2(0.0f, 0.0f),
			rblxmath::vec2(1.0f, 0.0f),
			rblxmath::vec2(1.0f, 1.0f),
			rblxmath::vec2(0.0f, 1.0f), 0.5f);

		Error += rblxmath::all(rblxmath::equal(Result2, rblxmath::vec2(0.75f, 1.5f), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec3 Result3 = rblxmath::cubic(
			rblxmath::vec3(0.0f, 0.0f, 0.0f), 
			rblxmath::vec3(1.0f, 0.0f, 0.0f), 
			rblxmath::vec3(1.0f, 1.0f, 0.0f), 
			rblxmath::vec3(0.0f, 1.0f, 0.0f), 0.5f);

		Error += rblxmath::all(rblxmath::equal(Result3, rblxmath::vec3(0.75f, 1.5f, 0.0f), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::vec4 Result4 = rblxmath::cubic(
			rblxmath::vec4(0.0f, 0.0f, 0.0f, 1.0f), 
			rblxmath::vec4(1.0f, 0.0f, 0.0f, 1.0f), 
			rblxmath::vec4(1.0f, 1.0f, 0.0f, 1.0f), 
			rblxmath::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.5f);

		Error += rblxmath::all(rblxmath::equal(Result4, rblxmath::vec4(0.75f, 1.5f, 0.0f, 1.875f), rblxmath::epsilon<float>())) ? 0 : 1;

		return Error;
	}
}//catmullRom

int main()
{
	int Error(0);

	Error += catmullRom::test();
	Error += hermite::test();
	Error += cubic::test();

	return Error;
}
