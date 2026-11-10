#define RBLXMATH_FORCE_EXPLICIT_CTOR
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/gtc/random.hpp>
#include <rblxmath/gtc/vec1.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/vector_float1.hpp>
#include <rblxmath/common.hpp>
#include <rblxmath/vec4.hpp>
#include <rblxmath/vec3.hpp>
#include <rblxmath/vec2.hpp>
#include <vector>
#include <cstdio>
#include <cmath>
#include <ctime>

// This file has divisions by zero to test isnan
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#	pragma warning(push)
#	pragma warning(disable : 4723)
#endif

namespace floor_
{
	static int test()
	{
		int Error = 0;

		{
			float A = 1.1f;
			float B = rblxmath::floor(A);
			Error += rblxmath::equal(B, 1.f, 0.0001f) ? 0 : 1;
		}

		{
			double A = 1.1;
			double B = rblxmath::floor(A);
			Error += rblxmath::equal(B, 1.0, 0.0001) ? 0 : 1;
		}

		{
			rblxmath::vec1 A(1.1f);
			rblxmath::vec1 B = rblxmath::floor(A);

			Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec1(1.0), 0.0001f)) ? 0 : 1;
		}

		{
			rblxmath::dvec1 A(1.1);
			rblxmath::dvec1 B = rblxmath::floor(A);

			Error += rblxmath::all(rblxmath::equal(B, rblxmath::dvec1(1.0), 0.0001)) ? 0 : 1;
		}

		{
			rblxmath::vec2 A(1.1f);
			rblxmath::vec2 B = rblxmath::floor(A);

			Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec2(1.0), 0.0001f)) ? 0 : 1;
		}

		{
			rblxmath::dvec2 A(1.1);
			rblxmath::dvec2 B = rblxmath::floor(A);

			Error += rblxmath::all(rblxmath::equal(B, rblxmath::dvec2(1.0), 0.0001)) ? 0 : 1;
		}

		{
			rblxmath::vec3 A(1.1f);
			rblxmath::vec3 B = rblxmath::floor(A);

			Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec3(1.0), 0.0001f)) ? 0 : 1;
		}

		{
			rblxmath::dvec3 A(1.1);
			rblxmath::dvec3 B = rblxmath::floor(A);

			Error += rblxmath::all(rblxmath::equal(B, rblxmath::dvec3(1.0), 0.0001)) ? 0 : 1;
		}

		{
			rblxmath::vec4 A(1.1f);
			rblxmath::vec4 B = rblxmath::floor(A);

			Error += rblxmath::all(rblxmath::equal(B, rblxmath::vec4(1.0), 0.0001f)) ? 0 : 1;
		}

		{
			rblxmath::dvec4 A(1.1);
			rblxmath::dvec4 B = rblxmath::floor(A);

			Error += rblxmath::all(rblxmath::equal(B, rblxmath::dvec4(1.0), 0.0001)) ? 0 : 1;
		}

		return Error;
	}
}//namespace floor

namespace modf_
{
	static int test()
	{
		int Error(0);

		{
			float X(1.5f);
			float I(0.0f);
			float A = rblxmath::modf(X, I);

			Error += rblxmath::equal(I, 1.0f, 0.0001f) ? 0 : 1;
			Error += rblxmath::equal(A, 0.5f, 0.0001f) ? 0 : 1;
		}

		{
			rblxmath::vec4 X(1.1f, 1.2f, 1.5f, 1.7f);
			rblxmath::vec4 I(0.0f);
			rblxmath::vec4 A = rblxmath::modf(X, I);

			Error += rblxmath::ivec4(I) == rblxmath::ivec4(1) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec4(0.1f, 0.2f, 0.5f, 0.7f), 0.00001f)) ? 0 : 1;
		}

		{
			rblxmath::dvec4 X(1.1, 1.2, 1.5, 1.7);
			rblxmath::dvec4 I(0.0);
			rblxmath::dvec4 A = rblxmath::modf(X, I);

			Error += rblxmath::ivec4(I) == rblxmath::ivec4(1) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(A, rblxmath::dvec4(0.1, 0.2, 0.5, 0.7), 0.000000001)) ? 0 : 1;
		}

		{
			double X(1.5);
			double I(0.0);
			double A = rblxmath::modf(X, I);

			Error += rblxmath::equal(I, 1.0, 0.0001) ? 0 : 1;
			Error += rblxmath::equal(A, 0.5, 0.0001) ? 0 : 1;
		}

		return Error;
	}
}//namespace modf

namespace mod_
{
	static int test()
	{
		int Error(0);

		{
			float A(1.5f);
			float B(1.0f);
			float C = rblxmath::mod(A, B);

			Error += rblxmath::equal(C, 0.5f, 0.00001f) ? 0 : 1;
		}

		{
			float A(-0.2f);
			float B(1.0f);
			float C = rblxmath::mod(A, B);

			Error += rblxmath::equal(C, 0.8f, 0.00001f) ? 0 : 1;
		}

		{
			float A(3.0);
			float B(2.0f);
			float C = rblxmath::mod(A, B);

			Error += rblxmath::equal(C, 1.0f, 0.00001f) ? 0 : 1;
		}

		{
			rblxmath::vec4 A(3.0);
			float B(2.0f);
			rblxmath::vec4 C = rblxmath::mod(A, B);

			Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec4(1.0f), 0.00001f)) ? 0 : 1;
		}

		{
			rblxmath::vec4 A(3.0);
			rblxmath::vec4 B(2.0f);
			rblxmath::vec4 C = rblxmath::mod(A, B);

			Error += rblxmath::all(rblxmath::equal(C, rblxmath::vec4(1.0f), 0.00001f)) ? 0 : 1;
		}

		return Error;
	}
}//namespace mod_

namespace floatBitsToInt
{
	static int test()
	{
		int Error = 0;
	
		{
			float A = 1.0f;
			int B = rblxmath::floatBitsToInt(A);
			float C = rblxmath::intBitsToFloat(B);
			Error += rblxmath::equal(A, C, 0.0001f) ? 0 : 1;
		}

		{
			rblxmath::vec2 A(1.0f, 2.0f);
			rblxmath::ivec2 B = rblxmath::floatBitsToInt(A);
			rblxmath::vec2 C = rblxmath::intBitsToFloat(B);
			Error += rblxmath::all(rblxmath::equal(A, C, 0.0001f)) ? 0 : 1;
		}

		{
			rblxmath::vec3 A(1.0f, 2.0f, 3.0f);
			rblxmath::ivec3 B = rblxmath::floatBitsToInt(A);
			rblxmath::vec3 C = rblxmath::intBitsToFloat(B);
			Error += rblxmath::all(rblxmath::equal(A, C, 0.0001f)) ? 0 : 1;
		}
	
		{
			rblxmath::vec4 A(1.0f, 2.0f, 3.0f, 4.0f);
			rblxmath::ivec4 B = rblxmath::floatBitsToInt(A);
			rblxmath::vec4 C = rblxmath::intBitsToFloat(B);
			Error += rblxmath::all(rblxmath::equal(A, C, 0.0001f)) ? 0 : 1;
		}
	
		return Error;
	}
}//namespace floatBitsToInt

namespace floatBitsToUint
{
	static int test()
	{
		int Error = 0;
	
		{
			float A = 1.0f;
			rblxmath::uint B = rblxmath::floatBitsToUint(A);
			float C = rblxmath::uintBitsToFloat(B);
			Error += rblxmath::equal(A, C, 0.0001f) ? 0 : 1;
		}
	
		{
			rblxmath::vec2 A(1.0f, 2.0f);
			rblxmath::uvec2 B = rblxmath::floatBitsToUint(A);
			rblxmath::vec2 C = rblxmath::uintBitsToFloat(B);
			Error += rblxmath::all(rblxmath::equal(A, C, 0.0001f)) ? 0 : 1;
		}
	
		{
			rblxmath::vec3 A(1.0f, 2.0f, 3.0f);
			rblxmath::uvec3 B = rblxmath::floatBitsToUint(A);
			rblxmath::vec3 C = rblxmath::uintBitsToFloat(B);
			Error += rblxmath::all(rblxmath::equal(A, C, 0.0001f)) ? 0 : 1;
		}
	
		{
			rblxmath::vec4 A(1.0f, 2.0f, 3.0f, 4.0f);
			rblxmath::uvec4 B = rblxmath::floatBitsToUint(A);
			rblxmath::vec4 C = rblxmath::uintBitsToFloat(B);
			Error += rblxmath::all(rblxmath::equal(A, C, 0.0001f)) ? 0 : 1;
		}
	
		return Error;
	}
}//namespace floatBitsToUint

namespace min_
{
	static int test()
	{
		int Error = 0;

		rblxmath::vec1 A0 = rblxmath::min(rblxmath::vec1(1), rblxmath::vec1(1));
		bool A1 = rblxmath::all(rblxmath::equal(A0, rblxmath::vec1(1), rblxmath::epsilon<float>()));
		Error += A1 ? 0 : 1;

		rblxmath::vec2 B0 = rblxmath::min(rblxmath::vec2(1), rblxmath::vec2(1));
		rblxmath::vec2 B1 = rblxmath::min(rblxmath::vec2(1), 1.0f);
		bool B2 = rblxmath::all(rblxmath::equal(B0, B1, rblxmath::epsilon<float>()));
		Error += B2 ? 0 : 1;

		rblxmath::vec3 C0 = rblxmath::min(rblxmath::vec3(1), rblxmath::vec3(1));
		rblxmath::vec3 C1 = rblxmath::min(rblxmath::vec3(1), 1.0f);
		bool C2 = rblxmath::all(rblxmath::equal(C0, C1, rblxmath::epsilon<float>()));
		Error += C2 ? 0 : 1;

		rblxmath::vec4 D0 = rblxmath::min(rblxmath::vec4(1), rblxmath::vec4(1));
		rblxmath::vec4 D1 = rblxmath::min(rblxmath::vec4(1), 1.0f);
		bool D2 = rblxmath::all(rblxmath::equal(D0, D1, rblxmath::epsilon<float>()));
		Error += D2 ? 0 : 1;

		return Error;
	}

	static int min_tern(int a, int b)
	{
		return a < b ? a : b;
	}

	static int min_int(int x, int y)
	{
		return y ^ ((x ^ y) & -(x < y)); 
	}

	static int perf(std::size_t Count)
	{
		std::vector<int> A(Count);
		std::vector<int> B(Count);

		std::size_t const InternalCount = 200000;

		for(std::size_t i = 0; i < Count; ++i)
		{
			A[i] = rblxmath::linearRand(-1000, 1000);
			B[i] = rblxmath::linearRand(-1000, 1000);
		}

		int Error = 0;

		rblxmath::int32 SumA = 0;
		{
			std::clock_t Timestamp0 = std::clock();

			for (std::size_t j = 0; j < InternalCount; ++j)
			for (std::size_t i = 0; i < Count; ++i)
				SumA += min_tern(A[i], B[i]);

			std::clock_t Timestamp1 = std::clock();

			std::printf("min_tern Time %d clocks\n", static_cast<int>(Timestamp1 - Timestamp0));
		}

		rblxmath::int32 SumB = 0;
		{
			std::clock_t Timestamp0 = std::clock();

			for (std::size_t j = 0; j < InternalCount; ++j)
			for (std::size_t i = 0; i < Count; ++i)
				SumB += min_int(A[i], B[i]);

			std::clock_t Timestamp1 = std::clock();

			std::printf("min_int Time %d clocks\n", static_cast<int>(Timestamp1 - Timestamp0));
		}

		Error += SumA == SumB ? 0 : 1;

		return Error;
	}
}//namespace min_

namespace max_
{
	static int test()
	{
		int Error = 0;

		rblxmath::vec1 A0 = rblxmath::max(rblxmath::vec1(1), rblxmath::vec1(1));
		bool A1 = rblxmath::all(rblxmath::equal(A0, rblxmath::vec1(1), rblxmath::epsilon<float>()));
		Error += A1 ? 0 : 1;


		rblxmath::vec2 B0 = rblxmath::max(rblxmath::vec2(1), rblxmath::vec2(1));
		rblxmath::vec2 B1 = rblxmath::max(rblxmath::vec2(1), 1.0f);
		bool B2 = rblxmath::all(rblxmath::equal(B0, B1, rblxmath::epsilon<float>()));
		Error += B2 ? 0 : 1;

		rblxmath::vec3 C0 = rblxmath::max(rblxmath::vec3(1), rblxmath::vec3(1));
		rblxmath::vec3 C1 = rblxmath::max(rblxmath::vec3(1), 1.0f);
		bool C2 = rblxmath::all(rblxmath::equal(C0, C1, rblxmath::epsilon<float>()));
		Error += C2 ? 0 : 1;

		rblxmath::vec4 D0 = rblxmath::max(rblxmath::vec4(1), rblxmath::vec4(1));
		rblxmath::vec4 D1 = rblxmath::max(rblxmath::vec4(1), 1.0f);
		bool D2 = rblxmath::all(rblxmath::equal(D0, D1, rblxmath::epsilon<float>()));
		Error += D2 ? 0 : 1;

		return Error;
	}
}//namespace max_

namespace clamp_
{
	static int test()
	{
		int Error = 0;

		return Error;
	}
}//namespace clamp_

namespace mix_
{
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wpadded"
#endif

	template<typename T, typename B>
	struct entry
	{
		T x;
		T y;
		B a;
		T Result;
	};

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wglobal-constructors"
#endif

	static entry<float, bool> const TestBool[] =
	{
		{0.0f, 1.0f, false, 0.0f},
		{0.0f, 1.0f, true, 1.0f},
		{-1.0f, 1.0f, false, -1.0f},
		{-1.0f, 1.0f, true, 1.0f}
	};

	static entry<float, float> const TestFloat[] =
	{
		{0.0f, 1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 1.0f, 1.0f},
		{-1.0f, 1.0f, 0.0f, -1.0f},
		{-1.0f, 1.0f, 1.0f, 1.0f}
	};

	static entry<rblxmath::vec2, bool> const TestVec2Bool[] =
	{
		{rblxmath::vec2(0.0f), rblxmath::vec2(1.0f), false, rblxmath::vec2(0.0f)},
		{rblxmath::vec2(0.0f), rblxmath::vec2(1.0f), true, rblxmath::vec2(1.0f)},
		{rblxmath::vec2(-1.0f), rblxmath::vec2(1.0f), false, rblxmath::vec2(-1.0f)},
		{rblxmath::vec2(-1.0f), rblxmath::vec2(1.0f), true, rblxmath::vec2(1.0f)}
	};

	static entry<rblxmath::vec2, rblxmath::bvec2> const TestBVec2[] =
	{
		{rblxmath::vec2(0.0f), rblxmath::vec2(1.0f), rblxmath::bvec2(false), rblxmath::vec2(0.0f)},
		{rblxmath::vec2(0.0f), rblxmath::vec2(1.0f), rblxmath::bvec2(true), rblxmath::vec2(1.0f)},
		{rblxmath::vec2(-1.0f), rblxmath::vec2(1.0f), rblxmath::bvec2(false), rblxmath::vec2(-1.0f)},
		{rblxmath::vec2(-1.0f), rblxmath::vec2(1.0f), rblxmath::bvec2(true), rblxmath::vec2(1.0f)},
		{rblxmath::vec2(-1.0f), rblxmath::vec2(1.0f), rblxmath::bvec2(true, false), rblxmath::vec2(1.0f, -1.0f)}
	};

	static entry<rblxmath::vec3, bool> const TestVec3Bool[] =
	{
		{rblxmath::vec3(0.0f), rblxmath::vec3(1.0f), false, rblxmath::vec3(0.0f)},
		{rblxmath::vec3(0.0f), rblxmath::vec3(1.0f), true, rblxmath::vec3(1.0f)},
		{rblxmath::vec3(-1.0f), rblxmath::vec3(1.0f), false, rblxmath::vec3(-1.0f)},
		{rblxmath::vec3(-1.0f), rblxmath::vec3(1.0f), true, rblxmath::vec3(1.0f)}
	};

	static entry<rblxmath::vec3, rblxmath::bvec3> const TestBVec3[] =
	{
		{rblxmath::vec3(0.0f), rblxmath::vec3(1.0f), rblxmath::bvec3(false), rblxmath::vec3(0.0f)},
		{rblxmath::vec3(0.0f), rblxmath::vec3(1.0f), rblxmath::bvec3(true), rblxmath::vec3(1.0f)},
		{rblxmath::vec3(-1.0f), rblxmath::vec3(1.0f), rblxmath::bvec3(false), rblxmath::vec3(-1.0f)},
		{rblxmath::vec3(-1.0f), rblxmath::vec3(1.0f), rblxmath::bvec3(true), rblxmath::vec3(1.0f)},
		{rblxmath::vec3(1.0f, 2.0f, 3.0f), rblxmath::vec3(4.0f, 5.0f, 6.0f), rblxmath::bvec3(true, false, true), rblxmath::vec3(4.0f, 2.0f, 6.0f)}
	};

	static entry<rblxmath::vec4, bool> const TestVec4Bool[] =
	{
		{rblxmath::vec4(0.0f), rblxmath::vec4(1.0f), false, rblxmath::vec4(0.0f)},
		{rblxmath::vec4(0.0f), rblxmath::vec4(1.0f), true, rblxmath::vec4(1.0f)},
		{rblxmath::vec4(-1.0f), rblxmath::vec4(1.0f), false, rblxmath::vec4(-1.0f)},
		{rblxmath::vec4(-1.0f), rblxmath::vec4(1.0f), true, rblxmath::vec4(1.0f)}
	};

	static entry<rblxmath::vec4, rblxmath::bvec4> const TestBVec4[] =
	{
		{rblxmath::vec4(0.0f, 0.0f, 1.0f, 1.0f), rblxmath::vec4(2.0f, 2.0f, 3.0f, 3.0f), rblxmath::bvec4(false, true, false, true), rblxmath::vec4(0.0f, 2.0f, 1.0f, 3.0f)},
		{rblxmath::vec4(0.0f), rblxmath::vec4(1.0f), rblxmath::bvec4(true), rblxmath::vec4(1.0f)},
		{rblxmath::vec4(-1.0f), rblxmath::vec4(1.0f), rblxmath::bvec4(false), rblxmath::vec4(-1.0f)},
		{rblxmath::vec4(-1.0f), rblxmath::vec4(1.0f), rblxmath::bvec4(true), rblxmath::vec4(1.0f)},
		{rblxmath::vec4(1.0f, 2.0f, 3.0f, 4.0f), rblxmath::vec4(5.0f, 6.0f, 7.0f, 8.0f), rblxmath::bvec4(true, false, true, false), rblxmath::vec4(5.0f, 2.0f, 7.0f, 4.0f)}
	};

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

	static int test()
	{
		int Error = 0;

		// Float with bool
		{
			for(std::size_t i = 0; i < sizeof(TestBool) / sizeof(entry<float, bool>); ++i)
			{
				float Result = rblxmath::mix(TestBool[i].x, TestBool[i].y, TestBool[i].a);
				Error += rblxmath::equal(Result, TestBool[i].Result, rblxmath::epsilon<float>()) ? 0 : 1;
			}
		}

		// Float with float
		{
			for(std::size_t i = 0; i < sizeof(TestFloat) / sizeof(entry<float, float>); ++i)
			{
				float Result = rblxmath::mix(TestFloat[i].x, TestFloat[i].y, TestFloat[i].a);
				Error += rblxmath::equal(Result, TestFloat[i].Result, rblxmath::epsilon<float>()) ? 0 : 1;
			}
		}

		// vec2 with bool
		{
			for(std::size_t i = 0; i < sizeof(TestVec2Bool) / sizeof(entry<rblxmath::vec2, bool>); ++i)
			{
				rblxmath::vec2 Result = rblxmath::mix(TestVec2Bool[i].x, TestVec2Bool[i].y, TestVec2Bool[i].a);
				Error += rblxmath::equal(Result.x, TestVec2Bool[i].Result.x, rblxmath::epsilon<float>()) ? 0 : 1;
				Error += rblxmath::equal(Result.y, TestVec2Bool[i].Result.y, rblxmath::epsilon<float>()) ? 0 : 1;
			}
		}

		// vec2 with bvec2
		{
			for(std::size_t i = 0; i < sizeof(TestBVec2) / sizeof(entry<rblxmath::vec2, rblxmath::bvec2>); ++i)
			{
				rblxmath::vec2 Result = rblxmath::mix(TestBVec2[i].x, TestBVec2[i].y, TestBVec2[i].a);
				Error += rblxmath::equal(Result.x, TestBVec2[i].Result.x, rblxmath::epsilon<float>()) ? 0 : 1;
				Error += rblxmath::equal(Result.y, TestBVec2[i].Result.y, rblxmath::epsilon<float>()) ? 0 : 1;
			}
		}

		// vec3 with bool
		{
			for(std::size_t i = 0; i < sizeof(TestVec3Bool) / sizeof(entry<rblxmath::vec3, bool>); ++i)
			{
				rblxmath::vec3 Result = rblxmath::mix(TestVec3Bool[i].x, TestVec3Bool[i].y, TestVec3Bool[i].a);
				Error += rblxmath::equal(Result.x, TestVec3Bool[i].Result.x, rblxmath::epsilon<float>()) ? 0 : 1;
				Error += rblxmath::equal(Result.y, TestVec3Bool[i].Result.y, rblxmath::epsilon<float>()) ? 0 : 1;
				Error += rblxmath::equal(Result.z, TestVec3Bool[i].Result.z, rblxmath::epsilon<float>()) ? 0 : 1;
			}
		}

		// vec3 with bvec3
		{
			for(std::size_t i = 0; i < sizeof(TestBVec3) / sizeof(entry<rblxmath::vec3, rblxmath::bvec3>); ++i)
			{
				rblxmath::vec3 Result = rblxmath::mix(TestBVec3[i].x, TestBVec3[i].y, TestBVec3[i].a);
				Error += rblxmath::equal(Result.x, TestBVec3[i].Result.x, rblxmath::epsilon<float>()) ? 0 : 1;
				Error += rblxmath::equal(Result.y, TestBVec3[i].Result.y, rblxmath::epsilon<float>()) ? 0 : 1;
				Error += rblxmath::equal(Result.z, TestBVec3[i].Result.z, rblxmath::epsilon<float>()) ? 0 : 1;
			}
		}

		// vec4 with bool
		{
			for(std::size_t i = 0; i < sizeof(TestVec4Bool) / sizeof(entry<rblxmath::vec4, bool>); ++i)
			{
				rblxmath::vec4 Result = rblxmath::mix(TestVec4Bool[i].x, TestVec4Bool[i].y, TestVec4Bool[i].a);
				Error += rblxmath::equal(Result.x, TestVec4Bool[i].Result.x, rblxmath::epsilon<float>()) ? 0 : 1;
				Error += rblxmath::equal(Result.y, TestVec4Bool[i].Result.y, rblxmath::epsilon<float>()) ? 0 : 1;
				Error += rblxmath::equal(Result.z, TestVec4Bool[i].Result.z, rblxmath::epsilon<float>()) ? 0 : 1;
				Error += rblxmath::equal(Result.w, TestVec4Bool[i].Result.w, rblxmath::epsilon<float>()) ? 0 : 1;
			}
		}

		// vec4 with bvec4
		{
			for(std::size_t i = 0; i < sizeof(TestBVec4) / sizeof(entry<rblxmath::vec4, rblxmath::bvec4>); ++i)
			{
				rblxmath::vec4 Result = rblxmath::mix(TestBVec4[i].x, TestBVec4[i].y, TestBVec4[i].a);
				Error += rblxmath::equal(Result.x, TestBVec4[i].Result.x, rblxmath::epsilon<float>()) ? 0 : 1;
				Error += rblxmath::equal(Result.y, TestBVec4[i].Result.y, rblxmath::epsilon<float>()) ? 0 : 1;
				Error += rblxmath::equal(Result.z, TestBVec4[i].Result.z, rblxmath::epsilon<float>()) ? 0 : 1;
				Error += rblxmath::equal(Result.w, TestBVec4[i].Result.w, rblxmath::epsilon<float>()) ? 0 : 1;
			}
		}

		return Error;
	}
}//namespace mix_

namespace step_
{
	template<typename EDGE, typename VEC>
	struct entry
	{
		EDGE edge;
		VEC x;
		VEC result;
	};

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wglobal-constructors"
#endif

	static const entry<float, rblxmath::vec4> TestVec4Scalar [] =
	{
		{ 1.0f, rblxmath::vec4(1.0f, 2.0f, 3.0f, 4.0f), rblxmath::vec4(1.0f) },
		{ 0.0f, rblxmath::vec4(1.0f, 2.0f, 3.0f, 4.0f), rblxmath::vec4(1.0f) },
		{ 0.0f, rblxmath::vec4(-1.0f, -2.0f, -3.0f, -4.0f), rblxmath::vec4(0.0f) }
	};

	static const entry<rblxmath::vec4, rblxmath::vec4> TestVec4Vector [] =
	{
		{ rblxmath::vec4(-1.0f, -2.0f, -3.0f, -4.0f), rblxmath::vec4(-2.0f, -3.0f, -4.0f, -5.0f), rblxmath::vec4(0.0f) },
		{ rblxmath::vec4( 0.0f, 1.0f, 2.0f, 3.0f), rblxmath::vec4( 1.0f, 2.0f, 3.0f, 4.0f), rblxmath::vec4(1.0f) },
		{ rblxmath::vec4( 2.0f, 3.0f, 4.0f, 5.0f), rblxmath::vec4( 1.0f, 2.0f, 3.0f, 4.0f), rblxmath::vec4(0.0f) },
		{ rblxmath::vec4( 0.0f, 1.0f, 2.0f, 3.0f), rblxmath::vec4(-1.0f,-2.0f,-3.0f,-4.0f), rblxmath::vec4(0.0f) }
	};

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

	static int test()
	{
		int Error = 0;

		// scalar
		{
			float const Edge = 2.0f;

			float const A = rblxmath::step(Edge, 1.0f);
			Error += rblxmath::equal(A, 0.0f, rblxmath::epsilon<float>()) ? 0 : 1;

			float const B = rblxmath::step(Edge, 3.0f);
			Error += rblxmath::equal(B, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;

			float const C = rblxmath::step(Edge, 2.0f);
			Error += rblxmath::equal(C, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
		}

		// vec4 and float
		{
			for (std::size_t i = 0; i < sizeof(TestVec4Scalar) / sizeof(entry<float, rblxmath::vec4>); ++i)
			{
				rblxmath::vec4 Result = rblxmath::step(TestVec4Scalar[i].edge, TestVec4Scalar[i].x);
				Error += rblxmath::all(rblxmath::equal(Result, TestVec4Scalar[i].result, rblxmath::epsilon<float>())) ? 0 : 1;
			}
		}

		// vec4 and vec4
		{
			for (std::size_t i = 0; i < sizeof(TestVec4Vector) / sizeof(entry<rblxmath::vec4, rblxmath::vec4>); ++i)
			{
				rblxmath::vec4 Result = rblxmath::step(TestVec4Vector[i].edge, TestVec4Vector[i].x);
				Error += rblxmath::all(rblxmath::equal(Result, TestVec4Vector[i].result, rblxmath::epsilon<float>())) ? 0 : 1;
			}
		}

		return Error;
	}
}//namespace step_

namespace smoothstep_
{
	static int test()
	{
		int Error = 0;

		float const Edge = 2.0f;

		// scalar
		{
			float const A = rblxmath::smoothstep(0.0f, Edge, 1.0f);
			Error += rblxmath::equal(A, 0.5f, rblxmath::epsilon<float>()) ? 0 : 1;

			float const B = rblxmath::smoothstep(0.0f, Edge, 1.0f);
			Error += rblxmath::equal(B, 0.5f, rblxmath::epsilon<float>()) ? 0 : 1;

			float const C = rblxmath::smoothstep(0.0f, Edge, 1.0f);
			Error += rblxmath::equal(C, 0.5f, rblxmath::epsilon<float>()) ? 0 : 1;
		}

		// vec4 and float
		{
			rblxmath::vec4 Result = rblxmath::smoothstep(0.0f, Edge, rblxmath::vec4(1.0f));
			Error += rblxmath::all(rblxmath::equal(Result, rblxmath::vec4(0.5f), rblxmath::epsilon<float>())) ? 0 : 1;
		}

		// vec4 and vec4
		{
			rblxmath::vec4 Result = rblxmath::smoothstep(rblxmath::vec4(0.0f), rblxmath::vec4(Edge), rblxmath::vec4(1.0f));
			Error += rblxmath::all(rblxmath::equal(Result, rblxmath::vec4(0.5f), rblxmath::epsilon<float>())) ? 0 : 1;
		}

		return Error;
	}
}//namespace smoothstep_

namespace round_
{
	static int test()
	{
		int Error = 0;

		{
			float A = rblxmath::round(0.0f);
			Error += rblxmath::equal(A, 0.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float B = rblxmath::round(0.5f);
			Error += (rblxmath::equal(B, 1.0f, rblxmath::epsilon<float>()) || rblxmath::equal(B, 0.0f, rblxmath::epsilon<float>())) ? 0 : 1;
			float C = rblxmath::round(1.0f);
			Error += rblxmath::equal(C, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float D = rblxmath::round(0.1f);
			Error += rblxmath::equal(D, 0.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float E = rblxmath::round(0.9f);
			Error += rblxmath::equal(E, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float F = rblxmath::round(1.5f);
			Error += rblxmath::equal(F, 2.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float G = rblxmath::round(1.9f);
			Error += rblxmath::equal(G, 2.0f, rblxmath::epsilon<float>()) ? 0 : 1;
		}
	
		{
			float A = rblxmath::round(-0.0f);
			Error += rblxmath::equal(A, 0.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float B = rblxmath::round(-0.5f);
			Error += (rblxmath::equal(B, -1.0f, rblxmath::epsilon<float>()) || rblxmath::equal(B, 0.0f, rblxmath::epsilon<float>())) ? 0 : 1;
			float C = rblxmath::round(-1.0f);
			Error += rblxmath::equal(C, -1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float D = rblxmath::round(-0.1f);
			Error += rblxmath::equal(D, 0.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float E = rblxmath::round(-0.9f);
			Error += rblxmath::equal(E, -1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float F = rblxmath::round(-1.5f);
			Error += rblxmath::equal(F, -2.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float G = rblxmath::round(-1.9f);
			Error += rblxmath::equal(G, -2.0f, rblxmath::epsilon<float>()) ? 0 : 1;
		}
	
		return Error;
	}
}//namespace round_

namespace roundEven
{
	static int test()
	{
		int Error = 0;

		{
			float A1 = rblxmath::roundEven(-1.5f);
			Error += rblxmath::equal(A1, -2.0f, 0.0001f) ? 0 : 1;

			float A2 = rblxmath::roundEven(1.5f);
			Error += rblxmath::equal(A2, 2.0f, 0.0001f) ? 0 : 1;

			float A5 = rblxmath::roundEven(-2.5f);
			Error += rblxmath::equal(A5, -2.0f, 0.0001f) ? 0 : 1;

			float A6 = rblxmath::roundEven(2.5f);
			Error += rblxmath::equal(A6, 2.0f, 0.0001f) ? 0 : 1;

			float A3 = rblxmath::roundEven(-3.5f);
			Error += rblxmath::equal(A3, -4.0f, 0.0001f) ? 0 : 1;

			float A4 = rblxmath::roundEven(3.5f);
			Error += rblxmath::equal(A4, 4.0f, 0.0001f) ? 0 : 1;

			float C7 = rblxmath::roundEven(-4.5f);
			Error += rblxmath::equal(C7, -4.0f, 0.0001f) ? 0 : 1;

			float C8 = rblxmath::roundEven(4.5f);
			Error += rblxmath::equal(C8, 4.0f, 0.0001f) ? 0 : 1;

			float C1 = rblxmath::roundEven(-5.5f);
			Error += rblxmath::equal(C1, -6.0f, 0.0001f) ? 0 : 1;

			float C2 = rblxmath::roundEven(5.5f);
			Error += rblxmath::equal(C2, 6.0f, 0.0001f) ? 0 : 1;

			float C3 = rblxmath::roundEven(-6.5f);
			Error += rblxmath::equal(C3, -6.0f, 0.0001f) ? 0 : 1;

			float C4 = rblxmath::roundEven(6.5f);
			Error += rblxmath::equal(C4, 6.0f, 0.0001f) ? 0 : 1;

			float C5 = rblxmath::roundEven(-7.5f);
			Error += rblxmath::equal(C5, -8.0f, 0.0001f) ? 0 : 1;

			float C6 = rblxmath::roundEven(7.5f);
			Error += rblxmath::equal(C6, 8.0f, 0.0001f) ? 0 : 1;

			Error += 0;
		}

		{
			float A7 = rblxmath::roundEven(-2.4f);
			Error += rblxmath::equal(A7, -2.0f, 0.0001f) ? 0 : 1;

			float A8 = rblxmath::roundEven(2.4f);
			Error += rblxmath::equal(A8, 2.0f, 0.0001f) ? 0 : 1;

			float B1 = rblxmath::roundEven(-2.6f);
			Error += rblxmath::equal(B1, -3.0f, 0.0001f) ? 0 : 1;

			float B2 = rblxmath::roundEven(2.6f);
			Error += rblxmath::equal(B2, 3.0f, 0.0001f) ? 0 : 1;

			float B3 = rblxmath::roundEven(-2.0f);
			Error += rblxmath::equal(B3, -2.0f, 0.0001f) ? 0 : 1;

			float B4 = rblxmath::roundEven(2.0f);
			Error += rblxmath::equal(B4, 2.0f, 0.0001f) ? 0 : 1;

			Error += 0;
		}

		{
			float A = rblxmath::roundEven(0.0f);
			Error += rblxmath::equal(A, 0.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float B = rblxmath::roundEven(0.5f);
			Error += rblxmath::equal(B, 0.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float C = rblxmath::roundEven(1.0f);
			Error += rblxmath::equal(C, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float D = rblxmath::roundEven(0.1f);
			Error += rblxmath::equal(D, 0.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float E = rblxmath::roundEven(0.9f);
			Error += rblxmath::equal(E, 1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float F = rblxmath::roundEven(1.5f);
			Error += rblxmath::equal(F, 2.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float G = rblxmath::roundEven(1.9f);
			Error += rblxmath::equal(G, 2.0f, rblxmath::epsilon<float>()) ? 0 : 1;
		}

		{
			float A = rblxmath::roundEven(-0.0f);
			Error += rblxmath::equal(A,  0.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float B = rblxmath::roundEven(-0.5f);
			Error += rblxmath::equal(B, -0.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float C = rblxmath::roundEven(-1.0f);
			Error += rblxmath::equal(C, -1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float D = rblxmath::roundEven(-0.1f);
			Error += rblxmath::equal(D,  0.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float E = rblxmath::roundEven(-0.9f);
			Error += rblxmath::equal(E, -1.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float F = rblxmath::roundEven(-1.5f);
			Error += rblxmath::equal(F, -2.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float G = rblxmath::roundEven(-1.9f);
			Error += rblxmath::equal(G, -2.0f, rblxmath::epsilon<float>()) ? 0 : 1;
		}

		{
			float A = rblxmath::roundEven(1.5f);
			Error += rblxmath::equal(A, 2.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float B = rblxmath::roundEven(2.5f);
			Error += rblxmath::equal(B, 2.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float C = rblxmath::roundEven(3.5f);
			Error += rblxmath::equal(C, 4.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float D = rblxmath::roundEven(4.5f);
			Error += rblxmath::equal(D, 4.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float E = rblxmath::roundEven(5.5f);
			Error += rblxmath::equal(E, 6.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float F = rblxmath::roundEven(6.5f);
			Error += rblxmath::equal(F, 6.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float G = rblxmath::roundEven(7.5f);
			Error += rblxmath::equal(G, 8.0f, rblxmath::epsilon<float>()) ? 0 : 1;
		}
	
		{
			float A = rblxmath::roundEven(-1.5f);
			Error += rblxmath::equal(A, -2.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float B = rblxmath::roundEven(-2.5f);
			Error += rblxmath::equal(B, -2.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float C = rblxmath::roundEven(-3.5f);
			Error += rblxmath::equal(C, -4.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float D = rblxmath::roundEven(-4.5f);
			Error += rblxmath::equal(D, -4.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float E = rblxmath::roundEven(-5.5f);
			Error += rblxmath::equal(E, -6.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float F = rblxmath::roundEven(-6.5f);
			Error += rblxmath::equal(F, -6.0f, rblxmath::epsilon<float>()) ? 0 : 1;
			float G = rblxmath::roundEven(-7.5f);
			Error += rblxmath::equal(G, -8.0f, rblxmath::epsilon<float>()) ? 0 : 1;
		}

		return Error;
	}
}//namespace roundEven

namespace isnan_
{
	static int test()
	{
		int Error = 0;

		float Zero_f = 0.0;
		double Zero_d = 0.0;

		{
			Error += true == rblxmath::isnan(0.0/Zero_d) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isnan(rblxmath::dvec2(0.0 / Zero_d))) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isnan(rblxmath::dvec3(0.0 / Zero_d))) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isnan(rblxmath::dvec4(0.0 / Zero_d))) ? 0 : 1;
		}

		{
			Error += true == rblxmath::isnan(0.0f/Zero_f) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isnan(rblxmath::vec2(0.0f/Zero_f))) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isnan(rblxmath::vec3(0.0f/Zero_f))) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isnan(rblxmath::vec4(0.0f/Zero_f))) ? 0 : 1;
		}

		return Error;
	}
}//namespace isnan_

namespace isinf_
{
	static int test()
	{
		int Error = 0;

		float Zero_f = 0.0;
		double Zero_d = 0.0;

		{
			Error += true == rblxmath::isinf( 1.0/Zero_d) ? 0 : 1;
			Error += true == rblxmath::isinf(-1.0/Zero_d) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isinf(rblxmath::dvec2( 1.0/Zero_d))) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isinf(rblxmath::dvec2(-1.0/Zero_d))) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isinf(rblxmath::dvec3( 1.0/Zero_d))) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isinf(rblxmath::dvec3(-1.0/Zero_d))) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isinf(rblxmath::dvec4( 1.0/Zero_d))) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isinf(rblxmath::dvec4(-1.0/Zero_d))) ? 0 : 1;
		}

		{
			Error += true == rblxmath::isinf( 1.0f/Zero_f) ? 0 : 1;
			Error += true == rblxmath::isinf(-1.0f/Zero_f) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isinf(rblxmath::vec2( 1.0f/Zero_f))) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isinf(rblxmath::vec2(-1.0f/Zero_f))) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isinf(rblxmath::vec3( 1.0f/Zero_f))) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isinf(rblxmath::vec3(-1.0f/Zero_f))) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isinf(rblxmath::vec4( 1.0f/Zero_f))) ? 0 : 1;
			Error += true == rblxmath::any(rblxmath::isinf(rblxmath::vec4(-1.0f/Zero_f))) ? 0 : 1;
		}

		return Error;
	}
}//namespace isinf_

namespace sign
{
	template<typename genFIType> 
	RBLXMATH_FUNC_QUALIFIER genFIType sign_if(genFIType x)
	{
		RBLXMATH_STATIC_ASSERT(
			std::numeric_limits<genFIType>::is_iec559 ||
			(std::numeric_limits<genFIType>::is_signed && std::numeric_limits<genFIType>::is_integer), "'sign' only accept signed inputs");

		genFIType result;
		if(x > genFIType(0))
			result = genFIType(1);
		else if(x < genFIType(0))
			result = genFIType(-1);
		else
			result = genFIType(0);
		return result;
	}

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wsign-conversion"
#endif

	template<typename genFIType> 
	RBLXMATH_FUNC_QUALIFIER genFIType sign_alu1(genFIType x)
	{
		RBLXMATH_STATIC_ASSERT(
			std::numeric_limits<genFIType>::is_signed && std::numeric_limits<genFIType>::is_integer, 
			"'sign' only accept integer inputs");

		return (x >> 31) | (static_cast<unsigned>(-x) >> 31);
	}

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

	RBLXMATH_FUNC_QUALIFIER int sign_alu2(int x)
	{
		RBLXMATH_STATIC_ASSERT(std::numeric_limits<int>::is_signed && std::numeric_limits<int>::is_integer, "'sign' only accept integer inputs");

#		if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#			pragma warning(push)
#			pragma warning(disable : 4146) //cast truncates constant value
#		endif

		return -(static_cast<unsigned>(x) >> 31) | (-static_cast<unsigned>(x) >> 31);

#		if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#			pragma warning(pop)
#		endif
	}

	template<typename genFIType> 
	RBLXMATH_FUNC_QUALIFIER genFIType sign_sub(genFIType x)
	{
		RBLXMATH_STATIC_ASSERT(
			std::numeric_limits<genFIType>::is_signed && std::numeric_limits<genFIType>::is_integer, 
			"'sign' only accept integer inputs");

		return (static_cast<unsigned>(-x) >> 31) - (static_cast<unsigned>(x) >> 31);
	}

	template<typename genFIType> 
	RBLXMATH_FUNC_QUALIFIER genFIType sign_cmp(genFIType x)
	{
		RBLXMATH_STATIC_ASSERT(
			std::numeric_limits<genFIType>::is_signed && std::numeric_limits<genFIType>::is_integer, 
			"'sign' only accept integer inputs");

		return (x > 0) - (x < 0);
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
			{ std::numeric_limits<rblxmath::int32>::max(),  1},
			{ std::numeric_limits<rblxmath::int32>::min(), -1},
			{ 0, 0},
			{ 1, 1},
			{ 2, 1},
			{ 3, 1},
			{-1,-1},
			{-2,-1},
			{-3,-1}
		};

		int Error = 0;

		for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<rblxmath::int32>); ++i)
		{
			rblxmath::int32 Result = rblxmath::sign(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<rblxmath::int32>); ++i)
		{
			rblxmath::int32 Result = sign_cmp(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<rblxmath::int32>); ++i)
		{
			rblxmath::int32 Result = sign_if(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<rblxmath::int32>); ++i)
		{
			rblxmath::int32 Result = sign_alu1(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<rblxmath::int32>); ++i)
		{
			rblxmath::int32 Result = sign_alu2(Data[i].Value);
			Error += Data[i].Return == Result ? 0 : 1;
		}

		return Error;
	}

	static int test_i32vec4()
	{
		type<rblxmath::ivec4> const Data[] =
		{
			{rblxmath::ivec4( 1), rblxmath::ivec4( 1)},
			{rblxmath::ivec4( 0), rblxmath::ivec4( 0)},
			{rblxmath::ivec4( 2), rblxmath::ivec4( 1)},
			{rblxmath::ivec4( 3), rblxmath::ivec4( 1)},
			{rblxmath::ivec4(-1), rblxmath::ivec4(-1)},
			{rblxmath::ivec4(-2), rblxmath::ivec4(-1)},
			{rblxmath::ivec4(-3), rblxmath::ivec4(-1)}
		};

		int Error = 0;

		for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<rblxmath::ivec4>); ++i)
		{
			rblxmath::ivec4 Result = rblxmath::sign(Data[i].Value);
			Error += rblxmath::all(rblxmath::equal(Data[i].Return, Result)) ? 0 : 1;
		}

		return Error;
	}

	static int test_f32vec4()
	{
		type<rblxmath::vec4> const Data[] =
		{
			{rblxmath::vec4( 1), rblxmath::vec4( 1)},
			{rblxmath::vec4( 0), rblxmath::vec4( 0)},
			{rblxmath::vec4( 2), rblxmath::vec4( 1)},
			{rblxmath::vec4( 3), rblxmath::vec4( 1)},
			{rblxmath::vec4(-1), rblxmath::vec4(-1)},
			{rblxmath::vec4(-2), rblxmath::vec4(-1)},
			{rblxmath::vec4(-3), rblxmath::vec4(-1)}
		};

		int Error = 0;

		for(std::size_t i = 0; i < sizeof(Data) / sizeof(type<rblxmath::vec4>); ++i)
		{
			rblxmath::vec4 Result = rblxmath::sign(Data[i].Value);
			Error += rblxmath::all(rblxmath::equal(Data[i].Return, Result, rblxmath::epsilon<float>())) ? 0 : 1;
		}

		return Error;
	}

	static int test()
	{
		int Error = 0;

		Error += test_int32();
		Error += test_i32vec4();
		Error += test_f32vec4();

		return Error;
	}

	static int perf_rand(std::size_t Samples)
	{
		int Error = 0;

		std::size_t const Count = Samples;
		std::vector<rblxmath::int32> Input, Output;
		Input.resize(Count);
		Output.resize(Count);
		for(std::size_t i = 0; i < Count; ++i)
			Input[i] = static_cast<rblxmath::int32>(rblxmath::linearRand(-65536.f, 65536.f));

		std::clock_t Timestamp0 = std::clock();

		for(std::size_t i = 0; i < Count; ++i)
			Output[i] = sign_cmp(Input[i]);

		std::clock_t Timestamp1 = std::clock();

		for(std::size_t i = 0; i < Count; ++i)
			Output[i] = sign_if(Input[i]);

		std::clock_t Timestamp2 = std::clock();

		for(std::size_t i = 0; i < Count; ++i)
			Output[i] = sign_alu1(Input[i]);

		std::clock_t Timestamp3 = std::clock();

		for(std::size_t i = 0; i < Count; ++i)
			Output[i] = sign_alu2(Input[i]);

		std::clock_t Timestamp4 = std::clock();

		for(std::size_t i = 0; i < Count; ++i)
			Output[i] = sign_sub(Input[i]);

		std::clock_t Timestamp5 = std::clock();

		for(std::size_t i = 0; i < Count; ++i)
			Output[i] = rblxmath::sign(Input[i]);

		std::clock_t Timestamp6 = std::clock();

		std::printf("sign_cmp(rand) Time %d clocks\n", static_cast<int>(Timestamp1 - Timestamp0));
		std::printf("sign_if(rand) Time %d clocks\n", static_cast<int>(Timestamp2 - Timestamp1));
		std::printf("sign_alu1(rand) Time %d clocks\n", static_cast<int>(Timestamp3 - Timestamp2));
		std::printf("sign_alu2(rand) Time %d clocks\n", static_cast<int>(Timestamp4 - Timestamp3));
		std::printf("sign_sub(rand) Time %d clocks\n", static_cast<int>(Timestamp5 - Timestamp4));
		std::printf("rblxmath::sign(rand) Time %d clocks\n", static_cast<int>(Timestamp6 - Timestamp5));

		return Error;
	}

	static int perf_linear(std::size_t Samples)
	{
		int Error = 0;

		std::size_t const Count = Samples;
		std::vector<rblxmath::int32> Input, Output;
		Input.resize(Count);
		Output.resize(Count);
		for(std::size_t i = 0; i < Count; ++i)
			Input[i] = static_cast<rblxmath::int32>(i);

		std::clock_t Timestamp0 = std::clock();

		for(std::size_t i = 0; i < Count; ++i)
			Output[i] = sign_cmp(Input[i]);

		std::clock_t Timestamp1 = std::clock();

		for(std::size_t i = 0; i < Count; ++i)
			Output[i] = sign_if(Input[i]);

		std::clock_t Timestamp2 = std::clock();

		for(std::size_t i = 0; i < Count; ++i)
			Output[i] = sign_alu1(Input[i]);

		std::clock_t Timestamp3 = std::clock();

		for(std::size_t i = 0; i < Count; ++i)
			Output[i] = sign_alu2(Input[i]);

		std::clock_t Timestamp4 = std::clock();

		for(std::size_t i = 0; i < Count; ++i)
			Output[i] = sign_sub(Input[i]);

		std::clock_t Timestamp5 = std::clock();

		std::printf("sign_cmp(linear) Time %d clocks\n", static_cast<int>(Timestamp1 - Timestamp0));
		std::printf("sign_if(linear) Time %d clocks\n", static_cast<int>(Timestamp2 - Timestamp1));
		std::printf("sign_alu1(linear) Time %d clocks\n", static_cast<int>(Timestamp3 - Timestamp2));
		std::printf("sign_alu2(linear) Time %d clocks\n", static_cast<int>(Timestamp4 - Timestamp3));
		std::printf("sign_sub(linear) Time %d clocks\n", static_cast<int>(Timestamp5 - Timestamp4));

		return Error;
	}

	static int perf_linear_cal(std::size_t Samples)
	{
		int Error = 0;

		rblxmath::int32 const Count = static_cast<rblxmath::int32>(Samples);

		std::clock_t Timestamp0 = std::clock();
		rblxmath::int32 Sum = 0;

		for(rblxmath::int32 i = 1; i < Count; ++i)
			Sum += sign_cmp(i);

		std::clock_t Timestamp1 = std::clock();

		for(rblxmath::int32 i = 1; i < Count; ++i)
			Sum += sign_if(i);

		std::clock_t Timestamp2 = std::clock();

		for(rblxmath::int32 i = 1; i < Count; ++i)
			Sum += sign_alu1(i);

		std::clock_t Timestamp3 = std::clock();

		for(rblxmath::int32 i = 1; i < Count; ++i)
			Sum += sign_alu2(i);

		std::clock_t Timestamp4 = std::clock();

		for(rblxmath::int32 i = 1; i < Count; ++i)
			Sum += sign_sub(i);

		std::clock_t Timestamp5 = std::clock();

		std::printf("Sum %d\n", static_cast<int>(Sum));

		std::printf("sign_cmp(linear_cal) Time %d clocks\n", static_cast<int>(Timestamp1 - Timestamp0));
		std::printf("sign_if(linear_cal) Time %d clocks\n", static_cast<int>(Timestamp2 - Timestamp1));
		std::printf("sign_alu1(linear_cal) Time %d clocks\n", static_cast<int>(Timestamp3 - Timestamp2));
		std::printf("sign_alu2(linear_cal) Time %d clocks\n", static_cast<int>(Timestamp4 - Timestamp3));
		std::printf("sign_sub(linear_cal) Time %d clocks\n", static_cast<int>(Timestamp5 - Timestamp4));

		return Error;
	}

	static int perf(std::size_t Samples)
	{
		int Error(0);

		Error += perf_linear_cal(Samples);
		Error += perf_linear(Samples);
		Error += perf_rand(Samples);

		return Error;
	}
}//namespace sign

namespace frexp_
{
	static int test()
	{
		int Error = 0;

		{
			rblxmath::vec1 const x(1024);
			rblxmath::ivec1 exp;
			rblxmath::vec1 A = rblxmath::frexp(x, exp);
			Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec1(0.5), rblxmath::epsilon<float>())) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(exp, rblxmath::ivec1(11))) ? 0 : 1;
		}

		{
			rblxmath::vec2 const x(1024, 0.24);
			rblxmath::ivec2 exp;
			rblxmath::vec2 A = rblxmath::frexp(x, exp);
			Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec2(0.5, 0.96), rblxmath::epsilon<float>())) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(exp, rblxmath::ivec2(11, -2))) ? 0 : 1;
		}

		{
			rblxmath::vec3 const x(1024, 0.24, 0);
			rblxmath::ivec3 exp;
			rblxmath::vec3 A = rblxmath::frexp(x, exp);
			Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec3(0.5, 0.96, 0.0), rblxmath::epsilon<float>())) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(exp, rblxmath::ivec3(11, -2, 0))) ? 0 : 1;
		}

		{
			rblxmath::vec4 const x(1024, 0.24, 0, -1.33);
			rblxmath::ivec4 exp;
			rblxmath::vec4 A = rblxmath::frexp(x, exp);
			Error += rblxmath::all(rblxmath::equal(A, rblxmath::vec4(0.5, 0.96, 0.0, -0.665), rblxmath::epsilon<float>())) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(exp, rblxmath::ivec4(11, -2, 0, 1))) ? 0 : 1;
		}

		return Error;
	}
}//namespace frexp_

namespace ldexp_
{
	static int test()
	{
		int Error = 0;

		{
			rblxmath::vec1 A = rblxmath::vec1(0.5);
			rblxmath::ivec1 exp = rblxmath::ivec1(11);
			rblxmath::vec1 x = rblxmath::ldexp(A, exp);
			Error += rblxmath::all(rblxmath::equal(x, rblxmath::vec1(1024),0.00001f)) ? 0 : 1;
		}

		{
			rblxmath::vec2 A = rblxmath::vec2(0.5, 0.96);
			rblxmath::ivec2 exp = rblxmath::ivec2(11, -2);
			rblxmath::vec2 x = rblxmath::ldexp(A, exp);
			Error += rblxmath::all(rblxmath::equal(x, rblxmath::vec2(1024, .24),0.00001f)) ? 0 : 1;
		}

		{
			rblxmath::vec3 A = rblxmath::vec3(0.5, 0.96, 0.0);
			rblxmath::ivec3 exp = rblxmath::ivec3(11, -2, 0);
			rblxmath::vec3 x = rblxmath::ldexp(A, exp);
			Error += rblxmath::all(rblxmath::equal(x, rblxmath::vec3(1024, .24, 0),0.00001f)) ? 0 : 1;
		}

		{
			rblxmath::vec4 A = rblxmath::vec4(0.5, 0.96, 0.0, -0.665);
			rblxmath::ivec4 exp = rblxmath::ivec4(11, -2, 0, 1);
			rblxmath::vec4 x = rblxmath::ldexp(A, exp);
			Error += rblxmath::all(rblxmath::equal(x, rblxmath::vec4(1024, .24, 0, -1.33),0.00001f)) ? 0 : 1;
		}

		return Error;
	}
}//namespace ldexp_

static int test_constexpr()
{
#if RBLXMATH_HAS_CONSTEXPR
	static_assert(rblxmath::abs(1.0f) > 0.0f, "RBLXMATH: Failed constexpr");
	constexpr rblxmath::vec1 const A = rblxmath::abs(rblxmath::vec1(1.0f));
	constexpr rblxmath::vec2 const B = rblxmath::abs(rblxmath::vec2(1.0f));
	constexpr rblxmath::vec3 const C = rblxmath::abs(rblxmath::vec3(1.0f));
	constexpr rblxmath::vec4 const D = rblxmath::abs(rblxmath::vec4(1.0f));

	static_assert(rblxmath::all(rblxmath::equal(A, rblxmath::vec1(1.0f), rblxmath::epsilon<float>())), "RBLXMATH: Failed constexpr");
	static_assert(rblxmath::all(rblxmath::equal(B, rblxmath::vec2(1.0f), rblxmath::epsilon<float>())), "RBLXMATH: Failed constexpr");
	static_assert(rblxmath::all(rblxmath::equal(C, rblxmath::vec3(1.0f), rblxmath::epsilon<float>())), "RBLXMATH: Failed constexpr");
	static_assert(rblxmath::all(rblxmath::equal(D, rblxmath::vec4(1.0f), rblxmath::epsilon<float>())), "RBLXMATH: Failed constexpr");
#endif // RBLXMATH_HAS_CONSTEXPR

	return 0;
}

int main()
{
	int Error = 0;

	Error += test_constexpr();
	Error += sign::test();
	Error += floor_::test();
	Error += mod_::test();
	Error += modf_::test();
	Error += floatBitsToInt::test();
	Error += floatBitsToUint::test();
	Error += mix_::test();
	Error += step_::test();
	Error += smoothstep_::test();
	Error += max_::test();
	Error += min_::test();
	Error += clamp_::test();
	Error += round_::test();
	Error += roundEven::test();
	Error += isnan_::test();
	Error += isinf_::test();
	Error += frexp_::test();
	Error += ldexp_::test();

#	ifdef NDEBUG
		std::size_t Samples = 1000;
#	else
		std::size_t Samples = 1;
#	endif
	Error += sign::perf(Samples);

	Error += min_::perf(Samples);

	return Error;
}

#if(RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC)
#	pragma warning(pop)
#endif
