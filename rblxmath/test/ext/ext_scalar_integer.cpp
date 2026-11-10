#include <rblxmath/ext/scalar_integer.hpp>
#include <rblxmath/ext/scalar_int_sized.hpp>
#include <rblxmath/ext/scalar_uint_sized.hpp>
#include <vector>
#include <ctime>
#include <cstdio>

#if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#include <chrono>

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

		int Error = 0;

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

		int Error = 0;

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

		int Error = 0;

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<int>); i < n; ++i)
		{
			bool Result = rblxmath::isPowerOfTwo(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
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

		int Error = 0;

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<rblxmath::uint>); i < n; ++i)
		{
			bool Result = rblxmath::isPowerOfTwo(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		return Error;
	}

	static int test()
	{
		int Error = 0;

		Error += test_int16();
		Error += test_uint16();
		Error += test_int32();
		Error += test_uint32();

		return Error;
	}
}//isPowerOfTwo

namespace nextPowerOfTwo_advanced
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
	static genType nextPowerOfTwo_loop(genType value)
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
			rblxmath::int32 Result = rblxmath::nextPowerOfTwo(Data[i].Value);
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
			rblxmath::uint32 Result = rblxmath::nextPowerOfTwo(Data[i].Value);
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
			v[i] = nextPowerOfTwo_loop(i);

		std::clock_t Timestramp1 = std::clock();

		for(rblxmath::uint32 i = 0, n = static_cast<rblxmath::uint>(v.size()); i < n; ++i)
			v[i] = rblxmath::nextPowerOfTwo(i);

		std::clock_t Timestramp2 = std::clock();

		std::printf("nextPowerOfTwo_loop: %d clocks\n", static_cast<int>(Timestramp1 - Timestramp0));
		std::printf("rblxmath::nextPowerOfTwo: %d clocks\n", static_cast<int>(Timestramp2 - Timestramp1));

		return Error;
	}

	static int test()
	{
		int Error(0);

		Error += test_int32();
		Error += test_uint32();

		return Error;
	}
}//namespace nextPowerOfTwo_advanced

namespace prevPowerOfTwo
{
	template <typename T>
	static int run()
	{
		int Error = 0;

		T const A = rblxmath::prevPowerOfTwo(static_cast<T>(7));
		Error += A == static_cast<T>(4) ? 0 : 1;

		T const B = rblxmath::prevPowerOfTwo(static_cast<T>(15));
		Error += B == static_cast<T>(8) ? 0 : 1;

		T const C = rblxmath::prevPowerOfTwo(static_cast<T>(31));
		Error += C == static_cast<T>(16) ? 0 : 1;

		T const D = rblxmath::prevPowerOfTwo(static_cast<T>(32));
		Error += D == static_cast<T>(32) ? 0 : 1;

		return Error;
	}

	static int test()
	{
		int Error = 0;

		Error += run<rblxmath::int8>();
		Error += run<rblxmath::int16>();
		Error += run<rblxmath::int32>();
		Error += run<rblxmath::int64>();

		Error += run<rblxmath::uint8>();
		Error += run<rblxmath::uint16>();
		Error += run<rblxmath::uint32>();
		Error += run<rblxmath::uint64>();

		return Error;
	}
}//namespace prevPowerOfTwo

namespace nextPowerOfTwo
{
	template <typename T>
	static int run()
	{
		int Error = 0;

		T const A = rblxmath::nextPowerOfTwo(static_cast<T>(7));
		Error += A == static_cast<T>(8) ? 0 : 1;

		T const B = rblxmath::nextPowerOfTwo(static_cast<T>(15));
		Error += B == static_cast<T>(16) ? 0 : 1;

		T const C = rblxmath::nextPowerOfTwo(static_cast<T>(31));
		Error += C == static_cast<T>(32) ? 0 : 1;

		T const D = rblxmath::nextPowerOfTwo(static_cast<T>(32));
		Error += D == static_cast<T>(32) ? 0 : 1;

		return Error;
	}

	static int test()
	{
		int Error = 0;

		Error += run<rblxmath::int8>();
		Error += run<rblxmath::int16>();
		Error += run<rblxmath::int32>();
		Error += run<rblxmath::int64>();

		Error += run<rblxmath::uint8>();
		Error += run<rblxmath::uint16>();
		Error += run<rblxmath::uint32>();
		Error += run<rblxmath::uint64>();

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

	template <typename T>
	static int run()
	{
		type<T> const Data[] =
		{
			{8, 3, 6},
			{7, 7, 7}
		};

		int Error = 0;
		
		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<T>); i < n; ++i)
		{
			T const Result = rblxmath::prevMultiple(Data[i].Source, Data[i].Multiple);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		return Error;
	}

	static int test()
	{
		int Error = 0;

		Error += run<rblxmath::int8>();
		Error += run<rblxmath::int16>();
		Error += run<rblxmath::int32>();
		Error += run<rblxmath::int64>();

		Error += run<rblxmath::uint8>();
		Error += run<rblxmath::uint16>();
		Error += run<rblxmath::uint32>();
		Error += run<rblxmath::uint64>();

		return Error;
	}
}//namespace prevMultiple

namespace nextMultiple
{
	static rblxmath::uint const Multiples = 128;

	static int perf_nextMultiple(rblxmath::uint Samples)
	{
		std::vector<rblxmath::uint> Results(Samples * Multiples);

		std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

		for(rblxmath::uint Source = 0; Source < Samples; ++Source)
		for(rblxmath::uint Multiple = 0; Multiple < Multiples; ++Multiple)
		{
			Results[Source * Multiples + Multiple] = rblxmath::nextMultiple(Source, Multiples);
		}

		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

		std::printf("- rblxmath::nextMultiple Time %d microseconds\n", static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count()));

		rblxmath::uint Result = 0;
		for(std::size_t i = 0, n = Results.size(); i < n; ++i)
			Result += Results[i];

		return Result > 0 ? 0 : 1;
	}

	template <typename T>
	RBLXMATH_FUNC_QUALIFIER
	static T nextMultipleMod(T Source, T Multiple)
	{
		T const Tmp = Source - static_cast<T>(1);
		return Tmp + (Multiple - (Tmp % Multiple));
	}

	static int perf_nextMultipleMod(rblxmath::uint Samples)
	{
		std::vector<rblxmath::uint> Results(Samples * Multiples);

		std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

		for(rblxmath::uint Multiple = 0; Multiple < Multiples; ++Multiple)
			for (rblxmath::uint Source = 0; Source < Samples; ++Source)
		{
			Results[Source * Multiples + Multiple] = nextMultipleMod(Source, Multiples);
		}

		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

		std::printf("- nextMultipleMod Time %d microseconds\n", static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count()));

		rblxmath::uint Result = 0;
		for(std::size_t i = 0, n = Results.size(); i < n; ++i)
			Result += Results[i];

		return Result > 0 ? 0 : 1;
	}

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#	pragma warning(push)
#	pragma warning(disable : 4146)
#endif
	template <typename T>
	RBLXMATH_FUNC_QUALIFIER
	static T nextMultipleNeg(T Source, T Multiple)
	{
		if(Source > static_cast<T>(0))
		{
			T const Tmp = Source - static_cast<T>(1);
			return Tmp + (Multiple - (Tmp % Multiple));
		}
		else
			return Source + (-Source % Multiple);
	}
#if(RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC)
#	pragma warning(pop)
#endif

	static int perf_nextMultipleNeg(rblxmath::uint Samples)
	{
		std::vector<rblxmath::uint> Results(Samples * Multiples);

		std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

		for(rblxmath::uint Source = 0; Source < Samples; ++Source)
		for(rblxmath::uint Multiple = 0; Multiple < Multiples; ++Multiple)
		{
			Results[Source * Multiples + Multiple] = nextMultipleNeg(Source, Multiples);
		}

		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

		std::printf("- nextMultipleNeg Time %d microseconds\n", static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count()));

		rblxmath::uint Result = 0;
		for (std::size_t i = 0, n = Results.size(); i < n; ++i)
			Result += Results[i];

		return Result > 0 ? 0 : 1;
	}

	template <typename T>
	RBLXMATH_FUNC_QUALIFIER
	static T nextMultipleUFloat(T Source, T Multiple)
	{
		return Source + (Multiple - std::fmod(Source, Multiple));
	}

	static int perf_nextMultipleUFloat(rblxmath::uint Samples)
	{
		std::vector<float> Results(Samples * Multiples);

		std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

		for(rblxmath::uint Source = 0; Source < Samples; ++Source)
		for(rblxmath::uint Multiple = 0; Multiple < Multiples; ++Multiple)
		{
			Results[Source * Multiples + Multiple] = nextMultipleUFloat(static_cast<float>(Source), static_cast<float>(Multiples));
		}

		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

		std::printf("- nextMultipleUFloat Time %d microseconds\n", static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count()));

		float Result = 0;
		for (std::size_t i = 0, n = Results.size(); i < n; ++i)
			Result += Results[i];

		return Result > 0.0f ? 0 : 1;
	}

	template <typename T>
	RBLXMATH_FUNC_QUALIFIER
	static T nextMultipleFloat(T Source, T Multiple)
	{
		if(Source > static_cast<float>(0))
			return Source + (Multiple - std::fmod(Source, Multiple));
		else
			return Source + std::fmod(-Source, Multiple);
	}

	static int perf_nextMultipleFloat(rblxmath::uint Samples)
	{
		std::vector<float> Results(Samples * Multiples);

		std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

		for(rblxmath::uint Source = 0; Source < Samples; ++Source)
		for(rblxmath::uint Multiple = 0; Multiple < Multiples; ++Multiple)
		{
			Results[Source * Multiples + Multiple] = nextMultipleFloat(static_cast<float>(Source), static_cast<float>(Multiples));
		}

		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

		std::printf("- nextMultipleFloat Time %d microseconds\n", static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count()));

		float Result = 0;
		for (std::size_t i = 0, n = Results.size(); i < n; ++i)
			Result += Results[i];

		return Result > 0.0f ? 0 : 1;
	}

	template<typename genIUType>
	struct type
	{
		genIUType Source;
		genIUType Multiple;
		genIUType Return;
	};

	template <typename T>
	static int test_uint()
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

		for(std::size_t i = 0, n = sizeof(Data) / sizeof(type<T>); i < n; ++i)
		{
			T const Result0 = rblxmath::nextMultiple(Data[i].Source, Data[i].Multiple);
			Error += Data[i].Return == Result0 ? 0 : 1;
			assert(!Error);

			T const Result1 = nextMultipleMod(Data[i].Source, Data[i].Multiple);
			Error += Data[i].Return == Result1 ? 0 : 1;
			assert(!Error);
		}

		return Error;
	}

	static int perf()
	{
		int Error = 0;

		rblxmath::uint const Samples = 100;

		for(int i = 0; i < 4; ++i)
		{
			std::printf("Run %d :\n", i);
			Error += perf_nextMultiple(Samples);
			Error += perf_nextMultipleMod(Samples);
			Error += perf_nextMultipleNeg(Samples);
			Error += perf_nextMultipleUFloat(Samples);
			Error += perf_nextMultipleFloat(Samples);
			std::printf("\n");
		}

		return Error;
	}

	static int test()
	{
		int Error = 0;

		Error += test_uint<rblxmath::int8>();
		Error += test_uint<rblxmath::int16>();
		Error += test_uint<rblxmath::int32>();
		Error += test_uint<rblxmath::int64>();

		Error += test_uint<rblxmath::uint8>();
		Error += test_uint<rblxmath::uint16>();
		Error += test_uint<rblxmath::uint32>();
		Error += test_uint<rblxmath::uint64>();

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

	template <typename T>
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
			int const Result0 = rblxmath::findNSB(Data[i].Source, Data[i].SignificantBitCount);
			Error += Data[i].Return == Result0 ? 0 : 1;
			assert(!Error);
		}

		return Error;
	}

	static int test()
	{
		int Error = 0;

		Error += run<rblxmath::uint8>();
		Error += run<rblxmath::uint16>();
		Error += run<rblxmath::uint32>();
		Error += run<rblxmath::uint64>();

		Error += run<rblxmath::int8>();
		Error += run<rblxmath::int16>();
		Error += run<rblxmath::int32>();
		Error += run<rblxmath::int64>();

		return Error;
	}
}//namespace findNSB

int main()
{
	int Error = 0;

	Error += findNSB::test();

	Error += isPowerOfTwo::test();
	Error += prevPowerOfTwo::test();
	Error += nextPowerOfTwo::test();
	Error += nextPowerOfTwo_advanced::test();
	Error += prevMultiple::test();
	Error += nextMultiple::test();

	Error += nextPowerOfTwo_advanced::perf();
	Error += nextMultiple::perf();

	return Error;
}

#else

int main()
{
	return 0;
}

#endif
