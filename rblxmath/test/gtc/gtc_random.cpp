#define RBLXMATH_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <rblxmath/gtc/random.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/gtc/type_precision.hpp>
#if RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG
#	include <array>
#endif

std::size_t const TestSamples = 10000;

static int test_linearRand()
{
	int Error = 0;

	rblxmath::int32 const Min = 16;
	rblxmath::int32 const Max = 32;

	{
		rblxmath::u8vec2 AMin(std::numeric_limits<rblxmath::u8>::max());
		rblxmath::u8vec2 AMax(std::numeric_limits<rblxmath::u8>::min());
		{
			for(std::size_t i = 0; i < TestSamples; ++i)
			{
				rblxmath::u8vec2 A = rblxmath::linearRand(rblxmath::u8vec2(Min), rblxmath::u8vec2(Max));
				AMin = rblxmath::min(AMin, A);
				AMax = rblxmath::max(AMax, A);

				if(!rblxmath::all(rblxmath::lessThanEqual(A, rblxmath::u8vec2(Max))))
					++Error;
				if(!rblxmath::all(rblxmath::greaterThanEqual(A, rblxmath::u8vec2(Min))))
					++Error;
				assert(!Error);
			}

			Error += rblxmath::all(rblxmath::equal(AMin, rblxmath::u8vec2(Min))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(AMax, rblxmath::u8vec2(Max))) ? 0 : 1;
			assert(!Error);
		}

		rblxmath::u16vec2 BMin(std::numeric_limits<rblxmath::u16>::max());
		rblxmath::u16vec2 BMax(std::numeric_limits<rblxmath::u16>::min());
		{
			for(std::size_t i = 0; i < TestSamples; ++i)
			{
				rblxmath::u16vec2 B = rblxmath::linearRand(rblxmath::u16vec2(Min), rblxmath::u16vec2(Max));
				BMin = rblxmath::min(BMin, B);
				BMax = rblxmath::max(BMax, B);

				if(!rblxmath::all(rblxmath::lessThanEqual(B, rblxmath::u16vec2(Max))))
					++Error;
				if(!rblxmath::all(rblxmath::greaterThanEqual(B, rblxmath::u16vec2(Min))))
					++Error;
				assert(!Error);
			}

			Error += rblxmath::all(rblxmath::equal(BMin, rblxmath::u16vec2(Min))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(BMax, rblxmath::u16vec2(Max))) ? 0 : 1;
			assert(!Error);
		}

		rblxmath::u32vec2 CMin(std::numeric_limits<rblxmath::u32>::max());
		rblxmath::u32vec2 CMax(std::numeric_limits<rblxmath::u32>::min());
		{
			for(std::size_t i = 0; i < TestSamples; ++i)
			{
				rblxmath::u32vec2 C = rblxmath::linearRand(rblxmath::u32vec2(Min), rblxmath::u32vec2(Max));
				CMin = rblxmath::min(CMin, C);
				CMax = rblxmath::max(CMax, C);

				if(!rblxmath::all(rblxmath::lessThanEqual(C, rblxmath::u32vec2(Max))))
					++Error;
				if(!rblxmath::all(rblxmath::greaterThanEqual(C, rblxmath::u32vec2(Min))))
					++Error;
				assert(!Error);
			}

			Error += rblxmath::all(rblxmath::equal(CMin, rblxmath::u32vec2(Min))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(CMax, rblxmath::u32vec2(Max))) ? 0 : 1;
			assert(!Error);
		}

		rblxmath::u64vec2 DMin(std::numeric_limits<rblxmath::u64>::max());
		rblxmath::u64vec2 DMax(std::numeric_limits<rblxmath::u64>::min());
		{
			for(std::size_t i = 0; i < TestSamples; ++i)
			{
				rblxmath::u64vec2 D = rblxmath::linearRand(rblxmath::u64vec2(Min), rblxmath::u64vec2(Max));
				DMin = rblxmath::min(DMin, D);
				DMax = rblxmath::max(DMax, D);

				if(!rblxmath::all(rblxmath::lessThanEqual(D, rblxmath::u64vec2(Max))))
					++Error;
				if(!rblxmath::all(rblxmath::greaterThanEqual(D, rblxmath::u64vec2(Min))))
					++Error;
				assert(!Error);
			}

			Error += rblxmath::all(rblxmath::equal(DMin, rblxmath::u64vec2(Min))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(DMax, rblxmath::u64vec2(Max))) ? 0 : 1;
			assert(!Error);
		}
	}

	{
		rblxmath::i8vec2 AMin(std::numeric_limits<rblxmath::i8>::max());
		rblxmath::i8vec2 AMax(std::numeric_limits<rblxmath::i8>::min());
		{
			for(std::size_t i = 0; i < TestSamples; ++i)
			{
				rblxmath::i8vec2 A = rblxmath::linearRand(rblxmath::i8vec2(Min), rblxmath::i8vec2(Max));
				AMin = rblxmath::min(AMin, A);
				AMax = rblxmath::max(AMax, A);

				if(!rblxmath::all(rblxmath::lessThanEqual(A, rblxmath::i8vec2(Max))))
					++Error;
				if(!rblxmath::all(rblxmath::greaterThanEqual(A, rblxmath::i8vec2(Min))))
					++Error;
				assert(!Error);
			}

			Error += rblxmath::all(rblxmath::equal(AMin, rblxmath::i8vec2(Min))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(AMax, rblxmath::i8vec2(Max))) ? 0 : 1;
			assert(!Error);
		}

		rblxmath::i16vec2 BMin(std::numeric_limits<rblxmath::i16>::max());
		rblxmath::i16vec2 BMax(std::numeric_limits<rblxmath::i16>::min());
		{
			for(std::size_t i = 0; i < TestSamples; ++i)
			{
				rblxmath::i16vec2 B = rblxmath::linearRand(rblxmath::i16vec2(Min), rblxmath::i16vec2(Max));
				BMin = rblxmath::min(BMin, B);
				BMax = rblxmath::max(BMax, B);

				if(!rblxmath::all(rblxmath::lessThanEqual(B, rblxmath::i16vec2(Max))))
					++Error;
				if(!rblxmath::all(rblxmath::greaterThanEqual(B, rblxmath::i16vec2(Min))))
					++Error;
				assert(!Error);
			}

			Error += rblxmath::all(rblxmath::equal(BMin, rblxmath::i16vec2(Min))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(BMax, rblxmath::i16vec2(Max))) ? 0 : 1;
			assert(!Error);
		}

		rblxmath::i32vec2 CMin(std::numeric_limits<rblxmath::i32>::max());
		rblxmath::i32vec2 CMax(std::numeric_limits<rblxmath::i32>::min());
		{
			for(std::size_t i = 0; i < TestSamples; ++i)
			{
				rblxmath::i32vec2 C = rblxmath::linearRand(rblxmath::i32vec2(Min), rblxmath::i32vec2(Max));
				CMin = rblxmath::min(CMin, C);
				CMax = rblxmath::max(CMax, C);

				if(!rblxmath::all(rblxmath::lessThanEqual(C, rblxmath::i32vec2(Max))))
					++Error;
				if(!rblxmath::all(rblxmath::greaterThanEqual(C, rblxmath::i32vec2(Min))))
					++Error;
				assert(!Error);
			}

			Error += rblxmath::all(rblxmath::equal(CMin, rblxmath::i32vec2(Min))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(CMax, rblxmath::i32vec2(Max))) ? 0 : 1;
			assert(!Error);
		}

		rblxmath::i64vec2 DMin(std::numeric_limits<rblxmath::i64>::max());
		rblxmath::i64vec2 DMax(std::numeric_limits<rblxmath::i64>::min());
		{
			for(std::size_t i = 0; i < TestSamples; ++i)
			{
				rblxmath::i64vec2 D = rblxmath::linearRand(rblxmath::i64vec2(Min), rblxmath::i64vec2(Max));
				DMin = rblxmath::min(DMin, D);
				DMax = rblxmath::max(DMax, D);

				if(!rblxmath::all(rblxmath::lessThanEqual(D, rblxmath::i64vec2(Max))))
					++Error;
				if(!rblxmath::all(rblxmath::greaterThanEqual(D, rblxmath::i64vec2(Min))))
					++Error;
				assert(!Error);
			}

			Error += rblxmath::all(rblxmath::equal(DMin, rblxmath::i64vec2(Min))) ? 0 : 1;
			Error += rblxmath::all(rblxmath::equal(DMax, rblxmath::i64vec2(Max))) ? 0 : 1;
			assert(!Error);
		}
	}

	for(std::size_t i = 0; i < TestSamples; ++i)
	{
		rblxmath::f32vec2 const A(rblxmath::linearRand(rblxmath::f32vec2(static_cast<float>(Min)), rblxmath::f32vec2(static_cast<float>(Max))));
		if(!rblxmath::all(rblxmath::lessThanEqual(A, rblxmath::f32vec2(static_cast<float>(Max)))))
			++Error;
		if(!rblxmath::all(rblxmath::greaterThanEqual(A, rblxmath::f32vec2(static_cast<float>(Min)))))
			++Error;

		rblxmath::f64vec2 const B(rblxmath::linearRand(rblxmath::f64vec2(Min), rblxmath::f64vec2(Max)));
		if(!rblxmath::all(rblxmath::lessThanEqual(B, rblxmath::f64vec2(Max))))
			++Error;
		if(!rblxmath::all(rblxmath::greaterThanEqual(B, rblxmath::f64vec2(Min))))
			++Error;
		assert(!Error);
	}

	{
		float ResultFloat = 0.0f;
		double ResultDouble = 0.0;
		for(std::size_t i = 0; i < TestSamples; ++i)
		{
			ResultFloat += rblxmath::linearRand(-1.0f, 1.0f);
			ResultDouble += rblxmath::linearRand(-1.0, 1.0);
		}

		Error += rblxmath::epsilonEqual(ResultFloat, 0.0f, 0.0001f);
		Error += rblxmath::epsilonEqual(ResultDouble, 0.0, 0.0001);
		assert(!Error);
	}

	return Error;
}

static int test_circularRand()
{
	int Error = 0;

	{
		std::size_t Max = TestSamples;
		float ResultFloat = 0.0f;
		double ResultDouble = 0.0;
		double Radius = 2.0;

		for(std::size_t i = 0; i < Max; ++i)
		{
			ResultFloat += rblxmath::length(rblxmath::circularRand(1.0f));
			ResultDouble += rblxmath::length(rblxmath::circularRand(Radius));
		}

		Error += rblxmath::epsilonEqual(ResultFloat, float(Max), 0.01f) ? 0 : 1;
		Error += rblxmath::epsilonEqual(ResultDouble, double(Max) * double(Radius), 0.01) ? 0 : 1;
		assert(!Error);
	}

	return Error;
}

static int test_sphericalRand()
{
	int Error = 0;

	{
		std::size_t Max = TestSamples;
		float ResultFloatA = 0.0f;
		float ResultFloatB = 0.0f;
		float ResultFloatC = 0.0f;
		double ResultDoubleA = 0.0;
		double ResultDoubleB = 0.0;
		double ResultDoubleC = 0.0;

		for(std::size_t i = 0; i < Max; ++i)
		{
			ResultFloatA += rblxmath::length(rblxmath::sphericalRand(1.0f));
			ResultDoubleA += rblxmath::length(rblxmath::sphericalRand(1.0));
			ResultFloatB += rblxmath::length(rblxmath::sphericalRand(2.0f));
			ResultDoubleB += rblxmath::length(rblxmath::sphericalRand(2.0));
			ResultFloatC += rblxmath::length(rblxmath::sphericalRand(3.0f));
			ResultDoubleC += rblxmath::length(rblxmath::sphericalRand(3.0));
		}

		Error += rblxmath::epsilonEqual(ResultFloatA, float(Max), 0.01f) ? 0 : 1;
		Error += rblxmath::epsilonEqual(ResultDoubleA, double(Max), 0.0001) ? 0 : 1;
		Error += rblxmath::epsilonEqual(ResultFloatB, float(Max * 2), 0.01f) ? 0 : 1;
		Error += rblxmath::epsilonEqual(ResultDoubleB, double(Max * 2), 0.0001) ? 0 : 1;
		Error += rblxmath::epsilonEqual(ResultFloatC, float(Max * 3), 0.01f) ? 0 : 1;
		Error += rblxmath::epsilonEqual(ResultDoubleC, double(Max * 3), 0.01) ? 0 : 1;
		assert(!Error);
	}

	return Error;
}

static int test_diskRand()
{
	int Error = 0;

	{
		float ResultFloat = 0.0f;
		double ResultDouble = 0.0;

		for(std::size_t i = 0; i < TestSamples; ++i)
		{
			ResultFloat += rblxmath::length(rblxmath::diskRand(2.0f));
			ResultDouble += rblxmath::length(rblxmath::diskRand(2.0));
		}

		Error += ResultFloat < float(TestSamples) * 2.f ? 0 : 1;
		Error += ResultDouble < double(TestSamples) * 2.0 ? 0 : 1;
		assert(!Error);
	}

	return Error;
}

static int test_ballRand()
{
	int Error = 0;

	{
		float ResultFloat = 0.0f;
		double ResultDouble = 0.0;

		for(std::size_t i = 0; i < TestSamples; ++i)
		{
			ResultFloat += rblxmath::length(rblxmath::ballRand(2.0f));
			ResultDouble += rblxmath::length(rblxmath::ballRand(2.0));
		}

		Error += ResultFloat < float(TestSamples) * 2.f ? 0 : 1;
		Error += ResultDouble < double(TestSamples) * 2.0 ? 0 : 1;
		assert(!Error);
	}

	return Error;
}
/*
#if(RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG)
int test_grid()
{
	int Error = 0;

	typedef std::array<int, 8> colors;
	typedef std::array<int, 8 * 8> grid;

	grid Grid;
	colors Colors;

	grid GridBest;
	colors ColorsBest;

	while(true)
	{
		for(std::size_t i = 0; i < Grid.size(); ++i)
			Grid[i] = int(rblxmath::linearRand(0.0, 8.0 * 8.0 * 8.0 - 1.0) / 64.0);

		for(std::size_t i = 0; i < Grid.size(); ++i)
			++Colors[Grid[i]];

		bool Exit = true;
		for(std::size_t i = 0; i < Colors.size(); ++i)
		{
			if(Colors[i] == 8)
				continue;

			Exit = false;
			break;
		}

		if(Exit == true)
			break;
	}

	return Error;
}
#endif
*/
int main()
{
	int Error = 0;

	Error += test_linearRand();
	Error += test_circularRand();
	Error += test_sphericalRand();
	Error += test_diskRand();
	Error += test_ballRand();
/*
#if(RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG)
	Error += test_grid();
#endif
*/
	return Error;
}
