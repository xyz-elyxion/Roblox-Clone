// Code sample from Filippo Ramaciotti

#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtc/matrix_transform.hpp>
#include <rblxmath/gtx/matrix_cross_product.hpp>
#include <rblxmath/gtx/matrix_operation.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/gtx/string_cast.hpp>
#include <rblxmath/gtx/euler_angles.hpp>
#include <cstdio>
#include <vector>
#include <utility>

namespace test_eulerAngleX
{
	static int test()
	{
		int Error = 0;

		float const Angle(rblxmath::pi<float>() * 0.5f);
		rblxmath::vec3 const X(1.0f, 0.0f, 0.0f);

		rblxmath::vec4 const Y(0.0f, 1.0f, 0.0f, 1.0f);
		rblxmath::vec4 const Y1 = rblxmath::rotate(rblxmath::mat4(1.0f), Angle, X) * Y;
		rblxmath::vec4 const Y2 = rblxmath::eulerAngleX(Angle) * Y;
		rblxmath::vec4 const Y3 = rblxmath::eulerAngleXY(Angle, 0.0f) * Y;
		rblxmath::vec4 const Y4 = rblxmath::eulerAngleYX(0.0f, Angle) * Y;
		rblxmath::vec4 const Y5 = rblxmath::eulerAngleXZ(Angle, 0.0f) * Y;
		rblxmath::vec4 const Y6 = rblxmath::eulerAngleZX(0.0f, Angle) * Y;
		rblxmath::vec4 const Y7 = rblxmath::eulerAngleYXZ(0.0f, Angle, 0.0f) * Y;
		Error += rblxmath::all(rblxmath::epsilonEqual(Y1, Y2, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Y1, Y3, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Y1, Y4, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Y1, Y5, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Y1, Y6, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Y1, Y7, 0.00001f)) ? 0 : 1;

		rblxmath::vec4 const Z(0.0f, 0.0f, 1.0f, 1.0f);
		rblxmath::vec4 const Z1 = rblxmath::rotate(rblxmath::mat4(1.0f), Angle, X) * Z;
		rblxmath::vec4 const Z2 = rblxmath::eulerAngleX(Angle) * Z;
		rblxmath::vec4 const Z3 = rblxmath::eulerAngleXY(Angle, 0.0f) * Z;
		rblxmath::vec4 const Z4 = rblxmath::eulerAngleYX(0.0f, Angle) * Z;
		rblxmath::vec4 const Z5 = rblxmath::eulerAngleXZ(Angle, 0.0f) * Z;
		rblxmath::vec4 const Z6 = rblxmath::eulerAngleZX(0.0f, Angle) * Z;
		rblxmath::vec4 const Z7 = rblxmath::eulerAngleYXZ(0.0f, Angle, 0.0f) * Z;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z2, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z3, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z4, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z5, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z6, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z7, 0.00001f)) ? 0 : 1;

		return Error;
	}
}//namespace test_eulerAngleX

namespace test_eulerAngleY
{
	static int test()
	{
		int Error = 0;

		float const Angle(rblxmath::pi<float>() * 0.5f);
		rblxmath::vec3 const Y(0.0f, 1.0f, 0.0f);

		rblxmath::vec4 const X(1.0f, 0.0f, 0.0f, 1.0f);
		rblxmath::vec4 const X1 = rblxmath::rotate(rblxmath::mat4(1.0f), Angle, Y) * X;
		rblxmath::vec4 const X2 = rblxmath::eulerAngleY(Angle) * X;
		rblxmath::vec4 const X3 = rblxmath::eulerAngleYX(Angle, 0.0f) * X;
		rblxmath::vec4 const X4 = rblxmath::eulerAngleXY(0.0f, Angle) * X;
		rblxmath::vec4 const X5 = rblxmath::eulerAngleYZ(Angle, 0.0f) * X;
		rblxmath::vec4 const X6 = rblxmath::eulerAngleZY(0.0f, Angle) * X;
		rblxmath::vec4 const X7 = rblxmath::eulerAngleYXZ(Angle, 0.0f, 0.0f) * X;
		Error += rblxmath::all(rblxmath::epsilonEqual(X1, X2, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(X1, X3, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(X1, X4, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(X1, X5, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(X1, X6, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(X1, X7, 0.00001f)) ? 0 : 1;

		rblxmath::vec4 const Z(0.0f, 0.0f, 1.0f, 1.0f);
		rblxmath::vec4 const Z1 = rblxmath::eulerAngleY(Angle) * Z;
		rblxmath::vec4 const Z2 = rblxmath::rotate(rblxmath::mat4(1.0f), Angle, Y) * Z;
		rblxmath::vec4 const Z3 = rblxmath::eulerAngleYX(Angle, 0.0f) * Z;
		rblxmath::vec4 const Z4 = rblxmath::eulerAngleXY(0.0f, Angle) * Z;
		rblxmath::vec4 const Z5 = rblxmath::eulerAngleYZ(Angle, 0.0f) * Z;
		rblxmath::vec4 const Z6 = rblxmath::eulerAngleZY(0.0f, Angle) * Z;
		rblxmath::vec4 const Z7 = rblxmath::eulerAngleYXZ(Angle, 0.0f, 0.0f) * Z;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z2, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z3, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z4, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z5, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z6, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z7, 0.00001f)) ? 0 : 1;

		return Error;
	}
}//namespace test_eulerAngleY

namespace test_eulerAngleZ
{
	static int test()
	{
		int Error = 0;

		float const Angle(rblxmath::pi<float>() * 0.5f);
		rblxmath::vec3 const Z(0.0f, 0.0f, 1.0f);

		rblxmath::vec4 const X(1.0f, 0.0f, 0.0f, 1.0f);
		rblxmath::vec4 const X1 = rblxmath::rotate(rblxmath::mat4(1.0f), Angle, Z) * X;
		rblxmath::vec4 const X2 = rblxmath::eulerAngleZ(Angle) * X;
		rblxmath::vec4 const X3 = rblxmath::eulerAngleZX(Angle, 0.0f) * X;
		rblxmath::vec4 const X4 = rblxmath::eulerAngleXZ(0.0f, Angle) * X;
		rblxmath::vec4 const X5 = rblxmath::eulerAngleZY(Angle, 0.0f) * X;
		rblxmath::vec4 const X6 = rblxmath::eulerAngleYZ(0.0f, Angle) * X;
		rblxmath::vec4 const X7 = rblxmath::eulerAngleYXZ(0.0f, 0.0f, Angle) * X;
		Error += rblxmath::all(rblxmath::epsilonEqual(X1, X2, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(X1, X3, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(X1, X4, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(X1, X5, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(X1, X6, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(X1, X7, 0.00001f)) ? 0 : 1;

		rblxmath::vec4 const Y(1.0f, 0.0f, 0.0f, 1.0f);
		rblxmath::vec4 const Z1 = rblxmath::rotate(rblxmath::mat4(1.0f), Angle, Z) * Y;
		rblxmath::vec4 const Z2 = rblxmath::eulerAngleZ(Angle) * Y;
		rblxmath::vec4 const Z3 = rblxmath::eulerAngleZX(Angle, 0.0f) * Y;
		rblxmath::vec4 const Z4 = rblxmath::eulerAngleXZ(0.0f, Angle) * Y;
		rblxmath::vec4 const Z5 = rblxmath::eulerAngleZY(Angle, 0.0f) * Y;
		rblxmath::vec4 const Z6 = rblxmath::eulerAngleYZ(0.0f, Angle) * Y;
		rblxmath::vec4 const Z7 = rblxmath::eulerAngleYXZ(0.0f, 0.0f, Angle) * Y;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z2, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z3, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z4, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z5, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z6, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(Z1, Z7, 0.00001f)) ? 0 : 1;

		return Error;
	}
}//namespace test_eulerAngleZ

namespace test_derivedEulerAngles
{
	static bool epsilonEqual(rblxmath::mat4 const& mat1, rblxmath::mat4 const& mat2, rblxmath::mat4::value_type const& epsilon)
	{
		return rblxmath::all(rblxmath::epsilonEqual(mat1[0], mat2[0], epsilon)) ?
				(
					rblxmath::all(rblxmath::epsilonEqual(mat1[1], mat2[1], epsilon)) ?
					(
						rblxmath::all(rblxmath::epsilonEqual(mat1[2], mat2[2], epsilon)) ?
						(
							rblxmath::all(rblxmath::epsilonEqual(mat1[3], mat2[3], epsilon)) ? true : false
						) : false
					) : false
				) : false;
	}

	template<typename RotationFunc, typename TestDerivedFunc>
	static int test(RotationFunc rotationFunc, TestDerivedFunc testDerivedFunc, const rblxmath::vec3& basis)
	{
		int Error = 0;

		typedef rblxmath::vec3::value_type value;
		value const zeroAngle(0.0f);
		value const Angle(rblxmath::pi<float>() * 0.75f);
		value const negativeAngle(-Angle);
		value const zeroAngleVelocity(0.0f);
		value const AngleVelocity(rblxmath::pi<float>() * 0.27f);
		value const negativeAngleVelocity(-AngleVelocity);

		typedef std::pair<value,value> AngleAndAngleVelocity;
		std::vector<AngleAndAngleVelocity> testPairs;
		testPairs.push_back(AngleAndAngleVelocity(zeroAngle, zeroAngleVelocity));
		testPairs.push_back(AngleAndAngleVelocity(zeroAngle, AngleVelocity));
		testPairs.push_back(AngleAndAngleVelocity(zeroAngle, negativeAngleVelocity));
		testPairs.push_back(AngleAndAngleVelocity(Angle, zeroAngleVelocity));
		testPairs.push_back(AngleAndAngleVelocity(Angle, AngleVelocity));
		testPairs.push_back(AngleAndAngleVelocity(Angle, negativeAngleVelocity));
		testPairs.push_back(AngleAndAngleVelocity(negativeAngle, zeroAngleVelocity));
		testPairs.push_back(AngleAndAngleVelocity(negativeAngle, AngleVelocity));
		testPairs.push_back(AngleAndAngleVelocity(negativeAngle, negativeAngleVelocity));

		for (size_t i = 0, size = testPairs.size(); i < size; ++i)
		{
			AngleAndAngleVelocity const& pair = testPairs.at(i);

			rblxmath::mat4 const W = rblxmath::matrixCross4(basis * pair.second);
			rblxmath::mat4 const rotMt = rblxmath::transpose(rotationFunc(pair.first));
			rblxmath::mat4 const derivedRotM = testDerivedFunc(pair.first, pair.second);

			Error += epsilonEqual(W, derivedRotM * rotMt, 0.00001f) ? 0 : 1;
		}

		return Error;
	}
}//namespace test_derivedEulerAngles

namespace test_eulerAngleXY
{
	static int test()
	{
		int Error = 0;

		rblxmath::vec4 const V(1.0f);

		float const AngleX(rblxmath::pi<float>() * 0.5f);
		float const AngleY(rblxmath::pi<float>() * 0.25f);

		rblxmath::vec3 const axisX(1.0f, 0.0f, 0.0f);
		rblxmath::vec3 const axisY(0.0f, 1.0f, 0.0f);

		rblxmath::vec4 const V1 = (rblxmath::rotate(rblxmath::mat4(1.0f), AngleX, axisX) * rblxmath::rotate(rblxmath::mat4(1.0f), AngleY, axisY)) * V;
		rblxmath::vec4 const V2 = rblxmath::eulerAngleXY(AngleX, AngleY) * V;
		rblxmath::vec4 const V3 = rblxmath::eulerAngleX(AngleX) * rblxmath::eulerAngleY(AngleY) * V;
		Error += rblxmath::all(rblxmath::epsilonEqual(V1, V2, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(V1, V3, 0.00001f)) ? 0 : 1;

		return Error;
	}
}//namespace test_eulerAngleXY

namespace test_eulerAngleYX
{
	static int test()
	{
		int Error = 0;

		rblxmath::vec4 const V(1.0f);

		float const AngleX(rblxmath::pi<float>() * 0.5f);
		float const AngleY(rblxmath::pi<float>() * 0.25f);

		rblxmath::vec3 const axisX(1.0f, 0.0f, 0.0f);
		rblxmath::vec3 const axisY(0.0f, 1.0f, 0.0f);

		rblxmath::vec4 const V1 = (rblxmath::rotate(rblxmath::mat4(1.0f), AngleY, axisY) * rblxmath::rotate(rblxmath::mat4(1.0f), AngleX, axisX)) * V;
		rblxmath::vec4 const V2 = rblxmath::eulerAngleYX(AngleY, AngleX) * V;
		rblxmath::vec4 const V3 = rblxmath::eulerAngleY(AngleY) * rblxmath::eulerAngleX(AngleX) * V;
		Error += rblxmath::all(rblxmath::epsilonEqual(V1, V2, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(V1, V3, 0.00001f)) ? 0 : 1;

		return Error;
	}
}//namespace test_eulerAngleYX

namespace test_eulerAngleXZ
{
	static int test()
	{
		int Error = 0;

		rblxmath::vec4 const V(1.0f);

		float const AngleX(rblxmath::pi<float>() * 0.5f);
		float const AngleZ(rblxmath::pi<float>() * 0.25f);

		rblxmath::vec3 const axisX(1.0f, 0.0f, 0.0f);
		rblxmath::vec3 const axisZ(0.0f, 0.0f, 1.0f);

		rblxmath::vec4 const V1 = (rblxmath::rotate(rblxmath::mat4(1.0f), AngleX, axisX) * rblxmath::rotate(rblxmath::mat4(1.0f), AngleZ, axisZ)) * V;
		rblxmath::vec4 const V2 = rblxmath::eulerAngleXZ(AngleX, AngleZ) * V;
		rblxmath::vec4 const V3 = rblxmath::eulerAngleX(AngleX) * rblxmath::eulerAngleZ(AngleZ) * V;
		Error += rblxmath::all(rblxmath::epsilonEqual(V1, V2, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(V1, V3, 0.00001f)) ? 0 : 1;

		return Error;
	}
}//namespace test_eulerAngleXZ

namespace test_eulerAngleZX
{
	static int test()
	{
		int Error = 0;

		rblxmath::vec4 const V(1.0f);

		float const AngleX(rblxmath::pi<float>() * 0.5f);
		float const AngleZ(rblxmath::pi<float>() * 0.25f);

		rblxmath::vec3 const axisX(1.0f, 0.0f, 0.0f);
		rblxmath::vec3 const axisZ(0.0f, 0.0f, 1.0f);

		rblxmath::vec4 const V1 = (rblxmath::rotate(rblxmath::mat4(1.0f), AngleZ, axisZ) * rblxmath::rotate(rblxmath::mat4(1.0f), AngleX, axisX)) * V;
		rblxmath::vec4 const V2 = rblxmath::eulerAngleZX(AngleZ, AngleX) * V;
		rblxmath::vec4 const V3 = rblxmath::eulerAngleZ(AngleZ) * rblxmath::eulerAngleX(AngleX) * V;
		Error += rblxmath::all(rblxmath::epsilonEqual(V1, V2, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(V1, V3, 0.00001f)) ? 0 : 1;

		return Error;
	}
}//namespace test_eulerAngleZX

namespace test_eulerAngleYZ
{
	static int test()
	{
		int Error = 0;

		rblxmath::vec4 const V(1.0f);

		float const AngleY(rblxmath::pi<float>() * 0.5f);
		float const AngleZ(rblxmath::pi<float>() * 0.25f);

		rblxmath::vec3 const axisY(0.0f, 1.0f, 0.0f);
		rblxmath::vec3 const axisZ(0.0f, 0.0f, 1.0f);

		rblxmath::vec4 const V1 = (rblxmath::rotate(rblxmath::mat4(1.0f), AngleY, axisY) * rblxmath::rotate(rblxmath::mat4(1.0f), AngleZ, axisZ)) * V;
		rblxmath::vec4 const V2 = rblxmath::eulerAngleYZ(AngleY, AngleZ) * V;
		rblxmath::vec4 const V3 = rblxmath::eulerAngleY(AngleY) * rblxmath::eulerAngleZ(AngleZ) * V;
		Error += rblxmath::all(rblxmath::epsilonEqual(V1, V2, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(V1, V3, 0.00001f)) ? 0 : 1;

		return Error;
	}
}//namespace test_eulerAngleYZ

namespace test_eulerAngleZY
{
	static int test()
	{
		int Error = 0;

		rblxmath::vec4 const V(1.0f);

		float const AngleY(rblxmath::pi<float>() * 0.5f);
		float const AngleZ(rblxmath::pi<float>() * 0.25f);

		rblxmath::vec3 const axisY(0.0f, 1.0f, 0.0f);
		rblxmath::vec3 const axisZ(0.0f, 0.0f, 1.0f);

		rblxmath::vec4 const V1 = (rblxmath::rotate(rblxmath::mat4(1.0f), AngleZ, axisZ) * rblxmath::rotate(rblxmath::mat4(1.0f), AngleY, axisY)) * V;
		rblxmath::vec4 const V2 = rblxmath::eulerAngleZY(AngleZ, AngleY) * V;
		rblxmath::vec4 const V3 = rblxmath::eulerAngleZ(AngleZ) * rblxmath::eulerAngleY(AngleY) * V;
		Error += rblxmath::all(rblxmath::epsilonEqual(V1, V2, 0.00001f)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(V1, V3, 0.00001f)) ? 0 : 1;

		return Error;
	}
}//namespace test_eulerAngleZY

namespace test_eulerAngleYXZ
{
	static int test()
	{
		rblxmath::f32 first =  1.046f;
		rblxmath::f32 second = 0.52f;
		rblxmath::f32 third = -0.785f;

		rblxmath::fmat4 rotationEuler = rblxmath::eulerAngleYXZ(first, second, third); 

		rblxmath::fmat4 rotationInvertedY  = rblxmath::eulerAngleY(-1.f*first) * rblxmath::eulerAngleX(second) * rblxmath::eulerAngleZ(third); 
		rblxmath::fmat4 rotationDumb = rblxmath::fmat4(); 
		rotationDumb = rblxmath::rotate(rotationDumb, first, rblxmath::fvec3(0,1,0));
		rotationDumb = rblxmath::rotate(rotationDumb, second, rblxmath::fvec3(1,0,0));
		rotationDumb = rblxmath::rotate(rotationDumb, third, rblxmath::fvec3(0,0,1));

		std::printf("%s\n", rblxmath::to_string(rblxmath::fmat3(rotationEuler)).c_str());
		std::printf("%s\n", rblxmath::to_string(rblxmath::fmat3(rotationDumb)).c_str());
		std::printf("%s\n", rblxmath::to_string(rblxmath::fmat3(rotationInvertedY)).c_str());

		std::printf("\nRESIDUAL\n");
		std::printf("%s\n", rblxmath::to_string(rblxmath::fmat3(rotationEuler-(rotationDumb))).c_str());
		std::printf("%s\n", rblxmath::to_string(rblxmath::fmat3(rotationEuler-(rotationInvertedY))).c_str());

		return 0;
	}
}//namespace eulerAngleYXZ

namespace test_eulerAngles
{
	template<typename TestRotationFunc>
	static int test(TestRotationFunc testRotationFunc, rblxmath::vec3 const& I, rblxmath::vec3 const& J, rblxmath::vec3 const& K)
	{
		int Error = 0;

		typedef rblxmath::mat4::value_type value;
		value const minAngle(-rblxmath::pi<value>());
		value const maxAngle(rblxmath::pi<value>());
		value const maxAngleWithDelta(maxAngle - 0.0000001f);
		value const minMidAngle(-rblxmath::pi<value>() * 0.5f);
		value const maxMidAngle(rblxmath::pi<value>() * 0.5f);

		std::vector<rblxmath::vec3> testEulerAngles;
		testEulerAngles.push_back(rblxmath::vec3(1.046f, 0.52f, -0.785f));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, minMidAngle, minAngle));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, minMidAngle, maxAngle));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, minMidAngle, maxAngleWithDelta));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, maxMidAngle, minAngle));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, maxMidAngle, maxAngle));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, maxMidAngle, maxAngleWithDelta));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, minMidAngle, minAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, minMidAngle, maxAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, minMidAngle, maxAngleWithDelta));
		testEulerAngles.push_back(rblxmath::vec3(maxAngleWithDelta, minMidAngle, maxAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngleWithDelta, minMidAngle, maxAngleWithDelta));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, maxMidAngle, minAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngleWithDelta, maxMidAngle, minAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, maxMidAngle, maxAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, maxMidAngle, maxAngleWithDelta));
		testEulerAngles.push_back(rblxmath::vec3(maxAngleWithDelta, maxMidAngle, maxAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngleWithDelta, maxMidAngle, maxAngleWithDelta));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, 0.0f, minAngle));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, 0.0f, maxAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, maxAngle, minAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, maxAngle, maxAngle));

		for (size_t i = 0, size = testEulerAngles.size(); i < size; ++i)
		{
			rblxmath::vec3 const& angles = testEulerAngles.at(i);
			rblxmath::mat4 const rotationEuler = testRotationFunc(angles.x, angles.y, angles.z);

			rblxmath::mat4 rotationDumb = rblxmath::diagonal4x4(rblxmath::mat4::col_type(1.0f));
			rotationDumb = rblxmath::rotate(rotationDumb, angles.x, I);
			rotationDumb = rblxmath::rotate(rotationDumb, angles.y, J);
			rotationDumb = rblxmath::rotate(rotationDumb, angles.z, K);

			rblxmath::vec4 const V(1.0f,1.0f,1.0f,1.0f);
			rblxmath::vec4 const V1 = rotationEuler * V;
			rblxmath::vec4 const V2 = rotationDumb * V;

			Error += rblxmath::all(rblxmath::epsilonEqual(V1, V2, 0.00001f)) ? 0 : 1;
		}

		return Error;
	}
}//namespace test_extractsEulerAngles

namespace test_extractsEulerAngles
{
	template<typename RotationFunc, typename TestExtractionFunc>
	static int test(RotationFunc rotationFunc, TestExtractionFunc testExtractionFunc)
	{
		int Error = 0;

		typedef rblxmath::mat4::value_type value;
		value const minAngle(-rblxmath::pi<value>());
		value const maxAngle(rblxmath::pi<value>());
		value const maxAngleWithDelta(maxAngle - 0.0000001f);
		value const minMidAngle(-rblxmath::pi<value>() * 0.5f);
		value const maxMidAngle(rblxmath::pi<value>() * 0.5f);

		std::vector<rblxmath::vec3> testEulerAngles;
		testEulerAngles.push_back(rblxmath::vec3(1.046f, 0.52f, -0.785f));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, minMidAngle, minAngle));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, minMidAngle, maxAngle));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, minMidAngle, maxAngleWithDelta));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, maxMidAngle, minAngle));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, maxMidAngle, maxAngle));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, maxMidAngle, maxAngleWithDelta));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, minMidAngle, minAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, minMidAngle, maxAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, minMidAngle, maxAngleWithDelta));
		testEulerAngles.push_back(rblxmath::vec3(maxAngleWithDelta, minMidAngle, maxAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngleWithDelta, minMidAngle, maxAngleWithDelta));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, maxMidAngle, minAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngleWithDelta, maxMidAngle, minAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, maxMidAngle, maxAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, maxMidAngle, maxAngleWithDelta));
		testEulerAngles.push_back(rblxmath::vec3(maxAngleWithDelta, maxMidAngle, maxAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngleWithDelta, maxMidAngle, maxAngleWithDelta));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, 0.0f, minAngle));
		testEulerAngles.push_back(rblxmath::vec3(minAngle, 0.0f, maxAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, maxAngle, minAngle));
		testEulerAngles.push_back(rblxmath::vec3(maxAngle, maxAngle, maxAngle));

		for (size_t i = 0, size = testEulerAngles.size(); i < size; ++i)
		{
			rblxmath::vec3 const& angles = testEulerAngles.at(i);
			rblxmath::mat4 const rotation = rotationFunc(angles.x, angles.y, angles.z);

			rblxmath::vec3 extractedEulerAngles(0.0f);
			testExtractionFunc(rotation, extractedEulerAngles.x, extractedEulerAngles.y, extractedEulerAngles.z);
			rblxmath::mat4 const extractedRotation = rotationFunc(extractedEulerAngles.x, extractedEulerAngles.y, extractedEulerAngles.z);

			rblxmath::vec4 const V(1.0f,1.0f,1.0f,1.0f);
			rblxmath::vec4 const V1 = rotation * V;
			rblxmath::vec4 const V2 = extractedRotation * V;

			Error += rblxmath::all(rblxmath::epsilonEqual(V1, V2, 0.00001f)) ? 0 : 1;
		}

		return Error;
	}
}//namespace test_extractsEulerAngles

int main()
{ 
	int Error = 0;

	typedef rblxmath::mat4::value_type value;
	rblxmath::vec3 const X(1.0f, 0.0f, 0.0f);
	rblxmath::vec3 const Y(0.0f, 1.0f, 0.0f);
	rblxmath::vec3 const Z(0.0f, 0.0f, 1.0f);

	Error += test_eulerAngleX::test();
	Error += test_eulerAngleY::test();
	Error += test_eulerAngleZ::test();

	Error += test_derivedEulerAngles::test(rblxmath::eulerAngleX<value>, rblxmath::derivedEulerAngleX<value>, X);
	Error += test_derivedEulerAngles::test(rblxmath::eulerAngleY<value>, rblxmath::derivedEulerAngleY<value>, Y);
	Error += test_derivedEulerAngles::test(rblxmath::eulerAngleZ<value>, rblxmath::derivedEulerAngleZ<value>, Z);

	Error += test_eulerAngleXY::test();
	Error += test_eulerAngleYX::test();
	Error += test_eulerAngleXZ::test();
	Error += test_eulerAngleZX::test();
	Error += test_eulerAngleYZ::test();
	Error += test_eulerAngleZY::test();
	Error += test_eulerAngleYXZ::test();

	Error += test_eulerAngles::test(rblxmath::eulerAngleXZX<value>, X, Z, X);
	Error += test_eulerAngles::test(rblxmath::eulerAngleXYX<value>, X, Y, X);
	Error += test_eulerAngles::test(rblxmath::eulerAngleYXY<value>, Y, X, Y);
	Error += test_eulerAngles::test(rblxmath::eulerAngleYZY<value>, Y, Z, Y);
	Error += test_eulerAngles::test(rblxmath::eulerAngleZYZ<value>, Z, Y, Z);
	Error += test_eulerAngles::test(rblxmath::eulerAngleZXZ<value>, Z, X, Z);
	Error += test_eulerAngles::test(rblxmath::eulerAngleXZY<value>, X, Z, Y);
	Error += test_eulerAngles::test(rblxmath::eulerAngleYZX<value>, Y, Z, X);
	Error += test_eulerAngles::test(rblxmath::eulerAngleZYX<value>, Z, Y, X);
	Error += test_eulerAngles::test(rblxmath::eulerAngleZXY<value>, Z, X, Y);

	Error += test_extractsEulerAngles::test(rblxmath::eulerAngleYXZ<value>, rblxmath::extractEulerAngleYXZ<value>);
	Error += test_extractsEulerAngles::test(rblxmath::eulerAngleXZX<value>, rblxmath::extractEulerAngleXZX<value>);
	Error += test_extractsEulerAngles::test(rblxmath::eulerAngleXYX<value>, rblxmath::extractEulerAngleXYX<value>);
	Error += test_extractsEulerAngles::test(rblxmath::eulerAngleYXY<value>, rblxmath::extractEulerAngleYXY<value>);
	Error += test_extractsEulerAngles::test(rblxmath::eulerAngleYZY<value>, rblxmath::extractEulerAngleYZY<value>);
	Error += test_extractsEulerAngles::test(rblxmath::eulerAngleZYZ<value>, rblxmath::extractEulerAngleZYZ<value>);
	Error += test_extractsEulerAngles::test(rblxmath::eulerAngleZXZ<value>, rblxmath::extractEulerAngleZXZ<value>);
	Error += test_extractsEulerAngles::test(rblxmath::eulerAngleXZY<value>, rblxmath::extractEulerAngleXZY<value>);
	Error += test_extractsEulerAngles::test(rblxmath::eulerAngleYZX<value>, rblxmath::extractEulerAngleYZX<value>);
	Error += test_extractsEulerAngles::test(rblxmath::eulerAngleZYX<value>, rblxmath::extractEulerAngleZYX<value>);
	Error += test_extractsEulerAngles::test(rblxmath::eulerAngleZXY<value>, rblxmath::extractEulerAngleZXY<value>);

	return Error; 
}
