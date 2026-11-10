#include <rblxmath/gtc/type_precision.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/vector_relational.hpp>
#include <rblxmath/packing.hpp>
#include <vector>

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

static int test_packUnorm4x8()
{
	int Error = 0;

	rblxmath::uint32 Packed = rblxmath::packUnorm4x8(rblxmath::vec4(1.0f, 0.5f, 0.0f, 1.0f));
	rblxmath::u8vec4 Vec(255, 128, 0, 255);
	rblxmath::uint32 & Ref = *reinterpret_cast<rblxmath::uint32*>(&Vec[0]);

	Error += Packed == Ref ? 0 : 1;

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

static int test_packHalf2x16()
{
	int Error = 0;
/*
	std::vector<rblxmath::hvec2> A;
	A.push_back(rblxmath::hvec2(rblxmath::half( 1.0f), rblxmath::half( 2.0f)));
	A.push_back(rblxmath::hvec2(rblxmath::half(-1.0f), rblxmath::half(-2.0f)));
	A.push_back(rblxmath::hvec2(rblxmath::half(-1.1f), rblxmath::half( 1.1f)));
*/
	std::vector<rblxmath::vec2> A;
	A.push_back(rblxmath::vec2( 1.0f, 2.0f));
	A.push_back(rblxmath::vec2(-1.0f,-2.0f));
	A.push_back(rblxmath::vec2(-1.1f, 1.1f));

	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec2 B(A[i]);
		rblxmath::uint C = rblxmath::packHalf2x16(B);
		rblxmath::vec2 D = rblxmath::unpackHalf2x16(C);
		//Error += B == D ? 0 : 1;
		Error += rblxmath::all(rblxmath::epsilonEqual(B, D, 1.0f / 127.f)) ? 0 : 1;
		assert(!Error);
	}
	
	return Error;
}

static int test_packDouble2x32()
{
	int Error = 0;
	
	std::vector<rblxmath::uvec2> A;
	A.push_back(rblxmath::uvec2( 1, 2));
	A.push_back(rblxmath::uvec2(-1,-2));
	A.push_back(rblxmath::uvec2(-1000, 1100));
	
	for(std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::uvec2 B(A[i]);
		double C = rblxmath::packDouble2x32(B);
		rblxmath::uvec2 D = rblxmath::unpackDouble2x32(C);
		Error += B == D ? 0 : 1;
		assert(!Error);
	}
	
	return Error;
}

int main()
{
	int Error = 0;
	
	Error += test_packSnorm4x8();
	Error += test_packUnorm4x8();
	Error += test_packSnorm2x16();
	Error += test_packUnorm2x16();
	Error += test_packHalf2x16();
	Error += test_packDouble2x32();

	return Error;
}

