#define RBLXMATH_ENABLE_EXPERIMENTAL
#define RBLXMATH_FORCE_CTOR_INIT
#include <rblxmath/gtx/dual_quaternion.hpp>
#include <rblxmath/gtc/matrix_transform.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/gtx/euler_angles.hpp>
#include <rblxmath/vector_relational.hpp>
#if RBLXMATH_HAS_TRIVIAL_QUERIES
#	include <type_traits>
#endif

static int myrand()
{
	static int holdrand = 1;
	return (((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}

static float myfrand() // returns values from -1 to 1 inclusive
{
	return float(double(myrand()) / double( 0x7ffff )) * 2.0f - 1.0f;
}

static int test_dquat_type()
{
	rblxmath::dvec3 vA;
	rblxmath::dquat dqA, dqB;
	rblxmath::ddualquat C(dqA, dqB);
	rblxmath::ddualquat B(dqA);
	rblxmath::ddualquat D(dqA, vA);
	return 0;
}

static int test_scalars()
{
	float const Epsilon = 0.0001f;

	int Error(0);

	rblxmath::quat src_q1 = rblxmath::quat(1.0f,2.0f,3.0f,4.0f);
	rblxmath::quat src_q2 = rblxmath::quat(5.0f,6.0f,7.0f,8.0f);
	rblxmath::dualquat src1(src_q1,src_q2);

	{
		rblxmath::dualquat dst1 = src1 * 2.0f;
		rblxmath::dualquat dst2 = 2.0f * src1;
		rblxmath::dualquat dst3 = src1;
		dst3 *= 2.0f;
		rblxmath::dualquat dstCmp(src_q1 * 2.0f,src_q2 * 2.0f);
		Error += rblxmath::all(rblxmath::epsilonEqual(dst1.real,dstCmp.real, Epsilon)) && rblxmath::all(rblxmath::epsilonEqual(dst1.dual,dstCmp.dual, Epsilon)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(dst2.real,dstCmp.real, Epsilon)) && rblxmath::all(rblxmath::epsilonEqual(dst2.dual,dstCmp.dual, Epsilon)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(dst3.real,dstCmp.real, Epsilon)) && rblxmath::all(rblxmath::epsilonEqual(dst3.dual,dstCmp.dual, Epsilon)) ? 0 : 1;
	}

	{
		rblxmath::dualquat dst1 = src1 / 2.0f;
		rblxmath::dualquat dst2 = src1;
		dst2 /= 2.0f;
		rblxmath::dualquat dstCmp(src_q1 / 2.0f,src_q2 / 2.0f);
		Error += rblxmath::all(rblxmath::epsilonEqual(dst1.real,dstCmp.real, Epsilon)) && rblxmath::all(rblxmath::epsilonEqual(dst1.dual,dstCmp.dual, Epsilon)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(dst2.real,dstCmp.real, Epsilon)) && rblxmath::all(rblxmath::epsilonEqual(dst2.dual,dstCmp.dual, Epsilon)) ? 0 : 1;
	}
	return Error;
}

static int test_inverse()
{
	int Error(0);

	float const Epsilon = 0.0001f;

	rblxmath::dualquat dqid = rblxmath::dual_quat_identity<float, rblxmath::defaultp>();
	rblxmath::mat4x4 mid(1.0f);

	for (int j = 0; j < 100; ++j)
	{
		rblxmath::mat4x4 rot = rblxmath::yawPitchRoll(myfrand() * 360.0f, myfrand() * 360.0f, myfrand() * 360.0f);
		rblxmath::vec3 vt = rblxmath::vec3(myfrand() * 10.0f, myfrand() * 10.0f, myfrand() * 10.0f);

		rblxmath::mat4x4 m = rblxmath::translate(mid, vt) * rot;

		rblxmath::quat qr = rblxmath::quat_cast(m);

		rblxmath::dualquat dq(qr);

		rblxmath::dualquat invdq = rblxmath::inverse(dq);

		rblxmath::dualquat r1 = invdq * dq;
		rblxmath::dualquat r2 = dq * invdq;

		Error += rblxmath::all(rblxmath::epsilonEqual(r1.real, dqid.real, Epsilon)) && rblxmath::all(rblxmath::epsilonEqual(r1.dual, dqid.dual, Epsilon)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(r2.real, dqid.real, Epsilon)) && rblxmath::all(rblxmath::epsilonEqual(r2.dual, dqid.dual, Epsilon)) ? 0 : 1;

		// testing commutative property
		rblxmath::dualquat r (   rblxmath::quat( myfrand() * rblxmath::pi<float>() * 2.0f, myfrand(), myfrand(), myfrand() ),
							rblxmath::vec3(myfrand() * 10.0f, myfrand() * 10.0f, myfrand() * 10.0f) );
		rblxmath::dualquat riq = (r * invdq) * dq;
		rblxmath::dualquat rqi = (r * dq) * invdq;

		Error += rblxmath::all(rblxmath::epsilonEqual(riq.real, rqi.real, Epsilon)) && rblxmath::all(rblxmath::epsilonEqual(riq.dual, rqi.dual, Epsilon)) ? 0 : 1;
	}

	return Error;
}

static int test_mul()
{
	int Error(0);

	float const Epsilon = 0.0001f;

	rblxmath::mat4x4 mid(1.0f);

	for (int j = 0; j < 100; ++j)
	{
		// generate random rotations and translations and compare transformed by matrix and dualquats random points 
		rblxmath::vec3 vt1 = rblxmath::vec3(myfrand() * 10.0f, myfrand() * 10.0f, myfrand() * 10.0f);
		rblxmath::vec3 vt2 = rblxmath::vec3(myfrand() * 10.0f, myfrand() * 10.0f, myfrand() * 10.0f);

		rblxmath::mat4x4 rot1 = rblxmath::yawPitchRoll(myfrand() * 360.0f, myfrand() * 360.0f, myfrand() * 360.0f);
		rblxmath::mat4x4 rot2 = rblxmath::yawPitchRoll(myfrand() * 360.0f, myfrand() * 360.0f, myfrand() * 360.0f);
		rblxmath::mat4x4 m1 = rblxmath::translate(mid, vt1) * rot1;
		rblxmath::mat4x4 m2 = rblxmath::translate(mid, vt2) * rot2;
		rblxmath::mat4x4 m3 = m2 * m1;
		rblxmath::mat4x4 m4 = m1 * m2;

		rblxmath::quat qrot1 = rblxmath::quat_cast(rot1);
		rblxmath::quat qrot2 = rblxmath::quat_cast(rot2);

		rblxmath::dualquat dq1 = rblxmath::dualquat(qrot1,vt1);
		rblxmath::dualquat dq2 = rblxmath::dualquat(qrot2,vt2);
		rblxmath::dualquat dq3 = dq2 * dq1;
		rblxmath::dualquat dq4 = dq1 * dq2;

		for (int i = 0; i < 100; ++i)
		{
			rblxmath::vec4 src_pt = rblxmath::vec4(myfrand() * 4.0f, myfrand() * 5.0f, myfrand() * 3.0f,1.0f);
			// test both multiplication orders        
			rblxmath::vec4 dst_pt_m3  = m3 * src_pt; 
			rblxmath::vec4 dst_pt_dq3 = dq3 * src_pt;

			rblxmath::vec4 dst_pt_m3_i  = rblxmath::inverse(m3) * src_pt;
			rblxmath::vec4 dst_pt_dq3_i = src_pt * dq3;

			rblxmath::vec4 dst_pt_m4  = m4 * src_pt;
			rblxmath::vec4 dst_pt_dq4 = dq4 * src_pt;

			rblxmath::vec4 dst_pt_m4_i  = rblxmath::inverse(m4) * src_pt;
			rblxmath::vec4 dst_pt_dq4_i = src_pt * dq4;

			Error += rblxmath::all(rblxmath::epsilonEqual(dst_pt_m3, dst_pt_dq3, Epsilon)) ? 0 : 1;
			Error += rblxmath::all(rblxmath::epsilonEqual(dst_pt_m4, dst_pt_dq4, Epsilon)) ? 0 : 1;
			Error += rblxmath::all(rblxmath::epsilonEqual(dst_pt_m3_i, dst_pt_dq3_i, Epsilon)) ? 0 : 1;
			Error += rblxmath::all(rblxmath::epsilonEqual(dst_pt_m4_i, dst_pt_dq4_i, Epsilon)) ? 0 : 1;
		}
	} 

	return Error;
}

static int test_dual_quat_ctr()
{
	int Error(0);

#	if RBLXMATH_HAS_TRIVIAL_QUERIES
	//	Error += std::is_trivially_default_constructible<rblxmath::dualquat>::value ? 0 : 1;
	//	Error += std::is_trivially_default_constructible<rblxmath::ddualquat>::value ? 0 : 1;
	//	Error += std::is_trivially_copy_assignable<rblxmath::dualquat>::value ? 0 : 1;
	//	Error += std::is_trivially_copy_assignable<rblxmath::ddualquat>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::dualquat>::value ? 0 : 1;
		Error += std::is_trivially_copyable<rblxmath::ddualquat>::value ? 0 : 1;

		Error += std::is_copy_constructible<rblxmath::dualquat>::value ? 0 : 1;
		Error += std::is_copy_constructible<rblxmath::ddualquat>::value ? 0 : 1;
#	endif

	return Error;
}

static int test_size()
{
	int Error = 0;

	Error += 32 == sizeof(rblxmath::dualquat) ? 0 : 1;
	Error += 64 == sizeof(rblxmath::ddualquat) ? 0 : 1;
	Error += rblxmath::dualquat().length() == 2 ? 0 : 1;
	Error += rblxmath::ddualquat().length() == 2 ? 0 : 1;
	Error += rblxmath::dualquat::length() == 2 ? 0 : 1;
	Error += rblxmath::ddualquat::length() == 2 ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_dual_quat_ctr();
	Error += test_dquat_type();
	Error += test_scalars();
	Error += test_inverse();
	Error += test_mul();
	Error += test_size();

	return Error;
}
