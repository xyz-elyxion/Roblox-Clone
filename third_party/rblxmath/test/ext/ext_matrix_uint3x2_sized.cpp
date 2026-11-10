#include <rblxmath/ext/matrix_uint3x2_sized.hpp>

#if RBLXMATH_HAS_STATIC_ASSERT
static_assert(sizeof(rblxmath::u8mat3x2) == 6, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::u16mat3x2) == 12, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::u32mat3x2) == 24, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::u64mat3x2) == 48, "uint64 size isn't 8 bytes on this platform");
#endif

static int test_comp()
{
	int Error = 0;

	Error += sizeof(rblxmath::u8mat3x2) < sizeof(rblxmath::u16mat3x2) ? 0 : 1;
	Error += sizeof(rblxmath::u16mat3x2) < sizeof(rblxmath::u32mat3x2) ? 0 : 1;
	Error += sizeof(rblxmath::u32mat3x2) < sizeof(rblxmath::u64mat3x2) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_comp();

	return Error;
}
