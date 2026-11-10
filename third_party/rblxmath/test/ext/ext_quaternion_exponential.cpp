#include <rblxmath/ext/quaternion_exponential.hpp>
#include <rblxmath/ext/quaternion_float.hpp>
#include <rblxmath/ext/quaternion_float_precision.hpp>
#include <rblxmath/ext/quaternion_double.hpp>
#include <rblxmath/ext/quaternion_double_precision.hpp>
#include <rblxmath/ext/quaternion_relational.hpp>
#include <rblxmath/ext/vector_float3.hpp>
#include <rblxmath/ext/vector_float3_precision.hpp>
#include <rblxmath/ext/vector_double3.hpp>
#include <rblxmath/ext/vector_double3_precision.hpp>
#include <rblxmath/ext/scalar_constants.hpp>

template <typename quaType, typename vecType>
static int test_log()
{
	typedef typename quaType::value_type T;
	
	T const Epsilon = static_cast<T>(0.001f);
	
	int Error = 0;
	
	quaType const Q(vecType(1, 0, 0), vecType(0, 1, 0));
	quaType const P = rblxmath::log(Q);
	Error += rblxmath::any(rblxmath::notEqual(Q, P, Epsilon)) ? 0 : 1;
	
	quaType const R = rblxmath::exp(P);
	Error += rblxmath::all(rblxmath::equal(Q, R, Epsilon)) ? 0 : 1;

	return Error;
}

template <typename quaType, typename vecType>
static int test_pow()
{
	typedef typename quaType::value_type T;
	
	T const Epsilon = static_cast<T>(0.001f);
	
	int Error = 0;
	
	quaType const Q(vecType(1, 0, 0), vecType(0, 1, 0));
	
	{
		T const One = static_cast<T>(1.0f);
		quaType const P = rblxmath::pow(Q, One);
		Error += rblxmath::all(rblxmath::equal(Q, P, Epsilon)) ? 0 : 1;
	}
	
	{
		T const Two = static_cast<T>(2.0f);
		quaType const P = rblxmath::pow(Q, Two);
		quaType const R = Q * Q;
		Error += rblxmath::all(rblxmath::equal(P, R, Epsilon)) ? 0 : 1;

		quaType const U = rblxmath::sqrt(P);
		Error += rblxmath::all(rblxmath::equal(Q, U, Epsilon)) ? 0 : 1;
	}
	
	return Error;
}

int main()
{
	int Error = 0;

	Error += test_log<rblxmath::quat, rblxmath::vec3>();
	Error += test_log<rblxmath::lowp_quat, rblxmath::lowp_vec3>();
	Error += test_log<rblxmath::mediump_quat, rblxmath::mediump_vec3>();
	Error += test_log<rblxmath::highp_quat, rblxmath::highp_vec3>();
	
	Error += test_log<rblxmath::dquat, rblxmath::dvec3>();
	Error += test_log<rblxmath::lowp_dquat, rblxmath::lowp_dvec3>();
	Error += test_log<rblxmath::mediump_dquat, rblxmath::mediump_dvec3>();
	Error += test_log<rblxmath::highp_dquat, rblxmath::highp_dvec3>();

	Error += test_pow<rblxmath::quat, rblxmath::vec3>();
	Error += test_pow<rblxmath::lowp_quat, rblxmath::lowp_vec3>();
	Error += test_pow<rblxmath::mediump_quat, rblxmath::mediump_vec3>();
	Error += test_pow<rblxmath::highp_quat, rblxmath::highp_vec3>();
	
	Error += test_pow<rblxmath::dquat, rblxmath::dvec3>();
	Error += test_pow<rblxmath::lowp_dquat, rblxmath::lowp_dvec3>();
	Error += test_pow<rblxmath::mediump_dquat, rblxmath::mediump_dvec3>();
	Error += test_pow<rblxmath::highp_dquat, rblxmath::highp_dvec3>();
	
	return Error;
}
