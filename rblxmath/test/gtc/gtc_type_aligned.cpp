#include <rblxmath/rblxmath.hpp>

#if RBLXMATH_CONFIG_ALIGNED_GENTYPES == RBLXMATH_ENABLE
#include <rblxmath/gtc/type_aligned.hpp>
#include <rblxmath/gtc/type_precision.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/matrix_relational.hpp>

RBLXMATH_STATIC_ASSERT(rblxmath::detail::is_aligned<rblxmath::aligned_lowp>::value, "aligned_lowp is not aligned");
RBLXMATH_STATIC_ASSERT(rblxmath::detail::is_aligned<rblxmath::aligned_mediump>::value, "aligned_mediump is not aligned");
RBLXMATH_STATIC_ASSERT(rblxmath::detail::is_aligned<rblxmath::aligned_highp>::value, "aligned_highp is not aligned");
RBLXMATH_STATIC_ASSERT(!rblxmath::detail::is_aligned<rblxmath::packed_highp>::value, "packed_highp is aligned");
RBLXMATH_STATIC_ASSERT(!rblxmath::detail::is_aligned<rblxmath::packed_mediump>::value, "packed_mediump is aligned");
RBLXMATH_STATIC_ASSERT(!rblxmath::detail::is_aligned<rblxmath::packed_lowp>::value, "packed_lowp is aligned");

struct my_vec4_packed
{
	rblxmath::uint32 a;
	rblxmath::vec4 b;
};
RBLXMATH_STATIC_ASSERT(sizeof(my_vec4_packed) == sizeof(rblxmath::uint32) + sizeof(rblxmath::vec4), "rblxmath::vec4 packed is not correct");

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wpadded"
#endif

struct my_vec4_aligned
{
	rblxmath::uint32 a;
	rblxmath::aligned_vec4 b;
};
RBLXMATH_STATIC_ASSERT(sizeof(my_vec4_aligned) == sizeof(rblxmath::aligned_vec4) * 2, "rblxmath::vec4 aligned is not correct");

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

struct my_dvec4_packed
{
	rblxmath::uint64 a;
	rblxmath::dvec4 b;
};
RBLXMATH_STATIC_ASSERT(sizeof(my_dvec4_packed) == sizeof(rblxmath::uint64) + sizeof(rblxmath::dvec4), "rblxmath::dvec4 packed is not correct");

struct my_dvec4_aligned
{
	rblxmath::uint64 a;
	rblxmath::aligned_dvec4 b;
};
//RBLXMATH_STATIC_ASSERT(sizeof(my_dvec4_aligned) == sizeof(rblxmath::aligned_dvec4) * 2, "rblxmath::dvec4 aligned is not correct");

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wpadded"
#endif

struct my_ivec4_packed
{
	rblxmath::uint32 a;
	rblxmath::ivec4 b;
};

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

RBLXMATH_STATIC_ASSERT(sizeof(my_ivec4_packed) == sizeof(rblxmath::uint32) + sizeof(rblxmath::ivec4), "rblxmath::ivec4 packed is not correct");

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wpadded"
#endif

struct my_ivec4_aligned
{
	rblxmath::uint32 a;
	rblxmath::aligned_ivec4 b;
};

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

RBLXMATH_STATIC_ASSERT(sizeof(my_ivec4_aligned) == sizeof(rblxmath::aligned_ivec4) * 2, "rblxmath::ivec4 aligned is not correct");

struct my_u8vec4_packed
{
	rblxmath::uint32 a;
	rblxmath::u8vec4 b;
};
RBLXMATH_STATIC_ASSERT(sizeof(my_u8vec4_packed) == sizeof(rblxmath::uint32) + sizeof(rblxmath::u8vec4), "rblxmath::u8vec4 packed is not correct");

static int test_copy_vec4()
{
	int Error = 0;
	{
		rblxmath::aligned_vec4 const u(1.f, 2.f, 3.f, 4.f);
		rblxmath::packed_vec4 const v(u);
		Error += rblxmath::equal(v.x, u.x, rblxmath::epsilon<float>()) ? 0 : 1;
		Error += rblxmath::equal(v.y, u.y, rblxmath::epsilon<float>()) ? 0 : 1;
		Error += rblxmath::equal(v.z, u.z, rblxmath::epsilon<float>()) ? 0 : 1;
		Error += rblxmath::equal(v.w, u.w, rblxmath::epsilon<float>()) ? 0 : 1;
	}
	{
		rblxmath::packed_vec4 const u(1.f, 2.f, 3.f, 4.f);
		rblxmath::aligned_vec4 const v(u);
		Error += rblxmath::equal(v.x, u.x, rblxmath::epsilon<float>()) ? 0 : 1;
		Error += rblxmath::equal(v.y, u.y, rblxmath::epsilon<float>()) ? 0 : 1;
		Error += rblxmath::equal(v.z, u.z, rblxmath::epsilon<float>()) ? 0 : 1;
		Error += rblxmath::equal(v.w, u.w, rblxmath::epsilon<float>()) ? 0 : 1;
	}

	{
		rblxmath::aligned_dvec4 const u(1., 2., 3., 4.);
		rblxmath::packed_dvec4 const v(u);
		Error += rblxmath::equal(v.x, u.x, rblxmath::epsilon<double>()) ? 0 : 1;
		Error += rblxmath::equal(v.y, u.y, rblxmath::epsilon<double>()) ? 0 : 1;
		Error += rblxmath::equal(v.z, u.z, rblxmath::epsilon<double>()) ? 0 : 1;
		Error += rblxmath::equal(v.w, u.w, rblxmath::epsilon<double>()) ? 0 : 1;
	}
	{
		rblxmath::packed_dvec4 const u(1.f, 2.f, 3.f, 4.f);
		rblxmath::aligned_dvec4 const v(u);
		Error += rblxmath::equal(v.x, u.x, rblxmath::epsilon<double>()) ? 0 : 1;
		Error += rblxmath::equal(v.y, u.y, rblxmath::epsilon<double>()) ? 0 : 1;
		Error += rblxmath::equal(v.z, u.z, rblxmath::epsilon<double>()) ? 0 : 1;
		Error += rblxmath::equal(v.w, u.w, rblxmath::epsilon<double>()) ? 0 : 1;
	}

	{
		rblxmath::aligned_ivec4 const u(1, 2, 3, 4);
		rblxmath::packed_ivec4 const v(u);
		Error += v.x == u.x ? 0 : 1;
		Error += v.y == u.y ? 0 : 1;
		Error += v.z == u.z ? 0 : 1;
		Error += v.w == u.w ? 0 : 1;
	}
	{
		rblxmath::packed_ivec4 const u(1, 2, 3, 4);
		rblxmath::aligned_ivec4 const v(u);
		Error += v.x == u.x ? 0 : 1;
		Error += v.y == u.y ? 0 : 1;
		Error += v.z == u.z ? 0 : 1;
		Error += v.w == u.w ? 0 : 1;
	}

	return Error;
}

static int test_copy_vec3()
{
	int Error = 0;
	{
		rblxmath::aligned_vec3 const u(1.f, 2.f, 3.f);
		rblxmath::packed_vec3 const v(u);
		Error += rblxmath::equal(v.x, u.x, rblxmath::epsilon<float>()) ? 0 : 1;
		Error += rblxmath::equal(v.y, u.y, rblxmath::epsilon<float>()) ? 0 : 1;
		Error += rblxmath::equal(v.z, u.z, rblxmath::epsilon<float>()) ? 0 : 1;
	}
	{
		rblxmath::packed_vec3 const u(1.f, 2.f, 3.f);
		rblxmath::aligned_vec3 const v(u);
		Error += rblxmath::equal(v.x, u.x, rblxmath::epsilon<float>()) ? 0 : 1;
		Error += rblxmath::equal(v.y, u.y, rblxmath::epsilon<float>()) ? 0 : 1;
		Error += rblxmath::equal(v.z, u.z, rblxmath::epsilon<float>()) ? 0 : 1;
	}

	{
		rblxmath::aligned_dvec3 const u(1., 2., 3.);
		rblxmath::packed_dvec3 const v(u);
		Error += rblxmath::equal(v.x, u.x, rblxmath::epsilon<double>()) ? 0 : 1;
		Error += rblxmath::equal(v.y, u.y, rblxmath::epsilon<double>()) ? 0 : 1;
		Error += rblxmath::equal(v.z, u.z, rblxmath::epsilon<double>()) ? 0 : 1;
	}
	{
		rblxmath::packed_dvec3 const u(1.f, 2.f, 3.f);
		rblxmath::aligned_dvec3 const v(u);
		Error += rblxmath::equal(v.x, u.x, rblxmath::epsilon<double>()) ? 0 : 1;
		Error += rblxmath::equal(v.y, u.y, rblxmath::epsilon<double>()) ? 0 : 1;
		Error += rblxmath::equal(v.z, u.z, rblxmath::epsilon<double>()) ? 0 : 1;
	}

	{
		rblxmath::aligned_ivec3 const u(1, 2, 3);
		rblxmath::packed_ivec3 const v(u);
		Error += v.x == u.x ? 0 : 1;
		Error += v.y == u.y ? 0 : 1;
		Error += v.z == u.z ? 0 : 1;
	}
	{
		rblxmath::packed_ivec3 const u(1, 2, 3);
		rblxmath::aligned_ivec3 const v(u);
		Error += v.x == u.x ? 0 : 1;
		Error += v.y == u.y ? 0 : 1;
		Error += v.z == u.z ? 0 : 1;
	}

	return Error;
}

static int test_copy()
{
	int Error = 0;

	{
		rblxmath::aligned_ivec4 const a(1, 2, 3, 4);
		rblxmath::ivec4 const u(a);

		Error += a.x == u.x ? 0 : 1;
		Error += a.y == u.y ? 0 : 1;
		Error += a.z == u.z ? 0 : 1;
		Error += a.w == u.w ? 0 : 1;
	}

	{
		my_ivec4_aligned a;
		a.b = rblxmath::ivec4(1, 2, 3, 4);

		my_ivec4_packed u;
		u.b = a.b;

		Error += a.b.x == u.b.x ? 0 : 1;
		Error += a.b.y == u.b.y ? 0 : 1;
		Error += a.b.z == u.b.z ? 0 : 1;
		Error += a.b.w == u.b.w ? 0 : 1;
	}

	return Error;
}

static int test_ctor()
{
	int Error = 0;

#	if RBLXMATH_HAS_CONSTEXPR
	{
		constexpr rblxmath::aligned_ivec4 v(1);

		Error += v.x == 1 ? 0 : 1;
		Error += v.y == 1 ? 0 : 1;
		Error += v.z == 1 ? 0 : 1;
		Error += v.w == 1 ? 0 : 1;
	}

	{
		constexpr rblxmath::packed_ivec4 v(1);

		Error += v.x == 1 ? 0 : 1;
		Error += v.y == 1 ? 0 : 1;
		Error += v.z == 1 ? 0 : 1;
		Error += v.w == 1 ? 0 : 1;
	}

	{
		constexpr rblxmath::ivec4 v(1);

		Error += v.x == 1 ? 0 : 1;
		Error += v.y == 1 ? 0 : 1;
		Error += v.z == 1 ? 0 : 1;
		Error += v.w == 1 ? 0 : 1;
	}
#	endif//RBLXMATH_HAS_CONSTEXPR

	return Error;
}

static int test_aligned_ivec4()
{
	int Error = 0;

	rblxmath::aligned_ivec4 const v(1, 2, 3, 4);
	Error += rblxmath::all(rblxmath::equal(v, rblxmath::aligned_ivec4(1, 2, 3, 4))) ? 0 : 1;

	rblxmath::aligned_ivec4 const u = v * 2;
	Error += rblxmath::all(rblxmath::equal(u, rblxmath::aligned_ivec4(2, 4, 6, 8))) ? 0 : 1;

	return Error;
}

static int test_aligned_mat4()
{
	int Error = 0;

	rblxmath::aligned_vec4 const u(1.f, 2.f, 3.f, 4.f);
	Error += rblxmath::all(rblxmath::equal(u, rblxmath::aligned_vec4(1.f, 2.f, 3.f, 4.f), 0.0001f)) ? 0 : 1;

	rblxmath::aligned_vec4 const v(1, 2, 3, 4);
	Error += rblxmath::all(rblxmath::equal(v, rblxmath::aligned_vec4(1.f, 2.f, 3.f, 4.f), 0.0001f)) ? 0 : 1;

	rblxmath::aligned_mat4 const m(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
	rblxmath::aligned_mat4 const t = rblxmath::transpose(m);
	rblxmath::aligned_mat4 const expected = rblxmath::mat4(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15);
	Error += rblxmath::all(rblxmath::equal(t, expected, 0.0001f)) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_ctor();
	Error += test_copy();
	Error += test_copy_vec4();
	Error += test_copy_vec3();
	Error += test_aligned_ivec4();
	Error += test_aligned_mat4();

	return Error;
}

#else

int main()
{
	return 0;
}

#endif
