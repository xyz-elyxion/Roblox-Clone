#include <rblxmath/gtc/matrix_transform.hpp>
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/matrix_relational.hpp>
#include <rblxmath/ext/scalar_relational.hpp>

static int test_perspective()
{
	int Error = 0;

	const float Near = 0.1f;
	const float Far = 100.0f;
	const float Eps = rblxmath::epsilon<float>();

	rblxmath::mat4 Projection = rblxmath::perspective(rblxmath::pi<float>() * 0.25f, 4.0f / 3.0f, Near, Far);

	Projection = rblxmath::perspectiveLH_ZO(rblxmath::pi<float>() * 0.25f, 4.0f / 3.0f, Near, Far);
	{
		rblxmath::vec4 N = Projection * rblxmath::vec4(0.f, 0.f, Near, 1.f);
		rblxmath::vec4 F = Projection * rblxmath::vec4(0.f, 0.f, Far, 1.f);
		N /= N.w;
		F /= F.w;
		Error += rblxmath::notEqual(N.z, 0.f, Eps);
		Error += rblxmath::notEqual(F.z, 1.f, Eps);
	}

	Projection = rblxmath::perspectiveLH_NO(rblxmath::pi<float>() * 0.25f, 4.0f / 3.0f, Near, Far);
	{
		rblxmath::vec4 N = Projection * rblxmath::vec4(0.f, 0.f, Near, 1.f);
		rblxmath::vec4 F = Projection * rblxmath::vec4(0.f, 0.f, Far, 1.f);
		N /= N.w;
		F /= F.w;
		Error += rblxmath::notEqual(N.z, -1.f, Eps);
		Error += rblxmath::notEqual(F.z, 1.f, Eps);
	}
	
	Projection = rblxmath::perspectiveRH_ZO(rblxmath::pi<float>() * 0.25f, 4.0f / 3.0f, Near, Far);
	{
		rblxmath::vec4 N = Projection * rblxmath::vec4(0.f, 0.f, -Near, 1.f);
		rblxmath::vec4 F = Projection * rblxmath::vec4(0.f, 0.f, -Far, 1.f);
		N /= N.w;
		F /= F.w;
		Error += rblxmath::notEqual(N.z, 0.f, Eps);
		Error += rblxmath::notEqual(F.z, 1.f, Eps);
	}

	Projection = rblxmath::perspectiveRH_NO(rblxmath::pi<float>() * 0.25f, 4.0f / 3.0f, Near, Far);
	{
		rblxmath::vec4 N = Projection * rblxmath::vec4(0.f, 0.f, -Near, 1.f);
		rblxmath::vec4 F = Projection * rblxmath::vec4(0.f, 0.f, -Far, 1.f);
		N /= N.w;
		F /= F.w;
		Error += rblxmath::notEqual(N.z, -1.f, Eps);
		Error += rblxmath::notEqual(F.z, 1.f, Eps);
	}

	return Error;
}

static int test_infinitePerspective()
{
	int Error = 0;

	const float Near = 0.1f;
	const float Inf = 1.0e+10f;
	const float Eps = rblxmath::epsilon<float>();

	rblxmath::mat4 Projection = rblxmath::infinitePerspective(rblxmath::pi<float>() * 0.25f, 4.0f / 3.0f, Near);

	Projection = rblxmath::infinitePerspectiveLH_ZO(rblxmath::pi<float>() * 0.25f, 4.0f / 3.0f, Near);
	{
		rblxmath::vec4 N = Projection * rblxmath::vec4(0.f, 0.f, Near, 1.f);
		rblxmath::vec4 F = Projection * rblxmath::vec4(0.f, 0.f, Inf, 1.f);
		N /= N.w;
		F /= F.w;
		Error += rblxmath::notEqual(N.z, 0.f, Eps);
		Error += rblxmath::notEqual(F.z, 1.f, Eps);
	}

	Projection = rblxmath::infinitePerspectiveLH_NO(rblxmath::pi<float>() * 0.25f, 4.0f / 3.0f, Near);
	{
		rblxmath::vec4 N = Projection * rblxmath::vec4(0.f, 0.f, Near, 1.f);
		rblxmath::vec4 F = Projection * rblxmath::vec4(0.f, 0.f, Inf, 1.f);
		N /= N.w;
		F /= F.w;
		Error += rblxmath::notEqual(N.z, -1.f, Eps);
		Error += rblxmath::notEqual(F.z, 1.f, Eps);
	}

	Projection = rblxmath::infinitePerspectiveRH_ZO(rblxmath::pi<float>() * 0.25f, 4.0f / 3.0f, Near);
	{
		rblxmath::vec4 N = Projection * rblxmath::vec4(0.f, 0.f, -Near, 1.f);
		rblxmath::vec4 F = Projection * rblxmath::vec4(0.f, 0.f, -Inf, 1.f);
		N /= N.w;
		F /= F.w;
		Error += rblxmath::notEqual(N.z, 0.f, Eps);
		Error += rblxmath::notEqual(F.z, 1.f, Eps);
	}

	Projection = rblxmath::infinitePerspectiveRH_NO(rblxmath::pi<float>() * 0.25f, 4.0f / 3.0f, Near);
	{
		rblxmath::vec4 N = Projection * rblxmath::vec4(0.f, 0.f, -Near, 1.f);
		rblxmath::vec4 F = Projection * rblxmath::vec4(0.f, 0.f, -Inf, 1.f);
		N /= N.w;
		F /= F.w;
		Error += rblxmath::notEqual(N.z, -1.f, Eps);
		Error += rblxmath::notEqual(F.z, 1.f, Eps);
	}

	return Error;
}

static int test_pick()
{
	int Error = 0;

	rblxmath::mat4 Pick = rblxmath::pickMatrix(rblxmath::vec2(1, 2), rblxmath::vec2(3, 4), rblxmath::ivec4(0, 0, 320, 240));
	Error += !rblxmath::all(rblxmath::notEqual(Pick, rblxmath::mat4(2.0), 0.001f));

	return Error;
}

static int test_tweakedInfinitePerspective()
{
	int Error = 0;

	rblxmath::mat4 ProjectionA = rblxmath::tweakedInfinitePerspective(45.f, 640.f/480.f, 1.0f);
	rblxmath::mat4 ProjectionB = rblxmath::tweakedInfinitePerspective(45.f, 640.f/480.f, 1.0f, 0.001f);

	Error += !rblxmath::all(rblxmath::notEqual(ProjectionA, rblxmath::mat4(1.0), 0.001f));
	Error += !rblxmath::all(rblxmath::notEqual(ProjectionB, rblxmath::mat4(1.0), 0.001f));

	return Error;
}

static int test_translate()
{
	int Error = 0;

	rblxmath::lowp_vec3 v(1.0);
	rblxmath::lowp_mat4 m(0);
	rblxmath::lowp_mat4 t = rblxmath::translate(m, v);

	rblxmath::bvec4 b = rblxmath::notEqual(t, rblxmath::lowp_mat4(1.0), 0.001f);

	Error += !rblxmath::all(b);

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_translate();
	Error += test_tweakedInfinitePerspective();
	Error += test_pick();
	Error += test_perspective();
	Error += test_infinitePerspective();

	return Error;
}
