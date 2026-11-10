#include <rblxmath/integer.hpp>
#include <rblxmath/vector_relational.hpp>
#include <rblxmath/ext/vector_int1.hpp>
#include <rblxmath/ext/vector_int2.hpp>
#include <rblxmath/ext/vector_int3.hpp>
#include <rblxmath/ext/vector_int4.hpp>
#include <rblxmath/ext/vector_uint1.hpp>
#include <rblxmath/ext/vector_uint2.hpp>
#include <rblxmath/ext/vector_uint3.hpp>
#include <rblxmath/ext/vector_uint4.hpp>
#include <rblxmath/ext/scalar_int_sized.hpp>
#include <rblxmath/ext/scalar_uint_sized.hpp>
#include <vector>
#include <ctime>
#include <cstdio>

enum result
{
	SUCCESS,
	FAIL,
	ASSERT,
	STATIC_ASSERT
};

namespace bitfieldInsert
{
	template<typename genType>
	struct type
	{
		genType		Base;
		genType		Insert;
		int			Offset;
		int			Bits;
		genType		Return;
	};

	typedef type<rblxmath::uint> typeU32;

	typeU32 const Data32[] =
	{
		{0x00000000, 0xffffffff,  0, 32, 0xffffffff},
		{0x00000000, 0xffffffff,  0, 31, 0x7fffffff},
		{0x00000000, 0xffffffff,  0,  0, 0x00000000},
		{0xff000000, 0x000000ff,  8,  8, 0xff00ff00},
		{0xffff0000, 0xffff0000, 16, 16, 0x00000000},
		{0x0000ffff, 0x0000ffff, 16, 16, 0xffffffff}
	};

	static int test()
	{
		int Error = 0;
		rblxmath::uint count = sizeof(Data32) / sizeof(typeU32);
		
		for(rblxmath::uint i = 0; i < count; ++i)
		{
			rblxmath::uint Return = rblxmath::bitfieldInsert(
				Data32[i].Base,
				Data32[i].Insert,
				Data32[i].Offset,
				Data32[i].Bits);

			Error += Data32[i].Return == Return ? 0 : 1;
		}
		
		return Error;
	}
}//bitfieldInsert

namespace bitfieldExtract
{
	template<typename genType>
	struct type
	{
		genType		Value;
		int			Offset;
		int			Bits;
		genType		Return;
		result		Result;
	};

	typedef type<rblxmath::uint> typeU32;

	typeU32 const Data32[] =
	{
		{0xffffffff, 0,32, 0xffffffff, SUCCESS},
		{0xffffffff, 8, 0, 0x00000000, SUCCESS},
		{0x00000000, 0,32, 0x00000000, SUCCESS},
		{0x0f0f0f0f, 0,32, 0x0f0f0f0f, SUCCESS},
		{0x00000000, 8, 0, 0x00000000, SUCCESS},
		{0x80000000,31, 1, 0x00000001, SUCCESS},
		{0x7fffffff,31, 1, 0x00000000, SUCCESS},
		{0x00000300, 8, 8, 0x00000003, SUCCESS},
		{0x0000ff00, 8, 8, 0x000000ff, SUCCESS},
		{0xfffffff0, 0, 5, 0x00000010, SUCCESS},
		{0x000000ff, 1, 3, 0x00000007, SUCCESS},
		{0x000000ff, 0, 3, 0x00000007, SUCCESS},
		{0x00000000, 0, 2, 0x00000000, SUCCESS},
		{0xffffffff, 0, 8, 0x000000ff, SUCCESS},
		{0xffff0000,16,16, 0x0000ffff, SUCCESS},
		{0xfffffff0, 0, 8, 0x00000000, FAIL},
		{0xffffffff,16,16, 0x00000000, FAIL},
		//{0xffffffff,32, 1, 0x00000000, ASSERT}, // Throw an assert 
		//{0xffffffff, 0,33, 0x00000000, ASSERT}, // Throw an assert 
		//{0xffffffff,16,16, 0x00000000, ASSERT}, // Throw an assert 
	};

	static int test()
	{
		int Error = 0;

		rblxmath::uint count = sizeof(Data32) / sizeof(typeU32);

		for(rblxmath::uint i = 0; i < count; ++i)
		{
			rblxmath::uint Return = rblxmath::bitfieldExtract(
				Data32[i].Value, 
				Data32[i].Offset, 
				Data32[i].Bits);
			
			bool Compare = Data32[i].Return == Return;

			if(Data32[i].Result == SUCCESS && Compare)
				continue;
			else if(Data32[i].Result == FAIL && !Compare)
				continue;

			Error += 1;
		}

		return Error;
	}
}//extractField

namespace bitfieldReverse
{
/*
	RBLXMATH_FUNC_QUALIFIER unsigned int bitfieldReverseLoop(unsigned int v)
	{
		unsigned int Result(0);
		unsigned int const BitSize = static_cast<unsigned int>(sizeof(unsigned int) * 8);
		for(unsigned int i = 0; i < BitSize; ++i)
		{
			unsigned int const BitSet(v & (static_cast<unsigned int>(1) << i));
			unsigned int const BitFirst(BitSet >> i);
			Result |= BitFirst << (BitSize - 1 - i);
		}
		return Result;
	}

	RBLXMATH_FUNC_QUALIFIER rblxmath::uint64_t bitfieldReverseLoop(rblxmath::uint64_t v)
	{
		rblxmath::uint64_t Result(0);
		rblxmath::uint64_t const BitSize = static_cast<rblxmath::uint64_t>(sizeof(unsigned int) * 8);
		for(rblxmath::uint64_t i = 0; i < BitSize; ++i)
		{
			rblxmath::uint64_t const BitSet(v & (static_cast<rblxmath::uint64_t>(1) << i));
			rblxmath::uint64_t const BitFirst(BitSet >> i);
			Result |= BitFirst << (BitSize - 1 - i);
		}
		return Result;
	}
*/
	template<rblxmath::length_t L, typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER rblxmath::vec<L, T, Q> bitfieldReverseLoop(rblxmath::vec<L, T, Q> const& v)
	{
		RBLXMATH_STATIC_ASSERT(std::numeric_limits<T>::is_integer, "'bitfieldReverse' only accept integer values");

		rblxmath::vec<L, T, Q> Result(0);
		T const BitSize = static_cast<T>(sizeof(T) * 8);
		for(T i = 0; i < BitSize; ++i)
		{
			rblxmath::vec<L, T, Q> const BitSet(v & (static_cast<T>(1) << i));
			rblxmath::vec<L, T, Q> const BitFirst(BitSet >> i);
			Result |= BitFirst << (BitSize - 1 - i);
		}
		return Result;
	}

	template<typename T>
	RBLXMATH_FUNC_QUALIFIER T bitfieldReverseLoop(T v)
	{
		return bitfieldReverseLoop(rblxmath::vec<1, T>(v)).x;
	}

	RBLXMATH_FUNC_QUALIFIER rblxmath::uint32 bitfieldReverseUint32(rblxmath::uint32 x)
	{
		x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
		x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
		x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
		x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
		x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;
		return x;
	}

	RBLXMATH_FUNC_QUALIFIER rblxmath::uint64 bitfieldReverseUint64(rblxmath::uint64 x)
	{
		x = (x & 0x5555555555555555) <<  1 | (x & 0xAAAAAAAAAAAAAAAA) >>  1;
		x = (x & 0x3333333333333333) <<  2 | (x & 0xCCCCCCCCCCCCCCCC) >>  2;
		x = (x & 0x0F0F0F0F0F0F0F0F) <<  4 | (x & 0xF0F0F0F0F0F0F0F0) >>  4;
		x = (x & 0x00FF00FF00FF00FF) <<  8 | (x & 0xFF00FF00FF00FF00) >>  8;
		x = (x & 0x0000FFFF0000FFFF) << 16 | (x & 0xFFFF0000FFFF0000) >> 16;
		x = (x & 0x00000000FFFFFFFF) << 32 | (x & 0xFFFFFFFF00000000) >> 32;
		return x;
	}

	template<bool EXEC = false>
	struct compute_bitfieldReverseStep
	{
		template<rblxmath::length_t L, typename T, rblxmath::qualifier Q>
		RBLXMATH_FUNC_QUALIFIER static rblxmath::vec<L, T, Q> call(rblxmath::vec<L, T, Q> const& v, T, T)
		{
			return v;
		}
	};

	template<>
	struct compute_bitfieldReverseStep<true>
	{
		template<rblxmath::length_t L, typename T, rblxmath::qualifier Q>
		RBLXMATH_FUNC_QUALIFIER static rblxmath::vec<L, T, Q> call(rblxmath::vec<L, T, Q> const& v, T Mask, T Shift)
		{
			return (v & Mask) << Shift | (v & (~Mask)) >> Shift;
		}
	};

#		if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#			pragma warning(push)
#			pragma warning(disable : 4309)
#		endif

	template<rblxmath::length_t L, typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER rblxmath::vec<L, T, Q> bitfieldReverseOps(rblxmath::vec<L, T, Q> const& v)
	{
		rblxmath::vec<L, T, Q> x(v);
		x = compute_bitfieldReverseStep<sizeof(T) * 8 >=  2>::call(x, static_cast<T>(0x5555555555555555ull), static_cast<T>( 1));
		x = compute_bitfieldReverseStep<sizeof(T) * 8 >=  4>::call(x, static_cast<T>(0x3333333333333333ull), static_cast<T>( 2));
		x = compute_bitfieldReverseStep<sizeof(T) * 8 >=  8>::call(x, static_cast<T>(0x0F0F0F0F0F0F0F0Full), static_cast<T>( 4));
		x = compute_bitfieldReverseStep<sizeof(T) * 8 >= 16>::call(x, static_cast<T>(0x00FF00FF00FF00FFull), static_cast<T>( 8));
		x = compute_bitfieldReverseStep<sizeof(T) * 8 >= 32>::call(x, static_cast<T>(0x0000FFFF0000FFFFull), static_cast<T>(16));
		x = compute_bitfieldReverseStep<sizeof(T) * 8 >= 64>::call(x, static_cast<T>(0x00000000FFFFFFFFull), static_cast<T>(32));
		return x;
	}

#		if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#			pragma warning(pop)
#		endif

	template<typename genType>
	RBLXMATH_FUNC_QUALIFIER genType bitfieldReverseOps(genType x)
	{
		return bitfieldReverseOps(rblxmath::vec<1, genType, rblxmath::defaultp>(x)).x;
	}

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wpadded"
#endif

	template<typename genType>
	struct type
	{
		genType		Value;
		genType		Return;
		result		Result;
	};

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

	typedef type<rblxmath::uint> typeU32;

	typeU32 const Data32[] =
	{
		{0x00000001, 0x80000000, SUCCESS},
		{0x0000000f, 0xf0000000, SUCCESS},
		{0x000000ff, 0xff000000, SUCCESS},
		{0xf0000000, 0x0000000f, SUCCESS},
		{0xff000000, 0x000000ff, SUCCESS},
		{0xffffffff, 0xffffffff, SUCCESS},
		{0x00000000, 0x00000000, SUCCESS}
	};

	typedef type<rblxmath::uint64> typeU64;

	typeU64 const Data64[] =
	{
		{0x00000000000000ff, 0xff00000000000000, SUCCESS},
		{0x000000000000000f, 0xf000000000000000, SUCCESS},
		{0xf000000000000000, 0x000000000000000f, SUCCESS},
		{0xffffffffffffffff, 0xffffffffffffffff, SUCCESS},
		{0x0000000000000000, 0x0000000000000000, SUCCESS}
	};

	static int test32_bitfieldReverse()
	{
		int Error = 0;
		std::size_t const Count = sizeof(Data32) / sizeof(typeU32);
		
		for(std::size_t i = 0; i < Count; ++i)
		{
			rblxmath::uint Return = rblxmath::bitfieldReverse(Data32[i].Value);
			
			bool Compare = Data32[i].Return == Return;
			
			if(Data32[i].Result == SUCCESS)
				Error += Compare ? 0 : 1;
			else
				Error += Compare ? 1 : 0;
		}
		
		return Error;
	}

	static int test32_bitfieldReverseLoop()
	{
		int Error = 0;
		std::size_t const Count = sizeof(Data32) / sizeof(typeU32);
		
		for(std::size_t i = 0; i < Count; ++i)
		{
			rblxmath::uint Return = bitfieldReverseLoop(Data32[i].Value);
			
			bool Compare = Data32[i].Return == Return;
			
			if(Data32[i].Result == SUCCESS)
				Error += Compare ? 0 : 1;
			else
				Error += Compare ? 1 : 0;
		}
		
		return Error;
	}

	static int test32_bitfieldReverseUint32()
	{
		int Error = 0;
		std::size_t const Count = sizeof(Data32) / sizeof(typeU32);
		
		for(std::size_t i = 0; i < Count; ++i)
		{
			rblxmath::uint Return = bitfieldReverseUint32(Data32[i].Value);
			
			bool Compare = Data32[i].Return == Return;
			
			if(Data32[i].Result == SUCCESS)
				Error += Compare ? 0 : 1;
			else
				Error += Compare ? 1 : 0;
		}
		
		return Error;
	}

	static int test32_bitfieldReverseOps()
	{
		int Error = 0;
		std::size_t const Count = sizeof(Data32) / sizeof(typeU32);
		
		for(std::size_t i = 0; i < Count; ++i)
		{
			rblxmath::uint Return = bitfieldReverseOps(Data32[i].Value);
			
			bool Compare = Data32[i].Return == Return;
			
			if(Data32[i].Result == SUCCESS)
				Error += Compare ? 0 : 1;
			else
				Error += Compare ? 1 : 0;
		}
		
		return Error;
	}

	static int test64_bitfieldReverse()
	{
		int Error = 0;
		std::size_t const Count = sizeof(Data64) / sizeof(typeU64);
		
		for(std::size_t i = 0; i < Count; ++i)
		{
			rblxmath::uint64 Return = rblxmath::bitfieldReverse(Data64[i].Value);
			
			bool Compare = Data64[i].Return == Return;
			
			if(Data64[i].Result == SUCCESS)
				Error += Compare ? 0 : 1;
			else
				Error += Compare ? 1 : 0;
		}
		
		return Error;
	}

	static int test64_bitfieldReverseLoop()
	{
		int Error = 0;
		std::size_t const Count = sizeof(Data64) / sizeof(typeU64);
		
		for(std::size_t i = 0; i < Count; ++i)
		{
			rblxmath::uint64 Return = bitfieldReverseLoop(Data64[i].Value);
			
			bool Compare = Data64[i].Return == Return;
			
			if(Data32[i].Result == SUCCESS)
				Error += Compare ? 0 : 1;
			else
				Error += Compare ? 1 : 0;
		}
		
		return Error;
	}

	static int test64_bitfieldReverseUint64()
	{
		int Error = 0;
		std::size_t const Count = sizeof(Data64) / sizeof(typeU64);
		
		for(std::size_t i = 0; i < Count; ++i)
		{
			rblxmath::uint64 Return = bitfieldReverseUint64(Data64[i].Value);
			
			bool Compare = Data64[i].Return == Return;
			
			if(Data64[i].Result == SUCCESS)
				Error += Compare ? 0 : 1;
			else
				Error += Compare ? 1 : 0;
		}
		
		return Error;
	}

	static int test64_bitfieldReverseOps()
	{
		int Error = 0;
		std::size_t const Count = sizeof(Data64) / sizeof(typeU64);
		
		for(std::size_t i = 0; i < Count; ++i)
		{
			rblxmath::uint64 Return = bitfieldReverseOps(Data64[i].Value);
			
			bool Compare = Data64[i].Return == Return;
			
			if(Data64[i].Result == SUCCESS)
				Error += Compare ? 0 : 1;
			else
				Error += Compare ? 1 : 0;
		}
		
		return Error;
	}

	static int test()
	{
		int Error = 0;

		Error += test32_bitfieldReverse();
		Error += test32_bitfieldReverseLoop();
		Error += test32_bitfieldReverseUint32();
		Error += test32_bitfieldReverseOps();

		Error += test64_bitfieldReverse();
		Error += test64_bitfieldReverseLoop();
		Error += test64_bitfieldReverseUint64();
		Error += test64_bitfieldReverseOps();

		return Error;
	}

	static int perf32(rblxmath::uint32 Count)
	{
		int Error = 0;

		std::vector<rblxmath::uint32> Data;
		Data.resize(static_cast<std::size_t>(Count));

		std::clock_t Timestamps0 = std::clock();

		for(rblxmath::uint32 k = 0; k < Count; ++k)
			Data[k] = rblxmath::bitfieldReverse(k);

		std::clock_t Timestamps1 = std::clock();

		for(rblxmath::uint32 k = 0; k < Count; ++k)
			Data[k] = bitfieldReverseLoop(k);

		std::clock_t Timestamps2 = std::clock();

		for(rblxmath::uint32 k = 0; k < Count; ++k)
			Data[k] = bitfieldReverseUint32(k);

		std::clock_t Timestamps3 = std::clock();

		for(rblxmath::uint32 k = 0; k < Count; ++k)
			Data[k] = bitfieldReverseOps(k);

		std::clock_t Timestamps4 = std::clock();

		std::printf("rblxmath::bitfieldReverse: %d clocks\n", static_cast<int>(Timestamps1 - Timestamps0));
		std::printf("bitfieldReverseLoop: %d clocks\n", static_cast<int>(Timestamps2 - Timestamps1));
		std::printf("bitfieldReverseUint32: %d clocks\n", static_cast<int>(Timestamps3 - Timestamps2));
		std::printf("bitfieldReverseOps: %d clocks\n", static_cast<int>(Timestamps4 - Timestamps3));

		return Error;
	}

	static int perf64(rblxmath::uint64 Count)
	{
		int Error = 0;

		std::vector<rblxmath::uint64> Data;
		Data.resize(static_cast<std::size_t>(Count));

		std::clock_t Timestamps0 = std::clock();

		for(rblxmath::uint64 k = 0; k < Count; ++k)
			Data[static_cast<std::size_t>(k)] = rblxmath::bitfieldReverse(k);

		std::clock_t Timestamps1 = std::clock();

		for(rblxmath::uint64 k = 0; k < Count; ++k)
			Data[static_cast<std::size_t>(k)] = bitfieldReverseLoop<rblxmath::uint64>(k);

		std::clock_t Timestamps2 = std::clock();

		for(rblxmath::uint64 k = 0; k < Count; ++k)
			Data[static_cast<std::size_t>(k)] = bitfieldReverseUint64(k);

		std::clock_t Timestamps3 = std::clock();

		for(rblxmath::uint64 k = 0; k < Count; ++k)
			Data[static_cast<std::size_t>(k)] = bitfieldReverseOps(k);

		std::clock_t Timestamps4 = std::clock();

		std::printf("rblxmath::bitfieldReverse - 64: %d clocks\n", static_cast<int>(Timestamps1 - Timestamps0));
		std::printf("bitfieldReverseLoop - 64: %d clocks\n", static_cast<int>(Timestamps2 - Timestamps1));
		std::printf("bitfieldReverseUint - 64: %d clocks\n", static_cast<int>(Timestamps3 - Timestamps2));
		std::printf("bitfieldReverseOps - 64: %d clocks\n", static_cast<int>(Timestamps4 - Timestamps3));

		return Error;
	}

	static int perf(std::size_t Samples)
	{
		int Error = 0;

		Error += perf32(static_cast<rblxmath::uint32>(Samples));
		Error += perf64(static_cast<rblxmath::uint64>(Samples));

		return Error;
	}
}//bitfieldReverse

namespace findMSB
{
	template<typename genType, typename retType>
	struct type
	{
		genType		Value;
		retType		Return;
	};

#	if RBLXMATH_HAS_BITSCAN_WINDOWS
		template<typename genIUType>
		static int findMSB_intrinsic(genIUType Value)
		{
			RBLXMATH_STATIC_ASSERT(std::numeric_limits<genIUType>::is_integer, "'findMSB' only accept integer values");

			if(Value == 0)
				return -1;

			unsigned long Result(0);
			_BitScanReverse(&Result, Value);
			return int(Result);
		}
#	endif//RBLXMATH_HAS_BITSCAN_WINDOWS

#	if RBLXMATH_ARCH & RBLXMATH_ARCH_AVX && RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
		template<typename genIUType>
		static int findMSB_avx(genIUType Value)
		{
			RBLXMATH_STATIC_ASSERT(std::numeric_limits<genIUType>::is_integer, "'findMSB' only accept integer values");

			if(Value == 0)
				return -1;

			return int(_tzcnt_u32(Value));
		}
#	endif//RBLXMATH_ARCH & RBLXMATH_ARCH_AVX && RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_WINDOWS

	template<typename genIUType>
	static int findMSB_095(genIUType Value)
	{
		RBLXMATH_STATIC_ASSERT(std::numeric_limits<genIUType>::is_integer, "'findMSB' only accept integer values");
		
		if(Value == genIUType(0) || Value == genIUType(-1))
			return -1;
		else if(Value > 0)
		{
			genIUType Bit = genIUType(-1);
			for(genIUType tmp = Value; tmp > 0; tmp >>= 1, ++Bit){}
			return static_cast<int>(Bit);
		}
		else //if(Value < 0)
		{
			int const BitCount(sizeof(genIUType) * 8);
			int MostSignificantBit(-1);
			for(int BitIndex(0); BitIndex < BitCount; ++BitIndex)
				MostSignificantBit = (Value & (1 << BitIndex)) ? MostSignificantBit : BitIndex;
			assert(MostSignificantBit >= 0);
			return MostSignificantBit;
		}
	}

	template<typename genIUType>
	static int findMSB_nlz1(genIUType x)
	{
		RBLXMATH_STATIC_ASSERT(std::numeric_limits<genIUType>::is_integer, "'findMSB' only accept integer values");

		if (x == 0)
			return -1;

		int n = 0;
		if (x <= 0x0000FFFF) {n = n +16; x = x <<16;}
		if (x <= 0x00FFFFFF) {n = n + 8; x = x << 8;}
		if (x <= 0x0FFFFFFF) {n = n + 4; x = x << 4;}
		if (x <= 0x3FFFFFFF) {n = n + 2; x = x << 2;}
		if (x <= 0x7FFFFFFF) {n = n + 1;}
		return 31 - n;
	}

	static int findMSB_nlz2(unsigned int x)
	{
		unsigned int y;
		int n = 32;

		y = x >>16;  if (y != 0) {n = n -16;  x = y;}
		y = x >> 8;  if (y != 0) {n = n - 8;  x = y;}
		y = x >> 4;  if (y != 0) {n = n - 4;  x = y;}
		y = x >> 2;  if (y != 0) {n = n - 2;  x = y;}
		y = x >> 1;  if (y != 0) return n - 2;
		return 32 - (n - static_cast<int>(x));
	}

	static int findMSB_pop(unsigned int x)
	{
		x = x | (x >> 1);
		x = x | (x >> 2);
		x = x | (x >> 4);
		x = x | (x >> 8);
		x = x | (x >>16);
		return 31 - rblxmath::bitCount(~x);
	}

	static int perf_int(std::size_t Count)
	{
		type<int, int> const Data[] =
		{
			{0x00000000, -1},
			{0x00000001,  0},
			{0x00000002,  1},
			{0x00000003,  1},
			{0x00000004,  2},
			{0x00000005,  2},
			{0x00000007,  2},
			{0x00000008,  3},
			{0x00000010,  4},
			{0x00000020,  5},
			{0x00000040,  6},
			{0x00000080,  7},
			{0x00000100,  8},
			{0x00000200,  9},
			{0x00000400, 10},
			{0x00000800, 11},
			{0x00001000, 12},
			{0x00002000, 13},
			{0x00004000, 14},
			{0x00008000, 15},
			{0x00010000, 16},
			{0x00020000, 17},
			{0x00040000, 18},
			{0x00080000, 19},
			{0x00100000, 20},
			{0x00200000, 21},
			{0x00400000, 22},
			{0x00800000, 23},
			{0x01000000, 24},
			{0x02000000, 25},
			{0x04000000, 26},
			{0x08000000, 27},
			{0x10000000, 28},
			{0x20000000, 29},
			{0x40000000, 30}
		};

		int Error(0);

		std::clock_t Timestamps0 = std::clock();

		for(std::size_t k = 0; k < Count; ++k)
		for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<int, int>); ++i)
		{
			int Result = rblxmath::findMSB(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		std::clock_t Timestamps1 = std::clock();

		for(std::size_t k = 0; k < Count; ++k)
		for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<int, int>); ++i)
		{
			int Result = findMSB_nlz1(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		std::clock_t Timestamps2 = std::clock();

		for(std::size_t k = 0; k < Count; ++k)
		for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<int, int>); ++i)
		{
			int Result = findMSB_nlz2(static_cast<unsigned int>(Data[i].Value));
			Error += Data[i].Return == Result ? 0 : 1;
		}

		std::clock_t Timestamps3 = std::clock();

		for(std::size_t k = 0; k < Count; ++k)
		for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<int, int>); ++i)
		{
			int Result = findMSB_095(static_cast<unsigned int>(Data[i].Value));
			Error += Data[i].Return == Result ? 0 : 1;
		}

		std::clock_t Timestamps4 = std::clock();

#		if RBLXMATH_HAS_BITSCAN_WINDOWS
			for(std::size_t k = 0; k < Count; ++k)
			for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<int, int>); ++i)
			{
				int Result = findMSB_intrinsic(Data[i].Value);
				Error += Data[i].Return == Result ? 0 : 1;
			}
#		endif//RBLXMATH_HAS_BITSCAN_WINDOWS

		std::clock_t Timestamps5 = std::clock();

		for(std::size_t k = 0; k < Count; ++k)
		for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<int, int>); ++i)
		{
			int Result = findMSB_pop(static_cast<unsigned int>(Data[i].Value));
			Error += Data[i].Return == Result ? 0 : 1;
		}

		std::clock_t Timestamps6 = std::clock();

#		if RBLXMATH_ARCH & RBLXMATH_ARCH_AVX && RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
			for(std::size_t k = 0; k < Count; ++k)
			for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<int, int>); ++i)
			{
				int Result = findMSB_avx(Data[i].Value);
				Error += Data[i].Return == Result ? 0 : 1;
			}

			std::clock_t Timestamps7 = std::clock();
#		endif

		std::printf("rblxmath::findMSB: %d clocks\n", static_cast<int>(Timestamps1 - Timestamps0));
		std::printf("findMSB - nlz1: %d clocks\n", static_cast<int>(Timestamps2 - Timestamps1));
		std::printf("findMSB - nlz2: %d clocks\n", static_cast<int>(Timestamps3 - Timestamps2));
		std::printf("findMSB - 0.9.5: %d clocks\n", static_cast<int>(Timestamps4 - Timestamps3));

#		if RBLXMATH_HAS_BITSCAN_WINDOWS
			std::printf("findMSB - intrinsics: %d clocks\n", static_cast<int>(Timestamps5 - Timestamps4));
#		endif//RBLXMATH_HAS_BITSCAN_WINDOWS
		std::printf("findMSB - pop: %d clocks\n", static_cast<int>(Timestamps6 - Timestamps5));

#		if RBLXMATH_ARCH & RBLXMATH_ARCH_AVX && RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
			std::printf("findMSB - avx tzcnt: %d clocks\n", static_cast<int>(Timestamps7 - Timestamps6));
#		endif//RBLXMATH_ARCH & RBLXMATH_ARCH_AVX && RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_WINDOWS

		return Error;
	}

	static int test_ivec4()
	{
		type<rblxmath::ivec4, rblxmath::ivec4> const Data[] =
		{
			{rblxmath::ivec4(0x00000000), rblxmath::ivec4(-1)},
			{rblxmath::ivec4(0x00000001), rblxmath::ivec4( 0)},
			{rblxmath::ivec4(0x00000002), rblxmath::ivec4( 1)},
			{rblxmath::ivec4(0x00000003), rblxmath::ivec4( 1)},
			{rblxmath::ivec4(0x00000004), rblxmath::ivec4( 2)},
			{rblxmath::ivec4(0x00000005), rblxmath::ivec4( 2)},
			{rblxmath::ivec4(0x00000007), rblxmath::ivec4( 2)},
			{rblxmath::ivec4(0x00000008), rblxmath::ivec4( 3)},
			{rblxmath::ivec4(0x00000010), rblxmath::ivec4( 4)},
			{rblxmath::ivec4(0x00000020), rblxmath::ivec4( 5)},
			{rblxmath::ivec4(0x00000040), rblxmath::ivec4( 6)},
			{rblxmath::ivec4(0x00000080), rblxmath::ivec4( 7)},
			{rblxmath::ivec4(0x00000100), rblxmath::ivec4( 8)},
			{rblxmath::ivec4(0x00000200), rblxmath::ivec4( 9)},
			{rblxmath::ivec4(0x00000400), rblxmath::ivec4(10)},
			{rblxmath::ivec4(0x00000800), rblxmath::ivec4(11)},
			{rblxmath::ivec4(0x00001000), rblxmath::ivec4(12)},
			{rblxmath::ivec4(0x00002000), rblxmath::ivec4(13)},
			{rblxmath::ivec4(0x00004000), rblxmath::ivec4(14)},
			{rblxmath::ivec4(0x00008000), rblxmath::ivec4(15)},
			{rblxmath::ivec4(0x00010000), rblxmath::ivec4(16)},
			{rblxmath::ivec4(0x00020000), rblxmath::ivec4(17)},
			{rblxmath::ivec4(0x00040000), rblxmath::ivec4(18)},
			{rblxmath::ivec4(0x00080000), rblxmath::ivec4(19)},
			{rblxmath::ivec4(0x00100000), rblxmath::ivec4(20)},
			{rblxmath::ivec4(0x00200000), rblxmath::ivec4(21)},
			{rblxmath::ivec4(0x00400000), rblxmath::ivec4(22)},
			{rblxmath::ivec4(0x00800000), rblxmath::ivec4(23)},
			{rblxmath::ivec4(0x01000000), rblxmath::ivec4(24)},
			{rblxmath::ivec4(0x02000000), rblxmath::ivec4(25)},
			{rblxmath::ivec4(0x04000000), rblxmath::ivec4(26)},
			{rblxmath::ivec4(0x08000000), rblxmath::ivec4(27)},
			{rblxmath::ivec4(0x10000000), rblxmath::ivec4(28)},
			{rblxmath::ivec4(0x20000000), rblxmath::ivec4(29)},
			{rblxmath::ivec4(0x40000000), rblxmath::ivec4(30)}
		};

		int Error(0);

		for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<rblxmath::ivec4, rblxmath::ivec4>); ++i)
		{
			rblxmath::ivec4 Result0 = rblxmath::findMSB(Data[i].Value);
			Error += rblxmath::all(rblxmath::equal(Data[i].Return, Result0)) ? 0 : 1;
		}

		return Error;
	}

	static int test_int()
	{
		typedef type<rblxmath::uint, int> entry;

		entry const Data[] =
		{
			{0x00000000, -1},
			{0x00000001,  0},
			{0x00000002,  1},
			{0x00000003,  1},
			{0x00000004,  2},
			{0x00000005,  2},
			{0x00000007,  2},
			{0x00000008,  3},
			{0x00000010,  4},
			{0x00000020,  5},
			{0x00000040,  6},
			{0x00000080,  7},
			{0x00000100,  8},
			{0x00000200,  9},
			{0x00000400, 10},
			{0x00000800, 11},
			{0x00001000, 12},
			{0x00002000, 13},
			{0x00004000, 14},
			{0x00008000, 15},
			{0x00010000, 16},
			{0x00020000, 17},
			{0x00040000, 18},
			{0x00080000, 19},
			{0x00100000, 20},
			{0x00200000, 21},
			{0x00400000, 22},
			{0x00800000, 23},
			{0x01000000, 24},
			{0x02000000, 25},
			{0x04000000, 26},
			{0x08000000, 27},
			{0x10000000, 28},
			{0x20000000, 29},
			{0x40000000, 30}
		};

		int Error(0);

		for(std::size_t i = 0; i < sizeof(Data) / sizeof(entry); ++i)
		{
			int Result0 = rblxmath::findMSB(Data[i].Value);
			Error += Data[i].Return == Result0 ? 0 : 1;
		}

		for(std::size_t i = 0; i < sizeof(Data) / sizeof(entry); ++i)
		{
			int Result0 = findMSB_nlz1(Data[i].Value);
			Error += Data[i].Return == Result0 ? 0 : 1;
		}
/*
		for(std::size_t i = 0; i < sizeof(Data) / sizeof(entry); ++i)
		{
			int Result0 = findMSB_nlz2(Data[i].Value);
			Error += Data[i].Return == Result0 ? 0 : 1;
		}
*/
		for(std::size_t i = 0; i < sizeof(Data) / sizeof(entry); ++i)
		{
			int Result0 = findMSB_095(Data[i].Value);
			Error += Data[i].Return == Result0 ? 0 : 1;
		}

#		if RBLXMATH_HAS_BITSCAN_WINDOWS
			for(std::size_t i = 0; i < sizeof(Data) / sizeof(entry); ++i)
			{
				int Result0 = findMSB_intrinsic(Data[i].Value);
				Error += Data[i].Return == Result0 ? 0 : 1;
			}
#		endif//RBLXMATH_HAS_BITSCAN_WINDOWS

		for(std::size_t i = 0; i < sizeof(Data) / sizeof(entry); ++i)
		{
			int Result0 = findMSB_pop(Data[i].Value);
			Error += Data[i].Return == Result0 ? 0 : 1;
		}

		return Error;
	}

	static int test()
	{
		int Error(0);

		Error += test_ivec4();
		Error += test_int();

		return Error;
	}

	static int perf(std::size_t Samples)
	{
		int Error(0);

		Error += perf_int(Samples);

		return Error;
	}
}//findMSB

namespace findLSB
{
	template<typename genType, typename retType>
	struct type
	{
		genType		Value;
		retType		Return;
	};

	typedef type<int, int> entry;

	entry const DataI32[] =
	{
		{0x00000001,  0},
		{0x00000003,  0},
		{0x00000002,  1},
		// {0x80000000, 31}, // Clang generates an error with this
		{0x00010000, 16},
		{0x7FFF0000, 16},
		{0x7F000000, 24},
		{0x7F00FF00,  8},
		{0x00000000, -1}
	};

#	if RBLXMATH_HAS_BITSCAN_WINDOWS
		template<typename genIUType>
		static int findLSB_intrinsic(genIUType Value)
		{
			RBLXMATH_STATIC_ASSERT(std::numeric_limits<genIUType>::is_integer, "'findLSB' only accept integer values");

			if(Value == 0)
				return -1;

			unsigned long Result(0);
			_BitScanForward(&Result, Value);
			return int(Result);
		}
#	endif

	template<typename genIUType>
	static int findLSB_095(genIUType Value)
	{
		RBLXMATH_STATIC_ASSERT(std::numeric_limits<genIUType>::is_integer, "'findLSB' only accept integer values");
		if(Value == 0)
			return -1;

		genIUType Bit;
		for(Bit = genIUType(0); !(Value & (1 << Bit)); ++Bit){}
		return Bit;
	}

	template<typename genIUType>
	static int findLSB_ntz2(genIUType x)
	{
		if(x == 0)
			return -1;

		return rblxmath::bitCount(~x & (x - static_cast<genIUType>(1)));
	}

	template<typename genIUType>
	static int findLSB_branchfree(genIUType x)
	{
		bool IsNull(x == 0);
		int const Keep(!IsNull);
		int const Discard(IsNull);

		return static_cast<int>(rblxmath::bitCount(~x & (x - static_cast<genIUType>(1)))) * Keep + Discard * -1;
	}

	static int test_int()
	{
		int Error(0);

		for(std::size_t i = 0; i < sizeof(DataI32) / sizeof(entry); ++i)
		{
			int Result = rblxmath::findLSB(DataI32[i].Value);
			Error += DataI32[i].Return == Result ? 0 : 1;
		}

		for(std::size_t i = 0; i < sizeof(DataI32) / sizeof(entry); ++i)
		{
			int Result = findLSB_095(DataI32[i].Value);
			Error += DataI32[i].Return == Result ? 0 : 1;
		}

#		if RBLXMATH_HAS_BITSCAN_WINDOWS
			for(std::size_t i = 0; i < sizeof(DataI32) / sizeof(entry); ++i)
			{
				int Result = findLSB_intrinsic(DataI32[i].Value);
				Error += DataI32[i].Return == Result ? 0 : 1;
			}
#		endif

		for(std::size_t i = 0; i < sizeof(DataI32) / sizeof(entry); ++i)
		{
			int Result = findLSB_ntz2(DataI32[i].Value);
			Error += DataI32[i].Return == Result ? 0 : 1;
		}

		for(std::size_t i = 0; i < sizeof(DataI32) / sizeof(entry); ++i)
		{
			int Result = findLSB_branchfree(DataI32[i].Value);
			Error += DataI32[i].Return == Result ? 0 : 1;
		}

		return Error;
	}

	static int test()
	{
		int Error(0);

		Error += test_int();

		return Error;
	}

	static int perf_int(std::size_t Count)
	{
		int Error(0);

		std::clock_t Timestamps0 = std::clock();

		for(std::size_t k = 0; k < Count; ++k)
		for(std::size_t i = 0; i < sizeof(DataI32) / sizeof(entry); ++i)
		{
			int Result = rblxmath::findLSB(DataI32[i].Value);
			Error += DataI32[i].Return == Result ? 0 : 1;
		}

		std::clock_t Timestamps1 = std::clock();

		for(std::size_t k = 0; k < Count; ++k)
		for(std::size_t i = 0; i < sizeof(DataI32) / sizeof(entry); ++i)
		{
			int Result = findLSB_095(DataI32[i].Value);
			Error += DataI32[i].Return == Result ? 0 : 1;
		}

		std::clock_t Timestamps2 = std::clock();

#		if RBLXMATH_HAS_BITSCAN_WINDOWS
			for(std::size_t k = 0; k < Count; ++k)
			for(std::size_t i = 0; i < sizeof(DataI32) / sizeof(entry); ++i)
			{
				int Result = findLSB_intrinsic(DataI32[i].Value);
				Error += DataI32[i].Return == Result ? 0 : 1;
			}
#		endif

		std::clock_t Timestamps3 = std::clock();

		for(std::size_t k = 0; k < Count; ++k)
		for(std::size_t i = 0; i < sizeof(DataI32) / sizeof(entry); ++i)
		{
			int Result = findLSB_ntz2(DataI32[i].Value);
			Error += DataI32[i].Return == Result ? 0 : 1;
		}

		std::clock_t Timestamps4 = std::clock();

		for(std::size_t k = 0; k < Count; ++k)
		for(std::size_t i = 0; i < sizeof(DataI32) / sizeof(entry); ++i)
		{
			int Result = findLSB_branchfree(DataI32[i].Value);
			Error += DataI32[i].Return == Result ? 0 : 1;
		}

		std::clock_t Timestamps5 = std::clock();

		std::printf("rblxmath::findLSB: %d clocks\n", static_cast<int>(Timestamps1 - Timestamps0));
		std::printf("findLSB - 0.9.5: %d clocks\n", static_cast<int>(Timestamps2 - Timestamps1));

#		if RBLXMATH_HAS_BITSCAN_WINDOWS
			std::printf("findLSB - intrinsics: %d clocks\n", static_cast<int>(Timestamps3 - Timestamps2));
#		endif

		std::printf("findLSB - ntz2: %d clocks\n", static_cast<int>(Timestamps4 - Timestamps3));
		std::printf("findLSB - branchfree: %d clocks\n", static_cast<int>(Timestamps5 - Timestamps4));

		return Error;
	}

	static int perf(std::size_t Samples)
	{
		int Error(0);

		Error += perf_int(Samples);

		return Error;
	}
}//findLSB

namespace uaddCarry
{
	static int test()
	{
		int Error(0);
		
		{
			rblxmath::uint x = std::numeric_limits<rblxmath::uint>::max();
			rblxmath::uint y = 0;
			rblxmath::uint Carry = 0;
			rblxmath::uint Result = rblxmath::uaddCarry(x, y, Carry);

			Error += Carry == 0 ? 0 : 1;
			Error += Result == std::numeric_limits<rblxmath::uint>::max() ? 0 : 1;
		}

		{
			rblxmath::uint x = std::numeric_limits<rblxmath::uint>::max();
			rblxmath::uint y = 1;
			rblxmath::uint Carry = 0;
			rblxmath::uint Result = rblxmath::uaddCarry(x, y, Carry);

			Error += Carry == 1 ? 0 : 1;
			Error += Result == 0 ? 0 : 1;
		}

		{
			rblxmath::uvec1 x(std::numeric_limits<rblxmath::uint>::max());
			rblxmath::uvec1 y(0);
			rblxmath::uvec1 Carry(0);
			rblxmath::uvec1 Result(rblxmath::uaddCarry(x, y, Carry));

			Error += rblxmath::all(rblxmath::equal(Carry, rblxmath::uvec1(0))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(Result, rblxmath::uvec1(std::numeric_limits<rblxmath::uint>::max()))) ? 0 : 1;
		}

		{
			rblxmath::uvec1 x(std::numeric_limits<rblxmath::uint>::max());
			rblxmath::uvec1 y(1);
			rblxmath::uvec1 Carry(0);
			rblxmath::uvec1 Result(rblxmath::uaddCarry(x, y, Carry));

			Error += rblxmath::all(rblxmath::equal(Carry, rblxmath::uvec1(1))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(Result, rblxmath::uvec1(0))) ? 0 : 1;
		}

		return Error;
	}
}//namespace uaddCarry

namespace usubBorrow
{
	static int test()
	{
		int Error(0);
		
		{
			rblxmath::uint x = 16;
			rblxmath::uint y = 17;
			rblxmath::uint Borrow = 0;
			rblxmath::uint Result = rblxmath::usubBorrow(x, y, Borrow);

			Error += Borrow == 1 ? 0 : 1;
			Error += Result == 1 ? 0 : 1;
		}

		{
			rblxmath::uvec1 x(16);
			rblxmath::uvec1 y(17);
			rblxmath::uvec1 Borrow(0);
			rblxmath::uvec1 Result(rblxmath::usubBorrow(x, y, Borrow));

			Error += rblxmath::all(rblxmath::equal(Borrow, rblxmath::uvec1(1))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(Result, rblxmath::uvec1(1))) ? 0 : 1;
		}

		{
			rblxmath::uvec2 x(16);
			rblxmath::uvec2 y(17);
			rblxmath::uvec2 Borrow(0);
			rblxmath::uvec2 Result(rblxmath::usubBorrow(x, y, Borrow));

			Error += rblxmath::all(rblxmath::equal(Borrow, rblxmath::uvec2(1))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(Result, rblxmath::uvec2(1))) ? 0 : 1;
		}

		{
			rblxmath::uvec3 x(16);
			rblxmath::uvec3 y(17);
			rblxmath::uvec3 Borrow(0);
			rblxmath::uvec3 Result(rblxmath::usubBorrow(x, y, Borrow));

			Error += rblxmath::all(rblxmath::equal(Borrow, rblxmath::uvec3(1))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(Result, rblxmath::uvec3(1))) ? 0 : 1;
		}

		{
			rblxmath::uvec4 x(16);
			rblxmath::uvec4 y(17);
			rblxmath::uvec4 Borrow(0);
			rblxmath::uvec4 Result(rblxmath::usubBorrow(x, y, Borrow));

			Error += rblxmath::all(rblxmath::equal(Borrow, rblxmath::uvec4(1))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(Result, rblxmath::uvec4(1))) ? 0 : 1;
		}

		return Error;
	}
}//namespace usubBorrow

namespace umulExtended
{
	static int test()
	{
		int Error(0);
		
		{
			rblxmath::uint x = 2;
			rblxmath::uint y = 3;
			rblxmath::uint msb = 0;
			rblxmath::uint lsb = 0;
			rblxmath::umulExtended(x, y, msb, lsb);

			Error += msb == 0 ? 0 : 1;
			Error += lsb == 6 ? 0 : 1;
		}

		{
			rblxmath::uvec1 x(2);
			rblxmath::uvec1 y(3);
			rblxmath::uvec1 msb(0);
			rblxmath::uvec1 lsb(0);
			rblxmath::umulExtended(x, y, msb, lsb);

			Error += rblxmath::all(rblxmath::equal(msb, rblxmath::uvec1(0))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(lsb, rblxmath::uvec1(6))) ? 0 : 1;
		}

		{
			rblxmath::uvec2 x(2);
			rblxmath::uvec2 y(3);
			rblxmath::uvec2 msb(0);
			rblxmath::uvec2 lsb(0);
			rblxmath::umulExtended(x, y, msb, lsb);

			Error += rblxmath::all(rblxmath::equal(msb, rblxmath::uvec2(0))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(lsb, rblxmath::uvec2(6))) ? 0 : 1;
		}

		{
			rblxmath::uvec3 x(2);
			rblxmath::uvec3 y(3);
			rblxmath::uvec3 msb(0);
			rblxmath::uvec3 lsb(0);
			rblxmath::umulExtended(x, y, msb, lsb);

			Error += rblxmath::all(rblxmath::equal(msb, rblxmath::uvec3(0))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(lsb, rblxmath::uvec3(6))) ? 0 : 1;
		}

		{
			rblxmath::uvec4 x(2);
			rblxmath::uvec4 y(3);
			rblxmath::uvec4 msb(0);
			rblxmath::uvec4 lsb(0);
			rblxmath::umulExtended(x, y, msb, lsb);

			Error += rblxmath::all(rblxmath::equal(msb, rblxmath::uvec4(0))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(lsb, rblxmath::uvec4(6))) ? 0 : 1;
		}

		return Error;
	}
}//namespace umulExtended

namespace imulExtended
{
	static int test()
	{
		int Error(0);
		
		{
			int x = 2;
			int y = 3;
			int msb = 0;
			int lsb = 0;
			rblxmath::imulExtended(x, y, msb, lsb);

			Error += msb == 0 ? 0 : 1;
			Error += lsb == 6 ? 0 : 1;
		}

		{
			rblxmath::ivec1 x(2);
			rblxmath::ivec1 y(3);
			rblxmath::ivec1 msb(0);
			rblxmath::ivec1 lsb(0);
			rblxmath::imulExtended(x, y, msb, lsb);

			Error += rblxmath::all(rblxmath::equal(msb, rblxmath::ivec1(0))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(lsb, rblxmath::ivec1(6))) ? 0 : 1;
		}

		{
			rblxmath::ivec2 x(2);
			rblxmath::ivec2 y(3);
			rblxmath::ivec2 msb(0);
			rblxmath::ivec2 lsb(0);
			rblxmath::imulExtended(x, y, msb, lsb);

			Error += rblxmath::all(rblxmath::equal(msb, rblxmath::ivec2(0))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(lsb, rblxmath::ivec2(6))) ? 0 : 1;
		}

		{
			rblxmath::ivec3 x(2);
			rblxmath::ivec3 y(3);
			rblxmath::ivec3 msb(0);
			rblxmath::ivec3 lsb(0);
			rblxmath::imulExtended(x, y, msb, lsb);

			Error += rblxmath::all(rblxmath::equal(msb, rblxmath::ivec3(0))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(lsb, rblxmath::ivec3(6))) ? 0 : 1;
		}

		{
			rblxmath::ivec4 x(2);
			rblxmath::ivec4 y(3);
			rblxmath::ivec4 msb(0);
			rblxmath::ivec4 lsb(0);
			rblxmath::imulExtended(x, y, msb, lsb);

			Error += rblxmath::all(rblxmath::equal(msb, rblxmath::ivec4(0))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(lsb, rblxmath::ivec4(6))) ? 0 : 1;
		}

		return Error;
	}
}//namespace imulExtended

namespace bitCount
{
	template<typename genType>
	struct type
	{
		genType		Value;
		genType		Return;
	};

	type<int> const DataI32[] =
	{
		{0x00000001,  1},
		{0x00000003,  2},
		{0x00000002,  1},
		{0x7fffffff, 31},
		{0x00000000,  0}
	};

	template<typename T>
	inline int bitCount_if(T v)
	{
		RBLXMATH_STATIC_ASSERT(std::numeric_limits<T>::is_integer, "'bitCount' only accept integer values");

		int Count(0);
		for(T i = 0, n = static_cast<T>(sizeof(T) * 8); i < n; ++i)
		{
			if(v & static_cast<T>(1 << i))
				++Count;
		}
		return Count;
	}

	template<typename T>
	inline int bitCount_vec(T v)
	{
		RBLXMATH_STATIC_ASSERT(std::numeric_limits<T>::is_integer, "'bitCount' only accept integer values");

		int Count(0);
		for(T i = 0, n = static_cast<T>(sizeof(T) * 8); i < n; ++i)
		{
			Count += static_cast<int>((v >> i) & static_cast<T>(1));
		}
		return Count;
	}

	template<bool EXEC = false>
	struct compute_bitfieldBitCountStep
	{
		template<rblxmath::length_t L, typename T, rblxmath::qualifier Q>
		RBLXMATH_FUNC_QUALIFIER static rblxmath::vec<L, T, Q> call(rblxmath::vec<L, T, Q> const& v, T, T)
		{
			return v;
		}
	};

	template<>
	struct compute_bitfieldBitCountStep<true>
	{
		template<rblxmath::length_t L, typename T, rblxmath::qualifier Q>
		RBLXMATH_FUNC_QUALIFIER static rblxmath::vec<L, T, Q> call(rblxmath::vec<L, T, Q> const& v, T Mask, T Shift)
		{
			return (v & Mask) + ((v >> Shift) & Mask);
		}
	};

#		if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#			pragma warning(push)
#			pragma warning(disable : 4309)
#		endif

	template<rblxmath::length_t L, typename T, rblxmath::qualifier Q>
	static rblxmath::vec<L, int, Q> bitCount_bitfield(rblxmath::vec<L, T, Q> const& v)
	{
		rblxmath::vec<L, typename rblxmath::detail::make_unsigned<T>::type, Q> x(v);
		x = compute_bitfieldBitCountStep<sizeof(T) * 8 >=  2>::call(x, static_cast<typename rblxmath::detail::make_unsigned<T>::type>(0x5555555555555555ull), static_cast<typename rblxmath::detail::make_unsigned<T>::type>( 1));
		x = compute_bitfieldBitCountStep<sizeof(T) * 8 >=  4>::call(x, static_cast<typename rblxmath::detail::make_unsigned<T>::type>(0x3333333333333333ull), static_cast<typename rblxmath::detail::make_unsigned<T>::type>( 2));
		x = compute_bitfieldBitCountStep<sizeof(T) * 8 >=  8>::call(x, static_cast<typename rblxmath::detail::make_unsigned<T>::type>(0x0F0F0F0F0F0F0F0Full), static_cast<typename rblxmath::detail::make_unsigned<T>::type>( 4));
		x = compute_bitfieldBitCountStep<sizeof(T) * 8 >= 16>::call(x, static_cast<typename rblxmath::detail::make_unsigned<T>::type>(0x00FF00FF00FF00FFull), static_cast<typename rblxmath::detail::make_unsigned<T>::type>( 8));
		x = compute_bitfieldBitCountStep<sizeof(T) * 8 >= 32>::call(x, static_cast<typename rblxmath::detail::make_unsigned<T>::type>(0x0000FFFF0000FFFFull), static_cast<typename rblxmath::detail::make_unsigned<T>::type>(16));
		x = compute_bitfieldBitCountStep<sizeof(T) * 8 >= 64>::call(x, static_cast<typename rblxmath::detail::make_unsigned<T>::type>(0x00000000FFFFFFFFull), static_cast<typename rblxmath::detail::make_unsigned<T>::type>(32));
		return rblxmath::vec<L, int, Q>(x);
	}

#		if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#			pragma warning(pop)
#		endif

	template<typename genType>
	static int bitCount_bitfield(genType x)
	{
		return bitCount_bitfield(rblxmath::vec<1, genType, rblxmath::defaultp>(x)).x;
	}

	static int perf(std::size_t Size)
	{
		int Error(0);

		std::vector<int> v;
		v.resize(Size);

		std::vector<rblxmath::ivec4> w;
		w.resize(Size);


		std::clock_t TimestampsA = std::clock();

		// bitCount - TimeIf
		{
			for(std::size_t i = 0, n = v.size(); i < n; ++i)
				v[i] = bitCount_if(static_cast<int>(i));
		}

		std::clock_t TimestampsB = std::clock();

		// bitCount - TimeVec
		{
			for(std::size_t i = 0, n = v.size(); i < n; ++i)
				v[i] = bitCount_vec(i);
		}

		std::clock_t TimestampsC = std::clock();

		// bitCount - TimeDefault
		{
			for(std::size_t i = 0, n = v.size(); i < n; ++i)
				v[i] = rblxmath::bitCount(i);
		}

		std::clock_t TimestampsD = std::clock();

		// bitCount - TimeVec4
		{
			for(std::size_t i = 0, n = v.size(); i < n; ++i)
				w[i] = rblxmath::bitCount(rblxmath::ivec4(static_cast<int>(i)));
		}

		std::clock_t TimestampsE = std::clock();

		{
			for(std::size_t i = 0, n = v.size(); i < n; ++i)
				v[i] = bitCount_bitfield(static_cast<int>(i));
		}

		std::clock_t TimestampsF = std::clock();

		std::printf("bitCount - TimeIf %d\n", static_cast<int>(TimestampsB - TimestampsA));
		std::printf("bitCount - TimeVec %d\n", static_cast<int>(TimestampsC - TimestampsB));
		std::printf("bitCount - TimeDefault %d\n", static_cast<int>(TimestampsD - TimestampsC));
		std::printf("bitCount - TimeVec4 %d\n", static_cast<int>(TimestampsE - TimestampsD));
		std::printf("bitCount - bitfield %d\n", static_cast<int>(TimestampsF - TimestampsE));

		return Error;
	}

	static int test()
	{
		int Error(0);

		for(std::size_t i = 0, n = sizeof(DataI32) / sizeof(type<int>); i < n; ++i)
		{
			int ResultA = rblxmath::bitCount(DataI32[i].Value);
			Error += DataI32[i].Return == ResultA ? 0 : 1;
			assert(!Error);

			int ResultB = bitCount_if(DataI32[i].Value);
			Error += DataI32[i].Return == ResultB ? 0 : 1;
			assert(!Error);

			int ResultC = bitCount_vec(DataI32[i].Value);
			Error += DataI32[i].Return == ResultC ? 0 : 1;
			assert(!Error);

			int ResultE = bitCount_bitfield(DataI32[i].Value);
			Error += DataI32[i].Return == ResultE ? 0 : 1;
			assert(!Error);
		}

		return Error;
	}
}//bitCount

int main()
{
	int Error = 0;

	Error += ::bitCount::test();
	Error += ::bitfieldReverse::test();
	Error += ::findMSB::test();
	Error += ::findLSB::test();
	Error += ::umulExtended::test();
	Error += ::imulExtended::test();
	Error += ::uaddCarry::test();
	Error += ::usubBorrow::test();
	Error += ::bitfieldInsert::test();
	Error += ::bitfieldExtract::test();

#	ifdef NDEBUG
		std::size_t const Samples = 1000;
#	else
		std::size_t const Samples = 1;
#	endif

	::bitCount::perf(Samples);
	::bitfieldReverse::perf(Samples);
	::findMSB::perf(Samples);
	::findLSB::perf(Samples);

	return Error;
}
