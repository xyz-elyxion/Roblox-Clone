#include <rblxmath/geometric.hpp>
#include <rblxmath/trigonometric.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/vector_float1.hpp>
#include <rblxmath/ext/vector_float2.hpp>
#include <rblxmath/ext/vector_float3.hpp>
#include <rblxmath/ext/vector_float4.hpp>
#include <rblxmath/ext/vector_double2.hpp>
#include <rblxmath/ext/vector_double3.hpp>
#include <rblxmath/ext/vector_double4.hpp>
#include <limits>

namespace length
{
	static int test()
	{
		float Length1 = rblxmath::length(rblxmath::vec1(1));
		float Length2 = rblxmath::length(rblxmath::vec2(1, 0));
		float Length3 = rblxmath::length(rblxmath::vec3(1, 0, 0));
		float Length4 = rblxmath::length(rblxmath::vec4(1, 0, 0, 0));

		int Error = 0;

		Error += rblxmath::abs(Length1 - 1.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += rblxmath::abs(Length2 - 1.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += rblxmath::abs(Length3 - 1.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += rblxmath::abs(Length4 - 1.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;

		return Error;
	}
}//namespace length

namespace distance
{
	static int test()
	{
		float Distance1 = rblxmath::distance(rblxmath::vec1(1), rblxmath::vec1(1));
		float Distance2 = rblxmath::distance(rblxmath::vec2(1, 0), rblxmath::vec2(1, 0));
		float Distance3 = rblxmath::distance(rblxmath::vec3(1, 0, 0), rblxmath::vec3(1, 0, 0));
		float Distance4 = rblxmath::distance(rblxmath::vec4(1, 0, 0, 0), rblxmath::vec4(1, 0, 0, 0));

		int Error = 0;

		Error += rblxmath::abs(Distance1) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += rblxmath::abs(Distance2) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += rblxmath::abs(Distance3) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += rblxmath::abs(Distance4) < std::numeric_limits<float>::epsilon() ? 0 : 1;

		return Error;
	}
}//namespace distance

namespace dot
{
	static int test()
	{
		float Dot1 = rblxmath::dot(rblxmath::vec1(1), rblxmath::vec1(1));
		float Dot2 = rblxmath::dot(rblxmath::vec2(1), rblxmath::vec2(1));
		float Dot3 = rblxmath::dot(rblxmath::vec3(1), rblxmath::vec3(1));
		float Dot4 = rblxmath::dot(rblxmath::vec4(1), rblxmath::vec4(1));

		int Error = 0;

		Error += rblxmath::abs(Dot1 - 1.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += rblxmath::abs(Dot2 - 2.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += rblxmath::abs(Dot3 - 3.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += rblxmath::abs(Dot4 - 4.0f) < std::numeric_limits<float>::epsilon() ? 0 : 1;

		return Error;
	}
}//namespace dot

namespace cross
{
	static int test()
	{
		rblxmath::vec3 Cross1 = rblxmath::cross(rblxmath::vec3(1, 0, 0), rblxmath::vec3(0, 1, 0));
		rblxmath::vec3 Cross2 = rblxmath::cross(rblxmath::vec3(0, 1, 0), rblxmath::vec3(1, 0, 0));

		int Error = 0;

		Error += rblxmath::all(rblxmath::lessThan(rblxmath::abs(Cross1 - rblxmath::vec3(0, 0, 1)), rblxmath::vec3(std::numeric_limits<float>::epsilon()))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::lessThan(rblxmath::abs(Cross2 - rblxmath::vec3(0, 0,-1)), rblxmath::vec3(std::numeric_limits<float>::epsilon()))) ? 0 : 1;

		return Error;
	}
}//namespace cross

namespace normalize
{
	static int test()
	{
		int Error = 0;

		rblxmath::vec3 Normalize1 = rblxmath::normalize(rblxmath::vec3(1, 0, 0));
		rblxmath::vec3 Normalize2 = rblxmath::normalize(rblxmath::vec3(2, 0, 0));

		Error += rblxmath::all(rblxmath::lessThan(rblxmath::abs(Normalize1 - rblxmath::vec3(1, 0, 0)), rblxmath::vec3(std::numeric_limits<float>::epsilon()))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::lessThan(rblxmath::abs(Normalize2 - rblxmath::vec3(1, 0, 0)), rblxmath::vec3(std::numeric_limits<float>::epsilon()))) ? 0 : 1;

		rblxmath::vec3 ro = rblxmath::vec3(rblxmath::cos(5.f) * 3.f, 2.f, rblxmath::sin(5.f) * 3.f);
		rblxmath::vec3 w = rblxmath::normalize(rblxmath::vec3(0, -0.2f, 0) - ro);
		rblxmath::vec3 u = rblxmath::normalize(rblxmath::cross(w, rblxmath::vec3(0, 1, 0)));
		rblxmath::vec3 v = rblxmath::cross(u, w);
		rblxmath::vec3 x = rblxmath::cross(w, u);

		Error += rblxmath::all(rblxmath::equal(x + v, rblxmath::vec3(0), 0.01f)) ? 0 : 1;

		return Error;
	}
}//namespace normalize

namespace faceforward
{
	static int test()
	{
		int Error = 0;

		{
			rblxmath::vec3 N(0.0f, 0.0f, 1.0f);
			rblxmath::vec3 I(1.0f, 0.0f, 1.0f);
			rblxmath::vec3 Nref(0.0f, 0.0f, 1.0f);
			rblxmath::vec3 F = rblxmath::faceforward(N, I, Nref);

			Error += rblxmath::all(rblxmath::equal(F, rblxmath::vec3(0.0, 0.0, -1.0), 0.0001f)) ? 0 : 1;
		}

		return Error;
	}
}//namespace faceforward

namespace reflect
{
	static int test()
	{
		int Error = 0;

		{
			rblxmath::vec2 A(1.0f,-1.0f);
			rblxmath::vec2 B(0.0f, 1.0f);
			rblxmath::vec2 C = rblxmath::reflect(A, B);
			Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec2(1.0, 1.0), 0.0001f)) ? 0 : 1;
		}

		{
			rblxmath::dvec2 A(1.0f,-1.0f);
			rblxmath::dvec2 B(0.0f, 1.0f);
			rblxmath::dvec2 C = rblxmath::reflect(A, B);
			Error += rblxmath::all(rblxmath::equal(C, rblxmath::dvec2(1.0, 1.0), 0.0001)) ? 0 : 1;
		}

		return Error;
	}
}//namespace reflect

namespace refract
{
	static int test()
	{
		int Error = 0;

		{
			float A(-1.0f);
			float B(1.0f);
			float C = rblxmath::refract(A, B, 0.5f);
			Error += rblxmath::equal(C, -1.0f, 0.0001f) ? 0 : 1;
		}

		{
			rblxmath::vec2 A(0.0f,-1.0f);
			rblxmath::vec2 B(0.0f, 1.0f);
			rblxmath::vec2 C = rblxmath::refract(A, B, 0.5f);
			Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec2(0.0, -1.0), 0.0001f)) ? 0 : 1;
		}

		{
			rblxmath::dvec2 A(0.0f,-1.0f);
			rblxmath::dvec2 B(0.0f, 1.0f);
			rblxmath::dvec2 C = rblxmath::refract(A, B, 0.5);
			Error += rblxmath::all(rblxmath::equal(C, rblxmath::dvec2(0.0, -1.0), 0.0001)) ? 0 : 1;
		}

		{
			rblxmath::vec4 A(0.0f, -1.0f, 0.0f, 0.0f);
			rblxmath::vec4 B(0.0f, 1.0f, 0.0f, 0.0f);
			rblxmath::vec4 C = rblxmath::refract(A, B, 0.5f);
			Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec4(0.0, -1.0, 0.0f, 0.0f), 0.0001f)) ? 0 : 1;
		}

		return Error;
	}
}//namespace refract

int main()
{
	int Error(0);

	Error += length::test();
	Error += distance::test();
	Error += dot::test();
	Error += cross::test();
	Error += normalize::test();
	Error += faceforward::test();
	Error += reflect::test();
	Error += refract::test();

	return Error;
}

