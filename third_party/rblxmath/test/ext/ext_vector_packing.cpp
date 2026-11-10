#include <rblxmath/ext/vector_packing.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/vector_uint2_sized.hpp>
#include <rblxmath/ext/vector_int2_sized.hpp>
#include <rblxmath/gtc/packing.hpp>
#include <rblxmath/vec2.hpp>
#include <vector>

int test_packUnorm()
{
	int Error = 0;

	std::vector<rblxmath::vec2> A;
	A.push_back(rblxmath::vec2(1.0f, 0.7f));
	A.push_back(rblxmath::vec2(0.5f, 0.1f));

	for (std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec2 B(A[i]);
		rblxmath::u16vec2 C = rblxmath::packUnorm<rblxmath::uint16>(B);
		rblxmath::vec2 D = rblxmath::unpackUnorm<float>(C);
		Error += rblxmath::all(rblxmath::equal(B, D, 1.0f / 255.f)) ? 0 : 1;
		assert(!Error);
	}

	return Error;
}

int test_packSnorm()
{
	int Error = 0;

	std::vector<rblxmath::vec2> A;
	A.push_back(rblxmath::vec2(1.0f, 0.0f));
	A.push_back(rblxmath::vec2(-0.5f, -0.7f));
	A.push_back(rblxmath::vec2(-0.1f, 0.1f));

	for (std::size_t i = 0; i < A.size(); ++i)
	{
		rblxmath::vec2 B(A[i]);
		rblxmath::i16vec2 C = rblxmath::packSnorm<rblxmath::int16>(B);
		rblxmath::vec2 D = rblxmath::unpackSnorm<float>(C);
		Error += rblxmath::all(rblxmath::equal(B, D, 1.0f / 32767.0f * 2.0f)) ? 0 : 1;
		assert(!Error);
	}

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_packUnorm();
	Error += test_packSnorm();

	return Error;
}
