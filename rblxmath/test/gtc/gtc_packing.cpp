#include <rblxmath/packing.hpp>
#include <rblxmath/gtc/packing.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <cstdio>
#include <vector>

/*
static void print_bits(float const& s)
{
	union
	{
		float f;
		unsigned int i;
	} uif;

	uif.f = s;

	std::printf("f32: ");
	for(std::size_t j = sizeof(s) * 8; j > 0; --j)
	{
		if(j == 23 || j == 31)
			std::printf(" ");
		std::printf("%d", (uif.i & (1 << (j - 1))) ? 1 : 0);
	}
}

static void print_10bits(rblxmath::uint const& s)
{
	std::printf("10b: ");
	for(std::size_t j = 10; j > 0; --j)
	{
		if(j == 5)
			std::printf(" ");
		std::printf("%d", (s & (1 << (j - 1))) ? 1 : 0);
	}
}

static void print_11bits(rblxmath::uint const& s)
{
	std::printf("11b: ");
	for(std::size_t j = 11; j > 0; --j)
	{
		if(j == 6)
			std::printf(" ");
		std::printf("%d", (s & (1 << (j - 1))) ? 1 : 0);
	}
}

static void print_value(float const& s)
{
	std::printf("%2.5f, ", static_cast<double>(s));
	print_bits(s);
	std::printf(", ");
//	print_11bits(detail::floatTo11bit(s));
//	std::printf(", ");
//	print_10bits(detail::floatTo10bit(s));
	std::printf("\n");
}
*/
static int test_Half1x16()
{
	int Error = 0;

	std::vector<float> Tests;
	Tests.push_back(0.0f);
	Tests.push_back(1.0f);
	Tests.push_back(-1.0f);
	Tests.push_back(2.0f);
	Tests.push_back(-2.0f);
	Tests.push_back(1.9f);

	for(std::size_t i = 0; i < Tests.size(); ++i)
	{
		rblxmath::uint16 p0 = rblxmath::packHalf1x16(Tests[i]);
		float v0 = rblxmath::unpackHalf1x16(p0);
		rblxmath::uint16 p1 = rblxmath::packHalf1x16(v0);
		float v1 = rblxmath::unpackHalf1x16(p1);
		Error += rblxmath::epsilonEqual(v0, v1, rblxmath::epsilon<float>()) ? 0 : 1;
	}

	return Error;
}

static int test_Half4x16()
{
	int Error = 0;

	std::vector<rblxmath::vec4> Tests;
	Tests.push_back(rblxmath::vec4(1.0f));
	Tests.push_back(rblxmath::vec4(0.0f));
	Tests.push_back(rblxmath::vec4(2.0f));
	Tests.push_back(rblxmath::vec4(0.1f));
	Tests.push_back(rblxmath::vec4(0.5f));
	Tests.push_back(rblxmath::vec4(-0.9f));

	for(std::size_t i = 0; i < Tests.size(); ++i)
	{
		rblxmath::uint64 p0 = rblxmath::packHalf4x16(Tests[i]);
		rblxmath::vec4 v0 = rblxmath::unpackHalf4x16(p0);
		rblxmath::uint64 p1 = rblxmath::packHalf4x16(v0);
		rblxmath::vec4 v1 = rblxmath::unpackHalf4x16(p1);
		rblxmath::u16vec4 p2 = rblxmath::packHalf(v0);
		rblxmath::vec4 v2 = rblxmath::unpackHalf(p2);

		Error += rblxmath::all(rblxmath::equal(v0, v1, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v0, v2, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	return Error;
}

static int test_I3x10_1x2()
{
	int Error = 0;

	std::vector<rblxmath::ivec4> Tests;
	Tests.push_back(rblxmath::ivec4(0));
	Tests.push_back(rblxmath::ivec4(1));
	Tests.push_back(rblxmath::ivec4(-1));
	Tests.push_back(rblxmath::ivec4(2));
	Tests.push_back(rblxmath::ivec4(-2));
	Tests.push_back(rblxmath::ivec4(3));

	for(std::size_t i = 0; i < Tests.size(); ++i)
	{
		rblxmath::uint32 p0 = rblxmath::packI3x10_1x2(Tests[i]);
		rblxmath::ivec4 v0 = rblxmath::unpackI3x10_1x2(p0);
		rblxmath::uint32 p1 = rblxmath::packI3x10_1x2(v0);
		rblxmath::ivec4 v1 = rblxmath::unpackI3x10_1x2(p1);
		Error += rblxmath::all(rblxmath::equal(v0, v1)) ? 0 : 1;
	}

	return Error;
}

static int test_U3x10_1x2()
{
	int Error = 0;

	std::vector<rblxmath::uvec4> Tests;
	Tests.push_back(rblxmath::uvec4(0));
	Tests.push_back(rblxmath::uvec4(1));
	Tests.push_back(rblxmath::uvec4(2));
	Tests.push_back(rblxmath::uvec4(3));
	Tests.push_back(rblxmath::uvec4(4));
	Tests.push_back(rblxmath::uvec4(5));

	for(std::size_t i = 0; i < Tests.size(); ++i)
	{
		rblxmath::uint32 p0 = rblxmath::packU3x10_1x2(Tests[i]);
		rblxmath::uvec4 v0 = rblxmath::unpackU3x10_1x2(p0);
		rblxmath::uint32 p1 = rblxmath::packU3x10_1x2(v0);
		rblxmath::uvec4 v1 = rblxmath::unpackU3x10_1x2(p1);
		Error += rblxmath::all(rblxmath::equal(v0, v1)) ? 0 : 1;
	}

	rblxmath::u8vec4 const v0(0xff, 0x77, 0x0, 0x33);
	rblxmath::uint32 const p0 = *reinterpret_cast<rblxmath::uint32 const*>(&v0[0]);
	rblxmath::uint32 const r0 = 0x330077ff;

	Error += p0 == r0 ? 0 : 1;

	rblxmath::uvec4 const v1(0xff, 0x77, 0x0, 0x33);
	rblxmath::uint32 const p1 = rblxmath::packU3x10_1x2(v1);
	rblxmath::uint32 const r1 = 0xc001dcff;

	Error += p1 == r1 ? 0 : 1;

	return Error;
}

static int test_Snorm3x10_1x2()
{
	int Error = 0;

	std::vector<rblxmath::vec4> Tests;
	Tests.push_back(rblxmath::vec4(1.0f));
	Tests.push_back(rblxmath::vec4(0.0f));
	Tests.push_back(rblxmath::vec4(2.0f));
	Tests.push_back(rblxmath::vec4(0.1f));
	Tests.push_back(rblxmath::vec4(0.5f));
	Tests.push_back(rblxmath::vec4(0.9f));

	for(std::size_t i = 0; i < Tests.size(); ++i)
	{
		rblxmath::uint32 p0 = rblxmath::packSnorm3x10_1x2(Tests[i]);
		rblxmath::vec4 v0 = rblxmath::unpackSnorm3x10_1x2(p0);
		rblxmath::uint32 p1 = rblxmath::packSnorm3x10_1x2(v0);
		rblxmath::vec4 v1 = rblxmath::unpackSnorm3x10_1x2(p1);

		Error += rblxmath::all(rblxmath::epsilonEqual(v0, v1, 0.01f)) ? 0 : 1;
	}

	return Error;
}

static int test_Unorm3x10_1x2()
{
	int Error = 0;

	std::vector<rblxmath::vec4> Tests;
	Tests.push_back(rblxmath::vec4(1.0f));
	Tests.push_back(rblxmath::vec4(0.0f));
	Tests.push_back(rblxmath::vec4(2.0f));
	Tests.push_back(rblxmath::vec4(0.1f));
	Tests.push_back(rblxmath::vec4(0.5f));
	Tests.push_back(rblxmath::vec4(0.9f));

	for(std::size_t i = 0; i < Tests.size(); ++i)
	{
		rblxmath::uint32 p0 = rblxmath::packUnorm3x10_1x2(Tests[i]);
		rblxmath::vec4 v0 = rblxmath::unpackUnorm3x10_1x2(p0);
		rblxmath::uint32 p1 = rblxmath::packUnorm3x10_1x2(v0);
		rblxmath::vec4 v1 = rblxmath::unpackUnorm3x10_1x2(p1);

		Error += rblxmath::all(rblxmath::epsilonEqual(v0, v1, 0.001f)) ? 0 : 1;
	}

	return Error;
}

static int test_F2x11_1x10()
{
	int Error = 0;

	std::vector<rblxmath::vec3> Tests;
	Tests.push_back(rblxmath::vec3(1.0f));
	Tests.push_back(rblxmath::vec3(0.0f));
	Tests.push_back(rblxmath::vec3(2.0f));
	Tests.push_back(rblxmath::vec3(0.1f));
	Tests.push_back(rblxmath::vec3(0.5f));
	Tests.push_back(rblxmath::vec3(0.9f));

	for(std::size_t i = 0; i < Tests.size(); ++i)
	{
		rblxmath::uint32 p0 = rblxmath::packF2x11_1x10(Tests[i]);
		rblxmath::vec3 v0 = rblxmath::unpackF2x11_1x10(p0);
		rblxmath::uint32 p1 = rblxmath::packF2x11_1x10(v0);
		rblxmath::vec3 v1 = rblxmath::unpackF2x11_1x10(p1);
		Error += rblxmath::all(rblxmath::equal(v0, v1, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	return Error;
}

static int test_F3x9_E1x5()
{
	int Error = 0;

	std::vector<rblxmath::vec3> Tests;
	Tests.push_back(rblxmath::vec3(1.0f));
	Tests.push_back(rblxmath::vec3(0.0f));
	Tests.push_back(rblxmath::vec3(2.0f));
	Tests.push_back(rblxmath::vec3(0.1f));
	Tests.push_back(rblxmath::vec3(0.5f));
	Tests.push_back(rblxmath::vec3(0.9f));

	for(std::size_t i = 0; i < Tests.size(); ++i)
	{
		rblxmath::uint32 p0 = rblxmath::packF3x9_E1x5(Tests[i]);
		rblxmath::vec3 v0 = rblxmath::unpackF3x9_E1x5(p0);
		rblxmath::uint32 p1 = rblxmath::packF3x9_E1x5(v0);
		rblxmath::vec3 v1 = rblxmath::unpackF3x9_E1x5(p1);
		Error += rblxmath::all(rblxmath::equal(v0, v1, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	return Error;
}

static int test_RGBM()
{
	int Error = 0;

	for(std::size_t i = 0; i < 1024; ++i)
	{
		rblxmath::vec3 const Color(static_cast<float>(i));
		rblxmath::vec4 const RGBM = rblxmath::packRGBM(Color);
		rblxmath::vec3 const Result= rblxmath::unpackRGBM(RGBM);

		Error += rblxmath::all(rblxmath::equal(Color, Result, 0.01f)) ? 0 : 1;
	}

	return Error;
}

static int test_packUnorm1x16()
{
	int Error = 0;

	std::vector<rblxmath::vec1> A;
	A.push_back(rblxmath::vec1(1.0f));
	A.push_back(rblxmath::vec1(0.5f));
	A.push_back(rblxmath::vec1(0.1f));
	A.push_back(rblxmath::vec1(0.0f));

	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec1 B(A[i]);
		rblxmath::uint16 C = rblxmath::packUnorm1x16(B.x);
		rblxmath::vec1 D(rblxmath::unpackUnorm1x16(C));
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 65535.f)) ? 0 : 1;
		assert(!Error);
	}
	
	return Error;
}

static int test_packSnorm1x16()
{
	int Error = 0;

	std::vector<rblxmath::vec1> A;
	A.push_back(rblxmath::vec1( 1.0f));
	A.push_back(rblxmath::vec1( 0.0f));
	A.push_back(rblxmath::vec1(-0.5f));
	A.push_back(rblxmath::vec1(-0.1f));

	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec1 B(A[i]);
		rblxmath::uint16 C = rblxmath::packSnorm1x16(B.x);
		rblxmath::vec1 D(rblxmath::unpackSnorm1x16(C));
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 32767.0f * 2.0f)) ? 0 : 1;
	}
	
	return Error;
}

static int test_packUnorm2x16()
{
	int Error = 0;

	std::vector<rblxmath::vec2> A;
	A.push_back(rblxmath::vec2(1.0f, 0.0f));
	A.push_back(rblxmath::vec2(0.5f, 0.7f));
	A.push_back(rblxmath::vec2(0.1f, 0.2f));

	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec2 B(A[i]);
		rblxmath::uint32 C = rblxmath::packUnorm2x16(B);
		rblxmath::vec2 D = rblxmath::unpackUnorm2x16(C);
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 65535.f)) ? 0 : 1;
		assert(!Error);
	}
	
	return Error;
}

static int test_packSnorm2x16()
{
	int Error = 0;

	std::vector<rblxmath::vec2> A;
	A.push_back(rblxmath::vec2( 1.0f, 0.0f));
	A.push_back(rblxmath::vec2(-0.5f,-0.7f));
	A.push_back(rblxmath::vec2(-0.1f, 0.1f));

	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec2 B(A[i]);
		rblxmath::uint32 C = rblxmath::packSnorm2x16(B);
		rblxmath::vec2 D = rblxmath::unpackSnorm2x16(C);
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 32767.0f * 2.0f)) ? 0 : 1;
		assert(!Error);
	}
	
	return Error;
}

static int test_packUnorm4x16()
{
	int Error = 0;

	std::vector<rblxmath::vec4> A;
	A.push_back(rblxmath::vec4(1.0f));
	A.push_back(rblxmath::vec4(0.5f));
	A.push_back(rblxmath::vec4(0.1f));
	A.push_back(rblxmath::vec4(0.0f));

	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec4 B(A[i]);
		rblxmath::uint64 C = rblxmath::packUnorm4x16(B);
		rblxmath::vec4 D(rblxmath::unpackUnorm4x16(C));
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 65535.f)) ? 0 : 1;
		assert(!Error);
	}
	
	return Error;
}

static int test_packSnorm4x16()
{
	int Error = 0;

	std::vector<rblxmath::vec4> A;
	A.push_back(rblxmath::vec4( 1.0f, 0.0f, -0.5f, 0.5f));
	A.push_back(rblxmath::vec4(-0.3f,-0.7f,  0.3f, 0.7f));
	A.push_back(rblxmath::vec4(-0.1f, 0.1f, -0.2f, 0.2f));

	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec4 B(A[i]);
		rblxmath::uint64 C = rblxmath::packSnorm4x16(B);
		rblxmath::vec4 D(rblxmath::unpackSnorm4x16(C));
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 32767.0f * 2.0f)) ? 0 : 1;
		assert(!Error);
	}
	
	return Error;
}

static int test_packUnorm1x8()
{
	int Error = 0;
	
	std::vector<rblxmath::vec1> A;
	A.push_back(rblxmath::vec1(1.0f));
	A.push_back(rblxmath::vec1(0.5f));
	A.push_back(rblxmath::vec1(0.0f));
	
	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec1 B(A[i]);
		rblxmath::uint8 C = rblxmath::packUnorm1x8(B.x);
		rblxmath::vec1 D(rblxmath::unpackUnorm1x8(C));
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 255.f)) ? 0 : 1;
		assert(!Error);
	}
	
	return Error;
}

static int test_packSnorm1x8()
{
	int Error = 0;
	
	std::vector<rblxmath::vec1> A;
	A.push_back(rblxmath::vec1( 1.0f));
	A.push_back(rblxmath::vec1(-0.7f));
	A.push_back(rblxmath::vec1(-1.0f));
	
	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec1 B(A[i]);
		rblxmath::uint8 C = rblxmath::packSnorm1x8(B.x);
		rblxmath::vec1 D(rblxmath::unpackSnorm1x8(C));
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 127.f)) ? 0 : 1;
	}
	
	return Error;
}

static int test_packUnorm2x8()
{
	int Error = 0;
	
	std::vector<rblxmath::vec2> A;
	A.push_back(rblxmath::vec2(1.0f, 0.7f));
	A.push_back(rblxmath::vec2(0.5f, 0.1f));
	
	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec2 B(A[i]);
		rblxmath::uint16 C = rblxmath::packUnorm2x8(B);
		rblxmath::vec2 D = rblxmath::unpackUnorm2x8(C);
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 255.f)) ? 0 : 1;
		assert(!Error);
	}
	
	return Error;
}

static int test_packSnorm2x8()
{
	int Error = 0;
	
	std::vector<rblxmath::vec2> A;
	A.push_back(rblxmath::vec2( 1.0f, 0.0f));
	A.push_back(rblxmath::vec2(-0.7f,-0.1f));
	
	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec2 B(A[i]);
		rblxmath::uint16 C = rblxmath::packSnorm2x8(B);
		rblxmath::vec2 D = rblxmath::unpackSnorm2x8(C);
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 127.f)) ? 0 : 1;
	}
	
	return Error;
}

static int test_packUnorm4x8()
{
	int Error = 0;
	
	std::vector<rblxmath::vec4> A;
	A.push_back(rblxmath::vec4(1.0f, 0.7f, 0.3f, 0.0f));
	A.push_back(rblxmath::vec4(0.5f, 0.1f, 0.2f, 0.3f));
	
	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec4 B(A[i]);
		rblxmath::uint32 C = rblxmath::packUnorm4x8(B);
		rblxmath::vec4 D = rblxmath::unpackUnorm4x8(C);
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 255.f)) ? 0 : 1;
		assert(!Error);
	}
	
	return Error;
}

static int test_packSnorm4x8()
{
	int Error = 0;
	
	std::vector<rblxmath::vec4> A;
	A.push_back(rblxmath::vec4( 1.0f, 0.0f,-0.5f,-1.0f));
	A.push_back(rblxmath::vec4(-0.7f,-0.1f, 0.1f, 0.7f));
	
	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec4 B(A[i]);
		rblxmath::uint32 C = rblxmath::packSnorm4x8(B);
		rblxmath::vec4 D = rblxmath::unpackSnorm4x8(C);
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 127.f)) ? 0 : 1;
		assert(!Error);
	}
	
	return Error;
}

static int test_packUnorm()
{
	int Error = 0;

	std::vector<rblxmath::vec2> A;
	A.push_back(rblxmath::vec2(1.0f, 0.7f));
	A.push_back(rblxmath::vec2(0.5f, 0.1f));

	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec2 B(A[i]);
		rblxmath::u16vec2 C = rblxmath::packUnorm<rblxmath::uint16>(B);
		rblxmath::vec2 D = rblxmath::unpackUnorm<float>(C);
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 255.f)) ? 0 : 1;
		assert(!Error);
	}

	return Error;
}

static int test_packSnorm()
{
	int Error = 0;

	std::vector<rblxmath::vec2> A;
	A.push_back(rblxmath::vec2( 1.0f, 0.0f));
	A.push_back(rblxmath::vec2(-0.5f,-0.7f));
	A.push_back(rblxmath::vec2(-0.1f, 0.1f));

	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec2 B(A[i]);
		rblxmath::i16vec2 C = rblxmath::packSnorm<rblxmath::int16>(B);
		rblxmath::vec2 D = rblxmath::unpackSnorm<float>(C);
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 32767.0f * 2.0f)) ? 0 : 1;
		assert(!Error);
	}

	return Error;
}

static int test_packUnorm2x4()
{
	int Error = 0;

	std::vector<rblxmath::vec2> A;
	A.push_back(rblxmath::vec2(1.0f, 0.7f));
	A.push_back(rblxmath::vec2(0.5f, 0.0f));

	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec2 B(A[i]);
		rblxmath::uint8 C = rblxmath::packUnorm2x4(B);
		rblxmath::vec2 D = rblxmath::unpackUnorm2x4(C);
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 15.f)) ? 0 : 1;
		assert(!Error);
	}

	return Error;
}

static int test_packUnorm4x4()
{
	int Error = 0;

	std::vector<rblxmath::vec4> A;
	A.push_back(rblxmath::vec4(1.0f, 0.7f, 0.5f, 0.0f));
	A.push_back(rblxmath::vec4(0.5f, 0.1f, 0.0f, 1.0f));

	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec4 B(A[i]);
		rblxmath::uint16 C = rblxmath::packUnorm4x4(B);
		rblxmath::vec4 D = rblxmath::unpackUnorm4x4(C);
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 15.f)) ? 0 : 1;
		assert(!Error);
	}

	return Error;
}

static int test_packUnorm3x5_1x1()
{
	int Error = 0;

	std::vector<rblxmath::vec4> A;
	A.push_back(rblxmath::vec4(1.0f, 0.7f, 0.5f, 0.0f));
	A.push_back(rblxmath::vec4(0.5f, 0.1f, 0.0f, 1.0f));

	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec4 B(A[i]);
		rblxmath::uint16 C = rblxmath::packUnorm3x5_1x1(B);
		rblxmath::vec4 D = rblxmath::unpackUnorm3x5_1x1(C);
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 15.f)) ? 0 : 1;
		assert(!Error);
	}

	return Error;
}

static int test_packUnorm1x5_1x6_1x5()
{
	int Error = 0;

	std::vector<rblxmath::vec3> A;
	A.push_back(rblxmath::vec3(1.0f, 0.7f, 0.5f));
	A.push_back(rblxmath::vec3(0.5f, 0.1f, 0.0f));

	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec3 B(A[i]);
		rblxmath::uint16 C = rblxmath::packUnorm1x5_1x6_1x5(B);
		rblxmath::vec3 D = rblxmath::unpackUnorm1x5_1x6_1x5(C);
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 15.f)) ? 0 : 1;
		assert(!Error);
	}

	return Error;
}

static int test_packUnorm2x3_1x2()
{
	int Error = 0;

	std::vector<rblxmath::vec3> A;
	A.push_back(rblxmath::vec3(1.0f, 0.7f, 0.5f));
	A.push_back(rblxmath::vec3(0.5f, 0.1f, 0.0f));

	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec3 B(A[i]);
		rblxmath::uint8 C = rblxmath::packUnorm2x3_1x2(B);
		rblxmath::vec3 D = rblxmath::unpackUnorm2x3_1x2(C);
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 3.f)) ? 0 : 1;
		assert(!Error);
	}

	return Error;
}

static int test_packUint2x8()
{
	int Error = 0;

	rblxmath::u8vec2 const Source(1, 2);

	rblxmath::uint16 const Packed = rblxmath::packUint2x8(Source);
	Error += Packed != 0 ? 0 : 1;

	rblxmath::u8vec2 const Unpacked = rblxmath::unpackUint2x8(Packed);
	Error += Source == Unpacked ? 0 : 1;

	return Error;
}

static int test_packUint4x8()
{
	int Error = 0;

	rblxmath::u8vec4 const Source(1, 2, 3, 4);

	rblxmath::uint32 const Packed = rblxmath::packUint4x8(Source);
	Error += Packed != 0 ? 0 : 1;

	rblxmath::u8vec4 const Unpacked = rblxmath::unpackUint4x8(Packed);
	Error += Source == Unpacked ? 0 : 1;

	return Error;
}

static int test_packUint2x16()
{
	int Error = 0;

	rblxmath::u16vec2 const Source(1, 2);

	rblxmath::uint32 const Packed = rblxmath::packUint2x16(Source);
	Error += Packed != 0 ? 0 : 1;

	rblxmath::u16vec2 const Unpacked = rblxmath::unpackUint2x16(Packed);
	Error += Source == Unpacked ? 0 : 1;

	return Error;
}

static int test_packUint4x16()
{
	int Error = 0;

	rblxmath::u16vec4 const Source(1, 2, 3, 4);

	rblxmath::uint64 const Packed = rblxmath::packUint4x16(Source);
	Error += Packed != 0 ? 0 : 1;

	rblxmath::u16vec4 const Unpacked = rblxmath::unpackUint4x16(Packed);
	Error += Source == Unpacked ? 0 : 1;

	return Error;
}

static int test_packUint2x32()
{
	int Error = 0;

	rblxmath::u32vec2 const Source(1, 2);

	rblxmath::uint64 const Packed = rblxmath::packUint2x32(Source);
	Error += Packed != 0 ? 0 : 1;

	rblxmath::u32vec2 const Unpacked = rblxmath::unpackUint2x32(Packed);
	Error += Source == Unpacked ? 0 : 1;

	return Error;
}

static int test_packInt2x8()
{
	int Error = 0;

	rblxmath::i8vec2 const Source(1, 2);

	rblxmath::int16 const Packed = rblxmath::packInt2x8(Source);
	Error += Packed != 0 ? 0 : 1;

	rblxmath::i8vec2 const Unpacked = rblxmath::unpackInt2x8(Packed);
	Error += Source == Unpacked ? 0 : 1;

	return Error;
}

static int test_packInt4x8()
{
	int Error = 0;

	rblxmath::i8vec4 const Source(1, 2, 3, 4);

	rblxmath::int32 const Packed = rblxmath::packInt4x8(Source);
	Error += Packed != 0 ? 0 : 1;

	rblxmath::i8vec4 const Unpacked = rblxmath::unpackInt4x8(Packed);
	Error += Source == Unpacked ? 0 : 1;

	return Error;
}

static int test_packInt2x16()
{
	int Error = 0;

	rblxmath::i16vec2 const Source(1, 2);

	rblxmath::int32 const Packed = rblxmath::packInt2x16(Source);
	Error += Packed != 0 ? 0 : 1;

	rblxmath::i16vec2 const Unpacked = rblxmath::unpackInt2x16(Packed);
	Error += Source == Unpacked ? 0 : 1;

	return Error;
}

static int test_packInt4x16()
{
	int Error = 0;

	rblxmath::i16vec4 const Source(1, 2, 3, 4);

	rblxmath::int64 const Packed = rblxmath::packInt4x16(Source);
	Error += Packed != 0 ? 0 : 1;

	rblxmath::i16vec4 const Unpacked = rblxmath::unpackInt4x16(Packed);
	Error += Source == Unpacked ? 0 : 1;

	return Error;
}

static int test_packInt2x32()
{
	int Error = 0;

	rblxmath::i32vec2 const Source(1, 2);

	rblxmath::int64 const Packed = rblxmath::packInt2x32(Source);
	Error += Packed != 0 ? 0 : 1;

	rblxmath::i32vec2 const Unpacked = rblxmath::unpackInt2x32(Packed);
	Error += Source == Unpacked ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_packUnorm();
	Error += test_packSnorm();

	Error += test_packSnorm1x16();
	Error += test_packSnorm2x16();
	Error += test_packSnorm4x16();

	Error += test_packSnorm1x8();
	Error += test_packSnorm2x8();
	Error += test_packSnorm4x8();

	Error += test_packUnorm1x16();
	Error += test_packUnorm2x16();
	Error += test_packUnorm4x16();

	Error += test_packUnorm1x8();
	Error += test_packUnorm2x8();
	Error += test_packUnorm4x8();

	Error += test_packUnorm2x4();
	Error += test_packUnorm4x4();
	Error += test_packUnorm3x5_1x1();
	Error += test_packUnorm1x5_1x6_1x5();
	Error += test_packUnorm2x3_1x2();

	Error += test_packUint2x8();
	Error += test_packUint4x8();
	Error += test_packUint2x16();
	Error += test_packUint4x16();
	Error += test_packUint2x32();

	Error += test_packInt2x8();
	Error += test_packInt4x8();
	Error += test_packInt2x16();
	Error += test_packInt4x16();
	Error += test_packInt2x32();

	Error += test_F2x11_1x10();
	Error += test_F3x9_E1x5();
	Error += test_RGBM();
	Error += test_Unorm3x10_1x2();
	Error += test_Snorm3x10_1x2();

	Error += test_I3x10_1x2();
	Error += test_U3x10_1x2();
	Error += test_Half1x16();
	Error += test_Half4x16();

	return Error;
}
