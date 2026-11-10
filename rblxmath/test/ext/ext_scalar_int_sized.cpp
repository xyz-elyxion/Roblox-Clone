#include <rblxmath/ext/scalar_int_sized.hpp>

#if RBLXMATH_HAS_STATIC_ASSERT
	static_assert(sizeof(rblxmath::int8) == 1, "int8 size isn't 1 byte on this platform");
	static_assert(sizeof(rblxmath::int16) == 2, "int16 size isn't 2 bytes on this platform");
	static_assert(sizeof(rblxmath::int32) == 4, "int32 size isn't 4 bytes on this platform");
	static_assert(sizeof(rblxmath::int64) == 8, "int64 size isn't 8 bytes on this platform");
	static_assert(sizeof(rblxmath::int16) == sizeof(short), "signed short size isn't 4 bytes on this platform");
	static_assert(sizeof(rblxmath::int32) == sizeof(int), "signed int size isn't 4 bytes on this platform");
#endif

static int test_size()
{
	int Error = 0;

	Error += sizeof(rblxmath::int8) == 1 ? 0 : 1;
	Error += sizeof(rblxmath::int16) == 2 ? 0 : 1;
	Error += sizeof(rblxmath::int32) == 4 ? 0 : 1;
	Error += sizeof(rblxmath::int64) == 8 ? 0 : 1;

	return Error;
}

static int test_comp()
{
	int Error = 0;

	Error += sizeof(rblxmath::int8) < sizeof(rblxmath::int16) ? 0 : 1;
	Error += sizeof(rblxmath::int16) < sizeof(rblxmath::int32) ? 0 : 1;
	Error += sizeof(rblxmath::int32) < sizeof(rblxmath::int64) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_size();
	Error += test_comp();

	return Error;
}
