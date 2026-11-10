#include <rblxmath/gtc/round.hpp>
#include <rblxmath/gtc/type_precision.hpp>
#include <rblxmath/gtc/vec1.hpp>
#include <rblxmath/gtc/epsilon.hpp>
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

	static int test_int16()
	{
		type<rblxmath::int16> const Data[] =
		{
			{0x0001, true},
			{0x0002, true},
			{0x0004, true},
			{0x0080, true},
			{0x0000, true},
			{0x0003, false}
		};

		int Error(0);

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<rblxmath::int16>); i < n; ++i)
		{
			bool Result = rblxmath::isPowerOfTwo(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		return Error;
	}

	static int test_uint16()
	{
		type<rblxmath::uint16> const Data[] =
		{
			{0x0001, true},
			{0x0002, true},
			{0x0004, true},
			{0x0000, true},
			{0x0000, true},
			{0x0003, false}
		};

		int Error(0);

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<rblxmath::uint16>); i < n; ++i)
		{
			bool Result = rblxmath::isPowerOfTwo(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		return Error;
	}

	static int test_int32()
	{
		type<int> const Data[] =
		{
			{0x00000001, true},
			{0x00000002, true},
			{0x00000004, true},
			{0x0000000f, false},
			{0x00000000, true},
			{0x00000003, false}
		};

		int Error(0);

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<int>); i < n; ++i)
		{
			bool Result = rblxmath::isPowerOfTwo(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<int>); i < n; ++i)
		{
			rblxmath::bvec1 Result = rblxmath::isPowerOfTwo(rblxmath::ivec1(Data[i].Value));
			Error += rblxmath::all(rblxmath::equal(rblxmath::bvec1(Data[i].Return), Result)) ? 0 : 1;
		}

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<int>); i < n; ++i)
		{
			rblxmath::bvec2 Result = rblxmath::isPowerOfTwo(rblxmath::ivec2(Data[i].Value));
			Error += rblxmath::all(rblxmath::equal(rblxmath::bvec2(Data[i].Return), Result)) ? 0 : 1;
		}

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<int>); i < n; ++i)
		{
			rblxmath::bvec3 Result = rblxmath::isPowerOfTwo(rblxmath::ivec3(Data[i].Value));
			Error += rblxmath::all(rblxmath::equal(rblxmath::bvec3(Data[i].Return), Result)) ? 0 : 1;
		}

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<int>); i < n; ++i)
		{
			rblxmath::bvec4 Result = rblxmath::isPowerOfTwo(rblxmath::ivec4(Data[i].Value));
			Error += rblxmath::all(rblxmath::equal(rblxmath::bvec4(Data[i].Return), Result)) ? 0 : 1;
		}

		return Error;
	}

	static int test_uint32()
	{
		type<rblxmath::uint> const Data[] =
		{
			{0x00000001, true},
			{0x00000002, true},
			{0x00000004, true},
			{0x80000000, true},
			{0x00000000, true},
			{0x00000003, false}
		};

		int Error(0);

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<rblxmath::uint>); i < n; ++i)
		{
			bool Result = rblxmath::isPowerOfTwo(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		return Error;
	}

	static int test()
	{
		int Error(0);

		Error += test_int16();
		Error += test_uint16();
		Error += test_int32();
		Error += test_uint32();

		return Error;
	}
}//isPowerOfTwo

namespace ceilPowerOfTwo_advanced
{
	template<typename genIUType>
	RBLXMATH_FUNC_QUALIFIER
	static genIUType highestBitValue(genIUType Value)
	{
		genIUType tmp = Value;
		genIUType result = genIUType(0);
		while(tmp)
		{
			result = (tmp & (~tmp + 1)); // grab lowest bit
			tmp &= ~result; // clear lowest bit
		}
		return result;
	}

	template<typename genType>
	RBLXMATH_FUNC_QUALIFIER
	static genType ceilPowerOfTwo_loop(genType value)
	{
		return rblxmath::isPowerOfTwo(value) ? value : highestBitValue(value) << 1;
	}

	template<typename genType>
	struct type
	{
		genType		Value;
		genType		Return;
	};

	static int test_int32()
	{
		type<rblxmath::int32> const Data[] =
		{
			{0x0000ffff, 0x00010000},
			{-3, -4},
			{-8, -8},
			{0x00000001, 0x00000001},
			{0x00000002, 0x00000002},
			{0x00000004, 0x00000004},
			{0x00000007, 0x00000008},
			{0x0000fff0, 0x00010000},
			{0x0000f000, 0x00010000},
			{0x08000000, 0x08000000},
			{0x00000000, 0x00000000},
			{0x00000003, 0x00000004}
		};

		int Error(0);

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<rblxmath::int32>); i < n; ++i)
		{
			rblxmath::int32 Result = rblxmath::ceilPowerOfTwo(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		return Error;
	}

	static int test_uint32()
	{
		type<rblxmath::uint32> const Data[] =
		{
			{0x00000001, 0x00000001},
			{0x00000002, 0x00000002},
			{0x00000004, 0x00000004},
			{0x00000007, 0x00000008},
			{0x0000ffff, 0x00010000},
			{0x0000fff0, 0x00010000},
			{0x0000f000, 0x00010000},
			{0x80000000, 0x80000000},
			{0x00000000, 0x00000000},
			{0x00000003, 0x00000004}
		};

		int Error(0);

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<rblxmath::uint32>); i < n; ++i)
		{
			rblxmath::uint32 Result = rblxmath::ceilPowerOfTwo(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		return Error;
	}

	static int perf()
	{
		int Error(0);

		std::vector<rblxmath::uint> v;
		v.resize(10000);

		std::clock_t Timestramp0 = std::clock();

		for(rblxmath::uint32 i = 0, n = static_cast<rblxmath::uint>(v.size()); i < n; ++i)
			v[i] = ceilPowerOfTwo_loop(i);

		std::clock_t Timestramp1 = std::clock();

		for(rblxmath::uint32 i = 0, n = static_cast<rblxmath::uint>(v.size()); i < n; ++i)
			v[i] = rblxmath::ceilPowerOfTwo(i);

		std::clock_t Timestramp2 = std::clock();

		std::printf("ceilPowerOfTwo_loop: %d clocks\n", static_cast<int>(Timestramp1 - Timestramp0));
		std::printf("rblxmath::ceilPowerOfTwo: %d clocks\n", static_cast<int>(Timestramp2 - Timestramp1));

		return Error;
	}

	static int test()
	{
		int Error(0);

		Error += test_int32();
		Error += test_uint32();

		return Error;
	}
}//namespace ceilPowerOfTwo_advanced

namespace roundPowerOfTwo
{
	static int test()
	{
		int Error = 0;
		
		rblxmath::uint32 const A = rblxmath::roundPowerOfTwo(7u);
		Error += A == 8u ? 0 : 1;
		
		rblxmath::uint32 const B = rblxmath::roundPowerOfTwo(15u);
		Error += B == 16u ? 0 : 1;

		rblxmath::uint32 const C = rblxmath::roundPowerOfTwo(31u);
		Error += C == 32u ? 0 : 1;
		
		rblxmath::uint32 const D = rblxmath::roundPowerOfTwo(9u);
		Error += D == 8u ? 0 : 1;
		
		rblxmath::uint32 const E = rblxmath::roundPowerOfTwo(17u);
		Error += E == 16u ? 0 : 1;
		
		rblxmath::uint32 const F = rblxmath::roundPowerOfTwo(33u);
		Error += F == 32u ? 0 : 1;
		
		return Error;
	}
}//namespace roundPowerOfTwo

namespace floorPowerOfTwo
{
	static int test()
	{
		int Error = 0;
		
		rblxmath::uint32 const A = rblxmath::floorPowerOfTwo(7u);
		Error += A == 4u ? 0 : 1;
		
		rblxmath::uint32 const B = rblxmath::floorPowerOfTwo(15u);
		Error += B == 8u ? 0 : 1;
		
		rblxmath::uint32 const C = rblxmath::floorPowerOfTwo(31u);
		Error += C == 16u ? 0 : 1;
		
		return Error;
	}
}//namespace floorPowerOfTwo

namespace ceilPowerOfTwo
{
	static int test()
	{
		int Error = 0;
		
		rblxmath::uint32 const A = rblxmath::ceilPowerOfTwo(7u);
		Error += A == 8u ? 0 : 1;
		
		rblxmath::uint32 const B = rblxmath::ceilPowerOfTwo(15u);
		Error += B == 16u ? 0 : 1;
		
		rblxmath::uint32 const C = rblxmath::ceilPowerOfTwo(31u);
		Error += C == 32u ? 0 : 1;
		
		return Error;
	}
}//namespace ceilPowerOfTwo

namespace floorMultiple
{
	template<typename genType>
	struct type
	{
		genType		Source;
		genType		Multiple;
		genType		Return;
		genType		Epsilon;
	};

	static int test_float()
	{
		type<rblxmath::float64> const Data[] = 
		{
			{3.4, 0.3, 3.3, 0.0001},
			{-1.4, 0.3, -1.5, 0.0001},
		};

		int Error(0);
		
		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<rblxmath::float64>); i < n; ++i)
		{
			rblxmath::float64 Result = rblxmath::floorMultiple(Data[i].Source, Data[i].Multiple);
			Error += rblxmath::epsilonEqual(Data[i].Return, Result, Data[i].Epsilon) ? 0 : 1;
		}

		return Error;
	}

	static int test()
	{
		int Error(0);

		Error += test_float();

		return Error;
	}
}//namespace floorMultiple

namespace ceilMultiple
{
	template<typename genType>
	struct type
	{
		genType		Source;
		genType		Multiple;
		genType		Return;
		genType		Epsilon;
	};

	static int test_float()
	{
		type<rblxmath::float64> const Data[] = 
		{
			{3.4, 0.3, 3.6, 0.0001},
			{-1.4, 0.3, -1.2, 0.0001},
		};

		int Error(0);

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<rblxmath::float64>); i < n; ++i)
		{
			rblxmath::float64 Result = rblxmath::ceilMultiple(Data[i].Source, Data[i].Multiple);
			Error += rblxmath::epsilonEqual(Data[i].Return, Result, Data[i].Epsilon) ? 0 : 1;
		}

		return Error;
	}

	static int test_int()
	{
		type<int> const Data[] = 
		{
			{3, 4, 4, 0},
			{7, 4, 8, 0},
			{5, 4, 8, 0},
			{1, 4, 4, 0},
			{1, 3, 3, 0},
			{4, 3, 6, 0},
			{4, 1, 4, 0},
			{1, 1, 1, 0},
			{7, 1, 7, 0},
		};

		int Error(0);

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<int>); i < n; ++i)
		{
			int Result = rblxmath::ceilMultiple(Data[i].Source, Data[i].Multiple);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		return Error;
	}

	static int test()
	{
		int Error(0);

		Error += test_int();
		Error += test_float();

		return Error;
	}
}//namespace ceilMultiple

int main()
{
	int Error(0);

	Error += isPowerOfTwo::test();
	Error += floorPowerOfTwo::test();
	Error += roundPowerOfTwo::test();
	Error += ceilPowerOfTwo::test();
	Error += ceilPowerOfTwo_advanced::test();
	
	Error += ceilPowerOfTwo_advanced::perf();

	Error += floorMultiple::test();
	Error += ceilMultiple::test();

	return Error;
}
