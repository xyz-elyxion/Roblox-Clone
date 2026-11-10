#define RBLXMATH_ENABLE_EXPERIMENTAL
#define RBLXMATH_FORCE_INLINE
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/gtc/integer.hpp>
#include <rblxmath/gtc/type_precision.hpp>
#include <rblxmath/gtc/vec1.hpp>
#include <rblxmath/gtx/type_aligned.hpp>
#include <rblxmath/vector_relational.hpp>
#include <rblxmath/vec2.hpp>
#include <rblxmath/vec3.hpp>
#include <rblxmath/vec4.hpp>
#include <ctime>
#include <cstdio>
#include <vector>
#include <cmath>

namespace log2_
{
	static int test()
	{
		int Error = 0;

		int A0 = static_cast<int>(rblxmath::log2(16.f));
		rblxmath::ivec1 B0(rblxmath::log2(rblxmath::vec1(16.f)));
		rblxmath::ivec2 C0(rblxmath::log2(rblxmath::vec2(16.f)));
		rblxmath::ivec3 D0(rblxmath::log2(rblxmath::vec3(16.f)));
		rblxmath::ivec4 E0(rblxmath::log2(rblxmath::vec4(16.f)));

		int A1 = rblxmath::log2(int(16));
		rblxmath::ivec1 B1 = rblxmath::log2(rblxmath::ivec1(16));
		rblxmath::ivec2 C1 = rblxmath::log2(rblxmath::ivec2(16));
		rblxmath::ivec3 D1 = rblxmath::log2(rblxmath::ivec3(16));
		rblxmath::ivec4 E1 = rblxmath::log2(rblxmath::ivec4(16));

		Error += A0 == A1 ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(B0, B1)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(C0, C1)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(D0, D1)) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(E0, E1)) ? 0 : 1;

		rblxmath::uint64 A2 = rblxmath::log2(rblxmath::uint64(16));
		rblxmath::u64vec1 B2 = rblxmath::log2(rblxmath::u64vec1(16));
		rblxmath::u64vec2 C2 = rblxmath::log2(rblxmath::u64vec2(16));
		rblxmath::u64vec3 D2 = rblxmath::log2(rblxmath::u64vec3(16));
		rblxmath::u64vec4 E2 = rblxmath::log2(rblxmath::u64vec4(16));

		Error += A2 == rblxmath::uint64(4) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(B2, rblxmath::u64vec1(4))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(C2, rblxmath::u64vec2(4))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(D2, rblxmath::u64vec3(4))) ? 0 : 1;
		Error += rblxmath::all(rblxmath::equal(E2, rblxmath::u64vec4(4))) ? 0 : 1;

		return Error;
	}

	static int perf(std::size_t Count)
	{
		int Error = 0;

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wsign-conversion"
#endif

		{
			std::vector<int> Result;
			Result.resize(Count);

			std::clock_t Begin = clock();

			for(int i = 0; i < static_cast<int>(Count); ++i)
				Result[i] = rblxmath::log2(static_cast<int>(i));

			std::clock_t End = clock();

			std::printf("rblxmath::log2<int>: %d clocks\n", static_cast<int>(End - Begin));
		}

		{
			std::vector<rblxmath::ivec4> Result;
			Result.resize(Count);

			std::clock_t Begin = clock();

			for(int i = 0; i < static_cast<int>(Count); ++i)
				Result[i] = rblxmath::log2(rblxmath::ivec4(i));

			std::clock_t End = clock();

			std::printf("rblxmath::log2<ivec4>: %d clocks\n", static_cast<int>(End - Begin));
		}

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

#		if RBLXMATH_HAS_BITSCAN_WINDOWS
		{
			std::vector<rblxmath::ivec4> Result;
			Result.resize(Count);

			std::clock_t Begin = clock();

#if RBLXMATH_COMPILER& RBLXMATH_COMPILER_VC
#	pragma warning(push)
#	pragma warning(disable: 4267)
#endif

			for(std::size_t i = 0; i < Count; ++i)
			{
				rblxmath::vec<4, unsigned long, rblxmath::defaultp> Tmp;
				_BitScanReverse(&Tmp.x, i);
				_BitScanReverse(&Tmp.y, i);
				_BitScanReverse(&Tmp.z, i);
				_BitScanReverse(&Tmp.w, i);
				Result[i] = rblxmath::ivec4(Tmp);
			}

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#	pragma warning(pop)
#endif

			std::clock_t End = clock();

			std::printf("rblxmath::log2<ivec4> inlined: %d clocks\n", static_cast<int>(End - Begin));
		}


		{
			std::vector<rblxmath::vec<4, unsigned long, rblxmath::defaultp> > Result;
			Result.resize(Count);

			std::clock_t Begin = clock();

#if RBLXMATH_COMPILER& RBLXMATH_COMPILER_VC
#	pragma warning(push)
#	pragma warning(disable: 4267)
#endif

			for(std::size_t i = 0; i < Count; ++i)
			{
				_BitScanReverse(&Result[i].x, i);
				_BitScanReverse(&Result[i].y, i);
				_BitScanReverse(&Result[i].z, i);
				_BitScanReverse(&Result[i].w, i);
			}

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#	pragma warning(pop)
#endif

			std::clock_t End = clock();

			std::printf("rblxmath::log2<ivec4> inlined no cast: %d clocks\n", static_cast<int>(End - Begin));
		}


		{
			std::vector<rblxmath::ivec4> Result;
			Result.resize(Count);

			std::clock_t Begin = clock();

#if RBLXMATH_COMPILER& RBLXMATH_COMPILER_VC
#	pragma warning(push)
#	pragma warning(disable: 4267)
#endif

			for(std::size_t i = 0; i < Count; ++i)
			{
				_BitScanReverse(reinterpret_cast<unsigned long*>(&Result[i].x), i);
				_BitScanReverse(reinterpret_cast<unsigned long*>(&Result[i].y), i);
				_BitScanReverse(reinterpret_cast<unsigned long*>(&Result[i].z), i);
				_BitScanReverse(reinterpret_cast<unsigned long*>(&Result[i].w), i);
			}

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#	pragma warning(pop)
#endif

			std::clock_t End = clock();

			std::printf("rblxmath::log2<ivec4> reinterpret: %d clocks\n", static_cast<int>(End - Begin));
		}
#		endif//RBLXMATH_HAS_BITSCAN_WINDOWS

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wsign-conversion"
#endif
		{
			std::vector<float> Result;
			Result.resize(Count);

			std::clock_t Begin = clock();

			for(std::size_t i = 0; i < Count; ++i)
				Result[i] = rblxmath::log2(static_cast<float>(i));

			std::clock_t End = clock();

			std::printf("rblxmath::log2<float>: %d clocks\n", static_cast<int>(End - Begin));
		}
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wsign-conversion"
#endif
		{
			std::vector<rblxmath::vec4> Result;
			Result.resize(Count);

			std::clock_t Begin = clock();

			for(int i = 0; i < static_cast<int>(Count); ++i)
				Result[i] = rblxmath::log2(rblxmath::vec4(static_cast<float>(i)));

			std::clock_t End = clock();

			std::printf("rblxmath::log2<vec4>: %d clocks\n", static_cast<int>(End - Begin));
		}
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

		return Error;
	}
}//namespace log2_

namespace iround
{
	static int test()
	{
		int Error = 0;

		for(float f = 0.0f; f < 3.1f; f += 0.05f)
		{
			int RoundFast = static_cast<int>(rblxmath::iround(f));
			int RoundSTD = static_cast<int>(rblxmath::round(f));
			Error += RoundFast == RoundSTD ? 0 : 1;
			assert(!Error);
		}

		return Error;
	}
}//namespace iround

namespace uround
{
	static int test()
	{
		int Error = 0;

		for(float f = 0.0f; f < 3.1f; f += 0.05f)
		{
			int RoundFast = static_cast<int>(rblxmath::uround(f));
			int RoundSTD = static_cast<int>(rblxmath::round(f));
			Error += RoundFast == RoundSTD ? 0 : 1;
			assert(!Error);
		}

		return Error;
	}
}//namespace uround

int main()
{
	int Error(0);

	Error += ::log2_::test();
	Error += ::iround::test();
	Error += ::uround::test();

	std::size_t const Samples(1000);
	Error += ::log2_::perf(Samples);

	return Error;
}
