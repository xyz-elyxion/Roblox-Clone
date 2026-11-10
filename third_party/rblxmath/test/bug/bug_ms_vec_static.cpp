#include <rblxmath/rblxmath.hpp>

#if RBLXMATH_CONFIG_ANONYMOUS_STRUCT == RBLXMATH_ENABLE
struct vec2;

struct _swizzle
{
	char _buffer[1];
};

struct vec2
{
	RBLXMATH_CONSTEXPR vec2() :
		x(0), y(0)
	{}

#if defined(_MSC_VER)
#	pragma warning(push)
#	pragma warning(disable: 4201)  // nonstandard extension used : nameless struct/union
#endif
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#	pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

	union
	{
		struct { float x, y; };
		struct { _swizzle xx; };
	};

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif
#if defined(_MSC_VER)
#	pragma warning(pop)
#endif
};
#endif

// Visual C++ has a bug generating the error: fatal error C1001: An internal error has occurred in the compiler.
// vec2 Bar;

int main()
{
	return 0;
}
