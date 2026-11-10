/// Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
///
/// Developed at SunPro, a Sun Microsystems, Inc. business.
/// Permission to use, copy, modify, and distribute this
/// software is freely granted, provided that this notice
/// is preserved.

#include "../detail/type_float.hpp"
#include "../ext/scalar_constants.hpp"
#include <cmath>
#include <cfloat>

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#	pragma warning(push)
#	pragma warning(disable : 4127)
#	pragma warning(disable : 4365) // '=': signed/unsigned mismatch
#elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wsign-conversion"
#	pragma clang diagnostic ignored "-Wpadded"
#endif

typedef union
{
	float value;
	/* FIXME: Assumes 32 bit int.  */
	unsigned int word;
} ieee_float_shape_type;

typedef union
{
	double value;
	struct
	{
		int lsw;
		int msw;
	} parts;
} ieee_double_shape_type;

#define RBLXMATH_EXTRACT_WORDS(ix0,ix1,d)		\
	do {									\
		ieee_double_shape_type ew_u;		\
		ew_u.value = (d);					\
		(ix0) = ew_u.parts.msw;				\
		(ix1) = ew_u.parts.lsw;				\
	} while (0)

#define RBLXMATH_GET_FLOAT_WORD(i,d)				\
	do {									\
		ieee_float_shape_type gf_u;			\
		gf_u.value = (d);					\
		(i) = static_cast<int>(gf_u.word);	\
	} while (0)

#define RBLXMATH_SET_FLOAT_WORD(d,i)				\
	do {									\
		ieee_float_shape_type sf_u;			\
		sf_u.word = static_cast<unsigned int>(i);	\
		(d) = sf_u.value;					\
	} while (0)

#define RBLXMATH_INSERT_WORDS(d,ix0,ix1)			\
	do {									\
		ieee_double_shape_type iw_u;		\
		iw_u.parts.msw = (ix0);				\
		iw_u.parts.lsw = (ix1);				\
		(d) = iw_u.value;					\
	} while (0)

namespace rblxmath{
namespace detail
{
	RBLXMATH_FUNC_QUALIFIER float nextafterf(float x, float y)
	{
		volatile float t;
		int hx, hy, ix, iy;

		RBLXMATH_GET_FLOAT_WORD(hx, x);
		RBLXMATH_GET_FLOAT_WORD(hy, y);
		ix = hx & 0x7fffffff;		// |x|
		iy = hy & 0x7fffffff;		// |y|

		if((ix > 0x7f800000) ||	// x is nan
			(iy > 0x7f800000))	// y is nan
			return x + y;
		if(abs(y - x) <= epsilon<float>())
			return y;		// x=y, return y
		if(ix == 0)
		{				// x == 0
			RBLXMATH_SET_FLOAT_WORD(x, (hy & 0x80000000) | 1);// return +-minsubnormal
			t = x * x;
			if(abs(t - x) <= epsilon<float>())
				return t;
			else
				return x;	// raise underflow flag
		}
		if(hx >= 0)
		{						// x > 0
			if(hx > hy)			// x > y, x -= ulp
				hx -= 1;
			else				// x < y, x += ulp
				hx += 1;
		}
		else
		{						// x < 0
			if(hy >= 0 || hx > hy)	// x < y, x -= ulp
				hx -= 1;
			else				// x > y, x += ulp
				hx += 1;
		}
		hy = hx & 0x7f800000;
		if(hy >= 0x7f800000)
			return x + x;  		// overflow
		if(hy < 0x00800000)		// underflow
		{
			t = x * x;
			if(abs(t - x) > epsilon<float>())
			{					// raise underflow flag
				RBLXMATH_SET_FLOAT_WORD(y, hx);
				return y;
			}
		}
		RBLXMATH_SET_FLOAT_WORD(x, hx);
		return x;
	}

	RBLXMATH_FUNC_QUALIFIER double nextafter(double x, double y)
	{
		volatile double t;
		int hx, hy, ix, iy;
		unsigned int lx, ly;

		RBLXMATH_EXTRACT_WORDS(hx, lx, x);
		RBLXMATH_EXTRACT_WORDS(hy, ly, y);
		ix = hx & 0x7fffffff;								// |x|
		iy = hy & 0x7fffffff;								// |y|

		if(((ix >= 0x7ff00000) && ((ix - 0x7ff00000) | lx) != 0) ||	// x is nan
			((iy >= 0x7ff00000) && ((iy - 0x7ff00000) | ly) != 0))	// y is nan
			return x + y;
		if(abs(y - x) <= epsilon<double>())
			return y;									// x=y, return y
		if((ix | lx) == 0)
		{													// x == 0
			RBLXMATH_INSERT_WORDS(x, hy & 0x80000000, 1);		// return +-minsubnormal
			t = x * x;
			if(abs(t - x) <= epsilon<double>())
				return t;
			else
				return x;   // raise underflow flag
		}
		if(hx >= 0) {                             // x > 0
			if(hx > hy || ((hx == hy) && (lx > ly))) {    // x > y, x -= ulp
				if(lx == 0) hx -= 1;
				lx -= 1;
			}
			else {                            // x < y, x += ulp
				lx += 1;
				if(lx == 0) hx += 1;
			}
		}
		else {                                // x < 0
			if(hy >= 0 || hx > hy || ((hx == hy) && (lx > ly))){// x < y, x -= ulp
				if(lx == 0) hx -= 1;
				lx -= 1;
			}
			else {                            // x > y, x += ulp
				lx += 1;
				if(lx == 0) hx += 1;
			}
		}
		hy = hx & 0x7ff00000;
		if(hy >= 0x7ff00000)
			return x + x;			// overflow
		if(hy < 0x00100000)
		{						// underflow
			t = x * x;
			if(abs(t - x) > epsilon<double>())
			{					// raise underflow flag
				RBLXMATH_INSERT_WORDS(y, hx, lx);
				return y;
			}
		}
		RBLXMATH_INSERT_WORDS(x, hx, lx);
		return x;
	}
}//namespace detail
}//namespace rblxmath

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#	pragma warning(pop)
#elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

namespace rblxmath
{
	template<>
	RBLXMATH_FUNC_QUALIFIER float nextFloat(float x)
	{
#		if RBLXMATH_HAS_CXX11_STL
			return std::nextafter(x, std::numeric_limits<float>::max());
#		elif((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) || ((RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL) && (RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_WINDOWS)))
			return detail::nextafterf(x, FLT_MAX);
#		elif(RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_ANDROID)
			return __builtin_nextafterf(x, FLT_MAX);
#		else
			return nextafterf(x, FLT_MAX);
#		endif
	}

	template<>
	RBLXMATH_FUNC_QUALIFIER double nextFloat(double x)
	{
#		if RBLXMATH_HAS_CXX11_STL
			return std::nextafter(x, std::numeric_limits<double>::max());
#		elif((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) || ((RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL) && (RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_WINDOWS)))
			return detail::nextafter(x, std::numeric_limits<double>::max());
#		elif(RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_ANDROID)
			return __builtin_nextafter(x, DBL_MAX);
#		else
			return nextafter(x, DBL_MAX);
#		endif
	}

	template<typename T>
	RBLXMATH_FUNC_QUALIFIER T nextFloat(T x, int ULPs)
	{
		RBLXMATH_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || RBLXMATH_CONFIG_UNRESTRICTED_FLOAT, "'next_float' only accept floating-point input");
		assert(ULPs >= 0);

		T temp = x;
		for(int i = 0; i < ULPs; ++i)
			temp = nextFloat(temp);
		return temp;
	}

	RBLXMATH_FUNC_QUALIFIER float prevFloat(float x)
	{
#		if RBLXMATH_HAS_CXX11_STL
			return std::nextafter(x, std::numeric_limits<float>::min());
#		elif((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) || ((RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL) && (RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_WINDOWS)))
			return detail::nextafterf(x, FLT_MIN);
#		elif(RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_ANDROID)
			return __builtin_nextafterf(x, FLT_MIN);
#		else
			return nextafterf(x, FLT_MIN);
#		endif
	}

	RBLXMATH_FUNC_QUALIFIER double prevFloat(double x)
	{
#		if RBLXMATH_HAS_CXX11_STL
			return std::nextafter(x, std::numeric_limits<double>::min());
#		elif((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) || ((RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL) && (RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_WINDOWS)))
			return _nextafter(x, DBL_MIN);
#		elif(RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_ANDROID)
			return __builtin_nextafter(x, DBL_MIN);
#		else
			return nextafter(x, DBL_MIN);
#		endif
	}

	template<typename T>
	RBLXMATH_FUNC_QUALIFIER T prevFloat(T x, int ULPs)
	{
		RBLXMATH_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || RBLXMATH_CONFIG_UNRESTRICTED_FLOAT, "'prev_float' only accept floating-point input");
		assert(ULPs >= 0);

		T temp = x;
		for(int i = 0; i < ULPs; ++i)
			temp = prevFloat(temp);
		return temp;
	}

	RBLXMATH_FUNC_QUALIFIER int floatDistance(float x, float y)
	{
		detail::float_t<float> const a(x);
		detail::float_t<float> const b(y);

		return abs(a.i - b.i);
	}

	RBLXMATH_FUNC_QUALIFIER int64 floatDistance(double x, double y)
	{
		detail::float_t<double> const a(x);
		detail::float_t<double> const b(y);

		return abs(a.i - b.i);
	}
}//namespace rblxmath
