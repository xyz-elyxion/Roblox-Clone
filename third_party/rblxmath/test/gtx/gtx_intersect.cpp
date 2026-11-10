#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/rblxmath.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/gtx/intersect.hpp>

static int test_intersectRayPlane()
{
	int Error = 0;
	rblxmath::vec3 const PlaneOrigin(0, 0, 1);
	rblxmath::vec3 const PlaneNormal(0, 0, -1);
	rblxmath::vec3 const RayOrigin(0, 0, 0);
	rblxmath::vec3 const RayDir(0, 0, 1);

	// check that inversion of the plane normal has no effect
	{
		float Distance = 0;
		bool const Result = rblxmath::intersectRayPlane(RayOrigin, RayDir, PlaneOrigin, PlaneNormal, Distance);
		Error += rblxmath::abs(Distance - 1.f) <= std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += Result ? 0 : 1;
	}
	{
		float Distance = 0;
		bool const Result = rblxmath::intersectRayPlane(RayOrigin, RayDir, PlaneOrigin, -1.f * PlaneNormal, Distance);
		Error += rblxmath::abs(Distance - 1.f) <= std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += Result ? 0 : 1;
	}

	// check if plane is before of behind the ray origin
	{
		float Distance = 9.9999f; // value should not be changed
		bool const Result = rblxmath::intersectRayPlane(RayOrigin, RayDir, -1.f * PlaneOrigin, PlaneNormal, Distance);
		Error += rblxmath::abs(Distance - 9.9999f) <= std::numeric_limits<float>::epsilon() ? 0 : 1;
		Error += Result ? 1 : 0; // there is no intersection in front of the ray origin, only behind
	}

	return Error;
}

static int test_intersectRayTriangle()
{
	int Error = 0;

	rblxmath::vec3 const Orig(0, 0, 2);
	rblxmath::vec3 const Dir(0, 0, -1);
	rblxmath::vec3 const Vert0(0, 0, 0);
	rblxmath::vec3 const Vert1(-1, -1, 0);
	rblxmath::vec3 const Vert2(1, -1, 0);
	rblxmath::vec2 BaryPosition(0);
	float Distance = 0;

	bool const Result = rblxmath::intersectRayTriangle(Orig, Dir, Vert0, Vert1, Vert2, BaryPosition, Distance);

	Error += rblxmath::all(rblxmath::epsilonEqual(BaryPosition, rblxmath::vec2(0), std::numeric_limits<float>::epsilon())) ? 0 : 1;
	Error += rblxmath::abs(Distance - 2.f) <= std::numeric_limits<float>::epsilon() ? 0 : 1;
	Error += Result ? 0 : 1;

	return Error;
}

static int test_intersectLineTriangle()
{
	int Error = 0;

	rblxmath::vec3 const Orig(0, 0, 2);
	rblxmath::vec3 const Dir(0, 0, -1);
	rblxmath::vec3 const Vert0(0, 0, 0);
	rblxmath::vec3 const Vert1(-1, -1, 0);
	rblxmath::vec3 const Vert2(1, -1, 0);
	rblxmath::vec3 Position(2.0f, 0.0f, 0.0f);

	bool const Result = rblxmath::intersectLineTriangle(Orig, Dir, Vert0, Vert1, Vert2, Position);

	Error += rblxmath::all(rblxmath::epsilonEqual(Position, rblxmath::vec3(2.0f, 0.0f, 0.0f), std::numeric_limits<float>::epsilon())) ? 0 : 1;
	Error += Result ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_intersectRayPlane();
	Error += test_intersectRayTriangle();
	Error += test_intersectLineTriangle();

	return Error;
}
