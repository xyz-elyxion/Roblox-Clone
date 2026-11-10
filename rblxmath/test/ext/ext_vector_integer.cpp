#define RBLXMATH_FORCE_UNRESTRICTED_GENTYPE

#include <rblxmath/ext/vector_integer.hpp>
#include <rblxmath/ext/scalar_int_sized.hpp>
#include <rblxmath/ext/scalar_uint_sized.hpp>
#include <rblxmath/ext/vector_int1.hpp>
#include <rblxmath/ext/vector_int2.hpp>
#include <rblxmath/ext/vector_int3.hpp>
#include <rblxmath/ext/vector_int4.hpp>
#include <rblxmath/ext/vector_bool1.hpp>
#include <rblxmath/ext/vector_bool2.hpp>
#include <rblxmath/ext/vector_bool3.hpp>
#include <rblxmath/ext/vector_bool4.hpp>
#include <vector>
#include <ctime>
#include <cstdio>

namespace isPowerOfTwo
{
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wpadded"
#endif

	template<typename genType>
	struct type
	{
		genType		Value;
		bool		Return;
	};

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

	template <rblxmath::length_t L>
	static int test_int16()
	{
		type<rblxmath::int16> const Data[] =
		{
			{ 0x0001, true },
			{ 0x0002, true },
			{ 0x0004, true },
			{ 0x0080, true },
			{ 0x0000, true },
			{ 0x0003, false }
		};

		int Error = 0;

		for (std::size_t i = 0, n = sizeof(Data) / sizeof(type<rblxmath::int16>); i < n; ++i)
		{
			rblxmath::vec<L, bool> const Result = rblxmath::isPowerOfTwo(rblxmath::vec<L, rblxmath::int16>(Data[i].Value));
			Error += rblxmath::vec<L, bool>(Data[i].Return) == Result ? 0 : 1;
		}

		return Error;
	}

	template <rblxmath::length_t L>
	static int test_uint16()
	{
		type<rblxmath::uint16> const Data[] =
		{
			{ 0x0001, true },
			{ 0x0002, true },
			{ 0x0004, true },
			{ 0x0000, true },
			{ 0x0000, true },
			{ 0x0003, false }
		};

		int Error = 0;

		for (std::size_t i = 0, n = sizeof(Data) / sizeof(type<rblxmath::uint16>); i < n; ++i)
		{
			rblxmath::vec<L, bool> const Result = rblxmath::isPowerOfTwo(rblxmath::vec<L, rblxmath::uint16>(Data[i].Value));
			Error += rblxmath::vec<L, bool>(Data[i].Return) == Result ? 0 : 1;
		}

		return Error;
	}

	template <rblxmath::length_t L>
	static int test_int32()
	{
		type<int> const Data[] =
		{
			{ 0x00000001, true },
			{ 0x00000002, true },
			{ 0x00000004, true },
			{ 0x0000000f, false },
			{ 0x00000000, true },
			{ 0x00000003, false }
		};

		int Error = 0;

		for (std::size_t i = 0, n = sizeof(Data) / sizeof(type<int>); i < n; ++i)
		{
			rblxmath::vec<L, bool> const Result = rblxmath::isPowerOfTwo(rblxmath::vec<L, rblxmath::int32>(Data[i].Value));
			Error += rblxmath::vec<L, bool>(Data[i].Return) == Result ? 0 : 1;
		}

		return Error;
	}

	template <rblxmath::length_t L>
	static int test_uint32()
	{
		type<rblxmath::uint> const Data[] =
		{
			{ 0x00000001, true },
			{ 0x00000002, true },
			{ 0x00000004, true },
			{ 0x80000000, true },
			{ 0x00000000, true },
			{ 0x00000003, false }
		};

		int Error = 0;

		for (std::size_t i = 0, n = sizeof(Data) / sizeof(type<rblxmath::uint>); i < n; ++i)
		{
			rblxmath::vec<L, bool> const Result = rblxmath::isPowerOfTwo(rblxmath::vec<L, rblxmath::uint32>(Data[i].Value));
			Error += rblxmath::vec<L, bool>(Data[i].Return) == Result ? 0 : 1;
		}

		return Error;
	}

	static int test()
	{
		int Error = 0;

		Error += test_int16<1>();
		Error += test_int16<2>();
		Error += test_int16<3>();
		Error += test_int16<4>();

		Error += test_uint16<1>();
		Error += test_uint16<2>();
		Error += test_uint16<3>();
		Error += test_uint16<4>();

		Error += test_int32<1>();
		Error += test_int32<2>();
		Error += test_int32<3>();
		Error += test_int32<4>();

		Error += test_uint32<1>();
		Error += test_uint32<2>();
		Error += test_uint32<3>();
		Error += test_uint32<4>();

		return Error;
	}
}//isPowerOfTwo

namespace prevPowerOfTwo
{
	template <rblxmath::length_t L, typename T>
	static int run()
	{
		int Error = 0;

		rblxmath::vec<L, T> const A = rblxmath::prevPowerOfTwo(rblxmath::vec<L, T>(7));
		Error += A == rblxmath::vec<L, T>(4) ? 0 : 1;

		rblxmath::vec<L, T> const B = rblxmath::prevPowerOfTwo(rblxmath::vec<L, T>(15));
		Error += B == rblxmath::vec<L, T>(8) ? 0 : 1;

		rblxmath::vec<L, T> const C = rblxmath::prevPowerOfTwo(rblxmath::vec<L, T>(31));
		Error += C == rblxmath::vec<L, T>(16) ? 0 : 1;

		rblxmath::vec<L, T> const D = rblxmath::prevPowerOfTwo(rblxmath::vec<L, T>(32));
		Error += D == rblxmath::vec<L, T>(32) ? 0 : 1;

		return Error;
	}

	static int test()
	{
		int Error = 0;

		Error += run<1, rblxmath::int8>();
		Error += run<2, rblxmath::int8>();
		Error += run<3, rblxmath::int8>();
		Error += run<4, rblxmath::int8>();

		Error += run<1, rblxmath::int16>();
		Error += run<2, rblxmath::int16>();
		Error += run<3, rblxmath::int16>();
		Error += run<4, rblxmath::int16>();

		Error += run<1, rblxmath::int32>();
		Error += run<2, rblxmath::int32>();
		Error += run<3, rblxmath::int32>();
		Error += run<4, rblxmath::int32>();

		Error += run<1, rblxmath::int64>();
		Error += run<2, rblxmath::int64>();
		Error += run<3, rblxmath::int64>();
		Error += run<4, rblxmath::int64>();

		Error += run<1, rblxmath::uint8>();
		Error += run<2, rblxmath::uint8>();
		Error += run<3, rblxmath::uint8>();
		Error += run<4, rblxmath::uint8>();

		Error += run<1, rblxmath::uint16>();
		Error += run<2, rblxmath::uint16>();
		Error += run<3, rblxmath::uint16>();
		Error += run<4, rblxmath::uint16>();

		Error += run<1, rblxmath::uint32>();
		Error += run<2, rblxmath::uint32>();
		Error += run<3, rblxmath::uint32>();
		Error += run<4, rblxmath::uint32>();

		Error += run<1, rblxmath::uint64>();
		Error += run<2, rblxmath::uint64>();
		Error += run<3, rblxmath::uint64>();
		Error += run<4, rblxmath::uint64>();

		return Error;
	}
}//namespace prevPowerOfTwo

namespace nextPowerOfTwo
{
	template <rblxmath::length_t L, typename T>
	static int run()
	{
		int Error = 0;

		rblxmath::vec<L, T> const A = rblxmath::nextPowerOfTwo(rblxmath::vec<L, T>(7));
		Error += A == rblxmath::vec<L, T>(8) ? 0 : 1;

		rblxmath::vec<L, T> const B = rblxmath::nextPowerOfTwo(rblxmath::vec<L, T>(15));
		Error += B == rblxmath::vec<L, T>(16) ? 0 : 1;

		rblxmath::vec<L, T> const C = rblxmath::nextPowerOfTwo(rblxmath::vec<L, T>(31));
		Error += C == rblxmath::vec<L, T>(32) ? 0 : 1;

		rblxmath::vec<L, T> const D = rblxmath::nextPowerOfTwo(rblxmath::vec<L, T>(32));
		Error += D == rblxmath::vec<L, T>(32) ? 0 : 1;

		return Error;
	}

	static int test()
	{
		int Error = 0;

		Error += run<1, rblxmath::int8>();
		Error += run<2, rblxmath::int8>();
		Error += run<3, rblxmath::int8>();
		Error += run<4, rblxmath::int8>();

		Error += run<1, rblxmath::int16>();
		Error += run<2, rblxmath::int16>();
		Error += run<3, rblxmath::int16>();
		Error += run<4, rblxmath::int16>();

		Error += run<1, rblxmath::int32>();
		Error += run<2, rblxmath::int32>();
		Error += run<3, rblxmath::int32>();
		Error += run<4, rblxmath::int32>();

		Error += run<1, rblxmath::int64>();
		Error += run<2, rblxmath::int64>();
		Error += run<3, rblxmath::int64>();
		Error += run<4, rblxmath::int64>();

		Error += run<1, rblxmath::uint8>();
		Error += run<2, rblxmath::uint8>();
		Error += run<3, rblxmath::uint8>();
		Error += run<4, rblxmath::uint8>();

		Error += run<1, rblxmath::uint16>();
		Error += run<2, rblxmath::uint16>();
		Error += run<3, rblxmath::uint16>();
		Error += run<4, rblxmath::uint16>();

		Error += run<1, rblxmath::uint32>();
		Error += run<2, rblxmath::uint32>();
		Error += run<3, rblxmath::uint32>();
		Error += run<4, rblxmath::uint32>();

		Error += run<1, rblxmath::uint64>();
		Error += run<2, rblxmath::uint64>();
		Error += run<3, rblxmath::uint64>();
		Error += run<4, rblxmath::uint64>();

		return Error;
	}
}//namespace nextPowerOfTwo

namespace prevMultiple
{
	template<typename genIUType>
	struct type
	{
		genIUType Source;
		genIUType Multiple;
		genIUType Return;
	};

	template <rblxmath::length_t L, typename T>
	static int run()
	{
		type<T> const Data[] =
		{
			{ 8, 3, 6 },
			{ 7, 7, 7 }
		};

		int Error = 0;

		for (std::size_t i = 0, n = sizeof(Data) / sizeof(type<T>); i < n; ++i)
		{
			rblxmath::vec<L, T> const Result0 = rblxmath::prevMultiple(rblxmath::vec<L, T>(Data[i].Source), Data[i].Multiple);
			Error += rblxmath::vec<L, T>(Data[i].Return) == Result0 ? 0 : 1;

			rblxmath::vec<L, T> const Result1 = rblxmath::prevMultiple(rblxmath::vec<L, T>(Data[i].Source), rblxmath::vec<L, T>(Data[i].Multiple));
			Error += rblxmath::vec<L, T>(Data[i].Return) == Result1 ? 0 : 1;
		}

		return Error;
	}

	static int test()
	{
		int Error = 0;

		Error += run<1, rblxmath::int8>();
		Error += run<2, rblxmath::int8>();
		Error += run<3, rblxmath::int8>();
		Error += run<4, rblxmath::int8>();

		Error += run<1, rblxmath::int16>();
		Error += run<2, rblxmath::int16>();
		Error += run<3, rblxmath::int16>();
		Error += run<4, rblxmath::int16>();

		Error += run<1, rblxmath::int32>();
		Error += run<2, rblxmath::int32>();
		Error += run<3, rblxmath::int32>();
		Error += run<4, rblxmath::int32>();

		Error += run<1, rblxmath::int64>();
		Error += run<2, rblxmath::int64>();
		Error += run<3, rblxmath::int64>();
		Error += run<4, rblxmath::int64>();

		Error += run<1, rblxmath::uint8>();
		Error += run<2, rblxmath::uint8>();
		Error += run<3, rblxmath::uint8>();
		Error += run<4, rblxmath::uint8>();

		Error += run<1, rblxmath::uint16>();
		Error += run<2, rblxmath::uint16>();
		Error += run<3, rblxmath::uint16>();
		Error += run<4, rblxmath::uint16>();

		Error += run<1, rblxmath::uint32>();
		Error += run<2, rblxmath::uint32>();
		Error += run<3, rblxmath::uint32>();
		Error += run<4, rblxmath::uint32>();

		Error += run<1, rblxmath::uint64>();
		Error += run<2, rblxmath::uint64>();
		Error += run<3, rblxmath::uint64>();
		Error += run<4, rblxmath::uint64>();

		return Error;
	}
}//namespace prevMultiple

namespace nextMultiple
{
	template<typename genIUType>
	struct type
	{
		genIUType Source;
		genIUType Multiple;
		genIUType Return;
	};

	template <rblxmath::length_t L, typename T>
	static int run()
	{
		type<T> const Data[] =
		{
			{ 3, 4, 4 },
			{ 6, 3, 6 },
			{ 5, 3, 6 },
			{ 7, 7, 7 },
			{ 0, 1, 0 },
			{ 8, 3, 9 }
		};

		int Error = 0;

		for (std::size_t i = 0, n = sizeof(Data) / sizeof(type<T>); i < n; ++i)
		{
			rblxmath::vec<L, T> const Result0 = rblxmath::nextMultiple(rblxmath::vec<L, T>(Data[i].Source), rblxmath::vec<L, T>(Data[i].Multiple));
			Error += rblxmath::vec<L, T>(Data[i].Return) == Result0 ? 0 : 1;

			rblxmath::vec<L, T> const Result1 = rblxmath::nextMultiple(rblxmath::vec<L, T>(Data[i].Source), Data[i].Multiple);
			Error += rblxmath::vec<L, T>(Data[i].Return) == Result1 ? 0 : 1;
		}

		return Error;
	}

	static int test()
	{
		int Error = 0;

		Error += run<1, rblxmath::int8>();
		Error += run<2, rblxmath::int8>();
		Error += run<3, rblxmath::int8>();
		Error += run<4, rblxmath::int8>();

		Error += run<1, rblxmath::int16>();
		Error += run<2, rblxmath::int16>();
		Error += run<3, rblxmath::int16>();
		Error += run<4, rblxmath::int16>();

		Error += run<1, rblxmath::int32>();
		Error += run<2, rblxmath::int32>();
		Error += run<3, rblxmath::int32>();
		Error += run<4, rblxmath::int32>();

		Error += run<1, rblxmath::int64>();
		Error += run<2, rblxmath::int64>();
		Error += run<3, rblxmath::int64>();
		Error += run<4, rblxmath::int64>();

		Error += run<1, rblxmath::uint8>();
		Error += run<2, rblxmath::uint8>();
		Error += run<3, rblxmath::uint8>();
		Error += run<4, rblxmath::uint8>();

		Error += run<1, rblxmath::uint16>();
		Error += run<2, rblxmath::uint16>();
		Error += run<3, rblxmath::uint16>();
		Error += run<4, rblxmath::uint16>();

		Error += run<1, rblxmath::uint32>();
		Error += run<2, rblxmath::uint32>();
		Error += run<3, rblxmath::uint32>();
		Error += run<4, rblxmath::uint32>();

		Error += run<1, rblxmath::uint64>();
		Error += run<2, rblxmath::uint64>();
		Error += run<3, rblxmath::uint64>();
		Error += run<4, rblxmath::uint64>();

		return Error;
	}
}//namespace nextMultiple

namespace findNSB
{
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wpadded"
#endif

	template<typename T>
	struct type
	{
		T Source;
		int SignificantBitCount;
		int Return;
	};

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

	template <rblxmath::length_t L, typename T>
	static int run()
	{
		type<T> const Data[] =
		{
			{ 0x00, 1,-1 },
			{ 0x01, 2,-1 },
			{ 0x02, 2,-1 },
			{ 0x06, 3,-1 },
			{ 0x01, 1, 0 },
			{ 0x03, 1, 0 },
			{ 0x03, 2, 1 },
			{ 0x07, 2, 1 },
			{ 0x05, 2, 2 },
			{ 0x0D, 2, 2 }
		};

		int Error = 0;

		for (std::size_t i = 0, n = sizeof(Data) / sizeof(type<T>); i < n; ++i)
		{
			rblxmath::vec<L, int> const Result0 = rblxmath::findNSB<L, T, rblxmath::defaultp>(rblxmath::vec<L, T>(Data[i].Source), rblxmath::vec<L, int>(Data[i].SignificantBitCount));
			Error += rblxmath::vec<L, int>(Data[i].Return) == Result0 ? 0 : 1;
			assert(!Error);
		}

		return Error;
	}

	static int test()
	{
		int Error = 0;

		Error += run<1, rblxmath::uint8>();
		Error += run<2, rblxmath::uint8>();
		Error += run<3, rblxmath::uint8>();
		Error += run<4, rblxmath::uint8>();

		Error += run<1, rblxmath::uint16>();
		Error += run<2, rblxmath::uint16>();
		Error += run<3, rblxmath::uint16>();
		Error += run<4, rblxmath::uint16>();

		Error += run<1, rblxmath::uint32>();
		Error += run<2, rblxmath::uint32>();
		Error += run<3, rblxmath::uint32>();
		Error += run<4, rblxmath::uint32>();

		Error += run<1, rblxmath::uint64>();
		Error += run<2, rblxmath::uint64>();
		Error += run<3, rblxmath::uint64>();
		Error += run<4, rblxmath::uint64>();

		Error += run<1, rblxmath::int8>();
		Error += run<2, rblxmath::int8>();
		Error += run<3, rblxmath::int8>();
		Error += run<4, rblxmath::int8>();

		Error += run<1, rblxmath::int16>();
		Error += run<2, rblxmath::int16>();
		Error += run<3, rblxmath::int16>();
		Error += run<4, rblxmath::int16>();

		Error += run<1, rblxmath::int32>();
		Error += run<2, rblxmath::int32>();
		Error += run<3, rblxmath::int32>();
		Error += run<4, rblxmath::int32>();

		Error += run<1, rblxmath::int64>();
		Error += run<2, rblxmath::int64>();
		Error += run<3, rblxmath::int64>();
		Error += run<4, rblxmath::int64>();


		return Error;
	}
}//namespace findNSB

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wpadded"
#endif

template<typename T, typename B>
struct test_mix_entry
{
	T x;
	T y;
	B a;
	T Result;
};

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

static int test_mix()
{
	test_mix_entry<int, bool> const TestBool[] =
	{
		{0, 1, false, 0},
		{0, 1, true, 1},
		{-1, 1, false, -1},
		{-1, 1, true, 1}
	};

	test_mix_entry<int, int> const TestInt[] =
	{
		{0, 1, 0, 0},
		{0, 1, 1, 1},
		{-1, 1, 0, -1},
		{-1, 1, 1, 1}
	};

	test_mix_entry<rblxmath::ivec2, bool> const TestVec2Bool[] =
	{
		{rblxmath::ivec2(0), rblxmath::ivec2(1), false, rblxmath::ivec2(0)},
		{rblxmath::ivec2(0), rblxmath::ivec2(1), true, rblxmath::ivec2(1)},
		{rblxmath::ivec2(-1), rblxmath::ivec2(1), false, rblxmath::ivec2(-1)},
		{rblxmath::ivec2(-1), rblxmath::ivec2(1), true, rblxmath::ivec2(1)}
	};

	test_mix_entry<rblxmath::ivec2, rblxmath::bvec2> const TestBVec2[] =
	{
		{rblxmath::ivec2(0), rblxmath::ivec2(1), rblxmath::bvec2(false), rblxmath::ivec2(0)},
		{rblxmath::ivec2(0), rblxmath::ivec2(1), rblxmath::bvec2(true), rblxmath::ivec2(1)},
		{rblxmath::ivec2(-1), rblxmath::ivec2(1), rblxmath::bvec2(false), rblxmath::ivec2(-1)},
		{rblxmath::ivec2(-1), rblxmath::ivec2(1), rblxmath::bvec2(true), rblxmath::ivec2(1)},
		{rblxmath::ivec2(-1), rblxmath::ivec2(1), rblxmath::bvec2(true, false), rblxmath::ivec2(1, -1)}
	};

	test_mix_entry<rblxmath::ivec3, bool> const TestVec3Bool[] =
	{
		{rblxmath::ivec3(0), rblxmath::ivec3(1), false, rblxmath::ivec3(0)},
		{rblxmath::ivec3(0), rblxmath::ivec3(1), true, rblxmath::ivec3(1)},
		{rblxmath::ivec3(-1), rblxmath::ivec3(1), false, rblxmath::ivec3(-1)},
		{rblxmath::ivec3(-1), rblxmath::ivec3(1), true, rblxmath::ivec3(1)}
	};

	test_mix_entry<rblxmath::ivec3, rblxmath::bvec3> const TestBVec3[] =
	{
		{rblxmath::ivec3(0), rblxmath::ivec3(1), rblxmath::bvec3(false), rblxmath::ivec3(0)},
		{rblxmath::ivec3(0), rblxmath::ivec3(1), rblxmath::bvec3(true), rblxmath::ivec3(1)},
		{rblxmath::ivec3(-1), rblxmath::ivec3(1), rblxmath::bvec3(false), rblxmath::ivec3(-1)},
		{rblxmath::ivec3(-1), rblxmath::ivec3(1), rblxmath::bvec3(true), rblxmath::ivec3(1)},
		{rblxmath::ivec3(1, 2, 3), rblxmath::ivec3(4, 5, 6), rblxmath::bvec3(true, false, true), rblxmath::ivec3(4, 2, 6)}
	};

	test_mix_entry<rblxmath::ivec4, bool> const TestVec4Bool[] = 
	{
		{rblxmath::ivec4(0), rblxmath::ivec4(1), false, rblxmath::ivec4(0)},
		{rblxmath::ivec4(0), rblxmath::ivec4(1), true, rblxmath::ivec4(1)},
		{rblxmath::ivec4(-1), rblxmath::ivec4(1), false, rblxmath::ivec4(-1)},
		{rblxmath::ivec4(-1), rblxmath::ivec4(1), true, rblxmath::ivec4(1)}
	};

	test_mix_entry<rblxmath::ivec4, rblxmath::bvec4> const TestBVec4[] = 
	{
		{rblxmath::ivec4(0, 0, 1, 1), rblxmath::ivec4(2, 2, 3, 3), rblxmath::bvec4(false, true, false, true), rblxmath::ivec4(0, 2, 1, 3)},
		{rblxmath::ivec4(0), rblxmath::ivec4(1), rblxmath::bvec4(true), rblxmath::ivec4(1)},
		{rblxmath::ivec4(-1), rblxmath::ivec4(1), rblxmath::bvec4(false), rblxmath::ivec4(-1)},
		{rblxmath::ivec4(-1), rblxmath::ivec4(1), rblxmath::bvec4(true), rblxmath::ivec4(1)},
		{rblxmath::ivec4(1, 2, 3, 4), rblxmath::ivec4(5, 6, 7, 8), rblxmath::bvec4(true, false, true, false), rblxmath::ivec4(5, 2, 7, 4)}
	};

	int Error = 0;

	// Float with bool
	{
		for(std::size_t i = 0; i < sizeof(TestBool) / sizeof(test_mix_entry<int, bool>); ++i)
		{
			int const Result = rblxmath::mix(TestBool[i].x, TestBool[i].y, TestBool[i].a);
			Error += Result == TestBool[i].Result ? 0 : 1;
		}
	}

	// Float with float
	{
		for(std::size_t i = 0; i < sizeof(TestInt) / sizeof(test_mix_entry<int, int>); ++i)
		{
			int const Result = rblxmath::mix(TestInt[i].x, TestInt[i].y, TestInt[i].a);
			Error += Result == TestInt[i].Result ? 0 : 1;
		}
	}

	// vec2 with bool
	{
		for(std::size_t i = 0; i < sizeof(TestVec2Bool) / sizeof(test_mix_entry<rblxmath::ivec2, bool>); ++i)
		{
			rblxmath::ivec2 const Result = rblxmath::mix(TestVec2Bool[i].x, TestVec2Bool[i].y, TestVec2Bool[i].a);
			Error += rblxmath::all(rblxmath::equal(Result, TestVec2Bool[i].Result)) ? 0 : 1;
		}
	}

	// vec2 with bvec2
	{
		for(std::size_t i = 0; i < sizeof(TestBVec2) / sizeof(test_mix_entry<rblxmath::ivec2, rblxmath::bvec2>); ++i)
		{
			rblxmath::ivec2 const Result = rblxmath::mix(TestBVec2[i].x, TestBVec2[i].y, TestBVec2[i].a);
			Error += rblxmath::all(rblxmath::equal(Result, TestBVec2[i].Result)) ? 0 : 1;
		}
	}

	// vec3 with bool
	{
		for(std::size_t i = 0; i < sizeof(TestVec3Bool) / sizeof(test_mix_entry<rblxmath::ivec3, bool>); ++i)
		{
			rblxmath::ivec3 const Result = rblxmath::mix(TestVec3Bool[i].x, TestVec3Bool[i].y, TestVec3Bool[i].a);
			Error += rblxmath::all(rblxmath::equal(Result, TestVec3Bool[i].Result)) ? 0 : 1;
		}
	}

	// vec3 with bvec3
	{
		for(std::size_t i = 0; i < sizeof(TestBVec3) / sizeof(test_mix_entry<rblxmath::ivec3, rblxmath::bvec3>); ++i)
		{
			rblxmath::ivec3 const Result = rblxmath::mix(TestBVec3[i].x, TestBVec3[i].y, TestBVec3[i].a);
			Error += rblxmath::all(rblxmath::equal(Result, TestBVec3[i].Result)) ? 0 : 1;
		}
	}

	// vec4 with bool
	{
		for(std::size_t i = 0; i < sizeof(TestVec4Bool) / sizeof(test_mix_entry<rblxmath::ivec4, bool>); ++i)
		{
			rblxmath::ivec4 const Result = rblxmath::mix(TestVec4Bool[i].x, TestVec4Bool[i].y, TestVec4Bool[i].a);
			Error += rblxmath::all(rblxmath::equal(Result, TestVec4Bool[i].Result)) ? 0 : 1;
		}
	}

	// vec4 with bvec4
	{
		for(std::size_t i = 0; i < sizeof(TestBVec4) / sizeof(test_mix_entry<rblxmath::ivec4, rblxmath::bvec4>); ++i)
		{
			rblxmath::ivec4 const Result = rblxmath::mix(TestBVec4[i].x, TestBVec4[i].y, TestBVec4[i].a);
			Error += rblxmath::all(rblxmath::equal(Result, TestBVec4[i].Result)) ? 0 : 1;
		}
	}

	return Error;
}

int main()
{
	int Error = 0;

	Error += isPowerOfTwo::test();
	Error += prevPowerOfTwo::test();
	Error += nextPowerOfTwo::test();
	Error += prevMultiple::test();
	Error += nextMultiple::test();
	Error += findNSB::test();

	Error += test_mix();

	return Error;
}
