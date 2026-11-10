#include <rblxmath/ext/scalar_uint_sized.hpp>

#if RBLXMATH_HAS_STATIC_ASSERT
	static_assert(sizeof(rblxmath::uint8) == 1, "uint8 size isn't 1 byte on this platform");
	static_assert(sizeof(rblxmath::uint16) == 2, "uint16 size isn't 2 bytes on this platform");
	static_assert(sizeof(rblxmath::uint32) == 4, "uint32 size isn't 4 bytes on this platform");
	static_assert(sizeof(rblxmath::uint64) == 8, "uint64 size isn't 8 bytes on this platform");
	static_assert(sizeof(rblxmath::uint16) == sizeof(unsigned short), "unsigned short size isn't 4 bytes on this platform");
	static_assert(sizeof(rblxmath::uint32) == sizeof(unsigned int), "unsigned int size isn't 4 bytes on this platform");
#endif

static int test_size()
{
	int Error = 0;

	Error += sizeof(rblxmath::uint8) == 1 ? 0 : 1;
	Error += sizeof(rblxmath::uint16) == 2 ? 0 : 1;
	Error += sizeof(rblxmath::uint32) == 4 ? 0 : 1;
	Error += sizeof(rblxmath::uint64) == 8 ? 0 : 1;

	return Error;
}

static int test_comp()
{
	int Error = 0;

	Error += sizeof(rblxmath::uint8) < sizeof(rblxmath::uint16) ? 0 : 1;
	Error += sizeof(rblxmath::uint16) < sizeof(rblxmath::uint32) ? 0 : 1;
	Error += sizeof(rblxmath::uint32) < sizeof(rblxmath::uint64) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_size();
	Error += test_comp();

	return Error;
}
