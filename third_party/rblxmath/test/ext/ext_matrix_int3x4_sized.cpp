#include <rblxmath/ext/matrix_int3x4_sized.hpp>

#if RBLXMATH_HAS_STATIC_ASSERT
static_assert(sizeof(rblxmath::i8mat3x4) == 12, "int8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::i16mat3x4) == 24, "int16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::i32mat3x4) == 48, "int32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::i64mat3x4) == 96, "int64 size isn't 8 bytes on this platform");
#endif

static int test_comp()
{
	int Error = 0;

	Error += sizeof(rblxmath::i8mat3x4) < sizeof(rblxmath::i16mat3x4) ? 0 : 1;
	Error += sizeof(rblxmath::i16mat3x4) < sizeof(rblxmath::i32mat3x4) ? 0 : 1;
	Error += sizeof(rblxmath::i32mat3x4) < sizeof(rblxmath::i64mat3x4) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_comp();

	return Error;
}
