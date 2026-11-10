#include <rblxmath/gtc/type_precision.hpp>
#include <rblxmath/gtc/quaternion.hpp>
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <vector>
#if RBLXMATH_HAS_OPENMP
#	include <omp.h>
#endif

#if RBLXMATH_HAS_STATIC_ASSERT
static_assert(sizeof(rblxmath::lowp_u8vec1) == 1, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::mediump_u8vec1) == 1, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::highp_u8vec1) == 1, "uint8 size isn't 1 byte on this platform");

static_assert(sizeof(rblxmath::lowp_u16vec1) == 2, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u16vec1) == 2, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u16vec1) == 2, "uint16 size isn't 2 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u32vec1) == 4, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u32vec1) == 4, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u32vec1) == 4, "uint32 size isn't 4 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u64vec1) == 8, "uint64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u64vec1) == 8, "uint64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u64vec1) == 8, "uint64 size isn't 8 bytes on this platform");


static_assert(sizeof(rblxmath::lowp_u8vec2) == 2, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::mediump_u8vec2) == 2, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::highp_u8vec2) == 2, "uint8 size isn't 1 byte on this platform");

static_assert(sizeof(rblxmath::lowp_u16vec2) == 4, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u16vec2) == 4, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u16vec2) == 4, "uint16 size isn't 2 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u32vec2) == 8, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u32vec2) == 8, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u32vec2) == 8, "uint32 size isn't 4 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u64vec2) == 16, "uint64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u64vec2) == 16, "uint64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u64vec2) == 16, "uint64 size isn't 8 bytes on this platform");


static_assert(sizeof(rblxmath::lowp_u8vec3) == 3, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::mediump_u8vec3) == 3, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::highp_u8vec3) == 3, "uint8 size isn't 1 byte on this platform");

static_assert(sizeof(rblxmath::lowp_u16vec3) == 6, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u16vec3) == 6, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u16vec3) == 6, "uint16 size isn't 2 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u32vec3) == 12, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u32vec3) == 12, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u32vec3) == 12, "uint32 size isn't 4 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u64vec3) == 24, "uint64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u64vec3) == 24, "uint64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u64vec3) == 24, "uint64 size isn't 8 bytes on this platform");


static_assert(sizeof(rblxmath::lowp_u8vec4) == 4, "int8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::mediump_u8vec4) == 4, "int8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::highp_u8vec4) == 4, "int8 size isn't 1 byte on this platform");

static_assert(sizeof(rblxmath::lowp_u16vec4) == 8, "int16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u16vec4) == 8, "int16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u16vec4) == 8, "int16 size isn't 2 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u32vec4) == 16, "int32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u32vec4) == 16, "int32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u32vec4) == 16, "int32 size isn't 4 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u64vec4) == 32, "int64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u64vec4) == 32, "int64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u64vec4) == 32, "int64 size isn't 8 bytes on this platform");


static_assert(sizeof(rblxmath::lowp_u8vec1) == 1, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::mediump_u8vec1) == 1, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::highp_u8vec1) == 1, "uint8 size isn't 1 byte on this platform");

static_assert(sizeof(rblxmath::lowp_u16vec1) == 2, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u16vec1) == 2, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u16vec1) == 2, "uint16 size isn't 2 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u32vec1) == 4, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u32vec1) == 4, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u32vec1) == 4, "uint32 size isn't 4 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u64vec1) == 8, "uint64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u64vec1) == 8, "uint64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u64vec1) == 8, "uint64 size isn't 8 bytes on this platform");


static_assert(sizeof(rblxmath::lowp_u8vec2) == 2, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::mediump_u8vec2) == 2, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::highp_u8vec2) == 2, "uint8 size isn't 1 byte on this platform");

static_assert(sizeof(rblxmath::lowp_u16vec2) == 4, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u16vec2) == 4, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u16vec2) == 4, "uint16 size isn't 2 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u32vec2) == 8, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u32vec2) == 8, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u32vec2) == 8, "uint32 size isn't 4 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u64vec2) == 16, "uint64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u64vec2) == 16, "uint64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u64vec2) == 16, "uint64 size isn't 8 bytes on this platform");


static_assert(sizeof(rblxmath::lowp_u8vec3) == 3, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::mediump_u8vec3) == 3, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::highp_u8vec3) == 3, "uint8 size isn't 1 byte on this platform");

static_assert(sizeof(rblxmath::lowp_u16vec3) == 6, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u16vec3) == 6, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u16vec3) == 6, "uint16 size isn't 2 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u32vec3) == 12, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u32vec3) == 12, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u32vec3) == 12, "uint32 size isn't 4 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u64vec3) == 24, "uint64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u64vec3) == 24, "uint64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u64vec3) == 24, "uint64 size isn't 8 bytes on this platform");


static_assert(sizeof(rblxmath::lowp_u8vec4) == 4, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::mediump_u8vec4) == 4, "uint8 size isn't 1 byte on this platform");
static_assert(sizeof(rblxmath::highp_u8vec4) == 4, "uint8 size isn't 1 byte on this platform");

static_assert(sizeof(rblxmath::lowp_u16vec4) == 8, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u16vec4) == 8, "uint16 size isn't 2 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u16vec4) == 8, "uint16 size isn't 2 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u32vec4) == 16, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u32vec4) == 16, "uint32 size isn't 4 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u32vec4) == 16, "uint32 size isn't 4 bytes on this platform");

static_assert(sizeof(rblxmath::lowp_u64vec4) == 32, "uint64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::mediump_u64vec4) == 32, "uint64 size isn't 8 bytes on this platform");
static_assert(sizeof(rblxmath::highp_u64vec4) == 32, "uint64 size isn't 8 bytes on this platform");

#endif

static int test_scalar_size()
{
	int Error = 0;

	Error += sizeof(rblxmath::int8) != 1;
	Error += sizeof(rblxmath::int16) != 2;
	Error += sizeof(rblxmath::int32) != 4;
	Error += sizeof(rblxmath::int64) != 8;
	Error += sizeof(rblxmath::uint8) != 1;
	Error += sizeof(rblxmath::uint16) != 2;
	Error += sizeof(rblxmath::uint32) != 4;
	Error += sizeof(rblxmath::uint64) != 8;
	Error += sizeof(rblxmath::float32) != 4;
	Error += sizeof(rblxmath::float64) != 8;

	Error += sizeof(rblxmath::lowp_int8) != 1;
	Error += sizeof(rblxmath::lowp_int16) != 2;
	Error += sizeof(rblxmath::lowp_int32) != 4;
	Error += sizeof(rblxmath::lowp_int64) != 8;
	Error += sizeof(rblxmath::lowp_uint8) != 1;
	Error += sizeof(rblxmath::lowp_uint16) != 2;
	Error += sizeof(rblxmath::lowp_uint32) != 4;
	Error += sizeof(rblxmath::lowp_uint64) != 8;
	Error += sizeof(rblxmath::lowp_float32) != 4;
	Error += sizeof(rblxmath::lowp_float64) != 8;

	Error += sizeof(rblxmath::mediump_int8) != 1;
	Error += sizeof(rblxmath::mediump_int16) != 2;
	Error += sizeof(rblxmath::mediump_int32) != 4;
	Error += sizeof(rblxmath::mediump_int64) != 8;
	Error += sizeof(rblxmath::mediump_uint8) != 1;
	Error += sizeof(rblxmath::mediump_uint16) != 2;
	Error += sizeof(rblxmath::mediump_uint32) != 4;
	Error += sizeof(rblxmath::mediump_uint64) != 8;
	Error += sizeof(rblxmath::mediump_float32) != 4;
	Error += sizeof(rblxmath::mediump_float64) != 8;

	Error += sizeof(rblxmath::highp_int8) != 1;
	Error += sizeof(rblxmath::highp_int16) != 2;
	Error += sizeof(rblxmath::highp_int32) != 4;
	Error += sizeof(rblxmath::highp_int64) != 8;
	Error += sizeof(rblxmath::highp_uint8) != 1;
	Error += sizeof(rblxmath::highp_uint16) != 2;
	Error += sizeof(rblxmath::highp_uint32) != 4;
	Error += sizeof(rblxmath::highp_uint64) != 8;
	Error += sizeof(rblxmath::highp_float32) != 4;
	Error += sizeof(rblxmath::highp_float64) != 8;

	return Error;
}

static int test_fvec_size()
{
	int Error = 0;

	Error += sizeof(rblxmath::f32vec2) != 8;
	Error += sizeof(rblxmath::f32vec3) != 12;
	Error += sizeof(rblxmath::f32vec4) != 16;
	Error += sizeof(rblxmath::f64vec2) != 16;
	Error += sizeof(rblxmath::f64vec3) != 24;
	Error += sizeof(rblxmath::f64vec4) != 32;

	Error += sizeof(rblxmath::lowp_f32vec2) != 8;
	Error += sizeof(rblxmath::lowp_f32vec3) != 12;
	Error += sizeof(rblxmath::lowp_f32vec4) != 16;
	Error += sizeof(rblxmath::lowp_f64vec2) != 16;
	Error += sizeof(rblxmath::lowp_f64vec3) != 24;
	Error += sizeof(rblxmath::lowp_f64vec4) != 32;

	Error += sizeof(rblxmath::mediump_f32vec2) != 8;
	Error += sizeof(rblxmath::mediump_f32vec3) != 12;
	Error += sizeof(rblxmath::mediump_f32vec4) != 16;
	Error += sizeof(rblxmath::mediump_f64vec2) != 16;
	Error += sizeof(rblxmath::mediump_f64vec3) != 24;
	Error += sizeof(rblxmath::mediump_f64vec4) != 32;

	Error += sizeof(rblxmath::highp_f32vec2) != 8;
	Error += sizeof(rblxmath::highp_f32vec3) != 12;
	Error += sizeof(rblxmath::highp_f32vec4) != 16;
	Error += sizeof(rblxmath::highp_f64vec2) != 16;
	Error += sizeof(rblxmath::highp_f64vec3) != 24;
	Error += sizeof(rblxmath::highp_f64vec4) != 32;

	return Error;
}

static int test_fvec_precision()
{
	int Error = 0;

	{
		rblxmath::f32vec2 v1(1.f);
		rblxmath::lowp_f32vec2 v2(v1);
		rblxmath::mediump_f32vec2 v3(v1);
		rblxmath::highp_f32vec2 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f32vec2(v2), rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f32vec2(v3), rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f32vec2(v4), rblxmath::epsilon<float>())) ? 0 : 1;
	}

	{
		rblxmath::f32vec3 v1(1.f);
		rblxmath::lowp_f32vec3 v2(v1);
		rblxmath::mediump_f32vec3 v3(v1);
		rblxmath::highp_f32vec3 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f32vec3(v2), rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f32vec3(v3), rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f32vec3(v4), rblxmath::epsilon<float>())) ? 0 : 1;
	}
	
	{
		rblxmath::f32vec4 v1(1.f);
		rblxmath::lowp_f32vec4 v2(v1);
		rblxmath::mediump_f32vec4 v3(v1);
		rblxmath::highp_f32vec4 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f32vec4(v2), rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f32vec4(v3), rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f32vec4(v4), rblxmath::epsilon<float>())) ? 0 : 1;
	}
	
	return Error;
}

static int test_dvec_precision()
{
	int Error = 0;

	{
		rblxmath::f64vec2 v1(1.0);
		rblxmath::lowp_f64vec2 v2(v1);
		rblxmath::mediump_f64vec2 v3(v1);
		rblxmath::highp_f64vec2 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f64vec2(v2), rblxmath::epsilon<double>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f64vec2(v3), rblxmath::epsilon<double>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f64vec2(v4), rblxmath::epsilon<double>())) ? 0 : 1;
	}

	{
		rblxmath::f64vec3 v1(1.0);
		rblxmath::lowp_f64vec3 v2(v1);
		rblxmath::mediump_f64vec3 v3(v1);
		rblxmath::highp_f64vec3 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f64vec3(v2), rblxmath::epsilon<double>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f64vec3(v3), rblxmath::epsilon<double>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f64vec3(v4), rblxmath::epsilon<double>())) ? 0 : 1;
	}

	{
		rblxmath::f64vec4 v1(1.0);
		rblxmath::lowp_f64vec4 v2(v1);
		rblxmath::mediump_f64vec4 v3(v1);
		rblxmath::highp_f64vec4 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f64vec4(v2), rblxmath::epsilon<double>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f64vec4(v3), rblxmath::epsilon<double>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::f64vec4(v4), rblxmath::epsilon<double>())) ? 0 : 1;
	}

	return Error;
}

static int test_ivec_size()
{
	int Error = 0;

	Error += sizeof(rblxmath::i8vec2) != 2;
	Error += sizeof(rblxmath::i8vec3) != 3;
	Error += sizeof(rblxmath::i8vec4) != 4;
	Error += sizeof(rblxmath::i16vec2) != 4;
	Error += sizeof(rblxmath::i16vec3) != 6;
	Error += sizeof(rblxmath::i16vec4) != 8;
	Error += sizeof(rblxmath::i32vec2) != 8;
	Error += sizeof(rblxmath::i32vec3) != 12;
	Error += sizeof(rblxmath::i32vec4) != 16;
	Error += sizeof(rblxmath::i64vec2) != 16;
	Error += sizeof(rblxmath::i64vec3) != 24;
	Error += sizeof(rblxmath::i64vec4) != 32;

	Error += sizeof(rblxmath::lowp_i8vec2) != 2;
	Error += sizeof(rblxmath::lowp_i8vec3) != 3;
	Error += sizeof(rblxmath::lowp_i8vec4) != 4;
	Error += sizeof(rblxmath::lowp_i16vec2) != 4;
	Error += sizeof(rblxmath::lowp_i16vec3) != 6;
	Error += sizeof(rblxmath::lowp_i16vec4) != 8;
	Error += sizeof(rblxmath::lowp_i32vec2) != 8;
	Error += sizeof(rblxmath::lowp_i32vec3) != 12;
	Error += sizeof(rblxmath::lowp_i32vec4) != 16;
	Error += sizeof(rblxmath::lowp_i64vec2) != 16;
	Error += sizeof(rblxmath::lowp_i64vec3) != 24;
	Error += sizeof(rblxmath::lowp_i64vec4) != 32;

	Error += sizeof(rblxmath::mediump_i8vec2) != 2;
	Error += sizeof(rblxmath::mediump_i8vec3) != 3;
	Error += sizeof(rblxmath::mediump_i8vec4) != 4;
	Error += sizeof(rblxmath::mediump_i16vec2) != 4;
	Error += sizeof(rblxmath::mediump_i16vec3) != 6;
	Error += sizeof(rblxmath::mediump_i16vec4) != 8;
	Error += sizeof(rblxmath::mediump_i32vec2) != 8;
	Error += sizeof(rblxmath::mediump_i32vec3) != 12;
	Error += sizeof(rblxmath::mediump_i32vec4) != 16;
	Error += sizeof(rblxmath::mediump_i64vec2) != 16;
	Error += sizeof(rblxmath::mediump_i64vec3) != 24;
	Error += sizeof(rblxmath::mediump_i64vec4) != 32;

	Error += sizeof(rblxmath::highp_i8vec2) != 2;
	Error += sizeof(rblxmath::highp_i8vec3) != 3;
	Error += sizeof(rblxmath::highp_i8vec4) != 4;
	Error += sizeof(rblxmath::highp_i16vec2) != 4;
	Error += sizeof(rblxmath::highp_i16vec3) != 6;
	Error += sizeof(rblxmath::highp_i16vec4) != 8;
	Error += sizeof(rblxmath::highp_i32vec2) != 8;
	Error += sizeof(rblxmath::highp_i32vec3) != 12;
	Error += sizeof(rblxmath::highp_i32vec4) != 16;
	Error += sizeof(rblxmath::highp_i64vec2) != 16;
	Error += sizeof(rblxmath::highp_i64vec3) != 24;
	Error += sizeof(rblxmath::highp_i64vec4) != 32;

	return Error;
}

static int test_ivec_precision()
{
	int Error = 0;

	{
		rblxmath::i8vec2 v1(0);
		rblxmath::lowp_i8vec2 v2(v1);
		rblxmath::mediump_i8vec2 v3(v1);
		rblxmath::highp_i8vec2 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i8vec2(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i8vec2(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i8vec2(v4))) ? 0 : 1;
	}

	{
		rblxmath::i8vec3 v1(0);
		rblxmath::lowp_i8vec3 v2(v1);
		rblxmath::mediump_i8vec3 v3(v1);
		rblxmath::highp_i8vec3 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i8vec3(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i8vec3(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i8vec3(v4))) ? 0 : 1;
	}

	{
		rblxmath::i8vec4 v1(0);
		rblxmath::lowp_i8vec4 v2(v1);
		rblxmath::mediump_i8vec4 v3(v1);
		rblxmath::highp_i8vec4 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i8vec4(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i8vec4(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i8vec4(v4))) ? 0 : 1;
	}

	{
		rblxmath::i16vec2 v1(0);
		rblxmath::lowp_i16vec2 v2(v1);
		rblxmath::mediump_i16vec2 v3(v1);
		rblxmath::highp_i16vec2 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i16vec2(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i16vec2(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i16vec2(v4))) ? 0 : 1;
	}

	{
		rblxmath::i16vec3 v1(0);
		rblxmath::lowp_i16vec3 v2(v1);
		rblxmath::mediump_i16vec3 v3(v1);
		rblxmath::highp_i16vec3 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i16vec3(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i16vec3(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i16vec3(v4))) ? 0 : 1;
	}

	{
		rblxmath::i16vec4 v1(0);
		rblxmath::lowp_i16vec4 v2(v1);
		rblxmath::mediump_i16vec4 v3(v1);
		rblxmath::highp_i16vec4 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i16vec4(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i16vec4(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i16vec4(v4))) ? 0 : 1;
	}

	{
		rblxmath::i32vec2 v1(0);
		rblxmath::lowp_i32vec2 v2(v1);
		rblxmath::mediump_i32vec2 v3(v1);
		rblxmath::highp_i32vec2 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i32vec2(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i32vec2(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i32vec2(v4))) ? 0 : 1;
	}

	{
		rblxmath::i32vec3 v1(0);
		rblxmath::lowp_i32vec3 v2(v1);
		rblxmath::mediump_i32vec3 v3(v1);
		rblxmath::highp_i32vec3 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i32vec3(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i32vec3(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i32vec3(v4))) ? 0 : 1;
	}

	{
		rblxmath::i32vec4 v1(0);
		rblxmath::lowp_i32vec4 v2(v1);
		rblxmath::mediump_i32vec4 v3(v1);
		rblxmath::highp_i32vec4 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i32vec4(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i32vec4(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i32vec4(v4))) ? 0 : 1;
	}

	{
		rblxmath::i64vec2 v1(0);
		rblxmath::lowp_i64vec2 v2(v1);
		rblxmath::mediump_i64vec2 v3(v1);
		rblxmath::highp_i64vec2 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i64vec2(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i64vec2(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i64vec2(v4))) ? 0 : 1;
	}

	{
		rblxmath::i64vec3 v1(0);
		rblxmath::lowp_i64vec3 v2(v1);
		rblxmath::mediump_i64vec3 v3(v1);
		rblxmath::highp_i64vec3 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i64vec3(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i64vec3(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i64vec3(v4))) ? 0 : 1;
	}

	{
		rblxmath::i64vec4 v1(0);
		rblxmath::lowp_i64vec4 v2(v1);
		rblxmath::mediump_i64vec4 v3(v1);
		rblxmath::highp_i64vec4 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i64vec4(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i64vec4(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::i64vec4(v4))) ? 0 : 1;
	}

	return Error;
}

static int test_uvec_size()
{
	int Error = 0;

	Error += sizeof(rblxmath::u8vec2) != 2;
	Error += sizeof(rblxmath::u8vec3) != 3;
	Error += sizeof(rblxmath::u8vec4) != 4;
	Error += sizeof(rblxmath::u16vec2) != 4;
	Error += sizeof(rblxmath::u16vec3) != 6;
	Error += sizeof(rblxmath::u16vec4) != 8;
	Error += sizeof(rblxmath::u32vec2) != 8;
	Error += sizeof(rblxmath::u32vec3) != 12;
	Error += sizeof(rblxmath::u32vec4) != 16;
	Error += sizeof(rblxmath::u64vec2) != 16;
	Error += sizeof(rblxmath::u64vec3) != 24;
	Error += sizeof(rblxmath::u64vec4) != 32;

	Error += sizeof(rblxmath::lowp_u8vec2) != 2;
	Error += sizeof(rblxmath::lowp_u8vec3) != 3;
	Error += sizeof(rblxmath::lowp_u8vec4) != 4;
	Error += sizeof(rblxmath::lowp_u16vec2) != 4;
	Error += sizeof(rblxmath::lowp_u16vec3) != 6;
	Error += sizeof(rblxmath::lowp_u16vec4) != 8;
	Error += sizeof(rblxmath::lowp_u32vec2) != 8;
	Error += sizeof(rblxmath::lowp_u32vec3) != 12;
	Error += sizeof(rblxmath::lowp_u32vec4) != 16;
	Error += sizeof(rblxmath::lowp_u64vec2) != 16;
	Error += sizeof(rblxmath::lowp_u64vec3) != 24;
	Error += sizeof(rblxmath::lowp_u64vec4) != 32;

	Error += sizeof(rblxmath::mediump_u8vec2) != 2;
	Error += sizeof(rblxmath::mediump_u8vec3) != 3;
	Error += sizeof(rblxmath::mediump_u8vec4) != 4;
	Error += sizeof(rblxmath::mediump_u16vec2) != 4;
	Error += sizeof(rblxmath::mediump_u16vec3) != 6;
	Error += sizeof(rblxmath::mediump_u16vec4) != 8;
	Error += sizeof(rblxmath::mediump_u32vec2) != 8;
	Error += sizeof(rblxmath::mediump_u32vec3) != 12;
	Error += sizeof(rblxmath::mediump_u32vec4) != 16;
	Error += sizeof(rblxmath::mediump_u64vec2) != 16;
	Error += sizeof(rblxmath::mediump_u64vec3) != 24;
	Error += sizeof(rblxmath::mediump_u64vec4) != 32;

	Error += sizeof(rblxmath::highp_u8vec2) != 2;
	Error += sizeof(rblxmath::highp_u8vec3) != 3;
	Error += sizeof(rblxmath::highp_u8vec4) != 4;
	Error += sizeof(rblxmath::highp_u16vec2) != 4;
	Error += sizeof(rblxmath::highp_u16vec3) != 6;
	Error += sizeof(rblxmath::highp_u16vec4) != 8;
	Error += sizeof(rblxmath::highp_u32vec2) != 8;
	Error += sizeof(rblxmath::highp_u32vec3) != 12;
	Error += sizeof(rblxmath::highp_u32vec4) != 16;
	Error += sizeof(rblxmath::highp_u64vec2) != 16;
	Error += sizeof(rblxmath::highp_u64vec3) != 24;
	Error += sizeof(rblxmath::highp_u64vec4) != 32;

	return Error;
}

static int test_uvec_precision()
{
	int Error = 0;

	{
		rblxmath::u8vec2 v1(0);
		rblxmath::lowp_u8vec2 v2(v1);
		rblxmath::mediump_u8vec2 v3(v1);
		rblxmath::highp_u8vec2 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u8vec2(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u8vec2(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u8vec2(v4))) ? 0 : 1;
	}

	{
		rblxmath::u8vec3 v1(0);
		rblxmath::lowp_u8vec3 v2(v1);
		rblxmath::mediump_u8vec3 v3(v1);
		rblxmath::highp_u8vec3 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u8vec3(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u8vec3(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u8vec3(v4))) ? 0 : 1;
	}
	
	{
		rblxmath::u8vec4 v1(0);
		rblxmath::lowp_u8vec4 v2(v1);
		rblxmath::mediump_u8vec4 v3(v1);
		rblxmath::highp_u8vec4 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u8vec4(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u8vec4(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u8vec4(v4))) ? 0 : 1;
	}

	{
		rblxmath::u16vec2 v1(0);
		rblxmath::lowp_u16vec2 v2(v1);
		rblxmath::mediump_u16vec2 v3(v1);
		rblxmath::highp_u16vec2 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u16vec2(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u16vec2(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u16vec2(v4))) ? 0 : 1;
	}

	{
		rblxmath::u16vec3 v1(0);
		rblxmath::lowp_u16vec3 v2(v1);
		rblxmath::mediump_u16vec3 v3(v1);
		rblxmath::highp_u16vec3 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u16vec3(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u16vec3(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u16vec3(v4))) ? 0 : 1;
	}
	
	{
		rblxmath::u16vec4 v1(0);
		rblxmath::lowp_u16vec4 v2(v1);
		rblxmath::mediump_u16vec4 v3(v1);
		rblxmath::highp_u16vec4 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u16vec4(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u16vec4(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u16vec4(v4))) ? 0 : 1;
	}

	{
		rblxmath::u32vec2 v1(0);
		rblxmath::lowp_u32vec2 v2(v1);
		rblxmath::mediump_u32vec2 v3(v1);
		rblxmath::highp_u32vec2 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u32vec2(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u32vec2(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u32vec2(v4))) ? 0 : 1;
	}

	{
		rblxmath::u32vec3 v1(0);
		rblxmath::lowp_u32vec3 v2(v1);
		rblxmath::mediump_u32vec3 v3(v1);
		rblxmath::highp_u32vec3 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u32vec3(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u32vec3(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u32vec3(v4))) ? 0 : 1;
	}
	
	{
		rblxmath::u32vec4 v1(0);
		rblxmath::lowp_u32vec4 v2(v1);
		rblxmath::mediump_u32vec4 v3(v1);
		rblxmath::highp_u32vec4 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u32vec4(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u32vec4(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u32vec4(v4))) ? 0 : 1;
	}
	
	{
		rblxmath::u64vec2 v1(0);
		rblxmath::lowp_u64vec2 v2(v1);
		rblxmath::mediump_u64vec2 v3(v1);
		rblxmath::highp_u64vec2 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u64vec2(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u64vec2(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u64vec2(v4))) ? 0 : 1;
	}

	{
		rblxmath::u64vec3 v1(0);
		rblxmath::lowp_u64vec3 v2(v1);
		rblxmath::mediump_u64vec3 v3(v1);
		rblxmath::highp_u64vec3 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u64vec3(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u64vec3(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u64vec3(v4))) ? 0 : 1;
	}
	
	{
		rblxmath::u64vec4 v1(0);
		rblxmath::lowp_u64vec4 v2(v1);
		rblxmath::mediump_u64vec4 v3(v1);
		rblxmath::highp_u64vec4 v4(v1);

		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u64vec4(v2))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u64vec4(v3))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(v1, rblxmath::u64vec4(v4))) ? 0 : 1;
	}
	
	return Error;
}

static int test_fmat_size()
{
	int Error = 0;

	Error += sizeof(rblxmath::mat2) != 16;
	Error += sizeof(rblxmath::mat3) != 36;
	Error += sizeof(rblxmath::mat4) != 64;
	Error += sizeof(rblxmath::mat2x2) != 16;
	Error += sizeof(rblxmath::mat2x3) != 24;
	Error += sizeof(rblxmath::mat2x4) != 32;
	Error += sizeof(rblxmath::mat3x2) != 24;
	Error += sizeof(rblxmath::mat3x3) != 36;
	Error += sizeof(rblxmath::mat3x4) != 48;
	Error += sizeof(rblxmath::mat4x2) != 32;
	Error += sizeof(rblxmath::mat4x3) != 48;
	Error += sizeof(rblxmath::mat4x4) != 64;

	Error += sizeof(rblxmath::fmat2) != 16;
	Error += sizeof(rblxmath::fmat3) != 36;
	Error += sizeof(rblxmath::fmat4) != 64;
	Error += sizeof(rblxmath::fmat2x2) != 16;
	Error += sizeof(rblxmath::fmat2x3) != 24;
	Error += sizeof(rblxmath::fmat2x4) != 32;
	Error += sizeof(rblxmath::fmat3x2) != 24;
	Error += sizeof(rblxmath::fmat3x3) != 36;
	Error += sizeof(rblxmath::fmat3x4) != 48;
	Error += sizeof(rblxmath::fmat4x2) != 32;
	Error += sizeof(rblxmath::fmat4x3) != 48;
	Error += sizeof(rblxmath::fmat4x4) != 64;

	Error += sizeof(rblxmath::f32mat2) != 16;
	Error += sizeof(rblxmath::f32mat3) != 36;
	Error += sizeof(rblxmath::f32mat4) != 64;
	Error += sizeof(rblxmath::f32mat2x2) != 16;
	Error += sizeof(rblxmath::f32mat2x3) != 24;
	Error += sizeof(rblxmath::f32mat2x4) != 32;
	Error += sizeof(rblxmath::f32mat3x2) != 24;
	Error += sizeof(rblxmath::f32mat3x3) != 36;
	Error += sizeof(rblxmath::f32mat3x4) != 48;
	Error += sizeof(rblxmath::f32mat4x2) != 32;
	Error += sizeof(rblxmath::f32mat4x3) != 48;
	Error += sizeof(rblxmath::f32mat4x4) != 64;


	Error += sizeof(rblxmath::lowp_mat2) != 16;
	Error += sizeof(rblxmath::lowp_mat3) != 36;
	Error += sizeof(rblxmath::lowp_mat4) != 64;
	Error += sizeof(rblxmath::lowp_mat2x2) != 16;
	Error += sizeof(rblxmath::lowp_mat2x3) != 24;
	Error += sizeof(rblxmath::lowp_mat2x4) != 32;
	Error += sizeof(rblxmath::lowp_mat3x2) != 24;
	Error += sizeof(rblxmath::lowp_mat3x3) != 36;
	Error += sizeof(rblxmath::lowp_mat3x4) != 48;
	Error += sizeof(rblxmath::lowp_mat4x2) != 32;
	Error += sizeof(rblxmath::lowp_mat4x3) != 48;
	Error += sizeof(rblxmath::lowp_mat4x4) != 64;

	Error += sizeof(rblxmath::lowp_fmat2) != 16;
	Error += sizeof(rblxmath::lowp_fmat3) != 36;
	Error += sizeof(rblxmath::lowp_fmat4) != 64;
	Error += sizeof(rblxmath::lowp_fmat2x2) != 16;
	Error += sizeof(rblxmath::lowp_fmat2x3) != 24;
	Error += sizeof(rblxmath::lowp_fmat2x4) != 32;
	Error += sizeof(rblxmath::lowp_fmat3x2) != 24;
	Error += sizeof(rblxmath::lowp_fmat3x3) != 36;
	Error += sizeof(rblxmath::lowp_fmat3x4) != 48;
	Error += sizeof(rblxmath::lowp_fmat4x2) != 32;
	Error += sizeof(rblxmath::lowp_fmat4x3) != 48;
	Error += sizeof(rblxmath::lowp_fmat4x4) != 64;

	Error += sizeof(rblxmath::lowp_f32mat2) != 16;
	Error += sizeof(rblxmath::lowp_f32mat3) != 36;
	Error += sizeof(rblxmath::lowp_f32mat4) != 64;
	Error += sizeof(rblxmath::lowp_f32mat2x2) != 16;
	Error += sizeof(rblxmath::lowp_f32mat2x3) != 24;
	Error += sizeof(rblxmath::lowp_f32mat2x4) != 32;
	Error += sizeof(rblxmath::lowp_f32mat3x2) != 24;
	Error += sizeof(rblxmath::lowp_f32mat3x3) != 36;
	Error += sizeof(rblxmath::lowp_f32mat3x4) != 48;
	Error += sizeof(rblxmath::lowp_f32mat4x2) != 32;
	Error += sizeof(rblxmath::lowp_f32mat4x3) != 48;
	Error += sizeof(rblxmath::lowp_f32mat4x4) != 64;

	Error += sizeof(rblxmath::mediump_mat2) != 16;
	Error += sizeof(rblxmath::mediump_mat3) != 36;
	Error += sizeof(rblxmath::mediump_mat4) != 64;
	Error += sizeof(rblxmath::mediump_mat2x2) != 16;
	Error += sizeof(rblxmath::mediump_mat2x3) != 24;
	Error += sizeof(rblxmath::mediump_mat2x4) != 32;
	Error += sizeof(rblxmath::mediump_mat3x2) != 24;
	Error += sizeof(rblxmath::mediump_mat3x3) != 36;
	Error += sizeof(rblxmath::mediump_mat3x4) != 48;
	Error += sizeof(rblxmath::mediump_mat4x2) != 32;
	Error += sizeof(rblxmath::mediump_mat4x3) != 48;
	Error += sizeof(rblxmath::mediump_mat4x4) != 64;

	Error += sizeof(rblxmath::mediump_fmat2) != 16;
	Error += sizeof(rblxmath::mediump_fmat3) != 36;
	Error += sizeof(rblxmath::mediump_fmat4) != 64;
	Error += sizeof(rblxmath::mediump_fmat2x2) != 16;
	Error += sizeof(rblxmath::mediump_fmat2x3) != 24;
	Error += sizeof(rblxmath::mediump_fmat2x4) != 32;
	Error += sizeof(rblxmath::mediump_fmat3x2) != 24;
	Error += sizeof(rblxmath::mediump_fmat3x3) != 36;
	Error += sizeof(rblxmath::mediump_fmat3x4) != 48;
	Error += sizeof(rblxmath::mediump_fmat4x2) != 32;
	Error += sizeof(rblxmath::mediump_fmat4x3) != 48;
	Error += sizeof(rblxmath::mediump_fmat4x4) != 64;

	Error += sizeof(rblxmath::mediump_f32mat2) != 16;
	Error += sizeof(rblxmath::mediump_f32mat3) != 36;
	Error += sizeof(rblxmath::mediump_f32mat4) != 64;
	Error += sizeof(rblxmath::mediump_f32mat2x2) != 16;
	Error += sizeof(rblxmath::mediump_f32mat2x3) != 24;
	Error += sizeof(rblxmath::mediump_f32mat2x4) != 32;
	Error += sizeof(rblxmath::mediump_f32mat3x2) != 24;
	Error += sizeof(rblxmath::mediump_f32mat3x3) != 36;
	Error += sizeof(rblxmath::mediump_f32mat3x4) != 48;
	Error += sizeof(rblxmath::mediump_f32mat4x2) != 32;
	Error += sizeof(rblxmath::mediump_f32mat4x3) != 48;
	Error += sizeof(rblxmath::mediump_f32mat4x4) != 64;

	Error += sizeof(rblxmath::highp_mat2) != 16;
	Error += sizeof(rblxmath::highp_mat3) != 36;
	Error += sizeof(rblxmath::highp_mat4) != 64;
	Error += sizeof(rblxmath::highp_mat2x2) != 16;
	Error += sizeof(rblxmath::highp_mat2x3) != 24;
	Error += sizeof(rblxmath::highp_mat2x4) != 32;
	Error += sizeof(rblxmath::highp_mat3x2) != 24;
	Error += sizeof(rblxmath::highp_mat3x3) != 36;
	Error += sizeof(rblxmath::highp_mat3x4) != 48;
	Error += sizeof(rblxmath::highp_mat4x2) != 32;
	Error += sizeof(rblxmath::highp_mat4x3) != 48;
	Error += sizeof(rblxmath::highp_mat4x4) != 64;

	Error += sizeof(rblxmath::highp_fmat2) != 16;
	Error += sizeof(rblxmath::highp_fmat3) != 36;
	Error += sizeof(rblxmath::highp_fmat4) != 64;
	Error += sizeof(rblxmath::highp_fmat2x2) != 16;
	Error += sizeof(rblxmath::highp_fmat2x3) != 24;
	Error += sizeof(rblxmath::highp_fmat2x4) != 32;
	Error += sizeof(rblxmath::highp_fmat3x2) != 24;
	Error += sizeof(rblxmath::highp_fmat3x3) != 36;
	Error += sizeof(rblxmath::highp_fmat3x4) != 48;
	Error += sizeof(rblxmath::highp_fmat4x2) != 32;
	Error += sizeof(rblxmath::highp_fmat4x3) != 48;
	Error += sizeof(rblxmath::highp_fmat4x4) != 64;

	Error += sizeof(rblxmath::highp_f32mat2) != 16;
	Error += sizeof(rblxmath::highp_f32mat3) != 36;
	Error += sizeof(rblxmath::highp_f32mat4) != 64;
	Error += sizeof(rblxmath::highp_f32mat2x2) != 16;
	Error += sizeof(rblxmath::highp_f32mat2x3) != 24;
	Error += sizeof(rblxmath::highp_f32mat2x4) != 32;
	Error += sizeof(rblxmath::highp_f32mat3x2) != 24;
	Error += sizeof(rblxmath::highp_f32mat3x3) != 36;
	Error += sizeof(rblxmath::highp_f32mat3x4) != 48;
	Error += sizeof(rblxmath::highp_f32mat4x2) != 32;
	Error += sizeof(rblxmath::highp_f32mat4x3) != 48;
	Error += sizeof(rblxmath::highp_f32mat4x4) != 64;

	return Error;
}

static int test_dmat_size()
{
	int Error = 0;

	Error += sizeof(rblxmath::f64mat2) != 32;
	Error += sizeof(rblxmath::f64mat3) != 72;
	Error += sizeof(rblxmath::f64mat4) != 128;
	Error += sizeof(rblxmath::f64mat2x2) != 32;
	Error += sizeof(rblxmath::f64mat2x3) != 48;
	Error += sizeof(rblxmath::f64mat2x4) != 64;
	Error += sizeof(rblxmath::f64mat3x2) != 48;
	Error += sizeof(rblxmath::f64mat3x3) != 72;
	Error += sizeof(rblxmath::f64mat3x4) != 96;
	Error += sizeof(rblxmath::f64mat4x2) != 64;
	Error += sizeof(rblxmath::f64mat4x3) != 96;
	Error += sizeof(rblxmath::f64mat4x4) != 128;

	Error += sizeof(rblxmath::lowp_f64mat2) != 32;
	Error += sizeof(rblxmath::lowp_f64mat3) != 72;
	Error += sizeof(rblxmath::lowp_f64mat4) != 128;
	Error += sizeof(rblxmath::lowp_f64mat2x2) != 32;
	Error += sizeof(rblxmath::lowp_f64mat2x3) != 48;
	Error += sizeof(rblxmath::lowp_f64mat2x4) != 64;
	Error += sizeof(rblxmath::lowp_f64mat3x2) != 48;
	Error += sizeof(rblxmath::lowp_f64mat3x3) != 72;
	Error += sizeof(rblxmath::lowp_f64mat3x4) != 96;
	Error += sizeof(rblxmath::lowp_f64mat4x2) != 64;
	Error += sizeof(rblxmath::lowp_f64mat4x3) != 96;
	Error += sizeof(rblxmath::lowp_f64mat4x4) != 128;

	Error += sizeof(rblxmath::mediump_f64mat2) != 32;
	Error += sizeof(rblxmath::mediump_f64mat3) != 72;
	Error += sizeof(rblxmath::mediump_f64mat4) != 128;
	Error += sizeof(rblxmath::mediump_f64mat2x2) != 32;
	Error += sizeof(rblxmath::mediump_f64mat2x3) != 48;
	Error += sizeof(rblxmath::mediump_f64mat2x4) != 64;
	Error += sizeof(rblxmath::mediump_f64mat3x2) != 48;
	Error += sizeof(rblxmath::mediump_f64mat3x3) != 72;
	Error += sizeof(rblxmath::mediump_f64mat3x4) != 96;
	Error += sizeof(rblxmath::mediump_f64mat4x2) != 64;
	Error += sizeof(rblxmath::mediump_f64mat4x3) != 96;
	Error += sizeof(rblxmath::mediump_f64mat4x4) != 128;

	Error += sizeof(rblxmath::highp_f64mat2) != 32;
	Error += sizeof(rblxmath::highp_f64mat3) != 72;
	Error += sizeof(rblxmath::highp_f64mat4) != 128;
	Error += sizeof(rblxmath::highp_f64mat2x2) != 32;
	Error += sizeof(rblxmath::highp_f64mat2x3) != 48;
	Error += sizeof(rblxmath::highp_f64mat2x4) != 64;
	Error += sizeof(rblxmath::highp_f64mat3x2) != 48;
	Error += sizeof(rblxmath::highp_f64mat3x3) != 72;
	Error += sizeof(rblxmath::highp_f64mat3x4) != 96;
	Error += sizeof(rblxmath::highp_f64mat4x2) != 64;
	Error += sizeof(rblxmath::highp_f64mat4x3) != 96;
	Error += sizeof(rblxmath::highp_f64mat4x4) != 128;

	return Error;
}

static int test_quat_size()
{
	int Error = 0;

	Error += sizeof(rblxmath::f32quat) != 16;
	Error += sizeof(rblxmath::f64quat) != 32;

	Error += sizeof(rblxmath::lowp_f32quat) != 16;
	Error += sizeof(rblxmath::lowp_f64quat) != 32;

	Error += sizeof(rblxmath::mediump_f32quat) != 16;
	Error += sizeof(rblxmath::mediump_f64quat) != 32;

	Error += sizeof(rblxmath::highp_f32quat) != 16;
	Error += sizeof(rblxmath::highp_f64quat) != 32;

	return Error;
}

static int test_quat_precision()
{
	int Error = 0;
	
	{
		rblxmath::f32quat q1(0.f, rblxmath::vec3(0.f, 0.f, 1.f));
		rblxmath::lowp_f32quat qA(q1);
		rblxmath::mediump_f32quat qB(q1);
		rblxmath::highp_f32quat qC(q1);
		rblxmath::f32quat q2(qA);
		rblxmath::f32quat q3(qB);
		rblxmath::f32quat q4(qC);
		
		Error += rblxmath::all(rblxmath::equal(q1, q2, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(q1, q3, rblxmath::epsilon<float>())) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(q1, q4, rblxmath::epsilon<float>())) ? 0 : 1;
	}

	return Error;
}

static int test_fvec_conversion()
{
	int Error(0);

	{
		rblxmath::highp_vec4 a = rblxmath::vec4(1, 2, 3, 4);
		Error += rblxmath::all(rblxmath::equal(a, rblxmath::vec4(1, 2, 3, 4), rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::mediump_vec4 b = rblxmath::vec4(1, 2, 3, 4);
		rblxmath::lowp_vec4 c = b;
		rblxmath::mediump_vec4 d = c;
		Error += rblxmath::all(rblxmath::equal(b, d, rblxmath::epsilon<float>())) ? 0 : 1;

		rblxmath::lowp_ivec4 e = rblxmath::ivec4(d);
		rblxmath::lowp_ivec3 f = rblxmath::ivec3(e);
		Error += rblxmath::all(rblxmath::equal(f, rblxmath::lowp_ivec3(1, 2, 3))) ? 0 : 1;
	}

	return Error;
}

#if RBLXMATH_HAS_OPENMP
static int test_openmp()
{
	std::vector<rblxmath::u8vec3> VectorA(1000);
	std::vector<rblxmath::u8vec3> VectorB(1000);
	std::vector<rblxmath::u8vec3> VectorC(1000);

	for (std::size_t i = 0; i < VectorA.size(); ++i)
	{
		VectorA[i] = rblxmath::u8vec3(1, 1, 1);
		VectorB[i] = rblxmath::u8vec3(1, 1, 1);
	}

	#pragma omp parallel for default(none) shared(VectorA, VectorB, VectorC)
	for (int i = 0; i < static_cast<int>(VectorC.size()); ++i)
	{
		VectorC[i] = VectorA[i] + VectorB[i];
	}

	return 0;
}
#endif//RBLXMATH_HAS_OPENMP

int main()
{
	int Error = 0;

	Error += test_scalar_size();
	Error += test_fvec_size();

	Error += test_fvec_precision();
	Error += test_fvec_conversion();

	Error += test_dvec_precision();

	Error += test_uvec_size();
	Error += test_uvec_precision();
	Error += test_ivec_size();
	Error += test_ivec_precision();

	Error += test_fmat_size();
	Error += test_dmat_size();
	Error += test_quat_size();
	Error += test_quat_precision();

#	if RBLXMATH_HAS_OPENMP
		Error += test_openmp();
#	endif//

	return Error;
}
