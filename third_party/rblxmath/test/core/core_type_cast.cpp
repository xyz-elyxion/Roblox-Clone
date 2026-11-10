#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/rblxmath.hpp>
#include <algorithm>
#include <vector>
#include <iterator>

struct my_vec2
{
	operator rblxmath::vec2() { return rblxmath::vec2(x, y); }
	float x, y;
};

static int test_vec2_cast()
{
	int Error(0);

	rblxmath::vec2 A(1.0f, 2.0f);
	rblxmath::lowp_vec2 B(A);
	rblxmath::mediump_vec2 C(A);
	rblxmath::highp_vec2 D(A);

	rblxmath::vec2 E = static_cast<rblxmath::vec2>(A);
	rblxmath::lowp_vec2 F = static_cast<rblxmath::lowp_vec2>(A);
	rblxmath::mediump_vec2 G = static_cast<rblxmath::mediump_vec2>(A);
	rblxmath::highp_vec2 H = static_cast<rblxmath::highp_vec2>(A);

	Error += rblxmath::all(rblxmath::equal(A, E, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(B, F, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(C, G, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(D, H, rblxmath::epsilon<float>())) ? 0 : 1;

	my_vec2 I;
	I.x = 1.0f;
	I.y = 2.0f;
	rblxmath::vec2 J0 = static_cast<rblxmath::vec2>(I);
	rblxmath::vec2 J1(1.0f, 2.0f);

	Error += rblxmath::all(rblxmath::equal(J1, J0, rblxmath::epsilon<float>())) ? 0 : 1;

	rblxmath::vec2 K(7.8f);
	rblxmath::vec2 L(7.8f, 7.8f);
	Error += rblxmath::all(rblxmath::equal(K, L, rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_vec3_cast()
{
	rblxmath::vec3 A(1.0f, 2.0f, 3.0f);
	rblxmath::lowp_vec3 B(A);
	rblxmath::mediump_vec3 C(A);
	rblxmath::highp_vec3 D(A);
	
	rblxmath::vec3 E = static_cast<rblxmath::vec3>(A);
	rblxmath::lowp_vec3 F = static_cast<rblxmath::lowp_vec3>(A);
	rblxmath::mediump_vec3 G = static_cast<rblxmath::mediump_vec3>(A);
	rblxmath::highp_vec3 H = static_cast<rblxmath::highp_vec3>(A);
	
	int Error(0);
	
	Error += rblxmath::all(rblxmath::equal(A, E, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(B, F, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(C, G, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(D, H, rblxmath::epsilon<float>())) ? 0 : 1;
	
	return Error;
}

static int test_vec4_cast()
{
	rblxmath::vec4 A(1.0f, 2.0f, 3.0f, 4.0f);
	rblxmath::lowp_vec4 B(A);
	rblxmath::mediump_vec4 C(A);
	rblxmath::highp_vec4 D(A);
	
	rblxmath::vec4 E = static_cast<rblxmath::vec4>(A);
	rblxmath::lowp_vec4 F = static_cast<rblxmath::lowp_vec4>(A);
	rblxmath::mediump_vec4 G = static_cast<rblxmath::mediump_vec4>(A);
	rblxmath::highp_vec4 H = static_cast<rblxmath::highp_vec4>(A);
	
	int Error(0);
	
	Error += rblxmath::all(rblxmath::equal(A, E, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(B, F, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(C, G, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(D, H, rblxmath::epsilon<float>())) ? 0 : 1;
	
	return Error;
}

static int test_std_copy()
{
	int Error = 0;

	{
		std::vector<int> High;
		High.resize(64);
		std::vector<int> Medium(High.size());
		
		std::copy(High.begin(), High.end(), Medium.begin());

		*Medium.begin() = *High.begin();
	}

	{
		std::vector<rblxmath::dvec4> High4;
		High4.resize(64);
		std::vector<rblxmath::vec4> Medium4(High4.size());
		
		std::copy(High4.begin(), High4.end(), Medium4.begin());

		*Medium4.begin() = *High4.begin();
	}

	{
		std::vector<rblxmath::dvec3> High3;
		High3.resize(64);
		std::vector<rblxmath::vec3> Medium3(High3.size());

		std::copy(High3.begin(), High3.end(), Medium3.begin());

		*Medium3.begin() = *High3.begin();
	}

	{
		std::vector<rblxmath::dvec2> High2;
		High2.resize(64);
		std::vector<rblxmath::vec2> Medium2(High2.size());

		std::copy(High2.begin(), High2.end(), Medium2.begin());

		*Medium2.begin() = *High2.begin();
	}

	rblxmath::dvec4 v1;
	rblxmath::vec4 v2;

	v2 = v1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_std_copy();
	Error += test_vec2_cast();
	Error += test_vec3_cast();
	Error += test_vec4_cast();

	return Error;
}
